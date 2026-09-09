#include "W25Qxx.h"
#include "graphics.h"
#include "exfuns.h"

uint8_t W25Q64_Write_Flag = 0;													//内存颗粒写入标志
// Overload and Use Fat FS Buffers
//uint8_t SPI_Buffer[4096];														//内存颗粒读写缓冲区，扇区读写
//uint8_t Read_Buf[768];															//定义读取数据的缓冲区(0-255→R,256-511→G,512-767→B)

uint8_t W25QxxTempAA[3];														//定义3个数组接收扇区数与每个扇区的帧数
uint8_t W25QxxFrame = 0;														//定义保存每个扇区需要显示的帧数
uint8_t W25QxxAddress = 0;														//内存颗粒每个扇区的数据的地址
uint16_t W25QxxSection = 0;														//定义需要读取的扇区个数

/******************************************************************************
/ 函数功能:初始化W25Q64的GPIO口连接
******************************************************************************/
void W25X_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1, ENABLE);

	GPIO_InitStruct.GPIO_Pin 	= GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode 	= GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStruct.GPIO_Pin 	= GPIO_Pin_3;
	//	GPIO_InitStruct.GPIO_Pin 	= GPIO_Pin_4; // If installed On AURA

	GPIO_InitStruct.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIOA->BSRR  = GPIO_Pin_3;
}

void W25X_GPIO_BJConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_SPI1, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStruct.GPIO_Pin 	= GPIO_Pin_4;
	//	GPIO_InitStruct.GPIO_Pin 	= GPIO_Pin_4; // If installed On AURA

	GPIO_InitStruct.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	GPIOB->BSRR  = GPIO_Pin_4;
   /*Lock of the gpio */ // Don't lock please !
	//GPIO_PinLockConfig(GPIOA,GPIO_Pin_3);
}

/******************************************************************************
/ 函数功能:初始化W25Q64
******************************************************************************/
void W25X_Init(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	
	//初始化SPI时钟	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);

	// SPI配置
	SPI_Cmd(SPI1,DISABLE);
	SPI_InitStructure.SPI_NSS 				= SPI_NSS_Soft;							//这里设置 SSM为 1软件管理
	SPI_InitStructure.SPI_Mode 				= SPI_Mode_Master;						//设置SPI工作模式:设置为主SPI				
//	SPI_InitStructure.SPI_CPOL 				= SPI_CPOL_Low;							//选择了串行时钟的稳态:时钟悬空低
//	SPI_InitStructure.SPI_CPHA 				= SPI_CPHA_1Edge;						//数据捕获于第一个时钟沿
	SPI_InitStructure.SPI_CPOL 				= SPI_CPOL_High;						//选择了串行时钟的稳态:时钟悬空高
	SPI_InitStructure.SPI_CPHA 				= SPI_CPHA_2Edge;						//数据捕获于第二个时钟沿
	SPI_InitStructure.SPI_DataSize  		= SPI_DataSize_8b;						//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_FirstBit  		= SPI_FirstBit_MSB;                     //SPI设置成LSB模式
	SPI_InitStructure.SPI_Direction 		= SPI_Direction_2Lines_FullDuplex;		//设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_CRCPolynomial 	= 7;									//CRC值计算的多项式
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;				//36MHz分频
	SPI_Init(SPI1, &SPI_InitStructure );
	SPI_Cmd(SPI1,ENABLE);                                                   		//启动SPI
}


