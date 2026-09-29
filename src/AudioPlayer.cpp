#include "AudioPlayer.h"
extern AudioPlayer audioPlayer;
void AudioPlayer::begin(){audio.setPinout(Config::I2S_BCLK,Config::I2S_LRC,Config::I2S_DOUT);audio.setVolume(volume);Serial.println("Audio initialised");}
void AudioPlayer::update(){audio.loop();}
void AudioPlayer::play(const char* path){paused=false;trackStarted=millis();noInterrupts();trackEnded=false;interrupts();audio.connecttoFS(SD,path);Serial.print("Playing: ");Serial.println(path);}
void AudioPlayer::togglePause(){audio.pauseResume();paused=!paused;}
void AudioPlayer::setVolume(int v){volume=constrain(v,Config::MIN_VOLUME,Config::MAX_VOLUME);audio.setVolume(volume);}
int AudioPlayer::getVolume()const{return volume;}
bool AudioPlayer::isPaused()const{return paused;}
bool AudioPlayer::consumeTrackEnded(){noInterrupts();bool ended=trackEnded;trackEnded=false;interrupts();return ended;}
void AudioPlayer::onEndOfFile(){if(millis()-trackStarted<Config::EOF_GUARD_MS){Serial.println("(ignoring spurious EOF)");return;}trackEnded=true;}
void audio_eof_mp3(const char* info){(void)info;audioPlayer.onEndOfFile();}
