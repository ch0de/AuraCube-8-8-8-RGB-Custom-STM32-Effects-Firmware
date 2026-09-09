#include "cubefile.h"	 
#include "mmc_sd.h"
#include "exfuns.h"
#include "malloc.h"		  
#include "ff.h"
#include "string.h"
#include "LED.h"
#include "System.h"
#include "stdio.h"
#include "ESP8266WIFI.h"
#include "CubeLink.h"
#ifdef doUSB
#include "tunes.h"
	volatile uint8_t DirstrM = 0;
	volatile uint8_t DirstrW = 0;
  uint8_t musicPath[80];
#endif

volatile uint8_t replaceHTML=0;
volatile uint32_t totalHtmlLen=0;  
volatile DWORD filePos=0;
volatile uint8_t SDinUse=0;

volatile uint8_t DirstrL = 0;
volatile uint8_t lastMusicFile = 255;
uint8_t StringPath[80];

uint8_t shufflePointer=255; //Init state
uint8_t shuffleMusic[100]; // File order

u8 mf_mount(u8* path, u8 mt)
{
	return f_mount(fs[0], (const TCHAR*)path, mt);
}

u8 mf_read(u16 len)
{
	u16 i;
	u8 res = 0;
	u16 tlen = 0;
	for (i = 0;i < len / 512;i++)
	{
		res = f_read(file, fatbuf, 512, &br);
		if (res)
			break;
		else
			tlen += br;
	}
	if (len % 512)
	{
		res = f_read(file, fatbuf, len % 512, &br);
		if (res)	//�����ݳ�����
		{
		}
		else
			tlen += br;
	}
	return res;
}


uint8_t strLenght(char *p)
{
	uint8_t lenght = 0;
	while(*p != '\0')
	{
		lenght++;
		p++;
	}
	return lenght;
}

void fileDirectory(uint8_t *s1,char *s2)
{
	uint8_t strL1 = 0, strL2 = 0, i = 0;	
	strL1 = strLenght((char *)s1);
	for(i = 0; i < strL1; i++)
		StringPath[i] = s1[i];
		
	StringPath[strL1] = '\\';
	
	strL2 = strLenght(s2);
	for(i = 0; i < strL2; i++)
		StringPath[i + strL1 + 1] = s2[i];
		
	StringPath[strL1 + strL2 + 1] = '\0';
}

uint8_t Directory_str(uint8_t *s1)
{
	uint8_t strL1 = 0, i = 0;	
	strL1 = strLenght((char *)s1);
	for(i = 0; i < strL1; i++)
		StringPath[i] = s1[i];
	
	StringPath[strL1] = '\\';
	
	return (strL1+1);
}


void file_str(uint8_t num, char *s1)
{
	uint8_t strL1 = 0, i = 0;	
	strL1 = strLenght(s1);
	for(i = 0; i < strL1; i++)
		StringPath[i + num] = s1[i];
	
	StringPath[num + strL1] = '\0';
}


uint8_t ASCII_TO_HEX(char datM,char datS)
{
	uint8_t temp = 0;
	temp = datM&0x0F;
    if (datM>'9') temp+=9;
	temp <<= 4;
	temp += datS&0x0F;
    if (datS>'9') temp+=9;
	return temp;
}

uint8_t mf_getSemaphore(uint8_t mode)
{
	uint32_t timeout=0;
	if (mode==1 && SDinUse==4) SDinUse=0;
	while(SDinUse && timeout<500000) 
	{
		 // Wait until SD Card Free
	 timeout++;
	} 
	if (timeout==500000 && hasUSB) 
	{
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nTIMEOUT ERROR: mf_getSemaphore - Waiting for SD Card %d",SDinUse);
		#endif
		return 0;
	}
	SDinUse=1;		
	return 1;
}
void mf_openText(char* fileName,uint8_t mode)
{
	// Display Text File on the CUBE
	FRESULT res;
	if (mf_getSemaphore(0))
	{
		res=f_open(file, fileName, FA_READ); 	
		if (res == FR_OK)
		// File Exists
		{
			#ifdef doUSB
			if (hasUSB) usb_printf("\r\nOpened %s for Text Display\r\n",fileName);
			#endif
		}
		SDinUse=0;
	}
}

uint8_t mf_readText(uint8_t mode)
{
	// Display Text File on the CUBE
	char c[2];
	if (!f_eof(file))
	{
		if (mf_getSemaphore(0))
		{		
			f_gets((TCHAR*)c,2,file); // Get one char at a time
      SDinUse=0;			
			#ifdef doUSB
			if (hasUSB) usb_printf("%c",c[0]);
			#endif
			if (mode==0) writeChar(c[0],mode);		
			if (mode==1) scrollReadText(c[0]);
		}
	}		
	else	
	{
		if (mode==1) 
		{
			scrollFinished=1;
			while (scrollReadText(' ')==0);
		}
		f_close(file);
		return 1;
	}
	return 0;
}

