#include "lcd.h"

uint8_t lcdMaxCol=20; // Default 2004 LCD
uint8_t lcdMaxRow=4;
uint8_t lcdCurRow=0;
uint8_t timeDispRow=0;

uint8_t uparrow[8] = {0x4,0xe,0xff,0x4,0x4,0x4,0x0};
uint8_t downarrow[8] = {0x0,0x4,0x4,0x4,0xff,0xe,0x4};

volatile uint8_t updateTime=0; // Time to update values ?

uint8_t oldBuf[20]; // Old chars on screen
uint8_t oldBufLine; // Line number of old buffer

uint8_t lcd_send_cmd(char cmd)
{
  char data_u, data_l;
	uint8_t data_t[4];
	data_u = cmd&0xf0;
	data_l = (cmd<<4)&0xf0;
	data_t[0] = data_u|0x04;  //en=1, rs=0
	data_t[1] = data_u;  //en=0, rs=0
	data_t[2] = data_l|0x04;  //en=1, rs=0
	data_t[3] = data_l;  //en=0, rs=0
	return i2cMasterTransmit(hasLCD,data_t,4,0);
}
 
uint8_t lcd_send_data(char data)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = data&0xf0;
	data_l = (data<<4)&0xf0;
	data_t[0] = (data_u|0x05)|0x08;  //en=1, rs=0
	data_t[1] = (data_u|0x01)|0x08;  //en=0, rs=0
	data_t[2] = (data_l|0x05)|0x08;  //en=1, rs=0
	data_t[3] = (data_l|0x01)|0x08;  //en=0, rs=0
	return i2cMasterTransmit(hasLCD,data_t,4,0);
}

void lcd_clear(void)
{
	if (inI2c==0)
	{
		inI2c=1;
		lcd_send_cmd(0x01);
		delay(20);
		inI2c=0;
	}
}

void createChar(uint8_t location, uint8_t charmap[]) 
{
	// Create custom LCD Chars
	uint8_t count;
	lcd_send_cmd(0x40 | (location << 3));
	for(count=0;count<8;count++)
	{
		lcd_send_data(charmap[count]);
	}
}

void lcdInit(void)
{
	uint8_t ret=0;
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nLCD Init");
	#endif
	inI2c=1;
	delay(100);
	ret=lcd_send_cmd(0x03);
	if (ret==0)
	{
		delay(45);
		lcd_send_cmd(0x03);
		delay(45);
		lcd_send_cmd(0x03);
		delay(45);
		lcd_send_cmd(0x02);
		delay(100);
	
		lcd_send_cmd(0x28);
		delay(1);
		lcd_send_cmd(0x0c);
		delay(1);
		lcd_send_cmd(0x80);
		delay(100);
		inI2c=0;
		if (LCDType==1 || LCDType==3)
		{
			// 1602 LCD Screen 2x16
			lcdMaxRow=2;
			lcdMaxCol=16;
		}
		lcd_clear();
		lcdCurRow=0; // Start from row 2
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nLCD Init Done - %d rows x %d cols",lcdMaxRow,lcdMaxCol);
		#endif
		createChar(1,uparrow);
		createChar(2,downarrow);
	}
	else
	{
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nERROR initialising LCD");
		#endif
		hasLCD=0;
	}
}

//void lcd_send_string (char *str)
//{
//	while (*str) lcd_send_data (*str++);
//}

void lcd_printxy(uint8_t line , uint8_t col, char *str)
{
  uint8_t line_x_y=0;
	if (inI2c==0)
	{
		inI2c=1;
		if (line==99)
		{
			lcdCurRow++;
			if (lcdCurRow>=lcdMaxRow) lcdCurRow=0;
			line=lcdCurRow;
		}
		if (line==98)
		{
			line=lcdCurRow;
		}
		if (line==97)
		{
			// next line temporary
			line=lcdCurRow+1;
			if (line>=lcdMaxRow) line=0;
		}
		if (line==96)
		{
			// last row
			line=lcdMaxRow-1;
		}		
		if (line==95)
		{
			// Bottom of top half
			if (lcdMaxRow==4) line=1;
			else line=0;
		}
		if (line==94)
		{
			// Top of bottom half
			if (lcdMaxRow==4) line=2;
			else line=1;
		}	
		if (line<lcdMaxRow)
		{
			// Make sure this LCD has enough lines
			if (line == 0) line_x_y=col;
			else if (line==1) line_x_y=0x40+col;
			else if (line==2) line_x_y=0x14+col;
			else if (line==3) line_x_y=0x54+col;
			lcd_send_cmd(line_x_y|0x80); // 0x80 is move command
			while (*str) lcd_send_data (*str++);
		}
		if (line==timeDispRow) timeDispRow=0; // Overwritten Time display so don't update
		inI2c=0;
	}
}

void lcd_println(uint8_t line, uint8_t pad, char* fmt, ...)
{
	uint8_t lcdBuf[50];
	u16 i;
	va_list ap;
	i = strlen((const char*)fmt);
	if (i<50)
	{
		va_start(ap, fmt);
		vsprintf((char*)lcdBuf, fmt, ap);
		va_end(ap);
		i = strlen((const char*)lcdBuf);
		if (i>lcdMaxCol)
		{
			i=lcdMaxCol;
			lcdBuf[i]='\0';
		}
		else
		{
			if (pad) 
			{
				memset(&lcdBuf[i],' ',lcdMaxCol-i);
				lcdBuf[lcdMaxCol]='\0';
			}
		}
		lcd_printxy(line,0,(char*)lcdBuf);
	}
  else
  {
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nString too big for lcd");
		#endif
	}
}

