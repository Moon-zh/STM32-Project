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
#define RD_COIL_STA		01	//读线圈
#define	RD_INPUT_STA	02	//读输入离散量
#define RD_HOLDING_REG	03	//读寄存器
#define	RD_INPUT_REG	04	//读输入寄存器
#define	WR_SINGLE_COIL	05	//写单个线圈
#define	WR_SINGLE_REG	06	//写单个寄存器
#define WR_MUL_COIL		15	//写多个线圈
#define	WR_MUL_REG		16	//写多个寄存器
#define INITIATIVE_COIL_STA	0X45//主动上报线圈
#define INITIATIVE_INPUT_STA 0X42//主动上报输入离散量
#define INITIATIVE_HOLDING_REG 0x46 //主动上报保持寄存器
#define INITIATIVE_INPUT_REG 0X44 //主动上报输入寄存器
typedef struct
{
	u8 buf[512];
	u8 WritePiont;
	u8 ReadPiont;
}Ring;
#define ParaNum 4

//鉴权信息
extern char ProductKey0[20];
extern char DeviceName0[50];
extern char DeviceSecret0[50];

extern char *ProductKey;
extern char *DeviceName;
extern char *DeviceSecret;

//json包内容  topic上报目录
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
