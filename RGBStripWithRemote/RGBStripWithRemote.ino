#include "IRremote.h"
#include <FastLED.h>

boolean isRainbow = 0;
boolean isFlash = 0;
boolean isSmooth = 0;
boolean up = 1; 
double pwrlvl = 1;

int r = 0;
int g = 0;
int b = 0;

int beat = 0;
int elGHue = 0;

int tempr = 0;
int tempg = 0;
int tempb = 0;

int receiver = 11;
#define DATA_PIN  5 // WHAT PORT IS LED IN
#define LED_TYPE WS2811
#define COLOR_ORDER GRB

// HOW MANY LEDS DO YOU HAVE???????
#define NUM_LEDS  28
// YOU CAN CHANGE THIS VALUE

CRGB leds[NUM_LEDS];

int BRIGHTNESS = 96;
#define FRAMES_PER_SECOND 120

IRrecv irrecv(receiver);
decode_results results;

void setup() {
  delay(3000);
  Serial.begin(9600);
  irrecv.enableIRIn();

  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);  
  FastLED.setBrightness(BRIGHTNESS);
}

uint8_t gHue = 0;

void loop() {
  delay(100);
  if (irrecv.decode(&results)) {
    delay(10);
    translateIR();
    irrecv.resume();
  }
  if (isSmooth) { smooth(); }
  if (isFlash) { flash(); } 
  
  if (isRainbow == 1) {
    FastLED.show();
    FastLED.delay(1000/FRAMES_PER_SECOND);

    
    EVERY_N_MILLISECONDS( beat ) { gHue+=elGHue; }
    rainbow();
  }
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
}

void translateIR() {
  switch(results.value) {

  case 16734375: case 3691091931: Serial.println("Rainbow"); beat = 10; elGHue = 1; clearAll(); rainbow(); break; // FADE (4, 5)
  case 16750695: case 2538093563: Serial.println("RED"); clearAll(); r = 255; g = 0; b = 0; color(); break; // RED (1, 1)
  case 16767015: case 2259740311: Serial.println("GREEN"); clearAll(); r = 0; g = 255; b = 0; color(); break; // GREEN (2, 1)
  case 16746615: case 2666828831: Serial.println("BLUE"); clearAll(); r = 0; g = 0; b = 255; color(); break; // BLUE (3, 1)
  case 16775175: case 3877748955: Serial.println("OFF"); changeBrit(-2); break; // OFF (1, 3)
  case 16773135: case 900285023: Serial.println("PINK"); clearAll(); r = 255; g = 0; b = 50; color(); break; // PINK (3, 6)
  case 16712445: case 3622325019: Serial.println("Orange"); clearAll(); r=255; g=70; b=0; color(); break; // Orange (1, 4)
  case 1541889663: case 16771095: Serial.println("RedOrange"); clearAll(); r=255; g=50; b=0; color(); break; // RedOrange (1, 3)
  case 16732335: case 713627999: Serial.println("Peach"); clearAll(); r=255; g=100; b=0; color(); break; // Peach (1, 5)
  case 16726215: case 1217346747: Serial.println("Yellow"); clearAll(); r=255; g=255; b=20; color(); break; // Yellow (1, 6)
  case 16730295: case 4084712887: Serial.println("LilGreen"); clearAll(); r=0; g=255; b=30; color(); break; // LilGreen (2, 2)
  case 16724685: case 3998141691: Serial.println("Teal"); clearAll(); r=20; g=255; b=60; color(); break; // Teal (2, 3)
  case 16742535: case 4131161687: Serial.println("DarkTeal"); clearAll(); r=0; g=255; b=100; color(); break; // DarkTeal (2, 4)
  case 16722135: case 324312031: Serial.println("DarkerTeal"); clearAll(); r=0; g=255; b=190; color(); break; // DarkerTeal (2, 6)
  case 16738455: case 3238126971: Serial.println("LilBlue"); clearAll(); r=0; g=100; b=255; color(); break; // LilBlue (3, 2)
  case 1373912347: case 16720095: Serial.println("Purple"); clearAll(); r=255; g=0; b=150; color(); break; // Purple (3, 3)
  case 1153697755: case 16740495: Serial.println("LilPurp"); clearAll(); r=255; g=0; b=100; color(); break; // LilPurp (3, 2)
  case 16754775: case 2747854299: Serial.println("White"); clearAll(); r=255; g=255; b=255; color(); break; // White (4, 2)
  case 16748655: case 3855596927: Serial.println("UP"); changeBrit(1); break; // UP (1, 1)
  case 16758855: case 2721879231: Serial.println("DOWN"); changeBrit(-1); break; // DOWN (2, 1)
  case 16756815: case 4039382595: Serial.println("ON"); changeBrit(2); break; // ON (4, 1)
  case 16711935: case 4198438303: Serial.println("Strobe"); beat = 1; clearAll(); rainbow(); elGHue = 200;  break; // Strobe (4, 4)
  case 16757325: case 2126716663: Serial.println("FLASH"); flash(); break; // FLASH (4, 3)
  case 16724175: case 2534850111: Serial.println("SMOOTH"); smooth(); break; // SMOOTH (4, 6)
  

    
    default: Serial.println(results.value);
  }
  delay(500);
}

void clearAll() {
  r = 0;
  g = 0;
  b = 0;
  color();
  isRainbow = 0;
  isFlash = 0; 
  isSmooth = 0;
}

void rainbow() {
  //clearAll();
  isRainbow = 1;
  fill_rainbow(leds, NUM_LEDS, gHue, 7);
}

void color() {
  tempr = r;
   tempg = g;
   tempb = b;
   colorr();
}
void colorr() {
   
  for (int i=0; i<=NUM_LEDS; i++) {
    
    leds[i] = CRGB(r * pwrlvl, g * pwrlvl, b * pwrlvl);
  }
  Serial.print(r); Serial.print(",");
  Serial.print(g); Serial.print(",");
  Serial.println(b);
  FastLED.show();
  Serial.println(pwrlvl);
  Serial.print(r); Serial.print(",");
  Serial.print(g); Serial.print(",");
  Serial.println(b); 
  delay(10);
}
void changeBrit(int val) {
  if ((val == 1) && (pwrlvl <= 0.9)) { pwrlvl += 0.1; }
  else if ((val == -1) && (pwrlvl >= 0.0)) { pwrlvl -= 0.1; }
  else if (val == 0) { pwrlvl = 0; }
  else if (val == 2) { pwrlvl = 1; }
  else if (val == -2) { pwrlvl = 0; }
  else if (val == 3) { pwrlvl += 0.02; }
  else if (val == -3) { pwrlvl -= 0.02; }
  
  r = tempr;
  b = tempb;
  g = tempg;
  Serial.print(tempr); Serial.print(",");
  Serial.print(tempg); Serial.print(",");
  Serial.println(tempb);
  colorr();
}

void flash() {
  changeBrit(-2);
  delay(300);
  changeBrit(2);
  delay(150);
  isFlash = 1; 
}

void smooth() {
  if ((pwrlvl <= .98) && (up)) { changeBrit(3); }
  else if (pwrlvl >= 0.98) { up = 0; }
  
  if ((pwrlvl >= 0.02) && (!up)) { changeBrit(-3); }
  else if (pwrlvl <= 0.02) { up = 1; }
  isSmooth = 1;
}

