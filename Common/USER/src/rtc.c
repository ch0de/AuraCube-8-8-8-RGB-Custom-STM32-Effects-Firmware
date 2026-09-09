
#include "rtc.h"

DS3231_date_TypeDef DS3231_dTime;

uint8_t BCDencode(uint8_t value)
{
	uint8_t encoded = ((value / 10) << 4) + (value % 10);
	return encoded;
}

void DS3231_ReadDateRAW(RAW_date_TypeDef* date) {
	uint8_t buffer[7];
	uint8_t data=DS3231_seconds;
	i2cMasterTransmit(DS3231_addr,&data,1,1);	
	i2cMasterReceive(DS3231_addr,&buffer[0],7,0);
	memcpy(date,&buffer[0],7);
}

void DS3231_WriteDate(DS3231_date_TypeDef* date) {
	uint8_t buffer[8];
  RAW_date_TypeDef rawDate;

	buffer[0]=DS3231_seconds;
	rawDate.dow=BCDencode(date->dow);
	rawDate.day=BCDencode(date->day);
	rawDate.month=BCDencode(date->month);
	rawDate.year=BCDencode(date->year);
	rawDate.hour=BCDencode(date->hour);
	rawDate.min=BCDencode(date->min);
	rawDate.sec=BCDencode(date->sec);

  memcpy(&buffer[1],&rawDate,7);
  i2cMasterTransmit(DS3231_addr,&buffer[0],8,0);
}

void DS3231_ReadDate(DS3231_date_TypeDef* hrf_date) {
	RAW_date_TypeDef raw_date;
	DS3231_ReadDateRAW(&raw_date);

	hrf_date->sec = (raw_date.sec >> 4) * 10 + (raw_date.sec & 0x0f);
	hrf_date->min = (raw_date.min >> 4) * 10 + (raw_date.min & 0x0f);
	hrf_date->hour = (raw_date.hour   >> 4) * 10 + (raw_date.hour   & 0x0f);
	hrf_date->day = (raw_date.day    >> 4) * 10 + (raw_date.day    & 0x0f);
	hrf_date->month = (raw_date.month   >> 4) * 10 + (raw_date.month   & 0x0f);
	hrf_date->year = (raw_date.year    >> 4) * 10 + (raw_date.year    & 0x0f);
	hrf_date->dow = raw_date.dow;
}

float DS3231_ReadTemp(void) 
{
	float temp;
	uint8_t buffer[2];
	uint8_t tMSB=0,tLSB=0;
	uint8_t data=DS3231_tmp_MSB;

	if (inI2c==0)
	{
		inI2c=1;
		i2cMasterTransmit(DS3231_addr,&data,1,1);	
		i2cMasterReceive(DS3231_addr,&buffer[0],2,0);
		inI2c=0;
	}
	tMSB=buffer[0];
	tLSB=buffer[1];
	temp = tMSB + ((tLSB >> 6) * 0.25);
	if (hasLCD) lcd_println(3,1,"Temperature = %.2f",temp);
	return temp;
}

void getRTCDateTime(void)
{
	if (inI2c==0)
	{
		inI2c=1;
		DS3231_ReadDate(&DS3231_dTime);
		inI2c=0;
		month=DS3231_dTime.month;
		if (dateFormat==0) sprintf(date,"%d/%0d/%d",DS3231_dTime.day,DS3231_dTime.month,DS3231_dTime.year);
		else sprintf(date,"%d/%0d/%d",DS3231_dTime.month,DS3231_dTime.day,DS3231_dTime.year);
		sprintf(time,"%02d:%02d.%02d",DS3231_dTime.hour,DS3231_dTime.min,DS3231_dTime.sec);
		weekDay=DS3231_dTime.dow-1; // DS3231 uses 1=Sun
		if (hasLCD)
		{
			if (timeDispRow>0)
			{
				// Stay on the same row as last time
				if (modeC==89) lcd_printDateTime(timeDispRow,1); // With seconds
				else lcd_printDateTime(timeDispRow,0);			
			}
			else
			{
				if (modeC==89) lcd_printDateTime(94,1); // With seconds
				else lcd_printDateTime(94,0);
			}
		}
	}
}

