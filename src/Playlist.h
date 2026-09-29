#pragma once
#include <Arduino.h>
#include <SD.h>
#include "Config.h"
struct Track { char path[Config::MAX_PATH_LENGTH]; };
class Playlist {
public: bool load(); uint16_t size() const; bool empty() const; const Track* get(int index) const; int randomIndexExcept(int currentIndex) const;
private: Track tracks[Config::MAX_TRACKS]; uint16_t trackCount=0; static bool isAudioFile(const char* filename); void addTrack(const char* filename);
};
