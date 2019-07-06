//MKR GSM

//TODO: Voltage Meter Sensor
//TODO: show GSM status in LCD menu on UNO
//TODO: led for GSM status
//TODO: multiple values to hologram or aws which we can plot
//TODO: allow offline working, but set in restricted regime
//TODO: power off if no temperature readings for a while??
//TODO: set current datetime in uno LCD menu + update uno from GSM GPS time

#include <math.h>

//---- START HOLOGRAM SIM ---
bool sendingOK=false; //static setting: do we want to send stuff to the hologram cloud?
// libraries
#include <MKRGSM.h>
boolean connected = false;
String Values;

String IMEI = "";
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
// modem verification object
GSMModem modem;
GSMServer gsmserver(80); // port 80 (http default)

// Hologram's Embedded API (https://hologram.io/docs/reference/cloud/embedded/) URL and port
char server[] = "cloudsocket.hologram.io";
int port = 9999;

// --- END HOLOGRAM

// VOLTAGE SENSOR TODO doesnt work
// voltage sensor: http://henrysbench.capnfatz.com/henrys-bench/arduino-voltage-measurements/arduino-25v-voltage-sensor-module-user-manual/
//https://www.electronicshub.org/interfacing-voltage-sensor-with-arduino/

/*
const int voltageSensor = A1;
float vOUT = 0.0;
float vIN = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;
int value = 0;
// end Voltage sensor*/

int counter = 0;
String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

const float bat_ah = 100; //our total battery capacaity

float solar_w = 30;
float solar_a = 1.3;
float bat_v = 12.1; //battery voltage, calculate percentage by lifepo nrs
float bat_p = 80; //battery capacity on percentages (calculated from bat_v)
float bat_wh = 800; //battery wh left, calculated from bat_p*bat_ah*12 (bat voltage)
float bat_time = 20.3; //battery time left at current load, calculated from bat_wh and load_w
float bat_temp = 30.3; //battery temperature in C (sensor)
float load_w = 60; //current usage in watt calculated from load_lv_w and load_hv_w
float load_lv_w = 20;
float load_hv_w = 40;
bool load_lv_b = true; //true if load_lv_w > passive threshold
bool load_hv_b = true; //true if load_hv_w > passive threshold
float humidity = 40; //humidity in percentage (sensor)

//-- DHT
//3 pin version of DHT11
// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include "DHT.h"
#define DHTPIN 6     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

#define DHTPIN2 7     // Digital pin connected to the DHT sensor
#define DHTTYPE2 DHT22   // DHT 22  (AM2302), AM2321
DHT dht2(DHTPIN2, DHTTYPE2);
//-- END DHT

// SET TIME from GPS
#include <RTCZero.h>
/* Create an rtc object */
RTCZero rtc;
//to set internal rtc clock after we get it from gps
//// END TIME

