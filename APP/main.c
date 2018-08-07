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
#include "ili9341_lcd.h"
#include "xpt2046.h"
#include "dht11.h"
#include "gy_906.h"
#include <stdio.h>
//delay���ﱨ���ԭ���ǣ�delay�����û��ʵ�ֵģ�������MDK�󱨡�
#include "delay.h"
#include "main.h"
#include "timer.h"
#include "malloc.h"	  
#include "GUI.h"
#include "GUIDemo.h"

//Э���
#include "onenet.h"

//�����豸
#include "net_device.h"

//������
DATA_STREAM dataStream[11];
unsigned char dataStreamLen = sizeof(dataStream) / sizeof(dataStream[0]);
							

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
	SMBus_Init();	/*GY 906 �������*/
	LED_Init();    //LEDָʾ�Ƴ�ʼ������
	USART1_Init(); //USART1���ڳ�ʼ������
	Lcd_GramScan(2);

	LCD_Clear(0, 0, 320, 240, BLUE);	 	//����Ϊ��ɫ����
	xpt2046_Init();			//��ʼ��xpt2046��IO�ڣ�ģ��SPI���ж�IO��	
	mem_init(); 			//��ʼ���ڲ��ڴ��
	
	TIM3_Int_Init(999,71);	//1KHZ ��ʱ��1ms 
	TIM6_Int_Init(999,719);	//10ms�ж�
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC,ENABLE);//ʹ��CRCʱ�ӣ�����STemWin����ʹ�� 
	
	//	while(Touch_Calibrate() !=0);
// 	WM_SetCreateFlags(WM_CF_MEMDEV);
//	GUI_Init();
////	Touch_MainTask();
//	MainTask();

//��ѭ��	
	while(1)	
	{			

	}
}
/********************************************END FILE********************/


