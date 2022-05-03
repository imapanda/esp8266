/*********
  C.BRESSY & P.BRESSY - 22.04.2022
  Tests afficheur DL1416 B

  # Doc DL1416-B :
  Digits order : 3, 2, 1, 0

  Top pins (face view):
    Up line :      20 to 11
    Bottom line :   1 to 10

  # PIN   FUNCTION
  #  1    DATA D5         # 11    Digit select A1
  #  2    DATA D4         # 12    Unused
  #  3    DATA D0         # 13    Unused
  #  4    DATA D1         # 14    Unused
  #  5    DATA D2         # 15    Unused
  #  6    DATA D3         # 16    Unused
  #  7    Chip enable     # 17    Unused
  #  8    Write           # 18    V+
  #  9    Cursor input    # 19    V-
  # 10    Digit select A0 # 20    DATA D6

  Useful informations from documentation :
  - TW (Time to Write) says at 25 degrees it's at least 0,25 microseconds
  - D0 to D6 you should wait 50 nanoseconds before OFF write
  - Port 7 must be set to GND if not used (if only 1 DL1416)

*********/
#include <ESP8266WiFi.h>
#include <Ticker.h>  //Ticker Library

#define NUM_DIGIT 4

#define DISPLAY_CLOCK 1

// Remove TX & RX pins for debugging
#define DEBUG_MODE 0


// GPIOs pins definition
// int GPIO_CHIP_ENABLE = 12;     // Removed cause GPIO_09 and GPIO_10 can ONLY receive data (no output)
const uint8_t GPIO_WRITE = 14;              // => D5
const uint8_t GPIO_DIGIT_SELECT_A0 = 13;    // => D7
const uint8_t GPIO_DIGIT_SELECT_A1 = 12;    // => D6

const uint8_t GPIO_D0 = 0;   // => D3  on NodeMCU
const uint8_t GPIO_D1 = 1;   // => D10 on NodeMCU
const uint8_t GPIO_D2 = 2;   // => D4  on NodeMCU
const uint8_t GPIO_D3 = 3;   // => D9  on NodeMCU
const uint8_t GPIO_D4 = 4;   // => D2  on NodeMCU
const uint8_t GPIO_D5 = 5;   // => D1  on NodeMCU
const uint8_t GPIO_D6 = 15;  // => D8  on NodeMCU

const uint8_t GPIO_CHIP_ENABLE_TEST = 16;

int SLEEPING_TIME = 1; // #seconds

int show_clock = 1;

Ticker myClock;
Ticker timeUpdate;

unsigned int count = 0;
char *msg = " SALUT LES GARS ";

unsigned int hh = 0;
unsigned int mm = 0;
unsigned int ss = 0;
unsigned int ms = 0;
unsigned int x = 0;
const unsigned int displayPeriodMS = 500;
char s[NUM_DIGIT + 1] = "0000";


/*
 * New clock update params :
 */
unsigned long local_epoch = 0;
const unsigned int displayPeriodMS_clock = 10;
/* Useful Constants */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)
#define MS_IN_SEC 200  // Normally 1000 but useful to fast-forward time for demo
// Useful Macros for getting elapsed time
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)  
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN) 
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)  

void chip_enable() {
  //digitalWrite(GPIO_CHIP_ENABLE, HIGH); // sets the pin on
}

void chip_disable() {
  //digitalWrite(GPIO_CHIP_ENABLE, LOW); // sets the pin off
}

void write_enable() {
  digitalWrite(GPIO_WRITE, HIGH); // Enable
  delayMicroseconds(50);          // pauses for 50 microseconds
}

void write_disable() {
  digitalWrite(GPIO_WRITE, LOW);  // Disable
  delayMicroseconds(50);          // pauses for 50 microseconds
}

