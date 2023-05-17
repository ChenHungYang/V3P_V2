#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <unistd.h>
#include <sqlite3.h>

#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"

#include "../FUNCTION/Function.h"
#include "../FUNCTION/FuncTable.h"
#include "../FUNCTION/Sqlite.h"
#include "../FUNCTION/Signpad.h"
#include "../FUNCTION/Accum.h"

#include "../FUNCTION/FILE_FUNC/File.h"
#include "../FUNCTION/FILE_FUNC/FIleLogFunc.h"

#include "../FUNCTION/Batch.h"
#include "../FUNCTION/KMS.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/HDPT.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/CPT.h"

#include "../FUNCTION/UNIT_FUNC/TimeUint.h"

#include "../DISPLAY/Display.h"

#include "../COMM/Comm.h"

#include "../EVENT/MenuMsg.h"
#include "../EVENT/Flow.h"

#include "../PRINT/Print.h"
/* 最好放在最下面，因為裏面有用到別的 .H 資料 */
#include "../FUNCTION/AccountFunction/PrintBillInfo.h"

#include "../KEY/ProcessTmk.h"
#include "../KEY/deslib.h"

#include "NexsysEsc.h"
#include "NexsysEscIso.h"
#include "NexEscTableFn.h"
#include "NexEscTablePack.h"



int	ginNEX_ESCHostIndex = -1;		/* ESC HOST INDEX, 抓取ESC HOST 位置 */
unsigned char	guszNEX_tSAMKeyIndex_ESC = 0x00;	/* TSAM 用，目前不使用*/
unsigned char	guszNEX_tSAMCheckSum_ESC57_E1[4 + 1]; /* TSAM 用，目前不使用*/
unsigned char	guszNEX_tSAMCheckSum_ESC57_E2[4 + 1]; /* TSAM 用，目前不使用*/

extern  int     ginDebug;  			/* Debug使用 extern */

/* ESC上傳使用 START */
int	ginNEX_EscDataIndex;		/* 總上傳次數, 只會在 Online上傳時初始 */
int	ginNEX_EscUploadIndex;	/* 上傳中使用，計算目前上傳是第幾個封包，會再傳送前初始為0 */

/* ginNEX_ESC_Idle_flag 目前沒使用到，需要在Idle 上傳時再使用 20190328 [SAM] */
//int	ginNEX_ESC_Idle_flag = _ESC_IDLE_NO_NEED_UPLOAD_;	/* idle畫面是否需要上傳電子簽單 */

int	ginNEX_ESC_Connect_flag = 0;	/* idle畫面是否與ESC主機連線 一直重連會耗時*/
int	ginNEX_ESC_Send_Cnt;		/* 電子簽單上傳計數 超過三次不上傳 */

char	gszNEX_ESC_Date[8 + 1] = "0000000";			/* yyyymmdd */
char	gszNEX_ESC_Time[6 + 1] = "0000000";			/* hhmmss */
/* ESC上傳使用 END */

extern NEX_ESC_DATA	 NEX_ESC_UPLOAD_DATA[_ESC_LIMIT_];	/* 總上傳資料 */
extern unsigned char	guszNEX_FiscBit;		/* Fisc要用ascii壓 */


