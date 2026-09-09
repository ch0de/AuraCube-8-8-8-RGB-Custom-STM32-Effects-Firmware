#include "effects.h"
#include "System.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

// --- existing globals (unchanged) ---
float fadeLevel = 100;
float fadeMax = 100;
float fadeIncrement = 0.2;

uint8_t slideLevel = 0;
uint8_t slideDirX=0;
uint8_t slideDirY=0;
uint8_t slideDirZ=0;
uint8_t shakeDir=0;
int8_t  explodeLevel=0;
uint8_t eCounterMax=0;
uint8_t transitionSpeed=8;
uint8_t doTransitions=1;
uint8_t inFadeMax=0;
uint8_t stripeN=0;

// ---- Cloud (corner-gradient) state ----
// EFFECT_CLOUD: bitmask flag used in eNum to trigger cloud() effect.
// This section defines parameters and state for the smooth corner-gradient
// cloud color effect that interpolates HSV values between cube corners.
//
// Tunable parameters and what changing them does:
//   cloud_gamma       - gamma correction for output (higher = darker mids, default 2.2)
//   cloud_v_max       - overall maximum brightness (0..255). Lower = dimmer cube.
//   cloud_drift_scale - base hue drift speed at cube corners. Larger = faster color change.
//   cloud_s_min       - minimum saturation at cube corners. Lower = more pastel colors.
//   cloud_s_max       - maximum saturation at cube corners. Higher = more vivid colors.
//   cloud_scale       - scales how strongly the drift applies. Smaller = smoother/slower, larger = more chaotic.
//
// State arrays:
//   corner[][][]      - HSV values for the 8 cube corners.
//   corner_drift[][][]- hue drift speeds per corner (each corner shifts differently).
//   cloud_inited      - ensures cloud_init() runs only once.
//   cloud_frame       - frame counter for temporal dithering.

#define EFFECT_CLOUD  (1u<<16)

static uint8_t  cloud_inited = 0;      // has initialization run yet?
static float    cloud_gamma  = 1.6f;   // adjust midtone brightness curve
static uint8_t  cloud_v_max  = 100;    // master brightness cap
static float    cloud_drift_scale = 1.0f; // base hue drift speed (increase = faster shifts)
static float    cloud_s_min  = 0.98f;  // lowest saturation allowed (closer to 0 = more white/pastel)
static float    cloud_s_max  = 1.0f;  // highest saturation allowed (closer to 1 = pure vivid color)
static float    cloud_scale  = 1.00f;  // user knob: scales drift/smoothness (higher = more active)

// 8 corner HSVs
typedef struct { float h, s, v; } HSVf;
static HSVf corner[2][2][2];
static float corner_drift[2][2][2];

static uint32_t cloud_frame = 0;

static inline float wrap360(float h){ while(h>=360.f)h-=360.f; while(h<0)h+=360.f; return h; }
static inline float lerp(float a,float b,float t){ return a + (b-a)*t; }

static float hue_lerp(float h0, float h1, float t)
{
    float d = fmodf(h1 - h0 + 540.f, 360.f) - 180.f;
    return wrap360(h0 + d * t);
}

static inline float gamma_encode(float x, float g){ x = fmaxf(0.f,fminf(1.f,x)); return powf(x, 1.f/g); }
static void hsv_to_rgb_gamma(uint16_t Hdeg, float S, float V, uint8_t *R, uint8_t *G, uint8_t *B)
{
    float H = fmodf((float)Hdeg, 360.f);
    float C = V * S;
    float X = C * (1.f - fabsf(fmodf(H/60.f, 2.f) - 1.f));
    float m = V - C;
    float r=0,g=0,b=0;
    if      (H <  60) { r=C; g=X; b=0; }
    else if (H < 120) { r=X; g=C; b=0; }
    else if (H < 180) { r=0; g=C; b=X; }
    else if (H < 240) { r=0; g=X; b=C; }
    else if (H < 300) { r=X; g=0; b=C; }
    else              { r=C; g=0; b=X; }
    r += m; g += m; b += m;

    float gr = gamma_encode(r, cloud_gamma);
    float gg = gamma_encode(g, cloud_gamma);
    float gb = gamma_encode(b, cloud_gamma);

    uint8_t vmax = cloud_v_max;
    *R = (uint8_t)(fminf(1.f, gr) * vmax);
    *G = (uint8_t)(fminf(1.f, gg) * vmax);
    *B = (uint8_t)(fminf(1.f, gb) * vmax);
}

static inline uint8_t dither_1lsb(uint8_t x, uint8_t y, uint8_t z, uint32_t f)
{
    uint32_t h = x*73856093u ^ y*19349663u ^ z*83492791u ^ (f*1664525u + 1013904223u);
    return (uint8_t)((h >> 24) & 1u);
}

static inline float frand01(void){ return (float)rand() / (float)RAND_MAX; }

