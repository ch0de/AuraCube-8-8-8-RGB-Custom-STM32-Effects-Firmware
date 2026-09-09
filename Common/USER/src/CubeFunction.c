#include "System.h"
#include "CubeFunction.h"
#include "LEDHardware.h"
#include "LED.h"
#include <math.h>

//FAST SINE APPROX
float mySin(float x){
	float sinr = 0;
	uint8_t g = 0;

	while(x > (float)myPI){
		x -= 2*(float)myPI; 
		g = 1;
	}

	while(!g&(x < -(float)myPI)){
		x += 2*(float)myPI;
	}

	sinr = (float)myDPI*x - (float)myDPI2*x*myAbs(x);
	sinr = (float)0.225*(sinr*myAbs(sinr)-sinr)+sinr;

	return sinr;
}

//FAST COSINE APPROX
float myCos(float x){
	return mySin(x+myPI/2);
}

float myTan(float x){
	return mySin(x)/myCos(x);
}

//SQUARE ROOT APPROX
float mySqrt(float in){
	int16_t d = 0;
	int16_t i = 0;
	int16_t in_ = in;
	float result = 2;
	
	for(d = 0; in_ > 0; in_ >>= 1){
		d++;
	}
	
	for(i = 0; i < d/2; i++){
		result = result*2;
	}
	
	for(i = 0; i < 3; i++){
		result = (float)0.5*(in/result + result);
	}
	
	return result;
}

//MAP NUMBERS TO NEW RANGE
float myMap(float in, float inMin, float inMax, float outMin, float outMax){
	float out;
	out = (in-inMin)/(inMax-inMin)*(outMax-outMin) + outMin;
	return out;
}

//ROUND A NUMBER
int16_t myRound(float in){
	int8_t s = in/myAbs(in);
	return (int16_t)(s*(myAbs(in) + (float)0.5));
}

//ABSOLUTE VALUE
float myAbs(float in){
	return (in)>0?(in):-(in);
}	

// A utility function to swap to integers
void swap (uint8_t *a, uint8_t *b)
{
    uint8_t temp = *a;
    *a = *b;
    *b = temp;
}

// A function to generate a random permutation of arr[]
void randomize ( uint8_t arr[], uint8_t n )
{
	uint8_t i=0,j=0;
     // Start from the last element and swap one by one. We don't
    // need to run for the first element that's why i > 0
    for (i = n-1; i > 0; i--)
    {
        j = cube_random(i+1); 
			  // Pick a random index from 0 to i
			  if (shuffleGraphics==4)
        {
					 // dont swap with 0 !
					 j = cube_random(i)+1; 
				}
        // Swap arr[i] with the element at random index
        swap(&arr[i], &arr[j]);
    }
}

uint8_t findGraphic (uint8_t graphicNum, uint8_t n)
{
	uint8_t i=0;
	for (i=0;i<n;i++)
	{
		 if (graphicOrder[i]==graphicNum) return i; // found it!
	}
	return 0;
}

void Trans(void)
{
	char x,y,z;
	int16_t i = 0,j = 0;
	for(z = 0; z < 8; z++)
	{
		for(y = 0; y < 8; y++)
		{
			for(x = 6; x; x--)
			{
				i = x*3+(7-y)*24;												
				j = 192*z;													
				rgbled[setLED][rgb_order[i+0]+j] = rgbled[1][rgb_order[i-3+0]+j];		
				rgbled[setLED][rgb_order[i+1]+j] = rgbled[1][rgb_order[i-3+1]+j];		
				rgbled[setLED][rgb_order[i+2]+j] = rgbled[1][rgb_order[i-3+2]+j];		
			}	
		}
	}		
	for(z = 0; z < 8; z++)
	{
		for(y = 0; y < 8; y++)
		{
			i = (7-y)*24;														
			j = 192*z;															
			rgbled[setLED][rgb_order[i+0]+j] = 0;									
			rgbled[setLED][rgb_order[i+1]+j] = 0;						
			rgbled[setLED][rgb_order[i+2]+j] = 0;			
		}
	}		
}

int16_t abss(int16_t dat)
{
	int16_t tpdat = 0;
	tpdat = dat/10;
	dat = dat-tpdat*10;
	if(dat >= 5)
		tpdat++;
	return tpdat;
}

void Max(int8_t *a,int8_t *b)
{
	if((*a) > (*b))
	{
		*a = *a ^ *b;
		*b = *b ^ *a;
		*a = *a ^ *b;	
	}
}

int8_t maxt(int8_t datA,int8_t datB,int8_t datC)
{
	if(datA <= datB)
		datA = datB;
	if(datA <= datC)
		datA = datC;
	return datA;
}