void mf_readLayoutFile(char* fileName)
{
	TCHAR line[81];
	uint8_t i,j;
	uint8_t a,b,c,d;
	char* comment;
	//uint8_t invalid =0;
	FRESULT res;

	res=f_open(file, fileName, FA_READ); 	
  if (res == FR_OK)
	// File Exists
	{
		#ifdef doUSB
	  displayOwnLED=0;
	  #endif
		populateDMAGrid(0); // Initialise DMA GRID Structure, but don't populate this time
		while(!f_eof(file))
		{	
			// Overload StringPath ;)
			f_gets((TCHAR*)fatbuf,80,file);       
			//Find first comma
			// Copy substring characters until you reach ','
			for (i=0; fatbuf[i] != ',' && fatbuf[i] != 0 && i<80 ; i++) 
			{
				line[i] = fatbuf[i];
			}
			if (fatbuf[i] == 0) continue; // Not valid
			line[i] = 0;                    // Add string terminator at the end of substring
			comment=strstr(line,"//"); 					// Look for comments
			if (!comment)
			{	
				a=atoi(line);
				j=i+1;
				for (i=0; fatbuf[j+i] != ',' && fatbuf[j+i] != 0 && j+i<80 ; i++) 
				{
					line[i] = fatbuf[j+i];
				}
				if (fatbuf[j+i] == 0) continue; // Not valid
				line[i] = 0;                    // Add string terminator at the end of substring
				b=atoi(line);
				j+=i+1;
				for (i=0; fatbuf[j+i] != '=' && fatbuf[j+i] != 0 && j+i<80 ; i++) 
				{
					line[i] = fatbuf[j+i];
				}
				if (fatbuf[j+i] == 0) continue; // Not valid
				line[i] = 0;                    // Add string terminator at the end of substring
				c=atoi(line);
				j+=i+1;
				for (i=0; fatbuf[j+i] != '\r' && fatbuf[j+i] != 0 && j+i<80 ; i++) 
				{
					line[i] = fatbuf[j+i];
				}
				line[i] = 0;                    // Add string terminator at the end of substring
				d=atoi(line);		
				#ifdef doUSB
				if (hasUSB) usb_printf("\r\nCube Layout %d,%d,%d assigned %d",a,b,c,d);	
				#endif
				if (d==0) 
				{
					cubeLayout[a][b][c]=10;
					#ifdef doUSB
					displayOwnLED=1;
					#endif
				}
				else 
				{
					if (a<layoutMaxX&&b<layoutMaxY&&c<layoutMaxZ) cubeLayout[a][b][c]=d;
					#ifdef doUSB
					else if (hasUSB) usb_printf("\r\nCube Layout Out of Bounds for %d",d);	
					#endif
					if (c==9) 
					{
						#ifdef doUSB
						if (hasUSB) usb_printf("\r\nOwn Cube");
						#endif
						cubeLayout[a][b][2]=d+20; // Own Cube
					}
				}
			}
		}
		f_close(file);
	}
}

void mf_readConfigFile(char* fileName)
{
	TCHAR option[81];
	TCHAR value[81];
  uint8_t i,j;
	FRESULT res;
	uint8_t inProgram = 0;
	
  graphicOrderMaxTemp=0;
  res=f_open(file, fileName, FA_READ);  	
  if (res == FR_OK)
				// File Exists
			{
			  if (hasUSB) usb_printf("\r\nLoading Config File");
				while(!f_eof(file))
				{
					// Overload StringPath ;)
					f_gets((TCHAR*)fatbuf,80,file);
					//Find first space 
					// Copy substring characters until you reach ' '
					for (i=0; fatbuf[i] != ' ' && fatbuf[i] != 0 && i<80 ; i++) 
					{
							option[i] = fatbuf[i];
					}
					option[i] = 0;                    // Add string terminator at the end of substring
					for (j=0; fatbuf[i+j+1] != ' ' && fatbuf[i+j+1] != 0 && j<80 ; j++) 
					{
							value[j] = fatbuf[i+j+1];
					}
					value[j] = 0;                    // Add string terminator at the end of substring
					if (!inProgram)
					{
						executeCommand(option,value);
						
						if (strncmp(option,"Program",7)==0)
						{
							inProgram=1;
						}
					}
					else
					{
						//program[programCounter][0]=1;
						//program[programCounter][1]=atoi(value);
						// Inside programming section
						if (strncmp(option,"End",3)==0||programCounter>98)
						{
							inProgram=0;
						}						
						programCounter++;
					}				
				}
				f_close(file);
				if (ledCubeType>1) mf_readLayoutFile("layout.txt"); // Layout Only for Cube12
		}
		else
		{
		  // No Config File - Default to Read SD as per normal cube
      Cube_MODE=0;			
		}
}

FRESULT mf_load_path(char* path)
{
	FRESULT res;
	DirstrL = 0;
	res = f_opendir(&dir, (const TCHAR*)path); //��һ��Ŀ¼
	DirstrL = Directory_str((uint8_t*)path);	//����Ŀ¼�ļ����ַ���
	return res;
}

