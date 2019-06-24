#include "logic.h"
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
#include "communication.h"
u8 flower1_error_count=0;//��ȡˮ��1���ݴ������
u8 flower2_error_count=0;//��ȡˮ��2���ݴ������
u8 flower3_error_count=0;//��ȡˮ��3���ݴ������
u8 flower4_error_count=0;//��ȡˮ��4���ݴ������
extern Targetred ctargetrcord;//��ǰ�����¼
fertaskdata fertasktimedate;//ˮ��ʵʱ���ݶ�ȡ
fercal fercaldate; //��������
u8 gu8Use_MA_Check_Flag=0;//���Ͽ��ϼ��
//u8 gu8PressButton=0;
float task_start_flag=0;
float Use_A_Kg=0;
float Use_B_Kg=0;
float Use_C_Kg=0;
float Use_D_Kg=0;
extern u8 set_Pressbutonn[2];//����ѹ�����ؼ�����
u8 Press_error=0;
//��ʼ����־λ
void Init_Flag(void)
{

    task_start_flag=0; //������ʼ��־
    gu8Use_MA_Check_Flag=0;



}
//������ʼ��ʼˮ�����ݿ���
void Copy_Folwer(void)
{
 u8 i=0;
	for(i=0;i<16;i++)
	fertasktimedate.flowerdatabuf[i+32]=fertasktimedate.flowerdatabuf[i];//��ʼˮ������

}
//��������
void Folwer_cal(void)
{
 fercaldate.Q_A=fertasktimedate.ufertask.flower1-fertasktimedate.ufertask.startflower1;
 fercaldate.Q_B=fertasktimedate.ufertask.flower2-fertasktimedate.ufertask.startflower2;
 fercaldate.Q_C=fertasktimedate.ufertask.flower3-fertasktimedate.ufertask.startflower3;
 fercaldate.Q_D=fertasktimedate.ufertask.flower4-fertasktimedate.ufertask.startflower4;
}
void Folwer_Data_Check(void)
{
//		MsgStruct Msgtemp;
		MsgStruct MsgtempBlk;
		
	  if((fertasktimedate.ufertask.flower1>fertasktimedate.ufertask.lastflower1)&&(fercaldate.M_A>=fercaldate.USE_M_A))//������ݴ����ϴ����� ����û�дﵽĿ����
		{
				fertasktimedate.ufertask.lastflower1=fertasktimedate.ufertask.flower1;//����ˮ������
				flower1_error_count=0;
		
		}
		else if(fercaldate.USE_M_A>=fercaldate.M_A)
		{
		
			flower1_error_count=0;
		}
		else if((fertasktimedate.ufertask.flower1==fertasktimedate.ufertask.lastflower1)&&(fercaldate.USE_M_A<fercaldate.M_A))
		{
			flower1_error_count++;
		
		}
	  if((fertasktimedate.ufertask.flower2>fertasktimedate.ufertask.lastflower2)&&(fercaldate.USE_M_B<=fercaldate.M_B))//������ݴ����ϴ�����
		{
			fertasktimedate.ufertask.lastflower2=fertasktimedate.ufertask.flower2;//����ˮ������				
			flower2_error_count=0;
		
		}
		else if(fercaldate.USE_M_B>=fercaldate.M_B)
		{
		
			flower2_error_count=0;
		}		
		else if((fertasktimedate.ufertask.flower2==fertasktimedate.ufertask.lastflower2)&&(fercaldate.USE_M_B<fercaldate.M_B))
		{
			flower2_error_count++;
		
		}

	  if((fertasktimedate.ufertask.flower3>fertasktimedate.ufertask.lastflower3)&&(fercaldate.M_C>=fercaldate.USE_M_C))//������ݴ����ϴ�����
		{
			fertasktimedate.ufertask.lastflower3=fertasktimedate.ufertask.flower3;//����ˮ������		
			flower3_error_count=0;
		
		}
		else if(fercaldate.USE_M_C>=fercaldate.M_C)
		{
		
			flower3_error_count=0;
		}				
		else if((fertasktimedate.ufertask.flower3==fertasktimedate.ufertask.lastflower3)&&(fercaldate.USE_M_C<fercaldate.M_C))
		{
			flower3_error_count++;
		
		}
	  if((fertasktimedate.ufertask.flower4>fertasktimedate.ufertask.lastflower4)&&(fercaldate.M_D>=fercaldate.USE_M_D))//������ݴ����ϴ�����
		{
				fertasktimedate.ufertask.lastflower4=fertasktimedate.ufertask.flower4;//����ˮ������		
				flower4_error_count=0;
		
		}
		else if(fercaldate.USE_M_D>=fercaldate.M_D)
		{
		
			flower4_error_count=0;
		}				
		else if((fertasktimedate.ufertask.flower4==fertasktimedate.ufertask.lastflower4)&&(fercaldate.USE_M_D<fercaldate.M_D))
		{
			flower4_error_count++;
		
		}	
   if((flower1_error_count>60)||(flower2_error_count>60)||(flower3_error_count>60)||(flower4_error_count>60))
	 {
				gu8Use_MA_Check_Flag=0;
				MsgtempBlk.CmdType = MSG_ALARM; //������Ϣ����״̬����������
				MsgtempBlk.CmdData[0]= FLOWER_ERROR;//ˮ�����
				PackSendMasterQ(&MsgtempBlk);		 
	 }
}

