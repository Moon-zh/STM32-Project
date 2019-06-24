#include "UserHmi.h"
#include "UserCore.h"
#include "Delay.h"
#include "FlashDivide.h"
#include "Emw3060.h"
#include "Alarm.h"
//#include "ObstScan.h"
//#include "AGVMsg.h"

extern u8 g_u8LockScreenCnt;
extern u8 g_u8LockScreenF;
//u16 u16temp; 
extern OS_TMR	CallerTmr;
#define CMD_MAX_BUFFER 128
u8 Hmi_cmd_buffer[CMD_MAX_BUFFER];  //����������ջ�����
u16 s_screen_id;                  //ҳ��ID��ű���
u16 s_control_id;                 //�ؼ�ID��ű���

u16 s_TargetID;				//������ȷ��ת����zbz
u16 s_ReturnID;				//������淵����ת����zbz

void SignedNumberToASCII ( u8 *Arry , s16 Number,u8 decimalnum);//����������
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
	//SetTextValue(LCD_MAIN_PAGE,47,u8Str);
	
	NumberToASCII(u8Str,ReMin);
	//SetTextValue(LCD_MAIN_PAGE,53,u8Str);
	
}

u8 HmiDestLevel[5]={0,0,0,0,0};
u8 HmiStirTime[5]={0,0,0,0,0};
u8 HmiPercent[5]={0,0,0,0,0};
u16 HmiSize[5]={0,0,0,0,0};
u8 HmiLow[5]={0,0,0,0,0};
void RecoverWateradd(u8 num)
{
	SetButtonValue(LCD_ADDWATER_PAGE,num+4,0);
	if((num == 1) &&(StirState1 == 0))
	{	
		//SetButtonValue(LCD_KEYSTART_PAGE,1,0);
	}
	else if((num == 2) &&(StirState2 == 0))
	{
		//SetButtonValue(LCD_KEYSTART_PAGE,2,0);
	}
	else if((num == 3) &&(StirState3 == 0))
	{
		//SetButtonValue(LCD_KEYSTART_PAGE,3,0);
	}
	else if((num == 4) &&(StirState4 == 0))
	{
		//SetButtonValue(LCD_KEYSTART_PAGE,4,0);
	}
	else if((num == 5) &&(StirState5 == 0))
	{
		//SetButtonValue(LCD_KEYSTART_PAGE,5,0);
	}
}
void SetWateradd(u8 num )
{
	SetButtonValue(LCD_ADDWATER_PAGE,num,1);
}
void RecoverStir(u8 num)
{
	SetButtonValue(LCD_STIRE_PAGE,num+8,0);
	if((num == 1) &&(WaterAddState1 == 0))
	{	
		//SetButtonValue(LCD_KEYSTART_PAGE,1,0);
	}
	else if((num == 2) &&(WaterAddState2 == 0))
	{
		//SetButtonValue(LCD_KEYSTART_PAGE,2,0);
	}
	else if((num == 3) &&(WaterAddState3 == 0))
	{
		//SetButtonValue(LCD_KEYSTART_PAGE,3,0);
	}
	else if((num == 4) &&(WaterAddState4 == 0))
	{
		//SetButtonValue(LCD_KEYSTART_PAGE,4,0);
	}
	else if((num == 5) &&(WaterAddState5 == 0))
	{
		//SetButtonValue(LCD_KEYSTART_PAGE,5,0);
	}
}
void SetStir(u8 num)
{
	//SetButtonValue(LCD_STIR_PAGE,num,1);
}
//עˮ��������
void WaterAddOperationShow(void)
{
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	FlashReadWaterADD(HmiDestLevel);
	NumberToASCII(u8Str,(u16)HmiDestLevel[0]);
	SetTextValue(LCD_ADDWATER_PAGE,11,u8Str);
	NumberToASCII(u8Str,(u16)HmiDestLevel[1]);
	SetTextValue(LCD_ADDWATER_PAGE,12,u8Str);
	NumberToASCII(u8Str,(u16)HmiDestLevel[2]);
	SetTextValue(LCD_ADDWATER_PAGE,13,u8Str);
	NumberToASCII(u8Str,(u16)HmiDestLevel[3]);
	SetTextValue(LCD_ADDWATER_PAGE,14,u8Str);
	NumberToASCII(u8Str,(u16)HmiDestLevel[4]);
	SetTextValue(LCD_ADDWATER_PAGE,15,u8Str);
}
//�����������
void StirOperationShow(void)
{
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	FlashReadStir(HmiStirTime);
	NumberToASCII(u8Str,(u16)HmiStirTime[0]);
//	SetTextValue(LCD_STIR_PAGE,11,u8Str);
	NumberToASCII(u8Str,(u16)HmiStirTime[1]);
//	SetTextValue(LCD_STIR_PAGE,12,u8Str);
	NumberToASCII(u8Str,(u16)HmiStirTime[2]);
//	SetTextValue(LCD_STIR_PAGE,13,u8Str);
	NumberToASCII(u8Str,(u16)HmiStirTime[3]);
//	SetTextValue(LCD_STIR_PAGE,14,u8Str);
	NumberToASCII(u8Str,(u16)HmiStirTime[4]);
//	SetTextValue(LCD_STIR_PAGE,15,u8Str);
}
//�������ý���
void ParaSetShow(void)
{
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	
	FlashReadWaterADD(HmiDestLevel);
	FlashReadStir(HmiStirTime);
	FlashReadPersent(HmiPercent);
	
	NumberToASCII(u8Str,(u16)HmiDestLevel[0]);
	SetTextValue(LCD_SET_PAGE,1,u8Str);
	NumberToASCII(u8Str,(u16)HmiDestLevel[1]);
	SetTextValue(LCD_SET_PAGE,2,u8Str);
	NumberToASCII(u8Str,(u16)HmiDestLevel[2]);
	SetTextValue(LCD_SET_PAGE,3,u8Str);
	NumberToASCII(u8Str,(u16)HmiDestLevel[3]);
	SetTextValue(LCD_SET_PAGE,4,u8Str);
	NumberToASCII(u8Str,(u16)HmiDestLevel[4]);
	SetTextValue(LCD_SET_PAGE,5,u8Str);
	
	NumberToASCII(u8Str,(u16)HmiStirTime[0]);
	SetTextValue(LCD_SET_PAGE,6,u8Str);
	NumberToASCII(u8Str,(u16)HmiStirTime[1]);
	SetTextValue(LCD_SET_PAGE,7,u8Str);
	NumberToASCII(u8Str,(u16)HmiStirTime[2]);
	SetTextValue(LCD_SET_PAGE,8,u8Str);
	NumberToASCII(u8Str,(u16)HmiStirTime[3]);
	SetTextValue(LCD_SET_PAGE,9,u8Str);
	NumberToASCII(u8Str,(u16)HmiStirTime[4]);
	SetTextValue(LCD_SET_PAGE,10,u8Str);
	
	NumberToASCII(u8Str,(u16)HmiPercent[0]);
	SetTextValue(LCD_SET_PAGE,13,u8Str);
	NumberToASCII(u8Str,(u16)HmiPercent[1]);
	SetTextValue(LCD_SET_PAGE,14,u8Str);
	NumberToASCII(u8Str,(u16)HmiPercent[2]);
	SetTextValue(LCD_SET_PAGE,15,u8Str);
	NumberToASCII(u8Str,(u16)HmiPercent[3]);
	SetTextValue(LCD_SET_PAGE,16,u8Str);
	NumberToASCII(u8Str,(u16)HmiPercent[4]);
	SetTextValue(LCD_SET_PAGE,17,u8Str);
	
	
}

