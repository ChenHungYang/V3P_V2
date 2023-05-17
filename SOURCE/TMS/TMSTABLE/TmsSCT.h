
#ifndef _TMS_SCT_H_
#define _TMS_SCT_H_

#define _TMS_SCT_DL_MODE_SIZE_		1
#define _TMS_SCT_DL_TYPE_SIZE_			1
#define _TMS_SCT_DL_START_DATE_SIZE_	8
#define _TMS_SCT_DL_START_TIME_SIZE_	6


/*
 * 
 * [檢查參數檔] 此檔案有初始設定值，會由TMS下載檔案，下載後會直接使用TMS下載的檔案  2022/8/12 上午 10:07 [SAM] 
 * 
 */
typedef struct
{
	char szTMSInquireMode[_TMS_SCT_DL_MODE_SIZE_ + 1];			/* TMS下載詢問機制 */
	char szTMSDownLoadType[_TMS_SCT_DL_TYPE_SIZE_+ 1];			/* TMS下載類型 */
	char szTMSInquireStartDate[_TMS_SCT_DL_START_DATE_SIZE_ + 1];	/* TMS 安排時間自動訊問 排程起始日 */
	char szTMSInquireTime[_TMS_SCT_DL_START_TIME_SIZE_ + 1];		/* TMS 安排時間自動訊問 指定詢問時間 */
} TMSSCT_REC;


/* 以欄位數決定 ex:欄位數是4個，理論上會有3個comma和兩個byte的0x0D 0X0A */
#define _SIZE_TMSSCT_COMMA_0D0A_		5
#define _SIZE_TMSSCT_REC_			(sizeof(TMSSCT_REC))	/*一個record不含comma和0D0A的長度*/
#define _TMSSCT_FILE_NAME_			"TMSSCT.dat"		/*file name*/
#define _TMSSCT_FILE_NAME_BAK_		"TMSSCT.bak"		/*bak name*/

#define _TMS_INQUIRE_00_SETTLE_			"0"
#define _TMS_INQUIRE_01_SETTLE_POWERON_	"1"
#define _TMS_INQUIRE_02_SCHEDHULE_SETTLE_	"2"

#define _TMS_INQUIRE_AP_	"A"
#define _TMS_INQUIRE_PA_	"C"

#define _TMS_DLD_IMMEDIATELY_MODE_	"0"
#define _TMS_DLD_SCHEDULE_MODE_		"1"


/* Load & Save function */
int inLoadTMSSCTRec(int inTMSSCTRec);
int inSaveTMSSCTRec(int inTMSSCTRec);
int inTMSSCT_Edit_TMSSCT_Table(void);

/*Set function*/
int inSetTMSInquireMode(char* szTMSInquireMode);
int inSetTMSDownLoadType(char* szTMSDownLoadType);
int inSetTMSInquireStartDate(char* szTMSInquireStartDate);
int inSetTMSInquireTime(char* szTMSInquireTime);


/*Get function*/
int inGetTMSInquireMode(char* szTMSInquireMode);
int inGetTMSDownLoadType(char* szTMSDownLoadType);
int inGetTMSInquireStartDate(char* szTMSInquireStartDate);
int inGetTMSInquireTime(char* szTMSInquireTime);

#endif
