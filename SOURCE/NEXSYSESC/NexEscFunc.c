#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>

#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../FUNCTION/FILE_FUNC/File.h"
#include "../FUNCTION/FILE_FUNC/FIleLogFunc.h"
#include "../INCLUDES/Transaction.h"
#include "../FUNCTION/Function.h"
#include "../FUNCTION/Signpad.h"
#include "../FUNCTION/FuncTable.h"
#include "../FUNCTION/Accum.h"
#include "../FUNCTION/Batch.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/HDPT.h"


#include "../EVENT/Flow.h"


#include "../DISPLAY/Display.h"

#include "NexEscFunc.h"

extern int ginMachineType;

static unsigned long st_ulEscFileHandle;


/*
Function        : inESC_OpenReprintEscFile
Date&Time   : 2019/10/30 上午 10:55
Describe        : 使用API 開啟要儲存的檔案，如果檔案不存在API會自動建立
 Return	  : 成功回傳 VS_SUCCESS，不成功回傳API的回傳值
*/
int inESC_OpenReprintEscFile(void)
{
	int usRetVal;

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	usRetVal = CTOS_FileOpen((unsigned char *)_ESC_REPIRNT_FILE_NAME_, d_STORAGE_FLASH, &st_ulEscFileHandle);

	if (usRetVal != d_OK)
	{
		inDISP_DispLogAndWriteFlie(" ESC Open File *Error* Handle[%ld] Ruslt[%d] Line[%d]", st_ulEscFileHandle, usRetVal, __LINE__);
	}else
	{
		usRetVal = VS_SUCCESS;
		inDISP_DispLogAndWriteFlie(" ESC Open File Success Line[%d]", __LINE__);
	}
	
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
	return usRetVal;
}

/*
Function        : inESC_CloseReprintEscFile
Date&Time   : 2019/10/30 上午 10:55
Describe        : 關閉已開啟的檔案
 Return	  : 成功回傳 VS_SUCCESS，不成功回傳API的回傳值
*/
int inESC_CloseReprintEscFile(void)
{
	unsigned long usRetVal;
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 關閉檔案Handle */
	usRetVal = CTOS_FileClose(st_ulEscFileHandle);

	if (usRetVal != d_OK)
	{
		inDISP_DispLogAndWriteFlie(" ESC Rep Close File *Error* Handle[%ld] Ruslt[%d] Line[%d]", st_ulEscFileHandle, usRetVal, __LINE__);
	}else{

		st_ulEscFileHandle = -1;
		usRetVal = VS_SUCCESS;
		inDISP_DispLogAndWriteFlie(" ESC Rep Close File Success Line[%d]", __LINE__);
	}
	
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return usRetVal;
}


/*
Function        : inESC_CloseReprintEscFile
Date&Time   : 2019/10/30 上午 10:55
Describe        : 刪除檔案
 Return	  : 成功回傳 VS_SUCCESS，不成功回傳API的回傳值
*/
int inESC_DeleteReprintEscFile(void)
{
	unsigned long usRetVal;
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 關閉檔案Handle */
	usRetVal = CTOS_FileDelete((unsigned char *)_ESC_REPIRNT_FILE_NAME_);

	if (usRetVal != d_OK)
	{
		inDISP_DispLogAndWriteFlie(" ESC Rep Del File *Error* Handle[%ld] Ruslt[%d] Line[%d]", st_ulEscFileHandle, usRetVal, __LINE__);
	}else{

		usRetVal = VS_SUCCESS;
		inDISP_DispLogAndWriteFlie(" ESC Rep Del File Success Line[%d]", __LINE__);
	}
	
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return usRetVal;
}


/*
Function        : inESC_WriteReprintEscFile
Date&Time   : 2019/10/30 上午 10:55
Describe        : 寫資料
 Return	  : 成功回傳 VS_SUCCESS，不成功回傳API的回傳值
*/
int inESC_WriteReprintEscFile(unsigned char *uszWriteBuff, unsigned long ulWriteSize)
{
	int  usRetVal = VS_ERROR ;
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	usRetVal = CTOS_FileWrite(st_ulEscFileHandle, uszWriteBuff, ulWriteSize);
	if (usRetVal != d_OK)
	{
		inDISP_DispLogAndWriteFlie(" ESC Rep Write File *Error* Handle[%ld] Ruslt[%d] Line[%d]", st_ulEscFileHandle, usRetVal, __LINE__);
	}else{
		usRetVal = VS_SUCCESS;
		inDISP_DispLogAndWriteFlie(" ESC Rep Write File Success Line[%d]", __LINE__);
	}
	
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return usRetVal;
}	


