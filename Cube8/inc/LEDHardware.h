#ifndef _LEDHARDWARE_H
#define _LEDHARDWARE_H

// Do NOT redefine 'random'. Use cube_random directly to avoid conflicts with libc.
// #define random cube_random

#include "System.h"
#include "stm32f10x.h"
#include "stdlib.h"
#include "graphics.h"
#include "W25Qxx.h"
#include "cubefile.h"
#include "ff.h"

// RGB CUBE

#define	rgb_OE_SET    	GPIOB->BSRR = 1<<15    
#define	rgb_ST_SET    	GPIOB->BSRR = 1<<14    
#define	rgb_CLK_SET   	GPIOB->BSRR = 1<<13    
#define	rgb_DATA_SET  	GPIOB->BSRR = 1<<12   

#define	O_CLK_SET   	GPIOC->BSRR = 1<<9    
#define	O_ST_SET    	GPIOC->BSRR = 1<<8    
#define	O_OE_SET    	GPIOC->BSRR = 1<<7    
#define	O_DATA_SET  	GPIOC->BSRR = 1<<6   

#define	rgb_OE_RESET    GPIOB->BRR = 1<<15    
#define	rgb_ST_RESET    GPIOB->BRR = 1<<14    
#define	rgb_CLK_RESET   GPIOB->BRR = 1<<13    
#define	rgb_DATA_RESET  GPIOB->BRR = 1<<12   

#define	O_CLK_RESET   	GPIOC->BRR = 1<<9    
#define	O_ST_RESET    	GPIOC->BRR = 1<<8    
#define	O_OE_RESET    	GPIOC->BRR = 1<<7    
#define	O_DATA_RESET  	GPIOC->BRR = 1<<6   

//??IO AURA CUBE???
#define	aura_OE_SET    		GPIOB->BSRR = 1 << 15    
#define	aura_ST_SET    		GPIOC->BSRR = 1 << 6    
#define	aura_CLK_SET   		GPIOC->BSRR = 1 << 7    
#define	aura_DAT_B_SET  	GPIOB->BSRR = 1 << 13   
#define	aura_DAT_A_SET  	GPIOB->BSRR = 1 << 12   

#define	HC138_A_SET   		GPIOA->BSRR = 1 << 8    
#define	HC138_B_SET    		GPIOC->BSRR = 1 << 9    
#define	HC138_C_SET    		GPIOC->BSRR = 1 << 8    

//??IO ???
#define	aura_OE_RESET    	GPIOB->BRR = 1 << 15    
#define	aura_ST_RESET    	GPIOC->BRR = 1 << 6    
#define	aura_CLK_RESET   	GPIOC->BRR = 1 << 7   
#define	aura_DAT_B_RESET  	GPIOB->BRR = 1 << 13   
#define	aura_DAT_A_RESET  	GPIOB->BRR = 1 << 12   

#define	HC138_A_RESET		GPIOA->BRR = 1 << 8    
#define	HC138_B_RESET		GPIOC->BRR = 1 << 9    
#define	HC138_C_RESET    	GPIOC->BRR = 1 << 8    

//#define ScanTime 		2	/* v2: faster PWM frame; dwell now starts after shifting */								
//#define ScanFosc 		90					
//#define ScanTime 		45
//#define ScanFosc 		300							
#define ScanTime 		2	/* v2: faster PWM frame; dwell now starts after shifting */
#define ScanFosc 		90		

#define ledArraySize 1536

extern int8_t debug;
extern uint8_t ledCubeType;
extern uint8_t bjCubeType;
extern uint8_t layerColourSame;

// 5 LED Fields
extern uint8_t rgbled[5][ledArraySize];		

extern uint8_t rgb_order[192];		
extern uint8_t rgb_order_transfer[192];			

#define layoutMaxX 3
#define layoutMaxY 3
#define layoutMaxZ 3

extern uint8_t cubeLayout[layoutMaxX][layoutMaxY][layoutMaxZ];

// Hack to redefine order array easily at run time
struct rgb_order_access
{
	uint8_t rgb_order_x[192];
};

extern uint8_t totalX; // Total Size of All Cubes
extern uint8_t totalY;
extern uint8_t totalZ;
extern uint8_t myTotalX; // Total Size of This Cube
extern uint8_t myTotalY;
extern uint8_t myTotalZ;
extern uint8_t myCentreX; // Centre of This Cube
extern uint8_t myCentreY;
extern uint8_t myCentreZ;
extern uint8_t curTotalX;
extern uint8_t curTotalY;
extern uint8_t curTotalZ;
extern uint8_t curCentreX;
extern uint8_t curCentreY;
extern uint8_t curCentreZ;
extern uint8_t trimTotalX;
extern uint8_t trimTotalY;
extern uint8_t trimTotalZ;
extern uint16_t totalPixels;
extern uint16_t myTotalPixels;
extern uint8_t totalMax; // The Biggest number of pixels in any one dimension
extern uint8_t totalMin; // The Smallest number of pixels in any one dimension
extern uint8_t mySize;   // Size of this cube

extern uint8_t ADC_MODE;
extern uint32_t ESP8266BAUD;
extern uint8_t timeMultipler;
extern uint8_t ADCRunning; // Not used on this cube

extern uint8_t musicPlay; // Not used on this cube
extern uint8_t currentPlay; // Not used on this cube
extern uint8_t musicPlaying; // Not used on this cube

void GPIO_Configuration(void);          						
void GPIO_AuraConfiguration(void);       
void NVIC_Configuration(void);				
void Anode(uint8_t temp);               							
void AnodeAura(uint8_t temp);               								
void Send_Data(uint8_t rgbBit,uint8_t layer);				
void Send_DataAura(uint8_t rgbBit,uint8_t layer);				
void Send_Data_Old(uint8_t rgbBit,uint8_t layer);						
void initRGBOrder(void);
void set(uint8_t x,uint8_t y,uint8_t z, uint8_t R, uint8_t G, uint8_t B);
uint8_t getR(uint8_t x,uint8_t y,uint8_t z);
uint8_t getG(uint8_t x,uint8_t y,uint8_t z);
uint8_t getB(uint8_t x,uint8_t y,uint8_t z);
void getRGB(uint8_t x,uint8_t y,uint8_t z,uint8_t *R,uint8_t *G,uint8_t *B);
void randomColour(uint8_t *gray_red,uint8_t *gray_green,uint8_t *gray_blue);	
uint32_t cube_random(uint8_t max);
uint32_t random32(uint32_t max);
u8 mf_read_to_dma(FIL *fp,uint8_t address);
void copyEffectBuffer(uint8_t to, uint8_t from, uint8_t extra,float level);
void copyDMAfromRGB(void);
void setLayerTimings(uint8_t y,uint8_t R,uint8_t G,uint8_t B);
void clearLayerTimings(void);
void copyDMABuffer(uint8_t addr, uint8_t from, uint8_t to);
void copyDMABuffers(uint8_t from, uint8_t to);
void checkUpdateDS3231(uint8_t weekday,uint8_t d,uint8_t m,uint8_t y,uint8_t localH,uint8_t localM,uint8_t localS);
void startGames(void);
void stopGames(void);
void processButton(uint8_t buttonNumber,uint8_t fire1,uint8_t fire2);
void delayBeat(uint32_t ms,uint32_t timeOut, uint8_t extraDelay);
void folderRunTune(uint8_t startIRQ,FRESULT res);
void stopTune(uint8_t stopIRQ);
void stopWave(void);
void playNext(void);
void stopSongs(void);
void checkMusic(void);
void executeComExtend(char* option, char* value);

#endif
