#include "Arduino.h"
#include <SPI.h>
#include <RF24.h>

// Setting up pin variables
// Joystick connections
const int joyX = 2;
const int joyY = 3;
const int joyKey = 4;
// Variables to store joystick state
int analogX = 0;
int analogY = 0;
int joyKeyState = 0;

// Array to store transmitted message
int outMessage[3];

// Hardware configuration: Set up nRF24L01 radio on SPI bus (pins 10, 11, 12, 13) plus pins 9 & 8
RF24 radio(9, 8);

byte addresses[][6] = {"1Node", "2Node"};

// -----------------------------------------------------------------------------
// SETUP   SETUP   SETUP   SETUP   SETUP   SETUP   SETUP   SETUP   SETUP
// -----------------------------------------------------------------------------
void setup() {
  // Debug output
  Serial.begin(9600);
  Serial.println("TRANSMITTER");
  
  // Initializing joystick module
  pinMode(joyX, INPUT);
  pinMode(joyY, INPUT);
  pinMode(joyKey, INPUT);

  // Initiate the radio object
  radio.begin();
  radio.setAutoAck(false);

  // Set the transmit power to lowest available to prevent power supply related issues
  radio.setPALevel(RF24_PA_MIN);

  // Set the speed of the transmission to the quickest available
  radio.setDataRate(RF24_250KBPS);

  // Use a channel unlikely to be used by Wifi, Microwave ovens etc
  radio.setChannel(124);

  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);
}

// -----------------------------------------------------------------------------
// LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP
// -----------------------------------------------------------------------------
void loop() {
  // Mapping the analog input range of the dual variable resistors of the
  // joystick to a scale usable by the motors.
  analogX = map(analogRead(joyX), 512, 1023, 0, 255);
  analogY = map(analogRead(joyY), 512, 1023, 0, 255);
  // Checking whether the joystick button is pressed or not.
  joyKeyState = digitalRead(joyKey);

  Serial.println(analogX);
  Serial.println(analogY);
  Serial.println(joyKeyState);

  // Store the analog input and joystick button state in the message array
  outMessage[0] = analogX;
  outMessage[1] = analogY;
  outMessage[2] = joyKeyState;
  outMessage[3] = 0;
    
  // Ensure we have stopped listening (even if we're not) or we won't be able to transmit
  radio.stopListening(); 

  // Did we manage to SUCCESSFULLY transmit that (by getting an acknowledgement back from the other Arduino)?
  // Even we didn't we'll continue with the sketch, you never know, the radio fairies may help us
  if (!radio.write( outMessage, sizeof(outMessage) )) {
    Serial.println("No acknowledgement of transmission - receiving radio device connected?");    
  }

  // Now listen for a response
  radio.startListening();
  
  // But we won't listen for long, 200 milliseconds is enough
  unsigned long started_waiting_at = millis();

  // Loop here until we get indication that some data is ready for us to read (or we time out)
  while ( ! radio.available() ) {

    // Oh dear, no response received within our timescale
    if (millis() - started_waiting_at > 200 ) {
      Serial.println("No response received - timeout!");
      return;
    }
  }

  // Now read the data that is waiting for us in the nRF24L01's buffer
  unsigned char inMessage;
  radio.read( &inMessage, sizeof(unsigned char) );

  // Show user what we sent and what we got back
  Serial.print("Sent: ");
  Serial.print(*outMessage);
  Serial.print(", received: ");
  Serial.println(inMessage);

  // Try again 1s later
  //delay(1000);
}
