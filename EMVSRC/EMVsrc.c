#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <emvaplib.h>
#include <sqlite3.h>

#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/MVT.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/FUNCTION/APDU.h"
#include "../SOURCE/FUNCTION/Batch.h"
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/Card.h"
#include "../SOURCE/FUNCTION/KMS.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/COMM/Comm.h"

#include "../SOURCE/FUNCTION/FILE_FUNC/File.h"
#include "../SOURCE/FUNCTION/FILE_FUNC/FIleLogFunc.h"

#include "../SOURCE/FUNCTION/ECR_FUNC/ECR.h"
#include "../SOURCE/FUNCTION/ECR_FUNC/RS232.h"

#include "../SOURCE/FUNCTION/UNIT_FUNC/TimeUint.h"
#include "../CTLS/CTLS.h"

#include "../FUBON/FUBONfunc.h"
#include "../FUBON/FUBONiso.h"

#include "EMVsrc.h"


#ifdef _DEBUG_9F1F_TAG_
	extern unsigned char gusz9F1F[129];
	extern long gln9F1FLen;
#endif

extern int ginDebug;
extern int ginISODebug;
extern int ginMenuKeyIn, ginIdleMSRStatus, ginIdleICCStatus;
extern int ginFallback;
extern int ginAPVersionType;
extern EMV_CONFIG EMVGlobConfig;
int ginGlobalRetVal;
int ginEMVAppSelection = -1;

TRANSACTION_OBJECT pobEmvTran;


/* EMV 流程會用到的所有 Callback Function */
EMV_EVENT g_emv_event ={
	1, /* version*/
	NULL, //OnDisplayShow,mark by green 191225 /* 執行EMV_TxnAppSelect開始時會CALL這支function是否要顯示訊息 */
	OnErrorMsg, /* 執行EMV_TxnAppSelect若是有錯誤訊息要顯示最後面會CALL這支function */
	OnEMVConfigActive, /* 執行EMV_Initialize時會先CALL這支function，選擇要跑的Config */
	NULL, /* HashVerify Not using (This event function is used for application to verify if the CAPKs of EMV kernel are correct.)*/
	OnTxnDataGet, /* 執行EMV_TxnAppSelect時選完後會要求輸入交易資料，如金額，這是必跑的CallBack */
	OnAppList, /* 執行EMV_TxnAppSelect時若是有多個AID會進入此function，要顯示AID列表供選擇 */
	OnAppSelectedConfirm, /* 執行EMV_TxnAppSelect時若是只有一個AID會進入此function */
	OnTerminalDataGet, /* 執行EMV_TxnPerform時若是Kernel在XML找不到需要的TAG值會從這裡要求提供所需的TAG */
	OnCAPKGet, /* 執行EMV_TxnPerform時若是Kernel在XML找不到需要的CAPK KEY值會從這裡要求提供所需的CAPK KEY */
	OnGetPINNotify, /* 執行EMV_TxnPerform時若是要用Internal PINPAD輸入PIN會CALL這支，輸入完後交由KERNAL處理，若是輸入錯則流程繼續往下跑，除非重新交易才會再進來 */
	OnOnlinePINBlockGet, /* 執行EMV_TxnPerform時若是需要用外接PINPAD輸入Online PIN BLOCK會CALL這支function，目前都會用內建，所以應該不會用這支 */
	OnOfflinePINBlockGet, /* 執行EMV_TxnPerform時若是需要用外接PINPAD輸入Offline PIN BLOCK會CALL這支function，目前都會用內建，所以應該不會用這支 */
	OnOfflinePINVerifyResult, /* 執行EMV_TxnPerform時若是輸入Offline PIN後會CALL這支function檢查PIN是否合法 */
	OnTxnOnline, /* 執行EMV_TxnPerform時若是First GenAC要求Online則進入這支function做電文送收，這支function一定要跑 */
	OnTxnIssuerScriptResult, /* 執行EMV_TxnPerform時若是要處理Issuer Script則要跑這支function */
	OnTxnResult, /* 執行EMV_TxnPerform的最後要判斷晶片處理結果且顯示通知使用者要在這處理 */
	OnTotalAmountGet, /* 執行EMV_TxnPerform時要在這輸入金額將金額回傳給Kernel */
	OnExceptionFileCheck, /* 執行EMV_TxnPerform時在進行FirstGenAC前會進入這檢查檔案 */
	OnCAPKRevocationCheck, /* 執行EMV_TxnPerform時取得CAPK後會檢查是否CAPK合法 */
};

int inCheckFallbackFlag(TRANSACTION_OBJECT *pobTran)
{

	inDISP_LogPrintfWithFlag("  inCheckFallbackFlag ginFallBank[%d] TransCode[%d] Service[%s]", ginFallback, pobTran->inTransactionCode, pobTran->srBRec.szServiceCode);

	/* AE晶片卡刷磁條要會過，所以不檢核Service Code */
	if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
	{
		inDISP_DispLogAndWriteFlie(" EMV AE 不檢核Service Code Line[%d] ", __LINE__);
		return (VS_SUCCESS);
	}

	if (ginFallback == VS_TRUE)
	{
		inEMV_SetICCReadFailure(VS_FALSE); /* 進入FALL BACK流程，把global標示要走FallBack的Bit關掉，否則會一直走Fallback */

		/* 不支援晶片卡交易別，不走Fallback流程 */
		if (pobTran->inTransactionCode == _SALE_OFFLINE_ ||
				pobTran->inTransactionCode == _REFUND_ ||
				pobTran->inTransactionCode == _INST_REFUND_ ||
				pobTran->inTransactionCode == _REDEEM_REFUND_ ||
				pobTran->inTransactionCode == _CUP_REFUND_ ||
				pobTran->inTransactionCode == _CUP_MAIL_ORDER_REFUND_ ||
				pobTran->inTransactionCode == _INST_ADJUST_ ||
				pobTran->inTransactionCode == _REDEEM_ADJUST_ ||
				pobTran->inTransactionCode == _ADJUST_ ||
				pobTran->inTransactionCode == _PRE_COMP_ ||
				pobTran->inTransactionCode == _CUP_PRE_COMP_ ||
				pobTran->inTransactionCode == _HG_REWARD_REFUND_ ||
				pobTran->inTransactionCode == _HG_REDEEM_REFUND_)
		{
			inDISP_LogPrintfWithFlag(" Dose not Support Fallback in [inCheckFallbackFlag] TransCode[%d] ", pobTran->inTransactionCode);
			return (VS_SUCCESS);
		} else
		{
			inDISP_LogPrintfWithFlag(" inCheckFallbackFlag 1,%s", pobTran->srBRec.szServiceCode);

			if (memcmp(&pobTran->srBRec.szServiceCode[0], "2", 1) == 0 || memcmp(&pobTran->srBRec.szServiceCode[0], "6", 1) == 0)
			{
				pobTran->srBRec.uszEMVFallBackBit = VS_TRUE;
				inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_FALLBACK_);
			} else
				return (VS_SUCCESS);
		}

		/* 晶片改刷磁條驗證中 */
		inDISP_Msg_BMP(_ERR_FALLBACK_, _COORDINATE_Y_LINE_8_5_, _NO_KEY_MSG_, 4, "", 0);
		return (VS_SUCCESS);
	} else if (pobTran->inTransactionCode != _SALE_OFFLINE_ &&
			pobTran->inTransactionCode != _REFUND_ &&
			pobTran->inTransactionCode != _INST_REFUND_ &&
			pobTran->inTransactionCode != _REDEEM_REFUND_ &&
			pobTran->inTransactionCode != _CUP_REFUND_ &&
			pobTran->inTransactionCode != _CUP_MAIL_ORDER_REFUND_ &&
			pobTran->inTransactionCode != _INST_ADJUST_ &&
			pobTran->inTransactionCode != _REDEEM_ADJUST_ &&
			pobTran->inTransactionCode != _ADJUST_ &&
			pobTran->inTransactionCode != _PRE_COMP_ &&
			pobTran->inTransactionCode != _CUP_PRE_COMP_ &&
			(memcmp(&pobTran->srBRec.szServiceCode[0], "2", 1) == 0 || memcmp(&pobTran->srBRec.szServiceCode[0], "6", 1) == 0)
			)
	{
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			/* 請讀晶片，無晶片 請持卡人洽發卡行 */
			inDISP_Msg_BMP(_ERR_READ_EMV_CUP_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		} else
		{
			/* 請改讀晶片卡 */
			inDISP_Msg_BMP(_ERR_READ_EMV_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}

		pobTran->inECRErrorMsg = VS_CALLBANK; /* 修改ECR 回傳CALL BANK 問題 20190215 [SAM] */
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function	:fICCEvent
Date&Time	:2015/11/23 上午 10:41
Describe	:偵測EMV插卡
 */
int inEMV_ICCEvent(void)
{
	int inRetVal;
	unsigned char uszStatus; /* 偵測晶片用狀態 */

	uszStatus = 0x00;

	/* 偵測晶片插卡事件，需依照各種不同機型去調整，以下為V3作法 */
	inRetVal = CTOS_SCStatus(d_SC_USER, &uszStatus);

	if (inRetVal != d_OK)
	{
		return (VS_ERROR);
	} else
	{
		/* 要用BitMask */
		if ((uszStatus & d_MK_SC_PRESENT) == d_MK_SC_PRESENT)
			return (VS_SUCCESS);
		else
			return (VS_ERROR);
	}
}

/*
Function        :inEMV_CheckRemoveCard
Date&Time       :2016/12/8 上午 11:08
Describe        :目前加在兩個地方 in(host)_RunTRT的尾端和接inFLOW_RunOperation回傳值的地方
 */
int inEMV_CheckRemoveCard(TRANSACTION_OBJECT *pobTran, int inIsError)
{
	int inRetVal;
	int inTimeOut = 180;

	inRetVal = inEMV_ICCEvent();
	if (inRetVal == VS_SUCCESS)
	{
		/* 仍然插著卡片 */
		/* 設定Timeout */
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, inTimeOut);

//		if (inIsError == _REMOVE_CARD_ERROR_)
//		{
//			/* 請退回晶片卡 */
//			inDISP_Msg_BMP(_ERR_PLS_REMOVE_CARD_, _COORDINATE_Y_LINE_8X16_6_, _CLEAR_KEY_MSG_, inTimeOut, "", 0);
//		}
//		else
//		{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_ERR_PLS_REMOVE_CARD_, 0, _COORDINATE_Y_LINE_8_6_);
//		}

	} else
		return (VS_SUCCESS);


	while (1)
	{
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			return (VS_SUCCESS);
		}

		inRetVal = inEMV_ICCEvent();
		if (inRetVal == VS_SUCCESS)
		{
			inDISP_PutGraphic(_ERR_PLS_REMOVE_CARD_, 0, _COORDINATE_Y_LINE_8_6_);
			inDISP_BEEP(1, 1000);
			/* 仍然插著卡片 */
		} else
		{
			return (VS_SUCCESS);
		}
	}

	return (VS_SUCCESS);
}

/*
Function	:inEMV_GetEMVCardData
Date&Time	:2015/12/20 PM 06:30:50
Describe	:進入EMV交易流程
 */
int inEMV_GetEMVCardData(TRANSACTION_OBJECT *pobTran)
{

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "EVM_256 GetEmvCardData INIT");

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	if (pobTran->inTransactionCode == _SALE_OFFLINE_ ||
			pobTran->inTransactionCode == _REFUND_ ||
			pobTran->inTransactionCode == _INST_REFUND_ ||
			pobTran->inTransactionCode == _REDEEM_REFUND_ ||
			pobTran->inTransactionCode == _CUP_REFUND_ ||
			pobTran->inTransactionCode == _CUP_MAIL_ORDER_REFUND_ ||
			pobTran->inTransactionCode == _ADJUST_ ||
			pobTran->inTransactionCode == _INST_ADJUST_ ||
			pobTran->inTransactionCode == _REDEEM_ADJUST_ ||
			pobTran->inTransactionCode == _PRE_COMP_ ||
			pobTran->inTransactionCode == _CUP_PRE_COMP_)
	{
		/* 本交易不接受晶片 請重新交易 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_Msg_BMP(_ERR_EMV_FAIL_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		return (VS_ERROR);
	}

#ifdef _CLER_SCREEN_AND_DISP_TITLE_
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 晶片卡讀取中 */
	inDISP_PutGraphic(_READ_ICC_, 0, _COORDINATE_Y_LINE_8_4_);
#else
	inDISP_PutGraphic(_READ_ICC_, 0, _COORDINATE_Y_LINE_8_4_);
#endif
	/* 跑EMV流程 */
	if (inEMV_SelectICCAID(pobTran) != VS_SUCCESS)
	{
		/* 金融卡沒磁條，不開Fallback */
		if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
		{
			inEMV_SetICCReadFailure(VS_TRUE); /* 開啟FALL BACK */
		}
		inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_TRANS_NOT_FOUND_);
		return (VS_ERROR);
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_256 Atf Run Select Aid");

	if (pobTran->uszFISCBit == VS_TRUE)
	{
		/* SMARTPAY不算晶片卡 */
	}		/* 此為晶片卡 */
	else
	{
		pobTran->srBRec.inChipStatus = _EMV_CARD_;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function	:inEMV_GetEMVTag
Date&Time	:2016/1/9 下午 11:42
Describe	:取得晶片卡EMVTag，若抓不到的會進TermainalDataGet裡面找來塞
 */
int inEMV_GetEMVTag(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;
	char szASCII[128 + 1];
	char szTemplate[20 + 1];
	unsigned short ushTagLen;
	unsigned char uszTagData[128];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inEMV_GetEMVTag()_START");
	}

	if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
	{
		/* 50 */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x50, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz50_APLabel, 0x00, sizeof (pobTran->srEMVRec.usz50_APLabel));
			pobTran->srEMVRec.in50_APLabelLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz50_APLabel[0], &uszTagData[0], pobTran->srEMVRec.in50_APLabelLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				sprintf(szASCII, "%s", pobTran->srEMVRec.usz50_APLabel);
				inCTLS_ISOFormatDebug_DISP("50", pobTran->srEMVRec.in50_APLabelLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				/* AP LABEL本來就是Ascii 不用轉 */
				memset(szASCII, 0x00, sizeof (szASCII));
				sprintf(szASCII, "%s", pobTran->srEMVRec.usz50_APLabel);

				inCTLS_ISOFormatDebug_PRINT("50", pobTran->srEMVRec.in50_APLabelLen, szASCII);
			}

		}

		/* 57 */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x57, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->usz57_Track2, 0x00, sizeof (pobTran->usz57_Track2));
			pobTran->in57_Track2Len = ushTagLen;
			memcpy(&pobTran->usz57_Track2[0], &uszTagData[0], pobTran->in57_Track2Len);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->usz57_Track2[0], pobTran->in57_Track2Len);
				inCTLS_ISOFormatDebug_DISP("57", pobTran->in57_Track2Len, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->usz57_Track2[0], pobTran->in57_Track2Len);
				inCTLS_ISOFormatDebug_PRINT("57", pobTran->in57_Track2Len, szASCII);
			}
		}

		/* 5A */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x5A, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz5A_ApplPan, 0x00, sizeof (pobTran->srEMVRec.usz5A_ApplPan));
			pobTran->srEMVRec.in5A_ApplPanLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz5A_ApplPan[0], &uszTagData[0], pobTran->srEMVRec.in5A_ApplPanLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz5A_ApplPan[0], pobTran->srEMVRec.in5A_ApplPanLen);
				inCTLS_ISOFormatDebug_DISP("5A", pobTran->srEMVRec.in5A_ApplPanLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz5A_ApplPan[0], pobTran->srEMVRec.in5A_ApplPanLen);
				inCTLS_ISOFormatDebug_PRINT("5A", pobTran->srEMVRec.in5A_ApplPanLen, szASCII);
			}

		}

		/* 5F20 */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x5F20, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz5F20_CardholderName, 0x00, sizeof (pobTran->srEMVRec.usz5F20_CardholderName));
			pobTran->srEMVRec.in5F20_CardholderNameLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz5F20_CardholderName[0], &uszTagData[0], pobTran->srEMVRec.in5F20_CardholderNameLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				memcpy(&szASCII[0], &pobTran->srEMVRec.usz5F20_CardholderName[0], pobTran->srEMVRec.in5F20_CardholderNameLen);
				inCTLS_ISOFormatDebug_DISP("5F20", pobTran->srEMVRec.in5F20_CardholderNameLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				memcpy(&szASCII[0], &pobTran->srEMVRec.usz5F20_CardholderName[0], pobTran->srEMVRec.in5F20_CardholderNameLen);
				inCTLS_ISOFormatDebug_PRINT("5F20", pobTran->srEMVRec.in5F20_CardholderNameLen, szASCII);
			}

		}

		/* 5F24 */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x5F24, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz5F24_ExpireDate, 0x00, sizeof (pobTran->srEMVRec.usz5F24_ExpireDate));
			pobTran->srEMVRec.in5F24_ExpireDateLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz5F24_ExpireDate[0], &uszTagData[0], pobTran->srEMVRec.in5F24_ExpireDateLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz5F24_ExpireDate[0], pobTran->srEMVRec.in5F24_ExpireDateLen);
				inCTLS_ISOFormatDebug_DISP("5F24", pobTran->srEMVRec.in5F24_ExpireDateLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				inCTLS_ISOFormatDebug_PRINT("5F24", pobTran->srEMVRec.in5F24_ExpireDateLen, "-----");
			}

		}

		/* 5F2A */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x5F2A, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz5F2A_TransCurrCode, 0x00, sizeof (pobTran->srEMVRec.usz5F2A_TransCurrCode));
			pobTran->srEMVRec.in5F2A_TransCurrCodeLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz5F2A_TransCurrCode[0], &uszTagData[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz5F2A_TransCurrCode[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);

				inCTLS_ISOFormatDebug_DISP("5F2A", pobTran->srEMVRec.in5F2A_TransCurrCodeLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz5F2A_TransCurrCode[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);

				inCTLS_ISOFormatDebug_PRINT("5F2A", pobTran->srEMVRec.in5F2A_TransCurrCodeLen, szASCII);
			}

		}

		/* 5F34 */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x5F34, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz5F34_ApplPanSeqnum, 0x00, sizeof (pobTran->srEMVRec.usz5F34_ApplPanSeqnum));
			pobTran->srEMVRec.in5F34_ApplPanSeqnumLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], &uszTagData[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);

				inCTLS_ISOFormatDebug_DISP("5F34", pobTran->srEMVRec.in5F34_ApplPanSeqnumLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);

				inCTLS_ISOFormatDebug_PRINT("5F34", pobTran->srEMVRec.in5F34_ApplPanSeqnumLen, szASCII);
			}

		}

		/* 82 */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x82, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz82_AIP, 0x00, sizeof (pobTran->srEMVRec.usz82_AIP));
			pobTran->srEMVRec.in82_AIPLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz82_AIP[0], &uszTagData[0], pobTran->srEMVRec.in82_AIPLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz82_AIP[0], pobTran->srEMVRec.in82_AIPLen);

				inCTLS_ISOFormatDebug_DISP("82", pobTran->srEMVRec.in82_AIPLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz82_AIP[0], pobTran->srEMVRec.in82_AIPLen);

				inCTLS_ISOFormatDebug_PRINT("82", pobTran->srEMVRec.in82_AIPLen, szASCII);
			}

		}

		/* 84 */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x84, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz84_DF_NAME, 0x00, sizeof (pobTran->srEMVRec.usz84_DF_NAME));
			pobTran->srEMVRec.in84_DFNameLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz84_DF_NAME[0], &uszTagData[0], pobTran->srEMVRec.in84_DFNameLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);

				inCTLS_ISOFormatDebug_DISP("84", pobTran->srEMVRec.in84_DFNameLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);

				inCTLS_ISOFormatDebug_PRINT("84", pobTran->srEMVRec.in84_DFNameLen, szASCII);
			}

		}

		/* 8A */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x8A, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz8A_AuthRespCode, 0x00, sizeof (pobTran->srEMVRec.usz8A_AuthRespCode));
			pobTran->srEMVRec.in8A_AuthRespCodeLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz8A_AuthRespCode[0], &uszTagData[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);

				inCTLS_ISOFormatDebug_DISP("8A", pobTran->srEMVRec.in8A_AuthRespCodeLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);

				inCTLS_ISOFormatDebug_PRINT("8A", pobTran->srEMVRec.in8A_AuthRespCodeLen, szASCII);
			}

		}

		/* 95 */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x95, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz95_TVR, 0x00, sizeof (pobTran->srEMVRec.usz95_TVR));
			pobTran->srEMVRec.in95_TVRLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz95_TVR[0], &uszTagData[0], pobTran->srEMVRec.in95_TVRLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz95_TVR[0], pobTran->srEMVRec.in95_TVRLen);

				inCTLS_ISOFormatDebug_DISP("95", pobTran->srEMVRec.in95_TVRLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz95_TVR[0], pobTran->srEMVRec.in95_TVRLen);

				inCTLS_ISOFormatDebug_PRINT("95", pobTran->srEMVRec.in95_TVRLen, szASCII);
			}

		}

		/* 9A */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x9A, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz9A_TranDate, 0x00, sizeof (pobTran->srEMVRec.usz9A_TranDate));
			pobTran->srEMVRec.in9A_TranDateLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz9A_TranDate[0], &uszTagData[0], pobTran->srEMVRec.in9A_TranDateLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);

				inCTLS_ISOFormatDebug_DISP("9A", pobTran->srEMVRec.in9A_TranDateLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);

				inCTLS_ISOFormatDebug_PRINT("9A", pobTran->srEMVRec.in9A_TranDateLen, szASCII);
			}
		}

		/* 9B */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x9B, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz9B_TSI, 0x00, sizeof (pobTran->srEMVRec.usz9B_TSI));
			pobTran->srEMVRec.in9B_TSILen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz9B_TSI[0], &uszTagData[0], pobTran->srEMVRec.in9B_TSILen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);

				inCTLS_ISOFormatDebug_DISP("9B", pobTran->srEMVRec.in9B_TSILen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);

				inCTLS_ISOFormatDebug_PRINT("9B", pobTran->srEMVRec.in9B_TSILen, szASCII);
			}

		}

		/* 9C */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x9C, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz9C_TranType, 0x00, sizeof (pobTran->srEMVRec.usz9C_TranType));
			pobTran->srEMVRec.in9C_TranTypeLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz9C_TranType[0], &uszTagData[0], pobTran->srEMVRec.in9C_TranTypeLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9C_TranType[0], pobTran->srEMVRec.in9C_TranTypeLen);

				inCTLS_ISOFormatDebug_DISP("9C", pobTran->srEMVRec.in9C_TranTypeLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9C_TranType[0], pobTran->srEMVRec.in9C_TranTypeLen);

				inCTLS_ISOFormatDebug_PRINT("9C", pobTran->srEMVRec.in9C_TranTypeLen, szASCII);
			}
		}

		/* 9F02 */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x9F02, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz9F02_AmtAuthNum, 0x00, sizeof (pobTran->srEMVRec.usz9F02_AmtAuthNum));
			pobTran->srEMVRec.in9F02_AmtAuthNumLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz9F02_AmtAuthNum[0], &uszTagData[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);

				inCTLS_ISOFormatDebug_DISP("9F02", pobTran->srEMVRec.in9F02_AmtAuthNumLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);

				inCTLS_ISOFormatDebug_PRINT("9F02", pobTran->srEMVRec.in9F02_AmtAuthNumLen, szASCII);
			}

		}

		/* 9F03 */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x9F03, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz9F03_AmtOtherNum, 0x00, sizeof (pobTran->srEMVRec.usz9F03_AmtOtherNum));
			pobTran->srEMVRec.in9F03_AmtOtherNumLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz9F03_AmtOtherNum[0], &uszTagData[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F03_AmtOtherNum[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);

				inCTLS_ISOFormatDebug_DISP("9F03", pobTran->srEMVRec.in9F03_AmtOtherNumLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F03_AmtOtherNum[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);

				inCTLS_ISOFormatDebug_PRINT("9F03", pobTran->srEMVRec.in9F03_AmtOtherNumLen, szASCII);
			}

		}

		/* 9F08 */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x9F08, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz9F08_AppVerNumICC, 0x00, sizeof (pobTran->srEMVRec.usz9F08_AppVerNumICC));
			pobTran->srEMVRec.in9F08_AppVerNumICCLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz9F08_AppVerNumICC[0], &uszTagData[0], pobTran->srEMVRec.in9F08_AppVerNumICCLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);

				inCTLS_ISOFormatDebug_DISP("9F08", ushTagLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);

				inCTLS_ISOFormatDebug_PRINT("9F08", ushTagLen, szASCII);
			}

		}

		/* 9F09 */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x9F09, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz9F09_TermVerNum, 0x00, sizeof (pobTran->srEMVRec.usz9F09_TermVerNum));
			pobTran->srEMVRec.in9F09_TermVerNumLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz9F09_TermVerNum[0], &uszTagData[0], pobTran->srEMVRec.in9F09_TermVerNumLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);

				inCTLS_ISOFormatDebug_DISP("9F09", ushTagLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);

				inCTLS_ISOFormatDebug_PRINT("9F09", ushTagLen, szASCII);
			}

		}

		/* 9F10 */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x9F10, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz9F10_IssuerAppData, 0x00, sizeof (pobTran->srEMVRec.usz9F10_IssuerAppData));
			pobTran->srEMVRec.in9F10_IssuerAppDataLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz9F10_IssuerAppData[0], &uszTagData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F10_IssuerAppData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);

				inCTLS_ISOFormatDebug_DISP("9F10", pobTran->srEMVRec.in9F10_IssuerAppDataLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F10_IssuerAppData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);

				inCTLS_ISOFormatDebug_PRINT("9F10", pobTran->srEMVRec.in9F10_IssuerAppDataLen, szASCII);
			}

		}

		/* 9F1A */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x9F1A, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz9F1A_TermCountryCode, 0x00, sizeof (pobTran->srEMVRec.usz9F1A_TermCountryCode));
			pobTran->srEMVRec.in9F1A_TermCountryCodeLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz9F1A_TermCountryCode[0], &uszTagData[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F1A_TermCountryCode[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);

				inCTLS_ISOFormatDebug_DISP("9F1A", pobTran->srEMVRec.in9F1A_TermCountryCodeLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F1A_TermCountryCode[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);

				inCTLS_ISOFormatDebug_PRINT("9F1A", pobTran->srEMVRec.in9F1A_TermCountryCodeLen, szASCII);
			}

		}

		/* 9F1E */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x9F1E, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz9F1E_IFDNum, 0x00, sizeof (pobTran->srEMVRec.usz9F1E_IFDNum));
			pobTran->srEMVRec.in9F1E_IFDNumLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz9F1E_IFDNum[0], &uszTagData[0], pobTran->srEMVRec.in9F1E_IFDNumLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);

				inCTLS_ISOFormatDebug_DISP("9F1E", pobTran->srEMVRec.in9F1E_IFDNumLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);

				inCTLS_ISOFormatDebug_PRINT("9F1E", pobTran->srEMVRec.in9F1E_IFDNumLen, szASCII);
			}

		}

		/* [富邦新增 TAG] */
		/* 9F1F */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x9F1F, &ushTagLen, uszTagData);
