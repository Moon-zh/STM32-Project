#include "Rs485.h"
#include "delay.h"
#include "sys.h"
//#include "Emw3060.h"
//#include "IO_BSP.h"
//#include "UserCore.h"
#include "string.h"
#include "NeowayN21.h"
#include "UserHmi.h"


//��Ȩ��Ϣ 
char ProductKey_NB0[20]=	"";//"a1q04qjosGa";
char DeviceName_NB0[50]=	"";//"test_single";
char DeviceSecret_NB0[50]="";	//"78eJYG6R0Y9AlPcpA9s4KXAUd3rjcQDn";

char *ProductKey_NB=ProductKey_NB0;
char *DeviceName_NB=DeviceName_NB0;
char *DeviceSecret_NB=DeviceSecret_NB0;



Ring_NB NETCircle_NB;//����ƽ̨�·����ݵĻ�������

OS_EVENT * RemoteQMsg;
void* 	RemoteMsgBlock[8];
OS_MEM* RemotePartitionPt;
u8 g_u8RemoteMsgMem[20][8];




//������Ϣ���к��ڴ��
//���0����������
//    1����Ϣ���д���ʧ��
//	  2���ڴ�鴴��ʧ��
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
//��Ϣ�������
//���룺��Ϣ��Ϣ
//�����0�����ͳɹ�
//		1�������ڴ�ʧ��
//		2��������Ϣʧ��

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
	MsgTemp ->CmdData[2] = MsgBlk->CmdData[2];
	MsgTemp ->CmdData[3] = MsgBlk->CmdData[3];
	os_err = OSQPost ( RemoteQMsg,(void*)MsgTemp );
	//������Ϣʧ���ͷ��ڴ�
	if(os_err!=OS_ERR_NONE)
	{
		OSMemPut(RemotePartitionPt, ( void * )MsgTemp);
		return 2;
	}
	return 0;
}
//������Ϣ��������Ϣ�ڴ��ͷ�
//���룺�յ�����Ϣ����ָ��
//�����0���ͷųɹ�
//		1���ͷ�ʧ��
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

/*
*
*�з�N21��ʼ��
*
*/
void NeowayN21_init()//�з�N21��ʼ��
{
	u8 buf[250];
	char *msg;
	u8 num;
	u8 CCID[50];
	
	while(1)//ģ�鿪��״̬�·���AT\rƥ�䲨����1
	{
		u3_printf("AT\r\n");
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	//�ָ������Զ�������
	u3_printf("AT&F\r\n");
	delay_ms(500);
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);	

	while(1)//��ѯ���Ƿ���2
	{
		u3_printf("AT+CPIN?\r\n");
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"+CPIN:READY")[0]=='+')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)//��ȡSIM����ʶ
	{
		u3_printf("AT+CCID\r\n");
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"+CCID:")[0]=='+')break;		
	}
	msg=strstr((const char *)buf,"+CCID:")+7;
	for(num=0;*msg!=0x0d;msg++)CCID[num++]=*msg;//ѭ���жϲ�����0X0D��\r��
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)//����Ƶ��
	{
		u3_printf("AT+NVSETBAND=1,8\r\n");
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;	
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);		
	
	while(1)//��ѯ�����ź�����3
	{
		u3_printf("AT+CSQ\r\n");
		delay_ms(500);
		COM1GetBuf(buf,200);
		msg=strstr((const char *)buf,"+CSQ:");
		msg=msg+6;
		if((msg[0]!='9')&&(msg[1]!='9'))break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);	
	
	while(1)//��ѯ����ע��״̬4
	{
		u3_printf("AT+CEREG?\r\n");
		delay_ms(500);
		COM1GetBuf(buf,200);
		msg=strstr((const char *)buf,"+CEREG:");
		msg=msg+10;
		if((msg[0]=='1')||(msg[0]=='5'))break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);			
}

