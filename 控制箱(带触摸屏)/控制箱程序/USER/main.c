#include "includes.h"
#include "led.h"
#include "delay.h"
#include "sys.h"	 
#include "Rs485.h"
#include "check.h"
#include "hmi_driver.h"
#include "hmi_user_uart.h"
#include "cmd_queue.h"
#include "cmd_process.h"
#include "my_HDMI.h"
#include "G405tf.h"
#include "kz.h"

void	UploadState(void);

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
#define LED_TASK_PRIO       			6 
//设置任务堆栈大小
#define LED_STK_SIZE  		    		64
//任务堆栈	
OS_STK LED_TASK_STK[LED_STK_SIZE];
//任务函数
void LED_task(void *pdata);

//设置任务优先级
#define HDMI_TASK_PRIO       			5 
//设置任务堆栈大小
#define HDMI_STK_SIZE  		    		512
//任务堆栈	
OS_STK HDMI_TASK_STK[HDMI_STK_SIZE];
//任务函数
void HDMI_task(void *pdata);

//设置任务优先级
#define Button_TASK_PRIO       			7 
//设置任务堆栈大小
#define Button_STK_SIZE  		    	512
//任务堆栈	
OS_STK Button_TASK_STK[Button_STK_SIZE];
//任务函数
void Button_task(void *pdata);

//设置任务优先级
#define AutoModel_TASK_PRIO       		3 
//设置任务堆栈大小
#define AutoModel_STK_SIZE  		    1024
//任务堆栈	
OS_STK AutoModel_TASK_STK[AutoModel_STK_SIZE];
//任务函数
void AutoModel_task(void *pdata);

//设置任务优先级
#define UpSky_TASK_PRIO       			4 
//设置任务堆栈大小
#define UpSky_STK_SIZE  		    	512
//任务堆栈	
OS_STK UpSky_TASK_STK[UpSky_STK_SIZE];
//任务函数
void UpSky_task(void *pdata);

void	HDMI_Init()
{
	queue_reset();			//清空缓存区
	delay_ms(300);			//必须等待300ms
}

void	init()				//系统初始化
{
	static qsize  size = 0;
	Model=0;
	SystemInit();
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	LED_Init();		  		//初始化与LED连接的硬件接口	
	comClearRxFifo(COM1);
	delay_ms(1500);
	bsp_InitUart();
	HDMI_Init();
	SetHandShake();			//触摸屏握手
	delay_ms(100);
	size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE); //从缓冲区中获取一条指令        
	if(size>0)//接收到指令
	{
		ProcessMessage((PCTRL_MSG)cmd_buffer, size);//指令处理
	}
	while(!init_4G());
}

int 	main(void)
{	 
	init();
	OSInit();   
	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();
}

//开始任务
void 	start_task(void *pdata)
{
    OS_CPU_SR cpu_sr=0;
	pdata = pdata; 
  	OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)    					   
 	OSTaskCreate(LED_task,(void *)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);
	OSTaskCreate(UpSky_task,(void *)0,(OS_STK*)&UpSky_TASK_STK[UpSky_STK_SIZE-1],UpSky_TASK_PRIO);
	if(Model)OSTaskCreate(HDMI_task,(void *)0,(OS_STK*)&HDMI_TASK_STK[HDMI_STK_SIZE-1],HDMI_TASK_PRIO);
	else OSTaskCreate(Button_task,(void *)0,(OS_STK*)&Button_TASK_STK[Button_STK_SIZE-1],Button_TASK_PRIO);
	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
}

void 	LED_task(void *pdata)		//LED运行状态指示灯任务
{	 
	while(1)
	{
		delay_ms(1000);
		LED0=!LED0;//提示系统正在运行	
	}
}

u8 tabbutton[]={0x01,0x10,0x03,0xeb,0x00,0x01,0x02,0x00,0x01,0x43,0x8b};			//设置继电器模式