#ifdef _DEBUG_9F1F_TAG_
		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(gusz9F1F, 0x00, sizeof (gusz9F1F));
			gln9F1FLen = ushTagLen;
			memcpy(&gusz9F1F[0], &uszTagData[0], gln9F1FLen);
			
//			inPRINT_ChineseFont("Chip Card", _PRT_ISO_);
//			memset(szASCII, 0x00, sizeof (szASCII));
//			inFunc_BCD_to_ASCII(&szASCII[0], uszTagData, ushTagLen);
//			inCTLS_ISOFormatDebug_PRINT("9F1F", ushTagLen, szASCII);
		}
#endif

		/* 9F26 */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x9F26, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz9F26_ApplCryptogram, 0x00, sizeof (pobTran->srEMVRec.usz9F26_ApplCryptogram));
			pobTran->srEMVRec.in9F26_ApplCryptogramLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz9F26_ApplCryptogram[0], &uszTagData[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F26_ApplCryptogram[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);

				inCTLS_ISOFormatDebug_DISP("9F26", pobTran->srEMVRec.in9F26_ApplCryptogramLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F26_ApplCryptogram[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);

				inCTLS_ISOFormatDebug_PRINT("9F26", pobTran->srEMVRec.in9F26_ApplCryptogramLen, szASCII);
			}

		}

		/* 9F27 */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x9F27, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz9F27_CID, 0x00, sizeof (pobTran->srEMVRec.usz9F27_CID));
			pobTran->srEMVRec.in9F27_CIDLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz9F27_CID[0], &uszTagData[0], pobTran->srEMVRec.in9F27_CIDLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F27_CID[0], pobTran->srEMVRec.in9F27_CIDLen);

				inCTLS_ISOFormatDebug_DISP("9F27", pobTran->srEMVRec.in9F27_CIDLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F27_CID[0], pobTran->srEMVRec.in9F27_CIDLen);

				inCTLS_ISOFormatDebug_PRINT("9F27", pobTran->srEMVRec.in9F27_CIDLen, szASCII);
			}

		}

		/* 9F33 */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x9F33, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz9F33_TermCapabilities, 0x00, sizeof (pobTran->srEMVRec.usz9F33_TermCapabilities));
			pobTran->srEMVRec.in9F33_TermCapabilitiesLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz9F33_TermCapabilities[0], &uszTagData[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F33_TermCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);

				inCTLS_ISOFormatDebug_DISP("9F33", pobTran->srEMVRec.in9F33_TermCapabilitiesLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F33_TermCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);

				inCTLS_ISOFormatDebug_PRINT("9F33", pobTran->srEMVRec.in9F33_TermCapabilitiesLen, szASCII);
			}

		}

		/* 9F34 */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x9F34, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz9F34_CVM, 0x00, sizeof (pobTran->srEMVRec.usz9F34_CVM));
			pobTran->srEMVRec.in9F34_CVMLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz9F34_CVM[0], &uszTagData[0], pobTran->srEMVRec.in9F34_CVMLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);

				inCTLS_ISOFormatDebug_DISP("9F34", pobTran->srEMVRec.in9F34_CVMLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);

				inCTLS_ISOFormatDebug_PRINT("9F34", pobTran->srEMVRec.in9F34_CVMLen, szASCII);
			}

		}

		/* 9F35 */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x9F35, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz9F35_TermType, 0x00, sizeof (pobTran->srEMVRec.usz9F35_TermType));
			pobTran->srEMVRec.in9F35_TermTypeLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz9F35_TermType[0], &uszTagData[0], pobTran->srEMVRec.in9F35_TermTypeLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);

				inCTLS_ISOFormatDebug_DISP("9F35", pobTran->srEMVRec.in9F35_TermTypeLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);

				inCTLS_ISOFormatDebug_PRINT("9F35", pobTran->srEMVRec.in9F35_TermTypeLen, szASCII);
			}

		}

		/* 9F36 */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x9F36, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz9F36_ATC, 0x00, sizeof (pobTran->srEMVRec.usz9F36_ATC));
			pobTran->srEMVRec.in9F36_ATCLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz9F36_ATC[0], &uszTagData[0], pobTran->srEMVRec.in9F36_ATCLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F36_ATC[0], pobTran->srEMVRec.in9F36_ATCLen);

				inCTLS_ISOFormatDebug_DISP("9F36", pobTran->srEMVRec.in9F36_ATCLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F36_ATC[0], pobTran->srEMVRec.in9F36_ATCLen);

				inCTLS_ISOFormatDebug_PRINT("9F36", pobTran->srEMVRec.in9F36_ATCLen, szASCII);
			}

		}

		/* 9F37 */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x9F37, &ushTagLen, uszTagData);
		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz9F37_UnpredictNum, 0x00, sizeof (pobTran->srEMVRec.usz9F37_UnpredictNum));
			pobTran->srEMVRec.in9F37_UnpredictNumLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz9F37_UnpredictNum[0], &uszTagData[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F37_UnpredictNum[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);

				inCTLS_ISOFormatDebug_DISP("9F37", pobTran->srEMVRec.in9F37_UnpredictNumLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F37_UnpredictNum[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);

				inCTLS_ISOFormatDebug_PRINT("9F37", pobTran->srEMVRec.in9F37_UnpredictNumLen, szASCII);
			}

		}

		/* 9F41 */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x9F41, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz9F41_TransSeqCounter, 0x00, sizeof (pobTran->srEMVRec.usz9F41_TransSeqCounter));
			pobTran->srEMVRec.in9F41_TransSeqCounterLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], &uszTagData[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);

				inCTLS_ISOFormatDebug_DISP("9F41", pobTran->srEMVRec.in9F41_TransSeqCounterLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);

				inCTLS_ISOFormatDebug_PRINT("9F41", pobTran->srEMVRec.in9F41_TransSeqCounterLen, szASCII);
			}

		}

		/* 9F5B不在這邊抓，在online完的OnTxnIssuerScriptResult抓 */

		if (pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
		{
			/* 現在不再送DFEC了 */
			/* DFEC */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEC_FALLBACK_INDICATOR, (unsigned char *) &szTemplate[0]) > 0)
			{
				memset(pobTran->srEMVRec.uszDFEC_FallBackIndicator, 0x00, sizeof (pobTran->srEMVRec.uszDFEC_FallBackIndicator));
				pobTran->srEMVRec.inDFEC_FallBackIndicatorLen = szTemplate[2];
				memcpy((char *) &pobTran->srEMVRec.uszDFEC_FallBackIndicator[0], &szTemplate[3], pobTran->srEMVRec.inDFEC_FallBackIndicatorLen);
			}
			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFEC_FallBackIndicator[0], pobTran->srEMVRec.inDFEC_FallBackIndicatorLen);

				inCTLS_ISOFormatDebug_DISP("DFEC", pobTran->srEMVRec.inDFEC_FallBackIndicatorLen, szASCII);
			}
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFEC_FallBackIndicator[0], pobTran->srEMVRec.inDFEC_FallBackIndicatorLen);

				inCTLS_ISOFormatDebug_PRINT("DFEC", pobTran->srEMVRec.inDFEC_FallBackIndicatorLen, szASCII);
			}

			/* DFED */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFED_CHIP_CONDITION_CODE, (unsigned char *) &szTemplate[0]) > 0)
			{
				memset(pobTran->srEMVRec.uszDFED_ChipConditionCode, 0x00, sizeof (pobTran->srEMVRec.uszDFED_ChipConditionCode));
				pobTran->srEMVRec.inDFED_ChipConditionCodeLen = szTemplate[2];
				memcpy((char *) &pobTran->srEMVRec.uszDFED_ChipConditionCode[0], &szTemplate[3], pobTran->srEMVRec.inDFED_ChipConditionCodeLen);
			}
			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFED_ChipConditionCode[0], pobTran->srEMVRec.inDFED_ChipConditionCodeLen);

				inCTLS_ISOFormatDebug_DISP("DFED", pobTran->srEMVRec.inDFED_ChipConditionCodeLen, szASCII);
			}
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFED_ChipConditionCode[0], pobTran->srEMVRec.inDFED_ChipConditionCodeLen);

				inCTLS_ISOFormatDebug_PRINT("DFED", pobTran->srEMVRec.inDFED_ChipConditionCodeLen, szASCII);
			}

			/* DFEE */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEE_TERMINAL_ENTRY_CAPABILITY, (unsigned char *) &szTemplate[0]) > 0)
			{
				memset(pobTran->srEMVRec.uszDFEE_TerEntryCap, 0x00, sizeof (pobTran->srEMVRec.uszDFEE_TerEntryCap));
				pobTran->srEMVRec.inDFEE_TerEntryCapLen = szTemplate[2];
				memcpy((char *) &pobTran->srEMVRec.uszDFEE_TerEntryCap[0], &szTemplate[3], pobTran->srEMVRec.inDFEE_TerEntryCapLen);
			}
			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFEE_TerEntryCap[0], pobTran->srEMVRec.inDFED_ChipConditionCodeLen);

				inCTLS_ISOFormatDebug_DISP("DFEE", pobTran->srEMVRec.inDFED_ChipConditionCodeLen, szASCII);
			}
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFEE_TerEntryCap[0], pobTran->srEMVRec.inDFED_ChipConditionCodeLen);

				inCTLS_ISOFormatDebug_PRINT("DFEE", pobTran->srEMVRec.inDFED_ChipConditionCodeLen, szASCII);
			}

			/* DFEF */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEF_REASON_ONLINE_CODE, (unsigned char *) &szTemplate[0]) > 0)
			{
				memset(pobTran->srEMVRec.uszDFEF_ReasonOnlineCode, 0x00, sizeof (pobTran->srEMVRec.uszDFEF_ReasonOnlineCode));
				pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen = szTemplate[2];
				memcpy((char *) &pobTran->srEMVRec.uszDFEF_ReasonOnlineCode[0], &szTemplate[3], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);
			}
			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFEF_ReasonOnlineCode[0], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);

				inCTLS_ISOFormatDebug_DISP("DFEF", pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen, szASCII);
			}
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFEF_ReasonOnlineCode[0], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);

				inCTLS_ISOFormatDebug_PRINT("DFEF", pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen, szASCII);
			}

		} else
		{
			if (pobTran->inTransactionCode == _SALE_ ||
					pobTran->inTransactionCode == _REDEEM_SALE_ ||
					pobTran->inTransactionCode == _INST_SALE_ ||
					pobTran->inTransactionCode == _PRE_AUTH_ ||
					pobTran->inTransactionCode == _TC_UPLOAD_ ||
					pobTran->inTransactionCode == _CASH_ADVANCE_ ||
					pobTran->inTransactionCode == _CUP_SALE_ ||
					pobTran->inTransactionCode == _CUP_PRE_AUTH_)
			{
				/* DFEE */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEE_TERMINAL_ENTRY_CAPABILITY, (unsigned char *) szTemplate) > 0)
				{
					memset(pobTran->srEMVRec.uszDFEE_TerEntryCap, 0x00, sizeof (pobTran->srEMVRec.uszDFEE_TerEntryCap));
					pobTran->srEMVRec.inDFEE_TerEntryCapLen = szTemplate[2];
					memcpy((char *) &pobTran->srEMVRec.uszDFEE_TerEntryCap[0], &szTemplate[3], pobTran->srEMVRec.inDFEE_TerEntryCapLen);
				}

				/* DFEF */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEF_REASON_ONLINE_CODE, (unsigned char *) szTemplate) > 0)
				{
					memset(pobTran->srEMVRec.uszDFEF_ReasonOnlineCode, 0x00, sizeof (pobTran->srEMVRec.uszDFEF_ReasonOnlineCode));
					pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen = szTemplate[2];
					memcpy((char *) &pobTran->srEMVRec.uszDFEF_ReasonOnlineCode[0], &szTemplate[3], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);
				}

			}

		}

	}

	/* Tag 9F63 若UICC卡片包含此Tag 時， EDC則應支援該Tag上傳。 */
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		/* 9F63 */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(0x9F63, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			memset(pobTran->srEMVRec.usz9F63_CardProductLabelInformation, 0x00, sizeof (pobTran->srEMVRec.usz9F63_CardProductLabelInformation));
			pobTran->srEMVRec.in9F63_CardProductLabelInformationLen = ushTagLen;
			memcpy(&pobTran->srEMVRec.usz9F63_CardProductLabelInformation[0], &uszTagData[0], pobTran->srEMVRec.in9F63_CardProductLabelInformationLen);

			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F63_CardProductLabelInformation[0], pobTran->srEMVRec.in9F63_CardProductLabelInformationLen);

				inCTLS_ISOFormatDebug_DISP("9F63", pobTran->srEMVRec.in9F63_CardProductLabelInformationLen, szASCII);
			}

			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				memset(szASCII, 0x00, sizeof (szASCII));
				inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F63_CardProductLabelInformation[0], pobTran->srEMVRec.in9F63_CardProductLabelInformationLen);

				inCTLS_ISOFormatDebug_PRINT("9F63", pobTran->srEMVRec.in9F63_CardProductLabelInformationLen, szASCII);
			}

		}
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inEMV_GetEMVTag()_END");
	}

	return (VS_SUCCESS);
}

/*
Function        :inEMV_CreditPowerON
Date&Time       :2017/7/14 上午 11:42
Describe        :
 */
int inEMV_CreditPowerON(TRANSACTION_OBJECT *pobTran)
{
	char szDebugMsg[100 + 1];
	unsigned short usRetVal;
	unsigned char szATR[128 + 1], szATRLen, szCardType;

	szATRLen = sizeof (szATR);
	//Power on the ICC and retrun the ATR contents metting the EMV2000 specification //
	usRetVal = CTOS_SCResetEMV(d_SC_USER, d_SC_5V, szATR, &szATRLen, &szCardType);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("CTOS_SCResetEMV ERROR!!");

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "RetVal: 0x%04x", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}

		if (usRetVal == d_SC_NOT_PRESENT)
		{
			return (VS_EMV_CARD_OUT);
		} else
		{
			return (VS_ERROR);
		}
	}

	return (VS_SUCCESS);
}

/*
Function        :inCard_CreditPowerOFF
Date&Time       :
Describe        :
 */
