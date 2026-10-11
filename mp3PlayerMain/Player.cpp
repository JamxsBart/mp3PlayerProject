#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "Audio_nopsram.h"
#include "Config.h"
#include "Display.h"
#include "Player.h"

Audio audio;
SPIClass sdSpi(HSPI);

char playlist[maxTracks][maxPath];
int trackCount = 0;
int currentTrack = -1;
int volume = 15;
bool paused = false;
bool playerReady = false;

volatile bool trackEnded = false;
volatile bool trackLoaded = false;

bool playTrack(int index);
void tryAnotherTrack();
bool isAudioFile(const char* name);

bool setupPlayer() {
  sdSpi.begin(sdSpiSck, sdSpiMiso, sdSpiMosi, sdSpiCs);
  if (!SD.begin(sdSpiCs, sdSpi, 8000000)) {
    Serial.println("SD FAILED");
    showSdError();
    return false;
  }
  Serial.println("SD working");

  audio.setPinout(i2sBclk, i2sLrc, i2sDout);
  audio.setVolume(volume);
  Serial.println("Audio working");

  File dir = SD.open("/library");
  if (dir) {
    while (trackCount < maxTracks) {
      File entry = dir.openNextFile();
      if (!entry) break;

      if (!entry.isDirectory()) {
        const char* name = entry.name();
        if (isAudioFile(name)) {
          if (name[0] == '/')
            snprintf(playlist[trackCount], maxPath, "%s", name);
          else
            snprintf(playlist[trackCount], maxPath, "/library/%s", name);
          trackCount++;
        }
      }
      entry.close();
    }
    dir.close();
  }

  Serial.print("Found ");
  Serial.print(trackCount);
  Serial.println(" songs");

  if (trackCount == 0) {
    showNoSongs();
    return false;
  }

  return true;
}

void startPlayer() {
  randomSeed(esp_random());
  playerReady = true;
  currentTrack = random(0, trackCount);

  if (!playTrack(currentTrack)) {
    tryAnotherTrack();
  }
}

void updatePlayer() {
  audio.loop();

  if (trackEnded) {
    trackEnded = false;
    skipToRandom();
  }
}

bool getPlayerReady() {
  return playerReady;
}

bool playTrack(int index) {
  if (index < 0 || index >= trackCount) return false;
  currentTrack = index;
  paused = false;
  trackLoaded = false;

  if (!audio.connecttoFS(SD, playlist[index])) {
    Serial.print("Failed to load: ");
    Serial.println(playlist[index]);
    showTrackLoadError();
    return false;
  }

  trackLoaded = true;
  showCurrentTrack();
  Serial.print("Playing: ");
  Serial.println(playlist[index]);
  return true;
}

void skipToRandom() {
  if (trackCount <= 1) return;
  int nextTrack = currentTrack;
  int tries = 0;

  while (nextTrack == currentTrack && tries < 20) {
    nextTrack = random(0, trackCount);
    tries++;
  }

  if (!playTrack(nextTrack)) {
    tryAnotherTrack();
  }
}

void tryAnotherTrack() {
  int failedTrack = currentTrack;

  for (int i = 1; i < trackCount; i++) {
    int nextTrack = (failedTrack + i) % trackCount;
    if (playTrack(nextTrack)) return;
  }

  playerReady = false;
  showNoPlayableSongs();
}

bool isAudioFile(const char* name) {
  int length = strlen(name);
  if (length < 5) return false;

  const char* extension = name + length - 4;
  return (strcasecmp(extension, ".mp3") == 0 ||
          strcasecmp(extension, ".wav") == 0);
}

void pauseResume() {
  audio.pauseResume();
  paused = !paused;
  showCurrentTrack();
}

void showCurrentTrack() {
  if (currentTrack < 0 || currentTrack >= trackCount) return;
  showTrack(playlist[currentTrack], currentTrack, trackCount, paused, volume);
}

void changeVolume(int amount) {
  volume += amount;
  volume = constrain(volume, 0, 21);
  audio.setVolume(volume);
  Serial.print("Volume: ");
  Serial.println(volume);
}

int getVolume() {
  return volume;
}

void audio_eof_mp3(const char *info) {
  if (!trackLoaded) {
    Serial.println("Ignoring EOF because no track is loaded");
    return;
  }

  trackLoaded = false;
  trackEnded = true;
}
