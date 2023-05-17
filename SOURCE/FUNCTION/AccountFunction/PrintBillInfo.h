/*
 * File:   CreditprtByBuffer.h
 * Author: user
 *
 * Created on 2016年1月30日, 上午 11:55
 */
#ifndef __PRINT_BILL_INFO_H__
#define __PRINT_BILL_INFO_H__


typedef enum
{
	_REPORT_INDEX_NORMAL_ = 0,
	_REPORT_INDEX_NORMAL_DCC_FOR_SALE_,
	_REPORT_INDEX_NORMAL_DCC_NOT_FOR_SALE_,
	_REPORT_INDEX_NORMAL_DCC_CHANGE_TWD_,
	_REPORT_INDEX_NORMAL_LOYALTY_REDEEM_,
	_REPORT_INDEX_NORMAL_VOID_LOYALTY_REDEEM_,
	_REPORT_INDEX_NORMAL_FISC_,
	_REPORT_INDEX_NORMAL_HG_SINGLE_,
	_REPORT_INDEX_NORMAL_HG_MULTIPLE_,
	_REPORT_INDEX_NORMAL_ESVC_IPASS_,
	_REPORT_INDEX_NORMAL_ESVC_ECC_,				/* 10 */
	_REPORT_INDEX_NORMAL_ESVC_ICASH_,
	_REPORT_INDEX_NORMAL_ESVC_HAPPYCASH_,
	_REPORT_INDEX_NORMAL_SMALL_,
	_REPORT_INDEX_NORMAL_DCC_FOR_SALE_SMALL_,
	_REPORT_INDEX_NORMAL_DCC_NOT_FOR_SALE_SMALL_,
	_REPORT_INDEX_NORMAL_DCC_CHANGE_TWD_SMALL_,
	_REPORT_INDEX_NORMAL_LOYALTY_REDEEM_SMALL_,
	_REPORT_INDEX_NORMAL_VOID_LOYALTY_REDEEM_SMALL_,
	_REPORT_INDEX_NORMAL_FISC_SMALL_,
	_REPORT_INDEX_NORMAL_HG_SINGLE_SMALL_,			/* 20 */
	_REPORT_INDEX_NORMAL_HG_MULTIPLE_SMALL_,
} RECEIPT_INDEX;

typedef enum
{
	_TOTAL_REPORT_INDEX_NORMAL_ = 0,
	_TOTAL_REPORT_INDEX_NCCC_,
} RECEIPT_TOTAL_INDEX;

typedef enum
{
	_DETAIL_REPORT_INDEX_NORMAL_ = 0,
	_DETAIL_REPORT_INDEX_NCCC_,
	_DETAIL_REPORT_INDEX_DCC_,
	_DETAIL_REPORT_INDEX_HG_,
} RECEIPT_DETAIL_INDEX;

typedef enum
{
	_TOTAL_REPORT_INDEX_ESVC_ = 0,
	_TOTAL_REPORT_INDEX_ESVC_SETTLE_,
} RECEIPT_TOTAL_ESVC_INDEX;

typedef enum
{
	_DETAIL_REPORT_INDEX_ESVC_ = 0,
} RECEIPT_DETAIL_ESVC_INDEX;

/* 列印帳單 (START) */
typedef struct
{
        int (*inLogo)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* LOGO */
        int (*inTop)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* TID and MID */
        int (*inData)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* DTAT */
        int (*inCUPAmount)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);	/* CUP_AMOUNT */
        int (*inAmount)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* AMOUNT */
        int (*inInstallment)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);	/* 分期資料 */
        int (*inRedemption)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);	/* 紅利資料 */
        int (*inOther)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* OTHER資料 */
        int (*inEnd)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* 簽名欄 */
} PRINT_RECEIPT_TYPE_TABLE_BYBUFFER;
/* 列印帳單 (END) */

#define _NCCC_SLOGAN_PRINT_UP_		1
#define _NCCC_SLOGAN_PRINT_DOWN_	2

/* 雜用function (START) */
int inCREDIT_PRINT_MarchantSlogan(TRANSACTION_OBJECT *pobTran, int inPrintPosition, unsigned char *uszBuffer, BufferHandle *srBhandle);
int inCREDIT_PRINT_MerchantLogo(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, BufferHandle *srBhandle);
int inCREDIT_PRINT_MerchantName(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, BufferHandle *srBhandle);
int inCREDIT_PRINT_MerchantNameTXT(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Notice(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, BufferHandle *srBhandle);
int inCREDIT_PRINT_Printing_Time(TRANSACTION_OBJECT *pobTran, int inFontSize, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_AutoSettle_Failed_ByBuffer(void);
int inCREDIT_PRINT_TerminalTraceLog_ByBuffer(void);
/* 雜用function (END) */

/* 列印帳單 (START) */
int inCREDIT_PRINT_Receipt_Flow_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_Logo_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Tidmid_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Data_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Cup_Amount_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Amount_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_NewAmountByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Inst_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_NewInstByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Redeem_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_NewRedeemByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ReceiptEND_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
/* 列印帳單 (END) */

/* 列印帳單縮小版 (START) */
int inCREDIT_PRINT_Tidmid_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Data_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Cup_Amount_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Amount_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Inst_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Redeem_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ReceiptEND_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_FISC_Data_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_FISC_Amount_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
/* 列印帳單縮小版 (END) */

int inCREDIT_PRINT_Check_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);

/* 列印總額報表(START) */
typedef struct
{
        int (*inReportCheck)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);					/* 檢查檔案是否存在 */
        int (*inReportLogo)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);					/* LOGO */
        int (*inReportTop)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);					/* TID and MID and DATA */
        int (*inAmount)(TRANSACTION_OBJECT *, ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);			/* 全部金額總計 */
        int (*inAmountByCard)(TRANSACTION_OBJECT *, ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* 卡別金額總計 */
        int (*inAmountByCredit)(TRANSACTION_OBJECT *, ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);
        int (*inAmountByInstallment)(TRANSACTION_OBJECT *, ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);	/* 分期金額總計 */
        int (*inAmountByRedemption)(TRANSACTION_OBJECT *, ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);	/* 紅利金額總計 */
	int (*inAmountByOther)(TRANSACTION_OBJECT *, ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* 其他總計 */
        int (*inReportEnd)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);												/* 結束 */
} TOTAL_REPORT_TABLE_BYBUFFER;
/* 列印總額報表 (END) */


