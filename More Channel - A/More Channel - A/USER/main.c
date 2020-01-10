/*485地址 1专用桶 2公用桶 3总水表 9压力表 10流量阀*/
#include "main.h"
#include "MCGS.h"

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
//	readflashthree();			//读取三元组
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
//	OSTaskCreate(SaveThree_task,(void *)0,(OS_STK*)&SaveThree_TASK_STK[SaveThree_STK_SIZE-1],SaveThree_TASK_PRIO);	//三元组存储任务
	OSTaskCreate(Upyun_task,(void *)0,(OS_STK*)&Upyun_TASK_STK[Upyun_STK_SIZE-1],Upyun_TASK_PRIO);					//WIFI任务
	OSTaskCreate(HDMI_task,(void *)0,(OS_STK*)&HDMI_TASK_STK[HDMI_STK_SIZE-1],HDMI_TASK_PRIO);						//触摸屏任务
	OSTaskCreate(Work_task,(void *)0,(OS_STK*)&Work_TASK_STK[Work_STK_SIZE-1],Work_TASK_PRIO);						//灌溉任务
	OSTaskCreate(Water_task,(void *)0,(OS_STK*)&Water_TASK_STK[Water_STK_SIZE-1],Water_TASK_PRIO);					//水表任务
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
	}
}

void 	Water_task(void *pdata)			//水表任务
{	 
	u32	w1;
	u8 ok;
	u8 i=0;
	while(1)
	{
		while(!RunState)ReadWater(),delay_ms(200);
		watertime=0;
		w1=thiswater1=water1;
		thiswater2=water2;
		while(RunState)
		{
			delay_ms(200);
			ok=ReadWater();
			Actual_Water=water0-thiswater0;	//实际用水量
			Actual_Fer=water1-thiswater1;		//实际用肥量
			Actual_FerP=water2-thiswater2;
			if(++i==10)
			{
				i=0;
				if((!ok)|(water1==w1))continue;
				if((water1flow-Mathflow)>20)
				{
					NeedFlow-=10;
					ChangeFlow();
				}
				else if((Mathflow-water1flow)>10)
				{
					NeedFlow+=10;
					ChangeFlow();
				}
				w1=water1;
			}
		}
	}
}

