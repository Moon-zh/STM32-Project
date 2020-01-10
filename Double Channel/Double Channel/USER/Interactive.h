#ifndef __inter_H
#define __inter_H
#include "sys.h"
#include "main.h"
void	Uptoaliyun_wifi_Env(Environmental data,u8 group)	//上传传感器数值到阿里云
{
	char a[250];
	char c[250];
	char *b=c;
	u8 i;i=0;b=c;
	memset(a,0,sizeof a);
	sprintf(b,"\"airtemp_%d\":%2.1f,",group+1,(float)(data.airtemp/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"airhumi_%d\":%2.1f,",group+1,(float)(data.airhumi/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"CO2_%d\":%d,",group+1,data.CO2);								for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"light_%d\":%d,",group+1,data.light);							for(;*b;b++)a[i++]=*b;
	
	sprintf(b,"\"soiltemp_%d\":%2.1f,",group+1,(float)(data.soiltemp/10.0));	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"soilhumi_%d\":%2.1f,",group+1,(float)(data.soilhumi/10.0));	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"EC_%d\":%d",group+1,data.EC);									for(;*b;b++)a[i++]=*b;
	
	sendEmw(a,0);
}

void	Uptoaliyun_wifi_State()			//上传当前状态到阿里云
{
	char a[250];
	char c[250];
	char *b=c;
	u8 data;
	u8 i;i=0;b=c;
	memset(a,0,sizeof a);
	sprintf(b,"\"FerState\":%d,",(int)FerState);			for(;*b;b++)a[i++]=*b;	//吸肥状态
	if(StirState1)data=1;else data=0;
	sprintf(b,"\"StirState1\":%d,",data);					for(;*b;b++)a[i++]=*b;	//搅拌1
	if(StirState2)data=1;else data=0;	
	sprintf(b,"\"StirState2\":%d,",data);					for(;*b;b++)a[i++]=*b;	//搅拌2
	if(SolenoidState1)data=1;else data=0;
	sprintf(b,"\"Partition1\":%d,",data);					for(;*b;b++)a[i++]=*b;	//电磁阀1
	if(SolenoidState2)data=1;else data=0;
	sprintf(b,"\"Partition2\":%d,",data);					for(;*b;b++)a[i++]=*b;	//电磁阀2
	sendEmw(a,0);
	
	if(MODEL==ManualModel)return;

	i=0;b=c;
	memset(a,0,sizeof a);
	if(IrrMethod)data=1;else data=0;
	sprintf(b,"\"IrrMethod\":%d,",data);					for(;*b;b++)a[i++]=*b;	//灌溉方式
	sprintf(b,"\"Current\":%d,",Current);					for(;*b;b++)a[i++]=*b;	//当前灌溉分区
	sprintf(b,"\"IrrTime\":%d,",IrrTime);					for(;*b;b++)a[i++]=*b;	//灌溉时间
	if(RunState)data=1;else data=0;
	sprintf(b,"\"State\":%d,",data);						for(;*b;b++)a[i++]=*b;	//运行状态
	sendEmw(a,0);
}

void	readset()						//解析平台下发的指令
{
	u8 Method=0,Partition=0;
	cJSON *json;
	char *jsa;
	if(emw_set)return;
	Method=MCGS_Button;
	Partition=MCGS_Partition;
	if(strstr((const char *)g_RxBuf1,"RECV")[0]!='R')return;
	else
	{
		delay_ms(200);
		json=cJSON_Parse(strstr((const char *)g_RxBuf1,"{"));
		atoi(jsa=cJSON_Print(cJSON_GetObjectItem(json,"IrrMethod")))?Method|=0x02:(Method&=0xFD);
		atoi(jsa=cJSON_Print(cJSON_GetObjectItem(json,"Partition1")))?Partition|=0x01:(Partition&=0xFE);
		atoi(jsa=cJSON_Print(cJSON_GetObjectItem(json,"Partition2")))?Partition|=0x02:(Partition&=0xFD);
		atoi(jsa=cJSON_Print(cJSON_GetObjectItem(json,"Partition3")))?Partition|=0x04:(Partition&=0xFB);
		atoi(jsa=cJSON_Print(cJSON_GetObjectItem(json,"Partition4")))?Partition|=0x08:(Partition&=0xF7);
		atoi(jsa=cJSON_Print(cJSON_GetObjectItem(json,"Partition5")))?Partition|=0x10:(Partition&=0xEF);
		atoi(jsa=cJSON_Print(cJSON_GetObjectItem(json,"Partition6")))?Partition|=0x20:(Partition&=0xDF);
		IrrTime=atoi(jsa=cJSON_Print(cJSON_GetObjectItem(json,"IrrTime")));
		atoi(jsa=cJSON_Print(cJSON_GetObjectItem(json,"State")))?Method|=(0x20|0x08):(Method&=(0xDF&0xF7));
		cJSON_Delete(json);
		cJSON_free(jsa);
		
		if(PlanButton)goto cc;						//计划进行中不接受指令
		if(RunState){if((Method&0x08))goto cc;}		//当前在运行中不接收新计划指令
		if(IrrTime<21)goto cc;						//设置时间小于21分钟不执行
		if(Partition<1)goto cc;						//最少开启一个分区
		HC_Partition=Partition;
		HC_IrrMode=Method;
		Net=1;
		ChoMode=2;
		SetRun=1;									//重载相关运行参数
		while(SetRun)delay_ms(200);
cc:		memset(g_RxBuf1,0,UART1_RX_BUF_SIZE);
		comClearRxFifo(COM1);
	}
}
#endif
