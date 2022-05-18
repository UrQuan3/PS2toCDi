/*******************************
 *
 *  File: sketchPS2toCDi.ino
 *  Description: adapt a PS2 mouse for use with a CD-I
 *  Updates: Brian Harrison
 *  Date: 05-04-2022
 *  Version: 1.0
 *  License: CC-BY 4.0 ( http://creativecommons.org/licenses/by/4.0/legalcode )
 *  
 *  File: sketchPS2toCDi.ino
 *  Description: adapt a PS2 mouse for use with a CD-I
 *  Author: Dan Brakeley
 *  Date: 10-30-2016
 *  Version: 0.1
 *  License: whatevs.
 *
 *  BASED ON:
 *  File: sketchSNEStoCDi.ino
 *  Description: software for the SNEStoCDi gamepad converter (allows to use a Nintendo SNES/SFC gamepad on a Philips CDi)
 *  Author: Laurent Berta
 *  Date: 03-21-2016
 *  Version: 0.2
 *  Thanks: Rob Duarte (SNESpad library), Paul Hackmann (documentation about the CDi pointing devices)
 *  License: CC-BY 4.0 ( http://creativecommons.org/licenses/by/4.0/legalcode )
 *  
 *  Settings:
 *  Board: Arduino Nano or Arduino Nano Every
 *  Optimize: Fastest
 *  select port
 *
 *  1.0: - added mouse accleleration.  2x^2 seems about right.
 *       - I can't seem to include time, because the units involved don't work with integer math.
 *         I'm leaving them in, but commented out.
 *         To edit, look for these lines:
 *             //X*v linear component, Y*(v^2) acceleration component
 *             vnew = (1 * v) + (2 * sq(v));
 *         Change the '2' to your desired acceleration.  Must be an integer value.
 *       - I moved the code to my prefered nested send verification.  However, I really don't know what to do if the CD-i
 *         cancels a send, except to drop the update.
 ******************************/

#include <PS2Mouse.h>
#include <SoftwareSerial.h>
#include <limits.h>

//function declarations
//bool assertRTS();
//byte capToByte(int);
//void printBits(byte);

// if true, enables init and write of data to CD-I
const bool ENABLE_CDI = true;

// if true, logs debug info to serial console
const bool DEBUG_OUT_ENABLED = false;


// digital pins
const int PS2_DATA_PIN = 5; // PS/2 mouse data
const int PS2_CLOCK_PIN = 6; // PS/2 mouse clock
const int CDI_RX_PIN = 19; // not connected, as CD-I transmits only on RTS line
const int CDI_TX_PIN = 20; // transmit to CD-I (connect to RDX)
const int LED_PIN = 13; // arduino onboard LED pin

// analog pins
const int CDI_RTS_PIN_A = A7; // CDI RTS signal


// cdi consts
const byte CDI_RELATIVE_DEVICE_ID = 0b11001101;
const int CDI_RTS_THRESHOLD = 328; // threshold for the CDi RTS analog detection

// cdi globals
SoftwareSerial serialCDI(CDI_RX_PIN, CDI_TX_PIN, true); // RX, TX, inverse_logic
bool firstId = true;


// mouse consts
const byte MOUSE_BTN_LEFT   = 0b0001;
const byte MOUSE_BTN_RIGHT  = 0b0010;
const byte MOUSE_BTN_MASK = MOUSE_BTN_LEFT | MOUSE_BTN_RIGHT;

// mouse globals
PS2Mouse mouse(PS2_CLOCK_PIN, PS2_DATA_PIN, STREAM);
byte prevBtns = 0;


void setup()
{
  if (DEBUG_OUT_ENABLED) {
    Serial.begin(38400);
  }

  mouse.initialize();

  if (ENABLE_CDI) {
    // open serial interface to send data to the CDi
    serialCDI.begin(1200);
  }

  pinMode(LED_PIN, OUTPUT);
}

