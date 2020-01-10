#include "sys.h"
#include "hmi_driver.h"
#include "hmi_user_uart.h"
#include "cmd_queue.h"
#include "cmd_process.h"
#include "stdio.h"
#include "stdlib.h"
#include "usart.h"
#include "delay.h"
#include "string.h"
#include "Includes.h"

uint8 cmd_buffer[CMD_MAX_SIZE];

static int32 test_value = 0;
static uint8 update_en = 0;
static u8 group=1;
//static u8 *AirModel;
//static u8 *SoilModel;
//static u8 AirModelNum=1;
//static u8 SoilModelNum=1;
//static u8 AirNum=2;
//static u8 SoilNum=2;

u16	Alarm_airtemp_up=800;
u16	Alarm_airtemp_dn=100;
u16	Alarm_airhumi_up=990;
u16	Alarm_airhumi_dn=100;
u32	Alarm_light_up=50000;
u32	Alarm_light_dn=0;
u16	Alarm_CO2_up=2000;
u16	Alarm_CO2_dn=100;

u16	Alarm_soiltemp_up=800;
u16	Alarm_soiltemp_dn=100;
u16	Alarm_soilhumi_up=990;
u16	Alarm_soilhumi_dn=100;
u16	Alarm_EC_up=0;
u16	Alarm_EC_dn=0;

u8 SaveButton;
#define uCOS_EN       1

char *reverse(char *s)
{
    char temp;
    char *p = s;    //pָ��s��ͷ��
    char *q = s;    //qָ��s��β��
    while(*q)
        ++q;
    q--;
    
    //�����ƶ�ָ�룬ֱ��p��q����
    while(q > p)
    {
        temp = *p;
        *p++ = *q;
        *q-- = temp;
    }
    return s;
}


char *my_itoa(int n)		//����ת�ַ���
{
    int i = 0,isNegative = 0;
    static  char s[35];     //����Ϊstatic������������ȫ�ֱ���
    if((isNegative = n) < 0) //����Ǹ�������תΪ����
    {
        n = -n;
    }
    do      //�Ӹ�λ��ʼ��Ϊ�ַ���ֱ�����λ�����Ӧ�÷�ת
    {
        s[i++] = n%10 + '0';
        n = n/10;
    }while(n > 0);
    
    if(isNegative < 0)   //����Ǹ��������ϸ���
    {
        s[i++] = '-';
    }
    s[i] = '\0';    //�������ַ���������
    return reverse(s);
}


//�ı��ؼ���ʾ����ֵ
void SetTextValueInt32(uint16 screen_id, uint16 control_id,int32 value)
{
	char s[35];
	char *a;
	a=s;
	strcpy(a,my_itoa(value));
	SetTextValue(screen_id,control_id,(uchar *)a);
}

