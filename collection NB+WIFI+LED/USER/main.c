#include "main.h"
#include "LED_dz.h"

#define 	sensor_num		1			//传感器数量

Environmental sensor[EnvNum];			//传感器数量上限组数的内存

u16	Alarm(u16 armtime)					//报警判断
{
	u8 i;
	char a[250];
	char c[250];
	char *b=c;
	u8 k=0;
	armtime++;
	memset(a,0,sizeof a);
	memset(a,0,sizeof c);
	for(i=0;i<sensor_num;i++,k=0)
	{
		if((sensor[i].airtemp>Alarm_war[0].Alarm_airtemp_up)|(sensor[i].airtemp<Alarm_war[0].Alarm_airtemp_dn))			//气温
		{
			SetControlForeColor(i+1,4,0xf800);
			delay_ms(100);
			sprintf(b,"\\\"airtemp_%d\\\":%2.1f,",i+1,(float)(sensor[i].airtemp/10.0));		for(;*b;b++)a[k++]=*b;
		}
		if((sensor[i].airhumi>Alarm_war[0].Alarm_airhumi_up)|(sensor[i].airhumi<Alarm_war[0].Alarm_airhumi_dn))			//气湿
		{
			SetControlForeColor(i+1,5,0xf800);
			delay_ms(100);
			sprintf(b,"\\\"airhumi_%d\\\":%2.1f,",i+1,(float)(sensor[i].airhumi/10.0));		for(;*b;b++)a[k++]=*b;
		}
		if((sensor[i].light>Alarm_war[0].Alarm_light_up)|(sensor[i].light<Alarm_war[0].Alarm_light_dn))					//光强
		{
			SetControlForeColor(i+1,6,0xf800);
			delay_ms(100);
			sprintf(b,"\\\"light_%d\\\":%d,",i+1,sensor[i].light);							for(;*b;b++)a[k++]=*b;
		}
		if((sensor[i].CO2>Alarm_war[0].Alarm_CO2_up)|(sensor[i].CO2<Alarm_war[0].Alarm_CO2_dn))							//CO2
		{
			SetControlForeColor(i+1,7,0xf800);
			delay_ms(100);
			sprintf(b,"\\\"CO2_%d\\\":%d,",i+1,sensor[i].CO2);								for(;*b;b++)a[k++]=*b;
		}
		if((sensor[i].soilhumi>Alarm_war[0].Alarm_soilhumi_up)|(sensor[i].soilhumi<Alarm_war[0].Alarm_soilhumi_dn))		//土温
		{
			SetControlForeColor(i+1,8,0xf800);
			delay_ms(100);
			sprintf(b,"\\\"soilhumi_%d\\\":%2.1f,",i+1,(float)(sensor[i].soilhumi/10.0));	for(;*b;b++)a[k++]=*b;
		}
		if((sensor[i].soiltemp>Alarm_war[0].Alarm_soiltemp_up)|(sensor[i].soiltemp<Alarm_war[0].Alarm_soiltemp_dn))		//土湿
		{
			SetControlForeColor(i+1,9,0xf800);
			delay_ms(100);
			sprintf(b,"\\\"soiltemp_%d\\\":%2.1f,",i+1,(float)(sensor[i].soiltemp/10.0));	for(;*b;b++)a[k++]=*b;
		}
		if((sensor[i].EC>Alarm_war[0].Alarm_EC_up)|(sensor[i].EC<Alarm_war[0].Alarm_EC_dn))								//电导率
		{
			SetControlForeColor(i+1,10,0xf800);
			delay_ms(100);
			sprintf(b,"\\\"EC_%d\\\":%d",i+1,sensor[i].EC);									for(;*b;b++)a[k++]=*b;
		}
		if((armtime/80)>=up_wartime)				//报警上报周期
		{
			armtime=0;
			if(k>2)
			{
				if(N21_B)
				{
					n21_set=1;
					OSTaskSuspend(Upyun_TASK_PRIO);
					sendN21(a,1);
					OSTaskResume(Upyun_TASK_PRIO);
					n21_set=0;
				}	

				if(Emw_B)
				{
					emw_set=1;
					OSTaskSuspend(UpyunWF_TASK_PRIO);
					sendEmw(a,1);
					OSTaskResume(UpyunWF_TASK_PRIO);
					emw_set=0;
				}	
				
				delay_ms(1000);delay_ms(1000);delay_ms(1000);delay_ms(1000);delay_ms(1000);
			}
		}
	}
	return armtime;
}

