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
static u8 ControlModel=0;	//0为手动模式 1为自动模式
static u8 Model=0;	//0为按钮控制 1为触摸屏控制
u8 SaveButton;
//void SetEnvironment(void);
//void UpdateUI(void);
#define uCOS_EN       1

char *reverse(char *s)
{
    char temp;
    char *p = s;    //p指向s的头部
    char *q = s;    //q指向s的尾部
    while(*q)
        ++q;
    q--;
    
    //交换移动指针，直到p和q交叉
    while(q > p)
    {
        temp = *p;
        *p++ = *q;
        *q-- = temp;
    }
    return s;
}

char *my_itoa(int n)		//整数转字符串
{
    int i = 0,isNegative = 0;
    static  char s[35];     //必须为static变量，或者是全局变量
    if((isNegative = n) < 0) //如果是负数，先转为正数
    {
        n = -n;
    }
    do      //从个位开始变为字符，直到最高位，最后应该反转
    {
        s[i++] = n%10 + '0';
        n = n/10;
    }while(n > 0);
    
    if(isNegative < 0)   //如果是负数，补上负号
    {
        s[i++] = '-';
    }
    s[i] = '\0';    //最后加上字符串结束符
    return reverse(s);
}

//文本控件显示整数值
void SetTextValueInt32(uint16 screen_id, uint16 control_id,int32 value)
{
	char s[35];
	char *a;
	a=s;
	strcpy(a,my_itoa(value));
	SetTextValue(screen_id,control_id,(uchar *)a);
}

//字符串转整数
int32 StringToInt32(uint8 *str)
{
	int32 v = 0;
	v=atoi((char *)str);
	return v;
}

//16位有符号数转换成ASCII码
void SignedNumberToASCII ( u8 *Arry , s16 Number,u8 decimalnum)//有正负的数
{
	u8 i = 0 , j = 0;
	u8 temp = 0;
	u8 strtemp[16] = {0,0,0,0,0,0,0,0};
	u16 u16Number;
	if(Number >= 0)
	{ 
		u16Number = Number;
		do					//将数字分开并装换成ASCII码
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
		while(i)//调整顺序
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
//		strtemp[i++] = 0x2d;//负号
		j = 1;
		while(i)//调整顺序
		{
			Arry[j++] =strtemp[--i];
		}
		Arry[0] = 0x2d;//负号
		Arry[j++] = 0;
	}
}

/*! 
 *  \brief  按钮控件通知
 *  \details  当按钮状态改变(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param state 按钮状态：0弹起，1按下
 */
void NotifyButton(uint16 screen_id, uint16 control_id, uint8  state)
{
	//TODO: 添加用户代码
	if(screen_id==0)
	{
		switch (control_id)
		{
			case 1://窗帘控制
					if(!ControlModel)
					GetControlValue(0,20);
					break;
			case 2://放风1控制
					if(!ControlModel)
					GetControlValue(0,21);
					break;
			case 3://放风2控制
					if(!ControlModel)
					GetControlValue(0,22);
					break;
			case 4://放风3控制
					if(!ControlModel)
					GetControlValue(0,23);
					break;
			case 5://补光1控制
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
			case 6://补光2控制
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
			case 7://补光3控制
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
			case 8://补气
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
			case 9://喷雾
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
			case 10://手动自动切换
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
	else if(screen_id==2&&control_id==13)	//保存按钮，存储设置的四个数据
	{
		SaveButton=1;
	}
	delay_ms(100);
	update_en = 1;
}

/*! 
 *  \brief  文本控件通知
 *  \details  当文本通过键盘更新(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param str 文本控件内容
 */
void NotifyText(uint16 screen_id, uint16 control_id, uint8 *str)
{
	//TODO: 添加用户代码
//	int32 value;
//	value = StringToInt32(str);
	if(screen_id==0&&control_id==20)
	{
		if(!strcmp((const char*)str,"窗帘停"))
		{
			curtainstop();
		}
		else if(!strcmp((const char*)str,"窗帘开"))
		{
			curtainopen();
		}
		else if(!strcmp((const char*)str,"窗帘关"))
		{
			curtainclose();
		}
	}
	if(screen_id==0&&control_id==21)
	{
		if(!strcmp((const char*)str,"放风1关"))
		{
			fan1stop();
		}
		else if(!strcmp((const char*)str,"放风1排"))
		{
			fan1open();
		}
		else if(!strcmp((const char*)str,"放风1吸"))
		{
			fan1close();
		}
	}
	if(screen_id==0&&control_id==22)
	{
		if(!strcmp((const char*)str,"放风2关"))
		{
			fan2stop();
		}
		else if(!strcmp((const char*)str,"放风2排"))
		{
			fan2open();
		}
		else if(!strcmp((const char*)str,"放风2吸"))
		{
			fan2close();
		}
	}
	if(screen_id==0&&control_id==23)
	{
		if(!strcmp((const char*)str,"放风3关"))
		{
			fan3stop();
		}
		else if(!strcmp((const char*)str,"放风3排"))
		{
			fan3open();
		}
		else if(!strcmp((const char*)str,"放风3吸"))
		{
			fan3close();
		}
	}
	//UpdateUI();
	update_en = 1;
}

/*! 
 *  \brief  消息处理流程，此处一般不需要更改
 *  \param msg 待处理消息
 *  \param size 消息长度
 */
void ProcessMessage( PCTRL_MSG msg, uint16 size )
{
	uint8 cmd_type = msg->cmd_type;//指令类型
	uint8 ctrl_msg = msg->ctrl_msg;   //消息的类型
	uint8 control_type = msg->control_type;//控件类型
	uint16 screen_id = PTR2U16(&msg->screen_id_high);//画面ID
	uint16 control_id = PTR2U16(&msg->control_id_high);//控件ID
	
	//uint32 value = PTR2U32(msg->param);//数值
	
	switch(cmd_type)
	{		
		case NOTIFY_CONTROL:
			{
				if(ctrl_msg==MSG_GET_CURRENT_SCREEN)//画面ID变化通知
				{
				}
				else
				{
					switch(control_type)
					{
						case kCtrlButton: //按钮控件
							NotifyButton(screen_id,control_id,msg->param[1]);
							break;
						case kCtrlText://文本控件
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
