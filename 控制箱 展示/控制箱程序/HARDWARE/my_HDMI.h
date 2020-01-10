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
#include "kz.h"

uint8 cmd_buffer[CMD_MAX_SIZE];
static int32 test_value = 0;
static uint8 update_en = 0;
static u8 ControlModel=0;	//0Ϊ�ֶ�ģʽ 1Ϊ�Զ�ģʽ
static u8 Model=0;	//0Ϊ��ť���� 1Ϊ����������
u8 SaveButton;
//void SetEnvironment(void);
//void UpdateUI(void);
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
}

//16λ�з�����ת����ASCII��
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
	if(screen_id==0)
	{
		switch (control_id)
		{
			case 1://��������
					if(!ControlModel)
					GetControlValue(0,20);
					break;
			case 2://�ŷ�1����
					if(!ControlModel)
					GetControlValue(0,21);
					break;
			case 3://�ŷ�2����
					if(!ControlModel)
					GetControlValue(0,22);
					break;
			case 4://�ŷ�3����
					if(!ControlModel)
					GetControlValue(0,23);
					break;
			case 5://����1����
					if(!ControlModel)
					{
						if(state==0)
						{
							filllight1open();
						}
						else if(state==1)
						{
							filllight1close();
						}
					}
					break;
			case 6://����2����
					if(!ControlModel)
					{
						if(state==0)
						{
							filllight2open();
						}
						else if(state==1)
						{
							filllight2close();
						}
					}
					break;
			case 7://����3����
					if(!ControlModel)
					{
						if(state==0)
						{
							filllight3open();
						}
						else if(state==1)
						{
							filllight3close();
						}
					}
					break;
			case 8://����
					if(!ControlModel)
					{
						if(state==0)
						{
							moregasopen();
						}
						else if(state==1)
						{
							moregasclose();
						}
					}
					break;
			case 9://����
					if(!ControlModel)
					{
						if(state==0)
						{
							sprayopen();
						}
						else if(state==1)
						{
							sprayclose();
						}
					}
					break;
			case 10://�ֶ��Զ��л�
					if(state==0)
					{
						ControlModel=0;
					}
					else if(state==1)
					{
						ControlModel=1;
					}
					break;
			default:
					break;
		}
	}
	else if(screen_id==2&&control_id==13)	//���水ť���洢���õ��ĸ�����
	{
		SaveButton=1;
	}
	delay_ms(100);
	update_en = 1;
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
//	int32 value;
//	value = StringToInt32(str);
	if(screen_id==0&&control_id==20)
	{
		if(!strcmp((const char*)str,"����ͣ"))
		{
			curtainstop();
		}
		else if(!strcmp((const char*)str,"������"))
		{
			curtainopen();
		}
		else if(!strcmp((const char*)str,"������"))
		{
			curtainclose();
		}
	}
	if(screen_id==0&&control_id==21)
	{
		if(!strcmp((const char*)str,"�ŷ�1��"))
		{
			fan1stop();
		}
		else if(!strcmp((const char*)str,"�ŷ�1��"))
		{
			fan1open();
		}
		else if(!strcmp((const char*)str,"�ŷ�1��"))
		{
			fan1close();
		}
	}
	if(screen_id==0&&control_id==22)
	{
		if(!strcmp((const char*)str,"�ŷ�2��"))
		{
			fan2stop();
		}
		else if(!strcmp((const char*)str,"�ŷ�2��"))
		{
			fan2open();
		}
		else if(!strcmp((const char*)str,"�ŷ�2��"))
		{
			fan2close();
		}
	}
	if(screen_id==0&&control_id==23)
	{
		if(!strcmp((const char*)str,"�ŷ�3��"))
		{
			fan3stop();
		}
		else if(!strcmp((const char*)str,"�ŷ�3��"))
		{
			fan3open();
		}
		else if(!strcmp((const char*)str,"�ŷ�3��"))
		{
			fan3close();
		}
	}
	//UpdateUI();
	update_en = 1;
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
		case 0x55:
			Model=1;
		default:
			break;
	}
}
