#include <Adafruit_PWMServoDriver.h>
#include <DifferentialSteering.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
DifferentialSteering diffSteer;

// 1st motor slot (pins 8, 9): left wheel
// 2nd motor slot (pins 10, 11): right wheel
#define leftFwd 8
#define leftBck 9
#define rightFwd 10
#define rightBck 11

int prevLeftFwd = 0;
int prevLeftBck = 0;
int prevRightFwd = 0;
int prevRightBck = 0;
int curLeftFwd;
int curLeftBck;
int curRightFwd; 
int curRightBck;
char debug[100];  // at most 100 characters

void coastMode()
{
    for (int spd = prevLeftFwd; spd != curLeftFwd; (prevLeftFwd > curLeftFwd? --spd : ++spd))
        pwm.setPWM(leftFwd, 0, spd);
    for (int spd = prevLeftBck; spd != curLeftBck; (prevLeftBck > curLeftBck? --spd : ++spd))
        pwm.setPWM(leftBck, 0, spd);
    for (int spd = prevRightFwd; spd != curRightFwd; (prevRightFwd > curRightFwd? --spd : ++spd))
        pwm.setPWM(rightFwd, 0, spd);
    for (int spd = prevRightBck; spd != curRightBck; (prevRightBck > curRightBck? --spd : ++spd))
        pwm.setPWM(rightBck, 0, spd);
}

void brakeMode()
{
    pwm.setPWM(leftFwd, 0, curLeftFwd);
    pwm.setPWM(leftBck, 0, curLeftBck);
    pwm.setPWM(rightFwd, 0, curRightFwd);
    pwm.setPWM(rightBck, 0, curRightBck);
}

void singleSteer(int anaX, int anaY, int maxSpeed)
{
    // The inputs of `computeMotors` method have to be mapped to [-127, 127]
    int mappedX = map(anaX, 0, 255, -127, 127);
    int mappedY = map(anaY, 0, 255, -127, 127);
    diffSteer.computeMotors(mappedX, mappedY);

    // The output range will be [-127, 127], map it to PWM value range
    int computedLeft = map(diffSteer.computedLeftMotor(), -127, 127, -maxSpeed, maxSpeed);
    int computedRight = map(diffSteer.computedRightMotor(), -127, 127, -maxSpeed, maxSpeed);

    // Note: I don't want the steering to be too sensitive, so if computedLeft
    //       in [-100, 100], left motor stays idle (the same for right motor)
    curLeftFwd = computedLeft > 100? computedLeft : 0;
    curLeftBck = computedLeft < -100? -computedLeft : 0;
    curRightFwd = computedRight > 100? computedRight : 0;
    curRightBck = computedRight < -100? -computedRight : 0;

    // Print debug
    sprintf(debug, "Analog from PS2:  PSS_RX: %d\tPSS_RY: %d\n", anaX, anaY);
    Serial.print(debug);
    sprintf(debug, "Computed output:  Left: %d\tRight: %d\n", computedLeft, computedRight);
    Serial.print(debug);
}

void dualSteer(int anaL, int anaR, int maxSpeed)
{
    // Note: I don't want the steering to be too sensitive, so if anaL
    //       in (112, 142) (the distance from 127 is less than 15),
    //       left motor stays idle (the same for right motor)
    curLeftFwd = anaL >= 142? map(anaL, 142, 255, 0, maxSpeed) : 0;
    curLeftBck = anaL <= 112? map(anaL, 0, 112, 0, maxSpeed) : 0;
    curRightFwd = anaR >= 142? map(anaR, 142, 255, 0, maxSpeed) : 0;
    curRightFwd = anaR <= 112? map(anaR, 0, 112, 0, maxSpeed) : 0;

    // Print debug
    sprintf(debug, "Analog from PS2:  PSS_LY: %d\tPSS_RY: %d\n", anaL, anaR);
    Serial.print(debug);
    sprintf(debug, "Computed output:  LeftFWD: %d\tLeftBCK: %d\n", curLeftFwd, curLeftBck);
    Serial.print(debug);
    sprintf(debug, "                  RightFWD: %d\tRightBCK: %d\n", curRightFwd, curRightBck);
    Serial.print(debug);
}

void steer(int LY, int RX, int RY, int maxSpeed, bool single, bool coast)
{
    // Select steer style
    single? singleSteer(RX, RY, maxSpeed) : dualSteer(LY, RY, maxSpeed);

    // Select mode to steer
    coast? coastMode() : brakeMode();

    // Save previous pwm output for coast mode
    std::tie(prevLeftFwd, prevLeftBck, prevRightFwd, prevRightBck) = {curLeftFwd, curLeftBck, curRightFwd, curRightBck};
}