#ifndef __ECR_PACK_FUNCTION__
#define __ECR_PACK_FUNCTION__


#define ECR_RL_SUCCESS      0
#define ECR_RL_ERROR         -1

int inECR_PACK_Filler(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr, int inSkipLen);
int inECR_PACK_Zero(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr, int inSkipLen);
int inECR_PACK_FillBlankWithoutAddLen(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr, int inSkipLen);
int inECR_PACK_ZeioWithoutAddLen(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr, int inSkipLen);
int inECR_PACK_EcrIndicator(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_CtlsIndicator(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_VersionDate(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_TransType(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_CardTypeIndicator(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);

int inECR_PACK_SuccessResponseCode(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_InvoiceNumber(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_HostId(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_CardNumber(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_ExpireDate(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_Reference(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_TransDate(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_TransTime(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_ApprovalNumber(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_TerminalId(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_CustomerName(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_CardType(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);

int inECR_PACK_MerchantId(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_StoreId(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_BatchNumber(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);

int inECR_PACK_InstallmentIndicator(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_InstallmentPeriod(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_InstallmentDownPaymentAmount(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_InstallmentAmount(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_InstallmentFormalityFee(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);

int inECR_PACK_RedeemIndicator(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_RedeemptionPaidAmount(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_RedeemptionPoint(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_RedeemptionBalancePoint(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_RedeemptionAmount(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);


int inECR_PACK_TransAmount(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_HashCardNumber(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);


/* 可以通用，所以沒有特定命名 */
int inECR_PACK_GetTotalAmountRecordFromSettleIso(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAmtTemp);
int inECR_PACK_GetTotalAmountRecord(TRANSACTION_OBJECT *pobTran);
int inECR_PACK_HostName(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_SaleTotalCount(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_SaleTotalAmt(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_RefundTotalCount(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);
int inECR_PACK_RefundTotalAmt(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);

int inECR_PACK_NewHashCardNumber(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr);

#endif

