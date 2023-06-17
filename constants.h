/**
 *    PWM Output configuration
**/
#define DELAY_MS 50
#define STEER_SPEED_DEF 3200
#define STEER_SPEED_MAX 4095
#define GRABBER_SPEED 1024
#define SHOOTER_SPEED 4095

/**
 *    4 PWM Motors
**/
#define LEFT_FWD 8
#define LEFT_BCK 9
#define RIGHT_FWD 10
#define RIGHT_BCK 11
#define GRABBER_FWD 12
#define GRABBER_BCK 13
#define SHOOTER_FWD 14
#define SHOOTER_BCK 15

/**
 *    6 PWM Servos
**/
#define SERVO_2 2
#define SERVO_3 3
#define SERVO_4 4
#define SERVO_5 5
#define SERVO_6 6
#define SERVO_7 7

/**
 *    PS2 controller - config gamepad
**/
#define PS2_DAT 12
#define PS2_CMD 13
#define PS2_CLK 14
#define PS2_ATT 15
#define PRESSURES false
#define RUMBLE false

/**
 *    Setup
**/
#define BAUD_RATE 112500
#define OSC_FREQ 27000000
#define PWM_FREQ 50
#define CLOCK_FREQ 400000
#define PIV_Y_LIMIT 64
