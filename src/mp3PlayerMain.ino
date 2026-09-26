//Simple MP3 Player v1.0
//jamxsbart

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <SD.h>
#include "Audio_nopsram.h"

#define SD_SPI_SCK   14
#define SD_SPI_MOSI  13
#define SD_SPI_MISO  19
#define SD_SPI_CS    27
#define I2S_BCLK  26
#define I2S_LRC   25
#define I2S_DOUT  22
#define ENC_CLK  17
#define ENC_DT   33
#define ENC_SW   21

#define MAX_TRACKS 300
#define MAX_PATH   96

TFT_eSPI tft = TFT_eSPI();
Audio audio;
SPIClass SDSPI(HSPI);

char playlist[MAX_TRACKS][MAX_PATH];
int trackCount = 0;
int currentTrack = -1;
bool paused = false;

volatile bool trackEnded = false;

unsigned long trackStartMs = 0;

//button edge detection
bool lastButtonState = HIGH;
int clickCount = 0;
unsigned long firstClickTime = 0;

//volume control code
int volume = 15;
int lastClkState;
unsigned long buttonPressTime = 0;
bool menuOpen = false;
bool longPressHandled = false;

void playTrack(int index);
void showTrackName();
void skipToRandom();
bool isAudioFile(const char* name);
void showMenu();
void checkEncoder();

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n--- Simple Player ---");

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Loading...");

  SDSPI.begin(SD_SPI_SCK, SD_SPI_MISO, SD_SPI_MOSI, SD_SPI_CS);
  if (!SD.begin(SD_SPI_CS, SDSPI, 8000000)) {
    Serial.println("SD FAILED");
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(10, 10);
    tft.println("SD card failed to load");
    return;
  }
  Serial.println("SD working");

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(15);
  Serial.println("Audio working");

  File dir = SD.open("/library");
  if (dir) {
    while (trackCount < MAX_TRACKS) {
      File entry = dir.openNextFile();
      if (!entry) break;
      if (!entry.isDirectory()) {
        const char* n = entry.name();
        if (isAudioFile(n)) {
          if (n[0] == '/')
            snprintf(playlist[trackCount], MAX_PATH, "%s", n);
          else
            snprintf(playlist[trackCount], MAX_PATH, "/library/%s", n);
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

  pinMode(ENC_SW, INPUT_PULLUP);
  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT, INPUT_PULLUP);
  lastClkState = digitalRead(ENC_CLK);

  randomSeed(esp_random());
  if (trackCount > 0) {
    currentTrack = random(0, trackCount);
    playTrack(currentTrack);
  } else {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(10, 10);
    tft.println("No songs found");
  }
}

void playTrack(int index) {
  if (index < 0 || index >= trackCount) return;
  currentTrack = index;
  paused = false;
  trackStartMs = millis();
  showTrackName();
  audio.connecttoFS(SD, playlist[index]);
  Serial.print("Playing: ");
  Serial.println(playlist[index]);
}

bool isAudioFile(const char* name) {
  int len = strlen(name);
  if (len < 5) return false;
  const char* ext = name + len - 4;
  return (strcasecmp(ext, ".mp3") == 0 ||
          strcasecmp(ext, ".wav") == 0);
}

void showTrackName() {
  const char* path = playlist[currentTrack];
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
}

void skipToRandom() {
  if (trackCount <= 1) return;
  int n = currentTrack;
  int tries = 0;
  while (n == currentTrack && tries < 20) {
    n = random(0, trackCount);
    tries++;
  }
  playTrack(n);
}

//simple menu design
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

void checkEncoder() {
  int clk = digitalRead(ENC_CLK);
  if (clk != lastClkState && clk == LOW) {
    if (digitalRead(ENC_DT) != clk) {
      volume++;
    } else {
      volume--;
    }
    volume = constrain(volume, 0, 21);
    audio.setVolume(volume);
    Serial.print("Volume: ");
    Serial.println(volume);
  }
  lastClkState = clk;
}

void loop() {
  audio.loop();

  if (trackEnded) {
    trackEnded = false;
    skipToRandom();
  }

  if (!menuOpen) {
    checkEncoder();
  }

  bool button = digitalRead(ENC_SW);

  if (button == LOW && lastButtonState == HIGH) {
    if (menuOpen) {
      //short press in menu goes back to music/main home screen
      menuOpen = false;
      showTrackName();
      lastButtonState = button;
      return;
    }
    //single press
    buttonPressTime = millis();
    if (clickCount == 0) firstClickTime = millis();
    clickCount++;
    Serial.print("Click: ");
    Serial.println(clickCount);
  }

  //long press detection 1 second
  if (button == LOW && !longPressHandled && (millis() - buttonPressTime > 1000)) {
    longPressHandled = true;
    menuOpen = true;
    showMenu();
    clickCount = 0;
  }

  if (button == HIGH && lastButtonState == LOW) {
    if (longPressHandled) {
      longPressHandled = false;
    }
  }

  lastButtonState = button;

  //once 400ms has passed with no new clicks
  if (!menuOpen && !longPressHandled && clickCount > 0 && (millis() - firstClickTime) > 400) {
    if (clickCount == 1) {
      audio.pauseResume();
      paused = !paused;
      showTrackName();
      Serial.println("Pause/Play");
    } else {
      skipToRandom();
      Serial.println("Skip");
    }
    clickCount = 0;
  }
}

void audio_eof_mp3(const char *info) {
  unsigned long now = millis();
  if (now - trackStartMs < 2000) {
    Serial.println("(ignoring spurious EOF)");
    return;
  }
  trackEnded = true;
}