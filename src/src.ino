// Import Libraries
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "LedControl.h"
#include <ThingSpeak.h>

// Setup temperature sensor
#define ONE_WIRE_BUS 26
OneWire ourWire(ONE_WIRE_BUS);
DallasTemperature sensors(&ourWire);

// Assign display:
LedControl lc = LedControl(4,2,0,1); // DIN, CLK, CS

// Assign photoresistor
const int photoPin = 32; // select the input pin for the photoresistor

// Global Variables to use with Millis
unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
unsigned long measurementMillis = 0;
unsigned long talkbackMillis = 0;
const unsigned long talkback_interval = 5*60*1000; // Take a check for qued messages every (5) minutes etc.- Can be changed for debugging
const unsigned long measurement_interval = 15*60*1000; // Take a measurement every (15) minutes etc.- Can be changed for debugging

// Global variables to use with the sensors:
int temperature;
int sunlight;

// The temperature threshold is the value, where temperatures above speed up bacteria growth
float temp_threshold = 8.0;
int counter_temp = 0;
int counter_threshold = 8;

// Assign the touch sensor 
bool lc_on = true;
int buttonPin = 15;
int buttonState = 0;
unsigned int old_time;

// Time for the heart:
unsigned int heart_time;

// Define all icons:
byte happy[] = {B00111100,B01000010,B10100101,B10000001,B10100101,B10011001,B01000010,B00111100};
byte sad[] = {B00111100,B01000010,B10100101,B10011001,B10011001,B10100101,B01000010,B00111100};
byte heart[] = {B00000000,B01100110,B11111111,B11111111,B01111110,B00111100,B00011000,B00000000};

// Mathworks guide referenced throughout the code: https://se.mathworks.com/help/thingspeak/control-a-light-with-talkback-and-esp32.html

/////////////////////////////////////
/////// Initialize Thingspeak ///////
/////////////////////////////////////
// The below lines 56-63 is taken from the course Thingspeak.ino file:
#define ssid "Christian - iPhone" //Indsæt navnet på jeres netværk her
#define password "04130413" //Indsæt password her

WiFiClient client;

unsigned long channelID = 1671827; //your channel
const char * myWriteAPIKey = "BSZB7OSERJ3O70S9"; // your WRITE API key const char *
const char * server = "api.thingspeak.com";

// Two lines below from mathworks to setup talkback: 
unsigned long myTalkBackID = 45773;
const char * myTalkBackKey = "XMDLP9ZTHSXRY7WL";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(10);

  //Wakeup call for The MAX72XX
  lc.shutdown(0,false);
  // Set the brightness to medium value
  lc.setIntensity(0,15);
  display_icon(happy);
  delay(5000);
  // Finally Clear Display
  lc.clearDisplay(0);

  // Define pinmode for toch-sensor
  pinMode(buttonPin, INPUT);

  // Start the millis:
  startMillis = millis();

  // Begin temperature sensor
  sensors.begin(); 

  // Do an initial reading before wifi is connected to confirm that everything works:
  Serial.println("INITIAL READINGS: ");
  temperature = read_temperature();
  sunlight = read_sunlight();
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Sunlight: ");
  Serial.println(sunlight);

  // Connecting to WiFi network. Below lines 102 to 118 is taken from the Thingspeak.ino file from the course:
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA); // From the mathworks guide

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

}

