#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "../../INCLUDES/Define_1.h"
#include "../../INCLUDES/Define_2.h"
#include "../../INCLUDES/TransType.h"
#include "../../INCLUDES/Transaction.h"



#include "../../DISPLAY/Display.h"

#include "../CARD_FUNC/CardFunction.h"
#include "../Sqlite.h"
#include "../Accum.h"
#include "../Batch.h"

#include "../CDT.h"
#include "../HDT.h"
#include "../CFGT.h"
#include "../HDPT.h"
#include "../Function.h"

#include "ECR.h"
#include "EcrPackFunc.h"


static char stTransType[4];

static ACCUM_TOTAL_REC	srAccumRec = {0};
static int inAmountReady = VS_ERROR;



int inECR_PACK_GetTransType(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{	
	memset(stTransType, 0x00, sizeof(stTransType));
	inECR_ECROB_GetTransType(stTransType);
	inDISP_LogPrintfWithFlag(" ECR Get TransType[%s] ", stTransType);
	
	return ECR_RL_SUCCESS;
}

int inECR_PACK_Filler(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr, int inSkipLen)
{
	int i;

	for (i =0 ; i < inSkipLen ; i++)
	{
		szPackBuf[i] = 0x20;
	}

	*inPackAddr += inSkipLen;        
	return ECR_RL_SUCCESS;
}

int inECR_PACK_Zero(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr, int inSkipLen)
{
	int i;

	for (i =0 ; i < inSkipLen ; i++)
	{
		szPackBuf[i] = 0x30;
	}

	*inPackAddr += inSkipLen;        
	return ECR_RL_SUCCESS;
}

int inECR_PACK_FillBlankWithoutAddLen(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr, int inSkipLen)
{
	int i;

	for (i =0 ; i < inSkipLen ; i++)
	{
		szPackBuf[i] = 0x20;
	}

	return ECR_RL_SUCCESS;
}

int inECR_PACK_ZeioWithoutAddLen(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr, int inSkipLen)
{
	int i;

	for (i =0 ; i < inSkipLen ; i++)
	{
		szPackBuf[i] = 0x30;
	}

	return ECR_RL_SUCCESS;
}


int inECR_PACK_EcrIndicator(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
        memcpy(szPackBuf, "I", 1);
        *inPackAddr += 1;        
        return ECR_RL_SUCCESS;
}

int inECR_PACK_CtlsIndicator(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE || 
		pobTran->srBRec.uszContactlessBit == VS_TRUE ||
		pobTran->srBRec.uszMobilePayBit == VS_TRUE)
		memcpy(szPackBuf, "Y", 1);
	else
		memcpy(szPackBuf, "N", 1);

	*inPackAddr += 1;        
	return ECR_RL_SUCCESS;
}

int inECR_PACK_VersionDate(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	//memcpy(szPackBuf, &gszTermVersionDate[2], 6);
	memcpy(szPackBuf, "200102", 6);	
	*inPackAddr += 6;
	return ECR_RL_SUCCESS;
}

