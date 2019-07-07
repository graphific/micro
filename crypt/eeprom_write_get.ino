#include <EEPROM.h>

struct Secret {
  //float field1;
  //byte field2;
  char key[16];
  char secretadd[20];
  char checkcode[4];
};

//char* key = "abcdefghijklmnop"; //store in EEPROM
//char* secretadd = "ag7hCj97jhbgh%4#Dbk1"; //store in EEPROM
//String checkcode = "5adb"; //store in EEPROM

bool writeValue = false; //set to true 1x, then rewrite with false and reboot device to see if EEPROM sustains

void setup(){

  Serial.begin( 9600 );
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  if(writeValue) {
    Serial.print( "Writing to EEPROM..." );
  
    Secret secret = {
      "abcdefghijklmnop",
      "ag7hCj97jhbgh%4#Dbk1",
      "5adb"
    };
  
    //eeAddress += sizeof(float);
    int eeAddress = 0; //EEPROM address to start writing to
    EEPROM.put(eeAddress, secret);
  }
}

bool runOnce = true;

void loop(){ 
 
  if(runOnce) {
    Serial.println( "Read from EEPROM: " );
    //eeAddress = sizeof(float);
    int eeAddress = 0;
    
    //Get the float data from the EEPROM at position 'eeAddress'
    //float f = 0.00f;
    //EEPROM.get( eeAddress, f );
    //Serial.println( f, 3 );  //This may print 'ovf, nan' if the data inside the EEPROM is not a valid float.
  
    // get() can be used with custom structures too. 
    Secret my_new_secret; //Variable to store custom object read from EEPROM.
    EEPROM.get( eeAddress, my_new_secret );
  
    Serial.println( "Read custom object from EEPROM: " );
    Serial.println( my_new_secret.key );
    Serial.println( my_new_secret.secretadd );
    Serial.println( my_new_secret.checkcode );
    runOnce = false;
  }
  delay(2000);
}
