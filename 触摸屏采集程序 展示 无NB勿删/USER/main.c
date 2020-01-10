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
#include "my_HDMI.h"
#include "LED_dz.h"
#include "stmflash.h"

#define FLASH_SAVE_ADDR  0X0802DE00

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
#define Alarm_TASK_PRIO       			7 
//设置任务堆栈大小
#define Alarm_STK_SIZE  		    	128
//任务堆栈	
OS_STK Alarm_TASK_STK[Alarm_STK_SIZE];
//任务函数
void Alarm_task(void *pdata);

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
#define Collection_TASK_PRIO       		4 
//设置任务堆栈大小
#define Collection_STK_SIZE  		    700
//任务堆栈	
OS_STK Collection_TASK_STK[Collection_STK_SIZE];
//任务函数
void Collection_task(void *pdata);

//设置任务优先级
#define LED_DZ_TASK_PRIO       		3 
//设置任务堆栈大小
#define LED_DZ_STK_SIZE  		    512
//任务堆栈	
OS_STK LED_DZ_TASK_STK[LED_DZ_STK_SIZE];
//任务函数
void LED_DZ_task(void *pdata);

#define 	CMD_AIRTEMP		0x01
#define		CMD_AIRHUMI 	0x00
#define		CMD_SOILTEMP	0x03
#define		CMD_SOILHUMI	0x02
#define		CMD_CO2			0x05
#define 	CMD_LIGTH		0x07
#define		CMD_EC			0x15
#define		EnvNum			5		//传感器最大上限

u8	sensor_num=1;

#ifndef Env
#define Env
typedef struct
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

Environmental sensor[EnvNum];			//传感器数量上限

void	Alarm()							//报警判断
{
	u8 i;
	u8 Alarm_bit;
	Alarm_bit=0;
	while(1)
	{
		if((sensor[0].airtemp>Alarm_airtemp_up)|(sensor[0].airtemp<Alarm_airtemp_dn))			//气温
		{
			SetControlForeColor(1,4,0xf800);
			delay_ms(100);Alarm_bit=1;
		}
		if((sensor[0].airhumi>Alarm_airhumi_up)|(sensor[0].airhumi<Alarm_airhumi_dn))			//气湿
		{
			SetControlForeColor(1,5,0xf800);
			delay_ms(100);Alarm_bit=1;
		}
		if((sensor[0].light>Alarm_light_up)|(sensor[0].light<Alarm_light_dn))					//光强
		{
			SetControlForeColor(1,6,0xf800);
			delay_ms(100);Alarm_bit=1;
		}
		if((sensor[0].CO2>Alarm_CO2_up)|(sensor[0].CO2<Alarm_CO2_dn))							//CO2
		{
			SetControlForeColor(1,7,0xf800);
			delay_ms(100);Alarm_bit=1;
		}
		if((sensor[0].soilhumi>Alarm_soilhumi_up)|(sensor[0].soilhumi<Alarm_soilhumi_dn))		//土温
		{
			SetControlForeColor(1,8,0xf800);
			delay_ms(100);Alarm_bit=1;
		}
		if((sensor[0].soiltemp>Alarm_soiltemp_up)|(sensor[0].soiltemp<Alarm_soiltemp_dn))		//土湿
		{
			SetControlForeColor(1,9,0xf800);
			delay_ms(100);Alarm_bit=1;
		}
		if((sensor[0].EC>Alarm_EC_up)|(sensor[0].EC<Alarm_EC_dn))								//电导率
		{
			SetControlForeColor(1,10,0xf800);
			delay_ms(100);Alarm_bit=1;
		}
		if(Alarm_bit==0)								//解除报警
		{
			for(i=4;i<11;i++)
			{
				delay_ms(100);
				SetControlForeColor(1,i,0x1c1c);
			}
			delay_ms(100);
			Alarm_bit=0;
			return;
		}
		break;
	}
}