// main loop
void loop()
{
  bool goodsend = false;
  if(!assertRTS()) {
    digitalWrite(LED_PIN, HIGH);
    while(!assertRTS()) { } // wait for CDi to assert the RTS line
    if(firstId) delay(100);
    else delay(1);
    firstId = false;
    serialCDI.write(0b11001010); // send device id ("maneuvering device")
  }
  digitalWrite(LED_PIN, LOW);

  // read PS/2 mouse state
  int data[3];
  mouse.report(data);

  // gather data we care about
  byte btns = byte(data[0] & MOUSE_BTN_MASK);
  //byte x = capToByte(data[1]);
  //byte y = capToByte(-data[2]); // swap y up/down by negating the coordinate

  //let's try mouse acceleration
  int dx, dy, dd, v, ddnew, vnew;
  //uint32_t currenttime = 0, prevtime = 0, dt = 0;   //with time
  //currenttime = millis();   //with time
  //if(currenttime > prevtime) {   //with time
  //  dt = currenttime - prevtime;   //with time
  //} else {   //with time
  //  //deal with wrap around   //with time
  //  dt = ULONG_MAX - prevtime + currenttime;   //with time
  //}   //with time
  dx = data[1];
  dy = data[2];
  //dd = sqrt(sq(dx) + sq(dy));     //with time
  //v = dd/dt;                      //with time
  v = sqrt(sq(dx) + sq(dy));    //ignoring time, assuming dt = 1
  if(v != 0) {
    //X*v linear component, Y*(v^2) acceleration component
    vnew = (1 * v) + (2 * sq(v));
    //ddnew = vnew * dt;         //with time
    //dx = dx * ddnew / dd;   //with time
    //dy = dy * ddnew / dd;   //with time
    dx = dx * vnew / v;   //ignoring time
    dy = dy * vnew / v;   //ignoring time
  }
  //prevtime = currenttime;   //with time

  byte x = capToByte(dx);
  byte y = capToByte(-dy); // swap y up/down by negating the coordinate
/*
  // original send logic
  if (prevBtns != btns || x != 0 || y != 0) {
    prevBtns = btns;

    // generate CDI data
    byte padbyte0 = 0b11000000 | ((btns & MOUSE_BTN_LEFT) << 5) | ((btns & MOUSE_BTN_RIGHT) << 3) | ((x & 0b11000000) >> 6) | ((y & 0b11000000) >> 4);
    byte padbyte1 = 0b10000000 | (x & 0b00111111);
    byte padbyte2 = 0b10000000 | (y & 0b00111111);

    if (ENABLE_CDI) {
      if (assertRTS()) serialCDI.write(padbyte0);
      if (assertRTS()) serialCDI.write(padbyte1);
      if (assertRTS()) serialCDI.write(padbyte2);
    }

    if (DEBUG_OUT_ENABLED) {
      printBits(padbyte0);
      Serial.print(" ");
      printBits(padbyte1);
      Serial.print(" ");
      printBits(padbyte2);
      Serial.print(" : ");
      Serial.print(int8_t(x));
      Serial.print(", ");
      Serial.print(int8_t(y));
      Serial.println();
    }
  }
*/
/**/
  // my send logic
  // generate CDI data
  byte padbyte0 = 0b11000000 | ((btns & MOUSE_BTN_LEFT) << 5) | ((btns & MOUSE_BTN_RIGHT) << 3) | ((x & 0b11000000) >> 6) | ((y & 0b11000000) >> 4);
  byte padbyte1 = 0b10000000 | (x & 0b00111111);
  byte padbyte2 = 0b10000000 | (y & 0b00111111);

  //if((padbyte0 != oldpadbyte0) || (padbyte1 != 0b10000000) || (padbyte2 != 0b10000000) || ((padbyte0 & 0b00001111) != 0))  // see if state has changed
  if((prevBtns != btns) || (x != 0) || (y != 0))  // see if state has changed
  {
    //Ah, if !assertRTS() during send, the rest of the send is lost, and save state prevents resend.
    //Make sure all three bytes send before saving to oldpadbyteX
    if(assertRTS()) goodsend = false;
    if(assertRTS()) {
      serialCDI.write(padbyte0);
      if(assertRTS()) {
        serialCDI.write(padbyte1);
        if(assertRTS()) {
          serialCDI.write(padbyte2);
          goodsend = true;
        }
      }
    }
  }

  // save state if sent
  if(goodsend) {
    prevBtns = btns;
    //oldpadbyte0 = padbyte0; 
    //oldpadbyte1 = padbyte1;
    //oldpadbyte2 = padbyte2;
  }

  if (DEBUG_OUT_ENABLED) {
    printBits(padbyte0);
    Serial.print(" ");
    printBits(padbyte1);
    Serial.print(" ");
    printBits(padbyte2);
    Serial.print(" : ");
    Serial.print(int8_t(x));
    Serial.print(", ");
    Serial.print(int8_t(y));
    Serial.println();
  }
/**/
}


// true if RTS asserted
bool assertRTS()
{
  if (analogRead(CDI_RTS_PIN_A) < CDI_RTS_THRESHOLD) return false;
  else return true;
}

byte capToByte(int n)
{
  if (n > 127) {
    return 0x7F;
  } else if (n < -128) {
    return 0x80;
  }
  return byte(n);
}

void printBits(byte n)
{
  Serial.print("0b");
  Serial.print(n & 0b10000000 ? "1" : "0");
  Serial.print(n & 0b01000000 ? "1" : "0");
  Serial.print(n & 0b00100000 ? "1" : "0");
  Serial.print(n & 0b00010000 ? "1" : "0");
  Serial.print(n & 0b00001000 ? "1" : "0");
  Serial.print(n & 0b00000100 ? "1" : "0");
  Serial.print(n & 0b00000010 ? "1" : "0");
  Serial.print(n & 0b00000001 ? "1" : "0");
}
