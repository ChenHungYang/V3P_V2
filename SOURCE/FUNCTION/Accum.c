#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>

#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"
#include "../EVENT/Flow.h"
#include "../EVENT/MenuMsg.h"
#include "Sqlite.h"
#include "Accum.h"
#include "Card.h"
#include "HDT.h"
#include "HDPT.h"
#include "FILE_FUNC/File.h"
#include "FILE_FUNC/FIleLogFunc.h"
#include "FuncTable.h"
#include "Function.h"
#include "EDC.h"


#include "../FUNCTION/AccountFunction/DispBillInfo.h"
#include "../FUNCTION/UNIT_FUNC/TimeUint.h"

/* [新增電票悠遊卡功能] 新增電票顯示條件 [SAM] 2022/6/22 下午 2:38*/
#include"../../CMAS/CMASsrc.h"

extern int	ginDebug; /* Debug使用 extern */

/*
Function        :inACCUM_UpdateTotalAmount
Date&Time       :2015/9/4 下午 14:00
Describe        :每當有交易時，到此fuction更新筆數和總額，回傳(VS_SUCCESS) or (VS_ERROR)
*/
int inACCUM_UpdateTotalAmount(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec)
{

	//char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

	/* inACCUM_UpdateTotalAmount()_START */
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 不是取消的交易 */
	if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
	{
		switch (pobTran->srBRec.inCode)
		{
			case _REDEEM_SALE_ :
				srAccumRec->lnRedeemSaleCount ++;
				srAccumRec->lnRedeemTotalCount ++;

				srAccumRec->llRedeemSaleAmount += pobTran->srBRec.lnTxnAmount;
				srAccumRec->llRedeemTotalAmount += pobTran->srBRec.lnTxnAmount;
				//srAccumRec->llRedeemOrigSaleAmount+= pobTran->srBRec.lnTxnAmount;
				//srAccumRec->llRedeemOrigTotalAmount += pobTran->srBRec.lnTxnAmount;

				srAccumRec->lnRedeemTotalPoint += pobTran->srBRec.lnRedemptionPoints;
			case _REDEEM_ADJUST_ :
				if(pobTran->srBRec.inCode == _REDEEM_ADJUST_)
				{
					srAccumRec->lnRedeemSaleCount ++;
					srAccumRec->lnRedeemTotalCount ++;

					srAccumRec->llRedeemSaleAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llRedeemTotalAmount += pobTran->srBRec.lnTxnAmount;

					
					//srAccumRec->llRedeemOrigSaleAmount += pobTran->srBRec.lnBaseTransactionAmount;
					//srAccumRec->llRedeemOrigTotalAmount += pobTran->srBRec.lnBaseTransactionAmount;

					srAccumRec->lnRedeemTotalPoint += pobTran->srBRec.lnRedemptionPoints;
				}
			case _INST_SALE_ :
				if(pobTran->srBRec.inCode == _INST_SALE_)
				{
					srAccumRec->lnInstSaleCount ++;
					srAccumRec->lnInstTotalCount ++;

					srAccumRec->llInstSaleAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llInstTotalAmount += pobTran->srBRec.lnTxnAmount;
				}
			case _INST_ADJUST_ :
				if(pobTran->srBRec.inCode == _INST_ADJUST_)
				{
					srAccumRec->lnInstSaleCount ++;
					srAccumRec->lnInstTotalCount ++;

					srAccumRec->llInstSaleAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llInstTotalAmount += pobTran->srBRec.lnTxnAmount;
				}
			case _SALE_ :
			case _SALE_OFFLINE_ :
			case _PRE_COMP_ :
			case _CUP_SALE_ :
			case _CUP_PRE_COMP_ :

				srAccumRec->lnTotalSaleCount ++;
				srAccumRec->lnTotalCount ++;

				srAccumRec->llTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
				srAccumRec->llTotalAmount += pobTran->srBRec.lnTxnAmount;

				if (pobTran->srBRec.uszRedeemBit != VS_TRUE && pobTran->srBRec.uszInstallmentBit != VS_TRUE)
				{

					srAccumRec->lnTotalCreditSaleCount ++;
					srAccumRec->lnTotalCreditCount ++;

					srAccumRec->llTotalCreditSaleAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llTotalCreditAmount += pobTran->srBRec.lnTxnAmount;
				}

				break;
			case _REDEEM_REFUND_ :

				srAccumRec->lnRedeemRefundCount ++;
				srAccumRec->lnRedeemTotalCount ++;

				srAccumRec->llRedeemRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
				srAccumRec->llRedeemTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);

				//srAccumRec->llRedeemOrigRefundAmount += (0 - (pobTran->srBRec.lnBaseTransactionAmount));
				//srAccumRec->llRedeemOrigTotalAmount += (0 - (pobTran->srBRec.lnBaseTransactionAmount));

				srAccumRec->lnRedeemTotalPoint += (0 - pobTran->srBRec.lnRedemptionPoints);
			case _INST_REFUND_ :
				if(pobTran->srBRec.inCode == _INST_REFUND_)
				{

					srAccumRec->lnInstRefundCount ++;
					srAccumRec->lnInstTotalCount ++;

					srAccumRec->llInstRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llInstTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
				}
			case _REFUND_ :
			case _CUP_REFUND_ :

				srAccumRec->lnTotalRefundCount ++;
				srAccumRec->lnTotalCount ++;

				srAccumRec->llTotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
				srAccumRec->llTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);

				if (pobTran->srBRec.uszRedeemBit != VS_TRUE && pobTran->srBRec.uszInstallmentBit != VS_TRUE)
				{

					srAccumRec->lnTotalCreditRefundCount ++;
					srAccumRec->lnTotalCreditCount ++;

					srAccumRec->llTotalCreditRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llTotalCreditAmount += (0 - pobTran->srBRec.lnTxnAmount);
				}

				break;
			case _TIP_ :

				srAccumRec->lnTotalTipsCount ++;
//				srAccumRec->lnTotalCount ++;

				srAccumRec->llTotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
				srAccumRec->llTotalAmount += pobTran->srBRec.lnTipTxnAmount;

				if (pobTran->srBRec.uszRedeemBit != VS_TRUE && pobTran->srBRec.uszInstallmentBit != VS_TRUE)
				{

					srAccumRec->lnTotalCreditTipsCount ++;
	//				srAccumRec->lnTotalCreditCount ++;

					srAccumRec->llTotalCreditTipsAmount += pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llTotalCreditAmount += pobTran->srBRec.lnTipTxnAmount;
				}

				break;
			case _ADJUST_:
				srAccumRec->llTotalSaleAmount -= (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnAdjustTxnAmount);
				srAccumRec->llTotalAmount -= (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnAdjustTxnAmount);

				if (pobTran->srBRec.uszRedeemBit != VS_TRUE && pobTran->srBRec.uszInstallmentBit != VS_TRUE)
				{
					srAccumRec->llTotalCreditSaleAmount -= (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnAdjustTxnAmount);
					srAccumRec->llTotalCreditAmount -= (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnAdjustTxnAmount);
				}

				break;
			default :
				inDISP_DispLogAndWriteFlie("  Accum Update Total Amt *Error* InCode[%d] Line[%d]", pobTran->srBRec.inCode,  __LINE__);
				return (VS_ERROR);
		}
	}
	/* 取消的時候 */
	else
	{
		switch (pobTran->srBRec.inOrgCode)
		{
			case _REDEEM_SALE_ :

				srAccumRec->lnRedeemSaleCount --;
				srAccumRec->lnRedeemTotalCount --;

				srAccumRec->llRedeemSaleAmount -= pobTran->srBRec.lnTxnAmount;
				srAccumRec->llRedeemTotalAmount -= pobTran->srBRec.lnTxnAmount;

				//srAccumRec->llRedeemOrigSaleAmount -= pobTran->srBRec.lnBaseTransactionAmount;
				//srAccumRec->llRedeemOrigTotalAmount -= pobTran->srBRec.lnBaseTransactionAmount;

				srAccumRec->lnRedeemTotalPoint -= pobTran->srBRec.lnRedemptionPoints;
			case _REDEEM_ADJUST_ :
				if(pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_)
				{

					srAccumRec->lnRedeemSaleCount --;
					srAccumRec->lnRedeemTotalCount --;

					srAccumRec->llRedeemSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llRedeemTotalAmount -= pobTran->srBRec.lnTxnAmount;

					//srAccumRec->llRedeemOrigSaleAmount -= pobTran->srBRec.lnBaseTransactionAmount;
					//srAccumRec->llRedeemOrigTotalAmount -= pobTran->srBRec.lnBaseTransactionAmount;

					srAccumRec->lnRedeemTotalPoint -= pobTran->srBRec.lnRedemptionPoints;
				}
			case _INST_SALE_ :
				if(pobTran->srBRec.inOrgCode == _INST_SALE_)
				{

					srAccumRec->lnInstSaleCount --;
					srAccumRec->lnInstTotalCount --;

					srAccumRec->llInstSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llInstTotalAmount -= pobTran->srBRec.lnTxnAmount;
				}
			case _INST_ADJUST_ :
				if(pobTran->srBRec.inOrgCode == _INST_ADJUST_)
				{
					srAccumRec->lnInstTotalCount --;
					srAccumRec->lnInstSaleCount --;
					/* �p����B */
					srAccumRec->llInstSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llInstTotalAmount -= pobTran->srBRec.lnTxnAmount;
				}
			case _SALE_ :
			case _SALE_OFFLINE_ :
			case _PRE_COMP_ :
			case _CUP_SALE_ :
			case _CUP_PRE_COMP_ :
			case _ADJUST_ :

				srAccumRec->lnTotalSaleCount --;
				srAccumRec->lnTotalCount --;

				if(pobTran->srBRec.inOrgCode == _ADJUST_)
					srAccumRec->llTotalSaleAmount -= pobTran->srBRec.lnAdjustTxnAmount;
				else
					srAccumRec->llTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;

				if(pobTran->srBRec.inOrgCode == _ADJUST_)
					srAccumRec->llTotalAmount -= pobTran->srBRec.lnAdjustTxnAmount;
				else
					srAccumRec->llTotalAmount -= pobTran->srBRec.lnTxnAmount;

				if (pobTran->srBRec.uszRedeemBit != VS_TRUE && pobTran->srBRec.uszInstallmentBit != VS_TRUE)
				{

					srAccumRec->lnTotalCreditSaleCount --;
					srAccumRec->lnTotalCreditCount --;

					if(pobTran->srBRec.inOrgCode == _ADJUST_)
						srAccumRec->llTotalCreditSaleAmount -= pobTran->srBRec.lnAdjustTxnAmount;
					else
						srAccumRec->llTotalCreditSaleAmount -= pobTran->srBRec.lnTxnAmount;

					if(pobTran->srBRec.inOrgCode == _ADJUST_)
						srAccumRec->llTotalCreditAmount -= pobTran->srBRec.lnAdjustTxnAmount;
					else
						srAccumRec->llTotalCreditAmount -= pobTran->srBRec.lnTxnAmount;
				}

				break;
			case _REDEEM_REFUND_ :

				srAccumRec->lnRedeemRefundCount --;
				srAccumRec->lnRedeemTotalCount --;

				srAccumRec->llRedeemRefundAmount += pobTran->srBRec.lnTxnAmount;
				srAccumRec->llRedeemTotalAmount += pobTran->srBRec.lnTxnAmount;

				//srAccumRec->llRedeemOrigRefundAmount += pobTran->srBRec.lnBaseTransactionAmount;
				//srAccumRec->llRedeemOrigTotalAmount += pobTran->srBRec.lnBaseTransactionAmount;

				srAccumRec->lnRedeemTotalPoint += pobTran->srBRec.lnRedemptionPoints;
			case _INST_REFUND_ :
				if(pobTran->srBRec.inOrgCode == _INST_REFUND_)
				{

					srAccumRec->lnInstRefundCount --;
					srAccumRec->lnInstTotalCount --;

					srAccumRec->llInstRefundAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llInstTotalAmount += pobTran->srBRec.lnTxnAmount;
				}
			case _REFUND_ :
			case _CUP_REFUND_ :

				srAccumRec->lnTotalRefundCount --;
				srAccumRec->lnTotalCount --;

				srAccumRec->llTotalRefundAmount += pobTran->srBRec.lnTxnAmount;
				srAccumRec->llTotalAmount += pobTran->srBRec.lnTxnAmount;

				if (pobTran->srBRec.uszRedeemBit != VS_TRUE && pobTran->srBRec.uszInstallmentBit != VS_TRUE)
				{

					srAccumRec->lnTotalCreditRefundCount --;
					srAccumRec->lnTotalCreditCount --;

					srAccumRec->llTotalCreditRefundAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llTotalCreditAmount += pobTran->srBRec.lnTxnAmount;
				}

				break;
			case _TIP_ :

				srAccumRec->lnTotalTipsCount --;
				srAccumRec->lnTotalSaleCount --;
//				srAccumRec->lnTotalCount -= 2;

				srAccumRec->llTotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
				srAccumRec->llTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
				srAccumRec->llTotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);

				if (pobTran->srBRec.uszRedeemBit != VS_TRUE && pobTran->srBRec.uszInstallmentBit != VS_TRUE)
				{

					srAccumRec->lnTotalCreditTipsCount --;
					srAccumRec->lnTotalCreditSaleCount --;
	//				srAccumRec->lnTotalCreditCount -= 2;

					srAccumRec->llTotalCreditTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llTotalCreditSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llTotalCreditAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
				}

				break;
			default :
				inDISP_DispLogAndWriteFlie("  Accum Update Total Amt *Error* inOrgCode[%d] Line[%d]", pobTran->srBRec.inOrgCode,  __LINE__);
				return (VS_ERROR);
		}
	}

	/* inACCUM_UpdateTotalAmount()_END */
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inACCUM_UpdateTotalAmount()_END");
	}

	return (VS_SUCCESS);
}