ISO_FIELD_NEXSYS_ESC_TABLE srNEXSYS_ESC_ISOFieldPack[] =
{
        {2,             inNEXSYS_ESC_Pack02},
        {3,             inNEXSYS_ESC_Pack03},
        {4,             inNEXSYS_ESC_Pack04},
        {11,            inNEXSYS_ESC_Pack11},
        {12,            inNEXSYS_ESC_Pack12},
        {13,            inNEXSYS_ESC_Pack13},
        {14,            inNEXSYS_ESC_Pack14},
        {22,            inNEXSYS_ESC_Pack22},
        {24,            inNEXSYS_ESC_Pack24},
        {25,            inNEXSYS_ESC_Pack25},
        {35,            inNEXSYS_ESC_Pack35},
        {37,            inNEXSYS_ESC_Pack37},
        {38,            inNEXSYS_ESC_Pack38},
        {41,            inNEXSYS_ESC_Pack41},
        {42,            inNEXSYS_ESC_Pack42},
        {48,            inNEXSYS_ESC_Pack48},
        {54,            inNEXSYS_ESC_Pack54},		
        {59,            inNEXSYS_ESC_Pack59},
        {60,            inNEXSYS_ESC_Pack60},
        {62,            inNEXSYS_ESC_Pack62},
        {63,            inNEXSYS_ESC_Pack63},
        {0,             NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_NEXSYS_ESC_TABLE srNEXSYS_ESC_ISOFieldPack_TSAM[] =
{
        {2,             inNEXSYS_ESC_Pack02},
        {3,             inNEXSYS_ESC_Pack03},
        {4,             inNEXSYS_ESC_Pack04},
        {11,            inNEXSYS_ESC_Pack11},
        {12,            inNEXSYS_ESC_Pack12},
        {13,            inNEXSYS_ESC_Pack13},
        {14,            inNEXSYS_ESC_Pack14},
        {22,            inNEXSYS_ESC_Pack22},
        {24,            inNEXSYS_ESC_Pack24},
        {25,            inNEXSYS_ESC_Pack25},
        {35,            inNEXSYS_ESC_Pack35_tSAM},
        {37,            inNEXSYS_ESC_Pack37},
        {38,            inNEXSYS_ESC_Pack38},
        {41,            inNEXSYS_ESC_Pack41},
        {42,            inNEXSYS_ESC_Pack42},
        {48,            inNEXSYS_ESC_Pack48},
        {54,            inNEXSYS_ESC_Pack54},
        {57,            inNEXSYS_ESC_Pack57_tSAM},
        {59,            inNEXSYS_ESC_Pack59},
        {60,            inNEXSYS_ESC_Pack60},
        {62,            inNEXSYS_ESC_Pack62},
        {63,            inNEXSYS_ESC_Pack63},
        {0,             NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_NEXSYS_ESC_TABLE srNEXSYS_ESC_ISOFieldUnPack[] =
{
        {24,            inNEXSYS_ESC_UnPack24},
        {38,            inNEXSYS_ESC_UnPack38},
        {39,            inNEXSYS_ESC_UnPack39},
        {0,             NULL}, /* 最後一組一定要放【0】!! */
};

ISO_CHECK_NEXSYS_ESC_TABLE srNEXSYS_ESC_ISOFieldCheck[] =
{
        {3,             inNEXSYS_ESC_Check03},
        {4,             inNEXSYS_ESC_Check04},
        {41,            inNEXSYS_ESC_Check41},
        {0,		NULL}, /* 最後一組一定要放【0】!! */
};

ISO_FIELD_TYPE_NEXSYS_ESC_TABLE srNEXSYS_ESC_ISOFieldType[] =
{
        {2,             _NEXSYS_ESC_ISO_NIBBLE_2_,         VS_FALSE,       0},
        {3,             _NEXSYS_ESC_ISO_BCD_,         VS_FALSE,       6},
        {4,             _NEXSYS_ESC_ISO_BCD_,         VS_FALSE,       12},
        {11,            _NEXSYS_ESC_ISO_BCD_,         VS_FALSE,       6},
        {12,            _NEXSYS_ESC_ISO_BCD_,         VS_FALSE,       6},
        {13,            _NEXSYS_ESC_ISO_BCD_,         VS_FALSE,       4},
        {14,            _NEXSYS_ESC_ISO_BCD_,         VS_FALSE,       4},
        {22,            _NEXSYS_ESC_ISO_BCD_,         VS_FALSE,       4},
        {24,            _NEXSYS_ESC_ISO_BCD_,         VS_FALSE,       4},
        {25,            _NEXSYS_ESC_ISO_BCD_,         VS_FALSE,       2},
        {35,            _NEXSYS_ESC_ISO_NIBBLE_2_,    VS_FALSE,       0},
        {37,            _NEXSYS_ESC_ISO_ASC_,         VS_FALSE,       12},
        {38,            _NEXSYS_ESC_ISO_ASC_,         VS_FALSE,       6},
        {39,            _NEXSYS_ESC_ISO_ASC_,         VS_FALSE,       2},
        {41,            _NEXSYS_ESC_ISO_ASC_,         VS_FALSE,       8},
        {42,            _NEXSYS_ESC_ISO_ASC_,         VS_FALSE,       15},
        {48,            _NEXSYS_ESC_ISO_BYTE_3_,      VS_TRUE,        0},
        {52,            _NEXSYS_ESC_ISO_BCD_,         VS_FALSE,       16},
        {54,            _NEXSYS_ESC_ISO_BYTE_3_,      VS_TRUE,        0},
        {55,            _NEXSYS_ESC_ISO_BYTE_3_,      VS_FALSE,       0},
        {57,            _NEXSYS_ESC_ISO_BYTE_3_,      VS_FALSE,       0},
        {58,            _NEXSYS_ESC_ISO_BYTE_3_,      VS_TRUE,        0},
        {59,            _NEXSYS_ESC_ISO_BYTE_3_,      VS_TRUE,        0},
        {60,            _NEXSYS_ESC_ISO_BYTE_3_,      VS_TRUE,        0},
        {62,            _NEXSYS_ESC_ISO_BYTE_3_,      VS_TRUE,        0},
        {63,            _NEXSYS_ESC_ISO_BYTE_3_,      VS_TRUE,        0},
        {64,            _NEXSYS_ESC_ISO_BCD_,         VS_FALSE,       16},
        {0,             _NEXSYS_ESC_ISO_BCD_,         VS_FALSE,       0},  /* 最後一組一定要放 0 */
};


/* ESC-UPLOAD */
int inNEXSYS_ESC_UPLOAD[] = {2, 3, 4, 11, 12, 13, 14, 24, 37, 38, 41, 42, 59, 60, 62, 0}; /* 最後一組一定要放 0!! */

BIT_MAP_NEXSYS_ESC_TABLE srNEXSYS_ESC_ISOBitMap[] =
{
	{_ESC_UPLOAD_,		inNEXSYS_ESC_UPLOAD,	"0320",		"610000"}, /* 電簽上傳 */
	{_NEXSYS_ESC_NULL_TX_,	NULL,			"0000",		"000000"}, /* 最後一組一定要放 TMS_NULL_TX!! */
};



ISO_TYPE_NEXSYS_ESC_TABLE srNEXSYS_ESC_ISOFunc[] =
{
        {
           srNEXSYS_ESC_ISOFieldPack,
           srNEXSYS_ESC_ISOFieldUnPack,
           srNEXSYS_ESC_ISOFieldCheck,
           srNEXSYS_ESC_ISOFieldType,
           srNEXSYS_ESC_ISOBitMap,
           inNEXSYS_ESC_ISOGetBitMapCode,
           inNEXSYS_ESC_ISOPackMessageType,
           inNEXSYS_ESC_ISOModifyBitMap,
           inNEXSYS_ESC_ISOModifyPackData,
           inNEXSYS_ESC_ISOCheckHeader,
           inNEXSYS_ESC_ISOOnlineAnalyse,
           inNEXSYS_ESC_ISOAdviceAnalyse
        },

	{
           srNEXSYS_ESC_ISOFieldPack_TSAM,
           srNEXSYS_ESC_ISOFieldUnPack,
           srNEXSYS_ESC_ISOFieldCheck,
           srNEXSYS_ESC_ISOFieldType,
           srNEXSYS_ESC_ISOBitMap,
           inNEXSYS_ESC_ISOGetBitMapCode,
           inNEXSYS_ESC_ISOPackMessageType,
           inNEXSYS_ESC_ISOModifyBitMap,
           inNEXSYS_ESC_ISOModifyPackData,
           inNEXSYS_ESC_ISOCheckHeader,
           inNEXSYS_ESC_ISOOnlineAnalyse,
           inNEXSYS_ESC_ISOAdviceAnalyse
        },

};

/*
Function        :inNEXSYS_ESC_Check
Date&Time   :
Describe        : check是否上傳ESC
*/
int inNEXSYS_ESC_Check(TRANSACTION_OBJECT *pobTran)
{
	int	inUpload = VS_TRUE;		/* 預設為上傳 */
//	int	inAgain = 0, inFail = 0;
	char	szHostEnable[2 + 1];
	char	szTRTFileName[16 + 1];
//	char	szESCReciptUploadUpLimit[4 + 1];
	char	szCOMMMode[2 + 1];
	char	szESCMode[2 + 1];

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, " inNEXSYS_ESC_Check INIT");	
	
	/* 若找不到ESC或ESC沒開或水位為0，不送欄位NE */
	memset(szESCMode, 0x00, sizeof(szESCMode));
	inGetESCMode(szESCMode);
	if (memcmp(szESCMode, "Y", strlen("Y")) != 0)
	{
		pobTran->srBRec.inESCUploadMode = _ESC_STATUS_NOT_SUPPORTED_;
		inUpload = VS_FALSE;
		inESC_DebugLog("  Not Upload ESC");
	}

	/* ESC沒開，不上傳 (這個條件已在ESCMode中判斷過，但暫時不拿掉) */
	if (inUpload == VS_TRUE)
	{
		memset(szHostEnable, 0x00, sizeof(szHostEnable));
		if (inNEXSYS_ESC_GetESC_Enable(pobTran->srBRec.inHDTIndex, szHostEnable) != VS_SUCCESS)
		{
			pobTran->srBRec.inESCUploadMode = _ESC_STATUS_NOT_SUPPORTED_;
			inUpload = VS_FALSE;
		}
		else
		{
			if (szHostEnable[0] != 'Y')
			{
				pobTran->srBRec.inESCUploadMode = _ESC_STATUS_NOT_SUPPORTED_;
				inUpload = VS_FALSE;
			}
		}
		
		inDISP_DispLogAndWriteFlie("  Get ESC Br Enable [%d] Line[%d]", pobTran->srBRec.inESCUploadMode, __LINE__);

	}

	/* 3.電子簽單是否上傳 (其他條件寫這，如不支援大來主機或不支援的交易) */
	if (inUpload == VS_TRUE)
	{
		memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
		if (inGetTRTFileName(szTRTFileName) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" ESC Get TRT Fail Line[%d]", __LINE__);
			return (VS_ERROR);
		}
		/* 例如 預先授權不上傳，因為不入帳 的話就打開以下條件  20190328 [SAM] */
//		if (pobTran->srBRec.inCode == _PRE_AUTH_	||
//			(pobTran->srBRec.inCode == _VOID_ && pobTran->srBRec.inOrgCode == _PRE_AUTH_)	||
//			 pobTran->srBRec.inCode == _CUP_PRE_AUTH_	||
//			 pobTran->srBRec.inCode == _CUP_PRE_AUTH_VOID_)
//		{
//			inESC_DebugLog("  InCode Not Support Esc [%d] ", pobTran->srBRec.inCode);
//			pobTran->srBRec.inESCUploadMode = _ESC_STATUS_NOT_SUPPORTED_;
//			inUpload = VS_FALSE;
//		}
		
	}

	/* 只支援TCP/IP */
	if (inUpload == VS_TRUE)
	{
		memset(szCOMMMode, 0x00, sizeof(szCOMMMode));
		inGetCommMode(szCOMMMode);
		if (memcmp(szCOMMMode, _COMM_ETHERNET_MODE_, 2) != 0	&&
		   (memcmp(szCOMMMode, _COMM_GPRS_MODE_, 2) != 0))
		{
			inDISP_DispLogAndWriteFlie("  ESC Comm Mode Err[%s] Line[%d]", szCOMMMode, __LINE__);
			pobTran->srBRec.inESCUploadMode = _ESC_STATUS_NOT_SUPPORTED_;
			inUpload = VS_FALSE;
		}
	}

	/* 水位檢核 */
	/* 水位檢查富邦沒有，所以先拿掉 20181225 [SAM] */
//	if (inUpload == VS_TRUE)
//	{
//		memset(szESCReciptUploadUpLimit, 0x00, sizeof(szESCReciptUploadUpLimit));
//		inGetESCReciptUploadUpLimit(szESCReciptUploadUpLimit);
//	
//		inAgain = inNEXSYS_ESC_Again_Table_Count(pobTran);
//		inFail = inNEXSYS_ESC_Fail_Table_Count(pobTran);
//		
//		inESC_DebugLog("  ESC Upload Limit[%d]", atoi(szESCReciptUploadUpLimit) );
//		inESC_DebugLog("  ESC Again[%d] Fail[%d]", inAgain, inFail );
//		
//		
//		/* 若水位為0，也當成不開ESC(這個條件已在ESCMode中判斷過，但暫時不拿掉) */
//		if (atoi(szESCReciptUploadUpLimit) == 0)
//		{
//			inESC_DebugLog("  水位為0，不送ESC");
//			pobTran->srBRec.inESCUploadMode = _ESC_STATUS_NOT_SUPPORTED_;
//			inUpload = VS_FALSE;
//		}
//		/* 若超過水位 */
//		else if ((inAgain + inFail) >= atoi(szESCReciptUploadUpLimit))
//		{
//			inESC_DebugLog("  %d > %d 超過水位", inAgain + inFail, atoi(szESCReciptUploadUpLimit));
//			pobTran->srBRec.inESCUploadMode = _ESC_STATUS_OVERLIMIT_;
//			inUpload = VS_FALSE;
//		}
//	}

	/* 要上傳都先預設沒簽，等簽之後再改狀態(避免斷電沒簽) */
	if (inUpload == VS_TRUE)
	{
		pobTran->srBRec.inESCUploadMode = _ESC_STATUS_SUPPORTED_;
	}

	/* 6.其他 */
	pobTran->srBRec.inESCTransactionCode = pobTran->inTransactionCode;      /* 重新上傳使用 */

	inESC_DebugLog("  ESC TransCode[%d] ",pobTran->srBRec.inESCTransactionCode);
	
	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inNEXSYS_ESC_Check END");
	
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inNEXSYS_ESC_MerchantCopy_Check
Date&Time   :
Describe        :check是否加印商店聯，如果沒檢查就認為不需列印商店聯
*/
int inNEXSYS_ESC_MerchantCopy_Check(TRANSACTION_OBJECT *pobTran)
{
	char		szESCMerchantCopyFlag[2 + 1];
	char		szStartDate[8 + 1];
	char		szEndDate[8 + 1];
	char		szNowDate[8 + 1];
	RTC_NEXSYS	srRTC;

	memset(szESCMerchantCopyFlag , 0x00, sizeof(szESCMerchantCopyFlag));
	inGetESCPrintMerchantCopy(szESCMerchantCopyFlag);
	if (szESCMerchantCopyFlag[0] == 'Y')
	{
		inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
		/* 起始日 */
		memset(szStartDate , 0x00, sizeof(szStartDate));
		inGetESCPrintMerchantCopyStartDate(szStartDate);
		inESC_DebugLog("  MerchantCopyStartDate = %s", szStartDate);

		/* 結束日 */
		memset(szEndDate , 0x00, sizeof(szEndDate));
		inGetESCPrintMerchantCopyEndDate(szEndDate);
		inESC_DebugLog("  MerchantCopyEndDate = %s", szEndDate);

		/* 現在日期 */
		memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
		inFunc_GetSystemDateAndTime(&srRTC);
		memset(szNowDate, 0x00, sizeof(szNowDate));
		sprintf(szNowDate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		inESC_DebugLog("  NowDate = %s", szNowDate);

		/* 設定值Y，且在起迄日內 */
		/* 用太陽日比較 */
		if (inFunc_SunDay_Sum_Check_In_Range(szNowDate, szStartDate, szEndDate) == VS_SUCCESS)
		{
			return (VS_SUCCESS);
		}

	}

	return (VS_ERROR);
}

/*
Function        :inNEXSYS_ESC_Func_Upload
Date&Time   :2016/5/3 下午 2:06
Describe        : 當筆上傳ESC
*/
int inNEXSYS_ESC_Func_Upload(TRANSACTION_OBJECT *pobTran)
{
	int	i = 0;
	int	inRetVal = VS_ERROR;
	char	szESCMode[2 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inNEXSYS_ESC_Func_Upload INIT" );	

	/* 先確認ESC功能有沒有開 */
	memset(szESCMode, 0x00, sizeof(szESCMode));
	inGetESCMode(szESCMode);
	
	inDISP_DispLogAndWriteFlie("  ESCMode[%s] UploadMode[%d] SignSataus[%d] ",
							szESCMode, pobTran->srBRec.inESCUploadMode, pobTran->srBRec.inSignStatus);
	
	if (memcmp(&szESCMode[0], "Y", 1) != 0)
	{
		/* 沒開，就不跑ESC流程 */
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] ESC NOT OPEN END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}

	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		for (i = 0; i < 1; i++)
		{
			inDISP_PutGraphic(_SEND_, 0, _COORDINATE_Y_LINE_8_7_);/* 傳送中... */
		}

		for (i = 0; i < 1; i++)
		{
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);/* 處理中... */
		}

		for (i = 0; i < 1; i++)
		{
			inDISP_PutGraphic(_RECEIVE_, 0, _COORDINATE_Y_LINE_8_7_);/* 接收中... */
		}

		for (i = 0; i < 1; i++)
		{
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);/* 處理中... */
		}

		/* 標示為已上傳 */
		inBATCH_GetTransRecord_By_Sqlite(pobTran);
		pobTran->srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_UPLOADED_;
		inBATCH_Update_ESC_Uploaded_By_Sqlite(pobTran);

		/* 上傳成功 刪除簽名圖檔 */
		/* 藉由TRT_FileName比對來組出bmp的檔名 */
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		inFunc_Delete_Signature(pobTran);

		/* 結帳帳條統計 */
		inESC_DebugLog("  DEMO ESC Upload UpdateEscAmt (SUCCESS) HID [%d] ",pobTran->srBRec.inHDTIndex);
		if( inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" DEMO ESC Host Lod HDT *Error* HDT[%d] Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
		}
		inACCUM_UpdateFlow_ESC(pobTran, _ESC_ACCUM_STATUS_UPLOADED_);
		
		inESC_DebugLog("-----[%s][%s][%d] DEMO END -----",__FILE__, __FUNCTION__, __LINE__);

        
                    
        
		return (VS_SUCCESS);
	}
	else
	{
		/* 當筆符合ESC上傳條件為
		 * 支援上傳且有簽名 _ESC_STATUS_SUPPORTED_
		 * 或者是支援上傳且免簽名 */
		if (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_SUPPORTED_	&&
		   (pobTran->srBRec.inSignStatus == _SIGN_SIGNED_		||
		    pobTran->srBRec.inSignStatus == _SIGN_NO_NEED_SIGN_))
		{
			/* 不分Online/offline 一律上傳 */
			/* 簽單可以先上傳再上傳advice交易 */
			/* 把之前交易的連線先斷掉 */
			inCOMM_End(pobTran);

			/* 切換到ESC HOST */
			inESC_DebugLog("  BEF ESC Switch Host [%d] ",pobTran->srBRec.inHDTIndex);
			
			/* 找到 ESC HOST 並存 ginNEX_ESCHostIndex 參數並讀取 HDT 不會影響 srBRec.inHDTIndex */
			/* 在這讀取的原因是為了 _COMM_START_ 用 */
			if(inNEXSYS_ESC_SwitchToESC_Host(pobTran->srBRec.inHDTIndex)  !=VS_SUCCESS )
			{
				/* 因為不回傳成功會影響後續處理，所以在只影響 ESC狀況下 先回傳 SUCCESS */
				inDISP_DispLogAndWriteFlie(" ESC SwitchEsc Host Switch *Error* HDT[%d] Line[%d] ", pobTran->srBRec.inHDTIndex, __LINE__);
				inFunc_EDCLock();
				if( inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
				{
					inDISP_DispLogAndWriteFlie(" ESC SwitchEsc Host Lod HDT *Error* HDT[%d] Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
				}
				return VS_SUCCESS;
			}
			/* 通訊也要切到ESC主機(通訊內會直接loadHDT內的CommIndex會自動切過去) */

			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_ESC_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示<電子簽單上傳> */
			/* 電子簽單連線中 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_ESC_CONNECTING_, 0, _COORDINATE_Y_LINE_8_4_);

			inRetVal = inFLOW_RunFunction(pobTran, _COMM_START_);

			/* 開始連線 */
			if (inRetVal != VS_SUCCESS)
			{
				inESC_DebugLog("  ESC_COMM_START ERR [%d] ",inRetVal);
				/* 通訊失敗‧‧‧‧ */
				if( inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
				{
					inDISP_DispLogAndWriteFlie(" ESC _COMM_START_ Lod HDT *Error* HDT[%d] Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
				}
				
				inDISP_DispLogAndWriteFlie("  ESC _COMM_START_ *Error* HostId[%d] Line[%d] ", pobTran->srBRec.inHDTIndex, __LINE__);
				/* 無法連線，直接就存AGAIN區 */
				inNEXSYS_ESC_Insert_Again_Record(pobTran);
			}
			else
			{
				/* 連線成功 */
				inRetVal = inNEXSYS_ESC_ProcessOnline(pobTran, _ESC_UPLOAD_TRADE_);
				
				/* [DEUBG] 測試用 [SAM]*/
//				inRetVal = inGetPrintEscCountData(pobTran);
				
				if (inRetVal == VS_ESCAPE)
				{
					/* 回應碼不為"00"，要存Fail區 */
					/* 因為不需要再上傳，所以不計入Fail 20181223 [SAM] */
					//inNEXSYS_ESC_Insert_Fail_Record(pobTran);
				
					inDISP_DispLogAndWriteFlie("  ESC Online ESCAPE HID[%d] Line[%d] ", pobTran->srBRec.inHDTIndex,  __LINE__);
					
					if( inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
					{
						inDISP_DispLogAndWriteFlie(" ESC Online ESCAPE Lod HDT *Error* HDT[%d] Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
					}
					
					/* 如電簽上傳主機回非00 就認定為紙本 */
					while (1)
					{
						/* 電簽上傳失敗需先顯示提示畫面 20190214 [SAM] */
						inSIGN_Check_Esc_Upload_Fail_Confirm(pobTran);						
						inDISP_Clear_Area(1, _LINE_8_4_, 16, _LINE_8_8_, _ENGLISH_FONT_8X16_);
						
						/* 列印帳單中 */
						inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);
						
						pobTran->srBRec.inPrintOption = _PRT_MERCH_;
						
						inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(pobTran);

						/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
						if (inRetVal != VS_SUCCESS)
						{
							continue;
						}
						else
						{
							/* 成功就砍簽名圖檔 */
							inFunc_Delete_Signature(pobTran);							
							inBATCH_GetTransRecord_By_Sqlite(pobTran);
							pobTran->srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_PAPER_;
							inBATCH_Update_ESC_Uploaded_By_Sqlite(pobTran);
							inDISP_DispLogAndWriteFlie("  ESC Upload Print Paper HID [%d]  Line[%d]",pobTran->srBRec.inHDTIndex ,__LINE__);
							
							/* 結帳帳條統計 */
							if(VS_SUCCESS != inACCUM_UpdateFlow_ESC(pobTran, _ESC_ACCUM_STATUS_BYPASS_)){
								inDISP_DispLogAndWriteFlie("  ESC ByPass Accum Update *Error*");
							}
							break;
						}

					}
					
				}else if (inRetVal == VS_ERROR)
				{
					/* 送電文失敗，存AGAIN區 */
					inDISP_DispLogAndWriteFlie("  ESC Upload Inster Again *Error* HID [%d]  Line[%d]",pobTran->srBRec.inHDTIndex, __LINE__);

					if(VS_SUCCESS != inNEXSYS_ESC_Insert_Again_Record(pobTran))
					{
						inDISP_DispLogAndWriteFlie("  ESC Upload Inster Again *Error*");
					}
				}
				else if (inRetVal == _NEXSYS_ESC_PACK_ERR_ ||		/* 組電文失敗 */
					  inRetVal == _NEXSYS_ESC_UNPACK_ERR_)		/* 解電文失敗 */
				{
					inDISP_DispLogAndWriteFlie("  ESC Upload Inster Again PACK *Error* HID [%d]  Line[%d]",pobTran->srBRec.inHDTIndex, __LINE__);
					/* 組解電文失敗，存Fail區 */
//					inNEXSYS_ESC_Insert_Fail_Record(pobTran);
					/* 目前富邦不記錄其它狀態，所以都存成Again 20181225 [SAM]*/
					if(VS_SUCCESS != inNEXSYS_ESC_Insert_Again_Record(pobTran))
					{
						inDISP_DispLogAndWriteFlie("  ESC Upload Inster Again Pack *Error*");
					}
				}
				else if (inRetVal == VS_SUCCESS)
				{
					inDISP_DispLogAndWriteFlie("  ESC Online Success Host[%d] Line[%d] ", pobTran->srBRec.inHDTIndex,  __LINE__);
					/* 標示為已上傳 */
					inBATCH_GetTransRecord_By_Sqlite(pobTran);
					pobTran->srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_UPLOADED_;
					inBATCH_Update_ESC_Uploaded_By_Sqlite(pobTran);

					/* 上傳成功 刪除簽名圖檔 */
					/* 藉由TRT_FileName比對來組出bmp的檔名 */
					inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
					inFunc_Delete_Signature(pobTran);
					
					/* 結帳帳條統計 */
					inESC_DebugLog("  ESC Upload UpdateEscAmt (SUCCESS) HID [%d] ",pobTran->srBRec.inHDTIndex);
					if( inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
					{
						inDISP_DispLogAndWriteFlie(" ESC Upload UpdateEscAmt Lod HDT *Error* HDT[%d] Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
					}

					if(VS_SUCCESS != inACCUM_UpdateFlow_ESC(pobTran, _ESC_ACCUM_STATUS_UPLOADED_)){
						inDISP_DispLogAndWriteFlie("  ESC Uploaded Accum Update *Error*");
					}
					
					/* 如果連線上傳成功，需要再補傳 Again 的交易 */
					ginNEX_ESC_Connect_flag = _ESC_CONNECT_ALREADY_;
					
					inNEXSYS_ESC_Func_Upload_AfterTrans(pobTran);					
				}
								
			}
			
			/* 回到Idle立刻上傳ESC */
//			ginNEX_ESC_Idle_flag = _ESC_IDLE_UPLOAD_;

		}
		else
		{	/* 在這裡計算 _ESC_STATUS_OVERLIMIT_ 和 _ESC_STATUS_BYPASS_ 筆數 （有出紙本的）*/
			/* 標示為已出紙本 */
			inBATCH_GetTransRecord_By_Sqlite(pobTran);
			pobTran->srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_PAPER_;
			inBATCH_Update_ESC_Uploaded_By_Sqlite(pobTran);

			/* 結帳帳條統計Bypass */
			/* 支援但是Bypass */
			/* 支援但沒簽名斷電 */
			if (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_SUPPORTED_	&&
			   (pobTran->srBRec.inSignStatus == _SIGN_BYPASS_	||
				pobTran->srBRec.inSignStatus == _SIGN_NONE_))
			{
				inDISP_DispLogAndWriteFlie("  ESC Upload  UpdateEsc( _SIGN_NONE_) HID [%d] ",pobTran->srBRec.inHDTIndex);

				if(VS_SUCCESS != inACCUM_UpdateFlow_ESC(pobTran, _ESC_ACCUM_STATUS_BYPASS_)){
					inDISP_DispLogAndWriteFlie("  ESC Uploaded Accum ByPass *Error*");
				}

//				/* 原本"NE"送Y的才要送advie */
//				/* 組成對應主機ADVICE */
//				if (pobTran->srBRec.uszOfflineBit == VS_TRUE)
//				{
//					/* 因為電文後送所以就不Append，改在原本advice送 */
//				}
//				else
//				{
//					/* 當筆出紙本要送advice告訴fes把當筆紙本的flag on 起來 */
//					inBATCH_ESC_Save_Advice_By_Sqlite(pobTran);
//				}
			}
			else
			{
				/* _ESC_STATUS_NOT_SUPPORTED_	不支援也會在這裡*/
				/* _ESC_STATUS_OVERLIMIT_	超過水位算上傳電簽失敗，因為馬上出紙本所以直接加入accum */
				inDISP_DispLogAndWriteFlie("  ESC Upload Other Paper AMT HID [%d] Line[%d]",pobTran->srBRec.inHDTIndex, __LINE__);
				//if(VS_SUCCESS != inACCUM_UpdateFlow_ESC(pobTran, _ESC_ACCUM_STATUS_FAIL_)){
				if(VS_SUCCESS != inACCUM_UpdateFlow_ESC(pobTran, _ESC_ACCUM_STATUS_BYPASS_)){
					inDISP_DispLogAndWriteFlie("  ESC Uploaded Accum Fail *Error*");
				}
				
			}
		}

		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inNEXSYS_ESC_Func_Upload END");

		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}
}

/*
Function        :inNEXSYS_ESC_Func_Upload_Idle
Date&Time       :2016/5/3 下午 2:06
Describe        :idle上傳ESC
*/
int inNEXSYS_ESC_Func_Upload_Idle(TRANSACTION_OBJECT *pobTran)
{
	char	szDemoMode[2 + 1] = {0};
	
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		int		inRetVal;
		int		inHostIndex = -1;
		int		inAgain = 0;						/* Again table中的筆數 */
		int		inTotalAgain = 0;
		char		szTemp[4 + 1];
		char		szHostName[8 + 1];
		char		szESCMode[2 + 1];
		char		szTMSOK[2 + 1];
		RTC_NEXSYS	srRTC;

		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
		
		inESC_DebugLog("  ConnectionBit [%d] gNexConnectFlag[%d]", pobTran->uszConnectionBit, ginNEX_ESC_Connect_flag );

		if (ginNEX_ESC_Connect_flag == _ESC_CONNECT_ALREADY_)
		{
			pobTran->uszConnectionBit = VS_TRUE;
		}
		else
		{
			pobTran->uszConnectionBit = VS_FALSE;
		}
		/* 基本檢核 */
		memset(szTMSOK, 0x00, sizeof(szTMSOK));
		inGetTMSOK(szTMSOK);

		if (szTMSOK[0] != 'Y')
		{
			inESC_DebugLog("-----[%s][%s][%d]  TMS NOT OK END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_SUCCESS);
		}

		/* 先確認ESC功能有沒有開 */
		memset(szESCMode, 0x00, sizeof(szESCMode));
		inGetESCMode(szESCMode);
		if (memcmp(&szESCMode[0], "Y", 1) != 0)
		{
			/* 沒開，就不跑ESC流程 */
			inESC_DebugLog("-----[%s][%s][%d]  ESC NOT OPEN END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_SUCCESS);
		}

		/* 更新時間成為新的判斷點 */
		memset(&srRTC, 0x00, sizeof(CTOS_RTC));
		/* 取得EDC時間日期 */
		if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
		{
			inESC_DebugLog("-----[%s][%s][%d]  GET TIME ERR END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
		}

		memset(&gszNEX_ESC_Date, 0x00, sizeof(gszNEX_ESC_Date));
		memset(&gszNEX_ESC_Time, 0x00, sizeof(gszNEX_ESC_Time));
		sprintf(gszNEX_ESC_Date, "20%02d%02d%02d",  srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		sprintf(gszNEX_ESC_Time, "%02d%02d%02d",  srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);

		/* 查各Host中是否有again筆數 */
		inTotalAgain = 0;

		/*TODO: 看能不能寫成可變動功能 */
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_FUBON_, &inHostIndex);

		inESC_DebugLog("  Get Host Id [%d]",inHostIndex );
		
		pobTran->srBRec.inHDTIndex = inHostIndex;
		inAgain = inNEXSYS_ESC_Again_Table_Count(pobTran);

		if (inAgain >= 0)
		{
			inTotalAgain += inAgain;
		}

		/* 沒筆數或是重試超過三次 */
		if (inTotalAgain <= 0 || ginNEX_ESC_Send_Cnt > _ESC_SEND_RETRY_LIMIT_)
		{
			if (ginNEX_ESC_Connect_flag == _ESC_CONNECT_ALREADY_)
			{
				/* 沒有上傳資料要斷線 */
				inCOMM_End(pobTran);
				ginNEX_ESC_Connect_flag = _ESC_CONNECT_NOT_CONNECT_;
			}

			ginNEX_ESC_Send_Cnt = 0;
//			ginNEX_ESC_Idle_flag = _ESC_IDLE_NO_NEED_UPLOAD_;
			inESC_DebugLog("-----[%s][%s][%d]  RE TRY  END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_SUCCESS);
		}

		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_ESC_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示<電子簽單上傳> */

		if (ginNEX_ESC_Connect_flag == _ESC_CONNECT_NOT_CONNECT_)
		{
			inNEXSYS_ESC_SwitchToESC_Host(pobTran->srBRec.inHDTIndex);
			/* 通訊也要切到ESC主機(通訊內會直接loadHDT內的CommIndex會自動切過去) */

			/* 電子簽單連線中 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_ESC_CONNECTING_, 0, _COORDINATE_Y_LINE_8_4_);

			/* 連線 */
			if ((inRetVal = inFLOW_RunFunction(pobTran, _COMM_START_)) != VS_SUCCESS)
			{
				ginNEX_ESC_Connect_flag = _ESC_CONNECT_NOT_CONNECT_;
//				ginNEX_ESC_Idle_flag = _ESC_IDLE_NO_NEED_UPLOAD_;	/* 失敗就等五分鐘時間 */
				/* 通訊失敗‧‧‧‧ */
			}
			else
			{
				ginNEX_ESC_Connect_flag = _ESC_CONNECT_ALREADY_;
//				ginNEX_ESC_Idle_flag = _ESC_IDLE_UPLOAD_;			/* 成功表示連線上，下次Idle要直接跑上傳 */
			}
		}
		else
		{
				/* 有連線 */
				inRetVal = VS_SUCCESS;
		}

		if (inRetVal == VS_SUCCESS)
		{
			/* 上傳A區失敗的電子簽名 ======================================================== */
			/*       失敗 = (1)有送出去，若主機拒絕就移掉Again區紀錄，存Fail區，其他則無任何動作
							(2)沒送出去，無任何動作，下次重試
					 成功 = 移掉Again區紀錄
			*/
			/* 這裡上傳NCCC的 */
			/*TODO: 看能不能寫成可變動功能 */
			inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_FUBON_, &inHostIndex);

			inESC_DebugLog("  ESC Aft Connect Get Host Id [%d]",inHostIndex );
			pobTran->srBRec.inHDTIndex = inHostIndex;

			if( inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
			{
				inDISP_DispLogAndWriteFlie(" ESC Aft Connect Load HDT[%d] *Error* Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
			}
			if( inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
			{
				inDISP_DispLogAndWriteFlie(" ESC Aft Connect Load HDPT[%d] *Error* Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
			}

			
			while(1)
			{
				/* 初始化 */
				inAgain = 0;
				/* Again table的count */
				inAgain = inNEXSYS_ESC_Again_Table_Count(pobTran);

				/* 若有次數則上傳 */
				if (inAgain > 0)
				{

				}
				/* 沒有上傳筆數 跳出*/
				else
				{
					break;
				}
				
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_ESC_NOT_UPLOADED_YET_, 0, _COORDINATE_Y_LINE_8_4_);
				
				/* 顯示主機別 */
				memset(szHostName, 0x00, sizeof(szHostName));
				if (inGetHostLabel(szHostName) != VS_SUCCESS)
				{
					inESC_DebugLog("-----[%s][%s][%d]  Get Host Label ERR  END -----",__FILE__, __FUNCTION__, __LINE__);
					return (VS_ERROR);
				}

				inDISP_EnglishFont(szHostName, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

				memset(szTemp, 0x00, sizeof(szTemp));
				sprintf(szTemp, "%d", inAgain);
				inDISP_ChineseFont(szTemp, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_);
				//inDISP_Wait(1000);	/* 等一秒，不然會看不到上傳狀態 */

				/* 這裡要跑上傳流程 : 連線沿用 收送 */
				inRetVal = inNEXSYS_ESC_ProcessOnline(pobTran, _ESC_UPLOAD_IDLE_);

				if (inRetVal == VS_ESCAPE)		        /* 回應碼不為"00"，要存Fail區 */
				{
					inESC_DebugLog("  ESC Upload_IDLE  Inster Fail [ESCAPE] HID [%d] ",pobTran->srBRec.inHDTIndex);
					/* 抓出資料 */
					inNEXSYS_ESC_Get_Again_Record_Most_TOP(pobTran);
					/* 塞到Fail區 */
					inNEXSYS_ESC_Insert_Fail_Record(pobTran);
					/* 刪掉again區的 */
					inNEXSYS_ESC_Delete_Again_Record_Most_TOP(pobTran);
					break;
				}
				else if (inRetVal == VS_ERROR)	                /* 送電文失敗 */
				{
					inESC_DebugLog("  ESC Upload_IDLE  Error Status HID [%d] ",pobTran->srBRec.inHDTIndex);
					ginNEX_ESC_Send_Cnt ++;
					break;
				}
				else if (inRetVal == _NEXSYS_ESC_PACK_ERR_ ||	/* 組電文失敗 */
					  inRetVal == _NEXSYS_ESC_UNPACK_ERR_)	/* 解電文失敗 */
				{
					inESC_DebugLog("  ESC Upload_IDLE  Inster Fail [UnpakErr] HID [%d] ",pobTran->srBRec.inHDTIndex);
					/* 抓出資料 */
					inNEXSYS_ESC_Get_Again_Record_Most_TOP(pobTran);
					/* 塞到Fail區 */
					inNEXSYS_ESC_Insert_Fail_Record(pobTran);
					/* 刪掉again區的 */
					inNEXSYS_ESC_Delete_Again_Record_Most_TOP(pobTran);
					break;
				}
				else if (inRetVal >= 0)
				{
					/* 標示為已上傳 */
					inBATCH_GetTransRecord_By_Sqlite(pobTran);
					pobTran->srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_UPLOADED_;
					inBATCH_Update_ESC_Uploaded_By_Sqlite(pobTran);
					inNEXSYS_ESC_Delete_Again_Record_Most_TOP(pobTran);

					/* 更新ESC上傳成功筆數金額 */
					inESC_DebugLog("  ESC Upload_IDLE  Update Esc AMT HID [%d] ",pobTran->srBRec.inHDTIndex);

					if( inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
					{
						inDISP_DispLogAndWriteFlie(" ESC Upload_IDLE  Update Esc AMT Load HDT[%d] *Error* Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
					}
					
					inACCUM_UpdateFlow_ESC(pobTran, _ESC_ACCUM_STATUS_UPLOADED_);
				}
			}

			inCOMM_End(pobTran);
		}
		
	

		inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

		return (VS_SUCCESS);
	}
}


/*
Function        :inNEXSYS_ESC_Func_Upload_Settle
Date&Time       :2016/5/4 下午 4:23
Describe        :Settle上傳ESC
*/
int inNEXSYS_ESC_Func_Upload_Settle(TRANSACTION_OBJECT *pobTran)
{
	int	inWriteIntoFailLog = VS_FALSE, inCofirmMessage = VS_TRUE;
	int	inRetVal = 0;
	int	inAgain = -1, inFail = -1;	/* Again table中的筆數 */
	char	szTemp[4 + 1] = {0};
	char	szHostName[8 + 1] = {0};
	char	szESCMode[2 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	TRANSACTION_OBJECT	pobESCTran;

	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

		inDISP_DispLogAndWriteFlie("  ginNEX_ESC_Connect_flag[%d] ", ginNEX_ESC_Connect_flag);
		/* 先確認ESC功能有沒有開 */
		memset(szESCMode, 0x00, sizeof(szESCMode));
		inGetESCMode(szESCMode);
		if (memcmp(&szESCMode[0], "Y", 1) != 0)
		{
			/* 沒開，就不跑ESC流程 */
			inESC_DebugLog("-----[%s][%s][%d]  ESC Not Open  END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_SUCCESS);
		}

		/* 修改讀資料會影響到結帳交易別的問題 */
		memset(&pobESCTran, 0x00, sizeof(pobESCTran));
		memcpy(&pobESCTran, pobTran, sizeof(TRANSACTION_OBJECT));

		inDISP_DispLogAndWriteFlie("  Settle pESC_ID[%d]  pob_HID[%d]",pobESCTran.srBRec.inHDTIndex , pobTran->srBRec.inHDTIndex);
		
		//inDISP_ClearAll();
		//inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		//inDISP_PutGraphic(_MENU_ESC_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示<電子簽單上傳> */


		/* Again table的count */
		inAgain = inNEXSYS_ESC_Again_Table_Count(&pobESCTran);

		inESC_DebugLog("  Settle Again Count[%d] ", inAgain);

		if (inAgain > 0)
		{
			inESC_DebugLog("  Settle Runt Again Count[%d] ", inAgain);
			inNEXSYS_ESC_SwitchToESC_Host(pobESCTran.srBRec.inHDTIndex);
			/* 通訊也要切到ESC主機(通訊內會直接loadHDT內的CommIndex會自動切過去) */

			/* 第一層顯示<電子簽單上傳> */
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_ESC_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示<電子簽單上傳> */
			/* 電子簽單連線中 */
			inDISP_PutGraphic(_ESC_CONNECTING_, 0, _COORDINATE_Y_LINE_8_4_);

			/* 連線 */
			inESC_DebugLog("  Settle Run Again Count CommStart ");
			inESC_DebugLog("  function id[%d] ",_COMM_START_ );
			if ((inRetVal = inFLOW_RunFunction(&pobESCTran, _COMM_START_)) != VS_SUCCESS)
			{
				/* 通訊失敗‧‧‧‧ */
				/* 後續不再試著連線，全部寫入失敗區 */
				inWriteIntoFailLog = VS_TRUE;
			}
			else
			{

			}
			/* 如果連線失敗，就全部轉為紙本簽單 20190220 [SAM] */
			if(inWriteIntoFailLog == VS_TRUE)
			{
				if( inLoadHDTRec(pobESCTran.srBRec.inHDTIndex) == VS_ERROR)
				{
					inDISP_DispLogAndWriteFlie(" ESC inWriteIntoFailLog  Load HDT[%d] *Error* Line[%d]",pobESCTran.srBRec.inHDTIndex,__LINE__);
				}
				if( inLoadHDPTRec(pobESCTran.srBRec.inHDTIndex) == VS_ERROR)
				{
					inDISP_DispLogAndWriteFlie(" ESC inWriteIntoFailLog Load HDPT[%d] *Error* Line[%d]",pobESCTran.srBRec.inHDTIndex,__LINE__);
				}
									
				while(1)
				{
					/* Again table的count */
					inAgain = -1;
					/* Again table的count */
					inAgain = inNEXSYS_ESC_Again_Table_Count(&pobESCTran);
				
					/* 若有次數則記錄進失敗區  */
					if (inAgain <= 0)
					{
						break;
					}
					
					/* 除了上傳成功外都記入需要出紙本的計數 20190107 [SAM] */
					/* 抓出資料 */
					if(VS_SUCCESS != inNEXSYS_ESC_Get_Again_Record_Most_TOP(&pobESCTran))
					{
						inDISP_DispLogAndWriteFlie("  ESC Settle Connect Get Again Top *Error* RecId[%d] HID[%d] Line[%d]",inAgain, pobESCTran.srBRec.inHDTIndex, __LINE__);
					}
					/* 塞到Fail區 */
					if(VS_SUCCESS != inNEXSYS_ESC_Insert_Fail_Record(&pobESCTran))
					{
						inDISP_DispLogAndWriteFlie("   ESC Settle Connect Insert Rec *Error* RecId[%d] HID[%d] Line[%d]",inAgain, pobESCTran.srBRec.inHDTIndex, __LINE__);
					}
					
					/* 刪掉again區的 */
					if(VS_SUCCESS != inNEXSYS_ESC_Delete_Again_Record_Most_TOP(&pobESCTran))
					{
						inDISP_DispLogAndWriteFlie("   ESC Settle Connect Del Rec Top *Error* RecId[%d] HID[%d] Line[%d]",inAgain, pobESCTran.srBRec.inHDTIndex, __LINE__);
					}
				}
				
			}else if (inRetVal == VS_SUCCESS)
			{
				/* 上傳A區失敗的電子簽名 ======================================================== */
				/*       失敗 = (1)有送出去，若主機拒絕就移掉Again區紀錄，存Fail區，其他則無任何動作
								(2)沒送出去，失敗就存存Fail區
						 成功 = 移掉Again區紀錄
				*/
				if( inLoadHDTRec(pobESCTran.srBRec.inHDTIndex) == VS_ERROR)
				{
					inDISP_DispLogAndWriteFlie(" ESC VS_SUCCESS  Load HDT[%d] *Error* Line[%d]",pobESCTran.srBRec.inHDTIndex,__LINE__);
				}
				if( inLoadHDPTRec(pobESCTran.srBRec.inHDTIndex) == VS_ERROR)
				{
					inDISP_DispLogAndWriteFlie(" ESC VS_SUCCESS Load HDPT[%d] *Error* Line[%d]",pobESCTran.srBRec.inHDTIndex,__LINE__);
				}
				
				while(1)
				{
					/* Again table的count */
					inAgain = -1;
					/* Again table的count */
					inAgain = inNEXSYS_ESC_Again_Table_Count(&pobESCTran);

					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_ESC_NOT_UPLOADED_YET_, 0, _COORDINATE_Y_LINE_8_4_);

					/* 顯示主機別 */
					memset(szHostName, 0x00, sizeof(szHostName));
					if (inGetHostLabel(szHostName) != VS_SUCCESS)
					{
						return (VS_ERROR);
					}
					inDISP_EnglishFont(szHostName, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

					memset(szTemp, 0x00, sizeof(szTemp));
					sprintf(szTemp, "%d", inAgain);
					inDISP_ChineseFont(szTemp, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_);
//					inDISP_Wait(1000);

					/* 若有次數則上傳 */
					if (inAgain > 0)
					{

					}
					/* 沒有上傳筆數 跳出*/
					else
					{
						break;
					}
					
					/* 如果連線失敗，就全部轉為紙本簽單 20190220 [SAM] */
					if(inWriteIntoFailLog == VS_TRUE)
					{
						/* 除了上傳成功外都記入需要出紙本的計數 20190107 [SAM] */
						if(VS_SUCCESS != inNEXSYS_ESC_Get_Again_Record_Most_TOP(&pobESCTran)){
							inDISP_DispLogAndWriteFlie("  ESC Settle Connect Get Again Top *Error* RecId[%d] HID[%d] Line[%d]",inAgain, pobESCTran.srBRec.inHDTIndex, __LINE__);
						}
						/* 塞到Fail區 */
						if(VS_SUCCESS != inNEXSYS_ESC_Insert_Fail_Record(&pobESCTran))
						{
							inDISP_DispLogAndWriteFlie("   ESC Settle Connect Insert Rec *Error* RecId[%d] HID[%d] Line[%d]",inAgain, pobESCTran.srBRec.inHDTIndex, __LINE__);
						}

						/* 刪掉again區的 */
						if(VS_SUCCESS != inNEXSYS_ESC_Delete_Again_Record_Most_TOP(&pobESCTran)){
							inDISP_DispLogAndWriteFlie("   ESC Settle Connect Del Rec Top *Error* RecId[%d] HID[%d] Line[%d]",inAgain, pobESCTran.srBRec.inHDTIndex, __LINE__);
						}
						
						//break;
						inWriteIntoFailLog = VS_TRUE;
					}else
					{
						/* 這裡要跑上傳流程 : 連線沿用 收送 */
						inRetVal = inNEXSYS_ESC_ProcessOnline(&pobESCTran, _ESC_UPLOAD_IDLE_);

						if (inRetVal >= 0)
						{
							inDISP_DispLogAndWriteFlie("  Settle ESC UPLOAD Success ");
							/* 標示為已上傳 */
							inBATCH_GetTransRecord_By_Sqlite(&pobESCTran);
							pobESCTran.srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_UPLOADED_;
							inBATCH_Update_ESC_Uploaded_By_Sqlite(&pobESCTran);

							if(VS_SUCCESS != inNEXSYS_ESC_Delete_Again_Record_Most_TOP(&pobESCTran)){
								inDISP_DispLogAndWriteFlie("  ESC Settle Online Del Again Top *Error* RecId[%d] HID[%d] Line[%d]",inAgain, pobESCTran.srBRec.inHDTIndex, __LINE__);
							}

							/* 更新ESC上傳成功筆數金額 */
							if( inLoadHDTRec(pobESCTran.srBRec.inHDTIndex) == VS_ERROR)
							{
								inDISP_DispLogAndWriteFlie(" ESC Bef Settle Online Accum Load HDT[%d] *Error* Line[%d]",pobESCTran.srBRec.inHDTIndex,__LINE__);
							}
							
							if(VS_SUCCESS != inACCUM_UpdateFlow_ESC(&pobESCTran, _ESC_ACCUM_STATUS_UPLOADED_)){
								inDISP_DispLogAndWriteFlie("  ESC Settle Online Accum Fail *Error* Line[%d]", __LINE__);
							}
						}else
						{	
							inDISP_DispLogAndWriteFlie("  Settle ESC UPLOAD Fail ");
							/* 除了上傳成功外都記入需要出紙本的計數 20190107 [SAM] */
							if(VS_SUCCESS != inNEXSYS_ESC_Get_Again_Record_Most_TOP(&pobESCTran)){
								inDISP_DispLogAndWriteFlie("  ESC Settle Online Get Again Top *Error* RecId[%d] HID[%d] Line[%d]",inAgain, pobESCTran.srBRec.inHDTIndex, __LINE__);
							}
							/* 塞到Fail區 */
							if(VS_SUCCESS != inNEXSYS_ESC_Insert_Fail_Record(&pobESCTran))
							{
								inDISP_DispLogAndWriteFlie("   ESC Settle Online Insert Rec *Error* RecId[%d] HID[%d] Line[%d]",inAgain, pobESCTran.srBRec.inHDTIndex, __LINE__);
							}

							/* 刪掉again區的 */
							if(VS_SUCCESS != inNEXSYS_ESC_Delete_Again_Record_Most_TOP(&pobESCTran)){
								inDISP_DispLogAndWriteFlie("   ESC Settle Online Del Rec Top *Error* RecId[%d] HID[%d] Line[%d]",inAgain, pobESCTran.srBRec.inHDTIndex, __LINE__);
							}
							
							//break;
							inWriteIntoFailLog = VS_TRUE;
						}
					}
				}

				inCOMM_End(&pobESCTran);
			}

		}

		inESC_DebugLog("  Settle Bef Reload  pESC_ID[%d]  pob_HID[%d]",pobESCTran.srBRec.inHDTIndex , pobTran->srBRec.inHDTIndex);
		inDISP_DispLogAndWriteFlie(" Settle Bef Reload pESC_ID[%d]  pob_HID[%d]",pobESCTran.srBRec.inHDTIndex , pobTran->srBRec.inHDTIndex);
		/* 印帳單load回原host */
		if( inLoadHDTRec(pobESCTran.srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" Settle Bef Reload Load HDT[%d] *Error* Line[%d]",pobESCTran.srBRec.inHDTIndex,__LINE__);
		}

		inFail = -1;
		/* Fail table的count */
		inFail = inNEXSYS_ESC_Fail_Table_Count(&pobESCTran);
		inESC_DebugLog("  Settle Fail Count[%d] ", inFail);
		if (inFail > 0)
		{
			/* 列印Fail區紙本 */
			while(1)
			{
				inFail = -1;
				/* Fail table的count */
				inFail = inNEXSYS_ESC_Fail_Table_Count(&pobESCTran);

				if (inFail <= 0)
				{
					break;
				}

				if (inNEXSYS_ESC_Get_Fail_Record_Most_TOP(&pobESCTran) != VS_SUCCESS)
				{
					break;
				}


				pobESCTran.srBRec.inPrintOption = _PRT_MERCH_;

				/* 已重LOAD 過,不需要再LOAD */
//				/* 印帳單load回原host */
//				inLoadHDTRec(pobESCTran.srBRec.inHDTIndex);

				while (1)
				{
					/* 電簽上傳失敗需先顯示提示畫面 20190214 [SAM] */
					/* 因結帳部份只需顯示一次，所以增加此判斷式 20190220 [SAM] */
					if(inCofirmMessage == VS_TRUE)
					{
						inSIGN_Check_Esc_Upload_Fail_Confirm(&pobESCTran);
						inCofirmMessage = VS_FALSE;
					}
					
					inDISP_Clear_Area(1, _LINE_8_4_, 16, _LINE_8_8_, _ENGLISH_FONT_8X16_);
					
					
					/* 列印帳單中 */
					inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

					inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(&pobESCTran);

					/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
					if (inRetVal != VS_SUCCESS)
					{
						continue;
					}
					else
					{
						/* 成功就砍簽名圖檔 */
						inFunc_Delete_Signature(&pobESCTran);
						break;
					}
				}

				/* 直接從Fail區移掉 */
				inNEXSYS_ESC_Delete_Fail_Record_Most_TOP(&pobESCTran);
				
				/* 結帳帳條統計 */
				//inACCUM_UpdateFlow_ESC(&pobESCTran, _ESC_ACCUM_STATUS_FAIL_);
				/* 因為富邦要算成出紙本, 所以要改成BYPASS */
				inDISP_DispLogAndWriteFlie("  ESC Upload_Settle Update Amt Aft Print Fail Paper Esc HID [%d] ",pobESCTran.srBRec.inHDTIndex);
				inACCUM_UpdateFlow_ESC(&pobESCTran, _ESC_ACCUM_STATUS_BYPASS_);
						
				/* 當筆出紙本要送advice告訴fes把當筆紙本的flag on 起來(DCC才會用到NE) */
				//inBATCH_ESC_Save_Advice_By_Sqlite(&pobESCTran);

				/* 標示為已出紙本 */
				inBATCH_GetTransRecord_By_Sqlite(&pobESCTran);
				pobTran->srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_PAPER_;
				inBATCH_Update_ESC_Uploaded_By_Sqlite(&pobESCTran);
			}

		}

		inDISP_DispLogAndWriteFlie("   Settle End Bef Reload  pESC_ID[%d]  pob_HID[%d]",pobESCTran.srBRec.inHDTIndex , pobTran->srBRec.inHDTIndex);
//		/* 結帳前最後紀錄 */
//		/* Sale已上傳 */
//		if (inBATCH_Get_ESC_Upload_Count_Flow(&pobESCTran, _TN_BATCH_TABLE_, _ESC_REINFORCE_TXNCODE_SALE_, VS_FALSE, &pobTran->inESC_Sale_UploadCnt, &pobTran->lnESC_Sale_UploadAmt) != VS_SUCCESS)
//		{
//			inESC_DebugLog("  ESC COUNT SQLite read fail.");
//
//		}
//
//		/* Refund已上傳 */
//		if (inBATCH_Get_ESC_Upload_Count_Flow(&pobESCTran, _TN_BATCH_TABLE_, _ESC_REINFORCE_TXNCODE_REFUND_, VS_FALSE, &pobTran->inESC_Refund_UploadCnt, &pobTran->lnESC_Refund_UploadAmt) != VS_SUCCESS)
//		{
//			inESC_DebugLog("  ESC COUNT SQLite read fail.");
//		}
//
//		/* Sale出紙本 */
//		if (inBATCH_Get_ESC_Upload_Count_Flow(&pobESCTran, _TN_BATCH_TABLE_, _ESC_REINFORCE_TXNCODE_SALE_, VS_TRUE, &pobTran->inESC_Sale_PaperCnt, &pobTran->lnESC_Sale_PaperAmt) != VS_SUCCESS)
//		{
//			inESC_DebugLog("  ESC COUNT SQLite read Flow fail.");
//		}
//
//		/* Refund出紙本 */
//		if (inBATCH_Get_ESC_Upload_Count_Flow(&pobESCTran, _TN_BATCH_TABLE_, _ESC_REINFORCE_TXNCODE_REFUND_, VS_TRUE, &pobTran->inESC_Refund_PaperCnt, &pobTran->lnESC_Refund_PaperAmt) != VS_SUCCESS)
//		{
//			inESC_DebugLog("  ESC COUNT SQLite read fail.");
//		}
		
		if( inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" Settle End Bef Reload Recover Load HDT[%d] *Error* Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
		}
		if( inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" Settle End Bef Reload Recover Load HDPT[%d] *Error* Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
		}

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}
}

/*
Function        :inNEXSYS_ESC_Func_Upload_AfterTrans
Date&Time       :2016/5/3 下午 2:06
Describe        : 在交易後上傳上傳ESC
*/
int inNEXSYS_ESC_Func_Upload_AfterTrans(TRANSACTION_OBJECT *pobTran)
{
	char	szDemoMode[2 + 1] = {0};
	int inEscSendCount = 0;
	
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		int		inRetVal;
		int		inHostIndex = -1;
		int		inAgain = 0;						/* Again table中的筆數 */
		int		inTotalAgain = 0;
		char		szTemp[4 + 1];
		char		szHostName[8 + 1];
		char		szESCMode[2 + 1];
		char		szTMSOK[2 + 1];
		RTC_NEXSYS	srRTC;
		TRANSACTION_OBJECT	pobESCTran;
		
		memset(&pobESCTran, 0x00, sizeof(pobESCTran));
		memcpy(&pobESCTran, pobTran, sizeof(TRANSACTION_OBJECT));

		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
		inDISP_DispLogAndWriteFlie("  ginNEX_ESC_Connect_flag[%d] ", ginNEX_ESC_Connect_flag);
		
		if (ginNEX_ESC_Connect_flag == _ESC_CONNECT_ALREADY_)
		{
			pobESCTran.uszConnectionBit = VS_TRUE;
		}
		else
		{
			pobESCTran.uszConnectionBit = VS_FALSE;
		}
		/* 基本檢核 */
		memset(szTMSOK, 0x00, sizeof(szTMSOK));
		inGetTMSOK(szTMSOK);

		if (szTMSOK[0] != 'Y')
		{
			inESC_DebugLog("-----[%s][%s][%d]  TMS NOT OK END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_SUCCESS);
		}

		/* 先確認ESC功能有沒有開 */
		memset(szESCMode, 0x00, sizeof(szESCMode));
		inGetESCMode(szESCMode);
		if (memcmp(&szESCMode[0], "Y", 1) != 0)
		{
			/* 沒開，就不跑ESC流程 */
			inESC_DebugLog("-----[%s][%s][%d]  ESC NOT OPEN END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_SUCCESS);
		}

		/* 更新時間成為新的判斷點 */
		memset(&srRTC, 0x00, sizeof(CTOS_RTC));
		/* 取得EDC時間日期 */
		if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("-----[%s][%s][%d]  GET TIME ERR END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
		}

		memset(&gszNEX_ESC_Date, 0x00, sizeof(gszNEX_ESC_Date));
		memset(&gszNEX_ESC_Time, 0x00, sizeof(gszNEX_ESC_Time));
		sprintf(gszNEX_ESC_Date, "20%02d%02d%02d",  srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		sprintf(gszNEX_ESC_Time, "%02d%02d%02d",  srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);

		/* 查各Host中是否有again筆數 */
		inTotalAgain = 0;
		
		/*TODO: 看能不能寫成可變動功能 */
		if(VS_SUCCESS != inFunc_Find_Specific_HDTindex(pobESCTran.srBRec.inHDTIndex, _HOST_NAME_CREDIT_FUBON_, &inHostIndex))
		{
			inDISP_DispLogAndWriteFlie("  ESC FU Aft Trans Upload Find HDT *Error* Line[%d]",  __LINE__);
		}

		inDISP_DispLogAndWriteFlie("  Find The HID For Again Count HID[%d] Line[%d]", inHostIndex, __LINE__);
		
		pobESCTran.srBRec.inHDTIndex = inHostIndex;
		inAgain = inNEXSYS_ESC_Again_Table_Count(&pobESCTran);
		inESC_DebugLog("  Esc Again Count  [%d] ", inAgain);
				
		if (inAgain >= 0)
		{
			inTotalAgain += inAgain;
		}

		/* 沒筆數 */
		if (inTotalAgain <= 0 )
		{
			inESC_DebugLog("-----[%s][%s][%d]  RE TRY  END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_SUCCESS);
		}

		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_ESC_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示<電子簽單上傳> */

		/*  這個裏設計成可以重新連線 */
		if (ginNEX_ESC_Connect_flag == _ESC_CONNECT_NOT_CONNECT_)
		{
			inESC_DebugLog("  Not Connect Switch HID  [%d] ",pobESCTran.srBRec.inHDTIndex);
			if(VS_SUCCESS != inNEXSYS_ESC_SwitchToESC_Host(pobESCTran.srBRec.inHDTIndex))
			{
				inDISP_DispLogAndWriteFlie("  ESC FU Aft Trans Upload ESC HD Switch *Error* Line[%d]",  __LINE__);
			}
			/* 通訊也要切到ESC主機(通訊內會直接loadHDT內的CommIndex會自動切過去) */

			/* 電子簽單連線中 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_ESC_CONNECTING_, 0, _COORDINATE_Y_LINE_8_4_);

			/* 連線 */
			if ((inRetVal = inFLOW_RunFunction(&pobESCTran, _COMM_START_)) != VS_SUCCESS)
			{
				ginNEX_ESC_Connect_flag = _ESC_CONNECT_NOT_CONNECT_;
				/* 通訊失敗‧‧‧‧ */
			}
			else
			{
				ginNEX_ESC_Connect_flag = _ESC_CONNECT_ALREADY_;
			}
		}
		else
		{
			/* 有連線 */
			inRetVal = VS_SUCCESS;
		}

		if (inRetVal == VS_SUCCESS)
		{
			/* 上傳A區失敗的電子簽名 ======================================================== */
			/*       失敗 = (1)有送出去，若主機拒絕就移掉Again區紀錄，存Fail區，其他則無任何動作
					   (2)沒送出去，無任何動作，下次重試
					 成功 = 移掉Again區紀錄
			*/
			/*TODO: 看能不能寫成可變動功能 */
			if(VS_SUCCESS != inFunc_Find_Specific_HDTindex(pobESCTran.srBRec.inHDTIndex, _HOST_NAME_CREDIT_FUBON_, &inHostIndex))
			{
				inDISP_DispLogAndWriteFlie("  ESC FU Aft Trans Upload Find HDT *Error* Line[%d]",  __LINE__);
			}

			inDISP_DispLogAndWriteFlie("  Connected Find The HID For Again Count HID[%d] Line[%d]", inHostIndex,  __LINE__);
			pobESCTran.srBRec.inHDTIndex = inHostIndex;
			
			if( inLoadHDTRec(pobESCTran.srBRec.inHDTIndex) == VS_ERROR)
			{
				inDISP_DispLogAndWriteFlie(" Connected Find The HID For Again Count Load HDT[%d] *Error* Line[%d]",pobESCTran.srBRec.inHDTIndex,__LINE__);
			}
			if( inLoadHDPTRec(pobESCTran.srBRec.inHDTIndex) == VS_ERROR)
			{
				inDISP_DispLogAndWriteFlie(" Connected Find The HID For Again Count Load Load HDPT[%d] *Error* Line[%d]",pobESCTran.srBRec.inHDTIndex,__LINE__);
			}

			while(1)
			{
				/* 初始化 */
				inAgain = 0;
				/* Again table的count */
				inAgain = inNEXSYS_ESC_Again_Table_Count(&pobESCTran);

				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_ESC_NOT_UPLOADED_YET_, 0, _COORDINATE_Y_LINE_8_4_);

				/* 若有次數則上傳 */
				if (inAgain > 0)
				{
					/* 目前設定只會上傳一筆 */
					if(inEscSendCount >= 1)
						break;
					
					inEscSendCount ++;
				}
				/* 沒有上傳筆數 跳出*/
				else
				{
					break;
				}

				/* 顯示主機別 */
				memset(szHostName, 0x00, sizeof(szHostName));
				if (inGetHostLabel(szHostName) != VS_SUCCESS)
				{
					inDISP_DispLogAndWriteFlie("-----[%s][%s][%d]  Get Host Label ERR  END -----",__FILE__, __FUNCTION__, __LINE__);
					return (VS_ERROR);
				}

				inDISP_EnglishFont(szHostName, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

				memset(szTemp, 0x00, sizeof(szTemp));
				sprintf(szTemp, "%d", inAgain);
				inDISP_ChineseFont(szTemp, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_);
				//inDISP_Wait(500);/* 等一秒，不然會看不到上傳狀態 */

				/* 這裡要跑上傳流程 : 連線沿用 收送 */
				inRetVal = inNEXSYS_ESC_ProcessOnline(&pobESCTran, _ESC_UPLOAD_IDLE_);

//				if (inRetVal == VS_ESCAPE || 
//					inRetVal == _NEXSYS_ESC_PACK_ERR_ || 
//					inRetVal == _NEXSYS_ESC_UNPACK_ERR_)	/* 重送如果回應失敗或資料分析失敗就改為紙本 */
//				{
//					
//				}
//				else if (inRetVal == VS_ERROR)	                /* 送電文失敗 */
//				{
//					inESC_DebugLog("  ESC AfterTrans  Error Status HID [%d] ",pobTran->srBRec.inHDTIndex);
//					ginNEX_ESC_Send_Cnt ++;
//					break;
//				}
//				else 
				if (inRetVal >= 0)
				{
					/* 更新ESC上傳成功筆數金額 */
					inDISP_DispLogAndWriteFlie("  ESC AftTrans Online [SUCCESS] HID = [%d] ",pobESCTran.srBRec.inHDTIndex);
					inDISP_DispLogAndWriteFlie("  InCode [%d]  OrgInv[%d] Line[%d]",pobESCTran.srBRec.inCode , pobESCTran.srBRec.lnOrgInvNum, __LINE__);

					if( inLoadHDTRec(pobESCTran.srBRec.inHDTIndex) == VS_ERROR)
					{
						inDISP_DispLogAndWriteFlie(" ESC AftTrans Online [SUCCESS] Load HDT[%d] *Error* Line[%d]",pobESCTran.srBRec.inHDTIndex,__LINE__);
					}

					if(VS_SUCCESS != inACCUM_UpdateFlow_ESC(&pobESCTran, _ESC_ACCUM_STATUS_UPLOADED_)){
						inDISP_DispLogAndWriteFlie("  ESC  AftTrans Update Accum Update *Error* Line[%d]", __LINE__);
					}
					
					/* 標示為已上傳 */
					inBATCH_GetTransRecord_By_Sqlite(&pobESCTran);
					pobESCTran.srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_UPLOADED_;
					inBATCH_Update_ESC_Uploaded_By_Sqlite(&pobESCTran);
					inNEXSYS_ESC_Delete_Again_Record_Most_TOP(&pobESCTran);
					
				}else
				{
					
					inDISP_DispLogAndWriteFlie("  ESC AftTrans Online Reslut = [%d] ", inRetVal);
					inDISP_DispLogAndWriteFlie("  ESC AftTrans Online Escape Bef Reload HID = [%d] ",pobESCTran.srBRec.inHDTIndex);
					inDISP_DispLogAndWriteFlie("  InCode [%d]  OrgInv[%d] Line[%d]",pobESCTran.srBRec.inCode , pobESCTran.srBRec.lnOrgInvNum, __LINE__);
					inLoadHDTRec(pobESCTran.srBRec.inHDTIndex);
					
					/* 因為未上傳轉紙本不能列圖檔，所以就砍簽名圖檔 20181225 [SAM]*/
					//inFunc_Delete_Signature(pobTran);
					
					while (1)
					{
						/* 電簽上傳失敗需先顯示提示畫面 20190214 [SAM] */
						inSIGN_Check_Esc_Upload_Fail_Confirm(&pobESCTran);
						inDISP_Clear_Area(1, _LINE_8_4_, 16, _LINE_8_8_, _ENGLISH_FONT_8X16_);
						
						/* 列印帳單中 */
						inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);
						
						pobESCTran.srBRec.inPrintOption = _PRT_MERCH_;
						
						inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(&pobESCTran);

						/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
						if (inRetVal != VS_SUCCESS)
						{
							continue;
						}
						else
						{
							/* 列印成功才跳開  */
							inFunc_Delete_Signature(&pobESCTran);
							break;
						}
					}
					
					/* 結帳帳條統計 */
					if(VS_SUCCESS != inACCUM_UpdateFlow_ESC(&pobESCTran, _ESC_ACCUM_STATUS_BYPASS_)){
						inDISP_DispLogAndWriteFlie("  ESC AftTrans Online  ByPass Accum Update *Error* Line[%d]", __LINE__);
					}
					
					/* 抓出資料 */
					if( VS_SUCCESS != inNEXSYS_ESC_Get_Again_Record_Most_TOP(&pobESCTran))
					{
						inDISP_DispLogAndWriteFlie("  ESC AftTrans Online  ByPass Get Again Rec Top *Error* Line[%d]", __LINE__);
					}
					/* 塞到Fail區 */

//					inNEXSYS_ESC_Insert_Fail_Record(pobTran);
					/* 刪掉again區的 */
					if(VS_SUCCESS != inNEXSYS_ESC_Delete_Again_Record_Most_TOP(&pobESCTran))
					{
						inDISP_DispLogAndWriteFlie("  ESC AftTrans Online  ByPass Del Again Top *Error* Line[%d]", __LINE__);
					}
					
					/* 成功就砍簽名圖檔 */
					/* 少更新狀態及砍簽名檔 20190905 [SAM] */
					inFunc_Delete_Signature(&pobESCTran);
					inBATCH_GetTransRecord_By_Sqlite(&pobESCTran);
					pobTran->srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_PAPER_;
					inBATCH_Update_ESC_Uploaded_By_Sqlite(&pobESCTran);
					inDISP_DispLogAndWriteFlie("  ESC AftTrans Online  Print Paper HID [%d]  Line[%d]",pobESCTran.srBRec.inHDTIndex ,__LINE__);
				
					break;
				}

			}
		}
		
		inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

		return (VS_SUCCESS);
	}
}




int inNEXSYS_ESC_ISOOnlineAnalyse(TRANSACTION_OBJECT *pobTran)
{
        return (VS_SUCCESS);
}

int inNEXSYS_ESC_ISOAdviceAnalyse(TRANSACTION_OBJECT *pobTran, unsigned char *uszTcUpload)
{
	return (VS_SUCCESS);
}


int inNEXSYS_ESC_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode)
{
	int     i, inSendCnt, inField, inCnt;
	int     inBitMap[_NEXSYS_ESC_MAX_BIT_MAP_CNT_ + 1];
	int     inRetVal, inISOFuncIndex = -1;
	char    szTemplate[40 + 1];
	char    szLogMessage[40 + 1];
	char    szCommMode[1 + 1];
	char    szEncryptMode[2 + 1];
	unsigned char   uszBCD[20 + 1];
	ISO_TYPE_NEXSYS_ESC_TABLE 	srISOFunc;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inSendCnt = 0;
	inField = 0;

	/* 加密模式，預設值 = 0。 ESC一定為軟加密 3DES */
	memset(szCommMode, 0x00, sizeof(szCommMode));
	inGetCommMode(szCommMode);

	inESC_DebugLog("  CommMode[%s]",szCommMode);

	if (memcmp(szCommMode, _COMM_MODEM_MODE_, 1) == 0)
			inISOFuncIndex = 0;
	else if (memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0)
	{
		memset(szEncryptMode, 0x00, sizeof(szEncryptMode));
		inGetEncryptMode(szEncryptMode);

		inESC_DebugLog("  szEncryptMode[%s]",szEncryptMode);

		if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_TSAM_, strlen(_NCCC_ENCRYPTION_TSAM_)) == 0)
		{
			inISOFuncIndex = 1;
		}
		else
		{
			inISOFuncIndex = 0;
		}
	}
	else if (memcmp(szCommMode, _COMM_GPRS_MODE_, 1) == 0)
	{
		memset(szEncryptMode, 0x00, sizeof(szEncryptMode));
		inGetEncryptMode(szEncryptMode);
		if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_TSAM_, strlen(_NCCC_ENCRYPTION_TSAM_)) == 0)
		{
			inISOFuncIndex = 1;
		}
		else
		{
			inISOFuncIndex = 0;
		}
	}
	else
	{
		inESC_DebugLog("  No such CommMode! szCommMode = %d", atoi(szCommMode));
		return (VS_ERROR); /* 防呆 */
	}

	/* 決定要執行第幾個 Function Index */
	inESC_DebugLog("  IosFunctionIndex[%d]",inISOFuncIndex);
	memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
	memcpy((char *) &srISOFunc, (char *) &srNEXSYS_ESC_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));
	memset((char *) inBitMap, 0x00, sizeof(inBitMap));

	/* 開始組 ISO 電文 */
	/* 轉換到ESC HOST */
	inNEXSYS_ESC_SwitchToESC_Host(pobTran->srBRec.inHDTIndex);
	/* Pack TPDU */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (inGetTPDU(szTemplate) != VS_SUCCESS)
	{
		inESC_DebugLog("  inGetTPDU() ERROR!");
		return (VS_ERROR);
	}

	memset(uszBCD, 0x00, sizeof(uszBCD));
	inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 5);
	memcpy((char *) &uszSendBuf[inSendCnt], (char *) &uszBCD[0], _NEXSYS_ESC_TPDU_SIZE_);
	inSendCnt += _NEXSYS_ESC_TPDU_SIZE_;

	/* Get Bit Map / Mesage Type / Processing Code */
	inRetVal = inNEXSYS_ESC_GetBitMapMessagegTypeField03(pobTran, &srISOFunc, inTxnCode, inBitMap, &uszSendBuf[inSendCnt]);
	if (inRetVal == VS_ERROR)
	{
		inESC_DebugLog("  inNEXSYS_ESC_GetBitMapMessagegTypeField03() ERROR!");
		return (VS_ERROR);
	}
	else
		inSendCnt += inRetVal; /* Bit Map 長度 */

	/* ESC組封包時，要使用原本Host的參數，例如batch Number */
	if( inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" inNEXSYS_ESC_PackISO Recover Load HDT[%d] *Error* Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
	}
	

	for (i = 0;; i++)
	{
		if (inBitMap[i] == 0)
			break;

		while (inBitMap[i] > srISOFunc.srPackISO[inField].inFieldNum)
		{
			inField++;
		}

		if (inBitMap[i] == srISOFunc.srPackISO[inField].inFieldNum)
		{
			inCnt = srISOFunc.srPackISO[inField].inISOLoad(pobTran, &uszSendBuf[inSendCnt]);
			if (inCnt <= 0)
			{
				memset(szLogMessage, 0x00, sizeof(szLogMessage));
				sprintf(szLogMessage, " ESC PackIso inField = %d Error", srISOFunc.srPackISO[inField].inFieldNum);
				inESC_DebugLog(szLogMessage);
				return (VS_ERROR);
			}
			else
				inSendCnt += inCnt;
		}
	}

	if (srISOFunc.inModifyPackData != _NEXSYS_ESC_NULL_TX_)
		srISOFunc.inModifyPackData(pobTran, uszSendBuf, &inSendCnt);

	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (inSendCnt);
}

int inNEXSYS_ESC_CheckUnPackField(int inField, ISO_FIELD_NEXSYS_ESC_TABLE *srCheckUnPackField)
{
        int 	i;

        for (i = 0; i < 64; i++)
        {
                if (srCheckUnPackField[i].inFieldNum == 0)
                        break;
                else if (srCheckUnPackField[i].inFieldNum == inField)
                {
                        return (VS_SUCCESS); /* i 是 ISO_FIELD_TABLE 裡的第幾個 */
                }
        }

        return (VS_ERROR);
}

int inNEXSYS_ESC_GetCheckField(int inField, ISO_CHECK_NEXSYS_ESC_TABLE *ISOFieldCheck)
{
        int 	i;

        for (i = 0; i < 64; i++)
        {
                if (ISOFieldCheck[i].inFieldNum == 0)
                        break;
                else if (ISOFieldCheck[i].inFieldNum == inField)
                {
                        return (i); /* i 是 ISO_CHECK_NEXSYS_ESC_TABLE 裡的第幾個 */
                }
        }

        return (VS_ERROR);
}

int inNEXSYS_ESC_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_NEXSYS_ESC_TABLE *srFieldType)
{
        int 	inCnt = 0, i, inLen;

        for (i = 0;; i++)
        {
                if (srFieldType[i].inFieldNum == 0)
                        break;

                if (srFieldType[i].inFieldNum != inField)
                        continue;

                switch (srFieldType[i].inFieldType)
                {
                        case _NEXSYS_ESC_ISO_ASC_:
                                inCnt += srFieldType[i].inFieldLen;
                                break;
                        case _NEXSYS_ESC_ISO_BCD_:
                                inCnt += srFieldType[i].inFieldLen / 2;
                                break;
                        case _NEXSYS_ESC_ISO_NIBBLE_2_:
                                inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
				/* Smart Pay卡號上傳格式(Field_35)不同一般信用卡。一般信用卡的卡號需要Pack成 BCD Code，Smart pay的卡號/帳號直接上傳ASCII Code不進行Pack。 */
				/* 因為reversal不讀batch，沒辦法知道fiscBit，所以還是只能強制用長度判斷 */
				if (srFieldType[i].inFieldNum == 35 && guszNEX_FiscBit == VS_TRUE)
					inCnt += inLen + 1;
				else
					inCnt += ((inLen + 1) / 2) + 1;
                                break;
                        case _NEXSYS_ESC_ISO_NIBBLE_3_:
                                inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
                                inCnt += ((inLen + 1) / 2) + 2;
                                break;
                        case _NEXSYS_ESC_ISO_BYTE_2_:
                                inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
                                inCnt += inLen + 1;
                                break;
                        case _NEXSYS_ESC_ISO_BYTE_3_:
                                inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
                                inCnt += inLen + 2;
                                break;
                        case _NEXSYS_ESC_ISO_BYTE_2_H_:
                                inLen = (int) uszSendData[0];
                                inCnt += inLen + 1;
                                break;
                        case _NEXSYS_ESC_ISO_BYTE_3_H_:
                                inLen = ((int) uszSendData[0] * 0xFF) + (int) uszSendData[1];
                                inCnt += inLen + 1;
                                break;
			case _NEXSYS_ESC_ISO_BYTE_1_:
				inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
				inCnt += inLen + 1;
				break;
                        default:
                                break;
                }

                break;
        }

        return (inCnt);
}

int inNEXSYS_ESC_GetFieldIndex(int inField, ISO_FIELD_TYPE_NEXSYS_ESC_TABLE *srFieldType)
{
	int 	i;

	for (i = 0 ;; i ++)
	{
		if (srFieldType[i].inFieldNum == 0)
			break;

		if (srFieldType[i].inFieldNum == inField)
			return (i);
	}

	return (VS_ERROR);
}

int inNEXSYS_ESC_UnPackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, unsigned char *uszRecvBuf)
{
	int	inRetVal;
	int	i, inRecvCnt, inCnt, inSendCnt, inSendField, inRecvField, inISOFuncIndex = -1;
	char	szBuf[_NEXSYS_ESC_TPDU_SIZE_ + _NEXSYS_ESC_MTI_SIZE_ + _NEXSYS_ESC_BIT_MAP_SIZE_ + 1];
	char	szCommMode[1 + 1];
	unsigned char uszSendMap[_NEXSYS_ESC_BIT_MAP_SIZE_ + 1], uszReceMap[_NEXSYS_ESC_BIT_MAP_SIZE_ + 1];
	ISO_TYPE_NEXSYS_ESC_TABLE 	srISOFunc;

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inSendField = inRecvField = 0;
	inSendCnt = inRecvCnt = 0;

	memset((char *) uszSendMap, 0x00, sizeof(uszSendMap));
	memset((char *) uszReceMap, 0x00, sizeof(uszReceMap));
	memset((char *) szBuf, 0x00, sizeof(szBuf));

	/* 加密模式，預設值 = 0。【0 = 3DES加密】 */
	memset(szCommMode, 0x00, sizeof(szCommMode));
	inGetCommMode(szCommMode);
	if (memcmp(szCommMode, _COMM_MODEM_MODE_, 1) == 0)
		inISOFuncIndex = 0;
	else if (memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0)
	{
		inISOFuncIndex = 0;
	}
	else if (memcmp(szCommMode, _COMM_GPRS_MODE_, 1) == 0)
	{
		inISOFuncIndex = 0;
	}
	else
	{
		inDISP_DispLogAndWriteFlie("  No suc! szCommMode [%d] Line[%d]", atoi(szCommMode), __LINE__);
		return (VS_ERROR); /* 防呆 */
	}

	memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
	memcpy((char *) &srISOFunc, (char *) &srNEXSYS_ESC_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

	if (srISOFunc.inCheckISOHeader != NULL)
	{
		inESC_DebugLog("  srISOFunc.inCheckISOHeader != NULL");

		if (srISOFunc.inCheckISOHeader(pobTran, (char *) &uszSendBuf[inRecvCnt], (char *) &uszRecvBuf[inRecvCnt]) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  inCheckISOHeader Error  Line[%d]",  __LINE__);
			return (_NEXSYS_ESC_UNPACK_ERR_);
		}
	}

        inSendCnt += _NEXSYS_ESC_TPDU_SIZE_;
        inRecvCnt += _NEXSYS_ESC_TPDU_SIZE_;
        inSendCnt += _NEXSYS_ESC_MTI_SIZE_;
        inRecvCnt += _NEXSYS_ESC_MTI_SIZE_;

        memcpy((char *) uszSendMap, (char *) &uszSendBuf[inSendCnt], _NEXSYS_ESC_BIT_MAP_SIZE_);
        memcpy((char *) uszReceMap, (char *) &uszRecvBuf[inRecvCnt], _NEXSYS_ESC_BIT_MAP_SIZE_);

        inSendCnt += _NEXSYS_ESC_BIT_MAP_SIZE_;
        inRecvCnt += _NEXSYS_ESC_BIT_MAP_SIZE_;

        /* 先檢查 ISO Field_39 */
        if (inNEXSYS_ESC_BitMapCheck(uszReceMap, 39) == VS_FALSE)
        {
		inDISP_DispLogAndWriteFlie("  ESC Check 39 *Error* Line[%d]",  __LINE__);
		return (VS_ERROR);
        }

        if (inNEXSYS_ESC_BitMapCheck(uszReceMap, 41) == VS_FALSE)
        {
		inDISP_DispLogAndWriteFlie("  ESC Check 41 *Error* Line[%d]",  __LINE__);
		return (VS_ERROR);
        }

        for (i = 1; i <= 64; i++)
        {
                /* 有送出去的 Field 但沒有收回來的 Field */
                if (inNEXSYS_ESC_BitMapCheck(uszSendMap, i) && !inNEXSYS_ESC_BitMapCheck(uszReceMap, i))
                {
                        inSendCnt += inNEXSYS_ESC_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
                }
                else if (inNEXSYS_ESC_BitMapCheck(uszReceMap, i)) /* 收的 BitMap */
                {
                        if (inNEXSYS_ESC_BitMapCheck(uszSendMap, i)) /* 送的 BitMap 都有 */
                        {
			inESC_DebugLog("  Check Data [%d] ",  i);
                                /* 是否要進行檢查封包資料包含【送】【收】 */
                                if ((inSendField = inNEXSYS_ESC_GetCheckField(i, srISOFunc.srCheckISO)) != VS_ERROR)
                                {
                                        if (srISOFunc.srCheckISO[inSendField].inISOCheck(pobTran, &uszSendBuf[inSendCnt], &uszRecvBuf[inRecvCnt]) != VS_SUCCESS)
                                        {
					inDISP_DispLogAndWriteFlie("  ESC inISOCheck *Error* inFieldNum [%d] inSendField[%d]", srISOFunc.srCheckISO[inSendField].inFieldNum, inSendField);

                                                return (_NEXSYS_ESC_UNPACK_ERR_);
                                        }
                                }

                                inSendCnt += inNEXSYS_ESC_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
                        }

                        while (i > srISOFunc.srUnPackISO[inRecvField].inFieldNum) /* 只有收 */
                        {
                                inRecvField++;
                        }

                        if (i == srISOFunc.srUnPackISO[inRecvField].inFieldNum)
                        {
                                /* 要先檢查 UnpackISO 是否存在 */
                                if (inNEXSYS_ESC_CheckUnPackField(i, srISOFunc.srUnPackISO) == VS_SUCCESS)
				{
                                        inRetVal = srISOFunc.srUnPackISO[inRecvField].inISOLoad(pobTran, &uszRecvBuf[inRecvCnt]);
					if (inRetVal != VS_SUCCESS)
					{
						inDISP_DispLogAndWriteFlie("  ESC srUnPackISO *Error* Fideld [%d] inRecvField[%d]", srISOFunc.srUnPackISO[inRecvField].inFieldNum, inRecvField);
						return (_TRAN_RESULT_UNPACK_ERR_);
					}

				}

                        }

                        inCnt = inNEXSYS_ESC_GetFieldLen(i, &uszRecvBuf[inRecvCnt], srISOFunc.srISOFieldType);
                        if (inCnt == VS_ERROR)
                        {
			inDISP_DispLogAndWriteFlie("  ESC GetFieldLen *Error* inCnt [%d] IsoType[%d]", inCnt, srISOFunc.srISOFieldType);
			return (_NEXSYS_ESC_UNPACK_ERR_);
                        }

                        inRecvCnt += inCnt;
                }
        }

	/* 這裡表示已經解完電文要檢查是否有回 ISO Field_38 */
	if (!memcmp(&pobTran->srBRec.szRespCode[0], "00", 2))
	{
		switch (pobTran->inISOTxnCode)
		{
			case _CUP_LOGON_:
				if (inNEXSYS_ESC_BitMapCheck(uszReceMap, 59) == VS_FALSE)
				{
					inDISP_DispLogAndWriteFlie("  ESC Check 59 *Error* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode,  __LINE__);
					return (VS_ERROR);
				}

				if (inNEXSYS_ESC_BitMapCheck(uszReceMap, 60) == VS_FALSE)
				{
					inDISP_DispLogAndWriteFlie("  ESC Check 60 *Error* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode,  __LINE__);
					return (VS_ERROR);
				}
				break;
			case _CUP_SALE_ :
			case _CUP_PRE_AUTH_ :
				if (inNEXSYS_ESC_BitMapCheck(uszReceMap, 04) == VS_FALSE)
				{
					inDISP_DispLogAndWriteFlie("  ESC Check 04 *Error* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode,  __LINE__);
					return (VS_ERROR);
				}

				if (inNEXSYS_ESC_BitMapCheck(uszReceMap, 59) == VS_FALSE)
				{
					inDISP_DispLogAndWriteFlie("  ESC Check 59 *Error* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode,  __LINE__);
					return (VS_ERROR);
				}
				break;

			case _CUP_REFUND_:
			case _CUP_PRE_COMP_:
			case _CUP_VOID_:
			case _CUP_PRE_AUTH_VOID_:
			case _CUP_PRE_COMP_VOID_:
			case _INST_SALE_:
			case _INST_ADJUST_:
			case _INST_REFUND_:
			case _REDEEM_SALE_:
			case _REDEEM_ADJUST_:
			case _REDEEM_REFUND_:
				if (inNEXSYS_ESC_BitMapCheck(uszReceMap, 04) == VS_FALSE)
				{
					inDISP_DispLogAndWriteFlie("  ESC Check 04 *Error* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode,  __LINE__);
					return (VS_ERROR);
				}

				if (inNEXSYS_ESC_BitMapCheck(uszReceMap, 38) == VS_FALSE)
				{
					inDISP_DispLogAndWriteFlie("  ESC Check 38 *Error* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode,  __LINE__);
					return (VS_ERROR);
				}

				if (inNEXSYS_ESC_BitMapCheck(uszReceMap, 59) == VS_FALSE)
				{
					inDISP_DispLogAndWriteFlie("  ESC Check 59 *Error* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode,  __LINE__);
					return (VS_ERROR);
				}

				break;

			case _SALE_:
			case _REFUND_:
			case _VOID_:
			case _TIP_:
			case _PRE_AUTH_:
			case _PRE_COMP_:
			case _MAIL_ORDER_:
			case _CASH_ADVANCE_:
				if (inNEXSYS_ESC_BitMapCheck(uszReceMap, 04) == VS_FALSE)
				{
					inDISP_DispLogAndWriteFlie("  ESC Check 04 *Error* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode,  __LINE__);
					return (VS_ERROR);
				}

				if (inNEXSYS_ESC_BitMapCheck(uszReceMap, 38) == VS_FALSE)
				{
					inDISP_DispLogAndWriteFlie("  ESC Check 38 *Error* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode,  __LINE__);
					return (VS_ERROR);
				}
				break;

			case _FISC_SALE_:
			case _FISC_VOID_:
			case _FISC_REFUND_:
				if (inNEXSYS_ESC_BitMapCheck(uszReceMap, 04) == VS_FALSE)
				{
					inDISP_DispLogAndWriteFlie("  ESC Check 04 *Error* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode,  __LINE__);
					return (VS_ERROR);
				}

				if (inNEXSYS_ESC_BitMapCheck(uszReceMap, 58) == VS_FALSE)
				{
					inDISP_DispLogAndWriteFlie("  ESC Check 58 *Error* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode,  __LINE__);
					return (VS_ERROR);
				}

				if (inNEXSYS_ESC_BitMapCheck(uszReceMap, 59) == VS_FALSE)
				{
					inDISP_DispLogAndWriteFlie("  ESC Check 59 *Error* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode,  __LINE__);
					return (VS_ERROR);
				}
				break;
			case _ESC_UPLOAD_:
				break;
			default:
				inDISP_DispLogAndWriteFlie("  ESC Check Default *Error* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode,  __LINE__);
				break;
		}
	}

	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
Function        :inNEXSYS_ESC_GetSTAN
Date&Time       :2016/5/10 上午 9:52
Describe        :這裡get是ESC主機的STAN
*/
int inNEXSYS_ESC_GetSTAN(TRANSACTION_OBJECT *pobTran)
{
	int	inESC_Hostindex;
        char 	szSTANNum[12 + 1];

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 這裡get是ESC主機的STAN */
	inNEXSYS_ESC_SwitchToESC_Host(pobTran->srBRec.inHDTIndex);
	if (inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_ESC_, &inESC_Hostindex) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	inLoadHDPTRec(inESC_Hostindex);

        memset(szSTANNum, 0x00, sizeof(szSTANNum));
        if (inGetSTANNum(szSTANNum) == VS_ERROR)
                return (VS_ERROR);

        pobTran->srBRec.lnSTANNum = atol(szSTANNum);

	/* 記得切回來 */
	if( inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" inNEXSYS_ESC_GetSTAN Recover Load HDT[%d] *Error* Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
	}
	if( inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" inNEXSYS_ESC_GetSTAN Recover Load HDPT[%d] *Error* Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
	}

	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

        return (VS_SUCCESS);
}

/*
Function        :inNEXSYS_ESC_SetSTAN
Date&Time       :2015/12/24 早上 10:25
Describe        :STAN++ 這裡加的是ESC主機的STAN
*/
int inNEXSYS_ESC_SetSTAN(TRANSACTION_OBJECT *pobTran)
{
	int	inESC_Hostindex;
	long 	lnSTAN;
	char 	szSTANNum[12 + 1];

	/* 這裡加的是ESC主機的STAN */
//	inNEXSYS_ESC_SwitchToESC_Host(pobTran->srBRec.inHDTIndex);
	
	if (inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_ESC_, &inESC_Hostindex) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC Set STAN Find HDT *Error* Name[%s] HDT[%d] Line[%d]",_HOST_NAME_ESC_, 
																		pobTran->srBRec.inHDTIndex, __LINE__);
		return (VS_ERROR);
	}
	
	if( inLoadHDTRec(inESC_Hostindex) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" inNEXSYS_ESC_SetSTAN Recover Load HDT[%d] *Error* Line[%d]",inESC_Hostindex,__LINE__);
	}
	if( inLoadHDPTRec(inESC_Hostindex) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" inNEXSYS_ESC_SetSTAN Recover Load HDPT[%d] *Error* Line[%d]",inESC_Hostindex,__LINE__);
	}
	

	memset(szSTANNum, 0x00, sizeof(szSTANNum));
	if (inGetSTANNum(szSTANNum) == VS_ERROR)
		return (VS_ERROR);

	lnSTAN = atol(szSTANNum);
	if (lnSTAN++ > 999999)
		lnSTAN = 1;

	memset(szSTANNum, 0x00, sizeof(szSTANNum));
	sprintf(szSTANNum, "%06ld", lnSTAN);
	if (inSetSTANNum(szSTANNum) == VS_ERROR)
			return (VS_ERROR);

	if (inSaveHDPTRec(inESC_Hostindex) < 0)
	{
		inDISP_DispLogAndWriteFlie("  ESC Set STAN Save HDT *Error* Name[%s] HDT[%d] Line[%d]",_HOST_NAME_ESC_, 
																	pobTran->srBRec.inHDTIndex, __LINE__);
		return (VS_ERROR);
	}

	/* 記得切回來 */
	if( inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" inNEXSYS_ESC_SetSTAN Recover Load HDT[%d] *Error* Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
	}
	if( inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" inNEXSYS_ESC_SetSTAN Recover Load HDPT[%d] *Error* Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
	}
	
	return (VS_SUCCESS);
}

int inNEXSYS_ESC_ProcessOnline(TRANSACTION_OBJECT *pobTran, int inType)
{
	int	i;
	int 	inRetVal;
	int inTempHostId = 0;

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inESC_DebugLog("  InType[%d] ", inType);

	/* 如果是Idel或Settle 就把之前again table資料讀出來 */
	if (inType == _ESC_UPLOAD_IDLE_ || inType == _ESC_UPLOAD_SETTLE_)
	{
		inESC_DebugLog("  Get Again Rec  ");
		inNEXSYS_ESC_Get_Again_Record_Most_TOP(pobTran);
	}

	/* 組簽單要原本的HDT */
	if (VS_ERROR == inLoadHDTRec(pobTran->srBRec.inHDTIndex))
	{
		inDISP_DispLogAndWriteFlie(" Esc Online Load HDPT *Error*  HID[%d] Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
	}
	
	/* 用來DEBUG 因為交易及存檔的HDT 要用原來交易的 */
	inTempHostId = pobTran->srBRec.inHDTIndex;
	
	/* 組上傳簽單 */
	/* 主要是在組簽單格式檔案，有需要時要參考聯合資訊再加入 20190328 [SAM]  */
//	if (inNEXSYS_ESC_Make_E1Data(pobTran) != VS_SUCCESS)
//	{
//		return (VS_ERROR);
//	}
	
	/* 【STEP 1】取得上傳資料(壓縮加密) ===========================================================
			A.簽單   : ESC_RECEIPT.txt (ESC_UPLOAD_RECEIPT)
			B.圖檔   : ESC_BMP.bmp     (ESC_UPLOAD_BMP)
			單次上傳Data limit : ESC_ISO_MAX_LEN
	*/

	/* 加密置換 - 裡面會使用不一樣的Host data */
	ginNEX_EscDataIndex = 0;
	if (inNEXSYS_ESC_Data_Compress_Encryption(pobTran) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	/* 【STEP 2】置入上傳結構 ============================================================== */
	for (i = 0; i < _ESC_LIMIT_; i++)
	{
		memset(&NEX_ESC_UPLOAD_DATA[i], 0x00, sizeof(NEX_ESC_UPLOAD_DATA[i]));
	}

	/* 以下會分解ESC_UPLOAD_RECEIPT and ESC_UPLOAD_BMP 到 ESC_UPLOAD_DATA並取得總上傳次數inEscDataIndex */
	/* 簽單 Append E1 */
	/* 主要是讀取已組完的簽單格式檔案再加入，有需要時要參考聯合資訊再加入 20190328 [SAM]  */
//	if (inNEXSYS_ESC_Data_Packet(_ESC_FILE_RECEIPT_GZ_ENCRYPTED_, _TABLE_E1_) != VS_SUCCESS)
//	{
//		return (VS_ERROR);
//	}

	/* 圖檔 Append E2 */
	/* 如果圖檔不存在就不塞 */
	if (inFILE_Check_Exist((unsigned char*)_ESC_FILE_BMP_GZ_ENCRYPTED_) == VS_SUCCESS)
	{
		if (inNEXSYS_ESC_Data_Packet(_ESC_FILE_BMP_GZ_ENCRYPTED_, _TABLE_E2_) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  ESC  Data Packet *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		}
	}else{
		inDISP_DispLogAndWriteFlie("  ESC Check File[%s] *Error* Line[%d]", _ESC_FILE_BMP_GZ_ENCRYPTED_, __LINE__);
	}

	/* 【STEP 3】開始上傳 ====================================================================== */
	/* 上傳到第幾個封包 */
	ginNEX_EscUploadIndex = 0;

	while(1)
	{
		/* 以防萬一，下兩個條件 1.inTableIndex不是E1 or E2 2.正在上傳的封包index大於全部需要上傳的封包數 */
		if ((NEX_ESC_UPLOAD_DATA[ginNEX_EscUploadIndex].inTableIndex != _TABLE_E1_ &&
			 NEX_ESC_UPLOAD_DATA[ginNEX_EscUploadIndex].inTableIndex != _TABLE_E2_) ||
			 (ginNEX_EscUploadIndex > ginNEX_EscDataIndex))
		{
			inDISP_DispLogAndWriteFlie("  ESC *Error* UP_DATA_INDEX[%d] ESC_DATA_INDEX[%d] Line[%d]",ginNEX_EscUploadIndex ,ginNEX_EscDataIndex, __LINE__);
			inDISP_DispLogAndWriteFlie("  ESC *Error* Table[%d] Line[%d]",NEX_ESC_UPLOAD_DATA[ginNEX_EscUploadIndex].inTableIndex, __LINE__);
			break;
		}

		/* 開始組交易封包，送、收、組、解 */
		pobTran->inISOTxnCode = _ESC_UPLOAD_;					/* 封包別一律為_ESC_UPLOAD_ */
		pobTran->inTransactionCode = pobTran->srBRec.inESCTransactionCode;

		/* 目前用同一個，所以先再覆蓋 */
		pobTran->inTransactionCode = _ESC_UPLOAD_;

		inESC_DebugLog("  BEF PACK REC TransCode[%d]  ", pobTran->inTransactionCode);
		inESC_DebugLog("  BEF PACK REC IsoCode[%d]  ", pobTran->inISOTxnCode);
		inESC_DebugLog("  BEF PACK REC InCode[%d]  ", pobTran->srBRec.inCode);
		
		if(pobTran->srBRec.inHDTIndex != inTempHostId )
		{
			inDISP_DispLogAndWriteFlie("  ESC Host *Error*  B_HDT[%d] TempHDT[%d] Line[%d]", pobTran->srBRec.inHDTIndex, inTempHostId, __LINE__);
		}
		
		inRetVal = inNEXSYS_ESC_SendPackRecvUnPack(pobTran);

		/* 成功或失敗 System_Trace_Number 都要加一 這裡加的是ESC主機的STAN */
		inNEXSYS_ESC_SetSTAN(pobTran);

		if (inRetVal != VS_SUCCESS)
		{
			ginNEX_ESC_Connect_flag = _ESC_CONNECT_NOT_CONNECT_;
			/* 不論上傳成功與否 都刪上傳用檔 */
			inFILE_Delete((unsigned char*)_ESC_FILE_RECEIPT_GZ_ENCRYPTED_);
			inFILE_Delete((unsigned char*)_ESC_FILE_BMP_GZ_ENCRYPTED_);

			inDISP_DispLogAndWriteFlie("  ESC SendPack *Error* inRetVal[%d] Line[%d]",inRetVal, __LINE__);
			return (inRetVal);
		}

		ginNEX_EscUploadIndex ++;
        }

	/* 不論上傳成功與否 都刪上傳用檔 */
	inFILE_Delete((unsigned char*)_ESC_FILE_RECEIPT_GZ_ENCRYPTED_);
	inFILE_Delete((unsigned char*)_ESC_FILE_BMP_GZ_ENCRYPTED_);

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

int inNEXSYS_ESC_AnalysePacket(TRANSACTION_OBJECT *pobTran)
{
        int				inRetVal = VS_SUCCESS, inISOFuncIndex = -1;
        ISO_TYPE_NEXSYS_ESC_TABLE 	srISOFunc;

        if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
        {
                if (pobTran->inTransactionResult == _NEXSYS_ESC_AUTHORIZED_ || pobTran->inTransactionResult == _NEXSYS_ESC_REFERRAL_ || pobTran->inTransactionResult == _NEXSYS_ESC_SETTLE_UPLOAD_BATCH_)
                {
                        /* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
                        inISOFuncIndex = 0; /* 不加密 */
                        memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
                        memcpy((char *) &srISOFunc, (char *) &srNEXSYS_ESC_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

                        if (srISOFunc.inOnAnalyse != NULL)
                                inRetVal = srISOFunc.inOnAnalyse(pobTran);
                }
                else
                {
                        inRetVal = VS_ERROR;
                }
        }

        return (inRetVal);
}

int inNEXSYS_ESC_CommSendRecvToHost(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket)
{
	int	inRetVal;
	int	inSendTimeout = 3;	
	int	inReceiveTimeout = 3;
	int	inReceiveSize = _COMM_RECEIVE_MAX_LENGTH_;		/* 配合Ingenico */
	char	szTimeOut[2 + 1] = {0};

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_SEND_, 0, _COORDINATE_Y_LINE_8_7_);

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 如果沒設定TimeOut，就用EDC.dat內的TimeOut */
	if (inSendTimeout <= 0)
	{
		memset(szTimeOut, 0x00, sizeof(szTimeOut));
		inGetIPSendTimeout(szTimeOut);
		inSendTimeout = atoi(szTimeOut);
	}

	if (ginDebug == VS_TRUE)	
		vdNEXSYS_ESC_ISO_FormatDebug_DISP(uszSendPacket, inSendLen);
#if 0
        if (ginDebug == VS_TRUE)
        {
                vdNEXSYS_ESC_ISO_FormatDebug_DISP(uszSendPacket, inSendLen);
        }

        if (ginISODebug == VS_TRUE)
        {
                vdNEXSYS_ESC_ISO_FormatDebug_PRINT(uszSendPacket, inSendLen);
        }
#endif
	if ((inRetVal = inCOMM_Send(uszSendPacket, inSendLen, inSendTimeout, VS_TRUE)) != VS_SUCCESS)
	{
		inESC_DebugLog("  inCOMM_Send() ERROR");
		return (VS_ERROR);
	}

        /* 這裡是用CPT.DAT的東西，所以一定要用 */
       /* 目前富邦規定是3秒，所以使用定義時的值 
	*如需要使用再打開  20181225 [SAM]*/
//	memset(szHostResponseTimeOut, 0x00, sizeof(szHostResponseTimeOut));
//	inRetVal = inGetHostResponseTimeOut(szHostResponseTimeOut);
//
//	if (inRetVal != VS_SUCCESS)
//		return (VS_ERROR);
//
//	inReceiveTimeout = atoi(szHostResponseTimeOut);

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_RECEIVE_, 0, _COORDINATE_Y_LINE_8_7_);

	if ((inRetVal = inCOMM_Receive(uszRecvPacket, inReceiveSize, inReceiveTimeout, VS_TRUE)) != VS_SUCCESS)
	{
		inESC_DebugLog("  inCOMM_Receive() ERROR");
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
		vdNEXSYS_ESC_ISO_FormatDebug_DISP(uszRecvPacket, inReceiveSize);
#if 0
        if (ginDebug == VS_TRUE)
        {
                vdNEXSYS_ESC_ISO_FormatDebug_DISP(uszRecvPacket, inReceiveSize);
        }

        if (ginISODebug == VS_TRUE)
        {
                vdNEXSYS_ESC_ISO_FormatDebug_PRINT(uszRecvPacket, inReceiveSize);
        }
#endif
        return (VS_SUCCESS);
}

int inNEXSYS_ESC_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran)
{
	int inSendCnt = 0, inRecvCnt = 0, inRetVal;
	unsigned char	uszSendPacket[_NEXSYS_ESC_ISO_SEND_ + 1], uszRecvPacket[_NEXSYS_ESC_ISO_RECV_ + 1];

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	memset(uszSendPacket, 0x00, sizeof(uszSendPacket));
	memset(uszRecvPacket, 0x00, sizeof(uszRecvPacket));

	/* 組 ISO 電文 */
	if ((inSendCnt = inNEXSYS_ESC_PackISO(pobTran, uszSendPacket, pobTran->inISOTxnCode)) <= 0)
	{
		inDISP_DispLogAndWriteFlie("  ESC PackISO *Error* inISOTxnCode[%d] Line[%d]",pobTran->inISOTxnCode,  __LINE__);
		return (_NEXSYS_ESC_PACK_ERR_); /* 組交易電文錯誤，不用組_REVERSAL_ */
	}

	/* 傳送及接收 ISO 電文 */
	if ((inRecvCnt = inNEXSYS_ESC_CommSendRecvToHost(pobTran, uszSendPacket, inSendCnt, uszRecvPacket)) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC SendRec *Error* inISOTxnCode[%d] uszOfflineBit[%d] Line[%d]",pobTran->inISOTxnCode, pobTran->srBRec.uszOfflineBit,  __LINE__);
		if (pobTran->srBRec.uszOfflineBit == VS_TRUE)
			memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));

		return (VS_ERROR);
	}


	/* 解 ISO 電文 */
	inRetVal = inNEXSYS_ESC_UnPackISO(pobTran, uszSendPacket, uszRecvPacket);
	if (inRetVal != VS_SUCCESS)
	{
		inRetVal = _NEXSYS_ESC_UNPACK_ERR_;
	}
	/* 上傳途中任一封包不接受 */
	else
	{
		if (memcmp(pobTran->srBRec.szRespCode, "00", 2))
		{
			/* 回覆碼不為"00"，表示有問題，先回拒絕 */
			inRetVal = VS_ESCAPE;
		}
	}
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

int inNEXSYS_ESC_DispHostResponseCode(TRANSACTION_OBJECT *pobTran)
{
	return (VS_SUCCESS);
}

int inNEXSYS_ESC_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRet;
	CTOS_RTC	srSetRTC; /* Date & Time */

	/* date */
	memcpy(&srSetRTC.bYear, &pobTran->srBRec.szDate[2], 2);
	memcpy(&srSetRTC.bMonth, &pobTran->srBRec.szDate[4], 2);
	memcpy(&srSetRTC.bDay, &pobTran->srBRec.szDate[6], 2);

	/* time */
	memcpy(&srSetRTC.bHour, &pobTran->srBRec.szTime[0], 2);
	memcpy(&srSetRTC.bMinute, &pobTran->srBRec.szTime[2], 2);
	memcpy(&srSetRTC.bSecond, &pobTran->srBRec.szTime[4], 2);

	usRet = CTOS_RTCSet(&srSetRTC);

	if (usRet != d_OK)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "時間同步失敗 錯誤代碼：%x", usRet);
		inDISP_LogPrintf(szDebugMsg);

		return (VS_ERROR);
	}

        return (VS_SUCCESS);
}

/*
Function        :inGetESC_Enable
Date&Time       :2016/4/15 下午 3:30
Describe        :傳HostEnable進來，會得到'Y' OR 'N'
*/
int inNEXSYS_ESC_GetESC_Enable(int inOrgHDTIndex, char *szHostEnable)
{
	/* 此function只用來查詢ESC是否開，不應該切換Host */
	/* 若之前已查詢到ESC的index為何，就可以直接使用 */
	if (ginNEX_ESCHostIndex != -1)
	{
		/* 直接使用 */
	}
	else
	{
		if (inFunc_Find_Specific_HDTindex(inOrgHDTIndex, _HOST_NAME_ESC_, &ginNEX_ESCHostIndex) != VS_SUCCESS)
		{
			/* 找不到直接return VS_ERROR */
			return (VS_ERROR);
		}
	}

	/* 如果Load失敗，回復原Host並回傳VS_ERROR(理論上不會有此情況) */
	if (inLoadHDTRec(ginNEX_ESCHostIndex) < 0)
	{
		if (inOrgHDTIndex >= 0)
		{
			/* 回覆原本的Host */
			inLoadHDTRec(inOrgHDTIndex);
		}
		else
		{
			/* load回信用卡主機 */
			inLoadHDTRec(0);
		}

		return (VS_ERROR);
	}

	/* 判斷HOST是否開啟  */
	memset(szHostEnable, 0x00, 1);
	inGetHostEnable(szHostEnable);
	if (szHostEnable[0] != 'Y')
	{
		inESC_DebugLog("  ESC Enable not open.");
	}
	else
	{
		inESC_DebugLog("  ESC Enable open.");

	}

	if (inOrgHDTIndex >= 0)
	{
		/* 回覆原本的Host */
		inLoadHDTRec(inOrgHDTIndex);
	}
	else
	{
		/* load回信用卡主機 */
		inLoadHDTRec(0);
	}

	return (VS_SUCCESS);
}

/*
Function        :inNEXSYS_ESC_SwitchToESC_Host
Date&Time       :2016/4/15 下午 3:30
Describe        :切換到HDT中ESC的REC上，之後要回覆就靠inOrgHDTIndex（在此function中回傳錯誤也會回覆原host）
 * 
*/
int inNEXSYS_ESC_SwitchToESC_Host(int inOrgHDTIndex)
{
	/* 開機後只找一次ESC */
	if (ginNEX_ESCHostIndex != -1)
	{
		/* 直接使用 */
	}
	else
	{
		if (inFunc_Find_Specific_HDTindex(inOrgHDTIndex, _HOST_NAME_ESC_, &ginNEX_ESCHostIndex) != VS_SUCCESS)
		{
			/* 找不到直接return VS_ERROR */
			return (VS_ERROR);
		}
	}

	/* 如果Load失敗，回復原Host並回傳VS_ERROR(理論上不會有此情況) */
	if (inLoadHDTRec(ginNEX_ESCHostIndex) < 0)
	{
		if (inOrgHDTIndex >= 0)
		{
			/* 回覆原本的Host */
			inLoadHDTRec(inOrgHDTIndex);
		}
		else
		{
			/* load回信用卡主機 */
			inLoadHDTRec(0);
		}

		inDISP_DispLogAndWriteFlie("  ESC Switch Host *Error*  EscHid[%d] Line[%d]", ginNEX_ESCHostIndex, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inNEXSYS_ESC_Data_Compress_Encryption
Date&Time       :2016/4/20 下午 4:40
Describe        :先壓縮再加密
 *		將ESC_E1.txt		轉成ESC_E1.txt.gz並加密
 *		將簽名圖檔多複製一份	轉成ESC_E2.bmp.gz並加密
*/
int inNEXSYS_ESC_Data_Compress_Encryption(TRANSACTION_OBJECT *pobTran)
{

	int	 inFileSize, inTempEncLen;
//	char	szData[_FILE_DATA_LENGTH_MAX_ + 1] = {0};		/* 注意大小 */
//	char	szEncryData[_FILE_DATA_LENGTH_MAX_ + 1] = {0};		/* 注意大小 */
	char *szData, *szEncryData;
	char	szFileName[16 + 1] = {0};
	char	szFileNameGzBmp[16 + 1] = {0};
	char	szTemplate[40 + 1] = {0};
	unsigned long	ulHandle = 0;
	int inCnt = 0, inRetVal;
	char szKeyTemp[32];
//	unsigned char	uszSlot = 0;

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	inESC_DebugLog("  Begin Host [%d] ",pobTran->srBRec.inHDTIndex);
	/* 抓tSAM Slot */
	/* 有重覆，因為沒裝也不會失敗....怪怪的，要重看 */
	/* 如果需要SAM CARD 加密, 需參考聯合程式 20190318 [SAM]*/
//	inRetVal = inNCCC_tSAM_Decide_tSAM_Slot(&uszSlot);
//	if (inRetVal != VS_SUCCESS)
//	{
//		inESC_DebugLog("inNCCC_tSAM_Decide_tSAM_Slot Failed");
//	}

	/* 預防漏砍，導致多送E2，這裡再預先砍一次 */
	inFILE_Delete((unsigned char*)_ESC_FILE_RECEIPT_GZ_ENCRYPTED_);
	inFILE_Delete((unsigned char*)_ESC_FILE_BMP_GZ_ENCRYPTED_);

	/* 壓縮加密檔案(GZIP) */
	/* 簽單 */
	/* 目前不組單資料 E1, 有需要再打開，需參考聯合程式 20190328 [SAM] */
#if 0
	if (inFunc_GZip_Data("", _ESC_FILE_RECEIPT_, _FS_DATA_PATH_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
#endif

	
	
#ifdef __CTOS_ENCRYPT_FUNC__	
	/* 加密並置換資料內容(軟加密) */
	/* 寫入當筆要用的key */
	if (inNCCC_TMK_Write_ESCKey(pobTran) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
#else

	memset(szKeyTemp, 0x00,sizeof(szKeyTemp));

	/* 卡號前4碼 */
	memcpy(&szKeyTemp[inCnt], &pobTran->srBRec.szPAN[0], 4);
	inCnt += 4;
	inDISP_LogPrintfWithFlag(" ESCKEY PAN[%s]", pobTran->srBRec.szPAN);

	/* 1299 */
	memcpy(&szKeyTemp[inCnt], "1299", 4);
	inCnt += 4;
	inDISP_LogPrintfWithFlag(" ESCKEY HARD CODE[%s]", "1299");

	/* TID */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);
	memcpy(&szKeyTemp[inCnt], szTemplate, 8);
	inCnt += 8;
	inDISP_LogPrintfWithFlag(" ESCKEY BUF [%s]", szKeyTemp);
		
#endif	
	/* 目前不組單資料 E1及SAM CARD 加密功能, 有需要再打開，需參考聯合程式 20190328 [SAM] */
#if 0
	/* 簽單 */
	if (inFILE_Open(&ulHandle, (unsigned char*)_ESC_FILE_RECEIPT_GZ_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 取得前17到32 Bytes (szData為前32Byte) */
	memset(szRawData32, 0x00, sizeof(szRawData32));
	if (inFILE_Read(&ulHandle, (unsigned char*)szRawData32, 32) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	memset(szRawData17_32, 0x00, sizeof(szRawData17_32));
	memcpy(szRawData17_32, &szRawData32[16], 16);
	inLen = 32;	/* 32 nibble , 16Bytes */

	/* DEBUG */
	memset(szAscii, 0x00, sizeof(szAscii));
	inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szRawData17_32, inLen);
	memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
	inESC_DebugLog("F_35 [szEncrypt ]");
	sprintf(szDebugMsg, "%s L:%d", szAscii, inLen);
	inESC_DebugLog(szDebugMsg);

	/* 加密 */
	/* tSAM加密用SAM加密，其餘用3DES */
	memset(szEncryptionMode, 0x00, sizeof(szEncryptionMode));
	inGetEncryptMode(szEncryptionMode);
	if (memcmp(szEncryptionMode, _NCCC_ENCRYPTION_TSAM_, strlen(_NCCC_ENCRYPTION_TSAM_)) == 0)
	{
		/* 開始加密 */
		/* 組 Field_37 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, pobTran->srBRec.szRefNo, 11);
		inFunc_PAD_ASCII(szTemplate, szTemplate, 'F', 16, _PAD_LEFT_FILL_RIGHT_); /* 補【F】 */
		memset(szField37, 0x00, sizeof(szField37));
		inFunc_ASCII_to_BCD((unsigned char*)szField37, szTemplate, 8);


		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szField37, 8);
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "[field37: %s] %d", szAscii, 8);
		inESC_DebugLog(szDebugMsg);


		/* 組 Field_41 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTerminalID(szTemplate);
		memset(szField41, 0x00, sizeof(szField41));
		inFunc_ASCII_to_BCD((unsigned char*)szField41, szTemplate, 4);

		/* DEBUG */
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szField41, 4);
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "[field41: %s] %d", szAscii, 4);
		inESC_DebugLog(szDebugMsg);


		inRetVal = inNCCC_tSAM_Encrypt(uszSlot,
						   inLen,
						   szRawData17_32,
						   (unsigned char*)&szField37,
						   (unsigned char*)&szField41,
						   &guszNEX_tSAMKeyIndex_ESC,
						   guszNEX_tSAMCheckSum_ESC57_E1);

		if (inRetVal == VS_ERROR)
			return (VS_ERROR);
		else
		{
			memset(szEncryData, 0x00, sizeof(szEncryData));
			memcpy(szEncryData, szRawData17_32, inRetVal);
		}

	}
	else
	{
		memset(szEncryData, 0x00, sizeof(szEncryData));
		if (inNCCC_TMK_ESC_3DES_Encrypt(szRawData17_32, 16, szEncryData) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
	}
	/* 取得加密後16 Bytes並塞回去 */
	memcpy(&szRawData32[16], szEncryData, 16);

	if (inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	if (inFILE_Write(&ulHandle, (unsigned char*)szRawData32, 32) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	if (inFILE_Rename((unsigned char*)_ESC_FILE_RECEIPT_GZ_, (unsigned char*)_ESC_FILE_RECEIPT_GZ_ENCRYPTED_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
#endif
	
	/* 壓縮加密檔案(GZIP) */
	/* 圖檔 */
	/* 藉由TRT_FileName比對來組出bmp的檔名 */
	inESC_DebugLog("  ReLoad Host [%d] ",pobTran->srBRec.inHDTIndex);
	if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Compress LoadHDPT Err Id[%d]  Line[%d]",pobTran->srBRec.inHDTIndex, __LINE__);
		return (VS_ERROR);
	}

	memset(szFileName, 0x00, sizeof(szFileName));
	/* 因為用invoice所以不用inFunc_ComposeFileName */
	inFunc_ComposeFileName_InvoiceNumber(pobTran, szFileName, _PICTURE_FILE_EXTENSION_, 6);
	
	inDISP_DispLogAndWriteFlie("  ESC Enc Fn[%s] Line[%d]",szFileName, __LINE__);
	
	/* 如果檔案不存在，就不壓 */
	if (inFILE_Check_Exist((unsigned char*)szFileName) == VS_SUCCESS)
	{
		/* 備份起來，因為Gzip會不留原檔 */
		inFunc_Copy_Data(szFileName, _FS_DATA_PATH_, _ESC_FILE_BMP_BACKUP_, _FS_DATA_PATH_);
		inESC_DebugLog("  Aft Copy Backup BMP F_NAME [%s] ",szFileName );

		if (inFunc_GZip_Data("", szFileName, _FS_DATA_PATH_) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Compress GZip *Error* FileName[%s]  Line[%d]",szFileName, __LINE__);
			return (VS_ERROR);
		}

		inESC_DebugLog("  Aft Gzip  BMP F_NAME [%s] ",szFileName );
		/* 還原 */
		inFunc_Rename_Data(_ESC_FILE_BMP_BACKUP_, _FS_DATA_PATH_, szFileName, _FS_DATA_PATH_);

		inESC_DebugLog("  Aft Rename  BMP F_NAME [%s] ",szFileName );

		memset(szFileNameGzBmp, 0x00, sizeof(szFileNameGzBmp));
		sprintf(szFileNameGzBmp, "%s%s", szFileName, _GZIP_FILE_EXTENSION_);

		inESC_DebugLog("  Gz BMP F_NAME [%s] ",szFileNameGzBmp );

		/* 當筆的一律用_ESC_UPLOAD_BMP_GZ_ */
		if (inFILE_Rename((unsigned char*)szFileNameGzBmp, (unsigned char*)_ESC_FILE_BMP_GZ_) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Compress Reame Gz *Error*  Line[%d]",  __LINE__);
			return (VS_ERROR);
		}

		/* 確認加密用 */
		inFunc_Copy_Data(_ESC_FILE_BMP_GZ_, _FS_DATA_PATH_, "", _SD_PATH_);

		/* 加密並置換資料內容(軟加密) */
		if (inFILE_Open(&ulHandle, (unsigned char*)_ESC_FILE_BMP_GZ_) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Compress File Open *Error* ulHandle[%lu]  Line[%d]",ulHandle, __LINE__);
			return (VS_ERROR);
		}

		inFileSize = lnFILE_GetSize(&ulHandle, (unsigned char *)_ESC_FILE_BMP_GZ_);

		if (inFileSize == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie("  Compress Get Size *Error* inFileSize[%d]  Line[%d]",inFileSize, __LINE__);
			/* GetSize失敗 ，關檔 */
			inFILE_Close(&ulHandle);
			return (VS_ERROR);
		}

		inESC_DebugLog("  GZ inFileSize = [%d] ", inFileSize);


		if (inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Compress Seek 0 *Error*  Line[%d]", __LINE__);
			inFILE_Close(&ulHandle);
			return (VS_ERROR);
		}
		
		szData = malloc(inFileSize + 1);

		memset(szData, 0x00, sizeof(szData));
		
		if (inFILE_Read(&ulHandle, (unsigned char*)szData, inFileSize) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Compress File Read *Error* ulHandle[%lu] Line[%d]",ulHandle,  __LINE__);
			inFILE_Close(&ulHandle);
			free(szData);
			return (VS_ERROR);
		}

		inESC_DebugLog("  GZ CLEAR DATA DISP START----- ");
		inESC_DisplayData(szData, inFileSize);
		inESC_DebugLog("  GZ CLEAR DATA DISP END----- ");


		/* 加密 */
		/* tSAM加密用SAM加密，其餘用3DES */
		/* 目前不用 SAM CARD 加密功能, 有需要再打開，需參考聯合程式 20190328 [SAM] */
#if 0
		memset(szEncryptionMode, 0x00, sizeof(szEncryptionMode));
		inGetEncryptMode(szEncryptionMode);
		if (memcmp(szEncryptionMode, _NCCC_ENCRYPTION_TSAM_, strlen(_NCCC_ENCRYPTION_TSAM_)) == 0)
		{
			/* 開始加密 */
			/* 組 Field_37 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, pobTran->srBRec.szRefNo, 11);
			inFunc_PAD_ASCII(szTemplate, szTemplate, 'F', 16, _PAD_LEFT_FILL_RIGHT_); /* 補【F】 */
			memset(szField37, 0x00, sizeof(szField37));
			inFunc_ASCII_to_BCD((unsigned char*)szField37, szTemplate, 8);


			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szField37, 8);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "[field37: %s] %d", szAscii, 8);
			inESC_DebugLog(szDebugMsg);


			/* 組 Field_41 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetTerminalID(szTemplate);
			memset(szField41, 0x00, sizeof(szField41));
			inFunc_ASCII_to_BCD((unsigned char*)szField41, szTemplate, 4);


			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szField41, 4);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "[field41: %s] %d", szAscii, 4);
			inESC_DebugLog(szDebugMsg);


			inRetVal = inNCCC_tSAM_Encrypt(uszSlot,
							   inLen,
							   szRawData17_32,
							   (unsigned char*)&szField37,
							   (unsigned char*)&szField41,
							   &guszNEX_tSAMKeyIndex_ESC,
							   guszNEX_tSAMCheckSum_ESC57_E2);

			if (inRetVal == VS_ERROR)
				return (VS_ERROR);
			else
			{
				memset(szEncryData, 0x00, sizeof(szEncryData));
				memcpy(szEncryData, szRawData17_32, inRetVal);
			}

		}
		else
#endif
		{
			szEncryData = malloc(inFileSize + 1);
			
			/* 不足8的倍數就不加密, 但需要以明文補回個數 */
			memset(szEncryData, 0x00, sizeof(szEncryData));
			inTempEncLen = inFileSize % 8;
			inESC_DebugLog(" Bf TempLen[%d] FileSize[%d] ",inTempEncLen, inFileSize);
			
			if(inTempEncLen != 0)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inFileSize -= inTempEncLen;
				memcpy(szTemplate, &szData[inFileSize], inTempEncLen);
			}

			inESC_DebugLog("  Af TempLen[%d] FileSize[%d] ",inTempEncLen, inFileSize);
#ifdef __CTOS_ENCRYPT_FUNC__
			
			
			if (inNCCC_TMK_ESC_3DES_Encrypt(szData, inFileSize, szEncryData) != VS_SUCCESS)
			{
				inESC_DebugLog("  TMK_ESC Enc Error ");
				free(szData);
				free(szEncryData);
				return (VS_ERROR);
			}
#else
			inRetVal = TDES_2LenKey_ECB_Encrypt((BYTE *)szKeyTemp, (BYTE *)szData, inFileSize, (BYTE *)szEncryData);
			
			inESC_DebugLog("  Encrypt Size Is [%d] ",inRetVal);
#endif

			if(inTempEncLen != 0)
			{
				memcpy(&szEncryData[inFileSize], szTemplate, inTempEncLen);
				/* 因為補的資料長度要加回去*/
				inFileSize += inTempEncLen;
			}
			
			inESC_DebugLog(" Fnl TempLen[%d] FileSize[%d] ",inTempEncLen, inFileSize);
			
			inESC_DebugLog("  GZ ENC DATA DISP START----- ");
			inESC_DisplayData(szEncryData, inFileSize);
			inESC_DebugLog("  GZENC DATA DISP END----- ");
		}

		if (inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Compress Seek 0 *Error*  Line[%d]", __LINE__);
			free(szData);
			free(szEncryData);
			inFILE_Close(&ulHandle);
			return (VS_ERROR);
		}
		
		if (inFILE_Write(&ulHandle, (unsigned char*)szEncryData, inFileSize) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Compress File Write *Error* ulHandle[%lu] Line[%d]",ulHandle,  __LINE__);
			free(szData);
			free(szEncryData);
			inFILE_Close(&ulHandle);
			return (VS_ERROR);
		}

		if (inFILE_Rename((unsigned char*)_ESC_FILE_BMP_GZ_, (unsigned char*)_ESC_FILE_BMP_GZ_ENCRYPTED_) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Compress Reame Gz *Error* Line[%d]", __LINE__);
			free(szData);
			free(szEncryData);
			inFILE_Close(&ulHandle);
			return (VS_ERROR);
		}

	}else /* 圖檔壓縮END*/
	{
		inDISP_DispLogAndWriteFlie("  Compress File Not Exist *Error* FileName[%s]  Line[%d]",szFileName, __LINE__);
	}
	/* 圖檔 */
	/* 確認加密用 */
	inFunc_Copy_Data(_ESC_FILE_RECEIPT_GZ_ENCRYPTED_, _FS_DATA_PATH_, "", _SD_PATH_);
	inFunc_Copy_Data(_ESC_FILE_BMP_GZ_ENCRYPTED_, _FS_DATA_PATH_, "", _SD_PATH_);

	inFILE_Close(&ulHandle);
	free(szData);
	free(szEncryData);
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
Function        :inNEXSYS_ESC_Data_Packet
Date&Time       :2016/4/21 下午 4:41
Describe        :將壓縮加密完的data切塊並塞進ESC_DATA中
*/
int inNEXSYS_ESC_Data_Packet(char *szFileName, int inTableIndex)
{
	int		i;
	int		inRetVal;
	int		inTableStart;			/* 當切完E1後，E2要從裡開始接 */
	int		inTotalCnt;			/* 總封包數 */
	long		lnFileSize, lnRemainSize;	/* 檔案長度、 剩餘長度 */
	unsigned long	ulHandle = 0;

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* E2從哪裡繼續append */
	/* 目前不組單資料 E1, 有需要再打開，需參考聯合程式 20190328 [SAM] */
#if 0
	NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].inTableIndex = inTableIndex;
	NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].inTotalPacketCnt = 1;
	NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].inPacketCnt = 1;
	NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].lnTotalPacketSize = 9;
	NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].inPacketSize = 9;
	memcpy(NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].szPackData,"\x31\x32\x33\x34\x35\x36\x37\x38\x39", 9);
	ginNEX_EscDataIndex ++;
#else
	inTableStart = ginNEX_EscDataIndex;

	inRetVal = inFILE_OpenReadOnly(&ulHandle, (unsigned char*)szFileName);

	if (inRetVal == VS_SUCCESS)
	{
		lnFileSize = lnFILE_GetSize(&ulHandle, (unsigned char*)szFileName);
		/* 設定剩餘長度 */
		lnRemainSize = lnFileSize;

		inESC_DebugLog(" EscPack Name[%s] size [%ld]", szFileName, lnFileSize);

		/* 移到檔案最前面 */
		inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_);

		/* E1 or E2的總封包數 */
		/* 因為整除還是會有1，所以不能再加1個 Count 2019/10/8 [SAM] */
		if(lnFileSize == _ESC_ISO_MAX_LEN_ )
			inTotalCnt = 1;
		else
			inTotalCnt = (lnFileSize / _ESC_ISO_MAX_LEN_) + 1;
		
		/* i表示第幾個封包 */
		for (i = 1 ;; i++)
		{

			NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].inTableIndex = inTableIndex;
			NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].inTotalPacketCnt = inTotalCnt;
			NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].inPacketCnt = i;
			NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].lnTotalPacketSize = lnFileSize;

			/* 清空 */
			memset(NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].szPackData, 0x00, sizeof(NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].szPackData));

			/* 如果剩餘長度小於ESC_ISO_MAX_LEN，就只讀剩下的 */
			if ((lnRemainSize - _ESC_ISO_MAX_LEN_) <= 0)
			{
				/* 讀出長度並設定structure中的封包長度 */
				if (inFILE_Read(&ulHandle, (unsigned char*)NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].szPackData, lnRemainSize) != VS_SUCCESS)
				{
					inDISP_DispLogAndWriteFlie("  ESC DATA  Read *Error* EscDataIndex[%d] Line[%d] ", ginNEX_EscDataIndex, __LINE__);
					inDISP_DispLogAndWriteFlie("  ESC DATA  Read *Error* lnRemainSize[%ld] lnFileSize[%ld] Line[%d] ", lnRemainSize, lnFileSize, __LINE__);
					return (VS_ERROR);
				}
				NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].inPacketSize = lnRemainSize;

				/* [DEUBG] */
				inESC_DebugLog("ESC Table[%d] TolPackCnt[%d] PackCnt[%d] TotalPackSz[%ld] PackSz[%d]", NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].inTableIndex,
								NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].inTotalPacketCnt,
								NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].inPacketCnt,
								NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].lnTotalPacketSize,
								NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].inPacketSize);
				/* [DEBUG] */
//				for (inTempCnt = 0 ; inTempCnt < lnRemainSize ; inTempCnt ++)
//				{
//					inESC_DebugLog("[%02x]",NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].szPackData[inTempCnt]);
//
//				}

				/* 總封包數(E1 + E2)加一 */
				ginNEX_EscDataIndex ++;

				/* 減去讀出長度 */
				lnRemainSize -= lnRemainSize;

				break;
			}
			else
			{
				/* 讀出長度並設定structure中的封包長度 */
				if (inFILE_Read(&ulHandle, (unsigned char*)NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].szPackData, _ESC_ISO_MAX_LEN_) != VS_SUCCESS)
				{
					inDISP_DispLogAndWriteFlie("  ESC DATA  Read *Error* EscDataIndex[%d] Line[%d] ", ginNEX_EscDataIndex, __LINE__);
					inDISP_DispLogAndWriteFlie("  ESC DATA  Read *Error* lnRemainSize[%ld] lnFileSize[%ld] Line[%d] ", lnRemainSize, lnFileSize, __LINE__);
					return (VS_ERROR);
				}
				NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].inPacketSize = _ESC_ISO_MAX_LEN_;

				/* [DEBUG] */
				inESC_DebugLog(" ESC2 Table[%d] TolPackCnt[%d] PackCnt[%d] TotalPackSz[%ld] PackSz[%d]", NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].inTableIndex,
								NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].inTotalPacketCnt,
								NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].inPacketCnt,
								NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].lnTotalPacketSize,
								NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].inPacketSize);
				/* [DEBUG] */
//				for (inTempCnt = 0 ; inTempCnt < lnRemainSize ; inTempCnt ++)
//				{
//					inESC_DebugLog("[%x]",NEX_ESC_UPLOAD_DATA[ginNEX_EscDataIndex].szPackData[inTempCnt]);
//					if(i > 20 )
//						inESC_DebugLog("\n");
//				}


				/* 總封包數(E1 + E2)加一 */
				ginNEX_EscDataIndex ++;

				/* 減去讀出長度 */
				lnRemainSize -= _ESC_ISO_MAX_LEN_;
			}

		}
		/* 關檔 */
		inFILE_Close(&ulHandle);
	}
	else
	{
		inDISP_DispLogAndWriteFlie("  ESC No DATA To Pack *Error* Fnm[%s] Line[%d] ", szFileName, __LINE__);
		/* 關檔 */
		inFILE_Close(&ulHandle);

		return (VS_ERROR);
	}
