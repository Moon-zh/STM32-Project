#include "Rs485.h"
#include <string.h>
#include "delay.h"
#include "sys.h"
#include "Emw3060.h"
#include "md5.h"
#include "UserCore.h"
#include "UserHmi.h"
#include "Alarm.h"
#include "FlashDivide.h"


//鉴权信息
 char ProductKey0[20]=	"a1gWvFoNbGW";//"a1q04qjosGa";
 char DeviceName0[50]=	"zbzfltkzx";//"test_single";
 char DeviceSecret0[50]="VvG2aUXJ7vcmx7LahDUhmN9Sk9RrLoCN";	//"78eJYG6R0Y9AlPcpA9s4KXAUd3rjcQDn";

 char *ProductKey=ProductKey0;
 char *DeviceName=DeviceName0;
 char *DeviceSecret=DeviceSecret0;
 
 char ssid[31]="CU_Z4eg";
 char password[31]="z4egdkuj";
 char ipaddr[16]="";
 char subnet[16]="";
 char gateway[16]="";
 
Ring NETCircle;

OS_EVENT * RemoteQMsg;
void* 	RemoteMsgBlock[8];
OS_MEM* RemotePartitionPt;
u8 g_u8RemoteMsgMem[20][8];


//创建消息队列和内存块
//输出0：创建正常
//    1：消息队列创建失败
//	  2：内存块创建失败
u8 RemoteQInit(void)
{
	INT8U os_err;
	
	RemoteQMsg = OSQCreate ( RemoteMsgBlock, 8);
	
	if(RemoteQMsg == (OS_EVENT *)0)
	{
		return 1;
	}
	
	RemotePartitionPt = OSMemCreate (
										g_u8RemoteMsgMem,
				  						20,
				  						8,
				  						&os_err 
				  					);
	if(os_err != OS_ERR_NONE)
	{
		return 2;
	}
	return 0;
}
//消息打包发走
//输入：消息信息
//输出：0：发送成功
//		1：申请内存失败
//		2：发送消息失败

