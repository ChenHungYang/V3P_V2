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

#include "../../FUNCTION/COSTCO_FUNC/Costco.h"

#include "../../TMS/TMSTABLE/TmsCPT.h"
#include "../../TMS/TMSTABLE/TmsSCT.h"

#include "../../EVENT/Flow.h"
#include "../../EVENT/MenuMsg.h"

#include "CreditPrintFunc.h"

extern  int     ginDebug;  /* Debug使用 extern */
extern	char	gszTermVersionID[16 + 1];
extern	char	gszTermVersionDate[16 + 1];
char		MemoSignBMPFile[32 + 1]; /* 電子簽名 */

int     inPrinttype = 0;        /* 0 = 橫式，1 = 直式 */

/* 列印帳單使用(START) */
PRINT_RECEIPT_TYPE_TABLE srReceiptType[] =
{
       /* 信用卡 */
       {
                inCREDIT_PRINT_Logo,
                inCREDIT_PRINT_Tidmid,
                inCREDIT_PRINT_Data,
                _NULL_CH_,
                inCREDIT_PRINT_Amount,
                inCREDIT_PRINT_Inst,
                inCREDIT_PRINT_Redeem,
                _NULL_CH_,
                inCREDIT_PRINT_Receiptend
       }
};

/* 列印總額報表使用 (START) */
TOTAL_REPORT_TABLE srTotalReport[] =
{
       {
               inCREDIT_PRINT_Check,
               inCREDIT_PRINT_Logo,
               inCREDIT_PRINT_Top,
               inCREDIT_PRINT_TotalAmount,
               inCREDIT_PRINT_TotalAmountByCard,
               inCREDIT_PRINT_TotalAmountByInstllment,
               inCREDIT_PRINT_TotalAmountByRedemption,
               inCREDIT_PRINT_End
       }
};

/* 列印明細報表使用 (START) */
DETAIL_REPORT_TABLE srDetailReport[] =
{
       {
                inCREDIT_PRINT_Check,                   /*inReportCheck*/
                inCREDIT_PRINT_Logo,                    /*inReportLogo*/
                inCREDIT_PRINT_Top,                     /*inReportTop*/
                inCREDIT_PRINT_TotalAmount,             /*inTotalAmount*/
                inCREDIT_PRINT_DetailReportMiddle,      /*inMiddle*/
                inCREDIT_PRINT_DetailReportBottom,      /*inBottom*/
                inCREDIT_PRINT_End                      /*inReportEnd*/
       }
};

/*
Function        :inCREDIT_PRINT_Receipt
Date&Time       :2015/8/10 上午 10:24
Describe        :列印信用卡
*/
int inCREDIT_PRINT_Receipt(TRANSACTION_OBJECT *pobTran)
{
        int     inPrintIndex = 0, inRetVal;

        while (1)
        {
                /* 列印LOGO */
                if (srReceiptType[inPrintIndex].inLogo != NULL)
                        if ((inRetVal = srReceiptType[inPrintIndex].inLogo(pobTran)) != VS_SUCCESS)
                                return (inRetVal);
                /* 列印TID MID */
                if (srReceiptType[inPrintIndex].inTop != NULL)
                        if ((inRetVal = srReceiptType[inPrintIndex].inTop(pobTran)) != VS_SUCCESS)
                                return (inRetVal);
                /* 列印DATA */
                if (srReceiptType[inPrintIndex].inData != NULL)
                        if ((inRetVal = srReceiptType[inPrintIndex].inData(pobTran)) != VS_SUCCESS)
                                return (inRetVal);
                /* 列印CUP金額 */
                if (srReceiptType[inPrintIndex].inCUPAmount != NULL)
                        if ((inRetVal = srReceiptType[inPrintIndex].inCUPAmount(pobTran)) != VS_SUCCESS)
                                return (inRetVal);
                /* 列印金額 */
                if (srReceiptType[inPrintIndex].inAmount != NULL)
                        if ((inRetVal = srReceiptType[inPrintIndex].inAmount(pobTran)) != VS_SUCCESS)
                                return (inRetVal);
                /* 分期資料 */
                if (srReceiptType[inPrintIndex].inInstallment != NULL)
                        if ((inRetVal = srReceiptType[inPrintIndex].inInstallment(pobTran)) != VS_SUCCESS)
                                return (inRetVal);
                /* 紅利資料 */
                if (srReceiptType[inPrintIndex].inRedemption != NULL)
                        if ((inRetVal = srReceiptType[inPrintIndex].inRedemption(pobTran)) != VS_SUCCESS)
                                return (inRetVal);
                /* OTHER資料 */
                if (srReceiptType[inPrintIndex].inOther != NULL)
                        if ((inRetVal = srReceiptType[inPrintIndex].inOther(pobTran)) != VS_SUCCESS)
                                return (inRetVal);
                /* 列印簽名欄  & 警語 */
                if (srReceiptType[inPrintIndex].inEnd != NULL)
                        if ((inRetVal = srReceiptType[inPrintIndex].inEnd(pobTran)) != VS_SUCCESS)
                                return (inRetVal);

                break;
        }

        return (VS_SUCCESS);
}


