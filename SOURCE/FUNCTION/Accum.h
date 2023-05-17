
#ifndef __ACCUM_H__
#define __ACCUM_H__

/*
 * [檢查參數檔] 
 * 用於計算端未機刷卡金額，在交易當下就會計算，不透過讀取批資檔計算 
 * 存檔檔名會由各主機名稱組合，不會有初始檔，在結帳後或單獨砍批次檔時要刪除
 * 2022/8/12 上午 10:07 [SAM]
 */

/* VEGA3000 為32位元系統
 * 在32位元系統下，int、long都用4個byte longlong為8個byte
 * 在64位元系統下，int 使用4個byte long、longlong為8個byte
 *
 * 至於double為浮點數，使用8個byte，有效位10位，但是超過10位精度可能會被吃掉，要特別注意。
 *
 * 除了*与long随操作系统子长变化而变化外，其他的都固定不变(32位和64位相比)
 */


typedef struct Account_Total_File
{
	/* 全部交易筆數 */
	long		lnTotalSaleCount;
	long		lnTotalRefundCount;
	long		lnTotalTipsCount;
	long		lnTotalAdjustCount;
	long		lnTotalCount;
	/* 全部交易金額 */
	long long	llTotalSaleAmount;
	long long	llTotalRefundAmount;
	long long	llTotalTipsAmount;
	long long	llTotalAmount;

	/* VISA 全部交易筆數*/
	long		lnVisaTotalSaleCount;
	long		lnVisaTotalRefundCount;
	long		lnVisaTotalTipsCount;
	long		lnVisaTotalCount;
	/* VISA 全部交易金額 */
	long long	llVisaTotalSaleAmount;
	long long	llVisaTotalRefundAmount;
	long long	llVisaTotalTipsAmount;
	long long	llVisaTotalAmount;
	/* MASTER 全部交易筆數*/
	long		lnMasterTotalSaleCount;
	long		lnMasterTotalRefundCount;
	long		lnMasterTotalTipsCount;
	long		lnMasterTotalCount;
	/* MASTER 全部交易金額 */
	long long	llMasterTotalSaleAmount;
	long long	llMasterTotalRefundAmount;
	long long	llMasterTotalTipsAmount;
	long long	llMasterTotalAmount;
	/* JCB 全部交易筆數*/
	long		lnJcbTotalSaleCount;
	long		lnJcbTotalRefundCount;
	long		lnJcbTotalTipsCount;
	long		lnJcbTotalCount;
	/* JCB 全部交易金額 */
	long long	llJcbTotalSaleAmount;
	long long	llJcbTotalRefundAmount;
	long long	llJcbTotalTipsAmount;
	long long	llJcbTotalAmount;
	/* U_CARD 全部交易筆數*/
	long		lnUCardTotalSaleCount;
	long		lnUCardTotalRefundCount;
	long		lnUCardTotalTipsCount;
	long		lnUCardTotalCount;
	/* U_CARD 全部交易金額 */
	long long	llUCardTotalSaleAmount;
	long long	llUCardTotalRefundAmount;
	long long	llUCardTotalTipsAmount;
	long long	llUCardTotalAmount;
	/* AMEX 全部交易筆數*/
	long		lnAmexTotalSaleCount;
	long		lnAmexTotalRefundCount;
	long		lnAmexTotalTipsCount;
	long		lnAmexTotalCount;
	/* AMEX 全部交易金額 */
	long long	llAmexTotalSaleAmount;
	long long	llAmexTotalRefundAmount;
	long long	llAmexTotalTipsAmount;
	long long	llAmexTotalAmount;
	/* DINERS 全部交易筆數*/
	long		lnDinersTotalSaleCount;
	long		lnDinersTotalRefundCount;
	long		lnDinersTotalTipsCount;
	long		lnDinersTotalCount;
	/* DINERS 全部交易金額 */
	long long	llDinersTotalSaleAmount;
	long long	llDinersTotalRefundAmount;
	long long	llDinersTotalTipsAmount;
	long long	llDinersTotalAmount;
	/* CUP 全部交易筆數*/
	long		lnCupTotalSaleCount;
	long		lnCupTotalRefundCount;
	long		lnCupTotalTipsCount;
	long		lnCupTotalCount;
	/* CUP 全部交易金額 */
	long long	llCupTotalSaleAmount;
	long long	llCupTotalRefundAmount;
	long long	llCupTotalTipsAmount;
	long long	llCupTotalAmount;
	/* Smart Pay 全部交易筆數 */
	long		lnFiscTotalSaleCount;
	long		lnFiscTotalRefundCount;
	long		lnFiscTotalCount;
	/* Smart Pay 全部交易金額 */
	long long	llFiscTotalSaleAmount;
	long long	llFiscTotalRefundAmount;
	long long	llFiscTotalAmount;

	/* 紅利抵扣筆數 */
	long		lnRedeemSaleCount;
	long		lnRedeemRefundCount;
	long		lnRedeemTipsCount;
	long		lnRedeemTotalCount;
	long		lnRedeemTotalPoint;
	/* 紅利抵扣金額 */
	long long	llRedeemSaleAmount;
	long long	llRedeemRefundAmount;
	long long	llRedeemTipsAmount;
	long long	llRedeemTotalAmount;

	/* 分期付款筆數 */
	long		lnInstSaleCount;
	long		lnInstRefundCount;
	long		lnInstTipsCount;
	long		lnInstTotalCount;
	/* 分期付款金額 */
	long long	llInstSaleAmount;
	long long	llInstRefundAmount;
	long long	llInstTipsAmount;
	long long	llInstTotalAmount;

	/* 優惠兌換數量 */
	long		lnLoyaltyRedeemSuccessCount;	/* 兌換成功總數量 */
	long		lnLoyaltyRedeemCancelCount;	/* 兌換取消總數量 */
	long		lnLoyaltyRedeemTotalCount;	/* 兌換合計 */

	/* ESC */
	long		lnESC_BypassNum;		/* ESC上傳時，因Bypasst出簽單的次數 */
	long		lnESC_TotalFailULNum;	 /* ESC上傳時，有出簽單的次數 */
	long		lnESC_FailUploadNum;		/* ESC上傳時，因上傳失敗出簽單的次數 */
	long		lnESC_SuccessNum;		/* ESC上傳時，成功的次數 */
	long		lnESC_PreAuthNum;		/* ESC上傳時，預先授權的次數 */
	long		lnESC_RePrintNum;		/* 重印時要加總紙本數量，另外統計用 20181220 [SAM] */

 	long long	llESC_BypassAmount;		/* ESC上傳時，因Bypasst出簽單的總金額 */
	long long	llESC_TotalFailULAmount;	/* ESC上傳時，有出簽單的總金額 */
	long long	llESC_FailUploadAmount;	/* ESC上傳時，因上傳失敗出簽單的總金額 */
	long long	llESC_SuccessAmount;		/* ESC上傳時，成功的總金額 */
	long long	llESC_PreAuthAmount;		/* ESC上傳時，預先授權的總金額 */

	/* 一般交易交易筆數 */
	long 	lnTotalCreditSaleCount;
	long	lnTotalCreditRefundCount;
	long	lnTotalCreditTipsCount;
	long	lnTotalCreditCount;
	/* 一般交易交易金額 */
	long long	llTotalCreditSaleAmount;
	long long	llTotalCreditRefundAmount;
	long long	llTotalCreditTipsAmount;
	long long	llTotalCreditAmount;
} ACCUM_TOTAL_REC;


