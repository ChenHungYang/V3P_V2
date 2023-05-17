/*
 * 主要使用在設定畫面第三層的函式，命名規則如下。
 *  inMENU03_NuwUiOtherMeun
 *  in: 回傳值型態
 *  MENU03 : 顯示UI層級
 *  NuwUiOtherMeun : 函式功能名稱  
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

#include "../../FUNCTION/Function.h"
#include "../../FUNCTION/CREDIT_FUNC/CreditCheckFunc.h"
#include "../Flow.h"

#include "Layer3Menu.h"
#include "Layer4Menu.h"

/*
Function        :inMENU03_NewNuOtherMenu
Date&Time       :2017/10/30 下午 5:44
Describe        :
*/
int inMENU03_NewNuOtherMenu(EventMenuItem *srEventMenuItem)
{
	int			inPageLoop = _PAGE_LOOP_1_;
        int			inRetVal = VS_SUCCESS;
        int			inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
        int			inCount= 1;
        int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char			szKey = 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_Touch_NONE_				, _TRANS_TYPE_NULL_	, NULL						, ""					}
	};

        /* 初始化 */
	inDISP_ClearAll();
	inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_10_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
	inDISP_PutGraphic(_TOUCH_NEWUI_OTHER_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
	inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

	/* 設定Timeout */
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

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
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_:
					inCount = 1;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_:
					inCount = 2;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_RETURN_IDLE_BUTTON_:
					inRetVal = VS_ERROR;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				case _NEWUI_LAST_PAGE_BUTTON_:
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				case _DisTouch_Slide_Down_To_Up_:
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				default:
					break;
			}

			if (szKey == _KEY_1_)
			{
				inCount = 1;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_2_)
			{
				inCount = 2;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_0_)
			{
				inCount = 0;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				inPageLoop = _PAGE_LOOP_0_;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				inPageLoop = _PAGE_LOOP_0_;
				break;
			}

		}

		/* 判斷MENU點進去按鈕之後要做的事情 */
		if (inPageLoop == _PAGE_LOOP_2_)
		{
			switch (inCount)
			{
				case 1:
					inRetVal = inMENU_COMM_SETTING(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 2:
					inRetVal = inMENU_CUP_LOGON(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				/*case 4:
					inRetVal = inMENU_TMS_TASK_REPORT(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;*/
				case 0:
					/* 壓住0後3秒內按clear */
					inDISP_Timer_Start(_TIMER_NEXSYS_4_, 3);
					do
					{
						szKey = uszKBD_Key_In();
					}while (szKey == 0	&&
						inTimerGet(_TIMER_NEXSYS_4_) != VS_SUCCESS);

					/* 不是按clear，不能進隱藏選單 */
					if (szKey != _KEY_CLEAR_)
					{
						inRetVal = VS_SUCCESS;
						inPageLoop = _PAGE_LOOP_1_;
						break;
					}

					inFlushKBDBuffer();

					srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
					srEventMenuItem->inCode = FALSE;
					/* 輸入管理號碼 */
					inDISP_ClearAll();
					if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
					}
					else
					{
						inRetVal = inMENU04_NewUiFunctionMenu(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
					}


					break;
				default:
					inPageLoop = _PAGE_LOOP_0_;
					break;
			}
		}

		/* 代表回上一頁，要回復UI */
		if (inPageLoop == _PAGE_LOOP_1_ &&
		    inRetVal == VS_ERROR)
		{
			/* 初始化 */
			inDISP_ClearAll();
			inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_10_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
			inDISP_PutGraphic(_TOUCH_NEWUI_OTHER_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
			inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

			inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
		}
		/* 功能未開而返回，不刷畫面 */
		else if (inPageLoop == _PAGE_LOOP_1_	&&
			 inRetVal == VS_SUCCESS)
		{

		}

	}/* _PAGE_LOOP_0_ */

	return (inRetVal);
}