#endif
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
Function        :inNEXSYS_ESC_Insert_Again_Record
Date&Time       :2017/3/23 上午 10:12
Describe        :
*/
int inNEXSYS_ESC_Insert_Again_Record(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	char	szTableID[12 + 1] = {0};

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC Insert Again Rec *Error* Ret[%d]",inRetVal);
		inFunc_EDCLock();
		return (inRetVal);
	}
	
	inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_EMV_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC Emv Insert Again Rec *Error* Ret[%d]",inRetVal);
		inFunc_EDCLock();
		return (inRetVal);
	}
	

	inRetVal = inBATCH_Insert_All_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC Batch Insert All Flow *Error* Ret[%d]",inRetVal);
		inFunc_EDCLock();
		return (inRetVal);
	}
	
	memset(szTableID, 0x00, sizeof(szTableID));
	/* 抓出TableID */
	inRetVal = inSqlite_Get_Max_TableID_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_, szTableID);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC Update Rec Get Max Table Id *Error* Line[%d]",  __LINE__);
		inFunc_EDCLock();
	}
	else
	{
		inESC_DebugLog("inTableID : %d", atoi(szTableID));
		pobTran->inTableID = atoi(szTableID);
	}
	
	
	inRetVal = inBATCH_Insert_All_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_EMV_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC Emv Batch Insert All Flow *Error* Ret[%d]",inRetVal);
		inFunc_EDCLock();
		return (inRetVal);
	}

	/* 強制同步
	 * Linux 系統中，為了加快資料的讀取速度，寫入硬盤的資料有的時候為了效能，會寫到 filesystem buffer 中，
	 * 這個 buffer 是一塊記憶體空間，如果欲寫入硬碟的資料存此 buffer 中，而系統又突然斷電的話，那資料就會流失!
	 * 可以透sync 指令，將存 buffer 中的資料強制寫入disk中；
	 * 或是在程式中利用fsync() function
	 * 虹堡文件有提到，這個指令會減少NAND壽命，但為了不掉帳還是要用
	 *
	 */
	sync();
	
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (inRetVal);
}

