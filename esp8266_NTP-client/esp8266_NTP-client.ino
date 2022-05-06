#include <Ticker.h>
#include <NTP.h>  //https://github.com/sstaub/NTP
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>

const uint8_t PIN_COLOR_1 = 0;   // => D3 green
const uint8_t PIN_COLOR_2 = 4;   // => D2

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
  // add millis diff to local epoch

  // Warning, delay() inside tickers does not work because because
  // Ticker functions are using the underlying timer
  Serial.println("update_ntp start");
  Serial.print("local_epoch : ");
  Serial.println(local_epoch);
  digitalWrite(PIN_COLOR_1, LOW);
  digitalWrite(PIN_COLOR_2, HIGH);

//  // Update NTP here.
//  ntp.update();

  Serial.println(ntp.formattedTime("%d. %B %Y")); // dd. Mmm yyyy
  Serial.println(ntp.formattedTime("%A %T")); // Www hh:mm:ss
  
  digitalWrite(PIN_COLOR_2, LOW);
  digitalWrite(PIN_COLOR_1, HIGH);
  Serial.println("update_ntp done");
}


void setup() {

  Serial.begin(9600);
  delay(100);
  Serial.println("Starting setup");
  
  pinMode(PIN_COLOR_1, OUTPUT);
  pinMode(PIN_COLOR_2, OUTPUT);
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
  ntp.updateInterval(5000);
  
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
  ntp.update();
  Serial.print("LOOP - epoch() : ");
  Serial.print(ntp.epoch());
  
  Serial.print(" - ");
  
  Serial.print(local_epoch);
  
  Serial.print(" - ");

  Serial.print(numberOfHours  (ntp.epoch())/10);
  Serial.print(numberOfHours  (ntp.epoch())/10);
  Serial.print(numberOfMinutes(ntp.epoch())/10);
  Serial.print(numberOfMinutes(ntp.epoch())%10);
  Serial.print(numberOfSeconds(ntp.epoch())/10);
  Serial.print(numberOfSeconds(ntp.epoch())%10);

  Serial.print(" - ");

  Serial.println(ntp.formattedTime("%H%M%S")); // Www hh:mm:ss
  
  delay(500);
}
