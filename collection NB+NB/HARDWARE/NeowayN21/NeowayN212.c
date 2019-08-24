#include "Rs485.h"
#include "delay.h"
#include "sys.h"
#include "NeowayN212.h"

//鉴权信息
char ProductKey1[20]=	"a1TymkIzezE";
char DeviceName1[50]=	"hardware";
char DeviceSecret1[50]=	"a0p0r0g833gUsbvZWqAycN7BrSnbRqXr";

extern u8 Nsendok;

char CCID2[25];

void	RESET_N212()						//N21初始化
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PB,PE端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;				 //LED0-->PB.5 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5
}

void	NeoWayN21_init2()				//有方N21初始化
{
	u8 buf[250];
	u8 num,num1;
	char *msg;
	RESET_N212();
	GPIO_SetBits(GPIOA,GPIO_Pin_1);
	delay_ms(1000);
	GPIO_ResetBits(GPIOA,GPIO_Pin_1);
	delay_ms(1000);
	do
	{
		printf_num=2;
		printf("AT\r\n");
		delay_ms(500);
		COM2GetBuf(buf,200);
		if(strchr((const char *)buf,'K')[0]=='K')break;
	}
	while(1);
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	printf_num=2;
	printf("AT&F\r\n");
	delay_ms(500);
	comClearRxFifo(COM2);
	
	do
	{
		printf_num=2;
		printf("AT+CCID\r\n");
		delay_ms(500);
		COM2GetBuf(buf,200);
		if(strstr((const char *)buf,"+CCID:")[0]=='+')break;
	}
	while(1);
	msg=strstr((const char *)buf,"+CCID:")+7;
	for(num=0;*msg!=0x0d;msg++)CCID2[num++]=*msg;
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	do
	{
		printf_num=2;
		printf("AT+NVSETBAND=1,8\r\n");
		delay_ms(500);
		COM2GetBuf(buf,200);
		if(strchr((const char *)buf,'K')[0]=='K')break;
	}
	while(1);
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
//	do
//	{
//		printf_num=2;
//		printf("AT+CGDCONT=1,\"IP\",\"snbiot\"\r\n");
//		delay_ms(500);
//		COM2GetBuf(buf,200);
//		if(strchr((const char *)buf,'K')[0]=='K')break;
//	}
//	while(1);
//	comClearRxFifo(COM2);
//	memset(buf,0,sizeof buf);
	
	do
	{
		printf_num=2;
		printf("AT+CSQ\r\n");
		delay_ms(500);
		COM2GetBuf(buf,200);
		num=strchr((const char *)buf,':')[2]-0x30;
		num=num*10+(strchr((const char *)buf,':')[3]-0x30);
		num1=strchr((const char *)buf,':')[5]-0x30;
		num1=num1*10+(strchr((const char *)buf,':')[6]-0x30);
		if((num!=99)&(num1==99))break;
		comClearRxFifo(COM2);
	}
	while(1);
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	do
	{
		printf_num=2;
		printf("AT+CREG?\r\n");
		delay_ms(500);
		COM2GetBuf(buf,200);
		if(strchr((const char *)buf,'0')[2]=='1')break;
	}
	while(1);
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	do
	{
		printf_num=2;
		printf("AT+XIIC=1\r\n");
		delay_ms(2000);
		COM2GetBuf(buf,200);
		if(strchr((const char *)buf,'K')[0]=='K')
		{
			if(strchr((const char *)buf,'E')[0]=='E')
			{
				comClearRxFifo(COM2);
				memset(buf,0,sizeof buf);
				continue;
			}
			break;
		}	
	}
	while(1);
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	do
	{
		printf_num=2;
		printf("AT+XIIC?\r\n");
		delay_ms(500);
		COM2GetBuf(buf,200);
		if(strchr((const char *)buf,',')[1]!='0')break;
	}
	while(1);
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	delay_ms(500);
}

void	conN212()						//N21连接到阿里云
{
	u8 i,k;
	u8 buf[250];
	i=k=0;
	do			//设备鉴权
	{
		i=0;
		printf_num=2;
		printf("AT+IMQTTAUTH=\"%s\",\"%s\",\"%s\"\r\n",ProductKey1,DeviceName1,DeviceSecret1);
		do{
			COM2GetBuf(buf,100);
			delay_ms(500);
			if(strstr((const char *)buf,"+IMQTTAUTH:")[0]=='+')break;
			if(++i==40)break;
		}while(1);
		
		if(strstr((const char *)buf,"+IMQTTAUTH:OK")[0]=='+')break;
		memset(buf,0,sizeof buf);
		comClearRxFifo(COM2);
		if(++k==5){Nsendok=0;return;}
	}
	while(1);
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	do
	{
		printf_num=2;
		printf("AT+IMQTTPARA=\"TIMEOUT\",10,\"CLEAN\",1,\"KEEPALIVE\",180,\"VERSION\",\"3.1\"\r\n");
		delay_ms(1000);
		COM2GetBuf(buf,200);
		if(strchr((const char *)buf,'K')[0]=='K')break;
	}
	while(1);
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	do			//设备连接
	{
		i=0;
		printf_num=2;
		printf("AT+IMQTTCONN\r\n");
		do
		{
			COM2GetBuf(buf,200);
			delay_ms(500);
			if(strchr((const char *)buf,'K')[0]=='K')break;
			if(strchr((const char *)buf,'E')[0]=='E')break;
			if(++i==40)break;
	
		}while(1);
		comClearRxFifo(COM2);
		if(strchr((const char *)buf,'K')[0]=='K')break;
		memset(buf,0,sizeof buf);
		delay_ms(1000);
	}
	while(1);
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	 
	do
	{
		printf_num=2;
		printf("AT+IMQTTSUB=\"/%s/%s/user/get\",1\r\n",ProductKey1,DeviceName1);
		delay_ms(1000);
		COM2GetBuf(buf,200);
		if(strchr((const char *)buf,'K')[0]=='K')break;
	}
	while(1);
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	Nsendok=1;
}

void	disconN212()						//断开N21连接
{
	u8 buf[250];
	do
	{
		printf_num=2;
		printf("AT+DISIMQTTCONN\r\n");
		COM2GetBuf(buf,200);
		delay_ms(500);
		if(strchr((const char *)buf,'K')[0]=='K')break;
		if(strchr((const char *)buf,'E')[0]=='E')break;
	}while(1);
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	delay_ms(1000);
}

u8	sendN212(char *data,u8 w)			//向订阅的topic发送数据
{
	char hc[500];u16 len;u8 buf[350];

	if(!w)sprintf(hc,"AT+IMQTTPUB=\"/sys/%s/%s/thing/event/property/post\",1,\"{\\\"params\\\":{%s}}\"\r\n",ProductKey1,DeviceName1,data);
	else sprintf(hc,"AT+IMQTTPUB=\"/%s/%s/user/war\",1,\"{\\\"params\\\":{%s}}\"\r\n",ProductKey1,DeviceName1,data);
	len=strlen(hc);						//删除上报报警值时最后一个逗号
	if(hc[len-6]==',')hc[len-6]=' ';
	printf_num=2;
	printf("%s",hc);
	delay_ms(2000);
	COM2GetBuf(buf,330);
	comClearRxFifo(COM2);
	if(strstr((const char *)buf,"+CME")[0]=='+')return 0;	//判断是否上报正常
	else return 1;
}
