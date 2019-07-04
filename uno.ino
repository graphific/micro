//ARDUINO
//working keypad, lcd, uart communication, hardcoded code top up, menu dynamic values, getting sensor values from mkr

//TODO: make refreshMenu int of which menu item to refresh instead of all = flicker per incoming new item
//or have a longer delay than all new incoming sensor items

//TODO: timeNotSet not changing
//TODO: only sync RTC every so many minutes...
//TODO: if no good time, all good, but cannot use offline credit codes
//TODO: test of gsm doesnt send data (=no correct menu values) = restart??

//TIME
#include <TimeLib.h>

//END TIME

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10,11); //RX,TX
bool show = false;
String vals="";
String old_vals = "";

bool show2 = false;
String vals2="";
String old_vals2 = "";

bool startSig = false;
bool execute = false;
int counter = 0;
String curcmd = "";
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

//LCD
//custom characters
/*
 Based on Adafruit's example at
 https://github.com/adafruit/SPI_VFD/blob/master/examples/createChar/createChar.pde

 This example code is in the public domain.
 http://www.arduino.cc/en/Tutorial/LiquidCrystalCustomCharacter

 Also useful:
 http://maxpromer.github.io/LCD-Character-Creator/
*/ 
// make some custom characters:
byte b_battery[8] = {
  B00000,
  B00110,
  B01111,
  B01001,
  B01001,
  B01001,
  B01001,
  B01111
};
byte b_solar[8] = {
  B11111,
  B10101,
  B11111,
  B10101,
  B10101,
  B11111,
  B10101,
  B11111
};
byte heart_h[8] = {
  B00000,
  B00000,
  B01010,
  B10101,
  B10001,
  B01010,
  B00100,  
  B00000
}; 
byte heart_f[8] = {
  B00000,
  B00000,
  B01010,
  B11111,
  B11111,
  B01110,
  B00100,  
  B00000
}; 
byte heart[8] = {
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};

byte smiley[8] = {
  0b00000,
  0b00000,
  0b01010,
  0b00000,
  0b00000,
  0b10001,
  0b01110,
  0b00000
};

byte frownie[8] = {
  0b00000,
  0b00000,
  0b01010,
  0b00000,
  0b00000,
  0b00000,
  0b01110,
  0b10001
};

byte armsDown[8] = {
  0b00100,
  0b01010,
  0b00100,
  0b00100,
  0b01110,
  0b10101,
  0b00100,
  0b01010
};

byte armsUp[8] = {
  0b00100,
  0b01010,
  0b00100,
  0b10101,
  0b01110,
  0b00100,
  0b00100,
  0b01010
};

// degree centigrade
byte b_c[8] = {
  B01000,
  B10100,
  B01000,
  B00011,
  B00100,
  B00100,
  B00011,
  B00000
};

// degree fahrenheit
byte b_f[8] = {
  B01000,
  B10100,
  B01000,
  B00011,
  B00100,
  B00111,
  B00100,
  B00000
};

// arrow right
byte arrow_r[8] = {
  B00000,
  B00100,
  B00010,
  B11111,
  B00010,
  B00100,
  B00000,
  B00000
};

// arrow left
byte arrow_l[8] = {
  B00000,
  B00100,
  B01000,
  B11111,
  B01000,
  B00100,
  B00000,
  B00000
};
byte arrow_u[8] = {
  B00100,
  B01110,
  B11111,
  B00100,
  B00100,
  B00100,
  B00100,  
  B00100
}; 
byte arrow_d[8] = {  
  B00100,
  B00100,
  B00100,
  B00100,  
  B00100,  
  B11111,
  B01110,
  B00100
}; 
byte b_lock[8] = {  
  0b01110,
  0b10001,
  0b10001,
  0b11111,
  0b11011,
  0b11011,
  0b11111,
  0b00000
};
byte b_unlock[8] = {  
  0b01110,
  0b10000,
  0b10000,
  0b11111,
  0b11011,
  0b11011,
  0b11111,
  0b00000
};


bool armsUpb = false;

//Include LCD library
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(A0,A1,A2,A3,A4,A5);

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
  Serial.begin(57600);
  while(!Serial){};
  Serial.println("hello i am arduino");
  //Serial1.begin(9600);

  //softwareserial
  mySerial.begin(57600);
  while(!mySerial){};
  mySerial.println("hello mySerial");
  
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  
  // Print a message to the LCD.
  lcd.print("Booting up...");

  setSyncProvider( requestSync);  //set function to call when sync required
  
  delay(1000);
  lcd.clear(); 
}

time_t requestSync()
{
  Serial.println("uno: I want to sync time!");
}


