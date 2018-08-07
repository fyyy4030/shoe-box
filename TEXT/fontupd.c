#include "fontupd.h" 
#include "flash.h"   
#include "ili9341_lcd.h"
#include "malloc.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK MiniSTM32������
//�ֿ���� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/3/14
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////

//�ֿ�����ʼ��ַ
#define FONTINFOADDR 	(4916+100)*1024 				//MiniSTM32�Ǵ�4.8M+100K��ַ��ʼ��
#define STMFLASHADDR    0x8019000
//�ֿ���Ϣ�ṹ��. 
//���������ֿ������Ϣ����ַ����С��
_font_info ftinfo;

//�ֿ�����sd���е�·��
const u8 *GBK24_PATH="0:/SYSTEM/FONT/GBK24.FON";		//GBK24�Ĵ��λ��
const u8 *GBK16_PATH="0:/SYSTEM/FONT/GBK16.FON";		//GBK16�Ĵ��λ��
const u8 *GBK12_PATH="0:/SYSTEM/FONT/GBK12.FON";		//GBK12�Ĵ��λ��
const u8 *UNIGBK_PATH="0:/SYSTEM/FONT/UNIGBK.BIN";		//UNIGBK.BIN�Ĵ��λ��
//��ȡָ����ַ�İ���(16λ����)
//faddr:����ַ(�˵�ַ����Ϊ2�ı���!!)
//����ֵ:��Ӧ����.
u16 STMFLASH_ReadHalfWord(uint32_t faddr)
{
	return *(vu16*)faddr; 
}

//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��
void STMFLASH_Read(uint32_t ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//��ȡ2���ֽ�.
		ReadAddr+=2;//ƫ��2���ֽ�.	
	}
}

//��ʾ��ǰ������½���
//x,y:����
//size:�����С
//fsize:�����ļ���С
//pos:��ǰ�ļ�ָ��λ��
uint32_t fupd_prog(u16 x,u16 y,u8 size,uint32_t fsize,uint32_t pos)
{
	float prog;
	u8 t=0XFF;
	prog=(float)pos/fsize;
	prog*=100;
	if(t!=prog)
	{
		LCD_DispStr(x+3*12/2,y,BLUE,"%");		
		t=prog;
		if(t>100)t=100;
		LCD_DisNum(x,y,t,BLUE);//��ʾ��ֵ
	}
	return 0;					    
} 
//����ĳһ��
//x,y:����
//size:�����С
//fxpath:·��
//fx:���µ����� 0,ungbk;1,gbk12;2,gbk16;3,gbk24;
//����ֵ:0,�ɹ�;����,ʧ��.
u8 updata_fontx(u16 x,u16 y,u8 size,u8 *fxpath,u8 fx)
{
	uint32_t flashaddr=0;								    
	u8 *tempbuf;
 	u8 res = 0;	
	u16 bread;
	uint32_t offx=0,write_size;
	u8 rval=0;	
	tempbuf=mymalloc(4096);	//����4096���ֽڿռ�
	if(tempbuf==NULL)
		rval=1;
// 	res=f_open(fftemp,(const TCHAR*)fxpath,FA_READ); 
// 	if(res)
//		rval=2;//���ļ�ʧ��  
 	if(rval==0)	 
	{
		switch(fx)
		{
			case 0:												//����UNIGBK.BIN
				ftinfo.ugbkaddr=FONTINFOADDR+sizeof(ftinfo);	//��Ϣͷ֮�󣬽���UNIGBKת�����
				ftinfo.ugbksize=174344;					//UNIGBK��С
				flashaddr=ftinfo.ugbkaddr;
			case 1:
				ftinfo.f12addr=ftinfo.ugbkaddr+ftinfo.ugbksize;	//UNIGBK֮�󣬽���GBK12�ֿ�
				ftinfo.gbk12size = 574560;				//GBK12�ֿ��С		
				flashaddr=ftinfo.f12addr;						//GBK12����ʼ��ַ
			case 2:
				ftinfo.f16addr = ftinfo.f12addr+ftinfo.gbk12size;	//GBK12֮�󣬽���GBK16�ֿ�
				ftinfo.gbk16size=766080;					//GBK16�ֿ��С
				flashaddr=ftinfo.f16addr;						//GBK16����ʼ��ַ
			case 3:
				ftinfo.f24addr=ftinfo.f16addr+ftinfo.gbk16size;	//GBK16֮�󣬽���GBK24�ֿ�
				ftinfo.gkb24size=1723680;					//GBK24�ֿ��С
				flashaddr=ftinfo.f24addr;						//GBK24����ʼ��ַ
				break;
		}   
		bread = 4096;
		//356480
		write_size = 85280;
		flashaddr = ftinfo.f24addr+400*1024*4; 
		while(0)//��ѭ��ִ��
		{
			STMFLASH_Read(STMFLASHADDR+offx,(uint16_t *)tempbuf,2048);

//	 		res=f_read(fftemp,tempbuf,4096,(UINT *)&bread);		//��ȡ����	 
//			if(res!=FR_OK)
//				break;								//ִ�д���
			SPI_Flash_Write(tempbuf,offx+flashaddr,4096);		//��0��ʼд��4096������  
	  		offx+=bread;	  
			fupd_prog(x,y,size,write_size,offx);	 			//������ʾ
			
			if(offx >= write_size)
			{
				bread = 0;
			}
			if(bread!=4096)
				break;								//������.
	 	} 	
//		f_close(fftemp);		
	}			 
	myfree(tempbuf);	//�ͷ��ڴ�
	return res;
}

