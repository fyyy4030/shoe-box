/****************************************Copyright (c)******************************************
**                                     ����ķ�Ƽ�(IKMSIK)   
**                                 �Ա�����: acmemcu.taobao.com
**                                 ������̳: 930ebbs.com
**
**--------------File Info-----------------------------------------------------------------------
** File name:		 main.c
** Last modified Date:          
** Last Version:		   
** Descriptions:		   			
**----------------------------------------------------------------------------------------------
** Created by:			FiYu
** Created date:		2018-1-20
** Version:			    1.0
** Descriptions:		����TFTLCD�������Դ���У׼���败����
**--------------------------------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "user_ili9341_lcd.h"
#include "user_xpt2046.h"
#include "dht11.h"
#include "gy_906.h"
#include <stdio.h>
//delay���ﱨ���ԭ���ǣ�delay�����û��ʵ�ֵģ�������MDK�󱨡�
#include "delay.h"
#include "main.h"

extern volatile uint8_t touch_flag;

void display_temp()
{
	DHT11_Data_TypeDef DHT11_Data;
	
	Read_DHT11(&DHT11_Data);
	
	LCD_DispStr(32,32,WHITE,"Temp:%d.%d'C Humi:%d.%d%%", DHT11_Data.temp_int,DHT11_Data.temp_deci, \
				DHT11_Data.humi_int,DHT11_Data.humi_deci);
	
	LCD_DispStr(32,44,WHITE,"Temp_TA:%0.2f", SMBus_ReadTemp(RAM_TA));
	LCD_DispStr(32,56,WHITE,"Temp_To1:%0.2f", SMBus_ReadTemp(RAM_TOBJ1));
}
/***************************************************************************
 * ��  �� : ������
 * ��  �� : ��
 * ����ֵ : ��
 **************************************************************************/
int main(void)
{ 
	//��ʼ��TFTLCD2.8����
	LCD_Init();	
	DHT11_GPIO_Config(); 
	SMBus_Init();
	LED_Init();    //LEDָʾ�Ƴ�ʼ������
	USART1_Init(); //USART1���ڳ�ʼ������
	USART3_Init(); //USART2���ڳ�ʼ������	
//	//ָ��λ����ʾһ����ɫ������ַ���
	Lcd_GramScan(2);
	//����Ϊ��ɫ����
	LCD_Clear(0, 0, 320, 240, BACKGROUND);	 

	
//  //��ʼ��xpt2046��IO�ڣ�ģ��SPI���ж�IO��
//  xpt2046_Init();
//  //��ʼ��xpt2046��IO�ڣ�ģ��SPI���ж�IO��
//  while(Touch_Calibrate() !=0);
 

  //��ѭ��	
	while(1)	
	{
		while(1)
		{		
			LED_Switch(LED_ON,LED_R|LED_G|LED_Y|LED_B);	   //�����������ĸ�ָʾ�ƣ���ʾ�������������
			ESP8266_Init();    //ESP8266��ʼ��
			ESP8266_DevLink(DEVICEID,APIKEY,20);    //��ƽ̨�����豸����
			LED_Switch(LED_OFF,LED_R|LED_G|LED_Y|LED_B); //Ϩ�𿪷����ĸ�ָʾ�ƣ���ʾ��������豸���ӣ��������ݷ���ѭ��
		
			while(1)
			{								
					if(!(ESP8266_CheckStatus(30)))    //���ESP8266ģ������״̬
					{
							ESP8266_SendDat();     //��ƽ̨��������
					}
					else
					{
							break;
					}
					display_temp();
			}
		}			

	}
}
/********************************************END FILE********************/


