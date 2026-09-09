#include "LEDHardware.h"
#include "LED.h"
#include "exfuns.h"

int8_t debug;
uint8_t rgbled[5][ledArraySize];
uint8_t rgb_order[192];
uint8_t rgb_order_transfer[192];

// Determine the LED CUBE board type
// Cube8(RGB Cube) = 0
// Aura Cube8 = 1
// RGBCube12 = 2
// RGBCube12 Config b = 3
uint8_t ledCubeType = 1;
uint8_t bjCubeType = 0;

uint8_t totalMax=8; // The Biggest number of pixels in any one dimension
uint8_t totalMin=8; // The Smallest number of pixels in any one dimension
uint8_t myTotalX=8; // Total Size of This Cube
uint8_t myTotalY=8;
uint8_t myTotalZ=8;
uint8_t myCentreX=4; // Centre of this Cube
uint8_t myCentreY=4;
uint8_t myCentreZ=4;
uint8_t mySize=8; // Size of this cube
uint16_t myTotalPixels=512;

uint8_t totalX=8; // Total Size of All Cubes
uint8_t totalY=8;
uint8_t totalZ=8;
uint8_t curTotalX=8;
uint8_t curTotalY=8;
uint8_t curTotalZ=8;
uint8_t curCentreX=4;
uint8_t curCentreY=4;
uint8_t curCentreZ=4;
uint8_t trimTotalX=8;
uint8_t trimTotalY=8;
uint8_t trimTotalZ=8;
uint16_t totalPixels=512;

uint8_t layerColourSame=0; // Not used on this cube
uint8_t musicPlay=0; // Not used on this cube
uint8_t currentPlay=0; // Not used on this cube
uint8_t musicPlaying=0; // Not used on this cube

uint8_t ADC_MODE = 0;
uint8_t ADCRunning = 0; // Not used on this cube

uint32_t ESP8266BAUD=1000000;
uint8_t timeMultipler=167;

uint8_t cubeLayout[layoutMaxX][layoutMaxY][layoutMaxZ]; // Layout of Addresses to Cubes. Which Cube is where in the Grid

/*******************************************************************************
 * @name     :void NVIC_Configuration(void)
 * @brief    :
 * @param    :
 * @return   :��
 * @version  :V1.0
 * @author   :�������������ҡ�ͼ��
 * @date     :2015-5-31
 * @details  :���ȼ��������ȼ�����
 *            
*******************************************************************************/
void NVIC_Configuration(void) 
{ 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
}

void clearLayerTimings(void)
{
	// Not implemented on this cube due to memory and CPU constraints
}

void setLayerTimings(uint8_t y,uint8_t R,uint8_t G,uint8_t B)
{
	// Not implemented on this cube due to memory and CPU constraints
}

void copyDMABuffer(uint8_t addr, uint8_t from, uint8_t to)
{
	// Not implemented on this cube due to memory and CPU constraints
}

void copyDMABuffers(uint8_t from, uint8_t to)
{
	// Not implemented on this cube due to memory and CPU constraints
}

/*******************************************************************************
 * @name     :void Anode(uint8_t temp)
 * @brief    :
 * @param    :temp�����ͨ���β�
 * @return   :��
 * @version  :V1.0
 * @author   :�������������ҡ�ͼ��
 * @date     :2015-5-31
 * @details  :��ѡ��
 *            
*******************************************************************************/
void Anode(uint8_t temp)    
{
    uint8_t i = 0;

    O_ST_RESET;																	//HC595���������棬Ϊ�γ���������׼��    
    for(i = 0; i < 8; i++)
    {
        O_CLK_RESET;															//HC595�����ݴ��䣬Ϊ�γ���������׼��
        if(0 == (temp >> i & 0x01))												//�ж������Ƿ�Ϊ0
            O_DATA_SET;															//����Ϊ0������ߵ�ƽ
        else
            O_DATA_RESET;														//����Ϊ1������͵�ƽ
        O_CLK_SET;																//HC595�����ݴ��䣬�γ�������
    }
    O_ST_SET;  																	//HC595���������棬�γ�������
}

void AnodeAura(uint8_t temp)
{
	if (temp & 0x01)
		HC138_A_SET;
	else
		HC138_A_RESET;

	if (temp & 0x02)
		HC138_B_SET;
	else
		HC138_B_RESET;

	if (temp & 0x04)
		HC138_C_SET;
	else
		HC138_C_RESET;
}

