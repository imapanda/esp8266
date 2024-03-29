/*********
  24.04.2022
  C.BRESSY
  P.BRESSY

  Using rotative encoder with software debounce for push button

  Pinout of rotative encoder :
  GND, +, SW, DT, CLK

*********/

// software debounce by using a timer
#include <Ticker.h>
// set the debounce time here in [ms] ; 40 seems to be a good value :)
#define MAX_BOUND_DURATION_MS 40

#define DEBOUNCE_TIMER_INTERVAL_MS        1
#define MAX_BOUND_COUNT (MAX_BOUND_DURATION_MS/DEBOUNCE_TIMER_INTERVAL_MS)

// GPIO definition
const int clkInterruptPin = 14; // GPIO14, pin D5
const int dataPin = 12;         // GPIO12, pin D6
const int pushButtonPin = 2;    // GPIO2,  pin D4


// Push button debounce stuff
Ticker debounceTimer;

volatile bool oldPushButtonState = false;
volatile int pushButtonStableLevelCount = 0;

volatile int pushButtonPushedCount = 0;
volatile int pushButtonReleasedCount = 0;


// Encoder stuff
#define ENCODER_STEPS 10
volatile int encoderPos = 0;
char oldDataPinState = 0;


// Used to print to serial
unsigned int loopCounter = 0;

// LEDs GPIO pinout
int RGBLED_RED   = 5;
int RGBLED_GREEN = 4;
int RGBLED_BLUE  = 13;

// ICACHE_RAM_ATTR function decorator which indicates to the linker that
// the function must be put in a special memory area which is dedicated to interrupts,
// with higher access speed, depends on processors as needed
// Without the ICACHE_RAM_ATTR decorator, esp8266 throws ISR not in IRAM! and crashes
ICACHE_RAM_ATTR void clkInterrupt() {
  //Adds or substract 1 to encoder pos multiplied by ENCODER_STEPS
  encoderPos += digitalRead(dataPin) == HIGH ? -1 * ENCODER_STEPS : +1 * ENCODER_STEPS;

  // encoderPos between 0 and 255 for LED control
  if(encoderPos < 0)  { encoderPos=0;  }
  if(encoderPos > 250){ encoderPos=250;}
}

void debounceHandler() {
  // read the current push button state
  bool newPushButtonState = getPushButtonState();

  // same state as previous => exit
  if (newPushButtonState == oldPushButtonState) {
    return;
  }


  // if old state is low (the user pushes the button)
  if (!oldPushButtonState) {

    if (newPushButtonState) { // button stills pushed
      pushButtonStableLevelCount++;
    }
    else { // button released
      pushButtonStableLevelCount = 0;
    }
    // button stills pushed enough longer to say "OK, no more bounce"
    if (pushButtonStableLevelCount >= MAX_BOUND_COUNT) {
      oldPushButtonState = true; // set internal state to PUSHED
      pushButtonPushedCount++;
      pushButtonStableLevelCount = 0;
    }
  }
  else {
    if (!newPushButtonState) { // button stills released
      pushButtonStableLevelCount++;
    }
    else { // button repushed
      pushButtonStableLevelCount = 0;
    }
    // button stills released enough longer to say "OK, no more bounce"
    if (pushButtonStableLevelCount >= MAX_BOUND_COUNT) {
      oldPushButtonState = false; // set internal state to RELEASED
      pushButtonReleasedCount++;
      pushButtonStableLevelCount = 0;
    }
  }
}


bool getPushButtonState(void) {
  return digitalRead(pushButtonPin) == HIGH;
}

void rgb_led_write(int led_mode, int intensity){

  // OFF mode
  digitalWrite(RGBLED_RED, LOW); // TODO : replace by analogWrite
  digitalWrite(RGBLED_BLUE, LOW); // TODO : replace by analogWrite
  digitalWrite(RGBLED_GREEN, LOW); // TODO : replace by analogWrite
  //analogWrite(RGBLED_RED, 0);
  //analogWrite(RGBLED_BLUE, 0);
  //analogWrite(RGBLED_GREEN, 0);
  
  intensity = 255; // TODO : temp
  
  if(led_mode == 0){
    digitalWrite(RGBLED_RED, HIGH); // TODO : replace by analogWrite
    //analogWrite(RGBLED_RED, intensity);
    return;
  }
  if(led_mode ==1){
    digitalWrite(RGBLED_GREEN, HIGH); // TODO : replace by analogWrite
    //analogWrite(RGBLED_GREEN, intensity);
    return;
  }
  if(led_mode ==2){
    digitalWrite(RGBLED_BLUE, HIGH); // TODO : replace by analogWrite
    //analogWrite(RGBLED_BLUE, intensity);
    return;
  }

  // White mode
  digitalWrite(RGBLED_RED, HIGH); // TODO : replace by analogWrite
  digitalWrite(RGBLED_BLUE, HIGH); // TODO : replace by analogWrite
  digitalWrite(RGBLED_GREEN, HIGH); // TODO : replace by analogWrite
//  analogWrite(RGBLED_RED, intensity);
//  analogWrite(RGBLED_BLUE, intensity);
//  analogWrite(RGBLED_GREEN, intensity);
  return;

}

void setup() {

  Serial.begin(9600);

  pinMode(RGBLED_RED, OUTPUT);
  pinMode(RGBLED_GREEN, OUTPUT);
  pinMode(RGBLED_BLUE, OUTPUT);

  pinMode(dataPin, INPUT_PULLUP);
  pinMode(clkInterruptPin, INPUT);
  pinMode(pushButtonPin, INPUT);

  attachInterrupt(digitalPinToInterrupt(clkInterruptPin), clkInterrupt, RISING);
  oldDataPinState = digitalRead(dataPin);

  oldPushButtonState = getPushButtonState();
  debounceTimer.attach(DEBOUNCE_TIMER_INTERVAL_MS / 1000., debounceHandler); // debounce period in [ms]
}


void loop() {
  if (oldDataPinState != digitalRead(dataPin)) {
    oldDataPinState = digitalRead(dataPin);
    delayMicroseconds(100000);
  }

  Serial.print(loopCounter);
  Serial.print(" encoderPos:");
  Serial.print(encoderPos, DEC);
  Serial.print(" pushButtonCount:");
  Serial.print(pushButtonPushedCount, DEC);
  Serial.print(" / ");
  Serial.print(pushButtonReleasedCount, DEC);
  Serial.print(" - ");
  Serial.print(pushButtonReleasedCount%4, DEC);
  Serial.println();

  rgb_led_write(pushButtonReleasedCount%4, encoderPos);
  
  delayMicroseconds(125000);
  loopCounter++;
}
