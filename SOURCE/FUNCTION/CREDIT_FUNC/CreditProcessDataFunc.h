
#ifndef __CREDIT_PROCESS_DATA_FUNC_H__
#define __CREDIT_PROCESS_DATA_FUNC_H__


int inCREDIT_Func_GetAmount(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_OPT_Amount(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetRefundAmount(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_OPT_RefundAmount(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetStoreID(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_OPT_StoreID(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetAuthCode(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_OPT_AuthCode(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetTipAmount(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetAdjustAmount(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetAdjustAmount2(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetPeriod(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_OPT_Period(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetDownPayment(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_OPT_DownPayment(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetInstPayment(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_OPT_InstPayment(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetInstFee(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_OPT_InstFee(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetPayAmount(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_OPT_PayAmount(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetPaidAmount(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_OPT_PaidAmount(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetRedeemPoint(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_OPT_RedeemPoint(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_CheckResult(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetOriTransDate(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_OPT_OriTransDate(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_OPT_OriTransDate_ESVC(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetOriAmount(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetOriPreAuthAmount(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_OPT_OriPreAuthAmount(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetPreCompAmount(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_OPT_PreCompAmount(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_Card_Number_Txno_Flow(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_Card_Number(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_TransactionNO(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_HG_Card_Number(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_Exp_Date(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_CheckNO(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_CheckNO_ExpDate_Flow(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_Barcode1(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_Barcode2(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetProductCode(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_OPT_ProductCode(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetCVV2(TRANSACTION_OBJECT *pobTran);


/* DCC 專用*/
int inCREDIT_Func_GetDCCOriTransDate(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetDCCOriAmount(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetDCCTipAmount(TRANSACTION_OBJECT *pobTran);
/* SMARTPAY專用 */
int inCREDIT_Func_GetFiscRRN(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_OPT_FiscRRN(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetFiscRefund(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_OPT_FiscRefund(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_GetFiscOriTransDate(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_OPT_FiscOriTransDate(TRANSACTION_OBJECT *pobTran);
/* 票證使用 */
int inCREDIT_Func_Get_OPT_RFNumber(TRANSACTION_OBJECT *pobTran);
int inCREDIT_Func_Get_OPT_CUP_TN(TRANSACTION_OBJECT *pobTran);



#endif
