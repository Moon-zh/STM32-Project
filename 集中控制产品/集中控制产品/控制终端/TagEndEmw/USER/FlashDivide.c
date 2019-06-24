#include "W25Qxx.h"
#include "Includes.h"
#include "FlashDivide.h"

/********************************************************************
//读浇水程序
*********************************************************************/
void FlashReadWaterProg(u8* _pBuf,u8 addr,u8 Num)
{
	W25QXX_Read ( _pBuf, ADDR_WARTERPROG+addr*6, Num );//保存时间为2字节
}
/********************************************************************
//写浇水程序
*********************************************************************/
void FlashWriteWaterProg(u8* _pBuf,u8 Num)
{
	W25QXX_Write ( _pBuf, ADDR_WARTERPROG, Num);
}
/********************************************************************
//读配肥程序
*********************************************************************/
void FlashReadDosingProg(u8* _pBuf,u8 addr,u8 Num)
{
	W25QXX_Read ( _pBuf, ADDR_DOSINGPROG+addr*5, Num );//保存时间为2字节
}
/********************************************************************
//写配肥程序
*********************************************************************/
void FlashWriteDosingProg(u8* _pBuf,u8 Num)
{
	W25QXX_Write ( _pBuf, ADDR_DOSINGPROG, Num );
}
/********************************************************************
//读灌溉程序
*********************************************************************/
void FlashReadIrrigationProg(u8* _pBuf,u8 addr,u8 Num)
{
	W25QXX_Read ( _pBuf, ADDR_IRRIGATIONPROG+addr*3, Num );//保存时间为2字节
}
/********************************************************************
//写灌溉程序
*********************************************************************/
void FlashWriteIrrigationProg(u8* _pBuf,u8 Num)
{
	W25QXX_Write ( _pBuf, ADDR_IRRIGATIONPROG, Num );
}
/********************************************************************
//读用肥量
*********************************************************************/
void FlashReadFertilizerAmount(u8* _pBuf,u8 addr,u8 Num)
{
	W25QXX_Read ( _pBuf, ADDR_FERTILIZERAMOUNT+addr, Num );//保存时间为2字节
}
/********************************************************************
//写用肥量
*********************************************************************/
void FlashWriteFertilizerAmount(u8* _pBuf,u8 Num)
{
	W25QXX_Write ( _pBuf, ADDR_FERTILIZERAMOUNT, Num );
}
/********************************************************************
//读固定参数
*********************************************************************/
void FlashReadFixedPara(u8* _pBuf,u8 addr,u8 Num)
{
	W25QXX_Read ( _pBuf, ADDR_FIXEDPARA+addr, Num );//保存时间为2字节
}
/********************************************************************
//写固定参数
*********************************************************************/
void FlashWriteFixedPara(u8* _pBuf,u8 Num)
{
	W25QXX_Write ( _pBuf, ADDR_FIXEDPARA, Num );
}
/********************************************************************
//读参数
*********************************************************************/
void FlashReadPara(u8* _pBuf,u8 addr,u8 Num)
{
	W25QXX_Read ( _pBuf, ADDR_PARA, Num*2 );//保存时间为2字节
}
/********************************************************************
//写参数
*********************************************************************/
void FlashWritePara(u8* _pBuf,u8 Num)
{
	W25QXX_Write ( _pBuf, ADDR_PARA, Num*2 );
}

void FlashReadWiFi(u8* _pBuf)
{
	W25QXX_Read ( _pBuf, ADDR_PARASET, 110 );//保存时间为2字节
}
void FlashWriteWiFi(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_PARASET, 110);
}
void FlashReadID(u8* _pBuf)
{
	W25QXX_Read ( _pBuf, ADDR_DEVICEID, 1 );//保存时间为2字节
}
void FlashWriteID(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_DEVICEID, 1);
}

