// Arduino Uno

//Include LCD library
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(A0,A1,A2,A3,A4,A5);

//----

/*
#include <SoftwareSerial.h>
 
//SIM800 TX is connected to Arduino D8
#define SIM800_TX_PIN 9
 
//SIM800 RX is connected to Arduino D7
#define SIM800_RX_PIN 10
 
//Create software serial object to communicate with SIM800
SoftwareSerial serialSIM800(SIM800_TX_PIN,SIM800_RX_PIN);
*/

// Select your modem:
#define TINY_GSM_MODEM_SIM800
// #define TINY_GSM_MODEM_SIM808

// Increase RX buffer if needed
//#define TINY_GSM_RX_BUFFER 512

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>


// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Use Hardware Serial on Mega, Leonardo, Micro
//#define SerialAT Serial1

// or Software Serial on Uno, Nano
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(10, 9); // RX, TX

// Hologram Credentials
String api_key = "4wrGBTiGXwGHolBFNz7xB7Zfp6nHET";
String deviceid = "315886";


// Your GPRS credentials
const char apn[]  = "hologram";

// Server details
const char server[] = "dashboard.hologram.io";
const int  port = 443; // port 443 is default for HTTPS

TinyGsm modem(SerialAT);

TinyGsmClientSecure client(modem);
HttpClient http(client, server, port);

//----

#include <Keypad.h>

const byte ROWS = 4; 
const byte COLS = 4; 
String Values="";
int Menu = 0;
String Val;
int Topping = 0;
String oldValues = "";
int budget_days = 3;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {8, 7, 6, 5}; 
byte colPins[COLS] = {4, 3, 2, 1}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

void setup() {
  //Begin serial comunication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  while(!Serial);
  //Serial1.begin(9600);

  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  
  // Print a message to the LCD.
  lcd.print("Booting up...");
  delay(1000);

  //--- SIM
  /*
  //Being serial communication witj Arduino and SIM800
  serialSIM800.begin(9600);
  */
  setup_modem();
  delay(1000);
  Serial.println("Setup Complete!");
  lcd.clear(); 
  lcd.print("Sim Ok");
  delay(1000);

  //----
  
  lcd.clear(); 
}

