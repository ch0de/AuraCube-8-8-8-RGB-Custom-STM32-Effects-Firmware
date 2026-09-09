#include "graphics.h"
#include "cubefile.h"	
#include "ff.h"	
#include "exfuns.h"
#include "CubeLink.h"
#include "usart.h"
#include "System.h"

uint8_t modeC=2;
uint8_t modeMood=10; // Greater than total so will reset to zero
uint8_t modeText=0;
uint8_t modeMenu=0;
uint8_t modeTime=0;

char graphicName[20]="";
uint8_t modeNext=0;
uint8_t modeWait=0;	
uint8_t modeWaitUp=0;
uint8_t msgWait=0;

uint8_t modeNextG=79; // next Graphic override
uint8_t mf_dirOpen=0;
uint8_t shuffleGraphics=0;
uint16_t numSDPlay=3;
uint16_t numSDFiles=0;

uint32_t counterNum=0;
uint32_t gCounter=0;
uint32_t gCounterMax=6000; // about 35 seconds for each graphic // CHANGE THIS BACK TO 6000

uint8_t graphicOrder[30]={90,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21}; // Graphic Order. Don't put >90 here in case no SD
uint8_t graphicOrderMax=21; // Number of Graphics in list above
uint8_t graphicsMax=22; // Maximum number of different graphics

uint8_t graphicOrderMaxTemp=0;
int8_t graphicOrderCounter=0; 
uint8_t welcome=1; // Do the text welcome ? // CHANGE THIS BACK TO 1 
uint8_t allowModeWait = 1;
uint8_t needToClearDMA=0;
	
uint32_t eCounter=0;
uint32_t eCounter2=0;
uint32_t eNum=0;
uint8_t eFadeout=0;
uint8_t eFadeoutDone=0;
uint8_t migrateDir=0;
uint16_t migrateThere=0;
uint16_t migrateNoMove=0;
uint8_t migrateFreeze=0;
uint8_t migrateSlide=0;
uint8_t migrateNum=0;
uint8_t migrateDelay=0;
uint16_t migrateSize=0;
uint8_t migrateTx=0;
uint8_t migrateTz=0;
uint8_t spinDir=0;
uint16_t plasmaColour=0;
uint8_t plasmaColourType=0;
uint8_t nextExplode=0;
uint8_t nextShape=0;
uint8_t nextShapeCounter=0;
int8_t nextShapeSpot=0;

uint8_t tailLength=1;

uint8_t lifeStartParents=3;
uint8_t lifeMarryRate=3; // 0-5
uint8_t lifeDeathRate=6; // 0-10
uint8_t lifeChildYears=20; // 1-40

uint8_t lifeGames=0; 
uint16_t conwayIteration=80;
uint8_t month=0; // Month of the year
uint8_t Xmas=0; // Xmas Mode or not. 1 = Xmas Tree Always. 2 = Xmas Tree During December

// Plasma Pointers to memory
char *ptrR=(char *)0x20000000;
char *ptrG=(char *)0x20000001;
char *ptrB=(char *)0x20000002;

uint8_t gLoops=0;     // For individual graphics to know how many loops they have done
int16_t gIteration=0; // For individual graphics to know where they are

uint32_t eInterval1=0;
uint32_t eInterval2=0;

int8_t rgb_dir=1,rot=1;
float phase=0; //myPI
float distort=1;
float narrow=0;
float speed=1;
uint8_t speedDelay=40;
uint8_t speedDirection=0;
uint8_t narrowDirection=0;

uint8_t R, G, B; 
uint8_t posIT=0;

float X=0,Y=0,Z=0;
float vXpos=0,vYpos=0,vZpos=0;
float vX=0,vY=0,vZ=0;
int vDir=0;
float vXpos2=0,vYpos2=0,vZpos2=0;
float vX2=0,vY2=0,vZ2=0;
int vDir2=0;
float vXpos3=0,vYpos3=0,vZpos3=0;
float vX3=0,vY3=0,vZ3=0;
int vDir3=0;

uint8_t pos;
uint8_t xPos;
uint8_t yPos;
uint8_t zPos;

swarm swarms[26];
uint8_t numSwarms=0;
uint8_t createSwarms=12;
uint8_t lastToMove=0;
uint8_t boxMode=0;
uint8_t numBoxes=0;
uint8_t spinBoxSize=0;

void randomTest()
{
   set(0,0,0,cube_random(255),cube_random(255),cube_random(255));
   delay(20*delaySpeed);	
}

void rasterOnce(uint8_t R, uint8_t G, uint8_t B, uint32_t speed)
{
	uint32_t x,y,z;
//	if (colour==0) colour=randomColour();
  for (z=0;z<curTotalZ;z++)
	{
		  for (y=0;y<curTotalY;y++)
		  {
			   for (x=0;x<curTotalX;x++)
			   {
					  set(x,y,z,R,G,B);
					  delay(speed);
					  clear(x,y,z);
				 }
		  }
	}			
}


void raster(uint8_t R, uint8_t G, uint8_t B, uint32_t speed, uint32_t loops)
{
  uint32_t x;
	setAll(BLACK);
	for (x=0;x<loops;x++)
	{
		rasterOnce(R,G,B, speed);
	}
}

void rasterChaseOnce(uint8_t R, uint8_t G, uint8_t B, uint32_t speed)
{
	uint32_t x,y,z,xx,yy,zz;
//	if (colour==0) colour=randomColour();
  for (z=0;z<curTotalZ;z++)
	{
		  for (y=0;y<curTotalY;y++)
		  {
			   for (x=0;x<curTotalX;x++)
			   {
					  zz=z;
					  if(y%2==0) xx=x;
						else xx=(curTotalX-1)-x;
					  //if(y%2==0) yy=y;
						//else yy=7-y;
					 	if(z%2==0) yy=y;
						else yy=(curTotalY-1)-y;
					  set(xx,yy,zz,R,G,B);
					  delay(speed);
					  clear(xx,yy,zz);
				 }
		  }
	}	
/*  for (z=7;z>-1;z--)
	{
		  for (y=0;y<8;y++)
		  {
			   for (x=0;x<8;x++)
			   {
					  zz=z;
					  if(y%2==0) xx=x;
						else xx=7-x;
					  //if(y%2==0) yy=y;
						//else yy=7-y;
					 	if(z%2==0) yy=y;
						else yy=7-y;
					  set(xx,yy,zz,R,G,B);
					  delay(speed);
					  clear(xx,yy,zz);
				 }
		  }
	}*/				
}

void rasterChaseDimOnce(uint8_t R, uint8_t G, uint8_t B, uint32_t speed, uint8_t brightness, uint8_t loops)
{
	uint32_t x,y,z,xx,yy,zz;
	uint8_t oR,oG,oB;
  uint8_t oldR,oldG,oldB;
//	float calcBright = loops/(0.1*brightness);
//	if (colour==0) colour=randomColour();
  for (z=0;z<curTotalZ;z++)
	{
		  for (y=0;y<curTotalY;y++)
		  {
			   for (x=0;x<curTotalX;x++)
			   {
					  zz=z;
					  if(y%2==0) xx=x;
						else xx=(curTotalX-1)-x;
					  //if(y%2==0) yy=y;
						//else yy=7-y;
					 	if(z%2==0) yy=y;
						else yy=(curTotalY-1)-y;
					 	getRGB(xx,yy,zz,&oldR,&oldG,&oldB);
					  set(xx,yy,zz,R,G,B);
					  delay(speed);
					  if(oldR==0 && oldG==0 && oldB==0) {
							 //dimmer(R,G,B,&oR,&oG,&oB,0.98);
						}
						else
						{
							 //dimmer(R,G,B,&oR,&oG,&oB,calcBright);
							hueToRGB(60,brightness,&oR,&oG,&oB);
						}
					  set(xx,yy,zz,oR,oG,oB);
				 }
		  }
	}	
}

void invertColours(void)
{
  // invert cube
  uint8_t x, y, z;
	uint8_t r, g, b;
	for (x=0;x<curTotalX;x++)
	{
	  for (y=0;y<curTotalY;y++)
	  {
	    for (z=0;z<curTotalX;z++)
		  {
				getRGB(x,y,z,&r,&g,&b);
				set(x,y,z,255-r,255-g,255-b);
			}
		}
	}
}


void rasterChase(uint8_t R, uint8_t G, uint8_t B, uint32_t speed, uint32_t loops)
{
  uint32_t x;
	setAll(BLACK);
	for (x=0;x<loops;x++)
	{
		rasterChaseDimOnce(R,G,B, speed,x,loops);
	}
}

void setName(char* str)
{
	// set the Graphic Name
	strncpy(graphicName,str,20);
	if (hasLCD) lcd_println(0,1,"%s",graphicName);
  #ifdef doUSB
  if (hasUSB) usb_printf("\r\n%s",graphicName);
  #endif
}

void setup(void) 
{
  pos=0;
  xPos = 0;
  yPos = 0;
  zPos = 0;

  fill_colour_wheel();
}

void cycleCubeColours()
{

     if (pos==0) setAll(RED);
     if (pos==1) setAll(GREEN);
     if (pos==2) setAll(BLUE);
     if (pos==3) setAll(ORANGE);
     if (pos==4) setAll(PINK);
     if (pos==5) setAll(0x00, 0x44, 0xA);   
     pos++;
     if (pos>5) pos=0;
		 delay(70*delaySpeed);
}

void stepThroughLEDs()
{
    setAll(BLACK);
    set(xPos, yPos, zPos, WHITE);
    xPos++;
    if (xPos>7) 
      {
        xPos=0;
        yPos++;
        if (yPos >7)
        {
          yPos=0;
          zPos++;
          if (zPos >7)
          {
            zPos=0; 
          }
        }
      }
		 delay(5*delaySpeed);
}


/*void fadeWhite()
{
    if (y==0){
    i--;
    if (i<1) y=1;
    }   
    else
    {
    i++;
    if (i>254) y=0;
    }
    setAll(i, i, i);
		delay(delaySpeed);
}
*/

void randomPastel(void)
{  
	uint8_t w;
	uint8_t x,y,z;
	if (gCounter == 1)
	{
		 setName("Random Pastel");
		 if (curTotalY>=12 && (month==12 || Xmas==1))
		 {
			 setName("Christmas Tree");
	 		 if (Xmas) modeWait=1; // Remain on the Christmas Tree Forever
		 }
		 setOtherCubesDMA(0,6); // Each cube to its own!
		 setAll(BLACK);
		 if (numCubes==1 || ledCubeType>=2) // AURA12 can do effects over multiple cubes
		 {
			setEffect("fadein",0,0,0.5);
			setEffect("slide",15,0,0); 
		  eFadeout=1;
		 }
	}
	if (eFadeoutDone==1)
	{
		 setEffect("slide",15,0,0);	
	   setEffect("fadeout",0,0,0.5);			
	}
	if (curTotalY>=12 && (month==12 || Xmas==1)) Box(5,0,5,6,2,6,102,51,0,1,0); // Christmas Tree Trunk
	for (w=0;w<5;w++)
	{
		x=cube_random(curTotalX);
		y=cube_random(curTotalY);
		z=cube_random(curTotalZ);
		if (curTotalY>=12 && (month==12 || Xmas==1)) // need at least 2 8x8 cubes on top of each other or a cube12
		{
			// Christmas Tree
			if (((x<=4 && x>=y/2-1) || (x>=5 && x<=7-(y/2-1))) && ((z<=4 && z>=(y/2-1)) || (z>=5 && z<=7-(y/2-1))))
			{
				if (y+3<curTotalY)
				{
					set(x+2,y+3,z+2, cube_random(100), cube_random(100), cube_random(100)); // a little duller
					delay(2);
					if (delaySpeed >15) delay(delaySpeed);
				}
			}			
			if ((x!=0&x!=11)&&(y==3||y==4)&&(z!=0&z!=11))
			{
				if (y+3<curTotalY)
				{
					set(x,y,z, cube_random(100), cube_random(100), cube_random(100)); // a little duller
					delay(2);
					if (delaySpeed >15) delay(delaySpeed);
				}				
			}
		}
		else
		{
				set(x,y,z, cube_random(100), cube_random(100), cube_random(100)); // a little duller
				delay(2);
				if (delaySpeed >15) delay(delaySpeed);		
		}
	}
}

double map(double in, double inMin, double inMax, double outMin, double outMax){
    double out;
    out = (in-inMin)/(inMax-inMin)*(outMax-outMin) + outMin;
    return out;
}

void sineWavePlus()
{
	 uint8_t x,y;
	 float phase2;
	 if (gCounter==1)
	 {
		  setName("Sine Wave 2+");
		  setOtherCubesDMA(0,6);
		  setAll(BLACK);
		  //setEffect("slide",12,0,0);
		  //eFadeout=3;
		  rgb_dir=0;
		  narrow=0;
		  narrowDirection=0;
		  phase=0;
		  phase2=0;
	 }
   for(x = 0; x < curTotalY; x++)
	 {
		for(y = 0; y < curTotalZ; y++)
		 {
			X = myMap(x, 0, 11, -2, 2);
			Y = myMap(y, 0, 11, -2, 2);
			Z = sin(phase - myPI/curTotalX + sqrt(X*X + Y*Y));
			Z = myRound(myMap(Z,-1,1,0,(curTotalX-1)));
			
			//CLEAR PREVIOUS Z
		  set(Z,x,y,BLACK);
			
			Z = sin(phase + sqrt(X*X + Y*Y));
			Z = myRound(myMap(Z,-1,1,0,(curTotalX-1)));
			set(Z,x,y,ORANGE); 	
       
			phase2=phase+(float)myPI;
			X = myMap(x, 0, 11, -2, 2);
			Y = myMap(y, 0, 11, -2, 2);
			Z = sin(phase2 - myPI/curTotalX + sqrt(X*X + Y*Y));
			Z = myRound(myMap(Z,-1,1,0,(curTotalX-1)));
			
			//CLEAR PREVIOUS Z
		  set(Z,x,y,BLACK);
			Z = sin(phase2 + sqrt(X*X + Y*Y));
			Z = myRound(myMap(Z,-1,1,0,(curTotalX-1)));
			
			R=0;G=0;B=255;
			set(Z,x,y,BLUE); 	
		  }
		}
	phase += myPI/curTotalX;
	if(phase-(float)myPI<(float)0.04 && phase-(float)myPI>-(float)0.04) delay(10*delaySpeed);
	if(phase >= 2*myPI){
		phase -= 2*myPI;
		delay(10*delaySpeed);
	}
	delay(5*delaySpeed);
}

void sineWaveTwo()
{
	 uint8_t x,y;
	 float phase2;
	 if (gCounter==1)
	 {
		  setName("Sine Wave 2");
		  setOtherCubesDMA(0,6);
		  setAll(BLACK);
		  //setEffect("slide",12,0,0);
		  //eFadeout=3;
		  rgb_dir=0;
		  narrow=0;
		  narrowDirection=0;
		  phase=0;
		  phase2=0;
	 }
   for(x = 0; x < curTotalY; x++)
	 {
		for(y = 0; y < curTotalZ; y++)
		 {
			X = myMap(x, 0, 7, -2, 2);
			Y = myMap(y, 0, 7, -2, 2);
			Z = sin(phase - myPI/curTotalX + sqrt(X*X + Y*Y));
			Z = myRound(myMap(Z,-1,1,0,(curTotalX-1)));
			
			//CLEAR PREVIOUS Z
		  set(Z,x,y,BLACK);
			
			Z = sin(phase + sqrt(X*X + Y*Y));
			Z = myRound(myMap(Z,-1,1,0,(curTotalX-1)));
			set(Z,x,y,PINK); 	
       
			phase2=phase+(float)myPI;
			X = myMap(x, 0, 7, -2, 2);
			Y = myMap(y, 0, 7, -2, 2);
			Z = sin(phase2 - myPI/curTotalX + sqrt(X*X + Y*Y));
			Z = myRound(myMap(Z,-1,1,0,(curTotalX-1)));
			
			//CLEAR PREVIOUS Z
		  set(Z,x,y,BLACK);
			Z = sin(phase2 + sqrt(X*X + Y*Y));
			Z = myRound(myMap(Z,-1,1,0,(curTotalX-1)));
			
			R=0;G=0;B=255;
			set(Z,x,y,YELLOW); 	
		  }
		}
	phase += myPI/curTotalX;
	if(phase-(float)myPI<(float)0.04 && phase-(float)myPI>-(float)0.04) delay(10*delaySpeed);
	if(phase >= 2*myPI){
		phase -= 2*myPI;
		delay(10*delaySpeed);
	}
	delay(5*delaySpeed);
}

void sineWave()
{
	 uint8_t x,y;
	 uint8_t R,G,B;	
	 if (gCounter==1)
	 {
		  setName("Sine Wave");
		  setOtherCubesDMA(0,2);
		 	//setOtherCubesDMA(0,6); // Too Slow
		  setAll(BLACK);
		  setEffect("slide",12,0,0);
		  eFadeout=3;
		  rgb_dir=0;
		  narrow=0;
		  narrowDirection=0;
		  phase=0;
	 }
   for(x = 0; x < curTotalX; x++){
		for(y = 0; y < curTotalZ; y++){
			X = myMap(x, 0, curTotalX-1, -2, 2);
			Y = myMap(y, 0, curTotalZ-1, -2, 2);
			Z = sin(phase - myPI/curTotalX + sqrt(X*X + Y*Y));
			Z = myRound(myMap(Z,-1,1,0,(curTotalX-1)));
			
			//CLEAR PREVIOUS Z
		  set(x,Z,y,BLACK);
			//usb_printf("\r\nBLACK = %d,%f,%d",x,Z,y);
			Z = sin(phase + sqrt(X*X + Y*Y));
			Z = myRound(myMap(Z,-1,1,0,(curTotalX-1)));
			
			get_colour(colourPos+Z*10,&R,&G,&B);
			set(x,Z,y,R,G,B); //arrayP[ar(x, y, Z)] = colourWheel.get_colour(colourPos + Z*10);		
			//usb_printf("\r\nSET = %d,%f,%d",x,Z,y);
		  }
		}
  increment_colour_pos(2);
	phase += myPI/curTotalX;
  if(phase >= 2*myPI)
	{
		phase -= 2*myPI;
	}
	if (ledCubeType!=3)	delay(3.5*delaySpeed);
	else delay(1.5*delaySpeed); // Quicker for Wide Cube12
}

void spin()
{
		uint8_t x,y,z;
		uint8_t R,G,B;
		if (gCounter==1)
	  {
			 setName("Spin");
			 setOtherCubesDMA(0,6);
			 setAll(BLACK);
		   if (numCubes==1 || ledCubeType>=2) // AURA12 can do effects over multiple cubes
			 {
				setEffect("fadein",0,0,0);
				eFadeout=2;
			 }
			 spinDir=cube_random(2);
			 if (shared2!=255) spinDir=shared2; // If we have got a common random number to send to all the cubes
			 phase=0;
			 distort=1;
			 rgb_dir=0;
		}
	  for(x = 0; x < curTotalX; x++){
        for(y = 0; y < curTotalZ; y++){
            for(z = 0; z < curTotalY; z++){
							X = (x-curTotalX/(float)2.3)*mySin(phase+myMap(y, 0, curTotalY, 0, distort*(float)myPI/2));  //3.5
							Z = (z-curTotalZ/(float)2.3)*myCos(phase+myMap(y, 0, curTotalY, 0, distort*(float)myPI/2));
							if (spinDir==0)
							{
								if(myAbs(X-Z) < (float)0.7)
								{
									get_colour(colourPos + x*2 +y*3,&R,&G,&B);
									set(x,z,y,R,G,B); 
								}
								else
								{
									set(x,z,y,BLACK);
								}
							}
						if (spinDir==1)
						{
								if(myAbs(X-Z) < (float)0.7)
								{
									get_colour(colourPos + x*2 +y*3,&R,&G,&B);
									set((curTotalX-1)-x,z,(curTotalZ-1)-y,R,G,B); 	
								}
								else
								{
									set((curTotalX-1)-x,z,(curTotalZ-1)-y,BLACK);
								}				
						}	
					}
        }
    }

    increment_colour_pos(2);

    distort+=0.02*rgb_dir;
	
	if(distort > (float)1.5 || distort < (float)0.5){
		rgb_dir*= -1;
	}

	phase += myPI/(3*(curTotalX/2.3));
	
	if(phase >= 2*myPI){
		phase -= 2*myPI;
	}
	if (ledCubeType!=3) delay(2*(delaySpeed/numCubes));
	//else delay((delaySpeed/numCubes)/4); // Faster for Wide Cube12
}


void spiral(void)
{
  uint8_t z,i;
  uint8_t R,G,B;

	if (gCounter==1)
	{
		 setName("Spiral");
		 setOtherCubesDMA(0,2);
		 setAll(BLACK);
	   setEffect("flash",12,4,0);
		 setEffect("increase",0,0,0);
 	   narrow=0;
		 phase=0;
  }
	//Calculate frame
	setAll(BLACK);
	for(z = 0; z < curTotalY; z++){
		for(i = 0; i < 4; i++){
			Y = myCos(phase + myMap(z, 0, curTotalZ-1, 0, 2*myPI) + i*myPI/curTotalZ);
			X = mySin(phase + myMap(z, 0, curTotalX-1, 0, 2*myPI) + i*myPI/curTotalX);
			Y = myMap(Y, -1.1, 0.9, narrow, (float)curTotalZ-1-narrow);
			X = myMap(X, -1.1, 0.9, narrow, (float)curTotalX-1-narrow);
			get_colour(colourPos + 10*z,&R,&G,&B);
			set(X,z,Y,R,G,B);
		}
	}
	
	increment_colour_pos(2);

	phase += myPI/5*speed;
	if(phase >= 2*myPI){
		phase -= 2*myPI;}
	delay(5*delaySpeed);
}