static void cloud_init(void)
{
    for (int z=0; z<2; ++z)
    for (int y=0; y<2; ++y)
    for (int x=0; x<2; ++x)
    {
        corner[z][y][x].h = frand01()*360.f;
        corner[z][y][x].s = lerp(cloud_s_min, cloud_s_max, frand01());
        corner[z][y][x].v = 1.0f;
        float sign = (frand01()<0.5f)?-1.f:1.f;
        corner_drift[z][y][x] = sign * (0.25f + 0.75f*frand01()) * cloud_drift_scale;
    }
    cloud_inited = 1;
}

void cloud(void)
{
    if (!cloud_inited) cloud_init();

    uint16_t tx, ty, tz;
    if (ledCubeType < 2 || numCubes == 1) {
        tx = trimTotalX; ty = trimTotalY; tz = trimTotalZ;
    } else {
        tx = curTotalX;  ty = curTotalY;  tz = curTotalZ;
    }

    float level = fadeLevel / 100.0f;

    float cx = (tx - 1) * 0.5f;
    float cy = (ty - 1) * 0.5f;
    float cz = (tz - 1) * 0.5f;

    setLED = 2;
    for (int x = 0; x < tx; x++)
    for (int y = 0; y < ty; y++)
    for (int z = 0; z < tz; z++) {
        float fx = (float)x/(float)(tx-1);
        float fy = (float)y/(float)(ty-1);
        float fz = (float)z/(float)(tz-1);

        float h01 = hue_lerp(corner[0][0][0].h, corner[0][0][1].h, fx);
        float h02 = hue_lerp(corner[0][1][0].h, corner[0][1][1].h, fx);
        float h03 = hue_lerp(corner[1][0][0].h, corner[1][0][1].h, fx);
        float h04 = hue_lerp(corner[1][1][0].h, corner[1][1][1].h, fx);

        float h05 = hue_lerp(h01, h02, fy);
        float h06 = hue_lerp(h03, h04, fy);
        float H   = hue_lerp(h05, h06, fz);

        float S = lerp(lerp(corner[0][0][0].s, corner[0][0][1].s, fx),
                       lerp(corner[0][1][0].s, corner[0][1][1].s, fx), fy);
        float S2 = lerp(lerp(corner[1][0][0].s, corner[1][0][1].s, fx),
                        lerp(corner[1][1][0].s, corner[1][1][1].s, fx), fy);
        S = lerp(S, S2, fz);

        float V = lerp(lerp(corner[0][0][0].v, corner[0][0][1].v, fx),
                       lerp(corner[0][1][0].v, corner[0][1][1].v, fx), fy);
        float V2= lerp(lerp(corner[1][0][0].v, corner[1][0][1].v, fx),
                       lerp(corner[1][1][0].v, corner[1][1][1].v, fx), fy);
        V = lerp(V, V2, fz);

        uint8_t R,G,B;
        hsv_to_rgb_gamma((uint16_t)H, S, V, &R, &G, &B);

        if (dither_1lsb(x,y,z,cloud_frame)) {
            if (R<255)R++;
            if (G<255)G++;
            if (B<255)B++;
        }

        set(x,y,z,(uint8_t)(R*level),(uint8_t)(G*level),(uint8_t)(B*level));
    }
    setLED = 0;

    // scale hue drift by user-provided cloud_scale (keeps moving even if tiny)
    float drift = fmaxf(0.05f, cloud_scale);
    for (int z=0; z<2; ++z)
    for (int y=0; y<2; ++y)
    for (int x=0; x<2; ++x)
        corner[z][y][x].h = wrap360(corner[z][y][x].h + corner_drift[z][y][x]*drift);

    cloud_frame++;

    if (numCubes > 1 && ledCubeType >= 2) {
        copyEffectBuffer(2, 0, 98, level);
    }
}

// runEffect() and setEffect() already call cloud() using EFFECT_CLOUD



void copyBuffer(uint8_t from, uint8_t to)
{
	uint32_t i = 0;
	for(i = 0; i < myTotalPixels*3; i++)
 	{
		rgbled[to][i] = rgbled[from][i];
	}
}

void fadeIn()
{
	float level;
  uint32_t i = 0;
	fadeLevel=fadeLevel+fadeIncrement;
	if (fadeLevel>50) fadeLevel=fadeLevel+2*fadeIncrement;
	level=fadeLevel/100;
	if (fadeLevel >= fadeMax)
	{
		 fadeLevel = fadeMax;
		 eNum = eNum & 65533;  // fadein done
		 if(eNum < 256) showLED=0;
  }
	if (eNum <256) // not slide or the others. They will fade itself
	{	
		for(i = 0; i < myTotalPixels*3; i++)
		{
			  rgbled[2][i] = rgbled[0][i] * level;
		}
		if (numCubes>1 && ledCubeType>=2) // CUBE12 and linked DMA
		{
			copyEffectBuffer(2,0,99,level);
		}
	}
}

