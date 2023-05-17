
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


#include "RedeemProcessDataFunc.h"

/*
Function        : inREDEEM_MakeRedeemDataFunc
Date&Time   : 20190312
Describe        : 用來確認紅利所需資料
*/
int inREDEEM_MakeRedeemDataFunc(TRANSACTION_OBJECT *pobTran)
{
	memset(pobTran->srBRec.szRedeemIndicator, 0x00, sizeof(pobTran->srBRec.szRedeemIndicator));
	if (pobTran->srBRec.lnRedemptionPaidCreditAmount == 0L)
		strcpy(pobTran->srBRec.szRedeemIndicator, "1");
	else
		strcpy(pobTran->srBRec.szRedeemIndicator, "2");

	/* 紅利扣抵剩餘紅利點數，不輸入也不印 */
	pobTran->srBRec.lnRedemptionPointsBalance = 0L;
	/* Sign of Balance(剩餘點數之正負值)	‘-‘，表負值	‘ ’，表正值 */
	pobTran->srBRec.szRedeemSignOfBalance[0] = ' ';


        return (VS_SUCCESS);
}

