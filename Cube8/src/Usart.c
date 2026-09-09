/*******************************************************************************
 * @file     :☆☆☆彩色光立方☆☆☆
 * @brief    :串口的设置
 * @mcu      :STM32F103RCT6
 * @version  :V1.0
 * @author   :创优物联工作室→图腾
 * @date     :2015-5-31
*******************************************************************************/
#include "usart.h"
#include "CubeLink.h"
#include "ESP8266WIFI.h"

/*******************************************************************************
 * @name     :void USART_Configuration(uint32_t usart_baudrate)
 * @brief    :
 * @param    :
 * @return   :无
 * @version  :V1.0
 * @author   :创优物联工作室→图腾
 * @date     :2015-6-6
 * @details  :串口的初始化
 *            
*******************************************************************************/
void USART_Configuration(uint32_t usart_baudrate)
{
	USART_InitTypeDef USART_InitStructure;											//定义串口的结构体变量
	GPIO_InitTypeDef GPIO_InitStructure;											//定义GPIO的结构体变量
	NVIC_InitTypeDef NVIC_InitStructure;											//定义中断优先级的结构体变量
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1,ENABLE);	//使能串口1的GPIO时钟
	
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_10;									//USART1的RXD引脚
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;						//设置GPIO引脚的模式：GPIO_Mode_IN_FLOATING→浮空输入模式
	GPIO_Init(GPIOA,&GPIO_InitStructure);											//初始化GPIO的外设
	
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_9;									//USART1的TXD引脚
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF_PP;								//设置GPIO引脚的模式：GPIO_Mode_AF_PP→复用推完输出模式
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;								//设置GPIO引脚的输出速度
	GPIO_Init(GPIOA,&GPIO_InitStructure);											//初始化GPIO的外设
	
	USART_InitStructure.USART_BaudRate 			  = usart_baudrate;					//USART串口的波特率
	USART_InitStructure.USART_StopBits 			  = USART_StopBits_1;				//1位的停止位
	USART_InitStructure.USART_Parity 			  = USART_Parity_No;				//无奇偶检验
	USART_InitStructure.USART_Mode 				  = USART_Mode_Rx | USART_Mode_Tx;	//USART串口的模式
	USART_InitStructure.USART_WordLength 		  = USART_WordLength_8b;			//8位的长度
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件流控制
	USART_Init(USART1,&USART_InitStructure);										//初始化USART串口的外设
	USART_Cmd(USART1,ENABLE);														//使能串口1*/
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);									//使能串口的接收中断
	
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;								//中断的通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;						//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;								//响应优先级
	NVIC_Init(&NVIC_InitStructure);													//初始化中断优先级的外设
}

/*******************************************************************************
 * @name     :void Usart_Send_Data(uint8_t ch)
 * @brief    :
 * @param    :
 * @return   :无
 * @version  :V1.0
 * @author   :创优物联工作室→图腾
 * @date     :2015-6-6
 * @details  :从串口发送一个字节的数据
 *            
*******************************************************************************/
void Usart_Send_Data(USART_TypeDef* USARTx, uint8_t ch)
{
	uint16_t timeout=0;
	USART_GetFlagStatus(USARTx, USART_FLAG_TC); 				//发送前先读取状态，否则第一位会丢失
	USART_SendData(USARTx,ch);
	while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET && timeout<20000)
	{
		 timeout++;
	}
	#ifdef doUSB
	if (timeout==20000 && hasUSB) usb_printf("\r\nTIMEOUT ERROR: sending to USART for Usart_Send_Data");
	#endif
}

void USART1_Configuration()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
  
	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
  
	  /* Enable GPIO and USART1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1,ENABLE);
	
	/*USART1 GPIO config*/

	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_9;

  if (DMAAddress==0) GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF_PP;	// Push Pull for Host Transmit
	else GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF_OD;	// Open Drain for Slaves transmit connected together (Needs Pull Up Resistor)
	
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;								
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_10;					
  //GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF_PP;	
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;						
	GPIO_Init(GPIOA,&GPIO_InitStructure);	

	USART_InitStructure.USART_BaudRate = DMABAUD;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

		/* Enable USART1 Receive interrupts */
	if (DMAAddress!=99) USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  		// Don't set if not really using DMA

		/* Enable the USART1 */
	USART_Cmd(USART1, ENABLE);
}

