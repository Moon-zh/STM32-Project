#ifndef __CGQ_H
#define __CGQ_H

#include	"sys.h"


//�Զ�ģʽ��ʹ�õı�־ Guangai.
typedef struct
{
	u8 qdyzbz;//�Զ�ģʽ����ʹ�ã�������ֵ��־���ɼ�����ʪ�ȣ�����������ֵ������ˮ��
	u8 tzyzbz;//�Զ�ģʽ����ʹ�ã�ֹͣ��ֵ��־���ɼ�����ʪ�ȣ�����ֹͣ��ֵֹͣ��ˮ��
	u8 zdms;//�Զ�ģʽ ��Ĭ��Ϊ0: 0Ϊ���Զ�ģʽ   1Ϊ���Զ�ģʽ��
	u8 qdbz;//�Զ�ģʽ������ˮ��رս�ˮ  ��Ĭ��Ϊ0 :  1Ϊ�ѿ�����ˮ  0Ϊ�ѹرս�ˮ��
	u8 yichi;//�Զ�ģʽ�£�ִ��һ�εı�־	
}Zidongmoshi;


//���Ͳ�ѯ��ص�ַ�¼Ĵ�����ַ������
void sendinstruct_cgq(u8 addr,u8 jcqdz1,u8 jcqdz2);
//��ȡ������ʪ�ȴ�����������
u8 readdata_cgq_tr(u8 ms);
//������-������ʪ�����ݴ���
void cgq_tr_sjcl(void);
//������-������ʪ�ȳ��� ����Ҫ����
void CGQ_cx(void);



extern u8 cgq_trsd[2];//����ʪ��
extern u8 cgq_trwd[3];//�����¶�,[0]Ϊ1��ʾΪ������[0]Ϊ0��ʾΪ����


extern u8 MasterState;//������״̬����
//�Զ�ģʽ��ʹ�õı�־
extern Zidongmoshi Guangai;


#endif

