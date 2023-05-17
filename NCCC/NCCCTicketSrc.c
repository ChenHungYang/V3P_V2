
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>
#include <libxml/parser.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/COMM/Comm.h"
#include "../SOURCE/COMM/Modem.h"
#include "../SOURCE/FUNCTION/ASMC.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/Batch.h"
#include "../SOURCE/FUNCTION/FILE_FUNC/File.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/Card.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/ECR_FUNC/ECR.h"
#include "../SOURCE/FUNCTION/ECR_FUNC/RS232.h"
#include "../SOURCE/FUNCTION/IPASSDT.h"
#include "../SOURCE/FUNCTION/ECCDT.h"
#include "../SOURCE/FUNCTION/TDT.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Flow.h"
//#include "../CREDIT/Creditfunc.h"
//#include "../FISC/NCCCfisc.h"
#include "../EMVSRC/EMVsrc.h"
//#include "../IPASS/IPASSFunc.h"
#include "../CTLS/CTLS.h"
#include "../ECC/ECC.h"

#include "NCCCTicketSrc.h"


extern int	ginDebug;			/* Debug使用 extern */
extern int	ginISODebug;		/* Debug使用 extern */
extern int	ginDisplayDebug;	/* Debug使用 extern */

extern	sqlite3		*gsrDBConnection;	/* 建立到資料庫的connection */
extern	sqlite3_stmt	*gsrSQLStat;

SQLITE_TAG_TABLE TABLE_TICKET_TAG[] = 
{
	{"inTableID"		,"INTEGER"	,"PRIMARY KEY"	,""},	/* Table ID Primary key, sqlite table專用避免PRIMARY KEY重複 */
	{"inCode"			,"INTEGER"	,""		,""},	
	{"inPrintOption"		,"INTEGER"	,""		,""},	/* Print Option Flag (also in TCT) */
	{"inTicketType"			,"INTEGER"	,""		,""},	/* 票證種類 - 交易或明細使用  */
	{"inTDTIndex"			,"INTEGER"	,""		,""},	/* 存TDT的index */
	{"lnTxnAmount"			,"INTEGER"	,""		,""},	/* 交易金額 */
	{"lnTopUpAmount"		,"INTEGER"	,""		,""},	/* 加值金額(基底) */
	{"lnTotalTopUpAmount"	,"INTEGER"	,""		,""},	/* 加值金額 */
	{"lnCardRemainAmount"	,"INTEGER"	,""		,""},	/* 卡片餘額 */
	{"lnInvNum"			,"INTEGER"	,""		,""},	/* For簽單使用，簽單序號 */
	{"lnECCInvNum"			,"INTEGER"	,""		,""},	/* ECC Transaction Invoice # */
	{"lnSTAN"				,"INTEGER"	,""		,""},	/* */
	{"lnFinalBeforeAmt"		,"INTEGER"	,""		,""},	/* 最後交易結構，交易前卡片餘額 */
	{"lnFinalAfterAmt"		,"INTEGER"	,""		,""},	/* 最後交易結構，交易後卡片餘額 */
	{"lnMainInvNum"		,"INTEGER"	,""		,""},	/* Confirm use 電文序號，因電票中一筆交易可能有數筆電文，所以簽單序號和電文序號分開 */
	{"lnCountInvNum"		,"INTEGER"	,""		,""},	/* Confirm use 若有advice，要預先跳過的電文序號 */
	{"lnTicketCTN"			,"INTEGER"	,""		,""},	/* [新增電票悠遊卡功能]  電子票證卡片交易序號  for IPASS 2.0  [SAM] 2022/6/14 上午 10:20 */
	
	{"szUID"				,"BLOB"		,""		,""},	/* 卡號 or UID number */
	{"szDate"				,"BLOB"		,""		,""},	/* YYMMDD */
	{"szOrgDate"			,"BLOB"		,""		,""},	/* YYMMDD */
	{"szTime"				,"BLOB"		,""		,""},	/* HHMMSS */
	{"szOrgTime"			,"BLOB"		,""		,""},	/* HHMMSS */
	{"szAuthCode"			,"BLOB"		,""		,""},	/* */
	{"szECCAuthCode"		,"BLOB"		,""		,""},	
	{"szRespCode"			,"BLOB"		,""		,""},	/* Response Code */
	{"szProductCode"		,"BLOB"		,""		,""},	/* 產品代碼  */
	{"szTicketRefundCode"	,"BLOB"		,""		,""},	/* 退貨序號  */
	{"szTicketRefundDate"	,"BLOB"		,""		,""},	/* MMDD */
	{"szStoreID"			,"BLOB"		,""		,""},	/* */
	{"szRefNo"				,"BLOB"		,""		,""},	/* Reference Number(RRN) */
	{"szAwardNum"			,"BLOB"		,""		,""},	/* 優惠個數 */
	{"szAwardSN"			,"BLOB"		,""		,""},	/* 優惠序號(Award S/N) TID(8Bytes)+YYYYMMDDhhmmss(16 Bytes)，共22Bytes */
	{"uszAutoTopUpBit"		,"BLOB"		,""		,""},	/* 是否自動加值 */
	{"uszBlackListBit"		,"BLOB"		,""		,""},	/* 是否在黑名單中 */
	{"uszOfflineBit"			,"BLOB"		,""		,""},	/* 離線交易 */
	{"uszTicketConnectBit"	,"BLOB"		,""		,""},	/* 是否連線中 */
	{"uszResponseBit"		,"BLOB"		,""		,""},	/* 票值回覆用 */
	{"uszCloseAutoADDBit"	,"BLOB"		,""		,""},	/* 關閉自動加值用 */
	{"uszStopPollBit"		,"BLOB"		,""		,""},	/* Mifare Stop */
	{"uszConfirmBit"			,"BLOB"		,""		,""},	/* IPASS Confirm Inv use */
	{"uszESVCTransBit"		,"BLOB"		,""		,""},	/* 代表是電票交易 */
	{"srIPASSRec"			,"BLOB"		,""		,""},	/* IPASS結構，求快先整個存 */
	{"srECCRec"			,"BLOB"		,""		,""},	/* ECC結構，求快先整個存 */
	{"srICASHRec"			,"BLOB"		,""		,""},	/* ICASH結構，求快先整個存 */
	{"srHAPPYCASHRec"		,"BLOB"		,""		,""},	/* HAPPYCASH結構，求快先整個存 */
	{"uszUpdated"			,"BLOB"		,""		,"DEFAULT 0"},	/* For SQLite使用，pobTran中不存，若設為1則代表該紀錄已不用，初始值設為0 */
	{""},
};


