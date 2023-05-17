/* 
 * File:   Sqlite.h
 * Author: user
 *
 * Created on 2016年3月10日, 上午 10:07
 */
#define _TAG_WIDTH_				50	/* 目前最長的Tag Name為30 */
#define _TAG_TYPE_WIDTH_			20	/* 型別 */
#define _ADDITIONAL_ATTRIBUTE_1_	20	/* 屬性1 */
#define _ADDITIONAL_ATTRIBUTE_2_	20	/* 屬性2 */

#define _TAG_MAX_LENGRH_			50	/* Tag最長長度 */
#define _TAG_INT_MAX_NUM_		100
#define _TAG_INT64T_MAX_NUM_		100
#define _TAG_CHAR_MAX_NUM_		300	/* 截至2018/1/15 下午 3:54為止，用了115個，若需要在往上調 */
#define _TAG_TEXT_MAX_NUM_		300

/* Table 名稱 */
#define _BATCH_TABLE_SUFFIX_		""
#define _EMV_TABLE_SUFFIX_		"_EMV"
#define _BATCH_TABLE_ADVICE_SUFFIX_	"_ADV"
#define _BATCH_TABLE_ESC_AGAIN_SUFFIX_	"_AGAIN"	/* 重送區table */
#define _BATCH_TABLE_ESC_AGAIN_EMV_SUFFIX_ "_AGAIN_EMV"
#define _BATCH_TABLE_ESC_FAIL_SUFFIX_	"_FAIL"		/* 失敗區table */
#define _BATCH_TABLE_ESC_FAIL_EMV_SUFFIX_ "_FALE_EMV"


/* ESC不送F_55 */

/* Link State */
#define _LS_INSERT_	0
#define _LS_READ_	1
#define _LS_UPDATE_	2

/* inSqlite_Get_Batch_ByCnt_Enormous_Flow 使用 */
#define _BYCNT_ENORMMOUS_SEARCH_	0
#define _BYCNT_ENORMMOUS_READ_		1
#define _BYCNT_ENORMMOUS_FREE_		2

#define _SQLITE_DEFAULT_FLAGS_	SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE

typedef enum
{
	_TN_NULL_ = 0x00,
	_TN_BATCH_TABLE_,
	_TN_EMV_TABLE_,
	_TN_BATCH_TABLE_ESC_AGAIN_,
	_TN_BATCH_TABLE_ESC_FAIL_,
	_TN_BATCH_TABLE_TICKET_,
	_TN_BATCH_TABLE_TICKET_ADVICE_,
	_TN_BATCH_TABLE_ESC_AGAIN_EMV_,/* 新增ESC EMV記錄 20190919 [SAM] */
	_TN_BATCH_TABLE_ESC_FAIL_EMV_  /* 新增ESC EMV記錄 20190919 [SAM] */
} TABLE_NAME;

/* 儲存變數的Table */
typedef struct
{
	char	szTag[_TAG_WIDTH_];
	char	szType[_TAG_TYPE_WIDTH_];
	char	szAttribute1[_ADDITIONAL_ATTRIBUTE_1_];
	char	szAttribute2[_ADDITIONAL_ATTRIBUTE_2_];
}SQLITE_TAG_TABLE;

typedef struct
{
	unsigned char	uszIsFind;			/* 若讀取時已找到，設為1 */
	char		szTag[_TAG_MAX_LENGRH_];
	void		*pTagValue;
}SQLITE_INT32T_TABLE;

typedef struct
{
	unsigned char	uszIsFind;			/* 若讀取時已找到，設為1 */
	char		szTag[_TAG_MAX_LENGRH_];
	void		*pTagValue;
}SQLITE_INT64T_TABLE;

typedef struct
{
	unsigned char	uszIsFind;			/* 若讀取時已找到，設為1 */
	char		szTag[_TAG_MAX_LENGRH_];
	void		*pCharVariable;
	int		inTagValueLen;
}SQLITE_CHAR_TABLE;

typedef struct
{
	char	*Variable;
	int	(*inTagUpdate)(TRANSACTION_OBJECT *, unsigned char *); /* 將pobTran資料update到資料庫中 */
}SQLITE_UPDATE_TABLE;

