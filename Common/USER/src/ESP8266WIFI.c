#include "ESP8266WIFI.h"
#include "usart.h"
#include "ESP8266_AT.h"
#include "stdio.h"
#include "System.h"
#include "string.h"
#include "IR_NEC.h"
#include "cubefile.h"	 
#include "exfuns.h"
#include "LED.h"
#include "effects.h"
#include "graphics.h"
#include "CubeLink.h"

////////////////////////
// Buffer Definitions //
////////////////////////

USART_TypeDef* ESP8266_USART=USART3;
char esp8266RxBuffer[ESP8266_RX_BUFFER_LEN];
char tcpRxBuffer[TCP_RX_BUFFER_LEN];
volatile uint16_t bufStart = 0; // Holds position of latest byte placed in buffer.
volatile uint16_t oldBufStart = 0; // Holds position of latest byte placed in buffer.
volatile uint16_t bufEnd = 0; // Holds position of latest byte placed in buffer.
volatile uint16_t bufTemp = 0; // Holds position of latest byte placed in buffer.
volatile uint16_t tcpBufferHead = 0;
volatile uint8_t tcpBufferClient = 0;
char page[80]; // current page the client is on

char receiveState = 0x00;
volatile uint8_t inSend=0;
volatile uint8_t sendUDP=0;
volatile uint8_t inIPD=99;
volatile uint8_t lastKey=0;
volatile uint8_t inCounter=0;
volatile uint8_t expectPOST=0;
char c[100];

////////////////////
// Initialization //
////////////////////

bool esp8266Begin()
{
	bool test = FALSE;
	test = esp8266Test();
	if(test)
	{
		if (esp8266SetMux(1))
			return TRUE;
	}
	return FALSE;
}

///////////////////////
// Basic AT Commands //
///////////////////////

bool esp8266Test()
{
	//esp8266SendCommand(ESP8266_TEST, ESP8266_CMD_EXECUTE, 0);
	esp8266ClearBuffer();
	usartSendArrar(ESP8266_USART, "AT\r\n");
	if(esp8266ReadForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT))
		return TRUE;
	return FALSE;
}

void resetSTM(void)
{
	// reset the cube. This can be called from non WIFI - so don't send wifi commands unless we have it
	setAll(BLACK);
	if (hasWIFI)
	{
		delay(20);
		strcpy(c,"HTTP/1.1 404 File Not Found\r\nConnection: Close\r\n\r\n");  // HTTP Header  // Send Not Found
		esp8266TcpSend(tcpBufferClient,strlen(c));
		#ifdef doUSB
		usb_printf("\r\nSent done");
		#endif
		esp8266TcpSendBuffer(c,strlen(c));
		#ifdef doUSB
		usb_printf("\r\nSent Buffer done");
		#endif
		delay(20);
		esp8266TcpClose(5); // Close All Connections
		#ifdef doUSB
		usb_printf("\r\nClosed");
		#endif
	}
	if (DMAAddress==0)
	{
		stopDMATransfer(); // Tell cubes to stop listening
		delay(300);
		// If Host
		sendDMA=0; // STOP DMA SEND
		//while (inSendDMA!=0) {}  // Wait for Transfer to finish
		delay(300);
	}
	#ifdef doUSB
	usb_printf("\r\nDMA Stopped");
	#endif
	delay(300);
	NVIC_SystemReset();
}

void resetCube(void)
{
	if (hasLCD)
	{
		lcd_clear();
		lcd_println(0,0,"Resetting...");
	}
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nResetting STM CUBE");	
	#endif
	setLED=0;
	setAll(RED);
	showLED=0;
 	delay(500);
 	resetSTM();
}

void esp8266ChangeBaud()
{
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nCHANGING ESP8266 BAUD...");
	#endif
	if (hasLCD)
	{
		lcd_clear();
		lcd_println(0,0,"Changing");
		lcd_println(1,0,"ESP8266 Speed...");
	}
	if (ledCubeType>1) esp8266SendCommand(ESP8266_UART, ESP8266_CMD_SETUP, "1500000,8,1,0,0\r\n");
	else esp8266SendCommand(ESP8266_UART, ESP8266_CMD_SETUP, "1000000,8,1,0,0\r\n");		
	delay(10000);
	while(1) {} // Loop Forever
}

void esp8266Change115200()
{
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nCHANGING ESP8266 BAUD TO 115200...");
	#endif
	if (hasLCD)
	{
		lcd_clear();
		lcd_println(0,0,"Changing ESP8266");
		lcd_println(1,0,"Speed 115200");
	}	
	esp8266SendCommand(ESP8266_UART, ESP8266_CMD_SETUP, "115200,8,1,0,0\r\n");	
	delay(10000);
	resetCube();
}

void esp8266Change9600()
{
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nCHANGING BAUD...");
	#endif
	esp8266SendCommand(ESP8266_UART, ESP8266_CMD_SETUP, "9600,8,1,0,0\r\n");	
	delay(10000);
	resetCube();
}

void ESP8266ChangeSpeed()
{
	power=2; // PAUSE
	if ((ESP8266BAUD!=1500000 && ledCubeType>1) || (ESP8266BAUD!=1000000 && ledCubeType<2)) 
	{
		// only change if it is not set already
		setAll(BLUE);
		esp8266ChangeBaud();
	}
	else setAll(RED);
}
////////////////////
// WiFi Functions //
////////////////////

int16_t esp8266GetMode()
{
	bool rsp = FALSE;
	char* p, mode;
	esp8266SendCommand(ESP8266_WIFI_MODE, ESP8266_CMD_QUERY, 0);
	rsp = esp8266ReadForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	if (rsp)
	{
		// Then get the number after ':':
		p = strchr(esp8266RxBuffer, ':');
		if (p != NULL)
		{
			mode = *(p+1);
			if ((mode >= '1') && (mode <= '3'))
				return (mode - 48); // Convert ASCII to decimal
		}
		
		return ESP8266_RSP_UNKNOWN;
	}
	
	return rsp;
}

bool esp8266SetMode(esp8266_wifi_mode mode)
{
	char modeChar[2] = {0, 0};
	sprintf(modeChar, "%d", mode);
	esp8266SendCommand(ESP8266_WIFI_MODE, ESP8266_CMD_SETUP, modeChar);
	return esp8266ReadForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
}

bool esp8266SetTimeout()
{
	// Set the server timeout to 2 hours
	bool rc = FALSE;
	esp8266SendCommand(ESP8266_SET_SERVER_TIMEOUT,ESP8266_CMD_SETUP,"0"); // Zero = No Time Out
  rc = esp8266ReadForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	return(rc);
}

void getDayOfWeek(uint8_t date, uint8_t mon, uint8_t year)
{ 
 int month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; 
 int i, s = 0 ; 
 if( (year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0)) ) 
  month[1] = 29 ; 
 for(i = 0 ; i < mon - 1 ; i++) s = s + month[i] ; 
 s = s + (date + year + (year / 4) - 2) ; 
 s = s % 7 ; 
 weekDay=s;
}

void processSlaveUDP(void)
{
	uint8_t gotKey=0;
	uint8_t gotmodeNext=0;
	uint8_t gotmodeC=0;
	uint32_t construct=0;
	uint32_t oldgCounterMax=0;
	
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nReceived and Processing a UDP sync message from a host cube");
	#endif

	oldgCounterMax=gCounterMax;
	gotKey=tcpRxBuffer[2];
	gotmodeC=tcpRxBuffer[3];
	gotmodeNext=tcpRxBuffer[8];
	
	if (gotmodeNext==4) modeNext=4; // Remain
	if (gotKey!=0 && gotKey!=255) KeyHandle(gotKey);
	
	power=tcpRxBuffer[0];   
  Cube_MODE=tcpRxBuffer[1];    
  if (welcome==1) welcome=tcpRxBuffer[4];
	delaySpeed=tcpRxBuffer[5];
	customBright=tcpRxBuffer[6];
	allowModeWait=tcpRxBuffer[7];
	doTransitions=tcpRxBuffer[9];
	fadeMax=tcpRxBuffer[10];
	construct = (uint32_t) tcpRxBuffer[14] << 24;
	construct |=  (uint32_t) tcpRxBuffer[15] << 16;
	construct |= (uint32_t) tcpRxBuffer[16] << 8;
	construct |= (uint32_t) tcpRxBuffer[17]; 
	gCounterMax=construct*timeMultipler; // Make sure other cube finishes first 
  shared2=tcpRxBuffer[18];
  shared3=tcpRxBuffer[19];
  shared4=tcpRxBuffer[20];
  shared5=tcpRxBuffer[21];
  shared6=tcpRxBuffer[22];
  sharedA=tcpRxBuffer[23];
  sharedB=tcpRxBuffer[24];
  sharedC=tcpRxBuffer[25];
 
  if (gotmodeC!=modeC || gotKey==255)  // Do this at the end coz it runs on a separate timer
	{
		modeNextG=gotmodeC;
		specificGraphic();
	}
	else
	{
		if (gCounterMax!=oldgCounterMax) gCleanup();
		if (gotKey==254) {modeNext=0;counterNum=gCounterMax;} // Force the graphic to enter fade out
	}
}

