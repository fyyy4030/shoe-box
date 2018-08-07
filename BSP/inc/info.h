#ifndef __INFO_H_
#define __INFO_H_


#define SSID_ADDRESS		0		//��ʼ��ַ����һ�����ݱ�ʾ���ȡ����59�ֽڡ�
#define PSWD_ADDRESS		60		//��ʼ��ַ����һ�����ݱ�ʾ���ȡ����59�ֽڡ�

#define DEVID_ADDRESS       120		//��ʼ��ַ����һ�����ݱ�ʾ���ȡ����19�ֽڡ�
#define AKEY_ADDRESS        140		//��ʼ��ַ����һ�����ݱ�ʾ���ȡ����59�ֽڡ�

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



unsigned char Info_Check(void);

unsigned char Info_WifiLen(unsigned char sp);

_Bool Info_Read(void);

_Bool Info_Alter(char *info);

#endif 

