/* 
 * File:   CMASprtByBuffer.h
 * Author: Hachi
 *
 * Created on 2020年6月20日, 下午 1:14
 */

#ifndef CMASPRTBYBUFFER_H
#define CMASPRTBYBUFFER_H

#ifdef __cplusplus
extern "C"
{
#endif
    
/*簽單、總額(結帳報表)、明細報表流程*/
int inCMAS_PRINT_Receipt_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inCMAS_PRINT_TotalReport_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inCMAS_PRINT_DetailReport_ByBuffer(TRANSACTION_OBJECT *pobTran);
/*功能*/
int inCMAS_PRINT_Check_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCMAS_PRINT_Notice(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, BufferHandle *srBhandle);
int inCMAS_PRINT_End_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);

/* 列印帳單 (START) */
typedef struct
{
    int (*inLogo)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);   /* LOGO */
    int (*inTop)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);    /* TID and MID */
    int (*inData)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);   /* DTAT */
    int (*inAmount)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *); /* AMOUNT */
    int (*inOther)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);  /* OTHER資料 */
    int (*inEnd)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);    /* 客服資訊、存根聯 */
} PRINT_CMAS_RECEIPT_TYPE_TABLE_BYBUFFER;
/* 列印帳單 (END) */
int inCMAS_PRINT_Logo_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCMAS_PRINT_Tidmid_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCMAS_PRINT_Data_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCMAS_PRINT_Amount_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCMAS_PRINT_ReceiptEND_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);

/* 票證相關 */
/* 列印票證總額報表(START) */
typedef struct
{
    int (*inReportCheck)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);                             /* 檢查檔案是否存在 */
    int (*inReportLogo)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);                              /* LOGO */
    int (*inReportTop)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);                               /* TID and MID and DATA */
    int (*inAmount)(TRANSACTION_OBJECT *, TICKET_ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);        /* 全部金額總計 */
    int (*inAmountByCard)(TRANSACTION_OBJECT *, TICKET_ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);  /* 卡別金額總計 */
    int (*inAmountByOther)(TRANSACTION_OBJECT *, TICKET_ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *); /* 其他總計 */
    int (*inReportEnd)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);                               /* 結束 */
} TOTAL_CMAS_REPORT_TABLE_BYBUFFER;
/* 列印票證總額報表 (END) */
int inCMAS_PRINT_Top_SETTLE_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCMAS_PRINT_TotalAmount_ByBuffer_Settle(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCMAS_PRINT_TotalAmountByCard_ByBuffer(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);

/* 列印明細報表(START) */
typedef struct
{
    int (*inReportCheck)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);                           /* 檢查檔案是否存在 */
    int (*inReportLogo)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);                            /* LOGO */
    int (*inReportTop)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);                             /* TID and MID and DATA */
    int (*inTotalAmount)(TRANSACTION_OBJECT *, TICKET_ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *); /* 全部金額總計 */
    int (*inMiddle)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);                                /* 明細中文 */
    int (*inBottom)(TRANSACTION_OBJECT *, int, unsigned char *, FONT_ATTRIB *, BufferHandle *);                           /* 所有明細 */
    int (*inReportEnd)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);                             /* 結束 */
} DETAIL_CMAS_REPORT_TABLE_BYBUFFER;
/* 列印明細報表 (END) */
int inCMAS_PRINT_DetailReportTop_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCMAS_PRINT_DetailReportMiddle_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCMAS_PRINT_DetailReportBottom_ByBuffer(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
#ifdef __cplusplus
}
#endif

#endif /* CMASPRTBYBUFFER_H */