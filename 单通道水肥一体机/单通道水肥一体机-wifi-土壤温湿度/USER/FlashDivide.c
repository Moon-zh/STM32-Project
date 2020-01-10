


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
//void FlashReadWaterTime(u8* _pBuf)
//{
//	W25QXX_Read ( _pBuf, ADDR_WARTERTIME, 2 );//读取时间为2字节
//}
//void FlashWriteWaterTime(u8* _pBuf)
//{
//	W25QXX_Write ( _pBuf, ADDR_WARTERTIME, 2 );//保存时间为2字节
//}

/*
wifi使用的flash存储 
保存wifi配置的（wifi名字，wifi密码，IP,子网掩码，网关，域名）
*/
void FlashReadWiFi(u8* _pBuf)  //wifi使用的
{
	W25QXX_Read ( _pBuf, ADDR_PARASET, 180 );
}
void FlashWriteWiFi(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_PARASET, 180);
}

/*
IP地址是否需要再次初始化使用的
(调试使用)
*/
void FlashReadIP(u8* _pBuf)  //IP地址初始化使用的
{
	W25QXX_Read ( _pBuf, ADDR_IP, 1);//读取为1字节
}
void FlashWriteIP(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_IP, 1 );//保存为1字节
}

/*
时间是否需要再次读取的使用的
24小时之后更新RTC的时间
*/
void FlashReadSJ(u8* _pBuf)  //时间使用的
{
	W25QXX_Read ( _pBuf, ADDR_SJ, 1);//读取为1字节
}
void FlashWriteSJ(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_SJ, 1 );//保存为1字节
}

/*
保存DHCP设置的参数，wifi配置时使用
*/
void FlashReadDHCP(u8* _pBuf)  //DHCP参数保存使用的
{
	W25QXX_Read ( _pBuf, ADDR_DHCP, 1);//读取为1字节
}
void FlashWriteDHCP(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_DHCP, 1 );//保存为1字节
}


/*
自动模式下的 启动阈值和停止阈值的 保存
*/
void FlashReadZDMS_QDYZ(u8* _pBuf)  //启动阈值
{
	W25QXX_Read ( _pBuf, ADDR_ZDMS_QDYZ, 1);//读取为1字节
}
void FlashWriteZDMS_QDYZ(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_ZDMS_QDYZ, 1 );//保存为1字节
}

void FlashReadZDMS_TZYZ(u8* _pBuf)  //停止阈值
{
	W25QXX_Read ( _pBuf, ADDR_ZDMS_TZYZ, 1);//读取为1字节
}
void FlashWriteZDMS_TZYZ(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_ZDMS_TZYZ, 1 );//保存为1字节
}
