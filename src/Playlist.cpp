#include "Playlist.h"
bool Playlist::load(){ trackCount=0; File dir=SD.open("/library"); if(!dir||!dir.isDirectory()){if(dir)dir.close();return false;} while(trackCount<Config::MAX_TRACKS){File e=dir.openNextFile();if(!e)break;if(!e.isDirectory()&&isAudioFile(e.name()))addTrack(e.name());e.close();}dir.close();Serial.print("Found ");Serial.print(trackCount);Serial.println(" songs");return true;}
uint16_t Playlist::size()const{return trackCount;}
bool Playlist::empty()const{return trackCount==0;}
const Track* Playlist::get(int index)const{return index<0||index>=trackCount?nullptr:&tracks[index];}
int Playlist::randomIndexExcept(int currentIndex)const{if(!trackCount)return -1;if(trackCount==1)return 0;int i;do{i=random(0,trackCount);}while(i==currentIndex);return i;}
bool Playlist::isAudioFile(const char* n){size_t l=strlen(n);if(l<5)return false;const char* e=n+l-4;return strcasecmp(e,".mp3")==0||strcasecmp(e,".wav")==0;}
void Playlist::addTrack(const char* n){if(trackCount>=Config::MAX_TRACKS)return;snprintf(tracks[trackCount].path,Config::MAX_PATH_LENGTH,n[0]=='/'?"%s":"/library/%s",n);++trackCount;}
