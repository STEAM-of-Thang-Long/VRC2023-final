#include <Adafruit_PWMServoDriver.h>
#include <PS2X_lib.h>
#include <Steer.h>
#include <Grabber.h>
#include <Shooter.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
PS2X ps2x;

bool grabberOn = false;
bool grabberClockwise = true;
bool shooterOn = false;
int maxSpeed;

bool coast = false;
bool single = true;

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

    // Initialize maximum allowed speed (press PSB_R2 to enable Fullspeed)
    maxSpeed = ps2x.Button(PSB_R2)? 4095 : 3200;
    
    if (ps2x.ButtonPressed(PSB_R1))      // Change state of shooter
        shooterOn = !shooterOn;
    if (ps2x.ButtonPressed(PSB_L1))      // Change state of grabber
        grabberOn = !grabberOn;
    if (ps2x.ButtonPressed(PSB_L2))      // Change spin direction of grabber
        grabberClockwise = !grabberClockwise;
    if (ps2x.ButtonPressed(PSB_CROSS))   // Change mode to steer
        coast = !coast;
    if (ps2x.ButtonPressed(PSB_SELECT))  // Change steer style
        single = !single;
}

void endLoop()
{
    // A delay of 30 milliseconds is added at the end
    // of each loop iteration to control the loop rate.
    delay(30);
    Serial.print('\n');
}

void loop()
{
    beginLoop();
    steer(255-ps2x.Analog(PSS_LY), ps2x.Analog(PSS_RX), 255-ps2x.Analog(PSS_RY), maxSpeed, single, coast);
    grabber(grabberOn, grabberClockwise, maxSpeed);
    shooter(shooterOn, maxSpeed);
    endLoop();
}
