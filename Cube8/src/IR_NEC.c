#include "IR_NEC.h"
#include "ESP8266WIFI.h"

void IR_ENC_Init(uint16_t psc)							//????????
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);		//????
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;					//??????
	GPIO_Init(GPIOB, &GPIO_InitStructure);							//???GPIO???

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);          	//????

	TIM_DeInit(TIM1);
	TIM_TimeBaseStructure.TIM_Period = Step;                 		//???????????????????????????
	TIM_TimeBaseStructure.TIM_Prescaler = (psc - 1);              	//??????TIMx???????????   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;      	//??????:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 	//TIM??????
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);              	//??TIM_TimeBaseInitStruct?????????TIMx???????

	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;        		//TIM1??
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;    	//?????
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;            	//????
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;               	//IRQ?????
	NVIC_Init(&NVIC_InitStructure);                              	//??NVIC_InitStruct???????????NVIC???

	TIM_ClearFlag(TIM1, TIM_FLAG_Update);
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

	TIM_Cmd(TIM1, ENABLE);                                       	//??TIMx??
}

unsigned char	IR_BT;		         													//解码效果返回：0无效，1有效
unsigned char	NEC[4];		         													//解码存放：16位用户码、操作码正反码
unsigned int	cntStep;																//步数计
unsigned char	IRa,IRb;			 													//电位状态保存
unsigned char	IRsync;				 													//同步标志
unsigned char	BitN;                													//位码装载数

/*┈┈┈┈┈┈┈┈┈┈ 基准 ┈┈┈┈┈┈┈┈┈┈┈*/
#define    Boot_Limit	    (((9000+4500) +2000)/Step)	  								//引导码周期上限    
#define    Boot_Lower	    (((9000+4500) -2000)/Step)	  								//引导码周期下限    
#define    Bit1_Limit       ((2250 +800)/Step)	          								//“1”周期上限
#define    Bit0_Limit       ((1125 +400)/Step)			  								//“0”周期上限

/*┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈
函数：红外线NEC周期采样解码法（定时中断，下降沿查询周期时间）
全局变量：IR_BT = 0无效
                 1有效，短按
┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈*/
void IR_NEC(void)
{ 	 
	cntStep++;		   																	//步数采样

	IRb = IRa;		   																	//上次电位状态
  if (ledCubeType==1) IRa = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9);	
	else IRa = GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0);									//当前电位状态

	if(IRb && !IRa)				  	  													//是否下降沿（上次高，当前低）
	{
		if(cntStep > Boot_Limit)      													//超过同步时间？
			IRsync = 0;				  													//同步位清0
		else if(cntStep > Boot_Lower)													//解码开始
		{ 
			IRsync = 1; 
			BitN = 32; 
		}																				//同步位置1，装载位码数			  
		else if(IRsync)																	//如果已同步
		{
			if(cntStep > Bit1_Limit)													//再次判断是否误判
				IRsync = 0;	           
			else
			{	
				NEC[3] >>= 1;				
				if(cntStep > Bit0_Limit)
					NEC[3] |= 0x80;    													//“0”与“1”
                if(--BitN == 0)				
				{
					IRsync = 0;															//同步位清0
					IR_BT = 1; 															//解码成功，成功标志置1
				}
				else if((BitN & 0x07)== 0)												//NEC[3]每装满8位，移动保存一次（即 BitN%8 == 0）
				{	
					NEC[0] = NEC[1]; 
					NEC[1] = NEC[2]; 
					NEC[2] = NEC[3];   
				}
			}
		}
		cntStep = 0;   																	//步数计清0
	}
} 

/*******************************************************************************
 * @name     :void TIM1_IRQHandler(void) 
 * @brief    :
 * @param    :
 * @return   :无
 * @version  :V1.0
 * @author   :创优物联工作室→图腾
 * @date     :2015-6-6
 * @details  :TIM5中断服务函数，对红外是按键进行解码
 *            
*******************************************************************************/
void TIM1_UP_IRQHandler(void)   									//TIM1??
{
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET) 				//?????TIM??????:TIM ??? 
	{
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);  				//??TIMx???????:TIM ??? 

		IR_NEC();													//???NEC???????(????,?????????)
		if (1 == IR_BT)												//????
		{
			KeyHandle(NEC[2]);      										//???????		  
			IR_BT = 0;		        								//?????
		}
	}
}



