

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <ctosapi.h>
#include <sqlite3.h>

#include "../../INCLUDES/Define_1.h"
#include "../../INCLUDES/Define_2.h"
#include "../../INCLUDES/Transaction.h"
#include "../../INCLUDES/TransType.h"

#include "../../FUNCTION/Batch.h"
#include "../../FUNCTION/Sqlite.h"
#include "../../PRINT/Print.h"
#include "../../FUNCTION/HDPT.h"
#include "../../FUNCTION/HDT.h"
#include "../../FUNCTION/CDT.h"
#include "../../FUNCTION/Function.h"
#include "../../FUNCTION/Card.h"
#include "../../FUNCTION/Signpad.h"
#include "../../DISPLAY/Display.h"

#include "../../FUNCTION/FILE_FUNC/File.h"

#include "../ECR_FUNC/ECR.h"

#include "../../../JSON/cJSON.h"

#include "MultiFunc.h"
#include "ExDevicePackData.h"
#include "ExDeviceUnPackData.h"

STRUCT_JSON_EMV_DATA st_TempJasonEmvData[] =
{
	{0,"Tag57","",inJSON_EMV_SetTag57Data},	{0,"Tag5A","",inJSON_EMV_SetTag5AData},
	{0,"Tag5F20","",inJSON_EMV_SetTag5F20Data},	{0,"Tag5F24","",inJSON_EMV_SetTag5F24Data},
	{0,"Tag5F2A","",inJSON_EMV_SetTag5F2AData}	,{0,"Tag5F34","",inJSON_EMV_SetTag5F34Data},
	{0,"Tag82","",inJSON_EMV_SetTag82Data},	{0,"Tag84","",inJSON_EMV_SetTag84Data},	
	{0,"Tag95","",inJSON_EMV_SetTag95Data},	{0,"Tag9A","",inJSON_EMV_SetTag9AData},
	{0,"Tag9B","",inJSON_EMV_SetTag9BData},	{0,"Tag9C","",inJSON_EMV_SetTag9CData},
	{0,"Tag9F02","",inJSON_EMV_SetTag9F02Data},	{0,"Tag9F03","",inJSON_EMV_SetTag9F03Data},
	{0,"Tag9F08","",inJSON_EMV_SetTag9F08Data},	{0,"Tag9F09","",inJSON_EMV_SetTag9F09Data},
	{0,"Tag9F10","",inJSON_EMV_SetTag9F10Data},	{0,"Tag9F1A","",inJSON_EMV_SetTag9F1AData},
	{0,"Tag9F1E","",inJSON_EMV_SetTag9F1EData},	{0,"Tag9F26","",inJSON_EMV_SetTag9F26Data},
	{0,"Tag9F35","",inJSON_EMV_SetTag9F35Data},	{0,"Tag9F36","",inJSON_EMV_SetTag9F36Data},
	{0,"Tag9F37","",inJSON_EMV_SetTag9F37Data},	{0,"Tag9F36","",inJSON_EMV_SetTag9F41Data},
	{0,"Tag9F63","",inJSON_EMV_SetTag9F63Data},	{0,"Tag9F1F","",inJSON_EMV_SetTag9F1FData},
	{0,"ENDTAG","",NULL}
};