/*
*
*N21���ӵ�������
*
*/
void NeowayN21_EMW()//N21���ӵ�������
{
	u8 buf[250];
	char *msg;
	u8 i=0,k=0;
	
	while(1)//��ȡIP,������������5
	{
		u3_printf("AT+XIIC=1\r\n");
		delay_ms(2000);
		COM1GetBuf(buf,200);	
		if(strstr((const char *)buf,"OK")[0]=='O')
		{
			if(strchr((const char *)buf,'E')[0]=='E')
			{
				comClearRxFifo(COM1);
				memset(buf,0,sizeof buf);
				continue;
			}
			break;
		}	
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);

	while(1)//��ѯ�Ƿ���䵽IP��ֻ�з��䵽IP���������6
	{
		u3_printf("AT+XIIC?\r\n");
		delay_ms(500);
		COM1GetBuf(buf,200);	
		msg=strstr((const char *)buf,"+XIIC:");
		msg=msg+10;
		if(msg[0]=='1')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);	
	
	while(1)//���м�Ȩ,��Ȩ�ɹ�����ܽ��ж��ĺͷ���7
	{
		i=0;
		u3_printf("AT+IMQTTAUTH=\"%s\",\"%s\",\"%s\"\r\n",ProductKey_NB,DeviceName_NB,DeviceSecret_NB);		
		while(1)
		{
			COM1GetBuf(buf,100);
			delay_ms(500);
			if(strstr((const char *)buf,"+IMQTTAUTH:")[0]=='+')break;
			if(++i==40)break;
		}
		if(strstr((const char *)buf,"+IMQTTAUTH:OK")[0]=='+')break;
		memset(buf,0,sizeof buf);
		comClearRxFifo(COM1);
		if(++k==5)
		{
//			Nsendok=0;
			return;
		}
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);		
	
	while(1)//����MQTT���Ӳ���8  AT+IMQTTPARA="TIMEOUT",2,"CLEAN",1,"KEEPALIVE",180,"VERSION","3.1" \r
	{
		u3_printf("AT+IMQTTPARA=\"TIMEOUT\",10,\"CLEAN\",1,\"KEEPALIVE\",180,\"VERSION\",\"3.1\"\r\n");
		delay_ms(500);
		COM1GetBuf(buf,200);	
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);	

	while(1)//����MQTT����9
	{
		i=0;
		u3_printf("AT+IMQTTCONN\r\n");
		while(1)
		{
			COM1GetBuf(buf,200);
			delay_ms(500);
			if(strchr((const char *)buf,'K')[0]=='K')break;
			if(strchr((const char *)buf,'E')[0]=='E')break;
			if(++i==40)break;
		}
		comClearRxFifo(COM1);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
		memset(buf,0,sizeof buf);
		delay_ms(1000);		
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);	

	while(1)//������������10  ���ĵ�����+QOS
	{
		u3_printf("AT+IMQTTSUB=\"/%s/%s/user/irr_set\",1\r\n",ProductKey_NB,DeviceName_NB);
		delay_ms(1000);
		COM1GetBuf(buf,200);
//		if(strstr((const char *)buf,"OK")[0]=='O')break;
		msg=strstr((const char *)buf,"+IMQTTSUB:");
  	if(strstr((const char *)msg,",")[1]=='1')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
//	Nsendok=1;
	
}

void	disconN21()//�Ͽ�N21����,�Ͽ�MQTT
{
	u8 buf[250];
	while(1)
	{
		u3_printf("AT+IMQTTDISCONN\r\n");
		COM1GetBuf(buf,200);
		delay_ms(500);
		if(strchr((const char *)buf,'K')[0]=='K')break;
		if(strchr((const char *)buf,'E')[0]=='E')break;	
	}	
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	delay_ms(1000);
}

/*
*�����Ƶ�topic��������
*AT+IMQTTPUB="/sys/a1q04qjosGa/v3cfYT4EFqN7gT2NERSg/thing/event/property/post",1,"{params:{to_type:1,to_time:22,need_partition:1,switch:2,work_state:1,in_time:2,at_partition:1}}"\r
*
*/
u8	sendN21(char *data,u8 w)			//�����Ƶ�topic��������
{
	char hc[500];
	u16 len;
	u8 buf[350];
	char *msg;

	if(!w)sprintf(hc,"AT+IMQTTPUB=\"/sys/%s/%s/thing/event/property/post\",1,%s\r\n",ProductKey_NB,DeviceName_NB,data);
	else sprintf(hc,"AT+IMQTTPUB=\"/%s/%s/user/war\",1,%s\r\n",ProductKey_NB,DeviceName_NB,data);
	
	u3_printf("%s",hc);
	delay_ms(2000);
	COM1GetBuf(buf,330);
	comClearRxFifo(COM1);
	
	msg=strstr((const char *)buf,"+IMQTTPUB:");
	if(strstr((const char *)msg,",")[1]=='0')return 0;	//�ж��Ƿ��ϱ�����
	else return 1;
}



 u8 N21ConnectFlag=1;
 u8 N21ConnectStep = 0; 
 u8 MQTTConnectFlag = 0;
 u8 MQTTConnetcStep = 0;




/*
*�������ݺͽ������ݹ��õı���
*
*/
s16 Remote_Para_NB[4];
s16 Remote_Read_NB[4];
void SetRemoteRead_NB(u8 addr,s16 value)
{
	Remote_Read_NB[addr] = value;
}
void SetRemotePara_NB(u8 addr,s16 value)
{
	Remote_Para_NB[addr] = value;
}
//��ƽ̨�������ݵĸ�ʽ����ʽ��ͷ���ַ���
const u8 TypeStr_NB[8]="to_type\0";
const u8 TimeStr_NB[8]="to_time\0";
const u8 PartitionStr_NB[15]="need_partition\0";
const u8 SwitchStr_NB[7]="switch\0";
const u8 WorkStateStr_NB[11]="work_state\0";
const u8 CountDownStr_NB[8]="in_time\0";
const u8 At_Partition_NB[13]="at_partition\0";
u8 CCIDstr_NB[21];
const u8* SetText_NB[] = 
{
	TypeStr_NB,
	TimeStr_NB,
	PartitionStr_NB,
	SwitchStr_NB
};
const u8* ReadText_NB[] = 
{
	TypeStr_NB,
	TimeStr_NB,
	PartitionStr_NB,
	SwitchStr_NB,
	WorkStateStr_NB,
	CountDownStr_NB,
	At_Partition_NB,
	CCIDstr_NB
};


/*
���ַ����е����֣�ȡ������ϳ���ֵ
*/
s16 RevFigure_NB(u8*src)
{
	u8 u8F = 0;
	s16 revalue=0;

	if(*src==0x2d)//����
	{
		u8F = 1;
		src++;
	}
	while((*src>=0x30)&&(*src<=0x39))//����0-9
	{
		revalue = revalue*10+ (*src-0x30);
		src++;
	}
	if(u8F == 1)//�����ݸĳɸ���
	{
		revalue = revalue*(-1);
	}
	return revalue;
}
//void GM3_check_cmd(u8*str)
//GM3���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//����,�ڴ�Ӧ������λ��(str��λ��)
u8* N21_check_cmd(u8 *str)
{
	char *strx=0;
	strx = strstr((const char*)NETCircle_NB.buf,(const char*)str);
	return (u8*)strx;
}
/*
����ƽ̨�·�������
{"to_type":1,"to_time":25,"need_partition":3,"switch":1}

SetText[0]��const u8 TypeStr[8]="to_type\0";
SetText[1]��const u8 TimeStr[8]="to_time\0";
SetText[2]��const u8 PartitionStr[15]="need_partition\0";
SetText[3]��const u8 SwitchStr[7]="switch\0";
����״̬��work_state�� ����-0����ˮ-1����ˮ-2 
�������(to_type)    Ĭ��-0����ˮ-1����ˮ-2
���ʱ����to_time��  ʱ�䳣��
��Ҫ��ȷ�����need_partition��  1����-1,2����-2,1��2����-3
����ʱ��in_time��
��ǰ��ȷ�����at_partition��  1����-1,2����-2
��ͣ(switch)    ֹͣ-0������-1

+IMQTTRCVPUB:24622,/a1q04qjosGa/v3cfYT4EFqN7gT2NERSg/user/irr_set,56,{"to_type":1,"to_time":25,"need_partition":3,"switch":1}
*/
void DepackRevData_NB(void)
{
	u8* p=NULL;
	u8 u8Cnt = 0;
	//u16 RevNum=0;
	
	p=N21_check_cmd("IMQTTRCVPUB:");
	if(p)
	{
			for(u8Cnt=0;u8Cnt<ParaNum_NB;u8Cnt++)
			{
				p=N21_check_cmd((u8*)SetText_NB[u8Cnt]);
				if(p)
				{
					Remote_Para_NB[u8Cnt] = RevFigure_NB(p+strlen(SetText_NB[u8Cnt])+2);
				}
			}
	}

	if(Remote_Para_NB[3]==1)//����
	{
		if(Remote_Para_NB[0]==1)//�������-��ˮ
		{
			if((Remote_Para_NB[1]==0)||(Remote_Para_NB[1]>1440))//���ʱ��
			{
				return;
			}
		}
		else	//if(Remote_Para[0]==2)//�������-��ˮ
		{
			if((Remote_Para_NB[1]<20)||(Remote_Para_NB[1]>1440))//���ʱ��
			{
				return;
			}			
		}
	  //��������������Ϣ
		Msgtemp.CmdSrc = SCREEN_TASK_CODE;   //����������
		Msgtemp.CmdType = MSG_START;   //����
		Msgtemp.CmdData[0] = Remote_Para_NB[0];//�������(to_type)    Ĭ��-0����ˮ-1����ˮ-2
		Msgtemp.CmdData[1] = Remote_Para_NB[2];//��Ҫ��ȷ�����need_partition��  1����-1,2����-2,1��2����-3
		Msgtemp.CmdData[2] = Remote_Para_NB[1]/60;//���ʱ����Сʱ��
		Msgtemp.CmdData[3] = Remote_Para_NB[1]%60;//���ʱ�������ӣ�
		PackSendMasterQ(&Msgtemp);
		SetScreen(LCD_STATESHOW_PAGE);//�л�����
	}
	else if(Remote_Para_NB[3]==0)//ֹͣ
	{
		//��������ֹͣ��Ϣ
		Msgtemp.CmdSrc = SCREEN_TASK_CODE;//����������
		Msgtemp.CmdType = MSG_STOP;  //����
		PackSendMasterQ(&Msgtemp);
	}
	
		comClearRxFifo(COM1);	
}





/*
�������ݵĸ�ʽ
{\"params\":{\"to_type\":1,\"to_time\":2,\"need_partition\":3,\"switch\":4,\"work_state\":5,\"in_time\":6,\"at_partition\":7}}
*/
void SplicingPubHead_NB(u8* Dest)
{
	strcpy((char*)Dest,"{\"params\":{");  //�Ѵ�src��ַ��ʼ�Һ���NULL���������ַ������Ƶ���dest��ʼ�ĵ�ַ�ռ�
}
void SignedNumberToASCII_NB( u8 *Arry , s16 Number);//����������
void SplicingPubData_NB(u8*Dest,const u8*Text,s16 Value,u8 FirstF)
{
	u8 u8Str[16];
	if(FirstF >0)
	{
		strcat((char*)Dest,",");///������char��������,�������Dest������
	}
	strcat((char*)Dest,"\"");//����\"
	strcat((char*)Dest,(char*)Text);
	strcat((char*)Dest,"\":");//����\":
	SignedNumberToASCII_NB(u8Str,Value);//16λ�з�����ת����ASCII��
	strcat((char*)Dest,(char*)u8Str);
}
void SplicingPubTail_NB(u8* Dest)
{
	strcat((char*)Dest,"}}");
}
u8 NetSendBuf_NB[256];
void  SplicingPubStr_NB(void)//����Ҫ�ϴ������ݣ����и�ʽ������
{
	memset(NetSendBuf_NB,0,256);//��NetSendBuf����ȫ������
	SplicingPubHead_NB(NetSendBuf_NB);//��ʽ�Ŀ�ͷ
	SplicingPubData_NB(NetSendBuf_NB,ReadText_NB[0],Remote_Para_NB[0],0);//to_type
	SplicingPubData_NB(NetSendBuf_NB,ReadText_NB[1],Remote_Para_NB[1],1);//to_time
	SplicingPubData_NB(NetSendBuf_NB,ReadText_NB[2],Remote_Para_NB[2],2);//need_partition
	SplicingPubData_NB(NetSendBuf_NB,ReadText_NB[3],Remote_Para_NB[3],3);//switch
	SplicingPubData_NB(NetSendBuf_NB,ReadText_NB[4],Remote_Read_NB[0],4);//work_state
	SplicingPubData_NB(NetSendBuf_NB,ReadText_NB[5],Remote_Read_NB[1],5);//in_time
	SplicingPubData_NB(NetSendBuf_NB,ReadText_NB[6],Remote_Read_NB[2],6);//at_partition
	//SplicingPubData(NetSendBuf,ReadText_NB[7],Remote_Read_NB[3],7);
	SplicingPubTail_NB(NetSendBuf_NB);//��ʽ�Ľ�β
}





u8 g_u8SendStep=0;
void N21Reset(void)//����ģ��
{
	IO_OutSet(6,0);
	delay_ms(300);
	IO_OutSet(6,0);
	N21ConnectFlag = 1;
	N21ConnectStep = 0;
	MQTTConnectFlag = 0;
	MQTTConnetcStep = 0;
	
}
u16 g_u16DogCnt=0;
u16 g_u16DogCnt1=0;

void N21Dog(void)
{
	//��ʼ����MQTT����ʱ�����쳣
	if((N21ConnectFlag!=0)||(MQTTConnectFlag !=0))
	{
		g_u16DogCnt++;
	}
	else
	{
		g_u16DogCnt = 0;
	}
	if(g_u16DogCnt>900)
	{
		N21Reset();
	}
	//����ʱ�����쳣
	if((g_u8SendStep==1)&&(N21ConnectFlag==0)&&(MQTTConnectFlag ==0))
	{
		g_u16DogCnt1++;
	}
	else
	{
		g_u16DogCnt1 = 0;
	}
	if(g_u16DogCnt1>600)
	{
		N21Reset();
	}
}


/*
*
*N21��������
*
*/
void network_nb_task(void *pdata)
{
	//u8 Com3RxBuf[10]={0};
//	u8 SendNum = 0;
	
	u8 os_err;
	MsgStruct * pMsgBlk = NULL;
	MsgStruct Msgtemp;
	RemoteQInit();//������Ϣ���к��ڴ��
	
	
//	IO_OutSet(6,1);
	delay_ms(1000);
	
	
	
	while(1)
	{
		delay_ms(100);
		N21Dog();
		if(N21ConnectFlag == 1)
		{
			N21Connect();
		}
		else if(MQTTConnectFlag == 1)
		{
			MQTTConnect();
		}
		else
		{
			if(COMGetBuf(COM1 ,NETCircle_NB.buf, 256) > 10)
			{
				//�����յ�������
				DepackRevData_NB();
			}
			else
			{
				if(g_u8SendStep == 0)//����
				{
					pMsgBlk = ( MsgStruct *) OSQPend ( RemoteQMsg,
						10,
						&os_err );
						
					if(os_err == OS_ERR_NONE)
					{
						memcpy((u8*)&Msgtemp,(u8*)pMsgBlk,sizeof(MsgStruct) );//��ԴpMsgBlk����n���ֽڵ�ĿMsgtemp��
						DepackReceiveRemoteQ(pMsgBlk);//������Ϣ��������Ϣ�ڴ��ͷ�

						//���ͺ���
						SplicingPubStr_NB();
						
						delay_ms(500);
					}
				}
			}
		}
	}
}

