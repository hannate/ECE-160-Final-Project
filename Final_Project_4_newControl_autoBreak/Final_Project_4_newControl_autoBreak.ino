#include <NewPing.h>
#include <Psx.h>
#include <Servo.h>
#include <SPI.h>

#define dataPin 7  //brown
#define cmndPin 6  //orange
#define atnPin 5   //yellow
#define clockPin 8 //blue
#define SONAR_NUM 3
#define PING_INTERVAL 33
#define MAX_DISTANCE 200

bool ps2ControllerMode = true; //true to use ps2 controller, false to use ir remote
int IRStep = 1000;

int prevGripperPressTime = 0;
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

bool btnLeft = false;
bool btnDown = false;
bool btnRight = false;
bool btnUp = false;
bool btnStart = false;
bool btnSelect = false;
bool btnSquare = false;
bool btnX = false;
bool btnO = false;
bool btnTriangle = false;
bool btnR1 = false;
bool btnR2 = false;
bool btnL1 = false;
bool btnL2 = false;
int error = 0;
byte type = 0;
byte vibrate = 0;
Servo wheelLeft;
Servo wheelRight;
Servo gripper;
Psx Psx;

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

  Psx.setupPins(dataPin, cmndPin, atnPin, clockPin, 10);

  lineLeftInit = analogRead(lineLeftPin);
  lineCenterInit = analogRead(lineCenterPin);
  lineRightInit = analogRead(lineRightPin);

  pingTimer[0] = millis() + 75;
  for (uint8_t i = 1; i < SONAR_NUM; i++) {
    pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;
  }

}

void loop() {
  readUltrasonicSensors();
  readController();
  if (btnSquare) {
    Serial.println("Square Pressed");
    chicken(1);
  } else if (btnX) {
    Serial.println("X Pressed");
    chicken(2);
  } else if (btnO) {
    Serial.println("Circle Pressed");
    chicken(0);
  }
  buttonControls();

  gripper.write(gripperPosition);

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
    readController();
    lineLeft = analogRead(lineLeftPin);
    lineCenter = analogRead(lineCenterPin);
    lineRight = analogRead(lineRightPin);

    if (btnSelect) {
      return;
    }
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
    readController();
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
    Serial.print(path);
    Serial.print(" ");
    Serial.println(cm[1]);
    if (btnSelect) {
      return;
    }
    if (path == 0 && cm[1] < 25 && cm[1] != 0) { //right
      wheelLeft.writeMicroseconds(1500);
      wheelRight.writeMicroseconds(1500);
      //Serial.println("LProx: ");
      //Serial.println(cm[1]);
      snoopy = false;
      break;
    } else if (path == 1 && cm[2] < 20 && cm[2] != 0) { //left
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
    readController();
    readUltrasonicSensors();
    lineLeft = analogRead(lineLeftPin);
    lineCenter = analogRead(lineCenterPin);
    lineRight = analogRead(lineRightPin);

    int distanceProx = 1000;
    distanceProx = cm[0];
    if (btnSelect) {
      return;
    }
    if (distanceProx < 5 && distanceProx != 0) {
      wheelLeft.writeMicroseconds(1500);
      wheelRight.writeMicroseconds(1500);
      gripperPosition = 0;
      loopy = false;
      return;
    } else if (lineLeft < lineLeftInit + 100) {
      wheelLeft.write(95);
      wheelRight.write(75);
      prevCorrect = 1;
    } else if (lineRight < lineRightInit + 100) {
      wheelLeft.write(125);
      wheelRight.write(90);
      prevCorrect = 2;
    } else if (lineCenter < lineCenterInit + 100) {
      wheelLeft.write(115);
      wheelRight.write(65);
    } else if (prevCorrect == 1) {
      wheelLeft.write(105);
      wheelRight.write(90);
    } else if (prevCorrect == 2) {
      wheelLeft.write(95);
      wheelRight.write(75);
    } else {
      wheelLeft.write(115);
      wheelRight.write(65);
    }
  }



}

void controller() {

  buttonControls();

  //gripper open/close
  if (btnTriangle) {
    if (gripperPosition == 0) {
      gripperPosition = 120;
    } else {
      gripperPosition = 0;
    }
  }

}
/*
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
*/
void buttonControls() {
  if (btnUp) {
    //Serial.println("Forward");
    wheelLeft.write(135);
    wheelRight.write(45);
  } else if (btnRight) {
    //Serial.println("Right");
    wheelLeft.write(135);
    wheelRight.write(90);
  } else if (btnLeft) {
    wheelLeft.write(95);
    wheelRight.write(45);
  } else if (btnDown) {
    wheelLeft.write(45);
    wheelRight.write(135);
  } else if (btnL2) {
    wheelLeft.write(45);
    wheelRight.write(45);
  }  else if (btnR2) {
    wheelLeft.write(135);
    wheelRight.write(135);
  } else {
    wheelLeft.writeMicroseconds(1500);
    wheelRight.writeMicroseconds(1500);
  }

  //gripper open/close
  if (btnTriangle) {
    if (gripperPosition == 0) {
      gripperPosition = 120;
    } else {
      gripperPosition = 0;
    }
    prevGripperPressTime = millis();
  }
}

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
          oneSensorCycle(); // Do something with results.
        sonar[currentSensor].timer_stop();
      currentSensor = i;
      cm[currentSensor] = 0;
      sonar[currentSensor].ping_timer(echoCheck);
    }
  }
}

void readController() {
  unsigned int data = 0;
  data = readData();
  Serial.print(data);
  Serial.print(" ");

  btnLeft = bitRead(data, 0);
  btnDown = bitRead(data, 1);
  btnRight = bitRead(data, 2);
  btnUp = bitRead(data, 3);
  btnStart = bitRead(data, 4);
  btnSelect = bitRead(data, 7);
  btnSquare = bitRead(data, 8);
  btnX = bitRead(data, 9);
  btnO = bitRead(data, 10);
  btnTriangle = bitRead(data, 11);
  btnR1 = bitRead(data, 12);
  btnL1 = bitRead(data, 13);
  btnR2 = bitRead(data, 14);
  btnL2 = bitRead(data, 15);
}

int readData() {
  unsigned int data = 0;
  data = Psx.read();
  delay(10);
  if (data != Psx.read()) {
    return readData();
  } else {
    return data;
  }
}
