#ifndef _SELFCHECK_H_
#define _SELFCHECK_H_







typedef struct
{

	unsigned short DHT11_OK : 1;		//��ʪ�ȴ�����������־λ
	unsigned short EEPROM_OK : 1;		//�洢��������־λ
	unsigned short GY906_OK : 1;		//
	unsigned short LCD_OK : 1;			//LCD������־
	unsigned short NET_DEVICE_OK : 1;	//�����豸������־λ

} CHECK_INFO;

#define DEV_OK		1
#define DEV_ERR		0

extern CHECK_INFO checkInfo;



void Check_PowerOn(void);


#endif
