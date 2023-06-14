#include <Adafruit_PWMServoDriver.h>
#include <DifferentialSteering.h>
#include <PS2X_lib.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
DifferentialSteering diffSteer;
PS2X ps2x;

// 1st motor slot (pins 8, 9): left wheel
#define leftFwd 8
#define leftBck 9
// 2nd motor slot (pins 10, 11): right wheel
#define rightFwd 10
#define rightBck 11
// 3rd motor slot (pins 12, 13): grabber
#define grabberFwd 12
#define grabberBck 13
// 4th motor slot (pins 14, 15): shooter
#define shooterFwd 14
#define shooterBck 15

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
  Serial.begin(115000);
  Wire.begin();
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(50);
  Wire.setClock(400000);

  // No pressures + No rumble (don't care about it)
  // ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_ATT, PS2_DAT, false, false);
  ps2x.config_gamepad(14, 13, 15, 12, false, false);

  diffSteer.begin(64);
}

void beginLoop()
{
  // No vibration, so it's false and vibration level is 0
  ps2x.read_gamepad(false, 0);

  // Initialize maximum allowed speed (hold PSB_R2 to enable Fullspeed)
  maxSpeed = ps2x.Button(PSB_R2)? 4095 : 3200;

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
  pwm.setPWM(grabberFwd, 0, (grabberOn? (grabberClockwise? maxSpeed : 0) : 0));
  pwm.setPWM(grabberBck, 0, (grabberOn? (grabberClockwise? 0 : maxSpeed) : 0));
}

void shooter()
{
  pwm.setPWM(shooterFwd, 0, (shooterOn? maxSpeed : 0));
  pwm.setPWM(shooterBck, 0, 0);
}

void endLoop()
{
  // A delay of 30 milliseconds is added at the end
  // of each loop iteration to control the loop rate.
  // Note: I don't want to use `delay(30)` as it will delay the whole system
  while (millis() - t <= 30);   // Do nothing
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
