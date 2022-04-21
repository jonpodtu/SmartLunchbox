// Import Libraries
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "LedControl.h"
#include <ThingSpeak.h>

///////////////////////////////
/////// Initialize code ///////
///////////////////////////////

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
unsigned long measurementMillis;
const unsigned long measurement_interval = 4000; // Take a measurement every (4) second etc.- Can be changed to every minute

// Global variables to use with the sensors:
int temperature;
int sunlight;

float temp_threshold = 25.0;

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

/////////////////////////////////////
/////// Initialize Thingspeak ///////
/////////////////////////////////////
// (The below lines 54-64 is taken from the course):

#define ssid "SGS21" //Indsæt navnet på jeres netværk her
#define password "MandKvindeAdamEva" //Indsæt password her
WiFiClient client;

unsigned long channelID = 1671827; //your channel
const char * myWriteAPIKey = "BSZB7OSERJ3O70S9"; // your WRITE API key
const char* server = "api.thingspeak.com";

//////////////////////////////
/////// Setup function ///////
//////////////////////////////

void setup(){

  Serial.begin(9600);
  delay(10);
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

  // (The below lines 93-107 is taken from the course):

  // Connecting to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

}

//////////////////////////////
///////// World loop /////////
//////////////////////////////

void loop(){

  currentMillis = millis();
  Serial.println(currentMillis);

  //ThingSpeak (The below lines 117-128 is taken from the course):
  ThingSpeak.begin(client);
  if (client.connect(server, 80)) {
    
    
    // Measure Signal Strength (RSSI) of Wi-Fi connection
    long rssi = WiFi.RSSI();

    //Serial.print("RSSI: ");
    //Serial.println(rssi); 

    //ThingSpeak.setField(2,rssi);
  
  
    // For debugging, take user input:
    //if(Serial.available() > 0)  {
    //  input=Serial.readStringUntil('\n');
    //  signal=input.toInt();
    //}
  
    // Take a measurement:
    sunlight = read_sunlight();
    
    if (currentMillis >= (measurement_interval + measurementMillis)){
      
      measurementMillis = millis();
      
      Serial.println("Taking Measurements");
      temperature = read_temperature();
  
      Serial.print("temperature: ");
      Serial.println(temperature);
      Serial.print("sunlight: ");
      Serial.println(sunlight);

      // After measurement is taken, send this to thingspeak:
      Serial.print("Sending to thingspeak ...");
      ThingSpeak.setField(1,20); //Sending 20 degrees
      ThingSpeak.writeFields(channelID, myWriteAPIKey);
      Serial.print("Sent!");
  
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
  client.stop();
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