void changeRGBOrder()
{
	uint8_t temp_rgb_order[192];	
	uint8_t x,z;
	uint8_t count_i;
	uint8_t temp_i;
  for (x=0;x<8;x++)
	{
		for (z=0;z<8;z++)
		{
			  count_i = (7-x)*3+z*24;									//���㵱ǰ�ĵ�������λ��
			  if (cubeDirection==0||cubeDirection==4)
					// normal
			  {
				  temp_i=count_i;
			  }
				if (cubeDirection==1||cubeDirection==5)
			  {
					// backwards
				  temp_i=(7-x)*3+(7-z)*24;
			  }
				if (cubeDirection==2||cubeDirection==6)
			  {
					// invert
				  temp_i=x*3+z*24;
			  }
				if (cubeDirection==3||cubeDirection==7)
			  {
					// invert & backwards
				  temp_i=x*3+(7-z)*24;
			  }
				temp_rgb_order[count_i+0]=rgb_order[temp_i+0];
				temp_rgb_order[count_i+1]=rgb_order[temp_i+1];
				temp_rgb_order[count_i+2]=rgb_order[temp_i+2];
		}
	}
  for (x=0;x<192;x++)
	{
		rgb_order[x]=temp_rgb_order[x];
	}
}

/*******************************************************************************
 * @name     :const uint8_t rgb_order[192]
 * @brief    :
 * @param    :
 * @return   :��
 * @version  :V1.0
 * @author   :�������������ҡ�ͼ��
 * @date     :2015-5-31
 * @details  :ˢ�����ݵ�˳��
 *            
*******************************************************************************/
void initRGBOrder()
{
		// the rgb circuit is different between rgb and aura cubes
	static const struct rgb_order_access cube_rgb_order =   //CUBE8
	{{
15,14,13,12,11,10,9,8,7,5,6,4,2,3,1,0,23,22,21,20,19,16,18,17,
47,46,45,44,43,42,41,40,39,37,38,36,34,35,33,32,24,25,27,26,28,29,31,30,
63,62,61,60,59,58,57,56,55,53,54,52,50,51,49,48,71,70,69,68,67,64,66,65,
95,94,93,92,91,90,89,88,87,85,86,84,82,83,81,80,72,73,75,74,76,77,79,78,
111,110,109,108,107,106,105,104,103,101,102,100,98,99,97,96,119,118,117,116,115,112,114,113,
143,142,141,140,139,138,137,136,135,132,134,133,130,131,129,128,120,121,123,122,124,125,127,126,
159,158,157,156,155,154,153,152,151,148,150,149,145,144,146,147,167,166,165,164,163,160,162,161,
191,190,189,188,187,186,184,185,183,180,182,181,178,179,177,176,168,169,171,170,172,173,175,174
	}};
	static const struct rgb_order_access cube_aura_order = 													//Aura LED
{{
190,191,189,187,188,186,184,185,183,181,182,180,178,179,177,167,176,166,164,165,163,161,162,160,
158,159,157,155,156,154,152,153,151,149,150,148,146,147,145,168,144,169,171,170,172,174,173,175,
142,143,141,139,140,138,136,137,135,133,134,132,130,131,129,119,128,118,116,117,115,113,114,112,
110,111,109,107,108,106,104,105,103,101,102,100,98,99,97,120,96,121,123,122,124,126,125,127,
94,95,93,91,92,90,88,89,87,85,86,84,82,83,81,71,80,70,68,69,67,65,66,64,
62,63,61,59,60,58,56,57,55,53,54,52,50,51,49,72,48,73,75,74,76,78,77,79,
46,47,45,43,44,42,40,41,39,37,38,36,34,35,33,23,32,22,20,21,19,17,18,16,
14,15,13,11,12,10,8,9,7,4,5,6,2,3,1,24,0,25,27,26,28,30,29,31,
}};	

	if (ledCubeType==0) *(struct rgb_order_access*)rgb_order = cube_rgb_order;
	if (ledCubeType==1) *(struct rgb_order_access*)rgb_order = cube_aura_order;
	*(struct rgb_order_access*)rgb_order_transfer = cube_aura_order; // For transfers to Cube8 we need to know the aura order
    changeRGBOrder();
}
	
