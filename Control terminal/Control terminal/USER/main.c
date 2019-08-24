#include "main.h"

void	Uptoaliyun_wifi(void);
void	sendflash(void);
void	readflash(void);

u8	run=0;
u8	ttm=0,up=0;	//定时计数器 上传标志
u16 down=0;		//已进行时间
OS_TMR   * tmr1;           	//软件定时器1
void tmr1_callback(OS_TMR *ptmr,void *p_arg)
{
    if(++ttm==6){up=1;ttm=0;down++;}
}

void	IO_OutSet(u8 OutPort, u8 Value)	//电子阀控制
{
	switch(OutPort)
	{
		case 1:
			if(Value == 0)
			{
				PCout(7) = 0;
			}
			else
			{
				PCout(7) = 1;
			}
			break;
		
		case 2:
			if(Value == 0)
			{
				PCout(8) = 0;
			}
			else
			{
				PCout(8) = 1;
			}
			break;
			
		case 3:
			if(Value == 0)
			{
				PCout(9) = 0;
			}
			else
			{
				PCout(9) = 1;
			}
			break;
		
		case 4:
			if(Value == 0)
			{
				PAout(8) = 0;
			}
			else
			{
				PAout(8) = 1;
			}
			break;
		default:
			break;
	}
}

u8		hextobcd(u8 data)				//十六进制转BCD码
{
		return ((data/10)<<4)+(data%10);
}

void	Calibration_time()				//校准触摸屏时间
{
	u8 buf[15];
	memset(buf,0,sizeof buf);
	buf[0]=0xEE;
	buf[1]=0x81;
	buf[2]=hextobcd(STIME.second);
	buf[3]=hextobcd(STIME.minute);
	buf[4]=hextobcd(STIME.hour);
	buf[5]=hextobcd(STIME.day);
	buf[6]=hextobcd(STIME.week);
	buf[7]=hextobcd(STIME.month);
	buf[8]=hextobcd(STIME.year%100);
	buf[9]=0xFF;
	buf[10]=0xFC;
	buf[11]=0xFF;
	buf[12]=0xFF;
	comSendBuf(COM5,buf,13);
	delay_ms(100);
	comClearRxFifo(COM5);
	memset(g_RxBuf5,0,UART5_RX_BUF_SIZE);
}

void	HDMI_Init()						//触摸屏初始化
{
	char buf[22];
	queue_reset();			//清空缓存区
	delay_ms(300);			//必须等待300ms
	sprintf(buf,"%s",ssid);
	SetTextValue(2,1,(unsigned char *)buf);delay_ms(100);
	sprintf(buf,"%s",password);
	SetTextValue(2,2,(unsigned char *)buf);delay_ms(100);
	sprintf(buf,"%s",ip);
	SetTextValue(2,3,(unsigned char *)buf);delay_ms(100);
	sprintf(buf,"%s",network);
	SetTextValue(2,4,(unsigned char *)buf);delay_ms(100);
	sprintf(buf,"%s",gateway);
	SetTextValue(2,5,(unsigned char *)buf);delay_ms(100);
	sprintf(buf,"%s",dns);
	SetTextValue(2,9,(unsigned char *)buf);delay_ms(150);
	SetTextValueInt32(2,11,DHCP);delay_ms(100);
	delay_ms(200);
	SetButtonValue(1,1,IOSTATE.IO1);delay_ms(100);
	SetButtonValue(1,2,IOSTATE.IO2);delay_ms(100);
	SetButtonValue(1,3,IOSTATE.IO3);delay_ms(100);
	SetButtonValue(1,4,IOSTATE.IO4);delay_ms(100);
	memset(g_RxBuf5,0,UART5_RX_BUF_SIZE);
	comClearRxFifo(COM5);
}