/*
Function        :inCREDIT_PRINT_TotalReport
Date&Time       :2016/3/7 下午 3:57
Describe        :列印總額帳單
*/
int inCREDIT_PRINT_TotalReport(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = 0, inPrintIndex = 0 ,inRecordCnt = 0;
        ACCUM_TOTAL_REC		srAccumRec;
        TOTAL_REPORT_TABLE	srTotalReport[8 + 1];

        while (1)
        {
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

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
                        /* 列印LOGO */
                        if (srTotalReport[inPrintIndex].inReportLogo != NULL)
                                if ((inRetVal = srTotalReport[inPrintIndex].inReportLogo(pobTran)) != VS_SUCCESS)
                                        return (inRetVal);

                        /* 列印TID MID */
                        if (srTotalReport[inPrintIndex].inReportTop != NULL)
                                if ((inRetVal = srTotalReport[inPrintIndex].inReportTop(pobTran)) != VS_SUCCESS)
                                        return (inRetVal);
                        /* 全部金額總計 */
                        if (srTotalReport[inPrintIndex].inAmount != NULL)
                                if ((inRetVal = srTotalReport[inPrintIndex].inAmount(pobTran, &srAccumRec, inRecordCnt)) != VS_SUCCESS)
                                        return (inRetVal);

                        /* 有金額才印 */
                        if (srAccumRec.llTotalSaleAmount != 0L || srAccumRec.llTotalRefundAmount != 0L)
                        {
                                /* 卡別金額總計 */
                                if (srTotalReport[inPrintIndex].inAmountByCard != NULL)
                                        if ((inRetVal = srTotalReport[inPrintIndex].inAmountByCard(pobTran, &srAccumRec, inRecordCnt)) != VS_SUCCESS)
                                                return (inRetVal);
                                /* 分期金額總計 */
                                if (srTotalReport[inPrintIndex].inAmountByInstallment != NULL)
                                        if ((inRetVal = srTotalReport[inPrintIndex].inAmountByInstallment(pobTran, &srAccumRec, inRecordCnt)) != VS_SUCCESS)
                                                return (inRetVal);
                                /* 紅利金額總計 */
                                if (srTotalReport[inPrintIndex].inAmountByRedemption != NULL)
                                        if ((inRetVal = srTotalReport[inPrintIndex].inAmountByRedemption(pobTran, &srAccumRec, inRecordCnt)) != VS_SUCCESS)
                                                return (inRetVal);
                        }

                        /* 結束 */
                        if (srTotalReport[inPrintIndex].inReportEnd != NULL)
                                if ((inRetVal = srTotalReport[inPrintIndex].inReportEnd(pobTran)) != VS_SUCCESS)
                                        return (inRetVal);

                        break;
                }

                break;
        }

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DetailReport
Date&Time       :2016/3/7 下午 4:08
Describe        :列印明細帳單
*/
int inCREDIT_PRINT_DetailReport(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = 0 ,inPrintIndex = 0 ,inRecordCnt = 0;
        ACCUM_TOTAL_REC		srAccumRec;
        DETAIL_REPORT_TABLE	srDetailReport[7 + 1];

        while (1)
        {
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

                /* 檢查是否有帳 */
                if (srDetailReport[inPrintIndex].inReportCheck != NULL)
                {
                        if ((inRecordCnt = srDetailReport[inPrintIndex].inReportCheck(pobTran)) == VS_ERROR)
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
                        /* 列印LOGO */
                        if (srDetailReport[inPrintIndex].inReportLogo != NULL)
                                if ((inRetVal = srDetailReport[inPrintIndex].inReportLogo(pobTran)) != VS_SUCCESS)
                                        return (inRetVal);

                        /* 列印TID MID */
                        if (srDetailReport[inPrintIndex].inReportTop != NULL)
                                if ((inRetVal = srDetailReport[inPrintIndex].inReportTop(pobTran)) != VS_SUCCESS)
                                        return (inRetVal);
                        /* 明細規格 */
                        if (srDetailReport[inPrintIndex].inMiddle != NULL)
                                if ((inRetVal = srDetailReport[inPrintIndex].inMiddle(pobTran, inRecordCnt)) != VS_SUCCESS)
                                        return (inRetVal);
                        /* 明細資料 */
                       if (srDetailReport[inPrintIndex].inBottom != NULL)
                               if (
                                       (inRetVal = srDetailReport[inPrintIndex].inBottom(pobTran, inRecordCnt)) != VS_SUCCESS &&
                                       (inRetVal = srDetailReport[inPrintIndex].inBottom(pobTran, inRecordCnt)) != VS_NO_RECORD
                                  )
                                       return (inRetVal);
                        /* 結束 */
                        if (srDetailReport[inPrintIndex].inReportEnd != NULL)
                                if ((inRetVal = srDetailReport[inPrintIndex].inReportEnd(pobTran)) != VS_SUCCESS)
                                        return (inRetVal);

                        break;
                }
                break;
        }

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_LOGO
Date&Time       :2015/8/10 上午 10:24
Describe        :列印LOGO
*/
int inCREDIT_PRINT_Logo(TRANSACTION_OBJECT *pobTran)
{
        /* 印NCC的LOGO */
        if (inPRINT_PutGraphic((unsigned char*)_BANK_LOGO_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 印商店的LOGO */
        if (inPRINT_PutGraphic((unsigned char*)_MERCHANT_LOGO_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TIDMID
Date&Time       :2015/8/10 上午 10:24
Describe        :列印TID & MID
*/
int inCREDIT_PRINT_Tidmid(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        if (inPrinttype)
        {
                /* 直式 */
                /* Get商店代號 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetMerchantID(szTemplate);

                /* 列印商店代號 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "商店代號：%s", szTemplate);
                inRetVal = inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* Get端末機代號 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetTerminalID(szTemplate);

                /* 列印端末機代號 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "端末機代號：%s", szTemplate);
                inRetVal = inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

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
                inRetVal = inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* Get端末機代號 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetTerminalID(szTemplate);

                /* 列印端末機代號 */
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PAD_RIGHT_FILL_LEFT_);
                sprintf(szPrintBuf, "端末機代號%s", szTemplate);
                inRetVal = inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                inRetVal = inPRINT_ChineseFont("================================================", _PRT_NORMAL_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
        }

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_DATA
Date&Time       :2015/8/10 上午 10:24
Describe        :列印DATA
*/
int inCREDIT_PRINT_Data(TRANSACTION_OBJECT *pobTran)
{
int     inRetVal;
char 	szPrintBuf[84 + 1], szPrintBuf1[42 + 1], szPrintBuf2[42 + 1], szTemplate1[42 + 1], szTemplate2[42 + 1];

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));


	if (inPrinttype)
	{
		/* 直式 */

		/*卡別、卡號*/
		sprintf(szPrintBuf, "卡別　　：%s", pobTran->srBRec.szCardLabel);
		inRetVal = inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "卡號　　：%s", pobTran->srBRec.szPAN);
		inRetVal = inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

		/*日期、時間*/
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "日期　　：%s",pobTran->srBRec.szDate);
		inRetVal = inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "時間　　：%s",pobTran->srBRec.szTime);
		inRetVal = inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

		/*調閱編號、批次號碼 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "調閱編號：%06ld",pobTran->srBRec.lnOrgInvNum);
		inRetVal = inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "批次號碼：%06ld",pobTran->srBRec.lnBatchNum);
		inRetVal = inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

		/*交易類別*/
		inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "交易類別：%s",szTemplate1);
		inRetVal = inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

		/*授權碼、序號*/
		sprintf(szPrintBuf, "授權碼　：%s",pobTran->srBRec.szAuthCode);
		inRetVal = inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "序號　　：%s",pobTran->srBRec.szRefNo);
		inRetVal = inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	}
	else
	{
		/* 橫式 */

		/* 城市別(City) */
		inRetVal = inPRINT_ChineseFont("城市別(City)", _PRT_HEIGHT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		inGetCityName(szPrintBuf1);
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		inRetVal = inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 卡別 授權碼 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		inRetVal = inPRINT_ChineseFont("卡別(Card Type)授權碼(Auth Code)", _PRT_HEIGHT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 12, _PAD_LEFT_FILL_RIGHT_);
		memcpy(&szPrintBuf1[0], &pobTran->srBRec.szAuthCode[0], _AUTH_CODE_SIZE_);
		strcat(szPrintBuf, szPrintBuf1);
		inRetVal = inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 卡號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, pobTran->srBRec.szPAN);
		inRetVal = inPRINT_ChineseFont("卡號(Card No.)", _PRT_HEIGHT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inRetVal = inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 主機別 & 交易別 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szTemplate2, 0x00, sizeof(szTemplate2));

                inRetVal = inPRINT_ChineseFont("主機別/交易類別(Host/Trans.Type)", _PRT_HEIGHT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inGetHostLabel(szTemplate1);
		sprintf(szPrintBuf1, "%s", szTemplate1);

		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
		sprintf(szPrintBuf2, "%s %s", szTemplate1, szTemplate2);

                sprintf(szPrintBuf, "%s %s", szPrintBuf1 , szPrintBuf2);
		inRetVal = inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 批次號碼 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%ld", pobTran->srBRec.lnBatchNum);
                inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, '0', 3, _PAD_RIGHT_FILL_LEFT_);
		inRetVal = inPRINT_ChineseFont("批次號碼(Batch No.)", _PRT_HEIGHT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inRetVal = inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 日期時間 */
		inRetVal = inPRINT_ChineseFont("日期/時間(Date/Time)", _PRT_HEIGHT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		memcpy(&szPrintBuf[0], &pobTran->srBRec.szDate[0], 4);
		strcat(szPrintBuf, "/");
		memcpy(&szPrintBuf[5], &pobTran->srBRec.szDate[4], 2);
		strcat(szPrintBuf, "/");
		memcpy(&szPrintBuf[8], &pobTran->srBRec.szDate[6], 2);
		strcat(szPrintBuf, "  ");
		memcpy(&szPrintBuf[12], &pobTran->srBRec.szTime[0], 2);
		strcat(szPrintBuf, ":");
		memcpy(&szPrintBuf[15], &pobTran->srBRec.szTime[2], 2);

		inRetVal = inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 序號 調閱編號 */
                inRetVal = inPRINT_ChineseFont("序號(Ref. No.) 調閱編號(Inv.No)", _PRT_HEIGHT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		/* 雖然電文RRN送12個byte，但RRN最後一碼是0x00，所以只看到11碼 */
		if (pobTran->srBRec.inCode == _SALE_OFFLINE_)
		{
			inFunc_PAD_ASCII(szPrintBuf,szPrintBuf,' ',12,_PAD_RIGHT_FILL_LEFT_);
                        sprintf(szPrintBuf, "%s %06ld",szPrintBuf, pobTran->srBRec.lnOrgInvNum);
		}
		else
		{
                        sprintf(szPrintBuf, "%s%06ld", pobTran->srBRec.szRefNo, pobTran->srBRec.lnOrgInvNum);
		}

		inRetVal = inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 櫃號 */
		inRetVal = inPRINT_ChineseFont("櫃號(Store ID)", _PRT_HEIGHT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inRetVal = inPRINT_ChineseFont(pobTran->srBRec.szStoreID, _PRT_DOUBLE_HEIGHT_WIDTH_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_CUP_AMOUNT
Date&Time       :2015/8/10 上午 10:24
Describe        :列印銀聯AMOUNT
*/
int inCREDIT_PRINT_Cup_Amount(TRANSACTION_OBJECT *pobTran)
{

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_AMOUNT
Date&Time       :2015/8/10 上午 10:24
Describe        :列印AMOUNT
*/
int inCREDIT_PRINT_Amount(TRANSACTION_OBJECT *pobTran)
{
	char    szPrintBuf[84 + 1], szTemplate[42 + 1];

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	if (inPrinttype)
	{
		/* 直式 */
		/* 金額 */
		if (pobTran->srBRec.inCode == _TIP_)
		{
			/* 金額 */
			/* 初始化 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, VS_TRUE);

			/* 把前面的字串和數字結合起來 */
			sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
			inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);


			/* 小費 */
			/* 初始化 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTipTxnAmount);
			inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, VS_TRUE);

			/* 把前面的字串和數字結合起來 */
			sprintf(szPrintBuf, "小費(Tips)  :%s", szTemplate);
			inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);


			/* 總計 */
			/* 初始化 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "NT$ %ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
			inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, VS_TRUE);

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
			inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, VS_TRUE);

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
			inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, VS_TRUE);

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
			inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, VS_TRUE);

			/* 把前面的字串和數字結合起來 */
			sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
		}
		inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);
		inPRINT_SpaceLine(2);
	}
	/* 橫式 */
	/* 小費 */
	else if ( pobTran->srBRec.inCode == _TIP_)
	{
		/* 金額 */
		/* 初始化 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, VS_TRUE);

		/* 把前面的字串和數字結合起來 */
		sprintf(szPrintBuf, "金額(Amount):NT$%s", szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_HEIGHT_);


		/* 小費 */
		/* 初始化 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTipTxnAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, VS_TRUE);

		/* 把前面的字串和數字結合起來 */
		sprintf(szPrintBuf, "小費(Tips)  :NT$%s", szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_HEIGHT_);


		/* 總計 */
		/* 初始化 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		sprintf(szTemplate, "%ld",  (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, VS_TRUE);

		/* 把前面的字串和數字結合起來 */
		sprintf(szPrintBuf, "總計(Total) :NT$%s", szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_HEIGHT_);
	}
	/* 橫式非小費的其他交易 */
	else
	{
		/* 橫式 */
		/* 金額 */
		/* 取消跟退貨因為是負數的關係，但取消退貨是正數所以當例外*/
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE &&
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
			sprintf(szTemplate, "%ld",  0 - pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, VS_TRUE);

			/* 把前面的字串和數字結合起來 */
			sprintf(szPrintBuf, "金額(Amount):NT$%s", szTemplate);
		}
		/* 調帳的數值放在pobTran->srBRec.lnAdjustTxnAmount中所以獨立出來 */
		else if (pobTran->srBRec.inCode == _ADJUST_)
		{
			/* 初始化 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnAdjustTxnAmount);
			inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, VS_TRUE);
			sprintf(szPrintBuf, "%s" , "金額(Amount):");

			/* 把前面的字串和數字結合起來 */
			sprintf(szPrintBuf, "金額(Amount):NT$%s", szTemplate);
		}
		else
		{
			/* 初始化 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_,  15, VS_TRUE);

			/* 把前面的字串和數字結合起來 */
			sprintf(szPrintBuf, "金額(Amount):NT$%s", szTemplate);
		}

		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_HEIGHT_);

		/* 取消和退貨不印小費和總計 ，只印金額 */
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE &&
			(pobTran->srBRec.inOrgCode != _REFUND_ && pobTran->srBRec.inOrgCode != _INST_REFUND_ && pobTran->srBRec.inOrgCode != _REDEEM_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_MAIL_ORDER_REFUND_))	||
			pobTran->srBRec.inCode == _REFUND_		||
			pobTran->srBRec.inCode == _INST_REFUND_	||
			pobTran->srBRec.inCode == _REDEEM_REFUND_	||
			pobTran->srBRec.inCode == _CUP_REFUND_	||
			pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
		{
			/* 小費 */
			inPRINT_ChineseFont("小費(Tips)  :__________________________", _PRT_HEIGHT_HEIGHT_);

			/* 總計 */
			inPRINT_ChineseFont("總計(Total) :__________________________", _PRT_HEIGHT_HEIGHT_);
		}
	}

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_INST
Date&Time       :2015/8/10 上午 10:24
Describe        :列印分期
*/
int inCREDIT_PRINT_Inst(TRANSACTION_OBJECT *pobTran)
{
	char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	if (pobTran->srBRec.inCode == _INST_SALE_ || pobTran->srBRec.inOrgCode == _INST_SALE_)
	{
		/* 和總計金額隔開 */
		inPRINT_SpaceLine(1);

		/* 分期期數 */
		/* 初始化 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                /* 將NT$ ＋數字塞到szTemplate中來inpad */
                sprintf(szTemplate, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 18, VS_TRUE);

                /* 把前面的字串和數字結合起來 */
                sprintf(szPrintBuf, "分期期數   :%s期", szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_HEIGHT_);

		/* 首期金額 */
		/* 初始化 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                /* 將NT$ ＋數字塞到szTemplate中來inpad */
                sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentDownPayment));
                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 12, VS_TRUE);

                /* 把前面的字串和數字結合起來 */
                sprintf(szPrintBuf, "首期金額   :NT$ %s", szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_HEIGHT_);

		/* 每期金額 */
		/* 初始化 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                /* 將NT$ ＋數字塞到szTemplate中來inpad */
                sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentPayment));
                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 12, VS_TRUE);

                /* 把前面的字串和數字結合起來 */
                sprintf(szPrintBuf, "每期金額   :NT$ %s", szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_HEIGHT_);

		/* 分期手續費 */
		/* 初始化 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                /* 將NT$ ＋數字塞到szTemplate中來inpad */
                sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentFormalityFee));
                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 12, VS_TRUE);

                /* 把前面的字串和數字結合起來 */
                sprintf(szPrintBuf, "分期手續費 :NT$ %s", szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_HEIGHT_);
	}
	/* 不是分期 */
	else
	{

	}

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_REDEEM
Date&Time       :2015/8/10 上午 10:24
Describe        :列印紅利
*/
int inCREDIT_PRINT_Redeem(TRANSACTION_OBJECT *pobTran)
{
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	if (pobTran->srBRec.inCode == _REDEEM_SALE_ || pobTran->srBRec.inOrgCode == _REDEEM_SALE_)
	{
		/* 和總計金額隔開 */
		inPRINT_SpaceLine(1);

		/* 支付金額 */
		/* 初始化 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                /* 將NT$ ＋數字塞到szTemplate中來inpad */
                sprintf(szTemplate, "%02ld", pobTran->srBRec.lnRedemptionPaidCreditAmount);
                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 10, VS_TRUE);

                /* 把前面的字串和數字結合起來 */
                sprintf(szPrintBuf, "支付金額　　 :NT$%s", szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_HEIGHT_);

		/* 紅利扣抵金額 */
		/* 初始化 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                /* 將NT$ ＋數字塞到szTemplate中來inpad */
                sprintf(szTemplate, "%ld", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 10, VS_TRUE);

                /* 把前面的字串和數字結合起來 */
                sprintf(szPrintBuf, "紅利扣抵金額 :NT$%s", szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_HEIGHT_);

		/* 紅利扣抵點數 */
		/* 初始化 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                /* 將NT$ ＋數字塞到szTemplate中來inpad */
                sprintf(szTemplate, "%ld", (pobTran->srBRec.lnRedemptionPoints));
                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 16, VS_TRUE);

                /* 把前面的字串和數字結合起來 */
                sprintf(szPrintBuf, "紅利扣抵點數 :%s點", szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_HEIGHT_);

		/* 紅利剩餘點數 */
		/* 初始化 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                /* 將NT$ ＋數字塞到szTemplate中來inpad */
                sprintf(szTemplate, " %ld", (pobTran->srBRec.lnRedemptionPointsBalance));
                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 16, VS_TRUE);

                /* 把前面的字串和數字結合起來 */
                sprintf(szPrintBuf, "紅利剩餘點數 :%s點", szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_HEIGHT_);
	}
	/* 不是紅利 */
	else
	{

	}

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_RRECEIPTEND
Date&Time       :2015/8/10 上午 10:24
Describe        :列印結尾
*/
int inCREDIT_PRINT_Receiptend(TRANSACTION_OBJECT *pobTran)
{
	char	szTemplate[42 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};

        if (inPrinttype)
        {
                /* 直式 */
                inPRINT_ChineseFont("簽名欄:_____________________", _PRT_DOUBLE_HEIGHT_WIDTH_);

                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
                {
                        inPRINT_ChineseFont("*** 商店收據 Merchant Copy ***", _PRT_NORMAL_);
                        pobTran->srBRec.inPrintOption = _PRT_CUST_;

                }
                else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
                {
                        inPRINT_ChineseFont("*** 持卡人收據 Customer Copy ***", _PRT_NORMAL_);
                        pobTran->srBRec.inPrintOption = _PRT_MERCH_;
                }

                inPRINT_ChineseFont("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_);
                inPRINT_ChineseFont("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_);
                inPRINT_SpaceLine(8);
        }
        else
        {
                /* 橫式 */
                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
                {
                        /* 簽名欄 */
                        if (strlen(MemoSignBMPFile) > 0)
                        {
                                /* 電子簽名 */
                                inPRINT_PutGraphic((unsigned char *)MemoSignBMPFile);
                        }
                        else
                        {
                                inPRINT_SpaceLine(2);  //a space 2 line
                        }

			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_PutGraphic((unsigned char*)_NCCC_DEMO_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inPRINT_ChineseFont("inPRINT_PutGraphic(_NCCC_DEMO_) failed", _PRT_HEIGHT_);
					}

				}
			}

                        inPRINT_ChineseFont("X:________________________________", _PRT_DOUBLE_HEIGHT_WIDTH_);
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate,"%s",pobTran->srBRec.szCardHolder);
			inPRINT_ChineseFont(szTemplate, _PRT_NORMAL_);
                        inPRINT_ChineseFont("　　　　　　　　　 持卡人簽名", _PRT_NORMAL_);
                }
                else
                {
			inPRINT_SpaceLine(1);
                        inPRINT_ChineseFont("　　　　　　　　　 持卡人存根", _PRT_NORMAL_);
			inPRINT_ChineseFont("　　　　　　　  Card holder stub", _PRT_NORMAL_);
			inPRINT_ChineseFont("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_);
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate,"%s",pobTran->srBRec.szCardHolder);
			inPRINT_ChineseFont(szTemplate, _PRT_NORMAL_);
                }
		if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
		{
			inPRINT_ChineseFont("           重印 REPRINT", _PRT_HEIGHT_);
		}


                inPRINT_ChineseFont("            I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_);
                inPRINT_ChineseFont("        ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_);
                inPRINT_SpaceLine(8);
        }

        return (VS_SUCCESS);
}


/*
Function        :inCREDIT_PRINT_CHECK
Date&Time       :2015/8/20 上午 10:24
Describe        :
*/
int inCREDIT_PRINT_Check(TRANSACTION_OBJECT *pobTran)
{
        int     inRecordCnt;

        inRecordCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
        /* 回傳VS_ERROR(回傳 -1 )會跳出，交易筆數小於0( VS_NoRecord 會回傳 -98 )會印空白簽單 */
        /* 其餘則回傳交易筆數*/

        return (inRecordCnt);
}


/*
Function        :inCREDIT_PRINT_TOP
Date&Time       :2015/8/20 上午 10:24
Describe        :
*/
int inCREDIT_PRINT_Top(TRANSACTION_OBJECT *pobTran)
{
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        /* Get商店代號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetMerchantID(szTemplate);

        /* 列印商店代號 */
        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PAD_RIGHT_FILL_LEFT_);
        sprintf(szPrintBuf, "商店代號%s", szTemplate);
        inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

        /* Get端末機代號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetTerminalID(szTemplate);

        /* 列印端末機代號 */
        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PAD_RIGHT_FILL_LEFT_);
        sprintf(szPrintBuf, "端末機代號%s", szTemplate);
        inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

        inPRINT_ChineseFont("================================================", _PRT_NORMAL_);

        /* 列印日期 / 時間 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inPRINT_ChineseFont("日期/時間(Date/Time)", _PRT_HEIGHT_);
        memcpy(&szPrintBuf[0], &pobTran->srBRec.szDate[0], 4);
        strcat(szPrintBuf, "/");
        memcpy(&szPrintBuf[5], &pobTran->srBRec.szDate[4], 2);
        strcat(szPrintBuf, "/");
        memcpy(&szPrintBuf[8], &pobTran->srBRec.szDate[6], 2);
        strcat(szPrintBuf, "   ");
        memcpy(&szPrintBuf[13], &pobTran->srBRec.szTime[0], 2);
        strcat(szPrintBuf, ":");
        memcpy(&szPrintBuf[16], &pobTran->srBRec.szTime[2], 2);
        inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

        /* 列印交易類別 */
        if (pobTran->srBRec.inCode == _SETTLE_)
        {
                inPRINT_ChineseFont("交易類別(Trans. Type)", _PRT_HEIGHT_);
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "60 結帳 SETTLEMENT");
                inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);
        }

        /* 列印批次號碼 */
        inPRINT_ChineseFont("批次號碼(Batch No.)", _PRT_HEIGHT_);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szTemplate, "%03ld", pobTran->srBRec.lnBatchNum);
        strcpy(szPrintBuf, szTemplate);
        inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);
        /* 列印主機 */
        inPRINT_ChineseFont("主機(Host)", _PRT_HEIGHT_);
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        inGetHostLabel(szPrintBuf);
        inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmount
Date&Time       :2015/8/20 上午 10:24
Describe        :列印總金額
*/
int inCREDIT_PRINT_TotalAmount(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, int inRecordCnt)
{
        char    szPrintBuf[84 + 1], szTemplate[84 + 1], szTemplate1[84 + 1];

        if (pobTran->inRunOperationID == _OPERATION_TOTAL_REPORT_)
                inPRINT_ChineseFont("         總額報表", _PRT_DOUBLE_HEIGHT_WIDTH_);
        else
        {
                inPRINT_ChineseFont("         結帳報表", _PRT_DOUBLE_HEIGHT_WIDTH_);
        }

        inPRINT_ChineseFont("    筆數(CNT)      金額(AMOUNT)", _PRT_HEIGHT_);

        if (srAccumRec->lnTotalCount == 0)
        {
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
                sprintf(szPrintBuf, "銷售 Ｄ　%03lu   NT$", 0L);
                sprintf(szTemplate, "%ld", 0L);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate1);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
                sprintf(szPrintBuf, "退貨 Ｒ　%03lu   NT$", 0L);
                sprintf(szTemplate, "%ld", 0L);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate1);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
                /* 淨額 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
                sprintf(szPrintBuf, "淨額 Ｔ　%03lu   NT$", 0L);
                sprintf(szTemplate, "%ld", 0L);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate1);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
                inPRINT_ChineseFont("    ------------------------------------------------------------", _PRT_NORMAL_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", 0L);
                sprintf(szTemplate, "%ld", 0L);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate1);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
                inPRINT_ChineseFont("==========================================", _PRT_HEIGHT_);
        }
        else
        {
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
                sprintf(szPrintBuf, "銷售 Ｄ　%03lu   NT$", srAccumRec->lnTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llTotalSaleAmount + srAccumRec->llTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
                sprintf(szPrintBuf, "退貨 Ｒ　%03lu   NT$", srAccumRec->lnTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
                /* 淨額 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
                sprintf(szPrintBuf, "淨額 Ｔ　%03lu   NT$", srAccumRec->lnTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
                inPRINT_ChineseFont("    ------------------------------------------------------------", _PRT_NORMAL_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
                inPRINT_ChineseFont("==========================================", _PRT_HEIGHT_);
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByCard
Date&Time       :2015/8/20 上午 10:24
Describe        :依卡別列印
*/
int inCREDIT_PRINT_TotalAmountByCard(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, int inRecordCnt)
{
        char    szPrintBuf[84 + 1], szTemplate[84 + 1];

        inPRINT_ChineseFont("         卡別小計", _PRT_DOUBLE_HEIGHT_WIDTH_);

        if (srAccumRec->llUCardTotalSaleAmount != 0L || srAccumRec->llUCardTotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", "UCARD");
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnUCardTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llUCardTotalSaleAmount + srAccumRec->llUCardTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnUCardTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llUCardTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnUCardTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llUCardTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                inPRINT_ChineseFont("    ------------------------------------------------------------", _PRT_NORMAL_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnUCardTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llUCardTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                inPRINT_ChineseFont("-------------------------------------------------------------------------------------", _PRT_NORMAL_);
        }

        if (srAccumRec->llVisaTotalSaleAmount != 0L || srAccumRec->llVisaTotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_VISA_);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnVisaTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llVisaTotalSaleAmount + srAccumRec->llVisaTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnVisaTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llVisaTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnVisaTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llVisaTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                inPRINT_ChineseFont("    ------------------------------------------------------------", _PRT_NORMAL_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnVisaTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llVisaTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                inPRINT_ChineseFont("-------------------------------------------------------------------------------------", _PRT_NORMAL_);
        }

        if (srAccumRec->llMasterTotalSaleAmount != 0L || srAccumRec->llMasterTotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", "MASTER CARD");
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnMasterTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llMasterTotalSaleAmount + srAccumRec->llMasterTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnMasterTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llMasterTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnMasterTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llMasterTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                inPRINT_ChineseFont("    ------------------------------------------------------------", _PRT_NORMAL_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnMasterTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llMasterTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                inPRINT_ChineseFont("-------------------------------------------------------------------------------------", _PRT_NORMAL_);
        }

        if (srAccumRec->llJcbTotalSaleAmount != 0L || srAccumRec->llJcbTotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_JCB_);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnJcbTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llJcbTotalSaleAmount + srAccumRec->llJcbTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnJcbTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llJcbTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnJcbTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llJcbTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                inPRINT_ChineseFont("    ------------------------------------------------------------", _PRT_NORMAL_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnJcbTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llJcbTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                inPRINT_ChineseFont("-------------------------------------------------------------------------------------", _PRT_NORMAL_);
        }

        if (srAccumRec->llAmexTotalSaleAmount != 0L || srAccumRec->llAmexTotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_AMEX_);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnAmexTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llAmexTotalSaleAmount + srAccumRec->llAmexTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnAmexTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llAmexTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnAmexTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llAmexTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                inPRINT_ChineseFont("    ------------------------------------------------------------", _PRT_NORMAL_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnAmexTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llAmexTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                inPRINT_ChineseFont("-------------------------------------------------------------------------------------", _PRT_NORMAL_);
        }

        if (srAccumRec->llCupTotalSaleAmount != 0L || srAccumRec->llCupTotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_CUP_);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnCupTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llCupTotalSaleAmount + srAccumRec->llCupTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnCupTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llCupTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnCupTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llCupTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                inPRINT_ChineseFont("    ------------------------------------------------------------", _PRT_NORMAL_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnCupTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llCupTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                inPRINT_ChineseFont("-------------------------------------------------------------------------------------", _PRT_NORMAL_);
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByInstllment
Date&Time       :2015/8/20 上午 10:24
Describe        :
*/
int inCREDIT_PRINT_TotalAmountByInstllment(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, int inRecordCnt)
{
	char    szPrintBuf[84 + 1], szTemplate[84 + 1];

        if (srAccumRec->lnInstTotalCount == 0)
        {
		/* 沒有分期記錄就不印 */
        }
        else
        {
		inPRINT_ChineseFont("       分期交易總額", _PRT_DOUBLE_HEIGHT_WIDTH_);

		inPRINT_ChineseFont("    筆數(CNT)      金額(AMOUNT)", _PRT_HEIGHT_);

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 Ｄ　%03lu   NT$", srAccumRec->lnInstSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llInstSaleAmount + srAccumRec->llInstTipsAmount));
                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 Ｒ　%03lu   NT$", srAccumRec->lnInstRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llInstRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
                /* 淨額 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "淨額 Ｔ　%03lu   NT$", srAccumRec->lnInstTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llInstTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
                inPRINT_ChineseFont("    ------------------------------------------------------------", _PRT_NORMAL_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnInstTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llInstTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
                inPRINT_ChineseFont("==========================================", _PRT_HEIGHT_);
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByRedemption
Date&Time       :2015/8/20 上午 10:24
Describe        :
*/
int inCREDIT_PRINT_TotalAmountByRedemption(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, int inRecordCnt)
{
	char    szPrintBuf[84 + 1], szTemplate[84 + 1];


        if (srAccumRec->lnRedeemTotalCount == 0)
        {
		/* 沒有紅利記錄就不印 */
        }
        else
        {
		inPRINT_ChineseFont("       紅利扣抵總額", _PRT_DOUBLE_HEIGHT_WIDTH_);

		inPRINT_ChineseFont("    筆數(CNT)      金額(AMOUNT)", _PRT_HEIGHT_);

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 Ｄ　%03lu   NT$", srAccumRec->lnRedeemSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llRedeemSaleAmount + srAccumRec->llRedeemTipsAmount));
                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 Ｒ　%03lu   NT$", srAccumRec->lnRedeemRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llRedeemRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
                /* 淨額 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "淨額 Ｔ　%03lu   NT$", srAccumRec->lnRedeemTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llRedeemTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
                inPRINT_ChineseFont("    ------------------------------------------------------------", _PRT_NORMAL_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnRedeemTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llRedeemTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
		inPRINT_ChineseFont("-----------------------------------------------------------------------", _PRT_NORMAL_);

		/* 紅利扣抵總點數 */
		/* 初始化 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                /* 將NT$ ＋數字塞到szTemplate中來inpad */
                sprintf(szTemplate, " %ld", (srAccumRec->lnRedeemTotalPoint));
                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 14, VS_TRUE);

                /* 把前面的字串和數字結合起來 */
                sprintf(szPrintBuf, "紅利扣抵總點數 :%s點", szTemplate);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);

		/* 結束隔線 */
		inPRINT_SpaceLine(1);
		inPRINT_ChineseFont("==========================================", _PRT_HEIGHT_);
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_END
Date&Time       :2015/8/20 上午 10:24
Describe        :列印結尾
*/
int inCREDIT_PRINT_End(TRANSACTION_OBJECT *pobTran)
{

        inPRINT_ChineseFont("     *** 列印完成 ***", _PRT_DOUBLE_HEIGHT_WIDTH_);

        inPRINT_SpaceLine(6);

        return (VS_SUCCESS);
}

int inCREDIT_PRINT_DetailReportMiddle(TRANSACTION_OBJECT *pobTran, int inRecordCnt)
{
        char    inRetChar[1+1];                 /* catch Y or N */

        inPRINT_ChineseFont("         明細報表", _PRT_DOUBLE_HEIGHT_WIDTH_);

        inPRINT_ChineseFont("交易類別", _PRT_HEIGHT_);
	inPRINT_ChineseFont("交易金額", _PRT_HEIGHT_);
        inPRINT_ChineseFont("卡號　　　　　　　　　", _PRT_HEIGHT_);
        inPRINT_ChineseFont("交易日期　　　　　 交易時間", _PRT_HEIGHT_);
        inPRINT_ChineseFont("授權碼　　　　　　 調閱編號", _PRT_HEIGHT_);
        inPRINT_ChineseFont("序號", _PRT_HEIGHT_);

        inGetStoreIDEnable(inRetChar);
        if (inRetChar[0] == 'Y')
                inPRINT_ChineseFont("櫃號", _PRT_HEIGHT_);

        inPRINT_ChineseFont("==========================================", _PRT_HEIGHT_);

        return (VS_SUCCESS);
}

int inCREDIT_PRINT_DetailReportBottom(TRANSACTION_OBJECT *pobTran, int inRecordCnt)
{
        char            szPrintBuf[62 + 1], szTemplate1[62 + 1], szTemplate2[62 + 1];;
        char            inRetChar[1+1];/* catch Y or N */
        int             inRetVal = VS_SUCCESS;
        int             i;

	if (ginDebug == VS_TRUE)
	{
		inPRINT_ChineseFont("inCREDIT_PRINT_DetailReportBottom()_START", _PRT_HEIGHT_);
	}

	/* 開始讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);

        for (i = 0; i < inRecordCnt; i ++)
        {
                /*. 開始讀取每一筆交易記錄 .*/
                if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, i) != VS_SUCCESS)
                {
                        inRetVal = VS_ERROR;
                        break;
                }

                if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
                        continue;

                /*Trans Type*/
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
                sprintf(szPrintBuf,"%s %s", szTemplate1, szTemplate2);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);

		/* Print Amount */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate1, 0x00, sizeof(szTemplate1));

                if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
                {
                        switch (pobTran->srBRec.inCode)
                        {
                                case _SALE_:
                                case _SALE_OFFLINE_ :
                                case _PRE_AUTH_ :
				case _INST_SALE_ :
				case _REDEEM_SALE_ :
                                        sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
                                        break;
                                case _TIP_ :
                                        sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
                                        break;
                                case _REFUND_ :
                                        sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
                                        break;
                                case _ADJUST_ :
                                        sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
                                        break;
                                default :
                                        inGetHostLabel(szPrintBuf);
                                        sprintf(szTemplate1,"%s_AMT_ERR_inCode(%d)",szPrintBuf, pobTran->srBRec.inCode);
                                        break;
                        } /* End switch () */
                }
                else
                {
                        switch (pobTran->srBRec.inOrgCode)
                        {
                                case _SALE_ :
                                case _SALE_OFFLINE_ :
                                case _PRE_AUTH_ :
				case _INST_SALE_ :
				case _REDEEM_SALE_ :
                                        sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
                                        break;
                                case _TIP_ :
                                        sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
                                        break;
                                case _REFUND_ :
                                        sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
                                        break;
                                case _ADJUST_ :
                                        sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
                                        break;
                                default :
                                        inGetHostLabel(szPrintBuf);
                                        sprintf(szTemplate1, "%s_AMT_VOID_ERR_inCode(%d)",szPrintBuf, pobTran->srBRec.inCode);
                                        break;
                        } /* End switch () */
                }
                strcat(szPrintBuf, "AMT:");
		inFunc_Amount_Comma(szTemplate1, "" , ' ', _SIGNED_NONE_, 13, VS_FALSE);
                strcat(szPrintBuf, szTemplate1);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);

                /*Pan*/
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                strcpy(szPrintBuf, pobTran->srBRec.szPAN);
		inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 19, _PAD_LEFT_FILL_RIGHT_);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);

                /*Trans Date Time*/
                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(&szTemplate1[0], '/', 10);
                memcpy(&szTemplate1[0], &(pobTran->srBRec.szDate[0]),4);
                memcpy(&szTemplate1[5], &(pobTran->srBRec.szDate[4]),2);
                memcpy(&szTemplate1[8], &(pobTran->srBRec.szDate[6]),2);
                szTemplate1[10] = _NULL_CH_;
                strcpy(szPrintBuf, "DATE:");
                strcat(szPrintBuf, szTemplate1);
                inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 18, _PAD_LEFT_FILL_RIGHT_);
                strcat(szPrintBuf, "TIME:");
                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                memset(&szTemplate1[0], ':', 8);
                memcpy(&szTemplate1[0], &(pobTran->srBRec.szTime[0]), 2);
                memcpy(&szTemplate1[3], &(pobTran->srBRec.szTime[2]), 2);
                memcpy(&szTemplate1[6], &(pobTran->srBRec.szTime[4]), 2);
                szTemplate1[8] = 0x00;
                strcat(szPrintBuf, szTemplate1);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);

                /* Approved No. */ /* 授權碼 = Batch Number + Invoice Number */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                strcpy(szPrintBuf, "APPR:");
                strcat(szPrintBuf, pobTran->srBRec.szAuthCode);
                inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 18, _PAD_LEFT_FILL_RIGHT_ );

                /*Invoice Number*/
                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                sprintf(szTemplate1, "INV:%06ld", pobTran->srBRec.lnOrgInvNum);
                strcat(szPrintBuf, szTemplate1);
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );

                /* RRN */ /*invoice number + RefNo */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "REF:%06ld", atol(pobTran->srBRec.szRefNo));
                inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);

                /* Store ID */
                memset(inRetChar, 0x00, sizeof(inRetChar));
                if (inGetStoreIDEnable(inRetChar) != VS_SUCCESS )
                {
                        /*取得StoreIDEnable失敗，回傳VS_Error*/
                        return (VS_ERROR);
                }
                else if (inRetChar[0] == 'Y')
                {
                        /*開啟櫃號功能*/
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "STORE ID:%s", pobTran->srBRec.szStoreID);
                        inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
                }
                else
                {
                       /*沒開啟櫃號功能，則不印櫃號*/
                }

                inPRINT_ChineseFont("-------------------------------------------------------------------------------------", _PRT_NORMAL_);
        } /* End for () .... */

	/* 結束讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);

	if (ginDebug == VS_TRUE)
	{
		inPRINT_ChineseFont("inCREDIT_PRINT_DetailReportBottom()_END", _PRT_HEIGHT_);
	}

        return (inRetVal);
}

/*
Function        :inCREDIT_Func7PrintParamTerm
Date&Time       :2015/12/22 下午 4:22
Describe        :功能7列印參數
*/
int inCREDIT_Func7PrintParamTerm(TRANSACTION_OBJECT *pobTran)
{
        if (inCREDIT_PRINT_ParamLOGO(pobTran) != VS_SUCCESS)
                return (VS_ERROR);

	if (inCREDIT_PRINT_ParamTermInformation(pobTran) != VS_SUCCESS)
		return (VS_ERROR);

	if (inCREDIT_PRINT_ParamHostDetailParam(pobTran) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 列印【卡別參數檔】【特殊卡別參數檔】【非接觸式】  */
	if (inCREDIT_PRINT_ParamCardType(pobTran) != VS_SUCCESS)
    		return (VS_ERROR);

	/* 列印【管理號碼】 */
	if (inCREDIT_PRINT_ParamManageNum(pobTran) != VS_SUCCESS)
		return (VS_ERROR);

	/* 列印【產品代碼】 */
	if (inCREDIT_PRINT_ProductCode(pobTran) != VS_SUCCESS)
		return (VS_ERROR);

	/* 列印【EMV CA Public Key】 */
	if (inCREDIT_PRINT_CAPublicKey(pobTran) != VS_SUCCESS)
		return (VS_ERROR);

	/* 列印【系統參數檔】【共用參數檔】  */
	if (inCREDIT_PRINT_ParamSystemConfig(pobTran) != VS_SUCCESS)
    		return (VS_ERROR);

	if (inCREDIT_PRINT_ParamLOGO_END(pobTran) != VS_SUCCESS)
    		return (VS_ERROR);

        return (VS_SUCCESS);
}

int inCREDIT_PRINT_ParamLOGO(TRANSACTION_OBJECT *pobTran)
{
        int		i;
	char		szPrintBuffer[100 + 1];
	char		szTemplate[64 + 1];
        char		szFlag[10 + 1];
        char		szTxnType[20 + 1];
        RTC_NEXSYS		srRTC; /* Date & Time */

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);/* 帳單列印中 */

        /* 列印日期 / 時間 */
        inPRINT_ChineseFont("", _PRT_HEIGHT_);
	memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        strcpy(szPrintBuffer, "列印時間 ： ");
        memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "20%02d/%02d/%02d  ", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
        strcat(szPrintBuffer, szTemplate);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "%02d:%02d:%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
        strcat(szPrintBuffer, szTemplate);
        inPRINT_ChineseFont(szPrintBuffer, _PRT_DOUBLE_HEIGHT_WIDTH_);

        /* PRINT NCCC LOGO */
        inPRINT_ChineseFont("BANK LOGO ：", _PRT_HEIGHT_);

        if (inPRINT_PutGraphic((unsigned char *)_BANK_LOGO_) == VS_ERROR)
        {
                return (VS_ERROR);
        }

        /* PRINT MERCHANT LOGO */
        memset(szFlag, 0x00, sizeof(szFlag));
        inGetPrtMerchantLogo(szFlag);

        if (szFlag[0] == 'Y')
        {
                inPRINT_ChineseFont("LOGO ：", _PRT_HEIGHT_);

                if (inPRINT_PutGraphic((unsigned char *)_MERCHANT_LOGO_) == VS_ERROR)
                {
                        return (VS_ERROR);
                }
        }
        else
        {
                inPRINT_ChineseFont("LOGO ： 無", _PRT_HEIGHT_);
        }

        /* PRINT MERCHANT NAME */
        memset(szFlag, 0x00, sizeof(szFlag));
        inGetPrtMerchantName(szFlag);

        if (szFlag[0] == 'Y')
        {
                inPRINT_ChineseFont("表頭 ：", _PRT_HEIGHT_);

                if (inPRINT_PutGraphic((unsigned char *)_NAME_LOGO_) == VS_ERROR)
                {
                        return (VS_ERROR);
                }
        }
        else
        {
                inPRINT_ChineseFont("表頭 ： 無", _PRT_HEIGHT_);
        }

        /* PRINT SLOGAN */
        memset(szFlag, 0x00, sizeof(szFlag));
        inGetPrtSlogan(szFlag);

        if (szFlag[0] == 'Y')
        {
                inPRINT_ChineseFont("SLOGAN ：", _PRT_HEIGHT_);

                if (inPRINT_PutGraphic((unsigned char *)_SLOGAN_LOGO_) == VS_ERROR)
                {
                        return (VS_ERROR);
                }
        }
        else
        {
                inPRINT_ChineseFont("SLOGAN ： 無", _PRT_HEIGHT_);
        }

        /* PRINT 分期付款條文 */
        memset(szFlag, 0x00, sizeof(szFlag));
        memset(szTxnType, 0x00, sizeof(szTxnType));
	if (inLoadHDTRec(0) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" CREDIT PRINT Param LOGO Load HDT 0 *Error* Line[%d]", __LINE__);
	}
        inGetTransFunc(szTxnType);

        //if (szTxnType[7] == 0x59)
	if (szTxnType[8] == 0x31) /* 修改TMS交易開關位置 20190211 [SAM] */
        {
                inPRINT_ChineseFont("分期付款條文 ：", _PRT_HEIGHT_);

                if (inPRINT_PutGraphic((unsigned char *)_LEGAL_LOGO_) == VS_ERROR)
                {
                        return (VS_ERROR);
                }
        }
        else
        {
                inPRINT_ChineseFont("分期付款條文 ： 無", _PRT_HEIGHT_);
        }

        /* PRINT 商店需求提示語 */
        memset(szFlag, 0x00, sizeof(szFlag));
        inGetPrtNotice(szFlag);

        if (szFlag[0] == 'Y')
        {
                inPRINT_ChineseFont("商店需求提示語 ：", _PRT_HEIGHT_);

                if (inPRINT_PutGraphic((unsigned char *)_NOTICE_LOGO_) == VS_ERROR)
                {
                        return (VS_ERROR);
                }
        }
        else
        {
                inPRINT_ChineseFont("商店需求提示語 ： 無", _PRT_HEIGHT_);
        }

        /* CUP 交易警語 */
        memset(szFlag, 0x00, sizeof(szFlag));

        for (i = 0 ;; i ++)
        {
                if (inLoadCDTRec(i) < 0)
                        break;

                inGetCardLabel(szFlag);

                if (!memcmp(szFlag, _CARD_TYPE_CUP_, 3))
                        break;
                else
                    memset(szFlag, 0x00, sizeof(szFlag));
        }

        if (!memcmp(szFlag, _CARD_TYPE_CUP_, 3))
        {
                inPRINT_ChineseFont("CUP交易警語 ：", _PRT_HEIGHT_);

                if (inPRINT_PutGraphic((unsigned char *)_CUP_LEGAL_LOGO_) == VS_ERROR)
                {
                        return (VS_ERROR);
                }
        }
        else
        {
                inPRINT_ChineseFont("CUP交易警語 ： 無", _PRT_HEIGHT_);
        }

        return (VS_SUCCESS);
}

