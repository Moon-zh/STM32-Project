#include "UserHmi.h"
#include "UserCore.h"
#include "Delay.h"
#include "FlashDivide.h"
#include "Emw3060.h"
#include "rtc.h"
#include "rs485.h"

#include	"cgq.h"
#include	"FlashDivide.h"

#define CMD_MAX_BUFFER 128
u8 Hmi_cmd_buffer[CMD_MAX_BUFFER];  //����������ջ�����
u8 hmi_cmp_rtc[7];//������RTC�ؼ���ʱ��


WorkPara Hmi_Para;


u8 DHCP_sz=1;//����DPCH��ֵ�����ý���ʹ��  ��Ĭ��Ϊ1  ��1Ϊ����DHCP   0��Ϊ�ر�DHCP��





void SignedNumberToASCII ( u8 *Arry , s16 Number);
/*******************************************************************************
����:�����ı������ȡ
����:�������е����ֽ�����������Ŀ�������ֵ
����: pDest:Ŀ����� msg:����
*******************************************************************************/
void FifureTextInput ( s16 *pDest , PCTRL_MSG msg )
{
	s16 datatxt = 0;
	u16 i = 0;
	u8 u8F = 0;
	while ( ( ( uint8 * ) ( &msg->param ) ) [i] )//ASCIIת��Ϊ����
	{
		if(( ( uint8 * ) ( &msg->param ) ) [i] != 0x2d )//0x2dΪ����
		{
			datatxt = datatxt * 10 + ( ( ( uint8 * ) ( &msg->param ) ) [i++] - 0x30 ); //�ӽ��ջ�����ȡ��һ���ֽ����ݣ���ת����ʮ��������
		}
		else
		{
			u8F = 1;
			i++;
		}
	}
	if(u8F == 1)
	{
	   datatxt = datatxt*(-1);
	}
	*pDest = datatxt;
}
/*******************************************************************************
����:�ı������ȡ
����:�������е����ֽ�����������Ŀ�������ֵ
����: pDest:Ŀ����� msg:����
*******************************************************************************/
void TextInput ( u8 *pDest , PCTRL_MSG msg )
{
	s16 datatxt = 0;
	u16 i = 0;
	while ( ( ( uint8 * ) ( &msg->param ) ) [i] )//ASCIIת��Ϊ����
	{
		*(pDest++) = ( ( uint8 * ) ( &msg->param ) ) [i++];
	}
	*pDest = datatxt;
}
/*******************************************************************************
����:����״̬��ȡ
����:�������еİ���״̬������������ֵ��Ŀ�Ĳ���
����: pDest:Ŀ����� msg:����
*******************************************************************************/
void ButtonStatueGet ( u8 *pDest , PCTRL_MSG msg )
{
	u8 buttonvalue = msg->param[1];
	*pDest = buttonvalue;
	
}
/*******************************************************************************
����:�б�ֵ��ȡ
����:�������еİ���״̬������������ֵ��Ŀ�Ĳ���
����: pDest:Ŀ����� msg:����
*******************************************************************************/
void MenuValueGet ( u8 *pDest , PCTRL_MSG msg )
{
	u8 value =  msg->param[0] ; //��ֵ
	*pDest = value;
}
//����ת����ASCII��
void NumberToASCII ( u8 *Arry , u16 Number)
{
	u8 i = 0 , j = 0;
	u8 temp = 0;
	u8 strtemp[16] = {0,0,0,0,0,0,0,0};
	do					//�����ַֿ���װ����ASCII��
	{
		temp = Number % 10;
		strtemp[i++] = temp + 0x30;
		Number = Number / 10;
	}
	while(Number);
	while(i)//����˳��
	{
		Arry[j++] =strtemp[--i];
	}
	Arry[j++] = 0;
	//Arry[j++] = 0x30;
	
}
//16λ�з�����ת����ASCII��
void SignedNumberToASCII ( u8 *Arry , s16 Number)//����������
{
	u8 i = 0 , j = 0;
	u8 temp = 0;
	u8 strtemp[16] = {0,0,0,0,0,0,0,0};
	u16 u16Number;
	if(Number >= 0)
	{ 
		u16Number = Number;
		do					//�����ַֿ���װ����ASCII��
		{
			temp = u16Number % 10;
			strtemp[i++] = temp + 0x30;
			u16Number = u16Number / 10;
		}
		while(u16Number);
		while(i)//����˳��
		{
			Arry[j++] =strtemp[--i];
		}
		Arry[j++] = 0;
	}
	else
	{
	  u16Number = -Number;
		do					//�����ַֿ���װ����ASCII��
		{
			temp = u16Number % 10;
			strtemp[i++] = temp + 0x30;
			u16Number = u16Number / 10;
		}
		while(u16Number);
//		strtemp[i++] = 0x2d;//����
		j = 1;
		while(i)//����˳��
		{
			Arry[j++] =strtemp[--i];
		}
		Arry[0] = 0x2d;//����
		Arry[j++] = 0;
	}
}
//��ʾ�ǵ���ʱ����
//���룺ʣ�����
//�������
//���ܣ���ҳ������ʾ����ʱ
void DisCountDown(u16 RemainderMinute)
{
	u16 ReHour = RemainderMinute/60;
	u16 ReMin = RemainderMinute%60;	
	u8 u8Str[16];
	
	NumberToASCII(u8Str,ReHour);
	SetTextValue(LCD_MAIN_PAGE,47,u8Str);
	
	NumberToASCII(u8Str,ReMin);
	SetTextValue(LCD_MAIN_PAGE,53,u8Str);
	
}
const u8 StaStr0[12] = "һ�ŷ���";
const u8 StaStr1[12] = "���ŷ���";
const u8 StaStr2[12] = "3�׶�";
const u8 StaStr3[12] = "��ˮ���";
const u8 StaStr4[12] = "��ˮ���";
const u8 StaStr5[12] = {0,0,0};
const u8* StaText[] = 
{
	StaStr0,
	StaStr1,
	StaStr2,
	StaStr3,
	StaStr4,
	StaStr5,
};



