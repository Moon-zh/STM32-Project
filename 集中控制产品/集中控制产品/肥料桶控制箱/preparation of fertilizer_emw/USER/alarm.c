#include "sys.h"
#include "includes.h"  
#include "Delay.h"
#include "Alarm.h"
#include "UserCore.h"
#include "FlashDivide.h"
#include "Emw3060.h"
#include "Hmi_driver.h"


OS_EVENT * AlarmQMsg;
void* 	AlarmMsgBlock[4];
OS_MEM* AlarmPartitionPt;
u8 g_u8AlarmMsgMem[5][4];

//������Ϣ���к��ڴ��
//���0����������
//    1����Ϣ���д���ʧ��
//	  2���ڴ�鴴��ʧ��
u8 AlarmQInit(void)
{
	INT8U os_err;
	
	AlarmQMsg = OSQCreate ( AlarmMsgBlock, 4);
	
	if(AlarmQMsg == (OS_EVENT *)0)
	{
		return 1;
	}
	
	AlarmPartitionPt = OSMemCreate (
										g_u8AlarmMsgMem,
				  						5,
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

u8 PackSendAlarmQ(MsgStruct* MsgBlk)
{
	INT8U os_err;
	MsgStruct * MsgTemp = NULL;
	MsgTemp = (MsgStruct *)OSMemGet(AlarmPartitionPt,&os_err);
	if(MsgTemp == NULL)
	{
		return 1;
	}
	MsgTemp ->CmdType = MsgBlk->CmdType;
	MsgTemp ->CmdSrc = MsgBlk->CmdSrc;
	MsgTemp ->CmdData[0] = MsgBlk->CmdData[0];
	MsgTemp ->CmdData[1] = MsgBlk->CmdData[1];
	os_err = OSQPost ( AlarmQMsg,(void*)MsgTemp );
	//������Ϣʧ���ͷ��ڴ�
	if(os_err!=OS_ERR_NONE)
	{
		OSMemPut(AlarmPartitionPt, ( void * )MsgTemp);
		return 2;
	}
	return 0;
}
//������Ϣ��������Ϣ�ڴ��ͷ�
//���룺�յ�����Ϣ����ָ��
//�����0���ͷųɹ�
//		1���ͷ�ʧ��
u8 DepackReceiveAlarmQ(MsgStruct * MasterQ)
{
	u8 os_err;
	os_err = OSMemPut(AlarmPartitionPt, ( void * )MasterQ);
	if(os_err != OS_ERR_NONE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
LevelParam LevelAlarmPara[5];
u8 	CollPerioF[5] = {0,0,0,0,0};//�ɼ���־
u16 LevelData[5] = {0,0,0,0,0};//��ȡҺλ���� ��λ mm
u8 	StartLevelColl[5] = {0,0,0,0,0};//��ʼ�ɼ���ʱ��־
u8	_20SecCnt[5] = {0,0,0,0,0};//20�����  RTC ʹ��
u16  SetLevelValue[5] = {0,0,0,0,0};//����ֵ
u8 LevelErrCnt[5] = {0,0,0,0,0};//���ٴ��Ժ��㻵��//��ֹҺλ��������

u8 	StirTimeF[5] = {0,0,0,0,0};//��ʱ�����μ�ʱ��־
u8	StirTimeFlag[5] = {0,0,0,0,0};//��ʼ�����־
u8	StirTimeMin[5] = {0,0,0,0,0};//���������
u8  SetStirTime[5] = {0,0,0,0,0};//����ֵ
u8 	StartStirTime[5] = {0,0,0,0,0};//��ʼ���������־ RTC ʹ��
u8  _60SecCnt[5] = {0,0,0,0,0};//60�����

u8 AlarmLow[5]={0,0,0,0,0};
u8 AlarmLowF[5]={0,0,0,0,0};

u8 SysStirTime[5]={0,0,0,0,0};
//���Һλ�жϵĲ���
u8 ClearLevelPara(u8 Num)
{
	if(LevelAlarmPara[Num].LevelFlag == 0)
	{
		LevelAlarmPara[Num].LastLevel = 0;
		CollPerioF[Num] = 0;
		StartLevelColl[Num] = 0;
		_20SecCnt[Num] = 0;
		return 1;
	}
	return 0;
}
//�������ʱ���жϵĲ���
u8 ClearStirTimePara(u8 Num)
{
	if(StirTimeFlag[Num] == 0)
	{
		StirTimeF[Num] = 0;
		StirTimeMin[Num] = 0;
		StartStirTime[Num]= 0;
		_60SecCnt[Num] = 0;
		return 1;
	}
	return 0;
}

void Alarm_task(void *pdata)
{
	u8 os_err;
	u8 i=0;
	u8 SetLevelBuf[5] = {0,0,0,0,0};
	MsgStruct * pMsgBlk = NULL;
	MsgStruct Msgtemp;
	MsgStruct MsgtempBlk;
	MsgStruct WaterAddMsg;
	AlarmQInit();
  	ParaApplication();
	while(1)
	{
		delay_ms(100);
		pMsgBlk = ( MsgStruct *) OSQPend ( AlarmQMsg,
				10,
				&os_err );
		
		if(os_err == OS_ERR_NONE)
		{
			memcpy((u8*)&Msgtemp,(u8*)pMsgBlk,sizeof(MsgStruct) );
			DepackReceiveAlarmQ(pMsgBlk);
			switch(Msgtemp.CmdType)
			{
				case ALARMMSG_LEVEL_OPEN://�յ�����ʱ�ȼ�¼
					//��ղ���
					if(ClearLevelPara(Msgtemp.CmdData[0]))
					{
						//FLASH��ȡ����ֵ
						FlashReadWaterADD(SetLevelBuf);
						SetLevelValue[0] = SetLevelBuf[0]*10;
						SetLevelValue[1] = SetLevelBuf[1]*10;
						SetLevelValue[2] = SetLevelBuf[2]*10;
						SetLevelValue[3] = SetLevelBuf[3]*10;
						SetLevelValue[4] = SetLevelBuf[4]*10;
						//�����ж�
						LevelAlarmPara[Msgtemp.CmdData[0]].LevelFlag = 1;
						//��ʼ��ʱ
						StartLevelColl[Msgtemp.CmdData[0]] = 1;
						//��¼ԭʼ����
						LevelAlarmPara[Msgtemp.CmdData[0]].LastLevel = LevelData[Msgtemp.CmdData[0]];
					}
					break;
				case ALARMMSG_LEVEL_CLOSE://�յ�����ʱ���
					LevelAlarmPara[Msgtemp.CmdData[0]].LevelFlag = 0;
					StartLevelColl[Msgtemp.CmdData[0]] = 0;
					break;
				case ALARMMSG_STIRTIME_OPEN://�յ�����ʱ�򿪼�ʱ��
					if(ClearStirTimePara(Msgtemp.CmdData[0]))
					{
						//FLASH��ȡ����ֵ
						FlashReadStir(SetStirTime);
						//�����ж�
						StirTimeFlag[Msgtemp.CmdData[0]] = 1;
						//������ʱ
						StartStirTime[Msgtemp.CmdData[0]] = 1;
						StirTimeMin[Msgtemp.CmdData[0]]=SetStirTime[Msgtemp.CmdData[0]];
						//memcpy(StirTimeMin,SetStirTime,5);
					}
					break;
				case ALARMMSG_STIRTIME_CLOSE://�յ�����ʱ���
					StirTimeFlag[Msgtemp.CmdData[0]] = 0;
					StartStirTime[Msgtemp.CmdData[0]] = 0;
					StirTimeMin[Msgtemp.CmdData[0]] = 0;
					ClearStirTimePara(Msgtemp.CmdData[0]);
					break;
				default:
					break;
			}
		}
		else
		{
			//Һλ�ͱ��� //
			for(i=0;i<5;i++)
			{
				if((AlarmLow[i]*10>= LevelData[i])&&AlarmLowF[i] == 0)
				{
					//����
					SetScreen(7);
					AlarmLowF[i] = 1;
				}
				else if(AlarmLow[i]*10< LevelData[i])
				{
					AlarmLowF[i] = 0;
				}
			}
		}
		//Һλ�ж�
		for(i=0;i<5;i++)
		{
			if((LevelAlarmPara[i].LevelFlag == 1)&&(CollPerioF[i] == 1))//����������ҵ����ж�����
			{
				if(SetLevelValue[i] <= LevelData[i])
				{
					//����Һλ�ﵽ��Ϣ
					WaterAddMsg.CmdType = MSG_WATERFULL;
					WaterAddMsg.CmdSrc = ALARM_TASK_CODE;
					WaterAddMsg.CmdData[0] = i+1;
					PackSendWaterAddQ(&WaterAddMsg);
					LevelAlarmPara[i].LevelFlag = 0;
					StartLevelColl[i] = 0;
					LevelErrCnt[i] = 0;
				}
				else if(LevelAlarmPara[i].LastLevel >= LevelData[i])
				{
					//����Һλ����������Ϣ.
					if(LevelErrCnt[i]++>30)
					{
						WaterAddMsg.CmdType = MSG_LEVEERR;
						WaterAddMsg.CmdSrc = ALARM_TASK_CODE;
						WaterAddMsg.CmdData[0] = i+1;
						PackSendWaterAddQ(&WaterAddMsg);
						LevelAlarmPara[i].LevelFlag = 0;
						StartLevelColl[i] = 0;
						LevelErrCnt[i] = 0;
					}
				}
				else
				{
					LevelErrCnt[i] = 0;
				}
				CollPerioF[i] = 0;
				//������ʷ����
				LevelAlarmPara[i].LastLevel = LevelData[i];
			}
		}
		//�����ж�
		for(i=0;i<5;i++)
		{
			if((StirTimeFlag[i] == 1)&&(StirTimeF[i] == 1))//����������ҵ����ж�����
			{
				if(--StirTimeMin[i] == 0)
				{
					WaterAddMsg.CmdType = MSG_TIMEOVER;
					WaterAddMsg.CmdSrc = ALARM_TASK_CODE;
					WaterAddMsg.CmdData[0] = i+1;
					PackSendStirQ(&WaterAddMsg);
					StirTimeFlag[i] = 0;
					StartStirTime[i] = 0;
					_60SecCnt[i] = 0;
				}
				SetRemoteRead(32+i,StirTimeMin[i]);
				MsgtempBlk.CmdSrc = ALARM_TASK_CODE;
				MsgtempBlk.CmdType= UPLOAD_COUNTDOWN;
				PackSendRemoteQ(&MsgtempBlk);
				StirTimeF[i] = 0;
			}
		}
	}
}
//��ˮ�� 246.1kg
u8 BucketPercent[5];//Ũ�ȱ��� //������ʼ�� �����ʼ��
u16 BucketLimit[5];//Һλ���� //������ʼ�� �����ʼ�� ��λ mm
u32 BucketS[5];//����� //������ʼ�� �����ʼ��
u16 DestFeitilizer[5];//�������� ��λKG
u16 BucketFeitilizer[5];//���з����� KG
u16 NeedFertilizer[5];//��Ҫ��ӵķ���
u16 Limit = 500;
u8 Persent = 50;
 void CaluculateFertilizer(void)
{
	u8 i = 0;
	
	
	/*BucketLimit[0] = Limit;
	BucketLimit[1] = Limit;
	BucketLimit[2] = Limit;
	BucketLimit[3] = Limit;
	BucketLimit[4] = Limit;
	BucketPercent[0] = Persent;
	BucketPercent[1] = Persent;
	BucketPercent[2] = Persent;
	BucketPercent[3] = Persent;
	BucketPercent[4] = Persent;*/
	
	//����ó��ܷ��� 1M����
	for(i=0;i<5;i++)
	{
		DestFeitilizer[i] = 2461*BucketPercent[i]/(100-BucketPercent[i]);
	}
	//����ó�������ӷ���
	for(i=0;i<5;i++)
	{
		NeedFertilizer[i] = (BucketLimit[i]-LevelData[i])*DestFeitilizer[i]/1000;
	}
}
void ParaApplication(void)
{
	u8 AlarmDestLevel[5] = {0,0,0,0,0};
	u8 AlarmStirTime[5] = {0,0,0,0,0};
	u8 AlarmPersent[5] = {0,0,0,0,0};
	u8 AlarmLowLevel[5] = {0,0,0,0,0};
	u16 AlarmSize[5] = {0,0,0,0,0};
	u8 i=0;
	
	FlashReadWaterADD(AlarmDestLevel);
	FlashReadStir(AlarmStirTime);
	FlashReadPersent(AlarmPersent);
	FlashReadLow(AlarmLowLevel);
	FlashReadD((u8*)AlarmSize);
	for(i=0;i<5;i++)
	{
		BucketS[i] = 314*((AlarmSize[i]*10)/2)*((AlarmSize[i]*10)/2)/10000;
		BucketLimit[i] = AlarmDestLevel[i]*10;
		BucketPercent[i] = AlarmPersent[i];
		AlarmLow[i] = AlarmLowLevel[i];
		SysStirTime[i] = AlarmStirTime[i];
	}
}
void FertilizerCalculate(void)
{
	u8 i =0;
	for(i=0;i<5;i++)
	{
		//���(cm3)=�����(cm2)*��(cm)
		//���(ml)=���(m3)
		//ˮ��(kg) = ���(ml)*�ܶȣ�1��
		//��(kg)=ˮ*Ũ��/(1-Ũ��) ��ʽ�ұ�����ͬʱ����100��
		//��������ȵ����*�߶Ȳ�*(p/1-p)/10(�߶Ȳ���mm����ģ���Ҫ��CM����)/1000(g�����kg)
		if((BucketLimit[i]>LevelData[i]))
		{
			BucketFeitilizer[i] = ((BucketS[i]*(BucketLimit[i]-LevelData[i])*BucketPercent[i])
									/(100-BucketPercent[i]))/10000;
		}
		else
		{
			BucketFeitilizer[i] = 0;
		}
	}
	//�ܷ���-���з��� = ��Ҫ��ӵķ���
}
//�ܶȼ���
u8 BeforeDensity[5];//�ӷ�ǰ����ó����ܶ�.�ô��ܶȼ���Һλ�߶�
u8 AfterDensity[5];//�ӷʺ����ó����ܶ�
void CalculateDensity(void)
{
	u8 i =0;
	for(i=0;i<5;i++)
	{
		/*BeforeDensity[i] = (BucketFeitilizer[i]/20
							+BucketS[i]*BucketLimit[i]/100000000)
							/(BucketS[i]*BucketLimit[i]/100000000000);*/
		//������ļ���仯����
		BeforeDensity[i]= (BucketFeitilizer[i]*5000000000
							+BucketS[i]*BucketLimit[i]*1000)
							/(BucketS[i]*BucketLimit[i]);
	}
	for(i=0;i<5;i++)
	{
		/*AfterDensity[i] = (BucketS[i]*BucketLimit[i]/100000000)/(100-BucketPercent[i])
							/(BucketS[i]*BucketLimit[i]/100000000000);*/
		AfterDensity[i] = (BucketS[i]*BucketLimit[i]*1000)/(100-BucketPercent[i])
							/(BucketS[i]*BucketLimit[i]);
	}
}

