#ifndef __NeowayN212_H
#define __NeowayN212_H

//json������  topic�ϱ�Ŀ¼
static char 	*topic=	"thing/event/property/post\"";
static char      *post="\\\"method\\\":\\\"thing.event.property.post\\\",\\\"params\\\":{";

static char  *wartopic=	"user/war\"";
static char   *warpost="\\\"method\\\":\\\"user.war\\\",\\\"params\\\":{";

void	NeoWayN21_init(void);	//�з�N21��ʼ��
void	conN21(void);			//N21���ӵ�������
void	disconN21(void);		//�Ͽ�IMQTT����
unsigned char sendN21(char *data,unsigned char w);	//���ĵ�topic��������

#endif
