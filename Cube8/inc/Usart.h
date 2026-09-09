/*******************************************************************************
 * @file     :☆☆☆彩色光立方☆☆☆
 * @brief    :串口的设置
 * @mcu      :STM32F103RCT6
 * @version  :V1.0
 * @author   :创优物联工作室→图腾
 * @date     :2015-5-31
*******************************************************************************/
#ifndef _USART_H
#define _USART_H

#include "System.h"
#include "stm32f10x.h"
#include "sys.h" 
#include "LEDHardware.h"
#include "ESP8266WIFI.h"
#include "exfuns.h"

#define ESP8266_RX_BUFFER_LEN 1524	// Number of bytes in the serial receive buffer (gives x less chars due to overhead)

void USART_Configuration(uint32_t usart_baudrate);									//串口的初始化
void Usart_Send_Data(USART_TypeDef* USARTx, uint8_t ch);													//从串口发送一个字节的数据
void USART1_Configuration(void);
void USART1_DMAConfiguration(void);
void USART1_HostDMAConfiguration(void);
void USART1_SlaveDMAConfiguration(void);
void USART2_Configuration(void);
void USART2_SlaveDMAConfiguration(void);
void USART2_DMAConfiguration(void);
void USART3_Configuration(void);
void USART3_WIFIReceiveDMAConfiguration(void);
void USART3_WIFISendDMAConfiguration(void);
void USART3_WIFIDMARun(char *buf, uint16_t size);
void USART1_WIFIConfiguration(void);
void USART1_WIFIReceiveDMAConfiguration(void);
void USART1_WIFISendDMAConfiguration(void);
void USART1_WIFIDMARun(char *buf, uint16_t size);
void USART2_WIFIConfiguration(void);
void USART2_WIFIReceiveDMAConfiguration(void);
void USART2_WIFISendDMAConfiguration(void);
void USART2_WIFIDMARun(char *buf, uint16_t size);
void usartSendArrar(USART_TypeDef *USART, uint8_t *Arrar);
void usartSendData(USART_TypeDef *USART, uint8_t data);
void UU_PutChar(USART_TypeDef* USART, uint8_t ch);
void runDMATransfer(void);
void DMA1_Channel4_IRQHandler(void);
void DMA1_Channel5_IRQHandler(void);
void DMA1_Channel2_IRQHandler(void);
void DMA1_Channel3_IRQHandler(void);
void DMA1_Channel7_IRQHandler(void);
void DMA1_Channel6_Stop(void);
void DMA1_Channel6_Start(void);
void DMA1_Channel5_Start(void);
void DMA1_Channel5_Stop(void);
void DMA_Channel_Start(void);
void powerOffSetup(void);
void USART1_IRQHandler(void);
void updateBounds(void);
uint8_t isOwnCube(uint8_t addr);
void DMA_Stop(void);
void clearDMA(void);
#endif

