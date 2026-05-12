/*
An Arduino Sketch for an FM Radio clock with TEA5767. A rotary encorder is used 
to change  the frequency of the radio. The push button on the rotary encoder is
used to change the station. The Dot Matrix Display shows the current time and 
frequency.
*/

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <radio.h>
#include <TEA5767.h>
#include <ezButton.h>  // the library to use for SW pin
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// Compile-time date/time → integer constants from __DATE__ ("Mmm dd yyyy") and __TIME__ ("hh:mm:ss")
#define COMPILE_YEAR  ((__DATE__[7]-'0')*1000 + (__DATE__[8]-'0')*100 + (__DATE__[9]-'0')*10 + (__DATE__[10]-'0'))
#define COMPILE_DAY   ((__DATE__[4]==' ' ? 0 : (__DATE__[4]-'0'))*10 + (__DATE__[5]-'0'))
#define COMPILE_MONTH ( \
  __DATE__[2]=='n' ? (__DATE__[1]=='a' ? 1 : 6) : \
  __DATE__[2]=='b' ? 2 : \
  __DATE__[2]=='r' ? (__DATE__[0]=='M' ? 3 : 4) : \
  __DATE__[2]=='y' ? 5 : \
  __DATE__[2]=='l' ? 7 : \
  __DATE__[2]=='g' ? 8 : \
  __DATE__[2]=='p' ? 9 : \
  __DATE__[2]=='t' ? 10 : \
  __DATE__[2]=='v' ? 11 : 12)
#define COMPILE_HOUR  ((__TIME__[0]-'0')*10 + (__TIME__[1]-'0'))
#define COMPILE_MIN   ((__TIME__[3]-'0')*10 + (__TIME__[4]-'0'))
#define COMPILE_SEC   ((__TIME__[6]-'0')*10 + (__TIME__[7]-'0'))

//Set RTC to the hard-coded time on upload, 0 = Leave RTC alone
#define FORCE_RTC_SET 0

// Define the pins for the rotary encoder
#define CLK_PIN 2
#define DT_PIN 3
#define SW_PIN 4


// Stuff for Dot Matrix Display
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
// Defining size, and output pins
#define MAX_DEVICES 4
#define CS_PIN 5
MD_Parola ledMatrix = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);



volatile unsigned long last_time;  // for debouncing

// range of FM frequencies
const int fm_freq_default = 8800; // default FM frequency to tune into

volatile int fm_freq_current = 8800;
volatile int fm_freq_min = 8750;
volatile int fm_freq_max = 10800;
const int delta_freq = 10;
int fm_freq_previous;

char s[12]; // Character array to store the frequency

// Clock update timing
unsigned long lastClockUpdate = 0;
const unsigned long CLOCK_UPDATE_INTERVAL = 1000; // ms

// Display state: false = show time, true = show radio frequency
volatile bool showingRadioInfo = false;
volatile unsigned long lastEncoderActivityMs = 0;

// Radio frequency presets
RADIO_FREQ preset [] = {
  8800,
  8970,
  9010,
  9070,
  9130,
  9290,
  9420,
  9450,
  9550,
  9660,
  9820,
  9880,
  10170,
  10370,
  10750
  };

uint16_t presetIndex = 0;  ///< Start at Station with index = 1

ezButton button(SW_PIN);  // create ezButton object that attach to pin 4

// RTC
RTC_DS3231 rtc;

// Radio audio
uint8_t radioVolumeActive = 2; // default volume

// Function prototypes
void ISR_encoderChange();
void showRadioFrequency();
void showClockTime();

/// The band that will be tuned by this sketch is FM.
#define FIX_BAND RADIO_BAND_FM

/// The default station that will be tuned  by this sketch is 88.00 MHz.
#define FIX_STATION 8800

TEA5767 radio;    // Create an instance of Class for Si4703 Chip

