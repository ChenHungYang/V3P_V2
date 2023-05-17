/*
 * 主要使用在設定畫面第三層的函式，命名規則如下。
 *  inMENU06_RebootFunc
 *  in: 回傳值型態
 *  MENU06 : 顯示UI層級
 *  RebootFunc : 函式功能名稱  
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
#include "../Flow.h"

#include "../../FUNCTION/CREDIT_FUNC/CreditCheckFunc.h"
#include "../../FUNCTION/Function.h"

/*
Function        : inMENU06_RebootFunc
Date&Time   : 20190509
Describe        :
*/
int inMENU06_RebootFunc(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_REBOOT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜重新開機＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	srEventMenuItem->inRunOperationID = _OPERATION_EDC_REBOOT_;
	srEventMenuItem->inRunTRTID = FALSE;

	inFunc_Reboot();

	return (inRetVal);
}