void	Update_Sensor_Number()			//更新传感器显示
{
	u8 i;
	for(i=0;i<sensor_num;i++)
	{
		delay_ms(50);
		SetTextFloat(i+1,4,(float)sensor[i].airtemp/10,1,1);
		delay_ms(50);
		SetTextFloat(i+1,5,(float)sensor[i].airhumi/10,1,1);
		delay_ms(50);
		SetTextValueInt32(i+1,6,sensor[i].light);
		delay_ms(50);
		SetTextValueInt32(i+1,7,sensor[i].CO2);
		delay_ms(50);
		SetTextFloat(i+1,9,(float)sensor[i].soiltemp/10,1,1);
		delay_ms(50);
		SetTextFloat(i+1,8,(float)sensor[i].soilhumi/10,1,1);
		delay_ms(50);
		SetTextValueInt32(i+1,10,sensor[i].EC);
	}
}

void 	UpdateUI()						//更新触摸屏显示
{
	delay_ms(100);
	Update_Sensor_Number();delay_ms(100);
}

extern uint8_t g_RxBuf5[UART5_RX_BUF_SIZE];
void	setalarm()						//显示报警设置值
{
	u8 i;char buf[22];
	for(i=0;i<war_group;i++)
	{
		SetTextValueInt32(i+war_group+1,3,Alarm_war[i].Alarm_airtemp_up/10);	delay_ms(100);
		SetTextValueInt32(i+war_group+1,4,Alarm_war[i].Alarm_airtemp_dn/10);	delay_ms(100);
		SetTextValueInt32(i+war_group+1,5,Alarm_war[i].Alarm_airhumi_up/10);	delay_ms(100);
		SetTextValueInt32(i+war_group+1,6,Alarm_war[i].Alarm_airhumi_dn/10);	delay_ms(100);
		SetTextValueInt32(i+war_group+1,7,Alarm_war[i].Alarm_light_up);			delay_ms(100);
		SetTextValueInt32(i+war_group+1,8,Alarm_war[i].Alarm_light_dn);			delay_ms(100);
		SetTextValueInt32(i+war_group+1,9,Alarm_war[i].Alarm_CO2_up);			delay_ms(100);
		SetTextValueInt32(i+war_group+1,10,Alarm_war[i].Alarm_CO2_dn);			delay_ms(100);
		SetTextValueInt32(i+war_group+1,13,Alarm_war[i].Alarm_soiltemp_up/10);	delay_ms(100);
		SetTextValueInt32(i+war_group+1,14,Alarm_war[i].Alarm_soiltemp_dn/10);	delay_ms(100);
		SetTextValueInt32(i+war_group+1,11,Alarm_war[i].Alarm_soilhumi_up/10);	delay_ms(100);
		SetTextValueInt32(i+war_group+1,12,Alarm_war[i].Alarm_soilhumi_dn/10);	delay_ms(100);
		SetTextValueInt32(i+war_group+1,15,Alarm_war[i].Alarm_EC_up);			delay_ms(100);
		SetTextValueInt32(i+war_group+1,16,Alarm_war[i].Alarm_EC_dn);			delay_ms(100);
		SetTextValueInt32(4,9,DHCP);delay_ms(100);
		SetTextValueInt32(4,30,up_time);delay_ms(100);
		SetTextValueInt32(4,31,up_wartime);delay_ms(100);
		sprintf(buf,"%s",ssid);
		SetTextValue(4,1,(unsigned char *)buf);
		sprintf(buf,"%s",password);
		SetTextValue(4,2,(unsigned char *)buf);
		sprintf(buf,"%s",ip);
		SetTextValue(4,3,(unsigned char *)buf);
		sprintf(buf,"%s",network);
		SetTextValue(4,4,(unsigned char *)buf);
		sprintf(buf,"%s",gateway);
		SetTextValue(4,5,(unsigned char *)buf);
		sprintf(buf,"%s",dns);
		SetTextValue(4,6,(unsigned char *)buf);
	}
	delay_ms(200);
	memset(g_RxBuf5,0,UART5_RX_BUF_SIZE);
	comClearRxFifo(COM5);
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

u16		readcmd1(u8 i)					//读取空气传感器返回的数据
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

u16		readcmd2(u8 i)					//读取土壤传感器返回的数据
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
		//if(sensor[i].soilhumi==0)return 0;
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
			k=readcmd1(0);
			if(k)break;
		}while(1);
		delay_ms(500);
		
		do{
			sendcmd_read(50+i,CMD_AIRHUMI);			//读取空气湿度
			delay_ms(200);
			k=readcmd2(0);
			if(k)break;
		}while(1);
	}
}