void fadeOut()
{
	float level;
	uint32_t i = 0;
	fadeLevel=fadeLevel-fadeIncrement;
	if (fadeLevel>50) fadeLevel=fadeLevel-2*fadeIncrement;
	level=fadeLevel/100;
	if (level < 0)
	{
		 eNum = eNum & 65531;  // fadeout done
		 if (eNum < 256)
		 {
			 fadeLevel = fadeMax;
			 if (eFadeoutDone==2) eFadeoutDone=3;
		 }
  }
	if (eNum<256) // not slide or others. They will fade itself
	{
		for(i = 0; i < myTotalPixels*3; i++)
		{
			  rgbled[2][i] = rgbled[0][i] * level;
		}
		if (numCubes>1 && ledCubeType>=2) // CUBE12 and linked DMA
		{
				copyEffectBuffer(2,0,99,level);
		}				
	}
}

void constantDim()
{
	float level;
  uint32_t i = 0;
	showLED=2;
	level=fadeMax/100;
	inFadeMax=1;
	for(i = 0; i < myTotalPixels*3; i++)
	{
			  rgbled[2][i] = rgbled[0][i] * level;
	}
	if (numCubes>1 && ledCubeType>=2) // CUBE12 and linked DMA
	{
     copyEffectBuffer(2,0,99,level);
	}			
}

void fadetrail()
{
//	float level;
  uint32_t i = 0;
	showLED=2;
//	level=fadeMax/100;
//	inFadeMax=1;
	for(i = 0; i < myTotalPixels*3; i++)
	{
		    if (rgbled[0][i]!=0)
				{
					rgbled[2][i]=rgbled[0][i];
				}
				else
				{
					rgbled[2][i]=rgbled[2][i]*fadeIncrement;
				}
	}
	if (numCubes>1 && ledCubeType>=2) // CUBE12 and linked DMA
	{
		// Not Implemented
	}
}

void flash()
{
	 if (eCounter2 > eInterval1 && showLED == 0)
	 {
		   showLED = 2;
   		 eCounter2=0;
	 }
	 if (eCounter2 > eInterval2 && showLED == 2)
	 {
		   showLED = 0;
   		 eCounter2=0;
	 }
}

void invert()
{
  uint32_t i = 0;
  for(i = 0; i < myTotalPixels*3; i++)
 	{
			  rgbled[2][i] = 255-rgbled[0][i];
	}
}

void background()
{
  uint32_t i = 0;
  for(i = 0; i < myTotalPixels*3; i++)
 	{
			  rgbled[2][i] = rgbled[0][i];
		    if (rgbled[0][i]==0) rgbled[2][i] = 30;
	}

}

void blackwhite()
{
	float level;
	float slevel;
	int x,y,z;
	uint8_t R,G,B;
	uint16_t tx,ty,tz;
	if (ledCubeType<2 || numCubes==1)
	{
		// Aura8 or own cube
		tx=trimTotalX;
		ty=trimTotalY;
		tz=trimTotalZ;		
	}
	else
	{
		// Cube12 extended
		tx=curTotalX;
		ty=curTotalY;
		tz=curTotalZ;		
	}	
	level=fadeLevel/100;
	slevel=((float)slideLevel)/255;
  for (x=0;x<tx;x++)
  {
	  for (y=0;y<ty;y++)
	  {
	    for (z=0;z<tz;z++)
	    {
				 getRGB(x,y,z,&R,&G,&B);
			   if (R>G && R>B) {G=G+slevel*((float)(R-G));;B=B+slevel*((float)(R-B));}
			   if (G>R && G>B) {R=R+slevel*((float)(G-R)); B=B+slevel*((float)(G-B));}
			   if (B>G && B>R) {R=R+slevel*((float)(B-R)); G=G+slevel*((float)(B-G));}
			   setLED=2;
			   set(x,y,z,R*level,G*level,B*level);
			   setLED=0;
		  }
	  }
	}
	if (numCubes>1 && ledCubeType>=2) // CUBE12 and linked DMA
	{
			copyEffectBuffer(2,0,98,level);
	}
	if (slideLevel == 0)
	{
		 fadeLevel = fadeMax;
		 eNum = eNum & 64511;  // fadein done
		 if(eNum < 256) showLED=0;
  }
	slideLevel--;
}

void increase()
{
  uint32_t i = 0;
  for(i = 0; i < myTotalPixels*3; i++)
 	{
		    if (rgbled[0][i] > 0)
				{
			      rgbled[2][i] = rgbled[0][i]+50;
	        if (rgbled[2][i]>255) rgbled[2][i] = 255;
				}
				else
				{
					  rgbled[2][i] = 0;
				}
	}
}

void decrease()
{
  uint32_t i = 0;
  for(i = 0; i < myTotalPixels*3; i++)
 	{
		    if (rgbled[0][i] > 0)
				{
					  if (rgbled[0][i] > 50)
							rgbled[2][i] = rgbled[0][i]-50;
						else rgbled[2][i] = 0;
				}
				else
				{
					  rgbled[2][i] = 0;
				}
	}
}

