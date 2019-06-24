#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "rtc.h" 		 	 
#include "string.h"
#include "Includes.h"
#include "Rs485.h"
#include "led.h"
#include "ZoneCtrl.h"
#include "timer.h"
#include "IO_BSP.h"
#include "W25qxx.h"
#include "GPS.h"
#include "Alarm.h"
#include "LevelGet.h"
#include "Emw3060.h"
#include "stmflash.h"
 
 
#define FLASH_THREE_ADDR  0X0802FE00

void	readflashthree(void);						//从flash中读取三元组数据

/////////////////////////UCOSII任务设置///////////////////////////////////
//START 任务
//设置任务优先级
#define START_TASK_PRIO      			13//开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  				64
//任务堆栈	
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	
 			   
//LED任务
//设置任务优先级
#define LED_TASK_PRIO       			12 
//设置任务堆栈大小
#define LED_STK_SIZE  		    		64
//任务堆栈
OS_STK LED_TASK_STK[LED_STK_SIZE];
//任务函数
void led_task(void *pdata);


//加水任务
//设置任务优先级
#define WATERADD_TASK_PRIO       			4 
//设置任务堆栈大小
#define WATERADD_STK_SIZE  					256
//任务堆栈	
OS_STK WATERADD_TASK_STK[WATERADD_STK_SIZE];
//任务函数
void WaterAdd_task(void *pdata);

//搅拌任务
//设置任务优先级
#define STIR_TASK_PRIO       			5 
//设置任务堆栈大小
#define STIR_STK_SIZE  					256
//任务堆栈	
OS_STK STIR_TASK_STK[STIR_STK_SIZE];
//任务函数
void Stir_task(void *pdata);

//信号量集任务
//设置任务优先级
#define FLAGS_TASK_PRIO       			3 
//设置任务堆栈大小
#define FLAGS_STK_SIZE  		 		56
//任务堆栈	
OS_STK FLAGS_TASK_STK[FLAGS_STK_SIZE];
//任务函数
void flags_task(void *pdata);

//GPRS任务
//设置任务优先级
#define GPRS_TASK_PRIO       			6 
//设置任务堆栈大小
#define GPRS_STK_SIZE  		 			1024
//任务堆栈	
OS_STK GPRS_TASK_STK[GPRS_STK_SIZE];
//任务函数
void network_task(void * pdata);

//电磁阀任务
//设置任务优先级
#define ZONE_TASK_PRIO       		 	7
//设置任务堆栈大小
#define ZONE_STK_SIZE  				512
//任务堆栈	
OS_STK ZONE_TASK_STK[ZONE_STK_SIZE];
//任务函数
void ZoneCtrl_task(void *pdata);

//界面任务
//设置任务优先级
#define HMI_TASK_PRIO       			8 
//设置任务堆栈大小
#define HMI_STK_SIZE  		 			512
//任务堆栈	
OS_STK HMI_TASK_STK[HMI_STK_SIZE];
//任务函数
void Task_HMIMonitor(void * pdata);

//报警任务
//设置任务优先级
#define ALARM_TASK_PRIO       			9 
//设置任务堆栈大小
#define ALARM_STK_SIZE  		 			128
//任务堆栈	
OS_STK ALARM_TASK_STK[ALARM_STK_SIZE];
//任务函数
void Alarm_task(void * pdata);
//液位获取任务
//液位采集任务
//设置任务优先级
#define LEVEL_TASK_PRIO       			10 
//设置任务堆栈大小
#define LEVEL_STK_SIZE  		 			128
//任务堆栈	
OS_STK LEVEL_TASK_STK[LEVEL_STK_SIZE];
//任务函数
void LevelGet_task(void * pdata);
//液位获取任务

//按键扫描任务
//设置任务优先级
#define KEY_TASK_PRIO       			2 
//设置任务堆栈大小
#define KEY_STK_SIZE  					56
//任务堆栈	
OS_STK KEY_TASK_STK[KEY_STK_SIZE];
//任务函数
void key_task(void *pdata);
//////////////////////////////////////////////////////////////////////////////
    
OS_EVENT * msg_key;			//按键邮箱事件块	  
OS_EVENT * q_msg;			//消息队列
OS_TMR   * tmr1;			//软件定时器1
OS_TMR   * tmr2;			//软件定时器2
OS_TMR   * tmr3;			//软件定时器3
OS_FLAG_GRP * flags_key;	//按键信号量集
void * MsgGrp[256];			//消息队列存储地址,最大支持256个消息


//软件定时器3的回调函数				  	   
void tmr3_callback(OS_TMR *ptmr,void *p_arg) 
{	
	//u8* p;	 
} 


 int main(void)
 {	 		    
	delay_init();	    	 //延时函数初始化	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	//uart_init(115200);	 	//串口初始化为115200
  	BSP_InitIO();				//初始化IO
  	GPS_Init();					//初始化GPS
//	my_mem_init(SRAMIN);		//初始化内部内存池
   	W25QXX_Init();
   	bsp_InitUart();				//初始化RS485
	OSInit();  	 				//初始化UCOSII
  	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();	  
}
///////////////////////////////////////////////////////////////////////////////////////////////////