typedef struct
{
	/* 全部交易筆數 */
	long		lnTotalCount;
	long		lnDeductTotalCount;
	long		lnVoidDeductTotalCount;
	long		lnADDTotalCount;
	long		lnAutoADDTotalCount;
	long		lnVoidADDTotalCount;
	long		lnRefundTotalCount;

	long long	llTotalAmount;
	long long	llDeductTotalAmount;
	long long	llVoidDeductTotalAmount;
	long long	llADDTotalAmount;
	long long	llAutoADDTotalAmount;
	long long	llVoidADDTotalAmount;
	long long	llRefundTotalAmount;

	/* IPASS */
	long		lnIPASS_TotalCount;
	long		lnIPASS_DeductTotalCount;
	long		lnIPASS_VoidDeductTotalCount;
	long		lnIPASS_ADDTotalCount;
	long		lnIPASS_AutoADDTotalCount;
	long		lnIPASS_VoidADDTotalCount;
	long		lnIPASS_RefundTotalCount;

	long long	llIPASS_TotalAmount;
	long long	llIPASS_DeductTotalAmount;
	long long	llIPASS_VoidDeductTotalAmount;
	long long	llIPASS_ADDTotalAmount;
	long long	llIPASS_AutoADDTotalAmount;
	long long	llIPASS_VoidADDTotalAmount;
	long long	llIPASS_RefundTotalAmount;

	/* ECC */
	long		lnEASYCARD_TotalCount;
	long		lnEASYCARD_DeductTotalCount;
	long		lnEASYCARD_VoidDeductTotalCount;
	long		lnEASYCARD_ADDTotalCount;
	long		lnEASYCARD_AutoADDTotalCount;
	long		lnEASYCARD_VoidADDTotalCount;
	long		lnEASYCARD_RefundTotalCount;

	long long	llEASYCARD_TotalAmount;
	long long	llEASYCARD_DeductTotalAmount;
	long long	llEASYCARD_VoidDeductTotalAmount;
	long long	llEASYCARD_ADDTotalAmount;
	long long	llEASYCARD_AutoADDTotalAmount;
	long long	llEASYCARD_VoidADDTotalAmount;
	long long	llEASYCARD_RefundTotalAmount;

	/* ICASH */
	long		lnICASH_TotalCount;
	long		lnICASH_DeductTotalCount;
	long		lnICASH_VoidDeductTotalCount;
	long		lnICASH_ADDTotalCount;
	long		lnICASH_AutoADDTotalCount;
	long		lnICASH_VoidADDTotalCount;
	long		lnICASH_RefundTotalCount;

	long long	llICASH_TotalAmount;
	long long	llICASH_DeductTotalAmount;
	long long	llICASH_VoidDeductTotalAmount;
	long long	llICASH_ADDTotalAmount;
	long long	llICASH_AutoADDTotalAmount;
	long long	llICASH_VoidADDTotalAmount;
	long long	llICASH_RefundTotalAmount;

	/* HAPPYCASH */
	long		lnHAPPYCASH_TotalCount;
	long		lnHAPPYCASH_DeductTotalCount;
	long		lnHAPPYCASH_VoidDeductTotalCount;
	long		lnHAPPYCASH_ADDTotalCount;
	long		lnHAPPYCASH_AutoADDTotalCount;
	long		lnHAPPYCASH_VoidADDTotalCount;
	long		lnHAPPYCASH_RefundTotalCount;

	long long	llHAPPYCASH_TotalAmount;
	long long	llHAPPYCASH_DeductTotalAmount;
	long long	llHAPPYCASH_VoidDeductTotalAmount;
	long long	llHAPPYCASH_ADDTotalAmount;
	long long	llHAPPYCASH_AutoADDTotalAmount;
	long long	llHAPPYCASH_VoidADDTotalAmount;
	long long	llHAPPYCASH_RefundTotalAmount;
} TICKET_ACCUM_TOTAL_REC;

