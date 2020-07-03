/*--------------------------------------------------
  some kind of smart traffic light
  has different kind of "modes" eg. show mood via smiley
  or display a clock
  controlable via external push-button, wifi, bluetooth (app)

  The MIT License (MIT)
  (c) 2019,2020 andreas loeffler <al@exitzero.de>

  --------------------------------------------------*/

/* ISSUES/BUGS
   18:11:48.262 -> [E][ssl_client.cpp:33] _handle_error(): [send_ssl_data():284]: (-80) UNKNOWN ERROR CODE (0050)
   mqtt disconnects (wifi still connected)
   -> forgot mqtt loop() call!

   OTA failed: no response from device
   -> firewall:
      $ sudo iptables -I INPUT  -s 192.168.1.56 -j ACCEPT
      $ sudo iptables -I OUTPUT -d 192.168.1.56 -j ACCEPT

 */
#define VERSION "0.4.0"
#define MQTTDEVICEID "ESP_AMPEL"

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
#include <LEDMatrix.h>
FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#include <SPI.h>
#include <TFT_eSPI.h>
#include <Button2.h>
#include <WiFi.h>
//#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Rotary.h>
//#include <DateTime.h>

#include "NotoSansBold15.h"
// The font names are arrays references, thus must NOT be in quotes ""
#define AA_FONT_SMALL NotoSansBold15

#include "ESP32_TTGO_Smart-Traffic-Light.h"
#include "icons.h"

// todo: do we get this from tft.width() and tft.height()?
#define TFT_WIDTH  240
#define TFT_HEIGHT 128

#define ADC_EN          14  //ADC_EN is the ADC detection enable port
//  -> does this interfere with input/pullup???
#define ADC_PIN         34

#define BUTTON_1        33
#define BUTTON_2        25
#define BUTTON_3        26
#define BUTTON_4        27

#define ENC_BUTTON_PUSH 15
#define ENC_BUTTON_A    37
#define ENC_BUTTON_B    17


#define DATA_PIN         2 // to rgb-strip Din

#define NUM_OF_MATRIX     3
#define ROWS_OF_MATRIX    8
#define NUM_LEDS_MATRIX  64
#define NUM_LEDS         NUM_OF_MATRIX * NUM_LEDS_MATRIX
#define CHIPSET          WS2812B
#define COLOR_ORDER      GRB
#define BRIGHTNESS       10
#define MAX_BRIGHTNESS  180 // for now
#define MIN_BRIGHTNESS    2 // for now

#define EVERY_SECOND 1000
#define EVERY_10_SECONDS 10 * EVERY_SECOND
#define EVERY_MINUTE EVERY_SECOND * 60
unsigned long sw_timer_10s;
unsigned long sw_timer_2s;
unsigned long sw_timer_4s;


// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];


/* code above (and hw wiring) assumes three 8x8 rgb matrix panels connected in series
 * which can then be addressed from 0..191
 * another alternative, maybe easier if we mainly control those three as single elements
 * would be to use three output pins and connect each on to seperate pin ...
 *
 * CRGB topLeds[NUM_LEDS_MATRIX];
 * CRGB middleLeds[NUM_LEDS_MATRIX];
 * CRGB bottomLeds[NUM_LEDS_MATRIX];
 * FastLED.addLeds<CHIPSET, DATA_PIN1, COLOR_ORDER>(topLeds,    NUM_LEDS_MATRIX).setCorrection(TypicalSMD5050);
 * FastLED.addLeds<CHIPSET, DATA_PIN2, COLOR_ORDER>(middleLeds, NUM_LEDS_MATRIX).setCorrection(TypicalSMD5050);
 * FastLED.addLeds<CHIPSET, DATA_PIN3, COLOR_ORDER>(bottomLeds, NUM_LEDS_MATRIX).setCorrection(TypicalSMD5050);
 *
 */


WiFiClientSecure net;
PubSubClient mqttClient(net);

TFT_eSPI tft = TFT_eSPI();

// defaults to INPUT_PULLUP, debounce time 50ms
Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);
Button2 btn3(BUTTON_3);
Button2 btn4(BUTTON_4);

Button2 encBtnP(ENC_BUTTON_PUSH);
Rotary rotary = Rotary(ENC_BUTTON_A, ENC_BUTTON_B);


