#include <string.h>
#include <TimerOne.h>

#include "FastLED.h"

#include "config.h"
#include "/home/scobosb/Escritorio/sist_emp/sistemas_empotrados/practicas/p4/follow-line-p4/src/serial_comms/comms.h"


// +++++ VARIABLES +++++
CRGB leds[NUM_LEDS];

char serial_msg_buffer[PROTOCOL_MSG_SIZE];

volatile enum program_states program_state;

int ir_sensors[] = IR_SENSORS_LIST;
String line_state = CENTER_ONLY;
bool last_turn_is_right = true, line_is_lost;
int right_error = 0, left_error = 0;
unsigned long start_time;
float iterations, n_times_line_is_detected;

// +++++ FUNCITONS DECLARATION +++++

void set_led_color(uint8_t r, uint8_t g, uint8_t b) {
  CRGB color = CRGB(r, g, b); // Crear un color en formato CRGB
  FastLED.showColor(color);  // Mostrar el color en los LEDs
}

void set_obstacle_state() {
  program_state = OBSTACLE_DETECTION;
}

// +++++ MAIN PROGRAM +++++
void setup() {
  struct protocol_msg start_msg, ack;
  start_msg.id = INVALID_MSG;

  // Neopixel led
  FastLED.addLeds<NEOPIXEL, PIN_RBGLED>(leds, NUM_LEDS);
  FastLED.setBrightness(LED_BRIGHTNESS);

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

  set_led_color(0,0,MAX_RGB_COMPONENT_VALUE);

  program_state = FOLLOW_LINE;
  line_is_lost = false;

  iterations = 0.0;
  n_times_line_is_detected = 0.0;

  // Serial comms
  Serial.begin(COMMS_BAUD_RATE);
  
  while(!Serial); // Wait until Serial port is intialized
  
  // FastLED.setBrightness(20);
  //set_led_color(0, 0, 255);

  do {
    start_msg = read_msg(&Serial);
    delay(INITIAL_COMMS_DELAY);
  } while(start_msg.id != START_LAP);

  delay(INITIAL_COMMS_DELAY);

  ack.id = ACK;
  ack.arg = 0.00;

  Serial.print(create_msg_with_protocol(&ack));

  start_time = millis();

  // Initialize obstacle interruption timer
  Timer1.initialize(OBSTACLE_INTERRUPTION_PERIOD);
  Timer1.attachInterrupt(set_obstacle_state);
}

void loop() {
  struct protocol_msg serial_msg;

  float vr, vl, dist;
  float ir_values[N_IR_SENSORS];

  unsigned long time;

  switch (program_state) {
    case FOLLOW_LINE:
      iterations++;

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
        if (!line_is_lost) {
          set_led_color(MAX_RGB_COMPONENT_VALUE,0,0);
          line_is_lost = true;

          serial_msg.id = LINE_LOST;
          Serial.print(create_msg_with_protocol(&serial_msg));

          serial_msg.id = INIT_LINE_SEARCH;
          Serial.print(create_msg_with_protocol(&serial_msg));
        }

        if (last_turn_is_right) {
          vr = V_MIN;
          vl = V_LOST;
        } else {
          vr = V_LOST;
          vl = V_MIN;
        }
        
      } else {
        n_times_line_is_detected++;

        if (line_is_lost) {
          serial_msg.id = STOP_LINE_SEARCH;
          Serial.print(create_msg_with_protocol(&serial_msg));

          serial_msg.id = LINE_FOUND;
          Serial.print(create_msg_with_protocol(&serial_msg));

          set_led_color(1, MAX_RGB_COMPONENT_VALUE,1);
        }

        line_is_lost = false;
      }
      
      analogWrite(PIN_Motor_PWMA, vr);
      analogWrite(PIN_Motor_PWMB, vl);

      right_error = ir_values[0];
      left_error = ir_values[1];

      break;

  case OBSTACLE_DETECTION:
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(TRIG_DELAY);
    digitalWrite(TRIG_PIN, LOW);

    time = pulseIn(ECHO_PIN, HIGH);

    //Serial.println(time);

    dist = (time / 2) * SOUND_SPEED; // Unit: cm

    program_state = FOLLOW_LINE;

    if (dist <= OBSTACLE_DIST_LIMIT) {
      Timer1.detachInterrupt();
      analogWrite(PIN_Motor_PWMA, MOTOR_STOP);
      analogWrite(PIN_Motor_PWMB, MOTOR_STOP);

      digitalWrite(PIN_Motor_STBY, LOW);

      serial_msg.id = OBSTACLE_DETECTED;
      serial_msg.arg = (int) dist;

      Serial.print(create_msg_with_protocol(&serial_msg));

      serial_msg.id = END_LAP;
      serial_msg.arg = millis() - start_time;

      Serial.print(create_msg_with_protocol(&serial_msg));

      serial_msg.id = VISIBLE_LINE;
      serial_msg.arg = (n_times_line_is_detected / iterations) * PERCENTAGE_MULTIPLIER;

      Serial.println();
      Serial.println(n_times_line_is_detected);
      Serial.println(iterations);
      Serial.println(serial_msg.arg);

      Serial.print(create_msg_with_protocol(&serial_msg));

      program_state = END;
    }

    break;

  case END:
    break;
  }
}