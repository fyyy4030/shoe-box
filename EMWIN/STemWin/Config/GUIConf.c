#include "GUI.h"
#include "malloc.h"

//����EMWIN�ڴ��С
#define GUI_NUMBYTES   (24*1024)
#define GUI_BLOCKSIZE  0X80  //���С

//GUI_X_Config
//��ʼ����ʱ�����,��������emwin��ʹ�õ��ڴ�
void GUI_X_Config(void) 
{
	uint32_t *aMemory = mymalloc(GUI_NUMBYTES); //���ڲ�RAM�з���GUI_NUMBYTES�ֽڵ��ڴ�
	GUI_ALLOC_AssignMemory((uint32_t *)aMemory, GUI_NUMBYTES); //Ϊ�洢����ϵͳ����һ���洢��
	GUI_ALLOC_SetAvBlockSize(GUI_BLOCKSIZE); //���ô洢���ƽ���ߴ�,����Խ��,���õĴ洢������Խ��
	GUI_SetDefaultFont(GUI_FONT_6X8); //����Ĭ������
	GUITASK_SetMaxTask(GUI_MAX_TASK);		
}
