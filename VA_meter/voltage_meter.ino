#include <Wire.h>
int val11;
float val2;

int analogInput = A0;

//2th method
float vout = 0.0;
float vin = 0.0;
float R1 = 30000.0; //  says 200 on the board
float R2 = 7500.0; // 
int value = 0;

void setup()
{
  Serial.begin(9600);
  pinMode(analogInput, INPUT);
  Serial.println("Voltage: ");
  Serial.print("V");
}
void loop()
{
  float temp;
  val11=analogRead(A0);
  temp=val11/4.092; //needs an aref
  val2=(temp/10);
  Serial.println(val2);

  //method2
  value = analogRead(analogInput);
   vout = (value * 5.0) / 1024.0; // see text
   vin = vout / (R2/(R1+R2)); 
   
  Serial.print("INPUT V= ");
  Serial.println(vin,2);
       
   delay(1000);
}
