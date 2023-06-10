#include <Adafruit_PWMServoDriver.h>

// 4th motor slot (pins 14, 15)
#define shooterFwd 14
#define shooterBck 15

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
char debug[100];  // at most 100 characters

void shooter(bool shooterOn, int maxSpeed)
{
    // Set pwm
    pwm.setPWM(shooterFwd, 0, (shooterOn? maxSpeed : 0));
    pwm.setPWM(shooterBck, 0, 0);
    
    // Print debug
    sprintf(debug, "Shooter state  :  %s\n", (shooterOn? "On" : "Off"));
    Serial.print(debug);
}