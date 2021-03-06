#include "stm32f10x.h"
#include "ili9341_lcd.h"
#include "xpt2046.h"
#include "sw_delay.h"
#include "delay.h"

volatile uint8_t touch_flag;

/* 触摸屏校正系数 */
#if 1
long double aa1=0,bb1=0,cc1=0,aa2=0,bb2=0,cc2=0;
#elif 0
long double aa1=0.088370,\
            bb1=-0.000468,\
            cc1=-24.042172,\
            aa2=0.0001891,\
            bb2=0.062395,\
            cc2=-10.223455;
#endif

/* 差值门限 */
#define THRESHOLD 2 

/*--------------------------------------------------------------------*/
// 四个重要的结构体变量

/* 触摸采样AD值保存结构体 */
Coordinate ScreenSample[4];

Coordinate DisplaySample[4] =   
{
    { 45,  35 },
    { 10,  200},
    { 290, 200},
    { 200, 35}
};
/* 用于保存校正系数 */
Parameter   touch_para ;

/* 液晶计算坐标，用于真正画点的时候用 */
Coordinate  display ;
/*------------------------------------------------------------------*/

/**************************************************************************************
 * 描  述 : 初始化xpt2046用SPI所用到的IO口
 * 入  参 : 无
 * 返回值 : 无
 **************************************************************************************/
void SPI_GPIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	//打开所用GPIO的时钟
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOG , ENABLE);  

	//配置的IO是PG13 PG14，SPI的 MOSI CLK
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;                
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;           //推挽输出
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	//配置的IO是PD12，SPI的 CS
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;                
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;           //推挽输出
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	//配置的IO是PG15，SPI的MISO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;              //上拉输入
	GPIO_Init(GPIOG, &GPIO_InitStructure);
}

/*******************************************************************************
 * 描  述 : 初始化xpt2046用PD6为外部中断6
 * 参  数 : 无
 * 返回值 : 无
 ******************************************************************************/
void EXTI_PD6_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable the EXTI_PD6 Clock */
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOD , ENABLE); 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;                 //上拉输入   
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource6);   //配置PD6管脚为外部中断线路用

	EXTI_InitStructure.EXTI_Line = EXTI_Line6;                    //配置为外部中断线6
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;           //配置为中断请求
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;       //输入线路下降沿为中断请求
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;	                    //使能中断
	EXTI_Init(&EXTI_InitStructure);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);               //NVIC_Group:先占优先级2位，从优先级2位  
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;            //配置为外部中断6中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;     //先占优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;            //从优先级为2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;               //使能中断通道
	NVIC_Init(&NVIC_InitStructure); 
}

/**************************************************************************************
 * 描  述 : 外部中断服务程序
 * 入  参 : 无
 * 返回值 : 无
 **************************************************************************************/
void EXTI9_5_IRQHandler(void)
{
	//do something...
	rt_interrupt_enter();	
	
	if(EXTI_GetITStatus(EXTI_Line6) != RESET)
	{	
		touch_flag=1;
		EXTI_ClearITPendingBit(EXTI_Line6);  //清除EXTI中断线路挂起位
	}
	
	/* leave interrupt */
	rt_interrupt_leave();	
}


/*******************************************************************************
 * 描  述 : 初始化xpt2046用IO口（模拟SPI、中断IO）
 * 参  数 : 无
 * 返回值 : 无
 ******************************************************************************/
void xpt2046_Init(void)
{
   SPI_GPIO_Init();
   EXTI_PD6_Init();  
}

/*******************************************************************************
 * 描  述 : 写命令
 * 参  数 : 控制命令字
 * 返回值 : 无
 ******************************************************************************/
static void XPT2046_WriteCMD(uint8_t cmd) 
{
  uint8_t temp,i;
  xpt2046_SPI_MOSI_0;
  xpt2046_SPI_SCK_0;
  for (i = 0; i < 8; i++) 
  {
     temp = (cmd>>(7-i))&0x1; 
     if(temp)
     { xpt2046_SPI_MOSI_1; }
     else 
     { xpt2046_SPI_MOSI_0; }
     sw_delay_us(5);
     xpt2046_SPI_SCK_1 ;
     sw_delay_us(5);
     xpt2046_SPI_SCK_0;
  }
}