//开始任务
void start_task(void *pdata)
{
    OS_CPU_SR cpu_sr=0;
//	u8 err;	    	    
	pdata = pdata; 	
	RTC_Init();
	OSStatInit();						//初始化统计任务.这里会延时1秒钟左右	
	readflashthree();
 	OS_ENTER_CRITICAL();				//进入临界区(无法被中断打断)    
	//delay_ms(1000);
 /*	OSTaskCreate(led_task,(void *)0,
		(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],
		LED_TASK_PRIO);		*/
	
 	OSTaskCreate(ZoneCtrl_task,(void *)0,
		(OS_STK*)&ZONE_TASK_STK[ZONE_STK_SIZE-1],
		ZONE_TASK_PRIO);		
	
 	OSTaskCreate(network_task,(void *)0,
		(OS_STK*)&GPRS_TASK_STK[GPRS_STK_SIZE-1],
		GPRS_TASK_PRIO);	 
	
 	OSTaskCreate(WaterAdd_task,(void *)0,
		(OS_STK*)&WATERADD_TASK_STK[WATERADD_STK_SIZE-1],
		WATERADD_TASK_PRIO);	 	
	
 	OSTaskCreate(Task_HMIMonitor,(void *)0,
		(OS_STK*)&HMI_TASK_STK[HMI_STK_SIZE-1],
		HMI_TASK_PRIO);	 	
	
 	OSTaskCreate(Alarm_task,(void *)0,
		(OS_STK*)&ALARM_TASK_STK[ALARM_STK_SIZE-1],
		ALARM_TASK_PRIO);	 	
	
 	OSTaskCreate(LevelGet_task,(void *)0,
		(OS_STK*)&LEVEL_TASK_STK[LEVEL_STK_SIZE-1],
		LEVEL_TASK_PRIO);	 		
	
 	OSTaskCreate(Stir_task,(void *)0,
		(OS_STK*)&STIR_TASK_STK[STIR_STK_SIZE-1],
		STIR_TASK_PRIO);	 				   
	
 	/*OSTaskCreate(key_task,(void *)0,
		(OS_STK*)&KEY_TASK_STK[KEY_STK_SIZE-1],
		KEY_TASK_PRIO);	 */				
	
	TIM3_Int_Init(9999,7199);//10Khz的计数频率，计数到10000为1s  
 	OSTaskSuspend(START_TASK_PRIO);		//挂起起始任务.
	OS_EXIT_CRITICAL();					//退出临界区(可以被中断打断)
}
//LED任务
void led_task(void *pdata)
{
	u8 t;
	while(1)
	{
		t++;
		delay_ms(10);
//		if(t==8)LED0=1;	//LED0灭
		if(t==100)		//LED0亮
		{
			t=0;
		//	LED0=0;
		}
	}									 
}
//触摸屏任务
void touch_task(void *pdata)
{	  	
//	u32 cpu_sr;
// 	u16 lastpos[2];		//最后一次的数据 
	while(1)
	{
		delay_ms(5);	 
	}
}     
//队列消息显示任务
void qmsgshow_task(void *pdata)
{
}
//主任务
void main_task(void *pdata)
{							 
//	u32 key=0;	
//	u8 err;	
//	u8 alfredTest[3] = {0X41,0X54,0X0D};
	
	//comSendBuf(COM3,alfredTest,sizeof(alfredTest) );
// 	u8 tmr2sta=1;	//软件定时器2开关状态   
// 	u8 tmr3sta=0;	//软件定时器3开关状态
//	u8 flagsclrt=0;	//信号量集显示清零倒计时   
	//tmr3=OSTmrCreate(10,10,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr3_callback,0,"tmr3",&err);		//100ms执行一次
 	while(1)
	{
		delay_ms(300);
		//comSendBuf(COM3,alfredTest,sizeof(alfredTest) );

			
	}
}		   
//信号量集处理任务
void flags_task(void *pdata)
{	
//	u16 flags;	
//	u8 err;	    						 
	while(1)
	{
		delay_ms(50);
 	}
}
void	readflashthree(void)						//从flash中读取三元组数据
{
	char buf[150];
	char *msg=buf;
	u8 i;
 	STMFLASH_Read(FLASH_THREE_ADDR,(u16*)buf,75);
	if((buf[0]==0)|(buf[0]==0xff))return;
	if(buf[1]=='K')
	{
		memset(ProductKey0,0,20);
		msg=buf+4;for(i=0;*msg;msg++)		ProductKey[i++]=*msg;
	}
	if(buf[25]=='N')
	{
		memset(DeviceName0,0,20);
		msg=buf+25+3;for(i=0;*msg;msg++)	DeviceName[i++]=*msg;
	}
	if(buf[79]=='S')
	{
		memset(DeviceSecret0,0,40);
		msg=buf+79+3;for(i=0;*msg;msg++)	DeviceSecret[i++]=*msg;
	}
	
	//STMFLASH_Read(100+FLASH_THREE_ADDR,(u16*)buf,50);
	//if((buf[0]==0)||(buf[0]==0xff))return;
//	if(buf[1]=='K')
//	{
//		memset(ProductKey1,0,20);
//		msg=buf+4;for(i=0;*msg;msg++)		ProductKeyw[i++]=*msg;
//	}
//	if(buf[25]=='N')
//	{
//		memset(DeviceName1,0,20);
//		msg=buf+25+4;for(i=0;*msg;msg++)	DeviceNamew[i++]=*msg;
//	}
//	if(buf[49]=='S')
//	{
//		memset(DeviceSecret1,0,40);
//		msg=buf+49+4;for(i=0;*msg;msg++)	DeviceSecretw[i++]=*msg;
//	}
//	printf("%s,%s,%s",ProductKey0,DeviceName0,DeviceSecret0);
}

