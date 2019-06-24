#include"ZoneCtrl.h"
#include"Rs485.h"
#include"includes.h"
#include "delay.h"
#include "UserCore.h"

ZoneCommand ZoneCtrl;
IOCtrlPara ZonePara;
u8 ZoneSendBuf[10];
u8 IOState;
void	readthree(void);								//��COM3 485���ڶ�ȡ�Ƿ���������Ԫ������

/*16λCRCУ���*/
const u8 auchCRCHi[]={
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,

0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40
};
const u8 auchCRCLo[]={
0x00,0xc0,0xc1,0x01,0xc3,0x03,0x02,0xc2,0xc6,0x06,0x07,0xc7,0x05,0xc5,0xc4,0x04,
0xcc,0x0c,0x0d,0xcd,0x0f,0xcf,0xce,0x0e,0x0a,0xca,0xcb,0x0b,0xc9,0x09,0x08,0xc8,
0xd8,0x18,0x19,0xd9,0x1b,0xdb,0xda,0x1a,0x1e,0xde,0xdf,0x1f,0xdd,0x1d,0x1c,0xdc,
0x14,0xd4,0xd5,0x15,0xd7,0x17,0x16,0xd6,0xd2,0x12,0x13,0xd3,0x11,0xd1,0xd0,0x10,
0xf0,0x30,0x31,0xf1,0x33,0xf3,0xf2,0x32,0x36,0xf6,0xf7,0x37,0xf5,0x35,0x34,0xf4,
0x3c,0xfc,0xfd,0x3d,0xff,0x3f,0x3e,0xfe,0xfa,0x3a,0x3b,0xfb,0x39,0xf9,0xf8,0x38,
0x28,0xe8,0xe9,0x29,0xeb,0x2b,0x2a,0xea,0xee,0x2e,0x2f,0xef,0x2d,0xed,0xec,0x2c,
0xe4,0x24,0x25,0xe5,0x27,0xe7,0xe6,0x26,0x22,0xe2,0xe3,0x23,0xe1,0x21,0x20,0xe0,

0xa0,0x60,0x61,0xa1,0x63,0xa3,0xa2,0x62,0x66,0xa6,0xa7,0x67,0xa5,0x65,0x64,0xa4,
0x6c,0xac,0xad,0x6d,0xaf,0x6f,0x6e,0xae,0xaa,0x6a,0x6b,0xab,0x69,0xa9,0xa8,0x68,
0x78,0xb8,0xb9,0x79,0xbb,0x7b,0x7a,0xba,0xbe,0x7e,0x7f,0xbf,0x7d,0xbd,0xbc,0x7c,
0xb4,0x74,0x75,0xb5,0x77,0xb7,0xb6,0x76,0x72,0xb2,0xb3,0x73,0xb1,0x71,0x70,0xb0,
0x50,0x90,0x91,0x51,0x93,0x53,0x52,0x92,0x96,0x56,0x57,0x97,0x55,0x95,0x94,0x54,
0x9c,0x5c,0x5d,0x9d,0x5f,0x9f,0x9e,0x5e,0x5a,0x9a,0x9b,0x5b,0x99,0x59,0x58,0x98,
0x88,0x48,0x49,0x89,0x4b,0x8b,0x8a,0x4a,0x4e,0x8e,0x8f,0x4f,0x8d,0x4d,0x4c,0x8c,
0x44,0x84,0x85,0x45,0x87,0x47,0x46,0x86,0x82,0x42,0x43,0x83,0x41,0x81,0x80,0x40
};

u16 CRC16(u8* puchMsg, u16 usDataLen)
{
	u8 uchCRCHi=0xff;
	u8 uchCRCLo=0xff;
	u8 uIndex;
	
	while(usDataLen--)
	{
		uIndex=uchCRCHi^*(puchMsg++);
		uchCRCHi=uchCRCLo^auchCRCHi[uIndex];
		uchCRCLo=auchCRCLo[uIndex];
	}
	return uchCRCHi<<8|uchCRCLo;
}

