#ifndef _TIME_H
#define _TIME_H

#include "stm32f10x.h"
void Timer1_Init(uint16_t psc,uint16_t arr);
void Timer2_Init(uint16_t psc,uint16_t arr);
void Timer3_Init(uint16_t psc,uint16_t arr);
void Timer4_Init(uint16_t psc,uint16_t arr);

#endif