void USART1_HostDMAConfiguration()
{
	DMA_InitTypeDef  DMA_InitStructure; //create DMA structure
	NVIC_InitTypeDef NVIC_InitStructure;  
	
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nDMA SETUP USART1 SEND.");
	#endif
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);   //enable DMA1 clock
	DMA_DeInit(DMA1_Channel4);    //reset DMA1 channe1 to default values;
	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; 	//channel will be used for memory to peripheral transfer
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  	//setting normal
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;  	//medium priority
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  	//source and destination data size byte=8bit
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  	//automatic memory increment enable. Destination and source
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //Location assigned to peripheral
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStructure.DMA_BufferSize = DMASize;  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR; //USART1_DR_Base; //source and destination start addresses
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&rgbled;  // DATA TO SEND
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);  	//send values to DMA registers

	//Enable DMA1 channel IRQ Channel */	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure);

		/* Enable USART */
	USART_Cmd(USART1, ENABLE);
	
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);	// Enable DMA1 Channel Transfer Complete interrupt
	//DMA_Cmd(DMA1_Channel4, ENABLE); // Don't enable until requested

	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nDMA SETUP DONE.\r\n");
	#endif
}

void USART2_Configuration()
{
	// BJ CUBE 8
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	/* Enable the USART2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;		
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable GPIO and USART2 clock */  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	
	/*USART2 GPIO config*/
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_2;							
//	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF_PP;	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_OD; // Open Drain TX to allow multiple slaves on one USART (Needs Pull Up Resistor)
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;		
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_3;									
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;						
	GPIO_Init(GPIOA,&GPIO_InitStructure);	
		
	USART_InitStructure.USART_BaudRate = DMABAUD;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);

		/* Enable USART2 Receive interrupts */
	if (DMAAddress!=99)	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);   // Don't set if not really using DMA

	/* Enable the USART2 */
	USART_Cmd(USART2, ENABLE);
}

void USART1_SlaveDMAConfiguration()
{
	// AURACUBE 8
	DMA_InitTypeDef  DMA_InitStructure; //create DMA structure
	NVIC_InitTypeDef NVIC_InitStructure;  
	
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nDMA SETUP USART1.");
	#endif
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);   //enable DMA1 clock
	DMA_DeInit(DMA1_Channel5);    //reset DMA1 channe1 to default values;
	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; 	//channel will be used for peripheral to memory transfer
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  	//setting normal
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;  	//medium priority
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  	//source and destination data size byte=8bit
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  	//automatic memory increment enable. Destination and source
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //Location assigned to peripheral
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStructure.DMA_BufferSize = DMASize;  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR; //USART1_DR_Base; //source and destination start addresses
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&rgbled;  // DATA TO SEND
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);  	//send values to DMA registers

	//Enable DMA1 channel IRQ Channel */	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure);
	
		/* Enable USART */
	USART_Cmd(USART1, ENABLE);
	
	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
	
	DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);	// Enable DMA1 Channel Transfer Complete interrupt
	//DMA_Cmd(DMA1_Channel5, ENABLE); // Don't enable until requested

	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nDMA SETUP DONE.");
	#endif
}

void USART2_SlaveDMAConfiguration()
{
	// BJ CUBE 8
	DMA_InitTypeDef  DMA_InitStructure; //create DMA structure
	NVIC_InitTypeDef NVIC_InitStructure;  
	
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nDMA SETUP USART2.");
	#endif
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);   //enable DMA1 clock
	DMA_DeInit(DMA1_Channel6);    //reset DMA1 channe1 to default values;
	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; 	//channel will be used for peripheral to memory transfer
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  	//setting normal
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;  	//medium priority
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  	//source and destination data size byte=8bit
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  	//automatic memory increment enable. Destination and source
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //Location assigned to peripheral
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStructure.DMA_BufferSize = DMASize;  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;  	//source and destination start addresses
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&rgbled;  // Place to send data received 
	DMA_Init(DMA1_Channel6, &DMA_InitStructure);  	//send values to DMA registers

	//Enable DMA1 channel IRQ Channel */	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure);
	
		/* Enable USART */
	USART_Cmd(USART2, ENABLE);
		
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
	
	DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);	// Enable DMA1 Channel Transfer Complete interrupt
	//DMA_Cmd(DMA1_Channel6, ENABLE); 
	
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nDMA SETUP DONE.");
	#endif
}