int inCREDIT_PRINT_ParamTermInformation(TRANSACTION_OBJECT *pobTran)
{
	char	szPrintBuffer[100 + 1];
	char	szTemplate[64 + 1];

        inLoadTMSCPTRec(0);
        
	/* Terminal AP Name */
	inPRINT_ChineseFont("", _PRT_HEIGHT_);
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
	inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);
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
	inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);
	/* 端末機通訊設定 */
        inPRINT_ChineseFont("", _PRT_HEIGHT_);
        inPRINT_ChineseFont("端末機通訊設定", _PRT_HEIGHT_);
	/* EDC IP */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetTermIPAddress(szTemplate);
	sprintf(szPrintBuffer, "EDC IP = %s", szTemplate);
	inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);
	/* SUBNET MASK */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetTermMASKAddress(szTemplate);
	sprintf(szPrintBuffer, "SUBNET MASK = %s", szTemplate);
	inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);
	/* DF GATEWAY */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetTermGetewayAddress(szTemplate);
	sprintf(szPrintBuffer, "DF GATEWAY = %s", szTemplate);
	inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);
	/* TMS IP */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetTMSIPAddress(szTemplate);
	sprintf(szPrintBuffer, "TMS IP = %s", szTemplate);
	inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);
	/* PORT NO */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetTMSPortNum(szTemplate);
	sprintf(szPrintBuffer, "Port No = %s", szTemplate);
	inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);
	/* TMS TEL */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetTMSPhoneNumber(szTemplate);
	sprintf(szPrintBuffer,"TMS TEL = %s", szTemplate);
	inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);
        /* TSP IP，客製化參數為Costco時列印，20230111 Miyano */
        if (vbCheckCostcoCustom(Costco_New))
        {
            memset(szTemplate, 0x00, sizeof(szTemplate));
            memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
            inGetTSP_IP(szTemplate);
            inDISP_LogPrintf("szTemplate TSP_IP = %s", szTemplate);
            sprintf(szPrintBuffer, "TSP IP = %s", szTemplate);
            inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);
            /* PORT NO */
            memset(szTemplate, 0x00, sizeof(szTemplate));
            memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
            inGetTSP_Port(szTemplate);
            inDISP_LogPrintf("szTemplate TSP_Port = %s", szTemplate);
            sprintf(szPrintBuffer, "Port No = %s", szTemplate);
            inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);
        }

        inPRINT_ChineseFont("", _PRT_HEIGHT_);
        return (VS_SUCCESS);
}