/******************************************************************************
/ 函数功能:SPI发送一个字节的数据
******************************************************************************/
uint8_t SPI_Flash_ReadWriteByte(uint8_t dat)
{
    while ((SPI1->SR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET)
	{
	}
	SPI1->DR = dat;
    while ((SPI1->SR & SPI_I2S_FLAG_RXNE) == (uint16_t)RESET)
	{
	}
	return (SPI1->DR);
}


/******************************************************************************
/ 函数功能:读取芯片ID
/   返回值如下:       
/   0XEF13,表示芯片型号为W25Q08  
/   0XEF14,表示芯片型号为W25Q16    
/   0XEF15,表示芯片型号为W25Q32  
/   0XEF16,表示芯片型号为W25Q64 // This One !
******************************************************************************/     
uint16_t SPI_Flash_ReadID(void)
{
    uint16_t Temp = 0;
     
    if (bjCubeType==1) GPIOB->BRR  = GPIO_Pin_4;
    else GPIOA->BRR  = GPIO_Pin_3;			   
    SPI_Flash_ReadWriteByte(0x90);       											//发送读取ID命令     
    SPI_Flash_ReadWriteByte(0x00);      
    SPI_Flash_ReadWriteByte(0x00);      
    SPI_Flash_ReadWriteByte(0x00);         
    Temp |= SPI_Flash_ReadWriteByte(0xFF) << 8;  
    Temp |= SPI_Flash_ReadWriteByte(0xFF);  
    if (bjCubeType==1) GPIOB->BSRR  = GPIO_Pin_4;
    else GPIOA->BSRR  = GPIO_Pin_3;			   
    return Temp;
} 

//SPI_FLASH写使能	
//将WEL置位   
void SPI_Flash_Write_Enable(void)   
{
    if (bjCubeType==1) GPIOB->BRR  = GPIO_Pin_4;
    else GPIOA->BRR  = GPIO_Pin_3;	                       										//使能器件   
    SPI_Flash_ReadWriteByte(W25X_WriteEnable);      								//发送写使能  
    if (bjCubeType==1) GPIOB->BSRR  = GPIO_Pin_4;
    else GPIOA->BSRR  = GPIO_Pin_3;	                          											//取消片选     	      
} 

//SPI_FLASH写禁止	
//将WEL清零  
void SPI_FLASH_Write_Disable(void)   
{  
    if (bjCubeType==1) GPIOB->BRR  = GPIO_Pin_4;
    else GPIOA->BRR  = GPIO_Pin_3;                          										//使能器件   
    SPI_Flash_ReadWriteByte(W25X_WriteDisable);										//发送写禁止指令    
    if (bjCubeType==1) GPIOB->BSRR  = GPIO_Pin_4;
    else GPIOA->BSRR  = GPIO_Pin_3;	                           											//取消片选     	      
} 			    

//读取SPI_FLASH的状态寄存器
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
uint8_t SPI_Flash_ReadSR(void)   
{  
	uint8_t byte=0;   
  if (bjCubeType==1) GPIOB->BRR  = GPIO_Pin_4;
  else GPIOA->BRR  = GPIO_Pin_3;                             										//使能器件   
	SPI_Flash_ReadWriteByte(W25X_ReadStatusReg);    								//发送读取状态寄存器命令    
	byte = SPI_Flash_ReadWriteByte(0Xff);             								//读取一个字节  
  if (bjCubeType==1) GPIOB->BSRR  = GPIO_Pin_4;
  else GPIOA->BSRR  = GPIO_Pin_3;	                             											//取消片选     	      
	return byte;   
} 

//写SPI_FLASH状态寄存器
//只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
void SPI_FLASH_Write_SR(u8 sr)   
{   
  if (bjCubeType==1) GPIOB->BRR  = GPIO_Pin_4;
  else GPIOA->BRR  = GPIO_Pin_3;                              										//使能器件   
	SPI_Flash_ReadWriteByte(W25X_WriteStatusReg);									//发送写取状态寄存器命令    
	SPI_Flash_ReadWriteByte(sr);													//写入一个字节  
  if (bjCubeType==1) GPIOB->BSRR  = GPIO_Pin_4;
  else GPIOA->BSRR  = GPIO_Pin_3;                            											//取消片选     	      
}   

//等待空闲
void SPI_Flash_Wait_Busy(void)   
{   
	while ((SPI_Flash_ReadSR()&0x01) == 0x01);   									// 等待BUSY位清空
}  

//擦除整个芯片
//整片擦除时间:
//W25X16:25s 
//W25X32:40s 
//W25X64:40s 
//等待时间超长...
void SPI_Flash_Erase_Chip(void)   
{                                             
  SPI_Flash_Write_Enable();                  										//SET WEL 	 
  SPI_Flash_Wait_Busy();   
  if (bjCubeType==1) GPIOB->BRR  = GPIO_Pin_4;
  else GPIOA->BRR  = GPIO_Pin_3;                             										//使能器件   
  SPI_Flash_ReadWriteByte(W25X_ChipErase);        								//发送片擦除命令  
  if (bjCubeType==1) GPIOB->BSRR  = GPIO_Pin_4;
  else GPIOA->BSRR  = GPIO_Pin_3;                         											//取消片选     	      
	SPI_Flash_Wait_Busy();   				   										//等待芯片擦除结束
}   

//擦除一个扇区
void SPI_Flash_Erase_Sector(uint32_t Dst_Addr)   
{   
	Dst_Addr *= 4096;
  SPI_Flash_Write_Enable();                  										//SET WEL 	 
  SPI_Flash_Wait_Busy();   
  if (bjCubeType==1) GPIOB->BRR  = GPIO_Pin_4;
  else GPIOA->BRR  = GPIO_Pin_3;                            										//使能器件   
  SPI_Flash_ReadWriteByte(W25X_SectorErase);      								//发送扇区擦除指令 
  SPI_Flash_ReadWriteByte((u8)((Dst_Addr) >> 16));  								//发送24bit地址    
  SPI_Flash_ReadWriteByte((u8)((Dst_Addr) >> 8));   
  SPI_Flash_ReadWriteByte((u8)Dst_Addr);  
  if (bjCubeType==1) GPIOB->BSRR  = GPIO_Pin_4;
  else GPIOA->BSRR  = GPIO_Pin_3;                            											//取消片选     	      
  SPI_Flash_Wait_Busy();   				   										//等待擦除完成
}

//读取SPI FLASH  
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(24bit)
//NumByteToRead:要读取的字节数(最大65535)
void SPI_Flash_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead)   
{ 
 	uint16_t i;    												    
  if (bjCubeType==1) GPIOB->BRR  = GPIO_Pin_4;
  else GPIOA->BRR  = GPIO_Pin_3;                           										//使能器件   
    SPI_Flash_ReadWriteByte(W25X_ReadData);         								//发送读取命令   
//    SPI_Flash_ReadWriteByte(W25X_FastReadData);         							//发送读取命令   
    SPI_Flash_ReadWriteByte((u8)((ReadAddr) >> 16));  								//发送24bit地址    
    SPI_Flash_ReadWriteByte((u8)((ReadAddr) >> 8));   
    SPI_Flash_ReadWriteByte((u8)ReadAddr);   
    for(i = 0; i < NumByteToRead; i++)
        pBuffer[i] = SPI_Flash_ReadWriteByte(0XFF);   								//循环读数  
  if (bjCubeType==1) GPIOB->BSRR  = GPIO_Pin_4;
  else GPIOA->BSRR  = GPIO_Pin_3;                             											//取消片选     	      
}  

