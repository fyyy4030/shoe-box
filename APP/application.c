//Ӳ������
#include "hw_includes.h"

//Э���
#include "onenet.h"

//�����豸
#include "net_device.h"

#include "main_dispaly.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define IWDG_TASK_PRIO		2	//���Ź�����
#define TOUCH_TASK_PRIO		3	//��������
#define USART_TASK_PRIO		5	//��������
#define HEART_TASK_PRIO		7	//��������
#define TIMER_TASK_PRIO		9 	//��ʱ������	
#define FAULT_TASK_PRIO		11	//���ϴ�������
#define SENSOR_TASK_PRIO	13	//����������
#define SEND_TASK_PRIO		15	//���ݷ�������
#define NET_TASK_PRIO		17	//�����ʼ������
#define DISPLAY_TASK_PRIO	19	//��ʾ����
#define ALTER_TASK_PRIO		20  //


#define IWDG_TASK_STK_SIZE		256	//����ջ��С
#define ALTER_TASK_STK_SIZE		1024
#define TOUCH_TASK_STK_SIZE		1024 
#define USART_TASK_STK_SIZE		1024 
#define HEART_TASK_STK_SIZE		1024
#define TIMER_TASK_STK_SIZE		512
#define FAULT_TASK_STK_SIZE		1024
#define SENSOR_TASK_STK_SIZE	1024
#define SEND_TASK_STK_SIZE		2048
#define DISPLAY_TASK_STK_SIZE	2048
#define NET_TASK_STK_SIZE		1024

//���Ź�����
ALIGN(RT_ALIGN_SIZE) unsigned char IWDG_TASK_STK[IWDG_TASK_STK_SIZE];
struct rt_thread IWDG_Task_Handle;	//������

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

//��Ϣ��������
ALIGN(RT_ALIGN_SIZE) unsigned char ALTER_TASK_STK[ALTER_TASK_STK_SIZE]; //
struct rt_thread ALTER_Task_Handle;	//������

DHT11_INFO 		dht11Info = {30,40,30,40};	/*DTH11*/							
GY_906_INFO 	gy906Info = {30,30};		/*gy906*/	
LED_STATUS 		ledStatus = {0,0,0,0};
DEVICE_STATUS	deviceStatus = {18000,10,0,0};


Warning_Info warning_info =
{
	0x0f,
	5,
	60,
	60,
	20,
	0,
	0,	
	145
};
//������
DATA_STREAM dataStream[] ={
								{"led1", &ledStatus.Led1Sta, TYPE_BOOL, 1},
								{"led2", &ledStatus.Led2Sta, TYPE_BOOL, 1},
								{"led3", &ledStatus.Led3Sta, TYPE_BOOL, 1},
								{"led4", &ledStatus.Led4Sta, TYPE_BOOL, 1},
								{"fanner_key",&deviceStatus.motor_key, TYPE_BOOL, 1},
								{"relay_key",&deviceStatus.relay_key, TYPE_BOOL, 1},								
								{"fanner_rate",&deviceStatus.motor_rate, TYPE_USHORT, 1},
								{"timing_sec",&deviceStatus.timing_sec, TYPE_USHORT, 1},
								{"temp_max", &warning_info.max_temp, TYPE_CHAR, 1},
								{"temp_min", &warning_info.min_temp, TYPE_CHAR, 1},
								{"humi_max", &warning_info.max_humi, TYPE_UCHAR, 1},
								
																
								{"dth11-1_temp", &dht11Info.tempreture_1, TYPE_FLOAT, 1},
								{"dth11-1_humi", &dht11Info.humidity_1, TYPE_FLOAT, 1},
								{"dth11-2_temp", &dht11Info.tempreture_2, TYPE_FLOAT, 1},
								{"dth11-2_humi", &dht11Info.humidity_2, TYPE_FLOAT, 1},
								
								{"gy-906_Ta", &gy906Info.temp_ta, TYPE_FLOAT, 1},
								{"gy-906_To", &gy906Info.temp_to, TYPE_FLOAT, 1},			
							};

unsigned char dataStreamLen = sizeof(dataStream) / sizeof(dataStream[0]);

						
							
#define NET_TIME	60			//�趨ʱ��--��λ��
unsigned short timerCount = 0;	//ʱ�����--��λ��

							
//void prinf_task_stk_size()
//{
//	uint16_t size,i,j,index;
//	for(j = 0; j < 11; j++)
//	{
//		size = sizeof(IWDG_TASK_STK);
//		for(i = 0;i<size;i++)
//		{
//		
//		}
//	}
//}	
							