int inEMV_CreditPowerOFF(TRANSACTION_OBJECT *pobTran)
{
	if (CTOS_SCPowerOff(d_SC_USER) != d_OK)
	{
		inDISP_DispLogAndWriteFlie(" EMV SCPower OFF *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inCard_CreditSelectAID
Date&Time       :
Describe        :
 */
int inEMV_CreditSelectAID(TRANSACTION_OBJECT *pobTran, int inContactType)
{
	int inRetVal = 0;
	int i = 0;
	int inAIDLen = 0;
	char szMVTAID[20 + 1] = {0}, szHexAID[13 + 1] = {0};
        
        int			inCnt = 0;
	int			inRemainLen = 0;
	int			inTagLen = 0;
	int			inLevel = 0;
	char			szASCII[2048 + 1] = {0};
	char			szDebugMsg[100 + 1] = {0};
	char			szTag[2 + 1] = {0};
//        char			szMVTAID[20 + 1] = {0}, szHexAID[13 + 1] = {0};
	unsigned char		uszPPSE_SuccessBit = VS_FALSE;
	unsigned char		uszFind4FBit = VS_FALSE;
	unsigned char		uszTag2ByteBit = VS_FALSE;
        
	APDU_COMMAND srAPDU_COMMAND;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inEMV_CreditSelectAID()_START");
	}

               /* 20220801鈺翔 有些卡select AID會失敗 所以先select PPSE判斷是不是信用卡 */
	/* 先select PPSE */
	if (inContactType == _CONTACT_TYPE_01_CONTACTLESS_)
	{
		/* 2PAY.SYS.DDF01 */
		memset(szMVTAID, 0x00, sizeof(szMVTAID));
		memset(szHexAID, 0x00, sizeof(szHexAID));
		sprintf(szMVTAID, "2PAY.SYS.DDF01");
		inAIDLen = strlen(szMVTAID);
		
		/* APDU Command */
		memset(&srAPDU_COMMAND, 0x00, sizeof(APDU_COMMAND));

		srAPDU_COMMAND.uszCommandINSData[0] = _FISC_SELECT_AID_CLA_COMMAND_;		/* CLA */
		srAPDU_COMMAND.uszCommandINSData[0] = _FISC_SELECT_AID_INS_COMMAND_;		/* INS */
		srAPDU_COMMAND.uszCommandP1Data[0] = _FISC_SELECT_AID_P1_COMMAND_;		/* P1 */
		srAPDU_COMMAND.uszCommandP2Data[0] = _FISC_SELECT_AID_P2_COMMAND_;		/* P2 */
		srAPDU_COMMAND.inCommandDataLen = inAIDLen;
		memcpy(srAPDU_COMMAND.uszCommandData, szMVTAID, inAIDLen);
		
		if (inAPDU_BuildAPDU(&srAPDU_COMMAND) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		
		inRetVal = inAPDU_Send_APDU_CTLS_Process(&srAPDU_COMMAND);
		
		if (inRetVal == VS_SUCCESS)
		{
			/* 6283 是鎖卡，需要到EMV執行步驟再回傳錯誤訊息 d_EMVAPLIB_ERR_ONLY_1_AP_NO_FALLBACK */
			if ((srAPDU_COMMAND.uszRecevData[srAPDU_COMMAND.inRecevLen -2] == 0x90 && srAPDU_COMMAND.uszRecevData[srAPDU_COMMAND.inRecevLen -1] == 0x00))
			{
				uszPPSE_SuccessBit = VS_TRUE;
			}
			else
			{
				uszPPSE_SuccessBit = VS_FALSE;
			}
		}
		else
		{
			/* 不支援PPSE，繼續select */
			uszPPSE_SuccessBit = VS_FALSE;
		}
	}
	else
	{
//		/* 1PAY.SYS.DDF01 */
//		memset(szMVTAID, 0x00, sizeof(szMVTAID));
//		sprintf(szMVTAID, "1PAY.SYS.DDF01");
//		inAIDLen = strlen(szMVTAID);
//		
//		/* APDU Command */
//                memset(&srAPDU_COMMAND, 0x00, sizeof(APDU_COMMAND));
//
//		srAPDU_COMMAND.uszCommandINSData[0] = _FISC_SELECT_AID_CLA_COMMAND_;		/* CLA */
//                srAPDU_COMMAND.uszCommandINSData[0] = _FISC_SELECT_AID_INS_COMMAND_;		/* INS */
//		srAPDU_COMMAND.uszCommandP1Data[0] = _FISC_SELECT_AID_P1_COMMAND_;		/* P1 */
//		srAPDU_COMMAND.uszCommandP2Data[0] = _FISC_SELECT_AID_P2_COMMAND_;		/* P2 */
//		srAPDU_COMMAND.inCommandDataLen = inAIDLen;
//		memcpy(srAPDU_COMMAND.uszCommandData, szMVTAID, inAIDLen);
//		
//                if (inAPDU_BuildAPDU(&srAPDU_COMMAND) != VS_SUCCESS)
//		{
//			return (VS_ERROR);
//		}
//		
//		inRetVal = inAPDU_Send_APDU_User_Slot_Process(&srAPDU_COMMAND);
//		
//		if (inRetVal == VS_SUCCESS)
//                {
//			/* 6283 是鎖卡，需要到EMV執行步驟再回傳錯誤訊息 d_EMVAPLIB_ERR_ONLY_1_AP_NO_FALLBACK */
//                        if ((srAPDU_COMMAND.uszRecevData[srAPDU_COMMAND.inRecevLen -2] == 0x90 && srAPDU_COMMAND.uszRecevData[srAPDU_COMMAND.inRecevLen -1] == 0x00))
//                        {
//				uszPPSE_SuccessBit = VS_TRUE;
//			}
//			else
//			{
//				uszPPSE_SuccessBit = VS_FALSE;
//			}
//		}
//		else
//		{
//			/* 不支援PPSE，繼續select */
//			uszPPSE_SuccessBit = VS_FALSE;
//		}
		
		/* 晶片卡不Select PSE，感應卡要優先select PPSE是因為修改統一JCB卡問題 */
		uszPPSE_SuccessBit = VS_FALSE;
	}
	
	/* 分析PPSE內容 */
	if (uszPPSE_SuccessBit == VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PPSE ChipDataLen = %d", srAPDU_COMMAND.inRecevLen);
			inDISP_LogPrintf(AT, szDebugMsg);

			memset(szASCII, 0x00, sizeof(szASCII));
			inFunc_BCD_to_ASCII(&szASCII[0], &srAPDU_COMMAND.uszRecevData[0], srAPDU_COMMAND.inRecevLen);

			/* 把整個chipData印出來 */
			inDISP_LogPrintf(AT, "0123456789012345678901234567890123456789");
			inDISP_LogPrintf(AT, "========================================");
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));

			for (inCnt = 0; inCnt < (srAPDU_COMMAND.inRecevLen * 2); inCnt++)
			{
				memcpy(&szDebugMsg[i], &szASCII[inCnt], 1);

				i ++;

				if (i == 40)
				{
					inDISP_LogPrintf(AT, szDebugMsg);
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					i = 0;
				}
			}

			inDISP_LogPrintf(AT, szDebugMsg);
			inDISP_LogPrintf(AT, "----------------------------------------");

			inCnt = 0;
			i = 0;
		}
		
		inLevel = 0;
		inCnt = 0;
		inRemainLen = srAPDU_COMMAND.inRecevLen;
		
		if (inContactType == _CONTACT_TYPE_01_CONTACTLESS_)
		{
			while (inRemainLen > 0)
			{
				memset(szTag, 0x00, sizeof(szTag));
				inTagLen = 0;
				uszTag2ByteBit = VS_FALSE;

				if (srAPDU_COMMAND.uszRecevData[inCnt] == (unsigned char)'\x8F'	||
				    srAPDU_COMMAND.uszRecevData[inCnt] == (unsigned char)'\xBF')
				{
					memcpy(szTag, &srAPDU_COMMAND.uszRecevData[inCnt], 2);
					inTagLen = srAPDU_COMMAND.uszRecevData[inCnt + 2];
					uszTag2ByteBit = VS_TRUE;
				}
				else
				{
					szTag[0] = srAPDU_COMMAND.uszRecevData[inCnt];
					inTagLen = srAPDU_COMMAND.uszRecevData[inCnt + 1];
				}

				/* 先找6F */
				if (inLevel == 0)
				{
					if (szTag[0] == '\x6F')
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += 3;
						}
						else
						{
							inCnt += 2;
						}
						inRemainLen = inTagLen;
						inLevel++;
					}
					else
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += inTagLen + 3;
							inRemainLen -= inTagLen + 3;
						}
						else
						{
							inCnt += inTagLen + 2;
							inRemainLen -= inTagLen + 2;
						}

					}
				}
				/* 再找A5 */
				else if (inLevel == 1)
				{
					if (szTag[0] == '\xA5')
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += 3;
						}
						else
						{
							inCnt += 2;
						}
						inRemainLen = inTagLen;
						inLevel++;
					}
					else
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += inTagLen + 3;
							inRemainLen -= inTagLen + 3;
						}
						else
						{
							inCnt += inTagLen + 2;
							inRemainLen -= inTagLen + 2;
						}

					}
				}
				/* 再找BF 0C */
				else if (inLevel == 2)
				{
					if (memcmp(szTag, "\xBF\x0C", 2) == 0)
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += 3;
						}
						else
						{
							inCnt += 2;
						}
						inRemainLen = inTagLen;
						inLevel++;
					}
					else
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += inTagLen + 3;
							inRemainLen -= inTagLen + 3;
						}
						else
						{
							inCnt += inTagLen + 2;
							inRemainLen -= inTagLen + 2;
						}

					}
				}
				/* 再找61 */
				else if (inLevel == 3)
				{
					if (szTag[0] == '\x61')
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += 3;
						}
						else
						{
							inCnt += 2;
						}
						inRemainLen = inTagLen;
						inLevel++;
					}
					else
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += inTagLen + 3;
							inRemainLen -= inTagLen + 3;
						}
						else
						{
							inCnt += inTagLen + 2;
							inRemainLen -= inTagLen + 2;
						}

					}
				}
				/* 再找4F */
				else if (inLevel == 4)
				{
					if (szTag[0] == '\x4F')
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += 3;
						}
						else
						{
							inCnt += 2;
						}
						inRemainLen = inTagLen;
						inLevel++;
						uszFind4FBit = VS_TRUE;
						break;
					}
					else
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += inTagLen + 3;
							inRemainLen -= inTagLen + 3;
						}
						else
						{
							inCnt += inTagLen + 2;
							inRemainLen -= inTagLen + 2;
						}

					}
				}
			}
		}
		else
		{
			/* 未寫非接觸式部份，直接設FALSE */
			uszFind4FBit = VS_FALSE;
		}
		
		if (uszFind4FBit == VS_TRUE)
		{
			memset(szHexAID, 0x00, sizeof(szHexAID));
			memcpy(szHexAID, &srAPDU_COMMAND.uszRecevData[inCnt], inTagLen);
			
			if(!memcmp(&szHexAID[0], "\xA0\x00\x00\x00\x03", 5)	|| //_EMV_AID_VISA_HEX_
                           !memcmp(&szHexAID[0], "\xA0\x00\x00\x00\x04", 5)	|| //_EMV_AID_MASTERCARD_HEX_
                           !memcmp(&szHexAID[0], "\xA0\x00\x00\x00\x65", 5)	|| //_EMV_AID_JCB_HEX_
			   !memcmp(&szHexAID[0], "\xA0\x00\x00\x00\x25", 5)	|| //_EMV_AID_AMEX_HEX_
			   !memcmp(&szHexAID[0], "\xA0\x00\x00\x01\x52", 5))         //_EMV_AID_DISCOVER_HEX_
			{
				if (ginDebug == VS_TRUE)
                                {
					inDISP_LogPrintf(AT,"pobTran->uszCreditBit = VS_TRUE");
                                } 
                
                                /* 確認為信用卡 */
                                pobTran->uszCreditBit = VS_TRUE;
				
				if (!memcmp(&szHexAID[0], "\xA0\x00\x00\x01\x52\x30\x88", 7)) //_EMV_AID_TWIN_HEX_
				{
//					pobTran->uszUCardBit = VS_TRUE;
				}
                        }
                        if(!memcmp(&szHexAID[0], "\xA0\x00\x00\x03\x33", 5)) //_EMV_AID_CUP_HEX_
                        {
                                        /* 確認為銀聯卡 */
                                pobTran->uszUICCBit = VS_TRUE;
                        }
		}		
	}
	
	/* 如果PPSEselect失敗或PPSE解失敗用老方法一個一個AID select */
	if (uszFind4FBit != VS_TRUE)
	{
                for (i = 0;; i++)
                {
                        if (inLoadMVTRec(i) < 0) /* 主機參數檔 */
                                break;

                        memset(szMVTAID, 0x00, sizeof (szMVTAID));
                        memset(szHexAID, 0x00, sizeof (szHexAID));
                        inGetMVTApplicationId(szMVTAID);
                        inAIDLen = strlen(szMVTAID) / 2;
                        inFunc_ASCII_to_BCD((unsigned char*) szHexAID, szMVTAID, inAIDLen);

                        /* APDU Command */
                        memset(&srAPDU_COMMAND, 0x00, sizeof (APDU_COMMAND));

                        srAPDU_COMMAND.uszCommandINSData[0] = _FISC_SELECT_AID_INS_COMMAND_; /* INS */
                        srAPDU_COMMAND.uszCommandP1Data[0] = _FISC_SELECT_AID_P1_COMMAND_; /* P1 */
                        srAPDU_COMMAND.uszCommandP2Data[0] = _FISC_SELECT_AID_P2_COMMAND_; /* P2 */
                        srAPDU_COMMAND.inCommandDataLen = inAIDLen;
                        memcpy(srAPDU_COMMAND.uszCommandData, szHexAID, inAIDLen);

                        if (inAPDU_BuildAPDU(&srAPDU_COMMAND) != VS_SUCCESS)
                        {
                                return (VS_ERROR);
                        }

                        /* 收送Command */
                        if (inContactType == _CONTACT_TYPE_01_CONTACTLESS_)
                        {
                                inRetVal = inAPDU_Send_APDU_CTLS_Process(&srAPDU_COMMAND);
                        } else
                        {
                                inRetVal = inAPDU_Send_APDU_User_Slot_Process(&srAPDU_COMMAND);
                        }

                        if (inRetVal == VS_SUCCESS)
                        {
                                if ((srAPDU_COMMAND.uszRecevData[srAPDU_COMMAND.inRecevLen - 2] == 0x90 && srAPDU_COMMAND.uszRecevData[srAPDU_COMMAND.inRecevLen - 1] == 0x00) ||
                                                (srAPDU_COMMAND.uszRecevData[srAPDU_COMMAND.inRecevLen - 2] == 0x62 && srAPDU_COMMAND.uszRecevData[srAPDU_COMMAND.inRecevLen - 1] == 0x83))
                                {
                                        if (!memcmp(&szHexAID[0], "\xA0\x00\x00\x00\x03", 5) ||
                                                        !memcmp(&szHexAID[0], "\xA0\x00\x00\x00\x04", 5) ||
                                                        !memcmp(&szHexAID[0], "\xA0\x00\x00\x00\x65", 5) ||
                                                        !memcmp(&szHexAID[0], "\xA0\x00\x00\x00\x25", 5) ||
                                                        !memcmp(&szHexAID[0], "\xA0\x00\x00\x01\x52", 5))
                                        {
                                                /* 確認為信用卡 */
                                                pobTran->uszCreditBit = VS_TRUE;
                                        }
                                        if (!memcmp(&szHexAID[0], "\xA0\x00\x00\x03\x33", 5))
                                        {
                                                /* 確認為銀聯卡 */
                                                pobTran->uszUICCBit = VS_TRUE;
                                        }
                                } else
                                {
                                        if (ginDebug == VS_TRUE)
                                                inDISP_LogPrintf("FISC Select AID != 9000");
                                }

                        } else
                        {
                                /* 下Command失敗 */
                                return (inRetVal);
                        }

                }
        }
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inEMV_CreditSelectAID()_END");
	}

	return (VS_SUCCESS);
}

