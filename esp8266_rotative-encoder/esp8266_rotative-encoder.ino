
int clkInterruptPin = 14; // GPIO14, pin D5
int dataPin = 12;         // GPIO12, pin D6
int pushButtonPin = 2;    // GPIO2, pin D4

volatile int encoderPos = 0;
volatile int pushButtonCount = 0;
char oldDataPinState=0;

unsigned int loopCounter = 0;

ICACHE_RAM_ATTR void clkInterrupt()
{
    encoderPos += digitalRead(dataPin) == HIGH ? 1 : -1;
}
#if 0
ICACHE_RAM_ATTR void pushButtonInterrupt()
{
    pushButtonCount++;
}
#endif
void setup()
{

    Serial.begin(9600);

    pinMode(dataPin, INPUT_PULLUP);
    pinMode(clkInterruptPin, INPUT);
    pinMode(pushButtonPin, INPUT);
    
    attachInterrupt(digitalPinToInterrupt(clkInterruptPin), clkInterrupt, RISING);
    //attachInterrupt(digitalPinToInterrupt(pushButtonPin), pushButtonInterrupt, RISING);
    
    oldDataPinState=digitalRead(dataPin);
}

void loop()
{
    if (oldDataPinState!=digitalRead(dataPin)) {
        oldDataPinState=digitalRead(dataPin);
        pushButtonCount++;
        delayMicroseconds(100000);
    }
           

    Serial.print(loopCounter);
    Serial.print(" encoderPos:");
    Serial.print(encoderPos, DEC);
    Serial.print(" pushButtonCount:");
    Serial.print(pushButtonCount, DEC);
    Serial.println();
    delayMicroseconds(125000);
    loopCounter++;
}
