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

//鉴权信息
extern char ProductKey_NB0[20];
extern char DeviceName_NB0[50];
extern char DeviceSecret_NB0[50];

extern char *ProductKey_NB;
extern char *DeviceName_NB;
extern char *DeviceSecret_NB;


////json包内容  topic上报目录
//static char 	*topic=	"thing/event/property/post\"";
//static char 	*id=	"\\\"id\\\":\\\"123\\\"";
//static char 	*iotid=	"\\\"iotId\\\":\\\"JrVJtxjxAEhjx8PNkq8S000100\\\"";
//static char  *uniMsgId=	"\\\"uniMsgId\\\":\\\"4434535619901050840\\\"";
//static char      *post="\\\"method\\\":\\\"thing.event.property.post\\\",\\\"params\\\":{";

//static char  *wartopic=	"user/war\"";
//static char   *warpost="\\\"method\\\":\\\"user.war\\\",\\\"params\\\":{";

//void	NeoWayN21_init(void);	//有方N21初始化
//void	conN21(void);			//N21连接到阿里云
//void	disconN21(void);		//断开IMQTT连接
//void	sendN21(char *data,char *hc,char *tp);	//向订阅的topic发送数据
//void N21Reset(void);
//void SetRemoteRead(u8 addr,s16 value);
//void SetRemotePara(u8 addr,s16 value);
//u8 PackSendRemoteQ(MsgStruct* MsgBlk);
//void N21Reset(void);

#endif
