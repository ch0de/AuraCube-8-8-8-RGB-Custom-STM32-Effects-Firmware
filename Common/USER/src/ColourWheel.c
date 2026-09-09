
#include "ColourWheel.h"

uint8_t colourR[COLOUR_WHEEL_LENGTH];
uint8_t colourG[COLOUR_WHEEL_LENGTH];
uint8_t colourB[COLOUR_WHEEL_LENGTH];

int16_t ColPos = 0;

int16_t colourPos = 0;

void fill_colour_wheel(void){
	float red, green, blue;
	float c, s;
	int32_t phase = 0;
	int16_t I = 0;

	while(phase < COLOUR_WHEEL_LENGTH){
		s = (1<<BAM_RESOLUTION)*mySin(myPI*(3*phase-I*COLOUR_WHEEL_LENGTH)/(2*COLOUR_WHEEL_LENGTH));
		c = (1<<BAM_RESOLUTION)*myCos(myPI*(3*phase-I*COLOUR_WHEEL_LENGTH)/(2*COLOUR_WHEEL_LENGTH));
		
		red = (I==0?1:0)*s + (I==1?1:0)*c;
		green = (I==1?1:0)*s + (I==2?1:0)*c;
		blue = (I==2?1:0)*s + (I==0?1:0)*c;
		
		colourR[phase] = red;
		colourG[phase] = green;
		colourB[phase] = blue;
		
		if(++phase >= (1+I)*COLOUR_WHEEL_LENGTH/3){
			I++;
		}
	}
}

void get_colour(int16_t p, uint8_t *R, uint8_t *G, uint8_t *B){
	if(p >= COLOUR_WHEEL_LENGTH){
		p -= COLOUR_WHEEL_LENGTH;
	}	
	
	*R = colourR[p];
	*G = colourG[p];
	*B = colourB[p];
}

void get_next_colour(uint8_t *R, uint8_t *G, uint8_t *B){
	if(++ColPos >= COLOUR_WHEEL_LENGTH){
		ColPos -= COLOUR_WHEEL_LENGTH;
	}
	*R = colourR[ColPos];
	*G = colourG[ColPos];
	*B = colourB[ColPos];
}

void increment_colour_pos(uint8_t i){
	colourPos += i;
	while(colourPos >= COLOUR_WHEEL_LENGTH){
		colourPos -= COLOUR_WHEEL_LENGTH;
	}
}
