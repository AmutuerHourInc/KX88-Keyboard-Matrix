/*
Im using the keybed from a Yamaha kx88 keybaord connected to a Teensy 4.1
The keybed has 28 pins total
Note pins are configured as INPUT_PULLUP 
B and M pins are configured as OUTPUT
Note pins recieve signal from both B and M pins(but in opposite ways)
    The B and M pin circuits can never be completed at the same time
Note pins = row
B and Mpins = columns 
12 note pins correspinding to each note of the scale
8 BPins(B0-B7) corresponding with the unpressed state of a key
The Bpin's circuit also corresponds with each octave of the notes
    B0 = octave 0
    B1 = ocatves 1
    ...and so on
8 MPins(M0-M7) corresponding with the pressed state of a key
The Mpin's circuit also corresponds with each octave of the notes
    M0 = octave 0
    M1 = ocatves 1
    ...and so on
The keybed matrix sequence is as follows 
    Key unpressed - note pin reads HIGH
      B circuit is completed
    Key itinal press - note pin reads LOW
      B circuit is broken
    Key full press -  note pin reads HIGH
      M circuit is completed 
    Key released - note pin reads LOW
      M circuit is broken
      
Note pins(INPUTS)
 - D is Pin 23 of the teensy board
 - E is Pin 22 of the teensy board
 - F# is Pin 21 of the teensy board
 - G# is Pin 20 of the teensy board
 - A# is Pin 19 of the teensy board
 - C is Pin 18 of the teensy board
 - C# is Pin 2 of the teensy board
 - D# is Pin 3 of the teensy board
 - F is Pin 4 of the teensy board
 - G is Pin 5 of the teensy board
 - A is Pin 6 of the teensy board
 - B is Pin 7 of the teensy board
M Pins(KEYSTATE_DOWN)(OUTPUTS)
- M0 is Pin 8 of the teensy board
- M1 is Pin 9 of the teensy board
- M2 is Pin 10 of the teensy board
- M3 is Pin 11 of the teensy board
- M4 is Pin 12 of the teensy board
- M5 is Pin 24 of the teensy board
- M6 is Pin 25 of the teensy board
- M7 is Pin 26 of the teensy board
B Pins(KEYSTATE_UP)(OUTPUTS)
- B0 is Pin 17 of the teensy board
- B1 is Pin 16 of the teensy board
- B2 is Pin 15 of the teensy board
- B3 is Pin 14 of the teensy board
- B4 is Pin 13 of the teensy board
- B5 is Pin 41 of the teensy board
- B6 is Pin 40 of the teensy board
- B7 is Pin 39 of the teensy board

*/

// Currently a scan takes 5-6 micro seconds 
#include <Arduino.h>
#include <MIDI.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial8, MIDI); // 5 pin midi connector wired to tx pin(8) of my Teensy 4.1

const int numRows = 12;
const int numCols = 8;
const int rowPins[numRows] = {18, 2, 23, 3, 22, 4, 21, 5, 20, 6, 19, 7}; // Note Pins
const int colPins1[numCols] = {17, 16, 15, 14, 13, 41, 40, 39}; // B pins
const int colPins2[numCols] = {8, 9, 10, 11, 12, 24, 25, 26}; // M pins

const int noteMapping[numRows] = {36, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35};

enum KeyState { UNPRESSED, FLOATING, PRESSED, RELEASED };

struct Key {
  KeyState state;
  unsigned long floatStartTime;
};

Key keys[numCols][numRows];

// Function declarations
int calculateVelocity(unsigned long floatTime);
void handleKeyPress(int col, int row, int velocity);
void handleKeyRelease(int col, int row);

void setup() {
  // Serial.begin(9600);
  // while (!Serial); // Wait for Serial Monitor to open

  MIDI.begin(MIDI_CHANNEL_OMNI);

  for (int r = 0; r < numRows; r++) {
    pinMode(rowPins[r], INPUT_PULLUP);
  }

  for (int c = 0; c < numCols; c++) {
    pinMode(colPins1[c], OUTPUT);
    pinMode(colPins2[c], OUTPUT);
    digitalWrite(colPins1[c], LOW);
    digitalWrite(colPins2[c], LOW);
  }

  // Initialize all keys to UNPRESSED state
  for (int c = 0; c < numCols; c++) {
    for (int r = 0; r < numRows; r++) {
      keys[c][r].state = UNPRESSED;
    }
  }
}

void loop() {
  for (int c = 0; c < numCols; c++) {
    // Set all column pins LOW
    for (int i = 0; i < numCols; i++) {
      digitalWrite(colPins1[i], LOW);
    }

    // Set current column pin HIGH
    digitalWrite(colPins1[c], HIGH);

    delayMicroseconds(10);

    for (int r = 0; r < numRows; r++) {
      bool keyState = !digitalRead(rowPins[r]);

      switch (keys[c][r].state) {
        case UNPRESSED:
          if (!keyState) {
            keys[c][r].state = FLOATING;
            keys[c][r].floatStartTime = micros();
            digitalWrite(colPins2[c], HIGH); // Set corresponding column 2 pin HIGH
          }
          break;

        case FLOATING:
          if (keyState) {
            keys[c][r].state = PRESSED;
            unsigned long floatTime = micros() - keys[c][r].floatStartTime;
            int velocity = calculateVelocity(floatTime);
            handleKeyPress(c, r, velocity);
          }
          break;

        case PRESSED:
          if (!keyState) {
            keys[c][r].state = RELEASED;
            handleKeyRelease(c, r);
          }
          break;

        case RELEASED:
          if (!keyState) {
            keys[c][r].state = UNPRESSED;
          }
          break;
      }
    }

    delayMicroseconds(10); // Add a delay before setting the second column pins LOW
    digitalWrite(colPins2[c], LOW); // Set the second column pin LOW after scanning the rows
  }

/*
  // Print the key states to the Serial Monitor
  for (int c = 0; c < numCols; c++) {
    Serial.print("Octave: ");
    Serial.print(c);
    Serial.print(" - ");
    for (int r = 0; r < numRows; r++) {
      Serial.print(keys[c][r].state == PRESSED ? "1" : "0");
    }
    Serial.println();
  }
  
  Serial.println("------");

  // Optional delay to avoid flooding the Serial Monitor
  delay(1000);
  */
}

int calculateVelocity(unsigned long floatTime) {
  // Implement your velocity calculation based on the float time
  return 127 - floatTime / 100; // Example calculation
}

void handleKeyPress(int col, int row, int velocity) {
  int note = noteMapping[row] + (col * 12);

  // Handle the key press event for the specified key with the given velocity
  MIDI.sendNoteOn(note, velocity, 1);

  /*
  Serial.print("Key Pressed: Octave ");
  Serial.print(col);
  Serial.print(", Note ");
  Serial.print(row);
  Serial.print(", Velocity ");
  Serial.println(velocity);

  delay(1000);*/
}

void handleKeyRelease(int col, int row) {
  int note = noteMapping[row] + (col * 12);

  // Handle the key release event for the specified key
  MIDI.sendNoteOff(note, 0, 1);

  /*
  Serial.print("Key Released: Octave ");
  Serial.print(col);
  Serial.print(", Note ");
  Serial.println(row);

  delay(1000);*/

}
