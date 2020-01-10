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
#include "Emw3060.h"
#include "stmflash.h"
#include "FlashDivide.h"

#include	"iwdg.h" //独立看门狗程序文件
#include	"UserHmi.h"
#include	"cgq.h"//传感器程序文件


 
void	readflashthree(void);					//从flash中读取三元组数据



/************************************************
 ALIENTEK战舰STM32开发板实验53
 UCOSII实验3-消息队列、信号量集和软件定时器  实验 
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/


/////////////////////////UCOSII任务设置///////////////////////////////////
//START 任务
//设置任务优先级
#define START_TASK_PRIO      			10 //开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  				64
//任务堆栈	
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	


//主任务
//设置任务优先级
#define MASTER_TASK_PRIO       			4 
//设置任务堆栈大小
#define MASTER_STK_SIZE  					512
//任务堆栈	
OS_STK MASTER_TASK_STK[MASTER_STK_SIZE];
//任务函数
void MasterCtrl_task(void *pdata);


//wifi任务
//设置任务优先级
#define GPRS_TASK_PRIO       			5 
//设置任务堆栈大小
#define GPRS_STK_SIZE  		 			1024
//任务堆栈	
__align(8)OS_STK GPRS_TASK_STK[GPRS_STK_SIZE];
//任务函数
void network_task(void * pdata);


//触摸屏界面任务
//设置任务优先级
#define HMI_TASK_PRIO       			8 
//设置任务堆栈大小
#define HMI_STK_SIZE  		 			1024
//任务堆栈	
OS_STK HMI_TASK_STK[HMI_STK_SIZE];
//任务函数
void Task_HMIMonitor(void * pdata);


//三元组任务
//设置任务优先级
#define KEY_TASK_PRIO       			3 
//设置任务堆栈大小
#define KEY_STK_SIZE  					1024
//任务堆栈	
OS_STK KEY_TASK_STK[KEY_STK_SIZE];
//任务函数
void key_task(void *pdata);


//LED任务
//设置任务优先级
#define LED_TASK_PRIO       			7 
//设置任务堆栈大小
#define LED_STK_SIZE  		    		64
//任务堆栈
OS_STK LED_TASK_STK[LED_STK_SIZE];
//任务函数
void led_task(void *pdata);

//土壤温湿度传感器任务
//设置任务优先级
#define CGQ_TR_TASK_PRIO       			11 
//设置任务堆栈大小
#define CGQ_TR_STK_SIZE  		    		64
//任务堆栈
OS_STK CGQ_TR_TASK_STK[CGQ_TR_STK_SIZE];
//任务函数
void cgq_tr_task(void *pdata);


//三元组的FLASH存储地址
#define FLASH_THREE_ADDR  0X0802FE00
//////////////////////////////////////////////////////////////////////////////
    




int main(void)
{	 	
	char tiaoshi[180];
	u8 ts_flash;

	
	delay_init();	    	 //延时函数初始化	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级

	//uart_init(115200);	 	//串口初始化为115200
  	BSP_InitIO();				//初始化IO(输入和输出)
  	GPS_Init();					//初始化GPS（GPS插口处的）
//	my_mem_init(SRAMIN);		//初始化内部内存池
   	W25QXX_Init();     //W25QXX初始化
   	bsp_InitUart();				//初始化串口 RS485
		RTC_Init();	  			//RTC初始化
	  HDMI_Init();    //触摸屏初始化
	  TIM3_Int_Init(9999,7199);//10Khz的计数频率，计数到10000为1s
//	  IWDG_Init(4,1250);//独立看门狗初始化 与分频数为64,重载值为1250,溢出时间为2s		 explorer.exe
    /*
			假如独立看门狗触发，需要保存现在的工作状态，使用flash保存。
	  */

	
	/*调试   预防flash中的存储区为空格*/
   FlashReadIP(&ts_flash);//读取是不是之前设置过flash
	if(ts_flash!=55)//之前没有在flash里面写过0x5656
	{
			strcpy(&tiaoshi[0],"mingji"); //wifi名称IoT
			strcpy(&tiaoshi[30],"mingji2015");//wifi密码ag20190520
			strcpy(&tiaoshi[60],"192.168.7.101");   //IP地址  "192.168.7.101";
			strcpy(&tiaoshi[90],"255.255.255.0"); //子网掩码  "255.255.255.0";
			strcpy(&tiaoshi[120],"192.168.7.1");  //网关地址  	"192.168.7.1";
			strcpy(&tiaoshi[150],"114.114.114.114"); //域名  "114.114.114.114";
			FlashWriteWiFi((u8 *)tiaoshi);//保存FLASH数据		
		
			ts_flash=55;
		  FlashWriteIP(&ts_flash);//把0x5656写入flash备份区域
	}
	
   //sec:秒 min：分 hour：时 day：日 week：星期 mon：月 year:年
	//SetRTC(0x05,0x14,0x17,0x30,0x2,0x7,0x19); //调试 设置触摸屏RTC时钟


		
   	OSInit();  	 				//初始化UCOSII
  	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	  OSStart();	  
}
///////////////////////////////////////////////////////////////////////////////////////////////////

