#pragma once
#include <Arduino.h>
#include <SD.h>
#include "Audio_nopsram.h"
#include "Config.h"
class AudioPlayer { public: void begin(); void update(); void play(const char*); void togglePause(); void setVolume(int); int getVolume()const; bool isPaused()const; bool consumeTrackEnded(); void onEndOfFile(); private: Audio audio; int volume=Config::DEFAULT_VOLUME; bool paused=false; volatile bool trackEnded=false; uint32_t trackStarted=0; };
void audio_eof_mp3(const char* info);
