// For more information about Inference on Raspberry Pi Pico 2040 via Edge Impulse, visit:
// https://www.youtube.com/watch?v=LmSrY_pZho0

#include <cmath>
#include "pico/stdlib.h"
#include "ei_run_classifier.h"
#include "mqtt_module.hpp"
#include "config.hpp"

// Variable definitions
MQTT_CLIENT_DATA_T *mqtt;
ip_addr_t addr;

// Function prototypes
int raw_feature_get_data(size_t offset, size_t length, float *out_ptr);
const char *detectar_anomalias(const float *dato_preprocesado, const ei_impulse_result_classification_t *reconstruccion);
void setupMQTT();
void performInferenceAndPublish();

int main()
{
    sleep_ms(3000);
    stdio_init_all();

    // Initialize and configure the Wi-Fi module
    cyw43_arch_init();
    cyw43_arch_enable_sta_mode();

    // Connect to the WiFI network - loop until connected
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, WIFI_CONNECTION_TIMEOUT) != ERR_OK)
    {
        printf("Failed to connect to Wi-Fi. Retrying...\n");
    }
    printf("Connected to Wi-Fi!\n");

    // Validate and convert BROKER_IP to an IP address.
    if (!ip4addr_aton(BROKER_IP, &addr))
    {
        printf("IP address error\n");
        return -1;
    }

    // MQTT setup
    printf("Inicio del programa mqtt\n");
    setupMQTT();

    while (true)
    {

        // Verify MQTT client connection status
        while (!mqtt_client_is_connected(mqtt->mqtt_client_inst))
        {
            printf("Disconnected from MQTT server, reconnecting...\n");
            // Verify Wi-Fi connection status and reconnect if necessary
            while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, WIFI_CONNECTION_TIMEOUT) != ERR_OK)
            {
                printf("Failed to connect to Wi-Fi. Retrying...\n");
            }
            printf("Connected to Wi-Fi!\n");

            mqtt_client_connect(mqtt->mqtt_client_inst, &addr, MQTT_PORT, &mqtt_connection_cb, mqtt, &mqtt->mqtt_client_info);
            sleep_ms(RECONNECT_DELAY);
        }

        // Si hay un nuevo dato MQTT ejecuta la inferencia
        if (nuevo_dato_mqtt)
        {
            performInferenceAndPublish();
            nuevo_dato_mqtt = false;
        }
    }

    return 0;
}

// Función para extraer un segmento de datos y copiarlo en un arreglo
int raw_feature_get_data(size_t offset, size_t length, float *out_ptr)
{
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
}

// Función para detectar anomalias
const char *detectar_anomalias(const float *dato_preprocesado, const ei_impulse_result_classification_t *reconstruccion)
{

    /*if (dato_preprocesado.size() != reconstruccion.size())
    {
        return "Error: Las dimensiones de dato_preprocesado y reconstruccion no coinciden.";
    }*/

    // Cálculo del error cuadrático medio
    double error = 0.0;
    for (int i = 0; i < NUM_FEATURES; i++)
    {
        error += std::pow(reconstruccion[i].value - dato_preprocesado[i], 2);
    }
    error /= NUM_FEATURES;
    printf("error: %f\n", error);

    // Comparación con el umbral
    if (error > UMBRAL)
    {
        return "ataque";
    }
    else
    {
        return "legitimate";
    }
}

// Function for MQTT setup
void setupMQTT()
{
    mqtt = (MQTT_CLIENT_DATA_T *)calloc(1, sizeof(MQTT_CLIENT_DATA_T));

    if (!mqtt)
    {
        printf("MQTT client instantiation error\n");
        return;
    }
    mqtt->playing = false;
    mqtt->newTopic = false;
    mqtt->mqtt_client_info = mqtt_client_info;

    mqtt->mqtt_client_inst = mqtt_client_new();
    mqtt_set_inpub_callback(mqtt->mqtt_client_inst, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, mqtt);

    mqtt_client_connect(mqtt->mqtt_client_inst, &addr, MQTT_PORT, &mqtt_connection_cb, mqtt, &mqtt->mqtt_client_info);
}

// Funcion para ejecutar la inferencia y publicar los resultados
void performInferenceAndPublish()
{

    /*if (sizeof(features) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE)
    {
        ei_printf("The size of your 'features' array is not correct. Expected %lu items, but had %lu\n",
                  EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(features) / sizeof(float));
    }*/

    ei_impulse_result_t result = {0};

    // the features are stored into flash, and we don't want to load everything into RAM
    signal_t features_signal;
    features_signal.total_length = sizeof(features) / sizeof(features[0]);
    features_signal.get_data = &raw_feature_get_data;

    ei_printf("Edge Impulse standalone inferencing (Raspberry Pico 2040)\n");
    // invoke the impulse
    EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false /* debug */);
    ei_printf("run_classifier returned: %d\n", res);

    if (res != 0)
    {
        printf("Error running classifier: %d\n", res);
        return;
    }

    // print the predictions
    ei_printf("Predictions ");
    ei_printf("(Classification: %d ms.,)",
              result.timing.classification);
    ei_printf(": \n");
    ei_printf("[");
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++)
    {
        ei_printf("%.6f", result.classification[ix].value);
        if (ix != EI_CLASSIFIER_LABEL_COUNT - 1)
        {
            ei_printf(", ");
        }
    }
    ei_printf("]\n");

    // Evaluacion
    const char *resultado = detectar_anomalias(features, result.classification);
    printf("Resultado: %s\n\n", resultado);

    // Publicar resultado
    example_publish(mqtt->mqtt_client_inst, resultado, mqtt);
}