int inJSON_EMV_SetTag57Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->usz57_Track2) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->usz57_Track2, 0x00, sizeof(pobTran->usz57_Track2));
	memset(pobTran->szTrack2, 0x00, sizeof(pobTran->szTrack2));
	
	memcpy(&pobTran->szTrack2[0], szData, strlen(szData));
	pobTran->shTrack2Len = strlen(szData);
	
	inFunc_ASCII_to_BCD(&pobTran->usz57_Track2[0], szData, inDataLen);
	pobTran->in57_Track2Len = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag5AData(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz5A_ApplPan) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz5A_ApplPan, 0x00, sizeof(pobTran->srEMVRec.usz5A_ApplPan));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz5A_ApplPan[0], szData, inDataLen);
	pobTran->srEMVRec.in5A_ApplPanLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag5F20Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	if(strlen(szData) > (sizeof(pobTran->srEMVRec.usz5F20_CardholderName) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv Tag5F20 Len[%s] *Error* Line[%d]", strlen(szData), __LINE__);
		return VS_ERROR;
	}
	memset(pobTran->srEMVRec.usz5F20_CardholderName, 0x00, sizeof(pobTran->srEMVRec.usz5F20_CardholderName));
	memset(pobTran->szTrack1, 0x00, sizeof(pobTran->szTrack1));
	
	memcpy(&pobTran->szTrack1[0], szData, strlen(szData));
	pobTran->shTrack1Len = strlen(szData);
	
	memcpy(&pobTran->srEMVRec.usz5F20_CardholderName[0], szData, strlen(szData));
	pobTran->srEMVRec.in5F20_CardholderNameLen = strlen(szData);
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag5F24Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	if(strlen(szData) > (sizeof(pobTran->srEMVRec.usz5F24_ExpireDate) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv Tag5F24 Len[%s] *Error* Line[%d]", strlen(szData), __LINE__);
		return VS_ERROR;
	}
	memset(pobTran->srEMVRec.usz5F24_ExpireDate, 0x00, sizeof(pobTran->srEMVRec.usz5F24_ExpireDate));
	memcpy(&pobTran->srEMVRec.usz5F24_ExpireDate[0], szData, strlen(szData));
	pobTran->srEMVRec.in5F24_ExpireDateLen = strlen(szData) ;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag5F2AData(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz5F2A_TransCurrCode) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz5F2A_TransCurrCode, 0x00, sizeof(pobTran->srEMVRec.usz5F2A_TransCurrCode));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz5F2A_TransCurrCode[0], szData, inDataLen);
	pobTran->srEMVRec.in5F2A_TransCurrCodeLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag5F34Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz5F34_ApplPanSeqnum) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz5F34_ApplPanSeqnum, 0x00, sizeof(pobTran->srEMVRec.usz5F34_ApplPanSeqnum));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], szData, inDataLen);
	pobTran->srEMVRec.in5F34_ApplPanSeqnumLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag82Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz82_AIP) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz82_AIP, 0x00, sizeof(pobTran->srEMVRec.usz82_AIP));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz82_AIP[0], szData, inDataLen);
	pobTran->srEMVRec.in82_AIPLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag84Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz84_DF_NAME) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz84_DF_NAME, 0x00, sizeof(pobTran->srEMVRec.usz84_DF_NAME));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz84_DF_NAME[0], szData, inDataLen);
	pobTran->srEMVRec.in84_DFNameLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag8AData(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz8A_AuthRespCode) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz8A_AuthRespCode, 0x00, sizeof(pobTran->srEMVRec.usz8A_AuthRespCode));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz8A_AuthRespCode[0], szData, inDataLen);
	pobTran->srEMVRec.in8A_AuthRespCodeLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag95Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz95_TVR) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz95_TVR, 0x00, sizeof(pobTran->srEMVRec.usz95_TVR));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz95_TVR[0], szData, inDataLen);
	pobTran->srEMVRec.in95_TVRLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag9AData(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz9A_TranDate) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz9A_TranDate, 0x00, sizeof(pobTran->srEMVRec.usz9A_TranDate));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9A_TranDate[0], szData, inDataLen);
	pobTran->srEMVRec.in9A_TranDateLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag9BData(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz9B_TSI) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz9B_TSI, 0x00, sizeof(pobTran->srEMVRec.usz9B_TSI));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9B_TSI[0], szData, inDataLen);
	pobTran->srEMVRec.in9B_TSILen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag9CData(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz9C_TranType) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz9C_TranType, 0x00, sizeof(pobTran->srEMVRec.usz9C_TranType));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9C_TranType[0], szData, inDataLen);
	pobTran->srEMVRec.in9C_TranTypeLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag9F02Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz9F02_AmtAuthNum) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz9F02_AmtAuthNum, 0x00, sizeof(pobTran->srEMVRec.usz9F02_AmtAuthNum));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9F02_AmtAuthNum[0], szData, inDataLen);
	pobTran->srEMVRec.in9F02_AmtAuthNumLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag9F03Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz9F03_AmtOtherNum) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz9F03_AmtOtherNum, 0x00, sizeof(pobTran->srEMVRec.usz9F03_AmtOtherNum));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9F03_AmtOtherNum[0], szData, inDataLen);
	pobTran->srEMVRec.in9F03_AmtOtherNumLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag9F08Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz9F08_AppVerNumICC) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz9F08_AppVerNumICC, 0x00, sizeof(pobTran->srEMVRec.usz9F08_AppVerNumICC));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9F08_AppVerNumICC[0], szData, inDataLen);
	pobTran->srEMVRec.in9F08_AppVerNumICCLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag9F09Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz9F09_TermVerNum) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz9F09_TermVerNum, 0x00, sizeof(pobTran->srEMVRec.usz9F09_TermVerNum));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9F09_TermVerNum[0], szData, inDataLen);
	pobTran->srEMVRec.in9F09_TermVerNumLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag9F10Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz9F10_IssuerAppData) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz9F10_IssuerAppData, 0x00, sizeof(pobTran->srEMVRec.usz9F10_IssuerAppData));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9F10_IssuerAppData[0], szData, inDataLen);
	pobTran->srEMVRec.in9F10_IssuerAppDataLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag9F1AData(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz9F1A_TermCountryCode) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz9F1A_TermCountryCode, 0x00, sizeof(pobTran->srEMVRec.usz9F1A_TermCountryCode));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9F1A_TermCountryCode[0], szData, inDataLen);
	pobTran->srEMVRec.in9F1A_TermCountryCodeLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag9F1EData(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz9F1E_IFDNum) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz9F1E_IFDNum, 0x00, sizeof(pobTran->srEMVRec.usz9F1E_IFDNum));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9F1E_IFDNum[0], szData, inDataLen);
	pobTran->srEMVRec.in9F1E_IFDNumLen = inDataLen;
	
	return VS_SUCCESS;
}


