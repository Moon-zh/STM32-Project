#ifndef __USERCORE_H
#define __USERCORE_H	

#include "sys.h"
#include "includes.h"  
#define STATE_START		            0x00
#define STATE_1		                0x01
#define STATE_2		                0x02
#define STATE_3		                0x03
#define STATE_4		                0x04
#define STATE_END		            0x05
#define CMD_RETURN_REAL_TIME_DATA      0x06
#define MANUAL_MODE 1//�ֶ�ģʽ
#define LOCAL_MODE	2//����ģʽ
#define NET_MODE	3//����ģʽ
//MASTER״̬
#define MASTER_IDLE		0//����
#define MASTER_WORK		1//����
#define MASTER_ERROR	2//�쳣
#define MASTER_END		3//����
#define MASTER_CANCLE	4//ȡ��

//��Ϣ
#define MSG_START      	0X10  	//����������Ϣ
#define MSG_NEXTTIME	0X11 	//��һ��
#define MSG_ALARM		0X12	//������Ϣ
#define MSG_ERROR		0X13	//������Ϣ
#define MSG_STOP		0X14	//ֹͣ��Ϣ	
#define MSG_END			0X15	//������Ϣ	
#define MSG_SINGEND		0X16	//�������������Ϣ
#define MSG_CANCLEOK	0X17	//ȡ�������Ϣ


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
	u8 Before;//ʩ��ǰ��ˮʱ��
	u8 Total;//�ܽ�ˮʱ��
	u8 After;//ʩ�ʺ�ˮʱ��
}WaterProgStruct;
typedef struct
{
	u8 Ch1Ratio;
	u8 Ch2Ratio;
	u8 Ch3Ratio;
	u8 Ch4Ratio;
	u8 Ch5Ratio;
}DosingProgStruct;
typedef struct
{
	u8  Fertilizer1Percent;
	u8  Fertilizer2Percent;
	u8  Fertilizer3Percent;
	u8  Fertilizer4Percent;
	u8  Fertilizer5Percent;
}PercentStruct;
typedef struct
{
	//����������
	u32 ActionDay;
	u8 ActionType[32];	//32��ÿ��Ķ������� ��ˮ ʩ�� �޶���
	//����8�������
	u8 ValveGroup[8];	//�����
	u8 RunTimeNum[8];	//��ˮ�����
	u8 DosingProg[8];	//ʩ�ʳ����
	//����4�������
	u8 StartTime[4];//��ʼʱ��
	u8 CycleTimes[4];//ѭ������
	u8 CyclePperiod[4];//ѭ������
}ProgramStruct;
typedef struct
{
	u8 ActionTypeIndex;//������������
	
	u8 ValveGroupIndex;//���������
	u8 RunTimeNumIndex;//��ˮ���������
//	u8 DosingProgIndex;//ʩ�ʳ��������

	u8 StartTimeIndex;//��ʼʱ������
	u8 CycleTimesIndex;//ѭ����������
	u8 CyclePperiodIndex;//ѭ����������

	u8 WaterProgIndex;//��ˮ���������
	u8 DosingProgIndex;//��ʳ��������
	u8 IrrigationProgIndex;//��ȳ��������
}MasterStruct;

extern u16 Hmi_Buf[10];
extern u8 Master_State;
void MasterCtrl_task(void *pdata);
u8 PackSendMasterQ(MsgStruct* MsgBlk);
u8 InterruptPackSendMasterQ(MsgStruct* MsgBlk);

#endif
