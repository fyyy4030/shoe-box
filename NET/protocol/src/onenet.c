/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	onenet.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2016-12-07
	*
	*	�汾�� 		V1.1
	*
	*	˵���� 		��onenetƽ̨�����ݽ�����Э���
	*
	*	�޸ļ�¼��	V1.1�����˿�������������������д---��غ�����OneNet_SendData��OneNet_Load_DataStream
	*					  ���������ӷ�ʽ2---��غ�����OneNet_DevLink
	*					  ������PUSHDATA���ܣ��豸���豸֮���ͨ��---��غ�����OneNet_PushData
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//�����豸
#include "net_device.h"

//Э���ļ�
#include "onenet.h"

//Ӳ������
#include "usart1.h"

#include "delay.h"
#include "led.h"

#include "fault.h"
//#include "selfcheck.h" 

//ͼƬ�����ļ�
//#include "image_2k.h"

  //Ӳ������
#include "hw_includes.h"
//C��
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "EdpKit.h"


EdpPacket *send_pkg;	//Э���

ONETNET_INFO oneNetInfo = {"35852269", "LwithCI7=NoIXIplbjRdDxe118g=", 0, 0, 0, 0};
extern DATA_STREAM dataStream[];
extern LED_STATUS ledStatus;
extern DEVICE_STATUS deviceStatus;
/*
************************************************************
*	�������ƣ�	OneNet_DevLink
*
*	�������ܣ�	��onenet��������
*
*	��ڲ�����	devid�������豸��devid
*				auth_key�������豸��masterKey��apiKey
*
*	���ز�����	��
*
*	˵����		��onenetƽ̨�������ӣ��ɹ������oneNetInfo.netWork����״̬��־
************************************************************
*/
void OneNet_DevLink(const char* devid, const char* auth_key)
{

	unsigned char *dataPtr;
	
	UsartPrintf(USART_DEBUG, "OneNet_DevLink\r\n"
                        "DEVID: %s,     APIKEY: %s\r\n"
                        , devid, auth_key);

#if 1
	send_pkg = PacketConnect1(devid, auth_key);						//����devid �� apikey��װЭ���
#else
	send_pkg = PacketConnect2(devid, auth_key);						//���ݲ�Ʒid �� ��Ȩ��Ϣ��װЭ���
#endif
	
	NET_DEVICE_SendData(send_pkg->_data, send_pkg->_write_pos);		//�ϴ�ƽ̨
	
	dataPtr = NET_DEVICE_GetIPD(250);								//�ȴ�ƽ̨��Ӧ
	if(dataPtr != NULL)
	{
		oneNetInfo.netWork = OneNet_EDPKitCmd(dataPtr);				//��������
	}
	
	DeleteBuffer(&send_pkg);										//ɾ��
	
	if(oneNetInfo.netWork)											//�������ɹ�
	{
		UsartPrintf(USART_DEBUG, "Tips:	NetWork OK\r\n");
		
		oneNetInfo.errCount = 0;
		
		NET_DEVICE_Set_DataMode(DEVICE_DATA_MODE);					//����Ϊ�����շ�ģʽ
	}
	else
	{
		UsartPrintf(USART_DEBUG, "Tips:	NetWork Fail\r\n");
		
		if(++oneNetInfo.errCount >= 5)								//��������趨�����󣬻�δ����ƽ̨
		{
			oneNetInfo.netWork = 0;
			faultType = faultTypeReport = FAULT_NODEVICE;			//���ΪӲ������
		}
	}
	
}

/*
************************************************************
*	�������ƣ�	OneNet_PushData
*
*	�������ܣ�	PUSHDATA
*
*	��ڲ�����	dst_devid�������豸��devid
*				data����������
*				data_len�����ݳ���
*
*	���ز�����	��
*
*	˵����		�豸���豸֮���ͨ��
************************************************************
*/
_Bool OneNet_PushData(const char* dst_devid, const char* data, unsigned int data_len)
{
	
	if(!oneNetInfo.netWork || NET_DEVICE_Get_DataMode() != DEVICE_DATA_MODE)		//�������δ���� �� ��Ϊ�����շ�ģʽ
		return 1;
	
	NET_DEVICE_Set_DataMode(DEVICE_CMD_MODE);										//����Ϊ�����շ�ģʽ

	send_pkg = PacketPushdata(dst_devid, data, data_len);
	
	NET_DEVICE_SendData(send_pkg->_data, send_pkg->_write_pos);						//�ϴ�ƽ̨
	
	DeleteBuffer(&send_pkg);														//ɾ��
	
	NET_DEVICE_Set_DataMode(DEVICE_DATA_MODE);
	
	return 0;

}

