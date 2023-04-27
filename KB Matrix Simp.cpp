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


This code currently isn't measuring velocity, just trying to get proper scanning and output first
*/




#include <Arduino.h>
#include <MIDI.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial8, MIDI); // 5 pin midi connector wired to tx pin(8) of my Teensy 4.1

struct Key {
  byte pin; // 12 pins, setup as INPUTS_PULLUP, each pin corresponds to one note of the scale. Read to determine the key.state
  byte mPin; // 8 pins, m0-m7, each pin corresponds with the each octave, m0 = octave 0, m1 = ocatve 1, etc. Set to OUTPUT. Corresponds with KEYPRESS_DOWN
  byte bPin; // 8 pins, b0-b7, each pin corresponds with the each octave, b0 = octave 0, b1 = ocatve 1, etc. Set to OUTPUT. Corresponds with KEYPRESS_UP
  byte state; // Current state of the key.pin(i)
  byte prevState; // Previous state of the key.pin(i)
  byte note; // The specific midi note
  byte octave; // The octave the midi note is in
};

enum KeyState {
  KEYSTATE_UP, // Key unpressed, circuit for bPin(i) is completed
  KEY_FLOAT, // Intial key press, circuit for bPin(i) is broken
  KEYSTATE_DOWN, // Key pressed at the end of its travel, mPin(i) circuit is completed 
  KEY_RELEASE, // Key released, mPin(i) circuit is broken
};

