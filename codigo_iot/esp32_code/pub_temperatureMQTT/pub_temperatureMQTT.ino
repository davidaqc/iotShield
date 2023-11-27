#include "EspMQTTClient.h"
#include "authentication.h"

String iot_topic = "iot/temperature";

EspMQTTClient client(
  SECRET_SSID,
  SECRET_PASS,
  BROKER_IP,         // MQTT Broker server ip
  BROKER_USERNAME,   // Can be omitted if not needed
  BROKER_PASSWORD,   // Can be omitted if not needed
  CLIENT_NAME,       // Client name that uniquely identify your device
  BROKER_PORT        // The MQTT port, default to 1883. this line can be omitted
);

unsigned long previousMillis = 0;
const long interval = 3000;  // Intervalo de 3 segundos

void setup() {
  Serial.begin(9600);
  Serial.println("Starting");
  client.enableDebuggingMessages();
  client.enableHTTPWebUpdater();
  client.setMqttReconnectionAttemptDelay(2000);
  client.setKeepAlive(60); //seconds
  randomSeed(12915);
}

void loop() {
  client.loop();
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
      onConnectionEstablished();  // Enviar datos al broker
  }
}

void onConnectionEstablished() {
  String msg = "Temperature: " + get_measure();
  client.publish(iot_topic, msg.c_str());
}

String get_measure() {
  int measure = random(15, 35);
  String msg = String(measure);
  Serial.println(msg);
  return msg;
}