FRESULT mf_load_path_gen()
{
	// Generic Load Path for Cube8 and Cube12
	FRESULT res;

	if (ledCubeType==0 || ledCubeType==1) res=mf_load_path("0:\\rgbCUBE888");
	if (ledCubeType==2 || ledCubeType==3 || ledCubeType==4) res=mf_load_path("0:\\rgbCUBE12");	
	return res;
}

FRESULT mf_load_next_file(FIL *fp, DIR *dp, uint8_t fileNum)
{
	uint8_t strL = 0;
	uint8_t pos =0;
	uint8_t fileOpen=0;
	FRESULT res;
	char *fn;   /* This function is assuming non-Unicode cfg. */
	
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nLoad Next file %d",fileNum);
	#endif
	while (pos <fileNum)
	{
		// move through the directory
		f_skimdir(dp); 
		pos++;
	}
	res = f_readdir(dp, &fileinfo);
	if (res == FR_OK && fileinfo.fname[0] != 0) 
	{
		fn = &fileinfo.lfname[0];
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nOpen File: %s",fn);
		#endif
		strL = strLenght(fn);							//�ַ�������
		if(strL > 9 && (ledCubeType==0 || ledCubeType==1)) // This method for RGBCube8 and AuraCube8 Only
		{			
			//.CUBE8
			if ('.' == fn[strL - 9] && 'r' == fn[strL - 8] && 'g' == fn[strL - 7] && 
	  			'b' == fn[strL - 6] && 'C' == fn[strL - 5] && 'U' == fn[strL - 4] && 
					'B' == fn[strL - 3] && 'E' == fn[strL - 2] && '8' == fn[strL - 1])
			{
				file_str(DirstrL,fn);	
				f_open(fp, (const TCHAR*)StringPath, FA_READ);	
				fileOpen=1;
			}
		}
		if(strL > 7 && (ledCubeType==2 || ledCubeType==3 || ledCubeType==4))
		{
			if (ledCubeType==2)
			{
				//.CUBE12
				if ('.' == fn[strL - 7] && 'C' == fn[strL - 6] && 'U' == fn[strL - 5] && 
						'B' == fn[strL - 4] && 'E' == fn[strL - 3] && '1' == fn[strL - 2] && '2' == fn[strL - 1])
				{
					file_str(DirstrL,fn);		//����Ŀ¼�ļ������ļ������ַ���	
					f_open(fp, (const TCHAR*)StringPath, FA_READ);	//���ļ���						
					fileOpen=1;
				}
			}
			if (ledCubeType==3)
			{
				//.WIDE12
				if ('.' == fn[strL - 7] && 'W' == fn[strL - 6] && 'I' == fn[strL - 5] && 
						'D' == fn[strL - 4] && 'E' == fn[strL - 3] && '1' == fn[strL - 2] && '2' == fn[strL - 1])
				{
					file_str(DirstrL,fn);		//����Ŀ¼�ļ������ļ������ַ���	
					f_open(fp, (const TCHAR*)StringPath, FA_READ);	//���ļ���	
					fileOpen=1;						
				}					
			}
			if ('.' == fn[strL - 9] && 'r' == fn[strL - 8] && 'g' == fn[strL - 7] && 
	  			'b' == fn[strL - 6] && 'C' == fn[strL - 5] && 'U' == fn[strL - 4] && 
					'B' == fn[strL - 3] && 'E' == fn[strL - 2] && '8' == fn[strL - 1])
			{
				//.CUBE8 - need to load cube8 for multicubes
				file_str(DirstrL,fn);		//����Ŀ¼�ļ������ļ������ַ���	
				f_open(fp, (const TCHAR*)StringPath, FA_READ);	//���ļ���							
				fileOpen=1;					
			}
		}
	}
	else res = FR_INVALID_PARAMETER;
	if (fileOpen==0) res=FR_INVALID_PARAMETER;
	return res;
}

