#include <Adafruit_PWMServoDriver.h>

// 3rd motor slot (pins 12, 13)
#define grabberFwd 12
#define grabberBck 13

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
char debug[100];  // at most 100 characters

void grabber(bool grabberOn, bool grabberClockwise, int maxSpeed)
{
    // Set pwm
    pwm.setPWM(grabberFwd, 0, (grabberOn? (grabberClockwise? maxSpeed : 0) : 0));
    pwm.setPWM(grabberBck, 0, (grabberOn? (grabberClockwise? 0 : maxSpeed) : 0));
    
    // Print debug
    sprintf(debug, "Grabber state  :  %s\n", (grabberOn? (grabberClockwise? "Clockwise" : "Counterclockwise") : "Off"));
    Serial.print(debug);
}