// Functions to manage the BME280 sensor:
//   - get version
//   - get chip ID
//   - reset the chip
//   - read calibration parameters
//   - configure the chip
//   - read uncompensated values of temperature, pressure and humidity
//   - calculate compensated values of temperature, pressure and humidity
//   - calculate pressure in millimeters of mercury
//   - calculate barometric altitude

#include "bme280.h"

// Carries fine temperature as global value for pressure and humidity calculation
static int32_t t_fine;

// Compensation parameters storage
BME280_Compensation_TypeDef cal_param;

// Write new value to BME280 register
// input:
//   reg - register number
//   value - new register value
void BME280_WriteReg(uint8_t reg, uint8_t value) {
	uint8_t buf[2];
	buf[0]=reg;
	buf[1]=value;

	//I2Cx_Write(BME280_I2C_PORT,buf,2,hasBME280,I2C_STOP);
	i2cMasterTransmit(hasBME280,&buf[0],2,0);
}

// Read BME280 register
// input:
//   reg - register number
// return:
//   register value
uint8_t BME280_ReadReg(uint8_t reg) {
	uint8_t value = 0; // Initialize value in case of I2C timeout
  uint8_t retVal = 0;
	// Send register address
	//I2Cx_Write(BME280_I2C_PORT,&reg,1,hasBME280,I2C_NOSTOP);
	i2cMasterTransmit(hasBME280,&reg,1,1);
	// Read register value
	//I2Cx_Read(BME280_I2C_PORT,&value,1,hasBME280);
	retVal=i2cMasterReceive(hasBME280,&value,1,0);
 
	if (retVal!=0) value=0xff;
	return value;
}

// Check if BME280 present on I2C bus
// return:
//   BME280_SUCCESS if BME280 present, BME280_ERROR otherwise (not present or it was an I2C timeout)
BME280_RESULT BME280_Check(void) {
	return (BME280_ReadReg(BME280_REG_ID) == 0x60) ? BME280_SUCCESS : BME280_ERROR;
}

// Order BME280 to do a software reset
// note: after reset the chip will be unaccessible during 3ms
void BME280_Reset(void) {
	BME280_WriteReg(BME280_REG_RESET,BME280_SOFT_RESET_KEY);
}

// Get version of the BME280 chip
// return:
//   BME280 chip version or zero if no BME280 present on the I2C bus or it was an I2C timeout
uint8_t BME280_GetVersion(void) {
	return BME280_ReadReg(BME280_REG_ID);
}

// Get current status of the BME280 chip
// return:
//   Status of the BME280 chip or zero if no BME280 present on the I2C bus or it was an I2C timeout
uint8_t BME280_GetStatus(void) {
	return BME280_ReadReg(BME280_REG_STATUS) & BME280_STATUS_MSK;
}

// Get current sensor mode of the BME280 chip
// return:
//   Sensor mode of the BME280 chip or zero if no BME280 present on the I2C bus or it was an I2C timeout
uint8_t BME280_GetMode(void) {
	return BME280_ReadReg(BME280_REG_CTRL_MEAS) & BME280_MODE_MSK;
}

// Set sensor mode of the BME280 chip
// input:
//   mode - new mode (BME280_MODE_SLEEP, BME280_MODE_FORCED or BME280_MODE_NORMAL)
void BME280_SetMode(uint8_t mode) {
	uint8_t reg;

	// Read the 'ctrl_meas' (0xF4) register and clear 'mode' bits
	reg = BME280_ReadReg(BME280_REG_CTRL_MEAS) & ~BME280_MODE_MSK;

	// Configure new mode
	reg |= mode & BME280_MODE_MSK;

	// Write value back to the register
	BME280_WriteReg(BME280_REG_CTRL_MEAS,reg);
}

// Set coefficient of the IIR filter
// input:
//   filter - new coefficient value (one of BME280_FILTER_x values)
void BME280_SetFilter(uint8_t filter) {
	uint8_t reg;

	// Read the 'config' (0xF5) register and clear 'filter' bits
	reg = BME280_ReadReg(BME280_REG_CONFIG) & ~BME280_FILTER_MSK;

	// Configure new filter value
	reg |= filter & BME280_FILTER_MSK;

	// Write value back to the register
	BME280_WriteReg(BME280_REG_CONFIG,reg);
}

// Set inactive duration in normal mode (Tstandby)
// input:
//   tsb - new inactive duration (one of BME280_STBY_x values)
void BME280_SetStandby(uint8_t tsb) {
	uint8_t reg;

	// Read the 'config' (0xF5) register and clear 'filter' bits
	reg = BME280_ReadReg(BME280_REG_CONFIG) & ~BME280_STBY_MSK;

	// Configure new standby value
	reg |= tsb & BME280_STBY_MSK;

	// Write value back to the register
	BME280_WriteReg(BME280_REG_CONFIG,reg);
}

