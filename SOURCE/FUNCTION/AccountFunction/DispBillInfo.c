
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

#include "../../EVENT/MenuMsg.h"

#include "../../FUNCTION/Batch.h"
#include "../../FUNCTION/Sqlite.h"

#include "../../DISPLAY/Display.h"
#include "../../DISPLAY/DisTouch.h"
#include "../../DISPLAY/DispMsg.h"

#include "../Function.h"
#include "../Accum.h"
#include "../EDC.h"

extern int ginDebug;


/*
Function        : inFunc_Display_Review_Settle
Date&Time   : 20190308
Describe        :
*/
int inFunc_Display_Review_Settle(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_BATCH_END_;
	char		szTemplate1[44 + 1] = {0};
	char		szTemplate2[44 + 1] = {0};
	char		szDispBuffer1[44 + 1] = {0};
	long long	llSum = 0;/* 用來加總金額用 */
	unsigned char	uszKey = 0x00;
	ACCUM_TOTAL_REC	srAccumRec = {0};

	/* 讀交易資料，並放交易查詢Title */
	memset(&srAccumRec, 0x00, sizeof(ACCUM_TOTAL_REC));
	inACCUM_GetRecord(pobTran, &srAccumRec);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_REVIEW_SETTLE_, 0, _COORDINATE_Y_LINE_8_4_);

	/* 銷售筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	llSum = srAccumRec.lnTotalSaleCount;
	sprintf(szTemplate1, "%03lld", llSum);

	/* 銷售金額 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	/* NCCC的要分別加起來 */
	llSum = srAccumRec.llTotalSaleAmount + srAccumRec.llTotalTipsAmount;
	sprintf(szTemplate2, "%01lld", llSum);

	inFunc_Amount(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PAD_RIGHT_FILL_LEFT_);
	sprintf(szDispBuffer1, "%s%s", szTemplate1, szTemplate2);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_,_COLOR_WHITE_, 7);

	/* 退貨筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	llSum = srAccumRec.lnTotalRefundCount;
	sprintf(szTemplate1, "%03lld", llSum);

	/* 退貨金額 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	llSum = srAccumRec.llTotalRefundAmount;
	sprintf(szTemplate2, "%01lld", (0 - llSum));

	inFunc_Amount(szTemplate2, "$", ' ', _SIGNED_MINUS_, 11, _PAD_RIGHT_FILL_LEFT_);
	sprintf(szDispBuffer1, "%s%s", szTemplate1, szTemplate2);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_,_COLOR_WHITE_, 7);

	/* 淨額筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	llSum = srAccumRec.lnTotalCount;
	sprintf(szTemplate1, "%03lld", llSum);

	/* 淨額金額 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	llSum = srAccumRec.llTotalAmount;
	sprintf(szTemplate2, "%01lld", llSum);

	inFunc_Amount(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PAD_RIGHT_FILL_LEFT_);
	sprintf(szDispBuffer1, "%s%s", szTemplate1, szTemplate2);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_,_COLOR_WHITE_, 7);

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

	uszKey = 0x00;
	while (1)
	{
		/* 抓取 KioskFlag的值，如為 TRUE 就不用確認畫面 2020/3/6 下午 4:28 [SAM] */
		if(inFunc_GetKisokFlag() == VS_TRUE)
		{
			uszKey = _KEY_0_;
			inDISP_LogPrintfWithFlag(" FU Kiosk DispReviewSettle Line[%d]", __LINE__);
		}else
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			uszKey = uszKBD_Key();
		}

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (inChoice == _BATCH_END_Touch_ENTER_BUTTON_	||
			 uszKey == _KEY_0_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	return (inRetVal);
}


