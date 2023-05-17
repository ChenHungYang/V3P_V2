#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <ctos_clapi.h>
#include <emv_cl.h>
#include <sqlite3.h>

#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Card.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/VWT.h"
#include "../SOURCE/FUNCTION/MVT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/EVENT/Event.h"
#include "../SOURCE/FUNCTION/KMS.h"
#include "../SOURCE/FUNCTION/ECR_FUNC/ECR.h"
/* [外接設備設定] */
#include "../SOURCE/FUNCTION/MULTI_FUNC/MultiFunc.h"
#include "../SOURCE/FUNCTION/MULTI_FUNC/MultiHostFunc.h"
#include "../SOURCE/FUNCTION/MULTI_FUNC/JsonMultiHostFunc.h"
#include "../EMVSRC/EMVsrc.h"

#include "CTLS.h"

#include "../SOURCE/FUNCTION/COSTCO_FUNC/Costco.h"

extern int ginDebug;
extern int ginISODebug;
extern int ginDisplayDebug;
extern int ginMachineType;
extern EMV_CONFIG EMVGlobConfig;

extern TRANSACTION_OBJECT pobEmvTran;

EMVCL_INIT_DATA emvcl_initdat;
EMVCL_ACT_DATA szACTData;
EMVCL_RC_DATA_EX szRCDataEx;
EMVCL_RC_DATA_ANALYZE szRCDataAnalyze;
CTLS_OBJECT srCtlsObj;
unsigned char guszCTLSInitiOK = VS_FALSE;

/* [外接設備設定] */
extern MULTI_TABLE gstMultiOb;

#ifdef _DEBUG_9F1F_TAG_
extern unsigned char gusz9F1F[129];
extern long gln9F1FLen;
#endif
/*
Function        :inCTLS_InitReader_Flow
Date&Time   : 2017/4/20 下午 1:41
Describe        :
*/
int inCTLS_InitReader_Flow(void)
{
	int inRetVal = VS_ERROR;
	char szCTLSMode[1 + 1];
	char szDebugMsg[128 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_InitReader_Flow()START");
	}

	memset(szCTLSMode, 0x00, sizeof(szCTLSMode));
	inGetContactlessReaderMode(szCTLSMode);
	/* 不使用Reader */
	if (memcmp(szCTLSMode, _CTLS_MODE_0_NO_, strlen(_CTLS_MODE_0_NO_)) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS MODE :None");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	/* 內建Reader */
	else if (memcmp(szCTLSMode, _CTLS_MODE_1_INTERNAL_, strlen(_CTLS_MODE_1_INTERNAL_)) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS MODE :Internal");
			inDISP_LogPrintf(szDebugMsg);
		}

		inRetVal = inCTLS_InitReader_Internal();
	}
	/* 外接Reader */
	else if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
			 !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)))
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS MODE :External");
			inDISP_LogPrintf(szDebugMsg);
		}

/* [外接設備設定] */
#ifdef __MUTI_FUCN_TEST_
		inRetVal = inMultiFunc_Host_InitialComPort(tbGetPobTranPoint());
#else
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("感應初始化失敗：外接", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_CENTER_);
		inDISP_Wait(2000);
		inRetVal = VS_ERROR;
#endif
	}

	if (inRetVal == VS_SUCCESS)
	{
		guszCTLSInitiOK = VS_TRUE;
	}
	else
	{
		guszCTLSInitiOK = VS_FALSE;
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_InitReader_Flow()END");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_InitReader_Internal
Date&Time       :2017/4/20 下午 1:41
Describe        :
*/
int inCTLS_InitReader_Internal(void)
{
	int inRetVal = VS_SUCCESS;
	char szTMSOK[1 + 1] = {0};
	char szDebugMsg[128 + 1] = {0};
	char szPath[100 + 1] = {0};

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_InitReader_Internal()START");
	}

	/* 原廠支援debug Production要關 */
	if (ginDebug == VS_TRUE)
	{
		//		EMVCL_SetDebug(TRUE, d_EMVCL_DEBUG_PORT_USB);
		//		EMVCL_SetDebug(TRUE, d_EMVCL_DEBUG_PORT_COM2);
	}

	/* OnCancelTransaction:
	 * (1) Implement a function for cancel transaction. When the event occurs,
	 *  returning TRUE indicates that the current transaction will be canceled, while returning FALSE indicates to continue the processing.
	 * (2) If OnCancelTransaction is not implemented (i.e Event_fCancelTransaction is set to NULL). No cancellation will be performed.
	 */
	/* OnShowMessage:
	 * (1) Implement a function to show messages during transaction.
	 * bKernel: Indicates which kernel to request this message to display
	 * baUserInterfaceRequestData: the information from kernel to inform customer about the progress of transaction.
	 * (Please refer to “EMV Contactless Specification for Payment Systems Book A” for detail)
	 * (2) If OnShowMessage is not implemented. No message will be shown during transaction.
	 */
	emvcl_initdat.stOnEvent.OnCancelTransaction = NULL;
	emvcl_initdat.stOnEvent.OnShowMessage = NULL;
	memset(szPath, 0x00, sizeof(szPath));
	sprintf(szPath, "%s%s", _EMV_EMVCL_DATA_PATH_, _EMVCL_CONFIG_FILENAME_);
	emvcl_initdat.bConfigFilenameLen = strlen(szPath);
	emvcl_initdat.pConfigFilename = (unsigned char *)szPath;

	/* 虹堡回覆:
	 * EMVCL 設定的方式有分成兩種
	 * 1. 透過 xml 設定
	 * 2. 透過 API 設定
	 * 因此 xml 檔案是可選的，不用 xml 並不會造成問題。
	 */
	/*
 * RETURN VALUE(EMVCL_Initialize)
   d_EMVCL_NO_ERROR
   d_EMVCL_RC_FAILURE
 */
	inRetVal = EMVCL_Initialize(&emvcl_initdat);

	if (inRetVal != d_EMVCL_NO_ERROR)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "EMVCL_InitERR:0X%04x", inRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}

		/* 若下完TMS，且EMV Initial失敗，提示錯誤訊息 */
		memset(szTMSOK, 0x00, sizeof(szTMSOK));
		inGetTMSOK(szTMSOK);
		if (!memcmp(szTMSOK, "Y", 1))
		{
			inDISP_ClearAll();
			inDISP_Msg_BMP(_ERR_INIT_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "EMVCL", _LINE_8_5_);
		}

		return (VS_ERROR);
	}

	/* 註冊Event，判斷SmartPay的call back Function */
	inRetVal = EMVCL_SpecialEventRegister(d_EMVCL_EVENTID_PRE_NON_EMV_CARD, vdCTLS_EVENT_EMVCL_PRE_NON_EMV_CARD);
	if (inRetVal != d_EMVCL_NO_ERROR)
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "EMVCL_REGERR:0X%04x", inRetVal);
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
		}
	}

	/* 設定LED燈號模式 */
	inCTLS_SetUIType();

	/* 開啟Wave 2感應 */
	inCTLS_Wave2TransactionEnable();

	inCTLS_Clear_LED();

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_InitReader_Internal()END");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_DeInitReader_Flow
Date&Time       :2018/3/21 上午 10:12
Describe        :
*/
int inCTLS_DeInitReader_Flow(void)
{
	int inRetVal = VS_ERROR;
	char szCTLSMode[1 + 1];
	char szDebugMsg[128 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_DeInitReader_Flow()START");
	}

	memset(szCTLSMode, 0x00, sizeof(szCTLSMode));
	inGetContactlessReaderMode(szCTLSMode);
	/* 不使用Reader */
	if (memcmp(szCTLSMode, _CTLS_MODE_0_NO_, strlen(_CTLS_MODE_0_NO_)) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS MODE :None");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	/* 內建Reader */
	else if (memcmp(szCTLSMode, _CTLS_MODE_1_INTERNAL_, strlen(_CTLS_MODE_1_INTERNAL_)) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS MODE :Internal");
			inDISP_LogPrintf(szDebugMsg);
		}

		inRetVal = inCTLS_DeInitReader_Internal();
	}
	/* 外接Reader */
	else if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
			 !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)))
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS MODE :External");
			inDISP_LogPrintf(szDebugMsg);
		}
/* [外接設備設定] */
#ifdef __MUTI_FUCN_TEST_
		inRetVal = inMultiFunc_CallSlave(tbGetPobTranPoint(), _MULTI_POLL_NO_, &gstMultiOb);
		//		inRetVal = inMultiFunc_JsonCallSlave(tbGetPobTranPoint(), _MULTI_POLL_NO_, &gstMultiOb);
#else
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("感應初始化失敗：外接", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_CENTER_);
		inDISP_Wait(2000);
		inRetVal = VS_ERROR;
#endif
	}

	if (inRetVal == VS_SUCCESS)
	{
		guszCTLSInitiOK = VS_TRUE;
	}
	else
	{
		guszCTLSInitiOK = VS_FALSE;
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_DeInitReader_Flow()END");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_DeInitReader_Internal
Date&Time       :2017/4/20 下午 1:41
Describe        :
*/
int inCTLS_DeInitReader_Internal(void)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_DeInitReader_Internal()START");
	}

	EMVCL_Close();

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_DeInitReader_Internal()END");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_Wave2TransactionEnable
Date&Time       :2016/12/14 上午 10:58
Describe        :虹堡回覆：Wave 2 的 Scheme 預設是不開啟 ，請參考以下開啟後，再試一次交易
 *		Visa has several contactless specification in the field, such as Wave1 card,
 *		MSD, Wave3 (qVSDC) card. This function is used to enable/disable to support such
 *		kind of Visa contactless cards by setting the corresponding scheme ID.
 *		The supported scheme IDs are as below.
 *		VISA_WAVE_QVSDC 0x17
 *
 *		baAction : Action field with setting 0x01 indicates “Active” while 0x00 indicates “Deactive”.
*/
int inCTLS_Wave2TransactionEnable(void)
{
	char szDebugMsg[100 + 1];
	unsigned long ulRetval;
	EMVCL_SCHEME_DATA stScheme, stRsp;

	stScheme.bNoS = 1;
	memcpy(stScheme.baID, "\x16", 1);
	memcpy(stScheme.baAction, "\x01", 1);

	ulRetval = EMVCL_VisaSetCapability(&stScheme, &stRsp);

	if (ulRetval != d_EMVCL_NO_ERROR)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inCTLS_Wave2TransactionEnable Failed");
			inDISP_LogPrintf(szDebugMsg);
		}
	}

	return (VS_SUCCESS);
}

/*
Function	:inCTLS_SetUIType
Date&Time	:2016/2/3 上午 10:39
Describe	:設定LED燈號模式
*/
int inCTLS_SetUIType(void)
{
	/*
	   [IN] bType  UI Type
	   0x00  Normal UI
	   0x01  EUR. UI
	*/
	EMVCL_SetUIType(0x00);
	inDISP_ClearAll();

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_DevicePooling_Flow
Date&Time       :2017/9/7 下午 2:38
Describe        :目前無使用
*/
int inCTLS_DevicePooling_Flow(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR;
	char szCTLSMode[1 + 1];
	char szDebugMsg[100 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_DevicePooling_Flow()START");
	}

	memset(szCTLSMode, 0x00, sizeof(szCTLSMode));
	inGetContactlessReaderMode(szCTLSMode);
	if (memcmp(szCTLSMode, _CTLS_MODE_0_NO_, strlen(_CTLS_MODE_0_NO_)) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS MODE :None");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else if (memcmp(szCTLSMode, _CTLS_MODE_1_INTERNAL_, strlen(_CTLS_MODE_1_INTERNAL_)) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS MODE :Internal");
			inDISP_LogPrintf(szDebugMsg);
		}

		inRetVal = inCTLS_DevicePooling_Internal(pobTran);
	}
	else if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
			 !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)))
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS MODE :External");
			inDISP_LogPrintf(szDebugMsg);
		}

/* [外接設備設定] */
#ifdef __MUTI_FUCN_TEST_
		inRetVal = inMultiFunc_CallSlave(tbGetPobTranPoint(), _MULTI_POLL_NO_, &gstMultiOb);
		//		inRetVal = inMultiFunc_JsonCallSlave(tbGetPobTranPoint(), _MULTI_POLL_NO_, &gstMultiOb);
#endif
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_DevicePooling_Flow() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inCTLS_DevicePooling_Internal
Date&Time       :2017/9/7 下午 2:41
Describe        :內部感應只能從有沒有Initial成功判斷
*/
int inCTLS_DevicePooling_Internal(TRANSACTION_OBJECT *pobTran)
{
	if (guszCTLSInitiOK != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_SendReadyForSale_Flow
Date&Time       :2017/4/20 下午 1:34
Describe        :
*/
int inCTLS_SendReadyForSale_Flow(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR;
	char szCTLSMode[1 + 1];
	char szDebugMsg[100 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_SendReadyForSale_Flow()START");
	}

	memset(szCTLSMode, 0x00, sizeof(szCTLSMode));
	inGetContactlessReaderMode(szCTLSMode);
	inDISP_LogPrintf("szCTLSMode = %s", szCTLSMode);
	if (memcmp(szCTLSMode, _CTLS_MODE_0_NO_, strlen(_CTLS_MODE_0_NO_)) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS MODE :None");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else if (memcmp(szCTLSMode, _CTLS_MODE_1_INTERNAL_, strlen(_CTLS_MODE_1_INTERNAL_)) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS MODE :Internal");
			inDISP_LogPrintf(szDebugMsg);
		}

		inRetVal = inCTLS_SendReadyForSale_Internal(pobTran);
	}
	else if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
			 !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)))
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS MODE :External");
			inDISP_LogPrintf(szDebugMsg);
		}

/* [外接設備設定] */
#ifdef __MUTI_FUCN_TEST_
		inRetVal = inMultiFunc_CallSlave(pobTran, _MULTI_POLL_NO_, &gstMultiOb); /* 新增外接感應設備 20190816[SAM]*/

		if (inRetVal == VS_SUCCESS)
			inRetVal = inMultiFunc_CallSlave(pobTran, _MULTI_CTLS_NO_, &gstMultiOb); /* 新增外接感應設備 20190816[SAM]*/

			//		inRetVal = inMultiFunc_JsonCallSlave(pobTran, _MULTI_SALE_NO_, &gstMultiOb);
#endif
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_SendReadyForSale_Flow(%d) END !", inRetVal);
		inDISP_LogPrintf("----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inCTLS_SendReadyForSale_Internal
Date&Time       :2017/4/20 下午 1:34
Describe        :
*/
int inCTLS_SendReadyForSale_Internal(TRANSACTION_OBJECT *pobTran)
{
	char szTemplate[20 + 1], szTxnAmount[12 + 1];
	unsigned char szTransaRelatedData[100 + 1];
	unsigned char uszBCD[20 + 1];

	/* SmartPay感應會跑到Call Back Function, pobTran傳不進去 轉存到global */
	memset((char *)&pobEmvTran, 0x00, _TRANSACTION_OBJECT_SIZE_);
	memcpy((char *)&pobEmvTran, (char *)pobTran, _TRANSACTION_OBJECT_SIZE_);

	memset(&szACTData, 0x00, sizeof(EMVCL_ACT_DATA));
	memset(&szRCDataEx, 0x00, sizeof(EMVCL_RC_DATA_EX));
	memset(&szRCDataAnalyze, 0x00, sizeof(EMVCL_RC_DATA_ANALYZE));
	memset(&srCtlsObj, 0x00, sizeof(CTLS_OBJECT));
	memset(szTransaRelatedData, 0x00, sizeof(szTransaRelatedData));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(szTxnAmount, 0x00, sizeof(szTxnAmount));
	// Prepare Input Data
	szACTData.bStart = d_EMVCL_ACT_DATA_START_A;
	szACTData.bTagNum = 0;
	szACTData.usTransactionDataLen = 0;

	// Put 0x9F02 Amount, Authorized (Numeric)
	szTransaRelatedData[szACTData.usTransactionDataLen++] = 0x9F;
	szTransaRelatedData[szACTData.usTransactionDataLen++] = 0x02;
	szTransaRelatedData[szACTData.usTransactionDataLen++] = 0x06;

	sprintf(szTemplate, "%ld00", pobTran->srBRec.lnTxnAmount);
	inFunc_PAD_ASCII(szTemplate, szTemplate, '0', 12, _PAD_RIGHT_FILL_LEFT_);
	memset(uszBCD, 0x00, sizeof(uszBCD));
	inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 6);
	memcpy((char *)&szTxnAmount[0], (char *)&uszBCD[0], 6);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf(szTemplate);
	}

	memcpy(&szTransaRelatedData[szACTData.usTransactionDataLen], &szTxnAmount[0], 6);
	szACTData.usTransactionDataLen += 6;
	szACTData.bTagNum++;

	/* 是否帶交易時間 */
	szTransaRelatedData[szACTData.usTransactionDataLen++] = 0x9A;
	szTransaRelatedData[szACTData.usTransactionDataLen++] = 0x03;
	memset(uszBCD, 0x00, sizeof(uszBCD));
	inFunc_ASCII_to_BCD(&uszBCD[0], &pobTran->srBRec.szDate[2], 3);
	memcpy(&szTransaRelatedData[szACTData.usTransactionDataLen], uszBCD, 3);
	szACTData.usTransactionDataLen += 3;
	szACTData.bTagNum++;

	// Put 0x9C
	szTransaRelatedData[szACTData.usTransactionDataLen++] = 0x9C;
	szTransaRelatedData[szACTData.usTransactionDataLen++] = 0x01;
	szTransaRelatedData[szACTData.usTransactionDataLen++] = 0x00; /* SALE */
	szACTData.bTagNum++;

	szACTData.pbaTransactionData = szTransaRelatedData;

	/* Start */
	if (EMVCL_InitTransactionEx(szACTData.bTagNum, szACTData.pbaTransactionData, szACTData.usTransactionDataLen) != d_EMVCL_NO_ERROR)
		return (VS_ERROR);

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_ReceiveReadyForSales_Flow
Date&Time       :2017/4/20 下午 2:36
Describe        :
*/
int inCTLS_ReceiveReadyForSales_Flow(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR;
	char szCTLSMode[1 + 1];
	unsigned long ulRetVal = VS_ERROR;

	memset(szCTLSMode, 0x00, sizeof(szCTLSMode));
	inGetContactlessReaderMode(szCTLSMode);
	/* 不使用Reader */
	if (memcmp(szCTLSMode, _CTLS_MODE_0_NO_, strlen(_CTLS_MODE_0_NO_)) == 0)
	{
	}
	/* 內建Reader */
	else if (memcmp(szCTLSMode, _CTLS_MODE_1_INTERNAL_, strlen(_CTLS_MODE_1_INTERNAL_)) == 0)
	{
		ulRetVal = ulCTLS_ReceiveReadyForSales_Internal(pobTran);
		if (ulRetVal != d_EMVCL_PENDING)
		{
			inRetVal = VS_SUCCESS;
		}
	}
	/* 外接Reader */
	else if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
			 !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)))
	{
		/* [修改外接感應設備] 2022/11/22 [SAM] */
		inRetVal = inMultiFunc_Host_RecvCtlsData(pobTran);
	}

	return (inRetVal);
}

/*
Function        :ulCTLS_ReceiveReadyForSales_Internal
Date&Time       :2017/4/20 下午 2:36
Describe        :
*/
unsigned long ulCTLS_ReceiveReadyForSales_Internal(TRANSACTION_OBJECT *pobTran)
{
	char szDebugMsg[128 + 1];
	unsigned long ulRetVal;

	/* EMVCL_PerformTransactionEx 執行時會跑call back Function vdCTLS_EVENT_EMVCL_NON_EMV_CARD */
	srCtlsObj.lnSaleRespCode = EMVCL_PerformTransactionEx(&szRCDataEx);

	/* 這裡要傳出是否選擇取消 */
	pobTran->uszMenuSelectCancelBit = pobEmvTran.uszMenuSelectCancelBit;

	/* 這邊就判斷SmartPay感應卡 */
	if (srCtlsObj.lnSaleRespCode == d_EMVCL_NON_EMV_CARD)
	{

		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "d_EMVCL_NON_EMV_CARD");
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
		}

		/* 感應結果已在inFISC_CTLSProcess完成，這邊只要轉存pobTran判斷結果 */
		memset((char *)pobTran, 0x00, _TRANSACTION_OBJECT_SIZE_);
		memcpy((char *)pobTran, (char *)&pobEmvTran, _TRANSACTION_OBJECT_SIZE_);

		if (pobTran->srBRec.inTxnResult == VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("SmartPay CTLS Process SUCCESS");

			/* 跑SmartPay感應流程 */
			/* incode 在vdCTLS_EVENT_EMVCL_NON_EMV_CARD內決定 */
		}
		else
		{
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("SmartPay CTLS Process ERROR");
		}
	}
	else if (srCtlsObj.lnSaleRespCode == d_EMVCL_PENDING)
	{
		/* 等待感應中 */
		//                if (ginDebug == VS_TRUE)
		//                {
		//                        memset(szDebug, 0x00, sizeof(szDebug));
		//                        sprintf(szDebug, "EMVCL_Transaction:0x%X", (unsigned int)srCtlsObj.lnSaleRespCode);
		//                        inDISP_LogPrintf(szDebug);
		//                }
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "EMVCL_Transaction:0x%X", (unsigned int)srCtlsObj.lnSaleRespCode);
			inDISP_LogPrintf(szDebugMsg);
		}
	}

	ulRetVal = srCtlsObj.lnSaleRespCode;

	return (ulRetVal);
}

/*
Function        :inCTLS_CancelTransacton_Flow
Date&Time       :2017/7/14 下午 1:27
Describe        :
*/
int inCTLS_CancelTransacton_Flow()
{
	int inRetVal = VS_ERROR;
	char szCTLSMode[1 + 1];
	char szDebugMsg[100 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_CancelTransacton_Flow()START");
	}

	memset(szCTLSMode, 0x00, sizeof(szCTLSMode));
	inGetContactlessReaderMode(szCTLSMode);
	if (memcmp(szCTLSMode, _CTLS_MODE_0_NO_, strlen(_CTLS_MODE_0_NO_)) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS MODE :None");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else if (memcmp(szCTLSMode, _CTLS_MODE_1_INTERNAL_, strlen(_CTLS_MODE_1_INTERNAL_)) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS MODE :Internal");
			inDISP_LogPrintf(szDebugMsg);
		}

		inRetVal = inCTLS_CancelTransacton_Internal();
	}
	else if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
			 !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)))
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS MODE :External");
			inDISP_LogPrintf(szDebugMsg);
		}
/* [外接設備設定] */
#ifdef __MUTI_FUCN_TEST_
		inRetVal = inMultiFunc_CallSlave(tbGetPobTranPoint(), _MULTI_TRANS_STOP_NO_, &gstMultiOb);
		//		inRetVal = inMultiFunc_JsonCallSlave(tbGetPobTranPoint(), _MULTI_TRANS_STOP_NO_, &gstMultiOb);
#endif
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_CancelTransacton_Flow() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inCTLS_CancelTransacton_Internal
Date&Time       :2017/7/14 下午 1:29
Describe        :
*/
int inCTLS_CancelTransacton_Internal()
{
	char szDebugMsg[100 + 1];
	unsigned short usRetVal = 0;

	usRetVal = EMVCL_CancelTransaction();

	/* Start */
	if (usRetVal != d_EMVCL_NO_ERROR)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS_Cancel_Internal Ret: 0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :ulCTLS_CheckResponseCode_SALE
Date&Time       :2017/7/14 下午 2:54
Describe        :
*/
unsigned long ulCTLS_CheckResponseCode_SALE(TRANSACTION_OBJECT *pobTran)
{
	char szDebugMsg[100 + 1];
	unsigned long ulRetVal;

	switch (srCtlsObj.lnSaleRespCode)
	{
		/* d_EMVCL_RC_DATA 和 d_EMVCL_NON_EMV_CARD 為有收到資料的狀況 */
	case d_EMVCL_RC_DATA:
		pobTran->srBRec.uszContactlessBit = VS_TRUE;
		if (inCTLS_UnPackReadyForSale_Flow(pobTran) != VS_SUCCESS)
		{
			ulRetVal = d_EMVCL_RC_FAILURE;
		}
		else
		{
			ulRetVal = srCtlsObj.lnSaleRespCode;
		}
		break;
	case d_EMVCL_NON_EMV_CARD:
		/* SmartPay會跑這個case */
		pobTran->srBRec.uszContactlessBit = VS_TRUE;
		ulRetVal = srCtlsObj.lnSaleRespCode;
		break;
		/* 感應無效 */
	case d_EMVCL_RC_FAILURE:
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("CTLS Resp Code = d_EMVCL_RC_FAILURE");

		ulRetVal = srCtlsObj.lnSaleRespCode;
		break;
	/* 請問d_EMVCL_RC_FAILURE和d_EMVCL_RC_FALLBACK的差別在哪？
	   [Ans] : 這兩個 Return code 都是表示交易的過程中發生了問題而交易終止了。差別在於 Kernel 是否要求要換另一個介面的交易 (例如 : CL 交易失敗，同一張卡改成 CT 或是 MSR 的介面交易)，當然這個轉換介面的要求是根據各個 payment 的規格而定的。
		   d_EMVCL_RC_FAILURE : 交易中止
		   d_EMVCL_RC_FALLBACK : 交易中止，但嘗試別的介面交易。
	 */
	/* 感應中止，改插卡或刷卡 */
	case d_EMVCL_RC_FALLBACK:
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("CTLS Resp Code = d_EMVCL_RC_FALLBACK");

		ulRetVal = srCtlsObj.lnSaleRespCode;
		break;
	/* 請重試 */
	case d_EMVCL_TRY_AGAIN:
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("CTLS Resp Code = d_EMVCL_TRY_AGAIN");

		ulRetVal = srCtlsObj.lnSaleRespCode;
		break;
	/* 取消的話，基本上會直接跳出迴圈，所以也不會進這裡 */
	case d_EMVCL_TX_CANCEL:
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("CTLS Resp Code = d_EMVCL_TX_CANCEL");

		ulRetVal = srCtlsObj.lnSaleRespCode;
		break;
		/* 多卡重疊 */
	case d_EMVCL_RC_MORE_CARDS:
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("CTLS Resp Code = d_EMVCL_RC_MORE_CARDS");

		ulRetVal = srCtlsObj.lnSaleRespCode;
		break;
	/* Timeout 沒感應到卡，這個Timeout指的是xml檔內設定，目前設定到最大，理論上不會出現此回應 */
	case d_EMVCL_RC_NO_CARD: /* Timeout 沒感應到卡 */
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("CTLS Resp Code = d_EMVCL_RC_NO_CARD");

		ulRetVal = srCtlsObj.lnSaleRespCode;
		break;
	case d_EMVCL_NO_ERROR:
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("CTLS Resp Code = d_EMVCL_NO_ERROR");

		ulRetVal = srCtlsObj.lnSaleRespCode;
		break;
	case d_EMVCL_PENDING:
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("CTLS Resp Code = d_EMVCL_PENDING");

		ulRetVal = srCtlsObj.lnSaleRespCode;
		break;
	/* 請問d_EMVCL_RC_DEK_SIGNAL這個回應碼是什麼意思？
	   [Ans] : 收到 d_EMVCL_RC_DEK_SIGNAL 表示交易中間 kernel 有訊息要帶出外面給 Application，交易尚未結束，仍須等待 EMVCL_PerformTransactionEx 給出真正的交易結果。
	 */
	case d_EMVCL_RC_DEK_SIGNAL:
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("CTLS Resp Code = d_EMVCL_RC_DEK_SIGNAL");
		}
		if (ginDisplayDebug == VS_TRUE)
		{
			inDISP_LOGDisplay("CTLS d_EMVCL_RC_DEK_SIGNAL", _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
		}

		ulRetVal = srCtlsObj.lnSaleRespCode;
		break;
	/* Two Tap流程 */
	case d_EMVCL_RC_SEE_PHONE:
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("CTLS Resp Code = d_EMVCL_RC_SEE_PHONE");
		/* 請輸密碼或指紋 並再感應一次 */
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		inDISP_PutGraphic(_CTLS_TWO_TAP_, 0, _COORDINATE_Y_LINE_8_7_);
		pobTran->uszTwoTapBit = VS_TRUE;

		ulRetVal = srCtlsObj.lnSaleRespCode;
		break;
	default:
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS Res: %08lX", srCtlsObj.lnSaleRespCode);
			inDISP_LogPrintf(szDebugMsg);
		}
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS Res: %08lX", srCtlsObj.lnSaleRespCode);
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
		}

		ulRetVal = d_EMVCL_RC_FAILURE;
		break;
	}

	return (ulRetVal);
}

/*
Function        :inCTLS_UnPackReadyForSale_Flow
Date&Time       :2017/4/21 上午 9:37
Describe        :
*/
int inCTLS_UnPackReadyForSale_Flow(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR;
	char szCTLSMode[1 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_UnPackReadyForSale_Flow()START");
	}

	memset(szCTLSMode, 0x00, sizeof(szCTLSMode));
	inGetContactlessReaderMode(szCTLSMode);
	/* 不使用Reader */
	if (memcmp(szCTLSMode, _CTLS_MODE_0_NO_, strlen(_CTLS_MODE_0_NO_)) == 0)
	{
	}
	/* 內建Reader */
	else if (memcmp(szCTLSMode, _CTLS_MODE_1_INTERNAL_, strlen(_CTLS_MODE_1_INTERNAL_)) == 0)
	{
		inRetVal = inCTLS_UnPackReadyForSale_Internal(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			inCTLS_CancelTransacton_Flow();
		}
	}
	/* 外接Reader */
	else if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
			 !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)))
	{
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_UnPackReadyForSale_Flow()END");
	}

	return (inRetVal);
}