int inEMV_SelectICCAID(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;
	char szCUPFunctionEnable[2 + 1];
	char szFiscFunctionEnable[2 + 1];
	char szMACEnable[2 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "EVM_1530 EmvSelectAid INIT");

	/* 初始化 */
	pobTran->uszCreditBit = VS_FALSE;
	pobTran->uszUICCBit = VS_FALSE;
	pobTran->uszFISCBit = VS_FALSE;

	/* ================================================ */
	/* 先檢查SmartPay功能是否可用 不能就直接跳過跑信用卡EMV */
	/* 因為偵測到有卡和powerOn有時間差(除非事先插入卡片再PowerOn)，所以先停0.4秒 */
	//inDISP_Wait(400);mark by green 191225

#if 0 /* 富邦沒有金融卡，所以先拿掉 20190305 [SAM] */
	inRetVal = inFISC_PowerON(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("inFISC_Func_PowerON_ERR");

		if (inRetVal == VS_EMV_CARD_OUT)
		{
			/* 晶片卡被取出 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;
		} else
		{
			/* 請改刷磁條 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_FALLBACK_;
		}

		return (VS_ERROR);
	}

	inRetVal = inFISC_SelectAID(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("inFISC_Func_SelectAID_ERR");

		if (inRetVal == VS_EMV_CARD_OUT)
		{
			/* 晶片卡被取出 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;
			return (VS_ERROR);
		}
	}

	inFISC_PowerOFF(pobTran);
	/* ================================================ */
#endif

	inRetVal = inEMV_CreditPowerON(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_LogPrintfWithFlag("  inEMV_CreditPowerON_ERR");

		if (inRetVal == VS_EMV_CARD_OUT)
		{
			/* 晶片卡被取出 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;
		} else
		{
			/* 請改刷磁條 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_FALLBACK_;
		}

		return (VS_ERROR);
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_1530 Atf Credit PowOn");

	inRetVal = inEMV_CreditSelectAID(pobTran, _CONTACT_TYPE_00_CONTACT_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_LogPrintfWithFlag("  inEMV_CreditSelectAID_ERR");

		if (inRetVal == VS_EMV_CARD_OUT)
		{
			/* 晶片卡被取出 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;
			return (VS_ERROR);
		}
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_1530 Atf Credit Select Aid");

	inEMV_CreditPowerOFF(pobTran);
	/* ================================================ */
	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_1530 Atf Credit Power Off ");

	/* 擋按銀聯鍵但插非銀聯晶片卡 */
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE && pobTran->uszUICCBit != VS_TRUE)
	{
		/* 請勿按銀聯鍵”(第一行), ”改其他卡別交易”(第二行) */
		inDISP_Msg_BMP(_ERR_NOT_CUP_CHANGE_OTHER_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

		return (VS_ERROR);
	}

	/* 目前不會用到FISC 所以先刪掉 20190328 [SAM] */
#if 0
	/* 其中有Select到AID代表有抓到其中一組 */
	if (pobTran->uszCreditBit == VS_TRUE && pobTran->uszFISCBit == VS_TRUE && pobTran->uszUICCBit == VS_FALSE)
	{
		inRetVal = inFISC_Select_Menu(pobTran);
		if (inRetVal == VS_SUCCESS)
		{
			if (pobTran->uszCreditBit == VS_TRUE)
			{
				if (pobTran->inTransactionCode == _SALE_)
				{
					pobTran->inRunTRTID = _TRT_SALE_ICC_;
				} else if (pobTran->inTransactionCode == _PRE_AUTH_)
				{
					pobTran->inRunTRTID = _TRT_PRE_AUTH_ICC_;
				} else if (pobTran->inTransactionCode == _INST_SALE_)
				{
					pobTran->inRunTRTID = _TRT_INST_SALE_ICC_;
				} else if (pobTran->inTransactionCode == _REDEEM_SALE_)
				{
					pobTran->inRunTRTID = _TRT_REDEEM_SALE_ICC_;
				} else
				{
					/* 請依正確卡別操作 */
					inDISP_Msg_BMP(_ERR_PLEASE_FOLLOW_RIGHT_OPT_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						sprintf(szDebugMsg, "AID_Select But Cat not Select TRT 1, inCode: %d", pobTran->srBRec.inCode);
						inDISP_LogPrintf(szDebugMsg);
					}

					return (VS_ERROR);
				}

				/* NCCC */
				pobTran->srBRec.inHDTIndex = 0;
				inLoadHDTRec(pobTran->srBRec.inHDTIndex);
				inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

				if (ginDebug == VS_TRUE)
					inDISP_LogPrintf("Select = CreditBit");

			} else if (pobTran->uszFISCBit == VS_TRUE)
			{
				/* IDLE進入，一開始預設是_SALE_ */
				if (pobTran->inTransactionCode == _SALE_)
				{
					pobTran->inTransactionCode = _FISC_SALE_;
					pobTran->srBRec.inCode = pobTran->inTransactionCode;
					pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
				}

				if (pobTran->inTransactionCode == _FISC_SALE_)
				{
					pobTran->inRunTRTID = _TRT_FISC_SALE_ICC_;
				} else if (pobTran->inTransactionCode == _FISC_REFUND_)
				{
					pobTran->inRunTRTID = _TRT_FISC_REFUND_ICC_;
				} else
				{
					/* 請依正確卡別操作 */
					inDISP_Msg_BMP(_ERR_PLEASE_FOLLOW_RIGHT_OPT_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						sprintf(szDebugMsg, "AID_Select But Cat not Select TRT 2, inCode: %d", pobTran->inTransactionCode);
						inDISP_LogPrintf(szDebugMsg);
					}

					return (VS_ERROR);
				}

				/* NCCC SMARTPAY同一個HOST */
				pobTran->srBRec.inHDTIndex = 0;
				inLoadHDTRec(pobTran->srBRec.inHDTIndex);
				inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

				pobTran->srBRec.uszFiscTransBit = VS_TRUE;
				/* SmartPay不用簽名 */
				if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
				{
					pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
				}

				/* 因為不需要在這邊換KEY,所以拿掉 20190328 [SAM] */
#if 0
				/* SMARTPAY要GEN MAC來算TCC，一定要安全認證 */
				if (inKMS_CheckKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_MAC_) != VS_SUCCESS)
				{
					if (inNCCC_Func_CUP_PowerOn_LogOn(pobTran) != VS_SUCCESS)
					{
						/* 安全認證失敗 */
						return (VS_ERROR);
					}
				}
#endif

				if (ginDebug == VS_TRUE)
					inDISP_LogPrintf("Select = FISCBit");
			} else
			{
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf("Selec Failed");
				}
				return (VS_ERROR);
			}

			return (VS_SUCCESS);
		} else
		{
			return (inRetVal);
		}


	} else
#endif			
	if (pobTran->uszCreditBit == VS_FALSE && pobTran->uszFISCBit == VS_FALSE && pobTran->uszUICCBit == VS_TRUE)
	{
		inDISP_LogPrintfWithFlag("  Select = UICCBit");

		/* 若銀聯功能沒開，擋掉 */
		/* 若EDC的CUPFunctionEnable 和 MACEnable 未開，顯示此功能以關閉 */
		memset(szCUPFunctionEnable, 0x00, sizeof (szCUPFunctionEnable));
		inGetCUPFuncEnable(szCUPFunctionEnable);
		memset(szMACEnable, 0x00, sizeof (szMACEnable));
		inGetMACEnable(szMACEnable);

		/* 沒開CUP */
		if ((memcmp(&szCUPFunctionEnable[0], "Y", 1) != 0) || memcmp(szMACEnable, "Y", 1) != 0)
		{
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜銀聯一般交易＞ */
			/* 此功能已關閉 */
			inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

			return (VS_ERROR);
		}

		/* IDLE進入，一開始預設是_SALE_ */
		if (pobTran->inTransactionCode == _SALE_)
		{
			pobTran->inTransactionCode = _CUP_SALE_;
			pobTran->srBRec.inCode = pobTran->inTransactionCode;
			pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
		} else if (pobTran->inTransactionCode == _PRE_AUTH_)
		{
			pobTran->inTransactionCode = _CUP_PRE_AUTH_;
			pobTran->srBRec.inCode = pobTran->inTransactionCode;
			pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
		}

		if (pobTran->inTransactionCode == _CUP_SALE_)
		{
			pobTran->inRunTRTID = _TRT_CUP_SALE_ICC_;
		} else if (pobTran->inTransactionCode == _CUP_PRE_AUTH_)
		{
			pobTran->inRunTRTID = _TRT_CUP_PRE_AUTH_ICC_;
		} else
		{
			/* 請依正確卡別操作 */
			inDISP_Msg_BMP(_ERR_PLEASE_FOLLOW_RIGHT_OPT_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

			inDISP_LogPrintfWithFlag("  AID_Select But Cat not Select TRT 3, inCode: %d", pobTran->inTransactionCode);
			return (VS_ERROR);
		}

		/* NCCC CUP同一個HOST */
		pobTran->srBRec.inHDTIndex = 0;
		if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" EMV UICCBit AID Select Load HDT[%d] *Error* Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
		}
		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" EMV UICCBit AID Selec Load HDPT[%d] *Error* Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
		}

		pobTran->srBRec.uszCUPTransBit = VS_TRUE;

		/* 確認是銀聯卡，檢查是否已做安全認證 */
		/* 有開CUP且MACEnable有開但安全認證沒過，不能執行CUP交易 */
		if (inKMS_CheckKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_PIN_ONLINE_) != VS_SUCCESS)
			//if (inKMS_CheckKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_MAC_) != VS_SUCCESS)
		{
			char szHostName[8 + 1];

			inDISP_LogPrintfWithFlag("  inEMV_SelectICCAID 111");

			inGetHostLabel(szHostName);

			/* TODO: 要改一下寫法，看能不能做成例表然後取代就好 */
			inRetVal = inEMV_FubonRunCupLogOn(pobTran, szHostName);

			inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_1530 Atf CupLog ON In Select Icc Aid ");
			if (inRetVal != VS_SUCCESS)
			{
				/* 安全認證失敗 */
				return (VS_ERROR);
			}
		}
		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_1530 End UICCBit Select Icc Aid ");
		return (VS_SUCCESS);
	} else if (pobTran->uszCreditBit == VS_FALSE && pobTran->uszFISCBit == VS_TRUE && pobTran->uszUICCBit == VS_FALSE)
	{
		inDISP_LogPrintfWithFlag("  Select = FISCBit");

		/* 若EDC的FiscFunctionEnable未開，顯示此功能以關閉 */
		memset(szFiscFunctionEnable, 0x00, sizeof (szFiscFunctionEnable));
		inGetFiscFuncEnable(szFiscFunctionEnable);

		/* 沒開Fisc */
		if ((memcmp(szFiscFunctionEnable, "Y", 1) != 0))
		{
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SMARTPAY_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第一層顯示 Smartpay */
			/* 此功能已關閉 */
			inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

			return (VS_ERROR);
		}

		/* IDLE進入，一開始預設是_SALE_ */
		if (pobTran->inTransactionCode == _SALE_)
		{
			pobTran->inTransactionCode = _FISC_SALE_;
			pobTran->srBRec.inCode = pobTran->inTransactionCode;
			pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
		}

		if (pobTran->inTransactionCode == _FISC_SALE_)
		{
			pobTran->inRunTRTID = _TRT_FISC_SALE_ICC_;
		} else if (pobTran->inTransactionCode == _FISC_REFUND_)
		{
			pobTran->inRunTRTID = _TRT_FISC_REFUND_ICC_;
		} else
		{
			/* 請依正確卡別操作 */
			inDISP_Msg_BMP(_ERR_PLEASE_FOLLOW_RIGHT_OPT_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

			inDISP_LogPrintfWithFlag("  AID_Select But Cat not Select TRT 4, inCode: %d", pobTran->inTransactionCode);
			return (VS_ERROR);
		}

		/* NCCC SMARTPAY同一個HOST */
		pobTran->srBRec.inHDTIndex = 0;
		if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" EMV FISCBit AID Select Load HDT[%d] *Error* Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
		}
		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" EMV FISCBit AID Selec Load HDPT[%d] *Error* Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
		}

		pobTran->srBRec.uszFiscTransBit = VS_TRUE;
		/* SmartPay不用簽名 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
		}

		/* 因為不需要在這邊換KEY,所以拿掉 20190328 [SAM] */
#if 0
		/* SMARTPAY要GEN MAC來算TCC，一定要安全認證 */
		if (inKMS_CheckKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_MAC_) != VS_SUCCESS)
		{
			inRetVal = inNCCC_Func_CUP_PowerOn_LogOn(pobTran);

			if (inRetVal != VS_SUCCESS)
			{
				/* 安全認證失敗 */
				return (VS_ERROR);
			}
		}
#endif

		/* 要再重啟一次 */
		//                if (inFISC_PowerON(pobTran) != VS_SUCCESS)
		//                {
		//                        if (ginDebug == VS_TRUE)
		//                                inDISP_LogPrintf("inFISC_Func_PowerON_ERR");
		//
		//                        return (VS_ERROR);
		//                }

		//                if (inFISC_SelectAID(pobTran) != VS_SUCCESS)
		//                {
		//                        if (ginDebug == VS_TRUE)
		//                                inDISP_LogPrintf("inFISC_Func_SelectAID_ERR");
		//
		//                        return (VS_ERROR);
		//                }

		return (VS_SUCCESS);
	} else if (pobTran->uszCreditBit == VS_TRUE && pobTran->uszFISCBit == VS_FALSE && pobTran->uszUICCBit == VS_FALSE)
	{
		inDISP_LogPrintfWithFlag("Select = CreditBit");

		if (pobTran->inTransactionCode == _SALE_)
		{
			pobTran->inRunTRTID = _TRT_SALE_ICC_;
		} else if (pobTran->inTransactionCode == _PRE_AUTH_)
		{
			pobTran->inRunTRTID = _TRT_PRE_AUTH_ICC_;
		} else if (pobTran->inTransactionCode == _INST_SALE_)
		{
			pobTran->inRunTRTID = _TRT_INST_SALE_ICC_;
		} else if (pobTran->inTransactionCode == _REDEEM_SALE_)
		{
			pobTran->inRunTRTID = _TRT_REDEEM_SALE_ICC_;
		}			/* 有設定HG交易別 */
		else if (pobTran->srBRec.lnHGTransactionType != 0)
		{
			/* HG交易的TRT由inHG_Func_SelectPaymentType決定 */
		} else
		{
			/* 請依正確卡別操作 */
			inDISP_Msg_BMP(_ERR_PLEASE_FOLLOW_RIGHT_OPT_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

			inDISP_LogPrintfWithFlag("  AID_Select But Cat not Select TRT 5, inCode: %d", pobTran->inTransactionCode);

			return (VS_ERROR);
		}

		/* NCCC同一個HOST */
		pobTran->srBRec.inHDTIndex = 0;
		if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" EMV CreditBit AID Select Load HDT[%d] *Error* Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
		}
		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" EMV CreditBit AID Selec Load HDPT[%d] *Error* Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
		}

		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_1530 End CreditBit Select Icc Aid ");
		return (VS_SUCCESS);
	} else if (pobTran->uszCreditBit == VS_FALSE && pobTran->uszUICCBit == VS_FALSE && pobTran->uszFISCBit == VS_FALSE)
	{
		/* 乙、插銀聯晶片卡(select不到AID), 顯示”請退出晶片卡” */
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintfWithFlag("  AID_Select_Not_Found");
		}

		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/* EMV CallBack Function */

/* 執行EMV_TxnAppSelect開始時會CALL這支function是否要顯示訊息 */
void OnDisplayShow(IN char *pStrMsg)
{
	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "EVM_2007 DisplayShow INIT");

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	//	inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_); /* 處理中... */

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_2007 DisplayShow END");
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);

}

/* 執行EMV_TxnAppSelect若是有錯誤訊息要顯示最後面會CALL這支function */
void OnErrorMsg(IN char *pStrMsg)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("OnErrorMsg_START!!");
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("OnErrorMsg_END!!");
	}
}

/* 執行EMV_Initialize時會先CALL這支function，選擇要跑的Config */
void OnEMVConfigActive(INOUT BYTE* pActiveIndex)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("OnEMVConfigActive_START!!");
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("OnEMVConfigActive_END!!");
	}
}

/*
Function        :OnTxnDataGet
Date&Time       :2016/9/30 下午 2:26
Describe        :執行EMV_TxnAppSelect時選完後會要求輸入交易資料，如金額，這是必跑的CallBack
 */
USHORT OnTxnDataGet(OUT EMV_TXNDATA *pTxnData)
{
	char szEMVForceOnline[2 + 1];
	unsigned long luTempAmt;

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "EVM_2054 TxnDataGet INIT");

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	luTempAmt = pobEmvTran.srBRec.lnTxnAmount;

	pTxnData->Version = 0x01;
	pTxnData->ulAmount = luTempAmt * 100; /* 沒有小數點 要補2個0 */

	/* Default為0x00 不設好像不會怎麼樣 */
	//	pTxnData->bPOSEntryMode = 0x00;

	/* YYMMDD */
	memcpy(pTxnData->TxnDate, &pobEmvTran.srBRec.szDate[2], 6);
	/* HHMMSS */
	memcpy(pTxnData->TxnTime, &pobEmvTran.srBRec.szTime[0], 6);

	memset(szEMVForceOnline, 0x00, sizeof (szEMVForceOnline));
	inGetEMVForceOnline(szEMVForceOnline);
	if (pobEmvTran.inTransactionCode == _PRE_AUTH_ ||
			pobEmvTran.inTransactionCode == _INST_SALE_ ||
			pobEmvTran.inTransactionCode == _REDEEM_SALE_ ||
			pobEmvTran.inTransactionCode == _CASH_ADVANCE_ ||
			pobEmvTran.srBRec.uszDCCTransBit == VS_TRUE ||
			memcmp(szEMVForceOnline, "Y", strlen("Y")) == 0)
	{
		/* DCC Sale和Pre-Auth因為流程關係，所以在OnEventTxnForcedOnline這個Special Event中
		 * On Force Online  紅利分期、銀聯TRT、FORCEOnlineBit FORCE_ONLINE */
		pTxnData->isForceOnline = VS_TRUE;
	} else
		pTxnData->isForceOnline = VS_FALSE;

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_2054 TxnDataGet END");
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return d_EMVAPLIB_OK;
}

/*
Function        :OnAppList
Date&Time       :2016/12/15 下午 3:52
Describe        :因為要看AID，所以OnAppList廢棄不用，改用OnEventAppListEx
 *		Range of pAppSelectedIndex value is 0 to (AppNum-1)
 *		執行EMV_TxnAppSelect時若是有多個AID會進入此function，要顯示AID列表供選擇
 */
USHORT OnAppList(IN BYTE AppNum, IN char AppLabel[][d_LABEL_STR_SIZE + 1], OUT BYTE *pAppSelectedIndex)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("OnAppList_START!!");
	}

	/* 此Function廢棄不用，改用 OnEventAppListEx，OnEventAppListEx 和 OnAppList若同時註冊，只會跑 OnEventAppListEx */

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("OnAppList_END!!");
	}

	return d_EMVAPLIB_OK;
}

/* Return d_OK to indicate CONFIRMED */

/* 執行EMV_TxnAppSelect時若是只有一個AID會進入此function */
USHORT OnAppSelectedConfirm(IN BOOL IsRequiredbyCard, IN BYTE *pLabel, IN BYTE bLabelLen)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("OnAppSelectedConfirm_START!!");
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("OnAppSelectedConfirm_END!!");
	}

	return d_OK;
}

/* 執行EMV_TxnPerform時若是Kernel在XML找不到需要的TAG值會從這裡要求提供所需的TAG */
BOOL OnTerminalDataGet(IN USHORT usTag, INOUT USHORT *pLen, OUT BYTE *pValue)
{
	char szDebugMsg[100 + 1];
	char szSerialNumber[16 + 1];

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "EVM_2140 TerminalDataGet INIT");

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	/* d_TAG_IFD_SN 0x9F1E */
	if (usTag == d_TAG_IFD_SN)
	{
		memset(szSerialNumber, 0x00, sizeof (szSerialNumber));
		inFunc_GetSeriaNumber(szSerialNumber);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%s", szSerialNumber);
			inDISP_LogPrintf(szDebugMsg);
		}
		/* 取後8碼，但最後一碼為CheckSum，所以取8~15 */
		*pLen = 8;
		memcpy((unsigned char*) pValue, &szSerialNumber[7], *pLen);
		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_2140 TerminalDataGet END");
		inDISP_LogPrintfWithFlag("--FindTag[%s][%s][%d] END --", __FILE__, __FUNCTION__, __LINE__);
		return TRUE;
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_2140 TerminalDataGet END");
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return FALSE;
}

/* 執行EMV_TxnPerform時若是Kernel在XML找不到需要的CAPK KEY值會從這裡要求提供所需的CAPK KEY */
BOOL OnCAPKGet(IN BYTE *pRID, IN BYTE bKeyIndex, OUT BYTE *pModulus, OUT USHORT *pModulusLen, OUT BYTE *pExponent, OUT USHORT *pExponentLen)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("OnCAPKGet_START!!");
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("OnCAPKGet_END!!");
	}

	return FALSE;
}

/* 執行 EMV_TxnPerform 時若是要用Internal PINPAD輸入PIN會CALL這支，輸入完後交由KERNAL處理，若是輸入錯則流程繼續往下跑，除非重新交易才會再進來 */

/*
mail 2013/09/27 RE: V5S開發EMV流程相關問題:
3.我想確認執行Internal EMV Offline PIN是否都是在OnGetPINNotify裡面執行?
Ans: 在OnGetPINNotify Return回Kernel後,由Kernel執行
因為若是PIN輸入錯誤Retry次數會-1，因此在OnPINVerifyResult判斷PIN錯誤後是否會再回到OnGetPINNotify且傳入的bRemainingCounter會-1?
Ans: 當VerifyPIN錯誤後,Kernel並不會再呼叫OnGetPINNotify,仍舊必須繼續其後的流程,直至結束
若User要再輸入第二次,必須重新發起另一個交易
 */
void OnGetPINNotify(IN BYTE bPINType, IN USHORT bRemainingCounter, OUT BOOL* pIsUseDefaultGetPINFunc, OUT DEFAULT_GETPIN_FUNC_PARA *pPara)
{
	char szTimeOut[2 + 1];
	char szAmountMsg[_DISP_MSG_SIZE_ + 1];
	char szPinpadMode[2 + 1];
	char szDebugMsg[100 + 1];
	char szEMVPINBypassEnable[2 + 1];
	unsigned long ulTimeout = 0;

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "EVM_2200 GetPinNotify INIT");

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memset(szPinpadMode, 0x00, sizeof (szPinpadMode));
	inGetPinpadMode(szPinpadMode);
	memset(szEMVPINBypassEnable, 0x00, sizeof (szEMVPINBypassEnable));
	inGetEMVPINBypassEnable(szEMVPINBypassEnable);

	/* 不使用密碼機 */
	/* 銀聯卡不受EMVPinByPass影響 */
	if (memcmp(szPinpadMode, _PINPAD_MODE_0_NO_, strlen(_PINPAD_MODE_0_NO_)) == 0 || (memcmp(szEMVPINBypassEnable, "Y", strlen("Y")) == 0 && pobEmvTran.srBRec.uszCUPTransBit != VS_TRUE))
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "不使用密碼機");
			inDISP_LogPrintf(szDebugMsg);
		}

		if (memcmp(szPinpadMode, _PINPAD_MODE_0_NO_, strlen(_PINPAD_MODE_0_NO_)) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Pinpad Mode = 0");
				inDISP_LogPrintf(szDebugMsg);
			}
		} else if (memcmp(szEMVPINBypassEnable, "Y", strlen("Y")) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "EMVPINBypassEnable :Y");
				inDISP_LogPrintf(szDebugMsg);
			}
		}

		/* 不用密碼機 */
		*pIsUseDefaultGetPINFunc = FALSE;

	}		/* 內建密碼機 */
	else if (memcmp(szPinpadMode, _PINPAD_MODE_1_INTERNAL_, strlen(_PINPAD_MODE_1_INTERNAL_)) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "內建密碼機");
			inDISP_LogPrintf(szDebugMsg);
		}

		/* 用內建密碼機 */
		*pIsUseDefaultGetPINFunc = TRUE;

		/* 如果程式內沒設定TimeOut，就用MVT.dat內的TimeOut */
		if (ulTimeout <= 0)
		{
			memset(szTimeOut, 0x00, sizeof (szTimeOut));
			memcpy(szTimeOut, "20", 2);
			//inGetEMVPINEntryTimeout(szTimeOut);
			ulTimeout = atoi(szTimeOut);
		}

		memset(pPara, 0x00, sizeof (DEFAULT_GETPIN_FUNC_PARA));

		if (bPINType == d_NOTIFY_OFFLINE_PIN)
		{
			pPara->Version = 0x01;
			pPara->usLineLeft_X = 5;
			pPara->usLineRight_X = 20;
			/* 因UPT底層會自己建出密碼介面，所以需要重新設定 * 號密碼的位置  20190523  [SAM] */
#if _MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_
			pPara->usLinePosition_Y = 2;
#else							
			pPara->usLinePosition_Y = 7;
#endif
			pPara->bPINDigitMaxLength = 8;
			pPara->bPINDigitMinLength = 4;
			pPara->ulTimeout = ulTimeout;

			/* 從右至左 */
			//			pPara->IsRightToLeft = TRUE;
			/* 反白 */
			//			pPara->IsReverseLine = TRUE;

			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("請輸入Offline PIN", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
		} else
		{
			pPara->Version = 0x01;
			pPara->usLineLeft_X = 1;
			pPara->usLineRight_X = 20;
			/* 因UPT底層會自己建出密碼介面，所以需要重新設定 * 號密碼的位置  20190523  [SAM] */
#if _MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_
			pPara->usLinePosition_Y = 2;
#else				
			pPara->usLinePosition_Y = 8;
#endif
			pPara->bPINDigitMaxLength = 8;
			pPara->bPINDigitMinLength = 4;
			pPara->ulTimeout = ulTimeout;

			pPara->ONLINEPIN_PARA.CipherKeySet = _TWK_KEYSET_NCCC_;
			pPara->ONLINEPIN_PARA.CipherKeyIndex = _TWK_KEYINDEX_NCCC_PIN_ONLINE_;
			pPara->ONLINEPIN_PARA.bPANLen = strlen(pobEmvTran.srBRec.szPAN);
			memcpy(pPara->ONLINEPIN_PARA.baPAN, pobEmvTran.srBRec.szPAN, strlen(pobEmvTran.srBRec.szPAN));
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintfArea(TRUE, "szPAN:", 6, (unsigned char*) pobEmvTran.srBRec.szPAN, strlen(pobEmvTran.srBRec.szPAN));

			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_CUP_GET_PASSWORD_IN_, 0, _COORDINATE_Y_LINE_8_4_);

			/* 顯示金額 */
			memset(szAmountMsg, 0x00, sizeof (szAmountMsg));
			if ((pobEmvTran.srBRec.uszVOIDBit == VS_TRUE &&
					(pobEmvTran.srBRec.inOrgCode != _REFUND_ && pobEmvTran.srBRec.inOrgCode != _INST_REFUND_ && pobEmvTran.srBRec.inOrgCode != _REDEEM_REFUND_ && pobEmvTran.srBRec.inOrgCode != _CUP_REFUND_ && pobEmvTran.srBRec.inOrgCode != _CUP_MAIL_ORDER_REFUND_)) ||
					pobEmvTran.srBRec.inCode == _REFUND_ ||
					pobEmvTran.srBRec.inCode == _INST_REFUND_ ||
					pobEmvTran.srBRec.inCode == _REDEEM_REFUND_ ||
					pobEmvTran.srBRec.inCode == _CUP_REFUND_ ||
					pobEmvTran.srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szAmountMsg, "%ld", 0 - pobEmvTran.srBRec.lnTotalTxnAmount);
			} else
			{
				sprintf(szAmountMsg, "%ld", pobEmvTran.srBRec.lnTotalTxnAmount);
			}
			inFunc_Amount_Comma(szAmountMsg, "NT$", ' ', _SIGNED_NONE_, 15, _PAD_LEFT_FILL_RIGHT_);
			inDISP_EnglishFont_Point_Color(szAmountMsg, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_RED_, _COLOR_WHITE_, 7);

			/* 嗶三聲 */
			inDISP_BEEP(3, 500);
			/* 因UPT底層會自己建出密碼介面，所以需要重新清畫面  20190523  [SAM] */