//���������ļ�,UNIGBK,GBK12,GBK16,GBK24һ�����
//x,y:��ʾ��Ϣ����ʾ��ַ
//size:�����С
//��ʾ��Ϣ�����С										  
//����ֵ:0,���³ɹ�;
//		 ����,�������.	  
u8 update_font(u16 x,u16 y,u8 size)
{	
//	u8 *gbk24_path=(u8*)GBK24_PATH;
	u8 *gbk16_path=(u8*)GBK16_PATH;
//	u8 *gbk12_path=(u8*)GBK12_PATH;
//	u8 *unigbk_path=(u8*)UNIGBK_PATH;
//	u8 res;		   
// 	res=0XFF;		
	ftinfo.fontok=0XFF;
  	SPI_Flash_Write((u8*)&ftinfo,FONTINFOADDR,sizeof(ftinfo));	//���֮ǰ�ֿ�ɹ��ı�־.��ֹ���µ�һ������,���µ��ֿⲿ�����ݶ�ʧ.
 	SPI_Flash_Read((u8*)&ftinfo,FONTINFOADDR,sizeof(ftinfo));	//���¶���ftinfo�ṹ������
// 	LCD_ShowString(x,y,240,320,size,"Updating UNIGBK.BIN");		

//	updata_fontx(x+20*size/2,y,size,unigbk_path,0);			//����UNIGBK.BIN
//	if(res)return 1;
// 	LCD_ShowString(x,y,240,320,size,"Updating GBK12.BIN  ");
//	updata_fontx(x+20*size/2,y,size,gbk12_path,0);			//����GBK12.FON
//	if(res)return 2;
	LCD_DispStr(x,y,BLUE,"Updating GBK16.BIN  ");
	updata_fontx(x+20*size/2,y,size,gbk16_path,0);			//����GBK16.FON
//	if(res)return 3;
//	LCD_ShowString(x,y,240,320,size,"Updating GBK24.BIN  ");
//	res=updata_fontx(x+20*size/2,y,size,gbk24_path,3);			//����GBK24.FON
//	if(res)return 4;	 
	//ȫ�����º��� 
	ftinfo.fontok=0XAA;
  	SPI_Flash_Write((u8*)&ftinfo,FONTINFOADDR,sizeof(ftinfo));	//�����ֿ���Ϣ
	return 0;//�޴���.		 
} 
//��ʼ������
//����ֵ:0,�ֿ����.
//		 ����,�ֿⶪʧ
u8 font_init(void)
{			  												 
 	SPI_Flash_Read((u8*)&ftinfo,FONTINFOADDR,sizeof(ftinfo));//����ftinfo�ṹ������
	if(ftinfo.fontok!=0XAA)
		return 1;			//�ֿ����. 
	return 0;		    
}





























