#include "Text.h"

uint8_t scrollFinished = 0;

const char words[NUM_WORDS][MAX_WORD_LENGTH] = {
    "WELCOME TO ", 
};

const uint8_t alphabet[26][8] = 
{{0x18,0x3C,0x42,0xC3,0xFF,0xC3,0xC3,0xC3} /*A*/
,{0x3F,0xC3,0xC3,0x3F,0x43,0xC3,0x43,0x3F} /*B*/
,{0xFC,0xFE,0x3,0x3,0x3,0x3,0xFE,0xFC} /*C*/
,{0x1F,0x7F,0xE3,0xC3,0xC3,0xE3,0x7F,0x1F} /*D*/					
,{0xFF,0x3,0x3,0xFF,0xFF,0x3,0x3,0xFF} /*E*/
,{0xFF,0xFF,0x3,0x3,0x1F,0x3,0x3,0x3} /*F*/
,{0xFF,0xFF,0x3,0xF3,0xC3,0xC3,0xFF,0xFF} /*G*/
,{0xC3,0xC3,0xC3,0xFF,0xFF,0xC3,0xC3,0xC3} /*H*/
,{0xFF,0xFF,0x18,0x18,0x18,0x18,0xFF,0xFF} /*I*/
,{0xFF,0xFF,0x60,0x60,0x60,0x63,0x7F,0x3E} /*J*/
,{0xE3,0x33,0x1B,0xF,0xF,0x1B,0x33,0xE3} /*K*/
,{0x3,0x3,0x3,0x3,0x3,0x3,0xFF,0xFF} /*L*/
,{0x81,0xC3,0xE7,0xFF,0xDB,0xC3,0xC3,0xC3} /*M*/
,{0xC3,0xC7,0xCF,0xDB,0xF3,0xE3,0xC3,0xC3} /*N*/
,{0xFF,0xFF,0xC3,0xC3,0xC3,0xC3,0xFF,0xFF} /*O*/
,{0x7F,0xFF,0xC3,0xC3,0x7F,0x3,0x3,0x3} /*P*/
,{0x1C,0x3E,0x63,0x63,0x63,0x22,0x7E,0x9C} /*Q*/
,{0x3F,0xC3,0xC3,0xC3,0x3F,0x33,0x63,0xC3} /*R*/
,{0xFE,0x3,0x3,0x3E,0x60,0xC0,0x60,0x3F} /*S*/
,{0xFF,0xFF,0x18,0x18,0x18,0x18,0x18,0x18} /*T*/
,{0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0xFF,0x3C} /*U*/
,{0xC3,0xC3,0xC3,0xC3,0xC3,0x66,0x3C,0x18} /*V*/
,{0xC3,0xC3,0xC3,0xDB,0xFF,0xE7,0xC3,0x81} /*W*/
,{0xC3,0x66,0x3C,0x18,0x18,0x3C,0x66,0xC3} /*X*/
,{0xC3,0xC3,0xC3,0x42,0x7E,0x18,0x18,0x18} /*Y*/
,{0xFF,0x7F,0x30,0x18,0xC,0x6,0xFF,0xFF} /*Z*/
};


const char words7[NUM_WORDS][MAX_WORD_LENGTH] = {
    "AURACUBE ", 
};

const char rgbwords7[NUM_WORDS][MAX_WORD_LENGTH] = {
    "RGB CUBE ", 
};

const char words7multi[NUM_WORDS][MAX_WORD_LENGTH] = {
    "XCUBES", 
};