void runDMATransfer(void)
{
	// HOST - Always run on USART1 
	uint8_t count=0;
	uint8_t oldPoint=0;
	uint8_t myShowLED=0;
	myShowLED=showLED;
	nextDMA=0;
	inSendDMA=1;
	if (myShowLED!=3)
	{
		while(nextDMA==0 && count <10)
		{
			nextDMA=DMAAddresses[DMAPointer];
			oldPoint=DMAPointer;
			DMAPointer++;
			if (DMAPointer>9) DMAPointer=1;
			count++;
		}
		DMASize=getDMASize(nextDMA);
		Usart_Send_Data(USART1,'X'); // X instead of C. Translate for Cube8 Host
		Usart_Send_Data(USART1,'Z');
		Usart_Send_Data(USART1,nextDMA+48); // convert to ASCII
		Usart_Send_Data(USART1,DMACommands[oldPoint]+48);
		//if (hasUSB) usb_printf("C%d",nextDMA);
		// let the receiver know the next DMA is coming
		/* Prepare DMA for next transfer */
		/* Important! DMA stream won't start if all flags are not cleared first */
		//if (hasUSB) usb_printf("\r\nSending %d to %d",DMASize,nextDMA);
		USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
		DMA1->IFCR = DMA1_FLAG_GL4 | DMA1_FLAG_HT4 | DMA1_FLAG_TC4 | DMA1_FLAG_TE4;
		DMA1_Channel4->CPAR = (uint32_t)&USART1->DR;

		//	if (nextDMA<5) DMA1_Channel4->CMAR = (uint32_t)&rgbled+DMASize*nextDMA; // only 5 array slots for individual access
		//	else DMA1_Channel4->CMAR = (uint32_t)&rgbled; // If more than 5 copy first cube
		if (DMACommands[oldPoint]==6)
		{
				if (nextDMA==2 && debug==0) DMA1_Channel4->CMAR = (uint32_t)&rgbled+DMASize*4; // special to allow address 2 to be separate - memory constraints
				else DMA1_Channel4->CMAR = (uint32_t)&rgbled+DMASize*3; 
		}
		else
		{
			// Mirror
			DMA1_Channel4->CMAR = (uint32_t)&rgbled+DMASize*myShowLED;   /* Set memory address for DMA again */
		}
		DMA1_Channel4->CNDTR = DMASize;    /* Set number of bytes to receive */
		DMA1_Channel4->CCR |= (uint16_t)(DMA_CCR4_EN);            /* Start DMA transfer */
	}
	else
	{
		// showLED 3 is blank - don't send data, just command
		Usart_Send_Data(USART1,'D');
		Usart_Send_Data(USART1,'Z');
		while(nextDMA==0 && count <10)
		{
			nextDMA=DMAAddresses[DMAPointer];
			DMAPointer++;
			if (DMAPointer>9) DMAPointer=1;
			count++;
		}
		Usart_Send_Data(USART1,nextDMA+48); // convert to ASCII
		Usart_Send_Data(USART1,'0');
		inSendDMA=0;
	}
}

void DMA1_Channel4_IRQHandler(void) 
{
	// Transfer Finished!
	uint16_t timeout=0;
	DMA_ClearITPendingBit(DMA1_IT_TC4);
	DMA_Cmd(DMA1_Channel4, DISABLE);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET && timeout<20000)
	{
		// Wait for USART to finish
		timeout++;
	}
	#ifdef doUSB
	if (timeout==20000 && hasUSB) usb_printf("\r\nTIMEOUT ERROR: DMA1_Channel4_IRQHandler");
	#endif
	USART_DMACmd(USART1, USART_DMAReq_Tx, DISABLE);
	if (cubeWIFI!=10) inSendDMA=0;
	else SDinUse=0; // Release the token for Aura8 MarkII Usart1 WIFI
}

void DMA1_Channel5_IRQHandler(void) 
{
	// Transfer Finished!
	DMA_ClearITPendingBit(DMA1_IT_TC5);
	DMA1_Channel5_Stop();	
}

