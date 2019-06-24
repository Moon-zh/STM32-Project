#include "W25Qxx.h"
#include "Includes.h"
#include "FlashDivide.h"

/********************************************************************
//����ˮ����
*********************************************************************/
void FlashReadWaterProg(u8* _pBuf,u8 addr,u8 Num)
{
	W25QXX_Read ( _pBuf, ADDR_WARTERPROG+addr*6, Num );//����ʱ��Ϊ2�ֽ�
}
/********************************************************************
//д��ˮ����
*********************************************************************/
void FlashWriteWaterProg(u8* _pBuf,u8 Num)
{
	W25QXX_Write ( _pBuf, ADDR_WARTERPROG, Num);
}
/********************************************************************
//����ʳ���
*********************************************************************/
void FlashReadDosingProg(u8* _pBuf,u8 addr,u8 Num)
{
	W25QXX_Read ( _pBuf, ADDR_DOSINGPROG+addr*5, Num );//����ʱ��Ϊ2�ֽ�
}
/********************************************************************
//д��ʳ���
*********************************************************************/
void FlashWriteDosingProg(u8* _pBuf,u8 Num)
{
	W25QXX_Write ( _pBuf, ADDR_DOSINGPROG, Num );
}
/********************************************************************
//����ȳ���
*********************************************************************/
void FlashReadIrrigationProg(u8* _pBuf,u8 addr,u8 Num)
{
	W25QXX_Read ( _pBuf, ADDR_IRRIGATIONPROG+addr*3, Num );//����ʱ��Ϊ2�ֽ�
}
/********************************************************************
//д��ȳ���
*********************************************************************/
void FlashWriteIrrigationProg(u8* _pBuf,u8 Num)
{
	W25QXX_Write ( _pBuf, ADDR_IRRIGATIONPROG, Num );
}
/********************************************************************
//���÷���
*********************************************************************/
void FlashReadFertilizerAmount(u8* _pBuf,u8 addr,u8 Num)
{
	W25QXX_Read ( _pBuf, ADDR_FERTILIZERAMOUNT+addr, Num );//����ʱ��Ϊ2�ֽ�
}
/********************************************************************
//д�÷���
*********************************************************************/
void FlashWriteFertilizerAmount(u8* _pBuf,u8 Num)
{
	W25QXX_Write ( _pBuf, ADDR_FERTILIZERAMOUNT, Num );
}
/********************************************************************
//���̶�����
*********************************************************************/
void FlashReadFixedPara(u8* _pBuf,u8 addr,u8 Num)
{
	W25QXX_Read ( _pBuf, ADDR_FIXEDPARA+addr, Num );//����ʱ��Ϊ2�ֽ�
}
/********************************************************************
//д�̶�����
*********************************************************************/
void FlashWriteFixedPara(u8* _pBuf,u8 Num)
{
	W25QXX_Write ( _pBuf, ADDR_FIXEDPARA, Num );
}
/********************************************************************
//������
*********************************************************************/
void FlashReadPara(u8* _pBuf,u8 addr,u8 Num)
{
	W25QXX_Read ( _pBuf, ADDR_PARA, Num*2 );//����ʱ��Ϊ2�ֽ�
}
/********************************************************************
//д����
*********************************************************************/
void FlashWritePara(u8* _pBuf,u8 Num)
{
	W25QXX_Write ( _pBuf, ADDR_PARA, Num*2 );
}

/********************************************************************
//�������¼
*********************************************************************/
void FlashReadtaskrecord(u8* _pBuf,u32 addr,u16 Num)
{
	W25QXX_Read ( _pBuf, addr, Num );//����ʱ��Ϊ2�ֽ�
}
/********************************************************************
//���������¼
*********************************************************************/
void FlashWritetaskrecord(u8* _pBuf,u32 addr,u16 Num)
{
	W25QXX_Write ( _pBuf, addr, Num );
}
//�������¼
/*********************************************************************/
void FlashReadtargetecord(u8* _pBuf,u16 Num)
{
	W25QXX_Read ( _pBuf, ADDR_UNCOMPLET, Num );//����ʱ��Ϊ2�ֽ�
}
/********************************************************************
//���������¼
*********************************************************************/
void FlashWritetargetrecord(u8* _pBuf,u16 Num)
{
	W25QXX_Write ( _pBuf, ADDR_UNCOMPLET, Num );
}
void FlashReadWiFi(u8* _pBuf)
{
	W25QXX_Read ( _pBuf, ADDR_PARASET, 126 );//����ʱ��Ϊ2�ֽ�
}
void FlashWriteWiFi(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_PARASET, 126);
}
void FlashReadDHCP(u8* _pBuf)
{
	W25QXX_Read ( _pBuf, ADDR_DHCPSET, 1 );//����ʱ��Ϊ2�ֽ�
}
void FlashWriteDHCP(u8* _pBuf)
{
	W25QXX_Write ( _pBuf, ADDR_DHCPSET, 1);
}