const uint8_t alphabet7[45][8] = 
{{0x0,0x0,0x0,0x0,0x0,0x0,0xC,0xC} /*.*/
,{0x20,0x30,0x10,0x18,0xC,0x6,0x3,0x1} /*slash*/
,{0x3F,0x21,0x21,0x21,0x21,0x21,0x21,0x3F} /*0*/
,{0xC,0xA,0x9,0x8,0x8,0x8,0x8,0x8} /*1*/					
,{0x3F,0x20,0x20,0x3F,0x1,0x1,0x1,0x3F} /*2*/
,{0x3F,0x20,0x20,0x3F,0x20,0x20,0x20,0x3F} /*3*/
,{0x18,0x14,0x12,0x11,0x3F,0x10,0x10,0x10} /*4*/
,{0x3F,0x1,0x1,0x3F,0x20,0x20,0x20,0x3F} /*5*/					
,{0x3F,0x1,0x1,0x3F,0x21,0x21,0x21,0x3F} /*6*/					
,{0x3F,0x30,0x18,0xC,0x6,0x2,0x3,0x1} /*7*/
,{0x3F,0x21,0x21,0x3F,0x21,0x21,0x21,0x3F} /*8*/
,{0x3F,0x21,0x21,0x3F,0x20,0x20,0x20,0x3F} /*9*/
,{0x0,0xC,0xC,0x0,0x0,0xC,0xC,0x0} /*:*/					
,{0x0,0x0,0xC,0xC,0x0,0xC,0x4,0x2} /*;*/
,{0x30,0x8,0x6,0x1,0x1,0x6,0x8,0x30} /*<*/
,{0x0,0x0,0x3F,0x0,0x0,0x3F,0x0,0x0} /*=*/
,{0x3,0x4,0x18,0x20,0x20,0x18,0x4,0x3} /*>*/
,{0x1F,0x20,0x20,0x20,0x1C,0x0,0xC,0xC} /*?*/
,{0x3E,0x41,0x41,0x5D,0x65,0x3D,0x1,0x3E} /*@*/
,{0xC,0x12,0x21,0x21,0x3F,0x21,0x21,0x21} /*A*/
,{0x1F,0x21,0x21,0x1F,0x21,0x21,0x21,0x1F} /*B*/
,{0x3E,0x3,0x1,0x1,0x1,0x1,0x3,0x3E} /*C*/
,{0x1F,0x21,0x21,0x41,0x41,0x21,0x21,0x1F} /*D*/
,{0x3F,0x1,0x1,0x3F,0x1,0x1,0x1,0x3F} /*E*/				
,{0x3F,0x1,0x1,0x1,0xF,0x1,0x1,0x1} /*F*/
,{0x3F,0x1,0x1,0x1,0x39,0x21,0x21,0x3F} /*G*/
,{0x21,0x21,0x21,0x21,0x3F,0x21,0x21,0x21} /*H*/
,{0x7F,0x8,0x8,0x8,0x8,0x8,0x8,0x7F} /*I*/
,{0x7F,0x10,0x10,0x10,0x10,0x11,0x11,0x1F} /*J*/
,{0x11,0x9,0x5,0x3,0x5,0x9,0x11,0x21} /*K*/
,{0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x3F} /*L*/
,{0x41,0x63,0x55,0x49,0x41,0x41,0x41,0x41} /*M*/
,{0x41,0x43,0x45,0x49,0x51,0x51,0x61,0x41} /*N*/
,{0x3F,0x21,0x21,0x21,0x21,0x21,0x21,0x3F} /*O*/
,{0x1F,0x21,0x21,0x21,0x1F,0x1,0x1,0x1} /*P*/
,{0xC,0x33,0x21,0x21,0x21,0x29,0x33,0x4C} /*Q*/
,{0x1F,0x21,0x21,0x21,0x1F,0x9,0x11,0x21} /*R*/				
,{0x3E,0x1,0x1,0x1E,0x20,0x20,0x20,0x1F} /*S*/			
,{0x7F,0x8,0x8,0x8,0x8,0x8,0x8,0x8} /*T*/
,{0x21,0x21,0x21,0x21,0x21,0x21,0x21,0x1E} /*U*/
,{0x21,0x21,0x21,0x21,0x21,0x21,0x12,0xC} /*V*/
,{0x41,0x41,0x41,0x41,0x49,0x55,0x63,0x41} /*W*/
,{0x41,0x22,0x14,0x8,0x8,0x14,0x22,0x41} /*X*/
,{0x21,0x21,0x21,0x21,0x3F,0x20,0x10,0xF} /*Y*/
,{0x3F,0x20,0x10,0x8,0x4,0x2,0x1,0x3F} /*Z*/
};