// Setup a FM only radio configuration
// with some debugging on the Serial port
void setup() {
  // open the Serial port
  Serial.begin(57600);

  // Initialize I2C (for TEA5767 and DS3231)
  Wire.begin();

  // configure encoder pins as inputs
  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);
  button.setDebounceTime(50);  // set debounce time to 50 milliseconds

  // use interrupt for CLK pin is enough
  // call ISR_encoderChange() when CLK pin changes from LOW to HIGH
  attachInterrupt(digitalPinToInterrupt(CLK_PIN), ISR_encoderChange, RISING);

  Serial.println("Radio...");
  delay(200);

  // Initialize the Radio 
  radio.init();

  // Enable information to the Serial port
  //radio.debugEnable();

  // HERE: adjust the frequency to a local sender
  radio.setBandFrequency(FIX_BAND, FIX_STATION); 
  radio.setVolume(radioVolumeActive);
  radio.setMono(false);


  // Initialize the Dot Matrix Display
  ledMatrix.begin();         // initialize the object 
  ledMatrix.setIntensity(0); // set the brightness of the LED matrix display (from 0 to 15)
  ledMatrix.displayClear();  // clear led matrix display

  // Initialize the RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
  } else {
    if (rtc.lostPower()) {
      Serial.println("RTC lost power, setting the time!");
      rtc.adjust(DateTime(COMPILE_YEAR, COMPILE_MONTH, COMPILE_DAY,
                          COMPILE_HOUR, COMPILE_MIN, COMPILE_SEC));
    }
  #if FORCE_RTC_SET
    rtc.adjust(DateTime(COMPILE_YEAR, COMPILE_MONTH, COMPILE_DAY,
                        COMPILE_HOUR, COMPILE_MIN, COMPILE_SEC));
  #endif
  }

  // Start by showing the current time on the display
  showClockTime();

} // End of setup


/// show the current chip data every 3 seconds.
void loop() {

  //Stuff for the encoder
  button.loop();  // MUST call the loop() function first

  // Handle station-change button (rotary encoder push)
  if (button.isPressed()) {
    Serial.println("Changing the station...");
    if (presetIndex < (sizeof(preset) / sizeof(RADIO_FREQ)) - 1) {
      presetIndex++;
      radio.setFrequency(preset[presetIndex]);
    }
    // Go back to start
    else {
      presetIndex = 0;
      radio.setFrequency(preset[presetIndex]);
    }
    //Set as current frequency
    fm_freq_current = preset[presetIndex];

    // Show radio info on display and mark activity
    showingRadioInfo = true;
    lastEncoderActivityMs = millis();
  }

  // If frequency changed (due to encoder rotation), update radio display
  if (fm_freq_previous != fm_freq_current) {
    showRadioFrequency();
  }

  unsigned long nowMs = millis();

  // Auto-revert from radio display back to clock after 3 seconds of inactivity
  if (showingRadioInfo && (nowMs - lastEncoderActivityMs >= 3000)) {
    showingRadioInfo = false;
    lastClockUpdate = 0;  // force immediate refresh on next clock update
    showClockTime();
  }

  // When not showing radio info, periodically refresh time display
  if (!showingRadioInfo && (nowMs - lastClockUpdate >= CLOCK_UPDATE_INTERVAL)) {
    lastClockUpdate = nowMs;
    showClockTime();
  }
  
} // End of loop

//Function to display radio frequency
void showRadioFrequency() {
  fm_freq_previous = fm_freq_current;

  Serial.print("FM frequency: ");
  Serial.println(fm_freq_current);

  // Tune radio and display formatted frequency
  radio.setFrequency(fm_freq_current);
  sprintf(s, "%d.%02d", fm_freq_current / 100, fm_freq_current % 100);
  Serial.print("Station:");
  Serial.println(s);

  Serial.print("Radio:");
  radio.debugRadioInfo();

  Serial.print("Audio:");
  radio.debugAudioInfo();

  // Display the frequency on the Dot Matrix Display
  ledMatrix.setTextAlignment(PA_CENTER);
  ledMatrix.print(s); // display text
}

//Display current time in 24H format
void showClockTime() {
  DateTime now = rtc.now();

  char timeStr[6]; // "HH:MM" + '\0'
  uint8_t hour = now.hour();
  uint8_t minute = now.minute();

  timeStr[0] = '0' + (hour / 10);
  timeStr[1] = '0' + (hour % 10);
  timeStr[2] = ':';
  timeStr[3] = '0' + (minute / 10);
  timeStr[4] = '0' + (minute % 10);
  timeStr[5] = '\0';

  ledMatrix.setTextAlignment(PA_CENTER);
  ledMatrix.print(timeStr);
}

// Interrupt service routine for changing frequency 
void ISR_encoderChange() {
  if ((millis() - last_time) < 50)  // debounce time is 50ms
    return;

  if (digitalRead(DT_PIN) == HIGH) {
    // the encoder is rotating in counter-clockwise direction => decrease the frequency
    fm_freq_current = fm_freq_current - delta_freq;
    if (fm_freq_current <= fm_freq_min) {
      fm_freq_current = fm_freq_min;
      }
  } 
  
  else {
    // the encoder is rotating in clockwise direction => increase the frequency
    fm_freq_current = fm_freq_current + delta_freq;
    if (fm_freq_current >= fm_freq_max) {
      fm_freq_current = fm_freq_max;
      }
  }

  last_time = millis();

  // Mark encoder activity so the display shows radio info
  showingRadioInfo = true;
  lastEncoderActivityMs = millis();
}
// End.