static volatile opModes MODE = TRAFFIC_AUTO;
static volatile opModes PREV_MODE;
static volatile unsigned int TRAFFIC_LIGHT_MODE = TRAFFIC_OFF;
static unsigned long stopwatch;

static volatile short ledBrightness = BRIGHTNESS;
bool toggleRed    = true;
bool toggleYellow = true;
bool toggleGreen  = true;
bool inConfigMode = false;
bool isWifiAvailable = false;
bool isMqttAvailable = false;

const char* mqttSetMode  = "/" MQTTDEVICEID "/setmode";
const char* mqttState    = "/" MQTTDEVICEID "/state";
const char* mqttTLM      = "/" MQTTDEVICEID "/tlm"; // traffic light mode
const char* mqttOpmode   = "/" MQTTDEVICEID "/opmode"; // current mode of operation
const char* mqttButton   = "/" MQTTDEVICEID "/button";

// ICACHE_RAM_ATTR void enc_button_B_cb()
// {
// }




void handleTrafficLight()
{
  DEBUG_PRINT("handleTrafficLight mode=");
  DEBUG_PRINTLN(TRAFFIC_LIGHT_MODE);

  if (isMqttAvailable) {
    char mode[4];
    sprintf(mode, "%d", TRAFFIC_LIGHT_MODE);
    isMqttAvailable = mqttClient.publish(mqttTLM, mode);
  }
  switch(TRAFFIC_LIGHT_MODE) {
  case TRAFFIC_OFF:
  default:
    allLedsOff();
    drawTrafficLight(0);
    drawModeText(MODE);
    break;
  case STOP: // 0
    fillTopRed();
    fillMiddleYellow(false);
    break;
  case GET_READY: // 1
    fillMiddleYellow();
    break;
  case GO:   // 2
    fillTopRed(false);
    fillMiddleYellow(false);
    fillBottomGreen();
    break;
  case ATTN: // 3
    fillBottomGreen(false);
    fillMiddleYellow();
    break;
  }
}


void buttonInit()
{
  //  encBtnA.setDebounceTime(0);
  //  encBtnB.setDebounceTime(0);

  btn1.setPressedHandler([](Button2 & b) {
    DEBUG_PRINTLN("Button 1 SHORT click...");
    b1();
  });

  btn2.setPressedHandler([](Button2 & b) {
    DEBUG_PRINTLN("Button 2 SHORT click...");
    b2();
  });

  btn3.setPressedHandler([](Button2 & b) {
    DEBUG_PRINTLN("Button 3 SHORT click...");
    b3();
  });

  // btn4.setPressedHandler([](Button2 & b) {
  //   DEBUG_PRINTLN("Button 4 SHORT click...start sequence");
  //   startSequence();
  // });

  
  encBtnP.setTapHandler([](Button2 & b) {
    unsigned int time = b.wasPressedFor();
    DEBUG_PRINTLN(time);
    if (time > 3000) { // > 3sec enters config menu
      //DEBUG_PRINTLN("very long click ... toggle config menu");
      if (MODE == CONFIG) {
	DEBUG_PRINTLN("exit config menu");
	MODE = PREV_MODE;
	allLedsOff();
      }
      else {
	DEBUG_PRINTLN("enter config menu");
	PREV_MODE = MODE;
	MODE = static_cast<opModes>(CONFIG);
	drawConfigMenu();
	drawVersion();
	return;
      }
    }
    else if (time > 600) {
      DEBUG_PRINTLN("Button PUSH long click...");
      MODE = static_cast<opModes>(static_cast<int>(MODE) + 1);
      MODE = static_cast<opModes>(static_cast<int>(MODE) % static_cast<int>(_NUM_MODES_));

      TRAFFIC_LIGHT_MODE = TRAFFIC_OFF;
    }
    else {
      DEBUG_PRINTLN("Button PUSH short click...");
      toggleRed    = true;
      toggleYellow = true;
      toggleGreen  = true;
    }

    tft.fillScreen(TFT_BLACK);

    isMqttAvailable = mqttClient.publish(mqttOpmode, mode2str(MODE), true);

    drawTrafficLight(0);
    drawModeText(MODE);

    allLedsOff();

    DEBUG_PRINT("TRAFFIC_LIGHT_MODE: ");
    DEBUG_PRINTLN(TRAFFIC_LIGHT_MODE);

  });
}