const uint8_t alphabet5[26][8] =
{{0x0,0x1C,0x22,0x3E,0x22,0x22,0x0,0x0} /*A*/
,{0x0,0x1E,0x22,0x1E,0x22,0x1E,0x0,0x0} /*B*/
,{0x0,0x3E,0x2,0x2,0x2,0x3E,0x0,0x0} /*C*/
,{0x0,0xE,0x12,0x22,0x12,0xE,0x0,0x0} /*D*/
,{0x0,0x3E,0x2,0x3E,0x2,0x3E,0x0,0x0} /*E*/
,{0x0,0x3E,0x2,0x1E,0x2,0x2,0x0,0x0} /*F*/
,{0x0,0x3E,0x2,0x32,0x22,0x3E,0x0,0x0} /*G*/
,{0x0,0x22,0x22,0x3E,0x22,0x22,0x0,0x0} /*H*/
,{0x0,0x3E,0x8,0x8,0x8,0x3E,0x0,0x0} /*I*/
,{0x0,0x7E,0x10,0x10,0x12,0x1E,0x0,0x0} /*J*/
,{0x0,0x22,0x12,0xE,0x12,0x22,0x0,0x0} /*K*/
,{0x0,0x2,0x2,0x2,0x2,0x3E,0x0,0x0} /*L*/
,{0x0,0x22,0x36,0x2A,0x22,0x22,0x0,0x0} /*M*/
,{0x0,0x22,0x26,0x2A,0x32,0x22,0x0,0x0} /*N*/
,{0x0,0x3E,0x22,0x22,0x22,0x3E,0x0,0x0} /*O*/
,{0x0,0x1E,0x22,0x1E,0x2,0x2,0x0,0x0} /*P*/
,{0x0,0x1E,0x12,0x12,0x1E,0x20,0x0,0x0} /*Q*/
,{0x0,0x1E,0x22,0x1E,0xA,0x32,0x0,0x0} /*R*/
,{0x0,0x3E,0x2,0x3E,0x20,0x3E,0x0,0x0} /*S*/
,{0x0,0x3E,0x8,0x8,0x8,0x8,0x0,0x0} /*T*/
,{0x0,0x22,0x22,0x22,0x22,0x1C,0x0,0x0} /*U*/
,{0x0,0x22,0x22,0x22,0x14,0x8,0x0,0x0} /*V*/
,{0x0,0x22,0x22,0x2A,0x36,0x22,0x0,0x0} /*W*/
,{0x0,0x22,0x14,0x8,0x14,0x22,0x0,0x0} /*X*/
,{0x0,0x22,0x14,0x8,0x8,0x8,0x0,0x0} /*Y*/
,{0x0,0x3E,0x10,0x8,0x4,0x3E,0x0,0x0} /*Z*/
};

const char wordsScroll[NUM_WORDS_SCROLL][MAX_WORD_LENGTH] = {
    {"WELCOME "},
		{"TO "} 
};

const uint8_t wordsScrollCol[NUM_WORDS_SCROLL][NUM_CHARS_SCROLL][3] = {
{
{PINK},
{GREEN},
{PURPLE},
{ORANGE},
{AQUA},
{RED},
{YELLOW}
},
{
{BLUE},
{WHITE}
}
};

/*
{{0x18,0x24,0x42,0x42,0x7E,0x42,0x42,0x42} //A
,{0x3E,0x42,0x42,0x3E,0x42,0x42,0x42,0x3E} //B
,{0x7C,0x2,0x2,0x2,0x2,0x2,0x2,0x7C} //C
,{0x3E,0x42,0x42,0x42,0x42,0x42,0x42,0x3E} //D
,{0x7E,0x2,0x2,0x3E,0x2,0x2,0x2,0x7E} //E
,{0x7E,0x2,0x2,0x2,0x1E,0x2,0x2,0x2} //F
,{0x7E,0x2,0x2,0x2,0x72,0x42,0x42,0x7E} //G
,{0x42,0x42,0x42,0x42,0x7E,0x42,0x42,0x42} //H
,{0x3E,0x8,0x8,0x8,0x8,0x8,0x8,0x3E} //I
,{0x7E,0x20,0x20,0x20,0x20,0x22,0x22,0x3E} //J
,{0x22,0x12,0xA,0x6,0xA,0x12,0x22,0x42} //K
,{0x2,0x2,0x2,0x2,0x2,0x2,0x2,0x7E} //L
,{0x81,0xC3,0xA5,0x99,0x81,0x81,0x81,0x81} //M
,{0x0,0x42,0x46,0x4A,0x52,0x62,0x42,0x0} //N
,{0x7E,0x42,0x42,0x42,0x42,0x42,0x42,0x7E} //O
,{0x3E,0x42,0x42,0x42,0x3E,0x2,0x2,0x2} //P
,{0x0,0x7E,0x42,0x42,0x52,0x62,0x7E,0x80} //Q
,{0x3E,0x42,0x42,0x42,0x3E,0x12,0x22,0x42} //R
,{0x7C,0x2,0x2,0x3C,0x40,0x40,0x40,0x3E} //S
,{0x7F,0x8,0x8,0x8,0x8,0x8,0x8,0x8} //T
,{0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x3C} //U
,{0x42,0x42,0x42,0x42,0x42,0x42,0x24,0x18} //V
,{0x81,0x81,0x81,0x81,0x99,0xA5,0xC3,0x81} //W
,{0x81,0x42,0x24,0x18,0x18,0x24,0x42,0x81} //X
,{0x42,0x42,0x42,0x42,0x7E,0x40,0x20,0x1E} //Y
,{0x7E,0x40,0x20,0x10,0x8,0x4,0x2,0x7E} //Z}																																																																																																																																																																																																								
};
*/
int8_t position =0;
int8_t positionb =0;
int8_t posSpin=0;