typedef struct
{
	int			inIntNum;
	int			inInt64tNum;
	int			inCharNum;
	int			inTextNum;
	SQLITE_INT32T_TABLE	srInt[_TAG_INT_MAX_NUM_];
	SQLITE_INT64T_TABLE	srInt64t[_TAG_INT64T_MAX_NUM_];
	SQLITE_CHAR_TABLE	srChar[_TAG_CHAR_MAX_NUM_];
	SQLITE_CHAR_TABLE	srText[_TAG_TEXT_MAX_NUM_];	/* 用BLOB的API塞進去的資料無法用字串比對，所以再生一個Table，EMV參數則維持使用BLOB塞*/
}SQLITE_ALL_TABLE;

/* 避免大量查詢的方法 */
typedef struct
{
	int	inRows;
	int	inCols;
	char**	szResult;
}SQLITE_RESULT;

#define	_DATA_BASE_NAME_NEXSYS_			"./fs_data/nexsys.db"
//#define	_DATA_BASE_NAME_NEXSYS_                                 "nexsys.db" /* 讓程式build過用 add by sampo Ticket要用 */
#define	_DATA_BASE_NAME_NEXSYS_TRANSACTION_	"nexsys.db"


/* Initial*/
int inSqlite_Initial(void);
int inSqlite_Initial_Setting(void);

/* Create */
int inSqlite_Create_BatchTable_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inSqlite_Create_Table(char* szDBName, char* szTableName, SQLITE_TAG_TABLE* pobSQLTag);
int inSqlite_Create_BatchTable(TRANSACTION_OBJECT *pobTran, char* szTableName);
int inSqlite_Create_BatchTable_EMV(TRANSACTION_OBJECT *pobTran, char* szTableName, char* szTableName2);
int inSqlite_Drop_Table_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inSqlite_Drop_Table(char* szDBName, char* szTableName);


/* Delete */
int inSqlite_Delete_Table_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inSqlite_Delete_Table(TRANSACTION_OBJECT *pobTran, char* szTableName);

/* Link */
int inSqlite_Table_Link_BRec(TRANSACTION_OBJECT *pobTran, SQLITE_ALL_TABLE *srAll, int inLinkState);
int inSqlite_Table_Link_EMVRec(TRANSACTION_OBJECT* pobTran, SQLITE_ALL_TABLE* srAll, int inLinkState);
int inSqlite_Insert_All_Batch(TRANSACTION_OBJECT *pobTran, char* szTableName);
int inSqlite_Insert_All_EMV(TRANSACTION_OBJECT *pobTran, char* szTableName);

/* Get Tag Value */
int inSqlite_Get_TagValue_ByInvoiceNumber_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inInvoiceNumber, char *szTagName, char *szTagValue);
int inSqlite_Get_TagValue_ByInvoiceNumber(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber, char *szTagName, char *szTagValue);

/* Table Count */
int inSqlite_Get_Table_Count_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int *inTableCount);
int inSqlite_Get_Table_Count(TRANSACTION_OBJECT *pobTran, char *szTableName1, int *inTableCount);

/* Get Batch */
int inBATCH_Get_Batch_ByInvNum_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inInvoiceNumber);
int inSqlite_Get_Batch_ByInvNum(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber);
int inSqlite_Get_EMV_ByInvNum(TRANSACTION_OBJECT *pobTran, char* szTableName);


int inSqlite_Update_ByInvoiceNumber_All(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber);

/* 取最早insert的紀錄 */
int inSqlite_ESC_Get_BRec_Top_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inSqlite_ESC_Get_EMVRec_Top_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inSqlite_ESC_Get_BRec_Top(TRANSACTION_OBJECT *pobTran, char* szTableName);
int inSqlite_ESC_Get_EMVRec_Top(TRANSACTION_OBJECT *pobTran, char* szTableName);

/* 刪除最早insert的紀錄 */
int inSqlite_ESC_Delete_Record_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inSqlite_ESC_Delete_Record(TRANSACTION_OBJECT *pobTran, char* szTableName);

/* 取最大的Primary Key值 */
int inSqlite_Get_Max_TableID_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, char *szTagValue);
//int inSqlite_Get_Max_TableID(char* szDBPath, char* szTableName, char *szTagValue);
int inSqlite_Get_Max_TableID(TRANSACTION_OBJECT *pobTran, char* szTableName, char *szTagValue);

/* 更改原先紀錄的狀態 */
int inSqlite_Update_ByInvNum_TranState_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inInvoiceNumber);
int inSqlite_Update_ByInvNum_TranState(TRANSACTION_OBJECT *pobTran, char* szTableType, int inInvoiceNumber);
int inSqlite_Update_ByInvNum_TranState_Ticket(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber);
int inSqlite_Update_ByInvNum_CLS_SettleBit_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inInvoiceNumber);
int inSqlite_Update_ByInvNum_CLS_SettleBit(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber);
int inSqlite_Update_ByInvNum_Trans_Rrn_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inInvoiceNumber);
int inSqlite_Update_ByInvNum_Trans_Rrn(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber);

