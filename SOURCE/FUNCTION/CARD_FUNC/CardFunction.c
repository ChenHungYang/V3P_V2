


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>

#include "../../../SOURCE/INCLUDES/Define_1.h"
#include "../../../SOURCE/INCLUDES/Define_2.h"
#include "../../../SOURCE/INCLUDES/TransType.h"
#include "../../../SOURCE/INCLUDES/Transaction.h"
#include "../../../SOURCE/DISPLAY/Display.h"
#include "../../../SOURCE/DISPLAY/DispMsg.h"
#include "../../../SOURCE/DISPLAY/DisTouch.h"

#include "../Function.h"
#include "../CFGT.h"
#include "../CDT.h"
#include "../HDT.h"
#include "../Card.h"


/*
Function        : inFunc_FormatPAN_UCARD
Date&Time   : 2019/03/08
Describe        :
 * 
*/
int inFunc_FormatPAN_UCARD(char *szPAN)
{
	char	szTemplate[_PAN_UCARD_SIZE_ + 1];

	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &szPAN[4], 2);
	memcpy(&szTemplate[2], "-", 1);
	memcpy(&szTemplate[3], &szPAN[6], 7);
	memcpy(&szTemplate[10], "-", 1);
	memcpy(&szTemplate[11], &szPAN[13], 2);

	memset(szPAN, 0x00, sizeof(szPAN));
	strcpy(szPAN, szTemplate);

	return (VS_SUCCESS);
}


/*
Function        :inFunc_CardNumber_Hash
Date&Time   : 2019/03/08
Describe        :
*/
int inFunc_CardNumber_Hash(char *szInputData, char *szOutputData)
{
        int		inRetVal;
        char		szOutput[50];
        unsigned char	uszHash[20 + 1];
        unsigned char	uszOutputHash[32 + 1];
        unsigned long	ulnHashSizes = 0;

        memset(uszHash, 0x00, sizeof(uszHash));
        memcpy(uszHash, szInputData, strlen(szInputData));
        ulnHashSizes += strlen((char*)uszHash);

        /* SHA256加密 : 16 => 32 */
        memset(uszOutputHash, 0x00, sizeof(uszOutputHash));
        inFunc_SHA256(uszHash, ulnHashSizes, uszOutputHash);

        /* Base64加密 : 32 => 44 */
        memset(szOutput, 0x00, sizeof(szOutput));
        inRetVal = inFunc_Base64_Encryption((char *)uszOutputHash, 32, szOutput);

        memcpy(&szOutputData[0], &szOutput[0], strlen(szOutput));

        return (VS_SUCCESS);
}


/*
Function        : inCARDFUNC_GetTransactionNoFromPANByNcccRule
Date&Time   : 2019/03/13
Describe        : 利用TID運算出卡號的Transaction Number 儲存至 srBRec.szTxNo
*/
int inCARDFUNC_GetTransactionNoFromPANByNcccRule(TRANSACTION_OBJECT *pobTran)
{
	int	i, j, inTempCard, inTempKey;
	char	szTID[8 + 1];
	char	szTemplate[24 + 1];
	char	szKeyNo1[8 + 1], szKeyNo2[8 + 1];
	char	szPAN[20 + 1], szTxNo[24 + 1] ,szPANTemp[24 + 1];
	char	szFuncEnable[2 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 商店自存聯卡號遮掩開關初始化 */
	pobTran->srBRec.uszTxNoCheckBit = VS_FALSE;

	/* config2 StoreStubCardNoTruncateEnable未開啟 */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetStore_Stub_CardNo_Truncate_Enable(szFuncEnable);
	if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0)
	{
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Function Not Open END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}

	/* 沒有卡號 */
	if (strlen(pobTran->srBRec.szPAN) == 0)
	{
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] No PAN END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}