/*
Function        : inFunc_Display_Review
Date&Time   : 20190308
Describe        :
*/
int inFunc_Display_Review(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_NEWUI_REVIEW_TOTAL_;
	char		szTemplate1[44 + 1] = {0};
	char		szTemplate2[44 + 1] = {0};
	char		szDispBuffer1[44 + 1] = {0};
	long long	llSum = 0; /* 用來加總金額用 */
	unsigned char	uszKey = 0x00;
	ACCUM_TOTAL_REC	srAccumRec = {0};

	/* 讀交易資料，並放交易查詢Title */
	memset(&srAccumRec, 0x00, sizeof(ACCUM_TOTAL_REC));
	inACCUM_GetRecord(pobTran, &srAccumRec);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_REVIEW_TOTAL_, 0, _COORDINATE_Y_LINE_8_4_);

	/* 銷售筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	llSum = srAccumRec.lnTotalSaleCount;
	sprintf(szTemplate1, "%03lld", llSum);

	/* 銷售金額 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	llSum = srAccumRec.llTotalSaleAmount + srAccumRec.llTotalTipsAmount;
	sprintf(szTemplate2, "%01lld", llSum);

	inFunc_Amount(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PAD_RIGHT_FILL_LEFT_);
	sprintf(szDispBuffer1, "%s%s", szTemplate1, szTemplate2);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_,_COLOR_WHITE_, 7);

	/* 退貨筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	llSum = srAccumRec.lnTotalRefundCount;
	sprintf(szTemplate1, "%03lld", llSum);

	/* 退貨金額 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	llSum = srAccumRec.llTotalRefundAmount;
	sprintf(szTemplate2, "%01lld", (0 - llSum));

	inFunc_Amount(szTemplate2, "$", ' ', _SIGNED_MINUS_, 11, _PAD_RIGHT_FILL_LEFT_);
	sprintf(szDispBuffer1, "%s%s", szTemplate1, szTemplate2);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_,_COLOR_WHITE_, 7);

	/* 淨額筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	llSum = srAccumRec.lnTotalCount;
	sprintf(szTemplate1, "%03lld", llSum);

	/* 淨額金額 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	llSum = srAccumRec.llTotalAmount;

	sprintf(szTemplate2, "%01lld", llSum);
	inFunc_Amount(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PAD_RIGHT_FILL_LEFT_);
	sprintf(szDispBuffer1, "%s%s", szTemplate1, szTemplate2);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_,_COLOR_WHITE_, 7);


	uszKey = 0x00;
	inTouchSensorFunc = _Touch_NEWUI_REVIEW_TOTAL_;
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		uszKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (uszKey == _KEY_ENTER_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (inChoice == _NEWUI_REVIEW_TOTAL_Touch_RETURN_IDLE_BUTTON_	||
			 uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_ERROR;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (inChoice == _NEWUI_REVIEW_TOTAL_Touch_LAST_PAGE_BUTTON_	||
			 uszKey == _KEY_CLEAR_)
		{
			inRetVal = VS_LAST_PAGE;
			pobTran->srBRec.inHDTIndex = -1;
			break;
		}
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	return (inRetVal);
}




/*
Function        : inFunc_Display_Review_ESC_Reinforce
Date&Time   : 20190308
Describe        :
*/
int inFunc_Display_Review_ESC_Reinforce(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_NEWUI_REVIEW_TOTAL_;
#ifndef __USE_ACCUM_DATA__
	int		inCnt = 0;
	int		inSaleUploadCnt = 0, inRefundUploadCnt = 0, inSalePaperCnt = 0, inRefundPaperCnt = 0;
	long		lnSaleUploadAmt = 0, lnRefundUploadAmt = 0, lnSalePaperAmt = 0, lnRefundPaperAmt = 0;
	long		lnAmt = 0;
#endif
	
	char		szTemplate1[44 + 1] = {0};
	char		szTemplate2[44 + 1] = {0};
	char		szDispBuffer1[64 + 1] = {0};
	char		szFuncEnable[2 + 1] = {0};
	unsigned char	uszKey = 0x00;
	ACCUM_TOTAL_REC	srAccumRec = {0};
	
	
	/* ESC沒開直接跳走 */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetESCMode(szFuncEnable);
	if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0)
	{
		return (VS_SUCCESS);
	}

#ifdef __USE_ACCUM_DATA__
	/* 讀交易資料，並放交易查詢Title */
	memset(&srAccumRec, 0x00, sizeof(ACCUM_TOTAL_REC));
	inACCUM_GetRecord(pobTran, &srAccumRec);
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	/* 銷售已上傳筆數 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	sprintf(szTemplate1, "%03ld", srAccumRec.lnESC_SuccessNum);
	inDISP_ChineseFont_Color("電簽已上傳總筆數", _FONTSIZE_16X22_, _LINE_16_7_, _COLOR_BLACK_, _DISP_LEFT_ );
	inDISP_ChineseFont_Point_Color(szTemplate1, _FONTSIZE_16X22_, _LINE_16_7_, _COLOR_BLACK_,_COLOR_WHITE_ , 21 );
	
	/* 銷售已上傳金額 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	if (srAccumRec.llESC_SuccessAmount >= 0L)
	{
		sprintf(szTemplate2, "%lld", srAccumRec.llESC_SuccessAmount);
	}
	else
	{
		sprintf(szTemplate2, "%lld", srAccumRec.llESC_SuccessAmount);
	}
	
	inDISP_LogPrintfWithFlag("  Success  [%s]  ", szTemplate2);
	
	inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);

	inDISP_LogPrintfWithFlag("  Success1  [%s]  ", szTemplate2);
	
	inDISP_ChineseFont_Color("電簽已上傳總金額", _FONTSIZE_16X22_, _LINE_16_8_, _COLOR_BLACK_, _DISP_LEFT_ );
	inDISP_ChineseFont_Color(szTemplate2, _FONTSIZE_16X22_, _LINE_16_9_, _COLOR_BLACK_, _DISP_RIGHT_ );
	
	
	/* 銷售已上傳筆數 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	sprintf(szTemplate1, "%03ld",  (srAccumRec.lnESC_TotalFailULNum + srAccumRec.lnESC_RePrintNum));
	inDISP_ChineseFont_Color("紙本簽單總筆數", _FONTSIZE_16X22_, _LINE_16_11_, _COLOR_BLACK_, _DISP_LEFT_ );
	inDISP_ChineseFont_Point_Color(szTemplate1, _FONTSIZE_16X22_, _LINE_16_11_, _COLOR_BLACK_, _COLOR_WHITE_,21 );
	
	/* 銷售已上傳金額 */
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	if (srAccumRec.llESC_TotalFailULAmount >= 0L)
	{
		sprintf(szTemplate2, "%lld", srAccumRec.llESC_TotalFailULAmount);
	}
	else
	{
		sprintf(szTemplate2, "%lld",  srAccumRec.llESC_TotalFailULAmount);
	}

	inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);
	
	inDISP_ChineseFont_Color("紙本簽單總金額", _FONTSIZE_16X22_, _LINE_16_12_, _COLOR_BLACK_, _DISP_LEFT_ );
	inDISP_ChineseFont_Color(szTemplate2, _FONTSIZE_16X22_, _LINE_16_13_, _COLOR_BLACK_, _DISP_RIGHT_ );
	
	
