#pragma once
#include <Arduino.h>
enum class ButtonEvent { None, SingleClick, DoubleClick, LongPress };
class Input { public: void begin(); ButtonEvent update(); private: bool lastPressed=false,longPressHandled=false; uint8_t clickCount=0; uint32_t firstClick=0,pressStart=0; };
