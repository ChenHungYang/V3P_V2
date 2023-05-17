/* 
 * File:   SvcAccum.h
 * Author: Sam chang
 *
 * Created on 2022年12月26日, 下午 7:52
 */

#ifndef SVCACCUM_H
#define	SVCACCUM_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct SVC_Account_Total_File
{
        /* 全部交易筆數 */
        long    lnTotalRedeemCount;
        long    lnTotalReloadCount;
        long    lnTotalActiveCardCount;
        long    lnTotalVoidCount;
        long    lnTotalRefundCount;
        long    lnTotalBackcardCount;
        long    lnTotalCount;

        /* 全部交易金額 */
        double  dbTotalRedeemAmount;
        double  dbTotalReloadAmount;
        double  dbTotalActiveCardAmount;
        double  dbTotalVoidAmount;
        double  dbTotalRefundAmount;
        double  dbTotalBackcardAmount;
        double  dbTotalAmount;
} SVC_ACCUM_TOTAL_REC;

#define SVC_ACCUM_TOTAL_REC_SIZE	sizeof(SVC_ACCUM_TOTAL_REC)

int inSVC_ACCUM_StoreRecord(SVC_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszFileName);
int inSVC_ACCUM_UpdateTotalAmount(TRANSACTION_OBJECT *pobTran, SVC_ACCUM_TOTAL_REC *srAccumRec);
int inSVC_ACCUM_Update_Flow(TRANSACTION_OBJECT *pobTran);

#ifdef	__cplusplus
}
#endif

#endif	/* SVCACCUM_H */

