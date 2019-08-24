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
	FlashReadMem(Logmem.mem);	//读取日志地址
	readflashthree();			//读取三元组
	ReadNet=1;					//读取网络参数
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
	OSTaskCreate(SaveThree_task,(void *)0,(OS_STK*)&SaveThree_TASK_STK[SaveThree_STK_SIZE-1],SaveThree_TASK_PRIO);	//三元组存储任务
	OSTaskCreate(UpyunWF_task,(void *)0,(OS_STK*)&UpyunWF_TASK_STK[UpyunWF_STK_SIZE-1],UpyunWF_TASK_PRIO);			//WIFI任务
	OSTaskCreate(HDMI_task,(void *)0,(OS_STK*)&HDMI_TASK_STK[HDMI_STK_SIZE-1],HDMI_TASK_PRIO);						//触摸屏任务
	OSTaskCreate(IO_task,(void *)0,(OS_STK*)&IO_TASK_STK[IO_STK_SIZE-1],IO_TASK_PRIO);								//IO任务
	OSTaskCreate(Model_task,(void *)0,(OS_STK*)&Model_TASK_STK[Model_STK_SIZE-1],Model_TASK_PRIO);					//模式识别 与 手动模式 任务
	OSTaskCreate(Work_task,(void *)0,(OS_STK*)&Work_TASK_STK[Work_STK_SIZE-1],Work_TASK_PRIO);						//灌溉任务
	OSTaskCreate(Plan_task,(void *)0,(OS_STK*)&Plan_TASK_STK[Plan_STK_SIZE-1],Plan_TASK_PRIO);						//计划任务
	OSTaskCreate(Collection_task,(void *)0,(OS_STK*)&Collection_TASK_STK[Collection_STK_SIZE-1],Collection_TASK_PRIO);//采集传感器任务
	OSTaskSuspend(START_TASK_PRIO);	
	OS_EXIT_CRITICAL();				
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
	u16 LogAddr;
	u8 i;
	while(1)
	{
		delay_ms(200);
		HDMI_Check_Button();										//读取屏幕按键状态
		if(Irrsign==1)HDMI_Check_Irrigation_time(),Irrsign=0;		//读取灌溉时常
		if(Remsign==1)HDMI_Set_Remaining_time(Remaining),Remsign=0;	//设置倒计时时间
		if(Cursing==1)HDMI_Set_Current_Partition(Current),Cursing=0;//设置灌溉分区
		if(ReadNet==1)												//读取屏幕网络参数
		{
			HDMI_Check_Net(MCGS_SSID,ssid);
			HDMI_Check_Net(MCGS_PASSWORD,password);
			HDMI_Check_Net(MCGS_IP,ip);
			HDMI_Check_Net(MCGS_NETWORK,network);
			HDMI_Check_Net(MCGS_GATEWAY,gateway);
			HDMI_Check_Net(MCGS_DNS,dns);
			HDMI_Check_Acquisition_Frequency();
			ReadNet=0;
		}
		if(TimeButton)SysTime=1,HDMI_Set_Partition(MCGS_Partition&0x7F);//申请时间同步
		if(SysTime==2)HDMI_SetSysTime(),SysTime=0;						//更新时间同步
		if(Err)HDMI_Set_Error(),Err=0;									//弹出错误
		if(GetTime)HDMI_Check_SysTime(),GetTime=0;						//获取触摸屏时间
		if(ReadPlan)HDMI_Check_Plan(PlanAddr),ReadPlan=0;				//获取计划内容
		
		if(SetRun)														//设置屏幕运行 用于计划任务
		{
			HDMI_Set_Partition(HC_Partition);
			HDMI_Set_Button(HC_IrrMode);
			SetRun=0;
		}
		
		if(LogButton)					//日志查询按钮按下
		{
			HDMI_Check_LogPage();		//查询当前页码
			LogAddr=(LogPage-1)*200;	//指向对应存储地址
			for(i=0;i<10;i++)			//显示10条日志
			{
				while(Logwait)delay_ms(200);						//等待外部flash操作完毕
				Logwait=1;
				FlashReadLog(&Log.StartTime[0],LogAddr+i*20);		//读取日志
				Logwait=0;
				HDMI_Set_Log((LogPage-1)*10+i+1,0x190+(i*17));		//显示日志
			}
		}
	}
}