int inECR_PACK_TransType(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{	
	char szTemplate[4];

	memset(szTemplate, 0x00, sizeof(szTemplate));
	
	inECR_ECROB_GetTransType(szTemplate);
	memcpy(szPackBuf, szTemplate, 2);
	
	*inPackAddr += 2;
	return ECR_RL_SUCCESS;
}

int inECR_PACK_CardTypeIndicator(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	if(pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		memcpy(szPackBuf, "C", 1);
	else
		memcpy(szPackBuf, "N", 1);

	*inPackAddr += 1;        
        return ECR_RL_SUCCESS;
}

int inECR_PACK_SuccessResponseCode(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{	
#ifdef _FUBON_MAIN_HOST_	
	inDISP_LogPrintfWithFlag(" Ecr Pack Suc Code ErrorCode[%d] Kiosk[%u] Line[%d]",inECR_ECROB_GetErrorCode(), pobTran->uszKioskFlag, __LINE__);
	/* 已接近結帳筆數，請先結帳的訊息會出現在成功交易中，所以要新增此判斷 2020/1/30 下午 4:44 [SAM] */
	if(inECR_ECROB_GetErrorCode() == _ECR_RESPONSE_CODE_NEAR_MAX_SETTLE_CNT_ && pobTran->uszKioskFlag == 'Y')
	{
		memcpy(szPackBuf, "0007", 4);
	}else
	{
		memcpy(szPackBuf, "0000", 4);
	}
#else
	memcpy(szPackBuf, "0000", 4);
#endif
	
	*inPackAddr += 4;        
	return ECR_RL_SUCCESS;
}

int inECR_PACK_InvoiceNumber(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char szTemplate[8];
	inDISP_LogPrintfWithFlag(" ECR Inv[%d] ", pobTran->srBRec.lnOrgInvNum);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if(pobTran->srBRec.lnOrgInvNum > 0)
	{
		sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
		memcpy(szPackBuf, szTemplate, 6);
	}else{
		memcpy(szPackBuf, "      ", 6);
	}
	*inPackAddr += 6;

	return ECR_RL_SUCCESS;

}

int inECR_PACK_HostId(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char szTemplate[13];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTRTFileName(szTemplate);
	
	inDISP_LogPrintfWithFlag(" TrtFileName[%s]", szTemplate);
	
	if (!memcmp(szTemplate, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
		 memcpy(szPackBuf, _ECR_7E1_FUBON_HOSTID_, 2);
	else if (!memcmp(szTemplate, _TRT_FILE_NAME_AMEX_, strlen(_TRT_FILE_NAME_AMEX_)))
		 memcpy(szPackBuf, _ECR_7E1_FUBON_HOSTID_AMEX_, 2);
	else if (!memcmp(szTemplate, _TRT_FILE_NAME_DINERS_, strlen(_TRT_FILE_NAME_DINERS_)))
		memcpy(szPackBuf, _ECR_7E1_FUBON_HOSTID_DINERS_, 2);
	else
		memcpy(szPackBuf, "  ", 2);
	
	*inPackAddr += 2;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return ECR_RL_SUCCESS;

}

int inECR_PACK_CardNumber(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char szTemplate[24];
	int inCardLen, i;

	inECR_PACK_FillBlankWithoutAddLen(pobTran, szPackBuf, inPackAddr, 19);

	inDISP_LogPrintfWithFlag(" ECR Card Pan[%s]",pobTran->srBRec.szPAN);
	
	if(strlen(pobTran->srBRec.szPAN) >0) 
	{
		/* 要遮卡號 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, pobTran->srBRec.szPAN);

		if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
		{
			inFunc_FormatPAN_UCARD(szTemplate);
		}

		/* 卡號長度 */
		inCardLen = strlen(szTemplate);

		/* 前6後4為明碼 */
		if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
		{
			for (i = 2; i < (inCardLen - 4); i ++)
				szTemplate[i] = '*';
		}
		else
		{
			for (i = 8; i < (inCardLen - 4); i ++)
				szTemplate[i] = '*';
		}

		memcpy(szPackBuf, szTemplate, strlen(szTemplate));
	}else
	{		
		inECR_PACK_FillBlankWithoutAddLen(pobTran,szPackBuf, inPackAddr, 19);
	}
   
//	memcpy(szPackBuf, pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));

	*inPackAddr += 19;

	return ECR_RL_SUCCESS;

}




int inECR_PACK_ExpireDate(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char szTemplate[7];


	if (strlen(pobTran->srBRec.szExpDate) > 0)
	{
		/* 有效期是否回傳 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetECRExpDateReturnEnable(szTemplate);

	/* 要回傳有效期 */
//	if (!memcmp(&szTemplate, "Y", 1))
//	{
		memcpy(szPackBuf, pobTran->srBRec.szExpDate, 4);
		
//	}else
//	{
//		
//		inECR_PACK_FillBlankWithoutAddLen(pobTran, szPackBuf, inPackAddr, 4);
//	}
	}else
	{		
		inECR_PACK_FillBlankWithoutAddLen(pobTran,szPackBuf, inPackAddr, 4);
	}
	*inPackAddr += 4;

	return ECR_RL_SUCCESS;
}

int inECR_PACK_Reference(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
        
	if (strlen(pobTran->srBRec.szRefNo) > 0)
	{
		memcpy(szPackBuf, pobTran->srBRec.szRefNo, 12);
		
	}else
	{		
		inECR_PACK_FillBlankWithoutAddLen(pobTran,szPackBuf, inPackAddr, 12);
	}
	
	*inPackAddr += 12;
	
	return ECR_RL_SUCCESS;
}


int inECR_PACK_TransDate(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char szTemplate[10];

	if (strlen(pobTran->srBRec.szDate) > 0)
	{
		/* Trans Date */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcat(szTemplate, pobTran->srBRec.szDate);
		memcpy(szPackBuf, &szTemplate[2], 6);
	}else{
		inECR_PACK_FillBlankWithoutAddLen(pobTran,szPackBuf, inPackAddr, 6);
	}
	*inPackAddr += 6;
     
	return ECR_RL_SUCCESS;
}

int inECR_PACK_TransTime(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	if (strlen(pobTran->srBRec.szTime) > 0)
	{
		/* Trans Time */
		memcpy(szPackBuf, pobTran->srBRec.szTime, 6);
	}else{
		inECR_PACK_FillBlankWithoutAddLen(pobTran,szPackBuf, inPackAddr, 6);
	}
	*inPackAddr += 6;
       
        
        return ECR_RL_SUCCESS;
}


int inECR_PACK_ApprovalNumber(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char szTemplate[10];
	
	inECR_PACK_FillBlankWithoutAddLen(pobTran, szPackBuf, inPackAddr, 9);
	
	if (strlen(pobTran->srBRec.szAuthCode) > 0)
	{		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, pobTran->srBRec.szAuthCode);
		memcpy(szPackBuf, szTemplate, 6);
	}

        *inPackAddr += 9;

        return ECR_RL_SUCCESS;
}





int inECR_PACK_TerminalId(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
        char szTemplate[9];
		
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetTerminalID(szTemplate);
	        
        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
        memcpy(szPackBuf, szTemplate, 8);
        *inPackAddr += 8;

        return ECR_RL_SUCCESS;
}

int inECR_PACK_CustomerName(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char szTemplate[42];
	memset(szTemplate, 0x00, sizeof(szTemplate));
	
	if (strlen(pobTran->srBRec.szCardHolder) > 0)
	{
		/* 先存成空白 */
		inECR_PACK_FillBlankWithoutAddLen(pobTran, szPackBuf, inPackAddr, 25);

		sprintf(szTemplate, "%s", pobTran->srBRec.szCardHolder);
		memcpy(szPackBuf, szTemplate, strlen(szTemplate));
	}else{
		inECR_PACK_FillBlankWithoutAddLen(pobTran, szPackBuf, inPackAddr, 25);
	}

	 *inPackAddr += 25;

	return ECR_RL_SUCCESS;
}


int inECR_PACK_CardType(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char szTemplate[22];

	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetCardLabel(szTemplate);
	
	if (!memcmp(szTemplate, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
		memcpy(szPackBuf, _ECR_8N1_NCCC_CARDTYPE_UCARD_, 2);
	else if (!memcmp(szTemplate, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)))
		memcpy(szPackBuf, _ECR_8N1_NCCC_CARDTYPE_VISA_, 2);
	else if (!memcmp(szTemplate, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
		memcpy(szPackBuf, _ECR_8N1_NCCC_CARDTYPE_MASTERCARD_, 2);
	else if (!memcmp(szTemplate, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)))
		memcpy(szPackBuf, _ECR_8N1_NCCC_CARDTYPE_JCB_, 2);
	else if (!memcmp(szTemplate, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
		memcpy(szPackBuf, _ECR_8N1_NCCC_CARDTYPE_AMEX_, 2);
	else if (!memcmp(szTemplate, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_)))
		memcpy(szPackBuf, _ECR_8N1_NCCC_CARDTYPE_CUP_, 2);
	else if (!memcmp(szTemplate, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)))
		memcpy(szPackBuf, _ECR_8N1_NCCC_CARDTYPE_DINERS_, 2);
	else if (!memcmp(szTemplate, _CARD_TYPE_SMARTPAY_, strlen(_CARD_TYPE_SMARTPAY_)))
		memcpy(szPackBuf, _ECR_8N1_NCCC_CARDTYPE_SMARTPAY_, 2);
	else
		memcpy(szPackBuf, "  ", 2);

        *inPackAddr += 2;

        return ECR_RL_SUCCESS;
}


int inECR_PACK_MerchantId(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char szTemplate[20];

	/* 先存成空白 */
	inECR_PACK_FillBlankWithoutAddLen(pobTran, szPackBuf, inPackAddr, 15);	   
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetMerchantID(szTemplate);
	memcpy(szPackBuf, szTemplate, strlen(szTemplate));
	*inPackAddr += 15;

	return ECR_RL_SUCCESS;
}

int inECR_PACK_StoreId(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char szTemplate[20];

	/* 先存成空白 */
	inECR_PACK_FillBlankWithoutAddLen(pobTran, szPackBuf, inPackAddr, 18);	   
	
	if (strlen(pobTran->srBRec.szStoreID) > 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetMerchantID(szTemplate);
		memcpy(szPackBuf, pobTran->srBRec.szStoreID, strlen(pobTran->srBRec.szStoreID));
	}
	
	*inPackAddr += 18;

	return ECR_RL_SUCCESS;
}


int inECR_PACK_BatchNumber(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char szTemplate[7] ;
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetBatchNum(szTemplate);
	memcpy(szPackBuf, szTemplate, 6);
	*inPackAddr += 6;

	return ECR_RL_SUCCESS;
}


int inECR_PACK_InstallmentIndicator(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{

	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		memcpy(szPackBuf, pobTran->srBRec.szInstallmentIndicator, 1);
	}else
	{
		inECR_PACK_FillBlankWithoutAddLen(pobTran, szPackBuf, inPackAddr, 1);
	}

	*inPackAddr += 1;

	return ECR_RL_SUCCESS;
}

int inECR_PACK_InstallmentPeriod(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char szTemplate[3];
	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		sprintf(szTemplate,"%02ld", pobTran->srBRec.lnInstallmentPeriod);
		memcpy(szPackBuf, szTemplate, 2);
	}else
	{
		inECR_PACK_FillBlankWithoutAddLen(pobTran, szPackBuf, inPackAddr, 2);
	}
	*inPackAddr += 2;
	return ECR_RL_SUCCESS;
}

int inECR_PACK_InstallmentDownPaymentAmount(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char szTemplate[13] ;

	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate,"%010ld00", pobTran->srBRec.lnInstallmentDownPayment);
		memcpy(szPackBuf, szTemplate, 12);
	}else
	{
		inECR_PACK_FillBlankWithoutAddLen(pobTran, szPackBuf, inPackAddr, 12);
	}
	*inPackAddr += 12;

	return ECR_RL_SUCCESS;
}



int inECR_PACK_InstallmentAmount(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{

	char szTemplate[13] ;

	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate,"%010ld00", pobTran->srBRec.lnInstallmentPayment);
		memcpy(szPackBuf, szTemplate, 12);
	}else
	{
		inECR_PACK_FillBlankWithoutAddLen(pobTran, szPackBuf, inPackAddr, 12);
	}
	
	*inPackAddr += 12;
	
	return ECR_RL_SUCCESS;
}





int inECR_PACK_InstallmentFormalityFee(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{

	char szTemplate[13] ;

	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate,"%010ld00", pobTran->srBRec.lnInstallmentFormalityFee);
		memcpy(szPackBuf, szTemplate, 12);
	}else
	{
		inECR_PACK_FillBlankWithoutAddLen(pobTran, szPackBuf, inPackAddr, 12);
	}
	
	*inPackAddr += 12;	
	
	return ECR_RL_SUCCESS;
}


int inECR_PACK_RedeemIndicator(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		memcpy(szPackBuf, pobTran->srBRec.szRedeemIndicator, 1);
	}else
	{
		inECR_PACK_FillBlankWithoutAddLen(pobTran, szPackBuf, inPackAddr, 1);
	}

	*inPackAddr += 1;

	return ECR_RL_SUCCESS;
}


