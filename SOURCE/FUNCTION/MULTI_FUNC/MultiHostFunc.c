
#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include <stdlib.h>
#include <emv_cl.h>
#include <sqlite3.h>

#include "../../INCLUDES/Define_1.h"
#include "../../INCLUDES/Define_2.h"
#include "../../INCLUDES/TransType.h"
#include "../../INCLUDES/Transaction.h"
#include "../../EVENT/Menu.h"
#include "../../PRINT/Print.h"
#include "../../DISPLAY/Display.h"
#include "../../DISPLAY/DisTouch.h"
#include "../../EVENT/MenuMsg.h"
#include "../../EVENT/Flow.h"
#include "../../EVENT/Event.h"
#include "../APDU.h"
#include "../FILE_FUNC/File.h"
#include "../FILE_FUNC/FIleLogFunc.h"
#include "../Function.h"
#include "../FuncTable.h"
#include "../Sqlite.h"
#include "../CDT.h"
#include "../Card.h"
#include "../CFGT.h"
#include "../EDC.h"
#include "../HDT.h"
#include "../HDPT.h"
#include "../KMS.h"
#include "../SCDT.h"
#include "../VWT.h"
#include "../ECR_FUNC/ECR.h"
#include "../ECR_FUNC/RS232.h"

#include "../../COMM/WiFi.h"
#include "../MVT.h"
#include "../../../CTLS/CTLS.h"
#include "../Signpad.h"
#include "../FILE_FUNC/File.h"
#include "../FILE_FUNC/FIleLogFunc.h"

#include "MultiFunc.h"
#include "MultiHostFunc.h"
#include "JsonMultiHostFunc.h"

#include "../USB.h"

#ifdef __MUTI_FUCN_TEST_

extern int              ginDebug;
extern int              ginDisplayDebug;
extern int              ginEngineerDebug;
extern int              ginMachineType;
extern MULTI_TABLE	gstMultiOb;

extern int              ginHandleUSBPort;

static int              st_inMultiHostCtlsLen;
static unsigned char    st_uszMultiHostCTLSData[1024+1];

static  BOOL            st_fFileMake = VS_FALSE;

static char	*st_pszAllocDynamic;	/* 動態大小 */

BOOL gfCTLS_Bit;
BOOL fTwo_Tap;

MULTI_TRANS_TABLE srMultiHostTransTb[] =
{
	/* 第0組，RS232 Comport */
        {
		inMultiFunc_HostInital,
		inMultiFunc_Host_RecePacket,
		inMultiFunc_Host_SendPacketWaitACK,
		inMultiFunc_Host_SendError,
		inMultiFunc_Host_End
	},
        /* 第一組 - _MULTI_PIN_NO_  */
	{
		inMultiFunc_HostInital,
		inMultiFunc_Host_RecePacket,
		inMultiFunc_Host_SendPacketDotWaitAck,
		inMultiFunc_Host_SendError,
		inMultiFunc_Host_End
	},/* 第二組 - _MULTI_CTLS_NO_ */
	{
		inMultiFunc_HostInital,
		inMultiFunc_Host_RecePacket,
		inMultiFunc_Host_SendPacketDotWaitAck,
		inMultiFunc_Host_SendError,
		inMultiFunc_Host_End
	},/* 第三組 - _MULTI_SIGNPAD_NO_ */
	{
		inMultiFunc_HostInital,
		inMultiFunc_Host_RecePacket,
		inMultiFunc_Host_SendPacketDotWaitAck,
		inMultiFunc_Host_SendError,
		inMultiFunc_Host_End
	},/* 第四組 - _MULTI_SLAVE_REBOOT_NO_ */
	{
		inMultiFunc_HostInital,
		inMultiFunc_Host_RecePacket,
		inMultiFunc_Host_SendPacketDotWaitAck,
		inMultiFunc_Host_SendError,
		inMultiFunc_Host_End
	},

};

int inMultiFunc_CopyDataInToTheHostData(char* szSourceData)
{
	memset(st_uszMultiHostCTLSData, 0x00, sizeof(st_uszMultiHostCTLSData));
	memcpy(st_uszMultiHostCTLSData, szSourceData, strlen(szSourceData));
	return VS_SUCCESS;
}

unsigned char* szMultiFunc_GetHostData()
{
	return st_uszMultiHostCTLSData;
}

/*
Function        : inMulti_HOST_PrintReceiveDeBug
Date&Time   : 2016/11/1 上午 11:06
Describe        : 印收到的ISODeBug
*/
int inMulti_HOST_PrintReceiveDeBug(char *szDataBuffer, unsigned short usReceiveBufferSize, int inDataSize)
{
//	int			i, inTempLen;
	int			inPrintPosition, inISOPosition;	/* ISODebug使用 */
	char			szDebugMsg[5124 + 1];	/* 會用來印400ECR，所以放大一點 */
	char			szTemplate[164 + 1],szTemplate2[6+1];
	
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetECRVersion(szTemplate);
		if (memcmp(szTemplate, "0", 1) == 0)
		{
			sprintf(szDebugMsg, "===============(%03d)(標準收_%d_BYTE)", usReceiveBufferSize, inDataSize);
		}
	
		inDISP_LogPrintf(szDebugMsg);
	
		
		inPrintPosition = 0; /* 在紙上要從哪裡開始印，因為前面要空LeftTag的位置 */
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			
		for (inISOPosition = 0; inISOPosition < usReceiveBufferSize; inISOPosition++)
		{
			if(szDataBuffer[inISOPosition] >= 0x20 && szDataBuffer[inISOPosition] <= 0x7F)
			{
				if(szDataBuffer[inISOPosition] == 0x20 )
					szDebugMsg[inPrintPosition ++] = 0x23;
				else	
					szDebugMsg[inPrintPosition ++] = szDataBuffer[inISOPosition];
			}else
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate2, 0x00, sizeof(szTemplate2));
				inFunc_BCD_to_ASCII(&szTemplate2[0],(unsigned char*) &szDataBuffer[inISOPosition], 1);				
				sprintf(szTemplate,"[%s]",szTemplate2);
				sprintf((char *)&szDebugMsg[inPrintPosition],szTemplate,strlen(szTemplate));
				inPrintPosition += strlen(szTemplate);
			}
		}
		/* 把最後那一行印出來 */
		inDISP_LogPrintf(szDebugMsg);

		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetECRVersion(szTemplate);
		if (memcmp(szTemplate, "0", 1) == 0)
		{
			sprintf(szDebugMsg, "==================(標準結束_%03d_BYTE)", inDataSize);
		}
		inDISP_LogPrintf(szDebugMsg);
	}
	
	return (VS_SUCCESS);
}



int inMultiFunc_DispTagValue(char fDebugFlag,char *chMsg,int inMsgLen,BYTE *chData,int inDataLen)
{
	BYTE *bBuf;
	int i;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	bBuf = (BYTE *)calloc(1, inDataLen * 4 + inMsgLen + 10);
	memset(bBuf, 0x00, inDataLen * 3 + inMsgLen + 10);
	memcpy(bBuf,chMsg,inMsgLen);
	sprintf((char *)&bBuf[strlen((char *)bBuf)]," (%d):",inDataLen);
	for(i=0;i<inDataLen;i++)
	{
		if(fDebugFlag ==FALSE)
		{
			if(chData[i] >= 0x20 && chData[i] <= 0x7F)
				bBuf[strlen((char *)bBuf)] = chData[i];
			else
				sprintf((char *)&bBuf[strlen((char *)bBuf)],"[%02X]",chData[i]);
		}
		else
			sprintf((char *)&bBuf[strlen((char *)bBuf)],"%02X",chData[i]);
	}
	inDISP_LogPrintf((char *)bBuf);
	free(bBuf);
	
	
//	memset(szPrintTag, 0x00, sizeof(szPrintTag));
//	if (strlen(szTagData) < 20)
//	{
//		inDISP_LogPrintfWithFlag(" Tag %s (%02d)[%s]", szTag, ushTaglen, szTagData);
//	}
//	else
//	{
//		inDISP_LogPrintfWithFlag(" Tag %s (%02d)", szTag, ushTaglen);
//
//		inPrintLineCnt = 0;
//		while ((inPrintLineCnt * inOneLineLen) < strlen(szTagData))
//		{
//			memset(szPrintLineData, 0x00, sizeof(szPrintLineData));
//			memset(szPrintTag, 0x00, sizeof(szPrintTag));
//			if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szTagData))
//			{
//				strcat(szPrintLineData, &szTagData[inPrintLineCnt * inOneLineLen]);
//			}
//			else
//			{
//				memcpy(szPrintLineData, &szTagData[inPrintLineCnt * inOneLineLen], inOneLineLen);
//			}
//			inDISP_LogPrintfWithFlag(" [%s]", szPrintLineData);
//			inPrintLineCnt ++;
//		};
//	}
//	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

int inMultiFunc_AnalysisEmvData(TRANSACTION_OBJECT *pobTran, unsigned char *uszCTLSData , int inDataSizes)
{
	int inCnt;
	char	szSerialNumber[16 + 1];
	char	szTagName[12] ={0};
	unsigned short	ushTagLen;
	unsigned char	uszTagData[128+1] ;	
	unsigned char	uszTag5F2A = VS_FALSE, uszTag9F1A = VS_FALSE;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	for (inCnt =0 ; inCnt < inDataSizes ;)
	{
		
		if (uszCTLSData[inCnt] == 0x4F)
		{
			inCnt += 1;
			ushTagLen = uszCTLSData[inCnt ++];
			memset(uszTagData, 0x00, sizeof(uszTagData));
			memcpy((char *)&uszTagData[0], &uszCTLSData[inCnt], ushTagLen);
			inCnt += ushTagLen;
			
			inDISP_LogPrintfArea(TRUE, "4F", 2,  uszTagData, ushTagLen);
			continue;
		}

	
		if (uszCTLSData[inCnt] == 0x50)
		{
			inCnt += 1;
			pobTran->srEMVRec.in50_APLabelLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz50_APLabel, 0x00, sizeof(pobTran->srEMVRec.usz50_APLabel));
			memcpy((char *)&pobTran->srEMVRec.usz50_APLabel[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in50_APLabelLen);
			inCnt += pobTran->srEMVRec.in50_APLabelLen;

			inDISP_LogPrintfArea(FALSE, "50", 2,  pobTran->srEMVRec.usz50_APLabel, pobTran->srEMVRec.in50_APLabelLen);

			continue;
		}

		if (uszCTLSData[inCnt] == 0x55)
		{
			inCnt += 1;
			ushTagLen = uszCTLSData[inCnt ++];
			memset(uszTagData, 0x00, sizeof(uszTagData));
			memcpy((char *)&uszTagData[0], &uszCTLSData[inCnt], ushTagLen);
			inCnt += ushTagLen;

			if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3 && uszTagData[inCnt] == 0x01)
				pobTran->srBRec.uszNoSignatureBit = VS_TRUE; /* VISA Paywave3 免簽名條件 */
			else
				pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* VISA Paywave3 免簽名條件 */

			pobTran->uszPayWave3Tag55Bit = VS_TRUE;

			inDISP_LogPrintfArea(TRUE, "55", 2,  uszTagData, ushTagLen);
			continue;
		}

		if (uszCTLSData[inCnt] == 0x57)
		{
			inCnt += 1;
			ushTagLen = uszCTLSData[inCnt ++];
			memset(uszTagData, 0x00, sizeof(uszTagData));
			memcpy((char *)&uszTagData[0], &uszCTLSData[inCnt], ushTagLen);
			inCnt += ushTagLen;

			/* 組TAG_57 */
			memset(pobTran->usz57_Track2, 0x00, sizeof(pobTran->usz57_Track2));
			memcpy(&pobTran->usz57_Track2[0], (char *)&uszTagData[0], ushTagLen);
			pobTran->in57_Track2Len = ushTagLen;
			
			inDISP_LogPrintfArea(TRUE, "57", 2,  uszTagData, ushTagLen);
			continue;
		}

		if (uszCTLSData[inCnt] == 0x5A)
		{
			inCnt += 1;
			pobTran->srEMVRec.in5A_ApplPanLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz5A_ApplPan, 0x00, sizeof(pobTran->srEMVRec.usz5A_ApplPan));
			memcpy((char *)&pobTran->srEMVRec.usz5A_ApplPan[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in5A_ApplPanLen);
			inCnt += pobTran->srEMVRec.in5A_ApplPanLen;

			inDISP_LogPrintfArea(TRUE, "5A", 2,  pobTran->srEMVRec.usz5A_ApplPan, pobTran->srEMVRec.in5A_ApplPanLen);
			continue;
		}

		if (uszCTLSData[inCnt] == 0x5F && uszCTLSData[inCnt + 1] == 0x20)
		{
			inCnt += 2;
			ushTagLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz5F20_CardholderName, 0x00, sizeof(pobTran->srEMVRec.usz5F20_CardholderName));
			memset(uszTagData, 0x00, sizeof(uszTagData));
			memcpy(&pobTran->srEMVRec.usz5F20_CardholderName[0], &uszCTLSData[inCnt], ushTagLen);
			inCnt += ushTagLen;
			
			inDISP_LogPrintfArea(FALSE, "5F20", 4,  pobTran->srEMVRec.usz5F20_CardholderName, ushTagLen);
			continue;
		}

		if (uszCTLSData[inCnt] == 0x5F && uszCTLSData[inCnt + 1] == 0x24)
		{
			inCnt += 2;
			pobTran->srEMVRec.in5F24_ExpireDateLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz5F24_ExpireDate, 0x00, sizeof(pobTran->srEMVRec.usz5F24_ExpireDate));
			memcpy((char *)&pobTran->srEMVRec.usz5F24_ExpireDate[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in5F24_ExpireDateLen);
			inCnt += pobTran->srEMVRec.in5F24_ExpireDateLen;
			
			inDISP_LogPrintfArea(TRUE, "5F24", 4,  pobTran->srEMVRec.usz5F24_ExpireDate, pobTran->srEMVRec.in5F24_ExpireDateLen);

			continue;
		}

		if (uszCTLSData[inCnt] == 0x5F && uszCTLSData[inCnt + 1] == 0x2A)
		{
			inCnt += 2;
			pobTran->srEMVRec.in5F2A_TransCurrCodeLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz5F2A_TransCurrCode, 0x00, sizeof(pobTran->srEMVRec.usz5F2A_TransCurrCode));
			memcpy((char *)&pobTran->srEMVRec.usz5F2A_TransCurrCode[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);
			inCnt += pobTran->srEMVRec.in5F2A_TransCurrCodeLen;

			if (pobTran->srEMVRec.usz5F2A_TransCurrCode[0] == 0x09 && pobTran->srEMVRec.usz5F2A_TransCurrCode[1] == 0x01)
				uszTag5F2A = VS_TRUE;
			
			inDISP_LogPrintfArea(TRUE, "5F2A", 4,  pobTran->srEMVRec.usz5F2A_TransCurrCode, pobTran->srEMVRec.in5F2A_TransCurrCodeLen);

			continue;
		}

		if (uszCTLSData[inCnt] == 0x5F && uszCTLSData[inCnt + 1] == 0x34)
		{
			inCnt += 2;
			pobTran->srEMVRec.in5F34_ApplPanSeqnumLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz5F34_ApplPanSeqnum, 0x00, sizeof(pobTran->srEMVRec.usz5F34_ApplPanSeqnum));
			memcpy((char *)&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
			inCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
			
			inDISP_LogPrintfArea(TRUE, "5F34", 4,  pobTran->srEMVRec.usz5F34_ApplPanSeqnum, pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
			continue;
		}

		if (uszCTLSData[inCnt] == 0x82)
		{
			inCnt += 1;
			pobTran->srEMVRec.in82_AIPLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz82_AIP, 0x00, sizeof(pobTran->srEMVRec.usz82_AIP));
			memcpy((char *)&pobTran->srEMVRec.usz82_AIP[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in82_AIPLen);
			inCnt += pobTran->srEMVRec.in82_AIPLen;

			inDISP_LogPrintfArea(TRUE, "82", 2,  pobTran->srEMVRec.usz82_AIP, pobTran->srEMVRec.in82_AIPLen);
			continue;
		}

		if (uszCTLSData[inCnt] == 0x84)
		{
			inCnt += 1;
			pobTran->srEMVRec.in84_DFNameLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz84_DF_NAME, 0x00, sizeof(pobTran->srEMVRec.usz84_DF_NAME));
			memcpy((char *)&pobTran->srEMVRec.usz84_DF_NAME[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in84_DFNameLen);

			/* 銀聯需要AID */
			memset(pobTran->srBRec.szCUP_EMVAID, 0x00, sizeof(pobTran->srBRec.szCUP_EMVAID));
			inFunc_BCD_to_ASCII(pobTran->srBRec.szCUP_EMVAID, pobTran->srEMVRec.usz84_DF_NAME, pobTran->srEMVRec.in84_DFNameLen);

			inCnt += pobTran->srEMVRec.in84_DFNameLen;
			
			inDISP_LogPrintfArea(TRUE, "84", 2,  pobTran->srEMVRec.usz84_DF_NAME, pobTran->srEMVRec.in84_DFNameLen);

			continue;
		}

		if (uszCTLSData[inCnt] == 0x8A)
		{
			inCnt += 1;
			pobTran->srEMVRec.in8A_AuthRespCodeLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz8A_AuthRespCode, 0x00, sizeof(pobTran->srEMVRec.usz8A_AuthRespCode));
			memcpy((char *)&pobTran->srEMVRec.usz8A_AuthRespCode[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in8A_AuthRespCodeLen);
			inCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
			
			inDISP_LogPrintfArea(TRUE, "8A", 2,  pobTran->srEMVRec.usz8A_AuthRespCode, pobTran->srEMVRec.in8A_AuthRespCodeLen);
		
			continue;
		}

		if (uszCTLSData[inCnt] == 0x95)
		{
			inCnt += 1;
			pobTran->srEMVRec.in95_TVRLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz95_TVR, 0x00, sizeof(pobTran->srEMVRec.usz95_TVR));
			memcpy((char *)&pobTran->srEMVRec.usz95_TVR[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in95_TVRLen);
			inCnt += pobTran->srEMVRec.in95_TVRLen;
			
			inDISP_LogPrintfArea(TRUE, "95", 2,  pobTran->srEMVRec.usz95_TVR, pobTran->srEMVRec.in95_TVRLen);
			
			continue;
		}

		/* Tag 99 For CUP DEBIT */
		if (uszCTLSData[inCnt] == 0x99)
		{
			inCnt += 1;
			ushTagLen = uszCTLSData[inCnt ++];
			memset(uszTagData, 0x00, sizeof(uszTagData));
			memcpy((char *)&uszTagData[0], &uszCTLSData[inCnt], ushTagLen);
			inCnt += ushTagLen;

			if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS && uszTagData[inCnt] == 0x00)
			{
				pobTran->uszQuickPassTag99 = VS_TRUE;
			}

			inDISP_LogPrintfArea(TRUE, "99", 2,  uszTagData, ushTagLen);

			continue;
		}

		if (uszCTLSData[inCnt] == 0x9A)
		{
			inCnt += 1;
			pobTran->srEMVRec.in9A_TranDateLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz9A_TranDate, 0x00, sizeof(pobTran->srEMVRec.usz9A_TranDate));
			memcpy((char *)&pobTran->srEMVRec.usz9A_TranDate[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9A_TranDateLen);
			inCnt += pobTran->srEMVRec.in9A_TranDateLen;
			
			inDISP_LogPrintfArea(TRUE, "9A", 2,  pobTran->srEMVRec.usz9A_TranDate, pobTran->srEMVRec.in9A_TranDateLen);
			
			continue;
		}

		if (uszCTLSData[inCnt] == 0x9B)
		{
			inCnt += 1;
			pobTran->srEMVRec.in9B_TSILen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz9B_TSI, 0x00, sizeof(pobTran->srEMVRec.usz9B_TSI));
			memcpy((char *)&pobTran->srEMVRec.usz9B_TSI[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9B_TSILen);
			inCnt += pobTran->srEMVRec.in9B_TSILen;
			
			inDISP_LogPrintfArea(TRUE, "9B", 2,  pobTran->srEMVRec.usz9B_TSI, pobTran->srEMVRec.in9B_TSILen);
			
			continue;
		}

		if (uszCTLSData[inCnt] == 0x9C)
		{
			inCnt += 1;
			pobTran->srEMVRec.in9C_TranTypeLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz9C_TranType, 0x00, sizeof(pobTran->srEMVRec.usz9C_TranType));
			memcpy((char *)&pobTran->srEMVRec.usz9C_TranType[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9C_TranTypeLen);
			inCnt += pobTran->srEMVRec.in9C_TranTypeLen;
			
			inDISP_LogPrintfArea(TRUE, "9C", 2,  pobTran->srEMVRec.usz9C_TranType, pobTran->srEMVRec.in9C_TranTypeLen);
			
			continue;
		}

		if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x02)
		{
			inCnt += 2;
			pobTran->srEMVRec.in9F02_AmtAuthNumLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz9F02_AmtAuthNum, 0x00, sizeof(pobTran->srEMVRec.usz9F02_AmtAuthNum));
			memcpy((char *)&pobTran->srEMVRec.usz9F02_AmtAuthNum[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F02_AmtAuthNumLen);
			inCnt += pobTran->srEMVRec.in9F02_AmtAuthNumLen;
			
			inDISP_LogPrintfArea(TRUE, "9F02", 4,  pobTran->srEMVRec.usz9F02_AmtAuthNum, pobTran->srEMVRec.in9F02_AmtAuthNumLen);
			
			continue;
		}

		if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x03)
		{
			inCnt += 2;
			pobTran->srEMVRec.in9F03_AmtOtherNumLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz9F03_AmtOtherNum, 0x00, sizeof(pobTran->srEMVRec.usz9F03_AmtOtherNum));
			memcpy((char *)&pobTran->srEMVRec.usz9F03_AmtOtherNum[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F03_AmtOtherNumLen);
			inCnt += pobTran->srEMVRec.in9F03_AmtOtherNumLen;
			
			inDISP_LogPrintfArea(TRUE, "9F03", 4,  pobTran->srEMVRec.usz9F03_AmtOtherNum, pobTran->srEMVRec.in9F03_AmtOtherNumLen);
		
			continue;
		}

		if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x08)
		{
			inCnt += 2;
			ushTagLen = uszCTLSData[inCnt ++];
			memset(uszTagData, 0x00, sizeof(uszTagData));
			memcpy((char *)&uszTagData[0], &uszCTLSData[inCnt], ushTagLen);
			inCnt += ushTagLen;
			
			inDISP_LogPrintfArea(TRUE, "9F08", 4,  uszTagData, ushTagLen);
			
			continue;
		}

		if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x09)
		{
			inCnt += 2;
			ushTagLen = uszCTLSData[inCnt ++];
			memset(uszTagData, 0x00, sizeof(uszTagData));
			memcpy((char *)&uszTagData[0], &uszCTLSData[inCnt], ushTagLen);
			inCnt += ushTagLen;
			
			inDISP_LogPrintfArea(TRUE, "9F09", 4,  uszTagData, ushTagLen);
					
			continue;
		}

		if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x10)
		{
			inCnt += 2;
			pobTran->srEMVRec.in9F10_IssuerAppDataLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz9F10_IssuerAppData, 0x00, sizeof(pobTran->srEMVRec.usz9F10_IssuerAppData));
			memcpy((char *)&pobTran->srEMVRec.usz9F10_IssuerAppData[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F10_IssuerAppDataLen);
			inCnt += pobTran->srEMVRec.in9F10_IssuerAppDataLen;
			
			inDISP_LogPrintfArea(TRUE, "9F10", 4,  pobTran->srEMVRec.usz9F10_IssuerAppData, pobTran->srEMVRec.in9F10_IssuerAppDataLen);

			continue;
		}

		if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x1A)
		{
			inCnt += 2;
			pobTran->srEMVRec.in9F1A_TermCountryCodeLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz9F1A_TermCountryCode, 0x00, sizeof(pobTran->srEMVRec.usz9F1A_TermCountryCode));
			memcpy((char *)&pobTran->srEMVRec.usz9F1A_TermCountryCode[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);
			inCnt += pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
			uszTag9F1A = VS_TRUE;
			
			inDISP_LogPrintfArea(TRUE, "9F1A", 4,  pobTran->srEMVRec.usz9F1A_TermCountryCode, pobTran->srEMVRec.in9F1A_TermCountryCodeLen);
		
			continue;
		}

		if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x1E)
		{
			inCnt += 2;
			pobTran->srEMVRec.in9F1E_IFDNumLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz9F1E_IFDNum, 0x00, sizeof(pobTran->srEMVRec.usz9F1E_IFDNum));
			memcpy((char *)&pobTran->srEMVRec.usz9F1E_IFDNum[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F1E_IFDNumLen);
			inCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
			
			inDISP_LogPrintfArea(TRUE, "9F1E", 4,  pobTran->srEMVRec.usz9F1E_IFDNum, pobTran->srEMVRec.in9F1E_IFDNumLen);

			continue;
		}

		if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x21)
		{
			inCnt += 2;
			ushTagLen = uszCTLSData[inCnt ++];
			memset(uszTagData, 0x00, sizeof(uszTagData));
			memcpy((char *)&uszTagData[0], &uszCTLSData[inCnt], ushTagLen);
			inCnt += ushTagLen;
			
			inDISP_LogPrintfArea(TRUE, "9F21", 4,  uszTagData, ushTagLen);
			
			continue;
		}

		if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x26)
		{
			inCnt += 2;
			pobTran->srEMVRec.in9F26_ApplCryptogramLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz9F26_ApplCryptogram, 0x00, sizeof(pobTran->srEMVRec.usz9F26_ApplCryptogram));
			memcpy((char *)&pobTran->srEMVRec.usz9F26_ApplCryptogram[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F26_ApplCryptogramLen);
			inCnt += pobTran->srEMVRec.in9F26_ApplCryptogramLen;

			inDISP_LogPrintfArea(TRUE, "9F26", 4,  pobTran->srEMVRec.usz9F26_ApplCryptogram, pobTran->srEMVRec.in9F26_ApplCryptogramLen);
				
			continue;
		}

		if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x27)
		{
			inCnt += 2;
			pobTran->srEMVRec.in9F27_CIDLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz9F27_CID, 0x00, sizeof(pobTran->srEMVRec.usz9F27_CID));
			memcpy((char *)&pobTran->srEMVRec.usz9F27_CID[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F27_CIDLen);
			inCnt += pobTran->srEMVRec.in9F27_CIDLen;

			inDISP_LogPrintfArea(TRUE, "9F27", 4,  pobTran->srEMVRec.usz9F27_CID, pobTran->srEMVRec.in9F27_CIDLen);
			
			continue;
		}

		if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x33)
		{
			inCnt += 2;
			pobTran->srEMVRec.in9F33_TermCapabilitiesLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz9F33_TermCapabilities, 0x00, sizeof(pobTran->srEMVRec.usz9F33_TermCapabilities));
			memcpy((char *)&pobTran->srEMVRec.usz9F33_TermCapabilities[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
			inCnt += pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
			
			inDISP_LogPrintfArea(TRUE, "9F33", 4,  pobTran->srEMVRec.usz9F33_TermCapabilities, pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
			
			continue;
		}

		if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x34)
		{
			inCnt += 2;
			pobTran->srEMVRec.in9F34_CVMLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz9F34_CVM, 0x00, sizeof(pobTran->srEMVRec.usz9F34_CVM));
			memcpy((char *)&pobTran->srEMVRec.usz9F34_CVM[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F34_CVMLen);
			inCnt += pobTran->srEMVRec.in9F34_CVMLen;
			
			inDISP_LogPrintfArea(TRUE, "9F34", 4,  pobTran->srEMVRec.usz9F34_CVM, pobTran->srEMVRec.in9F34_CVMLen);
			
			continue;
		}

		if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x35)
		{
			inCnt += 2;
			pobTran->srEMVRec.in9F35_TermTypeLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz9F35_TermType, 0x00, sizeof(pobTran->srEMVRec.usz9F35_TermType));
			memcpy((char *)&pobTran->srEMVRec.usz9F35_TermType[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F35_TermTypeLen);
			inCnt += pobTran->srEMVRec.in9F35_TermTypeLen;
			
			inDISP_LogPrintfArea(TRUE, "9F35", 4,  pobTran->srEMVRec.usz9F35_TermType, pobTran->srEMVRec.in9F35_TermTypeLen);
			
			continue;
		}

		if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x36)
		{
			inCnt += 2;
			pobTran->srEMVRec.in9F36_ATCLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz9F36_ATC, 0x00, sizeof(pobTran->srEMVRec.usz9F36_ATC));
			memcpy((char *)&pobTran->srEMVRec.usz9F36_ATC[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F36_ATCLen);
			inCnt += pobTran->srEMVRec.in9F36_ATCLen;
			
			inDISP_LogPrintfArea(TRUE, "9F36", 4,  pobTran->srEMVRec.usz9F36_ATC, pobTran->srEMVRec.in9F36_ATCLen);
	
			continue;
		}

		if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x37)
		{
			inCnt += 2;
			pobTran->srEMVRec.in9F37_UnpredictNumLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz9F37_UnpredictNum, 0x00, sizeof(pobTran->srEMVRec.usz9F37_UnpredictNum));
			memcpy((char *)&pobTran->srEMVRec.usz9F37_UnpredictNum[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F37_UnpredictNumLen);
			inCnt += pobTran->srEMVRec.in9F37_UnpredictNumLen;
			
			inDISP_LogPrintfArea(TRUE, "9F37", 4,  pobTran->srEMVRec.usz9F37_UnpredictNum, pobTran->srEMVRec.in9F37_UnpredictNumLen);
		
			continue;
		}

		if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x41)
		{
			inCnt += 2;
			pobTran->srEMVRec.in9F41_TransSeqCounterLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz9F41_TransSeqCounter, 0x00, sizeof(pobTran->srEMVRec.usz9F41_TransSeqCounter));
			memcpy((char *)&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
			inCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
			
			inDISP_LogPrintfArea(TRUE, "9F41", 4,  pobTran->srEMVRec.usz9F41_TransSeqCounter, pobTran->srEMVRec.in9F41_TransSeqCounterLen);
			
			continue;
		}

		if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x45)
		{
			inCnt += 2;
			ushTagLen = uszCTLSData[inCnt ++];
			memset(uszTagData, 0x00, sizeof(uszTagData));
			memcpy((char *)&uszTagData[0], &uszCTLSData[inCnt], ushTagLen);
			inCnt += ushTagLen;
			
			inDISP_LogPrintfArea(TRUE, "9F45", 4,  uszTagData, ushTagLen);
		
			continue;
		}

		if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x53)
		{
			inCnt += 2;
			ushTagLen = uszCTLSData[inCnt ++];
			memset(uszTagData, 0x00, sizeof(uszTagData));
			memcpy((char *)&uszTagData[0], &uszCTLSData[inCnt], ushTagLen);
			inCnt += ushTagLen;
			
			inDISP_LogPrintfArea(TRUE, "9F53", 4,  uszTagData, ushTagLen);
		
			continue;
		}

		if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x66)
		{
			inCnt += 2;
			pobTran->srEMVRec.in9F66_QualifiersLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz9F66_Qualifiers, 0x00, sizeof(pobTran->srEMVRec.usz9F66_Qualifiers));
			memcpy((char *)&pobTran->srEMVRec.usz9F66_Qualifiers[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F66_QualifiersLen);
			inCnt += pobTran->srEMVRec.in9F66_QualifiersLen;

			inDISP_LogPrintfArea(TRUE, "9F66", 4,  pobTran->srEMVRec.usz9F66_Qualifiers, pobTran->srEMVRec.in9F66_QualifiersLen);
			
			continue;
		}

		if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x6E)
		{
			inCnt += 2;
			pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.usz9F6E_From_Factor_Indicator, 0x00, sizeof(pobTran->srEMVRec.usz9F6E_From_Factor_Indicator));
			memcpy((char *)&pobTran->srEMVRec.usz9F6E_From_Factor_Indicator[0], &uszCTLSData[inCnt], pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen);
			inCnt += pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen;
			
			inDISP_LogPrintfArea(TRUE, "9F6E", 4,  pobTran->srEMVRec.usz9F6E_From_Factor_Indicator, pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen);
		
			continue;
		}

		if (uszCTLSData[inCnt] == 0x9F && uszCTLSData[inCnt + 1] == 0x74)
		{
			inCnt += 2;
			ushTagLen = uszCTLSData[inCnt ++];
			memset(uszTagData, 0x00, sizeof(uszTagData));
			memcpy((char *)&uszTagData[0], &uszCTLSData[inCnt], ushTagLen);

			/* 感應退貨不能塞VLP */
			if (pobTran->inTransactionCode != _REFUND_	&&
			pobTran->inTransactionCode != _CUP_REFUND_)
			{
				memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
				memcpy((char *)&pobTran->srBRec.szAuthCode[0], uszTagData, ushTagLen);
			}
			
			inCnt += ushTagLen;

			inDISP_LogPrintfArea(TRUE, "9F74", 4,  uszTagData, ushTagLen);	

			continue;
		}

		if (uszCTLSData[inCnt] == 0xDF && uszCTLSData[inCnt + 1] == 0x8F && uszCTLSData[inCnt + 2] == 0x4F)
		{
			/*
			   Add By Tim 2015-06-16 AM 10:09:51
			   DF8F4F Responded Advanced Transaction Result
			   0001h    PayPass MSD Online
			   0002h    Offline Approval
			   0003h    Offline Declined
			   0004h    Online Transaction
			   0005h    VisaWave MSD Online

			   Tag DF8F4F Responded Advanced Transaction Result，是判斷QuickPass、NewJspeedy交易最後的結果。
			   Tag是QP3000S自訂的Tag。
			*/
			inCnt += 3;
			pobTran->srEMVRec.inDF8F4F_TransactionResultLen = uszCTLSData[inCnt ++];
			memset(pobTran->srEMVRec.uszDF8F4F_TransactionResult, 0x00, sizeof(pobTran->srEMVRec.uszDF8F4F_TransactionResult));
			memcpy((char *)&pobTran->srEMVRec.uszDF8F4F_TransactionResult[0], &uszCTLSData[inCnt], pobTran->srEMVRec.inDF8F4F_TransactionResultLen);
			inCnt += pobTran->srEMVRec.inDF8F4F_TransactionResultLen;
			
			inDISP_LogPrintfArea(TRUE, "DF8F4F", 6,  pobTran->srEMVRec.uszDF8F4F_TransactionResult, pobTran->srEMVRec.inDF8F4F_TransactionResultLen);	
			
			continue;
		}

		/* EMV規格設計 欄為兩格的為 5F 9F DF，則前面第一個值和0x1F and會等於0x1F */
		/* 無法解開的值 */
		/* 有一些自定義的Tag值，像DF8129，也有DF8Fxx之類的值，所以用0x80來分 */
		if ((uszCTLSData[inCnt] & 0xDF) == 0xDF && (uszCTLSData[inCnt + 1] & 0x80) == 0x80)
		{
			memset(szTagName, 0x00, sizeof(szTagName));
			sprintf(szTagName, "%02X%02X%02X",uszCTLSData[inCnt],uszCTLSData[inCnt+1],uszCTLSData[inCnt+2] );
			inCnt += 3;
			ushTagLen = uszCTLSData[inCnt ++];
			memset(uszTagData, 0x00, sizeof(uszTagData));
			memcpy(uszTagData, &uszCTLSData[inCnt], ushTagLen);
			inCnt += ushTagLen;

			inDISP_LogPrintfArea(TRUE, szTagName, strlen(szTagName) , uszTagData, ushTagLen);	

			continue;
		}
		else if ((uszCTLSData[inCnt] & 0x1F) == 0x1F)
		{
			memset(szTagName, 0x00, sizeof(szTagName));
			sprintf(szTagName, "%02X%02X",uszCTLSData[inCnt],uszCTLSData[inCnt+1]);
			inCnt += 2;
			ushTagLen = uszCTLSData[inCnt ++];
			memset(uszTagData, 0x00, sizeof(uszTagData));
			memcpy(uszTagData, &uszCTLSData[inCnt], ushTagLen);
			inCnt += ushTagLen;
			
			inDISP_LogPrintfArea(TRUE, szTagName, strlen(szTagName), uszTagData, ushTagLen);	
			
			continue;
		}
		else
		{
			inCnt += 1;
			ushTagLen = uszCTLSData[inCnt ++];
			memset(uszTagData, 0x00, sizeof(uszTagData));
			memcpy(uszTagData, &uszCTLSData[inCnt], ushTagLen);
			inCnt += ushTagLen;
			
			inDISP_LogPrintfArea(TRUE, "OTHER", 5,  uszTagData, ushTagLen);	
			continue;
		}

		inCnt ++;
	} /* End for () .... */	
	
	if (uszTag5F2A == VS_TRUE && uszTag9F1A == VS_TRUE)
	{
		/* 表示是以本國貨幣交易 */
		memset(pobTran->srEMVRec.usz9F1A_TermCountryCode, 0x00, sizeof(pobTran->srEMVRec.usz9F1A_TermCountryCode));
		pobTran->srEMVRec.usz9F1A_TermCountryCode[0] = 0x01;
		pobTran->srEMVRec.usz9F1A_TermCountryCode[1] = 0x58;
		pobTran->srEMVRec.inDF8F4F_TransactionResultLen = 2;
	}

	/* Serial Number自己塞 */
	if (pobTran->srEMVRec.in9F1E_IFDNumLen == 0)
	{
		memset(szSerialNumber, 0x00, sizeof(szSerialNumber));
		inFunc_GetSeriaNumber(szSerialNumber);

		pobTran->srEMVRec.in9F1E_IFDNumLen = 8;
		memcpy((unsigned char*)pobTran->srEMVRec.usz9F1E_IFDNum, &szSerialNumber[7], pobTran->srEMVRec.in9F1E_IFDNumLen);
	}

	return VS_SUCCESS;
	
}


