//seems to be working without F*ing up EEPROM
//output same as in python :+1

#include <AESLib.h>
//https://github.com/DavyLandman/AESLib/issues/17
const PROGMEM char HEX_VALUES[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

#define CBLOCK (1 * N_BLOCK) + 1

char serialBuffer[120];

void setup() {
  Serial.begin(57600);
}

void loop() {
  crypt();
  delay(10000);
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

String smart_shorten_simple(String res, int lenn) {
  String new_string = "";
  //for (int i = 6; i < res.length(); i++){
  for (int i = 6; i < 6+lenn; i++){
    char c = res.charAt(i);
    new_string += c;
  }
  //Serial.println(new_string);
  return new_string;
}

void crypt() {
  uint8_t key[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
  uint8_t iv[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
  char data[16];
  aes_context ctx;
  char auxBuffer[129];

  memset(data, 0x00, 16);
  //memcpy(data, "1234567891923454", 16);
  memcpy(data, "1242847264023454", 16); //should give 1234
  memcpy(data, "1144285975783454", 16); //should give B9E1 //always all caps!

  ctx = aes128_cbc_enc_start((const uint8_t*)key, iv);
  aes128_cbc_enc_continue(ctx, data, 16);
  aes128_cbc_enc_finish(ctx);

  memset(auxBuffer, 0x00, 129);
  ByteToHexString(auxBuffer, (unsigned char *)data, sizeof(data));
  sprintf(serialBuffer, "encrypted-cbc: %s", auxBuffer);
  Serial.println(serialBuffer);

  /*ctx = aes128_cbc_dec_start((const uint8_t*)key, iv);
  aes128_cbc_dec_continue(ctx, data, 16);
  aes128_cbc_dec_finish(ctx);

  sprintf(serialBuffer, "decrypted-cbc: %s", data);
  Serial.println(serialBuffer);

  memset(auxBuffer, 0x00, 129);
  ByteToHexString(auxBuffer, (unsigned char *)iv, sizeof(iv));
  sprintf(serialBuffer, "iv: %s", auxBuffer);
  Serial.println(serialBuffer);*/

  String short_hash = smart_shorten_simple(auxBuffer,4);
  Serial.println(short_hash);
}