void	sendflashthree(u8 group)				//向内存写入设置的三元组数据
{
	OS_CPU_SR cpu_sr=0;
	OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)
	//LED_BZ=1;
	STMFLASH_Write(group*100+FLASH_THREE_ADDR,(u16*)"PK:0",4);
	STMFLASH_Write(group*100+FLASH_THREE_ADDR+4,(u16*)ProductKey0,sizeof(ProductKey0));
	STMFLASH_Write(group*100+FLASH_THREE_ADDR+24,(u16*)"DN:0",4);
	STMFLASH_Write(group*100+FLASH_THREE_ADDR+28,(u16*)DeviceName0,sizeof(DeviceName0));
	STMFLASH_Write(group*100+FLASH_THREE_ADDR+78,(u16*)"DS:0",4);
	STMFLASH_Write(group*100+FLASH_THREE_ADDR+82,(u16*)DeviceSecret0,sizeof(DeviceSecret0));
	readflashthree();
	//LED_BZ=0;
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
//	disconN21();
//	delay_ms(2000);
//	conN21();
//	printf("%s,%s,%s",ProductKey0,DeviceName0,DeviceSecret0);
}

void	readthree(void)								//从COM3 485串口读取是否设置了三元组数据
{
	u8 i;
	char hc[250];
	char *msg=hc;
	u8 len=0;
	u8 buf[250];
	memset(buf,0,sizeof buf);
	memset(hc,0,sizeof hc);
	i=0;
	len=COM3GetBuf(buf);
	if(len>10)
	{
		if(strstr((const char *)buf,"m1")[0]=='m')
		{
			msg=strstr((const char *)buf,"PK:");
			if(msg[1]=='K')
			{
				memset(ProductKey0,0,20);
				msg+=3;
				for(i=0;*msg!=',';msg++)	
				{
					ProductKey[i++]=*msg;	
					if(i>=20)return ;
				}
			}
			msg=strstr((const char *)buf,"DN:");
			if(msg[1]=='N')
			{
				memset(DeviceName0,0,50);
				msg+=3;
				for(i=0;*msg!=',';msg++)	
				{
					DeviceName[i++]=*msg;	
					if(i>=50)
						return ;
				}
			}
			msg=strstr((const char *)buf,"DS:");
			if(msg[1]=='S')
			{
				memset(DeviceSecret0,0,40);
				msg+=3;
				for(i=0;*msg;msg++)		
				{
					DeviceSecret[i++]=*msg;
					if(i>=40)return ;
				}
			}
			sendflashthree(0);
			comSendBuf(COM3,buf,strlen((char *)buf));
			comClearRxFifo(COM3);
			//memset(buf,0,sizeof buf);
			//memset(hc,0,sizeof buf);
		}
		/*else if(strstr((const char *)buf,"m2")[0]=='m')
		{
			msg=strstr((const char *)buf,"PK:");
			if(msg[1]=='K')
			{
				memset(ProductKey1,0,20);
				msg+=3;for(i=0;*msg!=',';msg++)	{ProductKeyw[i++]=*msg;	if(i>=20)return ;}
			}
			msg=strstr((const char *)buf,"DN:");
			if(msg[1]=='N')
			{
				memset(DeviceName1,0,50);
				msg+=3;for(i=0;*msg!=',';msg++)	{DeviceNamew[i++]=*msg;	if(i>=50)return ;}
			}
			msg=strstr((const char *)buf,"DS:");
			if(msg[1]=='S')
			{
				memset(DeviceSecret1,0,40);
				msg+=3;for(i=0;*msg;msg++)		{DeviceSecretw[i++]=*msg;if(i>=40)return ;}
			}
			comSendBuf(COM3,buf,strlen((char *)buf));
			comClearRxFifo(COM3);
			memset(buf,0,sizeof buf);
			memset(hc,0,sizeof buf);
			len=2;
		}*/
		//else return;
		
	}
}
//按键扫描任务
void key_task(void *pdata)
{	
	while(1)
	{
 		delay_ms(200);
		//readthree();					//判断是否扫码枪发送三元组数据
	}
}
