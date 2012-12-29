#ifndef BuildSegment_h
#define BuildSegment_h

#include "Arduino.h"
#include "Adafruit_WS2801.h"

enum buildmode {
  OFF,
  SUCCESS,
  FAILED,
  DEMO
};

enum animation {
  LOOP,
  BACKFORWARD,
  PULSE
};

class BuildSegment {

public:
  BuildSegment(int leds[], int ledcount, animation animationmode);
  boolean hasChange();
  void setMode(buildmode mode);
  void setBuilding(boolean building);
  void tick(Adafruit_WS2801 strip);
  String getStatus();

  uint32_t Color(byte r, byte g, byte b);
  uint32_t Wheel(byte WheelPos);

  // Constant colours
  uint32_t COLOR_BLACK;
  uint32_t COLOR_GREEN;
  uint32_t COLOR_RED;
  uint32_t COLOR_BLUE;

private:

  void colorWipeArray(Adafruit_WS2801 strip, int leds[], int ledcount, uint32_t c, uint8_t wait);

  boolean loopResetRequired(int activeIdx);
  int currentColour(int activeIdx);
  int previousColour(int activeIdx);

  boolean _building;
  buildmode _mode;

  // The led's allocated for this segment
  int *_leds;
  // The count of the leds
  int _ledcount;

  int _buildcycle;

  // How do we animate?
  animation _animationmode;

  long _lastmilli;
  long _lastAction;

  int _activeLed;
  int _activeCycle;

};

#endif