/*
App Name        : inCTLS_VIVO_CheckErrorCodes
App Builder     : Tusin
App Date&Time   : 2014/8/14 上午 11:21:14
App Function    : 分析感應交易錯誤碼
*/
int inCTLS_VIVO_CheckErrorCodes(TRANSACTION_OBJECT *pobTran, unsigned char *uszErrorCodes, int inLen)
{
	int	inCnt = 0, inRetVal = VS_ERROR, inLEN;
	char	szDispMsg1[32 + 1], szDispMsg2[32 + 1];
//	char    szCheckSELFMODE[1+1];/*無人自助模式開關*/
	unsigned char uszKey;
	
	VS_BOOL fDispMsg = VS_FALSE, fResult = VS_TRUE;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	inDISP_LogPrintfWithFlag("==============================================================================");
	/* Status Codes 【11 BYTE】
		00h OK
		01h Incorrect Header Tag
		02h Unknown Command
		03h Unknown Sub-Command
		04h CRC Error in Packet
		05h Incorrect Parameter
		06h Parameter Not Supported
		07h Mal-formatted Data
		08h Timeout
		0Ah Failed / Nak
		0Bh Command Not Allowed
		0Ch Sub-Command Not Allowed
		0Dh Buffer Overflow (data length too large for Reader buffer)
		0Eh User Interface Event
		23h Request Online Authorization
	*/
	inDISP_LogPrintfWithFlag("  Status_Codes_[11 BYTE][%02X]", uszErrorCodes[inCnt]);
	inCnt ++;
	/* Data Lenght */
	inCnt += 2;
	/* Error Codes 【14 BYTE】
		00h No Error None.
		02h Go to Contact Interface
		03h Transaction Amount is Zero
		20h Card Returned Error Status
		21h Collision Error
		22h Amount Over Maximum Limit
		23h Request Online Authorization
		25h Card Blocked
		26h Card Expired
		27h Unsupported Card
		30h Card Did Not Respond
		40h Unknown Data Element
		41h Required Data Element(s) Missing
		42h Card Generated AAC
		43h Card Generated ARQC
		44h SDA/DDA Failed (Not Supported by Card)
		50h SDA/DDA/CDDA Failed (CA Public Key)
		51h SDA/DDA/CDDA Failed (Issuer Public Key)
		52h SDA Failed (SSAD)
		53h DDA/CDDA Failed (ICC Public Key)
		54h DDA/CDDA Failed (Dynamic Signature Verification)
		55h Processing Restrictions Failed
		56h Terminal Risk Management (TRM) Failed
		57h Cardholder Verification Failed
		58h Terminal Action Analysis (TAA) Failed
		61h SD Memory Error
	*/
	inDISP_LogPrintfWithFlag("  Error_Codes[14 BYTE][%02X]", uszErrorCodes[inCnt]);
	memset(szDispMsg1, 0x00, sizeof(szDispMsg1));
	memset(szDispMsg2, 0x00, sizeof(szDispMsg2));

	switch (uszErrorCodes[inCnt])
	{
		case 0x02 :
			break;
		case 0x44 :
		case 0x50 :
		case 0x51 :
		case 0x52 :
		case 0x53 :
		case 0x54 :
		case 0x55 :
		case 0x56 :
		case 0x57 :
		case 0x58 :
			strcpy(szDispMsg1, "  卡片驗證失敗    ");

			if (pobTran->inTransactionCode == _REFUND_)
				strcpy(szDispMsg2, "    請改刷卡  ");
			else
			        strcpy(szDispMsg2, "  請改讀晶片卡    ");
			fDispMsg = VS_TRUE;
			break;
		case 0x00 :
		case 0x03 :
		case 0x20 :
		case 0x21 :
		case 0x22 :
		case 0x23 :
		case 0x25 :
		case 0x26 :
		case 0x27 :
		case 0x30 :
		case 0x40 :
		case 0x41 :
		case 0x42 :
		case 0x43 :
		case 0x61 :
		default :
			strcpy(szDispMsg1, "                  ");
			if (pobTran->inTransactionCode == _REFUND_)
				strcpy(szDispMsg2, "    請改刷卡  ");
			else
				strcpy(szDispMsg2, "  請改刷卡或插卡  ");
			fDispMsg = VS_TRUE;
			break;
	} /* End switch () ... */

	inCnt ++;
	/* SW1SW2 */
	inCnt += 2;
	/* RF State Codes 【17 BYTE】
		00h None		RF State Code not available
		01h PPSE		Error occurred during PPSE command
		02h SELECT		Error occurred during SELECT command
		03h GPO			Error occurred during GET PROCESSING OPTIONS command
		04h READ RECORD		Error occurred during READ RECORD command
		05h GEN AC 		Error occurred during GEN AC command
		06h CCC 		Error occurred during CCC command
		07h IA			Error occurred during IA command
		08h SDA			Error occurred during SDA command
		09h DDA			Error occurred during DDA command
		0ah CDA			Error occurred during CDA command
		0bh TAA			Error occurred during TAA processing
		0ch UPDATE RECORD	Error occurred during UPDATE RECORD command
		10h GET DATA (Ticket)	Error occurred during GET DATA command to retrieve the Ticket
		11h GET DATA (Ticketing Prof)	Error occurred during GET DATA command to retrieve the Ticketing Profile
		12h GET DATA (Balance)	Error occurred during GET DATA command to retrieve the Balance
		13h GET DATA (All)	Error occurred during GET DATA command to retrieve all data
		20h PUT DATA (Ticket)	Error occurred during PUT DATA command to retrieve the Ticket
	*/
	inDISP_LogPrintfWithFlag("  RF_State_Codes[17 BYTE][%02X]", uszErrorCodes[inCnt]);
	if (fDispMsg == VS_FALSE)
	{
		switch (uszErrorCodes[inCnt])
		{
			case 0x08 :
			case 0x09 :
			case 0x0a :
			case 0x0b :
				strcpy(szDispMsg1, "  卡片驗證失敗    ");

				if (pobTran->inTransactionCode == _REFUND_ )
					strcpy(szDispMsg2, "    請改刷卡  ");
				else
					strcpy(szDispMsg2, "  請改讀晶片卡    ");
				fDispMsg = VS_TRUE;
				break;
			case 0x00 :
			case 0x01 :
			case 0x02 :
			case 0x03 :
			case 0x04 :
			case 0x05 :
			case 0x06 :
			case 0x07 :
			case 0x0c :
			case 0x10 :
			case 0x11 :
			case 0x12 :
			case 0x13 :
			case 0x20 :
			default :
				strcpy(szDispMsg1, "                  ");
				if (pobTran->inTransactionCode == _REFUND_ )
					strcpy(szDispMsg2, "    請改刷卡  ");
				else
					strcpy(szDispMsg2, "  請改刷卡或插卡  ");
				fDispMsg = VS_TRUE;
				break;
		} /* End switch () ... */
	}

	inCnt ++;

	if (uszErrorCodes[inCnt] == 0x00 && uszErrorCodes[inCnt + 1] == 0x00)
	{
		/* 【18】【19】【20】
		[ 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 ]
		==============================================================================
		[ 56 69 56 4F 74 65 63 68 32 00 02 0A 00 2B 02 00 00 09 00 00 00 1F E3 01 C0 ]
		*/

		if (uszErrorCodes[inCnt + 2] == 0x01 && uszErrorCodes[inCnt + 3] == 0xE1 && fTwo_Tap == VS_TRUE)
		{
			/* [ 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 ]
			     ==============================================================================
			    [ 56 69 56 4F 74 65 63 68 32 00 02 0A 01 3A 58 90 00 05 00 00 01 E1 82 01 19 ]
			    [ DF 81 29 08 20 F0 F0 30 38 F0 FF 00 FF 81 05 81 C7 9F 02 06 00 00 00 29 99 ]
			 */
			inCnt += 7;
			fResult = VS_TRUE;
		}
		else
		{
			fResult = VS_FALSE;
		}
	}

	if (fResult == VS_FALSE)
	{
		inDISP_LogPrintfWithFlag("Non_MasterCard_ERROR");
	}
	else
	{
		if (fTwo_Tap == VS_TRUE)
		{
			inDISP_LogPrintfWithFlag("Two_Tap flow");

			inMultiFunc_AnalysisEmvData(pobTran, (unsigned char *)&uszErrorCodes[inCnt], inLen );
			return (VS_SUCCESS);
		}
		else
		{
			memset(szDispMsg1, 0x00, sizeof(szDispMsg1));
			memset(szDispMsg2, 0x00, sizeof(szDispMsg2));
			fDispMsg = VS_FALSE;
			/* 0xDF8129 Outcome Parameter Set */
			inDISP_LogPrintfWithFlag("0xDF8129_Outcome_Parameter_Set[%02X %02X %02X]", uszErrorCodes[inCnt], uszErrorCodes[inCnt + 1], uszErrorCodes[inCnt + 2]);
			inCnt += 3;
			/* Data Lenght */
			inLEN = uszErrorCodes[inCnt];
			inCnt ++;
			/* BYTE1 Status
				0001【0x10】: APPROVED
				0010【0x20】: DECLINED
				0011【0x30】: ONLINE REQUEST
				0100【0x40】: END APPLICATION
				0101【0x50】: SELECT NEXT
				0110【0x60】: TRY ANOTHER INTERFACE
				0111【0x70】: TRY AGAIN
				1111【】: N/A
				Other values: RFU
			*/

			inDISP_LogPrintfWithFlag("-----BYTE1_Status[%02X]", uszErrorCodes[inCnt]);

			switch (uszErrorCodes[inCnt])
			{
				case 0x10 :
					break;

				case 0x60 :
				case 0x70 :
					strcpy(szDispMsg1, "  請重新感應      ");
					strcpy(szDispMsg2, "                  ");
					fDispMsg = VS_TRUE;
					break;
				case 0x20 :
				case 0x30 :
				case 0x40 :
				case 0x50 :
				default :
				
					strcpy(szDispMsg1, "                  ");
					if (pobTran->inTransactionCode == _REFUND_ || pobTran->inTransactionCode == _CUP_REFUND_)
						strcpy(szDispMsg2, "    請改刷卡  ");
					else
						strcpy(szDispMsg2, "  請改刷卡或插卡  ");
					fDispMsg = VS_TRUE;
					break;
			} /* End switch () ... */

			inCnt += inLEN;
			/* 0xDF8116 UI Request Data */
			inDISP_LogPrintfWithFlag("0xDF8116_UI_Request_Data[%02X %02X %02X]", uszErrorCodes[inCnt], uszErrorCodes[inCnt + 1], uszErrorCodes[inCnt + 2]);
			inCnt += 3;
			/* Data Lenght */
			inLEN = uszErrorCodes[inCnt];
			inCnt ++;
			/* BYTE1 Message Identifier
				00010111【0x17】: CARD READ OK
				00100001【0x21】: TRY AGAIN
				00000011【0x03】: APPROVED
				00011010【0x1A】: APPROVED - SIGN
				00000111【0x07】: DECLINED
				00011100【0x1C】: ERROR - OTHER CARD
				00011101【0x1D】: INSERT CARD
				00100000【0x20】: SEE PHONE
				00011011【0x1B】: AUTHORISING – PLEASE WAIT
				00011110【0x1E】: CLEAR DISPLAY
				11111111【】: N/A
				Other values: RFU
			*/
			inDISP_LogPrintfWithFlag("-----BYTE1_Message_Identifier[%02X]", uszErrorCodes[inCnt]);
			inDISP_BEEP(1,0);

			/* 清畫面 */
			inDISP_Clear_Area(1, _LINE_8_4_, 16, _LINE_8_8_, _ENGLISH_FONT_8X16_);

			switch (uszErrorCodes[inCnt])
			{
				case 0x17 :
				case 0x1A :
					inRetVal = VS_CTLS_ONLINE_AUTH;
					break;
				case 0x21 :
					strcpy(szDispMsg1, "  請重新感應      ");
					strcpy(szDispMsg2, "                  ");
					inRetVal = VS_TAP_AGAIN;
					break;
				case 0x03 :
					inRetVal = VS_CTLE_OFFLINE_AUTH;
					break;
				case 0x1D :
					strcpy(szDispMsg1, "                  ");

					if (pobTran->inTransactionCode == _REFUND_ || pobTran->inTransactionCode == _CUP_REFUND_)
						strcpy(szDispMsg2, "    請改刷卡  ");
					else
						strcpy(szDispMsg2, "   請改讀晶片卡   ");
					inRetVal = VS_CTLS_INSERT_CARD;
					break;
				case 0x20 :
					memset(szDispMsg1, 0x00, sizeof(szDispMsg1));
					memset(szDispMsg2, 0x00, sizeof(szDispMsg2));
					inRetVal = VS_TWO_TAP;
					break;
//        			case 0xFF :
//                                        /* 【需求單 - 105019】修改內建感應機型，使用AE感應卡做交易，應改轉磁條交易 by Tusin - 2016/7/27 上午 09:54:26 */
//					/* 正確應該在第三行 modify by LingHsiung 2016-10-13 下午 01:42:27 */
//					strcpy(szDispMsg1, "                  ");
//					//modify by green 170502 新增一般信用卡感應式退貨
//					if (pobTran->inTransactionCode == REFUND || pobTran->inTransactionCode == CUP_REFUND)
//						strcpy(szDispMsg2, "    請改刷卡  ");
//					else
//						strcpy(szDispMsg2, "  請改刷卡或插卡  ");
//
//        				inRetVal = VS_CTLS_ERROR;
//        				fTitle = VS_TRUE;
//        				break;
				case 0x1B :
				case 0x1E :
				case 0x07 :
				case 0x1C :
				default :
					strcpy(szDispMsg1, "                  ");
					if (pobTran->inTransactionCode == _REFUND_ || pobTran->inTransactionCode == _CUP_REFUND_)
						strcpy(szDispMsg2, "    請改刷卡  ");
					else
						strcpy(szDispMsg2, "  請改刷卡或插卡  ");
			
					inRetVal = VS_CTLS_ERROR;
					break;
			} /* End switch () ... */

			inCnt += inLEN;
		}
	}

	inDISP_LogPrintfWithFlag("==============================================================================");
	
	/* 無人自助用 FLAG */
//	memset(szCheckSELFMODE, 0x00, sizeof(szCheckSELFMODE));
//	get_env("#SELFMODEFLAG", szCheckSELFMODE, 1);
	
	if (strlen(szDispMsg1) > 0)
	{
			/* 在這裡要發【CANCEL】 */
			inDISP_ChineseFont("處理中", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);

			inDISP_Clear_Area(1, _LINE_8_4_, 16, _LINE_8_8_, _ENGLISH_FONT_8X16_);
			inDISP_ChineseFont(szDispMsg1, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
			inDISP_ChineseFont(szDispMsg2, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);

//			if(szCheckSELFMODE[0] == '1')
//			{
//				/*無人自助模式開啟時，不應要按鍵而中斷流程 add by yude*/
//				SVC_WAIT(500);
//			}
//			else
			{
				uszKey = 0x00;
				while (1)
				{
//					inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
					uszKey = uszKBD_Key();

					/* Timeout */
					if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
					{
						uszKey = _KEY_TIMEOUT_;
					}

					if (uszKey == _KEY_CANCEL_)
					{
						inRetVal = VS_USER_CANCEL;
						break;
					}
					else if (uszKey == _KEY_TIMEOUT_)
					{
						inRetVal = VS_TIMEOUT;
						break;
					}else if (uszKey == _KEY_0_)
//					else if (inChoice == _BATCH_END_Touch_ENTER_BUTTON_	||
//						 uszKey == _KEY_0_)
					{
						inRetVal = VS_SUCCESS;
						break;
					}
				}
//				/* 清空Touch資料 */
//				inDisTouch_Flush_TouchFile();
			
	   		}
	}


	/* 清除ERROR MSG */
	inDISP_Clear_Area(1, _LINE_8_4_, 16, _LINE_8_8_, _ENGLISH_FONT_8X16_);
	
	inDISP_ChineseFont("外接裝置連接中...", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
	
	return (inRetVal);
}


/*
App Name        : inCTLS_VIVO_CheckStatusCodes
App Builder     : Tusin
App Date&Time   : 2014/8/14 上午 11:23:32
App Function    : 分析感應交易狀態
*/
int inCTLS_VIVO_CheckStatusCodes(unsigned char btStatusCodes)
{
//	int	inCnt = 0, inRetVal;
//	char	szDispMsg[DISP_STR_LEN + 1];
//	VS_BOOL fPressClear = VS_FALSE;

	inDISP_LogPrintfWithFlag("==============================================");
	inDISP_LogPrintfWithFlag("inCTLS_VIVO_CheckStatusCodes()_[11 BYTE][%02X]", btStatusCodes);
	inDISP_LogPrintfWithFlag("==============================================");

	/* Table 22 Status Codes for Version 2 Protocol
		00h OK
		01h Incorrect Header Tag
		02h Unknown Command
		03h Unknown Sub-Command
		04h CRC Error in Packet
		05h Incorrect Parameter
		06h Parameter Not Supported
		07h Mal-formatted Data
		08h Timeout
		0Ah Failed / Nak
		0Bh Command Not Allowed
		0Ch Sub-Command Not Allowed
		0Dh Buffer Overflow (data length too large for Reader buffer)
		0Eh User Interface Event
		23h Request Online Authorization
	*/

	switch (btStatusCodes)
	{
		case 0x00 :
			inDISP_LogPrintfWithFlag(" Return VS_CTLE_OFFLINE_AUTH ");
			return (VS_CTLE_OFFLINE_AUTH);
		/* add by green 150930  */
		case 0x23 :
		case 0x25 :
			inDISP_LogPrintfWithFlag(" Return VS_CTLS_ONLINE_AUTH ");

//			/* modify by green 151021 優化 */
//			if (fGetVXMultiDevice() == VS_FALSE)
//			{
//				inCTLS_VIVO_SetLCDMsgSend(500, VS_TRUE);
//				inCTLS_VIVO_SetLCDMsgRecv(1, 500);
//			}
//			gfStopSale = VS_TRUE;
			return (VS_CTLS_ONLINE_AUTH);
		case 0x01 :
		case 0x02 :
		case 0x03 :
		case 0x04 :
		case 0x05 :
		case 0x06 :
		case 0x07 :
		case 0x08 :
		case 0x0A :
		case 0x0B :
		case 0x0C :
		case 0x0D :
		case 0x0E :
		default :
//			if (fGetVXMultiDevice() == VS_TRUE)
//			{
//				/* -----------觸碰UI----------- */
//				if (fGetGUITouchScreenBit() == VS_TRUE)
//				{
//					/* 在這裡要發【CANCEL】 */
//					inCTLS_VIVO_CancelSend(100, VS_FALSE);
//					inCTLS_VIVO_CancelRecv(2, 500);
//				}
//			}
//			else
//			{
//				/* 在這裡要發【CANCEL】 */
//				inCTLS_VIVO_CancelSend(100, VS_FALSE);
//				inCTLS_VIVO_CancelRecv(2, 500);
//			}
			inDISP_LogPrintfWithFlag(" Return VS_ERROR ");
			return (VS_ERROR);
	}
}

#if 0
int inCTLS_VIVO_ProcessEMV_TLV(TRANSACTION_OBJECT *pobTran, unsigned char *szInputTLVData, int inDataSizes)
{
	int	inCnt = 0;
	int	i, inTLVName = 0;
	int	inOtherTLVLen;
	unsigned int uinTagName;
	unsigned short ushTagLen;
	char	szTemplate[128 + 1], szASCII[256 + 1];
	BYTE	btTagData[256 + 1];
	VS_BOOL  fTag5F2A = VS_FALSE, fTag9F1A = VS_FALSE;

	inDISP_LogPrintfWithFlag("inCTLS_VIVO_ProcessEMV_TLV()_[%d]_START", inDataSizes);
	
	gfCTLS_Check_VISApayWave = VS_FALSE; /* 判斷是【payWave1 = VS_TRUE】或【payWave3 = VS_FALSE】也許會錯 */

	for ( ; inCnt < inDataSizes ;)
	{
		inTLVName = 0;

		if ((szInputTLVData[inCnt] & 0x1F) == 0x1F)
		{
			uinTagName = (szInputTLVData[inCnt] << 8) + szInputTLVData[inCnt + 1];
			/* 2-Byte Tag */
			inTLVName = 2;
		}
		else
		{
			uinTagName = szInputTLVData[inCnt];
			/* 1-Byte Tag */
			inTLVName = 1;
		}

		/* 處理 EMV TLV 長度是【0】 */
		if (inTLVName == 1)
		{
			if (szInputTLVData[inCnt + 1] == 0x00)
			{
				inDISP_LogPrintfWithFlag("           CHECK_TAG_ZERO_[%02X][%02X]", szInputTLVData[inCnt], szInputTLVData[inCnt + 1]);
				inCnt += 2;
				continue;
			}
		}
		else
		{
			if (szInputTLVData[inCnt + 2] == 0x00)
			{
				inDISP_LogPrintfWithFlag("           CHECK_TAG_ZERO_[%02X][%02X][%02X]", szInputTLVData[inCnt], szInputTLVData[inCnt + 1], szInputTLVData[inCnt + 2]);
				inCnt += 3;
				continue;
			}
		}

		switch (uinTagName)
		{
			case 0x4F :
				inCnt += 1;
				ushTagLen = szInputTLVData[inCnt ++];
				memset(btTagData, 0x00, sizeof(btTagData));
				memcpy((char *)&btTagData[0], &szInputTLVData[inCnt], ushTagLen);
//				usEMVAddTLVToCollxn(TAG_4F_AID, (BYTE *)btTagData, ushTagLen);
				inCnt += ushTagLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&btTagData[0], szASCII, ushTagLen);
				LOG_PRINTF(("  TAG_4F[%02d][%s]", ushTagLen, szASCII));
				break;
			case 0x50 :
				inCnt += 1;
				ushTagLen = szInputTLVData[inCnt ++];

				if (ushTagLen > 75)
					pobTran->srEMVRec.us50_APLabel = 75;
				else
					pobTran->srEMVRec.us50_APLabel = ushTagLen;

				memset(pobTran->srEMVRec.b50_APLabel, 0x00, sizeof(pobTran->srEMVRec.b50_APLabel));
				memcpy(&pobTran->srEMVRec.b50_APLabel[0], &szInputTLVData[inCnt], ushTagLen);
				inCnt += ushTagLen;
				LOG_PRINTF(("  TAG_50[%02d][%s]", ushTagLen, pobTran->srEMVRec.b50_APLabel));
				break;
			case 0x55 :
				inCnt += 1;
				ushTagLen = szInputTLVData[inCnt ++];
				memset(btTagData, 0x00, sizeof(btTagData));
				memcpy((char *)&btTagData[0], &szInputTLVData[inCnt], ushTagLen);
				inCnt += ushTagLen;

				/* ViVo免簽名條件待確認 */
//				if (szSchemeID[0] == SCHEME_ID_17_WAVE3 && szInputTLVData[inCnt] == 0x01)
//					pobTran->srBRec.fSignature = VS_TRUE; /* VISA Paywave3 免簽名條件 */
//		        	else
//					pobTran->srBRec.fSignature = VS_FALSE; /* VISA Paywave3 免簽名條件 */

				pobTran->fPayWave3Tag55 = VS_TRUE;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&btTagData[0], szASCII, ushTagLen);
				LOG_PRINTF(("  TAG_55[%02d][%s](Signstur = %c)", ushTagLen, szASCII, (pobTran->srBRec.fSignature) ? '1' : '0'));
				break;
			case 0x57 :
				inCnt += 1;
				ushTagLen = szInputTLVData[inCnt ++];
				memset(btTagData, 0x00, sizeof(btTagData));
				memcpy((char *)&btTagData[0], &szInputTLVData[inCnt], ushTagLen);
				/* 程式會卡死這段 */
//				usEMVAddTLVToCollxn(TAG_57_TRACK2_EQ_DATA, (BYTE *)btTagData, ushTagLen);
				inCnt += ushTagLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&btTagData[0], szASCII, ushTagLen);
				LOG_PRINTF(("  TAG_57[%02d][%s]", ushTagLen, szASCII));

				/* 判斷 VISA payWave1 or payWave3 */
				if (strlen(pobTran->srBRec.stT1Data) == 0 && strlen(pobTran->srBRec.stT2Data) == 0)
					gfCTLS_Check_VISApayWave = VS_TRUE;

				if (strlen(pobTran->srBRec.stT2Data) == 0)
				{
					/* 因為沒有【Track 2】，因該會給【Tag 57】 */
					memset(pobTran->srBRec.szPAN, 0x00, sizeof(pobTran->srBRec.szPAN));
			        	memset(pobTran->srBRec.szExpDate, 0x00, sizeof(pobTran->srBRec.szExpDate));
			        	memset(pobTran->srBRec.szServiceCode, 0x00, sizeof(pobTran->srBRec.szServiceCode));
			        	memset(pobTran->srBRec.stT2Data, 0x00, sizeof(pobTran->srBRec.stT2Data));
					SVC_HEX_2_DSP((char *)&btTagData[0], &pobTran->srBRec.stT2Data[0], ushTagLen);

					for (i = 0; i < strlen(pobTran->srBRec.stT2Data); i ++)
					{
						/* 【0X44】【D】 */
						if (pobTran->srBRec.stT2Data[i] != 0x44)
							memcpy(&pobTran->srBRec.szPAN[i], &pobTran->srBRec.stT2Data[i], 1);
						else if (szASCII[i] == 0x44)
						{
							i ++;
							memcpy(&pobTran->srBRec.szExpDate[0], &pobTran->srBRec.stT2Data[i], 4);
							i += 4;
							memcpy(&pobTran->srBRec.szServiceCode[0], &pobTran->srBRec.stT2Data[i], 3);
							break;
						}
					}

					for (i = 0; i < strlen(pobTran->srBRec.stT2Data); i ++)
					{
						if (pobTran->srBRec.stT2Data[i] == 'F')
						{
							pobTran->srBRec.stT2Data[i] = 0;
							break;
						}

						if (pobTran->srBRec.stT2Data[i] == 'D')
							pobTran->srBRec.stT2Data[i] = '=';
					}

					LOG_PRINTF(("   TAG57_Card_Number	[%s]", pobTran->srBRec.szPAN));
					LOG_PRINTF(("   TAG57_Card_Expire_Date	[%s]", pobTran->srBRec.szExpDate));
					LOG_PRINTF(("   TAG57_Service_Code	[%s]", pobTran->srBRec.szServiceCode));
				}

                /* For EMV KERNEL UPT 6.0.1，避免兩分鐘Sensitive EMV TAG會被Kernel清除問題 by Wei Hsiu - 2013/5/30 下午 01:34:55 */
        		/* 不跑晶片流程，所以要另外把T2Data塞進去szEMV_UPT_TAG57暫存的buffer，F_55才會組到Tag_57 */
        		memset(pobTran->szEMV_UPT_TAG57, 0x00, sizeof(pobTran->szEMV_UPT_TAG57));
            	//modify by green 160128 修改感應交易 格式錯誤問題 應該為BCD
            	//memcpy(&pobTran->szEMV_UPT_TAG57[0], &pobTran->srBRec.stT2Data[0], ushTagLen);
            	memcpy(&pobTran->szEMV_UPT_TAG57[0], &btTagData[0], ushTagLen);
            	pobTran->inEMV_UPT_TAG57Len = ushTagLen;

        		LOG_PRINTF((" ushTagLen[%d]", ushTagLen));
        		LOG_PRINTF((" inEMV_UPT_TAG57Len[%d]", pobTran->inEMV_UPT_TAG57Len));
				break;
			case 0x5A :
				inCnt += 1;
				pobTran->srEMVRec.us5A_ApplPanLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b5A_ApplPan, 0x00, sizeof(pobTran->srEMVRec.b5A_ApplPan));
				memcpy((char *)&pobTran->srEMVRec.b5A_ApplPan[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us5A_ApplPanLen);
				inCnt += pobTran->srEMVRec.us5A_ApplPanLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b5A_ApplPan[0], szASCII, pobTran->srEMVRec.us5A_ApplPanLen);
				LOG_PRINTF(("  TAG_5A[%02d][%s]", pobTran->srEMVRec.us5A_ApplPanLen, szASCII));
				break;
			case 0x5F20 :
				inCnt += 2;
				ushTagLen = szInputTLVData[inCnt ++];
				memset(pobTran->srBRec.szCHolder, 0x00, sizeof(pobTran->srBRec.szCHolder));
				memset(btTagData, 0x00, sizeof(btTagData));
				memcpy(&pobTran->srBRec.szCHolder[0], &szInputTLVData[inCnt], ushTagLen);
				inCnt += ushTagLen;
				LOG_PRINTF(("  TAG_5F20[%02d][%s]", ushTagLen, pobTran->srBRec.szCHolder));
				break;
			case 0x5F24 :
				inCnt += 2;
				pobTran->srEMVRec.us5F24_ExpireDateLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b5F24_ExpireDate, 0x00, sizeof(pobTran->srEMVRec.b5F24_ExpireDate));
				memcpy((char *)&pobTran->srEMVRec.b5F24_ExpireDate[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us5F24_ExpireDateLen);
				inCnt += pobTran->srEMVRec.us5F24_ExpireDateLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b5F24_ExpireDate[0], szASCII, pobTran->srEMVRec.us5F24_ExpireDateLen);
				LOG_PRINTF(("  TAG_5F24[%02d][%s]", pobTran->srEMVRec.us5F24_ExpireDateLen, szASCII));
				continue;
			case 0x5F2A :
				inCnt += 2;
				pobTran->srEMVRec.us5F2A_TransCurrCodeLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b5F2A_TransCurrCode, 0x00, sizeof(pobTran->srEMVRec.b5F2A_TransCurrCode));
				memcpy((char *)&pobTran->srEMVRec.b5F2A_TransCurrCode[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us5F2A_TransCurrCodeLen);
				inCnt += pobTran->srEMVRec.us5F2A_TransCurrCodeLen;

				if (pobTran->srEMVRec.b5F2A_TransCurrCode[0] == 0x09 && pobTran->srEMVRec.b5F2A_TransCurrCode[1] == 0x01)
				fTag5F2A = VS_TRUE;

				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b5F2A_TransCurrCode[0], szASCII, pobTran->srEMVRec.us5F2A_TransCurrCodeLen);
				LOG_PRINTF(("  TAG_5F2A[%02d][%s]", pobTran->srEMVRec.us5F2A_TransCurrCodeLen, szASCII));
				break;
			case 0x5F2D :/*20151014浩瑋新增*/
                                /* 【需求單 - 104094】銀聯閃付需求 by Wei Hsiu - 2015/8/13 下午 10:12:06 */
				inCnt += 2;
				ushTagLen = szInputTLVData[inCnt ++];
				memset(btTagData, 0x00, sizeof(btTagData));
				memcpy((char *)&btTagData[0], &szInputTLVData[inCnt], ushTagLen);
				inCnt += ushTagLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&btTagData[0], szASCII, ushTagLen);
				LOG_PRINTF(("  TAG_5F2D[%02d][%s]", ushTagLen, szASCII));
				break;
			case 0x5F34 :
				inCnt += 2;
				pobTran->srEMVRec.us5F34_ApplPanSeqnumLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b5F34_ApplPanSeqnum, 0x00, sizeof(pobTran->srEMVRec.b5F34_ApplPanSeqnum));
				memcpy((char *)&pobTran->srEMVRec.b5F34_ApplPanSeqnum[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us5F34_ApplPanSeqnumLen);
				inCnt += pobTran->srEMVRec.us5F34_ApplPanSeqnumLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b5F34_ApplPanSeqnum[0], szASCII, pobTran->srEMVRec.us5F34_ApplPanSeqnumLen);
				LOG_PRINTF(("  TAG_5F34[%02d][%s]", pobTran->srEMVRec.us5F34_ApplPanSeqnumLen, szASCII));
				break;
			case 0x82 :
				inCnt += 1;
				pobTran->srEMVRec.us82_AIPLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b82_AIP, 0x00, sizeof(pobTran->srEMVRec.b82_AIP));
				memcpy((char *)&pobTran->srEMVRec.b82_AIP[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us82_AIPLen);
				inCnt += pobTran->srEMVRec.us82_AIPLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b82_AIP[0], szASCII, pobTran->srEMVRec.us82_AIPLen);
				LOG_PRINTF(("  TAG_82[%02d][%s]", pobTran->srEMVRec.us82_AIPLen, szASCII));
				break;
			case 0x84 :
				inCnt += 1;
				pobTran->srEMVRec.us84_DFNameLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b84_DF_NAME, 0x00, sizeof(pobTran->srEMVRec.b84_DF_NAME));
				memcpy((char *)&pobTran->srEMVRec.b84_DF_NAME[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us84_DFNameLen);
				inCnt += pobTran->srEMVRec.us84_DFNameLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b84_DF_NAME[0], szASCII, pobTran->srEMVRec.us84_DFNameLen);
				LOG_PRINTF(("  TAG_84[%02d][%s]", pobTran->srEMVRec.us84_DFNameLen, szASCII));
				/*為了820,在此修改,比對aid分辨銀聯卡*/
                                memset(szASCII, 0x00, sizeof(szASCII));/*20151014浩瑋新增*/
                                SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b84_DF_NAME[0], szASCII, pobTran->srEMVRec.us84_DFNameLen);
				if(!memcmp(szASCII,"A000000333010101",16) ||
					!memcmp(szASCII,"A000000333010102",16) ||
					!memcmp(szASCII,"A000000333010103",16)
					)
				{
					pobTran->srBRec.fCUPTrans = VS_TRUE;
				}

				break;
			case 0x8A :
				inCnt += 1;
				pobTran->srEMVRec.us8A_AuthRespCodeLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b8A_AuthRespCode, 0x00, sizeof(pobTran->srEMVRec.b8A_AuthRespCode));
				memcpy((char *)&pobTran->srEMVRec.b8A_AuthRespCode[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us8A_AuthRespCodeLen);
				inCnt += pobTran->srEMVRec.us8A_AuthRespCodeLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b8A_AuthRespCode[0], szASCII, pobTran->srEMVRec.us8A_AuthRespCodeLen);
				LOG_PRINTF(("  TAG_8A[%02d][%s]", pobTran->srEMVRec.us8A_AuthRespCodeLen, szASCII));
				break;
			case 0x8E :
				inCnt += 1;
				ushTagLen = szInputTLVData[inCnt ++];
				memset(btTagData, 0x00, sizeof(btTagData));
				memcpy((char *)&btTagData[0], &szInputTLVData[inCnt], ushTagLen);
				inCnt += ushTagLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&btTagData[0], szASCII, ushTagLen);
				LOG_PRINTF(("  TAG_8E[%02d][%s]", ushTagLen, szASCII));
				break;
			case 0x95 :
				inCnt += 1;
				pobTran->srEMVRec.us95_TVRLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b95_TVR, 0x00, sizeof(pobTran->srEMVRec.b95_TVR));
				memcpy((char *)&pobTran->srEMVRec.b95_TVR[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us95_TVRLen);
				inCnt += pobTran->srEMVRec.us95_TVRLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b95_TVR[0], szASCII, pobTran->srEMVRec.us95_TVRLen);
				LOG_PRINTF(("  TAG_95[%02d][%s]", pobTran->srEMVRec.us95_TVRLen, szASCII));
				break;
			case 0x9A :
				inCnt += 1;
				pobTran->srEMVRec.us9A_TranDateLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b9A_TranDate, 0x00, sizeof(pobTran->srEMVRec.b9A_TranDate));
				memcpy((char *)&pobTran->srEMVRec.b9A_TranDate[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us9A_TranDateLen);
				inCnt += pobTran->srEMVRec.us9A_TranDateLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b9A_TranDate[0], szASCII, pobTran->srEMVRec.us9A_TranDateLen);
				LOG_PRINTF(("  TAG_9A[%02d][%s]", pobTran->srEMVRec.us9A_TranDateLen, szASCII));
				break;
			case 0x9B :
				inCnt += 1;
				pobTran->srEMVRec.us9B_TSILen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b9B_TSI, 0x00, sizeof(pobTran->srEMVRec.b9B_TSI));
				memcpy((char *)&pobTran->srEMVRec.b9B_TSI[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us9B_TSILen);
				inCnt += pobTran->srEMVRec.us9B_TSILen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b9B_TSI[0], szASCII, pobTran->srEMVRec.us9B_TSILen);
				LOG_PRINTF(("  TAG_9B[%02d][%s]", pobTran->srEMVRec.us9B_TSILen, szASCII));
				break;
			case 0x9C :
				inCnt += 1;
				pobTran->srEMVRec.us9C_TranTypeLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b9C_TranType, 0x00, sizeof(pobTran->srEMVRec.b9C_TranType));
				memcpy((char *)&pobTran->srEMVRec.b9C_TranType[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us9C_TranTypeLen);
				inCnt += pobTran->srEMVRec.us9C_TranTypeLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b9C_TranType[0], szASCII, pobTran->srEMVRec.us9C_TranTypeLen);
				LOG_PRINTF(("  TAG_9C[%02d][%s]", pobTran->srEMVRec.us9C_TranTypeLen, szASCII));
				break;
			case 0x9F02 :
				inCnt += 2;
				pobTran->srEMVRec.us9F02_AmtAuthNumLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b9F02_AmtAuthNum, 0x00, sizeof(pobTran->srEMVRec.b9F02_AmtAuthNum));
				memcpy((char *)&pobTran->srEMVRec.b9F02_AmtAuthNum[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us9F02_AmtAuthNumLen);
				inCnt += pobTran->srEMVRec.us9F02_AmtAuthNumLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b9F02_AmtAuthNum[0], szASCII, pobTran->srEMVRec.us9F02_AmtAuthNumLen);
				LOG_PRINTF(("  TAG_9F02[%02d][%s]", pobTran->srEMVRec.us9F02_AmtAuthNumLen, szASCII));
				break;
			case 0x9F03 :
				inCnt += 2;
				pobTran->srEMVRec.us9F03_AmtOtherNumLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b9F03_AmtOtherNum, 0x00, sizeof(pobTran->srEMVRec.b9F03_AmtOtherNum));
				memcpy((char *)&pobTran->srEMVRec.b9F03_AmtOtherNum[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us9F03_AmtOtherNumLen);
				inCnt += pobTran->srEMVRec.us9F03_AmtOtherNumLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b9F03_AmtOtherNum[0], szASCII, pobTran->srEMVRec.us9F03_AmtOtherNumLen);
				LOG_PRINTF(("  TAG_9F03[%02d][%s]", pobTran->srEMVRec.us9F03_AmtOtherNumLen, szASCII));
				break;
			case 0x9F06 :/*20151014浩瑋新增*/
                                /* 【需求單 - 104094】銀聯閃付需求 by Wei Hsiu - 2015/8/13 下午 10:12:06 */
				inCnt += 2;
				ushTagLen = szInputTLVData[inCnt ++];
				memset(btTagData, 0x00, sizeof(btTagData));
				memcpy((char *)&btTagData[0], &szInputTLVData[inCnt], ushTagLen);
				inCnt += ushTagLen;

				/* AE晶片化 9F06 add by sunny20181003 */
				pobTran->srEMVRec.us9F06_AppIdentifierLen = ushTagLen;
				memcpy((char *) &pobTran->srEMVRec.b9F06_AppIdentifier[0],(char *)&btTagData[0],ushTagLen);
				//end

				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&btTagData[0], szASCII, ushTagLen);
				LOG_PRINTF(("  TAG_9F06[%02d][%s]", ushTagLen, szASCII));
				break;
			case 0x9F08 :
				inCnt += 2;
				ushTagLen = szInputTLVData[inCnt ++];
				memset(btTagData, 0x00, sizeof(btTagData));
				memcpy((char *)&btTagData[0], &szInputTLVData[inCnt], ushTagLen);
				inCnt += ushTagLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&btTagData[0], szASCII, ushTagLen);
				LOG_PRINTF(("  TAG_9F08[%02d][%s]", ushTagLen, szASCII));
				break;
			case 0x9F09 :
				inCnt += 2;
				ushTagLen = szInputTLVData[inCnt ++];
				memset(btTagData, 0x00, sizeof(btTagData));
				memcpy((char *)&btTagData[0], &szInputTLVData[inCnt], ushTagLen);
				inCnt += ushTagLen;

				/* AE晶片化 9F09 add by sunny20181003 */
				pobTran->srEMVRec.us9F09_TermVerNumLen = ushTagLen;
				memcpy((char *) &pobTran->srEMVRec.b9F09_TermVerNum[0],(char *)&btTagData[0],ushTagLen);
				//end

				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&btTagData[0], szASCII, ushTagLen);
				LOG_PRINTF(("  TAG_9F09[%02d][%s]", ushTagLen, szASCII));
				break;
			/* --START-- AE晶片化 9F0D 9F0E 9F0F add by sunny20181003 */
			case 0x9F0D :
				inCnt += 2;
				ushTagLen = szInputTLVData[inCnt ++];
				memset(btTagData, 0x00, sizeof(btTagData));
				memcpy((char *)&btTagData[0], &szInputTLVData[inCnt], ushTagLen);
				inCnt += ushTagLen;

				pobTran->srEMVRec.us9F0D_IACDefaultLen = ushTagLen;
				memcpy((char *) &pobTran->srEMVRec.b9F0D_IACDefault[0],(char *)&btTagData[0],ushTagLen);

				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&btTagData[0], szASCII, ushTagLen);
				LOG_PRINTF(("  TAG_9F0D[%02d][%s]", ushTagLen, szASCII));

				break;
			case 0x9F0E:
				inCnt += 2;
				ushTagLen = szInputTLVData[inCnt++];
				memset(btTagData, 0x00, sizeof(btTagData));
				memcpy((char *) &btTagData[0], &szInputTLVData[inCnt], ushTagLen);
				inCnt += ushTagLen;

				pobTran->srEMVRec.us9F0E_IACDenialLen = ushTagLen;
				memcpy((char *) &pobTran->srEMVRec.b9F0E_IACDenial[0],(char *) &btTagData[0], ushTagLen);

				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *) &btTagData[0], szASCII, ushTagLen);
				LOG_PRINTF(("  TAG_9F0E[%02d][%s]", ushTagLen, szASCII));

				break;
			case 0x9F0F:
				inCnt += 2;
				ushTagLen = szInputTLVData[inCnt++];
				memset(btTagData, 0x00, sizeof(btTagData));
				memcpy((char *) &btTagData[0], &szInputTLVData[inCnt], ushTagLen);
				inCnt += ushTagLen;

				pobTran->srEMVRec.us9F0F_IACOnlineLen = ushTagLen;
				memcpy((char *) &pobTran->srEMVRec.b9F0F_IACOnline[0],(char *) &btTagData[0], ushTagLen);

				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *) &btTagData[0], szASCII, ushTagLen);
				LOG_PRINTF(("  TAG_9F0F[%02d][%s]", ushTagLen, szASCII));

				break;
			/* ---END--- AE晶片化 9F0D 9F0E 9F0F add by sunny20181003 */
			case 0x9F10 :
				inCnt += 2;
				pobTran->srEMVRec.us9F10_IssuerAppDataLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b9F10_IssuerAppData, 0x00, sizeof(pobTran->srEMVRec.b9F10_IssuerAppData));
				memcpy((char *)&pobTran->srEMVRec.b9F10_IssuerAppData[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us9F10_IssuerAppDataLen);
				inCnt += pobTran->srEMVRec.us9F10_IssuerAppDataLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b9F10_IssuerAppData[0], szASCII, pobTran->srEMVRec.us9F10_IssuerAppDataLen);
				LOG_PRINTF(("  TAG_9F10[%02d][%s]", pobTran->srEMVRec.us9F10_IssuerAppDataLen, szASCII));
				break;
			case 0x9F12 :/*20151014浩瑋新增*/
                                /* 【需求單 - 104094】銀聯閃付需求 by Wei Hsiu - 2015/8/13 下午 10:12:06 */
				inCnt += 2;
				ushTagLen = szInputTLVData[inCnt ++];
				memset(btTagData, 0x00, sizeof(btTagData));
				memcpy((char *)&btTagData[0], &szInputTLVData[inCnt], ushTagLen);
				inCnt += ushTagLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&btTagData[0], szASCII, ushTagLen);
				LOG_PRINTF(("  TAG_9F12[%02d][%s]", ushTagLen, szASCII));
				break;
			case 0x9F1A :
				inCnt += 2;
				pobTran->srEMVRec.us9F1A_TermCountryCodeLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b9F1A_TermCountryCode, 0x00, sizeof(pobTran->srEMVRec.b9F1A_TermCountryCode));
				memcpy((char *)&pobTran->srEMVRec.b9F1A_TermCountryCode[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us9F1A_TermCountryCodeLen);
				inCnt += pobTran->srEMVRec.us9F1A_TermCountryCodeLen;
				fTag9F1A = VS_TRUE;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b9F1A_TermCountryCode[0], szASCII, pobTran->srEMVRec.us9F1A_TermCountryCodeLen);
				LOG_PRINTF(("  TAG_9F1A[%02d][%s]", pobTran->srEMVRec.us9F1A_TermCountryCodeLen, szASCII));
				break;
			case 0x9F1E :
				inCnt += 2;
				pobTran->srEMVRec.us9F1E_IFDNumLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b9F1E_IFDNum, 0x00, sizeof(pobTran->srEMVRec.b9F1E_IFDNum));
				memcpy((char *)&pobTran->srEMVRec.b9F1E_IFDNum[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us9F1E_IFDNumLen);
				inCnt += pobTran->srEMVRec.us9F1E_IFDNumLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b9F1E_IFDNum[0], szASCII, pobTran->srEMVRec.us9F1E_IFDNumLen);
				LOG_PRINTF(("  TAG_9F1E[%02d][%s]", pobTran->srEMVRec.us9F1E_IFDNumLen, szASCII));
				break;
			case 0x9F21 :
				inCnt += 2;
				ushTagLen = szInputTLVData[inCnt ++];
				memset(btTagData, 0x00, sizeof(btTagData));
				memcpy((char *)&btTagData[0], &szInputTLVData[inCnt], ushTagLen);
				inCnt += ushTagLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&btTagData[0], szASCII, ushTagLen);
				LOG_PRINTF(("  TAG_9F21[%02d][%s]", ushTagLen, szASCII));
				break;
			case 0x9F26 :
				inCnt += 2;
				pobTran->srEMVRec.us9F26_ApplCryptogramLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b9F26_ApplCryptogram, 0x00, sizeof(pobTran->srEMVRec.b9F26_ApplCryptogram));
				memcpy((char *)&pobTran->srEMVRec.b9F26_ApplCryptogram[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us9F26_ApplCryptogramLen);
				inCnt += pobTran->srEMVRec.us9F26_ApplCryptogramLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b9F26_ApplCryptogram[0], szASCII, pobTran->srEMVRec.us9F26_ApplCryptogramLen);
				LOG_PRINTF(("  TAG_9F26[%02d][%s]", pobTran->srEMVRec.us9F26_ApplCryptogramLen, szASCII));
				break;
			case 0x9F27 :
				inCnt += 2;
				pobTran->srEMVRec.us9F27_CIDLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b9F27_CID, 0x00, sizeof(pobTran->srEMVRec.b9F27_CID));
				memcpy((char *)&pobTran->srEMVRec.b9F27_CID[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us9F27_CIDLen);
				inCnt += pobTran->srEMVRec.us9F27_CIDLen;

				/* VIVO不會回傳SchemeID, 先忽略 */
				//			if (szSchemeID[0] == SCHEME_ID_21_PAYPASS_MCHIP || szSchemeID[0] == SCHEME_ID_20_PAYPASS_MAG_STRIPE)
				//		        {
				//				if ((pobTran->srEMVRec.b9F27_CID[0] & 0xC0) == 0x40)
				//				{
				//					strcpy(pobTran->srBRec.szAuthCode, TC_GEN1AC_APPROVE);
				//			        }
				//			}

				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b9F27_CID[0], szASCII, pobTran->srEMVRec.us9F27_CIDLen);
				LOG_PRINTF(("  TAG_9F27[%02d][%s]", pobTran->srEMVRec.us9F27_CIDLen, szASCII));
				break;
			case 0x9F33 :
				inCnt += 2;
				pobTran->srEMVRec.us9F33_TermCapabilitiesLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b9F33_TermCapabilities, 0x00, sizeof(pobTran->srEMVRec.b9F33_TermCapabilities));
				memcpy((char *)&pobTran->srEMVRec.b9F33_TermCapabilities[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us9F33_TermCapabilitiesLen);
				inCnt += pobTran->srEMVRec.us9F33_TermCapabilitiesLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b9F33_TermCapabilities[0], szASCII, pobTran->srEMVRec.us9F33_TermCapabilitiesLen);
				LOG_PRINTF(("  TAG_9F33[%02d][%s]", pobTran->srEMVRec.us9F33_TermCapabilitiesLen, szASCII));
				break;
			case 0x9F34 :
				inCnt += 2;
				pobTran->srEMVRec.us9F34_CVMLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b9F34_CVM, 0x00, sizeof(pobTran->srEMVRec.b9F34_CVM));
				memcpy((char *)&pobTran->srEMVRec.b9F34_CVM[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us9F34_CVMLen);
				inCnt += pobTran->srEMVRec.us9F34_CVMLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b9F34_CVM[0], szASCII, pobTran->srEMVRec.us9F34_CVMLen);
				LOG_PRINTF(("  TAG_9F34[%02d][%s]", pobTran->srEMVRec.us9F34_CVMLen, szASCII));
				break;
			case 0x9F35 :
				inCnt += 2;
				pobTran->srEMVRec.us9F35_TermTypeLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b9F35_TermType, 0x00, sizeof(pobTran->srEMVRec.b9F35_TermType));
				memcpy((char *)&pobTran->srEMVRec.b9F35_TermType[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us9F35_TermTypeLen);
				inCnt += pobTran->srEMVRec.us9F35_TermTypeLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b9F35_TermType[0], szASCII, pobTran->srEMVRec.us9F35_TermTypeLen);
				LOG_PRINTF(("  TAG_9F35[%02d][%s]", pobTran->srEMVRec.us9F35_TermTypeLen, szASCII));
				break;
			case 0x9F36 :
				inCnt += 2;
				pobTran->srEMVRec.us9F36_ATCLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b9F36_ATC, 0x00, sizeof(pobTran->srEMVRec.b9F36_ATC));
				memcpy((char *)&pobTran->srEMVRec.b9F36_ATC[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us9F36_ATCLen);
				inCnt += pobTran->srEMVRec.us9F36_ATCLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b9F36_ATC[0], szASCII, pobTran->srEMVRec.us9F36_ATCLen);
				LOG_PRINTF(("  TAG_9F36[%02d][%s]", pobTran->srEMVRec.us9F36_ATCLen, szASCII));
				break;
			case 0x9F37 :
				inCnt += 2;
				pobTran->srEMVRec.us9F37_UnpredictNumLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b9F37_UnpredictNum, 0x00, sizeof(pobTran->srEMVRec.b9F37_UnpredictNum));
				memcpy((char *)&pobTran->srEMVRec.b9F37_UnpredictNum[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us9F37_UnpredictNumLen);
				inCnt += pobTran->srEMVRec.us9F37_UnpredictNumLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b9F37_UnpredictNum[0], szASCII, pobTran->srEMVRec.us9F37_UnpredictNumLen);
				LOG_PRINTF(("  TAG_9F37[%02d][%s]", pobTran->srEMVRec.us9F37_UnpredictNumLen, szASCII));
				break;
			case 0x9F41 :
				inCnt += 2;
				pobTran->srEMVRec.us9F41_TransSeqCounterLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b9F41_TransSeqCounter, 0x00, sizeof(pobTran->srEMVRec.b9F41_TransSeqCounter));
				memcpy((char *)&pobTran->srEMVRec.b9F41_TransSeqCounter[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us9F41_TransSeqCounterLen);
				inCnt += pobTran->srEMVRec.us9F41_TransSeqCounterLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b9F41_TransSeqCounter[0], szASCII, pobTran->srEMVRec.us9F41_TransSeqCounterLen);
				LOG_PRINTF(("  TAG_9F41[%02d][%s]", pobTran->srEMVRec.us9F41_TransSeqCounterLen, szASCII));
				break;
			case 0x9F45 :
				inCnt += 2;
				ushTagLen = szInputTLVData[inCnt ++];
				memset(btTagData, 0x00, sizeof(btTagData));
				memcpy((char *)&btTagData[0], &szInputTLVData[inCnt], ushTagLen);
				inCnt += ushTagLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&btTagData[0], szASCII, ushTagLen);
				LOG_PRINTF(("  TAG_9F45[%02d][%s]", ushTagLen, szASCII));
				break;
			case 0x9F51 :/*20151014浩瑋新增*/
                                /* 【需求單 - 104094】銀聯閃付需求 by Wei Hsiu - 2015/8/13 下午 10:12:06 */
				inCnt += 2;
				ushTagLen = szInputTLVData[inCnt ++];
				memset(btTagData, 0x00, sizeof(btTagData));
				memcpy((char *)&btTagData[0], &szInputTLVData[inCnt], ushTagLen);
				inCnt += ushTagLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&btTagData[0], szASCII, ushTagLen);
				LOG_PRINTF(("  TAG_9F51[%02d][%s]", ushTagLen, szASCII));
				break;
			case 0x9F53 :
				inCnt += 2;
				ushTagLen = szInputTLVData[inCnt ++];
				memset(btTagData, 0x00, sizeof(btTagData));
				memcpy((char *)&btTagData[0], &szInputTLVData[inCnt], ushTagLen);
				inCnt += ushTagLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&btTagData[0], szASCII, ushTagLen);
				LOG_PRINTF(("  TAG_9F53[%02d][%s]", ushTagLen, szASCII));
				break;
			case 0x9F5D :/*20151014浩瑋新增*/
                                /* 【需求單 - 104094】銀聯閃付需求 by Wei Hsiu - 2015/8/13 下午 10:12:06 */
				inCnt += 2;
				ushTagLen = szInputTLVData[inCnt ++];
				memset(btTagData, 0x00, sizeof(btTagData));
				memcpy((char *)&btTagData[0], &szInputTLVData[inCnt], ushTagLen);
				inCnt += ushTagLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&btTagData[0], szASCII, ushTagLen);
				LOG_PRINTF(("  TAG_9F5D[%02d][%s]", ushTagLen, szASCII));
				break;
			case 0x9F66 :
				inCnt += 2;
				ushTagLen = szInputTLVData[inCnt ++];
				pobTran->srEMVRec.us9F66_QualifiersLen = ushTagLen;/*新增*//*20151014浩瑋新增*/
				memset(btTagData, 0x00, sizeof(btTagData));
				memcpy((char *)&btTagData[0], &szInputTLVData[inCnt], ushTagLen);
                                /*20151014浩瑋新增*/
				memset(pobTran->srEMVRec.b9F66_Qualifiers, 0x00, sizeof(pobTran->srEMVRec.b9F66_Qualifiers));/*新增*/
				memcpy((char *)&pobTran->srEMVRec.b9F66_Qualifiers[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us9F66_QualifiersLen);/*新增*/
				inCnt += ushTagLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&btTagData[0], szASCII, ushTagLen);
				LOG_PRINTF(("  TAG_9F66[%02d][%s]", ushTagLen, szASCII));
				break;
			case 0x9F6C :/*20151014浩瑋新增*/
                                /* 【需求單 - 104094】銀聯閃付需求 by Wei Hsiu - 2015/8/13 下午 10:12:06 */
				inCnt += 2;
				pobTran->srEMVRec.us9F6C_Card_Transaction_QualifiersLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b9F6C_Card_Transaction_Qualifiers, 0x00, sizeof(pobTran->srEMVRec.b9F6C_Card_Transaction_Qualifiers));
				memcpy((char *)&pobTran->srEMVRec.b9F6C_Card_Transaction_Qualifiers[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us9F6C_Card_Transaction_QualifiersLen);
				inCnt += pobTran->srEMVRec.us9F6C_Card_Transaction_QualifiersLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b9F6C_Card_Transaction_Qualifiers[0], szASCII, pobTran->srEMVRec.us9F6C_Card_Transaction_QualifiersLen);
				LOG_PRINTF(("  TAG_9F6C[%02d][%s]", pobTran->srEMVRec.us9F6C_Card_Transaction_QualifiersLen, szASCII));
				break;
			case 0x9F6D :
				inCnt += 2;
				ushTagLen = szInputTLVData[inCnt ++];
				memset(btTagData, 0x00, sizeof(btTagData));
				memcpy((char *)&btTagData[0], &szInputTLVData[inCnt], ushTagLen);
				inCnt += ushTagLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&btTagData[0], szASCII, ushTagLen);

				/* 0x9F6D MagStripe AVN: --> 00 01 */
				if (!memcmp(&szASCII[0], "0001", 4))
				{
					memset(pobTran->srBRec.szWAVESchemeID, 0x00, sizeof(pobTran->srBRec.szWAVESchemeID));
					pobTran->srBRec.szWAVESchemeID[0] = SCHEME_ID_20_PAYPASS_MAG_STRIPE;
					pobTran->srBRec.szWAVESchemeID[1] = 0x00;
				}

				LOG_PRINTF(("  TAG_9F6D[%02d][%s]", ushTagLen, szASCII));
				break;
			/* Enhance Contactless Reader Capabilities (START)*/
			/* EMV Contactless Bool C-4 Kernel 4 Spec V2.5 */
			//add by green 170331 新增9f6e
			case 0x9F6E :
				inCnt += 2;
				pobTran->srEMVRec.us9F6E_From_Factor_IndicatorLen = szInputTLVData[inCnt ++];
				memset(pobTran->srEMVRec.b9F6E_From_Factor_Indicator, 0x00, sizeof(pobTran->srEMVRec.b9F6E_From_Factor_Indicator));
				memcpy((char *)&pobTran->srEMVRec.b9F6E_From_Factor_Indicator[0], &szInputTLVData[inCnt], pobTran->srEMVRec.us9F6E_From_Factor_IndicatorLen);
				inCnt += pobTran->srEMVRec.us9F6E_From_Factor_IndicatorLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&pobTran->srEMVRec.b9F6E_From_Factor_Indicator[0], szASCII, pobTran->srEMVRec.us9F6E_From_Factor_IndicatorLen);
				LOG_PRINTF(("  TAG_9F6E[%02d][%s]", pobTran->srEMVRec.us9F6E_From_Factor_IndicatorLen, szASCII));
				break;
			/* Enhance Contactless Reader Capabilities (END)*/
			case 0x9F74 :
				inCnt += 2;
				ushTagLen = szInputTLVData[inCnt ++];
				memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
				memcpy((char *)&pobTran->srBRec.szAuthCode[0], &szInputTLVData[inCnt], ushTagLen);
				inCnt += ushTagLen;
				/* Print debug */
				LOG_PRINTF(("  TAG_9F74[%02d][%s]", ushTagLen, pobTran->srBRec.szAuthCode));
				break;
			case 0xDF55 ://add by green 161013 for newJspeedy start
				/* Transaction Mode (DF55) */
				/* The transaction mode describes in which mode (EMV, Legacy, or Magstripe) the transaction was performed.  */
				inCnt += 2;
				ushTagLen = szInputTLVData[inCnt ++];

				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy((char *)&szTemplate[0], &szInputTLVData[inCnt], ushTagLen);

				inCnt += ushTagLen;
				/* Print debug */
				memset(szASCII, 0x00, sizeof(szASCII));
				SVC_HEX_2_DSP((char *)&szTemplate[0], szASCII, ushTagLen);
				LOG_PRINTF(("  TAG_DF55[%02d][%s]", ushTagLen, szASCII));

				/* New Jspeedy 1.3 add by LingHsiung 2016-04-18 下午 05:29:48 */
				/* VX820是使用DF55來區分New Jspeedy卡片種類 沿用原判斷式的DF69 NewJspeedyMode */
				if (!memcmp(&szASCII[0], "0200" , 2))
				{
					memset(pobTran->srEMVRec.bDF69_NewJspeedyMode, 0x00, sizeof(pobTran->srEMVRec.bDF69_NewJspeedyMode));
					pobTran->srEMVRec.bDF69_NewJspeedyMode[0] = 0x02;

				}

