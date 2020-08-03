#ifndef _ESP32_TTGO_Smart_Traffic_Light_H_
#define _ESP32_TTGO_Smart_Traffic_Light_H_


#define DEBUG 1
#define DEBUG_MQTT 1
#include "debug_print.h"
#include "wifi_mqtt_creds.h"

// "cycle through" modes FIXME:  make proper state-machine implementation...
enum opModes: byte
{
  TRAFFIC_AUTO = 0  // cycle through trafficLightModes at fixed intervalls
    ,TRAFFIC_MANUAL // cycle through trafficLightModes at manual button press
    ,TRAFFIC        // manual toggle red/yellow/green on/off via buttons 1-3
    ,MOOD           // toggle simileys via buttons 1-2
    ,STARTAMPEL     // f1-like start signel, button1: start, button2: stop -> show time
    ,DISCO
    ,CLOCK
    ,APP_CONFIG
    ,_NUM_MODES_
};
// special modes
const byte MODE_SELECT = 0x10;
//const byte APP_CONFIG  = 0x20;


// german menu ;)
const char* mode2str(opModes mode)
{
  if (mode == TRAFFIC)        return "AMPEL Farben";
  if (mode == TRAFFIC_AUTO)   return "AMPEL (Auto)";
  if (mode == TRAFFIC_MANUAL) return "AMPEL (Manual)";
  if (mode == MOOD)           return "LAUNE";
  if (mode == STARTAMPEL)     return "START AMPEL";
  if (mode == DISCO)          return "DISCO";
  if (mode == CLOCK)          return "UHR (binary)";
  if (mode == APP_CONFIG)     return "CONFIG Menu";
  return "INVALID";
}


const byte TRAFFIC_OFF = 255;
enum trafficLightModes: byte
{
  //  OFF
     STOP                  // red
    ,GET_READY             // red/yellow
    ,GO                    // green
    ,ATTN                  // yellow
    ,_NUM_TRAFFIC_LIGHT_MODES_
};

enum moodType: byte
{
  HAPPY
    ,NEUTRAL
    ,SAD
    ,NO_MOOD
};

#define MATRIX_POS_TOP     1
#define MATRIX_POS_MIDDLE  2
#define MATRIX_POS_BOTTOM  4
#define MATRIX_POS_ALL     (MATRIX_POS_TOP|MATRIX_POS_MIDDLE|MATRIX_POS_BOTTOM)

// tft-display-control prototypes
void drawTrafficLight(int mode, bool clear=true);
void drawModeSelectMenu();
void drawConfigMenu(bool update=false);

// led-matrix-control prototypes
void fillTopMatrix(const struct CRGB& color, bool ledShow=true);
void fillMiddleMatrix(const struct CRGB& color, bool ledShow=true);
void fillBottomMatrix(const struct CRGB& color, bool ledShow=true);
void fillTopRed(bool on=true);
void fillMiddleYellow(bool on=true);
void fillBottomGreen(bool on=true);  
int mqttConnect(bool updateDisplay=true);

#endif