//���ܽ���עˮ���淢���ı���
void AddWaterPage(PCTRL_MSG msg, qsize size)
{
	
	MsgStruct HmiMsg;
	u16 u16Control_ID;				   //�ؼ�ID��ű���
	u8	u8ButtonVaule = msg->param[1];
//	OS_CPU_SR  cpu_sr;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //�ؼ�ID
	//�������������Ϣ
	if((u8ButtonVaule == 0x01)&&(u16Control_ID<=8))
	{
		HmiMsg.CmdType = MSG_WATERADDSTART;
		HmiMsg.CmdSrc = SCREEN_TASK_CODE;
		HmiMsg.CmdData[0] = (u8)u16Control_ID-4;
		PackSendWaterAddQ(&HmiMsg);
	}
	else if((u8ButtonVaule == 0x00)&&(u16Control_ID<=8))
	{
		HmiMsg.CmdType = MSG_WATERADDCANCEL;
		HmiMsg.CmdSrc = SCREEN_TASK_CODE;
		HmiMsg.CmdData[0] = (u8)u16Control_ID-4;
		PackSendWaterAddQ(&HmiMsg);
	}
	
}

//���ս���������淢���ı���
void StirPage(PCTRL_MSG msg, qsize size)
{
	MsgStruct HmiMsg;
	u16 u16Control_ID;				   //�ؼ�ID��ű���
	u8	u8ButtonVaule = msg->param[1];
//	OS_CPU_SR  cpu_sr;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //�ؼ�ID
	//�������������Ϣ
	
	if((u8ButtonVaule == 0x01)&&(u16Control_ID<=12))
	{
		HmiMsg.CmdType = MSG_STIRSTART;
		HmiMsg.CmdSrc = SCREEN_TASK_CODE;
		HmiMsg.CmdData[0] = (u8)u16Control_ID-8;
		PackSendStirQ(&HmiMsg);
	}
	else if(((u8ButtonVaule == 0x00)&&(u16Control_ID<=12)))
	{
		HmiMsg.CmdType = MSG_STIRCANCEL;
		HmiMsg.CmdSrc = SCREEN_TASK_CODE;
		HmiMsg.CmdData[0] = (u8)u16Control_ID-8;
		PackSendStirQ(&HmiMsg);
	}
}
//���ղ������ý��淢���ı���
void ParaSetPage(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //�ؼ�ID��ű���
//	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
//	OS_CPU_SR  cpu_sr;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //�ؼ�ID
	if(u16Control_ID >=1&&u16Control_ID<=4)
	{
		FifureTextInput ( &s16RevVal, msg );
		HmiSize[u16Control_ID-1] =  s16RevVal;
	}
	else if(u16Control_ID >=5&&u16Control_ID<=8)
	{
		FifureTextInput ( &s16RevVal, msg );
		HmiPercent[u16Control_ID-5] = (u8)s16RevVal;
	}
	else if(u16Control_ID >=9&&u16Control_ID<=12)
	{
		FifureTextInput ( &s16RevVal, msg );
		HmiDestLevel[u16Control_ID-9] = (u8)s16RevVal;
	}
	else if(u16Control_ID >=13&&u16Control_ID<=16)
	{
		FifureTextInput ( &s16RevVal, msg );
		HmiLow[u16Control_ID-13] = (u8)s16RevVal;
	}
	else if(u16Control_ID >=17&&u16Control_ID<=20)
	{
		FifureTextInput ( &s16RevVal, msg );
		HmiStirTime[u16Control_ID-17] = (u8)s16RevVal;
	}
}
void SavePara(PCTRL_MSG msg, qsize size)
{
		u16 u16Control_ID;				   //�ؼ�ID��ű���
		MsgStruct Msgtemp;
		u8	u8ButtonVaule = msg->param[1];
//		s16 s16RevVal  = 0;
	//	OS_CPU_SR  cpu_sr;
		u16Control_ID = PTR2U16 ( &msg->control_id ); //�ؼ�ID
	if(u16Control_ID == 1&&u8ButtonVaule == 1)
	{
		FlashWriteWaterADD(HmiDestLevel);
		FlashWriteStir(HmiStirTime);
		FlashWritePersent(HmiPercent);
		FlashWriteD((u8*)HmiSize);
		FlashWriteLow(HmiLow);
 		ParaApplication();
		//�����������
		//SetMulRemoteRead(4,(s16*)HmiSize,4);
		//SetMulRemoteRead(8,(s16*)HmiPercent,4);
		//SetMulRemoteRead(12,(s16*)HmiDestLevel,4);
		//SetMulRemoteRead(16,(s16*)HmiLow,4);
		//SetMulRemoteRead(20,(s16*)HmiStirTime,4);
		//���������ϱ�
		Msgtemp.CmdType = UPLOAD_DIAMETER;
		Msgtemp.CmdSrc = SCREEN_TASK_CODE;
		PackSendRemoteQ(&Msgtemp);
		
		Msgtemp.CmdType = UPLOAD_PERCENT;
		PackSendRemoteQ(&Msgtemp);
		
		Msgtemp.CmdType = UPLOAD_UPPER;
		PackSendRemoteQ(&Msgtemp);
		
		Msgtemp.CmdType = UPLOAD_LOWER;
		PackSendRemoteQ(&Msgtemp);
		
		Msgtemp.CmdType = UPLOAD_STIRTIME;
		PackSendRemoteQ(&Msgtemp);
	}
}
u8 g_u8WifiPara[62];