/*******************************************************************************
 * 描  述 : 读命令
 * 参  数 : 无
 * 返回值 : 返回读到的内容
 ******************************************************************************/
static unsigned short XPT2046_ReadCMD(void) 
{
    unsigned short buf=0,temp;
    unsigned char i,SDI;
    xpt2046_SPI_MOSI_0;
    xpt2046_SPI_SCK_1 ;
    for(i=0;i<12;i++) 
    {
        xpt2046_SPI_SCK_0; 
        SDI = GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_15);   //MISO接收数据
        temp= (SDI) ? 1:0;
        buf|=(temp<<(11-i));
        xpt2046_SPI_SCK_1 ;
    }
    buf&=0x0fff;

    return(buf);
}

/*******************************************************************************
 * 描  述 : 选择一个模拟通道，启动ADC，并返回ADC采样结果
 * 参  数 : _ucCh = 0x90 表示Y通道； 0xd0 表示X通道
 * 返回值 : 12位ADC值
 ******************************************************************************/
uint16_t XPT2046_ReadAdc(uint8_t _ucCh)
{
	uint16_t adc_value;
	rt_enter_critical();
	
	XPT2046_WriteCMD(_ucCh);
	adc_value = XPT2046_ReadCMD();
	
	rt_exit_critical();	
	return adc_value;
}


/*
 * 校正触摸时画十字专用 
 * x:0~300
 * y:0~230
 */
void DrawCross(uint16_t x,uint16_t y)
{
   LCD_Clear(x, y, 20, 1, RED);
   LCD_Clear(x+10, y-10, 1, 20, RED);
}

/*
 * 读取TP x y 的AD值(12bit，最大是4096)
 */