void	Update_Sensor_Number(u8 num,Environmental data)	//更新传感器显示
{
	//空气四参数
	delay_ms(50);
	SetTextFloat(num,4,(float)data.airtemp/10,1,1);
	delay_ms(50);
	SetTextFloat(num,5,(float)data.airhumi/10,1,1);
	delay_ms(50);
	SetTextValueInt32(num,6,data.light);
	delay_ms(50);
	SetTextValueInt32(num,7,data.CO2);
	delay_ms(50);
	SetTextFloat(num,9,(float)data.soiltemp/10,1,1);
	delay_ms(50);
	SetTextFloat(num,8,(float)data.soilhumi/10,1,1);
	delay_ms(50);
	SetTextValueInt32(num,10,data.EC);

}

void 	UpdateUI()						//更新触摸屏显示
{
//	delay_ms(100);
//	SetTextValueInt32(0,3,group);
	delay_ms(100);
	Update_Sensor_Number(1,sensor[0]);delay_ms(100);
//	Update_Sensor_Number(2,sensor[1]);delay_ms(100);
//	Update_Sensor_Number(3,sensor[2]);delay_ms(100);
}

void	setalarm()						//显示报警设置值
{
	SetTextValueInt32(2,3,Alarm_airtemp_up/10);delay_ms(100);
	SetTextValueInt32(2,4,Alarm_airtemp_dn/10);delay_ms(100);
	SetTextValueInt32(2,5,Alarm_airhumi_up/10);delay_ms(100);
	SetTextValueInt32(2,6,Alarm_airhumi_dn/10);delay_ms(100);
	SetTextValueInt32(2,7,Alarm_light_up);delay_ms(100);
	SetTextValueInt32(2,8,Alarm_light_dn);delay_ms(100);
	SetTextValueInt32(2,9,Alarm_CO2_up);delay_ms(100);
	SetTextValueInt32(2,10,Alarm_CO2_dn);delay_ms(100);
	SetTextValueInt32(2,11,Alarm_soiltemp_up/10);delay_ms(100);
	SetTextValueInt32(2,12,Alarm_soiltemp_dn/10);delay_ms(100);
	SetTextValueInt32(2,13,Alarm_soilhumi_up/10);delay_ms(100);
	SetTextValueInt32(2,14,Alarm_soilhumi_dn/10);delay_ms(100);
	SetTextValueInt32(2,15,Alarm_EC_up);delay_ms(100);
	SetTextValueInt32(2,16,Alarm_EC_dn);delay_ms(100);
}

void	HDMI_Init()						//触摸屏初始化
{
	queue_reset();			//清空缓存区
	delay_ms(300);			//必须等待300ms
	UpdateUI();
	setalarm();
}

void	init()							//系统初始化
{
	SystemInit();
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	LED_Init();		  		//初始化与LED连接的硬件接口	
	bsp_InitUart();
	HDMI_Init();
	comClearRxFifo(COM4);
//	while(!init_4G());
}

int 	main(void)						//系统开始
{	 
	init();
	OSInit();   
	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();
}

void 	start_task(void *pdata)			//任务开始
{
    OS_CPU_SR cpu_sr=0;
	pdata = pdata; 
  	OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)    
 	OSTaskCreate(Alarm_task,(void *)0,(OS_STK*)&Alarm_TASK_STK[Alarm_STK_SIZE-1],Alarm_TASK_PRIO);						   
 	OSTaskCreate(LED_task,(void *)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);
	OSTaskCreate(HDMI_task,(void *)0,(OS_STK*)&HDMI_TASK_STK[HDMI_STK_SIZE-1],HDMI_TASK_PRIO);
	OSTaskCreate(Collection_task,(void *)0,(OS_STK*)&Collection_TASK_STK[Collection_STK_SIZE-1],Collection_TASK_PRIO);
	OSTaskCreate(LED_DZ_task,(void *)0,(OS_STK*)&LED_DZ_TASK_STK[LED_DZ_STK_SIZE-1],LED_DZ_TASK_PRIO);
	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
}

