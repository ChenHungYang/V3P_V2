#define _BATCH_LAST_RECORD_		-1
#define _BATCH_INVALID_RECORD_		-11


int inBATCH_FuncUpdateTxnRecord(TRANSACTION_OBJECT *pobTran);
int inBATCH_FuncUserChoice(TRANSACTION_OBJECT *pobTran);


int inBATCH_StoreBatchKeyFile(TRANSACTION_OBJECT *pobTran, TRANS_BATCH_KEY *srBKeyRec);
int inBATCH_SearchRecord(TRANSACTION_OBJECT *pobTran, TRANS_BATCH_KEY *srBatchKeyRec, unsigned long ulBAK_Handle, int inRecTotalCnt);
int inBATCH_CheckReport(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetTotalCountFromBakFile(TRANSACTION_OBJECT *pobTran);
int inBATCH_OpenBatchKeyFile(TRANSACTION_OBJECT *pobTran, unsigned long ulBAK_Handle);
int inBATCH_OpenBatchRecFile(TRANSACTION_OBJECT *pobTran, unsigned long ulBAT_Handle);
int inBATCH_ReviewReport_Detail(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetDetailRecords(TRANSACTION_OBJECT *pobTran, int inStartCnt);
int inBATCH_GetInvoiceNumber(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetTransRecord(TRANSACTION_OBJECT *pobTran);
int inBATCH_GlobalBatchHandleReadOnly(void);
int inBATCH_GlobalBatchHandleClose(void);

int inBATCH_GlobalAdviceHandleReadOnly(void);
int inBATCH_GlobalAdviceHandleClose(void);
int inBATCH_GetAdviceDetailRecord(TRANSACTION_OBJECT *pobTran, int inADVCnt);

int inBATCH_ESC_Save_Advice(TRANSACTION_OBJECT *pobTran);
int inBATCH_Update_Sign_Status(TRANSACTION_OBJECT *pobTran);

int inADVICE_SaveAppend(TRANSACTION_OBJECT *pobTran, long lnInvNum);
int inADVICE_SaveTop(TRANSACTION_OBJECT *pobTran, long lnInvNum);
int inADVICE_DeleteRecordFlow(TRANSACTION_OBJECT *pobTran, long lnInvNum);
int inADVICE_SearchRecord_Index(TRANSACTION_OBJECT *pobTran, long lnInvNum, int *inADVIndex);
int inADVICE_DeleteRecored(TRANSACTION_OBJECT *pobTran, int inAdvIndex);
int inADVICE_DeleteAll(TRANSACTION_OBJECT *pobTran);
int inADVICE_Update(TRANSACTION_OBJECT *pobTran);
int inADVICE_GetTotalCount(TRANSACTION_OBJECT *pobTran);
int inADVICE_GetInvoiceNum(TRANSACTION_OBJECT *pobTran, char *szGetInvoiceNum);
int inADVICE_ESC_SaveAppend(TRANSACTION_OBJECT *pobTran, long lnInvNum);
int inADVICE_ESC_GetTotalCount(TRANSACTION_OBJECT *pobTran);
int inADVICE_ESC_DeleteRecordFlow(TRANSACTION_OBJECT *pobTran, long lnInvNum);
int inADVICE_ESC_SearchRecord_Index(TRANSACTION_OBJECT *pobTran, long lnInvNum, int *inADVIndex);
int inADVICE_ESC_DeleteRecored(TRANSACTION_OBJECT *pobTran, int inAdvIndex);

int inBATCH_FuncUpdateTxnRecord_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_CheckReport_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetTotalCountFromBakFile_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetTotalCountFromBakFile_By_Sqlite_ESVC(TRANSACTION_OBJECT *pobTran);
int inBATCH_ReviewReport_Detail_Flow_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_ReviewReport_Detail_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_ReviewReport_Detail_Txno_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_ReviewReport_Detail_NEWUI_Flow_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_ReviewReport_Detail_NEWUI_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_ReviewReport_Detail_Txno_NEWUI_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetDetailRecords_By_Sqlite(TRANSACTION_OBJECT *pobTran, int inStartCnt);
int inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(TRANSACTION_OBJECT *pobTran, int inStartCnt);
int inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_START(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_Read(TRANSACTION_OBJECT *pobTran, int inStartCnt);
int inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_END(TRANSACTION_OBJECT *pobTran);
int inBATCH_FuncUserChoice_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetTransRecord_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetTransRecord_By_Sqlite_ESVC(TRANSACTION_OBJECT *pobTran);
int inBATCH_GlobalBatchHandleReadOnly_By_Sqlite(void);
int inBATCH_GlobalBatchHandleClose_By_Sqlite(void);
int inBATCH_GlobalAdviceHandleReadOnly_By_Sqlite(void);
int inBATCH_GlobalAdviceHandleClose_By_Sqlite(void);
int inBATCH_GetAdviceDetailRecord_By_Sqlite(TRANSACTION_OBJECT *pobTran, int inADVCnt);
int inBATCH_GlobalAdvice_ESC_HandleReadOnly_By_Sqlite(void);
int inBATCH_GlobalAdvice_ESC_HandleClose_By_Sqlite(void);
int inBATCH_GetAdvice_ESC_DetailRecord_By_Sqlite(TRANSACTION_OBJECT *pobTran, int inADVCnt);
int inBATCH_ESC_Save_Advice_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_Update_Sign_Status_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_Update_MPAS_Reprint_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_Update_CLS_SettleBit_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_Update_ESC_Uploaded_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetTotalCount_BatchUpload_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_FuncUpdateTxnRecord_Ticket_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_Create_BatchTable_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inBATCH_Insert_All_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inBATCH_Get_ESC_Upload_Count_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inTxnType, unsigned char uszPaperBit, int *inTxnTotalCnt, long *lnTxnTotalAmt);
int inBATCH_GetAdvice_ESVC_DetailRecord_By_Sqlite(TRANSACTION_OBJECT *pobTran);

int inBATCH_Presssure_Test(void);
