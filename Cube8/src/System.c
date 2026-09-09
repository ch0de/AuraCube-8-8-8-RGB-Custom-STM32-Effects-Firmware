#include "System.h"
#include "stm32f10x_it.h"
#include "graphics.h"
#include "exfuns.h"
#include "W25Qxx.h"
#include "fft.h"
#include "ESP8266WIFI.h"
#include "CubeLink.h"

u8  USART_PRINTF_Buffer[USB_USART_REC_LEN];
#define TIMERSR 	(TIM_IT_Update ^ 0xffff)

const uint8_t DigitChar[8] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
const uint16_t ScanTimeData[8] = {ScanTime << 0,ScanTime << 1,ScanTime << 2,ScanTime << 3,
								  ScanTime << 4,ScanTime << 5,ScanTime << 6,ScanTime << 7};

#define	LED_ON   	GPIOB->BSRR = 1 << 6   
#define LED_OFF 	GPIOB->BRR = 1 << 6  
					
volatile uint8_t counterTimer=0;
volatile uint16_t counter2=0;

/* 0xFF means no display-buffer swap is waiting. */
volatile uint8_t requestedShowLED = 0xFF;

/* Defined in main.c.  Used only to isolate the experimental diagnostic
 * scan ordering from the normal display modes. */
extern volatile uint8_t cubeEffectMode;

void requestShowLEDSwap(uint8_t buffer)
{
    requestedShowLED = buffer;
}
									
void rgbTIM2(void)
{
	static uint8_t Layer = 0;																	//LEDĲɨ
	static uint8_t Digit = 0;																	//ʾʱ+λ
	if(TIM_GetITStatus(TIM2,TIM_IT_Update) != RESET) 											//ָTIMжϷ:TIM жԴ 
    {
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);  											//TIMxжϴλ:TIM жԴ 
		
		if(++Layer >= 8)
		{
			Layer = 0;
			if(++Digit >= 8)
			{
				Digit = 0;

				/* Swap only here: Layer 0 / PWM bit 0 is the start of a
				 * complete 8-bit PWM refresh cycle.  This prevents tearing. */
				if (requestedShowLED != 0xFF)
				{
					showLED = requestedShowLED;
					requestedShowLED = 0xFF;
				}
			}
			TIM_SetAutoreload(TIM2,ScanTime << Digit);
		}
		
		O_OE_SET; 																				//رղʾʹܶ
		rgb_OE_SET;																				//رݵʹܶ
    Anode(1 << Layer);																		//ѡ
    Send_Data(Digit,Layer);

        /* v2 PWM timing fix:
         * The original code lets TIM2 count while the 192 RGB bits are being
         * shifted.  On the short low-order PWM planes the timer can expire
         * before the LEDs are even enabled, effectively deleting the LSBs.
         * Start the programmed dwell interval NOW, after the shift/latch. */
        TIM_SetCounter(TIM2, 0);
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		O_OE_RESET;																				//򿪲ʾʹܶ	
		rgb_OE_RESET;																			//ݵʹܶ
		
		randCount++;																			//
	}	
}

/* Aura blanking delays.  Keep the v3 short blanking behavior for the normal
 * effects that are already working well.  Diagnostic mode deliberately uses
 * a much longer delay so we can test whether the remaining phantom pixels /
 * layers are caused by settling around the HC138 and RGB latch.
 *
 * At ~72 MHz, 96 NOPs is roughly 1.3 us plus loop overhead.  384 NOPs is
 * intentionally exaggerated (~5 us plus loop overhead) for diagnosis. */
#define AURA_LAYER_BLANK_NOPS       96U
#define AURA_DIAGNOSTIC_BLANK_NOPS 384U
static inline void AuraLayerBlankDelay(void)
{
    uint16_t n;
    for (n = 0U; n < AURA_LAYER_BLANK_NOPS; n++)
        __asm volatile ("nop");
}

static inline void AuraDiagnosticBlankDelay(void)
{
    uint16_t n;
    for (n = 0U; n < AURA_DIAGNOSTIC_BLANK_NOPS; n++)
        __asm volatile ("nop");
}

void AuraTIM2(void)
{
	static uint8_t Layer = 0;							//LED??????
	static uint8_t Digit = 0;							//???????+??
	if ((TIM2->SR & TIM_IT_Update) != (uint16_t)RESET && 
		(TIM2->DIER & TIM_IT_Update) != (uint16_t)RESET)	//?????TIM??????:TIM ??? 
	{
		TIM2->SR = TIMERSR;								//??TIMx???????:TIM ??? 

		if (++Layer >= 8)
		{
			Layer = 0;
			if (++Digit >= 8)
			{
				Digit = 0;
				if (requestedShowLED != 0xFF)
				{
					showLED = requestedShowLED;
					requestedShowLED = 0xFF;
				}
			}
			TIM2->ARR = ScanTimeData[Digit];
		}

		rgb_OE_SET;										// blank RGB outputs

        if (cubeEffectMode == 3U)
        {
            /* Diagnostic-only ghosting test.  Keep the OLD HC138 layer
             * selected while shifting/latching the NEXT layer's RGB data.
             * Because OE is blanked, neither the old nor the new pattern is
             * visible.  Only after the RGB outputs are stable do we change
             * the HC138 address, wait for it to settle, and enable LEDs. */
            AuraDiagnosticBlankDelay();             // make sure OE is fully off
            Send_DataAura(DigitChar[Digit], Layer); // shift/latch next data first
            AuraDiagnosticBlankDelay();             // let RGB latch settle
            AnodeAura(7 - Layer);                    // now select new physical layer
            AuraDiagnosticBlankDelay();             // let HC138/layer drive settle
        }
        else
        {
            /* Preserve the v3 scan sequence for the working hue/cloud/manual
             * modes. */
            AuraLayerBlankDelay();                   // let OE fully turn off
            AnodeAura(7 - Layer);                    // select new physical layer
            Send_DataAura(DigitChar[Digit], Layer); // shift and latch RGB data
            AuraLayerBlankDelay();                   // settle address/latch while blank
        }

		rgb_OE_RESET;									// enable RGB outputs

		randCount++;									//????????
	}
}

