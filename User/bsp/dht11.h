#include "stm32f10x.h"


#define HIGH  1
#define LOW   0

#define DHT11_CLK     	RCC_APB2Periph_GPIOB
#define DHT11_PIN     	GPIO_Pin_6                  
#define DHT11_PORT		GPIOB 

//���κ꣬��������������һ��ʹ��,����ߵ�ƽ��͵�ƽ
#define DHT11_DATA_OUT(a)	if (a)	\
					GPIO_SetBits(GPIOB,DHT11_PIN);\
					else		\
					GPIO_ResetBits(GPIOB,DHT11_PIN)
 //��ȡ���ŵĵ�ƽ
#define  DHT11_DATA_IN()	 GPIO_ReadInputDataBit(GPIOB,DHT11_PIN)

typedef struct
{
	uint8_t  humi_int;		  //ʪ�ȵ���������
	uint8_t  humi_deci;	 	//ʪ�ȵ�С������
	uint8_t  temp_int;	 	  //�¶ȵ���������
	uint8_t  temp_deci;	 	//�¶ȵ�С������
	uint8_t  check_sum;	 	//У���
		                 
}DHT11_Data_TypeDef;

void DHT11_GPIO_Config(void);
uint8_t Read_DHT11(DHT11_Data_TypeDef *DHT11_Data);
static uint8_t Read_Byte(void);