void	HDMI_Init()						//触摸屏初始化
{
	queue_reset();			//清空缓存区
	delay_ms(300);			//必须等待300ms
	init_war();				//读取设置报警值
	UpdateUI();				//显示传感器数值
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

void	readflash(void);
void	sendflash(void);
void	init()							//系统初始化
{
	SystemInit();
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	LED_Init();		  		//初始化与LED连接的硬件接口	
	bsp_InitUart();
	HDMI_Init();
	comClearRxFifo(COM4);
	readflashthree();				//读取内存里写入的三元组数据
//	readflash();
//	sendflash();
	readflash();
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
	OSTaskCreate(SaveThree_task,(void *)0,(OS_STK*)&SaveThree_TASK_STK[SaveThree_STK_SIZE-1],SaveThree_TASK_PRIO);
	OSTaskCreate(Upyun_task,(void *)0,(OS_STK*)&Upyun_TASK_STK[Upyun_STK_SIZE-1],Upyun_TASK_PRIO);
	OSTaskCreate(UpyunWF_task,(void *)0,(OS_STK*)&UpyunWF_TASK_STK[UpyunWF_STK_SIZE-1],UpyunWF_TASK_PRIO);
	OSTaskCreate(HDMI_task,(void *)0,(OS_STK*)&HDMI_TASK_STK[HDMI_STK_SIZE-1],HDMI_TASK_PRIO);
	OSTaskCreate(Collection_task,(void *)0,(OS_STK*)&Collection_TASK_STK[Collection_STK_SIZE-1],Collection_TASK_PRIO);
	OSTaskCreate(LED_DZ_task,(void *)0,(OS_STK*)&LED_DZ_TASK_STK[LED_DZ_STK_SIZE-1],LED_DZ_TASK_PRIO);
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

void	sendflash(void);
void	readflash()						//从flash读取报警设置
{
	u16 data;
	u16 buf[50];
	u8 i,k;
	u32 l;

	memset(buf,0,sizeof buf);
	STMFLASH_Read(400+FLASH_SAVE_ADDR+200,(u16*)&data,1);
	if(data!=0xaaaa){sendflash(); return;}

	STMFLASH_Read(400+FLASH_SAVE_ADDR,buf,50);
	
	k=0;
	for(i=0;i<war_group;i++)
	{
		Alarm_war[i].Alarm_airtemp_up=buf[k++];
		Alarm_war[i].Alarm_airtemp_dn=buf[k++];
		Alarm_war[i].Alarm_airhumi_up=buf[k++];
		Alarm_war[i].Alarm_airhumi_dn=buf[k++];
		Alarm_war[i].Alarm_CO2_up=buf[k++];
		Alarm_war[i].Alarm_CO2_dn=buf[k++];
		Alarm_war[i].Alarm_soiltemp_up=buf[k++];
		Alarm_war[i].Alarm_soiltemp_dn=buf[k++];
		Alarm_war[i].Alarm_soilhumi_up=buf[k++];
		Alarm_war[i].Alarm_soilhumi_dn=buf[k++];
		Alarm_war[i].Alarm_EC_up=buf[k++];
		Alarm_war[i].Alarm_EC_dn=buf[k++];
		l=buf[k++]<<16;Alarm_war[i].Alarm_light_up=l|buf[k++];
		l=buf[k++]<<16;Alarm_war[i].Alarm_light_dn=l|buf[k++];
	}
	
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
	
	STMFLASH_Read(610+FLASH_SAVE_ADDR,(u16*)&data,1);
	if(data>=1&&data<0xffff)up_time=data;
	
	STMFLASH_Read(620+FLASH_SAVE_ADDR,(u16*)&data,1);
	if(data>=1&&data<0xffff)up_wartime=data;
	
	STMFLASH_Read(630+FLASH_SAVE_ADDR,(u16*)&data,1);
	if(data<0xffff)DHCP=data;
}

void	sendflash()						//存储报警设置到flash
{
	u16 bz=0xaaaa;
	u16 buf[50];
	u8 i,k;
	k=0;
	for(i=0;i<war_group;i++)
	{
		buf[k++]=Alarm_war[i].Alarm_airtemp_up;
		buf[k++]=Alarm_war[i].Alarm_airtemp_dn;
		buf[k++]=Alarm_war[i].Alarm_airhumi_up;
		buf[k++]=Alarm_war[i].Alarm_airhumi_dn;
		buf[k++]=Alarm_war[i].Alarm_CO2_up;
		buf[k++]=Alarm_war[i].Alarm_CO2_dn;
		buf[k++]=Alarm_war[i].Alarm_soiltemp_up;
		buf[k++]=Alarm_war[i].Alarm_soiltemp_dn;
		buf[k++]=Alarm_war[i].Alarm_soilhumi_up;
		buf[k++]=Alarm_war[i].Alarm_soilhumi_dn;
		buf[k++]=Alarm_war[i].Alarm_EC_up;
		buf[k++]=Alarm_war[i].Alarm_EC_dn;
		buf[k++]=Alarm_war[i].Alarm_light_up>>16;
		buf[k++]=Alarm_war[i].Alarm_light_up&0xffff;
		buf[k++]=Alarm_war[i].Alarm_light_dn>>16;
		buf[k++]=Alarm_war[i].Alarm_light_dn&0xffff;
	}	
	
	STMFLASH_Write(400+FLASH_SAVE_ADDR,(u16*)buf,48);
	STMFLASH_Write(600+FLASH_SAVE_ADDR,(u16*)&bz,1);
	STMFLASH_Write(610+FLASH_SAVE_ADDR,(u16*)&up_time,1);
	STMFLASH_Write(620+FLASH_SAVE_ADDR,(u16*)&up_wartime,1);
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

void	UpSetAlarm_wifi(Waring data,u8 group);
void	UpSetAlarm(Waring data,u8 group);
void 	HDMI_task(void *pdata)			//触摸屏监控任务
{	  
	static qsize  size = 0;
	OS_CPU_SR cpu_sr=0;
	u8 t=0,i;
	
	setalarm();
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
			SaveButton=0;
			OS_ENTER_CRITICAL();
			sendflash();			//写入设置的值
			OS_EXIT_CRITICAL();
			OSTaskSuspend(UpyunWF_TASK_PRIO);
			if(Emw_B)UpSetAlarm_wifi(Alarm_war[0],0);	//上传设置消息
			OSTaskResume(UpyunWF_TASK_PRIO);
			OSTaskSuspend(Upyun_TASK_PRIO);
			if(N21_B)UpSetAlarm(Alarm_war[0],0);
			OSTaskResume(Upyun_TASK_PRIO);
			setalarm();							//更新触摸屏设置显示
		}
	}
}

void	Uptoaliyun_wifi(Environmental data,u8 group)	//上传传感器数据到阿里云 WIFI
{
	char a[250];
	char c[250];
	char *b=c;
	u8 i;i=0;
	memset(a,0,sizeof a);
	OSTaskSuspend(Alarm_TASK_PRIO);
	sprintf(b,"\"airtemp_%d\":%2.1f,",group+1,(float)(data.airtemp/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"airhumi_%d\":%2.1f,",group+1,(float)(data.airhumi/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"CO2_%d\":%d,",group+1,data.CO2);								for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"light_%d\":%d,",group+1,data.light);							for(;*b;b++)a[i++]=*b;
	
	sprintf(b,"\"soiltemp_%d\":%2.1f,",group+1,(float)(data.soiltemp/10.0));	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"soilhumi_%d\":%2.1f,",group+1,(float)(data.soilhumi/10.0));	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"EC_%d\":%d",group+1,data.EC);									for(;*b;b++)a[i++]=*b;
	
	sendEmw(a,0);
	OSTaskResume(Alarm_TASK_PRIO);
}

