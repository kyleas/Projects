/*    Home room automation 
 *    Goals:
 *      -Display time on an OLED 
 *      -Have 3 modes of LEDs (fun patterns, solid, off)
 *      -Connect to Google Assistant to change modes 
 *      -Use encoder to change specific color on "solid" mode
 *      -Use POT to change brightness 
 *      -Use encoder button to change mode
 */

#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h" 
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FastLED.h>


 /************************* WiFi Access Point *********************************/

#define WLAN_SSID       "-------"
#define WLAN_PASS       "------"
 
/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME     "kylenotshoe"
#define AIO_KEY          "aio_zLpU30VCcL24NjOM60mEADBCP55Q" 

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Subscribe LED_Control = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/LED_Control");

// NTP 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// OLED
Adafruit_SSD1306 display(128, 64, &Wire, -1);

/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");
Adafruit_MQTT_Subscribe slider = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/slider");

/**************************** Variables *************************************/

//Time
String reformattedTime = "";

//Leds
#define FRAMES_PER_SECOND 120
#define DATA_PIN           14
#define NUM_LEDS          150
CRGB leds[NUM_LEDS];
int color[3] = {0, 0, 0};
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current


int brightness = 96;
int state = 0;  

//Encoder
#define encoder0PinA      12
#define encoder0PinB      13
#define switchPin         0
int n = 0;
int encoder0PinALast = 0;
int encoder0Pos = 0;

//Time
unsigned long currentTime = millis(); 
unsigned long previous = 0; 
unsigned long displayDelay = 1000;

/****************************** Setup ***************************************/
void setup() {
  Serial.begin(115200); 

    // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: "); Serial.println(WiFi.localIP());

  timeClient.begin();
  timeClient.setTimeOffset(-25200);    

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

  pinMode(encoder0PinA, INPUT);
  pinMode(encoder0PinB, INPUT);
  pinMode(switchPin, INPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed, will not work"));
  }

  display.display();
  delay(1000);
  display.clearDisplay();

  mqtt.subscribe(&LED_Control);

}

uint8_t gHue = 0; // rotating "base color" used by many of the patterns
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

/****************************** Loop **************************************/
void loop() {
//  fetchTime();
//  Serial.println(reformattedTime);
//  delay(2000);

  brightness = map(analogRead(A0), 0, 1023, 0, 25);
  brightness *= 4;
  FastLED.setBrightness(brightness);

  buttons();

  currentTime = millis();
  if (currentTime - displayDelay > 1000) {
    displayTime();
    displayDelay = currentTime;
  }

  MQTT();
  Adafruit_MQTT_Subscribe *subscription;
    if ((subscription = mqtt.readSubscription(1))) {
      if (subscription == &LED_Control) {
        Serial.print(F("Got: ")); 
        Serial.println((char *)LED_Control.lastread);
        if (!strcmp((char*) LED_Control.lastread, "ON")) {
          state = 3;
        } else if (!strcmp((char*) LED_Control.lastread, "SOLID")) {
//          color[1] = 255;
          encoder0Pos = 512;
          state = 1;
        } else {
          state = 2;
        }
      }
    } 
  
}

void fetchTime() {
  while(!timeClient.update()) {
    Serial.print("Reconnecting to NTP... "); 
    timeClient.forceUpdate();
  }

  String currentTime = timeClient.getFormattedTime();
  String secondTime = currentTime.substring(2, currentTime.length());
  String currentHour = currentTime.substring(0, 2); 

  Serial.print("Current time: "); Serial.println(currentTime);
  
  if (currentHour.toInt() >= 13) {
    currentHour = currentHour.toInt() - 12; 
  }

  reformattedTime = currentHour + secondTime; 
}

void buttons() {
  if (digitalRead(switchPin) == LOW) {
    state++;
    Serial.print("Button pushed, value now: "); Serial.println(state);
    delay(200);
  }

  if (state % 3 == 0) {
    gPatterns[gCurrentPatternNumber]();
    FastLED.show();
    FastLED.delay(1000 / FRAMES_PER_SECOND);
    EVERY_N_MILLISECONDS(20) {
      gHue++;
    }
    EVERY_N_SECONDS(10) {
      nextPattern();
    }
  } else if (state % 3 == 1 || state % 3 == 2) {
    encoderMode();
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
    Serial.print("Encoder position: "); Serial.println(encoder0Pos);

    if (encoder0Pos < 0) {
      encoder0Pos = 1535;
    } else if (encoder0Pos > 1535) {
      encoder0Pos = 0;
    }
  }
  encoder0PinALast = n;

  if (encoder0Pos <= 255) {
    color[0] = 255;
    color[1] = encoder0Pos;
    color[2] = 0;
  } else if (encoder0Pos <= 511) {
    color[0] = 511 - encoder0Pos;
    color[1] = 255;
    color[2] = 0;
  } else if (encoder0Pos <= 767) {
    color[0] = 0;
    color[1] = 255;
    color[2] = encoder0Pos - 512;
  } else if (encoder0Pos <= 1023) {
    color[0] = 0; 
    color[1] = 1023 - encoder0Pos;
    color[2] = 255;
  } else if (encoder0Pos <= 1279) {
    color[0] = encoder0Pos - 1024;
    color[1] = 0;
    color[2] = 255;
  } else {
    color[0] = 255;
    color[1] = 0; 
    color[2] = 1535 - encoder0Pos;
  }

  currentTime = millis();

  if (state % 3 == 2) {
    color[0] = 0;
    color[1] = 0; 
    color[2] = 0;
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(color[0] * .2, color[1] * .2, color[2] * .2);
  }

  if (currentTime - previous >= 10) {
    FastLED.show();
    previous = currentTime;
  }
}

void displayTime() {
  fetchTime();
  Serial.println("Updating display... "); 
  display.clearDisplay();
  display.setTextSize(2.5);
  display.setTextColor(WHITE);
  display.setCursor(10,0);
  display.println(reformattedTime);
  if (state % 3 == 2) { display.clearDisplay(); }  
  display.display();
}

void MQTT() {
  int8_t ret; 
  if (mqtt.connected()) {
    return; 
  }
  Serial.print("Connecting to MQTT... "); 
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("retrying MQTT connection in 5 seconds...."); 
    mqtt.disconnect();
    delay(5000);
    retries--;
    if (retries == 0) {
      while(1); 
    }
  }
  Serial.println("MQTT Connected!");  
}
