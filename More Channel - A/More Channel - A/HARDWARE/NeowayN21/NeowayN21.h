#ifndef __NeowayN21_H
#define __NeowayN21_H

void	NeoWayN21_init(void);	//�з�N21��ʼ��
void	conN21(void);			//N21���ӵ�������
void	disconN21(void);		//�Ͽ�IMQTT����
void	N21GetTime(void);
unsigned char sendN21(char *data,unsigned char w);	//���ĵ�topic��������

#endif