u8 PackSendRemoteQ(MsgStruct* MsgBlk)
{
	INT8U os_err;
	MsgStruct * MsgTemp = NULL;
	MsgTemp = (MsgStruct *)OSMemGet(RemotePartitionPt,&os_err);
	if(MsgTemp == NULL)
	{
		return 1;
	}
	MsgTemp ->CmdType = MsgBlk->CmdType;
	MsgTemp ->CmdSrc = MsgBlk->CmdSrc;
	MsgTemp ->CmdData[0] = MsgBlk->CmdData[0];
	MsgTemp ->CmdData[1] = MsgBlk->CmdData[1];
	//MsgTemp ->CmdData[2] = MsgBlk->CmdData[2];
	//MsgTemp ->CmdData[3] = MsgBlk->CmdData[3];
	os_err = OSQPost ( RemoteQMsg,(void*)MsgTemp );
	//发送消息失败释放内存
	if(os_err!=OS_ERR_NONE)
	{
		OSMemPut(RemotePartitionPt, ( void * )MsgTemp);
		return 2;
	}
	return 0;
}
//队列消息解析，消息内存释放
//输入：收到的消息队列指针
//输出：0：释放成功
//		1：释放失败
u8 DepackReceiveRemoteQ(MsgStruct * RemoteQ)
{
	u8 os_err;
	os_err = OSMemPut(RemotePartitionPt, ( void * )RemoteQ);
	if(os_err != OS_ERR_NONE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


//void GM3_check_cmd(u8*str)
//GM3发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//其他,期待应答结果的位置(str的位置)
u8* EMW3060_check_cmd(u8 *str)
{
	char *strx=0;
	strx = strstr((const char*)NETCircle.buf,(const char*)str);
	return (u8*)strx;
}
//gm3发送命令
//cmd:发送的命令字符串(不需要添加回车了),当cmd<0XFF的时候,发送数字(比如发送0X1A),大于的时候发送字符串.
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
u8 EMW3060_send_cmd(u8 *cmd,u8 *ack,u16 waittime, u8 flag)
{
	u8 res=0; 
	u8*p = NULL;
//	u8 uCnt = 0;

	if(flag == 1)//
	{
		u3_printf("%s\r\n",cmd);//发送命令
	}
	else if(flag == 0)
	{
		u3_printf("%s",cmd);//发送命令
	}
	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{ 
			delay_ms(10);
			//串口收到新的数据，并判断是否有为有效数据
			COM1GetBuf(NETCircle.buf, 512);
			{
				p = EMW3060_check_cmd(ack);
				if(p)
				{
					memset(&NETCircle, 0 , sizeof(NETCircle));
					//*p = 1;//破坏字符串
					break;//得到有效数据 
				}
			}
		}
		if(waittime==0)res=1; 
	}
	return res;
} 
s16 Remote_Para[4];
s16 Remote_Read[36];
void SetRemoteRead(u8 addr,s16 value)
{
	Remote_Read[addr] = value;
}
void SetRemotePara(u8 addr,s16 value)
{
	Remote_Para[addr] = value;
}
void SetMulRemoteRead(u8 addr,s16* Buf,u8 Num)
{
	memcpy(&Remote_Para[addr],Buf,Num);
}
const u8 ABucketLevelStr[13]="ABucketLevel\0";
const u8 BBucketLevelStr[13]="BBucketLevel\0";
const u8 CBucketLevelStr[13]="CBucketLevel\0";
const u8 DBucketLevelStr[13]="DBucketLevel\0";

const u8 ABucketDiameterStr[16]="ABucketDiameter\0";
const u8 BBucketDiameterStr[16]="BBucketDiameter\0";
const u8 CBucketDiameterStr[16]="CBucketDiameter\0";
const u8 DBucketDiameterStr[16]="DBucketDiameter\0";

const u8 ABucketPercentStr[15]="ABucketPercent\0";
const u8 BBucketPercentStr[15]="BBucketPercent\0";
const u8 CBucketPercentStr[15]="CBucketPercent\0";
const u8 DBucketPercentStr[15]="DBucketPercent\0";

const u8 ABucketUpperStr[13]="ABucketUpper\0";
const u8 BBucketUpperStr[13]="BBucketUpper\0";
const u8 CBucketUpperStr[13]="CBucketUpper\0";
const u8 DBucketUpperStr[13]="DBucketUpper\0";

const u8 ABucketLowerStr[13]="ABucketLower\0";
const u8 BBucketLowerStr[13]="BBucketLower\0";
const u8 CBucketLowerStr[13]="CBucketLower\0";
const u8 DBucketLowerStr[13]="DBucketLower\0";

const u8 ABucketStirTimeStr[16]="ABucketStirTime\0";
const u8 BBucketStirTimeStr[16]="BBucketStirTime\0";
const u8 CBucketStirTimeStr[16]="CBucketStirTime\0";
const u8 DBucketStirTimeStr[16]="DBucketStirTime\0";

const u8 ABucketWaterSwitchStr[19]="ABucketWaterSwitch\0";
const u8 BBucketWaterSwitchStr[19]="BBucketWaterSwitch\0";
const u8 CBucketWaterSwitchStr[19]="CBucketWaterSwitch\0";
const u8 DBucketWaterSwitchStr[19]="DBucketWaterSwitch\0";

const u8 ABucketStirSwitchStr[18]="ABucketStirSwitch\0";
const u8 BBucketStirSwitchStr[18]="BBucketStirSwitch\0";
const u8 CBucketStirSwitchStr[18]="CBucketStirSwitch\0";
const u8 DBucketStirSwitchStr[18]="DBucketStirSwitch\0";

const u8 AStirCountDownStr[15]="AStirCountDown\0";
const u8 BStirCountDownStr[15]="BStirCountDown\0";
const u8 CStirCountDownStr[15]="CStirCountDown\0";
const u8 DStirCountDownStr[15]="DStirCountDown\0";

u8 CCIDstr[21];
const u8* SetText[] = 
{
	"0"
};
const u8* ReadText[] = 
{
	ABucketLevelStr,
	BBucketLevelStr,
	CBucketLevelStr,
	DBucketLevelStr,
	
	ABucketDiameterStr,
	BBucketDiameterStr,
	CBucketDiameterStr,
	DBucketDiameterStr,
	
	ABucketPercentStr,
	BBucketPercentStr,
	CBucketPercentStr,
	DBucketPercentStr,

	ABucketUpperStr,
	BBucketUpperStr,
	CBucketUpperStr,
	DBucketUpperStr,

	ABucketLowerStr,
	BBucketLowerStr,
	CBucketLowerStr,
	DBucketLowerStr,

	ABucketStirTimeStr,
	BBucketStirTimeStr,
	CBucketStirTimeStr,
	DBucketStirTimeStr,

	ABucketWaterSwitchStr,
	BBucketWaterSwitchStr,
	CBucketWaterSwitchStr,
	DBucketWaterSwitchStr,

	ABucketStirSwitchStr,
	BBucketStirSwitchStr,
	CBucketStirSwitchStr,
	DBucketStirSwitchStr,

	AStirCountDownStr,
	BStirCountDownStr,
	CStirCountDownStr,
	DStirCountDownStr,
	
	CCIDstr
};
s16 RevFigure(u8*src)
{
	u8 u8F = 0;
	s16 revalue=0;

	if(*src==0x2d)
	{
		u8F = 1;
		src++;
	}
	while((*src>=0x30)&&(*src<=0x39))
	{
		revalue = revalue*10+ (*src-0x30);
		src++;
	}
	if(u8F == 1)
	{
		revalue = revalue*(-1);
	}
	return revalue;
}
void DepackRevData(void)
{
	u8* p=NULL;
	u8 u8Cnt = 0;
	MsgStruct Msgtemp;
	//u16 RevNum=0;
	
	p=EMW3060_check_cmd("+MQTTRECV");
	if(p)
	{
		for(u8Cnt=0;u8Cnt<ParaNum;u8Cnt++)
		{
			//p=EMW3060_check_cmd((u8*)SetText[u8Cnt]);
			
			if(p)
			{
				*p = 1;
				//Remote_Para[u8Cnt] = RevFigure(p+strlen((const char*)SetText[u8Cnt])+2);
			}
		}
	}
	if(Remote_Para[3]==1)
	{
		Msgtemp.CmdSrc = SCREEN_TASK_CODE;
	//	Msgtemp.CmdType = MSG_START;
		Msgtemp.CmdData[0] = Remote_Para[0];
		Msgtemp.CmdData[1] = Remote_Para[2];
//		Msgtemp.CmdData[2] = Remote_Para[1]/60;
//		Msgtemp.CmdData[3] = Remote_Para[1]%60;
//		PackSendMasterQ(&Msgtemp);
		//SetScreen(LCD_STATESHOW_PAGE);
	}
	else if(Remote_Para[3]==0)
	{
		//发送任务停止消息
		Msgtemp.CmdSrc = SCREEN_TASK_CODE;
	//	Msgtemp.CmdType = MSG_STOP;
//		PackSendMasterQ(&Msgtemp);
	}
}
char xxbuf[34];

char	*hmacmd5(char *data,u8 size_data,u8 size_ds)
{
	char buf[20];
	char i=0,L=0;
	char *str=buf;
	memset(buf,0,20);
	hmac_md5((unsigned char*)data,size_data,(unsigned char*)DeviceSecret,size_ds,(unsigned char*)str);
	for(i=0;i<16;i++)
	{
		L = strlen(xxbuf);
		sprintf(xxbuf+L,"%02x",buf[i]);
	}
	str = xxbuf;
	return str;
}

void	Emw3060_init(void)
{
	u8 buf[250];
	delay_ms(1000);
	while(1)
	{
		u3_printf("AT+FACTORY\r");
		delay_ms(1000);
		COM1GetBuf(buf,100);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
		
	}
	while(1)
	{
		u3_printf("+++");
		delay_ms(500);
		COM1GetBuf(buf,10);
		if(strchr((const char *)buf,'+')[0]=='+')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		u3_printf("AT+WJAP=%s,%s\r",ssid,password);
		delay_ms(2000);delay_ms(2000);delay_ms(2000);delay_ms(2000);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"+WEVENT:STATION_UP")[0]=='+')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
}