void Touch_GetAdXY(int *x,int *y)  
{ 
    int adx,ady; 
    //adx=Read_X();
    adx = XPT2046_ReadAdc(CHX);
  
    sw_delay_us(1); 
    //ady=Read_Y();
    ady = XPT2046_ReadAdc(CHY);  
    *x=adx; 
    *y=ady; 
}
//读取一个坐标值(x或者y)
//连续读取READ_TIMES次数据,对这些数据升序排列,
//然后去掉最低和最高LOST_VAL个数,取平均值 
//xy:指令（CMD_RDX/CMD_RDY）
//返回值:读到的数据
#define READ_TIMES 5 	//读取次数
#define LOST_VAL 1	  	//丢弃值
uint16_t TP_Read_XOY(uint8_t xy)
{
	uint16_t i, j;
	uint16_t buf[READ_TIMES];
	uint16_t sum=0;
	uint16_t temp;
	for(i=0;i<READ_TIMES;i++)
		buf[i]=XPT2046_ReadAdc(xy);		 		    
	for(i=0;i<READ_TIMES-1; i++)//排序
	{
		for(j=i+1;j<READ_TIMES;j++)
		{
			if(buf[i]>buf[j])//升序排列
			{
				temp=buf[i];
				buf[i]=buf[j];
				buf[j]=temp;
			}
		}
	}	  
	sum=0;
	for(i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)
		sum+=buf[i];
	
	temp=sum/(READ_TIMES-2*LOST_VAL);
	return temp;   
}
/******************************************************
* 函数名：Read_2046
* 描述  ：得到滤波之后的X Y
* 输入  : 无
* 输出  ：Coordinate结构体地址
* 举例  ：无
* 注意  ：速度相对比较慢
*********************************************************/    
Coordinate *Read_2046(void)
{
    static Coordinate  screen;
    int m0,m1,m2,TP_X[1],TP_Y[1],temp[3];
    uint8_t count=0;
		
		/* 坐标X和Y进行9次采样*/
    int buffer[2][9]={{0},{0}};  	
    do
    {		   
        Touch_GetAdXY(TP_X,TP_Y);  
        buffer[0][count]=TP_X[0];  
        buffer[1][count]=TP_Y[0];
        count++; 
				 
    }	/*用户点击触摸屏时即TP_INT_IN信号为低 并且 count<9*/
    while(!INT_IN_2046&& count<9);  
    
		
		/*如果触笔弹起*/
		if(INT_IN_2046 ==1)
    {
        /*中断标志复位*/
				touch_flag =0;						
    }

		/* 如果成功采样9次,进行滤波 */ 
    if(count==9)   								
    {  
        /* 为减少运算量,分别分3组取平均值 */
        temp[0]=(buffer[0][0]+buffer[0][1]+buffer[0][2])/3;
        temp[1]=(buffer[0][3]+buffer[0][4]+buffer[0][5])/3;
        temp[2]=(buffer[0][6]+buffer[0][7]+buffer[0][8])/3;
        
        /* 计算3组数据的差值 */
        m0=temp[0]-temp[1];
        m1=temp[1]-temp[2];
        m2=temp[2]-temp[0];
        
        /* 对上述差值取绝对值 */
        m0=m0>0?m0:(-m0);
        m1=m1>0?m1:(-m1);
        m2=m2>0?m2:(-m2);
        
        /* 判断绝对差值是否都超过差值门限，如果这3个绝对差值都超过门限值，则判定这次采样点为野点,抛弃采样点，差值门限取为2 */
        if( m0>THRESHOLD  &&  m1>THRESHOLD  &&  m2>THRESHOLD ) 
            return 0;
        
        /* 计算它们的平均值，同时赋值给screen */ 
        if(m0<m1)
        {
            if(m2<m0) 
                screen.x=(temp[0]+temp[2])/2;
            else 
                screen.x=(temp[0]+temp[1])/2;	
        }
        else if(m2<m1) 
            screen.x=(temp[0]+temp[2])/2;
        else 
            screen.x=(temp[1]+temp[2])/2;
        
        /* 同上 计算Y的平均值 */
        temp[0]=(buffer[1][0]+buffer[1][1]+buffer[1][2])/3;
        temp[1]=(buffer[1][3]+buffer[1][4]+buffer[1][5])/3;
        temp[2]=(buffer[1][6]+buffer[1][7]+buffer[1][8])/3;
        m0=temp[0]-temp[1];
        m1=temp[1]-temp[2];
        m2=temp[2]-temp[0];
        m0=m0>0?m0:(-m0);
        m1=m1>0?m1:(-m1);
        m2=m2>0?m2:(-m2);
        if(m0>THRESHOLD&&m1>THRESHOLD&&m2>THRESHOLD) 
            return 0;
        
        if(m0<m1)
        {
            if(m2<m0) 
                screen.y=(temp[0]+temp[2])/2;
            else 
                screen.y=(temp[0]+temp[1])/2;	
        }
        else if(m2<m1) 
            screen.y=(temp[0]+temp[2])/2;
        else
            screen.y=(temp[1]+temp[2])/2;
        
        return &screen;
    }
    
    else if(count>1)
    {
        screen.x=buffer[0][0];
        screen.y=buffer[1][0];
        return &screen;
    }  
    return 0; 
}

/******************************************************
* 函数名：Read_2046
* 描述  ：得到简单滤波之后的X Y
* 输入  : 无
* 输出  ：Coordinate结构体地址
* 举例  ：无
* 注意  ：”画板应用实例"专用,不是很精准，但是速度比较快
*********************************************************/    
Coordinate *Read_2046_2(void)
{
    static Coordinate  screen2;
    int TP_X[1],TP_Y[1];
    uint8_t count=0;
    int buffer[2][10]={{0},{0}};  /*坐标X和Y进行多次采样*/
    int min_x,max_x;
    int min_y,max_y;
    int	i=0;
    
    do					       				
    {		/* 循环采样10次 */   
        Touch_GetAdXY(TP_X,TP_Y);  
        buffer[0][count]=TP_X[0];  
        buffer[1][count]=TP_Y[0];
        count++;  
    }	/*用户点击触摸屏时即TP_INT_IN信号为低 并且 count<10*/
    while(!INT_IN_2046&& count<10);
    
		/*如果触笔弹起*/
    if(INT_IN_2046)						
    {
				/*中断标志复位*/
        touch_flag = 0;					 
    }
		
		/*如果成功采样10个样本*/
    if(count ==10)		 					
    {
        max_x=min_x=buffer[0][0];
        max_y=min_y=buffer[1][0];       
        for(i=1; i<10; i++)
        {
            if(buffer[0][i]<min_x)
            {
                min_x=buffer[0][i];
            }
            else
            if(buffer[0][i]>max_x)
            {
                max_x = buffer[0][i];
            }
        }
        
        for(i=1; i<10; i++)
        {
            if(buffer[1][i]<min_y)
            {
                min_y=buffer[1][i];
            }
            else
            if(buffer[1][i]>max_y)
            {
                max_y = buffer[1][i];
            }
        }
				/*去除最小值和最大值之后求平均值*/
        screen2.x=(buffer[0][0]+buffer[0][1]+buffer[0][2]+buffer[0][3]+buffer[0][4]+buffer[0][5]+buffer[0][6]+buffer[0][7]+buffer[0][8]+buffer[0][9]-min_x-max_x)>>3;
        screen2.y=(buffer[1][0]+buffer[1][1]+buffer[1][2]+buffer[1][3]+buffer[1][4]+buffer[1][5]+buffer[1][6]+buffer[1][7]+buffer[1][8]+buffer[1][9]-min_y-max_y)>>3; 
        
        return &screen2;
    }    
    return 0;    
}