void line(int8_t x1,int8_t y1,int8_t z1,int8_t x2,int8_t y2,int8_t z2,uint8_t red,uint8_t green,uint8_t blue)
{
	int16_t mDat = 0,SumX = 0,SumY = 0,SumZ = 0,poorX = 0,poorY = 0,poorZ = 0;
	int8_t i = 0;	
	poorX = x2-x1;
	poorY = y2-y1;
	poorZ = z2-z1;
	mDat  = maxt(abs(poorX),abs(poorY),abs(poorZ));
	SumX  = x1*10;
	SumY  = y1*10;
	SumZ  = z1*10;
	poorX = poorX*10/mDat;
	poorY = poorY*10/mDat;
	poorZ = poorZ*10/mDat;
	set(x1,y1,z1,red,green,blue);
	
	for(i = 0; i <= mDat; i++)
	{
		set(abss(SumX),abss(SumY),abss(SumZ),red,green,blue);
		SumX = SumX+poorX;
		SumY = SumY+poorY;
		SumZ = SumZ+poorZ;
	}
	set(x2,y2,z2,red,green,blue);
}

void PlaneXY(int8_t x1,int8_t y1,int8_t z1,int8_t x2,int8_t y2,int8_t z2,
			 uint8_t red,uint8_t green,uint8_t blue, int8_t fill)
{
	int8_t i = 0;
	Max(&z1,&z2);
	if(fill)
	{
		for(i = z1; i <= z2; i++)
			line(x1,y1,i,x2,y2,i,red,green,blue);
	} 
	else
	{
		line(x1,y1,z1,x2,y2,z1,red,green,blue);
		line(x1,y1,z2,x2,y2,z2,red,green,blue);
		line(x2,y2,z1,x2,y2,z2,red,green,blue);
		line(x1,y1,z1,x1,y1,z2,red,green,blue);
	}
}

void PlaneYZ(int8_t x1,int8_t y1,int8_t z1,int8_t x2,int8_t y2,int8_t z2,
			 uint8_t red,uint8_t green,uint8_t blue,int8_t fill)
{
	int8_t i = 0;
	Max(&x1,&x2);
	if(fill)	
	{
		for(i = x1; i <= x2; i++)
			line(i,y1,z1,i,y2,z2,red,green,blue);
	}
	else
	{
		line(x1,y1,z1,x1,y2,z2,red,green,blue);
		line(x2,y1,z1,x2,y2,z2,red,green,blue);
		line(x1,y2,z2,x2,y2,z2,red,green,blue);
		line(x1,y1,z1,x2,y1,z1,red,green,blue);
	}
}

void PlaneZX(int8_t x1,int8_t y1,int8_t z1,int8_t x2,int8_t y2,int8_t z2,
			 uint8_t red,uint8_t green,uint8_t blue,int8_t fill)
{
	int8_t i = 0;
	Max(&y1,&y2);

	if(fill)	
	{
		for(i = y1; i <= y2; i++)
			line(x1,i,z1,x2,i,z2,red,green,blue);
	}
	else
	{
		line(x1,y1,z1,x2,y1,z2,red,green,blue);		
		line(x1,y2,z1,x2,y2,z2,red,green,blue);		
		line(x2,y1,z2,x2,y2,z2,red,green,blue);		
		line(x1,y1,z1,x1,y2,z1,red,green,blue);
	}
}

void filledBox(int8_t x1,int8_t y1,int8_t z1,int8_t x2,int8_t y2,int8_t z2,
		 uint8_t r,uint8_t g,uint8_t b)
{
	uint8_t x,y,z;
  for(x = x1; x <= x2; x++)
	{
	  for(y = y1; y <= y2; y++)
		{
			for(z = z1; z <= z2; z++)
			{
	       set(x,y,z,r,g,b);
			}
		}
	}
}