//����ͨ���÷�������
void Chanel_Fer_Weight(u16 data)
{
	float temp,temp1,temp2;
	if(data>0xf000)
	{	
		data=65536-data;
		temp =data;
		temp=temp/10;
	}
	else
			temp =data;
	temp1=fercaldate.Th_A;
	temp1=temp1/100;
	temp2=fercaldate.formula_A;
	temp2=temp2/100;	
	//fercaldate.M_A=temp2*temp*fercaldate.amount/temp1;	
	fercaldate.M_A=temp2*temp*fercaldate.amount;	 //��˴������Ϊ���������Ƿ�ˮ����
	temp1=fercaldate.Th_B;
	temp1=temp1/100;
	temp2=fercaldate.formula_B;
	temp2=temp2/100;	
	fercaldate.M_B=temp2*temp*fercaldate.amount;
	temp1=fercaldate.Th_C;
	temp1=temp1/100;
	temp2=fercaldate.formula_C;
	temp2=temp2/100;		
  fercaldate.M_C=temp2*temp*fercaldate.amount;
	temp1=fercaldate.Th_D;
	temp1=temp1/100;
	temp2=fercaldate.formula_D;
	temp2=temp2/100;		
 fercaldate.M_D=temp2*temp*fercaldate.amount;	

}
float Use_Weight_Cal(float qa,float ph,float tha)
{
 float temp;
//���ڵõ���qaΪm3��L֮�����1000���Ĺ�ϵ
//qa��4λ����4λС˵������Ҫ����10000
	temp=(qa*ph/10)*tha/100;
	return temp;
}
//ͨ���ܶȼ���
float PH_Cal(float pot)
{
 float temp;

	pot= pot/100;//�ٷֱ�ת��
	temp=1/(1-pot);
	return temp;

}
void Use_M(void)
{
	float temp1,temp2;

	temp1=fercaldate.Th_A;
	temp1=temp1/100;
	temp2=fercaldate.formula_A;
	temp2=temp2/100;	
Use_A_Kg=fercaldate.Q_A*fercaldate.PH_A*temp1/temp2;
	temp1=fercaldate.Th_B;
	temp1=temp1/100;
	temp2=fercaldate.formula_B;
	temp2=temp2/100;	
Use_B_Kg=fercaldate.Q_B*fercaldate.PH_C*temp1/temp2;
	temp1=fercaldate.Th_C;
	temp1=temp1/100;
	temp2=fercaldate.formula_C;
	temp2=temp2/100;		
Use_C_Kg=fercaldate.Q_C*fercaldate.PH_C*temp1/temp2;
	temp1=fercaldate.Th_D;
	temp1=temp1/100;
	temp2=fercaldate.formula_D;
	temp2=temp2/100;		
Use_D_Kg=fercaldate.Q_D*fercaldate.PH_D*temp1/temp2;		



}

