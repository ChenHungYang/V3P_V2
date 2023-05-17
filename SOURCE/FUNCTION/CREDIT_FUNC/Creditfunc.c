#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <ctos_qrcode.h>

#include "../../INCLUDES/Define_1.h"
#include "../../INCLUDES/Define_2.h"
#include "../../INCLUDES/Transaction.h"
#include "../../INCLUDES/TransType.h"
#include "../../DISPLAY/Display.h"
#include "../../DISPLAY/DispMsg.h"
#include "../../PRINT/Print.h"
#include "../../PRINT/PrtMsg.h"
#include "../../FUNCTION/Function.h"
#include "../../FUNCTION/FILE_FUNC/File.h"
#include "../../FUNCTION/FILE_FUNC/FIleLogFunc.h"
#include "../../COMM/Comm.h"
#include "../../FUNCTION/HDT.h"
#include "../../FUNCTION/CDT.h"
#include "../../FUNCTION/CFGT.h"
#include "../../FUNCTION/EDC.h"
#include "../../FUNCTION/EST.h"
#include "../../FUNCTION/CPT.h"
#include "../../FUNCTION/SCDT.h"
#include "../../FUNCTION/PWD.h"
#include "../../FUNCTION/PCD.h"
#include "../../FUNCTION/ASMC.h"
#include "../../FUNCTION/Accum.h"
#include "../../FUNCTION/Batch.h"

#include "../../../CTLS/CTLS.h"

#include "../../../SOURCE/DISPLAY/DisTouch.h"
#include "../../../SOURCE/COMM/Modem.h"

#include "../../EVENT/MenuMsg.h"

#include "../../EVENT/Flow.h"

#include "Creditfunc.h"

/*
Function        : inFunc_Dial_VoiceLine
Date&Time   : 2019/03/27
Describe        :	由 inNCCC_Func_Dial_VoiceLine 修改來
 */
int inFunc_Dial_VoiceLine(unsigned char *uszNumber, unsigned short usLen)
{
	int inRetVal = VS_SUCCESS;
	int inChoice = _DisTouch_No_Event_;
	char szPABXCode[2 + 1];
	unsigned short usPABXNumberLen = 0;
	unsigned char uszPABXNumber[20 + 1];
	unsigned char uszKey = 0;

	/* 處理PABX START */
	memset(szPABXCode, 0x00, sizeof (szPABXCode));
	inGetPABXCode(szPABXCode);

	memset(uszPABXNumber, 0x00, sizeof (uszPABXNumber));
	memcpy(uszPABXNumber, szPABXCode, strlen(szPABXCode));
	usPABXNumberLen += strlen(szPABXCode);

	memcpy(&uszPABXNumber[usPABXNumberLen], uszNumber, usLen);
	usPABXNumberLen += usLen;
	/* 處理PABX END */

	/* Initial Modem */
	inModem_Initial();
	inDISP_Wait(200);

	/* Flush */
	inModem_Flush_Rx();
	inDISP_Wait(200);

	/* 撥號 */
	inRetVal = inModem_Dial(uszPABXNumber, usPABXNumberLen);
	if (inRetVal == VS_SUCCESS)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_Msg_BMP(_MSG_PLEASE_PICK_UP_PHONE_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 8, "", 0); /* 請拿起話筒 */
		inDISP_PutGraphic(_MSG_PLEASE_PICK_UP_PHONE_, 0, _COORDINATE_Y_LINE_8_6_);
	} else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_Msg_BMP("", 0, _BEEP_1TIMES_MSG_, 5, "撥號失敗", _LINE_8_6_); /* 撥號失敗 */
		inDISP_ChineseFont("撥號失敗", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
	}

	/* 請按確認或清除鍵 */
	/* 請拿起話筒畫面Time Out時間由5分鐘調整為15分鐘。 */
	inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 900);

	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(_Touch_OX_LINE8_8_);
		uszKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (inChoice == _Touch_OX_LINE8_8_CANCEL_BUTTON_ ||
				uszKey == _KEY_CANCEL_ ||
				uszKey == _KEY_TIMEOUT_)
		{
			/* 使用者終止交易 */
			inRetVal = VS_USER_CANCEL;
			break;
		} else if (inChoice == _Touch_OX_LINE8_8_ENTER_BUTTON_ ||
				uszKey == _KEY_ENTER_)
		{
			break;
		} else
		{
			continue;
		}
	}

	/* Flush */
	inModem_Flush_Rx();
	inDISP_Wait(200);

	/* 掛電話 */
	inModem_Hook_On();
	inDISP_Wait(500);
	/* 現在設計不用close */

	return (inRetVal);
}

/*
Function        : inFunc_Disclaim_Auth
Date&Time   : 2019/03/27
Describe        : 請輸入您與銀行聯絡所核准之授權碼，以免遭詐騙被銀行扣款 由 inNCCC_Func_Disclaim_Auth 修改來
 */