int inECR_PACK_RedeemptionPaidAmount(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char szTemplate[13] ;

	if (pobTran->srBRec.uszRedeemBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate,"%010ld00", pobTran->srBRec.lnRedemptionPaidAmount);
		memcpy(szPackBuf, szTemplate, 12);
	}else
	{
		inECR_PACK_FillBlankWithoutAddLen(pobTran, szPackBuf, inPackAddr, 12);
	}
	
	*inPackAddr += 12;	
	
        return ECR_RL_SUCCESS;
}


int inECR_PACK_RedeemptionPoint(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{

	char szTemplate[9] ;

	if (pobTran->srBRec.uszRedeemBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate,"%08ld", pobTran->srBRec.lnRedemptionPoints);
		memcpy(szPackBuf, szTemplate, 8);
	}else
	{
		inECR_PACK_FillBlankWithoutAddLen(pobTran, szPackBuf, inPackAddr, 8);
	}
	
	*inPackAddr += 8;	
	
	return ECR_RL_SUCCESS;
}


int inECR_PACK_RedeemptionBalancePoint(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char szTemplate[9] ;

	if (pobTran->srBRec.uszRedeemBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate,"%08ld", pobTran->srBRec.lnRedemptionPointsBalance);
		memcpy(szPackBuf, szTemplate, 8);
	}else
	{
		inECR_PACK_FillBlankWithoutAddLen(pobTran, szPackBuf, inPackAddr, 8);
	}
	
	*inPackAddr += 8;	
	
	return ECR_RL_SUCCESS;
      
}