void loop() {
  counter++;
  
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
  
  //TODO: SEPERATE MENU SHOW FUNCTION = CAN REFRESH MENU WITHOUT KEY PRESS
  //MHH = SOMEHOW ALREADY UPDATES SCREEN, SO BELOW ALWAYS RUNS = BAD?
  if (customKey != NO_KEY){
    //Serial.println(customKey);
    Val=customKey;
    
    //lcd.setCursor(9, 1);
    //lcd.print("KEY= ");  
    //lcd.print(customKey);
    if(Val=="#") {
      Serial.println("pressed #");
      if(Menu > 5) {
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
        Menu = 7;
        Topping = 1;
        Values = "";
        lcd.clear();    
      } else if(Topping == 1) {//entered code lets check
        Menu = 8;
        Topping = 0;//back to normal after
        lcd.clear();   
      }
    }

    drawMenu();
    refreshMenu = false;
  }

  if(refreshMenu) {
    drawMenu();
    refreshMenu = false;
  }

  

  while(mySerial.available()) { //cmd
    //int val = Serial.read();
    char val2 = (char)mySerial.read();

    if (val2 == '[') {
      startSig = true;
    } else if (val2 == ']') {
       show2=true;
       if(!execute) {
          //check cmd
          //TODO: clear lcd if value changed but not here = wait long
          //=rerun drawMenu function()
          refreshMenu=true;
          Serial.print("myserial serial ");
          Serial.print(curcmd);
          Serial.print(" : ");
          Serial.print(vals2);
          Serial.println(" ");

          if(curcmd == "solar_w") {
            solar_w = vals2.toFloat();
          } else if(curcmd == "solar_a") {
            solar_a = vals2.toFloat();
          } else if(curcmd == "bat_v") {
            bat_v = vals2.toFloat();
            //TODO
            bat_p = 80; //battery capacity on percentages (calculated from bat_v)
            bat_wh = 800; //battery wh left, calculated from bat_p*bat_ah*12 (bat voltage)
            if(load_w>0) {
              bat_time = bat_wh/load_w; //battery time left in h at current load, calculated from bat_wh and load_w
            } else {
              //TODO: take avg of period (10w?)
              bat_time = bat_wh/10;
            }
          } else if(curcmd == "bat_temp") {
            bat_temp = vals2.toFloat();
          } else if(curcmd == "load_lv_w") {
            load_lv_w = vals2.toFloat();
            load_w = load_lv_w+load_hv_w;
            if(load_lv_w > 5) {
              load_lv_b = true;
            } else {
              load_lv_b = false;
            }
          } else if(curcmd == "load_hv_w") {
            load_hv_w = vals2.toFloat();
            load_w = load_lv_w+load_hv_w;
            if(load_hv_w > 5) {
              load_hv_b = true;
            } else {
              load_hv_b = false;
            }
          } else if(curcmd == "humidity") {
            humidity = vals2.toFloat();
          } else if(curcmd == "set_rtc") {
            setRTC(vals2.toFloat());

            /* 
             *  keep code for if we want to send multipel params a;b;c;d
             */
            //Hour, Minute, Second, Day, Month, Year
            /*
            int str_len = vals2.length() + 1; 
            char char_array[str_len];
            vals2.toCharArray(char_array, str_len);
            //(char*) yourString.c_str();

            char buf[sizeof(char_array)];
            vals2.toCharArray(buf, sizeof(buf));
            char *p = buf;
            char *str;
            while ((str = strtok_r(p, ";", &p)) != NULL) // delimiter is the semicolon
              Serial.println(str);*/
          }
          
          curcmd = "";
          //mySerial.print(vals2);
          //store to sample avg of values when we want to log it.
        } else {
          //execute
          Serial.print("do something:");
          Serial.print(vals2);
          Serial.println(" ");
          execute = false;
        }
        show2 = false;
        old_vals2 = vals2;
        vals2="";
    } else {
      if(startSig and val2 == 'g') {
        //need to do something
        Serial.println("do something...");
        execute = true;
        curcmd = ""; 
      } else if(startSig and val2 == 's') {
        curcmd = "solar_w"; 
      } else if(startSig and val2 == 'a') {
        curcmd = "solar_a"; 
      } else if(startSig and val2 == 'b') {
        curcmd = "bat_v"; 
      } else if(startSig and val2 == 't') {
        curcmd = "bat_temp"; 
      } else if(startSig and val2 == 'l') {
        curcmd = "load_lv_w"; 
      } else if(startSig and val2 == 'h') {
        curcmd = "load_hv_w"; 
      } else if(startSig and val2 == 'm') {
        curcmd = "humidity"; 
      } else if(startSig and val2 == 'r') {
        curcmd = "set_rtc"; 
      } else {
        vals2 += val2;
      }
    }
   
  }
  
  while(Serial.available()) { //cmd
    //int val = Serial.read();
    char val = (char)Serial.read();
    if (val == ']') {
       show=true;

       if(show) {
          Serial.print("normal serial ");
          Serial.print(vals);
          Serial.println(" ");
          //mySerial.print(vals);
          show = false;
          old_vals = vals;
          vals="";
        }
        
    } else {
      vals += val;
    }
   
  }

  if(counter%100 == 0) {
    if(Menu == 6) {
      armsUpb = !armsUpb;
      drawMenu();
    }
    
  } else if(counter>1000) {
    Serial.println("internal time set?");
    Serial.println(timeNotSet);
    if(!timeNotSet) {
      digitalClockDisplay();
    }
    /*Serial.print("old val ");
    Serial.print(old_vals);
    Serial.print("and ");
    Serial.println(old_vals2);
    Serial.println("--");*/
    
    //TODO SEND: NEED TIME IF NEEDED
    Serial.println("sending TEST&"); //& is ending char
    mySerial.print("TEST&");
    counter=0;
    //delay(200);
    //int test = analogRead(valPin);
    //Serial.println(test);//320-330
    //analogWrite(valPin2, HIGH);
  }
  delay(20);
  
}//end loop

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
  if( epoch >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
    setTime(epoch); // Sync Arduino clock to the time received on the serial port
  }
  Serial.print("Unix time = ");
  digitalClockDisplay();
}