void StartPage(PCTRL_MSG msg, qsize size)
{
		u16 u16Control_ID;				   //�ؼ�ID��ű���
//		u8	u8ButtonVaule = msg->param[1];
//		s16 s16RevVal  = 0;
 		u16Control_ID = PTR2U16 ( &msg->control_id ); //�ؼ�ID
		if(u16Control_ID == 2 )
		{
			ParaShow();
		}
		else if(u16Control_ID == 3)
		{
			//��flash�ж�ȡ����
			FlashReadWiFi(g_u8WifiPara);
			//չʾ����
			SetTextValue(LCD_WIFISET_PAGE,3,g_u8WifiPara);
			SetTextValue(LCD_WIFISET_PAGE,1,&g_u8WifiPara[31]);
			//��ת����
//SetScreen(LCD_WIFISET_PAGE);
		}
}

void WiFiSet(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //�ؼ�ID��ű���
	u8	u8ButtonVaule = msg->param[1];
	u16Control_ID = PTR2U16 ( &msg->control_id ); //�ؼ�ID
	if(u16Control_ID == 3)
	{
		memcpy(g_u8WifiPara,msg->param,(size-12));
	}
	else if(u16Control_ID == 1)
	{
		memcpy(&g_u8WifiPara[31],msg->param,(size-12));
	}
	else if(u16Control_ID == 5)
	{
		SetScreen(LCD_WIFISAVE_PAGE);
	}
}
void WiFiSave(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //�ؼ�ID��ű���
	u8	u8ButtonVaule = msg->param[1];
	u16Control_ID = PTR2U16 ( &msg->control_id ); //�ؼ�ID
	if(u16Control_ID == 1 && u8ButtonVaule ==1)
	{
		FlashWriteWiFi(g_u8WifiPara);
		SetScreen(LCD_WIFISET_PAGE);
	}
	else if(u16Control_ID == 2 && u8ButtonVaule ==1)
	{
		SetScreen(LCD_WIFISET_PAGE);
	}
}

