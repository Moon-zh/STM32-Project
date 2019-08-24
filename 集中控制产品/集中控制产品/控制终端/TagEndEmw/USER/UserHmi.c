#include "UserHmi.h"
#include "UserCore.h"
#include "Delay.h"
#include "FlashDivide.h"
//#include "GM3Dir.h"
#include "hmi_driver.h"
#include "string.h"

extern OS_TMR	CallerTmr;
#define CMD_MAX_BUFFER 128
u8 Hmi_cmd_buffer[CMD_MAX_BUFFER];  //����������ջ�����
u16 s_screen_id;                  //ҳ��ID��ű���
u16 s_control_id;                 //�ؼ�ID��ű���
u8 butonn1_state=0; //��ť1��ʾ/����״̬ 0���� 1��ʾ
u8 butonn2_state=0; //��ť2��ʾ/����״̬ 0���� 1��ʾ
u8 butonn3_state=0;	//��ť3��ʾ/����״̬ 0���� 1��ʾ
u8 butonn4_state=0; //��ť4��ʾ/����״̬ 0���� 1��ʾ
u8 update_count=0;//���½������
char	g_u8UserPassword[9]={0,0,0,0,0,0,0,0,0};//��֤���Ժ�Ҫ����
const char	g_u8AdminPassword[9]={0x31,0x39,0x39,0x31,0x30,0x37,0x30,0x31,0};
char g_u8SysTemPassword[9]={0,0,0,0,0,0,0,0,0};

extern char ssid[31];
extern char password[31];
extern char ipaddr[16];
extern char subnet[16];
extern char gateway[16];
extern char dns[16];

HmiRecord u_HmiRecord;
WorkPara HmiWorkPara;

u8 g_u8DestPage = 0;//������ȷ��ת����zbz
u8 g_u8SrcPage = 0;//������淵����ת����zbz
const u8 StaStr0[15] = "���Ϸʹ�ȳ���";
const u8 StaStr1[15] = "һ�׶ι�ȳ���";
const u8 StaStr2[15] = "���׶ι�ȳ���";
const u8 StaStr3[15] = "���׶ι�ȳ���";
const u8 StaStr4[15] = "�Ľ׶ι�ȳ���";
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

