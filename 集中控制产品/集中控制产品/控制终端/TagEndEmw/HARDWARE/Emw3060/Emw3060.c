#include "Rs485.h"
#include <string.h>
#include "delay.h"
#include "sys.h"
#include "Emw3060.h"
#include "md5.h"
#include "UserCore.h"
#include "UserHmi.h"
#include "FlashDivide.h"

//鉴权信息
 char ProductKey0[20]=	"a1EF5UgoDb2";//"a1q04qjosGa";
 char DeviceName0[50]=	"SG-ZD-01";//"test_single";
 char DeviceSecret0[50]="gH4T8svOdv6DD7gFG6C9WgMQrbYPtC4z";	//"78eJYG6R0Y9AlPcpA9s4KXAUd3rjcQDn";

 char *ProductKey=ProductKey0;
 char *DeviceName=DeviceName0;
 char *DeviceSecret=DeviceSecret0;

Ring NETCircle;

OS_EVENT * RemoteQMsg;
void* 	RemoteMsgBlock[8];
OS_MEM* RemotePartitionPt;
u8 g_u8RemoteMsgMem[20][8];

u8 DHCP=1;

char ssid[31]="CU_kquj";
char password[31]="abb62f7k";
char ipaddr[16]="";
char subnet[16]="";
char gateway[16]="";
char dns[16]="";

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
			COMGetBuf(COM1 ,NETCircle.buf, 256);
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
s16 Remote_Read[4];
void SetRemoteRead(u8 addr,s16 value)
{
	Remote_Read[addr] = value;
}
void SetRemotePara(u8 addr,s16 value)
{
	Remote_Para[addr] = value;
}
const u8 Switch1StateStr[13]="Switch1State\0";
const u8 Switch2StateStr[13]="Switch2State\0";
const u8 Switch3StateStr[13]="Switch3State\0";
const u8 Switch4StateStr[13]="Switch4State\0";
u8 CCIDstr[21];
const u8* SetText[] = 
{
	Switch1StateStr
};
const u8* ReadText[] = 
{
	Switch1StateStr,
	Switch2StateStr,
	Switch3StateStr,
	Switch4StateStr,
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
			p=EMW3060_check_cmd((u8*)SetText[u8Cnt]);
			
			if(p)
			{
				*p = 1;
//				Remote_Para[u8Cnt] = RevFigure(p+strlen(SetText[u8Cnt])+2);
			}
		}
	}
	if(Remote_Para[3]==1)
	{
//		Msgtemp.CmdSrc = SCREEN_TASK_CODE;
//		Msgtemp.CmdType = MSG_START;
//		Msgtemp.CmdData[0] = Remote_Para[0];
//		Msgtemp.CmdData[1] = Remote_Para[2];
//		Msgtemp.CmdData[2] = Remote_Para[1]/60;
//		Msgtemp.CmdData[3] = Remote_Para[1]%60;
//		PackSendMasterQ(&Msgtemp);
//		SetScreen(LCD_STATESHOW_PAGE);
	}
	else if(Remote_Para[3]==0)
	{
		//发送任务停止消息
		Msgtemp.CmdSrc = SCREEN_TASK_CODE;
		Msgtemp.CmdType = MSG_STOP;
		PackSendMasterQ(&Msgtemp);
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
	u8 DnchF=0;
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
	
	FlashReadDHCP(&DnchF);
	if(DnchF==0)
	{
		while(1)
		{
			u3_printf("AT+WDHCP=OFF\r");
			delay_ms(50);
			COM1GetBuf(buf,200);
			if(strstr((const char *)buf,"OK")[0]=='O')
			break;
		}
		comClearRxFifo(COM1);
		memset(buf,0,sizeof buf);
		
		while(1)
		{
			u3_printf("AT+WJAPIP=%s,%s,%s,%s\r",ipaddr,subnet,gateway,dns);
			delay_ms(50);
			COM1GetBuf(buf,200);
			if(strstr((const char *)buf,"OK")[0]=='O')
			break;
		}
		comClearRxFifo(COM1);
		memset(buf,0,sizeof buf);
	}
	
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
void SignedNumberToASCII ( u8 *Arry , s16 Number,u8 decimalnum);
void SplicingPubData(u8*Dest,const u8*Text,s16 Value,u8 FirstF)
{
	u8 u8Str[16];
	if(FirstF >0)
	{
		strcat((char*)Dest,",");
	}
	strcat((char*)Dest,"\"");
	strcat((char*)Dest,Text);
	strcat((char*)Dest,"\":");
	SignedNumberToASCII(u8Str,Value,0);
	strcat((char*)Dest,u8Str);
}
void SplicingPubTail(u8* Dest)
{
	strcat((char*)Dest,"}}");
}

u8 NetSendBuf[512];
void  SplicingPubStr(void)//
{
	memset(NetSendBuf,0,512);
	SplicingPubHead(NetSendBuf);
	SplicingPubData(NetSendBuf,ReadText[0],Remote_Read[0],0);
	SplicingPubData(NetSendBuf,ReadText[1],Remote_Read[1],1);
	SplicingPubData(NetSendBuf,ReadText[2],Remote_Read[2],2);
	SplicingPubData(NetSendBuf,ReadText[3],Remote_Read[3],3);
	//SplicingPubData(NetSendBuf,ReadText[7],Remote_Read[3],7);
	SplicingPubTail(NetSendBuf);
	//SplicingVer(NetSendBuf);
	//u3_printf("%s\r\n",NetSendBuf);//发送命令
}
u8 g_u8SendStep=0;
void InitUp(void)
{
	MsgStruct MsgtempBlk;
	MsgtempBlk.CmdType = INITIATIVE_INPUT_REG;
	MsgtempBlk.CmdData[0] = 00;
	MsgtempBlk.CmdData[1] = 01;
	PackSendRemoteQ(&MsgtempBlk);
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
	if((WifiPara[0]!=0xff)&(WifiPara[0]!=0))//如果flash中未写入，使用程序中固化的
	{
		memcpy(ssid,WifiPara,31);
		memcpy(password,&WifiPara[31],31);
		memcpy(ipaddr,&WifiPara[62],16);
		memcpy(subnet,&WifiPara[78],16);
		memcpy(gateway,&WifiPara[94],16);
		memcpy(dns,&WifiPara[110],16);
	}
	else
	{
		memcpy(WifiPara,ssid,31);
		memcpy(&WifiPara[31],password,31);
		memcpy(&WifiPara[62],ipaddr,16);
		memcpy(&WifiPara[78],subnet,16);
		memcpy(&WifiPara[94],gateway,16);
		memcpy(&WifiPara[110],dns,16);
		FlashWriteWiFi(WifiPara);
	}
	//IO_OutSet(6,1);
	delay_ms(1000);
	InitUp();
	Emw3060_init();
	Emw3060_con();
	while(1)
	{
		
		delay_ms(100);
		{
			if(COMGetBuf(COM1 ,NETCircle.buf, 512) > 0)
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
						SplicingPubStr();
						sendEmw(NetSendBuf,0);
					}
				}
			}
		}
	}
}

