// For more information about MQTT in lwIP, visit:
// https://www.nongnu.org/lwip/2_1_x/group__mqtt.html

#include "mqtt_module.hpp"
#include "config.hpp"

// Variable definitions
float features[NUM_FEATURES];
bool nuevo_dato_mqtt = false;
struct mqtt_connect_client_info_t mqtt_client_info =
    {
        MQTT_CLIENT_ID, /* clied_id */
        MQTT_USERNAME,  /* user */
        MQTT_PASSWORD,  /* pass */
        KEEP_ALIVE,     /* keep alive */
        NULL,           /* will_topic */
        NULL,           /* will_msg */
        0,              /* will_qos */
        0               /* will_retain */
#if LWIP_ALTCP && LWIP_ALTCP_TLS
        ,
        NULL
#endif
};

/* Called when publish is complete either with sucess or failure */
static void mqtt_pub_request_cb(void *arg, err_t result)
{
    if (result != ERR_OK)
    {
        printf("Publish result: %d\n", result);
    }
}

// Publishes a message to an MQTT broker.
err_t example_publish(mqtt_client_t *client, const char *pub_payload, void *arg)
{
    // const char *pub_payload= "PubSubHubLubJub";
    err_t err;
    u8_t qos = 0;    /* 0 1 or 2, see MQTT specification */
    u8_t retain = 0; /* No don't retain such crappy payload... */
    cyw43_arch_lwip_begin();
    err = mqtt_publish(client, IA_TOPIC_CLASIFICATION, pub_payload, strlen(pub_payload), qos, retain, mqtt_pub_request_cb, arg);
    cyw43_arch_lwip_end();
    if (err != ERR_OK)
    {
        printf("Publish err: %d\n", err);
    }
    return err;
}

// Callback function for handling incoming data from an MQTT server.
void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    // printf("mqtt_incoming_data_cb \n");
    MQTT_CLIENT_DATA_T *mqtt_client = (MQTT_CLIENT_DATA_T *)arg;
    LWIP_UNUSED_ARG(data);

    // strncpy(mqtt_client->data, data, len);
    strncpy((char *)mqtt_client->data, (char *)data, len);

    mqtt_client->len = len;
    mqtt_client->data[len] = '\0';

    mqtt_client->newTopic = true;
    mqtt->playing = false;

    printf("Received data: %.*s\n", len, (char *)data);

    // Dividir los datos por comas y almacenarlos en el arreglo features
    char *token = strtok((char *)data, ",");
    int i = 0;
    while (token != NULL && i < NUM_FEATURES)
    {
        features[i] = atof(token);
        // printf("Token[%d]: %f\n", i, features[i]);
        token = strtok(NULL, ",");
        i++;
    }

    nuevo_dato_mqtt = true;
}

// Callback function for handling MQTT connection status changes.
void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
    MQTT_CLIENT_DATA_T *mqtt_client = (MQTT_CLIENT_DATA_T *)arg;
    LWIP_UNUSED_ARG(client);

    LWIP_PLATFORM_DIAG(("MQTT client \"%s\" connection cb: status %d\n", mqtt_client->mqtt_client_info.client_id, (int)status));

    if (status == MQTT_CONNECT_ACCEPTED)
    {
        printf("MQTT_CONNECT_ACCEPTED\n");

        // example_publish(client, arg);
        // mqtt_disconnect(client);

        mqtt_sub_unsub(client,
                       IA_TOPIC_DATOS, 1,
                       mqtt_request_cb, arg,
                       1);
        /*mqtt_sub_unsub(client,
                       "stop", 0,
                       mqtt_request_cb, arg,
                       1);*/
    }
}

void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
    // printf("mqtt_incoming_publish_cb \n");
    MQTT_CLIENT_DATA_T *mqtt_client = (MQTT_CLIENT_DATA_T *)arg;
    // strcpy(mqtt_client->topic, topic);
    strcpy((char *)mqtt_client->topic, (char *)topic);
    // printf("Received topic: %s\n", mqtt_client->topic);
}

void mqtt_request_cb(void *arg, err_t err)
{
    MQTT_CLIENT_DATA_T *mqtt_client = (MQTT_CLIENT_DATA_T *)arg;

    LWIP_PLATFORM_DIAG(("MQTT client \"%s\" request cb: err %d\n", mqtt_client->mqtt_client_info.client_id, (int)err));
}
