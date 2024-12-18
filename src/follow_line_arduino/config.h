#ifndef CONFIG_H
#define CONFIG_H

// ----------- PINS DEFINITION --------------
// Ultrasonic sensor
#define TRIG_PIN 13
#define ECHO_PIN 12

// IR sensors
#define PIN_ITR20001_LEFT   A2
#define PIN_ITR20001_MIDDLE A1
#define PIN_ITR20001_RIGHT  A0

// Enable/Disable motor control.
//  HIGH: motor control enabled
//  LOW: motor control disabled
#define PIN_Motor_STBY 3

// Group A Motors (Right Side)
// PIN_Motor_AIN_1: Digital output. HIGH: Forward, LOW: Backward
#define PIN_Motor_AIN_1 7
// PIN_Motor_PWMA: Analog output [0-255]. It provides speed.
#define PIN_Motor_PWMA 5

// Group B Motors (Left Side)
// PIN_Motor_BIN_1: Digital output. HIGH: Forward, LOW: Backward
#define PIN_Motor_BIN_1 8
// PIN_Motor_PWMB: Analog output [0-255]. It provides speed.
#define PIN_Motor_PWMB 6

// Neopixel
#define PIN_RBGLED 4
// ------------------------------------------

// --------- NEOPIXEL UTILS --------------
#define MAX_RGB_COMPONENT_VALUE 255
#define RGB_COMPONENTS 3

#define NUM_LEDS 1
// ---------------------------------------

// ----------- ULTRASONIC UTILS --------------
#define OBSTACLE_DIST_LIMIT 8 // Unit: cm
#define SOUND_SPEED 0.0343 // Unit: cm/us
#define TRIG_DELAY 10 // Unit: us 
// -------------------------------------------

// ----------- MOTOR UTILS --------------
#define MOTOR_STOP 0
#define MOTOR_MAX_SPEED 255
// --------------------------------------

// ---------- IR UTILS ------------
#define IR_BLACK_THRESHOLD 200                    //  !!!!! NOTA PARA SERGIO !!!!! He cambiado esto por black threshold por lo de que se envía un valor alto cuando es negro, 
                                                  //                               entonces decimos que será negro si el valor devuelto es mayor que IR_BLACK_THRESHOLD

#define IR_SENSORS_LIST {PIN_ITR20001_LEFT, PIN_ITR20001_MIDDLE, PIN_ITR20001_RIGHT}
#define N_IR_SENSORS 3
// --------------------------------

// --------- FOLLOW LINE UTILS ---------
enum program_states {
  FOLLOW_LINE = 0,
  OBSTACLE_DETECTION,
  END
};

#define OBSTACLE_INTERRUPTION_PERIOD 15

#define V_REF 100
#define V_LOST 200
#define V_MIN 0

// Line states [1 = black (line), 0 = white (no line)]       !!!!! SERGIO LEE ESTO !!!!!
#define LEFT          "100"
#define CENTER_LEFT   "110"
#define CENTER_ALL    "111"
#define CENTER_ONLY   "010"
#define CENTER_RIGHT  "011"
#define RIGHT         "001"
#define NO_LINE       "000"

#define KP 0.12
#define KD 0.015

#define LED_BRIGHTNESS 20

#define PERCENTAGE_MULTIPLIER 100.0
// -------------------------------------

// -------- SERIAL CONFIG ----------
#define INITIAL_COMMS_DELAY 10 // Unit: ms
// ---------------------------------
#endif CONFIG_H