void	readflashthree()				//从flash中读取三元组数据
{
	char buf[150];
	char *msg=buf;
	u8 i;
	
	STMFLASH_Read(100+FLASH_THREE_ADDR,(u16*)buf,70);
	if(buf[1]=='K')
	{
		memset(ProductKey1,0,20);
		msg=buf+4;for(i=0;*msg;msg++)		ProductKey1[i++]=*msg;
	}
	if(buf[25]=='N')
	{
		memset(DeviceName1,0,50);
		msg=buf+24+4;for(i=0;*msg;msg++)	DeviceName1[i++]=*msg;
	}
	if(buf[79]=='S')
	{
		memset(DeviceSecret1,0,50);
		msg=buf+78+4;for(i=0;*msg;msg++)	DeviceSecret1[i++]=*msg;
	}
}

void	sendflashthree(u8 group)		//向内存写入设置的三元组数据
{
	LED_BZ=1;
	STMFLASH_Write(100+FLASH_THREE_ADDR,(u16*)"PK:0",4);
	STMFLASH_Write(100+FLASH_THREE_ADDR+4,(u16*)ProductKey1,sizeof(ProductKey1));
	STMFLASH_Write(100+FLASH_THREE_ADDR+24,(u16*)"DN:0",4);
	STMFLASH_Write(100+FLASH_THREE_ADDR+28,(u16*)DeviceName1,sizeof(DeviceName1));
	STMFLASH_Write(100+FLASH_THREE_ADDR+78,(u16*)"DS:0",4);
	STMFLASH_Write(100+FLASH_THREE_ADDR+82,(u16*)DeviceSecret1,sizeof(DeviceSecret1));
	readflashthree();
	LED_BZ=0;
}

void	readflash()						//从flash读取报警设置
{
	u16 data;
	u16 buf[50];
	u8 i,k;

	memset(buf,0,sizeof buf);
	STMFLASH_Read(400+FLASH_SAVE_ADDR+200,(u16*)&data,1);
	if(data!=0xaaaa){sendflash(); return;}

	STMFLASH_Read(400+FLASH_SAVE_ADDR,buf,50);
	
	STMFLASH_Read(700+FLASH_SAVE_ADDR,buf,10);
	for(k=i=0;i<20;i+=2,k++)
	{
		ssid[i]=buf[k]>>8;
		ssid[i+1]=buf[k]&0xff;
	}

	STMFLASH_Read(720+FLASH_SAVE_ADDR,buf,10);
	for(k=i=0;i<20;i+=2,k++)
	{
		password[i]=buf[k]>>8;
		password[i+1]=buf[k]&0xff;
	}

	STMFLASH_Read(740+FLASH_SAVE_ADDR,buf,10);
	for(k=i=0;i<20;i+=2,k++)
	{
		ip[i]=buf[k]>>8;
		ip[i+1]=buf[k]&0xff;
	}

	STMFLASH_Read(760+FLASH_SAVE_ADDR,buf,10);
	for(k=i=0;i<20;i+=2,k++)
	{
		network[i]=buf[k]>>8;
		network[i+1]=buf[k]&0xff;
	}

	STMFLASH_Read(780+FLASH_SAVE_ADDR,buf,10);
	for(k=i=0;i<20;i+=2,k++)
	{
		gateway[i]=buf[k]>>8;
		gateway[i+1]=buf[k]&0xff;
	}

	STMFLASH_Read(800+FLASH_SAVE_ADDR,buf,10);
	for(k=i=0;i<20;i+=2,k++)
	{
		dns[i]=buf[k]>>8;
		dns[i+1]=buf[k]&0xff;
	}
	
	STMFLASH_Read(630+FLASH_SAVE_ADDR,(u16*)&data,1);
	if(data<0xffff)DHCP=data;
}

