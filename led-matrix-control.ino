
void fillRow(unsigned row, const struct CRGB& color)
{
  if (row > (NUM_OF_MATRIX * ROWS_OF_MATRIX)) return;

  unsigned row0[ROWS_OF_MATRIX]  = { 191, 183, 175, 167, 159, 151, 143, 135 }; // note: check type if more leds used!
  unsigned row8[ROWS_OF_MATRIX]  = { 127, 119, 111, 103,  95,  87,  79,  71 };
  unsigned row16[ROWS_OF_MATRIX] = {  63,  55,  47,  39,  31,  23,  15,   7 };

  // row 1: row8[0] - 1 -> - row index
  // row X values are row0[n]-X
  for (unsigned n; n<ROWS_OF_MATRIX; ++n) {
    if (row > 15) // 2*ROWS_OF_MATRIX-1
      leds[row16[n] - (row-16)] = color;
    else if (row > 7) // 1*ROWS_OF_MATRIX-1
      leds[row8[n] - (row-8)] = color;
    else
      leds[row0[n] - row] = color;
  }
  
  FastLED.show();
}

void fillSolid(struct CRGB * leds, int start, int numToFill, const struct CRGB& color)
{
  if (numToFill > NUM_LEDS) numToFill = NUM_LEDS;
  for (int i = start; i < (start + numToFill); ++i) {
    leds[i] = color;
  }
}
//FIXME: const/enum for mode, here red=4, yello=2, green=1
// refactor/rename to fillTop(int pattern), fillMiddle(int pattern)..
// use some pattern arg eg. RED, SAD_SMILE, RAINBOW, etc
//FIXME: maybe do not call the drawTrafficLight function from here ... move to calling f
void fillTopMatrix(const struct CRGB& color) {
  fillSolid(leds, 0, 64, color);
  FastLED.show();
}
void fillMiddleMatrix(const struct CRGB& color) {
  fillSolid(leds, 64, 64, color);
  FastLED.show();
}
void fillBottomMatrix(const struct CRGB& color) {
  fillSolid(leds, 128, 64, color);
  FastLED.show();
}
  
void fillTopRed(bool on)
{
  fillTopMatrix(on ? CRGB::Red : CRGB::Black);
  drawTrafficLight(4, !on);
}
void fillMiddleYellow(bool on)
{
  fillMiddleMatrix(on ? CRGB::Yellow : CRGB::Black);
  drawTrafficLight(2, !on);
}
void fillBottomGreen(bool on)
{
  fillBottomMatrix(on ? CRGB::Green : CRGB::Black);
  drawTrafficLight(1, !on);
}
void allLedsOff()
{
  fillSolid(leds, 0, NUM_LEDS, CRGB::Black);
  FastLED.show();
  //FastLED.clear();
}