/*
************************************************************
*	�������ƣ�	OneNet_Load_DataStream
*
*	�������ܣ�	��������װ
*
*	��ڲ�����	type���������ݵĸ�ʽ
*				send_buf�����ͻ���ָ��
*				len�������������ĸ���
*
*	���ز�����	��
*
*	˵����		��װ��������ʽ
************************************************************
*/
void OneNet_Load_DataStream(unsigned char type, char *send_buf, unsigned char len)
{
	
	unsigned char count = 0;
	char stream_buf[50];
	char *ptr = send_buf;

	switch(type)
	{
		case kTypeFullJson:
			
			UsartPrintf(USART_DEBUG, "Tips:	OneNet_SendData-kTypeFullJson\r\n");
		
			strcpy(send_buf, "{\"datastreams\":[");
			for(; count < len; count++)
			{
				if(dataStream[count].flag) //���ʹ�ܷ��ͱ�־λ
				{
					memset(stream_buf, 0, sizeof(stream_buf));
					switch((unsigned char)dataStream[count].dataType)
					{
						case TYPE_BOOL:
							sprintf(stream_buf, "{\"id\":\"%s\",\"datapoints\":[{\"value\":%d}]},", dataStream[count].name, *(_Bool *)dataStream[count].data);
						break;
						
						case TYPE_CHAR:
							sprintf(stream_buf, "{\"id\":\"%s\",\"datapoints\":[{\"value\":%d}]},", dataStream[count].name, *(char *)dataStream[count].data);
						break;
						
						case TYPE_UCHAR:
							sprintf(stream_buf, "{\"id\":\"%s\",\"datapoints\":[{\"value\":%d}]},", dataStream[count].name, *(unsigned char *)dataStream[count].data);
						break;
						
						case TYPE_SHORT:
							sprintf(stream_buf, "{\"id\":\"%s\",\"datapoints\":[{\"value\":%d}]},", dataStream[count].name, *(short *)dataStream[count].data);
						break;
						
						case TYPE_USHORT:
							sprintf(stream_buf, "{\"id\":\"%s\",\"datapoints\":[{\"value\":%d}]},", dataStream[count].name, *(unsigned short *)dataStream[count].data);
						break;
						
						case TYPE_INT:
							sprintf(stream_buf, "{\"id\":\"%s\",\"datapoints\":[{\"value\":%d}]},", dataStream[count].name, *(int *)dataStream[count].data);
						break;
						
						case TYPE_UINT:
							sprintf(stream_buf, "{\"id\":\"%s\",\"datapoints\":[{\"value\":%d}]},", dataStream[count].name, *(unsigned int *)dataStream[count].data);
						break;
						
						case TYPE_LONG:
							sprintf(stream_buf, "{\"id\":\"%s\",\"datapoints\":[{\"value\":%ld}]},", dataStream[count].name, *(long *)dataStream[count].data);
						break;
						
						case TYPE_ULONG:
							sprintf(stream_buf, "{\"id\":\"%s\",\"datapoints\":[{\"value\":%ld}]},", dataStream[count].name, *(unsigned long *)dataStream[count].data);
						break;
							
						case TYPE_FLOAT:
							sprintf(stream_buf, "{\"id\":\"%s\",\"datapoints\":[{\"value\":%f}]},", dataStream[count].name, *(float *)dataStream[count].data);
						break;
						
						case TYPE_DOUBLE:
							sprintf(stream_buf, "{\"id\":\"%s\",\"datapoints\":[{\"value\":%f}]},", dataStream[count].name, *(float *)dataStream[count].data);
						break;
						
						case TYPE_GPS:
							sprintf(stream_buf, "{\"id\":\"%s\",\"datapoints\":[{\"value\":{\"lon\":1.1,\"lat\":1.1}}]},", dataStream[count].name);
						break;
					}
					
					strcat(send_buf, stream_buf);
				}
			}
			
			while(*ptr != '\0')					//�ҵ�������
				ptr++;
			*(--ptr) = '\0';					//������','�滻Ϊ������
			
			strcat(send_buf, "]}");
		
			send_pkg = PacketSaveJson(NULL, send_buf, kTypeFullJson);				//���
		
		break;
		
		case kTypeSimpleJsonWithTime:
			
			UsartPrintf(USART_DEBUG, "Tips:	OneNet_SendData-kTypeSimpleJsonWithTime\r\n");
		
			strcpy(send_buf, "{");
			for(; count < len; count++)
			{
				if(dataStream[count].flag) //���ʹ�ܷ��ͱ�־λ
				{
					memset(stream_buf, 0, sizeof(stream_buf));
					switch((unsigned char)dataStream[count].dataType)
					{
						case TYPE_BOOL:
							sprintf(stream_buf, "\"%s\":{\"2016-08-10T12:31:17\":%d},", dataStream[count].name, *(_Bool *)dataStream[count].data);
						break;
						
						case TYPE_CHAR:
							sprintf(stream_buf, "\"%s\":{\"2016-08-10T12:31:17\":%d},", dataStream[count].name, *(char *)dataStream[count].data);
						break;
						
						case TYPE_UCHAR:
							sprintf(stream_buf, "\"%s\":{\"2016-08-10T12:31:17\":%d},", dataStream[count].name, *(unsigned char *)dataStream[count].data);
						break;
						
						case TYPE_SHORT:
							sprintf(stream_buf, "\"%s\":{\"2016-08-10T12:31:17\":%d},", dataStream[count].name, *(short *)dataStream[count].data);
						break;
						
						case TYPE_USHORT:
							sprintf(stream_buf, "\"%s\":{\"2016-08-10T12:31:17\":%d},", dataStream[count].name, *(unsigned short *)dataStream[count].data);
						break;
						
						case TYPE_INT:
							sprintf(stream_buf, "\"%s\":{\"2016-08-10T12:31:17\":%d},", dataStream[count].name, *(int *)dataStream[count].data);
						break;
						
						case TYPE_UINT:
							sprintf(stream_buf, "\"%s\":{\"2016-08-10T12:31:17\":%d},", dataStream[count].name, *(unsigned int *)dataStream[count].data);
						break;
						
						case TYPE_LONG:
							sprintf(stream_buf, "\"%s\":{\"2016-08-10T12:31:17\":%ld},", dataStream[count].name, *(long *)dataStream[count].data);
						break;
						
						case TYPE_ULONG:
							sprintf(stream_buf, "\"%s\":{\"2016-08-10T12:31:17\":%ld},", dataStream[count].name, *(unsigned long *)dataStream[count].data);
						break;
						
						case TYPE_FLOAT:
							sprintf(stream_buf, "\"%s\":{\"2016-08-10T12:31:17\":%f},", dataStream[count].name, *(float *)dataStream[count].data);
						break;
						
						case TYPE_DOUBLE:
							sprintf(stream_buf, "\"%s\":{\"2016-08-10T12:31:17\":%f},", dataStream[count].name, *(float *)dataStream[count].data);
						break;
					}
					
					strcat(send_buf, stream_buf);
				}
			}
			
			while(*ptr != '\0')					//�ҵ�������
				ptr++;
			*(--ptr) = '\0';					//������','�滻Ϊ������
			
			strcat(send_buf, "}");
			
			send_pkg = PacketSaveJson(NULL, send_buf, kTypeSimpleJsonWithTime);				//���
		
		break;
		
		case kTypeSimpleJsonWithoutTime:
			
			UsartPrintf(USART_DEBUG, "Tips:	OneNet_SendData-kTypeSimpleJsonWithoutTime\r\n");
		
			strcpy(send_buf, "{");
			for(; count < len; count++)
			{
				if(dataStream[count].flag) //���ʹ�ܷ��ͱ�־λ
				{
					memset(stream_buf, 0, sizeof(stream_buf));
					switch((unsigned char)dataStream[count].dataType)
					{
						case TYPE_BOOL:
							sprintf(stream_buf, "\"%s\":%d,", dataStream[count].name, *(_Bool *)dataStream[count].data);
						break;
							
						case TYPE_CHAR:
							sprintf(stream_buf, "\"%s\":%d,", dataStream[count].name, *(char *)dataStream[count].data);
						break;
							
						case TYPE_UCHAR:
							sprintf(stream_buf, "\"%s\":%d,", dataStream[count].name, *(unsigned char *)dataStream[count].data);
						break;
							
						case TYPE_SHORT:
							sprintf(stream_buf, "\"%s\":%d,", dataStream[count].name, *(short *)dataStream[count].data);
						break;
							
						case TYPE_USHORT:
							sprintf(stream_buf, "\"%s\":%d,", dataStream[count].name, *(unsigned short *)dataStream[count].data);
						break;
							
						case TYPE_INT:
							sprintf(stream_buf, "\"%s\":%d,", dataStream[count].name, *(int *)dataStream[count].data);
						break;
							
						case TYPE_UINT:
							sprintf(stream_buf, "\"%s\":%d,", dataStream[count].name, *(unsigned int *)dataStream[count].data);
						break;
							
						case TYPE_LONG:
							sprintf(stream_buf, "\"%s\":%ld,", dataStream[count].name, *(long *)dataStream[count].data);
						break;
							
						case TYPE_ULONG:
							sprintf(stream_buf, "\"%s\":%ld,", dataStream[count].name, *(unsigned long *)dataStream[count].data);
						break;
							
						case TYPE_FLOAT:
							sprintf(stream_buf, "\"%s\":%0.2f,", dataStream[count].name, *(float *)dataStream[count].data);
						break;
							
						case TYPE_DOUBLE:
							sprintf(stream_buf, "\"%s\":%f,", dataStream[count].name, *(float *)dataStream[count].data);
						break;
					}
					
					strcat(send_buf, stream_buf);
				}
			}
			
			while(*ptr != '\0')					//�ҵ�������
				ptr++;
			*(--ptr) = '\0';					//������','�滻Ϊ������
			
			strcat(send_buf, "}");
			
			send_pkg = PacketSaveJson(NULL, send_buf, kTypeSimpleJsonWithoutTime);			//���
		
		break;
		
		case kTypeString:
			
			UsartPrintf(USART_DEBUG, "Tips:	OneNet_SendData-kTypeString\r\n");
		
			strcpy(send_buf, ",;");
			for(; count < len; count++)
			{
				if(dataStream[count].flag) //���ʹ�ܷ��ͱ�־λ
				{
					memset(stream_buf, 0, sizeof(stream_buf));
					switch((unsigned char)dataStream[count].dataType)
					{
						case TYPE_BOOL:
							sprintf(stream_buf, "%s,%d;", dataStream[count].name, *(_Bool *)dataStream[count].data);
						break;
						
						case TYPE_CHAR:
							sprintf(stream_buf, "%s,%d;", dataStream[count].name, *(char *)dataStream[count].data);
						break;
						
						case TYPE_UCHAR:
							sprintf(stream_buf, "%s,%d;", dataStream[count].name, *(unsigned char *)dataStream[count].data);
						break;
						
						case TYPE_SHORT:
							sprintf(stream_buf, "%s,%d;", dataStream[count].name, *(short *)dataStream[count].data);
						break;
						
						case TYPE_USHORT:
							sprintf(stream_buf, "%s,%d;", dataStream[count].name, *(unsigned short *)dataStream[count].data);
						break;
						
						case TYPE_INT:
							sprintf(stream_buf, "%s,%d;", dataStream[count].name, *(int *)dataStream[count].data);
						break;
						
						case TYPE_UINT:
							sprintf(stream_buf, "%s,%d;", dataStream[count].name, *(unsigned int *)dataStream[count].data);
						break;
						
						case TYPE_LONG:
							sprintf(stream_buf, "%s,%ld;", dataStream[count].name, *(long *)dataStream[count].data);
						break;
						
						case TYPE_ULONG:
							sprintf(stream_buf, "%s,%ld;", dataStream[count].name, *(unsigned long *)dataStream[count].data);
						break;
						
						case TYPE_FLOAT:
							sprintf(stream_buf, "%s,%f;", dataStream[count].name, *(float *)dataStream[count].data);
						break;
						
						case TYPE_DOUBLE:
							sprintf(stream_buf, "%s,%f;", dataStream[count].name, *(float *)dataStream[count].data);
						break;
					}
				}
				
				strcat(send_buf, stream_buf);
			}
		
			send_pkg = PacketSavedataSimpleString(NULL, send_buf);							//���
		
		break;
	}

}