//				if (fDebug == VS_TRUE)
//				{
//				        memset(szTemplate, 0x00, sizeof(szTemplate));
//					sprintf(szTemplate, "  Tag DF55 (%02d) [%s]", ushTagLen, szASCII);
//					vdVERIX_PRINT_ChineseFont(szTemplate, PRT_NORMAL, PRT_COLUMN_42);
//				}

				break;//add by green 161013 for newJspeedy end
			default :
				/* 列印沒有定義的【Tag】 */
				if (inTLVName == 1)
				{
					inCnt ++;
					inOtherTLVLen = szInputTLVData[inCnt ++];
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], &szInputTLVData[inCnt], inOtherTLVLen);
					inCnt += inOtherTLVLen;
					/* Print debug */
					memset(szASCII, 0x00, sizeof(szASCII));
					SVC_HEX_2_DSP((char *)&szTemplate[0], szASCII, inOtherTLVLen);
					LOG_PRINTF(("           OTHER_TAG_[%02X][%03d][%s]", szInputTLVData[inCnt - 2 - inOtherTLVLen], inOtherTLVLen, szASCII));
				}
				else
				{
					inCnt += 2;
					inOtherTLVLen = szInputTLVData[inCnt ++];
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], &szInputTLVData[inCnt], inOtherTLVLen);
					inCnt += inOtherTLVLen;
					/* Print debug */
					memset(szASCII, 0x00, sizeof(szASCII));
					SVC_HEX_2_DSP((char *)&szTemplate[0], szASCII, inOtherTLVLen);
					LOG_PRINTF(("           OTHER_TAG_[%02X %02X][%03d][%s]", szInputTLVData[inCnt - 3 - inOtherTLVLen], szInputTLVData[inCnt - 2 - inOtherTLVLen], inOtherTLVLen, szASCII));
				}

				break;
		}
	} /* End for () .... */

	if (fTag5F2A == VS_TRUE && fTag9F1A == VS_TRUE)
	{
		/* 表示是以本國貨幣交易 */
		memset(pobTran->srEMVRec.b9F1A_TermCountryCode, 0x00, sizeof(pobTran->srEMVRec.b9F1A_TermCountryCode));
		pobTran->srEMVRec.b9F1A_TermCountryCode[0] = 0x01;
		pobTran->srEMVRec.b9F1A_TermCountryCode[1] = 0x58;
	}

	LOG_PRINTF(("inCTLS_VIVO_ProcessEMV_TLV()_END"));
	return (VS_SUCCESS);
}