/* 列印總額報表 (START) */
int inCREDIT_PRINT_TotalReport_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_TotalReport_ByBuffer_NCCC(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_Top_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmount_ByBuffer(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmountByCredit_ByBuffer(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmountByInstllment_ByBuffer(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmountByRedemption_ByBuffer(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmountByOther(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Total_Loyalty_Redeem_ByBuffer(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_End_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ESC_Reinforce_Count_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
/* 列印總額報表 (END) */

/* 列印明細報表(START) */
typedef struct
{
        int (*inReportCheck)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* 檢查檔案是否存在 */
        int (*inReportLogo)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* LOGO */
        int (*inReportTop)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* TID and MID and DATA */
        int (*inTotalAmount)(TRANSACTION_OBJECT *, ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* 全部金額總計 */
        int (*inMiddle)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);					/* 明細中文 */
        int (*inBottom)(TRANSACTION_OBJECT *, int, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* 所有明細 */
        int (*inReportEnd)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* 結束 */
} DETAIL_REPORT_TABLE_BYBUFFER;
/* 列印明細報表 (END) */

/* 列印明細報表(START) */
int inCREDIT_PRINT_DetailReport_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_NCCC_DetailReport_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_DetailReportMiddle_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_NCCC_DetailReportMiddle_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DetailReportBottom_ByBuffer(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DetailReportBottomByBufferBigWord(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
/* 列印明細報表 (END) */

/* 列印端末機參數(START) */
int inCREDIT_PRINT_Func7PrintParamTerm_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_ParamLOGO_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char* uszBuffer, FONT_ATTRIB* srFont_Attrib, BufferHandle* srBhandle);
int inCREDIT_PRINT_ParamTermInformation_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ParamHostDetailParam_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ParamCardType_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ParamManageNum_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ProductCode_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_CAPublicKey_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ParamSystemConfig_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ParamLOGO_END_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
/* 列印端末機參數(END) */


/* SmartPay相關 */
int inCREDIT_PRINT_FISC_Data_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_FISC_Amount_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);


/* TMS排程使用 */
int inCREDIT_PRINT_Schedule_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);


/* 票證相關 */
/* 列印票證總額報表(START) */
typedef struct
{
        int (*inReportCheck)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);					/* 檢查檔案是否存在 */
        int (*inReportLogo)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);					/* LOGO */
        int (*inReportTop)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);					/* TID and MID and DATA */
        int (*inAmount)(TRANSACTION_OBJECT *, TICKET_ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* 全部金額總計 */
        int (*inAmountByCard)(TRANSACTION_OBJECT *, TICKET_ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* 卡別金額總計 */
        int (*inAmountByInstallment)(TRANSACTION_OBJECT *, TICKET_ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);	/* 分期金額總計 */
        int (*inAmountByRedemption)(TRANSACTION_OBJECT *, TICKET_ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);	/* 紅利金額總計 */
	int (*inAmountByOther)(TRANSACTION_OBJECT *, TICKET_ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* 其他總計 */
        int (*inReportEnd)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);												/* 結束 */
} TOTAL_REPORT_TABLE_BYBUFFER_ESVC;
/* 列印票證總額報表 (END) */

int inCREDIT_PRINT_Data_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Amount_ByBuffer_IPASS(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Amount_ByBuffer_ECC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ReceiptEND_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalReport_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_Logo_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Top_ESVC_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Top_ESVC_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Top_ESVC_SETTLE_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmount_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmount_ByBuffer_ESVC_Settle(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer_ESVC_Settle(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_End_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);

/* 列印明細報表(START) */
typedef struct
{
        int (*inReportCheck)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* 檢查檔案是否存在 */
        int (*inReportLogo)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* LOGO */
        int (*inReportTop)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* TID and MID and DATA */
        int (*inTotalAmount)(TRANSACTION_OBJECT *, TICKET_ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);	/* 全部金額總計 */
        int (*inMiddle)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);					/* 明細中文 */
        int (*inBottom)(TRANSACTION_OBJECT *, int, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* 所有明細 */
        int (*inReportEnd)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* 結束 */
} DETAIL_REPORT_TABLE_BYBUFFER_ESVC;
/* 列印明細報表 (END) */
int inCREDIT_PRINT_DetailReport_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_Check_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DetailReportMiddle_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DetailReportBottom_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);

int inCREDIT_PRINTBYBUFFER_Receipt_Test(void);
int inCREDIT_PRINT_Test(void);

int inEDC_PRINT_ParamQRCodeConfig_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);

#endif

