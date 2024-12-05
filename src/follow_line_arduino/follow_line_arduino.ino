#include <Arduino_FreeRTOS.h>
#include "FastLED.h"

#include "config.h"

const int rgb_red[] = RED;
const int rgb_green[] = GREEN;

void obstacleTask(void *params);
void followLineTask(void *params);

CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(9600);
  
  while(!Serial); // Wait until Serial port is intialized

  FastLED.addLeds<NEOPIXEL, PIN_RBGLED>(leds, NUM_LEDS);
  FastLED.setBrightness(20);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:

}

void followLineTask(void *params) {
  (void) params;


}

void obstacleRead(void *params) {
  (void) params;

  TickType_t xLastWakeTime;
  float dist;
  unsigned long last, now;

  while (true) {
    xLastWakeTime = xTaskGetTickCount();

    now = millis();
    Serial.print("us: ");
    Serial.println(now - last);
    last = now;

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(TRIG_DELAY);
    digitalWrite(TRIG_PIN, LOW);
    
    dist = (pulseIn(ECHO_PIN, HIGH) / 2) * SOUND_SPEED; // Unit: cm

    Serial.print("dist: ");
    Serial.println(dist);
    xTaskDelayUntil(&xLastWakeTime, PERIOD_US_TASK / portTICK_PERIOD_MS);
  }
}