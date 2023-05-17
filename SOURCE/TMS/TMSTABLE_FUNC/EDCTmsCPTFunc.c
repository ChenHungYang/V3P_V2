#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include "curl/curl.h"

#include "../../INCLUDES/Define_1.h"
#include "../../INCLUDES/Define_2.h"
#include "../../INCLUDES/TransType.h"
#include "../../INCLUDES/Transaction.h"
#include "../../DISPLAY/Display.h"
#include "../../DISPLAY/DispMsg.h"
#include "../../DISPLAY/DisTouch.h"
#include "../../PRINT/Print.h"
#include "../../PRINT/PrtMsg.h"

#include "../../FUNCTION/Function.h"
#include "../../FUNCTION/CPT.h"
#include "../../FUNCTION/CPT_Backup.h"

#include "../../FUNCTION/FILE_FUNC/File.h"

#include "EDCTmsCPTFunc.h"

extern	int	ginDebug;  /* Debug使用 extern */


/*
Function        : inEDCTMS_CPT_BackupCptFile
Date&Time   : 2017/9/15 上午 10:33
Describe        : TMS內CPT的需求，若IP和port沒有值，則用原本的
 *inEDCTMS_Backup_CPT_Parameter
*/
int inEDCTMS_CPT_BackupCptFile()
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	inFILE_Delete((unsigned char *)_CPT_BACKUP_FILE_NAME_);
	inFILE_Copy_File((unsigned char *)_CPT_FILE_NAME_, (unsigned char *)_CPT_BACKUP_FILE_NAME_);
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        : inEDCTMS_Recover_CPT_Parameter
Date&Time   : 2017/9/15 上午 10:33
Describe        : 檢查CPT內每一個IP 和 port
 *	目前沒在用 20190620 [SAM]
 * inEDCTMS_Recover_CPT_Parameter
*/
int inEDCTMS_CPT_RecoverCptFileData()
{
	int	i = 0;
	char	szIP[15 + 1];
	char	szPort[5 + 1];
	char	szIP_Backup[15 + 1];
	char	szPort_Backup[5 + 1];
	char	szDebugMsg[100 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inEDCTMS_Recover_CPT_Parameter() START !");
	}
	
	if (inFILE_Check_Exist((unsigned char *)_CPT_BACKUP_FILE_NAME_) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%s not exist, Not recover", _CPT_BACKUP_FILE_NAME_);
			inDISP_LogPrintf(szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		for (i = 0; ; i++)
		{
			if (inLoadCPTRec(i) != VS_SUCCESS)
			{
				break;
			}
			if (inLoadCPT_BackupRec(i) != VS_SUCCESS)
			{
				break;
			}

			/* 第一授權IP */
			memset(szIP, 0x00, sizeof(szIP));
			inGetHostIPPrimary(szIP);
			if (strlen(szIP) <= 0)
			{
				memset(szIP_Backup, 0x00, sizeof(szIP_Backup));
				inGetHostIPPrimary_Backup(szIP_Backup);
				
				if (strlen(szIP_Backup) > 0)
				{
					inSetHostIPPrimary(szIP_Backup);
					inSaveCPTRec(i);
				}
			}

			/* 第一授權Port */
			memset(szPort, 0x00, sizeof(szPort));
			inGetHostPortNoPrimary(szPort);
			if (strlen(szPort) <= 0)
			{
				memset(szPort_Backup, 0x00, sizeof(szPort_Backup));
				inGetHostPortNoPrimary_Backup(szPort_Backup);
				
				if (strlen(szPort_Backup) > 0)
				{
					inSetHostPortNoPrimary(szPort_Backup);
					inSaveCPTRec(i);
				}
			}
			
			/* 第二授權IP */
			memset(szIP, 0x00, sizeof(szIP));
			inGetHostIPSecond(szIP);
			if (strlen(szIP) <= 0)
			{
				memset(szIP_Backup, 0x00, sizeof(szIP_Backup));
				inGetHostIPSecond_Backup(szIP_Backup);
				
				if (strlen(szIP_Backup) > 0)
				{
					inSetHostIPSecond(szIP_Backup);
					inSaveCPTRec(i);
				}
			}

			/* 第二授權Port */
			memset(szPort, 0x00, sizeof(szPort));
			inGetHostPortNoSecond(szPort);
			if (strlen(szPort) <= 0)
			{
				memset(szPort_Backup, 0x00, sizeof(szPort_Backup));
				inGetHostPortNoSecond_Backup(szPort_Backup);
				
				if (strlen(szPort_Backup) > 0)
				{
					inSetHostPortNoSecond(szPort_Backup);
					inSaveCPTRec(i);
				}
			}

		}
	}
	
	return (VS_SUCCESS);
}
