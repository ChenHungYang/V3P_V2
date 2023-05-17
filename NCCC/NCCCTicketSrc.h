
#ifndef __NCCC_TICKET_SRC_H__
#define __NCCC_TICKET_SRC_H__

#define _TICKET_TYPE_NONE_		0
#define _TICKET_TYPE_IPASS_		1
#define _TICKET_TYPE_ECC_			2
#define _TICKET_TYPE_ICASH_		3
#define _TICKET_TYPE_HAPPYCASH_	4

int inNCCC_Ticket_Top_Up_Amount_Check(TRANSACTION_OBJECT *pobTran);
/* MNEUUI 使用 START */
/* [新增電票悠遊卡功能] 新增電票使用函式 [SAM] 2022/6/13 下午 2:55 */
int inNCCC_Ticket_Func_Check_Transaction_Deduct(int inCode);
int inNCCC_Ticket_Func_Check_Transaction_Refund(int inCode);
int inNCCC_Ticket_Func_Check_Transaction_Inquiry(int inCode);
int inNCCC_Ticket_Func_Check_Transaction_Top_Up(int inCode);
int inNCCC_Ticket_Func_Check_Transaction_Void_Top_Up(int inCode);
int inNCCC_Ticket_Logon_ShowResult(void);

int inNCCC_Ticket_CreateBatchTable_Ticket(TRANSACTION_OBJECT *pobTran, char* szTableName);
int inNCCC_Ticket_Insert_All_Batch(TRANSACTION_OBJECT *pobTran, char* szTableName);
int inNCCC_Ticket_Table_Link_TRec(TRANSACTION_OBJECT *pobTran, SQLITE_ALL_TABLE *srAll, int inLinkState);
int inNCCC_Ticket_Insert_Advice_Ticket_Record(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_ESVC_Get_TRec_Top_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inNCCC_Ticket_ESVC_Get_TRec_Top(TRANSACTION_OBJECT *pobTran, char* szTableName);
int inNCCC_Ticket_ESVC_Delete_TRec_Top_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inNCCC_Ticket_ESVC_Delete_TRec_Top(TRANSACTION_OBJECT *pobTran, char* szTableName);
int inNCCC_Ticket_ESVC_Get_Batch_ByInvNum(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber);
int inNCCC_Ticket_Get_TRec_ByCnt_Enormous_Search(TRANSACTION_OBJECT *pobTran, char* szTableName);
int inNCCC_Ticket_Get_TRec_ByCnt_Enormous_Get(TRANSACTION_OBJECT *pobTran, char* szTableName, int inRecCnt);
int inNCCC_Ticket_Get_Void_Top_Up_Amount_From_Batch(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Top_Up_Amount_Check(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Func_MakeRefNo(TRANSACTION_OBJECT *pobTran);


int inNCCC_Ticket_Get_Void_Top_Up_Amount_From_Batch(TRANSACTION_OBJECT *pobTran);
/* MNEUUI 使用 END */

#endif