uint8_t letter_p =0;
uint8_t inletter_p=0;
uint8_t current_word =0;
uint16_t start_colour =0;

uint8_t rgbPlaneR[8][8];
uint8_t rgbPlaneG[8][8];
uint8_t rgbPlaneB[8][8];

void beginText(void)
{
	rgb_dir = 1;
	position = myTotalX-1;
  letter_p = 0;
  start_colour += 10;

  if(start_colour > 512)
	{
    	start_colour = 0;
  }
}

void animateText(void)
{
  uint8_t x,z;
	uint8_t R,G,B;

	if(rgb_dir == 1)
	{
		    for(x = 0; x < myTotalX; x++)
			  {
		    	for(z = 0; z < myTotalZ; z++)
					{
						if(words[current_word][letter_p]!=32)
						{
		    		   if(alphabet[letter(words[current_word][letter_p])][7-z] & (1<<(x)))
						   {
						     get_colour(20*position + start_colour,&R,&G,&B);
			           set(x,z,position,R,G,B);
		    			   //arrayP[ar(x, pos, z)] = colourWheel.get_colour(20*pos + start_colour);
		    		   }
						 }
		    	}
		    }
	}
	else
	{
			if (position==0) delay(100*delaySpeed);
			for(x = 0; x < myTotalX; x++){
		    	for(z = 0; z < myTotalZ; z++){
						set(x,z,position,BLACK);
		    	}
		    }
	}
	if(--position < 0)
	{
	    	if(rgb_dir == 1)
				{
	    		rgb_dir = 2;	
	    		position = myTotalX-1;
	    	}
				else
				{
					  //setPlaneZ(z,BLACK);
	  //  		memset(arrayP, 0, SIZE3*sizeof(COLOUR));
	    		  if(!words[current_word][++letter_p])
						{
							  letter_p=0;
								if(++current_word >= NUM_WORDS)
								{
										current_word = 0;
									  modeWaitUp=1;
									  delay(30*delaySpeed);
								}
		  //  		fadeOutDone = 1;
		    	  }

		    	rgb_dir = 1;
		    	position = myTotalX -1;
	    	}
	}
	delay(3.5*delaySpeed);
}

void writeChar(uint8_t c,uint8_t mode)
{
	uint8_t done=0;
  uint8_t x,y;
	uint8_t cR,cG,cB;

	while (!done && c>=64 && c<=64+26)
	{
		if(rgb_dir == 1)
		{
		    for(x = 0; x < myTotalX; x++)
			  {
		    	for(y = 0; y < myTotalY; y++)
					{
						if(words[current_word][letter_p]!=32)
						{
		    		   if(alphabet[letter(c)][7-y] & (1<<(x)))
						   {
						     get_colour(20*position + start_colour,&cR,&cG,&cB);
								 if (position==0 && (R!=0 || G!=0 || B!=0) && modeText>=10 && textColourMode==0) {cR=R;cG=G;cB=B;}
			           set(x,y,position,cR,cG,cB);
		    		   }
						 }
		    	}
		    }
		}
		else
		{
			if (position==0) delay(100*delaySpeed);
			for(x = 0; x < myTotalX; x++){
		    	for(y = 0; y < myTotalY; y++){
						set(x,y,position,BLACK);
		    	}
		    }
		}
		if(--position < 0)
		{
	    	if(rgb_dir == 1)
				{
	    		rgb_dir = 2;	
	    		position = myTotalX-1;
	    	}
				else
				{
					done=1;
		    	rgb_dir = 1;
		    	position = myTotalX -1;
	    	}
		}
		delay(3.5*delaySpeed);	
	}
}

void beginText7(void){
	rgb_dir = 1;
	position = myTotalX-1;
	positionb = 0;
	posSpin=0;
  letter_p = 0; 
  start_colour += 10;
  if(start_colour > 512)
	{
		start_colour = 0;
  }
}