void	Emw3060_con(void)
{
	u8 buf[250];
	char hc[100];
	u8 Flag=0;
	memset(hc,0,100);
	sprintf(hc,"clientId123deviceName%sproductKey%stimestamp789",DeviceName,ProductKey);
	while(1)
	{
//		printf("AT+MQTTAUTH=%s&%s,%s\r",DeviceName,ProductKey,DeviceSecret);
		u3_printf("AT+MQTTAUTH=%s&%s,%s\r",DeviceName,ProductKey,hmacmd5(hc,strlen(hc),strlen(DeviceSecret)));
		//u3_printf("AT+MQTTAUTH=%s&%s,%s\r",DeviceName,ProductKey,"ff0facf561b96759dc44eea0c444100b");
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		u3_printf("AT+MQTTCID=123|securemode=3\\,signmethod=hmacmd5\\,timestamp=789|\r");
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		u3_printf("AT+MQTTSOCK=%s.iot-as-mqtt.cn-shanghai.aliyuncs.com,1883\r",ProductKey);
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		u3_printf("AT+MQTTKEEPALIVE=60\r");
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		u3_printf("AT+MQTTRECONN=ON\r");
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		u3_printf("AT+MQTTSTART\r");
		delay_ms(5000);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"+MQTTEVENT:CONNECT,SUCCESS")[0]=='+')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	while(1)
	{
		u3_printf("AT+MQTTSUB=?\r");
		delay_ms(1000);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"+MQTTSUB:0")[0]=='+')
		{
			Flag = 1;
			break;
		}
		else if(strstr((const char *)buf,"+MQTTSUB:1")[0]=='+')
		{
			//break;
		}
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	if(Flag == 1)
	{
		while(1)
		{
			u3_printf("AT+MQTTSUB=1,/%s/%s/user/irr_set,1\r",ProductKey,DeviceName);
			delay_ms(2000);
			COM1GetBuf(buf,200);
			if(strstr((const char *)buf,"+MQTTEVENT:1,SUBSCRIBE,SUCCESS")[0]=='+')break;
		}
		comClearRxFifo(COM1);
		memset(buf,0,sizeof buf);
	}
	
	while(1)///a1q04qjosGa/test_single/user/update
	{
		u3_printf("AT+MQTTPUB=/sys/%s/%s/thing/event/property/post,1\r",ProductKey,DeviceName);
		//u3_printf("AT+MQTTPUB=/a1q04qjosGa/test_single/user/update,1\r");
		delay_ms(2000);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
}