void interpretNTPTime(void)
{
	unsigned long low=0;
	unsigned long high=0;
	unsigned long secSince1900=0;
	const unsigned long seventyYears = 2208988800UL;
  // subtract seventy years:
  unsigned long epoch = 0;
	uint8_t epochH = 0;
	uint8_t epochM = 0;
	uint8_t epochS = 0;
	uint8_t localH = 0;
	uint8_t localM = 0;
	uint8_t localS = 0;
	int8_t remain = 0;
  int z;
  int era;
  unsigned doe;
  unsigned yoe;
  int y;
  unsigned doy;
  unsigned mp;
  unsigned d;
  unsigned m;

	high=tcpRxBuffer[40] <<8 | tcpRxBuffer[41];
	low=tcpRxBuffer[42] <<8 | tcpRxBuffer[43];
	secSince1900 = high << 16 | low;
	epoch = secSince1900 - seventyYears;
	srand(epoch);  // Seed Random Numbers on the epoch time
	epochH = (epoch % 86400L) / 3600; // 86400 secs per day
	epochM = (epoch % 3600) / 60; // 3600 secs per minute
	epochS = (epoch % 60); // 60 seconds per minute
	if (UTCOffsetH<0) UTCOffsetM=-UTCOffsetM;  // assume if offset hour is neg so is min
	localS=epochS;
	if (epochM+UTCOffsetM<0)
	{
		 localM=60+(epochM+UTCOffsetM); // this adds a negative
		 remain=-1;
	}
	else 
	{
		 localM=epochM+UTCOffsetM;
		 if (localM>=60) 
		 {	
				localM=localM-60;
				remain=1;
		 }
	}
	if (epochH+remain+UTCOffsetH+dSavings<0)
	{
		 localH=24+(epochH+remain+UTCOffsetH+dSavings);  // this adds a negative
 		 remain=-1;
	}
	else 
	{
			localH=epochH+remain+UTCOffsetH+dSavings;
		  if (localH>=24) 
			{
				localH=localH-24;
				remain=1;
			}
	}
	z = (epoch+remain*86400L) / 86400 + 719468; // Add remain from time
  era = (z >= 0 ? z : z - 146096) / 146097;
  doe = (unsigned)(z - era * 146097);
  yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;
  y = (int)(yoe) + era * 400;
  doy = doe - (365*yoe + yoe/4 - yoe/100);
  mp = (5*doy + 2)/153;
  d = doy - (153*mp+2)/5 + 1;
  m = mp + (mp < 10 ? 3 : -9);
  y += (m <= 2);
	getDayOfWeek(d,m,y);
	month=m;
	sprintf(time,"%02d:%02d.%02d",epochH,epochM,epochS);
	#ifdef doUSB
	if (hasUSB)
	{
		usb_printf("\r\nNTP. Seconds since Jan 1 1900 = ");
		usb_printf("%lu",secSince1900);	
		usb_printf("\r\nUTC Time is %s\r\n",time);
	}
	#endif	
	sprintf(time,"%02d:%02d.%02d",localH,localM,localS);
	y=y-2000; // turn 4 digit year into 2 digits
	if (dateFormat==0) sprintf(date,"%d/%0d/%d",d,m,y);
	else sprintf(date,"%d/%0d/%d",m,d,y);
	#ifdef doUSB
	if (hasUSB)
	{
		usb_printf("\r\nLocal Time is %s",time);
		usb_printf("\r\nDate is %s",date);
		usb_printf("\r\nDate of Week is %d",weekDay);
	}	
	#endif	
	if (hasDS3231==1) checkUpdateDS3231(weekDay,d,m,y,localH,localM,localS);
	else 	hasNTP=1;
}

uint8_t updateDateTime(void)
{
	 uint8_t ret=0;
	 uint16_t counterA=0;
	 memset(date, 32, 12);	
	 memset(time, 32, 10);	
	 if (hasDS3231==2 || (hasDS3231==1 && hasWIFI==0))
	 {
		  getRTCDateTime();
		  #ifdef doUSB
		  if (hasUSB) usb_printf("\r\nRTC Date %s",date);
		  #endif
	    #ifdef doUSB
		if (hasUSB) usb_printf("\r\nRTC Time %s\r\n",time);
		#endif
	 }
	 else
	 {
			if (hasWIFI && UTCOffsetH!=99) 
			{		
			  //delay(200);
				hasNTP=10; // update date time if we have WIFI
				while ((hasNTP!=1 && hasNTP!=2) && counterA<400) 
				{
					 // wait for NTP
					 delay(20);
					 counterA++;
				}
				if (hasNTP)
				{
					#ifdef doUSB
					if (hasUSB) usb_printf("\r\nNTP Date %s",date);
					#endif
					#ifdef doUSB
					if (hasUSB) usb_printf("\r\nNTP Time %s\r\n",time);
					#endif
				}
				else
				{
					if (hasDS3231>0)
					{
							getRTCDateTime();
							#ifdef doUSB
							if (hasUSB) usb_printf("\r\nFallback RTC Date %s",date);
							#endif
							#ifdef doUSB
							if (hasUSB) usb_printf("\r\nFallback RTC Time %s\r\n",time);
							#endif
					}
					else ret=1;
				}
			}
			else ret=1;
	 }
	 return ret;
}

bool sendHostUDP(void)
{
	bool rc=FALSE;
	const int UDP_PACKET_SIZE= 50; 
	char * pointer;
	uint32_t counterTime=0;
	inSend=0;
	sendUDP=0;

  #ifdef doUSB
  if (hasUSB) usb_printf("\r\nSend UDP on Client 4");	
  #endif
  // Initialize values needed to form UDP request
	memset(c, 0, UDP_PACKET_SIZE);
  c[0] = power;   
  c[1] = Cube_MODE;    
  c[2] = lastKey;
  c[3] = modeC;
	c[4] = welcome;
	c[5] = delaySpeed;
	c[6] = customBright;
	c[7] = allowModeWait;
	c[8] = modeNext;
	c[9] = doTransitions;
	pointer=&c[10];
	*pointer = fadeMax; // 4 bytes float
	counterTime=gCounterMax/timeMultipler;
	c[14] = (counterTime >> 24) & 0xFF;
	c[15] = (counterTime >> 16) & 0xFF;
	c[16] = (counterTime >> 8) & 0xFF;
	c[17] = counterTime & 0xFF;
	c[18] = shared2;
	c[19] = shared3;
	c[20] = shared4;
	c[21] = shared5;
	c[22] = shared6;		
	c[23] = sharedA;
	c[24] = sharedB;
	c[25] = sharedC;
	
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nUDP Sending...");
	#endif
	lastKey=0;
	rc=esp8266UDPSend(4,UDP_PACKET_SIZE);
	if (rc) esp8266TcpSendBufferDMA(c,UDP_PACKET_SIZE);
	else
	{
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nERROR sending UDP : UDP SEND ERROR");
		#endif
	}			
	inSend=97;
	return rc;
}

void hostUDP(void)
{
	bool rc=FALSE;
	rc=sendHostUDP();
	if (!rc)
	{
		// try connecting again 1st
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nConnect UDP on Client 4");	
		#endif
		rc=esp8266UDPConnect(4);
	}
	delay(20);
	rc=sendHostUDP(); // Send it Twice - UDP is unreliable !!
}

