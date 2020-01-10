


#include "W25Qxx.h"
#include "Includes.h"
#include "FlashDivide.h"

/********************************************************************
*������:vFlashRdVerInfo
*����:��FLASH�ж��汾��Ϣ
*����:pBuf:����Դ������
*����ֵ:��
*����:alfred
*********************************************************************/
//void FlashReadWaterTime(u8* _pBuf)
//{
//	W25QXX_Read ( _pBuf, ADDR_WARTERTIME, 2 );//��ȡʱ��Ϊ2�ֽ�
//}
//void FlashWriteWaterTime(u8* _pBuf)
//{
//	W25QXX_Write ( _pBuf, ADDR_WARTERTIME, 2 );//����ʱ��Ϊ2�ֽ�
//}

/*
wifiʹ�õ�flash�洢 
����wifi���õģ�wifi���֣�wifi���룬IP,�������룬���أ�������
*/
void FlashReadWiFi(u8* _pBuf)  //wifiʹ�õ�
{
	W25QXX_Read ( _pBuf, ADDR_PARASET, 180 );
}
void FlashWriteWiFi(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_PARASET, 180);
}

/*
IP��ַ�Ƿ���Ҫ�ٴγ�ʼ��ʹ�õ�
(����ʹ��)
*/
void FlashReadIP(u8* _pBuf)  //IP��ַ��ʼ��ʹ�õ�
{
	W25QXX_Read ( _pBuf, ADDR_IP, 1);//��ȡΪ1�ֽ�
}
void FlashWriteIP(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_IP, 1 );//����Ϊ1�ֽ�
}

/*
ʱ���Ƿ���Ҫ�ٴζ�ȡ��ʹ�õ�
24Сʱ֮�����RTC��ʱ��
*/
void FlashReadSJ(u8* _pBuf)  //ʱ��ʹ�õ�
{
	W25QXX_Read ( _pBuf, ADDR_SJ, 1);//��ȡΪ1�ֽ�
}
void FlashWriteSJ(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_SJ, 1 );//����Ϊ1�ֽ�
}

/*
����DHCP���õĲ�����wifi����ʱʹ��
*/
void FlashReadDHCP(u8* _pBuf)  //DHCP��������ʹ�õ�
{
	W25QXX_Read ( _pBuf, ADDR_DHCP, 1);//��ȡΪ1�ֽ�
}
void FlashWriteDHCP(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_DHCP, 1 );//����Ϊ1�ֽ�
}


/*
�Զ�ģʽ�µ� ������ֵ��ֹͣ��ֵ�� ����
*/
void FlashReadZDMS_QDYZ(u8* _pBuf)  //������ֵ
{
	W25QXX_Read ( _pBuf, ADDR_ZDMS_QDYZ, 1);//��ȡΪ1�ֽ�
}
void FlashWriteZDMS_QDYZ(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_ZDMS_QDYZ, 1 );//����Ϊ1�ֽ�
}

void FlashReadZDMS_TZYZ(u8* _pBuf)  //ֹͣ��ֵ
{
	W25QXX_Read ( _pBuf, ADDR_ZDMS_TZYZ, 1);//��ȡΪ1�ֽ�
}
void FlashWriteZDMS_TZYZ(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_ZDMS_TZYZ, 1 );//����Ϊ1�ֽ�
}
