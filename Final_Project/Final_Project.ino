

#include <SR04.h>

#include <EIRremote.h>
#include <EIRremoteInt.h>

#include <PS2X_lib.h>  //for v1.6
#include <Servo.h>
#include <SPI.h>
#include <NRFLite.h>


bool ps2ControllerMode = true; //true to use ps2 controller, false to use ir remote
int IRStep = 1000;

int triggerPin = 3;
int echoPin = 2;
int IRPin = 10;
int lineLeftPin = A0;
int lineCenterPin = A2;
int lineRightPin = A1;
int lineLeft = 0;
int lineRight = 0;
int lineCenter = 0;
int lineLeftInit = 0;
int lineRightInit = 0;
int lineCenterInit = 0;
int error = 0;
byte type = 0;
byte vibrate = 0;
Servo wheelLeft;
Servo wheelRight;
Servo gripper;
IRrecv myIR(IRPin);
decode_results results;
PS2X ps2x; // create PS2 Controller Class
SR04 prox = SR04(echoPin, triggerPin);
bool joystickMode = false;
int gripperPosition = 0;




void setup() {
  Serial.begin(9600);
  wheelLeft.attach(13);
  wheelRight.attach(12);
  gripper.attach(11);
  pinMode(IRPin, INPUT);
  myIR.enableIRIn();

  lineLeftInit = analogRead(lineLeftPin);
  lineCenterInit = analogRead(lineCenterPin);
  lineRightInit = analogRead(lineRightPin);

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


  if (ps2ControllerMode == true) { //control with ps2 controller
    if (ps2x.ButtonPressed(PSB_PINK)) {
      Serial.println("1");
      chicken(1);
    } else if (ps2x.ButtonPressed(PSB_BLUE)) {
      Serial.println("2");
      chicken(2);
    } else if (ps2x.ButtonPressed(PSB_RED)) {
      Serial.println("3");
      chicken(0);
    }
    controller();
  } else { //control with IR remote

    if (myIR.decode(&results)) {
      Serial.println("ir");
      Serial.println(results.value, HEX);
      translateIR();
      myIR.resume();
    }

  }

  gripper.write(gripperPosition);

  delay(50);

}

void chicken(int path) {
  int prevCorrect = 0;
  bool loopy = true;
  wheelLeft.write(135);
  wheelRight.write(45);
  delay(500);
  while (loopy) {
    lineLeft = analogRead(lineLeftPin);
    lineCenter = analogRead(lineCenterPin);
    lineRight = analogRead(lineRightPin);
    Serial.print(lineLeft);
    Serial.print(" ");
    Serial.print(lineCenter);
    Serial.print(" ");
    Serial.println(lineRight);

    if (lineLeft < lineLeftInit + 100 && lineRight < lineRightInit + 100) {
      wheelLeft.writeMicroseconds(1500);
      wheelRight.writeMicroseconds(1500);

      loopy = false;
      break;
    } else if (lineLeft < lineLeftInit + 100) {
      wheelLeft.write(95);
      wheelRight.write(45);
    } else if (lineRight < lineRightInit + 100) {
      wheelLeft.write(135);
      wheelRight.write(90);
    } else {
      wheelLeft.write(135);
      wheelRight.write(45);
    }
  }
  Serial.println("i did a lopp");
  if (path == 0) {
    wheelLeft.write(135);
    wheelRight.write(135);
    delay(500);
  } else if (path == 1) {
    wheelLeft.write(45);
    wheelRight.write(45);
    delay(500);
  } else if (path == 2) {

  }
  bool snoopy = true;
  while (snoopy) {
    lineLeft = analogRead(lineLeftPin);
    lineCenter = analogRead(lineCenterPin);
    lineRight = analogRead(lineRightPin);
    Serial.print(lineLeft);
    Serial.print(" ");
    Serial.print(lineCenter);
    Serial.print(" ");
    Serial.println(lineRight);

    if (lineLeft < lineLeftInit + 100 && lineRight < lineRightInit + 100) {
      wheelLeft.writeMicroseconds(1500);
      wheelRight.writeMicroseconds(1500);

      snoopy = false;
      break;
    }
  }
  loopy = true;
  while (loopy) {
    lineLeft = analogRead(lineLeftPin);
    lineCenter = analogRead(lineCenterPin);
    lineRight = analogRead(lineRightPin);
    Serial.print(lineLeft);
    Serial.print(" ");
    Serial.print(lineCenter);
    Serial.print(" ");
    Serial.println(lineRight);

    int distanceProx = 1000;
    distanceProx = prox.Distance();
    Serial.println(distanceProx);
    if (distanceProx < 20 && distanceProx !=0) {
      wheelLeft.writeMicroseconds(1500);
      wheelRight.writeMicroseconds(1500);
      gripperPosition = 0;
      loopy = false;
      break;
    } else if (lineLeft < lineLeftInit + 100) {
      wheelLeft.write(95);
      wheelRight.write(65);
      prevCorrect = 1;
    } else if (lineRight < lineRightInit + 100) {
      wheelLeft.write(115);
      wheelRight.write(90);
      prevCorrect = 2;
    } else if (lineCenter < lineCenterInit + 100) {
      wheelLeft.write(115);
      wheelRight.write(65);
    } else if (prevCorrect == 1) {
      wheelLeft.write(115);
      wheelRight.write(90);
    } else if (prevCorrect == 2) {
      wheelLeft.write(95);
      wheelRight.write(65);
    } else {
      wheelLeft.write(115);
      wheelRight.write(65);
    }
  }



}