#endif


/*
App Name        : inCTLS_VIVO_PayPassDiscretionaryData
App Builder     : Tusin
App Date&Time   : 2014/8/14 上午 11:23:49
App Function    : 針對PayPass資料特別解析
*/
//modify by green 170331 for 9f6e (820 VIVO)
int inCTLS_VIVO_PayPassDiscretionaryData(TRANSACTION_OBJECT *pobTran, unsigned char *szPayPassData, int inPayPassLength)
{
	int	inCnt = 0, inLEN, inTempCnt;
	int     inCntFindData = VS_FALSE,inUnpackLen = 0;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	inDISP_LogPrintfWithFlag("  Input Len[%d]",inPayPassLength);
	/* 0xFF8106 Discretionary Data
	   Discretionary Data description for PayPass transaction */
	inCnt += 3;
	/* 計算長度 */
	inLEN = szPayPassData[inCnt];
	inDISP_LogPrintfWithFlag("  PayPassData Len[%d]",inLEN);
	inCnt ++;
	/* 設定要解的位置 */
	inTempCnt = inCnt;
	//inCTLS_VIVO_ProcessEMV_TLV(pobTran, (unsigned char*)&szPayPassData[inCnt], inLEN);
	/*
		ERROR INDICATOR: 0000000000FF
		L1:0x00- OK.
		L2:0x00- OK.
		L3:0x00- OK.
		SW1SW2:0000
		Message on Error: 0xFF- NOT APPLICABLE.
	*/

	inDISP_LogPrintfWithFlag("  Real Cnt PayPassData Start[%02x %02x %02x]",szPayPassData[inCnt],szPayPassData[inCnt+1],szPayPassData[inCnt+2]);
	inDISP_LogPrintfWithFlag("  Temp Cnt PayPassData Start[%02x %02x %02x]",szPayPassData[inTempCnt],szPayPassData[inTempCnt+1],szPayPassData[inTempCnt+2]);

	/* 利用新的位置來擷取資料,不會影響擷取資料的原始長度 */
	do{
		if (szPayPassData[inTempCnt] == 0x9F && szPayPassData[inTempCnt + 1] == 0x42)
		{
			inTempCnt += 2;
			inUnpackLen = szPayPassData[inTempCnt ++]; /* 計算長度 */
			inTempCnt += inUnpackLen;
			inDISP_LogPrintfWithFlag("  9f42[%02d]", inUnpackLen);

		}else if (szPayPassData[inTempCnt] == 0xDF && szPayPassData[inTempCnt + 1] == 0x81 && szPayPassData[inTempCnt + 2] == 0x15)
		{
			inTempCnt += 3;
			inUnpackLen = szPayPassData[inTempCnt ++]; /* 計算長度 */
			inTempCnt += inUnpackLen;
			inDISP_LogPrintfWithFlag("  DF8115[%02d]", inUnpackLen);

		}else if (szPayPassData[inTempCnt] == 0x9F && szPayPassData[inTempCnt + 1] == 0x6E)
		{
			inDISP_LogPrintfWithFlag("  9F6E [%02x,%02x,%02x]", szPayPassData[inTempCnt],szPayPassData[inTempCnt+1],szPayPassData[inTempCnt+2]);
			inUnpackLen = szPayPassData[inTempCnt + 2];
			/* 傳進去的值需加tag的長度 */
			inMultiFunc_AnalysisEmvData(pobTran, (unsigned char*)&szPayPassData[inTempCnt], (inUnpackLen + 2) );
			/*Tag Name*/
			inTempCnt += 2;
			/*Tag Lenth + Tag Total Lenth*/
			inTempCnt += (inUnpackLen+1);
		}else
		{
			inCntFindData = VS_TRUE;
			inDISP_LogPrintfWithFlag(" No Match Data [%d]", inCntFindData);
		}
	}while(inCntFindData == VS_FALSE);

	/* 加入tag回的長度 */
	inCnt += inLEN;

	if (szPayPassData[inCnt] == 0xDF && szPayPassData[inCnt + 1] == 0x81 && szPayPassData[inCnt + 2] == 0x16)
	{
		inCnt += 3;
		inLEN = szPayPassData[inCnt ++]; /* 計算長度 */
		inCnt += inLEN;
		inDISP_LogPrintfWithFlag("  DF8116[%02d]", inLEN);
	}

	inMultiFunc_AnalysisEmvData(pobTran, (unsigned char*)&szPayPassData[inCnt], (inPayPassLength - inCnt));
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        : inMultiFunc_Host_Unpack_Header
Date&Time   : 2019/07/29
Describe        : 分析前30Bytes
 */
int inMultiFunc_Host_Unpack_Header(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, MULTI_TABLE *stMultiOb)
{
	int	inTotalSize = 0;
	char	szTemplate[6 + 1];
	char	szDebugMsg[100 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* Bank (4 Bytes) */
	memset(stMultiOb->stMulti_TransData.szTranHost, 0x00, sizeof (stMultiOb->stMulti_TransData.szTranHost));
	memcpy(stMultiOb->stMulti_TransData.szTranHost, &szDataBuffer[inTotalSize], 4);
	inTotalSize += 4;

	/* EDC SN (6 Bytes) */
	memset(stMultiOb->stMulti_TransData.szTermSN, 0x00, sizeof (stMultiOb->stMulti_TransData.szTermSN));
	memcpy(stMultiOb->stMulti_TransData.szTermSN, &szDataBuffer[inTotalSize], 6);
	inTotalSize += 6;

	/* Response Code (4 Bytes) */
	memset(stMultiOb->stMulti_TransData.szErrorCode, 0x00, sizeof (stMultiOb->stMulti_TransData.szErrorCode));
	memcpy(stMultiOb->stMulti_TransData.szErrorCode, &szDataBuffer[inTotalSize], 4);
	inTotalSize += 4;

	/* Trans Type(功能別)(2 Bytes) */
	memset(stMultiOb->stMulti_TransData.szTransType, 0x00, sizeof (stMultiOb->stMulti_TransData.szTransType));
	memcpy(stMultiOb->stMulti_TransData.szTransType, &szDataBuffer[inTotalSize], 2);
	inTotalSize += 2;

	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "功能別: %s", stMultiOb->stMulti_TransData.szTransType);
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
	}

	/* Total Packet (2 Bytes) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &szDataBuffer[inTotalSize], 2);

	stMultiOb->stMulti_TransData.inTotalPacketNum = 0;
	stMultiOb->stMulti_TransData.inTotalPacketNum = atoi(szTemplate);
	inTotalSize += 2;

	/* Sub Packet Index (2 Bytes) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &szDataBuffer[inTotalSize], 2);

	stMultiOb->stMulti_TransData.inSubPacketNum = 0;
	stMultiOb->stMulti_TransData.inSubPacketNum = atoi(szTemplate);
	inTotalSize += 2;

	/* Total Packet Size (6 Bytes) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &szDataBuffer[inTotalSize], 6);

	stMultiOb->stMulti_TransData.lnTotalPacketSize = 0;
	stMultiOb->stMulti_TransData.lnTotalPacketSize = atoi(szTemplate);
	inTotalSize += 6;

	/* Sub Packet Size (4 Bytes) - 最後再算，動態 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &szDataBuffer[inTotalSize], 4);

	stMultiOb->stMulti_TransData.inSubPacketSize = 0;
	stMultiOb->stMulti_TransData.inSubPacketSize = atoi(szTemplate);
	inTotalSize += 4;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        : inMultiFunc_Host_DataRecvMulti
Date&Time   : 
Describe        : 使用在多段收送資料時，目前在使用前固定需要先讀取前16個Bytes的值
*/
int inMultiFunc_Host_DataRecvMulti( MULTI_TABLE *MultiOb, char *szComData)
{
	int  	i;
	long	lnExpectedPacket, lnCurrentPacket;
	long	lnCnt = 0, lnTotalPacket, lnTotalPrintFileSize, lnCurrentPrintFileSize, lnDataLen;
	char	szTemplate[128 + 1];
	unsigned char uszSTX[2 + 1];
	char    szReceBuff[_MULTI_HOST_RS232_MAX_SIZES_+1];
	unsigned short	usOneSize;
	unsigned char	uszLRCData = 0x00;
	
	VS_BOOL fSTX = VS_FALSE, fAllocDynamic = VS_FALSE;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inDISP_LogPrintfWithFlag("  COMPORT[%u] Line[%d]", (MultiOb->srSetting.uszComPort +1), __LINE__);
	
	/* 因為只收前16個BYTE 所以需再繼續收資料 */
	if(strlen(szComData) > 16)
		inDISP_LogPrintfWithFlag("  InBuf Len Over Flow[%d] Line[%d]", strlen(szComData), __LINE__);
	
	memcpy((char *)&szReceBuff[lnCnt], &szComData[0], strlen(szComData));
	lnCnt += strlen(szComData);
	
	st_fFileMake = VS_FALSE;

	inDISP_LogPrintfWithFlag("  Length1 [%d]", lnCnt);

	lnCurrentPrintFileSize = -1;
	lnExpectedPacket = 0; /* 預期要收到第幾個封包 */
	lnCurrentPacket = 0; /* 現在收到第幾個封包 */
	lnTotalPacket = 1; /* 最少要有一個 */

	for (i = 0; i < lnTotalPacket; i ++)
	{
		/* 目前此功能以COMPORT 已有讀取前16 BYTE資料為主，
		 * 所以第一次進入迴圈是不應該啟動 STX的判斷 */
		
		if (fSTX == VS_TRUE)
			inDISP_Timer_Start(_TIMER_NEXSYS_2_, 5);
		
		while (fSTX == VS_TRUE)
		{
			while (1)
			{
				/* 檢查 Comport有東西 */
				while (inMultiFunc_Host_Data_Receive_Check(MultiOb->srSetting.uszComPort, usOneSize) != VS_SUCCESS)
				{
					/* Timeout */
					if (inTimerGet(_TIMER_NEXSYS_2_) == 0)
					{
						inDISP_LogPrintfWithFlag("  Muti Multi Host Rec Check Time Out *Error* Line[%d]", __LINE__);
						return (VS_TIMEOUT);
					}
				}

				/* 這是Buffer的大小，若沒收到則該值會轉為0，所以需重置 
				  *這裏只處理STX的資料	
				*/
				usOneSize = 1;
				if (inMultiFunc_Host_Data_Receive(MultiOb->srSetting.uszComPort, uszSTX[0], usOneSize) == VS_SUCCESS)
				{
					/* Timeout */
					if (inTimerGet(_TIMER_NEXSYS_2_) == 0)
					{
						inDISP_LogPrintfWithFlag("  Muti Multi Host Rec Time Out *Error* Line[%d]", __LINE__);
						return (VS_TIMEOUT);
					}

					if (uszSTX[0] == _STX_)
					{
						inDISP_LogPrintfWithFlag("  Muti Recv Multi  STX Line[%d]", __LINE__);
						szReceBuff[lnCnt ++] = _STX_;
						fSTX = VS_FALSE;
						break;
					}
					else
					{
						inDISP_LogPrintfWithFlag("  Val = %02X", uszSTX[0]);
					}
				}
				else
				{
					inDISP_LogPrintfWithFlag("  inMultiFunc Host Data Recv STX *Error* Line[%d]",__LINE__);
					return (VS_ESCAPE);
				}
			}
		}
		
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 10);
		
		/* 讀剩下的資料 */
		while (1)
		{
			/* 檢查 Comport有東西 */
			while (inMultiFunc_Host_Data_Receive_Check(MultiOb->srSetting.uszComPort, usOneSize) != VS_SUCCESS)
			{
				/* Timeout */
				if (inTimerGet(_TIMER_NEXSYS_1_) == 0)
				{
					inDISP_LogPrintfWithFlag("  Muti Multi Host Rec Check Time Out *Error* Line[%d]", __LINE__);
					return (VS_TIMEOUT);
				}
			}

			/* 這是Buffer的大小，若沒收到則該值會轉為0，所以需重置 */
			usOneSize = 1;
			if (inMultiFunc_Host_Data_Receive(MultiOb->srSetting.uszComPort,(unsigned char *)szReceBuff[lnCnt], usOneSize) == VS_SUCCESS)
			{
				
				/* 簽名檔有可能有ETX，必須加判斷長度
				 * 因為有可能是前面的DATA有資料，所以要使用 lnCnt 計算
				 */
				if (szReceBuff[lnCnt - 1] == _ETX_)
				{
					if (lnCnt == _MULTI_RECV_SIZE_ - 1 || lnCnt == _MULTI_SUB_SIZE_SMALL_ + 33 - 1)
					{
						lnCnt ++;
						break;
					}
				}
				
				lnCnt ++;
			}
			else
			{
				if (inTimerGet(_TIMER_NEXSYS_1_) == 0)
				{
					inMultiFunc_Host_DataSendMultiError(MultiOb, "0003");
					inDISP_LogPrintfWithFlag("  EICHECK_TIMEOUT_ERR2");
					return (_ECR_RESPONSE_CODE_TIMEOUT_);
				}
			}
		}
		
		/* 算LRC */
		for (i = 1; i < (lnCnt-1); i++)
		{
			uszLRCData ^= szReceBuff[i];
		}

		/* [DEBUG] 回覆收銀機資料 */
		inDISP_LogPrintfWithFlag("  Length2   [%d]", lnCnt);
		inDISP_LogPrintfWithFlag("  EDC LRC   [%02X]", uszLRCData);
		inDISP_LogPrintfWithFlag("  Packet LRC[%02X]", szReceBuff[lnCnt - 1]);
		
		/* 驗證LRC */
		if (uszLRCData == (unsigned char)szReceBuff[lnCnt - 1])
		{
			inDISP_LogPrintfWithFlag("  LRC OK Line[%d]",__LINE__);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &szReceBuff[11], 4);

			if (!memcmp(szTemplate, "0000", 4))
			{
				inMultiFunc_Host_DataSendMultiError(MultiOb, "0000");
			}
			else
				return (VS_ERROR);
			
		}
		else
		{
			inDISP_LogPrintfWithFlag("  LRC Not Match Line[%d]",__LINE__);
			inMultiFunc_Host_DataSendMultiError(MultiOb, "0001");
			return (VS_ERROR);
		}
	
		

	
		/* [DEBUG] 列印收銀機資料 */
		/* 1. 功能 */
		inDISP_LogPrintfWithFlag("  功能[%2.2s]",&szReceBuff[15]);
		/* 2. total packet */
		inDISP_LogPrintfWithFlag("  Total Pack[%2.2s]",&szReceBuff[17]);
		/* 3. sub packet */
		inDISP_LogPrintfWithFlag("  Sub Pack[%2.2s]",&szReceBuff[19]);
		/* 4. total size */
		inDISP_LogPrintfWithFlag("  Total Szie[%6.6s]",&szReceBuff[21]);
		/* 5. sub size */
		inDISP_LogPrintfWithFlag("  Sub Szie[%4.4s]",&szReceBuff[27]);
		/* 6. response code */
		inDISP_LogPrintfWithFlag("  Response Code [%4.4s]",&szReceBuff[11]);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &szReceBuff[11], 4);

		/* 用第一個封包決定有沒有要組圖檔 */
		if (!memcmp(szTemplate, "0000", 4) && fAllocDynamic == VS_FALSE)
			st_fFileMake = VS_TRUE;

		/* 檢核收銀機資料 */

		/* 封包數檢核
			lnExpectedPacket = 0;	預期要收到第幾個封包
			lnCurrentPacket = 0;	現在收到第幾個封包
			lnTotalPacket = 1;	最少要有一個
		*/
		if (lnTotalPacket >= 2)
		{
			if (lnExpectedPacket == 0L && lnCurrentPacket == 0L)
			{
				/* Current Packet No. */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szReceBuff[19], 2);
				lnCurrentPacket = atol(szTemplate);
				lnExpectedPacket = lnCurrentPacket;				
				lnExpectedPacket ++; /* 預期要收到第幾個封包 */
				inDISP_LogPrintfWithFlag("  Multi Host Rec Expect Pack[%ld] line[%d]", lnExpectedPacket, __LINE__ );
			}
			else
			{
				/* Current Packet No. */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szReceBuff[19], 2);
				lnCurrentPacket = atol(szTemplate);
				
				if (lnCurrentPacket != lnExpectedPacket)
				{
					inDISP_LogPrintfWithFlag("  Multi Host Rec Pack Cnt Not Mtch *Error* Curr[%ld] Expec[%ld] line[%d]",lnCurrentPacket, lnExpectedPacket, __LINE__ );
					inMultiFunc_Host_DataSendMultiError(MultiOb, "0010");
					return (VS_ERROR);
				}
				else
				{
					lnExpectedPacket = lnCurrentPacket;
					lnExpectedPacket ++;
				}
			}
		}

		/* 分析收銀機資料 */
		/* 計算總封包 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &szReceBuff[17], 2);
		lnTotalPacket = atol(szTemplate);

		inDISP_LogPrintfWithFlag("  Multi Host Rec TotalPack[%ld] line[%d]",lnTotalPacket, __LINE__ );
		
		/* 計算列印大小 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &szReceBuff[21], 6);
		lnTotalPrintFileSize = atol(szTemplate);

		inDISP_LogPrintfWithFlag("  Multi Host Rec PrintFileSize[%ld] line[%d]",lnTotalPrintFileSize, __LINE__ );
		
		/* 只有在第一次產生動態記憶體, 會在外部呼叫時釋放 */
		if (fAllocDynamic == VS_FALSE)
		{
			st_pszAllocDynamic = malloc(lnTotalPrintFileSize + 64 +1 );
			memset(st_pszAllocDynamic, 0x00, (lnTotalPrintFileSize + 64 +1));
			fAllocDynamic = VS_TRUE;
			lnCurrentPrintFileSize = 0;
		}

		/* 存列印的資料 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &szReceBuff[27], 4);
		lnDataLen = atol(szTemplate); /* 計算目前收到的資料的長度 */

		inDISP_LogPrintfWithFlag("  Multi Host Rec DataLen[%ld] line[%d]",lnDataLen, __LINE__ );
		
		
		/* 將資料存在動態記憶體 */
		if(lnDataLen >  (lnTotalPrintFileSize + 64 +1))
		{
			inDISP_DispLogAndWriteFlie(" Multi Host Rec Copy Len *Error*  DataLen[%ld] TlPrintLen[%ld] Line[%d]",lnDataLen, lnTotalPrintFileSize, __LINE__);
			memcpy(&st_pszAllocDynamic[lnCurrentPrintFileSize], &szReceBuff[31], lnDataLen);
		}else{
			memcpy(&st_pszAllocDynamic[lnCurrentPrintFileSize], &szReceBuff[31], lnDataLen);
		}
			
		lnCurrentPrintFileSize += lnDataLen;

		if (lnTotalPacket > 1)
		{
			memset(szReceBuff, 0x00, _MULTI_MAX_SIZES_); /* 將 Allocate 參數初始化 */
			lnCnt = 0; /* 要歸〈0〉 */
			fSTX = VS_TRUE;
			inDISP_Wait(300);
		}
	} /* END for () ..... */

	return (lnCurrentPrintFileSize);
}



/*
Function        : inMultiFunc_Host_DataRecv
Date&Time   : 2019/07/29
Describe        : 接收收銀機傳來的資料
*/
int inMultiFunc_Host_DataRecv(unsigned char inHandle, int inRespTimeOut, char *szGetMultiData, unsigned char uszSendAck, MULTI_TABLE* stMultiFuncOb)
{
	int inRetVal;
//	unsigned short usRetVal;
	unsigned long ulBMPHandle;
	int		inSize, i = 0;
	int		inExpectedSize = 0;
	char		szDebugMsg[100 + 1];
	char		szTemplate[30 + 1];
	unsigned char	uszSTX[2 + 1];
	unsigned char	uszRecBuf[_MULTI_MAX_SIZES_ + 4];
	unsigned char	uszLRCData = 0x00;
	unsigned short	usOneSize = 1;
	unsigned long uszLRC;
	unsigned short usMaxRecLen = 0;
	VS_BOOL	fMultiPacket;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
	inDISP_LogPrintfWithFlag("  inMultiHost Recv(%u)  Line(%d)", inHandle, __LINE__);

	inSize = 0;
	memset(uszRecBuf, 0x00, sizeof(uszRecBuf));
	memset(uszSTX, 0x00, sizeof(uszSTX));

	/* 設定計時器 */
	if(stMultiFuncOb->srSetting.inTimeout > 0)
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, stMultiFuncOb->srSetting.inTimeout);
	}else{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 5);
	}

	/* 收STX + 前16個Byte */
	while (1)
	{
		/* 檢查 Comport有東西 */
		while (inMultiFunc_Host_Data_Receive_Check(stMultiFuncOb->srSetting.uszComPort, usOneSize) != VS_SUCCESS)
		{
			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == 0)
			{
				inDISP_LogPrintfWithFlag("  inMultiHost Recv Check TimeOut *Error* Line[%d]", __LINE__);
				return (VS_TIMEOUT);
			}
		}

		/* 這是Buffer的大小，若沒收到則該值會轉為0，所以需重置 */
		usOneSize = 1;
		if (inMultiFunc_Host_Data_Receive(stMultiFuncOb->srSetting.uszComPort, uszSTX[0], usOneSize) == VS_SUCCESS)
		{
			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == 0)
			{
				inDISP_LogPrintfWithFlag("  inMultiHost Recv DataTimeOut *Error* Line[%d]", __LINE__);
				return (VS_TIMEOUT);
			}

			if (uszSTX[0] == _STX_)
			{
				inDISP_LogPrintfWithFlag("  inMultiHost Recv Get STX Success");

				uszRecBuf[inSize ++] = _STX_;

				i = 16;

				while (i > 0)
				{
					/* 銀行別 + 回應碼 + 功能別 */
					usOneSize = 1;
					if (inMultiFunc_Host_Data_Receive(stMultiFuncOb->srSetting.uszComPort, uszRecBuf[inSize], usOneSize) == VS_SUCCESS)
					{
						inSize += usOneSize;
						i -= usOneSize;
					}

				}
				break;
			}
			else
			{
				inDISP_LogPrintfWithFlag("  inMultiHost Recv Non STX Data [%02X]", uszSTX[0]);
			}
    		}
    		else
    		{
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Receive STX ERR END -----",__FILE__, __FUNCTION__, __LINE__);
    			return (VS_ESCAPE);
    		}
	}
	
	/* 前16Byte OK*/
	inDISP_LogPrintfWithFlag("  inMultiHost Recv 前16 OK Line[%d]", __LINE__);

	/* 備份設定的 Error Code */
	memcpy(stMultiFuncOb->stMulti_TransData.szErrorCode, &uszRecBuf[11], 4);
	inDISP_LogPrintfWithFlag("  inMultiHost Recv ErrorCode[%s] line[%d]", stMultiFuncOb->stMulti_TransData.szErrorCode, __LINE__);
	/* 看看是否要收SubData*/
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &uszRecBuf[15], 2);
	memcpy(stMultiFuncOb->stMulti_TransData.szTransType, &uszRecBuf[15], 2);
	inDISP_LogPrintfWithFlag("  inMultiHost Recv TransType[%s] line[%d]", stMultiFuncOb->stMulti_TransData.szTransType, __LINE__);
	
	/* +30是header +3表示STX ETX LRC */
	/* 根據功能別，決定要收的SubData長度 */
	if (!memcmp(szTemplate, _MULTI_POLL_, 2) ||
	    !memcmp(szTemplate, _MULTI_SLAVE_REBOOT_, 2)	||
	    !memcmp(szTemplate, _MULTI_NOSIGN_, 2))
	{
		inDISP_LogPrintfWithFlag("  inMultiHost Recv POOl No Sub Data line[%d]",  __LINE__);
		/* 目前沒有接收 */
                        return (VS_ERROR);
	}
	else if (!memcmp(szTemplate, _MULTI_PIN_, 2))
	{
		inExpectedSize = _MULTI_SUB_SIZE_SMALL_+ 30 + 3;
		
	}else if (!memcmp(szTemplate, _MULTI_EXCHANGE_, 2)	||
		 !memcmp(szTemplate, _MULTI_SIGN_CONFIRM_, 2))
	{
		inExpectedSize = _MULTI_SUB_SIZE_NONE_+ 30 + 3;
	}else if (!memcmp(szTemplate, _MULTI_CTLS_, 2))
	{
		inExpectedSize = _MULTI_SUB_SIZE_MAX_ + 30 + 3;
		
	}else if (!memcmp(szTemplate, _MULTI_TMS_CAPK_, 2) ||
		   !memcmp(szTemplate, _MULTI_TMS_MASTER_, 2) ||
		   !memcmp(szTemplate, _MULTI_TMS_VISA_, 2) ||
		   !memcmp(szTemplate, _MULTI_TMS_JCB_, 2) ||
		   !memcmp(szTemplate, _MULTI_TMS_CUP_, 2))
	{
	
		inExpectedSize = _MULTI_SUB_SIZE_SMALL_ + 30 + 3; /* STX + 30 + SubData(100) + ETX + LRC - 為了接回應碼 */
	}else if(!memcmp(szTemplate, _MULTI_SIGNPAD_, 2))
	{
		inExpectedSize = _MULTI_SUB_SIZE_SMALL_ + 30 + 3;
		/* 因為圖檔長度可能會過長，所以要個別處理 */
		fMultiPacket = VS_TRUE; 
	}else 		
	{
		inExpectedSize = _MULTI_SUB_SIZE_MAX_ + 30 + 3;
	}
	
	inDISP_LogPrintfWithFlag("  inMultiHost Recv inExpectedSize[%d] line[%d]", inExpectedSize,  __LINE__);
	
	usMaxRecLen = inExpectedSize;
	
	if (fMultiPacket == VS_FALSE)
	{
		/* 設定計時器 */
		if (inRespTimeOut > 0)
		{
			inDISP_LogPrintfWithFlag(" inMultiHost Recv Set Time[%d] Line[%d]", inRespTimeOut, __LINE__);
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, inRespTimeOut);
		}

		/* 收Sub Data */
		while (1)
		{
			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == 0)
			{
				inDISP_LogPrintfWithFlag(" inMultiHost Recv TimeOut Line[%d]", __LINE__);
				return (VS_TIMEOUT);
			}
			
			//usOneSize = 200;
			/* 設定讀取的值 */
			usOneSize = usMaxRecLen;
			
			if (inMultiFunc_Host_Data_Receive(stMultiFuncOb->srSetting.uszComPort, uszRecBuf[inSize], usOneSize) ==  VS_SUCCESS)
			{
				inSize += usOneSize;
				/* 用最大長度的值計算出下次要讀的剩餘長度 */
				usMaxRecLen -= usOneSize;
			}

			if (uszRecBuf[inSize - 2] == _ETX_)
			{
				break;
			}
			
			/* 讀完該功能別的 Subdata */
//			if (inSize >= inExpectedSize)
//			{
//				if (uszRecBuf[inSize - 2] == _ETX_)
//				{
//					break;
//				}
//			}
		} /* End while () .... */


		/* 算LRC */
		for (i = 1; i < (inSize-1); i++)
		{
			uszLRCData ^= uszRecBuf[i];
		}

		/* 驗證LRC */
		if (uszLRCData == uszRecBuf[inSize - 1])
		{
			uszLRC = VS_TRUE;
			inDISP_LogPrintfWithFlag(" inMultiHost Recv LRC OK Line[%d]", __LINE__);
		}
		else
		{
			uszLRC = VS_FALSE;
			inDISP_LogPrintfWithFlag(" inMultiHost Recv Size[%d] Line[%d]",inSize, __LINE__);
			inDISP_LogPrintfWithFlag(" inMultiHost Recv LRC[%x] RecLrc[%x] Line[%d]",uszLRCData, uszRecBuf[inSize - 1],  __LINE__);
			inDISP_LogPrintfWithFlag(" inMultiHost Recv LRC Not Match Line[%d]", __LINE__);
		}
		
		memcpy(&szGetMultiData[0], &uszRecBuf[1], inSize - 3);
		szGetMultiData[inSize - 3] = 0x00;
		
/* 看520 HOST 接收時沒回ACK這個條件 */
#if 0
		/* 是否Send ACK */
		if (uszSendAck)
		{
			/* SN是否核對正確 */
			if (uszSN)
			{
				if (uszLRC == VS_TRUE)
				{
					inMultiFunc_Host_Send_ACKorNAK(stMultiFuncOb, _ACK_);
				}
				else
				{
					inMultiFunc_Host_Send_ACKorNAK(stMultiFuncOb, _NAK_);

					return (VS_ERROR);
				}
			}
			else
			{
				inMultiFunc_Host_Send_ACKorNAK(stMultiFuncOb, _FS_);

				return (VS_ERROR);
			}
		}
		else
		{

		}
#endif 
	}else
	{

		memset(stMultiFuncOb->stMulti_TransData.szTransType, 0x00, sizeof (stMultiFuncOb->stMulti_TransData.szTransType));
		memcpy(stMultiFuncOb->stMulti_TransData.szTransType, &uszRecBuf[15], 2);

		memset(stMultiFuncOb->stMulti_TransData.szTranHost, 0x00, sizeof (stMultiFuncOb->stMulti_TransData.szTranHost));
		memcpy(stMultiFuncOb->stMulti_TransData.szTranHost, &uszRecBuf[1], 4);

		/* 收所有要使用的資料的資料 */
		inSize = inMultiFunc_Host_DataRecvMulti(stMultiFuncOb,(char *)uszRecBuf);

		/* 寫成圖檔 */
		if (st_fFileMake == VS_TRUE)
		{
			/* 移除原本的檔案 */
			inFILE_Delete((unsigned char *)TempSignBmpFile);
			
			inRetVal = inFILE_Create(&ulBMPHandle, (unsigned char *)TempSignBmpFile);

			if (inRetVal < 0)
			{
				inDISP_LogPrintfWithFlag(" inMultiHost Recv Create SignBmp *Error*  Line[%d]",  __LINE__);
				free(st_pszAllocDynamic); /* 要釋放記憶體 */
				return (VS_ERROR);
			}

			inRetVal = CTOS_FileWrite(ulBMPHandle, (unsigned char *)st_pszAllocDynamic, inSize);
			
			if (inRetVal != inSize)
			{
				inDISP_LogPrintfWithFlag(" inMultiHost Recv Write SignBmp *Error*  RetVal[%04x]  Line[%d]", (unsigned short) inRetVal, __LINE__);
				inFILE_Close(&ulBMPHandle);
				free(st_pszAllocDynamic); /* 要釋放記憶體 */
				return (VS_ERROR);
			}
			
			inFILE_Close(&ulBMPHandle);
		}

		free(st_pszAllocDynamic); /* 要釋放記憶體 */

	}

	return (VS_SUCCESS);
}