void Box(int8_t x1,int8_t y1,int8_t z1,int8_t x2,int8_t y2,int8_t z2,
		 uint8_t red,uint8_t green,uint8_t blue,int8_t fill,int8_t Up_Down)
{
	int8_t i = 0;
	int8_t number = abs(z2-z1);
	if(fill)
	{
		for(i = 0; i <= number; i++)
		{
			if(Up_Down)
				PlaneXY(x1+i,y1,(totalZ-1)-z1,x1+i,y2,(totalZ-1)-z2,red,green,blue,1);
			else
				PlaneXY(x1+i,y1,z1,x1+i,y2,z2,red,green,blue,1);
		}
	}
	else
	{
		if(Up_Down)
		{
			PlaneXY(x1,y1,(totalZ-1)-z1,x1,y2,(totalZ-1)-z2,red,green,blue,0);
			PlaneXY(x2,y1,(totalZ-1)-z1,x2,y2,(totalZ-1)-z2,red,green,blue,0);
			line(x1,y1,(totalZ-1)-z1,x2,y1,(totalZ-1)-z1,red,green,blue);
			line(x1,y2,(totalZ-1)-z1,x2,y2,(totalZ-1)-z1,red,green,blue);
			line(x1,y1,(totalZ-1)-z2,x2,y1,(totalZ-1)-z2,red,green,blue);
			line(x1,y2,(totalZ-1)-z2,x2,y2,(totalZ-1)-z2,red,green,blue);
		}
		else
		{
			PlaneXY(x1,y1,z1,x1,y2,z2,red,green,blue,0);
			PlaneXY(x2,y1,z1,x2,y2,z2,red,green,blue,0);
			line(x1,y1,z1,x2,y1,z1,red,green,blue);
			line(x1,y2,z1,x2,y2,z1,red,green,blue);
			line(x1,y1,z2,x2,y1,z2,red,green,blue);
			line(x1,y2,z2,x2,y2,z2,red,green,blue);
		}
	}
}

const uint8_t box1[24] =
{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x16,0x26,0x36,0x46,0x56,0x66,0x65,0x64,0x63,0x62,0x61,0x60,0x50,0x40,0x30,0x20,0x10};

void Box_Change(int8_t number,uint8_t red,uint8_t green,uint8_t blue,int8_t Up_Down)
{
	int8_t i = 0,x = 0,y = 0,t = 0;
	static int8_t datA[4];
	number %= 108;
	setAll(BLACK);																			//л
	if(number < 6)
	{
		Box(0,6,6,1,7,7,red,green,blue,1,!Up_Down);
		Box(number,6,6-number,number+1,7,7-number,red,green,blue,1,!Up_Down);
		Box(number,6,6,number+1,7,7,red,green,blue,1,!Up_Down);
		Box(0,6,6-number,1,7,7-number,red,green,blue,1,!Up_Down);
		Box(0,6-number,6,1,7-number,7,red,green,blue,1,!Up_Down);
		Box(number,6-number,6-number,number+1,7-number,7-number,red,green,blue,1,!Up_Down);
		Box(number,6-number,6,number+1,7-number,7,red,green,blue,1,!Up_Down);
		Box(0,6-number,6-number,1,7-number,7-number,red,green,blue,1,!Up_Down);
	}
	else if(number < 30)
	{
		if(6 == number)
			for(i = 0; i < 4;i++)
				datA[i] = 6*i;
		
		for(i = 0; i < 4;i++)
		{
			t = (uint8_t)datA[i] % 24;
			x = box1[t] >> 4;
			y = box1[t] & 0x0f;
			Box(x,y,0,x+1,y+1,1,red,green,blue,1,Up_Down);
			Box(x,y,6,x+1,y+1,7,red,green,blue,1,Up_Down);
		}
		for(i = 0; i < 4;i++)
			datA[i]++;
	}
	else if(number < 54)
	{
		for(i = 0; i < 4;i++)
		{
			t = (uint8_t)datA[i] % 24;
			x = box1[t] >> 4;
			y = box1[t] & 0x0f;
			Box(x,y,0,x+1,y+1,1,red,green,blue,1,Up_Down);
			Box(x,y,6,x+1,y+1,7,red,green,blue,1,Up_Down);
		}
		for(i = 0; i < 4;i++)
			datA[i]--;
	}
	else if(number < 78)
	{
		for(i = 0; i < 4;i++)
		{
			t = (uint8_t)datA[i] % 24;
			x = box1[t] >> 4;
			y = box1[t] & 0x0f;
			Box(x,0,y,x+1,1,y+1,red,green,blue,1,Up_Down);
			Box(x,6,y,x+1,7,y+1,red,green,blue,1,Up_Down);
		}
		for(i = 0; i < 4;i++)
			datA[i]++;
	}
	else if(number < 102)
	{
		for(i = 0; i < 4;i++)
		{
			t = (uint8_t)datA[i] % 24;
			x = box1[t] >> 4;
			y = box1[t] & 0x0f;
			Box(x,0,y,x+1,1,y+1,red,green,blue,1,Up_Down);
			Box(x,6,y,x+1,7,y+1,red,green,blue,1,Up_Down);
		}
		for(i = 0; i < 4;i++)
			datA[i]--;
	}
	else if(number < 108)
	{
		number = 108-number;
		Box(0,6,6,1,7,7,red,green,blue,1,Up_Down);
		Box(number,6,6-number,number+1,7,7-number,red,green,blue,1,Up_Down);
		Box(number,6,6,number+1,7,7,red,green,blue,1,Up_Down);
		Box(0,6,6-number,1,7,7-number,red,green,blue,1,Up_Down);
		Box(0,6-number,6,1,7-number,7,red,green,blue,1,Up_Down);
		Box(number,6-number,6-number,number+1,7-number,7-number,red,green,blue,1,Up_Down);
		Box(number,6-number,6,number+1,7-number,7,red,green,blue,1,Up_Down);
		Box(0,6-number,6-number,1,7-number,7-number,red,green,blue,1,Up_Down);
	}
}