//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!	 
void SPI_Flash_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
 	u16 i;  
    SPI_Flash_Write_Enable();                  										//SET WEL 	 
  if (bjCubeType==1) GPIOB->BRR  = GPIO_Pin_4;
  else GPIOA->BRR  = GPIO_Pin_3;                              										//使能器件   
    SPI_Flash_ReadWriteByte(W25X_PageProgram);      								//发送写页命令   
    SPI_Flash_ReadWriteByte((u8)((WriteAddr) >> 16)); 								//发送24bit地址    
    SPI_Flash_ReadWriteByte((u8)((WriteAddr) >> 8));   
    SPI_Flash_ReadWriteByte((u8)WriteAddr);   
    for(i = 0; i < NumByteToWrite; i++)
		SPI_Flash_ReadWriteByte(pBuffer[i]);										//循环写数  
  if (bjCubeType==1) GPIOB->BSRR  = GPIO_Pin_4;
  else GPIOA->BSRR  = GPIO_Pin_3;                            											//取消片选     	      
	SPI_Flash_Wait_Busy();					   										//等待写入结束
} 

//无检验写SPI FLASH 
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能 
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
void SPI_Flash_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{ 			 		 
	uint16_t pageremain;	   
	pageremain = 256-WriteAddr%256; 												//单页剩余的字节数		 	    
	if(NumByteToWrite <= pageremain)
		pageremain = NumByteToWrite;												//不大于256个字节
	while(1)
	{	   
		SPI_Flash_Write_Page(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite == pageremain)
			break;																	//写入结束了
	 	else 																		//NumByteToWrite > pageremain
		{
			pBuffer += pageremain;
			WriteAddr += pageremain;	

			NumByteToWrite -= pageremain;			  								//减去已经写入了的字节数
			if(NumByteToWrite > 256)
				pageremain = 256; 													//一次可以写入256个字节
			else 
				pageremain = NumByteToWrite; 	  									//不够256个字节了
		}
	}	    
} 