/*
Function        : inMultiFunc_HostInital
Date&Time   : 2019/07/29
Describe        : initial COM PORT
 */
int inMultiFunc_HostInital(MULTI_TABLE *stMultiOb)
{
	char	szMultiComPort1[4 + 1];
	unsigned char	uszParity;
	unsigned char	uszDataBits;
	unsigned char	uszStopBits;
	unsigned long	ulBaudRate;
	unsigned short	usRetVal;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	memset(&uszParity, 0x00, sizeof (uszParity));
	memset(&uszDataBits, 0x00, sizeof (uszDataBits));
	memset(&uszStopBits, 0x00, sizeof (uszStopBits));
	memset(&ulBaudRate, 0x00, sizeof (ulBaudRate));

	/* 從EDC.Dat抓出哪一個Comport，這裡先HardCode */
	/* inGetMultiComPort1 */
	memset(szMultiComPort1, 0x00, sizeof (szMultiComPort1));

	inGetMultiComPort1(szMultiComPort1);

	inDISP_LogPrintfWithFlag("  Multi Host Com[%s] Line[%d]",szMultiComPort1, __LINE__);
	
	/* Verifone用handle紀錄，Castle用Port紀錄 */
	if (!memcmp(szMultiComPort1, "COM1", 4))
		stMultiOb->srSetting.uszComPort = d_COM1;
	else if (!memcmp(szMultiComPort1, "COM2", 4))
		stMultiOb->srSetting.uszComPort = d_COM2;
	else if (!memcmp(szMultiComPort1, "COM3", 4))
		stMultiOb->srSetting.uszComPort = d_COM3;
	else if (!memcmp(szMultiComPort1, "COM4", 4))
		stMultiOb->srSetting.uszComPort = d_COM4;
        else if (!memcmp(szMultiComPort1, "USB", 4))  /* 20230328 Miyano add for UPT */
	{
                usRetVal = inMultiFunc_Host_USB_Initial();
                return usRetVal;
	}

	/* BaudRate = 115200 */
	ulBaudRate = 115200;

	/* Parity */
	uszParity = 'N';

	/* Data Bits */
	uszDataBits = 8;

	/* Stop Bits */
	uszStopBits = 1;

	/* 開port */
	usRetVal = inRS232_Open(stMultiOb->srSetting.uszComPort, ulBaudRate, uszParity, uszDataBits, uszStopBits);

	if (usRetVal != VS_SUCCESS)
	{
		inDISP_LogPrintfWithFlag(" Mutil Host Init Opne *Error* Retval [0x%04] Line[%d]", usRetVal, __LINE__);
		inDISP_LogPrintfWithFlag(" COM[%u] BaudRate:%lu %d%c%d", (stMultiOb->srSetting.uszComPort +1), ulBaudRate, uszDataBits, uszParity, uszStopBits);
		stMultiOb->srSetting.uszSettingOK = VS_FAILURE;
		return (VS_ERROR);
	}
	else
	{
		stMultiOb->srSetting.uszSettingOK = VS_TRUE;
		inDISP_LogPrintfWithFlag("  Mutil Host Inital Success ");
		inDISP_LogPrintfWithFlag("COM[%u] BaudRate[%lu] Bit[%d] Parity[%c] %d", (stMultiOb->srSetting.uszComPort +1), ulBaudRate, uszDataBits, uszParity, uszStopBits);
		
		/* 清空接收的buffer */
		inMultiFunc_Host_FlushRxBuffer(stMultiOb->srSetting.uszComPort);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * Function		: inMultiFunc_Host_RecePacket
 * Date&Time	: 2022/11/17 下午 2:03
 * Describe		: 
 * 處理外接設備透過 Rs232 傳送回來的資料
 */
int inMultiFunc_Host_RecePacket(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *stMultiOb)
{
	int     inRetVal;
	char    szTemplate[16];
        char	szCTLSReaderMode[1 + 1] = {0};
        
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);	
        
        /* 20230412 Miyano 如果外接設備是A30，要用JSON */
        memset(szCTLSReaderMode, 0x00, sizeof(szCTLSReaderMode));
	inGetContactlessReaderMode(szCTLSReaderMode);
        {
                if (!memcmp(szCTLSReaderMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)))
                {
                    inRetVal = inMultiFunc_Host_JsonRecePacket(pobTran, stMultiOb);
                    return inRetVal;
                }
        }
        
	memset(stMultiOb->stMulti_TransData.szReceData, 0x00, sizeof (stMultiOb->stMulti_TransData.szReceData));

	/* -----------------------開始接收資料------------------------------------------ */
	/* szReceData  讀出來的資料會少掉 STX ，所以算欄位時需減少一個 BYTE*/
	inRetVal = inMultiFunc_Host_DataRecv(stMultiOb->srSetting.uszComPort,
					  2,
					  stMultiOb->stMulti_TransData.szReceData,
					  VS_FALSE,
					  stMultiOb);

	if (inRetVal != VS_SUCCESS)
	{
		inDISP_LogPrintfWithFlag("第一階段分析不OK A");
		return (inRetVal);
	}
	else
	{
		inDISP_LogPrintfWithFlag("第一階段分析OK");
	}

	if (memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_TMS_CAPK_, 2) &&
	   memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_TMS_MASTER_, 2) &&
	   memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_TMS_VISA_, 2) &&
	   memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_TMS_JCB_, 2) &&
	   memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_TMS_CUP_, 2))
		inDISP_BEEP(2,0);
	
	if (!memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_PIN_, 2))
	 {
		/* 將【COM PORT】清空 */
		inMultiFunc_Host_FlushRxBuffer(stMultiOb->srSetting.uszComPort);

		if (!memcmp(stMultiOb->stMulti_TransData.szErrorCode, "0000", 4))//enter pin
		{
			/* add by green 150825 解PIN 將clear pin 放到 pobTran->szPIN */
			memcpy(&pobTran->szPIN[0], &stMultiOb->stMulti_TransData.szReceData[64], 16);
/* 要補 */			
//			inTCBDecryptPIN(pobTran);
			if ( pobTran->srBRec.inCode == _CUP_SALE_)//CUP 要算pinblock
			{
				/* add by green 150825 將clear pin 給內建pinpad 算block */
/* 要補 */
//				inRetVal = inTCB_VSS_Get3DESPINBlock(pobTran);
			}
			else//金融卡 不算pinblock 直接回存clear pin
				inRetVal = VS_SUCCESS;
		}
		else if (!memcmp(stMultiOb->stMulti_TransData.szErrorCode, "0006", 4))//pin by pass
		{
			/* VS_SUCCESS */
			memcpy(pobTran->szPIN, "                ", _PIN_SIZE_);
			inRetVal = VS_SUCCESS;
		}
		else if (!memcmp(stMultiOb->stMulti_TransData.szErrorCode, "0002", 4))//user abort
		{
			/* VS_ESCAPE */
			inRetVal = VS_ESCAPE;
		}
		else if (!memcmp(stMultiOb->stMulti_TransData.szErrorCode, "0003", 4))
		{
			/* VS_TIMEOUT */
			inRetVal = VS_TIMEOUT;
		}else{
			inDISP_LogPrintfWithFlag("  inMultiHost RecvPacket ErrorCode *Error* Code[%s] line[%d]", stMultiOb->stMulti_TransData.szErrorCode, __LINE__);
			inRetVal = VS_ERROR;
		}
	}
	else if (!memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_EXCHANGE_, 2))
	{
		/* 將【COM PORT】清空 */
		inMultiFunc_Host_FlushRxBuffer(stMultiOb->srSetting.uszComPort);

		/* Get SN */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &stMultiOb->stMulti_TransData.szReceData[4], 6);

		/* 檢核SN */
/* 要補 */		
//		memset(szSN, 0x00, sizeof(szSN));
//		szSN[get_env("#MATCH_SN", szSN, 6)] = 0x00;
//		LOG_PRINTF(("NOW 820 s/n: %s",szTemplate));
//		LOG_PRINTF(("ORIG 820 s/n: %s",szSN));
//		if (memcmp(szTemplate, szSN, 6))
//		{
//			LOG_PRINTF(("MATCH_SN error"));
//			/* 比對失敗要寫參數 */
//			SVC_WAIT(500);
//			inRetVal = inMultiFunc_TMS_UploadData(pobTran, VS_FALSE);
//
//			if (inRetVal == VS_SUCCESS)
//			{
//				put_env("#MATCH_SN", szTemplate, 6);
//				SVC_WAIT(500);
//				SVC_RESTART("");
//			}
//		}

		inRetVal = VS_SUCCESS;
	}
	else if (!memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_TMS_CAPK_, 2) ||
			 !memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_TMS_MASTER_, 2) ||
			 !memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_TMS_VISA_, 2) ||
			 !memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_TMS_CUP_, 2) ||   /*20151014浩瑋新增*/
			 !memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_TMS_JCB_, 2))
	{
		/* 這裡檢核回應碼 */
		if (memcmp(stMultiOb->stMulti_TransData.szErrorCode, "0000", 4))
		{
			inRetVal = VS_ERROR;
		}else
		{
			inRetVal = VS_SUCCESS;
		}
	}
	else if (!memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_SIGN_CONFIRM_, 2))
	{
		/* 將【COM PORT】清空 */
		inMultiFunc_Host_FlushRxBuffer(stMultiOb->srSetting.uszComPort);

		inRetVal = VS_SUCCESS;
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
	return inRetVal;
}

int inMultiFunc_Host_DataSend(MULTI_TABLE * stMultiOb, char *szDataBuffer, int inDataSize)
{
	int	i;
	int	inRetVal;
	int	inRetry = 0;
	int	inSendLen = 0;
	unsigned char	uszSendBuf[_ECR_RS232_BUFF_SIZE_];/* 包含STX、ETX、LRC的電文 */
	unsigned char	uszLRC = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	inDISP_LogPrintfWithFlag("  Multi Host Send Size[%d] Com[%u] Line[%d]", inDataSize, (stMultiOb->srSetting.uszComPort + 1), __LINE__);

	/* Send之前清Buffer，避免收到錯的回應 */
	inMultiFunc_Host_FlushRxBuffer(stMultiOb->srSetting.uszComPort);

	/* 將Buffer初始化 */
	memset(uszSendBuf, 0x00, sizeof(uszSendBuf));

	if (stMultiOb->stMulti_Optional_Setting.uszPadStxEtx == VS_TRUE)
	{
		inDISP_LogPrintfWithFlag("  Multi Host Send PadStxEtx Line[%d]",__LINE__);
		/* 在要傳送Buffer裡放STX */
		uszSendBuf[inSendLen] = _STX_;
		inSendLen ++;
		/* 在要傳送Buffer裡放要傳送的資料 */
		memcpy(&uszSendBuf[inSendLen], szDataBuffer, inDataSize);
		inSendLen += inDataSize;
		/* 在要傳送Buffer裡放ETX */
		uszSendBuf[inSendLen] = _ETX_;
		inSendLen ++;
	}
	else
	{
		inDISP_LogPrintfWithFlag("  Multi Host Send Device Straight Data Line[%d]",__LINE__);
		/* 在要傳送Buffer裡放要傳送的資料 */
		memcpy(&uszSendBuf[inSendLen], szDataBuffer, inDataSize);
		inSendLen += inDataSize;
	}

	/* 運算LRC(STX Not include) */
	for (i = 1; i < (inSendLen); i++)
	{
		uszLRC ^= uszSendBuf[i];
	}

	/* 在要傳送Buffer裡放LRC */
	uszSendBuf[inSendLen] = uszLRC;
	inSendLen ++;

	inDISP_LogPrintfWithFlag("  Multi Host Send LRC [0x%02x] Line[%d]", uszLRC, __LINE__);

	while (1)
	{
		/* 檢查port是否已經準備好要送資料 */
		while (inMultiFunc_Host_Data_Send_Check(stMultiOb->srSetting.uszComPort) != VS_SUCCESS);

		/* 經由port傳送資料 */
		inRetVal = inMultiFunc_Host_Data_Send(stMultiOb->srSetting.uszComPort, uszSendBuf, (unsigned short)inSendLen);

		if (inRetVal != VS_SUCCESS)
		{
			inDISP_LogPrintfWithFlag("  Multi Host Send DataSend *Error* line[%d]",__LINE__);
			return (VS_ERROR);
		}
		else
		{

			/*---------------------------------- 印Debug 開始---------------------------------------------------------------*/
			/* 列印紙本電文和顯示電文訊息 */
			inECR_Print_Send_ISODeBug(szDataBuffer, inDataSize, inSendLen);
			/*---------------------------------- 印Debug 結束---------------------------------------------------------------*/
			/* 傳送Retry */
                        if (inRetry < stMultiOb->stMulti_Optional_Setting.inMaxRetries && 
			    stMultiOb->stMulti_Optional_Setting.uszWaitForAck == VS_TRUE)
			{
				/* 接收ACK OR NAK */
				inRetVal = inMultiFunc_Host_Receive_ACKandNAK(stMultiOb);

				/* 超過一秒沒收到回應 */
				if (inRetVal == VS_TIMEOUT)
				{
					inDISP_LogPrintfWithFlag("  Multi Host Send Not Receive Response Retry Line[%d]",__LINE__);
					inRetry++;
					continue;
				}
				/* 收到NAK */
				else if (inRetVal == _NAK_)
				{
					inDISP_LogPrintfWithFlag("  Multi Host Send Receive NAK  Retry Line[%d]",__LINE__);
					inRetry++;
					continue;
				}
				/* 收到ACK */
				else
				{
					/* 成功 */
					inDISP_LogPrintfWithFlag("  Multi Host Send ECR ACK OK Line[%d]",__LINE__);
					return (VS_SUCCESS);
				}
			}
			/* 超過最大重試次數，仍要完成交易，收銀機提示補登畫面 */
                        /* 20230406 Miyano fix 回傳VS_SUCCESS 改成 VS_TIMEOUT */
			else
			{
				inDISP_LogPrintfWithFlag("  Multi Host Send Exceed max retry times Line[%d]",__LINE__);
				return (VS_TIMEOUT);
			}

		}/* inRS232_Data_Send */

	}/* while(1) */

}

int inMultiFunc_Host_PackResult(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *srMultiHost )
{
	int	inTotalSize = 0, inSubIndex = 0, inSubSize = 0, inLenlocation = 26;
	char 	szTemplate[50], szAmount[12 + 1], szFINATID[8 + 1];
	char    szINSTTemp[32], szMCC[6 + 1];
//	char    szEdcVersion[2];
//	int     inRetVal;
	char	szNATIONALPAY[2 + 1];
//	char 	szNatinalshow[ 1 + 1 ];
//	short    V3ULTransCode;
        char    szFiscTID[8 + 1];
        char    szFuncEnable[1 + 1];
	char 	szTempTransType[2 + 1];
//        char    szVersionID[10];
        char    szIssuerID[16];
        char    szDebugMSG[100];
        RTC_NEXSYS	srRTC;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* Bank (4 Bytes) */
	memcpy(&srMultiHost->stMulti_TransData.szSendData[inTotalSize], &srMultiHost->stMulti_TransData.szTranHost[0], 4);
	inTotalSize += 4;

	/* EDC SN (6 Bytes) */
	memcpy(&srMultiHost->stMulti_TransData.szSendData[inTotalSize], srMultiHost->stMulti_TransData.szTermSN, 6);
	inTotalSize += 6;

	
	memset(szTempTransType, 0x00, sizeof(szTempTransType));
	memcpy(szTempTransType, srMultiHost->stMulti_TransData.szTransType, 2);
	
	/* Response Code (4 Bytes) */	
	if (!memcmp(szTempTransType, _MULTI_EXCHANGE_, 2))/*20151014浩瑋新增*/
	{
//                if (fGetFISCCntlessFuncEnable() == VS_TRUE && fGetCUPCntlessFuncEnable() == VS_TRUE)
	                memcpy(&srMultiHost->stMulti_TransData.szSendData[inTotalSize], "0004", 4);    /* _CTLS_5_ */
//	        else if (fGetFISCCntlessFuncEnable() == VS_TRUE)
//	                memcpy(&srMultiHost->stMulti_TransData.szSendData[inTotalSize], "0002", 4);    /* _CTLS_4_SMARTPAY_ */
//	        else if (fGetCUPCntlessFuncEnable() == VS_TRUE)
//	                memcpy(&srMultiHost->stMulti_TransData.szSendData[inTotalSize], "0003", 4);    /* _CTLS_4_QUICKPASS_ */
//	        else
//	                memcpy(&srMultiHost->stMulti_TransData.szSendData[inTotalSize], "0001", 4);    /* _CTLS_3_ */
	}
	else if (!memcmp(szTempTransType, _MULTI_POLL_, 2))//add by green 160331  820 新增待機畫面判斷
	{
                //add by green 160531 修改(160331  820 新增待機畫面判斷造成的bug)  因為一直polling 820會一直更新畫面造成當機 所以只再功能1及開機更新畫面
		if (pobTran->inRunOperationID == _OPERATION_EDC_BOOTING_ )
//	要補		||     pobTran->inRunOperationID == CHOOSE_DEVICE_CONNECT_OPERATION) //功能1 及 開機
		{
			//add by green 170321 for 京站
//			memset(szEdcVersion, 0x00, sizeof(szEdcVersion));
//			szEdcVersion[get_env("#EDCVERSION", szEdcVersion, 3)] = 0x00;
//			if (szEdcVersion[0] == '2')//京站
//				memcpy(&srMultiHost->stMulti_TransData.szSendData[inTotalSize], "0004", 4);
//			else
				memcpy(&srMultiHost->stMulti_TransData.szSendData[inTotalSize], "0001", 4);
		}
		else
		{
			memcpy(&srMultiHost->stMulti_TransData.szSendData[inTotalSize], "0000", 4);
		}
	}
	else
	{
	        memcpy(&srMultiHost->stMulti_TransData.szSendData[inTotalSize], "0000", 4);
	}
	inTotalSize += 4;

	/* Trans Type (2 Bytes) */
	memcpy(&srMultiHost->stMulti_TransData.szSendData[inTotalSize], &szTempTransType[0], 2);
	inTotalSize += 2;

	/* Total Packet (2 Bytes) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%02d", srMultiHost->stMulti_TransData.inTotalPacketNum);
	memcpy(&srMultiHost->stMulti_TransData.szSendData[inTotalSize], &szTemplate[0], 2);
	inTotalSize += 2;

	/* Sub Packet Index (2 Bytes) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%02d", srMultiHost->stMulti_TransData.inSubPacketNum);
	memcpy(&srMultiHost->stMulti_TransData.szSendData[inTotalSize], &szTemplate[0], 2);
	inTotalSize += 2;

	/* Total Packet Size (6 Bytes) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%06ld", srMultiHost->stMulti_TransData.lnTotalPacketSize);
	memcpy(&srMultiHost->stMulti_TransData.szSendData[inTotalSize], &szTemplate[0], 6);
	inTotalSize += 6;

	/* Sub Packet Size (4 Bytes) - 最後再算，動態 */
	inLenlocation = inTotalSize;
	inTotalSize += 4;

	/* SubData by TranCode (0, 100 or 990) */
	inSubIndex = inTotalSize;
	
	if (!memcmp(szTempTransType, _MULTI_PIN_, 2))
	{
		#ifdef DEMO_EDC
//                        ginNCCC_TMK_Index = 1;
		#endif

		/* PIN Block (16 Bytes) */
		inSubIndex += 16;
		inSubSize += 16;

		/* Amount (12 Bytes) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnTxnAmount);
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], szTemplate, 12);
		inSubIndex += 12;
		inSubSize += 12;

		/* 卡號長度 (2 Bytes) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d", strlen(pobTran->srBRec.szPAN));
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], szTemplate, 2);
		inSubIndex += 2;
		inSubSize += 2;

		/* 卡號 (20 Bytes) */
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], &pobTran->srBRec.szPAN[0], strlen(pobTran->srBRec.szPAN));
		inSubIndex += 20;
		inSubSize += 20;
	}
	else if (!memcmp(szTempTransType, _MULTI_CTLS_, 2))
	{
		/* Amount (12 Bytes) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], szTemplate, 12);
		inSubIndex += 12;
		inSubSize += 12;

		/* Timeout (3 Bytes) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%03d",  srMultiHost->stMulti_TransData.inCTLS_Timeout);
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], szTemplate, 3);
		inSubIndex += 3;
		inSubSize += 3;

		//modify by green 160130 修改感應金融卡交易不過問題 因少帶DATA
		/* EDC TIME */
//		vdFINA_EDC_SetTranTime(pobTran);/* vx820 V3UL 金融卡SALE感應 因時間有錯 被主機拒絕,依感應成功的QP PPV3 BuildIn流程 算查核碼前 將過卡時間抓到szTime modify by sunny20180125 */
//		read_clock(pobTran->srBRec.szCardTime);/* vx820 V3UL 金融卡SALE感應 因時間有錯 被主機拒絕,依感應成功的QP PPV3 BuildIn流程 算查核碼前 將過卡時間抓到szTime modify by sunny20180125 */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		SVC_CLOCK(GET_CLOCK, szTemplate, 15);
		
		//memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], pobTran->srBRec.szDate, 8);
                
                /* 同步卡機運算TCC和感應器的時間 */
                if (1)
                {
                        /* 取得EDC時間日期 */
                        if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
                        {
                                if (ginDebug == VS_TRUE)
                                {
                                        sprintf(szDebugMSG,"inFunc_GetDateAndTime ERROR");
                                        inDISP_LogPrintf(AT,szDebugMSG);
                                }

                                /* 感應燈號及聲響 */
                                pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_03_TIME_ERROR_;

                                return (VS_ERROR);
                        }

                        memset(&pobTran->srBRec.szDate, 0x00, sizeof(pobTran->srBRec.szDate));
                        memset(&pobTran->srBRec.szTime, 0x00, sizeof(pobTran->srBRec.szTime));
                        sprintf(pobTran->srBRec.szDate, "20%02d%02d%02d",  srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
                        sprintf(pobTran->srBRec.szTime, "%02d%02d%02d",  srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
                        sprintf(pobTran->srBRec.szFiscDateTime, "%s%s", pobTran->srBRec.szDate, pobTran->srBRec.szTime);
                        inDISP_LogPrintfWithFlag(" inFunc_GetSystemDateAndTime(%s)", pobTran->srBRec.szFiscDateTime);    
                }    
                
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], pobTran->srBRec.szDate, 8);
		inSubIndex += 8;
		inSubSize += 8;
		inDISP_LogPrintfWithFlag(" Multi Fisc Date(%s)", pobTran->srBRec.szDate);    
                
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], pobTran->srBRec.szTime, 6);
		inSubIndex += 6;
		inSubSize += 6;
                
                inDISP_LogPrintfWithFlag(" Multi Fisc Time(%s)", pobTran->srBRec.szTime);    
                inDISP_LogPrintfWithFlag(" Multi Fisc Date Time(%s)", pobTran->srBRec.szFiscDateTime); 
                
//		inLoadHDTRec(FINA_HOST);/* add by sampo MID和TID需使用Smartpay的 */
//		inLoadMHTRec(FINA_HOST);/* 要金融卡的STAN */
                
                memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetFiscFuncEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
                {    
                        pobTran->srBRec.inHDTIndex = _HDT_INDEX_07_FISC_;
                        inLoadHDTRec(pobTran->srBRec.inHDTIndex);
                        inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

                        /* TID (10 Bytes) */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetTerminalID(szTemplate);
                        memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], &szTemplate[0], 8);
                        
                        memset(szFiscTID, 0x00, sizeof(szFiscTID));
                        memcpy(&szFiscTID[0], &szTemplate[0], 8);
                        
                        inSubIndex += 10;
                        inSubSize += 10;
                        inDISP_LogPrintfWithFlag(" Multi Fisc TID(%s)", szTemplate);

                        /* MID (15 Bytes) */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetMerchantID(szTemplate);
                        memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], &szTemplate[0], 15);
                        inSubIndex += 15;
                        inSubSize += 15;
                        inDISP_LogPrintfWithFlag(" Multi Fisc MID(%s)", szTemplate);

//                        if (inEDC_FISC_GenTerminalCheckCode(pobTran) != VS_SUCCESS)
//                        {
//                                if (ginDebug == VS_TRUE)
//                                {
//                                        memset(szDebugMSG, 0x00, sizeof(szDebugMSG));
//                                        sprintf(szDebugMSG, "inFISC_GenTCC Error");
//                                        inDISP_LogPrintf(AT,szDebugMSG);
//                                }
//
//                                /* 感應燈號及聲響 */
//                                pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_04_MAC_TAC_;
//
//                                return (VS_ERROR);
//                        }
                        
                        pobTran->srBRec.inHDTIndex = _HDT_INDEX_00_HOST_;
                        inLoadHDTRec(pobTran->srBRec.inHDTIndex);
                        inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
                        
                        inDISP_LogPrintfWithFlag(" Multi Fisc TCC(%s)",pobTran->srBRec.szFiscTCC);
                }
                else
                {
                        /* TID (10 Bytes) */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetTerminalID(szTemplate);
                        memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], &szTemplate[0], 8);
                        inSubIndex += 10;
                        inSubSize += 10;

                        /* MID (15 Bytes) */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetMerchantID(szTemplate);
                        memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], &szTemplate[0], 15);
                        inSubIndex += 15;
                        inSubSize += 15;
                }    
		//add by green 170216 修改金融卡算TAC 的checkcode 處理方式 為了給820算tac 所以要在給金額封包時就要給check code
//		if((inRetVal=inRunFunction(pobTran, FINA_EDC520820_CREATE_CHECK_CODE))!= VS_SUCCESS)
//		{
//			memset(szErrMsg,0x00,sizeof(szErrMsg));
//			memcpy(szErrMsg,"\x01\xf6\x03\xe6\x01\xba\x01\x9d\x01\xd0\x01\xc3\x01\xc4",14);/* 金利卡寫檔失敗,利(\x03\x6e)->融(\x03\xe6) modify by sunny20171115 */
//			vdDisplayAt(1, 3, szErrMsg, CLR_EOL);
//			inRunFunction(pobTran, EMV_REMOVE_CARD);
//			ACTIVITY_LOG("vdFINA_EDC_CreateTimeAndCheckCode_ERR");
//			return(inRetVal);
//		}

		/* NCCC使用 財金暫時無用  */
		/* TMK Key Index (2 Bytes) */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		sprintf(szTemplate, "%02d", ginNCCC_TMK_Index);
//		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], &szTemplate[0], 2);
		/* 暫時沒用，先塞空白 */
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], "   ", 2);
		inSubIndex += 2;
		inSubSize += 2;

		/* CUP MAC KEY (32 Bytes) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		//szTemplate[get_env("#CUPMAC", szTemplate, 32)] = 0x00;
		/* 暫時沒用，先塞空白 */
		memset(szTemplate, 0x20, 32);
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], &szTemplate[0], 32);
		inSubIndex += 32;
		inSubSize += 32;

		/* SmartPay CTLS TransLimit (12 Bytes) */
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], "000000300100", 12);
		inSubIndex += 12;
		inSubSize += 12;

		/* SmartPay CTLS EMVMerchantCategoryCode (4 Bytes) */
		memset(szMCC, 0x00, sizeof(szMCC));
                inGetMCCCode(szMCC);
		memset(szTemplate, 0x20, 4);
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], &szMCC[0], 4);
		inSubIndex += 4;
		inSubSize += 4;

		//inDISP_LogPrintfWithFlag(" szFiscTermCheckCode(%s)",pobTran->srBRec.szFiscTermCheckCode);
		//modify by green 170216 修改金融卡算TAC 的checkcode 處理方式
//		if(inGetFiscTransUseSam() == VS_TRUE)
			//memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex],&pobTran->srBRec.szFiscTermCheckCode[0],8);
			/* 暫時沒用，先塞空白 */
//                if (szFuncEnable[0] == 'Y')
//                {
//                        memset(szVersionID, 0x00, sizeof(szVersionID));
//                        inGetVersionID(szVersionID);
//                        
//                        if (!memcmp(&szVersionID[0], "29", 2) || !memcmp(&szVersionID[0], "11", 2) || !memcmp(&szVersionID[0], "14", 2))
//                                memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], &szFiscTID[0], 8);        
//                        else
//                                memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], &pobTran->srBRec.szFiscTCC[0], 8);
//                }
//                else
			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex],"        ",8);
//		else
//			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], &szFINATID[0], 8);
		inSubIndex += 8;
		inSubSize += 8;
		//modify by green 160130 修改感應金融卡交易不過問題 因少帶DATA end

		//add by green 170502 新增一般信用卡感應式退貨 start
		/* inTransactionCode (3 Bytes) */
/* 要補做 START  */
                /*20211116,浩瑋筆記,此部分須為送給820的資料,全國繳費需補*/
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		memset(szNATIONALPAY, 0x00, sizeof(szNATIONALPAY));
//		szNATIONALPAY[get_env("#NATIONALPAY", szNATIONALPAY, 2)] = 0x00;
//		memset(szNatinalshow, 0x00, sizeof(szNatinalshow));
//		szNatinalshow[get_env("#NATIONALSHOW", szNatinalshow, 1)] = 0x00;
//
//		//判斷是否全繳有開啟
//		/*此部分V3UL/820信用卡部分不檢核該欄位，金融卡才會針對該欄位來進行TAC的封裝，*/
//		/*而V3UL端也沒有另外判斷全繳交易別(sale),所以這邊做此處理 add by yude 180628*/
//		/*180809 add by yude  修正 不對transcode本身去做修改，否則會導致過卡流程判斷transcode部分出現問題*/
//              	if(szNATIONALPAY[0] == '1'&& pobTran->inTransactionCode == 1)/*代表全國繳費*/
//	  	{
//	  		if(szNatinalshow[0] == '1')/*一般繳費為主*/
//			{
//  				V3ULTransCode = NP_NORMAL_PAY;
//  			}
//  			else
//  			{
//		  		V3ULTransCode = NP_NATIONAL_PAY;
//  			}
//	  	}
//	  	else
//	  	{
//	  			V3ULTransCode = pobTran->inTransactionCode;
//
//	  	}
//
                /* 感應退貨使用 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%03d", pobTran->inTransactionCode);
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], szTemplate, 3);
/* 要補做 END  */		
		
		inSubIndex += 3;
		inSubSize += 3;
		//add by green 170502 新增一般信用卡感應式退貨 end

		//add by green 170222 for 820 合庫NP 使用 start
		//National pay enable
//		memset(szNATIONALPAY, 0x00, sizeof(szNATIONALPAY));
//		szNATIONALPAY[get_env("#NATIONALPAY", szNATIONALPAY, 2)] = 0x00;

		if (szNATIONALPAY[0] == '1')/*全國繳費版*/
			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex],"1",1);
		else
			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex],"0",1);
		inSubIndex += 1;
		inSubSize += 1;
/* 要補做 START  */
//		memcpy(&pobTran->srBRec.szEdcFiscIssuerId[0],szGetTermIssuerId(),8);
//		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex],&pobTran->srBRec.szEdcFiscIssuerId[0],8);
                
                memset(szIssuerID, 0x00, sizeof(szIssuerID));
                inGetIssuerID(szIssuerID); /* 財金 = 0060 */
		
                memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], szIssuerID, 4);
                inSubIndex += 4;
		inSubSize += 4;
                
                memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], "0000", 4);
		inSubIndex += 4;
		inSubSize += 4;
                
                inDISP_LogPrintfWithFlag(" szIssuerID(%s)", szIssuerID);

/* 要補做 START  */
//		if(VS_TRUE == inGetFdtFeeFlag())
//			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex],"Y",1);
//		else
			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex],"N",1);
		inSubIndex += 1;
		inSubSize += 1;

/* 要補做 START  */
//		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex],szGetFdtFeeAmt(),4);
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex],"0000",4);
		inSubIndex += 4;
		inSubSize += 4;
