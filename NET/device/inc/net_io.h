#ifndef _NET_IO_H_
#define _NET_IO_H_

typedef struct
{
	
	unsigned short dataLen;			//�������ݳ���
	unsigned short dataLenPre;		//��һ�εĳ������ݣ����ڱȽ�
	
	unsigned char buf[200];			//���ջ���

} NET_IO_INFO;

#define REV_OK		0	//������ɱ�־
#define REV_WAIT	1	//����δ��ɱ�־

#define NET_IO			USART3		//ESP8266 ����

#define NET_DEVICE_RST_OFF   GPIO_SetBits(GPIOE,GPIO_Pin_0)
#define NET_DEVICE_RST_ON    GPIO_ResetBits(GPIOE,GPIO_Pin_0)

extern NET_IO_INFO netIOInfo;


void NET_IO_Init(void);

void NET_IO_Send(unsigned char *str, unsigned short len);

_Bool NET_IO_WaitRecive(void);

void NET_IO_ClearRecive(void);


#endif
