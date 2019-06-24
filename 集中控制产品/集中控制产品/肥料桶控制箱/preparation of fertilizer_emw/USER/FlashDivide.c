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
// 注水参数读写
void FlashReadWaterADD(u8* _pBuf)
{
	W25QXX_Read ( _pBuf, ADDR_WARTERTIME, 5 );//保存时间为2字节
}
void FlashWriteWaterADD(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_WARTERTIME, 5 );
}
// 搅拌参数读写
void FlashReadStir(u8* _pBuf)
{
	W25QXX_Read ( _pBuf, ADDR_FERTILIZERTIME, 5 );
}
void FlashWriteStir(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_FERTILIZERTIME, 5 );
}

// 百分比读写
void FlashReadPersent(u8* _pBuf)
{
	W25QXX_Read ( _pBuf, ADDR_PERSENT, 5 );
}
void FlashWritePersent(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_PERSENT, 5 );
}
// 液位下限
void FlashReadLow(u8* _pBuf)
{
	W25QXX_Read ( _pBuf, ADDR_LOW, 5 );//保存时间为2字节
}
void FlashWriteLow(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_LOW, 5 );
}
// 桶直径
void FlashReadD(u8* _pBuf)
{
	W25QXX_Read ( _pBuf, ADDR_D, 10 );//保存时间为2字节
}
void FlashWriteD(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_D, 10 );
}

void FlashReadWiFi(u8* _pBuf)
{
	W25QXX_Read ( _pBuf, ADDR_PARASET, 62 );//保存时间为2字节
}
void FlashWriteWiFi(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_PARASET, 62);
}



