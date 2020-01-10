#include "GPS.h"

//GPS插口上
//GPS_EN = USART2_CTS = PA0
//RST_CTRL = USART2_RTS = PA1

void GPS_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//使能GPIOA时钟 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;	//GPS EN和RST
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO口输出速度为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//初始化GPIOA.0   GPIOA.1
	
	GPIO_SetBits(GPIOA,GPIO_Pin_0);//GPIOA.0输出高电平  GPS_EN
	
	GPIO_ResetBits(GPIOA,GPIO_Pin_1);//GPIOA.1输出低电平  RST_CTRL
}

