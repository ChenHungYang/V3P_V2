
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <ctosapi.h>
#include <sqlite3.h>

#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/TransType.h"

#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/Card.h"
#include "../SOURCE/DISPLAY/Display.h"

#include "FUBONfunc.h"
#include "FUBONTablePack.h"


/*
 * Function        : inFUBON_PackTableN1
 * Date&Time   : 2016/12/30 下午 1:38
 * Describe        :
 * Table Id : N1 
 * 非銀聯預先授權完成相關交易(pre auth com, revrsal pre auth com , void pre auth com,
 * reversal void pre auth com , batch upload pre auth com)須帶此Table id N1(原預先授權交易相關資訊)給主機
 */
int inFUBON_PackTableN1(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
        int inAuthCodeLen;
        int inSubPackLenAddr,inPackTemp;
        int inFixTreansDateLen = 8, inFixOrgAmtLen = 12, inFixAuthCodeLen = 6;
        char szTemp[32] = {0};
        inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

        inPackTemp = 0;

        memcpy(&uszPackBuf[inPackTemp], "N1", 2);
        inPackTemp += 2;
        /* keep for Sub Lenth  */
        inSubPackLenAddr = inPackTemp;
        inPackTemp++;
	
        /* Ori Pre-Auth Trans Date [an8] */
        memcpy(&uszPackBuf[inPackTemp], pobTran->srBRec.szTableTD_Data, inFixTreansDateLen);
        inPackTemp += inFixTreansDateLen;
        inDISP_LogPrintfWithFlag(" TD-DATA [%s] Line[%d] ",pobTran->srBRec.szTableTD_Data,  __LINE__);

        
        /* Ori Pre-Auth Amount [Num 12] */
        sprintf(szTemp,"%010ld00", pobTran->srBRec.lnOrgTxnAmount);
        inDISP_LogPrintfWithFlag(" OrgAmt  [%s] Line[%d] ", szTemp,  __LINE__);
        memcpy(&uszPackBuf[inPackTemp], szTemp, inFixOrgAmtLen);
        inPackTemp += inFixOrgAmtLen;
    
        /* Ori Pre-Auth Trans Approval code [an 6] 
         * 固定長度 , 不足6碼或未輸入授權碼，補0x20至滿6碼。
         */
        inAuthCodeLen = strlen(pobTran->srBRec.szAuthCode);
        if(inAuthCodeLen < inFixAuthCodeLen)
        {
                memcpy(&uszPackBuf[inPackTemp], pobTran->srBRec.szAuthCode, inAuthCodeLen);
                inPackTemp += inAuthCodeLen;	
                strncpy((char*)&uszPackBuf[inPackTemp], "      ", (inFixAuthCodeLen - inAuthCodeLen));
                inPackTemp += (inFixAuthCodeLen - inAuthCodeLen);		
                inDISP_LogPrintfWithFlag(" AuthCode[%s] Line[%d] ", pobTran->srBRec.szAuthCode,  __LINE__);
        }
        else
        {
                memcpy(&uszPackBuf[inPackTemp], pobTran->srBRec.szAuthCode, inFixAuthCodeLen);
                inDISP_LogPrintfWithFlag(" AuthCode  [%s] Line[%d] ", pobTran->srBRec.szAuthCode,  __LINE__);
                inPackTemp += inFixAuthCodeLen;
        }
        
                
        /* Sub Lenth */
        uszPackBuf[inSubPackLenAddr] = inPackTemp - 3;

        *inPackAddr += inPackTemp;

        inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
        return VS_SUCCESS;
}

/*
 * Function        : inFUBON_PackTableB1
 * Date&Time   : 2022/10/14
 * Describe        :
 * Table Id : B1 
 */
int inFUBON_PackTableB1(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int inSubPackLenAddr,inPackTemp;
	int inFixTreansDateLen = 3;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inPackTemp = 0;

	memcpy(&uszPackBuf[inPackTemp], "B1", 2);
	inPackTemp += 2;
	/* keep for Sub Lenth  */
	inSubPackLenAddr = inPackTemp;
	inPackTemp++;

	/* BANK ID [an3] */
	memcpy(&uszPackBuf[inPackTemp], "   ", inFixTreansDateLen);
	inPackTemp += inFixTreansDateLen;

	/* Sub Lenth */
	uszPackBuf[inSubPackLenAddr] = inPackTemp - 3;
	*inPackAddr += inPackTemp;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}

/*
 * Function        : inFUBON_PackTableC1_SendInitialData
 * Date&Time   : 2022/10/7 上午 11:17
 * Describe        :
 * Table Id :  
 */
int inFUBON_PackTableC1_SendInitialData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int inSubPackLenAddr,inPackTemp;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inPackTemp = 0;
	
	/* Table ID */
	memcpy(&uszPackBuf[inPackTemp], "C1", 2);
	inPackTemp += 2;
	
	/* CUP Length */
	/* keep for Sub Lenth  */
	inSubPackLenAddr = inPackTemp;
	inPackTemp++;

	/* CUP Tx */
	memcpy(&uszPackBuf[inPackTemp], "1", 1);
	inPackTemp += 1;

	memset(&uszPackBuf[inPackTemp], 0x20, 20);
	inPackTemp += 20;
	
	/* Sub Lenth */
	uszPackBuf[inSubPackLenAddr] = inPackTemp - 3;
	*inPackAddr += inPackTemp;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}