//写SPI FLASH  
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)  		   

void SPI_Flash_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
  u8 SPI_FLASH_BUF[4096];
	uint32_t secpos;
	uint16_t secoff;
	uint16_t secremain;	   
 	uint16_t i;    

	secpos = WriteAddr/4096;														//扇区地址
	secoff = WriteAddr%4096;														//在扇区内的偏移
	secremain = 4096-secoff;														//扇区剩余空间大小   

	if(NumByteToWrite <= secremain)
		secremain = NumByteToWrite;													//不大于4096个字节
	while(1) 
	{	
		SPI_Flash_Read(SPI_FLASH_BUF,secpos*4096,4096);								//读出整个扇区的内容
		for(i = 0;i < secremain; i++)												//校验数据
		{
			if(SPI_FLASH_BUF[secoff+i] != 0XFF)
				break;																//需要擦除  	  
		}
		if(i < secremain)															//需要擦除
		{
			SPI_Flash_Erase_Sector(secpos);											//擦除这个扇区
			for(i = 0; i < secremain; i++)	   										//复制
			{
				SPI_FLASH_BUF[i+secoff] = pBuffer[i];	  
			}
			SPI_Flash_Write_NoCheck(SPI_FLASH_BUF,secpos*4096,4096);				//写入整个扇区  

		}
		else 
			SPI_Flash_Write_NoCheck(pBuffer,WriteAddr,secremain);					//写已经擦除了的,直接写入扇区剩余区间. 				   
		if(NumByteToWrite == secremain)
			break;																	//写入结束了
		else																		//写入未结束
		{
			secpos++;																//扇区地址增1
			secoff = 0;																//偏移位置为0 	 

		   	pBuffer += secremain;  													//指针偏移
			WriteAddr += secremain;													//写地址偏移	   
		   	NumByteToWrite -= secremain;											//字节数递减
			if(NumByteToWrite > 4096)
				secremain = 4096;													//下一个扇区还是写不完
			else 
				secremain = NumByteToWrite;											//下一个扇区可以写完了
		}	 
	}	 	 
}

/*******************************************************************************
 * @name     :void ReadW25Q64(void)
 * @brief    :
 * @param    :
 * @return   :无
 * @version  :V
 * @author   :创优物联工作室→图腾
 * @date     :2015-6-9
 * @details  :读取内存颗粒的数据
 *            
*******************************************************************************/
void ReadW25Q64(void)
{
	if(++SpeedSum >= delaySpeed)																//速度控制
	{
		SpeedSum = 0;																	//速度控制变量清0
		SPI_Flash_Read(fatbuf,4096*NowSection+768*W25QxxAddress+3,768);				//读取flash，一次性都读完数据，不用每次都发地址，提高效率
		AxesConvert();																	//将数据转换到坐标上面
		W25QxxAddress++;
		if(W25QxxAddress >= W25QxxFrame)												//判断一个扇区需要显示的帧数
		{
			W25QxxAddress = 0;															//当前的扇区的地址清0	
			NowSection++;																//当前的扇区增加	
			
			SPI_Flash_Read(W25QxxTempAA,4096*NowSection,3);   							//读取flash
			W25QxxFrame = W25QxxTempAA[2];												//保存每个扇区需要显示的帧数
			//W25QxxSection = W25QxxTempAA[0]+(W25QxxTempAA[1] << 8);					//保存需要读取的扇区个数
			
			if(NowSection >= W25QxxSection)												//判断扇区是否是需要显示的最大值
			{
				NowSection = 0;
				SPI_Flash_Read(W25QxxTempAA,0,3);   									//读取flash
				W25QxxFrame = W25QxxTempAA[2];											//保存每个扇区需要显示的帧数
				W25QxxSection = W25QxxTempAA[0]+(W25QxxTempAA[1] << 8);					//保存需要读取的扇区个数
			}
		}	
	}			
}