/* 要補做 START  */
//		memcpy(&pobTran->srBRec.szEdcTccCode[0],szGetTccCode(),8);
//		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex],&pobTran->srBRec.szEdcTccCode[0],8);
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], "00000000", 8);
		inSubIndex += 8;
		inSubSize += 8;
		
		//add by green 170222 for 820 合庫NP 使用 end
		/* NCCC使用 財金暫時無用  */
/* 要補做 START  */
//		if(COMBO_CREDFLAG == VS_TRUE)/*180605 使用COMBO卡於收銀機25(信用卡)時走信用卡需求FLAG 讓820可以辨識  ADD BY YUDE*/
//			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex],"1",1);/*將520的flag傳至820內去啟動*/
//		else
			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex],"0",1);
		inSubIndex += 1;
		inSubSize += 1;

/* 要補做 START  */
//		if(COMBO_FISCFLAG == VS_TRUE)/*180605 使用COMBO卡於無人收銀機26(金融卡)時走金融卡需求FLAG 讓820可以辨識  ADD BY YUDE*/
//			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex],"1",1);
//		else
			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex],"0",1);
		inSubIndex += 1;
		inSubSize += 1;

        }
        else if (!memcmp(szTempTransType, _MULTI_SIGNPAD_, 2))
        {
		/* 幣別 (3 Bytes) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], "NTD", 3);
		inSubIndex += 3;
		inSubSize += 3;

		/* 小數點位數 (2 Bytes) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d", 2);
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], szTemplate, 2);
		inSubIndex += 2;
		inSubSize += 2;

		/* Minus (1 Bytes) */
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], "0", 1);
		inSubIndex ++;
		inSubSize ++;

		/* Amount (12 Bytes) */
		if (pobTran->srBRec.inCode == _TIP_)
		{
			memset(szAmount, 0x00, sizeof(szAmount));
			sprintf(szAmount, "%010lu00", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
		}
		else
		{
			memset(szAmount, 0x00, sizeof(szAmount));
			sprintf(szAmount, "%010lu00", pobTran->srBRec.lnTxnAmount);
		}

		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], szAmount, 12);
		inSubIndex += 12;
		inSubSize += 12;
	}
	else if (!memcmp(szTempTransType, _MULTI_TMS_CAPK_, 2) ||
		 !memcmp(szTempTransType, _MULTI_TMS_MASTER_, 2) ||
		 !memcmp(szTempTransType, _MULTI_TMS_VISA_, 2) ||
		 !memcmp(szTempTransType, _MULTI_TMS_JCB_, 2))
	{
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], &srMultiHost->stMulti_TransData.szReceData[0], srMultiHost->stMulti_TransData.inSubPacketSize);
		inSubSize += _MULTI_SUB_SIZE_MAX_;
	}
	else if (!memcmp(szTempTransType, _MULTI_SLAVE_REBOOT_, 2) ||
		 !memcmp(szTempTransType, _MULTI_POLL_, 2)  ||
		 !memcmp(szTempTransType, _MULTI_EXCHANGE_, 2))
	{
		/* 無SubData */
		inSubSize = 0;
	}
	else if (!memcmp(szTempTransType, _MULTI_NOSIGN_, 2))
	{
		/* Amount (12 Bytes) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnTxnAmount);
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], szTemplate, 12);
		inSubIndex += 12;
		inSubSize += 12;

		/* 授權碼 (6 Bytes) */
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], &pobTran->srBRec.szAuthCode[0], 6);
		inSubIndex += 6;
		inSubSize += 6;

		/* inCode (3 Bytes) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%03d", pobTran->srBRec.inCode);
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], szTemplate, 3);
		inSubIndex += 3;
		inSubSize += 3;

		/* inTransactionCode (3 Bytes) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%03d", pobTran->inTransactionCode);
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], szTemplate, 3);
		inSubIndex += 3;
		inSubSize += 3;
	}
	else if (!memcmp(szTempTransType, _MULTI_SIGN_CONFIRM_, 2))
	{
		/* 先設定正負號 */
		if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
		{
			if (pobTran->srBRec.inCode == _VOID_)
			{
				if (pobTran->srBRec.inOrgCode == _SALE_ || pobTran->srBRec.inOrgCode == _PRE_COMP_)
					pobTran->fMultiMinus = VS_TRUE;
				else if (pobTran->srBRec.inOrgCode == _REFUND_ || pobTran->srBRec.inOrgCode == _REDEEM_REFUND_ ||
					pobTran->srBRec.inOrgCode == _INST_REFUND_ || pobTran->srBRec.inOrgCode == _CUP_REFUND_)
					pobTran->fMultiMinus = VS_FALSE;
				else
					pobTran->fMultiMinus = VS_TRUE;
			}
			else if (pobTran->srBRec.inCode == _CUP_VOID_ || pobTran->srBRec.inCode == _CUP_REFUND_)
			{
				pobTran->fMultiMinus = VS_TRUE;
			}
		}
		else
		{
			if (pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_)
				pobTran->fMultiMinus = VS_FALSE;
			else if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inOrgCode == _REDEEM_REFUND_ ||
								pobTran->srBRec.inOrgCode == _INST_REFUND_ || pobTran->srBRec.inOrgCode == _CUP_REFUND_)
				pobTran->fMultiMinus = VS_TRUE;
			else
				pobTran->fMultiMinus = VS_FALSE;
		}

		/* 幣別 (3 Bytes) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], "NTD", 3);
		inSubIndex += 3;
		inSubSize += 3;

		/* 小數點位數 (2 Bytes) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d", 2);
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], szTemplate, 2);
		inSubIndex += 2;
		inSubSize += 2;

		/* Minus (1 Bytes) */
		if (pobTran->fMultiMinus == VS_TRUE)
			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], "1", 1);
		else
			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], "0", 1);

		inSubIndex ++;
		inSubSize ++;

		/* Amount (12 Bytes) */
		if (pobTran->srBRec.inCode == _TIP_)
		{
			memset(szAmount, 0x00, sizeof(szAmount));
			sprintf(szAmount, "%010lu00", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
		}
		else
		{
			memset(szAmount, 0x00, sizeof(szAmount));
			sprintf(szAmount, "%010lu00", pobTran->srBRec.lnTxnAmount);
		}

		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], szAmount, 12);
		inSubIndex += 12;
		inSubSize += 12;

		/* 授權碼 (6 Bytes) */
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], pobTran->srBRec.szAuthCode, 6);
		inSubIndex += 6;
		inSubSize += 6;

		/* 交易別 (3 Bytes) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%03d", pobTran->srBRec.inCode);
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], szTemplate, 3);
		inSubIndex += 3;
		inSubSize += 3;

		/* Display Confirm flag (1 Byte) */
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE || pobTran->srBRec.uszInstallment == VS_TRUE ||
			pobTran->srBRec.uszRedemption == VS_TRUE)
		{
			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], "Y", 1);
		}
//		else if (pobTran->srBRec.fDCCTrans == VS_TRUE && pobTran->srBRec.fVoided == VS_FALSE)
//		{
//				memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], "Y", 1);
//		}
		else
		{
			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], "N", 1);
		}

		inSubIndex ++;
		inSubSize ++;

		/* 警語提示列 (2 Byte + X Bytes) */
		if (pobTran->srBRec.uszInstallment == VS_TRUE)
		{
			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], "01", 2);
			inSubIndex += 2;
			inSubSize += 2;

			/* X = 32 Bytes */
			memset(szAmount, 0x00, sizeof(szAmount));
			memset(szINSTTemp, 0x00, sizeof(szINSTTemp));
/* 要補做 */		
//			SVC_HEX_2_DSP(pobTran->srBRec.szF_Period,szINSTTemp,1);
//			pobTran->srBRec.lnInstallmentPeriod = atoi(szINSTTemp);
			pobTran->srBRec.lnInstallmentPeriod = 3;
			sprintf(szAmount, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], szAmount, 2);
			inSubIndex += 2;
			inSubSize += 2;

			memset(szAmount, 0x00, sizeof(szAmount));
			memset(szINSTTemp, 0x00, sizeof(szINSTTemp));
/* 要補做 */
//			SVC_HEX_2_DSP(pobTran->srBRec.szF_DownPayment,szINSTTemp,4);
//			pobTran->srBRec.lnInstallmentDownPayment = atol(szINSTTemp);
			pobTran->srBRec.lnInstallmentDownPayment = 0;
			sprintf(szAmount, "%010lu", pobTran->srBRec.lnInstallmentDownPayment + pobTran->srBRec.lnTipTxnAmount);
			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], szAmount, 10);
			inSubIndex += 10;
			inSubSize += 10;

			memset(szAmount, 0x00, sizeof(szAmount));
			memset(szINSTTemp, 0x00, sizeof(szINSTTemp));
/* 要補做 */
//			SVC_HEX_2_DSP(pobTran->srBRec.szF_EachPayment,szINSTTemp,4);
//			pobTran->srBRec.lnInstallmentPayment = atol(szINSTTemp);
			pobTran->srBRec.lnInstallmentPayment = 0;
			sprintf(szAmount, "%010lu", pobTran->srBRec.lnInstallmentPayment);
			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], szAmount, 10);
			inSubIndex += 10;
			inSubSize += 10;

			memset(szAmount, 0x00, sizeof(szAmount));
			sprintf(szAmount, "%010lu", pobTran->srBRec.lnInstallmentFormalityFee);
			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], szAmount, 10);
			inSubIndex += 10;
			inSubSize += 10;
		}
		else if (pobTran->srBRec.uszRedemption == VS_TRUE)
		{
			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], "02", 2);
			inSubIndex += 2;
			inSubSize += 2;
			//modify by green 170426 新增紅利 修改送給820的資料
			/* X = 30 Bytes */
			//紅利折扣點數
			memset(szAmount, 0x00, sizeof(szAmount));
//			sprintf(szAmount, "%010lu", pobTran->srBRec.lnRedemptionPoints);
//			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], szAmount, 10);

/* 要補做 */
//			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], &pobTran->srBRec.srCRRec.szRedeemPointCost[0], 10);
			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], "0000000000", 10);
			inSubIndex += 10;
			inSubSize += 10;

			//紅利折扣後剩餘點數
			memset(szAmount, 0x00, sizeof(szAmount));
//			sprintf(szAmount, "%010lu", pobTran->srBRec.lnRedemptionPointsBalance);
//			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], szAmount, 10);
/* 要補做 */
//			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], &pobTran->srBRec.srCRRec.szRedeemBalance[0], 10);
			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], "0000000000", 10);
			inSubIndex += 10;
			inSubSize += 10;

			//支付金額
			memset(szAmount, 0x00, sizeof(szAmount));
//			sprintf(szAmount, "%010lu", pobTran->srBRec.lnRedemptionPaidCreditAmount);
//			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], szAmount, 10);
/* 要補做 */			
//			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], &pobTran->srBRec.srCRRec.szRedeemAmtAftCost[0], 10);
			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], "0000000000", 10);
			inSubIndex += 10;
			inSubSize += 10;
		}
		else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], "03", 2);
			inSubIndex += 2;
			inSubSize += 2;
			/* X = 0 Bytes */
		}
		else
		{
			memcpy(&srMultiHost->stMulti_TransData.szSendData[inSubIndex], "00", 2);
			inSubIndex += 2;
			inSubSize += 2;
			/* X = 0 Bytes */
		}
	}

	if (srMultiHost->stMulti_TransData.fMultiSend == VS_TRUE)
	{
		/* Sub Packet Size (4 Bytes) */
		inTotalSize += _MULTI_SUB_SIZE_MAX_;

		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%04d", srMultiHost->stMulti_TransData.inSubPacketSize);
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inLenlocation], &szTemplate[0], 4);
	}
	else
	{
		/* Sub Packet Size (4 Bytes) */
		inTotalSize += srMultiHost->stMulti_TransData.lnTotalPacketSize;

		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%04d", inSubSize);
		memcpy(&srMultiHost->stMulti_TransData.szSendData[inLenlocation], &szTemplate[0], 4);
	}

	inDISP_LogPrintfWithFlag("  Multi Host Pack TotalSize[%d] SubSize[%d]_END", inTotalSize, inSubSize);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (inTotalSize);
}


/*
 * Function		: inMultiFunc_Host_SendPacketWaitACK
 * Date&Time	: 2022/11/17 下午 2:25
 * Describe		: 
 * 在此處進行傳送資料組合，可利用 MULTI_TABLE 裏的 szTransType 進行判斷並組合相對應的資料
 * 使用開啟的 Rs232 Port 進行資料的傳送
 * 並且要收到外接設備回應的 ACK 訊息後，通訊才視為成功
 */
int inMultiFunc_Host_SendPacketWaitACK(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *srMultiHost)
{
	int 	inSendPacketSizes, inRetVal = VS_ERROR ;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag("  ComPort [%u]", (srMultiHost->srSetting.uszComPort +1));

	memset(srMultiHost->stMulti_TransData.szSendData, 0x20, sizeof(srMultiHost->stMulti_TransData.szSendData));
	
	/* 先預設初始值，如有需要改變再進行變動 */
	srMultiHost->stMulti_Optional_Setting.inACKTimeOut = 30;
	srMultiHost->stMulti_Optional_Setting.inMaxRetries = 1;
	srMultiHost->stMulti_Optional_Setting.uszPadStxEtx = VS_TRUE;
	srMultiHost->stMulti_Optional_Setting.uszWaitForAck = VS_TRUE;

	/* 組封包 */
	inSendPacketSizes = inMultiFunc_Host_PackResult(pobTran, srMultiHost);
	
	inDISP_LogPrintfWithFlag("  SnedPackAck PackLen [%d]", inSendPacketSizes);
	
	/* 送封包 */
	if (!memcmp(srMultiHost->stMulti_TransData.szTransType, _MULTI_SIGN_CONFIRM_, 2))
		srMultiHost->stMulti_Optional_Setting.inACKTimeOut = 10;
	
	inRetVal = inMultiFunc_Host_DataSend(srMultiHost, srMultiHost->stMulti_TransData.szSendData, inSendPacketSizes);
        
	inDISP_LogPrintfWithFlag("  MultiFUnc Send RetVal[%d]", inRetVal);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
	return (inRetVal);
}



int inMultiFunc_Host_SendPacketDotWaitAck(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *srMultiHost)
{
	int 	inSendPacketSizes, inRetVal = VS_ERROR ;
        char	szCTLSReaderMode[1 + 1] = {0};
        
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag("  ComPort [%u]", (srMultiHost->srSetting.uszComPort +1));
        
        /* 如果外接設備是A30，要用JSON */
        memset(szCTLSReaderMode, 0x00, sizeof(szCTLSReaderMode));
	inGetContactlessReaderMode(szCTLSReaderMode);
        {
                if (!memcmp(szCTLSReaderMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)))
                {
                    inRetVal = inMultiFunc_Host_JsonSendPacketWaitACK(pobTran, srMultiHost);
                    return inRetVal;
                }
        }
        
	memset(srMultiHost->stMulti_TransData.szSendData, 0x20, sizeof(srMultiHost->stMulti_TransData.szSendData));
	
	/* 先預設初始值，如有需要改變再進行變動 */
	srMultiHost->stMulti_Optional_Setting.inACKTimeOut = 30;
	srMultiHost->stMulti_Optional_Setting.inMaxRetries = 1;
	srMultiHost->stMulti_Optional_Setting.uszPadStxEtx = VS_TRUE;
	srMultiHost->stMulti_Optional_Setting.uszWaitForAck = VS_FALSE;
	
	/* 組封包 */
	inSendPacketSizes = inMultiFunc_Host_PackResult(pobTran, srMultiHost);

	inDISP_LogPrintfWithFlag("  SnedPackAck PackLen [%d]", inSendPacketSizes);
	/* 送封包 */
	if (!memcmp(srMultiHost->stMulti_TransData.szTransType, _MULTI_SIGN_CONFIRM_, 2))
		srMultiHost->stMulti_Optional_Setting.inACKTimeOut  = 10;
	
	inRetVal = inMultiFunc_Host_DataSend(srMultiHost, srMultiHost->stMulti_TransData.szSendData, inSendPacketSizes);

	inDISP_LogPrintfWithFlag("  MultiFUnc Send RetVal[%d]", inRetVal);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
	return (inRetVal);
}



/*
Function        : inMultiFunc_Host_CallSendErrorFunc
Date&Time   : 
Describe        : 傳送錯誤訊息ECR
 */
int inMultiFunc_Host_CallSendErrorFunc(TRANSACTION_OBJECT * pobTran, int inErrorType)
{
	int     inIndex = 0;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag("  Com[%u]", gstMultiOb.srSetting.uszComPort);

	gstMultiOb.stMulti_TransData.inErrorType = inErrorType;

	if (srMultiHostTransTb[inIndex].inMultiSendError(pobTran, &gstMultiOb) != VS_SUCCESS)
		return (VS_ERROR);

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}



/*
Function        : inMultiFunc_Host_SendError
Date&Time   : 
Describe        : 傳送錯誤訊息ECR
 */
int inMultiFunc_Host_SendError(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *stMultiOb)
{
	int	inRetVal = VS_SUCCESS;
	int	inTotalSize = 0;
	int	inSubSize = 0;
	int	inLenlocation = 0;
        char	szMultiComPort1[4 + 1];
	char	szTemplate[128 + 1];
	char	szPackData[_MULTI_MAX_SIZES_ + 1];
        char	szCTLSReaderMode[1 + 1] = {0};

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);	
	
        /* 如果外接設備是A30，要用JSON */
        memset(szCTLSReaderMode, 0x00, sizeof(szCTLSReaderMode));
	inGetContactlessReaderMode(szCTLSReaderMode);
        {
                if (!memcmp(szCTLSReaderMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)))
                {
                    inRetVal = inMultiFunc_Host_JsonSendError(pobTran, stMultiOb);
                    return inRetVal;
                }
        }
        
	memset(szPackData, 0x20, sizeof(szPackData));

	/* Bank (4 Bytes) */
	memcpy(&szPackData[inTotalSize], &stMultiOb->stMulti_TransData.szTranHost[0], 4);
	inTotalSize += 4;

	/* EDC SN (6 Bytes) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, stMultiOb->stMulti_TransData.szTermSN, 6);
	memcpy(&szPackData[inTotalSize], szTemplate, 6);
	inTotalSize += 6;

	inDISP_LogPrintfWithFlag("  Multi Host Send inErrorType[%d]", stMultiOb->stMulti_TransData.inErrorType);

	/* Response Code (4 Bytes) */
	if (stMultiOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_SUCCESS_)
		memcpy(&szPackData[inTotalSize], "0000", 4);
	else if (stMultiOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_USER_TERMINATE_ERROR_)
		memcpy(&szPackData[inTotalSize], "0002", 4);
	else if (stMultiOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_TIMEOUT_)
		memcpy(&szPackData[inTotalSize], "0003", 4);
	else if (stMultiOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_CTLS_HOST_CANCEL_)
		memcpy(&szPackData[inTotalSize], "0004", 4);
	else if (stMultiOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_AMOUNT_ERROR_)
		memcpy(&szPackData[inTotalSize], "0006", 4);
	else if (stMultiOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_)
		memcpy(&szPackData[inTotalSize], "0000", 4);
	else if (stMultiOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_CTLS_ERROR_)
		memcpy(&szPackData[inTotalSize], "0000", 4);
	else
		memcpy(&szPackData[inTotalSize], "0001", 4);

	inTotalSize += 4;

	/* Trans Type (2 Bytes) */
	memcpy(&szPackData[inTotalSize], &stMultiOb->stMulti_TransData.szTransType[0], 2);
	inTotalSize += 2;

	/* Total Packet (2 Bytes) */
	memcpy(&szPackData[inTotalSize], "01", 2);
	inTotalSize += 2;

	/* Sub Packet Index (2 Bytes) */
	memcpy(&szPackData[inTotalSize], "01", 2);
	inTotalSize += 2;

	/* Total Packet Size (6 Bytes) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%06d", _MULTI_SUB_SIZE_SMALL_);
	memcpy(&szPackData[inTotalSize], &szTemplate[0], 6);
	inTotalSize += 6;

	/* Sub Packet Size (4 Bytes) - 最後再算，動態 */
	inLenlocation = inTotalSize;
	inTotalSize += 4;

	inSubSize = 0;
	
	/* Sub Packet Size (4 Bytes) */
	memset(szTemplate, 0x00, sizeof (szTemplate));
	sprintf(szTemplate, "%04d", inSubSize);
	memcpy(&szPackData[inLenlocation], &szTemplate[0], 4);
	
	/* SubData Size */
	inTotalSize += _MULTI_SUB_SIZE_SMALL_;


	/* 送封包 */
	/* 用完要清掉設定 */
	memset(&stMultiOb->stMulti_Optional_Setting, 0x00, sizeof(stMultiOb->stMulti_Optional_Setting));
	stMultiOb->stMulti_Optional_Setting.inACKTimeOut = 2;
	stMultiOb->stMulti_Optional_Setting.inMaxRetries = 0;
	stMultiOb->stMulti_Optional_Setting.uszPadStxEtx = VS_TRUE;
	stMultiOb->stMulti_Optional_Setting.uszWaitForAck = VS_FALSE;
        
        inRetVal = inMultiFunc_Host_Send(pobTran, stMultiOb, szPackData, inTotalSize);

	/* 用完要清掉設定 */
	memset(&stMultiOb->stMulti_Optional_Setting, 0x00, sizeof(stMultiOb->stMulti_Optional_Setting));
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d]  Send Fail END -----",__FILE__, __FUNCTION__, __LINE__);	
		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
	
	return (VS_SUCCESS);
}


int inMultiFunc_Host_DataSendMultiError(MULTI_TABLE* srMultiHost, char *szECRRespCode)
{
	int	inTotalSize = 0;
	int	inRetVal;
	char	szTemplate[4];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);	
	memset(srMultiHost->stMulti_TransData.szSendData, 0x20, sizeof(srMultiHost->stMulti_TransData.szSendData));

	srMultiHost->stMulti_Optional_Setting.inACKTimeOut = 2;
	srMultiHost->stMulti_Optional_Setting.inMaxRetries = 0;
	srMultiHost->stMulti_Optional_Setting.uszPadStxEtx = VS_TRUE;
	srMultiHost->stMulti_Optional_Setting.uszWaitForAck = VS_FALSE;

	/* Bank (4 Bytes) */
	memcpy(&srMultiHost->stMulti_TransData.szSendData[inTotalSize], srMultiHost->stMulti_TransData.szTranHost, 4);
	inTotalSize += 4;

	/* EDC SN (6 Bytes) */
	memcpy(&srMultiHost->stMulti_TransData.szSendData[inTotalSize], srMultiHost->stMulti_TransData.szTermSN, 6);
	inTotalSize += 6;

	/* Response Code (4 Bytes) */
	memcpy(&srMultiHost->stMulti_TransData.szSendData[inTotalSize], &szECRRespCode[0], 4);
	inTotalSize += 4;

	/* Trans Type (2 Bytes) */
	memcpy(&srMultiHost->stMulti_TransData.szSendData[inTotalSize], srMultiHost->stMulti_TransData.szTransType, 2);
	inTotalSize += 2;

	/* Total Packet (2 Bytes) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%02d", 1);
	memcpy(&srMultiHost->stMulti_TransData.szSendData[inTotalSize], &szTemplate[0], 2);
	inTotalSize += 2;

	/* Sub Packet Index (2 Bytes) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%02d", 1);
	memcpy(&srMultiHost->stMulti_TransData.szSendData[inTotalSize], &szTemplate[0], 2);
	inTotalSize += 2;

	/* Total Packet Size (6 Bytes) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%06d", _MULTI_SUB_SIZE_SMALL_);
	memcpy(&srMultiHost->stMulti_TransData.szSendData[inTotalSize], &szTemplate[0], 6);
	inTotalSize += 6;

	/* Sub Packet Size (4 Bytes) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%04d", 0);
	memcpy(&srMultiHost->stMulti_TransData.szSendData[inTotalSize], &szTemplate[0], 4);
	inTotalSize += 4;

	/* SubData Size */
	inTotalSize += _MULTI_SUB_SIZE_SMALL_;


	inRetVal = inMultiFunc_Host_DataSend(srMultiHost, srMultiHost->stMulti_TransData.szSendData, inTotalSize);
	
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
	return (VS_SUCCESS);
}

/*
 * Function		: inMultiFunc_Host_End
 * Date&Time	: 2022/11/17 下午 1:57
 * Describe		: 
 *  清除暫存的Rs232 資料，並關閉已開啟的外接設備 COM PORT
 * Miyano新增USB 20230411
 */
int inMultiFunc_Host_End(MULTI_TABLE* stMultiOb)
{
	int     inRetVal = 0;
	char	szComPort[4 + 1];
        char	szDebugMsg[100 + 1];
        
        inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START-----",__FILE__, __FUNCTION__, __LINE__);
        
        /*清空接收的buffer*/
	inMultiFunc_Host_FlushRxBuffer(stMultiOb->srSetting.uszComPort);
        
	memset(szComPort, 0x00, sizeof(szComPort));
	inGetMultiComPort1(szComPort);
	if (memcmp(szComPort, "COM1", strlen("COM1")) == 0	||
	    memcmp(szComPort, "COM2", strlen("COM2")) == 0	||
	    memcmp(szComPort, "COM3", strlen("COM3")) == 0	||
	    memcmp(szComPort, "COM4", strlen("COM4")) == 0)
	{
		if (inRS232_Close(stMultiOb->srSetting.uszComPort) != VS_SUCCESS)
                {
                        inDISP_LogPrintfWithFlag("-----[%s][%s][%d] *Error* END-----",__FILE__, __FUNCTION__, __LINE__);	
                        return (VS_ERROR);
                }
	}
	else if (memcmp(szComPort, "USB", strlen("USB")) == 0)
	{
                if (inUSB_HostClose() != VS_SUCCESS)
                {
                    return (VS_ERROR);
                }
	}
	else if (memcmp(szComPort, "WIFI", strlen("WIFI")) == 0)
	{
                inDISP_LogPrintf("WIFI");
	}
	else
	{
		inDISP_LogPrintf("else");
                inRetVal = VS_ERROR;
	}	

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END-----",__FILE__, __FUNCTION__, __LINE__);	
	return (VS_SUCCESS);
}


/*
 * Function		: inMultiFunc_Host_InitialComPort
 * Date&Time	: 2022/11/17 下午 1:55
 * Describe		: 
 * 初始化要使用外接設備的 COM PORT
 */
int inMultiFunc_Host_InitialComPort(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
        int     inIndex = 0;
	char	szTMSOK[2 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);	

	/* 檢查是否做過【參數下載】 */
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);

	inDISP_LogPrintfWithFlag("  Multi Host Init TMSOK[%s]", szTMSOK);
	
	if (szTMSOK[0] == '0')
	{
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] TMS NOT OK END -----",__FILE__, __FUNCTION__, __LINE__);	
		return (VS_SUCCESS);
	}
	/* 如已設定過，又重新設定，就要先斷線再重新初始化 2022/11/17 */
	if(gstMultiOb.srSetting.uszSettingOK == VS_TRUE)
		srMultiHostTransTb[inIndex].inMultiEnd(&gstMultiOb);

	if ((inRetVal = srMultiHostTransTb[inIndex].inMultiInitial( &gstMultiOb)) == VS_ERROR)
	{
		inDISP_LogPrintfWithFlag(" Multi initial *Error* RetVal[%d]", inRetVal);		
		gstMultiOb.srSetting.uszComPort = 9;
		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag(" Return[%d] MultiCom [%u]", inRetVal, (gstMultiOb.srSetting.uszComPort+1));
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
 *  520 原名 inMultiFunc_HostData_Recv_CTLS
 * 
 */

int inMultiFunc_Host_RecvCtlsData(TRANSACTION_OBJECT *pobTran)
{
	int	i, inSize = 0;
        int     inRetVal = 0;
   	char	szTemplate[42 + 1];
    	char	szCTLSReaderMode[1 + 1] = {0};
	unsigned short usMaxRecLen = _MULTI_RECV_SIZE_;
	unsigned char uszLRCData;
	unsigned char	uszSTX[2 + 1];
	unsigned char	uszRecBuf[_MULTI_MAX_SIZES_ + 4], szGetECRData[_MULTI_MAX_SIZES_ + 4];
	unsigned short	usOneSize = 1;

	/* 如果外接設備是A30，要用JSON */
        memset(szCTLSReaderMode, 0x00, sizeof(szCTLSReaderMode));
	inGetContactlessReaderMode(szCTLSReaderMode);
        {
                if (!memcmp(szCTLSReaderMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)))
                {
                    inRetVal = inMultiFunc_Host_JasonWaitRecvDataForCallSlve(pobTran);
                    return inRetVal;
                }
        }
	
#if 0
	/* Comport有東西 */
	if(inMultiFunc_Host_Data_Receive_Check(gstMultiOb.srSetting.uszComPort, &usOneSize) != VS_SUCCESS)
	{
		return VS_WAVE_NO_DATA;
	}
	
	usOneSize = 1;
	if (inMultiFunc_Host_Data_Receive(gstMultiOb.srSetting.uszComPort, &uszSTX[0], &usOneSize) == VS_SUCCESS)
	{
		if (uszSTX[0] == _STX_)
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_2_, 5);
			
			inDISP_LogPrintfWithFlag(" Multi Host Rec STX Start LINE[%d]",__LINE__);

			uszRecBuf[inSize ++] = _STX_;

			usMaxRecLen -- ;

			while (1)
			{
				/* 銀行別 + 回應碼 + 功能別 */
				//usOneSize = 100;
				usOneSize = usMaxRecLen;

				inDISP_LogPrintfWithFlag("  Multi Host Rec Read Len [%u] Line[%d]", usOneSize,  __LINE__);

				if (inMultiFunc_Host_Data_Receive(gstMultiOb.srSetting.uszComPort, &uszRecBuf[inSize], &usOneSize) == VS_SUCCESS)
				{
					inSize += usOneSize;
					usMaxRecLen -= usOneSize;

					if (uszRecBuf[inSize - 2] == _ETX_)
					{
						/* 算LRC */
                                                inDISP_LogPrintfWithFlag("  Multi Host Remaining Len [%u][%d] Line[%d]", usMaxRecLen, inSize,  __LINE__);//修正部分卡片無法感應的問題 add by sampo 20201216
                                                
                                                for (i = 1; i < (inSize-1); i++)
                                                {
                                                        uszLRCData ^= uszRecBuf[i];
                                                        inDISP_LogPrintf("Miyano Flag 迴圈 i = %d, uszRecBuf[%d] = %02x", i, i , uszRecBuf[i]);
                                                }
              
						break;
					}

				}

				if (inTimerGet(_TIMER_NEXSYS_2_) == 0)
				{
					inDISP_LogPrintfWithFlag("  Multi Host Rec Data Aft EXT Timout *Error* Line[%d]", __LINE__);
					return (VS_TIMEOUT);
				}

			}
                        
                        inDISP_LogPrintf("Miyano Flag uszLRCData = %02x", uszLRCData);
                        inDISP_LogPrintf("Miyano Flag uszRecBuf[inSize - 1] = %02x", uszRecBuf[inSize - 1]);
                        
			/* 驗證LRC */
			if (uszLRCData == uszRecBuf[inSize - 1])
			{
				inDISP_LogPrintfWithFlag("  Multi Host Rec LRC OK Line[%d]", __LINE__);
			}
			else
			{
				inDISP_LogPrintfWithFlag("  Multi Host Rec LRC NOT OK Line[%d]", __LINE__);
				return (VS_FAILURE);
			}
		}
		else
		{
			inDISP_LogPrintfWithFlag("  Multi Host Rec Not Found STX Val[0x%x] Line[%d]", uszSTX[0], __LINE__ );
			return (VS_WAVE_NO_DATA);
		}
	}
	else
	{		
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d]  No Data *Error* END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_WAVE_NO_DATA);
	}
	
	
