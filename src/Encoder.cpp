#include "Encoder.h"
#include "Config.h"
volatile int Encoder::counter=0; volatile uint8_t Encoder::previousState=0;
void Encoder::begin(){pinMode(Config::ENC_CLK,INPUT_PULLUP);pinMode(Config::ENC_DT,INPUT_PULLUP);previousState=(digitalRead(Config::ENC_CLK)<<1)|digitalRead(Config::ENC_DT);counter=0;lastCounter=0;attachInterrupt(digitalPinToInterrupt(Config::ENC_CLK),isr,CHANGE);attachInterrupt(digitalPinToInterrupt(Config::ENC_DT),isr,CHANGE);}
int Encoder::consumeClicks(){noInterrupts();int current=counter;interrupts();int clicks=(current-lastCounter)/4;if(clicks)lastCounter+=clicks*4;return clicks;}
void IRAM_ATTR Encoder::isr(){uint8_t current=(digitalRead(Config::ENC_CLK)<<1)|digitalRead(Config::ENC_DT);uint8_t transition=(previousState<<2)|current;switch(transition){case 0b0001:case 0b0111:case 0b1110:case 0b1000:--counter;break;case 0b0010:case 0b1011:case 0b1101:case 0b0100:++counter;break;default:break;}previousState=current;}