/*
Function        :inACCUM_UpdateTotalAmountByCard
Date&Time       :2015/9/4 下午 15:00
Describe        :每當有交易時，到此fuction更新"卡別"的筆數和總額，回傳(VS_SUCCESS) or (VS_ERROR)
*/
int inACCUM_UpdateTotalAmountByCard(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec)
{
	/* inACCUM_UpdateTotalAmountByCard()_START */
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inACCUM_UpdateTotalAmountByCard()_START");
	}

	/* 不是取消的交易 */
	if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
	{
		switch (pobTran->srBRec.inCode)
		{
			case _SALE_:
			case _SALE_OFFLINE_:/* Offline Sale */
			case _INST_SALE_:
			case _REDEEM_SALE_:
			case _CASH_ADVANCE_ :
			case _FORCE_CASH_ADVANCE_ :
			case _PRE_COMP_ :
			case _CUP_SALE_ :
			case _CUP_PRE_COMP_ :
			case _FISC_SALE_ :
			case _INST_ADJUST_ :
			case _REDEEM_ADJUST_ :
			case _MAIL_ORDER_ :
			case _CUP_MAIL_ORDER_ :
				if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_VISA_, 4))
				{
					/* 計算筆數 */
					srAccumRec->lnVisaTotalSaleCount ++;
					srAccumRec->lnVisaTotalCount ++;
					/* 計算金額 */
					srAccumRec->llVisaTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llVisaTotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_MASTERCARD_, 10))
				{
					/* 計算筆數 */
					srAccumRec->lnMasterTotalSaleCount ++;
					srAccumRec->lnMasterTotalCount ++;
					/* 計算金額 */
					srAccumRec->llMasterTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llMasterTotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_JCB_, 3))
				{
					/* 計算筆數 */
					srAccumRec->lnJcbTotalSaleCount ++;
					srAccumRec->lnJcbTotalCount ++;
					/* 計算金額 */
					srAccumRec->llJcbTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llJcbTotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_U_CARD_, 6))
				{
					/* 計算筆數 */
					srAccumRec->lnUCardTotalSaleCount ++;
					srAccumRec->lnUCardTotalCount ++;
					/* 計算金額 */
					srAccumRec->llUCardTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llUCardTotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_AMEX_, 4))
				{
					/* 計算筆數 */
					srAccumRec->lnAmexTotalSaleCount ++;
					srAccumRec->lnAmexTotalCount ++;
					/* 計算金額 */
					srAccumRec->llAmexTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llAmexTotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_CUP_, 3))
				{
					/* 計算筆數 */
					srAccumRec->lnCupTotalSaleCount ++;
					srAccumRec->lnCupTotalCount ++;
					/* 計算金額 */
					srAccumRec->llCupTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llCupTotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_DINERS_, 6))
				{
					/* 計算筆數 */
					srAccumRec->lnDinersTotalSaleCount ++;
					srAccumRec->lnDinersTotalCount ++;
					/* 計算金額 */
					srAccumRec->llDinersTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llDinersTotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_SMARTPAY_, 8))
				{
					/* 計算筆數 */
					srAccumRec->lnFiscTotalSaleCount ++;
					srAccumRec->lnFiscTotalCount ++;
					/* 計算金額 */
					srAccumRec->llFiscTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llFiscTotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else
				{
					/* 卡別錯誤，沒有此卡別 */
					inDISP_DispLogAndWriteFlie("  Accum Update Total By Card Label *Error* InCode[%d] szCardLabel[%s] Line[%d]", pobTran->srBRec.inCode, pobTran->srBRec.szCardLabel,  __LINE__);
					return (VS_ERROR);
				}

				break;
			case _REFUND_:
			case _CUP_REFUND_:
			case _CUP_MAIL_ORDER_REFUND_ :
			case _INST_REFUND_:
			case _REDEEM_REFUND_:
			case _FISC_REFUND_ :
				if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_VISA_, 4))
				{
					/* 計算筆數 */
					srAccumRec->lnVisaTotalRefundCount ++;
					srAccumRec->lnVisaTotalCount ++;
					/* 計算金額 */
					srAccumRec->llVisaTotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llVisaTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_MASTERCARD_, 10))
				{
					/* 計算筆數 */
					srAccumRec->lnMasterTotalRefundCount ++;
					srAccumRec->lnMasterTotalCount ++;
					/* 計算金額 */
					srAccumRec->llMasterTotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llMasterTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_JCB_, 3))
				{
					/* 計算筆數 */
					srAccumRec->lnJcbTotalRefundCount ++;
					srAccumRec->lnJcbTotalCount ++;
					/* 計算金額 */
					srAccumRec->llJcbTotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llJcbTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_U_CARD_, 6))
				{
					/* 計算筆數 */
					srAccumRec->lnUCardTotalRefundCount ++;
					srAccumRec->lnUCardTotalCount ++;
					/* 計算金額 */
					srAccumRec->llUCardTotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llUCardTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_AMEX_, 4))
				{
					/* 計算筆數 */
					srAccumRec->lnAmexTotalRefundCount ++;
					srAccumRec->lnAmexTotalCount ++;
					/* 計算金額 */
					srAccumRec->llAmexTotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llAmexTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_CUP_, 3))
				{
					/* 計算筆數 */
					srAccumRec->lnCupTotalRefundCount ++;
					srAccumRec->lnCupTotalCount ++;
					/* 計算金額 */
					srAccumRec->llCupTotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llCupTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_DINERS_, 6))
				{
					/* 計算筆數 */
					srAccumRec->lnDinersTotalRefundCount ++;
					srAccumRec->lnDinersTotalCount ++;
					/* 計算金額 */
					srAccumRec->llDinersTotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llDinersTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_SMARTPAY_, 8))
				{
					/* 計算筆數 */
					srAccumRec->lnFiscTotalRefundCount ++;
					srAccumRec->lnFiscTotalCount ++;
					/* 計算金額 */
					srAccumRec->llFiscTotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llFiscTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
				}
				else
				{
					/* 卡別錯誤，沒有此卡別 */
					/* debug */
					inDISP_DispLogAndWriteFlie("  Accum Update Total By Card Label *Error* InCode[%d] szCardLabel[%s] Line[%d]", pobTran->srBRec.inCode, pobTran->srBRec.szCardLabel,  __LINE__);
					return (VS_ERROR);
				}

				break;
			case _TIP_:
				if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_VISA_, 4))
				{
					/* 計算筆數 */
					srAccumRec->lnVisaTotalTipsCount++;
					/* 計算金額 */
					srAccumRec->llVisaTotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llVisaTotalAmount += pobTran->srBRec.lnTipTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_MASTERCARD_, 10))
				{
					/* 計算筆數 */
					srAccumRec->lnMasterTotalTipsCount++;
					/* 計算金額 */
					srAccumRec->llMasterTotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llMasterTotalAmount += pobTran->srBRec.lnTipTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_JCB_, 3))
				{
					/* 計算筆數 */
					srAccumRec->lnJcbTotalTipsCount++;
					/* 計算金額 */
					srAccumRec->llJcbTotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llJcbTotalAmount += pobTran->srBRec.lnTipTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_U_CARD_, 6))
				{
					/* 計算筆數 */
					srAccumRec->lnUCardTotalTipsCount++;
					/* 計算金額 */
					srAccumRec->llUCardTotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llUCardTotalAmount += pobTran->srBRec.lnTipTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_AMEX_, 4))
				{
					/* 計算筆數 */
					srAccumRec->lnAmexTotalTipsCount++;
					/* 計算金額 */
					srAccumRec->llAmexTotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llAmexTotalAmount += pobTran->srBRec.lnTipTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_CUP_, 3))
				{
					/* 計算筆數 */
					srAccumRec->lnCupTotalTipsCount++;
					/* 計算金額 */
					srAccumRec->llCupTotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llCupTotalAmount += pobTran->srBRec.lnTipTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_DINERS_, 6))
				{
					/* 計算筆數 */
					srAccumRec->lnDinersTotalTipsCount++;
					/* 計算金額 */
					srAccumRec->llDinersTotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llDinersTotalAmount += pobTran->srBRec.lnTipTxnAmount;
				}
				else
				{
					/* 卡別錯誤，沒有此卡別 */
					inDISP_DispLogAndWriteFlie("  Accum Update Total By Card Label *Error* InCode[%d] szCardLabel[%s] Line[%d]", pobTran->srBRec.inCode, pobTran->srBRec.szCardLabel,  __LINE__);
					return (VS_ERROR);
				}

				break;
			case _ADJUST_:
				if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_VISA_, 4))
				{
					/* 計算金額 */
					srAccumRec->llVisaTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llVisaTotalAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llVisaTotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
					srAccumRec->llVisaTotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_MASTERCARD_, 10))
				{
					/* 計算金額 */
					srAccumRec->llMasterTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llMasterTotalAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llMasterTotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
					srAccumRec->llMasterTotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_JCB_, 3))
				{
					/* 計算金額 */
					srAccumRec->llJcbTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llJcbTotalAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llJcbTotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
					srAccumRec->llJcbTotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_U_CARD_, 6))
				{
					/* 計算金額 */
					srAccumRec->llUCardTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llUCardTotalAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llUCardTotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
					srAccumRec->llUCardTotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_AMEX_, 4))
				{
					/* 計算金額 */
					srAccumRec->llAmexTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llAmexTotalAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llAmexTotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
					srAccumRec->llAmexTotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_CUP_, 3))
				{
					/* 計算金額 */
					srAccumRec->llCupTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llCupTotalAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llCupTotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
					srAccumRec->llCupTotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_DINERS_, 6))
				{
					/* 計算金額 */
					srAccumRec->llDinersTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llDinersTotalAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llDinersTotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
					srAccumRec->llDinersTotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_SMARTPAY_, 8))
				{
					/* 計算金額 */
					srAccumRec->llFiscTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llFiscTotalAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llFiscTotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
					srAccumRec->llFiscTotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
				}
				else
				{
					/* 卡別錯誤，沒有此卡別 */
					inDISP_DispLogAndWriteFlie("  Accum Update Total By Card  Label *Error* InCode[%d] szCardLabel[%s] Line[%d]", pobTran->srBRec.inCode, pobTran->srBRec.szCardLabel,  __LINE__);
					return (VS_ERROR);
				}

				break;
			default :
				/* 交易類別錯誤，(沒有此交易類別) */
				inDISP_DispLogAndWriteFlie("  Accum Update Total By Card InCode *Error* InCode[%d] Line[%d]", pobTran->srBRec.inCode, __LINE__);
				return (VS_ERROR);
			}
		}
	/* 取消的時候 */
	else
	{
		switch (pobTran->srBRec.inOrgCode)
		{
			case _SALE_:
			case _SALE_OFFLINE_:/* Offline Sale */
			case _INST_SALE_:
			case _REDEEM_SALE_:
			case _CASH_ADVANCE_ :
			case _FORCE_CASH_ADVANCE_ :
			case _PRE_COMP_ :
			case _CUP_SALE_ :
			case _CUP_PRE_COMP_ :
			case _FISC_SALE_ :
			case _INST_ADJUST_ :
			case _REDEEM_ADJUST_ :
			case _MAIL_ORDER_ :
			case _CUP_MAIL_ORDER_ :
				if (!memcmp(&pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, 4))
				{
					/* 計算筆數 */
					srAccumRec->lnVisaTotalSaleCount --;
					srAccumRec->lnVisaTotalCount --;
					/* 計算金額 */
					srAccumRec->llVisaTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llVisaTotalAmount -= pobTran->srBRec.lnTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_MASTERCARD_, 10))
				{
					/* 計算筆數 */
					srAccumRec->lnMasterTotalSaleCount --;
					srAccumRec->lnMasterTotalCount --;
					/* 計算金額 */
					srAccumRec->llMasterTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llMasterTotalAmount -= pobTran->srBRec.lnTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_JCB_, 3))
				{
					/* 計算筆數 */
					srAccumRec->lnJcbTotalSaleCount --;
					srAccumRec->lnJcbTotalCount --;
					/* 計算金額 */
					srAccumRec->llJcbTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llJcbTotalAmount -= pobTran->srBRec.lnTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_U_CARD_, 6))
				{
					/* 計算筆數 */
					srAccumRec->lnUCardTotalSaleCount --;
					srAccumRec->lnUCardTotalCount --;
					/* 計算金額 */
					srAccumRec->llUCardTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llUCardTotalAmount -= pobTran->srBRec.lnTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_AMEX_, 4))
				{
					/* 計算筆數 */
					srAccumRec->lnAmexTotalSaleCount --;
					srAccumRec->lnAmexTotalCount --;
					/* 計算金額 */
					srAccumRec->llAmexTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llAmexTotalAmount -= pobTran->srBRec.lnTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_CUP_, 3))
				{
					/* 計算筆數 */
					srAccumRec->lnCupTotalSaleCount --;
					srAccumRec->lnCupTotalCount --;
					/* 計算金額 */
					srAccumRec->llCupTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llCupTotalAmount -= pobTran->srBRec.lnTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_DINERS_, 6))
				{
					/* 計算筆數 */
					srAccumRec->lnDinersTotalSaleCount --;
					srAccumRec->lnDinersTotalCount --;
					/* 計算金額 */
					srAccumRec->llDinersTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llDinersTotalAmount -= pobTran->srBRec.lnTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_SMARTPAY_, 8))
				{
					/* 計算筆數 */
					srAccumRec->lnFiscTotalSaleCount --;
					srAccumRec->lnFiscTotalCount --;
					/* 計算金額 */
					srAccumRec->llFiscTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llFiscTotalAmount -= pobTran->srBRec.lnTxnAmount;
				}
				else
				{
					/* 卡別錯誤，沒有此卡別 */
					inDISP_DispLogAndWriteFlie("  Accum Update Total By Card  Label *Error* inOrgCode[%d] szCardLabel[%s] Line[%d]", pobTran->srBRec.inOrgCode, pobTran->srBRec.szCardLabel,  __LINE__);
					return (VS_ERROR);
				}

				break;
			case _REFUND_:
			case _CUP_REFUND_:
			case _CUP_MAIL_ORDER_REFUND_ :
			case _INST_REFUND_:
			case _REDEEM_REFUND_:
				/* SmartPay退貨不能取消 */
				if (!memcmp(&pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, 4))
				{
					/* 計算筆數 */
					srAccumRec->lnVisaTotalRefundCount --;
					srAccumRec->lnVisaTotalCount --;
					/* 計算金額 */
					srAccumRec->llVisaTotalRefundAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llVisaTotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_MASTERCARD_, 10))
				{
					/* 計算筆數 */
					srAccumRec->lnMasterTotalRefundCount --;
					srAccumRec->lnMasterTotalCount --;
					/* 計算金額 */
					srAccumRec->llMasterTotalRefundAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llMasterTotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_JCB_, 3))
				{
					/* 計算筆數 */
					srAccumRec->lnJcbTotalRefundCount --;
					srAccumRec->lnJcbTotalCount --;
					/* 計算金額 */
					srAccumRec->llJcbTotalRefundAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llJcbTotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_U_CARD_, 6))
				{
					/* 計算筆數 */
					srAccumRec->lnUCardTotalRefundCount --;
					srAccumRec->lnUCardTotalCount --;
					/* 計算金額 */
					srAccumRec->llUCardTotalRefundAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llUCardTotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_AMEX_, 4))
				{
					/* 計算筆數 */
					srAccumRec->lnAmexTotalRefundCount --;
					srAccumRec->lnAmexTotalCount --;
					/* 計算金額 */
					srAccumRec->llAmexTotalRefundAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llAmexTotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_CUP_, 3))
				{
					/* 計算筆數 */
					srAccumRec->lnCupTotalRefundCount --;
					srAccumRec->lnCupTotalCount --;
					/* 計算金額 */
					srAccumRec->llCupTotalRefundAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llCupTotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_DINERS_, 6))
				{
					/* 計算筆數 */
					/* 計算筆數 */
					srAccumRec->lnDinersTotalRefundCount --;
					srAccumRec->lnDinersTotalCount --;
					/* 計算金額 */
					srAccumRec->llDinersTotalRefundAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llDinersTotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_SMARTPAY_, 8))
				{
					/* 計算筆數 */
					/* 計算筆數 */
					srAccumRec->lnFiscTotalRefundCount --;
					srAccumRec->lnFiscTotalCount --;
					/* 計算金額 */
					srAccumRec->llFiscTotalRefundAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llFiscTotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else
				{
					/* 卡別錯誤，沒有此卡別 */
					inDISP_DispLogAndWriteFlie("  Accum Update Total By Card  Label *Error* inOrgCode[%d] szCardLabel[%s] Line[%d]", pobTran->srBRec.inOrgCode, pobTran->srBRec.szCardLabel,  __LINE__);
					return (VS_ERROR);
				}

				break;
			case _TIP_:
				if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_VISA_, 4))
				{
					/* 取消整筆交易，計算筆數 */
					srAccumRec->lnVisaTotalTipsCount --;
					srAccumRec->lnVisaTotalSaleCount --;
					srAccumRec->lnVisaTotalCount --;
					/* 取消整筆交易，計算金額 */
					srAccumRec->llVisaTotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llVisaTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llVisaTotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_MASTERCARD_, 10))
				{
					/* 取消整筆交易，計算筆數 */
					srAccumRec->lnMasterTotalTipsCount --;
					srAccumRec->lnMasterTotalSaleCount --;
					srAccumRec->lnMasterTotalCount --;
					/* 取消整筆交易，計算金額 */
					srAccumRec->llMasterTotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llMasterTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llMasterTotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_JCB_, 3))
				{
					/* 取消整筆交易，計算筆數 */
					srAccumRec->lnJcbTotalTipsCount --;
					srAccumRec->lnJcbTotalSaleCount --;
					srAccumRec->lnJcbTotalCount --;
					/* 取消整筆交易，計算金額 */
					srAccumRec->llJcbTotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llJcbTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llJcbTotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_U_CARD_, 6))
				{
					/* 取消整筆交易，計算筆數 */
					srAccumRec->lnUCardTotalTipsCount --;
					srAccumRec->lnUCardTotalSaleCount --;
					srAccumRec->lnUCardTotalCount --;
					/* 取消整筆交易，計算金額 */
					srAccumRec->llUCardTotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llUCardTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llUCardTotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_AMEX_, 4))
				{
					/* 取消整筆交易，計算筆數 */
					srAccumRec->lnAmexTotalTipsCount --;
					srAccumRec->lnAmexTotalSaleCount --;
					srAccumRec->lnAmexTotalCount --;
					/* 取消整筆交易，計算金額 */
					srAccumRec->llAmexTotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llAmexTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llAmexTotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_CUP_, 3))
				{
					/* 取消整筆交易，計算筆數 */
					srAccumRec->lnCupTotalTipsCount --;
					srAccumRec->lnCupTotalSaleCount --;
					srAccumRec->lnCupTotalCount --;
					/* 取消整筆交易，計算金額 */
					srAccumRec->llCupTotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llCupTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llCupTotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_DINERS_, 6))
				{
					/* 計算筆數 */
					srAccumRec->lnDinersTotalTipsCount --;
					srAccumRec->lnDinersTotalSaleCount --;
					srAccumRec->lnDinersTotalCount --;
					/* 計算金額 */
					srAccumRec->llDinersTotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llDinersTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llDinersTotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
				}
				else
				{
					/* 卡別錯誤，沒有此卡別 */
					inDISP_DispLogAndWriteFlie("  Accum Update Total By Card  Label *Error* inOrgCode[%d] szCardLabel[%s] Line[%d]", pobTran->srBRec.inOrgCode, pobTran->srBRec.szCardLabel,  __LINE__);
					return (VS_ERROR);
				}

				break;
			case _ADJUST_:
				if (!memcmp(&pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, 4))
				{
					/* 計算金額 */
					//srAccumRec->llVisaTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					//srAccumRec->llVisaTotalAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llVisaTotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
					srAccumRec->llVisaTotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_MASTERCARD_, 10))
				{
					/* 計算金額 */
					//srAccumRec->llMasterTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					//srAccumRec->llMasterTotalAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llMasterTotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
					srAccumRec->llMasterTotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_JCB_, 3))
				{
					/* 計算金額 */
					//srAccumRec->llJcbTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					//srAccumRec->llJcbTotalAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llJcbTotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
					srAccumRec->llJcbTotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);


				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_U_CARD_, 6))
				{
					/* 計算金額 */
					//srAccumRec->llUCardTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					//srAccumRec->llUCardTotalAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llUCardTotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
					srAccumRec->llUCardTotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);

				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_AMEX_, 4))
				{
					/* 計算金額 */
					//srAccumRec->llAmexTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					//srAccumRec->llAmexTotalAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llAmexTotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
					srAccumRec->llAmexTotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);

				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_CUP_, 3))
				{
					/* 計算金額 */
					//srAccumRec->llCupTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					//srAccumRec->llCupTotalAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llCupTotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
					srAccumRec->llCupTotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_DINERS_, 6))
				{
					/* 計算金額 */
					//srAccumRec->llDinersTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					//srAccumRec->llDinersTotalAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llDinersTotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
					srAccumRec->llDinersTotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
				}
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_SMARTPAY_, 8))
				{
					/* 計算金額 */
					//srAccumRec->llFiscTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					//srAccumRec->llFiscTotalAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llFiscTotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
					srAccumRec->llFiscTotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
				}
				else
				{
					/* 卡別錯誤，沒有此卡別 */
					
					inDISP_DispLogAndWriteFlie("  Accum Update Total By Card  Label *Error* inOrgCode[%d] szCardLabel[%s] Line[%d]", pobTran->srBRec.inOrgCode, pobTran->srBRec.szCardLabel,  __LINE__);
					return (VS_ERROR);
				}

				break;
			default:
				/* 交易類別錯誤，(沒有此交易類別) */
				inDISP_DispLogAndWriteFlie("  Accum Update Total By Card inOrgCode *Error* inOrgCode[%d] Line[%d]", pobTran->srBRec.inOrgCode,  __LINE__);
				return (VS_ERROR);
			}
		}

		/* inACCUM_UpdateTotalAmountByCard()_END */
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("----------------------------------------");
			inDISP_LogPrintf("inACCUM_UpdateTotalAmountByCard()_END");
		}

		return (VS_SUCCESS);
	}

