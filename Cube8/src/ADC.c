/*******************************************************************************
 * @file     :☆☆☆彩色光立方☆☆☆
 * @brief    :ADC
 * @mcu      :STM32F103RCT6
 * @version  :V1.0
 * @author   :创优物联工作室→图腾
 * @date     :2015-6-6
*******************************************************************************/
#include "ADC.H"
		   
/*******************************************************************************
 * @name     :void fft_ADC_Init(void)
 * @brief    :
 * @param    :
 * @return   :无
 * @version  :V1.0
 * @author   :创优物联工作室→图腾
 * @date     :2015-6-6
 * @details  :对fft采集的数据进行ADC的初始化
 *            
*******************************************************************************/
void fft_ADC_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC1,ENABLE);	  //使能ADC1通道时钟
 
//	RCC_ADCCLKConfig(RCC_PCLK2_Div6);                                   //72M/6=12,ADC最大时间不能超过14M
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);                                   //128M/8=16,ADC最大时间不能超过14M
	//PC1与2 作为模拟通道输入引脚                         
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		                //模拟输入引脚
	GPIO_Init(GPIOC, &GPIO_InitStructure);	

	ADC_DeInit(ADC1);                                                   //将外设 ADC1 的全部寄存器重设为缺省值

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	                //ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	                    //模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	                //模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	            //ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	                            //顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);	                                //根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   
 
	ADC_Cmd(ADC1, ENABLE);	                                            //使能指定的ADC1
	ADC_ResetCalibration(ADC1);	                                        //重置指定的ADC1的校准寄存器
	while(ADC_GetResetCalibrationStatus(ADC1));	                        //获取ADC1重置校准寄存器的状态,设置状态则等待
	ADC_StartCalibration(ADC1);		                                    //开始指定ADC1的校准状态
	while(ADC_GetCalibrationStatus(ADC1));		                        //获取指定ADC1的校准程序,设置状态则等待
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		                        //使能指定的ADC1的软件转换启动功能
}				  

void fft_AuraADC_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);	  	//??ADC1????

	RCC_ADCCLKConfig(RCC_PCLK2_Div8);                                   			//72M/6=12,ADC????????14M
	//????????                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		                			//??????
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	ADC_DeInit(ADC1);                                                   			//??? ADC1 ????????????

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	                			//ADC????:ADC1?ADC2???????
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	                    			//????????????
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	                			//?????????????
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;				//??????????????
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	            			//ADC?????
	ADC_InitStructure.ADC_NbrOfChannel = 1;	                            			//?????????ADC?????
	ADC_Init(ADC1, &ADC_InitStructure);	                                			//??ADC_InitStruct???????????ADCx????   

	ADC_Cmd(ADC1, ENABLE);	                                            			//?????ADC1
	ADC_ResetCalibration(ADC1);	                                        			//?????ADC1??????
	while (ADC_GetResetCalibrationStatus(ADC1));	                        			//??ADC1??????????,???????
	ADC_StartCalibration(ADC1);		                                    			//????ADC1?????
	while (ADC_GetCalibrationStatus(ADC1));		                        			//????ADC1?????,???????
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		                        			//?????ADC1?????????
}

/*******************************************************************************
 * @name     :uint16_t Get_ADC(uint8_t ch)
 * @brief    :
 * @param    :
 * @return   :无
 * @version  :V1.0
 * @author   :创优物联工作室→图腾
 * @date     :2015-6-6
 * @details  :获取ADC的数据
 *            
*******************************************************************************/
uint16_t Get_ADC(uint8_t ch)   
{
  	//设置指定ADC的规则组通道，设置它们的转化顺序和采样时间
	ADC_RegularChannelConfig(ADC1,ch,1,ADC_SampleTime_1Cycles5);		//ADC1,ADC通道ch,规则采样顺序值为1,采样时间为1.5周期	  			    
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);		                        //使能指定的ADC1的软件转换启动功能	
	while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));						//等待转换结束
	return ADC_GetConversionValue(ADC1);	                            //返回最近一次ADC1规则组的转换结果
}





