void 	HDMI_task(void *pdata)			//触摸屏监控任务
{	  
	u16 LogAddr;
	u8 i,so1;
	u8 HouseNum=0;
	GetTime=1;
	while(1)
	{
		delay_ms(200);
		HDMI_Check_Button();		//读取屏幕按键状态0
		if(HDMI_READ_START)			//启动按下读取用户运行参数
		{
			if(!NET)
			HDMI_Read_Greenhouse();
			HDMI_Read_Fer();
			HDMI_READ_START=0;
		}
		if(HDMI_SET_STOP)			//系统停止触摸屏相关
		{
			HDMI_Set_Button(MCGS_Button&(~RunState));
			HDMI_Check_Button();
			HDMI_SET_STOP=0;
		}
		if(RunState)				//系统运行中 显示阶段 用水量 用肥量 倒计时  压力
		{
			HDMI_Set_Stage();
			HDMI_Set_Water_Fer();
		}
		if(FerSButton)HDMI_Set_Button(MCGS_Button&(~FerSButton)),HDMI_Read_Fer();	//肥料桶设置保存
		if(HouseRButton)			//温室配置读取
		{
			HDMI_Set_Button(MCGS_Button&(~HouseRButton));
			HDMI_Read_SetHouseNum(&HouseNum);
			FlashReadFer(&Run_SetMessage.Mu,HouseNum*10);
			HDMI_Set_HouseSet();	//显示温室配置内容
		}
		if(HouseSButton)HDMI_Set_Button(MCGS_Button&(~HouseSButton)),HDMI_Read_HouseSet();	//温室配置保存
		while(CechkSelf)
		{
			HDMI_Check_Button();		//读取屏幕按键状态
			HDMI_Check_Check();			//读取自检按键
			HDMI_Check_Checkvalue();	//读取自检设置值
			if(CheckFerM){FerRun;}else{FerStop;}
			if(CheckSolenoid){so1=1;FlashReadFer(&Run_SetMessage.Mu,checknum*10);OpenPartition();}else if(so1){so1=0;FlashReadFer(&Run_SetMessage.Mu,checknum*10);ClosePartition();}
			if(CheckFlow){NeedFlow=checkflow;ChangeFlow();}
			Run_SetMessage.SpecialFer=checkfer;OpenFer();
			if(!CechkSelf)CloseFer(),FerStop,ClosePartition();
		}
		if(GetTime)HDMI_Check_SysTime(),GetTime=0;						//获取触摸屏时间
		if(LogButton)					//日志查询按钮按下
		{
			HDMI_Check_LogPage();		//查询当前页码
			LogAddr=LogPage*200;		//指向对应存储地址
			for(i=0;i<10;i++)			//显示10条日志
			{
				HDMI_Check_Button();
				if(Details)goto De;
				FlashReadLog(&Log.HouseNum,LogAddr+i*20);				//读取日志
				HDMI_Set_Log(99+(i*13));								//显示日志
			}
		}
De:		if(Details)						//显示日志详情
		{
			HDMI_Check_LogDe();
			FlashReadLog(&Log.HouseNum,LogAddr+(LogDe-1)*20);
			HDMI_Set_LogDe();			
			while(Details){HDMI_Check_Button();delay_ms(200);}
		}
		if(SuspendButton)				//暂停
		{
			ClosePartition();
			FerStop;
			while(SuspendButton)
			{
				ttm=0;
				HDMI_Check_Button();
			}
			OpenPartition();
			if(MODEL)FerRun;
		}
//		if(TimeButton)SysTime=1,HDMI_Set_Partition(MCGS_Partition&0x7F);//申请时间同步
		if(SysTime==2)HDMI_SetSysTime(),SysTime=0;			//更新时间同步
//		if(Err)HDMI_Set_Error(),Err=0;									//弹出错误
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

void 	Work_task(void *pdata)			//灌溉任务
{
	while(1)
	{
	 	delay_ms(200);	
		if(RunState)
		{
			HDMI_READ_START=1;									//触摸屏读取运行所需数据
			while(HDMI_READ_START)delay_ms(200);				//读取完毕
			if(n21_set){GetTime=1;while(GetTime)delay_ms(200);}	//获取触摸屏时间
			else{SysTime=2;while(SysTime)delay_ms(200);}
			Log.StartTime[0]=STIME.year%100;
			Log.StartTime[1]=STIME.month;
			Log.StartTime[2]=STIME.day;
			Log.StartTime[3]=STIME.hour;
			Log.StartTime[4]=STIME.minute;
			Log.StartMode=NET;
			
			FlashReadFer(&Run_SetMessage.Mu,HouseNum*10);
			OpenPartition();									//打开分区电磁阀
			ttm=0;htime=0;										//重置时间计数器
			PreTime=Run_SetMessage.Pre;
			BehindTime=Run_SetMessage.Behind;
			nbup=1;
			if(MODEL==0){stage=3;while((Remaining>0)&RunState)delay_ms(200);}	//清水灌溉倒计时
			else						//肥水灌溉
			{
				Mathflow=NeedFlow=CalculateFlow();//装载计算好的流量
				FerRun;					//打开吸肥泵
				stage=0;while((htime!=PreTime)&RunState)delay_ms(200);			//前置建立时间
				if(!RunState)goto end;
				OpenFer();				//打开专用桶
				OpenFer_Public();		//打开公用桶
				CalculateFlow();		//计算流量
				ChangeFlow();			//流量调节
				stage=1;while((Remaining!=BehindTime)&RunState)delay_ms(200);	//肥水灌溉倒计时
				if(!RunState)goto end;
				CloseFer();				//关闭专用桶
				CloseFer_Pubilc();		//关闭公用桶
				stage=2;while((Remaining>0)&RunState)delay_ms(200);				//后置冲洗时间
				if(!RunState)goto end;
				FerStop;				//关闭吸肥泵
			}
end:		CloseFer();											//关闭肥料桶
			ClosePartition();									//关闭分区电磁阀
			FerStop;											//关闭吸肥泵
			{													//记录日志
				Log.HouseNum=HouseNum;							//记录日志  温室编号
				if(n21_set){GetTime=1;while(GetTime)delay_ms(200);}	//获取触摸屏时间
				else{SysTime=2;while(SysTime)delay_ms(200);}
				Log.StopTime[0]=STIME.year%100;
				Log.StopTime[1]=STIME.month;
				Log.StopTime[2]=STIME.day;
				Log.StopTime[3]=STIME.hour;
				Log.StopTime[4]=STIME.minute;
				Log.StopMode=NET;									//记录日志	启动方式
				Log.IrrMode=MODEL;									//记录日志	灌溉方式
				Log.Irrtime=htime+1;								//记录日志	灌溉时长
				Log.UseWater=Actual_Water;							//本次用水量
				Log.FerNum=Run_SetMessage.SpecialFer;				//本次用肥料桶号
				Log.SFerusage=Actual_Fer;							//本次专用肥量
				Log.PFerusage=Actual_Fer;							//本次公用肥量
				FlashWriteLog(&Log.HouseNum,Logmem.mem32,20);		//日志写入flash
				if(Logmem.mem32==20*100)Logmem.mem32=0;				//日志存储地址增加
				Logmem.mem32+=20;FlashWriteMem(Logmem.mem);			//日志地址增加			
			}
			HDMI_SET_STOP=1;									//触摸屏返回主界面
			while(HDMI_SET_STOP)delay_ms(200);
			stage=0;
			nbup=1;
			NET=0;
		}
	}
}

void	Uptoaliyun()					//上传设备状态到阿里云
{
	char a[250];
	char c[250];
	char *b=c;
	u8 i;i=0;
	memset(a,0,sizeof a);
	sprintf(b,"\\\"State\\\":%d,",RunState);		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"House\\\":%d,",HouseNum);		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"Water\\\":%d,",Actual_Water);	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"Fer\\\":%d,",Actual_Fer);		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"Pressure\\\":%.1f,",(float)pr.f);for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"IrrMode\\\":%d,",MODEL);			for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"Time\\\":%d,",Remaining);		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"Sbarrel\\\":%d,",Run_SetMessage.SpecialFer);		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"Pbarrel\\\":%d,",PublicFer);		for(;*b;b++)a[i++]=*b;
	Nsendok=sendN21(a,0);
}