/*
 * Function        : inFUBON_PackTableC1_SendReceivedData
 * Date&Time   : 
 * Describe        :
 * Table Id : B1 
 */
int inFUBON_PackTableC1_SendReceivedData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int inSubPackLenAddr,inPackTemp;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
		
	inPackTemp = 0;
	
	/* Table ID */
	memcpy(&uszPackBuf[inPackTemp], "C1", 2);
	inPackTemp += 2;
	
	/* CUP Length */
	/* keep for Sub Lenth  */
	inSubPackLenAddr = inPackTemp;
	inPackTemp++;

	/* CUP Tx */
	memcpy(&uszPackBuf[inPackTemp], "1", 1);
	inPackTemp += 1;
	/* CUP Serial Number */
	memcpy(&uszPackBuf[inPackTemp], pobTran->srBRec.szCUP_TN, 6);
	inPackTemp += 6;
	/* CUP Settlement Date(MMDD) */
	memcpy(&uszPackBuf[inPackTemp], pobTran->srBRec.szCUP_STD, 4);
	inPackTemp += 4;
	/* CUP TX Date(MMDD) */
	memcpy(&uszPackBuf[inPackTemp], pobTran->srBRec.szCUP_TD, 4);
	inPackTemp += 4;
	/* CUP TX Time(HHMMSS) */
	memcpy(&uszPackBuf[inPackTemp], pobTran->srBRec.szCUP_TT, 6);
	inPackTemp += 6;
	
	/* Sub Lenth */
	uszPackBuf[inSubPackLenAddr] = inPackTemp - 3;
	*inPackAddr += inPackTemp;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}


/*
 * Function        : inFUBON_PackTableC1_SendReceivedData
 * Date&Time   : 
 * Describe        :
 * Table Id : B1 
 */
int inFUBON_PackTableC1_ReversalSendReceivedData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int inSubPackLenAddr,inPackTemp;
	char CupTemp[10];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
		
	inPackTemp = 0;
	
	/* Table ID */
	memcpy(&uszPackBuf[inPackTemp], "C1", 2);
	inPackTemp += 2;
	
	/* CUP Length */
	/* keep for Sub Lenth  */
	inSubPackLenAddr = inPackTemp;
	inPackTemp++;

	/* CUP Tx */
	memcpy(&uszPackBuf[inPackTemp], "1", 1);
	inPackTemp += 1;
	/* CUP Serial Number */
	memset(CupTemp, 0x00, sizeof(CupTemp));
	memcpy(CupTemp, pobTran->srBRec.szCUP_TN, 6);
	if (strlen(CupTemp) > 0)
	{
		memcpy(&uszPackBuf[inPackTemp], pobTran->srBRec.szCUP_TN, 6);
	} else
	{
		memset(&uszPackBuf[inPackTemp], 0x20, 6);
	}

	/* CUP Settlement Date(MMDD) */
	memset(CupTemp, 0x00, sizeof(CupTemp));
	memcpy(CupTemp, pobTran->srBRec.szCUP_STD, 6);
	if (strlen(CupTemp) > 0)
	{
		memcpy(&uszPackBuf[inPackTemp], pobTran->srBRec.szCUP_STD, 6);
	} else
	{
		memset(&uszPackBuf[inPackTemp], 0x20, 6);
	}

	/* CUP TX Date(MMDD) */
	memset(CupTemp, 0x00, sizeof(CupTemp));
	memcpy(CupTemp, pobTran->srBRec.szCUP_TD, 6);
	if (strlen(CupTemp) > 0)
	{
		memcpy(&uszPackBuf[inPackTemp], pobTran->srBRec.szCUP_TD, 6);
	} else
	{
		memset(&uszPackBuf[inPackTemp], 0x20, 6);
	}

	/* CUP TX Time(HHMMSS) */
	memset(CupTemp, 0x00, sizeof(CupTemp));
	memcpy(CupTemp, pobTran->srBRec.szCUP_TT, 6);
	if (strlen(CupTemp) > 0)
	{
		memcpy(&uszPackBuf[inPackTemp], pobTran->srBRec.szCUP_TT, 6);
	} else
	{
		memset(&uszPackBuf[inPackTemp], 0x20, 6);
	}
	
	/* Sub Lenth */
	uszPackBuf[inSubPackLenAddr] = inPackTemp - 3;
	*inPackAddr += inPackTemp;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}

/*
 * Function        : inFUBON_PackTableP1
 * Date&Time   : 2022/10/7 上午 11:17
 * Describe        :
 * Table Id :  
 */
