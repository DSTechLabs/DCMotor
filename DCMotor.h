//=============================================================================
//
//     FILE : DCMotor.h
//
//  PROJECT : Driving DC Motors with PWM
//
//   AUTHOR : Bill Daniels (bill@dstechlabs.com)
//            See LICENSE.md
//
//=============================================================================
//
//  This class assumes you are using a standard DC motor driver with two signal pins
//  specified in the constructor.  Motor direction is specified as clockwise (CW) or
//  counter-clockwise (CCW).  Motor speed is specified as a percentage (0 - 100)%.
//
//  All control can be performed either by directly calling methods or by executing a command string,
//  from a UI for example.  Command strings and responses are sent through the serial port.
//
//  Your app should normally wait until the motor is finished with a previous motion command
//  before issuing a new command.  If a motion command is called while the motor is already
//  running, then the current motion is interrupted and the new motion begins from the
//  motor's current position.
//
//  The ExecuteCommand() method handles the following 2-char commands:
//
//    SRr  = SET RAMP   - Sets the speed ramping factor for smooth motor start and stop
//    GOds = GO         - Begin ramping up the motor to the specified speed and direction
//    ST   = STOP       - Begin ramping down the motor until it stops
//    ES   = E-STOP     - Stops the motor immediately
//    GS   = GET STATE  - Get the current state of the motor (see the MotorState enum)
//
//    where:
//      r is the ramp rate (0-9)
//      d is direction (< or >)
//      s is speed (0-100)%
//
//=============================================================================

#ifndef DCM_H
#define DCM_H

#define EC_RETURN_LENGTH  30

enum MotorState
{
  STOPPED,
  RAMPING_UP,
  AT_SPEED,
  RAMPING_DOWN
};

enum Direction
{
  CW,  // clockwise
  CCW  // counter-clockwise
};

enum RunReturn
{
  OKAY,                // Idle or still running
  RUN_COMPLETE,        // Motion complete, reached target position normally
  RANGE_ERROR_LOWER,   // Reached lower range limit
  RANGE_ERROR_UPPER,   // Reached upper range limit
  LIMIT_SWITCH_LOWER,  // Lower limit switch triggered
  LIMIT_SWITCH_UPPER   // Upper limit switch triggered
};


//=========================================================
//  class DCMotor
//=========================================================

class DCMotor
{
  private:
    int  pwmPin1, pwmPin2;          // PWM signal pins
    int  llSwitchPin, ulSwitchPin;  // Limit switch pins

    MotorState     state            = STOPPED;  // Current motor state
    Direction      currentDirection = CW;       // Currently running clockwise or counter-clockwise
    int            currentPWM       = 0;        // Current speed (PWM value)
    int            targetPWM        = 0;        // Upper speed to run motor to be AT_SPEED (PWM value)
    int            pwmIncrement     = 1;        // +1 (ramping up) or -1 (ramping down)
    unsigned long  rampPeriod       = 10000L;   // Delay time for ramping speed up/down (rate of acceleration)
    unsigned long  nextPWMMicros;               // Target micros for next speed increment
    unsigned long  now;

    char  ecReturnString[EC_RETURN_LENGTH];  // Return string from ExecuteCommand()

  public:
    DCMotor (int PWMPin1, int PWMPin2, int LLSwitchPin=0, int ULSwitchPin=0);

    RunReturn  Run ();  // Keeps the motor running (must be called from your loop() function with no delay)

    void          SetRamp        (int ramp);                  // Sets the run time for each ramping speed (0 - 9)
    void          Go             (Direction dir, int speed);  // Ramp up to speed
    void          Stop           ();                          // Ramp down to stop
    void          EStop          ();                          // Immediately stops motor
    MotorState    GetState       ();                          // Returns current state of motor
    const char *  ExecuteCommand (const char *packet);        // Execute a string command (see notes above)
};

#endif
