#include <WiFi.h>
#include "WiFiClientSecure.h"
#include "Adafruit_MQTT.h"
//#include "Adafruit_MQTT_Client.h"

#include "credentials.h"
#include "../serial_comms/comms.h"
#include "macros.h"
#include "config.h"

#define CONNECT_TO_EDUROAM false

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
  Serial.begin(BAUD_RATE); // De momemto para las trazas
  delay(10);

  connect_wifi();

  TRACE_LN("WiFi connected");
  TRACE("IP address: ");
  TRACE_LN(WiFi.localIP());

  // TODO ENVIAR LA CONFIRMACIÓN, DEL WIFI CONECTADO, AL ARDUINO

}

void loop() {
  // put your main code here, to run repeatedly:
  yield();

  
}
