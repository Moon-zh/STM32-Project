#ifndef __NeowayN21_H
#define __NeowayN21_H




//json������  topic�ϱ�Ŀ¼
static char 	*topic=	"thing/event/property/post\"";
static char      *post="\\\"method\\\":\\\"thing.event.property.post\\\",\\\"params\\\":{";

static char  *wartopic=	"user/war\"";
static char   *warpost="\\\"method\\\":\\\"user.war\\\",\\\"params\\\":{";


static char 	*id=	"\\\"id\\\":\\\"123\\\"";
static char 	*iotid=	"\\\"iotId\\\":\\\"JrVJtxjxAEhjx8PNkq8S000100\\\"";
static char  *uniMsgId=	"\\\"uniMsgId\\\":\\\"4434535619901050840\\\"";

void	NeoWayN21_init(void);	//�з�N21��ʼ��
void	conN21(void);			//N21���ӵ�������
void	disconN21(void);		//�Ͽ�IMQTT����
u8	sendN21(char *data,unsigned char w);	//���ĵ�topic��������


#endif
