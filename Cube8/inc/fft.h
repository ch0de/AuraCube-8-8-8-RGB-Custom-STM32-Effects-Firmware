#ifndef _fft_H
#define _fft_H

#include "System.h"
#include "fft_tab.h"
#include "math.h"
#include "adc.h"
#include "LED.h"
#include "LEDHardware.h"

typedef struct									//傅里叶变换 输入和输出缓存区
{
	float real;
	float imag;
}compx;

//#define fft_64 																		//64点的fft 
#define fft_128 																		//128点的fft
//#define fft_256 																		//256点的fft
//#define fft_512 																		//512点的fft 
//#define fft_1024 																		//1024点的fft

#ifdef fft_64
#define fft_N 64
#endif
#ifdef fft_128
#define fft_N 128
#endif
#ifdef fft_256
#define fft_N 256
#endif
#ifdef fft_512
#define fft_N 512
#endif
#ifdef fft_1024
#define fft_N 1024
#endif	 

extern compx fftData[fft_N];    														//fft数据段 
extern uint16_t fftIn[fft_N];    														//fft输入数据
extern uint8_t fftOut[fft_N / 2];    													//fft数据输出 
extern uint8_t sqrt_M(uint16_t temp);													//fft计算出M值

#define M sqrt_M(fft_N)																	//求得M的值 

void fft_Convert(compx *xin);															//N点fft计算函数,fft计算转换处理函数
void fft_powerMag(void);																//fft计算出幅值函数，fftOut数组输出
void fft_Data(void);																	//进行fft的ADC的数据采集
void fft_Display(void);																	//fft计算处理之后的显示函数
void SHOW_FFT(void);

extern uint8_t fft_MODE;
extern uint8_t fftPlasma;

#endif




