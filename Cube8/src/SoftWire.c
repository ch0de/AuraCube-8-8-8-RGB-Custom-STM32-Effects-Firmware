#include "SoftWire.h"

#define I2C_WRITE 0
#define I2C_READ  1

/* 0 level conventions:
 * - SDA/SCL idle 1 (expected 1)
 * - always start with i2c_delay rather than end
 */
 
uint32_t i2c_delay=0;

uint8_t set_scl(uint8_t state) 
{
	  uint16_t timeout=0;
	  int i;
    I2C_DELAY(i2c_delay);

	if (bjCubeType==2)
	{
	  if (state==0)  GPIO_WriteBit(GPIOA,GPIO_Pin_12,Bit_RESET);//SCL_OFF;
	  else GPIO_WriteBit(GPIOA,GPIO_Pin_12,Bit_SET); //SCL_ON;
	}
	else
	{
	  if (state==0)  GPIO_WriteBit(GPIOC,GPIO_Pin_10,Bit_RESET);//SCL_OFF;
	  else GPIO_WriteBit(GPIOC,GPIO_Pin_10,Bit_SET); //SCL_ON;		
	}
    if (state == 1) {
			if (bjCubeType==2)
			{
        while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_12) == 0 && timeout <20000)
				{
					 timeout++;
				}
				if (timeout==20000)
				{
					 #ifdef doUSB
					 if (hasUSB) usb_printf("\r\nERROR: Unable to talk to I2C - Timeout");
					 #endif
					 return 1;
				}				
			}
			else
			{
        while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_10) == 0 && timeout <20000)
				{
					 timeout++;
				}
				if (timeout==20000)
				{
					 #ifdef doUSB
					 if (hasUSB) usb_printf("\r\nERROR: Unable to talk to I2C - Timeout");
					 #endif
					 return 1;
				}								
			}
    }
		return 0;
}

void set_sda(uint8_t state) 
{
	int i;
	I2C_DELAY(i2c_delay);
	
	if (bjCubeType==2)
	{
		if (state==0)  GPIO_WriteBit(GPIOA,GPIO_Pin_11,Bit_RESET);//SDA_OFF;
		else GPIO_WriteBit(GPIOA,GPIO_Pin_11,Bit_SET); //SDA_ON;	
	}
	else
	{
	  if (state==0)  GPIO_WriteBit(GPIOC,GPIO_Pin_11,Bit_RESET);//SDA_OFF;
	  else GPIO_WriteBit(GPIOC,GPIO_Pin_11,Bit_SET); //SDA_ON;		
	}
}

void i2c_start() 
{
    set_sda(0);
    set_scl(0);
}

void i2c_stop() 
{
    set_sda(0);
    set_scl(1);
    set_sda(1);
}

void i2c_repeated_start()
{
    set_sda(1);
    set_scl(1);
    set_sda(0);
}

bool i2c_get_ack() 
{
	  bool ret;
    set_scl(0);
    set_sda(1);
    set_scl(1);
	  if (bjCubeType==2)
		{
			if (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11)==0) ret=TRUE;  // SDA	
			else ret=FALSE;
		}
		else
		{
  	  if (GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_11)==0) ret=TRUE;  // SDA		
			else ret=FALSE;
		}
    set_scl(0);
    return ret;
}

void i2c_send_ack() 
{
    set_sda(0);
    set_scl(1);
    set_scl(0);
}

void i2c_send_nack() 
{
    set_sda(1);
    set_scl(1);
    set_scl(0);
}

uint8_t i2c_shift_in() 
{
    uint8_t data = 0;
    int i;
	  set_sda(1);

    for (i = 0; i < 8; i++) 
	  {
        set_scl(1);
			  if (bjCubeType==2)
				{
			    data |= GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11) << (7-i); //SDA					
				}
				else
				{
          data |= GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_11) << (7-i); //SDA
				}
        set_scl(0);
    }
    return data;
}

void i2c_shift_out(uint8_t val) 
{
    int i;
    for (i = 0; i < 8; i++) 
	  {
        set_sda(!!(val & (1 << (7 - i)) ) );
        set_scl(1);
        set_scl(0);
    }
}

