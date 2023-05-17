
/*
 * [檢查參數檔] 此檔案有初始設定，下載後會直接使用TMS下載的檔案 2022/8/12 上午 10:07 [SAM]
 * 注意CDT.dat 的資料結構，資料總數可以比定義的個數多，但不能變少。
 * 
 */

typedef struct
{
	char szCardIndex[2 + 1];                /* 卡別索引 */
	char szLowBinRange[11 + 1];             /* 低卡號範圍 */
	char szHighBinRange[11 + 1];            /* 高卡號範圍 */
	char szHostCDTIndex[2 + 1];             /* 對應交易主機索引 */
	char szMinPANLength[2 + 1];             /* 檢查最短卡號長度 */
	char szMaxPANLength[2 + 1];             /* 檢查最長卡號長度 */
	char szModule10Check[1 + 1];            /* 檢查碼查核(U CARD以11碼卡號，依U CARD檢查碼邏輯進行查核)*/
	char szExpiredDateCheck[1 + 1];         /* 有效期查核 */
	char sz4DBCEnable[1 + 1];		/* 輸入AMEX 4DBC或MASTER/VISA CVV2。 */
	char szCardLabel[20 + 1];               /* 卡別名稱(VISA,MASTERCARD,JCB,U CARD,AMEX,DISCOVER,SMARTPAY) */
#ifdef _TX_CHECK_NO_
	char szPrint_Tx_No_Check_No[2 + 1];	/* Y = 該卡別交易簽單，列印交易編號、列印檢查碼及遮掩商店存根聯之卡號 N = 該卡別交易簽單，不列印交易編號、列印檢查碼及不遮掩商店存根聯之卡號 */
#endif
} CDT_REC;

#ifdef _TX_CHECK_NO_	
/* 以欄位數決定 ex:欄位數是11個，理論上會有10個comma和兩個byte的0x0D 0X0A */
#define _SIZE_CDT_COMMA_0D0A_                   12
#else
	/* 以欄位數決定 ex:欄位數是10個，理論上會有9個comma和兩個byte的0x0D 0X0A */
#define _SIZE_CDT_COMMA_0D0A_                   11
#endif
#define _SIZE_CDT_REC_				(sizeof(CDT_REC))			/* 一個record不含comma和0D0A的長度 */
#define _CDT_FILE_NAME_				"CDT.dat"				/* File name */
#define _CDT_FILE_NAME_BAK_			"CDT.bak"				/* Bak name */


/* Load & Save function */
int inLoadCDTRec(int inCDTRec);
int inSaveCDTRec(int inCDTRec);
int inCDT_Edit_CDT_Table(void);

/*Set function*/
int inSetCardIndex(char* szCardIndex);
int inSetLowBinRange(char* szLowBinRange);
int inSetHighBinRange(char* szHighBinRange);
int inSetHostCDTIndex(char* szHostIndex);
int inSetMinPANLength(char* szMinPANLength);
int inSetMaxPANLength(char* szMaxPANLength);
int inSetModule10Check(char* szModule10Check);
int inSetExpiredDateCheck(char* szExpiredDateCheck);
int inSet4DBCEnable(char* sz4DBCEnable);
int inSetCardLabel(char* szCardLabel);
#ifdef _TX_CHECK_NO_
int inSetPrint_Tx_No_Check_No(char* szPrint_Tx_No_Check_No);
#endif

/*Get function*/
int inGetCardIndex(char* szCardIndex);
int inGetLowBinRange(char* szLowBinRange);
int inGetHighBinRange(char* szHighBinRange);
int inGetHostCDTIndex(char* szHostIndex);
int inGetMinPANLength(char* szMinPANLength);
int inGetMaxPANLength(char* szMaxPANLength);
int inGetModule10Check(char* szModule10Check);
int inGetExpiredDateCheck(char* szExpiredDateCheck);
int inGet4DBCEnable(char* sz4DBCEnable);
int inGetCardLabel(char* szCardLabel);
#ifdef _TX_CHECK_NO_
int inGetPrint_Tx_No_Check_No(char* szPrint_Tx_No_Check_No);
#endif

int inCDT_Test (void);
int inSetCdtRealDataCount( int inRealCount);