// The keys array with the appropriate pin numbers, notes, and octave values corresponding with my Teensy 4.1
Key keyArray[] = {
  //notePin, mPin, bPin, state, prevState, note, octave}

  // Octave 0, C pin octave is always assiocated with the octave below for M/B pins
  {6,   8, 17, KEYSTATE_UP, KEYSTATE_UP, 21, 0}, // A
  {19,  8, 17, KEYSTATE_UP, KEYSTATE_UP, 22, 0}, // A#
  {7,   8, 17, KEYSTATE_UP, KEYSTATE_UP, 23, 0}, // B
  
  // Octave 1
  {18,  8, 17, KEYSTATE_UP, KEYSTATE_UP, 24,  1}, // C
  {2,   9, 16, KEYSTATE_UP, KEYSTATE_UP, 25,  1}, // C#
  {23,  9, 16, KEYSTATE_UP, KEYSTATE_UP, 26,  1}, // D
  {3,   9, 16, KEYSTATE_UP, KEYSTATE_UP, 27,  1}, // D#
  {22,  9, 16, KEYSTATE_UP, KEYSTATE_UP, 28,  1}, // E
  {4,   9, 16, KEYSTATE_UP, KEYSTATE_UP, 29,  1}, // F
  {21,  9, 16, KEYSTATE_UP, KEYSTATE_UP, 30,  1}, // F#
  {5,   9, 16, KEYSTATE_UP, KEYSTATE_UP, 31,  1}, // G
  {20,  9, 16, KEYSTATE_UP, KEYSTATE_UP, 32,  1}, // G#
  {6,   9, 16, KEYSTATE_UP, KEYSTATE_UP, 33,  1}, // A
  {19,  9, 16, KEYSTATE_UP, KEYSTATE_UP, 34,  1}, // A#
  {7,   9, 16, KEYSTATE_UP, KEYSTATE_UP, 35,  1}, // B

  // Octave 2
  {18,  9, 16, KEYSTATE_UP, KEYSTATE_UP, 36,  2}, // C
  {2,  10, 15, KEYSTATE_UP, KEYSTATE_UP, 37,  2}, // C#
  {23, 10, 15, KEYSTATE_UP, KEYSTATE_UP, 38,  2}, // D
  {3,  10, 15, KEYSTATE_UP, KEYSTATE_UP, 39,  2}, // D#
  {22, 10, 15, KEYSTATE_UP, KEYSTATE_UP, 40,  2}, // E
  {4,  10, 15, KEYSTATE_UP, KEYSTATE_UP, 41,  2}, // F
  {21, 10, 15, KEYSTATE_UP, KEYSTATE_UP, 42,  2}, // F#
  {5,  10, 15, KEYSTATE_UP, KEYSTATE_UP, 43,  2}, // G
  {20, 10, 15, KEYSTATE_UP, KEYSTATE_UP, 44,  2}, // G#
  {6,  10, 15, KEYSTATE_UP, KEYSTATE_UP, 45,  2}, // A
  {19, 10, 15, KEYSTATE_UP, KEYSTATE_UP, 46,  2}, // A#
  {7,  10, 15, KEYSTATE_UP, KEYSTATE_UP, 47,  2}, // B

  // Octave 3
  {18, 10, 15, KEYSTATE_UP, KEYSTATE_UP, 48,  3}, // C
  {2,  11, 14, KEYSTATE_UP, KEYSTATE_UP, 49,  3}, // C#
  {23, 11, 14, KEYSTATE_UP, KEYSTATE_UP, 50,  3}, // D
  {3,  11, 14, KEYSTATE_UP, KEYSTATE_UP, 51,  3}, // D#
  {22, 11, 14, KEYSTATE_UP, KEYSTATE_UP, 52,  3}, // E
  {4,  11, 14, KEYSTATE_UP, KEYSTATE_UP, 53,  3}, // F
  {21, 11, 14, KEYSTATE_UP, KEYSTATE_UP, 54,  3}, // F#
  {5,  11, 14, KEYSTATE_UP, KEYSTATE_UP, 55,  3}, // G
  {20, 11, 14, KEYSTATE_UP, KEYSTATE_UP, 56,  3}, // G#
  {6,  11, 14, KEYSTATE_UP, KEYSTATE_UP, 57,  3}, // A
  {19, 11, 14, KEYSTATE_UP, KEYSTATE_UP, 58,  3}, // A#
  {7,  11, 14, KEYSTATE_UP, KEYSTATE_UP, 59,  3}, // B

  // Octave 4
  {18, 11, 14, KEYSTATE_UP, KEYSTATE_UP, 60,  4}, // C
  {2,  12, 13, KEYSTATE_UP, KEYSTATE_UP, 61,  4}, // C#
  {23, 12, 13, KEYSTATE_UP, KEYSTATE_UP, 62,  4}, // D
  {3,  12, 13, KEYSTATE_UP, KEYSTATE_UP, 63,  4}, // D#
  {22, 12, 13, KEYSTATE_UP, KEYSTATE_UP, 64,  4}, // E
  {4,  12, 13, KEYSTATE_UP, KEYSTATE_UP, 65,  4}, // F
  {21, 12, 13, KEYSTATE_UP, KEYSTATE_UP, 66,  4}, // F#
  {5,  12, 13, KEYSTATE_UP, KEYSTATE_UP, 67,  4}, // G
  {20, 12, 13, KEYSTATE_UP, KEYSTATE_UP, 68,  4}, // G#
  {6,  12, 13, KEYSTATE_UP, KEYSTATE_UP, 69,  4}, // A
  {19, 12, 13, KEYSTATE_UP, KEYSTATE_UP, 70,  4}, // A#
  {7,  12, 13, KEYSTATE_UP, KEYSTATE_UP, 71,  4}, // B

  // Octave 5
  {18, 12, 13, KEYSTATE_UP, KEYSTATE_UP, 72,  5}, // C
  {2,  24, 41, KEYSTATE_UP, KEYSTATE_UP, 73,  5}, // C#
  {23, 24, 42, KEYSTATE_UP, KEYSTATE_UP, 74,  5}, // D
  {3,  24, 41, KEYSTATE_UP, KEYSTATE_UP, 75,  5}, // D#
  {22, 24, 41, KEYSTATE_UP, KEYSTATE_UP, 76,  5}, // E
  {4,  24, 41, KEYSTATE_UP, KEYSTATE_UP, 77,  5}, // F
  {21, 24, 41, KEYSTATE_UP, KEYSTATE_UP, 78,  5}, // F#
  {5,  24, 41, KEYSTATE_UP, KEYSTATE_UP, 79,  5}, // G
  {20, 24, 41, KEYSTATE_UP, KEYSTATE_UP, 80,  5}, // G#
  {6,  24, 41, KEYSTATE_UP, KEYSTATE_UP, 81,  5}, // A
  {19, 24, 41, KEYSTATE_UP, KEYSTATE_UP, 82,  5}, // A#
  {7,  24, 41, KEYSTATE_UP, KEYSTATE_UP, 83,  5}, // B

  // Octave 6
  {18, 24, 41, KEYSTATE_UP, KEYSTATE_UP, 84,  6}, // C
  {2,  25, 40, KEYSTATE_UP, KEYSTATE_UP, 85,  6}, // C#
  {23, 25, 40, KEYSTATE_UP, KEYSTATE_UP, 86,  6}, // D
  {3,  25, 40, KEYSTATE_UP, KEYSTATE_UP, 87,  6}, // D#
  {22, 25, 40, KEYSTATE_UP, KEYSTATE_UP, 88,  6}, // E
  {4,  25, 40, KEYSTATE_UP, KEYSTATE_UP, 89,  6}, // F
  {21, 25, 40, KEYSTATE_UP, KEYSTATE_UP, 90,  6}, // F#
  {5,  25, 40, KEYSTATE_UP, KEYSTATE_UP, 91,  6}, // G
  {20, 25, 40, KEYSTATE_UP, KEYSTATE_UP, 92,  6}, // G#
  {6,  25, 40, KEYSTATE_UP, KEYSTATE_UP, 93,  6}, // A
  {19, 25, 40, KEYSTATE_UP, KEYSTATE_UP, 94,  6}, // A#
  {7,  25, 40, KEYSTATE_UP, KEYSTATE_UP, 95,  6}, // B

  // Octave 7
  {18, 26, 40, KEYSTATE_UP, KEYSTATE_UP, 96,  7}, // C
  {2,  26, 39, KEYSTATE_UP, KEYSTATE_UP, 97,  7}, // C#
  {23, 26, 39, KEYSTATE_UP, KEYSTATE_UP, 98,  7}, // D
  {3,  26, 39, KEYSTATE_UP, KEYSTATE_UP, 99,  7}, // D#
  {22, 26, 39, KEYSTATE_UP, KEYSTATE_UP, 100, 7}, // E
  {4,  26, 39, KEYSTATE_UP, KEYSTATE_UP, 101, 7}, // F
  {21, 26, 39, KEYSTATE_UP, KEYSTATE_UP, 102, 7}, // F#
  {5,  26, 39, KEYSTATE_UP, KEYSTATE_UP, 103, 7}, // G
  {20, 26, 39, KEYSTATE_UP, KEYSTATE_UP, 104, 7}, // G#
  {6,  26, 39, KEYSTATE_UP, KEYSTATE_UP, 105, 7}, // A
  {19, 26, 39, KEYSTATE_UP, KEYSTATE_UP, 106, 7}, // A#
  {7,  26, 39, KEYSTATE_UP, KEYSTATE_UP, 107, 7}, // B

  // Octave 8
  {18, 26, 39, KEYSTATE_UP, KEYSTATE_UP, 108, 8} // C
  
};