FRESULT mf_read_to_cube(FIL *fp)
{
	// Read file to display on CUBE
	uint16_t sumM = 0, temp1 = 0, temp2 = 0;
	uint16_t x = 0, y = 0, z = 0;
  FRESULT res = FR_OK;
/*	uint8_t TmyTotalX, TmyTotalY, TmyTotalZ;
	
	// New Fudge to get something to load always
	TmyTotalX=myTotalX;
	TmyTotalY=myTotalY;
	TmyTotalZ=myTotalZ;
	if (ledCubeType<2)
	{
		// Cube8
		TmyTotalX=8;
		TmyTotalY=8;
		TmyTotalZ=8;
	}
	else
	{
		// Cube12
		TmyTotalX=12;
		TmyTotalY=12;
		TmyTotalZ=12;		
	}
*/

  br=1;
	for(z = 0; z < myTotalZ; z++)
	{
		  if (!res && br != 0)
			{
				 res = f_read(fp, fatbuf, (2*myTotalX*myTotalZ*3), &br); // only read a row at a time. 2 chars per LED
			}
			if(!res && br != 0)
			{
					for(y = 0; y < myTotalY; y++)
					{
							sumM = y * myTotalX*myTotalZ*3;
							for(x = 0; x < myTotalX; x++)
							{
								  if (ledCubeType==0 || ledCubeType==1)
									{
										temp1 = 3 * (x + (z * myTotalY));  // step by 8 or 12
										temp2 = 6 * ((myTotalX-1) - x + (y * myTotalY));
										rgbled[setLED][rgb_order[temp1 + ROffset] + sumM] = ASCII_TO_HEX(fatbuf[temp2 + 0],fatbuf[temp2 + 1])>>(4-customBright);  //R
										rgbled[setLED][rgb_order[temp1 + GOffset] + sumM] = ASCII_TO_HEX(fatbuf[temp2 + 2],fatbuf[temp2 + 3])>>(4-customBright);  //G
										rgbled[setLED][rgb_order[temp1 + BOffset] + sumM] = ASCII_TO_HEX(fatbuf[temp2 + 4],fatbuf[temp2 + 5])>>(4-customBright);  //B
										if (numCubes>2 && setLED==3) // Copy cube 1 to 2 due to memory constraints
										{
											rgbled[setLED+1][rgb_order[temp1 + 0] + sumM] = ASCII_TO_HEX(fatbuf[temp2 + 0],fatbuf[temp2 + 1])>>(4-customBright);  //R
											rgbled[setLED+1][rgb_order[temp1 + 1] + sumM] = ASCII_TO_HEX(fatbuf[temp2 + 2],fatbuf[temp2 + 3])>>(4-customBright);  //G
											rgbled[setLED+1][rgb_order[temp1 + 2] + sumM] = ASCII_TO_HEX(fatbuf[temp2 + 4],fatbuf[temp2 + 5])>>(4-customBright);  //B											
										}
									}
								  if (ledCubeType==2 || ledCubeType==3 || ledCubeType==4)
									{
										temp1 = 3 * (x + (z * myTotalY));  // step by 8 or 12
										temp2 = 6 * (x + (y * myTotalY));  // No need to swap X for rbgcube12                     
										rgbled[setLED][temp1+0+sumM] = ASCII_TO_HEX(fatbuf[temp2 + 0],fatbuf[temp2 + 1]);//>>(4-customBright);  //R
										rgbled[setLED][temp1+1+sumM] = ASCII_TO_HEX(fatbuf[temp2 + 2],fatbuf[temp2 + 3]);//>>(4-customBright);  //G
										rgbled[setLED][temp1+2+sumM] = ASCII_TO_HEX(fatbuf[temp2 + 4],fatbuf[temp2 + 5]);//>>(4-customBright);  //B
									}										
				 			 } 
					 }
			}
			if (br==0 || res) f_close(fp);
	 }
	 return res;
}

void readSDModeZero()
{	
	FRESULT res;
	uint8_t state = 0;

	if (mf_dirOpen==1)
	{
			f_closedir(&dir);
			mf_dirOpen=0;
	}
	res=mf_load_path_gen();
	if (res == FR_OK)
	{
	  state=mf_load_next_file(file,&dir,0); 
		while(0 == Cube_MODE && state==0)
		{
			mf_read_to_cube(file);
			delay(delaySpeed*6);
			if (br==0)
			{
					f_close(file);
					state=mf_load_next_file(file,&dir,0);
			}
		}
		f_close(file);
		f_closedir(&dir);
	 }
}

void mf_read_fileDMA(void)
{
	uint8_t first=0;
	uint16_t blocks=2048;  // blocks of 2048 
	uint16_t htmlLen=0; 	
	FRESULT res;
	
	if (totalHtmlLen>2048)
	{
			htmlLen=2048;
	}
	else htmlLen=totalHtmlLen;
	esp8266TcpSend(tcpBufferClient,htmlLen);
	res=f_read(htmlFile, fatbuf, blocks, &br);	
  if (res!=0) 
	{
		// Error Condition
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nERROR: SD Read File");
		#endif
		totalHtmlLen=0; 
	}
	if (first==0 && inSend==1) // first char
	{
		if (fatbuf[15]=='c' && fatbuf[16]=='u' && fatbuf[17]=='b' && fatbuf[18]=='e' ) replaceHTML=1;
		else replaceHTML=0;
        // doctype cube					
		first=1;
	}
	if (replaceHTML)
	{
		#ifdef doUSB
		if (hasUSB>1) usb_printf("\r\nReplace HTML Variables");
		#endif
		replaceHTMLVariables(blocks);
	}
	esp8266TcpSendBufferDMA((char *)fatbuf,br);
  totalHtmlLen-=br;
	if (totalHtmlLen>0) inSend++;
	else inSend=88;
}

