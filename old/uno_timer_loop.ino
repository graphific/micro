//RTC timer
//events

/* Shows time actually moving
 *  Still need to set the actual time itself
 *  23:21:34.961 -> 0:00:03 1 1 1970
    23:21:35.960 -> 0:00:04 1 1 1970
    23:21:36.958 -> 0:00:05 1 1 1970
    23:21:37.958 -> 0:00:06 1 1 1970
    23:21:38.951 -> 0:00:07 1 1 1970
 */

#include <TimeLib.h>
#include <Wire.h>
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
#include "Timer.h"

Timer t;

void setup()  {
  Serial.begin(9600);
  setSyncProvider(RTC.get);   // the function to sync the time from the RTC
  //int tickEvent = t.every(2000, timeShow);
  timeShow();
  int tickEvent = t.every(1000*60, timeShow);//every minute
}

void loop()
{
   t.update();
}

void timeShow() {
  time_t timeNow = now();
  showTime(timeNow);
}

void showTime(time_t t)
{
  // display the given time
  Serial.print(hour(t));
  printDigits(minute(t));
  printDigits(second(t));
  Serial.print(" ");
  Serial.print(day(t));
  Serial.print(" ");
  Serial.print(month(t));
  Serial.print(" ");
  Serial.print(year(t));
  Serial.println("");
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

