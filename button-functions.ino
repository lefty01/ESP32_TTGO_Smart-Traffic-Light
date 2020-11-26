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


  // encoder push-button handler
  encBtnP.setTapHandler([](Button2 & b) {
    unsigned int time = b.wasPressedFor();
    DEBUG_PRINTLN(time);
    if (time > 3000) { // > 3sec enters config menu
      DEBUG_PRINTLN("very long click ... do nothing");
    }
    else if (time > 600) {
      DEBUG_PRINTLN("Encoder Button PUSH long click...");
      // longer click while in select menu exit and restore prev mode
      if (opMode == MODE_SELECT) {
	DEBUG_PRINTLN("exit mode select menu");
	DEBUG_PRINTMQTT("exit mode select menu");
	opMode = prevMode;
	if (opMode == CLOCK) showTimeNow = true;
      }
      else if (opMode == APP_CONFIG_SELECT) {
	DEBUG_PRINTLN("exit config (sub) mode");
	DEBUG_PRINTMQTT("exit config sub menu");
	// ... or shall we exit menu directly?
	opMode = static_cast<opModes>(MODE_SELECT);
	allLedsOff();
	drawModeSelectMenu();
	return;
      }
      else {
	DEBUG_PRINTLN("enter mode select menu");
	DEBUG_PRINTMQTT("enter mode select menu");
	prevMode = opMode;
	opMode = static_cast<opModes>(MODE_SELECT);
	selectMode = prevMode;
	allLedsOff();
	drawModeSelectMenu();
	return;
      }
    }
    else {
      DEBUG_PRINTLN("Button PUSH short click...");
      // if we are in select menu then need short button click to enter/select that mode
      if (opMode == MODE_SELECT) {
	if (selectMode == APP_CONFIG) {
	  prevMode = opMode;
	  opMode   = static_cast<opModes>(APP_CONFIG_SELECT);
	  DEBUG_PRINTLN("enter app config menu");
	  DEBUG_PRINTMQTT("enter app config menu");
	  drawConfigSelectMenu();
	  return;
	}

	opMode = selectMode;
	DEBUG_PRINTF("select menu: choosing mode %s\n", mode2str(opMode));
	DEBUG_PRINTMQTT("select menu, choosing a new opmode");
      }
      // if we are in config select mode enter one of the configs (brightness, dst, )
      if (opMode == APP_CONFIG_SELECT) {
	DEBUG_PRINTLN("in app config selet menu... short click, select config");
	DEBUG_PRINTMQTT("in app config select menu... short click");

	//opMode = static_cast<opModes>(MODE_SELECT);
	if (configSelectMode == APP_CONFIG_BRIGHTNESS) {
	  DEBUG_PRINTLN("brightness config selected");
	  drawBrightnessConfigMenu(false);
	  allLedsOff();
	  return;
	}
      }
      if (opMode == DISCO) {
	// we exit disco mode ... clear leds
	DEBUG_PRINTLN("exit disco mode");
	allLedsOff();
      }
      if (opMode == CLOCK) {
	// we exit clock mode, reset flag
	// so we get time again immediately after a mode change
	showTimeNow = true;
      }
      toggleRed    = true;
      toggleYellow = true;
      toggleGreen  = true;
    } // short push-button

    tft.fillScreen(TFT_BLACK);

    isMqttAvailable = mqttClient.publish(mqttOpmode, mode2str(opMode), true);

    drawTrafficLight(0);
    drawModeText(opMode);

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

    encBtnP.loop();
}


// FIXME: function names
// FIXME: draw simley on tft in mood mode
// FIXME: call drawTrafficLight from here instead of within fill functions?!
void b1() // RED Button
{
  switch (opMode) {
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
  case DISCO:
  case APP_CONFIG:
  default:
    break;
  }
  toggleRed = !toggleRed;
}


void b2() // YELLOW Button
{
  switch (opMode) {
  case TRAFFIC:
    fillMiddleYellow(toggleYellow);
    break;

  case MOOD:
    drawSmiley(toggleYellow ? NEUTRAL : NO_MOOD, MATRIX_POS_MIDDLE);
    break;

  default:
    break;
  }
  toggleYellow = !toggleYellow;
}


void b3() // GREEN Button
{
  switch (opMode) {
  case TRAFFIC: {
    fillBottomGreen(toggleGreen);
    break;
  }
  case MOOD: {
    drawSmiley(toggleGreen ? SAD : NO_MOOD, MATRIX_POS_BOTTOM);
    break;
  }
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
  toggleGreen = !toggleGreen;
}

