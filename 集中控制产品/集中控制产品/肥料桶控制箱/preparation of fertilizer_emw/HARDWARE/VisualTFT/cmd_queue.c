/************************************��Ȩ����********************************************
**                             ���ݴ�ʹ��Ƽ����޹�˾                                
**                             http://www.gz-dc.com
**-----------------------------------�ļ���Ϣ--------------------------------------------
** �ļ�����:   cmd_queue.c 
** �޸�ʱ��:   2012-06-01
** �ļ�˵��:   ָ����л�����
**------------------------------------------------------------------------------------*/

#include "cmd_queue.h"

typedef struct _QUEUE
{
	qsize _head;
	qsize _tail;
	qdata _data[QUEUE_MAX_SIZE];	
}QUEUE;

QUEUE que = {0,0,0};
static qdata cmd_state = 0;
static qsize cmd_pos = 0;

static QUEUE soanar_que = {0,0,0};
static qdata soanar_cmd_state = 0;
static qsize soanar_cmd_pos = 0;

void queue_reset()
{
	que._head = que._tail = 0;
	cmd_pos = cmd_state = 0;
}
void soanar_queue_reset()
{
	soanar_que._head = soanar_que._tail = 0;
	soanar_cmd_pos = soanar_cmd_state = 0;
}

void queue_push(qdata _data)
{
	qsize pos = (que._head + 1) % QUEUE_MAX_SIZE;
	
	if(pos != que._tail)//����״̬
	{
		que._data[que._head] = _data;
		que._head = pos;
	}
}
void soanar_queue_push(qdata _data)
{
	qsize pos = (soanar_que._head + 1) % QUEUE_MAX_SIZE;
	
	if(pos != soanar_que._tail)//����״̬
	{
		soanar_que._data[soanar_que._head] = _data;
		soanar_que._head = pos;
	}
}

static void queue_pop(qdata* _data)
{
	if(que._tail != que._head)//�ǿ�״̬
	{
		*_data = que._data[que._tail];
		que._tail = (que._tail + 1) % QUEUE_MAX_SIZE;
	}
}
static void soanar_queue_pop(qdata* _data)
{
	if(soanar_que._tail != soanar_que._head)//�ǿ�״̬
	{
		*_data = soanar_que._data[soanar_que._tail];
		soanar_que._tail = (soanar_que._tail + 1) % QUEUE_MAX_SIZE;
	}
}

static qsize queue_size()
{
	return ((que._head + QUEUE_MAX_SIZE - que._tail) % QUEUE_MAX_SIZE);
}
static qsize soanar_queue_size()
{
	return ((soanar_que._head + QUEUE_MAX_SIZE - soanar_que._tail) % QUEUE_MAX_SIZE);
}

qsize queue_find_cmd(qdata *buffer, qsize buf_len)
{
	qsize cmd_size = 0;
	qdata _data = 0;
	
	while(queue_size() > 0)
	{
		//ȡһ������
		queue_pop(&_data);

		if(cmd_pos < buf_len)
			buffer[cmd_pos++] = _data;
		if(_data==0xee&&cmd_state ==0)
		{
			cmd_state = 1;
		}
		else if(_data==0xFF)
		{
			switch (cmd_state)
			{			
			case 3:cmd_state = 4;break; //FF FC FF ??
			case 4:cmd_state = 5;break; //FF FC FF FF
			case 1:cmd_state = 2;break; //FF ?? ?? ??
			default:cmd_state = 0;break;
			}
		}
		else if(_data==0xFC)
		{
			switch (cmd_state)
			{
			case 2:cmd_state = 3;break; //FF FC ?? ??
			case 4:cmd_state = 3;break; //FF FC FF FC
			case 1:cmd_state = 1;break; //?? ?? ?? ??			
			default:cmd_state = 0;break;
			}
		}
		else
		{
			//cmd_state = 0;
		}

		//�õ�������֡β
		if(cmd_state==5)
		{
			cmd_size = cmd_pos;
			cmd_state = 0;
			cmd_pos = 0;
			return cmd_size;
		}
	}

	return 0;
}
static qsize cmd_length = 0;

qsize soanar_queue_find_cmd(qdata *buffer,qsize buf_len)
{
    qsize cmd_size = 0;
    qdata _data = 0;
    while(soanar_queue_size()>0)
    {
        //ȡһ������
        soanar_queue_pop(&_data);

        if(soanar_cmd_pos==0&&_data!=0xFF)//֡ͷ��������
            continue;

        if(soanar_cmd_pos<buf_len)//��ֹ���������
            buffer[soanar_cmd_pos++] = _data;

        if(_data == 0xFF)
        {
        	switch(soanar_cmd_state)
        	{
        		case 0:
					soanar_cmd_state = 1;
					break;
        	}
					continue;
        }
				
		if(soanar_cmd_state == 1)
		{
			if (_data == 0xA2)
			{	
				soanar_cmd_state = 2;
			  cmd_length++;
			}
			else
			{			    
            soanar_cmd_state = 0;
            soanar_cmd_pos = 0;
		        cmd_length = 0;
			}
			continue;
		}		
		
		if(soanar_cmd_state == 2)
		{
			cmd_length++;
		}
		if(cmd_length >= 4)
		{
			soanar_cmd_state = 3;
			if(soanar_cmd_state==3)
        {
            cmd_size = soanar_cmd_pos;
            soanar_cmd_state = 0;
            soanar_cmd_pos = 0;
			      cmd_length = 0;
            return cmd_size;
        }		
		}
        
        //�õ�������֡β

    }

    return 0;//û���γ�������һ֡
}