/*
Function        :inNEXSYS_ESC_Insert_Fail_Record
Date&Time       :2017/3/23 上午 10:12
Describe        :
*/
int inNEXSYS_ESC_Insert_Fail_Record(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	char	szTableID[12 + 1] = {0};

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC Create Batch Table Err [%d]", inRetVal);
		inFunc_EDCLock();
		return (inRetVal);
	}
	
	inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_EMV_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC Emv Create Batch Table Err [%d]", inRetVal);
		inFunc_EDCLock();
		return (inRetVal);
	}

	inRetVal = inBATCH_Insert_All_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_);
	if (inRetVal != VS_SUCCESS)
	{
		inESC_DebugLog("  ESC Insert Data Err [%d] ", inRetVal);
		inFunc_EDCLock();
		return (inRetVal);
	}
	
	memset(szTableID, 0x00, sizeof(szTableID));
	/* 抓出TableID */
	inRetVal = inSqlite_Get_Max_TableID_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_, szTableID);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Update Rec Get Max Table Id *Error* Line[%d]",  __LINE__);
		inFunc_EDCLock();
	}
	else
	{
		inESC_DebugLog("inTableID : %d", atoi(szTableID));
		pobTran->inTableID = atoi(szTableID);
	}
	
	
	inRetVal = inBATCH_Insert_All_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_EMV_);
	if (inRetVal != VS_SUCCESS)
	{
		inESC_DebugLog("  ESC Emv Insert Data Err [%d] ", inRetVal);
		inFunc_EDCLock();
		return (inRetVal);
	}

	/* 強制同步
	 * Linux 系統中，為了加快資料的讀取速度，寫入硬盤的資料有的時候為了效能，會寫到 filesystem buffer 中，
	 * 這個 buffer 是一塊記憶體空間，如果欲寫入硬碟的資料存此 buffer 中，而系統又突然斷電的話，那資料就會流失!
	 * 可以透sync 指令，將存 buffer 中的資料強制寫入disk中；
	 * 或是在程式中利用fsync() function
	 * 虹堡文件有提到，這個指令會減少NAND壽命，但為了不掉帳還是要用
	 *
	 */
	sync();
	
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (inRetVal);
}

