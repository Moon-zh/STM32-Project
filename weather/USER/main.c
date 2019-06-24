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
#include "NeowayN21.h"
#include "stmflash.h"
#include "LED_dz.h"
 
/************************************************
 ALIENTEK战舰STM32开发板实验53
 UCOSII实验3-消息队列、信号量集和软件定时器  实验 
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/

extern Environmental sensor;

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
 			   
//LED任务
//设置任务优先级
#define LED_TASK_PRIO       			7 
//设置任务堆栈大小
#define LED_STK_SIZE  		    		64
//任务堆栈
OS_STK LED_TASK_STK[LED_STK_SIZE];
//任务函数
void led_task(void *pdata);

//温度采集任务
//设置任务优先级
#define ZONE_TASK_PRIO       		 	6
//设置任务堆栈大小
#define ZONE_STK_SIZE  				256
//任务堆栈	
OS_STK ZONE_TASK_STK[ZONE_STK_SIZE];
//任务函数
void Temperature_task(void *pdata);

//主任务
//设置任务优先级
#define MASTER_TASK_PRIO       			13 
//设置任务堆栈大小
#define MASTER_STK_SIZE  					256
//任务堆栈	
OS_STK MASTER_TASK_STK[MASTER_STK_SIZE];
//任务函数
void MasterCtrl_task(void *pdata);

//信号量集任务
//设置任务优先级
#define FLAGS_TASK_PRIO       			9
//设置任务堆栈大小
#define FLAGS_STK_SIZE  		 		56
//任务堆栈	
OS_STK FLAGS_TASK_STK[FLAGS_STK_SIZE];
//任务函数
void flags_task(void *pdata);

//GPRS任务
//设置任务优先级
#define GPRS_TASK_PRIO       			12 
//设置任务堆栈大小
#define GPRS_STK_SIZE  		 			512
//任务堆栈	
OS_STK GPRS_TASK_STK[GPRS_STK_SIZE];
//任务函数
void gprs_task(void * pdata);

//界面任务
//设置任务优先级
#define HMI_TASK_PRIO       			8 
//设置任务堆栈大小
#define HMI_STK_SIZE  		 			512
//任务堆栈	
OS_STK HMI_TASK_STK[HMI_STK_SIZE];
//任务函数
void Task_HMIMonitor(void * pdata);


//按键扫描任务
//设置任务优先级
#define KEY_TASK_PRIO       			11 
//设置任务堆栈大小
#define KEY_STK_SIZE  					56
//任务堆栈	
OS_STK KEY_TASK_STK[KEY_STK_SIZE];
//任务函数
void key_task(void *pdata);

//设置任务优先级
#define UpyunWF_TASK_PRIO       		2 
//设置任务堆栈大小
#define UpyunWF_STK_SIZE  		    	1024
//任务堆栈	
__align(8)OS_STK UpyunWF_TASK_STK[UpyunWF_STK_SIZE];
//任务函数
void UpyunWF_task(void *pdata);

//设置任务优先级
#define Upyun_TASK_PRIO       			1 
//设置任务堆栈大小
#define Upyun_STK_SIZE  		    	1024
//任务堆栈	
__align(8)OS_STK Upyun_TASK_STK[Upyun_STK_SIZE];
//任务函数
void Upyun_task(void *pdata);

#define LED_DZ_TASK_PRIO       			3 
//设置任务堆栈大小
#define LED_DZ_STK_SIZE  		    	1024
//任务堆栈	
__align(8)OS_STK LED_DZ_TASK_STK[LED_DZ_STK_SIZE];
//任务函数
void LED_DZ_task(void *pdata);

#define SaveThree_TASK_PRIO       			4 
//设置任务堆栈大小
#define SaveThree_STK_SIZE  		    	1024
//任务堆栈	
__align(8)OS_STK SaveThree_TASK_STK[SaveThree_STK_SIZE];
//任务函数
void SaveThree_task(void *pdata);
//////////////////////////////////////////////////////////////////////////////
    
OS_EVENT * msg_key;			//按键邮箱事件块	  
OS_EVENT * q_msg;			//消息队列
OS_TMR   * tmr1;			//软件定时器1
OS_TMR   * tmr2;			//软件定时器2
OS_TMR   * tmr3;			//软件定时器3
OS_FLAG_GRP * flags_key;	//按键信号量集
void * MsgGrp[256];			//消息队列存储地址,最大支持256个消息

u8	N21BZ=0;			//为了避免N21发送冲突，判断是否N21在发送数据
u8	READBZ=1;			//判断是否是一个上传周期结束，用于报警是否上传
u16	time=0;
u8	LED_BZ=0;
unsigned char printf_num=1;	//printf 串口指向标志 1指向uart1 2指向uart2
unsigned char N21_B=0;		//NB模组初始化完成标志
unsigned char Emw_B=0;		//wifi模组初始化完成标志
unsigned char emw_set=1;	//wifi报警与线程互斥标志
unsigned char n21_set=1;	//N21报警与线程互斥标志
unsigned int  up_time=15;		//数据上传频率
unsigned int  up_wartime=2;	//报警上传频率

extern char ssid[20];
extern char password[20];

extern char ProductKey1[20];
extern char DeviceName1[50];
extern char DeviceSecret1[50];

extern char *ProductKeyw;
extern char *DeviceNamew;
extern char *DeviceSecretw;

//鉴权信息
extern char ProductKey0[20];
extern char DeviceName0[50];
extern char DeviceSecret0[50];

extern char *ProductKey;
extern char *DeviceName;
extern char *DeviceSecret;

extern char CCID[25];


#define FLASH_SAVE_ADDR  0X0802DE00
#define FLASH_THREE_ADDR  0X0802FE00

void	readflashthree(void);
int main(void)
{	 		    
	SystemInit();
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	bsp_InitUart();				//初始化RS485
	OSInit();  	 				//初始化UCOSII
	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();	  
//	readflashthree();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void	uptodz()						//更改LED点阵显示内容
{
	setdz();
	setprogram1(sensor);
	setprogram2(sensor);
}

void 	LED_DZ_task(void *pdata)		//LED点阵显示任务
{	 
	u8 i;
	while(1)
	{
		delay_ms(200);
		if(++i==80)
		{
			i=0;
			uptodz();
		}	
	}
}

//开始任务
void 	start_task(void *pdata)
{
    OS_CPU_SR cpu_sr=0;
//	u8 err;	    	    
	pdata = pdata; 	
//	RTC_Init();
//	OSStatInit();						//初始化统计任务.这里会延时1秒钟左右	
 	OS_ENTER_CRITICAL();				//进入临界区(无法被中断打断)    
	//delay_ms(1000);
 OSTaskCreate(led_task,(void *)0,
		(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],
		LED_TASK_PRIO);		
	
 	OSTaskCreate(ZoneCtrl_task,(void *)0,
		(OS_STK*)&ZONE_TASK_STK[ZONE_STK_SIZE-1],
		ZONE_TASK_PRIO);	
	
	OSTaskCreate(UpyunWF_task,(void *)0,
		(OS_STK*)&UpyunWF_TASK_STK[UpyunWF_STK_SIZE-1],
		UpyunWF_TASK_PRIO);
	
	OSTaskCreate(Upyun_task,(void *)0,
	(OS_STK*)&Upyun_TASK_STK[Upyun_STK_SIZE-1],
	Upyun_TASK_PRIO);

	OSTaskCreate(LED_DZ_task,(void *)0,
		(OS_STK*)&LED_DZ_TASK_STK[LED_DZ_STK_SIZE-1],
		LED_DZ_TASK_PRIO);

	OSTaskCreate(SaveThree_task,(void *)0,
		(OS_STK*)&SaveThree_TASK_STK[SaveThree_STK_SIZE-1],
		SaveThree_TASK_PRIO);
	
 	OSTaskSuspend(START_TASK_PRIO);		//挂起起始任务.
	OS_EXIT_CRITICAL();					//退出临界区(可以被中断打断)
}

void	readflashthree()				//从flash中读取三元组数据
{
	char buf[150];
	char *msg=buf;
	u8 i;
	STMFLASH_Read(FLASH_THREE_ADDR,(u16*)buf,55);
//	if((buf[0]==0)|(buf[0]==0xff))return;
	if(buf[1]=='K')
	{
		memset(ProductKey0,0,20);
		msg=buf+4;for(i=0;*msg;msg++)		ProductKey[i++]=*msg;
	}
	if(buf[25]=='N')
	{
		memset(DeviceName0,0,20);
		msg=buf+24+4;for(i=0;*msg;msg++)	DeviceName[i++]=*msg;
	}
	if(buf[59]=='S')
	{
		memset(DeviceSecret0,0,40);
		msg=buf+62;for(i=0;*msg;msg++)	DeviceSecret[i++]=*msg;
	}
	
	STMFLASH_Read(100+FLASH_THREE_ADDR,(u16*)buf,70);
//	if((buf[0]==0)|(buf[0]==0xff))return;
	if(buf[1]=='K')
	{
		memset(ProductKey1,0,20);
		msg=buf+4;for(i=0;*msg;msg++)		ProductKeyw[i++]=*msg;
	}
	if(buf[25]=='N')
	{
		memset(DeviceName1,0,50);
		msg=buf+24+4;for(i=0;*msg;msg++)	DeviceNamew[i++]=*msg;
	}
	if(buf[79]=='S')
	{
		memset(DeviceSecret1,0,50);
		msg=buf+78+4;for(i=0;*msg;msg++)	DeviceSecretw[i++]=*msg;
	}

}

void	sendflashthree(u8 group)		//向内存写入设置的三元组数据
{
	
	LED_BZ=1;
	if(group==1)
	{
		STMFLASH_Write(FLASH_THREE_ADDR,(u16*)"PK:0",4);
		STMFLASH_Write(FLASH_THREE_ADDR+4,(u16*)ProductKey0,sizeof(ProductKey0));
		STMFLASH_Write(FLASH_THREE_ADDR+24,(u16*)"DN:0",4);
		STMFLASH_Write(FLASH_THREE_ADDR+28,(u16*)DeviceName0,sizeof(DeviceName0));
		STMFLASH_Write(FLASH_THREE_ADDR+58,(u16*)"DS:0",4);
		STMFLASH_Write(FLASH_THREE_ADDR+62,(u16*)DeviceSecret0,sizeof(DeviceSecret0));
	}
	else
	{
		STMFLASH_Write(100+FLASH_THREE_ADDR,(u16*)"PK:0",4);
		STMFLASH_Write(100+FLASH_THREE_ADDR+4,(u16*)ProductKey1,sizeof(ProductKey1));
		STMFLASH_Write(100+FLASH_THREE_ADDR+24,(u16*)"DN:0",4);
		STMFLASH_Write(100+FLASH_THREE_ADDR+28,(u16*)DeviceName1,sizeof(DeviceName1));
		STMFLASH_Write(100+FLASH_THREE_ADDR+78,(u16*)"DS:0",4);
		STMFLASH_Write(100+FLASH_THREE_ADDR+82,(u16*)DeviceSecret1,sizeof(DeviceSecret1));
	}
	readflashthree();
	LED_BZ=0;
}

void	sendflash(void);
void	readflash()						//从flash读取报警设置
{
	u16 data;
	u16 buf[50];
	u8 i,k;

	memset(buf,0,sizeof buf);
	STMFLASH_Read(400+FLASH_SAVE_ADDR+200,(u16*)&data,1);	//是否是出厂
	if(data!=0xaaaa){sendflash(); return;}

	STMFLASH_Read(400+FLASH_SAVE_ADDR,buf,50);				//读取报警设置
	
	STMFLASH_Read(450+FLASH_SAVE_ADDR,buf,20);				//读取ssid
	for(k=i=0;i<20;i+=2,k++)
	{
		ssid[i]=buf[k]>>8;
		ssid[i+1]=buf[k]&0xff;
	}

	STMFLASH_Read(500+FLASH_SAVE_ADDR,buf,20);				//读取password
	for(k=i=0;i<20;i+=2,k++)
	{
		password[i]=buf[k]>>8;
		password[i+1]=buf[k]&0xff;
	}
	
	STMFLASH_Read(610+FLASH_SAVE_ADDR,(u16*)&data,1);		//读取上报时间
	if(data>=1&&data<0xffff)up_time=data;
	
	STMFLASH_Read(620+FLASH_SAVE_ADDR,(u16*)&data,1);		//读取报警上报时间
	if(data>=1&&data<0xffff)up_wartime=data;			
}

void	sendflash()						//存储报警设置到flash
{
	u16 bz=0xaaaa;
	u16 buf[50];
	u8 i,k;
	
	STMFLASH_Write(400+FLASH_SAVE_ADDR,(u16*)buf,48);			//写入报警设置
	delay_ms(200);
	STMFLASH_Write(400+FLASH_SAVE_ADDR+200,(u16*)&bz,2);		//写入出厂标志
	delay_ms(100);
	STMFLASH_Write(610+FLASH_SAVE_ADDR,(u16*)&up_time,2);		//写入上报时间
	STMFLASH_Write(620+FLASH_SAVE_ADDR,(u16*)&up_wartime,2);	//写入报警上报时间

	
	for(k=i=0;i<20;i++)
	{
		buf[i]=ssid[k++]<<8;
		buf[i]=buf[i]|ssid[k++];
	}
	
	STMFLASH_Write(450+FLASH_SAVE_ADDR,(u16*)buf,20);			//写入ssid

	for(k=i=0;i<20;i++)
	{
		buf[i]=password[k++]<<8;
		buf[i]=buf[i]|password[k++];
	}
	
	STMFLASH_Write(500+FLASH_SAVE_ADDR,(u16*)buf,20);			//写入password
	delay_ms(200);
	readflash();
}

void	readthree()						//从COM3 485串口读取是否设置了三元组数据
{
	u8 len,i;
	char hc[250];
	char *msg=hc;
	u8 buf[100];
	memset(buf,0,sizeof buf);
	memset(hc,0,sizeof hc);
	len=0;i=0;
	len=COM5GetBuf(buf,99);
	if(len>50)
	{
		if(strstr((const char *)buf,"m1")[0]=='m')
		{
			msg=strstr((const char *)buf,"PK:");
			if(msg[1]=='K')
			{
				memset(ProductKey0,0,20);
				msg+=3;for(i=0;*msg!=',';msg++)	{ProductKey[i++]=*msg;	if(i>=20)return ;}
			}
			msg=strstr((const char *)buf,"DN:");
			if(msg[1]=='N')
			{
				memset(DeviceName0,0,50);
				msg+=3;for(i=0;*msg!=',';msg++)	{DeviceName[i++]=*msg;	if(i>=50)return ;}
			}
			msg=strstr((const char *)buf,"DS:");
			if(msg[1]=='S')
			{
				memset(DeviceSecret0,0,50);
				msg+=3;for(i=0;*msg;msg++)		{DeviceSecret[i++]=*msg;if(i>=50)return ;}
			}
			len=1;
		}
		else if(strstr((const char *)buf,"m2")[0]=='m')
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
				memset(DeviceSecret1,0,50);
				msg+=3;for(i=0;*msg;msg++)		{DeviceSecretw[i++]=*msg;if(i>=50)return ;}
			}
			len=2;
		}
		else if(strstr((const char *)buf,"wf")[0]=='w')
		{
			
		}
		else return;	
		sendflashthree(len);
		comSendBuf(COM5,buf,strlen((char *)buf));
		comClearRxFifo(COM5);
		memset(buf,0,sizeof buf);
		memset(hc,0,sizeof buf);
	}
}

void	readset(u8 t)					//读取阿里下发的数据
{
	u8 len;
	u8 buf[250];
	char c[100];
	char *msg=c;
	if(n21_set)goto qw;
	len=COM1GetBuf(buf,100);
	if(len>10)
	{
		delay_ms(200);COM1GetBuf(buf,100);			//读取主设备下发的数据
		if(strstr((const char *)buf,"RECV")[0]!='R'){memset(buf,0,200);memset(c,0,200);comClearRxFifo(COM1);return;}
		msg=strstr((const char *)buf,"up_time:");		if(msg[0]=='u')up_time=							(msg[8]-0x30)*10000+(msg[9]-0x30)*1000+(msg[10]-0x30)*100+(msg[11]-0x30)*10+(msg[12]-0x30);
		msg=strstr((const char *)buf,"up_wartime:");	if(msg[0]=='u')up_wartime=						(msg[11]-0x30)*10000+(msg[12]-0x30)*1000+(msg[13]-0x30)*100+(msg[14]-0x30)*10+(msg[15]-0x30);
		
		sendflash();
		comClearRxFifo(COM1);

		if(t)OSTaskSuspend(UpyunWF_TASK_PRIO);

		if(t)OSTaskResume(UpyunWF_TASK_PRIO);
	}
	else
	{
qw:		if(emw_set)return;
		len=COM2GetBuf(buf,100);
		if(len>10)
		{
			delay_ms(200);COM2GetBuf(buf,100);		//读取从设备下发的数据
			if(strstr((const char *)buf,"RECV")[0]!='R'){memset(buf,0,200);memset(c,0,200);comClearRxFifo(COM2);return;}
			msg=strstr((const char *)buf,"up_time:");		if(msg[0]=='u')up_time=							(msg[8]-0x30)*10000+(msg[9]-0x30)*1000+(msg[10]-0x30)*100+(msg[11]-0x30)*10+(msg[12]-0x30);
			msg=strstr((const char *)buf,"up_wartime:");	if(msg[0]=='u')up_wartime=						(msg[11]-0x30)*10000+(msg[12]-0x30)*1000+(msg[13]-0x30)*100+(msg[14]-0x30)*10+(msg[15]-0x30);
			
			sendflash();
			comClearRxFifo(COM2);
			
			if(!t)OSTaskSuspend(Upyun_TASK_PRIO);

			if(!t)OSTaskResume(Upyun_TASK_PRIO);
			
		}
	}
	memset(buf,0,200);
	memset(c,0,200);
}

u8 Nsendok=0;
void	Upccid()										//上报CCID
{
	char c[50];
	char *b=c;
	sprintf(b,"\\\"CCID\\\":\\\"%s\\\"",CCID);
	Nsendok=sendN21(b,0);
}
void	Uptoaliyun_wifi()	//上传传感器数据到阿里云 WIFI
{
	char a[400];
	char c[250];
	char *b=c;
	u8 i;i=0;
	memset(a,0,sizeof a);
	
//	sensor.temp=(rand()%100)*10;
//	sensor.humi=(rand()%100)*10;
//	sensor.pm25=rand()%1000;
//	sensor.co2=rand()%1000;
//	sensor.vol=rand()%1000;
//	sensor.pm10=rand()%1000;
//	sensor.windspeed=rand()%10;
//	sensor.winddir=rand()%16;
//	sensor.rain=rand()%30;
//	sensor.snow=rand()%1;
//	sensor.light=rand()%100000;
//	sensor.bmp=rand()%10000;
	
	sprintf(b,"\"airtemp_1\":%2.1f,",(float)(sensor.temp/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"airhumi_1\":%2.1f,",(float)(sensor.humi/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"pm25_1\":%d,",sensor.pm25);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"CO2_1\":%d,",sensor.co2);								for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"sound_1\":%d,",sensor.vol);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"pm10_1\":%d,",sensor.pm10);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"windspeed_1\":%d,",sensor.windspeed);					for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"winddir_1\":%d,",sensor.winddir);						for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"rain_1\":%d,",sensor.rain);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"snow_1\":%d,",sensor.snow);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"light_1\":%d,",sensor.light);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"bmp_1\":%d,",sensor.bmp);								for(;*b;b++)a[i++]=*b;
	
	sendEmw(a,0);
}

void	Uptoaliyun()			//上传传感器数据到阿里云
{
	char a[250];
	char c[250];
	char *b=c;
	u8 i;i=0;
	memset(a,0,sizeof a);
	
	sprintf(b,"\\\"airtemp_1\\\":%2.1f,",(float)(sensor.temp/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"airhumi_1\\\":%2.1f,",(float)(sensor.humi/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"pm25_1\\\":%d,",sensor.pm25);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"CO2_1\\\":%d,",sensor.co2);								for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"sound_1\\\":%d,",sensor.vol);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"pm10_1\\\":%d,",sensor.pm10);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"windspeed_1\\\":%2.1f,",(float)(sensor.windspeed/10.0));	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"winddir_1\\\":%d,",sensor.winddir);						for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"rain_1\\\":%d,",sensor.rain);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"snow_1\\\":%d,",sensor.snow);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"light_1\\\":%d,",sensor.light);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"bmp_1\\\":%d,",sensor.bmp);								for(;*b;b++)a[i++]=*b;
	
	Nsendok=sendN21(a,0);
}
//void	Uptoaliyun_wifi2()	//上传传感器数据到阿里云
//{
//	char a[400];
//	char c[250];
//	char *b=c;
//	u8 i;i=0;
//	memset(a,0,sizeof a);
//	sprintf(b,"\"airtemp_1\":%2.1f,",(float)(sensor.temp/10.0));		for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\"airhumi_1\":%2.1f,",(float)(sensor.humi/10.0));		for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\"pm25_1\":%d,",sensor.pm25);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\"CO2_1\":%d,",sensor.co2);								for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\"sound_1\":%d,",sensor.vol);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\"pm10_1\":%d,",sensor.pm10);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\"windspeed_1\":%d,",sensor.windspeed);					for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\"winddir_1\":%d,",sensor.winddir);						for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\"rain_1\":%d,",sensor.rain);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\"snow_1\":%d,",sensor.snow);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\"light_1\":%d,",sensor.light);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\"bmp_1\":%d,",sensor.bmp);								for(;*b;b++)a[i++]=*b;
//	
//	sendEmw2(a,0);
//}

extern uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];
void 	Upyun_task(void *pdata)							//上传云任务 N21
{
	u8 i;
	u8 buf[20];
	NeoWayN21_init();					//N21初始化 注网
	conN21();							//N21连接到阿里云
	for(i=0;i<3;i++)					//等待N21完全初始化完
	delay_ms(1000);
	Upccid();							//上报CCID
	memset(g_RxBuf1,0,UART1_RX_BUF_SIZE);
	N21_B=1;n21_set=0;
	while(1)
	{
		delay_ms(200);
		if(((++time)/250)>=up_time)					//发送间隔，每次只发送一组
		{
			n21_set=1;
			comClearRxFifo(COM1);
			memset(g_RxBuf1,0,UART1_RX_BUF_SIZE);
			printf_num=1;
			printf("AT\r\n");
			delay_ms(100);
			printf_num=1;
			printf("AT\r\n");
			delay_ms(300);
			COM1GetBuf(buf,15);
			if(strchr((const char *)buf,'K')[0]!='K')
			{
				n21_set=1;
				N21_B=0;
				NeoWayN21_init();					//N21初始化 注网
				conN21();							//N21连接到阿里云
				memset(buf,0,10);
				n21_set=0;
				N21_B=1;
				memset(g_RxBuf1,0,UART1_RX_BUF_SIZE);
			}
			time=0;
			Uptoaliyun();
			n21_set=0;
		}
		if(!Nsendok){n21_set=1;N21_B=0;NeoWayN21_init();conN21();n21_set=0;N21_B=1;}
		readset(0);						//读取阿里云下发数据	
	}
}

void 	UpyunWF_task(void *pdata)						//上传云任务 WIFI
{
	u16 i;u8 error=0;u8 buf[100];
	Emw3060_init();
	Emw3060_con();
	Emw_B=1;emw_set=0;
	while(1)
	{
		delay_ms(200);
		readset(1);										//读取阿里云下发数据
		comClearRxFifo(COM2);
		printf_num=2;
		printf("AT+WJAPS\r");
		delay_ms(200);
		COM2GetBuf(buf,45);
		if(strstr((const char *)buf,"STATION_UP")[0]!='S')
		{
			if(++error==10){emw_set=1;Emw_B=0;Emw3060_init();Emw3060_con();Emw_B=1;emw_set=0;}
			continue;
		}
		else error=0;		
		if(((++i)/250)>=up_time){i=0;Uptoaliyun_wifi();}
	}
}

void 	SaveThree_task(void *pdata)						//三元组存储任务
{
	while(1)
	{
		delay_ms(200);
		readthree();							//判断是否扫码枪发送三元组数据
	}
}

char *str()
{
	static char a[250];
	char c[150];
	char *b=c;

//	sprintf(b,"\\\"airtemp_1\\\":%2.1f,",(float)(temp/10.0));		for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\\\"airhumi_1\\\":%2.1f,",(float)(humi/10.0));		for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\\\"pm25_1\\\":%d,",pm25);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\\\"CO2_1\\\":%d,",co2);								for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\\\"sound_1\\\":%d,",vol);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\\\"pm10_1\\\":%d,",pm10);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\\\"windspeed_1\\\":%d,",windspeed);					for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\\\"winddir_1\\\":%d,",winddir);						for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\\\"rain_1\\\":%d,",rain);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\\\"snow_1\\\":%d,",snow);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\\\"light_1\\\":%d,",light);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\\\"bmp_1\\\":%d,",bmp);								for(;*b;b++)a[i++]=*b;
	b=a;
	return b;
}


//LED任务
void led_task(void *pdata)
{
	u8 i=0;
	while(1)
	{
		delay_ms(200);
		if(LED_BZ)
		{
			LED0=!LED0;		//提示系统正在写入三元组
		}
		else
		{
			if(++i==5)
			i=0,LED0=!LED0;	//提示系统正在运行
		}
		switch(Emwled)
		{
			case 0:			//关闭指示
				EmwLED1=0;
				EmwLED2=0;
				break;
			case 1:			//正在连接wifi
				EmwLED1=!EmwLED1;
				break;
			case 2:			//正在连接阿里  或  正在上报数据
				EmwLED1=1;
				EmwLED2=!EmwLED2;
				break;
			case 3:			//上报完成
				EmwLED2=0;
				break;
		}
//		switch(Emwled2)
//		{
//			case 0:
//				Emw2LED1=0;
//				Emw2LED2=0;
//				break;
//			case 1:
//				Emw2LED1=!Emw2LED1;
//				break;
//			case 2:
//				Emw2LED1=1;
//				Emw2LED2=!Emw2LED2;
//				break;
//			case 3:
//				Emw2LED2=0;
//				break;
//		}
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
   		    
//按键扫描任务
void key_task(void *pdata)
{	
	while(1)
	{
 		delay_ms(10);
	}
}