#else	
	/* 讀交易資料，並放交易查詢Title */
	/* Sale已上傳 */
	if (inBATCH_Get_ESC_Upload_Count_Flow(pobTran, _TN_BATCH_TABLE_, _ESC_REINFORCE_TXNCODE_SALE_, VS_FALSE, &inSaleUploadCnt, &lnSaleUploadAmt) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("ESC COUNT SQLite read fail.");
		}

	}

	/* Refund已上傳 */
	if (inBATCH_Get_ESC_Upload_Count_Flow(pobTran, _TN_BATCH_TABLE_, _ESC_REINFORCE_TXNCODE_REFUND_, VS_FALSE, &inRefundUploadCnt, &lnRefundUploadAmt) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("ESC COUNT SQLite read fail.");
		}

	}

	/* Sale出紙本 */
	if (inBATCH_Get_ESC_Upload_Count_Flow(pobTran, _TN_BATCH_TABLE_, _ESC_REINFORCE_TXNCODE_SALE_, VS_TRUE, &inSalePaperCnt, &lnSalePaperAmt) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("ESC COUNT SQLite read fail.");
		}

	}

	/* Refund出紙本 */
	if (inBATCH_Get_ESC_Upload_Count_Flow(pobTran, _TN_BATCH_TABLE_, _ESC_REINFORCE_TXNCODE_REFUND_, VS_TRUE, &inRefundPaperCnt, &lnRefundPaperAmt) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("ESC COUNT SQLite read fail.");
		}

	}


	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_REVIEW_TOTAL_ESC_REINFORCE_, 0, _COORDINATE_Y_LINE_8_4_);

	/* 銷售已上傳筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	inCnt = inSaleUploadCnt;
	sprintf(szTemplate1, "%03d", inCnt);

	/* 銷售已上傳金額 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	lnAmt = lnSaleUploadAmt;
	sprintf(szTemplate2, "%01ld", lnAmt);

	inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PAD_RIGHT_FILL_LEFT_);
	sprintf(szDispBuffer1, "%s%s", szTemplate1, szTemplate2);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_,_COLOR_WHITE_, 7);

	/* 銷售未上傳筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	inCnt = inSalePaperCnt;
	sprintf(szTemplate1, "%03d", inCnt);

	/* 銷售未上傳金額 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	lnAmt = lnSalePaperAmt;
	sprintf(szTemplate2, "%01ld", lnAmt);

	inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PAD_RIGHT_FILL_LEFT_);
	sprintf(szDispBuffer1, "%s%s", szTemplate1, szTemplate2);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_,_COLOR_WHITE_, 7);

	/* 退貨已上傳筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	inCnt = inRefundUploadCnt;
	sprintf(szTemplate1, "%03d", inCnt);

	/* 退貨已上傳金額 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	lnAmt = lnRefundUploadAmt;
	sprintf(szTemplate2, "%01ld", lnAmt);

	inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PAD_RIGHT_FILL_LEFT_);
	sprintf(szDispBuffer1, "%s%s", szTemplate1, szTemplate2);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_,_COLOR_WHITE_, 7);

	/* 退貨未上傳筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	inCnt = inRefundUploadCnt;
	sprintf(szTemplate1, "%03d", inCnt);

	/* 退貨未上傳金額 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	lnAmt = lnRefundUploadAmt;
	sprintf(szTemplate2, "%01ld", lnAmt);

	inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PAD_RIGHT_FILL_LEFT_);
	sprintf(szDispBuffer1, "%s%s", szTemplate1, szTemplate2);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_,_COLOR_WHITE_, 7);

	
#endif	
	
	uszKey = 0x00;
	inTouchSensorFunc = _Touch_NEWUI_REVIEW_TOTAL_;
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		uszKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (inChoice == _NEWUI_REVIEW_TOTAL_Touch_RETURN_IDLE_BUTTON_	||
		    uszKey == _KEY_ENTER_		||
		    uszKey == _KEY_CANCEL_	||
		    uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (inChoice == _NEWUI_REVIEW_TOTAL_Touch_LAST_PAGE_BUTTON_	||
			 uszKey == _KEY_CLEAR_)
		{
			inRetVal = VS_LAST_PAGE;
			pobTran->srBRec.inHDTIndex = -1;
			break;
		}
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	return (inRetVal);
}