void USART2_IRQHandler(void)
{
	char c;
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
			c = USART_ReceiveData(USART2);
			//if (hasUSB) usb_printf("%c",c);
			if (DMAAddress!=99) SlaveIRQ(c);	
		  else
			{
				// WIFI for CUBE 8
				WIFI_IRQ(c);
			}
	}	
	if (DMAAddress==99) // Only Do for WIFI
	{
		// Check for IDLE flag 
		if(USART_GetFlagStatus(USART2,USART_FLAG_IDLE))   
		{   
			USART_ClearFlag(USART2,USART_FLAG_IDLE);   
			c=USART2->DR;
			if (hasWIFI==4) WIFI_REC_DMA(DMA1_Channel6->CNDTR); // Only if already done WIFI init
		}
		if(USART_GetFlagStatus(USART2,USART_FLAG_ORE) && USART_GetITStatus(USART2, USART_IT_RXNE) == RESET)   
		{   
			USART_ClearFlag(USART2,USART_FLAG_ORE);  
			USART_ClearITPendingBit (USART2, USART_IT_ORE);		
			// Overrun Error     
			c=USART2->DR;
			#ifdef doUSB
			if (hasUSB) usb_printf("\r\nWIFI Overrun Error Char : %c",c);
			#endif
		}   
		if(USART_GetFlagStatus(USART2,USART_FLAG_FE))   
		{   
			USART_ClearFlag(USART2,USART_FLAG_FE);   
			// Framing Error   
			c=USART2->DR;
			#ifdef doUSB
			if (hasUSB) usb_printf("\r\nWIFI Framing Error Char : %c",c);
			#endif
		}   
		if(USART_GetFlagStatus(USART2,USART_FLAG_NE))   
		{   
			USART_ClearFlag(USART2,USART_FLAG_NE);   
			// Noise Error
			c=USART2->DR;	
			#ifdef doUSB
			if (hasUSB) usb_printf("\r\nWIFI Noise Error Char : %c",c);		
			#endif
		}   
		if(USART_GetFlagStatus(USART2,USART_FLAG_PE))   
		{   
			USART_ClearFlag(USART2,USART_FLAG_PE);   
			// Parity Error
			c=USART2->DR;
			#ifdef doUSB
			if (hasUSB) usb_printf("\r\nWIFI Parity Error Char : %c",c);		
			#endif
		} 	
	}
}

void DMA1_Channel5_Stop()
{
	//USART1 SLAVE
	inDMA=0;
	DMA_Cmd(DMA1_Channel5, DISABLE);
	USART_DMACmd(USART1, USART_DMAReq_Rx, DISABLE);
	
	// Start Receive IRQ's again
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE ); //Enable receive interrupt
}

void DMA1_Channel5_Start()
{
	//USART1 SLAVE
	/* Prepare DMA for next transfer */
	/* Important! DMA stream won't start if all flags are not cleared first */
	inDMA=1;
	
	// Stop Receive IRQ's while DMA in progress
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE ); //Disable receive interrupt
  USART_ClearITPendingBit(USART1, USART_IT_RXNE); //Clear pending interrupt bit
	
	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
	DMA1->IFCR = DMA1_FLAG_GL5 | DMA1_FLAG_HT5 | DMA1_FLAG_TC5 | DMA1_FLAG_TE5;
	DMA1_Channel5->CPAR = (uint32_t)&USART1->DR;
	DMA1_Channel5->CMAR = (uint32_t)&rgbled;   /* Set memory address for DMA again */
	DMA1_Channel5->CNDTR = DMASize;    /* Set number of bytes to receive */
	DMA1_Channel5->CCR |= (uint16_t)(DMA_CCR5_EN);            /* Start DMA transfer */
}

void DMA1_Channel6_Stop()
{
	//USART2 SLAVE
	inDMA=0;
	DMA_Cmd(DMA1_Channel6, DISABLE);
	USART_DMACmd(USART2, USART_DMAReq_Rx, DISABLE);
	
	// Start Receive IRQ's again
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE ); //Enable receive interrupt
}

void DMA1_Channel6_Start()
{
	// BJCube SLAVE
	/* Prepare DMA for next transfer */
	/* Important! DMA stream won't start if all flags are not cleared first */
	inDMA=1;
	// Stop Receive IRQ's while DMA in progress
	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE ); //Disable receive interrupt
  USART_ClearITPendingBit(USART2, USART_IT_RXNE); //Clear pending interrupt bit	
	
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
	DMA1->IFCR = DMA1_FLAG_GL6 | DMA1_FLAG_HT6 | DMA1_FLAG_TC6 | DMA1_FLAG_TE6;
	DMA1_Channel6->CPAR = (uint32_t)&USART2->DR;
	DMA1_Channel6->CMAR = (uint32_t)&rgbled;   /* Set memory address for DMA again */
	DMA1_Channel6->CNDTR = DMASize;    /* Set number of bytes to receive */
	DMA1_Channel6->CCR |= (uint16_t)(DMA_CCR6_EN);            /* Start DMA transfer */
}

void DMA_Stop(void)
{
	// Stop a DMA Stream mid session
	if (DMAUSART==USART1) DMA1_Channel5_Stop();
	else DMA1_Channel6_Stop();
}

void DMA_Channel_Start(void)
{
	if (exitDMA==0)
	{
		Cube_MODE=6;
		welcome=0;
	}
	if (DMAUSART==USART2) DMA1_Channel6_Start(); // BJCube USART2
	else DMA1_Channel5_Start(); // USART1
}