void 	Alarm_task(void *pdata)			//报警任务
{	
	while(1)
	{	
		delay_ms(2000);
		Alarm();
	};
}

void 	LED_task(void *pdata)			//LED运行状态指示灯任务
{	 
	while(1)
	{
		delay_ms(1000);
		LED0=!LED0;//提示系统正在运行	
	}
}

void	readhdmi()						//待用...........................................
{
	static qsize  size = 0;
	while(1)
	{
		size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE); //从缓冲区中获取一条指令        
		if(size>0)//接收到指令
		{
			ProcessMessage((PCTRL_MSG)cmd_buffer, size);//指令处理
			break;
		}
	}
}


void	sendflash()						//存储报警设置到flash
{
	u16 bz=0xaaaa;
	u16 buf[16];
//	u16 *buf[]={&Alarm_airtemp_up,&Alarm_airtemp_dn,&Alarm_airhumi_up,&Alarm_airhumi_dn,&Alarm_CO2_up,&Alarm_CO2_dn,
//				&Alarm_soiltemp_up,&Alarm_soiltemp_dn,&Alarm_soilhumi_up,&Alarm_soilhumi_dn,&Alarm_EC_up,&Alarm_EC_dn};
	
				
	buf[0]=Alarm_airtemp_up;
	buf[1]=Alarm_airtemp_dn;
	buf[2]=Alarm_airhumi_up;
	buf[3]=Alarm_airhumi_dn;
	buf[4]=Alarm_CO2_up;
	buf[5]=Alarm_CO2_dn;
	buf[6]=Alarm_soiltemp_up;
	buf[7]=Alarm_soiltemp_dn;
	buf[8]=Alarm_soilhumi_up;
	buf[9]=Alarm_soilhumi_dn;
	buf[10]=Alarm_EC_up;
	buf[11]=Alarm_EC_dn;
	STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)buf,12);			
				
	
	STMFLASH_Write(FLASH_SAVE_ADDR+30,(u16*)&Alarm_light_up,2);
	STMFLASH_Write(FLASH_SAVE_ADDR+34,(u16*)&Alarm_light_dn,2);
	STMFLASH_Write(FLASH_SAVE_ADDR+200,(u16*)&bz,2);
}

void	readflash()						//从flash读取报警设置
{
	u16 data;
	u16 buf[20];
//	u16 *buf[]={&Alarm_airtemp_up,&Alarm_airtemp_dn,&Alarm_airhumi_up,&Alarm_airhumi_dn,&Alarm_CO2_up,&Alarm_CO2_up,
//				&Alarm_soiltemp_up,&Alarm_soiltemp_dn,&Alarm_soilhumi_up,&Alarm_soilhumi_dn,&Alarm_EC_up,&Alarm_EC_dn};
	
	STMFLASH_Read(FLASH_SAVE_ADDR+200,(u16*)&data,1);
	if(data!=0xaaaa){sendflash(); return;}

	STMFLASH_Read(FLASH_SAVE_ADDR,buf,20);
	
	Alarm_airtemp_up=buf[0];
	Alarm_airtemp_dn=buf[1];
	Alarm_airhumi_up=buf[2];
	Alarm_airhumi_dn=buf[3];
	Alarm_CO2_up=buf[4];
	Alarm_CO2_dn=buf[5];
	Alarm_soiltemp_up=buf[6];
	Alarm_soiltemp_dn=buf[7];
	Alarm_soilhumi_up=buf[8];
	Alarm_soilhumi_dn=buf[9];
	Alarm_EC_up=buf[10];
	Alarm_EC_dn=buf[11];
	
	Alarm_light_up=(buf[16]<<16)|buf[15];
	Alarm_light_dn=(buf[18]<<16)|buf[17];
	
//	STMFLASH_Read(FLASH_SAVE_ADDR+30,(u16*)&Alarm_light_up,2);
//	STMFLASH_Read(FLASH_SAVE_ADDR+34,(u16*)&Alarm_light_dn,2);
	setalarm();				
}

