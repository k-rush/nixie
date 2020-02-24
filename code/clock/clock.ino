#include <DS1307RTC.h>
#include <Wire.h>
#include <TimeLib.h>

//Pin connected to ST_CP of 74HC595
int latchPin = 12;
//Pin connected to SH_CP of 74HC595
int clockPin = 8;
////Pin connected to DS of 74HC595
int dataPin = 11;
int rotaryA = 2;
int rotaryB = 4;

int shdnPin = 3;

volatile int master_count = 0; // universal count
volatile byte INTFLAG1 = 0; // interrupt status flag

//holders for infromation you're going to pass to shifting function
byte data;
tmElements_t tm;
tmElements_t newtm;
  
void setup() {
  //set pins to output because they are addressed in the main loop
  
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(rotaryA, INPUT);
  pinMode(rotaryB, INPUT);
  Serial.begin(9600);
  if (!RTC.read(tm)) {
    getTime(__TIME__);
    RTC.write(tm);
  }
  
  attachInterrupt(0, flag, RISING);
  pinMode(shdnPin, OUTPUT);
  digitalWrite(shdnPin, LOW);
}

void loop() {
  if(INTFLAG1)
  {
    RTC.write(newtm);
    INTFLAG1 = 0;
  }
  RTC.read(tm);
  int digits = tm.Hour * 100 + tm.Minute;
  Serial.println(digits);
  shiftOut(dataPin, clockPin, latchPin, digits);
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
  int reverseDigits;
  for(int i = 0; i < 4; i++)
  {
    reverseDigits *= 10;
    int digit = digits % 10;
    digits = (int)floor(digits / 10);
    reverseDigits += digit;
  }
  
  for(int j = 0; j < 4; j++)
  {
    int digit = reverseDigits % 10;
    reverseDigits = (int)floor(reverseDigits / 10);
    for (i=1; i<=10; i++)  {
      digitalWrite(myClockPin, 0);
      pinState= digit == i%10;
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

void flag() {
  newtm = tm;
  time_t unixTime = makeTime(newtm);
  if (digitalRead(rotaryA) && !digitalRead(rotaryB)) {
    Serial.println("CCW");
    unixTime -= unixTime % 60;
    unixTime -= 60;
    breakTime(unixTime, newtm);
  }
  if (digitalRead(rotaryA) && digitalRead(rotaryB)) {
    Serial.println("CW");    
    unixTime -= unixTime % 60;
    unixTime += 60;
    breakTime(unixTime, newtm);
  }
  INTFLAG1 = 1;
}

bool getTime(const char *str)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  return true;
}