void mf_read_file(void)
{
	uint8_t split=0;
	uint8_t first=0;
	uint16_t blocks=512;  // blocks of 512 
	uint16_t htmlLen=0; 	
	uint32_t timeout=0;
  FRESULT res;
	
	if (totalHtmlLen>2048)
	{
			htmlLen=2048;
	}
	else htmlLen=totalHtmlLen;
	while(totalHtmlLen>0 && split <4) // only send maximum once each run
	{
			res=f_read(htmlFile, fatbuf, blocks, &br);
    	if (res!=0) 
			{
				// Error Condition
				#ifdef doUSB
				if (hasUSB) usb_printf("\r\nERROR: SD Read File");
				#endif
				totalHtmlLen=0; 
			}
			if (first==0 && inSend==1) // first char
			{
				if (fatbuf[15]=='c' && fatbuf[16]=='u' && fatbuf[17]=='b' && fatbuf[18]=='e' ) replaceHTML=1;
				else replaceHTML=0;
        // doctype cube					
				first=1;
			}
			if (replaceHTML)
			{
				#ifdef doUSB
				if (hasUSB) usb_printf("\r\nReplace HTML Variables");
				#endif
				replaceHTMLVariables(blocks);
			}
			if (split==0) // every 4th
			{
					if (totalHtmlLen>2048)
					{
							htmlLen=2048;
					}
					else htmlLen=totalHtmlLen;
					esp8266TcpSend(tcpBufferClient,htmlLen);
					esp8266TcpSendBufferDMA((char *)fatbuf,br);
			}
			else
			{
					esp8266TcpSendBufferDMA((char *)fatbuf,br);
			}
			totalHtmlLen-=br;
			split++;
			if (totalHtmlLen>0 && split <4) 
			{
				if (ledCubeType<2)
				{
					while (SDinUse==2 && timeout<500000)
					{
						// Wait for last DMA to finish, can't read coz it will corrupt fatbuf
						timeout++;
					}
					#ifdef doUSB
					if (timeout==500000 && hasUSB) usb_printf("\r\nTIMEOUT ERROR: mf_read_file - Waiting for SD Card");
					#endif
				}
			}
	}	
	if (split==4 && totalHtmlLen>0) inSend++;
	else inSend=88;
  //if (hasUSB) usb_printf("\r\nDone Part Send\r\n");
}

void mf_read_html(char* fullFile)
{
	FRESULT res;
	if (inSend==1)
	{
		filePos=0;
		strcpy(c,"/Server");
		strcat(c,fullFile);	
		if (strlen(fullFile)<30)
		{
			res=f_open(htmlFile,c, FA_READ);	
			if (res == FR_OK)
			{
				sendOKHTTPHeader();
				totalHtmlLen=htmlFile->fsize;
				if (ledCubeType>1) mf_read_fileDMA();
				else mf_read_file(); 
				f_close(htmlFile);
			}
			else
			{
				#ifdef doUSB
				if (hasUSB) usb_printf("\r\nUnable to Serve - File Not Found %d",res);
				#endif
				strcpy(c,"HTTP/1.1 404 File Not Found\r\n\r\n"); //Connection: Close\r\n\r\n");  // HTTP Header
				esp8266TcpSend(tcpBufferClient,strlen(c));
				esp8266TcpSendBuffer(c,strlen(c));
				inSend=88;
				f_close(htmlFile);
			}
		}
		else
		{
				#ifdef doUSB
				if (hasUSB) usb_printf("\r\nUnable to Serve - File Too Long");
				#endif
				strcpy(c,"HTTP/1.1 404 File Not Found\r\n\r\n"); //Connection: Close\r\n\r\n");  // HTTP Header
				esp8266TcpSend(tcpBufferClient,strlen(c));
				esp8266TcpSendBuffer(c,strlen(c));
				inSend=88;
				f_close(htmlFile);			
		}
	}
	else
	{
		strcpy(c,"/Server");
		strcat(c,fullFile);
		res=f_open(htmlFile,c, FA_READ);	
		f_lseek(htmlFile,filePos);
		if (ledCubeType>1) mf_read_fileDMA();
		else mf_read_file(); 
	}
	filePos+=2048;
	f_close(htmlFile);
}

#ifdef doUSB

uint8_t Music_str(uint8_t *s1)
{
	uint8_t strL1 = 0, i = 0;	
	strL1 = strLenght((char *)s1);
	for(i = 0; i < strL1; i++)
		musicPath[i] = s1[i];
	
	musicPath[strL1] = '\\';
	
	return (strL1+1);
}

void musicFile_str(uint8_t num, char *s1)
{
	uint8_t strL1 = 0, i = 0;	
	strL1 = strLenght(s1);
	for(i = 0; i < strL1; i++)
		musicPath[i + num] = s1[i];
	
	musicPath[num + strL1] = '\0';
}

FRESULT musicOpenPath(char* path)
{
	FRESULT res;
	DirstrM = 0;
	f_closedir(&musicDir);
	res = f_opendir(&musicDir, (const TCHAR*)path); 
	DirstrM = Music_str((uint8_t*)path);	
	return res;
}

uint16_t countMusicFiles(char* path)
{
	uint16_t count = 0;
	FRESULT res = FR_OK;
	uint8_t state = 0;
	uint8_t type = 0;
	f_closedir(&musicDir);
	res=musicOpenPath(path);
  if (res == FR_OK)
	{
		while(state==0)
		// count up through the files
		{
			state=load_next_sound_file(musicFile,&musicDir,0,&type,0);
			f_close(file);
			count++;
		}
	}
	return(count);
}