/*
Function        :inCTLS_UnPackReadyForSale
Date&Time       :2017/4/21 上午 9:37
Describe        :
*/
int inCTLS_UnPackReadyForSale_Internal(TRANSACTION_OBJECT *pobTran)
{
	int i = 0, inRetVal = -1;
	char szDebug[1024 + 1] = {0};
	char szASCII[64 + 1] = {0};
	char szCUPFunctionEnable[2 + 1] = {0};
	char szMACEnable[2 + 1] = {0};

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_UnPackReadyForSale_START");
	}
	// Parse transaction response data
	// Parse Scheme ID

	srCtlsObj.uszSchemeID[0] = szRCDataEx.bSID;

	if (ginDebug == VS_TRUE)
	{
		memset(szDebug, 0x00, sizeof(szDebug));
		sprintf(szDebug, "Scheme ID = 0x%02X", srCtlsObj.uszSchemeID[0]);

		switch (srCtlsObj.uszSchemeID[0])
		{
		case d_EMVCL_SID_VISA_OLD_US:
			inDISP_LogPrintf("d_EMVCL_SID_VISA_OLD_US");
			break;
		case d_EMVCL_SID_VISA_WAVE_2:
			inDISP_LogPrintf("d_EMVCL_SID_VISA_WAVE_2");
			break;
		case d_EMVCL_SID_VISA_WAVE_MSD:
			inDISP_LogPrintf("d_EMVCL_SID_VISA_WAVE_MSD");
			break;
		case d_EMVCL_SID_VISA_WAVE_QVSDC:
			inDISP_LogPrintf("d_EMVCL_SID_VISA_WAVE_QVSDC");
			break;
		case d_EMVCL_SID_PAYPASS_MAG_STRIPE:
			inDISP_LogPrintf("d_EMVCL_SID_PAYPASS_MAG_STRIPE");
			break;
		case d_EMVCL_SID_PAYPASS_MCHIP:
			inDISP_LogPrintf("d_EMVCL_SID_PAYPASS_MCHIP");
			break;
		case d_EMVCL_SID_JCB_WAVE_2:
			inDISP_LogPrintf("d_EMVCL_SID_JCB_WAVE_2");
			break;
		case d_EMVCL_SID_JCB_WAVE_QVSDC:
			inDISP_LogPrintf("d_EMVCL_SID_JCB_WAVE_QVSDC");
			break;
		case d_EMVCL_SID_JCB_MSD:
			inDISP_LogPrintf("d_EMVCL_SID_JCB_MSD");
			break;
		case d_EMVCL_SID_JCB_LEGACY:
			inDISP_LogPrintf("d_EMVCL_SID_JCB_LEGACY");
			break;
		case d_EMVCL_SID_JCB_EMV:
			inDISP_LogPrintf("d_EMVCL_SID_JCB_EMV");
			break;
		case d_EMVCL_SID_CUP_QPBOC:
			inDISP_LogPrintf("d_EMVCL_SID_CUP_QPBOC");
			break;
		case SCHEME_ID_90_SMARTPAY:
			inDISP_LogPrintf("SCHEME_ID_90_SMARTPAY");
			break;
		case d_EMVCL_SID_AE_EMV:
			inDISP_LogPrintf("d_EMVCL_SID_AE_EMV");
			break;
		case d_EMVCL_SID_AE_MAG_STRIPE:
			inDISP_LogPrintf("d_EMVCL_SID_AE_MAG_STRIPE");
			break;
		case d_EMVCL_SID_DISCOVER:
			inDISP_LogPrintf("d_EMVCL_SID_DISCOVER");
			break;
		case d_EMVCL_SID_DISCOVER_DPAS:
			inDISP_LogPrintf("d_EMVCL_SID_DISCOVER_DPAS");
			break;
		case d_EMVCL_SID_INTERAC_FLASH:
			inDISP_LogPrintf("d_EMVCL_SID_INTERAC_FLASH");
			break;
		case d_EMVCL_SID_MEPS_MCCS:
			inDISP_LogPrintf("d_EMVCL_SID_MEPS_MCCS");
			break;
		default:
			inDISP_LogPrintf("???? Card");
			break;
		}
	}

	switch (srCtlsObj.uszSchemeID[0])
	{
	/* 原廠分一大堆Scheme ID 不支援的直接擋下 支援的存成要用的 */
	case d_EMVCL_SID_VISA_OLD_US:
	case d_EMVCL_SID_VISA_WAVE_2:
	case d_EMVCL_SID_VISA_WAVE_MSD:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_16_WAVE1;
		break;
	case d_EMVCL_SID_VISA_WAVE_QVSDC:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_17_WAVE3;
		break;
	case d_EMVCL_SID_PAYPASS_MAG_STRIPE:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_20_PAYPASS_MAG_STRIPE;
		break;
	case d_EMVCL_SID_PAYPASS_MCHIP:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_21_PAYPASS_MCHIP;
		break;
	case d_EMVCL_SID_JCB_WAVE_2:
	case d_EMVCL_SID_JCB_WAVE_QVSDC:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_61_JSPEEDY;
		break;
	case d_EMVCL_SID_JCB_MSD:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_64_NEWJSPEEDY_MSD;
		break;
	case d_EMVCL_SID_JCB_LEGACY:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_65_NEWJSPEEDY_LEGACY;
		break;
	case d_EMVCL_SID_JCB_EMV:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_63_NEWJSPEEDY_EMV;
		break;
	case d_EMVCL_SID_AE_EMV:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_50_EXPRESSSPAY;
		break;
	case d_EMVCL_SID_AE_MAG_STRIPE:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE;
		break;
	case d_EMVCL_SID_DISCOVER_DPAS:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_42_DPAS;
		break;
	case d_EMVCL_SID_CUP_QPBOC:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_91_QUICKPASS;

		/* 若銀聯功能沒開，擋掉 */
		/* 若EDC的CUPFunctionEnable 和 MACEnable 未開，顯示此功能以關閉 */
		memset(szCUPFunctionEnable, 0x00, sizeof(szCUPFunctionEnable));
		inGetCUPFuncEnable(szCUPFunctionEnable);
		memset(szMACEnable, 0x00, sizeof(szMACEnable));
		inGetMACEnable(szMACEnable);

		/* 沒開CUP */
		if ((memcmp(&szCUPFunctionEnable[0], "Y", 1) != 0) || memcmp(szMACEnable, "Y", 1) != 0)
		{
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);					  /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜銀聯一般交易＞ */
			/* 此功能已關閉 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;

			return (VS_ERROR);
		}

		/* IDLE進入，一開始預設是_SALE_ */
		if (pobTran->inTransactionCode == _SALE_)
		{
			pobTran->inTransactionCode = _CUP_SALE_;
			pobTran->srBRec.inCode = pobTran->inTransactionCode;
			pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
		}
		else if (pobTran->inTransactionCode == _PRE_AUTH_)
		{
			pobTran->inTransactionCode = _CUP_PRE_AUTH_;
			pobTran->srBRec.inCode = pobTran->inTransactionCode;
			pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
		}
		/* NCCC CUP同一個HOST */
		pobTran->srBRec.inHDTIndex = 0;
		if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" CLST Unpack Ready Sale Inter Load HDT[%d] *Error* Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
		}
		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" CLST Unpack Ready Sale Inter Load HDPT[%d] *Error* Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
		}

		pobTran->srBRec.uszCUPTransBit = VS_TRUE;
/* 因為不需要在這邊換KEY,所以拿掉 20190328 [SAM] */
#if 0
			/* 確認是銀聯卡，檢查是否已做安全認證 */
			/* 有開CUP且MACEnable有開但安全認證沒過，不能執行CUP交易 */
			if (inKMS_CheckKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_MAC_) != VS_SUCCESS)
			{
				if (inNCCC_Func_CUP_PowerOn_LogOn(pobTran) != VS_SUCCESS)
				{
					/* 安全認證失敗 */
					return (VS_ERROR);
				}
			}
#endif
		break;

		/* 目前不支援 */
	case d_EMVCL_SID_DISCOVER:
	case d_EMVCL_SID_INTERAC_FLASH:
	case d_EMVCL_SID_MEPS_MCCS:
	default:
		/* 不支援的scheme ID直接擋掉，之後組感應Tag也會錯誤 */
		if (ginDebug == VS_TRUE)
		{
			memset(szDebug, 0x00, sizeof(szDebug));
			sprintf(szDebug, "CardType No Def. SchemeID = 0x%02X", srCtlsObj.uszSchemeID[0]);
			inDISP_LogPrintf(szDebug);
		}

		return (VS_ERROR);
	}

	/* 將CTLSobj的schemID轉存到PobTran */
	pobTran->srBRec.uszWAVESchemeID = srCtlsObj.uszSchemeID[0];

	if (ginDebug == VS_TRUE)
	{
		memset(szDebug, 0x00, sizeof(szDebug));
		sprintf(szDebug, "uszSchemeID : %02X", pobTran->srBRec.uszWAVESchemeID);
		inDISP_LogPrintf(szDebug);
	}

	if ((pobTran->srBRec.uszWAVESchemeID != SCHEME_ID_17_WAVE3) && (pobTran->srBRec.uszWAVESchemeID != SCHEME_ID_16_WAVE1))
	{
		memset(szASCII, 0x00, sizeof(szASCII));
		inFunc_BCD_to_ASCII(&szASCII[0], &szRCDataEx.baTrack2Data[0], szRCDataEx.bTrack2Len * 2);
		szRCDataEx.bTrack2Len = szRCDataEx.bTrack2Len * 2;

		memset(szRCDataEx.baTrack2Data, 0x00, sizeof(szRCDataEx.baTrack2Data));
		memcpy(&szRCDataEx.baTrack2Data[0], &szASCII[0], szRCDataEx.bTrack2Len);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("Track 1 Data :");
		memset(szDebug, 0x00, sizeof(szDebug));
		memcpy(szDebug, (char *)szRCDataEx.baTrack1Data, szRCDataEx.bTrack1Len);
		inDISP_LogPrintf(szDebug);

		memset(szDebug, 0x00, sizeof(szDebug));
		memset(szASCII, 0x00, sizeof(szASCII));
		inFunc_BCD_to_ASCII(szASCII, &szRCDataEx.baTrack1Data[szRCDataEx.bTrack1Len - 1], 1);
		sprintf(szDebug, "LRC: %s", szASCII);
		inDISP_LogPrintf(szDebug);

		memset(szDebug, 0x00, sizeof(szDebug));
		sprintf(szDebug, "Track 1 Len = %d", szRCDataEx.bTrack1Len);
		inDISP_LogPrintf(szDebug);

		inDISP_LogPrintf("Track 2 Data :");
		memset(szDebug, 0x00, sizeof(szDebug));
		memcpy(szDebug, (char *)szRCDataEx.baTrack2Data, szRCDataEx.bTrack2Len - 1);
		inDISP_LogPrintf(szDebug);

		memset(szDebug, 0x00, sizeof(szDebug));
		memset(szASCII, 0x00, sizeof(szASCII));
		inFunc_BCD_to_ASCII(szASCII, &szRCDataEx.baTrack2Data[szRCDataEx.bTrack2Len - 1], 1);
		sprintf(szDebug, "LRC: %s", szASCII);
		inDISP_LogPrintf(szDebug);

		memset(szDebug, 0x00, sizeof(szDebug));
		sprintf(szDebug, "Track 2 Len = %d", szRCDataEx.bTrack2Len);
		inDISP_LogPrintf(szDebug);
	}

	memset(pobTran->szTrack1, 0x00, sizeof(pobTran->szTrack1));
	memset(pobTran->szTrack2, 0x00, sizeof(pobTran->szTrack2));

	/* 將讀到的Track123存到pobTran */
	pobTran->shTrack1Len = szRCDataEx.bTrack1Len;
	pobTran->shTrack2Len = szRCDataEx.bTrack2Len;

	memcpy(pobTran->szTrack1, szRCDataEx.baTrack1Data, szRCDataEx.bTrack1Len);

	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3 ||
		pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1)
	{
		/* 因為Track2第0個byte為";"，因此略過 */
		memcpy(pobTran->szTrack2, &szRCDataEx.baTrack2Data[1], szRCDataEx.bTrack2Len);
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
	{
		/* AE範例：3B3337343234353030313734313030373D3231303337303231353034313233343530303030303F */
		pobTran->shTrack2Len = (szRCDataEx.bTrack2Len - 2) / 2;
		inFunc_ASCII_to_BCD((unsigned char *)pobTran->szTrack2, (char *)&szRCDataEx.baTrack2Data[2], pobTran->shTrack2Len);
	}
	else
	{
		memcpy(pobTran->szTrack2, &szRCDataEx.baTrack2Data[0], szRCDataEx.bTrack2Len);
	}

	/* 這邊要修改檢查Track2 */
	for (i = 0; i < strlen(pobTran->szTrack2); i++)
	{
		/* F為結尾須刪除，<ETX>:"?" 必須去掉 */
		if (pobTran->szTrack2[i] == 'F' || pobTran->szTrack2[i] == '?')
		{
			pobTran->szTrack2[i] = 0;
			break;
		}

		if (pobTran->szTrack2[i] == 'D')
			pobTran->szTrack2[i] = '=';
	}

	/* 取卡號有效期 */
	if (strlen(pobTran->szTrack2) > 0)
	{
		inRetVal = inCARD_unPackCard(pobTran);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	else
	{
		return (VS_ERROR);
	}

	/* 組電文的EMV Data */
	inRetVal = inCTLS_ProcessChipData(pobTran);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 例外狀況 Tag檢核 感應限額等等 */
	inRetVal = inCTLS_ExceptionCheck(pobTran);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_UnPackReadyForSale_END");
	}

	return (VS_SUCCESS);
}

int inCTLS_ProcessChipData(TRANSACTION_OBJECT *pobTran)
{
	int inTempLen;
	char szTempBuf[512] = {0};
	int inCnt = 0, inDataSizes, i = 0;
	char szASCII[2048 + 1];
	char szDebugMsg[100 + 1];
	char szTagName[6 + 1];
	char szSerialNumber[16 + 1];
	unsigned short ushTagLen;
	unsigned char uszCTLSData[1024 + 1], uszTagData[128 + 1];
	unsigned char uszTag5F2A = VS_FALSE, uszTag9F1A = VS_FALSE;
	unsigned char uszChipFlag = VS_FALSE, uszAdditionalFlag = VS_FALSE;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_ProcessChipData_START");
		inDISP_LogPrintf("=================================");
	}

	if (szRCDataEx.usChipDataLen > 0)
		uszChipFlag = VS_TRUE;

	if (szRCDataEx.usAdditionalDataLen > 0)
		uszAdditionalFlag = VS_TRUE;

	memset(uszCTLSData, 0x00, sizeof(uszCTLSData));
	memcpy(uszCTLSData, szRCDataEx.baChipData, szRCDataEx.usChipDataLen);
	inDataSizes = szRCDataEx.usChipDataLen;

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "CTLS ChipDataLen = %d", inDataSizes);
		inDISP_LogPrintf(szDebugMsg);

		memset(szASCII, 0x00, sizeof(szASCII));
		inFunc_BCD_to_ASCII(&szASCII[0], &uszCTLSData[0], inDataSizes);

		/* 把整個chipData印出來 */
		inDISP_LogPrintf("0123456789012345678901234567890123456789");
		inDISP_LogPrintf("========================================");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));

		for (inCnt = 0; inCnt < (inDataSizes * 2); inCnt++)
		{
			memcpy(&szDebugMsg[i], &szASCII[inCnt], 1);

			i++;

			if (i == 40)
			{
				inDISP_LogPrintf(szDebugMsg);
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				i = 0;
			}
		}

		inDISP_LogPrintf(szDebugMsg);
		inDISP_LogPrintf("----------------------------------------");

		inCnt = 0;
		i = 0;
	}

	if (ginDebug == VS_TRUE)
	{
		memset(uszCTLSData, 0x00, sizeof(uszCTLSData));
		memcpy(uszCTLSData, szRCDataEx.baAdditionalData, szRCDataEx.usAdditionalDataLen);
		inDataSizes = szRCDataEx.usAdditionalDataLen;

		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "CTLS AdditionalDataLen = %d", inDataSizes);
		inDISP_LogPrintf(szDebugMsg);
		inDISP_LogPrintf("========================================");

		memset(szASCII, 0x00, sizeof(szASCII));
		inFunc_BCD_to_ASCII(&szASCII[0], &uszCTLSData[0], inDataSizes);

		/* 把整個chipData印出來 */
		inDISP_LogPrintf("0123456789012345678901234567890123456789");
		inDISP_LogPrintf("----------------------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));

		for (inCnt = 0; inCnt < (inDataSizes * 2); inCnt++)
		{
			memcpy(&szDebugMsg[i], &szASCII[inCnt], 1);

			i++;

			if (i == 40)
			{
				inDISP_LogPrintf(szDebugMsg);
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				i = 0;
			}
		}

		inDISP_LogPrintf(szDebugMsg);
		inDISP_LogPrintf("----------------------------------------");

		inCnt = 0;
		i = 0;
	}

	/* 有些tag不知道放chip data還是additional data 跑迴圈存EMV資料 */
	do
	{
		if (uszChipFlag == VS_TRUE)
		{
			memset(uszCTLSData, 0x00, sizeof(uszCTLSData));
			memcpy(uszCTLSData, szRCDataEx.baChipData, szRCDataEx.usChipDataLen);
			inDataSizes = szRCDataEx.usChipDataLen;
			i = 1; /* 第一圈 */
		}

		for (; inCnt < inDataSizes;)
		{
			if (uszCTLSData[inCnt] == 0x50)
			{
				inCnt += 1;
				pobTran->srEMVRec.in50_APLabelLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz50_APLabel, 0x00, sizeof(pobTran->srEMVRec.usz50_APLabel));
				memcpy((char *)&pobTran->srEMVRec.usz50_APLabel[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in50_APLabelLen);
				inCnt += pobTran->srEMVRec.in50_APLabelLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					/* AP LABEL本來就是Ascii 不用轉 */
					memset(szASCII, 0x00, sizeof(szASCII));
					sprintf(szASCII, "%s", pobTran->srEMVRec.usz50_APLabel);

					inCTLS_ISOFormatDebug_DISP("50", pobTran->srEMVRec.in50_APLabelLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					/* AP LABEL本來就是Ascii 不用轉 */
					memset(szASCII, 0x00, sizeof(szASCII));
					sprintf(szASCII, "%s", pobTran->srEMVRec.usz50_APLabel);

					inCTLS_ISOFormatDebug_PRINT("50", pobTran->srEMVRec.in50_APLabelLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x55)
			{
				inCnt += 1;
				ushTagLen = uszCTLSData[inCnt++];
				memset(uszTagData, 0x00, sizeof(uszTagData));
				memcpy((char *)&uszTagData[0], &uszCTLSData[inCnt], ushTagLen);
				inCnt += ushTagLen;

				if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3 && uszTagData[inCnt] == 0x01)
					pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* VISA Paywave3 免簽名條件 */
				else
					pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* VISA Paywave3 免簽名條件 */

				pobTran->uszPayWave3Tag55Bit = VS_TRUE;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);

					inCTLS_ISOFormatDebug_DISP("55", ushTagLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);

					inCTLS_ISOFormatDebug_PRINT("55", ushTagLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x57)
			{
				inCnt += 1;
				ushTagLen = uszCTLSData[inCnt++];
				memset(uszTagData, 0x00, sizeof(uszTagData));
				memcpy((char *)&uszTagData[0], &uszCTLSData[inCnt], ushTagLen);
				inCnt += ushTagLen;

				/* 組TAG_57 */
				memset(pobTran->usz57_Track2, 0x00, sizeof(pobTran->usz57_Track2));
				memcpy(&pobTran->usz57_Track2[0], (char *)&uszTagData[0], ushTagLen);
				pobTran->in57_Track2Len = ushTagLen;

				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					inCTLS_ISOFormatDebug_DISP("57", ushTagLen, "-----");
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					inCTLS_ISOFormatDebug_PRINT("57", ushTagLen, "-----");
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x5A)
			{
				inCnt += 1;
				pobTran->srEMVRec.in5A_ApplPanLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz5A_ApplPan, 0x00, sizeof(pobTran->srEMVRec.usz5A_ApplPan));
				memcpy((char *)&pobTran->srEMVRec.usz5A_ApplPan[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in5A_ApplPanLen);
				inCnt += pobTran->srEMVRec.in5A_ApplPanLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz5A_ApplPan[0], pobTran->srEMVRec.in5A_ApplPanLen);

					inCTLS_ISOFormatDebug_DISP("5A", pobTran->srEMVRec.in5A_ApplPanLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz5A_ApplPan[0], pobTran->srEMVRec.in5A_ApplPanLen);

					inCTLS_ISOFormatDebug_PRINT("5A", pobTran->srEMVRec.in5A_ApplPanLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x5F && uszCTLSData[inCnt + 1] == 0x20)
			{
				inCnt += 2;
				ushTagLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz5F20_CardholderName, 0x00, sizeof(pobTran->srEMVRec.usz5F20_CardholderName));
				memset(uszTagData, 0x00, sizeof(uszTagData));
				memcpy(&pobTran->srEMVRec.usz5F20_CardholderName[0], &uszCTLSData[inCnt], ushTagLen);
				inCnt += ushTagLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					inCTLS_ISOFormatDebug_DISP("5F20", ushTagLen, (char *)pobTran->srEMVRec.usz5F20_CardholderName);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					inCTLS_ISOFormatDebug_PRINT("5F20", ushTagLen, (char *)pobTran->srEMVRec.usz5F20_CardholderName);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x5F && uszCTLSData[inCnt + 1] == 0x24)
			{
				inCnt += 2;
				pobTran->srEMVRec.in5F24_ExpireDateLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz5F24_ExpireDate, 0x00, sizeof(pobTran->srEMVRec.usz5F24_ExpireDate));
				memcpy((char *)&pobTran->srEMVRec.usz5F24_ExpireDate[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in5F24_ExpireDateLen);
				inCnt += pobTran->srEMVRec.in5F24_ExpireDateLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					inCTLS_ISOFormatDebug_DISP("5F24", pobTran->srEMVRec.in5F24_ExpireDateLen, "-----");
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					inCTLS_ISOFormatDebug_PRINT("5F24", pobTran->srEMVRec.in5F24_ExpireDateLen, "-----");
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x5F && uszCTLSData[inCnt + 1] == 0x2A)
			{
				inCnt += 2;
				pobTran->srEMVRec.in5F2A_TransCurrCodeLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz5F2A_TransCurrCode, 0x00, sizeof(pobTran->srEMVRec.usz5F2A_TransCurrCode));
				memcpy((char *)&pobTran->srEMVRec.usz5F2A_TransCurrCode[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);
				inCnt += pobTran->srEMVRec.in5F2A_TransCurrCodeLen;

				if (pobTran->srEMVRec.usz5F2A_TransCurrCode[0] == 0x09 && pobTran->srEMVRec.usz5F2A_TransCurrCode[1] == 0x01)
					uszTag5F2A = VS_TRUE;

				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz5F2A_TransCurrCode[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);

					inCTLS_ISOFormatDebug_DISP("5F2A", pobTran->srEMVRec.in5F2A_TransCurrCodeLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz5F2A_TransCurrCode[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);

					inCTLS_ISOFormatDebug_PRINT("5F2A", pobTran->srEMVRec.in5F2A_TransCurrCodeLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x5F && uszCTLSData[inCnt + 1] == 0x34)
			{
				inCnt += 2;
				pobTran->srEMVRec.in5F34_ApplPanSeqnumLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz5F34_ApplPanSeqnum, 0x00, sizeof(pobTran->srEMVRec.usz5F34_ApplPanSeqnum));
				memcpy((char *)&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
				inCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);

					inCTLS_ISOFormatDebug_DISP("5F34", pobTran->srEMVRec.in5F34_ApplPanSeqnumLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);

					inCTLS_ISOFormatDebug_PRINT("5F34", pobTran->srEMVRec.in5F34_ApplPanSeqnumLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x82)
			{
				inCnt += 1;
				pobTran->srEMVRec.in82_AIPLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz82_AIP, 0x00, sizeof(pobTran->srEMVRec.usz82_AIP));
				memcpy((char *)&pobTran->srEMVRec.usz82_AIP[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in82_AIPLen);
				inCnt += pobTran->srEMVRec.in82_AIPLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz82_AIP[0], pobTran->srEMVRec.in82_AIPLen);

					inCTLS_ISOFormatDebug_DISP("82", pobTran->srEMVRec.in82_AIPLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz82_AIP[0], pobTran->srEMVRec.in82_AIPLen);

					inCTLS_ISOFormatDebug_PRINT("82", pobTran->srEMVRec.in82_AIPLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x84)
			{
				inCnt += 1;
				pobTran->srEMVRec.in84_DFNameLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz84_DF_NAME, 0x00, sizeof(pobTran->srEMVRec.usz84_DF_NAME));
				memcpy((char *)&pobTran->srEMVRec.usz84_DF_NAME[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in84_DFNameLen);

				/* 銀聯需要AID */
				memset(pobTran->srBRec.szCUP_EMVAID, 0x00, sizeof(pobTran->srBRec.szCUP_EMVAID));
				inFunc_BCD_to_ASCII(pobTran->srBRec.szCUP_EMVAID, pobTran->srEMVRec.usz84_DF_NAME, pobTran->srEMVRec.in84_DFNameLen);

				inCnt += pobTran->srEMVRec.in84_DFNameLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);

					inCTLS_ISOFormatDebug_DISP("84", pobTran->srEMVRec.in84_DFNameLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);

					inCTLS_ISOFormatDebug_PRINT("84", pobTran->srEMVRec.in84_DFNameLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x8A)
			{
				inCnt += 1;
				pobTran->srEMVRec.in8A_AuthRespCodeLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz8A_AuthRespCode, 0x00, sizeof(pobTran->srEMVRec.usz8A_AuthRespCode));
				memcpy((char *)&pobTran->srEMVRec.usz8A_AuthRespCode[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in8A_AuthRespCodeLen);
				inCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);

					inCTLS_ISOFormatDebug_DISP("8A", pobTran->srEMVRec.in8A_AuthRespCodeLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);

					inCTLS_ISOFormatDebug_PRINT("8A", pobTran->srEMVRec.in8A_AuthRespCodeLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x95)
			{
				inCnt += 1;
				pobTran->srEMVRec.in95_TVRLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz95_TVR, 0x00, sizeof(pobTran->srEMVRec.usz95_TVR));
				memcpy((char *)&pobTran->srEMVRec.usz95_TVR[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in95_TVRLen);
				inCnt += pobTran->srEMVRec.in95_TVRLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz95_TVR[0], pobTran->srEMVRec.in95_TVRLen);

					inCTLS_ISOFormatDebug_DISP("95", pobTran->srEMVRec.in95_TVRLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz95_TVR[0], pobTran->srEMVRec.in95_TVRLen);

					inCTLS_ISOFormatDebug_PRINT("95", pobTran->srEMVRec.in95_TVRLen, szASCII);
				}
				continue;
			}

			/* Tag 99 For CUP DEBIT */
			if (uszCTLSData[inCnt] == 0x99)
			{
				inCnt += 1;
				ushTagLen = uszCTLSData[inCnt++];
				memset(uszTagData, 0x00, sizeof(uszTagData));
				memcpy((char *)&uszTagData[0], &uszCTLSData[inCnt], ushTagLen);
				inCnt += ushTagLen;

				if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS && uszTagData[inCnt] == 0x00)
				{
					pobTran->uszQuickPassTag99 = VS_TRUE;
				}

				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);

					inCTLS_ISOFormatDebug_DISP("99", ushTagLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);

					inCTLS_ISOFormatDebug_PRINT("99", ushTagLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9A)
			{
				inCnt += 1;
				pobTran->srEMVRec.in9A_TranDateLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz9A_TranDate, 0x00, sizeof(pobTran->srEMVRec.usz9A_TranDate));
				memcpy((char *)&pobTran->srEMVRec.usz9A_TranDate[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9A_TranDateLen);
				inCnt += pobTran->srEMVRec.in9A_TranDateLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);

					inCTLS_ISOFormatDebug_DISP("9A", pobTran->srEMVRec.in9A_TranDateLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);

					inCTLS_ISOFormatDebug_PRINT("9A", pobTran->srEMVRec.in9A_TranDateLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9B)
			{
				inCnt += 1;
				pobTran->srEMVRec.in9B_TSILen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz9B_TSI, 0x00, sizeof(pobTran->srEMVRec.usz9B_TSI));
				memcpy((char *)&pobTran->srEMVRec.usz9B_TSI[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9B_TSILen);
				inCnt += pobTran->srEMVRec.in9B_TSILen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);

					inCTLS_ISOFormatDebug_DISP("9B", pobTran->srEMVRec.in9B_TSILen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);

					inCTLS_ISOFormatDebug_PRINT("9B", pobTran->srEMVRec.in9B_TSILen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9C)
			{
				inCnt += 1;
				pobTran->srEMVRec.in9C_TranTypeLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz9C_TranType, 0x00, sizeof(pobTran->srEMVRec.usz9C_TranType));
				memcpy((char *)&pobTran->srEMVRec.usz9C_TranType[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9C_TranTypeLen);
				inCnt += pobTran->srEMVRec.in9C_TranTypeLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9C_TranType[0], pobTran->srEMVRec.in9C_TranTypeLen);

					inCTLS_ISOFormatDebug_DISP("9C", pobTran->srEMVRec.in9C_TranTypeLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9C_TranType[0], pobTran->srEMVRec.in9C_TranTypeLen);

					inCTLS_ISOFormatDebug_PRINT("9C", pobTran->srEMVRec.in9C_TranTypeLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x02)
			{
				inCnt += 2;
				pobTran->srEMVRec.in9F02_AmtAuthNumLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz9F02_AmtAuthNum, 0x00, sizeof(pobTran->srEMVRec.usz9F02_AmtAuthNum));
				memcpy((char *)&pobTran->srEMVRec.usz9F02_AmtAuthNum[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F02_AmtAuthNumLen);
				inCnt += pobTran->srEMVRec.in9F02_AmtAuthNumLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);

					inCTLS_ISOFormatDebug_DISP("9F02", pobTran->srEMVRec.in9F02_AmtAuthNumLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);

					inCTLS_ISOFormatDebug_PRINT("9F02", pobTran->srEMVRec.in9F02_AmtAuthNumLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x03)
			{
				inCnt += 2;
				pobTran->srEMVRec.in9F03_AmtOtherNumLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz9F03_AmtOtherNum, 0x00, sizeof(pobTran->srEMVRec.usz9F03_AmtOtherNum));
				memcpy((char *)&pobTran->srEMVRec.usz9F03_AmtOtherNum[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F03_AmtOtherNumLen);
				inCnt += pobTran->srEMVRec.in9F03_AmtOtherNumLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F03_AmtOtherNum[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);

					inCTLS_ISOFormatDebug_DISP("9F03", pobTran->srEMVRec.in9F03_AmtOtherNumLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F03_AmtOtherNum[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);

					inCTLS_ISOFormatDebug_PRINT("9F03", pobTran->srEMVRec.in9F03_AmtOtherNumLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x08)
			{
				inCnt += 2;
				ushTagLen = uszCTLSData[inCnt++];
				memset(uszTagData, 0x00, sizeof(uszTagData));
				memcpy((char *)&uszTagData[0], &uszCTLSData[inCnt], ushTagLen);
				inCnt += ushTagLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);

					inCTLS_ISOFormatDebug_DISP("9F08", ushTagLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);

					inCTLS_ISOFormatDebug_PRINT("9F08", ushTagLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x09)
			{
				inCnt += 2;
				ushTagLen = uszCTLSData[inCnt++];
				memset(uszTagData, 0x00, sizeof(uszTagData));
				memcpy((char *)&uszTagData[0], &uszCTLSData[inCnt], ushTagLen);
				inCnt += ushTagLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);

					inCTLS_ISOFormatDebug_DISP("9F09", ushTagLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);

					inCTLS_ISOFormatDebug_PRINT("9F09", ushTagLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x10)
			{
				inCnt += 2;
				pobTran->srEMVRec.in9F10_IssuerAppDataLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz9F10_IssuerAppData, 0x00, sizeof(pobTran->srEMVRec.usz9F10_IssuerAppData));
				memcpy((char *)&pobTran->srEMVRec.usz9F10_IssuerAppData[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F10_IssuerAppDataLen);
				inCnt += pobTran->srEMVRec.in9F10_IssuerAppDataLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F10_IssuerAppData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);

					inCTLS_ISOFormatDebug_DISP("9F10", pobTran->srEMVRec.in9F10_IssuerAppDataLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F10_IssuerAppData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);

					inCTLS_ISOFormatDebug_PRINT("9F10", pobTran->srEMVRec.in9F10_IssuerAppDataLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x1A)
			{
				inCnt += 2;
				pobTran->srEMVRec.in9F1A_TermCountryCodeLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz9F1A_TermCountryCode, 0x00, sizeof(pobTran->srEMVRec.usz9F1A_TermCountryCode));
				memcpy((char *)&pobTran->srEMVRec.usz9F1A_TermCountryCode[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);
				inCnt += pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
				uszTag9F1A = VS_TRUE;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F1A_TermCountryCode[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);

					inCTLS_ISOFormatDebug_DISP("9F1A", pobTran->srEMVRec.in9F1A_TermCountryCodeLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F1A_TermCountryCode[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);

					inCTLS_ISOFormatDebug_PRINT("9F1A", pobTran->srEMVRec.in9F1A_TermCountryCodeLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x1E)
			{
				inCnt += 2;
				pobTran->srEMVRec.in9F1E_IFDNumLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz9F1E_IFDNum, 0x00, sizeof(pobTran->srEMVRec.usz9F1E_IFDNum));
				memcpy((char *)&pobTran->srEMVRec.usz9F1E_IFDNum[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F1E_IFDNumLen);
				inCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);

					inCTLS_ISOFormatDebug_DISP("9F1E", pobTran->srEMVRec.in9F1E_IFDNumLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);

					inCTLS_ISOFormatDebug_PRINT("9F1E", pobTran->srEMVRec.in9F1E_IFDNumLen, szASCII);
				}
				continue;
			}

#ifdef _DEBUG_9F1F_TAG_
			/* [富邦新增 TAG] */
			if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x1F)
			{
				inCnt += 2;
				gln9F1FLen = uszCTLSData[inCnt++];
				memcpy((char *)&gusz9F1F[0], &uszCTLSData[inCnt], gln9F1FLen);
				inCnt += gln9F1FLen;

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					inPRINT_ChineseFont("Countless Card", _PRT_ISO_);
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], (unsigned char *)&szTempBuf[0], inTempLen);
					inCTLS_ISOFormatDebug_PRINT("9F1F", inTempLen, szASCII);
				}
				continue;
			}