void textSpin()
{
	 uint8_t aPos, bPos, cPos;
 	 uint8_t x,y,z;
    
	 for (aPos=0; aPos <8; aPos++)
   {
		 	setAll(BLACK);
	    for(x = 0; x < 6; x++)
	    {
        for(y = 0; y < myTotalY; y++)
			  {
				   for(z = 0; z < 6; z++)
			     {
						   if (aPos==7) {bPos=3; cPos=0;}
						   if (aPos==0 || aPos==4) bPos=x;
						   if (aPos==1 || aPos==5) bPos=3;
						   if (aPos==2) {bPos=5-x; cPos=1;}
							 if (aPos==6) {bPos=5-x; cPos=0;}
							 if (aPos==3) bPos=3;
							 if (aPos!=1 && aPos!=5 && z==bPos)
							 {
								  if (cPos==0)
									{
										set(x+1,y,bPos,rgbPlaneR[x][y],rgbPlaneG[x][y],rgbPlaneB[x][y]); 
									}
									else
									{
										if(x!=6)
										{
											set(x+1,y,bPos,rgbPlaneR[5-x][y],rgbPlaneG[5-x][y],rgbPlaneB[5-x][y]); 
									  }
									}										
							 }
							 if ((aPos==1 || aPos==5) && x==bPos)
							 {
								  if (cPos==0)
									{
										set(x+1,y,z,rgbPlaneR[z][y],rgbPlaneG[z][y],rgbPlaneB[z][y]); 
									}
									else
									{
										set(x+1,y,z,rgbPlaneR[5-z][y],rgbPlaneG[5-z][y],rgbPlaneB[5-z][y]); 
									}
							 }
					 }
				}
			}
			delay(15*delaySpeed);
		}   
}

void animateText7(void)
{
  uint8_t x,y,z;
	uint8_t R,G,B;
  char currentLetter;
	
	if (ledCubeType==1) currentLetter=words7[current_word][letter_p]; // Aura Cube
	else currentLetter=rgbwords7[current_word][letter_p]; // RGB Cube
	if (numCubes>1) currentLetter=words7multi[current_word][letter_p]; // Multi Cube
	if(rgb_dir ==4)
	{	// spin
		 if (posSpin ==0)
		 {
			 	for(x = 0; x < myTotalX; x++){
		    	for(y = 0; y < myTotalY; y++){
					    rgbPlaneR[x][y]=getR(x+1,y,3);
					    rgbPlaneG[x][y]=getG(x+1,y,3);
					    rgbPlaneB[x][y]=getB(x+1,y,3);						
					}
				}
		 }
	   if (currentLetter!=32) textSpin();
		 if (++posSpin >2)
		 {
			 delay(20*delaySpeed);
			 rgb_dir =1;
			 posSpin=0;
			 positionb =0;
			 position--;
		 }
	}
	if(rgb_dir == 1)
	{
		    for(x = 0; x < myTotalX-1; x++)
			  {
		    	for(z = 0; z < myTotalZ; z++)
					{
						if(currentLetter!=32)
						{
		    		   if(alphabet7[letter7(currentLetter)][7-z] & (1<<(x)))
						   {
						     get_colour(20*position + start_colour,&R,&G,&B);
			           set(x+1,z,position,R,G,B);
		    		   }
						 }
		    	}
		    }
			  if (position==3) {rgb_dir = 3; positionb=1;delay(20*delaySpeed);}
	}
	else if (rgb_dir ==2)
	{
			if (position==0) delay(100*delaySpeed);
			for(x = 0; x < totalX-1; x++){
		    	for(z = 0; z < totalZ; z++){
						set(x+1,z,position,BLACK);
		    	}
		    }
	}
	else if (rgb_dir ==3)
	{		
		   for(x = 0; x < totalX-1; x++){
		    	for(z = 0; z < totalZ; z++){
						set(x+1,z,totalZ-positionb+1,BLACK);
		    	}
		    }
	}
	
if (positionb == 0)
	{
	  if(--position < 0)
	  {
	    	if(rgb_dir == 1)
				{
	    		rgb_dir = 2;	
	    		position = myTotalX-1;
	    	}
				else
				{
					  letter_p++;
						if (ledCubeType==1) currentLetter=words7[current_word][letter_p]; // Aura Cube
						else currentLetter=rgbwords7[current_word][letter_p]; // RGB Cube
						if (numCubes>1) currentLetter=words7multi[current_word][letter_p]; // Multi Cube
	    		  if(!currentLetter)
						{
							  letter_p=0;
								if(++current_word >= NUM_WORDS7)
								{
										current_word = 0;
									  modeWaitUp=1;
									  delay(20*delaySpeed);
								}
		    	  }

		    	rgb_dir = 1;
		    	position = myTotalX -1;
	    	}
	  }
  }
	else
	{
		positionb++;
		if (positionb >5 && rgb_dir !=4)
		{
			 delay(20*delaySpeed);
       rgb_dir=4;
		}
	}
	delay(2*delaySpeed);
}


