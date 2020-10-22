/*
 * ECE 160 Final Project Receiver 
 *
 * Version 1.0
 * Author: Zak Estrada
 * Last updated: 2020-10-18
 *
 * The receiver uses an infrared (IR) phototransistor to detect the presence of
 * a BOE-BOT and lights a series of LEDs to indicate the pickup and dropoff
 * location for the ECE160 Fall 2020-2021 maze.  The pickup and dropoff are
 * selected using the Arduino software random number generator (RNG) library.
 *
 * Hardware: Arduino Uno, 4 white LEDs, 3 color LEDs (Red, Yellow, Blue), 
 *           IR Phototransistor (PT)
 *
 * Functions:
 *            numberTest() - tests the wiring of the white LEDs.
 *            colorTest() - tests the wiring of the color LEDs.
 *            lightNumber() - shows pickup location number via white LEDs.
 *            lightColor() - shows dropoff location via color LEDs.
 *            PT_Hit() - when PT is activated, PT_Hit selects the stations and 
 *                       lights corresponding LEDs
 *
 */

/*
 * Configuration Parameters - may need to change based on ambient conditions
 *
 * PT_SENS is the sensitivity of the phototransistor. Values below PT_SENS will
 * indicate the robot is present. May need to be calibrated to a room, 
 * fluorescent lights notoriously give off IR.  *
 * 
 * PAUSE_TIME is the amount of time to wait before giving new instructions.  
 * This gives the robot a chance to get out of the way without changing
 * the objective
 *
 */
const int PT_SENS = 700; 
const int PAUSE_TIME = 10000; 

//Pins for the delivery station - red, yellow, blue
const int STATION_PINS[] = {9, 6, 3};

//Pins for the pickup station number - white LEDs
const int NUMBER_PINS[] = {10, 11, 12, 13};

//Phototransistor pin
const int PT_PIN = A5;

void setup() {
	Serial.begin(9600);

  //Set up LED pins as output and perform start-up tests
  for(int i=0; i<3; i++) {
    pinMode(STATION_PINS[i],OUTPUT);
  }
  colorTest();
  for(int i=0; i<4; i++) {
    pinMode(NUMBER_PINS[i], OUTPUT);
  }
  numberTest();

  //Set up the PT as input
  pinMode(PT_PIN, INPUT);

  //Set up the RNG
  randomSeed(analogRead(0));
}

//Read the PT repeatedly until we get a hit.
void loop() {
  int PT_value;
  PT_value = analogRead(PT_PIN);
  //We output the value read from the PT pin in case you need to calibrate
  Serial.println("PT Value: "+String(PT_value));
  if(PT_value < PT_SENS) {
    //We have interacted with a robot, choose stations and light LED sequences
    Serial.println("PT hit!");
    PT_Hit();
  } 
}

/* 
 * Tests the wiring of the white LEDs by lighting them in sequence 
 *
 * INPUTS: None
 * OUTPUTS: None
 *
 */
void numberTest() {
  for(int i=1; i<=4; i++) {
    Serial.println("Lighting "+String(i)+" LEDs for pickup station");
    lightNumber(i);
    delay(250); 
  }
  lightNumber(0);
}


/* 
 * Tests the wiring of the color LEDs by lighting them in sequence 
 *
 * INPUTS: None
 * OUTPUTS: None
 *
 */
void colorTest() {
  for(int i=0; i<4; i++) {
    Serial.println("Lighting color for dropoff station "+String(i));
    lightColor(i);
    delay(250);
  }
  lightColor(0);
}

/* 
 * Light up the number of white LEDs corresponding to the pickup station for an
 * object.
 *
 * INPUTS: 
 *          num - number identifying pickup station (1-4). 0 turns off all LEDs
 * OUTPUTS: None
 *
 */
void lightNumber(int num) {
    for(int i=0; i<4; i++) {
      if(i<num) {
        digitalWrite(NUMBER_PINS[i], HIGH);
      } else {
        digitalWrite(NUMBER_PINS[i], LOW);
      }
    }
}

/* 
 * Light up the color indicating which delivery station to deliver object to.
 *
 * INPUTS: 
 *          num - number identifying dropoff station (1-3). 0 turns off all LEDs
 * OUTPUTS: None
 *
 */
void lightColor(int station) {
  for(int i=0; i<3; i++) {
    if((station-1) == i) {
      digitalWrite(STATION_PINS[i], HIGH);
    } else {
      digitalWrite(STATION_PINS[i], LOW);
    } 
  }
}

/* 
 * When PT is activated, PT_Hit uses the RNG to select pickup and dropoff 
 * locations.  PT_Hit uses the other helper functions to light the appropriate 
 * LEDs and blocks execution until timeout is complete.
 *
 * INPUTS: None
 * OUTPUTS: None
 *
 */
void PT_Hit() {
  int pickup = random(1,4); //random(x,y) samples a random number from [x,y]
  int delivery = random(1,3);
  Serial.println("Pickup: "+String(pickup)+", Dropoff: "+ String(delivery)); 
  lightNumber(pickup);
  lightColor(delivery);
  delay(PAUSE_TIME); //ensure we do not double-detect robots
}
