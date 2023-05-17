#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <ctos_qrcode.h>
#include <sqlite3.h>

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
#include "../../FUNCTION/Sqlite.h"
#include "../../COMM/Comm.h"
#include "../../FUNCTION/Accum.h"
#include "../../FUNCTION/Batch.h"
#include "../../FUNCTION/ASMC.h"
#include "../../FUNCTION/Card.h"
#include "../../FUNCTION/CDT.h"
#include "../../FUNCTION/CFGT.h"
#include "../../FUNCTION/CPT.h"
#include "../../FUNCTION/EDC.h"
#include "../../FUNCTION/EST.h"
#include "../../FUNCTION/HDT.h"
#include "../../FUNCTION/HDPT.h"
#include "../../FUNCTION/SCDT.h"
#include "../../FUNCTION/Signpad.h"
#include "../../FUNCTION/PWD.h"
#include "../../FUNCTION/PCD.h"
#include "../../FUNCTION/IPASSDT.h"
#include "../../FUNCTION/ECCDT.h"
#include "../../FUNCTION/TDT.h"
#include "../../FUNCTION/UNIT_FUNC/TimeUint.h"

#include "../../EVENT/Flow.h"
#include "../../EVENT/MenuMsg.h"

#include "../../../CTLS/CTLS.h"
#include "../CARD_FUNC/CardFunction.h"

#include "../../TMS/TMSTABLE/TmsCPT.h"
#include "../../TMS/TMSTABLE/TmsFTP.h"
#include "../../TMS/TMSTABLE/TmsSCT.h"
#include "../../TMS/EDCTmsDefine.h"

#include "../COSTCO_FUNC/Costco.h"

#include "PrintBillInfo.h"


extern  int		ginDebug;			/* Debug使用 extern */
extern	int		ginMachineType;
extern	char		gszTermVersionID[16 + 1];
extern	char		gszTermVersionDate[16 + 1];
extern	BMPHeight	gsrBMPHeight;			/* 圖片高度 */

int     inPrinttype_ByBuffer = 0;        /* 0 = 橫式，1 = 直式 */