/*
************************************************************
*	�������ƣ�	OneNet_SendData
*
*	�������ܣ�	�ϴ����ݵ�ƽ̨
*
*	��ڲ�����	type���������ݵĸ�ʽ
*				len�������������ĸ���
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
_Bool OneNet_SendData(SaveDataType type, unsigned char len)
{
	
	char send_buf[SEND_BUF_SIZE];
	
	if(!oneNetInfo.netWork || NET_DEVICE_Get_DataMode() != DEVICE_DATA_MODE)		//�������δ���� �� ��Ϊ�����շ�ģʽ
		return 1;
	
	NET_DEVICE_Set_DataMode(DEVICE_CMD_MODE);										//����Ϊ�����շ�ģʽ
	memset(send_buf, 0, SEND_BUF_SIZE);
	
	if(type != kTypeBin)															//�������ļ���ȫ���������ã�����Ҫִ����Щ
	{
		OneNet_Load_DataStream(type, send_buf, len);								//����������
		NET_DEVICE_SendData(send_pkg->_data, send_pkg->_write_pos);					//�ϴ����ݵ�ƽ̨
//		UsartPrintf(USART_DEBUG, "%s \r\n",(char *)send_buf);
		DeleteBuffer(&send_pkg);													//ɾ��
		
		faultTypeReport = FAULT_NONE;												//����֮��������
	}
	else
	{
		UsartPrintf(USART_DEBUG, "Tips:	OneNet_SendData-kTypeBin\r\n");
		
//		OneNet_SendData_EDPType2();
	}
	
	NET_DEVICE_Set_DataMode(DEVICE_DATA_MODE);
	
	return 0;
	
}

/*
************************************************************
*	�������ƣ�	OneNet_SendData_EDPType2
*
*	�������ܣ�	�ϴ����������ݵ�ƽ̨
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		���ǵ����豸����������ʱ������ʹ�������豸��͸��ģʽ
************************************************************
*/
//#define PKT_SIZE 1024
//void OneNet_SendData_EDPType2(void)
//{

