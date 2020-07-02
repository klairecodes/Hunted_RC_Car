#include "Arduino.h"
#include <SPI.h>
#include <RF24.h>
#include <math.h>

// Setting up pin variables
// Motor A connections
const int enA = 10;
const int in1 = 47;
const int in2 = 45;
// Motor B connections
const int enB = 11;
const int in3 = 43;
const int in4 = 41;
// Variables to store received analog values
int analogX = 0;
int analogY = 0;
int joyKeyState = 0;
// Buzzer (horn) pin
const int buzzer = 37;

// Array to store received message
int inMessage[3];

// -----------------------------------------------------------------------------
// RADIO VALUES
// -----------------------------------------------------------------------------

// This is just the way the RF24 library works:
// Hardware configuration: Set up nRF24L01 radio on SPI bus (pins 10, 11, 12, 13) plus pins 9 & 8
RF24 radio(9, 8);

byte addresses[][6] = {"1Node","2Node"};


// -----------------------------------------------------------------------------
// SETUP   SETUP   SETUP   SETUP   SETUP   SETUP   SETUP   SETUP   SETUP
// -----------------------------------------------------------------------------
void setup() {
  // Set all the motor control pins to outputs
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  // Initializing buzzer
  pinMode(buzzer, OUTPUT);
 
  // Turn off motors - Initial state
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  
  Serial.begin(9600);
  Serial.println("Starting receiver...");

  // Initiate the radio object
  radio.begin();

  // Set auto acknowledgement to false for improperly made Chinese nRF24 modules
  radio.setAutoAck(false);

  // MIN power for testing of the protocol
  radio.setPALevel(RF24_PA_MIN);

  // Slow data transmission rate for less packet loss over long distances
  radio.setDataRate(RF24_250KBPS);

  // Use a channel unlikely to be used by Wifi, Microwave ovens etc
  radio.setChannel(124);

  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1, addresses[1]);

  // Start the radio listening for data
  radio.startListening();
}

// -----------------------------------------------------------------------------
// We are LISTENING on this device only (although we do transmit a response)
// -----------------------------------------------------------------------------
void loop() {
  // Is there any data for us to get?
  if ( radio.available()) {

    // Read the data and store it into a variable
    while (radio.available()) {
      radio.read( &inMessage, sizeof(inMessage));
    }
    analogX = inMessage[0];
    analogY = inMessage[1];
    joyKeyState = inMessage[2];

    // First, stop listening so we can talk
    radio.stopListening();
    
    direction();
    turn();
    honk();
    
    radio.write( &inMessage, sizeof(inMessage) );

    // Now, resume listening so we catch the next packets.
    radio.startListening();

    // Tell the user what we sent back (the random numer + 1)
    Serial.println("Sent response: ");
    Serial.println(analogX);
    Serial.println(analogY);
    Serial.println(joyKeyState);
  }
}

// determines what direction the motors should be turning
void direction() {
  // move forwards
  if (analogY > 0) {
    analogWrite(enA, analogY);
    analogWrite(enB, analogY);
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);;
  }
  // move backwards
  if (analogY < 0) {
    analogWrite(enA, analogY-5); // minus 5 because of mechanically blocked joystick -y
    analogWrite(enB, analogY-5);
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
  }
}

// determines how fast each individual motor should spin to facilitate tank turning
void turn() {
  if (analogX > 10) {
    analogWrite(enA, 255);
    analogWrite(enB, 0);
    Serial.println("RIGHT");
  }
  if (analogX < -10) {
    analogWrite(enA, 0);
    analogWrite(enB, 255);
    Serial.println("LEFT");
  }
  motorReset();
}

// turns all of the motors off
void motorReset() {
  delay(100);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

// honks the car's horn (a piezo buzzer)
void honk() {
  if (joyKeyState == LOW) {
    Serial.println("buzz");
    tone(buzzer, 1000);
  } else if (joyKeyState == HIGH) {
    noTone(buzzer);
  }
}
