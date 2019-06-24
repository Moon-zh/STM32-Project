#ifndef __LOGIC_H
#define __LOGIC_H
#include "Includes.h"


typedef struct
{
 u32 flower1;//ˮ��1
 u32 flower2;//ˮ��2
 u32 flower3;//ˮ��3
 u32 flower4;//ˮ��4
 u32 lastflower1;//�ϴ�ˮ��1����
 u32 lastflower2;//�ϴ�ˮ��2����
 u32 lastflower3;//�ϴ�ˮ��3����
 u32 lastflower4;//�ϴ�ˮ��4����	
 u32 startflower1;//��ʼˮ��1����	
 u32 startflower2;//��ʼˮ��2����	
 u32 startflower3;//��ʼˮ��3����	
 u32 startflower4;//��ʼˮ��4����		
}__attribute__((packed))fertask;

typedef union
{
	fertask ufertask;
  u8 flowerdatabuf[48];

}fertaskdata;

typedef struct
{
 float M_A;//a�޷���
 float M_B;//B�޷���
 float M_C;//C�޷���
 float M_D;//D�޷���
 float USE_M_A;//a�޷���ʹ��
 float USE_M_B;//B�޷���
 float USE_M_C;//C�޷���
 float USE_M_D;//D�޷���
 u8 amount; //Ķ��ʩ����
 u8 Th_A;//AŨ��
 u8 Th_B;//AŨ��
 u8 Th_C;//AŨ��
 u8 Th_D;//AŨ��	
 u8 formula_A;//�䷽A
 u8 formula_B;//�䷽B
 u8 formula_C;//�䷽C
 u8 formula_D;//�䷽D	
	
 float PH_A;//A���ܶ�	
 float PH_B;//A���ܶ�	
 float PH_C;//A���ܶ�	
 float PH_D;//A���ܶ�	
 float Q_A;//A��ʼ����������	
 float Q_B;//A��ʼ����������	
 float Q_C;//A��ʼ����������	
 float Q_D;//A��ʼ����������	
}__attribute__((packed))fercal;	//ʩ�ʼ�����Ҫ�ĸ�������

//typedef union
//{
//	fertask ufertask;
//  u8 flowerdatabuf[64];

//}fertaskdata;
void FerLogic_task(void *pdata);
void Copy_Folwer(void);
void Chanel_Fer_Weight(u16 data);
float PH_Cal(float pot);
#endif
