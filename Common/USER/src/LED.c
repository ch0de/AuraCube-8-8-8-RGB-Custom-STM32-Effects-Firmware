#include "LED.h"
#include "LEDHardware.h"
#include "CubeLink.h"
#include "exfuns.h"

uint8_t Cube_MODE = 2;															// Mode the cube is currently running in
uint8_t OldCube_MODE = 2;														// Used to go back to cube mode
uint8_t AuraCubes = 1;															// How many cubes in total including slave cubes
uint8_t cubeWIFI = 0;                               // Does the cube have wifi - Default no as SD will be required so can read from config
uint8_t hasSD = 0;      														// Is an SD Card Attached ?
uint8_t hasWIFI = 0;      													// Is an ESP8266 WIFI Attached ?
uint8_t hasUSB = 0;                                 // IS USB Serial debugging enabled ?
char wifiSSID[32];																  // WIFI SSID
char wifiPSWD[32];															  	// WIFI Password
char myIPAddress[16]="";                            // IP Address
char myGW[16]="";                                   // Gateway
char myMask[16]="";                                 // Mask

char NTPServer[32]="time.nist.gov";                 // Address of NTP Server for Time
char SDDateTime[14]="\0";                           // SD Card Date Time String to Update DS3231
int8_t UTCOffsetH = 10;                             // Hour Offset of local time from UTC Time
int8_t UTCOffsetM = 0;                              // Minute Offset of local time from UTC Time
uint8_t dSavings  = 1;                              // Are we in Daylight Savings or not
uint8_t dateFormat = 0;															// dd/mm/yy or mm/dd/yy date format
uint8_t weekDay = 0;                                // day of the week.0=Sunday
uint8_t hasNTP = 0;                                 // Do we have access to the network time ?
uint8_t hasDS3231 = 0;                              // Do we have access to the RTC Chip on I2C ?
uint8_t hasLCD = 0;																	// Address of LCD if we have one
uint8_t hasBME280 = 0;                              // Address of BME280 sensor if we have one
uint8_t hasSHT31 = 0;                               // Address of SHT31 sensor if we have one
uint8_t has24cxx = 0;																// Do we have a 24cxx eeprom
volatile uint8_t inI2c = 0;                         // Are we already in I2C
uint8_t LCDType=0; 																	// Type of LCD. 0=2004 (4 rows x 20 cols). 1=1602 (2 rows x 16 cols). 2=2004 slower speed 100khz i2c. 3=1602 slower speed 100khz i2c
uint8_t tempUnit=0;																	// Temperature Unit. 0=Celcius 1=Fahrenheit

uint8_t ROffset=0;                                  // Offset of Red in array. Can be used to swap colours
uint8_t GOffset=1;                                  // Offset of Green in array. Can be used to swap colours
uint8_t BOffset=2;                                  // Offset of Blue in array. Can be used to swap colours

char date[12];
char time[10];

volatile float temperature=0;
volatile float humidity=0;
volatile float altitude=0;

volatile uint8_t pressureReads=0;
volatile float totalPressure=0;
volatile float firstPressure=0;
volatile float pressure=0;

uint8_t wifiTimeoutMult=1;                          // Wifi Timeout Multiplier

uint8_t cubeDirection=0;															// Invert or flip the cube

uint8_t Cube_PROGRAM=0;
uint8_t programCounter=0;
uint16_t NowSection=0;

uint32_t randCount=0;
int16_t SpeedSum=0,NumCount=0;
uint8_t Color_red=0,Color_green=0,Color_blue=0;
uint8_t light_red=1,light_green=0,light_blue=0;

uint8_t setAdd=0;
uint8_t showLED=0;
uint8_t power=1;
uint8_t setLED=0;	
uint8_t delaySpeed=10;
uint8_t customBright=4;
uint8_t currentColour=0;
uint8_t textColourMode=0;
uint8_t textColourRandom=0;
uint8_t textMode=1;

uint8_t ESP8266CHANGE=0;

uint8_t DMACommands[10]; // Commands to send DMA
uint8_t DMAAddress=99;
uint32_t DMABAUD=1500000; //1500000;
uint8_t getDMA=0;
uint8_t exitDMA=0;

