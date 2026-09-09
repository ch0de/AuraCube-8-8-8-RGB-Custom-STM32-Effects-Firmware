#include "CubeLink.h"
#include "usart.h"
#include "System.h"

uint16_t DMASize = ledArraySize; // Max Size. Can't go bigger than this
uint8_t nextDMA = 1;
uint8_t inDMA = 0;
uint8_t inSendDMA =0;
uint8_t sendDMA = 0;
uint8_t stopDMAReq = 0;
uint8_t oldSendDMA=0;
//uint8_t oldStopDMAReq=0;
uint8_t DMAAddresses[10]; // Other Cubes
uint8_t DMASizes[10]; // Other Cubes Sizes
uint8_t DMAPointer=0;  // next one to send
uint8_t DMAPoint=0; // where to store
volatile uint8_t foundHost=0; // Is there a host cube ?
volatile uint8_t oldFoundHost=99; 
volatile uint8_t DMATranslate=0; // Don't Translate (CUBE 12 doesn't need, 8 needs);
uint8_t DMAOldChar=0; // Store old char in case we need to change DMATranslate;

USART_TypeDef* DMAUSART;

void setOtherCubesDMA(uint8_t address, uint8_t command)
{
	uint8_t x=0,y=0,z=0;
	uint8_t DMACount=0;
	uint8_t found=0;
	uint8_t cubeNum=0;
	if (hostCopyMode==1 && command==6) command=0; // Don't allow each to their own with copy mode
	if (hostCopyMode==2) command=0; // Don't allow each to their own or mirror with copy mode 2	
	if (command==6) 
	{
		// check what is tagged
	 for (x=0;x<layoutMaxX;x++)
	 {
		 for (y=0;y<layoutMaxY;y++)
		 {
 	 		 for (z=0;z<layoutMaxZ;z++)
			 {
				  if (cubeLayout[x][y][z]<20) // Don't count myself	or own cubes
					{
						if (cubeLayout[x][y][z]>10 && address==0)	
						{
							found=1;
						  //if (address!=0) address=99; // don't set them all
						  cubeNum=cubeLayout[x][y][z]-10;
						  if (!isOwnCube(cubeNum)) DMACommands[cubeNum]=6;
							else DMACommands[cubeNum]=1;
						}
						if (cubeLayout[x][y][z]==address+10 && address!=0)	
						{
							found=1;
						  //if (address!=0) address=99; // don't set them all
						  cubeNum=cubeLayout[x][y][z]-10;
						  if (!isOwnCube(cubeNum)) DMACommands[cubeNum]=6;
							else DMACommands[cubeNum]=1;
						}						
					}						
			 }
		 }
	  }
	  if (found==1)
	  {
		 curTotalX=totalX; 
		 curTotalY=totalY;
		 curTotalZ=totalZ;		
     curCentreX=curTotalX/2;
     curCentreY=curTotalY/2;
     curCentreZ=curTotalZ/2;			
		 extendCubes=1; // We have a extend cube
	  }
	  else
	  {
			// linked cubes are separate
		 	curTotalX=trimTotalX;
		  curTotalY=trimTotalY;
		  curTotalZ=trimTotalZ;		
      curCentreX=curTotalX/2;
      curCentreY=curTotalY/2;
      curCentreZ=curTotalZ/2;			
			extendCubes=0;
			command=0; //Normal Copy
	  }
	}
	else 
	{
		// linked cubes are separate
		curTotalX=trimTotalX; 
		curTotalY=trimTotalY;
		curTotalZ=trimTotalZ;		
    curCentreX=curTotalX/2;
    curCentreY=curTotalY/2;
    curCentreZ=curTotalZ/2;					
		extendCubes=0;
	}
	if (address==0)
	{
		for (DMACount=1;DMACount<10;DMACount++)
		{
			DMACommands[DMACount]=command;
		}
	}
	else 
	{ 
		if (address<10) DMACommands[address]=command;
	}
	totalPixels=curTotalX*curTotalY*curTotalZ; // total pixels
}

