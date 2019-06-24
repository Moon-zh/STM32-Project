#ifndef __Emw3060_H
#define __Emw3060_H
#include "UserCore.h"
#include "string.h"    
#include "Includes.h"

#define IMQTTAUTH_STEP		0
#define IMQTTPARA_STEP		1
#define IMQTTCONN_STEP		2
#define IMQTTSUB_STEP		3
#define FINISH_STEP			4
#define DISCONNECT_STEP		5
#define RD_COIL_STA		01	//����Ȧ
#define	RD_INPUT_STA	02	//��������ɢ��
#define RD_HOLDING_REG	03	//���Ĵ���
#define	RD_INPUT_REG	04	//������Ĵ���
#define	WR_SINGLE_COIL	05	//д������Ȧ
#define	WR_SINGLE_REG	06	//д�����Ĵ���
#define WR_MUL_COIL		15	//д�����Ȧ
#define	WR_MUL_REG		16	//д����Ĵ���
#define INITIATIVE_COIL_STA	0X45//�����ϱ���Ȧ
#define INITIATIVE_INPUT_STA 0X42//�����ϱ�������ɢ��
#define INITIATIVE_HOLDING_REG 0x46 //�����ϱ����ּĴ���
#define INITIATIVE_INPUT_REG 0X44 //�����ϱ�����Ĵ���
typedef struct
{
	u8 buf[512];
	u8 WritePiont;
	u8 ReadPiont;
}Ring;
#define ParaNum 4

//��Ȩ��Ϣ
extern char ProductKey0[20];
extern char DeviceName0[50];
extern char DeviceSecret0[50];

extern char *ProductKey;
extern char *DeviceName;
extern char *DeviceSecret;

//json������  topic�ϱ�Ŀ¼
static char 	*topicw=	"thing/event/property/post\"";
static char      *postw="\\\"params\\\":{";

static char  *wartopicw=	"user/war\"";
static char   *warpostw="\\\"params\\\":{";


void	Emw3060_init(void);
void	Emw3060_con(void);
unsigned char sendEmw(char *data,unsigned char w);
void SetRemoteRead(u8 addr,s16 value);
void SetRemotePara(u8 addr,s16 value);
u8 PackSendRemoteQ(MsgStruct* MsgBlk);

#endif