void ledTest()
{
  allLedsOff();

  for (unsigned i=0; i<8; ++i) {
    unsigned k = 8 * i; //
    unsigned k_ = 8 * (i-1);
    for (unsigned j=k; j<(k+8); ++j) {
      leds[j] = CRGB::Blue;
      if (j == k) leds[k_ + 128 + 7] = CRGB::Black;
      if (j > 0) leds[j-1] = CRGB::Black; // turn off previous pixel
      FastLED.show();
      delay(15);
    }
    for (unsigned j=k+64; j<(k+64+8); ++j) {
      leds[j] = CRGB::Blue;
      if (j == k+64) leds[j-57] = CRGB::Black;
      if (j > 0) leds[j-1] = CRGB::Black; // turn off previous pixel
      FastLED.show();
      delay(15);
    }
    for (unsigned j=k+128; j<(k+128+8); ++j) {
      leds[j] = CRGB::Blue;
      if (j == k+128) leds[j-57] = CRGB::Black;
      if (j > 0) leds[j-1] = CRGB::Black; // turn off previous pixel
      FastLED.show();
      delay(15);
    }
  }
  delay(30);
  allLedsOff();
}
/*
  mood 0: smile
  mood 1: neutral
  mood 2: sad
  mood 3: turn off panel at pos

  pos  7: all
  pos  1: top
  pos  2: middle
  pos  4: bottom
*/
void drawSmiley(int mood, int pos)
{
  DEBUG_PRINT("drawSmiley: mood="); DEBUG_PRINT(mood);
  DEBUG_PRINT(" pos="); DEBUG_PRINTLN(pos);
  // sanity check mood/pos
  if ((pos > 7)  || (pos <= 0)) return;
  //if ((mood > 3) || (mood < 0)) return;
  
  if (NO_MOOD == mood) {
    if (pos & MATRIX_POS_TOP) {
      fillTopRed(false);
    }
    if (pos & MATRIX_POS_MIDDLE) {
      fillMiddleYellow(false);
    }
    if (pos & MATRIX_POS_BOTTOM) {
      fillBottomGreen(false);
    }
    FastLED.show();
    return;
  }
  
  // fixme: rotate ... ccw - 90deg left
  unsigned smiley[24] = {2,3,4,5,        // col 8
                         9,14,           // col 7
                         16,20,23,       // col 6
                         24,29,31,       // col 5
                         32,37,39,       // col 4
                         40,44,47,       // col 3
                         49,54,          // col 2
                         58,59,60,61};   // col 1
  // sad: [7] 20 -> 22, [16] 44 -> 46
  if (SAD == mood) {
    smiley[7] = 22; smiley[16] = 46;
  }
  // neutral: [10] 37->36, [13] 29->28
  if (NEUTRAL == mood) {
    smiley[10] = 36; smiley[13] = 28;
  }

  for (unsigned i=0; i<24; i++) {
    if (pos & 1) leds[      smiley[i]] = CRGB::Yellow;
    if (pos & 2) leds[ 64 + smiley[i]] = CRGB::Yellow;
    if (pos & 4) leds[128 + smiley[i]] = CRGB::Yellow;
  }

  // blue eyes
  if (pos & MATRIX_POS_TOP) {
    leds[18] = CRGB::Blue;
    leds[42] = CRGB::Blue;
  }
  if (pos & MATRIX_POS_MIDDLE) {
    leds[64+18] = CRGB::Blue;
    leds[64+42] = CRGB::Blue;
  }
  if (pos & MATRIX_POS_BOTTOM) {
    leds[128+18] = CRGB::Blue;
    leds[128+42] = CRGB::Blue;
  }
  FastLED.show();
}

void startSequence()
{
  allLedsOff();
  delay(1000);

  // // 3 ... (leds: 48..63, 112..127, 176..191)
  // for (unsigned n=48; n<=63; n++) {
  //   leds[n] = CRGB::Red;
  //   leds[64+n] = CRGB::Red;
  //   leds[128+n] = CRGB::Red;
  // }
  //FastLED.show();
  fillTopMatrix(CRGB::Red);
  delay(1000);

  // 2 ... (leds: 24..39 ... )
  // for (unsigned n=24; n<=39; n++) {
  //   leds[n] = CRGB::Red;
  //   leds[64+n] = CRGB::Red;
  //   leds[128+n] = CRGB::Red;
  // }
  // FastLED.show();
  fillMiddleMatrix(CRGB::Red);
  delay(1000);

  // 1 ... (leds:  0..15, ...)
  // for (unsigned n=0; n<=15; n++) {
  //   leds[n] = CRGB::Red;
  //   leds[64+n] = CRGB::Red;
  //   leds[128+n] = CRGB::Red;
  // }
  // FastLED.show();
  fillBottomMatrix(CRGB::Red);
  
  // wert zwischen 500ms und 1500ms
  delay(random(500, 2000));
  fillSolid(leds, 0, NUM_LEDS, CRGB::Green);
  FastLED.show();
  stopwatch = millis();

  //DateTime.sync(0); // start the clock
}

