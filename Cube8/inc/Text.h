/*
        RGB CUBE Animations MK4 
        Nick Schulze September 2013
        HowNotToEngineer.com
*/

#ifndef TEXT_h
#define TEXT_h

#include "System.h"
#include "ESP8266WIFI.h"
#include "graphics.h"
#include "ColourWheel.h"
#include "rtc.h"

#define MAX_WORD_LENGTH 20
#define NUM_WORDS 1
#define NUM_WORDS7 1
#define NUM_WORDS_SCROLL 2
#define NUM_CHARS_SCROLL 7
#define letter(x) (x - 'A') //ascii 65
#define letter7(x) (x - '.') //ascii 46

extern uint8_t scrollFinished;

void beginText(void);
void animateText(void);
void writeChar(uint8_t c,uint8_t mode);
uint8_t scrollReadText(char currentLetter);
void beginText7(void);
void animateText7(void);
void animateIT(void);
void scrollText(uint8_t number);
void initScrollText(uint8_t param);
void scrollTime(uint8_t number);
void scrollDateTime(uint8_t number);
void printClock(void);

#endif
