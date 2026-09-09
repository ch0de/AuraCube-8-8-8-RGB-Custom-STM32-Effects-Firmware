#include "fft_tab.h"
#include "fft_list.h"
#include "fft.h"
#include "LEDHardware.h" // Assuming this header file is needed for ledCubeType, etc.

compx fftData[fft_N]; 																	//fftݶ
uint16_t fftIn[fft_N];    																//fft
uint8_t fftOut[fft_N / 2];    															//fft

uint8_t fft_MODE = 0;
uint8_t fftPlasma=0;

/*******************************************************************************
 * @name     :uint8_t sqrt_M(uint16_t temp)
 * @author   :ͼʵҡͼ
 * @details  :fftMֵ
*******************************************************************************/
uint8_t sqrt_M(uint16_t temp)
{
	uint8_t m = 0;
	for (m = 1; (temp = temp / 2) != 1; m++);												//Mֵ
	return m;
}

//m^n
//uint16_t mypow(uint8_t m,uint8_t n)
//{
//	uint16_t result = 1;	 
//	while(n--)
//		result *= m;    
//	return result;
//}   

/*******************************************************************************
 * @name     :void fft_Convert(compx *xin)
 * @author   :ͼʵҡͼ
 * @details  :fftת
*******************************************************************************/
//xin:СΪN+1
void fft_Convert(compx *xin)
{
	uint16_t i, j, L;
	uint16_t p;
	uint16_t le, B, ip;
	compx t;
	compx ws;

	for (L = 1; L <= M; L++) 																//fft  Ҷ任
	{
		le = (2 << L) >> 1;
		B = le / 2;
		for (j = 0; j <= B - 1; j++)
		{
			p = (((2 << M) >> L) >> 1)*j;
			ws.real = cos_tab[p];
			ws.imag = sin_tab[p];
			for (i = j; i <= fft_N - 1; i = i + le)											//Mеĵ
			{
				ip = i + B;
				//ִи˷
				t.real = xin[ip].real*ws.real - xin[ip].imag*ws.imag;
				t.imag = xin[ip].real*ws.imag + xin[ip].imag*ws.real;
				xin[ip].real = xin[i].real - t.real;
				xin[ip].imag = xin[i].imag - t.imag;
				xin[i].real = xin[i].real + t.real;
				xin[i].imag = xin[i].imag + t.imag;
			}
		}
	}
}


#define FFT_SHOW_MIN	10			//ʾfftСֵ
#define FFT_Compact		1			//ʾfftѹ

/*******************************************************************************
 * @name     :void fft_powerMag(void)
 * @author   :ͼʵҡͼ
 * @details  :fftֵfftOut
*******************************************************************************/
void fft_powerMag(void)
{
	int i;
	int temp = 0;
	for (i = 1; i < fft_N / 2; i++)															//ƵķֵԳԣһͺ
	{																						//任ģֵ
		fftData[i].real = sqrt(fftData[i].real * fftData[i].real + fftData[i].imag * fftData[i].imag)
			/ (i == 0 ? fft_N : (fft_N / 2));


		temp = (uint8_t)(8.0f * log10(fftData[i].real));

		if(temp <= FFT_SHOW_MIN)
			temp = 1;
		else
			temp = temp - FFT_SHOW_MIN;

		if (ledCubeType==0) 
		{
			temp = (uint8_t)fftData[i].real; 	//RGB CUBE8 no amplifier
			if(temp <= 1) temp = 1;
		}
		if(temp >= fftOut[i])
			fftOut[i] = temp;
		else
		{
			if(fftOut[i] > 0)
				fftOut[i]--;
		}
	}
}

/*******************************************************************************
 * @name     :void fft_Data(void)
 * @author   :ͼʵҡͼ
 * @details  :fftADCݲɼ
*******************************************************************************/
void fft_Data(void)
{
	int i = 0;
	for (i = 0; i < fft_N; i++)
	{
		fftData[List[i]].real = fftIn[i];													//ݸƵfftļȥòķʽٶ
		fftData[List[i]].imag = 0;															//鲿0
	}
}

const uint8_t sumfft[26] ={27,35,36,28,0,8,16,24,32,40,48,56,57,58,59,60,61,62,63,55,47,39,31,23,15,7};

/*******************************************************************************
 * @name     :void fft_Display(void)
 * @author   :ͼʵҡͼ
 * @details  :fft㴦֮ʾ
*******************************************************************************/
void fft_Display(void)
{
	char i,j;
	int layerB;

	for(j = 0; j < 8; j++)											//ж8ǷҪ
	{
		layerB = (7 - j) * 192;
		for(i = 0; i < 26; i++)										//ʾ64
		{
			if(fftOut[(uint8_t)i + 2] > (uint8_t)j)									//Ҫĵ
			{
				if(i < 4)
				{
					rgbled[setLED][rgb_order[3 * sumfft[(uint8_t)i] + ROffset] + layerB] = 220;
					rgbled[setLED][rgb_order[3 * sumfft[(uint8_t)i] + GOffset] + layerB] = 0;
					rgbled[setLED][rgb_order[3 * sumfft[(uint8_t)i] + BOffset] + layerB] = 50;
				}	
				else
				{
					rgbled[setLED][rgb_order[3 * sumfft[(uint8_t)i] + ROffset] + layerB] = 220;
					rgbled[setLED][rgb_order[3 * sumfft[(uint8_t)i] + GOffset] + layerB] = 50;
					rgbled[setLED][rgb_order[3 * sumfft[(uint8_t)i] + BOffset] + layerB] = 0;
				}
			}				
			else
			{
				rgbled[setLED][rgb_order[3 * sumfft[(uint8_t)i] + ROffset] + layerB] = 0;
				rgbled[setLED][rgb_order[3 * sumfft[(uint8_t)i] + GOffset] + layerB] = 0;
				rgbled[setLED][rgb_order[3 * sumfft[(uint8_t)i] + BOffset] + layerB] = 0;
			}				
		}			
	}
}

void SHOW_FFT(void)
{
	fft_Data();								
	fft_Convert(fftData);	
	fft_powerMag();	
	fft_Display();	
}
