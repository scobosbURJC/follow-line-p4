#include <WiFi.h>
#include "WiFiClientSecure.h"
#include <ArduinoMqttClient.h>

#include "credentials.h"
#include "../serial_comms/comms.h"
#include "macros.h"
#include "config.h"

#define CONNECT_TO_EDUROAM true

const String TEAM_NAME("SS");
const String ID("15");

// +++++++++++++ FUNCTIONS DECLARATION ++++++++++++
void connect_wifi();
String select_msg_to_send(struct protocol_msg* serial_msg);

// +++++++++++++ GLOBAL VARIABLES ++++++++++++++++
WiFiClient wifi_client;

MqttClient mqtt_client(&wifi_client);

unsigned long start_time;

// +++++++++++++ FUNCTIONS DEFINITION ++++++++++++
void connect_wifi() {
  byte led_state = LOW;

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
    led_state = !led_state;

    delay(WIFI_STATUS_CHECK_DELAY);
    TRACE(".");
  }
  TRACE_LN("");
}

String select_msg_to_send(struct protocol_msg* msg) {
  String msg_str("{\"team_name\": \"" + TEAM_NAME + "\", \"id\": \"" + ID + "\", \"action\": \"");
  switch (msg->id) {
    case START_LAP:
      return msg_str + "START_LAP" + "\"" + "}";
      
    case END_LAP:
      return msg_str + "END_LAP" + "\"" + ", \"time\": " + (unsigned long) msg->arg + "}";

    case OBSTACLE_DETECTED:
      return msg_str + "OBSTACLE_DETECTED" + "\"" + ", \"distance\": " + (int) msg->arg + "}";

    case LINE_LOST:
      return msg_str + "LINE_LOST" + "\"" + "}";
    
    case PING:
      return msg_str + "PING" + "\""", \"time\": " + (unsigned long) msg->arg + "}";

    case INIT_LINE_SEARCH:
      return msg_str + "INIT_LINE_SEARCH" + "\"" + "}";

    case STOP_LINE_SEARCH:
      return msg_str + "STOP_LINE_SEARCH" + "\"" + "}";

    case LINE_FOUND:
      return msg_str + "LINE_FOUND" + "\"" + "}";

    case VISIBLE_LINE:
      return msg_str + "VISIBLE_LINE" + "\"" + ", \"time\": " + (float) msg->arg + "}";
  }
}


// +++++++++++++ MAIN PROGRAM ++++++++++++++

int counter = 0; // TODO quitar esto

void setup() {
  String mqtt_msg;
  struct protocol_msg start_msg, ack;

  Serial.begin(TRACE_BAUD_RATE);

  while(!Serial);

  TRACE_LN("Serial initialized");

  Serial2.begin(COMMS_BAUD_RATE, SERIAL_8N1, COMMS_RX, COMMS_TX);

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

  TRACE_LN("SUCCESSFULL CONNECTION!!!");

  Serial2.write(START_MSG);

  start_time = millis();

  // Wait for Arduino to confirm
  do {
    delay(ACK_READ_DELAY);

    ack = read_msg(&Serial2);
    if (ack.id != INVALID_MSG)
      Serial.println(ack.id);
  } while (ack.id != ACK);

  start_msg.id = START_LAP;

  mqtt_msg = select_msg_to_send(&start_msg);

  mqtt_client.beginMessage(TOPIC);
  mqtt_client.print(mqtt_msg);
  mqtt_client.endMessage();

  start_time = millis();
}

void loop() {
  struct protocol_msg msg;
  unsigned long time;

  String mqtt_msg;

  mqtt_client.poll();

  msg.id = INVALID_MSG;

  msg = read_msg(&Serial2);

  if (msg.id != INVALID_MSG) {
    Serial.println(msg.id);
    mqtt_msg = select_msg_to_send(&msg);

    TRACE("Sending message (");
    TRACE(mqtt_msg);
    TRACE(") to topic: ");
    TRACE_LN(TOPIC);

    TRACE_LN();

    // send message, the Print interface can be used to set the message contents
    mqtt_client.beginMessage(TOPIC);
    mqtt_client.print(mqtt_msg);
    mqtt_client.endMessage();

    TRACE_LN();
  }

  time = millis();

  if (time - start_time >= PING_TIME) {
    msg.id = PING;
    msg.arg = time;

    mqtt_msg = select_msg_to_send(&msg);
    mqtt_client.beginMessage(TOPIC);
    mqtt_client.print(mqtt_msg);
    mqtt_client.endMessage();

    start_time = time;
  }
}