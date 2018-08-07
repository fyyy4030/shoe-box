//Ӳ������
#include "hw_includes.h"

//Э���
#include "onenet.h"

//�����豸
#include "net_device.h"

//��������
#define HEART_TASK_PRIO		3
#define HEART_TASK_STK_SIZE		2048
ALIGN(RT_ALIGN_SIZE) unsigned char HEART_TASK_STK[HEART_TASK_STK_SIZE]; //
struct rt_thread HEART_Task_Handle;	//������

//����������
#define SENSOR_TASK_PRIO	6
#define SENSOR_TASK_STK_SIZE		1024
ALIGN(RT_ALIGN_SIZE) unsigned char SENSOR_TASK_STK[SENSOR_TASK_STK_SIZE]; //
struct rt_thread SENSOR_Task_Handle;	//������

//���ݷ�������
#define SEND_TASK_PRIO		7
#define SEND_TASK_STK_SIZE		3072
ALIGN(RT_ALIGN_SIZE) unsigned char SEND_TASK_STK[SEND_TASK_STK_SIZE]; //
struct rt_thread SEND_Task_Handle;	//������

//�����ʼ������
#define NET_TASK_PRIO		9 //
#define NET_TASK_STK_SIZE	2048
ALIGN(RT_ALIGN_SIZE) unsigned char NET_TASK_STK[NET_TASK_STK_SIZE]; //
struct rt_thread NET_Task_Handle;	//������

DHT11_INFO dht11Info;	/*DTH11*/							
GY_906_INFO gy906Info;	/*gy906*/	

//������
DATA_STREAM dataStream[] ={
								{"dth11-1_temp", &dht11Info.tempreture, TYPE_FLOAT, 1},
								{"dth11-1_humi", &dht11Info.humidity, TYPE_FLOAT, 1},
								{"gy-906_Ta", &gy906Info.temp_ta, TYPE_FLOAT, 1},
								{"gy-906_To", &gy906Info.temp_to, TYPE_FLOAT, 1},			
							};

unsigned char dataStreamLen = sizeof(dataStream) / sizeof(dataStream[0]);

						
							
#define NET_TIME	60			//�趨ʱ��--��λ��
unsigned short timerCount = 0;	//ʱ�����--��λ��

/*
************************************************************
*	�������ƣ�	NET_Task
*
*	�������ܣ�	�������ӡ�ƽ̨����
*
*	��ڲ�����	void���͵Ĳ���ָ��
*
*	���ز�����	��
*
*	˵����		���������������񡣻������������߼����������״̬������д�����״̬��Ȼ���������������
************************************************************
*/
void NET_Task(void *pdata)
{
	
	NET_DEVICE_IO_Init();													//�����豸IO��ʼ��
	NET_DEVICE_Reset();														//�����豸��λ
	NET_DEVICE_Set_DataMode(DEVICE_CMD_MODE);								//����Ϊ�����շ�ģʽ(����ESP8266Ҫ����AT�ķ��ػ���ƽ̨�·����ݵķ���)

	while(1)
	{
		
		if(!oneNetInfo.netWork && (checkInfo.NET_DEVICE_OK == DEV_OK))		//��û������ �� ����ģ���⵽ʱ
		{
			NET_DEVICE_Set_DataMode(DEVICE_CMD_MODE);						//����Ϊ�����շ�ģʽ
			
			if(!NET_DEVICE_Init())											//��ʼ�������豸������������
			{
				OneNet_DevLink(oneNetInfo.devID, oneNetInfo.apiKey);		//����ƽ̨
				
				if(oneNetInfo.netWork)
				{
					RTOS_TimeDly(40);
				}
				else
				{
					RTOS_TimeDly(100);
				}
			}
		}
		
		if(checkInfo.NET_DEVICE_OK == DEV_ERR) 								//�������豸δ�����
		{
			NET_DEVICE_Set_DataMode(DEVICE_CMD_MODE);						//����Ϊ�����շ�ģʽ
			
			if(timerCount >= NET_TIME) 										//����������ӳ�ʱ
			{
				NET_DEVICE_Reset();											//��λ�����豸
				timerCount = 0;												//�������ӳ�ʱ����
//				faultType = FAULT_NONE;										//��������־
			}
			
			if(!NET_DEVICE_SendCmd("AT\r\n", "OK"))							//�����豸���
			{
				LCD_DispStr(32,68,WHITE,"NET Device :Ok");

				UsartPrintf(USART_DEBUG, "NET Device :Ok\r\n");
				checkInfo.NET_DEVICE_OK = DEV_OK;							//��⵽�����豸�����
				NET_DEVICE_Set_DataMode(DEVICE_DATA_MODE);					//����Ϊ�����շ�ģʽ
			}
			else
				UsartPrintf(USART_DEBUG, "NET Device :Error\r\n");
		}
		
		RTOS_TimeDly(5);													//��������25ms
	
	}

}
/*
************************************************************
*	�������ƣ�	SEND_Task
*
*	�������ܣ�	�ϴ�����������
*
*	��ڲ�����	void���͵Ĳ���ָ��
*
*	���ز�����	��
*
*	˵����		���ݷ�������
************************************************************
*/
void SEND_Task(void *parameter)
{

	while(1)
	{
		
		OneNet_SendData(kTypeSimpleJsonWithoutTime, dataStreamLen);	//�ϴ����ݵ�ƽ̨
		
		RTOS_TimeDly(3000);				//15sһ��
		
	}

}
/*
************************************************************
*	�������ƣ�	HEART_Task
*
*	�������ܣ�	�������
*
*	��ڲ�����	void���͵Ĳ���ָ��
*
*	���ز�����	��
*
*	˵����		�������񡣷����������󲢵ȴ�������Ӧ�������趨ʱ����û����Ӧ������ƽ̨����
************************************************************
*/
void HEART_Task(void *pdata)
{

	while(1)
	{
		
		OneNet_HeartBeat();				//�������
		
		RTOS_TimeDly(4000);				//��������20s
	
	}

}

