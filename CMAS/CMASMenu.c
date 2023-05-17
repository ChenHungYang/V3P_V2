#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctosapi.h>

#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/EVENT/Menu.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/TDT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/FUNCTION/FILE_FUNC/File.h"
#include "CMASAPIISO.h"
#include "CMASFunction.h"
#include "../ECC/ICER/stdAfx.h"
#include "CMASMenu.h"

extern int ginDebug;

/*Hachi Added Start*/

/*
Function        :inCMAS_MENU_ICON
Date&Time       :2019/12/26 上午 13:52 
Describe        :悠遊卡選單(參考一卡通二代卡UI)
Author          :Hachi  
 * 畫面說明
 * 畫面大小 320x480 每個ICON 約100x100
 * 一卡通交易
 * [銷售交易][退貨交易][現金加值]
 * [取消加值][結帳交易][餘額查詢]
 * [回上一層][回主畫面]
 */
int inCMAS_MENU_ICON(EventMenuItem *srEventMenuItem) {
	int inPageLoop = _PAGE_LOOP_1_;
	int inRetVal = VS_SUCCESS;
	int inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
	int inCount= 1;
	int inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char szKey = 0x00;

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintfAt(AT, "The READER_MANUFACTURERS = LINUX_API (%d) ", LINUX_API);

	if (inCMAS_MENU_Check_Enable() != VS_SUCCESS) {
		inRetVal = VS_FUNC_CLOSE_ERR;
	} else {
		inDISP_ClearAll();

		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_1_); /* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_2_);
		inDISP_PutGraphic(_TOUCH_NEWUI_ETICKET_PAGE_, 0, _COORDINATE_Y_LINE_8_3_);
		inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0, _COORDINATE_Y_LINE_8_8_);

		/* 設定Timeout */
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

		while (inPageLoop != _PAGE_LOOP_0_)
		{
			while (inPageLoop == _PAGE_LOOP_1_)
			{
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
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_:
						inCount = 3;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_:
						inCount = 4;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_:
						inCount = 5;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_:
						inCount = 6;
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
				else if (szKey == _KEY_3_)
				{
					inCount = 3;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_4_)
				{
					inCount = 4;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_5_)
				{
					inCount = 5;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_6_)
				{
					inCount = 6;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}else if (szKey == _KEY_0_) {
					inCount = 10;
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
						inRetVal = inCMAS_MENU_SALE(srEventMenuItem);
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
						inRetVal = inCMAS_MENU_REFUND(srEventMenuItem);
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
					case 3:
						inRetVal = inCMAS_MENU_QUERY(srEventMenuItem);
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
					case 4:
						inRetVal = inCMAS_MENU_ADD_VALUE(srEventMenuItem);
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
					case 5:
						inRetVal = inCMAS_MENU_VOID_ADD_VALUE(srEventMenuItem);
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
					case 6:
						inRetVal = inCMAS_MENU_SETTLE(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL) {
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						} else {
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;	
					case 10:
						// inRetVal = remove("ICERAPI_CMAS.rev");
						inRetVal = inFile_Unlink_File("ICERAPI_CMAS.rev", _CMAS_FOLDER_PATH_); /* 檢核用 */
						if (inRetVal == VS_SUCCESS) {
							inPageLoop = _PAGE_LOOP_0_;
						} else if (inRetVal == VS_USER_CANCEL) {
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						} else {
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					default:
						inPageLoop = _PAGE_LOOP_0_;
						break;
				}
			}			
			
			/* 代表回上一頁，要回復UI */
			if (inPageLoop == _PAGE_LOOP_1_ &&
				inRetVal == VS_ERROR) {
				
				/* 初始化 */
				inDISP_ClearAll();
				inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
				inDISP_PutGraphic(_TOUCH_NEWUI_ETICKET_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
				inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

			}				/* 功能未開而返回，不刷畫面 */
			else if (inPageLoop == _PAGE_LOOP_1_ &&
					inRetVal == VS_SUCCESS) {
			}
		} /* _PAGE_LOOP_0_ */

		/*20200519個別交易功能未開顯示 [Hachi]*/
		if (inRetVal == VS_FUNC_CLOSE_ERR) {
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 2, "", 0);
		}
	}

	return (inRetVal);
}

/*
Function        :inCMAS_MENU_SALE
Date&Time       :2019/12/26 下午 3:51 
Describe        :悠遊卡購貨
Author          :Hachi 
 */
int inCMAS_MENU_SALE(EventMenuItem *srEventMenuItem) {
	int inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
	inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);

	if (inCMAS_Func_Check_Transaction_Function(_TICKET_EASYCARD_DEDUCT_) != VS_SUCCESS) {
		inRetVal = VS_FUNC_CLOSE_ERR;
	} else {
		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inCode = _TICKET_EASYCARD_DEDUCT_;

		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inRunOperationID = _OPERATION_EASYCARD_DEDUCT_;
		srEventMenuItem->inRunTRTID = _TRT_CMAS_DEDUCT_;
	}

	return (inRetVal);
}

/*
Function        :inCMAS_MENU_REFUND
Date&Time       :2019/12/26 下午 3:51 
Describe        :悠遊卡退貨
Author          :Hachi
 */