uint8_t numCubes=1;
uint8_t extendCubes=0; // extend or mirror the multiple cubes
uint8_t swapCubes=0;   // swap the positions of the cubes
uint8_t outOfBounds=0; // Am I off the cube ?
uint8_t hostReAnnounce=1; // Do I re broadcast after each graphic to see if there are new joiners
uint8_t hostCopyMode=0; // Do I copy the host or extend cubes
uint8_t WIFICopyMode=0; // Do I use UDP to send commands to the other cubes

uint8_t shared2=255; // shared random variables to make sure synced cubes on wifi has same random numbers. 255 means not set
uint8_t shared3=0;
uint8_t shared4=0;
uint8_t shared5=0;
uint8_t shared6=0;
uint8_t sharedA=0;
uint8_t sharedB=0;
uint8_t sharedC=0;


//uint8_t program[100][3];	
volatile uint32_t millis_number = 0;
volatile uint32_t old_millis = 0; 
volatile uint32_t WIFI_old_millis = 0; 

uint32_t millis()
{
	return millis_number;
}

void delay(uint32_t ms)
{
	unsigned long timeIn = millis();
	while(timeIn + ms > millis());
}

void SysTick_Handler(void)
{
  millis_number++;
}

void setPlaneX(uint8_t x, uint8_t R, uint8_t G, uint8_t B) 
{
  uint8_t y,z;
	if (layerColourSame==1) setLayerTimings(y,R,G,B);
  for (y = 0;  y < curTotalY;  y++) {
    for (z = 0;  z < curTotalZ;  z++) {
      set(x, y, z, R, G, B);
    }
  }
}

void setPlaneY(uint8_t y, uint8_t R, uint8_t G, uint8_t B) 
{
  uint8_t x,z;
	if (layerColourSame==1) setLayerTimings(y,R,G,B);
  for (x = 0;  x < curTotalX;  x++) {
    for (z = 0;  z < curTotalZ;  z++) {
      set(x, y, z, R, G, B);
    }
  }
}

void setPlaneZ(uint8_t z, uint8_t R, uint8_t G, uint8_t B) 
{
  uint8_t x,y;
	if (layerColourSame==1) setLayerTimings(y,R,G,B);
  for (y = 0;  y < curTotalY;  y++) {
    for (x = 0;  x < curTotalX;  x++) {
      set(x, y, z, R, G, B);
    }
  }
}

void dimmer(uint8_t R, uint8_t G, uint8_t B, uint8_t *oR, uint8_t *oG, uint8_t *oB, float percent)
{
	 float calcR, calcG, calcB;
	 calcR=R-(R*percent);
	 calcG=G-(G*percent);
	 calcB=B-(B*percent);
	 if (calcR>0) *oR=(int)calcR;
	 else *oR=0;
	 if (calcG>0) *oG=(int)calcG;
   else *oG=0; 
	 if (calcB>0) *oB=(int)calcB;
	 else *oB=0;
}

void brighter(uint8_t R, uint8_t G, uint8_t B, uint8_t *oR, uint8_t *oG, uint8_t *oB, float percent)
{
	 if (R==0 && G==0 && B==0) {R=10;G=10;B=10;}
	 if (R*(1+percent)<255) *oR=R*(1+percent);
	 else *oR=255;
	 if (G*(1+percent)<255) *oG=G*(1+percent);
	 else *oG=255;
	 if (B*(1+percent)<255) *oB=B*(1+percent);
	 else *oB=255;
}

void hueToRGB(int hue, int brightness, uint8_t *oR, uint8_t *oG, uint8_t *oB)
{
    unsigned int scaledHue = (hue * 6);
    unsigned int segment = scaledHue / 256; // segment 0 to 5 around the
                                            // color wheel
    unsigned int segmentOffset =
      scaledHue - (segment * 256); // position within the segment

    unsigned int complement = 0;
    unsigned int prev = (brightness * ( 255 -  segmentOffset)) / 256;
    unsigned int next = (brightness *  segmentOffset) / 256;

    switch(segment ) {
    case 0:      // red
        *oR = brightness;
        *oG = next;
        *oB = complement;
    break;
    case 1:     // yellow
        *oR = prev;
        *oG = brightness;
        *oB = complement;
    break;
    case 2:     // green
        *oR = complement;
        *oG = brightness;
        *oB = next;
    break;
    case 3:    // cyan
        *oR = complement;
        *oG = prev;
        *oB = brightness;
    break;
    case 4:    // blue
        *oR = next;
        *oG = complement;
        *oB = brightness;
    break;
   case 5:      // magenta
    default:
        *oR = brightness;
        *oG = complement;
        *oB = prev;
    break;
    }
}