//�ַ���ת����
int32 StringToInt32(uint8 *str)
{
	int32 v = 0;
	v=atoi((char *)str);
	return v;
}//16λ�з�����ת����ASCII��
void SignedNumberToASCII ( u8 *Arry , s16 Number,u8 decimalnum)//����������
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
			if(decimalnum != i)
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
		while(i)//����˳��
		{
			Arry[j++] =strtemp[--i];
		}
		Arry[j++] = 0;
	}
	else
	{
	  u16Number = -Number;
	  if(decimalnum != i)
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

/*! 
 *  \brief  ��ť�ؼ�֪ͨ
 *  \details  ����ť״̬�ı�(�����GetControlValue)ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param state ��ť״̬��0����1����
 */
void NotifyButton(uint16 screen_id, uint16 control_id, uint8  state)
{
	//TODO: ����û�����
//	if(screen_id==0&&control_id==1)//������ť
//	{
//		if(group!=1)
//			group--;
//	}
//	else if(screen_id==0&&control_id==2)//������ť
//	{
//			group++;
//	}
//	else if(screen_id==2&&control_id==10)return ;
//	else if(screen_id==2&&control_id==11)return ;
//	else if(screen_id==2&&control_id==13)	//���水ť���洢���õ��ĸ�����
//	{
//		SaveButton=1;
//	}
	if(screen_id==3&&control_id==1)
	{
		SaveButton=1;
	}
	delay_ms(100);
	
//	SetTextValueInt32(1,5,233);	//���´�����������ʾ
//	update_en = 1;
}

/*! 
 *  \brief  �ı��ؼ�֪ͨ
 *  \details  ���ı�ͨ�����̸���(�����GetControlValue)ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param str �ı��ؼ�����
 */
void NotifyText(uint16 screen_id, uint16 control_id, uint8 *str)
{
	//TODO: ����û�����
	int32 value;
	value = StringToInt32(str);
	if(screen_id==2&&control_id==3)
	{
		Alarm_airtemp_up=value*10;
	}
	else if(screen_id==2&&control_id==4)
	{
		Alarm_airtemp_dn=value*10;
	}
	else if(screen_id==2&&control_id==5)
	{
		Alarm_airhumi_up=value*10;
	}
	else if(screen_id==2&&control_id==6)
	{
		Alarm_airhumi_dn=value*10;
	}
	else if(screen_id==2&&control_id==7)
	{
		Alarm_light_up=value;
	}
	else if(screen_id==2&&control_id==8)
	{
		Alarm_light_dn=value;
	}
	else if(screen_id==2&&control_id==9)
	{
		Alarm_CO2_up=value;
	}
	else if(screen_id==2&&control_id==10)
	{
		Alarm_CO2_dn=value;
	}
	
	else if(screen_id==2&&control_id==11)
	{
		Alarm_soilhumi_up=value*10;
	}
	else if(screen_id==2&&control_id==12)
	{
		Alarm_soilhumi_dn=value*10;
	}
	else if(screen_id==2&&control_id==13)
	{
		Alarm_soiltemp_up=value*10;
	}
	else if(screen_id==2&&control_id==14)
	{
		Alarm_soiltemp_dn=value*10;
	}
	else if(screen_id==2&&control_id==15)
	{
		Alarm_EC_up=value;
	}
	else if(screen_id==2&&control_id==16)
	{
		Alarm_EC_dn=value;
	}
//	if(screen_id==2&&control_id==5)
//	{
//		AirModel=str;
//		if(!strcmp((const char*)AirModel,"�ͺ�1"))
//		{
//			AirModelNum=1;
//		}
//		else if(!strcmp((const char*)AirModel,"�ͺ�2"))
//		{
//			AirModelNum=2;
//		}
//		else if(!strcmp((const char*)AirModel,"�ͺ�3"))
//		{
//			AirModelNum=3;
//		}
//		else if(!strcmp((const char*)AirModel,"�ͺ�4"))
//		{
//			AirModelNum=4;
//		}
//	}
//	else if(screen_id==2&&control_id==7)
//	{
//		SoilModel=str;
//		if(!strcmp((const char*)SoilModel,"�ͺ�1"))
//		{
//			SoilModelNum=1;
//		}
//		else if(!strcmp((const char*)SoilModel,"�ͺ�2"))
//		{
//			SoilModelNum=2;
//		}
//		else if(!strcmp((const char*)SoilModel,"�ͺ�3"))
//		{
//			SoilModelNum=3;
//		}
//		else if(!strcmp((const char*)SoilModel,"�ͺ�4"))
//		{
//			SoilModelNum=4;
//		}
//	}
//	else if(screen_id==2&&control_id==6)
//	{
//		AirNum=value;
//	}
//	else if(screen_id==2&&control_id==8)
//	{
//		SoilNum=value;
//	}
//	//UpdateUI();
//	update_en = 1;
}

/*! 
 *  \brief  ��Ϣ�������̣��˴�һ�㲻��Ҫ����
 *  \param msg ��������Ϣ
 *  \param size ��Ϣ����
 */
void ProcessMessage( PCTRL_MSG msg, uint16 size )
{
	uint8 cmd_type = msg->cmd_type;//ָ������
	uint8 ctrl_msg = msg->ctrl_msg;   //��Ϣ������
	uint8 control_type = msg->control_type;//�ؼ�����
	uint16 screen_id = PTR2U16(&msg->screen_id_high);//����ID
	uint16 control_id = PTR2U16(&msg->control_id_high);//�ؼ�ID
	
	//uint32 value = PTR2U32(msg->param);//��ֵ
	
	switch(cmd_type)
	{		
		case NOTIFY_CONTROL:
			{
				if(ctrl_msg==MSG_GET_CURRENT_SCREEN)//����ID�仯֪ͨ
				{
				}
				else
				{
					switch(control_type)
					{
						case kCtrlButton: //��ť�ؼ�
							NotifyButton(screen_id,control_id,msg->param[1]);
							break;
						case kCtrlText://�ı��ؼ�
							NotifyText(screen_id,control_id,msg->param);
							break;
						default:
							break;
					}
				}			
			}
			break;
		default:
			break;
	}
}
