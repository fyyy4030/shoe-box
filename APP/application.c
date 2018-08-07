//Ӳ������
#include "hw_includes.h"

//Э���
#include "onenet.h"

//�����豸
#include "net_device.h"

#include "GUI.h"
#include "GUIDemo.h"

#define TOUCH_TASK_PRIO		3	//��������
#define USART_TASK_PRIO		5	//��������
#define HEART_TASK_PRIO		7	//��������
#define TIMER_TASK_PRIO		9 	//��ʱ������	
#define FAULT_TASK_PRIO		11	//���ϴ�������
#define SENSOR_TASK_PRIO	13	//����������
#define SEND_TASK_PRIO		15	//���ݷ�������
#define NET_TASK_PRIO		17	//�����ʼ������
#define DISPLAY_TASK_PRIO	19	//��ʾ����

#define TOUCH_TASK_STK_SIZE		1024 
#define USART_TASK_STK_SIZE		2048 
#define HEART_TASK_STK_SIZE		2048
#define TIMER_TASK_STK_SIZE		1024
#define FAULT_TASK_STK_SIZE		2048
#define SENSOR_TASK_STK_SIZE	1024
#define SEND_TASK_STK_SIZE		3072
#define DISPLAY_TASK_STK_SIZE	2048
#define NET_TASK_STK_SIZE		2048

//��������
ALIGN(RT_ALIGN_SIZE) unsigned char TOUCH_TASK_STK[TOUCH_TASK_STK_SIZE]; //
struct rt_thread TOUCH_Task_Handle;	//������

//��������
ALIGN(RT_ALIGN_SIZE) unsigned char USART_TASK_STK[USART_TASK_STK_SIZE]; //
struct rt_thread USART_Task_Handle;	//������

//��������
ALIGN(RT_ALIGN_SIZE) unsigned char HEART_TASK_STK[HEART_TASK_STK_SIZE]; //
struct rt_thread HEART_Task_Handle;	//������

//��ʱ������
ALIGN(RT_ALIGN_SIZE) unsigned char TIMER_TASK_STK[TIMER_TASK_STK_SIZE]; //
struct rt_thread TIMER_Task_Handle;	//������

//���ϴ�������
ALIGN(RT_ALIGN_SIZE) unsigned char  FAULT_TASK_STK[FAULT_TASK_STK_SIZE]; //
struct rt_thread FAULT_Task_Handle;	//������

//����������
ALIGN(RT_ALIGN_SIZE) unsigned char SENSOR_TASK_STK[SENSOR_TASK_STK_SIZE]; //
struct rt_thread SENSOR_Task_Handle;	//������

//���ݷ�������
ALIGN(RT_ALIGN_SIZE) unsigned char SEND_TASK_STK[SEND_TASK_STK_SIZE]; //
struct rt_thread SEND_Task_Handle;	//������

//��ʾ����
ALIGN(RT_ALIGN_SIZE) unsigned char DISPLAY_TASK_STK[DISPLAY_TASK_STK_SIZE]; //
struct rt_thread DISPLAY_Task_Handle;//������

//�����ʼ������
ALIGN(RT_ALIGN_SIZE) unsigned char NET_TASK_STK[NET_TASK_STK_SIZE]; //
struct rt_thread NET_Task_Handle;	//������


DHT11_INFO 		dht11Info;	/*DTH11*/							
GY_906_INFO 	gy906Info;	/*gy906*/	
LED_STATUS 		ledStatus = {0,0,0,0};
DEVICE_STATUS	deviceStatus = {10,1};
//������
DATA_STREAM dataStream[] ={
								{"led1", &ledStatus.Led1Sta, TYPE_BOOL, 1},
								{"led2", &ledStatus.Led2Sta, TYPE_BOOL, 1},
								{"led3", &ledStatus.Led3Sta, TYPE_BOOL, 1},
								{"led4", &ledStatus.Led4Sta, TYPE_BOOL, 1},
								{"fanner_key", &deviceStatus.motor_key, TYPE_BOOL, 1},
								{"relay_key", &deviceStatus.relay_key, TYPE_BOOL, 1},								
								{"fanner_rate", &deviceStatus.motor_rate, TYPE_USHORT, 1},
																
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
				faultType = FAULT_NONE;										//��������־
			}
			
			if(!NET_DEVICE_SendCmd("AT\r\n", "OK"))							//�����豸���
			{
//				LCD_DispStr(32,68,WHITE,"NET Device :Ok");

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
*	�������ƣ�	TIMER_Task
*
*	�������ܣ�	��ʱ�������״̬��־λ
*
*	��ڲ�����	void���͵Ĳ���ָ��
*
*	���ز�����	��
*
*	˵����		��ʱ�����񡣶�ʱ�������״̬�������������趨ʱ�����������ӣ������ƽ̨����
************************************************************
*/
void TIMER_Task(void *pdata)
{

	while(1)
	{
	
		if(oneNetInfo.netWork == 0)											//�������Ͽ�
		{
			if(++timerCount >= NET_TIME) 									//�������Ͽ���ʱ
			{
				UsartPrintf(USART_DEBUG, "Tips:		Timer Check Err\r\n");	
				
				checkInfo.NET_DEVICE_OK = 0;								//���豸δ����־
				
				NET_DEVICE_ReConfig(0);										//�豸��ʼ����������Ϊ��ʼ״̬
				
				oneNetInfo.netWork = 0;
			}
		}
		else
		{
			timerCount = 0;													//�������
		}
		
		RTOS_TimeDly(200);													//��������1s
	
	}

}

/*
************************************************************
*	�������ƣ�	FAULT_Task
*
*	�������ܣ�	����״̬������
*
*	��ڲ�����	void���͵Ĳ���ָ��
*
*	���ز�����	��
*
*	˵����		���ϴ������񡣵�������������豸����ʱ�����Ƕ�Ӧ��־λ��Ȼ���н��д���
************************************************************
*/
void FAULT_Task(void *pdata)
{

	while(1)
	{
		
		if(faultType != FAULT_NONE)									//��������־������
		{
			UsartPrintf(USART_DEBUG, "WARN:	Fault Process\r\n");
			Fault_Process();										//�����������
		}
		
		RTOS_TimeDly(10);											//��������50ms
	
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
	uint8_t send_time = 0;
	while(1)
	{
		if(oneNetInfo.sendData == 1)		//������ݷ�����־λ������
		{
			oneNetInfo.sendData = OneNet_SendData(kTypeSimpleJsonWithoutTime, dataStreamLen);	//�ϴ����ݵ�ƽ̨
			if(oneNetInfo.sendData == 0)
			{
				send_time = 0;
			}
		}
		send_time++;
		
		if(send_time >= 150)  //15sһ��
		{
			send_time = 0;
			OneNet_SendData(kTypeSimpleJsonWithoutTime, dataStreamLen);	//�ϴ����ݵ�ƽ̨
		}
		RTOS_TimeDly(20);	//100ms			
		
	}

}

/*
************************************************************
*	�������ƣ�	USART_Task
*
*	�������ܣ�	����ƽ̨�·�������
*
*	��ڲ�����	void���͵Ĳ���ָ��
*
*	���ز�����	��
*
*	˵����		���ڽ�������������ģʽ��ʱ���ȴ�ƽ̨�·����������������
************************************************************
*/
void USART_Task(void *pdata)
{
	
	unsigned char *dataPtr;														//����ָ��

	while(1)
	{
		if(oneNetInfo.netWork && NET_DEVICE_Get_DataMode() == DEVICE_DATA_MODE)	//�������� �� ���������ģʽʱ
		{
			dataPtr = NET_DEVICE_GetIPD(0);										//���ȴ�����ȡƽ̨�·�������
			if(dataPtr != NULL)													//�������ָ�벻Ϊ�գ�������յ�������
			{
				OneNet_App(dataPtr);											//���д���
			}
		}
		
		RTOS_TimeDly(2);														//��������10ms
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
*	�������ƣ�	TOUCH_Task
*
*	�������ܣ�	����
*
*	��ڲ�����	void���͵Ĳ���ָ��
*
*	���ز�����	��
*
*	˵����		����
************************************************************
*/
void TOUCH_Task(void *pdata)
{
	while(1)
	{
		GUI_TOUCH_Exec();
		rt_thread_delay(2);
	}
}

/*
************************************************************
*	�������ƣ�	DISPLAY_Task
*
*	�������ܣ�	��ʾ
*
*	��ڲ�����	void���͵Ĳ���ָ��
*
*	���ز�����	��
*
*	˵����		��ʾ
************************************************************
*/
void DISPLAY_Task(void *pdata)
{
 	WM_SetCreateFlags(WM_CF_MEMDEV);
	GUI_Init();		
	while(1)
	{
		GUIDEMO_Main(); 
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
	while(1)
	{
		deviceStatus.motor_rate = Read_PWM_Arr();
		deviceStatus.relay_key = Read_Relay_Status(1);	
		deviceStatus.motor_key = Read_Relay_Status(2);
		deviceStatus.buzzer_key =Read_Buzzer_Status();
		
		ledStatus.Led1Sta = Read_LED_Status(1);
		ledStatus.Led2Sta = Read_LED_Status(2);
		ledStatus.Led3Sta = Read_LED_Status(3);
		ledStatus.Led4Sta = Read_LED_Status(4);
		
		Read_DHT11(&DHT11_Data);
		
		dht11Info.humidity = DHT11_Data.humi_int + DHT11_Data.humi_deci/100.0;
		dht11Info.tempreture = DHT11_Data.temp_int + DHT11_Data.temp_deci/100.0;	
				
		gy906Info.temp_ta = SMBus_ReadTemp(RAM_TA);
		gy906Info.temp_to = SMBus_ReadTemp(RAM_TOBJ1);

		LCD_DispStr(32,32,WHITE,"Temp:%d.%d'C Humi:%d.%d%%", DHT11_Data.temp_int,DHT11_Data.temp_deci, \
					DHT11_Data.humi_int,DHT11_Data.humi_deci);
					
		LCD_DispStr(32,44,WHITE,"Temp_TA:%0.2f", gy906Info.temp_ta);
		LCD_DispStr(32,56,WHITE,"Temp_To1:%0.2f",gy906Info.temp_to);
		
		RTOS_TimeDly(200); 										//��������1000ms	
	}
	
}

int rt_application_init(void)
{	
	if(rt_thread_init(&NET_Task_Handle, "NET", NET_Task, RT_NULL,
						(unsigned char *)&NET_TASK_STK[0], NET_TASK_STK_SIZE, NET_TASK_PRIO, 1) == RT_EOK)
		rt_thread_startup(&NET_Task_Handle);
						
	if(rt_thread_init(&HEART_Task_Handle, "HEART", HEART_Task, RT_NULL,
						(unsigned char *)&HEART_TASK_STK[0], HEART_TASK_STK_SIZE, HEART_TASK_PRIO, 1) == RT_EOK)
		rt_thread_startup(&HEART_Task_Handle);
						
	if(rt_thread_init(&FAULT_Task_Handle, "FAULT", FAULT_Task, RT_NULL,
						(unsigned char *)&FAULT_TASK_STK[0], FAULT_TASK_STK_SIZE, FAULT_TASK_PRIO, 1) == RT_EOK)
		rt_thread_startup(&FAULT_Task_Handle);						
						
	if(rt_thread_init(&SENSOR_Task_Handle, "SENSOR", SENSOR_Task, RT_NULL,
						(unsigned char *)&SENSOR_TASK_STK[0], SENSOR_TASK_STK_SIZE, SENSOR_TASK_PRIO, 1) == RT_EOK)
		rt_thread_startup(&SENSOR_Task_Handle);	

	if(rt_thread_init(&TIMER_Task_Handle, "TIMER", TIMER_Task, RT_NULL,
						(unsigned char *)&TIMER_TASK_STK[0], TIMER_TASK_STK_SIZE, TIMER_TASK_PRIO, 1) == RT_EOK)
		rt_thread_startup(&TIMER_Task_Handle);						
						
	if(rt_thread_init(&SEND_Task_Handle, "SEND", SEND_Task, RT_NULL,
						(unsigned char *)&SEND_TASK_STK[0], SEND_TASK_STK_SIZE, SEND_TASK_PRIO, 1) == RT_EOK)
		rt_thread_startup(&SEND_Task_Handle);

//	if(rt_thread_init(&SEND_Task_Handle, "DISPLAY", DISPLAY_Task, RT_NULL,
//						(unsigned char *)&DISPLAY_TASK_STK[0], DISPLAY_TASK_STK_SIZE, DISPLAY_TASK_PRIO, 1) == RT_EOK)
//		rt_thread_startup(&SEND_Task_Handle);
						
	if(rt_thread_init(&USART_Task_Handle, "USART", USART_Task, RT_NULL,
						(unsigned char *)&USART_TASK_STK[0], USART_TASK_STK_SIZE, USART_TASK_PRIO, 1) == RT_EOK)
		rt_thread_startup(&USART_Task_Handle);

	if(rt_thread_init(&TOUCH_Task_Handle, "TOUCH", TOUCH_Task, RT_NULL,
						(unsigned char *)&TOUCH_TASK_STK[0], TOUCH_TASK_STK_SIZE, TOUCH_TASK_PRIO, 1) == RT_EOK)
		rt_thread_startup(&TOUCH_Task_Handle);							
	return 0;
}