void	sendflash()						//存储报警设置到flash
{
	u16 bz=0xaaaa;
	u16 buf[50];
	u8 i,k;	
	
	STMFLASH_Write(400+FLASH_SAVE_ADDR,(u16*)buf,48);
	STMFLASH_Write(600+FLASH_SAVE_ADDR,(u16*)&bz,1);
	STMFLASH_Write(630+FLASH_SAVE_ADDR,(u16*)&DHCP,1);
	
	for(k=i=0;i<10;i++)
	{
		buf[i]=ssid[k++]<<8;
		buf[i]=buf[i]|ssid[k++];
	}
	
	STMFLASH_Write(700+FLASH_SAVE_ADDR,(u16*)buf,10);
	delay_ms(200);

	for(k=i=0;i<10;i++)
	{
		buf[i]=password[k++]<<8;
		buf[i]=buf[i]|password[k++];
	}
	
	STMFLASH_Write(720+FLASH_SAVE_ADDR,(u16*)buf,10);
	delay_ms(200);
	
	for(k=i=0;i<10;i++)
	{
		buf[i]=ip[k++]<<8;
		buf[i]=buf[i]|ip[k++];
	}
	
	STMFLASH_Write(740+FLASH_SAVE_ADDR,(u16*)buf,10);
	delay_ms(200);

	for(k=i=0;i<10;i++)
	{
		buf[i]=network[k++]<<8;
		buf[i]=buf[i]|network[k++];
	}
	
	STMFLASH_Write(760+FLASH_SAVE_ADDR,(u16*)buf,10);
	delay_ms(200);

	for(k=i=0;i<10;i++)
	{
		buf[i]=gateway[k++]<<8;
		buf[i]=buf[i]|gateway[k++];
	}
	
	STMFLASH_Write(780+FLASH_SAVE_ADDR,(u16*)buf,10);
	delay_ms(200);

	for(k=i=0;i<10;i++)
	{
		buf[i]=dns[k++]<<8;
		buf[i]=buf[i]|dns[k++];
	}
	
	STMFLASH_Write(800+FLASH_SAVE_ADDR,(u16*)buf,10);
	delay_ms(200);	
	
	readflash();
}

void	init()							//系统初始化
{
	SystemInit();
	delay_init();	    	 			//延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	LED_Init();		  					//初始化与LED连接的硬件接口	
	bsp_InitUart();
	comClearRxFifo(COM4);
	memset(g_RxBuf4,0,UART4_RX_BUF_SIZE);
	delay_ms(1000);
	delay_ms(1000);
//	sendflash();
	readflash();
	HDMI_Init();
	readflashthree();					//读取内存里写入的三元组数据
	SetScreen(0);
	StopTimer(4,2);
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
 	OSTaskCreate(LED_task,(void *)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);
	OSTaskCreate(SaveThree_task,(void *)0,(OS_STK*)&SaveThree_TASK_STK[SaveThree_STK_SIZE-1],SaveThree_TASK_PRIO);
	OSTaskCreate(UpyunWF_task,(void *)0,(OS_STK*)&UpyunWF_TASK_STK[UpyunWF_STK_SIZE-1],UpyunWF_TASK_PRIO);
	OSTaskCreate(HDMI_task,(void *)0,(OS_STK*)&HDMI_TASK_STK[HDMI_STK_SIZE-1],HDMI_TASK_PRIO);
	OSTaskCreate(IO_task,(void *)0,(OS_STK*)&IO_TASK_STK[IO_STK_SIZE-1],IO_TASK_PRIO);
	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
}

void 	LED_task(void *pdata)			//LED运行状态指示灯任务
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
			case 0:
				EmwLED1=0;
				EmwLED2=0;
				break;
			case 1:
				EmwLED1=!EmwLED1;
				break;
			case 2:
				EmwLED1=1;
				EmwLED2=!EmwLED2;
				break;
			case 3:
				EmwLED2=0;
				break;
		}
	}
}

void 	HDMI_task(void *pdata)			//触摸屏监控任务
{	  
	static qsize  size = 0;
	OS_CPU_SR cpu_sr=0;
	while(1)
	{
		delay_ms(200);
		size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE); //从缓冲区中获取一条指令        
		if(size>0)//接收到指令
		{
			ProcessMessage((PCTRL_MSG)cmd_buffer, size);//指令处理
			memset(g_RxBuf5,0,UART5_RX_BUF_SIZE);
			comClearRxFifo(COM5);
		}

		if(SaveButton==1)			//设置按钮按下，获取设置的值
		{
			SaveButton=0;
			OS_ENTER_CRITICAL();
			sendflash();
			OS_EXIT_CRITICAL();
			delay_ms(200);
		}
		if(StartButton==1)
		{
			StartButton=0;
			if(run)SetScreen(4);
			else SetScreen(1);
		}
		if(NextButton==1)
		{
			NextButton=0;
			if(IOSTATE.IO1|IOSTATE.IO2|IOSTATE.IO3|IOSTATE.IO4)SetScreen(3);
		}
	}
}