void FerLogic_task(void *pdata)
{

	pdata=pdata;
//	MsgStruct Msgtemp;
	MsgStruct MsgtempBlk;
//	MsgComStruct Msgtemp1;
	MsgComStruct MsgtempBlk1;
 while(1)
 {
		delay_ms(1000);//1S����������Ƿ��б䶯
	  if(gu8Use_MA_Check_Flag==1)
		{
			Folwer_cal();
			Folwer_Data_Check();//�������
	  fercaldate.USE_M_A=Use_Weight_Cal(fercaldate.Q_A,fercaldate.PH_A,fercaldate.Th_A);
 	  fercaldate.USE_M_B=Use_Weight_Cal(fercaldate.Q_B,fercaldate.PH_B,fercaldate.Th_B);
	 	fercaldate.USE_M_C=Use_Weight_Cal(fercaldate.Q_C,fercaldate.PH_C,fercaldate.Th_C);
	  fercaldate.USE_M_D=Use_Weight_Cal(fercaldate.Q_D,fercaldate.PH_D,fercaldate.Th_D);
	  if((fercaldate.USE_M_A>=fercaldate.M_A)&&((ctargetrcord.trecord.fer_chanle&0X01)==1))
	  {
			ctargetrcord.trecord.fer_chanle^=(1<<0);
			MsgtempBlk1.CmdType = COM_WRITE; //������Ϣ����״̬����������
			MsgtempBlk1.CmdData[0]= 2;//����4
			MsgtempBlk1.CmdData[1]= 4;//�ر�ˮ��
			PackSendComQ(&MsgtempBlk1); 					
		}
	  if((fercaldate.USE_M_B>=fercaldate.M_B)&&(((ctargetrcord.trecord.fer_chanle>>1)&0X01)==1))
	  {
			ctargetrcord.trecord.fer_chanle^=(1<<1);
			MsgtempBlk1.CmdType = COM_WRITE; //������Ϣ����״̬����������
			MsgtempBlk1.CmdData[0]= 2;//����4
			MsgtempBlk1.CmdData[1]= 4;//�ر�ˮ��
			PackSendComQ(&MsgtempBlk1); 			
		
		}
	  if((fercaldate.USE_M_C>=fercaldate.M_C)&&(((ctargetrcord.trecord.fer_chanle>>2)&0X01)==1))
	  {
			ctargetrcord.trecord.fer_chanle^=(1<<2);
			MsgtempBlk1.CmdType = COM_WRITE; //������Ϣ����״̬����������
			MsgtempBlk1.CmdData[0]= 2;//����4
			MsgtempBlk1.CmdData[1]= 4;//�ر�ˮ��
			PackSendComQ(&MsgtempBlk1); 			
		
		}
	  if((fercaldate.USE_M_D>=fercaldate.M_D)&&(((ctargetrcord.trecord.fer_chanle>>3)&0X01)==1))
	  {
			ctargetrcord.trecord.fer_chanle^=(1<<3);
			MsgtempBlk1.CmdType = COM_WRITE; //������Ϣ����״̬����������
			MsgtempBlk1.CmdData[0]= 2;//����4
			MsgtempBlk1.CmdData[1]= 4;//�ر�ˮ��
			PackSendComQ(&MsgtempBlk1); 			
		
		}				
			
      
	
		if((ctargetrcord.trecord.curtarget_back_time>=ctargetrcord.trecord.currenttargetlefttime)&&
			((fercaldate.USE_M_D<fercaldate.M_D)||(fercaldate.USE_M_A<fercaldate.M_A)||(fercaldate.USE_M_B<fercaldate.M_B)||(fercaldate.USE_M_C<fercaldate.M_C)))
		//����ָ��ʱ�仹û��ɽ��ʱ���
		{
				gu8Use_MA_Check_Flag=0;
				MsgtempBlk.CmdType = MSG_ALARM; //������Ϣ����״̬����������
				MsgtempBlk.CmdData[0]= PRESS_BUTTON_ERROR;//ˮ�����
				PackSendMasterQ(&MsgtempBlk);	 						
		
		
		}
		}

		else
		{
				flower1_error_count=0;
				flower2_error_count=0;
				flower3_error_count=0;
				flower4_error_count=0;
				fercaldate.USE_M_A=0;
				fercaldate.USE_M_B=0;
				fercaldate.USE_M_C=0;
				fercaldate.USE_M_D=0;	
				Use_A_Kg=0;
				Use_B_Kg=0;
				Use_C_Kg=0;
				Use_D_Kg=0;			
//        if((ctargetrcord.trecord.fer_chanle!=0))
//        {
//					ctargetrcord.trecord.fer_chanle=0;
//					MsgtempBlk1.CmdType = COM_WRITE; //������Ϣ����״̬����������
//					MsgtempBlk1.CmdData[0]= 2;//����4
//					MsgtempBlk1.CmdData[1]= 1;//�ر�ˮ��
//					PackSendComQ(&MsgtempBlk1); 				
//				
//				}					
		}
//		Use_M();
		if(task_start_flag==1)
		{
			if((set_Pressbutonn[1]==1)&&(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)==1))
			{
				Press_error++;
				
			}
			if((set_Pressbutonn[1]==1)&&(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)==0))
			{
				Press_error=0;
			
			}	
            if(Press_error>5)
			{
				task_start_flag=0;
				Press_error=0;
				MsgtempBlk.CmdType = MSG_ALARM; //������Ϣ����״̬����������
				MsgtempBlk.CmdData[0]= PRESS_BUTTON_ERROR;//ˮ�����
				PackSendMasterQ(&MsgtempBlk);	 				
			}
		}
 }

}
