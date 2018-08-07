#include "GUI.h"
#include "user_ili9341_lcd.h"
#include "user_xpt2046.h"
#include "GUIDRV_Template.h"
#include "GUIDRV_FlexColor.h"

//�봥�����йض��壬����ʵ�������д
#define TOUCH_AD_TOP		3810  	//���´������Ķ�����д�� Y ��ģ������ֵ��
#define TOUCH_AD_BOTTOM		260 	//���´������ĵײ���д�� Y ��ģ������ֵ��
#define TOUCH_AD_LEFT 		188		//���´���������࣬д�� X ��ģ������ֵ��
#define TOUCH_AD_RIGHT		3928	//���´��������Ҳ࣬д�� X ��ģ������ֵ��


//��Ļ��С
#define XSIZE_PHYS  320 //X��
#define YSIZE_PHYS  240 //Y��
#define VXSIZE_PHYS	320 
#define VYSIZE_PHYS 240



//���ü��
#ifndef   VXSIZE_PHYS
  #define VXSIZE_PHYS XSIZE_PHYS
#endif
#ifndef   VYSIZE_PHYS
  #define VYSIZE_PHYS YSIZE_PHYS
#endif
#ifndef   XSIZE_PHYS
  #error Physical X size of display is not defined!
#endif
#ifndef   YSIZE_PHYS
  #error Physical Y size of display is not defined!
#endif
#ifndef   GUICC_565
  #error Color conversion not defined!
#endif
#ifndef   GUIDRV_FLEXCOLOR
  #error No display driver defined!
#endif

  

//���ó���,���ڴ�����ʾ��������,������ɫת���������ʾ�ߴ�
void LCD_X_Config(void) {
	GUI_DEVICE_CreateAndLink(&GUIDRV_Template_API, GUICC_M565, 0, 0); //������ʾ��������
	LCD_SetSizeEx    (0, XSIZE_PHYS, YSIZE_PHYS);
	LCD_SetVSizeEx   (0, VXSIZE_PHYS, VYSIZE_PHYS);

	{
//		if(lcddev.dir == 0) //����
//		{					
//			GUI_TOUCH_Calibrate(GUI_COORD_X,0,lcddev.width,155,3903);
//			GUI_TOUCH_Calibrate(GUI_COORD_Y,0,lcddev.height,188,3935);
//		}else //����
		{
			GUI_TOUCH_SetOrientation(GUI_SWAP_XY|GUI_MIRROR_X); 
			GUI_TOUCH_Calibrate(GUI_COORD_X,0,240,260,3800); 	
			GUI_TOUCH_Calibrate(GUI_COORD_Y,0,320,188,3935);
		}
	}
}


//��ʾ�������Ļص�����
int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData) {
  int r;
  (void) LayerIndex;
  (void) pData;
  
  switch (Cmd) {
  case LCD_X_INITCONTROLLER: {
	//����ʼ����ʱ�򱻵���,��Ҫ��������ʾ������,�����ʾ���������ⲿ��ʼ������Ҫ�û���ʼ��
		
	//	TFTLCD_Init(); //��ʼ��LCD �Ѿ��ڿ�ʼ��ʼ����,���Դ˴�����Ҫ��ʼ����
    return 0;
  }
		default:
    r = -1;
	}
  return r;
}
