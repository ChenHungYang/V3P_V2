#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <ctos_clapi.h>
#include <emv_cl.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"
#include "../FUNCTION/Function.h"
#include "../FUNCTION/FILE_FUNC/File.h"
#include "../FUNCTION/FILE_FUNC/FIleLogFunc.h"
#include "../PRINT/Print.h"
#include "APDU.h"

extern int ginDebug;
extern int ginDisplayDebug;

/*
Function        :inAPDU_APDUTransmit
Date&Time       :2018/6/13 上午 10:01
Describe        :下APDU Command
 */
int inAPDU_APDUTransmit(unsigned char uszSlotID, unsigned char* uszSendBuffer, unsigned short usSendLen, unsigned char* uszReceiveBuffer, unsigned short* usReceiveLen)
{
	char szDebugMsg[1024 + 1] = {0};
	char szAscii[1024 + 1] = {0};
	unsigned short usRetVal = 0x00;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inAPDU_APDUTransmit()_START");

	}

	/* 記得要先把Buffer的最長長度放到usReceiveLen中，這邊只傳pointer所以要在外層做 */
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		inFunc_BCD_to_ASCII(szDebugMsg, uszSendBuffer, usSendLen);
		inDISP_LogPrintf(szDebugMsg);
	}

	usRetVal = CTOS_SCSendAPDU(uszSlotID, uszSendBuffer, usSendLen, uszReceiveBuffer, usReceiveLen);

	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "CTOS_SCSendAPDU_ERR : %04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}

		inDISP_DispLogAndWriteFlie(" CTOS_SCSendAPDU *Error* [%04X] Line[%d]", usRetVal, __LINE__);

		if (usRetVal == d_SC_NOT_PRESENT)
		{
			return (VS_EMV_CARD_OUT);
		} else
		{
			return (usRetVal);
		}
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("CTOS_SCSendAPDU_OK");

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%02x %02x Length = %d", uszReceiveBuffer[*usReceiveLen - 2], uszReceiveBuffer[*usReceiveLen - 1], *usReceiveLen);
			inDISP_LogPrintf(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			inFunc_BCD_to_ASCII(szAscii, uszReceiveBuffer, *usReceiveLen);
			sprintf(szDebugMsg, "Data: %s", szAscii);
			inDISP_LogPrintf(szDebugMsg);
		}

	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inAPDU_APDUTransmit()_END");
	}

	return (VS_SUCCESS);
}

/*
Function        :inAPDU_Send_APDU_User_Slot_Process
Date&Time       :2018/6/13 上午 10:24
Describe        :對大卡槽下命令
 */
int inAPDU_Send_APDU_User_Slot_Process(APDU_COMMAND* srAPDU)
{
	int inRetVal = VS_ERROR;
	unsigned char	uszSlot = d_SC_USER;
//	unsigned char uszSlot = d_SC_SAM1;


	/* 記得要先把Buffer的最長長度放到usReceiveLen中， */
	srAPDU->inRecevLen = sizeof (srAPDU->uszRecevData);

	inRetVal = inAPDU_APDUTransmit(uszSlot, srAPDU->uszSendData, (unsigned short) srAPDU->inSendLen, srAPDU->uszRecevData, (unsigned short*) &srAPDU->inRecevLen);

	return (inRetVal);
}

/*
Function        :inAPDU_APDUTransmit_CTLS
Date&Time       :2018/6/13 上午 10:48
Describe        :感應使用的APDU Command
 */
int inAPDU_APDUTransmit_CTLS(unsigned char* uszSendBuffer, unsigned short usSendLen, unsigned char* uszReceiveBuffer, unsigned short* usReceiveLen)
{
	int inRetVal = 0x00;
	char szDebugMsg[1024 + 1] = {0};
	char szAscii[1024 + 1] = {0};

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inAPDU_APDUTransmit_CTLS()_START");
	}

	/* 記得要先把Buffer的最長長度放到usReceiveLen中，這邊只傳pointer所以要在外層做 */

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		inFunc_BCD_to_ASCII(&szDebugMsg[0], uszSendBuffer, usSendLen);
		inDISP_LogPrintf(szDebugMsg);

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Len: %d", usSendLen);
		inDISP_LogPrintf(szDebugMsg);
	}

	inRetVal = CTOS_CLAPDU(uszSendBuffer, usSendLen, uszReceiveBuffer, usReceiveLen);

	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "CTOS_CLAPDU_ERR : %04X", inRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}

		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "CTOS_CLAPDU_ERR : %04X", inRetVal);
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
		}
		inDISP_DispLogAndWriteFlie("  CTOS_CLAPDU *Error* [%04X] Line[%d]", inRetVal, __LINE__);

		return (VS_ERROR);
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("CTOS_SCSendAPDU_OK");

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%02x %02x Length = %d", uszReceiveBuffer[*usReceiveLen - 2], uszReceiveBuffer[*usReceiveLen - 1], *usReceiveLen);
			inDISP_LogPrintf(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			inFunc_BCD_to_ASCII(szAscii, uszReceiveBuffer, *usReceiveLen);
			sprintf(szDebugMsg, "Data: %s", szAscii);
			inDISP_LogPrintf(szDebugMsg);
		}
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inAPDU_APDUTransmit_CTLS()_END");
	}

	return (VS_SUCCESS);
}

