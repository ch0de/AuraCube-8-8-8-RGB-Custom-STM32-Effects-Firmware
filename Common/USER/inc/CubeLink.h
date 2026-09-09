#ifndef _CUBELINK_H
#define _CUBELINK_H

#include "System.h"
#include "LED.h"

void checkDMA(void);
void startDMA(void);
void stopDMA(void);
void setOtherCubesDMA(uint8_t address, uint8_t command);
void calculateTotalSize(void);
void populateDMAGrid(uint8_t populate);
void setupDMA(void);
void stopDMATransfer(void);
void hostAnnounce(void);
void SlaveIRQ(char c);
void USARTIRQ(void);
uint16_t getDMASize(uint8_t cube);
void getSize(uint8_t cube, uint8_t *tx, uint8_t *ty, uint8_t *tz);

extern uint8_t nextDMA;
extern uint8_t inDMA;
extern uint8_t inSendDMA;
extern uint8_t sendDMA;
extern uint8_t DMAPointer;  // next one to send
volatile extern uint8_t foundHost; // Is there a host cube ?
volatile extern uint8_t oldFoundHost;
extern uint8_t DMAAddresses[10];
extern uint8_t DMASizes[10]; // Other Cubes Sizes
extern uint16_t DMASize;
extern USART_TypeDef* DMAUSART;
volatile extern uint8_t DMATranslate;

#endif








