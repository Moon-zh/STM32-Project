#ifndef __NeowayN21_H
#define __NeowayN21_H

//json������  topic�ϱ�Ŀ¼

void	NeoWayN21_init2(void);	//�з�N21��ʼ��
void	conN212(void);			//N21���ӵ�������
void	disconN212(void);		//�Ͽ�IMQTT����
unsigned char sendN212(char *data,unsigned char w);	//���ĵ�topic��������

#endif