/*
Function        :inNEXSYS_ESC_Get_Again_Record_Most_TOP
Date&Time       :2017/3/23 上午 10:45
Describe        :取ESC資料庫中最上面一筆
*/
int inNEXSYS_ESC_Get_Again_Record_Most_TOP(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = inSqlite_ESC_Get_BRec_Top_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC Get Again Rec Top *Error* Retval[%d] Line[%d]",inRetVal ,__LINE__);
		return (inRetVal);
	}
	
	inRetVal = inSqlite_ESC_Get_EMVRec_Top_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_EMV_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC Emv Get Again Rec Top *Error* Retval[%d] Line[%d]",inRetVal ,__LINE__);
		return (inRetVal);
	}

	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (inRetVal);
}

/*
Function        :inNEXSYS_ESC_Get_Fail_Record_Most_TOP
Date&Time       :2017/3/23 下午 5:24
Describe        :取ESC資料庫中最上面一筆
*/
int inNEXSYS_ESC_Get_Fail_Record_Most_TOP(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = inSqlite_ESC_Get_BRec_Top_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC Get Again Fail Rec Top *Error* Retval[%d] Line[%d]",inRetVal ,__LINE__);
		return (inRetVal);
	}
	
	inRetVal = inSqlite_ESC_Get_EMVRec_Top_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_EMV_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC Emv Get Again Fail Rec Top *Error* Retval[%d] Line[%d]",inRetVal ,__LINE__);
		return (inRetVal);
	}

	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (inRetVal);
}

