#ifndef MQTT_MODULE_H
#define MQTT_MODULE_H

#include "lwip/apps/mqtt_priv.h"
#include "pico/cyw43_arch.h"

typedef struct MQTT_CLIENT_DATA_T
{
    mqtt_client_t *mqtt_client_inst;
    struct mqtt_connect_client_info_t mqtt_client_info;
    uint8_t data[MQTT_OUTPUT_RINGBUF_SIZE];
    uint8_t topic[100];
    uint32_t len;
    bool playing;
    bool newTopic;
} MQTT_CLIENT_DATA_T;

// Global variables shared between main and mqtt_module (Declarations)
extern float features[4];
extern bool nuevo_dato_mqtt;
extern MQTT_CLIENT_DATA_T *mqtt;
extern struct mqtt_connect_client_info_t mqtt_client_info;

err_t example_publish(mqtt_client_t *client, const char *pub_payload, void *arg);
void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
void mqtt_request_cb(void *arg, err_t err);
void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);

#endif