void digit_select(int digit) {
  if (digit > 3 || digit < 0) {
    return;  // digit should be in 0-3 range.
  }

  // Désactivation des pins
  digitalWrite(GPIO_DIGIT_SELECT_A0, LOW); // Disable
  digitalWrite(GPIO_DIGIT_SELECT_A1, LOW); // Disable

  if (digit == 0) {
    return;
  }

  if (digit == 1) {
    digitalWrite(GPIO_DIGIT_SELECT_A0, HIGH); // Enable
    return;
  }

  if (digit == 2) {
    digitalWrite(GPIO_DIGIT_SELECT_A1, HIGH); // Enable
    return;
  }

  if (digit == 3) {
    digitalWrite(GPIO_DIGIT_SELECT_A0, HIGH); // Enable
    digitalWrite(GPIO_DIGIT_SELECT_A1, HIGH); // Enable
    return;
  }
}

void digit_unselect() {
  //  Désactivation des deux digit select
  digitalWrite(GPIO_DIGIT_SELECT_A0, LOW); // Disable
  digitalWrite(GPIO_DIGIT_SELECT_A1, LOW); // Disable
}

void set_data(unsigned int x) {
  // x = ascii repr of car

  // writes values a to g into D0 to D6
  // Values a to g should be >= 0 and <= 1
  digitalWrite(GPIO_D0, x & 0x01 ? 1 : 0);
  digitalWrite(GPIO_D1, x & 0x02 ? 1 : 0);
  digitalWrite(GPIO_D2, x & 0x04 ? 1 : 0);
  digitalWrite(GPIO_D3, x & 0x08 ? 1 : 0);
  digitalWrite(GPIO_D4, x & 0x10 ? 1 : 0);
  digitalWrite(GPIO_D5, x & 0x20 ? 1 : 0);
  digitalWrite(GPIO_D6, x & 0x40 ? 1 : 0);
}


void update_time_ntp() {
  // TODO : update time through NTP request
  digitalWrite(GPIO_CHIP_ENABLE_TEST, HIGH);
  delayMicroseconds(500000);
  digitalWrite(GPIO_CHIP_ENABLE_TEST, LOW);
}

void display_update() {

  // calculate difference
  unsigned long cur_epoch = millis();  // divide by 1000 to obtain seconds elapsed since boot

  // We can now update display every 200 ms (if cur_epoch > local_epoch + 200)
  if(cur_epoch >= local_epoch + 200){
    // sprintf(s, "%02d%02d", mm, ss); // print to string
    sprintf(s, "%02d%02d", numberOfMinutes(cur_epoch/MS_IN_SEC), numberOfSeconds(cur_epoch/MS_IN_SEC));  // print to string
    for (int digit = 0; digit < NUM_DIGIT; digit++) {
      chip_disable();
      delayMicroseconds(50);      // pauses for 50 microseconds
      digit_select(digit);
      delayMicroseconds(50);      // pauses for 50 microseconds
      x = s[NUM_DIGIT - 1 - digit];
  
      set_data(x);
      write_disable();
      delayMicroseconds(50);      // pauses for 50 microseconds
      write_enable();
      digit_unselect();
      chip_enable();
  
      delayMicroseconds(50);      // pauses for 50 microseconds
    }

    local_epoch = millis();
    
  }

}

