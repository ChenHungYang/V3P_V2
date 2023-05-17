#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>
#include <ctos_qrcode.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/PRINT/PrtMsg.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FILE_FUNC/File.h"
#include "../SOURCE/COMM/Comm.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Batch.h"
#include "../SOURCE/FUNCTION/ASMC.h"
#include "../SOURCE/FUNCTION/Card.h"
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/CPT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/EST.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/SCDT.h"
#include "../SOURCE/FUNCTION/Signpad.h"
#include "../SOURCE/FUNCTION/PWD.h"
#include "../SOURCE/FUNCTION/PCD.h"
#include "../SOURCE/FUNCTION/IPASSDT.h"
#include "../SOURCE/FUNCTION/TDT.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/EVENT/MenuMsg.h"
//#include "../NCCC/NCCCsrc.h"
//#include "../NCCC/NCCCdcc.h"
//#include "../NCCC/NCCCloyalty.h"
//#include "../NCCC/NCCCtms.h"
//#include "../NCCC/NCCCtmsCPT.h"
//#include "../NCCC/NCCCtmsSCT.h"
#include "../NCCC/NCCCTicketSrc.h"
//#include "../NCCC/NCCCesc.h"
//#include "../FISC/NCCCfisc.h"
#include "../CTLS/CTLS.h"
//#include "../HG/HGsrc.h"
//#include "../CREDIT/Creditfunc.h"
//#include "../CREDIT/CreditprtByBuffer.h"  // 2019/7/3 下午 2:43  Hachi
#include "CMASprtByBuffer.h"

extern  int	ginDebug;			/* Debug使用 extern */
extern char	gszTermVersionID[15 + 1];
extern BMPHeight	gsrBMPHeight;			/* 圖片高度 */
extern int	inPrinttype_ByBuffer;        /* 0 = 橫式，1 = 直式 */
extern char	gszTermVersionDate[16 + 1];
extern int ginMachineType;


/* 列印電票帳單使用(START) */
PRINT_CMAS_RECEIPT_TYPE_TABLE_BYBUFFER srReceiptType_CMAS_ByBuffer[] = /* srReceiptType_ByBuffer*/
{		      	
	/* 悠遊卡簽單 */ 
	{
		inCMAS_PRINT_Logo_ByBuffer,
		inCMAS_PRINT_Tidmid_ByBuffer,
		inCMAS_PRINT_Data_ByBuffer, 
		inCMAS_PRINT_Amount_ByBuffer, 
		_NULL_CH_,
		inCMAS_PRINT_ReceiptEND_ByBuffer
	},
};

/* 列印電票總額報表使用 (START) */
TOTAL_CMAS_REPORT_TABLE_BYBUFFER srTotalReport_ByBuffer_CMAS[] =
{	
	/* CMAS */
	{
		_NULL_CH_,
		inCMAS_PRINT_Logo_ByBuffer,
		inCMAS_PRINT_Top_SETTLE_ByBuffer,
		inCMAS_PRINT_TotalAmount_ByBuffer_Settle,
		_NULL_CH_,							
		_NULL_CH_,
		inCMAS_PRINT_End_ByBuffer
	},
};

/* 列印電票總額報表使用 (END) */

/* 列印電票明細報表使用 (START) */
DETAIL_CMAS_REPORT_TABLE_BYBUFFER srDetailReport_CMAS_ByBuffer[] = /* srDetailReport_CMAS_ByBuffer */
{
	{
		inCMAS_PRINT_Check_ByBuffer,                    /* inReportCheck */
		inCMAS_PRINT_Logo_ByBuffer,                     /* inReportLogo */
		inCMAS_PRINT_DetailReportTop_ByBuffer,          /* inReportTop */
		_NULL_CH_,                                      /* inTotalAmount */
		inCMAS_PRINT_DetailReportMiddle_ByBuffer,       /* inMiddle */
		inCMAS_PRINT_DetailReportBottom_ByBuffer,       /* inBottom */
		inCMAS_PRINT_End_ByBuffer                       /* inReportEnd */
	},
};
/* 列印電票明細報表使用 (END) */

