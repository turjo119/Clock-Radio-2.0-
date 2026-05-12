# Clock Radio 

### Overview
- The purpose of the markdown file is to list some of the details in the clock radio project
- The goal is to build a clock radio with a Dot Matrix display. The display shows the current time and we are able to change the current FM frequency to tune into a new station.
- The core components of the project is built using an Arduino Nano, a TEA5767 FM radio module, a DS3231 RTC module, a MAX7219 LED dot matrix display, and a rotary encoder.

### Libraries
- A list of software libraries used can be found here
#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <radio.h>
#include <TEA5767.h>
#include <ezButton.h>  // the library to use for SW pin
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

### Bugs to be addressed
Fix the following bugs one by one. Show how you plan to resolve the current issue, explain issues in the code, suggest changes, after changes have been tried and tested move to the next bug

### Bug 1(FIXED): FM frequency Display Bug 
- One small issue that needs to be addressed is that when the display shows the frequency of the current station, for example, it shows 94.20 as "94.19 FM" or 95.50 as "95.49 FM"
- The dot matix display should show the exact frequency. For example, 94.20 should be shown as it is
- The letters "FM" do not need to shown in the dot-matrix display.

### Bug 2: Displayed Time incorrect
- Currently the time displayed on the Dot-matrix is not the same as the actual time according to the system clock shown on the current windows computer (synced to time in Berlin, Germany)
- Read through the code, in particular the implementation of functions from the RTClib library, as the issues might be a result using a function without wholly understanding the implications
- Ideally the RTC should start with the time at which it was compiled. When there is a power-loss,it should remember the time elapsed and then when it regains power, show the correct time 

