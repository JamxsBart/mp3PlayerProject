#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "Config.h"
#include "AudioPlayer.h"
#include "Display.h"
#include "Encoder.h"
#include "Input.h"
#include "Playlist.h"
class PlayerController { public: void begin(); void update(); private: SPIClass sdSpi=SPIClass(HSPI); AudioPlayer audio; Display display; Encoder encoder; Input input; Playlist playlist; PlayerState state=PlayerState::Error; int currentTrack=-1; bool ready=false; void playTrack(int); void playRandomTrack(); void handleEnd(); void handleEncoder(); void handleButton(ButtonEvent); void refreshDisplay(); };
extern PlayerController playerController;