void DMA1_Channel6_IRQHandler(void) 
{
	// BJCube SLAVE
	DMA_ClearITPendingBit(DMA1_IT_TC6);
	if (DMAAddress!=99) DMA1_Channel6_Stop(); // Don't do for WIFI
}

void PVD_IRQHandler()
{
	// Power has gone off !
	if (DMAAddress==0) {stopDMATransfer();sendDMA=0;DMA1_Channel5_Stop();}
	if (DMAAddress>0 && DMAAddress<10) //SLAVE
	{
		Usart_Send_Data(DMAUSART,'B');
		Usart_Send_Data(DMAUSART,DMAAddress+48); // convert to ASCII
		getDMA=0;	
	}
	setAll(BLACK);
	while (1) {}
}

void USART1_IRQHandler(void)
{
	char c;
	if (cubeWIFI!=10) USARTIRQ();
	else
	{
		if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
		{
			c = USART_ReceiveData(USART1);
			//if (hasUSB) usb_printf("%c",c);
			// WIFI for AURA8 II
			WIFI_IRQ(c);
		}	
	}
	if (cubeWIFI==10) // Only do for WIFI
	{
		// Check for IDLE flag 
		if(USART_GetFlagStatus(USART1,USART_FLAG_IDLE))   
		{   
			USART_ClearFlag(USART1,USART_FLAG_IDLE);   
			c=USART1->DR;
			if (hasWIFI==4 && cubeWIFI==10) WIFI_REC_DMA(DMA1_Channel4->CNDTR); // Only if already done WIFI init
		}
		// Check for transmission problems
		if(USART_GetFlagStatus(USART1,USART_FLAG_ORE) && USART_GetITStatus(USART1, USART_IT_RXNE) == RESET)   
		{   
			USART_ClearFlag(USART1,USART_FLAG_ORE);  
			USART_ClearITPendingBit(USART1, USART_IT_ORE);		
			// Overrun Error     
			c=USART1->DR;
			set(1,0,0,GREEN);
			#ifdef doUSB
			if (hasUSB) usb_printf("\r\nChar : %c",c);
			#endif
		}   
		if(USART_GetFlagStatus(USART1,USART_FLAG_FE))   
		{   
			USART_ClearFlag(USART1,USART_FLAG_FE);   
			// Framing Error   
			c=USART1->DR;
			set(1,0,0,PURPLE);
		}   
		if(USART_GetFlagStatus(USART1,USART_FLAG_NE))   
		{   
			USART_ClearFlag(USART1,USART_FLAG_NE);   
			// Noise Error
			c=USART1->DR;	
		}   
		if(USART_GetFlagStatus(USART1,USART_FLAG_PE))   
		{   
			USART_ClearFlag(USART1,USART_FLAG_PE);   
			// Parity Error
			c=USART1->DR;
			set(1,0,0,WHITE);
		}
	}		
}

void powerOffSetup()
{
	NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the PVD Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = PVD_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	// NVIC_SetPriority(PVD_IRQn, 0);
	
	/* PWR clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  
  /* PVD configuration: 2.9Volts */
  PWR_PVDLevelConfig(PWR_PVDLevel_2V9);
  
  /* Enable the Power Voltage Detector(PVD) */
  PWR_PVDCmd(ENABLE);
}

void UU_PutChar(USART_TypeDef* USART, uint8_t ch)
{
	uint16_t timeout=0;
  while(!(USART->SR & USART_SR_TXE) && timeout<20000) // need bigger timeout for multiple UDP sends
	{
		timeout++;
	}
	#ifdef doUSB
	if (timeout==20000 && hasUSB) usb_printf("\r\nTIMEOUT ERROR: USART UU_PutChar");
	#endif
  USART->DR = ch;  
}

void usartSendArrar(USART_TypeDef *USART, uint8_t *Arrar)
{
  #ifdef doUSB
  if (hasUSB && USART==USART3 && hasWIFI!=4) usb_printf((char *)Arrar);	// Print out Wifi Details if USB is attached and USART3
  #endif
  while(*Arrar != 0)
  {
    UU_PutChar(USART, *Arrar);
    Arrar++;
  }
}

void updateBounds(void)
{
	// Not implemented on Aura8 - Just update the size
	calculateTotalSize();
}


uint8_t isOwnCube(uint8_t addr)
{
	// Not implemented on Aura8
  return 0;
}

void USART3_Configuration()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	 
	/* Enable the USART3 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	  /* Enable GPIO and USART3 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	
	/*USART3 GPIO config*/
	/*Configure USART3 Tx(PB.10) as alternate functiong push-pull*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/*Configure USART3 Rx(PB.11) as input floating*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = ESP8266BAUD;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);

	/* Enable USART3 Receive interrupts */
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
  USART_ITConfig(USART3, USART_IT_IDLE, DISABLE); 

	/* Enable the USART3 */
	//USART_Cmd(USART3, ENABLE);
}

