#include <string.h>

#include <Arduino_FreeRTOS.h>
#include "FastLED.h"

#include "config.h"

// +++++ VARIABLES +++++
CRGB leds[NUM_LEDS];

TaskHandle_t follow_line_task_handle; // TODO mencionarlo en el BLOG

// +++++ FUNCITONS DECLARATION +++++

void set_led_color(uint8_t r, uint8_t g, uint8_t b) {
  uint32_t color = (((uint32_t)r << 16) | ((uint32_t)g << 8) | b);

  FastLED.showColor(color);
}

void follow_line_task(void *params) {
  (void) params;

  TickType_t x_last_wake_time;
  float vr, vl;
  // unsigned long time_lost_start = 0, time_lost = 0;

  String line_state = CENTER_ONLY;
  float ir_values[N_IR_SENSORS]; // {l, m, r}

  bool last_turn_is_right = true; // !!!!! SERGIO !!!!! He añadido esto para los casos en los que pase de 010 o 111 a 000 directamente, porque pasa JJAJJAJA

  int ir_sensors[] = IR_SENSORS_LIST;

  int right_error = 0, left_error = 0;

  FastLED.setBrightness(LED_BRIGHTNESS);

  while (1) {
    x_last_wake_time = xTaskGetTickCount();

    for (int i = 0; i < N_IR_SENSORS; i++) {
      ir_values[i] = analogRead(ir_sensors[i]);
      line_state[i] = ir_values[i] >= IR_BLACK_THRESHOLD ? '1' : '0';  // !!!!! SERGIO !!!!! 1 = la línea está ahí (la linea está justo debajo del igual por el estilo)
    }

    // !!!!! SERGIO !!!!! VOY A CAMBIAR EL PANORAMA (EN VEZ DE TIEMPO, EL VALOR DE LOS IR)

    if (line_state == LEFT || line_state == CENTER_LEFT) {
      last_turn_is_right = false;
    } else if (line_state == RIGHT || line_state == CENTER_RIGHT) {
      last_turn_is_right = true;
    }

    // !!!!! SERGIO !!!!! He puesto lo de que no haya línea porque estaba dando problemas con la nueva implementación usando los valores del IR

    vr = V_REF + (KP * ir_values[0]) + (KD * (ir_values[0] - right_error));
    vl = V_REF + (KP * ir_values[2]) + (KD * (ir_values[2] - left_error));

    if (vr < V_MIN) {
      vr = V_MIN;
    }

    if (vl < V_MIN) {
      vl = V_MIN;
    }

    if (line_state == NO_LINE) {
      set_led_color(MAX_RGB_COMPONENT_VALUE,0,0);

      if (last_turn_is_right) {
        vr = V_MIN;
        vl = V_LOST;
      } else {
        vr = V_LOST;
        vl = V_MIN;
      }
      
    } else {
      set_led_color(0, MAX_RGB_COMPONENT_VALUE,0);
    }
    
    analogWrite(PIN_Motor_PWMA, vr);
    analogWrite(PIN_Motor_PWMB, vl);

    right_error = ir_values[0];
    left_error = ir_values[1];

    xTaskDelayUntil(&x_last_wake_time, FOLLOW_LINE_TASK_PERIOD / portTICK_PERIOD_MS);
  }
}

void obstacle_task(void *params) {
  (void) params;

  TickType_t x_last_wake_time;
  float dist;
  bool is_finished = false;

  unsigned long time;

  while (!is_finished) {
    x_last_wake_time = xTaskGetTickCount();

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(TRIG_DELAY);
    digitalWrite(TRIG_PIN, LOW);

    time = pulseIn(ECHO_PIN, HIGH);

    //Serial.println(time);

    dist = (time / 2) * SOUND_SPEED; // Unit: cm

    if (dist <= OBSTACLE_DIST_LIMIT) {
      Serial.println(dist);
      break;
    }
  
    xTaskDelayUntil(&x_last_wake_time, OBSTACLE_TASK_PERIOD / portTICK_PERIOD_MS);
  }

  vTaskSuspend(follow_line_task_handle);             // !!!!! SERGIO !!!!! Lo he comentado ahora para probar con trazas lo de los IR

  analogWrite(PIN_Motor_PWMA, MOTOR_STOP);
  analogWrite(PIN_Motor_PWMB, MOTOR_STOP);

  digitalWrite(PIN_Motor_STBY, LOW);

  is_finished = true; // TODO (quitar comentario) EN CASO DE QUE EL PROGRAMA TENGA QUE ACABAR CUANDO SE LLEGUE AL OBSTÁCULO

  set_led_color(MAX_RGB_COMPONENT_VALUE, 0, MAX_RGB_COMPONENT_VALUE);

  Serial.println("End");

  vTaskSuspend(NULL);
}

// +++++ MAIN PROGRAM +++++
void setup() {
  Serial.begin(9600);
  
  //while(!Serial); // Wait until Serial port is intialized

  Serial.println("Antes");

  while(Serial.available() == 0); // TODO sustituir por inicializacion de conexión

  delay(10);

  Serial.println("Despues");

  // Ultrasonic sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

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

  // Tasks initialization
  xTaskCreate(
    obstacle_task,
    "obstacle_task",
    100,
    NULL,
    OBSTACLE_PRIO,
    NULL
  );
  
  xTaskCreate(
    follow_line_task,
    "follow_line_task",
    100,
    NULL,
    FOLLOW_LINE_PRIO,
    &follow_line_task_handle
  );
}

void loop() {
  // put your main code here, to run repeatedly:

}