int inECR_PACK_RedeemptionAmount(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char szTemplate[13] ;

	if (pobTran->srBRec.uszRedeemBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate,"%010ld00", pobTran->srBRec.lnRedemptionPaidCreditAmount);
		memcpy(szPackBuf, szTemplate, 12);
	}else
	{
		inECR_PACK_ZeioWithoutAddLen(pobTran, szPackBuf, inPackAddr, 8);
	}
	
	*inPackAddr += 12;	
	
        return ECR_RL_SUCCESS;
}

int inECR_PACK_TransAmount(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
        char szTemplate[13];

        memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);
        memcpy(szPackBuf, szTemplate, 12);
        *inPackAddr += 12;
        
        return ECR_RL_SUCCESS;
}


int inECR_PACK_HashCardNumber(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char	szTemplate[100 + 1];
	char	szHash[44 + 1];
	/*
		步驟1. Hash Data = 卡號 (U Card則取11碼卡號進行HASH)
		步驟2. Hash Value(32Bytes) = 以SHA-256演算Hash Data
		步驟3. BASE64 Data(44 Bytes) = 將Hash Value以BASE64進行編碼
		步驟4. 加密卡號(50 Bytes) = BIN N0(6 Bytes) + BASE64 Data(44 Bytes)。
		步驟5. 將加密卡號(50 Bytes)依電文規格指定欄位(37)回傳收銀機。
	*/
	/* Card No. Hash Value (50 Bytes) */
	if(strlen(pobTran->srBRec.szPAN) >0 ) 
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));

		if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
			memcpy(&szTemplate[0], &pobTran->srBRec.szPAN[4], 11);
		else
			memcpy(&szTemplate[0], &pobTran->srBRec.szPAN[0], strlen(pobTran->srBRec.szPAN));

		if (strlen(szTemplate) > 0)
		{
			memcpy(&szPackBuf[0], &szTemplate[0], 6);
			*inPackAddr += 6;
			memset(szHash, 0x00, sizeof(szHash));
			inFunc_CardNumber_Hash(szTemplate, szHash);
			memcpy(&szPackBuf[6], &szHash[0], 44);
			*inPackAddr += 44;
		}
		else
		{
			int i;

			for (i =0 ; i < 50 ; i++)
			{
				szPackBuf[i] = 0x20;
			}
			*inPackAddr += 50;
		}
	}else
	{
		inECR_PACK_Filler(pobTran, szPackBuf, inPackAddr, 50);
	}
        return ECR_RL_SUCCESS;
}




