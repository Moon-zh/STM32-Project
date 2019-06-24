#include "timer.h"
#include "led.h"
#include "UserCore.h"
#include "IO_BSP.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//��ʱ�� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/3
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////   	
extern MsgStruct MasterMsg;
u8 TIM3SetFlag = 0;
//u16 water_time =0;
//u16 
//ͨ�ö�ʱ��3�жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��3!
void TIM3_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	if(TIM3SetFlag == 1)
		return;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʱ��ʹ��
	
	//��ʱ��TIM3��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM3�ж�,��������ж�

	//�ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //��ʼ��NVIC�Ĵ���


	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx					 
	TIM3SetFlag = 1;
}
/*
	*��ʱ��ʱ��
	*���룺��
	*�������
	*����:�ڳ�ʱ��ʱ����㳬ʱʱ�䡣
*/
 void TIM4_Config(void)  //��ʱ���ж�����Ϊ0�飬����Ӧ���ȼ�2��ʼ  ��ʱ1min�ж�1��
{
 	 TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure; 
	 NVIC_InitTypeDef NVIC_InitStructure;  
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);	//ʹ��TIM4ʱ��
   TIM_DeInit(TIM4);									//�ָ���Ĭ��ֵ
   TIM_TimeBaseStructure.TIM_Period = 9999; //�����Զ�װ�ؼĴ���		 
   TIM_TimeBaseStructure.TIM_Prescaler =7199; //��Ƶ���� 
   TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //ѡ�����ϼ���
   TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);			//�ṹ���ʼ��
   TIM_ClearFlag(TIM4, TIM_FLAG_Update);				//��tim4��־
   TIM_ITConfig(TIM4,  TIM_IT_Update, ENABLE);			//��ʱ��4�ж�ʹ��
   TIM_Cmd(TIM4,ENABLE);								//ʹ�ܶ�ʱ��4
	 TIM_Cmd(TIM4,DISABLE);       //ʧ�ܶ�ʱ��4
	
	 NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);            //0�飬ȫ�����ȼ�          
	 NVIC_InitStructure.NVIC_IRQChannel= TIM4_IRQn;             //ѡ���ж�ͨ������P166ҳ     
	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;    //��ռ���ȼ�0  
	 NVIC_InitStructure.NVIC_IRQChannelSubPriority=2;           //��Ӧ���ȼ�0    
	 NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;              //������ͨ�����ж�   
	 NVIC_Init(&NVIC_InitStructure);                            //�ṹ���ʼ��       
	 TIM_Cmd(TIM4,ENABLE);								//ʹ�ܶ�ʱ��4
 }
void TIM3_STOP(void)
{
	TIM3SetFlag = 0;
	TIM_Cmd(TIM3, DISABLE);  //ȡ����ʱ					 
}
u8 LED_Statue = 0;
//��ʱ��3�жϷ������
void TIM3_IRQHandler(void)   //TIM3�ж�
{
//	MsgStruct Msgtemp;
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //���TIMx�����жϱ�־ 
		IO_OutSet(5,LED_Statue);
		LED_Statue ^= 1;
	}
}

void TIM4_IRQHandler(void) //��ʱ��������
{
	TIM_ClearITPendingBit(TIM4, TIM_FLAG_Update);
}