int inJSON_EMV_SetTag9F26Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz9F26_ApplCryptogram) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz9F26_ApplCryptogram, 0x00, sizeof(pobTran->srEMVRec.usz9F26_ApplCryptogram));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9F26_ApplCryptogram[0], szData, inDataLen);
	pobTran->srEMVRec.in9F26_ApplCryptogramLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag9F27Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz9F27_CID) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz9F27_CID, 0x00, sizeof(pobTran->srEMVRec.usz9F27_CID));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9F27_CID[0], szData, inDataLen);
	pobTran->srEMVRec.in9F27_CIDLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag9F33Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz9F33_TermCapabilities) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz9F33_TermCapabilities, 0x00, sizeof(pobTran->srEMVRec.usz9F33_TermCapabilities));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9F33_TermCapabilities[0], szData, inDataLen);
	pobTran->srEMVRec.in9F33_TermCapabilitiesLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag9F34Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz9F34_CVM) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz9F34_CVM, 0x00, sizeof(pobTran->srEMVRec.usz9F34_CVM));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9F34_CVM[0], szData, inDataLen);
	pobTran->srEMVRec.in9F34_CVMLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag9F35Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz9F35_TermType) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz9F35_TermType, 0x00, sizeof(pobTran->srEMVRec.usz9F35_TermType));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9F35_TermType[0], szData, inDataLen);
	pobTran->srEMVRec.in9F35_TermTypeLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag9F36Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz9F36_ATC) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz9F36_ATC, 0x00, sizeof(pobTran->srEMVRec.usz9F36_ATC));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9F36_ATC[0], szData, inDataLen);
	pobTran->srEMVRec.in9F36_ATCLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag9F37Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz9F37_UnpredictNum) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz9F37_UnpredictNum, 0x00, sizeof(pobTran->srEMVRec.usz9F37_UnpredictNum));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9F37_UnpredictNum[0], szData, inDataLen);
	pobTran->srEMVRec.in9F37_UnpredictNumLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag9F41Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz9F41_TransSeqCounter) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz9F41_TransSeqCounter, 0x00, sizeof(pobTran->srEMVRec.usz9F41_TransSeqCounter));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], szData, inDataLen);
	pobTran->srEMVRec.in9F41_TransSeqCounterLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag9F63Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	inDataLen = (strlen(szData) / 2);
	
	if(inDataLen > (sizeof(pobTran->srEMVRec.usz9F63_CardProductLabelInformation) -1))
	{
		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
		return VS_ERROR;
	}
	
	memset(pobTran->srEMVRec.usz9F63_CardProductLabelInformation, 0x00, sizeof(pobTran->srEMVRec.usz9F63_CardProductLabelInformation));
	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9F63_CardProductLabelInformation[0], szData, inDataLen);
	pobTran->srEMVRec.in9F63_CardProductLabelInformationLen = inDataLen;
	
	return VS_SUCCESS;
}

