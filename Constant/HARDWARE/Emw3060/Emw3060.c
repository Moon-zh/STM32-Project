#include "Rs485.h"
#include <string.h>
#include "delay.h"
#include "sys.h"
#include "Emw3060.h"
#include "ipsec/md5.h"

unsigned char Emwled=0;

//鉴权信息
char ProductKey1[20]=	"a1ffIehmLRL";
char DeviceName1[50]=	"hxgs";
char DeviceSecret1[50]=	"XOJ35jflQg2FpkBVnHIsgAOSsitwRFoE";

char *ProductKeyw=ProductKey1;
char *DeviceNamew=DeviceName1;
char *DeviceSecretw=DeviceSecret1;

char ssid[20]=		"CU_Z4eg";
char password[20]=	"z4egdkuj";

extern uint8_t g_RxBuf2[UART2_RX_BUF_SIZE];

void	Emw3060_LED_Init()				//EMWLED初始化
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PB,PE端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_0;	 //LED0-->PB.5 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5
}

void	md5tohex(char *data,char *str)	//分解字节中包含的两个字符串值
{
	u8 i;
	for(i=0;i<16;i++)sprintf(str+i*2,"%02x",data[i]);
}

void	hmacmd5(char *data,u8 size_data,u8 size_ds,char *str)	//hmacmd5计算
{
	char buf[33];
	hmac_md5((unsigned char*)data,size_data,(unsigned char*)DeviceSecretw,size_ds,(unsigned char*)str);
	md5tohex(str,buf);
	sprintf(str,"%s",buf);
}

void	Emw3060_init(void)				//EMW初始化
{
	u8 buf[250];
	Emw3060_LED_Init();
	Emwled=1;
	while(1)
	{
		printf_num=2;
		printf("+++");
		delay_ms(200);
		COM2GetBuf(buf,10);
		if(strchr((const char *)buf,'+')[0]=='+')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		printf_num=2;
		printf("AT+FACTORY\r");
		delay_ms(200);
		COM2GetBuf(buf,100);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		printf_num=2;
		printf("+++");
		delay_ms(500);
		COM2GetBuf(buf,10);
		if(strchr((const char *)buf,'+')[0]=='+')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	printf_num=2;
	printf("AT+WJAP=%s,%s\r",ssid,password);
	delay_ms(2000);
	printf_num=2;
	printf("AT+WJAP=%s,%s\r",ssid,password);
	while(1)
	{
		delay_ms(2000);
		COM2GetBuf(buf,200);
		if(strstr((const char *)buf,"STATION_UP")[0]=='S')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
}

void	Emw3060_con(void)				//EMW连接阿里云
{
	u8 buf[250];
	char hc[100];
	char cs[40];
	Emwled=2;
	ProductKeyw=ProductKey1;
	DeviceNamew=DeviceName1;
	DeviceSecretw=DeviceSecret1;
	sprintf(hc,"clientId123deviceName%sproductKey%stimestamp789",DeviceNamew,ProductKeyw);
	while(1)
	{
		printf_num=2;
		hmacmd5(hc,strlen(hc),strlen(DeviceSecretw),cs);
		printf("AT+MQTTAUTH=%s&%s,%s\r",DeviceNamew,ProductKeyw,cs);
		delay_ms(200);
		COM2GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		printf_num=2;
		printf("AT+MQTTCID=123|securemode=3\\,signmethod=hmacmd5\\,timestamp=789|\r");
		delay_ms(200);
		COM2GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		printf_num=2;
		printf("AT+MQTTSOCK=%s.iot-as-mqtt.cn-shanghai.aliyuncs.com,1883\r",ProductKeyw);
		delay_ms(200);
		COM2GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		printf_num=2;
		printf("AT+MQTTKEEPALIVE=60\r");
		delay_ms(200);
		COM2GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		printf_num=2;
		printf("AT+MQTTRECONN=ON\r");
		delay_ms(200);
		COM2GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		printf_num=2;
		printf("AT+MQTTSTART\r");
		delay_ms(200);
		COM2GetBuf(buf,200);
		if(strstr((const char *)buf,"MQTTEVENT:CONNECT,SUCCESS")[0]=='M')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		printf_num=2;
		printf("AT+MQTTSUB=0,/%s/%s/user/get,1\r",ProductKeyw,DeviceNamew);
		delay_ms(200);
		COM2GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	memset(g_RxBuf2,0,UART2_RX_BUF_SIZE);
	Emwled=3;
}

u8	sendEmw(char *data,unsigned char w)	//EMW上报数据
{
	static u8 buf[201];
	char hc[500];
	u16 len;
	Emwled=2;
	ProductKeyw=ProductKey1;
	DeviceNamew=DeviceName1;
	DeviceSecretw=DeviceSecret1;
	memset(buf,0,sizeof buf);
	comClearRxFifo(COM2);
	while(1)
	{
		printf_num=2;
		if(!w)
			printf("AT+MQTTPUB=/sys/%s/%s/thing/event/property/post,1\r",ProductKeyw,DeviceNamew);
		else printf("AT+MQTTPUB=/%s/%s/user/war,1\r",ProductKeyw,DeviceNamew);
		delay_ms(1200);
		COM2GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		printf_num=2;
		printf("AT+MQTTSEND=%d\r",strlen(data)+13);
		delay_ms(200);
		COM2GetBuf(buf,10);
		if(strstr((const char *)buf,">")[0]=='>')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	sprintf(hc,"{\"params\":{%s}}",data);
	len=strlen(hc);
	if(hc[len-3]==',')hc[len-3]=' ';
	printf_num=2;
	printf("%s",hc);
	
	delay_ms(2000);
	COM2GetBuf(buf,100);
	Emwled=3;
	comClearRxFifo(COM2);
	if(strstr((const char *)buf,"+MQTTEVENT:PUBLISH,SUCCESS")[0]=='+')return 1;
	else return 0;
}
