//=============================================================================
//
//       FILE : DCMotor.ino
//
//    PROJECT : Any project needing to run un-encoded DC motors
//
//     AUTHOR : Bill Daniels
//              Copyright 2024, D+S Tech Labs, Inc.
//              All Rights Reserved
//
//=============================================================================

//--- Includes ---------------------------------------------

#include <Arduino.h>
#include "DCMotor.h"

//--- Defines ----------------------------------------------

#define SERIAL_BAUDRATE     115200L
#define MAX_COMMAND_LENGTH  20       // Update this length to hold your longest command

#define DCMOTOR_PWMPIN_1  5  // Motor Pins on Arduino NANO (must be PWM pins)
#define DCMOTOR_PWMPIN_2  6

//--- Globals ----------------------------------------------

bool  debugging = false;  // ((( Set to false for production builds )))

char        commandString[MAX_COMMAND_LENGTH] = "";  // Command from Serial client (Chrome browser web app)
int         commandLength = 0;
char        nextChar;
const char  *response;

DCMotor  *myMotor;

//--- Declarations ----------------------------------------

bool commandReady ();


//--- setup -----------------------------------------------

void setup()
{
  //Initialize serial
  Serial.begin (115200);
  while (!Serial);

  // Init motor
  myMotor = new DCMotor (DCMOTOR_PWMPIN_1, DCMOTOR_PWMPIN_2);  // No limit switches

  Serial.print ("Ready for commands ...");
}

//--- loop ------------------------------------------------

void loop()
{
  // Keep the motor running
  myMotor->Run ();

  // Check for any commands from UI app
  if (commandReady ())
  {
    response = myMotor->ExecuteCommand (commandString);

    // Show response, if any
    if (strlen (response) > 0)
      Serial.println (response);

    // Reset for a new command
    commandLength = 0;
  }
}

//--- commandReady ----------------------------------------

bool commandReady ()
{
  while (Serial.available ())
  {
    nextChar = (char) Serial.read ();

    if (nextChar != '\r')  // ignore CR's
    {
      if (nextChar == '\n')
      {
        // Command is ready, terminate it
        commandString[commandLength] = 0;
        return true;
      }

      // Add char to end of buffer
      if (commandLength < MAX_COMMAND_LENGTH - 1)
        commandString[commandLength++] = nextChar;
      else  // too long
      {
        Serial.println ("ERROR: Command is too long.");

        // Ignore and start new command
        commandLength = 0;
      }
    }
  }

  return false;
}
