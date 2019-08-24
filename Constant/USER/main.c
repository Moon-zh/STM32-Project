#include "main.h"

#define	readfrequency	0x1001	//频率地址
#define	readvoltage		0x1003	//电压地址
#define	readcurrent		0x1004	//电流地址
#define	readfault		0x8000	//故障地址
#define motor			0x2000	//电机地址
#define readmotor		0x3000	//电机状态
#define run				1		//启动
#define	stop			6		//停止
#define	write_defp		0xF901	//设置压力值
#define	read_defp		0x1010	//读取设置的压力值
#define	readp			0x100A	//当前压力

#define X1	GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)
#define Y1	PCout(7)
u8	x1,y1;

typedef struct				//传感器数组结构体
{
	float	water;		//水表读数
	u16		frequrncy;	//频率
	u16		voltage;	//输出电压
	float	current;	//输出电流
	u16		fault;		//故障状态
	u16		pressure;	//压力
	u16		motorstate;	//电机状态
}Meter;
Meter	PAW;

u8	motor_rs,def_pressure;	//接收电机控制，接收默认压力

u8 tab_send[]   ={0x01 ,0x03 ,0x00 ,0x01 ,0x00 ,0x00 ,0x00,0x00};	//变频器读指令
u8 tab_sendset[]={0x01 ,0x06 ,0x00 ,0x01 ,0x00 ,0x00 ,0x00,0x00};	//变频器设置指令
u8 tab_water[]  ={0x01 ,0x03 ,0x00 ,0x00 ,0x00 ,0x04 ,0x44 ,0x09};	//读水表指令
u8 tab_water_b[]={0x01 ,0x03 ,0x08 ,0x00 ,0x00 ,0x00 ,0x14 ,0x00 ,0x00 ,0x00 ,0x00 ,0xA5 ,0xD4};	//判断水表返回值

void	sendflash(void);

void	X1SET()
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;				//LED0-->PB.5 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			//推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//根据设定参数初始化GPIOB.5
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //使能PB,PE端口时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;				 //LED0-->PB.5 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5
	GPIO_ResetBits(GPIOC,GPIO_Pin_7);						 	//PB.5 输出高
	
}

void	sendvm(u8 cmd,u16 addr,u16 data)//发送指令到变频器
{
	u16 crc;
	tab_send[1]=cmd;
	tab_send[2]=addr>>8;
	tab_send[3]=addr&0xff;
	tab_send[4]=data>>8;
	tab_send[5]=data&0xff;
	crc=mc_check_crc16(tab_send,6);
	tab_send[6]=crc>>8;
	tab_send[7]=crc&0xff;
	comSendBuf(COM4,tab_send,8);
	delay_ms(60);
}

u8		readvm()						//读变频器指令
{
	u16 crc;
	u8 a[26];
	u8 len;
	len=COM4GetBuf(a,15);
	if(len<10)return 0;
	comClearRxFifo(COM4);
	crc=mc_check_crc16(a,len-2);
	if((a[len-2]==(crc>>8))&&(a[len-1]==(crc&0xff)))
	{
		PAW.frequrncy=(a[3]<<8)|a[4];
		PAW.voltage=(a[7]<<8)|a[8];
		PAW.current=(a[9]<<8)|a[10];
		return 1;
	}
	return 0;
}

u8		readvmf(u16 *b)					//读取变频器故障
{
	u16 crc;
	u8 a[26];
	u8 len;
	len=COM4GetBuf(a,10);
	if(len<5)return 0;
	comClearRxFifo(COM4);
	crc=mc_check_crc16(a,len-2);
	if((a[len-2]==(crc>>8))&&(a[len-1]==(crc&0xff)))
	{
		*b=(a[3]<<8)|a[4];
		return 1;
	}
	return 0;
}

u8		bcdtoint(u8 a)					//BCD码转int
{
	return (a>>4)*10+(a&0x0f);
}

void	sendwater(u8 *a)				//发送读水表指令
{
	comSendBuf(COM5,a,8);
}