void loop() {
  // put your main code here, to run repeatedly:
  currentMillis = millis();

  // Check talkback:
  // Below from mathworks guide:
  String message = "None";
  
  // Clear display of heart if it has been shown for more than seven seconds.
  if ((currentMillis > heart_time) && !lc_on) {
      lc.clearDisplay(0);
  }
  
  // Check if five minutes have passed since last talkback check:
  if (currentMillis >= (talkback_interval + talkbackMillis)){
    Serial.println("Checking talkback");
    talkbackMillis = millis();
    message = check_talckback();
   
  }
  
  if (message.indexOf("HEART_ON") > 0) {
    // As this does not depend on counter we manually set intensity of sunlight reading:
    sunlight = read_sunlight();
    int intensity = map(sunlight, 500, 4095, 3, 15);
    lc.setIntensity(0,intensity);
    Serial.println("SHOWING HEART");
    display_icon(heart);
    heart_time = millis() + 7000;
  }

  // Check if fifteen minutes have passed since last measurement:
  if (currentMillis >= (measurement_interval + measurementMillis)){
      // Measure:
      temperature = read_temperature();
      Serial.print("Temperature: ");
      Serial.println(temperature);
      sunlight = read_sunlight();


      // The below code lines 160-168 is heavily inspired by the Thingspeak.ino file given in the course:
      ThingSpeak.begin(client);
      if (client.connect(server, 80)) {
        Serial.print("Sending to thingspeak ...");
        ThingSpeak.setField(1,temperature);
        ThingSpeak.writeFields(channelID, myWriteAPIKey);
        Serial.print("Sent!");
      
      }
      client.stop();
      
      // After measurement is taken, determine whether or not the display should change:
      if (increment(temperature, temp_threshold)){
        counter_temp += 1;
        Serial.print("Counter Temp: ");
        Serial.println(counter_temp);
      }
  measurementMillis = millis();
  }
  
  // Reset display if symbol has been shown for seven seconds:
  if ((lc_on) && (currentMillis > old_time))
  {
    Serial.println("Resetting display");
    lc.clearDisplay(0);
    lc_on = false;
  }
  else if (!lc_on)
  {
    buttonState = digitalRead(buttonPin); //If button is pushed, and display is off, activate display.
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

// Temperature
int read_temperature(){
  sensors.requestTemperatures(); 
  return sensors.getTempCByIndex(0);
}

// Sunlight
int read_sunlight(){
  return analogRead(photoPin);
}

// Activate display
void display(int counter, int sunlight_reading){
  int intensity = map(sunlight_reading, 500, 4095, 3, 15);
  Serial.print("Intensity");
  Serial.println(intensity);
  lc.setIntensity(0,intensity);
  if (counter < 8){
    display_icon(happy);

  } else {
    display_icon(sad);
  }
}

// Display icon
void display_icon(byte icon[]){
  for(int i = 0; i < 8; i++){
    lc.setRow(0,i,icon[i]);
  }
}

// Counter incrementation
bool increment(int measurement, float measurement_threshold){
  if (measurement >= measurement_threshold) {
    return true;
  }
  else {return false;}
}


// Talkback setup function taken form mathworks, edited to be compatible with our code and thingspeak signal:
String check_talckback(){
  // Create the TalkBack URI
  String tbURI = String("/talkbacks/") + String(myTalkBackID) + String("/commands/execute");
  
  // Create the message body for the POST out of the values
  String postMessage =  String("api_key=") + String(myTalkBackKey);                      
                       
   // Make a string for any commands that might be in the queue
  String newCommand = String();

  // Make the POST to ThingSpeak
  int x = httpPOST(tbURI, postMessage, newCommand);
  client.stop();
  
  // Check the result
  if(x == 200){
    Serial.println("checking queue..."); 
    // check for a command returned from TalkBack
    if(newCommand.length() != 0){

      Serial.print("  Latest command from queue: ");
      Serial.println(newCommand);
      return newCommand;
    }
    else{
      Serial.println("  Nothing new.");
      return "None";
    }
    
  }
  else{
    Serial.println("Problem checking queue. HTTP error code " + String(x));
    return "None";
  }
}

// General function to POST to ThingSpeak taken from mathworks:
int httpPOST(String uri, String postMessage, String &response){

  bool connectSuccess = false;
  connectSuccess = client.connect("api.thingspeak.com",80);

  if(!connectSuccess){
      return -301;   
  }
  
  postMessage += "&headers=false";
  
  String Headers =  String("POST ") + uri + String(" HTTP/1.1\r\n") +
                    String("Host: api.thingspeak.com\r\n") +
                    String("Content-Type: application/x-www-form-urlencoded\r\n") +
                    String("Connection: close\r\n") +
                    String("Content-Length: ") + String(postMessage.length()) +
                    String("\r\n\r\n");

  client.print(Headers);
  client.print(postMessage);

  long startWaitForResponseAt = millis();
  while(client.available() == 0 && millis() - startWaitForResponseAt < 5000){
      delay(100);
  }

  if(client.available() == 0){       
    return -304; // Didn't get server response in time
  }

  if(!client.find(const_cast<char *>("HTTP/1.1"))){
      return -303; // Couldn't parse response (didn't find HTTP/1.1)
  }
  
  int status = client.parseInt();
  if(status != 200){
    return status;
  }

  if(!client.find(const_cast<char *>("\n\r\n"))){
    return -303;
  }

  String tempString = String(client.readString());
  response = tempString;

  Serial.println("The httpPost has run");
  return status;
}
