#ifndef __NeowayN21_H
#define __NeowayN21_H

#include "string.h"    
#include "Includes.h"




typedef struct
{
	u8 buf[256];
	u8 WritePiont;
	u8 ReadPiont;
}Ring_NB;
#define ParaNum_NB 4

//��Ȩ��Ϣ
extern char ProductKey_NB0[20];
extern char DeviceName_NB0[50];
extern char DeviceSecret_NB0[50];

extern char *ProductKey_NB;
extern char *DeviceName_NB;
extern char *DeviceSecret_NB;


////json������  topic�ϱ�Ŀ¼
//static char 	*topic=	"thing/event/property/post\"";
//static char 	*id=	"\\\"id\\\":\\\"123\\\"";
//static char 	*iotid=	"\\\"iotId\\\":\\\"JrVJtxjxAEhjx8PNkq8S000100\\\"";
//static char  *uniMsgId=	"\\\"uniMsgId\\\":\\\"4434535619901050840\\\"";
//static char      *post="\\\"method\\\":\\\"thing.event.property.post\\\",\\\"params\\\":{";

//static char  *wartopic=	"user/war\"";
//static char   *warpost="\\\"method\\\":\\\"user.war\\\",\\\"params\\\":{";

//void	NeoWayN21_init(void);	//�з�N21��ʼ��
//void	conN21(void);			//N21���ӵ�������
//void	disconN21(void);		//�Ͽ�IMQTT����
//void	sendN21(char *data,char *hc,char *tp);	//���ĵ�topic��������
//void N21Reset(void);
//void SetRemoteRead(u8 addr,s16 value);
//void SetRemotePara(u8 addr,s16 value);
//u8 PackSendRemoteQ(MsgStruct* MsgBlk);
//void N21Reset(void);

#endif