/*******************************************************************************
 * @name     :void Send_Data(uint8_t rgbBit,uint8_t layerA)
 * @brief    :
 * @param    :rgbBit��4λ��ȵĵڼ�λ
 * @param    :layerA�����ݵĵڼ���
 * @return   :��
 * @version  :V1.0
 * @author   :�������������ҡ�ͼ��
 * @date     :2015-5-31
 * @details  :���ݵ�ˢ��
 *            
*******************************************************************************/
void Send_Data(uint8_t rgbBit,uint8_t layerA)
{
	uint8_t i = 0;
	uint8_t BitRGB = 0;
	uint16_t Sum = 0;
	layerA &= 0x07;																//����8��֮��
	if (cubeDirection>3) Sum = (7-layerA) * 192;															//????????
	else Sum = layerA * 192;	
	BitRGB = 0x01 << rgbBit;									//��ȡ��һλ����ʱ����  
	rgb_ST_RESET;		//��������������õ͵�ƽ���������ݱ����������������
	if (showLED != 3)
	{
	if (eNum & 8) 
	{
		 for(i = 0; i < 192; i++)
     {
				rgb_CLK_RESET;															//���õͣ�Ϊ�γ�������׼��
				if((rgbled[0][i+Sum]) & BitRGB || (rgbled[1][i+Sum]) & BitRGB) 
				   rgb_DATA_SET;														//Ϊ������ߵ�ƽ
				else
					 rgb_DATA_RESET;														//Ϊ������͵�ƽ
				rgb_CLK_SET;															//�øߣ��γ������أ���������		   
			}		
	}
	else
	{
		 for(i = 0; i < 192; i++)
     {
				rgb_CLK_RESET;															//���õͣ�Ϊ�γ�������׼��
				if((rgbled[showLED][i+Sum]) & BitRGB)  
				   rgb_DATA_SET;														//Ϊ������ߵ�ƽ
				else
					 rgb_DATA_RESET;														//Ϊ������͵�ƽ
				rgb_CLK_SET;															//�øߣ��γ������أ���������		   
			}
	}	
}	
	else
	{
		 for(i = 0; i < 192; i++)
     {
				rgb_CLK_RESET;															//���õͣ�Ϊ�γ�������׼��
 			  rgb_DATA_RESET;														//Ϊ������͵�ƽ
				rgb_CLK_SET;															//�øߣ��γ������أ���������		   
			}		
  }	
	rgb_ST_SET;																	//������������øߵ�ƽ�������������
}

void Send_DataAura(uint8_t rgbBit, uint8_t layerA)
{
	uint8_t i = 0, j = 0;
	uint16_t Sum = 0;
	if (cubeDirection>3) Sum = (7-layerA) * 192;															//????????
	else Sum = layerA * 192;
	if (showLED != 3)
	{
		if (eNum & 8)
		{
			for (j = 0; j < AuraCubes; j++)
			{
				for (i = 0; i < 96; i++)
				{
					if ((rgbled[0][i + Sum]) & rgbBit || (rgbled[1][i + Sum]) & rgbBit)  
						aura_DAT_B_SET;													//???????
					else
						aura_DAT_B_RESET;												//???????

					if ((rgbled[0][i + 96 + Sum]) & rgbBit || (rgbled[1][i + 96 + Sum]) & rgbBit) 
						aura_DAT_A_SET;													//???????
					else
						aura_DAT_A_RESET;												//???????
					
					aura_CLK_RESET;														//???,????????
					aura_CLK_SET;														//??,?????,????		   
				}
			}
		}
		else
		{
			for (j = 0; j < AuraCubes; j++)
			{
				for (i = 0; i < 96; i++)
				{
					if ((rgbled[showLED][i + Sum]) & rgbBit)  
						aura_DAT_B_SET;													//???????
					else
						aura_DAT_B_RESET;												//???????

					if ((rgbled[showLED][i + 96 + Sum]) & rgbBit)  
						aura_DAT_A_SET;													//???????
					else
						aura_DAT_A_RESET;												//???????
					
					aura_CLK_RESET;														//???,????????
					aura_CLK_SET;														//??,?????,????		   
				}
			}
		}
	}
	else
	{
		aura_DAT_B_RESET;															//???????
		aura_DAT_A_RESET;															//???????
		for (j = 0; j < AuraCubes; j++)
		{
			for (i = 0; i < 96; i++)
			{
				aura_CLK_RESET;															//???,????????
				aura_CLK_SET;															//??,?????,????		   
			}
		}
	}
	aura_ST_RESET;																	//???????????,?????????????
	aura_ST_SET;																		//??????????,??????
}