// Set oversampling of temperature data
// input:
//   osrs - new oversampling value (one of BME280_OSRS_T_Xx values)
void BME280_SetOSRST(uint8_t osrs) {
	uint8_t reg;

	// Read the 'ctrl_meas' (0xF4) register and clear 'osrs_t' bits
	reg = BME280_ReadReg(BME280_REG_CTRL_MEAS) & ~BME280_OSRS_T_MSK;

	// Configure new oversampling value
	reg |= osrs & BME280_OSRS_T_MSK;

	// Write value back to the register
	BME280_WriteReg(BME280_REG_CTRL_MEAS,reg);
}

// Set oversampling of pressure data
// input:
//   osrs - new oversampling value (one of BME280_OSRS_P_Xx values)
void BME280_SetOSRSP(uint8_t osrs) {
	uint8_t reg;

	// Read the 'ctrl_meas' (0xF4) register and clear 'osrs_p' bits
	reg = BME280_ReadReg(BME280_REG_CTRL_MEAS) & ~BME280_OSRS_P_MSK;

	// Configure new oversampling value
	reg |= osrs & BME280_OSRS_P_MSK;

	// Write value back to the register
	BME280_WriteReg(BME280_REG_CTRL_MEAS,reg);
}

// Set oversampling of humidity data
// input:
//   osrs - new oversampling value (one of BME280_OSRS_H_Xx values)
void BME280_SetOSRSH(uint8_t osrs) {
	uint8_t reg;

	// Read the 'ctrl_hum' (0xF2) register and clear 'osrs_h' bits
	reg = BME280_ReadReg(BME280_REG_CTRL_HUM) & ~BME280_OSRS_H_MSK;

	// Configure new oversampling value
	reg |= osrs & BME280_OSRS_H_MSK;

	// Write value back to the register
	BME280_WriteReg(BME280_REG_CTRL_HUM,reg);

	// Changes to 'ctrl_hum' register only become effective after a write to 'ctrl_meas' register
	// Thus read a value of the 'ctrl_meas' register and write it back after write to the 'ctrl_hum'

	// Read the 'ctrl_meas' (0xF4) register
	reg = BME280_ReadReg(BME280_REG_CTRL_MEAS);

	// Write back value of 'ctrl_meas' register to activate changes in 'ctrl_hum' register
	BME280_WriteReg(BME280_REG_CTRL_MEAS,reg);
}

// Read calibration data
BME280_RESULT BME280_Read_Calibration(void) {
	uint8_t buf[7];
	
	// Read pressure and temperature calibration data (calib00..calib23)
	buf[0] = BME280_REG_CALIB00; // calib00 register address
	//if (I2Cx_Write(BME280_I2C_PORT,&buf[0],1,hasBME280,I2C_NOSTOP) != I2C_SUCCESS) return BME280_ERROR;
	if (i2cMasterTransmit(hasBME280,&buf[0],1,1)!=0) return BME280_ERROR;

	//if (I2Cx_Read(BME280_I2C_PORT,(uint8_t *)&cal_param,24,hasBME280) != I2C_SUCCESS) return BME280_ERROR;
  if (i2cMasterReceive(hasBME280,(uint8_t *)&cal_param,24,0)!=0) return BME280_ERROR;
  //if (i2cMasterReceive(hasBME280,(uint8_t *)&buf[0],24,0)!=0) return BME280_ERROR;

	// Skip one byte (calib24) and read H1 (calib25)
	cal_param.dig_H1 = BME280_ReadReg(BME280_REG_CALIB25);

	// Read humidity calibration data (calib26..calib41)
	buf[0] = BME280_REG_CALIB26; // calib26 register address
	//if (I2Cx_Write(BME280_I2C_PORT,&buf[0],1,hasBME280,I2C_NOSTOP) != I2C_SUCCESS) return BME280_ERROR;
	if (i2cMasterTransmit(hasBME280,&buf[0],1,1)!=0) return BME280_ERROR;
	//if (I2Cx_Read(BME280_I2C_PORT,buf,7,hasBME280) != I2C_SUCCESS) return BME280_ERROR;
  if (i2cMasterReceive(hasBME280,&buf[0],7,0)!=0) return BME280_ERROR;
	
	// Unpack data
	cal_param.dig_H2 = (int16_t)((((int8_t)buf[1]) << 8) | buf[0]);
	cal_param.dig_H3 = buf[2];
	cal_param.dig_H4 = (int16_t)((((int8_t)buf[3]) << 4) | (buf[4] & 0x0f));
	cal_param.dig_H5 = (int16_t)((((int8_t)buf[5]) << 4) | (buf[4]  >>  4));
	cal_param.dig_H6 = (int8_t)buf[6];

	return BME280_SUCCESS;
}

