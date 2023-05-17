
/*
 * [檢查參數檔] 此檔案有初始設定值，目前檔案內容不會刪除，
 * 會在TMS下載後根據 HDT 檔案內容挑出需更新的欄位進行更新 2022/8/12 上午 10:07 [SAM]
 */
typedef struct
{
#ifdef _MODIFY_HDPT_FUNC_	
	int	inRecordRowID;			/* SQLite使用，用於暫存Table的Rowid */
#endif
	char	szInvoiceNum [6 + 1];		/* Invoice Number */
	char	szBatchNum [6 + 1];		/* Batch Number */
	char	szSTANNum[6 + 1];		/* STAN Number */
	char	szReversalCnt[6 + 1];		/* Reversal Count */
	char	szTRTFileName[12 + 1];	/* szTRTFileName */
	char	szMustSettleBit[2 + 1];	/* 是否結帳的flag，預設值為N */
	char	szSendReversalBit[2 + 1];	/* 預設值為N */
	char	szCLS_SettleBit[2 + 1];		/* 看是否要續傳批次 */
	char	szTicket_InvNum[6 + 1];	/* 紀錄電票的invoiceNum，簽單的調閱編號用szInvoiceNum的 */
	char	szBatchNumLimit[4 + 1];	/* 單筆Batch的最大數量 */
	char	szMustISRUploadEnableBit[1 + 1];
} HDPT_REC;


/* 以欄位數決定 ex:欄位數是11個，理論上會有10個comma和 2 個byte的0x0D 0X0A */
#define _SIZE_HDPT_COMMA_0D0A_	12
#define _SIZE_HDPT_REC_			(sizeof(HDPT_REC))
#define _HDPT_FILE_NAME_			"HDPT.dat"
#define _HDPT_FILE_NAME_BAK_		"HDPT.bak"
#define _HDPT_TABLE_NAME_		"HDPT"
#define _HDPT_TABLE_NAME_BAK_	"HDPT_BAK"

/* Load & Save function */
int inLoadHDPTRec_SQLite(int inHDPTRec);
int inLoadHDPTRec(int inHDPTRec);
int inLoadHDPTRec_CTOS(int inHDPTRec);
int inLoadHDPTRec_SQLite(int inHDPTRec);

int inSaveHDPTRec(int inHDPTRec);
int inSaveHDPTRec_Linux(int inHDPTRec);
int inSaveHDPTRec_CTOS(int inHDPTRec);
int inSaveHDPTRec_SQLite(int inHDPTRec);
int inHDPT_Edit_HDPT_Table(void);

int inHDPT_Table_Link_HDPTRec(SQLITE_ALL_TABLE *srAll, int inLinkStates);
int inHDPT_Initial_AllRercord(char* szDBName, char* szTableName);

/* Set function */
int inSetInvoiceNum(char* szInvoiceNum);
int inSetBatchNum(char* szBatchNum);
int inSetSTANNum(char* szSTANNum);
int inSetReversalCnt(char* szReversalCnt);
int inSetTRTFileName(char* szTRTFileName);
int inSetMustSettleBit(char* szMustSettleBit);
int inSetSendReversalBit(char* szSendReversalBit);
int inSetCLS_SettleBit(char* szCLS_SettleBit);
int inSetTicket_InvNum(char* szTicket_InvNum);
int inSetBatchNumLimit(char* szBatchNumLimit);
int inSetMustISRUploadEnableBit(char* szMustISRUploadEnableBit);

/* Get function */
int inGetInvoiceNum(char* szInvoiceNum);
int inGetBatchNum(char* szBatchNum);
int inGetSTANNum(char* szSTANNum);
int inGetReversalCnt(char* szReversalCnt);
int inGetTRTFileName(char* szTRTFileName);
int inGetMustSettleBit(char* szMustSettleBit);
int inGetSendReversalBit(char* szSendReversalBit);
int inGetCLS_SettleBit(char* szCLS_SettleBit);
int inGetTicket_InvNum(char* szTicket_InvNum);
int inGetBatchNumLimit(char* szBatchNumLimit);
int inGetMustISRUploadEnableBit(char* szMustISRUploadEnableBit);

int inHDPT_CheckCopyHDPT(void);
int inHDPT_Test1(void);
int inHDPT_Test2(void);