void	SetIoNormal()				//设置IO模块为正常模式
{
	u8 i,k;
	u16 crc;
	for(k=0;k<3;k++)
	{
		tabbutton[0]=0xFE;
		tabbutton[8]=0;
		crc=mc_check_crc16(tabbutton,9);
		tabbutton[9]=crc>>8;
		tabbutton[10]=crc&0xff;
		for(i=0;i<11;i++)
		comSendChar(COM4,tabbutton[i]);
		delay_ms(110);
		tabbutton[0]=1;
		crc=mc_check_crc16(tabbutton,9);
		tabbutton[9]=crc>>8;
		tabbutton[10]=crc&0xff;
		for(i=0;i<11;i++)
		comSendChar(COM4,tabbutton[i]);
		delay_ms(60);
	}
}

void	SetIoLinkage()				//设置IO模块为联动模式
{
	u16 crc;
	u8 i,k;
	for(k=0;k<3;k++)
	{
		tabbutton[0]=0xFE;
		tabbutton[8]=1;
		crc=mc_check_crc16(tabbutton,9);
		tabbutton[9]=crc>>8;
		tabbutton[10]=crc&0xff;
		for(i=0;i<11;i++)
		comSendChar(COM4,tabbutton[i]);
		delay_ms(110);
		tabbutton[0]=1;
		crc=mc_check_crc16(tabbutton,9);
		tabbutton[9]=crc>>8;
		tabbutton[10]=crc&0xff;
		for(i=0;i<11;i++)
		comSendChar(COM4,tabbutton[i]);
		delay_ms(60);
	}
}

u8	sky_tab[]={0x01 ,0x45 ,0x00 ,0x00 ,0x00 ,0x0D ,0x02 ,0xff ,0xff ,0x85 ,0x13};	//开关寄存器

void	pdks(u16 data)		//上传继电器状态到云台
{
	u16 crc;
	sky_tab[7]=data>>8;
	sky_tab[8]=data&0xff;
	crc=mc_check_crc16(sky_tab,9);
	sky_tab[9]=crc>>8;
	sky_tab[10]=crc&0xff;
	comSendBuf(COM1,sky_tab,11);
	delay_ms(100);
}

u8 tabks[]={0xfd,0x01,0x00,0x00,0x00,0x08,0x29,0xc3};								//上传到云

void	UploadState()				//读取并上传继电器状态
{
	u8 a[9];
	u16 crc;
	u16 ks_state=0;
	comClearRxFifo(COM4);
	tabks[0]=0xfd;					//第一个继电器的地址
	sendcmd_kz(tabks);
	COM4GetBuf(a,6);
	comClearRxFifo(COM4);
	crc=mc_check_crc16(a,4);
	if((a[4]==crc>>8)&&(a[5]==(crc&0xff)))ks_state=a[3];
	
	memset(a,0,8);
	delay_ms(100);
	tabks[0]=1;						//第二个继电器的地址
	sendcmd_kz(tabks);
	COM4GetBuf(a,6);			
	comClearRxFifo(COM4);
	crc=mc_check_crc16(a,4);
	if((a[4]==crc>>8)&&(a[5]==(crc&0xff)))ks_state=(ks_state<<8)|a[3];
	else ks_state<<=8;

	memset(a,0,8);
	pdks(ks_state);
	delay_ms(200);
}

u8	Auto_bz=1;			//自动模式启动标志

void 	HDMI_task(void *pdata)		//液晶屏监控任务
{	  
	static qsize  size = 0;
	SetIoNormal();
	while(1)
	{
		delay_ms(200);
		size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE); //从缓冲区中获取一条指令        
		if(size>0)//接收到指令
		{
			ProcessMessage((PCTRL_MSG)cmd_buffer, size);//指令处理
		}
		if(ControlModel&Auto_bz)	//判断启动自动模式
		{
			Auto_bz=0;
			OSTaskCreate(AutoModel_task,(void *)0,(OS_STK*)&AutoModel_TASK_STK[AutoModel_STK_SIZE-1],AutoModel_TASK_PRIO);
		}
//		if(SaveButton==1)			//设置按钮按下，获取设置的值
//		{
//			GetControlValue(2,5);
//			delay_ms(50);
//			GetControlValue(2,7);
//			delay_ms(50);
//			GetControlValue(2,6);
//			delay_ms(50);
//			GetControlValue(2,8);
//			delay_ms(50);
//			SaveButton=0;
//		}
	}
}