#else	
	/* 檢核有無收到第一個STX */
	memset(uszRecBuf,0x00,sizeof(uszRecBuf));

	if(gstMultiOb.srSetting.inTimeout > 0)
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_2_, gstMultiOb.srSetting.inTimeout);
	}else
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_2_, 10);
	}
        
        /* Comport有東西 */
	if(inMultiFunc_Host_Data_Receive_Check(gstMultiOb.srSetting.uszComPort, usOneSize) != VS_SUCCESS)
	{
		return VS_WAVE_NO_DATA;
	}
        
	while (1)
	{
		/* Comport有東西 */
		while (inMultiFunc_Host_Data_Receive_Check(gstMultiOb.srSetting.uszComPort, usOneSize) != VS_SUCCESS)
		{
			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_2_) == 0)
			{
//				return (VS_TIMEOUT);
                                return (VS_WAVE_NO_DATA);
			}
		}

		/* 這是Buffer的大小，若沒收到則該值會轉為0，所以需重置 */
		usOneSize = 1;
                
		if (inMultiFunc_Host_Data_Receive(gstMultiOb.srSetting.uszComPort, uszSTX, usOneSize) == VS_SUCCESS)
                {
			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_2_) == 0)
			{
				return (VS_TIMEOUT);
			}

			if (uszSTX[0] == _STX_)
			{
				inDISP_LogPrintfWithFlag(" Multi Host Rec STX Start LINE[%d]",__LINE__);
				uszRecBuf[inSize ++] = _STX_;
				usMaxRecLen -- ;
				while (1)
				{
					/* 銀行別 + 回應碼 + 功能別 */
					//usOneSize = 100;
					usOneSize = usMaxRecLen;
					inDISP_LogPrintfWithFlag("  Multi Host Rec Read Len [%u] Line[%d]", usOneSize,  __LINE__);

					if (inMultiFunc_Host_Data_Receive(gstMultiOb.srSetting.uszComPort, &uszRecBuf[inSize], usOneSize) == VS_SUCCESS)
					{
						inSize += usOneSize;
						usMaxRecLen -= usOneSize;

						if (uszRecBuf[inSize - 2] == _ETX_)
						{
							/* 算LRC */
                                                        inDISP_LogPrintfWithFlag("  Multi Host Remaining Len [%u][%d] Line[%d]", usMaxRecLen, inSize,  __LINE__);//修正部分卡片無法感應的問題 add by sampo 20201216
                                                        if (usMaxRecLen == 0) //修正部分卡片無法感應的問題 add by sampo 20201216
                                                        {
                                                                for (i = 1; i < (inSize-1); i++)
                                                                {
                                                                        uszLRCData ^= uszRecBuf[i];
                                                                }
                                                                break;
                                                        }
						}
						
					}
					
					if (inTimerGet(_TIMER_NEXSYS_2_) == 0)
					{
						inDISP_LogPrintfWithFlag("  Multi Host Rec Data Aft EXT Timout *Error* Line[%d]", __LINE__);
						return (VS_TIMEOUT);
					}

				}
				
				/* 驗證LRC */

				if (uszLRCData == uszRecBuf[inSize - 1])
				{
					inDISP_LogPrintfWithFlag("  Multi Host Rec LRC OK Line[%d]", __LINE__);
					break;
				}
				else
				{
					inDISP_LogPrintfWithFlag("  Multi Host Rec LRC NOT OK Line[%d]", __LINE__);
                                        break;
//                                        return (VS_FAILURE);
				}
				
    			}
			else
			{
				inDISP_LogPrintfWithFlag("  Multi Host Rec Not Found STX Val[0x%x] Line[%d]", uszSTX[0], __LINE__ );
			}
    		}
    		else
    		{		
    			return (VS_WAVE_NO_DATA);
    		}
	}
#endif	
	
	/*  資料不含 STX ETX LRC */
	memset(szGetECRData, 0x00, sizeof(szGetECRData));
	memcpy(szGetECRData, (char*)&uszRecBuf[1], inSize - 3);

	inDISP_LogPrintfWithFlag("  Multi Host Rec RealDataLen[%d] Line[%d]", (inSize - 3), __LINE__);

	inMulti_HOST_PrintReceiveDeBug((char *)szGetECRData, inSize -3, inSize);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szGetECRData[14], 2);

	inDISP_LogPrintfWithFlag("  Code [%x][%x] Line[%d]",(unsigned char)szTemplate[0],(unsigned char)szTemplate[1], __LINE__);
	
	if (!memcmp(szTemplate, _MULTI_CTLS_, 2))
	{
		/* 檢核回應碼 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &szGetECRData[10], 4);

		if (!memcmp(szTemplate, "0000", 4))
		{
			inDISP_LogPrintfWithFlag("  Multi Host Rec ONE TAP Flow Line[%d]", __LINE__);
			/* ONE TAP */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &szGetECRData[26], 4);

			st_inMultiHostCtlsLen = atoi(szTemplate);

			memset(st_uszMultiHostCTLSData, 0x00, sizeof(st_uszMultiHostCTLSData));
			memcpy(&st_uszMultiHostCTLSData[0], &szGetECRData[30], st_inMultiHostCtlsLen);

			fTwo_Tap = VS_FALSE;
		}
		else if (!memcmp(szTemplate, "1111", 4))
		{
			inDISP_LogPrintfWithFlag("  Multi Host Rec TWO TAP Flow Line[%d]", __LINE__);
			/* TWO TAP */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &szGetECRData[26], 4);

			st_inMultiHostCtlsLen = atoi(szTemplate);

			memset(st_uszMultiHostCTLSData, 0x00, sizeof(st_uszMultiHostCTLSData));
			memcpy(&st_uszMultiHostCTLSData[0], &szGetECRData[30], st_inMultiHostCtlsLen);

			fTwo_Tap = VS_TRUE;
		}
		else if (!memcmp(szTemplate, "0002", 4))
		{
			inDISP_LogPrintfWithFlag("  Multi Host Rec Return User Cancel");
			/* Cancel */
			return (VS_USER_CANCEL);
		}
		else if (!memcmp(szTemplate, "0003", 4))
		{
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Reutrn TIMEOUT *Error* END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_TIMEOUT);
		}
		else if (!memcmp(szTemplate, "0005", 4))/*20151014浩瑋新增*/
		{
			inDISP_LogPrintfWithFlag("  Multi Host Rec SmartPay Flow");
/* 要補 先忽略金融卡  */
#if 0			
			/* SmartPay */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &szGetECRData[26], 4);

			st_inMultiHostCtlsLen = atoi(szTemplate);

			inDISP_LogPrintfWithFlag("inMultiFunc_HostData_Recv_CTLS st_inMultiHostCtlsLen(%d)",st_inMultiHostCtlsLen);

			memset(st_uszMultiHostCTLSData, 0x00, sizeof(st_uszMultiHostCTLSData));
			memcpy(&st_uszMultiHostCTLSData[0], &szGetECRData[30], st_inMultiHostCtlsLen);


			/* 金融卡發卡單位代號 (8 Bytes) */
			memset(pobTran->srBRec.szFiscIssuerId, 0x00, sizeof(pobTran->srBRec.szFiscIssuerId));
			memcpy(&pobTran->srBRec.szFiscIssuerId[0], &st_uszMultiHostCTLSData[0], FISC_ISSUER_ID_SIZE);
			pobTran->srBRec.inFiscIssuerIdLength = FISC_ISSUER_ID_SIZE;/*8*/

			inDISP_LogPrintfWithFlag("inMultiFunc_HostData_Recv_CTLS szFiscIssuerId(%s)",pobTran->srBRec.szFiscIssuerId);

			/* 金融卡備註欄 (30 Bytes) */
			memset(pobTran->srBRec.szFiscIcCardCommet, 0x00, sizeof(pobTran->srBRec.szFiscIcCardCommet));
			memcpy(&pobTran->srBRec.szFiscIcCardCommet[0], &st_uszMultiHostCTLSData[8], FISC_CARD_COMMET_SIZE);
			pobTran->srBRec.inFiscIcCardCommetLength = FISC_CARD_COMMET_SIZE;/*30*/

			inDISP_LogPrintfWithFlag("inMultiFunc_HostData_Recv_CTLS szFiscIcCardCommet(%s)",pobTran->srBRec.szFiscIcCardCommet);

			/* 金融卡帳號 (16 Bytes) */
			memset(pobTran->srBRec.szFiscAccount, 0x00, sizeof(pobTran->srBRec.szFiscAccount));
			memcpy(&pobTran->srBRec.szFiscAccount[0], &st_uszMultiHostCTLSData[38], FISC_ACCOUNT_SIZE);
			memset(pobTran->srBRec.szPAN, 0x00, sizeof(pobTran->srBRec.szPAN));
			memcpy(&pobTran->srBRec.szPAN[0], &pobTran->srBRec.szFiscAccount[0], FISC_ACCOUNT_SIZE);
			pobTran->srBRec.inFiscAccountLength = FISC_ACCOUNT_SIZE;/*FISC2_ACCOUNT_SIZE*/

			inDISP_LogPrintfWithFlag("inMultiFunc_HostData_Recv_CTLS szFiscAccount(%s)",pobTran->srBRec.szFiscAccount);
			inDISP_LogPrintfWithFlag("inMultiFunc_HostData_Recv_CTLS szPAN(%s)",pobTran->srBRec.szPAN);

			/* 金融卡交易序號 (8 Bytes) */
			memset(pobTran->srBRec.szFiscSTAN, 0x00, sizeof(pobTran->srBRec.szFiscSTAN));
			memcpy(&pobTran->srBRec.szFiscSTAN[0], &st_uszMultiHostCTLSData[54], FISC_STAN_SIZE);
			/*20151012浩瑋筆記,由於長度目前都固定,因此為了不改變架構的情況,就直接給值了*/
			pobTran->srBRec.inFiscSTANLength = FISC_STAN_SIZE;
			inDISP_LogPrintfWithFlag("inMultiFunc_HostData_Recv_CTLS szFiscSTAN(%s)",pobTran->srBRec.szFiscSTAN);

			/* 金融卡交易授權驗證碼 (8 Bytes) */
			memset(pobTran->srBRec.szFiscTac, 0x00, sizeof(pobTran->srBRec.szFiscTac));
			memcpy(&pobTran->srBRec.szFiscTac[0], &st_uszMultiHostCTLSData[62], 8);
			/*20151012浩瑋筆記,由於長度目前都固定,因此為了不改變架構的情況,就直接給值了*/
			memset(pobTran->srBRec.szFiscTacLength, 0x00, sizeof(pobTran->srBRec.szFiscTacLength));
			memcpy(&pobTran->srBRec.szFiscTacLength[0], "\x00\x08", 2);
			inDISP_LogPrintfWithFlag("inMultiFunc_HostData_Recv_CTLS szFiscTac(%s)",pobTran->srBRec.szFiscTac);

			/* 計算TAC(S2)的交易日期時間 (14 Bytes) */
			memset(pobTran->srBRec.szCardTime, 0x00, sizeof(pobTran->srBRec.szCardTime));
			memcpy(&pobTran->srBRec.szCardTime[0], &st_uszMultiHostCTLSData[70], FISC_DATE_AND_TIME_SIZE);

			inDISP_LogPrintfWithFlag("inMultiFunc_HostData_Recv_CTLS szCardTime(%s)",pobTran->srBRec.szCardTime);

			/* 金融卡MCC (15 Bytes) 目前不會用 */
			memset(pobTran->srBRec.szFiscMCC, 0x00, sizeof(pobTran->srBRec.szFiscMCC));
			memcpy(&pobTran->srBRec.szFiscMCC[0], &st_uszMultiHostCTLSData[84], FISC_MCC_SIZE);

			inDISP_LogPrintfWithFlag("inMultiFunc_HostData_Recv_CTLS szFiscMCC(%s)",pobTran->srBRec.szFiscMCC);

			//add by green for 820 smartpaywave  F-25使用 inpaymentPOS
			/* inpaymentPOS (1 Bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &st_uszMultiHostCTLSData[99], 1);
			pobTran->srBRec.inpaymentPOS = atoi(szTemplate);
			inDISP_LogPrintfWithFlag("inMultiFunc_HostData_Recv_CTLS inpaymentPOS(%d)",pobTran->srBRec.inpaymentPOS);

			/* add by green 170222 for 820 Nationalpay start */
			/* lnOrgBaseTransactionAmount 12 bytes */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &st_uszMultiHostCTLSData[100], 10);
			pobTran->srBRec.lnOrgBaseTransactionAmount = atol(szTemplate);

			/* lnBaseTransactionAmount 12 bytes */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &st_uszMultiHostCTLSData[112], 10);
			pobTran->srBRec.lnBaseTransactionAmount = atol(szTemplate);

			/* lnOldTotalTransactionAmount 12 bytes */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &st_uszMultiHostCTLSData[124], 10);
			pobTran->srBRec.lnOldTotalTransactionAmount = atol(szTemplate);

			/* inNormalPayRule 2 bytes */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &st_uszMultiHostCTLSData[136], 2);
			pobTran->srBRec.inNormalPayRule = atol(szTemplate);
			/* add by green 170222 for 820 Nationalpay end */

			/* 金融卡 卡別(1 Bytes) add by sunny20180330 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &st_uszMultiHostCTLSData[138], 1);
			pobTran->srBRec.inFiscCardType = atoi(szTemplate);
			inDISP_LogPrintfWithFlag("inMultiFunc_HostData_Recv_CTLS inFiscCardType(%d)",pobTran->srBRec.inFiscCardType);

			if(pobTran->srBRec.inFiscCardType == 1 || pobTran->srBRec.inFiscCardType == 2)
			{

				/* 行動金融卡 EF1004資料 變動長度(3 Bytes) add by sunny20180330 */
				memset(pobTran->srBRec.szFiscEF1004Len, 0x00, sizeof(pobTran->srBRec.szFiscEF1004Len));
				memcpy(&pobTran->srBRec.szFiscEF1004Len[0], &st_uszMultiHostCTLSData[139], 3);
				inDISP_LogPrintfWithFlag("inMultiFunc_HostData_Recv_CTLS szFiscEF1004Len(%s)",pobTran->srBRec.szFiscEF1004Len);

				/* 行動金融卡 EF1004資料 (szFiscEF1004Len Bytes) add by sunny20180330 */
				inFiscEF1004Len = atoi( pobTran->srBRec.szFiscEF1004Len );//ASCII Len
				if(inFiscEF1004Len > 0)
				{
					memset(pobTran->srBRec.szFiscEF1004, 0x00, sizeof(pobTran->srBRec.szFiscEF1004));
					memcpy(&pobTran->srBRec.szFiscEF1004[0], &st_uszMultiHostCTLSData[142], inFiscEF1004Len);
					inDISP_LogPrintfWithFlag("inMultiFunc_HostData_Recv_CTLS szFiscEF1004(%s)",pobTran->srBRec.szFiscEF1004);
				}

			}


/* NCCC 用 */
//                  /* 金融卡TCC (8 Bytes) */
//                  memset(pobTran->srBRec.szFiscTCC, 0x00, sizeof(pobTran->srBRec.szFiscTCC));
//                  memcpy(&pobTran->srBRec.szFiscTCC[0], &st_uszMultiHostCTLSData[99], FISC_TCC_SIZE);

//                  /* MAC (8 Bytes)*//* 【需求單 - 104094】銀聯閃付需求 by Wei Hsiu - 2015/8/13 下午 10:12:06 */
//                  memset(pobTran->srBRec.szMAC_HEX, 0x00, sizeof(pobTran->srBRec.szMAC_HEX));
//                  memcpy(&pobTran->srBRec.szMAC_HEX[0], &st_uszMultiHostCTLSData[107], 8);
/* NCCC 用 end */

			fTwo_Tap = VS_FALSE;
#endif
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Smart Pay END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_SMARTPAY_SUCCESS);
		}
		else if (!memcmp(szTemplate, "0006", 4))
		{
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Wave Amount *Error* END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_WAVE_AMOUNT_ERR);
		}
		else if (!memcmp(szTemplate, "0007", 4))
		{
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Wave Mac *Error* END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_WAVE_MAC_ERR);
		}
		else
		{
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] User Terminate END -----",__FILE__, __FUNCTION__, __LINE__);
			return (_ECR_RESPONSE_CODE_USER_TERMINATE_ERROR_);
		}
	}
	else
	{
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Ecr Nodata END -----",__FILE__, __FUNCTION__, __LINE__);
		return (_ECR_RESPONSE_CODE_NODATA);
	}

	return (VS_SUCCESS);
}


/*
 * 原名 inMultiFunc_CALL_Slave_CTLS_Result
 * 
 * inMultiFunc_CallSlaveCtlsResult
 * 
 * 
 */

/*  
 *	會利用在 inMultiFunc_HostData_Recv_CTLS 抓取到的 st_uszMultiHostCTLSData 來處理晶片資料
 */
int inMultiFunc_CallSlaveCtlsResult(TRANSACTION_OBJECT *pobTran)
{
	int	i = 0, j = 0;
	int	inCount = 0, inRetVal = VS_ERROR;
	int	inReadDataSize = 0;
	int	inTrack1Size = 0, inTrack2Size = 0;
	int	inTotalLEN;
        char	szCTLSReaderMode[1 + 1] = {0};
	char 	szASCII[1024 + 4];
	unsigned char uszReadBuffer[1024 + 4];
	VS_BOOL fContactlessRefundJudge = VS_FALSE;

	VS_BOOL fPayPassFormat = VS_FALSE;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
        /* 如果外接設備是A30，要用JSON */
        memset(szCTLSReaderMode, 0x00, sizeof(szCTLSReaderMode));
	inGetContactlessReaderMode(szCTLSReaderMode);
        {
                if (!memcmp(szCTLSReaderMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)))
                {
                    inRetVal = inMultiFunc_Host_JsonUnpackCallSlaveCtlsResult(pobTran);
                    return inRetVal;
                }
        }
        
	memset(uszReadBuffer, 0x00, sizeof(uszReadBuffer));
	inTotalLEN = st_inMultiHostCtlsLen ;
	
	if(inTotalLEN > 1024 ){
	
		inDISP_LogPrintfWithFlag("   Ctls Len *Error* gMultiLen[%d] Line[%d]",st_inMultiHostCtlsLen, __LINE__);
		memcpy(&uszReadBuffer[0], &st_uszMultiHostCTLSData[0], 1024);		
	}		
	else{
		memcpy(&uszReadBuffer[0], &st_uszMultiHostCTLSData[0], inTotalLEN);
	}

	/*
		Success >= 		【0】		Number of bytes received/stored in buff.
		Failure			【-1】	Operating system error.
		E_IN_USE			【-104】	Another application has ownership of the device.
		E_NOT_AVAIL 		【-105】	Terminal hardware does not include a CTLS device.
		E_CTLS_APP_ERROR	【-101】	CTLS Application is not present/running.
	*/

	if (inTotalLEN > 0)
	{
		inDISP_ChineseFont("資料處理中", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_CENTER_);

		inDISP_LogPrintfWithFlag("[CTLS_SALE_RECV	%04d                                                ]", inTotalLEN);
		inDISP_LogPrintfWithFlag("[ 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 ]");
		inDISP_LogPrintfWithFlag("==============================================================================");

		memset(szASCII, 0x00, sizeof(szASCII));

		for ( ; i < inTotalLEN ; )
		{

			inFunc_BCD_to_ASCII(&szASCII[j],(unsigned char *) &uszReadBuffer[i ++], 1);
			j += 2;
			szASCII[j ++] = 0x20; /* 補空白 */

			if (i % 25 == 0)
			{
				inDISP_LogPrintfWithFlag("[ %s]", szASCII);
				memset(szASCII, 0x00, sizeof(szASCII));
				j = 0;
			}
			else if (i == inTotalLEN)
			{
				inDISP_LogPrintfWithFlag("[ %s]", szASCII);
				memset(szASCII, 0x00, sizeof(szASCII));
				j = 0;
			}
		} /* End for () .... */

		inDISP_LogPrintfWithFlag("==============================================================================");
		/* 在這裡送結束 */
	}
	else
	{
		return (VS_ERROR);
	}

	/* Header Tag and Protocol Version + Command */
	inCount += 11;
	/* Check Error Code
	If the Status Code being returned in the Response Packet is “Failed” and the Error
	Code is not Request Online Authorization, then the contents of the Data field will
	contain further information on the cause of the failure and will not contain the
	Track or Clearing Record information.
	當Status Codes = 0x0A，會有一個 Error Codes，提示進階的錯誤原因 */
	/* 如果是【0x0A】表示交易失敗，要處理 */
	if (uszReadBuffer[inCount] == 0x0A)
	{
		//add by green 170502 新增一般信用卡感應式退貨 (因退貨command 使用new J 會解析失敗 )
		if (pobTran->inTransactionCode == _INST_REFUND_ || pobTran->inTransactionCode == _REDEEM_REFUND_ || pobTran->inTransactionCode == _REFUND_)
		{
			//退貨的話0A當作成功 資料要另外解析 在這設定一個Flag
			 fContactlessRefundJudge = VS_TRUE;
		}
		else
		{
			inRetVal = inCTLS_VIVO_CheckErrorCodes(pobTran, &uszReadBuffer[inCount], inTotalLEN - inCount); /* 解析 Error Codes */
			return (inRetVal);
		}
	}
	else if(uszReadBuffer[inCount] == 0x23)/*20151014浩瑋新增*/
	{
		/* 【需求單 - 104094】銀聯閃付需求 by Wei Hsiu - 2015/8/13 下午 10:12:06 */
		/* Check Status Code */
		if ((inRetVal = inCTLS_VIVO_CheckStatusCodes(uszReadBuffer[inCount])) == VS_ERROR)
		{
			return (VS_ERROR); /* 交易失敗的情形 */
		}
		
		//pobTran->fCTLS_QuickPass = VS_FALSE;
		pobTran->uszQuickPassTag99 = VS_FALSE;
		
	}
	else if(uszReadBuffer[inCount] == 0x25)/*20151014浩瑋新增*/
	{
		inRetVal = inCTLS_VIVO_CheckStatusCodes(uszReadBuffer[inCount]);
		return (inRetVal);
	}
//    	/*----------SMART PAY----------(START)*/
//        else if(uszReadBuffer[inCount] == 0x25)
//        {
//		SVC_WAIT(100);
//		inRetVal = inCTLS_VIVO_SelectAID(pobTran);
//
//		if(inRetVal == VS_ERROR)
//		{
//			LOG_PRINTF(("inCTLS_VIVO_SelectAID ERROR"));
//			inRetVal = inCTLS_VIVO_CommitTransation(pobTran);
//			return (inRetVal);
//		}
//		else
//		{
//			LOG_PRINTF(("inCTLS_VIVO_SelectAID SUCCESS"));
//			pobTran->srBRec.inCode = FISC_SALE;
//			inCTLS_Func_ProcessFISCFlow(pobTran);
//			inCTLS_VIVO_Mifare_Close();
//			//inCTLS_PRINT_FISC_DATA(pobTran);
//
//			return (VS_SMARTPAY_SUCCESS);
//		}
//	}
//        /*----------SMART PAY----------(END)*/
	else
	{
		/* Check Status Code */
		if ((inRetVal = inCTLS_VIVO_CheckStatusCodes(uszReadBuffer[inCount])) == VS_ERROR)
		{
			return (VS_ERROR); /* 交易失敗的情形 */
		}
	}

	inCount += 1;

	/* 解析晶片或磁條資料 */
	/* Data Length (MSB) + Data Length (LSB) */
	inReadDataSize = uszReadBuffer[inCount] * 256 + uszReadBuffer[inCount + 1];
	inDISP_LogPrintfWithFlag("  Data_Length[%02X %02X][%d]-[inCount = %d]", uszReadBuffer[inCount], uszReadBuffer[inCount + 1], inReadDataSize, inCount);
	inCount += 2;

	//add by green 170502 新增一般信用卡感應式退貨 (因退貨command 使用new J 會解析失敗 )
	if ( fContactlessRefundJudge == VS_TRUE )
	{
		inCount += 4;
	}

	/* Track 1
	   ‧If Track 1 is available, then this field will give the
	   ‧length of the Track 1 data that will follow. If Track
	   ‧1 is not available, then a Length of 00h will be
	   ‧returned.
	   判斷 Track 1 */
	if (uszReadBuffer[inCount] == 0x00) /* Track 1 Length */
	{
		inDISP_LogPrintfWithFlag("  T1_Length[0][inCount = %d]", inCount);
		inCount ++ ;
	}
	else
	{
		/* ‧ Track 1 present –
			. Followed by length and Track 1 data
			. Length of track 1 – X (1)
			. Track 1 data – ASCII (Variable. up to 76)
		*/
		inTrack1Size = uszReadBuffer[inCount];
		inDISP_LogPrintfWithFlag("  T1_Length[%02X][%d]-[inCount = %d]", uszReadBuffer[inCount], inTrack1Size, inCount);
		inCount += 1;
		
		memset(pobTran->szTrack1, 0x00, sizeof(pobTran->szTrack1));
		pobTran->shTrack1Len = 0;
		
		
		memcpy(&pobTran->szTrack1[0], &uszReadBuffer[inCount], inTrack1Size);
		pobTran->shTrack1Len = inTrack1Size;
//		pobTran->srBRec.fT1Data = VS_TRUE;
		inCount += inTrack1Size;

		memset(pobTran->srBRec.szCardHolder, 0x00, sizeof(pobTran->srBRec.szCardHolder));

		for (i = 0; i < inTrack1Size; i ++)
		{
			if (pobTran->szTrack1[i] == 0x5E)
			{
				i ++;

				for (j = 0; j < _MAX_TRACK1_NAME_SIZE_; j ++)
				{
					memcpy(&pobTran->srBRec.szCardHolder[j], &pobTran->szTrack1[i ++], 1);
					if (pobTran->szTrack1[i] == 0x5E)
						break;
				}

				break;
			}
		}

		inDISP_LogPrintfWithFlag("  Track 1 data[%s]", pobTran->szTrack1);
		inDISP_LogPrintfWithFlag("  Card Holder name[%s]", pobTran->srBRec.szCardHolder);
	}

	/* Track2 */
	if (uszReadBuffer[inCount] == 0x00) /* Track 2 Length */
	{
		/* 因為沒有【Track 2】，因該會給【Tag 57】 */
		memset(pobTran->szTrack2, 0x00, sizeof(pobTran->szTrack2));
		inDISP_LogPrintfWithFlag("  T2_Length[0][inCount = %d]", inCount);
		inCount ++ ;
	}
	else
	{
		/* ‧ Track 2 present – X (1)	.
  		   . Followed by length and Track 2
  		   . Length of track 2 – X (1)
		   . Track 2 data – ASCII (Variable up to 38)!!注意跟visawave用BCD不同, wave的規則如下. Track 2 data – BCD (Variable up to 19)
		   . Track 2 data can be read directly from chip Track 2 equivalent data, tag 57 */
		inTrack2Size = uszReadBuffer[inCount];
		inDISP_LogPrintfWithFlag("  T2_Length [0x%X][%d]-[inCount = %d]", uszReadBuffer[inCount], inTrack2Size, inCount);
		inCount ++;
		
		memset(pobTran->szTrack2, 0x00, sizeof(pobTran->szTrack2));
		
		memcpy(&pobTran->szTrack2[0], &uszReadBuffer[inCount], inTrack2Size);
		inCount += inTrack2Size;
		
		pobTran->shTrack2Len = inTrack2Size;
		
//		pobTran->srBRec.fT2Data = VS_TRUE;

		for (i = 0; i < strlen(pobTran->szTrack2); i ++)
		{
			/* 【0X3D】【=】 */
			if (pobTran->szTrack2[i] != 0x3D)
				memcpy(&pobTran->srBRec.szPAN[i], &pobTran->szTrack2[i], 1);
			else if (pobTran->szTrack2[i] == 0x3D)
			{
				i ++;
				memcpy(&pobTran->srBRec.szExpDate[0], &pobTran->szTrack2[i], 4);
				i += 4;
				memcpy(&pobTran->srBRec.szServiceCode[0], &pobTran->szTrack2[i], 3);
				break;
			}
		}

		for (i = 0; i < strlen(pobTran->szTrack2); i ++)
		{
			if (pobTran->szTrack2[i] == 'F')
			{
				pobTran->szTrack2[i] = 0;
				break;
			}

			if (pobTran->szTrack2[i] == 'D')
				pobTran->szTrack2[i] = '=';
		}

		inDISP_LogPrintfWithFlag("  Track 2 data(%s)", pobTran->szTrack2);
		inDISP_LogPrintfWithFlag("  Card Number(%s)", pobTran->srBRec.szPAN);
		inDISP_LogPrintfWithFlag("  Card Expire Date(%s)", pobTran->srBRec.szExpDate);
		inDISP_LogPrintfWithFlag("  Service Code(%s)", pobTran->srBRec.szServiceCode);
		//inDISP_LogPrintfWithFlag("  usz57_Track2 (%s)", pobTran->usz57_Track2);
	}

	/* 判斷是否有晶片類欄位, 有則解析 */
	/* DE055(Clearing Record) Present */
	/* If a Clearing Record (DE 055) field is available,
	then this field will be 01h. If there is no Clearing
	Record (DE 055) field, then this field will be 00h. */
	/* TLV DE 055(Clearing Record) */
	/* DE 055 data (if available) as a TLV data object
	encoded with Tag ‘E1’. The DE 055 data is the
	same data as is included in the Clearing Record.
	Details given in next Table.
	Tag: E1 Format: b1..126 variable. */
	
  	if (uszReadBuffer[inCount] == 0x01 && uszReadBuffer[inCount + 1] == 0xE1) /* &調整&& */
  	{
		inCount += 2;
		inDISP_LogPrintfWithFlag("  Get Tag Len[%x] Len1[%x] line[%d]", uszReadBuffer[inCount], uszReadBuffer[inCount+1], __LINE__);
		if (uszReadBuffer[inCount] == 0x82)
		{
			inCount ++;
			inReadDataSize = uszReadBuffer[inCount] * 256 + uszReadBuffer[inCount + 1];
			inCount += 2;
		}
		else if (uszReadBuffer[inCount] == 0x81)
		{
			inCount ++;
			inReadDataSize = uszReadBuffer[inCount];
			inCount ++;
		}
		else if (uszReadBuffer[inCount] == 0x67)/*20151014浩瑋新增*/
		{
			/* 【需求單 - 104094】銀聯閃付需求 by Wei Hsiu - 2015/8/13 下午 10:12:06 */
			/* QuickPass - 沒有帶後面Tag總長度 */
			inCount ++;
			inReadDataSize = 0;

			//pobTran->fCTLS_QuickPass = VS_FALSE;
			pobTran->uszQuickPassTag99 = VS_FALSE;
			pobTran->srBRec.uszCUPTransBit = VS_TRUE;
		}
		else
		{
			inReadDataSize = uszReadBuffer[inCount];
			inCount ++;
		}
	
		/* 判斷是否為 PayPass 資料格式，0xDF8129 Outcome Parameter Set 固定【8 BYTE】
		   20131119-Technical_Specifications_PP_MC_Reader_Spec_v3_0_1.pdf / A.1.110 Outcome Parameter Set */
		if (uszReadBuffer[inCount] == 0xDF && uszReadBuffer[inCount + 1] == 0x81 && uszReadBuffer[inCount + 2] == 0x29)
		{
			fPayPassFormat = VS_TRUE;
			inCount += 3;
			/* Data Length */
			inCount ++;
			/* BYTE1 Status
				0001: APPROVED
				0010: DECLINED
				0011: ONLINE REQUEST
				0100: END APPLICATION
				0101: SELECT NEXT
				0110: TRY ANOTHER INTERFACE
				0111: TRY AGAIN
				1111: N/A
				Other values: RFU
			*/
			switch (uszReadBuffer[inCount])
			{
				case 0x10 :
					inDISP_LogPrintfWithFlag("0xDF8129_Status[%02X]_APPROVED", uszReadBuffer[inCount]);
					break;
				case 0x20 :
					inDISP_LogPrintfWithFlag("0xDF8129_Status[%02X]_DECLINED", uszReadBuffer[inCount]);
					break;
				case 0x30 :
					inDISP_LogPrintfWithFlag("0xDF8129_Status[%02X]_ONLINE_REQUEST", uszReadBuffer[inCount]);
					break;
				case 0x40 :
					inDISP_LogPrintfWithFlag("0xDF8129_Status[%02X]_END_APPLICATION", uszReadBuffer[inCount]);
					break;
				case 0x50 :
					inDISP_LogPrintfWithFlag("0xDF8129_Status[%02X]_SELECT_NEXT", uszReadBuffer[inCount]);
					break;
				case 0x60 :
					inDISP_LogPrintfWithFlag("0xDF8129_Status[%02X]_TRY_ANOTHER_INTERFACE", uszReadBuffer[inCount]);
					break;
				case 0x70 :
					inDISP_LogPrintfWithFlag("0xDF8129_Status[%02X]_TRY_AGAIN", uszReadBuffer[inCount]);
					break;
				default :
					inDISP_LogPrintfWithFlag("0xDF8129_Status[%02X]", uszReadBuffer[inCount]);
					return (VS_ERROR);
			}

			inCount ++;
			/* Start: 0xF0- NOT APPLICABLE. */
			inCount ++;
			/* Online Response: 0xF0- NOT APPLICABLE. */
			inCount ++;
			/* CVM: 0x10- SIGNARURE. */
			inCount ++;
			/* UI and DATA: 0x28- Contain following Information:
			- DATA RECORD PRESENT.
			- RECEIPT. */
			inCount ++;
			/* Alternate Interface Preference: 0xF0- NOT APPLICABLE. */
			inCount ++;
			/* Field off Request: 0xFF */
			inCount ++;
			/* Removal Timeout: 0x00 */
			inCount ++;

			if (uszReadBuffer[inCount] == 0xFF && uszReadBuffer[inCount + 1] == 0x81 && uszReadBuffer[inCount + 2] == 0x05)
			{
				inCount += 3;
				if (uszReadBuffer[inCount] == 0x82)
				{
					inCount ++;
					inReadDataSize = uszReadBuffer[inCount] * 256 + uszReadBuffer[inCount + 1];
					inCount += 2;
				}
				else if (uszReadBuffer[inCount] == 0x81)
				{
					inCount ++;
					inReadDataSize = uszReadBuffer[inCount];
					inCount ++;
				}
				else
				{
					inReadDataSize = uszReadBuffer[inCount];
					inCount ++;
				}
			}

			inDISP_LogPrintfWithFlag("[0xFF 0x81 0x05][%d]-[inCount = %d]", inReadDataSize, inCount);
			inMultiFunc_AnalysisEmvData(pobTran, (unsigned char*)&uszReadBuffer[inCount], inReadDataSize);
			/* 0xFF8106 Discretionary Data
			   Discretionary Data description for PayPass transaction */
			inCount += inReadDataSize;

			if (uszReadBuffer[inCount] == 0xFF && uszReadBuffer[inCount + 1] == 0x81 && uszReadBuffer[inCount + 2] == 0x06)
			{
				inCTLS_VIVO_PayPassDiscretionaryData(pobTran, (unsigned char*)&uszReadBuffer[inCount], (inTotalLEN - inCount - 2)); /* 【-2】是【CRC】 */
			}
		}

		if (fPayPassFormat == VS_FALSE)
		{
			inDISP_LogPrintfWithFlag("  PayPassFmat InLen[%d] inBuf[%x] Count[%d] Line[%d]",  (inTotalLEN - inCount - 2), uszReadBuffer[inCount], inCount,  __LINE__);
			inMultiFunc_AnalysisEmvData(pobTran, (unsigned char*)&uszReadBuffer[inCount], (inTotalLEN - inCount - 2));
		}
  	}
	else
	{
		inDISP_LogPrintfWithFlag("inMultiFunc_CALL_Slave_CTLS_Result()_FORMAT_ERROR");
  		return (VS_ERROR);
	}

  	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
  	return (VS_SUCCESS);
}


