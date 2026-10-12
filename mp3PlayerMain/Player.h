#pragma once

bool setupPlayer();
void startPlayer();
void updatePlayer();
bool getPlayerReady();
void skipToRandom();
void pauseResume();
void showCurrentTrack();
void changeVolume(int amount);
int getVolume();
int getTrackCount();
const char* getTrackPath(int index);
void selectTrack(int index);
