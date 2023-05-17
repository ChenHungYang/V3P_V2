/*
 * File:   Creditprt.h
 * Author: user
 *
 * Created on 2015/8/13 下午 5:02
 */

/* 列印帳單 (START) */
typedef struct
{
        int (*inLogo)(TRANSACTION_OBJECT *);				/* LOGO */
        int (*inTop)(TRANSACTION_OBJECT *);				/* TID and MID */
        int (*inData)(TRANSACTION_OBJECT *);				/* DTAT */
        int (*inCUPAmount)(TRANSACTION_OBJECT *);			/* CUP_AMOUNT */
        int (*inAmount)(TRANSACTION_OBJECT *);				/* AMOUNT */
        int (*inInstallment)(TRANSACTION_OBJECT *);			/* 分期資料 */
        int (*inRedemption)(TRANSACTION_OBJECT *);			/* 紅利資料 */
        int (*inOther)(TRANSACTION_OBJECT *);				/* OTHER資料 */
        int (*inEnd)(TRANSACTION_OBJECT *);				/* 簽名欄 */
} PRINT_RECEIPT_TYPE_TABLE;
/* 列印帳單 (END) */

#define _NCCC_SLOGAN_PRINT_UP_		1
#define _NCCC_SLOGAN_PRINT_DOWN_	2

/* 列印帳單 (START) */
int inCREDIT_PRINT_Receipt(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_Logo(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_Tidmid(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_Data(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_CUP_Amount(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_Amount(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_Inst(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_Redeem(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_Receiptend(TRANSACTION_OBJECT *pobTran);
/* 列印帳單 (END) */

int inCREDIT_PRINT_Check(TRANSACTION_OBJECT *pobTran);


/* 列印總額報表(START) */
typedef struct
{
        int (*inReportCheck)(TRANSACTION_OBJECT *);                                             /* 檢查檔案是否存在 */
        int (*inReportLogo)(TRANSACTION_OBJECT *);                                              /* LOGO */
        int (*inReportTop)(TRANSACTION_OBJECT *);                                               /* TID and MID and DATA */
        int (*inAmount)(TRANSACTION_OBJECT *, ACCUM_TOTAL_REC *, int inRecordCnt);              /* 全部金額總計 */
        int (*inAmountByCard)(TRANSACTION_OBJECT *, ACCUM_TOTAL_REC *, int inRecordCnt);        /* 卡別金額總計 */
        int (*inAmountByInstallment)(TRANSACTION_OBJECT *, ACCUM_TOTAL_REC *, int inRecordCnt); /* 分期金額總計 */
        int (*inAmountByRedemption)(TRANSACTION_OBJECT *, ACCUM_TOTAL_REC *, int inRecordCnt);  /* 紅利金額總計 */
        int (*inReportEnd)(TRANSACTION_OBJECT *);                                               /* 結束 */
} TOTAL_REPORT_TABLE;
/* 列印總額報表 (END) */


/* 列印總額報表 (START) */
int inCREDIT_PRINT_TotalReport(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_Top(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_TotalAmount(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, int inRecordCnt);
int inCREDIT_PRINT_TotalAmountByCard(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, int inRecordCnt);
int inCREDIT_PRINT_TotalAmountByInstllment(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, int inRecordCnt);
int inCREDIT_PRINT_TotalAmountByRedemption(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, int inRecordCnt);
int inCREDIT_PRINT_End(TRANSACTION_OBJECT *pobTran);
/* 列印總額報表 (END) */

/* 列印明細報表(START) */
typedef struct
{
        int (*inReportCheck)(TRANSACTION_OBJECT *);                                             /* 檢查檔案是否存在 */
        int (*inReportLogo)(TRANSACTION_OBJECT *);                                              /* LOGO */
        int (*inReportTop)(TRANSACTION_OBJECT *);                                               /* TID and MID and DATA */
        int (*inTotalAmount)(TRANSACTION_OBJECT *, ACCUM_TOTAL_REC *, int inRecordCnt);         /* 全部金額總計 */
        int (*inMiddle)(TRANSACTION_OBJECT *, int inRecordCnt);                                 /* 明細中文 */
        int (*inBottom)(TRANSACTION_OBJECT *, int inRecordCnt);                                 /* 所有明細 */
        int (*inReportEnd)(TRANSACTION_OBJECT *);                                               /* 結束 */
} DETAIL_REPORT_TABLE;
/* 列印明細報表 (END) */

/* 列印明細報表(START) */
int inCREDIT_PRINT_DetailReport(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_DetailReportMiddle(TRANSACTION_OBJECT *pobTran, int inRecordCnt);
int inCREDIT_PRINT_DetailReportBottom(TRANSACTION_OBJECT *pobTran, int inRecordCnt);
/* 列印明細報表 (END) */

/* 列印端末機參數(START) */
int inCREDIT_Func7PrintParamTerm(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_ParamLOGO(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_ParamTermInformation(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_ParamHostDetailParam(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_ParamCardType(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_ParamManageNum(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_ProductCode(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_CAPublicKey(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_ParamSystemConfig(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_ParamLOGO_END(TRANSACTION_OBJECT *pobTran);
/* 列印端末機參數(END) */