void spiral2(void)
{
    uint8_t z,i;
		uint8_t R,G,B;
	//Calculate frame
	setAll(BLACK);
	for(z = 0; z < curTotalY; z++){
		for(i = 0; i < 4; i++){
			Y = myCos(phase + myMap(z, 0, curTotalZ-1, 0, 2*myPI) + i*myPI/curTotalZ);
			X = mySin(phase + myMap(z, 0, curTotalX-1, 0, 2*myPI) + i*myPI/curTotalX);
			Y = myMap(Y, -1.1, 0.9, narrow, (float)curTotalZ-1-narrow);
			X = myMap(X, -1.1, 0.9, narrow, (float)curTotalX-1-narrow);
			get_colour(colourPos + 10*z,&R,&G,&B);
			set(X,z,Y,R,G,B);
		}
	}	
	increment_colour_pos(2);
}

void runSpiral2()
{
	 uint8_t ran=0;
	 if (gCounter==1)
	 {
		 	setName("Spiral");
		  setOtherCubesDMA(0,1);
		 	setAll(BLACK);
		  speedDelay = 3.5*delaySpeed;
		  slideDirX=0;
      ran=cube_random(2);
		 	if (shared2!=255) ran=shared2; // If we have got a common random number to send to all the cubes
      if (ran==1) slideDirY=3;
		  else slideDirY=1;
	    slideDirZ=0;
	    setEffect("slide",30,0,1);
		  eFadeout=1;
	 }
	 if (eFadeoutDone==1)
	 {
		  slideDirX=0;
      ran=cube_random(2);
      if (ran==1) slideDirY=4;
		  else slideDirY=2;
	    slideDirZ=0;
	    setEffect("slide",30,0,1);
	 }
	 /*  if (counter%30 == 0)
	 {
		  if (dir==0)
			{
         narrow--;
		     if (narrow <0)
			   {
				    narrow=0;
					  dir=1;
			   }
			 }
			else
			{
				narrow++;
				if (narrow >2)
				{
					  narrow=2;
					  dir=0;
				}
			}
	 }
*/	 
/*	 if (counter%270 == 0)
	 {
		  if (rot==0) rot=1;
		  else rot =0;
	 }
*/
	 if (speedDirection==0) 
	 {
			if (eNum==0) speedDelay=speedDelay+(speedDelay/30);
		  //if (speedDelay > 60) speedDelay++;
		  //if (speedDelay > 90) speedDelay++;
		  //if (speedDelay > 120) speedDelay++;
	 }
	 else 
	 {
		  if (eNum==0) speedDelay=speedDelay-(speedDelay/30);
		  //if (speedDelay > 60) speedDelay--;
 		 // if (speedDelay > 90) speedDelay--;speedDelay--;
		 // if (speedDelay > 120) speedDelay--;speedDelay--;speedDelay--;		 
	 }
	 if (speedDelay < 40) speedDirection = 0;
	 if (speedDelay >120)
	 {
		  delay(2.5*delaySpeed);
		  speedDirection=1;
		  if (rot==0) rot=1;
		  else rot =0;
		  if (narrowDirection==0)
			{
		     narrow--;
				 if (narrow<0)
				 {
					 narrow=0;
					 narrowDirection=1;
				 }
			 }
			else
			{
		     narrow++;
				 if (narrow>2)
				 {
					 narrow=2;
					 narrowDirection=0;
				 }				
			}
	 }
	 spiral2();
  if (rot == 0)
	{
		phase -= myPI/5*speed;
	     if(phase <=0){
		   phase += 2*myPI;}
	}
	else
	{
			phase += myPI/5*speed;
			if(phase >= 2*myPI){
				phase -= 2*myPI;}
	}
	if (ledCubeType!=3) delay(speedDelay);				
	else delay(3*speedDelay); // slower for Wide Cube12
}

void textWelcome()
{
	if (gCounter==1)
	{
		setName("Welcome");
		modeWait=1;
		initScrollText(0);
		beginText();
		setAll(BLACK);
	}
	animateText();
}

void textCube()
{
	if (gCounter==1)
	{
		setName("Welcome");
		setOtherCubesDMA(0,6); // Each cube to its own!
		modeWait=1;
		initScrollText(0);
		beginText7();
		setAll(BLACK);
	}
	animateText7();
}

void textWelcomeScroll()
{
	if (gCounter==1)
	{
		setName("Welcome");
 	  setOtherCubesDMA(0,6); // Each cube to its own!
		initScrollText(0);
		modeWait=1;
		posIT=0;
		setAll(BLACK);
	}
  scrollText(posIT);
	posIT++;
  delay(15*delaySpeed);
}

void doDateTime()
{
	if (gCounter==1)
	{
		setName("Date Time");
 	  setOtherCubesDMA(0,6); // Each cube to its own!
		initScrollText(0);
		modeWait=1;
		posIT=0;
		setAll(BLACK);
	}
	if (!modeWaitUp)
	{
		scrollDateTime(posIT);
		posIT++;
		delay(15*delaySpeed);
		if (!hasNTP && !hasDS3231) modeWaitUp=1; // No Time to display
	}
}

void doTime(void)
{
	if (gCounter==1)
	{
		if (modeC!=89) setName("Time");
 	  setOtherCubesDMA(0,6); // Each cube to its own!
		initScrollText(0);
		modeWait=1;
		posIT=0;
		setAll(BLACK);
	}
	if (!modeWaitUp)
	{
		scrollTime(posIT);
		posIT++;
		delay(15*delaySpeed);
		if (!hasNTP && !hasDS3231) modeWaitUp=1; // No Time to display
	}
}

int getRound(float myFloat)
{
  double integral;
  float fraction = (float)modf(myFloat, &integral);
 
  if (fraction >= (float)0.5)
    integral += 1;
  if (fraction <= -0.5)
    integral -= 1;
 
  return (int)integral;
}

void accelerateVelocity(int gravity, float aX, float aY, float aZ)
{
//	float newVY = vY+aY/100;
	if (gravity==1)
	{ 
	//	if ((newVY > 0 && vY > 0) || (newVY <0 && vY < 0))
	//	{
		   vY += aY/100;
//		}
/*		else
		{
			 if (vY >0)
			 {
				  vY = 0.1;
			 }
			 else
			 {
				 vY = -0.1;
			 } */
		if (vYpos < 1 && vDir == 0)
		{
			vY = 0;
			vDir = 1;
		}
		if (vYpos > (curTotalY-1) && vDir == 1)
		{
			vY = 0;
			vDir = 0;			
		}
	/*	if (vY>0) 
		{
			vY += aY/100;
		}
		else
		{
      vY -= aY/100;
		}*/			
	}
	else 
	{
	 vX += aX/100;
	 vY += aY/100;
	 vZ += aZ/100;
	}
}
		
void moveVelocity(int bounces, int accelerate, int gravity, float aX, float aY, float aZ, int size, int sizeZ)
{
  float newX = vXpos + vX/100;
  float newY = vYpos + vY/100;
  float newZ = vZpos + vZ/100;
	
	if (newX+size > curTotalX || newX-size <0)
	{
		  if (bounces==1) {vX=-vX;}
			else {vX=0;}
	}
	if (newY+size > curTotalY || newY-size <0)
	{
		  if (bounces==1) {vY=-vY;}
			else {vY=0;}
	}
	if (newZ+size > curTotalZ || newZ-size <0 || newZ-sizeZ <0)
	{
				if (bounces==1) {vZ=-vZ;}
				else {vZ=0;}
	}
	vXpos+=vX/100;
	vYpos+=vY/100;
	vZpos+=vZ/100;
	if (accelerate==1) {accelerateVelocity(gravity,aX,aY,aZ);}
}

void moveVelocity2(int bounces, int accelerate, int gravity, float aX, float aY, float aZ, int size)
{
  float newX = vXpos2 + vX2/100;
  float newY = vYpos2 + vY2/100;
  float newZ = vZpos2 + vZ2/100;
	
	if (newX+size > curTotalX || newX-size <0)
	{
		  if (bounces==1) {vX2=-vX2;}
			else {vX2=0;}
	}
	if (newY+size > curTotalY || newY-size <0)
	{
		  if (bounces==1) {vY2=-vY2;}
			else {vY2=0;}
	}
	if (newZ+size > curTotalZ || newZ-size <0)
	{
		  if (bounces==1) {vZ2=-vZ2;}
			else {vZ2=0;}
	}
	vXpos2+=vX2/100;
	vYpos2+=vY2/100;
	vZpos2+=vZ2/100;
	if (accelerate==1) {accelerateVelocity(gravity,aX,aY,aZ);}
}

void moveVelocity3(int bounces, int accelerate, int gravity, float aX, float aY, float aZ, int size)
{
  float newX = vXpos3 + vX3/100;
  float newY = vYpos3 + vY3/100;
  float newZ = vZpos3 + vZ3/100;
	
	if (newX+size > curTotalX || newX-size <0)
	{
		  if (bounces==1) {vX3=-vX3;}
			else {vX3=0;}
	}
	if (newY+size > curTotalY || newY-size <0)
	{
		  if (bounces==1) {vY3=-vY3;}
			else {vY3=0;}
	}
	if (newZ+size > curTotalZ || newZ-size <0)
	{
		  if (bounces==1) {vZ3=-vZ3;}
			else {vZ3=0;}
	}
	vXpos3+=vX3/100;
	vYpos3+=vY3/100;
	vZpos3+=vZ3/100;
	if (accelerate==1) {accelerateVelocity(gravity,aX,aY,aZ);}
}

void bounceSphere()
{
	if (gCounter==1)
	{
		setName("Bounce Sphere");
		setOtherCubesDMA(0,6);
		setAll(BLACK);
		vXpos = curCentreX;
		vYpos = curCentreY;
		vZpos = curCentreZ;
		vX = 3;
		vY = 8;
		vZ = 4;
		/*if (ledCubeType==3) // Faster
		{
			 vX=vX*(float)1.5;
			 vY=vY*(float)1.5;
			 vZ=vZ*(float)1.5;
		}*/
 	 	if (numCubes==1 || ledCubeType>=2) eFadeout=1; // AURA12 can do effects over multiple cubes
	}
	if (eFadeoutDone==1)
	{
			//setEffect("fadeout",0,0,0.05);
			setEffect("dissolve",0,0,0);
	}
  //setAll(BLACK); 
  sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),2,BLACK);
  sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),4,BLACK);
  
	moveVelocity(1,0,0,3,3,3,0,4);
  
	sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),2,R,150,B);
  sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),4,R,1,B);
  if (ledCubeType<2) delay(delaySpeed);
	else delay(delaySpeed/2);
}

void bounceSphere2()
{
	if (gCounter==1)
	{
		setName("Bounce Sphere2");
		setOtherCubesDMA(0,6);
		setAll(BLACK);
		vXpos = curCentreX-1;
		vYpos = curCentreY;
		vZpos = curCentreZ;
		vX = 3; 
		vY = 8;
		vZ = 4;
		
		vXpos2 = 2;
		vYpos2 = curCentreY;
		vZpos2 = 2;
		vX2 = curTotalX/8+1; //4
		vY2 = 7;
		vZ2 = curTotalZ/8+1; //4;
		
		if (curTotalX>12)
		{
			vXpos3 = curCentreX+4;
			vYpos3 = curCentreY;
			vZpos3 = 2;
			vX3 = curTotalX/8+1; //4
			vY3 = 9;
			vZ3 = curTotalZ/8+1; //4;			
		}
/*		if (ledCubeType==3) // Faster
		{
			 vX=vX*(float)2;
			 vY=vY*(float)2;
			 vZ=vZ*(float)2;
			 vX2=vX2*(float)2;
			 vY2=vY2*(float)2;
			 vZ2=vZ2*(float)2;			
		}	*/	
		//setEffect("showboth",0,0,0);
		//setEffect("stripe",20,0,0);
	}
	//setAll(BLACK);
	sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),2,BLACK);
	sphere(getRound(vXpos2),getRound(vYpos2),getRound(vZpos2),2,BLACK);
	if (totalX>12) sphere(getRound(vXpos3),getRound(vYpos3),getRound(vZpos3),2,BLACK);

	moveVelocity(1,0,0,3,3,3,2,0);
	moveVelocity2(1,0,0,3,3,3,2);
	if (totalX>12) moveVelocity3(1,0,0,3,3,3,2);
	
	sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),2,255,100,0);
	sphere(getRound(vXpos2),getRound(vYpos2),getRound(vZpos2),2,255,1,255);
	if (totalX>12) sphere(getRound(vXpos3),getRound(vYpos3),getRound(vZpos3),2,YELLOW);
  if (ledCubeType<2) delay(delaySpeed);
	else delay(delaySpeed/2);
}

void drawSpheres2()
{
  if (gCounter==1)
	{
		setName("Draw Spheres2");
		setOtherCubesDMA(0,6);
		setAll(BLACK);
		if (numCubes==1 || ledCubeType>=2) setEffect("undissolve",0,0,0); // AURA12 can do effects over multiple cubes 
		modeWait=1;
		vXpos = curCentreX;
		vYpos = curCentreY;
		vZpos = curCentreZ;
	  B=cube_random(255);
	  R=cube_random(255);
		if (shared2!=255) 
		{
			// If we have got a common random number to send to all the cubes
			B=sharedA;
			R=sharedB;
		}
	}
  //delay(120*delaySpeed);
  sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),2,R,200,B);
  delay(120*delaySpeed);
	sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),4,R,0,B);
  delay(120*delaySpeed);
	delay(10*delaySpeed);
}

void drawSpheres()
{
	// Only runs through once
	B=cube_random(255);
	R=cube_random(255);
	if (shared2!=255) 
	{
		// If we have got a common random number to send to all the cubes
		B=sharedA;
		R=sharedB;
	}
  modeWait=1;
	setOtherCubesDMA(0,6);
	setAll(BLACK);
	vXpos = curCentreX;
	vYpos = curCentreY;
	vZpos = curCentreZ;
	setName("Draw Spheres");
	sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),1,RED);
	if (modeNext==0 || modeNext==4) delay(80*delaySpeed); // Only delay if we are still on this graphic
	sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),2,R,200,B);
	if (modeNext==0 || modeNext==4) delay(80*delaySpeed); // Only delay if we are still on this graphic
	sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),3,80,160,40);
	if (modeNext==0 || modeNext==4) delay(80*delaySpeed); // Only delay if we are still on this graphic
	sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),4,R,0,B);
	if (modeNext==0 || modeNext==4) delay(80*delaySpeed); // Only delay if we are still on this graphic
	setAll(BLACK);	
	sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),3,80,160,40);
	sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),2,R,200,B);
	sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),1,RED);
	if (modeNext==0 || modeNext==4) delay(80*delaySpeed); // Only delay if we are still on this graphic
	setAll(BLACK);
	sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),2,R,200,B);
	sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),1,RED);
	if (modeNext==0 || modeNext==4) delay(80*delaySpeed); // Only delay if we are still on this graphic
	setAll(BLACK);
	sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),1,RED);
	if (modeNext==0 || modeNext==4) delay(80*delaySpeed); // Only delay if we are still on this graphic
	setAll(BLACK);
	if (numCubes==1 || ledCubeType>=2) // AURA12 can do effects over multiple cubes 	
	{
		setEffect("undissolve",0,0,0);
		//setEffect("fadein",0,0,0.5);
  }
	else
	{
		sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),1,RED);
		if (modeNext==0 || modeNext==4) delay(80*delaySpeed); // Only delay if we are still on this graphic
		sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),2,R,200,B);
		if (modeNext==0 || modeNext==4) delay(80*delaySpeed); // Only delay if we are still on this graphic
		sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),3,80,160,40);
		if (modeNext==0 || modeNext==4) delay(80*delaySpeed); // Only delay if we are still on this graphic
		sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),4,R,0,B);
		if (modeNext==0 || modeNext==4) delay(80*delaySpeed); // Only delay if we are still on this graphic
		setAll(BLACK);			
	}
	sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),2,R,200,B);
	sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),4,R,0,B);
	while(eNum!=0 && (modeNext==0 || modeNext==4))
	{
		// wait for undissolve
		delay(100);
	}
	if (modeNext==0)
	{
    modeWaitUp = 1;
	  delay(50);
	}
}

int swarmClear(uint8_t x, uint8_t y, uint8_t z)
{ 	
	uint8_t i, clear=1;
  for (i=0;i<numSwarms;i++)
  {
     if (swarms[i].hX==x && swarms[i].hY==y && swarms[i].hZ==z) clear=0;
		 if (swarms[i].tX==x && swarms[i].tY==y && swarms[i].tZ==z) clear=0;
	}
	return(clear);
}

int moveTowardsQueen(uint8_t n)
{
	uint8_t notOk=1;
	int8_t x2,y2,z2=0;
	int8_t dX,dY,dZ=0;
	uint8_t qX,qY,qZ=0;
	uint8_t ranDir2=0;
	qX=swarms[0].hX;
	qY=swarms[0].hY;
	qZ=swarms[0].hZ;
	x2=swarms[n].hX;
	y2=swarms[n].hY;
	z2=swarms[n].hZ;
	dX=swarms[n].dirX;
	dY=swarms[n].dirY;
	dZ=swarms[n].dirZ;
	ranDir2=cube_random(10);
	// normally continue on path if closer to queen
	if (ranDir2<6)
	{	
	   if ((qX>x2 && dX !=-1) || (qX<x2 && dX != 1)) // ok for X 
		 {
				if ((qY>y2 && dY !=-1) || (qY<y2 && dY != 1)) // ok for Y 			 
				{
					  if ((qZ>z2 && dZ !=-1) || (qZ<z2 && dZ != 1)) // ok for Z 
						{
							notOk=0;
						}
					}
			}
	}
	if (notOk)
	{
		 // change direction but still no ok
		 swarms[n].dirX=swarms[0].o2DirX;
		 swarms[n].dirY=swarms[0].o2DirY;
		 swarms[n].dirZ=swarms[0].o2DirZ;		
		 notOk=1;
	}
	return(notOk);
}

void swarmMove(uint8_t n)
{
	uint8_t notOk=1;
  uint32_t counter=0;
	uint8_t ranDir=0;
	int8_t ran=0;
	int8_t x,y,z=0;
	int8_t x1,y1,z1=0;
	x=swarms[n].hX;
	y=swarms[n].hY;
	z=swarms[n].hZ;
	swarms[n].o2DirX=swarms[n].oDirX;
	swarms[n].o2DirY=swarms[n].oDirY;
	swarms[n].o2DirZ=swarms[n].oDirZ;	
	swarms[n].oDirX=swarms[n].dirX;
	swarms[n].oDirY=swarms[n].dirY;
	swarms[n].oDirZ=swarms[n].dirZ;	
	if (n>0)
	{
	 // move towards the queen
		notOk=moveTowardsQueen(n);
		if (!(notOk))
		{
				x1=x+swarms[n].dirX;
				y1=y+swarms[n].dirY;
				z1=z+swarms[n].dirZ;
		}
	}
	while (notOk)
	{
		if (notOk)
		{
			ranDir=cube_random(10);
			if (ranDir<9)
			{
				x1=x+swarms[n].dirX;
				y1=y+swarms[n].dirY;
				z1=z+swarms[n].dirZ;
				if (!(x1<0 || x1>=curTotalX || y1<0 || y1>=curTotalY || z1<0 || z1>=curTotalZ || (swarms[n].dirX==0 && swarms[n].dirY==0 && swarms[n].dirZ ==0))) notOk=0;
			}
		}
    if (notOk)
		{
			ran=cube_random(3)-1;
			x1=x+ran;
			swarms[n].dirX=ran;
			ran=cube_random(3)-1;
			y1=y+ran;
			swarms[n].dirY=ran;
			ran=cube_random(3)-1;
			z1=z+ran;
			swarms[n].dirZ=ran;
			if (x1>=0 && x1<curTotalX && y1>=0 && y1<curTotalY && z1>=0 && z1<curTotalZ)
			{
			  if (!(x1==x && y1==y && z1==z)) //we dont want to find ourself
				{
					 notOk=0;
				}
			}
		}			
	  counter++;
		if (counter>3000) {notOk=0;}//setAll(ORANGE);delay(1000);setAll(BLACK);} // FIX. not sure why this is happening
	}
	swarms[n].hX=x1;
	swarms[n].hY=y1;
	swarms[n].hZ=z1;	
	swarms[n].tX=x;
	swarms[n].tY=y;
	swarms[n].tZ=z;	
}

void createTail(uint8_t n, uint8_t x, uint8_t y, uint8_t z, uint8_t *error)
{
	uint8_t notOk=1;
//	uint8_t dir=0;
	int8_t ran=0;
	int8_t x1,y1,z1=0;
	*error=0;
	while (notOk)
	{
		ran=cube_random(3)-1;
	  x1=x+ran;
		ran=cube_random(3)-1;
	  y1=y+ran;
		ran=cube_random(3)-1;
	  z1=z+ran;
		if (x1>=0 && x1<curTotalX && y1>=0 && y1<curTotalY && z1>=0 && z1<curTotalZ)
		{
			  if (!(x1==x && y1==y && z1==z)) //we dont want to find ourself
				{
					 notOk=0;
				}
				if (!(swarmClear(x1,y1,z1)))
				{
					 notOk=0;
				}
		}			
	}
	swarms[n].tX=x1;
	swarms[n].tY=y1;
	swarms[n].tZ=z1;	
}

void swarmSetup()
{
	int8_t ran;
	uint8_t notOk=1; 
	uint8_t error=0;
	uint8_t x,y,z,i=0;
	numSwarms=0;
	ran=cube_random(3);
	if (shared2!=255) ran=shared3; // If we have got a common random number to send to all the cubes
	R=0;G=0;B=0;
	if (ran==0) R=255;
	if (ran==1) G=255;
	if (ran==2) B=255;
	ran=cube_random(curTotalX*2);
	createSwarms=10+ran;
	for (i=0;i<createSwarms;i++)
	{
		notOk=1;
		while (notOk)
	  {
			 x=cube_random(curTotalX);
			 y=cube_random(curTotalY);
		   z=cube_random(curTotalZ);
			 if ((swarmClear(x,y,z)))
			 {
					swarms[i].hX=x;
					swarms[i].hY=y;
					swarms[i].hZ=z;
					createTail(i,x,y,z,&error);
				  notOk=0;
			 }				 
		}
		swarms[i].dirX=0;
		swarms[i].dirY=0;
		swarms[i].dirZ=0;
		swarms[i].R=R;
		swarms[i].G=G;
		swarms[i].B=B;
		numSwarms++;
	}
}

