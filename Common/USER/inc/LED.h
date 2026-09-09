#ifndef _LED_H
#define _LED_H

#include "System.h"
#include "stdlib.h"
#include "graphics.h"
#include "lcd.h"

extern uint8_t Cube_MODE;													//Cube Mode
extern uint8_t OldCube_MODE;
extern uint8_t AuraCubes;
extern uint8_t cubeWIFI; 
extern uint8_t hasSD;
extern uint8_t hasWIFI;
extern uint8_t hasUSB;
extern uint8_t Cube_PROGRAM;
extern uint8_t cubeDirection;
extern uint8_t programCounter;
//extern uint8_t program[100][3];	
extern uint16_t NowSection;
extern uint32_t randCount;
extern int16_t SpeedSum,NumCount;
extern uint8_t Color_red,Color_green,Color_blue;
extern uint8_t light_red,light_green,light_blue;

extern char wifiSSID[32];															
extern char wifiPSWD[32];
extern char myIPAddress[16];
extern char myGW[16];
extern char myMask[16];
extern char NTPServer[32];
extern char SDDateTime[14];
extern int8_t UTCOffsetH;                               // Hour Offset of local time from UTC Time
extern int8_t UTCOffsetM;                               // Minute Offset of local time from UTC Time
extern uint8_t dSavings;
extern uint8_t dateFormat;
extern uint8_t weekDay;
extern uint8_t hasNTP;
extern uint8_t hasDS3231;
extern uint8_t hasLCD;																	// Address of LCD if we have one
extern uint8_t hasBME280;                              // Address of BME280 sensor if we have one
extern uint8_t hasSHT31;                               // Address of SHT31 sensor if we have one
extern uint8_t has24cxx;																// Do we have a 24cxx eeprom
extern volatile uint8_t inI2c;  
extern uint8_t LCDType;
extern uint8_t tempUnit;

extern char date[12];
extern char time[10];

extern volatile float temperature;
extern volatile float humidity;
extern volatile float altitude;

extern volatile uint8_t pressureReads;
extern volatile float firstPressure;
extern volatile float pressure;

extern uint8_t wifiTimeoutMult;
extern uint8_t ESP8266CHANGE;
extern uint32_t DMABAUD; // Speed of bus between cubes over USART
extern uint8_t getDMA;
extern uint8_t exitDMA;
extern uint8_t DMAAddress;
extern uint8_t DMACommands[10];

extern uint8_t extendCubes; // each cube is addressed separately
extern uint8_t swapCubes;   // swap the positions of the cubes
extern uint8_t outOfBounds;
extern uint8_t numCubes;
extern uint8_t hostReAnnounce;
extern uint8_t hostCopyMode;
extern uint8_t WIFICopyMode;
extern volatile uint32_t old_millis;
extern volatile uint32_t WIFI_old_millis;

extern uint8_t showLED;
extern uint8_t setLED;
extern uint8_t power;
extern uint8_t delaySpeed;
extern uint8_t customBright;
extern uint8_t textColourMode;
extern uint8_t textColourRandom;
extern uint8_t textMode;

extern uint8_t setAdd;

extern uint8_t shared2; // shared random variables to make sure synced cubes on wifi has same random numbers
extern uint8_t shared3;
extern uint8_t shared4;
extern uint8_t shared5;
extern uint8_t shared6;
extern uint8_t sharedA;
extern uint8_t sharedB;
extern uint8_t sharedC;

extern uint8_t ROffset;                                  // Offset of Red in array. Can be used to swap colours
extern uint8_t GOffset;                                  // Offset of Green in array. Can be used to swap colours
extern uint8_t BOffset;                                  // Offset of Blue in array. Can be used to swap colours

typedef struct 
{
	uint8_t R;
	uint8_t G;
	uint8_t B;
} colour;

void ClearRGB(char Crgb,int8_t rank);									
void AxesConvert(void);													
void setPlaneX(uint8_t x, uint8_t R, uint8_t G, uint8_t B);
void setPlaneY(uint8_t y, uint8_t R, uint8_t G, uint8_t B);
void setPlaneZ(uint8_t z, uint8_t R, uint8_t G, uint8_t B);
void clear(uint8_t x,uint8_t y, uint8_t z);
void setAll(uint8_t R, uint8_t G, uint8_t B);
void setMyAll(uint8_t R, uint8_t G, uint8_t B);
void delay(uint32_t us);
uint32_t millis(void);
void dimmer(uint8_t R, uint8_t G, uint8_t B, uint8_t *oR, uint8_t *oG, uint8_t *oB, float percent);
void brighter(uint8_t R, uint8_t G, uint8_t B, uint8_t *oR, uint8_t *oG, uint8_t *oB, float percent);
void hueToRGB(int hue, int brightness, uint8_t *oR, uint8_t *oG, uint8_t *oB);
void hsvToRGB(unsigned int hue, unsigned int sat, unsigned int val, \
              unsigned char * r, unsigned char * g, unsigned char * b, unsigned char maxBrightness );
int isClear(uint8_t x,uint8_t y,uint8_t z);
int isClearBuffer(uint8_t buffer, uint8_t x,uint8_t y,uint8_t z);
int isClearAll(void);
int isClearAllBuffer(uint8_t buffer);
int isOnCube(int8_t x, int8_t y, int8_t z);
uint8_t onThisCube(uint8_t x,uint8_t y,uint8_t z);
void nextColour(uint8_t *r,uint8_t *g,uint8_t *b, uint8_t ran);
	
#define BLACK  0x00, 0x00, 0x00
#define BLUE   0x00, 0x00, 0xff
#define GREEN  0x00, 0xff, 0x00
#define ORANGE 0xff, 0x45, 0x00
#define PINK   0xff, 0x14, 0x44
#define PURPLE 0xff, 0x00, 0xff
#define RED    0xff, 0x00, 0x00
#define WHITE  0xff, 0xff, 0xff
#define YELLOW 0xff, 0xff, 0x00
#define OLIVE  0x66, 0xcc, 0x33
#define TURQUOISE 0x00, 0xce, 0xd1
#define AQUA   0x33, 0xcc, 0x99
#define GRAY   0x44, 0x44, 0x44
#define CYAN   0x00, 0xff, 0xff
#define BROWN  0x4e, 0x64, 0x2f

#endif