void setup() {
  Serial.begin(57600);
  while(!Serial){};
  
  inputString.reserve(200);
  
  Serial.println("hello serial");

  Serial1.begin(57600);
  while(!Serial1){};
  Serial1.write("hello serial1");

  dht.begin();
  dht2.begin();

  rtc.begin(); // initialize RTC

  // start modem test (reset and check response)
  Serial.print("Starting modem test...");
  if (modem.begin()) {
    Serial.println("modem.begin() succeeded");
  } else {
    Serial.println("ERROR, no modem answer.");
  }
  checkIMEI();
  
  Serial.println("Starting Arduino web client.");
  // connection state
  
  // After starting the modem with GSM.begin()
  // attach to the GPRS network with the APN, login and password
  while (!connected) {
     Serial.println("Begin GSM Access");
    if ((gsmAccess.begin() == GSM_READY) &&
        (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY)) {
      connected = true;
      Serial.println("GSM Access Success");

      // start server
      gsmserver.begin();

      //Get IP.
      IPAddress LocalIP = gprs.getIPAddress();
      Serial.println("Server IP address=");
      Serial.println(LocalIP);

      unsigned long epoch = gsmAccess.getLocalTime();

      Serial.print("gsmAccess.getLocalTime=");
      //year
      Serial.println(epoch);

      Serial.println("setting RTC time...");
      rtc.setEpoch(epoch); // from GPS

      Serial.print("Unix time = ");
      Serial.println(rtc.getEpoch());
    
      //Serial.print("Seconds since Jan 1 2000 = ");
      //Serial.println(rtc.getY2kEpoch());
    
      // Print date...
      Serial.print(rtc.getDay());
      Serial.print("/");
      Serial.print(rtc.getMonth());
      Serial.print("/");
      Serial.print(rtc.getYear());
      Serial.print("\t");
    
      // ...and time
      print2digits(rtc.getHours());
      Serial.print(":");
      print2digits(rtc.getMinutes());
      Serial.print(":");
      print2digits(rtc.getSeconds());
    
      Serial.println();

      Serial.println("sending RTC time to uno...");
      Serial.println("[r"+String(epoch)+"]");
      Serial1.print("[r"+String(epoch)+"]");
      //Serial1.print("[r"+String(rtc.getHours())+";"+String(rtc.getHours())+";"+String(rtc.getMinutes())+";"+String(rtc.getSeconds())+";"+String(rtc.getDay())+";"+String(rtc.getMonth())+";"+String(rtc.getYear())+"]");
      //Hour, Minute, Second, Day, Month, Year
      //delay(1500);
      
      
      /*
      //date
      Serial.print("The UTC date is ");
      //year
      Serial.println((epoch / 31556926)+1970);

      // print the hour, minute and second:
      Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
      Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
      Serial.print(':');
      if ( ((epoch % 3600) / 60) < 10 ) {
        // In the first 10 minutes of each hour, we'll want a leading '0'
        Serial.print('0');
      }
      Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
      Serial.print(':');
      if ( (epoch % 60) < 10 ) {
        // In the first 10 seconds of each minute, we'll want a leading '0'
        Serial.print('0');
      }
      Serial.println(epoch % 60); // print the second*/

      

    } 
    else {
      Serial.println("Not connected");
      delay(1000);
    }
  }
  //delay(5000);
  //TODO if cannot connect can work offline as well, except no mobile topping up
  
  
}
 
