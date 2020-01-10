#include	"cgq.h"

#include	"crc16.h"
#include	"rs485.h"
#include	"delay.h"

#include "UserCore.h"
#include "UserHmi.h"



//�Զ�ģʽ��ʹ�õı�־
Zidongmoshi Guangai;


/*
Modbusͨ�ŵĴ�����
*/


/*
���Ͳ�ѯ��ص�ַ�¼Ĵ�����ַ������
[0]:��ַ [1]:������ [2][3]:��ʼ��ַ  [4][5]:���ݳ��� [6][7]:У����
*/
void sendinstruct_cgq(u8 addr,u8 jcqdz1,u8 jcqdz2)
{
	u8 xwsz[]={0x01,0x03,0x00,0x02,0x00,0x01,0x25,0xca};
	u16 crc16bz;
	u8 i;
	
	xwsz[0]=addr;//��ַ
	xwsz[2]=jcqdz1;//�Ĵ�����ʼ��ַ1
	xwsz[3]=jcqdz2;//�Ĵ�����ʼ��ַ2
	crc16bz=crc16table(xwsz,6);//����CRC16У��
	xwsz[6]=crc16bz&0xff;
	xwsz[7]=crc16bz>>8;
	
	for(i=0;i<8;i++)
	{
		comSendChar(COM4,xwsz[i]);		
	}
	delay_ms(100);
	
}


u16 cgq_trwsd_hc[2];
/*
��ȡ������ʪ�ȴ�����������
ms=0 �¶ȣ�
ms=1 ʪ�ȣ�
*/
u8 readdata_cgq_tr(u8 ms)
{
	u8 buf[30];
	u8 len=0,crc16sz[2];
	u16 crc16bz;
	u16 data;
	
	memset(buf,0,sizeof buf);
	
	len=COM4GetBuf(buf,28);
	if(len==0)return 0;
	crc16bz=crc16table(buf,len-2);
	crc16sz[0]=crc16bz>>8;//��8λ
	crc16sz[1]=crc16bz&0xff;//��8λ
	
	comClearRxFifo(COM4);
	
	if((buf[len-1]==crc16sz[0])&&(buf[len-2]==crc16sz[1]))//У��CRC��ȷ
	{
		if(ms==1)//����ʪ��
		{
				data=buf[3];
				data=data<<8;
				data|=buf[4];
			
		  	cgq_trwsd_hc[0]=data;
		}
		else	if(ms==0)//�����¶�
		{
				data=buf[3];
				data=data<<8;
				data|=buf[4];
				if(data>1000)//��ʾ�¶ȵ������  65435
				{
					data=(0XFFFF-data)+1;
					
					data|=0x8000; //���λΪ1����ʾΪ�¶ȵ������
				}
			 	cgq_trwsd_hc[1]=data;
		}
		
		return 1;
	}
	return 0;	
	
}

u8 cgq_trsd[2];//����ʪ��
u8 cgq_trwd[3];//�����¶�,[0]Ϊ1��ʾΪ������[0]Ϊ0��ʾΪ����
//������-������ʪ�����ݴ���
void cgq_tr_sjcl(void)
{
	u16 a;
	
	//����ʪ�� 345=34.5%RH
	a=cgq_trwsd_hc[0]/100;
	a*=10;
	a+=cgq_trwsd_hc[0]/10%10;
	cgq_trsd[0]=a;//����ʪ�ȣ�С����ǰ��ʮλ�͸�λ��
	cgq_trsd[1]=cgq_trwsd_hc[0]%10;//����ʪ��
	
	//�����¶� 456=45.6��
	if(cgq_trwsd_hc[1]&0x8000==1)//�¶ȵ���0��
	{
		cgq_trwd[0]=1;
	}
	else	cgq_trwd[0]=0;
	cgq_trwsd_hc[1]=cgq_trwsd_hc[1]&0x7FFF;
	a=cgq_trwsd_hc[1]/100;
	a*=10;
	a+=cgq_trwsd_hc[1]/10%10;
	cgq_trwd[1]=a;//�����¶�
	cgq_trwd[2]=cgq_trwsd_hc[1]%10;//�����¶�
	
}



