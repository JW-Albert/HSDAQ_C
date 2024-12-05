#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "./hsdaql.h"

#define MQ_HOST "localhost"
#define MQ_USER "user"
#define MQ_PASS "password"
#define MQ_QUEUE "data_queue"
#define SAMPLE_RATE 12800
#define BUFFERSIZE 1000
#define INITIAL_RETRY_INTERVAL 1000
#define RETRY_BACKOFF 2
#define RETRY_MAX_INTERVAL 60000

amqp_connection_state_t establish_rabbitmq_connection(
    const char *host,
    const char *username,
    const char *password,
    int retries
) {
    int retry_interval = INITIAL_RETRY_INTERVAL;
    amqp_connection_state_t conn;

    while (1) {
        conn = amqp_new_connection();
        amqp_socket_t *socket = amqp_tcp_socket_new(conn);
        if (!socket) {
            fprintf(stderr, "Failed to create RabbitMQ socket.\n");
            exit(EXIT_FAILURE);
        }

        if (amqp_socket_open(socket, host, 5672) == AMQP_STATUS_OK) {
            amqp_rpc_reply_t login_reply = amqp_login(
                conn,
                "/",
                0,
                131072,
                0,
                AMQP_SASL_METHOD_PLAIN,
                username,
                password
            );

            if (login_reply.reply_type == AMQP_RESPONSE_NORMAL) {
                printf("RabbitMQ connection established.\n");
                return conn;
            } else {
                fprintf(stderr, "Failed to login to RabbitMQ. Error type: %d\n", login_reply.reply_type);
            }
        } else {
            fprintf(stderr, "Failed to open RabbitMQ connection.\n");
        }

        fprintf(stderr, "(%d retries left) Retrying in %d ms...\n", retries, retry_interval);
        if (--retries == 0) {
            fprintf(stderr, "Exceeded maximum retries. Failed to connect to RabbitMQ.\n");
            exit(EXIT_FAILURE);
        }

        usleep(retry_interval * 1000);
        retry_interval = (retry_interval * RETRY_BACKOFF > RETRY_MAX_INTERVAL)
                             ? RETRY_MAX_INTERVAL
                             : retry_interval * RETRY_BACKOFF;
    }
}

void send_to_rabbitmq(amqp_connection_state_t conn, const float *data, size_t length) {
    char message[1024];
    snprintf(message, sizeof(message), "Sample Data: ");
    for (size_t i = 0; i < length; i++) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%f ", data[i]);
        strncat(message, buf, sizeof(message) - strlen(message) - 1);
    }

    amqp_bytes_t body = {.len = strlen(message), .bytes = message};
    amqp_basic_publish(conn, 1, amqp_cstring_bytes(""), amqp_cstring_bytes(MQ_QUEUE), 0, 0, NULL, body);

    printf("Data sent to RabbitMQ: %s\n", message);
}

int main(void) {
    HANDLE hHS;
    const char *IPadd = "192.168.9.40";
    amqp_connection_state_t conn = establish_rabbitmq_connection(MQ_HOST, MQ_USER, MQ_PASS, 10);

    float fdataBuffer[BUFFERSIZE];
    float accumulatedData[BUFFERSIZE];
    size_t accumulatedCount = 0;

    char tmp[128] = {0};
    sprintf(tmp, "%s,9999,10010", IPadd);

    // Step 1: Create a TCP connection with ET-AR400
    hHS = HS_Device_Create(tmp);
    if (hHS == NULL) {
        fprintf(stderr, "Failed to connect to device.\n");
        return -1;
    }

    // Step 2: Set AI Scan Parameters
    if (!HS_SetAIScanParam(hHS, 2, 0, 0, SAMPLE_RATE, 0, 0, 0)) {
        fprintf(stderr, "Failed to set scan parameters. Error: 0x%x\n", HS_GetLastError());
        HS_Device_Release(hHS);
        return -1;
    }

    // Step 3: Start AI Scan
    if (!HS_StartAIScan(hHS)) {
        fprintf(stderr, "Failed to start AI scan. Error: 0x%x\n", HS_GetLastError());
        HS_Device_Release(hHS);
        return -1;
    }

    // Step 4: Collect and upload data
    while (1) {
        WORD BufferStatus = 0;
        UL32 ulleng = 0;

        if (!HS_GetAIBufferStatus(hHS, &BufferStatus, &ulleng)) {
            fprintf(stderr, "Failed to get AI buffer status.\n");
            break;
        }

        if (ulleng > 0) {
            UL32 readsize = (ulleng > BUFFERSIZE) ? BUFFERSIZE : ulleng;
            readsize = HS_GetAIBuffer(hHS, fdataBuffer, readsize);

            if (readsize > 0) {
                for (UL32 i = 0; i < readsize; i++) {
                    accumulatedData[accumulatedCount++] = fdataBuffer[i];
                    if (accumulatedCount >= SAMPLE_RATE) {
                        send_to_rabbitmq(conn, accumulatedData, accumulatedCount);
                        accumulatedCount = 0;
                    }
                }
            }
        }
    }

    // Step 5: Stop AI Scan and release resources
    HS_StopAIScan(hHS);
    printf("Stop Scan\n");
    HS_Device_Release(hHS);
    printf("Release Device\n");
    amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
    amqp_destroy_connection(conn);

    return 0;
}