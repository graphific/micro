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

void setup() {
  Serial.begin(9600);
  crypto_feed_watchdog();      
  
  char* input = "178648632986ag7hCj97jhbgh%4#Dbk1";
  testIt(&aes128, input);
}

void loop() {
}

void testIt(BlockCipher *cipher, char* input) {
  unsigned char output[16];
  byte text[12];
  
  crypto_feed_watchdog();
  Serial.print(" Encryption ... ");
  cipher->setKey((const unsigned char*) key, cipher->keySize()); //strlen(key2) * 8);
  cipher->encryptBlock(buffer, (const unsigned char*)input); 
  
  Serial.println("");
  for (int i = 0; i < 16; i++) {
    char str[3];
    sprintf(str, "%02x", (int)buffer[i]);
    Serial.print(str);
  }
  Serial.println("");
}