/*
************************************************************
*	�������ƣ�	SENSOR_Task
*
*	�������ܣ�	���������ݲɼ�����ʾ
*
*	��ڲ�����	void���͵Ĳ���ָ��
*
*	���ز�����	��
*
*	˵����		���������ݲɼ����񡣽�����Ӵ����������ݲɼ�����ȡ����ʾ
************************************************************
*/
void SENSOR_Task(void *pdata) //
{
	DHT11_Data_TypeDef DHT11_Data;
	LCD_Init();
	LCD_Clear(0, 0, 320, 240, BLACK);	 	//����Ϊ��ɫ����
	while(1)
	{
		Read_DHT11(&DHT11_Data);

		dht11Info.humidity = DHT11_Data.humi_int + DHT11_Data.humi_deci/100.0;
		dht11Info.tempreture = DHT11_Data.temp_int + DHT11_Data.temp_deci/100.0;	
		
		LCD_DispStr(32,32,WHITE,"Temp:%d.%d'C Humi:%d.%d%%", DHT11_Data.temp_int,DHT11_Data.temp_deci, \
					DHT11_Data.humi_int,DHT11_Data.humi_deci);
		
		gy906Info.temp_ta = SMBus_ReadTemp(RAM_TA);
		gy906Info.temp_to = SMBus_ReadTemp(RAM_TOBJ1);
		
		LCD_DispStr(32,44,WHITE,"Temp_TA:%0.2f", gy906Info.temp_ta);
		LCD_DispStr(32,56,WHITE,"Temp_To1:%0.2f",gy906Info.temp_to);
		
		RTOS_TimeDly(200); 										//��������500ms	
	}
	
}

int rt_application_init(void)
{
	if(rt_thread_init(&HEART_Task_Handle, "HEART", HEART_Task, RT_NULL,
						(unsigned char *)&HEART_TASK_STK[0], HEART_TASK_STK_SIZE, HEART_TASK_PRIO, 1) == RT_EOK)
		rt_thread_startup(&HEART_Task_Handle);	
						
	if(rt_thread_init(&SENSOR_Task_Handle, "SENSOR", SENSOR_Task, RT_NULL,
						(unsigned char *)&SENSOR_TASK_STK[0], SENSOR_TASK_STK_SIZE, SENSOR_TASK_PRIO, 1) == RT_EOK)
		rt_thread_startup(&SENSOR_Task_Handle);	
						
	if(rt_thread_init(&NET_Task_Handle, "NET", NET_Task, RT_NULL,
						(unsigned char *)&NET_TASK_STK[0], NET_TASK_STK_SIZE, NET_TASK_PRIO, 1) == RT_EOK)
		rt_thread_startup(&NET_Task_Handle);
						
	if(rt_thread_init(&SEND_Task_Handle, "SEND", SEND_Task, RT_NULL,
						(unsigned char *)&SEND_TASK_STK[0], SEND_TASK_STK_SIZE, SEND_TASK_PRIO, 1) == RT_EOK)
		rt_thread_startup(&SEND_Task_Handle);						
	return 0;
}

