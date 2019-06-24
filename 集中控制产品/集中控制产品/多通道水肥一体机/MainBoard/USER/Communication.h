#ifndef __COMMUNICATION_H
#define __COMMUNICATION_H
#include "Includes.h"

#define STATE_START		            0x00
#define STATE_1		                0x01
#define STATE_2		                0x02
#define STATE_3		                0x03
#define STATE_4		                0x04
#define STATE_END		            0x05

#define COM_WRITE  0X01 //���ڶ�
#define COM_READ   0X02//����д

#define CMD_RETURN_REAL_TIME_DATA      0x06
//#define CMD_RETURN_WRITE_TIME_DATA     0x06
typedef struct
{
 u8 head_h; //֡ͷ
 u8 head_l;
 u8 ID;//�ն�ID
 u8 code;//������
 u8 datanum;//�ֽ���
 u8 solenoidvalve;//ѡ��ĵ�ŷ�
 u8 state; //״̬
 u8 tail_h;//β��
 u8 tail_l;    



}tagend; //ͨѶ�ն˽ṹ��
typedef struct
{
	u8 CmdType;
	u8 CmdSrc;
	u8 CmdData[2];
}MsgComStruct;
void CHECKCtrl_task(void *pdata);
void writetagend(u8 zone,u8 state);//Ҫд�ķ���״̬
void board2com(u8 readwrite,u8 ioadress,u8 iostate);
u8 PackSendComQ(MsgComStruct* MsgBlk);
#endif
