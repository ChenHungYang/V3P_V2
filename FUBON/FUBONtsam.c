#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>

#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/FILE_FUNC/File.h"
#include "../SOURCE/FUNCTION/FILE_FUNC/FIleLogFunc.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Batch.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/CPT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/MVT.h"
#include "../SOURCE/FUNCTION/KMS.h"
#include "../SOURCE/FUNCTION/Signpad.h"
#include "../SOURCE/COMM/Comm.h"
//#include "../CREDIT/Creditfunc.h"
#include "../CTLS/CTLS.h"
//#include "../FISC/NCCCfisc.h"
#include "../EMVSRC/EMVsrc.h"
#include "FUBONfunc.h"
#include "FUBONiso.h"
#include "FUBONtsam.h"

#define APDU_DEBUG

DWORD Slot_Type;
//extern DWORD Slot_Type;
extern int ginDebug; /* Debug使用 extern */


/* *************************************************************** */
/* **********	FUBON TSAM CIPHER USE FUNCTION (START)	********** */
/* *************************************************************** */

/*
App Name      : inFUBON_TSAM_Get_Slot_Type
App Builder    : sampo
App Date&Time : 2014/7/20 
App Function  :
Input Param   : BYTE usSlot,DWORD *Slot_Type
Output Param  : 1. -> VS_SUCCESS
			2. -> VS_ERROR
 */
int inFUBON_TSAM_Get_Slot_Type(BYTE usSlot, DWORD *Slot_Type)
{
	switch (usSlot)
	{
		case '0':
			inDISP_LogPrintfWithFlag("  GET_SLOT_TYPE_ERROR(0)");
			return (VS_ERROR);
		case '1':
			*Slot_Type = _SAM_SLOT_1_;
			break;
		case '2':
			*Slot_Type = _SAM_SLOT_2_;
			break;
		case '3':
			*Slot_Type = _SAM_SLOT_3_;
			break;
		case '4':
			*Slot_Type = _SAM_SLOT_4_;
			break;
		case '7':
			inDISP_LogPrintfWithFlag("  GET_SLOT_TYPE_ERROR(7)");
			return (VS_ESCAPE);
		default:
			*Slot_Type = _SAM_SLOT_1_;
			break;
	}

	return (VS_SUCCESS);
}

/*
App Name      : inFUBON_TSAM_APDUTransmit
App Builder   : sampo
App Date&Time : 2014/7/20 �U�� 06:05:41
App Function  :
Input Param   : TRANSACTION_OBJECT *pobTran
				DWORD Slot
				BYTE *bSendBuf
				int inSendLen
				BYTE *bReceBuf
				unsigned short *pusRespLen
Output Param  : 1. ���\ -> VS_SUCCESS
			2. ���� -> VS_ERROR
 */