#endif

			if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x21)
			{
				inCnt += 2;
				ushTagLen = uszCTLSData[inCnt++];
				memset(uszTagData, 0x00, sizeof(uszTagData));
				memcpy((char *)&uszTagData[0], &uszCTLSData[inCnt], ushTagLen);
				inCnt += ushTagLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);

					inCTLS_ISOFormatDebug_DISP("9F21", ushTagLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);

					inCTLS_ISOFormatDebug_PRINT("9F21", ushTagLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x26)
			{
				inCnt += 2;
				pobTran->srEMVRec.in9F26_ApplCryptogramLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz9F26_ApplCryptogram, 0x00, sizeof(pobTran->srEMVRec.usz9F26_ApplCryptogram));
				memcpy((char *)&pobTran->srEMVRec.usz9F26_ApplCryptogram[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F26_ApplCryptogramLen);
				inCnt += pobTran->srEMVRec.in9F26_ApplCryptogramLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F26_ApplCryptogram[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);

					inCTLS_ISOFormatDebug_DISP("9F26", pobTran->srEMVRec.in9F26_ApplCryptogramLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F26_ApplCryptogram[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);

					inCTLS_ISOFormatDebug_PRINT("9F26", pobTran->srEMVRec.in9F26_ApplCryptogramLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x27)
			{
				inCnt += 2;
				pobTran->srEMVRec.in9F27_CIDLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz9F27_CID, 0x00, sizeof(pobTran->srEMVRec.usz9F27_CID));
				memcpy((char *)&pobTran->srEMVRec.usz9F27_CID[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F27_CIDLen);
				inCnt += pobTran->srEMVRec.in9F27_CIDLen;

				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F27_CID[0], pobTran->srEMVRec.in9F27_CIDLen);

					inCTLS_ISOFormatDebug_DISP("9F27", pobTran->srEMVRec.in9F27_CIDLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F27_CID[0], pobTran->srEMVRec.in9F27_CIDLen);

					inCTLS_ISOFormatDebug_PRINT("9F27", pobTran->srEMVRec.in9F27_CIDLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x33)
			{
				inCnt += 2;
				pobTran->srEMVRec.in9F33_TermCapabilitiesLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz9F33_TermCapabilities, 0x00, sizeof(pobTran->srEMVRec.usz9F33_TermCapabilities));
				memcpy((char *)&pobTran->srEMVRec.usz9F33_TermCapabilities[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
				inCnt += pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F33_TermCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);

					inCTLS_ISOFormatDebug_DISP("9F33", pobTran->srEMVRec.in9F33_TermCapabilitiesLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F33_TermCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);

					inCTLS_ISOFormatDebug_PRINT("9F33", pobTran->srEMVRec.in9F33_TermCapabilitiesLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x34)
			{
				inCnt += 2;
				pobTran->srEMVRec.in9F34_CVMLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz9F34_CVM, 0x00, sizeof(pobTran->srEMVRec.usz9F34_CVM));
				memcpy((char *)&pobTran->srEMVRec.usz9F34_CVM[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F34_CVMLen);
				inCnt += pobTran->srEMVRec.in9F34_CVMLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);

					inCTLS_ISOFormatDebug_DISP("9F34", pobTran->srEMVRec.in9F34_CVMLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);

					inCTLS_ISOFormatDebug_PRINT("9F34", pobTran->srEMVRec.in9F34_CVMLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x35)
			{
				inCnt += 2;
				pobTran->srEMVRec.in9F35_TermTypeLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz9F35_TermType, 0x00, sizeof(pobTran->srEMVRec.usz9F35_TermType));
				memcpy((char *)&pobTran->srEMVRec.usz9F35_TermType[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F35_TermTypeLen);
				inCnt += pobTran->srEMVRec.in9F35_TermTypeLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);

					inCTLS_ISOFormatDebug_DISP("9F35", pobTran->srEMVRec.in9F35_TermTypeLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);

					inCTLS_ISOFormatDebug_PRINT("9F35", pobTran->srEMVRec.in9F35_TermTypeLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x36)
			{
				inCnt += 2;
				pobTran->srEMVRec.in9F36_ATCLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz9F36_ATC, 0x00, sizeof(pobTran->srEMVRec.usz9F36_ATC));
				memcpy((char *)&pobTran->srEMVRec.usz9F36_ATC[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F36_ATCLen);
				inCnt += pobTran->srEMVRec.in9F36_ATCLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F36_ATC[0], pobTran->srEMVRec.in9F36_ATCLen);

					inCTLS_ISOFormatDebug_DISP("9F36", pobTran->srEMVRec.in9F36_ATCLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F36_ATC[0], pobTran->srEMVRec.in9F36_ATCLen);

					inCTLS_ISOFormatDebug_PRINT("9F36", pobTran->srEMVRec.in9F36_ATCLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x37)
			{
				inCnt += 2;
				pobTran->srEMVRec.in9F37_UnpredictNumLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz9F37_UnpredictNum, 0x00, sizeof(pobTran->srEMVRec.usz9F37_UnpredictNum));
				memcpy((char *)&pobTran->srEMVRec.usz9F37_UnpredictNum[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F37_UnpredictNumLen);
				inCnt += pobTran->srEMVRec.in9F37_UnpredictNumLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F37_UnpredictNum[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);

					inCTLS_ISOFormatDebug_DISP("9F37", pobTran->srEMVRec.in9F37_UnpredictNumLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F37_UnpredictNum[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);

					inCTLS_ISOFormatDebug_PRINT("9F37", pobTran->srEMVRec.in9F37_UnpredictNumLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x41)
			{
				inCnt += 2;
				pobTran->srEMVRec.in9F41_TransSeqCounterLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz9F41_TransSeqCounter, 0x00, sizeof(pobTran->srEMVRec.usz9F41_TransSeqCounter));
				memcpy((char *)&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
				inCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);

					inCTLS_ISOFormatDebug_DISP("9F41", pobTran->srEMVRec.in9F41_TransSeqCounterLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);

					inCTLS_ISOFormatDebug_PRINT("9F41", pobTran->srEMVRec.in9F41_TransSeqCounterLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x45)
			{
				inCnt += 2;
				ushTagLen = uszCTLSData[inCnt++];
				memset(uszTagData, 0x00, sizeof(uszTagData));
				memcpy((char *)&uszTagData[0], &uszCTLSData[inCnt], ushTagLen);
				inCnt += ushTagLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);

					inCTLS_ISOFormatDebug_DISP("9F45", ushTagLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);

					inCTLS_ISOFormatDebug_PRINT("9F45", ushTagLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x53)
			{
				inCnt += 2;
				ushTagLen = uszCTLSData[inCnt++];
				memset(uszTagData, 0x00, sizeof(uszTagData));
				memcpy((char *)&uszTagData[0], &uszCTLSData[inCnt], ushTagLen);
				inCnt += ushTagLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);

					inCTLS_ISOFormatDebug_DISP("9F53", ushTagLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);

					inCTLS_ISOFormatDebug_PRINT("9F53", ushTagLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x66)
			{
				inCnt += 2;
				pobTran->srEMVRec.in9F66_QualifiersLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz9F66_Qualifiers, 0x00, sizeof(pobTran->srEMVRec.usz9F66_Qualifiers));
				memcpy((char *)&pobTran->srEMVRec.usz9F66_Qualifiers[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F66_QualifiersLen);
				inCnt += pobTran->srEMVRec.in9F66_QualifiersLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F66_Qualifiers[0], pobTran->srEMVRec.in9F66_QualifiersLen);

					inCTLS_ISOFormatDebug_DISP("9F66", pobTran->srEMVRec.in9F66_QualifiersLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F66_Qualifiers[0], pobTran->srEMVRec.in9F66_QualifiersLen);

					inCTLS_ISOFormatDebug_PRINT("9F66", pobTran->srEMVRec.in9F66_QualifiersLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x6E)
			{
				inCnt += 2;
				pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.usz9F6E_From_Factor_Indicator, 0x00, sizeof(pobTran->srEMVRec.usz9F6E_From_Factor_Indicator));
				memcpy((char *)&pobTran->srEMVRec.usz9F6E_From_Factor_Indicator[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen);
				inCnt += pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F6E_From_Factor_Indicator[0], pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen);

					inCTLS_ISOFormatDebug_DISP("9F6E", pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F6E_From_Factor_Indicator[0], pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen);

					inCTLS_ISOFormatDebug_PRINT("9F6E", pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x74)
			{
				inCnt += 2;
				ushTagLen = uszCTLSData[inCnt++];
				memset(uszTagData, 0x00, sizeof(uszTagData));
				memcpy((char *)&uszTagData[0], &uszCTLSData[inCnt], ushTagLen);

				/* 感應退貨不能塞VLP */
				if (pobTran->inTransactionCode == _REFUND_ ||
					pobTran->inTransactionCode == _CUP_REFUND_)
				{
				}
				else
				{
					memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
					memcpy((char *)&pobTran->srBRec.szAuthCode[0], uszTagData, ushTagLen);
				}

				inCnt += ushTagLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);

					inCTLS_ISOFormatDebug_DISP("9F74", ushTagLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);

					inCTLS_ISOFormatDebug_PRINT("9F74", ushTagLen, szASCII);
				}
				continue;
			}

			if (uszCTLSData[inCnt] == 0xDF && uszCTLSData[inCnt + 1] == 0x8F && uszCTLSData[inCnt + 2] == 0x4F)
			{
				/*
				   Add By Tim 2015-06-16 AM 10:09:51
				   DF8F4F Responded Advanced Transaction Result
				   0001h    PayPass MSD Online
				   0002h    Offline Approval
				   0003h    Offline Declined
				   0004h    Online Transaction
				   0005h    VisaWave MSD Online

				   Tag DF8F4F Responded Advanced Transaction Result，是判斷QuickPass、NewJspeedy交易最後的結果。
				   Tag是QP3000S自訂的Tag。
				*/
				inCnt += 3;
				pobTran->srEMVRec.inDF8F4F_TransactionResultLen = uszCTLSData[inCnt++];
				memset(pobTran->srEMVRec.uszDF8F4F_TransactionResult, 0x00, sizeof(pobTran->srEMVRec.uszDF8F4F_TransactionResult));
				memcpy((char *)&pobTran->srEMVRec.uszDF8F4F_TransactionResult[0], &uszCTLSData[inCnt], pobTran->srEMVRec.inDF8F4F_TransactionResultLen);
				inCnt += pobTran->srEMVRec.inDF8F4F_TransactionResultLen;
				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Not Caught");
					inDISP_LogPrintf(szDebugMsg);

					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDF8F4F_TransactionResult[0], pobTran->srEMVRec.inDF8F4F_TransactionResultLen);

					inCTLS_ISOFormatDebug_DISP("DF8F4F", pobTran->srEMVRec.inDF8F4F_TransactionResultLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Not Caught");
					inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);

					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDF8F4F_TransactionResult[0], pobTran->srEMVRec.inDF8F4F_TransactionResultLen);

					inCTLS_ISOFormatDebug_PRINT("DF8F4F", pobTran->srEMVRec.inDF8F4F_TransactionResultLen, szASCII);
				}
				continue;
			}

			/* EMV規格設計 欄為兩格的為 5F 9F DF，則前面第一個值和0x1F and會等於0x1F */
			/* 無法解開的值 */
			/* 有一些自定義的Tag值，像DF8129，也有DF8Fxx之類的值，所以用0x80來分 */
			if ((uszCTLSData[inCnt] & 0xDF) == 0xDF && (uszCTLSData[inCnt + 1] & 0x80) == 0x80)
			{
				inCnt += 3;
				ushTagLen = uszCTLSData[inCnt++];
				memset(uszTagData, 0x00, sizeof(uszTagData));
				memcpy(uszTagData, &uszCTLSData[inCnt], ushTagLen);
				inCnt += ushTagLen;

				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Not Caught");
					inDISP_LogPrintf(szDebugMsg);

					memset(szTagName, 0x00, sizeof(szTagName));
					inFunc_BCD_to_ASCII(szTagName, &uszCTLSData[inCnt - ushTagLen - 1 - 3], 3);

					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(szASCII, uszTagData, ushTagLen);

					inCTLS_ISOFormatDebug_DISP(szTagName, ushTagLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Not Caught");
					inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);

					memset(szTagName, 0x00, sizeof(szTagName));
					inFunc_BCD_to_ASCII(szTagName, &uszCTLSData[inCnt - ushTagLen - 1 - 3], 3);

					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(szASCII, uszTagData, ushTagLen);

					inCTLS_ISOFormatDebug_PRINT(szTagName, ushTagLen, szASCII);
				}
				continue;
			}
			else if ((uszCTLSData[inCnt] & 0x1F) == 0x1F)
			{
				inCnt += 2;
				ushTagLen = uszCTLSData[inCnt++];
				memset(uszTagData, 0x00, sizeof(uszTagData));
				memcpy(uszTagData, &uszCTLSData[inCnt], ushTagLen);
				inCnt += ushTagLen;

				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Not Caught");
					inDISP_LogPrintf(szDebugMsg);

					memset(szTagName, 0x00, sizeof(szTagName));
					inFunc_BCD_to_ASCII(szTagName, &uszCTLSData[inCnt - ushTagLen - 1 - 2], 2);

					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(szASCII, uszTagData, ushTagLen);

					inCTLS_ISOFormatDebug_DISP(szTagName, ushTagLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Not Caught");
					inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);

					memset(szTagName, 0x00, sizeof(szTagName));
					inFunc_BCD_to_ASCII(szTagName, &uszCTLSData[inCnt - ushTagLen - 1 - 2], 2);

					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(szASCII, uszTagData, ushTagLen);

					inCTLS_ISOFormatDebug_PRINT(szTagName, ushTagLen, szASCII);
				}
				continue;
			}
			else
			{
				inCnt += 1;
				ushTagLen = uszCTLSData[inCnt++];
				memset(uszTagData, 0x00, sizeof(uszTagData));
				memcpy(uszTagData, &uszCTLSData[inCnt], ushTagLen);
				inCnt += ushTagLen;

				/* ISO Display Debug */
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Not Caught");
					inDISP_LogPrintf(szDebugMsg);

					memset(szTagName, 0x00, sizeof(szTagName));
					inFunc_BCD_to_ASCII(szTagName, &uszCTLSData[inCnt - ushTagLen - 1 - 1], 1);

					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(szASCII, uszTagData, ushTagLen);

					inCTLS_ISOFormatDebug_DISP(szTagName, ushTagLen, szASCII);
				}

				/* ISO Print Debug */
				if (ginISODebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Not Caught");
					inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);

					memset(szTagName, 0x00, sizeof(szTagName));
					inFunc_BCD_to_ASCII(szTagName, &uszCTLSData[inCnt - ushTagLen - 1 - 1], 1);

					memset(szASCII, 0x00, sizeof(szASCII));
					inFunc_BCD_to_ASCII(szASCII, uszTagData, ushTagLen);

					inCTLS_ISOFormatDebug_PRINT(szTagName, ushTagLen, szASCII);
				}
				continue;
			}

			inCnt++;
		} /* End for () .... */

		uszChipFlag = VS_FALSE;

		if (i == 2)
			uszAdditionalFlag = VS_FALSE;

		if (uszAdditionalFlag == VS_TRUE)
		{
			memset(uszCTLSData, 0x00, sizeof(uszCTLSData));
			memcpy(uszCTLSData, szRCDataEx.baAdditionalData, szRCDataEx.usAdditionalDataLen);
			inDataSizes = szRCDataEx.usAdditionalDataLen;
			inCnt = 0;
			i = 2; /* 第二圈 */
		}
	} while ((uszChipFlag == VS_TRUE) || (uszAdditionalFlag == VS_TRUE));

	if (uszTag5F2A == VS_TRUE && uszTag9F1A == VS_TRUE)
	{
		/* 表示是以本國貨幣交易 */
		memset(pobTran->srEMVRec.usz9F1A_TermCountryCode, 0x00, sizeof(pobTran->srEMVRec.usz9F1A_TermCountryCode));
		pobTran->srEMVRec.usz9F1A_TermCountryCode[0] = 0x01;
		pobTran->srEMVRec.usz9F1A_TermCountryCode[1] = 0x58;
		pobTran->srEMVRec.inDF8F4F_TransactionResultLen = 2;
	}

	/* Serial Number自己塞 */
	if (pobTran->srEMVRec.in9F1E_IFDNumLen == 0)
	{
		memset(szSerialNumber, 0x00, sizeof(szSerialNumber));
		inFunc_GetSeriaNumber(szSerialNumber);

		pobTran->srEMVRec.in9F1E_IFDNumLen = 8;
		memcpy((unsigned char *)pobTran->srEMVRec.usz9F1E_IFDNum, &szSerialNumber[7], pobTran->srEMVRec.in9F1E_IFDNumLen);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("=================================");
		inDISP_LogPrintf("inCTLS_ProcessChipData_END");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_ISOFormatDebug_DISP
Date&Time       :2016/12/8 上午 10:01
Describe        :
*/
int inCTLS_ISOFormatDebug_DISP(char *szTag, unsigned short ushTaglen, char *szTagData)
{
	int inOneLineLen = 34;
	int inPrintLineCnt = 0;
	char szPrintTag[512 + 1];
	char szPrintLineData[36 + 1];

	memset(szPrintTag, 0x00, sizeof(szPrintTag));
	if (strlen(szTagData) < 20)
	{
		sprintf(&szPrintTag[0], " Tag %s (%02d)[%s]", szTag, ushTaglen, szTagData);
		inDISP_LogPrintf(szPrintTag);
	}
	else
	{
		sprintf(&szPrintTag[0], " Tag %s (%02d)", szTag, ushTaglen);
		inDISP_LogPrintf(szPrintTag);

		inPrintLineCnt = 0;
		while ((inPrintLineCnt * inOneLineLen) < strlen(szTagData))
		{
			memset(szPrintLineData, 0x00, sizeof(szPrintLineData));
			memset(szPrintTag, 0x00, sizeof(szPrintTag));
			if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szTagData))
			{
				strcat(szPrintLineData, &szTagData[inPrintLineCnt * inOneLineLen]);
			}
			else
			{
				memcpy(szPrintLineData, &szTagData[inPrintLineCnt * inOneLineLen], inOneLineLen);
			}
			sprintf(szPrintTag, " [%s]", szPrintLineData);
			inDISP_LogPrintf(szPrintTag);
			inPrintLineCnt++;
		};
	}

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_ISOFormatDebug_PRINT
Date&Time       :2016/12/8 上午 10:01
Describe        :
*/
int inCTLS_ISOFormatDebug_PRINT(char *szTag, unsigned short ushTaglen, char *szTagData)
{
	int inOneLineLen = 34;
	int inPrintLineCnt = 0;
	char szPrintTag[512 + 1];
	char szPrintLineData[36 + 1];

	memset(szPrintTag, 0x00, sizeof(szPrintTag));
	if (strlen(szTagData) < 20)
	{
		sprintf(&szPrintTag[0], " Tag %s (%02d)[%s]", szTag, ushTaglen, szTagData);
		inPRINT_ChineseFont(szPrintTag, _PRT_ISO_);
	}
	else
	{
		sprintf(&szPrintTag[0], " Tag %s (%02d)", szTag, ushTaglen);
		inPRINT_ChineseFont(szPrintTag, _PRT_ISO_);

		inPrintLineCnt = 0;
		while ((inPrintLineCnt * inOneLineLen) < strlen(szTagData))
		{
			memset(szPrintLineData, 0x00, sizeof(szPrintLineData));
			memset(szPrintTag, 0x00, sizeof(szPrintTag));
			if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szTagData))
			{
				strcat(szPrintLineData, &szTagData[inPrintLineCnt * inOneLineLen]);
			}
			else
			{
				memcpy(szPrintLineData, &szTagData[inPrintLineCnt * inOneLineLen], inOneLineLen);
			}
			sprintf(szPrintTag, " [%s]", szPrintLineData);
			inPRINT_ChineseFont(szPrintTag, _PRT_ISO_);
			inPrintLineCnt++;
		};
	}

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_ExceptionCheck
Date&Time       :2017/1/18 下午 1:57
Describe        :檢查例外狀況
*/
int inCTLS_ExceptionCheck(TRANSACTION_OBJECT *pobTran)
{
	int inVWTIndex = -1;
	int inMVTIndex = -1;
	int i;
	long lnCheckAmount = 0;
	char szASCII[256 + 1];
	char szTemplate[256 + 1];
	char szPayWaveAPID[32 + 1], szPayWaveAPIDContactlessTransactionLimit[12 + 1], szPayWaveAPIDCVMRequiredLimit[12 + 1];
	char szContactlessTransactionLimit[12 + 1], szCVMRequiredLimit[12 + 1], szTerminalCapabilities[6 + 1];
	char szWAVEDate[15 + 1], szEDCDate[15 + 1];
	char szCardSupEnable[2 + 1];
	char szDebugMsg[100 + 1];
	VS_BOOL fCheckAPID = VS_FALSE;
	RTC_NEXSYS srRTC; /* Date & Time */

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_ExceptionCheck START!!");
	}

	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1 || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
	{
		inMVTIndex = _VISA_MVT_INDEX_;
		inVWTIndex = _PAYWAVE_VWT_INDEX_;
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY)
	{
		inMVTIndex = _JSMART_MVT_INDEX_;
		inVWTIndex = _JSPEEDY_VWT_INDEX_;
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE)
	{
		inMVTIndex = _MCHIP_MVT_INDEX_;
		inVWTIndex = _PAYPASS_VWT_INDEX_;
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS && !memcmp(&pobTran->srEMVRec.usz84_DF_NAME[0], "\xA0\x00\x00\x03\x33\x01\x01\x01", 8))
	{
		inMVTIndex = _CUP_DEBIT_MVT_INDEX_;
		inVWTIndex = _QP_DEBIT_VWT_INDEX_;
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS && !memcmp(&pobTran->srEMVRec.usz84_DF_NAME[0], "\xA0\x00\x00\x03\x33\x01\x01\x02", 8))
	{
		inMVTIndex = _CUP_CREDIT_MVT_INDEX_;
		inVWTIndex = _QP_CREDIT_VWT_INDEX_;
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS && !memcmp(&pobTran->srEMVRec.usz84_DF_NAME[0], "\xA0\x00\x00\x03\x33\x01\x01\x03", 8))
	{
		inMVTIndex = _CUP_QUASI_CREDIT_MVT_INDEX_;
		inVWTIndex = _QP_QUASI_CREDIT_VWT_INDEX_;
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
	{
		inMVTIndex = _AEIPS_MVT_INDEX_;
		inVWTIndex = _AE_EXPRESSPAY_VWT_INDEX_;
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS)
	{
		inMVTIndex = _D_PAS_MVT_INDEX_;
		inVWTIndex = _D_PAS_VWT_INDEX_;
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("VWT 中無此感應卡");
		}
		return (VS_WAVE_ERROR); /* 表示記憶體有問題或是 VWT Table 不存在 */
	}

	if (inLoadMVTRec(inMVTIndex) < 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("MV 中無此感應卡");
		}

		return (VS_WAVE_ERROR); /* 表示記憶體有問題或是 MVT Table 不存在 */
	}

	if (inLoadVWTRec(inVWTIndex) < 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("VWT 中無此感應卡");
		}

		return (VS_WAVE_ERROR); /* 表示記憶體有問題或是 VWT Table 不存在 */
	}

	/* 先檢查是否TMS支援此卡別 */
	memset(szCardSupEnable, 0x00, sizeof(szCardSupEnable));
	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1 ||
		pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
	{
		inGetVISAPaywaveEnable(szCardSupEnable);
		if (memcmp(szCardSupEnable, "Y", strlen("Y")) != 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CFGT 不支援 VISA WAVE");
				inDISP_LogPrintf(szDebugMsg);
			}

			/* 不接受此感應卡 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC;

			return (VS_WAVE_ERROR);
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY)
	{
		inGetJCBJspeedyEnable(szCardSupEnable);
		if (memcmp(szCardSupEnable, "Y", strlen("Y")) != 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CFGT 不支援 JSPEEDY");
				inDISP_LogPrintf(szDebugMsg);
			}

			/* 不接受此感應卡 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC;

			return (VS_WAVE_ERROR);
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
	{
		inGetMCPaypassEnable(szCardSupEnable);
		if (memcmp(szCardSupEnable, "Y", strlen("Y")) != 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CFGT 不支援 MChip");
				inDISP_LogPrintf(szDebugMsg);
			}

			/* 不接受此感應卡 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC;

			return (VS_WAVE_ERROR);
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS)
	{
		inGetCUPContactlessEnable(szCardSupEnable);
		if (memcmp(szCardSupEnable, "Y", strlen("Y")) != 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CFGT 不支援 Quickpass");
				inDISP_LogPrintf(szDebugMsg);
			}

			/* 不接受此感應卡 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC;

			return (VS_WAVE_ERROR);
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
	{
		inGetAMEXContactlessEnable(szCardSupEnable);
		if (memcmp(szCardSupEnable, "Y", strlen("Y")) != 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CFGT 不支援 AE Contactless");
				inDISP_LogPrintf(szDebugMsg);
			}

			/* 不接受此感應卡 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC;

			return (VS_WAVE_ERROR);
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS)
	{
		inGetDFS_Contactless_Enable(szCardSupEnable);
		if (memcmp(szCardSupEnable, "Y", strlen("Y")) != 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CFGT 不支援 DFS Contactless");
				inDISP_LogPrintf(szDebugMsg);
			}

			/* 不接受此感應卡 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC;

			return (VS_WAVE_ERROR);
		}
	}

	/* 先看感應限額(ContactlessTransactionLimit) */
	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1 || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
	{
		/* 【需求單104154】VCPS2.1.3規範，新增APID判定國內外限額需求 */
		/* Paywave 有APID，有自己獨立的Transaction Limti, CVM Limit, Floor Limit，所以這裡先看是不是APID */
		memset(szPayWaveAPID, 0x00, sizeof(szPayWaveAPID));
		inGetPayWaveAPID(szPayWaveAPID);

		if (pobTran->srEMVRec.in9F5A_Application_Program_IdentifierLen > 0 && strlen(szPayWaveAPID) > 0)
		{
			memset(szASCII, 0x00, sizeof(szASCII));
			inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F5A_Application_Program_Identifier[0], pobTran->srEMVRec.in9F5A_Application_Program_IdentifierLen);

			/* 比對APID */
			if (!memcmp(&szASCII[0], &szPayWaveAPID[0], strlen(szPayWaveAPID)))
			{
				/* 有比對到APID */
				fCheckAPID = VS_TRUE;
			}
			else
			{
				/* 沒有比對到APID */
				fCheckAPID = VS_FALSE;
			}
		}

		/* 有比對到APID */
		if (fCheckAPID == VS_TRUE)
		{
			/* 檢核感應限額 */
			memset(szPayWaveAPIDContactlessTransactionLimit, 0x00, sizeof(szPayWaveAPIDContactlessTransactionLimit));
			memset(szTemplate, 0x00, sizeof(szTemplate));

			inGetPayWaveAPIDContactlessTransactionLimit(szTemplate);
			memcpy(szPayWaveAPIDContactlessTransactionLimit, szTemplate, 10);

			lnCheckAmount = atol(szPayWaveAPIDContactlessTransactionLimit);

			if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			{
				/* 超過感應限額 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_OVER_AMOUNT_;

				return (VS_WAVE_AMOUNT_ERR);
			}
		}
		/* 沒有比對到APID */
		else if (fCheckAPID == VS_FALSE)
		{
			memset(szContactlessTransactionLimit, 0x00, sizeof(szContactlessTransactionLimit));
			memset(szTemplate, 0x00, sizeof(szTemplate));

			inGetContactlessTransactionLimit(szTemplate);
			memcpy(szContactlessTransactionLimit, szTemplate, 10);

			/* 第一步檢核感應限額 */
			lnCheckAmount = atol(szContactlessTransactionLimit);

			if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			{
				/* 超過感應限額 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_OVER_AMOUNT_;

				return (VS_WAVE_AMOUNT_ERR);
			}
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS)
	{
		memset(szContactlessTransactionLimit, 0x00, sizeof(szContactlessTransactionLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetContactlessTransactionLimit(szTemplate);
		memcpy(szContactlessTransactionLimit, szTemplate, 10);

		lnCheckAmount = atol(szContactlessTransactionLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
		{
			/* 超過感應限額 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_OVER_AMOUNT_;
			return (VS_WAVE_AMOUNT_ERR);
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
	{
		/* VWT中ContactlessTransactionLimit 不For PayWave 使用 */
	}

	/* 確認CID，虹堡系列都會吐DF8F4F 但為了簡潔，直接看CID來看online offline */
	/*【PayWave3】感應卡 */
	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
	{
		if (pobTran->srEMVRec.in9F27_CIDLen <= 0)
		{
			/* 感應資料不足 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_CTLS_DATA_SHORT_;
			return (VS_WAVE_ERROR);
		}
		else
		{
			if (pobTran->srEMVRec.usz9F27_CID[0] == 0x40)
			{
				/* TC */
				/* 分期、紅利不能offline */
				if (pobTran->srBRec.uszInstallmentBit == VS_TRUE ||
					pobTran->srBRec.uszRedeemBit == VS_TRUE)
				{
				}
				else
				{
					memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
					strcpy(pobTran->srBRec.szAuthCode, "Y1");
				}
			}
			else if (pobTran->srEMVRec.usz9F27_CID[0] == 0x80)
			{
				/* ARQC */
				/* 要 Online */
			}
			else
			{
				/* AAC */
				/* 拒絕交易 Z1 */
				/* 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_Z1_;
				return (VS_WAVE_ERROR);
			}
		}

		/* (6)	PayWave 3卡片感應交易，若卡片未回傳 PAN_SEQ_NUM 時，端末機應以預設值 “00” (Hard code) 上傳。 */
		if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen <= 0)
		{
			pobTran->srEMVRec.in5F34_ApplPanSeqnumLen = 1;
			memset(pobTran->srEMVRec.usz5F34_ApplPanSeqnum, 0x00, sizeof(pobTran->srEMVRec.usz5F34_ApplPanSeqnum));
			pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0] = 0x00;
		}
	}
	/* 【Newjspeedy】感應卡 */
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY)
	{
		if (pobTran->srEMVRec.in9F27_CIDLen > 0)
		{
			if (pobTran->srEMVRec.usz9F27_CID[0] == 0x40)
			{
				/* TC */
				/* 分期、紅利不能offline */
				if (pobTran->srBRec.uszInstallmentBit == VS_TRUE ||
					pobTran->srBRec.uszRedeemBit == VS_TRUE)
				{
				}
				else
				{
					memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
					strcpy(pobTran->srBRec.szAuthCode, "Y1");
				}
			}
			else if (pobTran->srEMVRec.usz9F27_CID[0] == 0x80)
			{
				/* ARQC */
				/* 要 Online */
			}
			else
			{
				/* AAC */
				/* 拒絕交易 Z1 */
				/* 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_Z1_;
				return (VS_WAVE_ERROR);
			}
		}

		/* (6)	Newjspeedy卡片感應交易，若卡片未回傳 PAN_SEQ_NUM 時，端末機應以預設值 “00” (Hard code) 上傳。 */
		if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen <= 0)
		{
			pobTran->srEMVRec.in5F34_ApplPanSeqnumLen = 1;
			memset(pobTran->srEMVRec.usz5F34_ApplPanSeqnum, 0x00, sizeof(pobTran->srEMVRec.usz5F34_ApplPanSeqnum));
			pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0] = 0x00;
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS)
	{
		if (pobTran->srEMVRec.in9F27_CIDLen <= 0)
		{
			/* 感應資料不足 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_CTLS_DATA_SHORT_;
			return (VS_WAVE_ERROR);
		}
		else
		{
			if (pobTran->srEMVRec.usz9F27_CID[0] == 0x40)
			{
				/* TC */
				/* 銀聯不能offline */
			}
			else if (pobTran->srEMVRec.usz9F27_CID[0] == 0x80)
			{
				/* ARQC */
				/* 要 Online */
			}
			else
			{
				/* AAC */
				/* 拒絕交易 Z1 */
				/* 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_Z1_;
				return (VS_WAVE_ERROR);
			}
		}
	}
	/* 原先於ProcessChipData時塞Y1，現在統一拉下來塞 */
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
	{
		if (pobTran->srEMVRec.in9F27_CIDLen > 0)
		{
			if (pobTran->srEMVRec.usz9F27_CID[0] == 0x40)
			{
				/* TC */
				/* 分期、紅利不能offline */
				if (pobTran->srBRec.uszInstallmentBit == VS_TRUE ||
					pobTran->srBRec.uszRedeemBit == VS_TRUE)
				{
				}
				else
				{
					memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
					strcpy(pobTran->srBRec.szAuthCode, "Y1");
				}
			}
			else if (pobTran->srEMVRec.usz9F27_CID[0] == 0x80)
			{
				/* ARQC */
				/* 要 Online */
			}
			else
			{
				/* AAC */
				/* 拒絕交易 Z1 */
				/* 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_Z1_;
				return (VS_WAVE_ERROR);
			}
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
	{
		if (pobTran->srEMVRec.in9F27_CIDLen <= 0)
		{
			/* 感應資料不足 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_CTLS_DATA_SHORT_;
			return (VS_WAVE_ERROR);
		}
		else
		{
			if (pobTran->srEMVRec.usz9F27_CID[0] == 0x40)
			{
				/* TC */
				/* 分期、紅利不能offline */
				if (pobTran->srBRec.uszInstallmentBit == VS_TRUE ||
					pobTran->srBRec.uszRedeemBit == VS_TRUE)
				{
				}
				else
				{
					memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
					strcpy(pobTran->srBRec.szAuthCode, "Y1");
				}
			}
			else if (pobTran->srEMVRec.usz9F27_CID[0] == 0x80)
			{
				/* ARQC */
				/* 要 Online */
			}
			else
			{
				/* AAC */
				/* 拒絕交易 Z1 */
				/* 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_Z1_;
				return (VS_WAVE_ERROR);
			}
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS)
	{
		if (pobTran->srEMVRec.in9F27_CIDLen <= 0)
		{
			/* 感應資料不足 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_CTLS_DATA_SHORT_;
			return (VS_WAVE_ERROR);
		}
		else
		{
			if (pobTran->srEMVRec.usz9F27_CID[0] == 0x40)
			{
				/* TC */
				/* 分期、紅利不能offline */
				if (pobTran->srBRec.uszInstallmentBit == VS_TRUE ||
					pobTran->srBRec.uszRedeemBit == VS_TRUE)
				{
				}
				else
				{
					memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
					strcpy(pobTran->srBRec.szAuthCode, "Y1");
				}
			}
			else if (pobTran->srEMVRec.usz9F27_CID[0] == 0x80)
			{
				/* ARQC */
				/* 要 Online */
			}
			else
			{
				/* AAC */
				/* 拒絕交易 Z1 */
				/* 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_Z1_;
				return (VS_WAVE_ERROR);
			}
		}
	}

	/* 處理狀況一：如果是 VISA Paywave1 & VISA Paywave3 & Jspeedy 要從【mvt.txt】組【9F33】 */
	/* R-FES EDC Specifications v1.49.doc 規格
		1.	Contactless 交易上傳之 Tag 9F33 的 Value 規則如下：
		(1)	VISA Paywave 1/VISA Paywave 3/JSpeedy 請上傳Terminal 之 EMV 參數（因為 V & J 感應交易Reader 不會回 Tag 9F33）。
		(2)	Paypass 因為 Reader 會回覆 Tag 9F33 ，所以依Reader 回覆資料上傳。
	*/
	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1 ||
		pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3 ||
		pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY ||
		pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV ||
		pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD ||
		pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY)
	{
		if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1 ||
			pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
		{
			inMVTIndex = _VISA_MVT_INDEX_;
		}
		else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY ||
				 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV ||
				 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD ||
				 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY)
		{
			inMVTIndex = _JSMART_MVT_INDEX_;
		}

		if (inLoadMVTRec(inMVTIndex) < 0)
		{
			return (VS_WAVE_ERROR);
		}

		/* 有9F33直接用，沒有就從MVT抓 */
		if (pobTran->srEMVRec.in9F33_TermCapabilitiesLen > 0)
		{
		}
		else
		{
			inGetTerminalCapabilities(szTerminalCapabilities);
			memset(pobTran->srEMVRec.usz9F33_TermCapabilities, 0x00, sizeof(pobTran->srEMVRec.usz9F33_TermCapabilities));
			pobTran->srEMVRec.in9F33_TermCapabilitiesLen = ((strlen(szTerminalCapabilities) + 1) / 2);
			inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9F33_TermCapabilities[0], &szTerminalCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
		}
	}

	/* 處理狀況二：所有感應交易要組【DFEE】【DEEF】 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEE_TERMINAL_ENTRY_CAPABILITY, (unsigned char *)&szTemplate[0]) > 0)
	{
		pobTran->srEMVRec.inDFEE_TerEntryCapLen = szTemplate[2];
		memcpy((char *)&pobTran->srEMVRec.uszDFEE_TerEntryCap[0], &szTemplate[3], pobTran->srEMVRec.inDFEE_TerEntryCapLen);
	}

	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEF_REASON_ONLINE_CODE, (unsigned char *)&szTemplate[0]) > 0)
	{
		pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen = szTemplate[2];
		memcpy((char *)&pobTran->srEMVRec.uszDFEF_ReasonOnlineCode[0], &szTemplate[3], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);
	}

	/* 免簽名判斷(CVM Limit) */
	/* 先預設為免簽名 */
	pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
	/* 看9F34 */
	if (pobTran->srEMVRec.in9F34_CVMLen > 0)
	{
		if (pobTran->srEMVRec.usz9F34_CVM[0] == 0x1F)
		{
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
		}
		else
		{
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		}
	}
	else
	{
		/* 先讀CVM Limit */
		memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetCVMRequiredLimit(szTemplate);
		memcpy(szCVMRequiredLimit, szTemplate, 10); /* 小數點後兩位不用 */

		lnCheckAmount = atol(szCVMRequiredLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		else
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
	}

	/* 之前發生過9F34失效，所以下面根據金額再確認一次 */
	/* VISA */
	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1 || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
	{
		if (fCheckAPID == VS_TRUE)
		{
			memset(szPayWaveAPIDCVMRequiredLimit, 0x00, sizeof(szPayWaveAPIDCVMRequiredLimit));
			memset(szTemplate, 0x00, sizeof(szTemplate));

			inGetPayWaveAPIDCVMRequiredLimit(szTemplate);
			memcpy(szPayWaveAPIDCVMRequiredLimit, szTemplate, 10);

			/* 第二步檢核免簽名【VISA Paywave1】卡片要判斷是否免簽名 */
			if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1)
			{
				lnCheckAmount = atol(szPayWaveAPIDCVMRequiredLimit);
				if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
					pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
				else
					pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
			}

			/* 第三步檢核免簽名【VISA Paywave3】卡片要判斷是否免簽名 */
			if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
			{
				if (pobTran->uszPayWave3Tag55Bit == VS_TRUE && pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
				{
					/* 表示已經判斷過【TAG 55】要在判斷一次是否免簽,若要簽名就不用在判斷一次 */
					lnCheckAmount = atol(szPayWaveAPIDCVMRequiredLimit);
					if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
						pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
					else
						pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
				}
				else if (pobTran->uszPayWave3Tag55Bit == VS_FALSE)
				{
					/* 沒有回【TAG 55】 */
					lnCheckAmount = atol(szPayWaveAPIDCVMRequiredLimit);
					if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
						pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
					else
						pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
				}
			}
		}
		else
		{
			/* 先讀CVM Limit */
			memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
			memset(szTemplate, 0x00, sizeof(szTemplate));

			inGetCVMRequiredLimit(szTemplate);
			memcpy(szCVMRequiredLimit, szTemplate, 10); /* 小數點後兩位不用 */

			/* 第二步檢核免簽名【VISA Paywave1】卡片要判斷是否免簽名 */
			if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1)
			{
				lnCheckAmount = atol(szCVMRequiredLimit);
				if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
					pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
				else
					pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
			}

			/* 第三步檢核免簽名【VISA Paywave3】卡片要判斷是否免簽名 */
			if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
			{
				if (pobTran->uszPayWave3Tag55Bit == VS_TRUE && pobTran->srBRec.uszNoSignatureBit != VS_TRUE)
				{
					/* 表示已經判斷過【TAG 55】要在判斷一次 */
					lnCheckAmount = atol(szCVMRequiredLimit);
					if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
						pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
					else
						pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
				}
				else if (pobTran->uszPayWave3Tag55Bit == VS_FALSE)
				{
					/* 沒有回【TAG 55】 */
					lnCheckAmount = atol(szCVMRequiredLimit);
					if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
						pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
					else
						pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
				}
			}
		}
	}
	/* 【PAYpass】卡片要判斷是否免簽名 */
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
	{
		memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetPaypassCVMRequiredLimit(szTemplate);
		memcpy(szCVMRequiredLimit, szTemplate, 10);

		lnCheckAmount = atol(szCVMRequiredLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		else
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
	}
	/* 【JSPEEDY】卡片要判斷是否免簽名 */
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY)
	{
		memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetPaypassCVMRequiredLimit(szTemplate);
		memcpy(szCVMRequiredLimit, szTemplate, 10);

		lnCheckAmount = atol(szCVMRequiredLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		else
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
	}
	/* QUICKPASS */
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS)
	{
		/* 判斷【QuickPass】卡片要判斷是否免簽名(PaypassCVMRequiredLimit 由Paypass、NewJ、QuickPass共用) */
		memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		/* QuickPass此欄位只用來看是否免簽名 */
		inGetPaypassCVMRequiredLimit(szTemplate);
		memcpy(szCVMRequiredLimit, szTemplate, 10);

		lnCheckAmount = atol(szCVMRequiredLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		else
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
	{
		memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetPaypassCVMRequiredLimit(szTemplate);
		memcpy(szCVMRequiredLimit, szTemplate, 10);

		lnCheckAmount = atol(szCVMRequiredLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		else
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS)
	{
		memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetPaypassCVMRequiredLimit(szTemplate);
		memcpy(szCVMRequiredLimit, szTemplate, 10);

		lnCheckAmount = atol(szCVMRequiredLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		else
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
	}

	/* 處理狀況五：如果是授權碼是【Y1】要送【Tag 8A】 */
	if (!memcmp(&pobTran->srBRec.szAuthCode[0], "Y1", 2))
	{
		memset(pobTran->srEMVRec.usz8A_AuthRespCode, 0x00, sizeof(pobTran->srEMVRec.usz8A_AuthRespCode));
		pobTran->srEMVRec.in8A_AuthRespCodeLen = 2;
		memcpy(&pobTran->srEMVRec.usz8A_AuthRespCode[0], "Y1", 2);
	}

	/* 處理狀況六：存卡片有效期 */
	if (strlen(pobTran->srBRec.szExpDate) == 0)
	{
		if (pobTran->srEMVRec.in5F24_ExpireDateLen != 0)
		{
			inFunc_BCD_to_ASCII(&pobTran->srBRec.szExpDate[0], &pobTran->srEMVRec.usz5F24_ExpireDate[0], pobTran->srEMVRec.in5F24_ExpireDateLen);
			pobTran->srBRec.szExpDate[4] = 0x00;
		}
	}

	/* 處理狀況七：卡片沒有回傳Tag9F03 要自己塞長度6及資料填0 */
	if (pobTran->srEMVRec.in9F03_AmtOtherNumLen <= 0)
	{
		pobTran->srEMVRec.in9F03_AmtOtherNumLen = 0x06;
		memset(pobTran->srEMVRec.usz9F03_AmtOtherNum, 0x00, sizeof(pobTran->srEMVRec.usz9F03_AmtOtherNum));
	}

	/* 處理狀況八：感應wave 3的卡片後 組封包上傳主機時不要帶上 Tag 9F35 */
	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
	{
		pobTran->srEMVRec.in9F35_TermTypeLen = 0x00;
		memset(pobTran->srEMVRec.usz9F35_TermType, 0x00, sizeof(pobTran->srEMVRec.usz9F35_TermType));
	}

	/* 取得端末機日期時間 */
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 2011-07-11 PM 03:03:54 JCB & VLP & NewJspeedy卡片種類DF69 = 02:MSG Mode 不送F_55資料 */
	/* 吳升文 2015-05-25 (週一) 上午 11:26 信件說:
	   NewJspeedy 磁條感應卡，請比照 Paypass磁條 送 F_22 091X , 不送F_55。ATS及RFES 皆同。
	*/
	if (memcmp(&pobTran->srBRec.szAuthCode[0], "JCB", 3) != 0 &&
		memcmp(&pobTran->srBRec.szAuthCode[0], "VLP", 3) != 0 &&
		pobTran->srBRec.uszWAVESchemeID != SCHEME_ID_20_PAYPASS_MAG_STRIPE &&
		pobTran->srBRec.uszWAVESchemeID != SCHEME_ID_64_NEWJSPEEDY_MSD &&
		pobTran->srBRec.uszWAVESchemeID != SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
	{
		/* 處理狀況九：日期檢核 */
		/* Add by hanlin 2011.7.4 下午 1:47 (START) */
		if (pobTran->srEMVRec.in9A_TranDateLen <= 0)
		{
			/* 感應失敗 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
			return (VS_WAVE_ERROR);
		}
		else
		{
			memset(szEDCDate, 0x00, sizeof(szEDCDate));
			sprintf(szEDCDate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);

			memset(szWAVEDate, 0x00, sizeof(szWAVEDate));
			inFunc_BCD_to_ASCII(&szWAVEDate[0], &pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);

			if ((memcmp(&szEDCDate[2], &szWAVEDate[0], 6)) || (!memcmp(&szWAVEDate[0], "000000", 6)))
			{
				/* 感應失敗 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
				return (VS_WAVE_ERROR);
			}
		}
		/* Add by hanlin 2011.7.4 下午 1:47 (END) */

		/* 處理狀況十：檢核9F10(IAD)有無值 */
		/* 2011-07-11 PM 02:43:12 add by kakab */
		if (pobTran->srEMVRec.in9F10_IssuerAppDataLen <= 0)
		{
			/* 感應失敗 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
			return (VS_WAVE_ERROR);
		}

		/* 處理狀況十一：檢核9F02(Amount)有無值及範圍在 0~9 */
		/* 2011-07-11 PM 02:43:12 add by kakab */
		if (pobTran->srEMVRec.in9F02_AmtAuthNumLen <= 0)
		{
			/* 感應失敗 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
			return (VS_WAVE_ERROR);
		}
		else
		{
			memset(szWAVEDate, 0x00, sizeof(szWAVEDate));
			inFunc_BCD_to_ASCII(&szWAVEDate[0], &pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);

			for (i = 0; i < pobTran->srEMVRec.in9F02_AmtAuthNumLen * 2; i++)
			{
				if (szWAVEDate[i] <= '9' && szWAVEDate[i] >= '0')
					continue;
				else
				{
					/* 感應失敗 請改刷卡或插卡 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
					return (VS_WAVE_ERROR);
				}
			}
		}

		/* 處理狀況十二：檢核9F26(Application Cryptogram)有無值 */
		/* 2011-07-11 PM 02:43:12 add by kakab */
		if (pobTran->srEMVRec.in9F26_ApplCryptogramLen <= 0)
		{
			/* 感應失敗 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
			return (VS_WAVE_ERROR);
		}
	}

	/* 處理QuickPass感應卡是否要輸入密碼 */
	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS)
	{
		/* 是否輸入密碼移到TRT GET CUP PIN中判斷 */

		/* 再判斷一次【QuickPass】卡片要判斷是否免簽名 */
		memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetPaypassCVMRequiredLimit(szTemplate);
		memcpy(szCVMRequiredLimit, szTemplate, 10);

		lnCheckAmount = atol(szCVMRequiredLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		else
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
	}
	else
	{
		/* 指定為銀聯交易，但是非銀聯感應閃付要檔 */
		/* 2015-09-16 AM 10:13:39 Add By Tim HappyGo流程要支援閃付 */
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			/* 不接受此感應卡 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC;

			return (VS_WAVE_INVALID_SCHEME_ERR);
		}
	}

	/* 分期付款交易，使用感應卡進行交易，皆要簽名，不允許分期付款交易"免簽名"。 */
	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_ExceptionCheck END!!");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_SendReadyForRefund_Flow
Date&Time       :2017/6/12 上午 10:10
Describe        :
*/
int inCTLS_SendReadyForRefund_Flow(TRANSACTION_OBJECT *pobTran)
{
	char szCTLSMode[1 + 1];
	char szDebugMsg[100 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_SendReadyForRefund_Flow()START");
	}

	memset(szCTLSMode, 0x00, sizeof(szCTLSMode));
	inGetContactlessReaderMode(szCTLSMode);
	if (memcmp(szCTLSMode, _CTLS_MODE_0_NO_, strlen(_CTLS_MODE_0_NO_)) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS MODE :None");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else if (memcmp(szCTLSMode, _CTLS_MODE_1_INTERNAL_, strlen(_CTLS_MODE_1_INTERNAL_)) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS MODE :Internal");
			inDISP_LogPrintf(szDebugMsg);
		}

		inCTLS_SendReadyForRefund_Internal(pobTran);
	}
	else if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
			 !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)))
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS MODE :External");
			inDISP_LogPrintf(szDebugMsg);
		}
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_SendReadyForRefund_Flow() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_SendReadyForRefund_Internal
Date&Time       :2017/6/12 上午 10:09
Describe        :
*/
int inCTLS_SendReadyForRefund_Internal(TRANSACTION_OBJECT *pobTran)
{
	char szTemplate[20 + 1], szTxnAmount[12 + 1];
	unsigned char szTransaRelatedData[100 + 1];
	unsigned char uszBCD[20 + 1];

	/* SmartPay感應會跑到Call Back Function, pobTran傳不進去 轉存到global */
	memset((char *)&pobEmvTran, 0x00, _TRANSACTION_OBJECT_SIZE_);
	memcpy((char *)&pobEmvTran, (char *)pobTran, _TRANSACTION_OBJECT_SIZE_);

	memset(&szACTData, 0x00, sizeof(EMVCL_ACT_DATA));
	memset(&szRCDataEx, 0x00, sizeof(EMVCL_RC_DATA_EX));
	memset(&szRCDataAnalyze, 0x00, sizeof(EMVCL_RC_DATA_ANALYZE));
	memset(&srCtlsObj, 0x00, sizeof(CTLS_OBJECT));
	memset(szTransaRelatedData, 0x00, sizeof(szTransaRelatedData));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(szTxnAmount, 0x00, sizeof(szTxnAmount));
	// Prepare Input Data
	szACTData.bStart = d_EMVCL_ACT_DATA_START_A;
	szACTData.bTagNum = 0;
	szACTData.usTransactionDataLen = 0;

	// Put 0x9F02 Amount, Authorized (Numeric)
	szTransaRelatedData[szACTData.usTransactionDataLen++] = 0x9F;
	szTransaRelatedData[szACTData.usTransactionDataLen++] = 0x02;
	szTransaRelatedData[szACTData.usTransactionDataLen++] = 0x06;

	sprintf(szTemplate, "%ld00", pobTran->srBRec.lnTxnAmount);
	inFunc_PAD_ASCII(szTemplate, szTemplate, '0', 12, _PAD_RIGHT_FILL_LEFT_);
	memset(uszBCD, 0x00, sizeof(uszBCD));
	inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 6);
	memcpy((char *)&szTxnAmount[0], (char *)&uszBCD[0], 6);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf(szTemplate);
	}

	memcpy(&szTransaRelatedData[szACTData.usTransactionDataLen], &szTxnAmount[0], 6);
	szACTData.usTransactionDataLen += 6;
	szACTData.bTagNum++;

	/* 是否帶交易時間 */
	szTransaRelatedData[szACTData.usTransactionDataLen++] = 0x9A;
	szTransaRelatedData[szACTData.usTransactionDataLen++] = 0x03;
	memset(uszBCD, 0x00, sizeof(uszBCD));
	inFunc_ASCII_to_BCD(&uszBCD[0], &pobTran->srBRec.szDate[2], 3);
	memcpy(&szTransaRelatedData[szACTData.usTransactionDataLen], uszBCD, 3);
	szACTData.usTransactionDataLen += 3;
	szACTData.bTagNum++;

	// Put 0x9C(交易類別)
	/* For refund transaction it is mandatory to set "Transaction type "as "Returns" transaction (value = 0x20).  */
	/* 先註解掉 仍保留9C 20(refund) */
	/* 吳升文 2016-10-05 (週三) 下午 08:32 回覆 請改以正項交易進行感應。*/
	/* 因目前NewJspeedy 1.3 規格不支援 9C 送 20 (refund)交易 */
	szTransaRelatedData[szACTData.usTransactionDataLen++] = 0x9C;
	szTransaRelatedData[szACTData.usTransactionDataLen++] = 0x01;
	szTransaRelatedData[szACTData.usTransactionDataLen++] = 0x00;
	szACTData.bTagNum++;

	szACTData.pbaTransactionData = szTransaRelatedData;

	/* Start */
	if (EMVCL_InitTransactionEx(szACTData.bTagNum, szACTData.pbaTransactionData, szACTData.usTransactionDataLen) != d_EMVCL_NO_ERROR)
		return (VS_ERROR);

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_ReceiveReadyForRefund_Flow
Date&Time       :2017/6/28 下午 2:53
Describe        :
*/
int inCTLS_ReceiveReadyForRefund_Flow(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR;
	char szCTLSMode[1 + 1];
	unsigned long ulRetVal = VS_ERROR;

	memset(szCTLSMode, 0x00, sizeof(szCTLSMode));
	inGetContactlessReaderMode(szCTLSMode);
	/* 不使用Reader */
	if (memcmp(szCTLSMode, _CTLS_MODE_0_NO_, strlen(_CTLS_MODE_0_NO_)) == 0)
	{
	}
	/* 內建Reader */
	else if (memcmp(szCTLSMode, _CTLS_MODE_1_INTERNAL_, strlen(_CTLS_MODE_1_INTERNAL_)) == 0)
	{
		ulRetVal = ulCTLS_ReceiveReadyForRefund_Internal(pobTran);
		if (ulRetVal != d_EMVCL_PENDING)
		{
			inRetVal = VS_SUCCESS;
		}
	}
	/* 外接Reader */
	else if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
			 !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)))
	{
	}

	return (inRetVal);
}

/*
Function        :ulCTLS_ReceiveReadyForRefund_Internal
Date&Time       :2017/4/20 下午 2:36
Describe        :
*/
unsigned long ulCTLS_ReceiveReadyForRefund_Internal(TRANSACTION_OBJECT *pobTran)
{
	char szDebug[128 + 1];
	unsigned long ulRetVal;

	/* EMVCL_PerformTransactionEx 執行時會跑call back Function vdCTLS_EVENT_EMVCL_NON_EMV_CARD */
	srCtlsObj.lnSaleRespCode = EMVCL_PerformTransactionEx(&szRCDataEx);

	/* 這邊就判斷SmartPay感應卡 */
	if (srCtlsObj.lnSaleRespCode == d_EMVCL_NON_EMV_CARD)
	{
		/* 感應結果已在inFISC_CTLSProcess完成，這邊只要轉存pobTran判斷結果 */
		memset((char *)pobTran, 0x00, _TRANSACTION_OBJECT_SIZE_);
		memcpy((char *)pobTran, (char *)&pobEmvTran, _TRANSACTION_OBJECT_SIZE_);

		if (pobTran->srBRec.inTxnResult == VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("SmartPay CTLS Process SUCCESS");

			/* 跑SmartPay感應流程 */
			/* incode 在vdCTLS_EVENT_EMVCL_NON_EMV_CARD內決定 */
		}
		else
		{
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("SmartPay CTLS Process ERROR");
		}
	}
	else if (srCtlsObj.lnSaleRespCode == d_EMVCL_PENDING)
	{
		/* 等待感應中 */
		//                if (ginDebug == VS_TRUE)
		//                {
		//                        memset(szDebug, 0x00, sizeof(szDebug));
		//                        sprintf(szDebug, "EMVCL_Transaction:0x%X", (unsigned int)srCtlsObj.lnSaleRespCode);
		//                        inDISP_LogPrintf(szDebug);
		//                }
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebug, 0x00, sizeof(szDebug));
			sprintf(szDebug, "EMVCL_Transaction:0x%X", (unsigned int)srCtlsObj.lnSaleRespCode);
			inDISP_LogPrintf(szDebug);
		}
	}

	ulRetVal = srCtlsObj.lnSaleRespCode;

	return (ulRetVal);
}

/*
Function        :ulCTLS_CheckResponseCode_Refund
Date&Time       :2017/6/27 下午 5:25
Describe        :
*/
unsigned long ulCTLS_CheckResponseCode_Refund(TRANSACTION_OBJECT *pobTran)
{
	char szDebugMsg[100 + 1];
	unsigned long ulRetVal;

	switch (srCtlsObj.lnSaleRespCode)
	{
	/* d_EMVCL_RC_DATA 和 d_EMVCL_NON_EMV_CARD 為有收到資料的狀況 */
	case d_EMVCL_RC_DATA:
		pobTran->srBRec.uszContactlessBit = VS_TRUE;

		if (inCTLS_UnPackReadyForRefund_Flow(pobTran) != VS_SUCCESS)
		{
			ulRetVal = d_EMVCL_RC_FAILURE;
		}
		else
		{
			ulRetVal = srCtlsObj.lnSaleRespCode;
		}

		/* 【需求單-105244】端末設備支援以感應方式進行退貨交易 */
		/* 端末設備支援以感應方式進行退貨交易，並讀取卡號及有效期，以人工輸入卡號方式之電文上傳至ATS主機取的授權。 */
		/* 將contactless flag OFF掉並把manual keyin flag ON */
		/*pobTran->srBRec.uszContactlessBit = VS_FALSE;
		pobTran->srBRec.uszManualBit = VS_TRUE;
		pobTran->srBRec.uszRefundCTLSBit = VS_TRUE;*/

		break;
	case d_EMVCL_NON_EMV_CARD:
		/* SmartPay會跑這個case */
		pobTran->srBRec.uszContactlessBit = VS_TRUE;
		ulRetVal = srCtlsObj.lnSaleRespCode;
		break;
	/* 感應無效 */
	case d_EMVCL_RC_FAILURE:
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("CTLS Resp Code = d_EMVCL_RC_FAILURE");

		/* 感應失敗 請改插卡或刷卡 */
		ulRetVal = srCtlsObj.lnSaleRespCode;
		break;
	/* 請問d_EMVCL_RC_FAILURE和d_EMVCL_RC_FALLBACK的差別在哪？
	[Ans] : 這兩個 Return code 都是表示交易的過程中發生了問題而交易終止了。差別在於 Kernel 是否要求要換另一個介面的交易 (例如 : CL 交易失敗，同一張卡改成 CT 或是 MSR 的介面交易)，當然這個轉換介面的要求是根據各個 payment 的規格而定的。
	d_EMVCL_RC_FAILURE : 交易中止
	d_EMVCL_RC_FALLBACK : 交易中止，但嘗試別的介面交易。
	*/
	/* 感應中止，改插卡或刷卡 */
	case d_EMVCL_RC_FALLBACK:
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("CTLS Resp Code = d_EMVCL_RC_FALLBACK");

		/* 感應失敗 請改插卡或刷卡 */
		ulRetVal = srCtlsObj.lnSaleRespCode;
		break;
	/* 請重試 */
	case d_EMVCL_TRY_AGAIN:
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("CTLS Resp Code = d_EMVCL_TRY_AGAIN");

		/* 感應失敗 請改插卡或刷卡 */
		ulRetVal = srCtlsObj.lnSaleRespCode;
		break;
	/* 取消的話，基本上會直接跳出迴圈，所以也不會進這裡 */
	case d_EMVCL_TX_CANCEL:
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("CTLS Resp Code = d_EMVCL_TX_CANCEL");

		/* 感應失敗 請改插卡或刷卡 */
		ulRetVal = srCtlsObj.lnSaleRespCode;
		break;
	/* 多卡重疊 */
	case d_EMVCL_RC_MORE_CARDS:
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("CTLS Resp Code = d_EMVCL_RC_MORE_CARDS");

		/* 感應失敗 請改插卡或刷卡 */
		ulRetVal = srCtlsObj.lnSaleRespCode;
		break;
	/* Timeout 沒感應到卡，這個Timeout指的是xml檔內設定，目前設定到最大，理論上不會出現此回應 */
	case d_EMVCL_RC_NO_CARD: /* Timeout 沒感應到卡 */
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("CTLS Resp Code = d_EMVCL_RC_NO_CARD");

		ulRetVal = srCtlsObj.lnSaleRespCode;
		break;
	case d_EMVCL_NO_ERROR:
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("CTLS Resp Code = d_EMVCL_NO_ERROR");

		ulRetVal = srCtlsObj.lnSaleRespCode;
		break;
	case d_EMVCL_PENDING:
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("CTLS Resp Code = d_EMVCL_PENDING");

		ulRetVal = srCtlsObj.lnSaleRespCode;
		break;
	/* 請問d_EMVCL_RC_DEK_SIGNAL這個回應碼是什麼意思？
	[Ans] : 收到 d_EMVCL_RC_DEK_SIGNAL 表示交易中間 kernel 有訊息要帶出外面給 Application，交易尚未結束，仍須等待 EMVCL_PerformTransactionEx 給出真正的交易結果。
	*/
	case d_EMVCL_RC_DEK_SIGNAL:
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("CTLS Resp Code = d_EMVCL_RC_DEK_SIGNAL");

		/* 感應失敗 請改插卡或刷卡 */
		ulRetVal = srCtlsObj.lnSaleRespCode;
		break;
	/* Two Tap流程 */
	case d_EMVCL_RC_SEE_PHONE:
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("CTLS Resp Code = d_EMVCL_RC_SEE_PHONE");
		/* 請輸密碼或指紋 並再感應一次 */
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		inDISP_PutGraphic(_CTLS_TWO_TAP_, 0, _COORDINATE_Y_LINE_8_7_);
		pobTran->uszTwoTapBit = VS_TRUE;

		ulRetVal = srCtlsObj.lnSaleRespCode;
		break;
	default:
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS Res: %08lX", srCtlsObj.lnSaleRespCode);
			inDISP_LogPrintf(szDebugMsg);
		}
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS Res: %08lX", srCtlsObj.lnSaleRespCode);
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
		}

		ulRetVal = d_EMVCL_RC_FAILURE;
		break;
	}

	return (ulRetVal);
}

/*
Function        :inCTLS_UnPackReadyForRefund_Flow
Date&Time       :2017/6/28 下午 12:02
Describe        :
*/
int inCTLS_UnPackReadyForRefund_Flow(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR;
	char szCTLSMode[1 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_UnPackReadyForRefund_Flow()START");
	}

	memset(szCTLSMode, 0x00, sizeof(szCTLSMode));
	inGetContactlessReaderMode(szCTLSMode);
	/* 不使用Reader */
	if (memcmp(szCTLSMode, _CTLS_MODE_0_NO_, strlen(_CTLS_MODE_0_NO_)) == 0)
	{
	}
	/* 內建Reader */
	else if (memcmp(szCTLSMode, _CTLS_MODE_1_INTERNAL_, strlen(_CTLS_MODE_1_INTERNAL_)) == 0)
	{
		inRetVal = inCTLS_UnPackReadyForRefund_Internal(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			inCTLS_CancelTransacton_Flow();
		}
	}
	/* 外接Reader */
	else if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
			 !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)))
	{
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_UnPackReadyForSale_Flow()END");
	}

	return (inRetVal);
}

/*
Function        :inCTLS_UnPackReadyForRefund_Internal
Date&Time       :2017/6/28 下午 12:02
Describe        :
*/
int inCTLS_UnPackReadyForRefund_Internal(TRANSACTION_OBJECT *pobTran)
{
	int i = 0, inRetVal = -1;
	char szDebug[1024 + 1] = {0};
	char szASCII[64 + 1] = {0};
	char szCUPFunctionEnable[2 + 1] = {0};
	char szMACEnable[2 + 1] = {0};

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_UnPackReadyForRefund_Internal START");
	}
	// Parse transaction response data
	// Parse Scheme ID

	srCtlsObj.uszSchemeID[0] = szRCDataEx.bSID;

	if (ginDebug == VS_TRUE)
	{
		memset(szDebug, 0x00, sizeof(szDebug));
		sprintf(szDebug, "Scheme ID = 0x%02X", srCtlsObj.uszSchemeID[0]);

		switch (srCtlsObj.uszSchemeID[0])
		{
		case d_EMVCL_SID_VISA_OLD_US:
			inDISP_LogPrintf("d_EMVCL_SID_VISA_OLD_US");
			break;
		case d_EMVCL_SID_VISA_WAVE_2:
			inDISP_LogPrintf("d_EMVCL_SID_VISA_WAVE_2");
			break;
		case d_EMVCL_SID_VISA_WAVE_MSD:
			inDISP_LogPrintf("d_EMVCL_SID_VISA_WAVE_MSD");
			break;
		case d_EMVCL_SID_VISA_WAVE_QVSDC:
			inDISP_LogPrintf("d_EMVCL_SID_VISA_WAVE_QVSDC");
			break;
		case d_EMVCL_SID_PAYPASS_MAG_STRIPE:
			inDISP_LogPrintf("d_EMVCL_SID_PAYPASS_MAG_STRIPE");
			break;
		case d_EMVCL_SID_PAYPASS_MCHIP:
			inDISP_LogPrintf("d_EMVCL_SID_PAYPASS_MCHIP");
			break;
		case d_EMVCL_SID_JCB_WAVE_2:
			inDISP_LogPrintf("d_EMVCL_SID_JCB_WAVE_2");
			break;
		case d_EMVCL_SID_JCB_WAVE_QVSDC:
			inDISP_LogPrintf("d_EMVCL_SID_JCB_WAVE_QVSDC");
			break;
		case d_EMVCL_SID_JCB_MSD:
			inDISP_LogPrintf("d_EMVCL_SID_JCB_MSD");
			break;
		case d_EMVCL_SID_JCB_LEGACY:
			inDISP_LogPrintf("d_EMVCL_SID_JCB_LEGACY");
			break;
		case d_EMVCL_SID_JCB_EMV:
			inDISP_LogPrintf("d_EMVCL_SID_JCB_EMV");
			break;
		case d_EMVCL_SID_CUP_QPBOC:
			inDISP_LogPrintf("d_EMVCL_SID_CUP_QPBOC");
			break;
		case SCHEME_ID_90_SMARTPAY:
			inDISP_LogPrintf("SCHEME_ID_90_SMARTPAY");
			break;
		case d_EMVCL_SID_AE_EMV:
			inDISP_LogPrintf("d_EMVCL_SID_AE_EMV");
			break;
		case d_EMVCL_SID_AE_MAG_STRIPE:
			inDISP_LogPrintf("d_EMVCL_SID_AE_MAG_STRIPE");
			break;
		case d_EMVCL_SID_DISCOVER:
			inDISP_LogPrintf("d_EMVCL_SID_DISCOVER");
			break;
		case d_EMVCL_SID_DISCOVER_DPAS:
			inDISP_LogPrintf("d_EMVCL_SID_DISCOVER_DPAS");
			break;
		case d_EMVCL_SID_INTERAC_FLASH:
			inDISP_LogPrintf("d_EMVCL_SID_INTERAC_FLASH");
			break;
		case d_EMVCL_SID_MEPS_MCCS:
			inDISP_LogPrintf("d_EMVCL_SID_MEPS_MCCS");
			break;
		default:
			inDISP_LogPrintf("???? Card");
			break;
		}
	}

	switch (srCtlsObj.uszSchemeID[0])
	{
	/* 原廠分一大堆Scheme ID 不支援的直接擋下 支援的存成要用的 */
	case d_EMVCL_SID_VISA_OLD_US:
	case d_EMVCL_SID_VISA_WAVE_2:
	case d_EMVCL_SID_VISA_WAVE_MSD:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_16_WAVE1;
		break;
	case d_EMVCL_SID_VISA_WAVE_QVSDC:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_17_WAVE3;
		break;
	case d_EMVCL_SID_PAYPASS_MAG_STRIPE:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_20_PAYPASS_MAG_STRIPE;
		break;
	case d_EMVCL_SID_PAYPASS_MCHIP:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_21_PAYPASS_MCHIP;
		break;
	case d_EMVCL_SID_JCB_WAVE_2:
	case d_EMVCL_SID_JCB_WAVE_QVSDC:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_61_JSPEEDY;
		break;
	case d_EMVCL_SID_JCB_MSD:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_64_NEWJSPEEDY_MSD;
		break;
	case d_EMVCL_SID_JCB_LEGACY:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_65_NEWJSPEEDY_LEGACY;
		break;
	case d_EMVCL_SID_JCB_EMV:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_63_NEWJSPEEDY_EMV;
		break;
	case d_EMVCL_SID_AE_EMV:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_50_EXPRESSSPAY;
		break;
	case d_EMVCL_SID_AE_MAG_STRIPE:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE;
		break;
	case d_EMVCL_SID_DISCOVER_DPAS:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_42_DPAS;
		break;
	case d_EMVCL_SID_CUP_QPBOC:
		srCtlsObj.uszSchemeID[0] = SCHEME_ID_91_QUICKPASS;

		/* 若銀聯功能沒開，擋掉 */
		/* 若EDC的CUPFunctionEnable 和 MACEnable 未開，顯示此功能以關閉 */
		memset(szCUPFunctionEnable, 0x00, sizeof(szCUPFunctionEnable));
		inGetCUPFuncEnable(szCUPFunctionEnable);
		memset(szMACEnable, 0x00, sizeof(szMACEnable));
		inGetMACEnable(szMACEnable);

		/* 沒開CUP */
		if ((memcmp(&szCUPFunctionEnable[0], "Y", 1) != 0) || memcmp(szMACEnable, "Y", 1) != 0)
		{
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);					  /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜銀聯一般交易＞ */
			/* 此功能已關閉 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;

			return (VS_ERROR);
		}

		/* IDLE進入，一開始預設是_REFUND_ */
		if (pobTran->inTransactionCode == _REFUND_)
		{
			pobTran->inTransactionCode = _CUP_REFUND_;
			pobTran->srBRec.inCode = pobTran->inTransactionCode;
			pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
		}
		else if (pobTran->inTransactionCode == _PRE_COMP_)
		{
			pobTran->inTransactionCode = _CUP_PRE_COMP_;
			pobTran->srBRec.inCode = pobTran->inTransactionCode;
			pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
		}

		/* NCCC CUP同一個HOST */
		pobTran->srBRec.inHDTIndex = 0;
		if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" CLST Unpack Ready Refund Inter Load HDT[%d] *Error* Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
		}
		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" CLST Unpack Ready Refund Inter Load HDPT[%d] *Error* Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
		}

		pobTran->srBRec.uszCUPTransBit = VS_TRUE;

