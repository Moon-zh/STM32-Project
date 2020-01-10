#include "IO_BSP.h"

//输入管脚初始化
//X1:PB12 X2:PB13 X3:PB15 X4:PC6  
void DI_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_15;   //GPIOB.12  GPIOB.13  GPIOB.15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB.12  GPIOB.13  GPIOB.15
 	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;   //GPIOC.6 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOC.6
 	
}

//输出管脚初始化
//Y1:PC7  Y2:PC8  Y3:PC9  Y4:PA8 LED:PC1
void DO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;				
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO口输出速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//初始化GPIOC.7
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO口输出速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//初始化GPIOC.8

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO口输出速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//初始化GPIOC.9

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;			
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO口输出速度为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//初始化GPIOA.8

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;				
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO口输出速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//初始化GPIOC.1
}

//IO初始化函数
//X1:PB12 X2:PB13 X3:PB15 X4:PC6  
//Y1:PC7  Y2:PC8  Y3:PC9  Y4:PA8 LED:PC1
void BSP_InitIO(void)
{
	
 	//开时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);	 //开启GPIOA GPIOB GPIOC的时钟
	DI_Init();//对输入IO进行初始化
	DO_Init();//对输出IO进行初始化
	
}

//输出管脚设置，输出0或输出1
u8 ZoneValue[3];
void IO_OutSet(u8 OutPort, u8 Value)
{
	switch(OutPort)
	{
		case 1:   //Y1:PC7 
			if(Value == 0)
			{
				PCout(7) = 0;
			}
			else
			{
				PCout(7) = 1;
			}
			ZoneValue[0] = Value ;
			break;
		
		case 2:      //Y2:PC8
			if(Value == 0)
			{
				PCout(8) = 0;
			}
			else
			{
				PCout(8) = 1;
			}
			ZoneValue[1] = Value ;
			break;
			
		case 3:    // Y3:PC9
			if(Value == 0)
			{
				PCout(9) = 0;
			}
			else
			{
				PCout(9) = 1;
			}
			ZoneValue[2] = Value ;
			break;
		
		case 4:     //Y4:PA8
			if(Value == 0)
			{
				PAout(8) = 0;
			}
			else
			{
				PAout(8) = 1;
			}
			break;
			
		case 5:   // LED:PC1
			if(Value == 0)
			{
				PCout(1) = 0;
			}
			else
			{
				PCout(1) = 1;
			}
			break;
			
		default:
			break;
	}
}