void 	HDMI_task(void *pdata)			//触摸屏监控任务
{	  
	static qsize  size = 0;
	u8 t=0,i;
//	sendflash();
	readflash();
	while(1)
	{
		delay_ms(200);
		size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE); //从缓冲区中获取一条指令        
		if(size>0)//接收到指令
		{
			ProcessMessage((PCTRL_MSG)cmd_buffer, size);//指令处理		
		}
		UpdateUI();
		if(++t==5)					//定期复位报警背景色，以防没有更新
		{
			t=0;
			for(i=4;i<11;i++)
			{
				delay_ms(100);
				SetControlForeColor(1,i,0x3186);
			}
		}

		if(SaveButton==1)			//设置按钮按下，获取设置的值
		{
//			for(i=3;i<17;i++)
//			{
//				GetControlValue(2,i);readhdmi();
//			}
			GetControlValue(2,3);
			while(1)
			{
				size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE); //从缓冲区中获取一条指令        
				if(size>0)//接收到指令
				{
					ProcessMessage((PCTRL_MSG)cmd_buffer, size);//指令处理
					break;
				}
			}
			SaveButton=0;
			sendflash();
		}
	}
}

u8		check_value[]={0x01,0x03,0x00,0x00,0x00,0x09,0xc4,0xb0};
void	sendcmd_read(u8 addr,u8 CMD)	//发送查询相关数据指令
{
	u8 i;
	u16 crc;
	check_value[0]=addr;
	crc=mc_check_crc16(check_value,6);
	check_value[6]=crc>>8;
	check_value[7]=crc&0xff;
	for(i=0;i<8;i++)
	comSendChar(COM3,check_value[i]);
	delay_ms(60);
}

u8		readcmd1(u8 i)					//读取空气传感器返回的数据
{
	u16 crc;
	u8 a[26];
	u8 len;
	len=COM3GetBuf(a,25);
	if(len<20)return 0;
	comClearRxFifo(COM3);
	crc=mc_check_crc16(a,len-2);
	if((a[len-2]==(crc>>8))&&(a[len-1]==(crc&0xff)))
	{
		sensor[i].airhumi=(a[3]<<8)|a[4];
		if(sensor[i].airhumi>990)sensor[i].airhumi=990;
		sensor[i].airtemp=(a[5]<<8)|a[6];
		sensor[i].CO2=(a[13]<<8)|a[14];
		sensor[i].light=(((a[17]<<8)|a[18])<<16)|((a[19]<<8)|a[20]);
		return 1;
	}
	return 0;
}

u8		readcmd2(u8 i)					//读取土壤传感器返回的数据
{
	u16 crc;
	u8 a[26];
	u8 len;
	len=COM3GetBuf(a,25);
	if(len<10)return 0;
	comClearRxFifo(COM3);
	crc=mc_check_crc16(a,len-2);
	if((a[len-2]==(crc>>8))&&(a[len-1]==(crc&0xff)))
	{
		sensor[i].soilhumi=(a[7]<<8)|a[8];
		if(sensor[i].soilhumi>990)sensor[i].soilhumi=990;
		sensor[i].soiltemp=(a[9]<<8)|a[10];
//		if(sensor[i].soilhumi==0)return 0;
		return 1;
	}
	return 0;
}

void	ReadValue()						//读取传感器数据
{
	u16 k;
	u8 	i;
	comClearRxFifo(COM3);
	for(i=0;i<sensor_num;i++)
	{
		do{
			sendcmd_read(1+i,CMD_AIRHUMI);			//读取空气湿度
			delay_ms(200);
			k=readcmd1(i);
			if(k)break;
		}while(1);
		delay_ms(500);
		
		do{
			sendcmd_read(50+i,CMD_AIRHUMI);			//读取土壤湿度
			delay_ms(200);
			k=readcmd2(i);
			if(k)break;
		}while(1);
		delay_ms(500);
	}
}

