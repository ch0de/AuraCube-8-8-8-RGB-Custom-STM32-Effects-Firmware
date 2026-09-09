#include "i2c.h"
wireBase myWireBase;

void beginWireBase(uint8_t self_addr) {
    myWireBase.buf_overflow = FALSE;
    myWireBase.buf_idx = 0;
	  myWireBase.length = 0;
	  myWireBase.flags = 0;
}

void beginTransmission(uint8_t slave_address) {
    myWireBase.addr = slave_address;
	  myWireBase.buf_idx = 0;
    myWireBase.length = 0;
    myWireBase.flags = 0;
}

uint8_t endTransmission(uint8_t stop) 
{
    uint8_t retVal;
    if (myWireBase.buf_overflow) 
		{
        return EDATA;
    }
    retVal = processi2c(stop);
    myWireBase.buf_idx = 0;
		myWireBase.length = 0;
    myWireBase.buf_overflow = FALSE;
    return retVal;
}

uint8_t requestFromi2c(uint8_t address, int num_bytes) 
{
	  uint8_t retVal;
    if (num_bytes > BUFFER_LENGTH) {
        num_bytes = BUFFER_LENGTH;
    }
    myWireBase.addr = address;
    myWireBase.flags = I2C_MSG_READ;
    myWireBase.length = num_bytes;
		myWireBase.buf_idx = 0;
		myWireBase.bytesRec = 0;
    retVal = processi2c(TRUE);
    myWireBase.flags = 0;
    return retVal;
}

uint32_t puti2c(uint8_t value) {
  if (myWireBase.buf_idx == BUFFER_LENGTH) 
	{
     myWireBase.buf_overflow = TRUE;
     return 0;
  }
  myWireBase.buf[myWireBase.buf_idx++] = value;
	return 1;
}

uint32_t writei2c(uint8_t* buf, int len) 
{
	uint8_t i = 0;
  for (i = 0; i < len; i++) 
	{
      if (!puti2c(buf[i]))
		  {
				return i;
		  }
  }
	return len;
}

uint8_t availablei2c() 
{
    return myWireBase.bytesRec - myWireBase.buf_idx;
}

uint8_t readi2c() 
{
	uint8_t temp=0;
    if (myWireBase.buf_idx == myWireBase.length) 
		{
        myWireBase.buf_idx = 0;
        myWireBase.length = 0;
        return 0;
    } 
		else if (myWireBase.buf_idx == (myWireBase.length-1)) 
		{
        temp = myWireBase.buf[myWireBase.buf_idx];
        myWireBase.buf_idx = 0;
        myWireBase.length = 0;
        return temp;
    }
    return myWireBase.buf[myWireBase.buf_idx++];
}
