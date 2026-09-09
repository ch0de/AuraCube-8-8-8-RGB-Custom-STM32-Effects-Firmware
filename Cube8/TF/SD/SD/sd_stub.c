#include "ff.h"
#include "diskio.h"
#include <stdint.h>

DSTATUS SD_Initialize(BYTE pdrv) { (void)pdrv; return STA_NOINIT; }
DRESULT SD_ReadDisk(BYTE *buff, DWORD sector, UINT count)
{ (void)buff; (void)sector; (void)count; return RES_ERROR; }

void SD_SPI_SpeedLow(void) {}
void SD_SPI_SpeedHigh(void) {}
uint8_t SD_SPI_ReadWriteByte(uint8_t x) { return x; }
