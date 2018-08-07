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
#include "fontupd.h"
#include "GUI.h"
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
	NVIC_Configuration();												//�жϿ�������������

	LCD_GPIO_Init();													//��ʼ��TFTLCD2.8����
	
	LCD_Init();

	DHT11_GPIO_Config(); 												//DH11 ��ʪ��
	
	SMBus_Init();														/*GY 906 �������*/
	
	LED_Init();    														//LEDָʾ�Ƴ�ʼ������
	
	xpt2046_Init();														//��ʼ��xpt2046��IO�ڣ�ģ��SPI���ж�IO��	

	SPI_Flash_Init();
	
	font_init();
	
	IIC_Init();					//��DHT11��������
	
	Buzzer_Init();														//��������ʼ��
	
	Relay_Init();														//��ʼ���̵���
	
	Check_PowerOn(); 													//�ϵ��Լ�
	
	USART1_Init(115200); 												//USART1���ڳ�ʼ������	
	
//	TIME2_5PWM_Init(TIM3,9999,143);//Ƶ��Ϊ��72*10^6/(9999+1)/(143+1)=50Hz
//	TIM_SetCompare2(TIM3,4999);//�õ�ռ�ձ�Ϊ50%��pwm����
	TIME2_5PWM_Init(TIM3,99,35);//Ƶ��Ϊ��72*10^6/(99+1)/(35+1)=20KHz
	TIM_SetCompare2(TIM3,10);//�õ�ռ�ձ�Ϊ10%��pwm����
	
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
			//AT24C02_Clear(0, 255, 256);
			UsartPrintf(USART_DEBUG, " 1.ssid_pswd in EEPROM\r\n");
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
	
	Iwdg_Init(4, 1250); 	//64��Ƶ��ÿ��625�Σ�����1250�Σ�2s

	RTOS_TimerInit();
	
	UsartPrintf(USART_DEBUG, "3.Hardware init OK\r\n");		//��ʾ��ʼ�����
	mem_init(); 											//��ʼ���ڲ��ڴ��	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC,ENABLE);		//ʹ��CRCʱ�ӣ�����STemWin����ʹ�� 	
	LCD_Clear(0, 0, 320, 240, BLACK);
}

/*@}*/