#define _ACCUM_REC_SIZE_			sizeof(ACCUM_TOTAL_REC)
#define _TICKET_ACCUM_REC_SIZE_	sizeof(TICKET_ACCUM_TOTAL_REC)


int inACCUM_UpdateTotalAmount(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec);
int inACCUM_UpdateTotalAmountByCard(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec);
int inACCUM_StoreRecord(ACCUM_TOTAL_REC *srAccumRec,unsigned char *szFileName);
int inACCUM_GetRecord(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec);
int inACCUM_GetRecord_ESVC(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec);
int inACCUM_ReviewReport_Total(TRANSACTION_OBJECT *pobTran);
int inACCUM_ReviewReport_Total_Settle(TRANSACTION_OBJECT *pobTran);
int inACCUM_UpdateFlow(TRANSACTION_OBJECT *pobTran);
int inACCUM_UpdateFlow_ESC(TRANSACTION_OBJECT *pobTran, int inUpdateType);
int inACCUM_Update_ESC_TotalAmount(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, int inUpdateType);
int inACCUM_Check_Specific_Accum(TRANSACTION_OBJECT *pobTran, char *szHostName, ACCUM_TOTAL_REC* srACCUMRec);
int inACCUM_Check_Transaction_Count(TRANSACTION_OBJECT *pobTran, char *szHostName, char *szTrans);
int inACCUM_UpdateLoyaltyRedeem(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec);
/* 票證用 */
int inACCUM_Update_Ticket_Flow(TRANSACTION_OBJECT *pobTran);
int inACCUM_UpdateTotalAmount_Ticket(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec);
int inACCUM_StoreRecord_Ticket(TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszFileName);

/* [新增電票悠遊卡功能]  參考合庫新增功能 [SAM] 2022/6/8 下午 4:03 */
int inACCUM_Update_CMAS_Flow(TRANSACTION_OBJECT *pobTran);
int inACCUM_UpdateTotalAmount_CMAS(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec);
/* [新增電票悠遊卡功能] END 參考合庫新增功能 [SAM] 2022/6/8 下午 4:03 */

#endif
