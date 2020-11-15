#include <NewPing.h>



//#include <SR04.h>

//#include <EIRremote.h>
//#include <EIRremoteInt.h>

#include <PS2X_lib.h>  //for v1.6
#include <Servo.h>
#include <SPI.h>
//#include <NRFLite.h>

#define SONAR_NUM 3
#define PING_INTERVAL 33
#define MAX_DISTANCE 200

bool ps2ControllerMode = true; //true to use ps2 controller, false to use ir remote
int IRStep = 1000;

int triggerPin = 3;
int echoPin = 2;
int rightTriggerPin = 4;
int rightEchoPin = 4;
int leftTriggerPin = 9;
int leftEchoPin = 9;
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
//IRrecv myIR(IRPin);
//decode_results results;
PS2X ps2x; // create PS2 Controller Class

unsigned long pingTimer[SONAR_NUM];
unsigned int cm[SONAR_NUM];
uint8_t currentSensor = 0;

NewPing sonar[3] = { //1 is forward; 2 is right; 3 is left
  NewPing(triggerPin, echoPin, MAX_DISTANCE),
  NewPing(rightEchoPin, rightTriggerPin, MAX_DISTANCE),
  NewPing(leftEchoPin, leftTriggerPin, MAX_DISTANCE)
};
bool joystickMode = false;
int gripperPosition = 0;




void setup() {
  Serial.begin(9600);
  pinMode(leftEchoPin, INPUT);
  pinMode(leftTriggerPin, INPUT);
  wheelLeft.attach(13);
  wheelRight.attach(12);
  gripper.attach(11);
  pinMode(IRPin, INPUT);
  //myIR.enableIRIn();

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

  pingTimer[0] = millis() + 75;
  for (uint8_t i = 1; i < SONAR_NUM; i++) {
    pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;
  }


}

void loop() {
  readUltrasonicSensors();
  if (millis() > 1000) {

    if (ps2x.Button(PSB_L1)) {
      Serial.println("Square Pressed");
      chicken(1);
    } else if (ps2x.Button(PSB_R1)) {
      Serial.println("X Pressed");
      chicken(2);
    } else if (ps2x.Button(PSB_RED)) {
      Serial.println("Circle Pressed");
      chicken(0);
    }
    controller();
  }
  gripper.write(gripperPosition);

  //delay(50);

}

void chicken(int path) {
  Serial.println("Entering Auto");
  int prevCorrect = 0;
  bool loopy = true;
  lineLeftInit = analogRead(lineLeftPin);
  lineCenterInit = analogRead(lineCenterPin);
  lineRightInit = analogRead(lineRightPin);
  wheelLeft.write(100);
  wheelRight.write(80);
  delay(500);
  while (loopy) {
    lineLeft = analogRead(lineLeftPin);
    lineCenter = analogRead(lineCenterPin);
    lineRight = analogRead(lineRightPin);
    //Serial.print(lineLeft);
    //Serial.print(" ");
    //Serial.print(lineCenter);
    //Serial.print(" ");
    //Serial.println(lineRight);

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
      wheelLeft.write(100);
      wheelRight.write(80);
    }
  }
  Serial.print("Turning ");
  //Serial.println("i did a lopp");
  bool snoopy = false;
  if (path == 0) {
    wheelLeft.write(135);
    wheelRight.write(135);
    Serial.println("Right");
    snoopy = true;
    delay(500);
  } else if (path == 1) {
    wheelLeft.write(45);
    wheelRight.write(45);
    snoopy = true;
    delay(500);
    Serial.println("Left");
  } else if (path == 2) {
    wheelLeft.write(100);
    wheelRight.write(80);
    delay(100);
    Serial.println("Straight");
  }

  while (snoopy) {
    readUltrasonicSensors();
    lineLeft = analogRead(lineLeftPin);
    lineCenter = analogRead(lineCenterPin);
    lineRight = analogRead(lineRightPin);

    /*if (lineLeft < lineLeftInit + 100 && lineRight < lineRightInit + 100) {
      wheelLeft.writeMicroseconds(1500);
      wheelRight.writeMicroseconds(1500);

      snoopy = false;
      break;
      } else if (lineLeft > lineLeftInit + 100 && lineRight > lineRightInit + 100 && lineCenter < lineCenterInit + 100) {
      wheelLeft.writeMicroseconds(1500);
      wheelRight.writeMicroseconds(1500);

      snoopy = false;
      break;
      }*/
    if (path == 0 && cm[1] < 25 && cm[1] != 0) {
      wheelLeft.writeMicroseconds(1500);
      wheelRight.writeMicroseconds(1500);
      //Serial.println("LProx: ");
      //Serial.println(cm[1]);
      snoopy = false;
      break;
    } else if (path == 1 && cm[2] < 25 && cm[2] != 0) {
      wheelLeft.writeMicroseconds(1500);
      wheelRight.writeMicroseconds(1500);
      //Serial.print("RProx: ");
      //Serial.print(cm[2]);
      snoopy = false;
      break;
    }
  }
  Serial.println("Advancing");
  loopy = true;
  while (loopy) {
    readUltrasonicSensors();
    lineLeft = analogRead(lineLeftPin);
    lineCenter = analogRead(lineCenterPin);
    lineRight = analogRead(lineRightPin);

    int distanceProx = 1000;
    distanceProx = cm[0];
    //Serial.println(distanceProx);
    if (distanceProx < 10 && distanceProx != 0) {
      wheelLeft.writeMicroseconds(1500);
      wheelRight.writeMicroseconds(1500);
      gripperPosition = 0;
      loopy = false;
      delay(2000);
      return;
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
/*
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
*/
void echoCheck() { // If ping echo, set distance to array.
  if (sonar[currentSensor].check_timer())
    cm[currentSensor] = sonar[currentSensor].ping_result / US_ROUNDTRIP_CM;
}

void oneSensorCycle() { // Do something with the results.
  for (uint8_t i = 0; i < SONAR_NUM; i++) {
    Serial.print(i);
    Serial.print("=");
    Serial.print(cm[i]);
    Serial.print("cm ");
  }
  Serial.println();
}

void readUltrasonicSensors() {
  for (uint8_t i = 0; i < SONAR_NUM; i++) {
    if (millis() >= pingTimer[i]) {
      pingTimer[i] += PING_INTERVAL * SONAR_NUM;
      if (i == 0 && currentSensor == SONAR_NUM - 1)
        //  oneSensorCycle(); // Do something with results.
        sonar[currentSensor].timer_stop();
      currentSensor = i;
      cm[currentSensor] = 0;
      sonar[currentSensor].ping_timer(echoCheck);
    }
  }
}