//	char text[] = "{\"ds_id\":\"pic\"}";								//ͼƬ����ͷ
//	unsigned int len = sizeof(Array);
//	unsigned char *pImage = (unsigned char *)Array;					
//	
//	UsartPrintf(USART_DEBUG, "image len = %d", len);
//	
//	send_pkg = PacketSaveBin(NULL, text, len);							//һ����ͼƬ���ݾ�HardFault_Handler��
//										//ԭ����������ʱ��д��devid��ƽ̨����ϴ���ͼƬ�������·�һ�Σ����´���buf������Ӷ�����HardFault_Handler
//	//send_pkg = PacketSaveBin(oneNetInfo.devID, text, len);//���ڴ��ڽ���������˷�ֹ�������Խ��Ĵ������²����ˡ�
//	
//	NET_DEVICE_SendData(send_pkg->_data, send_pkg->_write_pos);			//��ƽ̨�ϴ����ݵ�
//	
//	DeleteBuffer(&send_pkg);											//ɾ��
//	
//	while(len > 0)
//	{
//		RTOS_TimeDly(10);												//��ͼʱ��ʱ�������һ�㣬�����������һ����ʱ
//		
//		if(len >= PKT_SIZE)
//		{
//			NET_DEVICE_SendData(pImage, PKT_SIZE);						//���ڷ��ͷ�Ƭ
//			
//			pImage += PKT_SIZE;
//			len -= PKT_SIZE;
//		}
//		else
//		{
//			NET_DEVICE_SendData(pImage, (unsigned short)len);			//���ڷ������һ����Ƭ
//			len = 0;
//		}
//	}

