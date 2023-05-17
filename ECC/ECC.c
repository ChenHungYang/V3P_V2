

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/CPT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/ECCDT.h"
#include "../SOURCE/FUNCTION/TDT.h"
#include "../SOURCE/FUNCTION/FILE_FUNC/File.h"
#include "../SOURCE/FUNCTION/XML.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/COMM/Ethernet.h"
#include "../CTLS/CTLS.h"
//#include "../NCCC/NCCCTicketSrc.h"
//#include "../NCCC/NCCCTicketIso.h"
#include "ICER/stdAfx.h"
#include "ICER/libutil.h"
#include "ECC.h"

extern int ginDebug; /* Debug使用 extern */
extern int ginISODebug;
extern int ginEngineerDebug;
extern int ginAPVersionType;
int ginECC_F59_ET_Len = 0;
;
unsigned char guszECCRetryBit = VS_FALSE;
ECC_RETRY_DATA gszECCRetryData;




/*
Function		: inECC_ParseXML_In_ICERData
Date&Time	: 2018/4/23 下午 4:45
Describe		: 
*/
int inECC_ParseXML_In_ICERData(char *szFileName, xmlDocPtr *srDoc)
{
	int inRetVal = VS_ERROR;
	char szFileName_New[50 + 1] = {0};
	char szDebugMsg[100 + 1] = {0};

	xmlKeepBlanksDefault(0); //libxml2 global variable .
	xmlIndentTreeOutput = 1; // indent .with \n

	sprintf(szFileName_New, "./ICERData/%s", szFileName);
	inRetVal = inXML_ReadFile(szFileName_New, srDoc);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Parse Failed, %s", szFileName);
			inDISP_LogPrintfAt(AT,szDebugMsg);
		}
	}

	return (inRetVal);
}