//process needs to be updated for repeated start.
uint8_t processi2c(uint8_t stop) 
{
	  uint8_t sla_addr;
	  uint8_t i;
    sla_addr = (myWireBase.addr << 1);
    if (myWireBase.flags == I2C_MSG_READ) 
		{
        sla_addr |= I2C_READ;
    }
			i2c_start();
			// shift out the address we're transmitting to
			i2c_shift_out(sla_addr);
			if (!i2c_get_ack()) 
			{
				#ifdef doUSB
				if (hasUSB && myWireBase.flags == I2C_MSG_READ) usb_printf("\r\ni2c - No ACK after start on Read");
				#endif
				i2c_stop();
        return ENACKADDR;
			}
    // Receiving
    if (myWireBase.flags == I2C_MSG_READ) 
		{
        while (myWireBase.bytesRec < myWireBase.length) 
				{
            myWireBase.buf[myWireBase.bytesRec++] = i2c_shift_in();
            if (myWireBase.bytesRec < myWireBase.length) 
						{
                i2c_send_ack();
            } 
						else 
						{
                i2c_send_nack();
            }
        }
    }
    // Sending
    else 
		{
        for (i = 0; i < myWireBase.buf_idx; i++) 
			  {
            i2c_shift_out(myWireBase.buf[i]);
            if (!i2c_get_ack()) 
						{
								i2c_stop();
                return ENACKTRNS;
            }
        }
    }
    if(stop == TRUE) i2c_stop();
    else i2c_repeated_start();
	
    return SUCCESS;
}

uint8_t i2cMasterReceive(uint8_t addr, uint8_t* pData, uint16_t size, uint8_t repeatStart)
{
	uint16_t count=0;
	uint8_t retVal;
	if (repeatStart==0) beginTransmission(addr);
	retVal=requestFromi2c(addr,size);
	if (retVal==0)
	{
		for (count=0;count<size;count++)
		{
			*pData++=readi2c();
		}	
		if (repeatStart==1) endTransmission(FALSE);	
		else endTransmission(TRUE);
	}
	return retVal;
}

uint8_t i2cMasterTransmit(uint8_t addr, uint8_t* pData, uint16_t size, uint8_t repeatStart)
{
	beginTransmission(addr);
	writei2c(pData,size);
	if (repeatStart==1) endTransmission(FALSE);	
	else endTransmission(TRUE);
	return 0;
}

uint8_t beginSoftWire(uint8_t self_addr) {
    myWireBase.buf_idx = 0;
    myWireBase.buf_overflow = FALSE;
    myWireBase.length = 0;
    if (set_scl(1)==1)
		{
			// Timeout
			return 1;
		}
		set_sda(1);
		return 0;
}

void seti2cClock(uint32_t frequencyHz)
{
	switch(frequencyHz)
	{
		case 400000:
			i2c_delay = SOFT_FAST;
			break;
		case 100000:
		default:
			i2c_delay = SOFT_STANDARD;
			break;
	}
}

void i2cScan(void) 
{
  uint8_t error, address;
  int nDevices;

  nDevices = 0;
  for(address = 1; address < 127; address++) 
	{
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.

    beginTransmission(address);
    error = endTransmission(TRUE);
    
    if (error == 0) 
		{
			#ifdef doUSB
			if (hasUSB) usb_printf("\r\nI2C device found at address 0x%X",address);
			#endif
			if ((address>=0x20 && address <=0x27)||(address>=0x38 && address <=0x3f))
			{
				// LCD Address - Must have one !
			    hasLCD=address;	
				#ifdef doUSB
				if (hasUSB) usb_printf(" - LCD");
				#endif
			}
			if (address==0x68) 
			{
				// DS3231 RTC
			    hasDS3231=10;	
				#ifdef doUSB
				if (hasUSB) usb_printf(" - RTC DS3231");				
				#endif
			}
			if (address==0x44 || address==0x45) 
			{
				// SHT31 Sensor
				hasSHT31=address;
				#ifdef doUSB
				if (hasUSB) usb_printf(" - SHT31");				
				#endif
			}
			if (address==0x76 || address==0x77) 
			{
				// BME280 Sensor
				hasBME280=address;
				#ifdef doUSB
				if (hasUSB) usb_printf(" - BME280");				
				#endif
			}						
			if (address>=0x50 && address <=0x57)
			{
				// EEPROM - Acks on 8 addresses
				if (has24cxx==0)
				{
					has24cxx=1;
					nDevices++;
				}
				#ifdef doUSB
				if (hasUSB) usb_printf(" - EEPROM 24CXX");				
				#endif
			}
      else nDevices++;
    }
    else if (error == 4) 
	{
      #ifdef doUSB
	  if (hasUSB) usb_printf("\r\nUnknown error at address 0x%X",address);
	  #endif
    }
  }
  if (nDevices == 0)
	{
    #ifdef doUSB
	if (hasUSB) usb_printf("\r\nNo I2C devices found");
	#endif
	}
  else
	{
    #ifdef doUSB
	if (hasUSB) usb_printf("\r\nI2C devices found = %d",nDevices);
	#endif
	}
}