void USART3_WIFISendDMAConfiguration(void) 
{
	// No DMA on Aura8 for WIFI
	// Configure DMA for USART3 TX, DMA1, Channel2
	DMA_InitTypeDef  DMA_InitStructure; //create DMA structure
	NVIC_InitTypeDef NVIC_InitStructure;  
	
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nDMA SETUP WIFI SEND USART3.");
	#endif
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);   //enable DMA1 clock
	DMA_DeInit(DMA1_Channel2);    //reset DMA1 channe1 to default values;
	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; 	//channel will be used for memory to peripheral transfer
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  	//setting normal
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;  	//low priority
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  	//source and destination data size byte=8bit
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  	//automatic memory increment enable. Destination and source
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //Location assigned to peripheral
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStructure.DMA_BufferSize = 512;
	//DMA_InitStructure.DMA_BufferSize = DMA_TX_BUFFER_SIZE;  // Don't know this yet;  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART3->DR; //USART3_DR_Base; //source and destination start addresses
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)fatbuf;   // DATA TO SEND
	DMA_Init(DMA1_Channel2, &DMA_InitStructure);  	//send values to DMA registers

	//Enable DMA1 channel IRQ Channel */	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure);
  
	USART_Cmd(USART3, ENABLE);
	
	USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
	DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);	// Enable DMA1 Channel Transfer Complete interrupt
	//DMA_Cmd(DMA1_Channel2, ENABLE); // Don't enable until requested

	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nDMA SETUP WIFI SEND USART3 DONE.");	
	#endif
}

void USART3_WIFIDMARun(char *buf, uint16_t size)
{
	SDinUse=2; // Grab DMA Token
	USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
	DMA1->IFCR = DMA1_FLAG_GL2 | DMA1_FLAG_HT2 | DMA1_FLAG_TC2 | DMA1_FLAG_TE2;
	DMA1_Channel2->CPAR = (uint32_t)&USART3->DR;
	DMA1_Channel2->CMAR = (uint32_t)buf; 
	DMA1_Channel2->CNDTR = size;    /* Set number of bytes to receive */
	DMA1_Channel2->CCR |= (uint16_t)(DMA_CCR2_EN);            /* Start DMA transfer */
}

void DMA1_Channel2_IRQHandler(void) 
{
	// Host Transfer Finished!
	uint16_t timeout=0;
	DMA_ClearITPendingBit(DMA1_IT_TC2);
	DMA_Cmd(DMA1_Channel2, DISABLE);
	while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET && timeout<20000) 
	{
		// Wait for USART to finish
		timeout++;
	}
	#ifdef doUSB
	if (timeout==20000 && hasUSB) usb_printf("\r\nTIMEOUT ERROR: DMA1_Channel2_IRQHandler");
	#endif
	USART_DMACmd(USART3, USART_DMAReq_Tx, DISABLE);
	SDinUse=0; // Release the token*/
}

void USART3_WIFIReceiveDMAConfiguration(void) 
{
	// No DMA on Aura8 for WIFI
	// Configure DMA1 for USART3 RX, Channel3 
/*	DMA_InitTypeDef  DMA_InitStructure; //create DMA structure
	NVIC_InitTypeDef NVIC_InitStructure;  
	
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nDMA SETUP WIFI RECEIVE USART3. %d",ESP8266_RX_BUFFER_LEN);
	#endif
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);   //enable DMA1 clock
	DMA_DeInit(DMA1_Channel3);    //reset DMA1 channe1 to default values;
	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; 	//channel will be used for memory to peripheral transfer
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  	//setting circular
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;  	//low priority
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  	//source and destination data size byte=8bit
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  	//automatic memory increment enable. Destination and source
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //Location assigned to peripheral
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStructure.DMA_BufferSize = ESP8266_RX_BUFFER_LEN;  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART3->DR; //USART3_DR_Base; //source and destination start addresses
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)esp8266RxBuffer; 
	DMA_Init(DMA1_Channel3, &DMA_InitStructure);  	//send values to DMA registers
	
		//Enable DMA1 channel IRQ Channel 	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;  // NEED THIS
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 2 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure);

  USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE); 
	
	// Enable IDLE line detection for DMA processing 
  USART_ITConfig(USART3, USART_IT_IDLE, ENABLE); 
  USART_ITConfig(USART3, USART_IT_RXNE, DISABLE ); //Disable receive interrupt	
  DMA_ITConfig(DMA1_Channel3, DMA_IT_TC, ENABLE);		
	DMA_Cmd(DMA1_Channel3, ENABLE); 	*/
	
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE); 
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nWIFI IDLE ENABLED.");
	#endif
}