int inSqlite_Get_Batch_ByCnt_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inRecCnt);
int inSqlite_Get_Batch_ByCnt(TRANSACTION_OBJECT *pobTran, char* szTableType, int inRecCnt);
int inSqlite_Get_EMV_ByCnt(TRANSACTION_OBJECT *pobTran, char* szTableName);
int inSqlite_Get_Batch_Count_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int *inTableCount);
int inSqlite_Get_Batch_Count(TRANSACTION_OBJECT *pobTran, char *szTableName, int *inTableCount);
int inSqlite_Get_Batch_Upload_Count_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int *inTableCount);
int inSqlite_Get_Batch_Upload_Count(TRANSACTION_OBJECT *pobTran, char *szTableName, int *inTableCount);

/* Get大量資料用 */
int inBATCH_Get_Batch_ByCnt_Enormous_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inRecCnt, int inState);
int inSqlite_Get_Batch_ByCnt_Enormous_Search(TRANSACTION_OBJECT *pobTran, char* szTableName);
int inSqlite_Get_Batch_ByCnt_Enormous_Get(TRANSACTION_OBJECT *pobTran, char* szTableName, int inRecCnt);
int inSqlite_Get_EMV_ByCnt_Enormous_Get(TRANSACTION_OBJECT *pobTran, char* szTableName, int inRecCnt);
int inSqlite_Get_Batch_ByCnt_Enormous_Free(void);

/* 確認表是否存在 */
int inSqlite_Check_Table_Exist_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inSqlite_Check_Table_Exist(TRANSACTION_OBJECT *pobTran, char *szTableName);

/* Vacumm 指令 */
int inSqlite_Vacuum_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inSqlite_Vacuum(TRANSACTION_OBJECT *pobTran, char *szTableName1);

int inSqlite_Check_Escape_Cnt(char* szString);
int inSqlite_Process_Escape(char* szOldString, char* szNewString);

/* Test Function */
int inSqlite_Batch_To_DB_Test(void);
int inSqlite_Get_Table_Count_Test(void);
int inSqlite_Vacuum_Flow_Test(void);

/* 算筆數使用 */
int inSqlite_Get_ESC_Sale_Upload_Count(TRANSACTION_OBJECT *pobTran, char *szTableName, int inTxnType, unsigned char uszPaperBit, int *inTxnTotalCnt, long *lnTxnTotalAmt);

int inSqlite_ShowDbHandle(void);

int inSqlite_Update_ByRecordID_All(char* szDBName, char* szTableName, int inRecordID, SQLITE_ALL_TABLE* srAll);
int inSqlite_Get_Table_ByRecordID_All(char* szDBName, char* szTableName, int inRecordID, SQLITE_ALL_TABLE* srAll);
int inSqlite_Insert_Or_Replace_ByRecordID_All(char* szDBName, char* szTableName, int inRecordID , SQLITE_ALL_TABLE* srAll);
int inSqlite_Get_Data_By_External_SQL(char* szDBName, char* szTableName, SQLITE_ALL_TABLE* srAll, char* szSQLStatement);
int inSqlite_Get_Binding_Value(sqlite3_stmt** srSQLStat, SQLITE_ALL_TABLE* srAll);
int inSqlite_Calculate_SQLLength(char* szSqlPrefix, char* szSqlSuffix, char* szSqlSuffix2, SQLITE_ALL_TABLE* srAll, int* inSqlLength);
int inSqlite_DB_Open_Or_Create(char* szDBPath, sqlite3** srDBConnection, int inFlags, char* szVfs);
int inSqlite_Copy_Table_Data(char* szDBPath, char* szOldTableName, char* szNewTableName);
int inSqlite_Rename_Table(char* szDBPath, char* szOldTableName, char* szNewTableName);
int inSqlite_Table_Relist_SQLite(char *szDBPath, char *szTableName, int inOldIndex, int inNewIndex);
int inSqlite_Delete_Table_Data_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inSqlite_Delete_Table_Data(char* szDBPath, char* szTableName);
int inSqlite_Update_ByInvNum_SignState(TRANSACTION_OBJECT * pobTran, char* szDBPath, char* szTableName, int inInvoiceNumber);
int inSqlite_DB_Close(sqlite3** srDBConnection);
int inSqlite_SQL_Finalize(sqlite3_stmt **srSQLStat);




