// Import Libraries
#include <OneWire.h>
#include <DallasTemperature.h>
#include "LedControl.h"

// Setup temperature sensor
#define ONE_WIRE_BUS 26
OneWire ourWire(ONE_WIRE_BUS);
DallasTemperature sensors(&ourWire);

// Assign display:
LedControl lc = LedControl(4,2,0,1); // DIN, CLK, CS

// Assign photoresistor
const int photoPin = 27; // select the input pin for the potentiometer

// Global Variables to use with Millis
unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long measurement_interval = 1000; // Take a measurement every second etc.- Can be changed to every minute

// Global variables to use with the sensors:
int temperature;
int sunlight;

float temp_threshold = 25.0;
float sunlight_threshold = 10.0; // This is for determining if the lunchbox has been in the sun. Currently not used.

// Global varibles to use with the counter:
int counter_temp = 0;

// Assign the touch sensor
bool lc_on = true;
int buttonPin = 15;
int buttonState = 0;
unsigned int old_time;

int signal = 0;
String input;

// Define all icons:
byte happy[] = {B00111100,B01000010,B10100101,B10000001,B10100101,B10011001,B01000010,B00111100};
byte sad[] = {B00111100,B01000010,B10100101,B10011001,B10011001,B10100101,B01000010,B00111100};
byte heart[] = {B00000000,B01100110,B11111111,B11111111,B01111110,B00111100,B00011000,B00000000};

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
  pinMode(buttonPin, INPUT);

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
  if ((lc_on) && (millis() > old_time))
  {
    Serial.println("Resetting display");
    lc.clearDisplay(0);
    lc_on = false;
  }
  else if (!lc_on)
  {
    buttonState = digitalRead(buttonPin);
    if(buttonState == HIGH)
    {
      display(counter_temp, sunlight);
      lc_on = true;
      old_time = millis() + 7000;
    }
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
    display_icon(happy);

  } else {
    display_icon(sad);
  }
}

// Define display function

void display_icon(byte icon[]){
  for(int i = 0; i < 8; i++){
    lc.setRow(0,i,icon[i]);
  }
}



void send_data(int temperature_reading){
  Serial.println("Do stuff here");
}

// Photoresister shows values between 1500 and 4095
// USE DELAY OR SOMETHING TO MAKE TOUCH LATCHING!!!