/*
Function        : inESC_ReadReprintEscFile
Date&Time   : 2019/10/30 上午 10:55
Describe        : 讀取資料
 Return	  : 成功回傳 VS_SUCCESS，不成功回傳API的回傳值
*/
int inESC_ReadReprintEscFile(unsigned char *uszReadBuff, unsigned long ulReadSize)
{
	int  usRetVal = VS_ERROR ;
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	usRetVal = CTOS_FileRead(st_ulEscFileHandle, uszReadBuff, &ulReadSize);
	if (usRetVal != d_OK)
	{
		inDISP_DispLogAndWriteFlie(" ESC Rep Read File *Error* Handle[%ld] Ruslt[%d] Line[%d]", st_ulEscFileHandle, usRetVal, __LINE__);
	}else{
		usRetVal = VS_SUCCESS;
		inDISP_DispLogAndWriteFlie(" ESC Rep Read File Success Size[%ld] Line[%d]", ulReadSize, __LINE__);
	}
		
	
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return usRetVal;
}


/*
Function        : inESC_CheckFlagAndReprintBill
Date&Time   : 2019/10/30 上午 10:55
Describe        : 檢查檔案內容是否要列印未上傳簽單及記錄對應金額及狀態
 Return	  : 成功回傳 VS_SUCCESS，不成功回傳 VS_ERROR
*/
int inESC_CheckFlagAndReprintBill(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_SUCCESS;
#ifndef _CATHAY_MAIN_HOST_	
	unsigned char uszEscBuf[10];
	TRANSACTION_OBJECT pobEscTempTran;

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	if (inFunc_Check_Print_Capability(ginMachineType) == VS_SUCCESS)
	{
		memset(&pobEscTempTran, 0x00, sizeof(pobEscTempTran));
		memcpy(&pobEscTempTran, pobTran, sizeof(TRANSACTION_OBJECT));

		if(st_ulEscFileHandle == -1)
		{
			inDISP_DispLogAndWriteFlie("  File Not Open *Error* Line[%d]", __LINE__);
			return VS_ERROR;
		}

		memset(uszEscBuf, 0x00, sizeof(uszEscBuf));
		inRetVal = inESC_ReadReprintEscFile(uszEscBuf, _ESC_RPT_DATA_FIX_LEN_);

		if(inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  ESC Rpt File *Error* RetVal[%d]  Line[%d]",inRetVal, __LINE__);
			return VS_ERROR;
		}

		inLoadHDTRec(0);
		inLoadHDPTRec(0);

		/*表示沒有上傳 */
		if(uszEscBuf[1] == 'N')
		{
			inDISP_DispLogAndWriteFlie(" ESC Rpt File *Error* EscBuf[%x]  Line[%d]",uszEscBuf[0], __LINE__);
			return VS_ERROR;
		}else{
			inDISP_DispLogAndWriteFlie(" ESC Rpt Check EscBuf[%s]  Line[%d]",(char *)uszEscBuf, __LINE__);
		}

		pobEscTempTran.srBRec.lnOrgInvNum = atol((char *)&uszEscBuf[1]);
		inRetVal = inBATCH_GetTransRecord_By_Sqlite(&pobEscTempTran);

		if( inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" ESC Rpt Batch Get Rec RetVal[%d] *Error* Line[%d]",inRetVal, __LINE__);
			return VS_ERROR;
		}

		/* 設定簽名狀態為不簽名印紙本 */
		pobEscTempTran.srBRec.inSignStatus = _SIGN_BYPASS_;

		inRetVal = inESC_RunOptForReprintBill(&pobEscTempTran);

		if( inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" ESC Rpt Batch Get Rec RetVal[%d] *Error* Line[%d]",inRetVal, __LINE__);
			return VS_ERROR;
		}else{
			pobEscTempTran.srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_PAPER_;
			inRetVal = inBATCH_Update_ESC_Uploaded_By_Sqlite(&pobEscTempTran);
			if( inRetVal != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie(" ESC Rpt Batch Update Rec RetVal[%d] *Error* Line[%d]",inRetVal, __LINE__);
				inFunc_EDCLock();
				return VS_ERROR;
			}


			if(VS_SUCCESS != inACCUM_UpdateFlow_ESC(&pobEscTempTran, _ESC_ACCUM_STATUS_BYPASS_)){
				inDISP_DispLogAndWriteFlie(" ESC Rpt Accum ByPass *Error* Line[%d]", __LINE__);
				inFunc_EDCLock();
				return VS_ERROR;
			}
		}
	
	}

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
#endif
	return inRetVal;
}

