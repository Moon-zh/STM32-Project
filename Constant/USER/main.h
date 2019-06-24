#include "led.h"
#include "delay.h"
#include "sys.h"	 
#include "Rs485.h"
#include "check.h"
#include "includes.h"
#include "check.h"
#include "stmflash.h"
#include "Emw3060.h"

extern char ssid[20];			//wifi ID
extern char password[20];		//wifi 密码

extern char ProductKey1[20];	//从设备三元组
extern char DeviceName1[50];
extern char DeviceSecret1[50];

extern char *ProductKeyw;
extern char *DeviceNamew;
extern char *DeviceSecretw;


#define FLASH_SAVE_ADDR  0X0802DE00		//报警设置存储地址
#define FLASH_THREE_ADDR  0X0802FE00	//三元组存储地址

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

//设置任务优先级
#define Converter_TASK_PRIO       		2 
//设置任务堆栈大小
#define Converter_STK_SIZE  		    512
//任务堆栈	
__align(8)OS_STK Converter_TASK_STK[Converter_STK_SIZE];
//任务函数
void Converter_task(void *pdata);

//设置任务优先级
#define VM_TASK_PRIO       		2 
//设置任务堆栈大小
#define VM_STK_SIZE  		    512
//任务堆栈	
__align(8)OS_STK VM_TASK_STK[VM_STK_SIZE];
//任务函数
void VM_task(void *pdata);

u8	READBZ=1;			//判断是否是一个上传周期结束，用于报警是否上传
u8	LED_BZ=0;			//LED显示状态切换标志
unsigned char printf_num=1;	//printf 串口指向标志 1指向uart1 2指向uart2
unsigned char Emw_B=0;		//wifi模组初始化完成标志
unsigned char emw_set=1;	//wifi报警与线程互斥标志
unsigned int  up_t1=2;	//数据上传频率
unsigned int  up_t2=1;	//报警上传频率