void getSize(uint8_t cube, uint8_t *tx, uint8_t *ty, uint8_t *tz)
{
	if (DMASizes[cube]==8)
	{
	   *tx=8;
     *ty=8;
     *tz=8;
	}	
	if (DMASizes[cube]==1)
	{
	   *tx=12;
     *ty=12;
     *tz=12;
	}		
	if (DMASizes[cube]==2)
	{
		 // 12 Cube split mode
	   *tx=36;
     *ty=12;
     *tz=4;
	}	
}

uint16_t getDMASize(uint8_t cube)
{
	if (DMASizes[cube]==8) 
	{
		if (ledArraySize>=1536) return 1536;
		else return ledArraySize;
	}
	if (DMASizes[cube]==1 || DMASizes[cube]==2) 
	{
		if (ledArraySize>=5184) return 5184;
		else return ledArraySize;		
	}
	return ledArraySize;
}

void calculateTotalSize()
{
	 uint8_t x=0,y=0,z=0;
	 uint8_t tx=0,ty=0,tz=0;
	 uint8_t Totx=0,Toty=0,Totz=0;
	 uint8_t nCubes=0;
	 for (x=0;x<layoutMaxX;x++)
	 {
		 for (y=0;y<layoutMaxY;y++)
		 {
 	 		 for (z=0;z<layoutMaxZ;z++)
			 {
				  if (cubeLayout[x][y][z]>9 && cubeLayout[x][y][z]<20) // If there is an active cube
					{
					  #ifdef doUSB
					  if (hasUSB) usb_printf("\r\nCUBE Layout %d,%d,%d = %d",x,y,z,cubeLayout[x][y][z]-10);
					  #endif
						getSize(cubeLayout[x][y][z]-10,&tx,&ty,&tz);		
						#ifdef doUSB
						if (hasUSB) usb_printf("\r\nCUBE %d Size = %d,%d,%d",cubeLayout[x][y][z]-10,tx,ty,tz);						
						#endif
						nCubes++;
						if (!isOwnCube(cubeLayout[x][y][z]-10))
						{
							if (x>0) Totx=Totx+tx;
							else 
							{
								if (tx>Totx) Totx=tx;	
							}
							if (y>0) Toty=Toty+ty;
							else
							{
								if (ty>Toty) Toty=ty;
							}
							if (z>0) Totz=Totz+tz;
							else
							{
								if (tz>Totz) Totz=tz;
							}
						}
					}
 				}		 
			}
		}
	 
   if (ledCubeType<2)
	 {
		 // Cube 8
		 if (Totx>8) {Toty=8;Totz=8;} // Can only extend one way
		 if (Toty>8) {Totx=8;Totz=8;}
		 if (Totz>8) {Totx=8;Toty=8;}
		 
		 if (Totx>24) Totx=24; // not enough memory for more on cube8
 		 if (Toty>24) Toty=24; // not enough memory for more on cube8
		 if (Totz>24) Totz=24; // not enough memory for more on cube8
	 }

 	 totalX=Totx;
	 totalY=Toty;
	 totalZ=Totz;
		
	 totalPixels=Totx*Toty*Totz;
	 totalMax=Totx;
   if (totalY>totalMax) totalMax=totalY;
   if (totalZ>totalMax) totalMax=totalZ;
	 totalMin=Totx;
   if (totalY<totalMin) totalMin=totalY;
   if (totalZ<totalMin) totalMin=totalZ;	 
	 numCubes=nCubes;
	 #ifdef doUSB
	 if (hasUSB) usb_printf("\r\nTotal %d cubes : %d,%d,%d",numCubes,totalX,totalY,totalZ);
	 #endif
}

