#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <amqp.h>
#include <amqp_tcp_socket.h>
#include "./hsdaql.h"

#define MQ_HOST "localhost"
#define MQ_USER "user"
#define MQ_PASS "password"
#define MQ_QUEUE "data_queue"
#define SAMPLE_RATE 12800
#define BUFFERSIZE 1000

// RabbitMQ 連線函數
amqp_connection_state_t establish_rabbitmq_connection(int retries) {
    int retry_interval = 1000; // 初始重試間隔（毫秒）
    amqp_connection_state_t conn;

    while (1) {
        conn = amqp_new_connection();
        amqp_socket_t *socket = amqp_tcp_socket_new(conn);
        if (!socket) {
            fprintf(stderr, "Failed to create RabbitMQ socket.\n");
            exit(EXIT_FAILURE);
        }

        if (amqp_socket_open(socket, MQ_HOST, 5672) == AMQP_STATUS_OK) {
            amqp_rpc_reply_t login_reply = amqp_login(
                conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, MQ_USER, MQ_PASS
            );

            if (login_reply.reply_type == AMQP_RESPONSE_NORMAL) {
                printf("RabbitMQ connection established.\n");
                return conn;
            } else {
                fprintf(stderr, "Failed to login to RabbitMQ. Error: %d\n", login_reply.reply_type);
            }
        } else {
            fprintf(stderr, "Failed to open RabbitMQ connection.\n");
        }

        fprintf(stderr, "(%d retries left) Retrying in %d ms...\n", retries, retry_interval);
        if (--retries == 0) {
            fprintf(stderr, "Exceeded maximum retries. Failed to connect to RabbitMQ.\n");
            exit(EXIT_FAILURE);
        }

        usleep(retry_interval * 1000); // 等待
        retry_interval = (retry_interval * 2 > 60000) ? 60000 : retry_interval * 2;
    }
}

// 資料上傳函數
void send_to_rabbitmq(amqp_connection_state_t conn, const char *json_data) {
    amqp_bytes_t body = {.len = strlen(json_data), .bytes = (void *)json_data};
    amqp_basic_publish(conn, 1, amqp_cstring_bytes(""), amqp_cstring_bytes(MQ_QUEUE), 0, 0, NULL, body);
    printf("Data sent to RabbitMQ: %s\n", json_data);
}

// 主程式
int main(void) {
    HANDLE hHS;
    const char *IPadd = "192.168.9.40";
    float fdataBuffer[BUFFERSIZE];
    float accumulatedData[SAMPLE_RATE];
    size_t accumulatedCount = 0;
    char json_data[1024];
    UL32 remChannel = 0;

    // 與 RabbitMQ 建立連線
    amqp_connection_state_t conn = establish_rabbitmq_connection(10);

    // 與設備建立連線
    char tmp[128] = {0};
    sprintf(tmp, "%s,9999,10010", IPadd);
    printf("Connecting to device...\n");
    hHS = HS_Device_Create(tmp);
    if (hHS == NULL) {
        fprintf(stderr, "Failed to connect to device.\n");
        return -1;
    }else{
        printf("Connected to device\n");
    }

    if (!HS_SetAIScanParam(hHS, 2, 0, 0, SAMPLE_RATE, 0, 0, 0)) {
        fprintf(stderr, "Failed to set scan parameters.\n");
        HS_Device_Release(hHS);
        return -1;
    }else{
        printf("Set scan parameters\n");
    }

    if (!HS_StartAIScan(hHS)) {
        fprintf(stderr, "Failed to start AI scan.\n");
        HS_Device_Release(hHS);
        return -1;
    }else{
        printf("Start Scan\n");
    }

    // 持續讀取與處理數據
    while (1) {
        WORD BufferStatus = 0;
        UL32 ulleng = 0;

        if (!HS_GetAIBufferStatus(hHS, &BufferStatus, &ulleng)) {
            fprintf(stderr, "Failed to get AI buffer status.\n");
            break;
        }

        if (ulleng) {
            UL32 size = (ulleng > BUFFERSIZE) ? BUFFERSIZE : ulleng;
            UL32 readsize = HS_GetAIBuffer(hHS, fdataBuffer, size);

            if (readsize) {
                for (I32 i = 0; i < readsize; i++) {
                    accumulatedData[accumulatedCount++] = fdataBuffer[i];

                    if (accumulatedCount >= SAMPLE_RATE) {
                        // 封裝數據為 JSON 格式
                        snprintf(json_data, sizeof(json_data), "{ \"data\": [");
                        for (size_t j = 0; j < accumulatedCount; j++) {
                            char buffer[32];
                            snprintf(buffer, sizeof(buffer), "%f", accumulatedData[j]);
                            strcat(json_data, buffer);
                            if (j < accumulatedCount - 1) {
                                strcat(json_data, ", ");
                            }
                        }
                        strcat(json_data, "] }");

                        // 上傳至 RabbitMQ
                        send_to_rabbitmq(conn, json_data);

                        // 重置累積數據
                        accumulatedCount = 0;
                    }
                }
            }
        }
    }

    // 停止掃描並釋放資源
    HS_StopAIScan(hHS);
    printf("Stop Scan\n");
    HS_Device_Release(hHS);
    printf("Device released\n");
    amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
    amqp_destroy_connection(conn);

    return 0;
}