void	save3m3h(u8 *a)					//存储水表读出的数据
{
	PAW.water=bcdtoint(a[7])*1000+bcdtoint(a[8])*100+bcdtoint(a[9])*10+bcdtoint(a[10])+bcdtoint(a[3])/10.0+bcdtoint(a[4])/100.0+bcdtoint(a[5])/1000.0+bcdtoint(a[6])/10000.0;
}

u8		pdcmd(u8 *a)					//判断水表读值
{
	u16 crc;
	u8 len;
	len=COM5GetBuf(a,15);
	if(len<7)return 0;
	comClearRxFifo(COM5);
	crc=mc_check_crc16(a,len-2);
	if((a[len-2]==(crc>>8))&&(a[len-1]==(crc&0xff)))
	{
		save3m3h(a);
		comClearRxFifo(COM5);
		return 1;
	}
	return 0;
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
	
	STMFLASH_Read(610+FLASH_SAVE_ADDR,(u16*)&data,1);
	if(data>=1&&data<0xffff)up_t1=data;
	
	STMFLASH_Read(620+FLASH_SAVE_ADDR,(u16*)&data,1);
	if(data>=1&&data<0xffff)up_t2=data;
	
	STMFLASH_Read(630+FLASH_SAVE_ADDR,(u16*)&data,1);
	if(data>=1&&data<0xffff)def_pressure=data;
}

void	sendflash()						//存储报警设置到flash
{
	u16 bz=0xaaaa;
	u16 buf[50];
	u8 i,k;

	STMFLASH_Write(600+FLASH_SAVE_ADDR,(u16*)&bz,1);
	STMFLASH_Write(610+FLASH_SAVE_ADDR,(u16*)&up_t1,1);
	STMFLASH_Write(620+FLASH_SAVE_ADDR,(u16*)&up_t2,1);
	STMFLASH_Write(630+FLASH_SAVE_ADDR,(u16*)&def_pressure,1);
	
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
	
	readflash();
}

void	init()							//系统初始化
{
	SystemInit();
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	LED_Init();		  		//初始化与LED连接的硬件接口	
	bsp_InitUart();
	X1SET();
//	readflash
//	readflashthree();				//读取内存里写入的三元组数据
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
 	OSTaskCreate(LED_task,(void *)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);
	OSTaskCreate(VM_task,(void *)0,(OS_STK*)&VM_TASK_STK[VM_STK_SIZE-1],VM_TASK_PRIO);
	OSTaskCreate(Converter_task,(void *)0,(OS_STK*)&Converter_TASK_STK[Converter_STK_SIZE-1],Converter_TASK_PRIO);
	OSTaskCreate(SaveThree_task,(void *)0,(OS_STK*)&SaveThree_TASK_STK[SaveThree_STK_SIZE-1],SaveThree_TASK_PRIO);
	OSTaskCreate(UpyunWF_task,(void *)0,(OS_STK*)&UpyunWF_TASK_STK[UpyunWF_STK_SIZE-1],UpyunWF_TASK_PRIO);
	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
}

void 	LED_task(void *pdata)			//LED运行状态指示灯任务
{	 
	u8 i=0;
	while(1)
	{
		delay_ms(200);
		y1=Y1;
		if(x1!=X1)
		{
			x1=X1;
			Y1=x1;
		}
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
	}
}

void	Uptoaliyun_wifi()				//上传传感器数据到阿里云 WIFI
{
	char a[250];
	char c[250];
	char *b=c;
	u8 i;i=0;
	memset(a,0,sizeof a);
	sprintf(b,"\"water\":%04.4f,",PAW.water);					for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"frequrncy\":%0.2f,",(float)PAW.frequrncy/100);	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"voltage\":%d,",PAW.voltage);					for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"current\":%0.1f,",PAW.current/1000);			for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"fault\":%d,",PAW.fault);						for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"pressure\":%0.1f,",(float)PAW.pressure/100);	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"motorstate\":%d,",PAW.motorstate);				for(;*b;b++)a[i++]=*b;

	sendEmw(a,0);
}

