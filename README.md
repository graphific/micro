# micro
microcontroller software

- Uart
- Menu
- Keypad
- LCD
- OTA
- GSM
- Sensors
- Case
- PCB

## Setup:

### UART
Arduino Uno <> MKR GSM 1400

GND <> GND

RX 10 14 TX

TX 11 13 RX

### Menu
code

already allows for topping up (* > code 12345 > *)

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

### GSM

MKR 1400 GSM communicates using hologram IOT sim card and sends data to hologram cloud using the hologram API

### Sensors

#### Measure Voltage

TODO

#### Measure Temperature

DHT11 uses pin 6

DHT22 uses pin 7

### Case
3d print

### Single PCB
DIY to design and manufacture in lab

