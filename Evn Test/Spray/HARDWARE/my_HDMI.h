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

extern char ssid[20];
extern char password[20];
extern char ip[20];
extern char network[20];
extern char gateway[20];
extern char dns[20];
extern u8	DHCP;

uint8 cmd_buffer[CMD_MAX_SIZE];
u8 SaveButton=0,RunButton=0,StopButton=0;

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
	if((screen_id==2)&&(control_id==6)&&(state==1))
	{
		SaveButton=1;
	}
	if((screen_id==3)&&(control_id==3)&&(state==1))
	{
		RunButton=1;
	}
	if((screen_id==4)&&(control_id==4)&&(state==1))
	{
		StopButton=1;
	}
	
	if((screen_id==1)&&(control_id==1))
	{
		IOSTATE.IO1=state;
	}
	if((screen_id==1)&&(control_id==2))
	{
		IOSTATE.IO2=state;
	}
	if((screen_id==1)&&(control_id==3))
	{
		IOSTATE.IO3=state;
	}
	if((screen_id==1)&&(control_id==4))
	{
		IOSTATE.IO4=state;
	}
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
	int32 value;
	value = StringToInt32(str);

	if(screen_id==2&&control_id==1)
	{
		sprintf(ssid,"%s",str);
	}
	else if(screen_id==2&&control_id==2)
	{
		sprintf(password,"%s",str);
	}
	else if(screen_id==2&&control_id==3)
	{
		sprintf(ip,"%s",str);
	}
	else if(screen_id==2&&control_id==4)
	{
		sprintf(network,"%s",str);
	}
	else if(screen_id==2&&control_id==5)
	{
		sprintf(gateway,"%s",str);
	}
	else if(screen_id==2&&control_id==9)
	{
		sprintf(dns,"%s",str);
	}
	else if(screen_id==2&&control_id==11)
	{
		DHCP=value;
	}
	else if(screen_id==3&&control_id==1)
	{
		Count_down=value;
	}
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
						case kCtrlRTC:
							Remaining_time=1;
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
