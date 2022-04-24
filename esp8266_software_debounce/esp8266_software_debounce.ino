// software debounce by using a timer

#include <Ticker.h>

// set the debounce time here in [ms] ; 40 seems to be a good value :)
#define MAX_BOUND_DURATION_MS 40

#define DEBOUNCE_TIMER_INTERVAL_MS        1
#define MAX_BOUND_COUNT (MAX_BOUND_DURATION_MS/DEBOUNCE_TIMER_INTERVAL_MS)





unsigned int mainLoopCounter = 0;




// push button debounce stuff

Ticker debounceTimer;

const int pushButtonPin = 2;    // GPIO2, pin D4

volatile bool oldPushButtonState = false;
volatile int pushButtonStableLevelCount = 0;

volatile int pushButtonPushedCount = 0;
volatile int pushButtonReleasedCount = 0;




void debounceHandler()
{
  // read the current push button state
  bool newPushButtonState=getPushButtonState();

  // same state as previous => exit
  if(newPushButtonState==oldPushButtonState) {
    return;
  }

  
  // if old state is low (the user pushes the button)
  if (!oldPushButtonState) {

      if(newPushButtonState) { // button stills pushed
        pushButtonStableLevelCount++;
      }
      else { // button released
        pushButtonStableLevelCount=0;
      }
       // button stills pushed enough longer to say "OK, no more bounce"
      if (pushButtonStableLevelCount>=MAX_BOUND_COUNT) {
        oldPushButtonState=true; // set internal state to PUSHED
        pushButtonPushedCount++;
        pushButtonStableLevelCount=0;
      }
  }
  else 
  {
      if(!newPushButtonState) { // button stills released
        pushButtonStableLevelCount++;
      }
      else { // button repushed
        pushButtonStableLevelCount=0;
      }
       // button stills released enough longer to say "OK, no more bounce"
      if (pushButtonStableLevelCount>=MAX_BOUND_COUNT) {
        oldPushButtonState=false; // set internal state to RELEASED
        pushButtonReleasedCount++;
        pushButtonStableLevelCount=0;
      }
  }
}



bool getPushButtonState(void) {
  return digitalRead(pushButtonPin) == HIGH;
}



void setup()
{

  Serial.begin(9600);

  pinMode(pushButtonPin, INPUT);

  oldPushButtonState = getPushButtonState();

  debounceTimer.attach(DEBOUNCE_TIMER_INTERVAL_MS / 1000., debounceHandler); // debounce period in [ms]

}

void loop()
{
  Serial.print(mainLoopCounter);
  Serial.print(" pushButtonCount:");
  Serial.print(pushButtonPushedCount, DEC);
  Serial.print(" / ");
  Serial.print(pushButtonReleasedCount, DEC);
  Serial.println();
  delayMicroseconds(125000);
  mainLoopCounter++;
}
