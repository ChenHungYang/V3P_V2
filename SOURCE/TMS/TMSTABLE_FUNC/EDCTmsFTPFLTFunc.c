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

#include "../../FUNCTION/FILE_FUNC/File.h"

#include "EDCTmsFTPFLTFunc.h"

/*
Function        : inEDCTMS_FTPFLT_Delete_AP
Date&Time   : 2017/1/3 下午 5:50
Describe        : 為了讓FTP拔掉AP那一欄Record
 *	目前沒在用 20190620 [SAM]
 * inEDCTMS_FTPFLT_Delete_AP
*/
int inEDCTMS_FTPFLT_DeleteAP(char *szFileName)
{
	/* lnTotalLen : 該檔案全長度 ,		inRecLen : 一個Record的長度 */
	/* lnFinalToalLen :Resort完的長度，用來確認檔案完整性 */
	/* szFile : 該檔案全部內容 ,		szRecord : 該Record全部內容 */

	int	i;
	int	inOldIndex = -1;
	int	inOldRecLen = 0;		/* Old Record 長度 */
	int	inRecCnt = 0;
	int	inOldRecStartOffset = 0;	/* Old Record 起始位置 */
	int	inRetVal;
	char	*szFile, *szRecord;
	char	szFTPFileAttribute[2 + 1];	/* 檔案屬性 */
	long	lnTotalLen = 0;
	long	lnLeftSize = 0;			/* 剩餘長度 */
	unsigned long	ulHandle;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	for (i = 0 ;; i ++)
	{  
		if (inLoadEDCFTPFLTRec(i) < 0)
			break;

		memset(szFTPFileAttribute, 0x00, sizeof(szFTPFileAttribute));
		inGetEDCFTPFileAttribute(szFTPFileAttribute);

		/* 須判斷是否下AP 手動下載不下AP */
		if (memcmp(szFTPFileAttribute, "A", 1) == 0)
		{
			inOldIndex = i;
			break;
		}
	}
	
	/* index不合法 */
	if (inOldIndex == -1)
	{
		return (VS_ERROR);
	}

	inRetVal = inFILE_Open(&ulHandle, (unsigned char*)szFileName);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
        
	/* 算總長度 */
	lnTotalLen = lnFILE_GetSize(&ulHandle, (unsigned char*)szFileName);

	/* 讀出HDT所有資料 */
	inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_);
	szFile = malloc((lnTotalLen + 1) * sizeof(char));
	memset(szFile, 0x00, lnTotalLen * sizeof(char));
	lnLeftSize = lnTotalLen;
	
	for (i = 0;; ++i)
	{
		/* 剩餘長度大於或等於1024 */
		if (lnLeftSize >= 1024)
		{
			if (inFILE_Read(&ulHandle, (unsigned char*)&szFile[1024 * i], 1024) == VS_SUCCESS)
			{
				/* 一次讀1024 */
				lnLeftSize -= 1024;

				/* 當剩餘長度剛好為1024，會剛好讀完 */
				if (lnLeftSize == 0)
					break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulHandle);

				/* Free pointer */
				free(szFile);

				return (VS_ERROR);
			}
		}
		/* 剩餘長度小於1024 */
		else if (lnLeftSize < 1024)
		{
			/* 就只讀剩餘長度 */
			if (inFILE_Read(&ulHandle, (unsigned char*)&szFile[1024 * i], lnLeftSize) == VS_SUCCESS)
			{
				break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulHandle);

				/* Free pointer */
				free(szFile);

				return (VS_ERROR);
			}
			
		}
		
	}
	
	/* 算出Old Record的長度 */
	for (i = 0, inRecCnt = 0; i < lnTotalLen; i++)
	{
		/* Record 長度 */
		if (inRecCnt == inOldIndex)
		{
			inOldRecLen++;
		}
		/* 若大於，代表已經算完，要跳出 */
		else if (inRecCnt > inOldIndex)
		{
			break;
		}
		
		/* Record 結尾 */
		if (szFile[i] == 0x0A && szFile[i - 1] == 0x0D)
		{
			inRecCnt++;
			
			/* 算出Record起始位置 若是第0 Record，永遠不會進去，則inOldRecStartOffset 為 0 */
			if (inRecCnt == inOldIndex)
			{
				inOldRecStartOffset = i + 1;
			}
		}
		
	}
	
	/* 移到Record的起始位置 */
	if (inFILE_Seek(ulHandle, inOldRecStartOffset, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		inFILE_Close(&ulHandle);
		free(szFile);
		
		return (VS_ERROR);
	}
	
	/* 讀出Record的資料 */
	szRecord = malloc((inOldRecLen + 1) * sizeof(char));
	memset(szRecord, 0x00, inOldRecLen * sizeof(char));
	if (inFILE_Read(&ulHandle, (unsigned char*)szRecord, inOldRecLen) != VS_SUCCESS)
	{
		inFILE_Close(&ulHandle);
		free(szRecord);
		free(szFile);
		
		return (VS_ERROR);
	}
	
	/* 因這裡V3模仿Verifone的insert和delete，但用自己的方式實做，所以需要先關檔再由該function操作*/
	if (inFILE_Close(&ulHandle) != VS_SUCCESS)
	{
		free(szRecord);
		free(szFile);
		
		return (VS_ERROR);
	}
	
	/* 釋放記憶體 */
        free(szFile);
	
	if (inFILE_Data_Delete((unsigned char*)szFileName, inOldRecStartOffset, inOldRecLen) != VS_SUCCESS)	/* 刪除DCC那一段 */
	{
		/* 釋放記憶體 */
		free(szRecord);
		
		return (VS_ERROR);
	}
	
	/* 釋放記憶體 */
	free(szRecord);

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}