void	readset()						//读取阿里下发的数据
{
	u8 len;cJSON *json;
	u8 buf[250];
	if(n21_set)return;
	len=COM1GetBuf(buf,200);
	if(len>64)
	{
		if(strstr((const char *)buf,"RCVPUB")[0]!='R'){memset(buf,0,200);comClearRxFifo(COM1);return;}
		json=cJSON_Parse(strstr((const char *)buf,"{"));
		NET=1;
		HouseNum=atoi(cJSON_Print(cJSON_GetObjectItem(json,"House")));
		MODEL=atoi(cJSON_Print(cJSON_GetObjectItem(json,"IrrMode")));
		PublicFer=atoi(cJSON_Print(cJSON_GetObjectItem(json,"Pbarrel")));
		Remaining=atoi(cJSON_Print(cJSON_GetObjectItem(json,"Time")));
		OSTaskSuspend(HDMI_TASK_PRIO);
		HDMI_Set_IrrMode(MODEL);
		len=atoi(cJSON_Print(cJSON_GetObjectItem(json,"State")));
		if(len)HDMI_Set_Button(MCGS_Button|1);
		else HDMI_Set_Button(MCGS_Button&0xfe);
		OSTaskResume(HDMI_TASK_PRIO);
		memset(buf,0,200);
	}
}

void	Upccid()						//上报CCID
{
	char c[50];
	char *b=c;
	sprintf(b,"\\\"CCID\\\":\\\"%s\\\"",CCID);
	Nsendok=sendN21(b,0);
}

void 	Upyun_task(void *pdata)			//上传云任务 N21
{
	u8 i;
	u8 buf[20];
	NeoWayN21_init();					//N21初始化 注网
	conN21();							//N21连接到阿里云
	for(i=0;i<3;i++)					//等待N21完全初始化完
	delay_ms(1000);
	Upccid();							//上报CCID
	memset(g_RxBuf1,0,UART1_RX_BUF_SIZE);
	while(1)
	{
		delay_ms(200);
		readset();						//读取阿里云下发数据
		if(nbup)
		{
			nbup=0;
			n21_set=1;
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
				NeoWayN21_init();					//N21初始化 注网
				conN21();							//N21连接到阿里云
				memset(buf,0,10);
				n21_set=0;
				memset(g_RxBuf1,0,UART1_RX_BUF_SIZE);
			}
			Uptoaliyun();
			n21_set=0;
		}
		if(!Nsendok)
		{
			n21_set=1;
			NeoWayN21_init();
			conN21();
			comClearRxFifo(COM1);
			memset(g_RxBuf1,0,UART1_RX_BUF_SIZE);
			n21_set=0;
		}	
	}
}