/*
Function        :inNCCC_Ticket_Func_Check_Transaction_Deduct
Date&Time       :2017/12/19 上午 9:42
Describe        :確認全票證扣款功能開關 主要for主頁面顯示開關使用，只要有一個開就顯示可使用
*/
int inNCCC_Ticket_Func_Check_Transaction_Deduct(int inCode)
{
	char		szIPASS_TransFunc[20 + 1];
	unsigned char	uszTxnEnable = VS_FALSE;
	
	/* 先load IPASS 參數檔 */
	inLoadIPASSDTRec(0);	

	memset(szIPASS_TransFunc, 0x00, sizeof(szIPASS_TransFunc));
	if (inGetIPASS_Transaction_Function(szIPASS_TransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* IPASS 扣款開關 */
	if (memcmp(&szIPASS_TransFunc[0], "Y", 1) == 0)
	{
		uszTxnEnable = VS_TRUE;
	}
	
	/* 悠遊卡扣款開關 */
	
	/* 愛金卡扣款開關 */
	
	/* 遠鑫卡扣款開關 */
	
	if (uszTxnEnable == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
	
}

/*
Function        :inNCCC_Ticket_Func_Check_Transaction_Refund
Date&Time       :2017/12/19 下午 1:28
Describe        :確認全票證退貨功能開關 主要for主頁面顯示開關使用，只要有一個開就顯示可使用
*/
int inNCCC_Ticket_Func_Check_Transaction_Refund(int inCode)
{
	char		szIPASS_TransFunc[20 + 1];
	unsigned char	uszTxnEnable = VS_FALSE;
	
	/* 先load IPASS 參數檔 */
	inLoadIPASSDTRec(0);	

	memset(szIPASS_TransFunc, 0x00, sizeof(szIPASS_TransFunc));
	if (inGetIPASS_Transaction_Function(szIPASS_TransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* IPASS 退貨開關 */
	if (memcmp(&szIPASS_TransFunc[2], "Y", 1) == 0)
	{
		uszTxnEnable = VS_TRUE;
	}
	
	/* 悠遊卡退貨開關 */
	
	/* 愛金卡退貨開關 */
	
	/* 遠鑫卡退貨開關 */
	
	if (uszTxnEnable == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
	
}

/*
Function        :inNCCC_Ticket_Func_Check_Transaction_Query
Date&Time       :2017/12/19 下午 1:28
Describe        :確認全票證詢卡功能開關 主要for主頁面顯示開關使用，只要有一個開就顯示可使用
*/
int inNCCC_Ticket_Func_Check_Transaction_Inquiry(int inCode)
{
	char		szIPASS_TransFunc[20 + 1];
	unsigned char	uszTxnEnable = VS_FALSE;
	
	/* 先load IPASS 參數檔 */
	inLoadIPASSDTRec(0);	

	memset(szIPASS_TransFunc, 0x00, sizeof(szIPASS_TransFunc));
	if (inGetIPASS_Transaction_Function(szIPASS_TransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* IPASS 詢卡開關 */
	if (memcmp(&szIPASS_TransFunc[7], "Y", 1) == 0)
	{
		uszTxnEnable = VS_TRUE;
	}
	
	/* 悠遊卡詢卡開關 */
	
	/* 愛金卡詢卡開關 */
	
	/* 遠鑫卡詢卡開關 */
	
	if (uszTxnEnable == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
	
}

/*
Function        :inNCCC_Ticket_Func_Check_Transaction_Top_Up
Date&Time       :2017/12/19 下午 1:28
Describe        :確認全票證加值功能開關 主要for主頁面顯示開關使用，只要有一個開就顯示可使用
*/
int inNCCC_Ticket_Func_Check_Transaction_Top_Up(int inCode)
{
	char		szIPASS_TransFunc[20 + 1];
	unsigned char	uszTxnEnable = VS_FALSE;
	
	/* 先load IPASS 參數檔 */
	inLoadIPASSDTRec(0);	

	memset(szIPASS_TransFunc, 0x00, sizeof(szIPASS_TransFunc));
	if (inGetIPASS_Transaction_Function(szIPASS_TransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* IPASS 加值開關 */
	if (memcmp(&szIPASS_TransFunc[4], "Y", 1) == 0)
	{
		uszTxnEnable = VS_TRUE;
	}
	
	/* 悠遊卡加值開關 */
	
	/* 愛金卡加值開關 */
	
	/* 遠鑫卡加值開關 */
	
	if (uszTxnEnable == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
	
}

/*
Function        :inNCCC_Ticket_Func_Check_Transaction_Void_Top_Up
Date&Time       :2017/12/19 下午 1:28
Describe        :確認全票證取消加值功能開關 主要for主頁面顯示開關使用，只要有一個開就顯示可使用
*/
int inNCCC_Ticket_Func_Check_Transaction_Void_Top_Up(int inCode)
{
	char		szIPASS_TransFunc[20 + 1];
	unsigned char	uszTxnEnable = VS_FALSE;
	
	/* 先load IPASS 參數檔 */
	inLoadIPASSDTRec(0);	

	memset(szIPASS_TransFunc, 0x00, sizeof(szIPASS_TransFunc));
	if (inGetIPASS_Transaction_Function(szIPASS_TransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* IPASS 取消加值開關 */
	if (memcmp(&szIPASS_TransFunc[5], "Y", 1) == 0)
	{
		uszTxnEnable = VS_TRUE;
	}
	
	/* 悠遊卡取消加值開關 */
	
	/* 愛金卡取消加值開關 */
	
	/* 遠鑫卡取消加值開關 */
	
	if (uszTxnEnable == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
	
}

/*
Function        : inNCCC_Ticket_Display_Transaction_Result
Date&Time   : 2022/6/14 上午 11:43
Describe        :
 * [新增電票悠遊卡功能]  [SAM] 
*/
int inNCCC_Ticket_Display_Transaction_Result(TRANSACTION_OBJECT *pobTran)
{
	char 	szDispMsg[50 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inNCCC_Ticket_Display_Transaction_Result() START");
		inDISP_LogPrintfAt(AT,"srTRec.inCode(%d), srTRec.lnTotalTopUpAmount(%ld)",pobTran->srTRec.inCode,pobTran->srTRec.lnTotalTopUpAmount);
	}        
        
	if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
	{        
		if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_)
		{
			/* 交易完成 自動加值 購貨金額 票卡餘額 請按確認或清除 */
			inDISP_Clear_Line(_LINE_8_3_, _LINE_8_8_);
			inDISP_PutGraphic(_CHECK_TICKET_RESULT_10_, 0, _COORDINATE_Y_LINE_8_3_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount);
			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_4_, _COLOR_RED_, _DISP_RIGHT_);
                        
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTxnAmount);
			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
		
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srTRec.lnCardRemainAmount < 0)
			{
				sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount - pobTran->srTRec.lnCardRemainAmount);
			}
			else
			{
				sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount + pobTran->srTRec.lnCardRemainAmount);
			}

			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
			inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
		}
		else
		{
			/* 交易完成 自動加值 購貨金額 票卡餘額 請按確認或清除 */
			inDISP_Clear_Line(_LINE_8_3_, _LINE_8_8_);
			inDISP_PutGraphic(_CHECK_TICKET_RESULT_10_, 0, _COORDINATE_Y_LINE_8_3_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount);
			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_4_, _COLOR_RED_, _DISP_RIGHT_);
                        
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTxnAmount);
			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
			
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srTRec.lnFinalAfterAmt > 100000)
			{
				sprintf(szDispMsg, "-$%ld ", pobTran->srTRec.lnFinalAfterAmt - 100000);
			}
			else
			{
				sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnFinalAfterAmt);
			}

			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
			inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
		}
	}
	else
	{
		if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_ ||
		    pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_)
		{
			/* 加值完成 加值金額 票卡餘額 請按確認或清除 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_CHECK_TICKET_RESULT_08_, 0, _COORDINATE_Y_LINE_8_4_);
			
			/* 加值金額 */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTxnAmount);
			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

			/* 票卡餘額 */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srTRec.lnFinalAfterAmt > 100000)
			{
				sprintf(szDispMsg, "-$%ld ", pobTran->srTRec.lnFinalAfterAmt - 100000);
			}
			else
			{
				sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnFinalAfterAmt);
			}

			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
			inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
		}
		else if (pobTran->srTRec.inCode == _TICKET_IPASS_TOP_UP_ ||
		    pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_)
		{
			/* 加值完成 加值金額 票卡餘額 請按確認或清除 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_CHECK_TICKET_RESULT_03_, 0, _COORDINATE_Y_LINE_8_4_);
			
			/* 加值金額 */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTxnAmount);
			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

			/* 票卡餘額 */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srTRec.lnFinalAfterAmt > 100000)
			{
				sprintf(szDispMsg, "-$%ld ", pobTran->srTRec.lnFinalAfterAmt - 100000);
			}
			else
			{
				sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnFinalAfterAmt);
			}

			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
			inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
		}
		else if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_ ||
			 pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_)
		{
			/* 取消加值完成 取消金額 票卡餘額 請按確認或清除 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_CHECK_TICKET_RESULT_04_, 0, _COORDINATE_Y_LINE_8_4_);
			
			/*20191231 Edited by Hachi start*/
			// memset(szDispMsg, 0x00, sizeof(szDispMsg));
			// sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnTxnAmount);
			// inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "$%ld", ( 0 - pobTran->srTRec.lnTxnAmount));
			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

			/*20191231 Edited by Hachi End*/

			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srTRec.lnFinalAfterAmt > 100000)
			{
				sprintf(szDispMsg, "-$%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);
			}
			else
			{
				sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnFinalAfterAmt);
			}

			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
			inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
		}        
		else if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ ||
			 pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_)
		{       
			/* 交易完成 票卡餘額 請按確認或清除 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_CHECK_TICKET_RESULT_01_, 0, _COORDINATE_Y_LINE_8_4_);
			
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srTRec.lnCardRemainAmount < 0)
			{
				sprintf(szDispMsg, "-$%ld", 0 - pobTran->srTRec.lnCardRemainAmount);
			}
			else
			{
				sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnCardRemainAmount);
			}

			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
			inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);
			
			inDISP_BEEP(1, 0); 
		}
		else if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_DEDUCT_ ||
			 pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_DEDUCT_)
		{       
			/* 取消購貨完成 取消購貨金額 票卡餘額 請按確認或清除 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_CHECK_TICKET_RESULT_07_, 0, _COORDINATE_Y_LINE_8_4_);
			
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnTxnAmount);
			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srTRec.lnFinalAfterAmt > 100000)
			{
				sprintf(szDispMsg, "-$%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);
			}
			else
			{
				sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnFinalAfterAmt);
			}

			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
			inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
		}
		else if (pobTran->srTRec.inCode == _TICKET_IPASS_ADD_RETURN_ )
		{       
			/* 溢扣返還完成 取消購貨金額 票卡餘額 請按確認或清除 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_CHECK_TICKET_RESULT_06_, 0, _COORDINATE_Y_LINE_8_4_);
			
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnTxnAmount);
			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srTRec.lnFinalAfterAmt > 100000)
			{
				sprintf(szDispMsg, "-$%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);
			}
			else
			{
				sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnFinalAfterAmt);
			}

			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
			inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
		}
		else if (pobTran->srTRec.inCode == _TICKET_IPASS_REFUND_ )
		{       
			/* 退貨完成 取消購貨金額 票卡餘額 請按確認或清除 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_CHECK_TICKET_RESULT_05_, 0, _COORDINATE_Y_LINE_8_4_);
			
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnTxnAmount);
			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srTRec.lnFinalAfterAmt > 100000)
			{
				sprintf(szDispMsg, "-$%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);
			}
			else
			{
				sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnFinalAfterAmt);
			}

			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
			inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
		}				
		else
		{
			/* 交易完成 票卡餘額 請按確認或清除 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_CHECK_TICKET_RESULT_01_, 0, _COORDINATE_Y_LINE_8_4_);
			
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srTRec.lnFinalAfterAmt > 100000)
			{
				sprintf(szDispMsg, "-$%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);
			}
			else
			{
				sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnFinalAfterAmt);
			}

			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
			
			inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
		}
	}
        	
	return (VS_SUCCESS);
}



/*
Function        :inTicket_Logon_ShowResult
Date&Time       :2018/1/17 下午 3:32
Describe        :
*/
int inNCCC_Ticket_Logon_ShowResult()
{
	int			i = 0;
	int			inHostOpenCnt = 0;
	int			inHostShowCnt = 0;
	int			inChoice = 0;
	int			inTouchSensorFunc = _Touch_BATCH_END_;
	char			szTemplate[2 + 1] = {0};
	char			szTicketEnable[4 + 1] = {0};	/* index0為IPASS 依序為ECC、ICASH、HAPPYCASH */
	char			szTicketLogOn[4 + 1] = {0};
	char			szTicketFailLogOnResult[4 + 1] = {0};
	char			szHost[4][10 + 1];
	unsigned char		uszKey = 0x00;
        
	/* IPASS */
        inLoadTDTRec(_TDT_INDEX_00_IPASS_);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostEnable(szTemplate);
	szTicketEnable[0] = szTemplate[0];
	
        inGetTicket_LogOnOK(szTemplate);
	szTicketLogOn[0] = szTemplate[0];
	
	/* ECC */
	inLoadTDTRec(_TDT_INDEX_01_ECC_);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostEnable(szTemplate);
	szTicketEnable[1] = szTemplate[0];
	
        inGetTicket_LogOnOK(szTemplate);
	szTicketLogOn[1] = szTemplate[0];
	
	/* IACSH */
	inLoadTDTRec(_TDT_INDEX_02_ICASH_);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostEnable(szTemplate);
	szTicketEnable[2] = szTemplate[0];
	
        inGetTicket_LogOnOK(szTemplate);
	szTicketLogOn[2] = szTemplate[0];
	
	/* HappyCash */
	inLoadTDTRec(_TDT_INDEX_03_HAPPYCASH_);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostEnable(szTemplate);
	szTicketEnable[3] = szTemplate[0];
	
        inGetTicket_LogOnOK(szTemplate);
	szTicketLogOn[3] = szTemplate[0];
	
	for (i = 0; i < 4; i++)
	{
		if (szTicketEnable[i] == 'Y')
		{
			inHostOpenCnt++;
		}
		
		/* 有開但沒LogOn成功 */
		if (szTicketEnable[i] == 'Y' &&
		    szTicketLogOn[i] == 'N')
		{
			szTicketFailLogOnResult[i] = 'Y';
		}
		else
		{
			szTicketFailLogOnResult[i] = 'N';
		}
	}
	

	memset(szHost, 0x00, sizeof(szHost));
	
	if (szTicketFailLogOnResult[1] == 'Y')
	{
		strcat(&szHost[inHostShowCnt][0], "ECC");
		inHostShowCnt++;
	}
	
	if (szTicketFailLogOnResult[0] == 'Y')
	{
		strcat(&szHost[inHostShowCnt][0], "IPASS");
		inHostShowCnt++;
	}
	
	if (szTicketFailLogOnResult[2] == 'Y')
	{
		strcat(&szHost[inHostShowCnt][0], "ICASH");
		inHostShowCnt++;
	}

	if (szTicketFailLogOnResult[3] == 'Y')
	{
		strcat(&szHost[inHostShowCnt][0], "HAPPYCASH");
		inHostShowCnt++;
	}
	   
        if (inHostShowCnt > 0)
        {
		/* 根據開的電票家數不同，格式亦不同 */
		/* 全失敗的情況 */
		if (inHostOpenCnt == inHostShowCnt)
		{
			/* 電票認證失敗 請重開機 */
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
			inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
			inDISP_PutGraphic(_MSG_TICKET_SIGN_ON_FAILED_REBOOT_, 0, _COORDINATE_Y_LINE_8_4_);
			inDISP_PutGraphic(_ERR_CLEAR_, 0, _COORDINATE_Y_LINE_8_7_);

			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
			while (1)
			{
				inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				uszKey = uszKBD_Key();

				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					uszKey = _KEY_TIMEOUT_;
				}

				if (uszKey == _KEY_CANCEL_	||
				    inChoice == _BATCH_END_Touch_ENTER_BUTTON_)
				{
					return (VS_USER_CANCEL);
				}
				else if (uszKey == _KEY_TIMEOUT_)
				{
					return (VS_TIMEOUT);
				}
				else
				{
					continue;
				}
			}
		}
		else if (inHostOpenCnt == 2	||
			 inHostOpenCnt == 3	||
			 inHostOpenCnt == 4)
		{
			/* 電票認證失敗 請重開機 */
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
			inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
			for (i = 0; i < inHostShowCnt; i++)
			{
				inDISP_ChineseFont(&szHost[i][0], _FONTSIZE_8X16_, _LINE_8_4_ + i, _DISP_LEFT_);
				inDISP_ChineseFont("認證失敗", _FONTSIZE_8X16_, _LINE_8_4_ + i, _DISP_RIGHT_);
			}
			inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);

			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
			while (1)
			{
				inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				uszKey = uszKBD_Key();
				
				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					uszKey = _KEY_TIMEOUT_;
				}

				if (uszKey == _KEY_ENTER_	||
				    inChoice == _BATCH_END_Touch_ENTER_BUTTON_)
				{
					return (VS_SUCCESS);
				}
				else if (uszKey == _KEY_TIMEOUT_)
				{
					return (VS_TIMEOUT);
				}
				else
				{
					continue;
				}
			}
		}	
        }
	
	return (VS_SUCCESS);
}