//���ܴ�������Ϣ
void ReceiveFromMonitor( void )
{
	uint8 cmd_type = 0;
	uint8 msg_type = 0;
//	uint8 control_type = 0;
//	static u16 IdleCnt = 0;
	PCTRL_MSG msg = 0;
	qsize size = 0;
	size = queue_find_cmd ( Hmi_cmd_buffer, CMD_MAX_BUFFER ); //�ӻ������л�ȡһ��ָ��
	if ( size <= 10 ) //û�н��յ�ָ��
		return;
//		continue;
//	IdleCnt = 0;//�յ�������ռ���
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
						case LCD_STANDBY_PAGE:
							break;
						case LCD_START_PAGE:
							StartPage(msg, size);
							break;
						case LCD_ADDFER_PAGE:
							break;
						case LCD_ADDWATER_PAGE:
							AddWaterPage(msg, size);
							break;
						case LCD_STIRE_PAGE:
							StirPage(msg, size);
							break;
						case LCD_SET_PAGE:
							ParaSetPage(msg, size);
							break;
						case LCD_SAVE_PAGE:
							SavePara(msg, size);
							break;
						case LCD_WIFISET_PAGE:
							WiFiSet(msg, size);
							break;
						case LCD_WIFISAVE_PAGE:
							WiFiSave(msg, size);
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
//����Һλ
void UpdateLevel(u16* data,u8 PageNum)
{
	uint32 u32ShowBuf[5]={0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	
	for(i=0;i<4;i++)
	{
		NumberToASCII(u8Str,data[i]/10);// /10Ϊ����ʾ��λΪCM
		if(u8Str[3]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<24|u8Str[1]<<16|u8Str[2]<<8|u8Str[3];
		}
		else if(u8Str[2]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<16|u8Str[1]<<8|u8Str[2];
		}
		else if(u8Str[1]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<8|u8Str[1];
		}
		else if(u8Str[0]>0)
		{
			u32ShowBuf[i] = u8Str[0];
		}
	}
		SetMulSerialText(LCD_ADDWATER_PAGE,1,4,u32ShowBuf);
		SetMulSerialText(LCD_STIRE_PAGE,1,4,u32ShowBuf);
		SetMulSerialText(LCD_ADDFER_PAGE,1,4,u32ShowBuf);
}
//���½���ʱ��
void UpdateStirTime(u8* data)
{
	uint32 u32ShowBuf[9]={0,0,0,0,0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	for(i=0;i<8;i++)
	{
		NumberToASCII(u8Str,data[i]);
		if(u8Str[3]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<24|u8Str[1]<<16|u8Str[2]<<8|u8Str[3];
		}
		else if(u8Str[2]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<16|u8Str[1]<<8|u8Str[2];
		}
		else if(u8Str[1]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<8|u8Str[1];
		}
		else if(u8Str[0]>0)
		{
			u32ShowBuf[i] = u8Str[0];
		}
	}
		SetMulSerialText(LCD_STIRE_PAGE,21,8,u32ShowBuf);
}
extern u16 BucketFeitilizer[5];
void UpdateFertilizer(void)
{
	uint32 u32ShowBuf[5]={0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	//CaluculateFertilizer();
	FertilizerCalculate();
	for(i=0;i<4;i++)
	{
		NumberToASCII(u8Str,BucketFeitilizer[i]);
		if(u8Str[3]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<24|u8Str[1]<<16|u8Str[2]<<8|u8Str[3];
		}
		else if(u8Str[2]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<16|u8Str[1]<<8|u8Str[2];
		}
		else if(u8Str[1]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<8|u8Str[1];
		}
		else if(u8Str[0]>0)
		{
			u32ShowBuf[i] = u8Str[0];
		}
	}
		SetMulSerialText(LCD_ADDFER_PAGE,5,4,u32ShowBuf);
}
void ParaShow(void)
{
	uint32 u32ShowBuf[5]={0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	FlashReadWaterADD(HmiDestLevel);
	FlashReadStir(HmiStirTime);
	FlashReadPersent(HmiPercent);
	FlashReadLow(HmiLow);
	FlashReadD((u8*)HmiSize);
	for(i=0;i<4;i++)
	{
		NumberToASCII(u8Str,HmiSize[i]);
		if(u8Str[3]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<24|u8Str[1]<<16|u8Str[2]<<8|u8Str[3];
		}
		else if(u8Str[2]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<16|u8Str[1]<<8|u8Str[2];
		}
		else if(u8Str[1]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<8|u8Str[1];
		}
		else if(u8Str[0]>0)
		{
			u32ShowBuf[i] = u8Str[0];
		}
	}
	SetMulSerialText(LCD_SET_PAGE,1,4,u32ShowBuf);
	for(i=0;i<4;i++)
	{
		NumberToASCII(u8Str,HmiPercent[i]);
		if(u8Str[3]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<24|u8Str[1]<<16|u8Str[2]<<8|u8Str[3];
		}
		else if(u8Str[2]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<16|u8Str[1]<<8|u8Str[2];
		}
		else if(u8Str[1]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<8|u8Str[1];
		}
		else if(u8Str[0]>0)
		{
			u32ShowBuf[i] = u8Str[0];
		}
	}
	SetMulSerialText(LCD_SET_PAGE,5,4,u32ShowBuf);
	for(i=0;i<4;i++)
	{
		NumberToASCII(u8Str,HmiDestLevel[i]);
		if(u8Str[3]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<24|u8Str[1]<<16|u8Str[2]<<8|u8Str[3];
		}
		else if(u8Str[2]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<16|u8Str[1]<<8|u8Str[2];
		}
		else if(u8Str[1]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<8|u8Str[1];
		}
		else if(u8Str[0]>0)
		{
			u32ShowBuf[i] = u8Str[0];
		}
	}
	SetMulSerialText(LCD_SET_PAGE,9,4,u32ShowBuf);
	for(i=0;i<4;i++)
	{
		NumberToASCII(u8Str,HmiLow[i]);
		if(u8Str[3]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<24|u8Str[1]<<16|u8Str[2]<<8|u8Str[3];
		}
		else if(u8Str[2]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<16|u8Str[1]<<8|u8Str[2];
		}
		else if(u8Str[1]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<8|u8Str[1];
		}
		else if(u8Str[0]>0)
		{
			u32ShowBuf[i] = u8Str[0];
		}
	}
	SetMulSerialText(LCD_SET_PAGE,13,4,u32ShowBuf);
	for(i=0;i<4;i++)
	{
		NumberToASCII(u8Str,HmiStirTime[i]);
		if(u8Str[3]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<24|u8Str[1]<<16|u8Str[2]<<8|u8Str[3];
		}
		else if(u8Str[2]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<16|u8Str[1]<<8|u8Str[2];
		}
		else if(u8Str[1]>0)
		{
			u32ShowBuf[i] = u8Str[0]<<8|u8Str[1];
		}
		else if(u8Str[0]>0)
		{
			u32ShowBuf[i] = u8Str[0];
		}
	}
	SetMulSerialText(LCD_SET_PAGE,17,4,u32ShowBuf);
}
u8 TimeUpdateCnt = 0;
u8 HmiStirTimeSZ[9];
extern u8 SysStirTime[5];
void TimeUpdate(void)
{
	HmiStirTimeSZ[0] = StirTimeMin[0];
	HmiStirTimeSZ[1] = _60SecCnt[0];
	HmiStirTimeSZ[2] = StirTimeMin[1];
	HmiStirTimeSZ[3] = _60SecCnt[1];
	HmiStirTimeSZ[4] = StirTimeMin[2];
	HmiStirTimeSZ[5] = _60SecCnt[2];
	HmiStirTimeSZ[6] = StirTimeMin[3];
	HmiStirTimeSZ[7] = _60SecCnt[3];
}
/*����AGV�Ĵ�������ʾ��������Ϣ*/
void Task_HMIMonitor ( void * parg )
{
	
	parg = parg;
	while ( 1 )
	{
		delay_ms(50);
		ReceiveFromMonitor();
		if(TimeUpdateCnt++>10)
		{
			TimeUpdateCnt = 0;
			UpdateLevel(LevelData,0);
			TimeUpdate();
			UpdateStirTime(HmiStirTimeSZ);
			UpdateFertilizer();
		}
	}
}

