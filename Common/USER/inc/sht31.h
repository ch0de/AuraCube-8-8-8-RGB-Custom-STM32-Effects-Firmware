#ifndef __SHT31_H
#define __SHT31_H

#include "System.h"
#include "LED.h"
#include "i2c.h"

#define SHT31_MEAS_HIGHREP_STRETCH 0x2C06
#define SHT31_MEAS_MEDREP_STRETCH  0x2C0D
#define SHT31_MEAS_LOWREP_STRETCH  0x2C10
#define SHT31_MEAS_HIGHREP         0x2400
#define SHT31_MEAS_MEDREP          0x240B
#define SHT31_MEAS_LOWREP          0x2416
#define SHT31_READSTATUS           0xF32D
#define SHT31_CLEARSTATUS          0x3041
#define SHT31_SOFTRESET            0x30A2
#define SHT31_HEATEREN             0x306D
#define SHT31_HEATERDIS            0x3066

float sht31Temperature(void);
float sht31Humidity(void);
uint16_t sht31Status(void);
void sht31Reset(void);
void sht31Heater(uint8_t);
void sht31Init(void);
uint8_t sht31TempHum(void);

#endif