/*
Function        : inNCCC_Ticket_CreateBatchTable_Ticket
Date&Time   : 2022/6/14 上午 11:06
Describe        : Open Database檔 建立table 這function只負責建batch的table
*  [新增電票悠遊卡功能] 加入此功能 [SAM] 
*/
int inNCCC_Ticket_CreateBatchTable_Ticket(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int		i;
	int		inRetVal;
	int		inSqlLength = 0;
	char		szSqlPrefix[100 + 2];		/* CREATE TABLE	szTableName( */
	char		szSqlSuffix[10 + 2];		/* ); */
	char		szDebugMsg[100 + 1];
	char		*szCreateSql;			/* 因為會很長且隨table浮動，所以用pointer */
	char		*szErrorMessage = NULL;
	sqlite3		*srDBConnection;		/* 建立到資料庫的connection */
		
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"----------------------------------------");
		inDISP_LogPrintfAt(AT,"inNCCC_Ticket_CreateBatchTable_Ticket()_START");
	}
	
	/* 開啟DataBase檔 */
	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
//	inRetVal = sqlite3_open_v2(gszTranDBPath, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);/*20200612 ask 儒勳*/ 
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database ERROR Num%d", inRetVal);
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
	}

	/* 塞入SQL語句 */
	/* 為了使table name可變動，所以拉出來組 */
	/* 加入了if not exists字串，若已建立不會重複建立 */
	/* 前綴 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "CREATE TABLE if not exists %s(", szTableName);
	/*  "CREATE TABLE	szTableName("的長度 */
	inSqlLength += strlen(szSqlPrefix);
	
	
	/* 計算要分配的記憶體長度 */
	for (i = 0;; i ++)
	{
		/* 碰到Table底部 */
		if (strlen((char*)&TABLE_TICKET_TAG[i].szTag) == 0)
		{
			break;
		}
		
		/* 第一行前面不加逗號，其他都要 */
		if (i > 0)
		{
			inSqlLength += strlen(",");
		}
		
		/* Tag Name */
		inSqlLength += strlen((char*)&TABLE_TICKET_TAG[i].szTag);
		/* Tag 型別 */
		if (strlen((char*)&TABLE_TICKET_TAG[i].szType) > 0)
		{
			inSqlLength += strlen(" ");
			inSqlLength += strlen((char*)&TABLE_TICKET_TAG[i].szType);
		}
		/* Tag 屬性1 */
		if (strlen((char*)&TABLE_TICKET_TAG[i].szAttribute1) > 0)
		{
			inSqlLength += strlen(" ");
			inSqlLength += strlen((char*)&TABLE_TICKET_TAG[i].szAttribute1);
		}
		/* Tag 屬性2 */
		if (strlen((char*)&TABLE_TICKET_TAG[i].szAttribute2) > 0)
		{
			inSqlLength += strlen(" ");
			inSqlLength += strlen((char*)&TABLE_TICKET_TAG[i].szAttribute2);
		}
	}
	
	/* 後綴 */
	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, ");");
	inSqlLength += strlen(szSqlSuffix);
        
	/* inSqlLength: */
	if (ginDebug == VS_TRUE) 
        {
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", inSqlLength);
		inDISP_LogPrintfAt(AT,szDebugMsg);
	}
	/* 配置記憶體(一定要+1，超級重要，不然會overflow) */
	szCreateSql = malloc(inSqlLength + 1);
	memset(szCreateSql, 0x00, inSqlLength);
	
	/* 先丟前綴Table Name */
	strcat(szCreateSql, szSqlPrefix);
	
	/* table要哪些tag */
	for (i = 0 ;; i ++)
        {
		/* 碰到Table底部 */
		if (strlen((char*)&TABLE_TICKET_TAG[i].szTag) == 0)
		{
			break;
		}
		
		/* 第一行前面不加逗號，其他都要 */
		if (i > 0)
		{
			strcat(szCreateSql , ",");
		}
		
		/* Tag Name */
		strcat(szCreateSql, (char*)&TABLE_TICKET_TAG[i].szTag);
		/* Tag 型別 */
		if (strlen((char*)&TABLE_TICKET_TAG[i].szType) > 0)
		{
			strcat(szCreateSql, " ");
			strcat(szCreateSql, (char*)&TABLE_TICKET_TAG[i].szType);
		}
		/* Tag 屬性1 */
		if (strlen((char*)&TABLE_TICKET_TAG[i].szAttribute1) > 0)
		{
			strcat(szCreateSql, " ");
			strcat(szCreateSql, (char*)&TABLE_TICKET_TAG[i].szAttribute1);
		}
		/* Tag 屬性2 */
		if (strlen((char*)&TABLE_TICKET_TAG[i].szAttribute2) > 0)
		{
			strcat(szCreateSql, " ");
			strcat(szCreateSql, (char*)&TABLE_TICKET_TAG[i].szAttribute2);
		}
        }
	
	/* 後綴 */
	strcat(szCreateSql, szSqlSuffix);
	
	/* 建立 Table */
	inRetVal = sqlite3_exec(srDBConnection, szCreateSql, 0, 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Create Table ERROR Num:%d", inRetVal);
			inDISP_LogPrintfAt(AT,szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason:%s", szErrorMessage);
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
		
		free(szCreateSql);
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (sqlite3_close(srDBConnection) != SQLITE_OK)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintfAt(AT,szDebugMsg);
			}
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Create Table OK");
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
	}
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (sqlite3_close(srDBConnection) != SQLITE_OK)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	
	/* 釋放記憶體 */
	free(szCreateSql);
	
	if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintfAt(AT,"inNCCC_Ticket_CreateBatchTable_Ticket()_END");
                inDISP_LogPrintfAt(AT,"----------------------------------------");
        }
	
	return (VS_SUCCESS);
}


