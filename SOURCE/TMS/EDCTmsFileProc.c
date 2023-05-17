#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <ctosapi.h>


#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"

#include "../FUNCTION/FILE_FUNC/File.h"
#include "../FUNCTION/FILE_FUNC/FIleLogFunc.h"
#include "../FUNCTION/Function.h"

#include "../PRINT/Print.h"

#include "../DISPLAY/Display.h"

#include "../TMS/EDCTmsScheduleFunc.h"

#include "TMSTABLE/TmsSCT.h"
#include "TMSTABLE/TmsCPT.h"
#include "TMSTABLE/TmsFTP.h"

#include "EDCTmsDefine.h"
#include "EDCTmsFileProc.h"
#include "EDCTmsUnitFunc.h"


/*
Function	: inEDCTMS_SaveSysDownloadDateTime
Date&Time	: 2019/11/1 下午 4:25
Describe	: 在SYSCONFIG下載後，儲存要比對日期的文件
 * 目前只使用FTP, 所以只更新 TmsFTP的檔案 
 *  而目前此函式只是在確認是否要更新時間，不會影響後續行為，
 * 所以都回傳成功
*/
int inEDCTMS_SaveSysDownloadDateTime(TRANSACTION_OBJECT *pobTran)
{
#if 1
	int inRetVal;
	char szStartDate[9+1], szStartTime[7+1];
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	inRetVal = inLoadTMSSCTRec(0);
	if(inRetVal != VS_SUCCESS){
		inDISP_DispLogAndWriteFlie("  SaveSys Load TMSSCT *Error* Line[%d]", __LINE__);
		return (VS_SUCCESS);
	}
	
	memset(szStartDate, 0x00, sizeof(szStartDate));
	memset(szStartTime, 0x00, sizeof(szStartTime));
	
	inGetTMSInquireStartDate(szStartDate);		
	inGetTMSInquireTime(szStartTime);
	
//	/* 檢查 TmsSCT時間是否 >= 端末機時間，如果是就更新 TmsFTP */
//	if(VS_SUCCESS !=  inEDCTMS_SCHEDULE_CheckUpdteFptInquireDateTime(szStartDate, szStartTime) )
//	{
//		inDISP_DispLogAndWriteFlie("  Check Update Date  *Error* Line[%d]", __LINE__);
//		return (VS_SUCCESS);
//	}
		
	/* 更新詢問日期  */
	if(VS_SUCCESS !=  inSetFTPInquireStartDate(szStartDate))
	{
		inDISP_DispLogAndWriteFlie("  Set FTP Inq Date *Error* Line[%d]", __LINE__);
		return (VS_SUCCESS);
	}

	/* 更新詢問時間  */
	if(VS_SUCCESS !=  inSetFTPInquireStartTime(szStartTime))
	{
		inDISP_DispLogAndWriteFlie("  Set FTP Inq Time *Error* Line[%d]", __LINE__);
		return (VS_SUCCESS);
	}
	
//	/* 人工排程把下載模式設為 AP及 參數一起下載 */
//	if(pobTran->inTMSDwdMode == _EDC_TMS_MANUAL_PARM_AP_DLD_MOD_) 
//	{
//		inSetTMSDownLoadType(_TMS_INQUIRE_AP_);
//	}
		
	inDISP_DispLogAndWriteFlie("  ReSave Date[%s] Time[%s] Line [%d]", szStartDate, szStartTime, __LINE__);
	
//	if (inSaveTMSFTPRec(0) < 0)
//	{
//		inDISP_DispLogAndWriteFlie("  Save TMSFPT *Error* Line [%d] END -----", __LINE__);
//		return (VS_SUCCESS);
//	}
		
#else	
	
	unsigned long ulFileHandle;
	int inRetVal, inWriteAddr = 0;
	char szWriteBuf[16], szCopyTemp[10];
	if (inFILE_Open(&ulFileHandle, (unsigned char *)_TMS_SYS_BACKUP_FILE_NAME_) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie("  SaveSys File Open *Error* Line[%d]", __LINE__);
		if (inFILE_Create(&ulFileHandle, (unsigned char *)_TMS_SYS_BACKUP_FILE_NAME_) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie("  SaveSys File Create *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		}
	}
	
	inRetVal = inLoadTMSSCTRec(0);
	if(inRetVal != VS_SUCCESS){
		inDISP_DispLogAndWriteFlie("  SaveSys Load TMSSCT *Error* Line[%d]", __LINE__);
		inFILE_Close(&ulFileHandle);
		return (VS_ERROR);
	}
	
	memset(szWriteBuf, 0x00, sizeof(szWriteBuf));
	
	memset(szCopyTemp, 0x00, sizeof(szCopyTemp));
	inGetTMSInquireStartDate(szCopyTemp);
	memcpy(&szWriteBuf[inWriteAddr], szCopyTemp, strlen(szCopyTemp));
	inWriteAddr += strlen(szCopyTemp);
	
	szWriteBuf[inWriteAddr] = ',';
	inWriteAddr ++;
	
	memset(szCopyTemp, 0x00, sizeof(szCopyTemp));
	inGetTMSInquireTime(szCopyTemp);
	memcpy(&szWriteBuf[inWriteAddr], szCopyTemp, strlen(szCopyTemp));
	inWriteAddr += strlen(szCopyTemp);
	szWriteBuf[inWriteAddr] = ',';
	inWriteAddr ++;
	
	inRetVal = inFILE_Write(&ulFileHandle, (unsigned char*)szWriteBuf, (unsigned long)inWriteAddr);
	if(inRetVal != VS_SUCCESS){
		inDISP_DispLogAndWriteFlie("  SaveSys Write File  *Error* Line[%d]", __LINE__);
		inFILE_Close(&ulFileHandle);
		return (VS_ERROR);
	}
			
	inRetVal = inFILE_Close(&ulFileHandle);
	if(inRetVal != VS_SUCCESS){
		inDISP_DispLogAndWriteFlie("  SaveSys Close File *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}
#endif	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}


 /*
Function	: inEDCTMS_DeleteCompareVersionDateFile
Date&Time	: 2019/11/4 下午 5:20
Describe	: 砍掉已建立要比對時間的檔案，目前只會用在人工輸入時
*/
 int inEDCTMS_DeleteCompareVersionDateFile()
 {
	 
	if (inFILE_Delete((unsigned char *)_TMS_SYS_BACKUP_FILE_NAME_) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie("  Delete CompareVesion File *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}
	 
	 return (VS_SUCCESS);
 }
 
int inEDCTMS_ProcessApVersionDateFile(char *szOutData)
{
	
	unsigned long ulFileHandle;
	int inRedLen = 8, i;
	char szApVersionTemp[16];
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	if (inFILE_Open(&ulFileHandle, (unsigned char *)_TMS_AP_VERSION_DATE_NAME_) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" Ap Version File Open *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}	
	
	
	if (inFILE_Seek(ulFileHandle, 0, _SEEK_BEGIN_) == VS_ERROR){
		inDISP_DispLogAndWriteFlie(" Ap Version Seek *Error* Line[%d]", __LINE__);
		inFILE_Close(&ulFileHandle);
		return (VS_ERROR);
	}
	
	memset(szApVersionTemp, 0x00, sizeof(szApVersionTemp));
	
	/* 固定讀 8Bytes */
	if (inFILE_Read(&ulFileHandle, (unsigned char*)szApVersionTemp, inRedLen) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" Ap Version Read File *Error* Line[%d]", __LINE__);
		inFILE_Close(&ulFileHandle);
		return (VS_ERROR);
	}
	
	inDISP_DispLogAndWriteFlie("  File Version[%s] Len[%d] Line[%d]",szApVersionTemp, inRedLen, __LINE__);
	
	/* 資料內容不能為 0~9 之外的數值 */
	for (i = 0; i < inRedLen; i ++)
	{
		if ((szApVersionTemp[i] >= '0') && (szApVersionTemp[i] <= '9'))
		{
			continue;
		}
		else
		{
			inFILE_Close(&ulFileHandle);
			inDISP_DispLogAndWriteFlie(" Ap Verion Value i[%d][%x] *Error* Line[%d] ", i, szApVersionTemp[i], __LINE__);
			return (VS_ERROR);			
		}
	}
	
	memcpy(szOutData, szApVersionTemp, 8);
	
	inDISP_DispLogAndWriteFlie("  Copy Version[%s] Line[%d]",szOutData, __LINE__);
	
	inFILE_Close(&ulFileHandle);
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}


