/*
 * 
 * Working Temperature, Humidity & Environment Monitoring
 * Arduino MKR GSM 1400 + Hologram Cloud + ThingSpeak
 * LDR+DHT11 Module+ MQ2 Module + MQ135 Module
 *
*/

// libraries
#include <MKRGSM.h>

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include "DHT.h"

#define DHTPIN 1     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

//Thing Speak Write Key
String ThingSpeakWriteKey="2LG8HFRI93Q9P5RR";

uint32_t delayMS;
String Values;
String MQ2_Val,MQ135_Val,LDR_Val;

const char PINNUMBER[] = " ";
// APN data
const char GPRS_APN[] = "hologram";
const char GPRS_LOGIN[] = " ";
const char GPRS_PASSWORD[] = " ";

//Hologram Config
String HOLOGRAM_DEVICE_KEY = "u_SVsfE1"; //"4F)o{}2_";
String HOLOGRAM_TOPIC = "MKR1400";

// initialize the library instance
GSMClient client;
GPRS gprs;
GSM gsmAccess;

// Hologram's Embedded API (https://hologram.io/docs/reference/cloud/embedded/) URL and port
char server[] = "cloudsocket.hologram.io";
int port = 9999;


void setup() {
  // initialize serial communications and wait for port to open:
  Serial.begin(9600);
 
  Serial.println(F("DHTxx test!"));

  dht.begin();
  
  Serial.println("Starting Arduino web client.");
  // connection state
  boolean connected = false;

  // After starting the modem with GSM.begin()
  // attach to the GPRS network with the APN, login and password
  while (!connected) {
     Serial.println("Begin GSM Access");
    if ((gsmAccess.begin() == GSM_READY) &&
        (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY)) {
      connected = true;
      Serial.println("GSM Access Success");
    } 
    else {
      Serial.println("Not connected");
      delay(1000);
    }
  }
  delay(5000);
}

void loop() {
    
    
    Values="api_key="+ThingSpeakWriteKey+"&";
    //Values="";
    delay(delayMS);

  /*
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  
    if (isnan(t)) {
      Serial.println("Error reading temperature!");
      Values+="field2=0&";
    }
    else {
      Serial.print("Temperature: ");
      Serial.print(t);
      Serial.println(" *C");
      Values+="field2="+String(t)+"&";
    }
  
    
    // Get humidity event and print its value.
    //dht.humidity().getEvent(&event);
    if (isnan(h)) {
      Serial.println("Error reading humidity!");
      Values+="field3=0&";
    }
    else {
      Serial.print("Humidity: ");
      Serial.print(h);
      Serial.println("%");
      Values+="field3="+String(h)+"&";
    }
  
    //Get Light Level
    //Serial.print("Light Level: ");
    //LDR_Val=String(analogRead(LDR));
    //Serial.println(LDR_Val);
    LDR_Val=String(0);
    Values+="field1="+LDR_Val+"&";
    
    //Get Smoke Level
    //MQ2_Val=String(analogRead(MQ2));
    //Serial.print("MQ2-Gas Level: ");
    //Serial.println(MQ2_Val);
    MQ2_Val=String(0);
    Values+="field4="+MQ2_Val+"&";
    
    //Get Environment Level
    //MQ135_Val=String(analogRead(MQ135));
    //Serial.print("MQ135-Env Level: ");
    //Serial.println(MQ135_Val);
    MQ135_Val=String(0);
    Values+="field5="+MQ135_Val;
    */

    Values = "test";
    
    //Send the Data to Hologram Network
    if (client.connect(server, port)) {
      Serial.println("connected");
      // Send a Message request:
      client.println("{\"k\":\"" + HOLOGRAM_DEVICE_KEY +"\",\"d\":\""+ Values+ "\",\"t\":\""+HOLOGRAM_TOPIC+"\"}");
    } else {
      // if you didn't get a connection to the server:
      Serial.println("connection failed");
    }
    
    client.stop();
    Serial.println("sleeping 10s...");
    delay(10000);
    Serial.println("woken up...");
}
