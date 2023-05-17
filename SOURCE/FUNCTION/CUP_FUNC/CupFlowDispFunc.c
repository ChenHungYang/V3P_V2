

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>

#include "../../INCLUDES/Define_1.h"
#include "../../INCLUDES/Define_2.h"
#include "../../INCLUDES/TransType.h"
#include "../../INCLUDES/Transaction.h"
#include "../../DISPLAY/Display.h"
#include "../../DISPLAY/DispMsg.h"
#include "../../DISPLAY/DisTouch.h"
#include "../../EVENT/MenuMsg.h"

#include "../../FUNCTION/Function.h"
#include "../../FUNCTION/VWT.h"
#include "../../FUNCTION/CFGT.h"

#include "../../KEY/ProcessTmk.h"

#include "CupFlowDispFunc.h"

/*
Function        : inCUP_DisplayPleaseLogonFirst
Date&Time   : 2018/4/17 下午 4:24
Describe        : 提示請先安全結帳
*/
int inCUP_DisplayPleaseLogonFirst(TRANSACTION_OBJECT *pobTran)
{
	if (pobTran->uszSettleLOGONFailedBit == VS_TRUE)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("請先安全認證", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
		inDISP_Wait(2000);
	}
	return (VS_SUCCESS);
}

/*
Function        : inCUP_ConfirmToRunVoidTrans
Date&Time   : 2019/03/12
Describe        : 按0確認是否要進行取消交易，這隻純粹是因為要改incode為CUP_VOID所以才分出來
*/
int inCUP_ConfirmToRunVoidTrans(TRANSACTION_OBJECT *pobTran)
{
        char    szAuthCodeMsg[_DISP_MSG_SIZE_ + 1];
        char    szAmountMsg[_DISP_MSG_SIZE_ + 1];
        char    szKey;
		RTC_NEXSYS		srRTC; 		/* Date & Time */

        /* 確認金額 */
        if (pobTran->srBRec.lnTotalTxnAmount <= 0)
                return (VS_ERROR);

	memset(szAuthCodeMsg, 0x00, sizeof(szAuthCodeMsg));
	memset(szAmountMsg, 0x00, sizeof(szAmountMsg));

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 提示檢核金額和授權碼 */
	inDISP_PutGraphic(_CHECK_AUTH_AMT_, 0, _COORDINATE_Y_LINE_8_4_);
	/* 顯示授權碼 */
	sprintf(szAuthCodeMsg, "%s", pobTran->srBRec.szAuthCode);
	inDISP_EnglishFont_Point_Color(szAuthCodeMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_,_COLOR_WHITE_, 11);

	/* 顯示金額 */
	if (pobTran->srBRec.inOrgCode == _CUP_REFUND_	||
	pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
	{
		sprintf(szAmountMsg, "%ld", 0 - pobTran->srBRec.lnTotalTxnAmount);
		inFunc_Amount_Comma(szAmountMsg, "NT$" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	}
	else
	{
		sprintf(szAmountMsg, "%ld", pobTran->srBRec.lnTotalTxnAmount);
		inFunc_Amount_Comma(szAmountMsg, "NT$" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	}

	inDISP_EnglishFont_Point_Color(szAmountMsg, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 8);

	while (1)
	{
		/* 抓取 KioskFlag的值，如為 TRUE 就不用確認畫面 2020/3/6 下午 4:28 [SAM] */
		if(inFunc_GetKisokFlag() == VS_TRUE)
		{
			inDISP_LogPrintfWithFlag(" FU Kiosk Cup ConfVoidTrans Line[%d]", __LINE__);
			szKey = _KEY_0_;
		}else
		{
			szKey = uszKBD_GetKey(_EDC_TIMEOUT_);

			if (szKey == _KEY_0_)
			{
				/* pobTran->uszUpdateBatchBit 表示 uszUpdateBatchBit / TRANS_BATCH_KEY】是要更新記錄 */
				/* 暫時放這裡 */
				pobTran->uszUpdateBatchBit = VS_TRUE;
				pobTran->srBRec.uszVOIDBit = VS_TRUE;
				pobTran->srBRec.inOrgCode = pobTran->srBRec.inCode;
				pobTran->srBRec.inCode =  pobTran->inTransactionCode;

				/* 因ESC需用新的日期為索引,所以重新取得 20181220 [SAM]*/
				memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
				if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
				{
					return (VS_USER_CANCEL);
				}
				inFunc_Sync_BRec_ESC_Date_Time(pobTran, &srRTC);
				/* 因為重新交易需要簽名，所以要把參數設回初始值  20190103 [SAM] */
				pobTran->srBRec.uszF56NoSignatureBit = VS_FALSE;				
				break;
			}
			else if (szKey == _KEY_CANCEL_)
			{
				return (VS_USER_CANCEL);
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				return (VS_TIMEOUT);
			}
		}
	}
        return (VS_SUCCESS);
}



/*
Function        : inCUP_GetPinFunc
Date&Time   : 2019/03/13
Describe        : 重置是否輸入密碼狀態(Void 會紀錄之前輸入密碼的狀態，所以要找一個地方重置)，並輸入CUP密碼
*/
int inCUP_GetPinFunc(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	long	lnCheckAmount = 0;
	char	szPinpadMode[2 + 1] = {0};
	char	szCVMRequiredLimit[12 + 1] = {0};
	char	szTemplate[256 + 1] = {0};

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	if (pobTran->srBRec.uszContactlessBit == VS_TRUE && pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		if (!memcmp(&pobTran->srEMVRec.usz84_DF_NAME[0], "\xA0\x00\x00\x03\x33\x01\x01\x01", pobTran->srEMVRec.in84_DFNameLen))
		{
			/* 閃付PIN及免簽名判斷邏輯.docx 判斷是否為Debit AID，如果為Debit AID就要直接輸入Online PIN */
			/* QuickPass 需要輸入密碼 */
		}
		else
		{
			memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
			memset(szTemplate, 0x00, sizeof(szTemplate));

			/* Quick Pass 有兩個CVM Limit 一個用來判斷免簽 一個用來判斷是否輸入Pin*/
			inGetCVMRequiredLimit(szTemplate);
			memcpy(szCVMRequiredLimit, szTemplate, 10);

			lnCheckAmount = atol(szCVMRequiredLimit);

			/* 閃付EDC判斷邏輯.docx
			 * 額外判斷CVM Required Limit的目的，避免Tx.Amount < CVM Required Limit時(Reader異常，或是卡片Perso錯誤)，被要求輸入密碼
			 * Tag 99 = "00"，表示本交易CVM為Online Pin (目前只有CUP Debit卡片才會有Tag 99) */
			if (pobTran->uszQuickPassTag99 == VS_TRUE && pobTran->srBRec.lnTxnAmount >= lnCheckAmount)
			{

				/* QuickPass 需要輸入密碼 */
			}
			else
			{
				return (VS_SUCCESS);
			}

		}
	}

	inFunc_ResetTitle(pobTran);

	/* 預設為False */
	pobTran->srBRec.uszPinEnterBit = VS_FALSE;

	memset(pobTran->szPIN, 0x00, sizeof(pobTran->szPIN));
	memset(szPinpadMode, 0x00, sizeof(szPinpadMode));
	inGetPinpadMode(szPinpadMode);

	/* 不使用密碼機 */
	if (memcmp(szPinpadMode, _PINPAD_MODE_0_NO_, strlen(_PINPAD_MODE_0_NO_)) == 0)
	{
		inDISP_DispLogAndWriteFlie("不使用密碼機");
	}
	/* 內建密碼機 */
	else if (memcmp(szPinpadMode, _PINPAD_MODE_1_INTERNAL_, strlen(_PINPAD_MODE_1_INTERNAL_)) == 0)
	{
		inDISP_DispLogAndWriteFlie("內建密碼機");

		/* 丟進去轉出來就是BCD了 */
		inRetVal = inNCCC_TMK_CalculatePINBlock(pobTran, pobTran->szPIN);
		if (inRetVal != VS_SUCCESS)
		{
/* 因為不用再顯示金額，但要重新顯示抬頭，就認為已確認完成  20190523  [SAM] */		
#if (_MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_)
			inFunc_ResetTitle(pobTran);
#endif
			/* 抓取 KioskFlag的值，如為 TRUE 就不用確認畫面 2020/3/6 下午 4:28 [SAM] */
			if(inFunc_GetKisokFlag() == VS_TRUE)
			{
				inFunc_ResetTitle(pobTran);
			}
			
			pobTran->srBRec.uszPinEnterBit = VS_FALSE;
			inDISP_DispLogAndWriteFlie(" CUP EnterPin *Error* inRetVal[%d] Line[%d] ",inRetVal ,__LINE__);
			return (inRetVal);
		}
		else
		{
/* 因為不用再顯示金額，但要重新顯示抬頭，就認為已確認完成  20190523  [SAM] */
#if (_MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_)
			inFunc_ResetTitle(pobTran);
#endif
			/* 抓取 KioskFlag的值，如為 TRUE 就不用確認畫面 2020/3/6 下午 4:28 [SAM] */
			if(inFunc_GetKisokFlag() == VS_TRUE)
			{
				inFunc_ResetTitle(pobTran);
			}
			/* 若輸入密碼BYPASS，此Bit不會On */
			if (strlen(pobTran->szPIN) > 0)
			{
				pobTran->srBRec.uszPinEnterBit = VS_TRUE;
			}
			else
			{
				pobTran->srBRec.uszPinEnterBit = VS_FALSE;
			}
			inDISP_DispLogAndWriteFlie(" CUP PinEnterBit[%d] Line[%d] ",pobTran->srBRec.uszPinEnterBit ,__LINE__);
		}

	}
	/* 外接密碼機 */
	else if (memcmp(szPinpadMode, _PINPAD_MODE_2_EXTERNAL_, strlen(_PINPAD_MODE_2_EXTERNAL_)) == 0)
	{
		inDISP_DispLogAndWriteFlie("外接密碼機");
		inDISP_PutGraphic(_CUP_GET_PASSWORD_OUT_, 0, _COORDINATE_Y_LINE_8_4_);
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}