void slideMove(int x, int y, int z)
{
	float level;
	int count_i = 0,count_j = 0;
	int count_x, count_y;
	int Ox,Px,Qx,Oy,Py,Qy,Oz,Pz,Qz;
	int PPz = 0;

	level=fadeLevel/100;
  if (slideDirX==1||slideDirX==3) {Ox=x; Px=0; Qx=-1;}
	else {Ox=(mySize-1)-x; Px=(mySize-1); Qx=1;}
	if (slideDirY==1||slideDirY==3) {Oy=y; Py=0; Qy=-1;}
  else {Oy=(mySize-1)-y; Py=(mySize-1); Qy=1;}
	if (slideDirZ==1||slideDirZ==3) {Oz=z; Pz=(mySize-1); PPz=0; Qz=-1;}
	else {Oz=(mySize-1)-z; Pz=0; PPz=(mySize-1); Qz=1;}
	
	if (slideDirX==0) {count_i = ((mySize-1)-Ox)*3; count_x = count_i;}
  if (slideDirX==1||slideDirX==2) {count_i = ((mySize-1)-Ox)*3; count_x = (((mySize-1)-Ox)-(-(Px-(mySize-1))+Qx*slideLevel))*3;}
	if (slideDirX==3||slideDirX==4) {count_i = (Ox)*3;	count_x = ((Ox)+(-(Px-(mySize-1))+Qx*slideLevel))*3;}
	if (slideDirZ==0) {count_i+=(Oz)*3*mySize; count_x+=(Oz)*3*mySize;}
	if (slideDirZ==1||slideDirZ==2) {count_i+=(Oz)*3*mySize; count_x+=(Oz+(Pz+Qz*slideLevel))*3*mySize;}
	if (slideDirZ==3||slideDirZ==4) {count_i+=((mySize-1)-Oz)*3*mySize; count_x+=(((mySize-1)-Oz)-(Pz+Qz*slideLevel))*3*mySize;}
	if (slideDirY==0) {count_j =  3*mySize*mySize*((mySize-1)-Oy); count_y = count_j;}
  if (slideDirY==1||slideDirY==2) {count_j = 3*mySize*mySize*((mySize-1)-Oy);	count_y=3*mySize*mySize*(((mySize-1)-Oy)-(-(Py-(mySize-1))+Qy*slideLevel));}
	if (slideDirY==3||slideDirY==4) {count_j = 3*mySize*mySize*(Oy); count_y=3*mySize*mySize*((Oy)+(-(Py-(mySize-1))+Qy*slideLevel));}
	if ((Ox<=(Px-Qx*slideLevel)||slideDirX==0)&& (Oy<=(Py-Qy*slideLevel)||slideDirY==0) && (Oz <= (PPz-Qz*slideLevel)||slideDirZ==0))
	{	
			if (ledCubeType==0 || ledCubeType==1) // Only need rgb_order for Cube8 and Aura8
			{	
				rgbled[2][rgb_order[count_i+0]+count_j] = rgbled[0][rgb_order[count_x+0]+count_y] * level;
				rgbled[2][rgb_order[count_i+1]+count_j] = rgbled[0][rgb_order[count_x+1]+count_y] * level;
				rgbled[2][rgb_order[count_i+2]+count_j] = rgbled[0][rgb_order[count_x+2]+count_y] * level;
			}
			else
			{
				  // Note X is swapped here - Don't think this will cause too much issues
					rgbled[2][count_i+0+count_j] = rgbled[0][count_x+0+count_y] * level;
					rgbled[2][count_i+1+count_j] = rgbled[0][count_x+1+count_y] * level;
					rgbled[2][count_i+2+count_j] = rgbled[0][count_x+2+count_y] * level;				
			}
	 }
	 else
	 {
		 	if (ledCubeType==0 || ledCubeType==1) // Only need rgb_order for Cube8 and Aura8
			{	
				rgbled[2][rgb_order[count_i+0]+count_j] = 0;
				rgbled[2][rgb_order[count_i+1]+count_j] = 0;
				rgbled[2][rgb_order[count_i+2]+count_j] = 0;
			}
			else
			{
				rgbled[2][count_i+0+count_j] = 0;
				rgbled[2][count_i+1+count_j] = 0;
				rgbled[2][count_i+2+count_j] = 0;
			}
	 }
}

void slide()
{
	int x,y,z;
 	for (x=0;x<mySize;x++)
	{
			for (y=0;y<mySize;y++)
		  {
			  for (z=0;z<mySize;z++)
			  {
					slideMove(x,y,z);
 			  }
		  }
	 }
	 if (eCounter2 > eInterval1)
	 {
			slideLevel++;
			if (slideLevel > (mySize-1))
			{
					eNum = eNum & 65279;
					showLED = 0;
					slideLevel = 0;
				  fadeLevel=fadeMax;
	      if (eFadeoutDone==2) eFadeoutDone=3;
			}
		  eCounter2=0;
	  }
	  if (ledCubeType>=2 && numCubes>1) copyDMAfromRGB(); // AURA12 Copy to other cubes
}

