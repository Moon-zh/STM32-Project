#include "led.h"
#include "delay.h"
#include "sys.h"	 
#include "Rs485.h"
#include "check.h"
#include "hmi_driver.h"
#include "hmi_user_uart.h"
#include "cmd_queue.h"
#include "cmd_process.h"
#include "includes.h"
#include "check.h"
#include "stmflash.h"
#include "Emw3060.h"

extern char ProductKey1[20];
extern char DeviceName1[50];
extern char DeviceSecret1[50];

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
#define HDMI_TASK_PRIO       			2 
//设置任务堆栈大小
#define HDMI_STK_SIZE  		    		512
//任务堆栈	
OS_STK HDMI_TASK_STK[HDMI_STK_SIZE];
//任务函数
void HDMI_task(void *pdata);

//设置任务优先级
#define IO_TASK_PRIO       				3 
//设置任务堆栈大小
#define IO_STK_SIZE  		    		1024
//任务堆栈	
OS_STK IO_TASK_STK[IO_STK_SIZE];
//任务函数
void IO_task(void *pdata);

//设置任务优先级
#define SaveThree_TASK_PRIO       		5 
//设置任务堆栈大小
#define SaveThree_STK_SIZE  		    1024
//任务堆栈	
__align(8)OS_STK SaveThree_TASK_STK[SaveThree_STK_SIZE];
//任务函数
void SaveThree_task(void *pdata);

//设置任务优先级
#define UpyunWF_TASK_PRIO       		7 
//设置任务堆栈大小
#define UpyunWF_STK_SIZE  		    	1024
//任务堆栈	
__align(8)OS_STK UpyunWF_TASK_STK[UpyunWF_STK_SIZE];
//任务函数
void UpyunWF_task(void *pdata);

#ifndef Env
#define Env
typedef struct
{
	u8	IO1;
	u8	IO2;
	u8	IO3;
	u8	IO4;
}IOSET;
#endif
IOSET IOSTATE;

#ifndef TIM
#define TIM
typedef struct
{
	u32	year;
	u8	month;
	u8	day;
	u8	week;
	u8	hour;
	u8	minute;
	u8	second;
}RTC_TIME;
#endif

RTC_TIME STIME;

u8	LED_BZ=0;
u16	Count_down=0;
u16 Remaining_time=0;
unsigned char printf_num=1;	//printf 串口指向标志 1指向uart1 2指向uart2
unsigned char Emw_B=0;		//wifi模组初始化完成标志
unsigned char emw_set=1;	//wifi报警与线程互斥标志
unsigned int  up_time=15;	//数据上传频率

#include "my_HDMI.h"

extern 	uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];
extern 	uint8_t g_RxBuf4[UART4_RX_BUF_SIZE];
extern 	uint8_t g_RxBuf5[UART5_RX_BUF_SIZE];
