#ifndef LED_H_H
#define LED_H_H
#include "stm32f10x.h"
#include "sys.h"
#define  LED_1_OFF()    GPIO_SetBits(GPIOG,GPIO_Pin_6)
#define  LED_2_OFF()    GPIO_SetBits(GPIOA,GPIO_Pin_8)
#define  LED_3_OFF()    GPIO_SetBits(GPIOB,GPIO_Pin_8)
#define  LED_4_OFF()    GPIO_SetBits(GPIOB,GPIO_Pin_9)

#define  LED_1_ON()     GPIO_ResetBits(GPIOG,GPIO_Pin_6)
#define  LED_2_ON()     GPIO_ResetBits(GPIOA,GPIO_Pin_8)
#define  LED_3_ON()     GPIO_ResetBits(GPIOB,GPIO_Pin_8)
#define  LED_4_ON()     GPIO_ResetBits(GPIOB,GPIO_Pin_9)

#define LED_1_KEY 	PGout(6)
#define LED_2_KEY 	PAout(8)
#define LED_3_KEY 	PBout(8)
#define LED_4_KEY 	PBout(9)

typedef struct
{
	_Bool Led1Sta;
	_Bool Led2Sta;
	_Bool Led3Sta;
	_Bool Led4Sta;

} LED_STATUS;
void LED_Init(void);
uint8_t Read_LED_Status(uint8_t led_index);
#endif
