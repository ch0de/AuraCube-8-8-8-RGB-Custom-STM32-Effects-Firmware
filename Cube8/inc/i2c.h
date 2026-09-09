#ifndef _I2C_H_
#define _I2C_H_

#include "System.h"
#include "stm32f10x.h"
#include "SoftWire.h"

#define BUFFER_LENGTH 25

/*#ifndef BOOL
#define BOOL
typedef enum
{
  FALSE = 0, TRUE  = !FALSE
}
bool;
#endif
*/

/* return codes from endTransmission() */
#define SUCCESS   0        /* transmission was successful */
#define EDATA     1        /* too much data */
#define ENACKADDR 2        /* received nack on transmit of address */
#define ENACKTRNS 3        /* received nack on transmit of data */
#define EOTHER    4        /* other error */

#define I2C_MSG_READ            0x1
#define I2C_MSG_10BIT_ADDR      0x2

typedef struct wireBase { 
    uint16_t addr;                /**< Address */
    uint16_t flags;               /**< Bitwise OR of I2C_MSG_READ and */
	  uint16_t length;              /**< Message length */
    uint8_t buf[BUFFER_LENGTH];      /* receive buffer */
    uint8_t buf_idx;               /* first buf */
	  uint8_t bytesRec;               /* # of bytes Received */
    bool buf_overflow;
} wireBase;


extern wireBase myWireBase;
void beginTransmission(uint8_t);
uint8_t endTransmission(uint8_t);
uint8_t requestFromi2c(uint8_t, int num_bytes);
uint32_t puti2c(uint8_t);
uint32_t writei2c(uint8_t*, int);
uint8_t availablei2c(void);
uint8_t readi2c(void);

#endif