//����������յ���ָ��
void StandbyPage(PCTRL_MSG msg, qsize size )
{
	u16 u16Control_ID;				   //������Ϣ�Ŀؼ�ID
	
	u8	u8ButtonVaule = msg->param[1]; //ֻȡ��ť״̬�Ĳ��� ���£�1  ����0
	u16Control_ID = PTR2U16 ( &msg->control_id );  //������Ϣ�Ŀؼ�ID

	if((u16Control_ID==1)&&(u8ButtonVaule == 0))
	{
		if(Guangai.zdms==1)//�ѿ����Զ�ģʽ����ʾ�Զ�ģʽ����
		{
				SetScreen(LCD_ZIDONG_MOSHI);	
		}
		else	
		{
				//���û�й���,��ʾ���ý���
				if(GetSysState()==SYSTEM_IDLE)//����״̬
				{
					SetScreen(LCD_MODELSEL_PAGE);   
				}
				//������ڹ���,��ʾ״̬����
				else
				{
					SetScreen(LCD_STATESHOW_PAGE);
				}
			
		}
	}
}

//����״̬��ʾ�����ڹ�Ƚ���
void WorkingState(PCTRL_MSG msg, qsize size )
{
	MsgStruct Msgtemp;
	u16 u16Control_ID;				   //������Ϣ�Ŀؼ�ID
	u8	u8ButtonVaule = msg->param[1]; //ֻȡ��ť״̬�Ĳ��� ���£�1  ����0
	u16Control_ID = PTR2U16 ( &msg->control_id );  //������Ϣ�Ŀؼ�ID

	if((u16Control_ID==5)&&(u8ButtonVaule == 1))//ֹͣ
	{
		//����ֹͣ��Ϣ
		Msgtemp.CmdSrc = SCREEN_TASK_CODE;
		Msgtemp.CmdType = MSG_STOP;  //����
		//Msgtemp.CmdData[0] = 1;//�������(to_type)    Ĭ��-0����ˮ-1����ˮ-2  ƽ̨�жϵĲ���Ϊ0
		Msgtemp.CmdData[1] = 0;//��Ҫ��ȷ�����need_partition��  1����-1,2����-2,1��2����-3
		Msgtemp.CmdData[2] = 0;//���ʱ����Сʱ��
		Msgtemp.CmdData[3] = 0;//���ʱ�������ӣ�	
		PackSendMasterQ(&Msgtemp);
		
	}
}


