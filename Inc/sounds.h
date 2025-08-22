#ifndef SOUNDS_H_
#define SOUNDS_H_

#include "main.h"

void playStartupTune(void);
void playInputTune(void);
void playBrushedStartupTune(void);
void playInputTune2(void);
void playBeaconTune3(void);
void playDuskingTune(void);
void playDefaultTone(void);
void playChangedTone(void);

void setVolume(uint8_t volume);

extern uint8_t beep_volume;


#endif /* SOUNDS_H_ */