void	Uptoaliyun_wifi()				//上传传感器数据到阿里云 WIFI
{
	char a[250];
	char c[250];
	char *b=c;
	u8 i;i=0;b=c;
	memset(a,0,sizeof a);
	if(Remaining_time)delay_ms(1000);
	sprintf(b,"\"Solenoid1\":%d,",IOSTATE.IO1);		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"Solenoid2\":%d,",IOSTATE.IO2);		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"Solenoid3\":%d,",IOSTATE.IO3);		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"Solenoid4\":%d,",IOSTATE.IO4);		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"Count_down\":%d",Count_down-down);	for(;*b;b++)a[i++]=*b;
	sendEmw(a,0);
}


void	readthree()						//从COM3 485串口读取是否设置了三元组数据
{
	u8 len,i;
	char hc[250];
	char *msg=hc;
	u8 buf[250];
	memset(buf,0,sizeof buf);
	memset(hc,0,sizeof hc);
	len=0;i=0;
	len=COM4GetBuf(buf,200);
	if(len>50)
	{
		if(strstr((const char *)buf,"m1:")[0]=='m')
		{
			msg=strstr((const char *)buf,"PK:");
			if(msg[1]=='K')
			{
				memset(ProductKey1,0,20);
				msg+=3;for(i=0;*msg!=',';msg++)	{ProductKey1[i++]=*msg;	if(i>=20)return ;}
			}
			msg=strstr((const char *)buf,"DN:");
			if(msg[1]=='N')
			{
				memset(DeviceName1,0,50);
				msg+=3;for(i=0;*msg!=',';msg++)	{DeviceName1[i++]=*msg;	if(i>=50)return ;}
			}
			msg=strstr((const char *)buf,"DS:");
			if(msg[1]=='S')
			{
				memset(DeviceSecret1,0,50);
				msg+=3;for(i=0;*msg;msg++)		{DeviceSecret1[i++]=*msg;if(i>=50)return ;}
			}
			len=2;
		}
		else return;	
		sendflashthree(len);
		comSendBuf(COM4,buf,strlen((char *)buf));
		comClearRxFifo(COM4);
		memset(g_RxBuf4,0,UART4_RX_BUF_SIZE);
	}
}

void	readset()						//读取阿里下发的数据
{
	u8 len;
	u8 buf[250];
	char c[100];
	char *msg=c;
	if(emw_set)return;
	len=COM1GetBuf(buf,200);
	if(len>40)
	{
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"RECV")[0]!='R'){memset(buf,0,sizeof buf);memset(c,0,sizeof c);comClearRxFifo(COM1);return;}
		msg=strstr((const char *)buf,"Solenoid1:");	if(msg[0]=='S')IOSTATE.IO1=msg[10]-0x30;
		msg=strstr((const char *)buf,"Solenoid2:");	if(msg[0]=='S')IOSTATE.IO2=msg[10]-0x30;
		msg=strstr((const char *)buf,"Solenoid3:");	if(msg[0]=='S')IOSTATE.IO3=msg[10]-0x30;
		msg=strstr((const char *)buf,"Solenoid4:");	if(msg[0]=='S')IOSTATE.IO4=msg[10]-0x30;
		msg=strstr((const char *)buf,"Count_down:");if(msg[0]=='C')Count_down=(msg[11]-0x30)*100+(msg[12]-0x30)*10+(msg[13]-0x30);

		if(Count_down==0){StopButton=1;goto cc;}
		SetButtonValue(1,1,IOSTATE.IO1);delay_ms(100);
		SetButtonValue(1,2,IOSTATE.IO2);delay_ms(100);
		SetButtonValue(1,3,IOSTATE.IO3);delay_ms(100);
		SetButtonValue(1,4,IOSTATE.IO4);delay_ms(100);
		SetTextValueInt32(3,1,Count_down);delay_ms(100);
		RunButton=1;
		
cc:		comClearRxFifo(COM1);
		memset(buf,0,sizeof buf);
		memset(c,0,sizeof c);
	}
}

