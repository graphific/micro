/*
** Example Arduino sketch for SainSmart I2C LCD2004 adapter for HD44780 LCD screens
** Readily found on eBay or http://www.sainsmart.com/ 
** The LCD2004 module appears to be identical to one marketed by YwRobot
**
** Edward Comer
** LICENSE: GNU General Public License, version 3 (GPL-3.0)
**
** sain_lcd_2.ino
** Simplified and modified by Andrew Scott for Arudino 1.0.1, Arudino Uno R3.
** NOTE: I2C pins are A4 SDA, A5 SCL
** Don't forget to use the new LiquidCrystal Library.
*/



#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

#define I2C_ADDR    0x27  // Define I2C Address where the SainSmart LCD is
//0x27 . or 0x3F
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

LiquidCrystal_I2C  lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

// Creat a set of new characters
const uint8_t charBitmap[][8] = {
   { 0xc, 0x12, 0x12, 0xc, 0, 0, 0, 0 },
   { 0x6, 0x9, 0x9, 0x6, 0, 0, 0, 0 },
   { 0x0, 0x6, 0x9, 0x9, 0x6, 0, 0, 0x0 },
   { 0x0, 0xc, 0x12, 0x12, 0xc, 0, 0, 0x0 },
   { 0x0, 0x0, 0xc, 0x12, 0x12, 0xc, 0, 0x0 },
   { 0x0, 0x0, 0x6, 0x9, 0x9, 0x6, 0, 0x0 },
   { 0x0, 0x0, 0x0, 0x6, 0x9, 0x9, 0x6, 0x0 },
   { 0x0, 0x0, 0x0, 0xc, 0x12, 0x12, 0xc, 0x0 }
   
};

void setup()
{
   int charBitmapSize = (sizeof(charBitmap ) / sizeof (charBitmap[0]));
   
 //lcd.begin (16,2);
 lcd.begin (20,4);
 
 // Switch on the backlight
 lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
 lcd.setBacklight(HIGH);

 //alt method doeesnt work
 // Switch on the backlight
 //pinMode ( BACKLIGHT_PIN, OUTPUT );
 //digitalWrite ( BACKLIGHT_PIN, HIGH );
  

  for ( int i = 0; i < charBitmapSize; i++ )
   {
      lcd.createChar ( i, (uint8_t *)charBitmap[i] );
   }
   
 // Position cursor and write some text
 lcd.home ();                   // go to the first line, first character
 lcd.print("Sunshine Lab");  
 lcd.setCursor ( 0, 1 );        // go to the 2nd line
 lcd.print("presents...");
delay ( 1000 );
}

void loop()
{
  lcd.home ();
  lcd.setCursor ( 0, 2 );
   // Do a little animation by writing to the same location
   for ( int i = 2; i < 4; i++ )
   {
      for ( int j = 0; j < 20; j++ )
      {
         lcd.print (char(random(7)));
      }
      lcd.setCursor ( 0, 3 );
   }
   delay (200);
}
