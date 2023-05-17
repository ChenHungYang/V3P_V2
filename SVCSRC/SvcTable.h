/* 
 * File:   SvcTable.h
 * Author: Sam chang
 *
 * Created on 2022年12月15日, 下午 2:29
 */

#ifndef	__SVCTABLE_H__
#define	__SVCTABLE_H__

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct
{
	char szSVCTmsOK[1+1];			/* [新增SVC功能]  參數下載是否完成版本[SAM] */
	char szSVCParameterVersion[6+1];		/* [新增SVC功能]  參數下載版本[SAM] */
	char szSVCParameterDateTime[14+1];	/* [新增SVC功能]  參數版本時間 [SAM] */
	char szSVCMaskData[20+1];			/* [新增SVC功能] 參考 chapter 3.4 訊息內容規範之亂碼化 [SAM] */
} SVC_TABLE_REC;

/* 以欄位數決定 ex:欄位4數個，理論上會有3個comma和兩個byte的0x0D 0X0A */
#define _SIZE_SVC_TABLE_COMMA_0D0A_	5
#define _SIZE_SVC_TABLE_REC_                  (sizeof(SVC_TABLE_REC))
#define _SVC_TABLE_FILE_NAME_                 "SVCTB.dat"
#define _SVC_TABLE_FILE_NAME_BAK_        "SVCTB.bak"	
	

int inLoadSvcTableRec(int inRecIndex);
int inSaveSvcTableRec(int inRecIndex);

int inGetSVCParameterVersion(char* szSVCParameterVersion);
int inSetSVCParameterVersion(char* szSVCParameterVersion);
int inGetSVCParameterDateTime(char* szSVCParameterDateTime);
int inSetSVCParameterDateTime(char* szSVCParameterDateTime);
int inGetSVCMaskData(char* szSVCParameterVersion);
int inSetSVCMaskData(char* szSVCParameterVersion);

int inSVC_TABLE_EditTable(void);

#ifdef	__cplusplus
}
#endif

#endif	/* SVCTABLE_H */

