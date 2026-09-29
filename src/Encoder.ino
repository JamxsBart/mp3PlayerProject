#pragma once
#include <Arduino.h>
class Encoder { public: void begin(); int consumeClicks(); private: static volatile int counter; static volatile uint8_t previousState; int lastCounter=0; static void IRAM_ATTR isr(); };
