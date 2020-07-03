

// FIXME: function names
// FIXME: draw simley on tft in mood mode
// FIXME: call drawTrafficLight from here instead of within fill functions?!
void b1()
{
  switch (MODE) {
  case TRAFFIC:  {
    fillTopRed(toggleRed);
    break;
  }
  case MOOD: {
    drawSmiley(toggleRed ? HAPPY : NO_MOOD, MATRIX_POS_TOP);
    break;
  }
  case TRAFFIC_MANUAL:
    TRAFFIC_LIGHT_MODE += 1;
    TRAFFIC_LIGHT_MODE %= _NUM_TRAFFIC_LIGHT_MODES_;
    handleTrafficLight();
    break;
  case STARTAMPEL: // start
    startSequence();
    break;
  case TRAFFIC_AUTO:
  case PATTERN:
  case PARTY:
  default:
    break;
  }
  toggleRed = !toggleRed;
}


void b2()
{
  switch (MODE) {
  case TRAFFIC:
    fillMiddleYellow(toggleYellow);
    break;

  case MOOD:
    drawSmiley(toggleYellow ? NEUTRAL : NO_MOOD, MATRIX_POS_MIDDLE);
    break;

  case STARTAMPEL: { // stop
    unsigned long time = millis() - stopwatch;
    drawTime(time);
    DEBUG_PRINT("time: ");
    DEBUG_PRINTLN(time);
    break;
  }
  default:
    break;
  }
  toggleYellow = !toggleYellow;
}


void b3()
{
  switch (MODE) {
  case TRAFFIC: {
    fillBottomGreen(toggleGreen);
    break;
  }
  case MOOD: {
    drawSmiley(toggleGreen ? SAD : NO_MOOD, MATRIX_POS_BOTTOM);
    break;
  }
  default:
    break;
  }
  toggleGreen = !toggleGreen;
}