void set(uint8_t x,uint8_t y,uint8_t z,uint8_t R,uint8_t G,uint8_t B)
{
	uint16_t count_i = 0,count_j = 0;
	uint8_t tempSet=0;
	tempSet=setLED;
	
	if (extendCubes)
	{
		if (onThisCube(x,y,z)==0) tempSet=3; // other cube
		// Off the edge of this cube
		if (x>=myTotalX) x=x-myTotalX;  
		if (y>=myTotalY) y=y-myTotalY;
		if (z>=myTotalZ) z=z-myTotalZ;	
		if (x>=myTotalX || y>=myTotalY || z>=myTotalZ )
		{
       // still off the cube - try address 2 - fudge
			if (debug==0) tempSet=4;
			if (x>=myTotalX) x=x-myTotalX;  
			if (y>=myTotalY) y=y-myTotalY;
			if (z>=myTotalZ) z=z-myTotalZ;	
		}			
  }		
	
	if (x>=myTotalX || y>=myTotalY || z>=myTotalZ || x>=curTotalX || y>=curTotalY || z>=curTotalZ) 
	{
		#ifdef doUSB
		if (hasUSB) usb_printf("\r\nOff the Cube on %d for %d,%d,%d",setLED,x,y,z);
		#endif
		return; // Off the cube
	}	
	
	count_i = ((mySize-1)-x)*3+z*(mySize*3);
	count_j = ((mySize*mySize)*3)*((mySize-1)-y);  

	if (setAdd==0)
	{		
		rgbled[tempSet][rgb_order[count_i+ROffset]+count_j] = R;	
		rgbled[tempSet][rgb_order[count_i+GOffset]+count_j] = G;	
		rgbled[tempSet][rgb_order[count_i+BOffset]+count_j] = B;	

	}
	else
	{
		rgbled[tempSet][rgb_order[count_i+ROffset]+count_j] += R;		
		rgbled[tempSet][rgb_order[count_i+GOffset]+count_j] += G;		
		rgbled[tempSet][rgb_order[count_i+BOffset]+count_j] += B;		
		if (rgbled[tempSet][rgb_order[count_i+ROffset]+count_j]>255) rgbled[tempSet][rgb_order[count_i+ROffset]+count_j]=255;
		if (rgbled[tempSet][rgb_order[count_i+GOffset]+count_j]>255) rgbled[tempSet][rgb_order[count_i+GOffset]+count_j]=255;
		if (rgbled[tempSet][rgb_order[count_i+BOffset]+count_j]>255) rgbled[tempSet][rgb_order[count_i+BOffset]+count_j]=255;		
	}
}

uint8_t getR(uint8_t x,uint8_t y,uint8_t z)
{
	uint8_t R = 0;
	uint16_t count_i = 0,count_j = 0;
	uint8_t tempSet=0;
	tempSet=setLED;
	outOfBounds=0;
	
	if (extendCubes)
	{
		if (onThisCube(x,y,z)==0) tempSet=3; // other cube
		// Off the edge of this cube
		if (x>=myTotalX) x=x-myTotalX;  
		if (y>=myTotalY) y=y-myTotalY;
		if (z>=myTotalZ) z=z-myTotalZ;	
		if (x>=myTotalX || y>=myTotalY || z>=myTotalZ )
		{
       // still off the cube - try address 2 - fudge
			if (debug==0) tempSet=4;
			if (x>=myTotalX) x=x-myTotalX;  
			if (y>=myTotalY) y=y-myTotalY;
			if (z>=myTotalZ) z=z-myTotalZ;	
		}			
  }	
	
	if (x>=myTotalX || y>=myTotalY || z>=myTotalZ || x>=curTotalX || y>=curTotalY || z>=curTotalZ) 
	{
		outOfBounds=1;
		return 0; // Off the cube
	}
	count_i = ((mySize-1)-x)*3+z*(mySize*3);
	count_j = ((mySize*mySize)*3)*((mySize-1)-y);  

	R = rgbled[tempSet][rgb_order[count_i+ROffset]+count_j];
	return R;
}