u8 g_u8WifiPara[200];
//ģʽѡ��
void ModelSel(PCTRL_MSG msg, qsize size )
{
	u8 sz_hc[20];//��������
	u16 u16Control_ID;				  //������Ϣ�Ŀؼ�ID
	u8	u8ButtonVaule = msg->param[1]; //ֻȡ��ť״̬�Ĳ��� ���£�1  ����0
	u16Control_ID = PTR2U16 ( &msg->control_id );  //������Ϣ�Ŀؼ�ID
	
	
	if(u16Control_ID == 1)//��ˮ   (�ڴ��������������л�����+�������¿ؼ���ֵ)
	{
		Hmi_Para.WorkModel = 1;
		Hmi_Para.Zone = 0x03;
	}
	else if(u16Control_ID == 2)//ʩ��
	{
		Hmi_Para.WorkModel = 2;
		Hmi_Para.Zone = 0x03;//Ĭ�Ͽ�ʼ��������
	}
	else if(u16Control_ID == 3)//�豸����
	{
		//��flash�ж�ȡ����
		FlashReadWiFi(g_u8WifiPara);
		//չʾ����
		SetTextValue(LCD_PARASET_PAGE,1,&g_u8WifiPara[0]);//wifi����
		SetTextValue(LCD_PARASET_PAGE,2,&g_u8WifiPara[30]);//wifi����
		SetTextValue(LCD_PARASET_PAGE,3,&g_u8WifiPara[60]);	//IP
		SetTextValue(LCD_PARASET_PAGE,4,&g_u8WifiPara[90]);	//��������
		SetTextValue(LCD_PARASET_PAGE,5,&g_u8WifiPara[120]);//����	
		SetTextValue(LCD_PARASET_PAGE,6,&g_u8WifiPara[150]);//����		
		
		FlashReadDHCP(&DHCP);//��ȡDHCP֮ǰ������
		SetButtonValue(LCD_PARASET_PAGE,9,DHCP);//DHCP״̬
		//��ת�豸WIFI���ý���
		SetScreen(LCD_PARASET_PAGE);
	}
	else	if(u16Control_ID == 4&&u8ButtonVaule==1)//�Զ�ģʽ
	{		
		 
		FlashReadZDMS_QDYZ(&Guangai.qdyzbz);//��flash�ж�ȡ������ֵ
//		Guangai.qdyzbz=15;//�Զ�ģʽ�е� ������ֵ��־	
		SignedNumberToASCII(sz_hc,Guangai.qdyzbz);//ʮ����ת���ַ���
		SetTextValue(8,3,sz_hc);//����������������ֵ
	
		FlashReadZDMS_TZYZ(&Guangai.tzyzbz);//��flash�ж�ȡֹͣ��ֵ		
//		Guangai.tzyzbz=25;//�Զ�ģʽ�е� ֹͣ��ֵ��־	
		SignedNumberToASCII(sz_hc,Guangai.tzyzbz);//ʮ����ת���ַ���
		SetTextValue(8,5,sz_hc);//����������ֹͣ��ֵ		
		
		Guangai.zdms=1;//�����Զ�ģʽ
		Guangai.qdbz=0;//�Զ�ģʽ�رս�ˮ
		Guangai.yichi=0;//��־���			

		//�����������õ�ǰ���״̬
		sprintf((char *)sz_hc,"    ��ˮ���");
		SetTextValue(8,4,sz_hc);			
		//��ת�Զ�ģʽ����
		SetScreen(LCD_ZIDONG_MOSHI);
		
	}
}

