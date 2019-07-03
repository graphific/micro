//MKR GSM
//Contains dummy sensor variables set and changed as simulation and send to uno over uart
//will have sensor connections for temp, humidy and voltage levels

//TODO: merge with working hologram code

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

void setup() {
  Serial.begin(57600);
  while(!Serial){};
  
  inputString.reserve(200);
  
  Serial.println("hello serial");

  Serial1.begin(57600);
  while(!Serial1){};
  Serial1.write("hello serial1");
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
  
  
  if(counter>20) {//time to update stats
    counter=0;
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
  }
  delay(150);
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

  //bat_temp rand increase or decrease
  if(random(0, 10)>5) {
    bat_temp += 0.1;
  } else {
    bat_temp -= 0.1;
  }
  
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
  
  humidity = random(30, 70);
}
