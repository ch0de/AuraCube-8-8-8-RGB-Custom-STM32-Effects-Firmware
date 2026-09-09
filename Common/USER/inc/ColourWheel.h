#include "System.h"
#include "CubeFunction.h"

#ifndef COLOURWHEEL_h
#define COLOURWHEEL_h

#include "graphics.h"

#define BAM_RESOLUTION 8
#define COLOUR_WHEEL_LENGTH 512
extern int16_t colourPos;

void fill_colour_wheel(void);
void get_colour(int16_t p, uint8_t *R, uint8_t *G, uint8_t *B);
void get_next_colour(uint8_t *R, uint8_t *G, uint8_t *B);
void increment_colour_pos(uint8_t i);

#endif
