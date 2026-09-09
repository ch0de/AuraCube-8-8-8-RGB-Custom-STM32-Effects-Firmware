#ifndef __SYSTEM_H
#define __SYSTEM_H

#include "stm32f10x.h"
#include "LED.h"
#include "LEDHardware.h"
#include "timer.h"
#include "adc.h"
#include "fft.h"
#include "CubeLink.h"
#include "usart.h"
#include "ir_nec.h"
#include "W25Qxx.h"
#include "graphics.h"

#include "malloc.h"  
#include "exfuns.h"
#include "MMC_SD.h" 
#include "stdbool.h"
#include "cubefile.h"	 
#include "ff.h"	
#include "ESP8266Wifi.h"
#include "Softwire.h"
#include "rtc.h"
#include "lcd.h"
#include "bme280.h"
#include "sht31.h"

#define USB_USART_REC_LEN	 	1//80 //256	//CHANGE
//#ifndef doUSB
//#define doUSB
//#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/*#ifndef BOOL
#define BOOL
typedef enum
{
  FALSE = 0, TRUE  = !FALSE
}
bool;
#endif
*/

void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);
void TIM4_IRQHandler(void);
void TIM5_IRQHandler(void);
void delay_STms(uint32_t num);
void TF_Init(void);
void Sytem_Star(void);
void usb_printf(char* fmt, ...);

/* Cube8 smooth-animation double-buffer support.
 * Main code renders into a hidden buffer and requests a swap.
 * TIM2 applies the swap only at the start of a complete PWM cycle so a
 * displayed PWM frame never contains a mixture of two animation frames.
 */
extern volatile uint8_t requestedShowLED;
void requestShowLEDSwap(uint8_t buffer);

#endif 



