#include "Flowmeter.h"
#include "string.h"
#include "Rs485.h"
#include "ZoneCtrl.h"
#include "delay.h"

static u8   s_u8State = 0;
static u16 s_u16Pos = 0;
static u16 s_u16Length = 0;
static u16 g_u8Length = 0;
FlowmeterStruct FlowmeterCount; //����������
u8 send_time =0; //���ͼ��ʱ��
u8 FlowmeterID=0;//485����ID

u8 Flowmeter_RX_Buf[20]; //�����ƽ���buf
/*
ˮ�����ݶ�ȡ
*/
void Read_Flowmeter(u8 deviceID)
{
	u16 crcdata =0;
  u8 buf[8] ={0,0x03,0,0,0,0x04,0,0};
	   buf[0] =deviceID;
			crcdata = CRC16(buf,6);
			buf[6] = crcdata>>8;
			buf[7] = (u8)(crcdata&0xff);
		 comSendBuf(COM3,buf,8);
}
//��ȡEC
void Read_Flowmeter_EC(u8 deviceID)
{
	u16 crcdata =0;
  u8 buf[8] ={0,0x03,0,0,0,0x02,0,0};
	   buf[0] =deviceID;
			crcdata = CRC16(buf,6);
			buf[6] = crcdata>>8;
			buf[7] = (u8)(crcdata&0xff);
		 comSendBuf(COM3,buf,8);
}
//��ȡPH
void Read_Flowmeter_PH(u8 deviceID)
{
	u16 crcdata =0;
  u8 buf[8] ={0,0x03,0,0,0,0x01,0,0};
	   buf[0] =deviceID;
			crcdata = CRC16(buf,6);
			buf[6] = crcdata>>8;
			buf[7] = (u8)(crcdata&0xff);
		 comSendBuf(COM3,buf,8);
}
//01 03 08 12 11 32 21 00 00 00 00 
u16 Flowmeter_Data ( u8 *buffer ) //�Ӵ��ڻ�ȡ������ԭʼ����,qsize buf_len
{
	u8 u8Data = 0;
//	GR_U8 u8CountTem = 0 ,u8DataTem;
//	GR_U16 u16CRC ,u16DataLength = 0;
	u16 u16SUM_Check;

	while ( comGetRxAvailableDataSize(COM3)>0 )
	{
		switch ( s_u8State )
		{
			case 0:
				comGetChar ( COM3 , &u8Data );

				if ( ( u8Data == 0x03 ) && ( s_u16Pos == 1 ) ) //֡ͷ
				{
					buffer[s_u16Pos ++] = u8Data;
				}
				else if ( ( u8Data != 0x03 ) && ( s_u16Pos == 1) ) //֡ͷ
				{
					s_u16Pos = 0;
					continue;
				}
				else
				{
					buffer[s_u16Pos ++] = u8Data;
				}

				//s_u16Pos ++;

				if ( ( s_u16Pos == 3 ) && ( buffer[2] == 0X08 ) )//��ȡˮ��
				{
					s_u8State = 1;
				}
				else if(( s_u16Pos == 3 ) && ( buffer[2] == 0X04 )) //��ȡEC
				{
					s_u8State = 2;
				}
 				else if(( s_u16Pos == 3 ) && ( buffer[2] == 0X02 )) //��ȡEC
				{
					s_u8State = 3;
				}               
				else if ( ( s_u16Pos == 3 ) && ( buffer[2] != 0X08 )&&(buffer[2] != 0X04)&&(buffer[2] != 0X02) )
				{
					s_u16Pos = 0;
					return s_u16Pos;
				}

				break;
			case 1://�������պ������ݣ�ȫ��������
				comGetChar (COM3 , &u8Data );
				buffer[s_u16Pos++] = u8Data;

				if ( s_u16Pos > 13 )
				{
					s_u16Pos = 0;
					memset ( Flowmeter_RX_Buf , 0 , 20 );
					s_u8State = 0;
					return 0;
				}

				if ( s_u16Pos == 13 ) //������
				{
					s_u8State = 0;
//							   u16DataLength = s_u16Length;
					u16SUM_Check = CRC16 ( buffer , s_u16Pos - 2);
					s_u16Length = s_u16Pos;
					s_u16Pos = 0;

					//��֤�������У���
					if  ( u16SUM_Check == ( ( buffer[s_u16Length - 2] << 8 ) | buffer[s_u16Length - 1] ) )  //
					{
						return s_u16Length;
					}
					else//ʧ��
					{
						memset ( Flowmeter_RX_Buf , 0 , s_u16Length );
						return 0;
					}
				}

				break;
			case 2:
				comGetChar (COM3 , &u8Data );
				buffer[s_u16Pos++] = u8Data;

				if ( s_u16Pos > 9 )
				{
					s_u16Pos = 0;
					memset ( Flowmeter_RX_Buf , 0 , 20 );
					s_u8State = 0;
					return 0;
				}

				if ( s_u16Pos == 9 ) //������
				{
					s_u8State = 0;
//							   u16DataLength = s_u16Length;
					u16SUM_Check = CRC16 ( buffer , s_u16Pos - 2);
					s_u16Length = s_u16Pos;
					s_u16Pos = 0;

					//��֤�������У���
					if  ( u16SUM_Check == ( ( buffer[s_u16Length - 2] << 8 ) | buffer[s_u16Length - 1] ) )  //
					{
						return s_u16Length;
					}
					else//ʧ��
					{
						memset ( Flowmeter_RX_Buf , 0 , s_u16Length );
						return 0;
					}
				}			
				break;
			case 3:
				comGetChar (COM3 , &u8Data );
				buffer[s_u16Pos++] = u8Data;

				if ( s_u16Pos > 5 )
				{
					s_u16Pos = 0;
					memset ( Flowmeter_RX_Buf , 0 , 20 );
					s_u8State = 0;
					return 0;
				}

				if ( s_u16Pos == 5 ) //������
				{
					s_u8State = 0;
//							   u16DataLength = s_u16Length;
					u16SUM_Check = CRC16 ( buffer , s_u16Pos - 2);
					s_u16Length = s_u16Pos;
					s_u16Pos = 0;

					//��֤�������У���
					if  ( u16SUM_Check == ( ( buffer[s_u16Length - 2] << 8 ) | buffer[s_u16Length - 1] ) )  //
					{
						return s_u16Length;
					}
					else//ʧ��
					{
						memset ( Flowmeter_RX_Buf , 0 , s_u16Length );
						return 0;
					}
				}			
				break;                
			default:
				break;
		}
	}

	if ( s_u16Pos > 13 )
	{
		s_u16Pos = 0;
		s_u8State = 0;
	}

	return 0;//û���γ�������һ֡
}
//��32λBCD������ת����HEX

