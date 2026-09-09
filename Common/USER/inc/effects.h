#ifndef __effects_H__
#define __effects_H__

#include "System.h"
#include "LED.h"
#include "LEDHardware.h"
#include "graphics.h"
#include "string.h"

extern float fadeMax;	
extern float fadeLevel;	
extern uint8_t inFadeMax;
extern uint8_t transitionSpeed;
extern uint8_t doTransitions;
extern uint8_t slideLevel;

extern uint8_t slideDirX;
extern uint8_t slideDirY;
extern uint8_t slideDirZ;

void flash(void);
void doEffect(void);
void randomFade(void);
void setEffect(const char *e, int interval1, int interval2, float increment);
void copyBuffer(uint8_t from, uint8_t to);
	
#endif
