#include <Arduino_FreeRTOS.h>
#include "FastLED.h"

#define TRIG_DELAY_US 10
#define TRIG_PIN 13  
#define ECHO_PIN 12 

#define PIN_RBGLED 4
#define NUM_LEDS 1

#define PERIOD_US_TASK 200
#define PERIOD_BLINK_TASK 1000

CRGB leds[NUM_LEDS];
volatile byte led_state;

// define two tasks for Blink & AnalogRead
void TaskBlink( void *pvParameters );
void TaskUsRead( void *pvParameters );

// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  FastLED.addLeds<NEOPIXEL, PIN_RBGLED>(leds, NUM_LEDS);
  FastLED.setBrightness(20);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  led_state = LOW;

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(TRIG_PIN, LOW);

  // Now set up two tasks to run independently.
  xTaskCreate(
    TaskBlink
    ,  "Blink"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    TaskUsRead
    ,  "UsRead"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*------------- Sensors and actuators --------------*/
/*--------------------------------------------------*/

uint32_t Color(uint8_t r, uint8_t g, uint8_t b)
{
  return (((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskBlink(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  TickType_t xLastWakeTime;
  unsigned long last, now;

  last = 0;

  while (1)
  {
    xLastWakeTime = xTaskGetTickCount();

    now = millis();
    Serial.print("blink: ");
    Serial.println(now - last);
    last = now;

    xTaskDelayUntil(&xLastWakeTime, PERIOD_BLINK_TASK / portTICK_PERIOD_MS);
  }
}

void TaskUsRead(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  TickType_t xLastWakeTime;
  float dist;
  unsigned long last, now;

  while (true)
  {
    xLastWakeTime = xTaskGetTickCount();

    now = millis();
    Serial.print("us: ");
    Serial.println(now - last);
    last = now;

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(TRIG_DELAY_US);
    digitalWrite(TRIG_PIN, LOW);
    
    dist = (pulseIn(ECHO_PIN, HIGH) / 2) * 0.0343;

    //Serial.println(dist);
    xTaskDelayUntil(&xLastWakeTime, PERIOD_US_TASK / portTICK_PERIOD_MS);  // one tick delay (15ms) in between reads for stability
  }
}