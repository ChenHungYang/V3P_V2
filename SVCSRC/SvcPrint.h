/* 
 * File:   SvcPrint.h
 * Author: Sam chang
 *
 * Created on 2022年12月26日, 下午 2:50
 */

#ifndef SVCPRINT_H
#define	SVCPRINT_H

#ifdef	__cplusplus
extern "C" {
#endif

/* 列印帳單 (START) */
typedef struct
{
        int (*inLogo)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* LOGO */
        int (*inTop)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* TID and MID */
        int (*inData)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* DTAT */
        int (*inAmount)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* AMOUNT */
        int (*inEnd)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* 簽名欄 */
} SVC_PRINT_RECEIPT_TYPE_TABLE_BYBUFFER;
	
void vdSVC_PRINT_GetTransType(TRANSACTION_OBJECT *pobTran, char *szPrintBuf1);
int inSVC_PRINT_ReceiptData(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inSVC_PRINT_ReceiptAmountData(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inSVC_PRINT_ReceiptBottomData(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);


#ifdef	__cplusplus
}
#endif

#endif	/* SVCPRINT_H */

