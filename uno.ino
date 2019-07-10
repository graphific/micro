//ARDUINO
//working keypad, lcd, uart communication, hardcoded code top up, menu dynamic values, getting sensor values from mkr

/*
   Sketch uses 21888 bytes (67%) of program storage space. Maximum is 32256 bytes.
  Global variables use 1389 bytes (67%) of dynamic memory, leaving 659 bytes for local variables. Maximum is 2048 bytes.
  TODO: make smaller and refactor

  no custom chars in menu and item removes
  Sketch uses 21572 bytes (66%) of program storage space. Maximum is 32256 bytes.
  Global variables use 1329 bytes (64%) of dynamic memory, leaving 719 bytes for local variables. Maximum is 2048 bytes.

  no void printvals()
  Sketch uses 21394 bytes (66%) of program storage space. Maximum is 32256 bytes.
  Global variables use 1297 bytes (63%) of dynamic memory, leaving 751 bytes for local variables. Maximum is 2048 bytes.

  no digital clock display
  Sketch uses 21048 bytes (65%) of program storage space. Maximum is 32256 bytes.
  Global variables use 1277 bytes (62%) of dynamic memory, leaving 771 bytes for local variables. Maximum is 2048 bytes.

  adding LCD menu, removing old one
  Sketch uses 20814 bytes (64%) of program storage space. Maximum is 32256 bytes.
  Global variables use 1325 bytes (64%) of dynamic memory, leaving 723 bytes for local variables. Maximum is 2048 bytes.
  //just refactor existing menu probably better
  =reverted

  //normal serial local commands = commented:
  Sketch uses 20862 bytes (64%) of program storage space. Maximum is 32256 bytes.
  Global variables use 1245 bytes (60%) of dynamic memory, leaving 803 bytes for local variables. Maximum is 2048 bytes

  //TODO = refactor to:
  1. keypressed function: switch case x y z
  2. showmenu(itemnr) function
  3. receiveSerialCmd function (from mkr)
  3. sendSerialCmd function (to mkr)

  Step1: send cmd as multiple params just 1x!
  Sketch uses 19660 bytes (60%) of program storage space. Maximum is 32256 bytes.
  Global variables use 1062 bytes (51%) of dynamic memory, leaving 986 bytes for local variables. Maximum is 2048 bytes.

  Step2: new DrawMenu():
  wip

  Step3 adding
  #include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
  #include "Timer.h"
  Timer t;
  Sketch uses 20826 bytes (64%) of program storage space. Maximum is 32256 bytes.
  Global variables use 1416 bytes (69%) of dynamic memory, leaving 632 bytes for local variables. Maximum is 2048 bytes.
  = now no problem!

*/

//TODO: make refreshMenu int of which menu item to refresh instead of all = flicker per incoming new item
//or have a longer delay than all new incoming sensor items

//TODO: timeNotSet not changing
//TODO: only sync RTC every so many minutes...
//TODO: if no good time, all good, but cannot use offline credit codes
//TODO: test of gsm doesnt send data (=no correct menu values) = restart??


// SRATCHCODE CRYPTO IMPORTS
#include <AESLib.h>
#include <EEPROM.h>
int eeAddress = 512;
bool bwritePROM = false; //EDITABLE
//vars to/from EEPOM:
uint8_t key[16];
//uint8_t ivv[16]; //skip = same as key
char secretadd[5];
char checkcode[5];
//uint8_t scratch[12];
bool testCrypt = true;
unsigned long ee_epoch; //time to save to mem