bool getNTPTime(void)
{
	bool rc =FALSE;
	const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message
	inSend=0;
	hasNTP=0;
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nGet NTP Time on Client 3");	
	#endif
	memset(c, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	c[0] = 227;   // Binary 0b11100011;   // LI, Version, Mode
	c[1] = 0;     // Stratum, or type of clock
	c[2] = 6;     // Polling Interval
	c[3] = 0xEC;  // Peer Clock Precision
	c[12]  = 49;
	c[13]  = 0x4E;
	c[14]  = 49;
	c[15]  = 52;	
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nNTP Sending...");
	#endif
	rc=esp8266UDPSend(3,NTP_PACKET_SIZE);
	if (rc) esp8266TcpSendBufferDMA(c,NTP_PACKET_SIZE);
	else
	{
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nERROR getting NTP Time : UDP SEND ERROR");
		#endif
	}			
	inSend=97;
	return rc;
}

void NTPTime(void)
{
	bool rc=FALSE;
	rc=getNTPTime();
	if (!rc)
	{
		// try connecting again 1st
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nConnect NTP on Client 3");	
		#endif
	  rc=esp8266NTPConnect(3);
   	rc=getNTPTime();
	}
}

bool esp8266StartServer()
{
	bool rc;
	memset(date, 32, 12);	
	memset(time, 32, 10);	
  esp8266SendCommand(ESP8266_SERVER_CONFIG, ESP8266_CMD_SETUP, "1,80");
	rc=esp8266ReadForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	if (rc)
	{
		rc=esp8266SetTimeout();
	}
	if (!rc)
	{
	 	setAll(RED);
		lcd_println(98,1,"Init WIFI - ERROR");
		lcd_println(99,1,"Server Start Error");
		#ifdef doUSB
		if (hasUSB) 
		{
			usb_printf("\r\nWIFI Details :\r\n");
			usb_printf((char *)esp8266RxBuffer);	// Print out Wifi Details if USB is attached
		}			
		#endif		
		delay(3000);
		setAll(BLACK);
	}
	if (rc)
	{
		esp8266ClearBuffer();
		if (cubeWIFI==10) USART1_WIFIReceiveDMAConfiguration();
		else
		{
			if (bjCubeType==1) USART2_WIFIReceiveDMAConfiguration();			
			else USART3_WIFIReceiveDMAConfiguration();
		}
		inIPD=0;
	  if (hasLCD) 
		{
			lcd_println(98,1,"Init WIFI - ...");
			//lcd_clearln(97);
		}		
		delay(2500);
		hasWIFI=4;
		if (WIFICopyMode==1)
		{
		  // Send first Multicast message
			sendUDP=1;
		}
		if (WIFICopyMode==2)
		{
		  // Start UDP Multicast Listen
			esp8266UDPConnect(4);
			delay(100);
		}
  	if (UTCOffsetH!=99) hasNTP=10; // GET NTP TIME
		if (hasLCD) lcd_println(98,1,"Init WIFI - OK");
	}
	return(rc);
}

void replaceVar(uint16_t start, uint16_t end)
{
	char var[20];
	uint8_t count=0;
	char replacement[20]="                    ";
	char *p;
	uint8_t found=0;
	uint8_t len=0;
	p=(char *)fatbuf+start+1;
	if (end-start<20)
	{
		len=end-start-1;
		strncpy(var,p,len);
		var[end-start-1]='\0';
		//if (hasUSB) usb_printf("\r\nReplace : %s",var);
		if (strncmp(var,"cubeMode",len)==0)
		{
			sprintf(replacement,"%d",Cube_MODE);
			found=1;
		}
		if (strncmp(var,"ledCubeType",len)==0)
		{
			if (ledCubeType==0) sprintf(replacement,"RGB Cube 8 ");
			else if (ledCubeType<2) sprintf(replacement,"AURA Cube 8 ");
      else sprintf(replacement,"RGB Cube 12 ");
			found=1;
		}
		if (strncmp(var,"delaySpeed",len)==0)
		{
			sprintf(replacement,"%d",delaySpeed);
			found=1;			
		}	
		if (strncmp(var,"\"delaySpeed\"",len)==0)
		{
			sprintf(replacement,"\"%d\"",delaySpeed);
			found=1;			
		}			
		if (strncmp(var,"ESP8266BAUD",len)==0)
		{
			sprintf(replacement,"%d",ESP8266BAUD);
			found=1;			
		}			
		if (strncmp(var,"currentGraphic",len)==0)
		{
			sprintf(replacement,"%d",modeC);
			if (modeNext)
			{
				sprintf(replacement,"%d",getNextGraphic());
			}			
			if (welcome) sprintf(replacement,"0        ");
			if (power==0) sprintf(replacement,"0        ");
			if (power==2) sprintf(replacement,"0        ");
			if (Cube_MODE!=2) sprintf(replacement,"-    ");
			if (Cube_MODE==6) sprintf(replacement,"Slave Cube");
			found=1;			
		}			
		if (strncmp(var,"theFullGraphicName",len)==0)
		{
			if (power==0) sprintf(replacement,"No Power ");
			if (power==2) sprintf(replacement,"Paused   ");
			if (power==1) sprintf(replacement,"%s",graphicName);
			found=1;			
		}					
		if (strncmp(var,"customBright",len)==0)
		{
			sprintf(replacement,"%d",customBright);
			found=1;	
		}		
		if (strncmp(var,"fadeMax",len)==0)
		{
			sprintf(replacement,"%.0f",fadeMax);
			found=1;	
		}			
		if (strncmp(var,"\"fadeMax\"",len)==0)
		{
			sprintf(replacement,"\"%.0f\"",fadeMax);
			found=1;	
		}	
		if (strncmp(var,"graphicTime",len)==0)
		{
			sprintf(replacement,"%d",gCounterMax/timeMultipler);
			found=1;	
		}			
		if (strncmp(var,"\"graphicTime\"",len)==0)
		{
			sprintf(replacement,"\"%d\"",gCounterMax/timeMultipler);
			found=1;	
		}				
		if (strncmp(var,"\"doTransitions\"",len)==0)
		{
			if (doTransitions==0) sprintf(replacement,"No  ");
			else sprintf(replacement,"Yes ");
			found=1;	
		}				
		if (strncmp(var,"timeLeft",len)==0)
		{
			if (counterNum<=gCounterMax) sprintf(replacement,"%d sec",(gCounterMax-counterNum)/timeMultipler);
			else sprintf(replacement,"Fading Out    ");
			if (modeWait && allowModeWait) sprintf(replacement,"Until end    ");
			if (modeNext==4) sprintf(replacement,"Remain     ");
			if (Cube_MODE==6) sprintf(replacement,"Slave Cube ");
			found=1;				
		}
		if (strncmp(var,"modeWait",len)==0)
		{
			if (modeWait && allowModeWait) sprintf(replacement,"Yes ");
			else sprintf(replacement,"No ");
			found=1;				
		}
		if (strncmp(var,"\"wait\"",len)==0)
		{
			if (allowModeWait==0) sprintf(replacement,"\"No\"");
			else sprintf(replacement,"\"Yes\"");
			found=1;				
		}
		if (strncmp(var,"\"currentGraphic\"",len)==0)
		{
			sprintf(replacement,"\"%d\"",modeC);
			found=1;			
		}			
		if (strncmp(var,"eFadeout",len)==0)
		{
			sprintf(replacement,"%d %d %d",eFadeout,eFadeoutDone,eNum);
			found=1;				
		}
		if (strncmp(var,"\"remain\"",len)==0)
		{
			if (modeNext==4) sprintf(replacement,"\"Yes\"");
			else sprintf(replacement,"\"No\"");
			found=1;
		}
		if (strncmp(var,"temperature",len)==0)
		{
			sprintf(replacement,"%.2f",temperature);
			found=1;				
		}
	}
	if (found)
	{
		for(count=0;count<len+2;count++)
		{
			*(p+count-1)=replacement[count];
		}
	}
}

void replaceHTMLVariables(uint16_t size)
{
	uint16_t count=0;
  uint16_t start=0;
	uint16_t end=0;
	for (count=0;count<size;count++)
	{
		 if(fatbuf[count]=='*')
		 {
			 if(start==0) start=count;
			 else
			 {
				 end=count;
				 replaceVar(start,end);
				 start=0;
			 }
		 }
	}
	//if (start!=0) setAll(WHITE);
	if (hasUSB) 
	{
		if (start!=0) 
		{
				#ifdef doUSB
				usb_printf("\r\nAsterix Over %d Boundary ***",size);
				#endif
				#ifdef doUSB
				if (start<size-4) usb_printf("%c%c%c",fatbuf[start],fatbuf[start+1],fatbuf[start+2]);
				#endif
		}
	}
}

void setupSharedVariables(void)
{
		// setup Random Variables
	  shared2=cube_random(2);
		shared3=cube_random(3);
		shared4=cube_random(4);
		shared5=cube_random(5);
		shared6=cube_random(6);
		sharedA=cube_random(255);
		sharedB=cube_random(255);
		sharedC=cube_random(255);		
}

uint8_t simulateKey(char* key, char* value)
{
	char k;
	uint8_t msg=0;
	// Do a IR key press
	k=0;
	if (!strncmp(key,"0",1)) k=0x16;
	if (!strncmp(key,"1",1)) k=0x0c;
	if (!strncmp(key,"2",1)) k=0x18;
	if (!strncmp(key,"3",1)) k=0x5e;
	if (!strncmp(key,"4",1)) k=0x08;
	if (!strncmp(key,"5",1)) k=0x1c;
	if (!strncmp(key,"6",1)) k=0x5a;
	if (!strncmp(key,"7",1)) k=0x42;
	if (!strncmp(key,"8",1)) k=0x52;
	if (!strncmp(key,"9",1)) k=0x4a;
	if (!strncmp(key,"FF",2)) k=0x09;
	if (!strncmp(key,"REW",3)) k=0x07;
	if (!strncmp(key,"PLAY",4)) k=0x15;
	if (!strncmp(key,"-",1)) k=0x19;
	if (!strncmp(key,"+",1)) k=0x40;
	if (!strncmp(key,"PLUS",4)) k=0x40; // + from browser	
	if (!strncmp(key,"MENU",4)) k=0x47;
	if (!strncmp(key,"PWR",3)) k=0x45;
	if (!strncmp(key,"TEST",4)) k=0x44;
	if (!strncmp(key,"C",1)) k=0x0d;
	if (!strncmp(key,"RTN",3)) k=0x43;
	if (!strncmp(key,"RESET",5)) resetCube();
	if (!strncmp(key,"dim",3)) {if (atoi(value)==-1) fadeMax=0;else fadeMax=atoi(value);if(atoi(value)==0) fadeMax=100;} // sometimes zero comes across so need to fudge this
	if (!strncmp(key,"graphic",7) && atoi(value)!=79) {Cube_MODE=2;modeNextG=atoi(value);specificGraphic();} // specific graphic unless selected number 79
	if (!strncmp(key,"ESPSPEED",8)) ESP8266ChangeSpeed();
	if (!strncmp(key,"speed",5)) delaySpeed=atoi(value);
	if (!strncmp(key,"time",4) && gCounterMax!=atoi(value)*timeMultipler) {gCounterMax=atoi(value)*timeMultipler;gCleanup();} // convert from seconds to interupt ticks
	if (!strncmp(key,"trans",5)) 
	{
		if (strncmp(value,"No",2) && doTransitions==1) {doTransitions=0;gCleanup();}
 	  if (!strncmp(value,"No",2) && doTransitions==0) {doTransitions=1;gCleanup();}
	}		
	if (!strncmp(key,"modeWait",8)) 
	{
		if (strncmp(value,"No",2) && allowModeWait==1) allowModeWait=0;
		if (!strncmp(value,"No",2) && allowModeWait==0) allowModeWait=1;
	}		
	if (!strncmp(key,"remain",6)) 
	{ 
		if (!strncmp(value,"No",2) && modeNext!=4) {msgWait=0;modeNext=4;gCleanup();}				
		else
		{
			msgWait=1;
			if (modeNext==4) {modeNext=0;gCleanup();}
		}
	} // stay on current graphic
  if (!strncmp(key,"r01",3)) 
	{ 
		R=atoi(value);
		light_red=R;
	}
  if (!strncmp(key,"g01",3)) 
	{ 
		G=atoi(value);
		light_green=G;
	}
  if (!strncmp(key,"b01",3)) 
	{ 
		B=atoi(value);
		light_blue=B;
	}
	
	if (!strncmp(key,"msg",3)) 
	{ 
		 msg=1;
		 Cube_MODE=2;
	   modeNextG=98; // Send a message
		 specificGraphic(); 
		 //modeText=10; // Specific Message
	}		
	if (!strncmp(key,"ttype",5)) 
	{ 
		modeText=atoi(value);
	}
	if (!strncmp(key,"ctype",5)) 
	{ 
		textColourMode=atoi(value);
	}
	if (!strncmp(key,"rtype",5)) 
	{ 
		textColourRandom=atoi(value);
	}
	if (!strncmp(key,"tmode",5)) 
	{ 
		textMode=atoi(value);
	}	
	if (k!=0) KeyHandle(k);	
	if (WIFICopyMode==1 && k!=0x09 && k!=0x07 && strncmp(key,"graphic",7)) // Already done for graphics change 
	{
		lastKey=k;
		setupSharedVariables();
		sendUDP=1;
	}
	if (msg==1) return 2;
	return 0;
}

void parseParams(char *start, char *end)
{
	char key[15];
	char value[15];
	uint8_t counter=0;
	uint8_t inKey=1;
	uint8_t inCounter=0;
	uint8_t msg=0;
	uint8_t msgStart=0;

	for (counter=start-c;counter<end-c;counter++)
	{
		if (c[counter]=='='&&inKey)
		{
			key[inCounter]='\0';
			inKey=0;
			inCounter=0;
		}
		else if (c[counter]=='&'&&!inKey)
		{
			value[inCounter]='\0';
			inKey=1;
			inCounter=0;
			// Process key
			//if (hasUSB)
			//{
				//usb_printf("\r\nKEY : %s",key);
				//usb_printf("\r\nVALUE : %s",value);
			//}
			if (msgStart!=0)
			{
				msg=simulateKey(key,value);
				if (msg==2)
				{
					strcpy(&tcpRxBuffer[TCP_RX_BUFFER_LEN-100],&c[msgStart]);
					//usb_printf("\r\nBUFFERED %s",&tcpRxBuffer[TCP_RX_BUFFER_LEN-100]);
				}
				msgStart=0;
			}
		}
		else
		{
			if(inKey)
			{
				key[inCounter]=c[counter];
				if (inCounter<14) inCounter++;
			}
			else
			{
	      if(msgStart==0) msgStart=counter;
				value[inCounter]=c[counter];
				if (inCounter<14) inCounter++;	
			}
		}
	}
	if(!inKey)
	{
			// Process key
			value[inCounter]='\0';
		  //if (hasUSB)
			//{
				//usb_printf("\r\nnot KEY : %s",key);
				//usb_printf("\r\nnot VALUE : %s",value);
			//}
			if (msgStart!=0)
			{
				msg=simulateKey(key,value);
				if (msg==2)
				{
					strcpy(&tcpRxBuffer[TCP_RX_BUFFER_LEN-100],&c[msgStart]);
					//usb_printf("\r\nBUFFERED %s",&tcpRxBuffer[TCP_RX_BUFFER_LEN-100]);
				}		
				msgStart=0;
			}
	}
	shared2=255; // Got a WIFI input so assume my own control again until I get another signal
}

void getParams(char *end)
{
	//char key[12];
	//char value[5]="    \0";
	char *spot;
  char *spot2;
	char *q;
	q=c+4;
	//key[0]=0;
	page[0]=0;
	spot=strstr(q,"?");
	if (spot==0) strcpy(page,q);
	else
	{
		if(spot-q<80) 
		{
			strncpy(page,q,(spot-q));
		  page[spot-q]=0;
			spot2=strstr(spot+1," HTTP");
			if(spot2)
			{
				end=spot2;
			}
			parseParams(spot+1,end);
		}
	}
	// strip HTTP/1.1
	spot2=strstr(page," HTTP");
	if(spot2)
	{
		page[spot2-page]=0;
	}
	if (strcmp(page,"/")==0)
	{
		strcpy(page,"/index.html");
	}
	#ifdef doUSB	
	if (hasUSB==2)
	{
		usb_printf("\r\nPAGE = ");
		usb_printf(page);
	}
	#endif	
	if (!strncmp(page,"/nightlight",11))
	{
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nStart Night Light");
		#endif
		if (Cube_MODE!=3) simulateKey("3","3");
	}
	//if (key) simulateKey(key,value);
}

void postParams(void)
{
	//char key[12];
	//char value[5]="    \0";
	char *spot;
  char *spot2;
	uint8_t length=0;
	char *q;
	q=c+5;
	//key[0]=0;
	page[0]=0;
	spot=strstr(q,"?");
	if (spot==0) strcpy(page,q);
	else
	{
		if(spot-q<80) 
		{
			strncpy(page,q,(spot-q));
		  page[spot-q]=0;
		}
	}
	// strip HTTP/1.1
	spot2=strstr(page," HTTP");
	if(spot2)
	{
		page[spot2-page]=0;
	}
	if (strcmp(page,"/")==0)
	{
		strcpy(page,"/index.html");
	}
	#ifdef doUSB
	if (hasUSB)
	{
		usb_printf("\r\nPOST PAGE = ");
		usb_printf(page);
	}
	#endif	
	if (!strncmp(page,"/nightLight",11))
	{
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nStart Night Light");
		#endif
		if (Cube_MODE!=3) simulateKey("3","3");
	}
	spot=strstr(tcpRxBuffer,"\r\n\r\n"); // Look for end of Section
	if (spot!=0)
	{
		spot+=4; // skip past \r\n\r\n
		spot2=tcpRxBuffer+tcpBufferHead;
		if (spot2-spot>98) 
		{
			spot2=spot+98; // Max length 100 chars
			*spot2='\0';
		}
		length=spot2-spot;
		if (length>0)
		{
			//usb_printf("\r\nLength=%d,%d",length,tcpBufferHead);
			strcpy(c,spot);
			#ifdef doUSB
			if (hasUSB) usb_printf("\r\nMessage=%s",c);
			#endif
			parseParams(c,c+length);	
		}
		else expectPOST=1; // The POST Data is comming separately
	}
}

uint8_t checkCommand(char *end)
{
		if (!strncmp(c,"GET ",4))
		{
			getParams(end);
		 	return(1);
		}
		if (!strncmp(c,"POST ",5))
		{
			postParams();
		 	return(1);
		}		
		if (!strncmp(c,"CMD ",4))
		{
		 	return(2);
		}
		if (!strncmp(c,"KEY ",4))
		{
		 	return(3);
		}
		return(0);
}

void sendAJAXData(void)
{
	 // Send a result to a web page for auto updates
	 char* spot;
   char data2[250];
	 char data[150];
	
	 #ifdef doUSB
	 if (hasUSB==2) usb_printf("\r\nSending AJAX Data");
	 #endif
	 strcpy(data2,"HTTP/1.1 200 OK\r\n\r\n");
	 spot=data;
	 if (counterNum<=gCounterMax) sprintf(spot,"%d sec,",(gCounterMax-counterNum)/timeMultipler);
	 else sprintf(spot,"Fading Out,");
	 if (modeWait && allowModeWait) sprintf(spot,"Until end,");
	 if (modeNext==4) sprintf(spot,"Remain,");
	 if (Cube_MODE==6) sprintf(spot,"Slave Cube,");
 	 spot=strstr(spot,",")+1;
   if (modeWait && allowModeWait) sprintf(spot,"Yes,");
	 else sprintf(spot,"No,");
	 spot=strstr(spot,",")+1;
	 sprintf(spot,"%.0f,",fadeMax);
	 spot=strstr(spot,",")+1;
   sprintf(spot,"%d,",delaySpeed);
	 spot=strstr(spot,",")+1;
	 sprintf(spot,"%d,",gCounterMax/timeMultipler);
	 spot=strstr(spot,",")+1;
	 sprintf(spot,"%d,",modeC);
	 if (modeNext)
	 {
			sprintf(spot,"%d,",getNextGraphic());
	 }			
	 if (welcome) sprintf(spot,"0,");
	 if (power==0) sprintf(spot,"0,");
	 if (power==2) sprintf(spot,"0,");
	 if (Cube_MODE!=2) sprintf(spot,"-,");
	 if (Cube_MODE==6) sprintf(spot,"Slave Cube,");
	 spot=strstr(spot,",")+1;
	 if (power==0) sprintf(spot,"No Power,");
	 if (power==2) sprintf(spot,"Paused,");
	 if (power==1) sprintf(spot,"%s,",graphicName);
	 spot=strstr(spot,",")+1;
 	 if (modeNext==4) sprintf(spot,"Yes,");
	 else sprintf(spot,"No,");
	 spot=strstr(spot,",")+1;
 	 if (doTransitions==0) sprintf(spot,"No,");
	 else sprintf(spot,"Yes,");
	 spot=strstr(spot,",")+1;
	 if (allowModeWait==0) sprintf(spot,"No");
	 else sprintf(spot,"Yes");	 
	 
	 strcpy(data2+strlen(data2),data);
	 esp8266TcpSend(tcpBufferClient,strlen(data2));
	 esp8266TcpSendBufferDMA(data2,strlen(data2));
	 inSend=88; // Send Close next time around
}

void esp8266CheckServerRead()
{
	char option[81];
	char value[81];
	char key[4];
	uint8_t cmd=0;
	uint16_t i = 0;
	uint16_t j = 0;
	char *spot;
	char *end;
  if(esp8266ReadTcpData())
  {
		spot=strstr(tcpRxBuffer,"\r\n");
		if(spot==0) spot=tcpRxBuffer+tcpBufferHead; // no \r\n
		if(spot-tcpRxBuffer<100)
		{
			strncpy(c,tcpRxBuffer,spot-tcpRxBuffer);
			c[spot-tcpRxBuffer]=0;
			end=c+(spot-tcpRxBuffer); //-tcpRxBuffer;
			if (expectPOST)
			{
				#ifdef doUSB
				if (hasUSB) usb_printf("\r\nSeparate Post Message");
				#endif
				if (end-tcpRxBuffer>98)
				{
					tcpRxBuffer[98]='\0';
				}
				strcpy(c,tcpRxBuffer);
				#ifdef doUSB
				if (hasUSB) usb_printf("\r\nMessage=%s",c);
				#endif
				parseParams(c,end);	
				expectPOST=0;
			}
			cmd=checkCommand(end);
			if (cmd==1)
			{
				if (!strncmp(page,"/readData",9)) // Read variable
				{
					sendAJAXData();
				}
				else if (!strncmp(page,"/sendMsg",8)) // Read variable
				{
					#ifdef doUSB
					if (hasUSB) usb_printf("\r\nAJAX Post Msg");
					#endif
					postParams(); // Same as post
					sendOKHTTPHeader();
					inSend=88;
				}
				else if (!strncmp(page,"/nightLight",11)) // Read variable
				{
					#ifdef doUSB
					if (hasUSB) usb_printf("\r\nAJAX Post NightLight");
					#endif
					postParams(); // Same as post
					sendOKHTTPHeader();
					inSend=88;
				}				
				else inSend=1; // get ready to send next IRQ
			}	
			if (cmd==2)
			{
				//if (hasUSB) usb_printf(tcpRxBuffer);
				//Find first space 
				// Copy substring characters until you reach ' '
				for (i=0; tcpRxBuffer[i+4] != ' ' && tcpRxBuffer[i+4] != 0 && i<80 ; i++) 
				{
					option[i] = tcpRxBuffer[i+4];
				}
				option[i] = 0;                    // Add string terminator at the end of substring
				for (j=0; tcpRxBuffer[i+j+5] != ' ' && tcpRxBuffer[i+j+5] != 0 && j<80 ; j++) 
				{
					value[j] = tcpRxBuffer[i+j+5];
				}
				value[j] = 0;                    // Add string terminator at the end of substring		
				executeCommand(option,value);
			}
			if (cmd==3)
			{
				key[0]=tcpRxBuffer[4];
				key[1]=tcpRxBuffer[5];
				key[2]=tcpRxBuffer[6];
				key[3]=tcpRxBuffer[7];			
				simulateKey(key,"     \0");
			}
		tcpBufferHead = 0;
		tcpRxBuffer[0]=0;
		}
	}
}

void esp8266CheckServer()
{
	//uint8_t client=0;
	if (inSend==0) esp8266CheckServerRead();
	if (millis() > WIFI_old_millis+20) //20 millis between sends
	{
		//usb_printf("%d",inSend);
		if(inSend>0 && inSend<88 && !SDinUse) 
		{
			SDinUse=1; // Take the token
			if (esp8266BufferAvailable(tcpBufferClient,2048))
			{
				#ifdef doUSB
				if (hasUSB==2) usb_printf("\r\nServe HTML %d to %d",inSend,tcpBufferClient);
				#endif
				mf_read_html(page);
			}
			if (SDinUse!=2)	SDinUse=0; // Release the token
		}
		if (sendUDP==3 && inSend==0 && hasNTP<11) // UDP Send
		{
			hostUDP();
		}
		if (sendUDP==2 && inSend==0 && hasNTP<11) // UDP Send
    {
			sendUDP=3;
		}
 		if (sendUDP==1 && inSend==0 && hasNTP<11) // UDP Send
    {
			sendUDP=2;
		}
		if (hasNTP==12 && inSend==0 && sendUDP==0) // NTP Request Start
		{
			NTPTime();
		}
		if (hasNTP==11 && inSend==0 && sendUDP==0) // NTP Request Start
		{
      hasNTP=12;
		}
		if (hasNTP==10 && inSend==0 && sendUDP==0) // NTP Request Start
		{
			hasNTP=11;
		}
		if(inSend==91) 
		{	 
			inSend=0;
		}	
		if(inSend==90) 
		{	 
			inSend=91;
		}	
		if (inSend==89) 
		{
			inSend=90; // space out close
			esp8266TcpClose(tcpBufferClient);
		}
		if (inSend==99)
		{
			// no close - just a delay
			inSend=0;
		}
		if (inSend==88) inSend=89;
		if (inSend==98) inSend=99;
		if (inSend==97) inSend=98;
		WIFI_old_millis=millis();
	}
}

bool esp8266Connect(const char * ssid, const char * pwd)
{
	// The ESP8266 can be set to one of three modes:
	//  1 - ESP8266_MODE_STA - Station only
	//  2 - ESP8266_MODE_AP - Access point only
	//  3 - ESP8266_MODE_STAAP - Station/AP combo
	bool rc = FALSE;

	if(esp8266SetMode(ESP8266_MODE_STAAP))
	{
		//usartSendArrar(USART2, "Mode set to station\n");
		usartSendArrar(ESP8266_USART, "AT");
		usartSendArrar(ESP8266_USART, (uint8_t *)ESP8266_CONNECT_AP);
		usartSendArrar(ESP8266_USART, "=\"");
		usartSendArrar(ESP8266_USART, (uint8_t *)ssid);
		usartSendArrar(ESP8266_USART, "\"");
		if (pwd != NULL)
		{
			usartSendArrar(ESP8266_USART, ",");
			usartSendArrar(ESP8266_USART, "\"");
			usartSendArrar(ESP8266_USART, (uint8_t *)pwd);
			usartSendArrar(ESP8266_USART, "\"");
		}
		usartSendArrar(ESP8266_USART, "\r\n");
		rc = esp8266ReadForResponses(RESPONSE_OK, RESPONSE_FAIL, WIFI_CONNECT_TIMEOUT,bufStart,1);

		return rc;
	}
	return FALSE;
}

/////////////////////
// TCP/IP Commands //
/////////////////////

bool esp8266UDPConnect(uint8_t client)
{
	uint16_t bS=0;
	bool rsp = FALSE;
	bS=bufEnd;
	//usartSendArrar(ESP8266_USART,"AT+CIPSTART=3,\"UDP\",\"time.nist.gov\",123\r\n");

	strcpy(c,"AT+CIPSTART=");
	sprintf(c+12,"%d",client);
	strcpy(c+13,",\"UDP\",\"");
	if (WIFICopyMode==1) strcpy(c+21,"255.255.255.255\",4445,4445,2\r\n\0");
	else strcpy(c+21,"0.0.0.0\",4445,4445,2\r\n\0");

	#ifdef doUSB
	if (hasUSB) usb_printf("\r\n%s",c);
	#endif
	usartSendArrar(ESP8266_USART,(uint8_t *)c);
  #ifdef doUSB
  if (hasUSB) usb_printf("\r\nUDP Connect:%s",c);	
  #endif
	rsp = esp8266ReadForResponses(RESPONSE_OK, RESPONSE_ERROR, CLIENT_CONNECT_TIMEOUT,bS,0);
	if(rsp == FALSE)
	{
		// We may see "ERROR", but be "ALREADY CONNECTED".
		// Search for "ALREADY", and return success if we see it.
		rsp = esp8266SearchBuffer(bufStart,"ALREADY",0);
		if (rsp)
			return TRUE;
		// Otherwise the connection failed. Return the error code:
		if (hasUSB) printWifiBuffer(bS);
		return FALSE;
	}
	
  return(TRUE);
}

bool esp8266NTPConnect(uint8_t client)
{
	uint16_t bS=0;
	bool rsp = FALSE;
	bS=bufEnd;
	//usartSendArrar(ESP8266_USART,"AT+CIPSTART=3,\"UDP\",\"time.nist.gov\",123\r\n");

	strcpy(c,"AT+CIPSTART=");
	sprintf(c+12,"%d",client);
	strcpy(c+13,",\"UDP\",\"");
	strcpy(c+21,NTPServer);
	strcpy(c+21+strlen(NTPServer),"\",123\r\n\0");
	usartSendArrar(ESP8266_USART,(uint8_t *)c);
	
	rsp = esp8266ReadForResponses(RESPONSE_OK, RESPONSE_ERROR, CLIENT_CONNECT_TIMEOUT,bS,0);
	if(rsp == FALSE)
	{
		// We may see "ERROR", but be "ALREADY CONNECTED".
		// Search for "ALREADY", and return success if we see it.
		rsp = esp8266SearchBuffer(bufStart,"ALREADY",0);
		if (rsp)
			return TRUE;
		// Otherwise the connection failed. Return the error code:
		if (hasUSB) printWifiBuffer(bS);
		return FALSE;
	}
	
  return(TRUE);
}

bool esp8266TcpConnect(uint8_t * destination, uint8_t * port)
{
	uint16_t bS=0;
	bool rsp = FALSE;
	bS=bufEnd;
	usartSendArrar(ESP8266_USART, "AT");
	usartSendArrar(ESP8266_USART, (uint8_t *)ESP8266_TCP_CONNECT);
	usartSendArrar(ESP8266_USART, "=4,"); // Use number 4 connection
	usartSendArrar(ESP8266_USART, "\"TCP\",");
	usartSendArrar(ESP8266_USART, "\"");
	usartSendArrar(ESP8266_USART, destination);
	usartSendArrar(ESP8266_USART, "\",");
	usartSendArrar(ESP8266_USART, port);
	usartSendArrar(ESP8266_USART, "\r\n");
	// Example good: CONNECT\r\n\r\nOK\r\n
	// Example bad: DNS Fail\r\n\r\nERROR\r\n
	// Example meh: ALREADY CONNECTED\r\n\r\nERROR\r\n
	rsp = esp8266ReadForResponses(RESPONSE_OK, RESPONSE_ERROR, CLIENT_CONNECT_TIMEOUT,bS,0);
	
	if(rsp == FALSE)
	{
		// We may see "ERROR", but be "ALREADY CONNECTED".
		// Search for "ALREADY", and return success if we see it.
		rsp = esp8266SearchBuffer(bufStart,"ALREADY",0);
		if (rsp)
			return TRUE;
		// Otherwise the connection failed. Return the error code:
		if (hasUSB) printWifiBuffer(bS);
		return FALSE;
	}
	// Return 1 on successful (new) connection
	return TRUE;
}

bool esp8266UDPSend(uint8_t con, uint16_t size)
{
	char params[20];
	uint16_t bS=0;
	//uint32_t delayCount=0;
	bool rsp = FALSE;
	bS=bufEnd;
	if (size > 2048)
		return FALSE; //ESP8266_CMD_BAD
	sprintf(params,"%d%s%d",con,",",size);
	esp8266SendCommand(ESP8266_TCP_SEND, ESP8266_CMD_SETUP, params);
	rsp = esp8266ReadForResponses(RESPONSE_OK, RESPONSE_ERROR, COMMAND_RESPONSE_TIMEOUT,bS,0);
	if (!rsp && hasUSB) printWifiBuffer(bS);
	return(rsp);
}

bool esp8266TcpSend(uint8_t con, uint16_t size)
{
	char params[20];
	uint16_t bS=0;
	bool rsp = FALSE;
	bS=bufEnd;
	if (size > 2048)
		return FALSE; //ESP8266_CMD_BAD
	sprintf(params,"%d%s%d",con,",",size);
	if (cubeWIFI==1||cubeWIFI==10) esp8266SendCommand(ESP8266_TCP_SENDBUF, ESP8266_CMD_SETUP, params); // ESP8266
	else esp8266SendCommand(ESP8266_TCP_SENDEX, ESP8266_CMD_SETUP, params); // No SENDBUF for ESP32
	rsp = esp8266ReadForResponses(RESPONSE_OK, RESPONSE_ERROR, COMMAND_RESPONSE_TIMEOUT,bS,0);
	if (!rsp && hasUSB) printWifiBuffer(bS);
	return(rsp);
}

void esp8266TcpSendBuffer(char *buf, uint16_t size)
{
	uint16_t i = 0;
	char *p = buf;
	uint16_t timeout=0;	
	for(i=0; i<size; i++)
	{
		  timeout=0;
			USART_GetFlagStatus(ESP8266_USART, USART_FLAG_TC);
			USART_SendData(ESP8266_USART,*p);
			while(USART_GetFlagStatus(ESP8266_USART, USART_FLAG_TC) == RESET && timeout<20000)
			{
				timeout++;
			}
			#ifdef doUSB
			if (timeout==20000 && hasUSB) usb_printf("\r\nTIMEOUT ERROR: sending to WIFI USART for esp8266TcpSendBuffer");
			#endif
			p++;
	}	
}

void sendOKHTTPHeader(void)
{
	 char tempC[50];
	 // Send Header for HTTP REQUEST - ALL GOOD
	 strcpy(tempC,"HTTP/1.1 200 OK\r\n\r\n"); //Connection: Close\r\n\r\n");  // HTTP Header
	 esp8266TcpSend(tcpBufferClient,strlen(tempC));
	 esp8266TcpSendBuffer(tempC,strlen(tempC));
}

void esp8266TcpSendBufferDMA(char *buf, uint16_t size)
{
	if (size>0)
	{
		if (cubeWIFI==10) USART1_WIFIDMARun(buf,size);
		else
		{
			if (bjCubeType==1) USART2_WIFIDMARun(buf,size);
			else USART3_WIFIDMARun(buf,size);
		}
	}
}

bool esp8266BufferAvailable(uint8_t con, uint16_t size)
{
	bool rc = FALSE;
	uint8_t countCommas=0;
	uint16_t bS=0;
	uint32_t timeout=0;
	char c;
	char free[10];
	uint8_t freePtr=0;
	uint16_t f=0;
	sprintf(&c, "%d", con);
	bS=bufEnd;
	esp8266SendCommand(ESP8266_TCP_CHECKBUF, ESP8266_CMD_SETUP, &c);
  while(esp8266CharsInBuffer(bS)<9 && timeout<800000) 
	{
		timeout++;
	} // wait for chars
	// check for the third comma
	while (countCommas<4 && timeout<800000)
	{
		timeout++;
		c=esp8266RxBuffer[bS];
		if (countCommas==3 && c!=',')
		{
			// Data we want
			free[freePtr]=c;
			freePtr++;
			if (freePtr>5)
			{
				// get out of here!
				//freePtr=0;
				countCommas++;
			}
		}
	 	if (c==',')
		{
			 countCommas++;
		}
		bS++;
	  if(bS >= ESP8266_RX_BUFFER_LEN)
		{
				bS = 0;
		}
	}
	free[freePtr]='\0';
	f=atoi(free);
	//if (hasUSB) usb_printf("\r\nCHECK SIZE for %d. Available %d, freePtr %d",size,f,freePtr);
	if (freePtr==0 || f>size) rc=TRUE; //freePtr==0 || f>size
	return rc;
}

bool esp8266CheckSend(void)
{
	bool rsp = FALSE;
	rsp = esp8266ReadForResponse("SEND OK\r\n", TCP_TIMEOUT);	 // SEND OK
	return (rsp);
}

bool esp8266TcpClose(uint8_t con)
{
	bool rc = FALSE;
	char c;
	sprintf(&c, "%d", con);
	esp8266SendCommand(ESP8266_TCP_CLOSE, ESP8266_CMD_SETUP, &c);
	//rc = esp8266ReadForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	/*if (!rc)
	{
		if (hasUSB) usb_printf("TCP Close Timeout");
	}*/
	return rc;
}

bool esp8266SetMux(uint8_t mux)
{
	bool rc = FALSE;
	char params[2] = {0, 0};
	params[0] = (mux > 0) ? '1' : '0';
	esp8266SendCommand(ESP8266_TCP_MULTIPLE, ESP8266_CMD_SETUP, params);
	rc = esp8266ReadForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	return rc;
}

//////////////////////////////////////////////////
// Private, Low-Level, Ugly, Hardware Functions //
//////////////////////////////////////////////////

void esp8266SendCommand(const char * cmd, esp8266_command_type type, const char * params)
{
	usartSendArrar(ESP8266_USART, "AT");
 	usartSendArrar(ESP8266_USART, (uint8_t *)cmd);
	if (type == ESP8266_CMD_QUERY)
		usartSendArrar(ESP8266_USART, "?");
	else if (type == ESP8266_CMD_SETUP)
	{
		usartSendArrar(ESP8266_USART, "=");
		usartSendArrar(ESP8266_USART, (uint8_t *)params);
	}
	usartSendArrar(ESP8266_USART, "\r\n");
}

bool esp8266Read(unsigned int timeout)
{
	unsigned long timeIn = millis();	// Timestamp coming into function
	while (timeIn + timeout * wifiTimeoutMult > millis()) // While we haven't timed out
	{
		if (esp8266RxBufferAvailable(bufStart)) // If data is available on ESP8266_USART RX
		{
				return TRUE;
		}
	}
	return FALSE; // Return the timeout error code
}

bool esp8266ReadForResponse(const char * rsp, unsigned int timeout)
{
	#ifdef doUSB	
	char str[20];
	#endif
	unsigned long timeIn = millis();	// Timestamp coming into function
	while (timeIn + timeout * wifiTimeoutMult > millis()) // While we haven't timed out
	{
		if (esp8266RxBufferAvailable(bufStart)) // If data is available on ESP8266_USART RX
		{
			if (esp8266SearchBuffer(bufStart,rsp,1))	// Search the buffer for goodRsp
				return TRUE;
		}
	}
	#ifdef doUSB	
	if (hasUSB)
	{
		usb_printf("\r\nR TIME OUT:");
		sprintf(str,"%s",rsp);
		usb_printf(str);
		usb_printf("\r\n");		
		usb_printf("\r\nBUF ST:");
		usb_printf("%d",bufStart);
		usb_printf("\r\n");
		usb_printf("\r\nBUF END:");
		usb_printf("%d",bufEnd);
		usb_printf("\r\n");
		printWifiBuffer(0);
		//usb_printf(esp8266RxBuffer);
	}
  #endif
  return FALSE; // Return the timeout error code
}

bool esp8266ReadForResponses(const char * pass, const char * fail, unsigned int timeout, uint16_t bS, uint8_t changeStart)
{
	#ifdef doUSB	
	char str[20];
	#endif
	bool rc = FALSE;
	unsigned long timeIn = millis();	// Timestamp coming into function
	while (timeIn + timeout * wifiTimeoutMult > millis()) // While we haven't timed out
	{
		if (esp8266RxBufferAvailable(bS)) // If data is available on UART RX
		{
			rc = esp8266SearchBuffer(bS,pass,changeStart);
			if (rc)	// Search the buffer for goodRsp
				return TRUE;	// Return how number of chars read
			rc = esp8266SearchBuffer(bS,fail,changeStart);
			if (rc)
			{
			  #ifdef doUSB
        if (hasUSB)
	      {
		      usb_printf("\r\nRS FAIL:");
		      sprintf(str,"%s",pass);
		      usb_printf(str);
		      usb_printf("\r\n");
					printWifiBuffer(0);
		      //usb_printf(esp8266RxBuffer);
	      }
	      #endif
				return FALSE;
			}
		}
	}
	#ifdef doUSB
	if (hasUSB)
	{
		usb_printf("\r\nRS TIME OUT:");
		sprintf(str,"%s",pass);
		usb_printf(str);
		usb_printf("\r\n");
    printWifiBuffer(0);		
		//usb_printf(esp8266RxBuffer);
	}
	#endif	
	return FALSE;
}

bool esp8266SetIPAddress()
{
	uint8_t spot=1;
	bool rc = FALSE;
	char ip[100];
	ip[0]='"';
	strcpy(ip+spot,myIPAddress);
	spot+=strlen(myIPAddress);
	ip[spot++]='"';
	ip[spot++]=',';
	ip[spot++]='"';
	strcpy(ip+spot,myGW);
	spot+=strlen(myGW);
	ip[spot++]='"';
	ip[spot++]=',';
	ip[spot++]='"';
	strcpy(ip+spot,myMask);
  spot+=strlen(myMask);
	ip[spot++]='"';	
	ip[spot]='\0';
	esp8266SendCommand(ESP8266_SET_STA_IP,ESP8266_CMD_SETUP,ip);
  rc = esp8266ReadForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	return(rc);
}

bool esp8266GetIPAddress()
{
	bool rc = FALSE;
	uint8_t len=0;
	esp8266SendCommand(ESP8266_GET_LOCAL_IP,ESP8266_CMD_EXECUTE,0);
	len=esp8266SearchData("STAIP,","OK",1);
	if (len)
	{
		strcpy(myIPAddress,tcpRxBuffer);
		tcpRxBuffer[0]=0;
		rc = TRUE;
	}
	return (rc);
}

//////////////////
// Buffer Stuff //
//////////////////

uint16_t esp8266CharsInBuffer(uint16_t bS)
{
	if (bS<=bufEnd) return (bufEnd-bS);
	else return(ESP8266_RX_BUFFER_LEN-bS+bufEnd);
}

void esp8266ClearBuffer()
{
	memset(esp8266RxBuffer, '\0', ESP8266_RX_BUFFER_LEN);
	bufStart = 0;
	bufEnd = 0;
	inIPD=0;
}

uint16_t parseIPD(char *IPD)
{
	char chars[5]="    ";
	uint8_t byteOfLen = 0;	
	uint16_t len = 0;
	tcpBufferClient=IPD[0]-'0';
	while(IPD[byteOfLen+2] != ':' && byteOfLen<6)
	{
		chars[byteOfLen]=IPD[byteOfLen+2];
		byteOfLen++;
	}
	 len=atoi(chars);
	 return(len);
}

// TCP Read
bool esp8266ReadTcpData(void)
{
	// read until next + for +IPD
	uint16_t i = 0;	
	uint16_t bS=0, bE=0, oF=0;
	uint8_t started=0;
	char IPD[20];
	bool rc = FALSE;
	char *q;
	uint8_t stopRead=0;
	uint16_t chars=0;
	char str[10];
	if (inIPD!=5) wifiProcessBuffer();
	IPD[0]=0;
	bS=bufStart;
	bE=bufEnd;
	if (bS>bE) oF=1;
	q=esp8266RxBuffer+bS;
	if(inIPD==5)
	{
		  if (bS+15<ESP8266_RX_BUFFER_LEN) strncpy(IPD,q,15);
		  else
			{
				// strands over buffer end
				strncpy(IPD,q,ESP8266_RX_BUFFER_LEN-bS);
				strncpy(IPD+(ESP8266_RX_BUFFER_LEN-bS),esp8266RxBuffer,15-(ESP8266_RX_BUFFER_LEN-bS));
			}
			IPD[15]=0;
			#ifdef doUSB
			if (hasUSB==2) usb_printf("\r\nIPD = %s",IPD);
			#endif
			chars=parseIPD(IPD);
			sprintf(str,"%d",chars);
			//if (hasUSB) usb_printf("\r\nLEN:");
			//if (hasUSB) usb_printf(str);
			while (!stopRead)
			{
				if (started) 
				{	
					tcpRxBuffer[i] = *q;
					i++;
					if (i>TCP_RX_BUFFER_LEN) i=0;
					if (i>=chars) 
					{
							stopRead=1; // put x chars into buffer	
							//if (hasUSB) usb_printf("\r\nSTOP READ");
							bufStart=bS;
							rc=TRUE;
					}
				}
				if (*q==':') started=1;
				bS++;
				q++;
				if (bS >= ESP8266_RX_BUFFER_LEN) 
				{
						q=esp8266RxBuffer; // return to start
						oF=0;
						bS=0;
				}
				if (oF==0 && bS>=bufEnd) stopRead=1; // at buffer end
			}
			tcpRxBuffer[i]=0; //terminate
			tcpBufferHead=i;
			inIPD=0;
	}
	if (bufStart>=ESP8266_RX_BUFFER_LEN)
	{
		 bufStart=0;
	}
	if (WIFICopyMode==2 && tcpBufferClient==4 && chars==50 && Cube_MODE!=6) processSlaveUDP(); // Host has sent a command - mirror it here 
	//if (hasNTP==2) usb_printf("\r\nwait. %d",chars);
	//if (hasNTP==2 && chars==48) interpretNTPTime(); // NTP Request is on channel 3 48 bytes
	if (tcpBufferClient==3 && chars==48) interpretNTPTime();
	return(rc);
}		

uint16_t esp8266SearchData(const char * find,const char * resp,uint8_t strip)
{
	bool rc;
	uint8_t byteOfLen = 0;
	char *p;
	char *q;
	rc = esp8266ReadForResponse(resp, COMMAND_RESPONSE_TIMEOUT);
	if(rc)
	{
		q=esp8266RxBuffer+bufStart;
		p = strstr(q, find);
		p += strlen(find); // don't include data we are looking for
		p += strip; // strip chars at start
		if(p)
		{
			while(*p != '\r' && byteOfLen < bufEnd) // while no new line and not going off the end of the buffer
			{			
				tcpRxBuffer[byteOfLen] = *p;
				p++;
				byteOfLen++;
			}
			tcpRxBuffer[byteOfLen-strip]=0;
			return (byteOfLen-strip);
		}
		else
			return 0;
	}
	else
	{
		return 0;
	}
}

bool esp8266RxBufferAvailable(uint16_t bS)
{
	return (bS != bufEnd) ? TRUE:FALSE;
}

bool esp8266SearchBuffer(uint16_t bS, const char * toFind, uint8_t changeStart)
{
	uint16_t bE=0, oF=0;
	uint16_t match=0;
	uint16_t len=strlen(toFind);
	bool found=FALSE;
	const char *p;
	const char *q=esp8266RxBuffer;
	bE=bufEnd;
	if (bS>bE) oF=1;
  p=toFind;
  while (((bS<bE && oF==0) || oF==1) && !found)
	{
		if (*p==*(q+bS))
				// found a char match
		{
			p++;
			match++;
			if (match==len) found=TRUE;
		}
		else
		{
			p=toFind;
			match=0;
		}
		bS++;
		if (bS>=ESP8266_RX_BUFFER_LEN)
		{
			 bS=0;
			 oF=0;
	  }
	}
	if (found==TRUE) 
	{
		if (changeStart) 
		{
			bufStart=bS; // Should we change the start place of the buffer or not
		}
		#ifdef doUSB
		if (hasUSB==2) usb_printf("\r\nResult : %s",toFind);
		#endif
		return TRUE;
	}
	else return FALSE;
}

void lcdProgress(uint8_t counter)
{
	char block[2];
	block[0]=0xff;
	block[1]='\0';	
	if (lcdMaxCol>=20) 
	{
		lcd_printxy(97,counter*2,block); //block char
		lcd_printxy(97,counter*2+1,block); //block char
	}
	else 
	{
		if (counter<=5) 
		{
			lcd_printxy(97,counter*2,block); //block char
			lcd_printxy(97,counter*2+1,block); //block char
		}
		else lcd_printxy(97,counter+6,block); //block char
	}	
}

void initWIFI()
{ uint8_t counter=0;
	bool rc;
	#ifdef doUSB		
	char p[5];
	#endif
	setAll(35,35,35);
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\n\r\nAttempting WIFI connection at %d BAUD\r\n",ESP8266BAUD);
	#endif
	if (hasLCD) lcd_println(99,1,"Init WIFI");
	//esp8266Test(); // initial connect
	delay(60); //100
	esp8266SendCommand(ESP8266_RESET,ESP8266_CMD_EXECUTE,0); // Reset WIFI
	delay(3300); //5000
	esp8266SendCommand(ESP8266_VERSION,ESP8266_CMD_EXECUTE,0); // Version
	esp8266SendCommand(ESP8266_DISCONNECT,ESP8266_CMD_EXECUTE,0); // Disconnect any WIFI
	delay(60);
	setAll(BLACK);
	if (esp8266Test())
	{
		if (hasLCD) lcd_clearln(97);
		// Delay for 2 seconds
		for (counter=0;counter<10;counter++)
		{
			delay(200); // Wait for Wifi Boot			
			if (hasLCD) lcdProgress(counter);
			if (mySize==8) {if (counter>1) Box(0,counter-2,0,myTotalX-1,counter-2,myTotalZ-1,YELLOW,1,1);}
			else {Box(0,counter,0,myTotalX-1,counter+2,myTotalZ-1,YELLOW,1,1);}
		}
		if(esp8266Begin()) 
		{
			hasWIFI=1;
		}
		else 
		{
			hasWIFI=0;
			setAll(PINK);
			if (hasLCD) 
			{
				lcd_println(98,1,"Init WIFI - ERROR");
				lcd_println(99,1,"Comms Error");
			}				
			delay(1000);
		}
	
		if (hasWIFI)
		// Connect to the controller
		{
			if (hasLCD) lcd_clearln(97);
			// Delay for 3 seconds
			for (counter=0;counter<10;counter++)
			{
				delay(300);
				if (hasLCD) lcdProgress(counter);
			}
			// DHCP
			if (strcmp(myIPAddress,"\0")==0) usartSendArrar(ESP8266_USART,"AT+CWDHCP_CUR=2,1\r\n");
			else usartSendArrar(ESP8266_USART,"AT+CWDHCP_CUR=2,0\r\n");
			delay(200);
			esp8266ClearBuffer();
			if (esp8266Connect(wifiSSID,wifiPSWD))
			{
			  // connected !
				hasWIFI=2;
				setAll(BLACK);
				// Delay for 2 seconds
				if (hasLCD) lcd_clearln(97);
				for (counter=0;counter<10;counter++)
				{
					delay(200); // Wait for Wifi Connect					
					if (hasLCD) lcdProgress(counter);
					if (mySize==8) {if (counter>1) Box(0,counter-2,0,myTotalX-1,counter-2,myTotalZ-1,ORANGE,1,1);}
					else {Box(0,counter,0,myTotalX-1,counter+2,myTotalZ-1,ORANGE,1,1);}
				}
				if (strcmp(myIPAddress,"\0")==0) rc=esp8266GetIPAddress();
				else rc=esp8266SetIPAddress();
				if(rc)
				{
					hasWIFI=3;
					setAll(GREEN);
					delay(300);
					if (ESP8266CHANGE==1) ESP8266ChangeSpeed(); // Change BAUD to 1500000
					if (hasUSB)
					{					 
						#ifdef doUSB
						usb_printf("\r\nIP Address = %s\r\n",myIPAddress);
						#endif
					}
					if (hasLCD) lcd_println(94,1,"%s",myIPAddress);
					esp8266SendCommand(ESP8266_ECHO_DISABLE,ESP8266_CMD_EXECUTE,0);
					delay(200);
				}
				else
				{
					if (hasLCD) 
					{
						lcd_println(98,1,"Init WIFI - ERROR");
						lcd_println(99,1,"IP Address");
					}		
				}
			}
			else
			{
				if (hasLCD) 
				{
				lcd_println(98,1,"Init WIFI - ERROR");
				lcd_println(99,1,"Username / Password");
				}		
			}
		}
	}
	else 
	{
		hasWIFI=0;
		if (hasLCD) 
		{
			lcd_println(98,1,"Init WIFI - ERROR");
			lcd_println(99,1,"No Connection");
		}		
	}
	if (hasWIFI!=3)
	{
		// something went wrong
		hasWIFI=0;
 	  setAll(RED);
	  #ifdef doUSB
  	if (hasUSB) 
		{
			usb_printf("\r\nWIFI Details :");
			sprintf(p,"%d",hasWIFI);
	    usb_printf(p);
			usb_printf("\r\n");
			usb_printf((char *)esp8266RxBuffer);	// Print out Wifi Details if USB is attached
		}	
    #endif	
		delay(1300);
	}
	setAll(BLACK);
	esp8266ClearBuffer();
}

void printWifiBuffer(uint16_t bS)
{
	uint16_t bE=0;
	if (bS>0) bE=bS-1;
	else bE=ESP8266_RX_BUFFER_LEN;
	usb_printf("\r\nWIFI BUFFER :\r\n\r\n");
	while (bS!=bE)
	{
		usb_printf("%c",esp8266RxBuffer[bS]);
		bS++;
		if (bS>ESP8266_RX_BUFFER_LEN) bS=0;
	}
	usb_printf("\r\n");
}

void wifiProcessBuffer(void)
{
	// search for +IPD in buffer
	uint16_t bS=0, bE=0, oF=0;
	char c;
	bS=bufStart; // Store the start and end as they will change with RX DMA
	bE=bufEnd;
	if (bS>bE) oF=1;
	while(((bS<bE && oF==0) || oF==1) && inIPD!=5)
	{
		 c=esp8266RxBuffer[bS];
		//usb_printf("\r\nPROCESS %c",c);
		 switch(inIPD)
		 {
				case 0 : if (c=='+') inIPD++;break;
				case 1 : if (c=='I') inIPD++;else inIPD=0;break;
				case 2 : if (c=='P') inIPD++;else inIPD=0;break;
				case 3 : if (c=='D') inIPD++;else inIPD=0;break;
				case 4 : if (c==',') 
				{
						inIPD=5;
					  bufStart=bS+1;
					  if (bufStart>=ESP8266_RX_BUFFER_LEN) bufStart=0;
					  //usb_printf("\r\nGot IPD : %c%c%c%c%c",esp8266RxBuffer[bS],esp8266RxBuffer[bS+1],esp8266RxBuffer[bS+2],esp8266RxBuffer[bS+3],esp8266RxBuffer[bS+4]);
					  //printWifiBuffer(bufStart);
				}
				else inIPD=0;
			}	
     bS++;
		 if(bS>=ESP8266_RX_BUFFER_LEN)
		 {
			  bS=0;
			  oF=0;
		 }
	}
	if (inIPD!=5)
	{
		// Not Found - Advance buffer pointers
		bufStart=bE;
	}
}

void WIFI_IRQ(char c)
{
	  if (hasWIFI!=4)
		{
			esp8266RxBuffer[bufEnd] = c;
			bufEnd++;
			if(bufEnd >= ESP8266_RX_BUFFER_LEN)
			{
				//overFlow = 1;
				bufEnd = 0;
			}
		}
		else
		{
			//CUBE8 IRQ
			esp8266RxBuffer[bufTemp] = c;
		  bufTemp++;
			if(bufTemp >= ESP8266_RX_BUFFER_LEN)
			{
				bufTemp = 0;
			}
		}
    //if (hasUSB) usb_printf("%c",c); // Comment this
}

void WIFI_REC_DMA(uint16_t ndtr)
{
	 //usb_printf("\r\nGOT WIFI... %d,%d",ndtr,bufEnd);
	 //printWifiBuffer(bufStart);
	 if (ledCubeType>1)  bufEnd=ESP8266_RX_BUFFER_LEN-ndtr; //DMA CUBE12
	 else bufEnd=bufTemp; //IRQ CUBE8
}
