#include "IO_BSP.h"

//����ܽų�ʼ��
//X1:PB12 X2:PB13 X3:PB15 X4:PC6  
void DI_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_15;   //GPIOB.12  GPIOB.13  GPIOB.15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB.12  GPIOB.13  GPIOB.15
 	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;   //GPIOC.6 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIOC.6
 	
}

//����ܽų�ʼ��
//Y1:PC7  Y2:PC8  Y3:PC9  Y4:PA8 LED:PC1
void DO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;				
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO������ٶ�Ϊ50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//��ʼ��GPIOC.7
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO������ٶ�Ϊ50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//��ʼ��GPIOC.8

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO������ٶ�Ϊ50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//��ʼ��GPIOC.9

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;			
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO������ٶ�Ϊ50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//��ʼ��GPIOA.8

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;				
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO������ٶ�Ϊ50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//��ʼ��GPIOC.1
}

//IO��ʼ������
//X1:PB12 X2:PB13 X3:PB15 X4:PC6  
//Y1:PC7  Y2:PC8  Y3:PC9  Y4:PA8 LED:PC1
void BSP_InitIO(void)
{
	
 	//��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);	 //����GPIOA GPIOB GPIOC��ʱ��
	DI_Init();//������IO���г�ʼ��
	DO_Init();//�����IO���г�ʼ��
	
}

//����ܽ����ã����0�����1
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