//�Զ�ģʽ����
void ZidongSave(PCTRL_MSG msg,qsize size)
{
	s16 yzbz;//��ֵ��־
	u8 sz_hc[8]={0,0,0,0,0,0,0,0};//��������
	MsgStruct Msgtemp;
	
	u16 u16Control_ID;				  //������Ϣ�Ŀؼ�ID
	u8	u8ButtonVaule = msg->param[1];//ֻȡ��ť״̬�Ĳ��� ���£�1  ����0
	u16Control_ID = PTR2U16 ( &msg->control_id );  //������Ϣ�Ŀؼ�ID
		
	if(u16Control_ID==3)//����������ֵ
	{
		  FifureTextInput(&yzbz,msg);
		
			if(yzbz<Guangai.tzyzbz)//�Զ�ģʽ�У����õ�������ֵС��ֹͣ��ֵ
			{
				Guangai.qdyzbz=yzbz;
				FlashWriteZDMS_QDYZ(&Guangai.qdyzbz);//��flash��д��������ֵ		
			}
			else	//���õ�������ֵ���ڵ���ֹͣ��ֵ����������
			{
					SignedNumberToASCII(sz_hc,Guangai.qdyzbz);//ʮ����ת���ַ���
					SetTextValue(8,3,sz_hc);//����������������ֵ			
			}

	}
	else	if(u16Control_ID==5)//����ֹͣ��ֵ
	{

		  FifureTextInput(&yzbz,msg);
		
			if(yzbz>Guangai.qdyzbz)//�Զ�ģʽ�У����õ�ֹͣ��ֵ����������ֵ
			{
				Guangai.tzyzbz=yzbz;
				FlashWriteZDMS_TZYZ(&Guangai.tzyzbz);//��flash��д��ֹͣ��ֵ	
			}
			else	//���õ�ֹͣ��ֵС�ڵ���������ֵ����������
			{
					SignedNumberToASCII(sz_hc,Guangai.tzyzbz);//ʮ����ת���ַ���
					SetTextValue(8,5,sz_hc);//����������ֹͣ��ֵ			
			}
			
	}	
	else	if(u16Control_ID==6&&u8ButtonVaule==1)//���ذ���
	{		
		Guangai.zdms=0;//�ر��Զ�ģʽ
		Guangai.qdbz=0;//�Զ�ģʽ�رս�ˮ
		Guangai.yichi=0;//��־���			
		
		//����ֹͣ��Ϣ
		Msgtemp.CmdSrc = SCREEN_TASK_CODE;
		Msgtemp.CmdType = MSG_STOP;  //����
		//Msgtemp.CmdData[0] = 1;//�������(to_type)    Ĭ��-0����ˮ-1����ˮ-2  ƽ̨�жϵĲ���Ϊ0
		Msgtemp.CmdData[1] = 0;//��Ҫ��ȷ�����need_partition��  1����-1,2����-2,1��2����-3
		Msgtemp.CmdData[2] = 0;//���ʱ����Сʱ��
		Msgtemp.CmdData[3] = 0;//���ʱ�������ӣ�	
		PackSendMasterQ(&Msgtemp);	


	}
}


