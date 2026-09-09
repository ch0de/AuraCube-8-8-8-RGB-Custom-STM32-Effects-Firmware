#include "sht31.h"

void sht31Command(uint16_t cmd) 
{
	uint8_t data[2];
	data[0]=cmd >> 8;
	data[1]=cmd & 0xFF;
	i2cMasterTransmit(hasSHT31,&data[0],2,0);
}

uint16_t sht31Status(void) 
{
	uint8_t retVal;
	uint8_t data[2];	
	uint16_t stat;

  sht31Command(SHT31_READSTATUS);
	retVal=i2cMasterReceive(hasSHT31,&data[0],2,0);
	if (retVal==0)
	{
		stat=data[0]<<8;
		stat|=data[1];
	}
	else stat=0xff;
  return stat;
}

void sht31Reset(void) 
{
  
	sht31Command(SHT31_SOFTRESET);
  delay(10);
}

void sht31heater(uint8_t h)
{
 if (h)
    sht31Command(SHT31_HEATEREN);
 else
    sht31Command(SHT31_HEATERDIS);
}

uint8_t sht31TempHum(void) 
{
	uint16_t ST, SRH;
	double stemp;
	double shum;
  uint8_t readbuffer[6];
  sht31Command(SHT31_MEAS_HIGHREP);

	if (inI2c!=0) return 1; // Exit if something already using i2c
	inI2c=1;  
  delay(500);
	i2cMasterReceive(hasSHT31,&readbuffer[0],6,0);
  ST = readbuffer[0];
  ST <<= 8;
  ST |= readbuffer[1];
  SRH = readbuffer[3];
  SRH <<= 8;
  SRH |= readbuffer[4];
	
  stemp=ST;
  stemp *= 175;
  stemp /= 0xffff;
  stemp = -45 + stemp;
  temperature = stemp;
  #ifdef doUSB
  if (hasUSB) usb_printf("\r\nSHT31 Temp: %.2f Celcius",temperature);
  if (hasUSB) usb_printf("\r\nSHT31 Temp: %.2f Fahrenheit",temperature*1.8f+32);
  #endif	
 	shum = SRH;
  shum *= 100;
  shum /= 0xFFFF;
  humidity = shum;
  #ifdef doUSB
  if (hasUSB) usb_printf("\r\nHumidity: %.2f",humidity);
  #endif
  inI2c=0;	
	if (hasLCD) 
	{
		delay(5);
		if (lcdMaxRow==4) 
		{
			if (tempUnit==0) lcd_println(3,1,"%.1f%cC %.1f%%RH",temperature,223,humidity); //Celcius
			else lcd_println(3,1,"%.1f%cC %.1f%%RH",temperature*1.8f+32,223,humidity); //Fahrenheit
		}
		else 
		{
			if (tempUnit==0) lcd_println(1,1,"%.1f%cC %.1f%%RH",temperature,223,humidity); //Celcius
			else lcd_println(3,1,"%.1f%cC %.1f%%RH",temperature*1.8f+32,223,humidity); //Fahrenheit
		}
	}	
  return 0;
}

void sht31Init(void)
{
	uint16_t status;
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nSHT31 Init");	
	#endif
	if (hasLCD) lcd_println(99,1,"Init SHT31");
	sht31Reset();
	status=sht31Status();
	if (status!=0xff)
	{
		sht31TempHum();
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nSHT31 Init OK");	
		#endif
		if (hasLCD) lcd_println(98,1,"Init SHT31 - OK");		
	}
	else
	{
		hasSHT31=0;
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nERROR - SHT31 Init");	
		#endif
		if (hasLCD) lcd_println(98,1,"Init SHT31 - ERROR");		
		delay(1000);
	}
}
