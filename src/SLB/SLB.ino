// Import Libraries
#include <OneWire.h>
#include <DallasTemperature.h>
#include "LedControl.h"

// Setup temperature sensor
#define ONE_WIRE_BUS 26
OneWire ourWire(ONE_WIRE_BUS);
DallasTemperature sensors(&ourWire);

// Setup display:
LedControl lc = LedControl(4,2,0,1); // DIN, CLK, CS

// Setup photoresistor
const int photoPin = 27; // select the input pin for the potentiometer

// Global Variables to use with Millis
unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long measurement_interval = 1000; // Take a measurement every second etc.

// Global variables to use with the sensors:
int temperature;
int sunlight;

float temp_threshold = 25.0;
float sunlight_threshold = 10.0;

// Global varibles to use with the counter:
int counter_temp = 0;

// Setup the touch sensor
int debounceTime = 1000; //debounce time for touch sensor
int tocuhPin = 15;

int signal = 0;
String input;

//////////////////////////////
/////// Setup function ///////
//////////////////////////////

void setup(){

  Serial.begin(9600);
  /*
  The MAX72XX is in power-saving mode on startup,
  we have to do a wakeup call
  */
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,1);
  /* and clear the display */
  lc.clearDisplay(0);

  // Begin temperature sensor
  sensors.begin(); 

  // Start the millis:
  startMillis = millis();

  // Define pinmode for toch-sensor
  pinMode(tocuhPin, INPUT);

}

//////////////////////////////
///////// World loop /////////
//////////////////////////////

void loop(){
  // For debugging, take user input:
  if(Serial.available() > 0)  {
    input=Serial.readStringUntil('\n');
    signal=input.toInt();
  }

  currentMillis = millis();
  // Take a measurement:
  if ((currentMillis - startMillis) % measurement_interval == 0){
    Serial.println("Taking Measurements");
    temperature = read_temperature();
    sunlight = read_sunlight();

    Serial.print("temperature: ");
    Serial.println(temperature);
    Serial.print("sunlight: ");
    Serial.println(sunlight);

    // After measurement is taken, determine whether or not the display should change:
    if (increment(temperature, temp_threshold)){
      counter_temp += 1;
    }
  }

  // Activate display
  
  if (digitalRead(tocuhPin) == HIGH){
    display(counter_temp, sunlight);
  }
  else {
    lc.clearDisplay(0);
  }

}

//////////////////////////////
////// Define functions //////
//////////////////////////////

// Sensor Input:

// Temperature
int read_temperature(){
  sensors.requestTemperatures(); 
  return sensors.getTempCByIndex(0);
}

// Sunlight
int read_sunlight(){
  return analogRead(photoPin);
}

bool increment(int measurement, float measurement_threshold){
  if (measurement >= measurement_threshold) {
    return true;
  }
  else {return false;}
}

// Activate display
void display(int counter, int sunlight_reading){
  float intensity = map(sunlight_reading, 1500, 4095, 0, 10);
  Serial.println(intensity);
  lc.setIntensity(0,intensity);
  if (counter <= 20){
    happySmileyMatrix();

  } else {
    sadSmileyMatrix();
  }
}

void happySmileyMatrix(){
  byte happy[] = {B00111100,B01000010,B10100101,B10000001,B10100101,B10011001,B01000010,B00111100};

  lc.setRow(0,0,happy[0]);
  lc.setRow(0,1,happy[1]);
  lc.setRow(0,2,happy[2]);
  lc.setRow(0,3,happy[3]);
  lc.setRow(0,4,happy[4]);
  lc.setRow(0,5,happy[5]);
  lc.setRow(0,6,happy[6]);
  lc.setRow(0,7,happy[7]);
}

void sadSmileyMatrix(){
  byte sad[] = {B00111100,B01000010,B10100101,B10011001,B10011001,B10100101,B01000010,B00111100};

  lc.setRow(0,0,sad[0]);
  lc.setRow(0,1,sad[1]);
  lc.setRow(0,2,sad[2]);
  lc.setRow(0,3,sad[3]);
  lc.setRow(0,4,sad[4]);
  lc.setRow(0,5,sad[5]);
  lc.setRow(0,6,sad[6]);
  lc.setRow(0,7,sad[7]);
}

void heartFromMom(){
  byte heart[] = {B00000000,B01100110,B11111111,B11111111,B01111110,B00111100,B00011000,B00000000};

  lc.setRow(0,0,heart[0]);
  lc.setRow(0,1,heart[1]);
  lc.setRow(0,2,heart[2]);
  lc.setRow(0,3,heart[3]);
  lc.setRow(0,4,heart[4]);
  lc.setRow(0,5,heart[5]);
  lc.setRow(0,6,heart[6]);
  lc.setRow(0,7,heart[7]);
}

void send_data(int temperature_reading){
  Serial.println("Do stuff here");
}

// Photoresister shows values between 1500 and 4095
// USE DELAY OR SOMETHING TO MAKE TOUCH LATCHING!!!