#if _MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_
			inDISP_Clear_Line(_LINE_8_1_, _LINE_8_8_);
#endif

		}

	}		/* 外接密碼機 */
	else if (memcmp(szPinpadMode, _PINPAD_MODE_2_EXTERNAL_, strlen(_PINPAD_MODE_2_EXTERNAL_)) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "外接密碼機");
			inDISP_LogPrintf(szDebugMsg);
		}

		/* 用外接密碼機 */
		*pIsUseDefaultGetPINFunc = FALSE;

		/* 繼續到OnOnlinePINBlockGet 或 OnOfflinePINBlockGet 才輸入 */
		inDISP_PutGraphic(_CUP_GET_PASSWORD_OUT_, 0, _COORDINATE_Y_LINE_8_4_);
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_2200 GetPinNotify END");
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
}

/* Return d_OK to indicate Online PIN block is ready for application */

/* 執行EMV_TxnPerform時若是需要用外接PINPAD輸入Online PIN BLOCK會CALL這支function，目前都會用內建，所以應該不會用這支 */
USHORT OnOnlinePINBlockGet(OUT ONLINE_PIN_DATA *pOnlinePINData)
{
	char szPinpadMode[2 + 1];
	char szEMVPINBypassEnable[2 + 1];

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "EVM_2379 OnlinePinBlockGet INIT");

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memset(szPinpadMode, 0x00, sizeof (szPinpadMode));
	inGetPinpadMode(szPinpadMode);
	memset(szEMVPINBypassEnable, 0x00, sizeof (szEMVPINBypassEnable));
	inGetEMVPINBypassEnable(szEMVPINBypassEnable);

	/* 不使用密碼機 */
	/* 銀聯卡不受EMVPinByPass影響 */
	if (memcmp(szPinpadMode, _PINPAD_MODE_0_NO_, strlen(_PINPAD_MODE_0_NO_)) == 0 || (memcmp(szEMVPINBypassEnable, "Y", strlen("Y")) == 0 && pobEmvTran.srBRec.uszCUPTransBit != VS_TRUE))
	{
		pOnlinePINData->isOnlinePINRequired = VS_FALSE;

		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("OnOnlinePINBlockGet_END!!");
		}
		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_2379 MD0 GetPinNotify END");
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] MD 0 END -----", __FILE__, __FUNCTION__, __LINE__);
		return d_EMVAPLIB_OK;
	}		/* 這裡放外接密碼機回傳的判斷或使用內建KMS加密後的判斷 */
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("OnOnlinePINBlockGet_END!!");
		}
		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_2379 GetPinNotify END");
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
		return d_EMVAPLIB_OK;
	}

	//	pOnlinePINData->bPINLen = 8;
	//	memcpy(pin_block, "\xB5\x1E\xC3\xBE\xB7\x24\x37\x13", 8);
	//	pOnlinePINData->pPIN = pin_block;
}

/* Return d_OK to indicate Offline PIN block is ready for Kernel */
/* If this function uses KMS_GetEncOfflinePIN function to get offline pin, return d_EMV_ENTER_KMS_OFFLINEPIN to indicate enciphed offline PIN is ready for Kernel */

/* 執行EMV_TxnPerform時若是需要用外接PINPAD輸入Offline PIN BLOCK會CALL這支function，目前都會用內建，所以應該不會用這支 */
USHORT OnOfflinePINBlockGet(void)
{
	char szPinpadMode[2 + 1];
	char szEMVPINBypassEnable[2 + 1];

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "EVM_2429 OfflinePinBlockGet INIT");

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memset(szPinpadMode, 0x00, sizeof (szPinpadMode));
	inGetPinpadMode(szPinpadMode);
	memset(szEMVPINBypassEnable, 0x00, sizeof (szEMVPINBypassEnable));
	inGetEMVPINBypassEnable(szEMVPINBypassEnable);

	/* 不使用密碼機 */
	/* 銀聯卡不受EMVPinByPass影響 */
	if (memcmp(szPinpadMode, _PINPAD_MODE_0_NO_, strlen(_PINPAD_MODE_0_NO_)) == 0 || (memcmp(szEMVPINBypassEnable, "Y", strlen("Y")) == 0 && pobEmvTran.srBRec.uszCUPTransBit != VS_TRUE))
	{
		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_2379 MD0 OfflinePinBlockGet END");
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] MD0 END -----", __FILE__, __FUNCTION__, __LINE__);

		return d_EMVAPLIB_OK;
	}		/* 這裡放外接密碼機回傳的判斷或使用內建KMS加密後的判斷 */
	else
	{

		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_2379 OfflinePinBlockGet END");
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
		return d_EMVAPLIB_OK;
	}


	//	USHORT ret;
	//	BYTE key;
	//	CTOS_stDefEncOffPINStruc para;
	//	CTOS_KMS2DATAENCRYPT_PARA stDataEncPara;
	//	BYTE PINBlock[32], PINBlockLen;
	//	BYTE msg1[20], msg2[20], procmsg[20];
	//
	//	stDataEncPara.Version = 0x00;
	//
	//	stDataEncPara.Protection.CipherKeyIndex = d_OFFLINE_KEYINDEX;
	//	stDataEncPara.Protection.CipherKeySet = d_ONLINE_KEYSET;
	//	stDataEncPara.Protection.CipherMethod = KMS2_DATAENCRYPTCIPHERMETHOD_EXTPIN_ECB;
	//	memcpy(PINBlock, "\xB5\x1E\xC3\xBE\xB7\x24\x37\x13", 8);
	//	PINBlockLen = 8;
	//	stDataEncPara.Input.pData = PINBlock;
	//	stDataEncPara.Input.Length = PINBlockLen;
	//
	//	ret = CTOS_KMS2DataEncrypt(&stDataEncPara);
	//
	//	if (ret != d_OK)
	//	{
	//		printf("OnOfflinePINBlockGet_ERROR!! ");
	//		return d_EMVAPLIB_ERR_CRITICAL_ERROR;
	//	}
}

/* 執行EMV_TxnPerform時若是輸入Offline PIN後會CALL這支function檢查PIN是否合法 */
void OnOfflinePINVerifyResult(IN USHORT usResult)
{
	//	BYTE data[32];

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "EVM_2492 OfflinePinVerify INIT");

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);


	/* Sample */
	//	memset(data, 0, sizeof(data));
	//
	//    	if (usResult == d_PIN_RESULT_OK)
	//    	{
	//		//"PIN Verify OK"
	//		printf("PIN Verify OK ");
	//        	sprintf((char *)data, "PIN Verify OK");
	//
	//	}
	//	else if (usResult == d_PIN_RESULT_FAIL)
	//	{
	//		//"!PIN Wrong!"
	//		printf("PIN Wrong ");
	//        	sprintf((char *)data, "PIN Wrong");
	//	}
	//	else if (usResult == d_PIN_RESULT_BLOCKED || usResult == d_PIN_RESULT_FAILBLOCKED)
	//	{
	//		//"!PIN Blocked!"
	//		printf("PIN Blocked ");
	//        	sprintf((char *)data, "PIN Blocked");
	//	}
	//
	//	CTOS_LCDTPrintXY(1, 6, data);

	if (usResult == d_PIN_RESULT_OK)
	{
		//"PIN Verify OK"
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("PIN Verify OK", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
	} else if (usResult == d_PIN_RESULT_FAIL)
	{
		//"!PIN Wrong!"
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("PIN Wrong", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
	} else if (usResult == d_PIN_RESULT_BLOCKED || usResult == d_PIN_RESULT_FAILBLOCKED)
	{
		//"!PIN Blocked!"
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("PIN Blocked", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_2492 OfflinePinVerify END");
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
}

/* 執行 EMV_TxnPerform 時若是First GenAC要求Online則進入這支function做電文送收，這支function一定要跑 */
void OnTxnOnline(IN ONLINE_PIN_DATA *pOnlinePINData, OUT EMV_ONLINE_RESPONSE_DATA* pOnlineResponseData)
{
	int inRetVal = 0;
	char szDemoMode[2 + 1] = {0};
	char szHostName[8 + 1];

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "EVM_2552 OnTxnOnline INIT");

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	inDISP_LogPrintfWithFlag(" bPINLen[%d]", pOnlinePINData->bPINLen);


	memset(szHostName, 0x00, sizeof (szHostName));
	inGetHostLabel(szHostName);

	pobEmvTran.inEMVDecision = _EMV_DECESION_ONLINE;

	/* 從API傳入OnlinePin資料 */
	if (pOnlinePINData->bPINLen > 0)
	{
		memset(pobEmvTran.szPIN, 0x00, 8 + 1);
		//memcpy(pobEmvTran.szPIN, pOnlinePINData->pPIN, pOnlinePINData->bPINLen);
		inFunc_BCD_to_ASCII(pobEmvTran.szPIN, pOnlinePINData->pPIN, pOnlinePINData->bPINLen);
		pobEmvTran.srBRec.uszPinEnterBit = VS_TRUE;
	} else
	{
		pobEmvTran.srBRec.uszPinEnterBit = VS_FALSE;
	}

	/* 初始化 EMV Config */
	memset(&EMVGlobConfig, 0x00, sizeof (EMV_CONFIG));

	ginGlobalRetVal = VS_SUCCESS;

	if (ginGlobalRetVal == VS_SUCCESS)
	{
		if (inEMV_GetEMVTag(&pobEmvTran) != VS_SUCCESS)
		{
			ginGlobalRetVal = VS_ERROR;
			inDISP_DispLogAndWriteFlie(" EMV TxOnline Get EmvTag *Error* Line[%d] ", __LINE__);
		}
		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_2552 OnTxnOnline Aft GetEmvTag");
	}


	if (ginGlobalRetVal == VS_SUCCESS)
	{
		/* TODO: 要改一下寫法，看能不能做成例表然後取代就好 */
		if (memcmp(szHostName, _HOST_NAME_CREDIT_FUBON_, strlen(_HOST_NAME_CREDIT_FUBON_)) == 0)
		{
			inRetVal = inFLOW_RunFunction(&pobEmvTran, _FUBON_SET_ONLINE_OFFLINE_);
		} else
		{
			inDISP_DispLogAndWriteFlie(" EMV OnTxnOnline Run Set Online Host *Error* szHostName[%s] Line[%d]", szHostName, __LINE__);
			inRetVal = VS_ERROR;
		}

		if (inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" EMV OnTxnOnline Run Set Online Offline *Error* Line[%d]", __LINE__);
			ginGlobalRetVal = VS_ERROR;
		}

		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_2552 OnTxnOnline Aft Set Onlie Ofline");
	}

	if (ginGlobalRetVal == VS_SUCCESS)
	{
		/* TODO: 要改一下寫法，看能不能做成例表然後取代就好 */
		if (memcmp(szHostName, _HOST_NAME_CREDIT_FUBON_, strlen(_HOST_NAME_CREDIT_FUBON_)) == 0)
		{
			inRetVal = inFLOW_RunFunction(&pobEmvTran, _FUBON_ISO_FUNCTION_BUILD_AND_SEND_PACKET_);
			if (inRetVal != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie(" EMV OnTxnOnline Run Send Host *Error* inRetVal[%d] Line[%d]", inRetVal, __LINE__);
			}
		} else
		{
			inDISP_DispLogAndWriteFlie(" EMV OnTxnOnline Run Send Host *Error* szHostName[%s] Line[%d]", szHostName, __LINE__);
			inRetVal = VS_ERROR;
		}

		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_2552 OnTxnOnline Aft Build And Send Pack");
		inTIME_UNIT_GetGlobalTimeToCompare("EVM_2552 OnTxnOnline Aft Build And Send Pack");

		if (inRetVal == VS_SUCCESS)
		{

		} else if (inRetVal == VS_COMM_ERROR)
		{
			EMVGlobConfig.uszAction = d_ONLINE_ACTION_UNABLE;
			ginGlobalRetVal = VS_ERROR;
		} else if (inRetVal != VS_SUCCESS)
		{
			ginGlobalRetVal = VS_ERROR;
		}
	}

	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof (szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		pOnlineResponseData->bAction = d_ONLINE_ACTION_APPROVAL;
		memcpy(EMVGlobConfig.uszAuthorizationCode, "00", 2);
		pOnlineResponseData->pAuthorizationCode = EMVGlobConfig.uszAuthorizationCode;
		pOnlineResponseData->pIssuerAuthenticationData = EMVGlobConfig.uszIssuerAuthenticationData;
		pOnlineResponseData->pIssuerScript = EMVGlobConfig.uszIssuerScript;
	} else
	{
		/* Second Gen AC會在Online後執行，執行完Second Gen AC的流程在OnTxnResult中執行 */
		/* Host Action 用global 變數 guszAction*/

		/* 主機Auth Code */
		memset(EMVGlobConfig.uszAuthorizationCode, 0x00, sizeof (EMVGlobConfig.uszAuthorizationCode));
		memcpy(EMVGlobConfig.uszAuthorizationCode, &pobEmvTran.srBRec.szRespCode, 2);

		inDISP_LogPrintfWithFlag("  EMVGlobConfig.uszAuthorizationCode:%s", EMVGlobConfig.uszAuthorizationCode);

		/* 填入IssuerAuthData */
		if (pobEmvTran.srEMVRec.in91_IssuerAuthDataLen > 0)
		{
			memset(EMVGlobConfig.uszIssuerAuthenticationData, 0x00, sizeof (EMVGlobConfig.uszIssuerAuthenticationData));
			memcpy(EMVGlobConfig.uszIssuerAuthenticationData, &pobEmvTran.srEMVRec.usz91_IssuerAuthData, pobEmvTran.srEMVRec.in91_IssuerAuthDataLen);
			EMVGlobConfig.usIssuerAuthenticationDataLen = pobEmvTran.srEMVRec.in91_IssuerAuthDataLen;

			inDISP_LogPrintfWithFlag("  IssuerAuthenticationDataLen : %d", EMVGlobConfig.usIssuerAuthenticationDataLen);
			inDISP_LogPrintfArea(TRUE, "", 0, EMVGlobConfig.uszIssuerAuthenticationData, EMVGlobConfig.usIssuerAuthenticationDataLen);

		}

		/*{
			pobEmvTran.srEMVRec.in71_IssuerScript1Len = 24;
			memcpy(pobEmvTran.srEMVRec.usz71_IssuerScript1,"\x71\x16\x9F\x18\x04\x01\x02\x03\x04\x86\x0D\x84\x1E\x00\x00\x08\x50\x48\x30\xC4\x24\x9B\x67\xCB",pobEmvTran.srEMVRec.in71_IssuerScript1Len);
		}*/

		/* 填入IssuerScript1 */
		if (pobEmvTran.srEMVRec.in71_IssuerScript1Len > 0)
		{
			memset(EMVGlobConfig.uszIssuerScript, 0x00, sizeof (EMVGlobConfig.uszIssuerScript));
			memcpy(EMVGlobConfig.uszIssuerScript, &pobEmvTran.srEMVRec.usz71_IssuerScript1, pobEmvTran.srEMVRec.in71_IssuerScript1Len);
			EMVGlobConfig.usIssuerScriptLen = pobEmvTran.srEMVRec.in71_IssuerScript1Len;

			inDISP_LogPrintfWithFlag("  IssuerScriptLen1 : %d,%d", EMVGlobConfig.usIssuerScriptLen, pobEmvTran.srEMVRec.in71_IssuerScript1Len);
			inDISP_LogPrintfArea(TRUE, "", 0, EMVGlobConfig.uszIssuerScript, EMVGlobConfig.usIssuerScriptLen);
		}

		/* 填入IssuerScript2 */
		if (pobEmvTran.srEMVRec.in72_IssuerScript2Len > 0)
		{
			memcpy(&EMVGlobConfig.uszIssuerScript[EMVGlobConfig.usIssuerScriptLen], &pobEmvTran.srEMVRec.usz72_IssuerScript2, pobEmvTran.srEMVRec.in72_IssuerScript2Len);
			EMVGlobConfig.usIssuerScriptLen += pobEmvTran.srEMVRec.in72_IssuerScript2Len;

			inDISP_LogPrintfWithFlag("  IssuerScriptLen2 : %d,%d", EMVGlobConfig.usIssuerScriptLen, pobEmvTran.srEMVRec.in72_IssuerScript2Len);
			inDISP_LogPrintfArea(TRUE, "", 0, &EMVGlobConfig.uszIssuerScript[EMVGlobConfig.usIssuerScriptLen], EMVGlobConfig.usIssuerScriptLen);
		}

		inDISP_LogPrintfWithFlag("  OnTxnOnline Emv Data  Line[%d]", __LINE__);
		inDISP_LogPrintfWithFlag("  uszAction[%x] uszAuthorizationCode[%s] usIssuerAuthenticationDataLen[%ld]", EMVGlobConfig.uszAction, EMVGlobConfig.uszAuthorizationCode, EMVGlobConfig.usIssuerAuthenticationDataLen);
		inDISP_LogPrintfWithFlag("  72_Len[%d] 71_Len[%d]  91_Len[%d] szRespCode[%s]", pobEmvTran.srEMVRec.in72_IssuerScript2Len, pobEmvTran.srEMVRec.in71_IssuerScript1Len, pobEmvTran.srEMVRec.in91_IssuerAuthDataLen, pobEmvTran.srBRec.szRespCode);

		if (ginDebug == VS_TRUE)
		{
			if (EMVGlobConfig.uszAction == d_ONLINE_ACTION_APPROVAL)
			{
				inDISP_LogPrintfWithFlag("  EMVAction: APPROVAL");
			} else if (EMVGlobConfig.uszAction == d_ONLINE_ACTION_DECLINE)
			{
				inDISP_LogPrintfWithFlag("  EMVAction: DECLINE");
			} else if (EMVGlobConfig.uszAction == d_ONLINE_ACTION_UNABLE)
			{
				inDISP_LogPrintfWithFlag("  EMVAction: UNABLE");
			} else
			{
				inDISP_LogPrintfWithFlag("  EMVAction: %d", EMVGlobConfig.uszAction);
			}
		}

		/* 注意，傳入的pointer一定要是global的，否則會吃不到資料 */
		pOnlineResponseData->bAction = EMVGlobConfig.uszAction;
		pOnlineResponseData->pAuthorizationCode = EMVGlobConfig.uszAuthorizationCode;
		pOnlineResponseData->pIssuerAuthenticationData = EMVGlobConfig.uszIssuerAuthenticationData;
		pOnlineResponseData->IssuerAuthenticationDataLen = EMVGlobConfig.usIssuerAuthenticationDataLen;
		pOnlineResponseData->pIssuerScript = EMVGlobConfig.uszIssuerScript;
		pOnlineResponseData->IssuerScriptLen = EMVGlobConfig.usIssuerScriptLen;
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_2552 OnTxnOnline END");
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
}

/* 執行EMV_TxnPerform時若是要處理Issuer Script則要跑這支function */
void OnTxnIssuerScriptResult(IN BYTE* pScriptResult, IN USHORT pScriptResultLen)
{
	char szAscii[100 + 1];
	char szDebugMsg[100 + 1];

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "EVM_2771 OnTxnIssuerScriptResult INIT");

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	pobEmvTran.srEMVRec.in9F5B_ISRLen = pScriptResultLen;
	memcpy(pobEmvTran.srEMVRec.usz9F5B_ISR, pScriptResult, pobEmvTran.srEMVRec.in9F5B_ISRLen);

	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof (szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*) pScriptResult, pScriptResultLen);
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "%s", szAscii);
		inDISP_LogPrintf(szDebugMsg);
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_2771 OnTxnIssuerScriptResult END");
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
}

