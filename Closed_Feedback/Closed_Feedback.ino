#include <SR04.h>

int motorPin = 11;
int redLED = 7;
int greenLED = 6;
int triggerPin = 3;
int echoPin = 2;

SR04 mySonar = SR04(echoPin, triggerPin);

long distance;
int stopDistance = 5;
int difference;
int motorSpeed;

void setup() {
  // put your setup code here, to run once:
  pinMode(motorPin, OUTPUT);
  pinMode(triggerPin, OUTPUT);  // Trigger is an output pin
  pinMode(echoPin, INPUT);      // Echo is an input pin

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  distance = mySonar.Distance();

  difference = (abs)(distance - stopDistance);  // how close are we?
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print("\t Difference: ");
  Serial.println(difference);
  motorSpeed = difference * 5;  // amplify the difference
  motorSpeed = map(motorSpeed, 0, 400, 0, 255); //keep it within the DC motor range
  motorSpeed = constrain(motorSpeed, 0, 255);
  analogWrite(motorPin, motorSpeed);  // turn on the motor proportionately
  delay(500);
}
