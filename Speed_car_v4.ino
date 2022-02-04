/********
Speed car v0.4
2022 Terry Seiver
**********/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

//Speed calculation variables - User changeable
long numRotations = 20.0; //the greater the rotations, the more accurate
float wheelRadius = 7.06; //wheel radius in mm
long modelScale = 87.0; //HO scale 

//Speed calculation variables - Do not change
bool lastRead; //last reading of hall effect sensor
int count = 0; //total count of rotations
int lastCount = 0; //last rotation count when measured
float circumference = 2.0*3.14*wheelRadius; //wheel circumference
float carSpeed;  //Speed in KM/H
unsigned long timeStarted = millis(); //time when rotation started
float timeTaken;  //time taken for 1 rotation
float distance = (numRotations*circumference*modelScale)/1000000.0; //distance in scaled km
unsigned long timeLastRead = 0;
unsigned long timeLastChanged = 0;

//acceleration variables
long prevTimeDifference = 0; //used to determine if accel
long newTimeDifference = 0; //used to determine if accel
long timeDifference = 0;
unsigned long timePrev= 0;
unsigned long timeCurr = 0;

//Display objects / variables
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();
String displayText; 
char charDisplay[26]; //array used for LED display. Size based on displayText.

int hallPin = 2;

void setup()
{
  Serial.begin(115200);
  pinMode(hallPin,INPUT); //hall effect sensor out
    
  alpha4.begin(0x70);  // pass in the address
  
  //Start up text sequence
  displayText = "    SPEED-CAR    IN KM/H";
  displayText.toCharArray(charDisplay,sizeof(charDisplay));
  for (byte i=0; i < sizeof(charDisplay) - 5; i++) {
    alpha4.writeDigitAscii(0, charDisplay[i]);
    alpha4.writeDigitAscii(1, charDisplay[i+1]);
    alpha4.writeDigitAscii(2, charDisplay[i+2]);
    alpha4.writeDigitAscii(3, charDisplay[i+3]);
    alpha4.writeDisplay();
    if (i==0) delay(1000); else delay(300);
  }
  delay(1000);
  displayText = "-GO-";
  displayText.toCharArray(charDisplay,sizeof(charDisplay));
  writeText(true);
  delay(1000);
  
  lastRead = digitalRead(hallPin); //read current state of hall sensor
}

void loop()
{
  if ((digitalRead(hallPin)!=lastRead)) 
  { //if hall effect has changed
    timeLastChanged = millis();
    if ((digitalRead(hallPin)==HIGH)&&(millis()-timeLastRead>=10))
    { //if hall sensor has been triggered, but not too recently (basic debounce)
      
      timeLastRead = millis();
      count++;

      //determine if accel or decel by comparing difference in time between reads
      if (count%5==0) //for every 5th read
      {
        timeCurr = millis();
        newTimeDifference = timeCurr-timePrev;
        timeDifference = newTimeDifference - prevTimeDifference;
        if (timeDifference < -5) {
          alpha4.writeDigitRaw(0, 0x3800); //upward arrow
        } else if (timeDifference > 5) {
          alpha4.writeDigitRaw(0, 0x700); //downward arrow
        } else {
          alpha4.writeDigitRaw(0, 0xC0); //dash
        }
        alpha4.writeDisplay();
        timePrev = timeCurr;
        prevTimeDifference = newTimeDifference;
      }

      if (count-lastCount==numRotations) {  
        //calculate speed
        timeTaken=(timeLastRead-timeStarted)/(60.0*60.0*1000.0); //in hours
        carSpeed=distance/timeTaken;        
       
        //Display car speed
         //itoa(count, charDisplay, 10); //displays rotation count - used for testing
        dtostrf(carSpeed,3,0,charDisplay);
        
        writeText(false);
 
        //reset calculation variables
        timeStarted=millis();
        lastCount = count; 
      } 
    }
    lastRead = digitalRead(hallPin);
  }
  if (millis()-timeLastChanged>3000)
  { //if the last reading is longer than 3 seconds, assume its stopped
    alpha4.writeDigitRaw(0, 0xC0); //dash
    alpha4.writeDisplay();
    carSpeed = 0;
    dtostrf(carSpeed,3,0,charDisplay);
    writeText(false);
  }
}

void writeText(bool useFirstChar)
{ //function to write text to LCD
  if (useFirstChar==true)
  {
    alpha4.writeDigitAscii(0, charDisplay[0]);
    alpha4.writeDigitAscii(1, charDisplay[1]);
    alpha4.writeDigitAscii(2, charDisplay[2]);
    alpha4.writeDigitAscii(3, charDisplay[3]);
  } else
  {
      alpha4.writeDigitAscii(1, charDisplay[0]);
      alpha4.writeDigitAscii(2, charDisplay[1]);
      alpha4.writeDigitAscii(3, charDisplay[2]);
  }
  alpha4.writeDisplay();
}