void resetI2C(void)
{
	uint8_t i = 0;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);			
	if (bjCubeType==2) GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; //SCL
	else GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	if (bjCubeType==2) GPIO_Init(GPIOA, &GPIO_InitStructure);
	else GPIO_Init(GPIOC, &GPIO_InitStructure);							//???GPIO???

	if (bjCubeType==2) GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; //SDA
	else GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	if (bjCubeType==2) GPIO_Init(GPIOA, &GPIO_InitStructure);
	else GPIO_Init(GPIOC, &GPIO_InitStructure);							//???GPIO???
	
    delay(50);
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nI2C Bus Recovery");
	#endif
	if (bjCubeType==2)
	{
	  GPIO_WriteBit(GPIOA,GPIO_Pin_11,Bit_SET); //SDA
	  for (i=0;i<10;i++)
	  {
		  GPIO_WriteBit(GPIOA,GPIO_Pin_12,Bit_SET);//SCL;
		  delay(1);
		  GPIO_WriteBit(GPIOA,GPIO_Pin_12,Bit_RESET);//SCL_OFF;
		  delay(1);		
	  }
	  GPIO_WriteBit(GPIOA,GPIO_Pin_11,Bit_RESET); //SDA
	  delay(1);		
	  GPIO_WriteBit(GPIOA,GPIO_Pin_12,Bit_SET);//SCL;
	  delay(1);
	  GPIO_WriteBit(GPIOA,GPIO_Pin_11,Bit_SET); //SDA
	  delay(1);
	}
	else
	{
	  GPIO_WriteBit(GPIOC,GPIO_Pin_11,Bit_SET); //SDA
	  for (i=0;i<10;i++)
	  {
		  GPIO_WriteBit(GPIOC,GPIO_Pin_10,Bit_SET);//SCL;
		  delay(1);
		  GPIO_WriteBit(GPIOC,GPIO_Pin_10,Bit_RESET);//SCL_OFF;
		  delay(1);		
	  }
	  GPIO_WriteBit(GPIOC,GPIO_Pin_11,Bit_RESET); //SDA
	  delay(1);		
	  GPIO_WriteBit(GPIOC,GPIO_Pin_10,Bit_SET);//SCL;
	  delay(1);
	  GPIO_WriteBit(GPIOC,GPIO_Pin_11,Bit_SET); //SDA
	  delay(1);		
	}
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nI2C Bus Recovery Done");
	#endif
}

uint8_t initI2C(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);			
	if (bjCubeType==2) GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; //SCL
	else GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	if (bjCubeType==2) GPIO_Init(GPIOA, &GPIO_InitStructure);
	else GPIO_Init(GPIOC, &GPIO_InitStructure);							//???GPIO???

	if (bjCubeType==2) GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; //SDA
	else GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	if (bjCubeType==2) GPIO_Init(GPIOA, &GPIO_InitStructure);
	else GPIO_Init(GPIOC, &GPIO_InitStructure);							//???GPIO???

	if (LCDType<2)
	{
		// Fast i2c
		seti2cClock(400000);
	}
	else
	{
		// Slow i2c
		seti2cClock(100000);		
	}
  if (beginSoftWire(0)==1)
	{
		//timeout error
		return 1;
	}
	i2cScan();
	return 0;
}