void lcd_printBuf(uint8_t line, uint8_t pad, char* fmt, ...)
{
	uint8_t lcdBuf[50];
	uint8_t change=0;
	u16 i;
	va_list ap;
	i = strlen((const char*)fmt);
	if (i<50)
	{
		va_start(ap, fmt);
		vsprintf((char*)lcdBuf, fmt, ap);
		va_end(ap);
		i = strlen((const char*)lcdBuf);
		if (i>lcdMaxCol)
		{
			i=lcdMaxCol;
			lcdBuf[i]='\0';
		}
		else
		{
			if (pad) 
			{
				memset(&lcdBuf[i],' ',lcdMaxCol-i);
				lcdBuf[lcdMaxCol]='\0';
			}
		}
		if (line!=oldBufLine)
		{
			oldBufLine=line;
			change=1;
		}
		for (i=0;i<20;i++)
		{
			if ((lcdBuf[i]!=oldBuf[i])||change)
			{
				// diff chars
				lcd_printxy(line,0,(char*)&lcdBuf[i]);
				oldBuf[i]=lcdBuf[i]; // copy for next time
			}
		}
		lcd_printxy(line,0,(char*)lcdBuf);
	}
  else
  {
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nString too big for lcd");
		#endif
	}	
}

void lcd_clearln(uint8_t line)
{
	uint8_t space[2];
	space[0]=' ';
	space[1]='\0';
	lcd_println(line,1,(char*)space);
}

void lcd_printDateTime(uint8_t row, uint8_t printSec)
{
	char dayOfWeek[5]="\0";
	char fullDayOfWeek[10]="\0";
	if (weekDay==0) 
	{
		strcpy(dayOfWeek,"Sun ");
		strcpy(fullDayOfWeek,"Sunday");
	}
	if (weekDay==1) 
	{
		strcpy(dayOfWeek,"Mon ");
		strcpy(fullDayOfWeek,"Monday");		
	}
	if (weekDay==2) 
	{
		strcpy(dayOfWeek,"Tue ");
		strcpy(fullDayOfWeek,"Tuesday");		
	}
	if (weekDay==3) 
	{
		strcpy(dayOfWeek,"Wed ");
		strcpy(fullDayOfWeek,"Wednesday");		
	}
	if (weekDay==4) 
	{
		strcpy(dayOfWeek,"Thu ");
  	strcpy(fullDayOfWeek,"Thursday");
	}
	if (weekDay==5) 
	{
		strcpy(dayOfWeek,"Fri ");
		strcpy(fullDayOfWeek,"Friday");		
	}
	if (weekDay==6) 
	{
		strcpy(dayOfWeek,"Sat ");
		strcpy(fullDayOfWeek,"Saturday");		
	}
	if (printSec==0) 
	{
		if (lcdMaxCol>=20)
		{		
			lcd_println(row,1,"%s%s %.5s",dayOfWeek,date,time);
		}
		else
		{
			lcd_println(row,1,"%s %.5s",date,time);
		}
	}
	else 
	{
		if (lcdMaxCol>=20)
		{
			if (strlen(date)==8&&strlen(time)==8) // not going to fit
			{
				dayOfWeek[3]='\0'; // Chop dow space
			}
			lcd_printBuf(row,1,"%s%s %s",dayOfWeek,date,time);
		}
		else
		{
			lcd_printBuf(row,1,"%s %s",date,time);			
		}
	}
	timeDispRow=row;
}

void lcd_printWeather(void)
{
  char trend;
	if (lcdMaxRow==4)
	{
		  if (tempUnit==0) lcd_println(1,1,"Temp: %.2f%cC",temperature*1.8f+32,223); // Celcius
		  else lcd_println(1,1,"Temp: %.1f%cF",temperature*1.8f+32,223); //Fahrenheit		
		  lcd_println(1,1,"Humidity: %.2f%%RH",temperature*1.8f+32,223,humidity); // Celcius		
		  if (pressure!=0) lcd_println(3,1,"Pres: %.2fhPa %c",pressure/100,trend);		
		  else lcd_clearln(3);
	}
	else
	{
			if (tempUnit==0) lcd_println(0,1,"%.1f%cC %.1f%%RH",temperature*1.8f+32,223,humidity); // Celcius
			else lcd_println(0,1,"%.0f%cF %.1f%%RH",temperature*1.8f+32,223,humidity); //Fahrenheit		
		  if (pressure!=0) lcd_println(1,1,"%.2fhPa %c",pressure/100,trend);
		  else lcd_clearln(1);
	}
}

void updateNow(void)
{
   // Update variable from i2c devices	
		//if (timeDispRow>0 && inI2c==0)
		if (inI2c==0)
		{
			// Put Time on Display
		  if (hasDS3231)getRTCDateTime();
		}
		if (hasBME280)
		{
			BME280Update();
		}
		if (hasSHT31 && !hasBME280)
		{
			sht31TempHum();
		}			
}

void doi2cUpdates(void)
{
  updateTime++;
	if (updateTime>10) // About every 5 minutes ?
	{
		updateNow();
		updateTime=0;
	}
	else
	{
		if (timeDispRow>0 && inI2c==0)
		{
			// time is on the display - update it
		  if (hasDS3231)getRTCDateTime();
		}
	}
}