void removeGrid(uint8_t DMAPointer,uint8_t reset)
{
	 uint8_t x,y,z;
	 uint8_t xx=99,yy=99,zz=99;
	 //if (hasUSB) usb_printf("\r\nRemove Cube %d",DMAPointer);
	 for (x=0;x<layoutMaxX;x++)
	 {
		 for (y=0;y<layoutMaxY;y++)
		 {
 	 		 for (z=0;z<layoutMaxZ;z++)
			 {
				  if (cubeLayout[x][y][z]==DMAPointer+10)
					{
						xx=x;
						yy=y;
						zz=z;
					}
			 }
		 }
	 }
	 if (xx!=99)
	 {
		 // found it
		 cubeLayout[xx][yy][zz]=DMAPointer;
		 if(reset)
		 {
				updateBounds();
				gCleanup();
		 }
	 }
}

void updateGrid(uint8_t DMAPointer)
{
	 uint8_t x,y,z;
	 uint8_t xx=99,yy=99,zz=99;
	 for (x=0;x<layoutMaxX;x++)
	 {
		 for (y=0;y<layoutMaxY;y++)
		 {
 	 		 for (z=0;z<layoutMaxZ;z++)
			 {
				  if (cubeLayout[x][y][z]==DMAPointer || cubeLayout[x][y][z]==DMAPointer+10) // Incase already think its here
					{
						xx=x;
						yy=y;
						zz=z;
						#ifdef doUSB
						if (hasUSB) usb_printf("\r\nAdd Cube %d to Layout %d,%d,%d",DMAPointer,x,y,z);
						#endif
					}
			 }
		 }
	 }
	 if (xx!=99)
	 {
		 // found it
		 cubeLayout[xx][yy][zz]=DMAPointer+10;
		 gCleanup();
	 }
	 else
	 {
		 #ifdef doUSB
		 if(hasUSB) usb_printf("\r\nNo Spot for CUBE %d",DMAPointer);
		 #endif
		 DMAAddresses[DMAPoint]=0;
	 }
}

void populateDMAGrid(uint8_t populate)
{
	//	cubeLayoutuint8_t cubeLayout[5][2][2];
	 uint8_t x,y,z;
	 uint8_t q=2; // prevent compiler warnings
	 // Initalize
	 for (x=0;x<layoutMaxX;x++)
	 {
		 for (y=0;y<layoutMaxY;y++)
		 {
 	 		 for (z=0;z<layoutMaxZ;z++)
			 {
				  cubeLayout[x][y][z]=99;
			 }
		 }
	 }
	 if (populate) // Only populate if we don't read from file
	 {
			cubeLayout[0][0][0]=10;  // Host. Add 10 for Available
			cubeLayout[1][0][0]=1;
			cubeLayout[2][0][0]=2;
		  if (ledCubeType>1)
			{
				// CUBE12
				cubeLayout[q+1][0][0]=3;
				cubeLayout[q+2][0][0]=4;
				cubeLayout[q+3][0][0]=5;
			}
			else
			{
				cubeLayout[0][1][0]=3;
				cubeLayout[1][1][0]=4;
				cubeLayout[2][1][0]=5;				
			}
	 }
}

void stopDMATransfer()
{
	Usart_Send_Data(USART1,'E');
	Usart_Send_Data(USART1,'1');
	Usart_Send_Data(USART1,'0');
	Usart_Send_Data(USART1,'E');
}

void startSendDMA()
{
	runDMATransfer();
}

void checkDMA()
{
	if (sendDMA && inSendDMA==0) runDMATransfer();
	if (inSendDMA==98) inSendDMA=0; // Safe to start again
}