/******************************************************
* 函数名：Cal_touch_para
* 描述  ：计算出触摸屏到液晶屏坐标变换的转换函数的 K A B C D E F系数
* 输入  : 无
* 输出  ：返回1表示成功 0失败
* 举例  ：无
* 注意  ：只有在LCD和触摸屏间的误差角度非常小时,才能运用下面公式
*********************************************************/    
FunctionalState Cal_touch_para( Coordinate * displayPtr,
                                Coordinate * screenPtr,
                                Parameter * para)
{
    
    FunctionalState retTHRESHOLD = ENABLE ;

    /* K＝(X0－X2) (Y1－Y2)－(X1－X2) (Y0－Y2) */
    para->Divider = ((screenPtr[0].x - screenPtr[2].x) * (screenPtr[1].y - screenPtr[2].y)) - 
                    ((screenPtr[1].x - screenPtr[2].x) * (screenPtr[0].y - screenPtr[2].y)) ;
    
		if( para->Divider == 0 )
    {
        retTHRESHOLD = DISABLE;
    }
    else
    {
        /* A＝((XD0－XD2) (Y1－Y2)－(XD1－XD2) (Y0－Y2))／K	*/
        para->An = ((displayPtr[0].x - displayPtr[2].x) * (screenPtr[1].y - screenPtr[2].y)) - 
                   ((displayPtr[1].x - displayPtr[2].x) * (screenPtr[0].y - screenPtr[2].y));
        
        /* B＝((X0－X2) (XD1－XD2)－(XD0－XD2) (X1－X2))／K	*/
        para->Bn = ((screenPtr[0].x - screenPtr[2].x) * (displayPtr[1].x - displayPtr[2].x)) - 
                   ((displayPtr[0].x - displayPtr[2].x) * (screenPtr[1].x - screenPtr[2].x));
        
        /* C＝(Y0(X2XD1－X1XD2)+Y1(X0XD2－X2XD0)+Y2(X1XD0－X0XD1))／K */
        para->Cn = (screenPtr[2].x * displayPtr[1].x - screenPtr[1].x * displayPtr[2].x) * screenPtr[0].y +
                   (screenPtr[0].x * displayPtr[2].x - screenPtr[2].x * displayPtr[0].x) * screenPtr[1].y +
                   (screenPtr[1].x * displayPtr[0].x - screenPtr[0].x * displayPtr[1].x) * screenPtr[2].y ;
        
        /* D＝((YD0－YD2) (Y1－Y2)－(YD1－YD2) (Y0－Y2))／K	*/
        para->Dn = ((displayPtr[0].y - displayPtr[2].y) * (screenPtr[1].y - screenPtr[2].y)) - 
                   ((displayPtr[1].y - displayPtr[2].y) * (screenPtr[0].y - screenPtr[2].y)) ;
        
        /* E＝((X0－X2) (YD1－YD2)－(YD0－YD2) (X1－X2))／K	*/
        para->En = ((screenPtr[0].x - screenPtr[2].x) * (displayPtr[1].y - displayPtr[2].y)) - 
                   ((displayPtr[0].y - displayPtr[2].y) * (screenPtr[1].x - screenPtr[2].x)) ;
        
        
        /* F＝(Y0(X2YD1－X1YD2)+Y1(X0YD2－X2YD0)+Y2(X1YD0－X0YD1))／K */
        para->Fn = (screenPtr[2].x * displayPtr[1].y - screenPtr[1].x * displayPtr[2].y) * screenPtr[0].y +
                   (screenPtr[0].x * displayPtr[2].y - screenPtr[2].x * displayPtr[0].y) * screenPtr[1].y +
                   (screenPtr[1].x * displayPtr[0].y - screenPtr[0].x * displayPtr[1].y) * screenPtr[2].y;
        
    }
    return( retTHRESHOLD ) ;
}