void hsvToRGB(unsigned int hue, unsigned int sat, unsigned int val, \
              unsigned char * r, unsigned char * g, unsigned char * b, unsigned char maxBrightness ) 
{ 
  unsigned int H_accent = hue/60;
  unsigned int bottom = ((255 - sat) * val)>>8;
  unsigned int top = val;
  unsigned char rising  = ((top-bottom)  *(hue%60   )  )  /  60  +  bottom;
  unsigned char falling = ((top-bottom)  *(60-hue%60)  )  /  60  +  bottom;

  switch(H_accent) {
  case 0:
    *r = top;
    *g = rising;
    *b = bottom;
    break;

  case 1:
    *r = falling;
    *g = top;
    *b = bottom;
    break;

  case 2:
    *r = bottom;
    *g = top;
    *b = rising;
    break;

  case 3:
    *r = bottom;
    *g = falling;
    *b = top;
    break;

  case 4:
    *r = rising;
    *g = bottom;
    *b = top;
    break;

  case 5:
    *r = top;
    *g = bottom;
    *b = falling;
    break;
  }
  // Scale values to maxBrightness
  *r = *r * maxBrightness/255;
  *g = *g * maxBrightness/255;
  *b = *b * maxBrightness/255;
}

uint8_t onThisCube(uint8_t x,uint8_t y,uint8_t z)
{
	 uint8_t result=1;
	 if ((x>=myTotalX || y>=myTotalY || z>=myTotalZ) && (!swapCubes || !extendCubes)) result=0; //not on this cube
	 if ((x<myTotalX && y<myTotalY && z<myTotalZ) && (swapCubes && extendCubes)) result=0;
	 return result;
}

int isClear(uint8_t x,uint8_t y,uint8_t z)
{
	 uint8_t r,g,b;
	 uint8_t ret;
	 getRGB(x,y,z,&r,&g,&b);
   //if (getR(x,y,z)==0 && getG(x,y,z)==0 && getB(x,y,z)==0) ret=1;
	 if(r==0 && g==0 && b==0) ret=1;
   else ret=0;	
   return(ret);	
}

int isClearBuffer(uint8_t buffer,uint8_t x,uint8_t y,uint8_t z)
{
	 uint8_t r,g,b;
	 uint8_t ret;
	 uint8_t oldSet;
	 oldSet=setLED;
	 setLED=buffer;
	 getRGB(x,y,z,&r,&g,&b);
   //if (getR(x,y,z)==0 && getG(x,y,z)==0 && getB(x,y,z)==0) ret=1;
	 if(r==0 && g==0 && b==0) ret=1;
   else ret=0;	
   setLED=oldSet;
   return(ret);	
}

int isClearAll(void)
{
	uint16_t x,y,z;
	for (x=0;x<curTotalX;x++)
	{
		for (y=0;y<curTotalY;y++)
		{
			for (z=0;z<curTotalZ;z++)
			{
				if (!isClear(x,y,z)) return(0);
			}
		}
	}
	return(1);
}

int isClearAllBuffer(uint8_t buffer)
{
	uint16_t x,y,z;
	for (x=0;x<curTotalX;x++)
	{
		for (y=0;y<curTotalY;y++)
		{
			for (z=0;z<curTotalZ;z++)
			{
				if (!isClearBuffer(buffer,x,y,z)) return(0);
			}
		}
	}
	return(1);
}

int isOnCube(int8_t x, int8_t y, int8_t z)
{
  // Is this cell on the cube
  if (x>=0 && y>=0 && z>=0 && x<curTotalX && y<curTotalY && z<curTotalZ) return (1);
  return (0);	
}

void clear(uint8_t x,uint8_t y,uint8_t z)
{
	set(x,y,z,0,0,0);
}