#ifdef _TX_CHECK_NO_	
	/* PrintTxNoCheckNo開關判斷 */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetPrint_Tx_No_Check_No(szFuncEnable);
	if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
	{
		pobTran->srBRec.uszTxNoCheckBit = VS_TRUE;
	}
	else
	{
		pobTran->srBRec.uszTxNoCheckBit = VS_FALSE;
		return (VS_SUCCESS);
	}
#else
	pobTran->srBRec.uszTxNoCheckBit = VS_FALSE;
	return (VS_SUCCESS);
#endif
	/* Get Terminal ID */
	memset(szTID, 0x00, sizeof(szTID));
	inGetTerminalID(szTID);

	/* 鍵值一：用10減端末機代號後4碼，連同原端末機代號後4碼，得到鍵值一，舉例說明：原端末機代號為13994017
		   10  10  10  10
		   -   -   -   -
		   4   0   1   7   4   0   1   7
		 = 6   0   9   3   4   0   1   7
	*/

	/* 鍵值一第1碼 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szTID[4], 1);
	inTempKey = atoi(szTemplate);
	if (inTempKey > 0)
		inTempKey = 10 - inTempKey;
	memset(szKeyNo1, 0x00, sizeof(szKeyNo1));
	sprintf(szKeyNo1, "%d", inTempKey);

	/* 鍵值一第2碼 */
	memcpy(&szTemplate[0], &szTID[5], 1);
	inTempKey = atoi(szTemplate);
	if (inTempKey > 0)
		inTempKey = 10 - inTempKey;
	sprintf(&szKeyNo1[1], "%d", inTempKey);

	/* 鍵值一第3碼 */
	memcpy(&szTemplate[0], &szTID[6], 1);
	inTempKey = atoi(szTemplate);
	if (inTempKey > 0)
		inTempKey = 10 - inTempKey;
	sprintf(&szKeyNo1[2], "%d", inTempKey);

	/* 鍵值一第4碼 */
	memcpy(&szTemplate[0], &szTID[7], 1);
	inTempKey = atoi(szTemplate);
	if (inTempKey > 0)
		inTempKey = 10 - inTempKey;
	sprintf(&szKeyNo1[3], "%d", inTempKey);

	/* 鍵值一第5~8碼 */
	memcpy(&szKeyNo1[4], &szTID[4], 4);

	/* 鍵值二：用鍵值一每個數值加上本身的數值後取個位數
		   6   0   9   3   4   0   1   7
		   +   +   +   +   +   +   +   +
		   6   0   9   3   4   0   1   7
		 = 2   0   8   6   8   0   2   4
	*/

	memset(szKeyNo2, 0x00, sizeof(szKeyNo2));

	/* 鍵值二第1~8碼 */
	for ( i = 0 ; i < 8 ; i++)
	{
		memcpy(&szTemplate[0], &szKeyNo1[i], 1);
		inTempKey = atoi(szTemplate);
		inTempKey = inTempKey + inTempKey;

		if (inTempKey >= 10)
			inTempKey = inTempKey - 10;

		sprintf(&szKeyNo2[i], "%d", inTempKey);
	}

	/* 1.卡號當被加數/被減數
	   2.鍵值一、鍵值二當加數/減數，並依卡號長度決定此加數/減數共幾位數，各數值說明如下：
		第一位數為鍵值二第3碼
		第二位數為鍵值一第5碼
		第三位數為鍵值二第7碼
		第四位數為鍵值一第3碼
		第五位數為鍵值二第1碼
		第六位數為鍵值一第6碼
		第七位數為鍵值二第2碼
		第八位數為鍵值一第2碼
		第九位數為鍵值二第5碼
		第十位數為鍵值一第7碼
		第十一位數為鍵值二第8碼
		第十二位數為鍵值一第4碼
		第十三位數為鍵值二第4碼
		第十四位數為鍵值一第8碼
		第十五位數為鍵值二第6碼
		第十六位數為鍵值一第1碼
		第十七位數為鍵值二第3碼
		第十八位數為鍵值一第5碼
		第十九位數為鍵值二第7碼
	   3.奇數位採相加，偶數位採相減，被減數若小於減數，則被減數先加10再和減數相減；
	     反之，還原時，偶採數位採相加，奇數位採相減，被減數若小於減數，則被減數先加10再和減數相減
	*/

	/* 取得卡號 */
	memset(szPANTemp, 0x00, sizeof(szPANTemp));
	if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)	/* U Card 11 碼 */
		memcpy(&szPANTemp[0], &pobTran->srBRec.szPAN[4], 11);
	else if (!memcmp(&pobTran->srBRec.szPAN[0], "000000", 6))			/* 大高 10 碼 */
		memcpy(&szPANTemp[0], &pobTran->srBRec.szPAN[6], 10);
	else
		strcpy(szPANTemp, pobTran->srBRec.szPAN);

	j = strlen(szPANTemp); /* 卡號長度 */

	memset(szPAN, 0x00, sizeof(szPAN)); /* 儲存卡號鍵值運算數值 */

	for (i = 0; i < j ; i++)
	{
		switch (i)
		{
			case 0:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szPANTemp[i], 1);
				inTempCard = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo2[2], 1);
				inTempKey = atoi(szTemplate);

				inTempCard = inTempCard + inTempKey;

				if (inTempCard >= 10)
					inTempCard = inTempCard - 10;

				sprintf(&szPAN[0], "%d", inTempCard);

				break;
			case 1:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szPANTemp[i], 1);
				inTempCard = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo1[4], 1);
				inTempKey = atoi(szTemplate);

				if (inTempCard < inTempKey)
					inTempCard = inTempCard + 10;

				inTempCard = inTempCard - inTempKey;

				sprintf(&szPAN[1], "%d", inTempCard);

				break;
			case 2:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szPANTemp[i], 1);
				inTempCard = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo2[6], 1);
				inTempKey = atoi(szTemplate);

				inTempCard = inTempCard + inTempKey;

				if (inTempCard >= 10)
					inTempCard = inTempCard - 10;

				sprintf(&szPAN[2], "%d", inTempCard);

				break;

			case 3:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szPANTemp[i], 1);
				inTempCard = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo1[2], 1);
				inTempKey = atoi(szTemplate);

				if (inTempCard < inTempKey)
					inTempCard = inTempCard + 10;

				inTempCard = inTempCard - inTempKey;

				sprintf(&szPAN[3], "%d", inTempCard);

				break;
			case 4:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szPANTemp[i], 1);
				inTempCard = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo2[0], 1);
				inTempKey = atoi(szTemplate);

				inTempCard = inTempCard + inTempKey;

				if (inTempCard >= 10)
					inTempCard = inTempCard - 10;

				sprintf(&szPAN[4], "%d", inTempCard);

				break;
			case 5:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szPANTemp[i], 1);
				inTempCard = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo1[5], 1);
				inTempKey = atoi(szTemplate);

				if (inTempCard < inTempKey)
					inTempCard = inTempCard + 10;

				inTempCard = inTempCard - inTempKey;

				sprintf(&szPAN[5], "%d", inTempCard);

				break;
			case 6:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szPANTemp[i], 1);
				inTempCard = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo2[1], 1);
				inTempKey = atoi(szTemplate);

				inTempCard = inTempCard + inTempKey;

				if (inTempCard >= 10)
					inTempCard = inTempCard - 10;

				sprintf(&szPAN[6], "%d", inTempCard);

				break;
			case 7:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szPANTemp[i], 1);
				inTempCard = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo1[1], 1);
				inTempKey = atoi(szTemplate);

				if (inTempCard < inTempKey)
					inTempCard = inTempCard + 10;

				inTempCard = inTempCard - inTempKey;

				sprintf(&szPAN[7], "%d", inTempCard);

				break;
			case 8:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szPANTemp[i], 1);
				inTempCard = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo2[4], 1);
				inTempKey = atoi(szTemplate);

				inTempCard = inTempCard + inTempKey;

				if (inTempCard >= 10)
					inTempCard = inTempCard - 10;

				sprintf(&szPAN[8], "%d", inTempCard);

				break;
			case 9:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szPANTemp[i], 1);
				inTempCard = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo1[6], 1);
				inTempKey = atoi(szTemplate);

				if (inTempCard < inTempKey)
					inTempCard = inTempCard + 10;

				inTempCard = inTempCard - inTempKey;

				sprintf(&szPAN[9], "%d", inTempCard);

				break;
			case 10:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szPANTemp[i], 1);
				inTempCard = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo2[7], 1);
				inTempKey = atoi(szTemplate);

				inTempCard = inTempCard + inTempKey;

				if (inTempCard >= 10)
					inTempCard = inTempCard - 10;

				sprintf(&szPAN[10], "%d", inTempCard);

				break;
			case 11:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szPANTemp[i], 1);
				inTempCard = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo1[3], 1);
				inTempKey = atoi(szTemplate);

				if (inTempCard < inTempKey)
					inTempCard = inTempCard + 10;

				inTempCard = inTempCard - inTempKey;

				sprintf(&szPAN[11], "%d", inTempCard);

				break;
			case 12:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szPANTemp[i], 1);
				inTempCard = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo2[3], 1);
				inTempKey = atoi(szTemplate);

				inTempCard = inTempCard + inTempKey;

				if (inTempCard >= 10)
					inTempCard = inTempCard - 10;

				sprintf(&szPAN[12], "%d", inTempCard);

				break;
			case 13:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szPANTemp[i], 1);
				inTempCard = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo1[7], 1);
				inTempKey = atoi(szTemplate);

				if (inTempCard < inTempKey)
					inTempCard = inTempCard + 10;

				inTempCard = inTempCard - inTempKey;

				sprintf(&szPAN[13], "%d", inTempCard);

				break;
			case 14:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szPANTemp[i], 1);
				inTempCard = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo2[5], 1);
				inTempKey = atoi(szTemplate);

				inTempCard = inTempCard + inTempKey;

				if (inTempCard >= 10)
					inTempCard = inTempCard - 10;

				sprintf(&szPAN[14], "%d", inTempCard);

				break;
			case 15:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szPANTemp[i], 1);
				inTempCard = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo1[0], 1);
				inTempKey = atoi(szTemplate);

				if (inTempCard < inTempKey)
					inTempCard = inTempCard + 10;

				inTempCard = inTempCard - inTempKey;

				sprintf(&szPAN[15], "%d", inTempCard);

				break;
			case 16:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szPANTemp[i], 1);
				inTempCard = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo2[2], 1);
				inTempKey = atoi(szTemplate);

				inTempCard = inTempCard + inTempKey;

				if (inTempCard >= 10)
					inTempCard = inTempCard - 10;

				sprintf(&szPAN[16], "%d", inTempCard);

				break;
			case 17:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szPANTemp[i], 1);
				inTempCard = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo1[4], 1);
				inTempKey = atoi(szTemplate);

				if (inTempCard < inTempKey)
					inTempCard = inTempCard + 10;

				inTempCard = inTempCard - inTempKey;

				sprintf(&szPAN[17], "%d", inTempCard);

				break;
			case 18:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szPANTemp[i], 1);
				inTempCard = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo2[6], 1);
				inTempKey = atoi(szTemplate);

				inTempCard = inTempCard + inTempKey;

				if (inTempCard >= 10)
					inTempCard = inTempCard - 10;

				sprintf(&szPAN[18], "%d", inTempCard);

				break;
			default:
				return (VS_ERROR);

		} /* end switch 卡號和鍵值運算 */
	} /* end for */


	/* 交易編號：以上計算的結果再將原端末機代號的後4碼值放入第3、5、7、9 byte中，故交易編號值將比卡號長度多4碼；
	   還原時，則將交易編號的第3、6、9、12 byte值截取出來當端末機代號後4碼來做還原的計算，扣除該4 bytes取得卡號還原值
	*/
	memset(szTxNo, 0x00, sizeof(szTxNo));

	memcpy(&szTxNo[0], &szPAN[0], 2);
	memcpy(&szTxNo[2], &szTID[4], 1);

	memcpy(&szTxNo[3], &szPAN[2], 2);
	memcpy(&szTxNo[5], &szTID[5], 1);

	memcpy(&szTxNo[6], &szPAN[4], 2);
	memcpy(&szTxNo[8], &szTID[6], 1);

	memcpy(&szTxNo[9], &szPAN[6], 2);
	memcpy(&szTxNo[11], &szTID[7], 1);

	j = j - 8; /* 卡號前八碼已填入 */
	memcpy(&szTxNo[12], &szPAN[8], j);

	/* 將Transaction Number 存入srBRec */
	memset(pobTran->srBRec.szTxnNo, 0x00, sizeof(pobTran->srBRec.szTxnNo));
	strcpy(pobTran->srBRec.szTxnNo, szTxNo);

	inDISP_LogPrintfWithFlag("TRANSACTION NUMBER = (%s)", pobTran->srBRec.szTxnNo);
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}