/*
Function        :inNEXSYS_ESC_Delete_Again_Record_Most_TOP
Date&Time       :2017/3/23 上午 10:45
Describe        :刪除ESC資料庫中最上面一筆
*/
int inNEXSYS_ESC_Delete_Again_Record_Most_TOP(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = inSqlite_ESC_Delete_Record_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC Del Again Fail Rec Top *Error* Retval[%d] Line[%d]",inRetVal ,__LINE__);
		return (inRetVal);
	}
	
	inRetVal = inSqlite_ESC_Delete_Record_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_EMV_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC Emv Del Again Fail Rec Top *Error* Retval[%d] Line[%d]",inRetVal ,__LINE__);
		return (inRetVal);
	}

	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (inRetVal);
}

/*
Function        :inNEXSYS_ESC_Delete_Fail_Record_Most_TOP
Date&Time       :2017/3/23 上午 10:45
Describe        :刪除ESC資料庫中最上面一筆
*/
int inNEXSYS_ESC_Delete_Fail_Record_Most_TOP(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = inSqlite_ESC_Delete_Record_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC Del File BRec Flow Err [%d]", inRetVal);
		return (inRetVal);
	}

	inRetVal = inSqlite_ESC_Delete_Record_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_EMV_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC Emv Del File BRec Flow Err [%d]", inRetVal);
		return (inRetVal);
	}
	
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (inRetVal);
}