//�������buf
void ZoneBufClear(void)
{
	u8 Cnt = 0;
	for(Cnt=0;Cnt<sizeof(ZoneSendBuf);Cnt++)
	{
		ZoneSendBuf[Cnt++] = 0;
	}
}
void SetIOstate(u8 Num)
{
		IOState = IOState|(1<<(Num-1));
}
//��ʼ��ZoneCtrl
void ZoneCtrl_Init(void)
{
	ZoneCtrl.Enable = 0;
	ZoneCtrl.CtrlNum = 0;
	ZoneCtrl.CtrlType = 0;
}
//������ŷ�������
u8 ZoneCtrl_Set(u8 Type,u8 Num)
{
	if(ZoneCtrl.Enable == 1)
	{
		return 0;
	}
	ZoneCtrl.Enable = 1;
	ZoneCtrl.CtrlNum = Num;
	ZoneCtrl.CtrlType = Type;
	return 1;
}
//��ʼ��ZonePara
void ZonePara_Init(void)
{
	ZonePara.IdleSta = 0;
	ZonePara.RWSta = 0;
	ZonePara.Error = 0;
	
}
//��ȡģ������
u8 XRead_Handl(u8*Buf)
{
	Buf[1] = 0x02;
	Buf[2] = 0x00;
	Buf[3] = 0x00;
	Buf[4] = 0x00;
	Buf[5] = 0x08;
	return 6;
}
//��ȡģ�����
u8 YRead_Handl(u8*Buf)
{
	Buf[1] = 0x01;
	Buf[2] = 0x00;
	Buf[3] = 0x10;
	Buf[4] = 0x00;
	Buf[5] = 0x04;
	return 6;
}
//����һ·�����ͬһģ�黥��
u8 YSet_Handl(u8*Buf, u8 on_off)
{
	Buf[1] = 0x05;
	Buf[2] = 0x00;
	//Buf[3] = 0x00;
	Buf[4] = on_off;
	Buf[5] = 0X00;//��Ҫ��Ӳ����Ӧ����
	return 6;
}
//���ģ�����
u8 YClear_Handl(u8*Buf)
{
	Buf[1] = 0x0F;
	Buf[2] = 0x00;
	Buf[3] = 0x10;
	Buf[4] = 0x00;
	Buf[5] = 0x04;
	Buf[6] = 0x01;
	Buf[7] = 0;
	return 8;
}
u8 YSetMul_Handl(u8*Buf, u8 ConduitF)
{
	Buf[0] = 0xfe;
	Buf[1] = 0x0F;
	Buf[2] = 0x00;
	Buf[3] = 0x10;
	Buf[4] = 0x00;
	Buf[5] = 0x04;
	Buf[6] = 0x01;
	Buf[7] = ConduitF;
	return 8;
}
void Frame_Send(u8 CmdType , u8 CmdNum ,u8*Buf)
{
	u8 Cnt = 0;
	u16 Crc= 0;
	OS_CPU_SR  cpu_sr;
	//ֻ����1��ioģ��
	Buf[0] = 0xfe;
	Buf[3] = CmdNum;
	//�����������������Ҫ����ָ��
	switch(CmdType)
	{
		case XREAD_ON://������
		case XREAD_OFF://������
			Cnt = XRead_Handl(Buf);
			break;
		case YSETSINGLE_ON://����ĳһ·OUT�����ͬһģ����������
			Cnt = YSet_Handl(Buf , 0XFF);
			break;
		case YSETSINGLE_OFF://����ĳһ·OUT�����ͬһģ����������
			Cnt = YSet_Handl(Buf , 0X00);
			break;
		default:
			break;
	}
	
	OS_ENTER_CRITICAL();
	Crc = CRC16(Buf , Cnt);
	OS_EXIT_CRITICAL();
	Buf[Cnt++] = (u8)((Crc>>8)&0xff);
	Buf[Cnt++] = (u8)(Crc&0xff);
	comSendBuf(COM3,Buf,Cnt);
	ZonePara.RWSta = READSTA;//�Ѿ�������ϣ�ת��Ϊ��ȡ״̬
	
}
ZoneCommand alfredTest[15];
//�������״̬
u8 Unfreame_YSta(u8* ReceiveBuf)
{
	MsgStruct ZoneMsg;
	IOState = ReceiveBuf[3];
	
	if(((1<<(ZoneCtrl.CtrlNum))&IOState)>0)
	{
		//���Ͳ�ѯ���
		if(ZoneCtrl.CtrlType == XREAD_ON)
		{
			ZoneMsg.CmdType = MSG_VERFYONOK;
		}
		else
		{
			ZoneMsg.CmdType = MSG_VERFYOFFERR;
		}
	}
	else
	{
		//���Ͳ�ѯ���
		if(ZoneCtrl.CtrlType == XREAD_OFF)
		{
			ZoneMsg.CmdType = MSG_VERFYOFFOK;
		}
		else
		{
			ZoneMsg.CmdType = MSG_VERFYONERR;
		}
	}
	ZoneMsg.CmdSrc = ZONE_TASK_CODE;
	ZoneMsg.CmdData[0] = ZoneCtrl.CtrlNum+1;
	PackSendStirQ(&ZoneMsg);
	return 1;
}
//�����յ��ı���
u8 Unfreame_Recive(u8* Buf)
{
	MsgStruct ZoneMsg;
	OS_CPU_SR  cpu_sr;
	u8 ReBuf[150] = {0};
	u16 Crc;
	u8 ReValue = 0;
	u8 BufLen = 0;
	memset(ReBuf,0,150);
	//�Ƿ��յ������ж�
	BufLen = COM3GetBuf(ReBuf);
	if(BufLen <= 5)
	{
		return 0;
	}
	//��ַ�ж�
	if(Buf[0] != ReBuf[0]&&ReBuf[0]!=0x6D)
	{
		return 0;
	}
	if(ReBuf[0]!=0x6D)
	{
		//CRC�ж�
		Crc = ReBuf[BufLen - 2]<<8|ReBuf[BufLen - 1];
		
		OS_ENTER_CRITICAL();
		if(Crc != CRC16(ReBuf, BufLen - 2))
		{
			OS_EXIT_CRITICAL();
			return 0;
		}
		OS_EXIT_CRITICAL();
		//��������
		switch(ReBuf[1])
		{
			case 0x01://���״̬����
				ReValue = Unfreame_YSta(ReBuf);
				break;
			case 0x05://���ƽ���
				ReValue =1;//��ַ������ֶԾ���������ִ�гɹ�
				if(ZoneCtrl.CtrlType == YSETSINGLE_ON)
				{
					if(ReBuf[3] == 0x05)
					{
						ZoneMsg.CmdType = MSG_SWITCHOPEN;
						ZoneMsg.CmdSrc = ZONE_TASK_CODE;
						ZoneMsg.CmdData[0] = 5;
						PackSendWaterAddQ(&ZoneMsg);
					}
					else if(ReBuf[3]<=0x04)
					{
						ZoneMsg.CmdType = MSG_SWITCHONOK;
						ZoneMsg.CmdSrc = ZONE_TASK_CODE;
						ZoneMsg.CmdData[0] = ZoneCtrl.CtrlNum+1;
						PackSendStirQ(&ZoneMsg);
					}
				}
				else if(ZoneCtrl.CtrlType == YSETSINGLE_OFF)
				{
					if(ReBuf[3] == 0x05)
					{
						ZoneMsg.CmdType = MSG_SWITCHOPEN;
						ZoneMsg.CmdSrc = ZONE_TASK_CODE;
						ZoneMsg.CmdData[0] = 5;
						PackSendWaterAddQ(&ZoneMsg);
					}
					else if(ReBuf[3]<=0x04)
					{
						ZoneMsg.CmdType = MSG_SWITCHOFFOK;
						ZoneMsg.CmdSrc = ZONE_TASK_CODE;
						ZoneMsg.CmdData[0] = ZoneCtrl.CtrlNum+1;
						PackSendStirQ(&ZoneMsg);
					}
				}
				break;
			default:
				break;
		}
		return ReValue;	
	}
	else
	{
		//readthree(ReBuf);
	}
	return 0;
}
//ͨѶ��ʱ����������ʧ����Ϣ
u8 AlfredOver = 0;
void OverTime(u8 MsgType,u8 Addr)
{
	OS_CPU_SR  cpu_sr;
	MsgStruct Msgtemp;
	
	AlfredOver = Addr;
	if(MsgType == XREAD_ON)
	{
		Msgtemp.CmdType = MSG_VERFYONOK;
	}
	else if(MsgType == XREAD_OFF)
	{
		Msgtemp.CmdType = MSG_VERFYOFFOK;
	}
	else if(MsgType == YSETSINGLE_ON)
	{
		//Msgtemp.CmdType = MSG_SWITCHONOK;
	}
	else if(MsgType == YSETSINGLE_OFF)
	{
		//Msgtemp.CmdType = MSG_SWITCHOFFOK;
	}
	else
	{return;}
	Msgtemp.CmdSrc = ZONE_TASK_CODE;
	Msgtemp.CmdData[0] = Addr+1;
	OS_ENTER_CRITICAL();
	PackSendStirQ(&Msgtemp);
	OS_EXIT_CRITICAL();
}
//������������
// alfred
// ��Ӿֲ�����������Ϣ�ж����ݶ�ȡ�����
OS_EVENT * ZoneQMsg;
void* 	ZoneMsgBlock[4];
OS_MEM* ZonePartitionPt;
u8 g_u8ZoneMsgMem[20][4];

