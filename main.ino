#include <Adafruit_PWMServoDriver.h>
#include <DifferentialSteering.h>
#include <PS2X_lib.h>
#include "constants.h"

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
DifferentialSteering diffSteer;
PS2X ps2x;

// maximum allowed pwm output for 4 motors
int maxSpeed;
// For `grabber` function
bool grabberOn = false;
bool grabberClockwise = true;
// For `shooter` function
bool shooterOn = false;

// For steering
int prevLeftFwd = 0;
int prevLeftBck = 0;
int prevRightFwd = 0;
int prevRightBck = 0;
int curLeftFwd;
int curLeftBck;
int curRightFwd;
int curRightBck;
bool coast = false;
bool single = true;

// joystick analogs
int ly;  // Analog of PSS_LY
int rx;  // Analog of PSS_RX
int ry;  // Analog of PSS_RY

// Get timestamp for `endLoop` function
unsigned long t = millis();


void setup()    // Keep it unchanged, as it's perfect :)
{
  Serial.begin(BAUD_RATE);
  Wire.begin();
  pwm.begin();
  pwm.setOscillatorFrequency(OSC_FREQ);
  pwm.setPWMFreq(PWM_FREQ);
  Wire.setClock(CLOCK_FREQ);
  ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_ATT, PS2_DAT, PRESSURES, RUMBLE);
  diffSteer.begin(PIV_Y_LIMIT);
}

void beginLoop()
{
  // No vibration, so it's false and vibration level is 0
  ps2x.read_gamepad(false, 0);

  // Initialize maximum allowed speed (hold PSB_R2 to enable Fullspeed)
  maxSpeed = ps2x.Button(PSB_R2)? STEER_SPEED_MAX : STEER_SPEED_DEF;

  // Toggle (if you don't know what is bitwise XOR, search for it)
  shooterOn ^= ps2x.ButtonPressed(PSB_R1);        // Change state of shooter
  grabberOn ^= ps2x.ButtonPressed(PSB_L1);        // Change state of grabber
  grabberClockwise ^= ps2x.ButtonPressed(PSB_L2); // Change spin direction of grabber
  coast ^= ps2x.ButtonPressed(PSB_CROSS);         // Change mode to steer
  single ^= ps2x.ButtonPressed(PSB_SELECT);       // Change steer style

  ly = 255 - ps2x.Analog(PSS_LY);
  rx = ps2x.Analog(PSS_RX);
  ry = 255 - ps2x.Analog(PSS_RY);
}

void coastMode()  // Bug ơi là bug, bug vcl, đừng dùng cái này làm ơn 🙏🙏🙏
{
  for (int spd = prevLeftFwd; spd != curLeftFwd; (prevLeftFwd > curLeftFwd? --spd : ++spd))
    pwm.setPWM(LEFT_FWD, 0, spd);
  for (int spd = prevLeftBck; spd != curLeftBck; (prevLeftBck > curLeftBck? --spd : ++spd))
    pwm.setPWM(LEFT_BCK, 0, spd);
  for (int spd = prevRightFwd; spd != curRightFwd; (prevRightFwd > curRightFwd? --spd : ++spd))
    pwm.setPWM(RIGHT_FWD, 0, spd);
  for (int spd = prevRightBck; spd != curRightBck; (prevRightBck > curRightBck? --spd : ++spd))
    pwm.setPWM(RIGHT_BCK, 0, spd);
}

void brakeMode()
{
  pwm.setPWM(LEFT_FWD, 0, curLeftFwd);
  pwm.setPWM(LEFT_BCK, 0, curLeftBck);
  pwm.setPWM(RIGHT_FWD, 0, curRightFwd);
  pwm.setPWM(RIGHT_BCK, 0, curRightBck);
}

void singleSteer(int anaX, int anaY)
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
}

void dualSteer(int anaL, int anaR)
{
  // Note: I don't want the steering to be too sensitive, so if anaL
  //       in (112, 142) (the distance from 127 is less than 15),
  //       left motor stays idle (the same for right motor)
  curLeftFwd = anaL >= 142? map(anaL, 142, 255, 0, maxSpeed) : 0;
  curLeftBck = anaL <= 112? map(anaL, 0, 112, 0, maxSpeed) : 0;
  curRightFwd = anaR >= 142? map(anaR, 142, 255, 0, maxSpeed) : 0;
  curRightBck = anaR <= 112? map(anaR, 0, 112, 0, maxSpeed) : 0;
}

void steer()
{
  // Select steer style
  single? singleSteer(rx, ry) : dualSteer(ly, ry);

  // Select mode to steer
  coast? coastMode() : brakeMode();

  // Save previous pwm output for coast mode
  std::tie(prevLeftFwd, prevLeftBck, prevRightFwd, prevRightBck) = {curLeftFwd, curLeftBck, curRightFwd, curRightBck};
}

void grabber()
{
  pwm.setPWM(GRABBER_FWD, 0, (grabberOn? (grabberClockwise? GRABBER_SPEED : 0) : 0));
  pwm.setPWM(GRABBER_BCK, 0, (grabberOn? (grabberClockwise? 0 : GRABBER_SPEED) : 0));
}

void shooter()
{
  pwm.setPWM(SHOOTER_FWD, 0, (shooterOn? SHOOTER_SPEED : 0));
  pwm.setPWM(SHOOTER_BCK, 0, 0);
}

void endLoop()
{
  while (millis() - t <= DELAY_MS);   // Do nothing
  t = millis();
}

void loop()
{
  beginLoop();
  steer();
  grabber();
  shooter();
  endLoop();
}