void drawSwarms()
{
 	uint8_t i;
  for (i=1;i<numSwarms;i++)
  {
		 set(swarms[i].hX,swarms[i].hY,swarms[i].hZ,swarms[i].R,swarms[i].G,swarms[i].B);
		 dimmer(swarms[i].R,swarms[i].G,swarms[i].B,&R,&G,&B,0.87);
		 set(swarms[i].tX,swarms[i].tY,swarms[i].tZ,R,G,B);
  }	
	set(swarms[0].hX,swarms[0].hY,swarms[0].hZ,WHITE);
	set(swarms[0].tX,swarms[0].tY,swarms[0].tZ,25,25,25);			 
}


void doSwarm()
{
	uint8_t i;
	if (gCounter==1)
  {
		setName("Swarm");
		setOtherCubesDMA(0,6);
		setAll(BLACK);
  	if (numCubes==1 || ledCubeType>=2) // AURA12 can do effects over multiple cubes 	
		{
			setEffect("fadein",0,0,0);
			//setEffect("fadetrail",0,0,0.95); 
			eFadeout=2;
		}
		swarmSetup();
	}
	setAll(BLACK);
	drawSwarms();
  for (i=1;i<numSwarms;i++)
  {
		swarmMove(i);
	}
	swarmMove(0);
	delay(delaySpeed*15);
}

const uint8_t around[29]={
0,1,2,3,4,5,6,7,15,23,31,39,47,55,63,62,61,60,59,58,57,56,48,40,32,24,16,8,0
};

void boxes()
{
	uint8_t beforeBoxes=5;
	uint8_t pos,posDir,clocked,oldTailLength,dBox;
	uint8_t offsetX; // Get Graphic in Centre
	uint8_t offsetY;
	int16_t secondPos;
	//,ranX,ranZ;
	//int8_t x,y,z;
	//uint16_t counter,ranCount;
	if (gCounter==1)
	{
    // set up
	  gIteration=0;
    modeWait=0;
		setOtherCubesDMA(0,6);
		if (numCubes==1 || ledCubeType>=2) setEffect("undissolve",0,0,0); // AURA12 can do effects over multiple cubes
		eFadeout=4;
		setName("Boxes");
		setAll(BLACK);
    randomColour(&Color_red,&Color_green,&Color_blue);	
	  if (shared2!=255) 
		{
			// If we have got a common random number to send to all the cubes			
			Color_red=sharedA;
			Color_green=sharedB;
			Color_blue=sharedC;
		}
		gLoops=0;
		clocked=0;
		spinDir=0;
		tailLength=totalMin;
	}
	offsetX=curTotalX-totalMin; // Get Graphic in Centre
	offsetY=curTotalY-totalMin;	
	if (offsetX>1) offsetX=offsetX/2;	
	if (offsetY>1) offsetY=offsetY/2;
	clocked=0;
	//usb_printf("\r\neNum=%d,gIteration=%d,gLoops=%d,clocked=%d",eNum,gIteration,gLoops,clocked);
	if (gIteration>(totalMin-1)*4-1 && spinDir==0) {gIteration=0;clocked=1;}
	if (gIteration<0 && spinDir==1) {gIteration=0;clocked=1;}
	if (clocked==1 && gLoops==(beforeBoxes+1)) {gLoops++;}
	if (clocked==1 && gLoops<(beforeBoxes+1))
	{
		clocked=0;
		randomColour(&Color_red,&Color_green,&Color_blue);	
		if (shared2!=255) 
		{
			// If we have got a common random number to send to all the cubes			
			Color_red=sharedA;
			Color_green=sharedB;
			Color_blue=sharedC;
		}
		oldTailLength=tailLength;
		tailLength=cube_random(3*totalMin-2)+1;
		if (eNum!=0 && tailLength<totalMin) tailLength=totalMin; // At least keep to one whole side when dissolve / undissolve
		if (tailLength<oldTailLength)
		{
			// reverse the direction to eat up the bigger tail
	    if (spinDir==0) spinDir=1;
		  else spinDir=0;
		}
		if (gLoops==beforeBoxes) tailLength=4*totalMin-5;
		if (shared2!=255) spinDir=shared2; // If we have got a common random number to send to all the cubes		
		if (spinDir==1) gIteration=(totalMin-1)*4;
		if (eNum==0) gLoops++; // Don't increase if in dissolve or undissolve
	}
	
	//usb_printf("gLoops %d\r\n",gLoops);
	if (gLoops>=(beforeBoxes+2)) 
	{
	   dBox=gLoops-(beforeBoxes+2);
		 setAll(BLACK);
		 Box(0+offsetX+dBox,0+offsetY+dBox,0+dBox,curTotalX-1-offsetX-dBox,curTotalY-1-offsetY-dBox,curTotalZ-1-dBox,Color_red,Color_green,Color_blue,0,0); 
		 gLoops++;
		 delay(20*delaySpeed);
		 if (dBox>totalMin-2) 
		 {
			 gLoops=0;
			 gIteration=0;
			 tailLength=totalMin;
			 randomColour(&Color_red,&Color_green,&Color_blue);	
		   if (shared2!=255) 
		   {
			   // If we have got a common random number to send to all the cubes			
			   Color_red=sharedA;
			   Color_green=sharedB;
			   Color_blue=sharedC;
		   }
			 setAll(BLACK);
		 }
	}
	else
	{
	  if (gIteration<totalMin) {posDir=0;pos=gIteration;}
	  if (gIteration>=totalMin && gIteration<(totalMin-1)*2) {posDir=1;pos=gIteration-totalMin+1;}
  	if (gIteration>=(totalMin-1)*2 && gIteration<(totalMin-1)*3) {posDir=2;pos=gIteration-(totalMin-1)*2;}
  	if (gIteration>=(totalMin-1)*3) {posDir=3;pos=gIteration-(totalMin-1)*3;}
    //usb_printf("\r\ngIteration = %d, posDir = %d, pos = %d\r\n",gIteration,posDir,pos);  
    //usb_printf("\r\ncurTotalX=%d,pos=%d,offsetX=%d",curTotalX,pos,offsetX);			
	  if (posDir==0) line(pos+offsetX,0+offsetY,0,pos+offsetX,0+offsetY,totalZ-1,Color_red,Color_green,Color_blue);
	  if (posDir==1) line(curTotalX-1-offsetX,pos+offsetY,0,curTotalX-1-offsetX,pos+offsetY,totalZ-1,Color_red,Color_green,Color_blue);
    if (posDir==2) line(curTotalX-1-pos-offsetX,curTotalY-1-offsetY,0,curTotalX-1-pos-offsetX,curTotalY-1-offsetY,totalZ-1,Color_red,Color_green,Color_blue);
	  if (posDir==3) line(0+offsetX,curTotalY-1-pos-offsetY,0,0+offsetX,curTotalY-1-pos-offsetY,totalZ-1,Color_red,Color_green,Color_blue);
	  // Second position
	  if (spinDir==0)
	  {
  		secondPos=gIteration-tailLength;
  		if (secondPos<0) secondPos=(totalMin-1)*4+secondPos;
  	}
	  else
	  {
		  secondPos=gIteration+tailLength;
		  if (secondPos>(totalMin-1)*4) secondPos=secondPos-((totalMin-1)*4);
	  }
	  if (secondPos<totalMin) {posDir=0;pos=secondPos;}
	  if (secondPos>=totalMin && secondPos<(totalMin-1)*2) {posDir=1;pos=secondPos-totalMin+1;}
	  if (secondPos>=(totalMin-1)*2 && secondPos<(totalMin-1)*3) {posDir=2;pos=secondPos-(totalMin-1)*2;}
  	if (secondPos>=(totalMin-1)*3) {posDir=3;pos=secondPos-(totalMin-1)*3;}
    //usb_printf("\r\nsecondPos = %d, posDir = %d, pos = %d\r\n",secondPos,posDir,pos);
		
	  if (posDir==0) line(pos+offsetX,0+offsetY,0,pos+offsetX,0+offsetY,totalZ-1,BLACK);
	  if (posDir==1) line(curTotalX-1-offsetX,pos+offsetY,0,curTotalX-1-offsetX,pos+offsetY,totalZ-1,BLACK);
    if (posDir==2) line(curTotalX-1-pos-offsetX,curTotalY-1-offsetY,0,curTotalX-1-pos-offsetX,curTotalY-1-offsetY,totalZ-1,BLACK);
	  if (posDir==3) line(0+offsetX,curTotalY-1-pos-offsetY,0,0+offsetX,curTotalY-1-pos-offsetY,totalZ-1,BLACK); 
	
	  if (spinDir==0) gIteration++;
	  else gIteration--;
		if (tailLength==(4*totalMin-5)) delay(4*delaySpeed);
		else 
		{
			if (totalMin<=8) delay((tailLength/6+3)*delaySpeed);
			else delay((tailLength/12+2)*delaySpeed);
		}
  }
}

uint8_t isBoxClear(int8_t x1,int8_t y1,int8_t z1,int8_t x2,int8_t y2,int8_t z2)
{
	// Are the spots in this box free ?
	uint8_t x,y,z;
	//usb_printf("\r\nIs Box Clear from %d,%d,%d to %d,%d,%d",x1,y1,z1,x2,y2,z2);
	if (x1<0 || x2<0 || y1<0 || y2<0 || z1<0 || z2<0) return (0);
  for(x = x1; x <= x2; x++)
	{
	  for(y = y1; y <= y2; y++)
		{
			for(z = z1; z <= z2; z++)
			{
				if (!isClear(x,y,z)) return (0);
			}
		}
	}
	return (1);
}

void drawShapes(uint8_t sNum, uint8_t one)
{
	uint8_t i;
	if (one!=255)
	{
		  i=one;
	    //usb_printf("\r\nDraw box %d,%d,%d to %d,%d,%d",swarms[i].hX,swarms[i].hY,swarms[i].hZ,swarms[i].dirX,swarms[i].dirY,swarms[i].dirZ);					
	    filledBox(swarms[i].hX,swarms[i].hY,swarms[i].hZ,swarms[i].dirX,swarms[i].dirY,swarms[i].dirZ,swarms[i].R,swarms[i].G,swarms[i].B);				
	}
	else
	{
    for(i = 0; i < sNum; i++)
  	{
	    //usb_printf("\r\nDraw box %d,%d,%d to %d,%d,%d",swarms[i].hX,swarms[i].hY,swarms[i].hZ,swarms[i].dirX,swarms[i].dirY,swarms[i].dirZ);					
	    filledBox(swarms[i].hX,swarms[i].hY,swarms[i].hZ,swarms[i].dirX,swarms[i].dirY,swarms[i].dirZ,swarms[i].R,swarms[i].G,swarms[i].B);		
	  }
	}
}


uint8_t randomShape(uint8_t sNum, uint8_t square, uint8_t fixedSize, uint8_t align)
{
	uint8_t x,y,z;  // Mid Points
	int8_t x1,y1,z1; // Start Points
	int8_t x2,y2,z2; // End Points
	uint8_t odd; // Odd base
	uint8_t xSize,ySize,zSize; // Size of Square / Rectangle
	uint8_t placed=0;
	uint8_t attempts1=0;
  uint8_t attempts2=0;
  uint8_t col=0;
	uint16_t colPos=0;
	
	col=cube_random(16);
  //usb_printf("\r\nCreate Shapes %d",sNum);
	while (attempts1<20 && !placed)
	{
		// Only have 5 attempts to draw a shape
	  xSize=cube_random(curTotalX/2)+2; // Min 2 // Max half
		if (square==1)
		{
			if (fixedSize==0)
			{
				ySize=xSize;
				zSize=xSize;
			}
			else
			{
				xSize=fixedSize;
				ySize=fixedSize;
				zSize=fixedSize;
			}
		}
		else
		{
			ySize=cube_random(curTotalY/2)+2; // Min 2 // Max half
			zSize=cube_random(curTotalZ/2)+2; // Min 2 // Max half
		}
		attempts2=0;
		//usb_printf("\r\nSizes : %d,%d,%d",xSize,ySize,zSize);
	  while (attempts2<20 && !placed)
	  {
		  // Only have 20 attempts to draw a shape of this size
			if (align==0)
			{
		    x=cube_random(curTotalX);
		    y=cube_random(curTotalY);
		    z=cube_random(curTotalZ);
			}
			else
			{
				// got to make sure only in certain positions
				x=cube_random(curTotalX/xSize)*xSize+((xSize-1)/2);
				y=cube_random(curTotalY/ySize)*ySize+((ySize-1)/2);
				z=cube_random(curTotalZ/zSize)*zSize+((zSize-1)/2);
			}
			odd=(xSize-1)/2;
			if ((xSize+2)%2==0)
			{
				// Even
				x1=x-odd;
				x2=x+odd+1;
			}
			else
			{
				x1=x-odd;
				x2=x+odd;
			}
			odd=(ySize-1)/2;			
			if ((ySize+2)%2==0)
			{
				// Even
				y1=y-odd;
				y2=y+odd+1;
			}
			else
			{
				y1=y-odd;
				y2=y+odd;
			}
			odd=(zSize-1)/2;
			if ((zSize+2)%2==0)
			{
				// Even
				z1=z-odd;
				z2=z+odd+1;
			}
			else
			{
				z1=z-odd;
				z2=z+odd;
			}
			if (isOnCube(x1,y1,z1) && isOnCube(x2,y2,z2))
			{
				// Both points are on the cube. Is it free ?
				if (isBoxClear(x1,y1,z1,x2,y2,z2))
				{	
          // Store the Shape
					swarms[sNum].hX=x1;
					swarms[sNum].hY=y1;
					swarms[sNum].hZ=z1;
					swarms[sNum].dirX=x2;
					swarms[sNum].dirY=y2;
					swarms[sNum].dirZ=z2;
			    swarms[sNum].o2DirX=0;
			    swarms[sNum].o2DirY=0;
			    swarms[sNum].o2DirZ=0;
					colPos=col+sNum*70;
					if (colPos>512) colPos=colPos-512;
					if (colPos>512) colPos=colPos-512;
					if (colPos>512) colPos=colPos-512;
          get_colour((colPos),&R,&G,&B);		
					swarms[sNum].R=R;
					swarms[sNum].G=G;
					swarms[sNum].B=B;
					drawShapes(0,sNum);
					placed=1;
				}
			}
			attempts2++;
	  }
		attempts1++;
	}
	return (placed);
}

uint8_t moveShapeNum(uint8_t sNum, uint8_t dirX, uint8_t dirY, uint8_t dirZ)
{
	uint8_t attempts=0;
	uint8_t moved=0;
	int8_t x1,y1,z1;
	int8_t x2,y2,z2;
	uint8_t tryX,tryY,tryZ;
	uint8_t plane;
	uint8_t boxClear;
	//usb_printf("\r\nAttempt move box %d",sNum);
	if ((dirX!=0 || dirY!=0 || dirZ!=0) && dirX!=3) attempts=19;
	while (attempts<20 && !moved)
	{
  	 // Only have 20 attempts to move this shape
	   boxClear=1;
     if ((dirX!=0 || dirY!=0 || dirZ!=0) && dirX!=3)
		 {
			 tryX=dirX;
			 tryY=dirY;
			 tryZ=dirZ;
		 }
		 else
		 {
       tryX=0;
		   tryY=0;
		   tryZ=0;
       plane=cube_random(3); //x, y or z
		   if (plane==0) tryX=cube_random(2)+1;
		   if (plane==1) tryY=cube_random(2)+1;
		   if (plane==2) tryZ=cube_random(2)+1;
		 }
		 x1=swarms[sNum].hX;
		 x2=swarms[sNum].dirX;
		 y1=swarms[sNum].hY;
		 y2=swarms[sNum].dirY;
		 z1=swarms[sNum].hZ;
		 z2=swarms[sNum].dirZ;
		 if (tryX==1) {x1=swarms[sNum].hX-1;x2=swarms[sNum].dirX-1;if(!isBoxClear(x1,y1,z1,x1,y2,z2)) boxClear=0;}
		 if (tryX==2) {x1=swarms[sNum].hX+1;x2=swarms[sNum].dirX+1;if(!isBoxClear(x2,y1,z1,x2,y2,z2)) boxClear=0;}
		 if (tryY==1) {y1=swarms[sNum].hY-1;y2=swarms[sNum].dirY-1;if(!isBoxClear(x1,y1,z1,x2,y1,z2)) boxClear=0;}
		 if (tryY==2) {y1=swarms[sNum].hY+1;y2=swarms[sNum].dirY+1;if(!isBoxClear(x1,y2,z1,x2,y2,z2)) boxClear=0;}
		 if (tryZ==1) {z1=swarms[sNum].hZ-1;z2=swarms[sNum].dirZ-1;if(!isBoxClear(x1,y1,z1,x2,y2,z1)) boxClear=0;}
		 if (tryZ==2) {z1=swarms[sNum].hZ+1;z2=swarms[sNum].dirZ+1;if(!isBoxClear(x1,y1,z2,x2,y2,z2)) boxClear=0;}
		 //usb_printf("\r\n  try %d,%d,%d",tryX,tryY,tryZ);
		 //usb_printf("\r\n    Move to %d,%d,%d to %d,%d,%d",x1,y1,z1,x2,y2,z2);
		 if (isOnCube(x1,y1,z1) && isOnCube(x2,y2,z2))
		 {
				// Both points are on the cube. Is it free ?
		   if (boxClear)
		   {
				 //usb_printf("\r\n    PLACED!!");
			   // Move the shape here
			   // clear behind
			   if (tryX==1) filledBox(swarms[sNum].dirX,swarms[sNum].hY,swarms[sNum].hZ,swarms[sNum].dirX,swarms[sNum].dirY,swarms[sNum].dirZ,BLACK);
			   if (tryX==2) filledBox(swarms[sNum].hX,swarms[sNum].hY,swarms[sNum].hZ,swarms[sNum].hX,swarms[sNum].dirY,swarms[sNum].dirZ,BLACK);
			   if (tryY==1) filledBox(swarms[sNum].hX,swarms[sNum].dirY,swarms[sNum].hZ,swarms[sNum].dirX,swarms[sNum].dirY,swarms[sNum].dirZ,BLACK);
			   if (tryY==2) filledBox(swarms[sNum].hX,swarms[sNum].hY,swarms[sNum].hZ,swarms[sNum].dirX,swarms[sNum].hY,swarms[sNum].dirZ,BLACK);
			   if (tryZ==1) filledBox(swarms[sNum].hX,swarms[sNum].hY,swarms[sNum].dirZ,swarms[sNum].dirX,swarms[sNum].dirY,swarms[sNum].dirZ,BLACK);
			   if (tryZ==2) filledBox(swarms[sNum].hX,swarms[sNum].hY,swarms[sNum].hZ,swarms[sNum].dirX,swarms[sNum].dirY,swarms[sNum].hZ,BLACK);
         // Store the Shape
			   swarms[sNum].hX=x1;
			   swarms[sNum].hY=y1;
			   swarms[sNum].hZ=z1;
			   swarms[sNum].dirX=x2;
			   swarms[sNum].dirY=y2;
			   swarms[sNum].dirZ=z2;
			   swarms[sNum].o2DirX=tryX;
			   swarms[sNum].o2DirY=tryY;
			   swarms[sNum].o2DirZ=tryZ;
         swarms[sNum].tX=1;
			   moved=1;
				 lastToMove=sNum;
			   drawShapes(0,sNum);
			 }
		 }
     attempts++;		 
	}
	return (moved);
}

void resetMoved(uint8_t sNum)
{
	uint8_t i;

	for (i=0;i<sNum;i++)
	{
		swarms[i].tX=0;
	}
}

void moveShapes(uint8_t sNum, uint8_t numToMove, uint8_t continuousMove)
{
	uint8_t i,j;
	uint8_t moving=0;
	uint8_t moved;
	uint8_t attempts;
	uint8_t notMoved;
	
	resetMoved(sNum);
	//check what is in motion
  //usb_printf("\r\nMove Shapes %d",numToMove);
	for (i=0;i<sNum;i++)
	{
		if (swarms[i].o2DirX!=0 || swarms[i].o2DirY!=0 || swarms[i].o2DirZ!=0)
		{
			moving=1;
		}
	}
	if (moving)
	{
		// Continue move
    for (j=0;j<sNum;j++)
	  {
			attempts=0;
			notMoved=1;
			while(notMoved && attempts<100)
			{
				i=cube_random(sNum);
		    if (swarms[i].tX==0 && (swarms[i].o2DirX!=0 || swarms[i].o2DirY!=0 || swarms[i].o2DirZ!=0))
		    {
          moved=moveShapeNum(i,swarms[i].o2DirX,swarms[i].o2DirY,swarms[i].o2DirZ);
				  if (!moved)
				  {
					  // couldn't move it				
					  swarms[i].o2DirX=0;
					  swarms[i].o2DirY=0;
					  swarms[i].o2DirZ=0;
					  if (continuousMove==1) swarms[i].o2DirX=3; // Start with a random dir again
					}
					notMoved=0;
				}
				swarms[i].tX=1;
				attempts++;
			}
		}				
	}
	if (!moving)
	{
		// start a new move
		if (numToMove==0) numToMove=sNum;
  	for (j=0;j<numToMove;j++)
		{		
			attempts=0;
			notMoved=1;
			while(notMoved && attempts<100)
			{
				i=cube_random(sNum);
				if(numToMove==1&&i==lastToMove&&attempts<50) 
				{
					attempts++;
					continue; // Try not to move the same one twice
				}
				// Don't move the same one twice
				if (swarms[i].tX==0)
				{		
					moveShapeNum(i,0,0,0);
					notMoved=0;
				}
				swarms[i].tX=1;
				attempts++;
			}
		}
	}
}

