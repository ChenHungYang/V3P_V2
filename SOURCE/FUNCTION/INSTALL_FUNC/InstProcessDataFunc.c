
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

#include "InstProcessDataFunc.h"

/*
Function        : inINST_MakeInstDataFunc
Date&Time   : 2019/03/12
Describe        : 用來確認分期所需資料
*/
int inINST_MakeInstDataFunc(TRANSACTION_OBJECT *pobTran)
{
	/* 聯合分期付款_【“I”: Service Fee Included】【“E”: Service Fee Excluded】 */
	memset(pobTran->srBRec.szInstallmentIndicator, 0x00, sizeof(pobTran->srBRec.szInstallmentIndicator));
	if (pobTran->srBRec.lnInstallmentFormalityFee == 0L)
		strcpy(pobTran->srBRec.szInstallmentIndicator, "I");
	else
		strcpy(pobTran->srBRec.szInstallmentIndicator, "E");

        return (VS_SUCCESS);
}

