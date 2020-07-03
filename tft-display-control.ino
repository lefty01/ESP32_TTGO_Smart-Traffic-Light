



  // tft.setTextColor(TFT_BLUE);    tft.setTextFont(4);
  // tft.print("Float = "); tft.println(fnumber);           // Print floating point number
  // tft.print("Binary = "); tft.println((int)fnumber, BIN); // Print as integer value in binary
  // tft.print("Hexadecimal = "); tft.println((int)fnumber, HEX); // Print as integer number in Hexadecimal

void drawTime(unsigned long time)
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextFont(4);
  tft.setCursor(80, 40);
  tft.println("Time was (sec.):");
  // long min =
  // long sec =
  tft.setCursor(90, 75);
  tft.print((double)time/1000);
}

void drawVersion()
{
  tft.setTextColor(TFT_WHITE);
  tft.setTextFont(4);
  tft.setCursor(120, 110);
  tft.print("Version: "); tft.println(VERSION);
}

// also publishes mqtt ...
void drawModeText(opModes mode)
{
  tft.setTextColor(TFT_BLUE);  // tft.setTextFont(4);
  // tft.print("Float = "); tft.println(fnumber);           // Print floating point number
  // tft.print("Binary = "); tft.println((int)fnumber, BIN); // Print as integer value in binary
  tft.fillRect(60, 18, TFT_WHITE-60, TFT_HEIGHT-50, TFT_BLACK);

  tft.setCursor(60, 18);
  tft.print("MODUS  "); tft.println(mode);
  tft.setCursor(60, 40);

  tft.println(mode2str(mode));

  // if (mode == TRAFFIC_AUTO)   tft.println("AMPEL (Auto)");
  // if (mode == TRAFFIC_MANUAL) tft.println("AMPEL (Manual)");
  // if (mode == MOOD)           tft.println("LAUNE");
  // if (mode == PATTERN)        tft.println("MUSTER");
  // if (mode == PARTY)          tft.println("PARTY");
}
// define config_t or config class to provide as arg (config_t* config)
void drawConfigMenu()
{
  fillSolid(leds, 0, NUM_LEDS, CRGB::Yellow);
  FastLED.show();

  tft.fillScreen(TFT_BLACK);

  tft.setCursor(10, 10, 4); // posx, posy, font size=4
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  //tft.setTextColor(TFT_GREEN);
  //tft.setTextFont(4);
  //tft.setCursor(10, 10);
  tft.println("SMART AMPEL CONFIG");
  tft.setTextFont(2);
  tft.println("");
  tft.println("adjust brightness"); // -> draw "progress bar" as brightness indicator...
  tft.println("config option 2 ...");
  tft.println("config option 3 ...");
}

// tft drawing ...
/*
  0,0            240,0

  128,0          240,128

  modes:
  b'000 - off
  b'100 - red     stop
  b'010 - yellow  attn. going red
  b'001 - green   go
  b'110 - red/yel attn. going green

 */

void drawTrafficLight(int mode, bool clear)
{
  int32_t radius = 15;

  int16_t x1, x2, x3;
  int16_t y1, y2, y3;
  int16_t d = 2;//TFT_HEIGHT / 2 - radius;

  // if (clear) {
  //   tft.fillRect(10, 18, 2*radius + 2*d, 100+2, TFT_BLACK);
  // }
  // 30 30 30 -> r=15 90px | o o o | 10/4=2.5 -> d=2
  
  tft.drawRect(10, 18, 2*radius + 2*d, 100+2, TFT_WHITE);

  x1 = 10 + d + radius;
  y1 = 18 + d + radius;
  x2 = x1;
  y2 = y1 + 2*d + 2*radius;
  x3 = x1;
  y3 = y2 + 2*d + 2*radius;

  tft.drawCircle(x1, y1, radius, TFT_RED);    // top    / red
  tft.drawCircle(x2, y2, radius, TFT_YELLOW); // middle / yellow
  tft.drawCircle(x3, y3, radius, TFT_GREEN);  // bottom / green

  if (mode & 4) {
    tft.fillCircle(x1, y1, radius, clear ? TFT_BLACK : TFT_RED);    // top    / red
    if (clear) tft.drawCircle(x1, y1, radius, TFT_RED);
  }
  if (mode & 2) {
    tft.fillCircle(x2, y2, radius, clear ? TFT_BLACK : TFT_YELLOW); // middle / yellow
    if (clear) tft.drawCircle(x2, y2, radius, TFT_YELLOW);
  }
  if (mode & 1) {
    tft.fillCircle(x3, y3, radius, clear ? TFT_BLACK : TFT_GREEN);  // bottom / green
    if (clear) tft.drawCircle(x3, y3, radius, TFT_GREEN);
  }

}
