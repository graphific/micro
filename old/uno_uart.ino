//ARDUINO
#include <SoftwareSerial.h>

SoftwareSerial mySerial(10,11); //RX,TX
//const int valPin = 5;
//const int valPin2 = 6;

//int incomingByte;   

//String inputString = ""; 
//bool stringComplete = false;

bool show = false;
String vals="";
String old_vals = "";

bool show2 = false;
String vals2="";
String old_vals2 = "";

bool startSig = false;
bool execute = false;

void setup() {
  //pinMode(valPin, INPUT);
  //pinMode(valPin, OUTPUT);
  
  Serial.begin(57600);
  while(!Serial){};
  Serial.println("hello i am arduino");

  //inputString.reserve(200);
  
  //softwareserial
  mySerial.begin(57600);
  while(!mySerial){};
  mySerial.println("hello mySerial");
}

int counter = 0;
 
void loop() {
  counter++;
  /*if (stringComplete) {
    Serial.print("stringComplete");
    Serial.println(inputString);
    // clear the string:
    inputString = "";
    stringComplete = false;
  }*/
  
  /*if(mySerial.available()){
    //incomingByte = mySerial.read();
    //int incomingByte = mySerial.parseInt();
    //char incomingByte = (char)mySerial.read();

    int incomingByte = mySerial.read();
    Serial.print("myserial ");
    Serial.print(incomingByte);
    Serial.println(" ");

    /*
    // get the new byte:
    char inChar = (char)mySerial.read();
    // add it to the inputString:
    inputString += inChar;
    if (inChar == '\n') {
      stringComplete = true;
    }
    Serial.print("myserial ");
    //Serial.print(incomingByte);
    Serial.print(inChar);
    Serial.println(" ");
    */
    
    //Serial.println(mySerial.read());
  //}

  while(mySerial.available()) { //cmd
    //int val = Serial.read();
    char val2 = (char)mySerial.read();

    if (val2 == '[') {
      startSig = true;
    } else if (val2 == ']') {
       show2=true;
       if(!execute) {
          Serial.print("myserial serial ");
          Serial.print(vals2);
          Serial.println(" ");
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
  
  
  
  //delay(500);
 if(counter>20) {
    
    /*Serial.print("old val ");
    Serial.print(old_vals);
    Serial.print("and ");
    Serial.println(old_vals2);
    Serial.println("--");*/

    Serial.println("sending TEST&"); //& is ending char
    mySerial.print("TEST&");
    counter=0;
    delay(200);
    //int test = analogRead(valPin);
    //Serial.println(test);//320-330
    //analogWrite(valPin2, HIGH);
  }
  delay(100);
}

