#ifndef __NeowayN21_H
#define __NeowayN21_H




//json包内容  topic上报目录
static char 	*topic=	"thing/event/property/post\"";
static char      *post="\\\"method\\\":\\\"thing.event.property.post\\\",\\\"params\\\":{";

static char  *wartopic=	"user/war\"";
static char   *warpost="\\\"method\\\":\\\"user.war\\\",\\\"params\\\":{";


static char 	*id=	"\\\"id\\\":\\\"123\\\"";
static char 	*iotid=	"\\\"iotId\\\":\\\"JrVJtxjxAEhjx8PNkq8S000100\\\"";
static char  *uniMsgId=	"\\\"uniMsgId\\\":\\\"4434535619901050840\\\"";

void	NeoWayN21_init(void);	//有方N21初始化
void	conN21(void);			//N21连接到阿里云
void	disconN21(void);		//断开IMQTT连接
u8	sendN21(char *data,unsigned char w);	//向订阅的topic发送数据


#endif