const PROGMEM char HEX_VALUES[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
#define CBLOCK (1 * N_BLOCK) + 1
// END SRATCHCODE CRYPTO IMPORTS

//TIME
#include <TimeLib.h>

//#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
#include "Timer.h"
Timer t;
//END TIME

//COMMUNICATION
#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); //RX,TX
//normal serial
//bool show = false;
//String vals="";
//String old_vals = "";

//rx/tx serial mkr <> uno:
//bool show2 = false;
String cmd_vals = ""; //stores incoming serial cmd string
//String old_vals2 = "";

//bool startSig = false;
//bool execute = false;
//int counter = 0;
//String curcmd = "";
//---------------
//UPDATED BY GSM:
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

bool refreshMenu = true;

///--------------

//=== LCD ===
//custom characters
/*
  Based on Adafruit's example at
  https://github.com/adafruit/SPI_VFD/blob/master/examples/createChar/createChar.pde

  This example code is in the public domain.
  http://www.arduino.cc/en/Tutorial/LiquidCrystalCustomCharacter

  Also useful:
  http://maxpromer.github.io/LCD-Character-Creator/
*/

// make some custom characters (rm for now)
  

// The LCD library
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

// === KEYPAD ===

#include <Keypad.h>
const byte ROWS = 4;
const byte COLS = 4;
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {8, 7, 6, 5};
byte colPins[COLS] = {4, 3, 2, 1};
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// === MENU VARS ===
String Values = "";
int Menu = 0; //which menu item is active
String Val;
int Topping = 0;
String oldValues = "";
int budget_days = 3;

//=== END INIT OF VARS ===

//temp dump:
  //Timer.h / DS1307RTC.h
  //setSyncProvider(RTC.get);   // the function to sync the time from the RTC
  //setSyncProvider( requestSync);  //set function to call when sync required
  //setSyncProvider(RTC.get);
  
  //clearPROM();

      
/* *************************** /*
 *  SETUP() CODE
 * *************************** */

void setup() {
  setupSerials();
  setupPROM();
  setupLCD();
  runPossibleTests();  //TODO run if asked for by * press

  //events running on a timer:
  int tickEvent = t.every(1000*60, AskMkrData);//every minute
}

void loop() {
  //counter++;
  t.update();
  
  char customKey = customKeypad.getKey();
  
  if (customKey != NO_KEY) { //TODO: BELOW ALWAYS RUNS = BAD?
    keyPressed(customKey);
    //drawMenu();
  }

  while (mySerial.available()) { //cmd
    //int val = Serial.read();
    char val = (char)mySerial.read();
    //Serial.println(cmd_vals);
    if (val == '[') {
      cmd_vals = "";
    } else if (val == ']') {
      parseCmd(cmd_vals);
    } else {
      cmd_vals += val;
    }
  }

  //delay(20);

}
// == END LOOP() ===


/* *************************** /*
 *  INIT CODE
 * *************************** */
void setupSerials() {
  Serial.begin(57600);
  while (!Serial) {};
  Serial.println("hello i am arduino");
  //Serial1.begin(9600);

  //softwareserial
  mySerial.begin(57600);
  while (!mySerial) {};
  mySerial.println("hello mySerial");
}

void setupPROM() {
  if (bwritePROM) {
    writePROM();
  }
  readPROM();
}

void setupLCD() {
  lcd.begin(16, 2); // sets up the LCD's number of columns and rows
  lcd.print("Booting up...");
  //lcd.clear();
}

void runPossibleTests() {
  if (testCrypt) {
    readPROM();
    testCrypt = false;
    crypt("118697312340");//correct
    crypt("118697322340");//incorrect
    crypt("118697312340");//correct
    crypt("118697322340");//incorrect
  }
}
// === END GENERAL INIT/STARTUP CODE ===


void AskMkrData() {
  Serial.println("sending TEST&"); //& is ending char
  mySerial.print("TEST&");
}

/* *************************** /*
 *  MENU CODE
 * *************************** */

void drawMenu() {

  lcd.clear();
  if (Menu == 0) {
    lcd.setCursor(0, 0);
    lcd.print("* to top up");

    lcd.setCursor(0, 1);
    lcd.print("Credits: ");
    lcd.print(budget_days);
    lcd.print(" days");
  }
  else if (Menu == 1) {
    lcd.setCursor(0, 0);
    //lcd.print("2.Stat");
    if (solar_w > 0) {
      lcd.print("Solar: y");
      lcd.setCursor(0, 1);
      lcd.print("Gen: " + String(solar_w, 2) + "w");
    } else {
      lcd.print("Solar: n");
      lcd.setCursor(0, 1);
      lcd.print("Gen: 0w");
    }
  }
  else if (Menu == 2) {
    lcd.setCursor(0, 0);
    //lcd.print("3.Settings");
    if (load_w > 0) {
      lcd.print("Usage: y");
      lcd.setCursor(0, 1);
      lcd.print("Load:" + String(load_w, 1) + "w");
    } else {
      lcd.print("Usage: n");
      lcd.setCursor(0, 1);
      lcd.print("Load: 0w");
    }
    lcd.setCursor(10, 0);
    if (load_lv_w > 0) {
      lcd.print("|lv: y");
    } else {
      lcd.print("|lv: n");
    }
    lcd.setCursor(10, 1);
    if (load_hv_w > 0) {
      lcd.print("|hv: y");
    } else {
      lcd.print("|hv: n");
    }
  }
  else if (Menu == 3) {
    lcd.setCursor(0, 0);
    lcd.print("Bat: " + String(bat_p, 1) + "% " + String(bat_wh, 0) + "wh");
    lcd.setCursor(0, 1);
    lcd.print("Time Left: " + String(bat_time, 1) + "h");
  }
  else if (Menu == 4) {
    lcd.setCursor(0, 0);
    lcd.print("Humid: " + String(humidity, 2) + "%");
    lcd.setCursor(0, 1);
    lcd.print("Temp:  " + String(bat_temp, 2) + "C");
  }
  else if (Menu == 5) {
    String hh = String(hour());
    if (hh.length() < 2)
      hh = '0' + hh;
    String mm = String(minute());
    if (mm.length() < 2)
      mm = '0' + mm;
    String ss = String(second());
    if (ss.length() < 2)
      ss = '0' + ss;
    String dd = String(day());
    if (dd.length() < 2)
      dd = '0' + dd;
    String mn = String(month());
    if (mn.length() < 2)
      mn = '0' + mn;
    String yy = String(year());

    lcd.setCursor(0, 0);
    lcd.print(dd + "-" + mn + "-" + yy);
    //lcd.setCursor(0, 1);
    //lcd.print(hh+":"+mm+":"+ss);
    lcd.setCursor(11, 0);
    lcd.print(hh + ":" + mm);
  }
  else if (Menu == 6) {
    /*lcd.setCursor(0, 0);
      lcd.print("Stats"); //Values);

      lcd.setCursor(0, 1);
      lcd.print("Uptime: ");
      lcd.print(millis() / 1000);

      //lcd.setCursor(8, 0);
      //lcd.print("| test");

      //lcd.setCursor(8, 1);
      //lcd.print("| test2"); */

    //FF DISABLE
    /*
      //custom hello
      // create a new character
      lcd.createChar(0, heart);
      // create a new character
      lcd.createChar(1, smiley);
      // create a new character
      lcd.createChar(2, frownie);
      // create a new character
      lcd.createChar(3, armsDown);
      // create a new character
      lcd.createChar(4, armsUp);

      // set the cursor to the top left
      lcd.setCursor(0, 0);

      // Print a message to the lcd.
      lcd.print("I ");
      lcd.write(byte(0)); // when calling lcd.write() '0' must be cast as a byte
      lcd.print(" Sunshine ");
      lcd.write((byte)1);

      lcd.setCursor(4, 1);
      lcd.print("Labs!");

      if(armsUpb) {
      lcd.setCursor(2, 1);
      // draw him arms up:
      lcd.write(4);

      lcd.setCursor(10, 1);
      lcd.write(3);
      } else {
      lcd.setCursor(2, 1);
       // draw the little man, arms down:
      lcd.write(3);
      lcd.setCursor(10, 1);
      lcd.write(4);
      }*/
  }
  else if (Menu == 7) { //topping up
    lcd.setCursor(0, 0);
    lcd.print("Type in code");

    lcd.setCursor(0, 1);
    lcd.print(Values);
  }
  if (Menu == 8) { //topping up: step2 entered code
    lcd.setCursor(0, 0);
    lcd.print("processing");

    if (Val == "*" and Values != "*") { //* in there but not only *
      //Serial.print("code:");
      Serial.println(Values);

      if (Values == "12345*") {
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("correct");
        budget_days += 3;
        delay(1000);
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("topped up");
        delay(1000);
        Menu = 0;
      } else {
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("error");
        delay(1000);
        lcd.clear();
        Val = "*";
        Menu = 7;
      }
      Values = "";
    }
  }//end menu cases
}
// === END MENU FUNCTIONS


/* *************************** /*
 *  KEY CODE
 * *************************** */

void keyPressed(char customKey) {
  Serial.print("keyPressed() ");
  Serial.println(customKey);

  if (customKey == '#') { //nav menu
    Serial.print("pressed #, Menu:");
    Serial.println(Menu);
    if (Menu > 5) {
      Menu = 0;
    } else {
      Menu += 1;
    }
    //lcd.clear();
  } else {
    Values += customKey;
  }

  if (customKey == '*') { //top up
    if (Topping == 0) { //starting out
      Menu = 7;
      Topping = 1;
      Values = "";
      lcd.clear();
    } else if (Topping == 1) { //entered code lets check
      Menu = 8;
      Topping = 0;//back to normal after
      lcd.clear();
    }
  }

  drawMenu();
  //refreshMenu = false;
}
// === END KEY CODE ===

/* *************************** /*
 *  EXT COMMUNICATION CODE
 * *************************** */
 
/* commands
   [r123456] = epoch time
   [s25.00;a0.50;b12.03;t30.30;l0.00;h0.00;m40.00] = solar_w;solar_a;bat_v;bat_temp;load_lv_w;load_hv_w;humidity;
*/
int MAXSIZE = 10;
char * stripCmd(char *input) {//strip first char
  memmove(input, input + 1, (MAXSIZE - 1) / sizeof(input[0]));
  //Serial.println(input);
  return input;
}

void parseCmd(String vals) {
  Serial.println(vals);
  int str_len = vals.length() + 1;
  char char_array[str_len];
  vals.toCharArray(char_array, str_len);
  //Serial.println(char_array);
  char buf[sizeof(char_array)];
  vals.toCharArray(buf, sizeof(buf));
  //Serial.println(buf);
  char *p = buf;
  char *str;
  while ((str = strtok_r(p, ";", &p)) != NULL) // delimiter is the semicolon
    //Serial.println(str);
    switch (str[0]) {
      case 'r'://rtc epoch
        Serial.println("Setting RTC as command from MKR");
        setRTC(atof(stripCmd(str)));
        //TODO: save as ee_epoch and to mem
        break;
      case 's':
        solar_w = atof(stripCmd(str));//tofloat
        //Serial.println(solar_w);
        break;
      case 'a':
        solar_a = atof(stripCmd(str));//tofloat
        break;
      case 'b':
        bat_v = atof(stripCmd(str));//tofloat
        
        bat_p = 80; //battery capacity on percentages (calculated from bat_v)
        bat_wh = 800; //battery wh left, calculated from bat_p*bat_ah*12 (bat voltage)
        if(load_w>0) {
          bat_time = bat_wh/load_w; //battery time left in h at current load, calculated from bat_wh and load_w
        } else {
          //TODO: take avg of period (10w?)
          bat_time = bat_wh/10;
        }
        break;
      case 't':
        bat_temp = atof(stripCmd(str));//tofloat
        break;
      case 'l':
        load_lv_w = atof(stripCmd(str));//tofloat
        break;
      case 'h':
        load_hv_w = atof(stripCmd(str));//tofloat
        break;
      case 'm':
        humidity = atof(stripCmd(str));//tofloat
        break;
    }
}
// === END EXT COMMUNICATION CODE ===


/* *************************** /*
 *  TIME CODE
 * *************************** */

/*time_t requestSync()
  {
  Serial.println("uno: I want to sync time!");
  }
*/

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year());
  Serial.println();
  }

  void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
  }