void mBoxes()
{
	// Move Boxes / Shapes. Reuse Swarm array to save memory
	uint8_t i;
	uint32_t j;

	uint8_t square=0;
	uint8_t fixedSize=0;
	uint8_t align=0;
	uint8_t continuousMove=0;
	uint8_t numToMove=0;
	uint8_t notDrawn=0;
	
	if(boxMode==99)
	{
		notDrawn=99;
		if (totalMin<12) boxMode=cube_random(4);
		else boxMode=cube_random(7);
	}
	
	if (boxMode==0)
	{
		square=1;
		align=1;
		numToMove=1;
		continuousMove=0;
		fixedSize=totalMin/2;
		if(numBoxes==0) numBoxes=cube_random(6)+2; // 2 to 7 boxes for 8 spots
	}
	if (boxMode==1)
	{
		square=1;
		align=1;
		numToMove=0;
		continuousMove=1;
		fixedSize=2;
		j=totalPixels/64-1; // leave some holes
		if(j>26) j=26; // Max 26 boxes
		if(j<10) j=10; // Min 10 boxes
		if(numBoxes==0) numBoxes=j;
	}
	if (boxMode==2)
	{
		square=0;
		align=0;
		numToMove=0;
		continuousMove=0;
		fixedSize=0;
		if(numBoxes==0) numBoxes=cube_random(7)+5; // At least 5 boxes
	}	
	if (boxMode==3)
	{
		square=1;
		align=1;
		numToMove=3;
		continuousMove=0;
		fixedSize=3;
		if(numBoxes==0)
		{
		  if (totalMin<12) numBoxes=5;
		  else numBoxes=26;
		}
	}
	if (boxMode==4)
	{
		square=1;
		align=1;
		numToMove=0;
		continuousMove=1;
		fixedSize=4;
    if(numBoxes==0) numBoxes=12;
	}
	if (boxMode==5)
	{
		square=1;
		align=1;
		numToMove=1;
		continuousMove=0;
		fixedSize=4;
    if(numBoxes==0) numBoxes=15;
	}	
	if (boxMode==6)
	{
		square=1;
		align=1;
		numToMove=4;
		continuousMove=0;
		fixedSize=4;
    if(numBoxes==0) numBoxes=20;
	}
	if (notDrawn==99)
	{
		 for (i=0;i<numBoxes;i++)
		 {
    		if (randomShape(numSwarms,square,fixedSize,align)) numSwarms++;
		 }
		 numBoxes=numSwarms;
		 #ifdef doUSB
		 if (hasUSB) usb_printf("\r\nCreated %d Boxes",numSwarms);
		 #endif
	}
	if (gCounter==1)
	{
		 setName("Move Boxes");
		 if (DMAAddress==0 && hostReAnnounce==4)
		 {
				hostAnnounce();
		 }
     modeWait=0;
		 setOtherCubesDMA(0,6);
		 setAll(BLACK);
		 if (numCubes==1 || ledCubeType>=2) setEffect("blackwhite",0,0,0); // AURA12 can do effects over multiple cubes
		 if (numCubes>1) eFadeout=2; // Fadeout
		 else eFadeout=3; // Slide
		 numSwarms=0;
		 lastToMove=0;
		 boxMode=99;
		 numBoxes=0;
	}	
	//if (gCounter==20) {eNum=0;setLED=0;showLED=0;}// Remove effect
	if (notDrawn!=99 && boxMode!=99) moveShapes(numSwarms,numToMove,continuousMove);
  delay(15*delaySpeed);
}

void spinBoxExp(int8_t spot)
{
   setAll(BLACK);
   filledBox(0,totalMin-spinBoxSize,totalMin-spinBoxSize,spinBoxSize-1,totalMin-1,totalMin-1,Color_red,Color_green,Color_blue);
   filledBox(0,totalMin-spinBoxSize*2-spot,totalMin-spinBoxSize,spinBoxSize-1,totalMin-spinBoxSize-1-spot,totalMin-1,Color_red,Color_green,Color_blue);
   filledBox(spinBoxSize+spot,totalMin-spinBoxSize,totalMin-spinBoxSize,spinBoxSize*2-1+spot,totalMin-1,totalMin-1,Color_red,Color_green,Color_blue);
   filledBox(spinBoxSize+spot,totalMin-spinBoxSize*2-spot,totalMin-spinBoxSize,spinBoxSize*2-1+spot,totalMin-spinBoxSize-1-spot,totalMin-1,Color_red,Color_green,Color_blue);	
   filledBox(0,totalMin-spinBoxSize,totalMin-spinBoxSize*2-spot,spinBoxSize-1,totalMin-1,totalMin-spinBoxSize-1-spot,Color_red,Color_green,Color_blue);
   filledBox(0,totalMin-spinBoxSize*2-spot,totalMin-spinBoxSize*2-spot,spinBoxSize-1,totalMin-spinBoxSize-1-spot,totalMin-spinBoxSize-1-spot,Color_red,Color_green,Color_blue);
   filledBox(spinBoxSize+spot,totalMin-spinBoxSize,totalMin-spinBoxSize*2-spot,spinBoxSize*2-1+spot,totalMin-1,totalMin-spinBoxSize-1-spot,Color_red,Color_green,Color_blue);
   filledBox(spinBoxSize+spot,totalMin-spinBoxSize*2-spot,totalMin-spinBoxSize*2-spot,spinBoxSize*2-1+spot,totalMin-spinBoxSize-1-spot,totalMin-spinBoxSize-1-spot,Color_red,Color_green,Color_blue);
}

void spinBoxMove(int8_t xMove,int8_t zMove,uint8_t skipDraw)
{
	 int8_t spot;
   spot=spinBoxSize*2;
   setAll(BLACK);
   filledBox(xMove,totalMin-spinBoxSize-zMove,totalMin-spinBoxSize,spinBoxSize-1+xMove,totalMin-1-zMove,totalMin-1,Color_red,Color_green,Color_blue);
   filledBox(0,totalMin-spinBoxSize*2-spot+xMove,totalMin-spinBoxSize-zMove,spinBoxSize-1,totalMin-spinBoxSize-1-spot+xMove,totalMin-1-zMove,Color_red,Color_green,Color_blue);
   if (skipDraw!=1) filledBox(spinBoxSize+spot,totalMin-spinBoxSize-xMove-zMove,totalMin-spinBoxSize,spinBoxSize*2-1+spot,totalMin-1-xMove-zMove,totalMin-1,Color_red,Color_green,Color_blue);
   filledBox(spinBoxSize+spot-xMove,totalMin-spinBoxSize*2-spot,totalMin-spinBoxSize-zMove,spinBoxSize*2-1+spot-xMove,totalMin-spinBoxSize-1-spot,totalMin-1-zMove,Color_red,Color_green,Color_blue);	
	
	 filledBox(xMove,totalMin-spinBoxSize,totalMin-spinBoxSize*2-spot+zMove,spinBoxSize-1+xMove,totalMin-1,totalMin-spinBoxSize-1-spot+zMove,Color_red,Color_green,Color_blue);
   if (skipDraw!=1) filledBox(0,totalMin-spinBoxSize*2-spot+xMove+zMove,totalMin-spinBoxSize*2-spot,spinBoxSize-1,totalMin-spinBoxSize-1-spot+xMove+zMove,totalMin-spinBoxSize-1-spot,Color_red,Color_green,Color_blue);
   filledBox(spinBoxSize+spot,totalMin-spinBoxSize-xMove,totalMin-spinBoxSize*2-spot+zMove,spinBoxSize*2-1+spot,totalMin-1-xMove,totalMin-spinBoxSize-1-spot+zMove,Color_red,Color_green,Color_blue);
   filledBox(spinBoxSize+spot-xMove,totalMin-spinBoxSize*2-spot+zMove,totalMin-spinBoxSize*2-spot,spinBoxSize*2-1+spot-xMove,totalMin-spinBoxSize-1-spot+zMove,totalMin-spinBoxSize-1-spot,Color_red,Color_green,Color_blue);
}

void spinBoxes()
{
	if (gCounter==1)
	{
		setName("Spin Boxes");
    modeWait=0;
		setOtherCubesDMA(0,1); // Mirror
		if (numCubes==1 || ledCubeType>=2) setEffect("fadein",0,0,0); // AURA12 can do effects over multiple cubes
		eFadeout=2; // Fadeout
		setAll(BLACK);		
	  randomColour(&Color_red,&Color_green,&Color_blue);
	  if (shared2!=255) 
		{
			// If we have got a common random number to send to all the cubes			
			Color_red=sharedA;
			Color_green=sharedB;
			Color_blue=sharedC;
		}
		nextShape=0;
		nextShapeSpot=0;
		nextShapeCounter=0;
		spinBoxSize=totalMin/4;
	}
	if (nextShape==0) 
	{
		spinBoxExp(nextShapeSpot);
		if (nextShapeSpot==spinBoxSize*2) 
	  {
		  nextShape++;
		  nextShapeSpot=0;
	  }
		nextShapeSpot++;
	}
	else if (nextShape>0 && nextShape<=5) 
	{
		spinBoxMove(nextShapeSpot,0,0);
	  if (nextShapeSpot==totalMin-spinBoxSize) 
	  { 
		  nextShape++;
		  nextShapeSpot=0;
	  }		
		nextShapeSpot++;
	}
	else if (nextShape>5 && nextShape<=10) 
	{
		spinBoxMove(totalMin-spinBoxSize-nextShapeSpot,0,0);
	  if (nextShapeSpot==totalMin-spinBoxSize) 
	  { 
		  nextShape++;
		  nextShapeSpot=0;
	  }		
		nextShapeSpot++;
	}
	else if (nextShape>10 && nextShape<=15) 
	{
		spinBoxMove(0,nextShapeSpot,0);
	  if (nextShapeSpot==totalMin-spinBoxSize) 
	  { 
		  nextShape++;
		  nextShapeSpot=0;
	  }		
		nextShapeSpot++;
	}
	else if (nextShape>15 && nextShape<=20) 
	{
		spinBoxMove(0,totalMin-spinBoxSize-nextShapeSpot,0);
	  if (nextShapeSpot==totalMin-spinBoxSize) 
	  { 
		  nextShape++;
		  nextShapeSpot=0;
	  }		
		nextShapeSpot++;
	}
	else if (nextShape>20 && nextShape<=25) 
	{
		spinBoxMove(totalMin-spinBoxSize-nextShapeSpot,nextShapeSpot,1);
	  if (nextShapeSpot==totalMin-spinBoxSize) 
	  { 
		  nextShape++;
		  nextShapeSpot=0;
	  }		
		nextShapeSpot++;
	}	
	else if (nextShape>25 && nextShape<=30) 
	{
		spinBoxMove(nextShapeSpot,totalMin-spinBoxSize-nextShapeSpot,1);
	  if (nextShapeSpot==totalMin-spinBoxSize) 
	  { 
		  nextShape++;
		  nextShapeSpot=0;
		  if(nextShape>30) nextShape=99;
	  }		
		nextShapeSpot++;
	}
	else //(nextShape==99) 
	{
		spinBoxExp(spinBoxSize*2-nextShapeSpot);
		if (spinBoxSize*2-nextShapeSpot==0) 
	  {
		  nextShape=0;
  	  nextShapeCounter++;
		  nextShapeSpot=0;
	    randomColour(&Color_red,&Color_green,&Color_blue);
	    if (shared2!=255) 
	  	{
			  // If we have got a common random number to send to all the cubes			
			  Color_red=sharedA;
			  Color_green=sharedB;
		  	Color_blue=sharedC;
		  }			
	  }
		else nextShapeSpot++;
	}
	delay(7*delaySpeed);
}

void migrate()
{
	uint8_t ran,ranX,ranZ;
	uint8_t x,y,z;
	uint16_t migrateTemp;
	uint16_t counter,ranCount;
	if (gCounter==1)
	{
    // set up
    //modeWait=1;
		setName("Migrate");
		setOtherCubesDMA(0,6);
		setAll(BLACK);
  	if (numCubes==1 || ledCubeType>=2) // AURA12 can do effects over multiple cubes 	
		{
			setEffect("fadein",0,0,0);
			eFadeout=2;
		}
		migrateDir=cube_random(6);
	  if (shared2!=255) migrateDir=shared6; // If we have got a common random number to send to all the cubes			
		migrateThere=0;
		migrateFreeze=0;
		migrateSlide=0;
		migrateNum=cube_random(totalPixels/34)+2;
		ran=cube_random(5);
	  if (shared2!=255) ran=shared5; // If we have got a common random number to send to all the cubes			
		R=0;G=0;B=0;
		if (ran==0) R=255;
		if (ran==1) G=255;
		if (ran==2) B=255;	 		
		if (ran==3) {R=255;G=50;B=100;}	 		
		if (ran==4) {R=255;G=255;}	 		
		if (migrateDir==0) {PlaneXY(0,0,0,curTotalX-1,0,curTotalZ-1,R,G,B,1);migrateSize=curTotalX*curTotalZ;migrateTx=curTotalX;migrateTz=curTotalZ;}
		if (migrateDir==1) {PlaneXY(0,curTotalY-1,0,curTotalX-1,curTotalY-1,curTotalZ-1,R,G,B,1);migrateSize=curTotalX*curTotalZ;migrateTx=curTotalX;migrateTz=curTotalZ;}
		if (migrateDir==2) {PlaneXY(0,0,0,0,curTotalY-1,curTotalZ-1,R,G,B,1);migrateSize=curTotalY*curTotalZ;migrateTx=curTotalY;migrateTz=curTotalZ;}
		if (migrateDir==3) {PlaneXY(curTotalX-1,0,0,curTotalX-1,curTotalY-1,curTotalZ-1,R,G,B,1);migrateSize=curTotalY*curTotalZ;migrateTx=curTotalY;migrateTz=curTotalZ;}
		if (migrateDir==4) {PlaneZX(0,0,0,curTotalX-1,curTotalY-1,0,R,G,B,1);migrateSize=curTotalX*curTotalY;migrateTx=curTotalX;migrateTz=curTotalY;}
		if (migrateDir==5) {PlaneZX(0,0,curTotalZ-1,curTotalX-1,curTotalY-1,curTotalZ-1,R,G,B,1);migrateSize=curTotalX*curTotalY;migrateTx=curTotalX;migrateTz=curTotalY;}
		migrateNoMove=migrateSize;
	}		
  ran=cube_random(migrateNum);
	ranCount=0;
	counter=0;
	while(counter<(migrateNum*2)&&ranCount<ran)
	{
		if (migrateDir==0)
		{
			ranX=cube_random(curTotalX);
			ranZ=cube_random(curTotalZ);
			if (!isClear(ranX,0,ranZ))
			{
				set(ranX,1,ranZ,R,G,B);
				clear(ranX,0,ranZ);
				ranCount++;
				migrateNoMove--;
			}		
	  }
		if (migrateDir==1)
		{
			ranX=cube_random(curTotalX);
			ranZ=cube_random(curTotalZ);
			if (!isClear(ranX,curTotalY-1,ranZ))
			{
				set(ranX,curTotalY-2,ranZ,R,G,B);
				clear(ranX,curTotalY-1,ranZ);
				ranCount++;
				migrateNoMove--;
			}		
	  }		
		if (migrateDir==2)
		{
			ranX=cube_random(curTotalY);
			ranZ=cube_random(curTotalZ);
			if (!isClear(0,ranX,ranZ))
			{
				set(1,ranX,ranZ,R,G,B);
				clear(0,ranX,ranZ);
				ranCount++;
				migrateNoMove--;				
			}		
	  }
		if (migrateDir==3)
		{
			ranX=cube_random(curTotalY);
			ranZ=cube_random(curTotalZ);
			if (!isClear(curTotalX-1,ranX,ranZ))
			{
				set(curTotalX-2,ranX,ranZ,R,G,B);
				clear(curTotalX-1,ranX,ranZ);
				ranCount++;
				migrateNoMove--;			
			}		
	  }	
		if (migrateDir==4)
		{
			ranX=cube_random(curTotalX);
			ranZ=cube_random(curTotalY);
			if (!isClear(ranX,ranZ,0))
			{
				set(ranX,ranZ,1,R,G,B);
				clear(ranX,ranZ,0);
				ranCount++;
				migrateNoMove--;
			}		
	  }				
		if (migrateDir==5)
		{
			ranX=cube_random(curTotalX);
			ranZ=cube_random(curTotalY);
			if (!isClear(ranX,ranZ,curTotalZ-1))
			{
				set(ranX,ranZ,curTotalZ-2,R,G,B);
				clear(ranX,ranZ,curTotalZ-1);
				ranCount++;
				migrateNoMove--;
			}		
	  }						
		counter++;
	}
	if (migrateThere>1&&migrateThere<(totalPixels/100)&&migrateFreeze==0)
	{
		ran=cube_random(2);
		if (ran==1)
		{
			delay(50*delaySpeed);
			migrateFreeze=1;
			migrateTemp=migrateThere;
			migrateThere=migrateNoMove;
			migrateNoMove=migrateTemp;
      switch(migrateDir)
		   {
			 case 0: migrateDir=1;break;
			 case 1: migrateDir=0;break;
			 case 2: migrateDir=3;break;
			 case 3: migrateDir=2;break;	
       case 4: migrateDir=5;break;
       case 5: migrateDir=4;break;			 
		  }
		}			 
	}
	if (migrateThere>(totalPixels/25)&&migrateFreeze==0)
	{
		delay(100*delaySpeed);
		migrateFreeze=1;
	}

	if (migrateThere==migrateSize)
	{
		 migrateThere=0;
		 migrateFreeze=0;
     switch(migrateDir)
		 {
			 case 0: migrateDir=1;break;
			 case 1: migrateDir=0;break;
			 case 2: migrateDir=3;break;
			 case 3: migrateDir=2;break;	
       case 4: migrateDir=5;break;
       case 5: migrateDir=4;break;			 
		 }	 
		 migrateNoMove=migrateSize;
		 delay(50*delaySpeed);
		 migrateSlide++;
		 if (migrateSlide>4&&migrateDir<4&&curTotalX==curTotalY&&curTotalX==curTotalZ) //Only slide if the sizes are the same!
		 {
			  if (migrateDir==0&&migrateSlide)
				{
					for (y=0;y<curTotalY-1;y++)
					{
						line(0,y+1,0,0,y+1,curTotalZ-1,R,G,B);
						line((curTotalX-1)-y,0,0,(curTotalX-1)-y,0,curTotalZ-1,BLACK);
						delay(10*delaySpeed);
					}
					migrateDir=2;
					migrateSlide=0;
					migrateSize=curTotalY*curTotalZ;
					migrateTx=curTotalY;migrateTz=curTotalZ;
				}
			  if (migrateDir==1&&migrateSlide)
				{
					for (y=0;y<curTotalY-1;y++)
					{
						line(curTotalX-1,(curTotalY-2)-y,0,curTotalZ-1,(curTotalY-2)-y,curTotalZ-1,R,G,B);
						line(y,curTotalY-1,0,y,curTotalY-1,curTotalZ-1,BLACK);
						delay(10*delaySpeed);
					}
					migrateDir=3;
					migrateSlide=0;
					migrateSize=curTotalY*curTotalZ;
					migrateTx=curTotalY;migrateTz=curTotalZ;
				}
			  if (migrateDir==2&&migrateSlide)
				{
					for (y=0;y<curTotalY-1;y++)
					{
						line(y+1,curTotalY-1,0,y+1,curTotalY-1,curTotalZ-1,R,G,B);
						line(0,y,0,0,y,curTotalZ-1,BLACK);
						delay(10*delaySpeed);
					}
					migrateDir=1;
					migrateSlide=0;
					migrateSize=curTotalX*curTotalZ;
					migrateTx=curTotalX;migrateTz=curTotalZ;
				}				
			  if (migrateDir==3&&migrateSlide)
				{
					for (y=0;y<curTotalY-1;y++)
					{
						line((curTotalX-2)-y,0,0,(curTotalX-2)-y,0,curTotalZ-1,R,G,B);
						line(curTotalX-1,(curTotalY-1)-y,0,curTotalX-1,(curTotalY-1)-y,curTotalZ-1,BLACK);
						delay(10*delaySpeed);
					}
					migrateDir=0;
					migrateSlide=0;
					migrateSize=curTotalX*curTotalZ;
					migrateTx=curTotalX;migrateTz=curTotalZ;
				}
			  delay(50*delaySpeed);
		 }
  }
	// move the ones already in motion
	for (x=0;x<migrateTx;x++)
	{
		  for (z=0;z<migrateTz;z++)
		  {
				 if (migrateDir==0)
				 // bottom to top
				 {
						if (isClear(x,0,z)&&isClear(x,curTotalY-1,z))
						{
							for (y=curTotalY-1;y>0;y--)
							{
								if (!isClear(x,y,z))
								{
								 set(x,y+1,z,R,G,B);
								 clear(x,y,z);
								 if(y==curTotalY-2) migrateThere++;
								}
							}
						}
					}
				 if (migrateDir==1)
				 // top to bottom 
				 {
						if (isClear(x,0,z)&&isClear(x,curTotalY-1,z))
						{
							for (y=1;y<curTotalY-1;y++)
							{
								if (!isClear(x,y,z))
								{
								 set(x,y-1,z,R,G,B);
								 clear(x,y,z);
								 if(y==1) migrateThere++;
								}
							}
						}
					}				
				 if (migrateDir==2)
				 // left to right
				 {
						if (isClear(0,x,z)&&isClear(curTotalX-1,x,z))
						{
							for (y=curTotalX-1;y>0;y--)
							{
								if (!isClear(y,x,z))
								{
								 set(y+1,x,z,R,G,B);
								 clear(y,x,z);
								 if(y==curTotalX-2) migrateThere++;
								}
							}
						}
					}
				 if (migrateDir==3)
				 // right to left
				 {
						if (isClear(0,x,z)&&isClear(curTotalX-1,x,z))
						{
							for (y=1;y<curTotalX-1;y++)
							{
								if (!isClear(y,x,z))
								{
								 set(y-1,x,z,R,G,B);
								 clear(y,x,z);
								 if(y==1) migrateThere++;
								}
							}
						}
					}				
				 if (migrateDir==4)
				 // forward to back
				 {
						if (isClear(x,z,0)&&isClear(x,z,curTotalZ-1))
						{
							for (y=curTotalZ-1;y>0;y--)
							{
								if (!isClear(x,z,y))
								{
								 set(x,z,y+1,R,G,B);
								 clear(x,z,y);
								 if(y==curTotalZ-2) migrateThere++;
								}
							}
						}
					}								 
				 if (migrateDir==5)
				 // back to forward
				 {
						if (isClear(x,z,0)&&isClear(x,z,curTotalZ-1))
						{
							for (y=1;y<curTotalZ-1;y++)
							{
								if (!isClear(x,z,y))
								{
								 set(x,z,y-1,R,G,B);
								 clear(x,z,y);
								 if(y==1) migrateThere++;
								}
							}
						}
					}			
			}
	}
	delay(5*delaySpeed);
}

