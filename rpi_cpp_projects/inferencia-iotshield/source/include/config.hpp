#ifndef CONFIG_H
#define CONFIG_H

// WIFI Configuration
const char WIFI_SSID[] = "LIB-3374188";
const char WIFI_PASSWORD[] = "ekqm6uhDfdsp";

// MQTT Configuration
#define MQTT_PORT 1884
const char BROKER_IP[] = "192.168.50.169";
const char MQTT_USERNAME[] = "user1_david";
const char MQTT_PASSWORD[] = "pass1_david";
const char MQTT_CLIENT_ID[] = "device_1_client_id";
const int KEEP_ALIVE = 60;

// MQTT Topics
const char IA_TOPIC_CLASIFICATION[] = "ia/rpi_1/clasificacion";
const char IA_TOPIC_DATOS[] = "ia/rpi_1/datos";

// Detection Algoritm and Wait Time
const double UMBRAL = 0.05;
const int NUM_FEATURES = 4;
const int RECONNECT_DELAY = 5000;
const int WIFI_CONNECTION_TIMEOUT = 30000;

#endif