u32 BCDTOHEX(u8 *BCDdata)
{
	u32 hexdata =0;
	u32 hextemp =0;
	u16 data =0;
	hextemp = ((BCDdata[3]>>4)&0x0f);
	hextemp = (hextemp*1000);
	data	= (BCDdata[3]&0x0f);
	hextemp +=(data*100);
	data = ((BCDdata[4]>>4)&0x0f);
	hextemp +=(data*10);
	data = (BCDdata[4]&0x0f);
	hextemp +=data;	
	hexdata |=hextemp<<16;
	hextemp=0;
	hextemp = ((BCDdata[5]>>4)&0x0f);
	hextemp = (hextemp*1000);
	data	= (BCDdata[5]&0x0f);
	hextemp +=(data*100);
	data = ((BCDdata[6]>>4)&0x0f);
	hextemp +=(data*10);
	data = (BCDdata[6]&0x0f);
	hextemp +=data;	
	hexdata |= hextemp;
	return hexdata;
}
void  DepackFlowmeterData ( void )
{
	u8 u8SourceDataLength , u8DataLength , u8CMD;

		u8SourceDataLength = Flowmeter_Data ( Flowmeter_RX_Buf );

	if ( u8SourceDataLength != 0 )
	{
		u8CMD = Flowmeter_RX_Buf[0];
		
		switch ( u8CMD )
		{
			case 1:
					/*���������ĸ�16λΪ����,��16λΪС�����ֵ�16λ��10000���Եõ�С������*/ 
						FlowmeterCount.flower.Flowmeter1 =	BCDTOHEX(Flowmeter_RX_Buf);
				break;
			case 2:
						FlowmeterCount.flower.Flowmeter2=	BCDTOHEX(Flowmeter_RX_Buf);

				break;
			case 3:
						FlowmeterCount.flower.Flowmeter3 =	BCDTOHEX(Flowmeter_RX_Buf);
				break;
			case 4:
						FlowmeterCount.flower.Flowmeter4 =	BCDTOHEX(Flowmeter_RX_Buf);	

				break;
//			case 5:
//						FlowmeterCount.Flowmeter5 =	BCDTOHEX(Flowmeter_RX_Buf);

//				break;
//			case 6:
//						FlowmeterCount.Flowmeter6 =	   Flowmeter_RX_Buf[3]<<24;
//						FlowmeterCount.Flowmeter6 |=	Flowmeter_RX_Buf[4]<<16;
//						FlowmeterCount.Flowmeter6 |=	Flowmeter_RX_Buf[5]<<8;	
//						FlowmeterCount.Flowmeter6 |=	Flowmeter_RX_Buf[6];			
//				break;			
			default :
				break;
		}
	}
}
void Task_Flowmeter ( void * parg )
{
	//uint8 cmd_type = 0;
	//uint8 msg_type = 0;
//	uint8 control_type = 0;
	//qsize size = 0;
//	uint8 update_en = 1;
	//u8 os_err;
	
	parg = parg;

	while ( 1 ) 
	{
		OSTimeDlyHMSM(0,0,0,100);
		DepackFlowmeterData();
		if(send_time++>=20)
		{ 
			send_time =0;
			FlowmeterID++;
			if(FlowmeterID<=4) //�ɼ�4��ˮ�������
				Read_Flowmeter(FlowmeterID); //��ȡˮ������
//			else if(FlowmeterID==6)
//				Read_Flowmeter_EC(FlowmeterID);//��ȡEC����
//            else if(FlowmeterID==7)
//            {
//                Read_Flowmeter_PH(FlowmeterID);//��ȡPH����
//                FlowmeterID =0;
//            }
			else
				FlowmeterID =0;
				
		
		}
//		if(TimeUpdateCnt++>10)
//		{
//			TimerUpdate();
//			TimeUpdateCnt = 0;
//		}
//		if(WaterSta!=OldWaterSta)
//		{
//			SetTextValue(LCD_MAIN_PAGE,7,(uchar *)StaText[WaterSta]);
//			OldWaterSta = WaterSta;
//		}
//		if(FertilizerSta!=OldFertilizerSta)
//		{
//			SetTextValue(LCD_MAIN_PAGE,8,(uchar *)StaText[FertilizerSta]);
//			OldFertilizerSta = FertilizerSta;
//		}
		
	}
}
