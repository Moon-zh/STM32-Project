#include "W25Qxx.h"
#include "Includes.h"
#include "FlashDivide.h"

/********************************************************************
*函数名:vFlashRdVerInfo
*功能:从FLASH中读版本信息
*输入:pBuf:数据源缓冲区
*返回值:无
*作者:alfred
*********************************************************************/
void FlashReadWaterTime(u8* _pBuf)
{
	W25QXX_Read ( _pBuf, ADDR_WARTERTIME, 2 );//保存时间为2字节
}
void FlashWriteWaterTime(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_WARTERTIME, 2 );
}

void FlashReadFertilizerTime(u8* _pBuf)
{
	W25QXX_Read ( _pBuf, ADDR_FERTILIZERTIME, 2 );//保存时间为2字节
}
void FlashWriteFertilizerTime(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_FERTILIZERTIME, 2 );
}


