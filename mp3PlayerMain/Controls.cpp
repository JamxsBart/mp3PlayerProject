#include <Arduino.h>
#include "Config.h"
#include "Controls.h"
#include "Display.h"
#include "Player.h"

bool lastButtonState = HIGH;
bool lastButtonReading = HIGH;
unsigned long lastDebounceTime = 0;
int clickCount = 0;
unsigned long firstClickTime = 0;
unsigned long buttonPressTime = 0;
bool menuOpen = false;
bool songListOpen = false;
int menuSelection = 0;
int songSelection = 0; // 0 is Back; songs start at 1.
bool longPressHandled = false;

volatile int encoderCounter = 0;
volatile uint8_t lastEncoderState = 0;
int lastEncoderCounter = 0;

void checkEncoder();
void IRAM_ATTR encoderISR();

bool isMenuOpen() {
  return menuOpen;
}

void resetEncoderPosition() {
  noInterrupts();
  lastEncoderCounter = encoderCounter;
  interrupts();
}

void setupControls() {
  pinMode(encoderSwitch, INPUT_PULLUP);
  pinMode(encoderClk, INPUT_PULLUP);
  pinMode(encoderDt, INPUT_PULLUP);

  lastButtonState = digitalRead(encoderSwitch);
  lastButtonReading = lastButtonState;
  lastEncoderState = (digitalRead(encoderClk) << 1) | digitalRead(encoderDt);

  attachInterrupt(digitalPinToInterrupt(encoderClk), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderDt), encoderISR, CHANGE);
}

void updateControls() {
  checkEncoder();

  bool buttonReading = digitalRead(encoderSwitch);

  // Wait for the button reading to stay the same before using it
  if (buttonReading != lastButtonReading) {
    lastDebounceTime = millis();
    lastButtonReading = buttonReading;
  }

  bool button = lastButtonState;
  if (millis() - lastDebounceTime > 30) {
    button = buttonReading;
  }

  if (button == LOW && lastButtonState == HIGH) {
    buttonPressTime = millis();
    if (menuOpen) {
      clickCount = 0;
      // Consume each menu press until release, including page changes.
      longPressHandled = true;
      resetEncoderPosition();
      if (songListOpen) {
        if (songSelection == 0) {
          songListOpen = false;
          showMenu(menuSelection);
        } else {
          menuOpen = false;
          songListOpen = false;
          selectTrack(songSelection - 1);
        }
      } else if (menuSelection == 0) {
        menuOpen = false;
        showCurrentTrack();
      } else {
        songListOpen = true;
        songSelection = 0;
        showSongList(songSelection);
      }
      lastButtonState = button;
      return;
    }
  }

  // Long press detection after 1 second
  if (button == LOW && !longPressHandled && (millis() - buttonPressTime > 1000)) {
    longPressHandled = true;
    menuOpen = true;
    songListOpen = false;
    menuSelection = 0;
    resetEncoderPosition();
    showMenu(menuSelection);
    clickCount = 0;
  }

  if (button == HIGH && lastButtonState == LOW) {
    if (longPressHandled) {
      longPressHandled = false;
    } else if (!menuOpen) {
      // Count short clicks on release; a hold must not pause playback first.
      if (clickCount == 0) firstClickTime = millis();
      clickCount++;
    }
  }

  lastButtonState = button;

  // Wait 400ms to see if there is another click
  if (button == HIGH && !menuOpen && !longPressHandled && clickCount > 0 && (millis() - firstClickTime) > 400) {
    if (clickCount == 1) {
      pauseResume();
      Serial.println("Pause/Play");
    } else {
      skipToRandom();
      Serial.println("Skip");
    }
    clickCount = 0;
  }
}

void IRAM_ATTR encoderISR() {
  uint8_t currentState = (digitalRead(encoderClk) << 1) | digitalRead(encoderDt);
  uint8_t transition = (lastEncoderState << 2) | currentState;

  // Only count valid encoder movements
  switch (transition) {
    case 0b0001: case 0b0111: case 0b1110: case 0b1000:
      encoderCounter--;
      break;
    case 0b0010: case 0b1011: case 0b1101: case 0b0100:
      encoderCounter++;
      break;
  }

  lastEncoderState = currentState;
}

void checkEncoder() {
  noInterrupts();
  int current = encoderCounter;
  interrupts();

  if (current != lastEncoderCounter) {
    int difference = current - lastEncoderCounter;
    int clicks = difference / 4;

    if (clicks != 0) {
      if (menuOpen) {
        if (songListOpen) {
          int next = constrain(songSelection + clicks, 0, getTrackCount());
          if (next != songSelection) {
            songSelection = next;
            showSongList(songSelection);
          }
        } else {
          int next = constrain(menuSelection + clicks, 0, 1);
          if (next != menuSelection) {
            menuSelection = next;
            showMenu(menuSelection);
          }
        }
      } else {
        changeVolume(clicks);
        drawVolume(getVolume());
      }

      // Keep any unfinished encoder steps for the next check
      lastEncoderCounter += clicks * 4;
    }
  }
}