//}

/*
************************************************************
*	�������ƣ�	OneNet_HeartBeat
*
*	�������ܣ�	�������
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void OneNet_HeartBeat(void)
{

	unsigned char heartBeat[2] = {PINGREQ, 0}, sCount = 5;
	unsigned char errType = 0;
	unsigned char *dataPtr;
	
	if(!oneNetInfo.netWork || NET_DEVICE_Get_DataMode() != DEVICE_DATA_MODE)	//�������Ϊ���� �� ��Ϊ�����շ�ģʽ
		return;
	
	NET_DEVICE_Set_DataMode(DEVICE_HEART_MODE);									//����Ϊ�����շ�ģʽ
	
	while(--sCount)																//ѭ��������
	{
		NET_DEVICE_SendData(heartBeat, sizeof(heartBeat));						//��ƽ̨�ϴ���������

		dataPtr = NET_DEVICE_GetIPD(200);										//��ȡ���ݣ��ȵ�2s
		if(dataPtr != NULL)														//�������ָ�벻Ϊ��
		{
			if(dataPtr[0] == PINGRESP)											//��һ������Ϊ������Ӧ
			{
				errType = CHECK_NO_ERR;											//����޴���
				UsartPrintf(USART_DEBUG, "Tips:	HeartBeat OK\r\n");
				break;
			}
		}
		else
		{
			if(dataPtr[0] != PINGRESP)											//����δ��Ӧ
			{
				UsartPrintf(USART_DEBUG, "Check Device\r\n");
					
				errType = NET_DEVICE_Check();									//�����豸״̬���
			}
		}
			
		RTOS_TimeDly(2);														//��ʱ�ȴ�
	}
	
	if(sCount == 0)																//����
	{
		UsartPrintf(USART_DEBUG, "HeartBeat TimeOut\r\n");
		
		errType = NET_DEVICE_Check();											//�����豸״̬���
	}
	
	if(errType == CHECK_CONNECTED || errType == CHECK_CLOSED || errType == CHECK_GOT_IP)
		faultTypeReport = faultType = FAULT_EDP;								//���ΪЭ�����
	else if(errType == CHECK_NO_DEVICE)
		faultTypeReport = faultType = FAULT_NODEVICE;							//���Ϊ�豸����
	else
		faultTypeReport = faultType = FAULT_NONE;								//�޴���
	
	NET_DEVICE_ClrData();														//�������
	
	NET_DEVICE_Set_DataMode(DEVICE_DATA_MODE);									//����Ϊ�����շ�ģʽ

}

/*
************************************************************
*	�������ƣ�	OneNet_EDPKitCmd
*
*	�������ܣ�	EDPЭ���������
*
*	��ڲ�����	data��ƽ̨�·�������
*
*	���ز�����	��
*
*	˵����		��ʱֻ�������ӽ���
************************************************************
*/
_Bool OneNet_EDPKitCmd(unsigned char *data)
{

	if(data[0] == CONNRESP) //������Ӧ
	{
		UsartPrintf(USART_DEBUG, "DevLink: %d\r\n", data[3]);
		
		//0		���ӳɹ�
		//1		��֤ʧ�ܣ�Э�����
		//2		��֤ʧ�ܣ��豸ID��Ȩʧ��
		//3		��֤ʧ�ܣ�������ʧ��
		//4		��֤ʧ�ܣ��û�ID��Ȩʧ��
		//5		��֤ʧ�ܣ�δ��Ȩ
		//6		��֤ʧ�ܣ�������δ����
		//7		��֤ʧ�ܣ����豸�ѱ�����
		//8		��֤ʧ�ܣ��ظ��������������
		//9		��֤ʧ�ܣ��ظ��������������
		
		if(data[3] == 0)
			return 1;
		else
			return 0;
	}
	
	return 0;

}