void buttonLoop()
{
    btn1.loop();
    btn2.loop();
    btn3.loop();
    btn4.loop();
    encBtnP.loop();
}


void setup()
{
  DEBUG_BEGIN(115200);

  pinMode(ENC_BUTTON_A, INPUT_PULLUP); // does not work right, need ext. pull-up
  pinMode(ENC_BUTTON_B, INPUT_PULLUP);
  //pinMode(ENC_BUTTON_PUSH, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(ENC_BUTTON_A), enc_button_A_cb, FALLING);
  //attachInterrupt(digitalPinToInterrupt(ENC_BUTTON_B), enc_button_B_cb, FALLING);
 
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  isWifiAvailable = setupWifi() ? false : true;
  net.setCACert(server_crt_str);
  net.setCertificate(client_crt_str);
  net.setPrivateKey(client_key_str);

  if (isWifiAvailable) {
    mqttClient.setServer(mqtt_host, mqtt_port);
    mqttClient.setCallback(mqttCallback);
    isMqttAvailable = mqttConnect() ? false : true;
  }
  if (isMqttAvailable) {
    // draw wifi icon ...
    // tft.setSwapBytes(true); // ??
    // tft.pushImage(0, 0,  53, 52, mqtt_bits); does not show correctly!
    // delay(5000);//espDelay(5000);
    tft.fillScreen(TFT_BLACK);
  }

  if (isWifiAvailable) {
    ArduinoOTA.setHostname("smart_ampel1");
    ArduinoOTA.onStart([]() {
			 String type;
			 if (ArduinoOTA.getCommand() == U_FLASH) {
			   type = "sketch";
			 } else { // U_FS
			   type = "filesystem";
			 }
			 // NOTE: if updating FS this would be the place to unmount FS using FS.end()
			 DEBUG_PRINTLN("Start updating " + type);
			 tft.fillScreen(TFT_BLACK);
			 allLedsOff();
		       });
    ArduinoOTA.onEnd([]() {
		       DEBUG_PRINTLN("\nEnd");
		     });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
			    unsigned int percent = progress / (total / 100);
			    //unsigned long pixel = ((unsigned long)(percent * 1.92)) - 1;
			    // fill from bottom row to top ... we have 24 rows
			    unsigned row = ((unsigned long)(percent * 24/100));

			    tft.fillScreen(TFT_BLACK);
			    tft.setTextColor(TFT_BLUE);
			    tft.setTextFont(4);
			    tft.setCursor(20, 40);
			    tft.println("***  OTA PROGRESS % ***");
			    tft.setCursor(30, 50);
			    tft.println(percent);
			    tft.println(row);
			    
			    fillRow(row, CRGB::Blue);
			  });
    ArduinoOTA.onError([](ota_error_t error) {
			 //Serial.printf("Error[%u]: ", error);
			 tft.fillScreen(TFT_BLACK);
			 tft.setTextColor(TFT_WHITE);
			 tft.setTextFont(2);
			 tft.println("***  OTA ERROR !!!");
			 delay(1000);

			 if (error == OTA_AUTH_ERROR) {
			   DEBUG_PRINTLN("Auth Failed");
			   tft.println("***  AUTH !!!");
			 } else if (error == OTA_BEGIN_ERROR) {
			   DEBUG_PRINTLN("Begin Failed");
			   tft.println("***  BEGIN failed !!!");
			 } else if (error == OTA_CONNECT_ERROR) {
			   DEBUG_PRINTLN("Connect Failed");
			   tft.println("***  connect failed !!!");
			 } else if (error == OTA_RECEIVE_ERROR) {
			   DEBUG_PRINTLN("Receive Failed");
			   tft.println("***  receive failed !!!");
			 } else if (error == OTA_END_ERROR) {
			   DEBUG_PRINTLN("End Failed");
			   tft.println("***  END failed !!!");
			 }
			 delay(5000);
		       });
    ArduinoOTA.begin();
  }


  /*
    ADC_EN is the ADC detection enable port
    If the USB port is used for power supply, it is turned on by default.
    If it is powered by battery, it needs to be set to high level
  */
  // could use light level to auto adjust led brightness
  // pinMode(ADC_EN, OUTPUT);
  // digitalWrite(ADC_EN, HIGH);
  // uint16_t v = analogRead(ADC_PIN); // could read ambient light via ldr?
				    // ... or use i2c light sensor

  // TODO: check if adc enable cause the issue with pullup???
  // setup globals
  //reqCount = 0;

  // prepare rgb data pin
  pinMode(DATA_PIN, OUTPUT);
  digitalWrite(DATA_PIN, 0);


  // AP mode
  // WiFi.mode(WIFI_AP);
  // WiFi.softAP(ssid, password);
  // //WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  // //server.begin();

  // // web server inline handler ...
  // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   request->send(200, "text/plain", "Hello, world");
  // });

  // server.on("/control", HTTP_GET, controlPage);

  // server.on("/", HTTP_POST, handlePost);

  // server.onNotFound(notFound);

  // server.begin();
  // // --- end server req. handler

  tft.setCursor(0, 0);            // Set cursor at top left of screen
  tft.loadFont(AA_FONT_SMALL);    // Must load the font first
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextWrap(true); // Wrap on width
  drawVersion();

  FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).
    setCorrection(TypicalSMD5050);
  FastLED.setBrightness(BRIGHTNESS);
  
  ledTest();

  tft.setCursor(5, 0);
  tft.println("ESP TTGO Smart-Ampel");

  drawTrafficLight(0);
  drawModeText(MODE);

  isMqttAvailable = mqttClient.publish(mqttOpmode, mode2str(MODE), true);
  
  buttonInit();
}