void shuffleSongs(char *path)
{
	uint8_t numFiles=0;
	uint8_t i=0;
	uint8_t j=0;
	uint8_t next=0;
	uint8_t placed=0;
	uint8_t found=0;
	
	for (j=0;j<100;j++)
	{
		shuffleMusic[j]=255;
	}
	numFiles=countMusicFiles(path)-1;
	if (numFiles==0)
	{
		musicPlay=0;
		currentPlay=0;
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nCouldn't load any Mod or Wave Music Files");
		#endif
		return;
	}
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nFound %d Music Files to SHUFFLE",numFiles);
	#endif	
	while (i<numFiles && i<100)
	{
		placed=0;
		while (!placed)
		{
			next=cube_random(numFiles);
			found=0;
			for (j=0;j<i;j++)
			{
				if (shuffleMusic[j]==next) found=1;
			}
			if (!found) 
			{
				shuffleMusic[i]=next;
				placed=1;
			}
		}
		i++;
	}
}
	
void playShuffleSound(char *path)
{
	uint8_t next;
	uint8_t type = 0;
	FRESULT res;
	
	currentPlay+=10;
	if (shufflePointer==255) 
	{
		// Re-Shuffle
		shuffleSongs(path);
		shufflePointer=0;
		next=shuffleMusic[shufflePointer];
		if (currentPlay==0) return; // Function had a problem
	}	
	else shufflePointer++;
	if (shufflePointer>99) 
	{
		shufflePointer=0;
		next=255;
	}
	else next=shuffleMusic[shufflePointer];
	if (next==255)
	{
		shufflePointer=0;
		next=shuffleMusic[shufflePointer];
		if (currentPlay==0) return; // Function had a problem
	}
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nShuffle Playing file %d is #%d",shufflePointer,next);
	#endif
	// play X number from the SD
	f_closedir(&musicDir);
	res=musicOpenPath(path);
  if (res == FR_OK)
	{	
	  res=load_next_sound_file(musicFile,&musicDir,next,&type,1);
	  if (soundOn)
	  {
  	  if (res == FR_OK && type==2)
	    {
		    runWave();
    	}
	    if (res == FR_OK && type==1)
	    {	
			  musicPlaying=90;
		    runTune();
			  if (musicPlaying==90) musicPlaying=0;
	    }
	  }
	}
	currentPlay-=10;
}

void playRandomSound(char *path)
{
	uint8_t ran;
	uint8_t numFiles=0;
	uint8_t type = 0;
	FRESULT res;
	
	currentPlay+=10;
	numFiles=countMusicFiles(path);
	if (numFiles==0)
	{
		musicPlay=0;
		currentPlay=0;
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nCouldn't load any Mod or Wave Music Files");
		#endif
		return;
	}
	ran=cube_random(numFiles);
	if(numFiles>1)
	{
		// Don't play the same file twice
		while (ran==lastMusicFile)
		{
				ran=cube_random(numFiles);
		}
	}
	lastMusicFile=ran;
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nRandom Playing file #%d",ran);
	#endif
	// play X number from the SD
	res=load_next_sound_file(musicFile,&musicDir,ran,&type,1);
	if (soundOn)
	{
  	if (res == FR_OK && type==2)
	  {
		  runWave();
  	}
	  if (res == FR_OK && type==1)
	  {	
			musicPlaying=90;
		  runTune();
			if (musicPlaying==90) musicPlaying=0;
	  }
	}
	currentPlay-=10;
}

FRESULT mf_read_to_tune(void)
{
	// Read file to play as a tune
	uint32_t len=0;
	FRESULT res;
  
	len=musicFile->fsize;
  if (len<55500) // less than 55.5k
	{
	  if (mf_getSemaphore(0))
	  {
      res=f_read(musicFile, mod_pointer, len, &br);	
      if (res==0) 
      {
				 #ifdef doUSB
		     if (hasUSB) usb_printf("\r\nMusic File Loaded. Size %d",br);
				 #endif
	    }	
	    else
  	  {
				#ifdef doUSB
  	    if (hasUSB) usb_printf("\r\nUnable to Load Music File");
				#endif
      }
			SDinUse=0;
		}
	}
	else
	{
		 if (hasUSB) usb_printf("\r\nUnable to Load Music File - File is too large = %d",len);
		 res=FR_INVALID_PARAMETER;
	}
  return res;
}

