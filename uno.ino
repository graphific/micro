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

  refactor (incl timer t there, but no DS1307)
  Sketch uses 19926 bytes (61%) of program storage space. Maximum is 32256 bytes.
  Global variables use 1227 bytes (59%) of dynamic memory, leaving 821 bytes for local variables. Maximum is 2048 bytes.

  time now re-set from memory
  Sketch uses 19000 bytes (58%) of program storage space. Maximum is 32256 bytes.
  Global variables use 1267 bytes (61%) of dynamic memory, leaving 781 bytes for local variables. Maximum is 2048 bytes.
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
bool bwritePROM = false; //EDITABLE
//vars to/from EEPOM:
uint8_t key[16];
char secretadd[5];
char checkcode[5];
bool testCrypt = true;
unsigned long ee_epoch; //time to save to mem

const PROGMEM char HEX_VALUES[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
#define CBLOCK (1 * N_BLOCK) + 1
// END SRATCHCODE CRYPTO IMPORTS

//TIME
#include <TimeLib.h>
#include "Timer.h"
Timer t;
//END TIME

//COMMUNICATION: rx/tx serial -> mkr <> uno:
#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); //RX,TX
String cmd_vals = ""; //stores incoming serial cmd string

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
#include <LiquidCrystal.h>
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
String typingValues = "";//when typing on keypad string stored here, eg for typing a password
int Menu = 0; //which menu item is active
String Val;
int Topping = 0;
String oldValues = "";
int budget_days = 3;

//=== END INIT OF VARS ===

      
/* *************************** /*
 *  SETUP() CODE
 * *************************** */

void setup() {
  setupSerials();
  setupPROM();
  setupLCD();
  runPossibleTests();  //TODO run if asked for by * press
  setupTimeEvents();
  postPROMinit();
}

void timeCheck() {
  if (timeStatus()!= timeNotSet) {
    digitalClockDisplay();  
    ee_epoch = now(); //timeNow;
    EEPROM.put(30, ee_epoch);
    drawMenu();//update menu
  }
}

void loop() {
  t.update();

  if (mySerial.available()) { //Time cmd
    processSyncMessage();
  }
  
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
  printVals();
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

void setupTimeEvents() {
  setSyncProvider( requestSync);
  //events running on a timer:
  int tickEvent = t.every(1000*60, AskMkrData);//every minute
  int tickEvent2 = t.every(2000,timeCheck);
}

void postPROMinit() {
  //sets time to last time stored in eeprom (counted or gps: when device was last on)
  //not accurate and needs gps time sync from mkr, but at least not too far off day-wise.
  //wont be much off as arduino should never be off, only if battery really at 0% as well as its own backup battery

  setTime(ee_epoch);
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
    
  }
  else if (Menu == 7) { //topping up
    lcd.setCursor(0, 0);
    lcd.print("Type in code");

    lcd.setCursor(0, 1);
    lcd.print(typingValues);
  }
  if (Menu == 8) { //topping up: step2 entered code
    lcd.setCursor(0, 0);
    lcd.print("processing");

    if (Val == "*" and typingValues != "*") { //* in there but not only *
      //Serial.print("code:");
      Serial.println(typingValues);

      if (typingValues == "12345*") {
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
      //typingValues = "";
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
    typingValues += customKey;
  }

  if (customKey == '*') { //top up
    if (Topping == 0) { //starting out
      Menu = 7;
      Topping = 1;
      typingValues = "";
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
    switch (str[0]) {
      case 'r'://rtc epoch
        Serial.print("Setting RTC as command from MKR: ");
        Serial.print(str);
        String new_string = "";
        for (int i = 1; i < 11; i++) {
          new_string += str[i];
        }
        //Serial.println(new_string);
        long ll = new_string.toInt();
        //Serial.println(ll);
        setRTC(ll);
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

time_t requestSync()
{
  Serial.println("uno: I want to sync time!");
}

void processSyncMessage() {
  /*unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

  if(mySerial.find(TIME_HEADER)) {
     pctime = mySerial.parseInt();
     Serial.print("processSyncMessage...");
     Serial.print(pctime);
     if( pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
       setTime(pctime); // Sync Arduino clock to the time received on the serial port
     }
  }*/
}

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
  Serial.print("setRTC...");
  Serial.println(epoch);
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013
  if ( epoch >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
    setTime(epoch); // Sync Arduino clock to the time received on the serial port
    Serial.print("saving to EEPROM...");
    EEPROM.put(30, epoch);
  }
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
  Serial.print("code ");
  Serial.print(code);
  Serial.print(": ");

  //concatenating sratchcode and addition
  String full_code = "";
  for (int i = 0; i < 12; i++) {
    full_code += code[i];
  }
  for (int i = 0; i < 4; i++) {
    full_code += secretadd[i];
  }
  full_code += '\n';
  //convert to char array "copy"
  char copy[17];
  full_code.toCharArray(copy, 17);

  char data[16];
  aes_context ctx;
  char auxBuffer[16 * 3]; //129

  memset(data, 0x00, 16);
  memcpy(data, copy, 16); //should give something like B9E1: always all caps!

  ctx = aes128_cbc_enc_start((const uint8_t*)key, key); //key and iv both same
  aes128_cbc_enc_continue(ctx, data, 16);
  aes128_cbc_enc_finish(ctx);

  memset(auxBuffer, 0x00, 16 * 3); //129);
  ByteToHexString(auxBuffer, (unsigned char *)data, sizeof(data));

  String short_hash = smart_shorten_simple(auxBuffer, 4);
  if (short_hash == checkcode) {
    Serial.println("Correct: lets top up");
  } else {
    Serial.println("Incorrect: not topping up");
  }
}

// === END CRYPTO CODE ===

// UTIL functions
void printIntArray(uint8_t * charval, int MAX) {
  for(int i = 0; i < MAX; i++)
  {
    Serial.print(charval[i]);
  }
  Serial.println("");
}

/* *************************** /*
 *  EEPROM MEMORY CODE
 * *************************** */
 
void clearPROM() {
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
}

void writePROM() {
  char ekey[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  EEPROM.put(0, ekey);
  
  char esecretadd[5] = "3454";
  EEPROM.put(16, esecretadd);
  
  char echeckcode[5] = "B9E1";
  EEPROM.put(20, echeckcode);
  
  EEPROM.put(30, 1562847038);
}

void readPROM() {
  EEPROM.get(0, key);
  EEPROM.get(16, secretadd);
  EEPROM.get(20, checkcode);
  EEPROM.get(30, ee_epoch);
}

void printVals() {
  Serial.println("key: ");
  printIntArray(key,16);
  
  Serial.println("secretadd: ");
  printIntArray(secretadd,4);

  Serial.println("checkcode: ");
  Serial.println(checkcode);

  Serial.println("ee_epoch: ");
  Serial.println(ee_epoch);
}

// === END EEPROM MEMORY ===