int inECR_PACK_GetTotalAmountRecordFromSettleIso(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAmtTemp)
{
	memset(&srAccumRec, 0x00, sizeof(srAccumRec));
	
	memcpy(&srAccumRec, srAmtTemp, sizeof(srAmtTemp));
	
	inAmountReady = VS_TRUE;
	return (VS_SUCCESS);
}	
	
int inECR_PACK_GetTotalAmountRecord(TRANSACTION_OBJECT *pobTran)
{
	int inBatchCnt, inAccumCnt;
	
	memset(&srAccumRec, 0x00, sizeof(srAccumRec));
	inAmountReady = VS_ERROR;
	
	inBatchCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
	//inBatchCnt = inBATCH_CheckBatchKeyFile(pobTran);
	if (inBatchCnt == VS_ERROR)
	{
		return (VS_ERROR);
	}else if (inBatchCnt == VS_NO_RECORD)
	{
		return (VS_NO_RECORD);
	}
	else
	{
		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		inAccumCnt = inACCUM_GetRecord(pobTran, &srAccumRec);
		if (inAccumCnt == VS_NO_RECORD)
		{
			return (VS_NO_RECORD);
		}
		else if (inAccumCnt == VS_ERROR)
			return (VS_ERROR);
	
		inAmountReady = VS_TRUE;
	}
	
	return (VS_SUCCESS);
}