// Read the raw pressure value
// input:
//   UP = pointer to store value
// return:
//   BME280_ERROR in case of I2C timeout, BME280_SUCCESS otherwise
// note: '0x80000' result means no data for pressure (measurement skipped or not ready yet)
BME280_RESULT BME280_Read_UP(int32_t *UP) {
	uint8_t buf[3];

	// Clear result value
	*UP = 0x80000;

	// Send 'press_msb' register address
	buf[0] = BME280_REG_PRESS_MSB;
	//if (!I2Cx_Write(BME280_I2C_PORT,&buf[0],1,hasBME280,I2C_NOSTOP)) return BME280_ERROR;
  if (i2cMasterTransmit(hasBME280,&buf[0],1,1)!=0) return BME280_ERROR;
	// Read the 'press' register (_msb, _lsb, _xlsb)
	//if (I2Cx_Read(BME280_I2C_PORT,&buf[0],3,hasBME280)) {
	//	*UP = (int32_t)((buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4));

  if (i2cMasterReceive(hasBME280,&buf[0],3,0)==0) {
		*UP = (int32_t)((buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4));
		
		return BME280_SUCCESS;
	}

	return BME280_ERROR;
}

// Read the raw temperature value
// input:
//   UT = pointer to store value
// return:
//   BME280_ERROR in case of I2C timeout, BME280_SUCCESS otherwise
// note: '0x80000' result means no data for temperature (measurement skipped or not ready yet)
BME280_RESULT BME280_Read_UT(int32_t *UT) {
	uint8_t buf[3];

	// Clear result value
	*UT = 0x80000;

	// Send 'temp_msb' register address
	buf[0] = BME280_REG_TEMP_MSB;
	//if (!I2Cx_Write(BME280_I2C_PORT,&buf[0],1,hasBME280,I2C_NOSTOP)) return BME280_ERROR;
  if (i2cMasterTransmit(hasBME280,&buf[0],1,1)!=0) return BME280_ERROR;
	
	// Read the 'temp' register (_msb, _lsb, _xlsb)
	//if (I2Cx_Read(BME280_I2C_PORT,&buf[0],3,hasBME280)) {
	//	*UT = (int32_t)((buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4));
  
	if (i2cMasterReceive(hasBME280,&buf[0],3,0)==0) {
		*UT = (int32_t)((buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4));		
		
		return BME280_SUCCESS;
	}

	return BME280_ERROR;
}


// Read the raw humidity value
// input:
//   UH = pointer to store value
// return:
//   BME280_ERROR in case of I2C timeout, BME280_SUCCESS otherwise
// note: '0x8000' result means no data for humidity (measurement skipped or not ready yet)
BME280_RESULT BME280_Read_UH(int32_t *UH) {
	uint8_t buf[2];

	// Clear result value
	*UH = 0x8000;

	// Send 'hum_msb' register address
	buf[0] = BME280_REG_HUM_MSB;
	//if (!I2Cx_Write(BME280_I2C_PORT,&buf[0],1,hasBME280,I2C_NOSTOP)) return BME280_ERROR;
  if (i2cMasterTransmit(hasBME280,&buf[0],1,1)!=0) return BME280_ERROR;
	
	// Read the 'hum' register (_msb, _lsb)
	//if (I2Cx_Read(BME280_I2C_PORT,&buf[0],2,hasBME280)) {
	//	*UH = (int32_t)((buf[0] << 8) | buf[1]);

  if (i2cMasterReceive(hasBME280,&buf[0],2,0)==0) {
		*UH = (int32_t)((buf[0] << 8) | buf[1]);

		return BME280_SUCCESS;
	}

	return BME280_ERROR;
}

// Read all raw values
// input:
//   UT = pointer to store temperature value
//   UP = pointer to store pressure value
//   UH = pointer to store humidity value
// return:
//   BME280_ERROR in case of I2C timeout, BME280_SUCCESS otherwise
// note: 0x80000 value for UT and UP and 0x8000 for UH means no data
BME280_RESULT BME280_Read_UTPH(int32_t *UT, int32_t *UP, int32_t *UH) {
	uint8_t buf[8];

	// Clear result values
	*UT = 0x80000;
	*UP = 0x80000;
	*UH = 0x8000;

	// Send 'press_msb' register address
	buf[0] = BME280_REG_PRESS_MSB;
	//if (!I2Cx_Write(BME280_I2C_PORT,&buf[0],1,hasBME280,I2C_NOSTOP)) return BME280_ERROR;
  if (i2cMasterTransmit(hasBME280,&buf[0],1,1)!=0) return BME280_ERROR;
	
	// Read the 'press', 'temp' and 'hum' registers
	//if (I2Cx_Read(BME280_I2C_PORT,&buf[0],8,hasBME280)) {
  if (i2cMasterReceive(hasBME280,&buf[0],8,0)==0) {		
		*UP = (int32_t)((buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4));
		*UT = (int32_t)((buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4));
		*UH = (int32_t)((buf[6] <<  8) |  buf[7]);

		return BME280_SUCCESS;
	}

	return BME280_ERROR;
}

