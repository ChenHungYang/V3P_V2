#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>

#include "../../INCLUDES/Define_1.h"
#include "../../INCLUDES/Define_2.h"
#include "../../INCLUDES/TransType.h"
#include "../../INCLUDES/Transaction.h"
#include "../../DISPLAY/Display.h"
#include "../../DISPLAY/DispMsg.h"
#include "../../DISPLAY/DisTouch.h"
#include "../../EVENT/MenuMsg.h"

#include "../../FUNCTION/Function.h"
#include "../../FUNCTION/Sqlite.h"
#include "../../FUNCTION/Card.h"
#include "../../FUNCTION/CFGT.h"
#include "../../FUNCTION/HDT.h"
#include "../../FUNCTION/HDPT.h"
#include "../../FUNCTION/PCD.h"
#include "../../FUNCTION/CDT.h"
#include "../../FUNCTION/CARD_FUNC/CardFunction.h"
#include "../../FUNCTION/UNIT_FUNC/TimeUint.h"

#include "../FILE_FUNC/File.h"
#include "../FILE_FUNC/FIleLogFunc.h"

#include "../../EVENT/Flow.h"



#include "CreditProcessDataFunc.h"


extern	int	ginDebug;	/* Debug使用 extern */
extern	int	ginEventCode;
extern	int	ginMenuKeyIn;

/*
Function	:inCREDIT_Func_GetAmount
Date&Time	:2015/8/11 下午 4:27
Describe	:輸入金額
*/
int inCREDIT_Func_GetAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT  srDispObj;

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 若ECR或idle有輸入金額就跳走，OPT已經輸入過 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		if (pobTran->srBRec.lnTxnAmount > 0)
		{
			inDISP_DispLogAndWriteFlie("  Ecr On Amt[%ld] Line[%d] ",pobTran->srBRec.lnTxnAmount, __LINE__);
			inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] ECR END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_SUCCESS);
		}
	}
	else if (pobTran->srBRec.lnTxnAmount > 0)
	{
		inDISP_DispLogAndWriteFlie(" Amt have Value[%ld] Line[%d]",pobTran->srBRec.lnTxnAmount, __LINE__);
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] Amt Not 0 END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 7;			/* 不可超過9，long變數最多放9位 */
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
	srDispObj.inTouchSensorFunc = _Touch_OX_LINE8_8_;
	strcpy(srDispObj.szPromptMsg, "NT$ ");

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 輸入金額 */
	inDISP_PutGraphic(_GET_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);
	inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);

	/* 進入畫面時先顯示金額為0 */
	strcpy(szTemplate, "NT$ 0");
	inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

	inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

	if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

	pobTran->srBRec.lnTxnAmount = atol(srDispObj.szOutput);
	pobTran->srBRec.lnOrgTxnAmount = atol(srDispObj.szOutput);
	pobTran->srBRec.lnTotalTxnAmount = atol(srDispObj.szOutput);

	inDISP_DispLogAndWriteFlie("  Amt After Enter Value[%ld] Line[%d]",pobTran->srBRec.lnTxnAmount, __LINE__);
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_Amount
Date&Time	:2016/12/29 上午 11:20
Describe	:輸入金額
*/
int inCREDIT_Func_Get_OPT_Amount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT  srDispObj;

	/* idle進入 */
	if (ginEventCode >= '1' && ginEventCode <= '9')
	{
		/* 若ECR或idle有輸入金額就跳走，OPT已經輸入過 */
		if (pobTran->uszECRBit == VS_TRUE)
		{
			/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
			if (pobTran->srBRec.lnTxnAmount > 0 || pobTran->uszCardInquiryFirstBit == VS_TRUE)
			{
				return (VS_SUCCESS);
			}
		}

		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMaxLen = 7;			/* 不可超過9，long變數最多放9位 */
		srDispObj.inMenuKeyIn = ginEventCode;
		srDispObj.inCanNotBypass = VS_TRUE;
		srDispObj.inCanNotZero = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
		srDispObj.inTouchSensorFunc = _Touch_OX_LINE8_8_;
		strcpy(srDispObj.szPromptMsg, "NT$ ");

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 輸入金額 */
		inDISP_PutGraphic(_GET_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);
		inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		pobTran->srBRec.lnTxnAmount = atol(srDispObj.szOutput);
		pobTran->srBRec.lnOrgTxnAmount = atol(srDispObj.szOutput);
		pobTran->srBRec.lnTotalTxnAmount = atol(srDispObj.szOutput);
		
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	}

	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetRefundAmount
Date&Time	:2017/1/3 上午 11:05
Describe	:輸入退貨金額
*/
int inCREDIT_Func_GetRefundAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT  srDispObj;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 若ECR或idle有輸入金額就跳走，OPT已經輸入過 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		if (pobTran->srBRec.lnTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
	}
	else if (pobTran->srBRec.lnTxnAmount > 0)
	{

		return (VS_SUCCESS);
	}

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 7;			/* 不可超過9，long變數最多放9位 */
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
	strcpy(srDispObj.szPromptMsg, "NT$ ");

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 輸入金額 */
	inDISP_PutGraphic(_GET_REFUND_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);

	/* 進入畫面時先顯示金額為0 */
	strcpy(szTemplate, "NT$ 0");
	inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

	inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

	if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

	pobTran->srBRec.lnTxnAmount = atol(srDispObj.szOutput);
	pobTran->srBRec.lnOrgTxnAmount = atol(srDispObj.szOutput);
	pobTran->srBRec.lnTotalTxnAmount = atol(srDispObj.szOutput);

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_RefundAmount
Date&Time	:2017/6/28 上午 10:24
Describe	:輸入退貨金額
*/
int inCREDIT_Func_Get_OPT_RefundAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;

	/* 若ECR或idle有輸入金額就跳走，OPT已經輸入過 */
        if (pobTran->uszECRBit == VS_TRUE )
	{
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->srBRec.lnTxnAmount > 0 || pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{

			return (VS_SUCCESS);
		}
	}

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 7;			/* 不可超過9，long變數最多放9位 */
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
	strcpy(srDispObj.szPromptMsg, "NT$ ");

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 輸入金額 */
        inDISP_PutGraphic(_GET_REFUND_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

        pobTran->srBRec.lnTxnAmount = atol(srDispObj.szOutput);
        pobTran->srBRec.lnOrgTxnAmount = atol(srDispObj.szOutput);
        pobTran->srBRec.lnTotalTxnAmount = atol(srDispObj.szOutput);

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetStoreID
Date&Time	:2015/8/11 下午 4:27
Describe	:輸入櫃號
*/
int inCREDIT_Func_GetStoreID(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szMinStoreIDLen[2 + 1];
	char		szMaxStoreIDLen[2 + 1];
	char		szStoreIDEnable[2 + 1];
	DISPLAY_OBJECT  srDispObj;

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
		
	memset(szStoreIDEnable, 0x00, sizeof(szStoreIDEnable));
	inGetStoreIDEnable(szStoreIDEnable);
	if (memcmp(szStoreIDEnable, "Y", strlen("Y")) != 0)
	{
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] StoreId N END -----",__FILE__, __FUNCTION__, __LINE__);	
		return (VS_SUCCESS);
	}

	/* 若ECR發動直接跳走或前面已輸入過櫃號(For idle 進入流程) */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szStoreID) > 0)
		{
			/* 因為不輸入不會清畫面，上個畫面可能會有殘留  */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] ECR Store[%s] END -----",__FILE__, __FUNCTION__, __LINE__, pobTran->srBRec.szStoreID);	
			return (VS_SUCCESS);
		}

	}
	else if (strlen(pobTran->srBRec.szStoreID) > 0)
	{
		/* 因為不輸入不會清畫面，上個畫面可能會有殘留  */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] Value is Exist Store[%s]  END -----",__FILE__, __FUNCTION__, __LINE__, pobTran->srBRec.szStoreID);
		return (VS_SUCCESS);
	}

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szMinStoreIDLen, 0x00, sizeof(szMinStoreIDLen));
	memset(szMaxStoreIDLen, 0x00, sizeof(szMinStoreIDLen));

	inGetMinStoreIDLen(szMinStoreIDLen);
	inGetMaxStoreIDLen(szMaxStoreIDLen);

	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 18;
	srDispObj.inColor = _COLOR_RED_;

	while (1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 輸入櫃號 */
		inDISP_PutGraphic(_GET_STOREID_, 0, _COORDINATE_Y_LINE_8_4_);
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		if (srDispObj.inOutputLen >= 0)
		{
			/* 長度不符合，清空 */
			if (srDispObj.inOutputLen < atoi(szMinStoreIDLen)	||
			    srDispObj.inOutputLen > atoi(szMaxStoreIDLen))
			{
				continue;
			}

			memcpy(&pobTran->srBRec.szStoreID[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
			/* 櫃號不滿18，補空白 */
			if (srDispObj.inOutputLen < 18)
			{
				memset(&pobTran->srBRec.szStoreID[srDispObj.inOutputLen], 0x20, 18 - srDispObj.inOutputLen);
			}
			break;
		}

	}

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] Enter Store[%s] END -----",__FILE__, __FUNCTION__, __LINE__, pobTran->srBRec.szStoreID);
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_StoreID
Date&Time	:2016/12/29 上午 11:11
Describe	:for idle進入流程用，輸入櫃號
*/
int inCREDIT_Func_Get_OPT_StoreID(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szMinStoreIDLen[2 + 1];
	char		szMaxStoreIDLen[2 + 1];
	char		szStoreIDEnable[2 + 1];
        DISPLAY_OBJECT  srDispObj;

	memset(szStoreIDEnable, 0x00, sizeof(szStoreIDEnable));
	inGetStoreIDEnable(szStoreIDEnable);
	if (memcmp(szStoreIDEnable, "Y", strlen("Y")) != 0)
	{
		return (VS_SUCCESS);
	}

	/* 若ECR發動直接跳走或前面已輸入過櫃號(For idle 進入流程) */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (strlen(pobTran->srBRec.szStoreID) > 0 || pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}
	}
	else if (strlen(pobTran->srBRec.szStoreID) > 0)
	{
		return (VS_SUCCESS);
	}

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szMinStoreIDLen, 0x00, sizeof(szMinStoreIDLen));
	memset(szMaxStoreIDLen, 0x00, sizeof(szMinStoreIDLen));

	inGetMinStoreIDLen(szMinStoreIDLen);
	inGetMaxStoreIDLen(szMaxStoreIDLen);


	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 18;
	srDispObj.inColor = _COLOR_RED_;

	while (1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 輸入櫃號 */
		inDISP_PutGraphic(_GET_STOREID_, 0, _COORDINATE_Y_LINE_8_4_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		if (srDispObj.inOutputLen >= 0)
		{
			/* 長度不符合，清空 */
			if (srDispObj.inOutputLen < atoi(szMinStoreIDLen)	||
			    srDispObj.inOutputLen > atoi(szMaxStoreIDLen))
			{
				memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
				continue;
			}

			memcpy(&pobTran->srBRec.szStoreID[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
			/* 櫃號不滿18，補空白 */
			if (srDispObj.inOutputLen < 18)
			{
				memset(&pobTran->srBRec.szStoreID[srDispObj.inOutputLen], 0x20, 18 - srDispObj.inOutputLen);
			}

			break;
		}

	}

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetAuthCode
Date&Time	:2015/8/11 下午 4:27
Describe	:輸入授權碼
		(二十七) 針對 Field 38 Approve Code 欄位，需區分一般信用卡及銀聯交易檢核如下:
			(1)Sale、Preauth 交易 : EDC 不查核未帶 Approve Code 之 CUP 交易，但原五卡交易仍需查核。
			(2)Refund 交易 : 退貨輸入授權碼之 CUP 交易可 Bypass 輸入，但五卡退貨交易需查核至少輸入 2 碼授權碼。
			(3)Preauth Complete 交易:輸入授權碼之 CUP 交易可 Bypass 輸入，但五卡交易需查核至少輸入 2 碼授權碼。

*/
int inCREDIT_Func_GetAuthCode(TRANSACTION_OBJECT *pobTran)
{
	int		i;
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	unsigned char	uszNotValidRetry = VS_FALSE;
	DISPLAY_OBJECT  srDispObj;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* ECR交易檢核 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		for (i = 0; i < strlen(pobTran->srBRec.szAuthCode); i++)
		{
			if (((pobTran->srBRec.szAuthCode[i] >= '0') && (pobTran->srBRec.szAuthCode[i] <= '9')) ||
			    ((pobTran->srBRec.szAuthCode[i] >= 'A') && (pobTran->srBRec.szAuthCode[i] <= 'Z')) ||
			    ((pobTran->srBRec.szAuthCode[i] >= 'a') && (pobTran->srBRec.szAuthCode[i] <= 'z')) ||
			    (pobTran->srBRec.szAuthCode[i] == 0x20))
			{
				continue;
			}
			else
			{
				uszNotValidRetry = VS_TRUE;
				inDISP_Msg_BMP(_ERR_AUTHCODE_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);

				break;
			}
		}

		if (uszNotValidRetry != VS_TRUE)
		{
			/* 分銀聯和非銀聯 */
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				/* CUP沒輸入授權碼補空白 */
				if (strlen(pobTran->srBRec.szAuthCode) == 0)
				{
					memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
					/* 補空白 */
					strcpy(pobTran->srBRec.szAuthCode, "      ");
				}

				return (VS_SUCCESS);
			}
			else
			{
				/* 小於2碼或全0或全空白不給過 */
				if (strlen(pobTran->srBRec.szAuthCode) < 2			||
				    memcmp(pobTran->srBRec.szAuthCode, "000000", 6) == 0	||
				    memcmp(pobTran->srBRec.szAuthCode, "      ", 6) == 0)
				{

				}
				else
				{
					return (VS_SUCCESS);
				}
			}
		}
		else
		{
			/* 含不合法字元，重新輸入 */
		}
	}
	else
	{
		if (strlen(pobTran->srBRec.szAuthCode) > 0)
		{
			return (VS_SUCCESS);
		}
	}

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	if(pobTran->inTransactionCode == _PRE_COMP_)
	{
		srDispObj.inCanNotBypass = VS_FALSE;
		srDispObj.inCanNotZero = VS_FALSE;
	}
		
	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 6;
	srDispObj.inColor = _COLOR_RED_;
	srDispObj.inTouchSensorFunc = _Touch_OX_LINE8_8_;
	/* 授權碼補登，顯示字眼為「請輸入銀行授權碼」，Time Out時間為5分鐘 */
	if (pobTran->srBRec.uszReferralBit == VS_TRUE)
	{
		srDispObj.inTimeout = 300;
	}
	strcpy(srDispObj.szPromptMsg, "APP.CODE= ");

	while (1)
	{	
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 輸入授權碼 */
		inDISP_PutGraphic(_GET_AUTHCODE_, 0, _COORDINATE_Y_LINE_8_4_);

		strcpy(szTemplate, srDispObj.szPromptMsg);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_RIGHT_);
		inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		/* [新增預授權完成] 因預先授權完成可以 by PASS 所以只要長度是0 就自動補空白 2022/11/7 [SAM] */
		if(srDispObj.inOutputLen == 0 && pobTran->inTransactionCode == _PRE_COMP_ )
		{
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
			strcpy(pobTran->srBRec.szAuthCode, "      ");
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			return VS_SUCCESS;
		}

		/* 確認是否授權碼是合法字元 */
		/* 輸入授權碼畫面，若輸入符號及小寫英文字母，應於端末機檢核擋下 */
		uszNotValidRetry = VS_FALSE;
		for (i = 0; i < srDispObj.inOutputLen; i++)
		{
			if (((srDispObj.szOutput[i] >= '0') && (srDispObj.szOutput[i] <= '9')) ||
				((srDispObj.szOutput[i] >= 'A') && (srDispObj.szOutput[i] <= 'Z')) ||
				((srDispObj.szOutput[i] >= 'a') && (srDispObj.szOutput[i] <= 'z')) ||
				(srDispObj.szOutput[i] == 0x20))
			{
				continue;
			}
			else
			{
				uszNotValidRetry = VS_TRUE;
				inDISP_Msg_BMP(_ERR_AUTHCODE_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
				break;
			}

		}
		/* 輸入不合法，重新輸入 */
		if (uszNotValidRetry == VS_TRUE)
		{
			continue;
		}


		/* 非CUP交易別 */
		if (pobTran->srBRec.inCode != _CUP_REFUND_	&&
			pobTran->srBRec.inCode != _CUP_PRE_COMP_)
		{
			/* 不是CUP而且授權碼輸入少於2碼或6個0，不給過 */
			if (srDispObj.inOutputLen < 2 || memcmp(srDispObj.szOutput, "000000", 6) == 0)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("授權碼檢查錯誤", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
				inDISP_BEEP(2, 500);

				continue;
			}
			else
			{
				break;
			}
		}
		/* CUP卡不檢核 */
		else
		{
			break;
		}
	}

	/* CUP沒輸入授權碼補空白 */
        if (pobTran->srBRec.uszCUPTransBit == VS_TRUE && srDispObj.inOutputLen == 0)
        {
                memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
                /* 補空白 */
                strcpy(pobTran->srBRec.szAuthCode, "      ");
        }
        else
        {
                memcpy(&pobTran->srBRec.szAuthCode[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
        }

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);	
	return (VS_SUCCESS);
}


/*
Function	:inCREDIT_Func_Get_OPT_AuthCode
Date&Time	:2017/6/27 下午 4:10
Describe	:輸入授權碼
(二十七) 針對 Field 38 Approve Code 欄位，需區分一般信用卡及銀聯交易檢核如下:
	(1)Sale、Preauth 交易 : EDC 不查核未帶 Approve Code 之 CUP 交易，但原五卡交易仍需查核。
	(2)Refund 交易 : 退貨輸入授權碼之 CUP 交易可 Bypass 輸入，但五卡退貨交易需查核至少輸入 2 碼授權碼。
	(3)Preauth Complete 交易:輸入授權碼之 CUP 交易可 Bypass 輸入，但五卡交易需查核至少輸入 2 碼授權碼。

*/
int inCREDIT_Func_Get_OPT_AuthCode(TRANSACTION_OBJECT *pobTran)
{
	int		i;
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	unsigned char	uszNotValidRetry = VS_FALSE;
	DISPLAY_OBJECT  srDispObj;

	/* ECR交易檢核 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}

		for (i = 0; i < strlen(pobTran->srBRec.szAuthCode); i++)
		{
			if (((pobTran->srBRec.szAuthCode[i] >= '0') && (pobTran->srBRec.szAuthCode[i] <= '9')) ||
			    ((pobTran->srBRec.szAuthCode[i] >= 'A') && (pobTran->srBRec.szAuthCode[i] <= 'Z')) ||
			    ((pobTran->srBRec.szAuthCode[i] >= 'a') && (pobTran->srBRec.szAuthCode[i] <= 'z')) ||
			    (pobTran->srBRec.szAuthCode[i] == 0x20))
			{
				continue;
			}
			else
			{
				uszNotValidRetry = VS_TRUE;
				inDISP_Msg_BMP(_ERR_AUTHCODE_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);

				break;
			}
		}

		if (uszNotValidRetry != VS_TRUE)
		{
			/* 分銀聯和非銀聯 */
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				/* CUP沒輸入授權碼補空白 */
				if (strlen(pobTran->srBRec.szAuthCode) == 0)
				{
					memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
					/* 補空白 */
					strcpy(pobTran->srBRec.szAuthCode, "      ");
				}

				return (VS_SUCCESS);
			}
			else
			{
				/* 小於2碼或全0或全空白不給過 */
				if (strlen(pobTran->srBRec.szAuthCode) < 2			||
				    memcmp(pobTran->srBRec.szAuthCode, "000000", 6) == 0	||
				    memcmp(pobTran->srBRec.szAuthCode, "      ", 6) == 0)
				{

				}
				else
				{
					return (VS_SUCCESS);
				}
			}
		}
		else
		{
			/* 含不合法字元，重新輸入 */
		}
	}

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 6;
	srDispObj.inColor = _COLOR_RED_;
	srDispObj.inTouchSensorFunc = _Touch_OX_LINE8_8_;
	/* 授權碼補登，顯示字眼為「請輸入銀行授權碼」，Time Out時間為5分鐘 */
	if (pobTran->srBRec.uszReferralBit == VS_TRUE)
	{
		srDispObj.inTimeout = 300;
	}
	strcpy(srDispObj.szPromptMsg, "APP.CODE= ");

	while (1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 輸入授權碼 */
		inDISP_PutGraphic(_GET_AUTHCODE_, 0, _COORDINATE_Y_LINE_8_4_);

		strcpy(szTemplate, srDispObj.szPromptMsg);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_RIGHT_);
		inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		/* 確認是否授權碼是合法字元 */
		/* 輸入授權碼畫面，若輸入符號及小寫英文字母，應於端末機檢核擋下 */
		uszNotValidRetry = VS_FALSE;
		for (i = 0; i < srDispObj.inOutputLen; i++)
		{
			if (((srDispObj.szOutput[i] >= '0') && (srDispObj.szOutput[i] <= '9')) ||
			    ((srDispObj.szOutput[i] >= 'A') && (srDispObj.szOutput[i] <= 'Z')) ||
			    ((srDispObj.szOutput[i] >= 'a') && (srDispObj.szOutput[i] <= 'z')) ||
			    (srDispObj.szOutput[i] == 0x20))
			{
				continue;
			}
			else
			{
				uszNotValidRetry = VS_TRUE;
				inDISP_Msg_BMP(_ERR_AUTHCODE_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);

				break;
			}

		}
		/* 輸入不合法，重新輸入 */
		if (uszNotValidRetry == VS_TRUE)
		{
			continue;
		}

		/* 非CUP交易別 */
		if (pobTran->srBRec.inCode != _CUP_REFUND_	&&
		    pobTran->srBRec.inCode != _CUP_PRE_COMP_)
		{
			/* 不是CUP而且授權碼輸入少於2碼或6個0，不給過 */
			if (srDispObj.inOutputLen < 2 || memcmp(srDispObj.szOutput, "000000", 6) == 0)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("授權碼檢查錯誤", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
				inDISP_BEEP(2, 500);
				continue;
			}
			else
			{
				break;
			}
		}
		/* CUP卡不檢核 */
		else
		{
			break;
		}

	}

	/* CUP沒輸入授權碼補空白 */
        if (pobTran->srBRec.uszCUPTransBit == VS_TRUE && srDispObj.inOutputLen == 0)
        {
                memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
                /* 補空白 */
                strcpy(pobTran->srBRec.szAuthCode, "      ");
        }
        else
        {
                memcpy(&pobTran->srBRec.szAuthCode[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
        }

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetTipAmount
Date&Time	:2015/9/16 上午 11:19
Describe	:螢幕顯示原金額，輸入小費金額
*/
int inCREDIT_Func_GetTipAmount(TRANSACTION_OBJECT *pobTran)
{
int		inRetVal, i;
long		lnTipAmount, lnTipPercent, lnTiplimit;
char		szTemplate[_DISP_MSG_SIZE_ + 1];
char		szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];
char		szKey;
DISPLAY_OBJECT  srDispObj;
RTC_NEXSYS		srRTC; 		/* Date & Time */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 取得HDT 小費百分比Tip percent 等於零的話提示錯誤畫面 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTipPercent(szTemplate);
	lnTipPercent = atol(szTemplate);

	if (lnTipPercent == 0L)
	{
		/* 小費百分比錯誤 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_Msg_BMP(_ERR_TIP_RATE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
		sprintf(szErrorMsg, "Tip Percent ： %ld", lnTipPercent);
		inDISP_LogPrintf(szErrorMsg);
	}

	/* ----------------------ECR小費是否超過判斷START-----------------------------------------------------*/
	if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 如果原金額過大（9個9），計算limit會溢位，所以加判斷 */
		if (pobTran->srBRec.lnOrgTxnAmount >= 100)
			lnTiplimit = ((pobTran->srBRec.lnOrgTxnAmount / 100L) * lnTipPercent);
		else
			lnTiplimit = ((pobTran->srBRec.lnOrgTxnAmount * lnTipPercent) / 100L);

		if (ginDebug == VS_TRUE)
		{
			memset(szErrorMsg, 0x00, sizeof (szErrorMsg));
			sprintf(szErrorMsg, "Tip: %ld, TipLimit: %ld", pobTran->srBRec.lnTipTxnAmount, lnTiplimit);
			inDISP_LogPrintf(szErrorMsg);
		}

		/* 若ECR發動直接跳走，且小費不為0 */
		if ((pobTran->srBRec.lnTipTxnAmount != 0) && (pobTran->srBRec.lnTipTxnAmount < lnTiplimit))
		{
			return (VS_SUCCESS);
		}
		else if (pobTran->srBRec.lnTipTxnAmount >= lnTiplimit)
		{
			/* 顯示小費過多 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_Msg_BMP(_ERR_BIG_TIP_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}

	}
	else
	{
		if (pobTran->srBRec.lnTipTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
	}
	/* ----------------------ECR小費是否超過判斷END-----------------------------------------------------*/


	for (i = 0;; i++)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 輸入小費 */
		inDISP_PutGraphic(_GET_TIP_, 0, _COORDINATE_Y_LINE_8_4_);

		/* 第二行原始金額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld",  pobTran->srBRec.lnOrgTxnAmount);
		inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 16, VS_TRUE);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_LEFT_);

		/* 第四行鍵盤輸入小費金額 */
		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMaxLen = 7;
		srDispObj.inCanNotBypass = VS_TRUE;
		srDispObj.inCanNotZero = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
		strcpy(srDispObj.szPromptMsg, "NT$ ");

		/* 進入畫面時先顯示金額為0 */
		strcpy(szTemplate, "NT$ 0");
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		lnTipAmount = atol(srDispObj.szOutput);

		/* 如果原金額過大（9個9），計算limit會溢位，所以加判斷 */
		if (pobTran->srBRec.lnOrgTxnAmount >= 100)
			lnTiplimit = ((pobTran->srBRec.lnOrgTxnAmount / 100L) * lnTipPercent);
		else
			lnTiplimit = ((pobTran->srBRec.lnOrgTxnAmount * lnTipPercent) / 100L);

		if (lnTipAmount > lnTiplimit)
		{
			/* 顯示小費過多 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_Msg_BMP(_ERR_BIG_TIP_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

			continue;
		}
		else
		{
			/* 儲存小費金額 */
			pobTran->srBRec.lnTipTxnAmount = lnTipAmount;
			break;
		}
	}

	/* 總金額 */
	pobTran->srBRec.lnTotalTxnAmount = pobTran->srBRec.lnOrgTxnAmount + pobTran->srBRec.lnTipTxnAmount;

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 確認小費新金額畫面 */
	inDISP_PutGraphic(_CHECK_TIP_, 0, _COORDINATE_Y_LINE_8_4_);
	/* 第三行總金額 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTotalTxnAmount);
	inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 16, VS_TRUE);
	inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_LEFT_);
	inDISP_BEEP(1, 0);

	while (1)
	{
		szKey = uszKBD_GetKey(180);
		if (szKey == _KEY_0_)
		{
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
		}else if (szKey == _KEY_CANCEL_)
			return (VS_USER_CANCEL);
		else if (szKey == _KEY_TIMEOUT_)
			return (VS_TIMEOUT);
		else
			continue;
	}

	pobTran->srBRec.inCode = _TIP_;
	pobTran->inTransactionCode = _TIP_;
	pobTran->uszUpdateBatchBit = VS_TRUE;
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCREDIT_Func_GetTipAmount END!!");
	}

	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetAdjustAmount
Date&Time	:2015/9/16 上午 11:19
Describe	:螢幕顯示原金額，輸入調帳金額
 *		調帳和小費邏輯不同，
 *		小費lnTotalTxnAmount = lnOrgTxnAmount + lnTipTxnAmount = lnTxnAmount + lnTipTxnAmount
 *		調帳lnTotalTxnAmount = lnAdjustTxnAmount調帳後金額基本上和原金額無關
*/
int inCREDIT_Func_GetAdjustAmount(TRANSACTION_OBJECT *pobTran)
{
int		inRetVal, i;
long		lnAdjustAmount;
char		szTemplate[_DISP_MSG_SIZE_ + 1];
unsigned char   uszKey;
DISPLAY_OBJECT  srDispObj;
RTC_NEXSYS		srRTC; 		/* Date & Time */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 若ECR發動直接跳走 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		if (pobTran->srBRec.lnAdjustTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
		else
		{

		}
	}
	else
	{
		if (pobTran->srBRec.lnAdjustTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
	}


	for (i = 0 ;; i++)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 第一行中文提示 */
		inDISP_ChineseFont("原交易金額：", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
		/* 第二行原始金額 */
		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		srDispObj.inY = _LINE_8_5_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMaxLen = 7;
		srDispObj.inCanNotBypass = VS_TRUE;
		srDispObj.inCanNotZero = VS_TRUE;
		sprintf(szTemplate, "NT$ %ld",  pobTran->srBRec.lnOrgTxnAmount);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PAD_RIGHT_FILL_LEFT_);
		inDISP_EnglishFont(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _DISP_LEFT_);
		/* 第三行中文提示 */
		inDISP_ChineseFont("請輸入調帳金額：", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
		/* 第四行鍵盤輸入金額 */
		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMaxLen = 7;
		srDispObj.inCanNotBypass = VS_TRUE;
		srDispObj.inCanNotZero = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
		strcpy(srDispObj.szPromptMsg, "NT$ ");

		/* 進入畫面時先顯示金額為0 */
		strcpy(szTemplate, "NT$ 0");
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PAD_RIGHT_FILL_LEFT_);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_LEFT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		lnAdjustAmount = atol(srDispObj.szOutput);

		if (lnAdjustAmount >= pobTran->srBRec.lnOrgTxnAmount || lnAdjustAmount <= 0)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			/* 調帳金額須小於原金額提示畫面 */
			//inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);
			//inDISP_PutGraphic(_MENU_ADJUST_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);

			/* 調帳金額超過上限 */
			inDISP_Msg_BMP(_ERR_ADJUST_OVER_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
			continue;
		}
		else
		{
			/* 儲存調帳金額 */
			pobTran->srBRec.lnAdjustTxnAmount = lnAdjustAmount;
			break;
		}
	}

	/* 總金額 */
	pobTran->srBRec.lnTotalTxnAmount = pobTran->srBRec.lnAdjustTxnAmount;

#if 1//By Ray For FUBON
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 確認小費新金額畫面 */
	inDISP_PutGraphic(_CHECK_TIP_, 0, _COORDINATE_Y_LINE_8_4_);
	/* 第三行總金額 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTotalTxnAmount);
	inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 16, VS_TRUE);
	inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_LEFT_);
	inDISP_BEEP(1, 0);

	while (1)
	{
		uszKey = uszKBD_GetKey(180);
		if (uszKey == _KEY_0_)
		{
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
		}else if (uszKey == _KEY_CANCEL_)
			return (VS_USER_CANCEL);
		else if (uszKey == _KEY_TIMEOUT_)
			return (VS_TIMEOUT);
		else
			continue;
	}
#else
	/* 確認小費新金額畫面 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	//inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);
	//inDISP_PutGraphic(_MENU_ADJUST_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
	/* 第二行中文提示 */
	inDISP_ChineseFont("調帳後金額", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
	/* 第三行總金額 */
	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	srDispObj.inY = _LINE_8_5_;
	srDispObj.inR_L = _DISP_LEFT_;
	srDispObj.inMaxLen = 7;
	sprintf(szTemplate, "NT$ %ld",  pobTran->srBRec.lnTotalTxnAmount);
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PAD_LEFT_FILL_RIGHT_);
	inDISP_EnglishFont(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _DISP_LEFT_);
	/* 第四行請按確認鍵 */
	inDISP_ChineseFont("請按確認鍵", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
	inDISP_BEEP(1, 0);

	while (1)
	{
		uszKey = uszKBD_Key();

		if (uszKey == _KEY_ENTER_)
			break;
		else if (uszKey == _KEY_CANCEL_)
			return (VS_USER_CANCEL);
		else
			continue;
	}
#endif

	pobTran->srBRec.inCode = _ADJUST_;
	pobTran->inTransactionCode = _ADJUST_;
	pobTran->uszUpdateBatchBit = VS_TRUE ;

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCREDIT_Func_GetAdjustAmount END!!");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_GetPeriod
Date&Time       :2015/12/3 上午 09:46
Describe        :用來輸入分期期數 by bai
*/
int inCREDIT_Func_GetPeriod(TRANSACTION_OBJECT *pobTran)
{
	int             inRetVal;
	char            szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT  srDispObj;

	/* 分期期數ECR沒給的話，要再從EDC輸入期數 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		if (pobTran->srBRec.lnInstallmentPeriod > 0)
		{
			return (VS_SUCCESS);
		}
	}
	else
	{
		if (pobTran->srBRec.lnInstallmentPeriod > 0)
		{
			return (VS_SUCCESS);
		}
	}

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

/* 不可輸入0，不可ByPass */
	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 2;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_GET_INST_PERIOD_, 0, _COORDINATE_Y_LINE_8_4_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

	inRetVal = inDISP_Enter8x16(&srDispObj);

	if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

	pobTran->srBRec.lnInstallmentPeriod = atol(srDispObj.szOutput);

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_OPT_Period
Date&Time       :2017/9/7 下午 3:09
Describe        :用來輸入分期期數
*/
int inCREDIT_Func_Get_OPT_Period(TRANSACTION_OBJECT *pobTran)
{
	int             inRetVal;
	char            szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT  srDispObj;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
		
	inDISP_LogPrintfWithFlag("  EcrBit [%d] Preiod [%ld]", pobTran->uszECRBit, pobTran->srBRec.lnInstallmentPeriod);
	
	/* 分期期數ECR沒給的話，要再從EDC輸入期數 */
	if (pobTran->uszECRBit == VS_TRUE)
	{		
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}else if (pobTran->srBRec.lnInstallmentPeriod > 0)
		{
			return (VS_SUCCESS);
		}
		
	}

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 不可輸入0，不可ByPass */
	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 2;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_GET_INST_PERIOD_, 0, _COORDINATE_Y_LINE_8_4_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

	inRetVal = inDISP_Enter8x16(&srDispObj);

	if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

	pobTran->srBRec.lnInstallmentPeriod = atol(srDispObj.szOutput);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetDownPayment
Date&Time	:2016/6/13 下午 3:33
Describe	:請輸入首期金額
*/
int inCREDIT_Func_GetDownPayment(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT  srDispObj;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
		
	inDISP_LogPrintfWithFlag("  EcrBit [%d] DownPayment [%ld]", pobTran->uszECRBit, pobTran->srBRec.lnInstallmentDownPayment);
	
	/* 若ECR發動直接跳走 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		/*  因為富邦規格,ECR 如果沒帶就要輸入，所以新增條件 20190121 [SAM] */
		if(pobTran->srBRec.lnInstallmentDownPayment > 0)
		{
			return (VS_SUCCESS);
		}
	}
	else
	{
		if (pobTran->inRunOperationID == _OPERATION_INST_REFUND_CTLS_	||
		    pobTran->inRunOperationID == _OPERATION_INST_ADJUST_CTLS_)
		{
			return (VS_SUCCESS);
		}
	}

	if(pobTran->inRunOperationID == _OPERATION_INST_ADJUST_)
	{
		if(pobTran->srBRec.lnInstallmentPeriod > 0)
			pobTran->srBRec.lnInstallmentDownPayment = pobTran->srBRec.lnTxnAmount / pobTran->srBRec.lnInstallmentPeriod + pobTran->srBRec.lnTxnAmount % pobTran->srBRec.lnInstallmentPeriod;
		else
			pobTran->srBRec.lnInstallmentDownPayment = pobTran->srBRec.lnTxnAmount;

		return (VS_SUCCESS);
	}

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 可輸入0，可ByPass */
	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 7;
	srDispObj.inColor = _COLOR_RED_;
	strcpy(srDispObj.szPromptMsg, "NT$ ");

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 輸入首期金額 */
	inDISP_PutGraphic(_GET_INST_DOWN_PAYMENT_, 0, _COORDINATE_Y_LINE_8_4_);

	/* 進入畫面時先顯示金額為0 */
	strcpy(szTemplate, "NT$ 0");
	inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

	inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

	if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

	pobTran->srBRec.lnInstallmentDownPayment = atol(srDispObj.szOutput);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_DownPayment
Date&Time	:2017/9/7 下午 3:58
Describe	:請輸入首期金額
*/
int inCREDIT_Func_Get_OPT_DownPayment(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT  srDispObj;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
		
	inDISP_LogPrintfWithFlag("  EcrBit [%d] DownPayment [%ld]", pobTran->uszECRBit, pobTran->srBRec.lnInstallmentDownPayment);
	
	/* 若ECR發動直接跳走 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}else if(pobTran->srBRec.lnInstallmentDownPayment > 0) /*  因為富邦規格,ECR 如果沒帶就要輸入，所以新增條件 20190121 [SAM] */
		{
			return (VS_SUCCESS);

		}		
	}

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 可輸入0，可ByPass */
	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 7;
	srDispObj.inColor = _COLOR_RED_;
	strcpy(srDispObj.szPromptMsg, "NT$ ");

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 輸入首期金額 */
	inDISP_PutGraphic(_GET_INST_DOWN_PAYMENT_, 0, _COORDINATE_Y_LINE_8_4_);

	/* 進入畫面時先顯示金額為0 */
	strcpy(szTemplate, "NT$ 0");
	inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

	inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

	if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

	pobTran->srBRec.lnInstallmentDownPayment = atol(srDispObj.szOutput);
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetInstPayment
Date&Time	:2016/6/13 下午 3:48
Describe	:請輸入每期金額
*/
int inCREDIT_Func_GetInstPayment(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT  srDispObj;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
		
	inDISP_LogPrintfWithFlag("  EcrBit [%d] InstallPayment [%ld]", pobTran->uszECRBit, pobTran->srBRec.lnInstallmentPayment);
	/* 若ECR發動且有值就直接跳走 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		if (pobTran->srBRec.lnInstallmentPayment > 0)
		{
			return (VS_SUCCESS);
		}
	}
	else
	{
		if (pobTran->srBRec.lnInstallmentPayment > 0)
		{
			return (VS_SUCCESS);
		}
	}

	if(pobTran->inRunOperationID == _OPERATION_INST_ADJUST_)
	{
		if(pobTran->srBRec.lnInstallmentPeriod > 0)
			pobTran->srBRec.lnInstallmentPayment = pobTran->srBRec.lnTxnAmount / pobTran->srBRec.lnInstallmentPeriod;
		else
			pobTran->srBRec.lnInstallmentPayment = 0;

		return (VS_SUCCESS);
	}

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 不可輸入0，不可ByPass */
	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 7;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;

	strcpy(srDispObj.szPromptMsg, "NT$ ");

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請輸入每期金額 */
	inDISP_PutGraphic(_GET_INST_PAYMENT_, 0, _COORDINATE_Y_LINE_8_4_);

	/* 進入畫面時先顯示金額為0 */
	strcpy(szTemplate, "NT$ 0");
	inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

	inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

	if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	pobTran->srBRec.lnInstallmentPayment = atol(srDispObj.szOutput);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_InstPayment
Date&Time	:2017/9/7 下午 4:04
Describe	:請輸入每期金額
*/
int inCREDIT_Func_Get_OPT_InstPayment(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT  srDispObj;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
		
	inDISP_LogPrintfWithFlag("  EcrBit [%d] InstallPayment [%ld]", pobTran->uszECRBit, pobTran->srBRec.lnInstallmentPayment);
	/* 若ECR發動且有值就直接跳走 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}else if (pobTran->srBRec.lnInstallmentPayment > 0)
		{
			return (VS_SUCCESS);
		}
	}

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 不可輸入0，不可ByPass */
	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 7;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;

	strcpy(srDispObj.szPromptMsg, "NT$ ");

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請輸入每期金額 */
	inDISP_PutGraphic(_GET_INST_PAYMENT_, 0, _COORDINATE_Y_LINE_8_4_);

	/* 進入畫面時先顯示金額為0 */
	strcpy(szTemplate, "NT$ 0");
	inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

	inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

	if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	pobTran->srBRec.lnInstallmentPayment = atol(srDispObj.szOutput);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetInstFee
Date&Time	:2016/6/13 下午 3:48
Describe	:請輸入手續費
*/
int inCREDIT_Func_GetInstFee(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;

	/* 若ECR發動直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
        {

	}
	else
	{
		if (pobTran->inRunOperationID == _OPERATION_INST_REFUND_CTLS_	||
		    pobTran->inRunOperationID == _OPERATION_INST_ADJUST_CTLS_)
		{
			return (VS_SUCCESS);
		}
		else
		{

		}
	}

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 可輸入0，也可以ByPass */
        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 7;
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請輸入手續費 */
	inDISP_PutGraphic(_GET_INST_FEE_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

        pobTran->srBRec.lnInstallmentFormalityFee = atol(srDispObj.szOutput);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_InstFee
Date&Time	:2017/9/7 下午 4:04
Describe	:請輸入手續費
*/
int inCREDIT_Func_Get_OPT_InstFee(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;

	/* 若ECR發動直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
			/*   20190121 [SAM] */
		if (pobTran->srBRec.lnInstallmentFormalityFee > 0)
		{
			return (VS_SUCCESS);
		}else
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}
		
	}

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 可輸入0，也可以ByPass */
        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 7;
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請輸入手續費 */
	inDISP_PutGraphic(_GET_INST_FEE_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        pobTran->srBRec.lnInstallmentFormalityFee = atol(srDispObj.szOutput);

	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetPayAmount
Date&Time	:2016/6/14 上午 9:40
Describe	:請輸入實付金額
*/
int inCREDIT_Func_GetPayAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT  srDispObj;

	/* 若ECR發動直接跳走 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		if(pobTran->srBRec.lnRedemptionPaidCreditAmount >0)
		{
				return (VS_SUCCESS);
		}
	}
	else
	{
		/* 目前富邦不會有紅利退貨感應交易，但佳昌有寫，就先保留 20190122 [SAM] */
		if (pobTran->inRunOperationID == _OPERATION_REDEEM_REFUND_CTLS_	||
			pobTran->inRunOperationID == _OPERATION_REDEEM_ADJUST_CTLS_)
		{
			return (VS_SUCCESS);
		}
	}

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 可輸入0，也可以ByPass */
	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 7;
	srDispObj.inColor = _COLOR_RED_;
	strcpy(srDispObj.szPromptMsg, "NT$ ");

	while (1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 請輸入實付金額 */
		inDISP_PutGraphic(_GET_REDEEM_PAY_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);

		/* 進入畫面時先顯示金額為0 */
		strcpy(szTemplate, "NT$ 0");
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		/* 不能等於或大於 */
		if (atol(srDispObj.szOutput) < pobTran->srBRec.lnTxnAmount)
		{
			break;
		}

	}

	pobTran->srBRec.lnRedemptionPaidCreditAmount = atol(srDispObj.szOutput);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_PayAmount
Date&Time	:2017/9/7 下午 6:27
Describe	:請輸入支付金額
*/
int inCREDIT_Func_Get_OPT_PayAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT  srDispObj;

	/* 若ECR發動直接跳走 */
	
	if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}else if(pobTran->srBRec.lnRedemptionPaidCreditAmount > 0)/*  因為富邦規格,ECR 如果沒帶就要輸入，所以新增條件 20190121 [SAM] */
		{
			return (VS_SUCCESS);
		}
	}

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 可輸入0，也可以ByPass */
	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 7;
	srDispObj.inColor = _COLOR_RED_;
	strcpy(srDispObj.szPromptMsg, "NT$ ");

	while (1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 請輸入實付金額 */
		inDISP_PutGraphic(_GET_REDEEM_PAY_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);

		/* 進入畫面時先顯示金額為0 */
		strcpy(szTemplate, "NT$ 0");
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		/* 不能等於或大於 */
		if (atol(srDispObj.szOutput) < pobTran->srBRec.lnTxnAmount)
		{
			break;
		}

	}
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	pobTran->srBRec.lnRedemptionPaidCreditAmount = atol(srDispObj.szOutput);

	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetPaidAmount
Date&Time	: 2019/01/22 
Describe	: 請輸入支付金額
*/
int inCREDIT_Func_GetPaidAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT  srDispObj;

	/* 若ECR發動直接跳走 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		if (pobTran->srBRec.lnRedemptionPaidAmount > 0)
		{
			return (VS_SUCCESS);
		}
	}
	else
	{
		/* 目前富邦不會有紅利退貨感應交易，但佳昌有寫，就先保留 20190122 [SAM] */
		if (pobTran->inRunOperationID == _OPERATION_REDEEM_REFUND_CTLS_	||
		    pobTran->inRunOperationID == _OPERATION_REDEEM_ADJUST_CTLS_)
		{
			return (VS_SUCCESS);
		}
		
	}

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 可輸入0，也可以ByPass */
	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 7;
	srDispObj.inColor = _COLOR_RED_;
	strcpy(srDispObj.szPromptMsg, "NT$ ");

	while (1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 請輸入折抵金額 */
		inDISP_PutGraphic(_GET_REDEEM_PAID_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);

		/* 進入畫面時先顯示金額為0 */
		strcpy(szTemplate, "NT$ 0");
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
		srDispObj.inCanNotZero = VS_TRUE;
		srDispObj.inCanNotBypass = VS_TRUE;

		inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		/* 不能等於或大於 */
		if (atol(srDispObj.szOutput) <= pobTran->srBRec.lnTxnAmount)
		{
			break;
		}

	}

	pobTran->srBRec.lnRedemptionPaidAmount = atol(srDispObj.szOutput);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_PaidAmount
Date&Time	:2017/9/7 下午 6:27
Describe	:請輸入支付金額
*/
int inCREDIT_Func_Get_OPT_PaidAmount(TRANSACTION_OBJECT *pobTran)
{
	int 	inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT	srDispObj;

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inCREDIT_Func_Get_OPT_PaidAmount 1");

			/* 若ECR發動直接跳走 */
	
	if (pobTran->uszECRBit == VS_TRUE )
	{
		/*  因為富邦規格,ECR 如果沒帶就要輸入，所以新增條件 20190121 [SAM] */
		if(pobTran->srBRec.lnRedemptionPaidAmount > 0)
		{
			return (VS_SUCCESS);
		}else/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}

	}

		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("inCREDIT_Func_Get_OPT_PaidAmount 2");

		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 可輸入0，也可以ByPass */
		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMaxLen = 7;
	srDispObj.inColor = _COLOR_RED_;
		strcpy(srDispObj.szPromptMsg, "NT$ ");

	while (1)
	{

		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("inCREDIT_Func_Get_OPT_PaidAmount 3");
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 請輸入折抵金額 */
		inDISP_PutGraphic(_GET_REDEEM_PAID_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);

		/* 進入畫面時先顯示金額為0 */
		strcpy(szTemplate, "NT$ 0");
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		/* 不能等於或大於 */
		if (atol(srDispObj.szOutput) <= pobTran->srBRec.lnTxnAmount)
		{
			break;
		}

	}


		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("inCREDIT_Func_Get_OPT_PaidAmount 4");
		pobTran->srBRec.lnRedemptionPaidAmount = atol(srDispObj.szOutput);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetRedeemPoint
Date&Time	:2016/6/14 上午 9:41
Describe	:請輸入扣抵紅利點數
*/
int inCREDIT_Func_GetRedeemPoint(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT  srDispObj;

	/* 若ECR發動而且有值就直接跳走 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		if (pobTran->srBRec.lnRedemptionPoints > 0)
		{
			return (VS_SUCCESS);
		}

	}
	else
	{
		if (pobTran->srBRec.lnRedemptionPoints > 0)
		{
			return (VS_SUCCESS);
		}
	}

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 不可輸入0，不可ByPass */
	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 8;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請輸入扣抵紅利點數 */
	inDISP_PutGraphic(_GET_REDEEM_POINT_, 0, _COORDINATE_Y_LINE_8_4_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

	inRetVal = inDISP_Enter8x16(&srDispObj);

	if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (VS_ERROR);

	pobTran->srBRec.lnRedemptionPoints = atol(srDispObj.szOutput);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_RedeemPoint
Date&Time	:2017/9/7 下午 6:27
Describe	:請輸入扣抵紅利點數
*/
int inCREDIT_Func_Get_OPT_RedeemPoint(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;

		if((inRetVal = inCREDIT_Func_Get_OPT_PaidAmount(pobTran)) != VS_SUCCESS)
			return(inRetVal);

	/* 若ECR發動而且有值就直接跳走 */
	if (pobTran->uszECRBit == VS_TRUE )
	{
		if (pobTran->srBRec.lnRedemptionPoints > 0)
		{
			return (VS_SUCCESS);
		}
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		else if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}
	}

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 不可輸入0，不可ByPass */
        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 8;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請輸入扣抵紅利點數 */
	inDISP_PutGraphic(_GET_REDEEM_POINT_, 0, _COORDINATE_Y_LINE_8_4_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

        inRetVal = inDISP_Enter8x16(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (VS_ERROR);

        pobTran->srBRec.lnRedemptionPoints = atol(srDispObj.szOutput);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_CheckResult
Date&Time       :2016/4/1 上午 9:53
Describe        :交易確認畫面，for 電子簽名用，避免直接把電子簽名bypass掉
*/
int inCREDIT_Func_CheckResult(TRANSACTION_OBJECT *pobTran)
{
	char	szKey;
	char	szDispBuf[_DISP_MSG_SIZE_ + 1];
	char    szAmountMsg[_DISP_MSG_SIZE_ + 1];
	char	szSignPadMode[2 + 1];

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, " CREDIT_2128 Func CheckResult  INIT" );	
	
	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, " CREDIT_2128 Func CheckResult END");

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	/* 等規格確定後再開 */
	return (VS_SUCCESS);

	inFunc_ResetTitle(pobTran);
	
        /* CFGT的Signpad開關判斷 */
        memset(szSignPadMode , 0x00, sizeof(szSignPadMode));
        inGetSignPadMode(szSignPadMode);

	/* 不開signpad */
        if (!memcmp(szSignPadMode, _SIGNPAD_MODE_0_NO_, 1))
	{
                return (VS_SUCCESS);
	}

	if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
	{
		/* 顯示免簽的確認圖 */
		inDISP_PutGraphic(_SIGNPAD_NO_SIGN_, 0, _COORDINATE_Y_LINE_8_4_);

		/* 授權碼 */
		memset(szDispBuf, 0x00, sizeof(szDispBuf));
		sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
		inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _COLOR_WHITE_, 11);

		/* 交易金額 */
		memset(szAmountMsg, 0x00, sizeof(szAmountMsg));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE &&
		    (pobTran->srBRec.inOrgCode != _REFUND_ && pobTran->srBRec.inOrgCode != _INST_REFUND_ && pobTran->srBRec.inOrgCode != _REDEEM_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_MAIL_ORDER_REFUND_))	||
		     pobTran->srBRec.inCode == _REFUND_		||
		     pobTran->srBRec.inCode == _INST_REFUND_	||
		     pobTran->srBRec.inCode == _REDEEM_REFUND_	||
		     pobTran->srBRec.inCode == _CUP_REFUND_	||
		     pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
		{
			sprintf(szAmountMsg, "%ld", 0 - pobTran->srBRec.lnTotalTxnAmount);
		}
		else
		{
			sprintf(szAmountMsg, "%ld", pobTran->srBRec.lnTotalTxnAmount);
		}
		inFunc_Amount_Comma(szAmountMsg, " NT$ " , ' ', _SIGNED_NONE_, 18, _PAD_LEFT_FILL_RIGHT_);
		inDISP_EnglishFont_Point_Color(szAmountMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _COLOR_WHITE_, 1);

		while (1)
		{
			szKey = uszKBD_GetKey(30);

			if (szKey == _KEY_ENTER_)
			{
				break;
			}
			/* 不接受清除鍵，timeout 預設為確認 */
			else if (szKey == _KEY_TIMEOUT_)
			{
				break;
			}

		}

	}
//	else if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
//	{
//		/* 顯示分期的確認圖 */
//		inDISP_PutGraphic(_SIGNPAD_INST_REDEEM_, 0, 180);
//
//		/* 授權碼 */
//		memset(szDispBuf, 0x00, sizeof(szDispBuf));
//		sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
//		inDISP_EnglishFont_Point_Color(szDispBuf, _FONESIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _COLOR_WHITE_, 11);
//
//		/* 交易金額 */
//		memset(szAmountMsg, 0x00, sizeof(szAmountMsg));
//		sprintf(szAmountMsg, "%ld", pobTran->srBRec.lnTotalTxnAmount);
//		inFunc_Amount_Comma(szAmountMsg, " NT$ " , 18, _LEFT_);
//		inDISP_EnglishFont_Point_Color(szAmountMsg, _FONESIZE_8X16_, _LINE_8_7_, _COLOR_RED_, _COLOR_WHITE_, 1);
//
//		while (1)
//		{
//			szKey = szKBD_GetKey(30);
//
//			if (szKey == _KEY_ENTER_)
//			{
//				break;
//			}
//			/* 不接受清除鍵，timeout 預設為確認 */
//			else if (szKey == _KEY_TIMEOUT_)
//			{
//				break;
//			}
//
//		}
//
//		inDISP_PutGraphic(_SIGNPAD_INST_, 0, 180);
//
//		/* 期數 */
//		memset(szDispBuf, 0x00, sizeof(szDispBuf));
//		sprintf(szDispBuf, "%ld", pobTran->srBRec.lnInstallmentPeriod);
//		inDISP_EnglishFont_Point_Color(szDispBuf, _FONESIZE_8X16_, _LINE_8_4_, _COLOR_RED_, _COLOR_WHITE_, 11);
//		/* 首期金額 */
//		memset(szAmountMsg, 0x00, sizeof(szAmountMsg));
//		sprintf(szAmountMsg, "%ld", pobTran->srBRec.lnInstallmentDownPayment);
//		inFunc_Amount_Comma(szAmountMsg, " NT$ " , 18, _LEFT_);
//		inDISP_EnglishFont_Point_Color(szAmountMsg, _FONESIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _COLOR_WHITE_, 9);
//		/* 每期金額 */
//		memset(szAmountMsg, 0x00, sizeof(szAmountMsg));
//		sprintf(szAmountMsg, "%ld", pobTran->srBRec.lnInstallmentPayment);
//		inFunc_Amount_Comma(szAmountMsg, " NT$ " , 18, _LEFT_);
//		inDISP_EnglishFont_Point_Color(szAmountMsg, _FONESIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _COLOR_WHITE_, 9);
//		/* 手續費 */
//		memset(szAmountMsg, 0x00, sizeof(szAmountMsg));
//		sprintf(szAmountMsg, "%ld", pobTran->srBRec.lnInstallmentFormalityFee);
//		inFunc_Amount_Comma(szAmountMsg, " NT$ " , 18, _LEFT_);
//		inDISP_EnglishFont_Point_Color(szAmountMsg, _FONESIZE_8X16_, _LINE_8_7_, _COLOR_RED_, _COLOR_WHITE_, 9);
//
//		while (1)
//		{
//			szKey = szKBD_GetKey(30);
//
//			if (szKey == _KEY_0_)
//			{
//				break;
//			}
//			/* 不接受清除鍵，timeout 預設為確認 */
//			else if (szKey == _KEY_TIMEOUT_)
//			{
//				break;
//			}
//
//		}
//
//	}
//	else if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
//	{
//		/* 顯示紅利的確認圖 */
//		inDISP_PutGraphic(_SIGNPAD_INST_REDEEM_, 0, 180);
//
//		/* 授權碼 */
//		memset(szDispBuf, 0x00, sizeof(szDispBuf));
//		sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
//		inDISP_EnglishFont_Point_Color(szDispBuf, _FONESIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _COLOR_WHITE_, 11);
//
//		/* 交易金額 */
//		memset(szAmountMsg, 0x00, sizeof(szAmountMsg));
//		sprintf(szAmountMsg, "%ld", pobTran->srBRec.lnTotalTxnAmount);
//		inFunc_Amount_Comma(szAmountMsg, " NT$ " , 18, _LEFT_);
//		inDISP_EnglishFont_Point_Color(szAmountMsg, _FONESIZE_8X16_, _LINE_8_7_, _COLOR_RED_, _COLOR_WHITE_, 1);
//
//		while (1)
//		{
//			szKey = szKBD_GetKey(30);
//
//			if (szKey == _KEY_ENTER_)
//			{
//				break;
//			}
//			/* 不接受清除鍵，timeout 預設為確認 */
//			else if (szKey == _KEY_TIMEOUT_)
//			{
//				break;
//			}
//
//		}
//
//		inDISP_PutGraphic(_SIGNPAD_REDEEM_, 0, 180);
//
//		/* 支付金額 */
//		memset(szDispBuf, 0x00, sizeof(szDispBuf));
//		sprintf(szDispBuf, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount);
//		inDISP_EnglishFont_Point_Color(szDispBuf, _FONESIZE_8X16_, _LINE_8_4_, _COLOR_RED_, _COLOR_WHITE_, 11);
//		/* 扣抵金額 */
//		memset(szDispBuf, 0x00, sizeof(szDispBuf));
//		sprintf(szDispBuf, "%ld", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
//		inDISP_EnglishFont_Point_Color(szDispBuf, _FONESIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _COLOR_WHITE_, 11);
//		/* 扣抵點數 */
//		memset(szDispBuf, 0x00, sizeof(szDispBuf));
//		sprintf(szDispBuf, "%ld", pobTran->srBRec.lnRedemptionPoints);
//		inDISP_EnglishFont_Point_Color(szDispBuf, _FONESIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _COLOR_WHITE_, 11);
//		/* 剩餘點數 */
//		memset(szDispBuf, 0x00, sizeof(szDispBuf));
//		sprintf(szDispBuf, "%ld", pobTran->srBRec.lnRedemptionPointsBalance);
//		inDISP_EnglishFont_Point_Color(szDispBuf, _FONESIZE_8X16_, _LINE_8_7_, _COLOR_RED_, _COLOR_WHITE_, 11);
//
//		while (1)
//		{
//			szKey = szKBD_GetKey(30);
//
//			if (szKey == _KEY_0_)
//			{
//				break;
//			}
//			/* 不接受清除鍵，timeout 預設為確認 */
//			else if (szKey == _KEY_TIMEOUT_)
//			{
//				break;
//			}
//
//		}
//	}
	else
	{
		inDISP_PutGraphic(_SIGNPAD_SALE_, 0, _COORDINATE_Y_LINE_8_4_);

		/* 授權碼 */
		memset(szDispBuf, 0x00, sizeof(szDispBuf));
		sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
		inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _COLOR_WHITE_, 11);

		/* 交易金額 */
		memset(szAmountMsg, 0x00, sizeof(szAmountMsg));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE	&&
		    (pobTran->srBRec.inOrgCode != _REFUND_ && pobTran->srBRec.inOrgCode != _INST_REFUND_ && pobTran->srBRec.inOrgCode != _REDEEM_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_MAIL_ORDER_REFUND_))	||
		     pobTran->srBRec.inCode == _REFUND_		||
		     pobTran->srBRec.inCode == _INST_REFUND_	||
		     pobTran->srBRec.inCode == _REDEEM_REFUND_	||
		     pobTran->srBRec.inCode == _CUP_REFUND_	||
		     pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
		{
			sprintf(szAmountMsg, "%ld", 0 - pobTran->srBRec.lnTotalTxnAmount);
		}
		else
		{
			sprintf(szAmountMsg, "%ld", pobTran->srBRec.lnTotalTxnAmount);
		}
		inFunc_Amount_Comma(szAmountMsg, " NT$ " , ' ', _SIGNED_NONE_, 18, _PAD_LEFT_FILL_RIGHT_);
		inDISP_EnglishFont_Point_Color(szAmountMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _COLOR_WHITE_, 1);

		while (1)
		{
			szKey = uszKBD_GetKey(30);

			if (szKey == _KEY_ENTER_)
			{
				break;
			}
			/* 不接受清除鍵，timeout 預設為確認 */
			else if (szKey == _KEY_TIMEOUT_)
			{
				break;
			}

		}
	}
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_GetOriTransDate
Date&Time       :2016/9/21 下午 5:40
Describe        :請輸入原交易日期
*/
int inCREDIT_Func_GetOriTransDate(TRANSACTION_OBJECT *pobTran)
{
        int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        char		szDebugMsg[100 + 1];
        DISPLAY_OBJECT  srDispObj;

        /* 若ECR發動而且有值就直接跳走 */
	/* 目前ECR規格不會帶此資料，所以改成都用手輸  2022/12/14 [SAM]*/
//        if (pobTran->uszECRBit == VS_TRUE)
//        {
//                if (strlen(pobTran->srBRec.szCUP_TD) > 0 && pobTran->srBRec.szCUP_TD[0] != 0x20)
//                {
//                        return (VS_SUCCESS);
//                }
//
//        }
//        else
        {
	/* 因為這些條件在跑 OPT時已經有做過，所以在 TRT 時不需要再輸入
	 * 拿掉  _OPERATION_PRE_COMP_CTLS_ ,這個交易不會在OPT流程輸入 2022/12/14 [SAM] */
                if (pobTran->inRunOperationID == _OPERATION_REFUND_AMOUNT_FIRST_CUP_	||
                    pobTran->inRunOperationID == _OPERATION_REFUND_CTLS_CUP_)
                {
                        return (VS_SUCCESS);
                }
        }


        while (1)
        {
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                memset(szTemplate, 0x00, sizeof(szTemplate));

                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
                srDispObj.inMaxLen = 4;
                srDispObj.inCanNotBypass = VS_TRUE;
                srDispObj.inColor = _COLOR_RED_;
                strcpy(srDispObj.szPromptMsg, "MMDD = ");

                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                /* 請輸入原交易日期 */
                inDISP_PutGraphic(_GET_ORI_DATE_, 0, _COORDINATE_Y_LINE_8_4_);

                strcpy(szTemplate, srDispObj.szPromptMsg);
                inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_RIGHT_);

                memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
                srDispObj.inOutputLen = 0;

                inRetVal = inDISP_Enter8x16(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                    return (inRetVal);
                else if(strlen(srDispObj.szOutput) != 4)
                    continue;

                memset(pobTran->srBRec.szCUP_TD, 0x00, sizeof(pobTran->srBRec.szCUP_TD));
                memcpy(pobTran->srBRec.szCUP_TD, srDispObj.szOutput, srDispObj.inOutputLen);

                if (inFunc_CheckValidOriDate(pobTran->srBRec.szCUP_TD) == VS_SUCCESS)
                {
                        break;
                }
                else
                {
                        if (ginDebug == VS_TRUE)
                        {
                                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                sprintf(szDebugMsg, "Date Not Valid: %s", pobTran->srBRec.szCUP_TD);
                                inDISP_LogPrintf(szDebugMsg);
                        }
                }

        }

//        if(pobTran->inRunOperationID == _OPERATION_PRE_COMP_)
                if((inRetVal = inCREDIT_Func_Get_OPT_CUP_TN(pobTran)) != VS_SUCCESS)
                        return(inRetVal);
        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_OPT_OriTransDate
Date&Time       :2017/6/28 上午 10:34
Describe        :
*/
int inCREDIT_Func_Get_OPT_OriTransDate(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	char		szDebugMsg[100 + 1];
	DISPLAY_OBJECT  srDispObj;

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inCREDIT_Func_Get_OPT_OriTransDate 1,%d",pobTran->inRunOperationID);

	if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 目前ECR規格不會帶此資料，所以改成都用手輸  2022/12/14 [SAM]*/
//		if (strlen(pobTran->srBRec.szCUP_TD) > 0)
//		{
//			return (VS_SUCCESS);
//		}		
//		else 
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}
		else
		{

		}
	}

	while (1)
	{
		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMaxLen = 4;
		srDispObj.inCanNotBypass = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
		strcpy(srDispObj.szPromptMsg, "MMDD = ");

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 請輸入原交易日期 */
		inDISP_PutGraphic(_GET_ORI_DATE_, 0, _COORDINATE_Y_LINE_8_4_);

		strcpy(szTemplate, srDispObj.szPromptMsg);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_RIGHT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);
		else if(strlen(srDispObj.szOutput) != 4)
			continue;

		memset(pobTran->srBRec.szCUP_TD, 0x00, sizeof(pobTran->srBRec.szCUP_TD));
		memcpy(pobTran->srBRec.szCUP_TD, srDispObj.szOutput, srDispObj.inOutputLen);

		if (inFunc_CheckValidOriDate(pobTran->srBRec.szCUP_TD) == VS_SUCCESS)
		{
			break;
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Date Not Valid: %s", pobTran->srBRec.szCUP_TD);
				inDISP_LogPrintf(szDebugMsg);
			}
		}

	}
	
	/* 因 CUP 退貨需要 CUP Serial number ，新增 _OPERATION_REFUND_AMOUNT_FIRST_CUP_ 條件 2022/11/7 [SAM] */
	if(pobTran->inRunOperationID == _OPERATION_REFUND_CTLS_CUP_ || 
		pobTran->inRunOperationID ==_OPERATION_REFUND_AMOUNT_FIRST_CUP_)
	{
		if((inRetVal = inCREDIT_Func_Get_OPT_CUP_TN(pobTran)) != VS_SUCCESS)
			return(inRetVal);
	}
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_OPT_OriTransDate_ESVC
Date&Time       :2018/5/29 下午 5:39
Describe        :For票證使用
*/
int inCREDIT_Func_Get_OPT_OriTransDate_ESVC(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	char		szDebugMsg[100 + 1];
	DISPLAY_OBJECT  srDispObj;

	if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srTRec.szTicketRefundDate) > 0)
		{
			return (VS_SUCCESS);
		}
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		else if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}
		else
		{

		}
	}

	while (1)
	{
		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMaxLen = 4;
		srDispObj.inCanNotBypass = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
		strcpy(srDispObj.szPromptMsg, "MMDD = ");

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 請輸入原交易日期 */
		inDISP_PutGraphic(_GET_ORI_DATE_, 0, _COORDINATE_Y_LINE_8_4_);

		strcpy(szTemplate, srDispObj.szPromptMsg);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_RIGHT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		memset(pobTran->srTRec.szTicketRefundDate, 0x00, sizeof(pobTran->srTRec.szTicketRefundDate));
		memcpy(pobTran->srTRec.szTicketRefundDate, srDispObj.szOutput, srDispObj.inOutputLen);

		if (inFunc_CheckValidOriDate(pobTran->srTRec.szTicketRefundDate) == VS_SUCCESS)
		{
			break;
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Date Not Valid: %s", pobTran->srTRec.szTicketRefundDate);
				inDISP_LogPrintf(szDebugMsg);
			}
		}

	}
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetOriAmount
Date&Time	:2016/9/21 下午 5:42
Describe	:請輸入原交易金額
*/
int inCREDIT_Func_GetOriAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;

	/* 若ECR發動而且有值就直接跳走 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 基本上一定大於0 */
		if (pobTran->srBRec.lnOrgTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
	}
	else
	{
		/* 基本上一定大於0 */
		if (pobTran->srBRec.lnOrgTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
	}


        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 7;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 請輸入原交易金額 */
        inDISP_PutGraphic(_GET_ORI_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

        pobTran->srBRec.lnOrgTxnAmount = atol(srDispObj.szOutput);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetOriPreAuthAmount
Date&Time	:2016/9/22 下午 3:51
Describe	:請輸入原預授權金額
*/
int inCREDIT_Func_GetOriPreAuthAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
        char		szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
        DISPLAY_OBJECT  srDispObj;

	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 基本上一定大於0 */
		if (pobTran->srBRec.lnOrgTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
	}
	else
	{
		/* 基本上一定大於0 */
		if (pobTran->srBRec.lnOrgTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
	}

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 7;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 請輸入原預授權金額 */
        inDISP_PutGraphic(_GET_ORI_PRE_AUTH_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

	pobTran->srBRec.lnOrgTxnAmount = atol(srDispObj.szOutput);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_OriPreAuthAmount
Date&Time	:2017/9/8 上午 11:12
Describe	:請輸入原預授權金額
*/
int inCREDIT_Func_Get_OPT_OriPreAuthAmount(TRANSACTION_OBJECT *pobTran)
{
        int		inRetVal = VS_ERROR;
        char		szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
        unsigned char uszKey;
        DISPLAY_OBJECT  srDispObj;

        /* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
        {
                /* 基本上一定大於0 */
                if (pobTran->srBRec.lnOrgTxnAmount > 0)
                {
                        return (VS_SUCCESS);
                }
                /* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
                else if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
                {
                        return (VS_SUCCESS);
                }
        }

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 7;
        srDispObj.inCanNotBypass = VS_TRUE;
        srDispObj.inCanNotZero = VS_TRUE;
        srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 請輸入原預授權金額 */
        inDISP_ChineseFont("請輸入原授權金額", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
//        inDISP_PutGraphic(_GET_ORI_PRE_AUTH_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

        memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
        srDispObj.inOutputLen = 0;

        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                return (inRetVal);

        pobTran->srBRec.lnOrgTxnAmount = atol(srDispObj.szOutput);
        
        /* [新增預授權完成] 預先授權完成金額 大於 原預先授權金額 2022/5/11 [SAM] */
        if( pobTran->srBRec.lnTxnAmount > pobTran->srBRec.lnOrgTxnAmount)
        {       
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                inDISP_ChineseFont("原授權金額低於完", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
                inDISP_ChineseFont("成金額請重新操作", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
                inDISP_ChineseFont("請按清除鍵", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);
                inDISP_BEEP(1,0);
                while (1)
                {
                        uszKey = uszKBD_Key();

                        if (uszKey == _KEY_CANCEL_)
                        {
                                /* 使用者終止交易 */
                                inRetVal = VS_USER_CANCEL;
                                break;
                        }
                        else
                        {
                                continue;
                        }
                }
                return (VS_ERROR);
        }
        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetPreCompAmount
Date&Time	:2016/9/22 下午 3:51
Describe	:請輸入預先授權完成金額
*/
int inCREDIT_Func_GetPreCompAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	long		lnPreCompMaxAmt = 0;
        char		szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
        DISPLAY_OBJECT  srDispObj;

	/* 若ECR發動而且有值就直接跳走 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 基本上一定大於0 */
		if (pobTran->srBRec.lnTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
	}
	else
	{
		/* 基本上一定大於0 */
		if (pobTran->srBRec.lnTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
	}

	/* 計算原預先授權115%的金額 */
	lnPreCompMaxAmt = ((pobTran->srBRec.lnOrgTxnAmount * 115) / 100);

	while (1)
	{
		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMaxLen = 7;
		srDispObj.inCanNotBypass = VS_TRUE;
		srDispObj.inCanNotZero = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
		strcpy(srDispObj.szPromptMsg, "NT$ ");

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 請輸入預先授權完成金額 */
		inDISP_PutGraphic(_GET_PRE_COMP_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);

		/* 進入畫面時先顯示金額為0 */
		strcpy(szTemplate, "NT$ 0");
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		/* 一般信用卡預先授權完成及銀聯預先授權完成皆有原預先授權金額之15%限制 */
		/* 這個限制只有聯合有，所以先拿掉  */
		if(1)
		//if (atol(srDispObj.szOutput) <= lnPreCompMaxAmt)
		{
			pobTran->srBRec.lnTxnAmount = atol(srDispObj.szOutput);
			pobTran->srBRec.lnTotalTxnAmount = atol(srDispObj.szOutput);
			break;
		}
		else
		{
			/* 輸入金額超過上限 請按清除鍵 */
			inDISP_Msg_BMP("", 0, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "輸入金額超過上限", _LINE_8_6_);
			continue;
		}
	}
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_PreCompAmount
Date&Time	:2017/9/8 上午 11:14
Describe	:請輸入預先授權完成金額
*/
int inCREDIT_Func_Get_OPT_PreCompAmount(TRANSACTION_OBJECT *pobTran)
{
        int		inRetVal = VS_ERROR;
        long		lnPreCompMaxAmt = 0;
        char		szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
        DISPLAY_OBJECT  srDispObj;

        /* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
        {
                if (pobTran->srBRec.lnTxnAmount > 0)
                {
                        return (VS_SUCCESS);
                }
                /* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
                else if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
                {
                        return (VS_SUCCESS);
                }
        }

        /* 計算原預先授權115%的金額 */
        lnPreCompMaxAmt = ((pobTran->srBRec.lnOrgTxnAmount * 115) / 100);

        while (1)
        {
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                memset(szTemplate, 0x00, sizeof(szTemplate));

                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
                srDispObj.inMaxLen = 7;
                srDispObj.inCanNotBypass = VS_TRUE;
                srDispObj.inCanNotZero = VS_TRUE;
                srDispObj.inColor = _COLOR_RED_;
                strcpy(srDispObj.szPromptMsg, "NT$ ");

                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                /* 請輸入預先授權完成金額 */
                inDISP_ChineseFont("輸入授權完成金額", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
                
//                inDISP_PutGraphic(_GET_PRE_COMP_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);

                /* 進入畫面時先顯示金額為0 */
                strcpy(szTemplate, "NT$ 0");
                inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

                memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
                srDispObj.inOutputLen = 0;

                inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                        return (inRetVal);

                /* 一般信用卡預先授權完成及銀聯預先授權完成皆有原預先授權金額之15%限制 */
                /* 這個限制只有聯合有，所以先拿掉  */
                if(1) 
                //if (atol(srDispObj.szOutput) <= lnPreCompMaxAmt)
                {
                        pobTran->srBRec.lnTxnAmount = atol(srDispObj.szOutput);
                        pobTran->srBRec.lnTotalTxnAmount = atol(srDispObj.szOutput);
                        break;
                }
                else
                {
                        /* 輸入金額超過上限 請按清除鍵 */
                        inDISP_Msg_BMP("", 0, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "輸入金額超過上限", _LINE_8_6_);
                        continue;
                }
        }
        
        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_Card_Number_Txno_Flow
Date&Time       :2017/6/13 下午 6:33
Describe        :請選擇？ 1.輸入交易編號 2.輸入卡號
*/
int inCREDIT_Func_Get_Card_Number_Txno_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char	szKey = 0x00;

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請選擇？ 1.輸入交易編號 2.輸入卡號 */
	inDISP_PutGraphic(_MENU_GET_CARD_NO_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                szKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		if (szKey == _KEY_1_			||
		    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
		{
			inRetVal = inCREDIT_Func_Get_TransactionNO(pobTran);
			break;
		}
		else if (szKey == _KEY_2_			||
		         inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
		{
			inRetVal = inCREDIT_Func_Get_Card_Number(pobTran);
			break;
		}
		else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}

	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (inRetVal);
}

/*
Function        :inCREDIT_Func_Get_Card_Number
Date&Time       :2017/1/20 下午 4:10
Describe        :
*/
int inCREDIT_Func_Get_Card_Number(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	DISPLAY_OBJECT  srDispObj;

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 第三行顯示 請輸入卡號? */
	inDISP_PutGraphic(_GET_CARD_NO_, 0, _COORDINATE_Y_LINE_8_4_);

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 19;
	srDispObj.inMenuKeyIn = pobTran->inMenuKeyin;

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

	inRetVal = inDISP_Enter8x16_MenuKeyIn(&srDispObj);

	if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
	{
		return (inRetVal);
	}

	if (srDispObj.inOutputLen > 0)
		memcpy(&pobTran->srBRec.szPAN[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_TransactionNO
Date&Time       :2017/6/15 下午 3:57
Describe        :請輸入交易編號
*/
int inCREDIT_Func_Get_TransactionNO(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT  srDispObj;

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 顯示請輸入交易編號? */
	inDISP_PutGraphic(_GET_TXNO_, 0, _COORDINATE_Y_LINE_8_4_);

	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_LEFT_;
	srDispObj.inMaxLen = 23;
	srDispObj.inColor = _COLOR_RED_;
	srDispObj.inCanNotBypass = VS_TRUE;

	while (1)
	{
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		if (srDispObj.inOutputLen > 0)
		{
			/* 判斷交易編號小於15碼都是長度錯誤 */
			if (srDispObj.inOutputLen < 15)
			{
				/* 交易編號長度錯誤 請按清除鍵 */
				inDISP_Msg_BMP(_ERR_TXNO_LEN_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

				return (VS_USER_CANCEL);
			}
			else
			{
				memset(pobTran->srBRec.szTxnNo, 0x00, sizeof(pobTran->srBRec.szTxnNo));
				memcpy(pobTran->srBRec.szTxnNo, srDispObj.szOutput, srDispObj.inOutputLen);
				if (inCARDFUNC_GetPANFromTransactionNo(pobTran) != VS_SUCCESS)
				{
					/* 交易編號錯誤 請按清除鍵 */
					inDISP_Msg_BMP(_ERR_TXNO_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

					return (VS_USER_CANCEL);
				}
				else
				{
					/* 標示由交易編號獲得卡號 */
					pobTran->uszInputTxnoBit = VS_TRUE;
					break;
				}

			}
		}

	}
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_HG_Card_Number
Date&Time       :2017/5/17 下午 5:49
Describe        :
*/
int inCREDIT_Func_Get_HG_Card_Number(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	DISPLAY_OBJECT  srDispObj;

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 第三行顯示 請輸入卡號? */
	inDISP_PutGraphic(_GET_CARD_NO_, 0, _COORDINATE_Y_LINE_8_4_);

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 19;
	srDispObj.inMenuKeyIn = pobTran->inMenuKeyin;

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

	inRetVal = inDISP_Enter8x16_MenuKeyIn(&srDispObj);

	if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
	{
		return (inRetVal);
	}

	if (srDispObj.inOutputLen > 0)
		memcpy(&pobTran->srBRec.szHGPAN[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_Exp_Date
Date&Time       :2017/1/20 下午 4:10
Describe        :請輸入有效期
*/
int inCREDIT_Func_Get_Exp_Date(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT  srDispObj;

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 顯示請輸入有效期 */
	inDISP_PutGraphic(_GET_EXPIRE_DATE_, 0, _COORDINATE_Y_LINE_8_4_);

	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 4;
	srDispObj.inColor = _COLOR_RED_;
	srDispObj.inCanNotBypass = VS_TRUE;
	strcpy(srDispObj.szPromptMsg, "MMYY= ");

	strcpy(szTemplate, srDispObj.szPromptMsg);

	while (1)
	{
		inDISP_Clear_Line(srDispObj.inY, _LINE_8_8_);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_RIGHT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				return (inRetVal);

		if (srDispObj.inOutputLen == 4)
		{
			/* 因為MenuKeyIn的有效期為MMYY，但檢核有效期格式為YYMM */
			memcpy(&pobTran->srBRec.szExpDate[0], &srDispObj.szOutput[2], 2);
			memcpy(&pobTran->srBRec.szExpDate[2], &srDispObj.szOutput[0], 2);
			break;
		}
		else
		{
			continue;
		}

	}
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_CheckNO
Date&Time       :2017/1/23 下午 3:07
Describe        :請輸入檢查碼
*/
int inCREDIT_Func_Get_CheckNO(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	char		szEXPDate[4 + 1];
	char		szCheckPANKey;
	DISPLAY_OBJECT  srDispObj;

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 顯示請輸入檢查碼? */
	inDISP_PutGraphic(_GET_CHECK_NO_, 0, _COORDINATE_Y_LINE_8_4_);

	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 5;
	srDispObj.inColor = _COLOR_RED_;

	/* 某些狀況要可以ByPass */
	/* 2013-06-03 AM 11:27:51 add by kakab 配合修改規格書CUP需求說明書20130423.doc 預先授權完成人工輸入檢查碼需可by pass */
	if (pobTran->inTransactionCode == _CUP_PRE_COMP_)
	{
		srDispObj.inCanNotBypass = VS_FALSE;
	}
	/* 2013-12-03 AM 11:27:51 add by kakab 配合修改規格書CUP需求說明書20131128.doc 銀聯退貨人工輸入檢查碼需可by pass */
	else if (pobTran->inTransactionCode == _CUP_REFUND_)
	{
		srDispObj.inCanNotBypass = VS_FALSE;
	}
	else
	{
		srDispObj.inCanNotBypass = VS_TRUE;
	}

	strcpy(srDispObj.szPromptMsg, "Check No.= ");

	strcpy(szTemplate, srDispObj.szPromptMsg);

	while (1)
	{
		inDISP_Clear_Line(srDispObj.inY, _LINE_8_8_);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_RIGHT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		if (srDispObj.inCanNotBypass == VS_FALSE && srDispObj.inOutputLen == 0)
		{
			break;
		}
		else if (srDispObj.inOutputLen < 5)
		{
			/* 檢查碼錯誤 */
			inDISP_Msg_BMP(_ERR_CHECK_NO_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
			return (VS_ERROR);
		}
		else
		{
			memset(szEXPDate, 0x00, sizeof(szEXPDate));
			inRetVal = inCARD_ExpDateEncryptAndDecrypt(pobTran, srDispObj.szOutput, szEXPDate, _EXP_DECRYPT_);
			if (inRetVal != VS_SUCCESS)
			{
				/* 檢查碼錯誤 */
				inDISP_Msg_BMP(_ERR_CHECK_NO_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
				return (VS_ERROR);
			}

			/* U卡特殊邏輯 */
			if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
				szCheckPANKey = pobTran->srBRec.szPAN[13];
			else
				szCheckPANKey = pobTran->srBRec.szPAN[9];

			/* 核對第十碼卡號 */
			if (szEXPDate[4] != szCheckPANKey)
			{
				/* 檢查碼錯誤 */
				inDISP_Msg_BMP(_ERR_CHECK_NO_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
				return (VS_ERROR);
			}
			else
			{
				/* 因為MenuKeyIn的有效期為MMYY(所以解開時也是)，但檢核有效期格式為YYMM */
				memcpy(&pobTran->srBRec.szExpDate[0], &szEXPDate[2], 2);
				memcpy(&pobTran->srBRec.szExpDate[2], &szEXPDate[0], 2);
				pobTran->uszInputCheckNoBit = VS_TRUE;
				break;
			}

		}

	}
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_CheckNO_ExpDate_Flow
Date&Time       :2017/1/23 下午 4:27
Describe        :請輸入檢查碼 和 輸入有效期的流程
*/
int inCREDIT_Func_Get_CheckNO_ExpDate_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	int	inDefaultChoice = 0;
	char	szKey = 0x00;

	/* 如果輸入交易編號，直接選擇檢查碼輸入*/
	/* TMS開啟自存聯卡號遮掩功能，退貨交易選擇輸入交易編號流程，僅支援檢查碼輸入，不支援有效期輸入。(參照規格：簽帳單之特店自存聯內卡號隱藏需求內容-20160125) */
	if (pobTran->uszInputTxnoBit == VS_TRUE)
	{
		inDefaultChoice = 1;
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_MENU_CHECKNO_EXPDATE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	}

	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                szKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		if (szKey == _KEY_1_						||
		    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	||
		    inDefaultChoice == 1)
		{
			inRetVal = inCREDIT_Func_Get_CheckNO(pobTran);
			break;
		}
		else if (szKey == _KEY_2_			||
		         inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
		{
			inRetVal = inCREDIT_Func_Get_Exp_Date(pobTran);
			break;
		}
		else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}

	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (inRetVal);
}

/*
Function        :inCREDIT_Func_Get_Barcode1
Date&Time       :2017/2/18 下午 3:19
Describe        :請掃描或輸入第一段條碼？
*/
int inCREDIT_Func_Get_Barcode1(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT  srDispObj;

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	srDispObj.inY = _LINE_8_8_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 20;
	srDispObj.inColor = _COLOR_RED_;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inMenuKeyIn = pobTran->inMenuKeyin;

	while (1)
	{
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				return (inRetVal);

		if (srDispObj.inOutputLen > 0 &&
		   ((memcmp(srDispObj.szOutput, "11", strlen("11")) == 0)	||
		    (memcmp(srDispObj.szOutput, "21", strlen("21")) == 0)))		/* 若有輸入兌換條碼，且前兩個值合法(11表示只有一個條碼，21表示有兩個條碼中的第一個條碼) */
		{
			memcpy(&pobTran->szL3_Barcode1[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
			sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
			break;
		}
		else
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			/* 請掃描或輸入第一段條碼？ */
			inDISP_PutGraphic(_GET_BARCODE_1_, 0, _COORDINATE_Y_LINE_8_4_);
			break;
		}

	}
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_Barcode2
Date&Time       :2017/2/18 下午 3:19
Describe        :請掃描或輸入第二段條碼？
*/
int inCREDIT_Func_Get_Barcode2(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT  srDispObj;

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	srDispObj.inY = _LINE_8_8_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 20;
	srDispObj.inColor = _COLOR_RED_;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inMenuKeyIn = pobTran->inMenuKeyin;

	while (1)
	{
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				return (inRetVal);

		if (srDispObj.inOutputLen > 0 &&
		   (memcmp(srDispObj.szOutput, "22", strlen("22")) == 0))		/* 若有輸入兌換條碼，且前兩個值合法(22表示有兩個條碼中的第二個條碼) */
		{
			memcpy(&pobTran->szL3_Barcode2[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
			sprintf(pobTran->szL3_Barcode2Len, "%02d", strlen(pobTran->szL3_Barcode2));
			break;
		}
		else
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			/* 請掃描或輸入第二段條碼？ */
			inDISP_PutGraphic(_GET_BARCODE_2_, 0, _COORDINATE_Y_LINE_8_4_);
			break;
		}

	}
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_GetDCCOriTransDate
Date&Time       :2016/11/25 下午 3:40
Describe        :請輸入DCC原交易日期
*/
int inCREDIT_Func_GetDCCOriTransDate(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;

	/* 非DCC交易跳走避免和一般交易混淆 */
	if (pobTran->srBRec.uszDCCTransBit != VS_TRUE)
		return (VS_SUCCESS);

	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szDCC_OTD) > 0)
		{
			return (VS_SUCCESS);
		}
		else
		{

		}
	}

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 4;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "MMDD = ");

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 請輸入原交易日期 */
        inDISP_PutGraphic(_GET_ORI_DATE_, 0, _COORDINATE_Y_LINE_8_4_);

        strcpy(szTemplate, srDispObj.szPromptMsg);

	while (1)
	{
		inDISP_Clear_Line(srDispObj.inY, _LINE_8_8_);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_RIGHT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		memset(pobTran->srBRec.szDCC_OTD, 0x00, sizeof(pobTran->srBRec.szDCC_OTD));
		memcpy(pobTran->srBRec.szDCC_OTD, srDispObj.szOutput, srDispObj.inOutputLen);

		if (inFunc_CheckValidOriDate(pobTran->srBRec.szDCC_OTD) == VS_SUCCESS)
		{
			break;
		}
		else
		{
			memset(pobTran->srBRec.szDCC_OTD, 0x00, sizeof(pobTran->srBRec.szDCC_OTD));
			srDispObj.inOutputLen = 0;
		}

	}
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetDCCOriAmount
Date&Time	:2016/11/25 下午 3:40
Describe	:請輸入DCC原交易金額
*/
int inCREDIT_Func_GetDCCOriAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;

	/* 非DCC交易跳走避免和一般交易混淆 */
	if (pobTran->srBRec.uszDCCTransBit != VS_TRUE)
		return (VS_SUCCESS);

	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (pobTran->srBRec.lnOrgTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
		else
		{

		}
	}
	else
	{

	}

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 7;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 請輸入原交易金額 */
        inDISP_PutGraphic(_GET_ORI_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

        pobTran->srBRec.lnOrgTxnAmount = atol(srDispObj.szOutput);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetDCCTipAmount
Date&Time	:2015/9/16 上午 11:19
Describe	:螢幕顯示原金額，輸入外幣小費金額
*/
int inCREDIT_Func_GetDCCTipAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal, i;
	int		inTWDMUPower;				/* 金額的指數*/
        long		lnTipPercent, lnTiplimit, lnOrgDCCAmt;
	long		lnTipAmount;
	long		lnTWD;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        char		szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];
        char		szKey;
	char		szTWDMU[12 + 1];
	double		dbTWDTipAmount;
        DISPLAY_OBJECT  srDispObj;

        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("inCREDIT_Func_GetDCCTipAmount START!!");
        }

	/* 非DCC交易跳走避免和一般交易混淆 */
	if (pobTran->srBRec.uszDCCTransBit != VS_TRUE)
		return (VS_SUCCESS);

	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (pobTran->srBRec.lnTipTxnAmount > 0l)
		{
			return (VS_SUCCESS);
		}
	}
	else
	{

	}

        /* 取得HDT 小費百分比Tip percent 等於零的話提示錯誤畫面 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetTipPercent(szTemplate);
        lnTipPercent = atol(szTemplate);

        if (lnTipPercent == 0L)
        {
		/* 小費百分比錯誤 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                inDISP_Msg_BMP(_ERR_TIP_RATE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

                return (VS_ERROR);
        }

        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "Tip Percent ： %ld", lnTipPercent);
                inDISP_LogPrintf(szErrorMsg);
        }

	/* DCC外幣金額(判斷及顯示用) */
	lnOrgDCCAmt = atol(pobTran->srBRec.szDCC_FCA);

	/* ----------------------ECR小費是否超過判斷START-----------------------------------------------------*/
	if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 如果原金額過大（9個9），計算limit會溢位，所以加判斷 */
		if (pobTran->srBRec.lnOrgTxnAmount >= 100)
			lnTiplimit = ((lnOrgDCCAmt / 100L) * lnTipPercent);
		else
			lnTiplimit = ((lnOrgDCCAmt * lnTipPercent) / 100L);

		if (ginDebug == VS_TRUE)
		{
			memset(szErrorMsg, 0x00, sizeof (szErrorMsg));
			sprintf(szErrorMsg, "Tip: %ld, TipLimit: %ld", pobTran->srBRec.lnTipTxnAmount, lnTiplimit);
			inDISP_LogPrintf(szErrorMsg);
		}

		/* 若ECR發動，且小費不為0直接跳走 */
		if (pobTran->srBRec.lnTipTxnAmount != 0 && (pobTran->srBRec.lnTipTxnAmount < lnTiplimit))
			return (VS_SUCCESS);
		else if (pobTran->srBRec.lnTipTxnAmount >= lnTiplimit)
		{
			/* 顯示小費過多 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_Msg_BMP(_ERR_BIG_TIP_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}

	}
	/* ----------------------ECR小費是否超過判斷END-----------------------------------------------------*/


        for (i = 0;; i++)
        {
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                /* 輸入小費 */
                inDISP_PutGraphic(_GET_DCC_TIP_, 0, _COORDINATE_Y_LINE_8_4_);

                /* 第二行原始金額 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "", 0x00, _SIGNED_NONE_, 16, _PAD_LEFT_FILL_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szTemplate);
                inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

                /* 第四行鍵盤輸入小費金額 */
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                memset(szTemplate, 0x00, sizeof(szTemplate));

                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
		/* J.當台幣輸入金額為最大值NT $9,999,999時，若外幣幣別選擇KRW時，
		 * 則交易金額換算過來大約為九位數 KRW 365,468,963，若商家欲輸入小費金額時，
		 * 若小費限額是30%，則端末機最大外幣小費金額允許輸入八位數以上。
		 * Vx520 Code裡面直接開到9位 */
                srDispObj.inMaxLen = 9;
		srDispObj.inCanNotBypass = VS_TRUE;
		srDispObj.inCanNotZero = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
                strcpy(srDispObj.szPromptMsg, "");
		strcpy(srDispObj.szMinorUnit, pobTran->srBRec.szDCC_FCMU);
		strcpy(srDispObj.szCurrencyCode, pobTran->srBRec.szDCC_FCAC);

                /* 進入畫面時先顯示金額為0 */
		inFunc_Amount_Comma_DCC("0", "", 0x00, _SIGNED_NONE_, 16, _PAD_LEFT_FILL_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szTemplate);
                inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

                inRetVal = inDISP_Enter8x16_GetDCCAmount(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                        return (inRetVal);

                lnTipAmount = atol(srDispObj.szOutput);

		/* 如果原金額過大（9個9），計算limit會溢位，所以加判斷 */
		if (lnOrgDCCAmt >= 100)
			lnTiplimit = ((lnOrgDCCAmt / 100L) * lnTipPercent);
		else
			lnTiplimit = ((lnOrgDCCAmt * lnTipPercent) / 100L);

                if (lnTipAmount > lnTiplimit)
                {
                        /* 顯示小費過多 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_Msg_BMP(_ERR_BIG_TIP_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

                        continue;
                }
                else
                {
                        /* 儲存小費金額 */
                        strcpy(pobTran->srBRec.szDCC_TIPFCA, srDispObj.szOutput);

                        break;
                }

        }

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* DCC小費金額確認 */
	inDISP_PutGraphic(_CHECK_DCC_TIP_, 0, _COORDINATE_Y_LINE_8_4_);
	/* 第二行金額 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_TIPFCA, "", 0x00, _SIGNED_NONE_, 16, _PAD_LEFT_FILL_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szTemplate);
	inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

	/* 第三行金額 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	/* 初始化 */
	inTWDMUPower = 0;

	inTWDMUPower += atoi(pobTran->srBRec.szDCC_FCMU);
	/* 10USD = 300 NTD → 1USD = 30 NTD */
	inTWDMUPower += atoi(pobTran->srBRec.szDCC_IRDU);

	inTWDMUPower += atoi(pobTran->srBRec.szDCC_IRMU);

	/* 看指數有幾位直接補0(因為是10的指數所以可以直接這樣做)，用乘的會比較耗效能 */
	memset(szTWDMU, 0x00, sizeof(szTWDMU));
	strcpy(szTWDMU, "1");
	/* inTWDMUPower + 1 是因為要10的inTWDMUPower次方，但是Pad會連最前面的1都算進去 */
	inFunc_PAD_ASCII(szTWDMU, szTWDMU, '0', inTWDMUPower + 1, _PAD_LEFT_FILL_RIGHT_);
	lnTWD = atol(szTWDMU);

	/* 先相乘，再除位數*/
	dbTWDTipAmount = (atol(pobTran->srBRec.szDCC_TIPFCA) * atol(pobTran->srBRec.szDCC_IRV) / lnTWD);

	/* 小費外幣轉台必須四捨五入 */
	if (dbTWDTipAmount - (long)dbTWDTipAmount >= 0.5)
		pobTran->srBRec.lnTipTxnAmount = ((long)dbTWDTipAmount + 1);
       	else
                pobTran->srBRec.lnTipTxnAmount = ((long)dbTWDTipAmount);

	sprintf(szTemplate, "%ld", pobTran->srBRec.lnTipTxnAmount);
	inFunc_Amount_Comma_DCC(szTemplate, "", 0x00, _SIGNED_NONE_, 16, _PAD_LEFT_FILL_RIGHT_, "0", "NTD", szTemplate);
	inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);

	/* 請按0確認 */
	while (1)
        {
                szKey = uszKBD_GetKey(180);
                if (szKey == _KEY_0_)
                        break;
                else if (szKey == _KEY_CANCEL_)
			return (VS_USER_CANCEL);
		else if (szKey == _KEY_TIMEOUT_)
                        return (VS_TIMEOUT);
                else
                        continue;
        }


        /* 總金額 */
        pobTran->srBRec.lnTotalTxnAmount = pobTran->srBRec.lnOrgTxnAmount + pobTran->srBRec.lnTipTxnAmount;

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 確認小費新金額畫面 */
        inDISP_PutGraphic(_CHECK_TIP_, 0, _COORDINATE_Y_LINE_8_4_);
	/* 第二行外幣新金額 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%ld", atol(pobTran->srBRec.szDCC_FCA) + atol(pobTran->srBRec.szDCC_TIPFCA));
	inFunc_Amount_Comma_DCC(szTemplate, "", 0x00, _SIGNED_NONE_, 16, _PAD_LEFT_FILL_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szTemplate);
	inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
        /* 第三行總金額 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTotalTxnAmount);
	inFunc_Amount_Comma(szTemplate, "NT$ " , 0x00, _SIGNED_NONE_, 16, _PAD_LEFT_FILL_RIGHT_);
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
        inDISP_BEEP(1, 0);

        while (1)
        {
                szKey = uszKBD_GetKey(180);
                if (szKey == _KEY_0_)
                        break;
                else if (szKey == _KEY_CANCEL_)
			return (VS_USER_CANCEL);
		else if (szKey == _KEY_TIMEOUT_)
                        return (VS_TIMEOUT);
                else
                        continue;
        }

        pobTran->srBRec.inCode = _TIP_;
	pobTran->inTransactionCode = _TIP_;
	pobTran->uszUpdateBatchBit = VS_TRUE;
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("inCREDIT_Func_GetDCCTipAmount END!!");
        }

        return (VS_SUCCESS);
}


/*
Function        :inCREDIT_Func_GetFiscRRN
Date&Time       :2016/11/25 下午 2:20
Describe        :請輸入調單編號
*/
int inCREDIT_Func_GetFiscRRN(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;

	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szFiscRRN) > 0)
		{
			return (VS_SUCCESS);
		}
		else
		{

		}
	}
	else
	{
		if (strlen(pobTran->srBRec.szFiscRRN) > 0)
		{
			return (VS_SUCCESS);
		}
	}

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 12;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 請輸入調單編號 */
        inDISP_PutGraphic(_GET_FISC_RRN_, 0, _COORDINATE_Y_LINE_8_4_);

	while (1)
	{
		inDISP_Clear_Line(srDispObj.inY, _LINE_8_8_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		memset(pobTran->srBRec.szFiscRRN, 0x00, sizeof(pobTran->srBRec.szFiscRRN));
		memcpy(pobTran->srBRec.szFiscRRN, srDispObj.szOutput, srDispObj.inOutputLen);

		/* 調單編號，只允許輸入12碼，不多不少 */
		if (srDispObj.inOutputLen == 12)
		{
			break;
		}
		else
		{
			memset(pobTran->srBRec.szFiscRRN, 0x00, sizeof(pobTran->srBRec.szFiscRRN));
			srDispObj.inOutputLen = 0;
		}

	}
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_OPT_FiscRRN
Date&Time       :2017/6/28 下午 4:18
Describe        :請輸入調單編號
*/
int inCREDIT_Func_Get_OPT_FiscRRN(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;

	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szFiscRRN) > 0)
		{
			return (VS_SUCCESS);
		}
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		else if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}
		else
		{

		}
	}

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 12;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 請輸入調單編號 */
        inDISP_PutGraphic(_GET_FISC_RRN_, 0, _COORDINATE_Y_LINE_8_4_);

	while (1)
	{
		inDISP_Clear_Line(srDispObj.inY, _LINE_8_8_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		memset(pobTran->srBRec.szFiscRRN, 0x00, sizeof(pobTran->srBRec.szFiscRRN));
		memcpy(pobTran->srBRec.szFiscRRN, srDispObj.szOutput, srDispObj.inOutputLen);

		/* 調單編號，只允許輸入12碼，不多不少 */
		if (srDispObj.inOutputLen == 12)
		{
			break;
		}
		else
		{
			memset(pobTran->srBRec.szFiscRRN, 0x00, sizeof(pobTran->srBRec.szFiscRRN));
			srDispObj.inOutputLen = 0;
		}

	}
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetFiscRefund
Date&Time	:2016/11/25 下午 3:18
Describe	:輸入退費金額
*/
int inCREDIT_Func_GetFiscRefund(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;

	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (pobTran->srBRec.lnTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
		else
		{

		}
	}
	else
	{
		if (pobTran->srBRec.lnTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
	}

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 7;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 請輸入退費金額 */
        inDISP_PutGraphic(_GET_FISC_REFUND_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

        pobTran->srBRec.lnTxnAmount = atol(srDispObj.szOutput);
        pobTran->srBRec.lnOrgTxnAmount = atol(srDispObj.szOutput);
        pobTran->srBRec.lnTotalTxnAmount = atol(srDispObj.szOutput);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_FiscRefund
Date&Time	:2017/6/28 下午 4:20
Describe	:輸入退費金額
*/
int inCREDIT_Func_Get_OPT_FiscRefund(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;

	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (pobTran->srBRec.lnTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		else if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}
		else
		{

		}
	}

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 7;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 請輸入退費金額 */
        inDISP_PutGraphic(_GET_FISC_REFUND_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

        pobTran->srBRec.lnTxnAmount = atol(srDispObj.szOutput);
        pobTran->srBRec.lnOrgTxnAmount = atol(srDispObj.szOutput);
        pobTran->srBRec.lnTotalTxnAmount = atol(srDispObj.szOutput);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_GetFiscOriTransDate
Date&Time       :2016/11/25 下午 6:02
Describe        :請輸入原交易日期，Fisc年由端末機產生
*/
int inCREDIT_Func_GetFiscOriTransDate(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	int		inYear = 0;
	char		szYear[4 + 1];
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
	char		szDebugMsg[100 + 1];
        DISPLAY_OBJECT  srDispObj;

	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szFiscRefundDate) > 0)
		{
			return (VS_SUCCESS);
		}
		else
		{

		}
	}
	else
	{
		if (strlen(pobTran->srBRec.szFiscRefundDate) > 0)
		{
			return (VS_SUCCESS);
		}
	}

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 4;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
	strcpy(srDispObj.szPromptMsg, "MMDD = ");

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請輸入原交易日期 */
	inDISP_PutGraphic(_GET_ORI_DATE_, 0, _COORDINATE_Y_LINE_8_4_);

	strcpy(szTemplate, srDispObj.szPromptMsg);

	while (1)
	{
		inDISP_Clear_Line(srDispObj.inY, _LINE_8_8_);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_RIGHT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		memset(&pobTran->srBRec.szFiscRefundDate[4], 0x00, 4);
		memcpy(&pobTran->srBRec.szFiscRefundDate[4], srDispObj.szOutput, srDispObj.inOutputLen);

		memcpy(szYear, pobTran->srBRec.szDate, 4);
		inYear = atoi(szYear);

		memcpy(pobTran->srBRec.szFiscRefundDate, pobTran->srBRec.szDate, 4);

		/* 日期大於今天日期就填入去年 */
		if (inFunc_SunDay_Sum(pobTran->srBRec.szFiscRefundDate) > inFunc_SunDay_Sum(pobTran->srBRec.szDate))
		{
			sprintf(pobTran->srBRec.szFiscRefundDate, "%04d%s", inYear - 1, &pobTran->srBRec.szFiscRefundDate[4]);
		}
		else
		{
			/* 已在上面填入今年 */
		}

		if (inFunc_CheckValidDate_Include_Year(pobTran->srBRec.szFiscRefundDate) == VS_SUCCESS)
		{
			break;
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Date Not Valid: %s", pobTran->srBRec.szFiscRefundDate);
				inDISP_LogPrintf(szDebugMsg);
			}
		}

	}
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_OPT_FiscOriTransDate
Date&Time       :2017/6/28 下午 4:21
Describe        :請輸入原交易日期，Fisc年由端末機產生
*/
int inCREDIT_Func_Get_OPT_FiscOriTransDate(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	int		inYear = 0;
	char		szYear[4 + 1];
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
	char		szDebugMsg[100 + 1];
        DISPLAY_OBJECT  srDispObj;

	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szFiscRefundDate) > 0)
		{
			return (VS_SUCCESS);
		}
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		else if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}
		else
		{

		}
	}

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 4;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
	strcpy(srDispObj.szPromptMsg, "MMDD = ");

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請輸入原交易日期 */
	inDISP_PutGraphic(_GET_ORI_DATE_, 0, _COORDINATE_Y_LINE_8_4_);

	strcpy(szTemplate, srDispObj.szPromptMsg);

	while (1)
	{
		inDISP_Clear_Line(srDispObj.inY, _LINE_8_8_);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_RIGHT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		memset(&pobTran->srBRec.szFiscRefundDate[4], 0x00, 4);
		memcpy(&pobTran->srBRec.szFiscRefundDate[4], srDispObj.szOutput, srDispObj.inOutputLen);

		memcpy(szYear, pobTran->srBRec.szDate, 4);
		inYear = atoi(szYear);

		memcpy(pobTran->srBRec.szFiscRefundDate, pobTran->srBRec.szDate, 4);

		/* 日期大於今天日期就填入去年 */
		if (inFunc_SunDay_Sum(pobTran->srBRec.szFiscRefundDate) > inFunc_SunDay_Sum(pobTran->srBRec.szDate))
		{
			sprintf(pobTran->srBRec.szFiscRefundDate, "%04d%s", inYear - 1, &pobTran->srBRec.szFiscRefundDate[4]);
		}
		else
		{
			/* 已在上面填入今年 */
		}

		if (inFunc_CheckValidDate_Include_Year(pobTran->srBRec.szFiscRefundDate) == VS_SUCCESS)
		{
			break;
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Date Not Valid: %s", pobTran->srBRec.szFiscRefundDate);
				inDISP_LogPrintf(szDebugMsg);
			}
		}

	}
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_GetProductCode
Date&Time       :2017/4/6 上午 11:07
Describe        :
*/
int inCREDIT_Func_GetProductCode(TRANSACTION_OBJECT *pobTran)
{
	int	i;
	char	szEnable[2 + 1];
	char	szKey = 0x00;
	char	szKeymap[1 + 1];	/* PCD檔用 */
	char 	szDispMsg[_DISP_MSG_SIZE_ + 1];
	char	uszFind = VS_FALSE;

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "CREDIT_4406 Func_GetProductCode INIT");	
	
	
	if (inGetProductCodeEnable(szEnable) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("   Get Produce Func *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	if (memcmp(szEnable, "Y", strlen("Y")) != 0)
	{
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] Not Enable END -----",__FILE__, __FUNCTION__, __LINE__);
		
		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "CREDIT_4406 Product  NotEnable END");
		return (VS_SUCCESS);
	}
	else
	{
		if (inLoadPCDRec(0) < 0)
		{
			inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] Not PCD Data END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_SUCCESS); /* 表示沒有下產品代碼檔 */
		}
	}

	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szProductCode) > 0)
		{
			inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] ECR PD > 0 END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_SUCCESS);
		}
		else
		{

		}
	}
	else
	{
		if (strlen(pobTran->srBRec.szProductCode) > 0)
		{
			inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] PD > 0 END -----",__FILE__, __FUNCTION__, __LINE__);	
			return (VS_SUCCESS);
		}
	}

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請輸入產品代碼 */
	inDISP_PutGraphic(_GET_PRODUCT_CODE_, 0, _COORDINATE_Y_LINE_8_4_);

	while (1)
	{

		while (1)
		{
			szKey = uszKBD_GetKey(_EDC_TIMEOUT_);

			if (szKey >= _KEY_0_ && szKey <= _KEY_9_)
			{
				break;
			}
			else if (szKey == _KEY_ENTER_)
			{
				/* 補空白 */
				if (strlen(pobTran->srBRec.szProductCode) == 0)
				{
					memset(pobTran->srBRec.szProductCode, 0x20, sizeof(pobTran->srBRec.szProductCode) - 1);
				}

				return (VS_SUCCESS);
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

		uszFind = VS_FALSE;
		/* 尋找剛剛輸入的產品代碼是否在PCD檔內 */
		for (i = 0; ; i++)
		{
			if (inLoadPCDRec(i) < 0)	/* 產品代碼檔 */
			{
				break;
			}

			memset(szKeymap, 0x00, sizeof(szKeymap));
			inGetKeyMap(szKeymap);

			/* _KEY_0_ 到 _KEY_9_ 的代碼為 '0'~'9' */
			if (szKey == szKeymap[0])
			{
				uszFind = VS_TRUE;
				break;
			}

		}

		/* 根據查詢到的結果顯示 */
		if (uszFind != VS_TRUE)
		{
			/* 顯示請重新選擇 */
			inDISP_Msg_BMP(_ERR_RESELECT_, _COORDINATE_Y_LINE_8_6_, _NO_KEY_MSG_, 1, "", 0);
			inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
			memset(pobTran->srBRec.szProductCode, 0x00, sizeof(pobTran->srBRec.szProductCode));
			strcpy(pobTran->srBRec.szProductCode, " ");

			/* 重置畫面 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			/* 請輸入產品代碼 */
			inDISP_PutGraphic(_GET_PRODUCT_CODE_, 0, _COORDINATE_Y_LINE_8_4_);

			continue;
		}
		else
		{
			inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
			memset(pobTran->srBRec.szProductCode, 0x00, sizeof(pobTran->srBRec.szProductCode));
			inGetProductScript(pobTran->srBRec.szProductCode);

			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memcpy(szDispMsg, pobTran->srBRec.szProductCode, 16);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memcpy(szDispMsg, &pobTran->srBRec.szProductCode[16], 16);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_8_, _COLOR_BLACK_, _DISP_LEFT_);
		}

	}
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_OPT_ProductCode
Date&Time       :2017/4/19 下午 3:00
Describe        :
*/
int inCREDIT_Func_Get_OPT_ProductCode(TRANSACTION_OBJECT *pobTran)
{
	int	i;
	char	szEnable[2 + 1];
	char	szKey = 0x00;
	char	szKeymap[1 + 1];	/* PCD檔用 */
	char 	szDispMsg[_DISP_MSG_SIZE_ + 1];
	char	uszFind = VS_FALSE;

	if (inGetProductCodeEnable(szEnable) != VS_SUCCESS)
		return (VS_ERROR);

	if (memcmp(szEnable, "Y", strlen("Y")) != 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		if (inLoadPCDRec(0) < 0)
			return (VS_SUCCESS); /* 表示沒有下產品代碼檔 */
	}


	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szProductCode) > 0)
		{
			return (VS_SUCCESS);
		}
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		else if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}
		else
		{

		}
	}

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請輸入產品代碼 */
	inDISP_PutGraphic(_GET_PRODUCT_CODE_, 0, _COORDINATE_Y_LINE_8_4_);

	while (1)
	{

		while (1)
		{
			szKey = uszKBD_GetKey(_EDC_TIMEOUT_);

			if (szKey >= _KEY_0_ && szKey <= _KEY_9_)
			{
				break;
			}
			else if (szKey == _KEY_ENTER_)
			{
				/* 補空白 */
				if (strlen(pobTran->srBRec.szProductCode) == 0)
				{
					memset(pobTran->srBRec.szProductCode, 0x20, sizeof(pobTran->srBRec.szProductCode) - 1);
				}

				return (VS_SUCCESS);
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

		uszFind = VS_FALSE;
		/* 尋找剛剛輸入的產品代碼是否在PCD檔內 */
		for (i = 0; ; i++)
		{
			if (inLoadPCDRec(i) < 0)	/* 產品代碼檔 */
			{
				break;
			}

			memset(szKeymap, 0x00, sizeof(szKeymap));
			inGetKeyMap(szKeymap);

			/* _KEY_0_ 到 _KEY_9_ 的代碼為 '0'~'9' */
			if (szKey == szKeymap[0])
			{
				uszFind = VS_TRUE;
				break;
			}

		}

		/* 根據查詢到的結果顯示 */
		if (uszFind != VS_TRUE)
		{
			/* 顯示請重新選擇 */
			inDISP_Msg_BMP(_ERR_RESELECT_, _COORDINATE_Y_LINE_8_6_, _NO_KEY_MSG_, 1, "", 0);
			inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
			memset(pobTran->srBRec.szProductCode, 0x00, sizeof(pobTran->srBRec.szProductCode));
			strcpy(pobTran->srBRec.szProductCode, " ");

			/* 重置畫面 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			/* 請輸入產品代碼 */
			inDISP_PutGraphic(_GET_PRODUCT_CODE_, 0, _COORDINATE_Y_LINE_8_4_);

			continue;
		}
		else
		{
			inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
			memset(pobTran->srBRec.szProductCode, 0x00, sizeof(pobTran->srBRec.szProductCode));
			inGetProductScript(pobTran->srBRec.szProductCode);

			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memcpy(szDispMsg, pobTran->srBRec.szProductCode, 16);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memcpy(szDispMsg, &pobTran->srBRec.szProductCode[16], 16);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_8_, _COLOR_BLACK_, _DISP_LEFT_);
		}

	}
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_GetCVV2
Date&Time       :2017/6/8 下午 3:16
Describe        :
*/
int inCREDIT_Func_GetCVV2(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szCardLable[20 + 1];
	char		szFuncEnable[2 + 1];
	DISPLAY_OBJECT  srDispObj;

	memset(szCardLable, 0x00, sizeof(szCardLable));
	inGetCardLabel(szCardLable);

	/* 擋大來卡 */
	if (memcmp(szCardLable, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

		return (VS_ERROR);
	}
	/* AE由GET 4DBC那一隻來處理 UCARD不用處理 */
	else if (memcmp(szCardLable, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0	||
		 memcmp(szCardLable, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		/* 剩下的要輸入CVV2 */
	}

	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->szCVV2Value) > 0)
		{
			return (VS_SUCCESS);
		}
		else
		{

		}
	}
	else
	{
		if (strlen(pobTran->szCVV2Value) > 0)
		{
			return (VS_SUCCESS);
		}
	}

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 3;
	srDispObj.inColor = _COLOR_RED_;

	while (1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 請輸入背面末三碼 */
		inDISP_PutGraphic(_GET_CVV2_, 0, _COORDINATE_Y_LINE_8_4_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		if (srDispObj.inOutputLen == 3)
		{
			memcpy(&pobTran->szCVV2Value[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
			break;
		}
		else if (inRetVal == 0)
		{
			/* 若FORCECVV2未開可By Pass */
			inGetFORCECVV2(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0)
			{
				break;
			}
		}
		else
		{
			memset(srDispObj.szOutput, 0x00, srDispObj.inOutputLen);
			srDispObj.inOutputLen = 0;
		}

	}

	if (strlen(pobTran->szCVV2Value) > 0)
		pobTran->srBRec.uszCVV2Bit = VS_TRUE;

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_OPT_RFNumber
Date&Time       :2018/1/18 下午 3:10
Describe        :請輸入RF序號
*/
int inCREDIT_Func_Get_OPT_RFNumber(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
	char		szDebugMsg[100 + 1];
        DISPLAY_OBJECT  srDispObj;

	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srTRec.szTicketRefundCode) > 0)
		{
			return (VS_SUCCESS);
		}
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		else if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}
		else
		{

		}
	}
	else
	{
		if (strlen(pobTran->srTRec.szTicketRefundCode) > 0)
		{
			return (VS_SUCCESS);
		}
	}

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 6;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 請輸入RF序號 */
        inDISP_PutGraphic(_GET_RF_NUMBER_, 0, _COORDINATE_Y_LINE_8_4_);

	while (1)
	{
		inDISP_Clear_Line(srDispObj.inY, _LINE_8_8_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		memset(pobTran->srTRec.szTicketRefundCode, 0x00, sizeof(pobTran->srTRec.szTicketRefundCode));
		memcpy(pobTran->srTRec.szTicketRefundCode, srDispObj.szOutput, srDispObj.inOutputLen);

		/* RF序號，只允許輸入12碼，不多不少 */
		if (srDispObj.inOutputLen == 6)
		{
			break;
		}
		else
		{
			memset(pobTran->srTRec.szTicketRefundCode, 0x00, sizeof(pobTran->srTRec.szTicketRefundCode));
			srDispObj.inOutputLen = 0;
		}

	}
	
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inCREDIT_Func_Get_OPT_RFNumber (RF = %s)END", pobTran->srTRec.szTicketRefundCode);
		inDISP_LogPrintf(szDebugMsg);
	}
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_CUP_TN
Date&Time	:2017/9/7 下午 6:27
Describe	: CUP Trace Number
*/
int inCREDIT_Func_Get_OPT_CUP_TN(TRANSACTION_OBJECT *pobTran)
{
	int 	inRetVal;
	char	szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT	srDispObj;

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inCREDIT_Func_Get_OPT_CUP_TN 1");

	/* 若ECR發動直接跳走 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 新增判斷，如果 szCUP_TN 已有資料，就不再繼續輸入 2022/11/7 [SAM] */
		/* 目前ECR規格不會帶此資料，所以改成都用手輸  2022/12/14 [SAM]*/
//		if (strlen(pobTran->srBRec.szCUP_TN) > 0 &&  pobTran->srBRec.szCUP_TN[0] != 0x20)
//		{
//			return (VS_SUCCESS);
//		}else	
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{

			return (VS_SUCCESS);
		}
		
	}

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inCREDIT_Func_Get_OPT_CUP_TN 2");

	while (1)
	{
		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		/* 可輸入0，也可以ByPass */
		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMaxLen = 6;
		srDispObj.inColor = _COLOR_RED_;
		//strcpy(srDispObj.szPromptMsg, "NT$ ");

		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("inCREDIT_Func_Get_OPT_CUP_TN 3");
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 請輸入原交易日期 */
		inDISP_PutGraphic(_GET_CUP_TN_, 0, _COORDINATE_Y_LINE_8_6_);

		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		{
			return (inRetVal);
		}else if(strlen(srDispObj.szOutput) == 6)
		{
			break;
		}else
			continue;
	}


	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inCREDIT_Func_Get_OPT_CUP_TN 4");
	
	sprintf(pobTran->srBRec.szCUP_TN,"%06ld",atol(srDispObj.szOutput));

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	return (VS_SUCCESS);
}
