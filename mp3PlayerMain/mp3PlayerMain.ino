//Simple MP3 Player v1.1
//jamxsbart

#include <Arduino.h>
#include "Controls.h"
#include "Display.h"
#include "Player.h"

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n--- Simple Player ---");

  setupDisplay();
  showLoading();

  if (!setupPlayer()) return;

  setupControls();
  startPlayer();
}

void loop() {
  if (!getPlayerReady()) {
    delay(20);
    return;
  }

  updatePlayer();
  updateControls();
}
