#pragma once

void setupDisplay();
void showLoading();
void showSdError();
void showNoSongs();
void showTrackLoadError();
void showNoPlayableSongs();
void showTrack(const char* path, int currentTrack, int trackCount, bool paused, int volume);
void showMenu(int selection);
void showSongList(int selection);
void drawVolume(int volume);
