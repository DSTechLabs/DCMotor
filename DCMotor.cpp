//==========================================================
//
//   FILE   : DCMotor.cpp
//
//  PROJECT : Driving DC Motors with PWM
//
//   AUTHOR : Bill Daniels (bill@dstechlabs.com)
//            See LICENSE.md
//
//==========================================================

#include <Arduino.h>
#include <string.h>
#include "DCMotor.h"

//--- Constructor ------------------------------------------

DCMotor::DCMotor (int PWMPin1, int PWMPin2, int LLSwitchPin, int ULSwitchPin)
{
  // Save the specified pins for the both driver types
  pwmPin1     = PWMPin1;
  pwmPin2     = PWMPin2;
  llSwitchPin = LLSwitchPin;
  ulSwitchPin = ULSwitchPin;

  // Set pin modes
  pinMode (pwmPin1, OUTPUT);
  pinMode (pwmPin2, OUTPUT);

  if (llSwitchPin != 0)
    pinMode (llSwitchPin, INPUT_PULLUP);

  if (ulSwitchPin != 0)
    pinMode (ulSwitchPin, INPUT_PULLUP);

  // Maker sure motor is stopped
  analogWrite (pwmPin1, 0);
  analogWrite (pwmPin2, 0);
  state = STOPPED;

  // Default ramping
  SetRamp (4);
}

//--- Run -------------------------------------------------
// Must be called inside your loop function with no delay.

RunReturn DCMotor::Run ()
{
  // Is the motor RUNNING
  if (state != STOPPED)
  {
    // Check limit switches, if specified
    if ((llSwitchPin != 0) && digitalRead (llSwitchPin) == LOW)
    {
      // Lower limit switch triggered
      EStop ();
      return LIMIT_SWITCH_LOWER;
    }

    if ((ulSwitchPin != 0) && digitalRead (ulSwitchPin) == LOW)
    {
      // Upper limit switch triggered
      EStop ();
      return LIMIT_SWITCH_UPPER;
    }

    // Is motor still ramping up/down?
    if (state != AT_SPEED)
    {
      // Is it time to adjust speed?
      now = micros ();
      if (now >= nextPWMMicros)
      {
        // Adjust speed
        currentPWM += pwmIncrement;  // +1 for ramping up, -1 for ramping down

        if (currentDirection == CW)
          analogWrite (pwmPin2, currentPWM);
        else
          analogWrite (pwmPin1, currentPWM);

        // Has the motor reached target speed?
        if (currentPWM == targetPWM)
        {
          if (targetPWM == 0)
            state = STOPPED;
          else
            state = AT_SPEED;
        }
        else
          // Still ramping, set new speed
          nextPWMMicros = now + rampPeriod;
      }
    }
  }

  return OKAY;
}

//--- SetRamp ---------------------------------------------

void DCMotor::SetRamp (int ramp)
{
  // Must be 0..9
  if (ramp >= 0 && ramp <= 9)
  {
    // Set run time for each ramping speed in microseconds
    rampPeriod = 2000L * ramp + 2000L;
  }
}

//--- Go --------------------------------------------------

