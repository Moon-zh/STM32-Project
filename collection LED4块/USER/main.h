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
#include "G405tf.h"

#include "stmflash.h"
#include "Emw3060.h"
#include "NeowayN21.h"

extern char ssid[20];			//wifi ID
extern char password[20];		//wifi 密码

extern char ProductKey1[20];	//从设备三元组
extern char DeviceName1[50];
extern char DeviceSecret1[50];

extern char *ProductKeyw;
extern char *DeviceNamew;
extern char *DeviceSecretw;

//鉴权信息
extern char ProductKey0[20];	//主设备三元组
extern char DeviceName0[50];
extern char DeviceSecret0[50];

extern char *ProductKey;
extern char *DeviceName;
extern char *DeviceSecret;

extern char CCID[25];


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
#define HDMI_TASK_PRIO       			2 
//设置任务堆栈大小
#define HDMI_STK_SIZE  		    		512
//任务堆栈	
OS_STK HDMI_TASK_STK[HDMI_STK_SIZE];
//任务函数
void HDMI_task(void *pdata);

//设置任务优先级
#define Upyun_TASK_PRIO       			3 
//设置任务堆栈大小
#define Upyun_STK_SIZE  		    	1024
//任务堆栈	
__align(8)OS_STK Upyun_TASK_STK[Upyun_STK_SIZE];
//任务函数
void Upyun_task(void *pdata);

//设置任务优先级
#define Alarm_TASK_PRIO       			4 
//设置任务堆栈大小
#define Alarm_STK_SIZE  		    	1024
//任务堆栈	
__align(8)OS_STK Alarm_TASK_STK[Alarm_STK_SIZE];
//任务函数
void Alarm_task(void *pdata);

//设置任务优先级
#define SaveThree_TASK_PRIO       		5 
//设置任务堆栈大小
#define SaveThree_STK_SIZE  		    1024
//任务堆栈	
__align(8)OS_STK SaveThree_TASK_STK[SaveThree_STK_SIZE];
//任务函数
void SaveThree_task(void *pdata);

//设置任务优先级
#define Collection_TASK_PRIO       		6 
//设置任务堆栈大小
#define Collection_STK_SIZE  		    256
//任务堆栈	
OS_STK Collection_TASK_STK[Collection_STK_SIZE];
//任务函数
void Collection_task(void *pdata);

//设置任务优先级
#define UpyunWF_TASK_PRIO       		7 
//设置任务堆栈大小
#define UpyunWF_STK_SIZE  		    	1024
//任务堆栈	
__align(8)OS_STK UpyunWF_TASK_STK[UpyunWF_STK_SIZE];
//任务函数
void UpyunWF_task(void *pdata);

//设置任务优先级
#define LED_DZ_TASK_PRIO       			8 
//设置任务堆栈大小
#define LED_DZ_STK_SIZE  		    	1024
//任务堆栈	
OS_STK LED_DZ_TASK_STK[LED_DZ_STK_SIZE];
//任务函数
void LED_DZ_task(void *pdata);


#define 	CMD_AIRTEMP		0x01	//传感器寄存器地址
#define		CMD_AIRHUMI 	0x00
#define		CMD_SOILTEMP	0x03
#define		CMD_SOILHUMI	0x02
#define		CMD_CO2			0x05
#define 	CMD_LIGTH		0x07
#define		CMD_EC			0x15
#define		EnvNum			5		//传感器最大上限

#define 	Alarm_airtemp 	800		//气温报警值
#define 	Alarm_airhumi 	900		//空气湿度报警值
#define 	Alarm_light 	10000	//光强度报警值
#define 	Alarm_CO2 		800		//CO2报警值
	
#define 	Alarm_soiltemp 	700		//土壤温度报警值
#define 	Alarm_soilhumi 	900		//土壤湿度报警值
#define 	Alarm_EC 		100		//电导率转速报警值

#ifndef Env
#define Env
typedef struct			//传感器数组结构体
{
	u16		airtemp;	//转为十进制后需要/10
	u16		airhumi;	//转为十进制后需要/10
	u16		soiltemp;	//转为十进制后需要/10
	u16		soilhumi;	//转为十进制后需要/10
	u16		CO2;
	u32		light;
	u16		EC;
}Environmental;
#endif

u8	N21BZ=0;			//为了避免N21发送冲突，判断是否N21在发送数据
u8	READBZ=1;			//判断是否是一个上传周期结束，用于报警是否上传
u16	time=0;				//主设备报警上报时间寄存器
u8	LED_BZ=0;			//LED显示状态切换标志
unsigned char printf_num=1;	//printf 串口指向标志 1指向uart1 2指向uart2
unsigned char N21_B=0;		//NB模组初始化完成标志
unsigned char Emw_B=0;		//wifi模组初始化完成标志
unsigned char emw_set=1;	//wifi报警与线程互斥标志
unsigned char n21_set=1;	//N21报警与线程互斥标志
unsigned int  up_time=15;	//数据上传频率
unsigned int  up_wartime=20;	//报警上传频率
unsigned char Three=0;
unsigned char Nsendok=0;
#include "my_HDMI.h"
extern uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];
extern uint8_t g_RxBuf5[UART5_RX_BUF_SIZE];
void	readflash(void);
void	sendflash(void);
void	UpSetAlarm_wifi(Waring data,u8 group);
void	UpSetAlarm(Waring data,u8 group);
