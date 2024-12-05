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
#define RED {255,0,0}
#define GREEN {0,255,0}
#define NUM_LEDS 1
// ---------------------------------------

// ----------- ULTRASONIC UTILS --------------
#define OBSTACLE_DIST_LIMIT 8 // Unit: cm
#define SOUND_SPEED 0.0343 // Unit: cm/us
#define TRIG_DELAY 10 // Unit: us
// -------------------------------------------

// --------- TASKS CONFIG ------------
#define OBSTACLE_TASK_PERIOD 150
// -----------------------------------

#endif CONFIG_H