/* 因為不需要在這邊換KEY,所以拿掉 20190328 [SAM] */
#if 0
			/* 確認是銀聯卡，檢查是否已做安全認證 */
			/* 有開CUP且MACEnable有開但安全認證沒過，不能執行CUP交易 */
			if (inKMS_CheckKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_MAC_) != VS_SUCCESS)
			{
				if (inNCCC_Func_CUP_PowerOn_LogOn(pobTran) != VS_SUCCESS)
				{
					/* 安全認證失敗 */
					return (VS_ERROR);
				}
			}
#endif
		break;

	/* 目前不支援 */
	case d_EMVCL_SID_DISCOVER:
	case d_EMVCL_SID_INTERAC_FLASH:
	case d_EMVCL_SID_MEPS_MCCS:
	default:
		/* 不支援的scheme ID直接擋掉，之後組感應Tag也會錯誤 */
		if (ginDebug == VS_TRUE)
		{
			memset(szDebug, 0x00, sizeof(szDebug));
			sprintf(szDebug, "CardType No Def. SchemeID = 0x%02X", srCtlsObj.uszSchemeID[0]);
			inDISP_LogPrintf(szDebug);
		}

		return (VS_ERROR);
	}

	/* 將CTLSobj的schemID轉存到PobTran */
	pobTran->srBRec.uszWAVESchemeID = srCtlsObj.uszSchemeID[0];

	if (ginDebug == VS_TRUE)
	{
		memset(szDebug, 0x00, sizeof(szDebug));
		sprintf(szDebug, "uszSchemeID : %d", pobTran->srBRec.uszWAVESchemeID);
		inDISP_LogPrintf(szDebug);
	}

	if ((pobTran->srBRec.uszWAVESchemeID != SCHEME_ID_17_WAVE3) && (pobTran->srBRec.uszWAVESchemeID != SCHEME_ID_16_WAVE1))
	{
		memset(szASCII, 0x00, sizeof(szASCII));
		inFunc_BCD_to_ASCII(&szASCII[0], &szRCDataEx.baTrack2Data[0], szRCDataEx.bTrack2Len * 2);
		szRCDataEx.bTrack2Len = szRCDataEx.bTrack2Len * 2;

		memset(szRCDataEx.baTrack2Data, 0x00, sizeof(szRCDataEx.baTrack2Data));
		memcpy(&szRCDataEx.baTrack2Data[0], &szASCII[0], szRCDataEx.bTrack2Len);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("Track 1 Data :");
		memset(szDebug, 0x00, sizeof(szDebug));
		memcpy(szDebug, (char *)szRCDataEx.baTrack1Data, szRCDataEx.bTrack1Len);
		inDISP_LogPrintf(szDebug);
		memset(szDebug, 0x00, sizeof(szDebug));
		sprintf(szDebug, "Track 1 Len = %d", szRCDataEx.bTrack1Len);
		inDISP_LogPrintf(szDebug);

		inDISP_LogPrintf("Track 2 Data :");
		memset(szDebug, 0x00, sizeof(szDebug));
		memcpy(szDebug, (char *)szRCDataEx.baTrack2Data, szRCDataEx.bTrack2Len);
		inDISP_LogPrintf(szDebug);
		memset(szDebug, 0x00, sizeof(szDebug));
		sprintf(szDebug, "Track 2 Len = %d", szRCDataEx.bTrack2Len);
		inDISP_LogPrintf(szDebug);
	}

	memset(pobTran->szTrack1, 0x00, sizeof(pobTran->szTrack1));
	memset(pobTran->szTrack2, 0x00, sizeof(pobTran->szTrack2));

	/* 將讀到的Track123存到pobTran */
	pobTran->shTrack1Len = szRCDataEx.bTrack1Len;
	pobTran->shTrack2Len = szRCDataEx.bTrack2Len;

	memcpy(pobTran->szTrack1, szRCDataEx.baTrack1Data, szRCDataEx.bTrack1Len);

	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3 ||
		pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1)
	{
		/* 因為Track2第0個byte為";"，因此略過 */
		memcpy(pobTran->szTrack2, &szRCDataEx.baTrack2Data[1], szRCDataEx.bTrack2Len);
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
	{
		/* AE範例：3B3337343234353030313734313030373D3231303337303231353034313233343530303030303F */
		pobTran->shTrack2Len = (szRCDataEx.bTrack2Len - 2) / 2;
		inFunc_ASCII_to_BCD((unsigned char *)pobTran->szTrack2, (char *)&szRCDataEx.baTrack2Data[2], pobTran->shTrack2Len);
	}
	else
	{
		memcpy(pobTran->szTrack2, &szRCDataEx.baTrack2Data[0], szRCDataEx.bTrack2Len);
	}

	/* 這邊要修改檢查Track2 */
	for (i = 0; i < strlen(pobTran->szTrack2); i++)
	{
		/* F為結尾須刪除，<ETX>:"?" 必須去掉 */
		if (pobTran->szTrack2[i] == 'F' || pobTran->szTrack2[i] == '?')
		{
			pobTran->szTrack2[i] = 0;
			break;
		}

		if (pobTran->szTrack2[i] == 'D')
			pobTran->szTrack2[i] = '=';
	}

	/* 取卡號有效期 */
	if (strlen(pobTran->szTrack2) > 0)
	{
		inRetVal = inCARD_unPackCard(pobTran);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	else
	{
		return (VS_ERROR);
	}

	/* 組電文的EMV Data */
	inRetVal = inCTLS_ProcessChipData(pobTran);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 例外狀況 Tag檢核 感應限額等等 */
	inRetVal = inCTLS_ExceptionCheck_Refund(pobTran);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_UnPackReadyForRefund_Internal END");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_ExceptionCheck_Refund
Date&Time       :2017/6/28 下午 2:05
Describe        :檢查例外狀況
*/
int inCTLS_ExceptionCheck_Refund(TRANSACTION_OBJECT *pobTran)
{
	//	int		inRetVal;
	int inVWTIndex = -1;
	int inMVTIndex = -1;
	//        int             i;
	long lnCheckAmount = 0;
	char szASCII[256 + 1];
	char szTemplate[256 + 1];
	char szPayWaveAPID[32 + 1], szPayWaveAPIDContactlessTransactionLimit[12 + 1], szPayWaveAPIDCVMRequiredLimit[12 + 1];
	char szContactlessTransactionLimit[12 + 1], szCVMRequiredLimit[12 + 1], szTerminalCapabilities[6 + 1];
	//        char		szWAVEDate[15 + 1], szEDCDate[15 + 1];
	char szCardSupEnable[2 + 1];
	char szDebugMsg[100 + 1];
	VS_BOOL fCheckAPID = VS_FALSE;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_ExceptionCheck_Refund START!!");
	}

	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1 || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
	{
		inVWTIndex = _PAYWAVE_VWT_INDEX_;
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY)
	{
		inVWTIndex = _JSPEEDY_VWT_INDEX_;
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE)
	{
		inVWTIndex = _PAYPASS_VWT_INDEX_;
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS && !memcmp(&pobTran->srEMVRec.usz84_DF_NAME[0], "\xA0\x00\x00\x03\x33\x01\x01\x01", 8))
	{
		inVWTIndex = _QP_DEBIT_VWT_INDEX_;
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS && !memcmp(&pobTran->srEMVRec.usz84_DF_NAME[0], "\xA0\x00\x00\x03\x33\x01\x01\x02", 8))
	{
		inVWTIndex = _QP_CREDIT_VWT_INDEX_;
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS && !memcmp(&pobTran->srEMVRec.usz84_DF_NAME[0], "\xA0\x00\x00\x03\x33\x01\x01\x03", 8))
	{
		inVWTIndex = _QP_QUASI_CREDIT_VWT_INDEX_;
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
	{
		inVWTIndex = _AE_EXPRESSPAY_VWT_INDEX_;
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS)
	{
		inMVTIndex = _D_PAS_MVT_INDEX_;
		inVWTIndex = _D_PAS_VWT_INDEX_;
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("MVT 中無此感應卡");
		}
		return (VS_WAVE_ERROR); /* 表示記憶體有問題或是 VWT Table 不存在 */
	}

	if (inLoadVWTRec(inVWTIndex) < 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("VWT 中無此感應卡");
		}

		return (VS_WAVE_ERROR); /* 表示記憶體有問題或是 VWT Table 不存在 */
	}

	/* 先檢查是否TMS支援此卡別 */
	memset(szCardSupEnable, 0x00, sizeof(szCardSupEnable));
	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1 ||
		pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
	{
		inGetVISAPaywaveEnable(szCardSupEnable);
		if (memcmp(szCardSupEnable, "Y", strlen("Y")) != 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CFGT 不支援 VISA WAVE");
				inDISP_LogPrintf(szDebugMsg);
			}

			/* 不接受此感應卡 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC;

			return (VS_WAVE_ERROR);
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY)
	{
		inGetJCBJspeedyEnable(szCardSupEnable);
		if (memcmp(szCardSupEnable, "Y", strlen("Y")) != 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CFGT 不支援 JSPEEDY");
				inDISP_LogPrintf(szDebugMsg);
			}

			/* 不接受此感應卡 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC;

			return (VS_WAVE_ERROR);
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
	{
		inGetMCPaypassEnable(szCardSupEnable);
		if (memcmp(szCardSupEnable, "Y", strlen("Y")) != 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CFGT 不支援 MChip");
				inDISP_LogPrintf(szDebugMsg);
			}

			/* 不接受此感應卡 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC;

			return (VS_WAVE_ERROR);
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS)
	{
		inGetCUPContactlessEnable(szCardSupEnable);
		if (memcmp(szCardSupEnable, "Y", strlen("Y")) != 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CFGT 不支援 Quickpass");
				inDISP_LogPrintf(szDebugMsg);
			}

			/* 不接受此感應卡 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC;

			return (VS_WAVE_ERROR);
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
	{
		inGetAMEXContactlessEnable(szCardSupEnable);
		if (memcmp(szCardSupEnable, "Y", strlen("Y")) != 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CFGT 不支援 AE Contactless");
				inDISP_LogPrintf(szDebugMsg);
			}

			/* 不接受此感應卡 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC;

			return (VS_WAVE_ERROR);
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS)
	{
		inGetDFS_Contactless_Enable(szCardSupEnable);
		if (memcmp(szCardSupEnable, "Y", strlen("Y")) != 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CFGT 不支援 DFS Contactless");
				inDISP_LogPrintf(szDebugMsg);
			}

			/* 不接受此感應卡 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_06_TMS_NOT_SUPPORT_;

			return (VS_WAVE_ERROR);
		}
	}

	/* 先看感應限額(ContactlessTransactionLimit) */
	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1 || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
	{
		/* 【需求單104154】VCPS2.1.3規範，新增APID判定國內外限額需求 */
		/* Paywave 有APID，有自己獨立的Transaction Limti, CVM Limit, Floor Limit，所以這裡先看是不是APID */
		memset(szPayWaveAPID, 0x00, sizeof(szPayWaveAPID));
		inGetPayWaveAPID(szPayWaveAPID);

		if (pobTran->srEMVRec.in9F5A_Application_Program_IdentifierLen > 0 && strlen(szPayWaveAPID) > 0)
		{
			memset(szASCII, 0x00, sizeof(szASCII));
			inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F5A_Application_Program_Identifier[0], pobTran->srEMVRec.in9F5A_Application_Program_IdentifierLen);

			/* 比對APID */
			if (!memcmp(&szASCII[0], &szPayWaveAPID[0], strlen(szPayWaveAPID)))
			{
				/* 有比對到APID */
				fCheckAPID = VS_TRUE;
			}
			else
			{
				/* 沒有比對到APID */
				fCheckAPID = VS_FALSE;
			}
		}

		/* 有比對到APID */
		if (fCheckAPID == VS_TRUE)
		{
			/* 檢核感應限額 */
			memset(szPayWaveAPIDContactlessTransactionLimit, 0x00, sizeof(szPayWaveAPIDContactlessTransactionLimit));
			memset(szTemplate, 0x00, sizeof(szTemplate));

			inGetPayWaveAPIDContactlessTransactionLimit(szTemplate);
			memcpy(szPayWaveAPIDContactlessTransactionLimit, szTemplate, 10);

			lnCheckAmount = atol(szPayWaveAPIDContactlessTransactionLimit);

			if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			{
				/* 超過感應限額 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_OVER_AMOUNT_;

				return (VS_WAVE_AMOUNT_ERR);
			}
		}
		/* 沒有比對到APID */
		else if (fCheckAPID == VS_FALSE)
		{
			memset(szContactlessTransactionLimit, 0x00, sizeof(szContactlessTransactionLimit));
			memset(szTemplate, 0x00, sizeof(szTemplate));

			inGetContactlessTransactionLimit(szTemplate);
			memcpy(szContactlessTransactionLimit, szTemplate, 10);

			/* 第一步檢核感應限額 */
			lnCheckAmount = atol(szContactlessTransactionLimit);

			if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			{
				/* 超過感應限額 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_OVER_AMOUNT_;

				return (VS_WAVE_AMOUNT_ERR);
			}
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS)
	{
		memset(szContactlessTransactionLimit, 0x00, sizeof(szContactlessTransactionLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetContactlessTransactionLimit(szTemplate);
		memcpy(szContactlessTransactionLimit, szTemplate, 10);

		lnCheckAmount = atol(szContactlessTransactionLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
		{
			/* 超過感應限額 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_OVER_AMOUNT_;
			return (VS_WAVE_AMOUNT_ERR);
		}
	}

	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE ||
		pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
	{
		/* VWT中ContactlessTransactionLimit 不For PayWave 使用 */
	}

	/* 確認CID，虹堡系列都會吐DF8F4F 但為了簡潔，直接看CID來看online offline */
	/*【PayWave3】感應卡 */
	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
	{
		if (pobTran->srEMVRec.in9F27_CIDLen <= 0)
		{
			/* 感應資料不足 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_CTLS_DATA_SHORT_;
			return (VS_WAVE_ERROR);
		}
		else
		{
			if (pobTran->srEMVRec.usz9F27_CID[0] == 0x40)
			{
				/* 感應退貨不能塞Y1 */
			}
			else if (pobTran->srEMVRec.usz9F27_CID[0] == 0x80)
			{
				/* ARQC */
				/* 要 Online */
			}
			else
			{
				/* AAC */
				/* 拒絕交易 Z1 */
				/* 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_Z1_;
				return (VS_WAVE_ERROR);
			}
		}

		/* (6)	PayWave 3卡片感應交易，若卡片未回傳 PAN_SEQ_NUM 時，端末機應以預設值 “00” (Hard code) 上傳。 */
		if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen <= 0)
		{
			pobTran->srEMVRec.in5F34_ApplPanSeqnumLen = 1;
			memset(pobTran->srEMVRec.usz5F34_ApplPanSeqnum, 0x00, sizeof(pobTran->srEMVRec.usz5F34_ApplPanSeqnum));
			pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0] = 0x00;
		}
	}
	/* 【Newjspeedy】感應卡 */
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY)
	{
		if (pobTran->srEMVRec.in9F27_CIDLen > 0)
		{
			if (pobTran->srEMVRec.usz9F27_CID[0] == 0x40)
			{
				/* 感應退貨不能塞Y1 */
			}
			else if (pobTran->srEMVRec.usz9F27_CID[0] == 0x80)
			{
				/* ARQC */
				/* 要 Online */
			}
			else
			{
				/* AAC */
				/* 拒絕交易 Z1 */
				/* 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_Z1_;
				return (VS_WAVE_ERROR);
			}
		}

		/* (6)	Newjspeedy卡片感應交易，若卡片未回傳 PAN_SEQ_NUM 時，端末機應以預設值 “00” (Hard code) 上傳。 */
		if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen <= 0)
		{
			pobTran->srEMVRec.in5F34_ApplPanSeqnumLen = 1;
			memset(pobTran->srEMVRec.usz5F34_ApplPanSeqnum, 0x00, sizeof(pobTran->srEMVRec.usz5F34_ApplPanSeqnum));
			pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0] = 0x00;
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS)
	{
		if (pobTran->srEMVRec.in9F27_CIDLen <= 0)
		{
			/* 感應資料不足 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_CTLS_DATA_SHORT_;
			return (VS_WAVE_ERROR);
		}
		else
		{
			if (pobTran->srEMVRec.usz9F27_CID[0] == 0x40)
			{
				/* 感應退貨不能塞Y1 */
			}
			else if (pobTran->srEMVRec.usz9F27_CID[0] == 0x80)
			{
				/* ARQC */
				/* 要 Online */
			}
			else
			{
				/* AAC */
				/* 拒絕交易 Z1 */
				/* 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_Z1_;
				return (VS_WAVE_ERROR);
			}
		}
	}
	/* 原先於inCTLS_ProcessChipData()時塞Y1，現在統一拉下來塞 */
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
	{
		if (pobTran->srEMVRec.in9F27_CIDLen > 0)
		{
			if (pobTran->srEMVRec.usz9F27_CID[0] == 0x40)
			{
				/* 感應退貨不能塞Y1 */
			}
			else if (pobTran->srEMVRec.usz9F27_CID[0] == 0x80)
			{
				/* ARQC */
				/* 要 Online */
			}
			else
			{
				/* AAC */
				/* 拒絕交易 Z1 */
				/* 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_Z1_;
				return (VS_WAVE_ERROR);
			}
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
	{
		if (pobTran->srEMVRec.in9F27_CIDLen <= 0)
		{
			/* 感應資料不足 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_CTLS_DATA_SHORT_;
			return (VS_WAVE_ERROR);
		}
		else
		{
			if (pobTran->srEMVRec.usz9F27_CID[0] == 0x40)
			{
				/* 感應退貨不能塞Y1 */
			}
			else if (pobTran->srEMVRec.usz9F27_CID[0] == 0x80)
			{
				/* ARQC */
				/* 要 Online */
			}
			else
			{
				/* AAC */
				/* 拒絕交易 Z1 */
				/* 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_Z1_;
				return (VS_WAVE_ERROR);
			}
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS)
	{
		if (pobTran->srEMVRec.in9F27_CIDLen <= 0)
		{
			/* 感應資料不足 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_CTLS_DATA_SHORT_;
			return (VS_WAVE_ERROR);
		}
		else
		{
			if (pobTran->srEMVRec.usz9F27_CID[0] == 0x40)
			{
				/* TC */
				memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
				strcpy(pobTran->srBRec.szAuthCode, "Y1");
			}
			else if (pobTran->srEMVRec.usz9F27_CID[0] == 0x80)
			{
				/* ARQC */
				/* 要 Online */
			}
			else
			{
				/* AAC */
				/* 拒絕交易 Z1 */
				/* 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_Z1_;
				return (VS_WAVE_ERROR);
			}
		}
	}

	/* 處理狀況一：如果是 VISA Paywave1 & VISA Paywave3 & Jspeedy 要從【mvt.txt】組【9F33】 */
	/* R-FES EDC Specifications v1.49.doc 規格
		1.	Contactless 交易上傳之 Tag 9F33 的 Value 規則如下：
		(1)	VISA Paywave 1/VISA Paywave 3/JSpeedy 請上傳Terminal 之 EMV 參數（因為 V & J 感應交易Reader 不會回 Tag 9F33）。
		(2)	Paypass 因為 Reader 會回覆 Tag 9F33 ，所以依Reader 回覆資料上傳。
	*/
	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1 ||
		pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3 ||
		pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY ||
		pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV ||
		pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD ||
		pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY)
	{
		if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1 ||
			pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
		{
			inMVTIndex = _VISA_MVT_INDEX_;
		}
		else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY ||
				 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV ||
				 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD ||
				 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY)
		{
			inMVTIndex = _JSMART_MVT_INDEX_;
		}

		if (inLoadMVTRec(inMVTIndex) < 0)
		{
			return (VS_WAVE_ERROR);
		}

		/* 有9F33直接用，沒有就從MVT抓 */
		if (pobTran->srEMVRec.in9F33_TermCapabilitiesLen > 0)
		{
		}
		else
		{
			inGetTerminalCapabilities(szTerminalCapabilities);
			memset(pobTran->srEMVRec.usz9F33_TermCapabilities, 0x00, sizeof(pobTran->srEMVRec.usz9F33_TermCapabilities));
			pobTran->srEMVRec.in9F33_TermCapabilitiesLen = ((strlen(szTerminalCapabilities) + 1) / 2);
			inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9F33_TermCapabilities[0], &szTerminalCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
		}
	}

	/* 處理狀況二：所有感應交易要組【DFEE】【DEEF】 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEE_TERMINAL_ENTRY_CAPABILITY, (unsigned char *)&szTemplate[0]) > 0)
	{
		pobTran->srEMVRec.inDFEE_TerEntryCapLen = szTemplate[2];
		memcpy((char *)&pobTran->srEMVRec.uszDFEE_TerEntryCap[0], &szTemplate[3], pobTran->srEMVRec.inDFEE_TerEntryCapLen);
	}

	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEF_REASON_ONLINE_CODE, (unsigned char *)&szTemplate[0]) > 0)
	{
		pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen = szTemplate[2];
		memcpy((char *)&pobTran->srEMVRec.uszDFEF_ReasonOnlineCode[0], &szTemplate[3], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);
	}

	/* 免簽名判斷(CVM Limit) */
	/* 先預設為免簽名 */
	pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
	/* 看9F34 */
	if (pobTran->srEMVRec.in9F34_CVMLen > 0)
	{
		if (pobTran->srEMVRec.usz9F34_CVM[0] == 0x1F)
		{
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
		}
		else
		{
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		}
	}
	else
	{
		/* 先讀CVM Limit */
		memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetCVMRequiredLimit(szTemplate);
		memcpy(szCVMRequiredLimit, szTemplate, 10); /* 小數點後兩位不用 */

		lnCheckAmount = atol(szCVMRequiredLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		else
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
	}

	/* 之前發生過9F34失效，所以下面根據金額再確認一次 */
	/* VISA */
	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1 || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
	{
		if (fCheckAPID == VS_TRUE)
		{
			memset(szPayWaveAPIDCVMRequiredLimit, 0x00, sizeof(szPayWaveAPIDCVMRequiredLimit));
			memset(szTemplate, 0x00, sizeof(szTemplate));

			inGetPayWaveAPIDCVMRequiredLimit(szTemplate);
			memcpy(szPayWaveAPIDCVMRequiredLimit, szTemplate, 10);

			/* 第二步檢核免簽名【VISA Paywave1】卡片要判斷是否免簽名 */
			if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1)
			{
				lnCheckAmount = atol(szPayWaveAPIDCVMRequiredLimit);
				if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
					pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
				else
					pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
			}

			/* 第三步檢核免簽名【VISA Paywave3】卡片要判斷是否免簽名 */
			if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
			{
				if (pobTran->uszPayWave3Tag55Bit == VS_TRUE && pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
				{
					/* 表示已經判斷過【TAG 55】要在判斷一次是否免簽,若要簽名就不用在判斷一次 */
					lnCheckAmount = atol(szPayWaveAPIDCVMRequiredLimit);
					if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
						pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
					else
						pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
				}
				else if (pobTran->uszPayWave3Tag55Bit == VS_FALSE)
				{
					/* 沒有回【TAG 55】 */
					lnCheckAmount = atol(szPayWaveAPIDCVMRequiredLimit);
					if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
						pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
					else
						pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
				}
			}
		}
		else
		{
			/* 先讀CVM Limit */
			memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
			memset(szTemplate, 0x00, sizeof(szTemplate));

			inGetCVMRequiredLimit(szTemplate);
			memcpy(szCVMRequiredLimit, szTemplate, 10); /* 小數點後兩位不用 */

			/* 第二步檢核免簽名【VISA Paywave1】卡片要判斷是否免簽名 */
			if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1)
			{
				lnCheckAmount = atol(szCVMRequiredLimit);
				if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
					pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
				else
					pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
			}

			/* 第三步檢核免簽名【VISA Paywave3】卡片要判斷是否免簽名 */
			if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
			{
				if (pobTran->uszPayWave3Tag55Bit == VS_TRUE && pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
				{
					/* 表示已經判斷過【TAG 55】要在判斷一次 */
					lnCheckAmount = atol(szCVMRequiredLimit);
					if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
						pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
					else
						pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
				}
				else if (pobTran->uszPayWave3Tag55Bit == VS_FALSE)
				{
					/* 沒有回【TAG 55】 */
					lnCheckAmount = atol(szCVMRequiredLimit);
					if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
						pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
					else
						pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
				}
			}
		}
	}
	/* 【PAYpass】卡片要判斷是否免簽名 */
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
	{
		memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetPaypassCVMRequiredLimit(szTemplate);
		memcpy(szCVMRequiredLimit, szTemplate, 10);

		lnCheckAmount = atol(szCVMRequiredLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		else
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
	}
	/* 【JSPEEDY】卡片要判斷是否免簽名 */
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY)
	{
		memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetPaypassCVMRequiredLimit(szTemplate);
		memcpy(szCVMRequiredLimit, szTemplate, 10);

		lnCheckAmount = atol(szCVMRequiredLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		else
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
	}
	/* QUICKPASS */
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS)
	{
		/* 判斷【QuickPass】卡片要判斷是否免簽名(PaypassCVMRequiredLimit 由Paypass、NewJ、QuickPass共用) */
		memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		/* QuickPass此欄位只用來看是否免簽名 */
		inGetPaypassCVMRequiredLimit(szTemplate);
		memcpy(szCVMRequiredLimit, szTemplate, 10);

		lnCheckAmount = atol(szCVMRequiredLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		else
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
	}
	/* AE */
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
	{
		memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetPaypassCVMRequiredLimit(szTemplate);
		memcpy(szCVMRequiredLimit, szTemplate, 10);

		lnCheckAmount = atol(szCVMRequiredLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		else
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS)
	{
		memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetPaypassCVMRequiredLimit(szTemplate);
		memcpy(szCVMRequiredLimit, szTemplate, 10);

		lnCheckAmount = atol(szCVMRequiredLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		else
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
	}

	/* 處理狀況五：如果是授權碼是【Y1】要送【Tag 8A】 */
	if (!memcmp(&pobTran->srBRec.szAuthCode[0], "Y1", 2))
	{
		memset(pobTran->srEMVRec.usz8A_AuthRespCode, 0x00, sizeof(pobTran->srEMVRec.usz8A_AuthRespCode));
		pobTran->srEMVRec.in8A_AuthRespCodeLen = 2;
		memcpy(&pobTran->srEMVRec.usz8A_AuthRespCode[0], "Y1", 2);
	}

	/* 處理狀況六：存卡片有效期 */
	if (strlen(pobTran->srBRec.szExpDate) == 0)
	{
		if (pobTran->srEMVRec.in5F24_ExpireDateLen != 0)
		{
			inFunc_BCD_to_ASCII(&pobTran->srBRec.szExpDate[0], &pobTran->srEMVRec.usz5F24_ExpireDate[0], pobTran->srEMVRec.in5F24_ExpireDateLen);
			pobTran->srBRec.szExpDate[4] = 0x00;
		}
	}

	/* 處理狀況七：卡片沒有回傳Tag9F03 要自己塞長度6及資料填0 */
	if (pobTran->srEMVRec.in9F03_AmtOtherNumLen <= 0)
	{
		pobTran->srEMVRec.in9F03_AmtOtherNumLen = 0x06;
		memset(pobTran->srEMVRec.usz9F03_AmtOtherNum, 0x00, sizeof(pobTran->srEMVRec.usz9F03_AmtOtherNum));
	}

	/* 處理狀況八：感應wave 3的卡片後 組封包上傳主機時不要帶上 Tag 9F35 */
	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
	{
		pobTran->srEMVRec.in9F35_TermTypeLen = 0x00;
		memset(pobTran->srEMVRec.usz9F35_TermType, 0x00, sizeof(pobTran->srEMVRec.usz9F35_TermType));
	}

	/* 處理QuickPass感應卡是否要輸入密碼 */
	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS)
	{
		/* 感應退貨不用輸入密碼，刪除掉 */

		/* 再判斷一次【QuickPass】卡片要判斷是否免簽名 */
		memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetPaypassCVMRequiredLimit(szTemplate);
		memcpy(szCVMRequiredLimit, szTemplate, 10);

		lnCheckAmount = atol(szCVMRequiredLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		else
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
	}
	else
	{
		/* 指定為銀聯交易，但是非銀聯感應閃付要檔 */
		/* 2015-09-16 AM 10:13:39 Add By Tim HappyGo流程要支援閃付 */
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			/* 不接受此感應卡 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC;

			return (VS_WAVE_INVALID_SCHEME_ERR);
		}
	}

	/** 感應退貨一定要簽名 **/
	pobTran->srBRec.uszNoSignatureBit = VS_FALSE;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_ExceptionCheck_Refund END!!");
	}

	return (VS_SUCCESS);
}

/*
Function        :vdCTLS_EVENT_EMVCL_PRE_NON_EMV_CARD
Date&Time       :2017/1/18 下午 2:45
Describe        :判斷非EMV卡使用 SmartPay感應
*/
void vdCTLS_EVENT_EMVCL_PRE_NON_EMV_CARD(OUT BYTE *pbNonEMVCard)
{
	//	int	inRetVal = VS_ERROR;
	//	int	inFinalDecision = 0;	/* 1 = EMV，2 = FISC */

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("vdCTLS_EVENT_EMVCL_PRE_NON_EMV_CARD START!!");

		// When this event is triggered, it means the EMVCL kernel can not identify
		// the card tapped on the filed.
		// This card might be a specific card for local market.

		// User can implement the proprietary porcess for the local specific card here.

#if 0 /* 富邦沒有金融卡，所以先拿掉 20190305 [SAM] */
	/* V3UL才出選單 */
	if (ginMachineType == _CASTLE_TYPE_V3UL_)
	{
		/* 加這個條件是因為，若選信用卡會Timeout，所以保留選項，第二進來就直接選 */
		/* 否則會無限迴圈 */
		if (pobEmvTran.uszCreditBit == VS_FALSE &&
		    pobEmvTran.uszUICCBit == VS_FALSE	&&
		    pobEmvTran.uszFISCBit == VS_FALSE)
		{
			inEMV_CreditSelectAID(&pobEmvTran, _CONTACT_TYPE_01_CONTACTLESS_);
			inFISC_SelectAID_CTLS(&pobEmvTran);
		}

		/* 兩種都有，出選單選哪一種卡 */
		if ((pobEmvTran.uszCreditBit == VS_TRUE ||
		     pobEmvTran.uszUICCBit == VS_TRUE)	&&
		     pobEmvTran.uszFISCBit == VS_TRUE)
		{
			/* 如果偏好信用卡 */
			if (pobEmvTran.uszECRPreferCreditBit == VS_TRUE)
			{
				pobEmvTran.uszCreditBit = VS_TRUE;
				pobEmvTran.uszFISCBit = VS_FALSE;
				pobEmvTran.uszUICCBit = VS_FALSE;
			}
			/* 如果偏好金融卡 */
			else if (pobEmvTran.uszECRPreferFiscBit == VS_TRUE)
			{
				pobEmvTran.uszCreditBit = VS_FALSE;
				pobEmvTran.uszFISCBit = VS_TRUE;
				pobEmvTran.uszUICCBit = VS_FALSE;
			}
			/* 如果沒特別偏好，出選單 */
			else
			{
				do
				{
					inDISP_BEEP(1, 1000);
					inRetVal = inFISC_Select_Menu(&pobEmvTran);
					if (inRetVal == VS_SUCCESS)
					{
						/* 本來考慮在若是顯示"請放回感應卡"要清除字樣 */
						/* 因為V3UL顯示太慢，所以要不能一直顯示 */
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						inDISP_PutGraphic(_ERR_PLS_PUT_BACK_CTLS_, 0, _COORDINATE_Y_LINE_8_6_);
						while (1)
						{
							/* 沿用感應的Timeout */
							if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
							{
								break;
							}

							if (inCTLS_IsCard_Still_Exist() == VS_SUCCESS)
							{
								break;
							}
						}

						/* 一定會超時，如果是金融卡要額外重新啟動，信用卡虹堡lib會自己重新啟動 */
						inCTLS_ReActive_TypeACard();
					}
				} while(inRetVal == VS_USER_CANCEL);
			}

			/* 最後選擇 */
			if (pobEmvTran.uszCreditBit == VS_TRUE)
			{
				inFinalDecision = 1;
			}
			else if (pobEmvTran.uszFISCBit == VS_TRUE)
			{
				inFinalDecision = 2;
			}
			else
			{
				inFinalDecision = 1;
			}

		}
		else if ((pobEmvTran.uszCreditBit == VS_FALSE	&&
			  pobEmvTran.uszUICCBit == VS_FALSE)	&&
			  pobEmvTran.uszFISCBit == VS_TRUE)
		{
			inFinalDecision = 2;
		}
		else if ((pobEmvTran.uszCreditBit == VS_TRUE	||
		          pobEmvTran.uszUICCBit == VS_TRUE)	&&
			  pobEmvTran.uszFISCBit == VS_FALSE)
		{
			inFinalDecision = 1;
		}
		else
		{
			inFinalDecision = 1;
		}
	}
	else
	{
		/* 如果選了FISC的交易別，一定走FISC*/
		if (pobEmvTran.srBRec.uszFiscTransBit == VS_TRUE)
		{
			inFinalDecision = 2;
		}
		/* 否則確認是否有FISC AID */
		else
		{
			inFISC_SelectAID_CTLS(&pobEmvTran);
			if(pobEmvTran.uszFISCBit == VS_TRUE)
			{
				inFinalDecision = 2;
			}
			else
			{
				inFinalDecision = 1;
			}
		}
	}

	if (inFinalDecision == 2)
	{
                if (ginDebug == VS_TRUE)
                        inDISP_LogPrintf("SmartPay CTLS FLOW!!");
		//If the proprietary process was performed successfully, please set *pbNonEMVCard = 0x01
		//to inform EMVCL kernel Non-EMV Card was detected.
                *pbNonEMVCard = 0x01;
                /* 跑SmartPay流程 */
		//EMVCL kernel will not change the UI such like sound, led shown and display
		//User can implement the UI according to their proprietary process.

		if (ginMachineType == _CASTLE_TYPE_V3UL_)
		{
				inRetVal = inMultiFunc_FISC_CTLSProcess(&pobEmvTran);
		}
		else
		{
				inRetVal = inFISC_CTLSProcess(&pobEmvTran);
		}

	}
	else
#endif
	{
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("EMV CTLS FLOW!!");
		// Otherwise, please set *pbNonEMVCard = 0x00
		/* 跑原本EMV流程 */
		*pbNonEMVCard = 0x00;
		// EMVCL kernel will go on with its original process
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("vdCTLS_EVENT_EMVCL_PRE_NON_EMV_CARD() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
}

/*
Function        :inCTLS_Set_LED
Date&Time       :2017/12/22 下午 3:48
Describe        :
*/
int inCTLS_Set_LED(int inColor)
{
	char szDebugMsg[100 + 1];

	if (guszCTLSInitiOK != VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			/* 未初始化CTLS 設定LED會crash */
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "未初始化CTLS 不能設LED");
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_SUCCESS);
	}

	switch (inColor)
	{
	case _CTLS_LIGHT_NONE_:
		EMVCL_SetLED(0x0F, 0x00);
		break;
	case _CTLS_LIGHT_BLUE_:
		EMVCL_SetLED(0x0F, 0x08);
		break;
	case _CTLS_LIGHT_YELLOW_:
		EMVCL_SetLED(0x0F, 0x04);
		break;
	case _CTLS_LIGHT_GREEN_:
		EMVCL_SetLED(0x0F, 0x02);
		break;
	case _CTLS_LIGHT_RED_:
		EMVCL_SetLED(0x0F, 0x01);
		break;
	default:
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_Clear_LED
Date&Time       :2017/5/4 上午 11:53
Describe        :將LED燈清空，主要for MP200 或 UL 這種有實體燈的，若不清會卡燈號
*/
int inCTLS_Clear_LED()
{
	char szContactlessEnable[2 + 1];
	char szContactlessReaderMode[2 + 1];
	char szDebugMsg[100 + 1];

	memset(szContactlessEnable, 0x00, sizeof(szContactlessEnable));
	memset(szContactlessReaderMode, 0x00, sizeof(szContactlessReaderMode));

	inGetContactlessEnable(szContactlessEnable);
	inGetContactlessReaderMode(szContactlessReaderMode);

	if (memcmp(szContactlessEnable, "Y", strlen("Y")) != 0 || memcmp(szContactlessReaderMode, "0", strlen("0")) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Contactless Not Open");
			inDISP_LogPrintf(szDebugMsg);
		}

		return (VS_ERROR);
	}

	/* 有實體燈的才要SET */
#ifdef _HALF_LCD_
	inCTLS_Set_LED(_CTLS_LIGHT_NONE_);
#else

#endif

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_Decide_Display_Image
Date&Time       :2017/9/12 下午 5:45
Describe        :
*/
int inCTLS_Decide_Display_Image(TRANSACTION_OBJECT *pobTran)
{
	char szFunEnable[2 + 1];
	inDISP_ClearAll();

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCTLS_Decide_Display_Image Start !");
	}

	memset(szFunEnable, 0x00, sizeof(szFunEnable));
	inGetStore_Stub_CardNo_Truncate_Enable(szFunEnable);

	/* 暫時，待討論，20230111 Miyano */
	if (vbCheckCostcoCustom(Costco_New))
	{
		inCostco_Decide_Display_Image(pobTran);
		return (VS_SUCCESS);
	}

	/* 是否是票證交易 */
	if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
	{
		if (pobTran->srTRec.inCode == _TICKET_DEDUCT_)
		{
			inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜購貨交易＞ */
			inDISP_PutGraphic(_CTLS_ESVC_, 0, _COORDINATE_Y_LINE_8_3_);				   /* 請將票卡放置感應區 */
		}
		else if (pobTran->srTRec.inCode == _TICKET_REFUND_)
		{
			inDISP_PutGraphic(_MENU_TICKET_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜退貨交易＞ */
			inDISP_PutGraphic(_CTLS_ESVC_, 0, _COORDINATE_Y_LINE_8_3_);				   /* 請將票卡放置感應區 */
		}
		else if (pobTran->srTRec.inCode == _TICKET_INQUIRY_)
		{
			inDISP_PutGraphic(_MENU_TICKET_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜餘額查詢＞ */
			inDISP_PutGraphic(_CTLS_ESVC_, 0, _COORDINATE_Y_LINE_8_3_);					/* 請將票卡放置感應區 */
		}
		else if (pobTran->srTRec.inCode == _TICKET_TOP_UP_)
		{
			inDISP_PutGraphic(_MENU_TICKET_TOP_UP_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜加值交易＞ */
			inDISP_PutGraphic(_CTLS_ESVC_, 0, _COORDINATE_Y_LINE_8_3_);				   /* 請將票卡放置感應區 */
		}
		else if (pobTran->srTRec.inCode == _TICKET_VOID_TOP_UP_)
		{
			inDISP_PutGraphic(_MENU_TICKET_VOID_TOP_UP_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜加值取消＞ */
			inDISP_PutGraphic(_CTLS_ESVC_, 0, _COORDINATE_Y_LINE_8_3_);						/* 請將票卡放置感應區 */
		}
		/* [新增電票悠遊卡功能]  新增顯示圖片條件 [SAM] 2022/6/21 下午 6:25 START */
		else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_)
		{
			inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜1.購貨交易＞ */
			inDISP_PutGraphic(_CTLS_ESVC_, 0, _COORDINATE_Y_LINE_8_3_);				   /* 請將票卡放置感應區 */
		}
		else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_)
		{
			inDISP_PutGraphic(_MENU_TICKET_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜2.退貨交易＞ */
			inDISP_PutGraphic(_CTLS_ESVC_, 0, _COORDINATE_Y_LINE_8_3_);				   /* 請將票卡放置感應區 */
		}
		else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_)
		{
			inDISP_PutGraphic(_MENU_TICKET_TOP_UP_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜3.加值交易＞ */
			inDISP_PutGraphic(_CTLS_ESVC_, 0, _COORDINATE_Y_LINE_8_3_);				   /* 請將票卡放置感應區 */
		}
		else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_)
		{
			inDISP_PutGraphic(_MENU_TICKET_VOID_TOP_UP_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜4.取消加值＞ */
			inDISP_PutGraphic(_CTLS_ESVC_, 0, _COORDINATE_Y_LINE_8_3_);						/* 請將票卡放置感應區 */
		}
		else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_)
		{
			inDISP_PutGraphic(_MENU_TICKET_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜6.餘額查詢＞ */
			inDISP_PutGraphic(_CTLS_ESVC_, 0, _COORDINATE_Y_LINE_8_3_);					/* 請將票卡放置感應區 */
		}
		/* [新增電票悠遊卡功能]  新增顯示圖片條件 [SAM] 2022/6/21 下午 6:25 END */
	}
	else
	{
		/* 是否要顯示人工輸入卡號 */
		if (memcmp(szFunEnable, "Y", strlen("Y")) == 0)
		{
			/* 顯示 請刷銀聯卡或感應 */
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				if (pobTran->srBRec.lnHGTransactionType != 0)
				{
					if (pobTran->srBRec.lnHGTransactionType == _HG_REWARD_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡紅利積點＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_);		/* 請刷卡、插卡或感應銀聯卡 */
					}
					else if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_ONLINE_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數抵扣＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_);			   /* 請刷卡、插卡或感應銀聯卡 */
					}
					else if (pobTran->srBRec.lnHGTransactionType == _HG_POINT_CERTAIN_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_POINT_CERTAIN_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡加價購＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_);			   /* 請刷卡、插卡或感應銀聯卡 */
					}
					else if (pobTran->srBRec.lnHGTransactionType == _HG_FULL_REDEMPTION_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_REDEMPTION_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數兌換＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_);			/* 請刷卡、插卡或感應銀聯卡 */
					}
					else if (pobTran->srBRec.lnHGTransactionType == _HG_INQUIRY_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數查詢＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_);		 /* 請刷卡、插卡或感應銀聯卡 */
					}
					else if (pobTran->srBRec.inCode == _REDEEM_SALE_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡紅利扣抵＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_);			   /* 請刷卡、插卡或感應銀聯卡 */
					}
					else if (pobTran->srBRec.inCode == _INST_SALE_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_INS_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡分期付款＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_);			/* 請刷卡、插卡或感應銀聯卡 */
					}
				}
				else
				{
					if (pobTran->srBRec.inCode == _CUP_SALE_)
					{
						inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯一般交易＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_);  /* 請刷卡、插卡或感應銀聯卡  */
					}
					else if (pobTran->srBRec.inCode == _CUP_REDEEM_SALE_)
					{
						inDISP_PutGraphic(_MENU_CUP_REDEEM_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯紅利扣抵＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_);		 /* 請刷卡、插卡或感應銀聯卡  */
					}
					else if (pobTran->srBRec.inCode == _CUP_INST_SALE_)
					{
						inDISP_PutGraphic(_MENU_CUP_INST_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯分期交易＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_);	   /* 請刷卡、插卡或感應銀聯卡  */
					}
					else if (pobTran->srBRec.inCode == _CUP_PRE_AUTH_)
					{
						inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜預先授權＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_);  /* 請刷卡、插卡或感應銀聯卡  */
					}
					else if (pobTran->srBRec.inCode == _CUP_PRE_COMP_)
					{
						inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜預先授權完成＞ */
						inDISP_PutGraphic(_CTLS_TX_CUPMEG_RF_, 0, _COORDINATE_Y_LINE_8_3_);	  /* 顯示 請刷銀聯卡或感應 人工輸入請按0 */
					}
					else if (pobTran->srBRec.inCode == _CUP_REFUND_)
					{
						inDISP_PutGraphic(_MENU_CUP_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯退貨＞ */
						inDISP_PutGraphic(_CTLS_TX_CUPMEG_RF_, 0, _COORDINATE_Y_LINE_8_3_);		/* 顯示 請刷銀聯卡或感應 人工輸入請按0 */
					}
					else if (pobTran->srBRec.inCode == _CUP_REDEEM_REFUND_)
					{
						inDISP_PutGraphic(_MENU_CUP_REDEEM_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯紅利退貨＞ */
						inDISP_PutGraphic(_CTLS_TX_CUPMEG_RF_, 0, _COORDINATE_Y_LINE_8_3_);			   /* 顯示 請刷銀聯卡或感應 人工輸入請按0 */
					}
					else if (pobTran->srBRec.inCode == _CUP_INST_REFUND_)
					{
						inDISP_PutGraphic(_MENU_CUP_INST_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯分期退貨＞ */
						inDISP_PutGraphic(_CTLS_TX_CUPMEG_RF_, 0, _COORDINATE_Y_LINE_8_3_);			 /* 顯示 請刷銀聯卡或感應 人工輸入請按0 */
					}
				}
			}
			/* 顯示 請刷卡或感應卡片 */
			else
			{
				if (pobTran->srBRec.lnHGTransactionType != 0)
				{
					if (pobTran->srBRec.lnHGTransactionType == _HG_REWARD_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡紅利積點＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);			/* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_ONLINE_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數抵扣＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);				   /* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.lnHGTransactionType == _HG_POINT_CERTAIN_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_POINT_CERTAIN_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡加價購＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);				   /* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.lnHGTransactionType == _HG_FULL_REDEMPTION_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_REDEMPTION_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數兌換＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);				/* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.lnHGTransactionType == _HG_INQUIRY_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數查詢＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);			 /* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.inCode == _REDEEM_SALE_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡紅利扣抵＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);				   /* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.inCode == _INST_SALE_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_INS_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡分期付款＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);				/* 請刷卡、插卡 或感應卡片 */
					}
				}
				else
				{
					if (pobTran->srBRec.inCode == _SALE_)
					{
						inDISP_PutGraphic(_MENU_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜一般交易＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.inCode == _SALE_OFFLINE_)
					{
						inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜補登交易＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);		  /* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.inCode == _REDEEM_SALE_)
					{
						inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜紅利扣抵＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);		 /* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.inCode == _INST_SALE_)
					{
						inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜分期交易＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);	   /* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.inCode == _PRE_AUTH_)
					{
						inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜預先授權＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);	  /* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.inCode == _PRE_COMP_)
					{
						inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜預先授權完成＞ */
						inDISP_PutGraphic(_CTLS_TX_MEG_RF_, 0, _COORDINATE_Y_LINE_8_3_);	  /* 顯示 請刷卡或感應卡片 人工輸入請按0 */
					}
					else if (pobTran->srBRec.inCode == _REFUND_)
					{
						inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜一般退貨＞ */
						inDISP_PutGraphic(_CTLS_TX_MEG_RF_, 0, _COORDINATE_Y_LINE_8_3_);	/* 顯示 請刷卡或感應卡片 人工輸入請按0 */
					}
					else if (pobTran->srBRec.inCode == _REDEEM_REFUND_)
					{
						inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜紅利退貨＞ */
						inDISP_PutGraphic(_CTLS_TX_MEG_RF_, 0, _COORDINATE_Y_LINE_8_3_);		   /* 顯示 請刷卡或感應卡片 人工輸入請按0 */
					}
					else if (pobTran->srBRec.inCode == _INST_REFUND_)
					{
						inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜分期退貨＞ */
						inDISP_PutGraphic(_CTLS_TX_MEG_RF_, 0, _COORDINATE_Y_LINE_8_3_);		 /* 顯示 請刷卡或感應卡片 人工輸入請按0 */
					}
					else if (pobTran->srBRec.inCode == _REDEEM_ADJUST_)
					{
						inDISP_PutGraphic(_MENU_REDEEM_ADJUST_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜紅利調帳＞ */
						inDISP_PutGraphic(_CTLS_TX_MEG_RF_, 0, _COORDINATE_Y_LINE_8_3_);		   /* 顯示 請刷卡或感應卡片 人工輸入請按0 */
					}
					else if (pobTran->srBRec.inCode == _INST_ADJUST_)
					{
						inDISP_PutGraphic(_MENU_INST_ADJUST_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜分期調帳＞ */
						inDISP_PutGraphic(_CTLS_TX_MEG_RF_, 0, _COORDINATE_Y_LINE_8_3_);		 /* 顯示 請刷卡或感應卡片 人工輸入請按0 */
					}
				}
			}
		}
		/* 不顯示人工輸入卡號 */
		else
		{
			/* 顯示 請刷銀聯卡或感應 */
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				if (pobTran->srBRec.lnHGTransactionType != 0)
				{
					if (pobTran->srBRec.lnHGTransactionType == _HG_REWARD_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡紅利積點＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_);		/* 請刷卡、插卡或感應銀聯卡 */
					}
					else if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_ONLINE_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數抵扣＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_);			   /* 請刷卡、插卡或感應銀聯卡 */
					}
					else if (pobTran->srBRec.lnHGTransactionType == _HG_POINT_CERTAIN_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_POINT_CERTAIN_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡加價購＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_);			   /* 請刷卡、插卡或感應銀聯卡 */
					}
					else if (pobTran->srBRec.lnHGTransactionType == _HG_FULL_REDEMPTION_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_REDEMPTION_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數兌換＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_);			/* 請刷卡、插卡或感應銀聯卡 */
					}
					else if (pobTran->srBRec.lnHGTransactionType == _HG_INQUIRY_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數查詢＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_);		 /* 請刷卡、插卡或感應銀聯卡 */
					}
					else if (pobTran->srBRec.inCode == _REDEEM_SALE_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡紅利扣抵＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_);			   /* 請刷卡、插卡或感應銀聯卡 */
					}
					else if (pobTran->srBRec.inCode == _INST_SALE_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_INS_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡分期付款＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_);			/* 請刷卡、插卡或感應銀聯卡 */
					}
				}
				else
				{
					if (pobTran->srBRec.inCode == _CUP_SALE_)
					{
						inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯一般交易＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_);  /* 請刷卡、插卡或感應銀聯卡  */
					}
					else if (pobTran->srBRec.inCode == _CUP_REDEEM_SALE_)
					{
						inDISP_PutGraphic(_MENU_CUP_REDEEM_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯紅利扣抵＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_);		 /* 請刷卡、插卡或感應銀聯卡  */
					}
					else if (pobTran->srBRec.inCode == _CUP_INST_SALE_)
					{
						inDISP_PutGraphic(_MENU_CUP_INST_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯分期交易＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_);	   /* 請刷卡、插卡或感應銀聯卡  */
					}
					else if (pobTran->srBRec.inCode == _CUP_PRE_AUTH_)
					{
						inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜預先授權＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_);  /* 請刷卡、插卡或感應銀聯卡  */
					}
					else if (pobTran->srBRec.inCode == _CUP_PRE_COMP_)
					{
						inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜預先授權完成＞ */
						inDISP_PutGraphic(_CTLS_CUPMEG_RF_, 0, _COORDINATE_Y_LINE_8_3_);	  /* 顯示 請刷銀聯卡或感應 */
					}
					else if (pobTran->srBRec.inCode == _CUP_REFUND_)
					{
						inDISP_PutGraphic(_MENU_CUP_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯退貨＞ */
						inDISP_PutGraphic(_CTLS_CUPMEG_RF_, 0, _COORDINATE_Y_LINE_8_3_);		/* 顯示 請刷銀聯卡或感應 */
					}
					else if (pobTran->srBRec.inCode == _CUP_REDEEM_REFUND_)
					{
						inDISP_PutGraphic(_MENU_CUP_REDEEM_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯紅利退貨＞ */
						inDISP_PutGraphic(_CTLS_CUPMEG_RF_, 0, _COORDINATE_Y_LINE_8_3_);			   /* 顯示 請刷銀聯卡或感應 */
					}
					else if (pobTran->srBRec.inCode == _CUP_INST_REFUND_)
					{
						inDISP_PutGraphic(_MENU_CUP_INST_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯分期退貨＞ */
						inDISP_PutGraphic(_CTLS_CUPMEG_RF_, 0, _COORDINATE_Y_LINE_8_3_);			 /* 顯示 請刷銀聯卡或感應 */
					}
				}
			}
			/* 顯示 請刷卡或感應卡片 */
			else
			{
				if (pobTran->srBRec.lnHGTransactionType != 0)
				{
					if (pobTran->srBRec.lnHGTransactionType == _HG_REWARD_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡紅利積點＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);			/* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_ONLINE_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數抵扣＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);				   /* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.lnHGTransactionType == _HG_POINT_CERTAIN_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_POINT_CERTAIN_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡加價購＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);				   /* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.lnHGTransactionType == _HG_FULL_REDEMPTION_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_REDEMPTION_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數兌換＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);				/* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.lnHGTransactionType == _HG_INQUIRY_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數查詢＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);			 /* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.lnHGTransactionType == _HG_REDEEM_REFUND_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_REDEEM_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡扣抵退貨＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);				   /* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.lnHGTransactionType == _HG_REWARD_REFUND_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡回饋退貨＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);				   /* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.inCode == _REDEEM_SALE_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡紅利扣抵＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);				   /* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.inCode == _INST_SALE_)
					{
						inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_INS_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡分期付款＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);				/* 請刷卡、插卡 或感應卡片 */
					}
				}
				else
				{
					if (pobTran->srBRec.inCode == _SALE_)
					{
						inDISP_PutGraphic(_MENU_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜一般交易＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.inCode == _SALE_OFFLINE_)
					{
						inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜補登交易＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);		  /* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.inCode == _REDEEM_SALE_)
					{
						inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜紅利扣抵＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);		 /* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.inCode == _INST_SALE_)
					{
						inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜分期交易＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);	   /* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.inCode == _PRE_AUTH_)
					{
						inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜預先授權＞ */
						inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);	  /* 請刷卡、插卡 或感應卡片 */
					}
					else if (pobTran->srBRec.inCode == _PRE_COMP_)
					{
						inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜預先授權完成＞ */
						inDISP_PutGraphic(_CTLS_MEG_RF_, 0, _COORDINATE_Y_LINE_8_3_);		  /* 顯示 請刷卡或感應卡片 */
					}
					else if (pobTran->srBRec.inCode == _REFUND_)
					{
						inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜一般退貨＞ */
						inDISP_PutGraphic(_CTLS_MEG_RF_, 0, _COORDINATE_Y_LINE_8_3_);		/* 顯示 請刷卡或感應卡片 */
					}
					else if (pobTran->srBRec.inCode == _REDEEM_REFUND_)
					{
						inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜紅利扣抵退貨＞ */
						inDISP_PutGraphic(_CTLS_MEG_RF_, 0, _COORDINATE_Y_LINE_8_3_);			   /* 顯示 請刷卡或感應卡片 */
					}
					else if (pobTran->srBRec.inCode == _INST_REFUND_)
					{
						inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜分期交易退貨＞ */
						inDISP_PutGraphic(_CTLS_MEG_RF_, 0, _COORDINATE_Y_LINE_8_3_);			 /* 顯示 請刷卡或感應卡片 */
					}
					else if (pobTran->srBRec.inCode == _REDEEM_ADJUST_)
					{
						inDISP_PutGraphic(_MENU_REDEEM_ADJUST_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜紅利調帳＞ */
						inDISP_PutGraphic(_CTLS_TX_MEG_RF_, 0, _COORDINATE_Y_LINE_8_3_);		   /* 顯示 請刷卡或感應卡片 人工輸入請按0 */
					}
					else if (pobTran->srBRec.inCode == _INST_ADJUST_)
					{
						inDISP_PutGraphic(_MENU_INST_ADJUST_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜分期調帳＞ */
						inDISP_PutGraphic(_CTLS_TX_MEG_RF_, 0, _COORDINATE_Y_LINE_8_3_);		 /* 顯示 請刷卡或感應卡片 人工輸入請按0 */
					}
				}
			}
		}
	}

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_IsCard_Still_Exist
Date&Time       :2017/11/6 上午 11:00
Describe        :
*/
int inCTLS_Check_TypeACard()
{
	int inRetVal = VS_SUCCESS;
	char szDebugMsg[100 + 1];
	unsigned char uszATQA1[20], uszSAK1[20], uszTCSN1[20];
	unsigned char uszTCSNLen1;
	unsigned short usRetVal = 0;

	usRetVal = CTOS_CLTypeAActiveFromIdle(0, uszATQA1, uszSAK1, uszTCSN1, &uszTCSNLen1);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			if (usRetVal != d_CL_ACCESS_TIMEOUT)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CTOS_CLTypeAActiveFromIdle Fail :0x%04X", usRetVal);
				inDISP_LogPrintf(szDebugMsg);
			}
		}

		inRetVal = VS_ERROR;
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%02X %02X %02X %02X", uszTCSN1[0], uszTCSN1[1], uszTCSN1[2], uszTCSN1[3]);
			inDISP_LogPrintf(szDebugMsg);
		}

		inRetVal = VS_SUCCESS;
	}

	return (inRetVal);
}

