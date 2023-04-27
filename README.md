# KX88-Keyboard-Matrix![IMG_0440](https://user-images.githubusercontent.com/132019719/235000816-0b0bb1fe-52ef-4dce-bb2a-5ab91f165012.jpg)
This shows the actual mechanism for registering key presses
It is comprised of two cicruits and three springs 
  Spring 1(postive spring): corresponds to the B pin(unpressed key state) and Spring 2 rest against Spring 1 compleleting B Pin circuit
  Spring 2(negative spring): corresponds with the note pins
  Spring 3(postive spring): corresponds with the Mpin(pressed key state) and Spring 2 comes in contact with Spring 3 once a key is pressed, completing M          pin circuit

![IMG_0419](https://user-images.githubusercontent.com/132019719/235001779-1d53a9ca-f931-4ad1-8358-647ad877a885.jpg)
This shows the Note pins, B pins and M pins(pin M7 is the unmarked pin just below M6)

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

This code currently isn't measuring velocity, just trying to get proper scanning and output first