/* 列印帳單使用(START) */
PRINT_RECEIPT_TYPE_TABLE_BYBUFFER srReceiptType_ByBuffer[] =
{
	/* 信用卡 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_Data_ByBuffer,
		inCREDIT_PRINT_Cup_Amount_ByBuffer,
		inCREDIT_PRINT_NewAmountByBuffer,
		inCREDIT_PRINT_NewInstByBuffer,
		inCREDIT_PRINT_NewRedeemByBuffer,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer
	},
	/* SmartPay簽單 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_FISC_Data_ByBuffer,
		_NULL_CH_,
		inCREDIT_PRINT_FISC_Amount_ByBuffer,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer
	},
	/* 一卡通簽單 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_Data_ByBuffer_ESVC,
		_NULL_CH_,
		inCREDIT_PRINT_Amount_ByBuffer_IPASS,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_ESVC
	},
	/* 悠遊卡簽單 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_Data_ByBuffer_ESVC,
		_NULL_CH_,
		inCREDIT_PRINT_Amount_ByBuffer_ECC,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_ESVC
	},
	/* 愛金卡簽單 */
	{
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_
	},

	/* happycash簽單 */
	{
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_
	},
	/* 縮小版簽單 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_Small,
		inCREDIT_PRINT_Data_ByBuffer_Small,
		inCREDIT_PRINT_Cup_Amount_ByBuffer_Small,
		inCREDIT_PRINT_Amount_ByBuffer_Small,
		inCREDIT_PRINT_Inst_ByBuffer_Small,
		inCREDIT_PRINT_Redeem_ByBuffer_Small,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_Small
	},
	/* 縮小版SmartPay簽單 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_Small,
		inCREDIT_PRINT_FISC_Data_ByBuffer_Small,
		_NULL_CH_,
		inCREDIT_PRINT_FISC_Amount_ByBuffer_Small,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_Small
	},

};

/* 列印總額報表使用 (START) */
TOTAL_REPORT_TABLE_BYBUFFER srTotalReport_ByBuffer[] =
{
	/* 不印ESC結帳條(Ex:Diners) */
	{
		 inCREDIT_PRINT_Check_ByBuffer,
		 inCREDIT_PRINT_Logo_ByBuffer,
		 inCREDIT_PRINT_Top_ByBuffer,
		 inCREDIT_PRINT_TotalAmount_ByBuffer,
		 inCREDIT_PRINT_TotalAmountByCard_ByBuffer,
		 _NULL_CH_,
		 inCREDIT_PRINT_TotalAmountByInstllment_ByBuffer,
		 inCREDIT_PRINT_TotalAmountByRedemption_ByBuffer,
		 _NULL_CH_,
		 inCREDIT_PRINT_End_ByBuffer
	},

	/* 印ESC結帳條(只有NCCC) */
	{
		 inCREDIT_PRINT_Check_ByBuffer,
		 inCREDIT_PRINT_Logo_ByBuffer,
		 inCREDIT_PRINT_Top_ByBuffer,
		 inCREDIT_PRINT_TotalAmount_ByBuffer,
		 inCREDIT_PRINT_TotalAmountByCard_ByBuffer,
		 inCREDIT_PRINT_TotalAmountByCredit_ByBuffer,
		 inCREDIT_PRINT_TotalAmountByInstllment_ByBuffer,
		 inCREDIT_PRINT_TotalAmountByRedemption_ByBuffer,
		 inCREDIT_PRINT_TotalAmountByOther,
		 inCREDIT_PRINT_End_ByBuffer
	},
};

/* 列印明細報表使用 (START) */
DETAIL_REPORT_TABLE_BYBUFFER srDetailReport_ByBuffer[] =
{
	/* 一般(ex:Diners) */
	{
		inCREDIT_PRINT_Check_ByBuffer,			/* inReportCheck */
		inCREDIT_PRINT_Logo_ByBuffer,			/* inReportLogo */
		inCREDIT_PRINT_Top_ByBuffer,			/* inReportTop */
		inCREDIT_PRINT_TotalAmount_ByBuffer,		/* inTotalAmount */
		inCREDIT_PRINT_DetailReportMiddle_ByBuffer,	/* inMiddle */
		inCREDIT_PRINT_DetailReportBottom_ByBuffer,	/* inBottom */
		inCREDIT_PRINT_End_ByBuffer			/* inReportEnd */
	},
	/* NCCC */
	{
		inCREDIT_PRINT_Check_ByBuffer,			/* inReportCheck */
		inCREDIT_PRINT_Logo_ByBuffer,			/* inReportLogo */
		inCREDIT_PRINT_Top_ByBuffer,			/* inReportTop */
		_NULL_CH_,							/* inTotalAmount */
		inCREDIT_PRINT_NCCC_DetailReportMiddle_ByBuffer,/* inMiddle */
		inCREDIT_PRINT_DetailReportBottomByBufferBigWord,/* inBottom */
		inCREDIT_PRINT_End_ByBuffer			/* inReportEnd */
	},

};

/* 列印電票總額報表使用 (START) */
TOTAL_REPORT_TABLE_BYBUFFER_ESVC srTotalReport_ByBuffer_ESVC[] =
{
	{
		_NULL_CH_,
		inCREDIT_PRINT_Logo_ByBuffer_ESVC,
		inCREDIT_PRINT_Top_ESVC_ByBuffer,
		inCREDIT_PRINT_TotalAmount_ByBuffer_ESVC,
		inCREDIT_PRINT_TotalAmountByCard_ByBuffer_ESVC,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_End_ByBuffer_ESVC
	},

	{
		_NULL_CH_,
		inCREDIT_PRINT_Logo_ByBuffer_ESVC,
		inCREDIT_PRINT_Top_ESVC_SETTLE_ByBuffer,
		inCREDIT_PRINT_TotalAmount_ByBuffer_ESVC_Settle,
		inCREDIT_PRINT_TotalAmountByCard_ByBuffer_ESVC,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_End_ByBuffer_ESVC
	},
};
/* 列印電票總額報表使用 (END) */

/* 列印電票明細報表使用 (START) */
DETAIL_REPORT_TABLE_BYBUFFER_ESVC srDetailReport_ByBuffer_ESVC[] =
{
	{
		inCREDIT_PRINT_Check_ByBuffer_ESVC,		/* inReportCheck */
		inCREDIT_PRINT_Logo_ByBuffer_ESVC,		/* inReportLogo */
		inCREDIT_PRINT_Top_ESVC_ByBuffer,		/* inReportTop */
		inCREDIT_PRINT_TotalAmount_ByBuffer_ESVC,	/* inTotalAmount */
		inCREDIT_PRINT_DetailReportMiddle_ByBuffer_ESVC,/* inMiddle */
		inCREDIT_PRINT_DetailReportBottom_ByBuffer_ESVC,/* inBottom */
		inCREDIT_PRINT_End_ByBuffer_ESVC		/* inReportEnd */
	},
};
/* 列印電票明細報表使用 (END) */

/*
Function        :inCREDIT_PRINT_Receipt_Flow_ByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :列印信用卡
*/
int inCREDIT_PRINT_Receipt_Flow_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
        int			inPrintIndex = 0, inRetVal;
	char			szTRTFileName[12 + 1];
	char			szShort_Receipt_Mode[2 + 1];
	char			szDebugMsg[100 + 1];
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "PRINT_275 inCREDIT_PRINT_Receipt_Flow_ByBuffer INIT" );
	
	inDISP_LogPrintfWithFlag(" CREDIT Print ByBuffer RRN [%s]", pobTran->srBRec.szRefNo );
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		inDISP_LogPrintfWithFlag(" No Print Funcional [%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}
	else
	{
		memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
		inGetTRTFileName(szTRTFileName);

		/* 縮小版帳單 */
		memset(szShort_Receipt_Mode, 0x00, sizeof(szShort_Receipt_Mode));
		inGetShort_Receipt_Mode(szShort_Receipt_Mode);
		if (memcmp(szShort_Receipt_Mode, "Y", strlen("Y")) == 0)
		{
			/* 信用卡DCC */
			if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
			{
				/* For Sale */
				if ((pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_DCC_FOR_SALE_SMALL_;
				}
				/* 信用卡DCC Not For Sale */
				else
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_DCC_NOT_FOR_SALE_SMALL_;
				}

			}
			/* DCC 當筆轉台幣 */
			else if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE && (pobTran->srBRec.inCode == _SALE_OFFLINE_ || pobTran->srBRec.inCode == _PRE_COMP_))
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_DCC_CHANGE_TWD_SMALL_;
			}
			/* 優惠兌換 */
			else if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_LOYALTY_REDEEM_SMALL_;
			}
			/* 優惠兌換取消 */
			else if (pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_VOID_LOYALTY_REDEEM_SMALL_;
			}
			/* 信用卡SmartPay */
			else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_FISC_SMALL_;
			}
			/* Happy GO交易 */
			else if (pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_HG_SINGLE_SMALL_;
			}
			/* Happy GO混合交易 */
			else if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_HG_MULTIPLE_SMALL_;
			}
			/* 電票交易目前沒有縮小簽單，導回同一個 */
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inLoadTDTRec(pobTran->srTRec.inTDTIndex);

				if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ || pobTran->inTransactionCode == _TICKET_EASYCARD_INQUIRY_)
					return (VS_SUCCESS);

				if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_IPASS_;
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_ECC_;
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_ICASH_;
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_HAPPYCASH_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_HAPPYCASH_;
				}
				else
				{
					return (VS_SUCCESS);
				}
			}
			/* 信用卡一般 */
			else
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_SMALL_;
			}
		}
		else
		{
			/* 信用卡DCC */
			if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
			{
				/* For Sale */
				if ((pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_DCC_FOR_SALE_;
				}
				/* 信用卡DCC Not For Sale */
				else
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_DCC_NOT_FOR_SALE_;
				}

			}
			/* DCC 當筆轉台幣 */
			else if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE && (pobTran->srBRec.inCode == _SALE_OFFLINE_ || pobTran->srBRec.inCode == _PRE_COMP_))
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_DCC_CHANGE_TWD_;
			}
			/* 優惠兌換 */
			else if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_LOYALTY_REDEEM_;
			}
			/* 優惠兌換取消 */
			else if (pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_VOID_LOYALTY_REDEEM_;
			}
			/* 信用卡SmartPay */
			else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_FISC_;
			}
			/* Happy GO交易 */
			else if (pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_HG_SINGLE_;
			}
			/* Happy GO混合交易 */
			else if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_HG_MULTIPLE_;
			}
			/* 電票交易 */
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inLoadTDTRec(pobTran->srTRec.inTDTIndex);

				if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ || 
					pobTran->inTransactionCode == _TICKET_EASYCARD_INQUIRY_)
					return (VS_SUCCESS);

				if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_IPASS_;
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_ECC_;
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_ICASH_;
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_HAPPYCASH_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_HAPPYCASH_;
				}
				else
				{
					return (VS_SUCCESS);
				}
			}
			/* 信用卡一般 */
			else
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_;
			}
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, " Receipt ByBuffer PrintIndex : %d", inPrintIndex);
			inDISP_LogPrintf(szDebugMsg);
		}

		while (1)
		{
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
			/* 列印LOGO */
			if (srReceiptType_ByBuffer[inPrintIndex].inLogo != NULL)
				if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			/* 列印TID MID */
			if (srReceiptType_ByBuffer[inPrintIndex].inTop != NULL)
				if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			/* 列印DATA */
			if (srReceiptType_ByBuffer[inPrintIndex].inData != NULL)
				if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inData(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);


			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				/* 列印CUP金額 */
				if (srReceiptType_ByBuffer[inPrintIndex].inCUPAmount != NULL)
					if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inCUPAmount(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
						return (inRetVal);
			}
			else
			{
				/* 列印金額 */
				if (srReceiptType_ByBuffer[inPrintIndex].inAmount != NULL)
					if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inAmount(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
						return (inRetVal);

				/* 分期資料 */
				if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
				{
					if (srReceiptType_ByBuffer[inPrintIndex].inInstallment != NULL)
						if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inInstallment(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
							return (inRetVal);
				}
				/* 紅利資料 */
				else if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
				{
					if (srReceiptType_ByBuffer[inPrintIndex].inRedemption != NULL)
						if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inRedemption(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
							return (inRetVal);
				}

			}

			/* OTHER資料 */
			if (srReceiptType_ByBuffer[inPrintIndex].inOther != NULL)
				if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inOther(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			/* HappyGo */

			/* LoyaltyRedeem */


			/* 列印簽名欄  & 警語 */
			/* 因為有電子簽名圖檔所以個別處理 */
			if (srReceiptType_ByBuffer[inPrintIndex].inEnd != NULL)
				if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "PRINT_275  Prepare Data To Print");
			if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);
			
			inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "PRINT_275  Aft Print Data");
			
			break;
		}
		
		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "PRINT_275 inCREDIT_PRINT_Receipt_Flow_ByBuffer END");
		
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_TotalReport_ByBuffer
Date&Time       :2016/3/7 下午 3:57
Describe        :列印總額帳單
*/
int inCREDIT_PRINT_TotalReport_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = 0, inPrintIndex = 0;
	char			szTRTFileName[12 + 1] = {0};
	char			szDebugMsg[100 + 1] = {0};
	char			szCustomerIndicator[3 + 1] = {0};
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
	ACCUM_TOTAL_REC		srAccumRec;


	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	/* 客製化107邦柏，不列印結帳條 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, strlen(_CUSTOMER_INDICATOR_107_BUMPER_)))
	{
		return (VS_SUCCESS);
	}
	else
	{

		memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
		inGetTRTFileName(szTRTFileName);
		if (memcmp(szTRTFileName, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)) == 0)
		{
			inPrintIndex = _TOTAL_REPORT_INDEX_NCCC_;
		}
		else
		{
			inPrintIndex = _TOTAL_REPORT_INDEX_NORMAL_;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
			inDISP_LogPrintf(szDebugMsg);
		}

		/* 開啟交易總的檔案 */
		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		inRetVal = inACCUM_GetRecord(pobTran, &srAccumRec);

		if (inRetVal == VS_NO_RECORD)
		{
			memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		}
		else if (inRetVal == VS_ERROR)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("Get record 失敗.");
			}

			return (VS_ERROR);
		}

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

		while (1)
		{
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
			/* 列印LOGO */
			if (srTotalReport_ByBuffer[inPrintIndex].inReportLogo != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 列印TID MID */
			if (srTotalReport_ByBuffer[inPrintIndex].inReportTop != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 全部金額總計 */
			if (srTotalReport_ByBuffer[inPrintIndex].inAmount != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmount(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			/* 有金額才印 */
			if (srAccumRec.llTotalSaleAmount != 0L || srAccumRec.llTotalRefundAmount != 0L)
			{
				/* 卡別金額總計 */
				if (srTotalReport_ByBuffer[inPrintIndex].inAmountByCard != NULL)
					if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmountByCard(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
						return (inRetVal);

				/* 有分期記錄才印 */
				if (srAccumRec.lnInstTotalCount > 0)
				{
					/* 分期金額總計 */
					if (srTotalReport_ByBuffer[inPrintIndex].inAmountByInstallment != NULL)
						if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmountByInstallment(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
							return (inRetVal);
				}

				/* 有紅利記錄才印 */
				if (srAccumRec.lnRedeemTotalCount > 0)
				{
					/* 紅利金額總計 */
					if (srTotalReport_ByBuffer[inPrintIndex].inAmountByRedemption != NULL)
						if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmountByRedemption(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
							return (inRetVal);
				}

			}

			/* 結束 */
			if (srTotalReport_ByBuffer[inPrintIndex].inReportEnd != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);
			break;
		}

		return (VS_SUCCESS);
	}
}


/*
Function        :inCREDIT_PRINT_TotalReport_ByBuffer_NCCC
Date&Time       :2017/3/7 下午 5:00
Describe        :列印總額帳單
*/
int inCREDIT_PRINT_TotalReport_ByBuffer_NCCC(TRANSACTION_OBJECT *pobTran)
{
	//int			inOrgIndex = -1;
	//int			inHGIndex = -1;
	int			inRetVal = 0, inPrintIndex = 0;
	//char			szTemplate[16 + 1] = {0};
	char			szDebugMsg[100 + 1] = {0};
	//char			szFuncEnable[2 + 1] = {0};
	char			szCustomerIndicator[3 + 1] = {0};
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
	ACCUM_TOTAL_REC		srAccumRec;
	//HG_ACCUM_TOTAL_REC	srHGAccumRec;

	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	/* 客製化107邦柏，不列印結帳條 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, strlen(_CUSTOMER_INDICATOR_107_BUMPER_)))
	{
		return (VS_SUCCESS);
	}
	else
	{

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

		inPrintIndex = _TOTAL_REPORT_INDEX_NCCC_;

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
			inDISP_LogPrintf(szDebugMsg);
		}

		/* 開啟交易總的檔案 */
		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		inRetVal = inACCUM_GetRecord(pobTran, &srAccumRec);

		if (inRetVal == VS_NO_RECORD)
			memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		else if (inRetVal == VS_ERROR)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("Get record 失敗.");
			}

			return (VS_ERROR);
		}

	
		while (1)
		{
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
			/* 列印LOGO */
			if (srTotalReport_ByBuffer[inPrintIndex].inReportLogo != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 列印TID MID */
			if (srTotalReport_ByBuffer[inPrintIndex].inReportTop != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 全部金額總計 */
			if (srTotalReport_ByBuffer[inPrintIndex].inAmount != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmount(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			/* 有金額才印 */
			//if (srAccumRec.llTotalSaleAmount != 0L || srAccumRec.llTotalRefundAmount != 0L)
			{
				/* 卡別金額總計 */
				if (srTotalReport_ByBuffer[inPrintIndex].inAmountByCard != NULL)
					if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmountByCard(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
						return (inRetVal);

				/* 一般交易總額報表 */
				if (srTotalReport_ByBuffer[inPrintIndex].inAmountByCredit != NULL)
					if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmountByCredit(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
						return (inRetVal);

				/* 有分期記錄才印 */
				//if (srAccumRec.lnInstTotalCount > 0)
				{
					/* 分期金額總計 */
					if (srTotalReport_ByBuffer[inPrintIndex].inAmountByInstallment != NULL)
						if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmountByInstallment(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
							return (inRetVal);
				}

				/* 有紅利記錄才印 */
				//if (srAccumRec.lnRedeemTotalCount > 0)
				{
					/* 紅利金額總計 */
					if (srTotalReport_ByBuffer[inPrintIndex].inAmountByRedemption != NULL)
						if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmountByRedemption(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
							return (inRetVal);
				}

			}

			/* 上傳電子簽單至ESC系統 */
			if (srTotalReport_ByBuffer[inPrintIndex].inAmountByOther != NULL)
				srTotalReport_ByBuffer[inPrintIndex].inAmountByOther(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1);

//			/* HappyGo */
//			inHG_GetHG_Enable(pobTran->srBRec.inHDTIndex, szFuncEnable);
//			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
//			{
//				/* 印HG總額 */
//				inCREDIT_PRINT_Total_HG_ByBuffer(pobTran, &srHGAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1);
//			}

			/* 是否印優惠兌換 */
//			inLoadASMCRec(0);
//			memset(szTemplate, 0x00, sizeof(szTemplate));
//			inGetASMFlag(szTemplate);
//			if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
//			{
//				inRetVal = inCREDIT_PRINT_Total_Loyalty_Redeem_ByBuffer(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1);
//				if (inRetVal != VS_SUCCESS)
//				{
//					return (inRetVal);
//				}
//
//			}

			/* 結束 */
			if (srTotalReport_ByBuffer[inPrintIndex].inReportEnd != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);
			break;
		}

		return (VS_SUCCESS);
	}
}


/*
Function        :inCREDIT_PRINT_DetailReport_ByBuffer
Date&Time       :2016/3/7 下午 4:08
Describe        :列印明細帳單
*/
int inCREDIT_PRINT_DetailReport_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = 0, inPrintIndex = 0, inRecordCnt = 0;
	char			szTRTFileName[16 + 1];
	char			szDebugMsg[100 + 1];
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
	ACCUM_TOTAL_REC		srAccumRec;

	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

		memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
		inGetTRTFileName(szTRTFileName);
		if (memcmp(szTRTFileName, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)) == 0)
		{
			inPrintIndex = _DETAIL_REPORT_INDEX_DCC_;
		}
		else if (memcmp(szTRTFileName, _TRT_FILE_NAME_HG_, strlen(_TRT_FILE_NAME_HG_)) == 0)
		{
			inPrintIndex = _DETAIL_REPORT_INDEX_HG_;
		}
		else
		{
			inPrintIndex = _DETAIL_REPORT_INDEX_NORMAL_;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
			inDISP_LogPrintf(szDebugMsg);
		}

		/* 檢查是否有帳 */
		if (srDetailReport_ByBuffer[inPrintIndex].inReportCheck != NULL)
		{
			if ((inRecordCnt = srDetailReport_ByBuffer[inPrintIndex].inReportCheck(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) == VS_ERROR)
				return (VS_ERROR); /* 表示檔案開啟失敗 */
		}

		/* 開啟交易總的檔案 */
		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		inRetVal = inACCUM_GetRecord(pobTran, &srAccumRec);

		if (inRetVal == VS_NO_RECORD)
			memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		else if (inRetVal == VS_ERROR)
		{
			inDISP_LogPrintf("Get record 失敗.");

			return (VS_ERROR);
		}

		while (1)
		{
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
			/* 列印LOGO */
			if (srDetailReport_ByBuffer[inPrintIndex].inReportLogo != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 列印TID MID */
			if (srDetailReport_ByBuffer[inPrintIndex].inReportTop != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 全部金額總計 */
			if (srDetailReport_ByBuffer[inPrintIndex].inTotalAmount != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inTotalAmount(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			/* 明細規格 */
			if (srDetailReport_ByBuffer[inPrintIndex].inMiddle != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inMiddle(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 明細資料 */
			if (srDetailReport_ByBuffer[inPrintIndex].inBottom != NULL)
			{
				inRetVal = srDetailReport_ByBuffer[inPrintIndex].inBottom(pobTran, inRecordCnt, uszBuffer1, &srFont_Attrib1, &srBhandle1);
				if (inRetVal != VS_SUCCESS	&&
				    inRetVal != VS_NO_RECORD)
				{
					return (inRetVal);
				}
			}

			/* 結束 */
			if (srDetailReport_ByBuffer[inPrintIndex].inReportEnd != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inReportEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);

			break;
		}

		return (VS_SUCCESS);
	}
}




/*
Function        :inCREDIT_PRINT_DetailReport_ByBuffer
Date&Time       :2016/3/7 下午 4:08
Describe        :列印明細帳單
*/
int inCREDIT_PRINT_NCCC_DetailReport_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = 0, inPrintIndex = 0, inRecordCnt = 0;
	char	szTemplate[16 + 1];
	char	szDebugMsg[100 + 1];
	unsigned char	uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle	srBhandle1;
	FONT_ATTRIB	srFont_Attrib1;
	ACCUM_TOTAL_REC	srAccumRec;

	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

		inPrintIndex = _DETAIL_REPORT_INDEX_NCCC_;

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
			inDISP_LogPrintf(szDebugMsg);
		}

		/* 檢查是否有帳 */
		if (srDetailReport_ByBuffer[inPrintIndex].inReportCheck != NULL)
		{
			if ((inRecordCnt = srDetailReport_ByBuffer[inPrintIndex].inReportCheck(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) == VS_ERROR)
				return (VS_ERROR); /* 表示檔案開啟失敗 */
		}

		/* 開啟交易總的檔案 */
		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		inRetVal = inACCUM_GetRecord(pobTran, &srAccumRec);

		if (inRetVal == VS_NO_RECORD)
			memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		else if (inRetVal == VS_ERROR)
		{
			inDISP_LogPrintf("Get record 失敗.");

			return (VS_ERROR);
		}

		while (1)
		{
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
			/* 列印LOGO */
			if (srDetailReport_ByBuffer[inPrintIndex].inReportLogo != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 列印TID MID */
			if (srDetailReport_ByBuffer[inPrintIndex].inReportTop != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 全部金額總計 */
			if (srDetailReport_ByBuffer[inPrintIndex].inTotalAmount != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inTotalAmount(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			/* 明細規格 */
			if (srDetailReport_ByBuffer[inPrintIndex].inMiddle != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inMiddle(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 明細資料 */
			if (srDetailReport_ByBuffer[inPrintIndex].inBottom != NULL)
			{
				inRetVal = srDetailReport_ByBuffer[inPrintIndex].inBottom(pobTran, inRecordCnt, uszBuffer1, &srFont_Attrib1, &srBhandle1);
				if (inRetVal != VS_SUCCESS &&
				    inRetVal != VS_NO_RECORD)
				{
				       return (inRetVal);
				}
			}

			/* 是否印優惠兌換 */
			inLoadASMCRec(0);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetASMFlag(szTemplate);
			if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
			{
				inRetVal = inCREDIT_PRINT_Total_Loyalty_Redeem_ByBuffer(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1);
				if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}

			}

			/* 結束 */
			if (srDetailReport_ByBuffer[inPrintIndex].inReportEnd != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inReportEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);

			break;
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_Schedule_ByBuffer
Date&Time       :2017/2/23 下午 5:16
Describe        :列印重要訊息通知
*/
int inCREDIT_PRINT_Schedule_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inBMPHeight = 0;

	if (inPRINT_Buffer_GetHeight((unsigned char*)"IMPORTANT.bmp", &inBMPHeight) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

        /* 列印重要訊息通知 */
        if (inPRINT_Buffer_PutGraphic((unsigned char*)_TMS_SCHEDULE_IMPORTANT_, uszBuffer, srBhandle, inBMPHeight, _APPEND_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_LOGO_ByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :列印LOGO
*/
int inCREDIT_PRINT_Logo_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	/* 印Slogan 384*180 */
	if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
	{
		if (inCREDIT_PRINT_MarchantSlogan(pobTran, _NCCC_SLOGAN_PRINT_UP_, uszBuffer, srBhandle) != VS_SUCCESS)
			return (VS_ERROR);
	}

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
/* 因為公司的TMS不能下商店名稱圖檔，所以要改用BIG5檔列印 20190213 [SAM] */
#if 1
	/* 印商店名稱 */
        if (inCREDIT_PRINT_MerchantNameTXT(pobTran, uszBuffer, srFont_Attrib, srBhandle) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }
	
#else
	/* 印商店名稱 */
        if (inCREDIT_PRINT_MerchantName(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }
#endif
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_MarchantSlogan
Date&Time       :2016/9/7 下午 5:17
Describe        :用來決定要不要印slogan的
*/
int inCREDIT_PRINT_MarchantSlogan(TRANSACTION_OBJECT *pobTran, int inPrintPosition, unsigned char *uszBuffer, BufferHandle *srBhandle)
{
	int	inDateNow, inDateStart, inDateEnd;
	char	szSloganEnable[2 + 1];
	char	szSloganPrtPosition[1 + 1];
	char	szDateNow[8 + 1], szDateStart[8 + 1], szDateEnd[8 + 1];

	/* SloganEnable沒On代表不用印Slogan */
	memset(szSloganEnable, 0x00, sizeof(szSloganEnable));
	inGetPrtSlogan(szSloganEnable);
	if(memcmp(szSloganEnable, "N", 1) == 0)
	{
		return (VS_SUCCESS);
	}

	/* 列印位置不對就跳過*/
	memset(szSloganPrtPosition, 0x00, sizeof(szSloganPrtPosition));
	inGetSloganPrtPosition(szSloganPrtPosition);
	if (inPrintPosition != atoi(szSloganPrtPosition))
		return (VS_SUCCESS);

	/* 算出目前的日期 */
	memset(szDateNow, 0x00, sizeof(szDateNow));
	inDateNow = atoi(pobTran->srBRec.szDate);

	/* 算出起始日期(起始日含此日) */
	memset(szDateStart, 0x00, sizeof(szDateStart));
	inGetSloganStartDate(szDateStart);
	inDateStart = atoi(szDateStart);

	/* 商店活動日期未到不用印 */
	if (inDateNow < inDateStart)
		return (VS_SUCCESS);

	/* 算出結束日期(結束日不含此日) */
	memset(szDateEnd, 0x00, sizeof(szDateEnd));
	inGetSloganEndDate(szDateEnd);
	inDateEnd = atoi(szDateEnd);

	/* 商店活動日期過期不用印 */
	if (inDateNow >= inDateEnd)
		return (VS_SUCCESS);

	if (inPRINT_Buffer_PutGraphic((unsigned char*)_SLOGAN_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inSloganHeight, _APPEND_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_MerchantLogo
Date&Time       :2016/9/7 下午 5:55
Describe        :用來決定要不要印商店LOGO
*/
int inCREDIT_PRINT_MerchantLogo(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, BufferHandle *srBhandle)
{
	char	szPrtMerchantLogo[1 + 1];

	/* PrtMerchantLogo沒On代表不用印商店Logo */
	memset(szPrtMerchantLogo, 0x00, sizeof(szPrtMerchantLogo));
	inGetPrtMerchantLogo(szPrtMerchantLogo);
	if(memcmp(szPrtMerchantLogo, "N", 1) == 0)
	{
		return (VS_SUCCESS);
	}

	if (inPRINT_Buffer_PutGraphic((unsigned char*)_MERCHANT_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inMerchantLogoHeight, _APPEND_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_MerchantName
Date&Time       :2016/9/7 下午 5:55
Describe        :用來決定要不要印商店表頭
*/
int inCREDIT_PRINT_MerchantName(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, BufferHandle *srBhandle)
{
	char	szPrtMerchantName[1 + 1];

	/* PrtMerchantName沒On代表不用印商店表頭 */
	memset(szPrtMerchantName, 0x00, sizeof(szPrtMerchantName));
	inGetPrtMerchantName(szPrtMerchantName);
	if(memcmp(szPrtMerchantName, "N", 1) == 0)
	{
		return (VS_SUCCESS);
	}

	if (inPRINT_Buffer_PutGraphic((unsigned char*)_NAME_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inTitleNameHeight, _APPEND_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

	return (VS_SUCCESS);
}

/*
Function        : inCREDIT_PRINT_MerchantNameTXT
Date&Time   : 2016/9/7 下午 5:55
Describe        : 用來決定要不要印商店表頭
*/
int inCREDIT_PRINT_MerchantNameTXT(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szDebuglog[100];
	char	szPrintBuf[100];
	char	szPrtMerchantName[1 + 1];
        int     inRetVal = VS_ERROR;
        int     i = 0 ,j = 0;
        int             inSearchResult = 0;		/* 判斷有沒有讀到0x0D 0x0A的Flag */
        unsigned char   *uszReadData;
        unsigned char   *uszTemp;			/* 暫存，放整筆EDC檔案 */
        unsigned long   ulFile_Handle;
        long            lnNameLength = 0;
	long		lnReadLength = 0;
        
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inEDC_PRINT_MerchantNameTxt() START!!");

        /* PrtMerchantName沒On代表不用印商店表頭 */
	memset(szPrtMerchantName, 0x00, sizeof(szPrtMerchantName));
	inGetPrtMerchantName(szPrtMerchantName);
	if (memcmp(szPrtMerchantName, "N", 1) == 0)
	{
		return (VS_SUCCESS);
	}
	
        /* 開啟Big5Name.txt */
        inRetVal = inFILE_Open(&ulFile_Handle, (unsigned char *)"Big5Name.txt");

        if (inRetVal != VS_SUCCESS)
        {   
                if (ginDebug == VS_TRUE)
                        inDISP_LogPrintf("inFILE_Open()_ERROR");

                return (VS_SUCCESS);
        }

        /* 取得檔案大小 */
        lnNameLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)"Big5Name.txt");

        /* 取得檔案大小失敗 */
        if (lnNameLength == VS_ERROR)
        {
                inFILE_Close(&ulFile_Handle);

                if (ginDebug == VS_TRUE)
                        inDISP_LogPrintf("lnFILE_GetSize()_ERROR");

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnNameLength + 1);
	uszTemp = malloc(lnNameLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnNameLength + 1);
	memset(uszTemp, 0x00, lnNameLength + 1);
	
        /* seek 到檔案開頭 & 從檔案開頭開始read */

        if (ginDebug == VS_TRUE)
        {        
                memset(szDebuglog, 0x00, sizeof(szDebuglog));
                sprintf(szDebuglog, "Big5Name Size = [%ld]", lnNameLength);
                inDISP_LogPrintf(szDebuglog);
        }
                        
        /* 讀檔案 */
        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnNameLength;

                for (i = 0;; ++i)
                {
                        /* 剩餘長度大於或等於1024 */
                        if (lnReadLength >= 1024)
                        {
                                if (inFILE_Read(&ulFile_Handle, &uszTemp[1024*i], 1024) == VS_SUCCESS)
                                {
                                        /* 一次讀1024 */
                                        lnReadLength -= 1024;

                                        /* 當剩餘長度剛好為1024，會剛好讀完 */
                                        if (lnReadLength == 0)
                                                break;
                                }
                                /* 讀失敗時 */
                                else
                                {
                                        /* Close檔案 */
                                        inFILE_Close(&ulFile_Handle);

                                        /* Free pointer */
                                        free(uszReadData);
                                        free(uszTemp);

                                        return (VS_ERROR);
                                }
                        }
                        /* 剩餘長度小於1024 */
                        else if (lnReadLength < 1024)
                        {
                                /* 就只讀剩餘長度 */
                                if (inFILE_Read(&ulFile_Handle, &uszTemp[1024*i], lnReadLength) == VS_SUCCESS)
                                {
                                        break;
                                }
                                /* 讀失敗時 */
                                else
                                {
                                        /* Close檔案 */
                                        inFILE_Close(&ulFile_Handle);

                                        /* Free pointer */
                                        free(uszReadData);
                                        free(uszTemp);

                                        return (VS_ERROR);
                                }
                        }
                } /* end for loop */
        }
        /* seek不成功時 */
        else
        {
                /* 關檔並回傳 */
                inFILE_Close(&ulFile_Handle);
                /* Free pointer */
                free(uszReadData);
                free(uszTemp);
		
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("inFILE_Seek()_ERROR");

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        inFILE_Close(&ulFile_Handle);

	
	/*
         *抓取所需要的那筆record
         *i為目前從EDC讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnNameLength || inSearchResult < 3; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
		if (ginDebug == VS_TRUE)
		{	
			memset(szDebuglog, 0x00, sizeof(szDebuglog));
			sprintf(szDebuglog, "Big5Name Data [%d][%d][%d][%02x]", i, inSearchResult, j, uszTemp[i]);
			inDISP_LogPrintf(szDebuglog);
		}
			
                /* 讀完一筆record或讀到EDC的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult ++;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnNameLength + 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j+3], (j-3));
			inFunc_Big5toUTF8(szPrintBuf, (char *)uszReadData);
			if (strlen(szPrintBuf) > 0)
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			//inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
                        
			/* 為了跳過 0x0D 0x0A */
                        i = i + 2;
                        /* 每讀完一筆record，j就歸0 */
                        j = 0;
                }

                j ++;
        }
	
        free(uszTemp);
        free(uszReadData);   
        
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inEDC_PRINT_MerchantNameTxt() END!!");
        
        return (VS_SUCCESS);
}


/*
Function        :inCREDIT_PRINT_Notice
Date&Time       :2016/9/13 上午 11:40
Describe        :用來決定要不要印商店提示
*/
int inCREDIT_PRINT_Notice(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, BufferHandle *srBhandle)
{
	char	szPrtNotice[1 + 1];

	/* szPrtNotice沒On代表不用印商店提示 */
	memset(szPrtNotice, 0x00, sizeof(szPrtNotice));
	inGetPrtNotice(szPrtNotice);
	if(memcmp(szPrtNotice, "N", 1) == 0)
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
Function        :inCREDIT_PRINT_TIDMID_ByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :列印TID & MID
*/
int inCREDIT_PRINT_Tidmid_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        int     inRetVal;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

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
        }
        else
        {
                /* 橫式 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetMerchantID(szTemplate);

                /* 列印商店代號 */
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PAD_RIGHT_FILL_LEFT_);
                sprintf(szPrintBuf, "商店代號 %s", szTemplate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* Get端末機代號 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetTerminalID(szTemplate);

                /* 列印端末機代號 */
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 13, _PAD_RIGHT_FILL_LEFT_);
                sprintf(szPrintBuf, "端末機代號 %s", szTemplate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
        }

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_Data_ByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :列印DATA
*/
int inCREDIT_PRINT_Data_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
	int     inRetVal;
	char 	szPrintBuf[84 + 1], szPrintBuf1[42 + 1], szPrintBuf2[42 + 1], szTemplate1[42 + 1], szTemplate2[42 + 1];
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	if (inPrinttype_ByBuffer)
	{
		/* 直式 */
		/*卡別、卡號*/
		sprintf(szPrintBuf, "卡別　　：%s", pobTran->srBRec.szCardLabel);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "卡號　　：%s", pobTran->srBRec.szPAN);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/*日期、時間*/
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "日期　　：%s",pobTran->srBRec.szDate);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "時間　　：%s",pobTran->srBRec.szTime);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/*調閱編號、批次號碼 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "調閱編號：%06ld",pobTran->srBRec.lnOrgInvNum);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "批次號碼：%06ld",pobTran->srBRec.lnBatchNum);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/*交易類別*/
		vdEDC_GetTransType(pobTran, szPrintBuf1, szTemplate2);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "交易類別：%s",szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/*授權碼、序號*/
		sprintf(szPrintBuf, "授權碼　：%s",pobTran->srBRec.szAuthCode);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "序號　　：%s",pobTran->srBRec.szRefNo);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	}
	else
	{
		/* 橫式 */
		/* 卡別 檢查碼 */
		inRetVal = inPRINT_Buffer_PutIn("卡別(Card Type)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("檢查碼(Check No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		/* 卡別 */
		inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 12, _PAD_LEFT_FILL_RIGHT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		/* 檢查碼 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		inCARD_ExpDateEncryptAndDecrypt(pobTran, szTemplate1, szTemplate1, _EXP_ENCRYPT_);
		memcpy(szPrintBuf1, szTemplate1, strlen(szTemplate1));
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 卡號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, pobTran->srBRec.szPAN);
		inRetVal = inPRINT_Buffer_PutIn("卡號(Card No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, pobTran->srBRec.szPAN);

		if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
		{
			inFunc_FormatPAN_UCARD(szPrintBuf);
		}

		/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
		if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
			{
				for (i = 3; i < (strlen(szPrintBuf) - 5); i ++)
					szPrintBuf[i] = 0x2A;
			}
			else
			{
				for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
					szPrintBuf[i] = 0x2A;
			}

		}
		else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
		{
			/* 商店聯卡號遮掩 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
			{
				if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
				{
					for (i = 3; i < (strlen(szPrintBuf) - 5); i ++)
						szPrintBuf[i] = 0x2A;
				}
				else
				{
					for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
						szPrintBuf[i] = 0x2A;
				}
			}

		}

		/* 過卡方式 */
		if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				strcat(szPrintBuf,"(C)");
			else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
				strcat(szPrintBuf, "(T)");
			else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf, "(W)");
			else if (pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
				strcat(szPrintBuf, "(F)");
			else
			{
				if (pobTran->srBRec.uszManualBit == VS_TRUE)
				{
					/* 【需求單-105244】端末設備支援以感應方式進行退貨交易 */
					/* 電文轉Manual Keyin但是簽單要印感應的W */
					if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
						strcat(szPrintBuf, "(W)");
					else
						strcat(szPrintBuf,"(M)");
				}
				else
					strcat(szPrintBuf,"(S)");
			}

		}
		else
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf, "(W)");
			else
				strcat(szPrintBuf, "(C)");
		}

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 主機別 & 交易別 */
		inRetVal = inPRINT_Buffer_PutIn("交易類別(Trans.Type)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		//inRetVal = inPRINT_Buffer_PutIn("主機別/交易類別(Host/Trans.Type)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

	#if 0//By Ray
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		inGetHostLabel(szTemplate1);
		sprintf(szPrintBuf1, "%s", szTemplate1);
	#endif

		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		vdEDC_GetTransType(pobTran, szTemplate1, szTemplate2);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));

		sprintf(szPrintBuf2, "%s", szTemplate1);
		sprintf(szPrintBuf, "%s", szPrintBuf2);
		//sprintf(szPrintBuf, "%s %s", szPrintBuf1 , szPrintBuf2);

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		if (strlen(szTemplate2) > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate2);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}

		/* 批次號碼、授權碼 */
		inRetVal = inPRINT_Buffer_PutIn("批次號碼(Batch No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("授權碼(Auth Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* Batch Num */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%06ld", pobTran->srBRec.lnBatchNum);
		//sprintf(szPrintBuf, "%03ld", pobTran->srBRec.lnBatchNum);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* Auth Code */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		memcpy(szPrintBuf1, pobTran->srBRec.szAuthCode, _AUTH_CODE_SIZE_);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 回覆碼 */
		/*if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			inRetVal = inPRINT_Buffer_PutIn("回覆碼(Resp. Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memcpy(szPrintBuf, pobTran->srBRec.szRespCode, 2);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}*/

		/* 日期時間 */
		inRetVal = inPRINT_Buffer_PutIn("日期/時間(Date/Time)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);

		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 序號 調閱編號 */
		inRetVal = inPRINT_Buffer_PutIn("調閱編號(Inv.No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		//inRetVal = inPRINT_Buffer_PutIn("序號(Ref. No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("序號(Ref. No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		//inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("調閱編號(Inv.No)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);


		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		/* 雖然電文RRN送12個byte，但RRN最後一碼是0x00，所以只看到11碼 */
		sprintf(szPrintBuf, "%06ld", pobTran->srBRec.lnOrgInvNum);
		//inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szRefNo, ' ', 12, _PAD_LEFT_FILL_RIGHT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		
		inDISP_LogPrintfWithFlag("  Final Print ByBuffer RRN [%s]", pobTran->srBRec.szRefNo );
		
		inFunc_PAD_ASCII(szPrintBuf1, pobTran->srBRec.szRefNo, ' ', 12, _PAD_LEFT_FILL_RIGHT_);
		
		inDISP_LogPrintfWithFlag("  Final Aft PAD RRN [%s]", szPrintBuf1 );
		
		//sprintf(szPrintBuf1, "%06ld", pobTran->srBRec.lnOrgInvNum);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* CUP交易序號 */
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			inRetVal = inPRINT_Buffer_PutIn("CUP交易序號(CUP STAN)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memcpy(szPrintBuf, pobTran->srBRec.szCUP_TN, strlen(pobTran->srBRec.szCUP_TN));
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}

		/* 櫃號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetStoreIDEnable(szTemplate1);
		if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
		{
			inRetVal = inPRINT_Buffer_PutIn("櫃號(Store ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szStoreID, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}

		/* 產品代碼 */
		inGetProductCodeEnable(szProductCodeEnable);
		if (memcmp(szProductCodeEnable, "Y", 1) == 0)
		{
			inRetVal = inPRINT_Buffer_PutIn("產品代碼(Product Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szProductCode, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}

		/* TC */
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_ || pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			/* 感應磁條 */
			if (!memcmp(pobTran->srBRec.szAuthCode, "VLP", 3)				||
			!memcmp(pobTran->srBRec.szAuthCode, "JCB", 3)				||
			pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE	||
			pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD		||
			pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
			{
				/* 商店聯卡號遮掩 */
				memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
				inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
				if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&&
				pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
				strlen(pobTran->srBRec.szTxnNo) > 0)
				{
					inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
			}
			else
			{
				if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
				{
					if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
					{
						/* 晶片碼(TC) */
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						strcpy(szPrintBuf, pobTran->srBRec.szPAN);
						inRetVal = inPRINT_Buffer_PutIn("晶片碼(TC)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%02X%02X%02X%02X%02X%02X%02X%02X",
						//sprintf(szPrintBuf, "TC:%02X%02X%02X%02X%02X%02X%02X%02X",
							pobTran->srEMVRec.usz9F26_ApplCryptogram[0],
							pobTran->srEMVRec.usz9F26_ApplCryptogram[1],
							pobTran->srEMVRec.usz9F26_ApplCryptogram[2],
							pobTran->srEMVRec.usz9F26_ApplCryptogram[3],
							pobTran->srEMVRec.usz9F26_ApplCryptogram[4],
							pobTran->srEMVRec.usz9F26_ApplCryptogram[5],
							pobTran->srEMVRec.usz9F26_ApplCryptogram[6],
							pobTran->srEMVRec.usz9F26_ApplCryptogram[7]);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);

					}

					/* 商店聯卡號遮掩 */
					memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
					inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
					if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&&
						pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
						strlen(pobTran->srBRec.szTxnNo) > 0)
					{
						inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}

				#if 0//BY Ray
					/* M/C交易列印AP Lable (START) */
					if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AP Label:");
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
					}
					/* M/C交易列印AP Lable (END) */
				#endif

				#if 0
					/* AID */
					/* 只有CUP晶片才要印 */
					if (pobTran->srBRec.uszCUPTransBit == VS_TRUE && pobTran->srBRec.inChipStatus == _EMV_CARD_ && strlen(pobTran->srBRec.szCUP_EMVAID) > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
					/* 銀聯閃付 */
					else
					if (pobTran->srBRec.uszCUPTransBit == VS_TRUE && pobTran->srBRec.uszContactlessBit == VS_TRUE && strlen(pobTran->srBRec.szCUP_EMVAID) > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
				#endif
				}
				else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
				{
					/* 商店聯卡號遮掩 */
					/* 持卡人存根也要印 */
					memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
					inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
					if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&&
						pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
						strlen(pobTran->srBRec.szTxnNo) > 0)
					{
						inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
				}
			}
		}
		else
		{
			/* 商店聯卡號遮掩 */
			/* 磁條卡列印交易編號 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&&
				pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
				strlen(pobTran->srBRec.szTxnNo) > 0)
			{
				inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
		}

		/* 斷行 */
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_CUP_AMOUNT_ByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :列印銀聯AMOUNT
*/
int inCREDIT_PRINT_Cup_Amount_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _CUP_REFUND_) ||
		     pobTran->srBRec.inCode == _CUP_REFUND_	||
		     pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", 0 - pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PAD_RIGHT_FILL_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                else if (pobTran->srBRec.inCode == _ADJUST_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnAdjustTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PAD_RIGHT_FILL_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                else
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PAD_RIGHT_FILL_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		for (i = 0; i < 2; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
        }
        /* 橫式 */
        /* 負向交易 */
	else if(pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		/* 橫式 */
                /* 金額 */
                /* 取消退貨是正數 */
                if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
		    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, _PAD_RIGHT_FILL_LEFT_);

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "總計(Total) :NT$%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		else
		{
			/* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, _PAD_RIGHT_FILL_LEFT_);

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "總計(Total) :NT$%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}
	else
	{
		/* 初始化 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
		if (pobTran->srBRec.inCode == _CUP_REFUND_	||
		    pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
		{
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
		}
		else
		{
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		}

		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, _PAD_RIGHT_FILL_LEFT_);

		/* 把前面的字串和數字結合起來 */
		sprintf(szPrintBuf, "總計(Total) :NT$%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	}

	/* 斷行 */
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 列印銀聯交易提示文字 */
	inPRINT_Buffer_PutGraphic((unsigned char*)_CUP_LEGAL_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inCupLegalHeight, _APPEND_);

	/* 斷行 */
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_AMOUNT_ByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :列印AMOUNT
*/
int inCREDIT_PRINT_Amount_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
	char    szPrintBuf[84 + 1], szTemplate[42 + 1];

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	if (inPrinttype_ByBuffer)
	{
		/* 直式 */
		/* 金額 */
		if(pobTran->srBRec.inCode == _TIP_)
		{
			/* 金額 */
			/* 初始化 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTxnAmount);
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PAD_RIGHT_FILL_LEFT_ );

			/* 把前面的字串和數字結合起來 */
			sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


			/* 小費 */
			/* 初始化 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTipTxnAmount);
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PAD_RIGHT_FILL_LEFT_);

			/* 把前面的字串和數字結合起來 */
			sprintf(szPrintBuf, "小費(Tips)  :%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


			/* 總計 */
			/* 初始化 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "NT$ %ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ' , 14, _PAD_RIGHT_FILL_LEFT_ );

			/* 把前面的字串和數字結合起來 */
			sprintf(szPrintBuf,"總計(Total) :%s", szTemplate);
		}
		else if ((pobTran->srBRec.uszVOIDBit == VS_TRUE	&&
				(pobTran->srBRec.inOrgCode != _REFUND_ && pobTran->srBRec.inOrgCode != _INST_REFUND_ && pobTran->srBRec.inOrgCode != _REDEEM_REFUND_)) ||
				pobTran->srBRec.inCode == _REFUND_		||
				pobTran->srBRec.inCode == _INST_REFUND_	||
				pobTran->srBRec.inCode == _REDEEM_REFUND_)
		{
			/* 初始化 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "NT$ %ld", 0 - pobTran->srBRec.lnTxnAmount);
			inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PAD_RIGHT_FILL_LEFT_ );

			/* 把前面的字串和數字結合起來 */
			sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
		}
		else if (pobTran->srBRec.inCode == _ADJUST_)
		{
			/* 初始化 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnAdjustTxnAmount);
			inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PAD_RIGHT_FILL_LEFT_ );

			/* 把前面的字串和數字結合起來 */
			sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
		}
		else
		{
			/* 初始化 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTxnAmount);
			inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PAD_RIGHT_FILL_LEFT_ );

			/* 把前面的字串和數字結合起來 */
			sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
		}
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		for (i = 0; i < 2; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}else
	{/* 橫式 */
		/* 負向交易 */
		if(pobTran->srBRec.uszVOIDBit == VS_TRUE)
		{
			/* 金額 */
			/* 取消退貨是正數 */
			if (pobTran->srBRec.inOrgCode == _REFUND_ || pobTran->srBRec.inOrgCode == _INST_REFUND_ || 
			     pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, VS_TRUE);

				/* 把前面的字串和數字結合起來 */
				sprintf(szPrintBuf, "金額(Amount):NT$%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, VS_TRUE);

				/* 把前面的字串和數字結合起來 */
				sprintf(szPrintBuf, "金額(Amount):NT$%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
		else
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetTransFunc(szTemplate);
			if (szTemplate[6] == 0x31) /* 修改TMS交易開關位置 20190211 [SAM] */
			{
				/* 退貨金額為負數 */
				if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
				{
					/* 初始化 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, VS_TRUE);

					/* 把前面的字串和數字結合起來 */
					sprintf(szPrintBuf, "金額(Amount):NT$%s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				/* 預授不會有小費，所以拉出來 */
				else if (pobTran->srBRec.inCode == _PRE_AUTH_ || pobTran->srBRec.inCode == _PRE_COMP_)
				{
					/* 初始化 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, VS_TRUE);

					/* 把前面的字串和數字結合起來 */
					sprintf(szPrintBuf, "金額(Amount):NT$%s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				else
				{
					/* 金額 */
					/* 初始化 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					if(pobTran->srBRec.inCode == _ADJUST_)
						sprintf(szTemplate, "%ld",  pobTran->srBRec.lnAdjustTxnAmount);
					else
						sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, VS_TRUE);

					/* 把前面的字串和數字結合起來 */
					sprintf(szPrintBuf, "金額(Amount):NT$%s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					if(pobTran->srBRec.uszInstallmentBit == VS_FALSE && pobTran->srBRec.uszRedeemBit == VS_FALSE)
					{
						/* lnTipAmount為0表示非小費 */
						if (pobTran->srBRec.lnTipTxnAmount == 0L)
						{
							/* 小費 */
							inPRINT_Buffer_PutIn("小費(Tips)  :__________________________", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

							/* 總計 */
							inPRINT_Buffer_PutIn("總計(Total) :__________________________", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						}
						else
						{
							/* 小費 */
							/* 初始化 */
							memset(szTemplate, 0x00, sizeof(szTemplate));
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

							/* 將NT$ ＋數字塞到szTemplate中來inpad */
							sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTipTxnAmount);
							inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, VS_TRUE);

							/* 把前面的字串和數字結合起來 */
							sprintf(szPrintBuf, "小費(Tips)  :NT$%s", szTemplate);
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


							/* 總計 */
							/* 初始化 */
							memset(szTemplate, 0x00, sizeof(szTemplate));
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

							/* 將NT$ ＋數字塞到szTemplate中來inpad */
							sprintf(szTemplate, "%ld",  (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
							inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, VS_TRUE);

							/* 把前面的字串和數字結合起來 */
							sprintf(szPrintBuf, "總計(Total) :NT$%s", szTemplate);
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						}
					}
				}

			}
			/* 小費沒開時 */
			else
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
				if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
				{
					sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
				}
				else if(pobTran->srBRec.inCode == _ADJUST_)
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnAdjustTxnAmount);
				else
				{
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				}

				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, VS_TRUE);

				/* 把前面的字串和數字結合起來 */
				sprintf(szPrintBuf, "金額(Amount):NT$%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}

		}
	}
	/* 斷行 */
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_AMOUNT_ByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :列印AMOUNT
*/
int inCREDIT_PRINT_NewAmountByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char    szPrintBuf[84 + 1], szTemplate[42 + 1];

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 負向交易 */
	if(pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		/* 金額 */
		/* 取消退貨是正數 */
		if (pobTran->srBRec.inOrgCode == _REFUND_ || pobTran->srBRec.inOrgCode == _INST_REFUND_ || 
			 pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
		{
			/* 初始化 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "金額(Amount): NT$");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		}
		else
		{
			/* 初始化 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "金額(Amount): NT$");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		}
	}
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTransFunc(szTemplate);
		if (szTemplate[6] == 0x31) /* 修改TMS交易開關位置 20190211 [SAM] */
		{
			/* 退貨金額為負數 */
			if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "金額(Amount): NT$");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			}
			/* 預授不會有小費，所以拉出來 */
			else if (pobTran->srBRec.inCode == _PRE_AUTH_ || pobTran->srBRec.inCode == _PRE_COMP_)
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "金額(Amount): NT$");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			}
			else
			{
				/* 金額 */
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "金額(Amount): NT$");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				if(pobTran->srBRec.inCode == _ADJUST_)
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnAdjustTxnAmount);
				else
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);

				/* 把前面的字串和數字結合起來 */
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

				if(pobTran->srBRec.uszInstallmentBit == VS_FALSE && 
				   pobTran->srBRec.uszRedeemBit == VS_FALSE)
				{
					/* lnTipAmount為0表示非小費 */
					if (pobTran->srBRec.lnTipTxnAmount == 0L)
					{
						/* 小費 */
						inPRINT_Buffer_PutIn("小費(Tips)  :__________________________", _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

						/* 總計 */
						inPRINT_Buffer_PutIn("總計(Total) :__________________________", _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					}
					else
					{
						/* 小費 */
						/* 初始化 */
						memset(szTemplate, 0x00, sizeof(szTemplate));
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "小費(Tips)  : NT$");
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
						/* 將NT$ ＋數字塞到szTemplate中來inpad */
						sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTipTxnAmount);
						inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", szTemplate);
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);


						/* 總計 */
						memset(szTemplate, 0x00, sizeof(szTemplate));
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "總計(Total) : NT$");
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

						sprintf(szTemplate, "%ld",  (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", szTemplate);
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
					}
				}
			}

		}
		/* 小費沒開時 */
		else
		{
			/* 初始化 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			
			sprintf(szPrintBuf, "金額(Amount): NT$");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			
			/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
			if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			}
			else if(pobTran->srBRec.inCode == _ADJUST_)
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnAdjustTxnAmount);
			else
			{
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			}

			inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

//			if (inRetVal != VS_SUCCESS)
//				return (VS_ERROR);
			
			
		}

	}
	/* 斷行 */
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	return (VS_SUCCESS);
}


/*
Function        :inCREDIT_PRINT_INST_ByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :列印分期
*/
int inCREDIT_PRINT_Inst_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char    szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1];

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inCREDIT_PRINT_Inst_ByBuffer() 1!!");

	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{

		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("inCREDIT_PRINT_Inst_ByBuffer() 2!!");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		/* 分期期數 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
		inFunc_Amount_Comma(szTemplate1, "" , ' ', _SIGNED_NONE_, 18, _PAD_RIGHT_FILL_LEFT_);
		sprintf(szTemplate, "%s期", szTemplate1);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "分期期數   :%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 首期金額 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		//if (pobTran->srBRec.lnTipTxnAmount > 0L)
		//	sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentDownPayment + pobTran->srBRec.lnTipTxnAmount));
		if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _INST_REFUND_) ||
			(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _INST_SALE_ || pobTran->srBRec.inOrgCode == _INST_ADJUST_)))
		//if (pobTran->srBRec.inCode == _INST_REFUND_ || (pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode == _INST_ADJUST_))
			sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnInstallmentDownPayment));
		else
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentDownPayment));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, _PAD_RIGHT_FILL_LEFT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "首期金額   :NT$ %s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 每期金額 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _INST_REFUND_) ||
			(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _INST_SALE_ || pobTran->srBRec.inOrgCode == _INST_ADJUST_)))
		//if (pobTran->srBRec.inCode == _INST_REFUND_ || (pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode == _INST_ADJUST_))
			sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnInstallmentPayment));
		else
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentPayment));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, _PAD_RIGHT_FILL_LEFT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "每期金額   :NT$ %s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 分期手續費 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _INST_REFUND_) ||
			(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _INST_SALE_ || pobTran->srBRec.inOrgCode == _INST_ADJUST_)))
		//if (pobTran->srBRec.inCode == _INST_REFUND_)
			sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnInstallmentFormalityFee));
		else
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentFormalityFee));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, _PAD_RIGHT_FILL_LEFT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "分期手續費 :NT$ %s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 斷行 */
		inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 分期警語*/
		inPRINT_Buffer_PutGraphic((unsigned char*)_LEGAL_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inInstHeight, _APPEND_);

		/* 斷行 */
		inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	}

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inCREDIT_PRINT_Inst_ByBuffer() 3!!");

        return (VS_SUCCESS);
}

/*
Function        : inCREDIT_PRINT_NewInstByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :列印分期
*/
int inCREDIT_PRINT_NewInstByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char    szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1];

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inCREDIT_PRINT_Inst_ByBuffer() 1!!");

	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		/* 分期期數 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "分期期數    :");
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
		inFunc_Amount_Comma(szTemplate1, "" , ' ', _SIGNED_NONE_, 4, _PAD_RIGHT_FILL_LEFT_);
		sprintf(szTemplate, "%s期", szTemplate1);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

		/* 首期金額 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "首期金額    : NT$");
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _INST_REFUND_) ||
			(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _INST_SALE_ || pobTran->srBRec.inOrgCode == _INST_ADJUST_)))
			sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnInstallmentDownPayment));
		else
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentDownPayment));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		
		/* 每期金額 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "每期金額    : NT$");
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _INST_REFUND_) ||
			(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _INST_SALE_ || pobTran->srBRec.inOrgCode == _INST_ADJUST_)))
			sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnInstallmentPayment));
		else
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentPayment));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

		/* 分期手續費 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "分期手續費  : NT$");
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _INST_REFUND_) ||
			(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _INST_SALE_ || pobTran->srBRec.inOrgCode == _INST_ADJUST_)))
			sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnInstallmentFormalityFee));
		else
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentFormalityFee));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

		/* 斷行 */
		inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 分期警語*/
		inPRINT_Buffer_PutGraphic((unsigned char*)_LEGAL_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inInstHeight, _APPEND_);

		/* 斷行 */
		inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	}

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inCREDIT_PRINT_Inst_ByBuffer() 3!!");

        return (VS_SUCCESS);
}


/*
Function        :inCREDIT_PRINT_REDEEM_ByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :列印紅利
*/
int inCREDIT_PRINT_Redeem_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char    szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1];

	if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
		{
			if (pobTran->srBRec.inOrgCode == _REDEEM_SALE_ || pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_)
			{
				/* 交易金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
				//sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 9, VS_TRUE);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "交易金額　　     :NT$%s", szTemplate);
				//inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 實付金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnRedemptionPaidCreditAmount));
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 9, VS_TRUE);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				//sprintf(szPrintBuf, "支付金額　　     :NT$%s", szTemplate);
				sprintf(szPrintBuf, "實付金額　　     :NT$%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnRedemptionPaidAmount));
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 9, VS_TRUE);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵金額     :NT$%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnRedemptionPoints));
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PAD_RIGHT_FILL_LEFT_);
				sprintf(szTemplate, "%s點",szTemplate1);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵點數     :%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else if(pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
			{
				/* 交易金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 9, VS_TRUE);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "交易金額　　     :NT$%s", szTemplate);
				//inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 實付金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount);
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 9, VS_TRUE);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				//sprintf(szPrintBuf, "支付金額　　     :NT$%s", szTemplate);
				sprintf(szPrintBuf, "實付金額　　     :NT$%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidAmount);
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 9, VS_TRUE);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵金額     :NT$%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnRedemptionPoints);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PAD_RIGHT_FILL_LEFT_);
				sprintf(szTemplate, "%s點",szTemplate1);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵點數     :%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
		else
		{
			if (pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				/* 交易金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 9, VS_TRUE);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "交易金額　　     :NT$%s", szTemplate);
				//inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 實付金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnRedemptionPaidCreditAmount));
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 9, VS_TRUE);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				//sprintf(szPrintBuf, "支付金額　    　 :NT$%s", szTemplate);
				sprintf(szPrintBuf, "實付金額　    　 :NT$%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnRedemptionPaidAmount));
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 9, VS_TRUE);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵金額     :NT$%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", (0-pobTran->srBRec.lnRedemptionPoints));
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PAD_RIGHT_FILL_LEFT_);
				sprintf(szTemplate, "%s點",szTemplate1);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵點數     :%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				/* 交易金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 9, VS_TRUE);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "交易金額　　     :NT$%s", szTemplate);
				//inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 實付金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount);
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 9, VS_TRUE);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "實付金額　    　 :NT$%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidAmount);
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 9, VS_TRUE);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵金額     :NT$%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnRedemptionPoints);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PAD_RIGHT_FILL_LEFT_);
				sprintf(szTemplate, "%s點",szTemplate1);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵點數     :%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (pobTran->srBRec.inCode == _REDEEM_SALE_)
				{
					/* 紅利剩餘點數 */
					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memset(szTemplate1, 0x00, sizeof(szTemplate1));
					sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnRedemptionPointsBalance));
					inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PAD_RIGHT_FILL_LEFT_);
					sprintf(szTemplate, "%s點",szTemplate1);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "紅利剩餘點數     :%s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}

			}

		}

		/* 斷行 */
		inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

        return (VS_SUCCESS);
}
/*
Function        : inCREDIT_PRINT_NewRedeemByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :列印紅利
*/
int inCREDIT_PRINT_NewRedeemByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char    szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1];

	if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
		{
			if (pobTran->srBRec.inOrgCode == _REDEEM_SALE_ || pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_)
			{
				/* 實付金額 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "實付金額    : NT$");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnRedemptionPaidCreditAmount));
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

				/* 紅利扣抵金額 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵金額: NT$");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnRedemptionPaidAmount));
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

				/* 紅利扣抵點數 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵點數:");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnRedemptionPoints));
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PAD_RIGHT_FILL_LEFT_);
				sprintf(szTemplate, "%s點",szTemplate1);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			}
			else if(pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
			{
				/* 實付金額 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "實付金額    : NT$");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
								
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount);
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

				/* 紅利扣抵金額 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵金額: NT$");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidAmount);
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

				/* 紅利扣抵點數 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵點數:");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnRedemptionPoints);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PAD_RIGHT_FILL_LEFT_);
				sprintf(szTemplate, "%s點",szTemplate1);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			}
		}
		else
		{
			if (pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				
				/* 實付金額 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "實付金額    : NT$");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnRedemptionPaidCreditAmount));
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

				/* 紅利扣抵金額 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵金額: NT$");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnRedemptionPaidAmount));
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

				/* 紅利扣抵點數 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵點數:");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", (0-pobTran->srBRec.lnRedemptionPoints));
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PAD_RIGHT_FILL_LEFT_);
				sprintf(szTemplate, "%s點",szTemplate1);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			}
			else
			{
			
				/* 實付金額 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "實付金額    : NT$");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount);
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

				/* 紅利扣抵金額 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵金額: NT$");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidAmount);
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

				/* 紅利扣抵點數 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵點數:");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnRedemptionPoints);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PAD_RIGHT_FILL_LEFT_);
				sprintf(szTemplate, "%s點",szTemplate1);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

				if (pobTran->srBRec.inCode == _REDEEM_SALE_)
				{
					/* 紅利剩餘點數 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "紅利剩餘點數:");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
					
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memset(szTemplate1, 0x00, sizeof(szTemplate1));
					sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnRedemptionPointsBalance));
					inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PAD_RIGHT_FILL_LEFT_);
					sprintf(szTemplate, "%s點",szTemplate1);

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				}

			}

		}

		/* 斷行 */
		inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

        return (VS_SUCCESS);
}


/*
Function        :inCREDIT_PRINT_ReceiptEND_ByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :列印結尾
*/
int inCREDIT_PRINT_ReceiptEND_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
int	i = 0;
int	inRetVal = VS_ERROR;
char	szTemplate[42 + 1]= {0};
char	szSignature[16 + 1] = {0};
char	szSignaturePath[50 + 1] = {0};
char	szDemoMode[2 + 1] = {0};

	if (inPrinttype_ByBuffer)
	{
		/* 直式 */
		inPRINT_Buffer_PutIn("簽名欄:_____________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
		{
			inPRINT_Buffer_PutIn("*** 商店收據 Merchant Copy ***", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			pobTran->srBRec.inPrintOption = _PRT_CUST_;
		}
		else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			inPRINT_Buffer_PutIn("*** 持卡人收據 Customer Copy ***", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			pobTran->srBRec.inPrintOption = _PRT_MERCH_;
		}

		inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

	}
	else
	{
		/* 橫式 */
		if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
		{
			/* 簽名欄 */
			/* 免簽名 */
			if (pobTran->srBRec.uszF56NoSignatureBit == VS_TRUE/* && pobTran->srBRec.inCode != _TIP_*/)
			//if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				inPRINT_Buffer_PutIn("免      簽      名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("No signature required", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			}
			/* 要簽名 */
			else
			{
				/* 藉由TRT_FileName比對來組出bmp的檔名 */
				inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

				memset(szSignature, 0x00, sizeof(szSignature));
				/* 因為用invoice所以不用inFunc_ComposeFileName */
				inFunc_ComposeFileName_InvoiceNumber(pobTran, szSignature, _PICTURE_FILE_EXTENSION_, 6);
				memset(szSignaturePath, 0x00, sizeof(szSignaturePath));
				sprintf(szSignaturePath, "./fs_data/%s", szSignature);
								
				inDISP_LogPrintfWithFlag("  PrintSignature FileName[%s]",szSignaturePath);
				
				/* 如果是KIOSK 的交易，而且是補空白圖檔，就不需要印圖  2020/3/27 下午 6:22 [SAM] */
				if(inFunc_GetKisokFlag() == VS_TRUE && pobTran->srBRec.inSignStatus == _SIGN_SIGNED_)
				{
					/* a space 2 line */
					for (i = 0; i < 2; i++)
					{
						inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					}
					
				}else
				{
				
					/* 圖檔存在、有在signpad簽名、且非重印（重印不出簽名）（目前簽名狀態存不了Batch，先把&& pobTran->srBRec.inSignStatus == _SIGN_SIGNED_此條件拿掉） */
					if (inFILE_Check_Exist((unsigned char *)szSignature) == VS_SUCCESS && pobTran->inRunOperationID != _OPERATION_REPRINT_)
					{
						/* 電子簽名 */
						inPRINT_Buffer_PutGraphic((unsigned char *)szSignaturePath, uszBuffer, srBhandle, 236, _APPEND_);
					}
					/* 手簽 */
					else
					{
						/* a space 2 line */
						for (i = 0; i < 2; i++)
						{
							inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						}

					}
				}

			}

			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}

				}
			}

			inPRINT_Buffer_PutIn("X:________________________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%s", pobTran->srBRec.szCardHolder);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("　　　　　　　　　 持卡人簽名", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
		{
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}

				}
			}

			if (pobTran->srBRec.uszF56NoSignatureBit == VS_TRUE)
			//if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
			{
				/* 免簽名 */
				inRetVal = inPRINT_Buffer_PutIn("免      簽      名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("No signature required", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("　　　　　　　　　 商店存根", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate,"%s",pobTran->srBRec.szCardHolder);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}

				}
			}

			if (pobTran->srBRec.uszF56NoSignatureBit == VS_TRUE)
			//if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
			{
				/* 免簽名 */
				inRetVal = inPRINT_Buffer_PutIn("免      簽      名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("No signature required", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("　　　　　　　　　 持卡人存根", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("　　　　　　　  Card holder stub", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate,"%s",pobTran->srBRec.szCardHolder);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
		{
			inPRINT_Buffer_PutIn("           重印 REPRINT", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		/* 列印警示語 */
		inPRINT_Buffer_PutIn("            I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("        ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* Print Notice */
		if (inCREDIT_PRINT_Notice(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
			return (VS_ERROR);

		/* Print Slogan */
		if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			if (inCREDIT_PRINT_MarchantSlogan(pobTran, _NCCC_SLOGAN_PRINT_DOWN_, uszBuffer, srBhandle) != VS_SUCCESS)
				return (VS_ERROR);

//			if (pobTran->srBRec.uszRewardL1Bit == VS_TRUE	||
//				pobTran->srBRec.uszRewardL2Bit == VS_TRUE	||
//				pobTran->srBRec.uszRewardL5Bit == VS_TRUE)
//			{
//				if (inCREDIT_PRINT_RewardAdvertisement(pobTran, uszBuffer, srFont_Attrib, srBhandle) != VS_SUCCESS)
//					return (VS_ERROR);
//			}

		}

		for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

	}

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TIDMID_ByBuffer_Small
Date&Time       :2016/3/17 上午 10:24
Describe        :列印TID & MID
*/
int inCREDIT_PRINT_Tidmid_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        int     inRetVal;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

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
        }
        else
        {
                /* 橫式 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                inGetMerchantID(szPrintBuf);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_WIDTH_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 列印商店代號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "商店代號");
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* Get端末機代號 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                inGetTerminalID(szPrintBuf);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_WIDTH_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 列印端末機代號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "端末機代號");
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
        }

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_Data_ByBuffer_Small
Date&Time       :2016/3/17 上午 10:24
Describe        :列印DATA
*/
int inCREDIT_PRINT_Data_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
        int     inRetVal;
        char 	szPrintBuf[84 + 1], szPrintBuf1[84 + 1], szPrintBuf2[84 + 1], szTemplate1[42 + 1], szTemplate2[84 + 1];
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

        if (inPrinttype_ByBuffer)
        {
                /* 直式 */

                /*卡別、卡號*/
                sprintf(szPrintBuf, "卡別　　：%s", pobTran->srBRec.szCardLabel);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡號　　：%s", pobTran->srBRec.szPAN);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*日期、時間*/
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "日期　　：%s",pobTran->srBRec.szDate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "時間　　：%s",pobTran->srBRec.szTime);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*調閱編號、批次號碼 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "調閱編號：%06ld",pobTran->srBRec.lnOrgInvNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "批次號碼：%06ld",pobTran->srBRec.lnBatchNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*交易類別*/
				vdEDC_GetTransType(pobTran, szTemplate1, szTemplate2);
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "交易類別：%s",szTemplate1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*授權碼、序號*/
                sprintf(szPrintBuf, "授權碼　：%s",pobTran->srBRec.szAuthCode);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "序號　　：%s",pobTran->srBRec.szRefNo);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        }
        else
        {
                /* 橫式 */
		/* "卡號 卡別" */
		/* "卡號" */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inFunc_PAD_ASCII(szPrintBuf, "卡號", ' ', 31, _PAD_LEFT_FILL_RIGHT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 卡別值 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 20, _PAD_RIGHT_FILL_LEFT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 卡別 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "卡別");
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 卡號值 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));

		strcpy(szPrintBuf1, pobTran->srBRec.szPAN);

		if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
		{
			inFunc_FormatPAN_UCARD(szPrintBuf1);
		}

		/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
		if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
			{
				for (i = 3; i < (strlen(szPrintBuf1) - 5); i ++)
					szPrintBuf1[i] = 0x2A;
			}
			else
			{
				for (i = 6; i < (strlen(szPrintBuf1) - 4); i ++)
					szPrintBuf1[i] = 0x2A;
			}

		}
		else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
		{
			/* 商店聯卡號遮掩 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
			{
				if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
				{
					for (i = 3; i < (strlen(szPrintBuf) - 5); i ++)
						szPrintBuf[i] = 0x2A;
				}
				else
				{
					for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
						szPrintBuf[i] = 0x2A;
				}
			}

		}

		/* 過卡方式 */
		if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				strcat(szPrintBuf1,"(C)");
			else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
				strcat(szPrintBuf, "(T)");
			else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf1, "(W)");
			else
			{
				if (pobTran->srBRec.uszManualBit == VS_TRUE)
				{
					/* 【需求單-105244】端末設備支援以感應方式進行退貨交易 */
					/* 電文轉Manual Keyin但是簽單要印感應的W */
					if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
						strcat(szPrintBuf1, "(W)");
					else
						strcat(szPrintBuf1,"(M)");
				}
				else
					strcat(szPrintBuf1,"(S)");
			}

		}
		else
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf1, "(W)");
			else
				strcat(szPrintBuf1, "(C)");
		}

		sprintf(szPrintBuf, "%s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_WIDTH_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 交易別 */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", "交易");

		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		vdEDC_GetTransType(pobTran, szTemplate1, szTemplate2);

		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", szTemplate1);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%s %s", szPrintBuf1, szPrintBuf2);

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		if (strlen(szTemplate2) > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate2);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}

                /* 城市 主機 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));

		/* 城市 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetCityName(szTemplate1);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s %s", "城市", szTemplate1);

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 主機 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetHostLabel(szTemplate1);
		inFunc_DiscardSpace(szTemplate1);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PAD_RIGHT_FILL_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "主機");
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 日期時間 批號 */
		/* 日期時間 */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szTemplate1, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		sprintf(szPrintBuf1, "%s %s", "日期/時間", szTemplate1);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);

                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 批號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%03ld", pobTran->srBRec.lnBatchNum);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PAD_RIGHT_FILL_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 批號 */
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "批號");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);

                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 授權碼 檢查碼 */
		/* 前半段 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szAuthCode[0], 12);
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s %s", "授權碼", szTemplate1);
		inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 32, _PAD_LEFT_FILL_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);

                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 後半段 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inCARD_ExpDateEncryptAndDecrypt(pobTran, szTemplate1, szTemplate1, _EXP_ENCRYPT_);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 9, _PAD_RIGHT_FILL_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "檢查碼");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 序號 調閱號 */
		/* 序號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szRefNo[0], 12);
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s %s", "序號", szTemplate1);
		inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 29, _PAD_LEFT_FILL_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);

                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);


		/* 調閱號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%06ld", pobTran->srBRec.lnOrgInvNum);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PAD_RIGHT_FILL_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* "調閱號" */
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "調閱號");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 櫃號 回覆碼 */
		/* 櫃號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetStoreIDEnable(szTemplate1);
		if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
		{
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memcpy(szTemplate1, &pobTran->srBRec.szStoreID[0], 23);
			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			sprintf(szPrintBuf1, "%s %s", "櫃號", szTemplate1);
			inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 33, _PAD_LEFT_FILL_RIGHT_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szPrintBuf1);

			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* 後半段(銀聯交易才印回覆碼) */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szRespCode);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 9, _PAD_RIGHT_FILL_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate1);

				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
				sprintf(szPrintBuf2, "%s", "回覆碼");
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szPrintBuf2);

				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			else
			{
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
		}
		else
		{
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				/* 後半段(銀聯交易才印回覆碼) */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szRespCode);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 9, _PAD_RIGHT_FILL_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate1);

				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
				sprintf(szPrintBuf2, "%s", "回覆碼");
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szPrintBuf2);

				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			else
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
		}

		/* TC */
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_ || pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			/* 感應磁條 */
			if (!memcmp(pobTran->srBRec.szAuthCode, "VLP", 3)				||
			    !memcmp(pobTran->srBRec.szAuthCode, "JCB", 3)				||
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE	||
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD		||
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
			{
				/* 商店聯卡號遮掩 */
				memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
				inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
				if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&&
				    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
				    strlen(pobTran->srBRec.szTxnNo) > 0)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
			}
			else
			{
				if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
				{
					if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "TC:   %02X%02X%02X%02X%02X%02X%02X%02X",
										pobTran->srEMVRec.usz9F26_ApplCryptogram[0],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[1],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[2],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[3],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[4],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[5],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[6],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[7]);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}

					/* 商店聯卡號遮掩 */
					memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
					inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
					if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&&
					    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
					    strlen(pobTran->srBRec.szTxnNo) > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}

					/* M/C交易列印AP Lable (START) */
					if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AP Label:");
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
					}
					/* M/C交易列印AP Lable (END) */

					/* AID */
					/* 只有CUP晶片才要印 */
					if (pobTran->srBRec.uszCUPTransBit == VS_TRUE && pobTran->srBRec.inChipStatus == _EMV_CARD_ && strlen(pobTran->srBRec.szCUP_EMVAID) > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "AID:  %s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
					}
					/* 銀聯閃付 */
					else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE && pobTran->srBRec.uszContactlessBit == VS_TRUE && strlen(pobTran->srBRec.szCUP_EMVAID) > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
					}

				}
				else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
				{
					/* 商店聯卡號遮掩 */
					/* 持卡人存根也要印 */
					memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
					inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
					if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&&
					    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
					    strlen(pobTran->srBRec.szTxnNo) > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "交易編號  %s", pobTran->srBRec.szTxnNo);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
				}
			}

		}
		else
		{
			/* 商店聯卡號遮掩 */
			/* 磁條卡列印交易編號 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&&
			    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
			    strlen(pobTran->srBRec.szTxnNo) > 0)
			{
		 		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		 		sprintf(szPrintBuf, "交易編號  %s", pobTran->srBRec.szTxnNo);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
		}

		/* 產品代碼 */
		inGetProductCodeEnable(szProductCodeEnable);
		if (memcmp(szProductCodeEnable, "Y", 1) == 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s %s", "產品代碼", pobTran->srBRec.szProductCode);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}

        }

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_Cup_Amount_ByBuffer_Small
Date&Time       :2016/3/17 上午 10:24
Describe        :列印銀聯AMOUNT
*/
int inCREDIT_PRINT_Cup_Amount_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                /* 金額 */
                if(pobTran->srBRec.inCode == _TIP_)
                {
                        /* 金額 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PAD_RIGHT_FILL_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


                        /* 小費 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTipTxnAmount);
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PAD_RIGHT_FILL_LEFT_);

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "小費(Tips)  :%s", szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


                        /* 總計 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ' , 14, _PAD_RIGHT_FILL_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf,"總計(Total) :%s", szTemplate);
                }
                else if ((pobTran->srBRec.uszVOIDBit == VS_TRUE &&
			 (pobTran->srBRec.inOrgCode != _REFUND_ && pobTran->srBRec.inOrgCode != _INST_REFUND_ && pobTran->srBRec.inOrgCode != _REDEEM_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_MAIL_ORDER_REFUND_))	||
			  pobTran->srBRec.inCode == _REFUND_		||
			  pobTran->srBRec.inCode == _INST_REFUND_	||
			  pobTran->srBRec.inCode == _REDEEM_REFUND_	||
			  pobTran->srBRec.inCode == _CUP_REFUND_	||
			  pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", 0 - pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PAD_RIGHT_FILL_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                else if (pobTran->srBRec.inCode == _ADJUST_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnAdjustTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PAD_RIGHT_FILL_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                else
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PAD_RIGHT_FILL_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		for (i = 0; i < 2; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
        }
        /* 橫式 */
        /* 負向交易 */
	else if(pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		/* 橫式 */
                /* 金額 */
                /* 取消退貨是正數 */
                if (pobTran->srBRec.inCode == _REFUND_		||
		    pobTran->srBRec.inCode == _INST_REFUND_	||
		    pobTran->srBRec.inCode == _REDEEM_REFUND_	||
		    pobTran->srBRec.inCode == _CUP_REFUND_	||
		    pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
		{
                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_,  17, _PAD_RIGHT_FILL_LEFT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

                        /* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Amount) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		else
		{
                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_,  17, _PAD_RIGHT_FILL_LEFT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

                        /* 把前面的字串和數字結合起來 */
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Amount) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

	}
	/* 正向交易 */
	else
	{
		/* 總計 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (pobTran->srBRec.inCode == _REFUND_		||
		    pobTran->srBRec.inCode == _INST_REFUND_	||
		    pobTran->srBRec.inCode == _REDEEM_REFUND_	||
		    pobTran->srBRec.inCode == _CUP_REFUND_	||
		    pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
		{
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
		}
		else
		{
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		}
		sprintf(szTemplate, "%ld",  (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
		inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 17, _PAD_RIGHT_FILL_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);


		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "總計(Amount) :");
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	/* 斷行 */
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 列印銀聯交易提示文字 */
	inPRINT_Buffer_PutGraphic((unsigned char*)_CUP_LEGAL_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inCupLegalHeight, _APPEND_);

	/* 斷行 */
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Amount_ByBuffer_Small
Date&Time       :2016/3/17 上午 10:23
Describe        :列印AMOUNT
*/
int inCREDIT_PRINT_Amount_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                /* 金額 */
                if(pobTran->srBRec.inCode == _TIP_)
                {
                        /* 金額 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PAD_RIGHT_FILL_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


                        /* 小費 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTipTxnAmount);
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PAD_RIGHT_FILL_LEFT_);

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "小費(Tips)  :%s", szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


                        /* 總計 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ' , 14, _PAD_RIGHT_FILL_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf,"總計(Total) :%s", szTemplate);
                }
                else if ((pobTran->srBRec.uszVOIDBit == VS_TRUE	&&
			 (pobTran->srBRec.inOrgCode != _REFUND_ && pobTran->srBRec.inOrgCode != _INST_REFUND_ && pobTran->srBRec.inOrgCode != _REDEEM_REFUND_)) ||
			  pobTran->srBRec.inCode == _REFUND_	 ||
			  pobTran->srBRec.inCode == _INST_REFUND_||
			  pobTran->srBRec.inCode == _REDEEM_REFUND_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", 0 - pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PAD_RIGHT_FILL_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                else if (pobTran->srBRec.inCode == _ADJUST_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnAdjustTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PAD_RIGHT_FILL_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                else
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PAD_RIGHT_FILL_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		for (i = 0; i < 2; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
        }
        /* 橫式 */
        /* 負向交易 */
	else if(pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		/* 橫式 */
                /* 金額 */
                /* 取消退貨是正數 */
                if (pobTran->srBRec.inOrgCode == _REFUND_ || pobTran->srBRec.inOrgCode == _INST_REFUND_ || pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
                {
                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_,  17, _PAD_RIGHT_FILL_LEFT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

                        /* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Amount) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		else
		{
                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_,  17, _PAD_RIGHT_FILL_LEFT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

                        /* 把前面的字串和數字結合起來 */
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Amount) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

	}
	/* 正向交易 */
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTransFunc(szTemplate);
//		if (szTemplate[6] == 'Y')	/* 檢查是否有開小費 */
		if (szTemplate[6] == 0x31) /* 修改TMS交易開關位置 20190211 [SAM] */
		{
			/* 退貨金額為負數 */
			if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
				inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_,  17, _PAD_RIGHT_FILL_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "總計(Amount) :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			/* 預授不會有小費，所以拉出來 */
			else if (pobTran->srBRec.inCode == _PRE_AUTH_ || pobTran->srBRec.inCode == _PRE_COMP_)
			{
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_,  17, _PAD_RIGHT_FILL_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "總計(Amount) :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				/* 金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 17, _PAD_RIGHT_FILL_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "金額(Amount):");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* lnTipAmount為0表示非小費 */
				if (pobTran->srBRec.lnTipTxnAmount == 0L)
				{
					/* 小費 */
					inPRINT_Buffer_PutIn("小費(Tips)  :______________________________________", _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					/* 總計 */
					inPRINT_Buffer_PutIn("總計(Total) :______________________________________", _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				else
				{
					/* 小費 */
					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTipTxnAmount);
					inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 17, _PAD_RIGHT_FILL_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", "小費(Tips)  :");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


					/* 總計 */
					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%ld",  (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
					inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 17, _PAD_RIGHT_FILL_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", "總計(Amount) :");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}

			}

		}
		/* 小費沒開時 */
		else
		{
			/* 總計 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			}
			else
			{
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			}
			sprintf(szTemplate, "%ld",  (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
			inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 17, _PAD_RIGHT_FILL_LEFT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);


			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Amount) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

	}

	/* 斷行 */
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Inst_ByBuffer_Small
Date&Time       :2016/3/17 上午 10:23
Describe        :列印分期
*/
int inCREDIT_PRINT_Inst_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char    szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		/* 分期期數 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
		inFunc_Amount_Comma(szTemplate1, "" , ' ', _SIGNED_NONE_, 18, VS_TRUE);
		sprintf(szTemplate, "%s期", szTemplate1);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "分期期數   :%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 首期金額 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (pobTran->srBRec.lnTipTxnAmount > 0L)
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentDownPayment + pobTran->srBRec.lnTipTxnAmount));
		else
			sprintf(szTemplate, "%ld", pobTran->srBRec.lnInstallmentDownPayment);
		inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 19, VS_TRUE);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "首期金額   :%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 每期金額 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentPayment));
		inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 19, VS_TRUE);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "每期金額   :%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 分期手續費 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentFormalityFee));
		inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 19, VS_TRUE);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "分期手續費 :%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	/* 分期警語*/
	inPRINT_Buffer_PutGraphic((unsigned char*)_LEGAL_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inInstHeight, _APPEND_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Redeem_ByBuffer_Small
Date&Time       :2016/3/17 上午 10:23
Describe        :列印紅利
*/
int inCREDIT_PRINT_Redeem_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		if (pobTran->srBRec.inOrgCode == _REDEEM_SALE_ || pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_)
		{
			/* 支付金額 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
			inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 17, VS_TRUE);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "支付金額　　 :%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紅利扣抵金額 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld", (0 - (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount)));
			inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 17, VS_TRUE);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "紅利扣抵金額 :%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紅利扣抵點數 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnRedemptionPoints));
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 16, _PAD_RIGHT_FILL_LEFT_);
			sprintf(szTemplate, "%s點",szTemplate1);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "紅利扣抵點數 :%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else if(pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
		{
			/* 支付金額 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount);
			inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 17, VS_TRUE);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "支付金額　　 :%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紅利扣抵金額 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
			inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 17, VS_TRUE);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "紅利扣抵金額 :%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紅利扣抵點數 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnRedemptionPoints);
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 15, _PAD_RIGHT_FILL_LEFT_);
			sprintf(szTemplate, "%s點",szTemplate1);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "紅利扣抵點數 :%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}
	else
	{
		if (pobTran->srBRec.inCode == _REDEEM_REFUND_)
		{
			/* 支付金額 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnRedemptionPaidCreditAmount));
			inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 17, VS_TRUE);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "支付金額　　 :%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紅利扣抵金額 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld", (0 - (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount)));
			inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 17, VS_TRUE);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "紅利扣抵金額 :%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紅利扣抵點數 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnRedemptionPoints));
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 15, _PAD_RIGHT_FILL_LEFT_);
			sprintf(szTemplate, "%s點",szTemplate1);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "紅利扣抵點數 :%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else if (pobTran->srBRec.inCode == _REDEEM_ADJUST_)
		{
			/* 支付金額 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount);
			inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 17, VS_TRUE);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "支付金額　　 :%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紅利扣抵金額 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
			inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 17, VS_TRUE);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "紅利扣抵金額 :%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紅利扣抵點數 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnRedemptionPoints);
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 15, _PAD_RIGHT_FILL_LEFT_);
			sprintf(szTemplate, "%s點",szTemplate1);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "紅利扣抵點數 :%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			/* 支付金額 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount);
			inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 17, VS_TRUE);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "支付金額　　 :%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紅利扣抵金額 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
			inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 17, VS_TRUE);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "紅利扣抵金額 :%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紅利扣抵點數 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnRedemptionPoints);
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 15, _PAD_RIGHT_FILL_LEFT_);
			sprintf(szTemplate, "%s點",szTemplate1);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "紅利扣抵點數 :%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紅利剩餘點數 */
			if (pobTran->inTransactionCode == _REDEEM_SALE_ || pobTran->srBRec.inCode == _REDEEM_SALE_)
			{
				/* 紅利扣抵才印 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnRedemptionPointsBalance));
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 15, _PAD_RIGHT_FILL_LEFT_);
				sprintf(szTemplate, "%s點",szTemplate1);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利剩餘點數 :%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}

		}

	}

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_ReceiptEND_ByBuffer_Small
Date&Time       :2016/3/17 上午 10:23
Describe        :列印結尾
*/
int inCREDIT_PRINT_ReceiptEND_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
	int	inRetVal = VS_ERROR;
	char	szSignature[16 + 1] = {0};
	char	szSignaturePath[50 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};

        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                inPRINT_Buffer_PutIn("簽名欄:_____________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
                {
                        inPRINT_Buffer_PutIn("*** 商店收據 Merchant Copy ***", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_CUST_;

                }
                else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
                {
                        inPRINT_Buffer_PutIn("*** 持卡人收據 Customer Copy ***", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_MERCH_;
                }

                inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

        }
        else
        {
                /* 橫式 */
                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
                {
                        /* 簽名欄 */
			/* 免簽名 */
			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				inRetVal = inPRINT_Buffer_PutIn("X:      免簽名       ", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			/* 要簽名 */
			else
			{
				/* 藉由TRT_FileName比對來組出bmp的檔名 */
				inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

				memset(szSignature, 0x00, sizeof(szSignature));
				/* 因為用invoice所以不用inFunc_ComposeFileName */
				inFunc_ComposeFileName_InvoiceNumber(pobTran, szSignature, _PICTURE_FILE_EXTENSION_, 6);
				memset(szSignaturePath, 0x00, sizeof(szSignaturePath));
				sprintf(szSignaturePath, "./fs_data/%s", szSignature);

				/* 有在signpad簽名*/
				if (inFILE_Check_Exist((unsigned char *)szSignature) == VS_SUCCESS)
				{
					/* 電子簽名 */
					inPRINT_Buffer_PutGraphic((unsigned char *)szSignaturePath, uszBuffer, srBhandle, 236, _APPEND_);
				}
				/* 手簽 */
				else
				{
					inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}

				inPRINT_Buffer_PutIn("X:", _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}

			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}
				}
			}

			inPRINT_Buffer_PutIn("------------------------------------------------------", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("　　　　　　　　　 持卡人簽名", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
                {
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}

				}
			}

			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
			{
				/* 免簽名 */
				inRetVal = inPRINT_Buffer_PutIn("免簽名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			inPRINT_Buffer_PutIn("　　　　　　　　　 商店存根", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
                else
                {
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}

				}
			}

			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
			{
				/* 免簽名 */
				inRetVal = inPRINT_Buffer_PutIn("免簽名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
                        inPRINT_Buffer_PutIn("　　　　　持卡人存根 Card holder stub", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }

		if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
		{
			inPRINT_Buffer_PutIn("                 重印 REPRINT", _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

                /* 列印警示語 */
                inPRINT_Buffer_PutIn("            I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("        ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* Print Notice */
		if (inCREDIT_PRINT_Notice(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
			return (VS_ERROR);

		/* Print Slogan */
		if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			if (inCREDIT_PRINT_MarchantSlogan(pobTran, _NCCC_SLOGAN_PRINT_DOWN_, uszBuffer, srBhandle) != VS_SUCCESS)
				return (VS_ERROR);

//			if (pobTran->srBRec.uszRewardL1Bit == VS_TRUE	||
//			    pobTran->srBRec.uszRewardL2Bit == VS_TRUE	||
//			    pobTran->srBRec.uszRewardL5Bit == VS_TRUE)
//			{
//				if (inCREDIT_PRINT_RewardAdvertisement(pobTran, uszBuffer, srFont_Attrib, srBhandle) != VS_SUCCESS)
//					return (VS_ERROR);
//			}
		}

                for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Data_ByBuffer_Small_SmartPay
Date&Time       :2016/3/17 上午 10:24
Describe        :列印DATA
*/
int inCREDIT_PRINT_FISC_Data_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
        int     inRetVal;
        char 	szPrintBuf[84 + 1], szPrintBuf1[84 + 1], szPrintBuf2[84 + 1], szTemplate1[42 + 1], szTemplate2[42 + 1];;
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

        if (inPrinttype_ByBuffer)
        {
                /* 直式 */

                /*卡別、卡號*/
                sprintf(szPrintBuf, "卡別　　：%s", pobTran->srBRec.szCardLabel);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡號　　：%s", pobTran->srBRec.szPAN);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*日期、時間*/
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "日期　　：%s",pobTran->srBRec.szDate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "時間　　：%s",pobTran->srBRec.szTime);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*調閱編號、批次號碼 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "調閱編號：%06ld",pobTran->srBRec.lnOrgInvNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "批次號碼：%06ld",pobTran->srBRec.lnBatchNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*交易類別*/
				vdEDC_GetTransType(pobTran, szTemplate1, szTemplate2);
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "交易類別：%s",szTemplate1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*授權碼、序號*/
                sprintf(szPrintBuf, "授權碼　：%s",pobTran->srBRec.szAuthCode);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "序號　　：%s",pobTran->srBRec.szRefNo);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        }
        else
        {
                /* 橫式 */
		/* "卡號 卡別" */
		/* "卡號" */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inFunc_PAD_ASCII(szPrintBuf, "卡號", ' ', 31, _PAD_LEFT_FILL_RIGHT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 卡別值 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 20, _PAD_RIGHT_FILL_LEFT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* "卡別" */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "卡別");
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 卡號值 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));

		strcpy(szPrintBuf1, pobTran->srBRec.szPAN);
		if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
		{
			inFunc_FormatPAN_UCARD(szPrintBuf1);
		}

		/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
		if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
			{
				for (i = 3; i < (strlen(szPrintBuf1) - 5); i ++)
					szPrintBuf1[i] = 0x2A;
			}
			else
			{
				for (i = 6; i < (strlen(szPrintBuf1) - 4); i ++)
					szPrintBuf1[i] = 0x2A;
			}

		}
		else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
		{
			/* 商店聯卡號遮掩 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
			{
				if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
				{
					for (i = 3; i < (strlen(szPrintBuf) - 5); i ++)
						szPrintBuf[i] = 0x2A;
				}
				else
				{
					for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
						szPrintBuf[i] = 0x2A;
				}
			}

		}

		/* 過卡方式 */
		if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				strcat(szPrintBuf1,"(C)");
			else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
				strcat(szPrintBuf, "(T)");
			else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf1, "(W)");
			else
			{
				if (pobTran->srBRec.uszManualBit == VS_TRUE)
				{
					/* 【需求單-105244】端末設備支援以感應方式進行退貨交易 */
					/* 電文轉Manual Keyin但是簽單要印感應的W */
					if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
						strcat(szPrintBuf1, "(W)");
					else
						strcat(szPrintBuf1,"(M)");
				}
				else
					strcat(szPrintBuf1,"(S)");
			}

		}
		else
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf1, "(W)");
			else
				strcat(szPrintBuf1, "(C)");
		}

		sprintf(szPrintBuf, "%s", szPrintBuf1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_WIDTH_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 交易別 */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", "交易");

		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		vdEDC_GetTransType(pobTran, szTemplate1, szTemplate2);

		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", szTemplate1);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%s %s", szPrintBuf1, szPrintBuf2);

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		if (strlen(szTemplate2) > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate2);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}

                /* 發卡行代碼 主機 */
		/* 發卡行代碼 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s %s", "發卡行代碼", pobTran->srBRec.szFiscIssuerID);
		inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 31, _PAD_LEFT_FILL_RIGHT_);

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 主機*/
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetHostLabel(szTemplate1);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PAD_RIGHT_FILL_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s %s", "主機", szTemplate1);

                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 日期時間 批號 */
		/* 日期時間 */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szTemplate1, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		sprintf(szPrintBuf1, "%s %s", "日期/時間", szTemplate1);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);

                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 批號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%03ld", pobTran->srBRec.lnBatchNum);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PAD_RIGHT_FILL_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 批號 */
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "批號");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);

                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 序號 調閱號 */
		/* 序號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szRefNo[0], 12);
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s %s", "序號", szTemplate1);
		inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 29, _PAD_LEFT_FILL_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);

                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);


		/* 調閱號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%06ld", pobTran->srBRec.lnOrgInvNum);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PAD_RIGHT_FILL_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* "調閱號" */
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "調閱號");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);


		/* 調單編號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		strcpy(szTemplate1, pobTran->srBRec.szFiscRRN);
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s %s" , "調單編號", szTemplate1);

		/* 合併 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%s",  szPrintBuf1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 櫃號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetStoreIDEnable(szTemplate1);
		if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
		{
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memcpy(szTemplate1, &pobTran->srBRec.szStoreID[0], 23);
			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			sprintf(szPrintBuf1, "%s %s", "櫃號", szTemplate1);
			inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 33, _PAD_LEFT_FILL_RIGHT_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szPrintBuf1);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		else
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}

		/* 商店聯卡號遮掩 */
		memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
		inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
		if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&&
		    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
		    strlen(pobTran->srBRec.szTxnNo) > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}

		/* 產品代碼 */
		inGetProductCodeEnable(szProductCodeEnable);
		if (memcmp(szProductCodeEnable, "Y", 1) == 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s %s", "產品代碼", pobTran->srBRec.szProductCode);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}

        }

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_FISC_Amount_ByBuffer_Small
Date&Time       :2017/5/16 下午 4:56
Describe        :列印FISC AMOUNT
*/
int inCREDIT_PRINT_FISC_Amount_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        /* 橫式 */
        /* 負向交易 */
	if(pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		/* 橫式 */
                /* 金額 */
                /* 取消退貨是正數 */
                if (pobTran->srBRec.inOrgCode == _FISC_REFUND_)
                {
                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_,  17, _PAD_RIGHT_FILL_LEFT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

                        /* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Amount) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		else
		{
                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_,  17, _PAD_RIGHT_FILL_LEFT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

                        /* 把前面的字串和數字結合起來 */
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Amount) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

	}
	/* 正向交易 */
	else
	{
		/* 總計 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (pobTran->srBRec.inCode == _FISC_REFUND_)
		{
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
		}
		else
		{
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		}
		sprintf(szTemplate, "%ld",  (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
		inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 17, _PAD_RIGHT_FILL_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);


		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "總計(Amount) :");
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_CHECK_ByBuffer
Date&Time       :2016/2/24 下午 3:47
Describe        :
*/
int inCREDIT_PRINT_Check_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        int     inRecordCnt;

        inRecordCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
        /* 回傳VS_ERROR(回傳 -1 )會跳出，交易筆數小於0( VS_NoRecord 會回傳 -98 )會印空白簽單 */
        /* 其餘則回傳交易筆數*/

        return (inRecordCnt);
}


/*
Function        :inCREDIT_PRINT_TOP_ByBuffer
Date&Time       :2016/2/24 下午 3:48
Describe        :
*/
int inCREDIT_PRINT_Top_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        /* Get商店代號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetMerchantID(szTemplate);

        /* 列印商店代號 */
        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PAD_RIGHT_FILL_LEFT_);
        sprintf(szPrintBuf, "商店代號%s", szTemplate);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        /* Get端末機代號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetTerminalID(szTemplate);

        /* 列印端末機代號 */
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PAD_RIGHT_FILL_LEFT_);
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

        /* 列印交易類別 */
        if (pobTran->srBRec.inCode == _SETTLE_)
        {
                inRetVal = inPRINT_Buffer_PutIn("交易類別(Trans. Type)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "60 結帳 SETTLEMENT");
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }

        /* 列印批次號碼 */
        inRetVal = inPRINT_Buffer_PutIn("批次號碼(Batch No.)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szTemplate, "%06ld", pobTran->srBRec.lnBatchNum);
        //sprintf(szTemplate, "%03ld", pobTran->srBRec.lnBatchNum);
        strcpy(szPrintBuf, szTemplate);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
        /* 列印主機 */
        inRetVal = inPRINT_Buffer_PutIn("主機(Host)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        inGetHostLabel(szPrintBuf);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmount_ByBuffer
Date&Time       :2016/2/24 下午 3:49
Describe        :列印總金額
*/
int inCREDIT_PRINT_TotalAmount_ByBuffer(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[84 + 1];

        if (pobTran->inRunOperationID == _OPERATION_SETTLE_)
	{
		inPRINT_Buffer_PutIn("         結帳報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	else
        {
                inPRINT_Buffer_PutIn("         總額報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }

        //inPRINT_Buffer_PutIn("    筆數(CNT)      金額(AMOUNT)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        if (srAccumRec->lnTotalCount == 0)
        {
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售   　%03lu   NT$", 0L);
                sprintf(szTemplate, "%ld", 0L);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨   　%03lu   NT$", 0L);
                sprintf(szTemplate, "%ld", 0L);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 淨額 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "總額   　%03lu   NT$", 0L);
                //sprintf(szPrintBuf, "淨額 Ｔ　%03lu   NT$", 0L);
                sprintf(szTemplate, "%ld", 0L);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", 0L);
                sprintf(szTemplate, "%ld", 0L);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }
        else
        {
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售   　%03lu   NT$", srAccumRec->lnTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llTotalSaleAmount + srAccumRec->llTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨   　%03lu   NT$", srAccumRec->lnTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 淨額 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "總額   　%03lu   NT$", srAccumRec->lnTotalCount);
                //sprintf(szPrintBuf, "淨額 Ｔ　%03lu   NT$", srAccumRec->lnTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByCard_ByBuffer
Date&Time       :2016/2/24 下午 3:49
Describe        :依卡別列印
*/
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
char    szPrintBuf[84 + 1], szTemplate[84 + 1];

	inPRINT_Buffer_PutIn("         卡別小計", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (srAccumRec->llUCardTotalSaleAmount != 0L || srAccumRec->llUCardTotalRefundAmount != 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "卡別 　　%s", "UCARD");
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

		/* 銷售 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnUCardTotalSaleCount);
		sprintf(szTemplate, "%lld", (srAccumRec->llUCardTotalSaleAmount + srAccumRec->llUCardTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		/* 退貨 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnUCardTotalRefundCount);
		sprintf(szTemplate, "%lld", (0 - srAccumRec->llUCardTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		/* 小計 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnUCardTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llUCardTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		/* 小費 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnUCardTotalTipsCount);
		sprintf(szTemplate, "%lld", srAccumRec->llUCardTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llVisaTotalSaleAmount != 0L || srAccumRec->llVisaTotalRefundAmount != 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_VISA_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		/* 銷售 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnVisaTotalSaleCount);
		sprintf(szTemplate, "%lld", (srAccumRec->llVisaTotalSaleAmount + srAccumRec->llVisaTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		/* 退貨 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnVisaTotalRefundCount);
		sprintf(szTemplate, "%lld", (0 - srAccumRec->llVisaTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		/* 小計 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnVisaTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llVisaTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		/* 小費 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnVisaTotalTipsCount);
		sprintf(szTemplate, "%lld", srAccumRec->llVisaTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llMasterTotalSaleAmount != 0L || srAccumRec->llMasterTotalRefundAmount != 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "卡別 　　%s", "MASTER CARD");
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

		/* 銷售 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnMasterTotalSaleCount);
		sprintf(szTemplate, "%lld", (srAccumRec->llMasterTotalSaleAmount + srAccumRec->llMasterTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		/* 退貨 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnMasterTotalRefundCount);
		sprintf(szTemplate, "%lld", (0 - srAccumRec->llMasterTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		/* 小計 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnMasterTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llMasterTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		/* 小費 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnMasterTotalTipsCount);
		sprintf(szTemplate, "%lld", srAccumRec->llMasterTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llJcbTotalSaleAmount != 0L || srAccumRec->llJcbTotalRefundAmount != 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_JCB_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

		/* 銷售 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnJcbTotalSaleCount);
		sprintf(szTemplate, "%lld", (srAccumRec->llJcbTotalSaleAmount + srAccumRec->llJcbTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		/* 退貨 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnJcbTotalRefundCount);
		sprintf(szTemplate, "%lld", (0 - srAccumRec->llJcbTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		/* 小計 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnJcbTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llJcbTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		/* 小費 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnJcbTotalTipsCount);
		sprintf(szTemplate, "%lld", srAccumRec->llJcbTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llAmexTotalSaleAmount != 0L || srAccumRec->llAmexTotalRefundAmount != 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_AMEX_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

		/* 銷售 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnAmexTotalSaleCount);
		sprintf(szTemplate, "%lld", (srAccumRec->llAmexTotalSaleAmount + srAccumRec->llAmexTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		/* 退貨 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnAmexTotalRefundCount);
		sprintf(szTemplate, "%lld", (0 - srAccumRec->llAmexTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		/* 小計 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnAmexTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llAmexTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		/* 小費 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnAmexTotalTipsCount);
		sprintf(szTemplate, "%lld", srAccumRec->llAmexTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llCupTotalSaleAmount != 0L || srAccumRec->llCupTotalRefundAmount != 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_CUP_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

		/* 銷售 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnCupTotalSaleCount);
		sprintf(szTemplate, "%lld", (srAccumRec->llCupTotalSaleAmount + srAccumRec->llCupTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		/* 退貨 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnCupTotalRefundCount);
		sprintf(szTemplate, "%lld", (0 - srAccumRec->llCupTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		/* 小計 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnCupTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llCupTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		/* 小費 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnCupTotalTipsCount);
		sprintf(szTemplate, "%lld", srAccumRec->llCupTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llDinersTotalSaleAmount != 0L || srAccumRec->llDinersTotalRefundAmount != 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_DINERS_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

		/* 銷售 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnDinersTotalSaleCount);
		sprintf(szTemplate, "%lld", (srAccumRec->llDinersTotalSaleAmount + srAccumRec->llDinersTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		/* 退貨 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnDinersTotalRefundCount);
		sprintf(szTemplate, "%lld", (0 - srAccumRec->llDinersTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		/* 小計 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnDinersTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llDinersTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		/* 小費 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnDinersTotalTipsCount);
		sprintf(szTemplate, "%lld", srAccumRec->llDinersTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llFiscTotalSaleAmount != 0L || srAccumRec->llFiscTotalRefundAmount != 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_SMARTPAY_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

		/* 銷售 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnFiscTotalSaleCount);
		sprintf(szTemplate, "%lld", (srAccumRec->llFiscTotalSaleAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		/* 退貨 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnFiscTotalRefundCount);
		sprintf(szTemplate, "%lld", (0 - srAccumRec->llFiscTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		/* 小計 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnFiscTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llFiscTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		/* 小費 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小費 　　%03lu   NT$", (unsigned long)0);
		sprintf(szTemplate, "%lld", (long long)0);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByCard_ByBuffer
Date&Time       :2016/2/24 下午 3:49
Describe        :依卡別列印
*/
int inCREDIT_PRINT_TotalAmountByCredit_ByBuffer(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
char    szPrintBuf[84 + 1], szTemplate[84 + 1];

	//if (srAccumRec->llTotalCreditSaleAmount != 0L || srAccumRec->lnTotalCreditRefundCount != 0L)
	{
		inPRINT_Buffer_PutIn("	     一般交易總額報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 銷售 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnTotalCreditSaleCount);
		sprintf(szTemplate, "%lld", (srAccumRec->llTotalCreditSaleAmount + srAccumRec->llTotalCreditTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		/* 退貨 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnTotalCreditRefundCount);
		if(srAccumRec->lnTotalCreditRefundCount != 0)
			sprintf(szTemplate, "%lld", (0 - srAccumRec->llTotalCreditRefundAmount));
		else
			sprintf(szTemplate, "%lld", srAccumRec->llTotalCreditRefundAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		/* 總額 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "總額 　　%03lu   NT$", srAccumRec->lnTotalCreditCount);
		//if (srAccumRec->llTotalCreditAmount >= 0L)
			sprintf(szTemplate, "%lld", srAccumRec->llTotalCreditAmount);
		//else
		//	sprintf(szTemplate, "%lld", (0 - srAccumRec->llTotalCreditAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		/* 小費 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnTotalTipsCount);
		sprintf(szTemplate, "%lld", srAccumRec->llTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByInstllment_ByBuffer
Date&Time       :2016/2/24 下午 3:50
Describe        :
*/
int inCREDIT_PRINT_TotalAmountByInstllment_ByBuffer(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char    szPrintBuf[84 + 1], szTemplate[84 + 1];


	inPRINT_Buffer_PutIn("       分期總額報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	//inPRINT_Buffer_PutIn("       分期交易總額", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	//inPRINT_Buffer_PutIn("    筆數(CNT)      金額(AMOUNT)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 銷售 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "銷售   　%03lu   NT$", srAccumRec->lnInstSaleCount);
	sprintf(szTemplate, "%lld", (srAccumRec->llInstSaleAmount + srAccumRec->llInstTipsAmount));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "退貨   　%03lu   NT$", srAccumRec->lnInstRefundCount);
	sprintf(szTemplate, "%lld", (0 - srAccumRec->llInstRefundAmount));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 淨額 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "總額   　%03lu   NT$", srAccumRec->lnInstTotalCount);
	//sprintf(szPrintBuf, "淨額 Ｔ　%03lu   NT$", srAccumRec->lnInstTotalCount);
	sprintf(szTemplate, "%lld", srAccumRec->llInstTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 小費 */
#if 0
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnInstTipsCount);
	sprintf(szTemplate, "%lld", srAccumRec->llInstTipsAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
#endif
	inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByRedemption_ByBuffer
Date&Time       :2016/2/24 下午 3:50
Describe        :
*/
int inCREDIT_PRINT_TotalAmountByRedemption_ByBuffer(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char    szPrintBuf[84 + 1], szTemplate[84 + 1];

	inPRINT_Buffer_PutIn("       紅利總額報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	//inPRINT_Buffer_PutIn("       紅利扣抵總額", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	//inPRINT_Buffer_PutIn("    筆數(CNT)      金額(AMOUNT)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 銷售 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "銷售   　%03lu   NT$", srAccumRec->lnRedeemSaleCount);
	sprintf(szTemplate, "%lld", (srAccumRec->llRedeemSaleAmount + srAccumRec->llRedeemTipsAmount));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "退貨   　%03lu   NT$", srAccumRec->lnRedeemRefundCount);
	sprintf(szTemplate, "%lld", (0 - srAccumRec->llRedeemRefundAmount));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 淨額 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "總額   　%03lu   NT$", srAccumRec->lnRedeemTotalCount);
	//sprintf(szPrintBuf, "淨額 Ｔ　%03lu   NT$", srAccumRec->lnRedeemTotalCount);
	sprintf(szTemplate, "%lld", srAccumRec->llRedeemTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 小費 */
#if 0
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnRedeemTipsCount);
	sprintf(szTemplate, "%lld", srAccumRec->llRedeemTipsAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("-----------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 紅利扣抵總點數 */
	/* 初始化 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

	/* 將NT$ ＋數字塞到szTemplate中來inpad */
	sprintf(szTemplate, " %ld", (srAccumRec->lnRedeemTotalPoint));
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PAD_RIGHT_FILL_LEFT_);

	/* 把前面的字串和數字結合起來 */
	sprintf(szPrintBuf, "紅利扣抵總點數 :%s點", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
#endif

	/* 結束隔線 */
	inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByOther
Date&Time       :2017/4/14 下午 1:26
Describe        :
*/
int inCREDIT_PRINT_TotalAmountByOther(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal;
	char	szESCMode[2 + 1];
	char	szPrintBuf[100 + 1];
	char	szTRTFileName[12 + 1];

	/* 若找不到ESC或ESC沒開或水位為0，不送欄位NE */
	memset(szESCMode, 0x00, sizeof(szESCMode));
	inGetESCMode(szESCMode);
	if (memcmp(szESCMode, "Y", strlen("Y")) != 0)
	{
		return (VS_SUCCESS);
	}
	else
        {
		/* 如果不是結帳就不用印 */
		if (pobTran->inRunOperationID != _OPERATION_SETTLE_)
		{
			return (VS_SUCCESS);
		}

		memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
		inGetTRTFileName(szTRTFileName);

		if (!memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)) ||
		    !memcmp(szTRTFileName, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)))
		{
			inRetVal = inPRINT_Buffer_PutIn("------------------------------------------", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			
			/* 電簽已上傳筆數 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "電簽已上傳總筆數 = ");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
						
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%03lu", srAccumRec->lnESC_SuccessNum);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

			
			/* 電簽已上傳金額 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "電簽已上傳總金額 = NT$");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			if (srAccumRec->llESC_SuccessAmount >= 0L)
			{
				sprintf(szPrintBuf, "%lld", srAccumRec->llESC_SuccessAmount);
				inFunc_Amount_Comma(szPrintBuf, "", ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);
			}
			else
			{
				sprintf(szPrintBuf, "%lld", (0 - srAccumRec->llESC_SuccessAmount));				
				inFunc_Amount_Comma(szPrintBuf, "", ' ', _SIGNED_MINUS_, 0, _DO_NOT_NEED_PAD_);
			}
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			
			inRetVal = inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 電簽未上傳筆數 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "紙本簽單總筆數 =");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

			/* 紙本簽單總筆數 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%03lu", (srAccumRec->lnESC_TotalFailULNum + srAccumRec->lnESC_RePrintNum));
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

			
			/* 紙本簽單總金額 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "紙本簽單總金額 = NT$");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			if (srAccumRec->llESC_TotalFailULAmount >= 0L)
			{
				sprintf(szPrintBuf, "%lld", srAccumRec->llESC_TotalFailULAmount);
				inFunc_Amount_Comma(szPrintBuf, "", ' ', _SIGNED_NONE_, 0, _DO_NOT_NEED_PAD_);
			}
			else
			{
				sprintf(szPrintBuf, "%lld", (0 - srAccumRec->llESC_TotalFailULAmount));
				inFunc_Amount_Comma(szPrintBuf, "", ' ', _SIGNED_MINUS_, 0, _DO_NOT_NEED_PAD_);
			}
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

			
			inRetVal = inPRINT_Buffer_PutIn("------------------------------------------", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

#if 0			/* 富邦不會統記這個   [SAM]20181214*/
			/* ESC預先授權列印紙本及預先授權不納入結帳總額 */
			inRetVal = inPRINT_Buffer_PutIn("＊以上統計不含預先授權交易", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inRetVal = inPRINT_Buffer_PutIn("＊預先授權", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "　%03lu", srAccumRec->lnESC_PreAuthNum);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "　紙本簽單總筆數 = ");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紙本簽單總金額 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%ld", (long)srAccumRec->llESC_PreAuthAmount);
			inFunc_Amount_Comma(szPrintBuf, "NT$ ", ' ', _SIGNED_NONE_, inSpace, _PAD_RIGHT_FILL_LEFT_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "　紙本簽單總金額 = ");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inRetVal = inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* ESC補強機制 */

			inCREDIT_PRINT_ESC_Reinforce_Count_ByBuffer(pobTran, uszBuffer, srFont_Attrib, srBhandle);
#endif
                }
        }

	inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Total_Loyalty_Redeem_ByBuffer
Date&Time       :2017/2/22 上午 10:07
Describe        :列印優惠兌換
*/
int inCREDIT_PRINT_Total_Loyalty_Redeem_ByBuffer(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char    szPrintBuf[84 + 1], szTemplate[84 + 1];

        inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 優惠兌換成功總數量 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "%ld 張", srAccumRec->lnLoyaltyRedeemSuccessCount);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "優惠兌換成功總數量");
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 優惠兌換取消總數量 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "%ld 張", srAccumRec->lnLoyaltyRedeemCancelCount);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "優惠兌換取消總數量");
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 優惠兌換合計 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "%ld 張", srAccumRec->lnLoyaltyRedeemTotalCount);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "優惠兌換合計");
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_END_ByBuffer
Date&Time       :2016/2/24 下午 3:50
Describe        :列印結尾
*/
int inCREDIT_PRINT_End_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;

        inPRINT_Buffer_PutIn("     *** 列印完成 ***", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        for (i = 0; i < 8; i++)
	{
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DetailReportMiddle_ByBuffer
Date&Time       :2016/2/24 下午 3:52
Describe        :
*/
int inCREDIT_PRINT_DetailReportMiddle_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szFuncEnable[1 + 1];			/* catch Y or N */

        inPRINT_Buffer_PutIn("明細報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

	inPRINT_Buffer_PutIn("調閱編號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn_Specific_X_Position("交易金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

        inPRINT_Buffer_PutIn("交易類別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        inPRINT_Buffer_PutIn("卡號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	inPRINT_Buffer_PutIn("交易日期", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn_Specific_X_Position("交易時間", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

	inPRINT_Buffer_PutIn("授權碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 櫃號功能有開才印櫃號 */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
        inGetStoreIDEnable(szFuncEnable);
        if (szFuncEnable[0] == 'Y')
                inPRINT_Buffer_PutIn("櫃號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_NCCC_DetailReportMiddle_ByBuffer
Date&Time       :2017/5/17 下午 2:20
Describe        :
*/
int inCREDIT_PRINT_NCCC_DetailReportMiddle_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szFuncEnable[1 + 1];			/* catch Y or N */
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];

	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);

	/* 商店自存聯卡號遮掩 */
	if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0)
	{
		inPRINT_Buffer_PutIn("明細報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

		inPRINT_Buffer_PutIn("調閱編號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("交易金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_04_);

		inPRINT_Buffer_PutIn("交易類別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("卡號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("交易編號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("檢查碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("交易日期", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("交易時間", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_04_);

		/* SmartPay要印調單編號 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetFiscFuncEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
		{
			inPRINT_Buffer_PutIn("授權碼/調單編號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			inPRINT_Buffer_PutIn("授權碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		/* 銀聯功能有開才印回覆碼 */
		/*memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetCUPFuncEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
		{
			inPRINT_Buffer_PutIn("回覆碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}*/

		/* 櫃號功能有開才印櫃號 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetStoreIDEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
			inPRINT_Buffer_PutIn("櫃號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 優惠平台有開才印 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetASMFlag(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
			inPRINT_Buffer_PutIn("優惠類別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	else
	{
		inPRINT_Buffer_PutIn("明細報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

		inPRINT_Buffer_PutIn("交易類別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("交易金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_04_);

		//inPRINT_Buffer_PutIn("交易類別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("卡號(卡別)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("交易時間", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_04_);

		inPRINT_Buffer_PutIn("交易日期", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("調閱編號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_04_);

		/* SmartPay要印調單編號 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetFiscFuncEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
		{
			inPRINT_Buffer_PutIn("授權碼/調單編號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		}
		else
		{
			inPRINT_Buffer_PutIn("授權碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		}

		inPRINT_Buffer_PutIn_Specific_X_Position("簽單狀態", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_04_);
		
		/* 銀聯功能有開才印回覆碼 */
		/*memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetCUPFuncEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
		{
			inPRINT_Buffer_PutIn("回覆碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}*/

		inPRINT_Buffer_PutIn("序號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 櫃號功能有開才印櫃號 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetStoreIDEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
			inPRINT_Buffer_PutIn("櫃號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 優惠平台有開才印 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetASMFlag(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
			inPRINT_Buffer_PutIn("優惠類別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("分期交易金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("分期交易期數", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("分期首期金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("分期每期金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("分期手續費", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("紅利交易金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("紅利折抵金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("紅利本次折抵點數", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("紅利剩餘點數", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("實付金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	}
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DetailReportBottom_ByBuffer
Date&Time       :2016/2/24 下午 3:52
Describe        :
*/
int inCREDIT_PRINT_DetailReportBottom_ByBuffer(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i, j;
	int	inReadCnt = 0;
	int	inCardLen = 0;
	int	inRetVal = VS_SUCCESS;
	int	inSelectFoneSize;
	char	szPrintBuf[62 + 1], szTemplate1[62 + 1], szTemplate2[62 + 1];
	char	szFuncEnable[1 + 1];			/* catch Y or N */
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCREDIT_PRINT_DetailReportBottom()_START");
	}

	inSelectFoneSize = _PRT_HEIGHT_;
	
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);

	/* 開始讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);

	for (inReadCnt = 0; inReadCnt < inRecordCnt; inReadCnt ++)
	{
		/*. 開始讀取每一筆交易記錄 .*/
		if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, inReadCnt) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
			break;
		}

		/* 優惠兌換 */
		if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_		||
		    pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
		{
			/* Invoice Number */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			sprintf(szTemplate1, "INV:%06ld", pobTran->srBRec.lnOrgInvNum);
			strcat(szPrintBuf, szTemplate1);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Trans Date Time */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "DATE: %.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "TIME: %.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 交易別 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_)
			{
				sprintf(szPrintBuf, "優惠兌換");
			}
			else if (pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
			{
				sprintf(szPrintBuf, "兌換取消");
			}
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			continue;
		}
		else
		{
			/* 商店自存聯卡號遮掩 */
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0)
			{
				/* Invoice Number */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				if (pobTran->srBRec.uszVOIDBit != VS_TRUE)
				{
					sprintf(szTemplate1, "INV.  NO = %06ld", pobTran->srBRec.lnOrgInvNum);
				}
				else
				{
					sprintf(szTemplate1, "*INV. NO = %06ld", pobTran->srBRec.lnOrgInvNum);
				}

				/* 【需求單 - 105259】總額明細報表及總額明細查詢補強機制 商店聯出紙本要印<P> */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetESCMode(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0 && pobTran->srBRec.inESCUploadStatus == _ESC_UPLOAD_STATUS_PAPER_)
				{
					strcat(szTemplate1, " <P>");
				}

				strcat(szPrintBuf, szTemplate1);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* Print Amount */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));

				if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
				{
					switch (pobTran->srBRec.inCode)
					{
						case _SALE_:
						case _INST_SALE_ :
						case _REDEEM_SALE_ :
						case _MAIL_ORDER_ :
						case _CUP_MAIL_ORDER_ :
						case _SALE_OFFLINE_ :
						case _PRE_COMP_ :
						case _PRE_AUTH_ :
						case _CUP_SALE_ :
						case _CUP_PRE_COMP_ :
						case _CUP_PRE_AUTH_ :
						case _FISC_SALE_ :
							sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
							break;
						case _TIP_ :
							sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
							break;
						case _REFUND_ :
						case _INST_REFUND_ :
						case _REDEEM_REFUND_ :
						case _CUP_REFUND_ :
						case _CUP_MAIL_ORDER_REFUND_ :
						case _FISC_REFUND_ :
							sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
							break;
						case _INST_ADJUST_ :
						case _REDEEM_ADJUST_ :
							sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
							break;
						case _ADJUST_ :
							sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
							break;
						default :
							memset(szTemplate2, 0x00, sizeof(szTemplate2));
							inGetHostLabel(szTemplate2);
							sprintf(szTemplate1,"%s_AMT_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
							break;
					} /* End switch () */
				}
				else
				{
					switch (pobTran->srBRec.inOrgCode)
					{
						 case _SALE_:
						case _INST_SALE_ :
						case _REDEEM_SALE_ :
						case _MAIL_ORDER_ :
						case _CUP_MAIL_ORDER_ :
						case _SALE_OFFLINE_ :
						case _PRE_COMP_ :
						case _PRE_AUTH_ :
						case _CUP_SALE_ :
						case _CUP_PRE_COMP_ :
						case _CUP_PRE_AUTH_ :
						case _FISC_SALE_ :
							sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
							break;
						/* NCCC小費不能取消 */
	//	                                case _TIP_ :
	//	                                        sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
	//	                                        break;
						case _REFUND_ :
						case _INST_REFUND_ :
						case _REDEEM_REFUND_ :
						case _CUP_REFUND_ :
						case _CUP_MAIL_ORDER_REFUND_ :
							sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
							break;
						case _INST_ADJUST_ :
						case _REDEEM_ADJUST_ :
							sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
							break;
						case _ADJUST_ :
							sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
							break;
						default :
							memset(szTemplate2, 0x00, sizeof(szTemplate2));
							inGetHostLabel(szTemplate2);
							sprintf(szTemplate1, "%s_AMT_VOID_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
							break;
					} /* End switch () */
				}
				inFunc_Amount_Comma(szTemplate1, "NT$" , 0x00, _SIGNED_NONE_, 16, _PAD_LEFT_FILL_RIGHT_);
				strcat(szPrintBuf, szTemplate1);
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_04_);

				if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

				/* Trans Type */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
				sprintf(szPrintBuf, "%s %s", szTemplate1, szTemplate2);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

				/* 卡號 */
				if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
				{
					memset(szTemplate1, 0x00, sizeof(szTemplate1));
					strcpy(szTemplate1, pobTran->srBRec.szPAN);
					if (pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
					{
						inCardLen = strlen(szTemplate1);

						for (j = 6; j < (inCardLen - 4); j ++)
							szTemplate1[j] = 0x2A;
					}
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "CARD  NO = %s", szTemplate1);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					memset(szTemplate1, 0x00, sizeof(szTemplate1));
					strcpy(szTemplate1, pobTran->srBRec.szPAN);

					if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
					{
						inFunc_FormatPAN_UCARD(szTemplate1);
					}

					/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
					if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
					{
						for (i = 3; i < (strlen(szTemplate1) - 5); i ++)
							szTemplate1[i] = 0x2A;
					}
					else
					{
						for (i = 6; i < (strlen(szTemplate1) - 4); i ++)
							szTemplate1[i] = 0x2A;
					}

					inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 19, _PAD_LEFT_FILL_RIGHT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "CARD  NO = %s",szTemplate1);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}

				/* 交易序號Transaction No. */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, "TXN.  NO = ");
				if (pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
					strcat(szPrintBuf, pobTran->srBRec.szTxnNo);

				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

				/* 檢查碼Check No */
				if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
				{
					/* SmartPay不印檢查碼 */
				}
				else
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memset(szTemplate1, 0x00, sizeof(szTemplate1));
					inCARD_ExpDateEncryptAndDecrypt(pobTran, szTemplate1, szTemplate1, _EXP_ENCRYPT_);
					sprintf(szPrintBuf, "CHECK NO = %s",szTemplate1);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}

				/* Trans Date Time */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "DATE=%.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "TIME=%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_04_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* Approved No. & RRN NO. */
				/* SmartPay印調單編號 */
				if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcpy(szPrintBuf, "RRN NO. = ");
					strcat(szPrintBuf, pobTran->srBRec.szFiscRRN);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcpy(szPrintBuf, "APPROVED CODE = ");
					strcat(szPrintBuf, pobTran->srBRec.szAuthCode);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}

				/* RESPONSE CODE */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetCUPFuncEnable(szFuncEnable);
				if (szFuncEnable[0] == 'Y')
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcpy(szPrintBuf, "RESPONSE CODE = ");
					strcat(szPrintBuf, pobTran->srBRec.szRespCode);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}

				/* Store ID */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetStoreIDEnable(szFuncEnable);
				if (szFuncEnable[0] == 'Y')
				{
					/*開啟櫃號功能*/
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "STORE ID: %s", pobTran->srBRec.szStoreID);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
				       /*沒開啟櫃號功能，則不印櫃號*/
				}

//				/* 列印優惠資訊 */
//				if (inNCCC_Loyalty_ASM_Flag() != VS_SUCCESS)
//				{
//					inRetVal = inPRINT_Buffer_PutIn("", inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
//					if (inRetVal != VS_SUCCESS)
//						return (VS_ERROR);
//				}
//				else
				{
					/* 原交易有優惠資訊，取消交易後，非小費及暫停優惠服務，則列印取消優惠資訊 */
					if (pobTran->srBRec.uszVOIDBit == VS_TRUE	&&
					   (pobTran->srBRec.uszRewardL1Bit == VS_TRUE	||
					    pobTran->srBRec.uszRewardL2Bit == VS_TRUE	||
					    pobTran->srBRec.uszRewardL5Bit == VS_TRUE)	&&
					    atoi(pobTran->srBRec.szAwardNum) > 0	&&
					    pobTran->srBRec.uszRewardSuspendBit != VS_TRUE)
					{
						inRetVal = inPRINT_Buffer_PutIn("取消優惠", inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
					else if (pobTran->srBRec.uszVOIDBit != VS_TRUE		&&
						(pobTran->srBRec.uszRewardL1Bit == VS_TRUE	||
						 pobTran->srBRec.uszRewardL2Bit == VS_TRUE	||
					         pobTran->srBRec.uszRewardL5Bit == VS_TRUE))
					{
						inRetVal = inPRINT_Buffer_PutIn("優惠", inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
					else
					{
						inRetVal = inPRINT_Buffer_PutIn("", inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}

				}
			}
			else
			{
				char chAmt[50];

				memset(chAmt, 0x00, sizeof(chAmt));
				/* Trans Type */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
				sprintf(szPrintBuf, "%s %s", szTemplate1, szTemplate2);
				inRetVal = inPRINT_Buffer_PutIn(szTemplate1, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* Print Amount */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));

				if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
				{
					switch (pobTran->srBRec.inCode)
					{
						case _SALE_:
						case _INST_SALE_ :
						case _REDEEM_SALE_ :
						case _MAIL_ORDER_ :
						case _CUP_MAIL_ORDER_ :
						case _SALE_OFFLINE_ :
						case _PRE_COMP_ :
						case _PRE_AUTH_ :
						case _CUP_SALE_ :
						case _CUP_PRE_COMP_ :
						case _CUP_PRE_AUTH_ :
						case _FISC_SALE_ :
							sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
							break;
						case _TIP_ :
							sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
							break;
						case _REFUND_ :
						case _INST_REFUND_ :
						case _REDEEM_REFUND_ :
						case _CUP_REFUND_ :
						case _CUP_MAIL_ORDER_REFUND_ :
						case _FISC_REFUND_ :
							sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
							break;
						case _INST_ADJUST_ :
						case _REDEEM_ADJUST_ :
							sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
							break;
						case _ADJUST_ :
							sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
							break;
						default :
							memset(szTemplate2, 0x00, sizeof(szTemplate2));
							inGetHostLabel(szTemplate2);
							sprintf(szTemplate1,"%s_AMT_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
							break;
					} /* End switch () */
				}
				else
				{
					switch (pobTran->srBRec.inOrgCode)
					{
						 case _SALE_:
						case _INST_SALE_ :
						case _REDEEM_SALE_ :
						case _MAIL_ORDER_ :
						case _CUP_MAIL_ORDER_ :
						case _SALE_OFFLINE_ :
						case _PRE_COMP_ :
						case _PRE_AUTH_ :
						case _CUP_SALE_ :
						case _CUP_PRE_COMP_ :
						case _CUP_PRE_AUTH_ :
						case _FISC_SALE_ :
							sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
							break;
						/* NCCC小費不能取消 */
//						case _TIP_ :
//							sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
//							break;
						case _REFUND_ :
						case _INST_REFUND_ :
						case _REDEEM_REFUND_ :
						case _CUP_REFUND_ :
						case _CUP_MAIL_ORDER_REFUND_ :
							sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
							break;
						case _INST_ADJUST_ :
						case _REDEEM_ADJUST_ :
							sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
							break;
						case _ADJUST_ :
							sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
							break;
						default :
							memset(szTemplate2, 0x00, sizeof(szTemplate2));
							inGetHostLabel(szTemplate2);
							sprintf(szTemplate1, "%s_AMT_VOID_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
							break;
					} /* End switch () */
				}
				/* 預防長度過長  20190903 [SAM] */
				if(strlen(szTemplate1) < 8 )
				{
					inFunc_Amount_Comma(szTemplate1, "NT$" , 0x00, _SIGNED_NONE_, 16, _PAD_LEFT_FILL_RIGHT_);
					memcpy(chAmt,szTemplate1,strlen(szTemplate1));
					strcat(szPrintBuf, szTemplate1);
					inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_04_);
				}else{
					inFunc_Amount_Comma(szTemplate1, "NT$" , 0x00, _SIGNED_NONE_, 16, _PAD_LEFT_FILL_RIGHT_);
					memcpy(chAmt,szTemplate1,strlen(szTemplate1));
					strcat(szPrintBuf, szTemplate1);
					inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, (_PRINT_DEFINE_X_04_- 10));
					
				}

				

				if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

				/* 卡號(卡別) */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, pobTran->srBRec.szPAN);
				/* 預防長度過長  20190903 [SAM] */
				if(strlen(pobTran->srBRec.szPAN) < 19)
				{
					if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)) == 0)
						strcat(szPrintBuf, "(V)");
					else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
						strcat(szPrintBuf, "(M)");
					else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)) == 0)
						strcat(szPrintBuf, "(J)");
					else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
						strcat(szPrintBuf, "(U)");
					else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
						strcat(szPrintBuf, "(A)");
					else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0)
						strcat(szPrintBuf, "(D)");
					else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_)) == 0)
						strcat(szPrintBuf, "(CUP)");
					inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 25, _PAD_LEFT_FILL_RIGHT_);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

					/* Time */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "TIME: %.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
					inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_04_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}else{
				
					
					inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 25, _PAD_LEFT_FILL_RIGHT_);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

					/* Time */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "TIME: %.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
					inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_04_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
					
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)) == 0)
						strcat(szPrintBuf, "(V)");
					else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
						strcat(szPrintBuf, "(M)");
					else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)) == 0)
						strcat(szPrintBuf, "(J)");
					else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
						strcat(szPrintBuf, "(U)");
					else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
						strcat(szPrintBuf, "(A)");
					else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0)
						strcat(szPrintBuf, "(D)");
					else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_)) == 0)
						strcat(szPrintBuf, "(CUP)");
					inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 6, _PAD_LEFT_FILL_RIGHT_);
					
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
				

				/* Date */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "DATE: %.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* Invoice Number */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				if (pobTran->srBRec.uszVOIDBit != VS_TRUE)
				{
					sprintf(szTemplate1, "INV:%06ld", pobTran->srBRec.lnOrgInvNum);
				}
				else
				{
					sprintf(szTemplate1, "*INV:%06ld", pobTran->srBRec.lnOrgInvNum);
				}
				
				/* 【需求單 - 105259】總額明細報表及總額明細查詢補強機制 商店聯出紙本要印<P> */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetESCMode(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0 && pobTran->srBRec.inESCUploadStatus == _ESC_UPLOAD_STATUS_PAPER_)
				{
					strcat(szTemplate1, " <P>");
				}
				
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szTemplate1, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_04_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				/* Approved No. */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, "APPR: ");
				strcat(szPrintBuf, pobTran->srBRec.szAuthCode);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* RRN. */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szRefNo, ' ', 12, _PAD_LEFT_FILL_RIGHT_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* Store ID */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetStoreIDEnable(szFuncEnable);
				if (szFuncEnable[0] == 'Y')
				{
					/*開啟櫃號功能*/
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "STORE ID:%s", pobTran->srBRec.szStoreID);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}

				if(pobTran->srBRec.uszInstallmentBit)
				{
#if 0
					/* 分期交易金額 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "INSTALLMENT AMOUNT     :%s",chAmt);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

					/* 分期交易期數 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "INSTALLMENT PERIOD     :%02ld",pobTran->srBRec.lnInstallmentPeriod);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

					/* 分期首期金額 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _INST_REFUND_) ||
						(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _INST_SALE_ || pobTran->srBRec.inOrgCode == _INST_ADJUST_)))
					//if(pobTran->srBRec.inCode == _INST_REFUND_)
						sprintf(szPrintBuf, "INSTALLMENT DOWNPAYMANT:NT$%ld",(0 - pobTran->srBRec.lnInstallmentDownPayment));
					else
						sprintf(szPrintBuf, "INSTALLMENT DOWNPAYMANT:NT$%ld",pobTran->srBRec.lnInstallmentDownPayment);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

					/* 分期每期金額 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _INST_REFUND_) ||
						(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _INST_SALE_ || pobTran->srBRec.inOrgCode == _INST_ADJUST_)))
					//if(pobTran->srBRec.inCode == _INST_REFUND_)
						sprintf(szPrintBuf, "INSTALLMENT PAYMANT    :NT$%ld",(0 - pobTran->srBRec.lnInstallmentPayment));
					else
						sprintf(szPrintBuf, "INSTALLMENT PAYMANT    :NT$%ld",pobTran->srBRec.lnInstallmentPayment);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

					/* 分期手續費 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _INST_REFUND_) ||
						(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _INST_SALE_ || pobTran->srBRec.inOrgCode == _INST_ADJUST_)))
					//if(pobTran->srBRec.inCode == _INST_REFUND_)
						sprintf(szPrintBuf, "INSTALLMENT FEE        :NT$%ld",(0 - pobTran->srBRec.lnInstallmentFormalityFee));
					else
						sprintf(szPrintBuf, "INSTALLMENT FEE        :NT$%ld",pobTran->srBRec.lnInstallmentFormalityFee);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
#else				
					/* 分期交易金額 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "INST. AMOUNT     :%s",chAmt);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

					/* 分期交易期數 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "INST. PERIOD     :%02ld",pobTran->srBRec.lnInstallmentPeriod);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

					/* 分期首期金額 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _INST_REFUND_) ||
						(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _INST_SALE_ || pobTran->srBRec.inOrgCode == _INST_ADJUST_)))
					//if(pobTran->srBRec.inCode == _INST_REFUND_)
						sprintf(szPrintBuf, "INST. DOWNPAYMANT:NT$%ld",(0 - pobTran->srBRec.lnInstallmentDownPayment));
					else
						sprintf(szPrintBuf, "INST. DOWNPAYMANT:NT$%ld",pobTran->srBRec.lnInstallmentDownPayment);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

					/* 分期每期金額 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _INST_REFUND_) ||
						(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _INST_SALE_ || pobTran->srBRec.inOrgCode == _INST_ADJUST_)))
					//if(pobTran->srBRec.inCode == _INST_REFUND_)
						sprintf(szPrintBuf, "INST. PAYMANT    :NT$%ld",(0 - pobTran->srBRec.lnInstallmentPayment));
					else
						sprintf(szPrintBuf, "INST. PAYMANT    :NT$%ld",pobTran->srBRec.lnInstallmentPayment);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

					/* 分期手續費 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _INST_REFUND_) ||
						(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _INST_SALE_ || pobTran->srBRec.inOrgCode == _INST_ADJUST_)))
					//if(pobTran->srBRec.inCode == _INST_REFUND_)
						sprintf(szPrintBuf, "INST. FEE        :NT$%ld",(0 - pobTran->srBRec.lnInstallmentFormalityFee));
					else
						sprintf(szPrintBuf, "INST. FEE        :NT$%ld",pobTran->srBRec.lnInstallmentFormalityFee);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
#endif
				}

				if(pobTran->srBRec.uszRedeemBit)
				{
#if 0
					/* 紅利交易金額 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _REDEEM_REFUND_) ||
						(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _REDEEM_SALE_ || pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_)))
					//if(pobTran->srBRec.inCode == _REDEEM_REFUND_)
						sprintf(szPrintBuf, "REDEMPTION AMOUNT        :NT$%ld",(0 - pobTran->srBRec.lnTxnAmount));
					else
						sprintf(szPrintBuf, "REDEMPTION AMOUNT        :NT$%ld",pobTran->srBRec.lnTxnAmount);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

					/* 紅利折抵金額 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _REDEEM_REFUND_) ||
						(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _REDEEM_SALE_ || pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_)))
					//if(pobTran->srBRec.inCode == _REDEEM_REFUND_)
						sprintf(szPrintBuf, "REDEMPTION PAID AMT      :NT$%ld",(0 - pobTran->srBRec.lnRedemptionPaidAmount));
					else
						sprintf(szPrintBuf, "REDEMPTION PAID AMT      :NT$%ld",pobTran->srBRec.lnRedemptionPaidAmount);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

					/* 紅利本次折抵點數 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _REDEEM_REFUND_) ||
						(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _REDEEM_SALE_ || pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_)))
					//if(pobTran->srBRec.inCode == _REDEEM_REFUND_)
						sprintf(szPrintBuf, "REDEMPTION POINTS        :%ld",(0 - pobTran->srBRec.lnRedemptionPoints));
					else
						sprintf(szPrintBuf, "REDEMPTION POINTS        :%ld",pobTran->srBRec.lnRedemptionPoints);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

					/* 紅利剩餘點數 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "REDEMPTION POINTSBALANCE :%ld",pobTran->srBRec.lnRedemptionPointsBalance);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

					/* 實付金額 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _REDEEM_REFUND_) ||
						(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _REDEEM_SALE_ || pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_)))
					//if(pobTran->srBRec.inCode == _REDEEM_REFUND_)
						sprintf(szPrintBuf, "REDEMPTION PAIDCREDITAMT :NT$%ld",(0 - pobTran->srBRec.lnRedemptionPaidCreditAmount));
					else
						sprintf(szPrintBuf, "REDEMPTION PAIDCREDITAMT :NT$%ld",pobTran->srBRec.lnRedemptionPaidCreditAmount);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
#else					
					/* 紅利交易金額 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//					if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _REDEEM_REFUND_) ||
//						(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _REDEEM_SALE_ || pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_)))
//					//if(pobTran->srBRec.inCode == _REDEEM_REFUND_)
//						sprintf(szPrintBuf, "REDE. AMOUNT        :NT$%ld",(0 - pobTran->srBRec.lnTxnAmount));
//					else
//						sprintf(szPrintBuf, "REDE. AMOUNT        :NT$%ld",pobTran->srBRec.lnTxnAmount);
					if(strlen(chAmt) > 11 )
						sprintf(szPrintBuf, "REDE. AMOUNT     :%s",chAmt);
					else
						sprintf(szPrintBuf, "REDE. AMOUNT        :%s",chAmt);
					
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

					/* 紅利折抵金額 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _REDEEM_REFUND_) ||
						(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _REDEEM_SALE_ || pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_)))
						sprintf(szPrintBuf, "REDE. PAID AMT      :NT$%ld",(0 - pobTran->srBRec.lnRedemptionPaidAmount));
					else
						sprintf(szPrintBuf, "REDE. PAID AMT      :NT$%ld",pobTran->srBRec.lnRedemptionPaidAmount);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

					/* 紅利本次折抵點數 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _REDEEM_REFUND_) ||
						(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _REDEEM_SALE_ || pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_)))
						sprintf(szPrintBuf, "REDE. POINTS        :%ld",(0 - pobTran->srBRec.lnRedemptionPoints));
					else
						sprintf(szPrintBuf, "REDE. POINTS        :%ld",pobTran->srBRec.lnRedemptionPoints);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

					/* 紅利剩餘點數 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "REDE. POINTSBALANCE :%ld",pobTran->srBRec.lnRedemptionPointsBalance);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

					/* 實付金額 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _REDEEM_REFUND_) ||
						(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _REDEEM_SALE_ || pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_)))
						sprintf(szPrintBuf, "REDE. PAIDCREDITAMT :NT$%ld",(0 - pobTran->srBRec.lnRedemptionPaidCreditAmount));
					else
						sprintf(szPrintBuf, "REDE. PAIDCREDITAMT :NT$%ld",pobTran->srBRec.lnRedemptionPaidCreditAmount);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
#endif
				}
			}/* 商店聯卡號遮掩 END */

		}/* 優惠兌換 */

        inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
    } /* End for () .... */

	/* 結束讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCREDIT_PRINT_DetailReportBottom()_END");
	}

	return (inRetVal);
}

int inCREDIT_PRINT_DetailReportBottomByBufferBigWord(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i, j;
	int	inReadCnt = 0;
	int	inCardLen = 0;
	int	inRetVal = VS_SUCCESS;
	int	inSelectFoneSize, inAmtXAddr, inHeaderAmtXAddr, inOtherAmtXAddr, inTempLen;
	char	szPrintBuf[62 + 1], szTemplate1[62 + 1], szTemplate2[62 + 1];
	char	szFuncEnable[1 + 1];			/* catch Y or N */
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inSelectFoneSize = _PRT_HEIGHT_;
	
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);

	/* 開始讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);

	for (inReadCnt = 0; inReadCnt < inRecordCnt; inReadCnt ++)
	{
		/*. 開始讀取每一筆交易記錄 .*/
		if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, inReadCnt) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
			break;
		}

		/* 商店自存聯卡號遮掩 */
		if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0)
		{
			/* Invoice Number */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			if (pobTran->srBRec.uszVOIDBit != VS_TRUE)
			{
				sprintf(szTemplate1, "INV.  NO = %06ld", pobTran->srBRec.lnOrgInvNum);
			}
			else
			{
				sprintf(szTemplate1, "*INV. NO = %06ld", pobTran->srBRec.lnOrgInvNum);
			}

			/* 【需求單 - 105259】總額明細報表及總額明細查詢補強機制 商店聯出紙本要印<P> */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetESCMode(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0 && pobTran->srBRec.inESCUploadStatus == _ESC_UPLOAD_STATUS_PAPER_)
			{
				strcat(szTemplate1, "紙本簽單");
			}

			strcat(szPrintBuf, szTemplate1);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Print Amount */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
			{
				switch (pobTran->srBRec.inCode)
				{
					case _SALE_:
					case _INST_SALE_ :
					case _REDEEM_SALE_ :
					case _MAIL_ORDER_ :
					case _CUP_MAIL_ORDER_ :
					case _SALE_OFFLINE_ :
					case _PRE_COMP_ :
					case _PRE_AUTH_ :
					case _CUP_SALE_ :
					case _CUP_PRE_COMP_ :
					case _CUP_PRE_AUTH_ :
					case _FISC_SALE_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						break;
					case _TIP_ :
						sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						break;
					case _REFUND_ :
					case _INST_REFUND_ :
					case _REDEEM_REFUND_ :
					case _CUP_REFUND_ :
					case _CUP_MAIL_ORDER_REFUND_ :
					case _FISC_REFUND_ :
						sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
						break;
					case _INST_ADJUST_ :
					case _REDEEM_ADJUST_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						break;
					case _ADJUST_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
						break;
					default :
						memset(szTemplate2, 0x00, sizeof(szTemplate2));
						inGetHostLabel(szTemplate2);
						sprintf(szTemplate1,"%s_AMT_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			else
			{
				switch (pobTran->srBRec.inOrgCode)
				{
					 case _SALE_:
					case _INST_SALE_ :
					case _REDEEM_SALE_ :
					case _MAIL_ORDER_ :
					case _CUP_MAIL_ORDER_ :
					case _SALE_OFFLINE_ :
					case _PRE_COMP_ :
					case _PRE_AUTH_ :
					case _CUP_SALE_ :
					case _CUP_PRE_COMP_ :
					case _CUP_PRE_AUTH_ :
					case _FISC_SALE_ :
						sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
						break;
						/* NCCC小費不能取消 */
//	                                case _TIP_ :
//	                                        sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
//	                                        break;
					case _REFUND_ :
					case _INST_REFUND_ :
					case _REDEEM_REFUND_ :
					case _CUP_REFUND_ :
					case _CUP_MAIL_ORDER_REFUND_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						break;
					case _INST_ADJUST_ :
					case _REDEEM_ADJUST_ :
						sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
						break;
					case _ADJUST_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						break;
					default :
						memset(szTemplate2, 0x00, sizeof(szTemplate2));
						inGetHostLabel(szTemplate2);
						sprintf(szTemplate1, "%s_AMT_VOID_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			inFunc_Amount_Comma(szTemplate1, "NT$" , 0x00, _SIGNED_NONE_, 16, _PAD_LEFT_FILL_RIGHT_);
			strcat(szPrintBuf, szTemplate1);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_04_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Trans Type */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
			sprintf(szPrintBuf, "%s %s", szTemplate1, szTemplate2);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

			/* 卡號 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				strcpy(szTemplate1, pobTran->srBRec.szPAN);
				if (pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
				{
					inCardLen = strlen(szTemplate1);

					for (j = 6; j < (inCardLen - 4); j ++)
						szTemplate1[j] = 0x2A;
				}
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "CARD  NO = %s", szTemplate1);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			else
			{
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				strcpy(szTemplate1, pobTran->srBRec.szPAN);

				if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
				{
					inFunc_FormatPAN_UCARD(szTemplate1);
				}

				/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
				if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
				{
					for (i = 3; i < (strlen(szTemplate1) - 5); i ++)
						szTemplate1[i] = 0x2A;
				}
				else
				{
					for (i = 6; i < (strlen(szTemplate1) - 4); i ++)
						szTemplate1[i] = 0x2A;
				}

				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 19, _PAD_LEFT_FILL_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "CARD  NO = %s",szTemplate1);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}

			/* 交易序號Transaction No. */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, "TXN.  NO = ");
			if (pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
				strcat(szPrintBuf, pobTran->srBRec.szTxnNo);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

			/* 檢查碼Check No */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				/* SmartPay不印檢查碼 */
			}
			else
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				inCARD_ExpDateEncryptAndDecrypt(pobTran, szTemplate1, szTemplate1, _EXP_ENCRYPT_);
				sprintf(szPrintBuf, "CHECK NO = %s",szTemplate1);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}

			/* Trans Date Time */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "DATE=%.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "TIME=%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_04_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Approved No. & RRN NO. */
			/* SmartPay印調單編號 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, "RRN NO. = ");
				strcat(szPrintBuf, pobTran->srBRec.szFiscRRN);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			else
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, "APPROVED CODE = ");
				strcat(szPrintBuf, pobTran->srBRec.szAuthCode);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}

			/* RESPONSE CODE */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetCUPFuncEnable(szFuncEnable);
			if (szFuncEnable[0] == 'Y')
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, "RESPONSE CODE = ");
				strcat(szPrintBuf, pobTran->srBRec.szRespCode);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}

			/* Store ID */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetStoreIDEnable(szFuncEnable);
			if (szFuncEnable[0] == 'Y')
			{
				/*開啟櫃號功能*/
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "STORE ID: %s", pobTran->srBRec.szStoreID);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			else
			{
				   /*沒開啟櫃號功能，則不印櫃號*/
			}

		}
		else
		{
			char chAmt[50];

			memset(chAmt, 0x00, sizeof(chAmt));
			/* Trans Type */
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
			sprintf(szPrintBuf, "%s %s", szTemplate1, szTemplate2);
			inRetVal = inPRINT_Buffer_PutIn(szTemplate1, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Print Amount */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
			{
				switch (pobTran->srBRec.inCode)
				{
					case _SALE_:
					case _INST_SALE_ :
					case _REDEEM_SALE_ :
					case _MAIL_ORDER_ :
					case _CUP_MAIL_ORDER_ :
					case _SALE_OFFLINE_ :
					case _PRE_COMP_ :
					case _PRE_AUTH_ :
					case _CUP_SALE_ :
					case _CUP_PRE_COMP_ :
					case _CUP_PRE_AUTH_ :
					case _FISC_SALE_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						break;
					case _TIP_ :
						sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						break;
					case _REFUND_ :
					case _INST_REFUND_ :
					case _REDEEM_REFUND_ :
					case _CUP_REFUND_ :
					case _CUP_MAIL_ORDER_REFUND_ :
					case _FISC_REFUND_ :
						sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
						break;
					case _INST_ADJUST_ :
					case _REDEEM_ADJUST_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						break;
					case _ADJUST_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
						break;
					default :
						memset(szTemplate2, 0x00, sizeof(szTemplate2));
						inGetHostLabel(szTemplate2);
						sprintf(szTemplate1,"%s_AMT_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			else
			{
				switch (pobTran->srBRec.inOrgCode)
				{
					case _SALE_:
					case _INST_SALE_ :
					case _REDEEM_SALE_ :
					case _MAIL_ORDER_ :
					case _CUP_MAIL_ORDER_ :
					case _SALE_OFFLINE_ :
					case _PRE_COMP_ :
					case _PRE_AUTH_ :
					case _CUP_SALE_ :
					case _CUP_PRE_COMP_ :
					case _CUP_PRE_AUTH_ :
					case _FISC_SALE_ :
						sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
						break;
						/* NCCC小費不能取消 */
//						case _TIP_ :
//							sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
//							break;
						case _REFUND_ :
						case _INST_REFUND_ :
						case _REDEEM_REFUND_ :
						case _CUP_REFUND_ :
						case _CUP_MAIL_ORDER_REFUND_ :
							sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
							break;
						case _INST_ADJUST_ :
						case _REDEEM_ADJUST_ :
							sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
							break;
						case _ADJUST_ :
							sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
							break;
						default :
							memset(szTemplate2, 0x00, sizeof(szTemplate2));
							inGetHostLabel(szTemplate2);
							sprintf(szTemplate1, "%s_AMT_VOID_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
							break;
				} /* End switch () */
			}
			/* 預防長度過長  20190903 [SAM] */
			inTempLen = strlen(szTemplate1);
			/* 長度大於7表示有負號 */
			if( inTempLen < 8 )
			{
				inAmtXAddr = _PRINT_DEFINE_X_04_;
				/*只要資料滿七個BYTES 就必需凸排  */
				if( inTempLen == 7 )
				{	
					inHeaderAmtXAddr = _PRINT_DEFINE_X_04_ - 10;
				}else{
					inHeaderAmtXAddr = _PRINT_DEFINE_X_04_ - 4;
				}
				
				inOtherAmtXAddr = _PRINT_DEFINE_X_04_ - 4;

			}else{
				inAmtXAddr = _PRINT_DEFINE_X_04_ - 10;
//				inRedeemAmtXAddr = _PRINT_DEFINE_X_04_ - 20;
				inHeaderAmtXAddr = _PRINT_DEFINE_X_04_ - 20;
				inOtherAmtXAddr = _PRINT_DEFINE_X_04_ - 4 ;
			}

			inFunc_Amount_Comma(szTemplate1, "NT$" , 0x00, _SIGNED_NONE_, 16, _PAD_LEFT_FILL_RIGHT_);
			memcpy(chAmt,szTemplate1,strlen(szTemplate1));
			strcat(szPrintBuf, szTemplate1);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, inAmtXAddr);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 卡號(卡別) */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, pobTran->srBRec.szPAN);
			/* 預防長度過長  20190903 [SAM] */
			/* 因為銀聯卡16碼還是會過長，所以獨立出來 */
			if(memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_)) == 0)
			{
				inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 25, _PAD_LEFT_FILL_RIGHT_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* Time */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "TIME: %.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_04_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcat(szPrintBuf, "(CUP)");
				inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 6, _PAD_LEFT_FILL_RIGHT_);

				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				
			}else	if(strlen(pobTran->srBRec.szPAN) < 19 )
			{
				if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)) == 0)
					strcat(szPrintBuf, "(V)");
				else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
					strcat(szPrintBuf, "(M)");
				else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)) == 0)
					strcat(szPrintBuf, "(J)");
				else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
					strcat(szPrintBuf, "(U)");
				else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
					strcat(szPrintBuf, "(A)");
				else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0)
					strcat(szPrintBuf, "(D)");

				inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 25, _PAD_LEFT_FILL_RIGHT_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);


				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* Time */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "TIME: %.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_04_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}else{
				
					
				inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 25, _PAD_LEFT_FILL_RIGHT_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* Time */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "TIME: %.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_04_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)) == 0)
					strcat(szPrintBuf, "(V)");
				else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
					strcat(szPrintBuf, "(M)");
				else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)) == 0)
					strcat(szPrintBuf, "(J)");
				else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
					strcat(szPrintBuf, "(U)");
				else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
					strcat(szPrintBuf, "(A)");
				else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0)
					strcat(szPrintBuf, "(D)");

				inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 6, _PAD_LEFT_FILL_RIGHT_);

				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
				

			/* Date */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "DATE: %.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Invoice Number */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			if (pobTran->srBRec.uszVOIDBit != VS_TRUE)
			{
				sprintf(szTemplate1, "INV:%06ld", pobTran->srBRec.lnOrgInvNum);
			}
			else
			{
				sprintf(szTemplate1, "*INV:%06ld", pobTran->srBRec.lnOrgInvNum);
			}

			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szTemplate1, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_04_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
			

			/* 【需求單 - 105259】總額明細報表及總額明細查詢補強機制 商店聯出紙本要印<P> */
			
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetESCMode(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0 && pobTran->srBRec.inESCUploadStatus == _ESC_UPLOAD_STATUS_PAPER_)
			{
				/* Approved No. */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, "APPR: ");
				strcat(szPrintBuf, pobTran->srBRec.szAuthCode);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				
				memset(szTemplate1,0x00,sizeof(szTemplate1));
				strcat(szTemplate1, "紙本簽單");
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szTemplate1, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_04_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}else{
				/* Approved No. */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, "APPR: ");
				strcat(szPrintBuf, pobTran->srBRec.szAuthCode);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}

			/* RRN. */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szRefNo, ' ', 12, _PAD_LEFT_FILL_RIGHT_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Store ID */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetStoreIDEnable(szFuncEnable);
			if (szFuncEnable[0] == 'Y')
			{
				/*開啟櫃號功能*/
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "STORE ID:%s", pobTran->srBRec.szStoreID);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}

			if(pobTran->srBRec.uszInstallmentBit)
			{
				/* 分期交易金額 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "分期交易金額");
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* 分期交易金額 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, ":%s",chAmt);
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, inHeaderAmtXAddr);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* 分期交易期數 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "分期交易期數");
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				
				/* 分期交易期數 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, ":%02ld",pobTran->srBRec.lnInstallmentPeriod);
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, inOtherAmtXAddr);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				
				/* 分期首期金額 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));				
				sprintf(szPrintBuf, "分期首期金額");
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _INST_REFUND_) ||
					(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _INST_SALE_ || pobTran->srBRec.inOrgCode == _INST_ADJUST_)))
					sprintf(szPrintBuf, ":NT$%ld",(0 - pobTran->srBRec.lnInstallmentDownPayment));
				else
					sprintf(szPrintBuf, ":NT$%ld",pobTran->srBRec.lnInstallmentDownPayment);

				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, inOtherAmtXAddr);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* 分期每期金額 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "分期每期金額");
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _INST_REFUND_) ||
					(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _INST_SALE_ || pobTran->srBRec.inOrgCode == _INST_ADJUST_)))
					sprintf(szPrintBuf, ":NT$%ld",(0 - pobTran->srBRec.lnInstallmentPayment));
				else
					sprintf(szPrintBuf, ":NT$%ld",pobTran->srBRec.lnInstallmentPayment);
				
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, inOtherAmtXAddr);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* 分期手續費 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "分期手續費");
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				
				if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _INST_REFUND_) ||
					(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _INST_SALE_ || pobTran->srBRec.inOrgCode == _INST_ADJUST_)))
					sprintf(szPrintBuf, ":NT$%ld",(0 - pobTran->srBRec.lnInstallmentFormalityFee));
				else
					sprintf(szPrintBuf, ":NT$%ld",pobTran->srBRec.lnInstallmentFormalityFee);
				
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, inOtherAmtXAddr);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}

			if(pobTran->srBRec.uszRedeemBit)
			{
				/* 紅利交易金額 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利交易金額");
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* 紅利交易金額 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, ":%s",chAmt);
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, inHeaderAmtXAddr);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);


				/* 紅利折抵金額 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利折抵金額");
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _REDEEM_REFUND_) ||
					(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _REDEEM_SALE_ || pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_)))
					sprintf(szPrintBuf, ":NT$%ld",(0 - pobTran->srBRec.lnRedemptionPaidAmount));
				else
					sprintf(szPrintBuf, ":NT$%ld",pobTran->srBRec.lnRedemptionPaidAmount);
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, inOtherAmtXAddr);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* 紅利本次折抵點數 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利折抵點數");
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _REDEEM_REFUND_) ||
					(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _REDEEM_SALE_ || pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_)))
					sprintf(szPrintBuf, ":%ld",(0 - pobTran->srBRec.lnRedemptionPoints));
				else
					sprintf(szPrintBuf, ":%ld",pobTran->srBRec.lnRedemptionPoints);
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, inOtherAmtXAddr);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* 紅利剩餘點數 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利剩餘點數");
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, ":%ld",pobTran->srBRec.lnRedemptionPointsBalance);
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, inOtherAmtXAddr);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* 實付金額 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "實付金額");
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				if ((pobTran->srBRec.uszVOIDBit == VS_FALSE && pobTran->srBRec.inCode == _REDEEM_REFUND_) ||
					(pobTran->srBRec.uszVOIDBit == VS_TRUE && (pobTran->srBRec.inOrgCode == _REDEEM_SALE_ || pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_)))
					sprintf(szPrintBuf, ":NT$%ld",(0 - pobTran->srBRec.lnRedemptionPaidCreditAmount));
				else
					sprintf(szPrintBuf, ":NT$%ld",pobTran->srBRec.lnRedemptionPaidCreditAmount);
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inSelectFoneSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, inOtherAmtXAddr);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
		}/* 商店聯卡號遮掩 END */

		inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	} /* End for () .... */

	/* 結束讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCREDIT_PRINT_DetailReportBottom()_END");
	}

	return (inRetVal);
}


/*
Function        :inCREDIT_PRINT_Func7PrintParamTerm_ByBuffer
Date&Time       :2016/2/17 上午 10:47
Describe        :功能7列印參數，雖然這裡用兩個buffer印，但體感上和用一個buffer差不多，仍會等到印完才開始Put in
*/
int inCREDIT_PRINT_Func7PrintParamTerm_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;

	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{

		inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);

		/* ParamLOGO_ByBuffer */
		if (inCREDIT_PRINT_ParamLOGO_ByBuffer(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1) != VS_SUCCESS)
			return (VS_ERROR);

		/* ParamTermInformation_ByBuffer */
		if (inCREDIT_PRINT_ParamTermInformation_ByBuffer(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1) != VS_SUCCESS)
			return (VS_ERROR);

		/* ParamHostDetailParam_ByBuffer */
		if (inCREDIT_PRINT_ParamHostDetailParam_ByBuffer(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1) != VS_SUCCESS)
			return (VS_ERROR);

		/* 列印【卡別參數檔】【特殊卡別參數檔】【非接觸式】  */
		if (inCREDIT_PRINT_ParamCardType_ByBuffer(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1) != VS_SUCCESS)
			return (VS_ERROR);

		/* 列印【管理號碼】 */
		if (inCREDIT_PRINT_ParamManageNum_ByBuffer(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1) != VS_SUCCESS)
			return (VS_ERROR);

		/* 列印【產品代碼】 */
		if (inCREDIT_PRINT_ProductCode_ByBuffer(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1) != VS_SUCCESS)
			return (VS_ERROR);

		/* 列印【EMV CA Public Key】 */
		if (inCREDIT_PRINT_CAPublicKey_ByBuffer(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1) != VS_SUCCESS)
			return (VS_ERROR);

		/* 列印【系統參數檔】【共用參數檔】  */
		if (inCREDIT_PRINT_ParamSystemConfig_ByBuffer(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1) != VS_SUCCESS)
			return (VS_ERROR);

		if (inEDC_PRINT_ParamQRCodeConfig_ByBuffer(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1) != VS_SUCCESS)
			return (VS_ERROR);
		
		/* ParamLOGO_END_ByBuffer */
		if (inCREDIT_PRINT_ParamLOGO_END_ByBuffer(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1) != VS_SUCCESS)
			return (VS_ERROR);

		
		if (inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1) != VS_SUCCESS)
			return (VS_ERROR);
		

		return (VS_SUCCESS);
	}
}

