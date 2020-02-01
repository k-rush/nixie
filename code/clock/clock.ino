#include <Wire.h>
#include <TimeLib.h>
#include "RotaryEncoder.h"
#include <DS1307RTC.h>

//Pin connected to ST_CP of 74HC595
int latchPin = 12;
//Pin connected to SH_CP of 74HC595
int clockPin = 8;
////Pin connected to DS of 74HC595
int dataPin = 11;

//holders for infromation you're going to pass to shifting function
byte data;
tmElements_t tm;
RotaryEncoder encoder(2, 4);

void setup() {
  //set pins to output because they are addressed in the main loop
  
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  Serial.begin(9600);
  if (!RTC.read(tm)) {
    Serial.print("Setting rtc...");
    tm.Hour = 12;
    tm.Minute = 12;
    tm.Second = 12;
    tm.Day = 1;
    tm.Month = 1;
    tm.Year = 2020;
    RTC.write(tm);
  }
}

void loop() {
  static int pos = 0;
  encoder.tick();

  int newPos = encoder.getPosition();
  
  if (pos != newPos) {
    Serial.print(newPos);
    Serial.println();
    pos = newPos;
  }
  
  if (RTC.read(tm)) {
      Serial.print("Ok, Time");
      int digits = tm.Hour * 100 + tm.Minute;
      Serial.print(digits);
      shiftOut(dataPin, clockPin, latchPin, digits);
      delay(300);
    }
}



// the heart of the program
void shiftOut(int myDataPin, int myClockPin, int latchPin, int digits) {
  // This shifts 8 bits out MSB first,
  //on the rising edge of the clock,
  //clock idles low

  //internal function setup
  int i=0;
  int pinState;
  digitalWrite(myDataPin, 0);
  digitalWrite(myClockPin, 0);
  digitalWrite(latchPin, 0);
  for(int j = 0; j < 4; j++)
  {
    int digit = digits % 10;
    digits = (int)floor(digits / 10);
    for (i=0; i<10; i++)  {
      digitalWrite(myClockPin, 0);
      pinState= digit == i;
      Serial.print(pinState);
      //Sets the pin to HIGH or LOW depending on pinState
      digitalWrite(myDataPin, pinState);
      //register shifts bits on upstroke of clock pin  
      digitalWrite(myClockPin, 1);
      //zero the data pin after shift to prevent bleed through
      digitalWrite(myDataPin, 0);
    }
  }
  digitalWrite(latchPin, 1);
  Serial.println();
  digitalWrite(myClockPin, 0);
}