//������Ϣ���к��ڴ��
//���0����������
//    1����Ϣ���д���ʧ��
//	  2���ڴ�鴴��ʧ��
u8 ZoneQInit(void)
{
	INT8U os_err;
	
	ZoneQMsg = OSQCreate ( ZoneMsgBlock, 4);
	
	if(ZoneQMsg == (OS_EVENT *)0)
	{
		return 1;
	}
	
	ZonePartitionPt = OSMemCreate (
										g_u8ZoneMsgMem,
				  						20,
				  						4,
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

u8 PackSendZoneQ(MsgStruct* MsgBlk)
{
	INT8U os_err;
	MsgStruct * MsgTemp = NULL;
	MsgTemp = (MsgStruct *)OSMemGet(ZonePartitionPt,&os_err);
	if(MsgTemp == NULL)
	{
		return 1;
	}
	MsgTemp ->CmdType = MsgBlk->CmdType;
	MsgTemp ->CmdSrc = MsgBlk->CmdSrc;
	MsgTemp ->CmdData[0] = MsgBlk->CmdData[0];
	MsgTemp ->CmdData[1] = MsgBlk->CmdData[1];
	os_err = OSQPost ( ZoneQMsg,(void*)MsgTemp );
	//������Ϣʧ���ͷ��ڴ�
	if(os_err!=OS_ERR_NONE)
	{
		OSMemPut(ZonePartitionPt, ( void * )MsgTemp);
		return 2;
	}
	return 0;
}
//������Ϣ��������Ϣ�ڴ��ͷ�
//���룺�յ�����Ϣ����ָ��
//�����0���ͷųɹ�
//		1���ͷ�ʧ��
u8 DepackReceiveZoneQ(MsgStruct * MasterQ)
{
	u8 os_err;
	os_err = OSMemPut(ZonePartitionPt, ( void * )MasterQ);
	if(os_err != OS_ERR_NONE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void ZoneCtrl_task(void *pdata)
{
	MsgStruct * pMsgBlk = NULL;
	MsgStruct Msgtemp;
//	MsgStruct ZoneMsg;
	u8 os_err;
	
	ZoneQInit();
	ZoneCtrl_Init();
	ZonePara_Init();
	ZoneBufClear();
	while(1)
	{
		delay_ms(100);//��Ҫ���Զ�ú��� alfred to be continue
		//��������
		if(ZonePara.IdleSta == 0)
		{
			//��ϵͳ����ʱ,��ȡ����Ϣ.
			
			pMsgBlk = ( MsgStruct *) OSQPend ( ZoneQMsg,
					40,
					&os_err );
			
			if(os_err == OS_ERR_NONE)
			{
				memcpy((u8*)&Msgtemp,(u8*)pMsgBlk,sizeof(MsgStruct) );
				DepackReceiveZoneQ(pMsgBlk);
				
				ZonePara.RWSta = WRITESTA;
				ZonePara.IdleSta = 1;
				
				ZoneCtrl.CtrlNum = Msgtemp.CmdData[0];
				ZoneCtrl.CtrlType = Msgtemp.CmdType;
			}
			else
			{
				readthree();
			}
		}
		else if(ZonePara.IdleSta == 1)//����æ
		{
			switch(ZonePara.RWSta)
			{
				case READSTA:
					if(Unfreame_Recive(ZoneSendBuf))//�����յ��ı���
					{
						//CmdComletion(MSG_ZONEALARM,ZoneSendBuf[0],0);
						ZoneBufClear();
						ZoneCtrl_Init();
						ZonePara_Init();
					}
					else 
					{
						if(ZonePara.Error++ > 3)
						{
							if(ZonePara.Error > 12)//�ط�һ���ղ���������Ϊģ�����
							{
								OverTime(ZoneCtrl.CtrlType,ZoneCtrl.CtrlNum);
								ZoneBufClear();
								ZoneCtrl_Init();
								ZonePara_Init();
							}
							else if(ZonePara.Error == 8)
							{
								ZonePara.RWSta = 0x02;//8���ղ��������ط�һ��
							}
						}
					}
					break;
				case WRITESTA:
					Frame_Send(ZoneCtrl.CtrlType,ZoneCtrl.CtrlNum, ZoneSendBuf);//��֡����
					break;
				default://�յ��������ݳ�ʼ������
					ZoneBufClear();
					ZoneCtrl_Init();
					ZonePara_Init();
					break;
			}
		}
		
	}
}
