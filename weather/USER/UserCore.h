#ifndef __USERCORE_H
#define __USERCORE_H	

#include "sys.h"
#include "includes.h"  

#define MANUAL_MODE 1//�ֶ�ģʽ
#define LOCAL_MODE	2//����ģʽ
#define NET_MODE	3//����ģʽ
//MASTER״̬
#define WARTER_IDLE			0//��ˮ����״̬
#define WARTER_WORK 		1//��ˮ����״̬
#define FERTILIZER_IDLE		0//ʩ�ʿ���
#define FERTILIZER_WORK		1//ʩ�ʹ���
//��Ϣ
#define MSG_WARTERING		1//��ˮ
#define MSG_FERTILIZER		2//ʩ��
#define MSG_MODECHANGE		3//ģʽ
#define MSG_RADIOTUBE		4//��ŷ�
#define MSG_ZONEALARM		5//�����澯


#define USERCORE_TASK_CODE 	1//������
#define GPRS_TASK_CODE 		2//����ͨ������	
#define ZONE_TASK_CODE 		3//������������
#define IO_TASK_CODE 		4//����IO����
#define SCREEN_TASK_CODE 	5//����������
#define TIMER_INTER_CODE	6//��ʱ��

typedef struct
{
	u8 CmdType;
	u8 CmdSrc;
	u8 CmdData[2];
}MsgStruct;

typedef struct
{
	u8 PumpWFlag;	//ˮ���Ƿ���
	u8 PumpFFlag;	//�ʱ��Ƿ���
	u8 Passageway[6]; //ͨ����λ����
	u8 Zone;		//����������
	u8 WorkHour;	//����ʱ��(h)
	u8 WorkMinute;	//����ʱ��(m)
	u8 WorkDay; 	//�����ܼ�
	u8 StartHour;	//��ʼСʱ
	u8 StartMinute; //��ʼ����
}
StrategyStruct;

extern u16 WarterRemainderTime ;//��ˮʣ�๤��ʱ��
extern u16 FertilizerRemainderTime ;//ʩ��ʣ�๤��ʱ��
extern u8 ZoneAlarm[2];

//extern u8 ModbusCoil[MAX_COIL_NUM/8];//modbus��Ȧ����40��

void MasterCtrl_task(void *pdata);
u8 SetStrategy(StrategyStruct *Strategy);
u8 PackSendMasterQ(MsgStruct* MsgBlk);
u8 InterruptPackSendMasterQ(MsgStruct* MsgBlk);

#endif