//������-������ʪ�ȳ��� ����Ҫ����
void CGQ_cx(void)
{	
	u8 sz_hc[20];//��������
	MsgStruct Msgtemp;
  WorkPara Hmi_Para_Zidong;	
	

	if(Guangai.zdms==1)//�������Զ�ģʽ
	{
			//��ȡ����ʪ��
			sendinstruct_cgq(0x01,0x00,0x02);//���Ͳ�ѯָ��
			delay_ms(300);
			readdata_cgq_tr(1);
			delay_ms(200);			
			
			//��ȡ�����¶�
			sendinstruct_cgq(0x01,0x00,0x03);//���Ͳ�ѯָ��
			delay_ms(300);
			readdata_cgq_tr(0);
			delay_ms(200);			
		
			
			cgq_tr_sjcl();//������-������ʪ�����ݴ���
			
			//������ʪ��ֵ�ϴ�������������
			sprintf((char *)sz_hc,"%d.%d%%RH",cgq_trsd[0],cgq_trsd[1]);
			SetTextValue(8,1,sz_hc);//����-����ʪ��ֵ
			if(cgq_trwd[0]==1)//�¶�����
			{
				sprintf((char *)sz_hc,"һ");
				SetTextValue(8,8,sz_hc);	
			}
			else	
			{
				sprintf((char *)sz_hc,"  ");
				SetTextValue(8,8,sz_hc);				
			}
			sprintf((char *)sz_hc,"%d.%d��",cgq_trwd[1],cgq_trwd[2]);
			SetTextValue(8,2,sz_hc);//����-�����¶�ֵ	


			if((cgq_trsd[0]<Guangai.qdyzbz)&&(Guangai.zdms==1))//����ʪ��С������������ֵ���Զ�ģʽ������������ˮ
			{
					Guangai.qdbz=1;//�Զ�ģʽ������ˮ
					if(Guangai.yichi==0)//ִ��һ�εı�־
					{
						Hmi_Para_Zidong.WorkModel = 1;//��ˮ
						Hmi_Para_Zidong.Zone = 0x03;//Ĭ�Ͽ�����������
						Hmi_Para_Zidong.TimeH =12;//���ʱ��
						Hmi_Para_Zidong.TimeM =0;//���ʱ��						
						
						Msgtemp.CmdSrc = SCREEN_TASK_CODE;
						Msgtemp.CmdType = MSG_START;//����
						Msgtemp.CmdData[0] = Hmi_Para_Zidong.WorkModel;//���֣�����ˮ����ʩ��
						Msgtemp.CmdData[1] = Hmi_Para_Zidong.Zone;//Ҫִ�м�������
						Msgtemp.CmdData[2] = Hmi_Para_Zidong.TimeH;//ʱ�䣺Сʱ
						Msgtemp.CmdData[3] = Hmi_Para_Zidong.TimeM;//ʱ�䣺����
						PackSendMasterQ(&Msgtemp);//��Ϣ�������	

						//�����������õ�ǰ���״̬
						sprintf((char *)sz_hc,"������ˮ���");
						SetTextValue(8,4,sz_hc);						
						
						Guangai.yichi=1;
					}
			}
			else	if((cgq_trsd[0]>=(Guangai.tzyzbz))&&(Guangai.qdbz==1))//����ʪ�ȴ��ڵ�������ֹͣ��ֵ���Ѿ���ʼ��ˮ��ֹͣ��ˮ
			{
					Guangai.qdbz=0;//�Զ�ģʽ�رս�ˮ
					Guangai.yichi=0;//��־���
					
					//����ֹͣ��Ϣ
					Msgtemp.CmdSrc = SCREEN_TASK_CODE;
					Msgtemp.CmdType = MSG_STOP;  //����
					//Msgtemp.CmdData[0] = 1;//�������(to_type)    Ĭ��-0����ˮ-1����ˮ-2  ƽ̨�жϵĲ���Ϊ0
					Msgtemp.CmdData[1] = 0;//��Ҫ��ȷ�����need_partition��  1����-1,2����-2,1��2����-3
					Msgtemp.CmdData[2] = 0;//���ʱ����Сʱ��
					Msgtemp.CmdData[3] = 0;//���ʱ�������ӣ�	
					PackSendMasterQ(&Msgtemp);	

			  	//�����������õ�ǰ���״̬
					sprintf((char *)sz_hc,"�ر���ˮ���");
					SetTextValue(8,4,sz_hc);				
			}
			
			if((Guangai.qdbz==1)&&(MasterState==0))//����Ҫ��ˮ����֮ǰ���õĵ���ʱ�����ˡ� ��������Ϊ�����ˣ���Ҫ���·�������
			{
					Guangai.yichi=0;//��ִ��һ����ˮ������	
			}
	}

									
	
	
}