//���ý���
//��msg->param��ָ�ڴ�������msg->param���ֽڵ�g_u8WifiPara��ָ�ڴ�����
//+1 ��Ϊ�˽��ַ��������'\0'�ַ���β���Ž�����ȥ��+1���ܳ�������
void ParaSet(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				 //������Ϣ�Ŀؼ�ID
	u8	u8ButtonVaule = msg->param[1]; //ֻȡ��ť״̬�Ĳ��� ���£�1  ����0
	u16Control_ID = PTR2U16 ( &msg->control_id );  //������Ϣ�Ŀؼ�ID
	
	if(u16Control_ID == 1)//wifi����
	{
//		memcpy(&g_u8WifiPara[0],msg->param,strlen((char *)msg->param)+1);
		strcpy((char *)g_u8WifiPara+0,(const char*)msg->param);
	}
	else if(u16Control_ID == 2)//wifi����
	{
//		memcpy(&g_u8WifiPara[30],msg->param,strlen((char *)msg->param)+1);
		strcpy((char *)g_u8WifiPara+30,(const char*)msg->param);
	}
	else if(u16Control_ID == 3)//IP
	{
//		memcpy(&g_u8WifiPara[60],msg->param,strlen((char *)msg->param)+1);
		strcpy((char *)g_u8WifiPara+60,(const char*)msg->param);
	}
	else if(u16Control_ID == 4)//��������
	{
//		memcpy(&g_u8WifiPara[90],msg->param,strlen((char *)msg->param)+1);
		strcpy((char *)g_u8WifiPara+90,(const char*)msg->param);
	}
	else if(u16Control_ID == 5)//����
	{
//		memcpy(&g_u8WifiPara[120],msg->param,strlen((char *)msg->param)+1);
		strcpy((char *)g_u8WifiPara+120,(const char*)msg->param);
	}
	else if(u16Control_ID == 6)//����
	{
//		memcpy(&g_u8WifiPara[150],msg->param,strlen((char *)msg->param)+1);
		strcpy((char *)g_u8WifiPara+150,(const char*)msg->param);
	}
	else if(u16Control_ID == 8)//����
	{
		SetScreen(LCD_PARASAVE_PAGE);//��ת����ȷ�Ͻ���
	}
	else	if(u16Control_ID==9)//����DHCP
	{
			if(u8ButtonVaule==0)//��DHCP
			{
				DHCP_sz=0;
			}
			else	if(u8ButtonVaule==1)//��DHCP ��Ĭ�ϣ�
			{
				DHCP_sz=1;
			}
	}
	
}


//����ȷ�Ͻ���
void ParaSave(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				 //������Ϣ�Ŀؼ�ID
	u8	u8ButtonVaule = msg->param[1]; //ֻȡ��ť״̬�Ĳ��� ���£�1  ����0
	u16Control_ID = PTR2U16 ( &msg->control_id );  //������Ϣ�Ŀؼ�ID
	if(u16Control_ID == 5 && u8ButtonVaule ==1)//���水��  5�ǰ�����ţ�1�ǰ���
	{
		FlashWriteWiFi(g_u8WifiPara);//����FLASH����
		//��������֮���������
//		memcpy(ssid,g_u8WifiPara+0,30);//wifi����       //��WifiPara�е�30λ���ݸ��Ƶ�SSID������  (�ڴ渴��)
//		memcpy(password,g_u8WifiPara+30,30);//wifi����
//		memcpy(ipword,g_u8WifiPara+60,30);//IP��ַ
//		memcpy(zwword,g_u8WifiPara+90,30);//��������
//		memcpy(wgword,g_u8WifiPara+120,30);//���ص�ַ
//		memcpy(ymword,g_u8WifiPara+150,30);//����			
		strncpy(ssid,(const char *)g_u8WifiPara+0,30);//wifi����       //��WifiPara�е�30λ���ݸ��Ƶ�SSID������  ���ַ������ƣ�
		strncpy(password,(const char *)g_u8WifiPara+30,30);//wifi����
		strncpy(ipword,(const char *)g_u8WifiPara+60,30);//IP��ַ
		strncpy(zwword,(const char *)g_u8WifiPara+90,30);//��������
		strncpy(wgword,(const char *)g_u8WifiPara+120,30);//���ص�ַ
		strncpy(ymword,(const char *)g_u8WifiPara+150,30);//����			
		
//		if(DHCP!=DHCP_sz)//DHCP���÷����仯����Ҫ����ģ�� (ֻҪ������水������Ҫ��������)
		{
				DHCP=DHCP_sz;//����Ƶ�DHCP���͸�DHCP���档
				FlashWriteDHCP(&DHCP);//����DHCP�Ĳ�����FLASH��ַ
				DHCP_cq=1;//��������,����EMW��ʼ����EMW���Ӱ�����			
		}
		
		SetScreen(LCD_PARASET_PAGE);//��ת�豸���ý���
	}
	else if(u16Control_ID == 1 && u8ButtonVaule ==1)//���ذ���
	{
		SetScreen(LCD_PARASET_PAGE);//��ת�豸���ý���
	}
}