void shake()
{
	int x,y,z;
	slideLevel=1;
	slideDirY=0;

	if (eCounter2 > eInterval1)
	{
		 if (shakeDir==0)
		 {
			  if (slideDirX==0) slideDirX=2;
			  else slideDirX=0;
		 }
 		 if (shakeDir==2)
		 {
			  if (slideDirX==0) slideDirX=4;
			  else slideDirX=0;
		 }
		 if (shakeDir==1)
		 {
			  if (slideDirZ==0) slideDirZ=2;
			  else slideDirZ=0;
		 }
 		 if (shakeDir==3)
		 {
			  if (slideDirZ==0) slideDirZ=4;
			  else slideDirZ=0;
		 }
		 if (shakeDir==0 && slideDirX==0) shakeDir=2;
		 else{
				if (shakeDir==1 && slideDirZ==0) shakeDir=3;
			  else{
				    if (shakeDir==2 && slideDirX==0) shakeDir=1;
					  else{
								if (shakeDir==3 && slideDirZ==0) shakeDir=0;		 
					    }
						}
				}
		 eCounter2=0;
  }
 	for (x=0;x<mySize;x++)
	{
			for (y=0;y<mySize;y++)
		  {
			  for (z=0;z<mySize;z++)
			  {
					slideMove(x,y,z);
 			  }
		  }
	 }
	if (ledCubeType>=2 && numCubes>1) copyDMAfromRGB(); // AURA12 Copy to other cubes
}

int isBlank(uint8_t tx,uint8_t ty,uint8_t tz)
{
	int clear=1;
  int x,y,z;
	for (x=0;x<tx;x++)
	{
		for (y=0;y<ty;y++)
		{
			for (z=0;z<tz;z++)
			{	
				if (!isClearBuffer(1,x,y,z) && !outOfBounds) {clear=0;return(clear);}
			}
		}
	}
	return(clear);	
}

int isFull(uint8_t tx,uint8_t ty,uint8_t tz)
{
	int full=1;
  int x,y,z;
	for (x=0;x<tx;x++)
	{
		for (y=0;y<ty;y++)
		{
			for (z=0;z<tz;z++)
			{	
				if (isClearBuffer(1,x,y,z) && !outOfBounds) {full=0;return(full);} 					
			}
		}
	}
  return(full);	
}

void dissolve()
{
  uint32_t i = 0;
	uint8_t notFound = 1;
	uint8_t numFound = 0;
	uint8_t numToFind = 1;
	uint16_t numTot=0;
  int x,y,z;
	uint16_t tx,ty,tz;
	float level;
	if (ledCubeType<2 || numCubes==1)
	{
		// Aura8 or own cube
		tx=trimTotalX;
		ty=trimTotalY;
		tz=trimTotalZ;		
	}
	else
	{
		// Cube12 extended
		tx=curTotalX;
		ty=curTotalY;
		tz=curTotalZ;		
	}
	level=fadeLevel/100;
	numTot=tx*ty*tz;
	if (numTot>512) numToFind=2; // Bigger so dissolve more at once
	if (numTot>1728) numToFind=3; // Bigger so dissolve more at once
	for(i = 0; i < myTotalPixels*3; i++)
		{
			  rgbled[2][i] = (rgbled[0][i] & rgbled[1][i]) * level;
		}
	if (numCubes>1 && ledCubeType>=2) // CUBE12 and linked DMA
	{
     copyEffectBuffer(2,0,1,level);
	}			
	 if (isBlank(tx,ty,tz))
	 {
		  eCounterMax=transitionSpeed;
		 	eNum = eNum & 63487;
			showLED = 0;
			fadeLevel=fadeMax;
		  if (eFadeoutDone==2) eFadeoutDone=3;	 
	 }
	 else
	 {
		 	while (numFound<numToFind && !isBlank(tx,ty,tz))
			{
				while (notFound)
				{
					x=cube_random(tx);
					y=cube_random(ty);
					z=cube_random(tz);
					if (!isClearBuffer(1,x,y,z))
					{
						setLED=1;
						clear(x,y,z);
						setLED=0;
						notFound=0;
					}
				}
				numFound++;
				notFound=1;
			}
	 }
}

