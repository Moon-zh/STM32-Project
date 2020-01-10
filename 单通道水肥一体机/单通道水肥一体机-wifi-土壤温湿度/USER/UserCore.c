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

#include	"cgq.h"


extern CountDownStruct WorkTime;

MsgStruct MasterMsg;
OS_EVENT *MasterQMsg; //�ź���������һ��OS_EVENT���͵�ָ��ָ�����ɵĶ���
void* 	MasterMsgBlock[8]; //��Ϣ�������飬 ����һ��ָ����Ϣ�����ָ�������Ĵ�С����ָ�������������Ϊvoid����
OS_MEM* MasterPartitionPt;/*�����ڴ���ƿ�ָ�룬����һ���ڴ����ʱ������ֵ������ */
u8 g_u8AgvCtrMsgMem[20][8];///*����һ������20���ڴ�飬ÿ���ڴ�鳤����8���ڴ���� */

WorkPara IrrPara;//����������ʱʹ�õı���
u8 MasterState = 0;//������״̬����


//������Ϣ���к��ڴ��
//���0����������
//    1����Ϣ���д���ʧ��
//	  2���ڴ�鴴��ʧ��
u8 MasterQInit(void)
{
	INT8U os_err;
	
	MasterQMsg = OSQCreate ( MasterMsgBlock, 8);
	
	if(MasterQMsg == (OS_EVENT *)0)
	{
		return 1;
	}
	
	MasterPartitionPt = OSMemCreate (
										g_u8AgvCtrMsgMem,
				  						20,
				  						8,
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
	MsgTemp ->CmdData[2] = MsgBlk->CmdData[2];
	MsgTemp ->CmdData[3] = MsgBlk->CmdData[3];
	MsgTemp ->CmdData[4] = MsgBlk->CmdData[4];
	MsgTemp ->CmdData[5] = MsgBlk->CmdData[5];
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
	MsgTemp ->CmdData[2] = MsgBlk->CmdData[2];
	MsgTemp ->CmdData[3] = MsgBlk->CmdData[3];
	MsgTemp ->CmdData[4] = MsgBlk->CmdData[4];
	MsgTemp ->CmdData[5] = MsgBlk->CmdData[5];
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

//����:8λ������
//���:��͵�Ϊ1��λ��
//����:������������͵�Ϊ1��λ������,������λ����
u8 Seek1LowBit(u8*Figure)
{
	u8 Ret = 0;
	u8 i = 0;
	for(i=0;i<8;i++)
	{
		if((*Figure&(1<<i))>0)
		{
			Ret = i+1;
			*Figure = *Figure^(1<<i);
			break;
		}
	}
	return Ret;
}

//��ȡ����������ڵ�����״̬
u8 GetSysState(void)
{
	return MasterState;
}

/*
//���������
IO_OutSet(1,1��0); ��ˮ�ĵ�ŷ� ��-1����-0
IO_OutSet(2,1��0); ��ˮ�ĵ�ŷ�-��һ������ ��-1����-0
IO_OutSet(3,1��0); ��ˮ�ĵ�ŷ�-�ڶ������� ��-1����-0
*/
void MasterCtrl_task(void *pdata)
{
	u8 os_err;
	MsgStruct *pMsgBlk = NULL;
  MsgStruct Msgtemp;
	

	MasterQInit();//������Ϣ���к��ڴ��
	
	while(1)
	{
		delay_ms(100);
		
		pMsgBlk = ( MsgStruct *) OSQPend ( MasterQMsg,
				10,
				&os_err );
	   //������Ϣ���У������ֱ��ǣ�RemoteQMsgΪ��������Ϣ���е�ָ��  �ڶ�������Ϊ�ȴ�ʱ��
		 //0��ʾ���޵ȴ���&errΪ������Ϣ������ֵΪ���п��ƿ�RemoteQMsg��ԱOSQOutָ�����Ϣ(�������������Ϣ���õĻ�)�����
		//û����Ϣ���ã���ʹ����OSQPend���������ʹ֮���ڵȴ�״̬��������һ���������
		
		if(os_err == OS_ERR_NONE)//ͨ���ɹ��������յ���Ϣ
		{ 
			memcpy((u8*)&Msgtemp,(u8*)pMsgBlk,sizeof(MsgStruct) );//��ԴpMsgBlk����n���ֽڵ�ĿMsgtemp��			
			DepackReceiveMasterQ(pMsgBlk);//������Ϣ��������Ϣ�ڴ��ͷ�
			
			switch(MasterState)//����������״̬
			{
				case SYSTEM_IDLE://����״̬ 0
					if(Msgtemp.CmdType == MSG_START)//����
					{
						SetRemotePara(3,1);//����ֹͣ-0������-1
						
						IrrPara.WorkModel  = Msgtemp.CmdData[0];//�������(to_type)    Ĭ��-0����ˮ-1����ˮ-2
						IrrPara.Zone = Msgtemp.CmdData[1];//��Ҫ��ȷ�����need_partition��  1����-1,2����-2,1��2����-3
						IrrPara.TimeH = Msgtemp.CmdData[2];//���ʱ����Сʱ��
						IrrPara.TimeM = Msgtemp.CmdData[3];//���ʱ�������ӣ�
						
						SetRemoteRead(0,IrrPara.WorkModel);//���õ�ǰ����״̬���������(to_type)    Ĭ��-0����ˮ-1����ˮ-2					
						SetRemotePara(0,IrrPara.WorkModel);//�������(to_type)    Ĭ��-0����ˮ-1����ˮ-2
						SetRemotePara(2,IrrPara.Zone);//��Ҫ��ȷ�����need_partition��  1����-1,2����-2,1��2����-3
						SetRemotePara(1,(IrrPara.TimeH*60+IrrPara.TimeM));//���ʱ����to_time��  ʱ�䳣��

											
						//������ͣ���ˮ
						if(IrrPara.WorkModel == 1)
						{
							//�������ĸ�������Ҫ���
							IrrPara.ImplementZone = Seek1LowBit(&IrrPara.Zone)+1;
							if(IrrPara.ImplementZone != 1)//��һ��������Ҫ��ȣ�IrrPara.ImplementZone����2���ڶ���������Ҫ��ȣ�����3
							{
								//�򿪷�����ŷ�  ��ˮ��ŷ�����
								IO_OutSet(IrrPara.ImplementZone,1);
								//��������ʱ
								StartCount(&WorkTime,IrrPara.TimeH,IrrPara.TimeM);
								MasterState = SYSTEM_WATER;//����״̬����ˮ
								//ˢ�½��� 
								WorkModelShow(1,IrrPara.ImplementZone);//���´�������ʾ���ݣ���ǰ���ģʽ+��ǰ��ȷ�����
								
								SetRemoteRead(2,IrrPara.ImplementZone-1);//���õ�ǰ��ȷ���1����-1,2����-2
	
								if(Guangai.zdms==1)fsbz_3060_cx();//����һ���ϴ����ݵĳ���ֻ���Զ�ģʽ�·�������)
							}
						}
						//������ͣ���ˮ
						else if(IrrPara.WorkModel == 2)
						{
							//�������ĸ�������Ҫ���
							IrrPara.ImplementZone = Seek1LowBit(&IrrPara.Zone)+1;
							
							if(IrrPara.ImplementZone != 1)//��һ��������Ҫ��ȣ�IrrPara.ImplementZone����2���ڶ���������Ҫ��ȣ�����3
							{
								//�򿪷�����ŷ����Ƚ��н���ˮ  ��ˮ��ŷ�����
								IO_OutSet(IrrPara.ImplementZone,1);
								//��������ʱ
								StartCount(&WorkTime,0,FERTILIZER1TIME);//�ȵ���ʱ10����
								MasterState = SYSTEM_FERTILIZER1;//ʩ��1�׶�
								//ˢ�½��� 
								WorkModelShow(0,IrrPara.ImplementZone);//���´�������ʾ���ݣ���ǰ���ģʽ+��ǰ��ȷ�����
								
								SetRemoteRead(2,IrrPara.ImplementZone-1);//���õ�ǰ��ȷ���1����-1,2����-2							
				
							}
						}
					}
					break;
					
				case SYSTEM_WATER://����״̬����ˮ 1
					if(Msgtemp.CmdType == MSG_STOP) //ֹͣ
					{
						//�رյ�ŷ�   ��ˮ��ŷ��ر�
						IO_OutSet(IrrPara.ImplementZone,0);
						//ֹͣ��ʱ
						ClearCount(&WorkTime);
						//�ı乤��״̬
						MasterState = SYSTEM_IDLE;  //����״̬
						//ˢ�½���
						if(Guangai.zdms==0)SetScreen(LCD_MODELSEL_PAGE);//�л����棬�ص�����ѡ�����(�ڿ����Զ�ģʽʱ�����л�����)
						//��տ��Ʋ���
						memset(&IrrPara,0,sizeof(IrrPara));
						SetRemotePara(3,0);//����ֹͣ-0������-1
						SetRemoteRead(2,0);//�����ǰ��ȷ�����at_partition��    1����-1,2����-2������-0
						SetRemoteRead(0,0);//����״̬��work_state�� ����-0����ˮ-1����ˮ-2 
						SetRemoteRead(1,0);//�������ʱ��in_time��0
						
            fsbz_3060_cx();//����һ���ϴ����ݵĳ��� 
			
							
					}
					else if(Msgtemp.CmdType == MSG_TIMEOVER) //����ʱ����
					{
						//�ر��Ѿ���ɵķ���  ��ˮ��ŷ��ر�
						IO_OutSet(IrrPara.ImplementZone,0);
						//�ı�ƽ̨״̬
						SetRemoteRead(2,0);//�����ǰ��ȷ�����at_partition��  1����-1,2����-2
						
						//���һ���û����Ҫ��ȵķ���
						IrrPara.ImplementZone = Seek1LowBit(&IrrPara.Zone)+1;
						//���з���û�н���
						if(IrrPara.ImplementZone!=1)//�ڶ���������Ҫ��ȣ�IrrPara.ImplementZone����3
						{

							//�򿪷�����ŷ�  ��ˮ��ŷ�����
							IO_OutSet(IrrPara.ImplementZone,1);
							//��ʼ����ʱ
							StartCount(&WorkTime,IrrPara.TimeH,IrrPara.TimeM);
							//ˢ�½���
							WorkModelShow(1,IrrPara.ImplementZone);//���´�������ʾ���ݣ���ǰ���ģʽ+��ǰ��ȷ�����
							//�ı�ƽ̨״̬
							SetRemoteRead(2,IrrPara.ImplementZone-1);//���õ�ǰ��ȷ���1����-1,2����-2			

							if(Guangai.zdms==1)fsbz_3060_cx();//����һ���ϴ����ݵĳ���ֻ���Զ�ģʽ�·�������)
//							Msgtemp.CmdType = MSG_START;//����
//		          PackSendMasterQ(&Msgtemp);//��Ϣ�������
						}
						else//���з���������
						{
							//�ı�ƽ̨״̬
							
							//ֹͣ��ʱ
						  ClearCount(&WorkTime);
							//�ı乤��״̬
							MasterState = SYSTEM_IDLE;//����״̬
							//ˢ�½���
							if(Guangai.zdms==0)SetScreen(LCD_MODELSEL_PAGE);//�л����棬�ص�����ѡ����� (�ڿ����Զ�ģʽʱ�����л�����)
							//��տ��Ʋ���
							memset(&IrrPara,0,sizeof(IrrPara));
							SetRemotePara(3,0);//����ֹͣ-0������-1
							SetRemoteRead(0,0);//����״̬��work_state�� ����-0����ˮ-1����ˮ-2 
							SetRemoteRead(2,0);//�����ǰ��ȷ�����at_partition��  1����-1,2����-2������-0
							SetRemoteRead(1,0);//�������ʱ��in_time��0
							
							fsbz_3060_cx();//����һ���ϴ����ݵĳ���
						}
						
					}				
					break;
					
				case SYSTEM_FERTILIZER1://ʩ��1�׶� (�˽׶��Ƚ��н���ˮ)
					if(Msgtemp.CmdType == MSG_STOP)//ֹͣ
					{
						//�رյ�ŷ����رս���ˮ ��ˮ��ŷ��ر�
						IO_OutSet(IrrPara.ImplementZone,0);
						//ֹͣ��ʱ
						ClearCount(&WorkTime);
						//�ı乤��״̬
						MasterState = SYSTEM_IDLE;//����״̬
						//ˢ�½���
						SetScreen(LCD_MODELSEL_PAGE);//�л�����
						//��տ��Ʋ���
						memset(&IrrPara,0,sizeof(IrrPara));
						SetRemotePara(3,0);//����ֹͣ-0������-1
						SetRemoteRead(2,0);//�����ǰ��ȷ�����at_partition��    1����-1,2����-2������-0
						SetRemoteRead(0,0);//����״̬��work_state�� ����-0����ˮ-1����ˮ-2 
						SetRemoteRead(1,0);//�������ʱ��in_time��0

            fsbz_3060_cx();//����һ���ϴ����ݵĳ���
					}
					else if(Msgtemp.CmdType == MSG_TIMEOVER)//����ʱ����
					{
						//����ʩ�� ��ˮ��ŷ���1,1������
						IO_OutSet(1,1);
						//��������ʱ�����õ���ʱʱ��
						IrrPara.FerTimeM=0;
						IrrPara.FerTimeH=0;
						if(IrrPara.TimeM>=(FERTILIZER1TIME+FERTILIZER3TIME))//��ȷ���
						{
							IrrPara.FerTimeM = IrrPara.TimeM-(FERTILIZER1TIME+FERTILIZER3TIME);
							IrrPara.FerTimeH = IrrPara.TimeH;
						}
						else   if(IrrPara.TimeH>0)//���Сʱ����0
						{
							IrrPara.FerTimeM = IrrPara.TimeM+60-(FERTILIZER1TIME+FERTILIZER3TIME);
							IrrPara.FerTimeH = IrrPara.TimeH-1;
						}
						StartCount(&WorkTime,IrrPara.FerTimeH,IrrPara.FerTimeM);//���õ���ʱ
									
						MasterState = SYSTEM_FERTILIZER2;//����ʩ��2�׶�						
					}
					break;
					
				case SYSTEM_FERTILIZER2://ʩ��2�׶�
					if(Msgtemp.CmdType == MSG_STOP)//ֹͣ
					{
						//ֹͣʩ��   ��ˮ��ŷ���1,0���ر� 
						IO_OutSet(1,0);
						//�رյ�ŷ����رս���ˮ  
						IO_OutSet(IrrPara.ImplementZone,0);
						//ֹͣ��ʱ
						ClearCount(&WorkTime);
						
						//�ı乤��״̬
						MasterState = SYSTEM_IDLE;//����״̬
						//ˢ�½���
						SetScreen(LCD_MODELSEL_PAGE);//�л�����
						//��տ��Ʋ���
						memset(&IrrPara,0,sizeof(IrrPara));
						SetRemotePara(3,0);//����ֹͣ-0������-1
						SetRemoteRead(2,0);//�����ǰ��ȷ�����at_partition��    1����-1,2����-2������-0
						SetRemoteRead(0,0);//����״̬��work_state�� ����-0����ˮ-1����ˮ-2 
						SetRemoteRead(1,0);//�������ʱ��in_time��0
						
            fsbz_3060_cx();//����һ���ϴ����ݵĳ���
					}
					else if(Msgtemp.CmdType == MSG_TIMEOVER)//����ʱ����
					{
						//ֹͣʩ��   ��ˮ��ŷ���1,0���ر� 
						IO_OutSet(1,0);
						//�򿪷�����ŷ����Ƚ��н���ˮ  ��ˮ��ŷ�����
			  		IO_OutSet(IrrPara.ImplementZone,1);					
						
						//��������ʱ
						StartCount(&WorkTime,0,FERTILIZER3TIME);//���е���ʱ10����
												
						MasterState = SYSTEM_FERTILIZER3;//ʩ��3�׶�

						WorkCountShow(WorkTime.hour,WorkTime.min);//������������ʾ��//����ʱ	
					}
					break;
					
				case SYSTEM_FERTILIZER3://ʩ��3�׶�(�ٴν���ˮ)
					if(Msgtemp.CmdType == MSG_STOP)//����ʱ����
					{
						//�رյ�ŷ����رս���ˮ   
						IO_OutSet(IrrPara.ImplementZone,0);
						//ֹͣ��ʱ
						ClearCount(&WorkTime);
						//�ı乤��״̬
						MasterState = SYSTEM_IDLE;//����״̬
						//ˢ�½���
						SetScreen(LCD_MODELSEL_PAGE);//�л�����
						//��տ��Ʋ���
						memset(&IrrPara,0,sizeof(IrrPara));
						SetRemotePara(3,0);//����ֹͣ-0������-1
						SetRemoteRead(2,0);//�����ǰ��ȷ�����at_partition��    1����-1,2����-2������-0
						SetRemoteRead(0,0);//����״̬��work_state�� ����-0����ˮ-1����ˮ-2 
						SetRemoteRead(1,0);//�������ʱ��in_time��0
						
            fsbz_3060_cx();//����һ���ϴ����ݵĳ���
					}
					else if(Msgtemp.CmdType == MSG_TIMEOVER)//����ʱ����
					{
						//�رյ�ŷ����رս���ˮ
						IO_OutSet(IrrPara.ImplementZone,0);
						//�ı�ƽ̨״̬
						SetRemoteRead(2,0);//�����ǰ��ȷ�����at_partition��  1����-1,2����-2
						
						//���һ���û����Ҫ��ȵķ���
						IrrPara.ImplementZone = Seek1LowBit(&IrrPara.Zone)+1;
						//���з���û�н���
						if(IrrPara.ImplementZone!=1)//�ڶ���������Ҫ��ȣ�IrrPara.ImplementZone����3
						{
							//�򿪷�����ŷ�
							IO_OutSet(IrrPara.ImplementZone,1);
							//��������ʱ
							StartCount(&WorkTime,0,FERTILIZER1TIME);//�ȵ���ʱ10����
							MasterState = SYSTEM_FERTILIZER1;//ʩ��1�׶�
							//ˢ�½��� 
							WorkModelShow(0,IrrPara.ImplementZone);
							//�ı�ƽ̨״̬
							SetRemoteRead(2,IrrPara.ImplementZone-1);//��ǰ��ȷ�����at_partition��  1����-1,2����-2
						
						}
						else//���з���������
						{		
							//�ı乤��״̬
							MasterState = SYSTEM_IDLE;//����״̬
							//ֹͣ��ʱ
						  ClearCount(&WorkTime);
							//ˢ�½���
							SetScreen(LCD_MODELSEL_PAGE);
							//��տ��Ʋ���
							memset(&IrrPara,0,sizeof(IrrPara));
							//�ı�ƽ̨״̬
							SetRemotePara(3,0);//����ֹͣ-0������-1
							SetRemoteRead(2,0);//�����ǰ��ȷ�����at_partition��    1����-1,2����-2������-0
							SetRemoteRead(0,0);//����״̬��work_state�� ����-0����ˮ-1����ˮ-2 
							SetRemoteRead(1,0);//�������ʱ��in_time��0	
							
							fsbz_3060_cx();//����һ���ϴ����ݵĳ���
						}
					
					}
					break;
			}
		}

	}
}