void drawMenu() {

  lcd.clear();  
  if(Menu==0) {
    lcd.setCursor(0, 0);
    lcd.print("* to top up");
    
    lcd.setCursor(0, 1);
    lcd.print("Credits: ");
    lcd.print(budget_days);
    lcd.print(" days"); 
  }
  else if(Menu==1) {
    lcd.setCursor(0, 0);
    //lcd.print("2.Stat"); 
    if(solar_w>0) {
      lcd.print("Solar: y"); 
      lcd.setCursor(0, 1);
      lcd.print("Gen: "+String(solar_w,2)+"w");
    } else {
      lcd.print("Solar: n"); 
      lcd.setCursor(0, 1);
      lcd.print("Gen: 0w");
    }
  }
  else if(Menu==2) {
    lcd.setCursor(0, 0);
    //lcd.print("3.Settings"); 
    if(load_w>0) {
      lcd.print("Usage: y"); 
      lcd.setCursor(0, 1);
      lcd.print("Load:"+String(load_w,1)+"w");
    } else {
      lcd.print("Usage: n"); 
      lcd.setCursor(0, 1);
      lcd.print("Load: 0w"); 
    }
    lcd.setCursor(10, 0);
    if(load_lv_w>0) {
      lcd.print("|lv: y"); 
    } else {
      lcd.print("|lv: n");
    }
    lcd.setCursor(10, 1);
    if(load_hv_w>0) {
      lcd.print("|hv: y"); 
    } else {
       lcd.print("|hv: n"); 
    }
  }
  else if(Menu==3) {
    lcd.setCursor(0, 0);
    lcd.print("Bat: "+String(bat_p,1)+"% "+String(bat_wh,0)+"wh"); 
    lcd.setCursor(0, 1);
    lcd.print("Time Left: "+String(bat_time,1)+"h"); 
  }
  else if(Menu==4) {
    lcd.setCursor(0, 0);
    lcd.print("Humid: "+String(humidity,2)+"%"); 
    lcd.setCursor(0, 1);
    lcd.print("Temp:  "+String(bat_temp,2)+"C"); 
  }
  else if(Menu==5) {
    String hh = String(hour());
    if(hh.length() < 2)
      hh = '0'+hh;
    String mm = String(minute());
    if(mm.length() < 2)
      mm = '0'+mm;
    String ss = String(second());
    if(ss.length() < 2)
      ss = '0'+ss;
    String dd = String(day());
    if(dd.length() < 2)
      dd = '0'+dd;
    String mn = String(month());
    if(mn.length() < 2)
      mn = '0'+mn;
    String yy = String(year());
   
    lcd.setCursor(0, 0);
    lcd.print(dd+"-"+mn+"-"+yy); 
    //lcd.setCursor(0, 1);
    //lcd.print(hh+":"+mm+":"+ss);
    lcd.setCursor(11, 0);
    lcd.print(hh+":"+mm); 
  }
  else if(Menu==6) {
    /*lcd.setCursor(0, 0);
    lcd.print("Stats"); //Values); 
    
    lcd.setCursor(0, 1);
    lcd.print("Uptime: "); 
    lcd.print(millis() / 1000);

    //lcd.setCursor(8, 0);
    //lcd.print("| test"); 

    //lcd.setCursor(8, 1);
    //lcd.print("| test2"); */
    
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
    }
  }
  else if(Menu==7) { //topping up
    lcd.setCursor(0, 0);
    lcd.print("Type in code");
      
    lcd.setCursor(0, 1);
    lcd.print(Values); 
  }
  if(Menu==8) { //topping up: step2 entered code
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
        Menu=7;
      }
      Values="";
    }
  }//end menu cases
}
