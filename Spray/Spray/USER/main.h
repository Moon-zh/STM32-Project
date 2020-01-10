#ifndef __MAIN_H
#define __MAIN_H
#include "led.h"
#include "delay.h"
#include "sys.h"	 
#include "Rs485.h"
#include "check.h"
#include "includes.h"
#include "check.h"
#include "stmflash.h"
#include "w25qxx.h"
#include "FlashDivide.h"
#include "NeowayN21.h"
#include "cJSON.h"

extern char ProductKey0[20];
extern char DeviceName0[50];
extern char DeviceSecret0[50];
extern char CCID[25];

#define FLASH_SAVE_ADDR  0X0802DE00
#define FLASH_THREE_ADDR 0X0802FE00

//START ����
//�����������ȼ�
#define START_TASK_PRIO      			10 //��ʼ��������ȼ�����Ϊ���
//���������ջ��С
#define START_STK_SIZE  				64
//�����ջ	
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);

//�����������ȼ�
#define LED_TASK_PRIO       			1 
//���������ջ��С
#define LED_STK_SIZE  		    		64
//�����ջ	
OS_STK LED_TASK_STK[LED_STK_SIZE];
//������
void LED_task(void *pdata);

//�����������ȼ�
#define SaveThree_TASK_PRIO       		5 
//���������ջ��С
#define SaveThree_STK_SIZE  		    256
//�����ջ	
__align(8)OS_STK SaveThree_TASK_STK[SaveThree_STK_SIZE];
//������
void SaveThree_task(void *pdata);

//�����������ȼ�
#define Upyun_TASK_PRIO       			3 
//���������ջ��С
#define Upyun_STK_SIZE  		    	1024
//�����ջ	
__align(8)OS_STK Upyun_TASK_STK[Upyun_STK_SIZE];
//������
void Upyun_task(void *pdata);

//�����������ȼ�
#define Main_TASK_PRIO       			2 
//���������ջ��С
#define Main_STK_SIZE  		    		512
//�����ջ	
__align(8)OS_STK Main_TASK_STK[Main_STK_SIZE];
//������
void Main_task(void *pdata);

#define	Back	PBin(12)

u16 Time=0;
u8	Run=0;

u8	Nsendok=1,nbup=0;
u8	N21_B=1,n21_set=0;
u8	LED_BZ=0,SysTime=0;		//LED��ʾ�л���־��Уʱ��־
unsigned char printf_num=1;	//printf ����ָ���־ 1ָ��uart1 2ָ��uart2
unsigned char Emw_B=0;		//wifiģ���ʼ����ɱ�־
unsigned char emw_set=1;	//wifi�������̻߳����־
unsigned int  up_time=15;	//�����ϴ�Ƶ��

extern 	uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];
extern 	uint8_t g_RxBuf4[UART4_RX_BUF_SIZE];
extern 	uint8_t g_RxBuf5[UART5_RX_BUF_SIZE];

extern 	char ssid[20];
extern 	char password[20];
extern 	char ip[20];
extern 	char network[20];
extern 	char gateway[20];
extern 	char dns[20];
extern 	u8 DHCP;

u8	upbz=0;
u8	rema=0;					//���е���ʱ��־
u8	ttm=0,upenv=0,up_state=0;					//���Ӽ�ʱ�����������ϴ�ʱ���ʱ���豸״̬�ϴ���־
OS_TMR   * tmr1;           						//�����ʱ��1
void tmr1_callback(OS_TMR *ptmr,void *p_arg)	//�����ʱ��1�ص�����
{
    if(++ttm==6)
	{
		ttm=0;									//��������ϵͳ��ʱ
		Time--;
		if(Run)nbup=1;
	}
	if(++upbz>=180)
	{
		upbz=0;
		nbup=1;
	}
}
   
void	readflashthree()				//��ȡ��Ԫ��
{
	char buf[150];
	char *msg=buf;
	u8 i;
	STMFLASH_Read(FLASH_THREE_ADDR,(u16*)buf,70);
	if(buf[1]=='K')
	{
		memset(ProductKey0,0,20);
		msg=buf+4;for(i=0;*msg;msg++)		ProductKey0[i++]=*msg;
	}
	if(buf[25]=='N')
	{
		memset(DeviceName0,0,50);
		msg=buf+24+4;for(i=0;*msg;msg++)	DeviceName0[i++]=*msg;
	}
	if(buf[79]=='S')
	{
		memset(DeviceSecret0,0,50);
		msg=buf+78+4;for(i=0;*msg;msg++)	DeviceSecret0[i++]=*msg;
	}
}

void	sendflashthree()				//д����Ԫ��
{
	LED_BZ=1;
	STMFLASH_Write(FLASH_THREE_ADDR,(u16*)"PK:0",4);
	STMFLASH_Write(FLASH_THREE_ADDR+4,(u16*)ProductKey0,sizeof(ProductKey0));
	STMFLASH_Write(FLASH_THREE_ADDR+24,(u16*)"DN:0",4);
	STMFLASH_Write(FLASH_THREE_ADDR+28,(u16*)DeviceName0,sizeof(DeviceName0));
	STMFLASH_Write(FLASH_THREE_ADDR+78,(u16*)"DS:0",4);
	STMFLASH_Write(FLASH_THREE_ADDR+82,(u16*)DeviceSecret0,sizeof(DeviceSecret0));
	readflashthree();
	LED_BZ=0;
}

#include "IO.h"
#endif
