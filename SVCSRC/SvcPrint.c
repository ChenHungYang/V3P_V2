
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <ctos_qrcode.h>

#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/TransType.h"

#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/DISPLAY/DisTouch.h"

#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/PRINT/PrtMsg.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FILE_FUNC/File.h"
#include "../SOURCE/FUNCTION/FILE_FUNC/FIleLogFunc.h"
#include "../SOURCE/COMM/Comm.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/EST.h"
#include "../SOURCE/FUNCTION/CPT.h"
#include "../SOURCE/FUNCTION/SCDT.h"
#include "../SOURCE/FUNCTION/PWD.h"
#include "../SOURCE/FUNCTION/PCD.h"
#include "../SOURCE/FUNCTION/ASMC.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/Batch.h"

#include "../SOURCE/FUNCTION/AccountFunction/PrintBillInfo.h"

#include "../SOURCE/TMS/TMSTABLE/TmsCPT.h"
#include "../SOURCE/TMS/TMSTABLE/TmsSCT.h"

#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/EVENT/MenuMsg.h"

#include "SvcPrint.h"

extern ginMachineType;

/* 列印帳單使用(START) */
SVC_PRINT_RECEIPT_TYPE_TABLE_BYBUFFER srSvcReceiptType_ByBuffer[] =
{
	/* 信用卡 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inSVC_PRINT_ReceiptData,
		inSVC_PRINT_ReceiptAmountData,
		inSVC_PRINT_ReceiptBottomData
	},
};


void vdSVC_PRINT_GetTransType(TRANSACTION_OBJECT *pobTran, char *szPrintBuf1)
{

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	
	inDISP_LogPrintfWithFlag("vd_SVC_GetTransType()[%d]", pobTran->srBRec.inCode);

	switch (pobTran->srBRec.inCode)
	{
		case _SVC_REDEEM_:
			strcpy(szPrintBuf1, "7647 購物卡消費");
			break;
		case _SVC_RELOAD_:
			/*
			2017/10/28 (週六) 上午 12:27
			Wallace Lu-呂庚霖-零售業務處 <wallacelu@systex.com>
			RE: Costco第二階段賣場上線及MPU開卡問題 status update.

			二、10/27 New CR from Ryan
			(1) 列印簽單現金加值 更名為 現金儲值 add by sampo 2017/10/30 上午 10:14:58 */

			strcpy(szPrintBuf1, "7707 購物卡儲值");
			break;
		case _SVC_ACTIVECARD_:
			strcpy(szPrintBuf1, "7147 開卡交易");
			break;
		case _SVC_VOID_:
			break;
		case _SVC_REFUND_: /* 把電子錢包消費的金額加回 */
			strcpy(szPrintBuf1, "7707 退貨交易");
			break;
		case _SVC_BACKCARD_: /* 即電子錢包消費(消費錢包所有金額) */
			strcpy(szPrintBuf1, "7577 購物卡兌現");
			break;
		case _SVC_INQUIRY_:
			strcpy(szPrintBuf1, "7567 查詢");
			break;
		default:
			break;
	}
}


