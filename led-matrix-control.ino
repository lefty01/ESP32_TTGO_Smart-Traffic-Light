
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
  
  /* FIXME/todo: use some library eg. https://github.com/Jorgen-VikingGod/LEDMatrix
     _ _ _ _ _ _ _ _
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|
    |_|_|_|_|_|_|_|_|

   */
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

