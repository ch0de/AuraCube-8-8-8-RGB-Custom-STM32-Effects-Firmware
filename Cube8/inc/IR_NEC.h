
#ifndef _IR_NEC_H
#define _IR_NEC_H

#include "System.h"
#include "IR.h"
#include "LED.h"
#include "W25Qxx.h"

extern unsigned char NEC[4];

void IR_ENC_Init(uint16_t psc);

void IR_NEC(void);


#define  Step 300		

extern unsigned char IR_BT;

#endif




