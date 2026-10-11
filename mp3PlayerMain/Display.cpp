#include <Arduino.h>
#include <TFT_eSPI.h>
#include "Display.h"

TFT_eSPI tft = TFT_eSPI();

void showMessage(const char* message, uint16_t colour) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(colour, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println(message);
}

void setupDisplay() {
  tft.init();
  tft.setRotation(1);
}

void showLoading() {
  showMessage("Loading...", TFT_WHITE);
}

void showSdError() {
  showMessage("SD card failed to load", TFT_WHITE);
}

void showNoSongs() {
  showMessage("No songs found", TFT_WHITE);
}

void showTrackLoadError() {
  showMessage("Track failed to load", TFT_RED);
}

void showNoPlayableSongs() {
  showMessage("No playable songs", TFT_RED);
}

void showTrack(const char* path, int currentTrack, int trackCount, bool paused, int volume) {
  const char* slash = strrchr(path, '/');
  const char* base = slash ? slash + 1 : path;

  char name[80];
  strncpy(name, base, sizeof(name) - 1);
  name[sizeof(name) - 1] = '\0';

  char* dot = strrchr(name, '.');
  if (dot) *dot = '\0';

  int nameLen = strlen(name);

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Now Playing");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  if (nameLen <= 22) {
    int x = (320 - nameLen * 12) / 2;
    if (x < 0) x = 0;
    tft.setCursor(x, 100);
    tft.println(name);
  } else {
    int middle = nameLen / 2;
    for (int i = 0; i < 10; i++) {
      if (middle + i < nameLen && name[middle + i] == ' ') {
        middle = middle + i;
        break;
      }
      if (middle - i > 0 && name[middle - i] == ' ') {
        middle = middle - i;
        break;
      }
    }

    char line1[80], line2[80];
    strncpy(line1, name, middle);
    line1[middle] = '\0';
    strncpy(line2, name + middle, sizeof(line2) - 1);
    line2[sizeof(line2) - 1] = '\0';
    tft.setCursor(10, 80);
    tft.println(line1);
    tft.setCursor(10, 110);
    tft.println(line2);
  }

  tft.setTextColor(paused ? TFT_ORANGE : TFT_GREEN, TFT_BLACK);
  tft.setCursor(10, 160);
  tft.println(paused ? "PAUSED" : "PLAYING");

  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(10, 220);
  tft.print("Song ");
  tft.print(currentTrack + 1);
  tft.print(" of ");
  tft.print(trackCount);

  drawVolume(volume);
}

void showMenu() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("MENU");
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setCursor(10, 60);
  tft.println("Return to Music");
}

void drawVolume(int volume) {
  tft.fillRect(0, 230, 320, 10, TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(10, 230);
  tft.print("Volume: ");
  tft.print(volume);
}
