
/*
 * [檢查參數檔] 此檔案有初始設定，會由TMS下載檔案更新 2022/8/12 上午 10:07 [SAM]
 */

typedef struct
{
	char szCommIndex[2 + 1];                /* 通訊參數索引 */
	char szTPDU[10 + 1];                    /* 60nnnnnnnn */
	char szNII[3 + 1];                      /* 網路識別碼 */
	char szHostTelPrimary[15 + 1];          /* 第一授權撥接電話 */
	char szHostTelSecond[15 + 1];           /* 第二授權撥接電話 */
	char szReferralTel[15 + 1];             /* Call Bank 撥接電話 */
	char szHostIPPrimary[15 + 1];           /* 第一授權主機 IP Address  */
	char szHostPortNoPrimary[5 + 1];        /* 第一授權主機 Port No. */
	char szHostIPSecond[15 + 1];            /* 第二授權主機 IP Address */
	char szHostPortNoSecond[5 + 1];         /* 第二授權主機 Port No. */
	char szTCPHeadFormat[1 + 1];            /* TCP 電文長度之格式。( B=BCD，H=Binary)，預設值= H。 */
	char szCarrierTimeOut[2 + 1];           /* 連線等候時間。(超過等候時間，自動撥第二授權電話或IP Address) */
	char szHostResponseTimeOut[2 + 1];      /* 授權等候時間。(超過等候時間，自動斷線) */
	char szSettleTelPrimary[20 + 1];           /* 第一結帳撥接電話 */
	char szSettleTelSecond[20 + 1];      /* 第二結帳撥接電話*/	
}CPT_REC;


/* 以欄位數決定 ex:欄位數是15個，理論上會有14個comma和兩個byte的0x0D 0X0A */
#define _SIZE_CPT_COMMA_0D0A_	16
#define _SIZE_CPT_REC_			(sizeof(CPT_REC))
#define _CPT_FILE_NAME_			"CPT.dat"
#define _CPT_FILE_NAME_BAK_		"CPT.bak"


/* Load & Save function */
int inLoadCPTRec(int inCPTRec);
int inSaveCPTRec(int inCPTRec);
int inCPT_Edit_CPT_Table(void);

/* Set function */
int inSetCommIndex(char* szCommIndex);
int inSetTPDU(char* szTPDU);
int inSetNII(char* szNII);
int inSetHostTelPrimary(char* szHostTelPrimary);
int inSetHostTelSecond(char* szHostTelSecond);
int inSetReferralTel(char* szReferralTel);
int inSetHostIPPrimary(char* szHostIPPrimary);
int inSetHostPortNoPrimary(char* szHostPortNoPrimary);
int inSetHostIPSecond(char* szHostIPSecond);
int inSetHostPortNoSecond(char* szHostPortNoSecond);
int inSetTCPHeadFormat(char* szTCPHeadFormat);
int inSetCarrierTimeOut(char* szCarrierTimeOut);
int inSetHostResponseTimeOut(char* szHostResponseTimeOut);
int inSetSettleTelPrimary(char* szSettleTelPrimary);
int inSetSettleTelSecond(char* szSettleTelSecond);

/* Get function */
int inGetCommIndex(char* szCommIndex);
int inGetTPDU(char* szTPDU);
int inGetNII(char* szNII);
int inGetHostTelPrimary(char* szHostTelPrimary);
int inGetHostTelSecond(char* szHostTelSecond);
int inGetReferralTel(char* szReferralTel);
int inGetHostIPPrimary(char* szHostIPPrimary);
int inGetHostPortNoPrimary(char* szHostPortNoPrimary);
int inGetHostIPSecond(char* szHostIPSecond);
int inGetHostPortNoSecond(char* szHostPortNoSecond);
int inGetTCPHeadFormat(char* szTCPHeadFormat);
int inGetCarrierTimeOut(char* szCarrierTimeOut);
int inGetHostResponseTimeOut(char* szHostResponseTimeOut);
int inGetSettleTelPrimary(char* szSettleTelPrimary);
int inGetSettleTelSecond(char* szSettleTelSecond);

int inCPT_Test1(void);