void checkUpdateDS3231(uint8_t weekday,uint8_t d,uint8_t m,uint8_t y,uint8_t localH,uint8_t localM,uint8_t localS)
{
	uint8_t needsUpdate=0;
	DS3231_ReadDate(&DS3231_dTime);
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nNTP Day %d, Month %d, Year %d, DOW %d, Time %d:%02d.%02d",d,m,y,weekday,localH,localM,localS);
	if (hasUSB) usb_printf("\r\nRTC Day %d, Month %d, Year %d, DOW %d, Time %d:%02d.%02d",DS3231_dTime.day,DS3231_dTime.month,DS3231_dTime.year,DS3231_dTime.dow-1,DS3231_dTime.hour,DS3231_dTime.min,DS3231_dTime.sec);
	#endif
	if (DS3231_dTime.dow-1!=weekday) needsUpdate=1; // DS3231 uses 1=Sun
	if (DS3231_dTime.day!=d) needsUpdate=1;
	if (DS3231_dTime.month!=m) needsUpdate=1;
	if (DS3231_dTime.year!=y) needsUpdate=1;
	if (DS3231_dTime.hour!=localH) needsUpdate=1;
	if (DS3231_dTime.min+1<localM || localM+1<DS3231_dTime.min) needsUpdate=1; // Within 1 minutes
	if (needsUpdate==1)
	{
		inI2c=1;
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nRTC needs update from NTP - Does the battery need replacing ?");
		#endif
		delay(100);
		DS3231_dTime.dow=weekday+1; // DS3231 uses 1=Sun
		DS3231_dTime.day=d;
		DS3231_dTime.month=m;
		DS3231_dTime.year=y;
		DS3231_dTime.hour=localH;
		DS3231_dTime.min=localM;
		DS3231_dTime.sec=localS;
		DS3231_WriteDate(&DS3231_dTime);
		delay(100);
		inI2c=0;
	}
	else
	{
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nRTC already accurate");
		#endif
	}
	hasNTP=2; // Don't use NTP
	hasDS3231=2; // Accurate Now
}

void updateSDDateTime()
{
	char temp[3];
	temp[2]=0;
	inI2c=1;
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nUpdating RTC from SD Card DateTime %s",SDDateTime);
	#endif
	delay(100);
	temp[0]='0';
	temp[1]=SDDateTime[0];
	DS3231_dTime.dow=atoi(temp)+1; // DS3231 uses Sun=1
	temp[0]=SDDateTime[1];
	temp[1]=SDDateTime[2];	
	DS3231_dTime.day=atoi(temp);
	temp[0]=SDDateTime[3];
	temp[1]=SDDateTime[4];		
	DS3231_dTime.month=atoi(temp);
	temp[0]=SDDateTime[5];
	temp[1]=SDDateTime[6];		
	DS3231_dTime.year=atoi(temp);
	temp[0]=SDDateTime[7];
	temp[1]=SDDateTime[8];		
	DS3231_dTime.hour=atoi(temp);
	temp[0]=SDDateTime[9];
	temp[1]=SDDateTime[10];		
	DS3231_dTime.min=atoi(temp);
	temp[0]=SDDateTime[11];
	temp[1]=SDDateTime[12];		
	DS3231_dTime.sec=atoi(temp);
	DS3231_WriteDate(&DS3231_dTime);
	delay(100);
	inI2c=0;
}

void initDS3231(void)
{
	uint8_t ret=0;
	uint8_t data[3];
	data[0]=DS3231_control;
	data[1]=0x24;
	data[2]=0x00;
	
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\ninitDS3231");
	#endif
	if (hasLCD) lcd_println(99,1,"Init RTC");
	ret=i2cMasterTransmit(DS3231_addr,&data[0],0,0);	
	if (ret==0)
	{	
		// Wait for DS3231 startup
		delay(1000);
		// DS3231 init
		i2cMasterTransmit(DS3231_addr,&data[0],3,0);	
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nRTC DS3231 Init OK");
		#endif
		if (hasLCD) lcd_println(98,1,"Init RTC - OK");
		hasDS3231=1;
		if (strlen(SDDateTime)!=0) updateSDDateTime(); // Update DS3231 from SD CARD
	}
	else
	{
		hasDS3231=0;
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nUnable to communicate with DS3231");
		#endif
		if (hasLCD) lcd_println(98,1,"Init RTC - ERROR");
	}
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\n");
	#endif
}
