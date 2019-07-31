#include <AccelStepper.h>

//These variables depend on your pinout, mine is for the CNC shield with drivers
#define pinStepperEnable 8
#define pinDirectionX 5
#define pinDirectionY 6
#define pinStepPulseX 2
#define pinStepPulseY 3

#define motorIntefaceType 1//1 is for external drivers such as DRV8825 and A4988

//Expected maximum size of command inputs sent from a computer
#define INPUT_SIZE 30

AccelStepper xStepper(motorIntefaceType, pinStepPulseX, pinDirectionX);
AccelStepper yStepper(motorIntefaceType, pinStepPulseY, pinDirectionY);

byte isXDone = true;//flags to let computer know if it's done
byte isYDone = true;

void setup() {
  Serial.begin(9600);
  Serial.println("Starting Serial Monitor");



  pinMode(pinStepperEnable, OUTPUT);//by default is pulled high to disable motors
  digitalWrite(pinStepperEnable, LOW);//pulling low means they will now respond

  pinMode(pinDirectionX, OUTPUT);
  pinMode(pinDirectionY, OUTPUT);
  pinMode(pinStepPulseX, OUTPUT);
  pinMode(pinStepPulseY, OUTPUT);

  xStepper.setMaxSpeed(1000);//sets the maximum allowed speed for the steppers
  yStepper.setMaxSpeed(1000);//speed is measured in steps per second
  xStepper.setSpeed(600);//ie. 32x microstepping can bring a full revolution up to 6400 steps
  yStepper.setSpeed(600);//so plan the max speed accordingly to your motor's datasheet
}

void loop() {
  while (Serial.available() > 0)
  {
    char input[INPUT_SIZE + 1];
    byte size = Serial.readBytes(input, INPUT_SIZE);
    input[size] = 0;

    //It will expect a command formatted like "1:+90&2:-80"
    /*
       commandID:commandValueOne&commandID:commandValueOne - can be chained

       1 - Xmove - set X steps to move with constant speed
       2 - Ymove - set Y steps to move with constant speed

       5 - check if the current operation is done

       11 - set current X position to value
       12 - set current Y position to value
       13 - get current X position
       14 - get current Y position
       15 - stop X
       16 - stop Y
       17 - disable X WIP
       18 - disable Y WIP
    */

    char* command = strtok(input, "&");
    while (command != 0)
    {
      char* separator = strchr(command, ':');
      if (separator != 0)
      {
        *separator = 0;
        int commandID = atoi(command);
        ++separator;

        //second value extraction
        char* separator_two = strchr(separator, ':');
        ++separator_two;
        int commandValueTwo = atoi(separator_two);
        int commandValueOne = atoi(separator);

        switch (commandID) {
          case 1 :
            xStepper.move(commandValueOne);
            xStepper.setSpeed(commandValueTwo);
            xStepper.runSpeedToPosition();
            Serial.print("Move X by: ["); Serial.print(commandValueOne);
            Serial.print("] at speed: ["); Serial.print(commandValueTwo); Serial.println("]");
            break;
          case 2 :
            yStepper.move(commandValueOne);
            yStepper.setSpeed(commandValueTwo);
            yStepper.runSpeedToPosition();
            Serial.print("Move Y by: "); Serial.print(commandValueOne);
            Serial.print(" at speed: ["); Serial.print(commandValueTwo); Serial.println("]");
            break;
          case 5 :
            if (isXDone && isYDone == true) {
              Serial.println("ok");
            }
            else {
              Serial.println("not_now");
            }
            break;
          case 11 :
            xStepper.setCurrentPosition(commandValueOne);
            Serial.print("Set X position at: "); Serial.println(commandValueOne);
            break;
          case 12 :
            yStepper.setCurrentPosition(commandValueOne);
            Serial.print("Set Y position at: "); Serial.println(commandValueOne);
            break;
          case 13 :
            Serial.println(xStepper.currentPosition());
            break;
          case 14 :
            Serial.println(yStepper.currentPosition());
            break;
          case 15 :
            xStepper.stop();
            Serial.println("Stop X");
            break;
          case 16 :
            yStepper.stop();
            Serial.println("Stop Y");
            break;
          case 17 :
            xStepper.disableOutputs();
            Serial.println("Disable X");
            break;
          case 18 :
            yStepper.disableOutputs();
            Serial.println("Disable Y");
            break;
          case 19 :
            xStepper.enableOutputs();
            Serial.println("Enable X");
            break;
          case 20 :
            yStepper.enableOutputs();
            Serial.println("Enable Y");
            break;
          default :
            Serial.print("Default case triggered on commandID: ");
            Serial.println(commandID);
        }
        isXDone = false;
        isYDone = false;
      }
      command = strtok(0, "&");
    }
  }

  /*after the "set" part we can now check if we need to move our steppers and then move them
    the accelstepper library suggests that we call run() as often as possible (atleast faster
    than the set speed of the steppers)*/

  if (xStepper.distanceToGo() != 0) {
    xStepper.runSpeed();
  }
  else {
    isXDone = true;
  }
  if (yStepper.distanceToGo() != 0) {
    yStepper.runSpeed();
  }
  else {
    isYDone = true;
  }
}

void status_update(int commandID, int commandValueOne, int commandValueTwo) {
  Serial.print("ID= "); Serial.println(commandID);
  Serial.print("ValueOne= "); Serial.println(commandValueOne);
  Serial.print("ValueTwo= "); Serial.println(commandValueTwo);
}
