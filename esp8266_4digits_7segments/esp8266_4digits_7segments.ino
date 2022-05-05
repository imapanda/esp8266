/*
 * 2022.04.22 - C.BRESSY
 * 
 * Running 4 digits common cathode 7 segments display.
 * Objective here is to run a NTP sync every X minutes to sync our time with NTP.
 * 
 * Used 4 digits display in current ocnfiguration : kw4-802CGB (common cathode)
 * https://github.com/imapanda/7-segments-datasheets/blob/main/kw4-802CGB%2Bkw4-802AGB.pdf
 * 
 */

// ----------------------------------------------------------------------
// Libraries includes
// ----------------------------------------------------------------------
#include <Ticker.h>

// ----------------------------------------------------------------------
// Easy define on/off gpios for each letter
// ----------------------------------------------------------------------
const byte segCode[15][8] = {
  //  a  b  c  d  e  f  g  .
    { 1, 1, 1, 1, 1, 1, 0, 0},  // 0
    { 0, 1, 1, 0, 0, 0, 0, 0},  // 1
    { 1, 1, 0, 1, 1, 0, 1, 0},  // 2
    { 1, 1, 1, 1, 0, 0, 1, 0},  // 3
    { 0, 1, 1, 0, 0, 1, 1, 0},  // 4
    { 1, 0, 1, 1, 0, 1, 1, 0},  // 5
    { 1, 0, 1, 1, 1, 1, 1, 0},  // 6
    { 1, 1, 1, 0, 0, 0, 0, 0},  // 7
    { 1, 1, 1, 1, 1, 1, 1, 0},  // 8
    { 1, 1, 1, 1, 0, 1, 1, 0},  // 9
    { 0, 0, 0, 0, 0, 0, 0, 1},  // .
    { 1, 0, 1, 1, 0, 1, 1, 0},  // s
    { 0, 1, 1, 0, 0, 1, 1, 0},  // y
    { 0, 0, 1, 0, 1, 0, 1, 0},  // n
    { 1, 0, 0, 1, 1, 1, 0, 0}   // c
};


// ----------------------------------------------------------------------
// This is used for displaying thousands, hundreds, tens and units each at a time
// ----------------------------------------------------------------------
typedef enum {
  E_3 = 0,
  E_2,
  E_1,
  E_0
} eDigit;

eDigit digit = E_3;  // On rythm with the call change state


// ----------------------------------------------------------------------
// Define segments GPIOs
// PIN_SEG_A = 2  => D4
// PIN_SEG_B = 14 => D5
// PIN_SEG_C = 12 => D6
// PIN_SEG_D = 13 => D7
// PIN_SEG_E = 15 => D8
// PIN_SEG_F = 3  => D9
// PIN_SEG_G = 1  => D10
// ----------------------------------------------------------------------
const uint8_t SEGMENT_PINS[] = {2, 14, 12, 13, 15, 3, 1 };   // { a b c d e f g ). --DOT is removed
//const uint8_t PIN_SEG_DP = 0; // => Pulses with 3rd digit because esp8266 doesn't have enough pins


// ----------------------------------------------------------------------
// Define cathodes
// Digit 1 is left, digit 4 is right
// ----------------------------------------------------------------------
const uint8_t PIN_CC_DIGIT_1 = 0;   // => D3
const uint8_t PIN_CC_DIGIT_2 = 4;   // => D2
const uint8_t PIN_CC_DIGIT_3 = 5;   // => D1
const uint8_t PIN_CC_DIGIT_4 = 16;  // => D0


// ----------------------------------------------------------------------
// Time variables
// ----------------------------------------------------------------------
unsigned long local_epoch = 0;
const unsigned int displayPeriodMS_clock = 10;
#define DELAY_MS 2

// ----------------------------------------------------------------------
// Useful Constants
// ----------------------------------------------------------------------
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)
#define MS_IN_SEC 1000 // Change here for fast forward


// ----------------------------------------------------------------------
// Useful Macros for getting elapsed time
// ----------------------------------------------------------------------
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)  
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN) 
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)  


// ----------------------------------------------------------------------
// hardware timer based on :
// ----------------------------------------------------------------------

// Push button debounce stuff
Ticker clock_ticker;

// ----------------------------------------------------------------------
// BEGINING OF CODE
// ----------------------------------------------------------------------


void displayDigit(int digit) {
  // Write values of digit using segCode onto SEGMENT_PINS
  uint8_t i;
  for (i = 0; i < 7; i++) {
    digitalWrite(SEGMENT_PINS[i], segCode[digit][i]);
  }
  return;
}