void setRTC(unsigned long epoch) {
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013
  //rtc.setEpoch(epoch); // from GPS
  if ( epoch >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
    setTime(epoch); // Sync Arduino clock to the time received on the serial port

    //RTC.set(epoch);
  }
  //Serial.print("Unix time = ");
  //digitalClockDisplay();

  Serial.print("saving to EEPROM...");
  eeAddress = 30;
  //char esecretadd[5] = "3454"; //{3,4,5,4}; //"3454"; //{4,3,2,1};
  EEPROM.put(eeAddress, epoch);

}
// === END TIME FUNCTIONS ===


/* *************************** /*
 *  CRYPTO CODE
 * *************************** */
 
String smart_shorten_simple(String res, int lenn) {

  String new_string = "";
  //for (int i = 6; i < res.length(); i++){
  for (int i = 6; i < 6 + lenn; i++) {
    char c = res.charAt(i);
    new_string += c;
  }
  //Serial.println(new_string);
  return new_string;
}


//char serialBuffer[120];

void ByteToHexString(char * hexStrParam, unsigned char * byteArrayParam, unsigned int byteArrayLength)
{
  unsigned char num;

  for (int i = 0, u = 0; i < byteArrayLength; i++, u++)
  {
    num = byteArrayParam[i] >> 4;
    hexStrParam[u] = (char)pgm_read_byte(HEX_VALUES + num);
    num = byteArrayParam[i] & 0xf;
    hexStrParam[++u] = (char)pgm_read_byte(HEX_VALUES + num);
  }
}
void crypt(char code[12]) {//causes some issues for restarting=when done save to eeprom and restart?
  //uint8_t key[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
  //uint8_t iv[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
  //char data[16];
  Serial.print("code ");
  Serial.print(code);
  Serial.print(": ");

  //char code[12] = "118697312340";//gives B9E1 ok even without memset 0
  String full_code = "";
  for (int i = 0; i < 12; i++) {
    full_code += code[i];
  }
  for (int i = 0; i < 4; i++) {
    //Serial.println(secretadd[i]);
    full_code += secretadd[i];
  }
  full_code += '\n';
  //Serial.println(full_code);
  char copy[17];
  full_code.toCharArray(copy, 17);

  char data[16];// = full_code; //"1186973123403454";//gives B9E1 ok even without memset 0
  //char *data = "1186973123403454"; //has null terminator? gives 3131
  aes_context ctx;
  char auxBuffer[16 * 3]; //129

  memset(data, 0x00, 16);
  //memcpy(data, "1242847264023454", 16); //should give 1234
  //memcpy(data, "1186973123403454", 16); //should give B9E1 //always all caps!
  memcpy(data, copy, 16); //should give B9E1 //always all caps!

  /*Serial.print("secretadd2: ");
    for(int i = 0; i < 4; i++)
    {
    Serial.print(secretadd[i]);
    }
    Serial.println("");
    Serial.println(data);*/

  //ctx = aes128_cbc_enc_start((const uint8_t*)key, iv);
  ctx = aes128_cbc_enc_start((const uint8_t*)key, key);
  aes128_cbc_enc_continue(ctx, data, 16);
  aes128_cbc_enc_finish(ctx);

  memset(auxBuffer, 0x00, 16 * 3); //129);
  ByteToHexString(auxBuffer, (unsigned char *)data, sizeof(data));
  //sprintf(serialBuffer, "encrypted-cbc: %s", auxBuffer);
  //Serial.println(serialBuffer);

  String short_hash = smart_shorten_simple(auxBuffer, 4);
  //Serial.println(short_hash);
  if (short_hash == checkcode) {
    Serial.println("Correct: lets top up");
  } else {
    Serial.println("Incorrect: not topping up");
  }
}