void initScrollText(uint8_t param)
{
	current_word=0;
	letter_p=0;
  inletter_p=0;
	scrollFinished=0;
}

void moveOutside(uint8_t aroundBack)
{ 
	uint8_t i,k;
	uint8_t R,G,B;
	for (i=0;i<(2*totalX+2*totalZ-4);i++)
	{
		for (k=0;k<8;k++)
	  {
			// back side first
			if (i<totalX-2 && aroundBack==1)  //6
			{
				R=getR((totalX-2)-i-1,k,totalZ-1); //12
				G=getG((totalX-2)-i-1,k,totalZ-1);
				B=getB((totalX-2)-i-1,k,totalZ-1);
				set((totalX-2)-i,k,totalZ-1,R,G,B);				
			}
			if (i>=totalX-2 && i < (totalZ-1+totalX-1))  //5 and 14
			{
				R=getR(0,k,(totalZ-2)+(totalX-2)-i);
				G=getG(0,k,(totalZ-2)+(totalX-2)-i);
				B=getB(0,k,(totalZ-2)+(totalX-2)-i);
				set(0,k,(totalZ-1)+(totalX-2)-i,R,G,B);				
			}
			if (i>=(totalZ-1+totalX-1) && i<(totalX-1+totalZ-1+totalX-1))  //14 and 21
			{
				R=getR(i-((totalZ-1+totalX-1)-1),k,0);
				G=getG(i-((totalZ-1+totalX-1)-1),k,0);
				B=getB(i-((totalZ-1+totalX-1)-1),k,0);
				set(i-(totalZ-1+totalX-1),k,0,R,G,B);												
			}
			if (i>=(totalX-1+totalZ-1+totalX-1)) //21
			{
				R=getR(totalX-1,k,(i-(totalX-1+totalZ-1+totalX-1)+1));
				G=getG(totalX-1,k,(i-(totalX-1+totalZ-1+totalX-1)+1));
				B=getB(totalX-1,k,(i-(totalX-1+totalZ-1+totalX-1)+1));
				set(totalX-1,k,(i-(totalX-1+totalZ-1+totalX-1)),R,G,B);
			}
		}
	}
};

void frontMoveOutside(uint8_t fill)
{ 
	uint8_t i,k,m;
	uint8_t R,G,B;
	for (i=0;i<(2*totalX+2*totalZ-4);i++)
	{
		for (k=0;k<8;k++)
	  {
			// back to front first
/*			if (i<totalZ-2)  //6
			{
				R=getR(totalX-1,k,i+2);
				G=getG(totalX-1,k,i+2);
				B=getB(totalX-1,k,i+2);
				set(totalX-1,k,i+1,R,G,B);
			}
			if (i>=totalZ-2 && i < (totalZ-1+totalX-1))  //5 and 14
			{
				R=getR((totalZ-2)+(totalX-2)-i,k,totalZ-1); //12
				G=getG((totalZ-2)+(totalX-2)-i,k,totalZ-1);
				B=getB((totalZ-2)+(totalX-2)-i,k,totalZ-1);
				set((totalZ-2)+(totalX-1)-i,k,totalZ-1,R,G,B);				
			}
			if (i>=(totalZ-1+totalX-1) && i<(totalZ-1+totalX-1+totalZ-1))  //14 and 21
			{
				R=getR(0,k,(totalZ-1+totalX-1)+(totalZ-2)-i);
				G=getG(0,k,(totalZ-1+totalX-1)+(totalZ-2)-i);
				B=getB(0,k,(totalZ-1+totalX-1)+(totalZ-2)-i);
				set(0,k,(totalZ-1+totalX-1)+(totalZ-1)-i,R,G,B);				
			}*/
			if (i>=(totalZ-1+totalX-1+totalZ-1)) //21
			{
				if (fill)
				{
					R=getR(i-((totalZ-1+totalX-1+totalZ-1)-1),k,0);
					G=getG(i-((totalZ-1+totalX-1+totalZ-1)-1),k,0);
					B=getB(i-((totalZ-1+totalX-1+totalZ-1)-1),k,0);
					for(m=0;m<3;m++)
					{
						set(i-(totalZ-1+totalX-1+totalZ-1),k,m,R,G,B);	
					}
				}
				else
				{
					R=getR(i-((totalZ-1+totalX-1+totalZ-1)-1),k,0);
					G=getG(i-((totalZ-1+totalX-1+totalZ-1)-1),k,0);
					B=getB(i-((totalZ-1+totalX-1+totalZ-1)-1),k,0);
					set(i-(totalZ-1+totalX-1+totalZ-1),k,0,R,G,B);								
				}
			}
		}
	} 	
};

