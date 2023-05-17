

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

#include "../../FUNCTION/Sqlite.h"
#include "../../PRINT/Print.h"
#include "../../FUNCTION/HDPT.h"
#include "../../FUNCTION/HDT.h"
#include "../../FUNCTION/CDT.h"
#include "../../FUNCTION/Function.h"
#include "../../FUNCTION/Card.h"
#include "../../DISPLAY/Display.h"

#include "../../../JSON/cJSON.h"

#include "ExDevicePackData.h"

int inJSON_DebugPackData(cJSON* cjson_temp)
{	
	inDISP_LogPrintfWithFlag(" JSize [%d]", cJSON_GetArraySize(cjson_temp));
	inDISP_LogPrintfWithFlag(" JData [%s]", cJSON_PrintUnformatted(cjson_temp));
	return VS_SUCCESS;
}

int inJSON_PackSaleData(TRANSACTION_OBJECT *pobTran, char* pszPackData)
{
	cJSON* cjson_root = NULL;
	char* szTemp;
	int inArraySize = 0;

	/* 建立一個JSON資料物件(連結串列頭結點) */
	cjson_root = cJSON_CreateObject();
	
	cJSON_AddStringToObject(cjson_root, "TransType", "01");
	cJSON_AddStringToObject(cjson_root, "TransAmount", "500");
	
	inJSON_DebugPackData(cjson_root);

	
	szTemp = cJSON_PrintUnformatted(cjson_root);
	
	inArraySize = strlen(szTemp);
	
	memcpy(pszPackData, szTemp, inArraySize);
	
	cJSON_Delete(cjson_root);
	
	return inArraySize;
}

int inJSON_PackTransStopData(char * pszPackData, char* szRespData)
{
	cJSON* cjson_root = NULL;
	char* szTemp;
	int inArraySize = 0;

	/* 建立一個JSON資料物件(連結串列頭結點) */
	cjson_root = cJSON_CreateObject();
	
	cJSON_AddStringToObject(cjson_root, "TransType", "98");
	cJSON_AddStringToObject(cjson_root, "RespCode", szRespData);
	
	inJSON_DebugPackData(cjson_root);

	szTemp = cJSON_PrintUnformatted(cjson_root);
	
	inArraySize = strlen(szTemp);
	
	memcpy(pszPackData, szTemp, inArraySize);
	
	cJSON_Delete(cjson_root);
	
	return inArraySize;
}


int inJSON_PackSignPadData(TRANSACTION_OBJECT *pobTran, char* pszPackData)
{
	cJSON* cjson_root = NULL;
	char* szTemp;
	char szTempData[42] ={0};
	int inArraySize = 0;

	/* 建立一個JSON資料物件(連結串列頭結點) */
	cjson_root = cJSON_CreateObject();
	
	cJSON_AddStringToObject(cjson_root, "TransType", "06");
	
	sprintf(szTempData,"%ld", pobTran->srBRec.lnTxnAmount);
	cJSON_AddStringToObject(cjson_root, "TransAmount", szTempData);
	
	inJSON_DebugPackData(cjson_root);
	
	szTemp = cJSON_PrintUnformatted(cjson_root);
	
	inArraySize = strlen(szTemp);
	
	memcpy(pszPackData, szTemp, inArraySize);
	
	cJSON_Delete(cjson_root);
	
	return inArraySize;
}

int inJSON_PackReSignData(TRANSACTION_OBJECT *pobTran, char* pszPackData)
{
	cJSON* cjson_root = NULL;
	char* szTemp;
	char szTempData[42] ={0};
	int inArraySize = 0;

	/* 建立一個JSON資料物件(連結串列頭結點) */
	cjson_root = cJSON_CreateObject();
	
	cJSON_AddStringToObject(cjson_root, "TransType", "16");
	
	sprintf(szTempData,"%ld", pobTran->srBRec.lnTxnAmount);
	cJSON_AddStringToObject(cjson_root, "TransAmount", szTempData);
	
	inJSON_DebugPackData(cjson_root);
	
	szTemp = cJSON_PrintUnformatted(cjson_root);
	
	inArraySize = strlen(szTemp);
	
	memcpy(pszPackData, szTemp, inArraySize);
	
	cJSON_Delete(cjson_root);
	
	return inArraySize;
}


