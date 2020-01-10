#include "W25Qxx.h"
#include "Includes.h"
#include "FlashDivide.h"

void	FlashWriteLog(u8* buf,u8 addr,u8 Num)	//写日志
{
	W25QXX_Write(buf,ADDR_Log+addr,Num);
}

void	FlashReadLog(u8* buf,u8 addr)			//读日志
{
	W25QXX_Read(buf,ADDR_Log+addr,20);
}

void	FlashWriteFer(u8* buf,u8 addr,u8 Num)	//写温室设置
{
	W25QXX_Write(buf,ADDR_Fer+addr,Num);
}

void	FlashReadFer(u8* buf,u8 addr)			//读温室设置
{
	W25QXX_Read(buf,ADDR_Fer+addr,10);
}

void	FlashWriteFer0(u8* buf)					//写肥料桶设置
{
	W25QXX_Write(buf,ADDR_Fer0,7);
}

void	FlashReadFer0(u8* buf)					//读肥料桶设置
{
	W25QXX_Read(buf,ADDR_Fer0,7);
}

void	FlashWriteMem(u8* buf)					//写日志地址
{
	W25QXX_Write(buf,ADDR_mem,4);
}

void	FlashReadMem(u8* buf)					//读日志地址
{
	W25QXX_Read(buf,ADDR_mem,4);
}