int inSVC_PRINT_ReceiptData(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
	int	inRetVal = 0;
	char	szPrintBuf[42 + 1], szTemplate[42 + 1];
	int	inCardOrgLen;
	char	szCardAscData[24 + 1];
	char	szSTANAsc[6 + 1];

	/* 交易類別 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcat(szPrintBuf, "交易類別(Trans. Type)");
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	vdSVC_PRINT_GetTransType(pobTran, &szTemplate[0]);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%s", szTemplate);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	//卡號
	memset(szCardAscData,0x00,sizeof(szCardAscData));
	inFunc_BCD_to_ASCII(szCardAscData, (unsigned char*)pobTran->srBRec.szSvcCardNumber, 6);
	inCardOrgLen = strlen (szCardAscData);

	//去掉F
	for ( i = 0 ;i < inCardOrgLen; i++)
	{
		if ( szCardAscData[i] == 'F')
		{
			szCardAscData[i] = 0x00;
		}
	}

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcat(szPrintBuf, "卡號(Card No.)        有效期");
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%s     %02X%02X/%02X/%02X",
				szCardAscData, pobTran->srBRec.szSvcExpireDay[0], pobTran->srBRec.szSvcExpireDay[1], pobTran->srBRec.szSvcExpireDay[2], pobTran->srBRec.szSvcExpireDay[3]);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcat(szPrintBuf, "日期/時間(Date/Time)  批次號碼(Batch No.)");
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcat(szTemplate, inGetBatchNum());

	strncat(szPrintBuf, &pobTran->srBRec.szSvcTxnDateTime[0],4);
	strncat(szPrintBuf, "\x2F",1);
	strncat(szPrintBuf, &pobTran->srBRec.szSvcTxnDateTime[4],2);
	strncat(szPrintBuf, "\x2F",1);
	strncat(szPrintBuf, &pobTran->srBRec.szSvcTxnDateTime[6],2);
	strncat(szPrintBuf, "\x20",1);
	strncat(szPrintBuf, &pobTran->srBRec.szSvcTxnDateTime[8],2);
	strncat(szPrintBuf, ":",1);
	strncat(szPrintBuf, &pobTran->srBRec.szSvcTxnDateTime[10],2);
	strncat(szPrintBuf, ":",1);
	strncat(szPrintBuf, &pobTran->srBRec.szSvcTxnDateTime[12],2);
	strncat(szPrintBuf, " ",1);
	strcat(szPrintBuf, " ");
	strcat(szPrintBuf, szTemplate);

	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* 序號 & 調閱編號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcat(szPrintBuf, "調閱編號(Inv. No.)    簽單重印編號(ROC. No.)");
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcat(szPrintBuf, &pobTran->srBRec.szSvcTxnDateTime[4]);

	memset(szSTANAsc, 0x00, sizeof(szSTANAsc));
	sprintf(szSTANAsc, "%06ld", pobTran->srBRec.lnSTANNum);
	strcat(szPrintBuf, &szSTANAsc[4]);

	strcat(szPrintBuf, "         ");
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgSTANNum);		
	strcat(szPrintBuf, szTemplate);
	
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	return (VS_SUCCESS);
}

int inSVC_PRINT_ReceiptAmountData(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = 0;
	char	szPrintBuf[42 + 1], szTemplate[42 + 1];
	long	lnBalaceAmount = 0;

	if (pobTran->srBRec.inCode == _SVC_INQUIRY_)
	{
		/* 購物卡餘額 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcat(szPrintBuf, "購物卡餘額 : ");
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_BCD_to_ASCII(szTemplate, (unsigned char*)pobTran->srBRec.szSvcPoint1, 5);
		lnBalaceAmount = atol(szTemplate)/100;
		sprintf(szPrintBuf, "NT$%ld", (lnBalaceAmount));
		inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 13, _PADDING_LEFT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	}
	else
	{
		/* 現場修正*/
		if ( pobTran->srBRec.lnSvcRedeemAmount > 0)
		{
			/* 交易金額 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcat(szPrintBuf, "交易金額　　 : ");
			inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "NT$%ld", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnSvcRedeemAmount));
			inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 13, _PADDING_LEFT_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		}
		else
		{
			/* 交易金額 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcat(szPrintBuf, "交易金額　　 : ");
			inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			if (pobTran->srBRec.inCode == _SVC_REFUND_)
			{
				sprintf(szPrintBuf, "NT$%ld", (0 - pobTran->srBRec.lnTxnAmount));
			}
			else
			{
				sprintf(szPrintBuf, "NT$%ld", (pobTran->srBRec.lnTxnAmount));
			}
			inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 13, _PADDING_LEFT_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		}

		inRetVal = inPRINT_Buffer_PutIn("==========================================", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inRetVal = inPRINT_Buffer_PutIn("-----------------餘額區-------------------", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 購物卡餘額 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcat(szPrintBuf, "購物卡餘額 : ");
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_BCD_to_ASCII(szTemplate, (unsigned char*)pobTran->srBRec.szSvcPoint1, 5);
		lnBalaceAmount = atol(szTemplate)/100;
		sprintf(szPrintBuf, "NT$%ld", (lnBalaceAmount));
		inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 13, _PADDING_LEFT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

#ifdef DEMO_EDC
	inVERIX_PRINT_LOGOFile(NCCC_TRAINING_LOGO); /* 列印教育訓練模式 */
	vdVERIX_PRINT_ChineseFont(" ", PRT_HEIGHT, PRT_COLUMN_42);
	vdVERIX_PRINT_ChineseFont(" ", PRT_NORMAL, PRT_COLUMN_42);
	vdVERIX_PRINT_ChineseFont(" ", PRT_NORMAL, PRT_COLUMN_42);
	vdVERIX_PRINT_ChineseFont(" ", PRT_NORMAL, PRT_COLUMN_42);
	vdVERIX_PRINT_ChineseFont(" ", PRT_NORMAL, PRT_COLUMN_42);
	vdVERIX_PRINT_ChineseFont(" ", PRT_NORMAL, PRT_COLUMN_42);
#else
	inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
#endif

        return (VS_SUCCESS);
}


/*
Function	: inSVC_PRINT_ReceiptBottomData
Date&Time	: 2015/8/10 上午 10:24
Describe	: 列印結尾
*/
int inSVC_PRINT_ReceiptBottomData(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
	char	szTemplate[42 + 1]= {0};
	char	szDemoMode[2 + 1] = {0};

	

	/* 橫式 */
	if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
	{

		/* 教育訓練模式 */
		memset(szDemoMode, 0x00, sizeof(szDemoMode));
		inGetDemoMode(szDemoMode);
		if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
		{
			if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
			{
				inDISP_LogPrintfWithFlag("inPRINT_PutGraphic(_NCCC_DEMO_) failed");
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
				inDISP_LogPrintfWithFlag("inPRINT_PutGraphic(_NCCC_DEMO_) failed");
			}
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
				inDISP_LogPrintfWithFlag("inPRINT_PutGraphic(_NCCC_DEMO_) failed");
			}
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

	for (i = 0; i < 8; i++)
	{
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}



	return (VS_SUCCESS);
}



int inSVC_PRINT_TestPrintTotal(TRANSACTION_OBJECT *pobTran)
{
	inCREDIT_PRINT_Logo(pobTran);
	
	inCOSTCO_PRINT_Top(pobTran);
	
	inCOSTCO_PRINT_TotalAmount(pobTran);
	
	inCOSTCO_PRINT_TotalAmountByCard(pobTran);
	
	inCREDIT_PRINT_End(pobTran);
	
	return VS_SUCCESS;
}

int inSVC_PRINT_TestPrintDetail(TRANSACTION_OBJECT *pobTran)
{
	ACCUM_TOTAL_REC srAccumRec;
	memset(&srAccumRec, 0x00, sizeof(ACCUM_TOTAL_REC));
	
	inCREDIT_PRINT_Logo(pobTran);
	
	inCOSTCO_PRINT_Top(pobTran);
	
	inCOSTCO_PRINT_TotalAmount(pobTran, &srAccumRec, 0);
	
	inCOSTCO_PRINT_DetailReportMiddle(pobTran, 0);
	
	inCOSTCO_PRINT_DetailReportBottom(pobTran, 4);
	
	inCREDIT_PRINT_End(pobTran);
	
	return VS_SUCCESS;
}


/*
Function	: inSVC_PRINT_Check
Date&Time	: 2015/8/20 上午 10:24
Describe	: 
*/
int inSVC_PRINT_Check(TRANSACTION_OBJECT *pobTran)
{
        int     inRecordCnt;

        inRecordCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
        /* 回傳VS_ERROR(回傳 -1 )會跳出，交易筆數小於0( VS_NoRecord 會回傳 -98 )會印空白簽單 */
        /* 其餘則回傳交易筆數*/

        return (inRecordCnt);
}


/*
Function	: inSVC_PRINT_ReceiptFlowByBuffer
Date&Time	:2015/8/10 上午 10:24
Describe	: 
 * 列印 SVC 卡帳單
*/
int inSVC_PRINT_ReceiptFlowByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int			inPrintIndex = 0, inRetVal;
	char			szTRTFileName[12 + 1];
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	inDISP_LogPrintfWithFlag(" Print ByBuffer RRN [%s]", pobTran->srBRec.szRefNo );

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

		while (1)
		{
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
			/* 列印LOGO */
			if (srSvcReceiptType_ByBuffer[inPrintIndex].inLogo != NULL)
				if ((inRetVal = srSvcReceiptType_ByBuffer[inPrintIndex].inLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			/* 列印TID MID */
			if (srSvcReceiptType_ByBuffer[inPrintIndex].inTop != NULL)
				if ((inRetVal = srSvcReceiptType_ByBuffer[inPrintIndex].inTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			/* 列印DATA */
			if (srSvcReceiptType_ByBuffer[inPrintIndex].inData != NULL)
				if ((inRetVal = srSvcReceiptType_ByBuffer[inPrintIndex].inData(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			/* 列印金額 */
			if (srSvcReceiptType_ByBuffer[inPrintIndex].inAmount != NULL)
				if ((inRetVal = srSvcReceiptType_ByBuffer[inPrintIndex].inAmount(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

		
			/* 列印簽名欄  & 警語 */
			if (srSvcReceiptType_ByBuffer[inPrintIndex].inEnd != NULL)
				if ((inRetVal = srSvcReceiptType_ByBuffer[inPrintIndex].inEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);
			
			break;
		}
		
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}
}


/*
Function	: inSVC_PRINT_ReceiptByBuffer
Date&Time	: 2022/12/27 下午 7:32
Describe	: 
*/
int inSVC_PRINT_ReceiptByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CUST_RECEIPT_;
	char		szDispBuf[_DISP_MSG_SIZE_ + 1] = {0};
	char		szPrtMode[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char   uszKey = 0x00;

	/* 為了恢復被signpad清掉的title */
	inFunc_ResetTitle(pobTran);

	memset(szPrtMode, 0x00, sizeof(szPrtMode));
	inGetPrtMode(szPrtMode);

	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/* 列印商店聯 */
	/* 簽單模式不是0就印商店聯 */
	if (memcmp(szPrtMode, "0", strlen("0")) != 0)
	{
		/* 列印帳單中 */
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

		while (1)
		{
			pobTran->srBRec.inPrintOption = _PRT_MERCH_;
			inRetVal = inSVC_PRINT_ReceiptFlowByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
			{
				/* 成功就砍簽名圖檔 */
				inFunc_Delete_Signature(pobTran);

				break;
			}

		}
	}

	/* 三聯式簽單，中間要多印一聯客戶聯 */
	if (memcmp(szPrtMode, "3", strlen("3")) == 0)
	{
		/* 列印帳單中 */
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

		/* 列印商店存根聯 */
		while (1)
		{
			pobTran->srBRec.inPrintOption = _PRT_MERCH_DUPLICATE_;
			inRetVal = inSVC_PRINT_ReceiptFlowByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
			{
				break;
			}
		}
	}

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		/* SmartPay交易要提示調閱編號FiscRRN */
		inDISP_PutGraphic(_CHECK_SIGNATURE_3_, 0, _COORDINATE_Y_LINE_8_4_);

		memset(szDispBuf, 0x00, sizeof(szDispBuf));
		sprintf(szDispBuf, "%s", pobTran->srBRec.szFiscRRN);
		inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 9);
	}
	else
	{
		if (pobTran->srBRec.uszF56NoSignatureBit == VS_TRUE)
		//if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
		{
			/* 提示免簽名和授權碼 */
			inDISP_PutGraphic(_CHECK_SIGNATURE_5_, 0, _COORDINATE_Y_LINE_8_4_);
		}
		else
		{
			/* 提示檢核簽名和授權碼 */
			inDISP_PutGraphic(_CHECK_SIGNATURE_1_, 0, _COORDINATE_Y_LINE_8_4_);
		}

		memset(szDispBuf, 0x00, sizeof(szDispBuf));
		sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
		inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 11);
	}

	/* 第一聯印完後15秒未按確認或清除鍵則端末機嗶嗶聲提示，響15秒後自動出第二聯 */
	/* 客製化107顯示訊息TimeOut 2秒 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, strlen(_CUSTOMER_INDICATOR_107_BUMPER_)))
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
		inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
	}
	else
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_2_, 30);
		inDISP_Timer_Start(_TIMER_NEXSYS_3_, 15);
	}
	
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		uszKey = uszKBD_Key();

		if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
		{
			inDISP_BEEP(1, 250);
		}

		if (uszKey == _KEY_ENTER_			||
		    uszKey == _KEY_TIMEOUT_			||
		    inChoice == _CUSTReceipt_Touch_ENTER_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (uszKey == _KEY_CANCEL_			||
			 inChoice == _CUSTReceipt_Touch_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	/* 因為這裡交易已完成，所以一定回傳成功*/
	if (inRetVal == VS_USER_CANCEL)
	{
		return (VS_SUCCESS);
	}

        /* 列印顧客聯 */
	/* 三聯式簽單，兩聯以上就要印顧客聯 */
	if (memcmp(szPrtMode, "2", strlen("2")) == 0	||
	    memcmp(szPrtMode, "3", strlen("3")) == 0)
	{
		while (1)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				/* SmartPay交易要提示調閱編號FiscRRN */
				inDISP_PutGraphic(_CHECK_SIGNATURE_4_, 0, _COORDINATE_Y_LINE_8_4_);

				memset(szDispBuf, 0x00, sizeof(szDispBuf));
				sprintf(szDispBuf, "%s", pobTran->srBRec.szFiscRRN);
				inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 9);
			}
			else
			{
				if (pobTran->srBRec.uszF56NoSignatureBit == VS_TRUE)
				//if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
				{
					/* 提示免簽名和授權碼 */
					inDISP_PutGraphic(_CHECK_SIGNATURE_6_, 0, _COORDINATE_Y_LINE_8_4_);
				}
				else
				{
					/* 提示檢核簽名和授權碼 */
					inDISP_PutGraphic(_CHECK_SIGNATURE_2_, 0, _COORDINATE_Y_LINE_8_4_);
				}

				memset(szDispBuf, 0x00, sizeof(szDispBuf));
				sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
				inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 11);
			}

			pobTran->srBRec.inPrintOption = _PRT_CUST_;
			
			inDISP_LogPrintfWithFlag(" Print ByBuffer  Cust RRN [%s]", pobTran->srBRec.szRefNo );
			
			inRetVal = inSVC_PRINT_ReceiptFlowByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
			{
				break;
			}
		}
	}

        return (VS_SUCCESS);
}