/* 執行EMV_TxnPerform的最後要判斷晶片處理結果且顯示通知使用者要在這處理 */
void OnTxnResult(IN BYTE bResult, IN BOOL IsSignatureRequired)
{
	int inRetVal = VS_ERROR;
	char szDemoMode[2 + 1] = {0};
	unsigned char uszValue[128 + 1] = {0};
	unsigned short usTagLen = 0;

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "EVM_2799 OnTxnResult INIT");

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	inDISP_DispLogAndWriteFlie(" EMV bResult[%d] IsSignatureRequired[%d]", bResult, IsSignatureRequired);


	/* bResult 為交易結果，IsSignatureRequired是CVM結果，目前只存，但我們自己抓TAG判斷 */
	if (bResult == d_TXN_RESULT_APPROVAL)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("Txn Approval");
		}

		pobEmvTran.inEMVResult = _EMV_RESULT_APPROVE_;

		if (pobEmvTran.inEMVDecision == _EMV_DECESION_ONLINE)
		{

		} else
		{
			pobEmvTran.inEMVDecision = _EMV_DECESION_OFFLINE;
		}
	} else if (bResult == d_TXN_RESULT_DECLINE)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("Txn Decline");
		}

		pobEmvTran.inEMVResult = _EMV_RESULT_DECLINE_;
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("Txn Not Recognize");
		}

		pobEmvTran.inEMVResult = _EMV_RESULT_UNKOWN_;
	}

	/* 判斷是否要簽名 */
	/* NCCC不判斷晶片卡結果(有事Vx520負責) 2018/6/22 下午 6:11 */
	if (ginAPVersionType == _APVERSION_TYPE_NCCC_)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("NCCC Don't Check CVM Result");
		}

		if (IsSignatureRequired == 1)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EMV Kernel Decide Need Signature");
			}

			pobEmvTran.inCVMResult = _CVM_RESULT_NEED_SIGNATURE_;
		} else
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EMV Kernel Decide Not Need Signature");
			}

			pobEmvTran.inCVMResult = _CVM_RESULT_NO_NEED_SIGNATURE_;
		}


	} else
	{
		if (IsSignatureRequired == 1)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EMV Kernel Decide Need Signature");
			}

			pobEmvTran.inCVMResult = _CVM_RESULT_NEED_SIGNATURE_;
		} else
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EMV Kernel Decide Not Need Signature");
			}
			pobEmvTran.inCVMResult = _CVM_RESULT_NO_NEED_SIGNATURE_;

			/* 分期付款交易，使用感應卡進行交易，皆要簽名，不允許分期付款交易"免簽名"。 */
			if (pobEmvTran.srBRec.uszInstallmentBit == VS_TRUE)
			{
				pobEmvTran.srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
			} else
			{
				pobEmvTran.srBRec.uszNoSignatureBit = VS_TRUE;
			}
		}
	}

	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof (szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		/* 不用送TC，但若失敗仍要回傳錯誤(For demo，正式版會在inXXX_OnlineEMV_Complete中檢核) */
		/* 8A */
		usTagLen = sizeof (uszValue);
		memset(uszValue, 0x00, sizeof (uszValue));
		inEMV_Get_Tag_Value(0x8A, &usTagLen, uszValue);

		/*  先檢核first gen AC的結果 若是Y1 或 Z1就不用在做Second Gen AC */
		if (!memcmp(uszValue, "Z1", 2))
		{
			/* Z1= declined offline (first Gen AC , CID=AAC) */
			/* 拒絕交易 */

			memset(pobEmvTran.srBRec.szRespCode, 0x00, sizeof (pobEmvTran.srBRec.szRespCode));
			strcpy(pobEmvTran.srBRec.szRespCode, "Z1");

			inEMV_Decide_DispHostRespCodeMsg();
			ginGlobalRetVal = VS_ERROR;
		} else if (!memcmp(uszValue, "Y1", 2))
		{
			/* EMV First Gen AC產生Y1 */
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("EMV First Gen AC產生Y1");

			memset(pobEmvTran.srBRec.szRespCode, 0x00, sizeof (pobEmvTran.srBRec.szRespCode));
			memset(pobEmvTran.srBRec.szAuthCode, 0x00, sizeof (pobEmvTran.srBRec.szAuthCode));
			strcpy(pobEmvTran.srBRec.szRespCode, "Y1");
			strcpy(pobEmvTran.srBRec.szAuthCode, "Y1");
		} else
		{
			/* 開始執行 Second Generate AC */
			inRetVal = inEMV_SecondGenerateAC(&pobEmvTran);

			inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_2797 OnTxnResult Aft Second Generate Ac");
			if (inRetVal == VS_SUCCESS)
			{

			} else
			{
				/* 拒絕交易Z3 */
				inEMV_Decide_DispHostRespCodeMsg();
				ginGlobalRetVal = VS_ERROR;
			}

		}
	} else
	{
		if (inEMV_Decide_OnlineEMV_Complete() != VS_SUCCESS)
		{
			ginGlobalRetVal = VS_ERROR;
		}

		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_2799 OnTxnResult Aft Decide Online EMV");

	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_2799 OnTxnResult END");
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
}

/* 執行EMV_TxnPerform時要在這輸入金額將金額回傳給Kernel */
void OnTotalAmountGet(IN BYTE *pPAN, IN BYTE bPANLen, OUT ULONG *pAmount)
{
	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "EVM_2985 OnTotalAmountGet INIT");

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	bPANLen = strlen(pobEmvTran.srBRec.szPAN);
	pPAN = (unsigned char*) pobEmvTran.srBRec.szPAN;

	if (ginDebug == VS_TRUE)
	{
		char szDebugMsg[100 + 1];

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Total Amount : %ld", *pAmount);
		inDISP_LogPrintf(szDebugMsg);
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_2985 OnTotalAmountGet END");
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
}

/* 執行EMV_TxnPerform時在進行FirstGenAC前會進入這檢查檔案 */
void OnExceptionFileCheck(IN BYTE *pPAN, IN BYTE bPANLen, OUT BOOL *isException)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("OnExceptionFileCheck_START!!");
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf((char *) isException);
	}


	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("OnExceptionFileCheck_END!!");
	}
}

/* 執行EMV_TxnPerform時取得CAPK後會檢查是否CAPK合法 */
BOOL OnCAPKRevocationCheck(IN BYTE *pbRID, IN BYTE bCAPKIdx, BYTE *pbSerialNumuber)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("OnCAPKRevocationCheck_START!!");
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("OnCAPKRevocationCheck_END!!");
	}

	return FALSE;
}

/*
Function        :OnEventTxnForcedOnline
Date&Time       :2016/12/13 下午 5:14
Describe        :For DCC不能在Txn Data Get中設定Force Online用，在OnTxnOnline前，OnExceptionFileCheck後執行
 */
void OnEventTxnForcedOnline(BYTE *pbForcedONL)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("ON_EVENT_TxnForcedOnline() START !");
	}

	//Inform kernel forced online or not
	/* forced online */
	if (pobEmvTran.srBRec.uszDCCTransBit == VS_TRUE)
	{
		*pbForcedONL = 0x01;
	}

	/* NO forced online */
	//*pbForcedONL = 0x00;
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("ON_EVENT_TxnForcedOnline() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

}

/*
Function        :OnEventAppListEx
Date&Time       :2016/12/15 下午 3:57
Describe        :
 * 端末機Select到多個UICC的AID時，端末機自動Select AID而不需要出現Select AP的List讓收銀員選擇。(本題僅針對UICC，V/M/J維持原EMV 流程)
 * 端末機自動Select AID的條件如下：
 * (1)	以優先權較高的AID，優先執行。
 * (2)	若所有AID都沒有標示優先權，則直接以TMS下載之AID順序進行Select。端末機自動以Select出來的第一個AID進行交易。

 */
USHORT OnEventAppListEx(BYTE bAppNum, EMV_APP_LIST_EX_DATA *pstAppListExData, BYTE *pbAppSelectedIndex)
{
	//	int		i, j;
	int i;
	int inAppIndex = -1;
	char szDebugMsg[100 + 1];
	//	char		szMVTAID[20 + 1];
	//	char		szAIDOption[20 + 1];
	char szTemplate[50 + 1];
	unsigned char uszKey;

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "EVM_3085 OnEventAppListEx INIT");

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	if (ginEMVAppSelection != -1)
	{
		*pbAppSelectedIndex = ginEMVAppSelection;

		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_3085 Seletion -1 OnEventAppListEx END");
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
		return d_EMVAPLIB_OK;
	}

#if 0//By Ray For FUBON
	/* 只有CUP要幫忙選 */
	if (pobEmvTran.srBRec.uszCUPTransBit == VS_TRUE)
	{
		/* (1)	以優先權較高的AID，優先執行。 虹堡Kernel已將優先權較高排序，所以列表第一項即為優先權最高 */
		if (bAppNum > 0)
		{
			/* 直接選第一項*/
			inAppIndex = 0;
		}			/* (2)	若所有AID都沒有標示優先權，則直接以TMS下載之AID順序進行Select。端末機自動以Select出來的第一個AID進行交易。 */
		else
		{
			for (i = 0;; i++)
			{
				/* load 參數檔第一個AID */
				if (inLoadMVTRec(i) < 0) /* 主機參數檔 */
					break;

				memset(szMVTAID, 0x00, sizeof (szMVTAID));
				inGetMVTApplicationId(szMVTAID);

				/* 搜尋APPList中是否有那一個選項 */
				for (j = 0; j < bAppNum; j++)
				{
					memset(szAIDOption, 0x00, sizeof (szAIDOption));
					inFunc_BCD_to_ASCII(szAIDOption, pstAppListExData[j].baAID, pstAppListExData[j].baAIDLen);

					if (memcmp(szMVTAID, szAIDOption, strlen(szAIDOption)) == 0)
					{
						inAppIndex = j;
						break;
					}

				}/* 搜尋TMS MVT */

				/* APPList中比對到TMS參數檔中的AID */
				if (inAppIndex != -1)
				{
					break;
				}

			}/* 搜尋AID 選項*/

		}

	}		/* VMJ手動選 */
	else
#endif
	{


		/* 抓取 KioskFlag的值，如為 TRUE 就不用確認畫面 2020/3/6 下午 4:28 [SAM] */
		if (inFunc_GetKisokFlag() == VS_TRUE)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inAppIndex = 0;
			inDISP_DispLogAndWriteFlie(" OnEventApp V3C Kiosk Select Default Apid [0]");
		} else
		{

			while (1)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

				for (i = 0; i < bAppNum; i++)
				{
					memset(szTemplate, 0x00, sizeof (szTemplate));
					sprintf(szTemplate, "%d.%s", i + 1, pstAppListExData[i].cAppLabel);
					inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_4_ + i, _DISP_LEFT_);
					//inDISP_ChineseFont(szTemplate, _FONTSIZE_8X16_, _LINE_8_4_ + i, _DISP_LEFT_);
				}

				uszKey = uszKBD_GetKey(30);

				if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
				{
					return d_EMVAPLIB_ERR_SELECTION_FAIL;
				} else if (uszKey == _KEY_1_ && bAppNum >= 1)
				{
					inAppIndex = 0;
					break;
				} else if (uszKey == _KEY_2_ && bAppNum >= 2)
				{
					inAppIndex = 1;
					break;
				} else if (uszKey == _KEY_3_ && bAppNum >= 3)
				{
					inAppIndex = 2;
					break;
				} else if (uszKey == _KEY_4_ && bAppNum >= 4)
				{
					inAppIndex = 3;
					break;
				} else if (uszKey == _KEY_5_ && bAppNum >= 5)
				{
					inAppIndex = 4;
					break;
				} else
				{
					continue;
				}

			}
		}

	}

	/* 停一秒，避免按鍵有聲音會爆音 */
	//	inDISP_Wait(1000);

	/* 將選擇的index塞回去 */
	if (inAppIndex != -1)
	{
		*pbAppSelectedIndex = (unsigned char) inAppIndex;

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Select %d.%s", inAppIndex + 1, pstAppListExData[inAppIndex].cAppLabel);
			inDISP_LogPrintf(szDebugMsg);

			inDISP_LogPrintf("OnEventAppListEx() END !");
			inDISP_LogPrintf("----------------------------------------");
		}

		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_3085 inAppIndex !=-1 OnEventAppListEx END");
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
		/* 記住選項，避免跳多次選單 */
		ginEMVAppSelection = *pbAppSelectedIndex;

		return d_EMVAPLIB_OK;
	} else
	{
		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_3085 inAppIndex = -1 OnEventAppListEx END");
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
		/* uszAppIndex = -1，沒設定index，return False */
		return d_EMVAPLIB_ERR_SELECTION_FAIL;
	}

}

/*
Function        :OnEventDisable_PinNull
Date&Time       :2019/11/13 上午 9:39
Describe        :設定為無法輸入null pin
 */
void OnEventDisable_PINNull(OUT BOOL *IsDisablePINNull)
{
	char szEMVPINBypassEnable[2 + 1] = {0};

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	/* 預設為可以輸入Null PIN */
	*IsDisablePINNull = FALSE;

	/* 如果PINBYPASS開關關閉代表 PIN NULL Disable */
	memset(szEMVPINBypassEnable, 0x00, sizeof (szEMVPINBypassEnable));
	inGetEMVPINBypassEnable(szEMVPINBypassEnable);
	if (memcmp(szEMVPINBypassEnable, "N", 1) == 0)
	{
		*IsDisablePINNull = TRUE;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] PNull[%d] END -----", __FILE__, __FUNCTION__, __LINE__, *IsDisablePINNull);
}

/*
Function        :inEMV_Initial
Date&Time       :2016/12/13 下午 12:52
Describe        :
 */
int inEMV_Initial()
{
	char szDebug[128] = {0};
	char szTMSOK[1 + 1] = {0};
	char szPath[100 + 1] = {0};
	unsigned short ushRetVal = 0x0000;
	/* EMV初始化 */
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("EMV_Initialize START!");

	//	/* 原廠支援debug Production要關 */
	//	if (ginDebug == VS_TRUE)
	//	{
	//		EMV_SetDebug(TRUE, 0xFF);
	//		EMV_SetDebug(TRUE, d_COM2);
	//
	//		/* 若是Debug意外開啟，且和ECR的PORT相同，則會關閉 */
	//		char	szECRComPort[4 + 1];
	//
	//		/* 從EDC.Dat抓出哪一個Comport */
	//		/* inGetECRComPort */
	//		memset(&szECRComPort, 0x00, sizeof(szECRComPort));
	//		inGetECRComPort(szECRComPort);
	//		/* Verifone用handle紀錄，Castle用Port紀錄 */
	//		if (!memcmp(szECRComPort, "COM1", 4))
	//		{
	//			EMV_SetDebug(FALSE, d_COM1);
	//		}
	//		else if (!memcmp(szECRComPort, "COM2", 4))
	//		{
	//			EMV_SetDebug(FALSE, d_COM2);
	//		}
	//		else if (!memcmp(szECRComPort, "COM3", 4))
	//		{
	//			EMV_SetDebug(FALSE, d_COM3);
	//		}
	//		else if (!memcmp(szECRComPort, "USBD", 4))
	//		{
	//			EMV_SetDebug(FALSE, 0xFF);
	//		}
	//
	//	}

	memset(szPath, 0x00, sizeof (szPath));
	sprintf(szPath, "%s%s", _EMV_EMVCL_DATA_PATH_, _EMV_CONFIG_FILENAME_);
	inDISP_LogPrintfWithFlag(szPath);
	ushRetVal = EMV_Initialize(&g_emv_event, szPath);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebug, 0x00, sizeof (szDebug));
		sprintf(szDebug, "EMV_Initialize:0X%04x", ushRetVal);
		inDISP_LogPrintf(szDebug);
	}

	if (ushRetVal != d_EMVAPLIB_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("EMVxml初始化錯誤");
		}

		/* 若下完TMS，且EMV Initial失敗，提示錯誤訊息 */
		memset(szTMSOK, 0x00, sizeof (szTMSOK));
		inGetTMSOK(szTMSOK);
		if (!memcmp(szTMSOK, "Y", 1))
		{
			inDISP_ClearAll();
			inDISP_Msg_BMP(_ERR_INIT_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "EMV XML", _LINE_8_5_);
		}

		return (VS_ERROR);
	}

	/* 註冊Event，*/
	/* appList可以看到選項的AID */
	EMV_SpecialEventRegister(d_EVENTID_APP_LIST_EX, OnEventAppListEx);
	/* 可以讓Force Online流程往前 */
	EMV_SpecialEventRegister(d_EVENTID_TXN_FORCED_ONLINE, OnEventTxnForcedOnline);
	/* Null PIN無法繼續輸入 */
	EMV_SpecialEventRegister(d_EVENTID_DISABLE_PINNULL, OnEventDisable_PINNull); /* 修正EMV函式註冊錯誤的問題 */

	return (VS_SUCCESS);
}

/*
Function        : inEMV_Process
Date&Time   : 2016/12/13 下午 12:53
Describe        :
 */
int inEMV_Process(TRANSACTION_OBJECT *pobTran)
{
#ifndef _MODIFY_EMV_SELECT_AID_	
	int inRetVal;
	char szDebug[200], szTemplate[100 + 1];
#endif
	//	char		szHostName[8 + 1];
	unsigned short ushRetVal;
#ifndef _MODIFY_EMV_SELECT_AID_	
	char szFuncEnable[2 + 1];
	unsigned char uszLabel[32 + 1] = {}, uszLabelLen = 0;
	unsigned char uszSelectedAID[20 + 1] = {}, uszSelectedAIDLen = 0;
	unsigned short ushTagLen;
	unsigned char uszTagData[128];
#endif

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "EVM_3338 inEMV_Process INIT");

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	/* pobTran的資料存入pobEmvTran */
	memset((char *) &pobEmvTran, 0x00, _TRANSACTION_OBJECT_SIZE_);
	memcpy((char *) &pobEmvTran, (char *) pobTran, _TRANSACTION_OBJECT_SIZE_);

