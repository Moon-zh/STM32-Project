#include "led.h"
#include "delay.h"
#include "sys.h"	 
#include "rs485.h"
#include "check.h"
#include "SETIO.h"

#define head 0xFE		//I0���ַ(ͷ֡)

u8 check_out[]	={head ,0x01 ,0x00 ,0x00 ,0x00 ,0x08 ,0x29 ,0xC3 ,0x00 ,0x00 ,0x00};	//��ѯ�̵���ָ��
u8 check_in[]	={head ,0x02 ,0x00 ,0x00 ,0x00 ,0x08 ,0x6D ,0xC3 ,0x00 ,0x00 ,0x00};	//��ѯ��������ָ��
u8 control[]	={head ,0x05 ,0x00 ,0x00 ,0x00 ,0x00 ,0xC5 ,0xD9 ,0x00 ,0x00 ,0x00};	//�����ֽ�ѡ��̵������� 0Ϊ�� 0xffΪ��
u8 control_all[]={head ,0x0F ,0x00 ,0x00 ,0x00 ,0x08 ,0x01 ,0xFF ,0xF1 ,0xD1 ,0x00};	//����ȫ���̵�����7���ֽ�ѡ��ȫ���̵�������

u8 check_out_back[]	={head ,0x01 ,0x01 ,0x00 ,0x00 ,0x00};				//���ؼ̵���״̬
u8 check_in_back[]	={head ,0x02 ,0x01 ,0x00 ,0x00 ,0x00};				//���ع���״̬
u8 check1_out_back[]={head ,0x05 ,0x00 ,0x00 ,0xFF ,0x00 ,0x98 ,0x35};	//���ص����̵���״̬

u8 setlinkage[]={head ,0x10 ,0x03 ,0xEB ,0x00 ,0x01 ,0x02 ,0x00 ,0x01 ,0x06 ,0x7F};		//����Ϊ����ģʽָ��
u8 setnormal[] ={head ,0x10 ,0x03 ,0xEB ,0x00 ,0x01 ,0x02 ,0x00 ,0x00 ,0xC7 ,0xBF};		//����Ϊ����ģʽָ��

u8 *tab[]={check_out,check_in,control,setlinkage,setnormal};			//������

u8 Model=0;				//0Ϊ����ģʽ 1Ϊ����ģʽ
u8 zt_ks=0,zt_go=0;		//�̵��������״̬�洢

void	send(u8 *a,u8 zt)	//485���ͺ��� ����CRC16У��	���ڷ���6�ֽڵ�ָ��
{
	u16 b;
	a[3]=zt;
	b=mc_check_crc16(a,4);
	a[4]=b>>8;
	a[5]=b&0xff;
	RS485_Send_Data(a,6);
}

void	send1(u8 *a,u8 addr,u8 data)	//���ڷ���8�ֽڵ�ָ��
{
	u16 b;
	a[3]=addr;
	a[4]=data;
	b=mc_check_crc16(a,6);
	a[6]=b>>8;
	a[7]=b&0xff;
	RS485_Send_Data(a,8);
}

u8		read(u8 *a,u8 *len)				//485��ȡ����
{
	if(RS485_RX_CNT>=8)
	{
		RS485_Receive_Data(a,len);
		return 1;
	}
	return 0;
}

void	change_ks(u8 addr,u8 data)		//�����̵���״̬�ı�
{
	u8 a;
	a=zt_ks;
	if(Model)return;		//����ģʽֱ������
	switch(addr)
	{
		case 7:
				if(data)GPIO_ResetBits(GPIOC,GPIO_Pin_9),zt_ks|=0x80;
				else GPIO_SetBits(GPIOC,GPIO_Pin_9),zt_ks&=~0x80;
				break;
		case 6:
				if(data)GPIO_ResetBits(GPIOC,GPIO_Pin_8),zt_ks|=0x40;
				else GPIO_SetBits(GPIOC,GPIO_Pin_8),zt_ks&=~0x40;
				break;
		case 5:
				if(data)GPIO_ResetBits(GPIOC,GPIO_Pin_7),zt_ks|=0x20;
				else GPIO_SetBits(GPIOC,GPIO_Pin_7),zt_ks&=~0x20;
				break;
		case 4:
				if(data)GPIO_ResetBits(GPIOC,GPIO_Pin_6),zt_ks|=0x10;
				else GPIO_SetBits(GPIOC,GPIO_Pin_6),zt_ks&=~0x10;
				break;
		case 3:
				if(data)GPIO_ResetBits(GPIOB,GPIO_Pin_15),zt_ks|=0x08;
				else GPIO_SetBits(GPIOB,GPIO_Pin_15),zt_ks&=~0x08;
				break;
		case 2:
				if(data)GPIO_ResetBits(GPIOB,GPIO_Pin_14),zt_ks|=0x04;
				else GPIO_SetBits(GPIOB,GPIO_Pin_14),zt_ks&=~0x04;
				break;
		case 1:
				if(data)GPIO_ResetBits(GPIOB,GPIO_Pin_13),zt_ks|=0x02;
				else GPIO_SetBits(GPIOB,GPIO_Pin_13),zt_ks&=~0x02;
				break;
		case 0:
				if(data)GPIO_ResetBits(GPIOB,GPIO_Pin_12),zt_ks|=0x01;
				else GPIO_SetBits(GPIOB,GPIO_Pin_12),zt_ks&=~0x01;
				break;
	}
	if(a!=zt_ks)		//�ޱ仯����������
	send1(check1_out_back,addr,data);
	delay_ms(100);
}

