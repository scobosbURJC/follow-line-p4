#include <string.h>

#include <Arduino_FreeRTOS.h>
#include "FastLED.h"

#include "config.h"

// +++++ CONSTANTS +++++
#define SEC_TO_MS 1000.0

// +++++ FUNCTONS DEFINITION +++++
void obstacle_task(void *params);
void follow_line_task(void *params);

// +++++ VARIABLES +++++
const int RGB_RED[RGB_COMPONENTS] = RED;
const int RGB_GREEN[RGB_COMPONENTS] = GREEN;

const int IR_SENSORS[N_IR_SENSORS] = IR_SENSORS_LIST;

CRGB leds[NUM_LEDS];

TaskHandle_t follow_line_task_handle; // TODO mencionarlo en el BLOG

// +++++ MAIN PROGRAM +++++
void setup() {
  Serial.begin(9600);
  
  while(!Serial); // Wait until Serial port is intialized

  // Motors
  pinMode(PIN_Motor_STBY, OUTPUT);
  
  pinMode(PIN_Motor_AIN_1, OUTPUT);
  pinMode(PIN_Motor_PWMA, OUTPUT);

  pinMode(PIN_Motor_BIN_1, OUTPUT);
  pinMode(PIN_Motor_PWMB, OUTPUT);

  analogWrite(PIN_Motor_PWMA, MOTOR_STOP);
  analogWrite(PIN_Motor_PWMB, MOTOR_STOP);

  digitalWrite(PIN_Motor_STBY, HIGH); // TODO (a lo mejor) deshabilitarlos hasta que se sincronice la ESP32

  digitalWrite(PIN_Motor_AIN_1, HIGH);
  digitalWrite(PIN_Motor_BIN_1, HIGH);

  // Neopixel led
  FastLED.addLeds<NEOPIXEL, PIN_RBGLED>(leds, NUM_LEDS);
  FastLED.setBrightness(0);

  // Ultrasonic sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Tasks initialization
  xTaskCreate(
    follow_line_task,
    "follow_line_task",
    100,
    NULL,
    FOLLOW_LINE_PRIO,
    &follow_line_task_handle
  );

  xTaskCreate(
    obstacle_task,
    "obstacle_task",
    100,
    NULL,
    OBSTACLE_PRIO,
    NULL
  );
}

void loop() {
  // put your main code here, to run repeatedly:

}

// +++++ FUNCITONS DECLARATION +++++

void follow_line_task(void *params) {
  (void) params;

  TickType_t x_last_wake_time;
  float kr = K_NULL, kl = K_NULL, vr, vl;
  unsigned long time_lost_start = 0, time_lost = 0;

  String line_state = NO_LINE;

  while (true) {
    x_last_wake_time = xTaskGetTickCount();

    Serial.println("---------");

    for (int i = 0; i < N_IR_SENSORS; i++) {
      //Serial.println(analogRead(IR_SENSORS[i]));

      line_state[i] =
        analogRead(IR_SENSORS[i]) >= IR_BLACK_THRESHOLD ? '1' : '0';  // !!!!! SERGIO !!!!! 1 = la línea está ahí (la linea está justo debajo del igual por el estilo)
    }

    Serial.println(line_state);

    if (line_state == LEFT) {
      kr = K_HIGH;
      kl = K_NULL;

      if (time_lost == 0) {
        time_lost_start = millis();
      }

      time_lost = millis() - time_lost_start;

    } else if (line_state == RIGHT) {
      kr = K_NULL;
      kl = K_HIGH;

      if (time_lost == 0) {
        time_lost_start = millis();
      }

      time_lost = millis() - time_lost_start;

    } else if (line_state == CENTER_LEFT) {
      kr = K_LOW;
      kl = K_NULL;

      if (time_lost == 0) {
        time_lost_start = millis();
      }

      time_lost = millis() - time_lost_start;

    } else if (line_state == CENTER_RIGHT) {
      kr = K_NULL;
      kl = K_LOW;

      if (time_lost == 0) {
        time_lost = millis();
      }

      time_lost = millis() - time_lost_start;

    } else if (line_state == CENTER_ONLY || line_state == CENTER_ALL) {
      kr = K_NULL;
      kl = K_NULL;

      time_lost = 0;

    } else {
      time_lost = millis() - time_lost_start;
    }

    // !!!!! SERGIO !!!!! Quizás hay que poner algo de que la velocidad no vuelva a ser directamente V_REF segun vuelve a detectar que va por el medio

    vr = V_REF - (kr * (time_lost / SEC_TO_MS)); // !!!!! SERGIO !!!!! Para que no sea tan basto lo hacemos con los segundos y no con los milisegundos
    vl = V_REF - (kl * (time_lost / SEC_TO_MS));

    if (vr < V_MIN) {
      vr = V_MIN;
    }

    if (vl < V_MIN) {
      vl = V_MIN;
    }
    
    //analogWrite(PIN_Motor_PWMA, vr); // NO PROBADO BIEN (pero parece estar bien por las trazas)
    //analogWrite(PIN_Motor_PWMB, vl);

    Serial.println(vr);
    Serial.println(vl);

    xTaskDelayUntil(&x_last_wake_time, FOLLOW_LINE_TASK_PERIOD / portTICK_PERIOD_MS);
  }
}

void obstacle_task(void *params) {
  (void) params;

  TickType_t x_last_wake_time;
  float dist;
  bool is_finished = false;

  while (!is_finished) {
    x_last_wake_time = xTaskGetTickCount();

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(TRIG_DELAY);
    digitalWrite(TRIG_PIN, LOW);
    
    dist = (pulseIn(ECHO_PIN, HIGH) / 2) * SOUND_SPEED; // Unit: cm

    if (dist <= OBSTACLE_DIST_LIMIT) {
      //vTaskSuspend(follow_line_task_handle);               // !!!!! SERGIO !!!!! Lo he comentado ahora para probar con trazas lo de los IR

      analogWrite(PIN_Motor_PWMA, MOTOR_STOP);
      analogWrite(PIN_Motor_PWMB, MOTOR_STOP);

      is_finished = true; // TODO (quitar comentario) EN CASO DE QUE EL PROGRAMA TENGA QUE ACABAR CUANDO SE LLEGUE AL OBSTÁCULO
    }
   
    xTaskDelayUntil(&x_last_wake_time, OBSTACLE_TASK_PERIOD / portTICK_PERIOD_MS);
  }
}