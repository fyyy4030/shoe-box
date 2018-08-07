/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	net_IO.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2016-11-23
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		�����豸����IO��
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

#include "stm32f10x.h"  //��Ƭ��ͷ�ļ�

#include "net_io.h"		//�����豸����IO

#include "delay.h"		//Ӳ������

#include <string.h>		//C��


NET_IO_INFO netIOInfo;


/**************************************************************************************
 * ��  �� : ��ʼ��ESP8266ģ���õ�������
 * ��  �� : ��
 * ����ֵ : ��
 **************************************************************************************/
void esp8266_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	//������GPIO
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOE, ENABLE); 

	//���õ�IO��PB10������3��TXD
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;                
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;           //�����������
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//���õ�IO��PB11������3��RXD
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;     //��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	 
	//���õ�IO��PE0��ESP8266��λ����RST
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;                
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;           //�������
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	//  //��ʼ״̬��ESP8266��λ�����ø�
	NET_DEVICE_RST_OFF;
}

/*
************************************************************
*	�������ƣ�	NET_IO_Init
*
*	�������ܣ�	��ʼ�������豸IO������
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		�ײ�������շ�����
************************************************************
*/
void NET_IO_Init(void)
{
	USART_InitTypeDef				USART_InitStructure;
	USART_ClockInitTypeDef  USART_ClockInitStructure;
	NVIC_InitTypeDef				NVIC_InitStructure;

	//�򿪴���3��ʱ��
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART3 , ENABLE); 

	esp8266_init();

	//���ô���3��Ӳ������
	USART_DeInit(USART3);                                            //������USART3�Ĵ�������Ϊȱʡֵ
	USART_InitStructure.USART_BaudRate = 115200;                      //���ô���2������Ϊ115200
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;      //����һ��֡�д�������λ 
	USART_InitStructure.USART_StopBits = USART_StopBits_1;           //���巢�͵�ֹͣλ��ĿΪ1
	USART_InitStructure.USART_Parity = USART_Parity_No;              //��żʧ��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;     //Ӳ��������ʧ��
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                     //���ͽ���ʹ��
	USART_Init(USART3, &USART_InitStructure);    

	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;      //ʱ�ӵ͵�ƽ�
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;            //����SCLK������ʱ������ļ���Ϊ�͵�ƽ
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_1Edge;          //ʱ�ӵ�һ�����ؽ������ݲ���
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;  //���һλ���ݵ�ʱ�����岻��SCLK���
	USART_ClockInit(USART3, &USART_ClockInitStructure); 

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);                   //ʹ��USART3�����ж�
	USART_Cmd(USART3, ENABLE);                                       //ʹ��USART3����

	//���ô���3�ж����ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;	               //����Ϊ����3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;          //��ռ���ȼ�Ϊ2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	             //�����ȼ�Ϊ3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                  //ʹ���ж�ͨ��
	NVIC_Init(&NVIC_InitStructure);

	
	NET_IO_ClearRecive();

}

/*
************************************************************
*	�������ƣ�	NET_IO_Send
*
*	�������ܣ�	��������
*
*	��ڲ�����	str����Ҫ���͵�����
*				len�����ݳ���
*
*	���ز�����	��
*
*	˵����		�ײ�����ݷ�������
*
************************************************************
*/
void NET_IO_Send(unsigned char *str, unsigned short len)
{

	unsigned short count = 0;
	
	for(; count < len; count++)											//����һ֡����
	{
		USART_SendData(NET_IO, *str++);
		while(USART_GetFlagStatus(NET_IO, USART_FLAG_TC) == RESET);
	}

}

/*
************************************************************
*	�������ƣ�	NET_IO_WaitRecive
*
*	�������ܣ�	�ȴ��������
*
*	��ڲ�����	��
*
*	���ز�����	REV_OK-�������		REV_WAIT-���ճ�ʱδ���
*
*	˵����		ѭ�����ü���Ƿ�������
************************************************************
*/
_Bool NET_IO_WaitRecive(void)
{

	if(netIOInfo.dataLen == 0) 						//������ռ���Ϊ0 ��˵��û�д��ڽ��������У�����ֱ����������������
		return REV_WAIT;
		
	if(netIOInfo.dataLen == netIOInfo.dataLenPre)	//�����һ�ε�ֵ�������ͬ����˵���������
	{
		netIOInfo.dataLen = 0;						//��0���ռ���
			
		return REV_OK;								//���ؽ�����ɱ�־
	}
		
	netIOInfo.dataLenPre = netIOInfo.dataLen;		//��Ϊ��ͬ
	
	return REV_WAIT;								//���ؽ���δ��ɱ�־

}

/*
************************************************************
*	�������ƣ�	NET_IO_ClearRecive
*
*	�������ܣ�	��ջ���
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void NET_IO_ClearRecive(void)
{

	netIOInfo.dataLen = 0;
	
	memset(netIOInfo.buf, 0, sizeof(netIOInfo.buf));

}

/*
************************************************************
*	�������ƣ�	USART3_IRQHandler
*
*	�������ܣ�	�����ж�
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void USART3_IRQHandler(void)
{	
	rt_interrupt_enter();

	if(USART_GetITStatus(NET_IO, USART_IT_RXNE) != RESET) //�����ж�
	{
		if(netIOInfo.dataLen >= sizeof(netIOInfo.buf))	netIOInfo.dataLen = 0; //��ֹ���ڱ�ˢ��
		netIOInfo.buf[netIOInfo.dataLen++] = NET_IO->DR;
		
		USART_ClearFlag(NET_IO, USART_FLAG_RXNE);
	}
	
	rt_interrupt_leave();
}