/*
Function        :inAPDU_User_Slot_Send_APDU
Date&Time       :2018/6/13 上午 10:24
Describe        :對大卡槽下命令
 */
int inAPDU_Send_APDU_CTLS_Process(APDU_COMMAND* srAPDU)
{
	int inRetVal = VS_ERROR;

	/* 記得要先把Buffer的最長長度放到usReceiveLen中， */
	srAPDU->inRecevLen = sizeof (srAPDU->uszRecevData);

	inRetVal = inAPDU_APDUTransmit_CTLS(srAPDU->uszSendData, (unsigned short) srAPDU->inSendLen, srAPDU->uszRecevData, (unsigned short*) &srAPDU->inRecevLen);

	return (inRetVal);
}

/*
Function        :inAPDU_APDUTransmit_Flow
Date&Time       :2018/6/13 下午 1:37
Describe        :下APDU Command
 */
int inAPDU_APDUTransmit_Flow(TRANSACTION_OBJECT *pobTran, APDU_COMMAND *srAPDU_COMMAND)
{
	int inRetVal = VS_ERROR;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inAPDU_APDUTransmit_Flow()_START");
	}

	/* 感應下APDU不同 另外寫 */
	if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
	{
		inRetVal = inAPDU_Send_APDU_CTLS_Process(srAPDU_COMMAND);
	} else
	{
		inRetVal = inAPDU_Send_APDU_User_Slot_Process(srAPDU_COMMAND);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inAPDU_APDUTransmit_Flow()_END");
	}

	return (inRetVal);
}

/*
App Name	: inAPDU_BuildAPDU
App Date&Time	: 2018/6/13 下午 2:03
App Function	: Build APDU Command
Input Param	: *srAPDUData --> APDU Command 結構
Output Param	: 成功 : VS_SUCCESS
		  失敗 : VS_ERROR
 */
int inAPDU_BuildAPDU(APDU_COMMAND *srAPDUData)
{
	int inCnt = 0;
	char szDebugMSG[100 + 1] = {0};

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inAPDU_BuildAPDU()_START");
	}

	srAPDUData->uszSendData[inCnt++] = _FISC_READ_RECORDS_CLA_COMMAND_;
	srAPDUData->uszSendData[inCnt++] = srAPDUData->uszCommandINSData[0];
	srAPDUData->uszSendData[inCnt++] = srAPDUData->uszCommandP1Data[0];
	srAPDUData->uszSendData[inCnt++] = srAPDUData->uszCommandP2Data[0];
	srAPDUData->uszSendData[inCnt++] = srAPDUData->inCommandDataLen;
	memcpy(&srAPDUData->uszSendData[inCnt], &srAPDUData->uszCommandData[0], srAPDUData->inCommandDataLen);
	inCnt += srAPDUData->inCommandDataLen;

	/* LE(有的要塞0x00) */
	/* Select AID要塞LE 0x00 */
	/* 因為有可能和select EF搞混，所以多判斷P1 COMMAND */
	if (srAPDUData->uszCommandINSData[0] == _FISC_SELECT_AID_INS_COMMAND_ &&
			srAPDUData->uszCommandP1Data[0] == _FISC_SELECT_AID_P1_COMMAND_)
	{
		srAPDUData->uszSendData[inCnt++] = 0x00;
	} else if (srAPDUData->uszCommandINSData[0] == _FISC_WRITE_RECORD_INS_COMMAND_)
	{
		srAPDUData->uszSendData[inCnt++] = 0x00;
	} else if (srAPDUData->uszCommandINSData[0] == _FISC_WRITE_AUTH_RECORD_INS_COMMAND_)
	{
		srAPDUData->uszSendData[inCnt++] = 0x00;
	} else if (srAPDUData->uszCommandINSData[0] == _FISC_READ_AUTH_RECORD_INS_COMMAND_)
	{
		srAPDUData->uszSendData[inCnt++] = 0x00;
	}

	srAPDUData->inSendLen = inCnt;

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMSG, 0x00, sizeof (szDebugMSG));
		sprintf(szDebugMSG, "srAPDUData->inSendLen : [%d]", srAPDUData->inSendLen);
		inDISP_LogPrintf(szDebugMSG);
		inDISP_LogPrintf("inAPDU_BuildAPDU()_END");
	}

	return (VS_SUCCESS);
}

/*
Function	:inAPDU_Send_APDU_User_Slot_Process
Date&Time	: 2022/12/28 下午 8:23
Describe        :
 */
int inAPDU_Send_APDU_Sam_Slot_Process(unsigned char * uszSlot, APDU_COMMAND* srAPDU)
{
	int inRetVal = VS_ERROR;
//	unsigned char	uszSlot = d_SC_USER;
//	unsigned char uszSlot = d_SC_SAM1;


	/* 記得要先把Buffer的最長長度放到usReceiveLen中， */
	srAPDU->inRecevLen = sizeof (srAPDU->uszRecevData);

	inRetVal = inAPDU_APDUTransmit(uszSlot, srAPDU->uszSendData, (unsigned short) srAPDU->inSendLen, srAPDU->uszRecevData, (unsigned short*) &srAPDU->inRecevLen);

	return (inRetVal);
}