/*
************************************************************
*	�������ƣ�	IWDG_Task
*
*	�������ܣ�	������Ź�
*
*	��ڲ�����	void���͵Ĳ���ָ��
*
*	���ز�����	��
*
*	˵����		���Ź�����
************************************************************
*/
void IWDG_Task(void *pdata)
{

	while(1)
	{
	
		Iwdg_Feed(); 		//ι��
		
		RTOS_TimeDly(50); 	//��������250ms
	
	}

}

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
static void auto_adjust_temp()
{
	uint16_t temp_Average = 0;
	uint16_t humi_Average = 0;	
	
	if(fabs(dht11Info.tempreture_1 - dht11Info.tempreture_2) <= 20)
	{
		temp_Average = (dht11Info.tempreture_1 +dht11Info.tempreture_2)/2;
		if(warning_info.enable_flag & TEMP_MAX_SWITCH)
		{	
			if(warning_info.max_temp < temp_Average || warning_info.max_temp < gy906Info.temp_to)
			{
				RELAY1_OFF();
				RELAY2_OFF();
				SET_PWM_Arr(1);		
				deviceStatus.relay_key = 0;
				deviceStatus.motor_key = 0;
				deviceStatus.motor_rate = 1;
			}
		}
		
		if(warning_info.enable_flag & TEMP_MIN_SWITCH)
		{	
			if(warning_info.min_temp > temp_Average || warning_info.min_temp > gy906Info.temp_to)
			{
				RELAY1_ON();
				RELAY2_ON();
				SET_PWM_Arr(40);
				deviceStatus.relay_key = 1;
				deviceStatus.motor_key = 1;
				deviceStatus.motor_rate = 40;				
			}
		}		
	}
	
	if(fabs(dht11Info.humidity_1 - dht11Info.humidity_2) <= 30)
	{
		humi_Average = (dht11Info.humidity_1 + dht11Info.humidity_2)/2;		
		if(warning_info.enable_flag & HUMI_MAX_SWITCH)
		{
			if(warning_info.max_humi < humi_Average)
			{		
				RELAY2_ON();
				SET_PWM_Arr(60);	
				deviceStatus.motor_key = 1;
				deviceStatus.motor_rate = 60;				
			}				
		}
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
		
		if(deviceStatus.timing_sec != 0)
		{
			deviceStatus.timing_sec--;
		}
		else 
		{
			RELAY1_OFF();
			RELAY2_OFF();
			SET_PWM_Arr(1);
			warning_info.enable_flag = 0;
		}
		auto_adjust_temp();
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
		
		if(send_time >= 250)  //25sһ��
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
*	�������ƣ�	ALTER_Task
*
*	�������ܣ�	ͨ�����ڸ���SSID��PSWD��DEVID��APIKEY
*
*	��ڲ�����	void���͵Ĳ���ָ��
*
*	���ز�����	��
*
*	˵����		���ĺ�ᱣ�浽EEPROM��
************************************************************
*/
void ALTER_Task(void *pdata)
{
    
    unsigned char usart1Count = 0;

    while(1)
    {
    
        rt_memset(alterInfo.alterBuf, 0, sizeof(alterInfo.alterBuf));
		alterInfo.alterCount = 0;
		usart1Count = 0;
        while((rt_strlen(alterInfo.alterBuf) != usart1Count) || (usart1Count == 0))	//�ȴ��������
        {
            usart1Count = rt_strlen(alterInfo.alterBuf);							//���㳤��
            RTOS_TimeDly(20);														//ÿ100ms���һ��
        }
        UsartPrintf(USART_DEBUG, "\r\nusart1Buf Len: %d, usart1Count = %d\r\n",
									rt_strlen(alterInfo.alterBuf), usart1Count);
        
		if(checkInfo.EEPROM_OK == DEV_OK)											//���EEPROM����
		{
			if(Info_Alter(alterInfo.alterBuf))										//������Ϣ
			{
				oneNetInfo.netWork = 0;												//���������ƽ̨
				NET_DEVICE_ReConfig(0);
			}
		}
    
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
void display_sensor_info()
{
	GUI_SetColor(GUI_WHITE);
	GUI_SetFont(&GUI_FontHZ24);
	GUI_DispStringAt("ƤƤЬ��",112,6);
	GUI_SetFont(&GUI_FontHZ16);
	GUI_DispStringAt("�¶�1:",10,40+16);
	GUI_DispFloat(dht11Info.tempreture_1,5);
	GUI_DispString("'C");
	GUI_DispString("        ʪ��1:");
	GUI_DispDec(dht11Info.humidity_1,2);
	GUI_DispString("%");

	GUI_DispStringAt("�¶�2:",10,50+16*2);
	GUI_DispFloat(dht11Info.tempreture_2,5);
	GUI_DispString("'C");
	GUI_DispString("        ʪ��2:");
	GUI_DispDec(dht11Info.humidity_2,2);	
	GUI_DispString("%");

	GUI_DispStringAt("�����¶�:",10,60 + 16*3);
	GUI_DispFloat(gy906Info.temp_ta,5);
	GUI_DispString("'C");

	GUI_DispString("     �����¶�:");
	GUI_DispFloat(gy906Info.temp_to,5);	
	GUI_DispString("'C");

	GUI_DispStringAt("����ٶ�:",10,70+16*4);
	GUI_DispDec(deviceStatus.motor_rate,2);
	GUI_DispString("%");

	GUI_DispString("        �̵���:");
	if(deviceStatus.relay_key)
	{
		GUI_DispString("��");
	}
	else
	{
		GUI_DispString("��");
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
	GUI_Init();	
//	GUI_CURSOR_Show();
//	//����Ƥ��
	BUTTON_SetDefaultSkin(BUTTON_SKIN_FLEX); 
	CHECKBOX_SetDefaultSkin(CHECKBOX_SKIN_FLEX);
	DROPDOWN_SetDefaultSkin(DROPDOWN_SKIN_FLEX);
	FRAMEWIN_SetDefaultSkin(FRAMEWIN_SKIN_FLEX);
	HEADER_SetDefaultSkin(HEADER_SKIN_FLEX);
	MENU_SetDefaultSkin(MENU_SKIN_FLEX);
	MULTIPAGE_SetDefaultSkin(MULTIPAGE_SKIN_FLEX);
	PROGBAR_SetDefaultSkin(PROGBAR_SKIN_FLEX);
	RADIO_SetDefaultSkin(RADIO_SKIN_FLEX);
	SCROLLBAR_SetDefaultSkin(SCROLLBAR_SKIN_FLEX);
	SLIDER_SetDefaultSkin(SLIDER_SKIN_FLEX);
	SPINBOX_SetDefaultSkin(SPINBOX_SKIN_FLEX);	
	
//	WM_SetDesktopColor(GUI_WHITE); /* Automacally update desktop window */
	WM_SetCreateFlags(WM_CF_MEMDEV); /* Use memory devices on all windows to avoid flicker */	
	Main_Display_Create();
	
	while(1)
	{
		GUI_Delay(500); 

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
		
		if(Read_DHT11(&DHT11_Data,1))
		{
			dht11Info.humidity_1 = DHT11_Data.humi_int + DHT11_Data.humi_deci/100.0;
			dht11Info.tempreture_1 = DHT11_Data.temp_int + DHT11_Data.temp_deci/100.0;	
		}
		
		if(Read_DHT11(&DHT11_Data,2))
		{
			dht11Info.humidity_2 = DHT11_Data.humi_int + DHT11_Data.humi_deci/100.0;
			dht11Info.tempreture_2 = DHT11_Data.temp_int + DHT11_Data.temp_deci/100.0;	
		}
		gy906Info.temp_ta = SMBus_ReadTemp(RAM_TA);
		gy906Info.temp_to = SMBus_ReadTemp(RAM_TOBJ1);
		
		RTOS_TimeDly(200); 										//��������1000ms	
	}
	
}

int rt_application_init(void)
{
	if(rt_thread_init(&IWDG_Task_Handle, "IWDG", IWDG_Task, RT_NULL,
						(unsigned char *)&IWDG_TASK_STK[0], IWDG_TASK_STK_SIZE, IWDG_TASK_PRIO, 1) == RT_EOK)
		rt_thread_startup(&IWDG_Task_Handle);
						
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

	if(rt_thread_init(&DISPLAY_Task_Handle, "DISPLAY", DISPLAY_Task, RT_NULL,
						(unsigned char *)&DISPLAY_TASK_STK[0], DISPLAY_TASK_STK_SIZE, DISPLAY_TASK_PRIO, 1) == RT_EOK)
		rt_thread_startup(&DISPLAY_Task_Handle);
						
	if(rt_thread_init(&USART_Task_Handle, "USART", USART_Task, RT_NULL,
						(unsigned char *)&USART_TASK_STK[0], USART_TASK_STK_SIZE, USART_TASK_PRIO, 1) == RT_EOK)
		rt_thread_startup(&USART_Task_Handle);

	if(rt_thread_init(&TOUCH_Task_Handle, "TOUCH", TOUCH_Task, RT_NULL,
						(unsigned char *)&TOUCH_TASK_STK[0], TOUCH_TASK_STK_SIZE, TOUCH_TASK_PRIO, 1) == RT_EOK)
		rt_thread_startup(&TOUCH_Task_Handle);	

	if(rt_thread_init(&ALTER_Task_Handle, "ALTER", ALTER_Task, RT_NULL,
						(unsigned char *)&ALTER_TASK_STK[0], ALTER_TASK_STK_SIZE, ALTER_TASK_PRIO, 1) == RT_EOK)
		rt_thread_startup(&ALTER_Task_Handle);						
	return 0;
}

