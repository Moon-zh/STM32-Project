#ifndef __NeowayN212_H
#define __NeowayN212_H

//json包内容  topic上报目录
static char 	*topic=	"thing/event/property/post\"";
static char      *post="\\\"method\\\":\\\"thing.event.property.post\\\",\\\"params\\\":{";

static char  *wartopic=	"user/war\"";
static char   *warpost="\\\"method\\\":\\\"user.war\\\",\\\"params\\\":{";

void	NeoWayN21_init(void);	//有方N21初始化
void	conN21(void);			//N21连接到阿里云
void	disconN21(void);		//断开IMQTT连接
unsigned char sendN21(char *data,unsigned char w);	//向订阅的topic发送数据

#endif