/*
Function        : inNCCC_Ticket_Insert_All_Batch
Date&Time   : 2022/6/14 上午 10:54
Describe        : Open Database檔 insert 票證 batch Record
 *  [新增電票悠遊卡功能] 加入此功能 [SAM] 
*/
int inNCCC_Ticket_Insert_All_Batch(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int			inBindingIndex = 1;	/* binding的index從1開始 */
	int			i;
	int			inRetVal;
	int			inSqlLength = 0;	/* 算組SQL語句的長度 */
	char			szDebugMsg[84 + 1];
	char			szSqlPrefix[100 + 1];	/* INSERT INTO	szTableName( */
	char			szSqlSuffix[20 + 1];	/* VALUES ( */
	char			szSqlSuffix2[10 + 1];	/* ); */
	char			szTemplate[100 + 1];	/* 因為最長有到711 */
	char			*szInsertSql;
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_ALL_TABLE	srAll;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"----------------------------------------");
		inDISP_LogPrintfAt(AT,"inNCCC_Ticket_Insert_All_Batch()_START");
	}
	
	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	//inRetVal = sqlite3_open_v2(gszTranDBPath, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);/*20200612 ask 儒勳*/
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
	}
	
	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inNCCC_Ticket_Table_Link_TRec(pobTran, &srAll, _LS_INSERT_);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 失敗");
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "INSERT INTO %s (", szTableName);
	
	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, ")VALUES (");
	
	memset(szSqlSuffix2, 0x00, sizeof(szSqlSuffix2));
	sprintf(szSqlSuffix2, ");");
	
	/* 算要配置多少記憶體 */
	inSqlLength += strlen(szSqlPrefix);
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"COUNT");
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "IntTag: %d", srAll.inIntNum);
		inDISP_LogPrintfAt(AT,szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Int64Tag: %d", srAll.inInt64tNum);
		inDISP_LogPrintfAt(AT,szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "CharTag: %d", srAll.inCharNum);
		inDISP_LogPrintfAt(AT,szDebugMsg);
	}
	
	for (i = 0; i < srAll.inIntNum; i++)
	{
		inSqlLength += strlen(srAll.srInt[i].szTag);
		/* Comma */
		inSqlLength += 2;;
	}
	
	for (i = 0; i < srAll.inInt64tNum; i++)
	{
		inSqlLength += strlen(srAll.srInt64t[i].szTag);
		/* Comma */
		inSqlLength += 2;;
	}
	
	for (i = 0; i < srAll.inCharNum; i++)
	{
		inSqlLength += strlen(srAll.srChar[i].szTag);
		/* Comma */
		inSqlLength += 2;;
	}
	
	/* 第一行最後面的) */
	inSqlLength ++;
	/* 第二行"VALUES ("的長度 */
	inSqlLength += strlen(szSqlSuffix);
	
	for (i = 0; i < srAll.inIntNum; i++)
	{
		/* 用問號 */
		inSqlLength += 1;
		/* Comma & Space */
		inSqlLength += 2;
	}
	
	for (i = 0; i < srAll.inInt64tNum; i++)
	{
		/* 用問號 */
		inSqlLength += 1;
		/* Comma & Space */
		inSqlLength += 2;
	}
	
	for (i = 0; i < srAll.inCharNum; i++)
	{
		/* 用問號 */
		inSqlLength += 1;
		/* Comma & Space & 兩個單引號 */
		inSqlLength += 2;
	}
	
	/* ); */
	inSqlLength += strlen(szSqlSuffix2);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", inSqlLength);
		inDISP_LogPrintfAt(AT,szDebugMsg);
	}
	
	/* 配置記憶體 */
	szInsertSql = malloc(inSqlLength + 100);
	memset(szInsertSql, 0x00, inSqlLength + 100);
	
	/* 先丟Table Name */
	strcat(szInsertSql, szSqlPrefix);
	
	/* 看上一個table是不是空的，有東西的話，第一項前面要加comma，但這已經是第一個table，所以放0 */
	if (0 > 0)
	{
		strcat(szInsertSql, ", ");
	}
	
	for (i = 0; i < srAll.inIntNum; i++)
	{
		if (i > 0)
		{
			strcat(szInsertSql, ", ");
		}
		strcat(szInsertSql, srAll.srInt[i].szTag);
	}
	
	/* 看上一個table是不是空的，有東西的話，第一項前面要加comma */
	if (srAll.inIntNum > 0)
	{
		strcat(szInsertSql, ", ");
	}
	
	for (i = 0; i < srAll.inInt64tNum; i++)
	{
		if (i > 0)
		{
			strcat(szInsertSql, ", ");
		}
		strcat(szInsertSql, srAll.srInt64t[i].szTag);
	}
	
	/* 看上一個table是不是空的，有東西的話，第一項前面要加comma */
	if (srAll.inInt64tNum > 0)
	{
		strcat(szInsertSql, ", ");
	}
	
	for (i = 0; i < srAll.inCharNum; i++)
	{
		
		if (i > 0)
		{
			strcat(szInsertSql, ", ");
		}
		strcat(szInsertSql, srAll.srChar[i].szTag);
	}
	
	/* 為了避免有空的table導致多塞, */
	if (memcmp((szInsertSql + strlen(szInsertSql) - 2), ", ", 2) == 0)
	{
		memset(szInsertSql + strlen(szInsertSql) - 2, 0x00, 2);
	}
	
	/* ")VALUES (" */
	strcat(szInsertSql, szSqlSuffix);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	
	for (i = 0; i < srAll.inIntNum; i++)
	{
		if (i == 0)
		{
			sprintf(szTemplate, "?");
		}
		else
		{
			sprintf(szTemplate, ", ?");
		}
		strcat(szInsertSql, szTemplate);
		/* 只清自己用過得長度 comma(1) + space(1) + ?(1) */
		memset(szTemplate, 0x00, 3);
	}
	
	/* 代表上一個table有東西，要加comma */
	if (srAll.inIntNum > 0)
	{
		strcat(szInsertSql, ", ");
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Int Insert OK");
		inDISP_LogPrintfAt(AT,szDebugMsg);
	}
	
	for (i = 0; i < srAll.inInt64tNum; i++)
	{
		if (i == 0)
		{
			sprintf(szTemplate, "?");
		}
		else
		{
			sprintf(szTemplate, ", ?");
		}
		strcat(szInsertSql, szTemplate);
		/* 只清自己用過得長度 comma(1) + space(1) + ?(1) */
		memset(szTemplate, 0x00, 3);
	}
	
	/* 代表上一個table有東西，要加comma */
	if (srAll.inInt64tNum > 0)
	{
		strcat(szInsertSql, ", ");
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Int64t Insert OK");
		inDISP_LogPrintfAt(AT,szDebugMsg);
	}
	
	for (i = 0; i < srAll.inCharNum; i++)
	{
		if (i == 0)
		{
			sprintf(szTemplate, "?");
		}
		else
		{
			sprintf(szTemplate, ", ?");
		}
		strcat(szInsertSql, szTemplate);
		/* 只清自己用過得長度 comma(1) + space(1) + ?(1) */
		memset(szTemplate, 0x00, 3);
	}
	
	/* 代表上一個table有東西，要加comma */
	if (srAll.inCharNum > 0)
	{
		strcat(szInsertSql, ", ");
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Char Insert OK");
		inDISP_LogPrintfAt(AT,szDebugMsg);
	}
	
	/* 為了避免有空的table導致多塞, */
	if (memcmp((szInsertSql + strlen(szInsertSql) - 2), ", ", 2) == 0)
	{
		memset(szInsertSql + strlen(szInsertSql) - 2, 0x00, 2);
	}
	
	/* 最後面的); */
	strcat(szInsertSql, szSqlSuffix2);
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szInsertSql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
		
	}
	
	/* Binding變數 */
	for (i = 0; i < srAll.inIntNum; i++)
	{
		inRetVal = sqlite3_bind_int(srSQLStat, inBindingIndex, *(int32_t*)srAll.srInt[i].pTagValue);
		if (inRetVal != SQLITE_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Binging Int Fail: %d", inRetVal);
				inDISP_LogPrintfAt(AT,szDebugMsg);
			}
		}
		else
		{
			inBindingIndex++;
		}
		
	}
	
	for (i = 0; i < srAll.inInt64tNum; i++)
	{
		inRetVal = sqlite3_bind_int(srSQLStat, inBindingIndex, *(int64_t*)srAll.srInt64t[i].pTagValue);
		if (inRetVal != SQLITE_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Binging Int64t Fail: %d", inRetVal);
				inDISP_LogPrintfAt(AT,szDebugMsg);
			}
		}
		else
		{
			inBindingIndex++;
		}
	}
	
	for (i = 0; i < srAll.inCharNum; i++)
	{
		inRetVal = sqlite3_bind_blob(srSQLStat, inBindingIndex, srAll.srChar[i].pCharVariable, srAll.srChar[i].inTagValueLen, NULL);
		if (inRetVal != SQLITE_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Binging Char Fail: %d", inRetVal);
				inDISP_LogPrintfAt(AT,szDebugMsg);
			}
		}
		else
		{
			inBindingIndex++;
		}
	}
	
	do
	{
		/* Insert */
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW	||
		    inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Insert OK");
				inDISP_LogPrintfAt(AT,szDebugMsg);
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Insert ERROR Num:%d", inRetVal);
				inDISP_LogPrintfAt(AT,szDebugMsg);
				
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
				inDISP_LogPrintfAt(AT,szDebugMsg);
				
				inDISP_LogPrintfAt(AT,szInsertSql);
			}

		}
		
	}while (inRetVal == SQLITE_ROW);
	
	/* 釋放事務，若要重用則用sqlite3_reset */
	sqlite3_finalize(srSQLStat);
	
	free(szInsertSql);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (sqlite3_close(srDBConnection) != SQLITE_OK)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
	}
	
	if (inRetVal == SQLITE_ERROR)
	{
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintfAt(AT,"inNCCC_Ticket_Insert_All_Batch()_END");
                inDISP_LogPrintfAt(AT,"----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        : inNCCC_Ticket_Table_Link_TRec
Date&Time   : 2022/6/14 上午 10:55
Describe        :
 * 將pobTran變數pointer位置放到Table中(用以解決每一個function都要放一個table的問題)
 * 這邊直接把pobTran的pointer直接指到srAll(之後可能要考慮給動態記憶體)，TagName因為是寫在這個Function內的Table，所以要給實體位置儲存
 * [新增電票悠遊卡功能] 加入此功能 [SAM] 
*/
int inNCCC_Ticket_Table_Link_TRec(TRANSACTION_OBJECT *pobTran, SQLITE_ALL_TABLE *srAll, int inLinkState)
{
	SQLITE_INT32T_TABLE TABLE_BATCH_INT[] = 
	{
		{0	,"inTableID"			,&pobTran->inTableID},	/* inTableID */
		{0	,"inCode"				,&pobTran->srTRec.inCode	},	
		{0	,"inPrintOption"			,&pobTran->srTRec.inPrintOption},	/* Print Option Flag (also in TCT) */
		{0	,"inTicketType"			,&pobTran->srTRec.inTicketType	},	/* 票證種類 - 交易或明細使用  */
		{0	,"inTDTIndex"			,&pobTran->srTRec.inTDTIndex},		/* 存TDT的index */
		{0	,"lnTxnAmount"			,&pobTran->srTRec.lnTxnAmount},	/* 交易金額 */
		{0	,"lnTopUpAmount"		,&pobTran->srTRec.lnTopUpAmount},	/* 加值金額(基底) */
		{0	,"lnTotalTopUpAmount"	,&pobTran->srTRec.lnTotalTopUpAmount},	/* 加值金額 */
		{0	,"lnCardRemainAmount"	,&pobTran->srTRec.lnCardRemainAmount},	/* 卡片餘額 */
		{0	,"lnInvNum"			,&pobTran->srTRec.lnInvNum},	/* For簽單使用，簽單序號 */
		{0	,"lnECCInvNum"			,&pobTran->srTRec.lnECCInvNum},
		{0	,"lnSTAN"				,&pobTran->srTRec.lnSTAN},	/* */
		{0	,"lnFinalBeforeAmt"		,&pobTran->srTRec.lnFinalBeforeAmt},/* 最後交易結構，交易前卡片餘額 */
		{0	,"lnFinalAfterAmt"		,&pobTran->srTRec.lnFinalAfterAmt},	/* 最後交易結構，交易後卡片餘額 */
		{0	,"lnMainInvNum"		,&pobTran->srTRec.lnMainInvNum},	/* Confirm use 電文序號，因電票中一筆交易可能有數筆電文，所以簽單序號和電文序號分開 */
		{0	,"lnCountInvNum"		,&pobTran->srTRec.lnCountInvNum},	/* Confirm use 若有advice，要預先跳過的電文序號 */
		{0	,"lnTicketCTN"			,&pobTran->srTRec.lnTicketCTN},	/* [新增電票悠遊卡功能]  電子票證卡片交易序號  for IPASS 2.0 但原本資料庫已有，不用新增 [SAM] 2022/6/14 上午 10:20 */
		{0	,""					,NULL}	/* 這行用Null用來知道尾端在哪 */
		
	};
	
	SQLITE_INT64T_TABLE TABLE_BATCH_INT64T[] = 
	{

		{0	,""				,NULL				}	/* 這行用Null用來知道尾端在哪 */
	};
	
	SQLITE_CHAR_TABLE TABLE_BATCH_CHAR[] =
	{

		{0	,"szUID"				,&pobTran->srTRec.szUID			,sizeof(pobTran->srTRec.szUID)	},
		{0	,"szDate"				,&pobTran->srTRec.szDate			,sizeof(pobTran->srTRec.szDate)			},	/* YYMMDD */
		{0	,"szOrgDate"			,&pobTran->srTRec.szOrgDate		,sizeof(pobTran->srTRec.szOrgDate)		},	/* YYMMDD */
		{0	,"szTime"				,&pobTran->srTRec.szTime			,sizeof(pobTran->srTRec.szTime)			},	/* HHMMSS */
		{0	,"szOrgTime"			,&pobTran->srTRec.szOrgTime		,sizeof(pobTran->srTRec.szOrgTime)		},	/* HHMMSS */
		{0	,"szAuthCode"			,&pobTran->srTRec.szAuthCode		,sizeof(pobTran->srTRec.szAuthCode)		},	/* */
		{0	,"szECCAuthCode"		,&pobTran->srTRec.szECCAuthCode	,sizeof(pobTran->srTRec.szECCAuthCode)	},
		{0	,"szRespCode"			,&pobTran->srTRec.szRespCode		,sizeof(pobTran->srTRec.szRespCode)		},	/* Response Code */
		{0	,"szProductCode"		,&pobTran->srTRec.szProductCode		,sizeof(pobTran->srTRec.szProductCode)		},	/* 產品代碼  */
		{0	,"szTicketRefundCode"	,&pobTran->srTRec.szTicketRefundCode	,sizeof(pobTran->srTRec.szTicketRefundCode)	},	/* 退貨序號  */
		{0	,"szTicketRefundDate"		,&pobTran->srTRec.szTicketRefundDate	,sizeof(pobTran->srTRec.szTicketRefundDate)	},	/* MMDD */
		{0	,"szStoreID"			,&pobTran->srTRec.szStoreID			,sizeof(pobTran->srTRec.szStoreID)		},	/* */
		{0	,"szRefNo"				,&pobTran->srTRec.szRefNo			,sizeof(pobTran->srTRec.szRefNo)		},	/* Reference Number(RRN) */	
		{0	,"szAwardNum"			,&pobTran->srTRec.szAwardNum		,sizeof(pobTran->srTRec.szAwardNum)	},	/* [新增電票悠遊卡功能]  優惠個數  [SAM] 2022/6/14 上午 10:39 */
		{0	,"szAwardSN"			,&pobTran->srTRec.szAwardSN			,sizeof(pobTran->srTRec.szAwardSN)		},	/* [新增電票悠遊卡功能]  [SAM] 優惠序號 2022/6/14 上午 10:40 */
		{0	,"uszAutoTopUpBit"		,&pobTran->srTRec.uszAutoTopUpBit		,sizeof(pobTran->srTRec.uszAutoTopUpBit)	},	/* 是否自動加值 */
		{0	,"uszBlackListBit"		,&pobTran->srTRec.uszBlackListBit		,sizeof(pobTran->srTRec.uszBlackListBit)	},	/* 是否在黑名單中 */
		{0	,"uszOfflineBit"			,&pobTran->srTRec.uszOfflineBit			,sizeof(pobTran->srTRec.uszOfflineBit)	},	/* [新增電票悠遊卡功能] 離線交易  [SAM] 2022/6/14 上午 10:42*/
		{0	,"uszTicketConnectBit"	,&pobTran->srTRec.uszTicketConnectBit	,sizeof(pobTran->srTRec.uszTicketConnectBit)	},	/* 是否連線中 */
		{0	,"uszResponseBit"		,&pobTran->srTRec.uszResponseBit		,sizeof(pobTran->srTRec.uszResponseBit)		},	/* 票值回覆用 */
		{0	,"uszCloseAutoADDBit"	,&pobTran->srTRec.uszCloseAutoTopUpBit	,sizeof(pobTran->srTRec.uszCloseAutoTopUpBit)	},	/* 關閉自動加值用 */
		{0	,"uszStopPollBit"			,&pobTran->srTRec.uszStopPollBit	,sizeof(pobTran->srTRec.uszStopPollBit)		},	/* Mifare Stop */
		{0	,"uszConfirmBit"			,&pobTran->srTRec.uszConfirmBit	,sizeof(pobTran->srTRec.uszConfirmBit)		},	/* IPASS Confirm Inv use */
		{0	,"uszESVCTransBit"		,&pobTran->srTRec.uszESVCTransBit	,sizeof(pobTran->srTRec.uszESVCTransBit)	},	/* 代表是電票交易 */
		{0	,"srIPASSRec"			,&pobTran->srTRec.srIPASSRec		,sizeof(pobTran->srTRec.srIPASSRec)		},	/* IPASS結構，求快先整個存 */
		{0	,"srECCRec"			,&pobTran->srTRec.srECCRec		,sizeof(pobTran->srTRec.srECCRec)		},	/* ECC結構，求快先整個存 */
		{0	,"srICASHRec"			,&pobTran->srTRec.srICASHRec		,sizeof(pobTran->srTRec.srICASHRec)		},	/* ICASH結構，求快先整個存 */
		{0	,"srHAPPYCASHRec"		,&pobTran->srTRec.srHAPPYCASHRec	,sizeof(pobTran->srTRec.srHAPPYCASHRec)	},	/* HAPPYCASH結構，求快先整個存 */
		{0	,""				,NULL					,0						}	/* 這行用Null用來知道尾端在哪 */
	};
	
	int	i;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"----------------------------------------");
		inDISP_LogPrintfAt(AT,"inNCCC_Ticket_Table_Link_TRec()_START");
	}
	
	
	for (i = 0;; i++)
	{
		/* 碰到Table底部，設定Tag數並跳出 */
		if (strlen(TABLE_BATCH_INT[i].szTag) == 0)
		{
			break;
		}
		
		/* pointer為空，則跳過 */
		if (TABLE_BATCH_INT[i].pTagValue == NULL)
			continue;
		
		/* 變數多過於原來設定的Tag數 */
		if (srAll->inIntNum == _TAG_INT_MAX_NUM_)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Int變數過多");
				inDISP_LogPrintfAt(AT,szDebugMsg);
			}
			return (VS_ERROR);
		}
		
		/* Tag 名稱過長 */
		if (strlen(TABLE_BATCH_INT[i].szTag) > _TAG_MAX_LENGRH_)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Tag 名稱過長");
				inDISP_LogPrintfAt(AT,szDebugMsg);
			}
			return (VS_ERROR);
		}
		
		/* Insert時不用塞TableID */
		if (inLinkState == _LS_INSERT_)
		{
			/* 判斷長度是因為避免相同字首比對錯誤 */
			if ((memcmp(TABLE_BATCH_INT[i].szTag, "inTableID", strlen("inTableID")) == 0) && (strlen(TABLE_BATCH_INT[i].szTag) == strlen("inTableID")))
			{
				continue;
			}
		}
		
		strcat(srAll->srInt[srAll->inIntNum].szTag, TABLE_BATCH_INT[i].szTag);
		srAll->srInt[srAll->inIntNum].pTagValue = TABLE_BATCH_INT[i].pTagValue;
		srAll->inIntNum++;
	}
	
	for (i = 0;; i++)
	{
		/* 碰到Table底部，設定Tag數並跳出 */
		if (strlen(TABLE_BATCH_INT64T[i].szTag) == 0)
		{
			break;
		}
		
		/* pointer為空，則跳過 */
		if (TABLE_BATCH_INT64T[i].pTagValue == NULL)
			continue;
		
		/* 變數多過於原來設定的Tag數 */
		if (srAll->inInt64tNum == _TAG_INT64T_MAX_NUM_)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "INT64T變數過多");
				inDISP_LogPrintfAt(AT,szDebugMsg);
			}
			return (VS_ERROR);
		}
		
		/* Tag 名稱過長 */
		if (strlen(TABLE_BATCH_INT64T[i].szTag) > _TAG_MAX_LENGRH_)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Tag 名稱過長");
				inDISP_LogPrintfAt(AT,szDebugMsg);
			}
			return (VS_ERROR);
		}
		
		strcat(srAll->srInt64t[srAll->inInt64tNum].szTag, TABLE_BATCH_INT64T[i].szTag);
		srAll->srInt64t[srAll->inInt64tNum].pTagValue = TABLE_BATCH_INT64T[i].pTagValue;
		srAll->inInt64tNum++;
	}
	
	for (i = 0;; i++)
	{
		/* 碰到Table底部，設定Tag數並跳出 */
		if (strlen(TABLE_BATCH_CHAR[i].szTag) == 0)
		{
			break;
		}
		
		/* pointer為空，則跳過 */
		if (TABLE_BATCH_CHAR[i].pCharVariable == NULL)
			continue;
		
		/* 變數多過於原來設定的Tag數 */
		if (srAll->inCharNum == _TAG_CHAR_MAX_NUM_)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Char變數過多");
				inDISP_LogPrintfAt(AT,szDebugMsg);
			}
			return (VS_ERROR);
		}
		
		/* Tag 名稱過長 */
		if (strlen(TABLE_BATCH_CHAR[i].szTag) > _TAG_MAX_LENGRH_)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Tag 名稱過長");
				inDISP_LogPrintfAt(AT,szDebugMsg);
			}
			return (VS_ERROR);
		}
		
		strcat(srAll->srChar[srAll->inCharNum].szTag, TABLE_BATCH_CHAR[i].szTag);
		srAll->srChar[srAll->inCharNum].pCharVariable = TABLE_BATCH_CHAR[i].pCharVariable;
		srAll->srChar[srAll->inCharNum].inTagValueLen = TABLE_BATCH_CHAR[i].inTagValueLen;
		srAll->inCharNum++;
	}
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inNCCC_Ticket_Table_Link_TRec()_END");
		inDISP_LogPrintfAt(AT,"----------------------------------------");
	}
	
	return (VS_SUCCESS);
}



