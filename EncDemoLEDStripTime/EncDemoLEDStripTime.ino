#include <FastLED.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <Wire.h>
//#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>


Adafruit_SSD1306 display(128, 64, &Wire, -1);
const char* ssid = "BlastOff";
const char* password = "AA$JJ@KK";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

String formattedDate;
String dayStamp;
String timeStamp;
String hour;

WiFiClient espClient;

FASTLED_USING_NAMESPACE

// FastLED "100-lines-of-code" demo reel, showing just a few
// of the kinds of animation patterns you can quickly and easily
// compose using FastLED.
//
// This example also shows one easy way to define multiple
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#define DATA_PIN    14
#define NUM_LEDS    150
CRGB leds[NUM_LEDS];

#define FRAMES_PER_SECOND  120

int brightness = 96;
int oldo = 96;
int colorVal;
int encoder0PinA = 12;
int encoder0PinB = 13;
int encoder0Pos = 0;
int encoder0PinALast = LOW;
int swi = 0;
int n = LOW;

int redVal;
int greenVal;
int blueVal;

int pushed = 0;

unsigned long currentTime = millis();
unsigned long previous = 0;
unsigned long forTime = 0;

int turnedOn = false; 
int morseLED = 50; 
int morseUnit = 500; 
unsigned long morseTime = 0; 
String morseCode = ""; 
int morseSoFar = 0; 
unsigned long holdFor = morseUnit; 
boolean morseUpdated = false; 