uint8_t scrollReadText(char currentLetter)
{
 uint8_t k,l,m;
 uint8_t space=0;
 uint8_t done=0;
 if (R==0&&G==0&&B==0) 
 {
	 R=255;
	 if (textColourMode==1) nextColour(&R,&G,&B,textColourRandom); 
 }
 if (textColourMode==2) nextColour(&R,&G,&B,textColourRandom); 
 while(!done && ((currentLetter>=46&&currentLetter<=65+26)||currentLetter==32))
 {
		if(currentLetter==32 || currentLetter==0) 
		{
			space=1;
			if (textColourMode==1) nextColour(&R,&G,&B,textColourRandom); 
		}
		for(k = 0; k < 8; k++)												//读取数据
		{
		  l=1<<inletter_p;
			if ((alphabet7[letter7(currentLetter)][k]&l)&&!space&&!scrollFinished)
			{
				if (modeText==11) set(totalX-1,7-k,totalZ-1,R,G,B);
				else 
				{
					if (textMode==0) set(totalX-1,7-k,0,R,G,B);
					else
					{
						for(m=0;m<3;m++)
						{
							set(totalX-1,7-k,m,R,G,B);
						}
					}
				}
			}
			else
			{
				if (modeText==11) set(totalX-1,7-k,totalZ-1,BLACK);
				else
				{
					if (textMode==0) set(totalX-1,7-k,0,BLACK);
					else
					{
						for(m=0;m<3;m++)
						{
							set(totalX-1,7-k,m,BLACK);
						}
					}
				}
			}
		}
	  inletter_p++;
		if (inletter_p>7)
		{
			inletter_p=0;
			done=1;
		}
		if (modeText==11) moveOutside(textMode);
		else frontMoveOutside(textMode);
		if (scrollFinished && isClearAll()) return 1;
		delay(15*delaySpeed);
	}
  return 0;
}

void scrollText(uint8_t number)
{
 uint8_t R,G,B;
 uint8_t k,l;
 uint8_t space=0;
 char currentLetter;
 currentLetter=wordsScroll[current_word][letter_p];
 if(currentLetter==32 || currentLetter==0) space=1;
 for(k = 0; k < 8; k++)												//读取数据
	 {
		  R=wordsScrollCol[current_word][letter_p][0];
			G=wordsScrollCol[current_word][letter_p][1];
			B=wordsScrollCol[current_word][letter_p][2];
		  l=1<<inletter_p;
			if ((alphabet7[letter7(currentLetter)][k]&l)&&!space&&!scrollFinished)
			{
				set(totalX-1,7-k,totalZ-1,R,G,B);
			}
			else
			{
				set(totalX-1,7-k,totalZ-1,BLACK);
			}
		}
	  inletter_p++;
		if (inletter_p>7)
		{
			inletter_p=0;
			if(currentLetter==32 || currentLetter==0) 
			{
				if (current_word < NUM_WORDS_SCROLL-1)
				{
					current_word++;
				}
				else
				{
					scrollFinished=1;
				}
				letter_p=0;
			}			
			else
			{
				letter_p++;
			}
		}
  moveOutside(1);
	if (scrollFinished && isClearAll()) modeWaitUp=1;
}

void scrollTime(uint8_t number)
{
 uint8_t R,G,B;
 uint8_t k,l;
 uint8_t space=0;
 char currentLetter;
	
  if (number==0) 
  {
    if(updateDateTime()==1) letter_p=10; // Can't get Date Time
  }
	if (letter_p<5) // Don't Print Seconds
	{
		 currentLetter=time[letter_p];
     R=0;G=255;B=0;	 
	}		
	if (letter_p>=5) 
	{
		currentLetter=32;
		R=0;G=0;B=0;
	}
	if(currentLetter==32 || currentLetter==0) space=1; // Include null terminator as space
  for(k = 0; k < 8; k++)												//读取数据
	{
		  l=1<<inletter_p;
			if ((alphabet7[letter7(currentLetter)][k]&l)&&!space&&!scrollFinished)
			{
				set(totalX-1,7-k,totalZ-1,R,G,B);
			}
			else
			{
				set(totalX-1,7-k,totalZ-1,BLACK);
			}
		}
	  inletter_p++;
		if (inletter_p>7)
		{
			inletter_p=0;
			if(letter_p>=5)
			{
				scrollFinished=1;
				letter_p=0;
			}
			else
			{
				letter_p++;
			}
		}
  moveOutside(1);
	if (scrollFinished && isClearAll()) modeWaitUp=1;
}