int inCREDIT_PRINT_ParamHostDetailParam(TRANSACTION_OBJECT *pobTran)
{
	char	szPrintBuffer[100 + 1];
	char	szTxnType[20 + 1];
        char	szTemplate[64 + 1];
	int	i;
	unsigned char ucTrunsFuncCondition = 0x31; /* 新增一個條件，用來判斷功能 20190211 [SAM] */

	for (i = 0 ;; i ++)
	{
		if (inLoadHDTRec(i) < 0) /* 主機參數檔 */
		{
			inDISP_LogPrintfWithFlag(" CREDIT PRINT Param Host Detial Load HDT[%d] *Error* Line[%d]", i,  __LINE__);
			break;
		}

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetHostEnable(szTemplate);

                /* 主機功能開關關閉不列印 */
                if (!memcmp(szTemplate, "N", 1))
                        continue;

                if (inLoadCPTRec(i) < 0)
                        break;

		/* 主機資料 */
                memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                inGetHostLabel(szPrintBuffer);
                inPRINT_ChineseFont(szPrintBuffer, _PRT_DOUBLE_HEIGHT_WIDTH_);

                /* Merchant ID */
	 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetMerchantID(szTemplate);
	 	sprintf(szPrintBuffer, "MID = %s", szTemplate);
	 	inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);

                /* Terminal ID */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetTerminalID(szTemplate);
	 	sprintf(szPrintBuffer, "TID = %s", szTemplate);
	 	inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);

                /* 第一授權撥接電話 */
                memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetHostTelPrimary(szTemplate);
	 	sprintf(szPrintBuffer, "TEL #1 = %s", szTemplate);
	 	inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);

                /* 第二授權撥接電話 */
	 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetHostTelSecond(szTemplate);
	 	sprintf(szPrintBuffer, "TEL #2 = %s", szTemplate);
	 	inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);

                /* Call Bank 撥接電話 */
	 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetReferralTel(szTemplate);
	 	sprintf(szPrintBuffer, "TEL #3 = %s", szTemplate);
	 	inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);

                /* 第一授權主機 IP Address  */
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetHostIPPrimary(szTemplate);
	 	sprintf(szPrintBuffer, "HOST IP = %s", szTemplate);
	 	inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);

                /* 第一授權主機 Port No. */
	 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetHostPortNoPrimary(szTemplate);
	 	sprintf(szPrintBuffer, "PORT NO. = %s", szTemplate);
	 	inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);

                /* TPDU */
	 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetTPDU(szTemplate);
	 	sprintf(szPrintBuffer, "TPDU = %s", szTemplate);
	 	inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);

                /* NII */
	 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetNII(szTemplate);
	 	sprintf(szPrintBuffer, "NII = %s", szTemplate);
	 	inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);

	 	/* 交易開關 */
		memset(szTxnType, 0x00, sizeof(szTxnType));
		inGetTransFunc(szTxnType);
		inPRINT_ChineseFont("　　　　　　　　　　　開　　關", _PRT_HEIGHT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetHostLabel(szTemplate);

		if (!memcmp(szTemplate, "HG      ", 8))
		{
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			if (szTxnType[0] == ucTrunsFuncCondition)
				inPRINT_ChineseFont("紅利積點　　　　　　　● 　　　", _PRT_HEIGHT_);
			else
				inPRINT_ChineseFont("紅利積點　　　　　　　　　　● ", _PRT_HEIGHT_);

			if (szTxnType[1] == ucTrunsFuncCondition)
				inPRINT_ChineseFont("點數扣抵　　　　　　　● 　　　", _PRT_HEIGHT_);
			else
				inPRINT_ChineseFont("點數扣抵　　　　　　　　　　● ", _PRT_HEIGHT_);

			if (szTxnType[2] == ucTrunsFuncCondition)
				inPRINT_ChineseFont("加價購　　　　　　　　● 　　　", _PRT_HEIGHT_);
			else
				inPRINT_ChineseFont("加價購　　　　　　　　　　　● ", _PRT_HEIGHT_);

			if (szTxnType[3] == ucTrunsFuncCondition)
				inPRINT_ChineseFont("點數兌換　　　　　　　● 　　　", _PRT_HEIGHT_);
			else
				inPRINT_ChineseFont("點數兌換　　　　　　　　　　● ", _PRT_HEIGHT_);

			if (szTxnType[4] == ucTrunsFuncCondition)
				inPRINT_ChineseFont("點數查詢　　　　　　　● 　　　", _PRT_HEIGHT_);
			else
				inPRINT_ChineseFont("點數查詢　　　　　　　　　　● ", _PRT_HEIGHT_);

			if (szTxnType[5] == ucTrunsFuncCondition)
				inPRINT_ChineseFont("回饋退貨　　　　　　　● 　　　", _PRT_HEIGHT_);
			else
				inPRINT_ChineseFont("回饋退貨　　　　　　　　　　● ", _PRT_HEIGHT_);

			if (szTxnType[6] == ucTrunsFuncCondition)
				inPRINT_ChineseFont("扣抵退貨　　　　　　　● 　　　", _PRT_HEIGHT_);
			else
				inPRINT_ChineseFont("扣抵退貨　　　　　　　　　　● ", _PRT_HEIGHT_);

			if (szTxnType[7] == ucTrunsFuncCondition)
				inPRINT_ChineseFont("紅利積點人工輸入卡號　● 　　　", _PRT_HEIGHT_);
			else
				inPRINT_ChineseFont("紅利積點人工輸入卡號　　　　● ", _PRT_HEIGHT_);

			if (szTxnType[8] == ucTrunsFuncCondition)
				inPRINT_ChineseFont("紅利積點列印簽單　　　● 　　　", _PRT_HEIGHT_);
			else
				inPRINT_ChineseFont("紅利積點列印簽單　　　　　　● ", _PRT_HEIGHT_);
		}
		else
		{
			if (szTxnType[0] == ucTrunsFuncCondition)
				inPRINT_ChineseFont("銷售　　　　　　　　　● 　　　", _PRT_HEIGHT_);
			else
				inPRINT_ChineseFont("銷售　　　　　　　　　　　　● ", _PRT_HEIGHT_);

			if (szTxnType[1] == ucTrunsFuncCondition)
				inPRINT_ChineseFont("取消　　　　　　　　　● 　　　", _PRT_HEIGHT_);
			else
				inPRINT_ChineseFont("取消　　　　　　　　　　　　● ", _PRT_HEIGHT_);

			if (szTxnType[2] == ucTrunsFuncCondition)
				inPRINT_ChineseFont("結帳　　　　　　　　　● 　　　", _PRT_HEIGHT_);
			else
				inPRINT_ChineseFont("結帳　　　　　　　　　　　　● ", _PRT_HEIGHT_);

			if (szTxnType[3] == ucTrunsFuncCondition)
				inPRINT_ChineseFont("退貨　　　　　　　　　● 　　　", _PRT_HEIGHT_);
			else
				inPRINT_ChineseFont("退貨　　　　　　　　　　　　● ", _PRT_HEIGHT_);

			if (!memcmp(szTemplate, "NCCC    ", 8))
			{
				if (szTxnType[4] != 0x30)
					inPRINT_ChineseFont("補登　　　　　　　　　● 　　　", _PRT_HEIGHT_);
				else
					inPRINT_ChineseFont("補登　　　　　　　　　　　　● ", _PRT_HEIGHT_);
			}
			else
			{
				if (szTxnType[4] == ucTrunsFuncCondition)
					inPRINT_ChineseFont("補登　　　　　　　　　● 　　　", _PRT_HEIGHT_);
				else
					inPRINT_ChineseFont("補登　　　　　　　　　　　　● ", _PRT_HEIGHT_);
			}

			if (szTxnType[5] == ucTrunsFuncCondition)
				inPRINT_ChineseFont("預先授權　　　　　　　● 　　　", _PRT_HEIGHT_);
			else
				inPRINT_ChineseFont("預先授權　　　　　　　　　　● ", _PRT_HEIGHT_);

			if (szTxnType[6] == ucTrunsFuncCondition)
				inPRINT_ChineseFont("小費　　　　　　　　　● 　　　", _PRT_HEIGHT_);
			else
				inPRINT_ChineseFont("小費　　　　　　　　　　　　● ", _PRT_HEIGHT_);

			if (!memcmp(szTemplate, "NCCC    ", 8))
			{
				if (szTxnType[7] == ucTrunsFuncCondition)
					inPRINT_ChineseFont("分期付款　　　　　　　● 　　　", _PRT_HEIGHT_);
				else
					inPRINT_ChineseFont("分期付款　　　　　　　　　　● ", _PRT_HEIGHT_);

				if (szTxnType[8] == ucTrunsFuncCondition)
					inPRINT_ChineseFont("紅利扣抵　　　　　　　● 　　　", _PRT_HEIGHT_);
				else
					inPRINT_ChineseFont("紅利扣抵　　　　　　　　　　● ", _PRT_HEIGHT_);

				if (szTxnType[9] == ucTrunsFuncCondition)
					inPRINT_ChineseFont("分期調帳　　　　　　　● 　　　", _PRT_HEIGHT_);
				else
					inPRINT_ChineseFont("分期調帳　　　　　　　　　　● ", _PRT_HEIGHT_);

				if (szTxnType[10] == ucTrunsFuncCondition)
					inPRINT_ChineseFont("紅利調帳　　　　　　　● 　　　", _PRT_HEIGHT_);
				else
					inPRINT_ChineseFont("紅利調帳　　　　　　　　　　● ", _PRT_HEIGHT_);

				if (szTxnType[11] == ucTrunsFuncCondition)
					inPRINT_ChineseFont("郵購(MO/TO)      ● 　　　", _PRT_HEIGHT_);
				else
					inPRINT_ChineseFont("郵購(MO/TO)      　　　● ", _PRT_HEIGHT_);
			}
		}

		if (!memcmp(szTemplate, "HG      ", 8))
		{
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetManualKeyin(szTemplate);

			if (!memcmp(&szTemplate[0], "Y", 1))
				inPRINT_ChineseFont("人工輸入卡號　　　　　● 　　　", _PRT_HEIGHT_);
			else
				inPRINT_ChineseFont("人工輸入卡號　　　　　　　　● ", _PRT_HEIGHT_);

                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetTipPercent(szTemplate);
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		 	sprintf(szPrintBuffer, "小費限額 = %s", szTemplate);
		 	strcat(szPrintBuffer, "%");
		 	inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);
		}

                inPRINT_ChineseFont("", _PRT_NORMAL_);
	}

	inPRINT_ChineseFont("", _PRT_NORMAL_);
	return (VS_SUCCESS);
}

