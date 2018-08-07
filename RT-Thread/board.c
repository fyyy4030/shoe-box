/*
 * File      : board.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009 RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      first implementation
 * 2013-07-12     aozima       update for auto initial.
 */

#include <rthw.h>
#include <rtthread.h>

#include "stm32f10x.h"
#include "board.h"

#ifdef  RT_USING_COMPONENTS_INIT
#include <components.h>
#endif  /* RT_USING_COMPONENTS_INIT */

/**
 * @addtogroup STM32
 */
 
  //Ӳ������
 #include "hw_includes.h"
 
 //Э��
 #include "onenet.h"
 
  //�����豸
 #include "net_device.h"
 

/*@{*/

/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NVIC_Configuration(void)
{
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		//��������Ϊ2:2
	
}

/*
************************************************************
*	�������ƣ�	rt_hw_board_init
*
*	�������ܣ�	Ŀ�����������ʼ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void rt_hw_board_init(void)
{
	
	NVIC_Configuration();														//�жϿ�������������
	
	Delay_Init();																//systick��ʼ��
	
	Led_Init();																	//LED��ʼ��
	
	Key_Init();																	//������ʼ��
	
	Beep_Init();																//��������ʼ��
	
	IIC_Init();																	//���IIC���߳�ʼ��
	
	Lcd1602_Init();																//LCD1602��ʼ��
	
	Usart1_Init(115200); 														//��ʼ������   115200bps
	
	Lcd1602_DisString(0x80, "Check Power On");									//��ʾ���п������
	Check_PowerOn(); 															//�ϵ��Լ�
	Lcd1602_Clear(0x80);														//���һ����ʾ
	
	if(checkInfo.ADXL345_OK == DEV_OK) 											//�����⵽ADXL345���ʼ��
		ADXL345_Init();
	if(checkInfo.OLED_OK == DEV_OK)												//�����⵽OLED���ʼ��
	{
		OLED_Init();
		OLED_ClearScreen();														//����
	}
	
	if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) == SET) 								//����ǿ��Ź���λ����ʾ
	{
		UsartPrintf(USART_DEBUG, "WARN:	IWDG Reboot\r\n");
		
		RCC_ClearFlag();														//������Ź���λ��־λ
		
		faultTypeReport = faultType = FAULT_REBOOT; 							//���Ϊ��������
		
		if(!Info_Check() && checkInfo.EEPROM_OK)								//���EEPROM������Ϣ
			Info_Read();
	}
	else
	{
		//�ȶ���ssid��pswd��devid��apikey
		if(!Info_Check())														//���EEPROM������Ϣ
		{
			//AT24C02_Clear(0, 255, 256);Iwdg_Feed();
			UsartPrintf(USART_DEBUG, "1.ssid_pswd in EEPROM\r\n");
			Info_Read();
		}
		else //û������
		{
			UsartPrintf(USART_DEBUG, "1.ssid_pswd in ROM\r\n");
		}
		
		UsartPrintf(USART_DEBUG, "2.SSID: %s,   PSWD: %s\r\n"
								"DEVID: %s,     APIKEY: %s\r\n"
								, netDeviceInfo.staName, netDeviceInfo.staPass
								, oneNetInfo.devID, oneNetInfo.apiKey);
	}
	
	Iwdg_Init(4, 1250); 														//64��Ƶ��ÿ��625�Σ�����1250�Σ�2s
	RTOS_TimerInit();
	
	UsartPrintf(USART_DEBUG, "3.Hardware init OK\r\n");							//��ʾ��ʼ�����
	
}

/*@}*/
