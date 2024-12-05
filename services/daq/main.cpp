#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "windows2linux.h"
#include <termios.h> //#include <conio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
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

    for (int attempt = 0; attempt < retries; ++attempt) {
        conn = amqp_new_connection();
        amqp_socket_t *socket = amqp_tcp_socket_new(conn);
        if (!socket) {
            fprintf(stderr, "Failed to create RabbitMQ socket.\n");
            exit(EXIT_FAILURE);
        }

        int status = amqp_socket_open(socket, MQ_HOST, 5672);
        if (status != true) {
            fprintf(stderr, "Failed to open RabbitMQ connection to %s:%d. Error: %s\n",
                    MQ_HOST, 5672, amqp_error_string2(status));
            amqp_destroy_connection(conn);
            usleep(retry_interval * 1000);
            retry_interval = (retry_interval * 2 > 60000) ? 60000 : retry_interval * 2;
            continue; // 跳過本次迴圈嘗試下一次連線
        }

        amqp_rpc_reply_t login_reply = amqp_login(
            conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, MQ_USER, MQ_PASS
        );
        if (login_reply.reply_type != AMQP_RESPONSE_NORMAL) {
            fprintf(stderr, "Failed to login to RabbitMQ. Error: %d\n", login_reply.reply_type);
            amqp_destroy_connection(conn);
            usleep(retry_interval * 1000);
            retry_interval = (retry_interval * 2 > 60000) ? 60000 : retry_interval * 2;
            continue; // 跳過本次迴圈嘗試下一次連線
        }

        // 聲明通道
        amqp_channel_open(conn, 1);
        amqp_rpc_reply_t channel_reply = amqp_get_rpc_reply(conn);
        if (channel_reply.reply_type != AMQP_RESPONSE_NORMAL) {
            fprintf(stderr, "Failed to open channel. Error: %d\n", channel_reply.reply_type);
            amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
            amqp_destroy_connection(conn);
            usleep(retry_interval * 1000);
            retry_interval = (retry_interval * 2 > 60000) ? 60000 : retry_interval * 2;
            continue; // 跳過本次迴圈嘗試下一次連線
        }

        printf("RabbitMQ connection established with channel.\n");
        return conn; // 成功連線，返回連線對象
    }

    fprintf(stderr, "Exceeded maximum retries. Failed to connect to RabbitMQ.\n");
    exit(EXIT_FAILURE); // 超出重試次數後退出
}


// 封裝數據並上傳至 RabbitMQ
void send_to_rabbitmq(amqp_connection_state_t conn, const float *data, size_t count, int chCnt) {
    char json_data[1024] = "{ \"data\": [";
    size_t json_len = strlen(json_data);

    for (size_t i = 0; i < count; i++) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "{\"Channel\": %d, \"Value\": %f}", (int)(i % chCnt) + 1, data[i]);
        strncat(json_data, buffer, sizeof(json_data) - json_len - 1);
        json_len = strlen(json_data);

        if (i < count - 1) {
            strncat(json_data, ", ", sizeof(json_data) - json_len - 1);
            json_len = strlen(json_data);
        }
    }
    strncat(json_data, "] }", sizeof(json_data) - json_len - 1);

    // 上傳數據
    amqp_bytes_t body = {.len = strlen(json_data), .bytes = (void *)json_data};
    amqp_basic_publish(conn, 1, amqp_cstring_bytes(""), amqp_cstring_bytes(MQ_QUEUE), 0, 0, NULL, body);
    printf("Data sent to RabbitMQ: %s\n", json_data);
}

I32 main(void) {
    HANDLE hHS;
    const char *IPadd = "192.168.9.40";
    float fdataBuffer[BUFFERSIZE];
    UL32 remChannel = 0;
    size_t accumulatedCount = 0;
    float accumulatedData[SAMPLE_RATE];

    // RabbitMQ 初始化
    amqp_connection_state_t conn = establish_rabbitmq_connection(10);

    // PET-AR400 初始化
    char tmp[128] = {0};
    sprintf(tmp, "%s,9999,10010", IPadd);
    printf("Connecting to device...\n");
    hHS = HS_Device_Create(tmp);
    if (hHS == false) {
        fprintf(stderr, "Failed to connect to device.\n");
        return -1;
    }

    if (!HS_SetAIScanParam(hHS, 2, 0, 0, SAMPLE_RATE, 0, 0, 0)) {
        fprintf(stderr, "Failed to set scan parameters.\n");
        HS_Device_Release(hHS);
        return -1;
    }

    if (!HS_StartAIScan(hHS)) {
        fprintf(stderr, "Failed to start AI scan.\n");
        HS_Device_Release(hHS);
        return -1;
    }

    printf("Start Scan\n");

    // 主循環
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
                        send_to_rabbitmq(conn, accumulatedData, accumulatedCount, 2);
                        accumulatedCount = 0; // 清空累積數據
                    }
                }
            }
        }
    }

    // 停止掃描並釋放資源
    HS_StopAIScan(hHS);
    printf("Scan stopped\n");
    HS_Device_Release(hHS);
    printf("Device released\n");
    amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
    amqp_destroy_connection(conn);

    return 0;
}
