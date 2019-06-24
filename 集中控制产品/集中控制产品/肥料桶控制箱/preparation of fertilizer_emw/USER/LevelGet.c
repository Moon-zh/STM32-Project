#include "sys.h"
#include "includes.h"  
#include "Delay.h"
#include "LevelGet.h"
#include "Rs485.h"
#include "ZoneCtrl.h"
#include "Alarm.h"
#include "UserHmi.h"
LevelCtrlPara LevelPara;
LevelCommand  LevelCtrl;
u8 LevelSendBuf[10];
u8 DevID = 0;


//������ʼ��
void LevelParaInit(void)
{
	LevelPara.Error = 0;
	LevelPara.IdleSta = 0;
	LevelPara.RWSta = 0;
}
void LevelCtrl_Init(void)
{
	LevelCtrl.Enable = 0;
	LevelCtrl.CtrlType = 0;
}
u8 LevelCtrl_Set(u8 Type)
{
	if(LevelCtrl.Enable == 1)
	{
		return 0;
	}
	LevelCtrl.Enable = 1;
	//LevelCtrl.CtrlNum = Num;
	LevelCtrl.CtrlType = Type;
	return 1;
}
//�������buf
void LevelBufClear(void)
{
	u8 Cnt = 0;
	for(Cnt=0;Cnt<sizeof(LevelSendBuf);Cnt++)
	{
		LevelSendBuf[Cnt++] = 0;
	}
}
//Һλ���ݻ�ȡ
u8  LevelCmdSend(u8* Buf,u8 DevNum)
{
	u8 Cnt = 0;
	Buf[Cnt++] = DevNum;
	Buf[Cnt++] = 0x03;
	Buf[Cnt++] = 0x00;
	Buf[Cnt++] = 0x00;
	Buf[Cnt++] = 0x00;
	Buf[Cnt++] = 0x01;
	return Cnt;
}
//������ȡ
u8 PowerCmdSend(u8* Buf,u8 DevNum)
{
	u8 Cnt = 0;
	Buf[Cnt++] = DevNum;
	Buf[Cnt++] = 0x03;
	Buf[Cnt++] = 0x00;
	Buf[Cnt++] = 0x00;
	Buf[Cnt++] = 0x00;
	Buf[Cnt++] = 0x01;
	return Cnt;
}
//���ķ���
void LevelFrame_Send(u8 CmdType ,u8*Buf)
{
	u8 Cnt = 0;
	u16 Crc= 0;
	OS_CPU_SR  cpu_sr;
	
	//�жϸÿ����ĸ�ioģ��//�����ڷ�����ŷ�
	//�����������������Ҫ����ָ��
	switch(CmdType)
	{
		case DEV1READ:
			Cnt = LevelCmdSend(Buf, 1);
			DevID = 1;
			break;
		case DEV2READ:
			Cnt = LevelCmdSend(Buf, 2);
			DevID = 2;
			break;
		case DEV3READ:
			Cnt = LevelCmdSend(Buf, 3);
			DevID = 3;
			break;
		case DEV4READ:
			Cnt = LevelCmdSend(Buf, 4);
			DevID = 4;
			break;
		case DEV5READ:
			Cnt = LevelCmdSend(Buf, 5);
			DevID = 5;
			break;
		case DEV6READ:
			Cnt = PowerCmdSend(Buf, 6);
			DevID = 6;
			break;
		default:
			break;
	}
	
	OS_ENTER_CRITICAL();
	Crc = CRC16(Buf , Cnt);
	OS_EXIT_CRITICAL();
	Buf[Cnt++] = (u8)((Crc>>8)&0xff);
	Buf[Cnt++] = (u8)(Crc&0xff);
	comSendBuf(COM4,Buf,Cnt);
	LevelPara.RWSta = READSTA;//�Ѿ�������ϣ�ת��Ϊ��ȡ״̬
	
}
u8 BucketDensity[5] ;
//����Һλ
u8 UnframeLevelData(u8 * ReBuf)
{
	if(ReBuf[1] == 0x03)
	{
		//Һλ�߶�(ˮ) = ����*����/����
		//Һλ�߶�(��Һ) = Һλ�߶�(ˮ)/��Һ�ܶ�  ��ʽ�Ҳ���������1000��BucketDensity(�Ѿ�������)
		//LevelData[ReBuf[0]-1] = ((ReBuf[3]<<8)|ReBuf[4])*3000*1000/BucketDensity[ReBuf[0]-1]/2000;
		//LevelData[ReBuf[0]-1] = ((ReBuf[3]<<8)|ReBuf[4])*3000*1000/BucketDensity[ReBuf[0]-1]/2000;
		LevelData[ReBuf[0]-1] = ((ReBuf[3]<<8)|ReBuf[4])*3000/2000;
		
		return 1;
	}
	return 0;
}