void	Uptoaliyun(Environmental data,u8 group)			//上传传感器数据到阿里云
{
	char a[250];
	char c[250];
	char *b=c;
	u8 i;i=0;
	memset(a,0,sizeof a);
	OSTaskSuspend(Alarm_TASK_PRIO);
	sprintf(b,"\\\"airtemp_%d\\\":%2.1f,",group+1,(float)(data.airtemp/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"airhumi_%d\\\":%2.1f,",group+1,(float)(data.airhumi/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"CO2_%d\\\":%d,",group+1,data.CO2);								for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"light_%d\\\":%d,",group+1,data.light);							for(;*b;b++)a[i++]=*b;
	
	sprintf(b,"\\\"soiltemp_%d\\\":%2.1f,",group+1,(float)(data.soiltemp/10.0));	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"soilhumi_%d\\\":%2.1f,",group+1,(float)(data.soilhumi/10.0));	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"EC_%d\\\":%d",group+1,data.EC);									for(;*b;b++)a[i++]=*b;
	
	Nsendok=sendN21(a,0);
	OSTaskResume(Alarm_TASK_PRIO);
	
}

void	UpSetAlarm(Waring data,u8 group)				//上传报警设置值到阿里云
{
	char a[350];
	char c[350];
	char *b=c;
	u16 i;i=0;
	memset(a,0,sizeof a);
	sprintf(b,"\\\"airtempUP_war\\\":%2.1f,",(float)(data.Alarm_airtemp_up/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"airtempDN_war\\\":%2.1f,",(float)(data.Alarm_airtemp_up/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"airhumiUP_war\\\":%2.1f,",(float)(data.Alarm_airhumi_up/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"airhumiDN_war\\\":%2.1f,",(float)(data.Alarm_airhumi_dn/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"lightUP_war\\\":%d,",data.Alarm_light_up);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"lightDN_war\\\":%d,",data.Alarm_light_dn);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"CO2UP_war\\\":%d,",data.Alarm_CO2_up);								for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"CO2DN_war\\\":%d,",data.Alarm_CO2_dn);								for(;*b;b++)a[i++]=*b;
	
	sprintf(b,"\\\"soiltempUP_war\\\":%2.1f,",(float)(data.Alarm_soiltemp_up/10.0));	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"soiltempDN_war\\\":%2.1f,",(float)(data.Alarm_soiltemp_dn/10.0));	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"soilhumiUP_war\\\":%2.1f,",(float)(data.Alarm_soilhumi_up/10.0));	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"soilhumiDN_war\\\":%2.1f,",(float)(data.Alarm_soilhumi_dn/10.0));	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"ECUP_war\\\":%d,",data.Alarm_EC_up);									for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"ECDN_war\\\":%d,",data.Alarm_EC_dn);									for(;*b;b++)a[i++]=*b;
	
	sprintf(b,"\\\"up_time\\\":%d,",up_time);											for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"up_wartime\\\":%d,",up_wartime);										for(;*b;b++)a[i++]=*b;

	Nsendok=sendN21(a,0);
}