FRESULT load_next_sound_file(FIL *fp, DIR *dp, uint8_t fileNum, uint8_t *type, uint8_t load)
{
	FRESULT res;
	uint8_t pos =0;
	char *fn;   /* This function is assuming non-Unicode cfg. */
	*type=0;
	while (pos <fileNum)
	{
		// move through the directory
		f_skimdir(dp); 
		pos++;
	}	
	res = f_readdir(dp, &fileinfo);
	if (res == FR_OK && fileinfo.fname[0] == 0) 
	{
		// End of Directory
		musicPath[DirstrM-1]='\0'; // remove filename
		if (load==1) res = FR_INVALID_PARAMETER;
		res=musicOpenPath((char *)musicPath);
	}
	if (res == FR_OK && fileinfo.fname[0] != 0) 
	{
		fn = &fileinfo.lfname[0];
		if (strlen(fn)==0) fn = &fileinfo.fname[0]; // No Long Filename. Use Short Instead
		//if (hasUSB) usb_printf("\r\nCheck %s",fn);
		if (strncmp(fn,"end",3)==0 || ((fn[strlen(fn)-3] != 'M' || fn[strlen(fn)-2] != 'O' || fn[strlen(fn)-1] != 'D')
		   && (fn[strlen(fn)-3] != 'm' || fn[strlen(fn)-2] != 'o' || fn[strlen(fn)-1] != 'd')
		   && (fn[strlen(fn)-3] != 'W' || fn[strlen(fn)-2] != 'A' || fn[strlen(fn)-1] != 'V')
		   && (fn[strlen(fn)-3] != 'w' || fn[strlen(fn)-2] != 'a' || fn[strlen(fn)-1] != 'v')))
		{
			// Don't process end.mod as this is for game over	
			if (load==1) res = FR_INVALID_PARAMETER;
      return res;				
		}
		if (fn[strlen(fn)-3] == 'M' || fn[strlen(fn)-3] == 'm') *type=1; //Mod
    else *type=2; // Wave
		#ifdef doUSB
		if (hasUSB && load==1) usb_printf("\r\nOpen Sound File %s, type %d",fn,*type);
		#endif
		musicFile_str(DirstrM,fn);
		res=f_open(fp, (const TCHAR*)musicPath, FA_READ);
		
		if (res == FR_OK && load==1)
		{
			if (hasLCD && Cube_MODE!=9) // Don't do for Game Mode
			{
				lcd_println(1,1,fn);
			}
			if (*type==1) res=mf_read_to_tune();
		}
		if (*type==1 && load==1) f_close(musicFile);
	}
	else 
	{
		res = FR_INVALID_PARAMETER;
	}
	return res;	
}

FRESULT load_next_music_file(FIL *fp, DIR *dp)
{
	FRESULT res;
	char *fn;   /* This function is assuming non-Unicode cfg. */
	
	res = f_readdir(dp, &fileinfo);
	if (res == FR_OK && fileinfo.fname[0] == 0) 
	{
		// End of Directory
		musicPath[DirstrM-1]='\0'; // remove filename
		res = FR_INVALID_PARAMETER;
		if (filesPlayed==0)
		{
			if (musicPlay==3)
			{
				musicPlay=1;
				currentPlay=1;
				musicOpenPath((char *)musicPath);
			}
			if (musicPlay==2)
			{
				musicPlay=0;
				currentPlay=0;
			}
			#ifdef doUSB
			if (hasUSB) usb_printf("\r\nCouldn't load any Mod Music Files");
		  #endif			
			return res;
		}
		if (musicPlay==3)
		{
			currentPlay=1;
			filesPlayed=0;	
			musicOpenPath((char *)musicPath);
			return res;
		}
		filesPlayed=0;		
		res=musicOpenPath((char *)musicPath);
	}
	//if (hasUSB) usb_printf("\r\nOK %s",fileinfo.fname);
	if (res == FR_OK && fileinfo.fname[0] != 0) 
	{
		fn = &fileinfo.lfname[0];
		if (strlen(fn)==0) fn = &fileinfo.fname[0]; // No Long Filename. Use Short Instead
		//if (hasUSB) usb_printf("\r\nCheck %s",fn);
		if (strncmp(fn,"end",3)==0 || ((fn[strlen(fn)-3] != 'M' || fn[strlen(fn)-2] != 'O' || fn[strlen(fn)-1] != 'D')
		   && (fn[strlen(fn)-3] != 'm' || fn[strlen(fn)-2] != 'o' || fn[strlen(fn)-1] != 'd')))
		{
			// Don't process end.mod as this is for game over	
			res = FR_INVALID_PARAMETER;
      return res;				
		}
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nOpen Music File %s",fn);
		#endif
		if (hasLCD && Cube_MODE!=9) // Don't do for Game Mode
		{
			lcd_println(1,1,fn);
		}
		filesPlayed++;
		musicFile_str(DirstrM,fn);
		res=f_open(fp, (const TCHAR*)musicPath, FA_READ);
		if (res == FR_OK)
		{
			res=mf_read_to_tune();
		}
		f_close(musicFile);
	}
	else 
	{
		res = FR_INVALID_PARAMETER;
	}
	return res;	
}

FRESULT load_specific_music_file(FIL *fp, char* fileName)
{
	FRESULT res;
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nOpen Music File %s",fileName);
	#endif
  res=f_open(fp, fileName, FA_READ);  
	if (res == FR_OK)
	{
		if (hasLCD && Cube_MODE!=9) // Don't do for Game Mode
		{
			lcd_println(1,1,fileName);
		}
		res=mf_read_to_tune();
	}
	f_close(fp);	
	return res;
}	

uint8_t mf_read_wav(FIL *fp, uint16_t area)
{
	// Read file to play as a wave
	FRESULT res;
	uint8_t ret=0;
  uint16_t blocks=1024;
	
	if (area!=0) area=blocks;
	SDinUse=1;
  res=f_read(musicFile, wavbuf+area, blocks, &br);
	if (res==FR_OK) ret=0;
	else ret=1;
	SDinUse=0;
  if (br<blocks)
  {
		f_close(fp);
		ret=1;
	}
  return ret;
}

