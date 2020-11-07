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
#define VERSION "0.8.4"
#define MQTTDEVICEID "ESP_AMPEL"
#define OTA_HOSTNAME "smart_ampel1"

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
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Rotary.h>


//#include "NotoSansBold15.h"
// The font names are arrays references, thus must NOT be in quotes ""
//#define AA_FONT_SMALL NotoSansBold15

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
#define BRIGHTNESS        8
#define MAX_BRIGHTNESS  180 // for now
#define MIN_BRIGHTNESS    2 // for now

#define EVERY_SECOND 1000
#define EVERY_10_SECONDS 10 * EVERY_SECOND
#define EVERY_MINUTE EVERY_SECOND * 60
unsigned long sw_timer_10s;
unsigned long sw_timer_2s;
unsigned long sw_timer_4s;
unsigned long sw_timer_10ms;
unsigned long sw_timer_clock;

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];
// fastled palette/blend
CRGBPalette16 currentPalette;
TBlendType    currentBlending;

const long gmtOffset = 7200; // sec
// Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {Last, Sun, Mar, 2, 120};     // Central European Summer Time
TimeChangeRule CET = {Last, Sun, Oct, 3, 60};       // Central European Standard Time


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
WiFiUDP ntpUDP;
PubSubClient mqttClient(net);
NTPClient timeClient(ntpUDP, gmtOffset, CEST, CET);
//setTimeZone(CEST, CET);
// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
//NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

TFT_eSPI tft = TFT_eSPI();

// defaults to INPUT_PULLUP, debounce time 50ms
Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);
Button2 btn3(BUTTON_3);

Button2 encBtnP(ENC_BUTTON_PUSH);
Rotary rotary = Rotary(ENC_BUTTON_A, ENC_BUTTON_B);


static volatile opModes opMode = CLOCK; // start mode
static volatile opModes prevMode;
static volatile opModes selectMode; // "temp" mode that will be assigned in select menu
static volatile unsigned int TRAFFIC_LIGHT_MODE = TRAFFIC_OFF;

static volatile int prev_hour;
static volatile int prev_min;
static volatile int prev_sec;
struct DateTime cur_dateTime;
struct DateTime prev_dateTime;
bool showTimeNow = true;
static unsigned long stopwatch;

static volatile short ledBrightness = BRIGHTNESS;
bool toggleRed    = true;
bool toggleYellow = true;
bool toggleGreen  = true;
bool inConfigMode = false;
bool isWifiAvailable = false;
bool isMqttAvailable = false;

const char* mqttSetMode    = MQTTDEVICEID "/setmode";
const char* mqttSetConfig  = MQTTDEVICEID "/setconfig";
const char* mqttState      = MQTTDEVICEID "/state";
const char* mqttTLM        = MQTTDEVICEID "/tlm"; // traffic light mode
const char* mqttOpmode     = MQTTDEVICEID "/opmode"; // current mode of operation
const char* mqttBrightness = MQTTDEVICEID "/brightness";
const char* mqttButton     = MQTTDEVICEID "/button";
const char* mqttClock      = MQTTDEVICEID "/clock";


// String TOPIC_STATE   = "%CHIP_ID%/state";
// ...
// void setupMqttTopic(const String &id)
// {
//   TOPIC_STATE.replace("%CHIP_ID%", id);
//   TOPIC_INFO.replace("%CHIP_ID%", id);
//   TOPIC_VERSION.replace("%CHIP_ID%", id);
// }


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
    drawModeText(opMode);
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

void FillLEDsFromPaletteColors(uint8_t colorIndex)
{
  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = ColorFromPalette(currentPalette, colorIndex,
			       ledBrightness, currentBlending);
    colorIndex += 3;
  }
}


void setup()
{
  DEBUG_BEGIN(115200);

  pinMode(ENC_BUTTON_A, INPUT_PULLUP); // does not work right, need ext. pull-up (here 10k)
  pinMode(ENC_BUTTON_B, INPUT_PULLUP);
  //pinMode(ENC_BUTTON_PUSH, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(ENC_BUTTON_A), enc_button_A_cb, FALLING);
  //attachInterrupt(digitalPinToInterrupt(ENC_BUTTON_B), enc_button_B_cb, FALLING);

  tft.begin();
  tft.setRotation(3);

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
    ArduinoOTA.setHostname(OTA_HOSTNAME);
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


  tft.setCursor(0, 0);            // Set cursor at top left of screen
  //tft.loadFont(AA_FONT_SMALL);    // Must load the font first
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextWrap(true); // Wrap on width
  drawVersion();

  FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).
    setCorrection(TypicalSMD5050);
  FastLED.setBrightness(BRIGHTNESS);
  currentPalette = PartyColors_p; //RainbowColors_p;
  currentBlending = LINEARBLEND;

  ledTest();

  tft.setCursor(5, 0);
  tft.println("ESP TTGO Smart-Ampel");

  drawTrafficLight(0);
  drawModeText(opMode);

  isMqttAvailable = mqttClient.publish(mqttOpmode, mode2str(opMode), true);

  buttonInit();

  timeClient.begin();
  timeClient.update();

  delay(100);
}