void	UpSetAlarm_wifi(Waring data,u8 group)			//上传报警设置值到阿里云
{
	char a[300];
	char c[300];
	char *b=c;
	u16 i;i=0;
	memset(a,0,sizeof a);
	sprintf(b,"\"airtempUP_war\":%2.1f,",(float)(data.Alarm_airtemp_up/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"airtempDN_war\":%2.1f,",(float)(data.Alarm_airtemp_up/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"airhumiUP_war\":%2.1f,",(float)(data.Alarm_airhumi_up/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"airhumiDN_war\":%2.1f,",(float)(data.Alarm_airhumi_dn/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"lightUP_war\":%d,",data.Alarm_light_up);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"lightDN_war\":%d,",data.Alarm_light_dn);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"CO2UP_war\":%d,",data.Alarm_CO2_up);								for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"CO2DN_war\":%d,",data.Alarm_CO2_dn);								for(;*b;b++)a[i++]=*b;
	
	sprintf(b,"\"soiltempUP_war\":%2.1f,",(float)(data.Alarm_soiltemp_up/10.0));	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"soiltempDN_war\":%2.1f,",(float)(data.Alarm_soiltemp_dn/10.0));	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"soilhumiUP_war\":%2.1f,",(float)(data.Alarm_soilhumi_up/10.0));	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"soilhumiDN_war\":%2.1f,",(float)(data.Alarm_soilhumi_dn/10.0));	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"ECUP_war\":%d,",data.Alarm_EC_up);									for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"ECDN_war\":%d,",data.Alarm_EC_dn);									for(;*b;b++)a[i++]=*b;

	sendEmw(a,0);
}

