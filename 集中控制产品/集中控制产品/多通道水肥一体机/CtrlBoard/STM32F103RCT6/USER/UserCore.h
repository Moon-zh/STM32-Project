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
//MOCHINE״̬��
#define MOCHINE_IDLE    0 //��������״̬
#define MOCHINE_ACTION  1 //����ִ��״̬
#define MOCHINE_ALARM   2 //��������״̬
#define MOCHINE_STOP    3 //����ֹͣ״̬
//��Ϣ
#define MSG_START      0X10  //����������Ϣ
#define MSG_ACTION_WATER_ON      0X11  // ��ˮ��ŷ�����
#define MSG_ACTION_WATER_OFF      0X12  // ��ˮ��ŷ��ر�
#define MSG_ACTION_ERTILIZER_ON      0X13  // ʩ�ʵ�ŷ�����
#define MSG_ACTION_ERTILIZER_OFF      0X14  //ʩ�ʵ�ŷ��ر�
#define MSG_ALARM			      0X15						//������Ϣ
#define MSG_STOP			      0X16						//ֹͣ��Ϣ	
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
#define IO_SEND_COUNT_MAX    20 //�ݶ�Ϊ1000�� ������
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
enum MOCHINEerrorstate
{
		MOCH_E_STATE_START_ON =1, //����ʧ��  
		MOCH_E_STATE_WATER_ON =2, //��ˮ��������ʧ��
		MOCH_E_STATE_FERT_ON  =3, //ʩ�ʿ���ʧ��
		MOCH_E_STATE_FLOW_ON  =4 //�����ƶ�ȡ����


};
typedef struct
{
u8 FertilizerOnID; //ʩ�ʱÿ���ID ��8·IO���õ�ID ��Ϊֻ��Ӧһ��ģ�����豸��װ���֮����Խ���̶�
u8 FertilizerIoNumber;//ʩ�ʵ�ŷ�Ҫ������·�� ��1-5��˳����
u8 FertilizerIoStatrNum; //��ǰ����ʩ�ʱõ�·��
u8 FertilizerIoSucess; //ʩ�ʷ������ɹ���ʼ��ʱ
u8 WaterOnID;			//��Ӧ��Ҫ������������ˮID 4·IO��ID
u8 WaterOnSucess; //��ˮ�������ɹ���ʼ��ʱ
u16 Fertilizertime; //ʩ����ʱ�����
u16 FertilizerCurrenttime;//��ǰʩ����ʱ��
u16 FertilizerCurrenttimeing;//ʩ�ʽ��е�ʱ��	
u16 WaterTime;		//��ˮ��ʱ��
u16 WaterTimeing;	//��ˮ���е�ʱ�� 
u8 Io_on_Off_Sucess;//io����ѭ����־
u8 Io_Step; //io��������	 ���ڹ���ͬһ��485ֻ��һ��һ������
u32	lastFlowmeter1;//������1����ֵ Ҫ�ϴ�
u32	lastFlowmeter2;//������2����ֵ Ҫ�ϴ�
u32	lastFlowmeter3;//������3����ֵ Ҫ�ϴ�
u32	lastFlowmeter4;//������4����ֵ Ҫ�ϴ�
u32	lastFlowmeter5;//������5����ֵ Ҫ�ϴ�
u16 u16MOCHINEerrorstate;	//ϵͳ����״̬
}
MOCHINEStruct;
extern u16 WarterRemainderTime ;//��ˮʣ�๤��ʱ��
extern u16 FertilizerRemainderTime ;//ʩ��ʣ�๤��ʱ��
extern u8 ZoneAlarm[2];

//extern u8 ModbusCoil[MAX_COIL_NUM/8];//modbus��Ȧ����40��

void MasterCtrl_task(void *pdata);
u8 SetStrategy(StrategyStruct *Strategy);
u8 PackSendMasterQ(MsgStruct* MsgBlk);
u8 InterruptPackSendMasterQ(MsgStruct* MsgBlk);

#endif