/*
Function        :inCTLS_Check_ISO14443_4_Card
Date&Time       :2018/1/24 下午 5:18
Describe        :
*/
int inCTLS_Check_ISO14443_4_Card()
{
	int inRetVal = VS_SUCCESS;
	char szDebugMsg[100 + 1];
	unsigned char uszATS[30 + 1] = {0};
	unsigned char uszAutoBR = 0;
	unsigned short usATSLen = 0;
	unsigned short usRetVal = 0;
	/*
	 * bAutoBR(AutoBaudRate)
	 * = 1 Perform PPS automatically after receiving correct ATS.
	 * = 0 Do not perform PPS, use the default baud rate (106k).
	 *
	 */

	uszAutoBR = 0;

	usRetVal = CTOS_CLRATS(uszAutoBR, uszATS, &usATSLen);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			if (usRetVal != d_CL_ACCESS_TIMEOUT)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CTOS_CLRATS Fail :0x%04X", usRetVal);
				inDISP_LogPrintf(szDebugMsg);
			}
		}

		inRetVal = VS_ERROR;
	}
	else
	{
		inRetVal = VS_SUCCESS;
	}

	return (inRetVal);
}

/*
Function        :inCTLS_IsCard_Still_Exist
Date&Time       :2017/11/6 上午 11:00
Describe        :
*/
int inCTLS_IsCard_Still_Exist()
{
	int inRetVal = VS_SUCCESS;

	/* TypeA Card */
	inRetVal = inCTLS_Check_TypeACard();

	return (inRetVal);
}