/*
Function        :inNCCC_Ticket_Insert_Advice_Ticket_Record
Date&Time       :2018/1/24 下午 12:10
Describe        :
*/
int inNCCC_Ticket_Insert_Advice_Ticket_Record(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
		
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"----------------------------------------");
		inDISP_LogPrintfAt(AT,"inNCCC_Ticket_Insert_Advice_Ticket_Record() START !");
	}
	
	inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_TICKET_ADVICE_);
	if (inRetVal != VS_SUCCESS)
	{
		inFunc_EDCLock();
		return (inRetVal);
	}
	
	inRetVal = inBATCH_Insert_All_Flow(pobTran, _TN_BATCH_TABLE_TICKET_ADVICE_);
	if (inRetVal != VS_SUCCESS)
	{
		inFunc_EDCLock();
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inNCCC_Ticket_Insert_Advice_Ticket_Record() END !");
		inDISP_LogPrintfAt(AT,"----------------------------------------");
	}
	
	return (inRetVal);
}


/*
Function        :inNCCC_Ticket_ESVC_Get_TRec_Top_Flow
Date&Time       :2017/3/14 下午 3:49
Describe        :
*/
int inNCCC_Ticket_ESVC_Get_TRec_Top_Flow(TRANSACTION_OBJECT *pobTran, int inTableType)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ADVICE_SUFFIX_);
			break;
		default :
			break;
	}
	
	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_EMV_TABLE_:
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			inRetVal = inNCCC_Ticket_ESVC_Get_TRec_Top(pobTran, szTableName);
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}
/*
Function        :inNCCC_Ticket_ESVC_Get_TRec_Top
Date&Time       :2018/1/26 下午 2:36
Describe        :ADVICE會取table中Primary Key最小的值
*/
int inNCCC_Ticket_ESVC_Get_TRec_Top(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int			j = 0;
	int			inIntIndex = 0, inInt64tIndex = 0, inCharIndex = 0;
	int			inCols = 0, inDataLen = 0;
	int			inRetVal = VS_SUCCESS;
	int			inFind = VS_FALSE;
	char			szDebugMsg[128 + 1];
	char			szQuerySql[300 + 1];	/* INSERT INTO	szTableName( */
	char			szTagName[_TAG_WIDTH_ + 1];
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_ALL_TABLE	srAll;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"----------------------------------------");
		inDISP_LogPrintfAt(AT,"inNCCC_Ticket_ESVC_Get_TRec_Top()_START");
	}
	
	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
