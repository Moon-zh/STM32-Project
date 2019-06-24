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