// Calculate temperature from raw value, resolution is 0.01 degree
// input:
//   UT - raw temperature value
// return: temperature in Celsius degrees (value of '5123' equals '51.23C')
// note: code from the BME280 datasheet (rev 1.1)
int32_t BME280_CalcT(int32_t UT) {
	t_fine  = ((((UT >> 3) - ((int32_t)cal_param.dig_T1 << 1))) * ((int32_t)cal_param.dig_T2)) >> 11;
	t_fine += (((((UT >> 4) - ((int32_t)cal_param.dig_T1)) * ((UT >> 4) - ((int32_t)cal_param.dig_T1))) >> 12) * ((int32_t)cal_param.dig_T3)) >> 14;
	return ((t_fine * 5) + 128) >> 8;
}

// Calculate pressure from raw value, resolution is 0.001 Pa
// input:
//   UP - raw pressure value
// return: pressure in Pa as unsigned 32-bit integer in Q24.8 format (24 integer and 8 fractional bits)
// note: output value of '24674867' represents 24674867/256 = 96386.2 Pa = 963.862 hPa
// note: BME280_CalcT must be called before calling this function
// note: using 64-bit calculations
// note: code from the BME280 datasheet (rev 1.1)
uint32_t BME280_CalcP(int32_t UP) {
#if (BME280_USE_INT64)
	int64_t v1,v2,p;

	v1 = (int64_t)t_fine - 128000;
	v2 = v1 * v1 * (int64_t)cal_param.dig_P6;
	v2 = v2 + ((v1 * (int64_t)cal_param.dig_P5) << 17);
	v2 = v2 + ((int64_t)cal_param.dig_P4 << 35);
	v1 = ((v1 * v1 * (int64_t)cal_param.dig_P3) >> 8) + ((v1 * (int64_t)cal_param.dig_P2) << 12);
	v1 = (((((int64_t)1) << 47) + v1)) * ((int64_t)cal_param.dig_P1) >> 33;
	if (v1 == 0) return 0; // avoid exception caused by division by zero
	p = 1048576 - UP;
	p = (((p << 31) - v2) * 3125) / v1;
	v1 = (((int64_t)cal_param.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
	v2 = (((int64_t)cal_param.dig_P8) * p) >> 19;
	p = ((p + v1 + v2) >> 8) + ((int64_t)cal_param.dig_P7 << 4);
#else // BME280_USE_INT64
	int32_t v1,v2;
	uint32_t p;

	v1 = (((int32_t)t_fine) >> 1) - (int32_t)64000;
	v2 = (((v1 >> 2) * (v1 >> 2)) >> 11 ) * ((int32_t)cal_param.dig_P6);
	v2 = v2 + ((v1 * ((int32_t)cal_param.dig_P5)) << 1);
	v2 = (v2 >> 2) + (((int32_t)cal_param.dig_P4) << 16);
	v1 = (((cal_param.dig_P3 * (((v1 >> 2) * (v1 >> 2)) >> 13 )) >> 3) + ((((int32_t)cal_param.dig_P2) * v1) >> 1)) >> 18;
	v1 = (((32768 + v1)) * ((int32_t)cal_param.dig_P1)) >> 15;
	if (v1 == 0) return 0; // avoid exception caused by division by zero
	p = (((uint32_t)(((int32_t)1048576) - UP) - (v2 >> 12))) * 3125;
	if (p < 0x80000000) {
		p = (p << 1) / ((uint32_t)v1);
	} else {
		p = (p / (uint32_t)v1) << 1;
	}
	v1 = (((int32_t)cal_param.dig_P9) * ((int32_t)(((p >> 3) * (p >> 3)) >> 13))) >> 12;
	v2 = (((int32_t)(p >> 2)) * ((int32_t)cal_param.dig_P8)) >> 13;
	p = (uint32_t)((int32_t)p + ((v1 + v2 + cal_param.dig_P7) >> 4));

	// Convert pressure to Q24.8 format (fractional part always be .000)
	p <<= 8;
#endif // BME280_USE_INT64

	return (uint32_t)p;
}

// Calculate humidity from raw value, resolution is 0.001 %RH
// input:
//   UH - raw humidity value
// return: humidity in %RH as unsigned 32-bit integer in Q22.10 format (22 integer and 10 fractional bits)
// note: output value of '47445' represents 47445/1024 = 46.333 %RH
// note: BME280_CalcT must be called before calling this function
// note: code from the BME280 datasheet (rev 1.1)
uint32_t BME280_CalcH(int32_t UH) {
	int32_t vx1;

	vx1  = t_fine - (int32_t)76800;
	vx1  = ((((UH << 14) - ((int32_t)cal_param.dig_H4 << 20) - ((int32_t)cal_param.dig_H5 * vx1)) +	(int32_t)16384) >> 15) *
			(((((((vx1 * (int32_t)cal_param.dig_H6) >> 10) * (((vx1 * (int32_t)cal_param.dig_H3) >> 11) +
			(int32_t)32768)) >> 10) + (int32_t)2097152) * ((int32_t)cal_param.dig_H2) + 8192) >> 14);
	vx1 -= ((((vx1 >> 15) * (vx1 >> 15)) >> 7) * (int32_t)cal_param.dig_H1) >> 4;
	vx1  = (vx1 < 0) ? 0 : vx1;
	vx1  = (vx1 > 419430400) ? 419430400 : vx1;

	return (uint32_t)(vx1 >> 12);
}

// Fixed point Pa to mmHg conversion (Pascals to millimeters of mercury)
// 1 Pa = 0.00750061683 mmHg
// input:
//    PQ24_8 - pressure in pascals (Q24.8 format, result of BME280_CalcP function)
// return:
//    pressure in millimeter of mercury
// note: return value of '746225' represents 746.225 mmHg
// note: maximum input value is 131071 Pa
uint32_t BME280_Pa_to_mmHg(uint32_t PQ24_8) {
	// Truncate the pressure value to Q24.2 format and multiply by Q0.20 constant (~0.00750061683)
	// The multiply product will be Q10.22 pressure value in mmHg
	uint32_t p_mmHg = (PQ24_8 >> 6) * BME_MMHG_Q0_20;

	// (p_mmHg >> 22) -> integer part from Q10.22 value
	// ((uint32_t)p_mmHg << 10) >> 18 -> fractional part truncated down to 14 bits
	// (XXX * 61039) / 1000000 is rough integer equivalent of float (XXX / 16383.0) * 1000
	return ((p_mmHg >> 22) * 1000) + ((((p_mmHg << 10) >> 18) * 61039) / 1000000);
}

// Convert pressure in Pascals to altitude in millimeters via barometric formula
// input:
//   P - pressure in Pascals
// return: altitude in millimeters
float BME280_Pa_to_Alt(float P) {
	// The value '101325.0' is 'standard' pressure at mean sea level (MSL) with temperature 15C

	// Hypsometric formula (for altitudes below 11km)
	// h = ((powf(P0 / P,1 / 5.257) - 1.0) * (T + 273.15)) / 0.0065

	// Original barometric formula
 	double alt_f = (44330.0f * (1.0f - powf(P / 101325.0f,1 / 5.255f))) * 1000;

	// Replace the powf() function with Taylor series expansion at point P = 101325
	// Using WolframAlpha to convert (44330.0 * (1.0 - powf(P / 101325.0,1 / 5.255)) into Taylor series
	// http://www.wolframalpha.com/input/?i=44330*%281+-+%28%28P%2F101325%29^%281%2F5.255%29%29%29+taylor+series+|+P%3D101325
	// The two terms of the series is enough for good result, take three of them to slightly improve precision
	//   -0.0832546 (P-101325)+3.32651×10^-7 (P-101325)^2-1.98043×10^-12 (P-101325)^3
//	int32_t p1 = P - 101325; // Substitute for equation
//	int32_t p2 = p1 * p1; // (P - 101325)^2
//	int32_t p3 = p1 * p2; // (P - 101325)^3
	// Calculate altitude centered at 'standard' sea level pressure (101325Pa)
//	float alt_t = ((-0.0832546 * p1) + (3.32651E-7 * p2) - (1.98043E-12 * p3)) * 1000.0;
//	float alt_t = ((-0.0832546 * p1) + (3.32651E-7 * p2)) * 1000.0;

	// Taylor series with integers only (centered at 'standard' sea level pressure 101325Pa)
//	int32_t alt_i = 0; // 0th term of series
//	alt_i -= (p1 * 21313) >> 8; // 1th term Q24.8: 0.0832546 * 1000 -> (83 << 8) + (0.2546 * 256) -> 21313
//	alt_i += (((p1 * p1) >> 8) * 22) >> 8; // 2nd term Q24.8: 3.32651E-7 * 1000 * 256 -> 22
	// It can be calculated by only shift and add:
	// ((p2 >> 8) * 22) >> 8 --> substitute (pp = p2 >> 8) --> (pp >> 4) + (pp >> 6) + (pp >> 7)
	// alt_i += (p2 >> 12) + (p2 >> 14) + (p2 >> 15); // 2nd term
	// ...but with same code size this will takes 3 more CPU clocks (STM32 hardware multiplier rocks =)

	// TODO: since the sensor operating range is 300 to 1100hPa, it is worth to center the Taylor series
	//       at other pressure value, something near of 90000Pa

/*
	printf("P: %uPa\t",P);
	printf("alt_f: %i.%03im\talt_t: %i.%03im\t",
			(int32_t)alt_f / 1000,
			(int32_t)alt_f % 1000,
			(int32_t)alt_t / 1000,
			(int32_t)alt_t % 1000
		);
	printf("diff_ft: %i.%03im\t",
			(int32_t)(alt_f - alt_t) / 1000,
			(int32_t)(alt_f - alt_t) % 1000
		);
	printf("diff_fi: %i.%03im\r\n",
			(int32_t)(alt_f - alt_i) / 1000,
			(int32_t)(alt_f - alt_i) % 1000
		);
*/

	return alt_f;

	// Calculation accuracy comparison: powf() vs Taylor series approximation
	//   P:      0Pa  alt_f: 44330.000m  alt_t:  8993.567m  diff: 35336.432m
	//   P:  10000Pa  alt_f: 15799.133m  alt_t:  7520.170m  diff:  8278.962m
	//   P:  40000Pa  alt_f:  7186.406m  alt_t:  4927.885m  diff:  2258.521m
	//   P:  50000Pa  alt_f:  5575.208m  alt_t:  3720.608m  diff:  1854.599m
	//   P:  60000Pa  alt_f:  4207.018m  alt_t:  4008.579m  diff:   198.438m
	//   P:  70000Pa  alt_f:  3012.615m  alt_t:  2934.363m  diff:    78.251m
	//   P:  80000Pa  alt_f:  1949.274m  alt_t:  1926.678m  diff:    22.595m
	//   P:  90000Pa  alt_f:   988.646m  alt_t:   985.524m  diff:     3.122m
	//   P: 100000Pa  alt_f:   110.901m  alt_t:   110.892m  diff:     0.009m
	//   P: 110000Pa  alt_f:  -698.421m  alt_t:  -697.199m  diff:    -1.221m
	//   P: 120000Pa  alt_f: -1450.201m  alt_t: -1438.769m  diff:   -11.432m
	// Thus higher/lower the sensor goes from centered value of 101325Pa the more error will give
	// a Taylor series calculation, but for altitudes in range of +/-2km from MSL this is more than enough

	// For altitudes which noticeably differs from the MSL, the Taylor series can be centered at appropriate value:
	// http://www.wolframalpha.com/input/?i=44330*%281+-+%28%28P%2F101325%29^%281%2F5.255%29%29%29+taylor+series+|+P%3DXXXXXX
	// where XXXXXX - pressure at centered altitude
}

// Calculate temperature from raw value using floats, resolution is 0.01 degree
// input:
//   UT - raw temperature value
// return: temperature in Celsius degrees (value of '51.23' equals '51.23C')
// note: code from the BME280 datasheet (rev 1.1)
float BME280_CalcTf(int32_t UT) {
	float v_x1,v_x2;

	v_x1 = (((float)UT) / 16384.0f - ((float)cal_param.dig_T1) / 1024.0f) * ((float)cal_param.dig_T2);
	v_x2 = ((float)UT) / 131072.0f - ((float)cal_param.dig_T1) / 8192.0f;
	v_x2 = (v_x2 * v_x2) * ((float)cal_param.dig_T3);
	t_fine = (uint32_t)(v_x1 + v_x2);


	return ((v_x1 + v_x2) / 5120.0f);
}

// Calculate pressure from raw value using floats, resolution is 0.001 Pa
// input:
//   UP - raw pressure value
// return: pressure in Pa (value of '99158.968' represents 99158.968Pa)
// note: BME280_CalcT of BME280_CalcTf must be called before calling this function
// note: code from the BME280 datasheet (rev 1.1)
float BME280_CalcPf(uint32_t UP) {
	float v_x1, v_x2, p;

	v_x1 = ((float)t_fine / 2.0f) - 64000.0f;
	v_x2 = v_x1 * v_x1 * ((float)cal_param.dig_P6) / 32768.0f;
	v_x2 = v_x2 + v_x1 * ((float)cal_param.dig_P5) * 2.0f;
	v_x2 = (v_x2 / 4.0f) + (((float)cal_param.dig_P4) * 65536.0f);
	v_x1 = (((float)cal_param.dig_P3) * v_x1 * v_x1 / 524288.0f + ((float)cal_param.dig_P2) * v_x1) / 524288.0f;
	v_x1 = (1.0f + v_x1 / 32768.0f) * ((float)cal_param.dig_P1);
	p = 1048576.0f - (float)UP;
	if (v_x1 == 0) return 0; // Avoid exception caused by division by zero
	p = (p - (v_x2 / 4096.0f)) * 6250.0f / v_x1;
	v_x1 = ((float)cal_param.dig_P9) * p * p / 2147483648.0f;
	v_x2 = p * ((float)cal_param.dig_P8) / 32768.0f;
	p += (v_x1 + v_x2 + ((float)cal_param.dig_P7)) / 16.0f;

	return p;
}

// Calculate humidity from raw value using floats, resolution is 0.001 %RH
// input:
//   UH - raw humidity value
// return: humidity in %RH (value of '46.333' represents 46.333%RH)
// note: BME280_CalcT or BME280_CalcTf must be called before calling this function
// note: code from the BME280 datasheet (rev 1.1)
float BME280_CalcHf(uint32_t UH) {
	float h;

	h = (((float)t_fine) - 76800.0f);
	if (h == 0) return 0;
	h = (UH - (((float)cal_param.dig_H4) * 64.0f + ((float)cal_param.dig_H5) / 16384.0f * h));
	h = h * (((float)cal_param.dig_H2) / 65536.0f * (1.0f + ((float)cal_param.dig_H6) / 67108864.0f * h * (1.0f + ((float)cal_param.dig_H3) / 67108864.0f * h)));
	h = h * (1.0f - ((float)cal_param.dig_H1) * h / 524288.0f);
	if (h > 100.0f) {
		h = 100.0f;
	} else if (h < 0.0f) {
		h = 0.0f;
	}

	return h;
}


void takeForcedMeasurement()
{   
    // If we are in forced mode, the BME sensor goes back to sleep after each
    // measurement and we need to set it to forced mode once at this point, so
    // it will take the next measurement and then return to sleep again.
    // In normal mode simply does new measurements periodically.
    // set to forced mode, i.e. "take next measurement"
	  BME280_SetMode(BME280_MODE_FORCED);
    // wait until measurement has been completed, otherwise we would read
    // the values from the last measurement
    while (BME280_GetStatus() & BME280_STATUS_MEASURING) delay(1);
}


void BME280Update(void)
{
    // RAW temperature, pressure and humidity values
    int32_t UT,UP,UH;
	  char trend=' ';
		if (inI2c!=0) return; // Exit if something already using i2c
	  inI2c=1;
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nBME280 - I2C Update");
		#endif	
	
	  if (cal_param.dig_T2==0)
		{
		  // No calibration yet. Go get it.
			BME280_Read_Calibration();
			/*if (hasUSB)
			{
				usb_printf("\r\nRead calibration:\r\n");
				usb_printf("\tdig_T1:%u\tdig_T2:%i\tdig_T3:%i\r\n",cal_param.dig_T1,cal_param.dig_T2,cal_param.dig_T3);
				usb_printf("\tdig_P1:%u\tdig_P2:%i\tdig_P3:%i\r\n",cal_param.dig_P1,cal_param.dig_P2,cal_param.dig_P3);
				usb_printf("\tdig_P4:%i\tdig_P5:%i\tdig_P6:%i\r\n",cal_param.dig_P4,cal_param.dig_P5,cal_param.dig_P6);
				usb_printf("\tdig_P7:%i\tdig_P8:%i\tdig_P9:%i\r\n",cal_param.dig_P7,cal_param.dig_P8,cal_param.dig_P9);
				usb_printf("\tdig_H1:%u\tdig_H2:%i\tdig_H3:%u\r\n",cal_param.dig_H1,cal_param.dig_H2,cal_param.dig_H3);
				usb_printf("\tdig_H4:%i\tdig_H5:%i\tdig_H6:%i\r\n",cal_param.dig_H4,cal_param.dig_H5,cal_param.dig_H6);				
			}*/
		}
  	// Get all raw readings from the chip
		// Use FORCED MODE
		takeForcedMeasurement();
		
  	BME280_Read_UTPH(&UT,&UP,&UH);
   	// Calculate compensated values
	
		if (UT == 0x80000) 
		{
			// Either temperature measurement is configured as 'skip' or first conversion is not completed yet
			#ifdef doUSB
			usb_printf("\r\nTemp: no data");
			#endif
		} 
		else 
		{
			temperature = BME280_CalcTf(UT);
			#ifdef doUSB
			usb_printf("\r\nBME280 Temp: %.2f Celcius",temperature);
			usb_printf("\r\nBME280 Temp: %.2f Fahrenheit",temperature*1.8f+32);
			#endif
		}

		if (UH == 0x8000) 
		{
			// Either humidity measurement is configured as 'skip' or first conversion is not completed yet
			#ifdef doUSB
			usb_printf("\r\nHumidity: no data");
			#endif
		} 
		else 
		{
			humidity = BME280_CalcHf(UH);

			#ifdef doUSB
			usb_printf("\r\nHumidity: %.2f%%RH",humidity);
			#endif
		}

		if (UT == 0x80000) 
		{
			// Either pressure measurement is configured as 'skip' or first conversion is not completed yet
			#ifdef doUSB
			usb_printf("\r\rnPressure: no data");
			#endif
		} 
		else 
		{
			if (pressureReads==0) 
			{
				if (pressure!=0) 
				{
					firstPressure = pressure;
					pressureReads++;
				}
			}
			else pressureReads++;
			if (pressureReads>72) // About 6 hours ? 
			{
				#ifdef doUSB
				if (hasUSB) usb_printf("\r\nBME280 - Reset Pressure Initial Value");
				#endif
				pressureReads=0; // Calc again after 100 reads
			}
			pressure = BME280_CalcPf(UP);

			#ifdef doUSB
			usb_printf("\r\nPressure: %.2fhPa", pressure / 100);
			#endif
	
			altitude = BME280_Pa_to_Alt(pressure);
			#ifdef doUSB
			usb_printf("\r\nAltitude: %.2f",altitude / 1000);
			#endif
		}	
		inI2c=0;
		if (hasLCD) 
		{
			//delay(5);
			if (firstPressure!=0)
			{
				if (fabs(firstPressure-pressure)<20) trend=126;  //1000.00 to 1000.20. Right Arrow
				else if (firstPressure>pressure) trend=2; //Down
				else trend=1; //Up
			}
			if (lcdMaxRow==4) 
			{
				if (tempUnit==0) lcd_println(3,1,"%.1f%cC %.0f%%RH %.0fP%c",temperature,223,humidity,pressure/100,trend); // Celcius
				else lcd_println(3,1,"%.0f%cF %.0f%%RH %.0fP%c",temperature*1.8f+32,223,humidity,pressure/100,trend); //Fahrenheit
			}
			else 
			{
				if (tempUnit==0) lcd_println(1,1,"%.1f%cC %.1f%%RH",temperature,223,humidity); // Celcius
				else lcd_println(1,1,"%.0f%cF %.1f%%RH",temperature*1.8f+32,223,humidity); //Fahrenheit
			}
		}
}

void BME280Init(void)
{
	uint8_t ver;
	uint32_t i=0;

	#ifdef doUSB
	if (hasUSB) usb_printf("\r\nBME280 Init");	
	#endif
	if (hasLCD) lcd_println(99,1,"Init BME280");
	BME280_Reset();
	#ifdef doUSB
	usb_printf("\r\nBME280 Check");	
	#endif
	while (!(BME280_GetStatus() & 0x01) && i<5000000)
	{
		i++;
	}
	if (i!=5000000)
	{
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nBME280 Init OK");
		#endif
		if (hasLCD) lcd_println(98,1,"Init BME280 - OK");
		ver=BME280_GetVersion();
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nBME280 Version 0x%02X",ver);	
		#endif
		if (ver!=0xff)
		{
			// Get current status of the chip
			i = BME280_GetStatus();
			#ifdef doUSB
			usb_printf("\r\nStatus: [%02X] %s %s",
				i,
				(i & BME280_STATUS_MEASURING) ? "MEASURING" : "READY",
				(i & BME280_STATUS_IM_UPDATE) ? "NVM_UPDATE" : "NVM_READY"
			);
			#endif
			// Set normal mode inactive duration (standby time)
			BME280_SetStandby(BME280_STBY_1s);
			// Set IIR filter constant
			BME280_SetFilter(BME280_FILTER_2);
			// Set oversampling for temperature
			BME280_SetOSRST(BME280_OSRS_T_x2);
			// Set oversampling for pressure
			BME280_SetOSRSP(BME280_OSRS_P_x2);
			// Set oversampling for humidity
			BME280_SetOSRSH(BME280_OSRS_H_x1);
			// Set normal mode (perpetual periodic conversion)
			BME280_SetMode(BME280_MODE_FORCED);	
			delay(500); // Wait for BME280 sensor readings
			//BME280Update();
		}
		else
		{
			#ifdef doUSB
			if (hasUSB) usb_printf("\r\nBME280 ERROR");
			#endif
			if (hasLCD) lcd_println(98,1,"Init BME280 - ERROR");
			hasBME280=0;					
		}
	}
	else
	{
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nBME280 ERROR");
		#endif
		if (hasLCD) lcd_println(98,1,"Init BME280 - ERROR");
		delay(1000);
    hasBME280=0;		
	}
}