void loop() {
  //SIM
  /*
  //Read SIM800 output (if available) and print it in Arduino IDE Serial Monitor
  if(serialSIM800.available()){
    Serial.write(serialSIM800.read());
  }
  //Read Arduino IDE Serial Monitor inputs (if available) and send them to SIM800
  if(Serial.available()){    
    serialSIM800.write(Serial.read());
  }*/

  // Connect Network
  connect_cellular();

  // Ready Data
  String data = String(analogRead(0));
  String tags = "arduino,hello_world";

  //Send to Hologram
  send_message(data, tags);

  //Disconnect GPRS and HTTP
  disconnect_networks();

  delay(120000); // Sleep for 2 minutes
  
  //END SIM -------
  
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):

  if(Values != oldValues) {
    /*Serial.print("Menu: ");
    Serial.println(Menu);
    Serial.print("Values: ");
    Serial.println(Values);
    Serial.print("Val: ");
    Serial.println(Val);
    Serial.print("Topping: ");
    Serial.println(Topping);
    Serial.println("---");*/
    oldValues = Values;
  }
  

  
  
  
  
  
  char customKey = customKeypad.getKey();
  
  if (customKey != NO_KEY){
    //Serial.println(customKey);
    Val=customKey;
    
    //lcd.setCursor(9, 1);
    //lcd.print("KEY= ");  
    //lcd.print(customKey);
    if(Val=="#") {
      Serial.println("pressed #");
      if(Menu > 4) {
        Menu = 0;
      } else {
        Menu += 1;
      }
      lcd.clear(); 
    } else {
      Values += customKey;
    }

    if(Val=="*") {
      if(Topping == 0) {//starting out
        Menu = 6;
        Topping = 1;
        Values = "";
        lcd.clear();    
      } else if(Topping == 1) {//entered code lets check
        Menu = 7;
        Topping = 0;//back to normal after
        lcd.clear();   
      }
    }
  }

  if(Menu==0) {
    lcd.setCursor(0, 0);
    lcd.print("* to top up");
    
    lcd.setCursor(0, 1);
    lcd.print("Credits: ");
    lcd.print(budget_days);
    lcd.print(" days"); 
  }
  if(Menu==1) {
    lcd.setCursor(0, 0);
    //lcd.print("2.Stat"); 
    lcd.print("Solar: y"); 
    
    lcd.setCursor(0, 1);
    lcd.print("Gen: 40w");
  }
  if(Menu==2) {
    lcd.setCursor(0, 0);
    //lcd.print("3.Settings"); 
  
    lcd.print("Usage:y"); 
    
    lcd.setCursor(0, 1);
    lcd.print("Load: 60w"); 

    lcd.setCursor(10, 0);
    lcd.print("|lv: y"); 

    lcd.setCursor(10, 1);
    lcd.print("|hv: y"); 
  }
  if(Menu==3) {
    lcd.setCursor(0, 0);
    
    lcd.print("Bat: 80%  800wh"); 
    
    lcd.setCursor(0, 1);
    lcd.print("Time Left:  20h"); 
  }
  if(Menu==4) {
    lcd.setCursor(0, 0);
    
    lcd.print("Humid: 40%"); 
    
    lcd.setCursor(0, 1);
    lcd.print("Temp:  20C"); 
  }
  if(Menu==5) {
    lcd.setCursor(0, 0);
    lcd.print("1.Menu"); //Values); 
    
    lcd.setCursor(0, 1);
    lcd.print("SEC= "); 
    lcd.print(millis() / 1000);

    lcd.setCursor(8, 0);
    lcd.print("| test"); 

    lcd.setCursor(8, 1);
    lcd.print("| test2"); 
  }
  if(Menu==6) { //topping up
    lcd.setCursor(0, 0);
    lcd.print("Type in code");
      
    lcd.setCursor(0, 1);
    lcd.print(Values); 
  }
  if(Menu==7) { //topping up: step2 entered code
    lcd.setCursor(0, 0);
    lcd.print("processing");

    if(Val=="*" and Values != "*") {//* in there but not only *
      //Serial.print("code:");
      Serial.println(Values);
      
      if(Values=="12345*") {
        lcd.clear();   
        lcd.setCursor(0, 1);
        lcd.print("correct");
        budget_days+=3;
        delay(1000);
        lcd.clear();   
        lcd.setCursor(0, 1);
        lcd.print("topped up");
        delay(1000);
        Menu=0;
      } else {
        lcd.clear();   
        lcd.setCursor(0, 1);
        lcd.print("error");
        delay(1000);
        lcd.clear();
        Val="*";
        Menu=6;
      }
      Values="";
    }
    
  }
}




void send_message(String data, String tags) {
  SerialMon.println("Sending Message to Hologram");

  String path = "/api/1/csr/rdm?apikey=" + api_key;
  String contentType = "application/x-www-form-urlencoded";
  
  String postData = "deviceid=" + deviceid + "&data=" + data;
  postData = postData + "&tags=" + tags;

  http.post(path, contentType, postData); //, contentType, buf

//  read the status code and body of the response
//  int statusCode = http.responseStatusCode();
//  String response = http.responseBody();

//  SerialMon.print("Status code: ");
//  SerialMon.println(statusCode);

//  SerialMon.print("Response: ");
//  SerialMon.println(response);

  delay(5000);
}

void setup_modem(){
  // Set console baud rate
  SerialMon.begin(115200);
  delay(10);

  // Set GSM module baud rate
  SerialAT.begin(115200);
  delay(3000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println(F("Initializing modem..."));
  modem.restart();

  if (!modem.hasSSL()) {
    SerialMon.println(F("SSL is not supported by this modem"));
    //while (true) {
    //  delay(1000);
   // }
  }
}

void connect_cellular(){
  SerialMon.print(F("Waiting for network..."));
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" OK");

  SerialMon.print(F("Connecting to "));
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, "", "")) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" OK");
}

void disconnect_networks(){
  SerialMon.println("Disconnecting");
  http.stop();
  SerialMon.println(F("Server disconnected"));

  modem.gprsDisconnect();
  SerialMon.println(F("GPRS disconnected"));
}