/*
Function        : inESC_RunOptForReprintBill
Date&Time   : 2019/10/30 上午 10:55
Describe        : 執行列印的 FUNCTION 
 Return	  : 成功回傳 VS_SUCCESS，不成功回傳 VS_ERROR
*/
int inESC_RunOptForReprintBill(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_);		

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
	return inRetVal;
}

/*
Function        : inESC_OpenAndSaveFileFlow
Date&Time   : 2019/10/30 上午 10:55
Describe        : 儲存狀態及調閱編號，需用在已存批次有檔案資料之後再存這個檔案
 Return	  : 成功回傳 VS_SUCCESS，不成功回傳 VS_ERROR
*/

int inESC_OpenAndSaveFileFlow(TRANSACTION_OBJECT *pobTran)
{
	
	int	inRetVal = VS_SUCCESS;
#ifndef _CATHAY_MAIN_HOST_
	char szTemp[12];
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = inESC_OpenReprintEscFile();
	if(inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC RPT Open File *Error* RetVal[%d]  Line[%d]",inRetVal, __LINE__);
		return VS_ERROR;
	}
	
	memset(szTemp, 0x00, sizeof(szTemp));
	memcpy(szTemp, "Y%06ld", pobTran->srBRec.lnOrgInvNum);
			
	inRetVal = inESC_WriteReprintEscFile((unsigned char*)szTemp, strlen(szTemp));
	if(inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC RPT Write File *Error* RetVal[%d]  Line[%d]",inRetVal, __LINE__);
		return VS_ERROR;
	}
	
	inRetVal = inESC_CloseReprintEscFile();
	if(inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC RPT Close File *Error* RetVal[%d]  Line[%d]",inRetVal, __LINE__);
		return VS_ERROR;
	}
	
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
#endif	
	return inRetVal;
}

/*
Function        : inESC_ReadAndCheckRepirntFileFlow
Date&Time   : 2019/10/30 上午 10:55
Describe        : 讀取檔案及列印未上傳簽單及更新ESC ACCUM
 Return	  : 成功回傳 VS_SUCCESS，不成功回傳 VS_ERROR
*/

int inESC_ReadAndCheckRepirntFileFlow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
#ifndef _CATHAY_MAIN_HOST_	
	char	szTemp[12];
	
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = inESC_OpenReprintEscFile();
	if(inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC RPT Open File *Error* RetVal[%d]  Line[%d]",inRetVal, __LINE__);
		return VS_ERROR;
	}
	
	inRetVal = inESC_CheckFlagAndReprintBill(pobTran);
	if(inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC RPT Check And Reprint *Error* RetVal[%d]  Line[%d]", inRetVal, __LINE__);
		return VS_ERROR;
	}
		
	/* 如果已列印完成，需把檔案內容初始化 */
	memset(szTemp, 0x00, sizeof(szTemp));
	memcpy(szTemp, "N000000", _ESC_RPT_DATA_FIX_LEN_);
			
	inRetVal = inESC_WriteReprintEscFile((unsigned char*)szTemp, strlen(szTemp));
	if(inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC RPT Write File *Error* RetVal[%d]  Line[%d]",inRetVal, __LINE__);
		return VS_ERROR;
	}
	
	inRetVal = inESC_CloseReprintEscFile();
	if(inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  ESC RPT Close File *Error* RetVal[%d]  Line[%d]",inRetVal, __LINE__);
		return VS_ERROR;
	}
	
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
#endif
	return inRetVal;
}