#include "Rs485.h"
#include <string.h>
#include "delay.h"
#include "sys.h"
#include "Emw3060.h"
#include "md5.h"
#include "UserCore.h"
#include "UserHmi.h"
#include "FlashDivide.h"

#include	"rtc.h"
#include	"IO_BSP.h"
#include	"cgq.h"
	/*
	��Ԫ��
	ProductKey:��ƷID     ˮ��һ���
  DeviceName���豸ID    23   24  25
	DeviceSecret���豸����Կ�����룩 
	*/

//��Ȩ��Ϣ 
 char ProductKey0[20]=	"";//"a1q04qjosGa";
 char DeviceName0[50]=	"";//"test_single";
 char DeviceSecret0[50]="";	//"78eJYG6R0Y9AlPcpA9s4KXAUd3rjcQDn";

 char *ProductKey=ProductKey0;
 char *DeviceName=DeviceName0;
 char *DeviceSecret=DeviceSecret0;
 
 //wifi��Ϣ
 char ssid[30]="";  //wifi����
 char password[30]=""; //wifi����
 char ipword[30]="";//IP��ַ  "192.168.7.112";
 char zwword[30]="";//��������  "255.255.255.0";
 char wgword[30]="";//���ص�ַ  	"192.168.7.1";
 char ymword[30]="";//����  "114.114.114.114";
 u8 DHCP=1;//����DHCP�Ŀ��� (Ĭ�Ͽ���DHCP)
 u8 DHCP_cq=0;//����DHCP����������   (1:Ϊ��������  0��Ϊ��������)
 
u8 csh_bz=0;//wifiģ���wifi���ӻ�MQTT�����Ƿ�ɹ��� csh_bz=0�����ӳɹ���csh_bz=1,û�����ӳɹ�

Ring NETCircle;//����ƽ̨�·����ݵĻ�������
u8 fsbz_3060=0;//ÿһ�ν���������ֹ֮ͣ�󣬶����ϴ�һ�����ڵ�����
u8 fsbz_3060_bf=0;//�������ݱ����ı��ݣ��ڿ���֮������ֹͣ��ʱ���ʹ��

OS_EVENT  *RemoteQMsg; //�ź���������һ��OS_EVENT���͵�ָ��ָ�����ɵĶ���
void  *RemoteMsgBlock[8]; // ��Ϣ�������飬 ����һ��ָ����Ϣ�����ָ�������Ĵ�С����ָ�������������Ϊvoid����
OS_MEM  *RemotePartitionPt;/*�����ڴ���ƿ�ָ�룬����һ���ڴ����ʱ������ֵ������ */
u8 g_u8RemoteMsgMem[20][8]; ///*����һ������20���ڴ�飬ÿ���ڴ�鳤����8���ڴ���� */