int inCREDIT_PRINT_ParamCardType(TRANSACTION_OBJECT *pobTran)
{
	char	szPrintBuffer[100 + 1];
        char	szTemplate[64 + 1];
        char    szLowBinRange[11 + 1], szHighBinRange[11 + 1], szMinPANLength[2 + 1], szMaxPANLength[2 + 1];
	int	i, inSpace = 20;

	inPRINT_ChineseFont("卡別參數", _PRT_HEIGHT_);
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

		inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);

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
		inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);
	}

	inPRINT_ChineseFont(" ", _PRT_NORMAL_);

	inPRINT_ChineseFont("非接觸卡別", _PRT_HEIGHT_);
                inPRINT_ChineseFont("      　　　　　　　　　　　開　　關", _PRT_HEIGHT_);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetVISAPaywaveEnable(szTemplate);

	if (!memcmp(&szTemplate[0], "Y", 1))
		inPRINT_ChineseFont("Visa Paywave     　　　　　● 　　　", _PRT_HEIGHT_);
	else
		inPRINT_ChineseFont("Visa Paywave     　　　　　　　　●", _PRT_HEIGHT_);

        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetJCBJspeedyEnable(szTemplate);

        if (!memcmp(&szTemplate[0], "Y", 1))
		inPRINT_ChineseFont("JCB Jspeedy        　　　　　● 　　　", _PRT_HEIGHT_);
	else
		inPRINT_ChineseFont("JCB Jspeedy        　　　　　　　　●", _PRT_HEIGHT_);

        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetMCPaypassEnable(szTemplate);

	if (!memcmp(&szTemplate[0], "Y", 1))
		inPRINT_ChineseFont("M/C Paypass      　　　　　● 　　　", _PRT_HEIGHT_);
	else
		inPRINT_ChineseFont("M/C Paypass      　　　　　　　　●", _PRT_HEIGHT_);

        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetSmartPayContactlessEnable(szTemplate);

	if (!memcmp(&szTemplate[0], "Y", 1))
		inPRINT_ChineseFont("FISC SmartPay    　　　　　● 　　　", _PRT_HEIGHT_);
	else
		inPRINT_ChineseFont("FISC SmartPay    　　　　　　　　●", _PRT_HEIGHT_);

        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetCUPContactlessEnable(szTemplate);

	if (!memcmp(&szTemplate[0], "Y", 1))
		inPRINT_ChineseFont("CUP QuickPass   　　　　　● 　　　", _PRT_HEIGHT_);
	else
		inPRINT_ChineseFont("CUP QuickPass   　　　　　　　　●", _PRT_HEIGHT_);

	inPRINT_ChineseFont(" ", _PRT_NORMAL_);

        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetSpecialCardRangeEnable(szTemplate);

	if (!memcmp(&szTemplate[0], "N", 1))
		inPRINT_ChineseFont("特殊卡別參數 : 無", _PRT_HEIGHT_);
	else
	{
		inPRINT_ChineseFont("特殊卡別參數", _PRT_HEIGHT_);

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
			inPRINT_ChineseFont(szPrintBuffer, _PRT_NORMAL_);

                        /* 卡號範圍 */
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetSCDTLowBinRange(szTemplate);
                        sprintf(szPrintBuffer, "   %s ~ ", szTemplate);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetSCDTHighBinRange(szTemplate);
                        strcat(szPrintBuffer, szTemplate);
			inPRINT_ChineseFont(szPrintBuffer, _PRT_NORMAL_);

                        /* 活動代碼 活動限額 */
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetCampaignNumber(szTemplate);
                        sprintf(szPrintBuffer, "   Campaign = %s  ", szTemplate);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetCampaignAmount(szTemplate);
                        strcat(szPrintBuffer, szTemplate);
			inPRINT_ChineseFont(szPrintBuffer, _PRT_NORMAL_);
		}
	}

	inPRINT_ChineseFont(" ", _PRT_NORMAL_);
	return (VS_SUCCESS);
}