int inJSON_EMV_SetTag9F1FData(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData)
{
	int inDataLen;
	inDataLen = (strlen(szData) / 2);
	
//	if(inDataLen > (sizeof(pobTran->srEMVRec.usz84_DF_NAME) -1))
//	{
//		inDISP_LogPrintfWithFlag(" Set Emv %s Len[%s] *Error* Line[%d]", szTag, inDataLen, __LINE__);
//		return VS_ERROR;
//	}
	
	inDISP_LogPrintfWithFlag(" 9f1f [%d]", inDataLen);
//	memset(pobTran->srEMVRec.usz9F63_CardProductLabelInformation, 0x00, sizeof(pobTran->srEMVRec.usz9F63_CardProductLabelInformation));
//	inFunc_ASCII_to_BCD(&pobTran->srEMVRec.usz9F63_CardProductLabelInformation[0], szData, inDataLen);
//	pobTran->srEMVRec.in9F63_CardProductLabelInformationLen = inDataLen;
	
	return VS_SUCCESS;
}


int inJSON_DebugUnPackDataTest(STRUCT_JASON_DATA *stUnpackData)
{	
//	int i;
	inDISP_LogPrintfWithFlag(" TagName [%s]", stUnpackData->bTagName);
	
//	for(i = 0 ; ; i++)
//	{
//		if(!memcmp(stUnpackData[i]->bTagName,"ENDTAG", 6))
//		{
//			break;
//		}else
//		{
//			inDISP_LogPrintfWithFlag(" TagName [%s]", stUnpackData[i].bTagName);
//			inDISP_LogPrintfWithFlag(" TagData [%s]", stUnpackData[i].bTagMessage);
//		}
//	}
//	
	
	return VS_SUCCESS;
}