void unDissolve()
{
  uint32_t i = 0;
	uint8_t notFound = 1;
	uint8_t numFound = 0;
	uint8_t numToFind = 1;
	uint16_t numTot=0;	
	uint16_t tx,ty,tz;
  int x,y,z;
	float level;
	if (ledCubeType<2 || numCubes==1)
	{
		// Aura8 or own cube
		tx=trimTotalX;
		ty=trimTotalY;
		tz=trimTotalZ;		
	}
	else
	{
		// Cube12 extended
		tx=curTotalX;
		ty=curTotalY;
		tz=curTotalZ;		
	}	
	level=fadeLevel/100;
	numTot=tx*ty*tz;
	if (numTot>512) numToFind=2; // Bigger so dissolve more at once
	if (numTot>1728) numToFind=3; // Bigger so dissolve more at once	
	for(i = 0; i < myTotalPixels*3; i++)
	{
			  rgbled[2][i] = (rgbled[0][i] & rgbled[1][i]) * level;
	}
	if (numCubes>1 && ledCubeType>=2) // CUBE12 and linked DMA
	{
     copyEffectBuffer(2,0,1,level);
	}	
	 if (isFull(tx,ty,tz))
	 {
		  eCounterMax=transitionSpeed;
		 	eNum = eNum & 61439;
			showLED = 0;
			fadeLevel=fadeMax;
	 }
	 else
	 {
		  while (numFound<numToFind && !isFull(tx,ty,tz))
			{
				while (notFound)
				{
					x=cube_random(tx);
					y=cube_random(ty);
					z=cube_random(tz);
					if (isClearBuffer(1,x,y,z))
					{
						setLED=1;
						set(x,y,z,WHITE);
						setLED=0;
						notFound=0;
					}
				}
				numFound++;
				notFound=1;
			}
	 }
}

void stripe()
{
  uint32_t i = 0;
  int x,z,col=0;
	float level;
	level=fadeLevel/100;
	if (stripeN==2) col=2;
	if (eCounter2 > eInterval1 || stripeN==0)
	{
		eCounter2=0;
		if (stripeN==2) stripeN=1;
		else stripeN=2;
		//Draw Stripe
     eCounterMax=1;
	   setLED=1;
 		 for (z=0;z<trimTotalZ;z++)
		 {
				for (x=0;x<trimTotalX;x++)
				{
					col++;
					if (col>3) col=0;
					if (col<2)
					{
						line(x,0,z,x,trimTotalY-1,z,0,0,0);
					}
					else
					{
						line(x,0,z,x,trimTotalY-1,z,0,200,0);				
					}
				}
				/*if (col==0)
					col=1;
				else
					col=0;*/
	 		}
		 setLED=0;	
	}
	for(i = 0; i < myTotalPixels*3; i++)
	{
		  //levelN=(float)rgbled[1][i]/100;
			//rgbled[2][i] = ((float)rgbled[0][i] * levelN) * level;
		  if (rgbled[0][i]==0)
			{
				rgbled[2][i] = 0;
			}
			else
			{
				rgbled[2][i] = (rgbled[0][i] +rgbled[1][i]) * level;
			}
	}
}

void moveOut(uint8_t level,int8_t x,int8_t y,int8_t z)
{
	int8_t xx,yy,zz;
	uint8_t R,G,B;
	uint8_t ran;
	if (x<myRound(vXpos)) xx=x-level;
	else xx=x+level;
	if (y<myRound(vYpos)) yy=y-level;
	else yy=y+level;
	if (z<myRound(vZpos)) zz=z-level;
	else zz=z+level;
	//if (xx>=0&&yy>=0&&zz>=0&&xx<trimTotalX&&yy<trimTotalY&&zz<trimTotalZ)
	{
		setLED=0;
		getRGB(x,y,z,&R,&G,&B);
		setLED=1;
		if (eInterval2==0 || eInterval2==2)
		{
			ran=cube_random(6);
			if (ran==0) set(xx,yy,zz,YELLOW);
			if (ran==1) set(xx,yy,zz,ORANGE);
			if (ran==2) set(xx,yy,zz,RED);
			if (ran==3) set(xx,yy,zz,WHITE);
			if (ran>3) set(xx,yy,zz,R,G,B);
		}
		else
		{
			set(xx,yy,zz,R,G,B);
		}
		if (eInterval2==0 || eInterval2==2)
		{
			// make more spots
			ran=cube_random(6);
			if (ran==0) set(x,yy,zz,YELLOW);
			if (ran==1) set(x,yy,zz,ORANGE);
			if (ran==2) set(x,yy,zz,RED);
			if (ran==3) set(x,yy,zz,R,G,B);
			ran=cube_random(6);
			if (ran==0) set(xx,y,zz,YELLOW);
			if (ran==1) set(xx,y,zz,ORANGE);
			if (ran==2) set(xx,y,zz,RED);
			if (ran==3) set(xx,y,zz,R,G,B);
			ran=cube_random(6);
			if (ran==0) set(xx,yy,z,YELLOW);
			if (ran==1) set(xx,yy,z,ORANGE);
			if (ran==2) set(xx,yy,z,RED);
			if (ran==3) set(xx,yy,z,R,G,B);			
		}
	}
}

void changeExplodeColour(uint8_t level,uint16_t tx,uint16_t ty,uint16_t tz)
{
//	uint8_t ran;
  int x,y,z;
	uint8_t R,G,B;
	for (x=0;x<tx;x++)
	{
		for (y=0;y<ty;y++)
		{
		  for (z=0;z<tz;z++)
		  {	
        if (!isClearBuffer(0,x,y,z))
				 {
						if ((eInterval2==0 || eInterval2==2) && level!=0)
						{
							set(x,y,z,WHITE);
							/*ran=cube_random(3);
							if (ran==0) set(x,y,z,YELLOW);
							if (ran==1) set(x,y,z,ORANGE);
							if (ran==2) set(x,y,z,RED);
							*/
						}
						else
						{
							setLED=0;
							getRGB(x,y,z,&R,&G,&B);
							setLED=1;
							set(x,y,z,R,G,B);
						}		
				 }
			}
		}
	}
}

