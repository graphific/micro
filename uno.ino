//ARDUINO
//working keypad, lcd, uart communication, hardcoded code top up, menu dynamic values, getting sensor values from mkr

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
  delay(1000);
  lcd.clear(); 
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

  if(counter>100) {
    
    /*Serial.print("old val ");
    Serial.print(old_vals);
    Serial.print("and ");
    Serial.println(old_vals2);
    Serial.println("--");*/

    Serial.println("sending TEST&"); //& is ending char
    mySerial.print("TEST&");
    counter=0;
    //delay(200);
    //int test = analogRead(valPin);
    //Serial.println(test);//320-330
    //analogWrite(valPin2, HIGH);
  }
  delay(200);
  
}//end loop

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
    lcd.setCursor(0, 0);
    lcd.print("Stats"); //Values); 
    
    lcd.setCursor(0, 1);
    lcd.print("Uptime: "); 
    lcd.print(millis() / 1000);

    //lcd.setCursor(8, 0);
    //lcd.print("| test"); 

    //lcd.setCursor(8, 1);
    //lcd.print("| test2"); 
  }
  else if(Menu==6) { //topping up
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
  }//end menu cases
}