void scanKeys(Key keyArray[], size_t size) {
  for (size_t i = 0; i < size; ++i) {
    Key &key = keyArray[i];
    byte currentState = digitalRead(key.pin);

    if (currentState != key.prevState) { // key.prevState gets initilized as KEYSTATE_UP in keyArray
      key.prevState = key.state; 
      switch (key.state = KEYSTATE_UP == HIGH) { // Key unpressed(key.pin == HIGH) = KEYSTATE_UP, Key pressed(key.pin ==LOW) = KEY_FLOAT, Key at bottom of travel(key.pin == HIGH) = KEYSTATE_DOWN, key released(key.pin == LOW) = KEY_RELEASE
        case KEY_FLOAT: 
          if (currentState == LOW && key.prevState == KEYSTATE_UP) {
            key.state = KEY_FLOAT;
            // Timer will start here, Timer is used to calculate Velocity
          } 
          break;

        case KEYSTATE_DOWN:
          if (currentState == HIGH && key.prevState == KEY_FLOAT) {
            key.state = KEYSTATE_DOWN;
            // Timer would stop here
            //Calculate Velocity

            MIDI.sendNoteOn(key.note, 127, 1);
            Serial.print("Note: ");
            Serial.print(key.note);
            Serial.print(", Octave: ");
            Serial.println(key.octave);
          } else if (currentState == LOW && key.prevState == KEYSTATE_DOWN) {
            key.state = KEY_RELEASE;
            MIDI.sendNoteOff(key.note, 0, 1);
          }
          break;
      }
      delay(10);// 10ms delay 
    }
  }
}

void setup() {
  for (size_t i = 0; i < sizeof(keyArray) / sizeof(keyArray[0]); i++) {
    pinMode(keyArray[i].pin, INPUT_PULLUP);
  }
  for (size_t i = 0; i < sizeof(keyArray) / sizeof(keyArray[0]); i++) {
    pinMode(keyArray[i].mPin, OUTPUT);
  }
  for (size_t i = 0; i < sizeof(keyArray) / sizeof(keyArray[0]); i++) {
    pinMode(keyArray[i].bPin, OUTPUT);
  }
  MIDI.begin(MIDI_CHANNEL_OMNI);

  Serial.begin(9600); // Serial for debugging 
  while (!Serial) {
    ; // Wait for Serial to be ready
  }
  Serial.println("Serial ready.");
}

void loop() {
  scanKeys(keyArray, sizeof(keyArray) / sizeof(Key));
}