u8		skycmd()						//判断是否从云台收到查询指令
{
	u8 a[9];
	u8 len;
	u16 crc;
	len=COM1GetBuf(a,20);
	if(len<6)return 0;
	comClearRxFifo(COM1);
	if(a[0]!=2)return 0;
	crc=mc_check_crc16(a,len-2);
	if((a[len-2]==(crc>>8))&&(a[len-1]==(crc&0xff)))return 1;
	else return 0;
}

u8 		tab_upsky1[]={0x01,0x46,0x00,0x00,0x00,0x09,0x12,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
u8 		tab_upsky2[]={0x02,0x44,0x00,0x00,0x00,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00};
void	uptosky(u8 group)				//上传数据到云台
{
	u16 crc;
	tab_upsky1[7]=sensor[group].airhumi>>8;
	tab_upsky1[8]=sensor[group].airhumi&0xff;
	tab_upsky1[9]=sensor[group].airtemp>>8;
	tab_upsky1[10]=sensor[group].airtemp&0xff;
	tab_upsky1[17]=sensor[group].CO2>>8;
	tab_upsky1[18]=sensor[group].CO2&0xff;
	tab_upsky1[21]=sensor[group].light>>16>>8;
	tab_upsky1[22]=(sensor[group].light>>16)&0xff;
	tab_upsky1[23]=(sensor[group].light&0xffff)>>8;
	tab_upsky1[24]=(sensor[group].light&0xffff)&0xff;
	crc=mc_check_crc16(tab_upsky1,25);
	tab_upsky1[25]=crc>>8;
	tab_upsky1[26]=crc&0xff;
	comSendBuf(COM1,tab_upsky1,27);
	
	delay_ms(500);
	
	tab_upsky2[7]=sensor[group].soiltemp*10>>8;
	tab_upsky2[8]=sensor[group].soiltemp*10&0xff;
	tab_upsky2[9]=sensor[group].soilhumi*10>>8;
	tab_upsky2[10]=sensor[group].soilhumi*10&0xff;
	crc=mc_check_crc16(tab_upsky2,11);
	tab_upsky2[11]=crc>>8;
	tab_upsky2[12]=crc&0xff;
	comSendBuf(COM1,tab_upsky2,13);
	delay_ms(500);
}

static u8 dz=0;
void	uptodz()						//更改LED点阵显示内容
{
	setdz();
	switch(dz)
	{
		case 0:
			setprogram1(1,sensor[0]);
			setprogram2(1,sensor[0]);
			dz++;break;
		case 1:
			setprogram1(2,sensor[1]);
			setprogram2(2,sensor[1]);
			dz++;break;
		case 2:
			setprogram1(3,sensor[2]);
			setprogram2(3,sensor[2]);
			dz=0;break;
		default:break;
	}
}

void 	LED_DZ_task(void *pdata)		//LED点阵显示任务
{	 
	u8 i;
	while(1)
	{
		delay_ms(200);
		if(++i==40)
		{
			i=0;dz=0;
			uptodz();
		}	
	}
}

void 	Collection_task(void *pdata)	//采集任务
{
//	u8 i;
//			sensor[0].airhumi=(rand()%100)*10;
//			sensor[0].airtemp=(rand()%100)*10;
//			sensor[0].soilhumi=(rand()%100)*10;
//			sensor[0].soiltemp=(rand()%100)*10;
//			sensor[0].CO2=rand()%1000;
//			sensor[0].light=rand()%10000;
	comClearRxFifo(COM1);
	while(1)
	{
		delay_ms(1000);
		ReadValue();
//		if(skycmd())
//		{
//			//for(i=0;i<sensor_num;i++)uptosky(i);
//			uptosky(0);
//		//	skycmd();
//		}		
	}
}
