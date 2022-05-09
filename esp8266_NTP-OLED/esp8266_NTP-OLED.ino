#include <Ticker.h>
#include <NTP.h>  //https://github.com/sstaub/NTP
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <Adafruit_SSD1306.h>


// ----------------------------------------------------------------------
// Status led (Green/Orange) pins
// ----------------------------------------------------------------------
const uint8_t PIN_COLOR_1 = 0;   // => D3 green
const uint8_t PIN_COLOR_2 = 2;   // => D4 orange


// ----------------------------------------------------------------------
// Oled screen definition (128x32)
// ----------------------------------------------------------------------
#define SCREEN_WIDTH        128
#define SCREEN_HEIGHT        32
#define SCREEN_RESET_PIN     -1  // -1 if your screen doesn't have reset pin
#define SCREEN_I2C_ADDRESS 0x3C

Adafruit_SSD1306 ecranOLED(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, SCREEN_RESET_PIN);

// 3 colors available in Adafruit_SSD1306.h
#define SSD1306_BLACK 0             // Draw 'off' pixels
#define SSD1306_WHITE 1             // Draw 'on' pixels
#define SSD1306_INVERSE 2           // Invert pixels





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
// Hardware timer
// ----------------------------------------------------------------------
Ticker segment_ticker;
Ticker NTP_sync_ticker;


// ----------------------------------------------------------------------
// set Wi-Fi SSID and password + NTP settings
// ----------------------------------------------------------------------
const char *ssid     = "RANTANPLAN";
const char *password = "F5QiRNX1rCf9iqNaYg";
WiFiUDP wifiUdp;
NTP ntp(wifiUdp);


// ----------------------------------------------------------------------
// BEGINING OF CODE
// ----------------------------------------------------------------------


void display_time(){
  local_epoch = millis();  // Update time
  return;
}
void update_ntp(){
//  // add millis diff to local epoch
//
//  // Warning, delay() inside tickers does not work because because
//  // Ticker functions are using the underlying timer
//  Serial.println("update_ntp start");
//  Serial.print("local_epoch : ");
//  Serial.println(local_epoch);
//  digitalWrite(PIN_COLOR_1, LOW);
//  digitalWrite(PIN_COLOR_2, HIGH);
//
////  // Update NTP here.
////  ntp.update();
//
//  Serial.println(ntp.formattedTime("%d. %B %Y")); // dd. Mmm yyyy
//  Serial.println(ntp.formattedTime("%A %T")); // Www hh:mm:ss
//  
//  digitalWrite(PIN_COLOR_2, LOW);
//  digitalWrite(PIN_COLOR_1, HIGH);
//  Serial.println("update_ntp done");
}


uint8_t display_oled(uint8_t row, uint16_t col, char* text){
  // Column O to 3
  if(col > 3){return -1;}
  
  //ecranOLED.clearDisplay(); // clear buffer
  //ecranOLED.display();

  // erase
  ecranOLED.setCursor(row, col);
  //ecranOLED.print("                ");

  ecranOLED.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
  //display.println(3.141592);


  ecranOLED.setCursor(row, col);
  ecranOLED.print(text);
  //ecranOLED.println("2022.04.29");
  ecranOLED.display();
  ecranOLED.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
  return 0;  
}


void setup() {
  pinMode(PIN_COLOR_1, OUTPUT);
  pinMode(PIN_COLOR_2, OUTPUT);
  
  Serial.begin(9600);
  delay(100);
  
  Serial.println("Starting setup");
  
  Serial.println("Begin OLED screen");
  if(!ecranOLED.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDRESS))
    while(1);

  ecranOLED.clearDisplay();
  ecranOLED.display();

  ecranOLED.setTextSize(1);  // Caracters size (1 to 3)
  ecranOLED.setCursor(0, 0);

  ecranOLED.setTextColor(SSD1306_WHITE);

  ecranOLED.println("github.com/imapanda");
  ecranOLED.println("2022.04.29");
  ecranOLED.display();
  

  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("pinMode PIN_COLOR OUTPUT done.");
  
  digitalWrite(PIN_COLOR_2, HIGH);
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting.");
  while ( WiFi.status() != WL_CONNECTED ) {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
  }
  Serial.println("connected");
  ntp.ruleDST("CEST", Last, Sun, Mar, 2, 120); // last sunday in march 2:00, timetone +120min (+1 GMT + 1h summertime offset)
  //ntp.ruleSTD("CET", Last, Sun, Oct, 3, 60); // last sunday in october 3:00, timezone +60min (+1 GMT)
  ntp.ruleSTD("CET", Last, Sun, Oct, 3, 60); // last sunday in october 3:00, timezone +60min (+1 GMT)

  //updateInterval(uint32_t interval);
  ntp.updateInterval(2000);  // every 2 seconds
  
  ntp.begin();
  Serial.println("start NTP");
  
  segment_ticker.attach_ms(DELAY_MS,display_time);
  NTP_sync_ticker.attach(5, update_ntp);  // 5 secs
  Serial.println("_ticker.attach_ms done.");
  
  digitalWrite(PIN_COLOR_2, LOW);
  
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(PIN_COLOR_1, HIGH);
}

void loop() {
  ESP.wdtFeed();
  ecranOLED.clearDisplay(); // clear buffer
  
  bool res = ntp.update();
  char s[16 + 1] = "                "; //16 times white space
  //sprintf(s, "%-6d", loopCounter); // print to string ( minus is left justified)
  ecranOLED.setCursor(0, 0);
  sprintf(s, "%10d", ntp.epoch()); // print to string
  ecranOLED.print("epoch() : ");
  ecranOLED.println(s);
  
  
  Serial.print("LOOP - epoch() : ");
  Serial.print(ntp.epoch());
  
  Serial.print(" - ");
  
  Serial.print(local_epoch);
  
  Serial.print(" - ");

  ecranOLED.print("hhmmss: ");

  sprintf(s, "%1d", ntp.hours()/10); // print to string
  ecranOLED.print(s);
  sprintf(s, "%1d", ntp.hours()%10); // print to string
  ecranOLED.print(s);
  sprintf(s, "%1d", ntp.minutes()/10); // print to string
  ecranOLED.print(s);
  sprintf(s, "%1d", ntp.minutes()%10); // print to string
  ecranOLED.print(s);
  sprintf(s, "%1d", ntp.seconds()/10); // print to string
  ecranOLED.print(s);
  sprintf(s, "%1d", ntp.seconds()%10); // print to string
  ecranOLED.println(s);
  
  Serial.print(ntp.hours()/10);
  Serial.print(ntp.hours()%10);
  Serial.print(ntp.minutes()/10);
  Serial.print(ntp.minutes()%10);
  Serial.print(ntp.seconds()/10);
  Serial.print(ntp.seconds()%10);

  ecranOLED.print("formattedTime");
  ecranOLED.println(ntp.formattedTime("%H%M%S")); // Www hh:mm:ss
  
  Serial.print(" - ");
  Serial.print(ntp.formattedTime("%H%M%S")); // Www hh:mm:ss

  Serial.print(" - ");  
  Serial.print(ntp.ruleDST());

  Serial.print(" - ");  
  Serial.print(ntp.ruleSTD());

  Serial.print(" - ");  
  Serial.print(ntp.tzName());

  Serial.print(" - res : ");
  Serial.println(res);

  ecranOLED.display();
  delay(1000);  // wait 5 seconds
}
