#include "stm32f10x.h"
#include "dht11.h"
#include "sw_delay.h"
#include "sys.h"

#define DHT11_Mode_Out_PP()
#define DHT11_Mode_IPU()

static uint8_t dht11_index = 0;

 /**************************************************************************************
 * ��  �� : ��ʼ��DHT11�ź�����Ϊ���
 * ��  �� : ��
 * ����ֵ : ��
 **************************************************************************************/
void DHT11_GPIO_Config(void)
{		
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(DHT11_CLK, ENABLE); 
													   
	GPIO_InitStructure.GPIO_Pin = DHT11_PIN1|DHT11_PIN2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;   //��©���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(DHT11_PORT, &GPIO_InitStructure);		  

	GPIO_SetBits(DHT11_PORT, DHT11_PIN1|DHT11_PIN2);	 
}

static uint8_t DHT11_DATA_IN()
{
	if(dht11_index == 1)
	{
		return GPIO_ReadInputDataBit(DHT11_PORT,DHT11_PIN1);
	}
	else 
	{
		return GPIO_ReadInputDataBit(DHT11_PORT,DHT11_PIN2);		
	}
}
void DHT11_DATA_OUT(uint8_t dat)
{
	if(dht11_index == 1)
	{
//		PBout(DHT11_PIN1) = dat;
		if(dat)
			GPIO_SetBits(DHT11_PORT,DHT11_PIN1);
		else
			GPIO_ResetBits(DHT11_PORT,DHT11_PIN1);	
	}
	else 
	{
//		PBout(DHT11_PIN2) = dat;
		if(dat)
			GPIO_SetBits(DHT11_PORT,DHT11_PIN2);
		else
			GPIO_ResetBits(DHT11_PORT,DHT11_PIN2);	
	}	
}
 /**************************************************************************************
 * ��  �� : ��DHT11��ȡһ���ֽڣ�MSB����
 * ��  �� : ��
 * ����ֵ : uint8_t
 **************************************************************************************/
static uint8_t Read_Byte(void) 
{
	uint8_t i, temp=0;

	for(i=0;i<8;i++)    
	{	   
		while(DHT11_DATA_IN()==Bit_RESET);  //ÿbit��50us�͵�ƽ���ÿ�ʼ����ѯֱ���ӻ����� ��50us �͵�ƽ ����

		/*DHT11 ��26~28us�ĸߵ�ƽ��ʾ��0������70us�ߵ�ƽ��ʾ��1��  */
		sw_delay_us(40); //��ʱ40us,�����ʱ��Ҫ��������0������ʱ�伴��	   	  

		if(DHT11_DATA_IN() == Bit_SET)  //  40us����Ϊ�ߵ�ƽ��ʾ���ݡ�1��
		{
			while(DHT11_DATA_IN()==Bit_SET);  //�ȴ�����1�ĸߵ�ƽ����
			temp|=(uint8_t)(0x01<<(7-i));  //�ѵ�7-iλ��1��MSB���� 
		}
		else	 // 40us��Ϊ�͵�ƽ��ʾ���ݡ�0��
		{			   
			temp&=(uint8_t)~(0x01<<(7-i)); //�ѵ�7-iλ��0��MSB����
		}
	}
	return temp;
}

 /**************************************************************************************
 * ��  �� : һ�����������ݴ���Ϊ40bit����λ�ȳ�
 * ��  �� : 8bit ʪ������ + 8bit ʪ��С�� + 8bit �¶����� + 8bit �¶�С�� + 8bit У��� 
 * ����ֵ : uint8_t
 **************************************************************************************/
uint8_t Read_DHT11(DHT11_Data_TypeDef *DHT11_Data,uint8_t index)
{
	uint8_t timeout = 80;
	DHT11_Mode_Out_PP();   //���ģʽ
	dht11_index = index;
	DHT11_DATA_OUT(LOW);   //��������
	sw_delay_ms(18);       //��ʱ18ms
	
	rt_enter_critical();
	DHT11_DATA_OUT(HIGH);  //�������� ������ʱ30us
	sw_delay_us(30);   	   //��ʱ30us

	DHT11_Mode_IPU();   	//������Ϊ���� �жϴӻ���Ӧ�ź�
   
	if(DHT11_DATA_IN()== Bit_RESET)  //�жϴӻ��Ƿ��е͵�ƽ��Ӧ�ź� �粻��Ӧ����������Ӧ����������   
	{  
		while(DHT11_DATA_IN() == Bit_RESET)  //��ѯֱ���ӻ����� ��80us �͵�ƽ ��Ӧ�źŽ���
		{
			if(timeout == 0)
			{
				rt_exit_critical();
				return ERROR;
			}
			timeout --;
			sw_delay_us(1);
		}
		timeout = 80;
		while(DHT11_DATA_IN() == Bit_SET)    //��ѯֱ���ӻ������� 80us �ߵ�ƽ �����źŽ���
		{
			if(timeout == 0)
			{
				rt_exit_critical();
				return ERROR;
			}
			timeout --;
			sw_delay_us(1);
		}
		/*��ʼ��������*/   
		DHT11_Data->humi_int= Read_Byte();
		DHT11_Data->humi_deci= Read_Byte();
		DHT11_Data->temp_int= Read_Byte();
		DHT11_Data->temp_deci= Read_Byte();
		DHT11_Data->check_sum= Read_Byte();
		rt_exit_critical();
		
		DHT11_Mode_Out_PP();   //��ȡ���������Ÿ�Ϊ���ģʽ
		DHT11_DATA_OUT(HIGH);   //��������

		/*����ȡ�������Ƿ���ȷ*/
		if(DHT11_Data->check_sum == DHT11_Data->humi_int + DHT11_Data->humi_deci + DHT11_Data->temp_int+ DHT11_Data->temp_deci)
		{	
			return SUCCESS;
		}
		else 
			return ERROR;
	}
	else
	{
		rt_exit_critical();	
		return ERROR;
	}   
}
