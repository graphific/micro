#include <Crypto.h>
#include <AES.h>
#include <string.h>

/*
 * 19:28:34.317 ->  Encryption ... 
 * 19:28:34.317 -> 37c1de2c855812392c3e960bbe7a7e74
 * = the same as python program so works on arduino and python pc
 */
AES128 aes128;
byte buffer[16];
char* key = "abcdefghijklmnop";
char* secretadd = "ag7hCj97jhbgh%4#Dbk1";

void setup() {
  Serial.begin(9600);
  crypto_feed_watchdog();      
  
  char* scratch_code = "178648632986"; 
  char input[strlen(scratch_code)+strlen(secretadd)];

  //char* input = scratch_code+secretadd; //first 12 chars is generated srcatch code other half secret
  sprintf(input,"%s%s",scratch_code,secretadd);
  
  String res, res2;
  res = testIt(&aes128, input); //37c1de2c855812392c3e960bbe7a7e74
  res2 = smart_shorten(res);
  Serial.print(res2);
}

void loop() {
}

String smart_shorten_simple(String res, int lenn=6) {
 
  String new_string = "";
  //for (int i = 6; i < res.length(); i++){
  for (int i = 6; i < 6+lenn; i++){
    char c = res.charAt(i);
    new_string += c;
  }
  //Serial.println(new_string);
  return new_string;
}

String smart_shorten(String res) {
  char prev_char = "-1";
  char prev_int = "-1";
  bool prev_is_digit = true;

  String new_string = "";
  
  for (int i = 0; i < res.length(); i++){
    char c = res.charAt(i);        
    if(isDigit(c)) {
      if(!prev_is_digit) {
        new_string += c;
      }
      prev_is_digit = true;
    } else {//string
      if(prev_is_digit) {
        new_string += c;
      }
      prev_is_digit = false;
    }
  }
  //Serial.println(new_string);
  String new_string2 = "";
  for (int i = 0; i < new_string.length(); i++){
    char c = new_string.charAt(i);
    if(new_string2.indexOf(String(c)) == -1) {//not there yet
       new_string2 += c;
    }
  }
  //Serial.println(new_string2);
  return new_string2;
}


String testIt(BlockCipher *cipher, char* input) {
  unsigned char output[16];
  byte text[12];
  
  crypto_feed_watchdog();
  Serial.print(" Encryption ... ");
  cipher->setKey((const unsigned char*) key, cipher->keySize()); //strlen(key2) * 8);
  cipher->encryptBlock(buffer, (const unsigned char*)input); 

  String res = ""; //[3*16];
  Serial.println("");
  //int ii = 0;
  for (int i = 0; i < 16; i++) {
    char str[3];
    sprintf(str, "%02x", (int)buffer[i]);
    Serial.print(str);
    res+=str;
    //ii++;
  }
  Serial.println("");
  Serial.println(res);
  return res;
  
}
