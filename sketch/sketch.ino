//Set Up Libraries
#include <Bounce2.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 6
#define LED_COUNT 7
#define BTN_PIN 13
#define LONGPRESS 1500
#define WORK_DURATION 10 * 1000      // in milliseconds
#define REST_DURATION 15 * 60 * 1000 // in milliseconds
#define WORK_LIGHT_TICK 2000         // in milliseconds
#define WORK_LIGHT_STEP 15000 * (WORK_LIGHT_TICK / WORK_DURATION)

unsigned long lastLightMillis = 0; // last time a color is shown on the led strip
unsigned long lastButtonPress = 0;
unsigned long lastWorkmillis = 0; // last time work watchdog was checked
unsigned long lastRestmillis = 0; // last time Rest watchdog was checked

unsigned long now = 0;
int step = 45 * 60 / 1; //TODO: fix this
bool paused = true;

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB);
unsigned short hue = 0;

Bounce btn = Bounce();
////////////////////////////Custom Functions///////////////////////////////////

byte press(Bounce debouncer)
{ // short press > 1 , long press > 2

  if (debouncer.fell())
  { // If button is pressed
    lastButtonPress = millis();
    now = millis();
    while (!debouncer.update())
    { // If buffon is still pressed
      unsigned long pressedTime = millis() - lastButtonPress;
      delay(10);
      if (pressedTime >= LONGPRESS)
      {
        return 2;
      }
    }
    return 1;
  }
}
////////////////////////////END OF Custom Functions///////////////////////////////////

//states for the state machine
enum STATE
{
  work,
  rest
};
STATE state = work;

void setup()
{

  Serial.begin(9600);
  btn.attach(BTN_PIN, INPUT_PULLUP); // Attatch button to input pin
  strip.begin();                     // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.setBrightness(120);
  strip.show();
  Serial.println("+++++++++++ SETUP +++++++++++");
}

void loop()
{
  now = millis(); // Update time. should be done first thing in the loop
  btn.update();
  handleState();
}

void handleState()
{
  if (paused)
  {
    // Render Lights
    if (now - lastLightMillis > 1000)
    {
      Serial.println("Paused...");
      lastLightMillis = now;
    }
    // Unpause if Button is tapped
    if (press(btn) == 1)
    {
      paused = false;
    }
  }
  else // If not paused
  {
    switch (state)
    {
      case rest :
      if (now - lastRestmillis > 1000) {

        Serial.println("Rest mode...");
        lastRestmillis = now;

      } 
      if (press(btn) == 1) {
        paused = true;
      }else if (press(btn) == 2)
      {
        state = work;
      }
      break;



      case work :
      if (now - lastWorkmillis > 1000) {

        Serial.println("Working....");
        lastWorkmillis = now;

      } 
      if (press(btn) == 1) {
        paused = true;
      }else if (press(btn) == 2)
      {
        state = rest;
      }
      break;
      }
  }
}
