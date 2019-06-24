#include "sys.h"
#include "includes.h"  
#include "UserCore.h"
#include "Delay.h"
#include "ZoneCtrl.h"
#include "GM3Dir.h"
#include "hmi_driver.h"
#include "UserHmi.h"
#include "IO_BSP.h"
#include "FlashDivide.h"
MsgStruct MasterMsg;
OS_EVENT * MasterQMsg;

void* 	MasterMsgBlock[4];
OS_MEM* MasterPartitionPt;
u8 g_u8AgvCtrMsgMem[5][4];

u16 WarterRemainderTime = 0;//��ˮʣ�๤��ʱ��
u16 FertilizerRemainderTime = 0;//ʩ��ʣ�๤��ʱ��

u8 ControlMode = 0;//ˮ��һ�������ģʽ��1������ 0:����
//u8 MasterState = 0;//��״̬��
u8 WarterState = 0;//��ˮ״̬
u8 FertilizerState = 0;//��ˮ״̬

StrategyStruct WorkStrategy;
//дִ�в���
//���룺Ҫ���õ�ֵ
//���أ�0�����óɹ� 1�����ù��ܱ�ռ�� 2��FLASHдʧ��
//ʹ��ʱ��ֹ�ж�
u8 SetStrategy(StrategyStruct *Strategy)
{
	static u8 Flag = 0;
	if(Flag == 1)
		return 1 ;
	Flag = 1;
	WorkStrategy.PumpWFlag = Strategy->PumpWFlag;
	WorkStrategy.PumpFFlag = Strategy->PumpFFlag;
	WorkStrategy.Passageway[0] = Strategy->Passageway[0];
	WorkStrategy.Passageway[1] = Strategy->Passageway[1];
	WorkStrategy.Passageway[2] = Strategy->Passageway[2];
	WorkStrategy.Passageway[3] = Strategy->Passageway[3];
	WorkStrategy.Passageway[4] = Strategy->Passageway[4];
	WorkStrategy.Passageway[5] = Strategy->Passageway[5];
	WorkStrategy.Zone = Strategy->Zone;
	WorkStrategy.WorkHour = Strategy->WorkHour;
	WorkStrategy.WorkMinute = Strategy->WorkMinute;
	WorkStrategy.WorkDay = Strategy->WorkDay;
	WorkStrategy.StartHour = Strategy->StartHour;
	WorkStrategy.StartMinute = Strategy->StartMinute;
	//д��Flash
	Flag = 0;
	return 0;
}
//���Կ�ʼ
//���룺���Բ���
//�������
// alfred ��ʣMODBUS��Ȧû��д ˮ�úͷʱõĿ���û��д
void StrategyStart(StrategyStruct *Strategy)
{
	u8 ConduitF = 0;
	u8 i = 0;
	
	//�򿪷�����ŷ�
	ConduitF = Strategy->Zone;
//	ZoneCtrl_Set(ConduitF,YSETSINGLE);
	SetMDCoil(ConduitF + 2 - 1 , 1);
	delay_ms(110);
	
	if(Strategy->PumpWFlag == 1)
	{
		//��ˮ��
		SetMDCoil(0 , 1);
	}
	else if(Strategy->PumpWFlag == 0)
	{
		//������ߴ�ˮ��
	}
	if(Strategy->PumpFFlag == 1)
	{
		//�򿪷ʱ�
		SetMDCoil(1 , 1);
	}
	else if(Strategy->PumpFFlag == 0)
	{
		//�رշʱ�
		SetMDCoil(1 , 0);
	}
	//��ͨ����ŷ� 
	for(i = 0;i<6; i++)
	{
		if(Strategy->Passageway[i] == 1)
		{
			ConduitF |= i<<i;
			SetMDCoil(8+i , 1);
		}
	}
//	ZoneCtrl_Set(ConduitF,YSETMUL);
}
//����ֹͣ
//���룺���Բ���
//�������
// alfred ��ʣMODBUS��Ȧû��д ˮ�úͷʱõĿ���û��д
void StrategyStop(StrategyStruct *Strategy)
{
	u8 ConduitF = 0;
	u8 i = 0;
	
	//�ر�ˮ��
	SetMDCoil(0 , 0);
	//�رշʱ�
	SetMDCoil(1 , 0);
	//�ر�ͨ����ŷ� 
	for(i = 0;i<6; i++)
	{
		SetMDCoil(8+i , 0);
	}
//	ZoneCtrl_Set(0,YCLEAR);
	delay_ms(110);
	//�رշ�����ŷ�
	ConduitF = Strategy->Zone;
//	ZoneCtrl_Set(ConduitF,YCLEAR);
	SetMDCoil(ConduitF + 2 - 1  , 0);
}
//������Ϣ���к��ڴ��
//���0����������
//    1����Ϣ���д���ʧ��
//	  2���ڴ�鴴��ʧ��
u8 MasterQInit(void)
{
	INT8U os_err;
	
	MasterQMsg = OSQCreate ( MasterMsgBlock, 4);
	
	if(MasterQMsg == (OS_EVENT *)0)
	{
		return 1;
	}
	
	MasterPartitionPt = OSMemCreate (
										g_u8AgvCtrMsgMem,
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
//�˺�����Ҫ���ж�ʹ��
u8 InterruptPackSendMasterQ(MsgStruct* MsgBlk)
{
	INT8U os_err;
	MsgStruct * MsgTemp = NULL;
	MsgTemp = (MsgStruct *)OSMemGet(MasterPartitionPt,&os_err);
	if(MsgTemp == NULL)
	{
		return 1;
	}
	MsgTemp ->CmdType = MsgBlk->CmdType;
	MsgTemp ->CmdSrc = MsgBlk->CmdSrc;
	MsgTemp ->CmdData[0] = MsgBlk->CmdData[0];
	MsgTemp ->CmdData[1] = MsgBlk->CmdData[1];
	os_err = OSQPost ( MasterQMsg,(void*)MsgTemp );
	//������Ϣʧ���ͷ��ڴ�
	if(os_err!=OS_ERR_NONE)
	{
		OSMemPut(MasterPartitionPt, ( void * )MsgTemp);
		return 2;
	}
	return 0;
}
//��Ϣ�������
//���룺��Ϣ��Ϣ
//�����0�����ͳɹ�
//		1�������ڴ�ʧ��
//		2��������Ϣʧ��

u8 PackSendMasterQ(MsgStruct* MsgBlk)
{
	INT8U os_err;
	MsgStruct * MsgTemp = NULL;
	MsgTemp = (MsgStruct *)OSMemGet(MasterPartitionPt,&os_err);
	if(MsgTemp == NULL)
	{
		return 1;
	}
	MsgTemp ->CmdType = MsgBlk->CmdType;
	MsgTemp ->CmdSrc = MsgBlk->CmdSrc;
	MsgTemp ->CmdData[0] = MsgBlk->CmdData[0];
	MsgTemp ->CmdData[1] = MsgBlk->CmdData[1];
	os_err = OSQPost ( MasterQMsg,(void*)MsgTemp );
	//������Ϣʧ���ͷ��ڴ�
	if(os_err!=OS_ERR_NONE)
	{
		OSMemPut(MasterPartitionPt, ( void * )MsgTemp);
		return 2;
	}
	return 0;
}
//������Ϣ��������Ϣ�ڴ��ͷ�
//���룺�յ�����Ϣ����ָ��
//�����0���ͷųɹ�
//		1���ͷ�ʧ��
u8 DepackReceiveMasterQ(MsgStruct * MasterQ)
{
	u8 os_err;
	os_err = OSMemPut(MasterPartitionPt, ( void * )MasterQ);
	if(os_err != OS_ERR_NONE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
u8 ZoneAlarm[2];
void SetZoneAlarm(u8 AlarmNum, u8 Flag)
{
	if(Flag == 1)
	{
		if(AlarmNum == 1)
		{
			ZoneAlarm[0] |= 0x0f; 
		}
		else if(AlarmNum == 2)
		{
			ZoneAlarm[0] |= 0xf0; 
		}
		else if(AlarmNum == 3)
		{
			ZoneAlarm[1] |= 0x0f; 
		}
		else if(AlarmNum == 4)
		{
			ZoneAlarm[1] |= 0xf0; 
		}
	}
	else if(Flag == 0)
	{
		if(AlarmNum == 1)
		{
			ZoneAlarm[0] &= 0xf0; 
		}
		else if(AlarmNum == 2)
		{
			ZoneAlarm[0] |= 0x0f; 
		}
		else if(AlarmNum == 3)
		{
			ZoneAlarm[1] |= 0xf0; 
		}
		else if(AlarmNum == 4)
		{
			ZoneAlarm[1] |= 0x0f; 
		}
	}
	
}
u8 RainFallCntF = 0;
u8 MasterTaskin = 0;
u8 MasterTaskout = 0;
void MasterCtrl_task(void *pdata)
{
	//u8 os_err;
	//MsgStruct * pMsgBlk = NULL;
	//MsgStruct Msgtemp;
//	u8 TestTemp[4];
//	OS_CPU_SR  cpu_sr;
//	InitativeStruct InitiativeParaTemp;
	MasterQInit();
	while(1)
	{
		delay_ms(50);
		MasterTaskin++;
		if(RainFlag == 1)
		{
			RainFallCntF = 1;
		}
		else if((RainFallCntF == 1) && (RainFlag == 0))
		{
			RainfallClearF = 1;
			RainFallCntF = 0;
		}
		MasterTaskout++;
	}

}

