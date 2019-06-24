#include "timer.h"
#include "led.h"
#include "UserCore.h"
#include "IO_BSP.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//定时器 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////   	
extern MsgStruct MasterMsg;
u8 TIM3SetFlag = 0;
//u16 water_time =0;
//u16 
//通用定时器3中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器3!
void TIM3_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	if(TIM3SetFlag == 1)
		return;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器


	TIM_Cmd(TIM3, ENABLE);  //使能TIMx					 
	TIM3SetFlag = 1;
}
/*
	*超时定时器
	*输入：无
	*输出：无
	*作用:在超时的时候计算超时时间。
*/
 void TIM4_Config(void)  //定时器中断配置为0组，从响应优先级2开始  计时1min中断1次
{
 	 TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure; 
	 NVIC_InitTypeDef NVIC_InitStructure;  
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);	//使能TIM4时钟
   TIM_DeInit(TIM4);									//恢复到默认值
   TIM_TimeBaseStructure.TIM_Period = 9999; //设置自动装载寄存器		 
   TIM_TimeBaseStructure.TIM_Prescaler =7199; //分频计数 
   TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //选择向上计数
   TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);			//结构体初始化
   TIM_ClearFlag(TIM4, TIM_FLAG_Update);				//清tim4标志
   TIM_ITConfig(TIM4,  TIM_IT_Update, ENABLE);			//定时器4中断使能
   TIM_Cmd(TIM4,ENABLE);								//使能定时器4
	 TIM_Cmd(TIM4,DISABLE);       //失能定时器4
	
	 NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);            //0组，全副优先级          
	 NVIC_InitStructure.NVIC_IRQChannel= TIM4_IRQn;             //选择中断通道，库P166页     
	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;    //抢占优先级0  
	 NVIC_InitStructure.NVIC_IRQChannelSubPriority=2;           //响应优先级0    
	 NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;              //启动此通道的中断   
	 NVIC_Init(&NVIC_InitStructure);                            //结构体初始化       
	 TIM_Cmd(TIM4,ENABLE);								//使能定时器4
 }
void TIM3_STOP(void)
{
	TIM3SetFlag = 0;
	TIM_Cmd(TIM3, DISABLE);  //取消计时					 
}
u8 LED_Statue = 0;
//定时器3中断服务程序
void TIM3_IRQHandler(void)   //TIM3中断
{
//	MsgStruct Msgtemp;
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx更新中断标志 
		IO_OutSet(5,LED_Statue);
		LED_Statue ^= 1;
	}
}

void TIM4_IRQHandler(void) //超时报文命令
{
	TIM_ClearITPendingBit(TIM4, TIM_FLAG_Update);
}










