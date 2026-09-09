#ifndef _LCD_H
#define _LCD_H

#include "System.h"
#include "LED.h"
#include "i2c.h"
#include "bme280.h"
#include "string.h"	
#include "stdarg.h"	
#include "stdio.h"

void lcd_clear(void);
void lcdInit (void);
void lcd_clearln(uint8_t line);
void lcd_printxy(uint8_t line , uint8_t col, char *str);
void lcd_println(uint8_t line, uint8_t pad, char* fmt, ...);
void lcd_printBuf(uint8_t line, uint8_t pad, char* fmt, ...);
void lcd_printDateTime(uint8_t row, uint8_t printSec);
void doi2cUpdates(void);
void updateNow(void);

extern uint8_t lcdMaxCol;
extern uint8_t lcdMaxRow;
extern uint8_t timeDispRow;

#endif

