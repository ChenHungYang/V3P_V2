
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctosapi.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <sqlite3.h>

#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/TransType.h"

#include "../SOURCE/FUNCTION/Function.h"

#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DisTouch.h"

#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Menu.h"
#include "../SOURCE/EVENT/Flow.h"

/*
Function	: inMENU_SVC_ACTIVECARD_AUTORELOAD
Date&Time	: 2022/12/26 下午 5:18
Describe	:
*/
int inMENU_SVC_ACTIVECARD_AUTORELOAD(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
//先不檢核，直接執行
//	if (inCREDIT_CheckTransactionFunction(_VOID_) != VS_SUCCESS)
//	{
//		inRetVal = VS_FUNC_CLOSE_ERR;
//	}
//	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);	/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_SVC_ACTIVE_AUTOLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示  "開卡自動儲值" */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _SVC_ACTIVECARD_AUTORELOAD_;

		/* 第一層輸入密碼 */
//		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
//			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_SVC_CARD_;
		srEventMenuItem->inRunTRTID = _TRT_SVC_ACTIVECARD_;
	}

	return (inRetVal);
}


/*
Function	: inMENU_SVC_INQUIRY
Date&Time	: 2022/12/26 下午 5:18
Describe	:
*/
int inMENU_SVC_INQUIRY(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
//先不檢核，直接執行
//	if (inCREDIT_CheckTransactionFunction(_VOID_) != VS_SUCCESS)
//	{
//		inRetVal = VS_FUNC_CLOSE_ERR;
//	}
//	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);	/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_SVC_ACTIVE_AUTOLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示  "開卡自動儲值" */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _SVC_INQUIRY_;

		/* 第一層輸入密碼 */
//		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
//			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_SVC_CARD_;
		srEventMenuItem->inRunTRTID = _TRT_SVC_INQUIRY_;
	}

	return (inRetVal);
}

/*
Function	: inMENU_SVC_RE_ACTIVECARD
Date&Time	: 2022/12/26 下午 5:18
Describe	:
*/
int inMENU_SVC_RE_ACTIVECARD(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
//先不檢核，直接執行
//	if (inCREDIT_CheckTransactionFunction(_VOID_) != VS_SUCCESS)
//	{
//		inRetVal = VS_FUNC_CLOSE_ERR;
//	}
//	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);	/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_SVC_ACTIVE_AUTOLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示  "開卡自動儲值" */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _SVC_ACTIVECARD_NOT_AUTORELOAD_;

		/* 第一層輸入密碼 */
//		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
//			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_SVC_CARD_;
		srEventMenuItem->inRunTRTID = _SVC_ACTIVECARD_NOT_AUTORELOAD_;
	}

	return (inRetVal);
}


/*
Function	: inMENU_SVC_REFUND
Date&Time	: 2022/12/26 下午 5:18
Describe	:
 * 購物卡退貨
*/
int inMENU_SVC_REFUND(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
//先不檢核，直接執行
//	if (inCREDIT_CheckTransactionFunction(_VOID_) != VS_SUCCESS)
//	{
//		inRetVal = VS_FUNC_CLOSE_ERR;
//	}
//	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);	/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_SVC_ACTIVE_AUTOLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示  "開卡自動儲值" */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _SVC_REFUND_;

		/* 第一層輸入密碼 */
//		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
//			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_SVC_CARD_;
		srEventMenuItem->inRunTRTID = _TRT_SVC_REFUND_;
	}

	return (inRetVal);
}



/*
Function	: inMENU_SVC_BACKCARD
Date&Time	: 2022/12/26 下午 5:18
Describe	: 
 * 退卡
*/
int inMENU_SVC_BACKCARD(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
//先不檢核，直接執行
//	if (inCREDIT_CheckTransactionFunction(_VOID_) != VS_SUCCESS)
//	{
//		inRetVal = VS_FUNC_CLOSE_ERR;
//	}
//	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);	/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_SVC_ACTIVE_AUTOLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示  "開卡自動儲值" */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _SVC_BACKCARD_;

		/* 第一層輸入密碼 */
//		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
//			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_SVC_CARD_;
		srEventMenuItem->inRunTRTID = _TRT_SVC_BACKCARD_;
	}

	return (inRetVal);
}


/*
Function	: inMENU_SVC_RELOAD
Date&Time	: 2022/12/26 下午 5:18
Describe	: 
 * 儲值
*/
int inMENU_SVC_RELOAD(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
//先不檢核，直接執行
//	if (inCREDIT_CheckTransactionFunction(_VOID_) != VS_SUCCESS)
//	{
//		inRetVal = VS_FUNC_CLOSE_ERR;
//	}
//	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);	/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_SVC_ACTIVE_AUTOLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示  "開卡自動儲值" */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _SVC_RELOAD_;

		/* 第一層輸入密碼 */
//		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
//			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_SVC_CARD_;
		srEventMenuItem->inRunTRTID = _TRT_SVC_RELOAD_;
	}

	return (inRetVal);
}


