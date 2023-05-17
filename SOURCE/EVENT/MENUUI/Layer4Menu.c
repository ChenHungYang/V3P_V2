/*
 * 主要使用在設定畫面第三層的函式，命名規則如下。
 *  inMENU04_NewUiFunctionMenu
 *  in: 回傳值型態
 *  MENU04 : 顯示UI層級
 *  NewUiFunctionMenu : 函式功能名稱  
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctosapi.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>


#include "../../INCLUDES/Define_1.h"
#include "../../INCLUDES/Define_2.h"
#include "../../INCLUDES/Transaction.h"
#include "../../INCLUDES/TransType.h"
#include "../../PRINT/Print.h"
#include "../../DISPLAY/Display.h"
#include "../../DISPLAY/DisTouch.h"

#include "../MenuMsg.h"
#include "../Menu.h"

#include "../../FUNCTION/CREDIT_FUNC/CreditCheckFunc.h"
#include "../../FUNCTION/Function.h"


#include "Layer4Menu.h"
#include "Layer5Menu.h"

/*
Function		: inMENU04_NewUiFunctionMenu
Date&Time	: 2018/5/21 下午 4:04
Describe		: 工程師設定頁面，
 */
int inMENU04_NewUiFunctionMenu(EventMenuItem *srEventMenuItem)
{
	int inPageLoop = _PAGE_LOOP_1_;
	int inRetVal = VS_SUCCESS;
	int inChioce1 = _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_1_;
	int inCount = 1;
	int inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_2_TO_7_3X4_;
	char szKey = 0x00;
	MENU_CHECK_TABLE srMenuChekDisplay[] ={
		{_Touch_NONE_, _TRANS_TYPE_NULL_, NULL, ""}
	};

	/* 初始化 */
	inDISP_ClearAll();
	inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_1_); /* 第一層顯示 LOGO */
	inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0, _COORDINATE_Y_LINE_8_2_);
	inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);

	/* 設定Timeout */
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);

	while (inPageLoop != _PAGE_LOOP_0_)
	{
		while (inPageLoop == _PAGE_LOOP_1_)
		{
			szKey = 0;
			szKey = uszKBD_Key();
			inChioce1 = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc); /* 回傳MENU畫面的按鈕選擇 */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			switch (inChioce1)
			{
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_1_:
					inCount = 1;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_2_:
					inCount = 2;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_3_:
					inCount = 3;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_4_:
					inCount = 4;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_5_:
					inCount = 5;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_6_:
					inCount = 6;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_7_:
					inCount = 7;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_8_:
					inCount = 8;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_9_:
					inCount = 9;
					inPageLoop = _PAGE_LOOP_2_;
					break;
					/*case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_10_:
						inCount = 10;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_11_:
						inCount = 11;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_12_:
						inCount = 12;
						inPageLoop = _PAGE_LOOP_2_;
						break;*/
				case _NEWUI_RETURN_IDLE_BUTTON_:
					inRetVal = VS_ERROR;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				case _DisTouch_Slide_Down_To_Up_:
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				default:
					break;
			}

			if (szKey == _KEY_UP_) /*向上按鍵*/
			{
				inCount--;
				switch (inCount)
				{
					case 1:
						inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
						inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0, _COORDINATE_Y_LINE_8_2_);
						inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
						break;
					case 2:
						inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
						inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0, _COORDINATE_Y_LINE_8_2_);
						inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
						break;
					case 3:
						inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
						inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0, _COORDINATE_Y_LINE_8_2_);
						inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
						break;
					case 4:
						inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
						inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0, _COORDINATE_Y_LINE_8_2_);
						inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
						break;
					case 5:
						inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
						inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0, _COORDINATE_Y_LINE_8_2_);
						inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
						break;
					case 6:
						inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
						inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0, _COORDINATE_Y_LINE_8_2_);
						inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
						break;
					case 7:
						inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
						inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0, _COORDINATE_Y_LINE_8_2_);
						inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_, VS_FALSE);
						break;
					case 8:
						inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
						inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0, _COORDINATE_Y_LINE_8_2_);
						inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_, VS_FALSE);
						break;
					case 9:
						inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
						inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0, _COORDINATE_Y_LINE_8_2_);
						inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_, VS_FALSE);
						break;
					case 10:
						inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
						inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0, _COORDINATE_Y_LINE_8_2_);
						inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_11_Y2_, VS_FALSE);
						break;
					default:
						inCount = 1;
						break;
				}
			} else if (szKey == _KEY_DOWN_) /*向下按鍵*/
			{
				inCount++;
				switch (inCount)
				{
					case 2:
						inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
						inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0, _COORDINATE_Y_LINE_8_2_);
						inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
						break;
					case 3:
						inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
						inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0, _COORDINATE_Y_LINE_8_2_);
						inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
						break;
					case 4:
						inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
						inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0, _COORDINATE_Y_LINE_8_2_);
						inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
						break;
					case 5:
						inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
						inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0, _COORDINATE_Y_LINE_8_2_);
						inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
						break;
					case 6:
						inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
						inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0, _COORDINATE_Y_LINE_8_2_);
						inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
						break;
					case 7:
						inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
						inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0, _COORDINATE_Y_LINE_8_2_);
						inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_, VS_FALSE);
						break;
					case 8:
						inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
						inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0, _COORDINATE_Y_LINE_8_2_);
						inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_, VS_FALSE);
						break;
					case 9:
						inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
						inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0, _COORDINATE_Y_LINE_8_2_);
						inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_, VS_FALSE);
						break;
					case 10:
						inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
						inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0, _COORDINATE_Y_LINE_8_2_);
						inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_11_Y2_, VS_FALSE);
						break;
					case 11:
						inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
						inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0, _COORDINATE_Y_LINE_8_2_);
						inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_11_Y2_, VS_FALSE);
						break;
					default:
						inCount = 11;
						break;
				}
			} else if (szKey == _KEY_1_)
			{
				inCount = 1;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			} else if (szKey == _KEY_2_)
			{
				inCount = 2;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			} else if (szKey == _KEY_3_)
			{
				inCount = 3;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			} else if (szKey == _KEY_4_)
			{
				inCount = 4;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			} else if (szKey == _KEY_5_)
			{
				inCount = 5;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			} else if (szKey == _KEY_6_)
			{
				inCount = 6;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			} else if (szKey == _KEY_7_)
			{
				inCount = 7;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			} else if (szKey == _KEY_8_)
			{
				inCount = 8;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			} else if (szKey == _KEY_9_)
			{
				inCount = 9;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}				/*else if (szKey == _KEY_0_)
			{
				inCount = 10;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}*/
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				inPageLoop = _PAGE_LOOP_0_;
				break;
			} else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				inPageLoop = _PAGE_LOOP_0_;
				break;
			} else if (szKey == _KEY_ENTER_)
			{
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}

		}

		/* 判斷MENU點進去按鈕之後要做的事情 */
		if (inPageLoop == _PAGE_LOOP_2_)
		{
			switch (inCount)
			{
				case 1:
					inRetVal = inMENU_TMS_PARAMETER_PRINT(srEventMenuItem); /* TMS參數列印 */
					inPageLoop = _PAGE_LOOP_0_;
					break;
#if 0
				case 2:
					inRetVal = inMENU_DCC_PARAMETER_PRINT(srEventMenuItem); /* DCC參數列印 */
					inPageLoop = _PAGE_LOOP_0_;
					break;
#endif
				case 2:
					inRetVal = inMENU_DELETE_BATCH(srEventMenuItem); /* 清除批次 */
					inPageLoop = _PAGE_LOOP_0_;
					break;
#if 0
				case 4:
					inRetVal = inMENU_SAM_REGISTER(srEventMenuItem); /* 註冊SAM卡 */
					inPageLoop = _PAGE_LOOP_0_;
					break;
#endif
				case 3:
					inRetVal = inMENU_DEBUG_SWITCH(srEventMenuItem); /* Debug開關 */
					/* 回上一頁 */
					inRetVal = VS_ERROR;
					inPageLoop = _PAGE_LOOP_1_;
					break;
				case 4:
					inRetVal = inMENU_TIME_SETTING(srEventMenuItem); /* 時間設定 */
					/* 回上一頁 */
					inRetVal = VS_ERROR;
					inPageLoop = _PAGE_LOOP_1_;
					break;
				case 5:
					inRetVal = inMENU_EDIT_TMEP_VERSION_ID(srEventMenuItem); /* 修改TMS版本 */
					/* 回上一頁 */
					inRetVal = VS_ERROR;
					inPageLoop = _PAGE_LOOP_1_;
					break;
				case 6:
					inRetVal = inMENU_EDIT_TMSOK(srEventMenuItem); /* 修改TMS完成 */
					/* 回上一頁 */
					inRetVal = VS_ERROR;
					inPageLoop = _PAGE_LOOP_1_;
					break;
				case 7:
					inRetVal = inMENU_UNLOCK_EDC(srEventMenuItem); /* EDC解鎖 */
					/* 回上一頁 */
					inRetVal = VS_ERROR;
					inPageLoop = _PAGE_LOOP_1_;
					break;
				case 8:
					inRetVal = inMENU05_NewUiFunction2Menu(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					} else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 9:
					inRetVal = inMENU_TRACELOG_UP(srEventMenuItem);
					/* 回上一頁 */
					inPageLoop = _PAGE_LOOP_0_;
					break;
				default:
					inRetVal = VS_SUCCESS;
					inPageLoop = _PAGE_LOOP_1_;
					break;
			}
		}

		/* 代表回上一頁，要回復UI */
		if (inPageLoop == _PAGE_LOOP_1_ &&
				inRetVal == VS_ERROR)
		{
			/* 初始化 */
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_1_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0, _COORDINATE_Y_LINE_8_2_);
			inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);
			inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

			inCount = 1;
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		}			/* 功能未開而返回，不刷畫面 */
		else if (inPageLoop == _PAGE_LOOP_1_ &&
				inRetVal == VS_SUCCESS)
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		}

	}/* _PAGE_LOOP_0_ */

	return (inRetVal);
}
