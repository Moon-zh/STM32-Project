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

#define UPLOAD_PARTITIONENABLE		01
#define UPLOAD_IRRPARA				02
#define UPLOAD_PERCENT				03
#define UPLOAD_IRRSTATE				04
#define UPLOAD_PARTITIONARE			05
#define UPLOAD_CFIP					06
#define UPLOAD_S1IP					07
#define UPLOAD_S2IP					8
#define UPLOAD_S3IP					9
#define UPLOAD_S4IP					10
#define READTIME					11
typedef struct
{
	u8 buf[512];
	u8 WritePiont;
	u8 ReadPiont;
}Ring;
#define ParaNum 36
typedef struct
{
	u16 Bit1:1;
	u16 Bit2:1;
	u16 Bit3:1;
	u16 Bit4:1;
	u16 Bit5:1;
	u16 Bit6:1;
	u16 Bit7:1;
	u16 Bit8:1;
	u16 Bit9:1;
	u16 Bit10:1;
	u16 Bit11:1;
	u16 Bit12:1;
	u16 Bit13:1;
	u16 Bit14:1;
	u16 Bit15:1;
	u16 Bit16:1;
}U16Bit;
typedef union
{
	U16Bit WordBit;
	u16 Word;
}U16Union;
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

//extern char ssid[31];
//extern char password[31];
// char ssid[31]="CU_Z4eg";
// char password[31]="z4egdkuj";
// char ipaddr[16]="";
// char subnet[16]="";
// char gateway[16]="";
// char dns[16]="";

void	Emw3060_init(void);
void	Emw3060_con(void);
unsigned char sendEmw(char *data,unsigned char w);
void SetRemoteRead(u8 addr,s16 value);
void SetRemotePara(u8 addr,s16 value);
u8 PackSendRemoteQ(MsgStruct* MsgBlk);

#endif
