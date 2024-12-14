#include <WiFi.h>
#include "WiFiClientSecure.h"
#include <ArduinoMqttClient.h>

#include "credentials.h"
#include "../serial_comms/comms.h"
#include "macros.h"
#include "config.h"

#define CONNECT_TO_EDUROAM false

// +++++++++++++ FUNCTIONS DECLARATION ++++++++++++
void connect_wifi();

// +++++++++++++ GLOBAL VARIABLES ++++++++++++++++
WiFiClient wifi_client;

MqttClient mqtt_client(&wifi_client);

//mqtt_publisher(&mqtt_client, TOPIC);

// +++++++++++++ FUNCTIONS DEFINITION ++++++++++++
void connect_wifi() {
  TRACE("Connecting to ");
  TRACE_LN(WLAN_SSID);
  WiFi.disconnect(true); // Ensuring the new connection

  delay(PRE_WIFI_BEGIN_DELAY);
  
  if (CONNECT_TO_EDUROAM) {
    WiFi.begin(WLAN_SSID, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD);
  } else {
    WiFi.begin(WLAN_SSID, WLAN_PASS);
  }

  delay(POST_WIFI_BEGIN_DELAY);

  while (WiFi.status() != WL_CONNECTED) {
    delay(WIFI_STATUS_CHECK_DELAY);
    TRACE(".");
  }
  TRACE_LN("");
}


// +++++++++++++ MAIN PROGRAM ++++++++++++++
void setup() {
  Serial.begin(TRACE_BAUD_RATE);
  
  while(!Serial);

  connect_wifi();

  TRACE_LN("WiFi connected");
  TRACE("IP address: ");
  TRACE_LN(WiFi.localIP());

  while (!mqtt_client.connect(MQTT_BROKER, MQTT_PORT)) {
    TRACE("MQTT connection failed! Error code = ");
    TRACE_LN(mqtt_client.connectError());
    TRACE_LN("Retrying to connect");

    delay(MQTT_CONNECT_RETRY_DELAY);
  }

  // TODO ENVIAR LA CONFIRMACIÓN, DEL WIFI Y MQTT CONECTADO, AL ARDUINO
  TRACE_LN("SUCCESSFULL CONNECTION!!!");
}

void loop() {
  // put your main code here, to run repeatedly:
  yield();

  mqtt_client.poll();

  TRACE("Sending message to topic: ");
  TRACE_LN(TOPIC);

  // send message, the Print interface can be used to set the message contents
  mqtt_client.beginMessage(TOPIC);
  mqtt_client.print("holaaaaaaa");
  mqtt_client.endMessage();

  TRACE_LN();

  delay(500);
}