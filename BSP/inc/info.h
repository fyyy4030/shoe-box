#ifndef __INFO_H_
#define __INFO_H_
#include "stdint.h"

#define SSID_ADDRESS		0		//��ʼ��ַ����һ�����ݱ�ʾ���ȡ����59�ֽڡ�
#define PSWD_ADDRESS		60		//��ʼ��ַ����һ�����ݱ�ʾ���ȡ����59�ֽڡ�

#define DEVID_ADDRESS       120		//��ʼ��ַ����һ�����ݱ�ʾ���ȡ����19�ֽڡ�
#define AKEY_ADDRESS        140		//��ʼ��ַ����һ�����ݱ�ʾ���ȡ����59�ֽڡ�

#define WARNING_INFO_ADDR	200		

typedef struct
{
	float tempreture_1;
	float humidity_1;
	float tempreture_2;
	float humidity_2;

}DHT11_INFO;	

typedef struct
{
	float temp_ta;
	float temp_to;

}GY_906_INFO;

typedef enum
{
	TIMING_MAX_SWITCH= 0x01,
	HUMI_MAX_SWITCH = 0x02,
	TEMP_MAX_SWITCH = 0x04,
	TEMP_MIN_SWITCH = 0x08,
	
}SWITCH_FLAG;

typedef struct
{
	/*
	bit 0 ��ʱ ����
	bit 1 ʪ������ ����
	bit 2 �¶����� ����
	bit 3 �¶����� ����
	bit 4-7����
	*/
	uint8_t enable_flag;
	uint8_t timing;
	uint8_t max_humi;
	int8_t  max_temp;
	int8_t  min_temp;
	uint8_t reserve[2];
	uint8_t sum_crc;
}Warning_Info;

unsigned char Info_Check(void);

unsigned char Info_WifiLen(unsigned char sp);

_Bool Info_Read(void);

_Bool Info_Alter(char *info);

uint8_t Get_Warning_info(Warning_Info *info);
uint8_t Save_Warning_info(Warning_Info *info);
#endif 