/*******************************************************************************
 * @name     :void TIM2_IRQHandler(void) 
 * @brief    :
 * @param    :
 * @return   :
 * @version  :V1.0
 * @author   :ҡͼ
 * @date     :2015-6-6
 * @details  :TIM2жϷ
 *            
*******************************************************************************/
void TIM2_IRQHandler(void)   								
{
  if (ledCubeType==0) rgbTIM2();  
  if (ledCubeType==1) AuraTIM2();	
}

/*******************************************************************************
 * @name     :void TIM3_IRQHandler(void) 
 * @brief    :
 * @param    :
 * @return   :
 * @version  :V1.0
 * @author   :ҡͼ
 * @date     :2015-6-6
 * @details  :TIM3жϷ
 *            
*******************************************************************************/
void TIM3_IRQHandler(void)   								
{
//	static uint16_t flagCount = 0;
//	static uint8_t tf_num = 0;
	
	if(TIM_GetITStatus(TIM3,TIM_IT_Update) != RESET) 											//ָTIMжϷ:TIM жԴ 
  {
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  											//TIMxжϴλ:TIM жԴ 	
		if (power==1)
		{
			switch(Cube_MODE)																		//ѡʾģʽ
			{
				case 0:
					if (ledCubeType==0 && !hasSD) ReadW25Q64();		 // read from Memory Chip if no SD Card
					break;
				case 1:
					break;
				case 2:
          controlGraphics();
				case 3:																				//ҹģʽ
					break;
				case 9:
					if (ledCubeType==0) ReadW25Q64();																//ʾƶ
					break;
			}
		}
		else 
		{
			//setLED=0;
			//showLED=0;
			//clearAll();
		}
		//if (hasWIFI==4) {esp8266CheckServer();}
		//if (DMAAddress==0) {checkDMA();} // If HOST
	}
}

/*******************************************************************************
 * @name     :void TIM4_IRQHandler(void) 
 * @brief    :
 * @param    :
 * @return   :
 * @version  :V1.0
 * @author   :ҡͼ
 * @date     :2015-6-6
 * @details  :TIM4жϷ
 *            
*******************************************************************************/
void TIM4_IRQHandler(void)   								
{
	static uint16_t fft_Num = 0;
	if ((TIM4->SR & TIM_IT_Update) != (uint16_t)RESET && 
		(TIM4->DIER & TIM_IT_Update) != (uint16_t)RESET) 	//?????TIM??????:TIM ??? 
	{
		TIM4->SR = TIMERSR;									//??TIMx???????:TIM ??? 

		if (Cube_MODE == 1)
		{
			if (ledCubeType==0) fftIn[fft_Num] = Get_ADC(ADC_Channel_11);  // Channel 11 for CUBE8
			if (ledCubeType==1)
			{
			if(ADC_MODE)
				fftIn[fft_Num] = Get_ADC(ADC_Channel_2);
			else
				fftIn[fft_Num] = Get_ADC(ADC_Channel_3);  // Channel 2 (line in) or 3 (mic) for AURA
			}
			if (++fft_Num > fft_N) fft_Num = 0;
		}
		counterTimer++;
		if (counterTimer>100)
		{
			if (hasWIFI==4) {esp8266CheckServer();}
			if (DMAAddress==0) {checkDMA();} // If HOST	
			counterTimer=0;
			counter2++;
		}
		if (modeC==89 && counter2%30==0 && counter2!=3000)
		{
			// get time more often
			if (hasDS3231) getRTCDateTime();
		}			
		if (counter2>3000)
		{
			// approx twice every minute
			doi2cUpdates();
			counter2=0;
		}
	}
}

void usb_printf(char* fmt, ...)
{
	// no longer used
	u16 i, j;
	va_list ap;
	va_start(ap, fmt);
	vsprintf((char*)USART_PRINTF_Buffer, fmt, ap);
	va_end(ap);
	i = strlen((const char*)USART_PRINTF_Buffer);					//?????????
	if (hasUSB)
	{
		for (j = 0; j < i; j++)											//??????
		{
//			if (ledCubeType==1) USB_USART_SendData(USART_PRINTF_Buffer[j]); // usb print for Aura Cube
//			else 
			 UU_PutChar(USART1,USART_PRINTF_Buffer[j]); // print to serial port for Cube8
		}
	}
	//if (hasLCD) lcd_println(99,1,fmt);	
}