//��ȷ���ѡ��
void ZoneSel(PCTRL_MSG msg, qsize size )
{ 
	char *i;
	u16 u16Control_ID;				   //������Ϣ�Ŀؼ�ID
	u8	u8ButtonVaule = msg->param[1];//ֻȡ��ť״̬�Ĳ��� ���£�1  ����0
	u16Control_ID = PTR2U16 ( &msg->control_id ); //������Ϣ�Ŀؼ�ID
	
	
	//һ�ŷ��������ŷ���ѡ��
	if(u16Control_ID == 3 ||u16Control_ID == 4)//1:Ϊѡ��   0��Ϊ��ѡ��
	{
		if(u8ButtonVaule == 0)
		{
			Hmi_Para.Zone = Hmi_Para.Zone^(1<<(u16Control_ID-3));
		}
		else
		{
			Hmi_Para.Zone = Hmi_Para.Zone|(1<<(u16Control_ID-3));
		}
	}
	else if(u16Control_ID == 2&&u8ButtonVaule==1)//��һ��
	{
		if(Hmi_Para.Zone != 0)//����Ҫ��ȵķ���
		{
				Hmi_Para.TimeH =0;//���ʱ������
	     	Hmi_Para.TimeM =0;
			
			  if(Hmi_Para.WorkModel==1)//��ˮ
				{
						i="20";
//					  strcpy((char *)i, "20"); 
						SetTextValue(3,3,(uchar *)i);//���ʱ�����ı��򣬳�ʼ��Ϊ20����
				}
				else	if(Hmi_Para.WorkModel==2)//��ˮ
				{
						i="25";
			    	SetTextValue(3,3,(uchar *)i);//���ʱ�����ı��򣬳�ʼ��Ϊ25����
				} 	
			  SetScreen(LCD_TIMESET_PAGE);//��ת�����ʱ������
		}
		else   //��ȷ���Ϊ��	
		{
				SetScreen(LCD_ZONESEL_PAGE_KONG);//��ת����ȷ���Ϊ�ս���
				delay_ms(2000);
			  SetScreen(LCD_ZONESEL_PAGE);//��ת����ȷ���ѡ�����
		}
	}
}



//���ʱ������
void TimeSet(PCTRL_MSG msg, qsize size )
{
	u16 u16Control_ID;				  	   //������Ϣ�Ŀؼ�ID
	u8	u8ButtonVaule = msg->param[1];//ֻȡ��ť״̬�Ĳ��� ���£�1  ����0
	s16 s16RevVal  = 0;//����ı�������Ĺ��ʱ��
	MsgStruct Msgtemp;
	u16Control_ID = PTR2U16 ( &msg->control_id ); 	   //������Ϣ�Ŀؼ�ID
	
	if(u16Control_ID >=4 && u16Control_ID<=9)//ʱ��ѡ����ʾ���棨60��210���ӣ�
	{
		u16Control_ID =u16Control_ID-3;
		Hmi_Para.TimeH = (u16Control_ID+1)/2 ;
		Hmi_Para.TimeM = ((u16Control_ID+1)%2)*30;
	}
	else if(u16Control_ID == 2&& u8ButtonVaule ==1 &&MasterState==0)//������������������Ϊ����״̬
	{
		delay_ms(1000);
		if(MasterState==0)//��ʱ1��֮�����ж�һ���ǲ��ǳ��ڿ���״̬
		{
			if(Hmi_Para.WorkModel==1)//��ˮ
			{
				if((Hmi_Para.TimeH==0)&&(Hmi_Para.TimeM<20))//����1    ����2����ȡ��Ļ���ı���Ĺ��ʱ��
				{
					Hmi_Para.TimeM=20;
			//		Hmi_Para.TimeM=3;//����
				}		
			}
			else	if(Hmi_Para.WorkModel==2)//��ˮ
			{
				if((Hmi_Para.TimeH==0)&&(Hmi_Para.TimeM<=20))//����1    ����2����ȡ��Ļ���ı���Ĺ��ʱ��
				{
					Hmi_Para.TimeM=25;
				}		
			}
		
			Msgtemp.CmdSrc = SCREEN_TASK_CODE;
			Msgtemp.CmdType = MSG_START;//����
			Msgtemp.CmdData[0] = Hmi_Para.WorkModel;//���֣�����ˮ����ʩ��
			Msgtemp.CmdData[1] = Hmi_Para.Zone;//Ҫִ�м�������
			Msgtemp.CmdData[2] = Hmi_Para.TimeH;//ʱ�䣺Сʱ
			Msgtemp.CmdData[3] = Hmi_Para.TimeM;//ʱ�䣺����
			PackSendMasterQ(&Msgtemp);//��Ϣ�������
			SetScreen(LCD_STATESHOW_PAGE); //���ڹ�ȣ�״̬��ʾ����
		
		}
			
	}
	else if(u16Control_ID == 3)//��ȡ�ı����е����ݣ��������д����˷�Χ
	{
		FifureTextInput ( &s16RevVal, msg );
		Hmi_Para.TimeH = s16RevVal/60;  //���Сʱ
		Hmi_Para.TimeM = s16RevVal%60;  //��ȷ���
	}
	
}


