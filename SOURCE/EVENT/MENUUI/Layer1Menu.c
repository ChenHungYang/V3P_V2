/*
 * 主要使用在設定畫面第一層的函式，命名規則如下。
 *  inMENU01_NewUiTransactionMenu
 *  in: 回傳值型態
 *  MENU01 : 顯示UI層級
 *  NewUiTransactionMenu : 函式功能名稱  
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctosapi.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <sqlite3.h>
#include <soundplayer.h>

#include "../../INCLUDES/Define_1.h"
#include "../../INCLUDES/Define_2.h"
#include "../../INCLUDES/Transaction.h"
#include "../../INCLUDES/TransType.h"
#include "../../PRINT/Print.h"
#include "../../DISPLAY/Display.h"
#include "../../DISPLAY/DisTouch.h"

#include "../MenuMsg.h"
#include "../Menu.h"
#include "../Flow.h"

#include "../../FUNCTION/CREDIT_FUNC/CreditCheckFunc.h"
#include "../../FUNCTION/Function.h"
#include "../../FUNCTION/EDC.h"
#include "../../FUNCTION/CFGT.h"
#include "../../FUNCTION/Sqlite.h"

#include "../../FUNCTION/Signpad.h"

#include "../../FUNCTION/FILE_FUNC/File.h"
#include "../../FUNCTION/FILE_FUNC/FIleLogFunc.h"

#include "../../FUNCTION/HDPT.h"

#include "../../FUNCTION/ECR_FUNC/ECR.h"
#include "../../FUNCTION/MULTI_FUNC/MultiFunc.h"
#include "../../FUNCTION/MULTI_FUNC/MultiHostFunc.h"

/* [新增電票悠遊卡功能]  [SAM] 2022/6/13 下午 3:00 */
#include "../../FUNCTION/TDT.h"
#include "../../../NCCC/NCCCTicketSrc.h"

#include "../../TMS/EDCTmsFlow.h"
#include "../../TMS/EDCTmsDefine.h"
#include "../../TMS/EDCTmsFileProc.h"

#include "Layer1Menu.h"
#include "Layer2Menu.h"


extern unsigned char guszCTLSInitiOK;
extern int ginIdleMSRStatus, ginIdleICCStatus, ginMenuKeyIn; 
extern int	ginMachineType;

extern MULTI_TABLE	gstMultiOb;


