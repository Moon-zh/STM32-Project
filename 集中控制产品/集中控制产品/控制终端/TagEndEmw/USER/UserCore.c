#include "sys.h"
#include "includes.h"  
#include "UserCore.h"
#include "Delay.h"
#include "ZoneCtrl.h"
#include "Emw3060.h"
#include "hmi_driver.h"
#include "UserHmi.h"
#include "IO_BSP.h"
#include "FlashDivide.h"
#include "rtc.h" 		
#include "rs485.h"	 
#include "delay.h"

extern u8 butonn1_state; //��ť1��ʾ/����״̬ 0���� 1��ʾ
extern u8 butonn2_state; //��ť2��ʾ/����״̬ 0���� 1��ʾ
extern u8 butonn3_state;	//��ť3��ʾ/����״̬ 0���� 1��ʾ
extern u8 butonn4_state; //��ť4��ʾ/����״̬ 0���� 1��ʾ
MsgStruct MasterMsg;
OS_EVENT * MasterQMsg;

void* 	MasterMsgBlock[4];
OS_MEM* MasterPartitionPt;
u8 g_u8AgvCtrMsgMem[20][4];

u16 Hmi_Buf[10];
	
ProgramStruct ProgramData;
MasterStruct MasterCtrl;
PercentStruct Percent ;

WaterProgStruct UserWaterProg;
u8 s_u8State; //���״̬
u8 com3state;//����3����״̬
u8 startzone;//��������
u8 closezone;//�رշ���
 u8   s_u8State = 0;
 u16 s_u16Pos = 0;
u16 s_u16Length = 0;
u8 tagendid=1;//�ն�ID
u8 com3databuf[9];
//tagend tagendstr;//�ն�ͨ�Žṹ��

