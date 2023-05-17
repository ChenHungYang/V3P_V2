#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <unistd.h>
#include <sqlite3.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Transaction.h"
#include "../DISPLAY/Display.h"
#include "Batch.h"
#include "Sqlite.h"
//#include "Utility.h"
#include "EDC_Para_Table_Func.h"

extern	int		ginDebug;		/* Debug使用 extern */
extern	char		gszParamDBPath[100 + 1];
/*
Function        :inEDCPara_Create_Table_Flow
Date&Time       :2019/4/23 下午 1:57
Describe        :在這邊決定名稱並分流
*/
int inEDCPara_Create_Table_Flow(char* szDBName, char* szTableName, SQLITE_TAG_TABLE* pobSQLTag)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
#ifdef _MODIFY_HDPT_FUNC_		
	inDISP_LogPrintfWithFlag("DBName[%s] szTableName[%s] ", szDBName, szTableName);
	inDISP_LogPrintfWithFlag("gszParamDBPath[%s] ", gszParamDBPath);
	
	/* DBName不合法 */
	if (szDBName == NULL	||
	    strlen(szDBName) == 0)
	{
		if (szDBName == NULL)
		{
			inDISP_DispLogAndWriteFlie("szDBName null pointer");
		}
		else if (strlen(szDBName) == 0)
		{
			inDISP_DispLogAndWriteFlie("szDBName Length = 0");
		}
		
		return (VS_ERROR);
	}
	
	/* TableName 不合法 */
	if (szTableName == NULL	||
	    strlen(szTableName) == 0)
	{
		if (szTableName == NULL)
		{
			inDISP_DispLogAndWriteFlie("szTableName null pointer");
		}
		else if (strlen(szTableName) == 0)
		{
			inDISP_DispLogAndWriteFlie("szDBName Length = 0");
		}
		
		return (VS_ERROR);
	}
		
	inRetVal = inSqlite_Create_Table(gszParamDBPath, szTableName, pobSQLTag);
#endif	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}