// binary 24h-clock on "top" matrix TODO: make position (top,middle,or bottom) an argument
// yellow "dots" (i.e. here 2x2 squares) on blue background
/* 
     _ _ _ _ _ _ _ _
    |_|_|f|f|_|_|m|m|
    |_|_|f|f|_|_|m|m|
    |_|_|e|e|i|i|l|l|
    |_|_|e|e|i|i|l|l|
    |b|b|d|d|h|h|k|k|
    |b|b|d|d|h|h|k|k|
    |a|a|c|c|g|g|j|j|
    |a|a|c|c|g|g|j|j|
    a,b     = hour 1st digit (a=2^0, b=2^1)
    c,d,e,f = hour 2nd digit (c=2^0, d=2^1, e=2^2, f=2^3)
    ... and so on ;)

    here leb numbers are:
    a: 62,63,54,55
    b: 60,61,52,53
    c: 46,38,47,39
    d: 44,36,45,37
    e: 42,34,43,35
    f: 40,32,41,33
    g: 30,22,31,23
    h: 28,20,29,21
    i: 26,18,27,19
    j: 14,6,15,7
    k: 12,4,13,5
    l: 10,2,11,3
    m: 8,0,9,1

*/
//fixme: maybe this is not the most elegant way ...
void drawBinClock(int hour, int min)
{
  allLedsOff();
  fillTopMatrix(CRGB::Blue);

  int Hdigit1 = (int) hour / 10;
  int Hdigit2 = (int) hour % 10;
  int Mdigit1 = (int) min  / 10;
  int Mdigit2 = (int) min  % 10;

  // isMqttAvailable = mqttClient.publish(mqttClock, String(Hdigit1).c_str());
  // isMqttAvailable = mqttClient.publish(mqttClock, String(Hdigit2).c_str());
  // isMqttAvailable = mqttClient.publish(mqttClock, String(Mdigit1).c_str());
  // isMqttAvailable = mqttClient.publish(mqttClock, String(Mdigit2).c_str());

  if (Hdigit1 & 1) { /* a */
    leds[62] = CRGB::Yellow;
    leds[63] = CRGB::Yellow;
    leds[54] = CRGB::Yellow;
    leds[55] = CRGB::Yellow;
  }
  if (Hdigit1 & 2) { /* b */
    leds[60] = CRGB::Yellow;
    leds[61] = CRGB::Yellow;
    leds[52] = CRGB::Yellow;
    leds[53] = CRGB::Yellow;
  }

  if (Hdigit2 & 1) { /* c */
    leds[46] = CRGB::Yellow;
    leds[47] = CRGB::Yellow;
    leds[38] = CRGB::Yellow;
    leds[39] = CRGB::Yellow;
  }
  if (Hdigit2 & 2) { /* d */
    leds[44] = CRGB::Yellow;
    leds[45] = CRGB::Yellow;
    leds[36] = CRGB::Yellow;
    leds[37] = CRGB::Yellow;
  }
  if (Hdigit2 & 4) { /* e */
    leds[42] = CRGB::Yellow;
    leds[43] = CRGB::Yellow;
    leds[34] = CRGB::Yellow;
    leds[35] = CRGB::Yellow;
  }
  if (Hdigit2 & 8) { /* f */
    leds[40] = CRGB::Yellow;
    leds[41] = CRGB::Yellow;
    leds[32] = CRGB::Yellow;
    leds[33] = CRGB::Yellow;
  }

  if (Mdigit1 & 1) { /* g */
    leds[30] = CRGB::Yellow;
    leds[31] = CRGB::Yellow;
    leds[22] = CRGB::Yellow;
    leds[23] = CRGB::Yellow;
  }
  if (Mdigit1 & 2) { /* h */
    leds[28] = CRGB::Yellow;
    leds[29] = CRGB::Yellow;
    leds[20] = CRGB::Yellow;
    leds[21] = CRGB::Yellow;
  }
  if (Mdigit1 & 4) { /* i */
    leds[26] = CRGB::Yellow;
    leds[27] = CRGB::Yellow;
    leds[18] = CRGB::Yellow;
    leds[19] = CRGB::Yellow;
  }

  if (Mdigit2 & 1) { /* j */
    leds[14] = CRGB::Yellow;
    leds[15] = CRGB::Yellow;
    leds[6]  = CRGB::Yellow;
    leds[7]  = CRGB::Yellow;
  }
  if (Mdigit2 & 2) { /* k */
    leds[12] = CRGB::Yellow;
    leds[13] = CRGB::Yellow;
    leds[4]  = CRGB::Yellow;
    leds[5]  = CRGB::Yellow;
  }
  if (Mdigit2 & 4) { /* l */
    leds[10] = CRGB::Yellow;
    leds[11] = CRGB::Yellow;
    leds[2]  = CRGB::Yellow;
    leds[3]  = CRGB::Yellow;
  }
  if (Mdigit2 & 8) { /* m */
    leds[8] = CRGB::Yellow;
    leds[9] = CRGB::Yellow;
    leds[0] = CRGB::Yellow;
    leds[1] = CRGB::Yellow;
  }

  FastLED.show();
}