#ifndef _MODIFY_EMV_SELECT_AID_
	/* ================================================ */
	/* EMV AID Select */
	inDISP_LogPrintfWithFlag("  EMV_TxnAppSelect  START");

	ushRetVal = EMV_TxnAppSelect(uszSelectedAID, &uszSelectedAIDLen, uszLabel, &uszLabelLen);
	/* 避免因抓卡號導致多次選單，清空 */
	ginEMVAppSelection = -1;

	if (ginDebug == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof (szTemplate));
		inFunc_BCD_to_ASCII(&szTemplate[0], &uszSelectedAID[0], uszSelectedAIDLen);
		memset(szDebug, 0x00, sizeof (szDebug));
		sprintf(szDebug, "SelectedAID:%s", szTemplate);
		inDISP_LogPrintf(szDebug);

		memset(szTemplate, 0x00, sizeof (szTemplate));
		memcpy(&szTemplate[0], &uszLabel[0], uszLabelLen);
		memset(szDebug, 0x00, sizeof (szDebug));
		sprintf(szDebug, "Label:%s", szTemplate);
		inDISP_LogPrintf(szDebug);
	}

	if (ushRetVal != d_EMVAPLIB_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebug, 0x00, sizeof (szDebug));
			sprintf(szDebug, "EMV_TxnAppSelect:0X%04x", ushRetVal);
			inDISP_LogPrintf(szDebug);
		}

		inDISP_DispLogAndWriteFlie("  Emv Process App Select *Error* ushRetVal[0x%04x] Line[%d]", ushRetVal, __LINE__);

		switch (ushRetVal)
		{
			case d_EMVAPLIB_ERR_FUNCTION_NOT_SUPPORTED:
			case d_EMVAPLIB_ERR_TERM_DATA_MISSING:
			case d_EMVAPLIB_ERR_CRITICAL_MISTAKES:
			case d_EMVAPLIB_ERR_DATA_BUFFER_EXCEEDED:
				inEMV_SetICCReadFailure(VS_TRUE); /* 開啟FALL BACK */
				/* 請改刷磁條 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_FALLBACK_;
				break;
			case d_EMVAPLIB_ERR_EVENT_CONFIRMED:
			case d_EMVAPLIB_ERR_EVENT_SELECTED:
			case d_EMVAPLIB_ERR_EVENT_GET_TXNDATA:
			case d_EMVAPLIB_ERR_EVENT_VERSION:
				inEMV_SetICCReadFailure(VS_TRUE); /* 開啟FALL BACK */
				/* 請改刷磁條 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_FALLBACK_;
				break;
			case d_EMVAPLIB_ERR_ONLY_1_AP_NO_FALLBACK:
				//				inDISP_Display(1, 4, "Card Blocked ", FONESIZE_SMALL, CLR_EOL); /* 本交易不成功 */
				//				inSGErrorMessage(ERR_EMV_FAILUER_MSG);
				break;
				/* 找不到該AID的AP，不開啟Fallback */
			case d_EMVAPLIB_ERR_NO_AP_FOUND:
				break;
			case d_SC_NOT_PRESENT:
			case d_EMVAPLIB_ERR_SEND_APDU_CMD_FAIL:
				/* 晶片卡被取出 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;
				break;
			default:
				inEMV_SetICCReadFailure(VS_TRUE); /* 開啟FALL BACK */
				if (inEMV_ICCEvent() != VS_SUCCESS)
				{
					/* 晶片卡被取出 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;
				} else
				{
					/* 請改刷磁條 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_FALLBACK_;
				}
				break;
		}

		return (VS_ERROR);
	} else
	{
		/* 4F (AID) */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(d_TAG_AID, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			/* 一律存AID */
			memset(pobEmvTran.srBRec.szCUP_EMVAID, 0x00, sizeof (pobEmvTran.srBRec.szCUP_EMVAID));
			inFunc_BCD_to_ASCII(pobEmvTran.srBRec.szCUP_EMVAID, uszTagData, ushTagLen);
		}

	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_3338 Aft Select Aid");

	/* 取得Track2資料 */
	if (inCARD_unPackCard(&pobEmvTran) != VS_SUCCESS)
	{

		inDISP_DispLogAndWriteFlie("  Emv Process UnPack Cacrd *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_3338 Aft Unpack Card ");
	/* 第三步驟 判斷card bin 讀HDT */
	if (inCARD_GetBin(&pobEmvTran) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Emv Process Get Bin *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_3338 Aft Card Get Bin");

	/* 第四步驟檢核PAN module 10 */
	memset(szFuncEnable, 0x00, sizeof (szFuncEnable));
	inGetModule10Check(szFuncEnable);
	if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
	{
		/* U CARD 有自己的檢核法 */
		if (!memcmp(pobEmvTran.srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
		{
			if (inCARD_ValidTrack2_UCard_PAN(pobEmvTran.srBRec.szPAN) != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie("  Emv Process Valid Ucard Pan Bin *Error* Line[%d]", __LINE__);
				return (VS_ERROR);
			}

		} else
		{
			if (inCARD_ValidTrack2_PAN(&pobEmvTran) != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie("  Emv Process Valid Track2 Pan Bin *Error* Line[%d]", __LINE__);
				return (VS_ERROR);
			}
		}
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_3338 Aft Get Module 10");

	/* 第五步驟檢核ExpDate */
	memset(szFuncEnable, 0x00, sizeof (szFuncEnable));
	inGetExpiredDateCheck(szFuncEnable);
	if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
	{
		if (inCARD_ValidTrack2_ExpDate(&pobEmvTran) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Emv Process Valid Track2 ExpDate *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		}
	}
	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_3338 Aft Check ExpireDate");
#endif

	/* ================================================ */

	/* 需要卡號才能跑得流程*/
	/* 因為晶片卡到這裡才讀卡號，所以從TRT拉到這邊 */
	if (inFLOW_RunFunction(&pobEmvTran, _NEXSYS_ESC_CHECK_) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  EMV Process _NEXSYS_ESC_CHECK_ *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_3338 Aft Esc Check");

	//		if (inFLOW_RunFunction(&pobEmvTran, _FUNCTION_GET_TRANSACTION_NO_FROM_PAN_) != VS_SUCCESS)
	//		{
	//			inDISP_DispLogAndWriteFlie("  Emv Process Get Ttans No Form Pan *Error* Line[%d]", __LINE__);
	//			return (VS_ERROR);
	//		}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_3338 Aft Get Trans No From Pan ");

	if (inNCCC_DCC_EMV_Set_Value(&pobEmvTran) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  EMV Process Nccc Dcc *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_3338 Aft DCC EMV SET VALUE ");

	inDISP_LogPrintfWithFlag("EMV_TxnPerform START!");
	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_3338 EMV_TxnPerform START");

	/* 用來控制TC Upload前不斷線 */
	pobEmvTran.uszEMVProcessDisconectBit = VS_TRUE;

	inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_); /* 處理中... */

	/* 執行EMV驗證及交易流程，到結束Second Generate AC為止 */
	ushRetVal = EMV_TxnPerform();

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_3338 EMV_TxnPerform END");

	/* 將callback使用的pobEmvTran結構傳回流程在用的pobTran結構 */
	memcpy((char *) pobTran, (char *) &pobEmvTran, _TRANSACTION_OBJECT_SIZE_);

	/* 斷線 */
	inCOMM_End(pobTran);

	/* 不論是Library function執行失敗或是中間callback跑的流程失敗都是不成功 */
	if (ushRetVal != d_EMVAPLIB_OK || ginGlobalRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  EMV Process TxnPerform *Error* Line[%d]", __LINE__);
		inDISP_DispLogAndWriteFlie("  EMV Process  *Error* ushRetVal[0x%04x] ginGlobalRetVal[%d]", ushRetVal, ginGlobalRetVal);

		if (inEMV_ICCEvent() != VS_SUCCESS)
		{
			/* 晶片卡被取出 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;
		}

		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_3338 inEMV_Process  *Error* END");

		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag("EMV_TxnPerform_END");

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_3338 inEMV_Process END");
	inTIME_UNIT_GetGlobalTimeToCompare("EVM_3338 inEMV_Process END");
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inEMV_SecondGenerateAC
Date&Time       :2016/10/3 下午 4:27
Describe        :
 */
int inEMV_SecondGenerateAC(TRANSACTION_OBJECT *pobTran)
{
	char szTagVal[128 + 1];
	char szTemplate[128 + 1];
	unsigned char uszValue[128 + 1];
	unsigned short usTagLen;

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "EVM_3622 inEMV_SecondGenerateAC INIT");

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	usTagLen = sizeof (uszValue);
	memset(uszValue, 0x00, sizeof (uszValue));
	inEMV_Get_Tag_Value(0x9F27, &usTagLen, uszValue);
	memset(szTagVal, 0x00, sizeof (szTagVal));
	inFunc_BCD_to_ASCII(szTagVal, uszValue, usTagLen);

	if (ginDebug == VS_TRUE)
	{
		sprintf(szTemplate, "TAG_9F27 = %s", szTagVal);
		inDISP_LogPrintf(szTemplate);
	}

	/* 根據verifone程式邏輯，只判斷9F27第一碼 */
	if (!memcmp(szTagVal, "4", 1))
	{
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("inEMVAPISecondGenerateAC() TC");

		if (pobTran->inTransactionResult == _TRAN_RESULT_COMM_ERROR_ && pobTran->inTransactionCode == _SALE_)
		{
			/* 這裡應該是 Y3 */
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("inEMV_SecondGenerateAC() Y3");

			pobTran->srEMVRec.in8A_AuthRespCodeLen = 2;
			memcpy(&pobTran->srEMVRec.usz8A_AuthRespCode[0], "Y3", 2);

			memset(pobTran->srBRec.szRespCode, 0x00, sizeof (pobTran->srBRec.szRespCode));
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof (pobTran->srBRec.szAuthCode));
			strcpy(pobTran->srBRec.szRespCode, "Y3");
			strcpy(pobTran->srBRec.szAuthCode, "Y3");
		}
		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_3622 Y3 inEMV_Process END");
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Y3 END -----", __FILE__, __FUNCTION__, __LINE__);
	} else if (!memcmp(szTagVal, "0", 1))
	{
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("inEMV_SecondGenerateAC() AAC!");

		memset(pobTran->srBRec.szRespCode, 0x00, sizeof (pobTran->srBRec.szRespCode));
		memcpy(pobTran->srBRec.szRespCode, "Z3", 2);

		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_3622 Z3 inEMV_Process END");
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Z3 END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}		/* Second Generate AC不會有ARQC，因此判斷為ERROR */
	else
	{
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("inEMV_SecondGenerateAC() ERROR!");

		/* 被卡片拒絕 */
		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_3622 Non inEMV_Process END");
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] 9f27 *Error* END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

int inNCCC_FuncEMVPrepareBatch(TRANSACTION_OBJECT *pobTran)
{
	USHORT usTagLen;
	BYTE value[128 + 1];
	char szAscii[200 + 1];
	char szTemplate[1024 + 1];
	char szDebugMsg[100 + 1];

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inNCCC_FuncEMVPrepareBatch() START!");

	if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
	{
		/* 5A */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x5A, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_5A = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz5A_ApplPan, 0x00, sizeof (pobTran->srEMVRec.usz5A_ApplPan));
		pobTran->srEMVRec.in5A_ApplPanLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz5A_ApplPan[0], &value[0], pobTran->srEMVRec.in5A_ApplPanLen);

		/* 5F24 */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x5F24, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_5F24 = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz5F24_ExpireDate, 0x00, sizeof (pobTran->srEMVRec.usz5F24_ExpireDate));
		pobTran->srEMVRec.in5F24_ExpireDateLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz5F24_ExpireDate[0], &value[0], pobTran->srEMVRec.in5F24_ExpireDateLen);

		/* 5F2A */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x5F2A, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_5F2A = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz5F2A_TransCurrCode, 0x00, sizeof (pobTran->srEMVRec.usz5F2A_TransCurrCode));
		pobTran->srEMVRec.in5F2A_TransCurrCodeLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz5F2A_TransCurrCode[0], &value[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);

		/* 5F34 */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x5F34, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_5F34 = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz5F34_ApplPanSeqnum, 0x00, sizeof (pobTran->srEMVRec.usz5F34_ApplPanSeqnum));
		pobTran->srEMVRec.in5F34_ApplPanSeqnumLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], &value[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);

		/* 8A（Ascii） */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x8A, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			sprintf(szTemplate, "TAG_8A = %s, length = %d", value, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz8A_AuthRespCode, 0x00, sizeof (pobTran->srEMVRec.usz8A_AuthRespCode));
		pobTran->srEMVRec.in8A_AuthRespCodeLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz8A_AuthRespCode[0], &value[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);

		/* 82 */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x82, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_82 = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz82_AIP, 0x00, sizeof (pobTran->srEMVRec.usz82_AIP));
		pobTran->srEMVRec.in82_AIPLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz82_AIP[0], &value[0], pobTran->srEMVRec.in82_AIPLen);

		/* 84 */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x84, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_84 = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz84_DF_NAME, 0x00, sizeof (pobTran->srEMVRec.usz84_DF_NAME));
		pobTran->srEMVRec.in84_DFNameLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz84_DF_NAME[0], &value[0], pobTran->srEMVRec.in84_DFNameLen);

		/* 95 */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x95, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_95 = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz95_TVR, 0x00, sizeof (pobTran->srEMVRec.usz95_TVR));
		pobTran->srEMVRec.in95_TVRLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz95_TVR[0], &value[0], pobTran->srEMVRec.in95_TVRLen);

		/* 9A */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x9A, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_9A = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz9A_TranDate, 0x00, sizeof (pobTran->srEMVRec.usz9A_TranDate));
		pobTran->srEMVRec.in9A_TranDateLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9A_TranDate[0], &value[0], pobTran->srEMVRec.in9A_TranDateLen);

		/* 9B */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x9B, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_9B = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz9B_TSI, 0x00, sizeof (pobTran->srEMVRec.usz9B_TSI));
		pobTran->srEMVRec.in9B_TSILen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9B_TSI[0], &value[0], pobTran->srEMVRec.in9B_TSILen);

		/* 9C */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x9C, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_9C = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz9C_TranType, 0x00, sizeof (pobTran->srEMVRec.usz9C_TranType));
		pobTran->srEMVRec.in9C_TranTypeLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9C_TranType[0], &value[0], pobTran->srEMVRec.in9C_TranTypeLen);

		/* 9F02 */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x9F02, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_9F02 = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F02_AmtAuthNum, 0x00, sizeof (pobTran->srEMVRec.usz9F02_AmtAuthNum));
		pobTran->srEMVRec.in9F02_AmtAuthNumLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F02_AmtAuthNum[0], &value[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);

		/* 9F03 */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x9F03, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_9F03 = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F03_AmtOtherNum, 0x00, sizeof (pobTran->srEMVRec.usz9F03_AmtOtherNum));
		pobTran->srEMVRec.in9F03_AmtOtherNumLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F03_AmtOtherNum[0], &value[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);

		/* 9F09 */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x9F09, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_9F09 = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F09_TermVerNum, 0x00, sizeof (pobTran->srEMVRec.usz9F09_TermVerNum));
		pobTran->srEMVRec.in9F09_TermVerNumLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F09_TermVerNum[0], &value[0], pobTran->srEMVRec.in9F09_TermVerNumLen);

		/* 9F10 */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x9F10, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_9F10 = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F10_IssuerAppData, 0x00, sizeof (pobTran->srEMVRec.usz9F10_IssuerAppData));
		pobTran->srEMVRec.in9F10_IssuerAppDataLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F10_IssuerAppData[0], &value[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);

		/* 9F1A */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x9F1A, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_9F1A = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F1A_TermCountryCode, 0x00, sizeof (pobTran->srEMVRec.usz9F1A_TermCountryCode));
		pobTran->srEMVRec.in9F1A_TermCountryCodeLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F1A_TermCountryCode[0], &value[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);

		/* 9F1E */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x9F1E, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_9F1E = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F1E_IFDNum, 0x00, sizeof (pobTran->srEMVRec.usz9F1E_IFDNum));
		pobTran->srEMVRec.in9F1E_IFDNumLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F1E_IFDNum[0], &value[0], pobTran->srEMVRec.in9F1E_IFDNumLen);

		/* 9F26 */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x9F26, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_9F26 = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F26_ApplCryptogram, 0x00, sizeof (pobTran->srEMVRec.usz9F26_ApplCryptogram));
		pobTran->srEMVRec.in9F26_ApplCryptogramLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F26_ApplCryptogram[0], &value[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);

		/* 9F27 */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x9F27, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_9F27 = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F27_CID, 0x00, sizeof (pobTran->srEMVRec.usz9F27_CID));
		pobTran->srEMVRec.in9F27_CIDLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F27_CID[0], &value[0], pobTran->srEMVRec.in9F27_CIDLen);

		/* 9F33 */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x9F33, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_9F33 = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F33_TermCapabilities, 0x00, sizeof (pobTran->srEMVRec.usz9F33_TermCapabilities));
		pobTran->srEMVRec.in9F33_TermCapabilitiesLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F33_TermCapabilities[0], &value[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);

		/* 9F34 */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x9F34, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_9F34 = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F34_CVM, 0x00, sizeof (pobTran->srEMVRec.usz9F34_CVM));
		pobTran->srEMVRec.in9F34_CVMLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F34_CVM[0], &value[0], pobTran->srEMVRec.in9F34_CVMLen);

		/* 9F35 */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x9F35, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_9F35 = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F35_TermType, 0x00, sizeof (pobTran->srEMVRec.usz9F35_TermType));
		pobTran->srEMVRec.in9F35_TermTypeLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F35_TermType[0], &value[0], pobTran->srEMVRec.in9F35_TermTypeLen);

		/* 9F36 */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x9F36, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_9F36 = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}


		memset(pobTran->srEMVRec.usz9F36_ATC, 0x00, sizeof (pobTran->srEMVRec.usz9F36_ATC));
		pobTran->srEMVRec.in9F36_ATCLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F36_ATC[0], &value[0], pobTran->srEMVRec.in9F36_ATCLen);

		/* 9F37 */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x9F37, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_9F37 = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F37_UnpredictNum, 0x00, sizeof (pobTran->srEMVRec.usz9F37_UnpredictNum));
		pobTran->srEMVRec.in9F37_UnpredictNumLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F37_UnpredictNum[0], &value[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);

		/* 9F41 */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x9F41, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_9F41 = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F41_TransSeqCounter, 0x00, sizeof (pobTran->srEMVRec.usz9F41_TransSeqCounter));
		pobTran->srEMVRec.in9F41_TransSeqCounterLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], &value[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);

		/* 9F5B */
		usTagLen = sizeof (value);
		memset(value, 0x00, sizeof (value));
		EMV_DataGet(0x9F5B, &usTagLen, value);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "TAG_9F5B = %s, length = %d", szAscii, usTagLen);
			inDISP_LogPrintf(szTemplate);
		}


		memset(pobTran->srEMVRec.usz9F5B_ISR, 0x00, sizeof (pobTran->srEMVRec.usz9F5B_ISR));
		pobTran->srEMVRec.in9F5B_ISRLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F5B_ISR[0], &value[0], pobTran->srEMVRec.in9F5B_ISRLen);

		if (pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEC_FALLBACK_INDICATOR, (unsigned char *) &szTemplate[0]) > 0)
			{
				memset(pobTran->srEMVRec.uszDFEC_FallBackIndicator, 0x00, sizeof (pobTran->srEMVRec.uszDFEC_FallBackIndicator));
				pobTran->srEMVRec.inDFEC_FallBackIndicatorLen = szTemplate[2];
				memcpy((char *) &pobTran->srEMVRec.uszDFEC_FallBackIndicator[0], &szTemplate[3], pobTran->srEMVRec.inDFEC_FallBackIndicatorLen);
			}

			memset(szTemplate, 0x00, sizeof (szTemplate));
			if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFED_CHIP_CONDITION_CODE, (unsigned char *) &szTemplate[0]) > 0)
			{
				memset(pobTran->srEMVRec.uszDFED_ChipConditionCode, 0x00, sizeof (pobTran->srEMVRec.uszDFED_ChipConditionCode));
				pobTran->srEMVRec.inDFED_ChipConditionCodeLen = szTemplate[2];
				memcpy((char *) &pobTran->srEMVRec.uszDFED_ChipConditionCode[0], &szTemplate[3], pobTran->srEMVRec.inDFED_ChipConditionCodeLen);
			}

			memset(szTemplate, 0x00, sizeof (szTemplate));
			if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEE_TERMINAL_ENTRY_CAPABILITY, (unsigned char *) &szTemplate[0]) > 0)
			{
				memset(pobTran->srEMVRec.uszDFEE_TerEntryCap, 0x00, sizeof (pobTran->srEMVRec.uszDFEE_TerEntryCap));
				pobTran->srEMVRec.inDFEE_TerEntryCapLen = szTemplate[2];
				memcpy((char *) &pobTran->srEMVRec.uszDFEE_TerEntryCap[0], &szTemplate[3], pobTran->srEMVRec.inDFEE_TerEntryCapLen);
			}

			memset(szTemplate, 0x00, sizeof (szTemplate));
			if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEF_REASON_ONLINE_CODE, (unsigned char *) &szTemplate[0]) > 0)
			{
				memset(pobTran->srEMVRec.uszDFEF_ReasonOnlineCode, 0x00, sizeof (pobTran->srEMVRec.uszDFEF_ReasonOnlineCode));
				pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen = szTemplate[2];
				memcpy((char *) &pobTran->srEMVRec.uszDFEF_ReasonOnlineCode[0], &szTemplate[3], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);
			}
		} else
		{
			if (pobTran->inTransactionCode == _SALE_ ||
					pobTran->inTransactionCode == _REDEEM_SALE_ ||
					pobTran->inTransactionCode == _INST_SALE_ ||
					pobTran->inTransactionCode == _PRE_AUTH_ ||
					pobTran->inTransactionCode == _TC_UPLOAD_)
			{
				/* 先hot code */
				/* DFEE */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEE_TERMINAL_ENTRY_CAPABILITY, (unsigned char *) &szTemplate[0]) > 0)
				{
					memset(pobTran->srEMVRec.uszDFEE_TerEntryCap, 0x00, sizeof (pobTran->srEMVRec.uszDFEE_TerEntryCap));
					pobTran->srEMVRec.inDFEE_TerEntryCapLen = szTemplate[2];
					memcpy((char *) &pobTran->srEMVRec.uszDFEE_TerEntryCap[0], &szTemplate[3], pobTran->srEMVRec.inDFEE_TerEntryCapLen);
				}

				if (ginDebug == VS_TRUE)
				{
					memset(szAscii, 0x00, sizeof (szAscii));
					inFunc_BCD_to_ASCII(szAscii, (unsigned char*) &szTemplate[3], pobTran->srEMVRec.inDFEE_TerEntryCapLen);
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "TAG_DFEE = %s, length = %d", szAscii, pobTran->srEMVRec.inDFEE_TerEntryCapLen);
					inDISP_LogPrintf(szDebugMsg);
				}

				/* DFEF */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEF_REASON_ONLINE_CODE, (unsigned char *) &szTemplate[0]) > 0)
				{
					memset(pobTran->srEMVRec.uszDFEF_ReasonOnlineCode, 0x00, sizeof (pobTran->srEMVRec.uszDFEF_ReasonOnlineCode));
					pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen = szTemplate[2];
					memcpy((char *) &pobTran->srEMVRec.uszDFEF_ReasonOnlineCode[0], &szTemplate[3], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);
				}

				if (ginDebug == VS_TRUE)
				{
					memset(szAscii, 0x00, sizeof (szAscii));
					inFunc_BCD_to_ASCII(szAscii, (unsigned char*) &szTemplate[3], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "TAG_DFEF = %s, length = %d", szAscii, pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);
					inDISP_LogPrintf(szDebugMsg);
				}

			}

		}

	} else if (pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof (szTemplate));
		if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEC_FALLBACK_INDICATOR, (unsigned char *) &szTemplate[0]) > 0)
		{
			memset(pobTran->srEMVRec.uszDFEC_FallBackIndicator, 0x00, sizeof (pobTran->srEMVRec.uszDFEC_FallBackIndicator));
			pobTran->srEMVRec.inDFEC_FallBackIndicatorLen = szTemplate[2];
			memcpy((char *) &pobTran->srEMVRec.uszDFEC_FallBackIndicator[0], &szTemplate[3], pobTran->srEMVRec.inDFEC_FallBackIndicatorLen);
		}

		memset(szTemplate, 0x00, sizeof (szTemplate));
		if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFED_CHIP_CONDITION_CODE, (unsigned char *) &szTemplate[0]) > 0)
		{
			memset(pobTran->srEMVRec.uszDFED_ChipConditionCode, 0x00, sizeof (pobTran->srEMVRec.uszDFED_ChipConditionCode));
			pobTran->srEMVRec.inDFED_ChipConditionCodeLen = szTemplate[2];
			memcpy((char *) &pobTran->srEMVRec.uszDFED_ChipConditionCode[0], &szTemplate[3], pobTran->srEMVRec.inDFED_ChipConditionCodeLen);
		}

		memset(szTemplate, 0x00, sizeof (szTemplate));
		if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEE_TERMINAL_ENTRY_CAPABILITY, (unsigned char *) &szTemplate[0]) > 0)
		{
			memset(pobTran->srEMVRec.uszDFEE_TerEntryCap, 0x00, sizeof (pobTran->srEMVRec.uszDFEE_TerEntryCap));
			pobTran->srEMVRec.inDFEE_TerEntryCapLen = szTemplate[2];
			memcpy((char *) &pobTran->srEMVRec.uszDFEE_TerEntryCap[0], &szTemplate[3], pobTran->srEMVRec.inDFEE_TerEntryCapLen);
		}

		memset(szTemplate, 0x00, sizeof (szTemplate));
		if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEF_REASON_ONLINE_CODE, (unsigned char *) &szTemplate[0]) > 0)
		{
			memset(pobTran->srEMVRec.uszDFEF_ReasonOnlineCode, 0x00, sizeof (pobTran->srEMVRec.uszDFEF_ReasonOnlineCode));
			pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen = szTemplate[2];
			memcpy((char *) &pobTran->srEMVRec.uszDFEF_ReasonOnlineCode[0], &szTemplate[3], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);
		}

	}

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inNCCC_FuncEMVPrepareBatch() END!");

	return (VS_SUCCESS);
}

