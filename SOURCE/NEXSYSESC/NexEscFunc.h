#ifndef _NEXSYS_ESC_FUNC_H_
#define _NEXSYS_ESC_FUNC_H_

#define _ESC_REPIRNT_FILE_NAME_ "EscReplog.txt"
#define _ESC_RPT_DATA_FIX_LEN_	7	/* 如果資料內容長度有變動需要一起更動 */

int inESC_OpenReprintEscFile(void);
int inESC_CloseReprintEscFile(void);
int inESC_DeleteReprintEscFile(void);
int inESC_WriteReprintEscFile(unsigned char *uszWriteBuff, unsigned long ulWriteSize);
int inESC_ReadReprintEscFile(unsigned char *uszReadBuff, unsigned long ulReadSize);
int inESC_CheckFlagAndReprintBill(TRANSACTION_OBJECT *pobTran);
int inESC_RunOptForReprintBill(TRANSACTION_OBJECT *pobTran);
int inESC_OpenAndSaveFileFlow(TRANSACTION_OBJECT *pobTran);
int inESC_ReadAndCheckRepirntFileFlow(TRANSACTION_OBJECT *pobTran);

#endif
