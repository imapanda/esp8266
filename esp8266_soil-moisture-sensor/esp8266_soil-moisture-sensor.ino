/*
 * 
 * C.BRESSY
 * 2022.05.16
 * 
 * Example code for the moisture sensor
 * Editor     : Lauren
 * Date       : 13.01.2012
 * Version    : 1.0
 * Connect the sensor to the A0(Analog 0) pin on the Arduino board
  
 * the sensor value description
 * 0  ~300     dry soil
 * 300~700     humid soil
 * 700~950     in water
 * 
 */

#include <Adafruit_SSD1306.h>

#define SENSOR_UPDATE_TIME 5  // in seconds
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


void setup(){
  
  Serial.begin(115200);
  Serial.println("Starting setup");
  
  Serial.println("Begin OLED screen");
  if(!ecranOLED.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDRESS))
    while(1);

  ecranOLED.clearDisplay();
  ecranOLED.display();

  ecranOLED.setTextSize(4);  // Caracters size (1 to 3)
  ecranOLED.setCursor(0, 0);

  ecranOLED.setTextColor(SSD1306_WHITE);

  ecranOLED.println("github.com/imapanda");
  ecranOLED.println("2022.05.22");
  ecranOLED.display();
  
}

void loop(){
  ecranOLED.clearDisplay(); // clear buffer
  
  Serial.print("Moisture Sensor Value:");
  Serial.println(analogRead(0));

  ecranOLED.setCursor(0, 0);
  char s[16 + 1] = "                "; 
  sprintf(s, "% 4d", analogRead(0)); // print to string
  ecranOLED.print("");
  ecranOLED.println(s);
  ecranOLED.print("");

  ecranOLED.display();
  delay(SENSOR_UPDATE_TIME * 1000); // 100 = .1 second
  
}