void flyIn()
{
	uint8_t ran,ranX,ranZ;
	int8_t x,y,z;
	uint8_t migrateTemp;
	uint16_t counter,ranCount;
	if (gCounter==1)
	{
    // set up
    //modeWait=1;
		setOtherCubesDMA(0,1);
		setName("Fly In");
		setAll(BLACK);
    setEffect("fadein",0,0,0);
		eFadeout=2;
		migrateDir=0;
		migrateThere=0;
		migrateNoMove=curTotalX*curTotalY;
		migrateSize=curTotalX*curTotalY;
		migrateFreeze=0;
		migrateSlide=0;
		migrateNum=migrateNoMove/4;
		ran=cube_random(5);
		if (shared2!=255) ran=shared5; // If we have got a common random number to send to all the cubes			
		R=0;G=0;B=0;
		if (ran==0) R=255;
		if (ran==1) G=255;
		if (ran==2) B=255;	 		
		if (ran==3) {R=255;G=50;B=100;}	 		
		if (ran==4) {R=255;G=255;}
		
    R=80;		
		setAll(BLACK);
		setLED=1;
   	setPlaneZ(0,R,G,B);
		setLED=0;
	}		
	counter=0;
	while(counter<(migrateNum*2))
	{
		ranX=cube_random(curTotalX);
		ranZ=cube_random(curTotalY);
		if (migrateDir==0)
		{
			if (!isClearBuffer(1,ranX,ranZ,0))
			{
				set(ranX,ranZ,0,R,G,B);
				setLED=1;
				clear(ranX,ranZ,0);
				setLED=0;
				ranCount++;
				migrateNoMove--;
			}		
	  }				
		if (migrateDir==1)
		{
			if (!isClearBuffer(1,ranX,ranZ,curTotalZ-1))
			{
				set(ranX,ranZ,curTotalZ-1,R,G,B);
				setLED=1;
				clear(ranX,ranZ,curTotalZ-1);
				setLED=0;
				ranCount++;
				migrateNoMove--;
			}		
	  }						
		counter++;
	}
	if (migrateThere>1&&migrateThere<5&&migrateFreeze==0)
	{
		ran=cube_random(2);
		if (ran==1)
		{
			delay(5*delaySpeed);
			migrateFreeze=1;
			migrateTemp=migrateThere;
			migrateThere=migrateNoMove;
			migrateNoMove=migrateTemp;
      switch(migrateDir)
		   {
			 case 0: migrateDir=1;break;
			 case 1: migrateDir=0;break;
		  }
		}			 
	}

	if (migrateThere>(curTotalX*curTotalY)/3&&migrateFreeze==0)
	{
		delay(100*delaySpeed);
		migrateFreeze=1;
	}
	if (migrateThere==migrateSize)
	{
		 migrateThere=0;
		 migrateFreeze=0;
		 migrateNoMove=curTotalX*curTotalY;
     switch(migrateDir)
		 {
			 case 0: migrateDir=1;break;
			 case 1: migrateDir=0;break;
		 }	 
		 delay(50*delaySpeed);
  }
	// move the ones already in motion
	for (x=0;x<curTotalX;x++)
	{
		  for (z=0;z<curTotalY;z++)
		  {
				 if (migrateDir==0)
				 // forward to back
				 {
						if (isClearBuffer(1,x,z,0)&&isClearBuffer(1,x,z,curTotalZ-1))
						{
							for (y=curTotalZ-1;y>=0;y--) 
							{
								if (!isClear(x,z,y))
								{
								 if(y==curTotalZ-1) 
								 {
									 setLED=1;
									 set(x,z,curTotalZ-1,R,G,B);
									 setLED=0;	
									 clear(x,z,y);
									 migrateThere++;
								 }
								 else
								 {
										set(x,z,y+1,R,G,B);
									  clear(x,z,y);
								 }
								}
							}
						}
					}								 
				 if (migrateDir==1)
				 // back to forward
				 {
						if (isClearBuffer(1,x,z,0)&&isClearBuffer(1,x,z,curTotalZ-1))
						{
							for (y=0;y<curTotalZ;y++)
							{
								if (!isClear(x,z,y))
								{
								 if (y>0)
								 {
										set(x,z,y-1,R,G,B);
										clear(x,z,y);
								 }
								 else
								 {
									 setLED=1;
									 set(x,z,0,R,G,B);
									 setLED=0;
									 clear(x,z,y);
									 migrateThere++;
								 }
								}
							}
						}
					}			
			}
	}
	delay(5*delaySpeed);
}

void migrateThreeWay()
{
	uint8_t ranX,ranZ;
	uint8_t x,y,z;
	uint8_t lineCount=0;
	uint16_t counter,ranCount;
	
	if (gCounter==1)
	{
    // set up
    // modeWait=1;
		setName("3 Way Migrate");
		setOtherCubesDMA(0,6); // Each to their Own
		setAll(BLACK);
		//setEffect("fadetrail",0,0,0.8);
    //setEffect("fadein",0,0,0);
		//eFadeout=2;
		migrateThere=0;
		migrateNoMove=0;
		migrateFreeze=0;
		migrateDir=0;
		migrateSlide=0;
		migrateNum=3;//cube_random(15)+2;
		R=0;G=0;B=0;
		for (lineCount=1;lineCount<curTotalZ;lineCount++)
		{
			line(0,0,lineCount,curTotalX,0,lineCount,RED);
		}
		//Box(0,0,0,curTotalX,0,curTotalZ,RED,1,0);
		// can't get box to work
		for (lineCount=1;lineCount<curTotalY;lineCount++)
		{
			line(0,lineCount,0,0,lineCount,curTotalZ,GREEN);
		}
		for (lineCount=1;lineCount<curTotalY;lineCount++)
		{
			 line(0,lineCount,0,curTotalX,lineCount,0,BLUE);
		}
		line(0,0,0,0,0,curTotalZ-1,255,255,0);
	  line(0,0,0,curTotalX-1,0,0,255,0,255);
		line(0,0,0,0,curTotalY-1,0,0,255,255);
		set(0,0,0,WHITE);
		migrateSize=curTotalX*curTotalY+curTotalY*curTotalZ+curTotalX*curTotalZ;
	}		
	ranCount=0;
	counter=0;
	migrateNoMove=0;
	while(migrateNoMove==0&&counter<10000)
	{
		ranX=cube_random(totalMax);
		ranZ=cube_random(totalMax);
		if (migrateDir==0)
		{
			if (ranX<curTotalX && ranZ<curTotalZ)
			{
				if (getR(ranX,0,ranZ))
				{
					set(ranX,1,ranZ,255,getG(ranX,1,ranZ),getB(ranX,1,ranZ)); //Add RED
					set(ranX,0,ranZ,0,getG(ranX,0,ranZ),getB(ranX,0,ranZ));	// Clear the RED		
					ranCount++;
					migrateNoMove=1;
				}
			}				
	  }
		if (migrateDir==1)
		{
			if (ranX<curTotalX && ranZ<curTotalZ)
			{
				if (getR(ranX,curTotalY-1,ranZ))
				{
					set(ranX,curTotalY-2,ranZ,255,getG(ranX,curTotalY-2,ranZ),getB(ranX,curTotalY-2,ranZ)); //Add RED
					set(ranX,curTotalY-1,ranZ,0,getG(ranX,curTotalY-1,ranZ),getB(ranX,curTotalY-1,ranZ));	// Clear the RED		
					ranCount++;
					migrateNoMove=1;
				}
			}				
	  }		
		if (migrateDir==0)
		{
			if (ranX<curTotalY && ranZ<curTotalZ)
			{
				if (getG(0,ranX,ranZ))
				{
					set(1,ranX,ranZ,getR(1,ranX,ranZ),255,getB(1,ranX,ranZ));
					set(0,ranX,ranZ,getR(0,ranX,ranZ),0,getB(0,ranX,ranZ));
					ranCount++;
					migrateNoMove=1;	
				}
			}		
	  }
		if (migrateDir==1)
		{
			if (ranX<curTotalY && ranZ<curTotalZ)
			{
				if (getG(curTotalX-1,ranX,ranZ))
				{
					set(curTotalX-2,ranX,ranZ,getR(curTotalX-2,ranX,ranZ),255,getB(curTotalX-2,ranX,ranZ));
					set(curTotalX-1,ranX,ranZ,getR(curTotalX-1,ranX,ranZ),0,getB(curTotalX-1,ranX,ranZ));
					ranCount++;
					migrateNoMove=1;
				}					
			}		
	  }	
		if (migrateDir==0)
		{
			if (ranX<curTotalX && ranZ<curTotalY)
			{
				if (getB(ranX,ranZ,0))
				{
					set(ranX,ranZ,1,getR(ranX,ranZ,1),getG(ranX,ranZ,1),255);
					set(ranX,ranZ,0,getR(ranX,ranZ,0),getG(ranX,ranZ,0),0);
					ranCount++;
					migrateNoMove=1;
				}
			}		
	  }				
		if (migrateDir==1)
		{
			if (ranX<curTotalX && ranZ<curTotalY)
			{
				if (getB(ranX,ranZ,curTotalZ-1))
				{
					set(ranX,ranZ,curTotalZ-2,getR(ranX,ranZ,curTotalZ-2),getG(ranX,ranZ,curTotalZ-2),255);
					set(ranX,ranZ,curTotalZ-1,getR(ranX,ranZ,curTotalZ-1),getG(ranX,ranZ,curTotalZ-1),0);
					ranCount++;
					migrateNoMove=1;
				}
			}		
	  }						
		counter++;
	}
	
	if (migrateThere==migrateSize)
	{
		 migrateThere=0;
		 migrateFreeze=0;
		 migrateNoMove=0;
		 if (migrateDir==0) migrateDir=1;
		 else migrateDir=0;
		 delay(100*delaySpeed);
  }
	// move the ones already in motion
	for (x=0;x<totalMax;x++)
	{
		  for (z=0;z<totalMax;z++)
		  {
				 if (migrateDir==0)
				 // bottom to top
				 {
					 if (x<curTotalX && z<curTotalZ)
					 {
						if (!getR(x,0,z)&&!getR(x,curTotalY-1,z))
						{
							for (y=curTotalY-1;y>0;y--)
							{
								if (getR(x,y,z))
								{
								 set(x,y+1,z,255,getG(x,y+1,z),getB(x,y+1,z));
								 set(x,y,z,0,getG(x,y,z),getB(x,y,z)); // Clear the Red
								 if(y==curTotalY-2) migrateThere++;
								}
							}
						}
					 }
					}
				 if (migrateDir==1)
				 // top to bottom 
				 {
					 if (x<curTotalX && z<curTotalZ)
					 {
						if (!getR(x,0,z)&&!getR(x,curTotalY-1,z))
						{
							for (y=1;y<curTotalY-1;y++)
							{
								if (getR(x,y,z))
								{
								 set(x,y-1,z,255,getG(x,y-1,z),getB(x,y-1,z));
								 set(x,y,z,0,getG(x,y,z),getB(x,y,z));
								 if(y==1) migrateThere++;
								}
							}
						}
					 }
					}				
			   if (migrateDir==0)
				 // left to right
				 {
					 if (x<curTotalY && z<curTotalZ)
					 {
						if (!getG(0,x,z)&&!getG(curTotalX-1,x,z))
						{
							for (y=curTotalX-1;y>0;y--)
							{
								if (getG(y,x,z))
								{
								 set(y+1,x,z,getR(y+1,x,z),255,getB(y+1,x,z));
								 set(y,x,z,getR(y,x,z),0,getB(y,x,z));
								 if(y==curTotalX-2) migrateThere++;
								}
							}
						}
					 }
					}
				 if (migrateDir==1)
				 // right to left
				 {
					 if (x<curTotalY && z<curTotalZ)
					 {
						if (!getG(0,x,z)&&!getG(curTotalX-1,x,z))
						{
							for (y=1;y<curTotalX-1;y++)
							{
								if (getG(y,x,z))
								{
								 set(y-1,x,z,getR(y-1,x,z),255,getB(y-1,x,z));
								 set(y,x,z,getR(y,x,z),0,getB(y,x,z));
								 if(y==1) migrateThere++;
								}
							}
						}
					 }
					}				
				 if (migrateDir==0)
				 // forward to back
				 {
					 if (x<curTotalX && z<curTotalY)
					 {
						if (!getB(x,z,0)&&!getB(x,z,curTotalZ-1))
						{
							for (y=curTotalZ-1;y>0;y--)
							{
								if (getB(x,z,y))
								{
								 set(x,z,y+1,getR(x,z,y+1),getG(x,z,y+1),255);
								 set(x,z,y,getR(x,z,y),getG(x,z,y),0);
								 if(y==curTotalZ-2) migrateThere++;
								}
							}
						}
					 }
					}								 
				 if (migrateDir==1)
				 // back to forward
				 {
					 if (x<curTotalX && z<curTotalY)
					 {
						if (!getB(x,z,0)&&!getB(x,z,curTotalZ-1))
						{
							for (y=1;y<curTotalZ-1;y++)
							{
								if (getB(x,z,y))
								{
								 set(x,z,y-1,getR(x,z,y-1),getG(x,z,y-1),255);
								 set(x,z,y,getR(x,z,y),getG(x,z,y),0);
								 if(y==1) migrateThere++;
								}
							}
						}
					 }
					}			
			}
	}
	if (ledCubeType!=3) delay(10*delaySpeed);
	else delay(5*delaySpeed); // Faster for Wide Cube12
}

void doPlasma(void)
{
	// create a plasma and then modify it from SRAM contents
	uint8_t x,y,z;
	uint8_t colour;
	float level;
	level=fadeLevel/100;
	if (gCounter==1 || (Cube_MODE==4 && gCounter==2))
	{
    // set up
    //modeWait=1;
		setOtherCubesDMA(0,1); // Mirror
		setName("Plasma");
		setAll(BLACK);
		if (gCounter==1) // skip MENU mode
    {
			setEffect("fadein",0,0,0);
			eFadeout=2;
		}
		//generate the plasma once
		plasmaColour=random32(1000);
		plasmaColourType=cube_random(4);
		if (shared2!=255) plasmaColourType=shared4; // If we have got a common random number to send to all the cubes
		setLED =1;  
		for(x = 0; x < curTotalX; x++)
	  {
			for(y = 0; y < curTotalY; y++)
			{
				for(z = 0; z < curTotalZ; z++)
				{
					if (plasmaColourType==0) colour = (uint8_t)(32.0 + (32.0 * sin(x / 1.0))+ 32.0 + (32.0 * sin(y / 1.0)) + 32.0 + (32.0 * sin(z / 1.0))) / 3;
					if (plasmaColourType==1) colour = (uint8_t)(32.0 + (32.0 * cos(x / 1.0))+ 32.0 + (32.0 * cos(y / 1.0)) + 32.0 + (32.0 * cos(z / 1.0))) / 3;
					if (plasmaColourType==2) colour = (uint8_t)(32.0 + (32.0 * sin(x / 1.0))+ 32.0 + (32.0 * cos(y / 1.0)) + 32.0 + (32.0 * sin(z / 1.0))) / 3;
					if (plasmaColourType==3) colour = (uint8_t)(32.0 + (32.0 * cos(x / 1.0))+ 32.0 + (32.0 * sin(y / 1.0)) + 32.0 + (32.0 * cos(z / 1.0))) / 3;
					//if (plasmaColourType==3) set(z,y,x,(255-colour)|64,0,0); // |15 is quite interesting
					set(x,y,z,colour,0,0);
				}
			}
		}
		plasmaColourType=cube_random(4); // Set random again
	}
	for(x=0;x<curTotalX;x++)  
	{
		for(y=0;y<curTotalY;y++)  
		{
			for(z=0;z<curTotalZ;z++)
			{
				setLED=1;
				R=getR(x,y,z);
				setLED=0;
				if (plasmaColourType==0)
				{
					set(x,y,z,(ptrR[R+ plasmaColour]) % 256 * level,(ptrG[R + plasmaColour]) % 256 * level,(ptrB[R + plasmaColour]) % 256 * level);
				}
				if (plasmaColourType==1)
				{
					set(x,y,z,(ptrG[R+ plasmaColour]) % 256 * level,(ptrR[R + plasmaColour]) % 256 * level,(ptrB[R + plasmaColour]) % 256 * level);
				}
				if (plasmaColourType==2)
				{
					set(x,y,z,(ptrB[R+ plasmaColour]) % 256 * level,(ptrR[R + plasmaColour]) % 256 * level,(ptrG[R + plasmaColour]) % 256 * level);
				}
				if (plasmaColourType==3)
				{
					set(x,y,z,(ptrR[R+ plasmaColour]) % 256 * level,(ptrB[R + plasmaColour]) % 256 * level,(ptrG[R + plasmaColour]) % 256 * level);
				}
				if (plasmaColourType==4)
				{
					// White - Don't Use
					set(x,y,z,(ptrR[R+ plasmaColour]) % 256 * level,(ptrR[R + plasmaColour]) % 256 * level,(ptrR[R + plasmaColour]) % 256 * level);
				}
			}
		}
	}
 delay(30*delaySpeed);
 plasmaColour+=1;
 // Read from SRAM. 
 if (plasmaColour>1024) plasmaColour=0;
}

void flashtst()
{
	if (gCounter==1)
	{
		modeWait=1;
		NumCount=0;
		setAll(BLACK);
		Color_red=255;
	}
	//DNA_Change(NumCount,PINK);
	//Box_Change(NumCount,PINK,0); Up to 108
	NumCount++;
	delay(8*delaySpeed);
}


void testColours()
{
	// Code for Testing cube colours - TESTING ONLY
//	uint16_t amt=4;
//	uint16_t counter=0;
	uint8_t x,y,z;
	if (gCounter==1)
	{
		modeWait=1;
	  NumCount=0;
		setAll(BLACK);
	
		/*set (0,0,0,0,0,0);
  	set (0,1,0,4,4,4);
		set (0,2,0,16,16,16);
		set (0,3,0,25,25,25);
		set (0,4,0,28,28,28);
		set (0,5,0,32,32,32);
  	set (0,6,0,40,40,40);
		set (0,7,0,50,50,50);
		set (0,8,0,64,64,64);
		set (0,9,0,128,128,128);
		set (0,10,0,252,252,252);
		set (0,11,0,255,255,255);
		*/
		
		set (1,3,0,0,22,0);
		set (1,4,0,0,16,0);
		set (1,5,0,0,32,0);
		set (1,6,0,0,44,0);
		set (1,7,0,0,55,0);
		set (1,8,0,0,64,0);
		set (1,9,0,0,96,0);
		set (1,10,0,0,128,0);
		set (1,11,0,0,255,0);

		set (2,3,0,22,0,0);
		set (2,4,0,16,0,0);
		set (2,5,0,32,0,0);
		set (2,6,0,44,0,0);
		set (2,7,0,55,0,0);
		set (2,8,0,64,0,0);
		set (2,9,0,96,0,0);
		set (2,10,0,128,0,0);
		set (2,11,0,255,0,0);

		set (10,11,0,25,25,25);
		set (10,10,0,190,190,0);
		set (10,9,0,0,38,0);
		set (10,8,0,0,44,0);
		set (10,7,0,0,55,0);
		set (10,6,0,0,64,0);
		set (10,5,0,0,96,0);
		set (10,4,0,0,128,0);
		set (10,3,0,0,205,0);
		
		set (11,11,3,WHITE);
		set (12,11,0,PURPLE);
		set (23,11,1,PINK);
		set (23,11,0,PINK);		
		set (23,11,2,PINK);
		set (23,11,3,PINK);		
		set (24,11,2,CYAN);
		
		//delay(10000);
		for (y=0;y<12;y++)
		{
			for (z=0;z<4;z++)
  		{
				for (x=0;x<36;x++)
				{
					 set(x,y,z,WHITE);
					 delay(60);
				}
			}
		}
/*
		set (10,11,0,0,22,0);
		set (10,10,0,0,16,0);
		set (10,9,0,0,32,0);
		set (10,8,0,0,44,0);
		set (10,7,0,0,55,0);
		set (10,6,0,0,64,0);
		set (10,5,0,0,96,0);
		set (10,4,0,0,128,0);
		set (10,3,0,0,205,0);

*/

	/*	set (2,0,0,0,0,1);
  	set (2,1,0,0,0,2);
		set (2,2,0,0,0,4);
		set (2,3,0,0,0,8);
		set (2,4,0,0,0,16);
		set (2,5,0,0,0,32);
		set (2,6,0,0,0,44);
		set (2,7,0,0,0,55);
		set (2,8,0,0,0,64);
		set (2,9,0,0,0,96);
		set (2,10,0,0,0,128);
	//	set (2,11,0,0,0,245);*/

		/*for (counter=0;counter<12;counter++)
		{
			set(counter,0,1,getR(counter,0,0)+amt,0,0);
			set(counter,0,2,getR(counter,0,1)+amt,0,0);
			set(counter,0,3,getR(counter,0,2)+amt,0,0);

			set(counter,1,1,0,getG(counter,1,0)+amt,0);
			set(counter,1,2,0,getG(counter,1,1)+amt,0);
			set(counter,1,3,0,getG(counter,1,2)+amt,0);

			set(counter,2,1,0,0,getB(counter,2,0)+amt);
			set(counter,2,2,0,0,getB(counter,2,1)+amt);
			set(counter,2,3,0,0,getB(counter,2,2)+amt);

		}*/

	}
	NumCount++;
/*	if (gCounter<256) setAll(NumCount,NumCount,NumCount);
	if (gCounter>=256 && gCounter <512) setAll(NumCount,0,0);
  if (gCounter>=512 && gCounter <768) setAll(NumCount,255-NumCount,0);
	usb_printf("\r\n Number : %d",NumCount);
	*/
	delay(20*delaySpeed);
	if (NumCount>1024) {NumCount=0;gCounter=0;}//modeWaitUp=1;
}

