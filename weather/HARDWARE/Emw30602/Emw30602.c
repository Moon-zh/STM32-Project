#include "Rs485.h"
#include <string.h>
#include "delay.h"
#include "sys.h"
#include "Emw30602.h"
#include "ipsec/md5.h"

unsigned char Emwled2=0;

//鉴权信息
char ProductKey0[20]=	"a1SThr9lW2z";
char DeviceName0[50]=	"ceshi";
char DeviceSecret0[50]=	"Yc8OxRDeowHtL9dPziGsYypV71BkRJQu";

char *ProductKey=ProductKey0;
char *DeviceName=DeviceName0;
char *DeviceSecret=DeviceSecret0;

char ssid2[20]=		"tutou";
char password2[20]=	"12345678";

extern uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];

void	Emw3060_LED_Init2()				//EMWLED初始化
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PB,PE端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_12;	 //LED0-->PB.5 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5
}

void	md5tohex2(char *data,char *str)	//分解字节中包含的两个字符串值
{
	u8 i;
	for(i=0;i<16;i++)sprintf(str+i*2,"%02x",data[i]);
}

void	hmacmd52(char *data,u8 size_data,u8 size_ds,char *str)	//hmacmd5计算
{
	char buf[33];
	hmac_md5((unsigned char*)data,size_data,(unsigned char*)DeviceSecret,size_ds,(unsigned char*)str);
	md5tohex2(str,buf);
	sprintf(str,"%s",buf);
}

void	Emw3060_init2(void)				//EMW初始化
{
	u8 buf[250];
	Emw3060_LED_Init2();
	Emwled2=1;
	while(1)
	{
		printf_num=1;
		printf("+++");
		delay_ms(200);
		COM1GetBuf(buf,10);
		if(strchr((const char *)buf,'+')[0]=='+')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		printf_num=1;
		printf("AT+FACTORY\r");
		delay_ms(200);
		COM1GetBuf(buf,100);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		printf_num=1;
		printf("+++");
		delay_ms(500);
		COM1GetBuf(buf,10);
		if(strchr((const char *)buf,'+')[0]=='+')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	printf_num=1;
	printf("AT+WJAP=%s,%s\r",ssid,password);
	delay_ms(2000);
	printf_num=1;
	printf("AT+WJAP=%s,%s\r",ssid,password);
	while(1)
	{
		
		delay_ms(2000);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"STATION_UP")[0]=='S')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
}

void	Emw3060_con2(void)				//EMW连接阿里云
{
	u8 buf[250];
	char hc[100];
	char cs[40];
	Emwled2=2;
	ProductKey=ProductKey0;
	DeviceName=DeviceName0;
	DeviceSecret=DeviceSecret0;
	sprintf(hc,"clientId123deviceName%sproductKey%stimestamp789",DeviceName,ProductKey);
	while(1)
	{
		printf_num=1;
		hmacmd52(hc,strlen(hc),strlen(DeviceSecret),cs);
		printf("AT+MQTTAUTH=%s&%s,%s\r",DeviceName,ProductKey,cs);
		delay_ms(200);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		printf_num=1;
		printf("AT+MQTTCID=123|securemode=3\\,signmethod=hmacmd5\\,timestamp=789|\r");
		delay_ms(200);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		printf_num=1;
		printf("AT+MQTTSOCK=%s.iot-as-mqtt.cn-shanghai.aliyuncs.com,1883\r",ProductKey);
		delay_ms(200);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		printf_num=1;
		printf("AT+MQTTKEEPALIVE=60\r");
		delay_ms(200);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		printf_num=1;
		printf("AT+MQTTRECONN=ON\r");
		delay_ms(200);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		printf_num=1;
		printf("AT+MQTTSTART\r");
		delay_ms(200);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"MQTTEVENT:CONNECT,SUCCESS")[0]=='M')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		printf_num=1;
		printf("AT+MQTTSUB=0,/%s/%s/user/get,1\r",ProductKey,DeviceName);
		delay_ms(200);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	memset(g_RxBuf1,0,UART2_RX_BUF_SIZE);
	Emwled2=3;
}

u8	sendEmw2(char *data,unsigned char w)	//EMW上报数据
{
	static u8 buf[201];
	char hc[500];
	u16 len;
	Emwled2=2;
	ProductKey=ProductKey0;
	DeviceName=DeviceName0;
	DeviceSecret=DeviceSecret0;
	memset(buf,0,sizeof buf);
	comClearRxFifo(COM1);
	while(1)
	{
		printf_num=1;
		if(!w)
			printf("AT+MQTTPUB=/sys/%s/%s/thing/event/property/post,1\r",ProductKey,DeviceName);
		else printf("AT+MQTTPUB=/%s/%s/user/war,1\r",ProductKey,DeviceName);
		delay_ms(1200);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		printf_num=1;
		printf("AT+MQTTSEND=%d\r",strlen(data)+13);
		delay_ms(200);
		COM1GetBuf(buf,10);
		if(strstr((const char *)buf,">")[0]=='>')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	sprintf(hc,"{\"params\":{%s}}",data);
	len=strlen(hc);
	if(hc[len-3]==',')hc[len-3]=' ';
	printf_num=1;
	printf("%s",hc);
	
	delay_ms(2000);
	COM1GetBuf(buf,100);
	Emwled2=3;
	comClearRxFifo(COM1);
	if(strstr((const char *)buf,"+MQTTEVENT:PUBLISH,SUCCESS")[0]=='+')return 1;
	else return 0;
}