void DMA1_Channel3_IRQHandler(void) 
{
	// Not Implemented
	DMA_ClearITPendingBit(DMA1_FLAG_TC3);
}

void USART3_IRQHandler(void)
{
	// WIFI Handler
	char c;
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		 c = USART_ReceiveData(USART3);
		 //usb_printf("%c",c);
		 WIFI_IRQ(c);
	}
  // Check for IDLE flag 
	if(USART_GetFlagStatus(USART3,USART_FLAG_IDLE))   
  {   
    USART_ClearFlag(USART3,USART_FLAG_IDLE);   
		c=USART3->DR;
		if (hasWIFI==4) WIFI_REC_DMA(DMA1_Channel3->CNDTR); // Only if already done WIFI init
	}
	if(USART_GetFlagStatus(USART3,USART_FLAG_ORE) && USART_GetITStatus(USART3, USART_IT_RXNE) == RESET)   
  {   
    USART_ClearFlag(USART3,USART_FLAG_ORE);  
    USART_ClearITPendingBit (USART3, USART_IT_ORE);		
    // Overrun Error     
		c=USART3->DR;
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nWIFI Overrun Error Char : %c",c);
		#endif
  }   
  if(USART_GetFlagStatus(USART3,USART_FLAG_FE))   
  {   
    USART_ClearFlag(USART3,USART_FLAG_FE);   
    // Framing Error   
		c=USART3->DR;
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nWIFI Framing Error Char : %c",c);
		#endif
  }   
  if(USART_GetFlagStatus(USART3,USART_FLAG_NE))   
  {   
    USART_ClearFlag(USART3,USART_FLAG_NE);   
    // Noise Error
		c=USART3->DR;	
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nWIFI Noise Error Char : %c",c);		
		#endif
  }   
  if(USART_GetFlagStatus(USART3,USART_FLAG_PE))   
  {   
    USART_ClearFlag(USART3,USART_FLAG_PE);   
    // Parity Error
		c=USART3->DR;
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nWIFI Parity Error Char : %c",c);		
		#endif
  } 	
}

void USART1_WIFIConfiguration()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
  
	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
  
	  /* Enable GPIO and USART1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1,ENABLE);
	
	/*USART1 GPIO config*/
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF_PP;	// Push Pull for Host Transmit
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;								
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_10;					
  //GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF_PP;	
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;						
	GPIO_Init(GPIOA,&GPIO_InitStructure);	

	USART_InitStructure.USART_BaudRate = ESP8266BAUD;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

		/* Enable USART1 Receive interrupts */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);   // Don't set if not really using DMA
  USART_ITConfig(USART1, USART_IT_IDLE, DISABLE); 
	
	/* Enable the USART1 */
	//USART_Cmd(USART1, ENABLE);
}

void USART1_WIFISendDMAConfiguration(void) 
{
	// No DMA on Aura8 for WIFI
	// Configure DMA for USART1 TX, DMA1, Channel4
	DMA_InitTypeDef  DMA_InitStructure; //create DMA structure
	NVIC_InitTypeDef NVIC_InitStructure;  
	
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nDMA SETUP WIFI SEND USART1.");
	#endif
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);   //enable DMA1 clock
	DMA_DeInit(DMA1_Channel4);    //reset DMA1 channe1 to default values;
	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; 	//channel will be used for memory to peripheral transfer
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  	//setting normal
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;  	//medium priority
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  	//source and destination data size byte=8bit
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  	//automatic memory increment enable. Destination and source
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //Location assigned to peripheral
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStructure.DMA_BufferSize = 512;  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR; //USART1_DR_Base; //source and destination start addresses
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)fatbuf;   // DATA TO SEND
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);  	//send values to DMA registers

	//Enable DMA1 channel IRQ Channel */	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure);

		/* Enable USART */
	USART_Cmd(USART1, ENABLE);
	
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);	// Enable DMA1 Channel Transfer Complete interrupt
	//DMA_Cmd(DMA1_Channel4, ENABLE); // Don't enable until requested
	
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nDMA SETUP WIFI SEND USART1 DONE.");		
	#endif
}

