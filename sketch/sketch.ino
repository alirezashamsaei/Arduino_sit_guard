//Set Up Libraries
#include <Bounce2.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 6
#define LED_COUNT 7
#define BTN_PIN 13
#define LONGPRESS 1500
#define WORK_DURATION 10000    // in milliseconds,  2700000
#define REST_DURATION 7 * 1000 // in milliseconds
#define DEBUG_MODE true
// #define WORK_HUE_STEP 65536 / (WORK_DURATION * 10)
size_t i = 0;
static uint16_t currentLed = 0;
const int workHueStep = (int)22000 / (WORK_DURATION / 1000);
// const int workHueStep = 2200;
unsigned short hue = 22000;
unsigned long lastLightMillis = 0; // last time a color is shown on the led strip
unsigned long lastButtonPress = 0;
unsigned long lastWorkmillis = 0; // last time work watchdog was checked
unsigned long lastRestmillis = 0; // last time Rest watchdog was checked
unsigned long now = 0;

bool paused = false;
bool waitLight = false;

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB);

Bounce btn = Bounce();
////////////////////////////Custom Functions///////////////////////////////////

byte press(Bounce debouncer)
{ // short press >> 1 , long press >> 2

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
  wait,
  work,
  rest
};
STATE state = rest;
STATE lastState = state;

void setup()
{

  Serial.begin(9600);
  btn.attach(BTN_PIN, INPUT_PULLUP); // Attatch button to input pin
  strip.begin();                     // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.setBrightness(100);
  strip.show();
  Serial.println("+++++++++++ SETUP +++++++++++");
  Serial.print("Work Duration Is: ");
  Serial.println(WORK_DURATION);
  Serial.print("Step Is: ");
  Serial.println(workHueStep);
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
    if (now - lastLightMillis > 500)
    {

      strip.fill(strip.gamma32(strip.ColorHSV(0, 0, 0)), 0, 7);
      strip.show();
      lastLightMillis = millis();
    }
    // Unpause if Button is tapped
    if (press(btn) == 1)
    {
      paused = false;
      Serial.println("Resuming...");
    }
  }
  else // If not paused
  {
    switch (state)
    {

    case wait:

      // Start wait routine
      if (press(btn) == 1)
      {
        if (lastState == rest)
        {
          goToWork();
          break;
        }
        else if (lastState == work)
        {
          goToRest();
          break;
        }
      }


      
      if (now - lastLightMillis >= 1000){
        if (waitLight)
        {
          strip.fill(strip.gamma32(strip.ColorHSV(50000, 255, 255)), 0, 7);
          strip.show();
          waitLight = !waitLight;
        }
        else
        {
          strip.clear();
          strip.show();
          waitLight = !waitLight;
        }
        lastLightMillis = millis();
      }


      // End wait routine
      break;

    case rest:
      lastState = rest;

      // Start rest Routine
      if (now - lastRestmillis >= REST_DURATION)
      {
        state = wait;
        break;
      }

      if (now - lastLightMillis >= 100)
      {

        strip.setPixelColor(currentLed, strip.gamma32(strip.ColorHSV(i, 255, 255)));
        strip.show();
        currentLed = currentLed >= strip.numPixels() ? 0 : currentLed + 1;
        i += 300;
        lastLightMillis = millis();
        break;
      }

      if (press(btn) == 1)
      {
        paused = true;
        Serial.println("Pausing...");
      }
      else if (press(btn) == 2)
      {
        goToWork();
      }
      // End rest Routine
      break;

    case work:
      lastState = work;
      if (now - lastLightMillis > 1000)
      {

        renderLights(hue);
        hue -= workHueStep;
      }

      if (now - lastWorkmillis >= WORK_DURATION)
      {
        state = wait;
        break;
      }

      if (press(btn) == 1)
      {
        Serial.println("Pausing ...");
        paused = true;
      }
      else if (press(btn) == 2)
      {
        goToRest();
      }
      break;
    }
  }
}

void renderLights(unsigned short hue)
{

  if (now - lastLightMillis > 1000)
  {

    strip.fill(strip.gamma32(strip.ColorHSV(hue, 255, 255)), 0, 7);
    strip.show();
    lastLightMillis = millis();
    Serial.print("HUE: ");
    Serial.println(hue);
  }
}

void printStats()
{
  if (DEBUG_MODE)
  {
    Serial.print("NOW: ");
    Serial.print(now);
    Serial.print(" LW: ");
    Serial.print(lastWorkmillis);
    Serial.print(" LR: ");
    Serial.println(lastRestmillis);
  }
}

void goToRest()
{

  if (DEBUG_MODE)
  {
    printStats();
    Serial.println("Entering Rest mode...");
  }
  state = rest;
  lastRestmillis = millis();
}

void goToWork()
{
  hue = 22000;

  if (DEBUG_MODE)
  {
    printStats();
    Serial.println("Entering Work mode...");
  }
  state = work;
  lastWorkmillis = millis();
}