int inECR_PACK_HostName(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char szTemplate[24];
	int inTemplen;

	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetHostLabel(szTemplate);
		
	inECR_PACK_FillBlankWithoutAddLen(pobTran, szPackBuf, inPackAddr, 10);
	
	inTemplen = strlen(szTemplate);
	
	if(inTemplen > 10)
		inTemplen = 10;
	
	memcpy(szPackBuf, szTemplate, inTemplen);
		
	*inPackAddr += 10;

	return ECR_RL_SUCCESS;
}

int inECR_PACK_SaleTotalCount(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char szTemplate[7];

	if(inAmountReady == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));

		sprintf(szTemplate, "%03lu", srAccumRec.lnTotalSaleCount);
		memcpy(szPackBuf, szTemplate, 3);
	}else
	{
		inECR_PACK_ZeioWithoutAddLen(pobTran, szPackBuf, inPackAddr, 3);
	}

	*inPackAddr += 3;

	return ECR_RL_SUCCESS;
}

int inECR_PACK_SaleTotalAmt(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char szTemplate[13];
	if(inAmountReady == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", ((long)srAccumRec.llTotalSaleAmount + (long)srAccumRec.llTotalTipsAmount));
		strcat(szPackBuf, szTemplate);
	}else{
		inECR_PACK_ZeioWithoutAddLen(pobTran, szPackBuf, inPackAddr, 12);
	}
	*inPackAddr += 12;

	return ECR_RL_SUCCESS;
}


int inECR_PACK_RefundTotalCount(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char szTemplate[7];
	if(inAmountReady == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%03lu", srAccumRec.lnTotalRefundCount);
		memcpy(szPackBuf, szTemplate, 3);
	}else{
		inECR_PACK_ZeioWithoutAddLen(pobTran, szPackBuf, inPackAddr, 3);
	}
		
	*inPackAddr += 3;

	return ECR_RL_SUCCESS;
}

int inECR_PACK_RefundTotalAmt(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char szTemplate[13];
	if(inAmountReady == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", ((long)srAccumRec.llTotalRefundAmount));
		memcpy(szPackBuf, szTemplate, 12);
	}else{
		inECR_PACK_ZeioWithoutAddLen(pobTran, szPackBuf, inPackAddr, 12);
	}
	*inPackAddr += 12;

	return ECR_RL_SUCCESS;
}