void	readthree()										//从COM3 485串口读取是否设置了三元组数据
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
		if(strstr((const char *)buf,"m1")[0]=='m')
		{
			Three=1;
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
			sendflashthree(len);
			comSendBuf(COM4,buf,strlen((char *)buf));
			comClearRxFifo(COM4);
			memset(buf,0,sizeof buf);
			memset(hc,0,sizeof buf);
		}
		else if(strstr((const char *)buf,"m2")[0]=='m')
		{
			Three=1;
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
			sendflashthree(len);
			comSendBuf(COM4,buf,strlen((char *)buf));
			comClearRxFifo(COM4);
			memset(buf,0,sizeof buf);
			memset(hc,0,sizeof buf);
		}
	}
}

void	readset(u8 t)									//读取阿里下发的数据
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
		msg=strstr((const char *)buf,"airtempUP_war:");	if(msg[0]=='a')Alarm_war[0].Alarm_airtemp_up=	((msg[17]-0x30)*10+(msg[18]-0x30))*10;
		msg=strstr((const char *)buf,"airtempDN_war:");	if(msg[0]=='a')Alarm_war[0].Alarm_airtemp_dn=	((msg[17]-0x30)*10+(msg[18]-0x30))*10;
		msg=strstr((const char *)buf,"airhumiUP_war:");	if(msg[0]=='a')Alarm_war[0].Alarm_airhumi_up=	((msg[17]-0x30)*10+(msg[18]-0x30))*10;
		msg=strstr((const char *)buf,"airhumiDN_war:");	if(msg[0]=='a')Alarm_war[0].Alarm_airhumi_dn=	((msg[17]-0x30)*10+(msg[18]-0x30))*10;
		msg=strstr((const char *)buf,"CO2UP_war:");		if(msg[0]=='C')Alarm_war[0].Alarm_CO2_up=		(msg[10]-0x30)*10000+(msg[11]-0x30)*1000+(msg[12]-0x30)*100+(msg[13]-0x30)*10+(msg[14]-0x30);
		msg=strstr((const char *)buf,"CO2DN_war:");		if(msg[0]=='C')Alarm_war[0].Alarm_CO2_dn=		(msg[10]-0x30)*10000+(msg[11]-0x30)*1000+(msg[12]-0x30)*100+(msg[13]-0x30)*10+(msg[14]-0x30);
		msg=strstr((const char *)buf,"lightUP_war:");	if(msg[0]=='l')Alarm_war[0].Alarm_light_up=		(msg[12]-0x30)*10000+(msg[13]-0x30)*1000+(msg[14]-0x30)*100+(msg[15]-0x30)*10+(msg[16]-0x30);
		msg=strstr((const char *)buf,"lightDN_war:");	if(msg[0]=='l')Alarm_war[0].Alarm_light_dn=		(msg[12]-0x30)*10000+(msg[13]-0x30)*1000+(msg[14]-0x30)*100+(msg[15]-0x30)*10+(msg[16]-0x30);
		msg=strstr((const char *)buf,"ECUP_war:");		if(msg[0]=='E')Alarm_war[0].Alarm_EC_up=		(msg[12]-0x30)*10+(msg[13]-0x30);
		msg=strstr((const char *)buf,"ECDN_war:");		if(msg[0]=='E')Alarm_war[0].Alarm_EC_dn=		(msg[12]-0x30)*10+(msg[13]-0x30);
		msg=strstr((const char *)buf,"soiltempUP_war:");if(msg[0]=='s')Alarm_war[0].Alarm_soiltemp_up=	((msg[18]-0x30)*10+(msg[19]-0x30))*10;
		msg=strstr((const char *)buf,"soiltempDN_war:");if(msg[0]=='s')Alarm_war[0].Alarm_soiltemp_dn=	((msg[18]-0x30)*10+(msg[19]-0x30))*10;
		msg=strstr((const char *)buf,"soilhumiUP_war:");if(msg[0]=='s')Alarm_war[0].Alarm_soilhumi_up=	((msg[18]-0x30)*10+(msg[19]-0x30))*10;
		msg=strstr((const char *)buf,"soilhumiDN_war:");if(msg[0]=='s')Alarm_war[0].Alarm_soilhumi_dn=	((msg[18]-0x30)*10+(msg[19]-0x30))*10;
		msg=strstr((const char *)buf,"up_time:");		if(msg[0]=='u')up_time=							(msg[8]-0x30)*10000+(msg[9]-0x30)*1000+(msg[10]-0x30)*100+(msg[11]-0x30)*10+(msg[12]-0x30);
		msg=strstr((const char *)buf,"up_wartime:");	if(msg[0]=='u')up_wartime=						(msg[11]-0x30)*10000+(msg[12]-0x30)*1000+(msg[13]-0x30)*100+(msg[14]-0x30)*10+(msg[15]-0x30);
		
		OSTaskSuspend(HDMI_TASK_PRIO);				//存储下发的设置内容
		sendflash();
		setalarm();
		OSTaskResume(HDMI_TASK_PRIO);
		comClearRxFifo(COM1);

		OSTaskSuspend(Alarm_TASK_PRIO);				//上报回阿里平台
		if(t)OSTaskSuspend(UpyunWF_TASK_PRIO);
		if(Emw_B)UpSetAlarm_wifi(Alarm_war[0],0);
		UpSetAlarm(Alarm_war[0],0);
		OSTaskResume(Alarm_TASK_PRIO);
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
			msg=strstr((const char *)buf,"airtempUP_war:");	if(msg[0]=='a')Alarm_war[0].Alarm_airtemp_up=	((msg[17]-0x30)*10+(msg[18]-0x30))*10;
			msg=strstr((const char *)buf,"airtempDN_war:");	if(msg[0]=='a')Alarm_war[0].Alarm_airtemp_dn=	((msg[17]-0x30)*10+(msg[18]-0x30))*10;
			msg=strstr((const char *)buf,"airhumiUP_war:");	if(msg[0]=='a')Alarm_war[0].Alarm_airhumi_up=	((msg[17]-0x30)*10+(msg[18]-0x30))*10;
			msg=strstr((const char *)buf,"airhumiDN_war:");	if(msg[0]=='a')Alarm_war[0].Alarm_airhumi_dn=	((msg[17]-0x30)*10+(msg[18]-0x30))*10;
			msg=strstr((const char *)buf,"CO2UP_war:");		if(msg[0]=='C')Alarm_war[0].Alarm_CO2_up=		(msg[10]-0x30)*10000+(msg[11]-0x30)*1000+(msg[12]-0x30)*100+(msg[13]-0x30)*10+(msg[14]-0x30);
			msg=strstr((const char *)buf,"CO2DN_war:");		if(msg[0]=='C')Alarm_war[0].Alarm_CO2_dn=		(msg[10]-0x30)*10000+(msg[11]-0x30)*1000+(msg[12]-0x30)*100+(msg[13]-0x30)*10+(msg[14]-0x30);
			msg=strstr((const char *)buf,"lightUP_war:");	if(msg[0]=='l')Alarm_war[0].Alarm_light_up=		(msg[12]-0x30)*10000+(msg[13]-0x30)*1000+(msg[14]-0x30)*100+(msg[15]-0x30)*10+(msg[16]-0x30);
			msg=strstr((const char *)buf,"lightDN_war:");	if(msg[0]=='l')Alarm_war[0].Alarm_light_dn=		(msg[12]-0x30)*10000+(msg[13]-0x30)*1000+(msg[14]-0x30)*100+(msg[15]-0x30)*10+(msg[16]-0x30);
			msg=strstr((const char *)buf,"ECUP_war:");		if(msg[0]=='E')Alarm_war[0].Alarm_EC_up=		(msg[12]-0x30)*10+(msg[13]-0x30);
			msg=strstr((const char *)buf,"ECDN_war:");		if(msg[0]=='E')Alarm_war[0].Alarm_EC_dn=		(msg[12]-0x30)*10+(msg[13]-0x30);
			msg=strstr((const char *)buf,"soiltempUP_war:");if(msg[0]=='s')Alarm_war[0].Alarm_soiltemp_up=	((msg[18]-0x30)*10+(msg[19]-0x30))*10;
			msg=strstr((const char *)buf,"soiltempDN_war:");if(msg[0]=='s')Alarm_war[0].Alarm_soiltemp_dn=	((msg[18]-0x30)*10+(msg[19]-0x30))*10;
			msg=strstr((const char *)buf,"soilhumiUP_war:");if(msg[0]=='s')Alarm_war[0].Alarm_soilhumi_up=	((msg[18]-0x30)*10+(msg[19]-0x30))*10;
			msg=strstr((const char *)buf,"soilhumiDN_war:");if(msg[0]=='s')Alarm_war[0].Alarm_soilhumi_dn=	((msg[18]-0x30)*10+(msg[19]-0x30))*10;
			msg=strstr((const char *)buf,"up_time:");		if(msg[0]=='u')up_time=							(msg[8]-0x30)*10000+(msg[9]-0x30)*1000+(msg[10]-0x30)*100+(msg[11]-0x30)*10+(msg[12]-0x30);
			msg=strstr((const char *)buf,"up_wartime:");	if(msg[0]=='u')up_wartime=						(msg[11]-0x30)*10000+(msg[12]-0x30)*1000+(msg[13]-0x30)*100+(msg[14]-0x30)*10+(msg[15]-0x30);
			
			OSTaskSuspend(HDMI_TASK_PRIO);			//存储下发的设置内容
			sendflash();
			setalarm();
			OSTaskResume(HDMI_TASK_PRIO);
			comClearRxFifo(COM2);
			
			OSTaskSuspend(Alarm_TASK_PRIO);			//上报回阿里平台
			if(!t)OSTaskSuspend(Upyun_TASK_PRIO);
			UpSetAlarm_wifi(Alarm_war[0],0);
			if(N21_B)UpSetAlarm(Alarm_war[0],0);
			OSTaskResume(Alarm_TASK_PRIO);
			if(!t)OSTaskResume(Upyun_TASK_PRIO);
			
		}
	}
	memset(buf,0,200);
	memset(c,0,200);
}