u8	sendEmw(char *data,unsigned char w)
{
	u8 buf[250];
	while(1)
	{
		if(!w)
			u3_printf("AT+MQTTPUB=/sys/%s/%s/thing/event/property/post,1\r",ProductKey,DeviceName);
		else u3_printf("AT+MQTTPUB=/%s/%s/user/war,1\r",ProductKey,DeviceName);
		delay_ms(100);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		u3_printf("AT+MQTTSEND=%d\r",strlen(data));
		delay_ms(50);
		COM1GetBuf(buf,20);
		if(strstr((const char *)buf,">")[0]=='>')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	u3_printf("%s",data);
	
	delay_ms(2000);
	COM1GetBuf(buf,100);
	if(strstr((const char *)buf,"+MQTTEVENT:PUBLISH,SUCCESS")[0]=='+')return 1;
	else return 0;
}
void SplicingPubHead(u8* Dest)
{
	strcpy((char*)Dest,"{\"params\":{");
}
void SignedNumberToASCII ( u8 *Arry , s16 Number,u8 decimalnum);//有正负的数
void SplicingPubData(u8*Dest,const u8*Text,s16 Value,u8 FirstF)
{
	u8 u8Str[16];
	if(FirstF >0)
	{
		strcat((char*)Dest,",");
	}
	strcat((char*)Dest,"\"");
	strcat((char*)Dest,(const char*)Text);
	strcat((char*)Dest,"\":");
	SignedNumberToASCII(u8Str,Value,0);
	strcat((char*)Dest,(const char*)u8Str);
}
void SplicingPubTail(u8* Dest)
{
	strcat((char*)Dest,"}}");
}

u8 NetSendBuf[512];
void  SplicingPubStr_Level(void)//
{
	u8 u8Cnt=0;
	for(u8Cnt=0;u8Cnt<4;u8Cnt++)
	{
		SetRemoteRead(u8Cnt,LevelData[u8Cnt]/10);
	}
	memset(NetSendBuf,0,512);
	SplicingPubHead(NetSendBuf);
	SplicingPubData(NetSendBuf,ReadText[0],Remote_Read[0],0);
	SplicingPubData(NetSendBuf,ReadText[1],Remote_Read[1],1);
	SplicingPubData(NetSendBuf,ReadText[2],Remote_Read[2],2);
	SplicingPubData(NetSendBuf,ReadText[3],Remote_Read[3],3);
	SplicingPubTail(NetSendBuf);
	//SplicingVer(NetSendBuf);
}
void  SplicingPubStr_Diameter(void)//
{
	u16 Buf[5];
	u8 u8Cnt=0;
	FlashReadD((u8*)Buf);
	for(u8Cnt=4;u8Cnt<8;u8Cnt++)
	{
		SetRemoteRead(u8Cnt,(s16)Buf[u8Cnt-4]);
	}
	memset(NetSendBuf,0,512);
	SplicingPubHead(NetSendBuf);
	SplicingPubData(NetSendBuf,ReadText[4],Remote_Read[4],0);
	SplicingPubData(NetSendBuf,ReadText[5],Remote_Read[5],1);
	SplicingPubData(NetSendBuf,ReadText[6],Remote_Read[6],2);
	SplicingPubData(NetSendBuf,ReadText[7],Remote_Read[7],3);
	SplicingPubTail(NetSendBuf);
	//SplicingVer(NetSendBuf);
}
void  SplicingPubStr_Percent(void)//
{
	u8 Buf[5];
	u8 u8Cnt=0;
	FlashReadPersent(Buf);
	//memcpy(&Remote_Read[8],Buf,4);
	for(u8Cnt=8;u8Cnt<12;u8Cnt++)
	{
		SetRemoteRead(u8Cnt,(s16)Buf[u8Cnt-8]);
	}
	memset(NetSendBuf,0,512);
	SplicingPubHead(NetSendBuf);
	SplicingPubData(NetSendBuf,ReadText[8],Remote_Read[8],0);
	SplicingPubData(NetSendBuf,ReadText[9],Remote_Read[9],1);
	SplicingPubData(NetSendBuf,ReadText[10],Remote_Read[10],2);
	SplicingPubData(NetSendBuf,ReadText[11],Remote_Read[11],3);
	SplicingPubTail(NetSendBuf);
	//SplicingVer(NetSendBuf);
}
void  SplicingPubStr_Upper(void)//
{
	u8 Buf[5];
	u8 u8Cnt=0;
	FlashReadWaterADD(Buf);
	//memcpy(&Remote_Read[12],Buf,4);
	for(u8Cnt=12;u8Cnt<16;u8Cnt++)
	{
		SetRemoteRead(u8Cnt,(s16)Buf[u8Cnt-12]);
	}
	memset(NetSendBuf,0,512);
	SplicingPubHead(NetSendBuf);
	SplicingPubData(NetSendBuf,ReadText[12],Remote_Read[12],0);
	SplicingPubData(NetSendBuf,ReadText[13],Remote_Read[13],1);
	SplicingPubData(NetSendBuf,ReadText[14],Remote_Read[14],2);
	SplicingPubData(NetSendBuf,ReadText[15],Remote_Read[15],3);
	SplicingPubTail(NetSendBuf);
	//SplicingVer(NetSendBuf);
}
void  SplicingPubStr_Lower(void)//
{
	u8 Buf[5];
	u8 u8Cnt=0;
	FlashReadLow(Buf);
	//memcpy(&Remote_Read[16],Buf,4);
	for(u8Cnt=16;u8Cnt<20;u8Cnt++)
	{
		SetRemoteRead(u8Cnt,(s16)Buf[u8Cnt-16]);
	}
	memset(NetSendBuf,0,512);
	SplicingPubHead(NetSendBuf);
	SplicingPubData(NetSendBuf,ReadText[16],Remote_Read[16],0);
	SplicingPubData(NetSendBuf,ReadText[17],Remote_Read[17],1);
	SplicingPubData(NetSendBuf,ReadText[18],Remote_Read[18],2);
	SplicingPubData(NetSendBuf,ReadText[19],Remote_Read[19],3);
	SplicingPubTail(NetSendBuf);
	//SplicingVer(NetSendBuf);
}

void  SplicingPubStr_StirTime(void)//
{
	u8 Buf[5];
	u8 u8Cnt=0;
	FlashReadStir(Buf);
	//memcpy(&Remote_Read[20],Buf,4);
	for(u8Cnt=20;u8Cnt<24;u8Cnt++)
	{
		SetRemoteRead(u8Cnt,(s16)Buf[u8Cnt-20]);
	}
	memset(NetSendBuf,0,512);
	SplicingPubHead(NetSendBuf);
	SplicingPubData(NetSendBuf,ReadText[20],Remote_Read[20],0);
	SplicingPubData(NetSendBuf,ReadText[21],Remote_Read[21],1);
	SplicingPubData(NetSendBuf,ReadText[22],Remote_Read[22],2);
	SplicingPubData(NetSendBuf,ReadText[23],Remote_Read[23],3);
	SplicingPubTail(NetSendBuf);
	//SplicingVer(NetSendBuf);
}
void  SplicingPubStr_WaterSwtitch(void)//
{
	memset(NetSendBuf,0,512);
	SplicingPubHead(NetSendBuf);
	SplicingPubData(NetSendBuf,ReadText[24],Remote_Read[24],0);
	SplicingPubData(NetSendBuf,ReadText[25],Remote_Read[25],1);
	SplicingPubData(NetSendBuf,ReadText[26],Remote_Read[26],2);
	SplicingPubData(NetSendBuf,ReadText[27],Remote_Read[27],3);
	SplicingPubTail(NetSendBuf);
	//SplicingVer(NetSendBuf);
}
void  SplicingPubStr_StirSwtitch(void)//
{
	memset(NetSendBuf,0,512);
	SplicingPubHead(NetSendBuf);
	SplicingPubData(NetSendBuf,ReadText[28],Remote_Read[28],0);
	SplicingPubData(NetSendBuf,ReadText[29],Remote_Read[29],1);
	SplicingPubData(NetSendBuf,ReadText[30],Remote_Read[30],2);
	SplicingPubData(NetSendBuf,ReadText[31],Remote_Read[31],3);
	SplicingPubTail(NetSendBuf);
	//SplicingVer(NetSendBuf);
}
void  SplicingPubStr_CountDown(void)//
{
	memset(NetSendBuf,0,512);
	SplicingPubHead(NetSendBuf);
	SplicingPubData(NetSendBuf,ReadText[32],Remote_Read[32],0);
	SplicingPubData(NetSendBuf,ReadText[33],Remote_Read[33],1);
	SplicingPubData(NetSendBuf,ReadText[34],Remote_Read[34],2);
	SplicingPubData(NetSendBuf,ReadText[35],Remote_Read[35],3);
	SplicingPubTail(NetSendBuf);
	//SplicingVer(NetSendBuf);
}

u8 g_u8SendStep=0;
void InitUp(void)
{
	MsgStruct Msgtemp;
	Msgtemp.CmdType = UPLOAD_DIAMETER;
	Msgtemp.CmdSrc = GPRS_TASK_CODE;
	PackSendRemoteQ(&Msgtemp);
	
	Msgtemp.CmdType = UPLOAD_PERCENT;
	PackSendRemoteQ(&Msgtemp);
	
	Msgtemp.CmdType = UPLOAD_UPPER;
	PackSendRemoteQ(&Msgtemp);
	
	Msgtemp.CmdType = UPLOAD_LOWER;
	PackSendRemoteQ(&Msgtemp);
	
	Msgtemp.CmdType = UPLOAD_STIRTIME;
	PackSendRemoteQ(&Msgtemp);
	
	Msgtemp.CmdType = UPLOAD_WATERSWITCH;
	PackSendRemoteQ(&Msgtemp);
	
	Msgtemp.CmdType = UPLOAD_STIRSWITCH;
	PackSendRemoteQ(&Msgtemp);
}
u8 WifiPara[110];
void network_task(void *pdata)
{
	//u8 Com3RxBuf[10]={0};
//	u8 SendNum = 0;
	//u8 buf[250];
	u8 os_err;
	MsgStruct * pMsgBlk = NULL;
	MsgStruct Msgtemp;
	RemoteQInit();
	FlashReadWiFi(WifiPara);
	if(WifiPara[0]!=0xff)//如果flash中未写入，使用程序中固化的
	{
		memcpy(ssid,WifiPara,31);
		memcpy(password,&WifiPara[31],31);
		memcpy(ipaddr,&WifiPara[62],16);
		memcpy(subnet,&WifiPara[78],16);
		memcpy(gateway,&WifiPara[94],16);
	}
	else
	{
		
	}
	//IO_OutSet(6,1);
	delay_ms(1000);
	Emw3060_init();
	Emw3060_con();
	InitUp();
	while(1)
	{
		
		delay_ms(100);
		{
			if(COM1GetBuf(NETCircle.buf, 512) > 0)
			{
				//解析收到的数据
				DepackRevData();
			}
			else
			{
				if(g_u8SendStep == 0)//解析
				{
					pMsgBlk = ( MsgStruct *) OSQPend ( RemoteQMsg,
						10,
						&os_err );
						
					if(os_err == OS_ERR_NONE)
					{
						memcpy((u8*)&Msgtemp,(u8*)pMsgBlk,sizeof(MsgStruct) );
						DepackReceiveRemoteQ(pMsgBlk);
						//发送函数
						//SplicingPubStr();
						switch(Msgtemp.CmdType)
						{
							case UPLOAD_LEVEL:
								SplicingPubStr_Level();
								break;
							case UPLOAD_DIAMETER:
								SplicingPubStr_Diameter();
								break;
							case UPLOAD_PERCENT:
								SplicingPubStr_Percent();
								break;
							case UPLOAD_UPPER:
								SplicingPubStr_Upper();
								break;
							case UPLOAD_LOWER:
								SplicingPubStr_Lower();
								break;
							case UPLOAD_STIRTIME:
								SplicingPubStr_StirTime();
								break;
							case UPLOAD_WATERSWITCH:
								SplicingPubStr_WaterSwtitch();
								break;
							case UPLOAD_STIRSWITCH:
								SplicingPubStr_StirSwtitch();
								break;
							case UPLOAD_COUNTDOWN:
								SplicingPubStr_CountDown();
								break;
							default:
								continue;
								//break;
						}
						sendEmw((char*)NetSendBuf,0);
					}
				}
			}
		}
	}
}