/*-----------------流程-----------------------*/
/*
Function        :inCMAS_PRINT_Receipt_ByBuffer
Date&Time       :2020/6/20 上午 15:20
Describe        :列印悠遊卡簽單流程
Author          :Hachi
*/
int inCMAS_PRINT_Receipt_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int			inPrintIndex = 0, inRetVal;
	char			szTRTFileName[12 + 1];
	char			szShort_Receipt_Mode[2 + 1];
	char			szDebugMsg[100 + 1];
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inCMAS_PRINT_Receipt_ByBuffer() START !");
		inDISP_LogPrintfAt(AT,"uszESVCTransBit(%d), inCode(%d)  !",pobTran->srTRec.uszESVCTransBit,pobTran->srTRec.inCode);
		inDISP_LogPrintfAt(AT,"inTransactionCode(%d),inTicketType(%d)  !",pobTran->inTransactionCode,pobTran->srTRec.inTicketType);
	}
	
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	
	/* 縮小版帳單 */
	memset(szShort_Receipt_Mode, 0x00, sizeof(szShort_Receipt_Mode));
	inGetShort_Receipt_Mode(szShort_Receipt_Mode);
	if (memcmp(szShort_Receipt_Mode, "Y", strlen("Y")) == 0)
	{
		//inPrintIndex = 1;
		
		/* 電票交易目前沒有縮小簽單，導回同一個 */
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{		
			if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
			{
				inPrintIndex = 0;
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintfAt(AT,"inTicketType not define() return !");
				}
				return (VS_SUCCESS);
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintfAt(AT,"uszESVCTransBit not true() return !");
			}
			return (VS_SUCCESS);
		}
	}
	else
	{
		/* 電票交易 */
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			if ( pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ )
				return (VS_SUCCESS);
			
			if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
			{
				inPrintIndex = 0;
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintfAt(AT,"inTicketType not define() return_1!");
				}
				return (VS_SUCCESS);
			}
		}
		/* 信用卡一般 */
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintfAt(AT,"uszESVCTransBit not true() return_1!");
			}
			return (VS_SUCCESS);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inCMAS_PRINT_Receipt_ByBuffer()");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "EDC PrintIndex : %d", inPrintIndex);
		inDISP_LogPrintfAt(AT,szDebugMsg);
	}
	
	while (1)
	{
		inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
		/* 列印LOGO */
		if (srReceiptType_CMAS_ByBuffer[inPrintIndex].inLogo != NULL)
			if ((inRetVal = srReceiptType_CMAS_ByBuffer[inPrintIndex].inLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);

		/* 列印TID MID */
		if (srReceiptType_CMAS_ByBuffer[inPrintIndex].inTop != NULL)
			if ((inRetVal = srReceiptType_CMAS_ByBuffer[inPrintIndex].inTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);

		/* 列印DATA */
		if (srReceiptType_CMAS_ByBuffer[inPrintIndex].inData != NULL)
			if ((inRetVal = srReceiptType_CMAS_ByBuffer[inPrintIndex].inData(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);

		/* 列印金額 */
		if (srReceiptType_CMAS_ByBuffer[inPrintIndex].inAmount != NULL)
			if ((inRetVal = srReceiptType_CMAS_ByBuffer[inPrintIndex].inAmount(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);
						
		/* OTHER資料 */
		if (srReceiptType_CMAS_ByBuffer[inPrintIndex].inOther != NULL)
			if ((inRetVal = srReceiptType_CMAS_ByBuffer[inPrintIndex].inOther(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);

		/* 列印簽名欄  & 警語 */
		/* 因為有電子簽名圖檔所以個別處理 */
		if (srReceiptType_CMAS_ByBuffer[inPrintIndex].inEnd != NULL)
			if ((inRetVal = srReceiptType_CMAS_ByBuffer[inPrintIndex].inEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);
	
		if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
			return (inRetVal);
		break;
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_PRINT_TotalReport_ByBuffer
Date&Time       :2020/6/24 上午 11:20
Describe        :列印總額報表流程
Author          :Hachi
*/
int inCMAS_PRINT_TotalReport_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int			inPrintIndex = 0, inRetVal = 0;
	char			szDebugMsg[100 + 1] = {0};
	char			szCustomerIndicator[3 + 1] = {0};
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
	TICKET_ACCUM_TOTAL_REC	srAccumRec;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	/* 是否有列印功能 */
	/* [新增電票悠遊卡功能] 補加入檢查有沒有列印功能的條件 [SAM] 2022/7/12 下午 1:28*/
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		inDISP_LogPrintfAt(AT," inCMAS_PRINT_TotalReport_ByBuffer() No Print Funcion ");
		return (VS_SUCCESS);
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"----------------------------------------");
		inDISP_LogPrintfAt(AT,"inCMAS_PRINT_TotalReport_ByBuffer() START !");
	}

        if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
        {
                inPrintIndex = 0;
        }
    
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
		inDISP_LogPrintfAt(AT,szDebugMsg);
	}

	/* 開啟交易總的檔案 */
	memset(&srAccumRec, 0x00, sizeof(srAccumRec));
	inRetVal = inACCUM_GetRecord_ESVC(pobTran, &srAccumRec);
	if (inRetVal == VS_NO_RECORD)
	{
		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
	}
	else if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}

	/* [新增電票悠遊卡功能] 因列印格式關係，修改顯示圖片及顯示欄位，取代 _TOUCH_TCBUI_PRT_RECEIPT_ 圖片 [SAM] 2022/6/23 上午 10:52 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);/* 列印帳單中 */

        
	inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);

	/* 列印LOGO */
	if (srTotalReport_ByBuffer_CMAS[inPrintIndex].inReportLogo != NULL)
		if ((inRetVal = srTotalReport_ByBuffer_CMAS[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
			return (inRetVal);
	/* 列印TID MID */
	if (srTotalReport_ByBuffer_CMAS[inPrintIndex].inReportTop != NULL)
		if ((inRetVal = srTotalReport_ByBuffer_CMAS[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
			return (inRetVal);
	/* 全部金額總計 */
	if (srTotalReport_ByBuffer_CMAS[inPrintIndex].inAmount != NULL)
		if ((inRetVal = srTotalReport_ByBuffer_CMAS[inPrintIndex].inAmount(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
			return (inRetVal);

	/* 卡別金額總計 */
	// if (srTotalReport_ByBuffer_CMAS[inPrintIndex].inAmountByCard != NULL)
	// 	if ((inRetVal = srTotalReport_ByBuffer_CMAS[inPrintIndex].inAmountByCard(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
	// 		return (inRetVal);

	/* 結束 */
	if (srTotalReport_ByBuffer_CMAS[inPrintIndex].inReportEnd != NULL)
		if ((inRetVal = srTotalReport_ByBuffer_CMAS[inPrintIndex].inReportEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
			return (inRetVal);

	if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
		return (inRetVal);

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_PRINT_DetailReport_ByBuffer
Date&Time       :2020/6/20 下午 16:37
Describe        :列印明細帳單流程
Author          :Hachi
*/
int inCMAS_PRINT_DetailReport_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = 0, inPrintIndex = 0, inRecordCnt = 0;
	char			szTRTFileName[16 + 1];
	char			szDebugMsg[100 + 1];
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
	TICKET_ACCUM_TOTAL_REC	srAccumRec;
	
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);

	if (memcmp(szTRTFileName, _TRT_FILE_NAME_CMAS_, strlen(_TRT_FILE_NAME_CMAS_)) == 0)
	{
		inPrintIndex = 0;
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
		inDISP_LogPrintfAt(AT,szDebugMsg);
	}

	/* 檢查是否有帳 */
	if (srDetailReport_CMAS_ByBuffer[inPrintIndex].inReportCheck != NULL)
	{
		if ((inRecordCnt = srDetailReport_CMAS_ByBuffer[inPrintIndex].inReportCheck(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) == VS_ERROR)
			return (VS_ERROR); /* 表示檔案開啟失敗 */
	}

	/* 開啟交易總的檔案 */
	memset(&srAccumRec, 0x00, sizeof(srAccumRec));
	inRetVal = inACCUM_GetRecord_ESVC(pobTran, &srAccumRec);

	if (inRetVal == VS_NO_RECORD)
		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
	else if (inRetVal == VS_ERROR)
	{
		inDISP_LogPrintfAt(AT,"Get record 失敗.");

		return (VS_ERROR);
	}
         
	/* [新增電票悠遊卡功能] 因列印格式關係，修改顯示圖片及顯示欄位，取代 _TOUCH_TCBUI_PRT_RECEIPT_ 圖片 [SAM] 2022/6/23 上午 10:52 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);/* 列印帳單中 */
        
	while (1)
	{
		inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
		/* 列印LOGO */
		if (srDetailReport_CMAS_ByBuffer[inPrintIndex].inReportLogo != NULL)
			if ((inRetVal = srDetailReport_CMAS_ByBuffer[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);
		/* 列印TID MID */
		if (srDetailReport_CMAS_ByBuffer[inPrintIndex].inReportTop != NULL)
			if ((inRetVal = srDetailReport_CMAS_ByBuffer[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);
		/* 全部金額總計 */
		if (srDetailReport_CMAS_ByBuffer[inPrintIndex].inTotalAmount != NULL)
			if ((inRetVal = srDetailReport_CMAS_ByBuffer[inPrintIndex].inTotalAmount(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);

		/* 明細規格 */
		if (srDetailReport_CMAS_ByBuffer[inPrintIndex].inMiddle != NULL)
			if ((inRetVal = srDetailReport_CMAS_ByBuffer[inPrintIndex].inMiddle(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);
		/* 明細資料 */
		if (srDetailReport_CMAS_ByBuffer[inPrintIndex].inBottom != NULL)
		{
			inRetVal = srDetailReport_CMAS_ByBuffer[inPrintIndex].inBottom(pobTran, inRecordCnt, uszBuffer1, &srFont_Attrib1, &srBhandle1);
			if (inRetVal != VS_SUCCESS	&&
			    inRetVal != VS_NO_RECORD)
			{
				return (inRetVal);
			}
		}
		
		/* 結束 */
		if (srDetailReport_CMAS_ByBuffer[inPrintIndex].inReportEnd != NULL)
			if ((inRetVal = srDetailReport_CMAS_ByBuffer[inPrintIndex].inReportEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);

		if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
			return (inRetVal);

		break;
	}

	return (VS_SUCCESS);
}

/*-----------------功能-----------------------*/

/*
Function        :inCMAS_PRINT_Check_ByBuffer
Date&Time       :2020/6/20 上午 11:50
Describe        :
Author          :Hachi
*/
int inCMAS_PRINT_Check_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int     inRecordCnt;

	inRecordCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite_ESVC(pobTran);
	/* 回傳VS_ERROR(回傳 -1 )會跳出，交易筆數小於0( VS_NoRecord 會回傳 -98 )會印空白簽單 */
	/* 其餘則回傳交易筆數*/

	return (inRecordCnt);
}

/*
Function        :inCMAS_PRINT_Notice
Date&Time       :2020/6/20 上午 11:40
Describe        :用來決定要不要印商店提示
*/
int inCMAS_PRINT_Notice(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, BufferHandle *srBhandle)
{
	char	szPrtNotice[1 + 1];
	
	/* szPrtNotice沒On代表不用印商店提示 */
	memset(szPrtNotice, 0x00, sizeof(szPrtNotice));
	inGetPrtNotice(szPrtNotice);
	if (memcmp(szPrtNotice, "N", 1) == 0)
	{
		return (VS_SUCCESS);
	}
	
	if (inPRINT_Buffer_PutGraphic((unsigned char*)_NOTICE_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inNoticeHeight, _APPEND_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        : inCMAS_PRINT_End_ByBuffer
Date&Time   : 2020/6/20 下午 4:13
Describe        : 列印結尾
Author           : Hachi
*/
int inCMAS_PRINT_End_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
	
	inPRINT_Buffer_PutIn("*** 報表結束 ***", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

	for (i = 0; i < 8; i++)
	{
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	return (VS_SUCCESS);
}

/*-----------------簽單-----------------------*/
/*
Function        : inCMAS_PRINT_Logo_ByBuffer
Date&Time   : 2020/6/20 下午 15:30
Describe        : 悠遊卡總額只印銀行LOGO
Author          : Hachi
 * [新增電票悠遊卡功能] 修改列印LOG的方法，與原合庫的列印有差表，不使用 _NCCC_LOGO_ [SAM] 2022/6/23 下午 2:53
*/
int inCMAS_PRINT_Logo_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	/* 印NCC的LOGO */
	if (inPRINT_Buffer_PutGraphic((unsigned char*)_BANK_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inBankLogoHeight, _APPEND_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	  /* 印商店的LOGO */
	if (inCREDIT_PRINT_MerchantLogo(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 印商店名稱 */
	if (inCREDIT_PRINT_MerchantNameTXT(pobTran, uszBuffer, srFont_Attrib, srBhandle) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_PRINT_TIDMID_ByBuffer
Date&Time       :2020/6/20 下午 3:24
Describe        :列印TID & MID
Author          :Hachi
*/
int inCMAS_PRINT_Tidmid_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        int inRetVal;
        char szPrintBuf[84 + 1], szTemplate[42 + 1];
        char szTRTFileName[12 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
        inGetTRTFileName(szTRTFileName);

        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintfAt(AT, "inCMAS_PRINT_TIDMID_ByBuffer start");
                inDISP_LogPrintfAt(AT, "szTRTFileName (%s), inPrinttype_ByBuffer(%d) ", szTRTFileName, inPrinttype_ByBuffer);
        }

        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                /* Get商店代號 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetMerchantID(szTemplate);

                /* 列印商店代號 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "商店代號：%s", szTemplate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                    return (VS_ERROR);

                /* Get端末機代號 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetTerminalID(szTemplate);

                /* 列印端末機代號 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "端末機代號：%s", szTemplate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                    return (VS_ERROR);

                if (!memcmp(_TRT_FILE_NAME_AMEX_, szTRTFileName, strlen(_TRT_FILE_NAME_AMEX_)))
                {
                    memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                    sprintf(szPrintBuf, "%s", "收單機構:美國運通");
                    inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                    if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
                }
        }
        else
        {
                /* 橫式 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetMerchantID(szTemplate);

                /* 列印商店代號 */
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_LEFT_);
                sprintf(szPrintBuf, "商店代號 %s", szTemplate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* Get端末機代號 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetTerminalID(szTemplate);

                /* 列印端末機代號 */
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 13, _PADDING_LEFT_);
                sprintf(szPrintBuf, "端末機代號 %s", szTemplate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (!memcmp(_TRT_FILE_NAME_AMEX_, szTRTFileName, strlen(_TRT_FILE_NAME_AMEX_)))
                {
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "%s", "收單機構:美國運通");
                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                        if (inRetVal != VS_SUCCESS)
                            return (VS_ERROR);
                }

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
        }

        return (inRetVal);
}

/*
Function        :inCMAS_PRINT_Data_ByBuffer
Date&Time       :2020/6/8 下午 4:37
Describe        :列印DATA
Author          :Hachi
*/
int inCMAS_PRINT_Data_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inLen = 0;
	int     inRetVal = VS_SUCCESS;
	char 	szPrintBuf[84 + 1], szTemplate1[42 + 1];
	char	szTemplate[40 + 1];
	//char	szProductCodeEnable[1 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inCMAS_PRINT_Data_ByBuffer() START");
		inDISP_LogPrintfAt(AT,"srTRec.inTicketType(%d), srTRec.inCode(%d)", pobTran->srTRec.inTicketType, pobTran->srTRec.inCode);
	}

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	/* 橫式 */
	/* 卡號、卡別 */
	inRetVal = inPRINT_Buffer_PutIn("卡號(Card No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 卡號值 */
	
        /*卡號加密*/
        // memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        // inLen = strlen(pobTran->srTRec.szUID);
        // memcpy(szPrintBuf, pobTran->srTRec.szUID, inLen);
        // szPrintBuf[inLen - 1] = 0x2A;
        // szPrintBuf[inLen - 2] = 0x2A;
        // strcat(szPrintBuf, "(W)");

        // inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        // if (inRetVal != VS_SUCCESS)
        // 	return (VS_ERROR);
	
	/* 卡號不加密 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        inLen = strlen(pobTran->srTRec.szUID);
        memcpy(szPrintBuf, pobTran->srTRec.szUID, inLen);
        strcat(szPrintBuf, "(W)");

        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
	
    
        /* 卡別 */
        inRetVal = inPRINT_Buffer_PutIn("卡別(Card Type)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcpy(szPrintBuf, "悠遊卡");
	
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
    

	/* 交易別 & 特店代號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	
	if (pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_)
	{
		if (pobTran->srTRec.lnTotalTopUpAmount > 0)
			strcpy(szTemplate, "購貨、自動加值");
		else
			strcpy(szTemplate, "購貨　　");
	}
	else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_)
		strcpy(szTemplate, "退貨　　");
	else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_)
		strcpy(szTemplate, "現金加值");  
	else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_)
		strcpy(szTemplate, "取消加值");              
	else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_)
		strcpy(szTemplate, "餘額查詢");  
	else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_AUTO_TOP_UP_)
		strcpy(szTemplate, "自動加值");
	// else if(pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_DEDUCT_) 
	// 	strcpy(szTemplate, "取消購貨");	
	else
		strcpy(szTemplate, "　　");
	
	sprintf(szPrintBuf, "交易　%s", szTemplate);
	
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 主機、調閱編號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetHostLabel(szTemplate);
	
	sprintf(szPrintBuf, "主機　%s", szTemplate);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	if (pobTran->srTRec.inCode != _TICKET_EASYCARD_INQUIRY_)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "調閱號　 %s", "");
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%06ld", pobTran->srTRec.lnInvNum);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	else
	{
		/* 現在查詢餘額不印簽單 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, " ");
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	/* 批號 */

        if (pobTran->srTRec.inTicketType != _TICKET_EASYCARD_INQUIRY_ )
        {
                //inLoadTDTRec(_TDT_INDEX_01_ECC_);/*20200515*/
                inLoadTDTRec(_TDT_INDEX_01_ECC_); /*20210427[Hachi]*/
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetTicket_Batch(szTemplate);
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "悠遊卡批號　%s", szTemplate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
        }
	
	/* 日期時間 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%.4s/%.2s/%.2s  %.2s:%.2s:%.2s",
	&pobTran->srTRec.szDate[0], 
	&pobTran->srTRec.szDate[4], 
	&pobTran->srTRec.szDate[6], 
	&pobTran->srTRec.szTime[0], 
	&pobTran->srTRec.szTime[2],
	&pobTran->srTRec.szTime[4]); 
		
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "日期/時間 %s", szTemplate); 
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetTicket_Device2(szTemplate);
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "二代設備編號　%s", szTemplate);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);

        memset(szTemplate, 0x00, sizeof(szTemplate));
        strcpy(szTemplate, pobTran->srTRec.srECCRec.szRRN);
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "RRN   %s", szTemplate);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
	
	/* 斷行 */
	inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	return (inRetVal);
}

/*
Function        :inCMAS_PRINT_Amount_ByBuffer
Date&Time       :2020/6/20 下午 1:01
Describe        :
Author          :Hachi
*/
int inCMAS_PRINT_Amount_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_SUCCESS;
	char    szPrintBuf[42 + 1];
	
	if (ginDebug ==VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inCMAS_PRINT_Amount_ByBuffer Start!!!");
		inDISP_LogPrintfAt(AT,"pobTran->srTRec.inCode == %d",pobTran->srTRec.inCode );
	}
	
	if (pobTran->srTRec.inCode == _TICKET_EASYCARD_AUTO_TOP_UP_) /* 20200413 marked [Hachi]*/
	{
		/* 自動加值金額 */
		inRetVal = inPRINT_Buffer_PutIn("自動加值金額 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf)); 
		sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount);
		inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_NONE_, 10, _PADDING_LEFT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		/* 交易前餘額 */
		inRetVal = inPRINT_Buffer_PutIn("交易前餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		if (pobTran->srTRec.lnCardRemainAmount < 0)
		{
			sprintf(szPrintBuf, "%ld", 0 - pobTran->srTRec.lnCardRemainAmount);
		}
		else
		{
			sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnCardRemainAmount);
		}
	        
		if (pobTran->srTRec.lnCardRemainAmount < 0)
		{
			inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_MINUS_, 10, _PADDING_LEFT_);
		}
		else
		{
			inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_NONE_, 10, _PADDING_LEFT_);
		}
                                
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);   
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		/* 交易後餘額 */
		inRetVal = inPRINT_Buffer_PutIn("交易後餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		if (pobTran->srTRec.lnCardRemainAmount < 0)
		{
			sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount - pobTran->srTRec.lnCardRemainAmount);
		}
		else
		{
			sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount + pobTran->srTRec.lnCardRemainAmount);
		}
		
		inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_NONE_, 10, _PADDING_LEFT_);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);      
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
	}		  
	else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_)  /*20200413 new*/		 
	{
		/*卡片餘額*/
		inRetVal = inPRINT_Buffer_PutIn("卡片餘額　　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalAfterAmt);
		inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_NONE_, 10, _PADDING_LEFT_);
                
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);  
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	else
	{
		if (pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_ ) 
		{
			if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
			{
				/* 自動加值金額 */
				inRetVal = inPRINT_Buffer_PutIn("自動加值金額 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
				{
					return (VS_ERROR);
				}

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf)); 
				sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount);
				inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_NONE_, 10, _PADDING_LEFT_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
				{
					return (VS_ERROR);
				}

				inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
				{ 
					return (VS_ERROR);
				}
			}
		}  

		/* 交易前餘額 */
		inRetVal = inPRINT_Buffer_PutIn("交易前餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		/*20200420這邊會造成簽單卡片餘額 = 卡片餘額-加值金額先註解掉*/
		// if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
		// {
		// 	if (pobTran->srTRec.lnFinalBeforeAmt > 100000)
		// 		{
		// 					sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount + (pobTran->srTRec.lnFinalBeforeAmt - 100000));
		// 		}
		// 	else
		// 		{
		// 		sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalBeforeAmt - pobTran->srTRec.lnTotalTopUpAmount);
		// 		}
		// }
    	// else
		{
			if (pobTran->srTRec.lnFinalBeforeAmt > 100000)
			{
				sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalBeforeAmt - 100000);
			}
			else
			{
				sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalBeforeAmt);
			}
		}

		if (pobTran->srTRec.lnFinalBeforeAmt > 100000)
		{
			inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_MINUS_, 10, _PADDING_LEFT_);
		}
		else
		{
			inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_NONE_, 10, _PADDING_LEFT_);
		}

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 交易金額 */
		inRetVal = inPRINT_Buffer_PutIn("交易金額　　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		/* 20200420 悠遊卡取消加值顯示為負值[Hachi]start*/
		if(pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_) /*取消加值金額顯示負值*/
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%ld",( 0 - pobTran->srTRec.lnTxnAmount));
			inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_NONE_, 10, _PADDING_LEFT_);
		}
		else
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTxnAmount);
			inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_NONE_, 10, _PADDING_LEFT_);
		}
		/* 20200420 Edited by Hachi end*/

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);  
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		/* 交易後餘額 */
		inRetVal = inPRINT_Buffer_PutIn("交易後餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		if (pobTran->srTRec.lnFinalAfterAmt > 100000)
		{
			sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);
			inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_MINUS_, 10, _PADDING_LEFT_);
		}
		else
		{
			sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalAfterAmt);
			inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_NONE_, 10, _PADDING_LEFT_);
		}

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_PRINT_ReceiptEND_ByBuffer
Date&Time       :2020/6/20 下午 3:30
Describe        :簽單底部(客服資訊、收執聯)
Author          :Hachi
*/
int inCMAS_PRINT_ReceiptEND_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
	int	inRetVal = VS_SUCCESS;
	
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		inRetVal = inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		inRetVal = inPRINT_Buffer_PutIn("備註：若有疑問請洽悠遊卡票證公司", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		inRetVal = inPRINT_Buffer_PutIn("客服專線：412-8880(手機或金馬地區請加02)", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	if (pobTran->srTRec.inPrintOption == _PRT_MERCH_)
	{
		inRetVal = inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	
		inRetVal = inPRINT_Buffer_PutIn("商店存根聯", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	else if (pobTran->srTRec.inPrintOption == _PRT_CUST_)
	{
		inRetVal = inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	
		inRetVal = inPRINT_Buffer_PutIn("顧客收執聯", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
	{
		inRetVal = inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	/* Print Notice */
	if (inCMAS_PRINT_Notice(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
		
	inRetVal = inPRINT_Buffer_PutIn("列印結束", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 列印空白行 */
	for (i = 0; i < 8; i++)
	{
		inRetVal = inPRINT_Buffer_PutIn(" ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	return (VS_SUCCESS);      
}

/*-----------------總額---------------------*/
/*
Function        :inCMAS_PRINT_Top_SETTLE_ByBuffer
Date&Time       :2020/6/20 下午 2:49
Describe        :
Author          :Hachi
*/
int inCMAS_PRINT_Top_SETTLE_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal;
	char    szPrintBuf[84 + 1], szTemplate[42 + 1];

	/* Get商店代號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetMerchantID(szTemplate);

	/* 列印商店代號 */
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_LEFT_);
	sprintf(szPrintBuf, "商店代號%s", szTemplate);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* Get端末機代號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);

	/* 列印端末機代號 */
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_);
	sprintf(szPrintBuf, "端末機代號%s", szTemplate);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 交易 特店代號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	
	sprintf(szPrintBuf, "交易　　 %s", "結帳");
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "特店代號　%s", "");
	inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 主機、批號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	
        inGetHostLabel(szTemplate);
        sprintf(szPrintBuf, "主機　　 %s", szTemplate);
    
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "批號　　 %s", "");
	inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	inGetTicket_Batch(szPrintBuf);
	
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 列印日期 / 時間 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inRetVal = inPRINT_Buffer_PutIn("日期/時間(Date/Time)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	sprintf(szPrintBuf, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_PRINT_TotalAmount_ByBuffer_Settle
Date&Time       :2020/6/20 下午 4:54
Describe        :列印總金額
Author          :Hachi
*/
int inCMAS_PRINT_TotalAmount_ByBuffer_Settle(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char    szPrintBuf[84 + 1], szTemplate[84 + 1];

	if (pobTran->inTransactionCode == _SETTLE_)
	{
                if (pobTran->inRunOperationID == _OPERATION_TOTAL_REPORT_) 
                        inPRINT_Buffer_PutIn("         總額報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_); //新增錢櫃悠遊卡功能 add by sampo 20211012
                else
                        inPRINT_Buffer_PutIn("         結帳報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	else
	{	
		inPRINT_Buffer_PutIn("         總額報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	inPRINT_Buffer_PutIn("       筆數           金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"********************************************************");
		inDISP_LogPrintfAt(AT,"in inCMAS_PRINT_TotalAmount_ByBuffer_Settle");
		inDISP_LogPrintfAt(AT,"llEASYCARD_DeductTotalAmount:%llu(%ld)",srAccumRec->llEASYCARD_DeductTotalAmount,srAccumRec->lnEASYCARD_DeductTotalCount);
		inDISP_LogPrintfAt(AT,"llEASYCARD_RefundTotalAmount:%lld(%ld)",srAccumRec->llEASYCARD_RefundTotalAmount,srAccumRec->lnEASYCARD_RefundTotalCount);
		inDISP_LogPrintfAt(AT,"lnEASYCARD_ADDTotalCount:%lld(%ld)",srAccumRec->llEASYCARD_ADDTotalAmount,srAccumRec->lnEASYCARD_ADDTotalCount);
		inDISP_LogPrintfAt(AT,"llEASYCARD_VoidADDTotalAmount:%lld(%ld)",srAccumRec->llEASYCARD_VoidADDTotalAmount,srAccumRec->lnEASYCARD_VoidADDTotalCount);
		inDISP_LogPrintfAt(AT,"********************************************************");
	}
	/*1. 銷售 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "購貨 　　%03lu   NT$", srAccumRec->lnEASYCARD_DeductTotalCount);
	sprintf(szTemplate, "%lld", srAccumRec->llEASYCARD_DeductTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/*2. 退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnEASYCARD_RefundTotalCount);
	sprintf(szTemplate, "%lld", ( 0 - srAccumRec->llEASYCARD_RefundTotalAmount));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* 3. 現金加值*/
	if (srAccumRec->lnEASYCARD_ADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "現金加值 %03lu   NT$", srAccumRec->lnEASYCARD_ADDTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llEASYCARD_ADDTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	/*4. 取消加值 */
	if (srAccumRec->lnEASYCARD_VoidADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "取消加值 %03lu   NT$", srAccumRec->lnEASYCARD_VoidADDTotalCount);
		sprintf(szTemplate, "%lld", ( 0 - srAccumRec->llEASYCARD_VoidADDTotalAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	/*5. 自動加值 */
	if (srAccumRec->lnEASYCARD_AutoADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "自動加值 %03lu   NT$", srAccumRec->lnEASYCARD_AutoADDTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llEASYCARD_AutoADDTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        /* 20200414 購貨總額 = 購貨 - 退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "購貨總額 %03lu   NT$",(srAccumRec->lnEASYCARD_DeductTotalCount + srAccumRec->lnEASYCARD_RefundTotalCount ));
	sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_DeductTotalAmount - srAccumRec->llEASYCARD_RefundTotalAmount ));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
    
        /* 20200414 加值總額 = 現金加值 - 取消加值*/
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "加值總額 %03lu   NT$",(srAccumRec->lnEASYCARD_ADDTotalCount + srAccumRec->lnEASYCARD_VoidADDTotalCount ));
	sprintf(szTemplate, "%lld", srAccumRec->llEASYCARD_ADDTotalAmount - srAccumRec->llEASYCARD_VoidADDTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_PRINT_TotalAmountByCard_ByBuffer
Date&Time       :2020/6/20 下午 4:19
Describe        :依卡別列印，列印所有票證資料，目前沒用到。
Author          :Author
*/
int inCMAS_PRINT_TotalAmountByCard_ByBuffer(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int		i = 0;
	char		szPrintBuf[100 + 1], szTemplate[42 + 1];
	char		szTxnType[20 + 1];
	unsigned char	uszNeedPrintBit = VS_FALSE;	
	
	/* 先檢查是否任一票證有開 */
	for (i = 0; i < 4; i++)
	{
		if (inLoadTDTRec(i) != VS_SUCCESS)
		{
			break;
		}
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTicket_HostEnable(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) != 0)
		{
			continue;
		}
		else
		{
			uszNeedPrintBit = VS_TRUE;
		}
	}
	
	/* 代表有任一票證要印 */
	if (uszNeedPrintBit == VS_TRUE)
	{
		inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("         卡別小計", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	else
	{
		return (VS_SUCCESS);
	}
	
	/* 個別票證 */
	for (i = 0; i < 4; i++)
	{
		if (inLoadTDTRec(i) != VS_SUCCESS)
		{
			break;
		}
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTicket_HostEnable(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) != 0)
		{
			continue;
		}

		memset(szTxnType, 0x00, sizeof(szTxnType));
		inGetTicket_HostTransFunc(szTxnType);

		switch (i)
		{
			case _TDT_INDEX_00_IPASS_:
				inPRINT_Buffer_PutIn("卡別 一卡通", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				break;
			case _TDT_INDEX_01_ECC_:
				inPRINT_Buffer_PutIn("卡別 悠遊卡", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				//sprintf(szPrintBuf, "二代設備編號　 %s", szGetCMAS_Device2());
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				//sprintf(szPrintBuf, "悠遊卡批次號碼 %s", szGetCMAS_SETTLE_DATE());
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				break;
			case _TDT_INDEX_02_ICASH_:
				inPRINT_Buffer_PutIn("卡別 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				break;
			case _TDT_INDEX_03_HAPPYCASH_:
				inPRINT_Buffer_PutIn("卡別 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				break;
			default:
				break;
		}

		/* 不合法 跳出 */
		if (i != _TDT_INDEX_00_IPASS_	&&
		    i != _TDT_INDEX_01_ECC_	&&
		    i != _TDT_INDEX_02_ICASH_	&&
		    i != _TDT_INDEX_03_HAPPYCASH_)
		{
			break;
		}
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		switch (i)
		{
			case 0:
				sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnIPASS_DeductTotalCount);
				sprintf(szTemplate, "%lld", srAccumRec->llIPASS_DeductTotalAmount);
				break;
			case 1:
				sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnEASYCARD_DeductTotalCount);
				sprintf(szTemplate, "%lld", srAccumRec->llEASYCARD_DeductTotalAmount);
				break;
			default:
				sprintf(szPrintBuf, "購貨　　　%03lu   NT$", 0l);
				sprintf(szTemplate, "%lld", 0ll);
				break;
		}

		inFunc_Amount_Comma(szTemplate, "", ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        	
		
		/* (有開通遠鑫卡時設定開啟及有筆數時列印) */
		if (i == _TDT_INDEX_03_HAPPYCASH_		&& 
		    szTxnType[4] == 0x59			&& 
		    srAccumRec->lnHAPPYCASH_VoidDeductTotalCount > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			switch(i)
			{
				case 0 :
					sprintf(szPrintBuf, "取消購貨　%03lu   NT$", srAccumRec->lnHAPPYCASH_VoidDeductTotalCount);                   
					sprintf(szTemplate, "%lld", (srAccumRec->llHAPPYCASH_VoidDeductTotalAmount));
					break;
				default :
					sprintf(szPrintBuf, "取消購貨　%03lu   NT$", 0l);                   
					sprintf(szTemplate, "%lld", 0ll);
					break;        
			}

			inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		}
        	
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		switch (i)
		{
			case 0:
				sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnIPASS_RefundTotalCount);
				sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_RefundTotalAmount));
				break;
			case 1:
				sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnEASYCARD_RefundTotalCount);
				sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_RefundTotalAmount));
				break;
			default:
				sprintf(szPrintBuf, "退貨　　　%03lu   NT$", 0l);
				sprintf(szTemplate, "%lld", 0ll);
				break;
		}

		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	
		/* 加值功能有開且有交易筆數 */
		if (szTxnType[4] == 0x59 && srAccumRec->lnIPASS_ADDTotalCount > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

			switch (i)
			{
				case 0:
					sprintf(szPrintBuf, "現金加值  %03lu   NT$", srAccumRec->lnIPASS_ADDTotalCount);
					sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_ADDTotalAmount));
					break;
				case 1:
					sprintf(szPrintBuf, "現金加值  %03lu   NT$", srAccumRec->lnEASYCARD_ADDTotalCount);
					sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_ADDTotalAmount));
					break;
				default:
					sprintf(szPrintBuf, "現金加值  %03lu   NT$", 0l);
					sprintf(szTemplate, "%lld", 0ll);
					break;
			}

			inFunc_Amount_Comma(szTemplate, "", ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		/* 加值取消功能有開且有交易筆數 */
		if (szTxnType[5] == 0x59 && srAccumRec->lnIPASS_VoidADDTotalCount > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			switch (i)
			{
				case 0:
					sprintf(szPrintBuf, "取消加值　%03lu   NT$", srAccumRec->lnIPASS_VoidADDTotalCount);
					sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_VoidADDTotalAmount));
					break;
				case 1:
					sprintf(szPrintBuf, "取消加值　%03lu   NT$", srAccumRec->lnEASYCARD_VoidADDTotalCount);
					sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_VoidADDTotalAmount));
					break;
				default:
					sprintf(szPrintBuf, "取消加值  %03lu   NT$", 0l);
					sprintf(szTemplate, "%lld", 0ll);
					break;
			}

			inFunc_Amount_Comma(szTemplate, "", ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		/* 自動加值功能有開且有交易筆數 */
		if (szTxnType[3] == 0x59 && srAccumRec->lnIPASS_AutoADDTotalCount > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			switch (i)
			{
				case 0:
					sprintf(szPrintBuf, "自動加值　%03lu   NT$", srAccumRec->lnIPASS_AutoADDTotalCount);
					sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_AutoADDTotalAmount));
					break;
				case 1:
					sprintf(szPrintBuf, "自動加值　%03lu   NT$", srAccumRec->lnEASYCARD_AutoADDTotalCount);
					sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_AutoADDTotalAmount));
					break;
				default:
					sprintf(szPrintBuf, "自動加值　%03lu   NT$", 0l);
					sprintf(szTemplate, "%lld", 0ll);
					break;
			}

			inFunc_Amount_Comma(szTemplate, "", ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		inPRINT_Buffer_PutIn("------------------------------------------", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		switch (i)
		{
			case 0:
				sprintf(szPrintBuf, "總額小計　%03lu   NT$", srAccumRec->lnIPASS_TotalCount);
				sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_TotalAmount));
				inFunc_Amount_Comma(szTemplate, "", ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
				strcat(szPrintBuf, szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				sprintf(szPrintBuf, "淨額小計　      NT$");
				sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_DeductTotalAmount - srAccumRec->llIPASS_RefundTotalAmount));
				inFunc_Amount_Comma(szTemplate, "", ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
				strcat(szPrintBuf, szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				break;
			case 1:
				sprintf(szPrintBuf, "總額小計　%03lu   NT$", srAccumRec->lnEASYCARD_TotalCount);
				sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_TotalAmount));
				inFunc_Amount_Comma(szTemplate, "", ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
				strcat(szPrintBuf, szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				sprintf(szPrintBuf, "淨額小計　      NT$");
				sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_DeductTotalAmount - srAccumRec->llEASYCARD_RefundTotalAmount));
				inFunc_Amount_Comma(szTemplate, "", ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
				strcat(szPrintBuf, szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				break;
			default:
				sprintf(szPrintBuf, "總額小計　%03lu   NT$", 0l);
				sprintf(szTemplate, "%lld", 0ll);
				inFunc_Amount_Comma(szTemplate, "", ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
				strcat(szPrintBuf, szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				sprintf(szPrintBuf, "淨額小計　%03lu   NT$", 0l);
				sprintf(szTemplate, "%lld", 0ll);
				inFunc_Amount_Comma(szTemplate, "", ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
				strcat(szPrintBuf, szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				break;        
        	}
        	
        	inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	}

	inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

	return (VS_SUCCESS);
}

/*-----------------明細---------------------*/
/*
Function        :inCMAS_PRINT_DetailReportTop_ByBuffer
Date&Time       :2020/6/20 下午 2:49
Describe        :明細報表簽單上部(商代、端代、批次號、主機)
Author          :Hachi
*/
int inCMAS_PRINT_DetailReportTop_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal;
	char    szPrintBuf[84 + 1], szTemplate[42 + 1];
	char	szHostName[84 + 1];
	
	/* Get商店代號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetMerchantID(szTemplate);

	/* 列印商店代號 */
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_LEFT_);
	sprintf(szPrintBuf, "商店代號%s", szTemplate);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* Get端末機代號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);

	/* 列印端末機代號 */
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_);
	sprintf(szPrintBuf, "端末機代號%s", szTemplate);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
        
	/* 列印日期 / 時間 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inRetVal = inPRINT_Buffer_PutIn("日期/時間(Date/Time)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	sprintf(szPrintBuf, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
        
	/* 批次號碼 */
	inRetVal = inPRINT_Buffer_PutIn("批次號碼(Batch No.)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        inGetTicket_Batch(szPrintBuf);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
	
	/* 主機*/
	inRetVal = inPRINT_Buffer_PutIn("主機(Host)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
    
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        inGetHostLabel(szPrintBuf);
        
	memset(szHostName, 0x00, sizeof(szHostName));
        if (memcmp(szPrintBuf, _HOST_NAME_CMAS_, strlen(_HOST_NAME_CMAS_)) == 0)
		sprintf(szHostName, "%s%s", "悠遊卡", szPrintBuf);
	else
		sprintf(szHostName, "%s%s", "", szPrintBuf);
	
	inRetVal = inPRINT_Buffer_PutIn(szHostName, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
        
	return (VS_SUCCESS);
}

/*
Function        :inCMAS_PRINT_DetailReportMiddle_ByBuffer
Date&Time       :2020/6/20 下午 4:27
Describe        :明細報表中段字樣(交易類別、交易金額、卡號、交易日期、交易時間、RRN)
Author          :Hachi
*/
int inCMAS_PRINT_DetailReportMiddle_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	inPRINT_Buffer_PutIn("明細報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

        inPRINT_Buffer_PutIn("交易類別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
        inPRINT_Buffer_PutIn_Specific_X_Position("交易金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

        inPRINT_Buffer_PutIn("卡號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        inPRINT_Buffer_PutIn("交易日期", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
        inPRINT_Buffer_PutIn_Specific_X_Position("交易時間", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

        // inPRINT_Buffer_PutIn("CTN序號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
        inPRINT_Buffer_PutIn("調閱編號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        inPRINT_Buffer_PutIn("RRN", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_PRINT_DetailReportBottom_ByBuffer
Date&Time       :2020/6/20 下午 4:16
Describe        :明細報表底部資料(交易類別、交易金額、卡號、交易日期、交易時間、RRN)
Author          :Hachi
*/
int inCMAS_PRINT_DetailReportBottom_ByBuffer(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inReadCnt = 0;
	int	inRetVal = VS_SUCCESS;
	char	szPrintBuf[62 + 1], szTemplate1[62 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inCMAS_PRINT_DetailReportBottom_ByBuffer()_START");
	}
	
	/* 開始讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_START(pobTran);

	for (inReadCnt = 0; inReadCnt < inRecordCnt; inReadCnt ++)
	{
			/*. 開始讀取每一筆交易記錄 .*/
		if (inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_Read(pobTran, inReadCnt) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
			break;
		}
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintfAt(AT,"inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_Read inRetVal = %x",inRetVal);
		}
		
		/* Trans type */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
                
		inFunc_GetTransType_ESVC(pobTran, szTemplate1);
		sprintf(szPrintBuf, "%s", szTemplate1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_); //2019/11/14 下午 3:58 Hachi
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* Print Amount */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));

		switch (pobTran->srTRec.inCode)
		{
			case _TICKET_EASYCARD_AUTO_TOP_UP_:
				sprintf(szTemplate1, "%ld", pobTran->srTRec.lnTotalTopUpAmount);
				break;	
			case _TICKET_EASYCARD_VOID_TOP_UP_:
				sprintf(szTemplate1, "%ld",  (0 -pobTran->srTRec.lnTxnAmount));
				break;	
			default :
				sprintf(szTemplate1, "%ld", pobTran->srTRec.lnTxnAmount);
				break;
		} /* End switch () */
		
		inFunc_Amount_Comma(szTemplate1, "NT$" , 0x00, _SIGNED_NONE_, 16, _PADDING_RIGHT_);
		strcat(szPrintBuf, szTemplate1);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
			
		/* 卡號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                strcpy(szPrintBuf, pobTran->srTRec.szUID);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* Trans Date Time */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "DATE: %.4s/%.2s/%.2s", &pobTran->srTRec.szDate[0], &pobTran->srTRec.szDate[4], &pobTran->srTRec.szDate[6]);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "TIME: %.2s:%.2s",  &pobTran->srTRec.szTime[0], &pobTran->srTRec.szTime[2]);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 調閱編號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "INV:%06ld", pobTran->srTRec.lnInvNum);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* RRN */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "RRN : %s",pobTran->srTRec.srECCRec.szRRN);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);


		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
				
		inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	} /* End for () .... */
	
	/* 結束讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_END(pobTran);
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"Fun is %s, Line is %d = %x",__FUNCTION__,__LINE__);
	}

	inPRINT_Buffer_PutIn("=====================================================================================", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inEDC_PRINT_DetailReportBottom_ByBuffer_CMAS()_END");
	}
	
	return (inRetVal);
}