uint8_t getG(uint8_t x,uint8_t y,uint8_t z)
{
	uint8_t G = 0;
	uint16_t count_i = 0,count_j = 0;
	uint8_t tempSet=0;
	tempSet=setLED;
  outOfBounds=0;
	
	if (extendCubes)
	{
		if (onThisCube(x,y,z)==0) tempSet=3; // other cube
		// Off the edge of this cube
		if (x>=myTotalX) x=x-myTotalX;  
		if (y>=myTotalY) y=y-myTotalY;
		if (z>=myTotalZ) z=z-myTotalZ;	
		if (x>=myTotalX || y>=myTotalY || z>=myTotalZ )
		{
       // still off the cube - try address 2 - fudge
			if (debug==0) tempSet=4;
			if (x>=myTotalX) x=x-myTotalX;  
			if (y>=myTotalY) y=y-myTotalY;
			if (z>=myTotalZ) z=z-myTotalZ;	
		}			
  }	
	
	if (x>=myTotalX || y>=myTotalY || z>=myTotalZ || x>=curTotalX || y>=curTotalY || z>=curTotalZ) 
	{
		outOfBounds=1;
		return 0; // Off the cube
	}
	count_i = ((mySize-1)-x)*3+z*(mySize*3);
	count_j = ((mySize*mySize)*3)*((mySize-1)-y);  

	G = rgbled[tempSet][rgb_order[count_i+GOffset]+count_j];
	return G;
}

uint8_t getB(uint8_t x,uint8_t y,uint8_t z)
{
	uint8_t B = 0;
	uint16_t count_i = 0,count_j = 0;
	uint8_t tempSet=0;
	tempSet=setLED;
  outOfBounds=0;
	
	if (extendCubes)
	{
		if (onThisCube(x,y,z)==0) tempSet=3; // other cube
		// Off the edge of this cube
		if (x>=myTotalX) x=x-myTotalX;  
		if (y>=myTotalY) y=y-myTotalY;
		if (z>=myTotalZ) z=z-myTotalZ;	
		if (x>=myTotalX || y>=myTotalY || z>=myTotalZ )
		{
       // still off the cube - try address 2 - fudge
			if (debug==0) tempSet=4;
			if (x>=myTotalX) x=x-myTotalX;  
			if (y>=myTotalY) y=y-myTotalY;
			if (z>=myTotalZ) z=z-myTotalZ;	
		}			
  }	
	
	if (x>=myTotalX || y>=myTotalY || z>=myTotalZ || x>=curTotalX || y>=curTotalY || z>=curTotalZ) 
	{
		outOfBounds=1;
		return 0; // Off the cube
	}
	count_i = ((mySize-1)-x)*3+z*(mySize*3);
	count_j = ((mySize*mySize)*3)*((mySize-1)-y);  

	B = rgbled[tempSet][rgb_order[count_i+BOffset]+count_j];
	return B;
}

void getRGB(uint8_t x,uint8_t y,uint8_t z,uint8_t *R,uint8_t *G,uint8_t *B)
{
	uint16_t count_i = 0,count_j = 0;
	uint8_t tempSet=0;
	tempSet=setLED;
  outOfBounds=0;
	
	if (extendCubes)
	{
		if (onThisCube(x,y,z)==0) tempSet=3; // other cube
		// Off the edge of this cube
		if (x>=myTotalX) x=x-myTotalX;  
		if (y>=myTotalY) y=y-myTotalY;
		if (z>=myTotalZ) z=z-myTotalZ;	
		if (x>=myTotalX || y>=myTotalY || z>=myTotalZ )
		{
       // still off the cube - try address 2 - fudge
			if (debug==0) tempSet=4;
			if (x>=myTotalX) x=x-myTotalX;  
			if (y>=myTotalY) y=y-myTotalY;
			if (z>=myTotalZ) z=z-myTotalZ;	
		}			
  }	
	
	if (x>=myTotalX || y>=myTotalY || z>=myTotalZ || x>=curTotalX || y>=curTotalY || z>=curTotalZ) 
	{
		outOfBounds=1;
		return; // Off the cube
	}
	count_i = ((mySize-1)-x)*3+z*(mySize*3);
	count_j = ((mySize*mySize)*3)*((mySize-1)-y);  
	
	*R = rgbled[tempSet][rgb_order[count_i+ROffset]+count_j];
	*G = rgbled[tempSet][rgb_order[count_i+GOffset]+count_j];
	*B = rgbled[tempSet][rgb_order[count_i+BOffset]+count_j];
}