void 	UpyunWF_task(void *pdata)		//WIFI任务
{
	u8 buf[50];u8 error=0,tt=0;
	delay_ms(1000);
	while(!WifiButton)delay_ms(200);	//WIFI开关打开
	while(ReadNet==1)delay_ms(200);		//网络参数读取完毕
	do	
	{
		Emw3060_init();
	}while(!Emw3060_con());
	Emw_B=1;emw_set=0;
	Uptoaliyun_wifi_Env(sensor[0],0);
	while(1)
	{
		delay_ms(200);
		if(WifiButton)
		{
			if(error==255)				//判断是否是重新启动WIFI
			{
				do	
				{
					Emw3060_init();
				}while(!Emw3060_con());
				error=0;
			}
			readset();					//读取云下发的指令
			if(tt!=ttm)					//10秒检测一次WIFI是否正常
			{
				tt=ttm;
				printf_num=1;
				printf("AT+WJAPS\r");
				delay_ms(50);
				COM1GetBuf(buf,45);
				if(strstr((const char *)buf,"STATION_UP")[0]!='S')
				{
					if(++error==10)		//错误重启
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
			}
			if(upenv>=up_time){Uptoaliyun_wifi_Env(sensor[0],0);upenv=0;}	//传感器上传
			if(up_state)Uptoaliyun_wifi_State(),up_state=0;					//设备状态上传
			if(SysTime==1)ReadNetTime(),SysTime=2;							//同步网络时间
		}
		else	if(error!=255){Emw_B=0;Emw3060_init();error=255;Emwled=0;}	//关闭WIFI连接
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
		OSTaskSuspend(Collection_TASK_PRIO);
		msg=strstr((const char *)g_RxBuf4,"PK:");
		if(msg[1]=='K')
		{
			memset(ProductKey1,0,20);
			msg+=3;for(i=0;*msg!=',';msg++)	{ProductKey1[i++]=*msg;	if(i>=20)return ;}
		}
		msg=strstr((const char *)g_RxBuf4,"DN:");
		if(msg[1]=='N')
		{
			memset(DeviceName1,0,50);
			msg+=3;for(i=0;*msg!=',';msg++)	{DeviceName1[i++]=*msg;	if(i>=50)return ;}
		}
		msg=strstr((const char *)g_RxBuf4,"DS:");
		if(msg[1]=='S')
		{
			memset(DeviceSecret1,0,50);
			msg+=3;for(i=0;*msg;msg++)		{DeviceSecret1[i++]=*msg;if(i>=50)return ;}
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

void 	IO_task(void *pdata)			//IO控制任务
{
	while(1)
	{
		delay_ms(200);
		CheckIO8();								//读取IO8状态
		if(IO8Set)SetIO8(IO8SWITCH),IO8Set=0;	//设置IO8输出
	}
}

void 	Collection_task(void *pdata)	//采集任务
{
	while(1)
	{
		delay_ms(2000);
		ReadValue();					//读取传感器数据
	}
}

void 	Model_task(void *pdata)			//模式识别任务	与	手动功能
{
	u8	State=0;
	u8	state=0;
	u8	i;
	SysLog HLog;
	while(1)
	{
		delay_ms(200);
		if((!FerSwitch)|StirSwitch1|StirSwitch2|SolenoidSwitch1|SolenoidSwitch2)MODEL=ManualModel,ChoMode=0;	//任何开关人为干预则为手动模式
		else if(MODEL==ManualModel)MODEL=localModel;		//所有开关都在自动档时  若之前在手动模式则改为本地模式
		if(MODEL==ManualModel)
		{
			if((State!=IO8STATE)|(state!=FerState))
			{
				HLog.Irrtime=0;								//设置计划时间为0
				TimePopup=1;								//不弹出时间同步弹窗
				for(i=0;i<10;i++)delay_ms(1000);			//等待10秒操作完设备后继续
				State=IO8STATE;								//记忆IO状态
				state=FerState;
				up_state=1;									//上传设备状态
				
				if(Emw_B)									//读取当前时间  网络或触摸屏
				{
					SysTime=1;
					while(SysTime)delay_ms(200);
					HLog.StartTime[0]=STIME.year%100;
					HLog.StartTime[1]=STIME.month;
					HLog.StartTime[2]=STIME.day;
					HLog.StartTime[3]=STIME.hour;
					HLog.StartTime[4]=STIME.minute;
				}
				else	
				{
					GetTime=1;while(GetTime)delay_ms(200);
					HLog.StartTime[0]=STIME.year%100;
					HLog.StartTime[1]=STIME.month;
					HLog.StartTime[2]=STIME.day;
					HLog.StartTime[3]=STIME.hour;
					HLog.StartTime[4]=STIME.minute;
				}
				
				HLog.StartMode=0;							//记录日志	启动方式
				HLog.IrrMode=FerState;						//记录日志	灌溉方式
				if(ChoPartition1)							//记录日志	灌溉分区
				{
					if(ChoPartition2)Log.Partition=3;
					else HLog.Partition=1;
				}
				else HLog.Partition=2;
				htime=0;									//清零手动计数器
				while((State==IO8STATE)&(state==FerState))delay_ms(200);	//当检测到IO状态改变后 结束本次日志记录
				HLog.Irrtime=htime+1;						//记录日志	灌溉时长
				HLog.RemTime=0;								//记录日志	用于显示时间计算	
				if(Emw_B)									//读取当前时间  网络或触摸屏
				{
					SysTime=1;
					while(SysTime)delay_ms(200);
					HLog.StopTime[0]=STIME.year%100;
					HLog.StopTime[1]=STIME.month;
					HLog.StopTime[2]=STIME.day;
					HLog.StopTime[3]=STIME.hour;
					HLog.StopTime[4]=STIME.minute;
				}
				else	
				{
					GetTime=1;while(GetTime)delay_ms(200);
					HLog.StopTime[0]=STIME.year%100;
					HLog.StopTime[1]=STIME.month;
					HLog.StopTime[2]=STIME.day;
					HLog.StopTime[3]=STIME.hour;
					HLog.StopTime[4]=STIME.minute;
				}
				HLog.StopMode=0;							//记录日志	停止模式
				while(Logwait)delay_ms(200);				//等待外部flash操作完成
				Logwait=1;OSTaskSuspend(HDMI_TASK_PRIO);
				FlashWriteLog(&HLog.StartTime[0],Logmem.mem32,17);	//日志写入flash
				if(Logmem.mem32==20*100)Logmem.mem32=0;				//日志存储地址增加
				Logmem.mem32+=20;FlashWriteMem(Logmem.mem);			//日志地址增加
				Logwait=0;OSTaskResume(HDMI_TASK_PRIO);
				up_state=1;									//上传设备状态
				TimePopup=1;
			}
		}
	}
}

void 	Plan_task(void *pdata)			//计划任务
{
	u8	addr=0;
	while(1)
	{
		delay_ms(200);
		if(!PlanButton)continue;		//计划开关是否打开
		if(!TimePopup)					//当前状态空闲
		{
			if(!Planing)				//没有任务在执行
			{
				GetTime=1;while(GetTime)delay_ms(200);		//获取触摸屏时间
				PlanAddr=0x23B+addr;
				ReadPlan=1;while(ReadPlan)delay_ms(200);	//读取计划任务
				if((STIME.month==Plan.month)&(STIME.day==Plan.day)&(STIME.hour==Plan.hour)&(STIME.minute==Plan.minute)&(Plan.State==0))	//是否需要执行任务
				{
					if(Plan.Irrtime<21)goto next;			//计划设置时间小于21分钟不执行
					Planing=1;								//装载相关运行参数
					IrrTime=Plan.Irrtime;					//设置任务时间
					HC_IrrMode=MCGS_Button&0XFE;			//设置触摸屏相关寄存器状态
					HC_Partition=Plan.Partition;
					HC_IrrMode|=Plan.IrrMode|0x28;
					SetRun=1;
					while(SetRun)delay_ms(200);				//等待触摸屏执行结束
					Net=0;
					ChoMode=3;								//模式为计划任务模式
					while(Planing)delay_ms(200);			//任务执行结束
					OSTaskSuspend(HDMI_TASK_PRIO);		delay_ms(100);
					HDMI_Set_Plan(0x242+addr);			delay_ms(100);
					OSTaskResume(HDMI_TASK_PRIO);
				}
next:			addr+=9;				//查找下一条任务
				if(addr>=99)addr=0;
			}
		}
	}
}

void	Judge_Partition()				//排序分区进入待灌溉队列
{
	u8 i;
	i=0;
	memset(waitPartition,0,sizeof(waitPartition));	//清除待灌溉分区队列
	if(ChoPartition1)waitPartition[i++]=1;			//装载待灌溉分区
	if(ChoPartition2)waitPartition[i++]=2;
	if(ChoPartition3)waitPartition[i++]=3;
	if(ChoPartition4)waitPartition[i++]=4;
	if(ChoPartition5)waitPartition[i++]=5;
	if(ChoPartition6)waitPartition[i]=6;
}

void 	Work_task(void *pdata)			//灌溉任务
{
	u8 ATPartition,first,error;
	while(1)
	{
	 	delay_ms(200);
		first=0;								//肥料桶是否开启	0未开启 1已开启
		ATPartition=0;							//当前分区计数器清零
		if(MODEL==ManualModel)continue;			//手动模式结束循环
		if(RunState)							//启动按钮按下
		{
			TimePopup=1;						//禁止时间同步弹窗
			if((ChoMode!=2)&(ChoMode!=3))ChoMode=1;
			if(Net==0&ChoMode!=3)				//根据是否为网络启动  判断是否需要读屏参
			{
				if(Irrsign==0)Irrsign=1;		//读取设置的时间
				while(Irrsign==1)delay_ms(20);
			}
			if(Emw_B)							//读取当前时间  网络或触摸屏
			{
				SysTime=1;
				while(SysTime)delay_ms(200);
				Log.StartTime[0]=STIME.year%100;
				Log.StartTime[1]=STIME.month;
				Log.StartTime[2]=STIME.day;
				Log.StartTime[3]=STIME.hour;
				Log.StartTime[4]=STIME.minute;
			}
			else	
			{
				GetTime=1;while(GetTime)delay_ms(200);
				Log.StartTime[0]=STIME.year%100;
				Log.StartTime[1]=STIME.month;
				Log.StartTime[2]=STIME.day;
				Log.StartTime[3]=STIME.hour;
				Log.StartTime[4]=STIME.minute;
			}
			
			Log.StartMode=ChoMode;				//记录日志	启动方式
			Log.IrrMode=IrrMethod;				//记录日志	灌溉方式
			if(ChoPartition1)					//记录日志	灌溉分区
			{
				if(ChoPartition2)Log.Partition=3;
				else Log.Partition=1;
			}
			else Log.Partition=2;
			Log.Irrtime=IrrTime;				//记录日志	灌溉时常
			while(Logwait)delay_ms(200);
			Logwait=1;OSTaskSuspend(HDMI_TASK_PRIO);
			FlashWriteLog(&Log.StartTime[0],Logmem.mem32,sizeof(Log));	//日志写入flash
			Logwait=0;OSTaskResume(HDMI_TASK_PRIO);
				//if(IrrMethod)IrrTime+=20;		//判断灌溉模式
			Remaining=IrrTime;					//更新倒计时
			Remsign=1;							//显示倒计时时间
			Judge_Partition();					//排序需灌溉的分区
			rema=1;								//打开倒计时计时器
			if(waitPartition[ATPartition])		//判断显示当前分区
			{
				Current=waitPartition[ATPartition];
				Cursing=1;
			}
			if(waitPartition[ATPartition]==1)Solenoid1Open;	//打开一分区电磁阀
			if(waitPartition[ATPartition]==2)Solenoid2Open;	//打开二分区电磁阀
			error=0;
			OpenLED;							//打开LED
			while(1)							//异常检查
			{
				IO8Set=1;delay_ms(200);
				if(waitPartition[ATPartition]==1)
				{
					if(SolenoidState1)break;
					if(++error==10)Error=4,Err=1;
				}
				if(waitPartition[ATPartition]==2)
				{
					if(SolenoidState1)break;
					if(++error==10)Error=5,Err=1;
				}
				if(error==10)break;
			}
			if(error==10)goto stop;				//设备异常结束运行
			up_state=1;
			while(1)							//运行中  需要更改当前轮灌区 剩余时间
			{
				delay_ms(200);
				if(MODEL==ManualModel)goto stop;
				if(IrrMethod)
				{
					if(Remaining<(IrrTime-10))	//启动吸肥并上传状态
					{
						if(first==0)first=up_state=1;
						FerRun;					//开始吸肥
						Stir1Open;				//肥料桶1搅拌
						Stir2Open;				//肥料桶2搅拌
						error=0;
						while(1)				//异常检测
						{
							IO8Set=1;delay_ms(200);
							if(FerState&StirState1&StirState2)break;
							if(++error==10)
							{
								if(FerState)Error=1;
								if(StirState1)Error=2;
								if(StirState2)Error=3;
								Err=1;
								goto stop;		//设备异常结束运行
							}	
						}
					}
					if(Remaining<10)			//停止吸肥并上传状态
					{
						if(first==1)first=2,up_state=1;
						FerStop;				//停止吸肥
						Stir1Close;				//肥料桶1停止搅拌
						Stir2Close;				//肥料桶2停止搅拌
						error=0;
						while(1)				//异常检测
						{
							IO8Set=1;delay_ms(200);
							if((FerState|StirState1|StirState2)==0)break;
							if(++error==10)
							{
								if(FerState)Error=1;
								if(StirState1)Error=2;
								if(StirState2)Error=3;
								Err=1;
								goto stop;		//设备异常结束运行
							}	
						}
					}
				}
				
				if(Remaining==0)				//当前分区结束重载相关参数
				{
					if(waitPartition[ATPartition]==1)Solenoid1Close;	//关闭一分区电磁阀
					if(waitPartition[ATPartition]==2)Solenoid2Close;	//关闭二分区电磁阀
					while(IO8Set)delay_ms(100);IO8Set=1;				//确保更改继电器状态
					if(waitPartition[++ATPartition])					//判断下一分区编号
					{
						Remaining=IrrTime;								//更新倒计时
						Remsign=1;										//触摸屏更新倒计时显示
						Current=waitPartition[ATPartition];				//更新分区显示
						Cursing=1;										//触摸屏更新分区显示
						if(waitPartition[ATPartition]==2)Solenoid2Open;	//打开二分区电磁阀
						while(1)										//异常检查
						{
							IO8Set=1;delay_ms(200);
							if(waitPartition[ATPartition]==1)
							{
								if(++error==10)Error=4,Err=1;
								if(SolenoidState1)break;
							}
							if(waitPartition[ATPartition]==2)
							{
								if(++error==10)Error=5,Err=1;
								if(SolenoidState1)break;
							}
							if(error==10)break;
						}
						if(error==10)goto stop;							//设备异常结束运行
						first=0;										//肥料桶更改为未开启
					}
					else
					{
						rema=0;					//关倒计时器
						break;
					}
					delay_ms(200);				//上传设备状态
					up_state=1;
				}

				if(StartButton==0)				//停止按钮按下
				{
stop:				rema=0;
					break;
				}
			}
			while(Err)delay_ms(200);			//判断错误弹窗是否弹出
			Current=0;							//当前灌溉分区清零
			Solenoid1Close;						//关闭一分区电磁阀
			Solenoid2Close;						//关闭二分区电磁阀
			FerStop;							//停止吸肥
			Stir1Close;							//肥料桶1停止搅拌
			Stir2Close;							//肥料桶2停止搅拌
			IO8Set=1;							//更新IO8输出状态
			while(IO8Set)delay_ms(100);			//确保更改继电器状态
			CloseLED;							//关闭LED
			OSTaskSuspend(HDMI_TASK_PRIO);		//停止触摸屏 ‘运行中’标志
			HDMI_Set_Button(MCGS_Button&0xD7);
			OSTaskResume(HDMI_TASK_PRIO);
			while(StartButton)delay_ms(200);	//清零启动按钮
			delay_ms(200);
			up_state=1;							//上传设备状态
			if(Emw_B)							//读取当前时间  网络或触摸屏
			{
				SysTime=1;
				while(SysTime)delay_ms(200);
				Log.StopTime[0]=STIME.year%100;
				Log.StopTime[1]=STIME.month;
				Log.StopTime[2]=STIME.day;
				Log.StopTime[3]=STIME.hour;
				Log.StopTime[4]=STIME.minute;
			}
			else	
			{
				GetTime=1;while(GetTime)delay_ms(200);
				Log.StopTime[0]=STIME.year%100;
				Log.StopTime[1]=STIME.month;
				Log.StopTime[2]=STIME.day;
				Log.StopTime[3]=STIME.hour;
				Log.StopTime[4]=STIME.minute;
			}
			Log.StopMode=ChoMode;
			Log.RemTime=Remaining;
			while(Logwait)delay_ms(200);
			Logwait=1;OSTaskSuspend(HDMI_TASK_PRIO);
			FlashWriteLog(&Log.StopTime[0],Logmem.mem32+10,8);	//日志写入flash
			if(Logmem.mem32==20*100)Logmem.mem32=0;				//日志存储地址增加
			Logmem.mem32+=20;FlashWriteMem(Logmem.mem);
			Logwait=0;OSTaskResume(HDMI_TASK_PRIO);
			Planing=0;	//任务结束标志
			ChoMode=1;	//切回本地模式
			TimePopup=0;//设为空闲状态
		}
	}
}