/*
************************************************************
*	�������ƣ�	OneNet_Replace
*
*	�������ܣ�	�滻ƽ̨�·������еĽ�����
*
*	��ڲ�����	res��ƽ̨�·�������
*
*	���ز�����	��
*
*	˵����		ƽ̨���ء���ť�·���������й̶�4�����������滻Ϊ�����������ַ����Է���string�����
************************************************************
*/
void OneNet_Replace(unsigned char *res, unsigned char num)
{
	
	unsigned char count = 0;

	while(count < num)
	{
		if(*res == '\0')		//�ҵ�������
		{
			*res = '~';			//�滻
			count++;
		}
		
		res++;
	}

}

/*
************************************************************
*	�������ƣ�	OneNet_App
*
*	�������ܣ�	ƽ̨�·��������������
*
*	��ڲ�����	cmd��ƽ̨�·�������
*
*	���ز�����	��
*
*	˵����		��ȡ�������Ӧ����
************************************************************
*/
void OneNet_App(unsigned char *cmd)
{

	char *dataPtr;
	char numBuf[10];
	int num = 0;
	
	if(cmd[0] == PUSHDATA)								//pushdata���ܽ��յ�����
		OneNet_Replace(cmd, 1);
	else
		OneNet_Replace(cmd, 4);
	
	dataPtr = strstr((const char *)cmd, ":");			//����'}'
	
	UsartPrintf(USART_DEBUG,"Tips:	OneNet Recvice: %s\r\n",&cmd[40]);
	
	if(dataPtr != NULL)									//����ҵ���
	{
		dataPtr++;
		
		while(*dataPtr >= '0' && *dataPtr <= '9')		//�ж��Ƿ����·��������������
		{
			numBuf[num++] = *dataPtr++;
		}
		
		num = atoi((const char *)numBuf);				//תΪ��ֵ��ʽ
		
		if(strstr((char *)cmd, "Rled_1"))					//����"redled"
		{
			if(num == 1)								//�����������Ϊ1������
			{
				LED_1_ON();
				ledStatus.Led1Sta = 1;
			}
			else if(num == 0)							//�����������Ϊ0�������
			{
				LED_1_OFF();
				ledStatus.Led1Sta = 0;				
			}
			
			oneNetInfo.sendData = 1;					//������ݷ���
		}
														//��ͬ
		else if(strstr((char *)cmd, "Rled_2"))
		{
			if(num == 1)
			{
				LED_2_ON();	
				ledStatus.Led2Sta = 1;					
			}
			else if(num == 0)
			{
				LED_2_OFF();	
				ledStatus.Led2Sta = 0;					
			}
			
			oneNetInfo.sendData = 1;
		}
		else if(strstr((char *)cmd, "Rled_3"))
		{
			if(num == 1)
			{
				LED_3_ON();	
				ledStatus.Led3Sta = 1;					
			}
			else if(num == 0)
			{
				LED_3_OFF();
				ledStatus.Led3Sta = 0;					
			}
			
			oneNetInfo.sendData = 1;
		}
		else if(strstr((char *)cmd, "Rled_4"))
		{
			if(num == 1)
			{
				LED_4_ON();				
				ledStatus.Led4Sta = 1;					
			}
			else if(num == 0)
			{
				LED_4_OFF();		
				ledStatus.Led4Sta = 0;					
			}
			
			oneNetInfo.sendData = 1;
		}
		else if(strstr((char *)cmd, "Rrelay_key"))
		{
			if(num == 1)
			{
				RELAY1_ON();				
				deviceStatus.relay_key = 1;					
			}
			else if(num == 0)
			{
				RELAY1_OFF();		
				deviceStatus.relay_key = 0;						
			}
			oneNetInfo.sendData = 1;
		}
		else if(strstr((char *)cmd, "Rfanner_key"))
		{
			if(num == 1)
			{
				deviceStatus.motor_key = 1;
				RELAY2_ON();					
			}
			else if(num == 0)
			{
				deviceStatus.motor_key = 0;
				RELAY2_OFF();					
			}
			
			oneNetInfo.sendData = 1;
		}
		else if(strstr((char *)cmd, "Rfanner_rate"))
		{
			if(num > 0 && num< 99)
			{
				deviceStatus.motor_rate = num;
				SET_PWM_Arr(num);
			}
			oneNetInfo.sendData = 1;
		}		
	}
	
	NET_DEVICE_ClrData();								//��ջ���
}
