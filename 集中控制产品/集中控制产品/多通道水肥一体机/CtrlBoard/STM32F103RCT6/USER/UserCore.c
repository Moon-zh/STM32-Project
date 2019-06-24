#include "sys.h"
#include "includes.h"  
#include "UserCore.h"
#include "Delay.h"
#include "ZoneCtrl.h"
#include "GM3Dir.h"
#include "hmi_driver.h"
#include "UserHmi.h"
#include "IO_BSP.h"
#include "FlashDivide.h"
#include "Flowmeter.h"
MsgStruct MasterMsg;
OS_EVENT * MasterQMsg;

void* 	MasterMsgBlock[4];
OS_MEM* MasterPartitionPt;
//u8 g_u8AgvCtrMsgMem[5][4];
u8 g_u8AgvCtrMsgMem[20][4];
MOCHINEStruct  MOCHINEDATE;//״̬��ִ����������
u16 WarterRemainderTime = 0;//��ˮʣ�๤��ʱ��
u16 FertilizerRemainderTime = 0;//ʩ��ʣ�๤��ʱ��

u8 ControlMode = 0;//ˮ��һ�������ģʽ��1������ 0:����
u8 SendIoCount=0;//��IO����ָ�������ʱ
//u8 MasterState = 0;//��״̬��
u8 WarterState = 0;//��ˮ״̬
u8 FertilizerState = 0;//��ˮ״̬
u8 Machine_Ctrl_State =0; 	//���ܹ��״̬��
StrategyStruct WorkStrategy;
extern FlowmeterStruct FlowmeterCount; //����������
extern HMIStruct UserHmiDate;
//дִ�в���
//���룺Ҫ���õ�ֵ
//���أ�0�����óɹ� 1�����ù��ܱ�ռ�� 2��FLASHдʧ��
//ʹ��ʱ��ֹ�ж�
u8 SetStrategy(StrategyStruct *Strategy)
{
//	static u8 Flag = 0;
//	if(Flag == 1)
//		return 1 ;
//	Flag = 1;
//	WorkStrategy.PumpWFlag = Strategy->PumpWFlag;
//	WorkStrategy.PumpFFlag = Strategy->PumpFFlag;
//	WorkStrategy.Passageway[0] = Strategy->Passageway[0];
//	WorkStrategy.Passageway[1] = Strategy->Passageway[1];
//	WorkStrategy.Passageway[2] = Strategy->Passageway[2];
//	WorkStrategy.Passageway[3] = Strategy->Passageway[3];
//	WorkStrategy.Passageway[4] = Strategy->Passageway[4];
//	WorkStrategy.Passageway[5] = Strategy->Passageway[5];
//	WorkStrategy.Zone = Strategy->Zone;
//	WorkStrategy.WorkHour = Strategy->WorkHour;
//	WorkStrategy.WorkMinute = Strategy->WorkMinute;
//	WorkStrategy.WorkDay = Strategy->WorkDay;
//	WorkStrategy.StartHour = Strategy->StartHour;
//	WorkStrategy.StartMinute = Strategy->StartMinute;
//	//д��Flash
//	Flag = 0;
//	return 0;
}
//���Կ�ʼ
//���룺���Բ���
//�������
// alfred ��ʣMODBUS��Ȧû��д ˮ�úͷʱõĿ���û��д
void StrategyStart(StrategyStruct *Strategy)
{
//	u8 ConduitF = 0;
//	u8 i = 0;
//	
//	//�򿪷�����ŷ�
//	ConduitF = Strategy->Zone;
//	ZoneCtrl_Set(ConduitF,YSETSINGLE);
//	SetMDCoil(ConduitF + 2 - 1 , 1);
//	delay_ms(110);
//	
//	if(Strategy->PumpWFlag == 1)
//	{
//		//��ˮ��
//		SetMDCoil(0 , 1);
//	}
//	else if(Strategy->PumpWFlag == 0)
//	{
//		//������ߴ�ˮ��
//	}
//	if(Strategy->PumpFFlag == 1)
//	{
//		//�򿪷ʱ�
//		SetMDCoil(1 , 1);
//	}
//	else if(Strategy->PumpFFlag == 0)
//	{
//		//�رշʱ�
//		SetMDCoil(1 , 0);
//	}
//	//��ͨ����ŷ� 
//	for(i = 0;i<6; i++)
//	{
//		if(Strategy->Passageway[i] == 1)
//		{
//			ConduitF |= i<<i;
//			SetMDCoil(8+i , 1);
//		}
//	}
//	ZoneCtrl_Set(ConduitF,YSETMUL);
//}
////����ֹͣ
////���룺���Բ���
////�������
//// alfred ��ʣMODBUS��Ȧû��д ˮ�úͷʱõĿ���û��д
//void StrategyStop(StrategyStruct *Strategy)
//{
//	u8 ConduitF = 0;
//	u8 i = 0;
//	
//	//�ر�ˮ��
//	SetMDCoil(0 , 0);
//	//�رշʱ�
//	SetMDCoil(1 , 0);
//	//�ر�ͨ����ŷ� 
//	for(i = 0;i<6; i++)
//	{
//		SetMDCoil(8+i , 0);
//	}
//	ZoneCtrl_Set(0,YCLEAR);
//	delay_ms(110);
//	//�رշ�����ŷ�
//	ConduitF = Strategy->Zone;
//	ZoneCtrl_Set(ConduitF,YCLEAR);
//	SetMDCoil(ConduitF + 2 - 1  , 0);
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
u8 ZoneAlarm[2];
//���Ҫ������ʩ�ʵ�ŷ�
u8 Check_FER_ON(u8 IOnumber)
{
  if((IOnumber&0x01)==0x01)	//����һ��ʩ�ʷ��Ƿ���Ҫ����
  {
		return 1;
	}		
  else if((IOnumber&0x02)==0x02)	//���ڶ���ʩ�ʷ��Ƿ���Ҫ����
  {
		return 2;
	}	
  else if((IOnumber&0x04)==0x04)	//��������ʩ�ʷ��Ƿ���Ҫ����
  {
		return 3;
	}	
	  else if((IOnumber&0x068)==0x08)	//�����ĸ�ʩ�ʷ��Ƿ���Ҫ����
  {
		return 4;
	}	
	  else if((IOnumber&0x10)==0x10)	//�������ʩ�ʷ��Ƿ���Ҫ����
  {
		return 5;
	}	
  else
	 return 0;

}
////���Ҫ��Ӧ�������Ƿ��б任
//u8 Check_FLOW_ON(u8 IOnumber)
//{
//  switch(IOnumber)
//	{
//		case 1:
//			if(FlowmeterCount.Flowmeter1>MOCHINEDATE.lastFlowmeter1)
//			{
//			  return 1;
//			}
//		break;
//	
//		case 2:
//			if(FlowmeterCount.Flowmeter2>MOCHINEDATE.lastFlowmeter2)
//			{
//			  return 1;
//			}			
//		break;
//		case 3:
//			if(FlowmeterCount.Flowmeter3>MOCHINEDATE.lastFlowmeter3)
//			{
//			  return 1;
//			}					
//		break;
//		case 4:
//			if(FlowmeterCount.Flowmeter4>MOCHINEDATE.lastFlowmeter4)
//			{
//			  return 1;
//			}					
//		break;
//		case 5:
//			if(FlowmeterCount.Flowmeter5>MOCHINEDATE.lastFlowmeter5)
//			{
//			  return 1;
//			}					
//		break;
//		default:
//			
//		break;		
//	
//	
//	
//	
//	}
// return 0;
//}

//������ˮ��ŷ� ֱ���ɹ�
void Water_on(void)
{
	u8 os_err;
//	u8 TestTemp[4];
//	u8 sendcount=0;
		MsgStruct Msgtemp;
	OS_CPU_SR  cpu_sr;
	SendIoCount=0;
	while(MOCHINEDATE.Io_on_Off_Sucess)
	{
		delay_ms(500);											//������ϢƵ��Ҫ���ڼ��Ƶ�� ��ֹ�󷢼��  ���ͺ�������Ҫ������Ӧ
		 if(SendIoCount>IO_SEND_COUNT_MAX) //�����������Ǹ�������Ҫ��ӵ���������
		 {
			 			 MOCHINEDATE.Io_Step=0;
			MOCHINEDATE.Io_on_Off_Sucess =0;	//����ѭ��
			MOCHINEDATE.u16MOCHINEerrorstate =MOCH_E_STATE_WATER_ON; //������ˮʧ��    
			 Machine_Ctrl_State =MOCHINE_ALARM;  //״̬���л�������״̬
			Msgtemp.CmdType = MSG_ALARM;								//����������Ϣ
	//								Msgtemp.CmdSrc = 0;
			Msgtemp.CmdData[0] = 2; //��ʾ����ִ�е��ڼ���
			OS_ENTER_CRITICAL();
			PackSendMasterQ(&Msgtemp);
			OS_EXIT_CRITICAL();	
		 }
		switch(MOCHINEDATE.Io_Step)		
		{
			case 0:										//дIOָ��
				  if(SendIoCount++<=IO_SEND_COUNT_MAX)			
						IO_SET_DATA(MOCHINEDATE.WaterOnID,0,0xff);
					
			break;
			case 1:
					if(SendIoCount++<=IO_SEND_COUNT_MAX)
					IO_READ_Input_Data(MOCHINEDATE.WaterOnID);	
			break;
			case 2:
					
					SendIoCount=0;
				  MOCHINEDATE.WaterOnSucess =1; //��ˮ��ŷ������ɹ�
					MOCHINEDATE.Io_Step =0;
					MOCHINEDATE.Io_on_Off_Sucess =0;
					SetTexFrontBColor(LCD_ACTION_PAGE,6,GREEN_COLOR_HMI);//�����ı�����
					SetTextValue(LCD_ACTION_PAGE,6,"����");
			
			    MOCHINEDATE.FertilizerIoStatrNum = Check_FER_ON(MOCHINEDATE.FertilizerIoNumber);	//������ڿ����ڼ�·ʩ�ʷ�

				if(MOCHINEDATE.FertilizerIoStatrNum!=0)
				{
					Msgtemp.CmdType = MSG_ACTION_ERTILIZER_ON;								//ʩ�ʵ�ŷ�����
		//								Msgtemp.CmdSrc = 0;
//				Msgtemp.CmdData[0] = 1; //��ʾ����ִ�е��ڼ���
				OS_ENTER_CRITICAL();
				PackSendMasterQ(&Msgtemp);
				OS_EXIT_CRITICAL();	
				}
			break;			
			default :
			break;
		
		}
	  
	
	}


}
void Water_off(void)
{
	u8 os_err;
//	u8 TestTemp[4];
//	u8 sendcount=0;
		MsgStruct Msgtemp;
	OS_CPU_SR  cpu_sr;
	SendIoCount=0;
	while(MOCHINEDATE.Io_on_Off_Sucess)
	{
		delay_ms(500);											//������ϢƵ��Ҫ���ڼ��Ƶ�� ��ֹ�󷢼��  ���ͺ�������Ҫ������Ӧ
		 if(SendIoCount>IO_SEND_COUNT_MAX) //�����������Ǹ�������Ҫ��ӵ���������
		 {
			MOCHINEDATE.Io_on_Off_Sucess =0;	//����ѭ��
			MOCHINEDATE.u16MOCHINEerrorstate =MOCH_E_STATE_WATER_ON; //������ˮʧ��    
			 Machine_Ctrl_State =MOCHINE_ALARM;  //״̬���л�������״̬
			Msgtemp.CmdType = MSG_ALARM;								//����������Ϣ
	//								Msgtemp.CmdSrc = 0;
			Msgtemp.CmdData[0] = 1; //��ʾ����ִ�е��ڼ���
			OS_ENTER_CRITICAL();
			PackSendMasterQ(&Msgtemp);
			OS_EXIT_CRITICAL();	
		 }
		switch(MOCHINEDATE.Io_Step)		
		{
			case 0:										//дIOָ��
				  if(SendIoCount++<=IO_SEND_COUNT_MAX)			
						IO_SET_DATA(MOCHINEDATE.WaterOnID,0,0);
					
			break;
			case 1:
					if(SendIoCount++<=IO_SEND_COUNT_MAX)
					IO_READ_Input_Data(MOCHINEDATE.WaterOnID);	
			break;
			case 2:
					
					SendIoCount=0;
					MOCHINEDATE.Io_Step=0;
					MOCHINEDATE.Io_on_Off_Sucess =0;
					Machine_Ctrl_State =MOCHINE_STOP;  //״̬���л���ֹͣ(�������)
					Msgtemp.CmdType = MSG_STOP;								//ʩ�ʵ�ŷ�����
		//								Msgtemp.CmdSrc = 0;
//				Msgtemp.CmdData[0] = 1; //��ʾ����ִ�е��ڼ���
				OS_ENTER_CRITICAL();
				PackSendMasterQ(&Msgtemp);
				OS_EXIT_CRITICAL();	
			
			break;			
			default :
			break;
		
		}
	  
	
	}


}
//����ʩ�ʵ�ŷ� ֱ���ɹ�
void Fertilizer_on(void)
{
	u8 os_err;
//	u8 TestTemp[4];
//	u8 sendcount=0;
	  u8 i;
		MsgStruct Msgtemp;
	OS_CPU_SR  cpu_sr;
	SendIoCount=0;
	while(MOCHINEDATE.Io_on_Off_Sucess)
	{
		delay_ms(500);											//������ϢƵ��Ҫ���ڼ��Ƶ�� ��ֹ�󷢼��  ���ͺ�������Ҫ������Ӧ
		 if(SendIoCount>IO_SEND_COUNT_MAX) //�����������Ǹ�������Ҫ��ӵ���������
		 {
			 MOCHINEDATE.Io_Step=0;
			MOCHINEDATE.Io_on_Off_Sucess =0;	//����ѭ��
			MOCHINEDATE.u16MOCHINEerrorstate =MOCH_E_STATE_FERT_ON; //����ʩ��ʧ��    
			 Machine_Ctrl_State =MOCHINE_ALARM;  //״̬���л�������״̬
			Msgtemp.CmdType = MSG_ALARM;								//����������Ϣ
	//								Msgtemp.CmdSrc = 0;
			Msgtemp.CmdData[0] = 3; //��ʾ����ִ�е��ڼ���
			OS_ENTER_CRITICAL();
			PackSendMasterQ(&Msgtemp);
			OS_EXIT_CRITICAL();	
		 }
		switch(MOCHINEDATE.Io_Step)		
		{
			case 0:										//дIOָ��
				  if(SendIoCount++<=IO_SEND_COUNT_MAX)			
						IO_SET_DATA(MOCHINEDATE.FertilizerOnID,(MOCHINEDATE.FertilizerIoStatrNum-1),0xff);
					
			break;
			case 1:
					if(SendIoCount++<=IO_SEND_COUNT_MAX)
					IO_READ_Input_Data(MOCHINEDATE.FertilizerOnID);	
			break;
			case 2:
						SendIoCount=0;
						IO_OutSet(1, 1);  //�������ʵ��
						delay_ms(50);
			//Ϊ�˲��Գɹ���ʱ����Ҫ�����˲���
//					 if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)==0) //�����ɹ�
//					 {
					 
							MOCHINEDATE.Io_Step =3;//�������������ݼ��		 
//					 }
//					 else																//����ʧ��
//					 {
//					 			 MOCHINEDATE.Io_Step=0;
//							MOCHINEDATE.Io_on_Off_Sucess =0;	//����ѭ��
//							MOCHINEDATE.u16MOCHINEerrorstate =MOCH_E_STATE_FERT_ON; //����ʩ��ʧ��    
//							 Machine_Ctrl_State =MOCHINE_ALARM;  //״̬���л�������״̬
//							Msgtemp.CmdType = MSG_ALARM;								//����������Ϣ
//					//								Msgtemp.CmdSrc = 0;
//							Msgtemp.CmdData[0] = 3; //��ʾ����ִ�е��ڼ���
//							OS_ENTER_CRITICAL();
//							PackSendMasterQ(&Msgtemp);
//							OS_EXIT_CRITICAL();						 
//					 
//					 
//					 }
				

			break;	
		  case 3:
				 if(SendIoCount++<=IO_SEND_COUNT_MAX) //5S��������Ӧ�������б仯 ����ʱ����Ҫ����
				 {
//						if(Check_FLOW_ON(MOCHINEDATE.FertilizerIoStatrNum))
//						{
//						
//							SendIoCount=0;
//							MOCHINEDATE.FertilizerIoSucess =1; //ʩ�ʵ�ŷ������ɹ�
//							MOCHINEDATE.Io_Step =0;
//							MOCHINEDATE.Io_on_Off_Sucess =0;
//							switch(MOCHINEDATE.FertilizerIoStatrNum)
//						  {
//								case 1:
//									MOCHINEDATE.FertilizerCurrenttime = UserHmiDate.fertilizer_Solenoid1_time;
//									break;
//								case 2:
//									MOCHINEDATE.FertilizerCurrenttime = UserHmiDate.fertilizer_Solenoid2_time;
//									break;
//								case 3:
//									MOCHINEDATE.FertilizerCurrenttime = UserHmiDate.fertilizer_Solenoid3_time;
//									break;
//								case 4:
//									MOCHINEDATE.FertilizerCurrenttime = UserHmiDate.fertilizer_Solenoid4_time;
//									break;
//								case 5:
//									MOCHINEDATE.FertilizerCurrenttime = UserHmiDate.fertilizer_Solenoid5_time;
//									break;
//								default:
//									break;
//								
//								
//								
//							
//							}
//							MOCHINEDATE.FertilizerCurrenttimeing =0;
//							SetTexFrontBColor(LCD_ACTION_PAGE,7,GREEN_COLOR_HMI);//�����ı�����
//							SetTextValue(LCD_ACTION_PAGE,7,"����");	
//							for(i=1;i<6;i++)
//							{
//							 if(i==MOCHINEDATE.FertilizerIoStatrNum)	//�����ı��ؼ�
//							 {
//									SetTexFrontBColor(LCD_ACTION_PAGE,i,GREEN_COLOR_HMI);//�����ı�����
//									SetTextValue(LCD_ACTION_PAGE,i,"����");
//							 
//							 }
//							 else
//							 {
//									SetTexFrontBColor(LCD_ACTION_PAGE,i,RED_COLOR_HMI);//�����ı�����
//									SetTextValue(LCD_ACTION_PAGE,i,"�ر�");							 
//							 
//							 
//							 }
//							
//							}	
//									
//						}
				 
				 
				 }
				 else																//����ʧ��
				 {
						MOCHINEDATE.Io_Step=0;
						MOCHINEDATE.Io_on_Off_Sucess =0;	//����ѭ��
						MOCHINEDATE.u16MOCHINEerrorstate =MOCH_E_STATE_FERT_ON; //����ʩ��ʧ��    
						 Machine_Ctrl_State =MOCHINE_ALARM;  //״̬���л�������״̬
						Msgtemp.CmdType = MSG_ALARM;								//����������Ϣ
				//								Msgtemp.CmdSrc = 0;
						Msgtemp.CmdData[0] = 2; //��ʾ����ִ�е��ڼ���
						OS_ENTER_CRITICAL();
						PackSendMasterQ(&Msgtemp);
						OS_EXIT_CRITICAL();						 
				 
				 
				 }				 
			 break;
			default :
			break;
		
		}
	  
	
	}


}
//�ر�ʩ�ʵ�ŷ� ֱ���ɹ�
void Fertilizer_off(void)
{
	u8 os_err;
//	u8 TestTemp[4];
//	u8 sendcount=0;
	  u8 i;
		MsgStruct Msgtemp;
	OS_CPU_SR  cpu_sr;
	SendIoCount=0;
	while(MOCHINEDATE.Io_on_Off_Sucess)
	{
		delay_ms(500);											//������ϢƵ��Ҫ���ڼ��Ƶ�� ��ֹ�󷢼��  ���ͺ�������Ҫ������Ӧ
		 if(SendIoCount>IO_SEND_COUNT_MAX) //�����������Ǹ�������Ҫ��ӵ���������
		 {
			 			 MOCHINEDATE.Io_Step=0;
			MOCHINEDATE.Io_on_Off_Sucess =0;	//����ѭ��
			MOCHINEDATE.u16MOCHINEerrorstate =MOCH_E_STATE_FERT_ON; //����ʩ��ʧ��    
			 Machine_Ctrl_State =MOCHINE_ALARM;  //״̬���л�������״̬
			Msgtemp.CmdType = MSG_ALARM;								//����������Ϣ
	//								Msgtemp.CmdSrc = 0;
			Msgtemp.CmdData[0] = 3; //��ʾ����ִ�е��ڼ���
			OS_ENTER_CRITICAL();
			PackSendMasterQ(&Msgtemp);
			OS_EXIT_CRITICAL();	
		 }
		switch(MOCHINEDATE.Io_Step)		
		{
			case 0:										//дIOָ��
				  if(SendIoCount++<=IO_SEND_COUNT_MAX)			
						IO_SET_DATA(MOCHINEDATE.FertilizerOnID,(MOCHINEDATE.FertilizerIoStatrNum-1),0);
					
			break;
			case 1:
					if(SendIoCount++<=IO_SEND_COUNT_MAX)
					IO_READ_Input_Data(MOCHINEDATE.FertilizerOnID);	
			break;
			case 2:
					SendIoCount=0;
			  	MOCHINEDATE.Io_Step=0;
					MOCHINEDATE.FertilizerCurrenttimeing =0;
					IO_OutSet(1, 0);  //�ر����ʵ��	
					SetTexFrontBColor(LCD_ACTION_PAGE,7,RED_COLOR_HMI);//�����ı�����
					SetTextValue(LCD_ACTION_PAGE,7,"�ر�");					
					for(i=1;i<6;i++)
					{
							SetTexFrontBColor(LCD_ACTION_PAGE,i,RED_COLOR_HMI);//�����ı�����
							SetTextValue(LCD_ACTION_PAGE,i,"�ر�");							 
			
					}			
//					MOCHINEDATE.Io_Step =0;
					MOCHINEDATE.Io_on_Off_Sucess =0;
			    MOCHINEDATE.FertilizerIoStatrNum = Check_FER_ON(MOCHINEDATE.FertilizerIoNumber);	//������ڿ����ڼ�·ʩ�ʷ�
				if(MOCHINEDATE.FertilizerIoStatrNum!=0)
				{
					Msgtemp.CmdType = MSG_ACTION_ERTILIZER_ON;								//ʩ�ʵ�ŷ�����
		//								Msgtemp.CmdSrc = 0;
//				Msgtemp.CmdData[0] = 1; //��ʾ����ִ�е��ڼ���
				OS_ENTER_CRITICAL();
				PackSendMasterQ(&Msgtemp);
				OS_EXIT_CRITICAL();	
				}
		
			break;	

			default :
			break;
		
		}
	  
	
	}
}
//����ʧ�ܹر����е���
void Task_error(void)
{
 	u8 os_err;
//	u8 TestTemp[4];
//	u8 sendcount=0;
	  u8 i;
		MsgStruct Msgtemp;
	OS_CPU_SR  cpu_sr;
	SendIoCount=0;
	while(MOCHINEDATE.Io_on_Off_Sucess)
	{
		delay_ms(500);
		switch(MOCHINEDATE.Io_Step)
		{
			case 0:					//�ر�ʩ�ʵ�ŷ�
				if(SendIoCount++<3)
						IO_RESETALL_DATA(MOCHINEDATE.FertilizerOnID);
				else
					MOCHINEDATE.Io_Step=1;
				break;
			case 1:				//�ر�ʩ�ʵ��
							IO_OutSet(1, 0);  //�ر����ʵ��		
					MOCHINEDATE.Io_Step=2;
				break;
			case 2:
				if(SendIoCount++<3) //�رս�ˮ��ŷ�
						IO_RESETALL_DATA(MOCHINEDATE.WaterOnID);
				else
					MOCHINEDATE.Io_Step=3;	
				break;			
			case 3:
					MOCHINEDATE.Io_Step=0;	
					MOCHINEDATE.Io_on_Off_Sucess=0;
			break;	
			default:
			break;
		}
		
	}


}
void SetZoneAlarm(u8 AlarmNum, u8 Flag)
{
	if(Flag == 1)
	{
		if(AlarmNum == 1)
		{
			ZoneAlarm[0] |= 0x0f; 
		}
		else if(AlarmNum == 2)
		{
			ZoneAlarm[0] |= 0xf0; 
		}
		else if(AlarmNum == 3)
		{
			ZoneAlarm[1] |= 0x0f; 
		}
		else if(AlarmNum == 4)
		{
			ZoneAlarm[1] |= 0xf0; 
		}
	}
	else if(Flag == 0)
	{
		if(AlarmNum == 1)
		{
			ZoneAlarm[0] &= 0xf0; 
		}
		else if(AlarmNum == 2)
		{
			ZoneAlarm[0] |= 0x0f; 
		}
		else if(AlarmNum == 3)
		{
			ZoneAlarm[1] |= 0xf0; 
		}
		else if(AlarmNum == 4)
		{
			ZoneAlarm[1] |= 0x0f; 
		}
	}
	
}
void MasterCtrl_task(void *pdata)
{
	u8 os_err;
	MsgStruct * pMsgBlk = NULL;
	MsgStruct Msgtemp;
//	u8 TestTemp[4];
	OS_CPU_SR  cpu_sr;
	InitativeStruct InitiativeParaTemp;
	MasterQInit();
	while(1)
	{
		delay_ms(50);
		pMsgBlk = ( MsgStruct *) OSQPend ( MasterQMsg,
				10,
				&os_err );
		
		if(os_err == OS_ERR_NONE)
		{
			memcpy((u8*)&Msgtemp,(u8*)pMsgBlk,sizeof(MsgStruct) );
			DepackReceiveMasterQ(pMsgBlk);
			switch (Machine_Ctrl_State)
			{
				case MOCHINE_IDLE:
					if(Msgtemp.CmdType == MSG_START)
					{
						if(MOCHINEDATE.WaterTime>=MOCHINEDATE.Fertilizertime) //��ˮʱ�����ʩ��ʱ��
						{
							if((MOCHINEDATE.FertilizerIoNumber!=0)&&(MOCHINEDATE.WaterOnID!=0)) //Ĭ�Ͻ�ˮ��ʩ����Ҫͬʱ��������Ҫֻ��ˮ���Ҳ���Կ��޸Ĵ�����
							{
								Refresh_ACTION_Page();
								SetScreen(1);
//								MOCHINEDATE.lastFlowmeter1 =FlowmeterCount.Flowmeter1;//��Ҫ�ϴ�������
//								MOCHINEDATE.lastFlowmeter2 =FlowmeterCount.Flowmeter2;//��Ҫ�ϴ�������
//								MOCHINEDATE.lastFlowmeter3 =FlowmeterCount.Flowmeter3;//��Ҫ�ϴ�������
//								MOCHINEDATE.lastFlowmeter4 =FlowmeterCount.Flowmeter4;//��Ҫ�ϴ�������
//								MOCHINEDATE.lastFlowmeter5 =FlowmeterCount.Flowmeter5;//��Ҫ�ϴ�������								
                                Machine_Ctrl_State = MOCHINE_ACTION; //����ʼִ��
								Msgtemp.CmdType = MSG_ACTION_WATER_ON;
//								Msgtemp.CmdSrc = 0;
//								Msgtemp.CmdData[0] = MOCHINEDATE.WaterOnID; //Ҫ������ˮ��ŷ���ID
								OS_ENTER_CRITICAL();
								PackSendMasterQ(&Msgtemp);
								OS_EXIT_CRITICAL();	
							}
							else  //�澯
                            {
								SetScreen(2);
                                MOCHINEDATE.u16MOCHINEerrorstate =MOCH_E_STATE_START_ON;
                                Machine_Ctrl_State = MOCHINE_ALARM; //����ʼִ��
								Msgtemp.CmdType = MSG_ALARM;

//								Msgtemp.CmdSrc = 0;
								Msgtemp.CmdData[0] = 1; //��1���ʹ����˳�ʼ���ṹ��Ϳ�����
								OS_ENTER_CRITICAL();
								PackSendMasterQ(&Msgtemp);
								OS_EXIT_CRITICAL();									
							 }
						}
						else  //�澯
                        {
							SetScreen(2);
							  Machine_Ctrl_State = MOCHINE_ALARM; //����ʼִ��
								Msgtemp.CmdType = MSG_ALARM;
							  MOCHINEDATE.u16MOCHINEerrorstate =MOCH_E_STATE_START_ON;
//								Msgtemp.CmdSrc = 0;
								Msgtemp.CmdData[0] = 1; //��1���ʹ����˳�ʼ���ṹ��Ϳ�����
								OS_ENTER_CRITICAL();
								PackSendMasterQ(&Msgtemp);
								OS_EXIT_CRITICAL();							
						 }
					}
			
				break;
				case MOCHINE_ACTION:
					if(Msgtemp.CmdType == MSG_ACTION_WATER_ON) //������ˮ��ŷ�
					{
						MOCHINEDATE.Io_on_Off_Sucess =1;
//						MOCHINEDATE.Io_Step=0;
							Water_on();					
					}
					if(Msgtemp.CmdType == MSG_ACTION_WATER_OFF) //�رչ�ˮ��ŷ�
					{
						MOCHINEDATE.Io_on_Off_Sucess =1;
//						MOCHINEDATE.Io_Step=0;
							Water_off();					
					}
					if(Msgtemp.CmdType == MSG_ACTION_ERTILIZER_ON) //����ʩ�ʵ�ŷ�
					{
						MOCHINEDATE.Io_on_Off_Sucess =1;
//						MOCHINEDATE.Io_Step=0;
							Fertilizer_on();					
					}
					if(Msgtemp.CmdType == MSG_ACTION_ERTILIZER_OFF) //�رյ�ŷ�
					{
						MOCHINEDATE.Io_on_Off_Sucess =1;
//						MOCHINEDATE.Io_Step=0;
							Fertilizer_off();					
					}
				break;
				case MOCHINE_ALARM:
					if(Msgtemp.CmdType == MSG_ALARM) //�رյ�ŷ�
					{
						Task_error();
						switch(Msgtemp.CmdData[0])
						{
								case 1 :		//����������
								SetScreen(2);	
								break;
							  case 2 :    //������ȵ�ŷ�ʧ��
								SetScreen(5);				
								break;					
							  case 3 :	 //����ʩ�ʵ�ŷ�ʧ��
								SetScreen(3);										
								break;
								case 4 :		//���ʵ������ʧ��
								SetScreen(6);										
								break;
								default:
								break;
						}
					
					}				
					
				break;
				case MOCHINE_STOP:
					if(Msgtemp.CmdType == MSG_STOP) //�رյ�ŷ�
					{
						Machine_Ctrl_State =MOCHINE_IDLE; //�л�������״̬
						SetScreen(4);	
					
					}
				break;				
			  default:
				
				break;
			
			
			
			
			}
//			if(ControlMode == 1)
//			{
//				if(Msgtemp.CmdSrc == GPRS_TASK_CODE)
//				{
//					continue;
//				}
//			}
//			switch(WarterState)
//			{
//				case WARTER_IDLE://
//					if(Msgtemp.CmdType == MSG_WARTERING && Msgtemp.CmdData[0] == 1)
//					{
//						//������ˮ����ʱ
//						FlashReadWaterTime((u8*)&WarterRemainderTime); 
//						if(WarterRemainderTime > 0)
//						{
//							IO_OutSet(2,1);
//							SetMDCoil(0,1);
//							//������ʾ��ˮ����״̬
//							
//							//OS_ENTER_CRITICAL();
//							WaterStaShow(1);
//							//OS_EXIT_CRITICAL();
//							
//							if(Msgtemp.CmdSrc != GPRS_TASK_CODE)
//							{
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
//							}
//							WarterState = WARTER_WORK;
//						}
//					}
//					break;
//				case WARTER_WORK:
//					if(Msgtemp.CmdType == MSG_WARTERING && Msgtemp.CmdData[0] == 0)
//					{
//						IO_OutSet(2,0);
//						SetMDCoil(0,0);
//						//ֹͣ��ˮ����ʱ
//						//OS_ENTER_CRITICAL();
//						WaterStaShow(0);
//						//OS_EXIT_CRITICAL();
//						WarterRemainderTime = 0;
//						//������ʾ��ˮ����״̬
//						if(Msgtemp.CmdSrc != GPRS_TASK_CODE)
//						{
//							InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//							InitiativeParaTemp.DataAddr = 0;
//							InitiativeParaTemp.DataNum = 1;
//							SetIniactivePara(InitiativeParaTemp);
//							ModbusPara.Initiative = 1;
//						}
//						WarterState = WARTER_IDLE;
//					}
//					break;
//				default:
//					break;
//			}
//			switch(FertilizerState)
//			{
//				case FERTILIZER_IDLE:
//					if(Msgtemp.CmdType == MSG_FERTILIZER && Msgtemp.CmdData[0] == 1)
//					{
//						FlashReadFertilizerTime((u8*)&FertilizerRemainderTime); 
//						if(FertilizerRemainderTime > 0)
//						{
//							IO_OutSet(3,1);
//							SetMDCoil(1,1);
//							//����ʩ�ʵ���ʱ
//							//������ʾʩ�ʹ���״̬
//							//OS_ENTER_CRITICAL();
//							FertilizerStaShow(2);
//							//OS_EXIT_CRITICAL();
//							if(Msgtemp.CmdSrc != GPRS_TASK_CODE)
//							{
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 1;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
//							}
//							FertilizerState = FERTILIZER_WORK;
//						}
//					}
//					break;
//				case FERTILIZER_WORK:
//					if(Msgtemp.CmdType == MSG_FERTILIZER && Msgtemp.CmdData[0] == 0)
//					{
//						IO_OutSet(3,0);
//						SetMDCoil(1,0);
//						//ֹͣʩ�ʵ���ʱ
//						FertilizerRemainderTime = 0;
//						//������ʾʩ�ʹ���״̬
//						//OS_ENTER_CRITICAL();
//						FertilizerStaShow(0);
//						//OS_EXIT_CRITICAL();
//						if(Msgtemp.CmdSrc != GPRS_TASK_CODE)
//						{
//							InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//							InitiativeParaTemp.DataAddr = 1;
//							InitiativeParaTemp.DataNum = 1;
//							SetIniactivePara(InitiativeParaTemp);
//							ModbusPara.Initiative = 1;
//						}
//						FertilizerState = FERTILIZER_IDLE;
//					}
//					break;
//				default:
//					break;
//			}
//			if(Msgtemp.CmdType == MSG_MODECHANGE)
//			{
//				if(Msgtemp.CmdData[0] == 0)
//				{
//					ControlMode = 0;
//					//�ı������źżĴ���
//					SetInSta(0,0);
//				}
//				else
//				{
//					ControlMode = 1;
//					//�ı������źżĴ���
//					SetInSta(0,1);
//				}
//				if(Msgtemp.CmdSrc != GPRS_TASK_CODE)
//				{
//					InitiativeParaTemp.DataType = INITIATIVE_INPUT_STA;
//					InitiativeParaTemp.DataAddr = 0;
//					InitiativeParaTemp.DataNum = 1;
//					SetIniactivePara(InitiativeParaTemp);
//					ModbusPara.Initiative = 1;
//				}
//			}
//			else if(Msgtemp.CmdType == MSG_RADIOTUBE)
//			{
//				if(Msgtemp.CmdData[1] == 0)
//				{
//					
//					//ȫ������
//					SetMDCoil(2,0);
//					SetMDCoil(3,0);
//					SetMDCoil(4,0);
//					SetMDCoil(5,0);
//					SetMDCoil(6,0);
//					SetMDCoil(7,0);
//					SetMDCoil(8,0);
//					SetMDCoil(9,0);
//					SetMDCoil(10,0);
//					SetMDCoil(11,0);
//					SetMDCoil(12,0);
//					SetMDCoil(13,0);
//					SetMDCoil(14,0);
//					SetMDCoil(15,0);
//					SetMDCoil(16,0);
//					SetMDCoil(17,0);
//					//��״̬����������
//					//if(Msgtemp.CmdSrc != GPRS_TASK_CODE)
//					{
//						InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//						InitiativeParaTemp.DataAddr = 2;
//						InitiativeParaTemp.DataNum = 16;
//						SetIniactivePara(InitiativeParaTemp);
//						ModbusPara.Initiative = 1;
//					}
//					//else
//					{
//						SetRadiotube(Msgtemp.CmdData[0],0);
//					}
//				}
//				else if(Msgtemp.CmdData[1] == 1)
//				{
//					//�Ȱ�ԭ����λ�������
//					//������
//					u8 i ;
//					for(i=0;i<6;i++)
//					{
//						if((ModbusCoil[0]&(1<<(2+i)))>0)
//						{
//							SetRadiotube(i,0);
//						}
//					}
//					for(i=0;i<8;i++)
//					{
//						if((ModbusCoil[1]&(1<<i))>0)
//						{
//							SetRadiotube(i+2,0);
//						}
//					}
//					for(i=0;i<2;i++)
//					{
//						if((ModbusCoil[1]&(1<<i))>0)
//						{
//							SetRadiotube(i+10,0);
//						}
//					}
//					SetMDCoil(2,0);
//					SetMDCoil(3,0);
//					SetMDCoil(4,0);
//					SetMDCoil(5,0);
//					SetMDCoil(6,0);
//					SetMDCoil(7,0);
//					SetMDCoil(8,0);
//					SetMDCoil(9,0);
//					SetMDCoil(10,0);
//					SetMDCoil(11,0);
//					SetMDCoil(12,0);
//					SetMDCoil(13,0);
//					SetMDCoil(14,0);
//					SetMDCoil(15,0);
//					SetMDCoil(16,0);
//					SetMDCoil(17,0);
//					SetMDCoil(Msgtemp.CmdData[0]+2,1);
//					//if(Msgtemp.CmdSrc != GPRS_TASK_CODE)
//					{
//						InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//						InitiativeParaTemp.DataAddr = 2;
//						InitiativeParaTemp.DataNum = 16;
//						SetIniactivePara(InitiativeParaTemp);
//						ModbusPara.Initiative = 1;
//					}
//					//else
//					{
//						SetRadiotube(Msgtemp.CmdData[0],1);
//					}
//				}
//			}
//			else if(Msgtemp.CmdType == MSG_ZONEALARM)
//			{
//				SetZoneAlarm(Msgtemp.CmdData[0],Msgtemp.CmdData[1]);
//				if((ZoneAlarm[0] != ModbusInSta[1]) || (ZoneAlarm[1] != ModbusInSta[2]))
//				{
//					InitiativeParaTemp.DataType = INITIATIVE_INPUT_STA;
//					InitiativeParaTemp.DataAddr = 0;
//					InitiativeParaTemp.DataNum = 24;
//					SetIniactivePara(InitiativeParaTemp);
//					ModbusPara.Initiative = 1;
//					SetSwitchAlarm();
//				}
//			}
		}
	}

}