/******************************************************
* 函数名：Touchl_Calibrate
* 描述  ：触摸屏校正函数
* 输入  : 无
* 输出  ：0	---	校正成功
					1	---	校正失败
* 举例  ：无
* 注意  ：无
*********************************************************/ 
int Touch_Calibrate(void)
{
    #if 1
    uint8_t i;
    uint16_t test_x=0, test_y=0;
    uint16_t gap_x=0, gap_y=0;
    Coordinate * Ptr;   
    
    for(i=0; i<4; i++)
    {        
        LCD_Clear(0, 0, 320, 240, BACKGROUND);        
        LCD_DispStr(110, 110,RED, "Touch Calibrate......");	         
        LCD_DisNum(160, 90, i+1, RED);
      
        /* 适当的延时很有必要 */        
        sw_delay_ms(500);     
        DrawCross(DisplaySample[i].x,DisplaySample[i].y);  //显示校正用的“十”字
        do
        {
            Ptr=Read_2046_2();        //读取XPT2046数据到变量ptr            
        }
        while( Ptr == (void*)0 );     //当ptr为空时表示没有触点被按下
        ScreenSample[i].x= Ptr->x; 	  //把读取的原始数据存放到全局变量ScreenSample结构体
        ScreenSample[i].y= Ptr->y;

    }
		/* 用原始参数计算出 原始参数与坐标的转换系数。 */
    Cal_touch_para( &DisplaySample[0],&ScreenSample[0],&touch_para ) ;  	   
    
		/*取一个点计算X值*/
    test_x = ( (touch_para.An * ScreenSample[3].x) + 
               (touch_para.Bn * ScreenSample[3].y) + 
                touch_para.Cn 
             ) / touch_para.Divider ;			 
    
		/*取一个点计算Y值*/
    test_y = ( (touch_para.Dn * ScreenSample[3].x) + 
               (touch_para.En * ScreenSample[3].y) + 
               touch_para.Fn 
             ) / touch_para.Divider ;
    
    /* 实际坐标与计算坐标的差 */
    gap_x = (test_x > DisplaySample[3].x)?(test_x - DisplaySample[3].x):(DisplaySample[3].x - test_x);
    gap_y = (test_y > DisplaySample[3].y)?(test_y - DisplaySample[3].y):(DisplaySample[3].y - test_y);
    

    //LCD_Rectangle(0,0,320,240,CAL_BACKGROUND_COLOR);
    LCD_Clear(0, 0, 320, 240, BACKGROUND);
    
    /* 可以通过修改这两个值的大小来调整精度 */
    if((gap_x>10)||(gap_y>10))
    {
      LCD_DispStr(100, 100, RED,"Calibrate fail");
      LCD_DispStr(100, 120, RED,"try again");     
      sw_delay_ms(2000);
      return 1;
    }    
    
    /* 校正系数为全局变量 */
    aa1 = (touch_para.An*1.0)/touch_para.Divider;
    bb1 = (touch_para.Bn*1.0)/touch_para.Divider;
    cc1 = (touch_para.Cn*1.0)/touch_para.Divider;
    
    aa2 = (touch_para.Dn*1.0)/touch_para.Divider;
    bb2 = (touch_para.En*1.0)/touch_para.Divider;
    cc2 = (touch_para.Fn*1.0)/touch_para.Divider;
    
    #elif 0
    aa1=0.088370;
    bb1=-0.000468;
    cc1=-24.042172;
    aa2=0.0001891;
    bb2=0.062395;
    cc2=-10.223455;
    
    #endif
    
    LCD_DispStr(58, 100, RED, "The touch calibration is OK!!!");  
    sw_delay_ms(1000);
    
    
    return 0;    
}

