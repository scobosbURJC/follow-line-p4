#define TRACE_BAUD_RATE 115200

#define PING_TIME 4000 // Unit: ms

#define RESEND_INIT_TIME 500

// ---------- MQTT -----------
#define MQTT_BROKER "teachinghub.eif.urjc.es"
#define MQTT_PORT 21883

#define TOPIC "/SETR/2024/15/"

#define MAX_MQTT_CONNECT_RETRIES 3

// ----------- DELAYS ------------
#define PRE_WIFI_BEGIN_DELAY 1000
#define POST_WIFI_BEGIN_DELAY 1000
#define WIFI_STATUS_CHECK_DELAY 500

#define MQTT_CONNECT_RETRY_DELAY 5000

#define ACK_READ_DELAY 2000

// ---------- SERIAL ----------------
#define COMMS_RX 33
#define COMMS_TX 4

#define UART_PORT 2

#define START_MSG "#0:0.00/"