void age()
{
	uint8_t x,y,z;
  uint8_t r,g,b;
	// age everyone
	for (x=0;x<curTotalX;x++)
	{
    for (y=0;y<curTotalY;y++)
		{
       for (z=0;z<curTotalZ;z++)
			 {
				 if(!isClear(x,y,z))
				 {
					  getRGB(x,y,z,&r,&g,&b);
					  if (r>0 && r<255) r++;
					  if (b>0 && b<255) b++;
            set(x,y,z,r,g,b);							 
					  if (b>154 && b<160)
						{
							// change to unmarried adult male
							set(x,y,z,0,150,240);
						}
						if (r==240)
						{
							// change to unmarried adult female
							set(x,y,z,240,40,0);
						}			 
				 }
			 }
		 }
	 }
}

uint8_t addNear(uint8_t xx,uint8_t yy,uint8_t zz,uint8_t R, uint8_t G, uint8_t B)
{
	int8_t x,y,z;
	uint16_t counter=0;
  uint8_t found=0;
	// add a person near the given cell
	while(counter<10000 && !found)
	{
		x=xx+cube_random(3)-1; // One spot is itself
		y=yy+cube_random(3)-1;
		z=zz+cube_random(3)-1;
		if (x>=0 && y>=0 && z>=0 && x<curTotalX && y<curTotalY && z<curTotalZ)
		// Is that spot still on the board ?
		{
			if (isClear(x,y,z))
			{
				// Found a spot for the cell
				set(x,y,z,R,G,B);
				found=1;
			}
		}			
		counter++;
	}
	return(found);
}

uint8_t marryMaleNear(uint8_t xx,uint8_t yy,uint8_t zz, uint8_t G)
{
	int8_t x,y,z;
	uint16_t counter=0;
  uint8_t found=0;
	uint8_t g;
	// find a person near the given cell
	while(counter<10000 && !found)
	{
		x=xx+cube_random(3)-1; // One spot is itself
		y=yy+cube_random(3)-1;
		z=zz+cube_random(3)-1;
		if (x>=0 && y>=0 && z>=0 && x<curTotalX && y<curTotalY && z<curTotalZ)
		// Is that spot still on the board ?
		{
			g=getG(x,y,z);
			if (g==G)
			{
				// change to Married male
				set(x,y,z,0,0,240);
				found=1;
			}
		}			
		counter++;
	}
	return(found);
}

uint8_t findNear(uint8_t xx,uint8_t yy,uint8_t zz)
{
	int8_t x,y,z;
	uint16_t counter=0;
  uint8_t found=0;
	uint8_t b;
	// find a person near the given cell
	while(counter<10000 && !found)
	{
		x=xx+cube_random(3)-1; // One spot is itself
		y=yy+cube_random(3)-1;
		z=zz+cube_random(3)-1;
		if (x>=0 && y>=0 && z>=0 && x<curTotalX && y<curTotalY && z<curTotalZ)
		// Is that spot still on the board ?
		{
			b=getB(x,y,z);
			if (b>=240)
			{
				found=1;
			}
		}			
		counter++;
	}
	return(found);
}

void addParents()
{
	uint8_t x,y,z;
	uint16_t counter=0;
  uint8_t found=0;
	// add a parent pair to the simulation
	while(counter<10000 && !found)
	{
		x=cube_random(curTotalX);
		y=cube_random(curTotalY);
		z=cube_random(curTotalZ);
	  if (isClear(x,y,z))
		{
			// Found a spot for the mother
			set(x,y,z,240,0,0);
			// Add a father
			addNear(x,y,z,0,0,240);
			found=1;
		}			
		counter++;
	}
}

void addChildren()
{
	uint8_t x,y,z;
  uint8_t r;
	uint8_t ran;
	// Find a mother that is married
	for (x=0;x<curTotalX;x++)
	{
    for (y=0;y<curTotalY;y++)
		{
       for (z=0;z<curTotalZ;z++)
			 {
				  r=getR(x,y,z);
				  if (r>240&&r<(242+lifeChildYears/5))
					{
						// can give birth for 20 years only
						if (findNear(x,y,z))
						// can only give birth if male still alive
						{
							ran=cube_random(2); // birth rate every 3 years
							if (ran==0)
							{
								// add a child
								ran=cube_random(2); // Male of Female
								if (ran==0)
								{
									// Female 4*5 = 20 years before adult
									addNear(x,y,z,236,20,70);
								}
								else
								{
									// Male 4*5 = 20 years before adult
									addNear(x,y,z,50,200,150);
								}
								ran=cube_random(10); // twin birth rate
								if (ran==0)
								{
								// add a twin
									ran=cube_random(2); // Male of Female
									if (ran==0)
									{
										// Female 4*5 = 20 years before adult
										addNear(x,y,z,236,20,70);
									}
									else
									{
										// Male 4*5 = 20 years before adult
										addNear(x,y,z,50,200,150);
									}
								}
							}
						}
					}
			 }
		 }
	 }
}

void moveUnMarried()
{
	uint8_t x,y,z;
  uint8_t r,g,b;
	uint8_t found=0;
	// move any unmarried adults
	for (x=0;x<curTotalX;x++)
	{
    for (y=0;y<curTotalY;y++)
		{
       for (z=0;z<curTotalZ;z++)
			 {
				 getRGB(x,y,z,&r,&g,&b);
				 if (g==40 || g==150) 
				 {
					 found=addNear(x,y,z,r,g,b);
					 if(found)
					 {
						 // we've moved so clear where we were
						 clear(x,y,z);
					 }
				 }
			 }
		 }
	 }
}

void marry()
{
	uint8_t x,y,z;
  uint8_t g;
	uint8_t ran,found;
	// Find a female that is unmarried
	for (x=0;x<curTotalX;x++)
	{
    for (y=0;y<curTotalY;y++)
		{
       for (z=0;z<curTotalZ;z++)
			 {
				  g=getG(x,y,z);
				  if (g==40)
					{
						// Find a male near here
						found=marryMaleNear(x,y,z,150);
						ran=cube_random(6-lifeMarryRate); // have I met the right person ?
						if (found && ran)
						{
							// change to Married Female
							set(x,y,z,240,0,0);
						}
					}
			 }
		 }
	 }
}

void die()
{
	uint8_t x,y,z;
  uint8_t r,b;
	uint8_t ran;
	float chance;
	// kill anyone old
	for (x=0;x<curTotalX;x++)
	{
    for (y=0;y<curTotalY;y++)
		{
       for (z=0;z<curTotalZ;z++)
			 {
				 r=getR(x,y,z);
				 b=getB(x,y,z);
				 if (r==255 || b==255)
				 {
					 clear(x,y,z);
				 }
				 if (r>240)
				 {
					 chance=255-r;
					 ran=cube_random(chance*(11-lifeDeathRate)); // death rate female
					 if (ran==0)
					 {
						 clear(x,y,z);
					 }
				 }
				 if (b>240)
				 {
					 chance=255-b;
					 ran=cube_random(chance*(11-lifeDeathRate)); // death rate male
					 if (ran==0)
					 {
						 clear(x,y,z);
					 }
				 }
			 }
		 }
	 }
}

void checkRestart()
{
	uint8_t x,y,z;
	uint16_t count=0;	
	for (x=0;x<curTotalX;x++)
	{
    for (y=0;y<curTotalY;y++)
		{
       for (z=0;z<curTotalZ;z++)
			 {
				 if (!isClear(x,y,z))
				 {
					 count++;
				 }
			 }
		 }
	 }
	if (isClearAll()|| count>250)
	// All Dead ! or OverPopulated
	{
		modeWaitUp=1;
		/*delay(200*delaySpeed);
		setAll(BLACK);
		for(count=0;count<lifeStartParents;count++)
		{
			addParents();	
		}*/
	}
}

void simulateLife()
{
	uint8_t count;	
	if (gCounter==1)
	{
    // set up
		setName("Simulate Life");
		setOtherCubesDMA(0,6);
    modeWait=1;
		setAll(BLACK);
		// Add Parents
		for(count=0;count<lifeStartParents*numCubes;count++)
		{
			addParents();	
		}
	}
	if (gCounter!=0)
	{
		age();
		addChildren();
		moveUnMarried();
		marry();
		die();
		checkRestart();
		delay(20*delaySpeed);
	}
}

uint8_t checkCell(uint8_t buf, uint8_t xx,uint8_t yy,uint8_t zz)
{
	int8_t x,y,z;
	int8_t Tx,Ty,Tz;
	uint8_t neighbours = 0;
	// check all 26 cells around me if they are on the board
	for (x=-1;x<2;x++)
	{
    for (y=-1;y<2;y++)
		{
       for (z=-1;z<2;z++)
			 {
				  Tx=xx+x;
				  Ty=yy+y;
				  Tz=zz+z;
				  if (Tx<0||Ty<0||Tz<0||Tx>=curTotalX||Ty>=curTotalY||Tz>=curTotalZ) continue; // off the board
				  if (x==0 && y==0 && z==0) continue; // this is me !
				  if (ledCubeType>=2)
					{
						//CUBE12			
						if (!isClearBuffer(buf,Tx,Ty,Tz) && !outOfBounds) neighbours++; // check the buffer not active display!	
					}
					else
					{
						//AURA8
						if (onThisCube(Tx,Ty,Tz)==1) // this cube or taken care of in aura12
						{
							// could be swap cubes
							if (Tx>=myTotalX) Tx=Tx-myTotalX;  
							if (Ty>=myTotalY) Ty=Ty-myTotalY;
							if (Tz>=myTotalZ) Tz=Tz-myTotalZ;
							Tx %= myTotalX;													
							Ty %= myTotalY;													
							Tz %= myTotalZ;	
							if (!isClearBuffer(buf,Tx,Ty,Tz)) neighbours++; // check the buffer not active display!
						}
						else
						{
							// Other Cube Cube8
							if (Tx>=myTotalX) Tx=Tx-myTotalX;  
							if (Ty>=myTotalY) Ty=Ty-myTotalY;
							if (Tz>=myTotalZ) Tz=Tz-myTotalZ;
							Tx %= myTotalX;													
							Ty %= myTotalY;													
							Tz %= myTotalZ;	
							if (buf==0)
							{
								if (!isClearBuffer(3,Tx,Ty,Tz)) neighbours++; // check the DMA Buffer	
							}
							else
							{
								if (!isClearBuffer(4,Tx,Ty,Tz)) neighbours++; // check the buffer not active display!													
							}
						}												
					}
			 }
		 }
	 }
	return(neighbours);
}

void reset3DLife(uint8_t seedCells, uint8_t startCells)
{ 
	uint8_t counter=0;
	uint16_t counter2=0;
	uint16_t counter3=0; // Don't get stuck in a loop
	uint8_t x,y,z,neighbours;
	uint8_t spacing=2;
	uint8_t spacingTemp;
	uint8_t spacingMax=6;
	setAll(BLACK);
	while (counter<seedCells && modeC==16) // Only keep going if we are on the right graphic
	{
	   x=cube_random(curTotalX);
     y=cube_random(curTotalY);
     z=cube_random(curTotalZ);
     if(isClear(x,y,z))
		 {
			  // added another one
        set(x,y,z,R,G,B);
			  counter++;
		 }			 
	}
	spacingTemp=spacing;
	// dont reset counter - seed cells count towards startCells
	while (counter<startCells && modeC==16) // Only keep going if we are on the right graphic
	{
	   x=cube_random(curTotalX);
     y=cube_random(curTotalY);
     z=cube_random(curTotalZ);
     if(isClear(x,y,z) && !outOfBounds)
		 {
			  neighbours=checkCell(setLED,x,y,z);
			  if (neighbours<spacingMax)
				{
					if (neighbours>=spacingTemp || counter<seedCells*4)
					{
						// added another one near seed cells
						set(x,y,z,R,G,B);
						counter++;
						spacingTemp=spacing;
					}
				}
		 }			 
		 counter2++;
		 counter3++;
		 if(counter2>200)
		 {
			 // nothingworking
			 if (spacingTemp>0) spacingTemp--;
			 counter2=0;
		 }
		 if (counter3>40000)
		 {
			 counter=startCells;
		 }
	}
}

void conway3DLife()
{
	// 3d Life 5766
	uint8_t SL,SH,CL,CH;
	uint8_t x,y,z,neighbours;
	uint8_t ran;
	uint8_t changedCells, seedCells, startCells;
	SL = 4;            // Minimum # cells to survive
  SH = 7;            // Maximum # cells to survive
  CL = 6;            // Minimun # cells to create a new one
  CH = 6;            // Maximum # cells to create a new one
	seedCells = 2;     // Cells to put other cells around
	if (gCounter==1)
	{
		// set up
		setName("Conway Life");
		setOtherCubesDMA(0,6);
		setAll(BLACK);
		R=0;G=0;B=0;
		startCells = totalPixels/12;   // Number of start cells 40
		ran=cube_random(5);
		if (shared2!=255) ran=shared5; // If we have got a common random number to send to all the cubes
		if (ran==0) R=255;
		if (ran==1) G=255;
		if (ran==2) B=255;	 		
		if (ran==3) {R=255;G=50;B=100;}	 		
		if (ran==4) {R=255;G=255;}
		lifeGames=0;
    modeWait=1;
    reset3DLife(seedCells,startCells);
	}
	copyBuffer(0,1); // need to store the start state to compare to (not the active changing one!)
	if (numCubes>1)
	{
		// set up for multi cubes
		if (ledCubeType<2) 
		{
			if (numCubes==2) copyBuffer(3,4); // Cube8. Dont copy with 3 cubes as cube 3 uses buffer 4
		}
		else
		{
			//Cube12
			copyDMABuffers(0,1);
		}
	}
	changedCells = 0;
	for (x=0;x<curTotalX;x++)
	{
    for (y=0;y<curTotalY;y++)
		{
       for (z=0;z<curTotalZ;z++)
			 {
			     neighbours=checkCell(1,x,y,z);
				   if (neighbours<SL||neighbours>SH) 
					 {
						 if(!isClear(x,y,z))
						 {
								clear(x,y,z); // Cell Die
								changedCells++; // simulation still doing something!
					   }
					 }
				   if (neighbours>=CL&&neighbours<=CH) 
					 {
						 if (isClear(x,y,z))
						 {
								set(x,y,z,R,G,B);
								changedCells++; // simulation still doing something!
					   }
					 }
			 }
		 }
	 }		
	gIteration++;
	if ((changedCells==0||gIteration>conwayIteration) && modeC==16 ) // Only keep going if we are on the right graphic 
	{
		// Nothing Changed - reset or gone on too long
	  delay(100*delaySpeed); 	
		gIteration=0;
		lifeGames++;
		if (lifeGames>3)
		{
			modeWaitUp=1;
		}
		else
		{
			R=0;G=0;B=0;
			ran=cube_random(5);
			if (ran==0) R=255;
			if (ran==1) G=255;
			if (ran==2) B=255;	 		
			if (ran==3) {R=255;G=50;B=100;}	 		
			if (ran==4) {R=255;G=255;}
			startCells = totalPixels/12;   // Number of start cells 40
			reset3DLife(seedCells,startCells);
		}
	}
	delay(30*delaySpeed);
}

void shapes()
{
	// Explode Shapes
	if (gCounter==1)
	{
		 setName("Shapes");
		 setOtherCubesDMA(0,1);
		 setAll(BLACK);
		 modeWait=1;
		 nextShapeCounter=0;
		 nextShape=0;
		 vXpos = curCentreX;
		 vYpos = curCentreY;
	   vZpos = curCentreZ;
		 vX = 3;
	   vY = 8;
	   vZ = 4;
		 if (ledCubeType>=2)
		 {
			 // Faster on Cube12
		   vX = 5;
			 vY = 10;
			 vZ = 6;			 
		 }
	}
	if (nextShape==0||nextShape==5)
	{
    setAll(BLACK);
		//if (mySize==12) Box(5,5,5,6,6,6,PURPLE,1,0);
		//else Box(3,3,3,4,4,4,PURPLE,1,0);
		Box(curCentreX-1,curCentreY-1,curCentreZ-1,curCentreX,curCentreY,curCentreZ,PURPLE,1,0);
	}
	if (nextShape==1||nextShape==6)
	{
		setAll(BLACK);
		//if (mySize==12) Box(4,4,4,7,7,7,BLUE,1,0);
		//else Box(2,2,2,5,5,5,BLUE,1,0);
		Box(curCentreX-2,curCentreY-2,curCentreZ-2,curCentreX+1,curCentreY+1,curCentreZ+1,BLUE,1,0);
	}
	if (nextShape==2||nextShape==7) 
	{
		setAll(BLACK);
		sphere(curCentreX-1,curCentreY-1,curCentreZ-1,1,GREEN);
		sphere(curCentreX-1,curCentreY-1,curCentreZ-1,2,RED);
		sphere(curCentreX-1,curCentreY-1,curCentreZ-1,3,AQUA);
    /*if (mySize==12)
		{
			sphere(5,5,5,1,GREEN);
			sphere(5,5,5,2,RED);
			sphere(5,5,5,3,AQUA);
		}
		else
		{
			sphere(3,3,3,1,GREEN);
			sphere(3,3,3,2,RED);
			sphere(3,3,3,3,AQUA);
		}*/
	}
	if (nextShape==3||nextShape==8)
	{
		//pyramid
		setAll(BLACK);
		set(curCentreX-1,curCentreY+1,curCentreZ-1,PINK);
		Box(curCentreX-2,curCentreY,curCentreZ-2,curCentreX,curCentreY,curCentreZ,OLIVE,1,0);
	  Box(curCentreX-3,curCentreY-1,curCentreZ-3,curCentreX+1,curCentreY-1,curCentreZ+1,AQUA,1,0);
		Box(curCentreX-4,curCentreY-2,curCentreZ-4,curCentreX+2,curCentreY-2,curCentreZ+2,PURPLE,1,0);
    /*if (mySize==12)
		{
			set(5,7,5,PINK);
			Box(4,6,4,6,6,6,OLIVE,1,0);
			Box(3,5,3,7,5,7,AQUA,1,0);
			Box(2,4,2,8,4,8,PURPLE,1,0);
		}
		else
		{
			set(3,5,3,PINK);
			Box(2,4,2,4,4,4,OLIVE,1,0);
			Box(1,3,1,5,3,5,AQUA,1,0);
			Box(0,2,0,6,2,6,PURPLE,1,0);
		}*/
	}
	if (nextShape==4||nextShape==9)
	{
		//pyramid
		setAll(BLACK);
		set(curCentreX-1,curCentreY+2,curCentreZ-1,OLIVE);
		Box(curCentreX-2,curCentreY+1,curCentreZ-2,curCentreX,curCentreY+1,curCentreZ,OLIVE,1,0);
		Box(curCentreX-2,curCentreY,curCentreZ-2,curCentreX,curCentreY,curCentreZ,OLIVE,1,0);
		Box(curCentreX-3,curCentreY-1,curCentreZ-3,curCentreX+1,curCentreY-1,curCentreZ+1,OLIVE,1,0);
		Box(curCentreX-3,curCentreY-2,curCentreZ-3,curCentreX+1,curCentreY-2,curCentreZ+1,OLIVE,1,0);
		Box(curCentreX-4,curCentreY-3,curCentreZ-4,curCentreX+2,curCentreY-3,curCentreZ+2,OLIVE,1,0);
		Box(curCentreX-4,curCentreY-4,curCentreZ-4,curCentreX+2,curCentreY-4,curCentreZ+2,OLIVE,1,0);		
		/*if (mySize==12)
		{
			set(5,8,5,OLIVE);
			Box(4,7,4,6,7,6,OLIVE,1,0);
			Box(4,6,4,6,6,6,OLIVE,1,0);
			Box(3,5,3,7,5,7,OLIVE,1,0);
			Box(3,4,3,7,4,7,OLIVE,1,0);
			Box(2,3,2,8,3,8,OLIVE,1,0);
			Box(2,2,2,8,2,8,OLIVE,1,0);
		}
		else
		{
			set(3,6,3,OLIVE);
			Box(2,5,2,4,5,4,OLIVE,1,0);
			Box(2,4,2,4,4,4,OLIVE,1,0);
			Box(1,3,1,5,3,5,OLIVE,1,0);
			Box(1,2,1,5,2,5,OLIVE,1,0);
			Box(0,1,0,6,1,6,OLIVE,1,0);
			Box(0,0,0,6,0,6,OLIVE,1,0);
		}*/
	}	
	if (nextShape==10)
	{
		setAll(BLACK);
	  sphere(getRound(vXpos),getRound(vYpos),getRound(vZpos),3,ORANGE);
		moveVelocity(1,1,1,3,3,3,2,4);
	}
	if (eNum==0) 
	{
		if (nextExplode==0) 
		{
			if (nextShape<5) setEffect("explode",0,2,0);  //part explode
			else setEffect("explode",0,0,0);  //explode
			nextExplode=1;
			nextShapeCounter++;
			if (nextShapeCounter>5 || (nextShapeCounter>2&&nextShape!=10))
			{
				nextShapeCounter=0;
				nextShape++;
				if (nextShape>10)
				{
					modeWaitUp=1;
				}
			}
		}
		else 
		{
			if (nextShape>4 && nextShape<10) setEffect("explode",30,1,0);  //unexplode
			else 
			{
				if (nextShape >4 && nextExplode==1) setEffect("explode",10,1,0);  //unexplode
			}
			if (nextShape!=10) nextExplode=0;
			else nextExplode++;
		}
		if (nextExplode>250) nextExplode=0;
	}
	if (nextShape!=10) delay(100*delaySpeed);
	else 
	{
		if (ledCubeType<2) delay(delaySpeed);
		else delay(delaySpeed/4);
	}
}

