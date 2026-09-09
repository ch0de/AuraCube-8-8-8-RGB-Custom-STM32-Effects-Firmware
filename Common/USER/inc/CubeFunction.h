
#ifndef __CubeFunction_H__
#define __CubeFunction_H__

#include "System.h"
#include "stdlib.h"
#include "graphics.h"

float mySin(float x);
float myCos(float x);
float myTan(float x);
float mySqrt(float x);
float myMap(float in, float inMin, float inMax, float outMin, float outMax);
int16_t myRound(float x);
float myAbs(float x);
void randomize ( uint8_t arr[], uint8_t n );
uint8_t findGraphic (uint8_t graphicNum, uint8_t n);
void line(int8_t x1,int8_t y1,int8_t z1,int8_t x2,int8_t y2,int8_t z2,
		  uint8_t red,uint8_t green,uint8_t blue);
void PlaneXY(int8_t x1,int8_t y1,int8_t z1,int8_t x2,int8_t y2,int8_t z2,
			 uint8_t red,uint8_t green,uint8_t blue,int8_t fill);	
void PlaneYZ(int8_t x1,int8_t y1,int8_t z1,int8_t x2,int8_t y2,int8_t z2,
			 uint8_t red,uint8_t green,uint8_t blue,int8_t fill);
void PlaneZX(int8_t x1,int8_t y1,int8_t z1,int8_t x2,int8_t y2,int8_t z2,
			 uint8_t red,uint8_t green,uint8_t blue,int8_t fill);	
void filledBox(int8_t x1,int8_t y1,int8_t z1,int8_t x2,int8_t y2,int8_t z2,
		 uint8_t r,uint8_t g,uint8_t b);
void Box(int8_t x1,int8_t y1,int8_t z1,int8_t x2,int8_t y2,int8_t z2,
		 uint8_t red,uint8_t green,uint8_t blue,int8_t fill,int8_t Up_Down);
void Box_Change(int8_t number,uint8_t red,uint8_t green,uint8_t blue,int8_t Up_Down);
void Waveform(int16_t number,uint8_t red,uint8_t green,uint8_t blue);	
void sphere(int x0,int y0,int z0,int r,uint8_t R, uint8_t G, uint8_t B);

#endif