void explode()
{
  int x,y,z;
	uint16_t tx,ty,tz;
	uint16_t i=0;
	float level;
	uint8_t ran;
	level=fadeLevel/100;
	if (ledCubeType<2 || numCubes==1)
	{
		// Aura8 or own cube
		tx=trimTotalX;
		ty=trimTotalY;
		tz=trimTotalZ;		
	}
	else
	{
		// Cube12 extended
		tx=curTotalX;
		ty=curTotalY;
		tz=curTotalZ;		
	}
	if (eCounter2 > eInterval1)
	{
		ran=cube_random(totalMax); //8
		if ((explodeLevel>2+ran && eInterval2==2))
		{
			eInterval1=30;
			eInterval2=3; // go the other way
			explodeLevel--;
		}
		eCounter2=0;
		eCounterMax=1;
		setLED=1;
		setAll(BLACK);
		if (explodeLevel>1)
		{
			for (x=0;x<tx;x++)
			{
				for (y=0;y<ty;y++)
				{
					for (z=0;z<tz;z++)
					{
						if(!isClearBuffer(0,x,y,z)) moveOut(explodeLevel-1,x,y,z);
					}
				}
			}
		}
		if (explodeLevel<2)
		{
      changeExplodeColour(explodeLevel,tx,ty,tz);
		}
		if ((explodeLevel>10 && eInterval2==0) || explodeLevel<0 ||(eInterval2==0 && explodeLevel>4 && isClearAllBuffer(1)))
		{
			eNum = eNum & 32767;
			setLED=0;
			if (eInterval2==0) setAll(BLACK);
			showLED=0;
		}
		if ((eInterval2==1 || eInterval2==3)&&isClearAllBuffer(1))
		{
       eCounter2=eInterval1+1;
		}			
		if (eInterval2 == 0 || eInterval2 == 2) explodeLevel++;
		else explodeLevel--;
		setLED=0;
  }
	for(i = 0; i < myTotalPixels*3; i++)
	{
			  rgbled[2][i] = (rgbled[1][i]) * level;
	}
	if (numCubes>1 && ledCubeType>=2) // CUBE12 and linked DMA
	{
     copyEffectBuffer(2,1,99,level);
	}			
}

void runEffect()
{	
		if (eNum & 1)
		{
			 flash();
		}	
		if (eNum & 2)
		{
			 fadeIn();
		}
		if (eNum & 4)
		{
			 fadeOut();
		}
		if (eNum & 16)
		{
			 invert();
		}
		if (eNum & 32)
		{
			 background();
		}		
		if (eNum & 64)
		{
			 increase();
		}		
		if (eNum & 128)
		{
			 decrease();
		}				
		if (eNum & 256)
		{
			 slide();
		}		
		if (eNum & 512)
		{
			 shake();
		}
		if (eNum & 1024)
		{
			 blackwhite();
		}
		if (eNum & 2048)
		{
			 dissolve();
		}
		if (eNum & 4096)
		{
			 unDissolve();
		}			
    if (eNum & 8192)
		{
       stripe();
		}
    if (eNum & 16384)
		{
       fadetrail();
		}			
    if (eNum & 32768)
		{
       explode();
		}
    if (eNum & EFFECT_CLOUD)
       {
          cloud();
       }
}

void doConstantDim()
{
    if (eNum <256 && eNum !=2 && eNum !=4)
		{	 // set constant fadelevel
					fadeLevel=fadeMax;
					constantDim();
		}
}

void doEffect()
{
    // Run CLOUD every tick for smooth updates (bypass eCounter/eCounterMax and doTransitions).
    if (power == 1 && (eNum & EFFECT_CLOUD))
    {
        // Keep constant dim logic active if user lowered fadeMax
        if (fadeMax != 100) {
            doConstantDim();
        } else if (inFadeMax == 1) {
            showLED = 0;
            inFadeMax = 0;
        }

        runEffect();      // cloud() will be called from runEffect()
        eCounter = 0;     // reset counters like the timed path does
        eCounter2++;
        return;           // we're done this loop
    }

    // Normal timed path for all other effects
    if (eCounter > eCounterMax && power == 1)
    {
        if (fadeMax != 100) doConstantDim();
        else if (inFadeMax == 1) { showLED = 0; inFadeMax = 0; }

        // Always allow effects to run
        if (eNum > 0) runEffect();
        eCounter = 0;
        eCounter2++;
    }
    else eCounter++;
}

