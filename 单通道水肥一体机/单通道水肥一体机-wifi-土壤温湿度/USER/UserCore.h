#ifndef __USERCORE_H
#define __USERCORE_H	

#include "sys.h"
#include "includes.h"  

#define FERTILIZER1TIME	10   //ʩ��1�׶εĽ���ˮʱ��
#define FERTILIZER3TIME	10   //ʩ��3�׶εĽ���ˮʱ��

#define MANUAL_MODE 1//�ֶ�ģʽ
#define LOCAL_MODE	2//����ģʽ
#define NET_MODE	3//����ģʽ

//MASTER״̬
#define SYSTEM_IDLE			0//����״̬
#define SYSTEM_WATER 		1//����״̬:����ˮ

#define SYSTEM_FERTILIZER1 		2//ʩ��1�׶�
#define SYSTEM_FERTILIZER2 		3//ʩ��2�׶�
#define SYSTEM_FERTILIZER3 		4//ʩ��3�׶�

//��Ϣ
#define MSG_START			1//����
#define MSG_STOP			2//ֹͣ
#define MSG_TIMEOVER		3//����ʱ����


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
	u8 CmdData[6];
}MsgStruct;

typedef struct
{
	u8 WorkModel; //1:��ˮ 2:ʩ��
	u8 Zone;	//���� ��λ
	u8 TimeH;
	u8 TimeM;
	u8 ImplementZone;//����ִ�е�����
	u8 FerTimeH;
	u8 FerTimeM;
}WorkPara;

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
u8 GetSysState(void);

void rct_dueishi_cx(void);//RCT��ʱ����  ��ѯ����ʱ�䣬����ʱ��ͬ����RTC��ÿ24Сʱͬ��һ��

#endif