void 	UpyunWF_task(void *pdata)		//上传云任务 WIFI
{
	u8 buf[50];u8 error=0;
	do	
	{
		Emw3060_init();
	}while(!Emw3060_con());
	Emw_B=1;emw_set=0;
	Calibration_time();
	while(1)
	{
		delay_ms(200);
		readset();						//读取阿里云下发数据
//		comClearRxFifo(COM1);
		printf_num=1;
		printf("AT+WJAPS\r");
		delay_ms(50);
		COM1GetBuf(buf,45);
		if(strstr((const char *)buf,"STATION_UP")[0]!='S')
		{
			if(++error==10)
			{
				emw_set=1;Emw_B=0;
				do	
				{
					Emw3060_init();
				}while(!Emw3060_con());
				Emw_B=1;emw_set=0;
			
			}
			continue;
		}
		else error=0;
		if(up){Uptoaliyun_wifi();up=0;}
	}
}

void 	SaveThree_task(void *pdata)		//三元组存储任务
{
	while(1)
	{
		delay_ms(200);
		readthree();							//判断是否扫码枪发送三元组数据
	}
}

void 	IO_task(void *pdata)			//IO控制任务
{
	char a[60];
	char c[60];
	char *b=c;
	u8 err;
	u8 i;i=0;
	while(1)
	{
		delay_ms(200);
		if(RunButton)					//判断启动按钮按下
		{
			GetControlValue(3,1);
			delay_ms(500);
			if(Count_down==0)continue;
			SetScreen(4);
			run=1;
			
			IO_OutSet(1,IOSTATE.IO1);	//IO设置
			IO_OutSet(2,IOSTATE.IO2);
			IO_OutSet(3,IOSTATE.IO3);
			IO_OutSet(4,IOSTATE.IO4);
			
			memset(a,0,sizeof a);		//触摸屏显示打开分区
			memset(c,0,sizeof c);
			b=c;i=0;
			sprintf(b,"打开分区：");					for(;*b;b++)a[i++]=*b;
			if(IOSTATE.IO1)sprintf(b," 分区一");		for(;*b;b++)a[i++]=*b;
			if(IOSTATE.IO2)sprintf(b," 分区二");		for(;*b;b++)a[i++]=*b;
			if(IOSTATE.IO3)sprintf(b," 分区三");		for(;*b;b++)a[i++]=*b;
			if(IOSTATE.IO4)sprintf(b," 分区四");		for(;*b;b++)a[i++]=*b;
			SetTextValue(4,5,(unsigned char *)a);		delay_ms(200);
			
			SeTimer(4,2,Count_down*60);	//设置触摸屏倒计时控件
			delay_ms(100);
			StartTimer(4,2);			//开启倒计时
			delay_ms(100);
			tmr1=OSTmrCreate(0,100,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr1_callback,0,(u8 *)tmr1,&err);//10s执行一次
			OSTmrStart(tmr1,&err);//启动软件定时器1
			Remaining_time=0;
			up=1;
			while(1)
			{
				delay_ms(200);
				if(Remaining_time|StopButton)	//判断时间到或者停止按钮按下
				{
					OSTmrStop(tmr1,OS_TMR_OPT_NONE,tmr1_callback,&err);	//关闭软件定时器
					SetScreen(0);										//切换首页屏幕
					IO_OutSet(1,0);IO_OutSet(2,0);						//复位相关设置
					IO_OutSet(3,0);IO_OutSet(4,0);
					IOSTATE.IO1=IOSTATE.IO2=IOSTATE.IO3=IOSTATE.IO4=0;
					StopTimer(4,2);										//停止触摸屏定时器
					SetButtonValue(1,1,IOSTATE.IO1);delay_ms(100);		//复位触摸屏电磁阀显示
					SetButtonValue(1,2,IOSTATE.IO2);delay_ms(100);
					SetButtonValue(1,3,IOSTATE.IO3);delay_ms(100);
					SetButtonValue(1,4,IOSTATE.IO4);delay_ms(100);
					down=Count_down=0;
					up=1;						//上传最后一帧消息
					break;
				}
			}
			run=RunButton=StopButton=Remaining_time=0;
		}
	}
}