int inFunc_Disclaim_Auth(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_SUCCESS;
	int inChoice = _DisTouch_No_Event_;
	unsigned char uszKey = 0;

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_DISCLAIMER_AUTH_, 0, _COORDINATE_Y_LINE_8_4_);
	/* 請按確認或清除鍵 */
	inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	inDISP_Timer_Start_MicroSecond(_TIMER_NEXSYS_2_, 50);

	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(_Touch_OX_LINE8_8_);
		uszKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
		{
			inDISP_BEEP(1, 0);
			inDISP_Timer_Start_MicroSecond(_TIMER_NEXSYS_2_, 50);
		}

		if (inChoice == _Touch_OX_LINE8_8_CANCEL_BUTTON_ ||
				uszKey == _KEY_CANCEL_ ||
				uszKey == _KEY_TIMEOUT_)
		{
			/* 使用者終止交易 */
			inRetVal = VS_USER_CANCEL;
			break;
		} else if (inChoice == _Touch_OX_LINE8_8_ENTER_BUTTON_ ||
				uszKey == _KEY_ENTER_)
		{
			break;
		} else
		{
			continue;
		}
	}

	return (inRetVal);
}

/*
Function        :inFunc_CL_Power_Off
Date&Time       :2019/8/2 上午 10:07
Describe        :此流程一過完卡就關天線，避免沒關天線導致影響到簽名板
 */
int inFunc_CL_Power_Off(TRANSACTION_OBJECT* pobTran)
{
	inCTLS_Power_Off();
	return (VS_SUCCESS);
}

/*
Function        : inFunc_SelectInstRedeemPaymentType
Date&Time   : 
Describe        :  
 */
int inFunc_SelectInstRedeemPaymentType(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_SUCCESS;
	char szCTLSEnable[2 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);

	if (!memcmp(szCTLSEnable, "Y", 1))
	{
		if (pobTran->srBRec.inCode == _INST_SALE_)
		{
			pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
		} else if (pobTran->srBRec.inCode == _REDEEM_SALE_)
		{
			pobTran->inRunOperationID = _OPERATION_REDEEM_SALE_CTLS_;
		} else
		{
			inDISP_DispLogAndWriteFlie(" CRFUNC Select Inst Ctls Incode *Error* InCode[%d]", pobTran->srBRec.inCode);
			return (VS_ERROR);
		}
	} else
	{
		if (pobTran->srBRec.inCode == _INST_SALE_)
		{
			pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
		} else if (pobTran->srBRec.inCode == _REDEEM_SALE_)
		{
			pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
		} else
		{
			inDISP_DispLogAndWriteFlie(" CRFUNC Select Inst No Ctls Incode *Error* InCode[%d]", pobTran->srBRec.inCode);
			return (VS_ERROR);
		}
	}

	/* 因為已經在OPT流程，所以決定交易後要再跑一次OPT流程 20190806 [SAM] */
	if (inFLOW_RunOperation(pobTran, pobTran->inRunOperationID) != VS_SUCCESS)
	{
		return (VS_ERROR);
	} else
	{
		return (VS_SUCCESS);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);

	return (inRetVal);
}

int inFunc_ConvertIncodeToStringWord(int inCode, char szStringBuf)
{
	switch (inCode)
	{
		default:
			break;
	}
	return VS_SUCCESS;
}

/*
Function        : inFunc_InputYearDateTime
Date&Time   : 2022/5/12 上午 9:31
Describe        :
 * [新增預授權完成] 
 */
int inFunc_InputYearDateTime(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;
	char szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT srDispObj;


	while (1)
	{
		memset(&srDispObj, 0x00, sizeof (DISPLAY_OBJECT));
		memset(szTemplate, 0x00, sizeof (szTemplate));

		srDispObj.inMaxLen = 8;
		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;
		strcpy(srDispObj.szPromptMsg, "YYYYMMDD = ");

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 請輸入原交易日期 */
		inDISP_PutGraphic(_GET_ORI_DATE_, 0, _COORDINATE_Y_LINE_8_4_);

		strcpy(szTemplate, srDispObj.szPromptMsg);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_RIGHT_);

		memset(srDispObj.szOutput, 0x00, sizeof (srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (VS_ERROR);

		if (strlen(srDispObj.szOutput) != 8)
		{
			/* 提示 "輸入錯誤" 訊息 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_ERR_PWD_, 0, _COORDINATE_Y_LINE_8_6_);
			inDISP_BEEP(1, 1000);
			continue;
		} else
		{
			/* 檢核日期 */
			if (inFunc_CheckValidDate_Include_Year(srDispObj.szOutput) != VS_SUCCESS)
			{
				/* 提示錯誤訊息 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_ERR_DATE_TIME_, 0, _COORDINATE_Y_LINE_8_6_);
				inDISP_BEEP(1, 1000);
				continue;
			} else
			{
				/* 此欄位沒在使用，借用來當預授權完成的日期使用 */
				memset(pobTran->srBRec.szTableTD_Data, 0x00, sizeof (pobTran->srBRec.szTableTD_Data));
				memcpy(pobTran->srBRec.szTableTD_Data, srDispObj.szOutput, srDispObj.inOutputLen);
				inDISP_LogPrintfWithFlag(" YYMMDD = [%s] Line[%d]", pobTran->srBRec.szTableTD_Data, __LINE__);
				break;
			}
		}
	}

	return (VS_SUCCESS);
}