int inCREDIT_PRINT_ParamManageNum(TRANSACTION_OBJECT *pobTran)
{
        char szCheckEnable[2 + 1];

        inLoadPWDRec(0);
	inPRINT_ChineseFont("管理號碼", _PRT_HEIGHT_);
	inPRINT_ChineseFont("　　　　　　　　　　　　開　　關", _PRT_HEIGHT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetRebootPwdEnale(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_ChineseFont("開機管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_);
	else
		inPRINT_ChineseFont("開機管理號碼　　　　　　　　　● ", _PRT_HEIGHT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetSalePwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_ChineseFont("銷售管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_);
	else
		inPRINT_ChineseFont("銷售管理號碼　　　　　　　　　● ", _PRT_HEIGHT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetPreauthPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_ChineseFont("預先授權管理號碼　　　　●　　　 ", _PRT_HEIGHT_);
	else
		inPRINT_ChineseFont("預先授權管理號碼　　　　　　　● ", _PRT_HEIGHT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetInstallmentPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_ChineseFont("分期管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_);
	else
		inPRINT_ChineseFont("分期管理號碼　　　　　　　　　● ", _PRT_HEIGHT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetRedeemPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_ChineseFont("紅利管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_);
	else
		inPRINT_ChineseFont("紅利管理號碼　　　　　　　　　● ", _PRT_HEIGHT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetMailOrderPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_ChineseFont("郵購管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_);
	else
		inPRINT_ChineseFont("郵購管理號碼　　　　　　　　　● ", _PRT_HEIGHT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetOfflinePwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_ChineseFont("補登管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_);
	else
		inPRINT_ChineseFont("補登管理號碼　　　　　　　　　● ", _PRT_HEIGHT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetInstallmentAdjustPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_ChineseFont("分期調帳管理號碼　　　　●　　　 ", _PRT_HEIGHT_);
	else
		inPRINT_ChineseFont("分期調帳管理號碼　　　　　　　● ", _PRT_HEIGHT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetRedeemAdjustPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_ChineseFont("紅利調帳管理號碼　　　　●　　　 ", _PRT_HEIGHT_);
	else
		inPRINT_ChineseFont("紅利調帳管理號碼　　　　　　　● ", _PRT_HEIGHT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetVoidPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_ChineseFont("取消管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_);
	else
		inPRINT_ChineseFont("取消管理號碼　　　　　　　　　● ", _PRT_HEIGHT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetSettlementPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_ChineseFont("結帳管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_);
	else
		inPRINT_ChineseFont("結帳管理號碼　　　　　　　　　● ", _PRT_HEIGHT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetRefundPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_ChineseFont("退貨管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_);
	else
		inPRINT_ChineseFont("退貨管理號碼　　　　　　　　　● ", _PRT_HEIGHT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetHGRefundPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_ChineseFont("ＨＧ退貨管理號碼　　　　●　　　 ", _PRT_HEIGHT_);
	else
		inPRINT_ChineseFont("ＨＧ退貨管理號碼　　　　　　　● ", _PRT_HEIGHT_);

	inPRINT_ChineseFont("", _PRT_NORMAL_);
	return (VS_SUCCESS);
}

int inCREDIT_PRINT_ProductCode(TRANSACTION_OBJECT *pobTran)
{
	int	i;
	char	szPrintBuf[100 + 1];
        char    szTemplate[42 + 1];

	if (inLoadCFGTRec(0) < 0)
	{
		return (VS_ERROR);
	}

        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetProductCodeEnable(szTemplate);

	if (!memcmp(&szTemplate[0], "N", 1))
                inPRINT_ChineseFont("產品代碼 : 無 ", _PRT_HEIGHT_);
	else
	{
                inPRINT_ChineseFont("產品代碼", _PRT_HEIGHT_);

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
                        inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
		}
	}

	inPRINT_ChineseFont("", _PRT_NORMAL_);
	return (VS_SUCCESS);
}

int inCREDIT_PRINT_CAPublicKey(TRANSACTION_OBJECT *pobTran)
{
	char	szPrintBuffer[100 + 1];
        char    szEMVCAPKIndex[2 + 1], szApplicationId[10 + 1], szCAPKIndex[2 + 1];
	int	i;

	inPRINT_ChineseFont("EMV CAPK", _PRT_HEIGHT_);

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
                inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);
	}

        inPRINT_ChineseFont(" ", _PRT_NORMAL_);
	return (VS_SUCCESS);
}

int inCREDIT_PRINT_ParamSystemConfig(TRANSACTION_OBJECT *pobTran)
{
	char	szPrintBuffer[100 + 1];
	char    szTemplate[42 + 1];
	char    szTemplate2[42 + 1];

	while (1)
	{
		inLoadTMSSCTRec(0);
		inPRINT_ChineseFont("系統參數", _PRT_HEIGHT_);
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSInquireMode(szTemplate);
		sprintf(szPrintBuffer, "TMS詢問模式 = %s", szTemplate);
		inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);

//		if (!memcmp(&szTemplate[0], _TMS_INQUIRE_02_SCHEDHULE_SETTLE_, 1))
//		{
//			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
//                        memset(szTemplate, 0x00, sizeof(szTemplate));
//                        inGetTMSInquireStartDate(szTemplate);
//			sprintf(szPrintBuffer, "TMS詢問起始日期 = %s", szTemplate);
//			inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);
//			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
//                        memset(szTemplate, 0x00, sizeof(szTemplate));
//                        inGetTMSInquireTime(szTemplate);
//			sprintf(szPrintBuffer, "TMS詢問起始時間 = %s", szTemplate);
//			inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);
//			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
//                        memset(szTemplate, 0x00, sizeof(szTemplate));
//                        inGetTMSInquireGap(szTemplate);
//			sprintf(szPrintBuffer, "TMS詢問間隔天數 = %s", szTemplate);
//			inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);
//		}

		inPRINT_ChineseFont("", _PRT_HEIGHT_);
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

		inPRINT_ChineseFont("共用參數", _PRT_HEIGHT_);
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetCustomIndicator(szTemplate);
		sprintf(szPrintBuffer, "1.客製化專屬參數 = %s", szTemplate);
		inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetNCCCFESMode(szTemplate);

		if (!memcmp(szTemplate, "01", 2))
			inPRINT_ChineseFont("2.NCCC FES 模式 = 集訊機", _PRT_HEIGHT_);
		else if (!memcmp(szTemplate, "02", 2))
			inPRINT_ChineseFont("2.NCCC FES 模式 = RFES", _PRT_HEIGHT_);
		else if (!memcmp(szTemplate, "03", 2))
			inPRINT_ChineseFont("2.NCCC FES 模式 = MFES", _PRT_HEIGHT_);
		else if (!memcmp(szTemplate, "04", 2))
			inPRINT_ChineseFont("2.NCCC FES 模式 = MPAS", _PRT_HEIGHT_);
		else if (!memcmp(szTemplate, "05", 2))
			inPRINT_ChineseFont("2.NCCC FES 模式 = ATS", _PRT_HEIGHT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetCommMode(szTemplate);

		if (memcmp(szTemplate, _COMM_MODEM_MODE_, 1) == 0)
			inPRINT_ChineseFont("3.通訊模式 = DialUp", _PRT_HEIGHT_);
		else if (memcmp(szTemplate, _COMM_ETHERNET_MODE_, 1) == 0)
			inPRINT_ChineseFont("3.通訊模式 = TCP/IP", _PRT_HEIGHT_);
		else if (memcmp(szTemplate, _COMM_GPRS_MODE_, 1) == 0)
			inPRINT_ChineseFont("3.通訊模式 = GPRS", _PRT_HEIGHT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetDialBackupEnable(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_ChineseFont("4.撥接備援 = On", _PRT_HEIGHT_);
		else
			inPRINT_ChineseFont("4.撥接備援 = Off", _PRT_HEIGHT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetEncryptMode(szTemplate);

		if (memcmp(szTemplate, _NCCC_ENCRYPTION_NONE_, 1) == 0)
			inPRINT_ChineseFont("5.加密模式 = 不加密", _PRT_HEIGHT_);
		else if (memcmp(szTemplate, _NCCC_ENCRYPTION_TSAM_, 1) == 0)
			inPRINT_ChineseFont("5.加密模式 = tSAM加密", _PRT_HEIGHT_);
		else if (memcmp(szTemplate, _NCCC_ENCRYPTION_SOFTWARE_, 1) == 0)
			inPRINT_ChineseFont("5.加密模式 = 軟體加密", _PRT_HEIGHT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetSplitTransCheckEnable(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_ChineseFont("6.不可連續刷卡 = On", _PRT_HEIGHT_);
		else
			inPRINT_ChineseFont("6.不可連續刷卡 = Off", _PRT_HEIGHT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetCityName(szTemplate);
		sprintf(szPrintBuffer, "7.城市別 = %s", szTemplate);
		inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);

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

                inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetECREnable(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_ChineseFont("9.ECR連線= On", _PRT_HEIGHT_);
		else
			inPRINT_ChineseFont("9.ECR連線 = Off", _PRT_HEIGHT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetECRCardNoTruncateEnable(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_ChineseFont("10.ECR卡號遮掩 = On", _PRT_HEIGHT_);
		else
			inPRINT_ChineseFont("10.ECR卡號遮掩 = Off", _PRT_HEIGHT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetECRExpDateReturnEnable(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_ChineseFont("11.ECR卡片有效期回傳 = On", _PRT_HEIGHT_);
		else
			inPRINT_ChineseFont("11.ECR卡片有效期回傳 = Off", _PRT_HEIGHT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetProductCodeEnable(szTemplate);

                if (!memcmp(szTemplate, "Y", 1))
			inPRINT_ChineseFont("12.列印產品代碼 = On", _PRT_HEIGHT_);
		else
			inPRINT_ChineseFont("12.列印產品代碼 = Off", _PRT_HEIGHT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetPrtSlogan(szTemplate);

                if (!memcmp(szTemplate, "Y", 1))
			inPRINT_ChineseFont("13.列印商店Slogan = On", _PRT_HEIGHT_);
		else
			inPRINT_ChineseFont("13.列印商店Slogan = Off", _PRT_HEIGHT_);

		if (!memcmp(szTemplate, "Y", 1))
		{
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szTemplate2, 0x00, sizeof(szTemplate2));
                        inGetSloganStartDate(szTemplate);
                        inGetSloganEndDate(szTemplate2);
			sprintf(szPrintBuffer, "14.Slogan起迄日 = %s ~ %s", szTemplate, szTemplate2);
			inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);

			memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetSloganPrtPosition(szTemplate);

			if (atoi(szTemplate) == _NCCC_SLOGAN_PRINT_DOWN_)
				inPRINT_ChineseFont("15.Slogan列印位置 = 簽單下方", _PRT_HEIGHT_);
			else if (atoi(szTemplate) == _NCCC_SLOGAN_PRINT_UP_)
				inPRINT_ChineseFont("15.Slogan列印位置 = 簽單上方", _PRT_HEIGHT_);
		}
		else
		{
			inPRINT_ChineseFont("14.Slogan起迄日 =", _PRT_HEIGHT_);
			inPRINT_ChineseFont("15.Slogan列印位置 =", _PRT_HEIGHT_);
		}

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetPrtMode(szTemplate);
		sprintf(szPrintBuffer, "16.簽單列印張數 = %s", szTemplate);
		inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetCUPRefundLimit(szTemplate);
		sprintf(szPrintBuffer, "17.CUP退貨限額 = %s", szTemplate);
		inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetCUPKeyExchangeTimes(szTemplate);
		sprintf(szPrintBuffer, "18.CUP安全認證 = %s", szTemplate);
		inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetMACEnable(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_ChineseFont("19.上傳MAC = On", _PRT_HEIGHT_);
		else
			inPRINT_ChineseFont("19.上傳MAC = Off", _PRT_HEIGHT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetPinpadMode(szTemplate);

		if (!memcmp(szTemplate, "0", 1))
		{
		        inPRINT_ChineseFont("20.密碼機 = None", _PRT_HEIGHT_);
		}
		else if (!memcmp(szTemplate, "1", 1))
		{
		        inPRINT_ChineseFont("20.密碼機 = Internal", _PRT_HEIGHT_);
		}
		else if (!memcmp(szTemplate, "2", 1))
		{
		        inPRINT_ChineseFont("20.密碼機 = External", _PRT_HEIGHT_);
                }

		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetFORCECVV2(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_ChineseFont("21.強制輸入CVV2 = On", _PRT_HEIGHT_);
		else
			inPRINT_ChineseFont("21.強制輸入CVV2 = Off", _PRT_HEIGHT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetElecCommerceFlag(szTemplate);

		if (!memcmp(szTemplate, "00", 2))
			inPRINT_ChineseFont("22.EC參數 = 非郵購及定期性行業", _PRT_HEIGHT_);
		else if (!memcmp(szTemplate, "01", 2))
			inPRINT_ChineseFont("22.EC參數 = 郵購行業", _PRT_HEIGHT_);
		else if (!memcmp(szTemplate, "02", 2))
			inPRINT_ChineseFont("22.EC參數 = 定期性行業", _PRT_HEIGHT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetDccFlowVersion(szTemplate);

                if (memcmp(szTemplate, _NCCC_DCC_FLOW_VER_NOT_SUPORTED_, 1) == 0)
                        inPRINT_ChineseFont("23.DCC Flow Version = Not Supported DCC", _PRT_HEIGHT_);
                else if (memcmp(szTemplate, _NCCC_DCC_FLOW_VER_BY_CARD_BIN_, 1) == 0)
                        inPRINT_ChineseFont("23.DCC Flow Version = By BIN Select", _PRT_HEIGHT_);
                else if (memcmp(szTemplate, _NCCC_DCC_FLOW_VER_BY_MANUAL_, 1) == 0)
                        inPRINT_ChineseFont("23.DCC Flow Version = By Currency Select", _PRT_HEIGHT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetSupDccVisa(szTemplate);

                if (!memcmp(szTemplate, "Y", 1))
                        inPRINT_ChineseFont("24.Support DCC VISA = On", _PRT_HEIGHT_);
                else
                        inPRINT_ChineseFont("24.Support DCC VISA = Off", _PRT_HEIGHT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetSupDccMasterCard(szTemplate);

                if (!memcmp(szTemplate, "Y", 1))
                        inPRINT_ChineseFont("25.Support DCC M/C = On", _PRT_HEIGHT_);
                else
                        inPRINT_ChineseFont("25.Support DCC M/C = Off", _PRT_HEIGHT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetBarCodeReaderEnable(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_ChineseFont("26.BarCode Reader = On", _PRT_HEIGHT_);
		else
			inPRINT_ChineseFont("26.BarCode Reader = Off", _PRT_HEIGHT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetCreditCardFlag(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_ChineseFont("27.刷卡兌換 = On", _PRT_HEIGHT_);
		else
			inPRINT_ChineseFont("27.刷卡兌換 = Off", _PRT_HEIGHT_);

                memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
                memset(szTemplate2, 0x00, sizeof(szTemplate2));
                inGetCreditCardStartDate(szTemplate);
                inGetCreditCardEndDate(szTemplate2);
		sprintf(szPrintBuffer, "28.刷卡兌換起迄日 = %s~%s", szTemplate, szTemplate2);
		inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetBarCodeFlag(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_ChineseFont("29.條碼兌換 = On", _PRT_HEIGHT_);
		else
			inPRINT_ChineseFont("29.條碼兌換 = Off", _PRT_HEIGHT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
                memset(szTemplate2, 0x00, sizeof(szTemplate2));
                inGetBarCodeStartDate(szTemplate);
                inGetBarCodeEndDate(szTemplate2);
		sprintf(szPrintBuffer, "30.條碼兌換起迄日 = %s~%s", szTemplate, szTemplate2);
		inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetVoidRedeemFlag(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_ChineseFont("31.條碼兌換取消 = On", _PRT_HEIGHT_);
		else
			inPRINT_ChineseFont("31.條碼兌換取消 = Off", _PRT_HEIGHT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
                memset(szTemplate2, 0x00, sizeof(szTemplate2));
                inGetVoidRedeemStartDate(szTemplate);
                inGetVoidRedeemEndDate(szTemplate2);
		sprintf(szPrintBuffer, "32.條碼兌換取消起迄日 = %s~%s", szTemplate, szTemplate2);
		inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetASMFlag(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_ChineseFont("33.優惠功能 = On", _PRT_HEIGHT_);
		else
			inPRINT_ChineseFont("33.優惠功能 = Off", _PRT_HEIGHT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
                memset(szTemplate2, 0x00, sizeof(szTemplate2));
                inGetASMStartDate(szTemplate);
                inGetASMEndDate(szTemplate2);
		sprintf(szPrintBuffer, "34.優惠功能參數起迄日 = %s~%s", szTemplate, szTemplate2);
		inPRINT_ChineseFont(szPrintBuffer, _PRT_HEIGHT_);
		break;
	}

	return (VS_SUCCESS);
}

int inCREDIT_PRINT_ParamLOGO_END(TRANSACTION_OBJECT *pobTran)
{
	inPRINT_ChineseFont("", _PRT_HEIGHT_);

        if (inPRINT_PutGraphic((unsigned char *)_END_LOGO_) == VS_ERROR)
        {
                return (VS_ERROR);
        }

        inPRINT_SpaceLine(8);

	return (VS_SUCCESS);
}


/*
Function        :inCREDIT_PRINT_TOP
Date&Time       :2015/8/20 上午 10:24
Describe        :
*/
int inCOSTCO_PRINT_Top(TRANSACTION_OBJECT *pobTran)
{
	char    szPrintBuf[84 + 1], szTemplate[42 + 1];

	/* Get商店代號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetMerchantID(szTemplate);

	/* 列印商店代號 */
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PAD_RIGHT_FILL_LEFT_);
	sprintf(szPrintBuf, "商店代號%s", szTemplate);
	inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

	/* Get端末機代號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);

	/* 列印端末機代號 */
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PAD_RIGHT_FILL_LEFT_);
	sprintf(szPrintBuf, "端末機代號%s", szTemplate);
	inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

	inPRINT_ChineseFont("================================================", _PRT_NORMAL_);

	/* 列印日期 / 時間 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inPRINT_ChineseFont("日期/時間(Date/Time)", _PRT_HEIGHT_);
//	memcpy(&szPrintBuf[0], &pobTran->srBRec.szDate[0], 4);
	memcpy(&szPrintBuf[0], "2022", 4);
	strcat(szPrintBuf, "/");
//	memcpy(&szPrintBuf[5], &pobTran->srBRec.szDate[4], 2);
	memcpy(&szPrintBuf[5], "12", 2);
	strcat(szPrintBuf, "/");
//	memcpy(&szPrintBuf[8], &pobTran->srBRec.szDate[6], 2);
	memcpy(&szPrintBuf[8], "27", 2);
	strcat(szPrintBuf, "   ");
//	memcpy(&szPrintBuf[13], &pobTran->srBRec.szTime[0], 2);
	memcpy(&szPrintBuf[13], "18", 2);
	strcat(szPrintBuf, ":");
//	memcpy(&szPrintBuf[16], &pobTran->srBRec.szTime[2], 2);
	memcpy(&szPrintBuf[16], "10", 2);
	inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

	/* 列印交易類別 */
	if (pobTran->srBRec.inCode == _SETTLE_)
	{
		inPRINT_ChineseFont("交易類別(Trans. Type)", _PRT_HEIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "60 結帳 SETTLEMENT");
		inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);
	}

	/* 列印批次號碼 */
	inPRINT_ChineseFont("批次號碼(Batch No.)", _PRT_HEIGHT_);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//	sprintf(szTemplate, "%03ld", pobTran->srBRec.lnBatchNum);
	sprintf(szTemplate, "%03ld", 999);
	strcpy(szPrintBuf, szTemplate);
	inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);
	/* 列印主機 */
	inPRINT_ChineseFont("主機(Host)", _PRT_HEIGHT_);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//	inGetHostLabel(szPrintBuf);
	strcat(szTemplate, "FUBON");
	inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmount
Date&Time       :2015/8/20 上午 10:24
Describe        :列印總金額
*/
int inCOSTCO_PRINT_TotalAmount(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, int inRecordCnt)
{
	char    szPrintBuf[84 + 1], szTemplate[84 + 1], szTemplate1[84 + 1];

	if (pobTran->inRunOperationID == _OPERATION_TOTAL_REPORT_)
		inPRINT_ChineseFont("         總額報表", _PRT_DOUBLE_HEIGHT_WIDTH_);
	else
	{
		inPRINT_ChineseFont("         結帳報表", _PRT_DOUBLE_HEIGHT_WIDTH_);
	}

	inPRINT_ChineseFont("    筆數(CNT)      金額(AMOUNT)", _PRT_HEIGHT_);

	srAccumRec->lnTotalCount = 999;
	
	if (srAccumRec->lnTotalCount == 0)
	{
		/* 銷售 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szPrintBuf, "銷售 Ｄ　%03lu   NT$", 0L);
		sprintf(szTemplate, "%ld", 0L);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate1);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
		/* 退貨 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szPrintBuf, "退貨 Ｒ　%03lu   NT$", 0L);
		sprintf(szTemplate, "%ld", 0L);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate1);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
		/* 淨額 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szPrintBuf, "淨額 Ｔ　%03lu   NT$", 0L);
		sprintf(szTemplate, "%ld", 0L);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate1);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
		inPRINT_ChineseFont("    ------------------------------------------------------------", _PRT_NORMAL_);
		/* 小費 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szPrintBuf, "小費 　　%03lu   NT$", 0L);
		sprintf(szTemplate, "%ld", 0L);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate1);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
		inPRINT_ChineseFont("==========================================", _PRT_HEIGHT_);
	}
	else
	{
		srAccumRec->lnTotalSaleCount = 999;
		srAccumRec->llTotalSaleAmount  = 999999999;
		
		srAccumRec->lnTotalRefundCount = 999;
		srAccumRec->llTotalRefundAmount  = 999999999;
		
		/* 銷售 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szPrintBuf, "銷售 Ｄ　%03lu   NT$", srAccumRec->lnTotalSaleCount);
		sprintf(szTemplate, "%lld", (srAccumRec->llTotalSaleAmount + srAccumRec->llTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
		/* 退貨 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szPrintBuf, "退貨 Ｒ　%03lu   NT$", srAccumRec->lnTotalRefundCount);
		sprintf(szTemplate, "%lld", (0 - srAccumRec->llTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
		/* 淨額 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szPrintBuf, "淨額 Ｔ　%03lu   NT$", srAccumRec->lnTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
		inPRINT_ChineseFont("    ------------------------------------------------------------", _PRT_NORMAL_);
		/* 小費 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnTotalTipsCount);
		sprintf(szTemplate, "%lld", srAccumRec->llTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
		inPRINT_ChineseFont("==========================================", _PRT_HEIGHT_);
	}
	return (VS_SUCCESS);
}


/*
Function        :inCREDIT_PRINT_TotalAmountByCard
Date&Time       :2015/8/20 上午 10:24
Describe        :依卡別列印
*/
int inCOSTCO_PRINT_TotalAmountByCard(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, int inRecordCnt)
{
	char    szPrintBuf[84 + 1], szTemplate[84 + 1];

	inPRINT_ChineseFont("         卡別小計", _PRT_DOUBLE_HEIGHT_WIDTH_);

	if (srAccumRec->llUCardTotalSaleAmount != 0L || srAccumRec->llUCardTotalRefundAmount != 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "卡別 　　%s", "UCARD");
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );

		/* 銷售 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnUCardTotalSaleCount);
		sprintf(szTemplate, "%lld", (srAccumRec->llUCardTotalSaleAmount + srAccumRec->llUCardTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		/* 退貨 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnUCardTotalRefundCount);
		sprintf(szTemplate, "%lld", (0 - srAccumRec->llUCardTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		/* 小計 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnUCardTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llUCardTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		inPRINT_ChineseFont("    ------------------------------------------------------------", _PRT_NORMAL_);
		/* 小費 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnUCardTotalTipsCount);
		sprintf(szTemplate, "%lld", srAccumRec->llUCardTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		inPRINT_ChineseFont("-------------------------------------------------------------------------------------", _PRT_NORMAL_);
	}

	srAccumRec->llVisaTotalSaleAmount = 99999999;
	srAccumRec->llVisaTotalRefundAmount = 99999999;
	
	srAccumRec->lnVisaTotalCount = 999;
	srAccumRec->lnVisaTotalRefundCount = 999;
	
	if (srAccumRec->llVisaTotalSaleAmount != 0L || srAccumRec->llVisaTotalRefundAmount != 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_VISA_);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		/* 銷售 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnVisaTotalSaleCount);
		sprintf(szTemplate, "%lld", (srAccumRec->llVisaTotalSaleAmount + srAccumRec->llVisaTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		/* 退貨 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnVisaTotalRefundCount);
		sprintf(szTemplate, "%lld", (0 - srAccumRec->llVisaTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		/* 小計 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnVisaTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llVisaTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		inPRINT_ChineseFont("    ------------------------------------------------------------", _PRT_NORMAL_);
		/* 小費 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnVisaTotalTipsCount);
		sprintf(szTemplate, "%lld", srAccumRec->llVisaTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		inPRINT_ChineseFont("-------------------------------------------------------------------------------------", _PRT_NORMAL_);
	}

	srAccumRec->llMasterTotalSaleAmount = 99999999;
	srAccumRec->llMasterTotalRefundAmount = 99999999;
	
	srAccumRec->lnMasterTotalSaleCount = 999;
	srAccumRec->lnMasterTotalRefundCount = 999;
	
	if (srAccumRec->llMasterTotalSaleAmount != 0L || srAccumRec->llMasterTotalRefundAmount != 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "卡別 　　%s", "MASTER CARD");
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );

		/* 銷售 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnMasterTotalSaleCount);
		sprintf(szTemplate, "%lld", (srAccumRec->llMasterTotalSaleAmount + srAccumRec->llMasterTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		/* 退貨 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnMasterTotalRefundCount);
		sprintf(szTemplate, "%lld", (0 - srAccumRec->llMasterTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		/* 小計 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnMasterTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llMasterTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		inPRINT_ChineseFont("    ------------------------------------------------------------", _PRT_NORMAL_);
		/* 小費 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnMasterTotalTipsCount);
		sprintf(szTemplate, "%lld", srAccumRec->llMasterTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		inPRINT_ChineseFont("-------------------------------------------------------------------------------------", _PRT_NORMAL_);
	}

	srAccumRec->llJcbTotalSaleAmount = 99999999;
	srAccumRec->llJcbTotalRefundAmount = 99999999;
	
	srAccumRec->lnJcbTotalSaleCount = 999;
	srAccumRec->lnJcbTotalRefundCount = 999;
	
	if (srAccumRec->llJcbTotalSaleAmount != 0L || srAccumRec->llJcbTotalRefundAmount != 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_JCB_);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );

		/* 銷售 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnJcbTotalSaleCount);
		sprintf(szTemplate, "%lld", (srAccumRec->llJcbTotalSaleAmount + srAccumRec->llJcbTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		/* 退貨 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnJcbTotalRefundCount);
		sprintf(szTemplate, "%lld", (0 - srAccumRec->llJcbTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		/* 小計 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnJcbTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llJcbTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		inPRINT_ChineseFont("    ------------------------------------------------------------", _PRT_NORMAL_);
		/* 小費 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnJcbTotalTipsCount);
		sprintf(szTemplate, "%lld", srAccumRec->llJcbTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		inPRINT_ChineseFont("-------------------------------------------------------------------------------------", _PRT_NORMAL_);
	}

	if (srAccumRec->llAmexTotalSaleAmount != 0L || srAccumRec->llAmexTotalRefundAmount != 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_AMEX_);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );

		/* 銷售 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnAmexTotalSaleCount);
		sprintf(szTemplate, "%lld", (srAccumRec->llAmexTotalSaleAmount + srAccumRec->llAmexTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		/* 退貨 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnAmexTotalRefundCount);
		sprintf(szTemplate, "%lld", (0 - srAccumRec->llAmexTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		/* 小計 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnAmexTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llAmexTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		inPRINT_ChineseFont("    ------------------------------------------------------------", _PRT_NORMAL_);
		/* 小費 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnAmexTotalTipsCount);
		sprintf(szTemplate, "%lld", srAccumRec->llAmexTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		inPRINT_ChineseFont("-------------------------------------------------------------------------------------", _PRT_NORMAL_);
	}

	if (srAccumRec->llCupTotalSaleAmount != 0L || srAccumRec->llCupTotalRefundAmount != 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_CUP_);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );

		/* 銷售 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnCupTotalSaleCount);
		sprintf(szTemplate, "%lld", (srAccumRec->llCupTotalSaleAmount + srAccumRec->llCupTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		/* 退貨 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnCupTotalRefundCount);
		sprintf(szTemplate, "%lld", (0 - srAccumRec->llCupTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		/* 小計 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnCupTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llCupTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		inPRINT_ChineseFont("    ------------------------------------------------------------", _PRT_NORMAL_);
		/* 小費 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnCupTotalTipsCount);
		sprintf(szTemplate, "%lld", srAccumRec->llCupTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		inPRINT_ChineseFont("-------------------------------------------------------------------------------------", _PRT_NORMAL_);
	}

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "卡別 　　%s", "國際卡");
	inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );

	/* 銷售 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "銷售 　　%03lu   NT$", 999);
	sprintf(szTemplate, "%lld", 99999999);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
	/* 退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "退貨 　　%03lu   NT$", 999);
	sprintf(szTemplate, "%lld", (0 - 99999999));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
	/* 小計 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "小計 　　%03lu   NT$", 999);
	sprintf(szTemplate, "%lld", 99999999);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
	inPRINT_ChineseFont("    ------------------------------------------------------------", _PRT_NORMAL_);
	/* 小費 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "小費 　　%03lu   NT$", 0);
	sprintf(szTemplate, "%lld", 0);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
	inPRINT_ChineseFont("-------------------------------------------------------------------------------------", _PRT_NORMAL_);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "卡別 　　%s", "金融卡");
	inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );

	/* 銷售 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "銷售 　　%03lu   NT$", 999);
	sprintf(szTemplate, "%lld", 99999999);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
	/* 退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "退貨 　　%03lu   NT$", 999);
	sprintf(szTemplate, "%lld", (0 - 99999999));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
	/* 小計 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "小計 　　%03lu   NT$", 999);
	sprintf(szTemplate, "%lld", 99999999);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
	inPRINT_ChineseFont("    ------------------------------------------------------------", _PRT_NORMAL_);
	/* 小費 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "小費 　　%03lu   NT$", 0);
	sprintf(szTemplate, "%lld", 0);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
	inPRINT_ChineseFont("-------------------------------------------------------------------------------------", _PRT_NORMAL_);
	
	
	return (VS_SUCCESS);
}


int inCOSTCO_PRINT_DetailReportMiddle(TRANSACTION_OBJECT *pobTran, int inRecordCnt)
{
	char    inRetChar[1+1];                 /* catch Y or N */

	inPRINT_ChineseFont("         明細報表", _PRT_DOUBLE_HEIGHT_WIDTH_);

	inPRINT_ChineseFont("交易類別", _PRT_HEIGHT_);
	inPRINT_ChineseFont("交易金額", _PRT_HEIGHT_);
	inPRINT_ChineseFont("卡號　　　　　　　　　", _PRT_HEIGHT_);
	inPRINT_ChineseFont("交易日期　　　　　 交易時間", _PRT_HEIGHT_);
	inPRINT_ChineseFont("授權碼　　　　　　 調閱編號", _PRT_HEIGHT_);
	inPRINT_ChineseFont("序號", _PRT_HEIGHT_);

	inGetStoreIDEnable(inRetChar);
	if (inRetChar[0] == 'Y')
		inPRINT_ChineseFont("櫃號", _PRT_HEIGHT_);

	inPRINT_ChineseFont("==========================================", _PRT_HEIGHT_);

	return (VS_SUCCESS);
}


int inCOSTCO_PRINT_DetailReportBottom(TRANSACTION_OBJECT *pobTran, int inRecordCnt)
{
	char            szPrintBuf[62 + 1], szTemplate1[62 + 1], szTemplate2[62 + 1];;
	char            inRetChar[1+1];/* catch Y or N */
	int             inRetVal = VS_SUCCESS;
	int             i;

	/* 開始讀取 */
//	inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);

	for (i = 0; i < inRecordCnt; i ++)
	{
//		/*. 開始讀取每一筆交易記錄 .*/
//		if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, i) != VS_SUCCESS)
//		{
//			inRetVal = VS_ERROR;
//			break;
//		}
//
//		if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
//			continue;

		/*Trans Type*/
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//		inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
//		sprintf(szPrintBuf,"%s %s", szTemplate1, szTemplate2);
		switch(i)
		{
			case 0:
				sprintf(szPrintBuf,"%s ", "信用卡");
				break;
			case 1:
				sprintf(szPrintBuf,"%s ", "好多金");
				break;
			case 2:
				sprintf(szPrintBuf,"%s ", "分期");
				break;
			case 3:
				sprintf(szPrintBuf,"%s ", "購物金");
				break;
		}
			
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);

		/* Print Amount */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));

//		if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
//		{
//			switch (pobTran->srBRec.inCode)
//			{
//				case _SALE_:
//				case _SALE_OFFLINE_ :
//				case _PRE_AUTH_ :
//				case _INST_SALE_ :
//				case _REDEEM_SALE_ :
//					sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
//					break;
//				case _TIP_ :
//					sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
//					break;
//				case _REFUND_ :
//					sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
//					break;
//				case _ADJUST_ :
//					sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
//					break;
//				default :
//					inGetHostLabel(szPrintBuf);
//					sprintf(szTemplate1,"%s_AMT_ERR_inCode(%d)",szPrintBuf, pobTran->srBRec.inCode);
//					break;
//			} /* End switch () */
//		}
//		else
//		{
//			switch (pobTran->srBRec.inOrgCode)
//			{
//				case _SALE_ :
//				case _SALE_OFFLINE_ :
//				case _PRE_AUTH_ :
//				case _INST_SALE_ :
//				case _REDEEM_SALE_ :
//					sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
//					break;
//				case _TIP_ :
//					sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
//					break;
//				case _REFUND_ :
//					sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
//					break;
//				case _ADJUST_ :
//					sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
//					break;
//				default :
//					inGetHostLabel(szPrintBuf);
//					sprintf(szTemplate1, "%s_AMT_VOID_ERR_inCode(%d)",szPrintBuf, pobTran->srBRec.inCode);
//					break;
//			} /* End switch () */
//		}
		
		sprintf(szTemplate1, "%ld", 9999999);
		
		strcat(szPrintBuf, "AMT:");
		inFunc_Amount_Comma(szTemplate1, "" , ' ', _SIGNED_NONE_, 13, VS_FALSE);
		strcat(szPrintBuf, szTemplate1);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);

		/*Pan*/
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//		strcpy(szPrintBuf, pobTran->srBRec.szPAN);
		strcpy(szPrintBuf, "1234567890123456");
		inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 19, _PAD_LEFT_FILL_RIGHT_);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);

		/*Trans Date Time*/
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(&szTemplate1[0], '/', 10);
//		memcpy(&szTemplate1[0], &(pobTran->srBRec.szDate[0]),4);
//		memcpy(&szTemplate1[5], &(pobTran->srBRec.szDate[4]),2);
//		memcpy(&szTemplate1[8], &(pobTran->srBRec.szDate[6]),2);
		memcpy(&szTemplate1[0], "2022",4);
		memcpy(&szTemplate1[5], "12",2);
		memcpy(&szTemplate1[8], "27",2);
		szTemplate1[10] = _NULL_CH_;
		strcpy(szPrintBuf, "DATE:");
		strcat(szPrintBuf, szTemplate1);
		inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 18, _PAD_LEFT_FILL_RIGHT_);
		strcat(szPrintBuf, "TIME:");
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(&szTemplate1[0], ':', 8);
//		memcpy(&szTemplate1[0], &(pobTran->srBRec.szTime[0]), 2);
//		memcpy(&szTemplate1[3], &(pobTran->srBRec.szTime[2]), 2);
//		memcpy(&szTemplate1[6], &(pobTran->srBRec.szTime[4]), 2);
		memcpy(&szTemplate1[0], "18", 2);
		memcpy(&szTemplate1[3], "10", 2);
		memcpy(&szTemplate1[6], "33", 2);
		szTemplate1[8] = 0x00;
		strcat(szPrintBuf, szTemplate1);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);

		/* Approved No. */ /* 授權碼 = Batch Number + Invoice Number */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, "APPR:");
//		strcat(szPrintBuf, pobTran->srBRec.szAuthCode);
		strcat(szPrintBuf, "654321");
		inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 18, _PAD_LEFT_FILL_RIGHT_ );

		/*Invoice Number*/
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
//		sprintf(szTemplate1, "INV:%06ld", pobTran->srBRec.lnOrgInvNum);
		sprintf(szTemplate1, "INV:%06ld", 999999);
		strcat(szPrintBuf, szTemplate1);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );

		/* RRN */ /*invoice number + RefNo */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//		sprintf(szPrintBuf, "REF:%06ld", atol(pobTran->srBRec.szRefNo));
		sprintf(szPrintBuf, "REF:%06ld", atol(pobTran->srBRec.szRefNo));
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);

		/* Store ID */
		memset(inRetChar, 0x00, sizeof(inRetChar));
		if (inGetStoreIDEnable(inRetChar) != VS_SUCCESS )
		{
			/*取得StoreIDEnable失敗，回傳VS_Error*/
			return (VS_ERROR);
		}
		else if (inRetChar[0] == 'Y')
		{
			/*開啟櫃號功能*/
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "STORE ID:%s", pobTran->srBRec.szStoreID);
			inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_ );
		}
		else
		{
			   /*沒開啟櫃號功能，則不印櫃號*/
		}

		inPRINT_ChineseFont("-------------------------------------------------------------------------------------", _PRT_NORMAL_);
	} /* End for () .... */

	/* 結束讀取 */
//	inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);

	return (inRetVal);
}


