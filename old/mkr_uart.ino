//MKR GSM

//SoftwareSerial mySerial(13,14); //RX,TX

//const int valPin = A1;
//const int valPin2 = A3;

int counter = 0;
String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete


void setup() {
  //pinMode(valPin, OUTPUT);
  //pinMode(valPin2, INPUT);
  
  Serial.begin(57600);
  while(!Serial){};
  
  inputString.reserve(200);
  
  Serial.println("hello serial");

  Serial1.begin(57600);
  while(!Serial1){};
  Serial1.write("hello serial1");
  
  //softwareserial
  //mySerial.begin(4800);
  //mySerial.println("hello mySerial");
}

void serialEvent() {
  Serial.println("hello serialevent");
}
void serialEvent1() {
  Serial.println("hello serialevent1");
  
}

void loop() {
  counter++;

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
  
  //if(Serial1.available()){
  //  Serial.write(Serial1.read());
  //}
  
  //if(mySerial.available()){
  //  Serial.write(mySerial.read());
  //}
  //Serial1.print("hello mySerial from MKR gsm 1400");
  //Serial.write("hello mySerial from MKR gsm 1400");

  //Serial1.write(70);
  //Serial1.write(74);
  //Serial1.write("a"); //97 = indeed dec val
  //Serial1.write("abc"); //97 98 99 = indeed dec val, as sep events

  if(counter>10) {
    
    counter=0;
    /*
    Serial1.write("a"); //97 98 99 = indeed dec val, as sep events --> Doesnt always work, why??? TODO
    //Serial.println("hello");
    delay(500);
    Serial1.write("b"); //97 98 99 = indeed dec val, as sep events
    delay(500);
    Serial1.write("c"); //97 98 99 = indeed dec val, as sep events
    delay(500);
    //Serial1.write("def"); //multple goes wrong?
    //delay(500);
    Serial1.write("]"); //97 98 99 = indeed dec val, as sep events
    delay(500);*/
  
  
    Serial1.print("abs]"); //97 98 99 = indeed dec val, as sep events
    delay(100);
    //bat 12.1v,solar 20w always 5 chars 1.2a
    //b12.1s0020a01.2a
    //execute=[g for go [g1=...
    
    //values to log inside gsm to get avg when logging and sending to cloud
    //sending here to arduino to show it on lcd display and controls
    //1sided communication for now = cannot change settings?
    //need to change/enter phone number? = do at office or by mobile phone?
    //need to change/enter credit code that works offline = logic on arduino
    Serial1.print("[b12.1]");
    delay(100);
    Serial1.print("[s20]");
    delay(100);
    Serial1.print("[a1.2]");
    delay(150);
  
    //can send commands to execute when demanded from the cloud or mobile (through cloud)
    //Serial1.print("[gcredit]");//
    //delay(1500);
    Serial1.print("[gtopup3d]");//when done through mobile phone = also enable if disabled = show new credit on LCD & play tune??
    delay(150);
    Serial1.print("[gdisable]");//power off when out of credit = disable if not disabled yet
    delay(150);
    Serial1.print("[genable]");//power off when out of credit = disable if not disabled yet
    delay(150);
  }
  delay(150);

  //analogWrite(valPin, HIGH);
    //analogRead(valPin);
  //int test = analogRead(valPin2);
  //Serial.println(test);//181
  
}