/*
Function        :inNEXSYS_ESC_Again_Table_Count
Date&Time       :2017/3/23 下午 5:28
Describe        :回傳Table資料筆數
*/
int inNEXSYS_ESC_Again_Table_Count(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	int	inAgain = -1;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = inSqlite_Get_Table_Count_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_, &inAgain);
	if (inRetVal == VS_NO_RECORD)
	{
		inESC_DebugLog("  Get Again Table Count No Rec [%d]", inRetVal);
		inAgain = 0;
	}
	else if (inRetVal == VS_ERROR)
	{
		inESC_DebugLog("  Get Again Table Count Err [%d]", inRetVal);
		inAgain = 0;
	}

	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (inAgain);
}

/*
Function        :inNEXSYS_ESC_Fail_Table_Count
Date&Time       :2017/3/23 下午 5:28
Describe        :回傳Table資料筆數
*/
int inNEXSYS_ESC_Fail_Table_Count(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	int	inFail = -1;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = inSqlite_Get_Table_Count_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_, &inFail);
	if (inRetVal == VS_NO_RECORD)
	{
		inESC_DebugLog("  Get Fail Table Count No Rec [%d]", inRetVal);
		inFail = 0;
	}
	else if (inRetVal == VS_ERROR)
	{
		inESC_DebugLog("  Get Fail Table Count Err [%d]", inRetVal);
		inFail = 0;
	}

	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (inFail);
}

