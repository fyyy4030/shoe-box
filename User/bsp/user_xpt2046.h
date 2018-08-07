#include "stm32f10x.h"


#define  xpt2046_SPI_CS_0     GPIO_ResetBits(GPIOD, GPIO_Pin_12) 
#define  xpt2046_SPI_CS_1     GPIO_SetBits(GPIOD, GPIO_Pin_12) 
#define  xpt2046_SPI_SCK_0    GPIO_ResetBits(GPIOG, GPIO_Pin_14) 
#define  xpt2046_SPI_SCK_1    GPIO_SetBits(GPIOG, GPIO_Pin_14) 
#define  xpt2046_SPI_MOSI_0   GPIO_ResetBits(GPIOG, GPIO_Pin_13) 
#define  xpt2046_SPI_MOSI_1   GPIO_SetBits(GPIOG, GPIO_Pin_13)

#define INT_IN_2046   GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6)

/* Һ������ṹ�� */
typedef	struct POINT 
{
   uint16_t x;		
   uint16_t y;
}Coordinate;

/*  У��ϵ���ṹ�� */
typedef struct Parameter 
{						
long double An,  			 //ע:sizeof(long double) = 8
            Bn,     
            Cn,   
            Dn,    
            En,    
            Fn,     
            Divider ;
}Parameter ;

extern Coordinate ScreenSample[4];
extern Coordinate DisplaySample[4];
extern Parameter touch_para ;
extern Coordinate  display ;

#define	CHX 	0x90 	//ͨ��Y+��ѡ�������	
#define	CHY 	0xd0	//ͨ��X+��ѡ�������


void xpt2046_Init(void);
int Touch_Calibrate(void);
void Palette_Init(void);
FunctionalState Get_touch_point(Coordinate * displayPtr,
                                Coordinate * screenPtr,
                                Parameter * para );
Coordinate *Read_2046_2(void);
void Palette_draw_point(uint16_t x, uint16_t y);