/*
Function        :inCTLS_Check_Mifare_Card
Date&Time       :2018/1/24 下午 5:33
Describe        :Mifare卡分別法，是Type A Card，但不是ISO14443-4的Card
 *		 注意:這只能分純Mifare卡，聯名卡無法區分
*/
int inCTLS_Check_Mifare_Card()
{
	int inRetVal = VS_ERROR;
	unsigned char uszTypeABit = VS_FALSE;
	unsigned char uszISO14443_4Bit = VS_FALSE;

	/* TypeA Card */
	inRetVal = inCTLS_Check_TypeACard();
	if (inRetVal == VS_SUCCESS)
	{
		uszTypeABit = VS_TRUE;
	}

	/* ISO14443-4 */
	inRetVal = inCTLS_Check_ISO14443_4_Card();
	if (inRetVal == VS_SUCCESS)
	{
		uszISO14443_4Bit = VS_TRUE;
	}

	if (uszTypeABit == VS_TRUE && uszISO14443_4Bit == VS_FALSE)
	{
		inRetVal = VS_SUCCESS;
	}
	else
	{
		inRetVal = VS_ERROR;
	}

	return (inRetVal);
}

/*
Function        :inCTLS_CheckRemoveCard
Date&Time       :2017/11/6 上午 10:48
Describe        :
*/
int inCTLS_CheckRemoveCard(TRANSACTION_OBJECT *pobTran, int inIsError)
{
	int inRetVal = VS_ERROR;
	int inTimeOut = 180;

	inRetVal = inCTLS_IsCard_Still_Exist();
	if (inRetVal == VS_SUCCESS)
	{
		/* 仍然插著卡片 */
		/* 設定Timeout */
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, inTimeOut);

		//		if (inIsError == _REMOVE_CARD_ERROR_)
		//		{
		//			/* 請取回感應卡 */
		//			inDISP_Msg_BMP(_ERR_PLS_REMOVE_CTLS_, _COORDINATE_Y_LINE_8X16_6_, _CLEAR_KEY_MSG_, inTimeOut, "", 0);
		//		}
		//		else
		//		{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_ERR_PLS_REMOVE_CTLS_, 0, _COORDINATE_Y_LINE_8_6_);
		//		}
	}
	else
		return (VS_SUCCESS);

	while (1)
	{
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			return (VS_SUCCESS);
		}

		inRetVal = inCTLS_IsCard_Still_Exist();
		if (inRetVal == VS_SUCCESS)
		{
			/* 請取回感應卡 */
			inDISP_PutGraphic(_ERR_PLS_REMOVE_CTLS_, 0, _COORDINATE_Y_LINE_8_6_);
			inDISP_BEEP(1, 1000);
		}
		else
		{
			return (VS_SUCCESS);
		}
	}
}

