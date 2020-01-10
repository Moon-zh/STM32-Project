#include "main.h"

void	init()							//系统初始化
{
	SystemInit();
	delay_init();	    	 		  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	LED_Init();		  					
	bsp_InitUart();
	comClearRxFifo(COM4);
	memset(g_RxBuf4,0,UART4_RX_BUF_SIZE);
	memset(g_RxBuf5,0,UART5_RX_BUF_SIZE);
	comClearRxFifo(COM5);
	delay_ms(1000);
	W25QXX_Init();				//外部flash初始化
//	readflashthree();			//读取三元组
}

int 	main(void)						//主函数
{	 
	u8 err;
	init();
	OSInit();
	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );
	tmr1=OSTmrCreate(0,100,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr1_callback,0,(u8 *)tmr1,&err);
	OSTmrStart(tmr1,&err);
	OSStart();
}

void 	start_task(void *pdata)			//线程初始化
{
    OS_CPU_SR cpu_sr=0;
	pdata = pdata; 
  	OS_ENTER_CRITICAL();						   
 	OSTaskCreate(LED_task,(void *)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);							//LED灯任务
	OSTaskCreate(Main_task,(void *)0,(OS_STK*)&Main_TASK_STK[Main_STK_SIZE-1],Main_TASK_PRIO);						//喷雾任务
	OSTaskCreate(Upyun_task,(void *)0,(OS_STK*)&Upyun_TASK_STK[Upyun_STK_SIZE-1],Upyun_TASK_PRIO);					//NB任务
//	OSTaskCreate(SaveThree_task,(void *)0,(OS_STK*)&SaveThree_TASK_STK[SaveThree_STK_SIZE-1],SaveThree_TASK_PRIO);	//三元组存储任务
	OSTaskSuspend(START_TASK_PRIO);	
	OS_EXIT_CRITICAL();				
}

u16		ReadEnv(u8 addr,u8 data)
{
	u8 Env[]={01,03,00,01,00,01,0xD5,0xC9};
	u8 buf[20];
	u16 crc;
	u8 cnt=0;
	Env[0]=addr;
	Env[3]=data;
	crc=mc_check_crc16(Env,6);
	Env[6]=crc>>8;
	Env[7]=crc&0xff;
ag:	comClearRxFifo(COM4);
	comSendBuf(COM4,Env,sizeof(Env));
	delay_ms(1000);
	COM4GetBuf(buf,7);
	crc=mc_check_crc16(buf,5);
	if((buf[5]==(crc>>8))&(buf[6]==(crc&0xff)))
		return (buf[3]<<8)|buf[4];
	if(++cnt==10)return 65535;else goto ag;
}

#define	xph1				1
#define	xph2				2
#define	CMD_xph_soiltemp	0
#define	CMD_xph_soilhumi	1
#define	CMD_xph_PH			0

u16	xph_soiltemp,xph_soilhumi,xph_PH;

void	Main_task(void	*pdata)			//喷雾任务
{
	while(1)
	{
		delay_ms(200);
		xph_soiltemp=	ReadEnv(xph1,CMD_xph_soiltemp);
		xph_soilhumi=	ReadEnv(xph1,CMD_xph_soilhumi);
		xph_PH=			ReadEnv(xph2,CMD_xph_PH);
	}
}

void	Uptoaliyun()					//上传设备状态
{
	char a[250];
	char c[250];
	char *b=c;
	u8 i;i=0;
	memset(a,0,sizeof a);
	sprintf(b,"\\\"xph_soiltemp\\\":%.1f,",(float)xph_soiltemp/10);	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"xph_soilhumi\\\":%.1f,",(float)xph_soilhumi/10);	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"xph_PH\\\":%.2f",(float)xph_PH/100);				for(;*b;b++)a[i++]=*b;
	Nsendok=sendN21(a);
}

void	Upccid()						//上报CCID
{            
	char c[50];
	char *b=c;
	sprintf(b,"\\\"CCID\\\":\\\"%s\\\"",CCID);
	Nsendok=sendN21(b);
}

void 	Upyun_task(void *pdata)			//上传云任务 N21
{
	u8 i;u8 state=0;
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
		if(state!=(!Back))state=(!Back),Uptoaliyun();
		if(nbup)
		{
			nbup=0;
			n21_set=1;
			comClearRxFifo(COM1);
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
			n21_set=0;
			Uptoaliyun();
		}
		if(!Nsendok){n21_set=1;N21_B=0;NeoWayN21_init();conN21();n21_set=0;N21_B=1;}	
	}
}

void 	LED_task(void *pdata)			//LED任务
{	 
	u8 i=0;
	while(1)
	{
		delay_ms(200);
		if(LED_BZ)
		{
			LED0=!LED0;		
		}
		else
		{
			if(++i==5)
			i=0,LED0=!LED0;	
		}
	}
}

void	readthree()						//识别三元组
{
	u8 i;
	char hc[250];
	char *msg=hc;
	memset(hc,0,sizeof hc);
	i=0;
	if(strstr((const char *)g_RxBuf4,"m1:")[0]=='m')
	{
		msg=strstr((const char *)g_RxBuf4,"PK:");
		if(msg[1]=='K')
		{
			memset(ProductKey0,0,20);
			msg+=3;for(i=0;*msg!=',';msg++)	{ProductKey0[i++]=*msg;	if(i>=20)return ;}
		}
		msg=strstr((const char *)g_RxBuf4,"DN:");
		if(msg[1]=='N')
		{
			memset(DeviceName0,0,50);
			msg+=3;for(i=0;*msg!=',';msg++)	{DeviceName0[i++]=*msg;	if(i>=50)return ;}
		}
		msg=strstr((const char *)g_RxBuf4,"DS:");
		if(msg[1]=='S')
		{
			memset(DeviceSecret0,0,50);
			msg+=3;for(i=0;*msg;msg++)		{DeviceSecret0[i++]=*msg;if(i>=50)return ;}
		}
		sendflashthree();
		comSendBuf(COM4,g_RxBuf4,strlen((char *)g_RxBuf4));
		comClearRxFifo(COM4);
		memset(g_RxBuf4,0,UART4_RX_BUF_SIZE);
	}	
}

void 	SaveThree_task(void *pdata)		//三元组任务
{
	while(1)
	{
		delay_ms(200);
		readthree();							
	}
}
