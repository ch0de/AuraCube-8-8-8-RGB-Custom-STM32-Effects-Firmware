#ifndef _W25Qxx_H
#define _W25Qxx_H			    

#include "stm32f10x.h"

//指令表
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 

extern uint8_t W25Q64_Write_Flag;											//内存颗粒写入标志
//extern uint8_t SPI_Buffer[4096];											//内存颗粒读写缓冲区，扇区读写
//extern uint8_t Read_Buf[768];												//定义读取数据的缓冲区(0-255→R,256-511→G,512-767→B)
extern uint8_t W25QxxTempAA[3];												//定义3个数组接收扇区数与每个扇区的帧数
extern uint8_t W25QxxFrame;													//定义保存每个扇区需要显示的帧数
extern uint8_t W25QxxAddress;												//内存颗粒每个扇区的数据的地址
extern uint16_t W25QxxSection;												//定义需要读取的扇区个数

//初始化
void W25X_GPIO_Config(void);         														//配置GPIO口
void W25X_GPIO_BJConfig(void);
void W25X_Init(void);                														//初始化SPI

//获取状态
uint16_t SPI_Flash_ReadID(void);          													//读取ID号
void SPI_FLASH_Write_SR(uint8_t sr);  														//写状态寄存器
void SPI_FLASH_Write_Enable(void);  														//写使能 
void SPI_FLASH_Write_Disable(void);															//写保护
void SPI_Flash_Erase_Sector(uint32_t Dst_Addr);												//擦除一个扇区  
void SPI_Flash_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead);   			//读取flash
void SPI_Flash_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);			//写入flash
void SPI_Flash_Erase_Chip(void);    	  													//整片擦除
void SPI_Flash_Erase_Sector(uint32_t Dst_Addr);												//扇区擦除
void SPI_Flash_Wait_Busy(void);           													//等待空闲
void ReadW25Q64(void);

#endif