void DCMotor::Go (Direction dir, int speed)
{
  // We need to handle this command from any MotorState.
  // If STOPPED, we just ramp up to speed.
  // If running in the same direction, we alter our speed
  //   and either ramp up or down to the new speed.
  // If running in the opposite direction, we need to
  //   ramp down to a stop, then ramp back up to speed
  //   in the opposite direction.

  // Check speed param
  if      (speed > 100) speed = 100;
  else if (speed <   0) speed = 0;

  if (state != STOPPED)
  {
    //--- Already running ---
    if (dir == currentDirection)
    {
      //--- Same direction ---
      targetPWM = speed * 250 / 100;  // new target speed

      // Do we need to ramp up or down to the new speed?
      if (targetPWM > currentPWM)
      {
        pwmIncrement = 1;    // ramp up
        state = RAMPING_UP;
      }
      else if (targetPWM < currentPWM)
      {
        pwmIncrement = -1;    // ramp down
        state = RAMPING_DOWN;
      }

      return;  // Ignore same speed request
    }
    else
    {
      //--- Reverse direction ---
      Stop ();
      while (state != STOPPED) Run ();  // Wait for motor to stop

      // Fall into code below to begin new motion
    }
  }

  if (speed != 0)
  {
    // Begin ramping up to speed from STOPPED:
    // Speed is specified as a percentage (0-100) of max pulse width value of 250
    // 50% speed would be a pulse width of 125

    // Set direction
    currentDirection = dir;
    if (currentDirection == CW)
      digitalWrite (pwmPin1, LOW);  // pwmPin2 will be the PWM pin
    else
      digitalWrite (pwmPin2, LOW);  // pwmPin1 will be the PWM pin

    currentPWM   = 0;                  // starting from stopped
    targetPWM    = speed * 250 / 100;  // pwm is % of 250
    pwmIncrement = 1;                  // ramping up

    // Begin motion
    nextPWMMicros = micros ();
    state = RAMPING_UP;
  }
}

//--- Stop ------------------------------------------------

void DCMotor::Stop ()
{
  if (state != STOPPED && currentPWM != 0)
  {
    // Begin ramping down to stop:
    targetPWM    = 0;
    pwmIncrement = -1;  // ramping down

    // Begin motion
    nextPWMMicros = micros ();
    state = RAMPING_DOWN;
  }
}

//--- EStop -----------------------------------------------

void DCMotor::EStop ()
{
  // Emergency Stop
	analogWrite (pwmPin1, 0);
	analogWrite (pwmPin2, 0);

  state = STOPPED;
}

//--- GetState --------------------------------------------

MotorState DCMotor::GetState ()
{
  // Return current motor state
  return state;
}


//=========================================================
//  ExecuteCommand
//=========================================================

const char * DCMotor::ExecuteCommand (const char *packet)
{
  char  command[3];
  int   ramp, speed;

  // Initialize return string
  ecReturnString[0] = 0;

  // Command string must be at least 2 chars
  if (strlen(packet) < 2)
  {
    strcpy (ecReturnString, "Bad command");
    return ecReturnString;
  }

  // Set 2-Char Command and parse all commands
  strncpy (command, packet, 2);
  command[2] = 0;

  //--- E-Stop ---
  if (strcmp (command, "ES") == 0)
    EStop ();

  //--- Set Ramp ---
  else if (strcmp (command, "SR") == 0)
  {
    // Check for value
    if (strlen (packet) != 3)
      strcpy (ecReturnString, "Missing ramp value 0-9");
    else
    {
      ramp = atoi (packet+2);

      // Check specified ramp value
      if (ramp >= 0 && ramp <= 9)
        SetRamp (ramp);
    }
  }

  //--- GO ---
  else if (strcmp (command, "GO") == 0)
  {
    // GO command must be at least 4 chars
    if (strlen (packet) < 4)
      strcpy (ecReturnString, "Bad command");
    else
    {
      Direction dir = CW;

      // Parse direction and speed
      if (packet[2] == '<')
        dir = CCW;

      speed = atoi (packet + 3);

      Go (dir, speed);
    }
  }

  //--- STOP ---
  else if (strcmp (command, "ST") == 0)
    Stop ();

  //--- GET STATE ---
  else if (strcmp (command, "GS") == 0)
  {
    switch (state)
    {
      case STOPPED      : strcpy (ecReturnString, "ST"); break;
      case RAMPING_UP   : strcpy (ecReturnString, "RU"); break;
      case RAMPING_DOWN : strcpy (ecReturnString, "RD"); break;
      case AT_SPEED     : strcpy (ecReturnString, "AS"); break;

      default : strcpy (ecReturnString, "??"); break;
    }
  }

  else
    strcpy (ecReturnString, "Unknown command");

  return ecReturnString;
}