void	Upccid()										//上报CCID
{
	char c[50];
	char *b=c;
	sprintf(b,"\\\"CCID\\\":\\\"%s\\\"",CCID);
	sendN21(b,0);
}
extern uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];
void 	Upyun_task(void *pdata)							//上传云任务 N21
{
	u8 g=0;
	u8 i;
	u8 buf[20];
	NeoWayN21_init();					//N21初始化 注网
	conN21();							//N21连接到阿里云
	for(i=0;i<3;i++)					//等待N21完全初始化完
	delay_ms(1000);
	UpSetAlarm(Alarm_war[0],0);delay_ms(1000);		//上报设置的报警值
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
			Uptoaliyun(sensor[g],g);if(++g==sensor_num)g=0;
			n21_set=0;
		}
		if(!Nsendok){n21_set=1;N21_B=0;NeoWayN21_init();conN21();n21_set=0;N21_B=1;}
		readset(0);						//读取阿里云下发数据	
	}
}

void 	UpyunWF_task(void *pdata)						//上传云任务 WIFI
{
	u16 i;u8 buf[50];u8 error=0;
	Emw3060_init();
	Emw3060_con();
	UpSetAlarm_wifi(Alarm_war[0],0);
	Emw_B=1;emw_set=0;
	while(1)
	{
		delay_ms(200);
		readset(1);						//读取阿里云下发数据
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
		if(((++i)/270)>=up_time){i=0;emw_set=1;Uptoaliyun_wifi(sensor[0],0);emw_set=0;}
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

void 	Collection_task(void *pdata)					//采集任务
{
//	u8 i;
	while(1)
	{
		
		delay_ms(200);
		ReadValue();								//读取传感器数值
//		for(i=0;i<sensor_num;i++)					//测试用随机数
//		{
//			sensor[i].airhumi=(rand()%100)*10;
//			sensor[i].airtemp=(rand()%100)*10;
//			sensor[i].soilhumi=(rand()%100)*10;
//			sensor[i].soiltemp=(rand()%100)*10;
//			sensor[i].CO2=rand()%1000;
//			sensor[i].light=rand()%10000;
//		}
	}
}

void 	Alarm_task(void *pdata)							//报警任务
{	
	u16 armtime=0;
	while(1)
	{	
		delay_ms(200);
		armtime=Alarm(armtime);
	}
}

void 	LED_DZ_task(void *pdata)		//LED点阵显示任务
{	 
	u8 i;
	while(1)
	{
		delay_ms(200);
		if(++i==80)
		{
			i=0;dz=0;
			if(Three)continue;
			OSTaskSuspend(SaveThree_TASK_PRIO);
			uptodz();
			OSTaskResume(SaveThree_TASK_PRIO);
		}	
	}
}