void controller() {
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
      Serial.println("Changing control scheme");
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


  }
}

void joystickControls() {
  int leftStickYAxis = ps2x.Analog(PSS_RY);
  int leftStickXAxis = ps2x.Analog(PSS_RX);
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

void translateIR() {                        // takes action based on IR code received
  switch (results.value) {
    case 0xFFA25D:
      Serial.println("POWER");
      break;
    case 0xFFE21D:
      Serial.println("FUNC/STOP");
      break;
    case 0xFF629D:
      Serial.println("VOL+");

      break;
    case 0xFF22DD:
      Serial.println("REWIND");
      break;
    case 0xFF02FD:
      Serial.println("PLAY/PAUSE");
      break;
    case 0xFFC23D:
      Serial.println("FAST FORWARD");
      break;
    case 0xFFE01F:
      Serial.println("DOWN");
      break;
    case 0xFFA857:
      Serial.println("VOL-");
      if (gripperPosition == 0) {
        gripperPosition = 120;
      } else {
        gripperPosition = 0;
      }
      Serial.println(gripperPosition);
      break;
    case 0xFF906F:
      Serial.println("UP");
      break;
    case 0xFF9867:
      Serial.println("EQ");
      break;
    case 0xFFB04F:
      Serial.println("ST/REPT");
      break;
    case 0xFF6897:
      Serial.println("0");
      break;
    case 0xFF30CF:
      Serial.println("1");
      wheelLeft.write(45);
      wheelRight.write(45);
      delay(IRStep);
      wheelLeft.writeMicroseconds(1500);
      wheelRight.writeMicroseconds(1500);
      break;
    case 0xFF18E7:
      Serial.println("2");
      wheelLeft.write(135);
      wheelRight.write(45);
      delay(IRStep);
      wheelLeft.writeMicroseconds(1500);
      wheelRight.writeMicroseconds(1500);
      break;
    case 0xFF7A85:
      Serial.println("3");
      wheelLeft.write(135);
      wheelRight.write(135);
      delay(IRStep);
      wheelLeft.writeMicroseconds(1500);
      wheelRight.writeMicroseconds(1500);
      break;
    case 0xFF10EF:
      Serial.println("4");
      wheelLeft.write(95);
      wheelRight.write(45);
      delay(IRStep);
      wheelLeft.writeMicroseconds(1500);
      wheelRight.writeMicroseconds(1500);
      break;
    case 0xFF38C7:
      Serial.println("5");
      break;
    case 0xFF5AA5:
      Serial.println("6");
      wheelLeft.write(135);
      wheelRight.write(90);
      delay(IRStep);
      wheelLeft.writeMicroseconds(1500);
      wheelRight.writeMicroseconds(1500);
      break;
    case 0xFF42BD:
      Serial.println("7");
      break;
    case 0xFF4AB5:
      Serial.println("8");
      wheelLeft.write(45);
      wheelRight.write(135);
      delay(IRStep);
      wheelLeft.writeMicroseconds(1500);
      wheelRight.writeMicroseconds(1500);
      break;
    case 0xFF52AD:
      Serial.println("9");
      break;
    case 0xFFFFFFFF:
      Serial.println(" REPEAT");
      break;
    default:
      Serial.println("other button");
      break;
  }
  delay(500);
}
