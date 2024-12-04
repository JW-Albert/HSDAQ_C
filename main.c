#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <amqp.h>
#include <amqp_tcp_socket.h>
#include "hsdaql.h"

#define SAMPLERATE 1000 // 每秒數據量
#define MQ_HOST "192.168.109.239"
#define MQ_USER "admin"
#define MQ_PASS "admin"
#define MQ_QUEUE "sensor_data"
#define CHANNELS 2

typedef struct {
    float data[CHANNELS];
    long timestamp;
} SensorData;

typedef struct {
    SensorData buffer[SAMPLERATE];
    int count;
} SensorBuffer;

void error_exit(const char *message) {
    fprintf(stderr, "Error: %s\n", message);
    exit(EXIT_FAILURE);
}

amqp_connection_state_t connect_rabbitmq(const char *host, const char *user, const char *pass) {
    amqp_connection_state_t conn = amqp_new_connection();
    amqp_socket_t *socket = amqp_tcp_socket_new(conn);
    if (socket == NULL) error_exit("Failed to create RabbitMQ socket");

    if (amqp_socket_open(socket, host, 5672)) error_exit("Failed to open RabbitMQ socket");

    if (amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, user, pass).reply_type != AMQP_RESPONSE_NORMAL)
        error_exit("Failed to login to RabbitMQ");

    amqp_channel_open(conn, 1);
    if (amqp_get_rpc_reply(conn).reply_type != AMQP_RESPONSE_NORMAL)
        error_exit("Failed to open RabbitMQ channel");

    return conn;
}

void publish_to_rabbitmq(amqp_connection_state_t conn, const char *queue, SensorBuffer *buffer) {
    char message[10240] = "[";

    for (int i = 0; i < buffer->count; i++) {
        char entry[256];
        snprintf(entry, sizeof(entry), "{\"timestamp\": %ld, \"channel_1\": %.2f, \"channel_2\": %.2f}",
                 buffer->buffer[i].timestamp, buffer->buffer[i].data[0], buffer->buffer[i].data[1]);
        strcat(message, entry);
        if (i < buffer->count - 1) strcat(message, ",");
    }
    strcat(message, "]");

    amqp_bytes_t message_bytes = amqp_cstring_bytes(message);
    amqp_bytes_t queue_bytes = amqp_cstring_bytes(queue);

    amqp_basic_publish(conn, 1, amqp_empty_bytes, queue_bytes, 0, 0, NULL, message_bytes);
    printf("Published %d samples to RabbitMQ\n", buffer->count);

    buffer->count = 0; // 清空緩衝區
}

int main() {
    // RabbitMQ 連線
    amqp_connection_state_t mq_conn = connect_rabbitmq(MQ_HOST, MQ_USER, MQ_PASS);
    printf("Connected to RabbitMQ\n");

    // 設備連接
    char device_ip[] = "192.168.9.40";
    HANDLE device_handle = HS_Device_Create(device_ip);
    if (device_handle == false) error_exit("Failed to connect to the device");

    printf("Connected to device at %s\n", device_ip);

    // 設置掃描參數
    if (!HS_SetAIScanParam(device_handle, CHANNELS, 0, 0, SAMPLERATE, 0, 0, 0))
        error_exit("Failed to set scan parameters");

    if (!HS_ClearAIBuffer(device_handle)) error_exit("Failed to clear buffer");

    if (!HS_StartAIScan(device_handle)) error_exit("Failed to start AI scan");

    printf("Started AI scan\n");

    SensorBuffer buffer = {.count = 0};
    bool running = true;

    while (running) {
        unsigned short buffer_status;
        unsigned int data_count;

        if (!HS_GetAIBufferStatus(device_handle, &buffer_status, &data_count)) {
            fprintf(stderr, "Failed to get buffer status\n");
            break;
        }

        if (data_count >= CHANNELS) {
            float data[CHANNELS];
            unsigned int read_count = HS_GetAIBuffer(device_handle, data, CHANNELS);

            if (read_count > 0) {
                for (unsigned int i = 0; i < read_count; i += CHANNELS) {
                    SensorData sample;
                    sample.timestamp = time(NULL);
                    sample.data[0] = data[0];
                    sample.data[1] = data[1];
                    buffer.buffer[buffer.count++] = sample;

                    if (buffer.count >= SAMPLERATE) {
                        publish_to_rabbitmq(mq_conn, MQ_QUEUE, &buffer);
                    }
                }
            } else {
                fprintf(stderr, "No data read from buffer\n");
            }
        }

        if (buffer_status & 0x02 || buffer_status & 0x04 || buffer_status & 0x08) {
            fprintf(stderr, "Buffer error detected\n");
            break;
        }
    }

    // 清理資源
    HS_StopAIScan(device_handle);
    HS_Device_Release(device_handle);
    amqp_channel_close(mq_conn, 1, AMQP_REPLY_SUCCESS);
    amqp_connection_close(mq_conn, AMQP_REPLY_SUCCESS);
    amqp_destroy_connection(mq_conn);

    printf("Stopped AI scan and closed RabbitMQ connection\n");
    return 0;
}
