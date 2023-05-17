#ifndef _NEXSYS_ESC_TABLE_FUNC_
#define _NEXSYS_ESC_TABLE_FUNC_


/*
 * 
 * [檢查參數檔] 此檔案有初始設定值，但目前此檔沒在用，但 ESC的功能有在使用，不能整個砍掉  2022/8/12 上午 10:07 [SAM] 
 * TODO: 最後看要不要砍掉 NEX_ESCT_REC 結構
 */

int (*inDebugFunction)(char *, ...);

typedef struct 
{
	char szESCHostIndex[2+1];		/* 2 */
	char szESCULPort[5+1];		/* 2 */
	char szESCULIp[15+1];		/* 15 */
	char szESCIdleTime[12+1];		/* 12 */
	char szESCMode[3+1];		/* 3 */
	char szESCUploadMode[2+1];	/* 2 */
	char szESCReceiptUploadUpLimit[2+1];	/* 2 */
	char szESCUploadCnt[2+1];				/* 2 */
	char szESCUploadFailCnt[2+1];			/* 2 */
	char szESCFailHostIndex[2+1];			/* 2 */
	char szESCSettleTimeoutCnt[2+1];		/* 2 */
	char szESCDataHead[1+1];				/* 1 */
}NEX_ESCT_REC;



/* 以欄位數決定 ex:欄位數是 12個，理論上會有11個comma和兩個byte的0x0D 0X0A */
//#define _SIZE_HDT_COMMA_0D0A_           13
#define _SIZE_NEX_ESC_REC_                  (sizeof(NEX_ESCT_REC))
#define _NEX_ESC_FILE_NAME_                     "NESC.dat"
#define _NEX_ESC_FILE_NAME_BAK_             "NESC.bak"

#define _ESC_HOST_IDEX_	101
#define _ESC_UL_PORT_	102
#define _ESC_UL_IP_		103
#define _ESC_IDLE_TIME_	104
#define _ESC_MODE_		105
#define _ESC_UPLOAD_MODE_		106
#define _ESC_RECPIT_UPLOAD_LIMIT_	107
#define _ESC_UPLOAD_COUNT_		108
#define _ESC_UPLOAD_FAIL_COUNT_	109
#define _ESC_FAIL_HOST_INDEX_		110
#define _ESC_SETTLE_TIMEOUT_COUNT_	111
#define _ESC_DATA_HEAD_			112

int inESC_SetMachineTypeFunc(int inSetType);
int inESC_GetMachineTypeFunc(void);

int inESC_DisplayData(char * szTemp, int inLen);
int inESC_DebugLog(char *szStr,...);
int inParsingData(unsigned char * szSourceData, char* szDestData, int inTotalDataLen, int* inSourecLen, int *inDestLen);
int inLoadNexEscDataRec(int inESCRec);
void *pvdGetESCTRec(void);

int inGetESCRecord(char * szTempData, int inMode);
int inSetESCRecord(char * szTempData, int inMode);

int inGetPrintEscCountData(TRANSACTION_OBJECT *pobTran);

#endif