void USART1_WIFIDMARun(char *buf, uint16_t size)
{
	SDinUse=2; // Grab DMA Token
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
	DMA1->IFCR = DMA1_FLAG_GL4 | DMA1_FLAG_HT4 | DMA1_FLAG_TC4 | DMA1_FLAG_TE4; 
	DMA1_Channel4->CPAR = (uint32_t)&USART1->DR;
	DMA1_Channel4->CMAR = (uint32_t)buf; 
	DMA1_Channel4->CNDTR = size;    /* Set number of bytes to receive */
	DMA1_Channel4->CCR |= (uint16_t)(DMA_CCR4_EN);            /* Start DMA transfer */
}

void USART1_WIFIReceiveDMAConfiguration(void) 
{
	// No DMA on Aura8 for WIFI
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE); 
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nWIFI IDLE ENABLED.");
	#endif
}

void USART2_WIFIConfiguration(void)
{
	// BJ CUBE 8
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	/* Enable the USART2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;		
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable GPIO and USART2 clock */  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	
	/*USART2 GPIO config*/
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_2;							
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF_PP;	
//	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_OD; // Open Drain TX to allow multiple slaves on one USART (Needs Pull Up Resistor)
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;		
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_3;									
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;						
	GPIO_Init(GPIOA,&GPIO_InitStructure);	
		
	USART_InitStructure.USART_BaudRate = ESP8266BAUD;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);

		/* Enable USART2 Receive interrupts */
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);   // Don't set if not really using DMA
  USART_ITConfig(USART2, USART_IT_IDLE, DISABLE); 
	
	/* Enable the USART2 */
	//USART_Cmd(USART2, ENABLE);
}

void USART2_WIFISendDMAConfiguration(void) 
{
	// No DMA on Aura8 for WIFI
	// Configure DMA for USART2 TX, DMA1, Channel7
	DMA_InitTypeDef  DMA_InitStructure; //create DMA structure
	NVIC_InitTypeDef NVIC_InitStructure;  
	
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nDMA SETUP WIFI SEND USART2.");
	#endif
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);   //enable DMA1 clock
	DMA_DeInit(DMA1_Channel7);    //reset DMA1 channe1 to default values;
	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; 	//channel will be used for memory to peripheral transfer
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  	//setting normal
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;  	//low priority
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  	//source and destination data size byte=8bit
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  	//automatic memory increment enable. Destination and source
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //Location assigned to peripheral
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStructure.DMA_BufferSize = 512;
	//DMA_InitStructure.DMA_BufferSize = DMA_TX_BUFFER_SIZE;  // Don't know this yet;  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR; //USART3_DR_Base; //source and destination start addresses
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)fatbuf;   // DATA TO SEND
	DMA_Init(DMA1_Channel7, &DMA_InitStructure);  	//send values to DMA registers

	//Enable DMA1 channel IRQ Channel */	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure);
  
	USART_Cmd(USART2, ENABLE);
	
	USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
	DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);	// Enable DMA1 Channel Transfer Complete interrupt
	//DMA_Cmd(DMA1_Channel7, ENABLE); // Don't enable until requested

	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nDMA SETUP WIFI SEND USART2 DONE.");		
	#endif
}

void USART2_WIFIDMARun(char *buf, uint16_t size)
{
	SDinUse=2; // Grab DMA Token
	USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
	DMA1->IFCR = DMA1_FLAG_GL7 | DMA1_FLAG_HT7 | DMA1_FLAG_TC7 | DMA1_FLAG_TE7; 
	DMA1_Channel7->CPAR = (uint32_t)&USART2->DR;
	DMA1_Channel7->CMAR = (uint32_t)buf; 
	DMA1_Channel7->CNDTR = size;    /* Set number of bytes to receive */
	DMA1_Channel7->CCR |= (uint16_t)(DMA_CCR7_EN);            /* Start DMA transfer */
}

void DMA1_Channel7_IRQHandler(void) 
{
	// Host Transfer Finished!
	uint16_t timeout=0;
	DMA_ClearITPendingBit(DMA1_IT_TC7);
	DMA_Cmd(DMA1_Channel7, DISABLE);
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET && timeout<20000) 
	{
		// Wait for USART to finish
		timeout++;
	}
	#ifdef doUSB
	if (timeout==20000 && hasUSB) usb_printf("\r\nTIMEOUT ERROR: DMA1_Channel2_IRQHandler");
	#endif
	USART_DMACmd(USART2, USART_DMAReq_Tx, DISABLE);
	SDinUse=0; // Release the token*/
}

void USART2_WIFIReceiveDMAConfiguration(void) 
{
	// No DMA on Aura8 for WIFI
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE); 
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nWIFI IDLE ENABLED.");
	#endif
}

void clearDMA(void)
{
  // Not used in this cube
}
