#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "Config.h"
enum class PlayerState { Playing, Paused, Menu, Error };
class Display { public: void begin(); void showLoading(); void showError(const char*); void showNoTracks(); void showNowPlaying(const char*,int,int,PlayerState,int); void showMenu(); private: TFT_eSPI tft; void clear(); void text(uint16_t,uint8_t); void drawVolume(int); void drawTrackName(const char*); };