/*
Function        :inCTLS_LED_Wait_Start
Date&Time       :2018/1/10 上午 11:24
Describe        :
*/
int inCTLS_LED_Wait_Start()
{
	if (ginMachineType != _CASTLE_TYPE_UPT1000_) /* UPT會當機註掉 */
	{
		EMVCL_StartIdleLEDBehavior(NULL);
	}

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_LED_Wait_STOP
Date&Time       :2018/1/10 上午 11:24
Describe        :
*/
int inCTLS_LED_Wait_STOP()
{
	EMVCL_StopIdleLEDBehavior(NULL);

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_Mifare_LoadKey
Date&Time       :2018/2/7 下午 5:24
Describe        :
*/
int inCTLS_Mifare_LoadKey(unsigned char *uszKey)
{
	int inRetVal = VS_SUCCESS;
	char szDebugMsg[100 + 1];
	unsigned short usRetVal = 0x00;

	usRetVal = CTOS_MifareLOADKEY(uszKey);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_MifareLOADKEY Fail :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}

		inRetVal = VS_ERROR;
	}

	return (inRetVal);
}

/*
Function        :inCTLS_Mifare_Auth
Date&Time       :2018/2/7 下午 5:24
Describe        :uszKeyType : The type of key. = 0x60: Key A = 0x61: Key B
*/
int inCTLS_Mifare_Auth(unsigned char uszKeyType, unsigned char uszBlockIndex, unsigned char *uszCardSN, unsigned char uszCardSNLen)
{
	int inRetVal = VS_ERROR;
	char szDebugMsg[100 + 1];
	unsigned short usRetVal = 0x00;

	usRetVal = CTOS_MifareAUTHEx(uszKeyType, uszBlockIndex, uszCardSN, uszCardSNLen);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_MifareAUTHEx Fail :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}

		inRetVal = VS_ERROR;
	}
	else
	{
		inRetVal = VS_SUCCESS;
	}

	return (inRetVal);
}

/*
Function        :inCTLS_Get_TypeACardSN
Date&Time       :2018/2/7 下午 6:30
Describe        :
*/
int inCTLS_Get_TypeACardSN(char *szUID)
{
	int inRetVal = VS_SUCCESS;
	char szDebugMsg[100 + 1];
	unsigned char uszATQA1[20], uszSAK1[20], uszTCSN1[20];
	unsigned char uszTCSNLen1;
	unsigned short usRetVal = 0;

	usRetVal = CTOS_CLTypeAActiveFromIdle(0, uszATQA1, uszSAK1, uszTCSN1, &uszTCSNLen1);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			if (usRetVal != d_CL_ACCESS_TIMEOUT)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CTOS_CLTypeAActiveFromIdle Fail :0x%04X", usRetVal);
				inDISP_LogPrintf(szDebugMsg);
			}
		}

		inRetVal = VS_ERROR;
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%02X %02X %02X %02X", uszTCSN1[0], uszTCSN1[1], uszTCSN1[2], uszTCSN1[3]);
			inDISP_LogPrintf(szDebugMsg);
		}

		sprintf(szUID, "%02X%02X%02X%02X", uszTCSN1[0], uszTCSN1[1], uszTCSN1[2], uszTCSN1[3]);
		inRetVal = VS_SUCCESS;
	}

	return (inRetVal);
}