// === END CRYPTO CODE ===


/* *************************** /*
 *  EEPROM MEMORY CODE
 * *************************** */
 
void clearPROM() {
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
}

void writePROM() {
  int eeAddress = 0;
  char ekey[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  EEPROM.put(eeAddress, ekey);
  //Serial.println(sizeof(ekey));

  /* eeAddress = 20;
    char eiv[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    EEPROM.put(eeAddress, eiv);
    Serial.println(sizeof(eiv));*/

  eeAddress = 16;
  char esecretadd[5] = "3454"; //{3,4,5,4}; //"3454"; //{4,3,2,1};
  EEPROM.put(eeAddress, esecretadd);
  //Serial.println(sizeof(esecretadd));

  eeAddress = 20;
  char echeckcode[5] = "B9E1";
  EEPROM.put(eeAddress, echeckcode);
  //Serial.println(sizeof(echeckcode));

  /*eeAddress = 24;
    char escratch[12] = {1,2,3,4,5,6,7,8,9,10,11,12};
    EEPROM.put(eeAddress, escratch);
    Serial.println(sizeof(escratch));*/

}

void readPROM() {
  int eeAddress = 0;
  EEPROM.get(eeAddress, key);

  //eeAddress = 20;
  //EEPROM.get(eeAddress, ivv);

  eeAddress = 16;
  EEPROM.get(eeAddress, secretadd);

  eeAddress = 20;
  EEPROM.get(eeAddress, checkcode);
  /*char echeckcode[5];
    memcpy( echeckcode, &checkcode[0], 4 );
    echeckcode[4] = '\0';
    checkcode = echeckcode*/
  //Serial.println(subbuff);
  //eeAddress = 24;
  //EEPROM.get(eeAddress, scratch);

  eeAddress = 40;
  //unsigned long ee_epoch;
  EEPROM.get(eeAddress, ee_epoch);
}

/*void printVals() {
  Serial.println("key: ");
  for(int i = 0; i < 16; i++)
  {
    Serial.print(key[i]);
  }
  Serial.println("");

  //iv=key

  Serial.println("secretadd: ");
  for(int i = 0; i < 4; i++)
  {
    Serial.print(secretadd[i]);
  }
  Serial.println("");

  Serial.println("checkcode: ");
  Serial.println(checkcode);
  /*for(int i = 0; i < 4; i++)
  {
    Serial.print(checkcode[i]);
  }
  Serial.println("");*/

/*
  Serial.println("scratch: ");
  for(int i = 0; i < 12; i++)
  {
  Serial.print(scratch[i]);
  }
  Serial.println("");*/
//Serial.println("---");
//}

// === END EEPROM MEMORY ===
