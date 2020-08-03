
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
//FIXME: maybe do not call the drawTrafficLight function from here ...
//       move to calling function
void fillTopMatrix(const struct CRGB& color, bool ledShow) {
  fillSolid(leds, 0, 64, color);
  if (ledShow) FastLED.show();
}
void fillMiddleMatrix(const struct CRGB& color, bool ledShow) {
  fillSolid(leds, 64, 64, color);
  if (ledShow) FastLED.show();
}
void fillBottomMatrix(const struct CRGB& color, bool ledShow) {
  fillSolid(leds, 128, 64, color);
  if (ledShow) FastLED.show();
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
// here we have yellow "dots" on blue background
/* 
hh:mm (1 bit : 2x2 square)
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

    here led numbers are (first/top matrix):
    a: H_1_0: 62,63,54,55
    b: H_1_1: 60,61,52,53
    c: H_2_0: 46,38,47,39
    d: H_2_1: 44,36,45,37
    e: H_2_2: 42,34,43,35
    f: H_2_3: 40,32,41,33
    g: M_1_0: 30,22,31,23
    h: M_1_1: 28,20,29,21
    i: M_1_2: 26,18,27,19
    j: M_2_0: 14, 6,15, 7
    k: M_2_1: 12, 4,13, 5
    l: M_2_2: 10, 2,11, 3
    m: M_2_3:  8, 0, 9, 1


seconds: (1 bit : 2x4 square)
     _ _ _ _ _ _ _ _
    |d|d|d|d|h|h|h|h|
    |d|d|d|d|h|h|h|h|
    |c|c|c|c|g|g|g|g|
    |c|c|c|c|g|g|g|g|
    |b|b|b|b|f|f|f|f|
    |b|b|b|b|f|f|f|f|
    |a|a|a|a|e|e|e|e|
    |a|a|a|a|e|e|e|e|
    a-d: seconds 1st digit
    e-h: seconds 2nd digit

    here led numbers are (first/top matrix):
    a: S_1_0: 126 118 110 102 127 119 111 103
    b: S_1_1: 124 116 108 100 125 117 109 101
    c: S_1_2: 122 114 106  98 123 115 107  99
    d: S_1_3: 120 112 104  96 121 113 105  97
    e: S_2_0:  94  86  78  70  95  87  79  71
    f: S_2_1:  92  84  76  68  93  85  77  69
    g: S_2_2:  90  82  74  66  91  83  75  67
    h: S_2_3:  88  80  72  64  89  81  73  65

*/

const unsigned H_1_0[4] = { 62, 63, 54, 55 };
const unsigned H_1_1[4] = { 60, 61, 52, 53 };
const unsigned H_2_0[4] = { 46, 38, 47, 39 };
const unsigned H_2_1[4] = { 44, 36, 45, 37 };
const unsigned H_2_2[4] = { 42, 34, 43, 35 };
const unsigned H_2_3[4] = { 40, 32, 41, 33 };
const unsigned M_1_0[4] = { 30, 22, 31, 23 };
const unsigned M_1_1[4] = { 28, 20, 29, 21 };
const unsigned M_1_2[4] = { 26, 18, 27, 19 };
const unsigned M_2_0[4] = { 14,  6, 15,  7 };
const unsigned M_2_1[4] = { 12,  4, 13,  5 };
const unsigned M_2_2[4] = { 10,  2, 11,  3 };
const unsigned M_2_3[4] = {  8,  0,  9,  1 };
const unsigned S_1_0[8] = { 126, 118, 110, 102, 127, 119, 111, 103 };
const unsigned S_1_1[8] = { 124, 116, 108, 100, 125, 117, 109, 101 };
const unsigned S_1_2[8] = { 122, 114, 106,  98, 123, 115, 107,  99 };
//const unsigned S_1_3[8] = { 120, 112, 104,  96, 121, 113, 105,  97 }; // UNUSED
const unsigned S_2_0[8] = {  94,  86,  78,  70,  95,  87,  79,  71 };
const unsigned S_2_1[8] = {  92,  84,  76,  68,  93,  85,  77,  69 };
const unsigned S_2_2[8] = {  90,  82,  74,  66,  91,  83,  75,  67 };
const unsigned S_2_3[8] = {  88,  80,  72,  64,  89,  81,  73,  65 };
// day of month and month
const unsigned d_1_0[4] = { 190, 182, 191, 183 };
const unsigned d_1_1[4] = { 188, 180, 189, 181 };
const unsigned d_2_0[4] = { 174, 166, 175, 167 };
const unsigned d_2_1[4] = { 172, 164, 173, 165 };
const unsigned d_2_2[4] = { 170, 162, 171, 163 };
const unsigned d_2_3[4] = { 168, 160, 169, 161 };
const unsigned m_1_0[4] = { 158, 150, 159, 151 };
const unsigned m_2_0[4] = { 142, 134, 143, 135 };
const unsigned m_2_1[4] = { 140, 132, 141, 133 };
const unsigned m_2_2[4] = { 138, 130, 139, 131 };
const unsigned m_2_3[4] = { 136, 128, 137, 129 };

void turnBitOn(const unsigned in[], int num, const struct CRGB& color) {
  for (unsigned n = 0; n < num; ++n) {
    leds[in[n]] = color;
  }
}

void drawBinClockSec(int sec)
{
  int s1 = (int) sec / 10;
  int s2 = (int) sec % 10;

  fillMiddleMatrix(CRGB::Blue, false);

  if (s1 & 1) turnBitOn(S_1_0, 8, CRGB::Yellow);
  if (s1 & 2) turnBitOn(S_1_1, 8, CRGB::Yellow);
  if (s1 & 4) turnBitOn(S_1_2, 8, CRGB::Yellow);

  if (s2 & 1) turnBitOn(S_2_0, 8, CRGB::Yellow);
  if (s2 & 2) turnBitOn(S_2_1, 8, CRGB::Yellow);
  if (s2 & 4) turnBitOn(S_2_2, 8, CRGB::Yellow);
  if (s2 & 8) turnBitOn(S_2_3, 8, CRGB::Yellow);

  FastLED.show();
  delay(10);
}


// currently gets called every second if in CLOCK mode
//void drawBinClockHourMin(const DateTime &dtNow, const DateTime &dtPrev)
void drawBinClockHourMin(const DateTime &dtNow)
{
  int hour    = dtNow.dt_hours;
  int min     = dtNow.dt_minutes;

  // TODO:  maybe add some "fade in/out" effect from fastled (which bits changed?)
  fillTopMatrix(CRGB::Blue, false);

  int Hdigit1 = (int) hour / 10;
  int Hdigit2 = (int) hour % 10;
  int Mdigit1 = (int) min  / 10;
  int Mdigit2 = (int) min  % 10;

  if (Hdigit1 & 1) turnBitOn(H_1_0, 4, CRGB::Yellow);
  if (Hdigit1 & 2) turnBitOn(H_1_1, 4, CRGB::Yellow);

  if (Hdigit2 & 1) turnBitOn(H_2_0, 4, CRGB::Yellow);
  if (Hdigit2 & 2) turnBitOn(H_2_1, 4, CRGB::Yellow);
  if (Hdigit2 & 4) turnBitOn(H_2_2, 4, CRGB::Yellow);
  if (Hdigit2 & 8) turnBitOn(H_2_3, 4, CRGB::Yellow);

  if (Mdigit1 & 1) turnBitOn(M_1_0, 4, CRGB::Yellow);
  if (Mdigit1 & 2) turnBitOn(M_1_1, 4, CRGB::Yellow);
  if (Mdigit1 & 4) turnBitOn(M_1_2, 4, CRGB::Yellow);
  if (Mdigit2 & 1) turnBitOn(M_2_0, 4, CRGB::Yellow);
  if (Mdigit2 & 2) turnBitOn(M_2_1, 4, CRGB::Yellow);
  if (Mdigit2 & 4) turnBitOn(M_2_2, 4, CRGB::Yellow);
  if (Mdigit2 & 8) turnBitOn(M_2_3, 4, CRGB::Yellow);

  FastLED.show();
  delay(10);
}

void drawBinClockDate(int day, int month)
{
  int d1 = (int) day / 10;
  int d2 = (int) day % 10;
  int m1 = (int) month / 10;
  int m2 = (int) month % 10;

  fillBottomMatrix(CRGB::Blue, false);

  if (d1 & 1) turnBitOn(d_1_0, 4, CRGB::Yellow);
  if (d1 & 2) turnBitOn(d_1_1, 4, CRGB::Yellow);

  if (d2 & 1) turnBitOn(d_2_0, 4, CRGB::Yellow);
  if (d2 & 2) turnBitOn(d_2_1, 4, CRGB::Yellow);
  if (d2 & 4) turnBitOn(d_2_2, 4, CRGB::Yellow);
  if (d2 & 8) turnBitOn(d_2_3, 4, CRGB::Yellow);

  if (m1 & 1) turnBitOn(m_1_0, 4, CRGB::Yellow);

  if (m2 & 1) turnBitOn(m_2_0, 4, CRGB::Yellow);
  if (m2 & 2) turnBitOn(m_2_1, 4, CRGB::Yellow);
  if (m2 & 4) turnBitOn(m_2_2, 4, CRGB::Yellow);
  if (m2 & 8) turnBitOn(m_2_3, 4, CRGB::Yellow);

  FastLED.show();
  delay(10);
}


// void drawBinClockHour(int h)
// {
//   int h1 = (int) h / 10; // first  digit 0, 1, or 2
//   int h2 = (int) h % 10; // second digit 0-9

//   // changes = h1 ^ prev_h1
//   // if changes_h1 & 1 && h1 & 1 direction blue->yellow H_1_0
//   // if changed_h1 & 2 && h1 & 2 ..
//   // okay for 1st digit of hour does not make sense since
//   // there will always be a bit change ...

//   //  changes_h2 ...


// }