int inCMAS_MENU_REFUND(EventMenuItem *srEventMenuItem) {
	int inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_TICKET_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜退貨交易＞ */

	if (inCMAS_Func_Check_Transaction_Function(_TICKET_EASYCARD_REFUND_) != VS_SUCCESS) {
		inRetVal = VS_FUNC_CLOSE_ERR;
	} else {
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inCode = _TICKET_EASYCARD_REFUND_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inRunOperationID = _OPERATION_EASYCARD_DEDUCT_;
		srEventMenuItem->inRunTRTID = _TRT_CMAS_REFUND_;
		// srEventMenuItem->uszESVCTransBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function        :inCMAS_MENU_ADD_VALUE
Date&Time       :2019/12/26 下午 3:51 
Describe        :悠遊卡現金加值
Author          :Hachi
 */
int inCMAS_MENU_ADD_VALUE(EventMenuItem *srEventMenuItem) {
	int inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_TICKET_TOP_UP_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜加值交易＞ */

	if (inCMAS_Func_Check_Transaction_Function(_TICKET_EASYCARD_TOP_UP_) != VS_SUCCESS) {
		inRetVal = VS_FUNC_CLOSE_ERR;
	} else {
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inCode = _TICKET_EASYCARD_TOP_UP_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inRunOperationID = _OPERATION_EASYCARD_DEDUCT_;
		srEventMenuItem->inRunTRTID = _TRT_CMAS_ADD_VALUE_;
		// srEventMenuItem->uszESVCTransBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function        :inCMAS_MENU_VOID_ADD_VALUE
Date&Time       :2019/12/26 下午 3:51 
Describe        :悠遊卡取消加值
Author          :Hachi
 */
int inCMAS_MENU_VOID_ADD_VALUE(EventMenuItem *srEventMenuItem) {
	int inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_TICKET_VOID_TOP_UP_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜加值取消＞ */

	if (inCMAS_Func_Check_Transaction_Function(_TICKET_EASYCARD_VOID_TOP_UP_) != VS_SUCCESS) {
		inRetVal = VS_FUNC_CLOSE_ERR;
	} else {
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inCode = _TICKET_EASYCARD_VOID_TOP_UP_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inRunOperationID = _OPERATION_EASYCARD_VOID_;
		srEventMenuItem->inRunTRTID = _TRT_CMAS_VOID_ADD_VALUE_;
		// srEventMenuItem->uszESVCTransBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function        :inCMAS_MENU_SETTLE
Date&Time       :2019/12/26 下午 3:51 
Describe        :悠遊卡結帳
Author          :Hachi
 */
int inCMAS_MENU_SETTLE(EventMenuItem *srEventMenuItem) {
	int inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
	inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);

	if (inCMAS_Func_Check_Transaction_Function(_SETTLE_) != VS_SUCCESS) {
		inRetVal = VS_FUNC_CLOSE_ERR;
	} else {
		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inCode = _SETTLE_;

		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inRunOperationID = _OPERATION_EASYCARD_NORMAL_;
		srEventMenuItem->inRunTRTID = _TRT_CMAS_SETTLE_;
	}

	return (inRetVal);
}

/*
Function        :inCMAS_MENU_QUERY
Date&Time       :2019/12/26 下午 3:51 
Describe        :悠遊卡餘額查詢/詢卡
Author          :Hachi 
 */
int inCMAS_MENU_QUERY(EventMenuItem *srEventMenuItem) {
	int inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_TICKET_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜餘額查詢＞ */

	if (inCMAS_Func_Check_Transaction_Function(_TICKET_EASYCARD_INQUIRY_) != VS_SUCCESS) {
		inRetVal = VS_FUNC_CLOSE_ERR;
	} else {
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inCode = _TICKET_EASYCARD_INQUIRY_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inRunOperationID = _OPERATION_EASYCARD_NORMAL_;
		srEventMenuItem->inRunTRTID = _TRT_CMAS_QUERY_;
	}

	return (inRetVal);
}

/*
Function        :inCMAS_MENU_Check_Enable
Date&Time       :2020/2/3 上午 10:32
Describe        :參考inIPASS2_Func_Check_Enable
Author          :Hachi
 */
int inCMAS_MENU_Check_Enable(void) {
	char szHostEnable[2 + 1];
	int inHDTindex = -1;

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "inCMAS_MENU_Check_Enable START!!!");
	}

	inFunc_Find_Specific_HDTindex(-1, _HOST_NAME_CMAS_, &inHDTindex);

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "inHDTindex (%d)", inHDTindex);
	}

	if (inLoadHDTRec(inHDTindex) != VS_SUCCESS)
		return (VS_ERROR);

	memset(szHostEnable, 0x00, 1);
	inGetHostEnable(szHostEnable);

	if (szHostEnable[0] != 'Y') {
		if (ginDebug == VS_TRUE) {
			inDISP_LogPrintfAt(AT, "CMAS HOST not open.");
		}
		//此功能已關閉
		inDISP_ClearAll();
		inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 2, "", 0); /*20200519 add[Hachi]*/
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "inCMAS_MENU_Check_Enable END!!!");
	}

	return (VS_SUCCESS);
}