int inCREDIT_PRINT_ParamLOGO_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int			inRetVal;
        char			szFlag[10 + 1];
        char			szTxnType[20 + 1];

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);/* 帳單列印中 */

        /* 列印日期 / 時間 */
	inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
	inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
        inRetVal =inCREDIT_PRINT_Printing_Time(pobTran, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle);
	if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);

        /* PRINT NCCC LOGO */
	inRetVal = inPRINT_Buffer_PutIn("BANK LOGO ：", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);

	inPRINT_Buffer_PutGraphic((unsigned char *)_BANK_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inBankLogoHeight, _APPEND_);

        /* PRINT MERCHANT LOGO */
        memset(szFlag, 0x00, sizeof(szFlag));
        inGetPrtMerchantLogo(szFlag);

        if (szFlag[0] == 'Y')
        {
		inRetVal = inPRINT_Buffer_PutIn("LOGO ：", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inPRINT_Buffer_PutGraphic((unsigned char *)_MERCHANT_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inMerchantLogoHeight, _APPEND_);
        }
        else
        {
		inRetVal = inPRINT_Buffer_PutIn("LOGO ： 無", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }

        /* PRINT MERCHANT NAME */
        memset(szFlag, 0x00, sizeof(szFlag));
        inGetPrtMerchantName(szFlag);
#if 0 /* 因為目前公司TMS不支援圖檔表頭下載，只會下載文字，所以要改 20190211 [SAM]  */
        if (szFlag[0] == 'Y')
        {
		inRetVal = inPRINT_Buffer_PutIn("表頭 ：", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inPRINT_Buffer_PutGraphic((unsigned char *)_NAME_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inTitleNameHeight, _APPEND_);
		
        }
        else
        {
		inRetVal = inPRINT_Buffer_PutIn("表頭 ： 無", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }
#else 
	 if (szFlag[0] == 'Y')
        {
		inRetVal = inPRINT_Buffer_PutIn("表頭 ：", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
                
                inCREDIT_PRINT_MerchantNameTXT(pobTran, uszBuffer, srFont_Attrib, srBhandle);
        }
        else
        {
		inRetVal = inPRINT_Buffer_PutIn("表頭 ： 無", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }	
#endif		
		
        /* PRINT SLOGAN */
        memset(szFlag, 0x00, sizeof(szFlag));
        inGetPrtSlogan(szFlag);

        if (szFlag[0] == 'Y')
        {
		inRetVal = inPRINT_Buffer_PutIn("SLOGAN ：", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inPRINT_Buffer_PutGraphic((unsigned char *)_SLOGAN_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inSloganHeight, _APPEND_);
        }
        else
        {
		inRetVal = inPRINT_Buffer_PutIn("SLOGAN ： 無", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }

        /* PRINT 分期付款條文 */
        memset(szFlag, 0x00, sizeof(szFlag));
        memset(szTxnType, 0x00, sizeof(szTxnType));

	if (inLoadHDTRec(0) == VS_ERROR)	
	{
		inDISP_DispLogAndWriteFlie(" CREDIT Print Par Logo Load HDT[%d] *Error* Line[%d]", 0, __LINE__);
	}
	
        inGetTransFunc(szTxnType);

        //if (szTxnType[7] == 0x59)
        if (szTxnType[8] == 0x31) /* 修改TMS交易開關位置 20190211 [SAM] */
        {
		inRetVal = inPRINT_Buffer_PutIn("分期付款條文 ：", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inPRINT_Buffer_PutGraphic((unsigned char *)_LEGAL_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inInstHeight, _APPEND_);
        }
        else
        {
		inRetVal = inPRINT_Buffer_PutIn("分期付款條文 ： 無", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }

        /* PRINT 商店需求提示語 */
        memset(szFlag, 0x00, sizeof(szFlag));
        inGetPrtNotice(szFlag);

        if (szFlag[0] == 'Y')
        {
		inRetVal = inPRINT_Buffer_PutIn("商店需求提示語 ：", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inPRINT_Buffer_PutGraphic((unsigned char *)_NOTICE_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inNoticeHeight, _APPEND_);
        }
        else
        {
		inRetVal = inPRINT_Buffer_PutIn("商店需求提示語 ： 無", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }

        /* CUP 交易警語 */
        memset(szFlag, 0x00, sizeof(szFlag));
	inGetCUPFuncEnable(szFlag);

        if (memcmp(szFlag, "Y", strlen("Y")) == 0)
        {
		inRetVal = inPRINT_Buffer_PutIn("CUP交易警語 ：", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inPRINT_Buffer_PutGraphic((unsigned char *)_CUP_LEGAL_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inCupLegalHeight, _APPEND_);
        }
        else
        {
		inRetVal = inPRINT_Buffer_PutIn("CUP交易警語 ： 無", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

int inCREDIT_PRINT_ParamTermInformation_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char			szPrintBuffer[100 + 1];
	char			szTemplate[64 + 1];

	inLoadTMSCPTRec(0);
	inLoadTMSFTPRec(0);

	/* Terminal AP Name */
	inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
	if (strlen(gszTermVersionID) > 0)
	{
		memcpy(szTemplate, gszTermVersionID, strlen(gszTermVersionID));
	}
	else
	{
		inGetTermVersionID(szTemplate);
	}
	sprintf(szPrintBuffer, "VERSION ID = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* Terminal AP Version */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
	if (strlen(gszTermVersionDate) > 0)
	{
		memcpy(szTemplate, gszTermVersionDate, strlen(gszTermVersionDate));
	}
	else
	{
		inGetTermVersionDate(szTemplate);
	}
	sprintf(szPrintBuffer, "VERSION DATE = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 端末機通訊設定 */
        inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        inPRINT_Buffer_PutIn("端末機通訊設定", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* EDC IP */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetTermIPAddress(szTemplate);
	sprintf(szPrintBuffer, "EDC IP = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* SUBNET MASK */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetTermMASKAddress(szTemplate);
	sprintf(szPrintBuffer, "SUBNET MASK = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* DF GATEWAY */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetTermGetewayAddress(szTemplate);
	sprintf(szPrintBuffer, "DF GATEWAY = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* TMS IP */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetTMSIPAddress(szTemplate);
	sprintf(szPrintBuffer, "TMS IP = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* PORT NO */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetTMSPortNum(szTemplate);
	sprintf(szPrintBuffer, "Port No = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* TMS TEL */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetTMSPhoneNumber(szTemplate);
	sprintf(szPrintBuffer,"TMS TEL = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* FTP IP */
	memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetFTPIPAddress(szTemplate);
	sprintf(szPrintBuffer, "FTP IP = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* FTP PORT NO */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetFTPPortNum(szTemplate);
	sprintf(szPrintBuffer, "Port No = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        /* TSP IP，客製化參數為Costco時列印，20230111 Miyano */
        if (vbCheckCostcoCustom(Costco_New))
        {
            memset(szTemplate, 0x00, sizeof(szTemplate));
            memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
            inGetTSP_IP(szTemplate);
            inDISP_LogPrintf("szTemplate TSP_IP = %s", szTemplate);
            sprintf(szPrintBuffer, "TSP IP = %s", szTemplate);
            inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
            /* PORT NO */
            memset(szTemplate, 0x00, sizeof(szTemplate));
            memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
            inGetTSP_Port(szTemplate);
            inDISP_LogPrintf("szTemplate TSP_Port = %s", szTemplate);
            sprintf(szPrintBuffer, "Port No = %s", szTemplate);
            inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }

        inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


        return (VS_SUCCESS);
}

int inCREDIT_PRINT_ParamHostDetailParam_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int		i = 0, j = 0;
	char		szCommunicationIndex[2 + 1] = {0};
	char		szPrintBuffer[100 + 1] = {0};
	char		szTxnType[20 + 1] = {0};
	char		szTemplate[64 + 1] = {0};
	char		szHostEnable[2 + 1] = {0};
	unsigned char ucTrunsFuncCondition = 0x31; /* 新增一個條件，用來判斷功能 20190211 [SAM] */
	
	for (i = 0 ;; i ++)
	{
		if (inLoadHDTRec(i) < 0) /* 主機參數檔 */
		{
			inDISP_LogPrintfWithFlag(" CREDIT Print Host Detial Parm Load HDT[%d] *Error* Line[%d]", i, __LINE__);
			break;
		}
			
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetHostEnable(szTemplate);

		inDISP_LogPrintfWithFlag(" TMS Print Host Enable[%s]",szTemplate);
				
                /* 主機功能開關關閉不列印 */
                if (!memcmp(szTemplate, "N", 1))
                        continue;
		
		/* 因為公司TMS會下CUP主機，因富邦是同批次，所以CUP要跳過 所以新增 20190214 [SAM] */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetHostLabel(szTemplate);
		if (!memcmp(szTemplate, _HOST_NAME_CUP_, strlen(_HOST_NAME_CUP_)))
		{
			continue;
		}	
		
		inGetCommunicationIndex(szCommunicationIndex);

		if (inLoadCPTRec(atoi(szCommunicationIndex) - 1) < 0)
		{
			inDISP_LogPrintfWithFlag(" TMS CPT Err Comm Id [%s]",szCommunicationIndex);
			break;
		}
		
		/* 主機資料 */
                memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                inGetHostLabel(szPrintBuffer);
                inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /* Merchant ID */
	 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetMerchantID(szTemplate);
	 	sprintf(szPrintBuffer, "MID = %s", szTemplate);
	 	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /* Terminal ID */
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetTerminalID(szTemplate);
	 	sprintf(szPrintBuffer, "TID = %s", szTemplate);
	 	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /* 第一授權撥接電話 */
                memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetHostTelPrimary(szTemplate);
	 	sprintf(szPrintBuffer, "TEL #1 = %s", szTemplate);
	 	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /* 第二授權撥接電話 */
	 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetHostTelSecond(szTemplate);
	 	sprintf(szPrintBuffer, "TEL #2 = %s", szTemplate);
	 	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /* Call Bank 撥接電話 */
	 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetReferralTel(szTemplate);
	 	sprintf(szPrintBuffer, "TEL #3 = %s", szTemplate);
	 	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /* 第一授權主機 IP Address  */
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetHostIPPrimary(szTemplate);
	 	sprintf(szPrintBuffer, "HOST IP = %s", szTemplate);
	 	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /* 第一授權主機 Port No. */
	 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetHostPortNoPrimary(szTemplate);
	 	sprintf(szPrintBuffer, "PORT NO. = %s", szTemplate);
	 	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 第二授權主機 IP Address  */
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetHostIPSecond(szTemplate);
	 	sprintf(szPrintBuffer, "HOST IP2 = %s", szTemplate);
	 	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /* 第二授權主機 Port No. */
	 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetHostPortNoSecond(szTemplate);
	 	sprintf(szPrintBuffer, "PORT NO. = %s", szTemplate);
	 	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /* TPDU */
	 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTPDU(szTemplate);
	 	sprintf(szPrintBuffer, "TPDU = %s", szTemplate);
	 	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /* NII */
	 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetNII(szTemplate);
	 	sprintf(szPrintBuffer, "NII = %s", szTemplate);
	 	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	 	/* 交易開關 */
		memset(szTxnType, 0x00, sizeof(szTxnType));
		inGetTransFunc(szTxnType);
				inPRINT_Buffer_PutIn("　　　　　　　　　　　開　　關", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetHostLabel(szTemplate);

		if (!memcmp(szTemplate, "HG      ", 8))
		{
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			if (szTxnType[0] == 0x59)
				inPRINT_Buffer_PutIn("紅利積點　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("紅利積點　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[1] == 0x59)
				inPRINT_Buffer_PutIn("點數扣抵　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("點數扣抵　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[2] == 0x59)
				inPRINT_Buffer_PutIn("加價購　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("加價購　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[3] == 0x59)
				inPRINT_Buffer_PutIn("點數兌換　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("點數兌換　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[4] == 0x59)
				inPRINT_Buffer_PutIn("點數查詢　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("點數查詢　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[5] == 0x59)
				inPRINT_Buffer_PutIn("回饋退貨　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("回饋退貨　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[6] == 0x59)
				inPRINT_Buffer_PutIn("扣抵退貨　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("扣抵退貨　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[7] == 0x59)
				inPRINT_Buffer_PutIn("紅利積點人工輸入卡號　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("紅利積點人工輸入卡號　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[8] == 0x59)
				inPRINT_Buffer_PutIn("紅利積點列印簽單　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("紅利積點列印簽單　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else if (!memcmp(szTemplate, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)))
		{
			for (j = 0 ;; j ++)
                	{
                		if (inLoadTDTRec(j) < 0)
				{
                			break;
				}

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetTicket_HostName(szTemplate);
				memset(szHostEnable, 0x00, sizeof(szHostEnable));
				inGetTicket_HostEnable(szHostEnable);
                		memset(szTxnType, 0x00, sizeof(szTxnType));
				inGetTicket_HostTransFunc(szTxnType);

		                if (!memcmp(szTemplate, _HOST_NAME_IPASS_, strlen(_HOST_NAME_IPASS_)) && !memcmp(szHostEnable, "Y", strlen(szHostEnable)))
                		{
					inPRINT_Buffer_PutIn("一卡通", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[0] == 0x59)
						inPRINT_Buffer_PutIn("購貨　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("購貨　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[1] == 0x59)
						inPRINT_Buffer_PutIn("購貨取消　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("購貨取消　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[2] == 0x59)
						inPRINT_Buffer_PutIn("退貨　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("退貨　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[3] == 0x59)
                				inPRINT_Buffer_PutIn("自動加值　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("自動加值　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[4] == 0x59)
						inPRINT_Buffer_PutIn("現金加值　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("現金加值　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[5] == 0x59)
						inPRINT_Buffer_PutIn("現金加值取消　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("現金加值取消　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[7] == 0x59)
						inPRINT_Buffer_PutIn("餘額查詢　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("餘額查詢　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[8] == 0x59)
						inPRINT_Buffer_PutIn("結帳　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("結帳　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			        }
                		else if (!memcmp(szTemplate, _HOST_NAME_ECC_, strlen(_HOST_NAME_ECC_)) && !memcmp(szHostEnable, "Y", strlen(szHostEnable)))
                		{
					inPRINT_Buffer_PutIn("悠遊卡", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[0] == 0x59)
						inPRINT_Buffer_PutIn("購貨　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("購貨　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[1] == 0x59)
						inPRINT_Buffer_PutIn("購貨取消　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("購貨取消　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[2] == 0x59)
						inPRINT_Buffer_PutIn("退貨　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("退貨　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[3] == 0x59)
                				inPRINT_Buffer_PutIn("自動加值　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("自動加值　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[4] == 0x59)
						inPRINT_Buffer_PutIn("現金加值　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("現金加值　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[5] == 0x59)
						inPRINT_Buffer_PutIn("現金加值取消　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("現金加值取消　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[7] == 0x59)
						inPRINT_Buffer_PutIn("餘額查詢　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("餘額查詢　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[8] == 0x59)
						inPRINT_Buffer_PutIn("結帳　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("結帳　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                		}
                		else if (!memcmp(szTemplate, _HOST_NAME_ICASH_, strlen(_HOST_NAME_ICASH_)) && !memcmp(szHostEnable, "Y", strlen(szHostEnable)))
                		{
                		        inPRINT_Buffer_PutIn("ICash尚未完成", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                		}
                		else if (!memcmp(szTemplate, _HOST_NAME_HAPPYCASH_, strlen(_HOST_NAME_HAPPYCASH_)) && !memcmp(szHostEnable, "Y", strlen(szHostEnable)))
                		{
                		        inPRINT_Buffer_PutIn("HappyCash尚未完成", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                		}
			}
		}
#ifdef __NCCC_DCC_FUNC__				
		/* DCC 不印補登 */
		else if (!memcmp(szTemplate, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_)))
		{
			if (szTxnType[0] == 0x59)
				inPRINT_Buffer_PutIn("銷售　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("銷售　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[1] == 0x59)
				inPRINT_Buffer_PutIn("取消　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("取消　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[2] == 0x59)
				inPRINT_Buffer_PutIn("結帳　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("結帳　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[3] == 0x59)
				inPRINT_Buffer_PutIn("退貨　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("退貨　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[5] == 0x59)
				inPRINT_Buffer_PutIn("預先授權　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("預先授權　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[6] == 0x59)
				inPRINT_Buffer_PutIn("小費　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("小費　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (!memcmp(szTemplate, "NCCC    ", 8))
			{
				if (szTxnType[7] == 0x59)
					inPRINT_Buffer_PutIn("分期付款　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("分期付款　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (szTxnType[8] == 0x59)
					inPRINT_Buffer_PutIn("紅利扣抵　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("紅利扣抵　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (szTxnType[9] == 0x59)
					inPRINT_Buffer_PutIn("分期調帳　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("分期調帳　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (szTxnType[10] == 0x59)
					inPRINT_Buffer_PutIn("紅利調帳　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("紅利調帳　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (szTxnType[11] == 0x59)
					inPRINT_Buffer_PutIn("郵購(MO/TO) 　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("郵購(MO/TO) 　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
#endif
		else
		{
			if (szTxnType[0] == ucTrunsFuncCondition)
				inPRINT_Buffer_PutIn("銷售　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("銷售　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[1] == ucTrunsFuncCondition)
				inPRINT_Buffer_PutIn("取消　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("取消　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[2] == ucTrunsFuncCondition)
				inPRINT_Buffer_PutIn("結帳　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("結帳　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[3] == ucTrunsFuncCondition)
				inPRINT_Buffer_PutIn("退貨　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("退貨　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (!memcmp(szTemplate, "NCCC    ", 8))
			{
				if (szTxnType[4] != 0x30)
					inPRINT_Buffer_PutIn("補登　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("補登　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				if (szTxnType[4] == ucTrunsFuncCondition)
					inPRINT_Buffer_PutIn("補登　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("補登　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}

			if (szTxnType[5] == ucTrunsFuncCondition)
				inPRINT_Buffer_PutIn("預先授權　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("預先授權　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[6] == ucTrunsFuncCondition)
				inPRINT_Buffer_PutIn("小費　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("小費　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			
			if (szTxnType[7] == ucTrunsFuncCondition)
				inPRINT_Buffer_PutIn("調帳　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("調帳　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			//if (!memcmp(szTemplate, "NCCC    ", 8))
			{
				if (szTxnType[8] == ucTrunsFuncCondition)
					inPRINT_Buffer_PutIn("分期付款　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("分期付款　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (szTxnType[9] == ucTrunsFuncCondition)
					inPRINT_Buffer_PutIn("紅利扣抵　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("紅利扣抵　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (szTxnType[10] == ucTrunsFuncCondition)
					inPRINT_Buffer_PutIn("分期調帳　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("分期調帳　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (szTxnType[11] == ucTrunsFuncCondition)
					inPRINT_Buffer_PutIn("紅利調帳　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("紅利調帳　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

//				if (szTxnType[11] == ucTrunsFuncCondition)
//					inPRINT_Buffer_PutIn("郵購(MO/TO) 　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
//				else
//					inPRINT_Buffer_PutIn("郵購(MO/TO) 　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}


		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetManualKeyin(szTemplate);

		if (!memcmp(&szTemplate[0], "Y", 1))
			inPRINT_Buffer_PutIn("人工輸入卡號　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("人工輸入卡號　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTipPercent(szTemplate);
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		sprintf(szPrintBuffer, "小費限額 = %s", szTemplate);
		strcat(szPrintBuffer, "%");
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                inPRINT_Buffer_PutIn("", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	inPRINT_Buffer_PutIn("", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	return (VS_SUCCESS);
}

int inCREDIT_PRINT_ParamCardType_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i, inSpace = 20;
	char	szPrintBuffer[100 + 1];
        char	szTemplate[64 + 1];
        char	szLowBinRange[11 + 1], szHighBinRange[11 + 1], szMinPANLength[2 + 1], szMaxPANLength[2 + 1];

	inPRINT_Buffer_PutIn("卡別參數", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	for (i = 0 ;; i ++)
	{
		/* 卡別參數檔 */
		if (inLoadCDTRec(i) < 0)
			break;

                /* 卡別 CVV(4DBC) */
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetCardLabel(szTemplate);
		sprintf(szPrintBuffer, "%02d.%s", (i + 1), szTemplate);
		inFunc_PAD_ASCII(szPrintBuffer, szPrintBuffer, ' ', inSpace, _PAD_LEFT_FILL_RIGHT_);
		strcat(szPrintBuffer, "CVV(4DBC) = ");
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGet4DBCEnable(szTemplate);

		if (!memcmp(&szTemplate[0], "Y", 1))
			strcat(szPrintBuffer, "ON");
		else
			strcat(szPrintBuffer, "OFF");

		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /* 卡號範圍 卡號長度 */
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szLowBinRange, 0x00, sizeof(szLowBinRange));
                memset(szHighBinRange, 0x00, sizeof(szHighBinRange));
                memset(szMinPANLength, 0x00, sizeof(szMinPANLength));
                memset(szMaxPANLength, 0x00, sizeof(szMaxPANLength));
                inGetLowBinRange(szLowBinRange);
                inGetHighBinRange(szHighBinRange);
                inGetMinPANLength(szMinPANLength);
                inGetMaxPANLength(szMaxPANLength);
		sprintf(szPrintBuffer, "   %s ~ %s   %s ~ %s", szLowBinRange,
		                                               szHighBinRange,
		                                               szMinPANLength,
		                                               szMaxPANLength);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	inPRINT_Buffer_PutIn("非接觸卡別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("　　　　　　　　　　　開　　關", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetVISAPaywaveEnable(szTemplate);

	if (!memcmp(&szTemplate[0], "Y", 1))
		inPRINT_Buffer_PutIn("Visa Paywave　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("Visa Paywave　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetJCBJspeedyEnable(szTemplate);

        if (!memcmp(&szTemplate[0], "Y", 1))
		inPRINT_Buffer_PutIn("JCB Jspeedy 　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("JCB Jspeedy 　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetMCPaypassEnable(szTemplate);

	if (!memcmp(&szTemplate[0], "Y", 1))
		inPRINT_Buffer_PutIn("M/C Paypass 　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("M/C Paypass 　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetSmartPayContactlessEnable(szTemplate);

	if (!memcmp(&szTemplate[0], "Y", 1))
		inPRINT_Buffer_PutIn("FISC SmartPay 　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("FISC SmartPay 　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetCUPContactlessEnable(szTemplate);

	if (!memcmp(&szTemplate[0], "Y", 1))
		inPRINT_Buffer_PutIn("CUP QuickPass 　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("CUP QuickPass 　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetAMEXContactlessEnable(szTemplate);

	if (!memcmp(&szTemplate[0], "Y", 1))
		inPRINT_Buffer_PutIn("AE ExpressPay 　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("AE ExpressPay 　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetDFS_Contactless_Enable(szTemplate);

	if (!memcmp(&szTemplate[0], "Y", 1))
		inPRINT_Buffer_PutIn("DFS           　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("DFS           　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetSpecialCardRangeEnable(szTemplate);

	if (!memcmp(&szTemplate[0], "N", 1))
		inPRINT_Buffer_PutIn("特殊卡別參數 : 無", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
	{
		inPRINT_Buffer_PutIn("特殊卡別參數", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		for (i = 0 ;; i ++)
		{
			if (inLoadSCDTRec(i) < 0)
				break;

                        /* 活動起始日及結束日 */
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			sprintf(szPrintBuffer, "%02d.", (i + 1));
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetCampaignStartDate(szTemplate);
			strcat(szPrintBuffer, szTemplate);
			strcat(szPrintBuffer, " ~ ");
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetCampaignEndDate(szTemplate);
			strcat(szPrintBuffer, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                        /* 卡號範圍 */
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetSCDTLowBinRange(szTemplate);
                        sprintf(szPrintBuffer, "   %s ~ ", szTemplate);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetSCDTHighBinRange(szTemplate);
                        strcat(szPrintBuffer, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                        /* 活動代碼 活動限額 */
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetCampaignNumber(szTemplate);
                        sprintf(szPrintBuffer, "   Campaign = %s  ", szTemplate);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetCampaignAmount(szTemplate);
                        strcat(szPrintBuffer, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}

	inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	return (VS_SUCCESS);
}

int inCREDIT_PRINT_ParamManageNum_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char			szCheckEnable[2 + 1];

        inLoadPWDRec(0);
	inPRINT_Buffer_PutIn("管理號碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("　　　　　　　　　　　　開　　關", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetRebootPwdEnale(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("開機管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("開機管理號碼　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetSalePwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("銷售管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("銷售管理號碼　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetPreauthPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("預先授權管理號碼　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("預先授權管理號碼　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetInstallmentPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("分期管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("分期管理號碼　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetRedeemPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("紅利管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("紅利管理號碼　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetMailOrderPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("郵購管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("郵購管理號碼　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetOfflinePwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("補登管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("補登管理號碼　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetInstallmentAdjustPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("分期調帳管理號碼　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("分期調帳管理號碼　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetRedeemAdjustPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("紅利調帳管理號碼　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("紅利調帳管理號碼　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetVoidPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("取消管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("取消管理號碼　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetSettlementPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("結帳管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("結帳管理號碼　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetRefundPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("退貨管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("退貨管理號碼　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetHGRefundPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("ＨＧ退貨管理號碼　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("ＨＧ退貨管理號碼　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	inPRINT_Buffer_PutIn("", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	return (VS_SUCCESS);
}

int inCREDIT_PRINT_ProductCode_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int			i;
	char			szPrintBuf[100 + 1];
        char			szTemplate[42 + 1];

	if (inLoadCFGTRec(0) < 0)
	{
		return (VS_ERROR);
	}

        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetProductCodeEnable(szTemplate);

	if (!memcmp(&szTemplate[0], "N", 1))
                inPRINT_Buffer_PutIn("產品代碼 : 無 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
	{
                inPRINT_Buffer_PutIn("產品代碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		for (i = 0 ;; i ++)
		{
			if (inLoadPCDRec(i) < 0) /* 產品代碼【PCodeDef.txt】 */
				break;

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetProductCodeIndex(szTemplate);
                        sprintf(szPrintBuf, "%s. ", szTemplate);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetKeyMap(szTemplate);
                        strcat(szPrintBuf, szTemplate);
                        strcat(szPrintBuf, " ");
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetProductScript(szTemplate);
                        strcat(szPrintBuf, szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}

	inPRINT_Buffer_PutIn("", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	return (VS_SUCCESS);
}

int inCREDIT_PRINT_CAPublicKey_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int			i;
	char			szPrintBuffer[100 + 1];
        char			szEMVCAPKIndex[2 + 1], szApplicationId[10 + 1], szCAPKIndex[2 + 1];

	inPRINT_Buffer_PutIn("EMV CAPK", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	for (i = 0 ;; i ++)
	{
                if (inLoadESTRec(i) < 0) /* 共用參數檔 */
                        break;

                memset(szEMVCAPKIndex, 0x00, sizeof(szEMVCAPKIndex));
                memset(szApplicationId, 0x00, sizeof(szApplicationId));
                memset(szCAPKIndex, 0x00, sizeof(szCAPKIndex));
                inGetEMVCAPKIndex(szEMVCAPKIndex);
                inGetCAPKApplicationId(szApplicationId);
                inGetCAPKIndex(szCAPKIndex);
                memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                sprintf(szPrintBuffer, "%s. %s.%s", szEMVCAPKIndex, szApplicationId, szCAPKIndex);
                inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

        inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	return (VS_SUCCESS);
}

int inCREDIT_PRINT_ParamSystemConfig_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char	szPrintBuffer[100 + 1];
	char	szTemplate[42 + 1];
	char	szTemplate2[42 + 1];

	while (1)
	{
		inLoadTMSSCTRec(0);
		inPRINT_Buffer_PutIn("系統參數", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSInquireMode(szTemplate);
		sprintf(szPrintBuffer, "TMS詢問模式 = %s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (!memcmp(&szTemplate[0], _TMS_INQUIRE_02_SCHEDHULE_SETTLE_, 1))
		{
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetTMSInquireStartDate(szTemplate);
			sprintf(szPrintBuffer, "TMS詢問起始日期 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetTMSInquireTime(szTemplate);
			sprintf(szPrintBuffer, "TMS詢問起始時間 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			memset(szTemplate, 0x00, sizeof(szTemplate));
//			inGetTMSInquireGap(szTemplate);
//			sprintf(szPrintBuffer, "TMS詢問間隔天數 = %s", szTemplate);
//			inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		break;
	}

	while (1)
	{
		/* 共用參數檔 */
		if (inLoadCFGTRec(0) < 0)
		{
			/* 共用參數檔不存在 */
			inDISP_Msg_BMP(_ERR_NO_FILE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
			return (VS_ERROR);
		}

		inPRINT_Buffer_PutIn("共用參數", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetCustomIndicator(szTemplate);
		sprintf(szPrintBuffer, "1.客製化專屬參數 = %s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetCommMode(szTemplate);

		if (memcmp(szTemplate, _COMM_MODEM_MODE_, 1) == 0)
			inPRINT_Buffer_PutIn("3.通訊模式 = DialUp", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (memcmp(szTemplate, _COMM_ETHERNET_MODE_, 1) == 0)
			inPRINT_Buffer_PutIn("3.通訊模式 = TCP/IP", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (memcmp(szTemplate, _COMM_GPRS_MODE_, 1) == 0)
			inPRINT_Buffer_PutIn("3.通訊模式 = GPRS", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetDialBackupEnable(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("4.撥接備援 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("4.撥接備援 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetEncryptMode(szTemplate);

		if (memcmp(szTemplate, _NCCC_ENCRYPTION_NONE_, 1) == 0)
			inPRINT_Buffer_PutIn("5.加密模式 = 不加密", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (memcmp(szTemplate, _NCCC_ENCRYPTION_TSAM_, 1) == 0)
			inPRINT_Buffer_PutIn("5.加密模式 = tSAM加密", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (memcmp(szTemplate, _NCCC_ENCRYPTION_SOFTWARE_, 1) == 0)
			inPRINT_Buffer_PutIn("5.加密模式 = 軟體加密", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetSplitTransCheckEnable(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("6.不可連續刷卡 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("6.不可連續刷卡 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetCityName(szTemplate);
		sprintf(szPrintBuffer, "7.城市別 = %s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		inGetStoreIDEnable(szPrintBuffer);

		if (!memcmp(szPrintBuffer, "Y", 1))
		{
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			inGetMinStoreIDLen(szTemplate);
			inGetMaxStoreIDLen(szTemplate2);
			sprintf(szPrintBuffer, "8.櫃號 = On   長度 = %s ~ %s", szTemplate, szTemplate2);
		}
		else
		{
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			inGetMinStoreIDLen(szTemplate);
			inGetMaxStoreIDLen(szTemplate2);
			sprintf(szPrintBuffer, "8.櫃號 = Off  長度 = %s ~ %s", szTemplate, szTemplate2);
		}

		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetECREnable(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("9.ECR連線= On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("9.ECR連線 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetECRCardNoTruncateEnable(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("10.ECR卡號遮掩 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("10.ECR卡號遮掩 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetECRExpDateReturnEnable(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("11.ECR卡片有效期回傳 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("11.ECR卡片有效期回傳 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetProductCodeEnable(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("12.列印產品代碼 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("12.列印產品代碼 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetPrtSlogan(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("13.列印商店Slogan = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("13.列印商店Slogan = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (!memcmp(szTemplate, "Y", 1))
		{
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szTemplate2, 0x00, sizeof(szTemplate2));
			inGetSloganStartDate(szTemplate);
			inGetSloganEndDate(szTemplate2);
			sprintf(szPrintBuffer, "14.Slogan起迄日 = %s ~ %s", szTemplate, szTemplate2);
			inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetSloganPrtPosition(szTemplate);

			if (atoi(szTemplate) == _NCCC_SLOGAN_PRINT_DOWN_)
				inPRINT_Buffer_PutIn("15.Slogan列印位置 = 簽單下方", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else if (atoi(szTemplate) == _NCCC_SLOGAN_PRINT_UP_)
				inPRINT_Buffer_PutIn("15.Slogan列印位置 = 簽單上方", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			inPRINT_Buffer_PutIn("14.Slogan起迄日 =", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("15.Slogan列印位置 =", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetPrtMode(szTemplate);
		sprintf(szPrintBuffer, "16.簽單列印張數 = %s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetCUPRefundLimit(szTemplate);
		sprintf(szPrintBuffer, "17.CUP退貨限額 = %s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetCUPKeyExchangeTimes(szTemplate);
		sprintf(szPrintBuffer, "18.CUP安全認證 = %s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetMACEnable(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("19.上傳MAC = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("19.上傳MAC = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetPinpadMode(szTemplate);

		if (!memcmp(szTemplate, "0", 1))
		{
			inPRINT_Buffer_PutIn("20.密碼機 = None", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else if (!memcmp(szTemplate, "1", 1))
		{
			inPRINT_Buffer_PutIn("20.密碼機 = Internal", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else if (!memcmp(szTemplate, "2", 1))
		{
			inPRINT_Buffer_PutIn("20.密碼機 = External", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetFORCECVV2(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("21.強制輸入CVV2 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("21.強制輸入CVV2 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetElecCommerceFlag(szTemplate);

		if (!memcmp(szTemplate, "00", 2))
			inPRINT_Buffer_PutIn("22.EC參數 = 非郵購及定期性行業", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (!memcmp(szTemplate, "01", 2))
			inPRINT_Buffer_PutIn("22.EC參數 = 郵購行業", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (!memcmp(szTemplate, "02", 2))
			inPRINT_Buffer_PutIn("22.EC參數 = 定期性行業", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 端末機可下載封包數 */
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		sprintf(szPrintBuffer, "23.端末機可下載封包數 = %s", _EDC_TMS_TREMINAL_PACKET_SIZE_);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetDccFlowVersion(szTemplate);

		if (memcmp(szTemplate, _NCCC_DCC_FLOW_VER_NOT_SUPORTED_, 1) == 0)
			inPRINT_Buffer_PutIn("24.DCC Flow Version = Not Supported DCC", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (memcmp(szTemplate, _NCCC_DCC_FLOW_VER_BY_CARD_BIN_, 1) == 0)
			inPRINT_Buffer_PutIn("24.DCC Flow Version = By BIN Select", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (memcmp(szTemplate, _NCCC_DCC_FLOW_VER_BY_MANUAL_, 1) == 0)
			inPRINT_Buffer_PutIn("24.DCC Flow Version = By Currency Select", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetSupDccVisa(szTemplate);

                if (!memcmp(szTemplate, "Y", 1))
                        inPRINT_Buffer_PutIn("25.Support DCC VISA = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                else
                        inPRINT_Buffer_PutIn("25.Support DCC VISA = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetSupDccMasterCard(szTemplate);

                if (!memcmp(szTemplate, "Y", 1))
                        inPRINT_Buffer_PutIn("26.Support DCC M/C = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                else
                        inPRINT_Buffer_PutIn("26.Support DCC M/C = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetBarCodeReaderEnable(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("27.BarCode Reader = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("27.BarCode Reader = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetCreditCardFlag(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("28.刷卡兌換 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("28.刷卡兌換 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
                memset(szTemplate2, 0x00, sizeof(szTemplate2));
                inGetCreditCardStartDate(szTemplate);
                inGetCreditCardEndDate(szTemplate2);
		sprintf(szPrintBuffer, "29.刷卡兌換起迄日 = %s~%s", szTemplate, szTemplate2);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetBarCodeFlag(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("30.條碼兌換 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("30.條碼兌換 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
                memset(szTemplate2, 0x00, sizeof(szTemplate2));
                inGetBarCodeStartDate(szTemplate);
                inGetBarCodeEndDate(szTemplate2);
		sprintf(szPrintBuffer, "31.條碼兌換起迄日 = %s~%s", szTemplate, szTemplate2);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetVoidRedeemFlag(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("32.條碼兌換取消 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("32.條碼兌換取消 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
                memset(szTemplate2, 0x00, sizeof(szTemplate2));
                inGetVoidRedeemStartDate(szTemplate);
                inGetVoidRedeemEndDate(szTemplate2);
		sprintf(szPrintBuffer, "33.條碼兌換取消起迄日 = %s~%s", szTemplate, szTemplate2);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetASMFlag(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("34.優惠功能 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("34.優惠功能 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
                memset(szTemplate2, 0x00, sizeof(szTemplate2));
                inGetASMStartDate(szTemplate);
                inGetASMEndDate(szTemplate2);
		sprintf(szPrintBuffer, "35.優惠功能參數起迄日 = %s~%s", szTemplate, szTemplate2);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetStore_Stub_CardNo_Truncate_Enable(szTemplate);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetContactlessReaderMode(szTemplate);
		if (memcmp(szTemplate, _CTLS_MODE_1_INTERNAL_, 1) == 0)
			inPRINT_Buffer_PutIn("36.感應 = 內建", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (!memcmp(szTemplate, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
                         !memcmp(szTemplate, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)) )
			inPRINT_Buffer_PutIn("36.感應 = 外接", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
		        inPRINT_Buffer_PutIn("36.感應 = 無設定", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetSignPadMode(szTemplate);
		if (memcmp(szTemplate, _SIGNPAD_MODE_1_INTERNAL_, 1) == 0)
			inPRINT_Buffer_PutIn("38.簽名模式 = 內建", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (memcmp(szTemplate, _SIGNPAD_MODE_2_EXTERNAL_, 1) == 0)
			inPRINT_Buffer_PutIn("38.簽名模式 = 外接", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
		        inPRINT_Buffer_PutIn("38.簽名模式 = 無設定", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSDownloadMode(szTemplate);
		if (memcmp(szTemplate, _TMS_MODE_1_ISO_, 1) == 0)
			inPRINT_Buffer_PutIn("39.TMS下載模式 = ISO", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (memcmp(szTemplate, _TMS_MODE_2_FTPS_, 1) == 0)
			inPRINT_Buffer_PutIn("39.TMS下載模式 = FTP", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 繳費項目 */
		memset(szTemplate, 0x00, sizeof(szTemplate));;
		inGetPayItemEnable(szTemplate);

		if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
			inPRINT_Buffer_PutIn("40.Payitem Enable = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("40.Payitem Enable = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* ESC開關 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetESCMode(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
		{
		        inPRINT_Buffer_PutIn("41.ESC Enable = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetESCReciptUploadUpLimit(szTemplate);
		        sprintf(szPrintBuffer, "   ESC Limit = %s ", szTemplate);
		        inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
			inPRINT_Buffer_PutIn("41.ESC Enable = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 商店自存聯卡號遮掩 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetStore_Stub_CardNo_Truncate_Enable(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
			inPRINT_Buffer_PutIn("42.商店自存聯卡號遮掩 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("42.商店自存聯卡號遮掩 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 縮小簽單 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetShort_Receipt_Mode(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
			inPRINT_Buffer_PutIn("43.縮小簽單功能 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("43.縮小簽單功能 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 整合性設備 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetIntegrate_Device(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
			inPRINT_Buffer_PutIn("44.整合型週邊設備 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("44.整合型週邊設備 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* IFES */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetI_FES_Mode(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
			inPRINT_Buffer_PutIn("45.I-FESMode = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("45.I-FESMode = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* DHCP */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetDHCP_Mode(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
			inPRINT_Buffer_PutIn("46.DHCP = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("46.DHCP = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 電票功能及自動詢卡優先順序 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		inGetESVC_Priority(szTemplate);
		if (strlen(szTemplate) > 0)
		{
			sprintf(szPrintBuffer, "47.電票順序 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			inPRINT_Buffer_PutIn("47.電票順序 = 無", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		/* CloudMFes */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetCloud_MFES(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
			inPRINT_Buffer_PutIn("48.CloudMFes = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("48.CloudMFes = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		break;
	}

	return (VS_SUCCESS);
}

int inCREDIT_PRINT_ParamLOGO_END_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;

	inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        if (inPRINT_Buffer_PutGraphic((unsigned char *)_END_LOGO_, uszBuffer, srBhandle, 40, _APPEND_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

	for (i = 0; i < 8; i++)
	{
	   inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Printing_Time
Date&Time       :2016/10/19 上午 11:08
Describe        :列印"列印時間"
*/
int inCREDIT_PRINT_Printing_Time(TRANSACTION_OBJECT *pobTran, int inFontSize, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        int		inRetVal;
        char		szPrintBuf[84 + 1], szTemplate[42 + 1];
	RTC_NEXSYS	srRTC;

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	memset(&srRTC, 0x00, sizeof(srRTC));
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
        strcpy(szPrintBuf, "列印時間 :");
        memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "20%02d/%02d/%02d  ", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
        strcat(szPrintBuf, szTemplate);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "%02d:%02d:%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
        strcat(szPrintBuf, szTemplate);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_FISC_Data_ByBuffer
Date&Time       :2016/11/23 下午 3:58
Describe        :
*/
int inCREDIT_PRINT_FISC_Data_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
	int     inRetVal;
	char 	szPrintBuf[84 + 1], szPrintBuf1[42 + 1], szPrintBuf2[42 + 1], szTemplate1[42 + 1], szTemplate2[42 + 1];
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	/* 發卡行代碼 */
	inRetVal = inPRINT_Buffer_PutIn("發卡行代碼", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memcpy(szPrintBuf1, &pobTran->srBRec.szFiscIssuerID[0], 3);
	sprintf(szPrintBuf, "%s", szPrintBuf1);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);


	/* 卡別 */
	inRetVal = inPRINT_Buffer_PutIn("卡別(Card Type)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	/* 卡別 */
	inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 12, _PAD_LEFT_FILL_RIGHT_);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 卡號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcpy(szPrintBuf, pobTran->srBRec.szPAN);
	inRetVal = inPRINT_Buffer_PutIn("卡號(Card No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcpy(szPrintBuf, pobTran->srBRec.szPAN);
	/* 卡號遮掩 */
	if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
	{
		for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
			szPrintBuf[i] = 0x2A;
	}
	else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
	{
		/* 商店聯卡號遮掩 */
		memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
		inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
		if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
		{
			if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
			{
				for (i = 3; i < (strlen(szPrintBuf) - 5); i ++)
					szPrintBuf[i] = 0x2A;
			}
			else
			{
				for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
					szPrintBuf[i] = 0x2A;
			}
		}

	}

	/* 過卡方式 */
	if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
	{
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
			strcat(szPrintBuf,"(C)");
		else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
			strcat(szPrintBuf, "(T)");
		else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szPrintBuf, "(W)");
		else
		{
			if (pobTran->srBRec.uszManualBit == VS_TRUE)
			{
				/* 【需求單-105244】端末設備支援以感應方式進行退貨交易 */
				/* 電文轉Manual Keyin但是簽單要印感應的W */
				if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
					strcat(szPrintBuf, "(W)");
				else
					strcat(szPrintBuf,"(M)");
			}
			else
				strcat(szPrintBuf,"(S)");
		}

	}
	else
	{
		if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szPrintBuf, "(W)");
		else
			strcat(szPrintBuf, "(C)");
	}

	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 主機別 & 交易別 */
	inRetVal = inPRINT_Buffer_PutIn("主機別/交易類別(Host/Trans.Type)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	inGetHostLabel(szTemplate1);
	sprintf(szPrintBuf1, "%s", szTemplate1);

	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	vdEDC_GetTransType(pobTran, szTemplate1, szTemplate2);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));

	sprintf(szPrintBuf2, "%s", szTemplate1);
	sprintf(szPrintBuf, "%s %s", szPrintBuf1 , szPrintBuf2);

	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	if (strlen(szTemplate2) > 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate2);

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	/* 批次號碼 */
	inRetVal = inPRINT_Buffer_PutIn("批次號碼(Batch No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* Batch Num */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%03ld", pobTran->srBRec.lnBatchNum);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 日期時間 */
	inRetVal = inPRINT_Buffer_PutIn("日期/時間(Date/Time)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);

	inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 序號 調閱編號 */
	inRetVal = inPRINT_Buffer_PutIn("序號(Ref. No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("調閱編號(Inv.No)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);


	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	/* 雖然電文RRN送12個byte，但RRN最後一碼是0x00，所以只看到11碼 */
	inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szRefNo, ' ', 12, _PAD_LEFT_FILL_RIGHT_);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "%06ld", pobTran->srBRec.lnOrgInvNum);
	inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 調單編號 */
	inRetVal = inPRINT_Buffer_PutIn("調單編號(RRN NO.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//	memcpy(szPrintBuf, pobTran->srBRec.szFiscRRN, _FISC_RRN_SIZE_);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);



	/* 櫃號 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inGetStoreIDEnable(szTemplate1);
	if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
	{
		inRetVal = inPRINT_Buffer_PutIn("櫃號(Store ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szStoreID, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	/* 產品代碼 */
	inGetProductCodeEnable(szProductCodeEnable);
	if (memcmp(szProductCodeEnable, "Y", 1) == 0)
	{
		inRetVal = inPRINT_Buffer_PutIn("產品代碼(Product Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szProductCode, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	/* 商店聯卡號遮掩 */
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
	if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&&
	    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
	    strlen(pobTran->srBRec.szTxnNo) > 0)
	{
		inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_FISC_Amount_ByBuffer
Date&Time       :2016/11/23 下午 3:58
Describe        :
 */
int inCREDIT_PRINT_FISC_Amount_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 橫式 */
        /* 負向交易 */
	if(pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		/* 橫式 */
                /* 金額 */
                /* 取消退貨是正數 */
                if (pobTran->srBRec.inOrgCode == _FISC_REFUND_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 15, _PAD_RIGHT_FILL_LEFT_);

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "總計(Total) :%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		else
		{
			/* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 15, _PAD_RIGHT_FILL_LEFT_);

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "總計(Total) :%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}
	else
	{

		/* 初始化 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
		if (pobTran->srBRec.inCode == _FISC_REFUND_)
		{
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
		}
		else
		{
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		}

		inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 15, _PAD_RIGHT_FILL_LEFT_);

		/* 把前面的字串和數字結合起來 */
		sprintf(szPrintBuf, "總計(Total) :%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	}

	/* 斷行 */
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_AutoSettle_Failed_ByBuffer
Date&Time       :2016/10/13 下午 6:27
Describe        :列印結帳失敗的資料
*/
int inCREDIT_PRINT_AutoSettle_Failed_ByBuffer()
{
	int			i;
	int			inRetVal;
	char			szHostName[8 + 1];
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;

	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{

		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

		inRetVal = inPRINT_Buffer_PutIn("******************************************", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		memset(szHostName, 0x00, sizeof(szHostName));
		inRetVal = inGetHostLabel(szHostName);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		/* 去除空白，使列印置中 */
		inFunc_DiscardSpace(szHostName);

		inRetVal = inPRINT_Buffer_PutIn(szHostName, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		inRetVal = inPRINT_Buffer_PutIn("結帳失敗，請重試。", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		inRetVal = inPRINT_Buffer_PutIn("******************************************", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		for (i = 0; i < 8; i++)
		{
			inRetVal = inPRINT_Buffer_PutIn(" ", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			if (inRetVal != VS_SUCCESS)
			{
				return (VS_ERROR);
			}
		}

		if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer, &srBhandle)) != VS_SUCCESS)
			return (inRetVal);

		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_TerminalTraceLog_ByBuffer
Date&Time       :2017/5/19 下午 4:20
Describe        :列印端末機的 TRACE LOG
*/
int inCREDIT_PRINT_TerminalTraceLog_ByBuffer(void)
{
	int		i, j = 0, inCnt = 1;
	int		inRetVal;
	int		inReadSize;
	char		*szReadData = NULL;
	char		szPrintBuf[256 + 1], szTemplate[42 + 1];
	unsigned long	ulHandle = 0;
	unsigned char	uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle	srBhandle;
	FONT_ATTRIB	srFont_Attrib;

	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

		inRetVal = inFILE_OpenReadOnly(&ulHandle, (unsigned char*)_TMS_TRACE_LOG_);
		if (inRetVal != VS_SUCCESS)
		{
			inPRINT_Buffer_PutIn("!!NO_TERM._TRACE_LOG!!", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			inReadSize = (int)lnFILE_GetSize(&ulHandle, (unsigned char*)_TMS_TRACE_LOG_);
			szReadData = malloc(inReadSize + 1);
			inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_);
			inRetVal = inFILE_Read(&ulHandle, (unsigned char*)szReadData, (unsigned long)inReadSize);
			if (inRetVal == VS_SUCCESS)
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				for (i = 0; i < inReadSize; i ++)
				{
					szPrintBuf[j ++] = szReadData[i];
					if (szReadData[i] == 0x0A && szReadData[i - 1] == 0x0D)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "%03d. =====================================", (inCnt ++));
						inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						szPrintBuf[j] = 0x00;
						szPrintBuf[j - 1] = 0x00;
						szPrintBuf[j - 2] = 0x00;
						inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						j = 0;
					}
				}
			}
		}

		inFILE_Close(&ulHandle);
		free(szReadData);

		inPRINT_Buffer_PutIn(" ", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn(" ", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn(" ", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn(" ", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);

		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_Data_ByBuffer_ESVC
Date&Time       :2018/1/8 下午 5:37
Describe        :列印DATA
*/
int inCREDIT_PRINT_Data_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inLen = 0;
	int     inRetVal = VS_SUCCESS;
	char 	szPrintBuf[84 + 1], szTemplate1[42 + 1];
	char	szTemplate[40 + 1];
	char	szProductCodeEnable[1 + 1];

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	/* 橫式 */
	/* 卡號、卡別 */
	inRetVal = inPRINT_Buffer_PutIn("卡號(Card No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("卡別(Card Type)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
	{
	        strcpy(szPrintBuf, "一卡通");
	}
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
	        strcpy(szPrintBuf, "悠遊卡");
	}

	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 卡號值 */
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inLen = strlen(pobTran->srTRec.szUID);
		memcpy(szPrintBuf, pobTran->srTRec.szUID, inLen);
		szPrintBuf[inLen - 1] = 0x2A;
                szPrintBuf[inLen - 2] = 0x2A;
		strcat(szPrintBuf, "(W)");

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	else
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inLen = strlen(pobTran->srTRec.szUID);
		memcpy(szPrintBuf, pobTran->srTRec.szUID, inLen);
		strcat(szPrintBuf, "(W)");

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	/* 交易別 & 特店代號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_)
	        strcpy(szTemplate, "購貨　　");
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_REFUND_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_)
	        strcpy(szTemplate, "退貨　　");
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_TOP_UP_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_)
	        strcpy(szTemplate, "現金加值");
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_)
	        strcpy(szTemplate, "加值取消");
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_)
	        strcpy(szTemplate, "餘額查詢");
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_AUTO_TOP_UP_)
		strcpy(szTemplate, "自動加值");
	else
		strcpy(szTemplate, "　　");

	sprintf(szPrintBuf, "交易　　 %s", szTemplate);

	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);


	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	sprintf(szPrintBuf, "特店代號 %s", szTemplate1);

	inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetIPASS_System_ID(szTemplate);
                memcpy(&szPrintBuf[0], szTemplate, 2);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetIPASS_SP_ID(szTemplate);
        	memcpy(&szPrintBuf[2], szTemplate, 2);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetIPASS_Sub_Company_ID(szTemplate);
        	memcpy(&szPrintBuf[4], szTemplate, 4);
        }
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetECC_New_SP_ID(szTemplate);
	        memcpy(&szPrintBuf[0], szTemplate, 8);
	}

	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);


	/* 主機、調閱編號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	inGetHostLabel(szTemplate);

	sprintf(szPrintBuf, "主機　　 %s", szTemplate);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	if (pobTran->srTRec.inCode != _TICKET_IPASS_INQUIRY_	&&
	    pobTran->srTRec.inCode != _TICKET_EASYCARD_INQUIRY_)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "調閱號　 %s", "");
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%06ld", pobTran->srTRec.lnInvNum);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	else
	{
		/* 現在查詢餘額不印簽單 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, " ");
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}



	/* 城市 & 批號 */
	if (pobTran->srTRec.inTicketType != _TICKET_IPASS_INQUIRY_	&&
	    pobTran->srTRec.inTicketType != _TICKET_EASYCARD_INQUIRY_)
	{
	        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetCityName(szTemplate);

		sprintf(szPrintBuf, "城市　　 %s", szTemplate);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	sprintf(szPrintBuf, "批號　　 %s", "");
        	inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inGetBatchNum(szPrintBuf);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }

	/* 日期時間 */
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
        	memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%.4s/%.2s/%.2s  %.2s/%.2s/%.2s",
			&pobTran->srTRec.srECCRec.szDate[0],
			&pobTran->srTRec.srECCRec.szDate[4],
			&pobTran->srTRec.srECCRec.szDate[6],
			&pobTran->srTRec.srECCRec.szTime[0],
			&pobTran->srTRec.srECCRec.szTime[2],
			&pobTran->srTRec.srECCRec.szTime[4]);
        }
        else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
        	sprintf(szTemplate, "20%.2s/%.2s/%.2s  %.2s:%.2s",
			&pobTran->srTRec.szDate[0],
			&pobTran->srTRec.szDate[2],
			&pobTran->srTRec.szDate[4],
			&pobTran->srTRec.szTime[0],
			&pobTran->srTRec.szTime[2]);
        }

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "日期/時間 %s", szTemplate);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 序號 & RF序號 */
        if (pobTran->srTRec.inCode != _TICKET_IPASS_INQUIRY_	&&
	    pobTran->srTRec.inCode != _TICKET_EASYCARD_INQUIRY_)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	memset(szTemplate, 0x00, sizeof(szTemplate));
		/* 雖然電文RRN送12個byte，但RRN最後一碼是0x00，所以只看到11碼 */
		inFunc_PAD_ASCII(szTemplate, pobTran->srTRec.szRefNo, ' ', 12, _PAD_LEFT_FILL_RIGHT_);

		sprintf(szPrintBuf, "序號　　 %s", szTemplate);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);


		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	sprintf(szPrintBuf, "RF序號　 %s", "");
        	inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	memcpy(szPrintBuf, pobTran->srTRec.szTicketRefundCode, 6);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }

        if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
        {
		/* 設備編號 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTicket_Device1(szTemplate);
        	sprintf(szPrintBuf, "一代設備編號　 %s",szTemplate);
        	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTicket_Device2(szTemplate);
        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	sprintf(szPrintBuf, "二代設備編號　 %s", szTemplate);
        	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTicket_Batch(szTemplate);
        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	sprintf(szPrintBuf, "悠遊卡批次號碼 %s", szTemplate);
        	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, pobTran->srTRec.srECCRec.szRRN);
        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	sprintf(szPrintBuf, "RRN    　　　 %s", szTemplate);
        	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	/* 櫃號 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inGetStoreIDEnable(szTemplate1);
	if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srTRec.szStoreID) > 0))
	{
		inRetVal = inPRINT_Buffer_PutIn("櫃號(Store ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inRetVal = inPRINT_Buffer_PutIn(pobTran->srTRec.szStoreID, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	/* 產品代碼 */
	inGetProductCodeEnable(szProductCodeEnable);
	if (memcmp(szProductCodeEnable, "Y", 1) == 0)
	{
		inRetVal = inPRINT_Buffer_PutIn("產品代碼(Product Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inRetVal = inPRINT_Buffer_PutIn(pobTran->srTRec.szProductCode, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	/* 斷行 */
	inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_Amount_ByBuffer_IPASS
Date&Time       :2018/1/16 下午 3:01
Describe        :
*/
int inCREDIT_PRINT_Amount_ByBuffer_IPASS(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_SUCCESS;
        char    szPrintBuf[42 + 1];

        if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_)
        {
                /* 自動加值金額 */
		inRetVal = inPRINT_Buffer_PutIn("自動加值金額 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

	        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount);
                inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_NONE_, 10, _PAD_RIGHT_FILL_LEFT_);
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
                        inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_MINUS_, 10, _PAD_RIGHT_FILL_LEFT_);
		}
                else
		{
                        inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_NONE_, 10, _PAD_RIGHT_FILL_LEFT_);
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

	        inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_NONE_, 10, _PAD_RIGHT_FILL_LEFT_);

                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

                inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }
        else
        {
        	if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_)
        	{
        	        if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
        	        {
                	        /* 自動加值金額 */
				inRetVal = inPRINT_Buffer_PutIn("自動加值金額 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount);
				inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_NONE_, 10, _PAD_RIGHT_FILL_LEFT_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
                        }
                }

                /* 交易前餘額 */
        	/* 交易前餘額 */
        	inRetVal = inPRINT_Buffer_PutIn("交易前餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
        	if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
        	{
        	        if (pobTran->srTRec.lnFinalBeforeAmt > 100000)
			{
        	                sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount + (pobTran->srTRec.lnFinalBeforeAmt - 100000));
			}
        	        else
			{
        	                sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalBeforeAmt - pobTran->srTRec.lnTotalTopUpAmount);
			}
        	}
        	else
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
			inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_MINUS_, 10, _PAD_RIGHT_FILL_LEFT_);
		}
                else
		{
			inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_NONE_, 10, _PAD_RIGHT_FILL_LEFT_);
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

        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTxnAmount);
                inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_NONE_, 10, _PAD_RIGHT_FILL_LEFT_);

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
                        inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_MINUS_, 10, _PAD_RIGHT_FILL_LEFT_);
                }
                else
                {
                        sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalAfterAmt);
                        inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_NONE_, 10, _PAD_RIGHT_FILL_LEFT_);
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
Function        :inCREDIT_PRINT_Amount_ByBuffer_ECC
Date&Time       :2018/1/16 下午 3:05
Describe        :
*/
int inCREDIT_PRINT_Amount_ByBuffer_ECC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_SUCCESS;
        char    szPrintBuf[42 + 1];

        if (pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_ && pobTran->srTRec.lnTotalTopUpAmount > 0L)
        {
                /* 自動加值金額 */
	        inRetVal = inPRINT_Buffer_PutIn("自動加值金額 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount);
                inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_NONE_, 10, _PAD_RIGHT_FILL_LEFT_);

                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

                inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }

	/* 交易前餘額 */
	inRetVal = inPRINT_Buffer_PutIn("交易前餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalBeforeAmt);
	inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_NONE_, 10, _PAD_RIGHT_FILL_LEFT_);

	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 交易金額 */
	inRetVal = inPRINT_Buffer_PutIn("交易金額　　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTxnAmount);
	inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_NONE_, 10, _PAD_RIGHT_FILL_LEFT_);

	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 交易後餘額 */
	inRetVal = inPRINT_Buffer_PutIn("交易後餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalAfterAmt);
	inFunc_Amount_Comma(szPrintBuf, "$", ' ', _SIGNED_NONE_, 10, _PAD_RIGHT_FILL_LEFT_);

	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_ReceiptEND_ByBuffer_ESVC
Date&Time       :2018/1/16 下午 3:07
Describe        :
*/
int inCREDIT_PRINT_ReceiptEND_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
	int	inRetVal = VS_SUCCESS;

        if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
        {
                inRetVal = inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

                inRetVal = inPRINT_Buffer_PutIn("備註：若有疑問請洽一卡通票證公司", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

                inRetVal = inPRINT_Buffer_PutIn("客服專線：(07)791-2000", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		inRetVal = inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

                inRetVal = inPRINT_Buffer_PutIn("備註：若有疑問請洽", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

                inRetVal = inPRINT_Buffer_PutIn("悠遊卡公司專線：412-8880", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inRetVal = inPRINT_Buffer_PutIn("(手機及金馬地區請加02)", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
	if (inCREDIT_PRINT_Notice(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
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

/*
Function        :inCREDIT_PRINT_TotalReport_ByBuffer_ESVC
Date&Time       :2018/1/29 上午 11:20
Describe        :
*/
int inCREDIT_PRINT_TotalReport_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran)
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
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	/* 客製化107邦柏，不列印結帳條 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, strlen(_CUSTOMER_INDICATOR_107_BUMPER_)))
	{
		return (VS_SUCCESS);
	}
	else
	{

		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("----------------------------------------");
			inDISP_LogPrintf("inCREDIT_PRINT_TotalReport_ByBuffer_ESVC() START !");
		}

		if (pobTran->inTransactionCode == _SETTLE_)
		{
			inPrintIndex = _TOTAL_REPORT_INDEX_ESVC_SETTLE_;
		}
		else
		{
			inPrintIndex = _TOTAL_REPORT_INDEX_ESVC_;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
			inDISP_LogPrintf(szDebugMsg);
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

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

		inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
		/* 列印LOGO */
		if (srTotalReport_ByBuffer_ESVC[inPrintIndex].inReportLogo != NULL)
			if ((inRetVal = srTotalReport_ByBuffer_ESVC[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);
		/* 列印TID MID */
		if (srTotalReport_ByBuffer_ESVC[inPrintIndex].inReportTop != NULL)
			if ((inRetVal = srTotalReport_ByBuffer_ESVC[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);
		/* 全部金額總計 */
		if (srTotalReport_ByBuffer_ESVC[inPrintIndex].inAmount != NULL)
			if ((inRetVal = srTotalReport_ByBuffer_ESVC[inPrintIndex].inAmount(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);

		/* 卡別金額總計 */
		if (srTotalReport_ByBuffer_ESVC[inPrintIndex].inAmountByCard != NULL)
			if ((inRetVal = srTotalReport_ByBuffer_ESVC[inPrintIndex].inAmountByCard(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);

		/* 結束 */
		if (srTotalReport_ByBuffer_ESVC[inPrintIndex].inReportEnd != NULL)
			if ((inRetVal = srTotalReport_ByBuffer_ESVC[inPrintIndex].inReportEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);

		if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
			return (inRetVal);

		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_Logo_ByBuffer_ESVC
Date&Time       :2018/1/29 下午 1:57
Describe        :票證總額只印銀行LOGO
*/
int inCREDIT_PRINT_Logo_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        /* 印NCC的LOGO */
        if (inPRINT_Buffer_PutGraphic((unsigned char*)_BANK_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inBankLogoHeight, _APPEND_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Top_ESVC_ByBuffer
Date&Time       :2018/1/29 下午 2:49
Describe        :
*/
int inCREDIT_PRINT_Top_ESVC_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        /* Get商店代號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetMerchantID(szTemplate);

        /* 列印商店代號 */
        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PAD_RIGHT_FILL_LEFT_);
        sprintf(szPrintBuf, "商店代號%s", szTemplate);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        /* Get端末機代號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetTerminalID(szTemplate);

        /* 列印端末機代號 */
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PAD_RIGHT_FILL_LEFT_);
        sprintf(szPrintBuf, "端末機代號%s", szTemplate);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
	inGetBatchNum(szPrintBuf);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 列印日期時間 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "日期時間  :  %.4s/%.2s/%.2s %.2s:%.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2], &pobTran->srBRec.szTime[4]);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Top_ESVC_SETTLE_ByBuffer
Date&Time       :2018/1/29 下午 2:49
Describe        :
*/
int inCREDIT_PRINT_Top_ESVC_SETTLE_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        /* Get商店代號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetMerchantID(szTemplate);

        /* 列印商店代號 */
        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PAD_RIGHT_FILL_LEFT_);
        sprintf(szPrintBuf, "商店代號%s", szTemplate);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        /* Get端末機代號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetTerminalID(szTemplate);

        /* 列印端末機代號 */
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PAD_RIGHT_FILL_LEFT_);
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
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
	inGetBatchNum(szPrintBuf);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 列印日期時間 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "日期時間  :  %.4s/%.2s/%.2s %.2s:%.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2], &pobTran->srBRec.szTime[4]);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmount_ByBuffer_ESVC
Date&Time       :2018/1/29 下午 3:44
Describe        :列印總金額
*/
int inCREDIT_PRINT_TotalAmount_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[84 + 1];

	inPRINT_Buffer_PutIn("         總額報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        inPRINT_Buffer_PutIn("       筆數           金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 銷售 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "購貨 　　%03lu   NT$", srAccumRec->lnDeductTotalCount);
	sprintf(szTemplate, "%lld", srAccumRec->llDeductTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnRefundTotalCount);
	sprintf(szTemplate, "%lld", srAccumRec->llRefundTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 現金加值 */
	if (srAccumRec->lnADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "現金加值 %03lu   NT$", srAccumRec->lnADDTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llADDTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	/* 加值取消 */
	if (srAccumRec->lnVoidADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "加值取消 %03lu   NT$", srAccumRec->lnVoidADDTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llVoidADDTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 交易淨額(總購貨 - 總退貨) */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "交易淨額 %03lu   NT$", (srAccumRec->lnDeductTotalCount + srAccumRec->lnRefundTotalCount));
	sprintf(szTemplate, "%lld", (srAccumRec->llDeductTotalAmount - srAccumRec->llRefundTotalAmount));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 加值淨額 */
	if (srAccumRec->lnADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "加值淨額 %03lu   NT$", (srAccumRec->lnADDTotalCount + srAccumRec->lnVoidADDTotalCount));
		sprintf(szTemplate, "%lld", (srAccumRec->llADDTotalAmount - srAccumRec->llVoidADDTotalAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmount_ByBuffer_ESVC
Date&Time       :2018/1/29 下午 3:44
Describe        :列印總金額
*/
int inCREDIT_PRINT_TotalAmount_ByBuffer_ESVC_Settle(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char		szPrintBuf[84 + 1] = {0}, szTemplate[84 + 1] = {0};

	inPRINT_Buffer_PutIn("         結帳報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        inPRINT_Buffer_PutIn("       筆數           金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 銷售 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "購貨 　　%03lu   NT$", srAccumRec->lnDeductTotalCount);
	sprintf(szTemplate, "%lld", srAccumRec->llDeductTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnRefundTotalCount);
	sprintf(szTemplate, "%lld", srAccumRec->llRefundTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 現金加值 */
	if (srAccumRec->lnADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "現金加值 %03lu   NT$", srAccumRec->lnADDTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llADDTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	/* 加值取消 */
	if (srAccumRec->lnVoidADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "加值取消 %03lu   NT$", srAccumRec->lnVoidADDTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llVoidADDTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 交易淨額(總購貨 - 總退貨) */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "交易淨額 %03lu   NT$", (srAccumRec->lnDeductTotalCount + srAccumRec->lnRefundTotalCount));
	sprintf(szTemplate, "%lld", (srAccumRec->llDeductTotalAmount - srAccumRec->llRefundTotalAmount));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 加值淨額 */
	if (srAccumRec->lnADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "加值淨額 %03lu   NT$", (srAccumRec->lnADDTotalCount + srAccumRec->lnVoidADDTotalCount));
		sprintf(szTemplate, "%lld", (srAccumRec->llADDTotalAmount - srAccumRec->llVoidADDTotalAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByCard_ByBuffer_ESVC
Date&Time       :2018/1/29 下午 4:19
Describe        :依卡別列印
*/
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int		i = 0;
	char		szPrintBuf[100 + 1] = {0}, szTemplate[42 + 1] = {0};
	char		szTxnType[20 + 1] = {0};
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

                switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                inPRINT_Buffer_PutIn("卡別 一卡通", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);

				/* 特店代號 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "特店代號　%s", "");
				inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_System_ID(szTemplate);
				memcpy(&szPrintBuf[0], szTemplate, 2);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_SP_ID(szTemplate);
				memcpy(&szPrintBuf[2], szTemplate, 2);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_Sub_Company_ID(szTemplate);
				memcpy(&szPrintBuf[4], szTemplate, 4);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                inPRINT_Buffer_PutIn("卡別 悠遊卡", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);

				/* 特店代號 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "特店代號　%s", "");
				inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetECC_New_SP_ID(szTemplate);
				strcpy(&szPrintBuf[0], szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

				/* 二代設備編號 */
          	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetTicket_Device2(szTemplate);
                        	sprintf(szPrintBuf, "二代設備編號　 %s", szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

				/* 悠遊卡批次號碼 */
                        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetTicket_Batch(szTemplate);
                        	sprintf(szPrintBuf, "悠遊卡批次號碼 %s", szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        case _TDT_INDEX_02_ICASH_ :
        	                inPRINT_Buffer_PutIn("卡別 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        case _TDT_INDEX_03_HAPPYCASH_ :
        	                inPRINT_Buffer_PutIn("卡別 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        default :
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
                switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnIPASS_DeductTotalCount);
        	                sprintf(szTemplate, "%lld", srAccumRec->llIPASS_DeductTotalAmount);
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnEASYCARD_DeductTotalCount);
        	                sprintf(szTemplate, "%lld", srAccumRec->llEASYCARD_DeductTotalAmount);
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", 0l);
        	                sprintf(szTemplate, "%lld", 0ll);
        	                break;
        	}

        	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
        	strcat(szPrintBuf, szTemplate);
        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);


		/* (有開通遠鑫卡時設定開啟及有筆數時列印) */
		if (i == _TDT_INDEX_03_HAPPYCASH_		&&
		    szTxnType[4] == 0x59			&&
		    srAccumRec->lnHAPPYCASH_VoidDeductTotalCount > 0)
        	{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			switch(i)
			{
				case _TDT_INDEX_00_IPASS_ :
					sprintf(szPrintBuf, "購貨取消　%03lu   NT$", srAccumRec->lnHAPPYCASH_VoidDeductTotalCount);
					sprintf(szTemplate, "%lld", (srAccumRec->llHAPPYCASH_VoidDeductTotalAmount));
					break;
				default :
					sprintf(szPrintBuf, "購貨取消　%03lu   NT$", 0l);
					sprintf(szTemplate, "%lld", 0ll);
					break;
			}

			inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		}


        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnIPASS_RefundTotalCount);
        	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_RefundTotalAmount));
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnEASYCARD_RefundTotalCount);
        	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_RefundTotalAmount));
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", 0l);
        	                sprintf(szTemplate, "%lld", 0ll);
        	                break;
        	}

		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
        	strcat(szPrintBuf, szTemplate);
        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

		/* 加值功能有開且有交易筆數 */
        	if (szTxnType[4] == 0x59 &&
		   ((i == _TDT_INDEX_00_IPASS_ && srAccumRec->lnIPASS_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_01_ECC_ && srAccumRec->lnEASYCARD_ADDTotalCount > 0)))
        	{
        	        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                	switch(i)
                        {
                                case _TDT_INDEX_00_IPASS_ :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", srAccumRec->lnIPASS_ADDTotalCount);
                	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_ADDTotalAmount));
                	                break;
                	        case _TDT_INDEX_01_ECC_ :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", srAccumRec->lnEASYCARD_ADDTotalCount);
                	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_ADDTotalAmount));
                	                break;
                	        default :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", 0l);
                	                sprintf(szTemplate, "%lld", 0ll);
                	                break;
                	}

                	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	}

		/* 加值取消功能有開且有交易筆數 */
        	if (szTxnType[5] == 0x59	&&
		   ((i == _TDT_INDEX_00_IPASS_ && srAccumRec->lnIPASS_VoidADDTotalCount > 0)	||
		    (i == _TDT_INDEX_01_ECC_ && srAccumRec->lnEASYCARD_VoidADDTotalCount > 0)))
        	{
                	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                	switch(i)
                        {
                                case _TDT_INDEX_00_IPASS_ :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", srAccumRec->lnIPASS_VoidADDTotalCount);
                	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_VoidADDTotalAmount));
                	                break;
                	        case _TDT_INDEX_01_ECC_ :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", srAccumRec->lnEASYCARD_VoidADDTotalCount);
                	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_VoidADDTotalAmount));
                	                break;
                	        default :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", 0l);
                	                sprintf(szTemplate, "%lld", 0ll);
                	                break;
                	}

                	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                }

        	inPRINT_Buffer_PutIn("------------------------------------------", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", (srAccumRec->lnIPASS_DeductTotalCount + srAccumRec->lnIPASS_RefundTotalCount));
        	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_DeductTotalAmount - srAccumRec->llIPASS_RefundTotalAmount));
        	                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
                        	strcat(szPrintBuf, szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", (srAccumRec->lnEASYCARD_DeductTotalCount + srAccumRec->lnEASYCARD_RefundTotalCount));
        	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_DeductTotalAmount - srAccumRec->llEASYCARD_RefundTotalAmount));
        	                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
                        	strcat(szPrintBuf, szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", 0l);
        	                sprintf(szTemplate, "%lld", 0ll);
        	                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
                        	strcat(szPrintBuf, szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	}

		if ((i == _TDT_INDEX_00_IPASS_ && srAccumRec->lnIPASS_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_01_ECC_ && srAccumRec->lnEASYCARD_ADDTotalCount > 0))
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			switch(i)
			{
				case _TDT_INDEX_00_IPASS_ :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", (srAccumRec->lnIPASS_ADDTotalCount + srAccumRec->lnIPASS_VoidADDTotalCount));
					sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_ADDTotalAmount - srAccumRec->llIPASS_VoidADDTotalAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
				case _TDT_INDEX_01_ECC_ :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", (srAccumRec->lnEASYCARD_ADDTotalCount + srAccumRec->lnEASYCARD_VoidADDTotalCount));
					sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_ADDTotalAmount - srAccumRec->llEASYCARD_VoidADDTotalAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
				default :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", 0l);
					sprintf(szTemplate, "%lld", 0ll);
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
			}
		}

        	inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        }

        inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByCard_ByBuffer_ESVC_Settle
Date&Time       :2018/1/30 下午 2:32
Describe        :依卡別列印
*/
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer_ESVC_Settle(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
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

                switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                inPRINT_Buffer_PutIn("卡別 一卡通", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                inPRINT_Buffer_PutIn("卡別 悠遊卡", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

				/* 二代設備編號 */
          	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetTicket_Device2(szTemplate);
                        	sprintf(szPrintBuf, "二代設備編號　 %s", szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

				/* 悠遊卡批次號碼 */
                        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetTicket_Batch(szTemplate);
                        	sprintf(szPrintBuf, "悠遊卡批次號碼 %s", szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
				/* 列印交易時間 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szPrintBuf, "交易時間  :  %.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        	                break;
        	        case _TDT_INDEX_02_ICASH_ :
        	                inPRINT_Buffer_PutIn("卡別 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        case _TDT_INDEX_03_HAPPYCASH_ :
        	                inPRINT_Buffer_PutIn("卡別 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        default :
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
                switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnIPASS_DeductTotalCount);
        	                sprintf(szTemplate, "%lld", srAccumRec->llIPASS_DeductTotalAmount);
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnEASYCARD_DeductTotalCount);
        	                sprintf(szTemplate, "%lld", srAccumRec->llEASYCARD_DeductTotalAmount);
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", 0l);
        	                sprintf(szTemplate, "%lld", 0ll);
        	                break;
        	}

        	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
        	strcat(szPrintBuf, szTemplate);
        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);


		/* (有開通遠鑫卡時設定開啟及有筆數時列印) */
		if (i == _TDT_INDEX_03_HAPPYCASH_		&&
		    szTxnType[4] == 0x59			&&
		    srAccumRec->lnHAPPYCASH_VoidDeductTotalCount > 0)
        	{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			switch(i)
			{
				case _TDT_INDEX_00_IPASS_ :
					sprintf(szPrintBuf, "購貨取消　%03lu   NT$", srAccumRec->lnHAPPYCASH_VoidDeductTotalCount);
					sprintf(szTemplate, "%lld", (srAccumRec->llHAPPYCASH_VoidDeductTotalAmount));
					break;
				default :
					sprintf(szPrintBuf, "購貨取消　%03lu   NT$", 0l);
					sprintf(szTemplate, "%lld", 0ll);
					break;
			}

			inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		}


        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnIPASS_RefundTotalCount);
        	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_RefundTotalAmount));
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnEASYCARD_RefundTotalCount);
        	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_RefundTotalAmount));
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", 0l);
        	                sprintf(szTemplate, "%lld", 0ll);
        	                break;
        	}

		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
        	strcat(szPrintBuf, szTemplate);
        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

		/* 加值功能有開且有交易筆數 */
        	if (szTxnType[4] == 0x59  &&
		   ((i == _TDT_INDEX_00_IPASS_ && srAccumRec->lnIPASS_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_01_ECC_ && srAccumRec->lnEASYCARD_ADDTotalCount > 0)))
        	{
        	        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                	switch(i)
                        {
                                case _TDT_INDEX_00_IPASS_ :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", srAccumRec->lnIPASS_ADDTotalCount);
                	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_ADDTotalAmount));
                	                break;
                	        case _TDT_INDEX_01_ECC_ :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", srAccumRec->lnEASYCARD_ADDTotalCount);
                	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_ADDTotalAmount));
                	                break;
                	        default :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", 0l);
                	                sprintf(szTemplate, "%lld", 0ll);
                	                break;
                	}

                	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	}

		/* 加值取消功能有開且有交易筆數 */
        	if (szTxnType[5] == 0x59  &&
		   ((i == _TDT_INDEX_00_IPASS_ && srAccumRec->lnIPASS_VoidADDTotalCount > 0)	||
		    (i == _TDT_INDEX_01_ECC_ && srAccumRec->lnEASYCARD_VoidADDTotalCount > 0)))
        	{
                	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                	switch(i)
                        {
                                case _TDT_INDEX_00_IPASS_ :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", srAccumRec->lnIPASS_VoidADDTotalCount);
                	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_VoidADDTotalAmount));
                	                break;
                	        case _TDT_INDEX_01_ECC_ :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", srAccumRec->lnEASYCARD_VoidADDTotalCount);
                	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_VoidADDTotalAmount));
                	                break;
                	        default :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", 0l);
                	                sprintf(szTemplate, "%lld", 0ll);
                	                break;
                	}

                	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                }

        	inPRINT_Buffer_PutIn("------------------------------------------", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", (srAccumRec->lnIPASS_DeductTotalCount + srAccumRec->lnIPASS_RefundTotalCount));
        	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_DeductTotalAmount - srAccumRec->llIPASS_RefundTotalAmount));
        	                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
                        	strcat(szPrintBuf, szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", (srAccumRec->lnEASYCARD_DeductTotalCount + srAccumRec->lnEASYCARD_RefundTotalCount));
        	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_DeductTotalAmount - srAccumRec->llEASYCARD_RefundTotalAmount));
        	                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
                        	strcat(szPrintBuf, szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", 0l);
        	                sprintf(szTemplate, "%lld", 0ll);
        	                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
                        	strcat(szPrintBuf, szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	}

		if ((i == _TDT_INDEX_00_IPASS_ && srAccumRec->lnIPASS_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_01_ECC_ && srAccumRec->lnEASYCARD_ADDTotalCount > 0))
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			switch(i)
			{
				case _TDT_INDEX_00_IPASS_ :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", (srAccumRec->lnIPASS_ADDTotalCount + srAccumRec->lnIPASS_VoidADDTotalCount));
					sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_ADDTotalAmount - srAccumRec->llIPASS_VoidADDTotalAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
				case _TDT_INDEX_01_ECC_ :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", (srAccumRec->lnEASYCARD_ADDTotalCount + srAccumRec->lnEASYCARD_VoidADDTotalCount));
					sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_ADDTotalAmount - srAccumRec->llEASYCARD_VoidADDTotalAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
				default :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", 0l);
					sprintf(szTemplate, "%lld", 0ll);
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PAD_RIGHT_FILL_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
			}
		}

        	inPRINT_Buffer_PutIn(" ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        }

        inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_End_ByBuffer_ESVC
Date&Time       :2018/1/29 下午 4:13
Describe        :列印結尾
*/
int inCREDIT_PRINT_End_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;

        inPRINT_Buffer_PutIn("*** 報表結束 ***", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

        for (i = 0; i < 8; i++)
	{
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DetailReport_ByBuffer_ESVC
Date&Time       :2018/1/31 上午 10:16
Describe        :列印明細帳單
*/
int inCREDIT_PRINT_DetailReport_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = 0, inPrintIndex = 0, inRecordCnt = 0;
	char			szDebugMsg[100 + 1];
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
        TICKET_ACCUM_TOTAL_REC	srAccumRec;

	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

		inPrintIndex = _DETAIL_REPORT_INDEX_ESVC_;

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
			inDISP_LogPrintf(szDebugMsg);
		}

		/* 檢查是否有帳 */
		if (srDetailReport_ByBuffer_ESVC[inPrintIndex].inReportCheck != NULL)
		{
			if ((inRecordCnt = srDetailReport_ByBuffer_ESVC[inPrintIndex].inReportCheck(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) == VS_ERROR)
				return (VS_ERROR); /* 表示檔案開啟失敗 */
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
			inDISP_LogPrintf("Get record 失敗.");

			return (VS_ERROR);
		}

		while (1)
		{
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
			/* 列印LOGO */
			if (srDetailReport_ByBuffer_ESVC[inPrintIndex].inReportLogo != NULL)
				if ((inRetVal = srDetailReport_ByBuffer_ESVC[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 列印TID MID */
			if (srDetailReport_ByBuffer_ESVC[inPrintIndex].inReportTop != NULL)
				if ((inRetVal = srDetailReport_ByBuffer_ESVC[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 全部金額總計 */
			if (srDetailReport_ByBuffer_ESVC[inPrintIndex].inTotalAmount != NULL)
				if ((inRetVal = srDetailReport_ByBuffer_ESVC[inPrintIndex].inTotalAmount(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			/* 明細規格 */
			if (srDetailReport_ByBuffer_ESVC[inPrintIndex].inMiddle != NULL)
				if ((inRetVal = srDetailReport_ByBuffer_ESVC[inPrintIndex].inMiddle(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 明細資料 */
			if (srDetailReport_ByBuffer_ESVC[inPrintIndex].inBottom != NULL)
			{
				inRetVal = srDetailReport_ByBuffer_ESVC[inPrintIndex].inBottom(pobTran, inRecordCnt, uszBuffer1, &srFont_Attrib1, &srBhandle1);
				if (inRetVal != VS_SUCCESS	&&
				    inRetVal != VS_NO_RECORD)
				{
				       return (inRetVal);
				}
			}

			/* 結束 */
			if (srDetailReport_ByBuffer_ESVC[inPrintIndex].inReportEnd != NULL)
				if ((inRetVal = srDetailReport_ByBuffer_ESVC[inPrintIndex].inReportEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);

			break;
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_Check_ByBuffer_ESVC
Date&Time       :2018/1/31 上午 11:50
Describe        :
*/
int inCREDIT_PRINT_Check_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        int     inRecordCnt;

        inRecordCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite_ESVC(pobTran);
        /* 回傳VS_ERROR(回傳 -1 )會跳出，交易筆數小於0( VS_NoRecord 會回傳 -98 )會印空白簽單 */
        /* 其餘則回傳交易筆數*/

        return (inRecordCnt);
}

/*
Function        :inCREDIT_PRINT_DetailReportMiddle_ByBuffer_ESVC
Date&Time       :2018/1/31 上午 11:27
Describe        :
*/
int inCREDIT_PRINT_DetailReportMiddle_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        inPRINT_Buffer_PutIn("明細報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

	inPRINT_Buffer_PutIn("調閱編號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn_Specific_X_Position("交易金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

        inPRINT_Buffer_PutIn("交易類別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn_Specific_X_Position("卡別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

        inPRINT_Buffer_PutIn("卡號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	inPRINT_Buffer_PutIn("交易日期", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn_Specific_X_Position("交易時間", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

	inPRINT_Buffer_PutIn("RF序號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DetailReportBottom_ByBuffer_ESVC
Date&Time       :2018/1/31 上午 11:27
Describe        :
*/
int inCREDIT_PRINT_DetailReportBottom_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inReadCnt = 0;
	int	inRetVal = VS_SUCCESS;
        char	szPrintBuf[62 + 1], szTemplate1[62 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCREDIT_PRINT_DetailReportBottom_ByBuffer_ESVC()_START");
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

		/* 調閱編號 & Amount */
		/* Invoice Number */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "INV:%06ld", pobTran->srTRec.lnInvNum);
		strcat(szPrintBuf, szTemplate1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* Print Amount */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));

		switch (pobTran->srTRec.inCode)
		{
			case _TICKET_IPASS_AUTO_TOP_UP_:
				sprintf(szTemplate1, "%ld", pobTran->srTRec.lnTotalTopUpAmount);
				break;
			default :
				sprintf(szTemplate1, "%ld", pobTran->srTRec.lnTxnAmount);
				break;
		} /* End switch () */

		inFunc_Amount_Comma(szTemplate1, "NT$" , 0x00, _SIGNED_NONE_, 16, _PAD_LEFT_FILL_RIGHT_);
		strcat(szPrintBuf, szTemplate1);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 交易類別 & 卡別 */
		/* Trans Type */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inFunc_GetTransType_ESVC(pobTran, szTemplate1);
		sprintf(szPrintBuf, "%s", szTemplate1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

		/* 卡別 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
		{
		        strcat(szPrintBuf, "一卡通");
		}
		else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
		{
		        strcat(szPrintBuf, "悠遊卡");
		}
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);


		/* 卡號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
		{
			memcpy(szPrintBuf, pobTran->srTRec.szUID, strlen(pobTran->srTRec.szUID) - 2);
			szPrintBuf[strlen(pobTran->srTRec.szUID) - 2] = 0x2A;
        		szPrintBuf[strlen(pobTran->srTRec.szUID) - 1] = 0x2A;
		}
		else
		{
			strcpy(szPrintBuf, pobTran->srTRec.szUID);
		}
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* Trans Date Time */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "DATE: 20%.4s/%.2s/%.2s", &pobTran->srTRec.szDate[0], &pobTran->srTRec.szDate[2], &pobTran->srTRec.szDate[4]);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "TIME: %.2s:%.2s",  &pobTran->srTRec.szTime[0], &pobTran->srTRec.szTime[2]);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* RF序號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, "RF no: ");
		strcat(szPrintBuf, pobTran->srTRec.szTicketRefundCode);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        } /* End for () .... */

	/* 結束讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_END(pobTran);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCREDIT_PRINT_DetailReportBottom_ByBuffer_ESVC()_END");
	}

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_ESC_Reinforce_Count_ByBuffer
Date&Time       :2018/5/31 下午 5:47
Describe        :【需求單 - 105259】總額明細報表及總額明細查詢補強機制
*/
int inCREDIT_PRINT_ESC_Reinforce_Count_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_SUCCESS;
	int     inSaleUploadCnt = 0, inRefundUploadCnt = 0, inSalePaperCnt = 0, inRefundPaperCnt = 0;
	long    lnSaleUploadAmt = 0, lnRefundUploadAmt = 0, lnSalePaperAmt = 0, lnRefundPaperAmt = 0;
	char    szPrintBuf[84 + 1] = {0}, szTemplate[84 + 1] = {0};
	char	szFuncEnable[2 + 1] = {0};

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inNCCC_ESC_Reinforce_Count() START !");
	}

	/* ESC沒開直接跳走 */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetESCMode(szFuncEnable);
	if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0)
	{
		return (VS_SUCCESS);
	}

	if (pobTran->inTransactionCode == _SETTLE_)
	{
		inSaleUploadCnt = pobTran->inESC_Sale_UploadCnt;
		inRefundUploadCnt = pobTran->inESC_Refund_UploadCnt;
		inSalePaperCnt = pobTran->inESC_Sale_PaperCnt;
		inRefundPaperCnt = pobTran->inESC_Refund_PaperCnt;
		lnSaleUploadAmt = pobTran->lnESC_Sale_UploadAmt;
		lnRefundUploadAmt = pobTran->lnESC_Refund_UploadAmt;
		lnSalePaperAmt = pobTran->lnESC_Sale_PaperAmt;
		lnRefundPaperAmt = pobTran->lnESC_Refund_PaperAmt;
	}
	else
	{
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
	}

	inPRINT_Buffer_PutIn("==========================================", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("銷售   筆數(CNT)   金額(AMOUNT)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 銷售 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	sprintf(szPrintBuf, "已上傳   　%03d   NT$", inSaleUploadCnt);
	sprintf(szTemplate, "%ld", lnSaleUploadAmt);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 12, _PAD_RIGHT_FILL_LEFT_);

	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	sprintf(szPrintBuf, "未上傳   　%03d   NT$", inSalePaperCnt);
	sprintf(szTemplate, "%ld", lnSalePaperAmt);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 12, _PAD_RIGHT_FILL_LEFT_);

	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	inPRINT_Buffer_PutIn("退貨   筆數(CNT)   金額(AMOUNT)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	sprintf(szPrintBuf, "已上傳   　%03d   NT$", inRefundUploadCnt);
	sprintf(szTemplate, "%ld", lnRefundUploadAmt);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 12, _PAD_RIGHT_FILL_LEFT_);

	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	sprintf(szPrintBuf, "未上傳   　%03d   NT$", inRefundPaperCnt);
	sprintf(szTemplate, "%ld", lnRefundPaperAmt);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 12, _PAD_RIGHT_FILL_LEFT_);

	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("==========================================", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	return (inRetVal);
}

/*
Function        :inCREDIT_PRINTBYBUFFER_Receipt_Test
Date&Time       :2016/9/2 下午 4:23
Describe        :
*/
int inCREDIT_PRINTBYBUFFER_Receipt_Test(void)
{
	int			i;
	char			szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1], szTransType[42 + 1], szHostLabel[8 + 1];
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	FONT_ATTRIB		srFont_Attrib;
	BufferHandle		srBhandle;

	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(uszBuffer, 0x00, sizeof(uszBuffer));

		inPRINT_Buffer_PutIn("RIGHT", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutGraphic((unsigned char*)_BANK_LOGO_, uszBuffer, &srBhandle, 40, _APPEND_);
		inPRINT_Buffer_PutGraphic((unsigned char*)_MERCHANT_LOGO_, uszBuffer, &srBhandle, 89, _APPEND_);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 17, _PAD_RIGHT_FILL_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, "0108000237", 15);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PAD_RIGHT_FILL_LEFT_);
		sprintf(szPrintBuf, "商店代號 %s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, "13994020", 8);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 13, _PAD_RIGHT_FILL_LEFT_);
		sprintf(szPrintBuf, "端末機代號 %s", szTemplate);

		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 城市別(City) */
		inPRINT_Buffer_PutIn("城市別(City)", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, "KINMEN-LIENCHIANG   ", 20);
		sprintf(szPrintBuf, "%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 卡別 授權碼 */
		inPRINT_Buffer_PutIn("卡別(Card Type)  授權碼(Auth Code)", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inFunc_PAD_ASCII(szPrintBuf, "AMEX", ' ', 13, _PAD_LEFT_FILL_RIGHT_);
		memcpy(&szTemplate[0], "123456", _AUTH_CODE_SIZE_);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 卡號 */
		inPRINT_Buffer_PutIn("卡號(Card No.)", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memcpy(szPrintBuf, "376348129192026    ", 19);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 主機別 & 交易別 */
		inPRINT_Buffer_PutIn("主機別/交易類別(Host/Trans. Type)", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szTransType, 0x00, sizeof(szTransType));

		memcpy(&szTemplate, "00 一般交易 SALE", 16);

		memset(szHostLabel, 0x00, sizeof(szHostLabel));
		memcpy(&szHostLabel, "NCCC    ", 8);
		sprintf(szPrintBuf, "%s %s", szHostLabel , szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 批次號碼 */
		inPRINT_Buffer_PutIn("批次號碼(Batch No.)", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memcpy(szPrintBuf, "001", 3);
		inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, '0', 3, _PAD_RIGHT_FILL_LEFT_);

		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 日期時間 */
		inPRINT_Buffer_PutIn("日期/時間(Date/Time)", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szPrintBuf[0], "2016/01/30", 10);
		memcpy(&szPrintBuf[13], "13:38", 5);

		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 序號 調閱編號 */
		inPRINT_Buffer_PutIn("序號(Ref. No.)　 調閱編號(Inv.No)", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		memcpy(szPrintBuf,"99402001001  000001", 19);

		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 櫃號 */
		inPRINT_Buffer_PutIn("櫃號(Store ID)", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memcpy(szPrintBuf,"                   ", 19);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 初始化 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memcpy(szTemplate, "100", 3);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 8, VS_TRUE);

		/* 把前面的字串和數字結合起來 */
		sprintf(szPrintBuf, "金額(Amount):NT$%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 小費 */
		inPRINT_Buffer_PutIn("小費(Tips)  :__________________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 總計 */
		inPRINT_Buffer_PutIn("總計(Total) :__________________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, &srBhandle, 50, _APPEND_);

		inPRINT_Buffer_PutIn("X:________________________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 持卡人姓名 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%s", "SAM");

		inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("　　　　　　　　　　持卡人簽名", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("            I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("        ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);

		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_Test
Date&Time       :2018/3/12 上午 10:53
Describe        :
*/
int inCREDIT_PRINT_Test(void)
{
	BYTE			key;
        unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * 800];
	CTOS_FONT_ATTRIB	srFont_Attrib;
//	// TODO: Add your program here //
//	CTOS_LCDTClearDisplay();
//
//        CTOS_LCDTPrintXY(1, 1, "Hello");

	srFont_Attrib.X_Zoom = 1;		/* 1, it means normal size, and 2 means double size. 0 means print nothing . */
	srFont_Attrib.Y_Zoom = 1;		/* 1, it means normal size, and 2 means double size. 0 means print nothing . */
	srFont_Attrib.X_Space = 0;		/* The space in dot to insert between each character in x coordinate. */
	srFont_Attrib.Y_Space = 0;		/* The space in dot to insert between each character in y coordinate. */
	srFont_Attrib.FontSize = d_FONT_12x24;

	CTOS_PrinterBufferInit(uszBuffer, 800);
	CTOS_PrinterBufferPutStringAligned(uszBuffer, 20, (unsigned char*)"123", &srFont_Attrib, d_PRINTER_ALIGNLEFT);
	CTOS_PrinterBufferPutString(uszBuffer, 160, 20, (unsigned char*)"123", &srFont_Attrib);
	CTOS_PrinterBufferPutStringAligned(uszBuffer, 20, (unsigned char*)"123", &srFont_Attrib, d_PRINTER_ALIGNRIGHT);

	CTOS_PrinterBufferOutput(uszBuffer, 12);

        CTOS_KBDGet(&key);

	return 0;
}


/*
Function        :inEDC_PRINT_ParamQRCodeConfig_ByBuffer
Date&Time       :2018/3/07 上午 15:00
Describe        :列印DATA
*/
int inEDC_PRINT_ParamQRCodeConfig_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char	szPrintBuffer[200 + 1];
	char	szTemplate[64 + 1];
	int	inCnt = 0;
	RTC_NEXSYS              srRTC;

	/* Qrcodee顯示規格如下，都以逗號隔開：
	信用卡商代,信用卡端代,機器S/N,嘉利版本日期(YYYYMMDD),嘉利版本名稱,列印參數表時間年月日時分(YYYYMMDDhhmm),外接設備1,外接設備2,COMPORTMODE
	EX:
	111111111111111,22222222,12345678,20190710,NEXSYS000_UF,201907100918,4,0,3
	外接設備：
	0 = 無外接設備，1 = PINPAD1000，2 = QP3000，3 = PP1000CTLS (V3) (二合一)，4 = Vx820，5 = GHL (暫定保留)，6 = V3UltraLite */
	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));

	/* 信用卡商代 */
	if (inLoadHDTRec(0) == VS_ERROR) /* 主機參數檔 */
	{
		inDISP_DispLogAndWriteFlie(" EDC Print QRCode Parm Load HDT[%d] *Error* Line[%d]",0, __LINE__);
	}

	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetMerchantID(szTemplate);
	memcpy(&szPrintBuffer[inCnt], &szTemplate[0], strlen(szTemplate));
	inCnt += strlen(szTemplate);

	szPrintBuffer[inCnt++] = 0x2c;

	/* 信用卡端代 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);
	memcpy(&szPrintBuffer[inCnt], &szTemplate[0], strlen(szTemplate));
	inCnt += strlen(szTemplate);

	szPrintBuffer[inCnt++] = 0x2c;
        
	/* 機器S/N */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_GetSeriaNumber(szTemplate);
	szTemplate[15] = 0x00;
	memcpy(&szPrintBuffer[inCnt], &szTemplate[0], strlen(szTemplate));
	inCnt += strlen(szTemplate);

	szPrintBuffer[inCnt++] = 0x2c;

	/* 嘉利版本日期 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (strlen(gszTermVersionDate) > 0)
		memcpy(&szPrintBuffer[inCnt], &gszTermVersionDate[0], 8);
	else
		memcpy(&szPrintBuffer[inCnt], "00000000", 8);
	inCnt += 8;

	szPrintBuffer[inCnt++] = 0x2c;

	 /* 嘉利版本名稱 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (strlen(gszTermVersionID) > 0)
	{
		memcpy(&szPrintBuffer[inCnt], &gszTermVersionID[0], strlen(gszTermVersionID));
		inCnt += strlen(gszTermVersionID);
	}
	else
	{
		memcpy(&szPrintBuffer[inCnt], "000000000", 9);
		inCnt += 9;
	}
	
	szPrintBuffer[inCnt++] = 0x2c;
        
	/* 列印參數表時間年月日時分 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(&srRTC, 0x00, sizeof(srRTC));
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "20%02d%02d%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute);
	memcpy(&szPrintBuffer[inCnt], &szTemplate[0], 12);
	inCnt += 12;

	szPrintBuffer[inCnt++] = 0x2c;

	/* 外接設備 */
	memcpy(&szPrintBuffer[inCnt], "0,0,0", 5);
	inCnt += 5;

	/* 列印QR CODE */
	inPRINT_Buffer_QRcode(szPrintBuffer, uszBuffer, srBhandle, _PRINT_DEFINE_X_03_);

	return (VS_SUCCESS);
}