/*�����ڹ�Ƚ��� 
���� ��ǰ���ģʽ�͵�ǰ��ȷ���

*/
//0:ʩ�� 1:��ˮ   
//���´�������ʾ����
void WorkModelShow(u8 Value,u8 ZoneNum)
{
	SetTextValue(LCD_STATESHOW_PAGE,10,(uchar *)StaText[Value+3]);   //��ǰ���ģʽ
	SetTextValue(LCD_STATESHOW_PAGE,3,(uchar *)StaText[ZoneNum-2]);  //��ǰ��ȷ���
}


//0:1���� 1:2����
void WorkZoneShow(u8 Value)
{
	SetButtonValue(LCD_STATESHOW_PAGE,1,Value);
}


extern  WorkPara IrrPara;
extern CountDownStruct WorkTime;
//��������ʾ����ʱʱ��
void WorkCountShow(u8 hour,u8 minute)
{
	u8 TimeH = hour ;
	u8 TimeM = minute;
	u8 u8Str[7] = {0,0};

	//Ӧ���½��� ���ݽ׶���ʾ����ʱʱ��
	if(GetSysState()==2)//�����������״̬  ʩ�ʽ׶�1
	{
		if(IrrPara.TimeM>=FERTILIZER1TIME)//���Ӵ���10
		{
			TimeM = IrrPara.TimeM-FERTILIZER1TIME+minute;
			TimeH =  IrrPara.TimeH;
		}
		else
		{
			TimeM = IrrPara.TimeM+60-FERTILIZER1TIME+minute;
			TimeH = IrrPara.TimeH-1;
		}
	}
	else if(GetSysState()==3)// ʩ�ʽ׶�2
	{
		TimeM = minute+FERTILIZER3TIME;
	}
	
	NumberToASCII(u8Str,(u16)((u16)(TimeH*60)+(u16)TimeM));////����ת����ASCII��
	SetTextValue(LCD_STATESHOW_PAGE,2,u8Str);//�ڴ������ϸ��µ���ʱʱ�䣨���ӣ�	
	
}


void FerStageShow(u8 Stage)
{
	if(Stage>2)
		return;
	SetTextValue(LCD_STATESHOW_PAGE,4,(uchar *)StaText[Stage]);
}
void FerStateShow(u8 State)
{
	if(State>4)
		return;
	SetTextValue(LCD_STATESHOW_PAGE,8,(uchar *)StaText[State]);
}