//void writetagend(u8 zone,u8 state)//Ҫд�ķ���״̬
//{
// u8 date[9]={0xaa,0x55,0,0x6,0x02,0,0,0xa5,0x5a};
//    tagendstr.head_h=0xaa;
//    tagendstr.head_l=0x55;
//    tagendstr.ID=date[2]=(zone-1)/4+1;
//    tagendstr.code=date[3]=0x6;
//    tagendstr.datanum=0x02;
//    tagendstr.solenoidvalve=date[5]=zone-(zone-1)/4*4;//��Ӧ������ŷ�
//    tagendstr.solenoidvalve=date[6]=state;
//    tagendstr.tail_h=0xa5;
//    tagendstr.tail_l=0x5a; 
//}
//void readtagend(u8 zone)//Ҫ��������״̬
//{
// u8 date[9]={0xaa,0x55,0,0x6,0x02,0,0,0xa5,0x5a};
//    tagendstr.head_h=0xaa;
//    tagendstr.head_l=0x55;
//    tagendstr.ID=date[2]=(zone-1)/4+1;
//    tagendstr.code=date[3]=0x3;
//    tagendstr.datanum=0x02;
//    tagendstr.solenoidvalve=date[5]=zone-(zone-1)/4*4;//��Ӧ������ŷ�
//    tagendstr.solenoidvalve=date[6]=0;
//    tagendstr.tail_h=0xa5;
//    tagendstr.tail_l=0xa5; 
//}
u16 verifyrecviedata(u8 *buffer)
{
	u8 u8Data = 0;
//	GR_U8 u8CountTem = 0 ,u8DataTem;
//	GR_U16 u16CRC ,u16DataLength = 0;
	u8 u16SUM_Check;

	while ( comGetRxAvailableDataSize(COM3)>0 )
	{
		switch ( s_u8State )
		{
			case STATE_START:
				comGetChar ( COM3 , &u8Data );

				if ( ( u8Data == 0xaa ) && ( s_u16Pos == 0 ) ) //֡ͷ
				{
					buffer[s_u16Pos ++] = u8Data;
				}
				else if ( ( u8Data != 0xAA ) && ( s_u16Pos == 0 ) ) //֡ͷ
				{
					s_u16Pos = 0;
					continue;
				}
				else
				{
					buffer[s_u16Pos ++] = u8Data;
				}

				if(( u8Data != 0x55 ) && ( s_u16Pos == 2 ))
				{
							s_u16Pos = 0;
						return s_u16Pos;
				}
				//s_u16Pos ++;

				if ( ( s_u16Pos == 3 ) && ( buffer[2] == tagendid ) )
				{
					s_u8State = STATE_1;
				}
				else if ( ( s_u16Pos == 3 ) && ( buffer[2] != tagendid ) )
				{
					s_u16Pos = 0;
					return s_u16Pos;
				}

				break;
			case STATE_1://�������պ������ݣ�ȫ��������
				comGetChar ( COM3 , &u8Data );
				buffer[s_u16Pos++] = u8Data;

				if ( s_u16Pos > 9 )
				{
					s_u16Pos = 0;
					memset ( com3databuf , 0 , 9 );
					s_u8State = STATE_START;
					return 0;
				}

				if ( s_u16Pos == 9 ) //������
				{
					s_u8State = STATE_START;
//							   u16DataLength = s_u16Length;
					s_u16Length = s_u16Pos;
					s_u16Pos = 0;

					//��֤�������У���
					if ( ( buffer[s_u16Length - 1] == 0x5a ) && ( buffer[s_u16Length - 2]==0xa5 ) )  //
					{
						return s_u16Length;
					}
					else//ʧ��
					{
						memset ( com3databuf , 0 , s_u16Length );
						return 0;
					}
				}

				break;
			default:
				break;
		}
	}

	if ( s_u16Pos > 9 )
	{
		s_u16Pos = 0;
		s_u8State = STATE_START;
	}

	return 0;//û���γ�������һ֡
}
void  DepackCom3Data ( void )
{
	MsgStruct MsgtempBlk;
	u8 u8SourceDataLength , u8DataLength , u8CMD;
	u8SourceDataLength = verifyrecviedata ( com3databuf );

	if ( u8SourceDataLength != 0 )
	{
		u8CMD = com3databuf[3];

		switch ( u8CMD )
		{
			case 3:
			comSendBuf(COM3,com3databuf ,9); //������Ӧ
			break;
			case CMD_RETURN_REAL_TIME_DATA:
				if(com3databuf[6]==1)
				{
				  switch(com3databuf[5])
					{
						case 1:
						IO_OutSet(1,1 );//����Y1
						butonn1_state=1;
						//�ϴ���
						SetRemoteRead(0,1);
						MsgtempBlk.CmdType = INITIATIVE_HOLDING_REG;
						MsgtempBlk.CmdData[0] = 00;
						MsgtempBlk.CmdData[1] = 03;
						PackSendRemoteQ(&MsgtempBlk);
						break;
						case 2:
						IO_OutSet(2,1 );//����Y2	
						butonn2_state=1;						
						//�ϴ���
						SetRemoteRead(1,1);
						MsgtempBlk.CmdType = INITIATIVE_HOLDING_REG;
						MsgtempBlk.CmdData[0] = 00;
						MsgtempBlk.CmdData[1] = 03;
						PackSendRemoteQ(&MsgtempBlk);
						break;
						case 3:
						IO_OutSet(3,1 );//����Y3
						butonn3_state=1;						
						//�ϴ���
						SetRemoteRead(2,1);
						MsgtempBlk.CmdType = INITIATIVE_HOLDING_REG;
						MsgtempBlk.CmdData[0] = 00;
						MsgtempBlk.CmdData[1] = 03;
						PackSendRemoteQ(&MsgtempBlk);
						break;
						case 4:
						IO_OutSet(4,1 );//����Y4	
						butonn4_state=1;						
						//�ϴ���
						SetRemoteRead(3,1);
						MsgtempBlk.CmdType = INITIATIVE_HOLDING_REG;
						MsgtempBlk.CmdData[0] = 00;
						MsgtempBlk.CmdData[1] = 03;
						PackSendRemoteQ(&MsgtempBlk);
						break;
						
						default:
						break;
					
					}
				}
				else
				{
				  switch(com3databuf[5])
					{
						case 1:
						IO_OutSet(1,0 );//�ر�Y1	
						butonn1_state=0;	
						//�ϴ���
						SetRemoteRead(0,0);
						MsgtempBlk.CmdType = INITIATIVE_HOLDING_REG;
						MsgtempBlk.CmdData[0] = 00;
						MsgtempBlk.CmdData[1] = 03;
						PackSendRemoteQ(&MsgtempBlk);
						break;
						case 2:
						IO_OutSet(2,0 );//�ر�Y2
						butonn2_state=0;				
						
						//�ϴ���
						SetRemoteRead(1,0);
						MsgtempBlk.CmdType = INITIATIVE_HOLDING_REG;
						MsgtempBlk.CmdData[0] = 00;
						MsgtempBlk.CmdData[1] = 03;
						PackSendRemoteQ(&MsgtempBlk);
						break;
						case 3:
						IO_OutSet(3,0 );//�ر�Y3	
						butonn3_state=0;				
						
						//�ϴ���
						SetRemoteRead(2,0);
						MsgtempBlk.CmdType = INITIATIVE_HOLDING_REG;
						MsgtempBlk.CmdData[0] = 00;
						MsgtempBlk.CmdData[1] = 03;
						PackSendRemoteQ(&MsgtempBlk);
						break;
						case 4:
						IO_OutSet(4,0 );//�ر�Y4
						butonn4_state=0;			
						
						//�ϴ���
						SetRemoteRead(3,0);
						MsgtempBlk.CmdType = INITIATIVE_HOLDING_REG;
						MsgtempBlk.CmdData[0] = 00;
						MsgtempBlk.CmdData[1] = 03;
						PackSendRemoteQ(&MsgtempBlk);
						break;
						default:
						break;
					
					}		
				
				
				}
        comSendBuf(COM3,com3databuf ,9);
				com3state=0;//�˴�д�������
				break;
			default :
				break;
		}
	}
}
void CalculateProgPara(u8 Partition ,u8 IrrPro)
{
	u8 DataTemp[3];
	u16 WaterData[3];
	u8 DosingData[5];
	u8 AmountData;
	
	FlashReadIrrigationProg(DataTemp,IrrPro-1,3);
	FlashReadWaterProg((u8*)WaterData,DataTemp[0]-1,6);
	FlashReadDosingProg(DosingData,DataTemp[1]-1,5);
	FlashReadFertilizerAmount(&AmountData,DataTemp[2]-1,1);
	Hmi_Buf[0] = WaterData[0];
	Hmi_Buf[1] = WaterData[1];
	Hmi_Buf[2] = WaterData[2];
	Hmi_Buf[3] = 0;
	Hmi_Buf[4] = 0;
	Hmi_Buf[5] = 0;
	Hmi_Buf[6] = 0;
	Hmi_Buf[7] = 0;
	Hmi_Buf[8] = 736;
	Hmi_Buf[9] = 2;
	//WorkStateShow();
	
	SlavePara.WaterBefore = WaterData[0];
	SlavePara.WaterAfter = WaterData[2];
	SlavePara.WaterTime = WaterData[1];
	SlavePara.DosingQty[0] = DosingData[0];
	SlavePara.DosingQty[1] = DosingData[1];
	SlavePara.DosingQty[2] = DosingData[2];
	SlavePara.DosingQty[3] = DosingData[3];
	SlavePara.DosingQty[4] = DosingData[4];
	if(AmountData!=0)
		SlavePara.DosingSwitch = 1;
	SlavePara.ValveGroup = 1;
}
//������Ϣ���к��ڴ��
//���0����������
//    1����Ϣ���д���ʧ��
//	  2���ڴ�鴴��ʧ��
u8 MasterQInit(void)
{
	INT8U os_err;
	
	MasterQMsg = OSQCreate ( MasterMsgBlock, 4);
	
	if(MasterQMsg == (OS_EVENT *)0)
	{
		return 1;
	}
	
	MasterPartitionPt = OSMemCreate (
										g_u8AgvCtrMsgMem,
				  						20,
				  						4,
				  						&os_err 
				  					);
	if(os_err != OS_ERR_NONE)
	{
		return 2;
	}
	return 0;
}
//��Ϣ�������
//���룺��Ϣ��Ϣ
//�����0�����ͳɹ�
//		1�������ڴ�ʧ��
//		2��������Ϣʧ��
//�˺�����Ҫ���ж�ʹ��
u8 InterruptPackSendMasterQ(MsgStruct* MsgBlk)
{
	INT8U os_err;
	MsgStruct * MsgTemp = NULL;
	MsgTemp = (MsgStruct *)OSMemGet(MasterPartitionPt,&os_err);
	if(MsgTemp == NULL)
	{
		return 1;
	}
	MsgTemp ->CmdType = MsgBlk->CmdType;
	MsgTemp ->CmdSrc = MsgBlk->CmdSrc;
	MsgTemp ->CmdData[0] = MsgBlk->CmdData[0];
	MsgTemp ->CmdData[1] = MsgBlk->CmdData[1];
	os_err = OSQPost ( MasterQMsg,(void*)MsgTemp );
	//������Ϣʧ���ͷ��ڴ�
	if(os_err!=OS_ERR_NONE)
	{
		OSMemPut(MasterPartitionPt, ( void * )MsgTemp);
		return 2;
	}
	return 0;
}
//��Ϣ�������
//���룺��Ϣ��Ϣ
//�����0�����ͳɹ�
//		1�������ڴ�ʧ��
//		2��������Ϣʧ��

u8 PackSendMasterQ(MsgStruct* MsgBlk)
{
	INT8U os_err;
	MsgStruct * MsgTemp = NULL;
	MsgTemp = (MsgStruct *)OSMemGet(MasterPartitionPt,&os_err);
	if(MsgTemp == NULL)
	{
		return 1;
	}
	MsgTemp ->CmdType = MsgBlk->CmdType;
	MsgTemp ->CmdSrc = MsgBlk->CmdSrc;
	MsgTemp ->CmdData[0] = MsgBlk->CmdData[0];
	MsgTemp ->CmdData[1] = MsgBlk->CmdData[1];
	os_err = OSQPost ( MasterQMsg,(void*)MsgTemp );
	//������Ϣʧ���ͷ��ڴ�
	if(os_err!=OS_ERR_NONE)
	{
		OSMemPut(MasterPartitionPt, ( void * )MsgTemp);
		return 2;
	}
	return 0;
}
//������Ϣ��������Ϣ�ڴ��ͷ�
//���룺�յ�����Ϣ����ָ��
//�����0���ͷųɹ�
//		1���ͷ�ʧ��
u8 DepackReceiveMasterQ(MsgStruct * MasterQ)
{
	u8 os_err;
	os_err = OSMemPut(MasterPartitionPt, ( void * )MasterQ);
	if(os_err != OS_ERR_NONE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
u8 alfred_test = 0;
void TestProg(u8 Test_Type)
{
	MsgStruct MsgtempBlk;
	switch(Test_Type)
	{
		case 1:
			MsgtempBlk.CmdType = MSG_START;
			PackSendMasterQ(&MsgtempBlk);
			break;
		case 2:
			break;
	}
}
u8 Master_State;
void MasterCtrl_task(void *pdata)
{
	u8 os_err;
	pdata=pdata;
	FlashReadID(&tagendid);
//	MsgStruct * pMsgBlk = NULL;
//	MsgStruct Msgtemp;
//	MsgStruct MsgtempBlk;
//	//OS_CPU_SR  cpu_sr;
//	MasterQInit();
	while(1)
	{
		delay_ms(200);

		DepackCom3Data ();		


	}
}