void doFFT1()
{
	if (gCounter==1)
	{
		setName("FFT 1");
	  ADCRunning=1;
		fft_MODE=0;
	}
	SHOW_FFT();
}

void doFFT2()
{
	if (gCounter==1)
	{
		setName("FFT 2");
	  ADCRunning=1;
		fft_MODE=4;
	}
	SHOW_FFT();
}

void doFFT3()
{
	if (gCounter==1)
	{
		setName("FFT 3");
	  ADCRunning=1;
		fft_MODE=7;
	}
	SHOW_FFT();
}

void doFFT4()
{
	if (gCounter==1)
	{
		setName("FFT 4");
	  ADCRunning=1;
		fft_MODE=10;
	}
	SHOW_FFT();
}

void doFFT5()
{
	if (gCounter==1)
	{
		setName("FFT 5");
	  ADCRunning=1;
		fft_MODE=11;
	}
	SHOW_FFT();
}

void playSDAllOneCube()
{
	FRESULT res;
	uint8_t state = 0;
	if (mf_getSemaphore(0))
	{
		res=mf_load_path_gen();
		if (res == FR_OK)
		{
			state=mf_load_next_file(file,&dir,0);
			SDinUse=4;
			while(2 == Cube_MODE && state==0 && Cube_PROGRAM==2)
			{
				if (power==1)
				{
					checkMusic();
					if (mf_getSemaphore(1))
					{
						mf_read_to_cube(file);
						if (br==0)
						{
							f_close(file);
							state=mf_load_next_file(file,&dir,0);
						}
						SDinUse=0;
					  //if (musicPlaying==0 && musicPlay && Cube_MODE!=9)
	          //{
		           //folderRunTune(0,res);
	          //}	
						delay(delaySpeed*3.5);
					}
				}			
			}
			f_close(file);
			f_closedir(&dir);
		}
	}
}

FRESULT openAllFiles()
{
	char path[15]="0:\\rgbCUBE8A0";
	char path2[15]="0:\\rgbCUBE12A0";
	FRESULT res;
  uint8_t x=0;
	// Open Host First
	f_closedir(&dir);
	if (ledCubeType<2) res=mf_load_path(path); //CUBE8
	else res=mf_load_path(path2); // CUBE12
	if (res == FR_OK)
	{
		mf_load_next_file(file,&dir,0);
	}
	f_closedir(&dir);
	for (x=0;x<10;x++) 
	{
		// Other Cubes
		if (ledCubeType<2) 
		{
			//AURA8
			if (DMAAddresses[x]!=0 && DMAAddresses[x]<2) // Only have 5 array slots - So max 5 cubes
			{
				path[12]=DMAAddresses[x]+48; // place the address in the path - convert to ASCII 
				res=mf_load_path(path);
				if (res == FR_OK)
				{		
						if (DMAAddresses[x]==1) mf_load_next_file(htmlFile,&dir,0);  // shouldn't do this but no ram for extra buffers
				}
				f_closedir(&dir);
			}
		}
	  else
		{
			//CUBE12
				if (DMAAddresses[x]!=0)
				{
					path2[13]=DMAAddresses[x]+48; // place the address in the path - convert to ASCII 
					res=mf_load_path(path2);
					if (res == FR_OK)
					{		
							if (DMAAddresses[x]==1) mf_load_next_file(file1,&dir,0); 
						  if (DMAAddresses[x]==2) mf_load_next_file(file2,&dir,0);
						  if (DMAAddresses[x]==3) mf_load_next_file(file3,&dir,0);
						  if (DMAAddresses[x]==4) mf_load_next_file(file4,&dir,0);
						  if (DMAAddresses[x]==5) mf_load_next_file(file5,&dir,0);
//						  if (DMAAddresses[x]==6) mf_load_next_file(file6,&dir,0);
//						  if (DMAAddresses[x]==7) mf_load_next_file(file7,&dir,0);
//						  if (DMAAddresses[x]==8) mf_load_next_file(file8,&dir,0);
//						  if (DMAAddresses[x]==9) mf_load_next_file(file9,&dir,0);						
					}
					f_closedir(&dir);
				}			
		}
	}
	return res;
}

FRESULT loadNextFile(uint8_t address, uint8_t fileNum)
{
	char path[15]="0:\\rgbCUBE8A0";
	char path2[15]="0:\\rgbCUBE12A0";
	FRESULT res;
  f_closedir(&dir);
	if (address==0)
	{
		//Host
		if (ledCubeType<2) 	res=mf_load_path(path); //CUBE8
		else 	res=mf_load_path(path2); // CUBE12
		if (res == FR_OK) res=mf_load_next_file(file,&dir,fileNum);
	}
	else
	{
			if (ledCubeType<2)
			{
				//Cube8
				path[12]=address+48; // place the address in the path - convert to ASCII 
				res=mf_load_path(path);
				if (res == FR_OK)	res=mf_load_next_file(htmlFile,&dir,fileNum); // overloading htmlfile
			}
			else
			{
				//CUBE12
				path2[13]=address+48; // place the address in the path - convert to ASCII 
				res=mf_load_path(path2);
				if (res == FR_OK)
				{
					  #ifdef doUSB
						if (hasUSB) usb_printf("\r\nNext File Cube Address %d",address);
            #endif
						if (address==1) mf_load_next_file(file1,&dir,fileNum);  // shouldn't do this but no ram for extra buffers
					  if (address==2) mf_load_next_file(file2,&dir,fileNum);
					  if (address==3) mf_load_next_file(file3,&dir,fileNum);
					  if (address==4) mf_load_next_file(file4,&dir,fileNum);
					  if (address==5) mf_load_next_file(file5,&dir,fileNum);
//					  if (address==6) mf_load_next_file(file6,&dir,fileNum);
//					  if (address==7) mf_load_next_file(file7,&dir,fileNum);
//					  if (address==8) mf_load_next_file(file8,&dir,fileNum);
//					  if (address==9) mf_load_next_file(file9,&dir,fileNum);			
				}
			}
	}
	return res;
}

void playSDAllMultiCubes()
{
	uint8_t state = 0;
	FRESULT res;
	uint8_t x=0;
	uint8_t address=0;
  uint16_t filePos=1;
  uint16_t filePos1=1;
  uint16_t filePos2=1;
  uint16_t filePos3=1;
  uint16_t filePos4=1;
  uint16_t filePos5=1;
//  uint16_t filePos6=1;
//  uint16_t filePos7=1;
//  uint16_t filePos8=1;
//  uint16_t filePos9=1;
	showLED=0; // rgbled 0 for host cube 
	setLED=0;
	setOtherCubesDMA(0,6); // Each cube to its own!
	curTotalX=trimTotalX; // Full Size each to its own !!
	curTotalY=trimTotalY;
	curTotalZ=trimTotalZ;	
	needToClearDMA=2;
  #ifdef doUSB
  if (hasUSB) usb_printf("\r\nPlay SD All MULTI CUBES");
  #endif	
	if (ledCubeType <2 && hasWIFI==4) delay(2000); // wait for html page to reload since we are using htmlfiles and sd card for html on cube8
	if (mf_getSemaphore(0))
	{
		res=openAllFiles();
		SDinUse=4;
		while(2 == Cube_MODE && state==0 && Cube_PROGRAM==2 && res == FR_OK)
		{
			if (power==1)
			{
				checkMusic();
				setLED=0;  // load into cube version
				if (mf_getSemaphore(1))
				{
					mf_read_to_cube(file);
					if (br==0)
					{
						f_close(file);
						state=loadNextFile(0,filePos);
						filePos++;
					}
					for (x=0;x<10;x++) 
					{
						if (ledCubeType<2)
						{					
							// Other Cubes - CUBE8
							if (DMAAddresses[x]!=0 && DMAAddresses[x]<2 && state==0) // Only have 5 array slots - So max 5 cubes
							{
								setLED=3; // hard code for one cube for now due to memory constraints. DMAAddresses[x];  // load into cube version
								mf_read_to_cube(htmlFile);
								if (br==0)
								{
									f_close(htmlFile);
									state=loadNextFile(1,filePos1);
									filePos1++;
								}
							}
						}
						else
						{
							//CUBE12
							address=DMAAddresses[x];
							if (address!=0 && state==0)
							{
								if (address==1) mf_read_to_dma(file1,address);
								if (address==2) mf_read_to_dma(file2,address);
								if (address==3) mf_read_to_dma(file3,address);
								if (address==4) mf_read_to_dma(file4,address);
								if (address==5) mf_read_to_dma(file5,address);
//								if (address==6) mf_read_to_dma(file6,address);
//								if (address==7) mf_read_to_dma(file7,address);
//								if (address==8) mf_read_to_dma(file8,address);
//								if (address==9) mf_read_to_dma(file9,address);
								if (br==0)
								{
									if (address==1) {f_close(file1);loadNextFile(address,filePos1);filePos1++;} //Don't care about slave state opening files
									if (address==2) {f_close(file2);loadNextFile(address,filePos2);filePos2++;}
									if (address==3) {f_close(file3);loadNextFile(address,filePos3);filePos3++;}
									if (address==4) {f_close(file4);loadNextFile(address,filePos4);filePos4++;}
									if (address==5) {f_close(file5);loadNextFile(address,filePos5);filePos5++;}
//									if (address==6) {f_close(file6);loadNextFile(address,filePos6);filePos6++;}
//									if (address==7) {f_close(file7);loadNextFile(address,filePos7);filePos7++;}
//									if (address==8) {f_close(file8);loadNextFile(address,filePos8);filePos8++;}
//									if (address==9) {f_close(file9);loadNextFile(address,filePos9);filePos9++;}
								}
							}					
						}
					}
				}
				SDinUse=0;
				delay(delaySpeed*3.5);
			}
		}
		f_close(file);
		if (ledCubeType<2) f_close(htmlFile);
		else
		{
			for (x=0;x<10;x++) 
			{
				address=DMAAddresses[x];
				if (address==1) f_close(file1);
				if (address==2) f_close(file2);
				if (address==3) f_close(file3);
				if (address==4) f_close(file4);
				if (address==5) f_close(file5);
//				if (address==6) f_close(file6);
//				if (address==7) f_close(file7);
//				if (address==8) f_close(file8);
//				if (address==9) f_close(file9);
			}		
		}
		f_closedir(&dir);
	}
	SDinUse=0;
}

void playSDAll()
{
	// play all from the SD
	Cube_PROGRAM=2;
	modeWait=1;
	setName("Play SD All");
	if (mf_dirOpen==1)
	{
		f_closedir(&dir);
		mf_dirOpen=0;
	}	
	if (DMAAddress==0 && numCubes >1) 
	{
		if (hasSD) playSDAllMultiCubes(); //host 
	}		
	else 
	{
		if (hasSD) playSDAllOneCube();
	}
	Cube_PROGRAM=0;
	modeWaitUp=1;
	delay(50);
}

void playSDSome()
{
	// play some from the SD	
	FRESULT res;
	modeWait=0;
	setName("Play SD Some");
	Cube_PROGRAM=2;
	if (mf_getSemaphore(0))
	{
		if (mf_dirOpen==1)
		{
			f_closedir(&dir);
			mf_dirOpen=0;
		}
		res=mf_load_path_gen();
		if (res == FR_OK)
		{
			res=mf_load_next_file(file,&dir,0);
			SDinUse=4;
			if (res==FR_OK)
			{
				while(2 == Cube_MODE && res == FR_OK && Cube_PROGRAM==2)
				{
					if (power==1)
					{
						checkMusic();
						if (mf_getSemaphore(1))
						{
							mf_read_to_cube(file);
							if (br==0)
							{
								f_close(file);
								res=mf_load_next_file(file,&dir,0);
							}
							SDinUse=0;
							delay(delaySpeed*3.5);
						}
					}
				}
				f_close(file);
			}
			else counterNum = gCounterMax+1; // Stop this graphic
			f_closedir(&dir);
		}
		else counterNum = gCounterMax+1; // Stop this graphic
		delay(50);
		Cube_PROGRAM=0;
	}
}

void playPartSD()
{
	FRESULT res = FR_OK;
	uint8_t state = 0;
	br=1; // do at least one loop !
	if (mf_getSemaphore(0))
	{
		if (!mf_dirOpen) // Only open the directory again if it's been closed
		{		
			res=mf_load_path_gen();
		}
		if (res == FR_OK)
		{
			state=mf_load_next_file(file,&dir,0);
			SDinUse=4;
			while(2 == Cube_MODE && br!=0 && Cube_PROGRAM==2)
			{
				if (mf_getSemaphore(1))
				{
					mf_read_to_cube(file);
					SDinUse=0;
					delay(delaySpeed*3.5);
				}
			}
			f_close(file);
		}
		if (state==0) mf_dirOpen=1;
		else mf_dirOpen=0; // at end of file list	
	}
}

uint16_t countSDFiles()
{
	uint16_t count = 0;
	FRESULT res = FR_OK;
	uint8_t state = 0;
	f_closedir(&dir);
	res=mf_load_path_gen();
  if (res == FR_OK)
	{
		while(state==0)
		// count up through the files
		{
			state=mf_load_next_file(file,&dir,0);
			f_close(file);
			count++;
		}
	}
	return(count);
}

void playPartRandomSD(uint16_t fileNum)
{
	FRESULT res = FR_OK;
	uint8_t state = 0;
	uint16_t count = 0;
	br=1; // do at least one loop !
	if (mf_getSemaphore(0))
	{
		f_closedir(&dir);	
		res=mf_load_path_gen();
		if (res == FR_OK)
		{
			while(state==0 && count<fileNum)
			// count up through the files
			{
				f_close(file);
				state=mf_load_next_file(file,&dir,0);
				count++;
			}
			SDinUse=4;
			while(2 == Cube_MODE && br!=0 && Cube_PROGRAM==2)
			{
				if (mf_getSemaphore(1))
				{
					mf_read_to_cube(file);
					SDinUse=0;
					delay(delaySpeed*3.5);
				}
			}
			f_close(file);
		}
		f_closedir(&dir);
		mf_dirOpen=0;
	}
}

void playSDOne()
{
	// play one from the SD
	Cube_PROGRAM=2;
	modeWait=1;
	setName("Play SD One");
  playPartSD();
	Cube_PROGRAM=0;
	modeWaitUp=1;
	delay(50);
}

void playSDX()
{
	uint16_t i;
	Cube_PROGRAM=2;
	modeWait=1;
	setName("Play SD X");
	// play X number from the SD
	for (i=0;i<numSDPlay;i++)
	{
     playPartSD();
		 if (Cube_PROGRAM != 2) break;
	}
	Cube_PROGRAM=0;
	modeWaitUp=1;
	delay(50);
}

void playSDXcube_random()
{
	uint16_t i;
	uint8_t ran;
	Cube_PROGRAM=2;
	modeWait=1;
	setName("Play SD X Random");	
	if(numSDFiles==0) 
	{
		numSDFiles=countSDFiles();
	}
	ran=cube_random(numSDFiles);
	// play X number from the SD
	for (i=0;i<numSDPlay;i++)		
	{
			playPartRandomSD(ran+i);
		  if (Cube_PROGRAM != 2) break;
	}
	Cube_PROGRAM=0;
	modeWaitUp=1;
	delay(50);
}

void playSDOneEffect()
{
	// play one from the SD
	if (gCounter==1)
	{
		modeWait=1;
		setName("Play SD One Effect");
		randomFade();
	}
	Cube_PROGRAM=2;
  playPartSD();
	Cube_PROGRAM=0;
	modeWaitUp=1;
	delay(50);
}

void playSDXEffect()
{
	uint16_t i;
	if (gCounter==1)
	{
		Cube_PROGRAM=2;
		modeWait=1;
		setName("Play SD X Effect");
		randomFade();
	}
	// play X number from the SD
	for (i=0;i<numSDPlay;i++)
	{
     playPartSD();
		 if (Cube_PROGRAM != 2) break;
	}
	Cube_PROGRAM=0;
	modeWaitUp=1;
	delay(50);
}

void playSDXSlide()
{
	uint16_t i;
	if (gCounter==1)
	{
		Cube_PROGRAM=2;
		modeWait=1;
		setName("Play SD X Slide");
		setEffect("slide",0,0,0);
	}
	// play X number from the SD
	for (i=0;i<numSDPlay;i++)
	{
     playPartSD();
		 if (Cube_PROGRAM != 2) break;
	}
	Cube_PROGRAM=0;
	modeWaitUp=1;
	delay(50);
}

void doPrintClock()
{
	 if (gCounter==1)
   {
			setName("Display Clock");
 	    modeNext=4; // Remain
			modeTime++;
			if (modeTime>1) modeTime=0;
	 }
   printClock();	
}

void blank()
{
	setAll(BLACK);
	setMyAll(BLACK); // In case we are trimmed down
	if (needToClearDMA) clearDMA();
	//delay(50);
}

void resetGraphics()
{
	setLED=0;
	showLED=0;
	ADCRunning=0;
	fadeLevel=fadeMax;
	layerColourSame=0;
	gCounter=0;
	setOtherCubesDMA(0,0); // Default is copy
	blank();
}

void gCleanup()
{
	changeCleanup();
	resetGraphics();
	gCounter=0;
}

uint8_t getNextGraphic()
{
	int8_t graphicOrderTemp = graphicOrderCounter;
	if (modeNext==2) // previous graphic
	{
		graphicOrderTemp--;
	  if (graphicOrderTemp < 0) graphicOrderTemp=graphicOrderMax-1;
	}
	else if (modeNext==0)
	{
		// Current Graphic
	}
	else if (modeNext==3 || modeNext==4) //specific Graphic or stay on graphic
	{
		if (modeNext==4 && modeNextG==79) return(graphicOrder[graphicOrderCounter]);
		return(modeNextG);
	}
	else
	{
		graphicOrderTemp++;
		if (graphicOrderTemp >= graphicOrderMax) graphicOrderTemp=0;
  }
	return(graphicOrder[graphicOrderTemp]);
}

void changeCleanup()
{
	  if (Cube_PROGRAM==2) Cube_PROGRAM=0;
		setOtherCubesDMA(0,0);
	  showLED=3;
		counterNum=0;
	  modeWait=0;
    modeWaitUp=0;
	  eCounter=0;
	  eCounter2=0;
	  setAdd=0;
	  eNum=0;
	  eFadeout=0;
	  eFadeoutDone=0;
	  setLED=0;
		blank(); // In case we are trimmed down
  	setAll(BLACK);
	  setLED=1;
  	setAll(BLACK);
	  setLED=2;
	  setAll(BLACK);
	  setLED=0;
}

void nextGraphic()
{
	  if (modeNext!=3) // don't change if still going to specific graphic
		{
			changeCleanup();
			modeNext=1;
			if (modeText>=10) modeText=0; // Reset text if displaying from web page
			if (DMAAddress==0 && hostReAnnounce==3) 
			{
				hostAnnounce();
			}
		}
}

void prevGraphic()
{
		if (modeNext!=3) 
		{
			changeCleanup();
			if (modeText>=10) modeText=0; // Reset text if displaying from web page
			modeNext=2;	
		}
}

void specificGraphic()
{
		changeCleanup();
		modeNext=3;
}

void checkUDP(void)
{
	if (WIFICopyMode==1) 
	{
		// Let them know fade out has started
		lastKey=254;
		//setupSharedVariables();
		sendUDP=1;
	}	
}	
			
void checkFade()
{
	 if (eFadeout==0 || !doTransitions || eFadeoutDone==3)
	 {
		  if (WIFICopyMode==2 && shared2!=255) showLED=3; // Don't move to next graphic if getting UDP from host
			else nextGraphic(); 
	 }
	 if (eFadeout==1)
	 {
		 // roll your own fade out. You must set it !
		 if (counterNum==gCounterMax+1) {eFadeoutDone=1; checkUDP();}
	 }	
   if (eFadeout==2)
	 {
 		 if (counterNum==gCounterMax+1 || modeWait) {eFadeoutDone=1; checkUDP(); eNum = eNum & 45053; setEffect("fadeout",0,0,0);} //and stop fadein,undissolve,fadetrail if still running
	 }
	 if (eFadeout==3)
	 {
		 if (counterNum==gCounterMax+1 || modeWait) {eFadeoutDone=1; checkUDP(); eNum = eNum & 45053; setEffect("slide",0,0,0);} //and stop fadein,undissolve,fadetrail if still running
	 }	
	 if (eFadeout==4)
	 {
		 if (counterNum==gCounterMax+1 || modeWait) {eFadeoutDone=1; checkUDP(); eNum = eNum & 45053; setEffect("dissolve",0,0,0);} //and stop fadein,undissolve,fadetrail if still running
	 }	
	 if (eFadeout>4)
	 {
		  setAll(GREEN); delay(1000); eFadeoutDone=3;
	 }
}

void controlGraphics()
{
	 doEffect();
	 if (modeWait && allowModeWait)
	 {	
	    if (modeWaitUp)
			{
					if(modeNext!=4) nextGraphic();
				  else 
					{
							gCounter=0;
						  eNum=0;
							showLED=0;
							setLED=0;
							modeWaitUp=0;
					}
			}								
		}
		else
		{
				counterNum++;
			  if (counterNum > gCounterMax || modeWaitUp) // don't wait forever on ModeWait - whatever comes first
				{
            if (modeNext!=4) checkFade(); // don't move to next if remain is on
				}
		}
}

