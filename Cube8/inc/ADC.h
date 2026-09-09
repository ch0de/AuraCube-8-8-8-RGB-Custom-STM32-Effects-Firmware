/*******************************************************************************
 * @file     :☆☆☆彩色光立方☆☆☆
 * @brief    :ADC
 * @mcu      :STM32F103RCT6
 * @version  :V1.0
 * @author   :创优物联工作室→图腾
 * @date     :2015-6-6
*******************************************************************************/
#ifndef _ADC_H
#define _ADC_H

#include "System.h"
#include "stm32f10x.h"

void fft_ADC_Init(void);															//对fft采集的数据进行ADC的初始化
void fft_AuraADC_Init(void);
uint16_t Get_ADC(uint8_t ch); 														//获取ADC的数据  

#endif