FRESULT open_wav_file(FIL *fp, char* path, char* fileName)
{
	FRESULT res;
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nOpen Sound File %s",fileName);
	#endif
	if (hasLCD && Cube_MODE!=9) // Don't do for Game Mode
	{
		lcd_println(1,1,fileName);
	}	
	res=musicOpenPath(path);
	if (res==FR_OK) res=f_open(fp, fileName, FA_READ);
	return res;
}	

FRESULT load_next_wave_file(FIL *fp, DIR *dp)
{
	FRESULT res;
	char *fn;   /* This function is assuming non-Unicode cfg. */
	
	res = f_readdir(dp, &fileinfo);
	if (res == FR_OK && fileinfo.fname[0] == 0) 
	{
		// End of Directory
		musicPath[DirstrM-1]='\0'; // remove filename
		res = FR_INVALID_PARAMETER;
		if (filesPlayed==0)
		{
			if (musicPlay==3)
			{
				musicPlay=2;
				currentPlay=2;
				musicOpenPath((char *)musicPath);
			}
			if (musicPlay==1)
			{
				musicPlay=0;
				currentPlay=0;
			}
			#ifdef doUSB
			if (hasUSB) usb_printf("\r\nCouldn't load any Wave Files");
			#endif
			return res;
		}
		if (musicPlay==3)
		{
			currentPlay=2;
			filesPlayed=0;	
			musicOpenPath((char *)musicPath);
			return res;
		}		
		filesPlayed=0;
		res=musicOpenPath((char *)musicPath);
	}
	if (res == FR_OK && fileinfo.fname[0] != 0) 
	{
		fn = &fileinfo.lfname[0];
		if (strlen(fn)==0) fn = &fileinfo.fname[0]; // No Long Filename. Use Short Instead
		//if (hasUSB) usb_printf("\r\nCheck %s",fn);
		if ((fn[strlen(fn)-3] != 'W' || fn[strlen(fn)-2] != 'A' || fn[strlen(fn)-1] != 'V')
		   && (fn[strlen(fn)-3] != 'w' || fn[strlen(fn)-2] != 'a' || fn[strlen(fn)-1] != 'v'))
		{
			res = FR_INVALID_PARAMETER;
      return res;			
		}
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nOpen Wave File %s",fn);
		#endif
		if (hasLCD && Cube_MODE!=9) // Don't do for Game Mode
		{
			lcd_println(1,1,fn);
		}
		filesPlayed++;
		musicFile_str(DirstrM,fn);
		res=f_open(fp, (const TCHAR*)musicPath, FA_READ);
	}
	else 
	{
		res = FR_INVALID_PARAMETER;
	}
	return res;	
}

FRESULT load_specific_wave_file(FIL *fp, char* fileName)
{
	FRESULT res;
	#ifdef doUSB	
	if (hasUSB) usb_printf("\r\nOpen Wave File %s",fileName);
	#endif
  res=f_open(fp, fileName, FA_READ); 
	return res;
}	

#endif

u8 mf_lseek(u32 offset)
{
	return f_lseek(file, offset);
}


u32 mf_tell(void)
{
	return f_tell(file);
}


u32 mf_size(void)
{
	return f_size(file);
}

#if !_FS_READONLY
u8 mf_mkdir(u8*pname)
{
	return f_mkdir((const TCHAR *)pname);
}
#endif

#if !_FS_READONLY
u8 mf_fmkfs(u8* path, u8 mode, u16 au)
{
	return f_mkfs((const TCHAR*)path, mode, au);//��ʽ��,drv:�̷�;mode:ģʽ;au:�ش�С
}
#endif

#if !_FS_READONLY
u8 mf_unlink(u8 *pname)
{
	return  f_unlink((const TCHAR *)pname);
}
#endif

#if !_FS_READONLY
u8 mf_rename(u8 *oldname, u8* newname)
{
	return  f_rename((const TCHAR *)oldname, (const TCHAR *)newname);
}
#endif

void mf_getlabel(u8 *path)
{
	u8 buf[20];
	u32 sn = 0;
	u8 res;
	res = f_getlabel((const TCHAR *)path, (TCHAR *)buf, (DWORD*)&sn);
	if (res == FR_OK)
	{
	}
}

#if !_FS_READONLY
void mf_setlabel(u8 *path)
{
	u8 res;
	res = f_setlabel((const TCHAR *)path);
	if (res == FR_OK)
	{
	}
}
#endif

void mf_gets(u16 size)
{
	TCHAR* rbuf;
	rbuf = f_gets((TCHAR*)fatbuf, size, file);
	if (*rbuf == 0)
		return;							//û�����ݶ���
	else
	{
	}
}

#if !_FS_READONLY
u8 mf_putc(u8 c)
{
	return f_putc((TCHAR)c, file);
}
#endif

#if !_FS_READONLY
u8 mf_puts(u8*c)
{
	return f_puts((TCHAR*)c, file);
}
#endif