const uint8_t waveline[13]={0,1,2,0x23,5,6,7,6,5,0x23,2,1,0};
const uint8_t wave[24]=
{
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x16,0x26,0x36,0x46,0x56,
0x66,0x65,0x64,0x63,0x62,0x61,0x60,0x50,0x40,0x30,0x20,0x10
};
const uint8_t place[63]=
{
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,0,2,4,6,8,10,12,14,
16,18,20,22,0,3,6,9,12,15,18,21,0,4,8,12,16,20,0,5,10,15,20,0,6,12,18,0,7,14,21
};

void Waveform(int16_t number,uint8_t red,uint8_t green,uint8_t blue)
{
	char t,x,y;

	number %= 204;
	if(number < 78)
	{
		number %= 13;
		if(waveline[number] >> 4)
		{
			t = waveline[number] & 0x0f;
			line(0,0,t+1,0,7,t+1,red,green,blue);
		}
		else 
			t = waveline[number];
		line(0,0,t,0,7,t,red,green,blue);
		Trans();
	}
	else if(number < 141)
	{
		t = number-78;
		x = wave[(uint8_t)place[t]] >> 4;
		y = wave[(uint8_t)place[t]] & 0x0f;
		PlaneXY(0,x,y,0,x+1,y+1,red,green,blue,1);
		Trans();
	}
	else if(number < 204)
	{
		t = number-141;
		x = wave[(uint8_t)place[t]] >> 4;
		y = wave[(uint8_t)place[t]] & 0x0f;
		set(0,x,y,red,green,blue);
		Trans();
	}
}

void sphere(int x0,int y0,int z0,int r,uint8_t R, uint8_t G, uint8_t B)
{
    int x,y,z,xa,ya,za,xb,yb,zb,xr,yr,zr,xx,yy,zz,rr=r*r;
    // bounding box
    xa=x0-r; if (xa<0) xa=0; xb=x0+r; if (xb>totalX) xb=totalX;
    ya=y0-r; if (ya<0) ya=0; yb=y0+r; if (yb>totalY) yb=totalY;
    za=z0-r; if (za<0) za=0; zb=z0+r; if (zb>totalZ) zb=totalZ;
    // project xy plane
    for (x=xa,xr=x-x0,xx=xr*xr;x<xb;x++,xr++,xx=xr*xr)
     for (y=ya,yr=y-y0,yy=yr*yr;y<yb;y++,yr++,yy=yr*yr)
        {
        zz=rr-xx-yy; if (zz<0) continue; zr=sqrt(zz);
        z=z0-zr; if ((z>0)&&(z<totalZ)) set(x,y,z,R,G,B);
        z=z0+zr; if ((z>0)&&(z<totalZ)) set(x,y,z,R,G,B);
        }
    // project xz plane
    for (x=xa,xr=x-x0,xx=xr*xr;x<xb;x++,xr++,xx=xr*xr)
     for (z=za,zr=z-z0,zz=zr*zr;z<zb;z++,zr++,zz=zr*zr)
        {
        yy=rr-xx-zz; if (yy<0) continue; yr=sqrt(yy);
        y=y0-yr; if ((y>0)&&(y<totalY)) set(x,y,z,R,G,B);
        y=y0+yr; if ((y>0)&&(y<totalY)) set(x,y,z,R,G,B);
        }
    // project yz plane
    for (y=ya,yr=y-y0,yy=yr*yr;y<yb;y++,yr++,yy=yr*yr)
     for (z=za,zr=z-z0,zz=zr*zr;z<zb;z++,zr++,zz=zr*zr)
        {
        xx=rr-zz-yy; if (xx<0) continue; xr=sqrt(xx);
        x=x0-xr; if ((x>0)&&(x<totalX)) set(x,y,z,R,G,B);
        x=x0+xr; if ((x>0)&&(x<totalX)) set(x,y,z,R,G,B);
        }
}
