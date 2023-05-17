

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>

#include "../../INCLUDES/Define_1.h"
#include "../../INCLUDES/Define_2.h"
#include "../../INCLUDES/TransType.h"
#include "../../INCLUDES/Transaction.h"
#include "../../DISPLAY/Display.h"
#include "../../DISPLAY/DispMsg.h"
#include "../../DISPLAY/DisTouch.h"
#include "../../EVENT/MenuMsg.h"

#include "../../FUNCTION/Function.h"

#include "../FILE_FUNC/File.h"
#include "../FILE_FUNC/FIleLogFunc.h"


#include "CreditFlowDispFunc.h"

/*
 Function		: inCREDIT_ConfirmToRunVoidTrans
Date&Time	: 2019/03/12
Describe		: 按0確認是否要進行取消交易
 */
int inCREDIT_ConfirmToRunVoidTrans(TRANSACTION_OBJECT *pobTran) {
	char szAuthCodeMsg[_DISP_MSG_SIZE_ + 1];
	char szAmountMsg[_DISP_MSG_SIZE_ + 1];
	char szKey;
	RTC_NEXSYS srRTC; /* Date & Time */

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	/* 確認金額 */
	if (pobTran->srBRec.lnTotalTxnAmount <= 0) {

		inDISP_DispLogAndWriteFlie("  CreditVoidConfirm  Amt[%ld] *Error* Line[%d]", pobTran->srBRec.lnTotalTxnAmount, __LINE__);
		return (VS_ERROR);
	}


	memset(szAuthCodeMsg, 0x00, sizeof (szAuthCodeMsg));
	memset(szAmountMsg, 0x00, sizeof (szAmountMsg));

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 提示檢核金額和授權碼 */
	inDISP_PutGraphic(_CHECK_AUTH_AMT_, 0, _COORDINATE_Y_LINE_8_4_);
	/* 顯示授權碼 */
	sprintf(szAuthCodeMsg, "%s", pobTran->srBRec.szAuthCode);
	inDISP_EnglishFont_Point_Color(szAuthCodeMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _COLOR_WHITE_, 11);

	/* 顯示金額 */
	if (pobTran->srBRec.inOrgCode == _REFUND_ || pobTran->srBRec.inOrgCode == _INST_REFUND_ ||
			pobTran->srBRec.inOrgCode == _REDEEM_REFUND_) {
		sprintf(szAmountMsg, "%ld", 0 - pobTran->srBRec.lnTotalTxnAmount);
		inFunc_Amount_Comma(szAmountMsg, "NT$", ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
	} else {
		sprintf(szAmountMsg, "%ld", pobTran->srBRec.lnTotalTxnAmount);
		inFunc_Amount_Comma(szAmountMsg, "NT$", ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
	}

	inDISP_EnglishFont_Point_Color(szAmountMsg, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_RED_, _COLOR_WHITE_, 8);

	while (1) {
		/* 抓取 KioskFlag的值，如為 TRUE 就不用確認畫面 2020/3/6 下午 4:28 [SAM] */
		if (inFunc_GetKisokFlag() == VS_TRUE) {
			szKey = _KEY_0_;
			inDISP_DispLogAndWriteFlie(" FU Kiosk Credit ConfVoidTrans Line[%d]", __LINE__);
		} else {
			szKey = uszKBD_GetKey(_EDC_TIMEOUT_);
		}

		if (szKey == _KEY_0_) {
			/* pobTran->uszUpdateBatchBit 表示 uszUpdateBatchBit / TRANS_BATCH_KEY】是要更新記錄 */
			/* 暫時放這裡 */
			pobTran->uszUpdateBatchBit = VS_TRUE;
			pobTran->srBRec.uszVOIDBit = VS_TRUE;
			pobTran->srBRec.inOrgCode = pobTran->srBRec.inCode;
			pobTran->srBRec.inCode = pobTran->inTransactionCode;

			/* 因ESC需用新的日期為索引,所以重新取得 20181220 [SAM]*/
			memset(&srRTC, 0x00, sizeof (RTC_NEXSYS));
			if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS) {
				inDISP_DispLogAndWriteFlie("  CreditVoidConfirm  ESC Get SysTime *Error* Line[%d]", __LINE__);
				return (VS_USER_CANCEL);
			}
			inFunc_Sync_BRec_ESC_Date_Time(pobTran, &srRTC);

			/* 因為重新交易需要簽名，所以要把參數設回初始值  20190103 [SAM] */
			pobTran->srBRec.uszF56NoSignatureBit = VS_FALSE;
			break;
		} else if (szKey == _KEY_CANCEL_) {
			inDISP_DispLogAndWriteFlie("  CreditVoidConfirm  CANCEL *Error* Line[%d]", __LINE__);
			return (VS_USER_CANCEL);
		} else if (szKey == _KEY_TIMEOUT_) {
			inDISP_DispLogAndWriteFlie("  CreditVoidConfirm  TIMEOUT *Error* Line[%d]", __LINE__);
			return (VS_TIMEOUT);
		}
	}

	return (VS_SUCCESS);
}
