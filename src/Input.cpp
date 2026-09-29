#include "Input.h"
#include "Config.h"
void Input::begin(){pinMode(Config::ENC_SW,INPUT_PULLUP);}
ButtonEvent Input::update(){bool pressed=digitalRead(Config::ENC_SW)==LOW;uint32_t now=millis();if(pressed&&!lastPressed){pressStart=now;longPressHandled=false;if(!clickCount)firstClick=now;++clickCount;}if(pressed&&!longPressHandled&&now-pressStart>=Config::LONG_PRESS_MS){longPressHandled=true;clickCount=0;lastPressed=pressed;return ButtonEvent::LongPress;}if(!pressed&&lastPressed&&longPressHandled){longPressHandled=false;lastPressed=pressed;return ButtonEvent::None;}lastPressed=pressed;if(clickCount&&now-firstClick>=Config::CLICK_TIMEOUT_MS){ButtonEvent e=clickCount==1?ButtonEvent::SingleClick:ButtonEvent::DoubleClick;clickCount=0;return e;}return ButtonEvent::None;}