void loop()
{
  buttonLoop(); // fixme: in this sample we have long delay on led_test and led start
  unsigned char result = rotary.process();
  timeClient.update();

  if (isWifiAvailable) ArduinoOTA.handle();

  if (isWifiAvailable && (false == (isMqttAvailable = mqttClient.loop()))) {
    DEBUG_PRINTLN("mqtt connection lost ... try re-connect");
    isMqttAvailable = mqttConnect(false) ? false : true;
  }

  if (DISCO == opMode) {
    if ((millis() - sw_timer_10ms) > 500) {
      static uint8_t startIndex = 0;
      startIndex = startIndex + 1; /* motion speed */
      FillLEDsFromPaletteColors(startIndex);
      FastLED.show();
     }
  }

  if (CLOCK == opMode) {
    if ((millis() - sw_timer_clock) > EVERY_SECOND) {
      sw_timer_clock = millis();
      cur_dateTime = timeClient.getDateTime();

      drawBinClockSec(cur_dateTime.dt_seconds);

      if (cur_dateTime.dt_minutes != prev_dateTime.dt_minutes || showTimeNow) {
	isMqttAvailable = mqttClient.publish(mqttClock, timeClient.getFormattedDateTime("%d.%m. %H:%M:%S").c_str());
	drawBinClockHourMin(cur_dateTime);
      }

      if (cur_dateTime.dt_date != prev_dateTime.dt_date || showTimeNow) {
	drawBinClockDate(cur_dateTime.dt_date, cur_dateTime.dt_month);
      }

      prev_dateTime = cur_dateTime;
      showTimeNow = false;
     }
  }

  if (MODE_SELECT == opMode) {
    if (result == DIR_CW) {
      selectMode = static_cast<opModes>(static_cast<int>(selectMode) + 1);
      selectMode = static_cast<opModes>(static_cast<int>(selectMode) %
					static_cast<int>(_NUM_MODES_));
      DEBUG_PRINT("selectMode: ");
      DEBUG_PRINTLN(selectMode);
      drawModeSelectMenu();
    } else if (result == DIR_CCW) {
      if (selectMode == TRAFFIC_AUTO)
	selectMode = static_cast<opModes>(static_cast<int>(_NUM_MODES_) - 1);
      else
	selectMode = static_cast<opModes>(static_cast<int>(selectMode) - 1);

      DEBUG_PRINT("SELECT MODE: ");
      DEBUG_PRINTLN(selectMode);
      drawModeSelectMenu();
    }
  }

  if (APP_CONFIG == opMode) {
    TRAFFIC_LIGHT_MODE = TRAFFIC_OFF;
    // all leds to yellow to adjust brightness ...
    fillSolid(leds, 0, NUM_LEDS, CRGB::Yellow);
    FastLED.show();

    // TODO select items from app config menu ...
    if (result == DIR_CW) {

      // if APP_CONFIG_DISCO_MODE
      // select fast led palette eg. rainbow or somehting ...

      // if APP_CONFIG_DISCO_SPEED
      // control "speed" (delay) when filling colors from palette ...

      // if APP_CONFIG_BRIGHTNESS ... current default while the only option
      if (ledBrightness < MAX_BRIGHTNESS)
	ledBrightness++;

      DEBUG_PRINT("set brightness: ");
      DEBUG_PRINTLN(ledBrightness);
      if (isMqttAvailable) {
        char b[4];
        sprintf(b, "%d", ledBrightness);
        isMqttAvailable = mqttClient.publish(mqttBrightness, b);
      }

      drawConfigMenu(true);

      FastLED.setBrightness(ledBrightness);
      FastLED.show();
    } else if (result == DIR_CCW) {
      if (ledBrightness > MIN_BRIGHTNESS)
        ledBrightness--;

      DEBUG_PRINT("set brightness: ");
      DEBUG_PRINTLN(ledBrightness);
      if (isMqttAvailable) {
        char b[4];
        sprintf(b, "%d", ledBrightness);
        isMqttAvailable = mqttClient.publish(mqttBrightness, b);
      }

      drawConfigMenu(true);

      FastLED.setBrightness(ledBrightness);
      FastLED.show();
    }
  } // APP_CONFIG mode

  // cycle through "regular" traffic light sequence
  if (TRAFFIC_AUTO == opMode) {
    if (TRAFFIC_LIGHT_MODE == GET_READY) {
      if ((millis() - sw_timer_2s) > EVERY_SECOND * 2) {
	sw_timer_2s = millis();
	TRAFFIC_LIGHT_MODE += 1;
	TRAFFIC_LIGHT_MODE %= _NUM_TRAFFIC_LIGHT_MODES_;
	handleTrafficLight();
      }
    }
    else if (TRAFFIC_LIGHT_MODE == ATTN) {
      if ((millis() - sw_timer_4s) > EVERY_SECOND * 4) {
	sw_timer_4s = millis();
	TRAFFIC_LIGHT_MODE += 1;
	TRAFFIC_LIGHT_MODE %= _NUM_TRAFFIC_LIGHT_MODES_;
	handleTrafficLight();
      }
    }
    else if ((millis() - sw_timer_10s) > EVERY_10_SECONDS * 2) { // every 20 seconds
      sw_timer_10s = millis();
      sw_timer_2s = millis();
      sw_timer_4s = millis();
      TRAFFIC_LIGHT_MODE += 1;
      TRAFFIC_LIGHT_MODE %= _NUM_TRAFFIC_LIGHT_MODES_;
      handleTrafficLight();
    }
  }
}