/*
 * Function		: inMultiFunc_CallSlave
 * Date&Time	: 2022/11/17 下午 2:08
 * Describe		: 
 * 啟動外接設備的函式，針對傳進來的 inTranCode ，進行參數的設定。
  * [Verifone 系列使用的方法]
 */
int inMultiFunc_CallSlave(TRANSACTION_OBJECT *pobTran, int inTranCode, MULTI_TABLE * srMultiTable)
{
        char	szCTLSMode[1 + 1];
	int     inIndex = 0;
	int     inRetVal, inTimeout;
        
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);	
	
        memset(szCTLSMode, 0x00, sizeof(szCTLSMode));
	inGetContactlessReaderMode(szCTLSMode);
        
        if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)))
        {
                inRetVal = inMultiFunc_JsonCallSlave(pobTran, inTranCode, srMultiTable);
                return inRetVal;
        }
        
	/* 判斷交易類別是否要進行多段傳送  */
	if (inTranCode != _MULTI_TMS_CAPK_NO_ &&
		inTranCode != _MULTI_TMS_MASTER_NO_ &&
		inTranCode != _MULTI_TMS_VISA_NO_ &&
		inTranCode != _MULTI_TMS_JCB_NO_ &&
		inTranCode != _MULTI_TMS_CUP_NO_)

	{
		memset(srMultiTable->stMulti_TransData.szTransType, 0x00, sizeof(srMultiTable->stMulti_TransData.szTransType));
		memset(srMultiTable->stMulti_TransData.szTranHost, 0x00, sizeof(srMultiTable->stMulti_TransData.szTranHost));
		
		srMultiTable->stMulti_TransData.fMultiSend = VS_FALSE;
		srMultiTable->stMulti_TransData.inTotalPacketNum = 1;
		srMultiTable->stMulti_TransData.inSubPacketNum = 1;

	}

	/* 領頭的銀行別 */
	memcpy(&srMultiTable->stMulti_TransData.szTranHost[0], "NCCC", 4);

	switch(inTranCode)
	{
		case _MULTI_POLL_NO_ :
			memcpy(&srMultiTable->stMulti_TransData.szTransType[0], _MULTI_POLL_, 2);
			srMultiTable->stMulti_TransData.lnTotalPacketSize = _MULTI_SUB_SIZE_NONE_;/* Sub Size 0 */
			inIndex = 0;
			inTimeout = 100;
			break;
		case _MULTI_PIN_NO_ :
			memcpy(&srMultiTable->stMulti_TransData.szTransType[0], _MULTI_PIN_, 2);
			srMultiTable->stMulti_TransData.lnTotalPacketSize = _MULTI_SUB_SIZE_SMALL_;/* Sub Size 100 */
			inIndex = 1;
			inTimeout = 200;
			break;
		case _MULTI_CTLS_NO_ :
			memcpy(&srMultiTable->stMulti_TransData.szTransType[0], _MULTI_CTLS_, 2);
			srMultiTable->stMulti_TransData.lnTotalPacketSize = _MULTI_SUB_SIZE_MIDDLE_; /* Sub Size 200 */
			srMultiTable->stMulti_TransData.inCTLS_Timeout = 30;
			inIndex = 2;
			inTimeout = 200;
			break;
		case _MULTI_SIGNPAD_NO_ :
			memcpy(&srMultiTable->stMulti_TransData.szTransType[0], _MULTI_SIGNPAD_, 2);
			srMultiTable->stMulti_TransData.lnTotalPacketSize = _MULTI_SUB_SIZE_SMALL_;/* Sub Size 100 */
			inIndex = 3;
			inTimeout = 200;
			break;
		case _MULTI_SLAVE_REBOOT_NO_ :
			memcpy(&srMultiTable->stMulti_TransData.szTransType[0], _MULTI_SLAVE_REBOOT_, 2);
			srMultiTable->stMulti_TransData.lnTotalPacketSize = _MULTI_SUB_SIZE_NONE_;/* Sub Size 0 */
			inIndex = 4;
			inTimeout = 100;
			break;
		case _MULTI_EXCHANGE_NO_ :
			memcpy(&srMultiTable->stMulti_TransData.szTransType[0], _MULTI_EXCHANGE_, 2);
			srMultiTable->stMulti_TransData.lnTotalPacketSize = _MULTI_SUB_SIZE_NONE_;/* Sub Size 0 */
			inIndex = 5;
			inTimeout = 100;
			inDISP_ClearAll();
			inDISP_ChineseFont("外接裝置連接中...", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
			break;
		case _MULTI_NOSIGN_NO_ :
			memcpy(&srMultiTable->stMulti_TransData.szTransType[0], _MULTI_NOSIGN_, 2);
			srMultiTable->stMulti_TransData.lnTotalPacketSize = _MULTI_SUB_SIZE_SMALL_;/* Sub Size 100 */
			inIndex = 6;
			inTimeout = 200;
			break;
		case _MULTI_TMS_CAPK_NO_ :
			memcpy(&srMultiTable->stMulti_TransData.szTransType[0], _MULTI_TMS_CAPK_, 2);
			
//			inIndex = 7;
			inTimeout = 1000;
			break;
		case _MULTI_TMS_MASTER_NO_ :
			memcpy(&srMultiTable->stMulti_TransData.szTransType[0], _MULTI_TMS_MASTER_, 2);
			inIndex = 8;
			inTimeout = 1000;
			break;
		case _MULTI_TMS_VISA_NO_ :
			memcpy(&srMultiTable->stMulti_TransData.szTransType[0], _MULTI_TMS_VISA_, 2);
			inIndex = 9;
			inTimeout = 1000;
			break;
		case _MULTI_TMS_JCB_NO_ :
			memcpy(&srMultiTable->stMulti_TransData.szTransType[0], _MULTI_TMS_JCB_, 2);
			inIndex = 10;
			inTimeout = 1000;
			break;
		case _MULTI_TMS_CUP_NO_ :
			/*銀聯閃付需求 */
			memcpy(&srMultiTable->stMulti_TransData.szTransType[0], _MULTI_TMS_CUP_, 2);
			inIndex = 10;
			inTimeout = 1000;
			break;
		case _MULTI_SIGNPAD_API_NO_ :
			inIndex = 12;
			inTimeout = 200;
			break;
		case _MULTI_SIGN_CONFIRM_NO_ :
			/* 優化整合型周邊設備交易時間  */
			memcpy(&srMultiTable->stMulti_TransData.szTransType[0], _MULTI_SIGN_CONFIRM_, 2);
			srMultiTable->stMulti_TransData.lnTotalPacketSize = _MULTI_SUB_SIZE_SMALL_;/* Sub Size 100 */
			inIndex = 0;
			inTimeout = 0;
			break;
		default :
			return (VS_ERROR);
	}
	inDISP_LogPrintfWithFlag("  Call Slave TotalSz[%ld] Type[%s] Line[%d]",srMultiTable->stMulti_TransData.lnTotalPacketSize, 
												srMultiTable->stMulti_TransData.szTransType, __LINE__);	
	
	srMultiTable->stMulti_Optional_Setting.uszWaitForAck = VS_FALSE;
	/* Host Send */
	inRetVal = srMultiHostTransTb[inIndex].inMultiSend(pobTran, srMultiTable);
	
	/* modify by green 151021 優化 */
	if (inTimeout > 0)
		inDISP_Wait(inTimeout);
        
        if (inRetVal == VS_SUCCESS)
	{
		/* 傳送成功就認為必需回傳資料 */
		srMultiTable->stMulti_TransData.unzAlreadySend = VS_TRUE;
	}

	switch(inTranCode)
	{
		case _MULTI_POLL_NO_ :
			if (inRetVal == VS_SUCCESS)
			{
				/* POLL成功當作感應功能ok */
				gfCTLS_Bit = VS_TRUE;
			}
			//else if (inRetVal == ERR_COM_NAK)
			else if (inRetVal == VS_ERROR || VS_TIMEOUT)
			{
				/* 表示POLL失敗 */
				inDISP_Clear_Area(1, _LINE_8_4_, 16, _LINE_8_8_, _ENGLISH_FONT_8X16_);
				inDISP_ChineseFont("CTLS_SN_ERR", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_BEEP(2,0);
			}
			
			inDISP_Wait(500);

			return (inRetVal);
		case _MULTI_PIN_NO_ :
			srMultiTable->stMulti_TransData.inCTLS_Timeout = 35;
			break;
		case _MULTI_SIGN_CONFIRM_NO_ :
			/* 優化整合型周邊設備交易時間  */
			if (inRetVal != VS_SUCCESS)
				return (inRetVal);

			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE || 
				pobTran->srBRec.uszInstallmentBit == VS_TRUE ||
				pobTran->srBRec.uszRedeemBit == VS_TRUE)
			{
				inDISP_Clear_Area(1, _LINE_8_4_, 16, _LINE_8_8_, _ENGLISH_FONT_8X16_);
				inDISP_ChineseFont("　　交易完成", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont(" 請持卡人確認", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);

			}

			srMultiTable->stMulti_TransData.inCTLS_Timeout = 35;
			break;
		case _MULTI_SIGNPAD_NO_ :
			srMultiTable->stMulti_TransData.inCTLS_Timeout = 95;
			break;
		case _MULTI_EXCHANGE_NO_ :
			srMultiTable->stMulti_TransData.inCTLS_Timeout = 6;
			break;
		case _MULTI_TMS_CAPK_NO_ :
		case _MULTI_TMS_MASTER_NO_ :
		case _MULTI_TMS_VISA_NO_ :
		case _MULTI_TMS_JCB_NO_ :
		case _MULTI_TMS_CUP_NO_ :  /* 銀聯閃付需求  */
			srMultiTable->stMulti_TransData.inCTLS_Timeout = 5;
			break;
		case _MULTI_CTLS_NO_ :
		case _MULTI_SLAVE_REBOOT_NO_ :
		case _MULTI_NOSIGN_NO_ :
			return (VS_SUCCESS);
		 case _MULTI_SIGNPAD_API_NO_ :
			return (inRetVal);
		default :
			return (VS_ERROR);
	}

	/* Host Recv */
	/* 如果要立既等接收，就在這邊進行 2022/11/22 [SAM] */
	inRetVal = srMultiHostTransTb[inIndex].inMultiRece(pobTran, srMultiTable);

	inDISP_Wait(500);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
	return (inRetVal);
}

int inMultiFunc_Host_USB_Initial(void)
{
        int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CUP_LOGON_;
	char		szKey = 0x00;
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = 0x00;
        unsigned int    uiVIDPID = 0;
        unsigned short  usVID = 0;
        unsigned short  usPID = 0;

        /* 開port */
	/* Portable 機型若沒接上底座再開Ethernet會失敗 */
	if (inFunc_Is_Portable_Type() == VS_TRUE)
	{
		do
		{
			if(inUSB_SelectMode(_USB_MODE_HOST_) != d_OK)
                        {
                                if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintfAt(AT,"inUSB_SelectMode Error");
				}
                                return (VS_ERROR);
                        }

                        usRetVal = OpenUSBPort(USB_Host);
                        if(usRetVal >= 0)
                        {
                                if (ginDebug == VS_TRUE)
                                    inDISP_LogPrintfAt(AT,"OpenUSBPort Success");
                        }
                        else
                        {
                                if (ginDebug == VS_TRUE)
                                    inDISP_LogPrintfAt(AT,"OpenUSBPort Error");
                                return VS_ERROR;
                        }
        
			if (usRetVal == VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintfAt(AT,"inUSB_Open OK");
				}
				break;
				
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "inUSB_Open Error: 0x%04x", usRetVal);
					inDISP_LogPrintf(AT,szDebugMsg);
				}
				
				/* 未接上底座，提示接上底座後並按確認 */
				if (inFunc_Is_Cradle_Attached() != VS_SUCCESS)
				{
					/* 請插回底座 繼續交易 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_CHECK_CRADLE_ATTATCHED_, 0, _COORDINATE_Y_LINE_8_4_);
					inDISP_BEEP(1, 0);
					
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
					
					while (1)
					{
						inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
						szKey = uszKBD_Key();

						/* Timeout */
						if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
						{
							szKey = _KEY_TIMEOUT_;
							inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
						}

						if (szKey == _KEY_ENTER_		||
						    szKey == _KEY_TIMEOUT_		||
						    inChoice == _CUPLogOn_Touch_KEY_2_)
						{	
							inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
							break;
						}
//						else if (szKey == _KEY_CANCEL_)
//						{
//							inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
//							
//							return (VS_ERROR);
//						}
						else if (szKey == _KEY_0_)
						{
							/* 壓住0後3秒內按clear */
							inDISP_Timer_Start(_TIMER_NEXSYS_4_, 3);
							do
							{
								szKey = uszKBD_Key_In();
							}while (szKey == 0	&&
								inTimerGet(_TIMER_NEXSYS_4_) != VS_SUCCESS);
							
							/* 按clear */
							if (szKey == _KEY_CLEAR_)
							{
								return (VS_SUCCESS);
							}
						}
						else
						{

						}
						
					}/* 重新初始化迴圈 */
					/* 清空Touch資料 */
					inDisTouch_Flush_TouchFile();
					
				}
				/* 若接上底座還是錯誤，就回傳錯誤 */
				else
				{
					gstMultiOb.srSetting.uszSettingOK = VS_FAILURE;
                                        return (VS_ERROR);
				}
				
			}
			
		}
		while (1);
				
	}
	else/* CounterTop 機型 */
	{
		if(inUSB_SelectMode(_USB_MODE_HOST_) != d_OK)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintfAt(AT,"inUSB_SelectMode Error");
                        }
                        return (VS_ERROR);
                }

                usRetVal = OpenUSBPort(USB_Host);

		if (usRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "inUSB_Open Error: 0x%04x", usRetVal);
				inDISP_LogPrintfAt(AT,szDebugMsg);
			}
                        gstMultiOb.srSetting.uszSettingOK = VS_FAILURE;
			return (VS_ERROR);         
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintfAt(AT,"inUSB_Open OK");
			}
		}
		
	}
        
        gstMultiOb.srSetting.uszSettingOK = VS_TRUE;

        return (VS_SUCCESS);
}

int inMultiFunc_Host_Data_Receive_Check(unsigned char uszComPort, unsigned short *usReceiveLen)
{
        int     inRetVal = 0;
	char	szComPort[4 + 1];
        char	szDebugMsg[100 + 1];

	memset(szComPort, 0x00, sizeof(szComPort));
	inGetMultiComPort1(szComPort);
	if (memcmp(szComPort, "COM1", strlen("COM1")) == 0	||
	    memcmp(szComPort, "COM2", strlen("COM2")) == 0	||
	    memcmp(szComPort, "COM3", strlen("COM3")) == 0	||
	    memcmp(szComPort, "COM4", strlen("COM4")) == 0)
	{
		inRetVal = inRS232_Data_Receive_Check(uszComPort, usReceiveLen);
	}
	else if (memcmp(szComPort, "USB1", strlen("USB1")) == 0)
	{
                inRetVal = VS_SUCCESS;
	}
	else if (memcmp(szComPort, "WIFI", strlen("WIFI")) == 0)
	{
                inDISP_LogPrintfWithFlag("WIFI");
	}
	else
	{
                inDISP_LogPrintfWithFlag("else");
                inRetVal = VS_ERROR;
	}
        
        return (inRetVal);
}

int inMultiFunc_Host_Data_Receive(unsigned char uszComPort, unsigned char *uszReceBuff, unsigned short *usReceSize)
{
        int     inRetVal = 0;
        int     inTransTimeout = 5000;
	char	szComPort[4 + 1];
        char	szDebugMsg[100 + 1];

	memset(szComPort, 0x00, sizeof(szComPort));
	inGetMultiComPort1(szComPort);
	if (memcmp(szComPort, "COM1", strlen("COM1")) == 0	||
	    memcmp(szComPort, "COM2", strlen("COM2")) == 0	||
	    memcmp(szComPort, "COM3", strlen("COM3")) == 0	||
	    memcmp(szComPort, "COM4", strlen("COM4")) == 0)
	{
		inRetVal = inRS232_Data_Receive(uszComPort, uszReceBuff, usReceSize);
	}
	else if (memcmp(szComPort, "USB1", strlen("USB1")) == 0)
	{
                inRetVal = inUSB_Host_Read(uszReceBuff, usReceSize);
                
	}
	else if (memcmp(szComPort, "WIFI", strlen("WIFI")) == 0)
	{
                inDISP_LogPrintfWithFlag("WIFI");
	}
	else
	{
                inDISP_LogPrintfWithFlag("else");
                inRetVal = VS_ERROR;
	}

        return (inRetVal);
}

int inMultiFunc_Host_Data_Send_Check(unsigned char uszComPort)
{
        int     inRetVal = 0;
	char	szComPort[4 + 1];
        char	szDebugMsg[100 + 1];
	
	memset(szComPort, 0x00, sizeof(szComPort));
	inGetMultiComPort1(szComPort);
	if (memcmp(szComPort, "COM1", strlen("COM1")) == 0	||
	    memcmp(szComPort, "COM2", strlen("COM2")) == 0	||
	    memcmp(szComPort, "COM3", strlen("COM3")) == 0	||
	    memcmp(szComPort, "COM4", strlen("COM4")) == 0)
	{
		inRetVal = inRS232_Data_Send_Check(uszComPort);
	}
	else if (memcmp(szComPort, "USB1", strlen("USB1")) == 0)
	{
                inDISP_LogPrintfWithFlag("USB Host No Need Send Check");
                inRetVal = VS_SUCCESS;
	}
	else if (memcmp(szComPort, "WIFI", strlen("WIFI")) == 0)
	{
                inDISP_LogPrintfWithFlag("WIFI");
	}
	else
	{
                inDISP_LogPrintfWithFlag("else");
                inRetVal = VS_ERROR;
	}

        return (inRetVal);
}

int inMultiFunc_Host_Data_Send(unsigned char uszComPort, unsigned char *uszSendBuff, unsigned short usSendSize)
{
        int     inRetVal = 0;
	char	szComPort[4 + 1];
        char	szDebugMsg[100 + 1];

	memset(szComPort, 0x00, sizeof(szComPort));
	inGetMultiComPort1(szComPort);
	if (memcmp(szComPort, "COM1", strlen("COM1")) == 0	||
	    memcmp(szComPort, "COM2", strlen("COM2")) == 0	||
	    memcmp(szComPort, "COM3", strlen("COM3")) == 0	||
	    memcmp(szComPort, "COM4", strlen("COM4")) == 0)
	{
		inRetVal = inMultiFunc_RS232_Data_Send(uszComPort, uszSendBuff, usSendSize);
	}
	else if (memcmp(szComPort, "USB1", strlen("USB1")) == 0)
	{
                inRetVal = inUSB_Host_Send(uszSendBuff, usSendSize);
	}
	else if (memcmp(szComPort, "WIFI", strlen("WIFI")) == 0)
	{
                if (ginDebug == VS_TRUE)
                    inDISP_LogPrintf("WIFI");
	}
	else
	{
		if (ginDebug == VS_TRUE)
                    inDISP_LogPrintf("else");
                inRetVal = VS_ERROR;
	}

        return (inRetVal);
}

int inMultiFunc_Host_Receive_ACKandNAK(MULTI_TABLE *stMultiOb)
{
	int		inRetVal;
	char		szDebugMsg[100 + 1];
        char            szComPort[4 + 1];
	unsigned char	uszBuf[_ECR_RS232_BUFF_SIZE_];
	unsigned short	usTwoSize = 2;
	unsigned short	usReceiveLen = 0;

        memset(szComPort, 0x00, sizeof(szComPort));
	inGetMultiComPort1(szComPort);

	/* 設定Timeout */
	if (stMultiOb->stMulti_Optional_Setting.uszWaitForAck == VS_TRUE)
	{
		inRetVal = inDISP_Timer_Start(_TIMER_NEXSYS_2_, stMultiOb->stMulti_Optional_Setting.inACKTimeOut);
	}

	while (1)
	{
		memset(uszBuf, 0x00, sizeof(uszBuf));
		/* 當Comport中有東西就開始分析 */
		/* 檢查 Comport有東西，註RS232才會有，USB Host不會有 */
		while (inMultiFunc_Host_Data_Receive_Check(stMultiOb->srSetting.uszComPort, usReceiveLen) != VS_SUCCESS)
		{
			/* Timeout */
                        if (stMultiOb->stMulti_Optional_Setting.uszWaitForAck == VS_TRUE)
			{
                                if (inTimerGet(_TIMER_NEXSYS_2_) == 0)
                                {
                                        inDISP_LogPrintfWithFlag("  inMultiHost Wait ACK or Nak TimeOut *Error* Line[%d]", __LINE__);
                                        return (VS_TIMEOUT);
                                }
                        }
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Len : %d", usReceiveLen);
			inDISP_LogPrintf(szDebugMsg);
                        inDISP_LogPrintf("Comport = %s", szComPort);
		}

		while (usReceiveLen > 0 || !memcmp(szComPort, "USB1", 4))
		{
			if (stMultiOb->stMulti_Optional_Setting.uszWaitForAck == VS_TRUE)
			{
				/* 如果timeout就跳出去 */
				if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
				{
					return (VS_TIMEOUT);
				}
			}

			/* 這邊一次只收兩個byte */
			usTwoSize = 2;
			inRetVal = inMultiFunc_Host_Data_Receive(stMultiOb->srSetting.uszComPort, uszBuf, usTwoSize);

			if (inRetVal == VS_SUCCESS)
			{
				/* buffer讀出兩個byte，長度減二 */
				usReceiveLen -= 2;

				/* 判斷收到資料是否為ACK */
				if (uszBuf[0] == _ACK_ && uszBuf[1] == _ACK_)
				{
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("Receive ACK!");
					}

					return (_ACK_);
				}
				/* 判斷收到資料是否為NAK */
				else if (uszBuf[0] == _NAK_ && uszBuf[1] == _NAK_)
				{
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("Receive NAK!");
					}

					return (_NAK_);
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("Receive Not Ack Neither NAK!");
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						inFunc_BCD_to_ASCII(szDebugMsg, uszBuf, usTwoSize);
						inDISP_LogPrintf(szDebugMsg);
					}

					continue;
				}

			}/* inMultiFunc_Host_Data_Receive */

		}/* while (usReceiveLen > 0) (有收到資料) */

	}/* while(1)...... */

}

int inMultiFunc_Host_Send_ACKorNAK(MULTI_TABLE *stMultiOb, int inResponse)
{
	int		inRetVal;
	unsigned char	uszSendBuffer[2 + 1];

	memset(uszSendBuffer, 0x00, sizeof(uszSendBuffer));

	/* 檢查port是否已經準備好要送資料 */
        while (inMultiFunc_Host_Data_Send_Check(stMultiOb->srSetting.uszComPort) != VS_SUCCESS)
	{
		/* 等TxReady*/
	};

	if (inResponse == _ACK_)
	{
		/* 成功，回傳ACK */
		uszSendBuffer[0] = _ACK_;
		uszSendBuffer[1] = _ACK_;

		inRetVal = inMultiFunc_Host_Data_Send(stMultiOb->srSetting.uszComPort, uszSendBuffer, 2);

		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("Send ACK ACK Not OK");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("Send ACK ACK Not OK", _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}

			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("ACK ACK");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("Send ACK ACK", _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}

		}

	}
	else if (inResponse == _NAK_)
	{
		/* 失敗，回傳NAK */
		uszSendBuffer[0] = _NAK_;
		uszSendBuffer[1] = _NAK_;

		inRetVal = inMultiFunc_Host_Data_Send(stMultiOb->srSetting.uszComPort, uszSendBuffer, 2);

		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("Send NAK NAK Not OK");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("Send NAK NAK Not OK", _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}

			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("NAK_NAK");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("Send NAK NAK", _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}

		}

	}
	else if (inResponse == _FS_)
	{
		/* 失敗，回傳_FS_ */
		uszSendBuffer[0] = _FS_;
		uszSendBuffer[1] = _FS_;

		inRetVal = inMultiFunc_Host_Data_Send(stMultiOb->srSetting.uszComPort, uszSendBuffer, 2);

		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("Send FS FS Not OK");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("Send FS FS Not OK", _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}

			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("FS FS");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("Send FS FS", _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}

		}

	}
	else
	{
		/* 傳入錯誤的參數 */
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("Can't Send Neither Response");
		}
		if (ginDisplayDebug == VS_TRUE)
		{
			inDISP_LOGDisplay("Can't Send Response", _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
		}

		return (VS_ERROR);
	}

        return (VS_SUCCESS);
}

int inMultiFunc_Host_FlushRxBuffer(unsigned char uszComPort)
{    
        int     inRetVal = 0;
	char	szComPort[4 + 1];
        char	szDebugMsg[100 + 1];

        /* 沒設定完成，不用檢查 */
	if (gstMultiOb.srSetting.uszSettingOK != VS_TRUE)
	{
		inDISP_DispLogAndWriteFlie(" Setting Not OK [%u] Line[%d]", gstMultiOb.srSetting.uszSettingOK, __LINE__);
		return (VS_ERROR);
	}

	memset(szComPort, 0x00, sizeof(szComPort));
	inGetMultiComPort1(szComPort);
	if (memcmp(szComPort, "COM1", strlen("COM1")) == 0	||
	    memcmp(szComPort, "COM2", strlen("COM2")) == 0	||
	    memcmp(szComPort, "COM3", strlen("COM3")) == 0	||
	    memcmp(szComPort, "COM4", strlen("COM4")) == 0)
	{
		inRetVal = CTOS_RS232FlushRxBuffer(uszComPort);
	}
	else if (memcmp(szComPort, "USB1", strlen("USB1")) == 0)
	{
                inDISP_LogPrintfWithFlag("USB Host Can Not Flush Buffer");
                inRetVal = VS_SUCCESS;
	}
	else if (memcmp(szComPort, "WIFI", strlen("WIFI")) == 0)
	{
                inDISP_LogPrintfWithFlag("WIFI");
	}
	else
	{
                inDISP_LogPrintfWithFlag("else");
                inRetVal = VS_ERROR;
	}

	if (inRetVal != d_OK)
	{
		inDISP_DispLogAndWriteFlie(" MultiFunc FlashRxBuf *Error* RetVal[0x%04x] Line[%d]", inRetVal, __LINE__);
                inRetVal = VS_ERROR;
        }
	else
	{
		inDISP_LogPrintfWithFlag(" MultiFunc FlashRxBuf Successs Com[%d]", uszComPort + 1);
		inRetVal =  VS_SUCCESS;
	}

	return (inRetVal);
}

int inMultiFunc_Host_Send(TRANSACTION_OBJECT *pobTran, MULTI_TABLE * stMultiOb, char *szDataBuffer, int inDataSize)
{
	int		i;
	int		inRetVal;
	int		inRetry = 0;
	int		inSendLen = 0;
	char		szDebugMsg[100 +1];
	unsigned char	uszSendBuf[_ECR_RS232_BUFF_SIZE_];					/* 包含STX、ETX、LRC的電文 */
	unsigned char	uszLRC = 0;

	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Send %d", inDataSize);
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
	}

	/* Send之前清Buffer，避免收到錯的回應 */
        inMultiFunc_Host_FlushRxBuffer(stMultiOb->srSetting.uszComPort);
        
	/* 將Buffer初始化 */
	memset(uszSendBuf, 0x00, sizeof(uszSendBuf));

	if (stMultiOb->stMulti_Optional_Setting.uszPadStxEtx == VS_TRUE)
	{
		/* 在要傳送Buffer裡放STX */
		uszSendBuf[inSendLen] = _STX_;
		inSendLen ++;
		/* 在要傳送Buffer裡放要傳送的資料 */
		memcpy(&uszSendBuf[inSendLen], szDataBuffer, inDataSize);
		inSendLen += inDataSize;
		/* 在要傳送Buffer裡放ETX */
		uszSendBuf[inSendLen] = _ETX_;
		inSendLen ++;
	}
	else
	{
		/* 在要傳送Buffer裡放要傳送的資料 */
		memcpy(&uszSendBuf[inSendLen], szDataBuffer, inDataSize);
		inSendLen += inDataSize;
	}

	/* 運算LRC(STX Not include) */
	for (i = 1; i < (inSendLen); i++)
	{
		uszLRC ^= uszSendBuf[i];
	}

	/* 在要傳送Buffer裡放LRC */
	uszSendBuf[inSendLen] = uszLRC;
	inSendLen ++;

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "LRC : 0x%02X", uszLRC);
		inDISP_LogPrintf(szDebugMsg);
	}

	while (1)
	{
		/* 檢查port是否已經準備好要送資料 */
		while (inMultiFunc_Host_Data_Send_Check(stMultiOb->srSetting.uszComPort) != VS_SUCCESS);

		/* 經由port傳送資料 */
		inRetVal = inMultiFunc_Host_Data_Send(stMultiOb->srSetting.uszComPort, uszSendBuf, (unsigned short)inSendLen);

		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		else
		{

		/*---------------------------------- 印Debug 開始---------------------------------------------------------------*/
			/* 列印紙本電文和顯示電文訊息 */
			inECR_Print_Send_ISODeBug(szDataBuffer, inSendLen, inDataSize);
		/*---------------------------------- 印Debug 結束---------------------------------------------------------------*/
			/* 傳送Retry */
                        inDISP_LogPrintfWithFlag("inRetry = %d", inRetry);
                        inDISP_LogPrintfWithFlag("inMaxRetries = %d", stMultiOb->stMulti_Optional_Setting.inMaxRetries);
			if (inRetry < stMultiOb->stMulti_Optional_Setting.inMaxRetries)
			{
				/* 接收ACK OR NAK */
				inRetVal = inMultiFunc_Host_Receive_ACKandNAK(stMultiOb);

				/* 超過一秒沒收到回應 */
				if (inRetVal == VS_TIMEOUT)
				{
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("Not Receive Response, Retry");
					}
					inRetry++;
					continue;
				}
				/* 收到NAK */
				else if (inRetVal == _NAK_)
				{
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("Receive NAK, Retry");
					}
					inRetry++;
					continue;
				}
				/* 收到ACK */
				else
				{
					/* 成功 */
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("ECR ACK OK!");
					}
					return (VS_SUCCESS);
				}
			}
			/* 超過最大重試次數，仍要完成交易，收銀機提示補登畫面 */
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf("Exceed max retry times!");
				}
				return (VS_SUCCESS);
			}

		}/* inMultiFunc_Data_Send */

	}/* while(1) */

}


#endif