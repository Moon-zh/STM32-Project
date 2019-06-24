#ifndef __USERCORE_H
#define __USERCORE_H	

#include "sys.h"
#include "includes.h"  

#define MANUAL_MODE 	1//�ֶ�ģʽ
#define LOCAL_MODE		2//����ģʽ
#define NET_MODE		3//����ģʽ

//WARTERADD״̬
#define WARTERADD_IDLE				0//עˮ ����
#define WARTERADD_WAITESWITCH		1//עˮ �ȵ�ŷ�
//#define WARTERADD_WAITELEVEL		2//עˮ	��Һλ
#define WARTERADD_WAITEFULL			3//עˮ	ˮ��
//#define WARTERADD_END				4//עˮ	����
#define WATERADD_CANCEL				5//עˮ ȡ��
//WARTERADD��Ϣ
#define MSG_WATERADDSTART		1//עˮ
#define MSG_SWITCHOPEN			2//��ŷ���
#define MSG_SWITCHERR			3//��ŷ�ʧ��
#define MSG_LEVEGET				4//��ȡ��Һλ����
#define MSG_LEVEERR				5//Һλ��ȡʧ��
#define MSG_WATERFULL			6//ˮ��
#define MSG_WATERADDCANCEL		7//ȡ��

//STIR״̬
#define STIR_IDLE				0//���� ����
#define STIR_WATESWITCH_ON		1//���� �ȵ�ŷ�
#define STIR_VERIFYSWITCH_ON 	2//���� ��֤��ŷ�
#define STIR_WAITETIME 			3//���� ʱ�䵽
#define STIR_WATESWITCH_OFF		4//���� �ȵ�ŷ�
#define STIR_VERIFYSWITCH_OFF 	5//���� ��֤��ŷ�
#define STIR_END				6//���� ����
//STIR ��Ϣ
#define MSG_STIRSTART			1//����
#define MSG_SWITCHONOK			2//��ŷ���OK
#define MSG_SWITCHONERROR		3//��ŷ�������
#define MSG_VERFYONOK			4//�ȴ�����֤ OK
#define MSG_VERFYONERR			5//�ȴ�����֤ ERR
#define MSG_TIMEOVER			6//ʱ�䵽
#define MSG_SWITCHOFFOK			7//��ŷ���OK
#define MSG_SWITCHOFFERROR		8//��ŷ��ش���
#define MSG_VERFYOFFOK			9//�ȴ�����֤ OK
#define MSG_VERFYOFFERR			10//�ȴ�����֤ ERR
#define MSG_STIRCANCEL			11//����ȡ��





#define GPRS_TASK_CODE 		2//����ͨ������	
#define ZONE_TASK_CODE 		3//������������
#define IO_TASK_CODE 		4//����IO����
#define SCREEN_TASK_CODE 	5//����������
#define TIMER_INTER_CODE	6//��ʱ��
#define ALARM_TASK_CODE		7//�澯����
#define WATERADD_TASK_CODE	8//
#define STIR_TASK_CODE		9//

//������
#define FAULT_SW1 0x01
#define FAULT_SW2 0x02
#define FAULT_SW3 0x04
#define FAULT_SW4 0x08
#define FAULT_SW5 0x10
#define FAULT_STIR1 0x20
#define FAULT_STIR2 0x40
#define FAULT_STIR3 0x80
#define FAULT_STIR4 0x100
#define FAULT_STIR5 0x200
#define FAULT_LEVEL1 0x400
#define FAULT_LEVEL2 0x800
#define FAULT_LEVEL3 0x1000
#define FAULT_LEVEL4 0x2000
#define FAULT_LEVEL5 0x4000

typedef struct
{
	u8 CmdType;
	u8 CmdSrc;
	u8 CmdData[2];
}MsgStruct;

extern u8 ControlMode ;//	����ģʽ��1������ 0:����
extern u8 WaterAddState1;
extern u8 WaterAddState2;
extern u8 WaterAddState3;
extern u8 WaterAddState4;
extern u8 WaterAddState5;

extern u8 StirState1;
extern u8 StirState2;
extern u8 StirState3;
extern u8 StirState4;
extern u8 StirState5;
extern u32 g_FaultCode;//���������ڴ�����Ϻͱ���ʹ��

void WaterAdd_task(void *pdata);
void Stir_task(void *pdata);
u8 PackSendMasterQ(MsgStruct* MsgBlk);
u8 InterruptPackSendMasterQ(MsgStruct* MsgBlk);
u8 PackSendWaterAddQ(MsgStruct* MsgBlk);
u8 PackSendStirQ(MsgStruct* MsgBlk);

#endif