//int inJSON_DebugUnPackData(STRUCT_JSON_EMV_DATA *stUnpakData)
int inJSON_DebugUnPackData()
{	
	int i;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START-----",__FILE__, __FUNCTION__, __LINE__);
	
	for(i = 0 ; ; i++)
	{
		if(!memcmp(st_TempJasonEmvData[i].bTagName,"ENDTAG", 6))
		{
			break;
		}else if(st_TempJasonEmvData[i].inSetValue == VS_TRUE)
		{
			inDISP_LogPrintfWithFlag(" TagName [%s]", st_TempJasonEmvData[i].bTagName);
			inDISP_LogPrintfWithFlag(" TagData [%s]", st_TempJasonEmvData[i].bTagMessage);
		}else{
			inDISP_LogPrintfWithFlag(" Tag[%s] is NULL", st_TempJasonEmvData[i].bTagName);
		}
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END-----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}

//int inJSON_AnylisysEmvDataIntoBrec(TRANSACTION_OBJECT *pobTran, char *szEmvTag, char* szEmvData)
//int inJSON_AnylisysEmvDataIntoBrec(TRANSACTION_OBJECT *pobTran, STRUCT_JSON_EMV_DATA *stUnpakData)
int inJSON_AnylisysEmvDataIntoBrec(TRANSACTION_OBJECT *pobTran)
{
	int i;
	int inRetVal;
	
	for(i = 0 ; ; i++)
	{
		/* 解到最後一個參數，就表示資料已都完成 */
		if(!memcmp(st_TempJasonEmvData[i].bTagName,"ENDTAG", 6))
		{
			return VS_SUCCESS;
		}
		
		if(st_TempJasonEmvData[i].inSetValue == VS_TRUE)
		{
			inRetVal = st_TempJasonEmvData[i].inSetFunctionPoint(pobTran, (char*)st_TempJasonEmvData[i].bTagName, (char*)st_TempJasonEmvData[i].bTagMessage);
		
			if(inRetVal == VS_ERROR)
			{
				inDISP_LogPrintfWithFlag(" Jaon Emv Set Tag[%s] Le[%d] *Error* Line[%d]",
					st_TempJasonEmvData[i].bTagName, strlen((char*)st_TempJasonEmvData[i].bTagMessage), __LINE__);
				return inRetVal;
			}
		}else{
			inDISP_LogPrintfWithFlag(" Jaon Emv Tag[%s] is NULL Line[%d]",
					st_TempJasonEmvData[i].bTagName, __LINE__);
		}

	}
	
	return VS_SUCCESS;
}

int inJSON_GetCjsonString(cJSON *Param, char *szResult)
{

	if( !Param->valuestring )
		return (0);

	strcpy((char *)szResult, Param->valuestring);

	return (strlen((char *)szResult));
}

int inJSON_CheckEmvTagAndUnPackData(TRANSACTION_OBJECT *pobTran, cJSON *root)
{
	cJSON	*cjEmvData;

	/* 先判斷Return Code (START) */
	cjEmvData = cJSON_GetObjectItem(root, "EMV");
	if( !cjEmvData )
	{
		inDISP_DispLogAndWriteFlie("  JsonUnpack NonEmv Data  *Warning* Line[%d]", __LINE__);
		return (VS_ERROR);
	}else{
		if(VS_ERROR == inJSON_UnPackEmvDataAfterParse(pobTran, cjEmvData))
		{
			inDISP_DispLogAndWriteFlie("  JsonUnpack EmvData *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		}
	
	}
	return VS_SUCCESS;
}

int inJSON_UnPackEmvDataAfterParse(TRANSACTION_OBJECT *pobTran, cJSON *root)
{
	cJSON	*cjEmvData = NULL;
	int i;
	
	for(i = 0 ; ; i++)
	{
		if(!memcmp(st_TempJasonEmvData[i].bTagName,"ENDTAG", 6))
		{
			break;
		}else
		{
			cjEmvData = cJSON_GetObjectItem(root, (char *)st_TempJasonEmvData[i].bTagName);
			
			if( !cjEmvData )
			{
				/* 這邊只記錄 Tag 沒找到，不中斷交易 */
				inDISP_DispLogAndWriteFlie("  JsonUnpack Tag[%s] Index NULL *Warning* Line[%d]",
								st_TempJasonEmvData[i].bTagName,  __LINE__);
			}else{
				/* 重新初始化變動參數 */
				memset(st_TempJasonEmvData[i].bTagMessage, 0x00, sizeof(st_TempJasonEmvData[i].bTagMessage));
				st_TempJasonEmvData[i].inSetValue = VS_FALSE;
				
				/* 目前 Json 回傳的都是字串， 所以用 strlen() 計算長度， 
				 *  如有其它數值，此條件需要修改
				 */
				if(strlen(cjEmvData->valuestring) < MAX_EMV_BUFFER_VALUE)
				{
//					inJSON_AnylisysEmvDataIntoBrec(pobTran, (char *)st_JasonUnpackEmvData[i].bTagName, cjEmvData->valuestring);
					memcpy(st_TempJasonEmvData[i].bTagMessage, cjEmvData->valuestring, strlen(cjEmvData->valuestring));
					st_TempJasonEmvData[i].inSetValue = VS_TRUE;
				}else
				{
					inDISP_DispLogAndWriteFlie("  JsonUnpack Tag[%s] Len[%d] *Error* Line[%d]", (char*)st_TempJasonEmvData[i].bTagName,
										strlen(cjEmvData->valuestring), __LINE__);
					return VS_ERROR;
				}
				
				cjEmvData = NULL;
			}

		}
	}

//	 inJSON_DebugUnPackDataTest(st_TempJasonEmvData);
	 inJSON_DebugUnPackData();
	 
//	 inJSON_AnylisysEmvDataIntoBrec(pobTran, st_TempJasonEmvData);
	 if(inJSON_AnylisysEmvDataIntoBrec(pobTran) != VS_SUCCESS)
	 {
		 return VS_ERROR;
	 }
	 
	return VS_SUCCESS;
}

int inJSON_UnpackSingData(TRANSACTION_OBJECT *pobTran, cJSON *root)
{
	long lnDataLen = 0;
	unsigned long ulSignHandle;
	cJSON	*cjSignatureData;
	char *szBcdData, *szDecData;
	char szFileName[32] = {0};
	char szFileNameGzBmp[36] = {0};
	/* 先判斷Return Code (START) */
	cjSignatureData = cJSON_GetObjectItem(root, "SignData");
	if( !cjSignatureData )
	{
		inDISP_DispLogAndWriteFlie("  JsonUnpack NonSingData *Warning* Line[%d]", __LINE__);
		return (VS_ERROR);
	}else{
		lnDataLen = strlen(cjSignatureData->valuestring);
		szDecData = malloc(lnDataLen +1);
		szBcdData = malloc(lnDataLen +1);
		memset(szDecData, 0x00, lnDataLen +1);
		memset(szBcdData, 0x00, lnDataLen +1);
		
		memcpy(szDecData, cjSignatureData->valuestring , lnDataLen);
//		inFunc_Base64_Decryption(strlen(cjSignatureData->valuestring), cjSignatureData->valuestring, szDecData);
		
		inFunc_ComposeFileName_InvoiceNumber(pobTran, szFileName, _PICTURE_FILE_EXTENSION_, 6);

		memset(szFileNameGzBmp, 0x00, sizeof(szFileNameGzBmp));
		sprintf(szFileNameGzBmp, "%s%s", szFileName, _GZIP_FILE_EXTENSION_);
		inFILE_Delete((unsigned char*)szFileName);
		inFILE_Delete((unsigned char*)szFileNameGzBmp);
		
		inDISP_LogPrintfWithFlag(" Signed Bmp Len[%d] Name[%s] Line[%d]",strlen(szFileName), szFileName, __LINE__);
		inDISP_LogPrintfWithFlag(" Gz Signed Bmp Len[%d] Name[%s] Line[%d]",strlen(szFileNameGzBmp), szFileNameGzBmp, __LINE__);
		
//		inFILE_Delete("Test1.bmp.gz");
//		inFILE_Delete("Test1.bmp");
		
		inFunc_ASCII_to_BCD((unsigned char*)szBcdData, szDecData,  (lnDataLen / 2));
		
		inFILE_Create(&ulSignHandle, (unsigned char*)szFileNameGzBmp);
		inFILE_Write(&ulSignHandle, (unsigned char*)szBcdData, (lnDataLen / 2));
		inFILE_Close(&ulSignHandle);
		
		free(szDecData);
		free(szBcdData);
		
		if (inFILE_Check_Exist((unsigned char*)szFileNameGzBmp) == VS_SUCCESS)
		{
			/* unZip完成後，會去除 .gz副檔名 */
			if (inFunc_GUnZip_Data("", szFileNameGzBmp, _FS_DATA_PATH_) != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie("  Compress GZip *Error* FileName[%s]  Line[%d]",szFileNameGzBmp, __LINE__);
				return (VS_ERROR);
			}
			/* 更新簽名狀態 */
			pobTran->srBRec.inSignStatus = _SIGN_SIGNED_;
			inBATCH_Update_Sign_Status_By_Sqlite(pobTran);
		}

	}
	return VS_SUCCESS;
}


int inJSON_UnPackReadMifareData(TRANSACTION_OBJECT *pobTran, cJSON *root)
{
	cJSON	*cjRdMifareData;
	char szBcdUdid[16+1] = {0};

	/* 先判斷Return Code (START) */
	cjRdMifareData = cJSON_GetObjectItem(root, "MifareUID");
	if( !cjRdMifareData )
	{
		inDISP_DispLogAndWriteFlie("  JsonUnpack ReadMifare Data  *Warning* Line[%d]", __LINE__);
		return (VS_ERROR);
	}else{
		/* 外接設備讀到UID資料會回傳 16 個BYTES */
		if(strlen(cjRdMifareData->valuestring) != 16)
		{
			inDISP_DispLogAndWriteFlie("  JsonUnpack ReadMifare *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		}
		
		inFunc_ASCII_to_BCD((unsigned char*)szBcdUdid, cjRdMifareData->valuestring, 8);
		
		memcpy(&pobTran->srBRec.szSvcUID[0], &szBcdUdid[0], 3);
		memcpy(&pobTran->srBRec.szSvcUID[3], &szBcdUdid[4], 4);
	
		/* 用檢查(Lock0 && 02h) == 02h來確認是否開卡成功 */
		//TODO_SVC: 這個還沒做，要請Reader改
		
	}
	return VS_SUCCESS;
}

int inJSON_UnpackScanData(TRANSACTION_OBJECT *pobTran, cJSON *root)
{
	long lnDataLen = 0;
	unsigned long ulScanHandle;
	cJSON	*cjScanData;
	char *szDecData;
	
	cjScanData = cJSON_GetObjectItem(root, "ScanData");
	if( !cjScanData )
	{
		inDISP_DispLogAndWriteFlie("  JsonUnpack ScanData *Warning* Line[%d]", __LINE__);
		return (VS_ERROR);
	}else{
		/* 不知道資料多大，所以寫成固定檔 */
		lnDataLen = strlen(cjScanData->valuestring);
		szDecData = malloc(lnDataLen +1);
		memset(szDecData, 0x00, lnDataLen +1);
		memcpy(szDecData, cjScanData->valuestring , lnDataLen);
                
                /* 20230204 Miyano add Start */
                memset(pobTran->szQrData, 0x00, sizeof(pobTran->szQrData));
                memcpy(&pobTran->szQrData[0], cjScanData->valuestring, lnDataLen);
                pobTran->inQrDataLen = lnDataLen;
                /* 20230204 Miyano add End */
		
		inFILE_Delete((unsigned char*)_SCANDATA_FILE_NAME_);
		inFILE_Create(&ulScanHandle, (unsigned char*)_SCANDATA_FILE_NAME_);
		inFILE_Write(&ulScanHandle, (unsigned char*)szDecData, (lnDataLen / 2));
		inFILE_Close(&ulScanHandle);
		
		free(szDecData);
		
		if (inFILE_Check_Exist((unsigned char*)_SCANDATA_FILE_NAME_) != VS_SUCCESS)     /* 20230202 Miyano Fix */
		{
			inDISP_DispLogAndWriteFlie(" JsonUnpack ScanData  NotExist *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		}		
	}
	return VS_SUCCESS;
}


/*
 * 主要解外接設備回傳的地方
 */
int inJSON_UnPackReceiveData(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *stMultiOb)
{
	int inRetVal = VS_SUCCESS;
	cJSON* cjson_Parse_Root = NULL;
	cJSON* cjson_Temp = NULL;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START-----",__FILE__, __FUNCTION__, __LINE__);
	cjson_Parse_Root = cJSON_Parse(stMultiOb->stMulti_TransData.szReceData);
	if(cjson_Parse_Root == NULL)
	{
		inDISP_DispLogAndWriteFlie("  JsonUnpack *Error* Line[%d]", __LINE__);
		return VS_ERROR;
	}
	
	inJSON_DebugPackData(cjson_Parse_Root);

	cjson_Temp = cJSON_GetObjectItem(cjson_Parse_Root, "RespCode");
	
	if(memcmp(cjson_Temp->valuestring, "00", 2) == 0)
	{
		if (!memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_SALE_, 2))
		{
			if(VS_ERROR == inJSON_CheckEmvTagAndUnPackData(pobTran, cjson_Parse_Root))
			{
				inRetVal = VS_ERROR;
			}
		}else if (!memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_SIGNPAD_, 2) ||
				!memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_RESIGN_TRANS_, 2))
		{
			if(VS_ERROR == inJSON_UnpackSingData(pobTran, cjson_Parse_Root))
			{
				inRetVal = VS_ERROR;
			}			
		}else if (!memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_READ_MIFARE_, 2))
		{
			if(VS_ERROR == inJSON_UnPackReadMifareData(pobTran, cjson_Parse_Root))
			{
				inRetVal = VS_ERROR;
			}			
		}else if (!memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_WRITE_MIFARE_, 2))
		{
			/* 寫卡只要有回傳成功，後續不用處理資料 */
			inRetVal = VS_SUCCESS;
		}else if (!memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_SCAN_TRANS_, 2))
		{
			if(VS_ERROR == inJSON_UnpackScanData(pobTran, cjson_Parse_Root))
			{
				inRetVal = VS_ERROR;
			}	
		}
		
		cJSON_Delete(cjson_Parse_Root); 
		return inRetVal;
	} 
        else if(!memcmp(cjson_Temp->valuestring, "08", 2))      /* 20230202 Miyano add for A30_Scan */
	{
		inDISP_DispLogAndWriteFlie("----JsonUnpack Resp *User_Cancel* END---- Line[%d]", __LINE__);
		cJSON_Delete(cjson_Parse_Root); 
		return VS_USER_CANCEL;
	}
        else
	{
		inDISP_DispLogAndWriteFlie("----JsonUnpack Resp *Error* END---- Line[%d]", __LINE__);
		cJSON_Delete(cjson_Parse_Root); 
		return VS_ERROR;
	}
}