void loop() 
{
	uint16_t timeout=0;
  if(power==1)
	{
	if (modeNext==1) // next graphic
	{
		changeCleanup(); // Do it again incase stuff was changed in the middle
	  gCounter=0;
		modeNext=0;
		modeNextG=79;
		showLED=0;
		setLED=0;
		fadeLevel = fadeMax;
		if (welcome)
		{
			welcome++;
			if (welcome>3) 
			{
				if (DMAAddress==0 && hostReAnnounce>0)
				{	
					// Master - Send Broadcast
					hostAnnounce();
				}
				welcome = 0;
			}
		}
		else
		{
				graphicOrderCounter++;
				if (graphicOrderCounter >= graphicOrderMax)
            // At the end of the Graphics Array					
				{
				   if (shuffleGraphics == 3)
						// Change the order of the graphics in the array
					 {
							randomize(graphicOrder,graphicOrderMax);
					 }
					 graphicOrderCounter=0;
				}
				modeC=graphicOrder[graphicOrderCounter];
		}
		if (WIFICopyMode==1) 
		{
			lastKey=255;
			setupSharedVariables();
			sendUDP=1;
		}		
	}  
	if (modeNext==2) // previous graphic
	{
		changeCleanup(); // Do it again incase stuff was changed in the middle
	  gCounter=0;
		modeNext=0;
		modeNextG=79;
		showLED=0;
		setLED=0;
		fadeLevel = fadeMax;
		welcome=0; // just in case backward in intro
		graphicOrderCounter--;
	  if (graphicOrderCounter < 0) graphicOrderCounter=graphicOrderMax-1;
		modeC=graphicOrder[graphicOrderCounter];
		if (WIFICopyMode==1) 
		{
			lastKey=255;
			setupSharedVariables();
			sendUDP=1;
		}		
	}  
	if (modeNext==3) // specific graphic
	{
		changeCleanup(); // Do it again incase stuff was changed in the middle
	  gCounter=0;
		if (WIFICopyMode==2 && shared2!=255) modeNext=4; // remain on this graphic for WIFI followers on UDP
		else modeNext=0;
		showLED=0;
		setLED=0;
		fadeLevel = fadeMax;
		welcome=0; // just in case backward in intro
		modeC=modeNextG;		
		graphicOrderCounter=findGraphic(modeC,graphicOrderMax); // find the graphic so ff and rew work from here
		modeNextG=79;
		if (WIFICopyMode==1) 
		{
			lastKey=255;
			setupSharedVariables();
			sendUDP=1;
		}		
	}
	if (WIFICopyMode==1)
	{
		  if (sendUDP!=0 && !(counterNum > gCounterMax || modeWaitUp)) // Only delay on start of graphics - Don't care about fade out
			{
				while(sendUDP!=0 && timeout<2000) // Wait for the other cubes to get the message
				{
					timeout++;
				}
				delay(160);
			}
	}
	
	gCounter++;

	if (welcome)
	{
		  if (welcome==2 && ledCubeType==3) welcome=3; // skip text spin - not enough room on Wide 12
		  if (gCounter==1) setOtherCubesDMA(0,6); // Check if we have extend cubes or not
			if (welcome==1 && !extendCubes) textWelcome();  //textWelcome
		  if (welcome==1 && extendCubes) textWelcomeScroll();
	    if (welcome==2) textCube();
		  if (welcome==3) doDateTime();
		  if (Xmas)
			{
				if (Xmas==1 || month==12)
				{
					modeC=14;
				}
			}
	}
	else
	{
		  if (modeC==0) doDateTime();
    	if (modeC==1) doTime();
			if (modeC==2) mBoxes();
			//if (modeC==2) testColours(); //TESTING ONLY
			if (modeC==3) sineWave();
		  if (modeC==4) flyIn();
      if (modeC==5) drawSpheres();
	    if (modeC==6) bounceSphere();	
	    if (modeC==7) bounceSphere2();	
      if (modeC==8) migrate();	
			if (modeC==9) spin();
      if (modeC==10) doPlasma(); 
			if (modeC==11) doSwarm();
	    if (modeC==12) runSpiral2();
		  if (modeC==13) migrateThreeWay();
		  if (modeC==14) randomPastel();
		  if (modeC==15) simulateLife();
			if (modeC==16) conway3DLife();
			if (modeC==17) shapes();
			if (modeC==18) boxes();	
			if (modeC==19) spinBoxes();
			if (modeC==20) sineWaveTwo();		
			if (modeC==21) sineWavePlus();
  		// Control Modes SD etc
			if (modeC==80) doFFT1();
			if (modeC==81) doFFT2();			
			if (modeC==82) doFFT3();
			if (modeC==83) doFFT4();
			if (modeC==84) doFFT5();
			if (modeC==89) doPrintClock();
			if (modeC==90) playSDAll();
  		if (modeC==91) playSDSome();
			if (modeC==92) playSDOne();
			if (modeC==93) playSDOneEffect();
			if (modeC==94) playSDX();
			if (modeC==95) playSDXEffect();
			if (modeC==96) playSDXSlide();
			if (modeC==97) playSDXcube_random();
			if (modeC==98) loopReadText();
			if (modeC==99) loopMood();
			if ((modeC>=graphicsMax && modeC <80) || modeC>99)
			{
				// unknown mode
				setAll(RED);
				delay(2000);
				modeC=2;
				graphicOrderCounter=0;
			}
	}
 }
}

void loopMoodNormal()
{
	uint8_t R,G,B;	
	uint8_t x;
	uint16_t dSpeed;
	uint8_t nextR,nextG,nextB;	
	uint8_t diffR,diffG,diffB;	
	if (gCounter<2)
  {	
	  setName("Mood Light");
		gCounter++;
	}
	get_colour(colourPos,&R,&G,&B);
	//setAll(R,G,B);
	colourPos++;
	if (colourPos> COLOUR_WHEEL_LENGTH) colourPos=0;
	get_colour(colourPos,&nextR,&nextG,&nextB);
	dSpeed=delaySpeed*3;
	for (x=0;x<dSpeed;x++)
	{
		diffR=R+(float)((float)(nextR-R)/dSpeed);
		diffG=G+(float)((float)(nextG-G)/dSpeed);
		diffB=B+(float)((float)(nextB-B)/dSpeed);		
		
		//diffR=R+((nextR-R)/dSpeed);
		//diffG=G+((nextG-G)/dSpeed);
		//diffB=B+((nextB-B)/dSpeed);		
		
		diffR=diffR>>(4-customBright);
		diffG=diffG>>(4-customBright);
		diffB=diffB>>(4-customBright);
  	setAll(diffR,diffG,diffB);
		if (ledCubeType<2) delay(10*delaySpeed); // Aura12 is slowed already with layer timings
	}
}

void rgbLedRainbow(int delayVal, int maxBrightness, int rainbowWidth, int dir)
{
  // Displays a rainbow spread over all LED's, which shifts in hue.
 uint8_t hue, sat, val; 
 uint8_t red, green, blue;
 uint8_t layer;
 if (gCounter<2)
 {
   setName("Mood Rainbow");
	 gCounter++;
 }
 counterNum++;
 if (counterNum>=360) counterNum=0;
 // Shift over full color range (like the hue slider in photoshop)
 for(layer=0;layer<12;layer++)
 { // loop over all y Layers
    hue = ((layer)*360/(rainbowWidth-1)+counterNum)%360; // Set hue from 0 to 360 from first to last led and shift the hue
    sat = 255;
    val = 255;
    hsvToRGB(hue, sat, val, &red, &green, &blue, maxBrightness); // convert hsv to rgb values
		if (dir==0) setPlaneY(layer,red,green,blue);
		else setPlaneX(layer,red,green,blue);
  }
  if (ledCubeType<2) delay(delaySpeed*delayVal);
	else delay((delaySpeed/2)*delayVal);
}

void loopMood() 
{
	if (modeMood>4) modeMood=0; //shouldn't need this
	if (modeMood==0) loopMoodNormal();
	if (modeMood==1) rgbLedRainbow(3, 255, 100,0); // Fast, over all LED's
	if (modeMood==2) rgbLedRainbow(3, 255, 100,1); // Fast, over all LED's
	if (modeMood==3) rgbLedRainbow(3, 255, 800,0); // Slower, over all LED's
	if (modeMood==4) rgbLedRainbow(3, 255, 800,1); // Slower, over all LED's
}

char getSpecialChar(char* charPtr)
{
	char c1=0;
	char c2=0;
	char returnChar=0;
	c1=charPtr[0];
	c2=charPtr[1];
	if (c1=='2' && c2=='0') returnChar=' ';
	if (c1=='3' && c2=='C') returnChar='<';
	if (c1=='3' && c2=='E') returnChar='>';
	if (c1=='2' && c2=='3') returnChar='#';
	if (c1=='2' && c2=='5') returnChar='%';
	if (c1=='7' && c2=='B') returnChar='{';
	if (c1=='7' && c2=='D') returnChar='}';
	if (c1=='7' && c2=='C') returnChar='|';
	if (c1=='5' && c2=='C') returnChar='\\';
	if (c1=='5' && c2=='E') returnChar='^';
	if (c1=='7' && c2=='E') returnChar='~';
	if (c1=='5' && c2=='B') returnChar='[';
	if (c1=='5' && c2=='D') returnChar=']';
	if (c1=='2' && c2=='7') returnChar='\'';
	if (c1=='6' && c2=='0') returnChar='\'';
	if (c1=='3' && c2=='B') returnChar=';';
	if (c1=='2' && c2=='F') returnChar='/';
	if (c1=='3' && c2=='F') returnChar='?';
	if (c1=='3' && c2=='A') returnChar=':';
	if (c1=='4' && c2=='0') returnChar='@';
	if (c1=='3' && c2=='D') returnChar='=';
	if (c1=='2' && c2=='6') returnChar='&';
	if (c1=='2' && c2=='4') returnChar='$';
	if (c1=='2' && c2=='B') returnChar='+';
	return returnChar;
}

void printChar(uint8_t mode,char c)
{
	if (mode==10) 
	{
		if (ledCubeType<2 && c>='a' && c<='z') c-=32; // convert to upper case
		writeChar(c,0);
	}
	else
	{
		if (ledCubeType<2 && c>='a' && c<='z') c-=32; // convert to upper case
		scrollReadText(c);
	}
}

void loopReadText(void)
{
	uint8_t res=0;
	char c;
	if (gCounter==1 || (Cube_MODE==7 && gCounter==2))
	{
		setName("Display Text 1");
		modeWait=1;
		initScrollText(0);
   	beginText();
		if (modeText<10) // Don't do for specific text
		{
			modeText=0;
			mf_openText("0:\\text1.txt",0);
		}
		else if (modeText==11 || modeText==12) setOtherCubesDMA(0,6); // Each cube to its own! for scroll
		setAll(BLACK);
	}
	if (modeText>=10)
	{
		c=tcpRxBuffer[TCP_RX_BUFFER_LEN-101+gCounter];
		if (c=='\0' || c=='&') res=1;
		else if (c=='%') 
		{
			c=getSpecialChar(&tcpRxBuffer[TCP_RX_BUFFER_LEN-100+gCounter]);
			gCounter+=2; // skip these control chars
			if (c!=0) printChar(modeText,c);
		}
		else if (c=='+') printChar(modeText,' ');	
		else printChar(modeText,c);	
	}
	else 
	{
		res=mf_readText(modeText);
	}
	if (res && modeText<10)
	{
		if (modeText==0) 
		{
			modeText=1;
			setOtherCubesDMA(0,6); // Each cube to its own! for scroll
			setName("Display Text 2");
			initScrollText(0);
			beginText();
			setAll(BLACK);
			mf_openText("0:\\text2.txt",1);
		}
		else
		{
			if (Cube_MODE!=7) modeWaitUp=1;
			else gCounter=1;
		}
	}
	else
	{
		if (res) 
		{
			if (modeText==11 || modeText==12) 
			{
				scrollFinished=1;
				while (scrollReadText(' ')==0);
			}
			if (msgWait==0)	modeWaitUp=1;
			else 
			{
				if (modeText==10) delay(2000);
				gCounter=0;
			}
		}
	}
}

void loopSlave() 
{
	uint8_t R = 0;
  uint8_t G = 0;
	uint8_t B = 0;
	uint16_t i=0;
	uint8_t command;
	uint16_t count_i = 0,count_j = 0;
  uint8_t x,y,z=0;
	// swap between different cubes
	command=DMACommands[DMAAddress];
		
 // for(x=0;x<255;x++)
	//  {
	//  	set(0,0,0,x,0,0);
	//  }
  //set (0,0,0,255,0,0);
		
	if (millis()-old_millis>10000 || (millis()-old_millis>4000 && welcome>=20)) // wait 10 seconds (or 4 seconds for welcome)
	{
		if (welcome>=20) 
		{
			if (foundHost>0)
			{
				Cube_MODE=6; // stay in slave mode
			}
			welcome-=20; // Welcome still exit out of slave mode
			// reset counter for another x seconds
			old_millis=millis();
			oldFoundHost=foundHost;
		}
		else
		{
			// Exit out if Host Gone
			if (foundHost==oldFoundHost) // found host hasn't incremented - No more DMA
			{
					 foundHost=0;
					 Cube_MODE=OldCube_MODE;
					 setAll(BLACK);																	
					 resetGraphics();
					 DMA_Stop();
				   //stopDMA(); // Just in case host still there - Stop will happen anyway
			}
			else
			{
				// reset counter for another x seconds
				old_millis=millis();
				oldFoundHost=foundHost;
			}
		}
	}
	if (welcome>=10 && welcome<20)
	{
		old_millis=millis();
		welcome+=10;  // don't store inital time again
	}
	for (x=0;x<myTotalX;x++)
		{
			for (y=0;y<myTotalY;y++)
			{
			  for (z=0;z<myTotalZ;z++)
				{
					if(DMATranslate==1) // Cube 8 Host that needs translation
					{
						  // CUBE8 HOST
						  if (x>7||y>7||z>7) continue; // Off cube 8
						  if (rgb_order_transfer[0]==0) 	initRGBOrder(); // Needed for Cube Transfers
							count_i = (7-x)*3+z*24;									
							count_j = 192*(7-y);										
							R = rgbled[0][(rgb_order_transfer[count_i+0]+count_j)]; 
							G = rgbled[0][(rgb_order_transfer[count_i+1]+count_j)];
							B = rgbled[0][(rgb_order_transfer[count_i+2]+count_j)];		
							if (command==0||command==6) set(x,y,z,R,G,B);   // Normal
							if (command==1) set(7-x,y,z,R,G,B); // Mirror Image
							if (command==2) set(x,7-y,z,R,G,B); // Upside Down
							if (command==3) set(x,y,7-z,R,G,B);	// Inside Out				
							if (command==4) set(7-x,7-y,7-z,R,G,B);	// All Backwards		
							if (command==5) set(x,y,z,R,G,B);
							// fix stupid out of alignment pattern
						  i=5000;
						  i=7000;
						  i=6000;
					}						
					else
					{
						  // CUBE12 HOST
							count_i = x*3+z*(myTotalX*3); 
							count_j = ((myTotalX*myTotalZ)*3)*((myTotalY-1)-y);  
						
							R = rgbled[0][count_i+0+count_j]; // No Order with Cube12
							G = rgbled[0][count_i+1+count_j]; // No Order with Cube12
							B = rgbled[0][count_i+2+count_j]; // No Order with Cube12
							if (command==0||command==6) set(x,y,z,R,G,B);   // Normal
							if (command==1) set((myTotalX-1)-x,y,z,R,G,B); // Mirror Image
							if (command==2) set(x,(myTotalY-1)-y,z,R,G,B); // Upside Down
							if (command==3) set(x,y,(myTotalZ-1)-z,R,G,B);	// Inside Out				
							if (command==4) set((myTotalX-1)-x,(myTotalY-1)-y,(myTotalZ-1)-z,R,G,B);	// All Backwards		
							if (command==5) set(x,y,z,R,G,B);
					}
				}
 		}
	}
	if (command==5)
	{
	  for(x=0;x<255;x++)
	  {
	  	set(0,0,0,x,0,0);
	  }
	}
}

void executeCommand(char* option, char* value)
{
		char temp[3];
		if (strncmp(option,"ledCubeType",11)==0)
		{
		 	ledCubeType=atoi(value);
			if (ledCubeType==10) 
			{
				// Newer Cube8 Hardware. Fudge to avoid renumber
				ledCubeType=1;
				bjCubeType=2;
			}
		}
		if (strncmp(option,"cubeMode",8)==0)
		{
			Cube_MODE=atoi(value);
		}
		if (strncmp(option,"cubeWIFI",8)==0)
		{
			cubeWIFI=atoi(value);
		}		
		if (strncmp(option,"wifiSSID",8)==0)
		{
			strncpy(wifiSSID,value,32);
			wifiSSID[strlen(value)-2]='\0'; // strip newline
		}		
		if (strncmp(option,"wifiPSWD",8)==0)
		{
			strncpy(wifiPSWD,value,32);
			wifiPSWD[strlen(value)-2]='\0';
		}				
		if (strncmp(option,"wifiIP",6)==0)
		{
			strncpy(myIPAddress,value,16);
			myIPAddress[strlen(value)-2]='\0';
		}
		if (strncmp(option,"wifiGW",6)==0)
		{
			strncpy(myGW,value,16);
			myGW[strlen(value)-2]='\0';
		}
		if (strncmp(option,"wifiMask",8)==0)
		{
			strncpy(myMask,value,16);
			myMask[strlen(value)-2]='\0';
		}		
		if (strncmp(option,"NTPServer",9)==0)
		{
			strncpy(NTPServer,value,32);
			NTPServer[strlen(value)-2]='\0';
		}				
		if (strncmp(option,"UTCOffsetH",10)==0)
		{
			UTCOffsetH=atoi(value);
		}				
		if (strncmp(option,"UTCOffsetM",10)==0)
		{
			UTCOffsetM=atoi(value);
		}				
		if (strncmp(option,"dSavings",8)==0)
		{
			dSavings=atoi(value);
		}				
		if (strncmp(option,"dateFormat",10)==0)
		{
			dateFormat=atoi(value);
		}						
		if (strncmp(option,"wifiTimeoutMult",15)==0)
		{
			wifiTimeoutMult=atoi(value);
		}							
		if (strncmp(option,"delaySpeed",10)==0)
		{
			delaySpeed=atoi(value);
		}
		if (strncmp(option,"customBright",12)==0)
		{
			customBright=atoi(value);
		}
		if (strncmp(option,"power",5)==0)
		{
			power=atoi(value);
		}					
		if (strncmp(option,"fadeMax",7)==0)
		{
			fadeMax=atoi(value);
		}
		if (strncmp(option,"cubeDirection",13)==0)
		{
			cubeDirection=atoi(value);
		}
		if (strncmp(option,"AuraCubes",9)==0)
		{
			AuraCubes=atoi(value);
		}
		if (strncmp(option,"gCounterMax",11)==0)
		{
			gCounterMax=atoi(value);
		}
		if (strncmp(option,"graphicTime",11)==0)
		{
			gCounterMax=atoi(value)*timeMultipler; // convert from seconds to interupt ticks
		}
		if (strncmp(option,"welcome",7)==0)
		{
			welcome=atoi(value);
		}
		if (strncmp(option,"allowModeWait",13)==0)
		{
			allowModeWait=atoi(value);
		}
		if (strncmp(option,"doTransitions",13)==0)
		{
			doTransitions=atoi(value);
		}
		if (strncmp(option,"transitionSpeed",15)==0)
		{
			transitionSpeed=atoi(value);
		}	
		if (strncmp(option,"graphicOrder",12)==0)
		{
		  char *token;
			const char s[2] = ","; // comma separated
			/* get the first token */
			token = strtok(value, s);

			/* walk through other tokens */
			while( token != NULL ) 
			{
					graphicOrder[graphicOrderMaxTemp] = atoi(token);
					token = strtok(NULL, s);
					graphicOrderMaxTemp++;
			}
			graphicOrderMax=graphicOrderMaxTemp;
		}
		if (strncmp(option,"shuffleGraphics",15)==0)
		{
				shuffleGraphics=atoi(value);
		}
		if (strncmp(option,"numSDPlay",9)==0)
		{
				numSDPlay=atoi(value);
		}
		if (strncmp(option,"lifeStartParents",16)==0)
		{
				lifeStartParents=atoi(value);
		}
		if (strncmp(option,"lifeMarryRate",13)==0)
		{
				lifeMarryRate=atoi(value);
		}
		if (strncmp(option,"lifeDeathRate",13)==0)
		{
				lifeDeathRate=atoi(value);
		}
		if (strncmp(option,"lifeChildYears",14)==0)
		{
				lifeChildYears=atoi(value);
		}
		if (strncmp(option,"ESP8266BAUD",11)==0)
		{
				ESP8266BAUD=atoi(value);
		}
		if (strncmp(option,"ESP8266CHANGE",13)==0)
		{
				ESP8266CHANGE=atoi(value);
		}
		if (strncmp(option,"cubeAddress",11)==0)
		{
				DMAAddress=atoi(value);
		}		
		if (strncmp(option,"swapCubes",9)==0)
		{
				swapCubes=atoi(value);
		}			
		if (strncmp(option,"conwayIteration",15)==0)
		{
				conwayIteration=atoi(value);
		}			
		if (strncmp(option,"hostReAnnounce",14)==0)
		{
				hostReAnnounce=atoi(value);
		}			
		if (strncmp(option,"hostCopyMode",12)==0)
		{
				hostCopyMode=atoi(value);
		}			
		if (strncmp(option,"WIFICopyMode",12)==0)
		{
				WIFICopyMode=atoi(value);
		}		
		if (strncmp(option,"audioIn",12)==0)
		{
				ADC_MODE=atoi(value);
		}		
		if (strncmp(option,"dateTime",8)==0)
		{
			 strncpy(SDDateTime,value,13);
			 SDDateTime[13]='\0';
			 temp[0]=SDDateTime[3]; // Store the month in case 12 for xmas
	     temp[1]=SDDateTime[4];		
	     month=atoi(temp);
		}
		if (strncmp(option,"Xmas",4)==0)
		{
			 Xmas=atoi(value);
		}
		if (strncmp(option,"lcdType",7)==0)
		{
			 LCDType=atoi(value);
		}		
		if (strncmp(option,"tempUnit",8)==0)
		{
			 tempUnit=atoi(value);
		}
		if (strncmp(option,"redOrder",8)==0)
		{
			 ROffset=atoi(value);
		}
		if (strncmp(option,"greenOrder",10)==0)
		{
			 GOffset=atoi(value);
		}		
		if (strncmp(option,"blueOrder",9)==0)
		{
			 BOffset=atoi(value);
		}		
		// extra commands for Cube12 Only
    executeComExtend(option, value);
}
