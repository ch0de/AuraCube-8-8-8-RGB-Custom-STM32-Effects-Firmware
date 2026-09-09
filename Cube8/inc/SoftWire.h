#ifndef _SOFTWIRE_H_
#define _SOFTWIRE_H_

#include "System.h"
#include "stdbool.h"
#include "i2c.h"
#include "LED.h" // Delay

#define SOFT_STANDARD 19
#define SOFT_FAST 0

/*#ifndef BOOL
#define BOOL
typedef enum
{
  FALSE = 0, TRUE  = !FALSE
}
bool;
#endif
*/

//#define I2C_DELAY(x) {uint32 time=micros(); while(time>(micros()+x));}
#define I2C_DELAY(x) do{for(i=0;i<x;i++) {__asm volatile("nop");}}while(0)
uint8_t set_scl(uint8_t state);
void set_sda(uint8_t state);
void i2c_start(void);
void  i2c_stop(void);
void i2c_repeated_start(void);
bool i2c_get_ack(void);
void i2c_send_ack(void);
void i2c_send_nack(void);
uint8_t i2c_shift_in(void);
void i2c_shift_out(uint8_t);
void seti2cClock(uint32_t frequencyHz);
uint8_t processi2c(uint8_t stop);	

uint8_t i2cMasterReceive(uint8_t addr, uint8_t* pData, uint16_t size, uint8_t repeatStart);
uint8_t i2cMasterTransmit(uint8_t addr, uint8_t* pData, uint16_t size, uint8_t repeatStart);
void i2cScan(void);
uint8_t initI2C(void);
void resetI2C(void);

#endif // _SOFTWIRE_H_
