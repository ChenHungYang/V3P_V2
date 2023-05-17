
#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/TransType.h"

#include "../SOURCE/FUNCTION/APDU.h"

#include "SvcCommand.h"


/*
Function	: inSVC_BuildApduData
Date&Time	: 2022/12/29 下午 1:19
Describe	:
 * 獨立出 SVC 進行 APDU 資料格式組合功能
*/
int inSVC_BuildApduData(APDU_COMMAND *srAPDUData, unsigned char* CLA)
{
	int	inCnt = 0;

	inDISP_LogPrintfWithFlag("inAPDU_BuildAPDU()_START");

	srAPDUData->uszSendData[inCnt ++] = CLA;
	srAPDUData->uszSendData[inCnt ++] = srAPDUData->uszCommandINSData[0];
	srAPDUData->uszSendData[inCnt ++] = srAPDUData->uszCommandP1Data[0];
	srAPDUData->uszSendData[inCnt ++] = srAPDUData->uszCommandP2Data[0];
	srAPDUData->uszSendData[inCnt ++] = srAPDUData->inCommandDataLen; // LC
	memcpy(&srAPDUData->uszSendData[inCnt], &srAPDUData->uszCommandData[0], srAPDUData->inCommandDataLen);
	inCnt += srAPDUData->inCommandDataLen;

	/* LE(有的要塞0x00) */
	/* Select AID要塞LE 0x00 */
	/* 因為有可能和select EF搞混，所以多判斷P1 COMMAND */
	if (srAPDUData->uszCommandINSData[0]  == _COSTCO_SELECT_AID_INS_COMMAND &&
		srAPDUData->uszCommandP1Data[0]  == _COSTCO_SELECT_AID_P1_COMMAND)
	{
		srAPDUData->uszSendData[inCnt ++] = 0x00; /* Le 0x00 */
	}else if (CLA == _COSTCO_GEN_MAC_CLA_COMMAND &&
			srAPDUData->uszCommandINSData[0] == _COSTCO_GEN_MAC_INS_COMMAND)
	{
		srAPDUData->uszSendData[inCnt ++] = 0x08;
	}
	
	srAPDUData->inSendLen = inCnt;

	inDISP_LogPrintfWithFlag("srAPDUData->inSendLen : [%d]", srAPDUData->inSendLen);
     
	return (VS_SUCCESS);
}


/*
Function	: inSVC_GenMac
Date&Time	: 2022/12/29 下午 1:19
Describe	:
 * SVC 在 64Field 使用的 Mac 運算功能
*/
int inSVC_GenMac(TRANSACTION_OBJECT *pobTran,char* szInputData,int inInputDataLen)
{
	int inContactType = 1;
	int inRetVal;
	BYTE    btSVC_CardAID[] = {0xA0, 0x00, 0x00, 0x01, 0x59, 0x53, 0x41, 0x4D, 0x03, 0x00};
	APDU_COMMAND srAPDU_COMMAND;
	
	unsigned char baATR[128], bATRLen, CardType;
	
	bATRLen = 128;

	inRetVal = CTOS_SCResetISO(d_SC_SAM1, d_SC_5V, baATR, &bATRLen, &CardType);
	
	if (inRetVal== d_OK)
	{
		/* Select AID APDU Command */
		memset(&srAPDU_COMMAND, 0x00, sizeof (APDU_COMMAND));

		srAPDU_COMMAND.uszCommandINSData[0] = _COSTCO_SELECT_AID_INS_COMMAND; /* INS */
		srAPDU_COMMAND.uszCommandP1Data[0] = _COSTCO_SELECT_AID_P1_COMMAND; /* P1 */
		srAPDU_COMMAND.uszCommandP2Data[0] = _COSTCO_SELECT_AID_P2_COMMAND; /* P2 */
		srAPDU_COMMAND.inCommandDataLen = sizeof(btSVC_CardAID);
		memcpy(srAPDU_COMMAND.uszCommandData, btSVC_CardAID, sizeof(btSVC_CardAID));

		if (inSVC_BuildApduData(&srAPDU_COMMAND, _COSTCO_SELECT_AID_CLA_COMMAND) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		
		if (inContactType == 0)
		{
			inRetVal = inAPDU_Send_APDU_CTLS_Process(&srAPDU_COMMAND);
		} else
		{
			inRetVal = inAPDU_Send_APDU_Sam_Slot_Process(d_SC_SAM1, &srAPDU_COMMAND);
		}

		inDISP_LogPrintfArea(FALSE, "64 Select Aid: ", 15, (BYTE *) srAPDU_COMMAND.uszRecevData, srAPDU_COMMAND.inRecevLen);		
		
		
		/* APDU Command */
		memset(&srAPDU_COMMAND, 0x00, sizeof (APDU_COMMAND));

		srAPDU_COMMAND.uszCommandINSData[0] = _COSTCO_GEN_MAC_INS_COMMAND; /* INS */
		srAPDU_COMMAND.uszCommandP1Data[0] = _COSTCO_GEN_MAC_P1_COMMAND; /* P1 */
		srAPDU_COMMAND.uszCommandP2Data[0] = _COSTCO_GEN_MAC_P2_COMMAND; /* P2 */
		srAPDU_COMMAND.inCommandDataLen = inInputDataLen;
		memcpy(srAPDU_COMMAND.uszCommandData, szInputData, inInputDataLen);

		if (inSVC_BuildApduData(&srAPDU_COMMAND, _COSTCO_GEN_MAC_CLA_COMMAND) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		/* 收送Command */
		if (inContactType == 0)
		{
			inRetVal = inAPDU_Send_APDU_CTLS_Process(&srAPDU_COMMAND);
		} else
		{
			inRetVal = inAPDU_Send_APDU_Sam_Slot_Process(d_SC_SAM1, &srAPDU_COMMAND);
		}

		inDISP_LogPrintfArea(FALSE, "64 MAC: ", 8, (BYTE *) srAPDU_COMMAND.uszRecevData, srAPDU_COMMAND.inRecevLen);
	}else{
		inDISP_LogPrintfWithFlag("Open SamSlot Error : [%d]", inRetVal);
	}
	return (inRetVal);
}