//开始任务
void start_task(void *pdata)
{
   OS_CPU_SR cpu_sr=0;	    	    
	 pdata = pdata; 
	
	
	OSStatInit();						//初始化统计任务.这里会延时1秒钟左右
	
	readflashthree();//从flash中读取三元组数据
	
	OS_ENTER_CRITICAL();				//进入临界区(无法被中断打断)  
	
	
	//wifiEMW任务
 	OSTaskCreate(network_task,(void *)0,       
		(OS_STK*)&GPRS_TASK_STK[GPRS_STK_SIZE-1],
		GPRS_TASK_PRIO);	 
	
	//主任务
 	OSTaskCreate(MasterCtrl_task,(void *)0,
		(OS_STK*)&MASTER_TASK_STK[MASTER_STK_SIZE-1],
		MASTER_TASK_PRIO);	 	
	
	//触摸屏界面任务
 	OSTaskCreate(Task_HMIMonitor,(void *)0,
		(OS_STK*)&HMI_TASK_STK[HMI_STK_SIZE-1],
		HMI_TASK_PRIO);	 		
			   
	//三元组任务
 	OSTaskCreate(key_task,(void *)0,
		(OS_STK*)&KEY_TASK_STK[KEY_STK_SIZE-1],
		KEY_TASK_PRIO);	 				

	//LED任务
 	OSTaskCreate(led_task,(void *)0,
		(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],
		LED_TASK_PRIO);	

	//土壤温湿度传感器任务
	OSTaskCreate(cgq_tr_task,(void *)0,
		(OS_STK*)&CGQ_TR_TASK_STK[CGQ_TR_STK_SIZE-1],
		CGQ_TR_TASK_PRIO);		
 	
	
 	OSTaskSuspend(START_TASK_PRIO);		//挂起起始任务.
	OS_EXIT_CRITICAL();					//退出临界区(可以被中断打断)
}



//LED任务
void led_task(void *pdata)
{
	
	while(1)
	{

		delay_ms(10);

	}									 
}


//土壤温湿度传感器任务
void cgq_tr_task(void *pdata)
{
	
  FlashReadZDMS_QDYZ(&Guangai.qdyzbz);//从flash中读取启动阈值
	if(Guangai.qdyzbz==0XFF)
	{
			Guangai.qdyzbz=16;
			FlashWriteZDMS_QDYZ(&Guangai.qdyzbz);//先flash中写入启动阈值		
	}
	
  FlashReadZDMS_TZYZ(&Guangai.tzyzbz);//从flash中读取停止阈值
	if(Guangai.tzyzbz==0XFF)
	{
			Guangai.tzyzbz=24;
			FlashWriteZDMS_TZYZ(&Guangai.tzyzbz);//先flash中写入停止阈值		
	}	
	
	while(1)
	{		
		delay_ms(10);
		
    CGQ_cx();//传感器程序
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


void	readthree()								//从COM3 485串口读取是否设置了三元组数据
{
	u8 len,i;
	char hc[250];
	char *msg=hc;
	u8 buf[250];
	memset(buf,0,sizeof buf);
	memset(hc,0,sizeof hc);
	len=0;i=0;
	len=COM3GetBuf(buf,200);
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
			memset(buf,0,sizeof buf);
			memset(hc,0,sizeof buf);
			len=1;
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
		else return;
		
	}
}


//三元组数据
void key_task(void *pdata)
{	
	while(1)
	{
		delay_ms(200);
		readthree();					//判断是否扫码枪发送三元组数据  	//从COM3 485串口读取是否设置了三元组数据
	}
}