void updateClockAndDisplay() {

#if DISPLAY_CLOCK

  show_clock = ms < 500;


  if (show_clock) {
    sprintf(s, "%02d%02d", mm, ss); // print to string
    for (int digit = 0; digit < NUM_DIGIT; digit++) {
      chip_disable();
      delayMicroseconds(50);      // pauses for 50 microseconds
      digit_select(digit);
      delayMicroseconds(50);      // pauses for 50 microseconds
      x = s[NUM_DIGIT - 1 - digit];
#if DEBUG_MODE
      Serial.println(x);
#endif
      set_data(x);
      write_disable();
      delayMicroseconds(50);      // pauses for 50 microseconds
      write_enable();
      digit_unselect();
      chip_enable();

      delayMicroseconds(50);      // pauses for 50 microseconds
    }


  }
  else {
    // Display temp for 1/2 second
    //sprintf(s, "%04f", DHT.temperature); // print to string
    for (int digit = 0; digit < NUM_DIGIT; digit++) {
      chip_disable();
      digit_select(digit);
      x = s[NUM_DIGIT - 1 - digit];
      //Serial.println(x);
      set_data(x);
      write_disable();
      delayMicroseconds(50);      // pauses for 50 microseconds
      write_enable();
      digit_unselect();
      chip_enable();

      delayMicroseconds(50);      // pauses for 50 microseconds
    }
  }


  //always update hhmmss
  ms += displayPeriodMS;
  if (ms >= 1000) {
    ms -= 1000;
    ss++;
    if (ss > 59) {
      ss = 0;
      mm++;
    }
    if (mm > 59) {
      mm = 0;
      hh++;
    }
    if (hh > 23) {
      hh = 0;
    }

  }


#else
  // Here we print ascii car
  int k = 0;
  for (k = 0; k < NUM_DIGIT; k++) {
    x = msg[count + k];
    chip_disable();
    digit_select(NUM_DIGIT - 1 - k);
    set_data(x);
    write_disable();
    delayMicroseconds(50); // pauses for 50 microseconds
    write_enable();
    digit_unselect();
    chip_enable();
    delayMicroseconds(50); // pauses for 50 microseconds
  }

  count++;
  // loop to the begining of the message
  if (msg[count + k - 1 ] == 0) {
    count = 0;
  }

#endif
  return;
}


void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

#if DEBUG_MODE
  Serial.begin(9600);
  delay(100);
  Serial.println("Starting setup");
#endif

  // initialize GPIOs as outputs.
  // pinMode(GPIO_CHIP_ENABLE, OUTPUT);
  pinMode(GPIO_WRITE, OUTPUT);
  pinMode(GPIO_DIGIT_SELECT_A0, OUTPUT);
  pinMode(GPIO_DIGIT_SELECT_A1, OUTPUT);
  //pinMode(DHT11_PIN, INPUT);

  pinMode(GPIO_D0, OUTPUT);
  pinMode(GPIO_D1, OUTPUT);
  pinMode(GPIO_D2, OUTPUT);
  pinMode(GPIO_D3, OUTPUT);
  pinMode(GPIO_D4, OUTPUT);
  pinMode(GPIO_D5, OUTPUT);
  pinMode(GPIO_D6, OUTPUT);

  pinMode(GPIO_CHIP_ENABLE_TEST, OUTPUT);

  chip_enable();
  write_disable();

  // Watchdog error fixed ?
  // ESP.wdtDisable();

  // Setup Wifi
  digitalWrite(LED_BUILTIN, HIGH);

  WiFi.begin("RANTANPLAN", "F5QiRNX1rCf9iqNaYg");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }

#if DEBUG_MODE
  //Serial.print("Connected, IP address: ");
  //Serial.println(WiFi.localIP());
#endif


  // TODO : how to not fail updateclock when running update_time
  //myClock.attach(displayPeriodMS / 1000., updateClockAndDisplay); // displayPeriod in [ms]
  myClock.attach(displayPeriodMS_clock / 1000., display_update); // displayPeriod in [ms]
  timeUpdate.attach(2000 / 1000., update_time_ntp); // displayPeriod in [ms]

  //analogWriteFreq(1000); // 1000Hz
  //analogWrite(LED_BUILTIN, 30); // ratio=30/255


  // Display boot end to user through LED BUILTIN
  int i = 0;
  for (i = 0; i < 10; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delayMicroseconds(30000);
    digitalWrite(LED_BUILTIN, LOW);
    delayMicroseconds(30000);
  }

#if DEBUG_MODE
  Serial.println("End setup");
#endif
}

// The loop function runs over and over again forever
// Warning, does need to be < 1 second total time
void loop() {
  // Bark to watchdog
  ESP.wdtFeed();

  // dummy freq on LED_BUILTIN
  digitalWrite(LED_BUILTIN, LOW);
  delayMicroseconds(1000);
  digitalWrite(LED_BUILTIN, HIGH);
  delayMicroseconds(30000);

}