/*
Function        :inNEXSYS_ESC_Process_PowerOff_When_Signing
Date&Time       :2018/3/23 下午 3:59
Describe        :1.有開簽名版才需重印
 *		    2.有開電簽上傳才需補統計bypass
*/
int inNEXSYS_ESC_Process_PowerOff_When_Signing(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	int	inHostIndex = -1;
	char	szESCMode[2 + 1] = {0};
	char	szSignpadMode[2 + 1] = {0};
	unsigned char	uszKey;
	TRANSACTION_OBJECT	pobESCTran;

	memset(szESCMode, 0x00, sizeof(szESCMode));
	inGetESCMode(szESCMode);

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 不是內建簽名板，不用重印 */
	memset(szSignpadMode, 0x00, sizeof(szSignpadMode));
	inGetSignPadMode(szSignpadMode);
	if (memcmp(szSignpadMode, _SIGNPAD_MODE_1_INTERNAL_, strlen(_SIGNPAD_MODE_1_INTERNAL_)) != 0)
	{
		inESC_DebugLog("  Sing Pad Mod Not Internal [%s]", szSignpadMode);
		return (VS_SUCCESS);
	}
	
	if (memcmp(szESCMode, "Y", strlen("Y")) == 0)
	{
	/* 先撈前一筆出來是否有簽 */
	memset(&pobESCTran, 0x00, sizeof(TRANSACTION_OBJECT));

	inHostIndex = -1;
	inFunc_Find_Specific_HDTindex(0, _HOST_NAME_CREDIT_MAINLY_USE_, &inHostIndex);
	pobESCTran.srBRec.inHDTIndex = inHostIndex;

	if( inLoadHDTRec(pobESCTran.srBRec.inHDTIndex) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" ESC Process Siging Load HDT[%d] *Error* Line[%d]",pobESCTran.srBRec.inHDTIndex,__LINE__);
	}
	
	if( inLoadHDPTRec(pobESCTran.srBRec.inHDTIndex) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" ESC Process Siging Load HDPT[%d] *Error* Line[%d]",pobESCTran.srBRec.inHDTIndex,__LINE__);
	}
		
	/* 撈最後一筆 */
	pobESCTran.srBRec.lnOrgInvNum =_BATCH_LAST_RECORD_;
	inRetVal = inBATCH_GetTransRecord_By_Sqlite(&pobESCTran);
	if (inRetVal == VS_SUCCESS)
	{
		
		inDISP_DispLogAndWriteFlie(" ESC Process Siging  Mode[%s] Status[%d] Line[%d]", szESCMode, pobESCTran.srBRec.inSignStatus, __LINE__);
		
		/* 如果是沒簽的狀態就重印簽單 */
		if (pobESCTran.srBRec.inSignStatus == _SIGN_NONE_)
		{
			
			inDISP_PutGraphic(_ESC_REPRINT_PAPER_BILL_, 0,  _COORDINATE_Y_LINE_8_3_);

			while (1)
			{
				uszKey = uszKBD_Key();
				if (uszKey == _KEY_ENTER_)
				{
					inRetVal = VS_SUCCESS;
					break;
				}
				inDISP_BEEP(0,1);
			}
		
			
			pobESCTran.srBRec.inSignStatus = _SIGN_BYPASS_;

			inFunc_RePrintReceipt_ByBuffer_Flow(&pobESCTran);


			/* 重印完後改狀態並統計 */
			pobESCTran.srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_PAPER_;
			inRetVal = inBATCH_Update_ESC_Uploaded_By_Sqlite(&pobESCTran);
			if( inRetVal != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie(" ESC Rpt Batch Update Rec RetVal[%d] *Error* Line[%d]",inRetVal, __LINE__);
				inFunc_EDCLock();
				return VS_ERROR;
			}


			if(VS_SUCCESS != inACCUM_UpdateFlow_ESC(&pobESCTran, _ESC_ACCUM_STATUS_BYPASS_)){
				inDISP_DispLogAndWriteFlie(" ESC Rpt Accum ByPass *Error* Line[%d]", __LINE__);
				inFunc_EDCLock();
				return VS_ERROR;
			}

		}
	}
	}else{
		inDISP_DispLogAndWriteFlie(" ESC Process Siging  Mode[%s] Status[%d] Line[%d]", szESCMode, pobESCTran.srBRec.inSignStatus, __LINE__);
	}
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
Function        :inNEXSYS_ESC_Func_Upload_Idle_Check_Time
Date&Time       :2018/5/23 上午 9:33
Describe        :
*/
int inNEXSYS_ESC_Func_Upload_Idle_Check_Time(void)
{
	int		inEDCTime = 0, inCheckTime = 0, inTimeout = 300;	/* timeout 失敗等五分鐘 (300秒) */
	char		szTimeTemp[2 + 1];
	unsigned char	uszTimeup = VS_FALSE;
	RTC_NEXSYS	srRTC;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* Idle等待 */
	/* 全為0 還沒開始計時 設定完起始時間 直接跳出 */
	if (atoi(gszNEX_ESC_Date) == 0 && atoi(gszNEX_ESC_Time) == 0)
	{
		/* 取得EDC時間日期 */
		memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
		if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
		{

		}
		else
		{
			memset(&gszNEX_ESC_Date, 0x00, sizeof(gszNEX_ESC_Date));
			memset(&gszNEX_ESC_Time, 0x00, sizeof(gszNEX_ESC_Time));
			sprintf(gszNEX_ESC_Date, "20%02d%02d%02d",  srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
			sprintf(gszNEX_ESC_Time, "%02d%02d%02d",  srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		}

		uszTimeup = VS_FALSE;
	}
	else
	{
		memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
		/* 取得EDC時間日期 */
		if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
		{
			uszTimeup = VS_FALSE;
		}
		else
		{
			memset(szTimeTemp, 0x00, sizeof(szTimeTemp));
			memcpy(&szTimeTemp, &gszNEX_ESC_Date[2], 2);
			/* 跨年就更新時間 */
			if (srRTC.uszYear != atoi(szTimeTemp))
			{
				uszTimeup = VS_FALSE;
			}
			else
			{
				memset(szTimeTemp, 0x00, sizeof(szTimeTemp));
				memcpy(&szTimeTemp, &gszNEX_ESC_Date[4], 2);
				/* 跨月就更新時間 */
				if (srRTC.uszMonth != atoi(szTimeTemp))
				{
					uszTimeup = VS_FALSE;
				}
				else
				{
					memset(szTimeTemp, 0x00, sizeof(szTimeTemp));
					memcpy(&szTimeTemp, &gszNEX_ESC_Date[6], 2);
					/* 跨日就更新時間 */
					if (srRTC.uszDay != atoi(szTimeTemp))
					{
						uszTimeup = VS_FALSE;
					}
					/* 同日 */
					else
					{
						/* 時 */
						memset(szTimeTemp, 0x00, sizeof(szTimeTemp));
						memcpy(&szTimeTemp, gszNEX_ESC_Time, 2);
						inCheckTime += atoi(szTimeTemp) * 60 * 60;
						inEDCTime += srRTC.uszHour * 60 * 60;

						/* 分 */
						memset(szTimeTemp, 0x00, sizeof(szTimeTemp));
						memcpy(&szTimeTemp, &gszNEX_ESC_Time[2], 2);
						inCheckTime += atoi(szTimeTemp) * 60;
						inEDCTime += srRTC.uszMinute * 60;

						/* 秒 */
						memset(szTimeTemp, 0x00, sizeof(szTimeTemp));
						memcpy(&szTimeTemp, &gszNEX_ESC_Time[4], 2);
						inCheckTime += atoi(szTimeTemp);
						inEDCTime += srRTC.uszSecond;


						if (inEDCTime >= (inCheckTime + inTimeout))
							uszTimeup = VS_TRUE;
						else
							uszTimeup = VS_FALSE;

					}/* 跨日 */

				}/* 垮月 */

			}/* 跨年 */
		}

	}/* 確認是否預設值 */


	if (uszTimeup == VS_TRUE)
	{
		inESC_DebugLog("-----[%s][%s][%d] Check Success END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}
	else
	{
		inESC_DebugLog("-----[%s][%s][%d] Check Error END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
}

/*
Function        :vdNEXSYS_ESC_ISOFormatDebug_DISP
Date&Time       :2016/12/1 上午 9:41
Describe        :
*/
void vdNEXSYS_ESC_ISO_FormatDebug_DISP(unsigned char *uszSendBuf, int inSendLen)
{
	char			szBuf[3024 + 1], szBitMap[_NEXSYS_ESC_BIT_MAP_SIZE_ + 1];
	int			i, inCnt = 0, inField, inLen, inFieldLen;
	int			inPrintLineCnt = 0;
	int			inOneLineLen = 34;
	int			inBitMapCnt1, inBitMapCnt2;
	char			szPrtBuf[50 + 1];
	char			szPrintLineData[36 + 1];
	unsigned char		uszBitMap[9], uszBitMapDisp[66];
	ISO_TYPE_NEXSYS_ESC_TABLE srISOTypeTablePrt;

	memset((char *)&srISOTypeTablePrt, 0x00, sizeof(srISOTypeTablePrt));
	memcpy((char *)&srISOTypeTablePrt, (char *)srNEXSYS_ESC_ISOFunc, sizeof(srISOTypeTablePrt));

	if (uszSendBuf[6] == 0x00 || uszSendBuf[6] == 0x20 || uszSendBuf[6] == 0x40)
	{
		inESC_DebugLog(" ");
		inESC_DebugLog("==========================================");
		inESC_DebugLog("ISO8583 Format  <<ESC Send>>");
	}
	else
		inESC_DebugLog("ISO8583 Format  <<ESC Recvive>>");

	memset(szBuf, 0x00, sizeof(szBuf));
	sprintf(szBuf,"TPDU = [");
	inFunc_BCD_to_ASCII(&szBuf[8], (unsigned char*)&uszSendBuf[inCnt], _NEXSYS_ESC_TPDU_SIZE_);
	strcat(szBuf,"]");
	inESC_DebugLog(szBuf);
	inCnt += _NEXSYS_ESC_TPDU_SIZE_;

	memset(szBuf, 0x00, sizeof(szBuf));
	sprintf(szBuf,"MTI  = [");
	inFunc_BCD_to_ASCII(&szBuf[8], (unsigned char*)&uszSendBuf[inCnt], _NEXSYS_ESC_MTI_SIZE_);
	strcat(szBuf,"]");
	inESC_DebugLog(szBuf);
	inCnt += _NEXSYS_ESC_MTI_SIZE_;

	memset(szBuf, 0x00, sizeof(szBuf));
	memset(szBitMap, 0x00, sizeof(szBitMap));
	memcpy(szBitMap, (char *)&uszSendBuf[inCnt], _NEXSYS_ESC_BIT_MAP_SIZE_);
	sprintf(szBuf, "BMap = [%02X %02X %02X %02X %02X %02X %02X %02X]",
						uszSendBuf[inCnt + 0],
						uszSendBuf[inCnt + 1],
						uszSendBuf[inCnt + 2],
						uszSendBuf[inCnt + 3],
						uszSendBuf[inCnt + 4],
						uszSendBuf[inCnt + 5],
						uszSendBuf[inCnt + 6],
						uszSendBuf[inCnt + 7]);

	inESC_DebugLog(szBuf);
	memset(uszBitMapDisp, 0x00, sizeof(uszBitMapDisp));
	memset(uszBitMap, 0x00, sizeof(uszBitMap));
	memcpy(uszBitMap, (char *)&uszSendBuf[inCnt], _NEXSYS_ESC_BIT_MAP_SIZE_);

	for (inBitMapCnt1 = 0; inBitMapCnt1 < 8; inBitMapCnt1 ++)
        {
                for (inBitMapCnt2 = 0; inBitMapCnt2 < 8; inBitMapCnt2 ++)
                {
                        if (uszBitMap[inBitMapCnt1] & 0x80)
                                uszBitMapDisp[(inBitMapCnt1*8) + inBitMapCnt2 + 1] = '1';
                        else
                                uszBitMapDisp[(inBitMapCnt1*8) + inBitMapCnt2 + 1] = '0';

                        uszBitMap[inBitMapCnt1] = uszBitMap[inBitMapCnt1] << 1;
            }
        }

	inESC_DebugLog("   1234567890    1234567890    1234567890");
	memset(szBuf, 0x00, sizeof(szBuf));
	sprintf(szBuf, " 0>%10.10s  1>%10.10s  2>%10.10s", &uszBitMapDisp[1], &uszBitMapDisp[11], &uszBitMapDisp[21]);
        inESC_DebugLog(szBuf);
        sprintf(szBuf, " 3>%10.10s  4>%10.10s  5>%10.10s", &uszBitMapDisp[31], &uszBitMapDisp[41], &uszBitMapDisp[51]);
      	inESC_DebugLog(szBuf);
        sprintf(szBuf, " 6>%4.4s", &uszBitMapDisp[61]);
        inESC_DebugLog(szBuf);

	inCnt += _NEXSYS_ESC_BIT_MAP_SIZE_;

	for (i = 1; i <= 64; i ++)
	{
		if (!inNEXSYS_ESC_BitMapCheck((unsigned char *)szBitMap, i))
			continue;

		inLen = 0;
		inFieldLen = 0;
		memset(szBuf, 0x00, sizeof(szBuf));
		sprintf(szBuf, "F_%02d ", i);
		inLen = 5;
		strcat(&szBuf[inLen ++], "[");

		switch (i)
		{
			case 2 :
				strcat(szBuf, "PAN ");
				inLen += 4;
				break;
			case 3 :
				strcat(szBuf, "P_CODE ");
				inLen += 7;
				break;
			case 4 :
				strcat(szBuf, "AMT ");
				inLen += 4;
				break;
			case 11 :
				strcat(szBuf, "STAN ");
				inLen += 5;
				break;
			case 12 :
				strcat(szBuf, "TIME ");
				inLen += 5;
				break;
			case 13 :
				strcat(szBuf, "DATE ");
				inLen += 5;
				break;
			case 24 :
				strcat(szBuf, "NII ");
				inLen += 4;
				break;
			case 37 :
				strcat(szBuf, "RRN ");
				inLen += 4;
				break;
			case 38 :
				strcat(szBuf, "AUTH_ID ");
				inLen += 8;
				break;
			case 39 :
				strcat(szBuf, "RESP_CODE ");
				inLen += 10;
				break;
			case 41 :
				strcat(szBuf, "TID ");
				inLen += 4;
				break;
			case 42 :
				strcat(szBuf, "MID ");
				inLen += 4;
				break;
			case 48 :
				strcat(szBuf, "STORE_ID ");
				inLen += 9;
				break;
			case 52 :
				strcat(szBuf, "PIN_BLOCK ");
				inLen += 10;
				break;
			case 54 :
				strcat(szBuf, "TIP_AMT ");
				inLen += 8;
				break;
			case 55 :
				strcat(szBuf, "EMV ");
				inLen += 4;
				break;
			case 56 :
				strcat(szBuf, "EMV ");
				inLen += 4;
				break;
			case 57 :
				strcat(szBuf, "CSUM ");
				inLen += 5;
				break;
			case 59 :
				strcat(szBuf, "TABLE ");
				inLen += 6;
				break;
			case 60 :
				strcat(szBuf, "BATCH ");
				inLen += 6;
				break;
			case 62 :
				strcat(szBuf, "INV ");
				inLen += 4;
				break;
			case 64 :
				strcat(szBuf, "MAC ");
				inLen += 4;
				break;
			default :
				break;
		}

		inField = inNEXSYS_ESC_GetFieldIndex(i, srISOTypeTablePrt.srISOFieldType);
		inFieldLen = inNEXSYS_ESC_GetFieldLen(i, &uszSendBuf[inCnt], srISOTypeTablePrt.srISOFieldType);
		switch (srISOTypeTablePrt.srISOFieldType[inField].inFieldType)
		{
			case _NEXSYS_ESC_ISO_ASC_ :
				memcpy(&szBuf[inLen], &uszSendBuf[inCnt], inFieldLen);
				break;
			case _NEXSYS_ESC_ISO_BYTE_3_ :
				inFunc_BCD_to_ASCII(&szBuf[inLen], (unsigned char*)&uszSendBuf[inCnt], 2);
				inCnt += 2;
				inLen += 4;
				strcat(szBuf, " ");
				inLen ++;
				inFieldLen --;
				inFieldLen --;

				if (srISOTypeTablePrt.srISOFieldType[inField].uszDispAscii == VS_TRUE)
				{
					if (i != 59)
						memcpy(&szBuf[inLen], (char *)&uszSendBuf[inCnt], inFieldLen);
				}

				break;
			case _NEXSYS_ESC_ISO_NIBBLE_2_ :
			case _NEXSYS_ESC_ISO_BYTE_2_ :
			case _NEXSYS_ESC_ISO_BYTE_2_H_ :
			case _NEXSYS_ESC_ISO_BYTE_3_H_ :
//				strcat(&szBuf[inLen ++], "[");
				inFunc_BCD_to_ASCII(&szBuf[inLen], (unsigned char*)&uszSendBuf[inCnt ++], 1);
				inLen += 2;
				strcat(szBuf, " ");
				inLen ++;

				inFieldLen --;
				if (srISOTypeTablePrt.srISOFieldType[inField].inFieldType == _NEXSYS_ESC_ISO_BYTE_3_ ||
				    srISOTypeTablePrt.srISOFieldType[inField].inFieldType == _NEXSYS_ESC_ISO_BYTE_3_H_)
				{
					inFunc_BCD_to_ASCII(&szBuf[inLen], (unsigned char*)&uszSendBuf[inCnt ++], 1);
					inLen += 2;
					inFieldLen --;
				}

			case _NEXSYS_ESC_ISO_BCD_ :
				if (srISOTypeTablePrt.srISOFieldType[inField].uszDispAscii == VS_TRUE)
					memcpy(&szBuf[inLen], (char *)&uszSendBuf[inCnt], inFieldLen);
				else
					inFunc_BCD_to_ASCII(&szBuf[inLen], (unsigned char*)&uszSendBuf[inCnt], inFieldLen);

				break;
			default :
				break;
		}

		strcat(&szBuf[inLen ++], "]");

		inPrintLineCnt = 0;
		while ((inPrintLineCnt * inOneLineLen) < strlen(szBuf))
		{
			memset(szPrintLineData, 0x00, sizeof(szPrintLineData));
			memset(szPrtBuf, 0x00, sizeof(szPrtBuf));
			if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szBuf))
			{
				strcat(szPrintLineData, &szBuf[inPrintLineCnt * inOneLineLen]);
			}
			else
			{
				memcpy(szPrintLineData, &szBuf[inPrintLineCnt * inOneLineLen], inOneLineLen);
			}

			sprintf(szPrtBuf, "  %s", szPrintLineData);

			inESC_DebugLog(szPrtBuf);
			inPrintLineCnt ++;
		};

//		if (i == 59)
//			vdNEXSYS_ESC_ISO_FormatDebug_DISP_59(&uszSendBuf[inCnt], inFieldLen);

		inCnt += inFieldLen;
	}

	inESC_DebugLog("==========================================");
	inESC_DebugLog(" ");
}

/*
Function        :vdNEXSYS_ESC_ISO_FormatDebug_DISP_59
Date&Time       :2016/12/1 上午 9:42
Describe        :
*/
void vdNEXSYS_ESC_ISO_FormatDebug_DISP_59(unsigned char *uszSendBuf, int inFieldLen)
{
	int	inCnt = 0;
	int inUnPackMode = 1;
	
	inESC_DebugLog("  59 Field Len [%d] ", inFieldLen );
	
	for (inCnt = 0; inCnt < inFieldLen; )
	{
		switch (uszSendBuf[inCnt++])
		{
			case 'E':
				inESC_DebugLog(" Cont [%d] ", inCnt );
				switch (uszSendBuf[inCnt++])
				{
					case '1':
						inESC_DebugLog(" E1Cont2 [%d] ", inCnt );
						inESC_DebugLog("  First Data [%02x] ", uszSendBuf[inCnt] );
						inESC_UuPackTableE1(&uszSendBuf[inCnt],  &inCnt, inUnPackMode);
						inESC_DebugLog(" E1 End Count 2 [%d] ", inCnt );
						break;
					case '2':
						inESC_DebugLog(" E2Cont2 [%d] ", inCnt );
						inESC_DebugLog("  First Data [%02x] ", uszSendBuf[inCnt] );
						inESC_UuPackTableE2(&uszSendBuf[inCnt],  &inCnt, inUnPackMode);
						inESC_DebugLog(" E2 End Count 2 [%d] ", inCnt );
						break;	
					default:
						inESC_DebugLog(" Analisy E Table Id Err [%c%c] [%d] ", uszSendBuf[inCnt-1] ,uszSendBuf[inCnt] , inCnt );
						return ;
				}
				break;
			case 'R':
				inESC_DebugLog(" R1 Cont [%d] ", inCnt );
				//inDISP_Wait(1000);
				switch (uszSendBuf[inCnt++])
				{
					case '1':
						inESC_UuPackTableR1(&uszSendBuf[inCnt],  &inCnt, inUnPackMode);
						break;
					case '2':
						inESC_UuPackTableR2(&uszSendBuf[inCnt],  &inCnt, inUnPackMode);
						break;
					case '3':
						inESC_UuPackTableR3(&uszSendBuf[inCnt],  &inCnt, inUnPackMode);
						break;
					case '4':
						inESC_UuPackTableR4(&uszSendBuf[inCnt],  &inCnt, inUnPackMode);
						break;
					case '5':
						inESC_UuPackTableR5(&uszSendBuf[inCnt],  &inCnt, inUnPackMode);
						inESC_DebugLog(" R5 End Count 2 [%d] ", inCnt );
						break;
					default:
						inESC_DebugLog(" Analisy R Table Id Err [%c%c] [%d] ", uszSendBuf[inCnt-1] ,uszSendBuf[inCnt] , inCnt );
						return ;
				}
				break;
			default:
				inESC_DebugLog(" Analisy DEF Table Id Err [%s] [%d] ", uszSendBuf[inCnt] , inCnt );
				return ;
		}
	}
	
	return ;
}

/*
Function        :vdNEXSYS_ESC_ISO_FormatDebug_PRINT
Date&Time       :2016/12/1 上午 9:41
Describe        :
*/
void vdNEXSYS_ESC_ISO_FormatDebug_PRINT(unsigned char *uszSendBuf, int inSendLen)
{
	char			szBuf[1024 + 1], szBitMap[_NEXSYS_ESC_BIT_MAP_SIZE_ + 1];
	int			i, inCnt = 0, inField, inLen, inFieldLen;
	int			inPrintLineCnt = 0;
	int			inOneLineLen = 34;
	int			inBitMapCnt1, inBitMapCnt2;
	char			szPrtBuf[50 + 1];
	char			szPrintLineData[36 + 1];
	unsigned char		uszBitMap[9], uszBitMapDisp[66];
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;
	ISO_TYPE_NEXSYS_ESC_TABLE srISOTypeTablePrt;

	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(inESC_GetMachineTypeFunc()) != VS_SUCCESS)
	{

	}
	else
	{
		/* 初始化 */
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

		memset((char *)&srISOTypeTablePrt, 0x00, sizeof(srISOTypeTablePrt));
		memcpy((char *)&srISOTypeTablePrt, (char *)srNEXSYS_ESC_ISOFunc, sizeof(srISOTypeTablePrt));

		if (uszSendBuf[6] == 0x00 || uszSendBuf[6] == 0x20 || uszSendBuf[6] == 0x40)
		{
			inPRINT_Buffer_PutIn(" ", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("==========================================", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("ISO8583 Format  <<ESC Send>>", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
			inPRINT_Buffer_PutIn("ISO8583 Format  <<ESC Recvive>>", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szBuf, 0x00, sizeof(szBuf));
		sprintf(szBuf,"TPDU = [");
		inFunc_BCD_to_ASCII(&szBuf[8], (unsigned char*)&uszSendBuf[inCnt], _NEXSYS_ESC_TPDU_SIZE_);
		strcat(szBuf,"]");
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inCnt += _NEXSYS_ESC_TPDU_SIZE_;

		memset(szBuf, 0x00, sizeof(szBuf));
		sprintf(szBuf,"MTI  = [");
		inFunc_BCD_to_ASCII(&szBuf[8], (unsigned char*)&uszSendBuf[inCnt], _NEXSYS_ESC_MTI_SIZE_);
		strcat(szBuf,"]");
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inCnt += _NEXSYS_ESC_MTI_SIZE_;

		memset(szBuf, 0x00, sizeof(szBuf));
		memset(szBitMap, 0x00, sizeof(szBitMap));
		memcpy(szBitMap, (char *)&uszSendBuf[inCnt], _NEXSYS_ESC_BIT_MAP_SIZE_);
		sprintf(szBuf, "BMap = [%02X %02X %02X %02X %02X %02X %02X %02X]",
							uszSendBuf[inCnt + 0],
							uszSendBuf[inCnt + 1],
							uszSendBuf[inCnt + 2],
							uszSendBuf[inCnt + 3],
							uszSendBuf[inCnt + 4],
							uszSendBuf[inCnt + 5],
							uszSendBuf[inCnt + 6],
							uszSendBuf[inCnt + 7]);

		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		memset(uszBitMapDisp, 0x00, sizeof(uszBitMapDisp));
		memset(uszBitMap, 0x00, sizeof(uszBitMap));
		memcpy(uszBitMap, (char *)&uszSendBuf[inCnt], _NEXSYS_ESC_BIT_MAP_SIZE_);

		for (inBitMapCnt1 = 0; inBitMapCnt1 < 8; inBitMapCnt1 ++)
		{
			for (inBitMapCnt2 = 0; inBitMapCnt2 < 8; inBitMapCnt2 ++)
			{
				if (uszBitMap[inBitMapCnt1] & 0x80)
					uszBitMapDisp[(inBitMapCnt1*8) + inBitMapCnt2 + 1] = '1';
				else
					uszBitMapDisp[(inBitMapCnt1*8) + inBitMapCnt2 + 1] = '0';

				uszBitMap[inBitMapCnt1] = uszBitMap[inBitMapCnt1] << 1;
		    }
		}

		inPRINT_Buffer_PutIn("   1234567890    1234567890    1234567890", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		memset(szBuf, 0x00, sizeof(szBuf));
		sprintf(szBuf, " 0>%10.10s  1>%10.10s  2>%10.10s", &uszBitMapDisp[1], &uszBitMapDisp[11], &uszBitMapDisp[21]);
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		sprintf(szBuf, " 3>%10.10s  4>%10.10s  5>%10.10s", &uszBitMapDisp[31], &uszBitMapDisp[41], &uszBitMapDisp[51]);
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		sprintf(szBuf, " 6>%4.4s", &uszBitMapDisp[61]);
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inCnt += _NEXSYS_ESC_BIT_MAP_SIZE_;

		for (i = 1; i <= 64; i ++)
		{
			if (!inNEXSYS_ESC_BitMapCheck((unsigned char *)szBitMap, i))
				continue;

			inLen = 0;
			inFieldLen = 0;
			memset(szBuf, 0x00, sizeof(szBuf));
			sprintf(szBuf, "F_%02d ", i);
			inLen = 5;
			strcat(&szBuf[inLen ++], "[");

			switch (i)
			{
				case 3 :
					strcat(szBuf, "P_CODE ");
					inLen += 7;
					break;
				case 4 :
					strcat(szBuf, "AMT ");
					inLen += 4;
					break;
				case 11 :
					strcat(szBuf, "STAN ");
					inLen += 5;
					break;
				case 12 :
					strcat(szBuf, "TIME ");
					inLen += 5;
					break;
				case 13 :
					strcat(szBuf, "DATE ");
					inLen += 5;
					break;
				case 24 :
					strcat(szBuf, "NII ");
					inLen += 4;
					break;
				case 37 :
					strcat(szBuf, "RRN ");
					inLen += 4;
					break;
				case 38 :
					strcat(szBuf, "AUTH_ID ");
					inLen += 8;
					break;
				case 39 :
					strcat(szBuf, "RESP_CODE ");
					inLen += 10;
					break;
				case 41 :
					strcat(szBuf, "TID ");
					inLen += 4;
					break;
				case 42 :
					strcat(szBuf, "MID ");
					inLen += 4;
					break;
				case 48 :
					strcat(szBuf, "STORE_ID ");
					inLen += 9;
					break;
				case 52 :
					strcat(szBuf, "PIN_BLOCK ");
					inLen += 10;
					break;
				case 54 :
					strcat(szBuf, "TIP_AMT ");
					inLen += 8;
					break;
				case 55 :
					strcat(szBuf, "EMV ");
					inLen += 4;
					break;
				case 56 :
					strcat(szBuf, "EMV ");
					inLen += 4;
					break;
				case 57 :
					strcat(szBuf, "CSUM ");
					inLen += 5;
					break;
				case 59 :
					strcat(szBuf, "TABLE ");
					inLen += 6;
					break;
				case 64 :
					strcat(szBuf, "MAC ");
					inLen += 4;
					break;
				default :
					break;
			}

			inField = inNEXSYS_ESC_GetFieldIndex(i, srISOTypeTablePrt.srISOFieldType);
			inFieldLen = inNEXSYS_ESC_GetFieldLen(i, &uszSendBuf[inCnt], srISOTypeTablePrt.srISOFieldType);
			switch (srISOTypeTablePrt.srISOFieldType[inField].inFieldType)
			{
				case _NEXSYS_ESC_ISO_ASC_ :
					memcpy(&szBuf[inLen], &uszSendBuf[inCnt], inFieldLen);
					break;
				case _NEXSYS_ESC_ISO_BYTE_3_ :
					inFunc_BCD_to_ASCII(&szBuf[inLen], (unsigned char*)&uszSendBuf[inCnt], 2);
					inCnt += 2;
					inLen += 4;
					strcat(szBuf, " ");
					inLen ++;
					inFieldLen --;
					inFieldLen --;

					if (srISOTypeTablePrt.srISOFieldType[inField].uszDispAscii == VS_TRUE)
					{
						if (i != 59)
							memcpy(&szBuf[inLen], (char *)&uszSendBuf[inCnt], inFieldLen);
					}

					break;
				case _NEXSYS_ESC_ISO_NIBBLE_2_ :
				case _NEXSYS_ESC_ISO_BYTE_2_ :
				case _NEXSYS_ESC_ISO_BYTE_2_H_ :
				case _NEXSYS_ESC_ISO_BYTE_3_H_ :
	//				strcat(&szBuf[inLen ++], "[");
					inFunc_BCD_to_ASCII(&szBuf[inLen], (unsigned char*)&uszSendBuf[inCnt ++], 1);
					inLen += 2;
					strcat(szBuf, " ");
					inLen ++;

					inFieldLen --;
					if (srISOTypeTablePrt.srISOFieldType[inField].inFieldType == _NEXSYS_ESC_ISO_BYTE_3_ ||
					    srISOTypeTablePrt.srISOFieldType[inField].inFieldType == _NEXSYS_ESC_ISO_BYTE_3_H_)
					{
						inFunc_BCD_to_ASCII(&szBuf[inLen], (unsigned char*)&uszSendBuf[inCnt ++], 1);
						inLen += 2;
						inFieldLen --;
					}

				case _NEXSYS_ESC_ISO_BCD_ :
					if (srISOTypeTablePrt.srISOFieldType[inField].uszDispAscii == VS_TRUE)
						memcpy(&szBuf[inLen], (char *)&uszSendBuf[inCnt], inFieldLen);
					else
						inFunc_BCD_to_ASCII(&szBuf[inLen], (unsigned char*)&uszSendBuf[inCnt], inFieldLen);

					break;
				default :
					break;
			}

			strcat(&szBuf[inLen ++], "]");

			inPrintLineCnt = 0;
			while ((inPrintLineCnt * inOneLineLen) < strlen(szBuf))
			{
				memset(szPrintLineData, 0x00, sizeof(szPrintLineData));
				memset(szPrtBuf, 0x00, sizeof(szPrtBuf));
				if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szBuf))
				{
					strcat(szPrintLineData, &szBuf[inPrintLineCnt * inOneLineLen]);
				}
				else
				{
					memcpy(szPrintLineData, &szBuf[inPrintLineCnt * inOneLineLen], inOneLineLen);
				}

				sprintf(szPrtBuf, "  %s", szPrintLineData);

				inPRINT_Buffer_PutIn(szPrtBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPrintLineCnt ++;
			};

			if ( i == 59)
				vdNEXSYS_ESC_ISO_FormatDebug_PRINT_59(&uszSendBuf[inCnt], inFieldLen, uszBuffer, &srFont_Attrib, &srBhandle);

			inCnt += inFieldLen;
		}

		inPRINT_Buffer_PutIn("==========================================", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
	}
}

/*
Function        :vdNEXSYS_ESC_ISO_FormatDebug_PRINT_59
Date&Time       :2016/12/1 上午 9:42
Describe        :
*/
void vdNEXSYS_ESC_ISO_FormatDebug_PRINT_59(unsigned char *uszSendBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inCnt = 0, inTableLen;
	long	lnTableLen;
	char	szPrintBuf[128], szTemplate[42 + 1];
	char 	szTempTable[1 + 1];

	while (inFieldLen > inCnt)
	{
		switch (uszSendBuf[inCnt])
		{
			case 'C' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %x / %s", uszSendBuf[inCnt], szTemplate);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* Table Length */
				inTableLen = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %x / %d", uszSendBuf[inCnt], inTableLen);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* Function Number / The Terminal Support CUP Function (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Function Number = %s", szTemplate);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* Acquire Indicator / Terminal Hot Key Control (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Acquire Indicator = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				if (szTemplate[0] == '1')
				{
					/* CUP Trace Number (6 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], &uszSendBuf[inCnt], 6);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    CUP Trace Number = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				        inCnt += 6;

					/* CUP Transaction Date(MMDD) (4 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], &uszSendBuf[inCnt], 4);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    CUP Transaction Date(MMDD) = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt += 4;

					/* CUP Transaction Time(hhmmss) (6 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], &uszSendBuf[inCnt], 6);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    CUP Transaction Time(hhmmss) = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt += 6;

					/* CUP Retrieve Reference Number(CRRN) (12 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], &uszSendBuf[inCnt], 12);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    CUP Retrieve Reference Number(CRRN) = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt += 12;

					/* Settlement Date(MMDD) (4 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], &uszSendBuf[inCnt], 4);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Settlement Date(MMDD) = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt += 4;
				}

				break;
			case 'D' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %x / %s", uszSendBuf[inCnt], szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* Table Length */
				inTableLen = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %x / %d", uszSendBuf[inCnt], inTableLen);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* Online Rate DCC (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Online Rate DCC = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				break;
			case 'O' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %x / %s", uszSendBuf[inCnt], szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* Table Length */
				inTableLen = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %x / %d", uszSendBuf[inCnt], inTableLen);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* Original Transaction Date(MMDD) (4 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 4);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Original Transaction Date(MMDD) = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 4;

				/* Original Amount (12 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 12);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Original Amount = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 12;

				break;
			case 'Y' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %x / %s", uszSendBuf[inCnt], szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* Table Length */
				inTableLen = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %x / %d", uszSendBuf[inCnt], inTableLen);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* 授權主機的西元年 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 4);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Host YYYY = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 4;

				break;
                        /* For ESC電子簽單，ISODebug */
			case 'N' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				inCnt ++;

				memset(szTempTable, 0x00, sizeof(szTempTable));
				szTempTable[0] = uszSendBuf[inCnt];

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %x%x / %s%s", uszSendBuf[inCnt - 1], uszSendBuf[inCnt], szTemplate, szTempTable);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* Table Length */
				/* 目前都是0x00先跳過 */
				inCnt ++;

				inTableLen = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %x / %d", uszSendBuf[inCnt], inTableLen);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				if (szTempTable[0] == 0x31)		/* 1 */
				{
					/* Function Number / The Terminal Support CUP Function (1 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Function Number = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt ++;

					/* Acquire Indicator / Terminal Hot Key Control (1 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Acquire Indicator = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt ++;
				}
				else if (szTempTable[0] == 0x34)	/* 4 */
				{
					/* Installment Indicator (1 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Inst Indicator = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt ++;

					/* Installment Period (2 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					inCnt ++;

					memset(szTempTable, 0x00, sizeof(szTempTable));
					szTempTable[0] = uszSendBuf[inCnt];

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Inst Period = %s%s", szTemplate, szTempTable);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt ++;
				}
				else if (szTempTable[0] == 0x35)	/* 5 */
				{
					/* Installment Indicator (1 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Redeem Indicator = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt ++;
				}
				else if (szTempTable[0] == 0x45)	/* E */
				{
					/* ESC Indicator (1 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    ESC = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt ++;
				}
				else if (szTempTable[0] == 0x49)	/* I */
				{
					/* VEPS Indicator (1 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Stand In = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt ++;
				}
                                else if (szTempTable[0] == 0x51)	/* Q */
                                {
                                        /*【需求單 - 104058】免簽名需求《 依照MFES授權主機回覆交易電文的Indicator判斷帳單是否列印免簽名》by Tusin - 2015/12/2 上午 11:22:04 */
                                        /* VEPS Indicator (1 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Quick Pay = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt ++;
                                }

				break;
			case 'E' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				inCnt ++;

				memset(szTempTable, 0x00, sizeof(szTempTable));
				szTempTable[0] = uszSendBuf[inCnt];

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %x%x / %s%s", uszSendBuf[inCnt - 1], uszSendBuf[inCnt], szTemplate, szTempTable);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* Table Length */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 2);

				memset(szTempTable, 0x00, sizeof(szTempTable));
				inFunc_BCD_to_ASCII(szTempTable, (unsigned char*)szTemplate, 2);

				inTableLen = atoi(szTempTable);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %02x%02x / %04d", szTemplate[0], szTemplate[1], inTableLen);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 2;

				/* 唯一序號 (28 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 14);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    ESC S/N = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 14;

				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 14);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "              %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 14;

				/* Card Information (10 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 10);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Card Type = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 10;

				/* 封包總個數 (2 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 2);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Total Cnt = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 2;

				/* 目前封包編號 (2 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 2);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Packet Cnt = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 2;

				/* 封包總大小 (5 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 5);

				lnTableLen = atol(szTemplate);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Total Size = %ld", lnTableLen);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 5;

				/* 目前封包大小 (3 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 3);

				inTableLen = atoi(szTemplate);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Packet Size = %d", inTableLen);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 3;

				/* 封包內容 */
				inCnt += inTableLen;

				/* LRC (1 Byte) */
//				memset(szTemplate, 0x00, sizeof(szTemplate));
//				memcpy(szTemplate, (char*)&uszSendBuf[inCnt], 1);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    LRC = %02x", uszSendBuf[inCnt]);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				break;
			default :
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "      OTHER = (%s)", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				break;
		}
	}
}

/* 記錄 Amt 中的紙本計數, 因為不用上傳，所以統計在  ACCUM_TOTAL_REC 裏就好 */
int inNEXSYS_ESC_Func_UpdateReprintCount(TRANSACTION_OBJECT *pobTran)
{
	
	inESC_DebugLog("-----[%s][%s][%d]  START -----",__FILE__, __FUNCTION__, __LINE__);
	if (inFLOW_RunFunction(pobTran, _NEXSYS_ESC_CHECK_) == VS_SUCCESS)
	{
		if( inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" inNEXSYS_ESC_Func_UpdateReprintCount Load HDT[%d] *Error* Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
		}
		
		inACCUM_UpdateFlow_ESC(pobTran, _ESC_ACCUM_STATUS_REPRINT_);
	}
	inESC_DebugLog("-----[%s][%s][%d]  END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}

