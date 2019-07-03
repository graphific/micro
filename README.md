# micro
microcontroller software

## Setup:

### UART
Arduino Uno <> MKR GSM 1400

GND <> GND

RX 10 14 TX

TX 11 13 RX

### Menu
code

### Keypad
Arduino Uno port 2-7

### LCD
default but A0-A5 (inc potentiometer)
https://www.instructables.com/id/Connecting-an-LCD-to-the-Arduino/

lcd todo:
only 3 pins
https://www.instructables.com/id/Hookup-a-16-pin-HD44780-LCD-to-an-Arduino-in-6-sec/

5 pins
https://www.instructables.com/id/How-to-use-an-LCD-displays-Arduino-Tutorial/

### OTA
Over-the-Air (OTA) programming for remote update

Adafruit Micro SD breakout

store code on SD and bootload from it

a = download the hex file, and reprogram the Main arduino

b = saving an UPDATE.bin file to the SD card and restarting the board.

todo: check if file correctly downloaded first (hash?)

### Sensors
Measure Voltage

Measure Temperature

### Case
3d print

### Single PCB
DIY to design and manufacture in lab