void	Upset()							//回报阿里下发的设置值
{
	char a[250];
	char c[250];
	char *b=c;
	u8 i;i=0;
	memset(a,0,sizeof a);

	sprintf(b,"\"up_t1\":%d,",up_t1);				for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"up_t2\":%d,",up_t2);				for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"motor_rs\":%d,",motor_rs);			for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"def_pressure\":%d,",def_pressure);	for(;*b;b++)a[i++]=*b;

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
	len=COM3GetBuf(buf,200);
	if(len>50)
	{
		if(strstr((const char *)buf,"m2")[0]=='m')
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
			
		sendflashthree(len);
		comSendBuf(COM3,buf,strlen((char *)buf));
		comClearRxFifo(COM3);
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

	if(emw_set)return;
	len=COM2GetBuf(buf,100);
	if(len>10)
	{
		delay_ms(200);COM2GetBuf(buf,100);		//读取从设备下发的数据
		if(strstr((const char *)buf,"RECV")[0]!='R'){memset(buf,0,200);memset(c,0,200);comClearRxFifo(COM2);return;}
		msg=strstr((const char *)buf,"up_t1:");			if(msg[0]=='u')up_t1=		(msg[6]-0x30)*10000+(msg[7]-0x30)*1000+(msg[8]-0x30)*100+(msg[9]-0x30)*10+(msg[10]-0x30);
		msg=strstr((const char *)buf,"up_t2:");			if(msg[0]=='u')up_t2=		(msg[6]-0x30)*10000+(msg[7]-0x30)*1000+(msg[8]-0x30)*100+(msg[9]-0x30)*10+(msg[10]-0x30);
		msg=strstr((const char *)buf,"def_pressure:");	
		if(msg[0]=='d')
		{
			def_pressure=	(msg[13]-0x30)*10000+(msg[13]-0x30)*1000+(msg[14]-0x30)*100+(msg[15]-0x30)*10+(msg[16]-0x30);
			sendvm(6,write_defp,def_pressure);
		}	
		
		msg=strstr((const char *)buf,"motor_rs:");		
		if(msg[0]=='m')
		{
			motor_rs=		(msg[9]-0x30)*10000+(msg[10]-0x30)*1000+(msg[11]-0x30)*100+(msg[12]-0x30)*10+(msg[13]-0x30);
			if(motor_rs==1)Y1=1;//sendvm(6,motor,run);
			if(motor_rs==0)Y1=0;//sendvm(6,motor,stop);
			delay_ms(60);
			comClearRxFifo(COM4);
		}	
		Upset();
	}

	memset(buf,0,200);
	memset(c,0,200);
}

void 	UpyunWF_task(void *pdata)		//上传云任务 WIFI
{
	u32 i;u8 buf[50];u8 error=0;
	do	
	{
		Emw3060_init();
	}while(!Emw3060_con());
	Upset();
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
		if(((++i)/250)>=up_t1){i=0;Uptoaliyun_wifi();}
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

void 	Converter_task(void *pdata)		//读水表
{
	u8 *rs485buf;
	u8 buf[200];
	rs485buf=buf;
	while(1)
	{
		delay_ms(600);
		sendwater(tab_water);	//读水表
		delay_ms(200);
		while(!pdcmd(rs485buf));
		memset(buf,0,200);
	} 
}

void 	VM_task(void *pdata)			//变频器
{
	while(1)
	{
		delay_ms(2000);
		do	//读频率 电压 电流
		{
			sendvm(3,readfrequency,4);
			delay_ms(200);
		}while(!readvm());
		do	//读故障
		{
			sendvm(3,readfault,1);
			delay_ms(200);
		}while(!readvmf(&PAW.fault));
		do	//读电机状态
		{
			sendvm(3,readmotor,1);
			delay_ms(200);
		}
		while(!readvmf(&PAW.motorstate));
		do	//读电机当前压力
		{
			sendvm(3,readp,1);
			delay_ms(200);
		}
		while(!readvmf(&PAW.pressure));
	} 
}
