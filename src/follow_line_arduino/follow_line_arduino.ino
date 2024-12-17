#include <string.h>

#include <Arduino_FreeRTOS.h>
#include "FastLED.h"

#include "config.h"
#include "/home/scobosb/Escritorio/sist_emp/sistemas_empotrados/practicas/p4/follow-line-p4/src/serial_comms/comms.h"


// +++++ VARIABLES +++++
CRGB leds[NUM_LEDS];

TaskHandle_t follow_line_task_handle; // TODO mencionarlo en el BLOG

char serial_msg_buffer[PROTOCOL_MSG_SIZE];

// +++++ FUNCITONS DECLARATION +++++

void set_led_color(uint8_t r, uint8_t g, uint8_t b) {
  uint32_t color = (((uint32_t)r << 16) | ((uint32_t)g << 8) | b);

  FastLED.showColor(color);
}

void follow_line_task(void *params) {
  Serial.println("follow line task created");

  (void) params;

  TickType_t x_last_wake_time;
  float vr, vl;
  struct protocol_msg serial_msg;
  // unsigned long time_lost_start = 0, time_lost = 0;

  String line_state = CENTER_ONLY;
  float ir_values[N_IR_SENSORS]; // {l, m, r}

  bool last_turn_is_right = true; // !!!!! SERGIO !!!!! He añadido esto para los casos en los que pase de 010 o 111 a 000 directamente, porque pasa JJAJJAJA

  int ir_sensors[] = IR_SENSORS_LIST;

  int right_error = 0, left_error = 0;

  serial_msg.id = LINE_LOST;

  FastLED.setBrightness(LED_BRIGHTNESS);

  while (1) {
    for (int i = 0; i < N_IR_SENSORS; i++) {
      ir_values[i] = analogRead(ir_sensors[i]);
      line_state[i] = ir_values[i] >= IR_BLACK_THRESHOLD ? '1' : '0';  // !!!!! SERGIO !!!!! 1 = la línea está ahí (la linea está justo debajo del igual por el estilo)
    }

    Serial.println(line_state);

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
      create_msg_with_protocol(&serial_msg, serial_msg_buffer, PROTOCOL_MSG_SIZE);
      Serial.write(serial_msg_buffer);

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
    
    //analogWrite(PIN_Motor_PWMA, vr);
    //analogWrite(PIN_Motor_PWMB, vl);

    right_error = ir_values[0];
    left_error = ir_values[1];
  }
}


void obstacle_task(void *params) {
  Serial.println("obstacle task created");
  (void) params;

  TickType_t x_last_wake_time;
  float dist = 100.0;
  struct protocol_msg serial_msg;

  unsigned long time;

  while (true) {
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

  Serial.println("task suspended");

  analogWrite(PIN_Motor_PWMA, MOTOR_STOP);
  analogWrite(PIN_Motor_PWMB, MOTOR_STOP);

  digitalWrite(PIN_Motor_STBY, LOW);

  Serial.println("obstacle detected");

  serial_msg.id = OBSTACLE_DETECTED;
  serial_msg.arg = (int) dist;

  create_msg_with_protocol(&serial_msg, serial_msg_buffer, PROTOCOL_MSG_SIZE);
  Serial.write(serial_msg_buffer);

  Serial.println("To send end lap");

  serial_msg.id = END_LAP;
  serial_msg.arg = millis();

  create_msg_with_protocol(&serial_msg, serial_msg_buffer, PROTOCOL_MSG_SIZE);
  Serial.write(serial_msg_buffer);

  vTaskSuspend(NULL);
}

// +++++ MAIN PROGRAM +++++
void setup() {
  struct protocol_msg start_msg, ack;
  start_msg.id = INVALID_MSG;

  // Neopixel led
  FastLED.addLeds<NEOPIXEL, PIN_RBGLED>(leds, NUM_LEDS);
  FastLED.setBrightness(0);

  // Serial comms
  Serial.begin(COMMS_BAUD_RATE);
  
  while(!Serial); // Wait until Serial port is intialized
  
  FastLED.setBrightness(20);
  set_led_color(0, 0, 255);

  do {
    start_msg = read_msg(&Serial);
    delay(INITIAL_COMMS_DELAY);
  } while(start_msg.id != START_LAP);

  set_led_color(255, 255, 0);

  delay(INITIAL_COMMS_DELAY);

  ack.id = ACK;
  ack.arg = 0.00;

  create_msg_with_protocol(&ack, serial_msg_buffer, PROTOCOL_MSG_SIZE);

  Serial.write(serial_msg_buffer); // Confirmation for ESP

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

  Serial.println("Pins initialized");

  // Tasks initialization

  /*
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
  */
}

void loop() {
  for (int i = 0; i < N_IR_SENSORS; i++) {
      ir_values[i] = analogRead(ir_sensors[i]);
      line_state[i] = ir_values[i] >= IR_BLACK_THRESHOLD ? '1' : '0';  // !!!!! SERGIO !!!!! 1 = la línea está ahí (la linea está justo debajo del igual por el estilo)
    }

    Serial.println(line_state);

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
      create_msg_with_protocol(&serial_msg, serial_msg_buffer, PROTOCOL_MSG_SIZE);
      Serial.write(serial_msg_buffer);

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
    
    //analogWrite(PIN_Motor_PWMA, vr);
    //analogWrite(PIN_Motor_PWMB, vl);

    right_error = ir_values[0];
    left_error = ir_values[1];

}