void SignedNumberToASCII ( u8 *Arry , s16 Number,u8 decimalnum);
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
		if((( ( uint8 * ) ( &msg->param ) ) [i] != 0x2d) 
			&&(( ( uint8 * ) ( &msg->param ) ) [i] != 0x2e))//0x2dΪ����
		{
			datatxt = datatxt * 10 + ( ( ( uint8 * ) ( &msg->param ) ) [i++] - 0x30 ); //�ӽ��ջ�����ȡ��һ���ֽ����ݣ���ת����ʮ��������
		}
		else if(( ( uint8 * ) ( &msg->param ) ) [i] != 0x2d)
		{
			u8F = 1;
			i++;
		}
		else if(( ( uint8 * ) ( &msg->param ) ) [i] != 0x2e)
		{
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
void SignedNumberToASCII ( u8 *Arry , s16 Number,u8 decimalnum)//����������
	{
		u8 i = 0 , j = 0,m=0;
		u8 temp = 0;
		u8 strtemp[16] = {0,0,0,0,0,0,0,0};
		u16 u16Number;
		if(Number >= 0)
		{ 
			u16Number = Number;
			do					//�����ַֿ���װ����ASCII��
			{
				if(decimalnum != i||decimalnum==0)
				{
					temp = u16Number % 10;
					strtemp[i++] = temp + 0x30;
					u16Number = u16Number / 10;
				}
				else
				{
					if(decimalnum!=0)
					{
						strtemp[i++] = 0x2e;
					}
					else
						i++;
				}
			}
			while(u16Number);
			if(decimalnum>=i)
			{
					for(m=0;m<decimalnum-i;m++)
					{
						strtemp[i++] = 0x30;
					}
					strtemp[i++] = 0x2e;
					strtemp[i++] = 0x30;
			}
			while(i)//����˳��
			{
				Arry[j++] =strtemp[--i];
			}
			Arry[j++] = 0;
		}
		else
		{
		  u16Number = -Number;
		  if(decimalnum != i||decimalnum==0)
		  {
			  temp = u16Number % 10;
			  strtemp[i++] = temp + 0x30;
			  u16Number = u16Number / 10;
		  }
		  else
		  {
			  if(decimalnum!=0)
			  {
				  strtemp[i++] = 0x2e;
			  }
			  else
				  i++;
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


/*******************************************************************************
����:�ı������ȡ
����:�������е����ֽ�����������Ŀ�������ֵ
����: Number:Ŀ����� msg:����
*******************************************************************************/
u16 ASCIITONUMBER (PCTRL_MSG msg )
{
	u8 i = 0 , j = 0;
//	u8 temp = 0;
	u16 strtemp[4] = {0,0,0,0};
  u16 Number=0;
    for(i=0;i<5;i++)
    {
        if(0==msg->param[i])
        {   j=i;
            break;
        }
    }
    switch(j)
    {
        case 1:
        strtemp[0]=msg->param[0]-0x30;
        Number+=strtemp[0];           
         return Number;
        case 2:
     strtemp[0]=msg->param[0]-0x30;
    Number+=strtemp[0]*10;
    strtemp[1]=msg->param[1]-0x30;
    Number+=strtemp[1]*1;	
        return Number;
        case 3:
     strtemp[0]=msg->param[0]-0x30;
    Number+=strtemp[0]*100;
    strtemp[1]=msg->param[1]-0x30;
    Number+=strtemp[1]*10;	
    strtemp[2]=msg->param[2]-0x30;
    Number+=strtemp[2];            
        return Number;
        case 4:
     strtemp[0]=msg->param[0]-0x30;
    Number+=strtemp[0]*1000;
    strtemp[1]=msg->param[1]-0x30;
    Number+=strtemp[1]*100;	
    strtemp[2]=msg->param[2]-0x30;
    Number+=strtemp[2]*10;
    strtemp[3]=msg->param[3]-0x30;
    Number+=strtemp[3];           
        return Number;
        default:
        break;
    }
return Number;
}
u8 g_u8WifiPara[110];
u8 g_u8DeviceID;
//��������
void StangbyPage(PCTRL_MSG msg, qsize size)
{
	u8 Temp=0;
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u16 u16Control_ID;				   //�ؼ�ID��ű���
	u8	u8ButtonVaule = msg->param[1];
	u16Control_ID = PTR2U16 ( &msg->control_id ); //�ؼ�ID

	if(u16Control_ID == 5 &&u8ButtonVaule == 1)
	{
		//����WIFI��Ϣ������ʾ
		FlashReadWiFi(g_u8WifiPara);
		//WIFI������Ļ��ʾ��������
		SetTextValue(1,1,&g_u8WifiPara[0]);
		SetTextValue(1,2,&g_u8WifiPara[31]);
		SetTextValue(1,3,&g_u8WifiPara[62]);
		SetTextValue(1,4,&g_u8WifiPara[78]);
		SetTextValue(1,5,&g_u8WifiPara[94]);
		FlashReadID(&g_u8DeviceID);
		NumberToASCII(u8Str,g_u8DeviceID);
		SetTextValue(1,8,u8Str);
	}
}
u16 FlashPara[76];
u8 g_u8DHCP=1;
//���OR����
void IrrOrSet(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //�ؼ�ID��ű���
	s16 s16RevVal  = 0;
	u8	u8ButtonVaule = msg->param[1];
	uint32 u32ShowBuf[5]={0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	s16 value=0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //�ؼ�ID
	if(u16Control_ID == 1)
    {
        memcpy(g_u8WifiPara,msg->param,(size-12));
        memcpy(ssid,msg->param,(size-12));
    }
    else if(u16Control_ID == 2)
    {
        memcpy(&g_u8WifiPara[31],msg->param,(size-12));
        memcpy(password,msg->param,(size-12));
    }
    else if(u16Control_ID == 3)
    {
        memcpy(&g_u8WifiPara[62],msg->param,(size-12));
        memcpy(ipaddr,msg->param,(size-12));
    }
    else if(u16Control_ID == 4)
    {
        memcpy(&g_u8WifiPara[78],msg->param,(size-12));
        memcpy(subnet,msg->param,(size-12));
    }
    else if(u16Control_ID == 5)
    {
        memcpy(&g_u8WifiPara[94],msg->param,(size-12));
        memcpy(gateway,msg->param,(size-12));
    }
    else if(u16Control_ID == 11)
    {
        FifureTextInput ( &s16RevVal, msg );
        g_u8DHCP = s16RevVal;
    }
    else if(u16Control_ID == 9)
    {
        memcpy(&g_u8WifiPara[110],msg->param,(size-12));
        memcpy(dns,msg->param,(size-12));
    }
	else if(u16Control_ID == 8)
	{
		FifureTextInput(&value,msg);
		g_u8DeviceID = (u8)value;
	}
	
}
//��ˮѡ���������
void WaterZoneSel(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //�ؼ�ID��ű���
	u8	u8ButtonVaule = msg->param[1];
	u16Control_ID = PTR2U16 ( &msg->control_id ); //�ؼ�ID
	if((u16Control_ID>=1)&&(u16Control_ID<=16))
	{
		if(u8ButtonVaule==0)
		{
			HmiWorkPara.WaterPartition^=(1<<(u16Control_ID-1));
		}
		else if(u8ButtonVaule==1)
		{
			HmiWorkPara.WaterPartition|=(1<<(u16Control_ID-1));
		}
	}
	else if(u16Control_ID == 18)
	{
		if(HmiWorkPara.WaterPartition != 0)
		{
			SetScreen(LCD_WATERSTEP3_PAGE);
		}
	}
}
//��ˮѡ���������
void FerZoneSel(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //�ؼ�ID��ű���
	u8	u8ButtonVaule = msg->param[1];
	u16Control_ID = PTR2U16 ( &msg->control_id ); //�ؼ�ID
	if((u16Control_ID>=1)&&(u16Control_ID<=16))
	{
		if(u8ButtonVaule==0)
		{
			HmiWorkPara.FerPartition^=(1<<(u16Control_ID-1));
		}
		else if(u8ButtonVaule==1)
		{
			HmiWorkPara.FerPartition|=(1<<(u16Control_ID-1));
		}
	}
	else if(u16Control_ID == 18)
	{
		if(HmiWorkPara.FerPartition != 0)
		{
			SetScreen(LCD_FERSTEP3_PAGE);
		}
	}
}
//��ˮ���������� ���ý�ˮʱ��
void WaterTimeSet(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //�ؼ�ID��ű���
	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	MsgStruct Msgtemp;
	MsgStruct MsgtempBlk;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //�ؼ�ID
	if(u16Control_ID >=4 && u16Control_ID<=9)
	{
		HmiWorkPara.Para.WaterTime = (u16Control_ID-4)*30+60;
	}
	else if(u16Control_ID == 1)
	{
		FifureTextInput ( &s16RevVal, msg );
		HmiWorkPara.Para.WaterTime = s16RevVal;
	}
}
//��ˮ������ ѡ���ȳ���
void FerPjSel(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //�ؼ�ID��ű���
	u8	u8ButtonVaule = msg->param[1];
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u16Control_ID = PTR2U16 ( &msg->control_id ); //�ؼ�ID
	if((u16Control_ID >=3)&&(u16Control_ID<=7))
	{
		HmiWorkPara.Para.IrrPjNum = u16Control_ID-2;
	}
	else if(u16Control_ID== 2)
	{
		if(HmiWorkPara.Para.IrrPjNum!=0)
		{
			//������ѡ��ȳ������ʾ��ȷ�ϵ�����
			FlashReadPara((u8*)FlashPara,0,76);
			NumberToASCII(u8Str,HmiWorkPara.Para.IrrPjNum);
			SetTextValue(LCD_FERSTEP4_PAGE,5,u8Str);
			SetTextValue(LCD_FERSTEP4_PAGE,3,(uchar *)StaText[HmiWorkPara.Para.IrrPjNum-1]);
			NumberToASCII(u8Str,(FlashPara[(HmiWorkPara.Para.IrrPjNum-1)*8+36]
							+FlashPara[(HmiWorkPara.Para.IrrPjNum-1)*8+37]
							+FlashPara[(HmiWorkPara.Para.IrrPjNum-1)*8+38]));
			SetTextValue(LCD_FERSTEP4_PAGE,4,u8Str);
			NumberToASCII(u8Str,FlashPara[(HmiWorkPara.Para.IrrPjNum-1)*8+43]);
			SetTextValue(LCD_FERSTEP4_PAGE,6,u8Str);
			SetScreen(LCD_FERSTEP4_PAGE);
		}
	}
}
//���õ�һ�����Ϲ�Ũ������
void PercentSet(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //�ؼ�ID��ű���
	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	uint32 u32ShowBuf[17]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //�ؼ�ID
	if((u16Control_ID>=1)&&(u16Control_ID<=4))
	{
		FifureTextInput ( &s16RevVal, msg );
		FlashPara[u16Control_ID-1] = s16RevVal;
	}
	else if(u16Control_ID == 7)
	{
		for(i=4;i<=19;i++)
		{
			NumberToASCII(u8Str,FlashPara[i]);
			if(u8Str[3]>0)
			{
				u32ShowBuf[i-4] = u8Str[0]<<24|u8Str[1]<<16|u8Str[2]<<8|u8Str[3];
			}
			else if(u8Str[2]>0)
			{
				u32ShowBuf[i-4] = u8Str[0]<<16|u8Str[1]<<8|u8Str[2];
			}
			else if(u8Str[1]>0)
			{
				u32ShowBuf[i-4] = u8Str[0]<<8|u8Str[1];
			}
			else if(u8Str[0]>0)
			{
				u32ShowBuf[i-4] = u8Str[0];
			}
		}
		SetMulSerialText2(LCD_SETSTEP2_PAGE,1,16,u32ShowBuf);
	}
}
//���õڶ��� ��ֲ�������
void AreaSet(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //�ؼ�ID��ű���
	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	uint32 u32ShowBuf[17]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //�ؼ�ID
	
	if((u16Control_ID>=1)&&(u16Control_ID<=16))
	{
		FifureTextInput ( &s16RevVal, msg );
		FlashPara[u16Control_ID-1+4] = s16RevVal;
	}
	else if(u16Control_ID == 19)
	{
		for(i=36;i<=43;i++)
		{
			NumberToASCII(u8Str,FlashPara[i]);
			if(u8Str[3]>0)
			{
				u32ShowBuf[i-36] = u8Str[0]<<24|u8Str[1]<<16|u8Str[2]<<8|u8Str[3];
			}
			else if(u8Str[2]>0)
			{
				u32ShowBuf[i-36] = u8Str[0]<<16|u8Str[1]<<8|u8Str[2];
			}
			else if(u8Str[1]>0)
			{
				u32ShowBuf[i-36] = u8Str[0]<<8|u8Str[1];
			}
			else if(u8Str[0]>0)
			{
				u32ShowBuf[i-36] = u8Str[0];
			}
		}
		SetMulSerialText2(LCD_SETSTEP3_PAGE,1,8,u32ShowBuf);
	}
	
}
//���õ����� ���Ϸʹ�ȳ���
void ComFerSet(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //�ؼ�ID��ű���
	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	uint32 u32ShowBuf[9]={0,0,0,0,0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //�ؼ�ID
	
	if((u16Control_ID>=1)&&(u16Control_ID<=8))
	{
		FifureTextInput ( &s16RevVal, msg );
		FlashPara[u16Control_ID-1+36] = s16RevVal;
	}
	else if(u16Control_ID == 19)
	{
		for(i=44;i<=51;i++)
		{
			NumberToASCII(u8Str,FlashPara[i]);
			if(u8Str[3]>0)
			{
				u32ShowBuf[i-44] = u8Str[0]<<24|u8Str[1]<<16|u8Str[2]<<8|u8Str[3];
			}
			else if(u8Str[2]>0)
			{
				u32ShowBuf[i-44] = u8Str[0]<<16|u8Str[1]<<8|u8Str[2];
			}
			else if(u8Str[1]>0)
			{
				u32ShowBuf[i-44] = u8Str[0]<<8|u8Str[1];
			}
			else if(u8Str[0]>0)
			{
				u32ShowBuf[i-44] = u8Str[0];
			}
		}
		SetMulSerialText2(LCD_SETSTEP4_PAGE,1,8,u32ShowBuf);
	}
	
}
//���õ��Ĳ� 1�׶ι�ȳ���
void Stage1Set(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //�ؼ�ID��ű���
	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	uint32 u32ShowBuf[9]={0,0,0,0,0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //�ؼ�ID
	
	if((u16Control_ID>=1)&&(u16Control_ID<=8))
	{
		FifureTextInput ( &s16RevVal, msg );
		FlashPara[u16Control_ID-1+44] = s16RevVal;
	}
	else if(u16Control_ID == 19)
	{
		for(i=52;i<=59;i++)
		{
			NumberToASCII(u8Str,FlashPara[i]);
			if(u8Str[3]>0)
			{
				u32ShowBuf[i-52] = u8Str[0]<<24|u8Str[1]<<16|u8Str[2]<<8|u8Str[3];
			}
			else if(u8Str[2]>0)
			{
				u32ShowBuf[i-52] = u8Str[0]<<16|u8Str[1]<<8|u8Str[2];
			}
			else if(u8Str[1]>0)
			{
				u32ShowBuf[i-52] = u8Str[0]<<8|u8Str[1];
			}
			else if(u8Str[0]>0)
			{
				u32ShowBuf[i-52] = u8Str[0];
			}
		}
		SetMulSerialText2(LCD_SETSTEP5_PAGE,1,8,u32ShowBuf);
	}
	
}
//���õ��岽 2�׶ι�ȳ���
void Stage2Set(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //�ؼ�ID��ű���
	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	uint32 u32ShowBuf[9]={0,0,0,0,0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //�ؼ�ID
	
	if((u16Control_ID>=1)&&(u16Control_ID<=8))
	{
		FifureTextInput ( &s16RevVal, msg );
		FlashPara[u16Control_ID-1+52] = s16RevVal;
	}
	else if(u16Control_ID == 19)
	{
		for(i=60;i<=67;i++)
		{
			NumberToASCII(u8Str,FlashPara[i]);
			if(u8Str[3]>0)
			{
				u32ShowBuf[i-60] = u8Str[0]<<24|u8Str[1]<<16|u8Str[2]<<8|u8Str[3];
			}
			else if(u8Str[2]>0)
			{
				u32ShowBuf[i-60] = u8Str[0]<<16|u8Str[1]<<8|u8Str[2];
			}
			else if(u8Str[1]>0)
			{
				u32ShowBuf[i-60] = u8Str[0]<<8|u8Str[1];
			}
			else if(u8Str[0]>0)
			{
				u32ShowBuf[i-60] = u8Str[0];
			}
		}
		SetMulSerialText2(LCD_SETSTEP6_PAGE,1,8,u32ShowBuf);
	}
	
}
//���õ����� 3�׶ι�ȳ���
void Stage3Set(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //�ؼ�ID��ű���
	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	uint32 u32ShowBuf[9]={0,0,0,0,0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //�ؼ�ID
	
	if((u16Control_ID>=1)&&(u16Control_ID<=8))
	{
		FifureTextInput ( &s16RevVal, msg );
		FlashPara[u16Control_ID-1+60] = s16RevVal;
	}
	else if(u16Control_ID == 19)
	{
		for(i=68;i<=75;i++)
		{
			NumberToASCII(u8Str,FlashPara[i]);
			if(u8Str[3]>0)
			{
				u32ShowBuf[i-68] = u8Str[0]<<24|u8Str[1]<<16|u8Str[2]<<8|u8Str[3];
			}
			else if(u8Str[2]>0)
			{
				u32ShowBuf[i-68] = u8Str[0]<<16|u8Str[1]<<8|u8Str[2];
			}
			else if(u8Str[1]>0)
			{
				u32ShowBuf[i-68] = u8Str[0]<<8|u8Str[1];
			}
			else if(u8Str[0]>0)
			{
				u32ShowBuf[i-68] = u8Str[0];
			}
		}
		SetMulSerialText2(LCD_SETSTEP7_PAGE,1,8,u32ShowBuf);
	}
}
//���õ��߲� 4�׶ι�ȳ���
void Stage4Set(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //�ؼ�ID��ű���
	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //�ؼ�ID
	
	if((u16Control_ID>=1)&&(u16Control_ID<=8))
	{
		FifureTextInput ( &s16RevVal, msg );
		FlashPara[u16Control_ID-1+68] = s16RevVal;
	}
}
extern u8 tagendid;//�ն�ID

//����
void SaveConfim(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //�ؼ�ID��ű���
	u8	u8ButtonVaule = msg->param[1];
	u16Control_ID = PTR2U16 ( &msg->control_id ); //�ؼ�ID

	if(u16Control_ID == 5)
	{
		FlashWriteWiFi((u8*)g_u8WifiPara);
		FlashWriteID(&g_u8DeviceID);
		tagendid = g_u8DeviceID;
		FlashWriteDHCP(&g_u8DHCP);
		SetScreen(1);
	}
	else if(u16Control_ID==1)
	{
		SetScreen(1);
	}
}
//���ܴ�������Ϣ
void ReceiveFromMonitor( void )
{
	uint8 cmd_type = 0;
	uint8 msg_type = 0;
	PCTRL_MSG msg = 0;
	qsize size = 0;
	size = queue_find_cmd ( Hmi_cmd_buffer, CMD_MAX_BUFFER ); //�ӻ������л�ȡһ��ָ��
	if ( size <= 10 ) //û�н��յ�ָ��
		return;
	msg = ( PCTRL_MSG ) Hmi_cmd_buffer;
	cmd_type = msg->cmd_type;
	switch ( cmd_type ) //ָ������
	{
		case CMD_GIFMOVE_END:
			break;
		case CMD_HAND_SHAKE:
			break;
		case CMD_TYPE_UPDATE_CONTROL://�ؼ�������Ϣ
			msg_type = msg->ctrl_msg;
			switch ( msg_type ) //��Ϣ����
			{
				case kCtrlMsgAnimationEnd://��������
					break;
				case kCtrlMsgCountdownEnd://����ʱ�ؼ���
					break;
				case kCtrlMsgGetCurrentScreen:
					break;
				case kCtrlMsgGetMenuData:
				case kCtrlMsgGetData:
					
					s_screen_id = PTR2U16 ( &msg->screen_id );
					switch( s_screen_id )
					{
						case LCD_STANDBY_PAGE://��������
							StangbyPage(msg,size);
							break;
						case LCD_START_PAGE ://��������
							IrrOrSet(msg,size);
							break;
						case LCD_IRRSTEP1_PAGE://��Ȳ�����һ��
							SaveConfim(msg,size);
							break;
						case LCD_WATERSTEP2_PAGE://��ˮ��ȵڶ���
							WaterZoneSel(msg,size);
							break;
						case LCD_FERSTEP2_PAGE://��ˮ�����ڶ���
							FerZoneSel(msg,size);
							break;
						case LCD_WATERSTEP3_PAGE://��ˮ����������
							WaterTimeSet(msg,size);
							break;		
						case LCD_FERSTEP3_PAGE://��ˮ����������
							FerPjSel(msg,size);
							break;							
						case LCD_FERSTEP4_PAGE://��ˮ�������Ĳ�
							
							break;	
						case LCD_WATERSHOW_PAGE://��ˮ��ʾ
							break;
						case LCD_FERSHOW_PAGE://��ˮ��ʾ
							break;
						case LCD_SETSTEP1_PAGE://���õ�һ��
							PercentSet(msg,size);
							break;
						case LCD_SETSTEP2_PAGE://���õڶ���
							AreaSet(msg,size);
							break;
						case LCD_SETSTEP3_PAGE://���õ�����
							ComFerSet(msg,size);
							break;
						case LCD_SETSTEP4_PAGE://���õ��Ĳ�
							Stage1Set(msg,size);
							break;
						case LCD_SETSTEP5_PAGE://���õ��岽
							Stage2Set(msg,size);
							break;
						case LCD_SETSTEP6_PAGE://���õ�����
							Stage3Set(msg,size);
							break;
						case LCD_SETSTEP7_PAGE://���õ��߲�
							Stage4Set(msg,size);
							break;
						case LCD_SAVE1_PAGE://Ũ�ȱ���
						case LCD_SAVE2_PAGE://�����������
						case LCD_SAVE3_PAGE://���Ϸʱ���
						case LCD_SAVE4_PAGE://һ�׶α���
						case LCD_SAVE5_PAGE://���׶α���
						case LCD_SAVE6_PAGE://���׶α���
						case LCD_SAVE7_PAGE://�Ľ׶α���
							SaveConfim(msg,size);
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}
		default:
			break;
	}
}
u8 TimeUpdateCnt = 0;
u8 Task_percent=0;//����ִ�н��Ȱٷֱ�
/*����AGV�Ĵ�������ʾ��������Ϣ*/
void Task_HMIMonitor ( void * parg )
{
	parg = parg;
	SetScreen(0);
	FlashReadWiFi(g_u8WifiPara);
    FlashReadDHCP(&g_u8DHCP);
	while ( 1 ) 
	{
		delay_ms(100);
		update_count++;
		ReceiveFromMonitor();
	  if(update_count>5) //500ms����һ�ν���
		{
			update_count=0;
			if(butonn1_state==0)//��ɫ��������
			{
				SetTexFrontBColor(0,1,GRAY_COLOR_HMI);//�����ı�����			
			}
			else
				SetTexFrontBColor(0,1,GREEN_COLOR_HMI);//�����ı�����			
			if(butonn2_state==0)//��ɫ��������
			{
				SetTexFrontBColor(0,2,GRAY_COLOR_HMI);//�����ı�����			
			}
			else
				SetTexFrontBColor(0,2,GREEN_COLOR_HMI);//�����ı�����			
			if(butonn3_state==0)//��ɫ��������
			{
				SetTexFrontBColor(0,3,GRAY_COLOR_HMI);//�����ı�����			
			}
			else
				SetTexFrontBColor(0,3,GREEN_COLOR_HMI);//�����ı�����			
			if(butonn4_state==0)//��ɫ��������
			{
				SetTexFrontBColor(0,4,GRAY_COLOR_HMI);//�����ı�����			
			}
			else
				SetTexFrontBColor(0,4,GREEN_COLOR_HMI);//�����ı�����						
		}
	}
}

