
#ifndef __graphics_H__
#define __graphics_H__

#include "System.h"
#include "effects.h"
#include "LED.h"
#include "LEDHardware.h"
#include "math.h"
#include "CubeFunction.h"
#include "ColourWheel.h"
#include "Text.h"

#define myPI  3.14159265358979323846
#define myDPI 1.2732395
#define myDPI2 0.40528473

extern float phase;
extern float vXpos,vYpos,vZpos;

extern uint32_t counter;	
extern uint32_t gCounter;
extern uint32_t counterNum;

extern uint8_t mf_dirOpen;
extern uint8_t shuffleGraphics;
extern uint16_t numSDPlay;
extern uint16_t numSDFiles;

extern uint32_t eCounter;
extern uint32_t eCounter2;
extern uint32_t eNum;
extern uint32_t eInterval1;
extern uint32_t eInterval2;
extern uint8_t eFadeout;
extern uint8_t eFadeoutDone;
extern uint32_t gCounterMax;
extern uint8_t welcome;
extern uint8_t allowModeWait;
extern uint8_t needToClearDMA;
extern uint8_t graphicOrder[30];
extern int8_t graphicOrderCounter; 
extern uint8_t graphicOrderMax;
extern uint8_t graphicOrderMaxTemp;
extern uint8_t modeNext;
extern uint8_t modeNextG; 
extern uint8_t modeText;
extern uint8_t msgWait;
extern uint8_t R, G, B;

extern uint8_t lifeStartParents;
extern uint8_t lifeMarryRate;
extern uint8_t lifeDeathRate;
extern uint8_t lifeChildYears;

extern uint8_t modeC;	
extern uint8_t modeMood;	
extern uint8_t modeMenu;
extern uint8_t modeTime;
extern uint8_t month;

extern char graphicName[20];
extern uint8_t modeWait;	
extern uint8_t modeWaitUp;	
extern int8_t rgb_dir;

typedef struct swarm {
   uint8_t hX;
   uint8_t hY;
   uint8_t hZ;
   uint8_t tX;
   uint8_t tY;
   uint8_t tZ;
   int8_t dirX;
   int8_t dirY;
   int8_t dirZ;
	 int8_t oDirX;
   int8_t oDirY;
   int8_t oDirZ;
	 int8_t o2DirX;
   int8_t o2DirY;
   int8_t o2DirZ;	
   uint8_t R;
   uint8_t G;
   uint8_t B;
} swarm;

void raster(uint8_t R, uint8_t G, uint8_t B, uint32_t speed, uint32_t loops);
void rasterChase(uint8_t R, uint8_t G, uint8_t B, uint32_t speed, uint32_t loops);
void setup(void);
void controlGraphics(void);
void nextGraphic(void);
void prevGraphic(void);
void gCleanup(void);
void specificGraphic(void);
void resetGraphics(void);
void changeCleanup(void);
void loop(void);
void loopMood(void);
void loopReadText(void);
void loopSlave(void);
void executeCommand(char* option, char* value);
uint8_t getNextGraphic(void);
void doPlasma(void);
void randomPastel(void);
void doTime(void);
void setName(char* str);
void invertColours(void);

#endif