void	change_all_ks(u8 a)				//ȫ���̵���״̬�ı�
{
	if(a&0x80)GPIO_ResetBits(GPIOC,GPIO_Pin_9);
	else GPIO_SetBits(GPIOC,GPIO_Pin_9);
	
	if(a&0x40)GPIO_ResetBits(GPIOC,GPIO_Pin_8);
	else GPIO_SetBits(GPIOC,GPIO_Pin_8);
	
	if(a&0x20)GPIO_ResetBits(GPIOC,GPIO_Pin_7);
	else GPIO_SetBits(GPIOC,GPIO_Pin_7);
	
	if(a&0x10)GPIO_ResetBits(GPIOC,GPIO_Pin_6);
	else GPIO_SetBits(GPIOC,GPIO_Pin_6);
	
	if(a&0x08)GPIO_ResetBits(GPIOB,GPIO_Pin_15);
	else GPIO_SetBits(GPIOB,GPIO_Pin_15);
	
	if(a&0x04)GPIO_ResetBits(GPIOB,GPIO_Pin_14);
	else GPIO_SetBits(GPIOB,GPIO_Pin_14);
	
	if(a&0x02)GPIO_ResetBits(GPIOB,GPIO_Pin_13);
	else GPIO_SetBits(GPIOB,GPIO_Pin_13);
	
	if(a&0x01)GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	else GPIO_SetBits(GPIOB,GPIO_Pin_12);
	
	if(zt_ks!=a)zt_ks=a;			//�ޱ仯����������
	else return;

	send(check_out_back,zt_ks);
}

void	read_go()		//��ȡ����״̬
{
	u8 a;
	a=zt_go;
	zt_go=0;
	zt_go|=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4)<<7;
	zt_go|=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5)<<6;
	zt_go|=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6)<<5;
	zt_go|=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_7)<<4;
	zt_go|=GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_4)<<3;
	zt_go|=GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)<<2;
	zt_go|=GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)<<1;
	zt_go|=GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1);
	zt_go=~zt_go;
	
	if(Model)					//����ģʽ����
	{
		change_all_ks(zt_go);
		delay_ms(50);
		if(zt_go!=a)			//�����ޱ仯ʱ����������
		send(check_in_back,zt_go);
	}
}

void	pdcmd(u8 *a)	//�ж�ָ��
{
	u8 k,g,sum;
	u16 checkcrc=0;
	for(g=0;g<5;g++)
	{
		sum=0;
		for(k=0;k<11;k++)	//�ж��Ƿ�����ͬ������
		{
			if(a[k]==(tab[g])[k])sum++;		
		}
		if(sum>=8)			//�ж��Ƿ���8�ֽڵ�ָ��
		{
			if(tab[g]==check_out){send(check_out_back,zt_ks);return ;}						//�̵���״̬��ѯ
			else if(tab[g]==check_in){send(check_in_back,zt_go);return ;}					//����״̬��ѯ
			else if((sum==11)&(tab[g]==setlinkage)){Model=1;RS485_Send_Data(a,11);return ;}	//����Ϊ����״̬
			else if((sum==11)&(tab[g]==setnormal)){Model=0;RS485_Send_Data(a,11);return ;}	//����Ϊ����״̬
		}
		if(sum>=4)				//�ж��Ƿ��Ǽ̵�������ָ��
		{
			checkcrc=mc_check_crc16(a,6);
			if(((checkcrc>>8)==a[6])&&((checkcrc&0xff)==a[7]))
			{
				if(a[1]==0x05)	//�ж��Ƿ�ڶ��ֽ�Ϊ�����̵�����������
				{
					change_ks(a[3],a[4]);
					return ;
				}
			}
			checkcrc=mc_check_crc16(a,8);
			if(((checkcrc>>8)==a[8])&&((checkcrc&0xff)==a[9]))
			{
				if(a[1]==0x0f)	//�ж��Ƿ�ڶ��ֽ�Ϊȫ���̵�����������
				{
					change_all_ks(a[7]);
					return ;
				}
			}
		}
	}
}

void	init()			//ϵͳ��ʼ��
{
	SystemInit();
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	LED_Init();		  		//��ʼ����LED���ӵ�Ӳ���ӿ�	
	GO_Init();				//�����ʼ��
	KS_Init();				//�̵�����ʼ��
	RS485_Init(9600);		//��ʼ��RS485
}

int 	main(void)
{	 
	u8 t=0;
	u8 *rs485buf;
	u8 len;
	init();
	while(1)
	{
		if(read(rs485buf,&len))pdcmd(rs485buf);	//��ȡ��485ָ����ж�ָ��
		read_go();								//��ȡ����״̬
		
		t++; 
		delay_ms(10);
		if(t==20)
		{			
			LED0=!LED0;//��ʾϵͳ��������	
			t=0;
		}		   
	} 
}


