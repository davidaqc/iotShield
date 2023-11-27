#include "EspMQTTClient.h"
#include "authentication.h"

String iot_topic = "iot/intensity";
String iot_response = "iot/intensity-response";

EspMQTTClient client(
  SECRET_SSID,
  SECRET_PASS,
  BROKER_IP,         // MQTT Broker server ip
  BROKER_USERNAME,   // Can be omitted if not needed
  BROKER_PASSWORD,   // Can be omitted if not needed
  CLIENT_NAME,       // Client name that uniquely identifies your device
  BROKER_PORT        // The MQTT port, default to 1883. this line can be omitted
);

void onConnectionEstablished() {
  Serial.println("MQTT connection established!");
  // Subscribe to the MQTT topic during setup
  client.subscribe(iot_topic, [] (const String &payload)  {
    client.publish(iot_response, payload.c_str());
  });
}

void setup() {
  Serial.begin(9600);
  Serial.println("Starting");
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater();    // Enable the web updater. 
                                    // User and password default to values of MQTTUsername 
                                    // and MQTTPassword. These can be overridden with 
                                    // enableHTTPWebUpdater("user", "password").
  client.setMqttReconnectionAttemptDelay(2000);
  client.setKeepAlive(60);
}

void loop() {
  client.loop();
}