int inJSON_PackReadMifaeCardData(TRANSACTION_OBJECT *pobTran, char* pszPackData)
{
	cJSON* cjson_root = NULL;
	char* szTemp;
	int inArraySize = 0;

	/* 建立一個JSON資料物件(連結串列頭結點) */
	cjson_root = cJSON_CreateObject();
	
	cJSON_AddStringToObject(cjson_root, "TransType", "02");

	inJSON_DebugPackData(cjson_root);
	
	szTemp = cJSON_PrintUnformatted(cjson_root);
	
	inArraySize = strlen(szTemp);
	
	memcpy(pszPackData, szTemp, inArraySize);
	
	cJSON_Delete(cjson_root);
	
	return inArraySize;
}


int inJSON_PackWirteMifaeCardData(TRANSACTION_OBJECT *pobTran, char* pszPackData)
{
	cJSON* cjson_root = NULL;
	char* szTemp;
	int inArraySize = 0;

	/* 建立一個JSON資料物件(連結串列頭結點) */
	cjson_root = cJSON_CreateObject();
	
	cJSON_AddStringToObject(cjson_root, "TransType", "03");
	cJSON_AddStringToObject(cjson_root, "MifareIssuerId", pobTran->srBRec.szSvcIssueId);
	cJSON_AddStringToObject(cjson_root, "MifareCardNo", pobTran->srBRec.szSvcCardNumber);
	cJSON_AddStringToObject(cjson_root, "MifareExpDate", pobTran->srBRec.szSvcExpireDay);
	cJSON_AddStringToObject(cjson_root, "MifareATID", pobTran->srBRec.szSvcCardTid);

	inJSON_DebugPackData(cjson_root);
	
	szTemp = cJSON_PrintUnformatted(cjson_root);
	
	inArraySize = strlen(szTemp);
	
	memcpy(pszPackData, szTemp, inArraySize);
	
	cJSON_Delete(cjson_root);
	
	return inArraySize;
}



int inJSON_PackScanData(TRANSACTION_OBJECT *pobTran, char* pszPackData)
{
	cJSON* cjson_root = NULL;
	char* szTemp;
	int inArraySize = 0;

	/* 建立一個JSON資料物件(連結串列頭結點) */
	cjson_root = cJSON_CreateObject();
	
	cJSON_AddStringToObject(cjson_root, "TransType", "04");

	inJSON_DebugPackData(cjson_root);
	
	szTemp = cJSON_PrintUnformatted(cjson_root);
	
	inArraySize = strlen(szTemp);
	
	memcpy(pszPackData, szTemp, inArraySize);
	
	cJSON_Delete(cjson_root);
	
	return inArraySize;
}

int inJSON_PackDisplayMsgData(TRANSACTION_OBJECT *pobTran, char* pszPackData)
{
	cJSON* cjson_root = NULL;
	char* szTemp;
	int inArraySize = 0;

	/* 建立一個JSON資料物件(連結串列頭結點) */
	cjson_root = cJSON_CreateObject();
	
	cJSON_AddStringToObject(cjson_root, "TransType", "05");
	cJSON_AddStringToObject(cjson_root, "DisplayMsg", "01");

	inJSON_DebugPackData(cjson_root);
	
	szTemp = cJSON_PrintUnformatted(cjson_root);
	
	inArraySize = strlen(szTemp);
	
	memcpy(pszPackData, szTemp, inArraySize);
	
	cJSON_Delete(cjson_root);
	
	return inArraySize;
}



int inJSON_PackRebootData(TRANSACTION_OBJECT *pobTran, char* pszPackData)
{
	cJSON* cjson_root = NULL;
	char* szTemp;
	int inArraySize = 0;

	/* 建立一個JSON資料物件(連結串列頭結點) */
	cjson_root = cJSON_CreateObject();
	
	cJSON_AddStringToObject(cjson_root, "TransType", "99");

	inJSON_DebugPackData(cjson_root);
	
	szTemp = cJSON_PrintUnformatted(cjson_root);
	
	inArraySize = strlen(szTemp);
	
	memcpy(pszPackData, szTemp, inArraySize);
	
	cJSON_Delete(cjson_root);
	
	return inArraySize;
}