/*
 * 画板初始化，用于取色用
 */
void Palette_Init(void)
{
  /* 整屏清为白色 */
  LCD_Clear(0, 0, 320, 240, GREY);
  
  /* 画两条直线 */
  LCD_Clear(39, 0, 1, 30, BLACK);
  LCD_Clear(0, 29, 40, 1, BLACK);
  LCD_DispStr(7, 10, RED, "CLR");
  
  LCD_Clear(0, 30, 40, 30, GREEN);
  LCD_Clear(0, 60, 40, 30, BLUE);
  LCD_Clear(0, 90, 40, 30, BRED);
  LCD_Clear(0, 120, 40, 30, GRED);
  LCD_Clear(0, 150, 40, 30, GBLUE);
  LCD_Clear(0, 180, 40, 30, BLACK);
  LCD_Clear(0, 210, 40, 30, RED);  
  
  sw_delay_ms(500);
}

/******************************************************
* 函数名：Get_touch_point
* 描述  ：通过 K A B C D E F 把通道X Y的值转换为液晶屏坐标
* 输入  : 无
* 输出  ：返回1表示成功 0失败
* 举例  ：无
* 注意  ：如果获取的触点信息有误，将返回DISABLE
*********************************************************/    
//long double linear=0 ;
//long double aa1=0,bb1=0,cc1=0,aa2=0,bb2=0,cc2=0;
FunctionalState Get_touch_point(Coordinate * displayPtr,
                                Coordinate * screenPtr,
                                Parameter * para )
{
  FunctionalState retTHRESHOLD =ENABLE ;

  if(screenPtr==0)
  {
    /*如果获取的触点信息有误，则返回DISABLE*/
    retTHRESHOLD = DISABLE;			
  }
  else
  {    
    if( para->Divider != 0 )
    {        
      displayPtr->x = ( (aa1 * screenPtr->x) + (bb1 * screenPtr->y) + cc1);        
      displayPtr->y = ((aa2 * screenPtr->x) + (bb2 * screenPtr->y) + cc2 );
    }
    else
    {
      retTHRESHOLD = DISABLE;
    }
  }
  return(retTHRESHOLD);
} 

/******************************************************
* 函数名：Palette_draw_point
* 描述  ：在LCD指定位置画一个大点(包含四个小点)
* 输入  : Xpos		--X方向位置
*         Ypos		--Y方向位置
* 输出  ：无
* 举例  ：Palette_draw_point(100,100);
* 注意  ：该函数是 "触摸画板应用实例" 专用函数
*********************************************************/    
void Palette_draw_point(uint16_t x, uint16_t y)
{
  /* 画笔默认为黑色 */
  static uint16_t Pen_color=0; 
  uint16_t y_pos = y;

  /* 在画板内取色 */
  if( x<40 )
  {
    if( y>30 )
    Pen_color = (y_pos<60)?GREEN:\
                (y_pos<90)?BLUE:\
                (y_pos<120)?BRED:\
                (y_pos<150)?GRED:\
                (y_pos<180)?GBLUE:\
                (y_pos<210)?BLACK:\
                (y_pos<240)?RED:BLUE;
    else
    {/* 清屏 */      
      #if 1
      LCD_Clear(40, 0, 280, 240, BLACK);
      #elif 0
      LCD_Clear(40, 0, 280, 240, WHITE);
      #endif
      return;
    }
  }
  else
  {
    #if 0
    LCD_SetPoint(x , y , Pen_color);
    LCD_SetPoint(x+1 , y , Pen_color);
    LCD_SetPoint(x , y+1 , Pen_color);
    LCD_SetPoint(x+1 , y+1 , Pen_color);
    #elif 1
    LCD_SetPoint(x , y , Pen_color);
    LCD_SetPoint(x-1 , y , Pen_color);
    LCD_SetPoint(x , y-1 , Pen_color);
    LCD_SetPoint(x+1 , y , Pen_color);
    LCD_SetPoint(x , y+1 , Pen_color);
    #endif
  }	
}