/*
Function        :inCTLS_ReActive_TypeACard
Date&Time       :2018/7/31 下午 5:22
Describe        :啟動typeA Card使用，因為前面已經使用EMVCL_InitTransactionEx，要重新啟動A Card直接使用這隻，就可以繼續下APDU命令了
*/
int inCTLS_ReActive_TypeACard()
{
	int inRetVal = VS_ERROR;
	unsigned char uszATQA[20] = {0}, uszSAK[20] = {0}, uszTCSN[20] = {0};
	unsigned char uszTCSNLen = 0;
	unsigned char uszAutoBR = 0x00;
	unsigned char uszATS[30] = {0};
	unsigned short usATSLen = 0;

	uszTCSNLen = sizeof(uszTCSN);

	inRetVal = CTOS_CLTypeAActiveFromIdle(0, uszATQA, uszSAK, uszTCSN, &uszTCSNLen);
	inRetVal = CTOS_CLRATS(uszAutoBR, uszATS, &usATSLen);

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_Power_On
Date&Time       :2018/8/15 上午 11:05
Describe        :開啟感應天線
*/
int inCTLS_Power_On(void)
{
	char szDebugMsg[100 + 1] = {0};
	unsigned short usRetVal = VS_ERROR;

	usRetVal = CTOS_CLPowerOn();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_CLPowerOn() OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_Power_Off
Date&Time       :2018/8/15 上午 11:05
Describe        :關閉感應天線
*/
int inCTLS_Power_Off(void)
{
	char szDebugMsg[100 + 1] = {0};
	unsigned short usRetVal = VS_ERROR;

	usRetVal = CTOS_CLPowerOff();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_CLPowerOff() OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_ExceptionCheck_External
Date&Time       :2017/1/18 下午 1:57
Describe        :檢查例外狀況
 * [修改外接感應設備] 2022/11/22 [SAM]
 */
int inCTLS_ExceptionCheck_External(TRANSACTION_OBJECT *pobTran)
{
	int inVWTIndex = -1;
	int inMVTIndex = -1;
	int i;
	long lnCheckAmount = 0;
	char szASCII[256 + 1];
	char szTemplate[256 + 1];
	char szPayWaveAPID[32 + 1], szPayWaveAPIDContactlessTransactionLimit[12 + 1], szPayWaveAPIDCVMRequiredLimit[12 + 1];
	char szContactlessTransactionLimit[12 + 1], szCVMRequiredLimit[12 + 1], szTerminalCapabilities[6 + 1];
	char szWAVEDate[15 + 1], szEDCDate[15 + 1];
	char szCardSupEnable[2 + 1];
	char szDebugMsg[100 + 1];
	VS_BOOL fCheckAPID = VS_FALSE;
	RTC_NEXSYS srRTC; /* Date & Time */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START-----", __FILE__, __FUNCTION__, __LINE__);

	if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, 4))
	{
		if (pobTran->in57_Track2Len > 0)
			pobTran->srBRec.uszWAVESchemeID = SCHEME_ID_17_WAVE3;
		else
			pobTran->srBRec.uszWAVESchemeID = SCHEME_ID_16_WAVE1;
	}
	else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, 10))
	{
		if (pobTran->srBRec.uszWAVESchemeID != SCHEME_ID_20_PAYPASS_MAG_STRIPE)
			pobTran->srBRec.uszWAVESchemeID = SCHEME_ID_21_PAYPASS_MCHIP;
	}
	else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_JCB_, 3))
	{
		pobTran->srBRec.uszWAVESchemeID = SCHEME_ID_61_JSPEEDY;
	}
	else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, 4))
	{
		pobTran->srBRec.uszWAVESchemeID = SCHEME_ID_50_EXPRESSSPAY;
	}
	else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_CUP_, 3) ||
			 !memcmp(&pobTran->srEMVRec.usz84_DF_NAME[0], "\xA0\x00\x00\x03\x33\x01\x01\x01", 8) ||
			 !memcmp(&pobTran->srEMVRec.usz84_DF_NAME[0], "\xA0\x00\x00\x03\x33\x01\x01\x02", 8) ||
			 !memcmp(&pobTran->srEMVRec.usz84_DF_NAME[0], "\xA0\x00\x00\x03\x33\x01\x01\x03", 8))
	{
		pobTran->srBRec.uszWAVESchemeID = SCHEME_ID_91_QUICKPASS;
	}
	else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, 6))
	{
		pobTran->srBRec.uszWAVESchemeID = SCHEME_ID_42_DPAS;
	}
	else
	{
		inDISP_LogPrintfAt(AT, "  uszWAVESchemeID Not Set *Error* Line[%d]", __LINE__);
	}

	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1 || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
	{
		inMVTIndex = _VISA_MVT_INDEX_;
		inVWTIndex = _PAYWAVE_VWT_INDEX_;
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY)
	{
		inMVTIndex = _JSMART_MVT_INDEX_;
		inVWTIndex = _JSPEEDY_VWT_INDEX_;
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE)
	{
		inMVTIndex = _MCHIP_MVT_INDEX_;
		inVWTIndex = _PAYPASS_VWT_INDEX_;
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS && !memcmp(&pobTran->srEMVRec.usz84_DF_NAME[0], "\xA0\x00\x00\x03\x33\x01\x01\x01", 8))
	{
		inMVTIndex = _CUP_DEBIT_MVT_INDEX_;
		inVWTIndex = _QP_DEBIT_VWT_INDEX_;
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS && !memcmp(&pobTran->srEMVRec.usz84_DF_NAME[0], "\xA0\x00\x00\x03\x33\x01\x01\x02", 8))
	{
		inMVTIndex = _CUP_CREDIT_MVT_INDEX_;
		inVWTIndex = _QP_CREDIT_VWT_INDEX_;
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS && !memcmp(&pobTran->srEMVRec.usz84_DF_NAME[0], "\xA0\x00\x00\x03\x33\x01\x01\x03", 8))
	{
		inMVTIndex = _CUP_QUASI_CREDIT_MVT_INDEX_;
		inVWTIndex = _QP_QUASI_CREDIT_VWT_INDEX_;
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
	{
		inMVTIndex = _AEIPS_MVT_INDEX_;
		inVWTIndex = _AE_EXPRESSPAY_VWT_INDEX_;
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE)
	{
		inMVTIndex = _D_PAS_MVT_INDEX_;
		inVWTIndex = _D_PAS_VWT_INDEX_;
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintfAt(AT, "VWT 中無此感應卡");
		}
		return (VS_WAVE_ERROR); /* 表示記憶體有問題或是 VWT Table 不存在 */
	}

	if (inLoadMVTRec(inMVTIndex) < 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintfAt(AT, "MV 中無此感應卡");
		}

		return (VS_WAVE_ERROR); /* 表示記憶體有問題或是 MVT Table 不存在 */
	}

	if (inLoadVWTRec(inVWTIndex) < 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintfAt(AT, "VWT 中無此感應卡");
		}

		return (VS_WAVE_ERROR); /* 表示記憶體有問題或是 VWT Table 不存在 */
	}

	/* 先檢查是否TMS支援此卡別 */
	memset(szCardSupEnable, 0x00, sizeof(szCardSupEnable));
	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1 ||
		pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
	{
		inGetVISAPaywaveEnable(szCardSupEnable);
		if (memcmp(szCardSupEnable, "Y", strlen("Y")) != 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CFGT 不支援 VISA WAVE");
				inDISP_LogPrintfAt(AT, szDebugMsg);
			}

			/* 不接受此感應卡 請改刷卡或插卡 */
			//			  inFISC_CTLS_LED_TONE(VS_ERROR);
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC;

			return (VS_WAVE_ERROR);
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY)
	{
		inGetJCBJspeedyEnable(szCardSupEnable);
		if (memcmp(szCardSupEnable, "Y", strlen("Y")) != 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CFGT 不支援 JSPEEDY");
				inDISP_LogPrintfAt(AT, szDebugMsg);
			}

			/* 不接受此感應卡 請改刷卡或插卡 */
			//                        inFISC_CTLS_LED_TONE(VS_ERROR);
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC;

			return (VS_WAVE_ERROR);
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
	{
		inGetMCPaypassEnable(szCardSupEnable);
		if (memcmp(szCardSupEnable, "Y", strlen("Y")) != 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CFGT 不支援 MChip");
				inDISP_LogPrintfAt(AT, szDebugMsg);
			}

			/* 不接受此感應卡 請改刷卡或插卡 */
			//                        inFISC_CTLS_LED_TONE(VS_ERROR);
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC;

			return (VS_WAVE_ERROR);
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS)
	{
		inGetCUPContactlessEnable(szCardSupEnable);
		if (memcmp(szCardSupEnable, "Y", strlen("Y")) != 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CFGT 不支援 Quickpass");
				inDISP_LogPrintfAt(AT, szDebugMsg);
			}

			/* 不接受此感應卡 請改刷卡或插卡 */
			//                        inFISC_CTLS_LED_TONE(VS_ERROR);
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC;

			return (VS_WAVE_ERROR);
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
	{
		inGetAMEXContactlessEnable(szCardSupEnable);
		if (memcmp(szCardSupEnable, "Y", strlen("Y")) != 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CFGT 不支援 AE Contactless");
				inDISP_LogPrintfAt(AT, szDebugMsg);
			}

			/* 不接受此感應卡 請改刷卡或插卡 */
			//                        inFISC_CTLS_LED_TONE(VS_ERROR);
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC;

			return (VS_WAVE_ERROR);
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE)
	{
		inGetDFS_Contactless_Enable(szCardSupEnable);
		if (memcmp(szCardSupEnable, "Y", strlen("Y")) != 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CFGT 不支援 DFS Contactless");
				inDISP_LogPrintfAt(AT, szDebugMsg);
			}

			/* 不接受此感應卡 請改刷卡或插卡 */
			//                        inFISC_CTLS_LED_TONE(VS_ERROR);
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC;

			return (VS_WAVE_ERROR);
		}
	}

	/* 先看感應限額(ContactlessTransactionLimit) */
	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1 || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
	{
		/* 【需求單104154】VCPS2.1.3規範，新增APID判定國內外限額需求 */
		/* Paywave 有APID，有自己獨立的Transaction Limti, CVM Limit, Floor Limit，所以這裡先看是不是APID */
		memset(szPayWaveAPID, 0x00, sizeof(szPayWaveAPID));
		inGetPayWaveAPID(szPayWaveAPID);

		if (pobTran->srEMVRec.in9F5A_Application_Program_IdentifierLen > 0 && strlen(szPayWaveAPID) > 0)
		{
			memset(szASCII, 0x00, sizeof(szASCII));
			inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F5A_Application_Program_Identifier[0], pobTran->srEMVRec.in9F5A_Application_Program_IdentifierLen);

			/* 比對APID */
			if (!memcmp(&szASCII[0], &szPayWaveAPID[0], strlen(szPayWaveAPID)))
			{
				/* 有比對到APID */
				fCheckAPID = VS_TRUE;
			}
			else
			{
				/* 沒有比對到APID */
				fCheckAPID = VS_FALSE;
			}
		}

		/* 有比對到APID */
		if (fCheckAPID == VS_TRUE)
		{
			/* 檢核感應限額 */
			memset(szPayWaveAPIDContactlessTransactionLimit, 0x00, sizeof(szPayWaveAPIDContactlessTransactionLimit));
			memset(szTemplate, 0x00, sizeof(szTemplate));

			inGetPayWaveAPIDContactlessTransactionLimit(szTemplate);
			memcpy(szPayWaveAPIDContactlessTransactionLimit, szTemplate, 10);

			lnCheckAmount = atol(szPayWaveAPIDContactlessTransactionLimit);

			if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			{
				/* 超過感應限額 請改刷卡或插卡 */
				//                                inFISC_CTLS_LED_TONE(VS_ERROR);
				pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_OVER_AMOUNT_;
				return (VS_WAVE_AMOUNT_ERR);
			}

		} /* 沒有比對到APID */
		else if (fCheckAPID == VS_FALSE)
		{
			memset(szContactlessTransactionLimit, 0x00, sizeof(szContactlessTransactionLimit));
			memset(szTemplate, 0x00, sizeof(szTemplate));

			inGetContactlessTransactionLimit(szTemplate);
			memcpy(szContactlessTransactionLimit, szTemplate, 10);

			/* 第一步檢核感應限額 */
			lnCheckAmount = atol(szContactlessTransactionLimit);

			if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			{
				/* 超過感應限額 請改刷卡或插卡 */
				//                                inFISC_CTLS_LED_TONE(VS_ERROR);
				pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_OVER_AMOUNT_;
				return (VS_WAVE_AMOUNT_ERR);
			}
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE)
	{
		memset(szContactlessTransactionLimit, 0x00, sizeof(szContactlessTransactionLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetContactlessTransactionLimit(szTemplate);
		memcpy(szContactlessTransactionLimit, szTemplate, 10);

		lnCheckAmount = atol(szContactlessTransactionLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
		{
			/* 超過感應限額 請改刷卡或插卡 */
			//                        inFISC_CTLS_LED_TONE(VS_ERROR);
			pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_OVER_AMOUNT_;
			return (VS_WAVE_AMOUNT_ERR);
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
	{
		/* VWT中ContactlessTransactionLimit 不For PayWave 使用 */
	}

	/* 確認CID，虹堡系列都會吐DF8F4F 但為了簡潔，直接看CID來看online offline */
	/*【PayWave3】感應卡 */
	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
	{
		if (pobTran->srEMVRec.in9F27_CIDLen <= 0)
		{
			/* 感應資料不足 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_CTLS_DATA_SHORT_;
			return (VS_WAVE_ERROR);
		}
		else
		{
			if (pobTran->srEMVRec.usz9F27_CID[0] == 0x40)
			{
				/* TC */
				/* 分期、紅利不能offline */
				if (pobTran->srBRec.uszInstallmentBit == VS_TRUE ||
					pobTran->srBRec.uszRedeemBit == VS_TRUE)
				{
				}
				else
				{
					/* 財金不允許Y1 */
					// memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
					// strcpy(pobTran->srBRec.szAuthCode, "Y1");
				}
			}
			else if (pobTran->srEMVRec.usz9F27_CID[0] == 0x80)
			{
				/* ARQC */
				/* 要 Online */
			}
			else
			{
				/* AAC */
				/* 拒絕交易 Z1 */
				/* 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_Z1_;
				return (VS_WAVE_ERROR);
			}
		}

		/* (6)	PayWave 3卡片感應交易，若卡片未回傳 PAN_SEQ_NUM 時，端末機應以預設值 “00” (Hard code) 上傳。 */
		if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen <= 0)
		{
			pobTran->srEMVRec.in5F34_ApplPanSeqnumLen = 1;
			memset(pobTran->srEMVRec.usz5F34_ApplPanSeqnum, 0x00, sizeof(pobTran->srEMVRec.usz5F34_ApplPanSeqnum));
			pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0] = 0x00;
		}
	} /* 【Newjspeedy】感應卡 */
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY)
	{
		if (pobTran->srEMVRec.in9F27_CIDLen > 0)
		{
			if (pobTran->srEMVRec.usz9F27_CID[0] == 0x40)
			{
				/* TC */
				/* 分期、紅利不能offline */
				if (pobTran->srBRec.uszInstallmentBit == VS_TRUE ||
					pobTran->srBRec.uszRedeemBit == VS_TRUE)
				{
				}
				else
				{
					/* 財金不允許Y1 */
					// memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
					// strcpy(pobTran->srBRec.szAuthCode, "Y1");
				}
			}
			else if (pobTran->srEMVRec.usz9F27_CID[0] == 0x80)
			{
				/* ARQC */
				/* 要 Online */
			}
			else
			{
				/* AAC */
				/* 拒絕交易 Z1 */
				/* 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_Z1_;
				return (VS_WAVE_ERROR);
			}
		}

		/* (6)	Newjspeedy卡片感應交易，若卡片未回傳 PAN_SEQ_NUM 時，端末機應以預設值 “00” (Hard code) 上傳。 */
		if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen <= 0)
		{
			pobTran->srEMVRec.in5F34_ApplPanSeqnumLen = 1;
			memset(pobTran->srEMVRec.usz5F34_ApplPanSeqnum, 0x00, sizeof(pobTran->srEMVRec.usz5F34_ApplPanSeqnum));
			pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0] = 0x00;
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS)
	{
		if (pobTran->srEMVRec.in9F27_CIDLen <= 0)
		{
			/* 感應資料不足 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_CTLS_DATA_SHORT_;
			return (VS_WAVE_ERROR);
		}
		else
		{
			if (pobTran->srEMVRec.usz9F27_CID[0] == 0x40)
			{
				/* TC */
				/* 銀聯不能offline */
			}
			else if (pobTran->srEMVRec.usz9F27_CID[0] == 0x80)
			{
				/* ARQC */
				/* 要 Online */
			}
			else
			{
				/* AAC */
				/* 拒絕交易 Z1 */
				/* 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_Z1_;
				return (VS_WAVE_ERROR);
			}
		}

	} /* 原先於ProcessChipData時塞Y1，現在統一拉下來塞 */
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
	{
		if (pobTran->srEMVRec.in9F27_CIDLen > 0)
		{
			if (pobTran->srEMVRec.usz9F27_CID[0] == 0x40)
			{
				/* TC */
				/* 分期、紅利不能offline */
				if (pobTran->srBRec.uszInstallmentBit == VS_TRUE ||
					pobTran->srBRec.uszRedeemBit == VS_TRUE)
				{
				}
				else
				{
					/* 財金不允許Y1 */
					// memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
					// strcpy(pobTran->srBRec.szAuthCode, "Y1");
				}
			}
			else if (pobTran->srEMVRec.usz9F27_CID[0] == 0x80)
			{
				/* ARQC */
				/* 要 Online */
			}
			else
			{
				/* AAC */
				/* 拒絕交易 Z1 */
				/* 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_Z1_;
				return (VS_WAVE_ERROR);
			}
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
	{
		if (pobTran->srEMVRec.in9F27_CIDLen <= 0)
		{
			/* 感應資料不足 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_CTLS_DATA_SHORT_;
			return (VS_WAVE_ERROR);
		}
		else
		{
			if (pobTran->srEMVRec.usz9F27_CID[0] == 0x40)
			{
				/* TC */
				/* 分期、紅利不能offline */
				if (pobTran->srBRec.uszInstallmentBit == VS_TRUE ||
					pobTran->srBRec.uszRedeemBit == VS_TRUE)
				{
				}
				else
				{
					memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
					strcpy(pobTran->srBRec.szAuthCode, "Y1");
				}
			}
			else if (pobTran->srEMVRec.usz9F27_CID[0] == 0x80)
			{
				/* ARQC */
				/* 要 Online */
			}
			else
			{
				/* AAC */
				/* 拒絕交易 Z1 */
				/* 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_Z1_;
				return (VS_WAVE_ERROR);
			}
		}
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE)
	{
		if (pobTran->srEMVRec.in9F27_CIDLen <= 0)
		{
			/* 感應資料不足 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_CTLS_DATA_SHORT_;
			return (VS_WAVE_ERROR);
		}
		else
		{
			if (pobTran->srEMVRec.usz9F27_CID[0] == 0x40)
			{
				/* TC */
				/* 分期、紅利不能offline */
				if (pobTran->srBRec.uszInstallmentBit == VS_TRUE ||
					pobTran->srBRec.uszRedeemBit == VS_TRUE)
				{
				}
				else
				{
					memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
					strcpy(pobTran->srBRec.szAuthCode, "Y1");
				}
			}
			else if (pobTran->srEMVRec.usz9F27_CID[0] == 0x80)
			{
				/* ARQC */
				/* 要 Online */
			}
			else
			{
				/* AAC */
				/* 拒絕交易 Z1 */
				/* 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_Z1_;
				return (VS_WAVE_ERROR);
			}
		}
	}

	/* 處理狀況一：如果是 VISA Paywave1 & VISA Paywave3 & Jspeedy 要從【mvt.txt】組【9F33】 */
	/* R-FES EDC Specifications v1.49.doc 規格
		1.	Contactless 交易上傳之 Tag 9F33 的 Value 規則如下：
		(1)	VISA Paywave 1/VISA Paywave 3/JSpeedy 請上傳Terminal 之 EMV 參數（因為 V & J 感應交易Reader 不會回 Tag 9F33）。
		(2)	Paypass 因為 Reader 會回覆 Tag 9F33 ，所以依Reader 回覆資料上傳。
	 */
	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1 ||
		pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3 ||
		pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY ||
		pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV ||
		pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD ||
		pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY)
	{
		if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1 ||
			pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
		{
			inMVTIndex = _VISA_MVT_INDEX_;
		}
		else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY ||
				 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV ||
				 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD ||
				 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY)
		{
			inMVTIndex = _JSMART_MVT_INDEX_;
		}

		if (inLoadMVTRec(inMVTIndex) < 0)
		{
			return (VS_WAVE_ERROR);
		}

		/* 有9F33直接用，沒有就從MVT抓 */
		if (pobTran->srEMVRec.in9F33_TermCapabilitiesLen > 0)
		{
		}
		else
		{
			inGetTerminalCapabilities(szTerminalCapabilities);
			memset(pobTran->srEMVRec.usz9F33_TermCapabilities, 0x00, sizeof(pobTran->srEMVRec.usz9F33_TermCapabilities));
			pobTran->srEMVRec.in9F33_TermCapabilitiesLen = ((strlen(szTerminalCapabilities) + 1) / 2);
			inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9F33_TermCapabilities[0], &szTerminalCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
		}
	}

	/* 處理狀況二：所有感應交易要組【DFEE】【DEEF】 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEE_TERMINAL_ENTRY_CAPABILITY, (unsigned char *)&szTemplate[0]) > 0)
	{
		pobTran->srEMVRec.inDFEE_TerEntryCapLen = szTemplate[2];
		memcpy((char *)&pobTran->srEMVRec.uszDFEE_TerEntryCap[0], &szTemplate[3], pobTran->srEMVRec.inDFEE_TerEntryCapLen);
	}

	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEF_REASON_ONLINE_CODE, (unsigned char *)&szTemplate[0]) > 0)
	{
		pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen = szTemplate[2];
		memcpy((char *)&pobTran->srEMVRec.uszDFEF_ReasonOnlineCode[0], &szTemplate[3], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);
	}

	/* 免簽名判斷(CVM Limit) */
	/* 先預設為免簽名 */
	pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
	/* 看9F34 */
	if (pobTran->srEMVRec.in9F34_CVMLen > 0)
	{
		if (pobTran->srEMVRec.usz9F34_CVM[0] == 0x1F)
		{
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
		}
		else
		{
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		}
	}
	else
	{
		/* 先讀CVM Limit */
		memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetCVMRequiredLimit(szTemplate);
		memcpy(szCVMRequiredLimit, szTemplate, 10); /* 小數點後兩位不用 */

		lnCheckAmount = atol(szCVMRequiredLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		else
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
	}

	/* 之前發生過9F34失效，所以下面根據金額再確認一次 */
	/* VISA */
	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1 || pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
	{
		if (fCheckAPID == VS_TRUE)
		{
			memset(szPayWaveAPIDCVMRequiredLimit, 0x00, sizeof(szPayWaveAPIDCVMRequiredLimit));
			memset(szTemplate, 0x00, sizeof(szTemplate));

			inGetPayWaveAPIDCVMRequiredLimit(szTemplate);
			memcpy(szPayWaveAPIDCVMRequiredLimit, szTemplate, 10);

			/* 第二步檢核免簽名【VISA Paywave1】卡片要判斷是否免簽名 */
			if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1)
			{
				lnCheckAmount = atol(szPayWaveAPIDCVMRequiredLimit);
				if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
					pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
				else
					pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
			}

			/* 第三步檢核免簽名【VISA Paywave3】卡片要判斷是否免簽名 */
			if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
			{
				if (pobTran->uszPayWave3Tag55Bit == VS_TRUE && pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
				{
					/* 表示已經判斷過【TAG 55】要在判斷一次是否免簽,若要簽名就不用在判斷一次 */
					lnCheckAmount = atol(szPayWaveAPIDCVMRequiredLimit);
					if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
						pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
					else
						pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
				}
				else if (pobTran->uszPayWave3Tag55Bit == VS_FALSE)
				{
					/* 沒有回【TAG 55】 */
					lnCheckAmount = atol(szPayWaveAPIDCVMRequiredLimit);
					if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
						pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
					else
						pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
				}
			}

			/* 檢核是否通過CDCVM，若VISA手機信用卡通過一律免簽 */
			/* 新增VISA CDCVM判斷是否要免簽名 */
			if ((pobTran->srEMVRec.in9F6C_Card_Transaction_QualifiersLen > 0) &&
				((pobTran->srEMVRec.usz9F6C_Card_Transaction_Qualifiers[1] & 0x80) == 0x80))
			{
				/* 9F6C Byte 2 bit 8 判斷是不是VISA CDCVM */
				pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名 */
			}
		}
		else
		{
			/* 先讀CVM Limit */
			memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
			memset(szTemplate, 0x00, sizeof(szTemplate));

			inGetCVMRequiredLimit(szTemplate);
			memcpy(szCVMRequiredLimit, szTemplate, 10); /* 小數點後兩位不用 */

			/* 第二步檢核免簽名【VISA Paywave1】卡片要判斷是否免簽名 */
			if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1)
			{
				lnCheckAmount = atol(szCVMRequiredLimit);
				if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
					pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
				else
					pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
			}

			/* 第三步檢核免簽名【VISA Paywave3】卡片要判斷是否免簽名 */
			if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
			{
				if (pobTran->uszPayWave3Tag55Bit == VS_TRUE && pobTran->srBRec.uszNoSignatureBit != VS_TRUE)
				{
					/* 表示已經判斷過【TAG 55】要在判斷一次 */
					lnCheckAmount = atol(szCVMRequiredLimit);
					if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
						pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
					else
						pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
				}
				else if (pobTran->uszPayWave3Tag55Bit == VS_FALSE)
				{
					/* 沒有回【TAG 55】 */
					lnCheckAmount = atol(szCVMRequiredLimit);
					if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
						pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
					else
						pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
				}
			}

			/* 檢核是否通過CDCVM，若VISA手機信用卡通過一律免簽 */
			/* 新增VISA CDCVM判斷是否要免簽名 */
			if ((pobTran->srEMVRec.in9F6C_Card_Transaction_QualifiersLen > 0) &&
				((pobTran->srEMVRec.usz9F6C_Card_Transaction_Qualifiers[1] & 0x80) == 0x80))
			{
				/* 9F6C Byte 2 bit 8 判斷是不是VISA CDCVM */
				pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名 */
			}
		}

	} /* 【PAYpass】卡片要判斷是否免簽名 */
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
	{
		memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetPaypassCVMRequiredLimit(szTemplate);
		memcpy(szCVMRequiredLimit, szTemplate, 10);

		lnCheckAmount = atol(szCVMRequiredLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		else
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
	}													 /* 【JSPEEDY】卡片要判斷是否免簽名 */
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY)
	{
		memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetPaypassCVMRequiredLimit(szTemplate);
		memcpy(szCVMRequiredLimit, szTemplate, 10);

		lnCheckAmount = atol(szCVMRequiredLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		else
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
	}													 /* QUICKPASS */
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS)
	{
		/* 判斷【QuickPass】卡片要判斷是否免簽名(PaypassCVMRequiredLimit 由Paypass、NewJ、QuickPass共用) */
		memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		/* QuickPass此欄位只用來看是否免簽名 */
		inGetPaypassCVMRequiredLimit(szTemplate);
		memcpy(szCVMRequiredLimit, szTemplate, 10);

		lnCheckAmount = atol(szCVMRequiredLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		else
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
	{
		memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetCVMRequiredLimit(szTemplate);
		memcpy(szCVMRequiredLimit, szTemplate, 10);

		lnCheckAmount = atol(szCVMRequiredLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		else
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
	}
	else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS ||
			 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE)
	{
		memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetCVMRequiredLimit(szTemplate);
		memcpy(szCVMRequiredLimit, szTemplate, 10);

		lnCheckAmount = atol(szCVMRequiredLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		else
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */

		/* 檢核是否通過CDCVM，若DISCOVER手機信用卡通過一律免簽 */
		/* 新增DISCOVER CDCVM判斷是否要免簽名 */
		if ((pobTran->srEMVRec.in9F6C_Card_Transaction_QualifiersLen > 0) &&
			((pobTran->srEMVRec.usz9F6C_Card_Transaction_Qualifiers[1] & 0x80) == 0x80))
		{
			/* 9F6C Byte 2 bit 8 判斷是不是 CDCVM */
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名 */
		}
	}

	/* 處理狀況五：如果是授權碼是【Y1】要送【Tag 8A】 */
	if (!memcmp(&pobTran->srBRec.szAuthCode[0], "Y1", 2))
	{
		memset(pobTran->srEMVRec.usz8A_AuthRespCode, 0x00, sizeof(pobTran->srEMVRec.usz8A_AuthRespCode));
		pobTran->srEMVRec.in8A_AuthRespCodeLen = 2;
		memcpy(&pobTran->srEMVRec.usz8A_AuthRespCode[0], "Y1", 2);
	}

	/* 處理狀況六：存卡片有效期 */
	if (strlen(pobTran->srBRec.szExpDate) == 0)
	{
		if (pobTran->srEMVRec.in5F24_ExpireDateLen != 0)
		{
			inFunc_BCD_to_ASCII(&pobTran->srBRec.szExpDate[0], &pobTran->srEMVRec.usz5F24_ExpireDate[0], pobTran->srEMVRec.in5F24_ExpireDateLen);
			pobTran->srBRec.szExpDate[4] = 0x00;
		}
	}

	/* 處理狀況七：卡片沒有回傳Tag9F03 要自己塞長度6及資料填0 */
	if (pobTran->srEMVRec.in9F03_AmtOtherNumLen <= 0)
	{
		pobTran->srEMVRec.in9F03_AmtOtherNumLen = 0x06;
		memset(pobTran->srEMVRec.usz9F03_AmtOtherNum, 0x00, sizeof(pobTran->srEMVRec.usz9F03_AmtOtherNum));
	}

	/* 處理狀況八：感應wave 3的卡片後 組封包上傳主機時不要帶上 Tag 9F35 */
	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3)
	{
		pobTran->srEMVRec.in9F35_TermTypeLen = 0x00;
		memset(pobTran->srEMVRec.usz9F35_TermType, 0x00, sizeof(pobTran->srEMVRec.usz9F35_TermType));
	}

	/* 取得端末機日期時間 */
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintfAt(AT, "取得時間失敗");
		}
		return (VS_ERROR);
	}

	/* 2011-07-11 PM 03:03:54 JCB & VLP & NewJspeedy卡片種類DF69 = 02:MSG Mode 不送F_55資料 */
	/* 吳升文 2015-05-25 (週一) 上午 11:26 信件說:
	   NewJspeedy 磁條感應卡，請比照 Paypass磁條 送 F_22 091X , 不送F_55。ATS及RFES 皆同。
	 */
	if (memcmp(&pobTran->srBRec.szAuthCode[0], "JCB", 3) != 0 &&
		memcmp(&pobTran->srBRec.szAuthCode[0], "VLP", 3) != 0 &&
		pobTran->srBRec.uszWAVESchemeID != SCHEME_ID_20_PAYPASS_MAG_STRIPE &&
		pobTran->srBRec.uszWAVESchemeID != SCHEME_ID_64_NEWJSPEEDY_MSD &&
		pobTran->srBRec.uszWAVESchemeID != SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
	{
		/* 處理狀況九：日期檢核 */
		/* Add by hanlin 2011.7.4 下午 1:47 (START) */
		if (pobTran->srEMVRec.in9A_TranDateLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintfAt(AT, "日期檢核失敗");
			}
			/* 感應失敗 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
			return (VS_WAVE_ERROR);
		}
		else
		{
			memset(szEDCDate, 0x00, sizeof(szEDCDate));
			sprintf(szEDCDate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);

			memset(szWAVEDate, 0x00, sizeof(szWAVEDate));
			inFunc_BCD_to_ASCII(&szWAVEDate[0], &pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);

			if ((memcmp(&szEDCDate[2], &szWAVEDate[0], 6)) || (!memcmp(&szWAVEDate[0], "000000", 6)))
			{
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintfAt(AT, "感應日期有誤");
					inDISP_LogPrintfAt(AT, "卡片日期,%s", szWAVEDate);
					inDISP_LogPrintfAt(AT, "機器日期,%s", szEDCDate);
				}
				/* 感應失敗 請改刷卡或插卡 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
				return (VS_WAVE_ERROR);
			}
		}
		/* Add by hanlin 2011.7.4 下午 1:47 (END) */

		/* 處理狀況十：檢核9F10(IAD)有無值 */
		/* 2011-07-11 PM 02:43:12 add by kakab */
		if (pobTran->srEMVRec.in9F10_IssuerAppDataLen <= 0)
		{
			/* 感應失敗 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
			return (VS_WAVE_ERROR);
		}

		/* 處理狀況十一：檢核9F02(Amount)有無值及範圍在 0~9 */
		/* 2011-07-11 PM 02:43:12 add by kakab */
		if (pobTran->srEMVRec.in9F02_AmtAuthNumLen <= 0)
		{
			/* 感應失敗 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
			return (VS_WAVE_ERROR);
		}
		else
		{
			memset(szWAVEDate, 0x00, sizeof(szWAVEDate));
			inFunc_BCD_to_ASCII(&szWAVEDate[0], &pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);

			for (i = 0; i < pobTran->srEMVRec.in9F02_AmtAuthNumLen * 2; i++)
			{
				if (szWAVEDate[i] <= '9' && szWAVEDate[i] >= '0')
					continue;
				else
				{
					/* 感應失敗 請改刷卡或插卡 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
					return (VS_WAVE_ERROR);
				}
			}
		}

		/* 處理狀況十二：檢核9F26(Application Cryptogram)有無值 */
		/* 2011-07-11 PM 02:43:12 add by kakab */
		if (pobTran->srEMVRec.in9F26_ApplCryptogramLen <= 0)
		{
			/* 感應失敗 請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
			return (VS_WAVE_ERROR);
		}
	}

	/* 處理QuickPass感應卡是否要輸入密碼 */
	if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS)
	{
		/* 是否輸入密碼移到TRT GET CUP PIN中判斷 */

		/* 再判斷一次【QuickPass】卡片要判斷是否免簽名 */
		memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetPaypassCVMRequiredLimit(szTemplate);
		memcpy(szCVMRequiredLimit, szTemplate, 10);

		lnCheckAmount = atol(szCVMRequiredLimit);
		if (pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
		else
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* 免簽名條件 */
	}
	else
	{
		/* 指定為銀聯交易，但是非銀聯感應閃付要檔 */
		/* 2015-09-16 AM 10:13:39 Add By Tim HappyGo流程要支援閃付 */
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			/* 不接受此感應卡 請改刷卡或插卡 */
			//                        inFISC_CTLS_LED_TONE(VS_ERROR);
			pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC;

			return (VS_WAVE_INVALID_SCHEME_ERR);
		}
	}

	/* 分期付款交易，使用感應卡進行交易，皆要簽名，不允許分期付款交易"免簽名"。 */
	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
	}

	if (ginDebug == VS_TRUE)
	{
		if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
		{
			inDISP_LogPrintfAt(AT, "本感應交易免簽名");
		}
		else
		{
			inDISP_LogPrintfAt(AT, "本感應交易須簽名");
		}

		if (memcmp(pobTran->srBRec.szAuthCode, "Y1", strlen("Y1")) == 0)
		{
			inDISP_LogPrintfAt(AT, "本感應交易Y1");
		}
		else
		{
			inDISP_LogPrintfAt(AT, "本感應交易Online");
		}

		inDISP_LogPrintfAt(AT, "inCTLS_ExceptionCheck_External END!!");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCTLS_Mifare_Read_Block
Date&Time       :2018/10/22 下午 2:49
Describe        :讀出16byte資料
*/
int inCTLS_Mifare_Read_Block(unsigned char usBlockNum, unsigned char *uszBlockContent)
{
	char szDebugMsg[100 + 1] = {0};
	unsigned short usRetVal = 0;

	usRetVal = CTOS_MifareREADBLOCK(usBlockNum, uszBlockContent);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf(AT, "CTOS_MifareREADBLOCK Success");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_MifareREADBLOCK Err :0x%04X", usRetVal);
			inDISP_LogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}