void display_time(){
  switch(digit) {
    case E_3:
      displayDigit(numberOfMinutes(local_epoch/MS_IN_SEC)/10);
      digitalWrite(PIN_CC_DIGIT_1, HIGH);
      delay(DELAY_MS);
      digitalWrite(PIN_CC_DIGIT_1, LOW);
      displayDigit(10);
      digit=E_2;
      break;
    case E_2:
      displayDigit(numberOfMinutes(local_epoch/MS_IN_SEC)%10);
      digitalWrite(PIN_CC_DIGIT_2, HIGH);
      delay(DELAY_MS);
      digitalWrite(PIN_CC_DIGIT_2, LOW);
      displayDigit(10);
      digit=E_1;
      break;
    case E_1:
      displayDigit(numberOfSeconds(local_epoch/MS_IN_SEC)/10);
      digitalWrite(PIN_CC_DIGIT_3, HIGH);
      delay(DELAY_MS);
      digitalWrite(PIN_CC_DIGIT_3, LOW);
      displayDigit(10);
      digit=E_0;
      break;
    case E_0:
      displayDigit(numberOfSeconds(local_epoch/MS_IN_SEC)%10);
      digitalWrite(PIN_CC_DIGIT_4, HIGH);
      delay(DELAY_MS);
      digitalWrite(PIN_CC_DIGIT_4, LOW);
      displayDigit(10);
      digit=E_3;
      break;
  }
  
  
  /*
  if (digit == E_3) {  // Enum thousands mode
    digitalWrite(PIN_CC_DIGIT_1, HIGH);
    displayDigit(numberOfMinutes(local_epoch/MS_IN_SEC)/10);
    delay(DELAY_MS);
    digitalWrite(PIN_CC_DIGIT_1, LOW);
    digit = E_2;
  } else if (digit == E_2) {  // Enum hundreds mode
    digitalWrite(PIN_CC_DIGIT_2, HIGH);
    displayDigit(numberOfMinutes(local_epoch/MS_IN_SEC)%10);
    delay(DELAY_MS);
    digitalWrite(PIN_CC_DIGIT_2, LOW);
    digit = E_1;
  } else if (digit == E_1) {  // Enum tens mode
    digitalWrite(PIN_CC_DIGIT_3, HIGH);
    displayDigit(numberOfSeconds(local_epoch/MS_IN_SEC)/10);
    delay(DELAY_MS);
    digitalWrite(PIN_CC_DIGIT_3, LOW);
    digit = E_0;
  } else {  // Enum units mode digit == E_0
    digitalWrite(PIN_CC_DIGIT_4, HIGH);
    displayDigit(numberOfSeconds(local_epoch/MS_IN_SEC)%10);
    delay(DELAY_MS);
    digitalWrite(PIN_CC_DIGIT_4, LOW);
    digit = E_3;
  }
  */
  local_epoch = millis();
  return;
}


void display_sync(){
  switch(digit) {
    case E_3:
      displayDigit(11);
      digitalWrite(PIN_CC_DIGIT_1, HIGH);
      delay(DELAY_MS);
      digitalWrite(PIN_CC_DIGIT_1, LOW);
      displayDigit(10);
      digit = E_2;
      break;
    case E_2:
      displayDigit(12);
      digitalWrite(PIN_CC_DIGIT_2, HIGH);
      delay(DELAY_MS);
      digitalWrite(PIN_CC_DIGIT_2, LOW);
      displayDigit(10);
      digit = E_1;
      break;
    case E_1:
      displayDigit(13);
      digitalWrite(PIN_CC_DIGIT_3, HIGH);
      delay(DELAY_MS);
      digitalWrite(PIN_CC_DIGIT_3, LOW);
      displayDigit(10);
      digit = E_0;
      break;
    case E_0:
      displayDigit(14);
      digitalWrite(PIN_CC_DIGIT_4, HIGH);
      delay(DELAY_MS);
      digitalWrite(PIN_CC_DIGIT_4, LOW);
      displayDigit(10);
      digit = E_3;
      break;
  } 
  /*
  if (digit == E_3) {  // Enum thousands mode
    digitalWrite(PIN_CC_DIGIT_1, HIGH);
    displayDigit(11);
    delay(DELAY_MS);
    digitalWrite(PIN_CC_DIGIT_1, LOW);
    digit = E_2;
  } else if (digit == E_2) {  // Enum hundreds mode
    digitalWrite(PIN_CC_DIGIT_2, HIGH);
    displayDigit(12);
    delay(DELAY_MS);
    digitalWrite(PIN_CC_DIGIT_2, LOW);
    digit = E_1;
  } else if (digit == E_1) {  // Enum tens mode
    digitalWrite(PIN_CC_DIGIT_3, HIGH);
    displayDigit(13);
    delay(DELAY_MS);
    digitalWrite(PIN_CC_DIGIT_3, LOW);
    digit = E_0;
  } else {  // Enum units mode digit == E_0
    digitalWrite(PIN_CC_DIGIT_4, HIGH);
    displayDigit(14);
    delay(DELAY_MS);
    digitalWrite(PIN_CC_DIGIT_4, LOW);
    digit = E_3;
  }
  */

  return;
}

void setup() {
  
  // initialize GPIO as outputs.
  pinMode(PIN_CC_DIGIT_1, OUTPUT);
  pinMode(PIN_CC_DIGIT_2, OUTPUT);
  pinMode(PIN_CC_DIGIT_3, OUTPUT);
  pinMode(PIN_CC_DIGIT_4, OUTPUT);

  // initialize digital pins as outputs.
  unsigned int i;
  for (i = 0; i < 7; i++) {
    pinMode(SEGMENT_PINS[i], OUTPUT);
  }
  
  

  // Calculate in seconds time you want to display :
  unsigned int loop_count = 2000/DELAY_MS;
  for (i = 0; i < loop_count; i++) {
    display_sync();
  }


  // TODO : Board crashes
  clock_ticker.attach_ms(100., display_time);
}

// the loop function runs over and over again forever
void loop() {
  // Bark to watchdog
  ESP.wdtFeed();
  // TODO : move this next line into an interrupt
  // display_time();
  delay(1);
}