/*
Function        : inCARDFUNC_GetPANFromTransactionNo
Date&Time   : 2019/03/13
Describe        :
*/
int inCARDFUNC_GetPANFromTransactionNo(TRANSACTION_OBJECT *pobTran)
{
	int	i, j, inTempTxNo, inTempKey;
	char	szTemplate[24 + 1];
	char	szKeyNo1[8 + 1], szKeyNo2[8 + 1];
	char	szPAN[20 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 鍵值一第1碼 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[2], 1);
	inTempKey = atoi(szTemplate);
	if (inTempKey > 0)
		inTempKey = 10 - inTempKey;
	memset(szKeyNo1, 0x00, sizeof(szKeyNo1));
	sprintf(szKeyNo1, "%d", inTempKey);

	/* 鍵值一第2碼 */
	memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[5], 1);
	inTempKey = atoi(szTemplate);
	if (inTempKey > 0)
		inTempKey = 10 - inTempKey;
	sprintf(&szKeyNo1[1], "%d", inTempKey);

	/* 鍵值一第3碼 */
	memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[8], 1);
	inTempKey = atoi(szTemplate);
	if (inTempKey > 0)
		inTempKey = 10 - inTempKey;
	sprintf(&szKeyNo1[2], "%d", inTempKey);

	/* 鍵值一第4碼 */
	memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[11], 1);
	inTempKey = atoi(szTemplate);
	if (inTempKey > 0)
		inTempKey = 10 - inTempKey;
	sprintf(&szKeyNo1[3], "%d", inTempKey);

	/* 鍵值一第5~8碼 */
	memcpy(&szKeyNo1[4], &pobTran->srBRec.szTxnNo[2], 1);
	memcpy(&szKeyNo1[5], &pobTran->srBRec.szTxnNo[5], 1);
	memcpy(&szKeyNo1[6], &pobTran->srBRec.szTxnNo[8], 1);
	memcpy(&szKeyNo1[7], &pobTran->srBRec.szTxnNo[11], 1);

	/* 鍵值二 */
	memset(szKeyNo2, 0x00, sizeof(szKeyNo2));

	/* 鍵值二第1~8碼 */
	for ( i = 0 ; i < 8 ; i++)
	{
		memcpy(&szTemplate[0], &szKeyNo1[i], 1);
		inTempKey = atoi(szTemplate);
		inTempKey = inTempKey + inTempKey;

		if (inTempKey >= 10)
			inTempKey = inTempKey - 10;

		sprintf(&szKeyNo2[i], "%d", inTempKey);
	}

	/* 交易編號還原卡號 */
	memset(szPAN, 0x00, sizeof(szPAN)); /* 儲存TxNo還原卡號 */
	j = strlen(pobTran->srBRec.szTxnNo); /* 交易編號長度 */

	for (i = 0; i < j ; i++)
	{
		switch(i)
		{
			case 0:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[i], 1);
				inTempTxNo = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo2[2], 1);
				inTempKey = atoi(szTemplate);

				inTempTxNo = inTempTxNo + 10 - inTempKey;

				if (inTempTxNo >= 10)
					inTempTxNo = inTempTxNo - 10;

				sprintf(&szPAN[0], "%d", inTempTxNo);

				break;
			case 1:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[i], 1);
				inTempTxNo = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo1[4], 1);
				inTempKey = atoi(szTemplate);

				inTempTxNo = inTempTxNo + inTempKey;

				if (inTempTxNo >= 10)
					inTempTxNo = inTempTxNo - 10;

				sprintf(&szPAN[1], "%d", inTempTxNo);

				break;
			case 3:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[i], 1);
				inTempTxNo = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo2[6], 1);
				inTempKey = atoi(szTemplate);

				inTempTxNo = inTempTxNo + 10 - inTempKey;

				if (inTempTxNo >= 10)
					inTempTxNo = inTempTxNo - 10;

				sprintf(&szPAN[2], "%d", inTempTxNo);

				break;
			case 4:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[i], 1);
				inTempTxNo = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo1[2], 1);
				inTempKey = atoi(szTemplate);

				inTempTxNo = inTempTxNo + inTempKey;

				if (inTempTxNo >= 10)
					inTempTxNo = inTempTxNo - 10;

				sprintf(&szPAN[3], "%d", inTempTxNo);

				break;
			case 6:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[i], 1);
				inTempTxNo = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo2[0], 1);
				inTempKey = atoi(szTemplate);

				inTempTxNo = inTempTxNo + 10 - inTempKey;

				if (inTempTxNo >= 10)
					inTempTxNo = inTempTxNo - 10;

				sprintf(&szPAN[4], "%d", inTempTxNo);

				break;
			case 7:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[i], 1);
				inTempTxNo = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo1[5], 1);
				inTempKey = atoi(szTemplate);

				inTempTxNo = inTempTxNo + inTempKey;

				if (inTempTxNo >= 10)
					inTempTxNo = inTempTxNo - 10;

				sprintf(&szPAN[5], "%d", inTempTxNo);

				break;
			case 9:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[i], 1);
				inTempTxNo = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo2[1], 1);
				inTempKey = atoi(szTemplate);

				inTempTxNo = inTempTxNo + 10 - inTempKey;

				if (inTempTxNo >= 10)
					inTempTxNo = inTempTxNo - 10;

				sprintf(&szPAN[6], "%d", inTempTxNo);

				break;
			case 10:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[i], 1);
				inTempTxNo = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo1[1], 1);
				inTempKey = atoi(szTemplate);

				inTempTxNo = inTempTxNo + inTempKey;

				if (inTempTxNo >= 10)
					inTempTxNo = inTempTxNo - 10;

				sprintf(&szPAN[7], "%d", inTempTxNo);

				break;
			case 12:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[i], 1);
				inTempTxNo = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo2[4], 1);
				inTempKey = atoi(szTemplate);

				inTempTxNo = inTempTxNo + 10 - inTempKey;

				if (inTempTxNo >= 10)
					inTempTxNo = inTempTxNo - 10;

				sprintf(&szPAN[8], "%d", inTempTxNo);

				break;
			case 13:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[i], 1);
				inTempTxNo = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo1[6], 1);
				inTempKey = atoi(szTemplate);

				inTempTxNo = inTempTxNo + inTempKey;

				if (inTempTxNo >= 10)
					inTempTxNo = inTempTxNo - 10;

				sprintf(&szPAN[9], "%d", inTempTxNo);

				break;
			case 14:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[i], 1);
				inTempTxNo = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo2[7], 1);
				inTempKey = atoi(szTemplate);

				inTempTxNo = inTempTxNo + 10 - inTempKey;

				if (inTempTxNo >= 10)
					inTempTxNo = inTempTxNo - 10;

				sprintf(&szPAN[10], "%d", inTempTxNo);

				break;
			case 15:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[i], 1);
				inTempTxNo = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo1[3], 1);
				inTempKey = atoi(szTemplate);

				inTempTxNo = inTempTxNo + inTempKey;

				if (inTempTxNo >= 10)
					inTempTxNo = inTempTxNo - 10;

				sprintf(&szPAN[11], "%d", inTempTxNo);

				break;
			case 16:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[i], 1);
				inTempTxNo = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo2[3], 1);
				inTempKey = atoi(szTemplate);

				inTempTxNo = inTempTxNo + 10 - inTempKey;

				if (inTempTxNo >= 10)
					inTempTxNo = inTempTxNo - 10;

				sprintf(&szPAN[12], "%d", inTempTxNo);

				break;
			case 17:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[i], 1);
				inTempTxNo = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo1[7], 1);
				inTempKey = atoi(szTemplate);

				inTempTxNo = inTempTxNo + inTempKey;

				if (inTempTxNo >= 10)
					inTempTxNo = inTempTxNo - 10;

				sprintf(&szPAN[13], "%d", inTempTxNo);

				break;
			case 18:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[i], 1);
				inTempTxNo = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo2[5], 1);
				inTempKey = atoi(szTemplate);

				inTempTxNo = inTempTxNo + 10 - inTempKey;

				if (inTempTxNo >= 10)
					inTempTxNo = inTempTxNo - 10;

				sprintf(&szPAN[14], "%d", inTempTxNo);

				break;
			case 19:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[i], 1);
				inTempTxNo = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo1[0], 1);
				inTempKey = atoi(szTemplate);

				inTempTxNo = inTempTxNo + inTempKey;

				if (inTempTxNo >= 10)
					inTempTxNo = inTempTxNo - 10;

				sprintf(&szPAN[15], "%d", inTempTxNo);

				break;
			case 20:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[i], 1);
				inTempTxNo = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo2[2], 1);
				inTempKey = atoi(szTemplate);

				inTempTxNo = inTempTxNo + 10 - inTempKey;

				if (inTempTxNo >= 10)
					inTempTxNo = inTempTxNo - 10;

				sprintf(&szPAN[16], "%d", inTempTxNo);

				break;
			case 21:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[i], 1);
				inTempTxNo = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo1[4], 1);
				inTempKey = atoi(szTemplate);

				inTempTxNo = inTempTxNo + inTempKey;

				if (inTempTxNo >= 10)
					inTempTxNo = inTempTxNo - 10;

				sprintf(&szPAN[17], "%d", inTempTxNo);

				break;
			case 22:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &pobTran->srBRec.szTxnNo[i], 1);
				inTempTxNo = atoi(szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szKeyNo2[6], 1);
				inTempKey = atoi(szTemplate);

				inTempTxNo = inTempTxNo + 10 - inTempKey;

				if (inTempTxNo >= 10)
					inTempTxNo = inTempTxNo - 10;

				sprintf(&szPAN[18], "%d", inTempTxNo);

				break;
			default:
				break;

		} /* end switch */
	} /* end for */

	if (inCARD_Generate_Special_Card(szPAN) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	memset(pobTran->srBRec.szPAN, 0x00, sizeof(pobTran->srBRec.szPAN));
	strcpy(pobTran->srBRec.szPAN, szPAN);

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}


int  inCARDFUNC_ModifyTrack2(char *szTrack2,int inTrack2Len)
{
	int i;

	for(i=0;i<inTrack2Len;i++)
	{
		if(szTrack2[i] == 'D')
			szTrack2[i] = '=';
	}
	
	return (VS_SUCCESS);
}