//������Ϣ���к��ڴ��
//���0����������
//    1����Ϣ���д���ʧ��
//	  2���ڴ�鴴��ʧ��
//    QSQPostΪ�Ƚ��ȳ�����(FIFO)��QSQPostForntΪ����ȳ�����(LIFO)
u8 RemoteQInit(void)
{
	INT8U os_err;
	
	RemoteQMsg = OSQCreate ( RemoteMsgBlock, 8);//����OSQcreate()����������Ϣ����
	
	if(RemoteQMsg == (OS_EVENT *)0)
	{
		return 1;
	}
	
	RemotePartitionPt = OSMemCreate (   ///*������̬�ڴ���  *//*����һ������20���ڴ�飬ÿ���ڴ�鳤����8���ڴ���� */
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
//   MsgStruct��8�������Ľṹ��
u8 PackSendRemoteQ(MsgStruct* MsgBlk)
{
	INT8U os_err;
	MsgStruct  *MsgTemp = NULL;//����ṹ��ָ��
	MsgTemp = (MsgStruct *)OSMemGet(RemotePartitionPt,&os_err);//��ȡ�洢�飬�ڴ�����
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
	os_err = OSQPost ( RemoteQMsg,(void*)MsgTemp ); //����Ϣ���з���һ����Ϣ(FIFO)
	
	//������Ϣʧ���ͷ��ڴ�
	if(os_err!=OS_ERR_NONE)
	{
		OSMemPut(RemotePartitionPt, ( void * )MsgTemp);//�ͷ��ڴ�
		return 2;
	}
	return 0;
}
//������Ϣ��������Ϣ�ڴ��ͷ�
//���룺�յ�����Ϣ����ָ��
//�����0���ͷųɹ�
//		 1���ͷ�ʧ��
u8 DepackReceiveRemoteQ(MsgStruct * RemoteQ)
{
	u8 os_err;
	os_err = OSMemPut(RemotePartitionPt, ( void * )RemoteQ);//�ͷ��ڴ�
	if(os_err != OS_ERR_NONE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

//����Ϣ���ж�ȡ����
//���룺�յ���Ϣ�����е����ݣ��洢�ĵط�
//����� 0����ȡ�ɹ�
//       1����ȡʧ��
u8 PendReceiveRemoteQ(MsgStruct *Pend1RemoteQ)
{
	u8 os_err;
	MsgStruct * pMsgBlk = NULL;

	pMsgBlk = ( MsgStruct *) OSQPend ( RemoteQMsg,
		10,
		&os_err );
	// //������Ϣ���У������ֱ��ǣ�RemoteQMsgΪ��������Ϣ���е�ָ��  �ڶ�������Ϊ�ȴ�ʱ��
		 //0��ʾ���޵ȴ���&errΪ������Ϣ������ֵΪ���п��ƿ�RemoteQMsg��ԱOSQOutָ�����Ϣ(�������������Ϣ���õĻ�)�����
		//û����Ϣ���ã���ʹ����OSQPend���������ʹ֮���ڵȴ�״̬��������һ���������
		
	if(os_err == OS_ERR_NONE)
	{
		memcpy(Pend1RemoteQ,(u8*)pMsgBlk,sizeof(MsgStruct) );//��ԴpMsgBlk����n���ֽڵ�ĿMsgtemp��
		DepackReceiveRemoteQ(pMsgBlk);//������Ϣ��������Ϣ�ڴ��ͷ�
		
		return 0;
	}	
	return 1;
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

//��ƽ̨�������ݵĸ�ʽ����ʽ��ͷ���ַ���
const u8 TypeStr[8]="to_type\0";
const u8 TimeStr[8]="to_time\0";
const u8 PartitionStr[15]="need_partition\0";
const u8 SwitchStr[7]="switch\0";
const u8 WorkStateStr[11]="work_state\0";
const u8 CountDownStr[8]="in_time\0";
const u8 At_Partition[13]="at_partition\0";
u8 CCIDstr[21];
const u8* SetText[] = 
{
	TypeStr,
	TimeStr,
	PartitionStr,
	SwitchStr
};
const u8* ReadText[] = 
{
	TypeStr,
	TimeStr,
	PartitionStr,
	SwitchStr,
	WorkStateStr,
	CountDownStr,
	At_Partition,
	CCIDstr
};

/*
���ַ����е����֣�ȡ������ϳ���ֵ
*/
s16 RevFigure(u8*src)
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
u8* EMW3060_check_cmd(u8 *str)
{
	char *strx=0;
	strx = strstr((const char*)NETCircle.buf,(const char*)str);
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
*/
s16 Remote_Para_hc[4];//Remote_Para_hc������Remote_Para�Ļ�������
void DepackRevData(void)
{
	u8* p=NULL;
	u8 u8Cnt = 0;
	MsgStruct Msgtemp;
	
	p=EMW3060_check_cmd("MQTTRECV:");
	if(p)
	{     //   EmwLED1=!EmwLED1;//����
		for(u8Cnt=0;u8Cnt<ParaNum;u8Cnt++)
		{
			p=EMW3060_check_cmd((u8*)SetText[u8Cnt]);
			
			if(p)
			{
				Remote_Para_hc[u8Cnt] = RevFigure(p+strlen((const char*)SetText[u8Cnt])+2);
			}
		}
		
		if(Remote_Para_hc[3]==1&&MasterState==0)//����������������״̬Ϊ����
		{
			delay_ms(1000);
			if(MasterState==0)//��ʱ1��֮�����ж�һ���ǲ��ǳ��ڿ���״̬
			{
				//��ֹ֮ͣǰ�Ĺ�����ͺͷ������ڽ����µĹ�����ͺͷ���		
				if(Guangai.zdms==1)//�����������Զ�ģʽ�£�����Զ�ģʽ�Ĳ���
				{
						Guangai.zdms=0;//�ر��Զ�ģʽ
						Guangai.qdbz=0;//�Զ�ģʽ�رս�ˮ
						Guangai.yichi=0;//��־���						
				}
				//��������ֹͣ��Ϣ
				Msgtemp.CmdSrc = SCREEN_TASK_CODE;//����������
				Msgtemp.CmdType = MSG_STOP;  //����	
				PackSendMasterQ(&Msgtemp);//��Ϣ���У���Ϣ�������
				while(MasterState!=0)delay_ms(10);//����������״̬Ϊ��������
				IO_OutSet(1,0);//�رյ�ŷ�-��ˮ
				IO_OutSet(2,0);//�رյ�ŷ�-��һ������ˮ
				IO_OutSet(3,0);//�رյ�ŷ�-�ڶ�������ˮ

				Remote_Para[0]=Remote_Para_hc[0];//�������(to_type)    Ĭ��-0����ˮ-1����ˮ-2
				Remote_Para[1]=Remote_Para_hc[1];//���ʱ����to_time��  ʱ�䳣��			
				Remote_Para[2]=Remote_Para_hc[2];//��Ҫ��ȷ�����need_partition��  1����-1,2����-2,1��2����-3			
				Remote_Para[3]=Remote_Para_hc[3];	//����-1 ֹͣ-0		
						
				if(Remote_Para[0]==1)//�������-��ˮ
				{
					if((Remote_Para[1]==0)||(Remote_Para[1]>1440))//���ʱ��
					{
						return;
					}
				}
				else	//if(Remote_Para[0]==2)//�������-��ˮ
				{
					if((Remote_Para[1]<=20)||(Remote_Para[1]>1440))//���ʱ��
					{
						return;
					}			
				}

				Msgtemp.CmdSrc = SCREEN_TASK_CODE;   //����������
				Msgtemp.CmdType = MSG_START;   //����
				Msgtemp.CmdData[0] = Remote_Para[0];//�������(to_type)    Ĭ��-0����ˮ-1����ˮ-2
				Msgtemp.CmdData[1] = Remote_Para[2];//��Ҫ��ȷ�����need_partition��  1����-1,2����-2,1��2����-3
				Msgtemp.CmdData[2] = Remote_Para[1]/60;//���ʱ����Сʱ��
				Msgtemp.CmdData[3] = Remote_Para[1]%60;//���ʱ�������ӣ�
				PackSendMasterQ(&Msgtemp);//��Ϣ���У���Ϣ�������
				SetScreen(LCD_STATESHOW_PAGE);//�л�����
			
				Remote_Read[3]+=2;//����
				
			}
		}
		else if(Remote_Para_hc[3]==0)//ֹͣ
		{
			if(Guangai.zdms==1)//�����������Զ�ģʽ�£�����Զ�ģʽ�Ĳ���
			{
					Guangai.zdms=0;//�ر��Զ�ģʽ
					Guangai.qdbz=0;//�Զ�ģʽ�رս�ˮ
					Guangai.yichi=0;//��־���						
			}
			
			//��������ֹͣ��Ϣ
			Msgtemp.CmdSrc = SCREEN_TASK_CODE;//����������
			Msgtemp.CmdType = MSG_STOP;  //����
			//Msgtemp.CmdData[0] = 1;//�������(to_type)    Ĭ��-0����ˮ-1����ˮ-2  ƽ̨�жϵĲ���Ϊ0
			Msgtemp.CmdData[1] = 0;//��Ҫ��ȷ�����need_partition��  1����-1,2����-2,1��2����-3
			Msgtemp.CmdData[2] = 0;//���ʱ����Сʱ��
			Msgtemp.CmdData[3] = 0;//���ʱ�������ӣ�		
			PackSendMasterQ(&Msgtemp);//��Ϣ���У���Ϣ�������
			
	
			Remote_Read[3]+=1;//����
			
		}
		
		
	}
		comClearRxFifo(COM1);	
//		memset(g_RxBuf1,0,UART1_RX_BUF_SIZE);//��ջ�������g_RxBuf1
	  memset(NETCircle.buf,0,510);//��ջ�������NETCircle.buf
}



char xxbuf[34];

char	*hmacmd5_2(char *data,u8 size_data,u8 size_ds)
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

void	md5tohex(char *data,char *str)	//�ֽ��ֽ��а����������ַ���ֵ
{
	u8 i;
	for(i=0;i<16;i++)sprintf(str+i*2,"%02x",data[i]);
}

void	hmacmd5(char *data,u8 size_data,u8 size_ds,char *str)	//hmacmd5����
{
	char buf[33];
	hmac_md5((unsigned char*)data,size_data,(unsigned char*)DeviceSecret,size_ds,(unsigned char*)str);
	md5tohex(str,buf);
	sprintf(str,"%s",buf);
}

char	*hmacmd5_5(char *data,u8 size_data,u8 size_ds)
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
//char	*hmacmd5(char *data,u8 size_data,u8 size_ds)
//{
//	char buf[20];
//	char i=0,L=0;
//	char *str=buf;
//	memset(buf,0,20);
//	hmac_md5((unsigned char*)data,size_data,(unsigned char*)DeviceSecret,size_ds,(unsigned char*)str);
//	for(i=0;i<16;i++)
//	{
//		L = strlen(xxbuf);
//		sprintf(xxbuf+L,"%02x",buf[i]);
//	}
//	str = xxbuf;
//	return str;
//}

void	Emw3060_LED_Init(void)				//EMWLED��ʼ��
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //ʹ��PB,PE�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_12;	 //LED0-->PB.5 �˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOB.5
	
	
}

//8421�루BCD�룩תʮ������
unsigned char bcd_to_hex(unsigned char data)
{
	unsigned	char temp;
	temp=((data>>4)*10+(data&0x0f));
	return temp;
}

//ʮ������ת8421��(BCD��)
unsigned	char hex_to_bcd(unsigned	char data)
{
	unsigned	char temp;
	temp=(((data/10)<<4)+(data%10));
	return temp;
}


u8 tiaoshi_cs_1[50];
//EMW3060��ʼ��ʱʹ�ã���ȡNTPʱ��
void dq_ntp_cx()
{
	
	u8 buf[250];
	char *cs_1;
	u8 cs[50];
	u8 i,a=0;
	u8 error=0;//��ʱ�����־
	_calendar_obj calendar2;	//�����ṹ��
	
	
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);

	error=0;
	while(1)	//��ȡNTPʱ��
	{
		u1_printf("AT+SNTPTIME\r");
		delay_ms(5000);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"SNTPTIME:20")[0]=='S')break;
		delay_ms(500);
		comClearRxFifo(COM1);
		delay_ms(500);
		if(++error==10)//��ʱ����
		{
			a=1;
			break;
		}
	}
	
	if(a==1)//��ȡ����ʱ�䳬ʱ
	{
		error=0;
		while(1)//��ȡ������RTCʱ��
		{
			hmi_cmp_rtc[0]=0;
			ReadRTC();//��ȡ��ĻRTCʱ��
			delay_ms(2000);
			if(hmi_cmp_rtc[0]!=0)break;
			if(++error==5)//��ʱ����
			{
				csh_bz=1;
				return;
			}
		}		
	}

	if(a==0)//��ȡ����ʱ��ɹ�
	{
			cs_1=(char *)strchr((const char *)buf,'2'); 
			//2019-06-12  T  16:09:09.030464
			//0123456789  10 11 12  
			
			for(i=0;i<=18;i++)
			{
				cs[i]=cs_1[i]-0x30;
				
				tiaoshi_cs_1[i]=cs[i];//����
			}
			
			//��
			calendar2.w_year=(cs[0]*1000)+(cs[1]*100)+(cs[2]*10)+cs[3];       
			//��
			calendar2.w_month=(cs[5]*10)+cs[6];  			
			//����
			calendar2.w_date=(cs[8]*10)+cs[9];        
			 //Сʱ
			calendar2.hour=(cs[11]*10)+cs[12];  
			 //����
			calendar2.min=(cs[14]*10)+cs[15];        
			 //��
			calendar2.sec=(cs[17]*10)+cs[18];	

      //sec:�� min���� hour��ʱ day���� week������ mon���� year:��
			//���ô�����RTCʱ��
			SetRTC(
			hex_to_bcd(calendar2.sec),	//��
			hex_to_bcd(calendar2.min),  //��
			hex_to_bcd(calendar2.hour),  //ʱ
			hex_to_bcd(calendar2.w_date),   //��
			hex_to_bcd(RTC_Get_Week((cs[2]*10)+cs[3],calendar2.w_month,calendar2.w_date)),		//����
			hex_to_bcd(calendar2.w_month),   //��
			hex_to_bcd((cs[2]*10)+cs[3])	//��			
			);
			
	}
	else	if(a==1)//��ȡ������ʱ��ɹ�
	{
//					hmi_cmp_rtc[0]=Hmi_cmd_buffer[2];//��
//					hmi_cmp_rtc[1]=Hmi_cmd_buffer[3];//��
//					hmi_cmp_rtc[2]=Hmi_cmd_buffer[4];//����
//					hmi_cmp_rtc[3]=Hmi_cmd_buffer[5];//��
//					hmi_cmp_rtc[4]=Hmi_cmd_buffer[6];//Сʱ
//					hmi_cmp_rtc[5]=Hmi_cmd_buffer[7];//��
//					hmi_cmp_rtc[6]=Hmi_cmd_buffer[8];//��

			//��
			calendar2.w_year=2000+bcd_to_hex(hmi_cmp_rtc[0]);       
			//��
			calendar2.w_month=bcd_to_hex(hmi_cmp_rtc[1]);  			
			//����
			calendar2.w_date=bcd_to_hex(hmi_cmp_rtc[3]);         
			 //Сʱ
			calendar2.hour=bcd_to_hex(hmi_cmp_rtc[4]); 
			 //����
			calendar2.min=bcd_to_hex(hmi_cmp_rtc[5]);        
			 //��
			calendar2.sec=bcd_to_hex(hmi_cmp_rtc[6]);				
	}



	RTC_Set(calendar2.w_year,
	calendar2.w_month,
	calendar2.w_date,
	calendar2.hour,
	calendar2.min,
	calendar2.sec);  //����ʱ��
	
	i=calendar2.w_date;
	FlashWriteSJ(&i);//������д��д��flash��������
}


//extern _calendar_obj calendar;	//�����ṹ��
void rct_dueishi_cx()//RCT��ʱ����  ��ѯ����ʱ�䣬����ʱ��ͬ����RTC��ÿ24Сʱͬ��һ��
{
	u8 sj;//֮ǰ����
	u8 dqsj;//��ǰ����
	
	u8 buf[250];
	char *cs_1;
	u8 cs[50];
	u8 i;
	u8 error=0;//��ʱ�����־
	_calendar_obj calendar2;	//�����ṹ��
	
	
	FlashReadSJ(&sj);//��ȡflash�洢��֮ǰ����
	
	tiaoshi_cs_1[30]=RTC_Get();//����ʱ��	calendar.w_date
	
	dqsj=calendar.w_date;//��ǰ����
	tiaoshi_cs_1[31]=dqsj;
//		dqsj=calendar.min;
	tiaoshi_cs_1[28]=sj;
	
	//����ʵʱ��RTCʱ��
	tiaoshi_cs_1[32]=(calendar.w_year/1000);
	tiaoshi_cs_1[33]=(calendar.w_year/100%10);
	tiaoshi_cs_1[34]=(calendar.w_year/10%10);
	tiaoshi_cs_1[35]=(calendar.w_year%10);
	tiaoshi_cs_1[36]=calendar.w_month;
	tiaoshi_cs_1[37]=calendar.w_date;	
	tiaoshi_cs_1[38]=calendar.hour;
	tiaoshi_cs_1[39]=calendar.min;
	tiaoshi_cs_1[40]=calendar.sec;

	
	delay_ms(500);
	if(dqsj!=sj)//���ڸı䣬��ѯ����ʱ�䣬����ʱ��ͬ����RTC��ÿ24Сʱͬ��һ��
	{	
			comClearRxFifo(COM1);
			memset(buf,0,sizeof buf);
		
			error=0;
			while(1)	//��ȡNTPʱ��
			{
				u1_printf("AT+SNTPTIME\r");
				delay_ms(5000);
				COM1GetBuf(buf,200);
				if(strstr((const char *)buf,"SNTPTIME:20")[0]=='S')break;
				delay_ms(500);
				comClearRxFifo(COM1);
	    	delay_ms(500);
				if(++error==10)
				{
					return;
				}
				
			}
			
			cs_1=(char *)strchr((const char *)buf,'2');   
			//2019-06-12  T  16:09:09.030464
			//0123456789  10 11 12                                 
			
			for(i=0;i<=18;i++)
			{
				cs[i]=cs_1[i]-0x30;
				
				tiaoshi_cs_1[i]=cs[i];//����
			}
	
			//��
			calendar2.w_year=(cs[0]*1000)+(cs[1]*100)+(cs[2]*10)+cs[3];       
			//��
			calendar2.w_month=(cs[5]*10)+cs[6];  			
			//����
			calendar2.w_date=(cs[8]*10)+cs[9];        
			 //Сʱ
			calendar2.hour=(cs[11]*10)+cs[12];  
			 //����
			calendar2.min=(cs[14]*10)+cs[15];        
			 //��
			calendar2.sec=(cs[17]*10)+cs[18];
 			
      //����ʱ��
			RTC_Set(calendar2.w_year,
			calendar2.w_month,
			calendar2.w_date,
			calendar2.hour,
			calendar2.min,
			calendar2.sec);  //����ʱ��


      //sec:�� min���� hour��ʱ day���� week������ mon���� year:��
			//���ô�����RTCʱ��
			SetRTC(
			hex_to_bcd(calendar2.sec),	//��
			hex_to_bcd(calendar2.min),  //��
			hex_to_bcd(calendar2.hour),  //ʱ
			hex_to_bcd(calendar2.w_date),   //��
			hex_to_bcd(RTC_Get_Week((cs[2]*10)+cs[3],calendar2.w_month,calendar2.w_date)),		//����
			hex_to_bcd(calendar2.w_month),   //��
			hex_to_bcd((cs[2]*10)+cs[3])	//��			
			);
			
			
			i=calendar2.w_date;
			tiaoshi_cs_1[29]=i;
			FlashWriteSJ(&i);//������д��д��flash��������
			
	}
}


u8 tiaoshi_3060=0;//����ʱʹ�õı������鿴��ǰ���е��ĸ��׼�
void	Emw3060_init(void)  	//EMW��ʼ��
{
	u8 buf[250];
//	u8 error=0;//��ʱ�����־
	
	csh_bz=0;//�����ʼ���ɹ�
	
	Emw3060_LED_Init();//ģ��3060����LED��ʼ������
	
	
	EmwLED2=0;//����ʹ�ã�����D4��1Ϊ����LED�ƣ�
	
	while(1)//ATģʽ
	{
		u1_printf("+++"); //ATָ��ģʽ
		delay_ms(200);
		COM1GetBuf(buf,10);
		if(strchr((const char *)buf,'+')[0]=='+')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);//��buf�е�ǰλ�ú����buf���ֽ� �� 0 �滻������ s 	
tiaoshi_3060=1;/*����*/
	
	while(1)//�ָ�ģ���������
	{
		u1_printf("AT+FACTORY\r");//�ָ�ģ���������
		delay_ms(200);
		COM1GetBuf(buf,100);
		if(strstr((const char *)buf,"OK")[0]=='O')break;	
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
tiaoshi_3060=2;/*����*/
	
	if(DHCP==0)//�ر�DHCP
	{
			while(1)//�ر� DHCP 
			{
				u1_printf("AT+WDHCP=OFF\r");//�ر� DHCP ����
				delay_ms(500);
				COM1GetBuf(buf,30);
				if(strchr((const char *)buf,'O')[0]=='O')break;
			}
			comClearRxFifo(COM1);
			memset(buf,0,sizeof buf);
tiaoshi_3060=3;/*����*/
			
			while(1)//�����������
			{
				u1_printf("AT+WJAPIP=%s,%s,%s,%s\r",ipword,zwword,wgword,ymword);  //���� Station ģʽʱ�� IP ��ַ����IP,�������룬���ص�ַ������dns��
				delay_ms(500);
				COM1GetBuf(buf,100);
				if(strchr((const char *)buf,'O')[0]=='O')break;
			}
			comClearRxFifo(COM1);
			memset(buf,0,sizeof buf);		
tiaoshi_3060=4;/*����*/
	}

//	u1_printf("AT+WJAP=%s,%s\r",ssid,password);//wifi����+wifi����   ������Station ģʽ��Ҫ����� AP �����ƺ����롣
//	delay_ms(2000);
//	u1_printf("AT+WJAP=%s,%s\r",ssid,password);//wifi����+wifi����   ������Station ģʽ��Ҫ����� AP �����ƺ����롣	
	while(1)//����WIFI
	{
		u1_printf("AT+WJAP=%s,%s\r",ssid,password);//wifi����+wifi����   ������Station ģʽ��Ҫ����� AP �����ƺ����롣
		delay_ms(2000);delay_ms(2000);delay_ms(2000);delay_ms(2000);delay_ms(2000);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"STATION_UP")[0]=='S')break;
		comClearRxFifo(COM1);
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);	
tiaoshi_3060=5;/*����*/
	
	while(1)	//���ñ���ʱ��ʱ��
	{
		u1_printf("AT+SNTPCFG=+8\r");
		delay_ms(500);
		COM1GetBuf(buf,30);
		if(strchr((const char *)buf,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
tiaoshi_3060=6;/*����*/

//	error=0;
//	while(1)	//У׼����ʱ��    +SNTPTIME:2019-06-18T09:00:29.001392
//	{
//		u1_printf("AT+SNTPTIME\r");	
//		delay_ms(3000);
//		COM1GetBuf(buf,100);
//		if(strstr((const char *)buf,"SNTPTIME:20")[0]=='S')break;
//		delay_ms(500);
//		comClearRxFifo(COM1);
//		delay_ms(500);
//		if(++error==10)//��ʱ����
//		{
//			csh_bz=1;
//			return;
//		}
//	}
//	comClearRxFifo(COM1);
//	memset(buf,0,sizeof buf);	
//tiaoshi_3060=7;/*����*/
	
	//��ȡNTPʱ�䣬�������ڡ�
	dq_ntp_cx();
tiaoshi_3060=8;/*����*/
	
	delay_ms(500);
}

//MQTTЭ�飺����  ����  ����  ��Ȩ
void	Emw3060_con(void)  	//EMW���Ӱ�����  �������޸�zq��
{
	u8 buf[250];
	char hc[100];
	char cs[40];
	char mac[33];
	char *m;
	u8 error=0;
	
	memset(hc,0,100);
	
	while(1)	//��ȡMAC��ַ
	{
		u1_printf("AT+WMAC?\r");
		delay_ms(200);
		COM1GetBuf(buf,100);
		if(strstr((const char *)buf,"+WMAC:")[0]=='+')break;
	}
	m=strstr((const char *)buf,"+WMAC:")+6;  //+6:��ʾָ���ַ����
	m[12]=0;//0��ʾ�ַ���Ϊ��NULL,���е��ַ������������ж�NULL���գ�0��Ϊ�ַ�����ֹ��
	memset(mac,0,sizeof mac);
	sprintf(mac,"%s",m);
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
tiaoshi_3060=9;/*����*/
	
	
	/*
	Ӧ�ò㣺MQTT֧�ֿͻ���ʶ���û�����������֤��
	�ͻ���ʶ��Client Identifier��һ����˵����ʹ��Ƕ��ʽоƬ��MAC��ַ����оƬ���кš�
	MQTTЭ��֧��ͨ��CONNECT��Ϣ��username��password�ֶη����û��������롣
	
	*/
	sprintf(hc,"clientId%sdeviceName%sproductKey%stimestamp789",mac,DeviceName,ProductKey); //��ʽ������
	while(1)	//д���豸��Ȩ��Ϣ    ����MQTT�û���Ȩ��Ϣ
	{
		hmacmd5(hc,strlen(hc),strlen(DeviceSecret),cs);//����MD5ǩ���㷨
		u1_printf("AT+MQTTAUTH=%s&%s,%s\r",DeviceName,ProductKey,cs);
		delay_ms(200);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);	
tiaoshi_3060=10;/*����*/	
	
	/*
	���ÿͻ���ID
	securemode=3��ģʽ3
	signmethod=hmacmd5��У�鷽ʽ
	timestamp=789��ʱ���
	*/
	while(1)//�����豸CID  ����MQTT�ͻ��˱�ʶ����
	{
		u1_printf("AT+MQTTCID=%s|securemode=3\\,signmethod=hmacmd5\\,timestamp=789|\r",mac);
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
tiaoshi_3060=11;/*����*/		
	
	/*
	����TCP��������   ProductKey:��ƷID    
	1883���������˿ں�
	��������
	*/
	while(1)//����TCPĿ��IP��ַ   ����MQTT socket��Ϣ  
	{
		u1_printf("AT+MQTTSOCK=%s.iot-as-mqtt.cn-shanghai.aliyuncs.com,1883\r",ProductKey);
//		u1_printf("AT+MQTTSOCK=%s.iot-as-mqtt.cn-shanghai.aliyuncs.com,443\r",ProductKey);
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
tiaoshi_3060=12;/*����*/
	
	/*
	ÿ300S�������������������̵�ָ�
	*/
	while(1) //����ʱ������   ����MQTT�������ڡ� 
	{
		u1_printf("AT+MQTTKEEPALIVE=300\r");
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
tiaoshi_3060=13;/*����*/	
	
	/*
	�ͻ���MQTT�Զ������򿪡�
	*/
	while(1)//MQTT�Զ�������   ����MQTT�Զ�����ʹ��״̬
	{
		u1_printf("AT+MQTTRECONN=ON\r");
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
tiaoshi_3060=14;/*����*/
	
	error=0;
	while(1)//MQTT����   ����MQTT����
	{
		u1_printf("AT+MQTTSTART\r");
		delay_ms(5000);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"MQTTEVENT:CONNECT,SUCCESS")[0]=='M')break;
		if(++error==10)//MQTT���ӳ�ʱ����
		{
			csh_bz=1;
			return;
		}
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
tiaoshi_3060=15;/*����*/	
	

//	while(1)//����TOPIC  ����MQTT������Ϣ
//	{
//		u3_printf("AT+MQTTSUB=0,/%s/%s/user/get,1\r",ProductKey,DeviceName);
//		delay_ms(2000);
//		COM1GetBuf(buf,200);
//		if(strstr((const char *)buf,"OK")[0]=='O')break;
//	}
//	comClearRxFifo(COM1);
//	memset(buf,0,sizeof buf);
	
	error=0;
	while(1)//��������  ����TOPIC  ����MQTT������Ϣ
	{
		u1_printf("AT+MQTTSUB=0,/%s/%s/user/irr_set,1\r",ProductKey,DeviceName);
		delay_ms(2000);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"+MQTTEVENT:0,SUBSCRIBE,SUCCESS")[0]=='+')break;
		if(++error==10)//MQTT���ĳ�ʱ����
		{
			csh_bz=1;
			return;
		}
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
		
tiaoshi_3060=16;/*����*/
		
	EmwLED2=1;//����ʹ�ã�����D4��

}


//u8	sendEmw(char *data,unsigned char w)//EMW�ϱ�����  ��zq��
//{
//	u16 len;u8 time;
//	u8 buf[250];
//	char hc[500];
//  u8 sumbz=0;
	/*
	ProductKey:��ƷID     ˮ��һ���
  DeviceName���豸ID    23   24  25
	DeviceSecret���豸����Կ�����룩 
  MQTTЭ��ֻ����ͨ�Ų��֣��û�Э������Լ�ѡ��(MQTT+JSON��Ŀǰ���ŷ���)
	*/
//	
//	while(1)//���÷���topic  //MQTT������Ϣ
//	{
//		if(!w)
//			u3_printf("AT+MQTTPUB=/sys/%s/%s/thing/event/property/post,1\r",ProductKey,DeviceName); //�л��ϱ�topic
//		else u3_printf("AT+MQTTPUB=/%s/%s/user/war,1\r",ProductKey,DeviceName);
//		delay_ms(100);
//		COM1GetBuf(buf,200);
//		if(strstr((const char *)buf,"OK")[0]=='O')break;
//	}
//	comClearRxFifo(COM1);
//	memset(buf,0,sizeof buf);

//	len=0;time=0;	
//	while(1)	//��������ָ��  MQTT��������
//	{
//		u3_printf("AT+MQTTSEND=%d\r",strlen(data)+13);   //�������ݵ��ֽڳ���
//		while(1)
//		{
//			delay_ms(200);
//			COM1GetBuf(buf,20);
//			if(strstr((const char *)buf,">")[0]=='>')break;
//			if(strstr((const char *)buf,"ERROR")[0]=='E')break;//ģ������æ
//			if(++time==10)return 0;//�ж�ģ���ǲ���������
//		}
//		if(strstr((const char *)buf,">")[0]=='>')break;//���Է���������
//		comClearRxFifo(COM1);
//		if(++len==5){return 0;}
//	}
//	memset(buf,0,sizeof buf);
//	
		/*
		ʹ��C�����е�sprintf����JSON�ַ���,��������һ��{��String��:��Hello World!��, ��Value��:12345}JSON�ַ����ˡ�
		
		*/
//	sprintf(hc,"{\"params\":{%s}}",data);   //��ʽҪ����AlinkЭ��
//	len=strlen(hc);
//	if(hc[len-3]==',')hc[len-3]=' ';
//	u3_printf("%s",hc);//��������������
//	
//	delay_ms(3000);
//	COM1GetBuf(buf,100);

//	comClearRxFifo(COM1);
//	if(strstr((const char *)buf,"+MQTTEVENT:PUBLISH,SUCCESS")[0]=='+')sumbz=0;//�����ݷ��ͳɹ���	//�ж��Ƿ���������
//	else sumbz++;
//	if(sumbz>=3)return 0;
//	else return 1;
//}



////EMW�ϱ�����
u8	sendEmw(char *data,unsigned char w)
{
	u8 buf[250];
	u16 len;
	/*
	ProductKey:��ƷID     ˮ��һ���
  DeviceName���豸ID    23   24  25
	*/
	while(1)//MQTT������Ϣ
	{
		if(!w)
			u1_printf("AT+MQTTPUB=/sys/%s/%s/thing/event/property/post,1\r",ProductKey,DeviceName);
		else u1_printf("AT+MQTTPUB=/%s/%s/user/war,1\r",ProductKey,DeviceName);
		delay_ms(100);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	if(COMGetBuf(COM1 ,NETCircle.buf, 500) > 10)DepackRevData();//�����յ������ݣ��������·�������	
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	len=0;
	while(1)
	{
		u1_printf("AT+MQTTSEND=%d\r",strlen(data)); //�������ݵ��ֽڳ���

		delay_ms(50);
		COM1GetBuf(buf,50);

		delay_ms(20);
	  if(COMGetBuf(COM1 ,NETCircle.buf, 500) > 10)DepackRevData();//�����յ������ݣ��������·�������			
		if(strstr((const char *)buf,">")[0]=='>')break;//���Է���������
		comClearRxFifo(COM1);
		delay_ms(500);
		if(++len==10){return 0;} //��ʱ����
	}
	if(COMGetBuf(COM1 ,NETCircle.buf, 500) > 10)DepackRevData();//�����յ������ݣ��������·�������	
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	u1_printf("%s",data);

	delay_ms(5000);//����ʱһ�ᣬ�ȶ�ȡ��+MQTTEVENT:PUBLISH,SUCCESS�����ͳɹ��ı�־
	COM1GetBuf(buf,200);

	if(COMGetBuf(COM1 ,NETCircle.buf, 500) > 10)DepackRevData();//�����յ������ݣ��������·�������	
	
	if(strstr((const char *)buf,"+MQTTEVENT:PUBLISH,SUCCESS")[0]=='+')return 1;//�ϴ����ݳɹ�
	else return 0; //�ϴ�����û�гɹ�
}
/*
�������ݵĸ�ʽ
{\"params\":{\"to_type\":1,\"to_time\":2,\"need_partition\":3,\"switch\":4,\"work_state\":5,\"in_time\":6,\"at_partition\":7}}
*/
void SplicingPubHead(u8* Dest)
{
	strcpy((char*)Dest,"{\"params\":{");  //�Ѵ�src��ַ��ʼ�Һ���NULL���������ַ������Ƶ���dest��ʼ�ĵ�ַ�ռ�
}
void SignedNumberToASCII ( u8 *Arry , s16 Number);//����������
void SplicingPubData(u8*Dest,const u8*Text,s16 Value,u8 FirstF)
{
	u8 u8Str[16];
	if(FirstF >0)
	{
		strcat((char*)Dest,",");///������char��������,�������Dest������
	}
	strcat((char*)Dest,"\"");//����\"
	strcat((char*)Dest,(char*)Text);
	strcat((char*)Dest,"\":");//����\":
	SignedNumberToASCII(u8Str,Value);//16λ�з�����ת����ASCII��
	strcat((char*)Dest,(char*)u8Str);
}
void SplicingPubTail(u8* Dest)
{
	strcat((char*)Dest,"}}");
}
u8 NetSendBuf[256];
void  SplicingPubStr(void)//��ƽ̨�������ݵ����ϣ�����ˮ����ˮ��
{
	memset(NetSendBuf,0,256);//��NetSendBuf����ȫ������
	SplicingPubHead(NetSendBuf);
	SplicingPubData(NetSendBuf,ReadText[0],Remote_Para[0],0);//to_type �������(to_type)    Ĭ��-0����ˮ-1����ˮ-2
	SplicingPubData(NetSendBuf,ReadText[1],Remote_Para[1],1);//to_time ���ʱ����to_time��  ʱ�䳣��
	SplicingPubData(NetSendBuf,ReadText[2],Remote_Para[2],2);//need_partition ��Ҫ��ȷ�����need_partition��  1����-1,2����-2,1��2����-3
	SplicingPubData(NetSendBuf,ReadText[3],Remote_Para[3],3);//switch ��ͣ(switch)    ֹͣ-0������-1
	SplicingPubData(NetSendBuf,ReadText[4],Remote_Read[0],4);//work_state ����״̬��work_state�� ����-0����ˮ-1����ˮ-2
	SplicingPubData(NetSendBuf,ReadText[5],Remote_Read[1],5);//in_time ����ʱ��in_time)
	SplicingPubData(NetSendBuf,ReadText[6],Remote_Read[2],6);//at_partition ��ǰ��ȷ�����at_partition��  1����-1,2����-2  Ӧ�ò���һ������-0
	//SplicingPubData(NetSendBuf,ReadText[7],Remote_Read[3],7);
	SplicingPubTail(NetSendBuf);
}

void  SplicingPubStr_zdms(void)//�Զ�ģʽ��ƽ̨��������
{
	memset(NetSendBuf,0,256);//��NetSendBuf����ȫ������
	SplicingPubHead(NetSendBuf);
	SplicingPubData(NetSendBuf,ReadText[0],Remote_Para[0],0);//to_type �������(to_type)    Ĭ��-0����ˮ-1����ˮ-2
	SplicingPubData(NetSendBuf,ReadText[2],Remote_Para[2],1);//need_partition ��Ҫ��ȷ�����need_partition��  1����-1,2����-2,1��2����-3
	SplicingPubData(NetSendBuf,ReadText[3],Remote_Para[3],2);//switch ��ͣ(switch)    ֹͣ-0������-1
	SplicingPubData(NetSendBuf,ReadText[4],Remote_Read[0],3);//work_state ����״̬��work_state�� ����-0����ˮ-1����ˮ-2
	SplicingPubData(NetSendBuf,ReadText[6],Remote_Read[2],4);//at_partition ��ǰ��ȷ�����at_partition��  1����-1,2����-2  Ӧ�ò���һ������-0
	SplicingPubTail(NetSendBuf);
}

/*
��ѯwifi״̬
*/
void CX_wifi_zt(void)
{
	u8 buf[250];
	static u8 error=0;
	static u8 error1=0;
	static u8 error2=0;
	
	if(++error==5)
	{
		
			if(csh_bz==1)//֮ǰģ��ĳ�ʼ��û����ɣ�����ִ��һ�γ�ʼ��
			{
					Emw3060_init();
					Emw3060_con();				
			}
			else	//ά���������
			{
					//WIFI����������ÿ�������Ժ������ϱ���	
					u1_printf("AT+WJAPS\r");//�鿴��ǰ Station ģʽ������״̬
					delay_ms(200);						
					COM1GetBuf(buf,100);
					if(strstr((const char *)buf,"STATION_UP")[0]!='S')
					{	
							if(++error1==3)
							{ 	
								EmwLED1=1;//����ʹ�ã�����D5��wifi����������ʼ
								
								Emw3060_init();
								Emw3060_con();
								
								EmwLED1=0;//����ʹ�ã�wifi�����������
								error1=0;
							}
					}
					else error1=0;				
					if(COMGetBuf(COM1 ,NETCircle.buf, 500) > 10)DepackRevData();//�����յ������ݣ��������·�������								
					comClearRxFifo(COM1);
					
			
					//��ѯMQTT״̬	   
					//MQTT���ӳɹ� +MQTTSTATUS:CONNECT,SUCCESS
					//MQTT����ʧ�� ������Ҫ��10���ӲŻ���ߣ�
					if(csh_bz==0)//֮ǰģ��ĳ�ʼ���ɹ���
					{
							u1_printf("AT+MQTTSTATUS=connect\r");
							delay_ms(200);
							COM1GetBuf(buf,200);
							if(strstr((const char *)buf,"+MQTTSTATUS:CONNECT,SUCCESS")[0]!='+')
							{
									if(++error2==3)
									{
											EmwLED1=1;//����ʹ�ã�����D5��wifi����������ʼ
										
											Emw3060_init();
											Emw3060_con();//MQTT����֮����Ҫ��������MQTT
										
											EmwLED1=0;//����ʹ�ã�wifi�����������
											error2=0;
									}					
							}
							else	error2=0;
							if(COMGetBuf(COM1 ,NETCircle.buf, 500) > 10)DepackRevData();//�����յ������ݣ��������·�������				
							comClearRxFifo(COM1);				
					}
				
			}
		
			error=0;
	}	
}

/*������-�����ϴ����ݳ���*/
void fsbz_3060_cx(void)
{
		if(fsbz_3060==1)//�������ڷ���������
		{
			fsbz_3060_bf=1; //�ѷ������ݵ�ָ��洢�����ݱ�־��
		}
		else	if(fsbz_3060==0)//���ڲ��Ƿ���״̬
		{
			//��Ϸ������ݵĸ�ʽ
			if(Guangai.zdms==1)
			{
				SplicingPubStr_zdms();//�Զ�ģʽ״̬����������
			}
			else	  //��ͨģʽ�£���������
			{
				SplicingPubStr();	
			}
			fsbz_3060=1;//����һ���ϴ����ݵĳ��� 
		}
}


u8 WifiPara[180];
//EMW����
void network_task(void *pdata)
{
	u16 i=0,i1=0,i2=0;
	
	Remote_Para[0]=1;//�������(to_type)    Ĭ��-0����ˮ-1����ˮ-2
	Remote_Para[1]=1;//���ʱ����to_time��  ʱ�䳣��
	Remote_Para[2]=1;//��Ҫ��ȷ�����need_partition��  1����-1,2����-2,1��2����-3
	Remote_Para[3]=0;//����-1 ֹͣ-0
	SplicingPubStr();//��Ϸ������ݵĸ�ʽ
	fsbz_3060=1;	
	
	
	FlashReadWiFi(WifiPara);//��ȡWIFI��ص�����
	memcpy(ssid,&WifiPara[0],30);//wifi����       //��WifiPara�е�31λ���ݸ��Ƶ�SSID������
	memcpy(password,&WifiPara[30],30);//wifi����
	memcpy(ipword,&WifiPara[60],30);//IP��ַ
	memcpy(zwword,&WifiPara[90],30);//��������
  memcpy(wgword,&WifiPara[120],30);//���ص�ַ
  memcpy(ymword,&WifiPara[150],30);//����	
	
	FlashReadDHCP(&DHCP);//��ȡDHCP֮ǰ������
	if(DHCP==0XFF)//��ʾDHCP֮ǰû�����ù��������޸ĳ�DHCP��
	{
		DHCP=1;//��DHCP
		FlashWriteDHCP(&DHCP);//����DHCP�Ĳ�����FLASH��ַ
	}
	
	delay_ms(1000);
	
	Emw3060_init();	//EMW��ʼ��
	Emw3060_con();	//EMW���Ӱ�����	

	rct_dueishi_cx();//RCT��ʱ����  ��ѯ����ʱ�䣬����ʱ��ͬ����RTC��ÿ24Сʱͬ��һ��
	
	while(1)
	{
		
		if(DHCP_cq==1)//DHCP����֮����������
		{
				FlashReadWiFi(WifiPara);//��ȡWIFI��ص�����
				memcpy(ssid,&WifiPara[0],30);//wifi����       //��WifiPara�е�31λ���ݸ��Ƶ�SSID������
				memcpy(password,&WifiPara[30],30);//wifi����
				memcpy(ipword,&WifiPara[60],30);//IP��ַ
				memcpy(zwword,&WifiPara[90],30);//��������
				memcpy(wgword,&WifiPara[120],30);//���ص�ַ
				memcpy(ymword,&WifiPara[150],30);//����	
					
				Emw3060_init();	//EMW��ʼ��
				Emw3060_con();	//EMW���Ӱ�����		
				DHCP_cq=0;//�������
		
		}
				
		delay_ms(1000);
		
		
		if(COMGetBuf(COM1 ,NETCircle.buf, 500) > 10)
		{
			//�����յ������ݣ��������·�������
			DepackRevData();
		}

				
		CX_wifi_zt();//��ѯwifiģ���״̬
	

		if(++i1==300)//60��һ����  
		{
			 if(csh_bz==0)rct_dueishi_cx();//RCT��ʱ����  ��ѯ����ʱ�䣬����ʱ��ͬ����RTC��ÿ24Сʱͬ��һ��
			 i1=0;
		}
							
		

				
		if((fsbz_3060==1)&&(csh_bz==0))
		{
				delay_ms(1000);			
				
				//���ͺ���,��ƽ̨��������
				i=sendEmw((char *)NetSendBuf,0);
				if(i==1)//���ͳɹ�
				{
					fsbz_3060=0;
					i2=0;
				}		
				else	//����ʧ��
				{
					delay_ms(8000);
					if(++i2==5)//����ʧ��5�Σ�����ִ������MQTT����
					{
							if(csh_bz==0)//֮ǰwifiģ���ʼ���ɹ���������MQTT�Ͽ�������
							{	
								  EmwLED1=1;//����ʹ�ã�����D5�� ���+�̵���
								
									Emw3060_con();	//EMW���Ӱ�����

								  EmwLED1=0;//����ʹ�� 
							}
							i2=0;
					}
				}
		}
		else	if((fsbz_3060==0)&&(csh_bz==0)&&(fsbz_3060_bf==1))//���ͱ�־�����֮ǰ��ʼ�������ҷ��ͱ��ݱ�־Ϊ1
		{

				//��Ϸ������ݵĸ�ʽ
				if(Guangai.zdms==1)
				{
					SplicingPubStr_zdms();//�Զ�ģʽ״̬����������
				}
				else	
				{
					SplicingPubStr();	
				}	
			  fsbz_3060=1;//��Ҫ�ϴ�һ������
			  fsbz_3060_bf=0;//���	
				
		}

		
	}
}