void loop()
{
  buttonLoop(); // fixme: in this sample we have long delay on led_test and led start
  unsigned char result = rotary.process();
  if (isWifiAvailable) ArduinoOTA.handle();

  if (isWifiAvailable && (false == (isMqttAvailable = mqttClient.loop()))) {
    DEBUG_PRINTLN("mqtt connection lost ... try re-connect");
    isMqttAvailable = mqttConnect(false) ? false : true;
  }


  if (CONFIG == MODE) { // = static_cast<opModes>(CONFIG);
    // all leds to yellow to adjust brightness ...
    
    if (result == DIR_CW) {
      if (ledBrightness < MAX_BRIGHTNESS)
	ledBrightness++;

      DEBUG_PRINT("set brightness: ");
      DEBUG_PRINTLN(ledBrightness);
      FastLED.setBrightness(ledBrightness);
      FastLED.show();
    } else if (result == DIR_CCW) {
      if (ledBrightness > MIN_BRIGHTNESS)
	ledBrightness--;

      DEBUG_PRINT("set brightness: ");
      DEBUG_PRINTLN(ledBrightness);
      FastLED.setBrightness(ledBrightness);
      FastLED.show();
    }
    
  }

  // cycle through "regular" traffic light sequence
  if (TRAFFIC_AUTO == MODE) {
    if (TRAFFIC_LIGHT_MODE == GET_READY) {
      if (millis() > (sw_timer_2s + EVERY_SECOND * 2)) {
	sw_timer_2s = millis();
	TRAFFIC_LIGHT_MODE += 1;
	TRAFFIC_LIGHT_MODE %= _NUM_TRAFFIC_LIGHT_MODES_;
	handleTrafficLight();
      }
    }
    else
    if (TRAFFIC_LIGHT_MODE == ATTN) {
      if (millis() > (sw_timer_4s + EVERY_SECOND * 4)) {
	sw_timer_4s = millis();
	TRAFFIC_LIGHT_MODE += 1;
	TRAFFIC_LIGHT_MODE %= _NUM_TRAFFIC_LIGHT_MODES_;
	handleTrafficLight();
      }
    }
    else
    if (millis() > (sw_timer_10s + EVERY_10_SECONDS)) {
      sw_timer_10s = millis();
      sw_timer_2s = millis();
      sw_timer_4s = millis();
      TRAFFIC_LIGHT_MODE += 1;
      TRAFFIC_LIGHT_MODE %= _NUM_TRAFFIC_LIGHT_MODES_;
      handleTrafficLight();
    }
  }

}
