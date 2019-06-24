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
void FlashReadWaterTime(u8* _pBuf)
{
	W25QXX_Read ( _pBuf, ADDR_WARTERTIME, 2 );//����ʱ��Ϊ2�ֽ�
}
void FlashWriteWaterTime(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_WARTERTIME, 2 );
}

void FlashReadFertilizerTime(u8* _pBuf)
{
	W25QXX_Read ( _pBuf, ADDR_FERTILIZERTIME, 2 );//����ʱ��Ϊ2�ֽ�
}
void FlashWriteFertilizerTime(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_FERTILIZERTIME, 2 );
}
// עˮ������д
void FlashReadWaterADD(u8* _pBuf)
{
	W25QXX_Read ( _pBuf, ADDR_WARTERTIME, 5 );//����ʱ��Ϊ2�ֽ�
}
void FlashWriteWaterADD(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_WARTERTIME, 5 );
}
// ���������д
void FlashReadStir(u8* _pBuf)
{
	W25QXX_Read ( _pBuf, ADDR_FERTILIZERTIME, 5 );
}
void FlashWriteStir(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_FERTILIZERTIME, 5 );
}

// �ٷֱȶ�д
void FlashReadPersent(u8* _pBuf)
{
	W25QXX_Read ( _pBuf, ADDR_PERSENT, 5 );
}
void FlashWritePersent(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_PERSENT, 5 );
}
// Һλ����
void FlashReadLow(u8* _pBuf)
{
	W25QXX_Read ( _pBuf, ADDR_LOW, 5 );//����ʱ��Ϊ2�ֽ�
}
void FlashWriteLow(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_LOW, 5 );
}
// Ͱֱ��
void FlashReadD(u8* _pBuf)
{
	W25QXX_Read ( _pBuf, ADDR_D, 10 );//����ʱ��Ϊ2�ֽ�
}
void FlashWriteD(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_D, 10 );
}

void FlashReadWiFi(u8* _pBuf)
{
	W25QXX_Read ( _pBuf, ADDR_PARASET, 62 );//����ʱ��Ϊ2�ֽ�
}
void FlashWriteWiFi(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_PARASET, 62);
}



