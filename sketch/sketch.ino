//Set Up Libraries
#include <Bounce2.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 6
#define LED_COUNT 7
#define BTN_PIN 13
#define LONGPRESS 1500
#define WORK_DURATION 5 * 1000      // in milliseconds
#define REST_DURATION 10  * 1000 // in milliseconds
#define WORK_LIGHT_TICK 2000         // in milliseconds
#define WORK_LIGHT_STEP 15000 * (WORK_LIGHT_TICK / WORK_DURATION)

unsigned long lastLightMillis = 0; // last time a color is shown on the led strip
unsigned long lastButtonPress = 0;
unsigned long lastWorkmillis = 0; // last time work watchdog was checked
unsigned long lastRestmillis = 0; // last time Rest watchdog was checked

unsigned long now = 0;
int step = 45 * 60 / 1; //TODO: fix this
bool paused = false;

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB);
unsigned short hue = 0;

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
      // Start Work Routine

      
      renderLights(0);

      // End Work Routine

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
      case rest :

        // Start rest Routine
      
        renderLights(10000);
        // End rest Routine

         if ( now - lastRestmillis >=  REST_DURATION )
      {
        Serial.println("Entering Work mode...");
        state = work;
        lastWorkmillis = millis();
        printStats();
        break;
      }

      if (press(btn) == 1) {
        paused = true;
        Serial.println("Pausing...");
      }else if (press(btn) == 2)
      {
        Serial.println("Entering Work mode...");
        state = work;
        lastWorkmillis = millis();
        printStats();
      }
      break;



      case work :

      renderLights(50000);

      if ( now - lastWorkmillis >=  WORK_DURATION )
      {
        Serial.println("Entering Rest mode...");
        state = rest;
        lastRestmillis = millis();
        printStats();
        break;
      }

      if (press(btn) == 1) {
        Serial.println("Pausing ...");
        paused = true;
      }else if (press(btn) == 2)
      {
        Serial.println("Entering Rest mode...");
        state = rest;
        lastRestmillis = millis();
        printStats();
      }
      break;
      }
  }
}

void renderLights(unsigned short hue){

 if (now - lastLightMillis > 1000) {

        
        strip.fill(strip.gamma32(strip.ColorHSV(hue, 255, 255)), 0, 7);
        strip.show();
        lastLightMillis = millis();

      } 
}


void printStats(){
  Serial.print("NOW: ");
  Serial.print(now);
  Serial.print(" LW: ");
  Serial.print(lastWorkmillis);
  Serial.print(" LR: ");
  Serial.println(lastRestmillis);
}