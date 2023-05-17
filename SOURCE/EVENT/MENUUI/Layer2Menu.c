/*
 * 主要使用在設定畫面第二層的函式，命名規則如下。
 *  inMENU02_VoidTrans
 *  in: 回傳值型態
 *  MENU02 : 顯示UI層級
 *  VoidTrans : 函式功能名稱  
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


#include "../../../EMVSRC/EMVsrc.h"

#include "../MenuMsg.h"
#include "../Menu.h"
#include "../Flow.h"


#include "../../FUNCTION/Function.h"
#include "../../FUNCTION/EDC.h"
#include "../../FUNCTION/Card.h"

#include "../../FUNCTION/CREDIT_FUNC/CreditCheckFunc.h"


#include "../../../FUBON/FUBONiso.h"

#include "../../../SVCSRC/SvcMenu.h"

#include "Layer2Menu.h"
#include "Layer3Menu.h"

extern int ginDebug;
extern int ginIdleMSRStatus;
extern int ginIdleICCStatus;

/*
Function        :inMENU02_NewUiTransactionMenu
Date&Time       :2017/10/25 下午 1:55
Describe        :
*/
int inMENU02_NewUiTransactionMenu(EventMenuItem *srEventMenuItem)
{
	int			inPageLoop = _PAGE_LOOP_1_;
        int			inRetVal = VS_SUCCESS;
        int			inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
        int			inCount = 1;
        int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char			szKey = 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_	, _VOID_			, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_1_VOID_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_	, _REFUND_		, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_2_REFUND_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_	, _INST_SALE_		, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_3_INST_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_	, _REDEEM_SALE_	, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_4_REDEEM_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_	, _SALE_OFFLINE_	, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_5_SALEOFFLINE_	},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_	, _TIP_			, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_6_TIP_			},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_7_	, _PRE_AUTH_		, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_7_PREAUTH_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_8_	, _TRANS_TYPE_NULL_	, inMENU_Check_Adjust			, _ICON_HIGHTLIGHT_1_8_ADJUST_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_9_	, _ADJUST_			, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_9_ADJUST_		},
		{_Touch_NONE_				, _TRANS_TYPE_NULL_	, NULL			, ""					}
	};

	if (inMENU_Check_Transaction_Enable(0) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		/* 初始化 */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_1_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
		inDISP_PutGraphic(_TOUCH_NEWUI_TRANSACTION_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
		inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

		/* 檢查功能開關，並顯示反白的圖 */
		inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

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
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_7_:
						inCount = 7;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_8_:
						inCount = 8;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_9_:
						inCount = 9;
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
				}
				else if (szKey == _KEY_7_)
				{
					inCount = 7;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_8_)
				{
					inCount = 8;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_9_)
				{
					inCount = 9;
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
						inRetVal = inMENU_VOID(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 2:
						inRetVal = inMENU_REFUND(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 3:
						inRetVal = inMENU_INST(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 4:
						inRetVal = inMENU_REDEEM(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 5:
						inRetVal = inMENU_SALEOFFLINE(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 6:
						inRetVal = inMENU_TIP(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 7:
						inRetVal = inMENU_PREAUTH(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 8:
						inRetVal = inMENU_ADJUST(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 9:
						inRetVal = inMENU_SALE_ADJUST(srEventMenuItem);
						//inRetVal = inMENU_MAILORDER(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					default:
						inPageLoop = _PAGE_LOOP_0_;
						break;
				}
			}

			/* 代表回上一頁，要回復UI */
			if (inPageLoop == _PAGE_LOOP_1_		&&
			    inRetVal == VS_ERROR)
			{
				/* 初始化 */
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_1_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
				inDISP_PutGraphic(_TOUCH_NEWUI_TRANSACTION_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
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
	}

	return (inRetVal);
}




/*
Function        :inMENU02_NuwUiCupMenu
Date&Time       :2017/10/25 下午 1:55
Describe        :
*/
int inMENU02_NuwUiCupMenu(EventMenuItem *srEventMenuItem)
{
	int	inMSR_RetVal = -1;	/* 磁條事件的反應，怕和其他用到inRetVal的搞混所以獨立出來 */
	int	inEMV_RetVal = -1;	/* 晶片卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
	int	inPageLoop = _PAGE_LOOP_1_;
	int	inRetVal = VS_SUCCESS;
	int	inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
	int	inCount= 1;
	int	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char	szKey = 0x00;
	char szCUPTMKOK[2],szCUPTPKOK[2],szCUPLOGONOK[2];

	TRANSACTION_OBJECT	pobTran;
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_	, _CUP_SALE_	, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_2_7_CUP_SALE_	},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_	, _REFUND_		, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_2_REFUND_		},
//		{_NEWUI_FUNC_LINE_3_TO_7_Touch_KEY_3_	, _CUP_INST_SALE_	, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_3_INST_		},
//		{_NEWUI_FUNC_LINE_3_TO_7_Touch_KEY_4_	, _CUP_REDEEM_SALE_	, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_4_REDEEM_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_	, _CUP_PRE_AUTH_	, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_2_5_CUP_PREAUTH_	},
		//{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_	, _CUP_MAIL_ORDER_	, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_2_6_CUP_MAILORDER_	},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_	, _VOID_		, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_1_VOID_		},
		{_Touch_NONE_					, _TRANS_TYPE_NULL_	, NULL						, ""					}
	};



	if (inMENU_Check_CUP_Enable(0) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		

		inGetCUPTMKOK(szCUPTMKOK);
		inGetCUPTPKOK(szCUPTPKOK);
		inGetCUPLOGONOK(szCUPLOGONOK);
		
		if(szCUPTMKOK[0] != '1' || szCUPTPKOK[0] != '1' || szCUPLOGONOK[0] != '1')
		/* 有開CUP且MACEnable有開但安全認證沒過，不能執行CUP交易 */
		//if (inKMS_CheckKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_PIN_ONLINE_) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("inMENU02_NuwUiCupMenu 2");
			
			memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));

			if (inFUBON_ISO_CUP_FuncAutoLogon(&pobTran) != VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
					inDISP_LogPrintf("inMENU02_NuwUiCupMenu 3");
				/* 安全認證失敗 */
				return (VS_ERROR);
			}

		}

		/* 初始化 */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_2_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
		inDISP_PutGraphic(_TOUCH_NEWUI_CUP_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
		inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

		/* 檢查功能開關，並顯示反白的圖 */
		inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

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

				/* 偵測磁條刷卡 */
				inMSR_RetVal = inCARD_MSREvent();

				if (inMSR_RetVal == VS_SUCCESS)
				{
					ginIdleMSRStatus = VS_TRUE;
				}

				/* 偵測晶片插卡 */
				inEMV_RetVal = inEMV_ICCEvent();
				if (inEMV_RetVal == VS_SUCCESS)
				{
					ginIdleICCStatus = VS_TRUE;
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
					/*case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_:
						inCount = 5;
						inPageLoop = _PAGE_LOOP_2_;
						break;*/
//					case _NEWUI_FUNC_LINE_3_TO_7_Touch_KEY_6_:
//						inCount = 6;
//						inPageLoop = _PAGE_LOOP_2_;
//						break;
//					case _NEWUI_FUNC_LINE_3_TO_7_Touch_KEY_7_:
//						inCount = 7;
//						inPageLoop = _PAGE_LOOP_2_;
//						break;
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
				/*else if (szKey == _KEY_5_)
				{
					inCount = 5;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}*/
//				else if (szKey == _KEY_6_)
//				{
//					inCount = 6;
//					inPageLoop = _PAGE_LOOP_2_;
//					break;
//				}
//				else if (szKey == _KEY_7_)
//				{
//					inCount = 7;
//					inPageLoop = _PAGE_LOOP_2_;
//					break;
//				}
				/* 銀聯選單插卡或刷卡，直接當銀聯一般交易 */
				else if (inMSR_RetVal == VS_SUCCESS || inEMV_RetVal == VS_SUCCESS)
				{
					inCount = 5;
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
						inRetVal = inMENU_CUP_SALE(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 2:
						inRetVal = inMENU_CUP_REFUND_NORMAL(srEventMenuItem);
						//inRetVal = inMENU_CUP_REFUND(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
//					case 3:
//						inRetVal = inMENU_CUP_INST(srEventMenuItem);
//						if (inRetVal == VS_USER_CANCEL)
//						{
//							inRetVal = VS_ERROR;
//							inPageLoop = _PAGE_LOOP_0_;
//						}
//						else if (inRetVal == VS_FUNC_CLOSE_ERR)
//						{
//							inRetVal = VS_SUCCESS;
//							inPageLoop = _PAGE_LOOP_1_;
//						}
//						else
//						{
//							inPageLoop = _PAGE_LOOP_0_;
//						}
//						break;
//					case 4:
//						inRetVal = inMENU_CUP_REDEEM(srEventMenuItem);
//						if (inRetVal == VS_USER_CANCEL)
//						{
//							inRetVal = VS_ERROR;
//							inPageLoop = _PAGE_LOOP_0_;
//						}
//						else if (inRetVal == VS_FUNC_CLOSE_ERR)
//						{
//							inRetVal = VS_SUCCESS;
//							inPageLoop = _PAGE_LOOP_1_;
//						}
//						else
//						{
//							inPageLoop = _PAGE_LOOP_0_;
//						}
//						break;
					case 3:
						inRetVal = inMENU_CUP_PREAUTH(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					/*case 4:
						inRetVal = inMENU_CUP_MAILORDER(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;*/
					case 4:
						inRetVal = inMENU_CUP_VOID(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					default:
						inPageLoop = _PAGE_LOOP_0_;
						break;
				}
			}

			/* 代表回上一頁，要回復UI */
			if (inPageLoop == _PAGE_LOOP_1_		&&
			    inRetVal == VS_ERROR)
			{
				/* 初始化 */
				inDISP_ClearAll();
				inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_2_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
				inDISP_PutGraphic(_TOUCH_NEWUI_CUP_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
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

	}

	return (inRetVal);
}



/*
Function        :inMENU02_NewUiSettleMenu
Date&Time       :2017/10/25 下午 1:55
Describe        :
*/
int inMENU02_NewUiSettleMenu(EventMenuItem *srEventMenuItem)
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

	if (inMENU_Check_SETTLE_Enable(0) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		/* 計算有效的主機個數 */
		if(inCountEnableHost() == 1)
		{
			inRetVal = inMENU_SETTLE_BY_HOST(srEventMenuItem);
			if (inRetVal == VS_USER_CANCEL)
				inRetVal = VS_ERROR;
			return inRetVal;
		}

		/* 初始化 */
		inDISP_ClearAll();
		inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_7_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
		inDISP_PutGraphic(_TOUCH_NEWUI_SETTLE_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
		inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

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
						inRetVal = inMENU_SETTLE_AUTOSETTLE(srEventMenuItem);
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
						inRetVal = inMENU_SETTLE_BY_HOST(srEventMenuItem);
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
					default:
						inPageLoop = _PAGE_LOOP_0_;
						break;
				}
			}

			/* 代表回上一頁，要回復UI */
			if (inPageLoop == _PAGE_LOOP_1_		&&
			    inRetVal == VS_ERROR)
			{
				/* 初始化 */
				inDISP_ClearAll();
				inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_7_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
				inDISP_PutGraphic(_TOUCH_NEWUI_SETTLE_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
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
	}

	return (inRetVal);
}




/*
Function        :inMENU02_NewUiReviewPrintMenu
Date&Time       :2017/10/25 下午 1:55
Describe        :
*/
int inMENU02_NewUiReviewPrintMenu(EventMenuItem *srEventMenuItem)
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

	if (inMENU_Check_REVIEW_SETTLE_Enable(0) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		/* 初始化 */
		inDISP_ClearAll();
		inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_8_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
		inDISP_PutGraphic(_TOUCH_NEWUI_REVIEW_PRINT_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
		inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

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
					/*case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_:
						inCount = 6;
						inPageLoop = _PAGE_LOOP_2_;
						break;*/
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
				/*else if (szKey == _KEY_6_)
				{
					inCount = 6;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}*/
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
						inRetVal = inMENU_REPRINT(srEventMenuItem);
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
						inRetVal = inMENU_TOTAL_REVIEW(srEventMenuItem);
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
						inRetVal = inMENU_DETAIL_REVIEW(srEventMenuItem);
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
						inRetVal = inMENU_DCC_RATE(srEventMenuItem);
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
					case 4:
						inRetVal = inMENU_TOTAL_REPORT(srEventMenuItem);
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
						inRetVal = inMENU_DETAIL_REPORT(srEventMenuItem);
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
					default:
						inPageLoop = _PAGE_LOOP_0_;
						break;
				}
			}

			/* 代表回上一頁，要回復UI */
			if (inPageLoop == _PAGE_LOOP_1_		&&
			    inRetVal == VS_ERROR)
			{
				/* 初始化 */
				inDISP_ClearAll();
				inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_8_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
				inDISP_PutGraphic(_TOUCH_NEWUI_REVIEW_PRINT_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
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
	}

	return (inRetVal);
}


/*
Function        :inMENU02_NuwUiSettingMenu
Date&Time       :2017/10/25 下午 1:55
Describe        :
*/
int inMENU02_NuwUiSettingMenu(EventMenuItem *srEventMenuItem)
{
	int inPageLoop = _PAGE_LOOP_1_;
	int inRetVal = VS_SUCCESS;
	int inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
	int inCount = 1;
	int inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char szKey = 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_Touch_NONE_				, _TRANS_TYPE_NULL_	, NULL						, ""					}
	};

        /* 初始化 */
	inDISP_ClearAll();
	inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_9_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
	inDISP_PutGraphic(_TOUCH_NEWUI_SETTING_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
	inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

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
				/*case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_:
					inCount = 4;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_:
					inCount = 5;
					inPageLoop = _PAGE_LOOP_2_;
					break;*/
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
			/*else if (szKey == _KEY_4_)
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
			}*/
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
				/*case 1:
					inRetVal = inMENU_EDIT_PASSWORD(srEventMenuItem);
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
				/*case 2:
					inRetVal = inMENU_TRACELOG_UP(srEventMenuItem);
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
				case 1:
					inRetVal = inMENU_Download_Parameter(srEventMenuItem);
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
					inRetVal = inMENU_CHECK_VERSION(srEventMenuItem);
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
					inRetVal = inMENU03_NewNuOtherMenu(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				default:
					inPageLoop = _PAGE_LOOP_0_;
					break;
			}
		}

		/* 代表回上一頁，要回復UI */
		if (inPageLoop == _PAGE_LOOP_1_		&&
		    inRetVal == VS_ERROR)
		{
			/* 初始化 */
			inDISP_ClearAll();
			inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_9_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
			inDISP_PutGraphic(_TOUCH_NEWUI_SETTING_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
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



/*
Function        : inMENU02_VoidTrans
Date&Time   : 
Describe        :
*/
int inMENU02_VoidTrans(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	if (inCREDIT_CheckTransactionFunction(_VOID_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _VOID_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_VOID_;
		srEventMenuItem->inRunTRTID = _TRT_VOID_;
	}

	return (inRetVal);
}


/*
Function	: inMENU02_SvcUiTransactionMenu
Date&Time	: 2022/12/28 下午 7:05
Describe	:
 * [新增SVC功能]  [SAM] 
*/
int inMENU02_SvcUiTransactionMenu(EventMenuItem *srEventMenuItem)
{
	int	inPageLoop = _PAGE_LOOP_1_;
	int	inRetVal = VS_SUCCESS;
	int	inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
	int	inCount = 1;
	int	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char	szKey = 0x00;

	
#if 0 //先不檢核
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_	, _VOID_			, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_1_VOID_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_	, _REFUND_		, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_2_REFUND_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_	, _INST_SALE_		, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_3_INST_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_	, _REDEEM_SALE_	, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_4_REDEEM_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_	, _SALE_OFFLINE_	, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_5_SALEOFFLINE_	},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_	, _TIP_			, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_6_TIP_			},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_7_	, _PRE_AUTH_		, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_7_PREAUTH_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_8_	, _TRANS_TYPE_NULL_	, inMENU_Check_Adjust			, _ICON_HIGHTLIGHT_1_8_ADJUST_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_9_	, _ADJUST_			, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_9_ADJUST_		},
		{_Touch_NONE_				, _TRANS_TYPE_NULL_	, NULL			, ""					}
	};
#endif
 
#if 0 //先不檢核
	if (inMENU_Check_Transaction_Enable(0) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
#endif
	{
		/* 初始化 */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_SVC_SELECT_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
		inDISP_PutGraphic(_TOUCH_SVC_FUNCTION_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
		inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

#if 0 //先不檢核
		/* 檢查功能開關，並顯示反白的圖 */
		inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
#endif
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
				}else if (szKey == _KEY_5_)
				{
					inCount = 5;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}else if (szKey == _KEY_6_)
				{
					inCount = 6;
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
						inRetVal = inMENU_SVC_ACTIVECARD_AUTORELOAD(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 2:
						inRetVal = inMENU_SVC_INQUIRY(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 3:
						inRetVal = inMENU_SVC_RE_ACTIVECARD(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 4:
						inRetVal = inMENU_SVC_REFUND(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 5:
						inRetVal = inMENU_SVC_BACKCARD(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 6:
						inRetVal = inMENU_SVC_RELOAD(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					default:
						inPageLoop = _PAGE_LOOP_0_;
						break;
				}
			}

			/* 代表回上一頁，要回復UI */
			if (inPageLoop == _PAGE_LOOP_1_		&&
			    inRetVal == VS_ERROR)
			{
				/* 初始化 */
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SVC_SELECT_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
				inDISP_PutGraphic(_TOUCH_SVC_FUNCTION_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
				inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);
#if 0 //先不檢核
				inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
#endif
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
			}
			/* 功能未開而返回，不刷畫面 */
			else if (inPageLoop == _PAGE_LOOP_1_	&&
				 inRetVal == VS_SUCCESS)
			{

			}
		}/* _PAGE_LOOP_0_ */
	}

	return (inRetVal);
}
