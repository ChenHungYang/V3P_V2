
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>

#include "../../../SOURCE/INCLUDES/Define_1.h"
#include "../../../SOURCE/INCLUDES/Define_2.h"
#include "../../../SOURCE/INCLUDES/TransType.h"
#include "../../../SOURCE/INCLUDES/Transaction.h"
#include "../../../SOURCE/DISPLAY/Display.h"
#include "../../../SOURCE/DISPLAY/DispMsg.h"
#include "../../../SOURCE/DISPLAY/DisTouch.h"

#include "../../EVENT/MenuMsg.h"

#include "../../FUNCTION/Function.h"
#include "../../FUNCTION/CFGT.h"

#include "CupCheckFunc.h"


/*
Function        : inCUP_CheckVoidFunc
Date&Time   : 2019/03/12
Describe        : 檔非銀聯交易的取消
*/
int inCUP_CheckVoidFunc(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;

        if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
        {
                /* 顯示此交易已取消 */
                inRetVal = inDISP_Msg_BMP(_ERR_CANCEL_ALREADY_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

		return (inRetVal);
        }
	/* 擋非銀聯交易 按銀聯鍵取消 */
	else if (pobTran->srBRec.uszCUPTransBit != VS_TRUE)
	{
		/* 請勿按銀聯鍵 */
		inRetVal = inDISP_Msg_BMP(_ERR_NOT_PRESS_CUP_BUTTON_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

		return (inRetVal);
	}
	else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		/* 請依正確卡別操作 */
		inRetVal = inDISP_Msg_BMP(_ERR_PLEASE_FOLLOW_RIGHT_OPT_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

		return (inRetVal);
	}

	if (pobTran->inTransactionCode == _CUP_VOID_)
	{
		if (pobTran->srBRec.inOrgCode == _CUP_PRE_AUTH_)
		{
			/* 請按預先授權取消 */
			inRetVal = inDISP_Msg_BMP(_ERR_PLEASE_PRESS_PREAUTH_VOID_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

			return (inRetVal);
		}
		else if (pobTran->srBRec.inOrgCode == _CUP_PRE_COMP_)
		{
			/* 請按授權完成取消 */
			inRetVal = inDISP_Msg_BMP(_ERR_PLEASE_PRESS_PRECOMP_VOID_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

			return (inRetVal);
		}
		else if (pobTran->srBRec.inOrgCode == _LOYALTY_REDEEM_)
		{
			/* 無交易紀錄 */
			inRetVal = inDISP_Msg_BMP(_ERR_RECORD_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

			return (inRetVal);
		}

	}
	else if (pobTran->inTransactionCode == _CUP_PRE_AUTH_VOID_)
	{
		if (pobTran->srBRec.inOrgCode == _CUP_SALE_	||
		    pobTran->srBRec.inOrgCode == _CUP_REFUND_	||
		    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
		{
			/* 請按銀聯取消 */
			inRetVal = inDISP_Msg_BMP(_ERR_PLEASE_PRESS_CUP_VOID_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

			return (inRetVal);
		}
		else if (pobTran->srBRec.inOrgCode == _CUP_PRE_COMP_)
		{
			/* 請按授權完成取消 */
			inRetVal = inDISP_Msg_BMP(_ERR_PLEASE_PRESS_PRECOMP_VOID_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

			return (inRetVal);
		}
		else if (pobTran->srBRec.inOrgCode == _LOYALTY_REDEEM_)
		{
			/* 無交易紀錄 */
			inRetVal = inDISP_Msg_BMP(_ERR_RECORD_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

			return (inRetVal);
		}
	}
	else if (pobTran->inTransactionCode == _CUP_PRE_COMP_VOID_)
	{
		if (pobTran->srBRec.inOrgCode == _CUP_SALE_	||
		    pobTran->srBRec.inOrgCode == _CUP_REFUND_	||
		    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
		{
			/* 請按銀聯取消 */
			inRetVal = inDISP_Msg_BMP(_ERR_PLEASE_PRESS_CUP_VOID_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

			return (inRetVal);
		}
		else if (pobTran->srBRec.inOrgCode == _CUP_PRE_AUTH_)
		{
			/* 請按預先授權取消 */
			inRetVal = inDISP_Msg_BMP(_ERR_PLEASE_PRESS_PREAUTH_VOID_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

			return (inRetVal);
		}
		else if (pobTran->srBRec.inOrgCode == _LOYALTY_REDEEM_)
		{
			/* 無交易紀錄 */
			inRetVal = inDISP_Msg_BMP(_ERR_RECORD_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

			return (inRetVal);
		}
	}



	return (inRetVal);
}


/*
Function        : inCUP_CheckRefuneLimitFunc
Date&Time   : 2019/03/13
Describe        : 不能超出CFGT設定的限額
*/
int inCUP_CheckRefuneLimitFunc(TRANSACTION_OBJECT *pobTran)
{
	char	szCUPRefundLimit[12 + 1];
	char	szTemplate[10 + 1];

	/* CUP退貨限額(含)，包含兩位小數位, 預設值="999999999900" */
	memset(szCUPRefundLimit, 0x00, sizeof(szCUPRefundLimit));
	inGetCUPRefundLimit(szCUPRefundLimit);

	/* 只取整數位 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, szCUPRefundLimit, 10);

	if (pobTran->srBRec.lnTxnAmount > atol(szTemplate))
	{
		/* 退貨金額超過上限 */
		inDISP_Msg_BMP(_ERR_REFUND_OVER_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}