void SlaveIRQ(char c)
{
	uint8_t oldInDMA=0;
	oldInDMA=inDMA;
	if (inDMA==0)
	{
		 if (getDMA)
		 {	
			   if(c=='C'||c=='X') inDMA=1; // C for Cube12, X for translate Cube8 Host
			   if(c=='X') DMAOldChar=1; // Store for later
			   else DMAOldChar=0;
		 }
	}
	if (inDMA==1)
	{
		 if (getDMA && c=='Z') inDMA=2;
	}
	if (inDMA==3)
	{
		 if (getDMA && c>=48 && c<=57) // 0-9 for DMA Command 
		 {
				DMACommands[DMAAddress]=c-48;
				if (showLED==3) showLED=1;
			  foundHost++;
			  if (foundHost>254) foundHost=1;
			  if (DMAOldChar==0) DMATranslate=0;
			  else 
				{
					DMATranslate=1;
					DMASize=1536; // We have a CUBE8 Host
				}
				DMA_Channel_Start();
		 }
	}
	if (inDMA==2)
	{
		if (getDMA && c==DMAAddress+48) inDMA=3;
	}
	if (inDMA==0)
	{
		if (getDMA && c=='D') inDMA=4;
	}
	if (inDMA==4)
	{
		 if (getDMA && c=='Z') inDMA=5;
	}	
	if (inDMA==6)
	{
		if (getDMA && c=='0')
		{
			// show blank 
			showLED=3;
			inDMA=0;
		}
	}		
	if (inDMA==5)
	{
		if (getDMA && c==DMAAddress+48) inDMA=6;
	}
	if (c=='M') inDMA=7;  // Regardless of if we think we are inside DMA or not
	if (inDMA==8)
	{
		if (c=='0')
		{
				// Host/Master has arrived
			if (Cube_MODE!=6)
			{
				Cube_MODE = 6;																
				setAll(BLACK);																	
				resetGraphics();
			}
			else
			{
				// Send again - Just in case you forgot me !
				startDMA();
			}
			inDMA=0;
		}
	}
	if (inDMA==7)
	{
		if (c=='0') inDMA=8;
	}
	
	if (inDMA==0)
	{
		if (getDMA && c=='E') inDMA=9;
	}
	if (inDMA==11)
	{
		if (getDMA && c=='0')
		{
			// Stop DMA
			// Host/Master has gone
			Cube_MODE = OldCube_MODE;																
			setAll(BLACK);																	
			resetGraphics();
			inDMA=0;
			foundHost=0;
		}
	}	
	if (inDMA==10)
	{
		if (getDMA && c=='0')
		{
			inDMA=11;
		}
	}
	if (inDMA==9)
	{
		if (getDMA && c==DMAAddress+48) inDMA=10;
	}
	if (oldInDMA==inDMA) inDMA=0; // need to be consecutive
}

void USARTIRQ(void)
{
	  char c;
	  uint8_t counter=1;
	  uint8_t free=0;

	  if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
		{
			c = USART_ReceiveData(USART1);
		  //if (hasUSB) usb_printf("%c",c); //CHANGE
			if (DMAAddress==0)  // HOST/MASTER
			{
				if (sendDMA==3)
				{
					sendDMA=1;
					#ifdef doUSB
					if (hasUSB) usb_printf("\r\nCUBE %d SIZE REMOTE = %d",DMAPoint,c-48);
					#endif
					DMASizes[DMAPoint]=c-48;
					updateBounds();
					startSendDMA();				
				}
				if (sendDMA==2)
				{
					sendDMA=3;
					DMAPoint=c-48;
					DMAAddresses[DMAPoint]=DMAPoint;
					updateGrid(DMAPoint);
				}
				if (c=='A' && getDMA) sendDMA=2;// Start DMA wait for address
				if (stopDMAReq==1)
				{
					while (counter<10)
					{
						if (DMAAddresses[counter]==c-48)
						{
							DMAAddresses[counter]=0;
						}
						else
						{
							if (DMAAddresses[counter]!=0) free=1;
						}
						counter++;
					}
					removeGrid(c-48,1);
					stopDMAReq=0;
					if (free==0) 
					{
						sendDMA=0; // no addresses left to send so stop DMA
						#ifdef doUSB
						if (hasUSB) usb_printf("\r\nNo More DMA!!!");
						#endif
					}
				}
				if (c=='B' && getDMA) stopDMAReq=1;// Stop DMA wait for address
				//if (sendDMA==oldSendDMA && stopDMAReq==oldStopDMAReq) {sendDMA=0;stopDMAReq=0;} // Need to be consecutive
				//oldSendDMA=sendDMA;
				//oldStopDMAReq=stopDMAReq;
			}
			else //SLAVE
			{
				//if (hasUSB && inDMA==0) usb_printf("%c",c);
				SlaveIRQ(c);
			}
		}
}

