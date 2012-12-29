#include "Arduino.h"
#include "BuildSegment.h"

BuildSegment::BuildSegment(int leds[], int ledcount, animation animationmode)
{
  _leds = leds;
  _ledcount = ledcount;
  _animationmode = animationmode;

  _building = false;
  _mode = OFF;
  _activeCycle = 0;
  _buildcycle = 3000;

  COLOR_BLACK = Color(0, 0, 0);
  COLOR_GREEN = Color(0, 255, 0);
  COLOR_RED = Color(255, 0, 0);
  COLOR_BLUE = Color(0, 0, 255);
}

String BuildSegment::getStatus() {
  /*
  if (state[i] == OFF) client.print(" Off");       
   if (state[i] == SUCCESS) client.print(" SUCCESSFUL");       
   if (state[i] == FAILED) client.print(" FAILED");       
   if (state[i] == DEMO) client.print(" in DEMO mode");       
   */
  if (_building) return "is building";

}

// Create a 24 bit color value from R,G,B
uint32_t BuildSegment::Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

uint32_t BuildSegment::Wheel(byte WheelPos)
{
  if (WheelPos < 85) {
    return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } 
  else if (WheelPos < 170) {
    WheelPos -= 85;
    return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } 
  else {
    WheelPos -= 170; 
    return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

boolean BuildSegment::hasChange() {
  if (_mode == OFF) {
    return false;
  }

  if (millis() - _lastmilli > 20) {
    _lastmilli = millis();
    return true;
  }

  return false;
}

void BuildSegment::setMode(buildmode mode) {
  _mode = mode;
}

void BuildSegment::setBuilding(boolean building) {
  _building = building;
}

void BuildSegment::tick(Adafruit_WS2801 strip) {

  uint32_t onColor = COLOR_BLACK;

  switch (_mode) {
  case OFF:
    onColor = COLOR_BLACK;
    break;
  case SUCCESS:
    onColor = COLOR_GREEN;
    break;
  case FAILED:
    onColor = COLOR_RED;
    break;
  case DEMO:
  default: 
    onColor = COLOR_BLUE;
  }

  if (_building) {

    // We are in loop mode, so do we have a tick of the clock to move onto the next light
    if (millis() - _lastAction > _buildcycle/_ledcount) {

      int p = previousColour(_activeLed);
      int c = currentColour(_activeLed);

      //strip.setPixelColor(0, COLOR_BLACK);

      strip.setPixelColor(_leds[p], COLOR_BLACK);
      strip.setPixelColor(_leds[c], onColor);

      strip.show();

      Serial.print("DEBUG - ActiveIdx is ");
      Serial.print(_activeLed);
      Serial.print(", previusColour is ");
      Serial.print(p);
      Serial.print(", _leds[p] is ");
      Serial.println(_leds[p]);
      /*
			Serial.print("DEBUG - ActiveIdx is ");
       			Serial.print(_activeLed);
       			Serial.print(", currentColour is ");
       			Serial.print(c);
       			Serial.print(", _leds[c] is ");
       			Serial.println(_leds[c]);
       */
      _activeLed++;
      if (loopResetRequired(_activeLed)) 
        _activeLed=0;

      _lastAction = millis();
    }
  }    
  else {
    if (_mode == DEMO) {
      for (int i=0; i < _ledcount; i++) {
        // Offset complex rainbow
        strip.setPixelColor(_leds[i], Wheel( ((i * 256 / _ledcount) + _activeCycle) % 256) );
      }  
      strip.show();   // write all the pixels out

      _activeCycle++;
      if (_activeCycle >= 256) {
        _activeCycle = 0;
      }
    }
    else {
      colorWipeArray(strip, _leds, _ledcount, onColor, 0);
    }
  }
}

boolean BuildSegment::loopResetRequired(int activeIdx) {

  switch (_animationmode) {
  case BACKFORWARD:
    return _activeLed >= (_ledcount*2) - 2;
  }

  return _activeLed >= _ledcount;
}

int BuildSegment::previousColour(int activeIdx) {

  switch (_animationmode) {
  case BACKFORWARD:
    if (activeIdx == 0) {
      return 1;
    }
    else if (activeIdx >= _ledcount) {
      return _ledcount - (activeIdx - _ledcount) - 1;
    }
    else {
      return (activeIdx - 1);
    }

    break;

  case LOOP:
    if (activeIdx == 0) {
      // On the first one, turn off the last on in the sequence
      return _ledcount-1;
    }

    return activeIdx - 1;
    break;
  }

  return 2;
}


int BuildSegment::currentColour(int activeIdx) {

  switch (_animationmode) {

  case LOOP:
    return activeIdx;

  case BACKFORWARD:
    if (activeIdx == 0) {
      return 0;
    }
    else if (activeIdx >= _ledcount) {
      return _ledcount - (activeIdx - _ledcount)-2;
    }
    else {
      return activeIdx;
    }
  }

  return 3;
}

void BuildSegment::colorWipeArray(Adafruit_WS2801 strip, int leds[], int ledcount, uint32_t c, uint8_t wait) {
  int i;

  for (i=0; i < ledcount; i++) {
    strip.setPixelColor(leds[i], c);
    strip.show();
    delay(wait);
  }
}


