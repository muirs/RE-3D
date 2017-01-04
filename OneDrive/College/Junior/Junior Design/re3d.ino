// Libraries to include
#include <SPI.h>
#include <Wire.h>
#include "Adafruit_MAX31855.h"
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

// Digital Pin assignments
#define LED    12
#define MAXDO   8
#define MAXCS   7
#define MAXCLK  6
#define RELAY   2
#define ESC     3

// Set up the display and thermocouple over I2C and SPI, respectively
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

// Analog Pin assignments
int HEATERCTRL = A2;
int MOTORCTRL = A0;

// Variables to be used throughout the cde
int temp = 0;
int lastTemp = 0;
int mtr = 0;
int spd = 0;
int c = 0;

// buffer to store characters to be placed on the display
char displaybuffer[4] = {' ', ' ', ' ', ' '};

// Function: updateDisplay()
// Description: Write the contents of the display buffer to the display
void updateDisplay()
{
  alpha4.writeDigitAscii(0, displaybuffer[0]);
  alpha4.writeDigitAscii(1, displaybuffer[1]);
  alpha4.writeDigitAscii(2, displaybuffer[2]);
  alpha4.writeDigitAscii(3, displaybuffer[3]);
  
  alpha4.writeDisplay();
  
  return; 
}

// Function: updateHotLight()
// Description: Turn the warning light about safe temperature on or off 
// depending on the current temperature
void updateHotLight()
{
  if(c > 44)
  {
    digitalWrite(LED,1);
  }
  else if (c > 5)
  {
    digitalWrite(LED,0);
  }
  return;
}

// Function: updateRelay()
// Description: Set the relay (heat band controller) to on or off depending 
// on the current temperatures relation to the goal temperature
void updateRelay()
{
  if(c < temp && temp > 180 && c != 0){
    digitalWrite(RELAY,1);
  }
  else{
    digitalWrite(RELAY,0);
  }
  return;
}

// Function: updateMotor()
// Description: Read from the potentiometer for speed control, adjust 
// the value for our speed range, and write it to the speed controller
void updateMotor()
{
  mtr = analogRead(MOTORCTRL)/6 - 10;
  if(mtr < 0)
    mtr = 0;
  analogWrite(ESC, mtr);
  return;
}

// Function: setup()
// Description: Run code once at boot to set up various values and 
// establish pin settings
void setup() 
{
  // Assign LED and Relay Pins as outputs
  pinMode(LED, OUTPUT);
  pinMode(RELAY, OUTPUT);

  // Set the address of the Display
  alpha4.begin(0x70);

  // Run startup test of each display to make sure they are working
  alpha4.writeDigitRaw(3, 0x0);
  alpha4.writeDigitRaw(0, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(0, 0x0);
  alpha4.writeDigitRaw(1, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(1, 0x0);
  alpha4.writeDigitRaw(2, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(2, 0x0);
  alpha4.writeDigitRaw(3, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);

  // Clear display before starting main loop
  alpha4.clear();
  alpha4.writeDisplay();

  // Write startup code to ESC (will not move otherwise!)
  analogWrite(ESC, 160);

}

// Function: loop()
// Description: Main program loop. Will continue running as long as Arduino is on
void loop() 
{
  // get the current temperature from the thermocouple
  c = thermocouple.readCelsius();

  // By default, print the hot tip temperature to display
  if(c != 0)
    sprintf(displaybuffer, "%3iC", c);

  // Update components with their current status
  updateRelay();
  updateHotLight();
  updateMotor();

  // If a change is detected in the temperature dial, update the display to 
  // temporarily show the target temperature
  if((temp + 1) < ((1024 - analogRead(HEATERCTRL))/8 + 170) || (temp - 1) > ((1024 - analogRead(HEATERCTRL))/8 + 170))
  {
    temp = (1024 - analogRead(HEATERCTRL))/8 + 170;
    sprintf(displaybuffer, "%3iC", temp);
    updateDisplay();
    delay(100);
  }

  // If a change is detected in the speed dial, update the display to temporarily show 
  // the speed
  if(spd != 10 - analogRead(MOTORCTRL)/100)
  {
    spd = 10 - analogRead(MOTORCTRL)/100;
    sprintf(displaybuffer, "%4i", spd);
    updateDisplay();
    delay(100);
  }

  //update display with whatever the current value should be
  updateDisplay();

  // Delay to establish sampling rate
  delay(100);

}