//	inRetVal = sqlite3_open_v2(gszTranDBPath, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);/*20200612 ask 儒勳*/
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
	}
	
	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inNCCC_Ticket_Table_Link_TRec(pobTran, &srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 失敗");
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "SELECT * FROM %s WHERE (inTableID = (SELECT MIN(inTableID) FROM %s))", szTableName, szTableName);

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
		
	}
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
		/* 替換資料前先清空srTRec */
		memset(&pobTran->srTRec, 0x00, sizeof(pobTran->srTRec));
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
		return (VS_NO_RECORD);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
			inDISP_LogPrintfAt(AT,szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
			inDISP_LogPrintfAt(AT,szDebugMsg);

			inDISP_LogPrintfAt(AT,szQuerySql);
		}

		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (sqlite3_close(srDBConnection) != SQLITE_OK)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintfAt(AT,szDebugMsg);
			}
		}

		return (VS_ERROR);
	}
	
	inCols = sqlite3_column_count(srSQLStat);
	
	/* binding 取得值 */
	for (j = 0; j < inCols; j++)
	{
		inFind = VS_FALSE;
		memset(szTagName, 0x00, sizeof(szTagName));
		strcat(szTagName, sqlite3_column_name(srSQLStat, j));
		
		
		for (inIntIndex = 0; inIntIndex < srAll.inIntNum; inIntIndex++)
		{
			if (srAll.srInt[inIntIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}
			
			
			/* 比對Tag Name */
			if (memcmp(szTagName, srAll.srInt[inIntIndex].szTag, strlen(srAll.srInt[inIntIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srInt[inIntIndex].szTag))
			{
				*(int32_t*)srAll.srInt[inIntIndex].pTagValue = sqlite3_column_int(srSQLStat, j);
				srAll.srInt[inIntIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}
		
		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inInt64tIndex = 0; inInt64tIndex < srAll.inInt64tNum; inInt64tIndex++)
		{
			if (srAll.srInt64t[inInt64tIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll.srInt64t[inInt64tIndex].szTag, strlen(srAll.srInt64t[inInt64tIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srInt64t[inInt64tIndex].szTag))
			{
				*(int64_t*)srAll.srInt64t[inInt64tIndex].pTagValue = sqlite3_column_int64(srSQLStat, j);
				srAll.srInt64t[inInt64tIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}
			
		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}
		
		for (inCharIndex = 0; inCharIndex < srAll.inCharNum; inCharIndex++)
		{
			if (srAll.srChar[inCharIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}
			
			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll.srChar[inCharIndex].szTag, strlen(srAll.srChar[inCharIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srChar[inCharIndex].szTag))
			{
				inDataLen = sqlite3_column_bytes(srSQLStat, j);
				memcpy(srAll.srChar[inCharIndex].pCharVariable, sqlite3_column_blob(srSQLStat, j), inDataLen);
				srAll.srChar[inCharIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}
		
		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Find no variable to insert:Tag: %s", szTagName);
			inDISP_LogPrintfAt(AT,szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Value: %s", (char*)sqlite3_column_blob(srSQLStat, j));
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
		
	}
	
	/* 釋放事務 */
	sqlite3_finalize(srSQLStat);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (sqlite3_close(srDBConnection) != SQLITE_OK)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintfAt(AT,"inNCCC_Ticket_ESVC_Get_TRec_Top()_END");
                inDISP_LogPrintfAt(AT,"----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_ESVC_Delete_TRec_Top_Flow
Date&Time       :2017/3/14 下午 3:57
Describe        :
*/
int inNCCC_Ticket_ESVC_Delete_TRec_Top_Flow(TRANSACTION_OBJECT *pobTran, int inTableType)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ADVICE_SUFFIX_);
			break;
		default :
			break;
	}
	
	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_EMV_TABLE_:
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			inRetVal = inNCCC_Ticket_ESVC_Delete_TRec_Top(pobTran, szTableName);
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_Ticket_ESVC_Delete_TRec_Top
Date&Time       :2018/1/26 下午 3:04
Describe        :adviec刪除table id最小的那一筆
*/
int inNCCC_Ticket_ESVC_Delete_TRec_Top(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int		inRetVal;
	char		szDebugMsg[84 + 1];
	char		szQuerySql[300 + 1];	/* INSERT INTO	szTableName( */
	char		*szErrorMessage = NULL;
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"----------------------------------------");
		inDISP_LogPrintfAt(AT,"inNCCC_Ticket_ESVC_Delete_TRec_Top()_START");
	}
	
	 inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
//	inRetVal = sqlite3_open_v2(gszTranDBPath, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);/*20200612 ask 儒勳*/
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "DELETE FROM %s WHERE inTableID = (SELECT MIN(inTableID) FROM %s)", szTableName, szTableName);
	
	/* 取得 database 裡所有的資料 */
	inRetVal = sqlite3_exec(srDBConnection , szQuerySql, 0 , 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Delete Record ERROR Num:%d, ", inRetVal);
			inDISP_LogPrintfAt(AT,szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason:%s", szErrorMessage);
			inDISP_LogPrintfAt(AT,szDebugMsg);
			
			inDISP_LogPrintfAt(AT,szQuerySql);
		}
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (sqlite3_close(srDBConnection) != SQLITE_OK)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintfAt(AT,szDebugMsg);
			}
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Delete Record OK");
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
		
	}
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (sqlite3_close(srDBConnection) != SQLITE_OK)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintfAt(AT,"inNCCC_Ticket_ESVC_Delete_TRec_Top()_END");
                inDISP_LogPrintfAt(AT,"----------------------------------------");
        }

	return (VS_SUCCESS);
}



/*
Function        :inNCCC_Ticket_ESVC_Get_Batch_ByInvNum
Date&Time       :2018/1/30 下午 6:34
Describe        :利用調閱標號來將該筆資料全塞回pobTran中的TRec、會取最新狀態(如取消、調帳)
*/
int inNCCC_Ticket_ESVC_Get_Batch_ByInvNum(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber)
{
	int			j = 0;
	int			inIntIndex = 0, inInt64tIndex = 0, inCharIndex = 0;
	int			inCols = 0, inDataLen = 0;
	int			inRetVal = VS_SUCCESS;
	int			inFind = VS_FALSE;
	char			szDebugMsg[128 + 1];
	char			szQuerySql[200 + 1];	/* INSERT INTO	szTableName( */
	char			szTagName[_TAG_WIDTH_ + 1];
	char			szErrorMessage[100 + 1];
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_ALL_TABLE	srAll;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"----------------------------------------");
		inDISP_LogPrintfAt(AT,"inNCCC_Ticket_ESVC_Get_Batch_ByInvNum()_START");
	}
	
	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
//	inRetVal = sqlite3_open_v2(gszTranDBPath, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);/*20200612 ask 儒勳*/
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
	}
	
	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inNCCC_Ticket_Table_Link_TRec(pobTran, &srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 失敗");
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	if (inInvoiceNumber > 0)
	{
		sprintf(szQuerySql, "SELECT * FROM %s WHERE lnInvNum = %d ORDER BY inTableID DESC LIMIT 1", szTableName, inInvoiceNumber);
	}
	else if (inInvoiceNumber == _BATCH_LAST_RECORD_)
	{
		/* sqlite3_last_insert_rowid 只有在同一connection才有用 所以這邊的邏輯是最後一筆理論上invoiceNumber會最大 
		   若是同一筆，可能有調帳等操作，加上用max(inTableID)來判斷 */

		
		/* 再重新組查詢語句，把剛剛查到的invoiceNumber放進去 第一列第0行是所查的值 */
		memset(szQuerySql, 0x00, sizeof(szQuerySql));
		sprintf(szQuerySql, "SELECT * FROM %s WHERE lnInvNum = (SELECT MAX(lnInvNum) FROM %s) ORDER BY inTableID DESC LIMIT 1", szTableName, szTableName);
	}
	else
	{
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (sqlite3_close(srDBConnection) != SQLITE_OK)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintfAt(AT,szDebugMsg);
			}
		}
	
		return (VS_ERROR);
	}
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
	}
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
		/* 替換資料前先清空srTRec */
		memset(&pobTran->srTRec, 0x00, sizeof(pobTran->srTRec));
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
		return (VS_NO_RECORD);
	}
	else
	{
		memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
		strcpy(szErrorMessage, sqlite3_errmsg(srDBConnection));
			
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
			inDISP_LogPrintfAt(AT,szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", szErrorMessage);
			inDISP_LogPrintfAt(AT,szDebugMsg);

			inDISP_LogPrintfAt(AT,szQuerySql);
		}

		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (sqlite3_close(srDBConnection) != SQLITE_OK)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintfAt(AT,szDebugMsg);
			}
		}

		/* 因為直接get 不存在的table回傳值是-1，只有在Error Msg才能得知錯誤原因 */
		if (memcmp(szErrorMessage, "no such table", strlen("no such table")) == 0)
		{
			return (VS_NO_RECORD);
		}
		else
		{
			return (VS_ERROR);
		}
	}
	
	inCols = sqlite3_column_count(srSQLStat);
	
	/* binding 取得值 */
	for (j = 0; j < inCols; j++)
	{
		inFind = VS_FALSE;
		memset(szTagName, 0x00, sizeof(szTagName));
		strcat(szTagName, sqlite3_column_name(srSQLStat, j));
		
		
		for (inIntIndex = 0; inIntIndex < srAll.inIntNum; inIntIndex++)
		{
			if (srAll.srInt[inIntIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}
			
			
			/* 比對Tag Name */
			if (memcmp(szTagName, srAll.srInt[inIntIndex].szTag, strlen(srAll.srInt[inIntIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srInt[inIntIndex].szTag))
			{
				*(int32_t*)srAll.srInt[inIntIndex].pTagValue = sqlite3_column_int(srSQLStat, j);
				srAll.srInt[inIntIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}
		
		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inInt64tIndex = 0; inInt64tIndex < srAll.inInt64tNum; inInt64tIndex++)
		{
			if (srAll.srInt64t[inInt64tIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll.srInt64t[inInt64tIndex].szTag, strlen(srAll.srInt64t[inInt64tIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srInt64t[inInt64tIndex].szTag))
			{
				*(int64_t*)srAll.srInt64t[inInt64tIndex].pTagValue = sqlite3_column_int64(srSQLStat, j);
				srAll.srInt64t[inInt64tIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}
			
		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}
		
		for (inCharIndex = 0; inCharIndex < srAll.inCharNum; inCharIndex++)
		{
			if (srAll.srChar[inCharIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}
			
			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll.srChar[inCharIndex].szTag, strlen(srAll.srChar[inCharIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srChar[inCharIndex].szTag))
			{
				inDataLen = sqlite3_column_bytes(srSQLStat, j);
				memcpy(srAll.srChar[inCharIndex].pCharVariable, sqlite3_column_blob(srSQLStat, j), inDataLen);
				srAll.srChar[inCharIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}
		
		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Find no variable to insert:Tag: %s", szTagName);
			inDISP_LogPrintfAt(AT,szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Value: %s", (char*)sqlite3_column_blob(srSQLStat, j));
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
		
	}
	
	/* 釋放事務 */
	sqlite3_finalize(srSQLStat);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (sqlite3_close(srDBConnection) != SQLITE_OK)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintfAt(AT,"inNCCC_Ticket_ESVC_Get_Batch_ByInvNum()_END");
                inDISP_LogPrintfAt(AT,"----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_Get_TRec_ByCnt_Enormous_Search
Date&Time       :2018/1/31 下午 2:28
Describe        :
*/
int inNCCC_Ticket_Get_TRec_ByCnt_Enormous_Search(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int	inRetVal;
	char	szDebugMsg[128 + 1];
	char	szQuerySql[200 + 1];	/* INSERT INTO	szTableName( */
	//sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
    
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"----------------------------------------");
		inDISP_LogPrintfAt(AT,"inNCCC_Ticket_Get_TRec_ByCnt_Enormous_Search()_START");
	}
	
	 inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &gsrDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
//	inRetVal = sqlite3_open_v2(gszTranDBPath, &gsrDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);/*20200612 ask 儒勳*/
//	inRetVal = sqlite3_open_v2(gszTranDBPath, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
	}
	
	/* 塞入SQL語句 */
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "SELECT * FROM %s WHERE (uszUpdated = 0)ORDER BY lnInvNum ASC", szTableName);
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(gsrDBConnection, szQuerySql, -1, &gsrSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
		
	}
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inNCCC_Ticket_Get_TRec_ByCnt_Enormous_Search()_END");
		inDISP_LogPrintfAt(AT,"----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_Get_TRec_ByCnt_Enormous_Get
Date&Time       :2018/1/31 下午 2:29
Describe        :
*/
int inNCCC_Ticket_Get_TRec_ByCnt_Enormous_Get(TRANSACTION_OBJECT *pobTran, char* szTableName, int inRecCnt)
{
	int			inRetVal = 0;
	int			i = 0, j = 0;
	int			inIntIndex = 0, inInt64tIndex = 0, inCharIndex = 0;
	int			inCols = 0, inDataLen = 0;
	int			inFind = VS_FALSE;
	char			szDebugMsg[100 + 1] = {0};
	char			szTagName[_TAG_WIDTH_ + 1] = {0};
	SQLITE_ALL_TABLE	srAll;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"----------------------------------------");
		inDISP_LogPrintfAt(AT,"inNCCC_Ticket_Get_TRec_ByCnt_Enormous_Get() START !");
	}
	
	/* 重置前一次結果 */
	sqlite3_reset(gsrSQLStat);
	
	/* 取得 database 裡所有的資料 */
	for (i = 0; i <= inRecCnt; i++)
	{
		/* Qerry */
		inRetVal = sqlite3_step(gsrSQLStat);
		if (inRetVal == SQLITE_ROW)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Get Table OK");
				inDISP_LogPrintfAt(AT,szDebugMsg);
			}
		}
		else if (inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "NO DATA");
				inDISP_LogPrintfAt(AT,szDebugMsg);
			}
			return (VS_NO_RECORD);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
				inDISP_LogPrintfAt(AT,szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
				inDISP_LogPrintfAt(AT,szDebugMsg);
			}

			/* 關閉 database, close null pointer 是NOP(No Operation) */
			if (sqlite3_close(gsrDBConnection) != SQLITE_OK)
			{
				/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
				return (VS_ERROR);
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "Close Database OK");
					inDISP_LogPrintfAt(AT,szDebugMsg);
				}
			}

			return (VS_ERROR);
		}
	}
	
	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inNCCC_Ticket_Table_Link_TRec(pobTran, &srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 失敗");
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	
	/* 替換資料前先清空srTRec */
	memset(&pobTran->srTRec, 0x00, sizeof(pobTran->srTRec));
	
	inCols = sqlite3_column_count(gsrSQLStat);
	
	/* binding 取得值 */
	for (j = 0; j < inCols; j++)
	{
		inFind = VS_FALSE;
		memset(szTagName, 0x00, sizeof(szTagName));
		strcat(szTagName, sqlite3_column_name(gsrSQLStat, j));
		
		
		for (inIntIndex = 0; inIntIndex < srAll.inIntNum; inIntIndex++)
		{
			if (srAll.srInt[inIntIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}
			
			
			/* 比對Tag Name */
			if (memcmp(szTagName, srAll.srInt[inIntIndex].szTag, strlen(srAll.srInt[inIntIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srInt[inIntIndex].szTag))
			{
				*(int32_t*)srAll.srInt[inIntIndex].pTagValue = sqlite3_column_int(gsrSQLStat, j);
				srAll.srInt[inIntIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}
		
		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inInt64tIndex = 0; inInt64tIndex < srAll.inInt64tNum; inInt64tIndex++)
		{
			if (srAll.srInt64t[inInt64tIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll.srInt64t[inInt64tIndex].szTag, strlen(srAll.srInt64t[inInt64tIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srInt64t[inInt64tIndex].szTag))
			{
				*(int64_t*)srAll.srInt64t[inInt64tIndex].pTagValue = sqlite3_column_int64(gsrSQLStat, j);
				srAll.srInt64t[inInt64tIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}
			
		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}
		
		for (inCharIndex = 0; inCharIndex < srAll.inCharNum; inCharIndex++)
		{
			if (srAll.srChar[inCharIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}
			
			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll.srChar[inCharIndex].szTag, strlen(srAll.srChar[inCharIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srChar[inCharIndex].szTag))
			{
				inDataLen = sqlite3_column_bytes(gsrSQLStat, j);
				memcpy(srAll.srChar[inCharIndex].pCharVariable, sqlite3_column_blob(gsrSQLStat, j), inDataLen);
				srAll.srChar[inCharIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}
		
		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Find no variable to insert:Tag: %s", szTagName);
			inDISP_LogPrintfAt(AT,szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Value: %s", (char*)sqlite3_column_blob(gsrSQLStat, j));
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
		
	}
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inNCCC_Ticket_Get_TRec_ByCnt_Enormous_Get() END !");
		inDISP_LogPrintfAt(AT,"----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_Get_Void_Top_Up_Amount_From_Batch
Date&Time       :2018/7/18 下午 4:32
Describe        :
*/
int inNCCC_Ticket_Get_Void_Top_Up_Amount_From_Batch(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = VS_ERROR;
	TRANSACTION_OBJECT	pobTranTemp;
	
	memset(&pobTranTemp, 0x00, sizeof(pobTranTemp));
	/* 先抓上一筆 */
	pobTranTemp.srBRec.inHDTIndex = pobTran->srBRec.inHDTIndex;
	pobTranTemp.srBRec.lnOrgInvNum = _BATCH_LAST_RECORD_;
	inRetVal = inBATCH_GetTransRecord_By_Sqlite_ESVC(&pobTranTemp);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                /* 顯示無交易紀錄 */
		inDISP_Msg_BMP(_ERR_RECORD_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, 2, "", 0);
		
		return (VS_ERROR);
	}
	
	/* 抓成功就直接塞金額 */
	/* 這邊流程在_TICKET_DECIDE_TRANS_TYPE_後，所以直接塞」TRec */
	pobTran->srTRec.lnTxnAmount = pobTranTemp.srTRec.lnTxnAmount;
	/* 這邊要多抓AuthCode原加值時回傳的AuthCode */
	memcpy(pobTran->srTRec.szAuthCode, pobTranTemp.srTRec.szAuthCode, strlen(pobTranTemp.srTRec.szAuthCode));
	
	return (VS_SUCCESS);
}

/*----------------------------從NCCC移植過來-----------------------------------------------------*/

/*
Function        :inNCCC_Ticket_Top_Up_Amount_Check
Date&Time       :2018/1/15 下午 6:00
Describe        :檢核是否可以自動加值
*/
int inNCCC_Ticket_Top_Up_Amount_Check(TRANSACTION_OBJECT *pobTran)
{
	long    lnAddAmt = 0;
	long    lnAmount, lnSubAmount, lnDeductAmount;

	lnAmount = pobTran->srTRec.lnCardRemainAmount;		/* 餘額 */
	lnSubAmount = pobTran->srTRec.lnTopUpAmount;		/* 自動加值金額 */
	lnDeductAmount = pobTran->srTRec.lnTxnAmount;		/* 扣款金額 */

	while(1)
	{
		lnAmount += lnSubAmount;
		lnAddAmt += lnSubAmount;
		
		/* 防呆 */
		/* 卡內金額不得高於10000 */
		if (lnAmount > 10000)
			return (VS_ERROR);

		if (lnAmount >= lnDeductAmount)
		{
			if (lnAddAmt > 1500)
			{
				/* 自動加值最多1000 *//* 20211111,修改為最高1500*/
				return (VS_ERROR);
			}
			else
			{
				pobTran->srTRec.lnTotalTopUpAmount = lnAddAmt;
				return (VS_SUCCESS);
			}
		}
	}
}

/*
Function        : inNCCC_Ticket_Func_MakeRefNo
Date&Time   : 2022/6/22 上午 9:37
Describe        :
 *  [新增電票悠遊卡功能]  [SAM] 
*/
int inNCCC_Ticket_Func_MakeRefNo(TRANSACTION_OBJECT *pobTran)
{
	char	szBatchNum[6 + 1];
	char	szOrgInvoiceNum[6 + 1];
	char	szTerminalID[8 + 1];
	
	memset(pobTran->srTRec.szRefNo, 0x00, sizeof(pobTran->srTRec.szRefNo));
	strcpy(pobTran->srTRec.szRefNo, "9");
	/* Terminal ID */
	memset(szTerminalID, 0x00, sizeof(szTerminalID));
	inGetTerminalID(szTerminalID);
	memcpy(&pobTran->srTRec.szRefNo[1], &szTerminalID[3], 5);
	/* Batch Number */;
	memset(szBatchNum, 0x00, sizeof(szBatchNum));
	inGetBatchNum(szBatchNum);
	memcpy(&pobTran->srTRec.szRefNo[6], &szBatchNum[4], 2);
	/* Invoice Number */
	memset(szOrgInvoiceNum, 0x00, sizeof(szOrgInvoiceNum));
	inGetInvoiceNum(szOrgInvoiceNum);
	memcpy(&pobTran->srTRec.szRefNo[8], &szOrgInvoiceNum[3], 3);
	/* 補空白 */
	inFunc_PAD_ASCII(pobTran->srTRec.szRefNo, pobTran->srTRec.szRefNo, ' ', 12, _PADDING_RIGHT_);
	
	return (VS_SUCCESS);
}