int inFUBON_PackTableP1(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int inSubPackLenAddr,inPackTemp;
	char szPinPadSnBuf[20];


	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
		
	inPackTemp = 0;
	
	/* Table ID */
	memcpy(&uszPackBuf[inPackTemp], "P1", 2);
	inPackTemp += 2;

	/* 抓取機器序號 */
	memset(szPinPadSnBuf, 0x00, sizeof (szPinPadSnBuf));
	inFunc_GetSeriaNumber(szPinPadSnBuf);
	inDISP_LogPrintfWithFlag(" szPinPadSnBuf(%d) START 2,%s!!", strlen(szPinPadSnBuf), szPinPadSnBuf);

	/* keep for Sub Lenth  */
	inSubPackLenAddr = inPackTemp;
	inPackTemp++;

	memcpy(&uszPackBuf[inPackTemp], szPinPadSnBuf, strlen(szPinPadSnBuf) - 1);
	inPackTemp += strlen(szPinPadSnBuf) - 1;
	
	/* Sub Lenth */
	uszPackBuf[inSubPackLenAddr] = inPackTemp - 3;
	*inPackAddr += inPackTemp;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}


/*
 * Function        : inFUBON_PackTableP4
 * Date&Time   : 2022/10/7 上午 11:17
 * Describe        :
 * Table Id :  
 */
int inFUBON_PackTableP4(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int inSubPackLenAddr,inPackTemp;
	char EncrptyCmk[16 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inPackTemp = 0;
		
	/* Table ID */
	memcpy(&uszPackBuf[inPackTemp], "P4", 2);
	inPackTemp += 2;

	/* keep for Sub Lenth  */
	inSubPackLenAddr = inPackTemp;
	inPackTemp++;

	memset(EncrptyCmk, 0x00, sizeof (EncrptyCmk));

	if (inFUBON_CUP_FuncGetCMK(pobTran, &EncrptyCmk[0]) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" Get CMK *Error*l Line[%d] ", __LINE__);
		return (VS_ERROR);
	}

	memcpy(&uszPackBuf[inPackTemp], EncrptyCmk, 16);
	inPackTemp += 16;
	
	/* Sub Lenth */
	uszPackBuf[inSubPackLenAddr] = inPackTemp - 3;
	*inPackAddr += inPackTemp;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}

/*
 * Function        : inFUBON_PackTableA1
 * Date&Time   : 2022/10/7 上午 11:17
 * Describe        :
 * Table Id :  
 */
int inFUBON_PackTableA1(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int inSubPackLenAddr,inPackTemp;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inPackTemp = 0;

	/* Table ID */
	memcpy(&uszPackBuf[inPackTemp], "A1", 2);
	inPackTemp += 2;

	/* DateTime Length */
	/* keep for Sub Lenth  */
	inSubPackLenAddr = inPackTemp;
	inPackTemp++;

	/* DateTime Value */
	memcpy(&uszPackBuf[inPackTemp], pobTran->srBRec.szDate, 8);
	inPackTemp += 8;
	memcpy(&uszPackBuf[inPackTemp], pobTran->srBRec.szTime, 6);
	inPackTemp += 6;
	
	/* Sub Lenth */
	uszPackBuf[inSubPackLenAddr] = inPackTemp - 3;
	*inPackAddr += inPackTemp;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}


/*
 * Function        : inFUBON_PackTableA2
 * Date&Time   : 2022/10/7 上午 11:17
 * Describe        :
 * Table Id :  
 */
int inFUBON_PackTableA2(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int inSubPackLenAddr,inPackTemp;
	char szSTAN[6 + 1];
	char szRRNTemp[12 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inPackTemp = 0;

	/* Table ID */
	memcpy(&uszPackBuf[inPackTemp], "A2", 2);
	inPackTemp += 2;
	
	/* keep for Sub Lenth  */
	inSubPackLenAddr = inPackTemp;
	inPackTemp++;
	
	/* Original System trace No */
	memset(szSTAN, 0x00, sizeof (szSTAN));
	sprintf(szSTAN, "%06ld", pobTran->srBRec.lnSTANNum);
	memcpy(&uszPackBuf[inPackTemp], szSTAN, 6);
	inPackTemp += 6;
	
	/* Original TX EDC Date Time */
	memcpy(&uszPackBuf[inPackTemp], pobTran->srBRec.szDate, 8);
	inPackTemp += 8;
	memcpy(&uszPackBuf[inPackTemp], pobTran->srBRec.szTime, 6);
	inPackTemp += 6;

	/* Original TX Ref No */
	memset(szRRNTemp, 0x00, sizeof (szRRNTemp));
	memcpy(szRRNTemp, pobTran->srBRec.szRefNo, 12);
	if (strlen(szRRNTemp) > 0)
	{
		memcpy(&uszPackBuf[inPackTemp], pobTran->srBRec.szRefNo, 12);
	} else
	{
		memset(&uszPackBuf[inPackTemp], 0x20, 12);
	}
	inPackTemp += 12;
		
	/* Sub Lenth */
	uszPackBuf[inSubPackLenAddr] = inPackTemp - 3;
	*inPackAddr += inPackTemp;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}