/*
Function        :inACCUM_StoreRecord
Date&Time       :2015/9/4 下午 15:30
Describe        :再做加總後，將資料存回record，回傳(VS_SUCCESS) or (VS_ERROR)
*/
int inACCUM_StoreRecord(ACCUM_TOTAL_REC *srAccumRec,unsigned char *uszFileName)
{
        unsigned long   ulHandlePtr;    /* File Handle，type為pointer */
        int inRetVal; /* return value，來判斷是否回傳error */

        /* inACCUM_StoreRecord()_START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inACCUM_StoreRecord()_START 1(%s)",uszFileName);
        }

        /*開檔*/
        inRetVal = inFILE_Open(&ulHandlePtr, uszFileName);
        /*開檔成功*/
        if (inRetVal != VS_SUCCESS)
        {
                /* 開檔失敗時，不必關檔(開檔失敗，handle回傳NULL) */
                /* 開檔失敗，所以回傳error */
                return(VS_ERROR);
        }

        /* 把指針指到開頭*/
        inRetVal = inFILE_Seek(ulHandlePtr, 0, _SEEK_BEGIN_);

        /* seek不成功時 */
        if (inRetVal != VS_SUCCESS)
        {
                /* inFILE_Seek失敗時 */
                /* 關檔並回傳VS_ERROR */
                inFILE_Close(&ulHandlePtr);
                /* seek失敗，所以回傳error。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }

        /* 寫檔 */
        if (inFILE_Write(&ulHandlePtr,(unsigned char*)srAccumRec,_ACCUM_REC_SIZE_) != VS_SUCCESS)
        {
                /*寫檔失敗時*/
                /* 關檔  */
                inFILE_Close(&ulHandlePtr);
                /* 寫檔失敗，所以回傳error。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }

        /* 關檔  */
        if (inFILE_Close(&ulHandlePtr) != VS_SUCCESS)
        {
                /*關檔失敗時*/
                /* 關檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /* inACCUM_StoreRecord()_END */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inACCUM_StoreRecord()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inACCUM_GetRecord
Date&Time       :2015/9/4 下午 16:00
Describe        :將.amt中的資料讀取到srAccumRec來做使用，回傳(VS_SUCCESS) or (VS_ERROR)
*/
int inACCUM_GetRecord(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec)
{
        unsigned long   ulHandlePtr;				/* File Handle */
        int             inRetVal;                               /* return value，來判斷是否回傳error */
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        unsigned char   uszFileName[14 + 1];                    /* 儲存交易金額檔案的檔案名稱(最大為15) */

        /* inACCUM_GetRecord()_START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inACCUM_GetRecord()_START");
        }

	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _ACCUM_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

        /* 純開檔不建檔  */
        if (inFILE_OpenReadOnly(&ulHandlePtr, uszFileName) == (VS_ERROR))
        {
                /* 開檔失敗時 */
                /* 開檔錯誤，確認是否有檔案，若有檔案仍錯誤，則可能是handle的問題 */
                if (inFILE_Check_Exist(uszFileName) != (VS_ERROR))
                {
                        /* 開啟失敗，但檔案存在回傳error */
                        return (VS_ERROR);
                }
                /* 如果沒有檔案時，則為沒有交易紀錄，回傳NO_RECORD並印空簽單 */
                else
                {
                        return (VS_NO_RECORD);
                }

        }
        /* 開檔成功 */
        else
        {
                /*先清空srAccumRec 為讀檔案作準備  */
                memset(srAccumRec, 0x00, sizeof(srAccumRec));

                /* 把指針指到開頭*/
                inRetVal = inFILE_Seek(ulHandlePtr, 0, _SEEK_BEGIN_);

                /* inFILE_Seek，成功時 */
                if (inRetVal != VS_SUCCESS)
                {
                        /* inFILE_Seek失敗時 */
                        /* 關檔並回傳VS_ERROR */
                        inFILE_Close(&ulHandlePtr);

                        /* Seek檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
                        return (VS_ERROR);
                }

                /*確認檔案大小*/
                if (lnFILE_GetSize(&ulHandlePtr, uszFileName) == 0)
                {
                        /* 長度為0，不必讀 */
                        /* debug */
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("lnFILE_GetSize ＝＝ 0 ：");
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "(FName = %s), (Ptr = %d)", uszFileName, (int)ulHandlePtr);
                                inDISP_LogPrintf(szErrorMsg);
                        }
                        /* 關檔並回傳VS_NO_RECORD */
                        inFILE_Close(&ulHandlePtr);
                        return (VS_NO_RECORD);
                }
                /* Get不到Size的時候 */
                else if (lnFILE_GetSize(&ulHandlePtr, uszFileName) == VS_ERROR)
                {
                        /* 關檔並回傳VS_ERROR */
                        inFILE_Close(&ulHandlePtr);
                        return (VS_ERROR);
                }
                /* 檔案大小大於0(裡面有資料時) */
                else
                {
                        /* 讀檔 */
                        if (inFILE_Read(&ulHandlePtr, (unsigned char *) srAccumRec, _ACCUM_REC_SIZE_) == VS_ERROR)
                        {
                                /* 讀檔失敗就關檔 */
                                inFILE_Close(&ulHandlePtr);
                                /* 讀檔失敗，所以回傳（VS_ERROR）。(關檔不論成功與否都要回傳(VS_ERROR)) */
                                return (VS_ERROR);
                        }
                }

                /* 將檔案關閉 */
                if (inFILE_Close(&ulHandlePtr) == VS_ERROR)
                {
                        /*關檔失敗*/
                        /* 關檔失敗，所以回傳VS_ERROR */
                        return (VS_ERROR);
                }
        }
        /* inACCUM_GetRecord()_END */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inACCUM_GetRecord()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inACCUM_GetRecord_ESVC
Date&Time       :2018/1/18 下午 4:54
Describe        :將.amt中的資料讀取到srAccumRec來做使用，回傳(VS_SUCCESS) or (VS_ERROR)
*/
int inACCUM_GetRecord_ESVC(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec)
{
        unsigned long   ulHandlePtr;				/* File Handle */
        int             inRetVal;                               /* return value，來判斷是否回傳error */
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        unsigned char   uszFileName[14 + 1];                    /* 儲存交易金額檔案的檔案名稱(最大為15) */

        /* inACCUM_GetRecord()_START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inACCUM_GetRecord_ESVC()_START");
        }

	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _ACCUM_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

        /* 純開檔不建檔  */
        if (inFILE_OpenReadOnly(&ulHandlePtr, uszFileName) == (VS_ERROR))
        {
                /* 開檔失敗時 */
                /* 開檔錯誤，確認是否有檔案，若有檔案仍錯誤，則可能是handle的問題 */
                if (inFILE_Check_Exist(uszFileName) != (VS_ERROR))
                {
                        /* 開啟失敗，但檔案存在回傳error */
                        return (VS_ERROR);
                }
                /* 如果沒有檔案時，則為沒有交易紀錄，回傳NO_RECORD並印空簽單 */
                else
                {
                        return (VS_NO_RECORD);
                }

        }
        /* 開檔成功 */
        else
        {
                /*先清空srAccumRec 為讀檔案作準備  */
                memset(srAccumRec, 0x00, sizeof(srAccumRec));

                /* 把指針指到開頭*/
                inRetVal = inFILE_Seek(ulHandlePtr, 0, _SEEK_BEGIN_);

                /* inFILE_Seek，成功時 */
                if (inRetVal != VS_SUCCESS)
                {
                        /* inFILE_Seek失敗時 */
                        /* 關檔並回傳VS_ERROR */
                        inFILE_Close(&ulHandlePtr);

                        /* Seek檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
                        return (VS_ERROR);
                }

                /*確認檔案大小*/
                if (lnFILE_GetSize(&ulHandlePtr, uszFileName) == 0)
                {
                        /* 長度為0，不必讀 */
                        /* debug */
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("lnFILE_GetSize ＝＝ 0 ：");
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "(FName = %s), (Ptr = %d)", uszFileName, (int)ulHandlePtr);
                                inDISP_LogPrintf(szErrorMsg);
                        }
                        /* 關檔並回傳VS_NO_RECORD */
                        inFILE_Close(&ulHandlePtr);
                        return (VS_NO_RECORD);
                }
                /* Get不到Size的時候 */
                else if (lnFILE_GetSize(&ulHandlePtr, uszFileName) == VS_ERROR)
                {
                        /* 關檔並回傳VS_ERROR */
                        inFILE_Close(&ulHandlePtr);
                        return (VS_ERROR);
                }
                /* 檔案大小大於0(裡面有資料時) */
                else
                {
                        /* 讀檔 */
                        if (inFILE_Read(&ulHandlePtr, (unsigned char *) srAccumRec, _TICKET_ACCUM_REC_SIZE_) == VS_ERROR)
                        {
                                /* 讀檔失敗就關檔 */
                                inFILE_Close(&ulHandlePtr);
                                /* 讀檔失敗，所以回傳（VS_ERROR）。(關檔不論成功與否都要回傳(VS_ERROR)) */
                                return (VS_ERROR);
                        }
                }

                /* 將檔案關閉 */
                if (inFILE_Close(&ulHandlePtr) == VS_ERROR)
                {
                        /*關檔失敗*/
                        /* 關檔失敗，所以回傳VS_ERROR */
                        return (VS_ERROR);
                }
        }
        /* inACCUM_GetRecord()_END */
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

        return (VS_SUCCESS);
}

/*
Function        :inACCUM_UpdateFlow
Date&Time       :2015/9/4 下午 13:30
Describe        :做交易後計算總額的flow，回傳(VS_SUCCESS) or (VS_ERROR)
*/
int inACCUM_UpdateFlow(TRANSACTION_OBJECT *pobTran)
{
	unsigned long   ulHandlePtr;		/* File Handle，type為pointer */
	int		inRetVal;				/* return value，來判斷是否回傳error */
	unsigned char   uszFileName[14 + 1]; /* 儲存交易金額檔案的檔案名稱(最大為15) */
	ACCUM_TOTAL_REC srAccumRec; /*用來放總筆數、總金額的結構體*/
		
	/* inACCUM_UpdateFlow()_START */
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inACCUM_UpdateFlow INIT" );
	
		
	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _ACCUM_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Accum Update Compose *Error* FName[%s] Line[%d]", uszFileName, __LINE__);
		/* Update Accum失敗 鎖機*/
		inFunc_EDCLock();

		return (VS_ERROR);
	}

	/* 開檔 create檔(若沒有檔案則創建檔案) */
	if (inFILE_Open(&ulHandlePtr, uszFileName) == (VS_ERROR))
	{
		/* 開檔失敗時，不必關檔(開檔失敗，handle回傳NULL) */
		/* 若檔案不存在時，Create檔案 */
		if (inFILE_Check_Exist(uszFileName) == (VS_ERROR))
		{
			inRetVal = inFILE_Create(&ulHandlePtr, uszFileName);
		}
		/* 檔案存在還是開檔失敗，回傳錯誤跳出 */
		else
		{
			inDISP_DispLogAndWriteFlie("  Accum Update Amt  *Error* FName[%s] Line[%d]", uszFileName, __LINE__);
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock();
			return (VS_ERROR);
		}

		/* Create檔 */
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Accum Update Amt Create File *Error* FName[%s] Line[%d]", uszFileName, __LINE__);
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock();

			/* Create檔失敗時不關檔，因為create檔失敗handle回傳NULL */
			return (VS_ERROR);
		}
		/* create檔成功就繼續(因為create檔已經把handle指向檔案，所以不用在開檔) */
	}

	/* 開檔成功或create檔成功後 */
	/*先清空srAccumRec 為讀檔案作準備  */
	memset(&srAccumRec, 0x00, sizeof(srAccumRec));

	/* 把指針指到開頭*/
	inRetVal = inFILE_Seek(ulHandlePtr, 0, _SEEK_BEGIN_);

	/* inFile_seek */
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Accum Update Amt Seek File *Error* FName[%s] Line[%d]", uszFileName, __LINE__);
		/* Update Accum失敗 鎖機*/
		inFunc_EDCLock();

		/* inFILE_Seek失敗時 */
		/* Seek檔失敗，所以關檔 */
		inFILE_Close(&ulHandlePtr);

		/* Seek檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
		return (VS_ERROR);
	}

	/* 2015/11/20 修改 */
	/* 當文件內容為空時，read會回傳error;所以當read失敗， 而且檔案長度不等於0的時候，才是真的出錯 */
	if (inFILE_Read(&ulHandlePtr, (unsigned char *)&srAccumRec, _ACCUM_REC_SIZE_) == VS_ERROR && lnFILE_GetSize(&ulHandlePtr, (unsigned char *) &srAccumRec) != 0)
	{
		inDISP_DispLogAndWriteFlie("  Accum Update Amt Read File *Error* FName[%s] Line[%d]", uszFileName, __LINE__);
		/* Update Accum失敗 鎖機*/
		inFunc_EDCLock();

		/* 讀檔失敗 */
		/* Read檔失敗，所以關檔 */
		inFILE_Close(&ulHandlePtr);

		/* Read檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
		return (VS_ERROR);
	}
	else
	{
		if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_	||
			pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
		{
			/* 計算優惠兌換成功數量 */
			if (inACCUM_UpdateLoyaltyRedeem(pobTran, &srAccumRec) == VS_ERROR)
			{
				inDISP_DispLogAndWriteFlie("  Accum Update Amt Update Loyalty File *Error* InCode [%d] Line[%d]", pobTran->srBRec.inCode, __LINE__);
				/* Update Accum失敗 鎖機*/
				inFunc_EDCLock();

				/* 計算卡別全部交易金額、筆數失敗 */
				/* 計算卡別全部交易金額、筆數失敗，所以關檔 */
				inFILE_Close(&ulHandlePtr);

				/* 計算卡別全部交易金額、筆數失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
				return (VS_ERROR);
			}
		}
		else
		{
			/* 計算全部交易金額、筆數 */
			if (inACCUM_UpdateTotalAmount(pobTran, &srAccumRec) == VS_ERROR)
			{
				inDISP_DispLogAndWriteFlie("  Accum Update Amt Update Totle Amt *Error* Line[%d]",  __LINE__);
				/* Update Accum失敗 鎖機*/
				inFunc_EDCLock();

				/* 計算全部交易金額、筆數失敗 */
				/* 計算全部交易金額、筆數失敗，所以關檔 */
				inFILE_Close(&ulHandlePtr);

				/* 計算全部交易金額、筆數失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
				return (VS_ERROR);
			}

			/* 計算卡別全部交易金額、筆數 */
			if (inACCUM_UpdateTotalAmountByCard(pobTran, &srAccumRec) == VS_ERROR)
			{
				inDISP_DispLogAndWriteFlie("  Accum Update Amt Update By Card *Error* Line[%d]",  __LINE__);
				/* Update Accum失敗 鎖機*/
				inFunc_EDCLock();

				/* 計算卡別全部交易金額、筆數失敗 */
				/* 計算卡別全部交易金額、筆數失敗，所以關檔 */
				inFILE_Close(&ulHandlePtr);

				/* 計算卡別全部交易金額、筆數失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
				return (VS_ERROR);
			}
		}

		/* 先將檔案關閉 */
		if (inFILE_Close(&ulHandlePtr) == (VS_ERROR))
		{
			inDISP_DispLogAndWriteFlie("  Accum Update Amt Close File *Error* ulHandlePtr[%lu] Line[%d]", ulHandlePtr, __LINE__);
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock();

			/* 關檔失敗 */
			/* 回傳VS_ERROR */
			return (VS_ERROR);
		}

		/* 存檔案 */
		if (inACCUM_StoreRecord(&srAccumRec, uszFileName) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie("  Accum Update Amt Store Rec *Error* uszFileName[%s] Line[%d]", uszFileName, __LINE__);
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock();

			/* 存檔失敗 */
			/* 因為inACCUM_StoreRecord失敗，所以回傳VS_ERROR */
			return (VS_ERROR);
		}

	}/* read、計算總額 和 存檔 成功 */

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inACCUM_UpdateFlow END");

	/* inACCUM_UpdateFlow()_END */
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		
	return (VS_SUCCESS);
}

/*
Function        :inACCUM_ReviewReport_Total
Date&Time       :2016/2/25 下午 4:06
Describe        :螢幕顯示總額
*/
int inACCUM_ReviewReport_Total(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szHostLabel[8 + 1];
	int (*inDisplayRevew)(TRANSACTION_OBJECT *pobTran);
	int (*inDisplayEscRevew)(TRANSACTION_OBJECT *pobTran);

	inDisplayRevew = NULL;
	inDisplayEscRevew = NULL;
	
	memset(szHostLabel, 0x00, sizeof(szHostLabel));
	inGetHostLabel(szHostLabel);
	
	/*  票證顯示格式不同 票證部份先保留條件，先用信用卡的代替 20190502 [SAM]  */
	if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
	{	
		/* [新增電票悠遊卡功能] 新增電票顯示條件 [SAM] 2022/6/22 下午 2:38*/
		if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
		{
			inDisplayRevew = inCMASSrc_Display_Review;
		}else{
			inDisplayRevew = inFunc_Display_Review;
		}
			
		inDISP_LogPrintfWithFlag("  ESVCT Function Point [%x]", inDisplayRevew);
		
	}else	if (!memcmp(szHostLabel, _HOST_NAME_CREDIT_MAINLY_USE_, strlen(_HOST_NAME_CREDIT_MAINLY_USE_)))
	{
		inDisplayRevew = inFunc_Display_Review;
		inDisplayEscRevew = inFunc_Display_Review_ESC_Reinforce;
		
		inDISP_LogPrintfWithFlag("  ESVCT Normal FP [%x] ESC FP[%x]", inDisplayRevew, inDisplayEscRevew);
		inDISP_LogPrintfWithFlag("  ESVCT Label [%s]", szHostLabel);
	}
	
	/* 顯示的流程 */
	if( inDisplayRevew != NULL )
	{		
		inRetVal = inDisplayRevew(pobTran);
		
		if (inRetVal == VS_SUCCESS)
		{
			if( inDisplayEscRevew != NULL )
			{
				inRetVal = inDisplayEscRevew(pobTran);
			}
		}
	}
	
	

	return (inRetVal);

}

/*
Function        :inACCUM_ReviewReport_Total_Settle
Date&Time       :2016/10/12 上午 11:45
Describe        :交易確認畫面，for 結帳用
*/
int inACCUM_ReviewReport_Total_Settle(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_BATCH_END_;
	char	szHostLabel[8 + 1];
	unsigned char	uszKey;

	/* 連動結帳的話 */
	if (pobTran->uszAutoSettleBit == VS_TRUE)
	{
		/* 如果是ECR發動不用再按0確認 */
		if (pobTran->uszECRBit == VS_TRUE)
		{
			inRetVal = VS_SUCCESS;
		}
		else
		{
			inDISP_Clear_Line(_LINE_8_6_, _LINE_8_8_);
			inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);

			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

			uszKey = 0x00;
			while (1)
			{
				inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				uszKey = uszKBD_Key();

				/* Timeout */
				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					uszKey = _KEY_TIMEOUT_;
				}

				if (uszKey == _KEY_CANCEL_)
				{
					inRetVal = VS_USER_CANCEL;
					break;
				}
				else if (uszKey == _KEY_TIMEOUT_)
				{
					inRetVal = VS_TIMEOUT;
					break;
				}
				else if (inChoice == _BATCH_END_Touch_ENTER_BUTTON_	||
					 uszKey == _KEY_0_)
				{
					inRetVal = VS_SUCCESS;
					break;
				}
			}
			/* 清空Touch資料 */
			inDisTouch_Flush_TouchFile();
		}
	}
	else
	{
		
		/* 票證顯示格式不同 */
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
//			inRetVal = inNCCC_Ticket_Func_Display_Review_Settle(pobTran);
				
			/* [新增電票悠遊卡功能] 新增電票結帳功能條件 [SAM] 2022/6/23 下午 5:53 */
			memset(szHostLabel, 0x00, sizeof(szHostLabel));
			inGetHostLabel(szHostLabel);
			/* IPASS 的先留著 */
//			if (!memcmp(szHostLabel, _HOST_NAME_IPASS_, strlen(_HOST_NAME_IPASS_)))
//			{
//				inRetVal = inIPASS2Src_Display_Review(pobTran);
//			}
//			else 
			if (!memcmp(szHostLabel, _HOST_NAME_CMAS_, strlen(_HOST_NAME_CMAS_)))
			{
				inRetVal = inCMASSrc_Display_Review(pobTran);
			} 
		}
		else
		{
			memset(szHostLabel, 0x00, sizeof(szHostLabel));
			inGetHostLabel(szHostLabel);
			if (!memcmp(szHostLabel, _HOST_NAME_CREDIT_MAINLY_USE_, strlen(_HOST_NAME_CREDIT_MAINLY_USE_)))
			{
				inRetVal = inFunc_Display_Review_Settle(pobTran);
			}
#ifdef __NCCC_DCC_FUNC__			
			else if (!memcmp(szHostLabel, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_)))
			{
//				inRetVal = inNCCC_Func_Display_Review_Settle_DCC(pobTran);
			}
#endif
			/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
		}
	}

	return (inRetVal);
}