void setup() {

  delay(3000); // 3 second delay for recovery
  Serial.begin(115200);

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

  pinMode(encoder0PinA, INPUT);
  pinMode(encoder0PinB, INPUT);
  pinMode(swi, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  timeClient.begin();
  timeClient.setTimeOffset(-25200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.display();
  delay(2000);

  display.clearDisplay();
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void loop()
{
  brightness = map(analogRead(A0), 0, 1023, 0, 100);
  if (oldo != brightness) {
    FastLED.setBrightness(brightness);
    oldo = brightness;
    Serial.print("UPDATED BRIGHTNESS: ");
    Serial.println(brightness);
  }

  // Call the current pattern function once, updating the 'leds' array
  if (digitalRead(swi) == LOW) {
    pushed++;
    Serial.println(pushed);
    delay(200);
  }

  if (pushed % 3 == 0) {
    gPatterns[gCurrentPatternNumber]();
    // send the 'leds' array out to the actual LED strip
//    showMorse(); Idk let's not do this rn
    FastLED.show();
    // insert a delay to keep the framerate modest
    FastLED.delay(1000 / FRAMES_PER_SECOND);
    // do some periodic updates
    EVERY_N_MILLISECONDS( 20 ) {
      gHue++;  // slowly cycle the "base color" through the rainbow
    }
    EVERY_N_SECONDS( 10 ) {
      nextPattern();  // change patterns periodically
    }
  } else if (pushed % 3 == 1 || pushed % 3 == 2) {
    encoderMode();
  }
  currentTime = millis();
  if (currentTime - forTime > 1000) {
    displayTime();
    forTime = currentTime;
  }
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS - 1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16( i + 7, 0, NUM_LEDS - 1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

void encoderMode() {
  n = digitalRead(encoder0PinA);
  if ((encoder0PinALast == LOW) && (n == HIGH)) {
    if (digitalRead(encoder0PinB) == LOW) {
      encoder0Pos -= 8;
    } else if (digitalRead(encoder0PinB) == HIGH) {
      encoder0Pos += 8;
    }
    Serial.println(encoder0Pos);

    if (encoder0Pos < 0) {
      encoder0Pos = 1535;
    } else if (encoder0Pos > 1535) {
      encoder0Pos = 0;
    }
  }
  encoder0PinALast = n;

  if (encoder0Pos <= 255) {
    colorVal = encoder0Pos;
    redVal = 255;
    greenVal = colorVal;
    blueVal = 0;
  } else if (encoder0Pos <= 511) {
    colorVal = encoder0Pos - 256;
    redVal = 255 - colorVal;
    greenVal = 255;
    blueVal = 0;
  } else if (encoder0Pos <= 767) {
    colorVal = encoder0Pos - 512;
    redVal = 0;
    greenVal = 255;
    blueVal = colorVal;
  } else if (encoder0Pos <= 1023) {
    colorVal = encoder0Pos - 768;
    redVal = 0;
    greenVal = 255 - colorVal;
    blueVal = 255;
  } else if (encoder0Pos <= 1279) {
    colorVal = encoder0Pos - 1024;
    redVal = colorVal;
    greenVal = 0;
    blueVal = 255;
  } else {
    colorVal = encoder0Pos - 1280;
    redVal = 255;
    greenVal = 0;
    blueVal = 255 - colorVal;
  }

  currentTime = millis();

  if (pushed % 3 == 2) {
    redVal = 0;
    greenVal = 0;
    blueVal = 0;
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(redVal * .2, greenVal * .2, blueVal * .2);
  }
//  showMorse(); 
  if (currentTime - previous >= 10) {
    FastLED.show();
    previous = currentTime;
  }
}

void displayTime() {
  getTime();
  Serial.println("DISPLAYING");
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 0);
  //  display.setFont(ArialMT_PLAIN_24);
  //  display.drawString(0, 10, timeStamp);
  display.println(timeStamp);
  display.display();
}

void getTime() {
  while (!timeClient.update()) {
    Serial.print("hmmmOOO");
    timeClient.forceUpdate();
  }
  // The formattedDate comes with following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  Serial.println(formattedDate);

  // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  Serial.print("DATE: ");
  Serial.println(dayStamp);
  //Extract time


  timeStamp = formattedDate.substring(splitT + 3, formattedDate.length() - 1);
  hour = formattedDate.substring(splitT + 1, formattedDate.length() - 7);
  if (hour.toInt() >= 13 && hour.toInt() <= 24) {
    hour = hour.toInt() - 12;
  }
  else if (hour.toInt() == 0) {
    hour = hour.toInt() + 12;
  }
  timeStamp = hour + timeStamp;

  Serial.print("HOUR: ");
  Serial.println(timeStamp);

  if (turnedOn == false && formattedDate.indexOf("7:50:0") >= 0) {
    pushed = 0;
    turnedOn = true; 
  } else if (formattedDate.indexOf("10:00:1") >= 0) {
    turnedOn = false; ; 
  }
  //  displayTime();
  //delay(1000);
}

//void showMorse() {
//  if (formattedDate.indexOf("22:59:00") >= 0 && morseUpdated == false) {
//    //morseLED = (int)random(0,NUM_LEDS); 
//    morseCode = morseCon(); 
//
//    Serial.println("LED # = " + morseLED); 
//    Serial.println("Phrase = " + morseCode); 
//    morseUpdated = true; 
//  }
//
//  if (millis() - morseTime >= holdFor) {
//    Serial.println("morse update"); 
//    morseTime = millis(); 
//    if (morseCode.charAt(morseSoFar) == '.') {
//      leds[morseLED] = CRGB(100, 0, 0);
//      holdFor = morseUnit; 
//    } else if (morseCode.charAt(morseSoFar) == '-') {
//      leds[morseLED] = CRGB(100, 0, 0); 
//      holdFor = morseUnit * 3; 
//    } else if (morseCode.charAt(morseSoFar) == ' ') {
//      leds[morseLED] = CRGB(0, 0, 0); 
//      if (morseCode.substring(morseSoFar, morseSoFar+3).indexOf("   ") >= 0) {
//        holdFor = morseUnit * 7; 
//        morseSoFar += 3; 
//      } else {
//        holdFor = morseUnit * 3; 
//      }
//    }
//
//    if (morseSoFar >= morseCode.length()) {
//      morseSoFar = 0; 
//      holdFor = morseUnit * 10; 
//    }
//  }
//}
