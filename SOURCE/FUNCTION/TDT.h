/* 
 * File:   TDT.h
 * Author: user
 *
 * Created on 2017年12月19日, 下午 3:46
 */


/*
 * 
 * [檢查參數檔] 此檔案有初始設定值，目前固定四組，內容會在電票初始化時更動  2022/8/12 上午 10:07 [SAM] 
 *
 */

typedef struct
{
	char	szTicket_HostIndex[2 + 1];
	char	szTicket_HostName[12 + 1];
	char	szTicket_HostTransFunc[20 + 1];
	char	szTicket_HostEnable[2 + 1];
	char	szTicket_LogOnOK[2 + 1];
	char	szTicket_SAM_Slot[2 + 1];
	char	szTicket_ReaderID[4 + 1];		/* For IPASS */
	char	szTicket_STAN[6 + 1];			/* FOR ECC */
	char	szTicket_LastTransDate[8 + 1];	/* FOR ECC 上次交易日期 YYYYMMDD */
	char	szTicket_LastRRN[15 + 1];		/* FOR ECC 上次交易RRN */
	char	szTicket_Device1[20 + 1];		/* FOR ECC 一代設備編號 */
	char	szTicket_Device2[20 + 1];		/* FOR ECC 二代設備編號 */
	char	szTicket_Batch[8 + 1];			/* FOR ECC 批次號碼之規則為yymmddxx(年月日(6) + 流水號(2)) */
	char	szTicket_NeedNewBatch[2 + 1];	/* FOR ECC 是否要更新批號(Y/N) */
	char	szTicket_Device3[20 + 1];		/* FOR ECC 悠遊卡Dongle Device ID */
} TDT_REC;


/* 以欄位數決定 ex:欄位數是15個，理論上會有14個comma和兩個byte的0x0D 0X0A */
#define _SIZE_TDT_COMMA_0D0A_		16
#define _SIZE_TDT_REC_			(sizeof(TDT_REC))			/* 一個record不含comma和0D0A的長度 */
#define _TDT_FILE_NAME_			"TDT.dat"				/* File name */
#define _TDT_FILE_NAME_BAK_		"TDT.bak"				/* Bak name */

#define _TDT_INDEX_00_IPASS_		0
#define _TDT_INDEX_01_ECC_		1
#define _TDT_INDEX_02_ICASH_		2
#define _TDT_INDEX_03_HAPPYCASH_	3

/* Load & Save function */
int inLoadTDTRec(int inTDTRec);
int inSaveTDTRec(int inTDTRec);
int inTDT_Edit_TDT_Table(void);

/* Get function */
int inGetTicket_HostIndex(char* szTicket_HostIndex);
int inGetTicket_HostName(char* szTicket_HostName);
int inGetTicket_HostTransFunc(char* szTicket_HostTransFunc);
int inGetTicket_HostEnable(char* szTicket_HostEnable);
int inGetTicket_LogOnOK(char* szTicket_LogOnOK);
int inGetTicket_SAM_Slot(char* szTicket_SAM_Slot);
int inGetTicket_ReaderID(char* szTicket_ReaderID);
int inGetTicket_STAN(char* szTicket_STAN);
int inGetTicket_LastTransDate(char* szTicket_LastTransDate);
int inGetTicket_LastRRN(char* szTicket_LastRRN);
int inGetTicket_Device1(char* szTicket_Device1);
int inGetTicket_Device2(char* szTicket_Device2);
int inGetTicket_Batch(char* szTicket_Batch);
int inGetTicket_NeedNewBatch(char* szTicket_NeedNewBatch);
int inGetTicket_Device3(char* szTicket_Device3);

/* Set function */
int inSetTicket_HostIndex(char* szTicket_HostIndex);
int inSetTicket_HostName(char* szTicket_HostName);
int inSetTicket_HostTransFunc(char* szTicket_HostTransFunc);
int inSetTicket_HostEnable(char* szTicket_HostEnable);
int inSetTicket_LogOnOK(char* szTicket_LogOnOK);
int inSetTicket_SAM_Slot(char* szTicket_SAM_Slot);
int inSetTicket_ReaderID(char* szTicket_ReaderID);
int inSetTicket_STAN(char* szTicket_STAN);
int inSetTicket_LastTransDate(char* szTicket_LastTransDate);
int inSetTicket_LastRRN(char* szTicket_LastRRN);
int inSetTicket_Device1(char* szTicket_Device1);
int inSetTicket_Device2(char* szTicket_Device2);
int inSetTicket_Batch(char* szTicket_Batch);
int inSetTicket_NeedNewBatch(char* szTicket_NeedNewBatch);
int inSetTicket_Device3(char* szTicket_Device3);