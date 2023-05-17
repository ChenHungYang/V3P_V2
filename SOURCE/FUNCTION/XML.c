#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <unistd.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Transaction.h"
#include "../DISPLAY/Display.h"

extern int	ginDebug;

/*
Function        :inXML_ParseFile
Date&Time       :2018/3/27 下午 3:36
Describe        :解析失敗回NULL
*/
int inXML_ParseFile(char* szFileName, xmlDocPtr* srDoc)
{
	*srDoc = xmlParseFile(szFileName);
	if (*srDoc == NULL)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inXML_SaveFile
Date&Time       :2018/3/27 下午 3:00
Describe        :返回值：写入文件中的字节数量
*/
int inXML_SaveFile(char *szFilename, xmlDocPtr* srDoc, char *szEncoding, int inFormat)
{
	int	inRetVal = -1;
	int	inEncodingLen = 0;
	char	szEncodingDefault[20 + 1] = "utf-8";
	
	inEncodingLen = strlen(szEncoding);
	if (inEncodingLen > 0)
	{
		memset(szEncodingDefault, 0x00, sizeof(szEncodingDefault));
		if (inEncodingLen >= sizeof(szEncodingDefault))
		{
			memcpy(szEncodingDefault, szEncoding, sizeof(szEncodingDefault));
		}
		else
		{
			memcpy(szEncodingDefault, szEncoding, strlen(szEncoding));
		}
	}
	
	inRetVal = xmlSaveFormatFileEnc(szFilename, *srDoc, szEncodingDefault, inFormat);
	if (inRetVal < 0)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inXML_Get_RootElement
Date&Time       :2018/3/27 下午 4:32
Describe        :
*/
int inXML_Get_RootElement(xmlDocPtr* srDoc, xmlNodePtr* srCur)
{
	char	szDebugMsg[100 + 1] = {0};
	
	*srCur = xmlDocGetRootElement(*srDoc);
	if (*srCur == NULL)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Root Failed");
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}
/*
Function        :inXML_FreeDOC
Date&Time       :2018/3/27 下午 3:11
Describe        :釋放結構
*/
int inXML_FreeDOC(xmlDocPtr* srdoc)
{
	xmlFreeDoc(*srdoc);
	
	return (VS_SUCCESS);
}

/*
Function        :inXML_CleanParser
Date&Time       :2018/3/27 下午 3:13
Describe        :
*/
int inXML_CleanParser(void)
{
	xmlCleanupParser();
	
	return (VS_SUCCESS);
}

/*
Function        :inXML_MemoryDump
Date&Time       :2018/3/27 下午 3:14
Describe        :
*/
int inXML_MemoryDump(void)
{
	xmlMemoryDump();
	
	return (VS_SUCCESS);
}

/*
Function        :inXML_End
Date&Time       :2018/3/27 下午 3:16
Describe        :
*/
int inXML_End(xmlDocPtr* srdoc)
{
	inXML_FreeDOC(srdoc);
	inXML_CleanParser();
	inXML_MemoryDump();

	return (VS_SUCCESS);
}

/*
Function        :直接修改xml裡面特定node裡面的值
Date&Time       :2016/12/6 下午 2:13
Describe        :參照http://www.xmlsoft.org/examples/xpath2.c ，先寫部份，未完成
*/
int inEMVXML_Edit_Properties(void)
{
	char			szDebugMsg[100 + 1];
	char			szFilename[20 + 1] = "emv_config.xml";
//	char			szValue[100 + 1] = "這裡填要改的值";
	xmlDocPtr		srDoc;
	xmlXPathContextPtr	srXpathCtx;
	xmlXPathObjectPtr	srXpathObj;
	const xmlChar*		srXpathExpr = (unsigned char*)"這裡填node路徑，可能要做路徑判斷";
	
	/* Init*/
	xmlInitParser();
	
	/* 這裡應該要丟進另一隻function以確保有初始化和釋放 */
	{
		/* Load XML document */
		srDoc = xmlParseFile(szFilename);
		if (srDoc == NULL)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Error: unable to parse file \"%s\"", szFilename);
				inDISP_LogPrintf(szDebugMsg);
			}

			return (VS_ERROR);
		}
		/* Create xpath evaluation context */
		srXpathCtx = xmlXPathNewContext(srDoc);
		if(srXpathCtx == NULL) {
		    fprintf(stderr,"Error: unable to create new XPath context");
		    xmlFreeDoc(srDoc); 
		    return(-1);
		}

		/* Evaluate xpath expression */
		srXpathObj = xmlXPathEvalExpression(srXpathExpr, srXpathCtx);
		if(srXpathObj == NULL) {
		    fprintf(stderr,"Error: unable to evaluate xpath expression \"%s\"", srXpathExpr);
		    xmlXPathFreeContext(srXpathCtx); 
		    xmlFreeDoc(srDoc); 
		    return(-1);
		}

		/* update selected nodes */
//		update_xpath_nodes(srXpathObj->nodesetval, szValue);
		/* 裡面應該用到srXpathObj.nodesetval.nodeTab->name 來比對Tag值逡巡 */


		/* Cleanup of XPath data */
		xmlXPathFreeObject(srXpathObj);
		xmlXPathFreeContext(srXpathCtx); 

		/* dump the resulting srDocument */
		xmlDocDump(stdout, srDoc);


		/* free the srDocument */
		xmlFreeDoc(srDoc); 
		
	}
	
	/* Shutdown libxml(deinital) */
	xmlCleanupParser();

	/* this is to debug memory for regression tests (釋放記憶體)*/
	xmlMemoryDump();
	
	return (VS_SUCCESS);
}

/*
Function        :inXML_ReadFile
Date&Time       :2018/3/27 下午 3:36
Describe        :*url:可填網址或文件名
 *		 *encoding:編碼方式
*/
int inXML_ReadFile(char* szFileName, xmlDocPtr* srDoc)
{
	*srDoc = xmlReadFile(szFileName, NULL, XML_PARSE_RECOVER | XML_PARSE_PEDANTIC);
	if (*srDoc == NULL)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}