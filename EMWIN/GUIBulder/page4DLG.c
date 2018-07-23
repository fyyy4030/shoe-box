/*********************************************************************
*                                                                    *
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
*                                                                    *
**********************************************************************
*                                                                    *
* C-file generated by:                                               *
*                                                                    *
*        GUI_Builder for emWin version 5.26                          *
*        Compiled Aug 18 2014, 17:12:05                              *
*        (c) 2014 Segger Microcontroller GmbH & Co. KG               *
*                                                                    *
**********************************************************************
*                                                                    *
*        Internet: www.segger.com  Support: support@segger.com       *
*                                                                    *
**********************************************************************
*/

// USER START (Optionally insert additional includes)
// USER END

#include "DIALOG.h"
#include "device_info.h"
/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define ID_WINDOW_3     (GUI_ID_USER + 0x40)
#define ID_GRAPH_40     (GUI_ID_USER + 0x41)
#define ID_CHECKBOX_40     (GUI_ID_USER + 0x42)


// USER START (Optionally insert additional defines)
// USER END

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

// USER START (Optionally insert additional static data)
// USER END

/*********************************************************************
*
*       _aDialogpage4Create
*/
static const GUI_WIDGET_CREATE_INFO _aDialogpage4Create[] = {
  { WINDOW_CreateIndirect, "page4", ID_WINDOW_3, 0, 0, 310, 120, 0, 0x0, 0 },
  { GRAPH_CreateIndirect, "p4rate", ID_GRAPH_40, 0, 24, 306, 76, 0, 0x0, 2 },
  { CHECKBOX_CreateIndirect, "p4rate", ID_CHECKBOX_40, 106, 2, 80, 20, 0, 0x0, 0 },
  // USER START (Optionally insert additional widgets)
  // USER END
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

// USER START (Optionally insert additional static code)
// USER END

/*********************************************************************
*
*       _cbpage4Dialog
*/
static void _cbpage4Dialog(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  int     NCode;
  int     Id;
  // USER START (Optionally insert additional variables)
  // USER END

  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
	//
	// Initialization of 'alltemp'
	//
	hItem = pMsg->hWin;
	WINDOW_SetBkColor(hItem, GUI_DARKCYAN);
    // Initialization of 'p4rate'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_GRAPH_40);
    GRAPH_SetBorder(hItem, 2, 2, 2, 2);
    //
    // Initialization of 'p4rate'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_CHECKBOX_40);
    CHECKBOX_SetText(hItem, "����");
    CHECKBOX_SetTextColor(hItem, 0x00FF0000);
  	CHECKBOX_SetFont(hItem,&GUI_FontHZ12);  
    // USER START (Optionally insert additional code for further widget initialization)
    // USER END
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch(Id) {
    case ID_CHECKBOX_40: // Notifications sent by 'p4rate'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_VALUE_CHANGED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    // USER START (Optionally insert additional code for further Ids)
    // USER END
    }
    break;
  // USER START (Optionally insert additional message handling)
  // USER END
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       Createpage4
*/
WM_HWIN Createpage4(void);
WM_HWIN Createpage4(void) {
  WM_HWIN hWin;

  hWin = GUI_CreateDialogBox(_aDialogpage4Create, GUI_COUNTOF(_aDialogpage4Create), _cbpage4Dialog, WM_HBKWIN, 0, 0);
  return hWin;
}

// USER START (Optionally insert additional public code)
// USER END

/*************************** End of file ****************************/
