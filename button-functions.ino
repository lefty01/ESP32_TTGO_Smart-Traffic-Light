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
      allLedsOff();
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

    //allLedsOff();

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