void scrollDateTime(uint8_t number)
{
 uint8_t R,G,B;
 uint8_t k,l;
 uint8_t space=0;
 char currentLetter;
 char dayOfWeek[5];
	
  if (number==0) 
  {
    if(updateDateTime()==1) letter_p=26; // Can't get Date Time
  }
  if (letter_p<4)
	{
	   if (weekDay==0) strcpy(dayOfWeek,"SUN ");
	   if (weekDay==1) strcpy(dayOfWeek,"MON ");
	   if (weekDay==2) strcpy(dayOfWeek,"TUE ");
	   if (weekDay==3) strcpy(dayOfWeek,"WED ");
	   if (weekDay==4) strcpy(dayOfWeek,"THU ");
	   if (weekDay==5) strcpy(dayOfWeek,"FRI ");
	   if (weekDay==6) strcpy(dayOfWeek,"SAT ");
		 currentLetter=dayOfWeek[letter_p];
		 R=0;G=0;B=255;	
	}
	if (letter_p>=4 && letter_p<16) 
	{
		 currentLetter=date[letter_p-4];
		 R=255;G=0;B=0;	 
	}
	if (letter_p>=16 && letter_p<21) // 26 but don't print seconds 
	{
		 currentLetter=time[letter_p-16];
     R=0;G=255;B=0;	 
	}		
	if (letter_p>=21) 
	{
		currentLetter=32;
		R=0;G=0;B=0;
	}
	if(currentLetter==32 || currentLetter==0) space=1; // Include null terminator as space
  for(k = 0; k < 8; k++)												//读取数据
	{
		  l=1<<inletter_p;
			if ((alphabet7[letter7(currentLetter)][k]&l)&&!space&&!scrollFinished)
			{
				set(totalX-1,7-k,totalZ-1,R,G,B);
			}
			else
			{
				set(totalX-1,7-k,totalZ-1,BLACK);
			}
		}
	  inletter_p++;
		if (inletter_p>7)
		{
			inletter_p=0;
			if(letter_p>=21)
			{
				scrollFinished=1;
				letter_p=0;
			}
			else
			{
				letter_p++;
			}
		}
  moveOutside(1);
	if (scrollFinished && isClearAll()) modeWaitUp=1;
}

void spinIT()
{
	 uint8_t aPos, bPos, cPos;
 	 uint8_t x,y,z;
    
	 for (aPos=0; aPos < 4; aPos++)
   {
		 	setAll(BLACK);
	    for(x = 0; x < myTotalX; x++)
	    {
        for(y = 0; y < myTotalY; y++)
			  {
				   for(z = 0; z < myTotalZ; z++)
			     {
						   if (aPos==0) {bPos=x-1;cPos=x;}
						   if (aPos==1) {bPos=3;cPos=5;}
						   if (aPos==2) {bPos=7-x+1;cPos=7-x;} 
							 if (aPos==3) {bPos=3;cPos=4;}
  						 if (aPos==0 && z==bPos)
							 {
								  if (x<7) set(x,y,bPos,rgbPlaneR[x][y],rgbPlaneG[x][y],rgbPlaneB[x][y]); 
								  if (x<7 && x>0) set(x-1,y,cPos,rgbPlaneR[x][y],rgbPlaneG[x][y],rgbPlaneB[x][y]); 
							 }
							 if ((aPos==1) && x==bPos)
							 {
								  set(x,y,z,rgbPlaneR[z][y],rgbPlaneG[z][y],rgbPlaneB[z][y]); 
							    set(x+1,y,z,rgbPlaneR[z][y],rgbPlaneG[z][y],rgbPlaneB[z][y]); 
							 }
							 if (aPos==2 && z==bPos)
							 {
								  if (x>1) set(x,y,bPos,rgbPlaneR[x-1][y],rgbPlaneG[x-1][y],rgbPlaneB[x-1][y]); 
								  if (x>1) set(x-1,y,cPos,rgbPlaneR[x-1][y],rgbPlaneG[x-1][y],rgbPlaneB[x-1][y]); 
							 }
							 if ((aPos==3) && z==bPos)
							 {
								  set(x,y,bPos,rgbPlaneR[x][y],rgbPlaneG[x][y],rgbPlaneB[x][y]); 
							    set(x,y,cPos,rgbPlaneR[x][y],rgbPlaneG[x][y],rgbPlaneB[x][y]); 								 
							 }
					 }
				}
			}
			delay(10*delaySpeed);
		}   
}

void printClock(void)
{
	// Can't do printClock - Do Time instead
	doTime();
}