int inFUBON_TSAM_APDUTransmit(TRANSACTION_OBJECT *pobTran,
		DWORD Slot,
		BYTE *bSendBuf,
		int inSendLen,
		BYTE *bReceBuf,
		unsigned short *pusRespLen)
{
	char usRetVal;
	BYTE stMsg1[5] = {0x00, 0xC0, 0x00, 0x00};
	BYTE bBuf[50];
	BYTE bTmp[800];
	int i;

	inDISP_LogPrintfWithFlag("-----[%s][%s] Line[%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memset((char *) bTmp, 0x00, sizeof (bTmp));
	sprintf((char *) bTmp, "(%ld) APDU Send 1:", (unsigned long) Slot);
	for (i = 0; i < inSendLen; i++)
		sprintf((char *) &bTmp[16 + i * 2], "%02X", bSendBuf[i]);

	inDISP_LogPrintfWithFlag("%s", (char *) bTmp);

	memset((char *) bBuf, 0x00, sizeof (bBuf));
	bReceBuf[0] = 0x00;

	/*
	  Tag_Select_ICC
	  Length = 4 Bytes
	  This tag select ICC. This tag must be used before any data communication is
	  exchanged with the card.This tag can only be set and cannot be read.
	 */
	*pusRespLen = 0;
	usRetVal = CTOS_SCSendAPDU(Slot, bSendBuf, inSendLen, bReceBuf, pusRespLen);

	/********************* DEBUG *************************/
	memset((char *) bTmp, 0x00, sizeof (bTmp));
	sprintf((char *) bTmp, "APDU Rece 1:");

	for (i = 0; i < *pusRespLen; i++)
		sprintf((char *) &bTmp[12 + i * 2], "%02X", bReceBuf[i]);

	inDISP_LogPrintfWithFlag("%s", (char *) bTmp);
	/*****************************************************/

	if (*pusRespLen > 1 && !memcmp((char *) &bReceBuf[*pusRespLen - 2], "\x61", 1))
	{
		/********************* DEBUG *************************/
		stMsg1[4] = bReceBuf[1];
		memset((char *) bTmp, 0x00, sizeof (bTmp));
		sprintf((char *) bTmp, "(%ld) APDU Send 2:", (unsigned long) Slot);

		for (i = 0; i < 5; i++)
			sprintf((char *) &bTmp[16 + i * 2], "%02X", stMsg1[i]);

		inDISP_LogPrintfWithFlag("%s", (char *) bTmp);
		/*****************************************************/

		usRetVal = CTOS_SCSendAPDU(Slot, stMsg1, 5, bReceBuf, pusRespLen);

		/********************* DEBUG *************************/
		memset((char *) bTmp, 0x00, sizeof (bTmp));
		sprintf((char *) bTmp, "APDU Rece 2:");

		for (i = 0; i < *pusRespLen; i++)
			sprintf((char *) &bTmp[12 + i * 2], "%02X", bReceBuf[i]);

		inDISP_LogPrintfWithFlag("%s", (char *) bTmp);
		/*****************************************************/
	}


	if (memcmp((char *) &bReceBuf[*pusRespLen - 2], "\x90\x00", 2))
	{

		if (*pusRespLen <= 1)
		{
			sprintf((char *) bBuf, "(%ld) APDU <Send:%02X%02X%02X%02X%02X Rece Len = 0>", (unsigned long) Slot, bSendBuf[0],
					bSendBuf[1],
					bSendBuf[2],
					bSendBuf[3],
					bSendBuf[4]);
		} else
		{
			sprintf((char *) bBuf, "(%ld) APDU <Send:%02X%02X%02X%02X%02X Rece:%02X%02X>", (unsigned long) Slot, bSendBuf[0],
					bSendBuf[1],
					bSendBuf[2],
					bSendBuf[3],
					bSendBuf[4],
					bReceBuf[*pusRespLen - 2],
					bReceBuf[*pusRespLen - 1]);
		}

		inDISP_LogPrintfWithFlag("%s", (char *) bBuf);
		inDISP_LogPrintfWithFlag("-----[%s][%s] Line[%d] *Error* END-----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag("%s", (char *) bTmp);
	inDISP_LogPrintfWithFlag("-----[%s][%s] Line[%d] SUCCESS END-----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
App Name      : inFUBON_TSAM_FuncInitialSLOT
App Builder   : sampo
App Date&Time : 2014/7/20   05:12:52
App Function  : 
Input Param   : TRANSACTION_OBJECT *pobTran
Output Param  : ���\ -> Return VS_SUCCESS
		  :  -> Return VS_ERROR
		 : SAM External Spec _2 0_ for EDC
 */

int inFUBON_TSAM_FuncInitialSLOT(TRANSACTION_OBJECT *pobTran)
{
	BYTE usSlot = 0;
	int in_RetVal/*, inKey*/;
	BYTE b_SAMAid[] = {0xA0, 0x00, 0x00, 0x01, 0x59, 0x53, 0x41, 0x04D, 0x02, 0x00};
	int in_SAMAidLen = 10;
	char szTMSOK[2];
	TFB_APDU_COMMAND srAPDUData;

	inDISP_LogPrintfWithFlag("-----[%s][%s] Line[%d] START -----", __FILE__, __FUNCTION__, __LINE__);

#ifdef DEMO_EDC
	return (VS_SUCCESS);
#endif
	inDISP_LogPrintfWithFlag("CustomerIndicator [%s] Line[%d]", szGetCustomerIndicator(), __LINE__);

	//        if (memcmp(szGetCustomerIndicator(),_CUSTOMER_000_STANDARD_, 3) ||
	//            memcmp(szGetCustomerIndicator(),_CUSTOMER_005_STANDARD_, 3))
	//        {
	inDISP_LogPrintfWithFlag("It is not TSAM Version");
	//                return (VS_SUCCESS);
	//        }

	memset(szTMSOK, 0x00, sizeof (szTMSOK));
	inGetTMSOK(szTMSOK);
	if (szTMSOK[0] == '0')
	{
		inDISP_LogPrintfWithFlag("inFUBON_TSAM_FuncInitialSLOT TMSOK = 0 END");
		return (VS_SUCCESS);
	}

	if ((inGetEncryptionModeEnable() == _FUBON_ENCRYPTION_TSAM_))
	{
		/* Get SAM Slot Type */
		inGetHostSAMSlot((char *) &usSlot);

		inDISP_LogPrintfWithFlag(" #SLOT =(%c)", usSlot);

		in_RetVal = inFUBON_TSAM_Get_Slot_Type(usSlot, &Slot_Type);

		if (in_RetVal == VS_ERROR)
		{
			inDISP_LogPrintfWithFlag("inFUBON_TSAM_FuncInitialSLOT() EncryptMode ERROR!!");
			return (VS_ERROR);
		} else if (in_RetVal == VS_ESCAPE)
		{
			inDISP_LogPrintfWithFlag("inFUBON_TSAM_FuncInitialSLOT() EncryptMode VS_ESCAPE!!");
			return (VS_SUCCESS);
		}

		/* Initialize SAM Slot */
		if (inFUBON_TSAM_InitialSLOT(Slot_Type) != VS_SUCCESS)
		{
			inDISP_Msg_BMP(_ERR_REGISTER_SAM_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
			return (VS_ERROR);
		}

		/* SELECT_FILE */

		/*----------------------------------------------------------
		 CLS = [1]  00
		 INS = [1]  A4 Status of SAM is to be transitioned
		 P1  = [1]  04 Only occurrence
		 p2  = [1]  00
		 Lc  = [1]  0A
		 Data= [10] A0 00 00 01 59 53 41 4D 02 00   SAM AID
		 Le  = [1]  00
		------------------------------------------------------------*/
		srAPDUData.inSendLen = inFUBON_TSAM_BuildSAMAPDU(pobTran,
				&srAPDUData,
				b_SAMAid,
				in_SAMAidLen,
				SAM_CLA_SELECT,
				SAM_INS_SELECT,
				SAM_P1_SELECT,
				SAM_P2_SELECT,
				VS_FALSE);

		/* FUBON SELECT_FILE */
		if (inFUBON_TSAM_APDUTransmit(pobTran,
				Slot_Type,
				srAPDUData.usSend_Buf,
				srAPDUData.inSendLen,
				srAPDUData.usRece_Buf,
				&srAPDUData.usReceLen) != VS_SUCCESS)
		{

			inDISP_Msg_BMP(_ERR_REGISTER_SAM_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
			return (VS_ERROR);
		}
	} else
	{
		inDISP_LogPrintfWithFlag("  Encrypt %d ", inGetEncryptionModeEnable());
		inDISP_LogPrintfWithFlag("-----[%s][%s] Line[%d] *Error* END-----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s] Line[%d] SUCCESS END-----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
App Name	: inFUBON_TSAM_InitialSLOT
App Builder	: sampo
App Date&Time	: 2014/7/21 01:08:37
App Function	: 
Field Note      :

 */
int inFUBON_TSAM_InitialSLOT(DWORD Slot_Type)
{
#if 0//By Ray
	BYTE ucProtocol, IFBuff[33];
	short IFLen = -1;
	DWORD Value;
	RESPONSECODE ret;

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("****inFUBON_TSAM_InitialSLOT() START****");

	ret = Init_CardSlot(Slot_Type);
	if (ret != CARDSLOT_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("  INITIAL_SLOT_ERROR_Init_CardSlot()");
		inDISP_ErrorWarningMsg(CLM_CHECK_SAM_MSG, ERROR_MSG);
		return (VS_ERROR);
	}
	/*-----------------------------------
	CLASS_A = {DWORD 0x0001}
	Tag_Class_Management ={DWORD 0x0194}
	------------------------------------*/
	Value = CLASS_A;
	ret = Set_Capability(Slot_Type, Tag_Class_Management, (BYTE *) (&Value));
	if (ret != CARDSLOT_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("  INITIAL_SLOT_ERROR_Set_Capability()");
		inDISP_ErrorWarningMsg(CLM_CHECK_SAM_MSG, ERROR_MSG);
		return (VS_ERROR);
	}

	ret = Reset_CardSlot(Slot_Type, RESET_COLD); //POWER ON
	if (ret != CARDSLOT_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("  Reset_CardSlot_RESET_COLD(%ld)", ret);

		ret = Reset_CardSlot(Slot_Type, RESET_WARM); //POWER ON
		if (ret != CARDSLOT_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("  Reset_CardSlot_RESET_WARM(%ld)", ret);
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("  INITIAL_SLOT_ERROR_Reset_CardSlot()");
			inDISP_ErrorWarningMsg(CLM_CHECK_SAM_MSG, ERROR_MSG);
			return (VS_ERROR);
		}
	}
	/*-----------------------------------------------------------------
	This function retrieves the protocol set during Reset_CardSlot()API
	as a part of the ATR string(Only T0 and T1 protocols are supported)
	-----------------------------------------------------------------*/
	ucProtocol = Get_Protocol(Slot_Type);
	if (ucProtocol != PROTOCOL_T0 && ucProtocol != PROTOCOL_T1)
	{
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("  INITIAL_SLOT_ERROR_Get_Protocol()");
		inDISP_ErrorWarningMsg(CLM_CHECK_SAM_MSG, ERROR_MSG);
		return (VS_ERROR);
	}

	IFLen = Get_Interface_Bytes(Slot_Type, IFBuff);
	if (IFLen < 0)
	{
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("  INITIAL_SLOT_ERROR_Get_Interface_Bytes()");
		inDISP_ErrorWarningMsg(CLM_CHECK_SAM_MSG, ERROR_MSG);
		return (VS_ERROR);
	}

	if (Get_Card_State(Slot_Type) != CARD_PRESENT)
	{
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("  INITIAL_SLOT_ERROR_Get_Card_State()");
		inDISP_ErrorWarningMsg(CLM_CHECK_SAM_MSG, ERROR_MSG);
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("****inFUBON_TSAM_InitialSLOT() SUCCESS END****");
#endif
	return (VS_SUCCESS);
}

/*
App Name	: inFUBON_TSAM_BuildSAMAPDU
App Builder	: sampo
App Date&Time	: 2014/7/21 01:07:58
App Function	: FUBON TSAM
Field Note      :

 */
int inFUBON_TSAM_BuildSAMAPDU(TRANSACTION_OBJECT *pobTran,
		TFB_APDU_COMMAND *srAPDUData,
		BYTE *bInData,
		int inInCLen,
		BYTE CLA,
		BYTE Ins,
		BYTE P1,
		BYTE P2,
		VS_BOOL blTCK)
{
	int in_Cnt = 0;

	/*--------------------------------------------------------------------------------
	Command APDU(APDU = Application Protocol Data Unit )
	CLA     - Instruction class
	INS     - Instruction code
	P1-P2   - Instruction parameters for the command
	Lc	- DATA���ƪ���
			  Encodes the number (Nc) of bytes of command data to follow Command data
	Nc	- ���O��Ƥ��e
			  Nc bytes of data
	Le	- ����^���T�� DATA���ƪ���
			  Encodes the maximum number (Ne) of response bytes expected
	----------------------------------------------------------------------------------
	Response APDU
	SW1-SW2 - Command processing status
	---------------------------------------------------------------------------------*/

	memset(srAPDUData, _NULL_CH_, sizeof (TFB_APDU_COMMAND));

	srAPDUData->usSend_Buf[in_Cnt++] = CLA;
	srAPDUData->usSend_Buf[in_Cnt++] = Ins;
	srAPDUData->usSend_Buf[in_Cnt++] = P1;
	srAPDUData->usSend_Buf[in_Cnt++] = P2;

	if (Ins == SAM_INS_SELECT)
	{
		srAPDUData->usSend_Buf[in_Cnt++] = inInCLen; /* Lc */
		memcpy((char *) &srAPDUData->usSend_Buf[in_Cnt], (char *) bInData, inInCLen);
		in_Cnt += inInCLen;
	}

	return (in_Cnt);
}

/*
App Name	: inFUBON_TSAM_EncryptISOData
App Builder	: sampo
App Date&Time	: 2014/7/14 �U�� 01:20:56
App Function	: FUBON TSAM�[�K
Field Note      :
�ѦҤ��        : fubon_ADSL_EDC�[�ѱK�M�פu�@������.doc
				  SAM�d External Spec _2 0_ for EDC�ŧQ.pdf
 */

int inFUBON_TSAM_EncryptISOData(TRANSACTION_OBJECT *pobTran, char *szEncrypt, int inEncryptLen)
{

	//int	inFieldDataLen = 0;
	//int	inSAMSLen = 0, inSAMTLen = 0, inSAMELen = 0;
	int i/*, j*/;
	//int     inTempLen;
	int in_InputLen = 0;
	int in_SAMEncrptyLen = 0;

	long ln_CipherLc = 0L;
	//char    szSAMBuf[2 + 1];
	//char    szSAMDSP[4 + 1];
	//char    szTempBuf[512],szCopyBuf[3];
	char szEncryptDataBuffer[256 + 1];
	//char szMsgBuf[20];

	//	char	ch_CipherData[80 + 1];
	char ch_FieldPackData[24 + 1];
	char ch_FieldCipherData[24 + 1];
	//char	ch_FieldDataLenDSP[2 + 1];
	//char	ch_FieldDataLenHEX[1 + 1];
	char ch_FieldDataBuffer[50 + 1];

	char szSTAN[6 + 1];


	TFB_APDU_COMMAND srAPDUData;

	memset(ch_FieldDataBuffer, _NULL_CH_, sizeof (ch_FieldDataBuffer));
	inFunc_BCD_to_ASCII(ch_FieldDataBuffer, (unsigned char *) szEncrypt, ((inEncryptLen + 1) / 2));
	ch_FieldDataBuffer[inEncryptLen] = 0x00;

	/*-----------------------------
	CLA = [1] 80h
	INS = [1] 44h
	P1  = [1] Domain Index (1-20)
	P2  = [1] Working Key Index(1-10) or Domain Key (when P2 =0)
	Lc  = [1] Var
	Data [Var]
	Diversify Alg[1]
	Diversify Len[1]
	Diversify Data[8/16/24]
	Cipher Alg[1]
	Input Len[1]
	Input Data
	IV[8]
	Le  = [1] 00h
	------------------------------*/

	memset(szEncryptDataBuffer, _NULL_CH_, sizeof (szEncryptDataBuffer));
	/* Diversify Alg */
	szEncryptDataBuffer[0] = 0x01;
	ln_CipherLc++;

	/* Diversify Len 16 */
	szEncryptDataBuffer[1] = 0x10;
	ln_CipherLc++;

	/*----------------------------------------------------------------------------------------
	a.	EDC KEY
	I.	Master Key T-SAM Key index.
	II.	Diversity Data : Terminal ID [8] + system trace no [6] + 0x00 +  0x00 16 bytes
	------------------------------------------------------------------------------------------*/
	/* Diversify Data */
	/* Terminal ID [8] + system trace no [6] + 0x00 + 0x00 */
	inGetTerminalID(&szEncryptDataBuffer[2]);
	//memcpy(&szEncryptDataBuffer[2], szGetTerminalID(), 8);
	ln_CipherLc += 8;

	memset(szSTAN, 0x00, sizeof (szSTAN));
	//      sprintf(szSTAN, "%06ld", pobTran->srBRec.lnSTANNum);

	if (pobTran->srBRec.uszOfflineBit == VS_TRUE &&
			pobTran->srBRec.uszUpload1Bit == VS_TRUE &&
			pobTran->srBRec.uszUpload2Bit == VS_TRUE)
		sprintf(szSTAN, "%06ld", pobTran->srBRec.lnSTAN_Multi);
	else
		sprintf(szSTAN, "%06ld", pobTran->srBRec.lnSTANNum);

	memcpy(&szEncryptDataBuffer[10], &szSTAN[0], 6);

	ln_CipherLc += 6;
	szEncryptDataBuffer[16] = 0x00;
	ln_CipherLc++;
	szEncryptDataBuffer[17] = 0x00;
	ln_CipherLc++;

	/* Cipher Alg */
	szEncryptDataBuffer[18] = 0x01;
	ln_CipherLc++;

	/* Input Len */
	if (inEncryptLen < 17)
	{
		in_InputLen = 16;
	} else if (inEncryptLen < 33)
	{
		in_InputLen = 32;
	} else if (inEncryptLen < 49)
	{
		in_InputLen = 48;
	} else
	{
		inDISP_Msg_BMP(_ERR_ENCODE_LENGTH_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		return (VS_ERROR);
	}

	in_SAMEncrptyLen = (in_InputLen / 2);
	szEncryptDataBuffer[19] = 0x08; /* wlength = 8 */
	ln_CipherLc++;

	/* Input Data 0xFF */
	for (i = 0; i < in_InputLen; i++)
	{
		if (ch_FieldDataBuffer[i] == 0x00)
			ch_FieldDataBuffer[i] = 0x46;
	}

	memset(ch_FieldPackData, _NULL_CH_, sizeof (ch_FieldPackData));
	hex_asc_to_bcd(ch_FieldDataBuffer, (unsigned char *) ch_FieldPackData);

	ln_CipherLc += 8;

	/* IV */
	ln_CipherLc += 8;
	i = 0;
	memset(ch_FieldCipherData, _NULL_CH_, sizeof (ch_FieldCipherData));

	while (1)
	{
		memcpy(&szEncryptDataBuffer[20], &ch_FieldPackData[i], 8);

		/* WRITE_RECORD_WITH_DATA_ENCRYPTION */
		/*-------------------------------------------
		CIPHER command
		The CIPHER command is used to encipher/decipher the Specified data
		 -------------------------------------------*/
		memset((char *) &srAPDUData, _NULL_CH_, sizeof (srAPDUData));
		srAPDUData.usSend_Buf[srAPDUData.inSendLen++] = SAM_CLA_CIPHER;
		srAPDUData.usSend_Buf[srAPDUData.inSendLen++] = SAM_INS_CIPHER;
		srAPDUData.usSend_Buf[srAPDUData.inSendLen++] = SAM_P1_CIPHER;
		srAPDUData.usSend_Buf[srAPDUData.inSendLen++] = SAM_P2_CIPHER;
		srAPDUData.usSend_Buf[srAPDUData.inSendLen++] = (BYTE) ((ln_CipherLc & 0x000000FF));
		memcpy(&srAPDUData.usSend_Buf[srAPDUData.inSendLen], (BYTE *) & szEncryptDataBuffer[0], ln_CipherLc);
		srAPDUData.inSendLen += ln_CipherLc;
		srAPDUData.usSend_Buf[srAPDUData.inSendLen++] = SAM_LE_CIPHER;


		if (inFUBON_TSAM_APDUTransmit(pobTran,
				Slot_Type,
				&srAPDUData.usSend_Buf[0],
				srAPDUData.inSendLen,
				&srAPDUData.usRece_Buf[0],
				&srAPDUData.usReceLen) != VS_SUCCESS)
		{
			inDISP_LogPrintfWithFlag("tSAM_ENCRYPT_ERR");
			return (VS_ERROR);
		} else
		{
			if (srAPDUData.usRece_Buf[srAPDUData.usReceLen - 2] == 0x90 && srAPDUData.usRece_Buf[srAPDUData.usReceLen - 1] == 0x00)
			{
				memcpy((char *) &ch_FieldCipherData[i], (char *) &srAPDUData.usRece_Buf[0], 8);
				in_SAMEncrptyLen -= 8;

				if (in_SAMEncrptyLen == 0)
					break;
				else
				{
					memset(&szEncryptDataBuffer[20], _NULL_CH_, 8);
					i += 8;
					continue;
				}
			} else
			{
				inDISP_LogPrintfWithFlag("tSAM_RESP_NOT_9000_ERR");
				return (VS_ERROR);
			}
		}
	}

	/* Encrption Data */
	memcpy((char *) &szEncrypt[0], &ch_FieldCipherData[0], (in_InputLen / 2));
	return (in_InputLen / 2);

}

int hex_asc_to_bcd(char *asc, unsigned char *bcd)
{
	unsigned char temp[100];
	int length, i;

	length = strlen(asc);
	if (length % 2 != 0)
	{
		sprintf((char *) temp, asc);
		strcat((char *) temp, "0");
		length++;
	} else
		sprintf((char *) temp, asc);

	for (i = 0; i < length; i++)
		if ((temp[i] > '9') || (temp[i] < '0'))
		{
			/* not in the range of 0~9 */
			if ((temp[i] >= 'A') && (temp[i] <= 'F'))
				temp[i] += 1 + '9' - 'A';
			else if ((temp[i] >= 'a') && (temp[i] <= 'f'))
				temp[i] += 1 + '9' - 'a';
			else
				temp[i] = 0x3d;
		}

	for (i = 0; i < length / 2; i++)
		bcd[i] = (unsigned char) ((unsigned char) ((temp[i * 2] - '0') * 16) +
			(unsigned char) (temp[i * 2 + 1] - '0'));

	return (i);
}

