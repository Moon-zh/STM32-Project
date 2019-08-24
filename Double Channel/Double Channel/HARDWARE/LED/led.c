#include "led.h"
//初始化PB5和PE5为输出口.并使能这两个口的时钟		    
//LED IO初始化
void LED_Init(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;				//LED0-->PB.5 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//根据设定参数初始化GPIOB.5
	//GPIO_SetBits(GPIOC,GPIO_Pin_7);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				//LED0-->PB.5 端口配置
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//根据设定参数初始化GPIOB.5
	//GPIO_SetBits(GPIOC,GPIO_Pin_8);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				//LED0-->PB.5 端口配置
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//根据设定参数初始化GPIOB.5
	//GPIO_SetBits(GPIOC,GPIO_Pin_9);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				//LED0-->PB.5 端口配置
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//根据设定参数初始化GPIOB.5
	//GPIO_SetBits(GPIOA,GPIO_Pin_8);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;				//LED0-->PB.5 端口配置
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//根据设定参数初始化GPIOB.5
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOE2,3,4
	//GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	
 	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOE2,3,4
}
 