/*******************************************************************************
 * @name     :void AxesConvert(void)
 * @brief    :
 * @param    :
 * @return   :��
 * @version  :V1.0
 * @author   :�������������ҡ�ͼ��
 * @date     :2015-6-6
 * @details  :������ת������������
 *            
*******************************************************************************/
void AxesConvert(void)
{				
	char i,j;
	int nn,mm,kk;
	for(j = 0; j < 8; j++)
	{
		mm = 192*j;
		nn = 32*j;
		for(i = 0; i < 32; i++)
		{
			kk = 6*i;
			rgbled[setLED][rgb_order[kk+0]+mm] =  (fatbuf[i+nn] & 0x0f)<<customBright;				//(��ɫ)ȡ���ݵĵ�4λ				
			rgbled[setLED][rgb_order[kk+3]+mm] = ((fatbuf[i+nn] >> 4)& 0x0f)<<customBright;			//(��ɫ)ȡ���ݵĸ�4λ				
			rgbled[setLED][rgb_order[kk+1]+mm] =  (fatbuf[256+i+nn] & 0x0f)<<customBright;			//(��ɫ)ȡ���ݵĵ�4λ				
			rgbled[setLED][rgb_order[kk+4]+mm] = ((fatbuf[256+i+nn] >> 4)& 0x0f)<<customBright;		//(��ɫ)ȡ���ݵĸ�4λ				
			rgbled[setLED][rgb_order[kk+2]+mm] =  (fatbuf[512+i+nn] & 0x0f)<<customBright;			//(��ɫ)ȡ���ݵĵ�4λ				
			rgbled[setLED][rgb_order[kk+5]+mm] = ((fatbuf[512+i+nn] >> 4)& 0x0f)<<customBright;		//(��ɫ)ȡ���ݵĸ�4λ				
		
			}				
	}					
}	

void copyEffectBuffer(uint8_t to, uint8_t from, uint8_t extra,float level)
{
	// Not Used on Aura8
}

void copyDMAfromRGB(void)
{
	// Not Used on Aura8
}

void startGames(void)
{
	// Not Used on Aura8
}

void stopGames(void)
{
	// Not Used on Aura8
}

void processButton(uint8_t buttonNumber,uint8_t fire1,uint8_t fire2)
{
	// Not Used on Aura8
}

void folderRunTune(uint8_t startIRQ,FRESULT res)
{
	// Not Used on Aura8	
}

void stopTune(uint8_t stopIRQ)
{
	// Not Used on Aura8	
}

void stopWave(void)
{
	// Not Used on Aura8	
}

void delayBeat(uint32_t ms,uint32_t timeOut, uint8_t extraDelay)
{
	// Only Used for regular delay on Aura8
	if (extraDelay)
	{
		delay(ms);
	}	
}

void playNext(void)
{
	// Not Used on Aura8
}

void stopSongs(void)
{
	// Not Used on Aura8	
}

/*******************************************************************************
 * @name     :void randomColour(int8_t *gray_red,int8_t *gray_green,int8_t *gray_blue)
 * @brief    :
 * @param    :
 * @return   :��
 * @version  :V1.0
 * @author   :�������������ҡ�ͼ��
 * @date     :2015-6-8
 * @details  :ȡ���������ɫ����
 *            
*******************************************************************************/
void randomColour(uint8_t *gray_red,uint8_t *gray_green,uint8_t *gray_blue)
{
	while(1)																	//ȡ�������ɫ
	{
		srand(5*randCount+7*TIM1->CNT);											//������
		*gray_red = (int8_t)(rand()%256);										//ȡ�����
		srand(3*randCount+9*TIM1->CNT);											//������
		*gray_green = rand()%256;												//ȡ�����
		srand(7*randCount+5*TIM1->CNT);											//������
		*gray_blue = rand()%256;													//ȡ�����
		if(*gray_red != 0 || *gray_green != 0 || *gray_blue != 0)				//ȷ������ȡ��0,0,0
			break;
	}
}

uint32_t cube_random(uint8_t max)
{
    return (rand()%max);
}

uint32_t random32(uint32_t max)
{
    return (rand()%max);
}

u8 mf_read_to_dma(FIL *fp,uint8_t address)
{
  // Not used on this AURA8		
	return 0;
}

void executeComExtend(char* option, char* value)
{
	// Not used on this AURA8	
}

void checkMusic(void)
{
	// Not used on this AURA8	
}

/*******************************************************************************
 * @name     :void GPIO_Configuration(void)
 * @brief    :
 * @param    :
 * @return   :��
 * @version  :V1.0
 * @author   :�������������ҡ�ͼ��
 * @date     :2015-5-31
 * @details  :GPIO�ӿ�ģʽ�����ó�ʼ��
 *            
*******************************************************************************/
void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;	

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB |  \
                            RCC_APB2Periph_GPIOC, ENABLE);//����GPIOx��ʱ��
    //��ʼ��GPIOB
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    //��ʼ��GPIOC
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void GPIO_AuraConfiguration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);//??GPIOx???

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}