int inNCCC_EMVUnPackData55(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf, int inLen)
{
	int inTotalLen;
	unsigned short usTag;

	memset(pobTran->srEMVRec.usz8A_AuthRespCode, 0x00, sizeof (pobTran->srEMVRec.usz8A_AuthRespCode));
	pobTran->srEMVRec.in8A_AuthRespCodeLen = strlen(pobTran->srBRec.szRespCode);
	memcpy(&pobTran->srEMVRec.usz8A_AuthRespCode[0], &pobTran->srBRec.szRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);

	for (inTotalLen = 0; inTotalLen < inLen;)
	{
		usTag = (unsigned short) uszUnPackBuf[inTotalLen];

		if (uszUnPackBuf[inTotalLen++] & 0x1F00)
			usTag += ((unsigned short) uszUnPackBuf[inTotalLen++]);

		switch (usTag)
		{
			case _TAG_91_ISS_AUTH_DATA_:
				memset(pobTran->srEMVRec.usz91_IssuerAuthData, 0x00, sizeof (pobTran->srEMVRec.usz91_IssuerAuthData));
				pobTran->srEMVRec.in91_IssuerAuthDataLen = (unsigned short) uszUnPackBuf[inTotalLen++];
				memcpy(&pobTran->srEMVRec.usz91_IssuerAuthData[0], (char *) &uszUnPackBuf[inTotalLen], pobTran->srEMVRec.in91_IssuerAuthDataLen);
				inTotalLen += pobTran->srEMVRec.in91_IssuerAuthDataLen;
				break;
			case _TAG_71_ISUER_SCRPT_TEMPL_:
				memset(pobTran->srEMVRec.usz71_IssuerScript1, 0x00, sizeof (pobTran->srEMVRec.usz71_IssuerScript1));
				pobTran->srEMVRec.in71_IssuerScript1Len = (unsigned short) uszUnPackBuf[inTotalLen++] + 2;
				memcpy(&pobTran->srEMVRec.usz71_IssuerScript1[0], (char *) &uszUnPackBuf[inTotalLen - 2], pobTran->srEMVRec.in71_IssuerScript1Len);
				inTotalLen += pobTran->srEMVRec.in71_IssuerScript1Len - 2;
				break;
			case _TAG_72_ISUER_SCRPT_TEMPL_:
				memset(pobTran->srEMVRec.usz72_IssuerScript2, 0x00, sizeof (pobTran->srEMVRec.usz72_IssuerScript2));
				pobTran->srEMVRec.in72_IssuerScript2Len = (unsigned short) uszUnPackBuf[inTotalLen++] + 2;
				memcpy(&pobTran->srEMVRec.usz72_IssuerScript2[0], (char *) &uszUnPackBuf[inTotalLen - 2], pobTran->srEMVRec.in72_IssuerScript2Len);
				inTotalLen += pobTran->srEMVRec.in72_IssuerScript2Len - 2;
				break;
			default:
				return (VS_SUCCESS);
		}
	}

	return (VS_SUCCESS);
}

/*
Function        :inEMV_Set_TagValue_During_Txn
Date&Time       :2016/9/30 下午 1:28
Describe        :交易中直接改Tag值，基本上不會直接設定，這邊因為DCC會用到
 */
int inEMV_Set_TagValue_During_Txn(unsigned short usTag, unsigned short usLen, unsigned char *uszValue)
{
	char szDebugMsg[300 + 1];
	char szAscii[300 + 1];
	unsigned short usRetVal;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inEMV_Set_TagValue_During_Txn() START !");

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Tag :%X Len :%d Value :%s", usTag, usLen, uszValue);
		inDISP_LogPrintf(szDebugMsg);

		memset(szAscii, 0x00, sizeof (szAscii));
		inFunc_BCD_to_ASCII(szAscii, uszValue, usLen);
		sprintf(szDebugMsg, "Ascii : %s", szAscii);
		inDISP_LogPrintf(szDebugMsg);
	}

	usRetVal = EMV_TxnDataSet(usTag, usLen, uszValue);

	/* 不論是Library function執行失敗或是中間callback跑的流程失敗都是不成功 */
	if (usRetVal != d_EMVAPLIB_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Tag :%X Len :%d Value :%s", usTag, usLen, uszValue);
			inDISP_LogPrintf(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "EMV_TxnDataSet_ERR:0X%04x", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}

		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inEMV_Set_TagValue_During_Txn() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_EMV_Set_Value
Date&Time       :2016/9/30 下午 1:43
Describe        :若是DCC 交易要改5F2A和9F02 兩個Tag，而且晶片卡都force online
 */
int inNCCC_DCC_EMV_Set_Value(TRANSACTION_OBJECT * pobTran)
{
	int inRetVal;
	char szTemplate[100 + 1];
	unsigned char uszHex[100 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inNCCC_DCC_EMV_Set_Value() START !");
	}

	if (pobTran->srBRec.uszDCCTransBit != VS_TRUE)
	{
		return (VS_SUCCESS);
	}

	/* 5F2A */
	memset(szTemplate, 0x00, sizeof (szTemplate));
	memset(uszHex, 0x00, sizeof (uszHex));
	sprintf(szTemplate, "0%s", pobEmvTran.srBRec.szDCC_FCN);
	inFunc_ASCII_to_BCD(uszHex, szTemplate, 2);
	inRetVal = inEMV_Set_TagValue_During_Txn(d_TAG_TERM_CURRENCY_CODE, 2, (unsigned char*) uszHex);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 9F02 */
	memset(uszHex, 0x00, sizeof (uszHex));
	inFunc_ASCII_to_BCD(uszHex, pobEmvTran.srBRec.szDCC_FCA, 6);
	inRetVal = inEMV_Set_TagValue_During_Txn(d_TAG_AMOUNT_AUTHORIZED, 6, (unsigned char*) uszHex); /* 要注意ascii to bcd會出現0x00，strlen會出錯 */
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* DCC交易所有EMV晶片卡一律Force go online，不可有離線授權之交易 (Y1 及Y3 授權) (NCCC規格書)，這邊直接把Floor limit設為0 */
	memset(uszHex, 0x00, sizeof (uszHex));
	inFunc_ASCII_to_BCD(uszHex, "00", 2);
	inRetVal = inEMV_Set_TagValue_During_Txn(d_TAG_FLOOR_LIMIT, 4, uszHex);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inNCCC_DCC_EMV_Set_Value() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inEMV_Get_Tag_Value
Date&Time       :2016/10/3 上午 11:19
Describe        :usLen放uszValue陣列的大小
 */
int inEMV_Get_Tag_Value(unsigned short usTag, unsigned short *usLen, unsigned char *uszValue)
{
	int inRetVal;
	char szDebugMsg[300 + 1];
	char szAscii[300 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inEMV_Get_Tag_Value() START !");
	}

	memset(uszValue, 0x00, *usLen);
	inRetVal = EMV_DataGet(usTag, usLen, uszValue);

	if (inRetVal == d_EMVAPLIB_OK && *usLen > 0)
	{
		/* ISO Display Debug */
		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, uszValue, *usLen);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "TAG %X = %s, Length = %d", usTag, uszValue, *usLen);
			inDISP_LogPrintf(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Ascii : %s", szAscii);
			inDISP_LogPrintf(szDebugMsg);
		}
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inEMV_Get_Tag_Value() ERROR !");

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "EMV_DataGet_ERR:0X%04x", inRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}

	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inEMV_Get_Tag_Value() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inEMV_SetICCReadFailure
Date&Time       :2016/12/5 下午 3:44
Describe        :開啟Fallback
 */
int inEMV_SetICCReadFailure(int inFallbackSwitch)
{
	if (inFallbackSwitch == VS_TRUE)
	{
		ginFallback = VS_TRUE;
	} else if (inFallbackSwitch == VS_FALSE)
	{
		ginFallback = VS_FALSE;
	} else
	{
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inEMV_GetKernelVersion
Date&Time       :2016/12/28 上午 11:20
Describe        :
 */
int inEMV_GetKernelVersion()
{
	int inRetVal;
	char szKernelVersion[100 + 1];
	char szDebugMsg[100 + 1];

	memset(szKernelVersion, 0x00, sizeof (szKernelVersion));
	inRetVal = EMV_TxnKernelVersionGet((unsigned char*) szKernelVersion);
	if (inRetVal != d_EMVAPLIB_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inEMV_GetKernelVersion() ERROR !");

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "EMV_DataGet_ERR:0X%04x", inRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Kernel Version : %s", szKernelVersion);
			inDISP_LogPrintf(szDebugMsg);
		}
	}

	return (VS_SUCCESS);
}

/*
Function        :inEMV_GetCardNoFlow
Date&Time       :2017/3/6 上午 10:07
Describe        :抓卡號，目前只For HG 使用
 */
int inEMV_GetCardNoFlow(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;
	char szDebug[200], szTemplate[64 + 1];
	unsigned short ushRetVal;
	unsigned char uszLabel[32 + 1] = {}, uszLabelLen = 0;
	unsigned char uszSelectedAID[20 + 1] = {}, uszSelectedAIDLen = 0;
	unsigned short ushTagLen;
	unsigned char uszTagData[128];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, " EVM_4571 GetCardNoFlow INIT");

	/* pobTran的資料存入pobEmvTran */
	memset((char *) &pobEmvTran, 0x00, _TRANSACTION_OBJECT_SIZE_);
	memcpy((char *) &pobEmvTran, (char *) pobTran, _TRANSACTION_OBJECT_SIZE_);

	/* ================================================ */
	/* EMV AID Select */
	ushRetVal = EMV_TxnAppSelect(uszSelectedAID, &uszSelectedAIDLen, uszLabel, &uszLabelLen);

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_4571 Aft TxnAppSelect ");

	if (ginDebug == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof (szTemplate));
		inFunc_BCD_to_ASCII(&szTemplate[0], &uszSelectedAID[0], uszSelectedAIDLen);
		memset(szDebug, 0x00, sizeof (szDebug));
		sprintf(szDebug, "SelectedAID:%s", szTemplate);
		inDISP_LogPrintf(szDebug);

		memset(szTemplate, 0x00, sizeof (szTemplate));
		memcpy(&szTemplate[0], &uszLabel[0], uszLabelLen);
		memset(szDebug, 0x00, sizeof (szDebug));
		sprintf(szDebug, "Label:%s", szTemplate);
		inDISP_LogPrintf(szDebug);
	}

	if (ushRetVal != d_EMVAPLIB_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebug, 0x00, sizeof (szDebug));
			sprintf(szDebug, "EMV_TxnAppSelect:0X%04x", ushRetVal);
			inDISP_LogPrintf(szDebug);
		}

		switch (ushRetVal)
		{
			case d_EMVAPLIB_ERR_FUNCTION_NOT_SUPPORTED:
			case d_EMVAPLIB_ERR_TERM_DATA_MISSING:
			case d_EMVAPLIB_ERR_CRITICAL_MISTAKES:
			case d_EMVAPLIB_ERR_DATA_BUFFER_EXCEEDED:
				inEMV_SetICCReadFailure(VS_TRUE); /* 開啟FALL BACK */
				/* 請改刷磁條 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_FALLBACK_;
				break;
			case d_EMVAPLIB_ERR_EVENT_CONFIRMED:
			case d_EMVAPLIB_ERR_EVENT_SELECTED:
			case d_EMVAPLIB_ERR_EVENT_GET_TXNDATA:
			case d_EMVAPLIB_ERR_EVENT_VERSION:
				inEMV_SetICCReadFailure(VS_TRUE); /* 開啟FALL BACK */
				/* 請改刷磁條 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_FALLBACK_;
				break;
			case d_EMVAPLIB_ERR_ONLY_1_AP_NO_FALLBACK:
				//				inDISP_Display(1, 4, "Card Blocked ", FONESIZE_SMALL, CLR_EOL); /* 本交易不成功 */
				//				inSGErrorMessage(ERR_EMV_FAILUER_MSG);
				/* 請改刷磁條 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_FALLBACK_;
				break;
				/* 找不到該AID的AP，不開啟Fallback */
			case d_EMVAPLIB_ERR_NO_AP_FOUND:
				break;
			case d_SC_NOT_PRESENT:
			case d_EMVAPLIB_ERR_SEND_APDU_CMD_FAIL:
				/* 晶片卡被取出 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;
				break;
			default:
				inEMV_SetICCReadFailure(VS_TRUE); /* 開啟FALL BACK */
				if (inEMV_ICCEvent() != VS_SUCCESS)
				{
					/* 晶片卡被取出 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;
				} else
				{
					/* 請改刷磁條 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_FALLBACK_;
				}
				break;
		}

		return (VS_ERROR);
	} else
	{
		/* 4F (AID) */
		ushTagLen = sizeof (uszTagData);
		memset(uszTagData, 0x00, sizeof (uszTagData));
		inRetVal = EMV_DataGet(d_TAG_AID, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK && ushTagLen > 0)
		{
			/* 一律存AID */
			memset(pobEmvTran.srBRec.szCUP_EMVAID, 0x00, sizeof (pobEmvTran.srBRec.szCUP_EMVAID));
			inFunc_BCD_to_ASCII(pobEmvTran.srBRec.szCUP_EMVAID, uszTagData, ushTagLen);
		}

	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_4571 Bef UnPackCard ");

	/* 取得Track2資料 */
	if (inCARD_unPackCard(&pobEmvTran) != VS_SUCCESS)
		return (VS_ERROR);

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVM_4571 Aft UnPackCard ");

	/* 將callback使用的pobEmvTran結構傳回流程在用的pobTran結構 */
	memcpy((char *) pobTran, (char *) &pobEmvTran, _TRANSACTION_OBJECT_SIZE_);

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
Function        :inEMV_Decide_OnlineEMV_Complete
Date&Time   :2016/10/3 下午 5:25
Describe        :分Host流程
 */
int inEMV_Decide_OnlineEMV_Complete()
{
	int inRetVal;
	char szTRTFileName[12 + 1];
	char szFesMode[2 + 1];
	char szHostName[8 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memset(szTRTFileName, 0x00, sizeof (szTRTFileName));
	inGetTRTFileName(szTRTFileName);

	memset(szFesMode, 0x00, sizeof (szFesMode));
	inGetNCCCFESMode(szFesMode);

	/* 這裏可依照各主機判斷的不同進行分流  */
	if (!memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
	{
		memset(szHostName, 0x00, sizeof (szHostName));
		inGetHostLabel(szHostName);
		/*TODO: 看能不能寫成可變動功能 */
		if (memcmp(szHostName, _HOST_NAME_CREDIT_FUBON_, strlen(_HOST_NAME_CREDIT_FUBON_)) == 0)
		{
			inRetVal = inFUBON_OnlineEMV_Complete(&pobEmvTran);
		} else
		{
			inDISP_DispLogAndWriteFlie(" EMV Decide Online Emv *Error* szHostName[%s]  Line[%d]", szHostName, __LINE__);
			inRetVal = VS_ERROR;
		}

		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] CREDIT END -----", __FILE__, __FUNCTION__, __LINE__);
		return (inRetVal);
	} else
	{
		inDISP_DispLogAndWriteFlie(" EMV Decide Online Emv *Error* szTRTFileName[%s]  Line[%d]", szTRTFileName, __LINE__);
		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);

}

/*
Function        : inEMV_Decide_DispHostRespCodeMsg
Date&Time   : 2016/11/15 下午 6:25
Describe        : 分顯示錯誤代碼流程
 */
int inEMV_Decide_DispHostRespCodeMsg()
{
	int inRetVal;
	char szDebugMsg[100 + 1];
	char szTRTFileName[12 + 1];

	memset(szTRTFileName, 0x00, sizeof (szTRTFileName));
	inGetTRTFileName(szTRTFileName);

	if (!memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
	{
		/* 由原本 inNCCC_ATS_DispHostResponseCode 修改為富邦的 20190328 [SAM]   */
		inRetVal = inFUBON_DispHostResponseCode(&pobEmvTran);
		return (inRetVal);
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inEMV_Decide_DispHostRespCodeMsg :EMV無此TRT流程 :%s", szTRTFileName);
			inDISP_LogPrintf(szDebugMsg);
		}

		return (VS_ERROR);
	}

}

int inEMV_FubonRunCupLogOn(TRANSACTION_OBJECT *pobTran, char * szCheckHostName)
{
	int inCheckResult = VS_SUCCESS;
	int inCode, inTransactionCode, inISOTxnCode;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	if (memcmp(szCheckHostName, _HOST_NAME_CREDIT_FUBON_, strlen(_HOST_NAME_CREDIT_FUBON_)) == 0)
	{
		inCode = pobTran->srBRec.inCode;
		inTransactionCode = pobTran->inTransactionCode;
		inISOTxnCode = pobTran->inISOTxnCode;

		inCheckResult = inFUBON_ISO_CUP_FuncAutoLogon(pobTran);

		pobTran->srBRec.inCode = inCode;
		pobTran->inTransactionCode = inTransactionCode;
		pobTran->inISOTxnCode = inISOTxnCode;

	} else
	{
		inDISP_DispLogAndWriteFlie(" EMV Fubon Cup Logon *Error* CheckHostName[%s]  Line[%d]", szCheckHostName, __LINE__);
		inCheckResult = VS_ERROR;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);

	return inCheckResult;
}