void randomFade()
{
	 uint8_t eFadein;
	 if (doTransitions)
	 {
	    eFadein=cube_random(3)+1;
	    if (eFadein==1) setEffect("fadein",0,0,0);
	    if (eFadein==2) setEffect("slide",0,0,0);
	    if (eFadein==3) setEffect("undissolve",0,0,0);
	    if (!modeWait) eFadeout=cube_random(3)+2; // skip none and roll your own. Can't fade if the end is abrupt with modeWait
   }
}

// NOTE: Signature now matches effects.h exactly (const char *e)
void setEffect(const char *e, int interval1, int interval2, float increment)
{
 if (doTransitions)
 {
	eCounterMax=transitionSpeed;
	if (interval1 != 0) eInterval1 = interval1;
	if (interval2 != 0) eInterval2 = interval2;
	if (increment != 0) fadeIncrement = increment;
	if (strcmp(e,"flash")==0)
	{
		eNum = eNum | 1;
	}
	if (strcmp(e,"fadein")==0)
	{
		eNum = eNum | 2;
    if (increment==0) fadeIncrement = 0.2;
		fadeLevel=0;
		showLED=2;
	}	
	if (strcmp(e,"fadeout")==0)
	{
		eNum = eNum | 4;
		if (increment==0) fadeIncrement = 0.2;
		showLED=2;
	}	
	if (strcmp(e,"showboth")==0)
	{
		// Bit 8 not currently used in runEffect(); left for compatibility
		eNum = eNum | 8;
	}	
	if (strcmp(e,"invert")==0)
	{
		eNum = eNum | 16;
		showLED=2;
	}	
	if (strcmp(e,"background")==0)
	{
		eNum = eNum | 32;
		showLED=2;
	}		
	if (strcmp(e,"increase")==0)
	{
		eNum = eNum | 64;
		showLED=2;
	}		
	if (strcmp(e,"decrease")==0)
	{
		eNum = eNum | 128;
		showLED=2;
	}		
	if (strcmp(e,"slide")==0)
	{
		slideLevel = 0;
		if (interval1==0) eInterval1=10;
		eNum = eNum | 256;
		showLED=2;
  	if (increment==0)
		{
				slideDirX=cube_random(8);
				slideDirY=cube_random(8);
				slideDirZ=cube_random(8);
				if (slideDirX>4) slideDirX=0;
				if (slideDirY>4) slideDirY=0;
				if (slideDirZ>4) slideDirZ=0;		
			  if (eFadeoutDone==1)
				{
					 if (slideDirX==1||slideDirX==3) slideDirX+=1;
					 if (slideDirY==1||slideDirY==3) slideDirY+=1;
					 if (slideDirZ==1||slideDirZ==3) slideDirZ+=1;	
				}
        else
				{
					 if (slideDirX==2||slideDirX==4) slideDirX-=1;
					 if (slideDirY==2||slideDirY==4) slideDirY-=1;
					 if (slideDirZ==2||slideDirZ==4) slideDirZ-=1;
				}					
		}
	}
	if (strcmp(e,"shake")==0)
	{
		eNum = eNum | 512;
		showLED=2;
	}
	if (strcmp(e,"blackwhite")==0)
	{
		eNum = eNum | 1024;
		slideLevel=255;
		showLED=2;
	}
	if (strcmp(e,"dissolve")==0)
	{
		eCounterMax=1;
		eNum = eNum | 2048;
		showLED=2;
		setLED=1;
		setAll(WHITE);
		setLED=0;
	}
	if (strcmp(e,"undissolve")==0)
	{
		eCounterMax=1;
		eNum = eNum | 4096;
		showLED=2;
		setLED=1;
		setAll(BLACK);
		setLED=0;
	}
	if (strcmp(e,"stripe")==0)
	{
		eCounterMax=1;
		stripeN=0;
		eNum = eNum | 8192;
		showLED=2;
		setLED=1;
		setAll(BLACK);
		setLED=0;
	}
	if (strcmp(e,"fadetrail")==0)
	{
		eNum = eNum | 16384;
		if (increment==0) fadeIncrement = 0.8;
		showLED=2;
	}
	if (strcmp(e,"explode")==0)
	{
		eCounterMax=1;
		explodeLevel=0;
		eInterval2=interval2;
		if (interval2==1)
		{
			eInterval2=1;
			explodeLevel=9;
		}
		if (interval1==0) eInterval1=2;
		eCounter2=eInterval1+1; // force run
		eNum = eNum | 32768;
		showLED=2;
	}
  if (eFadeoutDone==1) eFadeoutDone=2; //already set
	runEffect(); // have the initial run
 }
 if (strcmp(e,"cloud")==0)
    {
        eNum |= EFFECT_CLOUD;
        showLED = 2;
        // Make cloud as smooth as possible
        eCounterMax = 0;          // we also bypass timing in doEffect(), but this doesn't hurt
        if (interval1 != 0) eInterval1 = interval1;   // optional speed override (not used by cloud timing)
        if (increment != 0) cloud_scale = increment;  // optional activity/smoothness override
    }
}
