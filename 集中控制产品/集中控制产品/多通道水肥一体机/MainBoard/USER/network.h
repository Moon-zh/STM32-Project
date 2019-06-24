#ifndef NETWORK_H
#define NETWORK_H

//#include "DataTypes.h"
//#include "user_app.h"
#include "includes.h"

typedef struct
{
	u16 pre_flushtime;//ǰ�ó�ϴʱ��
	u16 fer_irrtimr; //ʩ��ʱ��
	u16 post_flushtimr;	//���ó�ϴʱ��
	u16 AferPer;	//A�ʰٷֱ�
	u16 BferPer;	//B�ʰٷֱ�
	u16 CferPer;	//C�ʰٷֱ�
	u16 DferPer;	//D�ʰٷֱ�
  	u16 are_fer;	//Ķ���÷�KG
}__attribute__((packed))IrrProStruct;
typedef struct
{
	u16 PartitionState[16];//16����״̬
	u16 DeviceState;//�豸״̬ 
	u16 TimeCountDown;//����ʱ
	u16 IrrProj;//��ȳ����
	u16 BucketFerPer[4];	//���Ϲ��ڷ���Ũ��
	u16 PartitionAre[16]; //�������
	//u16 post_flushtimr;	//����ʼ��
	IrrProStruct CFIP;	//����ʼ��
	IrrProStruct S1IP;	//����ʼʱ
	IrrProStruct S2IP;	//����ʼ��
	IrrProStruct S3IP;	//����ʼ��
  	IrrProStruct S4IP;//���ģʽ
}__attribute__((packed))NetReadStruct;

typedef union
{
	NetReadStruct NetRead;
	s16 Remote_Read[79];
}__attribute__((packed))RemoteReadStruct;
typedef struct
{
	u16 PartitionEnable[16];
	u16 IrrType;
	u16 partitionSel[16];
	u16 IrrTime;
	u16 IrrPioj;
	u16 Switch;
}__attribute__((packed))NetSetStruct;

typedef union
{
	NetSetStruct NetSet;
	s16 Remote_Para[36];
}__attribute__((packed))RemoteSetStruct;

extern const u8* SetText[];
extern const u8* ReadText[];
extern RemoteReadStruct ReRead;
extern RemoteSetStruct ReSet;

#endif