u8	Button_bz=0;
void Button_task(void *pdata)		//无触摸屏箱任务
{
	SetIoLinkage();					//设置继电器为联动模式	
	ControlModel=1;
	Auto_bz=0;
	UploadState();
	OSTaskCreate(AutoModel_task,(void *)0,(OS_STK*)&AutoModel_TASK_STK[AutoModel_STK_SIZE-1],AutoModel_TASK_PRIO);
	Button_bz=1;
	while(1)
	{
		delay_ms(200);
	}
}

void	controlks(u8 *data)			//判断云台指令继电器执行动作	
{
	if(data[0]!=1)return;
	if(data[1]==5)
	{
		switch(data[3])
		{
			case 0:
					switch(data[4])
					{
						case 0xff:
								curtainopen();
								break;
						case 0:
								curtainclose();
								break;
						default:break;
					}
					break;
			case 2:
					switch(data[4])
					{
						case 0:
								fan1stop();
								break;
						case 0xff:
								fan1open();
								break;
						default:break;
					}
					break;
			case 3:
					switch(data[4])
					{
						case 0:
								fan1stop();
								break;
						case 0xff:
								fan1close();
								break;
						default:break;
					}
					break;
			case 4:
					switch(data[4])
					{
						case 0:
								fan2stop();
								break;
						case 0xff:
								fan2open();
								break;
						default:break;
					}
					break;
			case 5:
					switch(data[4])
					{
						case 0:
								fan2stop();
								break;
						case 0xff:
								fan2close();
								break;
						default:break;
					}
					break;
			case 6:
					switch(data[4])
					{
						case 0:
								fan3stop();
								break;
						case 0xff:
								fan3open();
								break;
						default:break;
					}
					break;
			case 7:
					switch(data[4])
					{
						case 0:
								fan3stop();
								break;
						case 0xff:
								fan3close();
								break;
						default:break;
					}
					break;
			case 8:
					switch(data[4])
					{
						case 0:
								filllight1open();
								break;
						case 0xff:
								filllight1close();
								break;
						default:break;
					}
					break;
			case 9:
					switch(data[4])
					{
						case 0:
								filllight2open();
								break;
						case 0xff:
								filllight2close();
								break;
						default:break;
					}
					break;
			case 0X0A:
					switch(data[4])
					{
						case 0:
								filllight3open();
								break;
						case 0xff:
								filllight3close();
								break;
						default:break;
					}
					break;		
			case 0x0B:
					switch(data[4])
					{
						case 0:
								moregasopen();
								break;
						case 0xff:
								moregasclose();
								break;
						default:break;
					}
					break;	
			case 0X0C:
					switch(data[4])
					{
						case 0:
								sprayopen();
								break;
						case 0xff:
								sprayclose();
								break;
						default:break;
					}
					break;
					
			default:break;
		}
	}
	delay_ms(100);
}

static u8 ks_tab[8];	//继电器控制指令存储
static u8 ks_bz=0;		//判断是否改变继电器状态

void	readks()					//判断是否从云台收到继电器控制指令
{
	u8 b[21];
	u8 a[8];
	u16 crc;
	u8 len;
	len=COM1GetBuf(b,20);
	if(len<6)return;
	if(b[1]==0x45)
	{
		for(len=0;len<8;len++)
		{
			a[len]=b[len+8];
		}
	}
	else
	{
		for(len=0;len<8;len++)
		{
			a[len]=b[len];
		}
	}
	comClearRxFifo(COM1);
	crc=mc_check_crc16(a,6);
	if((a[6]==crc>>8)&&(a[7]==(crc&0xff)))
	{
		for(;len>0;len--)ks_tab[len]=a[len];
		ks_tab[0]=a[0];
		ks_bz=1;
	}
}

void AutoModel_task(void *pdata)	//自动模式
{
	while(1)
	{
		delay_ms(200);
		readks();					//反复查找是否有远程遥控命令
		if(!ControlModel)			//切换回手动模式
		{
			Auto_bz=1;
			OSTaskSuspend(AutoModel_TASK_PRIO);
		}	
	}
}

void	UpSky_task(void *pdata)
{
	while(1)
	{
		delay_ms(200);
		if(!ControlModel)UploadState();	//自动模式下不主动上传数据
		if(Button_bz)UploadState();
		if(ks_bz)					//判断是否需要改变继电器状态
		{
			ks_bz=0;
			controlks(ks_tab);
			delay_ms(100);
			UploadState();
		}	
	}
}