/*
Function	: inMENU01_FunctionNewUi
Date&Time	:2017/11/3 下午 3:42
Describe	:
*/
int inMENU01_FunctionNewUi(EventMenuItem *srEventMenuItem)
{
	int	inPageLoop = _PAGE_LOOP_1_;
	int	inRetVal = VS_SUCCESS;
	int	inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
	int	inCount= 1;
	int	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char	szKey = 0x00;
		
	/* 測試用參數，只在這頁使用 */
	TRANSACTION_OBJECT tpobTran;
	
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_	, _TRANS_TYPE_NULL_	, inMENU_Check_Transaction_Enable	, _ICON_HIGHTLIGHT_1_TRANSACTION_},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_	, _TRANS_TYPE_NULL_	, inMENU_Check_CUP_Enable		, _ICON_HIGHTLIGHT_2_CUP_                 },
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_	, _TRANS_TYPE_NULL_	, inMENU_Check_SETTLE_Enable	, _ICON_HIGHTLIGHT_7_SETTLE_	},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_	, _TRANS_TYPE_NULL_	, inMENU_Check_REVIEW_SETTLE_Enable		, _ICON_HIGHTLIGHT_8_REVIEW_PRINT_		},
		{_Touch_NONE_				, _TRANS_TYPE_NULL_	, NULL					, ""					}
	};

	/* 初始化 */
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第1.5層顯示 LOGO */
	inDISP_PutGraphic(_TOUCH_NEWUI_FUNCTION_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
	inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);
	
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
			

			/* 接觸式螢幕判斷  */
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
				case _NEWUI_RETURN_IDLE_BUTTON_:
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
			
			/* 實體按鍵判斷  */
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
			else if (szKey == _KEY_ALPHA_)
			{
				inCount = 7;
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

			inFunc_Display_All_Status_By_Machine_Type();
		}

		/* 判斷MENU點進去按鈕之後要做的事情 */
		if (inPageLoop == _PAGE_LOOP_2_)
		{
			switch (inCount)
			{
				case 1:
#ifdef __SVC_ENABLE__
					inRetVal = inMENU02_SvcUiTransactionMenu(srEventMenuItem);
#else					
					inRetVal = inMENU02_NewUiTransactionMenu(srEventMenuItem);
#endif

//					inRetVal = inCMAS_MENU_ICON(srEventMenuItem);/* 替代功能測試悠遊卡用 FOR_TEST_USE */
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
				case 2:
					inRetVal = inMENU02_NuwUiCupMenu(srEventMenuItem);
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
					inRetVal = inMENU02_NewUiSettleMenu(srEventMenuItem);
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
				case 4:
					inRetVal = inMENU02_NewUiReviewPrintMenu(srEventMenuItem);
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
				case 5:
					inRetVal = inMENU02_NuwUiSettingMenu(srEventMenuItem);
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
				case 6:
//					inTestJason();
//					inSVC_CalMac(&tpobTran, "999999999999999999999999", 24);
//					TTTT();
					//inFunc_TestBase64();
#ifdef __MUTI_FUCN_TEST_				
										
//					inRetVal = inMultiFunc_JsonCallSlave(&tpobTran, _MULTI_SIGNPAD_NO_, &gstMultiOb);
//
//					memcpy(tpobTran.srBRec.szAuthCode, "123456", 6);
//					inSIGN_CheckSignatureOfVerticalView(&tpobTran);
					
//					inDISP_Timer_Start(_TIMER_NEXSYS_4_, 10);
//					while(1)
//					{
//						inRetVal = inMultiFunc_Host_RecvCtlsData(&tpobTran);
//						if (inTimerGet(_TIMER_NEXSYS_4_) == 0)
//						{
//							inDISP_LogPrintfWithFlag(" MultiFunc Receive Time ");
//							return (VS_TIMEOUT);
//						}
//						
//						if(VS_SUCCESS == inRetVal)
//							break;
//					}
//					
//					if(VS_SUCCESS == inRetVal)
//					{
//						inMultiFunc_JsonCallSlave(&tpobTran, _MULTI_TRANS_STOP_NO_, &gstMultiOb);
//					}else{
//						inDISP_LogPrintfWithFlag(" MultiFunc Receive *Error* RetVal[%d]", inRetVal);
//					}
					
					inDISP_LogPrintfWithFlag(" MultiFunc Receive Rval[%d] ",inRetVal);	
#endif
					/* 驗證簽名功能用，不能拿 [SAM] */
					memset(&tpobTran, 0x00, sizeof(tpobTran));
					inRetVal = inSIGN_TestSignatureView(&tpobTran);
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
				case 7:
//					inSVC_PRINT_TestPrintDetail(&tpobTran);
					/* [新增SVC功能]  [SAM] */
					srEventMenuItem->inCode = _SVC_INQUIRY_;
					srEventMenuItem->inRunOperationID = _OPERATION_SVC_CARD_;
					srEventMenuItem->inRunTRTID = _TRT_SVC_INQUIRY_;
					inPageLoop = _PAGE_LOOP_0_;
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
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第1.5層顯示 LOGO */
			inDISP_PutGraphic(_TOUCH_NEWUI_FUNCTION_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
			inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);
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
Function	:  inMENU01_MenuKeyInAndGetAmount
Date&Time	:2015/8/24 上午 11:49
Describe	: idle輸入
*/
int inMENU01_MenuKeyInAndGetAmount(EventMenuItem *srEventMenuItem)
{
	char	szCTLSEnable[2 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);

	inDISP_LogPrintfWithFlag("  ContactlessEnable [%s] ", szCTLSEnable);	
		
	if (!memcmp(&szCTLSEnable[0], "Y", 1) && srEventMenuItem->inEventCode == '0')
	{
		/* 不接受金額第一位為0 */
		inDISP_LogPrintfWithFlag(" Input Amount Is Zero ");
		return (VS_ERROR);
	}
	
	/* 第三層顯示 ＜一般交易＞ */
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */

	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
	srEventMenuItem->inCode = _SALE_;
	
	inDISP_LogPrintfWithFlag("  guszCTLSInitiOK [%c] ", (guszCTLSInitiOK == VS_TRUE) ? 'Y' : 'N' );	
	
	/* 這邊先設定TRT是因為在抓卡號時inFunc_ResetTitle這隻函數會用到 */
	if (!memcmp(&szCTLSEnable[0], "Y", 1) && guszCTLSInitiOK == VS_TRUE)
	{
		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;

		/* Contactless功能開啟時，信用卡正向(銷售)交易管理號碼在輸入金額後 */
		srEventMenuItem->inRunOperationID = _OPERATION_SALE_CTLS_IDLE_;
		srEventMenuItem->inRunTRTID = _TRT_SALE_CTLS_;
	}
	else
	{
		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inRunOperationID = _OPERATION_SALE_ICC_;
		srEventMenuItem->inRunTRTID = _TRT_SALE_;
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function	: inMENU01_Swipe
Date&Time : 2015/8/24 上午 11:49
Describe	:磁條流程
*/
int inMENU01_Swipe(EventMenuItem *srEventMenuItem)
{
        inDISP_ClearAll();
        inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */
        /* 輸入密碼的層級 */
        srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
        srEventMenuItem->inCode = _SALE_;

        /* 第一層輸入密碼 */
        if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
                return (VS_ERROR);

        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
        srEventMenuItem->inCode = _SALE_;
        srEventMenuItem->inRunOperationID = _OPERATION_SALE_ICC_;
        srEventMenuItem->inRunTRTID = _TRT_SALE_;

        /* Idle 刷卡流程 */
        ginIdleMSRStatus = VS_TRUE;

        return (VS_SUCCESS);
}



/*
Function	:inMENU01_ICC
Date&Time	:2015/8/24 上午 11:49
Describe	:晶片卡流程
*/
int inMENU01_ICC(EventMenuItem *srEventMenuItem)
{
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
	srEventMenuItem->inCode = _SALE_;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = _SALE_;
	srEventMenuItem->inRunOperationID = _OPERATION_SALE_ICC_;
	srEventMenuItem->inRunTRTID = _TRT_SALE_ICC_;

	/* Idle 插晶片卡流程 */
	ginIdleICCStatus = VS_TRUE;
	return (VS_SUCCESS);
}


/*
Function	  :  inMENU01_MultiFunctionSlave
Date&Time  : 2017/6/30 下午 6:17
Describe	  :  當外接設備時
*/
int inMENU01_MultiFunctionSlave(EventMenuItem *srEventMenuItem)
{
	inDISP_ClearAll();
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
        /* 第三層顯示 ＜ECR連線＞ */
//	inDISP_PutGraphic(_ECR_CONNECTING_, 0, _COORDINATE_Y_LINE_8X16_4_);
        /* 這裡不設incode */

	/* 這裡不設incode */
	srEventMenuItem->inRunOperationID = _OPERATION_MULTIFUNC_SLAVE_;
	/* 這裡不設TRT */

	srEventMenuItem->uszMultiFuncSlaveBit = VS_TRUE;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (VS_SUCCESS);
}


/*
Function	:inMENU01_ECR
Date&Time	:2016/6/23 上午 11:18
Describe	:ECR連線事件
*/
int inMENU01_ECR(EventMenuItem *srEventMenuItem)
{
	inDISP_ClearAll();

	/* 第三層顯示 ＜ECR連線＞ */
	inDISP_PutGraphic(_ECR_CONNECTING_, 0, _COORDINATE_Y_LINE_8_4_);
	/* 這裡不設incode */

	/* 這裡不設incode */
	srEventMenuItem->inRunOperationID = _OPERATION_ECR_TRANSACTION_;
	/* 這裡不設TRT */

	/* ECR的Bit On起來，表示是收銀機發動交易 */
	srEventMenuItem->uszECRBit = VS_TRUE;

	return (VS_SUCCESS);
}



/*
Function	:inMENU01_EscIdleUpload
Date&Time	:2016/6/23 上午 11:18
Describe	:IDLE上傳ESC事件
*/
int inMENU01_EscIdleUpload(EventMenuItem *srEventMenuItem)
{
        /* 這裡不設incode */
        srEventMenuItem->inRunOperationID = _OPERATION_ESC_IDLE_UPLOAD_;
        /* 這裡不設TRT */

        return (VS_SUCCESS);
}

/*
Function	:inMENU01_DccSchedule
Date&Time	:2016/10/21 下午 3:18
Describe	:DCC排程下載
*/
int inMENU01_DccSchedule(EventMenuItem *srEventMenuItem)
{
        /* 這裡不設incode */
        srEventMenuItem->inRunOperationID = _OPERATION_DCC_SCHEDULE_;
        /* 這裡不設TRT */

        return (VS_SUCCESS);
}



/*
Function	:inMENU01_TmsDccSchedule
Date&Time	:2016/10/21 下午 3:18
Describe	:TMS連動DCC下載
*/
int inMENU01_TmsDccSchedule(EventMenuItem *srEventMenuItem)
{
        /* 這裡不設incode */
        srEventMenuItem->inRunOperationID = _OPERATION_TMS_DCC_SCHEDULE_;
        /* 這裡不設TRT */

        return (VS_SUCCESS);
}



/*
Function	:inMENU01_TmsSchduleInquire
Date&Time	:2016/12/6 下午 5:54
Describe	:TMS排程詢問
*/
int inMENU01_TmsSchduleInquire(EventMenuItem *srEventMenuItem)
{
	/* 這裡不設incode */
        srEventMenuItem->inRunOperationID = _OPERATION_TMS_SCHEDULE_INQUIRE_;
        /* 這裡不設TRT */

	return (VS_SUCCESS);
}



/*
Function	:inMENU01_TmsSchduleDownload
Date&Time	:2017/1/25 下午 3:09
Describe	:TMS排程下載
*/
int inMENU01_TmsSchduleDownload(EventMenuItem *srEventMenuItem)
{
	/* 這裡不設incode */
	srEventMenuItem->inRunOperationID = _OPERATION_TMS_SCHEDULE_DOWNLOAD_;
	/* 這裡不設TRT */

	return (VS_SUCCESS);
}



/*
Function	:inMENU01_TmsProcessEffective
Date&Time	:2016/12/6 下午 5:54
Describe	:TMS IDLE檢查生效
*/
int inMENU01_TmsProcessEffective(EventMenuItem *srEventMenuItem)
{
	/* 這裡不設incode */
        srEventMenuItem->inRunOperationID = _OPERATION_TMS_PROCESS_EFFECTIVE_;
        /* 這裡不設TRT */

	return (VS_SUCCESS);
}



/*
Function	:inMENU01_EdcBooting
Date&Time	:2017/10/2 下午 5:31
Describe	:開機流程
*/
int inMENU01_EdcBooting(EventMenuItem *srEventMenuItem)
{
	/* 這裡不設incode */
#if defined	_LOAD_KEY_AP_
        srEventMenuItem->inRunOperationID = _OPERATION_EDC_LOAD_KEY_BOOTING_;
#else
	if (ginMachineType == _CASTLE_TYPE_V3UL_ ||
            ginMachineType == _CASTLE_TYPE_V3P_  ||
            ginMachineType == _CASTLE_TYPE_V3C_)
	{
		inDISP_LogPrintf("inMENU01_EdcBooting V3UL or V3P or V3C");
                srEventMenuItem->inRunOperationID = _OPERATION_EDC_V3UL_BOOTING_;
	}
	else
	{
		inDISP_LogPrintf("inMENU01_EdcBooting Normal Booting");
                srEventMenuItem->inRunOperationID = _OPERATION_EDC_BOOTING_;
	}
#endif
        /* 這裡不設TRT */

	return (VS_SUCCESS);
}



/*
Function	:inMENU01_PowerManagement
Date&Time	:2018/3/21 上午 11:01
Describe	:電量管理流程
*/
int inMENU01_PowerManagement(EventMenuItem *srEventMenuItem)
{
	char	szPWMMode[2 + 1] = {0};

	memset(szPWMMode, 0x00, sizeof(szPWMMode));
	inGetPWMMode(szPWMMode);

	/* 這裡不設incode */
	if (memcmp(szPWMMode, _PWM_MODE_01_SLEEP_, strlen(_PWM_MODE_01_SLEEP_)) == 0)
	{
		srEventMenuItem->inRunOperationID = _OPERATION_POWER_SAVING_SLEEP_;
	}
	else if (memcmp(szPWMMode, _PWM_MODE_00_STANDBY_, strlen(_PWM_MODE_00_STANDBY_)) == 0)
	{
		srEventMenuItem->inRunOperationID = _OPERATION_POWER_SAVING_STANDBY_;
	}
	else
	{

	}
        /* 這裡不設TRT */

	return (VS_SUCCESS);
}


