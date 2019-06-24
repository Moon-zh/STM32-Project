#ifndef __PCHMI_H
#define __PCHMI_H	
#include "sys.h"	 

//�����������ֵ����
#define PC_COIL_NUM  	88//����Ϊ8��������
#define PC_INSTA_NUM 	48	//MAX_INSTA_NUM /8���ܴ��� 256-9  ֡���Ȳ��ܴ���256 ����Ϊ8��������
#define PC_REG_NUM		255
#define PC_INREG_NUM	20

typedef struct
{
	u8 RevType;//0 ���մ��� 1 �ӻ����� 2 ��������  
	u8 SlaveTxFlag;//�ӻ����ͱ�־ 1 ��Ҫ����
	u8 MasterState;//����״̬ 
	u16 MasterErrCnt;//�����������
	u8 Off_Line;
} PCCtrlStruct;
void PCHmi_task(void *pdata);
#endif	   