void loop() {
  counter++;

  //GSMClient client = gsmserver.available();

  //if (client) {
  //  if (client.available()) {
  //    Serial.write(client.read());
  //  }
  //}
  
  while (Serial1.available()) {
    // get the new byte:
    char inChar = (char)Serial1.read();
    // add it to the inputString:
    
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '&') {
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }
  
  // print the string when a newline arrives:
  if (stringComplete) {
    Serial.println(inputString);
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
  
  
  if(counter>20) {//time to update stats
    counter=0;

    /*
    //Voltage
    Serial.println("reading Voltage");
    //value = analogRead(voltageSensor);
    //vOUT = (value * 5.0) / 1024.0;
    //vIN = vOUT / (R2/(R1+R2));
    //Serial.println(value);
    int val11=analogRead(8);
    float temp=val11/4.092;
    float val2=(temp/10);
    Serial.println(val2);*/
    
    //DHT
    Serial.println("reading DHT");
    readDHT();
    //END DHT
    
    simulateSettings();
    //bat 12.1v,solar 20w always 5 chars 1.2a
    //b12.1s0020a01.2a
    //execute=[g for go [g1=...
    
    //values to log inside gsm to get avg when logging and sending to cloud
    //sending here to arduino to show it on lcd display and controls
    //1sided communication for now = cannot change settings?
    //need to change/enter phone number? = do at office or by mobile phone?
    //need to change/enter credit code that works offline = logic on arduino
    
    Serial1.print("[s"+String(solar_w)+"]");
    delay(100);
    Serial1.print("[a"+String(solar_a)+"]");
    delay(100);
    Serial1.print("[b"+String(bat_v)+"]");
    delay(100);
    Serial1.print("[t"+String(bat_temp,3)+"]");
    delay(100);
    Serial1.print("[l"+String(load_lv_w,3)+"]");
    delay(100);
    Serial1.print("[h"+String(load_hv_w,3)+"]");
    delay(100);
    Serial1.print("[m"+String(humidity)+"]");
    delay(100);
    //bat_p, bat_wh, bat_time and others calculated
    //TIME
    //Serial1.print("[r"+String(rtc.getHours())+";"+String(rtc.getHours())+";"+String(rtc.getMinutes())+";"+String(rtc.getSeconds())+";"+String(rtc.getDay())+";"+String(rtc.getMonth())+";"+String(rtc.getYear())+"]");
    Serial1.print("[r"+String(rtc.getEpoch())+"]");
    /*
    Serial1.print("[b12.1]"); 
    delay(100);
    Serial1.print("[s20]");
    delay(100);
    Serial1.print("[a1.2]");
    delay(100);
    Serial1.print("[a1.2]");
    delay(100);
    */

    //todo: serial send excute command rtc: datetime
    Serial.print("Unix time = ");
    Serial.println(rtc.getEpoch());
      
    //EXECUTE COMMANDS
    //can send commands to execute when demanded from the cloud or mobile (through cloud)
    //Serial1.print("[gcredit]");//
    //delay(1500);
    /*Serial1.print("[gtopup3d]");//when done through mobile phone = also enable if disabled = show new credit on LCD & play tune??
    delay(150);
    Serial1.print("[gdisable]");//power off when out of credit = disable if not disabled yet
    delay(150);
    Serial1.print("[genable]");//power off when out of credit = disable if not disabled yet
    delay(150);*/

    sendDataGsm();
  }
  delay(150);
  
}

void sendDataGsm() {
  if(connected & sendingOK) {
    //Send the Data to Hologram Network
    if (client.connect(server, port)) {
      Serial.println("connected");
      Values= "test message 2";
      // Send a Message request:
      client.println("{\"k\":\"" + HOLOGRAM_DEVICE_KEY +"\",\"d\":\""+ Values+ "\",\"t\":\""+HOLOGRAM_TOPIC+"\"}");
    } else {
      // if you didn't get a connection to the server:
      Serial.println("connection failed");
    }
    
    client.stop();
    //Serial.println("sleeping 10s...");
    //delay(10000);
    //Serial.println("woken up...");
  }
}

void checkIMEI() {
  Serial.print("Checking IMEI...");
  IMEI = modem.getIMEI();

  // check IMEI response
  if (IMEI != NULL) {
    // show IMEI in serial monitor
    Serial.println("Modem's IMEI: " + IMEI);
    // reset modem to check booting:
    Serial.print("Resetting modem...");
    modem.begin();
    // get and check IMEI one more time
    if (modem.getIMEI() != NULL) {
      Serial.println("Modem is functoning properly");
    } else {
      Serial.println("Error: getIMEI() failed after modem.begin()");
    }
  } else {
    Serial.println("Error: Could not get IMEI");
  }
}
void simulateSettings() {
  //simple random for now
  
  //on,off?
  if(random(0, 10)>3) {
    solar_w = random(10, 30);
  } else {
    solar_w = 0;
  }
  if(solar_w > 3) {
    solar_a = random(10, 30)/20.0;
    solar_a = solar_a*(random(10, 20)/10);
  }
  //bat_v starts high and becomes less
  bat_v-=0.01;

  //commented = we got actual sensor data (DHT)!
  //bat_temp rand increase or decrease
  //if(random(0, 10)>5) {
  //  bat_temp += 0.1;
  //} else {
  //  bat_temp -= 0.1;
  //}
  
  //load_lv_w
  if(random(0, 10)>5) {
    load_lv_w = random(15, 30);
  } else {
    load_lv_w = 0;
  }
  
  //load_hv_w
  if( random(0, 10) > 5) {
    load_hv_w = random(30, 100);
  } else {
    load_hv_w = 0;
  }

  //commented = we got actual sensor data (DHT)!
  //humidity = random(30, 70);
}


void readDHT() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
    //TODO: set to resetting values to 0 as no readings if too many fail?
  }
  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));

  //DHT2
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h2 = dht2.readHumidity();
  // Read temperature as Celsius (the default)
  float t2 = dht2.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f2 = dht2.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(h2) || isnan(t2) || isnan(f2)) {
    Serial.println(F("Failed to read from DHT2 sensor!"));
    //return; //TODO tends to happen, ignore it? FIX?
    //TODO: set to resetting values to 0 as no readings if too many fail?
  }
  // Compute heat index in Fahrenheit (the default)
  float hif2 = dht.computeHeatIndex(f2, h2);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic2 = dht.computeHeatIndex(t2, h2, false);

  Serial.print(F("Humidity2: "));
  Serial.print(h2);
  Serial.print(F("%  Temperature2: "));
  Serial.print(t2);
  Serial.print(F("°C "));
  Serial.print(f2);
  Serial.print(F("°F  Heat index2: "));
  Serial.print(hic2);
  Serial.print(F("°C "));
  Serial.print(hif2);
  Serial.println(F("°F"));

  //TODO: for now take average, in real system 2 temps
  //1 for bat, 1 for housing
  //saving humidity, temp (TODO: heat index as well?)
  //to be send on simulate settings function
  humidity = (h+h2)/2;
  bat_temp = (t+t2)/2;
  
}

void print2digits(int number) {
  if (number < 10) {
    Serial.print("0");
  }
  Serial.print(number);
}
