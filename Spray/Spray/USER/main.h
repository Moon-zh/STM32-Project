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

//START 任务
//设置任务优先级
#define START_TASK_PRIO      			10 //开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  				64
//任务堆栈	
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);

//设置任务优先级
#define LED_TASK_PRIO       			1 
//设置任务堆栈大小
#define LED_STK_SIZE  		    		64
//任务堆栈	
OS_STK LED_TASK_STK[LED_STK_SIZE];
//任务函数
void LED_task(void *pdata);

//设置任务优先级
#define SaveThree_TASK_PRIO       		5 
//设置任务堆栈大小
#define SaveThree_STK_SIZE  		    256
//任务堆栈	
__align(8)OS_STK SaveThree_TASK_STK[SaveThree_STK_SIZE];
//任务函数
void SaveThree_task(void *pdata);

//设置任务优先级
#define Upyun_TASK_PRIO       			3 
//设置任务堆栈大小
#define Upyun_STK_SIZE  		    	1024
//任务堆栈	
__align(8)OS_STK Upyun_TASK_STK[Upyun_STK_SIZE];
//任务函数
void Upyun_task(void *pdata);

//设置任务优先级
#define Main_TASK_PRIO       			2 
//设置任务堆栈大小
#define Main_STK_SIZE  		    		512
//任务堆栈	
__align(8)OS_STK Main_TASK_STK[Main_STK_SIZE];
//任务函数
void Main_task(void *pdata);

#define	Back	PBin(12)

u16 Time=0;
u8	Run=0;

u8	Nsendok=1,nbup=0;
u8	N21_B=1,n21_set=0;
u8	LED_BZ=0,SysTime=0;		//LED显示切换标志，校时标志
unsigned char printf_num=1;	//printf 串口指向标志 1指向uart1 2指向uart2
unsigned char Emw_B=0;		//wifi模组初始化完成标志
unsigned char emw_set=1;	//wifi报警与线程互斥标志
unsigned int  up_time=15;	//数据上传频率

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
u8	rema=0;					//运行倒计时标志
u8	ttm=0,upenv=0,up_state=0;					//分钟计时器，传感器上传时间计时，设备状态上传标志
OS_TMR   * tmr1;           						//软件定时器1
void tmr1_callback(OS_TMR *ptmr,void *p_arg)	//软件定时器1回调函数
{
    if(++ttm==6)
	{
		ttm=0;									//传感器，系统计时
		Time--;
		if(Run)nbup=1;
	}
	if(++upbz>=180)
	{
		upbz=0;
		nbup=1;
	}
}
   
void	readflashthree()				//读取三元组
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

void	sendflashthree()				//写入三元组
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