/*******************************************************************************
 * @name     :void ClearLayer(uint16_t layerA)
 * @brief    :
 * @param    :layerA����Ҫ�������ݵĲ�
 * @return   :��
 * @version  :V1.0
 * @author   :�������������ҡ�ͼ��
 * @date     :2015-5-31
 * @details  :�������
 *            
*******************************************************************************/
void clearLayer(uint16_t layerA)
{
	uint16_t i = 0;
	layerA = 3*myTotalX*myTotalZ*layerA;
	for(i = 0; i < 3*myTotalX*myTotalZ; i++)
	{
		rgbled[setLED][i+layerA] = 0;
		if (extendCubes) rgbled[3][i+layerA] = 0;
	}
}

/*******************************************************************************
 * @name     :void setALL(void)
 * @brief    :
 * @param    :layerA����Ҫ�������ݵĲ�
 * @return   :��
 * @version  :V1.0
 * @author   :�������������ҡ�ͼ��
 * @date     :2015-5-31
 * @details  :������л���
 *            
*******************************************************************************/
void setAll(uint8_t R, uint8_t G, uint8_t B)
{
	uint16_t x,y,z;
	if (R==0&&G==0&&B==0) clearLayerTimings();
	for (x=0;x<curTotalX;x++)
	{
		for (y=0;y<curTotalY;y++)
		{
			if (layerColourSame==1) setLayerTimings(y,R,G,B);
			for (z=0;z<curTotalZ;z++)
			{
				set(x,y,z,R,G,B);
			}
		}
  }
}

void setMyAll(uint8_t R, uint8_t G, uint8_t B)
{
	// Just set all on this cube
	uint16_t x,y,z;
	if (R==0&&G==0&&B==0) clearLayerTimings();
	for (x=0;x<myTotalX;x++)
	{
		for (y=0;y<myTotalY;y++)
		{
			if (layerColourSame==1) setLayerTimings(y,R,G,B);
			for (z=0;z<myTotalZ;z++)
			{
				set(x,y,z,R,G,B);
			}
		}
	}
}

void setNextCol(uint8_t cR,uint8_t cG,uint8_t cB, uint8_t *r,uint8_t *g,uint8_t *b)
{
	*r=cR;
	*g=cG;
	*b=cB;
}

void nextColour(uint8_t *r,uint8_t *g,uint8_t *b, uint8_t ran)
{
	uint8_t randomNum=0;
	uint8_t myR=0,myG=0,myB=0;
	uint8_t oldR=0,oldG=0,oldB=0;
	
	if (ran==0)
	{
		switch (currentColour)
		{
			case 0 : setNextCol(PINK,r,g,b);
						 break;
			case 1 : setNextCol(BLUE,r,g,b);
						 break;
			case 2 : setNextCol(ORANGE,r,g,b);
						 break;
			case 3 : setNextCol(GREEN,r,g,b);
						 break;
			case 4 : setNextCol(AQUA,r,g,b);
						 break;
			case 5 : setNextCol(YELLOW,r,g,b);
						 break;
			case 6 : setNextCol(PURPLE,r,g,b);
						 break;
			case 7 : setNextCol(RED,r,g,b);
						 break;
			case 8 : setNextCol(WHITE,r,g,b);
						 break;		
		}
		currentColour++;
		if (currentColour>8) currentColour=0;
	}
	else
	{
		// Random but not the same colour
		myR=R;myG=G;myB=B;
		oldR=R;oldG=G;oldB=B;
		while (myR==oldR && myG==oldG && myB==oldB)
		{
			randomNum=cube_random(11);
			switch (randomNum)
			{
				case 0 : setNextCol(PINK,r,g,b);
						 break;
				case 1 : setNextCol(BLUE,r,g,b);
						 break;
				case 2 : setNextCol(ORANGE,r,g,b);
						 break;
				case 3 : setNextCol(GREEN,r,g,b);
						 break;
				case 4 : setNextCol(AQUA,r,g,b);
						 break;
				case 5 : setNextCol(YELLOW,r,g,b);
						 break;
				case 6 : setNextCol(PURPLE,r,g,b);
						 break;
				case 7 : setNextCol(RED,r,g,b);
						 break;
				case 8 : setNextCol(WHITE,r,g,b);
						 break;		
				case 9 : setNextCol(BROWN,r,g,b);
						 break;		
				case 10: setNextCol(OLIVE,r,g,b);
						 break;						
			}
			myR=*r;
			myG=*g;
			myB=*b;
		}
	}
}