/*
//���ܴ�������Ϣ
EE B1 11 00 01 00 01  11          31 32 33        00         FF FC FF FF
         ����ID�ؼ�ID �ı��ؼ�    ASCLL���123     ������
*/
void ReceiveFromMonitor( void )
{
	uint8 cmd_type = 0;//ָ������
	uint8 ctrl_msg = 0;//��Ϣ������
	uint16 screen_id = 0;//����ID

	PCTRL_MSG msg = 0;
	qsize size = 0;
	
	size = queue_find_cmd ( Hmi_cmd_buffer, CMD_MAX_BUFFER ); //�ӻ������л�ȡһ��ָ��	
	
	if(size>0)//���յ�����
	{	
			msg=(PCTRL_MSG)Hmi_cmd_buffer;//�ѽ��յĻ���������ת��Ϊ�ṹ��ģʽ
			cmd_type = msg->cmd_type;//ָ������
			ctrl_msg = msg->ctrl_msg;//��Ϣ������
			screen_id = PTR2U16(&msg->screen_id);//����ID
			
			switch (cmd_type) //ָ������
			{
				case NOTIFY_CONTROL://�ؼ�����֪ͨ  0XB1
				{
						if(ctrl_msg==MSG_GET_CURRENT_SCREEN) //����ID�仯֪ͨ
						{
	
						}
						else     //���������ȡ�ؼ�����ֵ  0X11 MSG_GET_DATA	
						{
					
							switch(screen_id)//����ID
							{							
								case LCD_STANDBY_PAGE://��������
									StandbyPage(msg, size);
									break;
								
								case LCD_STATESHOW_PAGE: //���ڹ�ȣ�״̬��ʾ����
									WorkingState(msg, size);
									break;
								
								case LCD_MODELSEL_PAGE: //ģʽѡ��
									ModelSel(msg, size);
									break;
								
								case LCD_ZONESEL_PAGE: //����ѡ��
									ZoneSel(msg, size);
									break;
								
								case LCD_TIMESET_PAGE: //ʱ������
									TimeSet(msg, size);
									break;
								
								case LCD_PARASET_PAGE: //wifi����
									ParaSet(msg, size);
									break;
								
								case LCD_PARASAVE_PAGE: //�������
									ParaSave(msg, size);
									break;
								
								case LCD_ZIDONG_MOSHI:  //�Զ�ģʽ����
									ZidongSave(msg,size);
									break;
									
								default:
									break;
							}					 
					 }			
				}
				break;
			
				case 0xf7 : //��ȡ������RTCʱ��
				{
					hmi_cmp_rtc[0]=Hmi_cmd_buffer[2];//��
					hmi_cmp_rtc[1]=Hmi_cmd_buffer[3];//��
					hmi_cmp_rtc[2]=Hmi_cmd_buffer[4];//����
					hmi_cmp_rtc[3]=Hmi_cmd_buffer[5];//��
					hmi_cmp_rtc[4]=Hmi_cmd_buffer[6];//Сʱ
					hmi_cmp_rtc[5]=Hmi_cmd_buffer[7];//��
					hmi_cmp_rtc[6]=Hmi_cmd_buffer[8];//��				
				}
				break;

				default:
					break;
			}

			memset(g_RxBuf5,0,UART5_RX_BUF_SIZE);
			comClearRxFifo(COM5);			
		
	}

	

}


void	HDMI_Init(void)	//��������ʼ��
{
	queue_reset();			//��ջ�����
	delay_ms(300);			//����ȴ�300ms
}



u8 RefreshF=0;
extern CountDownStruct WorkTime;
/*����AGV�Ĵ�������ʾ��������Ϣ*/
void Task_HMIMonitor ( void * parg )
{
	parg = parg;
	
	while ( 1 )
	{
		delay_ms(50);
		ReceiveFromMonitor();//���ܴ�������Ϣ
		
		if(RefreshF == 1)
		{		
			delay_ms(50);
			WorkCountShow(WorkTime.hour,WorkTime.min);//��������ʾ����ʱʱ��
			RefreshF = 0;
		}
		
	}
}