//�����յ��ı���
u8 LevelUnfreame_Recive(u8* Buf)
{
	OS_CPU_SR  cpu_sr;
	u8 ReBuf[15] = {0};
	u16 Crc;
	u8 ReValue = 0;
	u8 BufLen = 0;
	//�Ƿ��յ������ж�
	BufLen = COM4GetBuf(ReBuf);
	if(BufLen <= 5)
	{
		return 0;
	}
	//��ַ�ж�
	if(ReBuf[0] != DevID)
	{
		return 0;
	}
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
	switch(ReBuf[0])
	{
		case 0x01://���״̬����
		case 0x02://����״̬����
		case 0x03:
		case 0x04:
		case 0x05:
			ReValue = UnframeLevelData(ReBuf);
			break;
		case 0x06:
			ReValue = UnframeLevelData(ReBuf);
			break;
		case 0x0f://���ƽ���
			ReValue =1;//��ַ������ֶԾ���������ִ�гɹ�
			break;
		default:
			break;
	}
	return ReValue;	
}
u8 LevelErr[5];
//������һ��Ӧ��������豸��,�ж��ҵ���һ��û��������豸�ŷ���
u8 FindDevId(u8 NextNum)
{
	u8 i=0;
	for(i=NextNum;i<=5;i++)
	{
		if(LevelErr[i] == 0)
		{
			return i;
		}
	}
	if(i==5)
	{
		for(i=1;i<NextNum;i++)
		{
			if(LevelErr[i] == 0)
			{
				return i;
			}
		}
		if(i==NextNum)
		{
			return NextNum;
		}
	}
	return 1;
}
u8 FirstSendF=0;

//Һλ���ݻ�ȡ����
void LevelGet_task(void *pdata)
{
	while(1)
	{
		delay_ms(200);
		if(LevelPara.IdleSta == 0)
		{
			if(LevelCtrl.Enable == 1)
			{
				LevelPara.RWSta = WRITESTA;
				LevelPara.IdleSta = 1;
			}
			else 
			{
				{
					if(DevID < 5)
					{
						LevelCtrl_Set(DevID+1);
					}
					else
					{
						LevelCtrl_Set(1);
					}
						
				}
			}
		}		
		else if(LevelPara.IdleSta == 1)//����æ
		{
			switch(LevelPara.RWSta)
			{
				case READSTA:
					if(LevelUnfreame_Recive(LevelSendBuf))//�����յ��ı���
					{
						//CmdComletion(MSG_ZONEALARM,ZoneSendBuf[0],0);
						LevelBufClear();
						LevelCtrl_Init();
						LevelParaInit();
						//����ȫ���յ������ϴ�һ�Ρ�
						if((LevelData[0]!=0)&&(LevelData[1]!=0)
							&&(LevelData[2]!=0)&&(LevelData[3]!=0)&&(FirstSendF==0))
						{
							CycleUpLevel();
							FirstSendF = 1;
						}
					}
					else 
					{
						if(LevelPara.Error++ >=  2)
						{
							if(LevelPara.Error > 4)//�ط�һ���ղ���������Ϊģ�����
							{
								LevelErr[DevID -1] = 1;
								//CmdComletion(MSG_ZONEALARM,ZoneSendBuf[0],1);
								LevelBufClear();
								LevelCtrl_Init();
								LevelParaInit();
							}
							else if(LevelPara.Error == 5)
							{
								//LevelPara.RWSta = 0x02;//5���ղ��������ط�һ��
							}
						}
					}
					break;
				case WRITESTA:
					LevelFrame_Send(LevelCtrl.CtrlType, LevelSendBuf);//��֡����
					break;
				default://�յ��������ݳ�ʼ������
					LevelBufClear();
					LevelCtrl_Init();
					LevelParaInit();
					break;
			}
		}
	}
}