/*
Function        :inACCUM_UpdateFlow_ESC
Date&Time       :2018/4/17 下午 3:34
Describe        :
*/
int inACCUM_UpdateFlow_ESC(TRANSACTION_OBJECT *pobTran, int inUpdateType)
{
	unsigned long   ulHandlePtr;			/* File Handle，type為pointer */
	int	inRetVal;			/* return value，來判斷是否回傳error */
	unsigned char   uszFileName[14 + 1] = {0};	/* 儲存交易金額檔案的檔案名稱(最大為15) */
	ACCUM_TOTAL_REC srAccumRec;			/*用來放總筆數、總金額的結構體*/

	/* inACCUM_UpdateFlow()_START */
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	memset(uszFileName, 0x00, sizeof(uszFileName));
	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _ACCUM_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Accum ESC Update Amt Compose *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	/* 開檔 create檔(若沒有檔案則創建檔案) */
	if (inFILE_Open(&ulHandlePtr, uszFileName) == (VS_ERROR))
	{
		/* 開檔失敗時，不必關檔(開檔失敗，handle回傳NULL) */
		/* 若檔案不存在時，Create檔案 */
		if (inFILE_Check_Exist(uszFileName) == (VS_ERROR))
		{
			inRetVal = inFILE_Create(&ulHandlePtr, uszFileName);
		}
		/* 檔案存在還是開檔失敗，回傳錯誤跳出 */
		else
		{
			inDISP_DispLogAndWriteFlie("  Accum ESC Update Amt Open File *Error* F_Name[%s] Line[%d]", uszFileName, __LINE__);
			return (VS_ERROR);
		}

		/* Create檔 */
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Accum ESC Update Amt Create File *Error* F_Name[%s] Line[%d]", uszFileName, __LINE__);
			/* Create檔失敗時不關檔，因為create檔失敗handle回傳NULL */
			return (VS_ERROR);
		}
		/* create檔成功就繼續(因為create檔已經把handle指向檔案，所以不用在開檔) */
	}

	/* 開檔成功或create檔成功後 */
	/*先清空srAccumRec 為讀檔案作準備  */
	memset(&srAccumRec, 0x00, sizeof(srAccumRec));

	/* 把指針指到開頭*/
	inRetVal = inFILE_Seek(ulHandlePtr, 0, _SEEK_BEGIN_);

	/* inFile_seek */
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Accum ESC Update Amt Seek File *Error* F_Name[%s] HDL[%d] Line[%d]", uszFileName, ulHandlePtr,  __LINE__);
		/* inFILE_Seek失敗時 */
		/* Seek檔失敗，所以關檔 */
		inFILE_Close(&ulHandlePtr);

		/* Seek檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
		return (VS_ERROR);
	}


	/* 當文件內容為空時，read會回傳error;所以當read失敗， 而且檔案長度不等於0的時候，才是真的出錯 */
	if (inFILE_Read(&ulHandlePtr, (unsigned char *)&srAccumRec, _ACCUM_REC_SIZE_) == VS_ERROR && lnFILE_GetSize(&ulHandlePtr, (unsigned char *) &srAccumRec) != 0)
	{
		inDISP_DispLogAndWriteFlie("  Accum ESC Update Amt Read File Size *Error* ");

		/* 讀檔失敗 */
		/* Read檔失敗，所以關檔 */
		inFILE_Close(&ulHandlePtr);

		/* Read檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
		return (VS_ERROR);
	}
	else
	{
		/* 計算ESC全部交易金額、筆數 */
		if (inACCUM_Update_ESC_TotalAmount(pobTran, &srAccumRec, inUpdateType) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie("  Accum ESC Run Update Amt *Error* Line[%d]", __LINE__);
			inDISP_DispLogAndWriteFlie("  inCode [%d]　Update Type[%d] Line[%d] ", pobTran->srBRec.inCode, inUpdateType, __LINE__ );			
			
			
			/* 計算ESC全部交易金額、筆數失敗 */
			/* 計算ESC全部交易金額、筆數失敗，所以關檔 */
			inFILE_Close(&ulHandlePtr);

			/* 計算全部交易金額、筆數失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
			return (VS_ERROR);
		}

		/* 先將檔案關閉 */
		if (inFILE_Close(&ulHandlePtr) == (VS_ERROR))
		{
			inDISP_DispLogAndWriteFlie("  Accum ESC Update Amt Close File *Error* ulHandlePtr[%d] Line[%d]", ulHandlePtr,  __LINE__);
			/* 關檔失敗 */
			/* 回傳VS_ERROR */
			return (VS_ERROR);
		}

		/* 存檔案 */
		if (inACCUM_StoreRecord(&srAccumRec, uszFileName) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie("  Accum ESC Update Amt Store Rec *Error* uszFileName[%s] Line[%d]", uszFileName,  __LINE__);
			/* 存檔失敗 */
			/* 因為inACCUM_StoreRecord失敗，所以回傳VS_ERROR */
			return (VS_ERROR);
		}

	}/* read、計算總額 和 存檔 成功 */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/* 不是　_ESC_ACCUM_STATUS_FAIL_ 也不是　 _ESC_ACCUM_STATUS_BYPASS_　就代表是成功的交易*/
int inACCUM_Update_ESC_TotalAmount(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, int inUpdateType)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inACCUM_Update_ESC_TotalAmount()_START");
		inDISP_LogPrintf("   inCode [%d]　Update Type[%d]", pobTran->srBRec.inCode, inUpdateType );
	}
	
	if ( inUpdateType == _ESC_ACCUM_STATUS_REPRINT_ )
	{
		srAccumRec->lnESC_RePrintNum ++;
//		srAccumRec->lnESC_TotalFailULNum ++;
		
	}else	if (inUpdateType == _ESC_ACCUM_STATUS_BYPASS_ )	/* ESC_BYPASS */
	{
		/* 計算筆數 */
		srAccumRec->lnESC_BypassNum ++;
		srAccumRec->lnESC_TotalFailULNum ++;
		
		if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
		{
			switch (pobTran->srBRec.inCode)
			{
				case _SALE_ :
				case _CUP_SALE_ :
				case _INST_SALE_ :
				case _REDEEM_SALE_ :
				case _SALE_OFFLINE_ :
				case _INST_ADJUST_ :
				case _REDEEM_ADJUST_ :
				case _PRE_COMP_ :
				case _CUP_PRE_COMP_ :
				case _MAIL_ORDER_ :
				case _CUP_MAIL_ORDER_ :
				case _CASH_ADVANCE_ :
				case _FORCE_CASH_ADVANCE_ :
				case _FISC_SALE_ :
					/* 計算金額 */
					srAccumRec->llESC_BypassAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llESC_TotalFailULAmount += pobTran->srBRec.lnTxnAmount;
					break;
				case _REFUND_ :
				case _REDEEM_REFUND_ :
				case _INST_REFUND_ :
				case _CUP_REFUND_ :
				case _CUP_MAIL_ORDER_REFUND_ :
				case _FISC_REFUND_ :
					/* 計算金額 */
					srAccumRec->llESC_BypassAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llESC_TotalFailULAmount -= pobTran->srBRec.lnTxnAmount;
					break;
				case _TIP_ :
					/* 計算金額 */
					srAccumRec->llESC_BypassAmount += pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llESC_TotalFailULAmount += pobTran->srBRec.lnTipTxnAmount;
					break;
				case _PRE_AUTH_ :
				case _CUP_PRE_AUTH_ :
					srAccumRec->lnESC_BypassNum --;
					srAccumRec->lnESC_TotalFailULNum --;

					/* ESC預先授權列印紙本及預先授權不納入結帳總額 */
					srAccumRec->lnESC_PreAuthNum ++;
					srAccumRec->llESC_PreAuthAmount += pobTran->srBRec.lnTxnAmount;
					break;
						default :
							return (VS_ERROR);
			}

		}
		else
		{
			switch (pobTran->srBRec.inOrgCode)
			{
				case _SALE_ :
				case _CUP_SALE_ :
				case _INST_SALE_ :
				case _REDEEM_SALE_ :
				case _SALE_OFFLINE_ :
				case _INST_ADJUST_ :
				case _REDEEM_ADJUST_ :
				case _PRE_COMP_ :
				case _CUP_PRE_COMP_ :
				case _MAIL_ORDER_ :
				case _CUP_MAIL_ORDER_ :
				case _CASH_ADVANCE_ :
				case _FORCE_CASH_ADVANCE_ :
				case _FISC_SALE_ :
					/* 計算金額 */
					srAccumRec->llESC_BypassAmount += (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llESC_TotalFailULAmount += (0 - pobTran->srBRec.lnTxnAmount);
					break;
				case _REFUND_ :
				case _REDEEM_REFUND_ :
				case _INST_REFUND_ :
				case _CUP_REFUND_ :
				case _CUP_MAIL_ORDER_REFUND_ :
				case _FISC_REFUND_ :
					/* 計算金額 */
					srAccumRec->llESC_BypassAmount -= (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llESC_TotalFailULAmount -= (0 - pobTran->srBRec.lnTxnAmount);
					break;
				case _PRE_AUTH_ :
				case _CUP_PRE_AUTH_ :
					srAccumRec->lnESC_BypassNum --;
					srAccumRec->lnESC_TotalFailULNum --;

					/* ESC預先授權列印紙本及預先授權不納入結帳總額 */
					srAccumRec->lnESC_PreAuthNum ++;
					srAccumRec->llESC_PreAuthAmount += pobTran->srBRec.lnTxnAmount;
					break;
				default :
					return (VS_ERROR);
			}
		}
	}
	else if (inUpdateType == _ESC_ACCUM_STATUS_FAIL_)		/* ESC_FAIL */
	{
		/* 計算筆數 */
		srAccumRec->lnESC_FailUploadNum ++;
		srAccumRec->lnESC_TotalFailULNum ++;

		if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
		{
			switch (pobTran->srBRec.inCode)
			{
				case _SALE_ :
				case _CUP_SALE_ :
				case _INST_SALE_ :
				case _REDEEM_SALE_ :
				case _SALE_OFFLINE_ :
				case _INST_ADJUST_ :
				case _REDEEM_ADJUST_ :
				case _PRE_COMP_ :
				case _CUP_PRE_COMP_ :
				case _MAIL_ORDER_ :
				case _CUP_MAIL_ORDER_ :
				case _CASH_ADVANCE_ :
				case _FORCE_CASH_ADVANCE_ :
				case _FISC_SALE_ :
					/* 計算金額 */
					srAccumRec->llESC_FailUploadAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llESC_TotalFailULAmount += pobTran->srBRec.lnTxnAmount;
					break;
				case _REFUND_ :
				case _REDEEM_REFUND_ :
				case _INST_REFUND_ :
				case _CUP_REFUND_ :
				case _CUP_MAIL_ORDER_REFUND_ :
				case _FISC_REFUND_ :
					/* 計算金額 */
					srAccumRec->llESC_FailUploadAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llESC_TotalFailULAmount -= pobTran->srBRec.lnTxnAmount;
					break;
				case _TIP_ :
					/* 計算金額 */
					srAccumRec->llESC_FailUploadAmount += pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llESC_TotalFailULAmount += pobTran->srBRec.lnTipTxnAmount;
					break;
				case _PRE_AUTH_ :
				case _CUP_PRE_AUTH_ :
					srAccumRec->lnESC_FailUploadNum --;
					srAccumRec->lnESC_TotalFailULNum --;

					/* ESC預先授權列印紙本及預先授權不納入結帳總額 */
					srAccumRec->lnESC_PreAuthNum ++;
					srAccumRec->llESC_PreAuthAmount += pobTran->srBRec.lnTxnAmount;
					break;
				default :
					return (VS_ERROR);
			}
		}
		else
		{
			switch (pobTran->srBRec.inOrgCode)
			{
				case _SALE_ :
				case _CUP_SALE_ :
				case _INST_SALE_ :
				case _REDEEM_SALE_ :
				case _SALE_OFFLINE_ :
				case _INST_ADJUST_ :
				case _REDEEM_ADJUST_ :
				case _PRE_COMP_ :
				case _CUP_PRE_COMP_ :
				case _MAIL_ORDER_ :
				case _CUP_MAIL_ORDER_ :
				case _CASH_ADVANCE_ :
				case _FORCE_CASH_ADVANCE_ :
				case _FISC_SALE_ :
					/* 計算金額 */
					srAccumRec->llESC_FailUploadAmount += (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llESC_TotalFailULAmount += (0 - pobTran->srBRec.lnTxnAmount);
					break;
				case _REFUND_ :
				case _REDEEM_REFUND_ :
				case _INST_REFUND_ :
				case _CUP_REFUND_ :
				case _CUP_MAIL_ORDER_REFUND_ :
				case _FISC_REFUND_ :
					/* 計算金額 */
					srAccumRec->llESC_FailUploadAmount -= (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llESC_TotalFailULAmount -= (0 - pobTran->srBRec.lnTxnAmount);
					break;
				case _PRE_AUTH_ :
				case _CUP_PRE_AUTH_ :
					srAccumRec->lnESC_FailUploadNum --;
					srAccumRec->lnESC_TotalFailULNum --;

					/* ESC預先授權列印紙本及預先授權不納入結帳總額 */
					srAccumRec->lnESC_PreAuthNum ++;
					srAccumRec->llESC_PreAuthAmount += pobTran->srBRec.lnTxnAmount;
					break;
				default :
					return (VS_ERROR);
			}
		}
	}
        else		/* ESC_SUCCESS */
	{
		/* 計算筆數 */
		srAccumRec->lnESC_SuccessNum ++;

		if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
		{
			switch (pobTran->srBRec.inCode)
			{
				case _SALE_ :
				case _CUP_SALE_ :
				case _INST_SALE_ :
				case _REDEEM_SALE_ :
				case _SALE_OFFLINE_ :
				case _INST_ADJUST_ :
				case _REDEEM_ADJUST_ :
				case _PRE_COMP_ :
				case _CUP_PRE_COMP_ :
				case _MAIL_ORDER_ :
				case _CUP_MAIL_ORDER_ :
				case _CASH_ADVANCE_ :
				case _FORCE_CASH_ADVANCE_ :
				case _FISC_SALE_ :
					/* 計算金額 */
					srAccumRec->llESC_SuccessAmount += pobTran->srBRec.lnTxnAmount;
					break;
				case _REFUND_ :
				case _REDEEM_REFUND_ :
				case _INST_REFUND_ :
				case _CUP_REFUND_ :
				case _CUP_MAIL_ORDER_REFUND_ :
				case _FISC_REFUND_ :
					/* 計算金額 */
					srAccumRec->llESC_SuccessAmount -= pobTran->srBRec.lnTxnAmount;
					break;
				case _TIP_ :
					/* 計算金額 */
					srAccumRec->llESC_SuccessAmount += pobTran->srBRec.lnTipTxnAmount;
					break;
				case _PRE_AUTH_ :
				case _CUP_PRE_AUTH_ :
					srAccumRec->lnESC_SuccessNum --;

					/* ESC預先授權列印紙本及預先授權不納入結帳總額 */
					srAccumRec->lnESC_PreAuthNum ++;
					srAccumRec->llESC_PreAuthAmount += pobTran->srBRec.lnTxnAmount;
					break;
				default :
					return (VS_ERROR);
			}
		}
		else
		{
			switch (pobTran->srBRec.inOrgCode)
			{
				case _SALE_ :
				case _CUP_SALE_ :
				case _INST_SALE_ :
				case _REDEEM_SALE_ :
				case _SALE_OFFLINE_ :
				case _INST_ADJUST_ :
				case _REDEEM_ADJUST_ :
				case _PRE_COMP_ :
				case _CUP_PRE_COMP_ :
				case _MAIL_ORDER_ :
				case _CUP_MAIL_ORDER_ :
				case _CASH_ADVANCE_ :
				case _FORCE_CASH_ADVANCE_ :
				case _FISC_SALE_ :
					/* 計算金額 */
					srAccumRec->llESC_SuccessAmount += (0 - pobTran->srBRec.lnTxnAmount);
					break;
				case _REFUND_ :
				case _REDEEM_REFUND_ :
				case _INST_REFUND_ :
				case _CUP_REFUND_ :
				case _CUP_MAIL_ORDER_REFUND_ :
				case _FISC_REFUND_ :
					/* 計算金額 */
					srAccumRec->llESC_SuccessAmount -= (0 - pobTran->srBRec.lnTxnAmount);
					break;
				case _PRE_AUTH_ :
				case _CUP_PRE_AUTH_ :
					srAccumRec->lnESC_SuccessNum --;

					/* ESC預先授權列印紙本及預先授權不納入結帳總額 */
					srAccumRec->lnESC_PreAuthNum ++;
					srAccumRec->llESC_PreAuthAmount += pobTran->srBRec.lnTxnAmount;
					break;
				default :
					return (VS_ERROR);
			}

		}
	}

	/* inACCUM_UpdateFlow()_END */
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inACCUM_Update_ESC_TotalAmount()_END");
	}
	return (VS_SUCCESS);
}

/*
Function        :inACCUM_Check_Specific_Accum
Date&Time       :2017/1/10 下午 3:42
Describe        :原來用來確認該Host有沒有做過交易，但後來想到更好的辦法
*/
int inACCUM_Check_Specific_Accum(TRANSACTION_OBJECT *pobTran, char *szHostName, ACCUM_TOTAL_REC* srACCUMRec)
{
	int	inHostIndex = -1;

	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, szHostName, &inHostIndex);

	if (inHostIndex == -1)
	{
		return (VS_ERROR);
	}

	if (inLoadHDPTRec(inHostIndex) != VS_SUCCESS)
	{
		/* load回來 */
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

		return (VS_ERROR);
	}

	if (inACCUM_GetRecord(pobTran, srACCUMRec) != VS_SUCCESS)
	{
		/* load回來 */
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

		return (VS_ERROR);
	}

	/* load回來 */
	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

	return (VS_SUCCESS);
}

/*
Function        :inACCUM_Check_Transaction_Count
Date&Time       :2017/1/10 下午 4:13
Describe        :確認是否有交易紀錄，若沒做交易，連Accum檔都不會產生
*/
int inACCUM_Check_Transaction_Count(TRANSACTION_OBJECT *pobTran, char *szHostName, char *szTrans)
{
	int	inHostIndex = -1;
	char	szFileName[15 + 1];

	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, szHostName, &inHostIndex);

	if (inHostIndex == -1)
	{
		return (VS_ERROR);
	}

	if (inLoadHDPTRec(inHostIndex) != VS_SUCCESS)
	{
		/* load回來 */
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

		return (VS_ERROR);
	}

	memset(szFileName, 0x00, sizeof(szFileName));
	if (inFunc_ComposeFileName(pobTran, szFileName, _ACCUM_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		/* load回來 */
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

		return (VS_ERROR);
	}

	if (inFILE_Check_Exist((unsigned char*)szFileName) != VS_SUCCESS)
	{
		*szTrans = 'N';
	}
	else
	{
		*szTrans = 'Y';
	}

	/* load回來 */
	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

	return (VS_SUCCESS);
}

/*
Function        :inACCUM_UpdateLoyaltyRedeem
Date&Time       :2017/2/21 下午 1:41
Describe        :計算優惠兌換數量
*/
int inACCUM_UpdateLoyaltyRedeem(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec)
{
	/* 兌換成功或兌換取消 */
	if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_)
	{
		srAccumRec->lnLoyaltyRedeemSuccessCount ++;
		srAccumRec->lnLoyaltyRedeemTotalCount ++;
	}
	else if (pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
	{
		srAccumRec->lnLoyaltyRedeemCancelCount ++;
		srAccumRec->lnLoyaltyRedeemTotalCount --;
	}

	return (VS_SUCCESS);
}

/*
Function        :inACCUM_Update_Ticket_Flow
Date&Time       :2018/1/12 下午 2:27
Describe        :紀錄電票交易的帳
*/
int inACCUM_Update_Ticket_Flow(TRANSACTION_OBJECT *pobTran)
{
        unsigned long		ulHandlePtr = 0;	/* File Handle，type為pointer */
        int			inRetVal;		/* return value，來判斷是否回傳error */
        unsigned char		uszFileName[14 + 1];	/* 儲存交易金額檔案的檔案名稱(最大為15) */
        TICKET_ACCUM_TOTAL_REC	srAccumRec;		/*用來放總筆數、總金額的結構體*/

        /* inACCUM_UpdateFlow()_START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inACCUM_Update_Ticket_Flow()_START");
        }

	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _ACCUM_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		/* Update Accum失敗 鎖機*/
		inFunc_EDCLock();

		return (VS_ERROR);
	}

        /* 開檔 create檔(若沒有檔案則創建檔案) */
        if (inFILE_Open(&ulHandlePtr, uszFileName) == (VS_ERROR))
        {
                /* 開檔失敗時，不必關檔(開檔失敗，handle回傳NULL) */
                /* 若檔案不存在時，Create檔案 */
                if (inFILE_Check_Exist(uszFileName) == (VS_ERROR))
                {
                        inRetVal = inFILE_Create(&ulHandlePtr, uszFileName);
                }
                /* 檔案存在還是開檔失敗，回傳錯誤跳出 */
                else
                {
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock();

                        return (VS_ERROR);
                }

                /* Create檔 */
                if (inRetVal != VS_SUCCESS)
                {
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock();

                        /* Create檔失敗時不關檔，因為create檔失敗handle回傳NULL */
                        return (VS_ERROR);
                }
                /* create檔成功就繼續(因為create檔已經把handle指向檔案，所以不用在開檔) */
        }

        /* 開檔成功或create檔成功後 */
        /*先清空srAccumRec 為讀檔案作準備  */
        memset(&srAccumRec, 0x00, sizeof(srAccumRec));

        /* 把指針指到開頭*/
        inRetVal = inFILE_Seek(ulHandlePtr, 0, _SEEK_BEGIN_);

        /* inFile_seek */
        if (inRetVal != VS_SUCCESS)
        {
		/* Update Accum失敗 鎖機*/
		inFunc_EDCLock();

                /* inFILE_Seek失敗時 */
                /* Seek檔失敗，所以關檔 */
                inFILE_Close(&ulHandlePtr);

                /* Seek檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }

        /* 2015/11/20 修改 */
        /* 當文件內容為空時，read會回傳error;所以當read失敗， 而且檔案長度不等於0的時候，才是真的出錯 */
        if (inFILE_Read(&ulHandlePtr, (unsigned char *)&srAccumRec, _TICKET_ACCUM_REC_SIZE_) == VS_ERROR && lnFILE_GetSize(&ulHandlePtr, (unsigned char *) &srAccumRec) != 0)
        {
		/* Update Accum失敗 鎖機*/
		inFunc_EDCLock();

                /* 讀檔失敗 */
                /* Read檔失敗，所以關檔 */
                inFILE_Close(&ulHandlePtr);

                /* Read檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }
        else
        {
		/* 計算全部交易金額、筆數 */
		if (inACCUM_UpdateTotalAmount_Ticket(pobTran, &srAccumRec) == VS_ERROR)
		{
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock();

			/* 計算全部交易金額、筆數失敗 */
			/* 計算全部交易金額、筆數失敗，所以關檔 */
			inFILE_Close(&ulHandlePtr);

			/* 計算全部交易金額、筆數失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
			return (VS_ERROR);
		}


                /* 先將檔案關閉 */
                if (inFILE_Close(&ulHandlePtr) == (VS_ERROR))
                {
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock();

                        /* 關檔失敗 */
                        /* 回傳VS_ERROR */
                        return (VS_ERROR);
                }

                /* 存檔案 */
                if (inACCUM_StoreRecord_Ticket(&srAccumRec, uszFileName) == VS_ERROR)
                {
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock();

                        /* 存檔失敗 */
                        /* 因為inACCUM_StoreRecord失敗，所以回傳VS_ERROR */
                        return (VS_ERROR);
                }

        }/* read、計算總額 和 存檔 成功 */

        /* inACCUM_UpdateFlow()_END */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inACCUM_UpdateFlow()_END");
        }
        return (VS_SUCCESS);
}

/*
Function        :inACCUM_UpdateTotalAmount_Ticket
Date&Time       :2018/1/12 下午 3:28
Describe        :更新電票交易金額
*/
int inACCUM_UpdateTotalAmount_Ticket(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec)
{
	char	szDebugMsg[100 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inACCUM_UpdateTotalAmount_Ticket(%d) START !", pobTran->srBRec.inCode);
		inDISP_LogPrintf(szDebugMsg);
	}

        switch (pobTran->srTRec.inCode)
        {
                case _TICKET_IPASS_DEDUCT_ :
                        /* 計算筆數 */
                	srAccumRec->lnTotalCount ++;
                	srAccumRec->lnIPASS_TotalCount ++;
                	srAccumRec->lnDeductTotalCount ++;
                	srAccumRec->lnIPASS_DeductTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llIPASS_TotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llDeductTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llIPASS_DeductTotalAmount += pobTran->srTRec.lnTxnAmount;
                        break;
                case _TICKET_IPASS_VOID_TOP_UP_ :
                        /* 計算筆數 */
                	srAccumRec->lnTotalCount ++;
                	srAccumRec->lnIPASS_TotalCount ++;
                	srAccumRec->lnVoidADDTotalCount ++;
                	srAccumRec->lnIPASS_VoidADDTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llIPASS_TotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llVoidADDTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llIPASS_VoidADDTotalAmount += pobTran->srTRec.lnTxnAmount;
                        break;
                case _TICKET_IPASS_REFUND_ :
                        /* 計算筆數 */
                	srAccumRec->lnTotalCount ++;
                	srAccumRec->lnIPASS_TotalCount ++;
                	srAccumRec->lnRefundTotalCount ++;
                	srAccumRec->lnIPASS_RefundTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llIPASS_TotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llRefundTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llIPASS_RefundTotalAmount += pobTran->srTRec.lnTxnAmount;
                        break;
                case _TICKET_IPASS_TOP_UP_ :
                        /* 計算筆數 */
                	srAccumRec->lnTotalCount ++;
                	srAccumRec->lnIPASS_TotalCount ++;
                	srAccumRec->lnADDTotalCount ++;
                	srAccumRec->lnIPASS_ADDTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llIPASS_TotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llADDTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llIPASS_ADDTotalAmount += pobTran->srTRec.lnTxnAmount;
                        break;
                case _TICKET_IPASS_AUTO_TOP_UP_ :
		/* 計算筆數 */
		srAccumRec->lnTotalCount ++;
                	srAccumRec->lnIPASS_TotalCount ++;
                	srAccumRec->lnAutoADDTotalCount ++;
                	srAccumRec->lnIPASS_AutoADDTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                	srAccumRec->llIPASS_TotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                	srAccumRec->llAutoADDTotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                	srAccumRec->llIPASS_AutoADDTotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                        break;
                case _TICKET_EASYCARD_DEDUCT_ :
                        /* 計算筆數 */
                	srAccumRec->lnTotalCount ++;
                	srAccumRec->lnEASYCARD_TotalCount ++;
                	srAccumRec->lnDeductTotalCount ++;
                	srAccumRec->lnEASYCARD_DeductTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llEASYCARD_TotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llDeductTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llEASYCARD_DeductTotalAmount += pobTran->srTRec.lnTxnAmount;
                        break;
                case _TICKET_EASYCARD_VOID_TOP_UP_ :
                        /* 計算筆數 */
                	srAccumRec->lnTotalCount ++;
                	srAccumRec->lnEASYCARD_TotalCount ++;
                	srAccumRec->lnVoidADDTotalCount ++;
                	srAccumRec->lnEASYCARD_VoidADDTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llEASYCARD_TotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llVoidADDTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llEASYCARD_VoidADDTotalAmount += pobTran->srTRec.lnTxnAmount;
                        break;
                case _TICKET_EASYCARD_REFUND_ :
                        /* 計算筆數 */
                	srAccumRec->lnTotalCount ++;
                	srAccumRec->lnEASYCARD_TotalCount ++;
                	srAccumRec->lnRefundTotalCount ++;
                	srAccumRec->lnEASYCARD_RefundTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llEASYCARD_TotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llRefundTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llEASYCARD_RefundTotalAmount += pobTran->srTRec.lnTxnAmount;
                        break;
                case _TICKET_EASYCARD_TOP_UP_ :
                        /* 計算筆數 */
                	srAccumRec->lnTotalCount ++;
                	srAccumRec->lnEASYCARD_TotalCount ++;
                	srAccumRec->lnADDTotalCount ++;
                	srAccumRec->lnEASYCARD_ADDTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llEASYCARD_TotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llADDTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llEASYCARD_ADDTotalAmount += pobTran->srTRec.lnTxnAmount;
                        break;
                case _TICKET_EASYCARD_AUTO_TOP_UP_ :
		/* 計算筆數 */
		srAccumRec->lnTotalCount ++;
                	srAccumRec->lnEASYCARD_TotalCount ++;
                	srAccumRec->lnAutoADDTotalCount ++;
                	srAccumRec->lnEASYCARD_AutoADDTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                	srAccumRec->llEASYCARD_TotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                	srAccumRec->llAutoADDTotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                	srAccumRec->llEASYCARD_AutoADDTotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                        break;
                default :
                        return (VS_ERROR);
        }

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inACCUM_UpdateTotalAmount_Ticket() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inACCUM_StoreRecord_Ticket
Date&Time       :2018/1/12 下午 3:42
Describe        :再做加總後，將資料存回record，回傳(VS_SUCCESS) or (VS_ERROR)
*/
int inACCUM_StoreRecord_Ticket(TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszFileName)
{
        unsigned long   ulHandlePtr;    /* File Handle，type為pointer */
        int		inRetVal; /* return value，來判斷是否回傳error */

        /* inACCUM_StoreRecord()_START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inACCUM_StoreRecord()_START");
        }

        /*開檔*/
        inRetVal = inFILE_Open(&ulHandlePtr, uszFileName);
        /*開檔成功*/
        if (inRetVal != VS_SUCCESS)
        {
                /* 開檔失敗時，不必關檔(開檔失敗，handle回傳NULL) */
                /* 開檔失敗，所以回傳error */
                return(VS_ERROR);
        }

        /* 把指針指到開頭*/
        inRetVal = inFILE_Seek(ulHandlePtr, 0, _SEEK_BEGIN_);

        /* seek不成功時 */
        if (inRetVal != VS_SUCCESS)
        {
                /* inFILE_Seek失敗時 */
                /* 關檔並回傳VS_ERROR */
                inFILE_Close(&ulHandlePtr);
                /* seek失敗，所以回傳error。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }

        /* 寫檔 */
        if (inFILE_Write(&ulHandlePtr,(unsigned char*)srAccumRec, _TICKET_ACCUM_REC_SIZE_) != VS_SUCCESS)
        {
                /*寫檔失敗時*/
                /* 關檔  */
                inFILE_Close(&ulHandlePtr);
                /* 寫檔失敗，所以回傳error。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }

        /* 關檔  */
        if (inFILE_Close(&ulHandlePtr) != VS_SUCCESS)
        {
                /*關檔失敗時*/
                /* 關檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /* inACCUM_StoreRecord()_END */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inACCUM_StoreRecord()_END");
        }

        return (VS_SUCCESS);
}



/*
Function        : inACCUM_Update_CMAS_Flow
Date&Time   : 2022/6/8 下午 3:58
Describe        : 紀錄電票交易的帳
 *  [新增電票悠遊卡功能]  參考合庫新增功能 [SAM] 2022/6/8 下午 4:03 
*/
int inACCUM_Update_CMAS_Flow(TRANSACTION_OBJECT *pobTran)
{
	unsigned long ulHandlePtr = 0;	 /* File Handle，type為pointer */
	int inRetVal;					   /* return value，來判斷是否回傳error */
	unsigned char uszFileName[50 + 1]; /* 儲存交易金額檔案的檔案名稱(最大為15) */
	TICKET_ACCUM_TOTAL_REC srAccumRec; /*用來放總筆數、總金額的結構體*/

	/* inACCUM_Update_CMAS_Flow()_START */
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"----------------------------------------");
		inDISP_LogPrintfAt(AT,"inACCUM_Update_CMAS_Flow()_START");
	}

	memset(uszFileName, 0x00, sizeof(uszFileName));
	if (inFunc_ComposeFileName(pobTran, (char *)uszFileName, _ACCUM_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		/* Update Accum失敗 鎖機*/
		inFunc_EDCLock();
		return (VS_ERROR);
	}

	/* 開檔 create檔(若沒有檔案則創建檔案) */
	if (inFILE_Open(&ulHandlePtr, uszFileName) == (VS_ERROR))
	{
		/* 開檔失敗時，不必關檔(開檔失敗，handle回傳NULL) */
		/* 若檔案不存在時，Create檔案 */
		if (inFILE_Check_Exist(uszFileName) == (VS_ERROR))
		{
			inRetVal = inFILE_Create(&ulHandlePtr, uszFileName);
		}
		/* 檔案存在還是開檔失敗，回傳錯誤跳出 */
		else
		{
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock();
			return (VS_ERROR);
		}

		/* Create檔 */
		if (inRetVal != VS_SUCCESS)
		{
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock();
			/* Create檔失敗時不關檔，因為create檔失敗handle回傳NULL */
			return (VS_ERROR);
		}
		/* create檔成功就繼續(因為create檔已經把handle指向檔案，所以不用在開檔) */
	}

	/* 開檔成功或create檔成功後 */
	/*先清空srAccumRec 為讀檔案作準備  */
	memset(&srAccumRec, 0x00, sizeof(srAccumRec));

	/* 把指針指到開頭*/
	inRetVal = inFILE_Seek(ulHandlePtr, 0, _SEEK_BEGIN_);

	/* inFile_seek */
	if (inRetVal != VS_SUCCESS)
	{
		/* Update Accum失敗 鎖機*/
		inFunc_EDCLock();

		/* inFILE_Seek失敗時 */
		/* Seek檔失敗，所以關檔 */
		inFILE_Close(&ulHandlePtr);

		/* Seek檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
		return (VS_ERROR);
	}

	/* 2015/11/20 修改 */
	/* 當文件內容為空時，read會回傳error;所以當read失敗， 而且檔案長度不等於0的時候，才是真的出錯 */
	if (inFILE_Read(&ulHandlePtr, (unsigned char *)&srAccumRec, _TICKET_ACCUM_REC_SIZE_) == VS_ERROR && lnFILE_GetSize(&ulHandlePtr, (unsigned char *)&srAccumRec) != 0)
	{
		/* Update Accum失敗 鎖機*/
		inFunc_EDCLock();

		/* 讀檔失敗 */
		/* Read檔失敗，所以關檔 */
		inFILE_Close(&ulHandlePtr);

		/* Read檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
		return (VS_ERROR);
	}
	else
	{
		/* 計算全部交易金額、筆數 */
		if (inACCUM_UpdateTotalAmount_CMAS(pobTran, &srAccumRec) == VS_ERROR)
		{
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock();

			/* 計算全部交易金額、筆數失敗 */
			/* 計算全部交易金額、筆數失敗，所以關檔 */
			inFILE_Close(&ulHandlePtr);

			/* 計算全部交易金額、筆數失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
			return (VS_ERROR);
		}

		
		/* 先將檔案關閉 */
		if (inFILE_Close(&ulHandlePtr) == (VS_ERROR))
		{
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock();

			/* 關檔失敗 */
			/* 回傳VS_ERROR */
			return (VS_ERROR);
		}

		/* 存檔案 */
		if (inACCUM_StoreRecord_Ticket(&srAccumRec, uszFileName) == VS_ERROR)
		{
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock();

			/* 存檔失敗 */
			/* 因為inACCUM_StoreRecord失敗，所以回傳VS_ERROR */
			return (VS_ERROR);
		}

	} /* read、計算總額 和 存檔 成功 */

	/* inACCUM_Update_CMAS_Flow()_END */
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf( AT,"----------------------------------------");
		inDISP_LogPrintf( AT,"inACCUM_Update_CMAS_Flow()_END");
	}
	return (VS_SUCCESS);
}

/*
Function		: inACCUM_UpdateTotalAmount_CMAS
Date&Time	: 2022/6/8 下午 3:59
Describe		: 更新電票交易金額
 *  [新增電票悠遊卡功能]  參考合庫新增功能 [SAM] 2022/6/8 下午 4:03 
*/
int inACCUM_UpdateTotalAmount_CMAS(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec)
{
	char szDebugMsg[100 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT, "----------------------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inACCUM_UpdateTotalAmount_CMAS(%d) START !", pobTran->srBRec.inCode);
		inDISP_LogPrintfAt(AT, szDebugMsg);
	}

	switch (pobTran->srTRec.inCode)
	{
		case _TICKET_EASYCARD_DEDUCT_:
			/* 計算筆數 */
			srAccumRec->lnTotalCount++;
			srAccumRec->lnEASYCARD_TotalCount++;
			srAccumRec->lnDeductTotalCount++;
			srAccumRec->lnEASYCARD_DeductTotalCount++;
			/* 計算金額 */
			srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
			srAccumRec->llEASYCARD_TotalAmount += pobTran->srTRec.lnTxnAmount;
			srAccumRec->llDeductTotalAmount += pobTran->srTRec.lnTxnAmount;
			srAccumRec->llEASYCARD_DeductTotalAmount += pobTran->srTRec.lnTxnAmount;
                        /*20210426 mark start[Hachi]*/
//			if (pobTran->srTRec.uszAutoTopUpBit == VS_TRUE && pobTran->srTRec.lnTotalTopUpAmount > 0) /*自動加值FLAG = ON && 自動加值金額>0*/ // 如果有自動加值
//			{
//				/* 計算筆數 */
//				srAccumRec->lnTotalCount++;
//				srAccumRec->lnEASYCARD_TotalCount++;
//				srAccumRec->lnAutoADDTotalCount++;
//				srAccumRec->lnEASYCARD_AutoADDTotalCount++;
//				/* 計算金額 */
//				srAccumRec->llTotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
//				srAccumRec->llEASYCARD_TotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
//				srAccumRec->llAutoADDTotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
//				srAccumRec->llEASYCARD_AutoADDTotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
//			}
                         /*20210426 mark End[Hachi]*/
			break;
		case _TICKET_EASYCARD_VOID_TOP_UP_:
			/* 計算筆數 */
			srAccumRec->lnTotalCount++;
			srAccumRec->lnEASYCARD_TotalCount++;
			srAccumRec->lnVoidADDTotalCount++;
			srAccumRec->lnEASYCARD_VoidADDTotalCount++;
			/* 計算金額 */
			srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
			srAccumRec->llEASYCARD_TotalAmount += pobTran->srTRec.lnTxnAmount;
			srAccumRec->llVoidADDTotalAmount += pobTran->srTRec.lnTxnAmount;
			srAccumRec->llEASYCARD_VoidADDTotalAmount += pobTran->srTRec.lnTxnAmount;
			break;
		case _TICKET_EASYCARD_REFUND_:
			/* 計算筆數 */
			srAccumRec->lnTotalCount++;
			srAccumRec->lnEASYCARD_TotalCount++;
			srAccumRec->lnRefundTotalCount++;
			srAccumRec->lnEASYCARD_RefundTotalCount++;
			/* 計算金額 */
			srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
			srAccumRec->llEASYCARD_TotalAmount += pobTran->srTRec.lnTxnAmount;
			srAccumRec->llRefundTotalAmount += pobTran->srTRec.lnTxnAmount;
			srAccumRec->llEASYCARD_RefundTotalAmount += pobTran->srTRec.lnTxnAmount;
			break;
		case _TICKET_EASYCARD_TOP_UP_:
			/* 計算筆數 */
			srAccumRec->lnTotalCount++;
			srAccumRec->lnEASYCARD_TotalCount++;
			srAccumRec->lnADDTotalCount++;
			srAccumRec->lnEASYCARD_ADDTotalCount++;
			/* 計算金額 */
			srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
			srAccumRec->llEASYCARD_TotalAmount += pobTran->srTRec.lnTxnAmount;
			srAccumRec->llADDTotalAmount += pobTran->srTRec.lnTxnAmount;
			srAccumRec->llEASYCARD_ADDTotalAmount += pobTran->srTRec.lnTxnAmount;
			break;
                /*20210426[Hachi] start*/        
                case _TICKET_EASYCARD_AUTO_TOP_UP_ :
                        /* 計算筆數 */
                        srAccumRec->lnTotalCount ++;
                	srAccumRec->lnEASYCARD_TotalCount ++;
                	srAccumRec->lnAutoADDTotalCount ++;
                	srAccumRec->lnEASYCARD_AutoADDTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                	srAccumRec->llEASYCARD_TotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                	srAccumRec->llAutoADDTotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                	srAccumRec->llEASYCARD_AutoADDTotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                        break;
                 /*20210426[Hachi] End*/           
		default:
			return (VS_ERROR);
		}

		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintfAt(AT, "inACCUM_UpdateTotalAmount_CMAS() END !");
			inDISP_LogPrintfAt(AT, "----------------------------------------");
		}

		return (VS_SUCCESS);
}