/*
 *  
 * 舊規則 : 交易卡號經過 SHA-256後(32碼) , 再轉Base64文字(44碼)
 *   	     於44碼前冠原卡號前6碼 , 組合成加密卡號共50碼(6+44)
 * 
 * 新規則 : 交易卡號經過 SHA-256後(32碼) , 再轉Base64文字(44碼)
 *    	於44碼前冠B00 + 發卡行銀行代碼(3碼) , 組合成加密卡號共50碼(6+44)
 * EDC 有開啟ECR功能且交易確實是由ECR管道進到EDC且交易別不為結帳(表示其餘交易別皆需要計算載具加密字串) 
 * 則request的交易電文須於F_59 帶出新增之Table ID B1 (放置3碼0x20)
 * 讓主機辨識出EDC需要交易卡號之發卡銀行代碼(Bank id) ,
 * 則主機 於 response 電文F_59 Table ID B1放入交易卡號之發卡銀行代碼(Bank id)
 * EDC 使用 Table ID B1資料計算新規則之載具加密字串並回傳收銀機。
 * 2022/10/18 [SAM]
 */
int inECR_PACK_NewHashCardNumber(TRANSACTION_OBJECT *pobTran, char* szPackBuf, int* inPackAddr)
{
	char	szHeaderTemp[24 + 1];
	char	szCardNumTemp[24 + 1];
	char	szHash[44 + 1];
	
	memset(szHeaderTemp, 0x00, sizeof(szHeaderTemp));
	memset(szCardNumTemp, 0x00, sizeof(szCardNumTemp));
	/* Card No. Hash Value (50 Bytes) */
	if (!memcmp(pobTran->srBRec.szEI_BankId,"NNN", 3 ) ||
		pobTran->srBRec.szEI_BankId[0]  == 0x00)
	{		
		inECR_PACK_Filler(pobTran, szPackBuf, inPackAddr, 50);
		inDISP_LogPrintfWithFlag(" Pack New Hash szEI_BankId = NNN Line[%d]", __LINE__);
		return ECR_RL_SUCCESS;
	}
	
	if(pobTran->srBRec.inCode != _SALE_ &&
		pobTran->srBRec.inCode != _REDEEM_SALE_ && 
		pobTran->srBRec.inCode != _INST_SALE_ &&
		pobTran->srBRec.inCode != _CUP_SALE_ )
	{		
		inECR_PACK_Filler(pobTran, szPackBuf, inPackAddr, 50);
		inDISP_LogPrintfWithFlag(" Pack New Hash Incode[%d] *Error* Line[%d]", pobTran->srBRec.inCode, __LINE__);
		return ECR_RL_SUCCESS;
	}
	
	memcpy(&szHeaderTemp[0],"B00", 3);
	memcpy(&szHeaderTemp[3], pobTran->srBRec.szEI_BankId, 3);

	if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
		memcpy(&szCardNumTemp[0], &pobTran->srBRec.szPAN[4], 11);
	else
		memcpy(&szCardNumTemp[0], &pobTran->srBRec.szPAN[0], strlen(pobTran->srBRec.szPAN));

	if (strlen(szCardNumTemp) > 0)
	{
		memcpy(&szPackBuf[0], &szHeaderTemp[0], 6);
		*inPackAddr += 6;
		memset(szHash, 0x00, sizeof(szHash));
		inFunc_CardNumber_Hash(szCardNumTemp, szHash);
		memcpy(&szPackBuf[6], &szHash[0], 44);
		*inPackAddr += 44;
	}
	else
	{
		inECR_PACK_Filler(pobTran, szPackBuf, inPackAddr, 50);
		inDISP_LogPrintfWithFlag(" Pack New Hash Card Nub[%x] *Error* Line[%d]", pobTran->srBRec.szPAN[0], __LINE__);
	}
	
	return ECR_RL_SUCCESS;
}


