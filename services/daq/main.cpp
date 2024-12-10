#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "windows2linux.h"
#include <termios.h> //#include <conio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <amqp.h>
#include <amqp_tcp_socket.h>
#include "./hsdaql.h"

#define MQ_HOST "172.18.0.2"
#define MQ_USER "user"
#define MQ_PASS "password"
#define MQ_QUEUE "data_queue"
#define SAMPLE_RATE 12800
#define BUFFERSIZE 1000

void get_formatted_time(char *buffer, size_t buffer_size) {
    time_t now = time(NULL);             // 獲取當前時間
    struct tm *tm_info = localtime(&now); // 將時間轉換為本地時間

    // 格式化時間為 YY_MM_DD_HH_mm
    strftime(buffer, buffer_size, "%y_%m_%d_%H_%M_%S", tm_info);
}

// RabbitMQ 連線函數
amqp_connection_state_t establish_rabbitmq_connection() {
    const int retry_interval = 1000; // 重試間隔（毫秒）
    amqp_connection_state_t conn;

    while (1) {
        conn = amqp_new_connection();
        amqp_socket_t *socket = amqp_tcp_socket_new(conn);
        if (!socket) {
            fprintf(stderr, "Failed to create RabbitMQ socket.\n");
            usleep(retry_interval * 1000);
            continue;
        }

        int status = amqp_socket_open(socket, MQ_HOST, 5672);
        if (status != AMQP_STATUS_OK) {
            fprintf(stderr, "Failed to open RabbitMQ connection. Retrying...\n");
            amqp_destroy_connection(conn);
            usleep(retry_interval * 1000);
            continue;
        }

        amqp_rpc_reply_t login_reply = amqp_login(
            conn, "/", 0, 131072, 70, AMQP_SASL_METHOD_PLAIN, MQ_USER, MQ_PASS
        );
        if (login_reply.reply_type != AMQP_RESPONSE_NORMAL) {
            fprintf(stderr, "Failed to login to RabbitMQ. Retrying...\n");
            amqp_destroy_connection(conn);
            usleep(retry_interval * 1000);
            continue;
        }

        amqp_channel_open(conn, 1);
        amqp_rpc_reply_t channel_reply = amqp_get_rpc_reply(conn);
        if (channel_reply.reply_type != AMQP_RESPONSE_NORMAL) {
            fprintf(stderr, "Failed to open channel. Retrying...\n");
            amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
            amqp_destroy_connection(conn);
            usleep(retry_interval * 1000);
            continue;
        }

        fprintf(stderr, "RabbitMQ connection established.\n");
        return conn;
    }

    fprintf(stderr, "Exceeded maximum retries. Exiting...\n");
    exit(EXIT_FAILURE);
}

int is_rabbitmq_connected(amqp_connection_state_t conn) {
    int status = amqp_basic_publish(
        conn, 1, amqp_cstring_bytes(""), amqp_empty_bytes, 0, 0, NULL, amqp_empty_bytes
    );

    if (status != AMQP_STATUS_OK) {
        fprintf(stderr, "RabbitMQ connection lost. Status: %d\n", status);
        return 0; // 連線中斷
    }

    return 1; // 連線正常
}

// 封裝數據並上傳至 RabbitMQ
void send_to_rabbitmq(amqp_connection_state_t conn, const float *data ,int size_of_data, size_t count, int chCnt) {
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
    char time_str[20]; // 儲存時間字串的緩衝區
    get_formatted_time(time_str, sizeof(time_str));
    fprintf(stderr, "%d's of data sent to RabbitMQ at %s\n", size_of_data/4 ,time_str);
}

I32 main(void) {
    HANDLE hHS;
    const char *IPadd = "192.168.9.40";
    float fdataBuffer[BUFFERSIZE];
    UL32 remChannel = 0;
    size_t accumulatedCount = 0;
    float accumulatedData[SAMPLE_RATE * 60];

    // RabbitMQ 初始化
    amqp_connection_state_t conn = establish_rabbitmq_connection();

    // PET-AR400 初始化
    char tmp[128] = {0};
    sprintf(tmp, "%s,9999,10010", IPadd);
    fprintf(stderr, "Connecting to device...\n");
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

    if(!HS_ClearAIBuffer(hHS)) {
        fprintf(stderr, "Failed to clear AI buffer.\n");
        HS_Device_Release(hHS);
        return -1;
    }

    fprintf(stderr, "Start Scan\n");

    char time_str[20]; // 儲存時間字串的緩衝區

    // 主循環
    int count = 0;
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

                    if (accumulatedCount >= SAMPLE_RATE * 60) {
                        /*if (!is_rabbitmq_connected(conn)) {
                            fprintf(stderr, "RabbitMQ connection lost. Attempting to reconnect...\n");
                            amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
                            amqp_destroy_connection(conn);
                            conn = establish_rabbitmq_connection(); // 重新建立連線
                            if (conn == NULL) {
                                fprintf(stderr, "Failed to reconnect to RabbitMQ. Exiting...\n");
                                break;
                            }
                        }*/
                        count ++;
                        get_formatted_time(time_str, sizeof(time_str)); // 呼叫函數取得時間字串
                        fprintf(stderr, "The %d is Sending data to RabbitMQ at %s\n", count, time_str);
                        send_to_rabbitmq(conn, accumulatedData ,sizeof(accumulatedData), accumulatedCount, 2);
                        accumulatedCount = 0; // 清空累積數據
                    }
                }
            }
        }
    }

    // 停止掃描並釋放資源
    HS_StopAIScan(hHS);
    fprintf(stderr, "Scan stopped\n");
    HS_Device_Release(hHS);
    fprintf(stderr, "Device released\n");
    amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
    amqp_destroy_connection(conn);

    return 0;
}
