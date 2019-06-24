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

/********************************************************************
//读任务记录
*********************************************************************/
void FlashReadtaskrecord(u8* _pBuf,u32 addr,u16 Num)
{
	W25QXX_Read ( _pBuf, addr, Num );//保存时间为2字节
}
/********************************************************************
//更新任务记录
*********************************************************************/
void FlashWritetaskrecord(u8* _pBuf,u32 addr,u16 Num)
{
	W25QXX_Write ( _pBuf, addr, Num );
}
//读任务记录
/*********************************************************************/
void FlashReadtargetecord(u8* _pBuf,u16 Num)
{
	W25QXX_Read ( _pBuf, ADDR_UNCOMPLET, Num );//保存时间为2字节
}
/********************************************************************
//更新任务记录
*********************************************************************/
void FlashWritetargetrecord(u8* _pBuf,u16 Num)
{
	W25QXX_Write ( _pBuf, ADDR_UNCOMPLET, Num );
}
void FlashReadWiFi(u8* _pBuf)
{
	W25QXX_Read ( _pBuf, ADDR_PARASET, 126 );//保存时间为2字节
}
void FlashWriteWiFi(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_PARASET, 126);
}
void FlashReadDHCP(u8* _pBuf)
{
	W25QXX_Read ( _pBuf, ADDR_DHCPSET, 1 );//保存时间为2字节
}
void FlashWriteDHCP(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_DHCPSET, 1);
}


