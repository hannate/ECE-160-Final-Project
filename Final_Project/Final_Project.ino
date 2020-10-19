#include <PS2X_lib.h>  //for v1.6
#include <Servo.h>


PS2X ps2x; // create PS2 Controller Class

//right now, the library does NOT support hot pluggable controllers, meaning
//you must always either restart your Arduino after you conect the controller,
//or call config_gamepad(pins) again after connecting the controller.
int error = 0;
byte type = 0;
byte vibrate = 0;
Servo wheelLeft;
Servo wheelRight;
Servo gripper;
bool joystickMode = false;
int gripperPosition = 0;
void setup() {
  Serial.begin(9600);
  wheelLeft.attach(13);
  wheelRight.attach(12);
  gripper.attach(11);

  //CHANGES for v1.6 HERE!!! **************PAY ATTENTION*************

  error = ps2x.config_gamepad(8, 6, 5, 7, true, true); //setup pins and settings:  GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error

  if (error == 0) {
    Serial.println("Found Controller, configured successful");
    Serial.println("Try out all the buttons, X will vibrate the controller, faster as you press harder;");
    Serial.println("holding L1 or R1 will print out the analog stick values.");
    Serial.println("Go to www.billporter.info for updates and to report bugs.");
  }

  else if (error == 1)
    Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");

  else if (error == 2)
    Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");

  else if (error == 3)
    Serial.println("Controller refusing to enter Pressures mode, may not support it. ");

  //Serial.print(ps2x.Analog(1), HEX);

  type = ps2x.readType();
  switch (type) {
    case 0:
      Serial.println("Unknown Controller type");
      break;
    case 1:
      Serial.println("DualShock Controller Found");
      break;
    case 2:
      Serial.println("GuitarHero Controller Found");
      break;
  }

}

void loop() {

  if (error == 1) //skip loop if no controller found
    return;

  if (type == 2) { //Guitar Hero Controller
    //Do nothing
  }

  else { //DualShock Controller

    ps2x.read_gamepad(false, vibrate);          //read controller and set small motor off and large motor to spin at 'vibrate' speed

    //Change between button mode and joystick mode
    if (ps2x.ButtonPressed(PSB_SELECT)) {
      joystickMode = !joystickMode;
    }

    //Move robot based on selected control scheme
    if (joystickMode == true) {
      joystickControls();
    } else {
      buttonControls();
    }

    //gripper open/close
    if (ps2x.ButtonPressed(PSB_GREEN)) {
      if (gripperPosition == 0) {
        gripperPosition = 120;
      } else {
        gripperPosition = 0;
      }
    }
    gripper.write(gripperPosition);
    
  }


  delay(50);

}

void joystickControls() {
  int leftStickYAxis = ps2x.Analog(PSS_LY);
      int leftStickXAxis = ps2x.Analog(PSS_LX);
      int speedLeft = map(leftStickYAxis, 0, 255, 135, 45);
      Serial.print(speedLeft);
      Serial.print(" ");
      speedLeft = speedLeft + map(leftStickXAxis, 0, 255, -45, 45);
      int speedRight = map(leftStickYAxis, 0, 255, 45, 135);
      Serial.print(speedRight);
      Serial.print(" ");
      speedRight = speedRight + map(leftStickXAxis, 0, 255, -45, 45);
      Serial.print(speedLeft);
      Serial.print(" ");
      Serial.println(speedRight);
      if (speedLeft <= 95 && speedLeft >= 85 && speedRight <= 95 && speedRight >= 85) {
        wheelLeft.writeMicroseconds(1500);
        wheelRight.writeMicroseconds(1500);
      } else {
        wheelLeft.write(speedLeft);
        wheelRight.write(speedRight);
      }
}

void buttonControls() {
  if (ps2x.Button(PSB_PAD_UP)) {        //will be TRUE as long as button is pressed
        Serial.print("Up held this hard: ");
        Serial.println(ps2x.Analog(PSAB_PAD_UP), DEC);
        wheelLeft.write(135);
        wheelRight.write(45);
      } else if (ps2x.Button(PSB_PAD_RIGHT)) {
        Serial.print("Right held this hard: ");
        Serial.println(ps2x.Analog(PSAB_PAD_RIGHT), DEC);
        wheelLeft.write(135);
        wheelRight.write(90);
      } else if (ps2x.Button(PSB_PAD_LEFT)) {
        Serial.print("LEFT held this hard: ");
        Serial.println(ps2x.Analog(PSAB_PAD_LEFT), DEC);
        wheelLeft.write(95);
        wheelRight.write(45);
      } else if (ps2x.Button(PSB_PAD_DOWN)) {
        Serial.print("DOWN held this hard: ");
        Serial.println(ps2x.Analog(PSAB_PAD_DOWN), DEC);
        wheelLeft.write(45);
        wheelRight.write(135);
      } else if (ps2x.Button(PSB_L2)) {
        wheelLeft.write(45);
        wheelRight.write(45);
      }  else if (ps2x.Button(PSB_R2)) {
        wheelLeft.write(135);
        wheelRight.write(135);
      } else {
        wheelLeft.writeMicroseconds(1500);
        wheelRight.writeMicroseconds(1500);
      }
}