void DMAUSARTContentionDelay(uint8_t Add)
{
	uint32_t delayCounter=0;
	// Avoid all cubes transmitting together if more than one slave
	while(delayCounter<500000*Add) 
	{
		delayCounter++;
	}
}

void startDMA()
{
	//SLAVE
	DMAUSARTContentionDelay(DMAAddress);
	getDMA=1;
	Usart_Send_Data(DMAUSART,'A');
	Usart_Send_Data(DMAUSART,DMAAddress+48); // convert to ASCII
	if (ledCubeType<2) Usart_Send_Data(DMAUSART,mySize+48); // Cube Size is 8	
	if (ledCubeType==2) Usart_Send_Data(DMAUSART,1+48); // Cube Size is 12 Normal	
	if (ledCubeType==3) Usart_Send_Data(DMAUSART,2+48); // Cube Size is 12 Wide
}

void stopDMA()
{
	//SLAVE
	DMAUSARTContentionDelay(DMAAddress);
	getDMA=0;	
	Usart_Send_Data(DMAUSART,'B');
	Usart_Send_Data(DMAUSART,DMAAddress+48); // convert to ASCII	
}

void hostAnnounce()
{
	uint8_t DMACount=0;
	uint16_t timeout=0;
	// Wait for DMA to finish
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nAnnounce Host");
	#endif
	if (sendDMA)
	{		
		while(inSendDMA==1 && timeout<5000) 
		{
	     timeout++;
		}// wait to current DMA to end
		inSendDMA=99; // Don't Send Yet!!
	}
	// Initialise DMA Structure
	for (DMACount=1;DMACount<10;DMACount++) // Leave Host
	{
		  removeGrid(DMACount,0);
			DMAAddresses[DMACount]=0;
			DMACommands[DMACount]=0;
	}
	updateBounds();
  Usart_Send_Data(DMAUSART,'M');
	Usart_Send_Data(DMAUSART,'0');
	Usart_Send_Data(DMAUSART,'0');
	getDMA=1; // Host/Master Ready
	inSendDMA=98; // Don't start a DMA while sending chars
}

void setupDMA()
{
	uint8_t DMACount=0;
	DMAUSART=USART1; // Assume USART 1 AuraCube for both send and receive
	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nDMA Address : %d\r\n",DMAAddress);
	#endif
	if (hasLCD) lcd_println(99,1,"Cube Address %d",DMAAddress);
	powerOffSetup();  // set up IRQ to stop everything when the power drops
	if (DMAAddress==0)
		// Host DMA
	{
		USART1_Configuration();	
		USART1_HostDMAConfiguration();
		// Initialise DMA Structure
		for (DMACount=0;DMACount<10;DMACount++)
		{
			DMAAddresses[DMACount]=0;
			DMACommands[DMACount]=0;
		}
		if (mySize==8) DMASizes[0]=8; // HOST CUBE is 8 or 12
		if (mySize==12) 
		{
			if (ledCubeType==2)	DMASizes[0]=1; // 1 for normal CUBE12
			else DMASizes[0]=2; // 2 for WIDE CUBE12
		}
		updateBounds();
	}
	else
		// Slave DMA
	{
		delay(800);
		DMAUSARTContentionDelay(DMAAddress);
		if (bjCubeType==1)
		{
			DMAUSART=USART2;
			USART2_Configuration();	
			USART2_SlaveDMAConfiguration();
		}
		else
		{
			USART1_Configuration();	
			USART1_SlaveDMAConfiguration();
		}
	}
}
