#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include "curl/curl.h"
#include <sqlite3.h>

#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/Transaction.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DispMsg.h"
#include "../DISPLAY/DisTouch.h"
#include "../PRINT/Print.h"
#include "../PRINT/PrtMsg.h"

#include "../FUNCTION/EDC.h"
#include "../FUNCTION/Function.h"
#include "../FUNCTION/Sqlite.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/HDPT.h"
#include "../FUNCTION/CPT.h"
#include "../FUNCTION/CDT.h"
#include "../FUNCTION/MVT.h"
#include "../FUNCTION/VWT.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/PWD.h"
#include "../FUNCTION/EST.h"
#include "../FUNCTION/ASMC.h"
#include "../FUNCTION/IPASSDT.h"

#include "../FUNCTION/SCDT.h"
#include "../FUNCTION/PCD.h"
#include "../FUNCTION/PIT.h"

#include "../../EMVSRC/EMVxml.h"
#include "../../EMVSRC/EMVsrc.h"
#include "../../CTLS/CTLS.h"

#include "TMSTABLE/TmsCPT.h"
#include "TMSTABLE/TmsFTP.h"
#include "TMSTABLE/TmsSCT.h"
#include "TMSTABLE/EDCtmsFTPFLT.h"

#include "../FUNCTION/FILE_FUNC/File.h"
#include "../FUNCTION/FILE_FUNC/FIleLogFunc.h"

#include"../FUNCTION/EDC_Para_Table_Func.h"
#include"../FUNCTION/COSTCO_FUNC/Costco.h"

#include "../COMM/Ftps.h"

#include "../EVENT/MenuMsg.h"

#include "EDCTmsDefine.h"

#include "../TMS/EDCTmsFileProc.h"
#include "../TMS/EDCTmsScheduleFunc.h"

#include "TMSTABLE_FUNC/EDCTmsCPTFunc.h"
#include "TMSTABLE_FUNC/EDCTmsCFGFFunc.h"
#include "TMSTABLE_FUNC/EDCTmsMVTFunc.h"
#include "TMSTABLE_FUNC/EDCTmsHDPTFunc.h"
#include "TMSTABLE_FUNC/EDCTmsTDTFunc.h"




#include "EDCTmsUnitFunc.h"
#include "EDCTmsFlow.h"
#include "EDCTmsFtpFunc.h"


extern int ginDebug;  /* Debug使用 extern */
extern char gszTermVersionDate[16 + 1];	/* 暫存的TerminalVersionID */

EDC_FTP_OBJECT gsrFTP;


/*
Function        : inEDCTMS_FTP_FtpsDownloadApVersionFile
Date&Time   : 2016/2/2 下午 8:022019/10/30 下午 4:45
Describe        : 執行FTPS下載 VersionDate 檔案的步驟
 * 若使用FTP ，請調整 CURLOPT_USE_SSL = CURLUSESSL_ALL
 * 若使用FTPS，請調整 CURLOPT_USE_SSL = CURLUSESSL_LAST
 * 
*/

int inEDCTMS_FTP_FtpsDownloadApVersionFile(TRANSACTION_OBJECT *pobTran)
{
	int	i, inFTPPort, inRetVal;
	char	szTemplate[128 + 1];
	char	szTerminalID[8 + 1];
	char	szMerchantID[15 + 1];
	char	szFTPIPAddress[16 + 1];
	char	szFTPPortNum[6 + 1];
	char	szFTPID[20 + 1];
	char	szFTPPW[20 + 1];

	FTPS_REC        srFtpsObj;
        
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 取得FTPS需要用的IP PORT ID PW */
	memset(&srFtpsObj, 0x00, sizeof(srFtpsObj));
	memset(szFTPIPAddress, 0x00, sizeof(szFTPIPAddress));
	inGetFTPIPAddress(szFTPIPAddress);
	memset(szFTPPortNum, 0x00, sizeof(szFTPPortNum));
	inGetFTPPortNum(szFTPPortNum);
	memset(szFTPID, 0x00, sizeof(szFTPID));
	inGetFTPID(szFTPID);
	memset(szFTPPW, 0x00, sizeof(szFTPPW));
	inGetFTPPW(szFTPPW);
    
	if (inLoadHDTRec(0) != VS_SUCCESS)
		return (VS_ERROR);

	if (inLoadHDPTRec(0) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* FTPS目錄用TID來命名 */
	memset(szTerminalID, 0x00, sizeof(szTerminalID));
	inGetTerminalID(szTerminalID);

	memset(szMerchantID, 0x00, sizeof(szMerchantID));	
	inGetMerchantID(szMerchantID);

	/* 組FTPS URL */
	/* TMS BANK ID 富邦為 20*/
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "ftps://%s/Config/20/%s/%s/%s", szFTPIPAddress, szMerchantID, szTerminalID, _TMS_AP_VERSION_DATE_NAME_);
	strcpy(srFtpsObj.szFtpsURL, szTemplate);

	/* 設定CA憑證名稱 */
	strcpy(srFtpsObj.szCACertFileName, _PEM_FTPS_CREDIT_HOST_FILE_NAME_);
	/* 設定CA憑證路徑 */
	strcpy(srFtpsObj.szCACertFilePath, _CA_DATA_PATH_);
	/* FTPS Port */
	inFTPPort = atoi(szFTPPortNum);
	srFtpsObj.inFtpsPort = inFTPPort;
	/* FTPS ID */
	strcpy(srFtpsObj.szFtpsID, _NEXSYS_READ_FTP_ID_); /* 公司FTP讀取的帳號 EDC_TMSFtp2_Reader */
	/* FTPS PW */
	strcpy(srFtpsObj.szFtpsPW, _NEXSYS_READ_FTP_PW_); /* 公司目前的密碼 */
    
	memset(srFtpsObj.szFtpsFileName, 0x00, sizeof(srFtpsObj.szFtpsFileName));
	/* 下載的檔案名 */
	strcpy(srFtpsObj.szFtpsFileName, _TMS_AP_VERSION_DATE_NAME_);
	
	/* 刪除File List */
	inFILE_Delete((unsigned char *)srFtpsObj.szFtpsFileName);
	
	inFTPS_SetMessageForDownload(_DISP_INQE_MESSAGE_);
	
	/* FTPS下載 重試3次 */
	for (i = 0 ; i < 3 ; i ++)
	{
		inRetVal = inFTPS_Download(&srFtpsObj);

		if (inRetVal != VS_SUCCESS)
		{
			/* 提示下載失敗錯誤訊息 */
			if (inRetVal == CURLE_REMOTE_ACCESS_DENIED)
			{
				inDISP_DispLogAndWriteFlie("  FTP Donload Spconfig Access Denied *Error* Line[%d]", __LINE__);
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("檔案路徑設定有誤", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
				inDISP_ChineseFont("下載檔案失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont(srFtpsObj.szFtpsFileName, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_BEEP(3, 500);
			}
			else if (inRetVal == CURLE_REMOTE_FILE_NOT_FOUND)
			{
				inDISP_DispLogAndWriteFlie("  FTP Donload Spconfig File Not Found *Error* Line[%d]", __LINE__);
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("檔案不存在主機上", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
				inDISP_ChineseFont("下載檔案失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont(srFtpsObj.szFtpsFileName, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_BEEP(3, 500);
			}
			else
			{
				inDISP_DispLogAndWriteFlie("  FTP Donload Spconfig Connect *Error* Line[%d]", __LINE__);
				inDISP_Msg_BMP(_ERR_CONNECT_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
			}

			continue;
		}
		else
		{
			/* 檢查檔案是否已下載 */
			if (inFILE_Check_Exist((unsigned char*)srFtpsObj.szFtpsFileName) != VS_SUCCESS)
			{
				continue;
			}
			else
			{
				break;
			}
		}
	}
        
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return inRetVal;
}

/*
Function        : inEDCTMS_FTP_FtpsDownloadSpconfig
Date&Time   : 2016/2/2 下午 8:022019/10/30 下午 4:45
Describe        : 執行FTPS下載 SYSCONFIG 檔案的步驟
 * 若使用FTP ，請調整 CURLOPT_USE_SSL = CURLUSESSL_ALL
 * 若使用FTPS，請調整 CURLOPT_USE_SSL = CURLUSESSL_LAST
 * 
*/

int inEDCTMS_FTP_FtpsDownloadSpconfig(TRANSACTION_OBJECT *pobTran)
{
	int	i, inFTPPort, inRetVal;
	char	szTemplate[128 + 1];
	char	szTerminalID[8 + 1];
	char	szMerchantID[15 + 1];
	char	szFTPIPAddress[16 + 1];
	char	szFTPPortNum[6 + 1];
	char	szFTPID[20 + 1];
	char	szFTPPW[20 + 1];

	FTPS_REC        srFtpsObj;
        
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 取得FTPS需要用的IP PORT ID PW */
	memset(&srFtpsObj, 0x00, sizeof(srFtpsObj));
	memset(szFTPIPAddress, 0x00, sizeof(szFTPIPAddress));
	inGetFTPIPAddress(szFTPIPAddress);
	memset(szFTPPortNum, 0x00, sizeof(szFTPPortNum));
	inGetFTPPortNum(szFTPPortNum);
	memset(szFTPID, 0x00, sizeof(szFTPID));
	inGetFTPID(szFTPID);
	memset(szFTPPW, 0x00, sizeof(szFTPPW));
	inGetFTPPW(szFTPPW);
		
	if (inLoadHDTRec(0) != VS_SUCCESS)
		return (VS_ERROR);

	if (inLoadHDPTRec(0) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* FTPS目錄用TID來命名 */
	memset(szTerminalID, 0x00, sizeof(szTerminalID));
	inGetTerminalID(szTerminalID);
	//memcpy(szTerminalID,"95126001", 8);

	memset(szMerchantID, 0x00, sizeof(szMerchantID));	
	inGetMerchantID(szMerchantID);
	//memcpy(szMerchantID,"000001011140085",15);
	
	/* 組FTPS URL */

	/* TMS BANK ID 富邦為 20*/
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "ftps://%s/Config/20/%s/%s/%s", szFTPIPAddress, szMerchantID, szTerminalID, _TMS_SYSCONFIG_NAME_);
	strcpy(srFtpsObj.szFtpsURL, szTemplate);

	/* 設定CA憑證名稱 */
	strcpy(srFtpsObj.szCACertFileName, _PEM_FTPS_CREDIT_HOST_FILE_NAME_);
	/* 設定CA憑證路徑 */
	strcpy(srFtpsObj.szCACertFilePath, _CA_DATA_PATH_);
	/* FTPS Port */
	inFTPPort = atoi(szFTPPortNum);
	srFtpsObj.inFtpsPort = inFTPPort;
	/* FTPS ID */
	strcpy(srFtpsObj.szFtpsID, _NEXSYS_READ_FTP_ID_); /* 公司FTP讀取的帳號 EDC_TMSFtp2_Reader */
	/* FTPS PW */
	strcpy(srFtpsObj.szFtpsPW,_NEXSYS_READ_FTP_PW_); /* 公司目前的密碼 */
	
	/* 下載的檔案名 */
	strcpy(srFtpsObj.szFtpsFileName, _TMS_SYSCONFIG_NAME_);
	/* 刪除File List */
	inFILE_Delete((unsigned char *)srFtpsObj.szFtpsFileName);
	
	inFTPS_SetMessageForDownload(_DISP_INQE_MESSAGE_);
	
	/* FTPS下載 重試3次 */
	for (i = 0 ; i < 3 ; i ++)
	{
		inRetVal = inFTPS_Download(&srFtpsObj);

		if (inRetVal != VS_SUCCESS)
		{
			/* 提示下載失敗錯誤訊息 */
			if (inRetVal == CURLE_REMOTE_ACCESS_DENIED)
			{
				inDISP_DispLogAndWriteFlie("  FTP Donload Spconfig Access Denied *Error* Line[%d]", __LINE__);
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("檔案路徑設定有誤", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
				inDISP_ChineseFont("下載檔案失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont(srFtpsObj.szFtpsFileName, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_BEEP(3, 500);
			}
			else if (inRetVal == CURLE_REMOTE_FILE_NOT_FOUND)
			{
				inDISP_DispLogAndWriteFlie("  FTP Donload Spconfig File Not Found *Error* Line[%d]", __LINE__);
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("檔案不存在主機上", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
				inDISP_ChineseFont("下載檔案失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont(srFtpsObj.szFtpsFileName, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_BEEP(3, 500);
			}
			else
			{
				inDISP_DispLogAndWriteFlie("  FTP Donload Spconfig Connect *Error* Line[%d]", __LINE__);
				inDISP_Msg_BMP(_ERR_CONNECT_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
			}
			
			continue;
		}
		else
		{
			/* 檢查檔案是否已下載 */
			if (inFILE_Check_Exist((unsigned char*)srFtpsObj.szFtpsFileName) != VS_SUCCESS)
			{
				continue;
			}
			else
			{
				break;
			}
		}
	}
        
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return inRetVal;
}

/*
Function        : inEDCTMS_FTP_FtpsDownload
Date&Time   : 2016/2/2 下午 8:02
Describe        : 執行FTPS下載 FileList 檔案的步驟
 * 若使用FTP ，請調整 CURLOPT_USE_SSL = CURLUSESSL_ALL
 * 若使用FTPS，請調整 CURLOPT_USE_SSL = CURLUSESSL_LAST
 * inEDCTMS_FTPS_Download
*/
int inEDCTMS_FTP_FtpsDownloadFileList(TRANSACTION_OBJECT *pobTran)
{
	int		i, inFTPPort, inRetVal;
	char		szTemplate[128 + 1];
	char		szTerminalID[8 + 1];
	char		szMerchantID[15 + 1];
	char		szFTPIPAddress[16 + 1];
	char		szFTPPortNum[6 + 1];
	char		szFTPID[20 + 1];
	char		szFTPPW[20 + 1];

	FTPS_REC        srFtpsObj;
        
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 取得FTPS需要用的IP PORT ID PW */
	memset(&srFtpsObj, 0x00, sizeof(srFtpsObj));
	memset(szFTPIPAddress, 0x00, sizeof(szFTPIPAddress));
	inGetFTPIPAddress(szFTPIPAddress);
	memset(szFTPPortNum, 0x00, sizeof(szFTPPortNum));
	inGetFTPPortNum(szFTPPortNum);
	memset(szFTPID, 0x00, sizeof(szFTPID));
	inGetFTPID(szFTPID);
	memset(szFTPPW, 0x00, sizeof(szFTPPW));
	inGetFTPPW(szFTPPW);
		
	if (inLoadHDTRec(0) != VS_SUCCESS)
		return (VS_ERROR);

	if (inLoadHDPTRec(0) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* FTPS目錄用TID來命名 */
	memset(szTerminalID, 0x00, sizeof(szTerminalID));
	inGetTerminalID(szTerminalID);
	//memcpy(szTerminalID,"95126001", 8);

	memset(szMerchantID, 0x00, sizeof(szMerchantID));	
	inGetMerchantID(szMerchantID);
	//memcpy(szMerchantID,"000001011140085",15);

	/* TMS BANK ID 富邦為 20*/
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "ftps://%s/Config/20/%s/%s/FileList.txt", szFTPIPAddress, szMerchantID, szTerminalID);
	strcpy(srFtpsObj.szFtpsURL, szTemplate);

	/* 設定CA憑證名稱 */
	strcpy(srFtpsObj.szCACertFileName, _PEM_FTPS_CREDIT_HOST_FILE_NAME_);
	/* 設定CA憑證路徑 */
	strcpy(srFtpsObj.szCACertFilePath, _CA_DATA_PATH_);
	/* FTPS Port */
	inFTPPort = atoi(szFTPPortNum);
	srFtpsObj.inFtpsPort = inFTPPort;
	/* FTPS ID */
	strcpy(srFtpsObj.szFtpsID, _NEXSYS_READ_FTP_ID_); /* 公司FTP讀取的帳號 EDC_TMSFtp2_Reader */
	/* FTPS PW */
	strcpy(srFtpsObj.szFtpsPW, _NEXSYS_READ_FTP_PW_); /* 公司目前的密碼 */

	/* 下載的檔案名 */
	strcpy(srFtpsObj.szFtpsFileName, _TMS_FILELIST_NAME_);
	/* 刪除File List */
	inFILE_Delete((unsigned char *)srFtpsObj.szFtpsFileName);
	
	/* 初始化curl庫 */
	if (inFTPS_Initial(pobTran) != VS_SUCCESS)
	{
		inDISP_Msg_BMP(_ERR_CONNECT_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, 30, "", 0);
		return (VS_ERROR);
	}
	
	inFTPS_SetMessageForDownload(_DISP_PARA_DLD_MESSAGE_);
	/* FTPS下載 重試3次 */
	for (i = 0 ; i < 3 ; i ++)
	{
		inRetVal = inFTPS_Download(&srFtpsObj);

		if (inRetVal != VS_SUCCESS)
		{
			/* 提示下載失敗錯誤訊息 */
			if (inRetVal == CURLE_REMOTE_ACCESS_DENIED)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("檔案路徑設定有誤", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
				inDISP_ChineseFont("下載檔案失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont(srFtpsObj.szFtpsFileName, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_BEEP(3, 500);
			}
			else if (inRetVal == CURLE_REMOTE_FILE_NOT_FOUND)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("檔案不存在主機上", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
				inDISP_ChineseFont("下載檔案失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont(srFtpsObj.szFtpsFileName, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_BEEP(3, 500);
			}
			else
			{
				inDISP_Msg_BMP(_ERR_CONNECT_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
			}
			continue;
		}
		else
		{
			/* 檢查檔案是否已下載 */
			if (inFILE_Check_Exist((unsigned char*)srFtpsObj.szFtpsFileName) != VS_SUCCESS)
			{
				continue;
			}
			else
			{
				break;
			}
		}
	}
	
	inFTPS_Deinitial(pobTran);
        	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}


/*
Function        : inEDCTMS_FTP_FtpsUpload
Date&Time   : 2016/2/2 下午 8:02
Describe        : 執行FTPS上傳檔案的步驟
* 若使用FTP ，請調整 CURLOPT_USE_SSL = CURLUSESSL_ALL
*若使用FTPS，請調整 CURLOPT_USE_SSL = CURLUSESSL_LAST
* inEDCTMS_FTPS_Upload
*/

int inEDCTMS_FTP_FtpsUpload(TRANSACTION_OBJECT *pobTran)
{
	int	inCnt;
	int	i, inFTPPort, inRetVal = VS_ERROR;
	char	szTime[6 + 1];
	char	szDate[12 + 1];
	char	szTemplate[128 + 1];
	char	szReportFileName[128];
	char	szTerminalID[8 + 1];
	char	szMerchantID[15 + 1];
	char	szSN[20 + 1];
	char	szFTPIPAddress[16 + 1];
	char	szFTPPortNum[6 + 1];
	char	szFTPID[20 + 1];
	char	szFTPPW[20 + 1];
	char	szReportData[100];
	char	szRespondCode[4];

	unsigned long   ulFHandle;                           /* FILE Handle */
	FTPS_REC        srFtpsObj;
	RTC_NEXSYS	srRTC;	
        
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 取得FTPS需要用的IP PORT ID PW */
	memset(&srFtpsObj, 0x00, sizeof(srFtpsObj));
	memset(szFTPIPAddress, 0x00, sizeof(szFTPIPAddress));
	inGetFTPIPAddress(szFTPIPAddress);
	memset(szFTPPortNum, 0x00, sizeof(szFTPPortNum));
	inGetFTPPortNum(szFTPPortNum);
	memset(szFTPID, 0x00, sizeof(szFTPID));
	inGetFTPID(szFTPID);
	memset(szFTPPW, 0x00, sizeof(szFTPPW));
	inGetFTPPW(szFTPPW);
		
	if (inLoadHDTRec(0) != VS_SUCCESS)
		return (VS_ERROR);

	if (inLoadHDPTRec(0) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* FTPS目錄用TID來命名 */
	memset(szTerminalID, 0x00, sizeof(szTerminalID));
	inGetTerminalID(szTerminalID);

	memset(szMerchantID, 0x00, sizeof(szMerchantID));	
	inGetMerchantID(szMerchantID);
	
	/* 設定CA憑證名稱 */
	strcpy(srFtpsObj.szCACertFileName, _PEM_FTPS_CREDIT_HOST_FILE_NAME_);
	/* 設定CA憑證路徑 */
	strcpy(srFtpsObj.szCACertFilePath, _CA_DATA_PATH_);
	/* FTPS Port */
	inFTPPort = atoi(szFTPPortNum);
	srFtpsObj.inFtpsPort = inFTPPort;
	/* FTPS ID */
	//strcpy(srFtpsObj.szFtpsID, szFTPID); /* EDC_TMSFtp1_Writer */
	strcpy(srFtpsObj.szFtpsID, _NEXSYS_WRITE_FTP_ID_);
	/* FTPS PW */
	//strcpy(srFtpsObj.szFtpsPW, szFTPPW); /* *&!!$!)%!!^!)!!!$ */
	strcpy(srFtpsObj.szFtpsPW, _NEXSYS_WRITE_FTP_PW_); /* *&!!$!)%!!^!)!!!$ */
	/* 下載的檔案名 */
	
	memset(&srRTC, 0x00, sizeof(srRTC));

	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	memset(szTime, 0x00, sizeof(szTime));
	sprintf(szTime, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);

	memset(szDate, 0x00, sizeof(szDate));
	sprintf(szDate, "%04d%02d%02d", (srRTC.uszYear+2000), srRTC.uszMonth, srRTC.uszDay);

	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%02d%02d", srRTC.uszMonth, srRTC.uszDay);

	memset(szReportFileName, 0x00, sizeof(szReportFileName));
	sprintf(szReportFileName, "%s%s00", szTemplate, szTerminalID);
	
	/* S/N */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	/* 取後8碼，但最後一碼為CheckSum，所以取8~15 */
	inFunc_GetSeriaNumber(szTemplate);
	szTemplate[15] = ' ';
	
	memset(szSN, 0x00, sizeof(szSN));
	memcpy(szSN, &szTemplate[0], strlen(szTemplate));

	memset(szRespondCode, 0x00, sizeof(szRespondCode));
	inGetFTPDownloadResponseCode(szRespondCode);
		
	/* 組FTPS URL */
	/* specify target URL, and note that this URL should include a file name, not only a directory */ 
	memset(szTemplate, 0x00, sizeof(szTemplate));

	sprintf(szTemplate, "ftps://%s/Reply/20/%s", szFTPIPAddress,szReportFileName);

	strcpy(srFtpsObj.szFtpsURL, szTemplate);
	
	inDISP_LogPrintfWithFlag("  FTP FileName[%s] Time[%s] ", szReportFileName, szTime);
	
	/* 新建回報檔案 */
	inRetVal = inFILE_Create(&ulFHandle, (unsigned char *)szReportFileName);
	if (inRetVal != VS_SUCCESS)
	{
		inFILE_Close(&ulFHandle);
		inFILE_Delete((unsigned char *)szReportFileName);

		inDISP_LogPrintfWithFlag("  FTP Report Create File Err [%d] END -----", inRetVal);
		return (VS_ERROR);
	}

	/* 開啟回報檔案 */
	inRetVal = inFILE_Open(&ulFHandle, (unsigned char *)szReportFileName);

	if (inRetVal != VS_SUCCESS)
	{
		inFILE_Close(&ulFHandle);
		inFILE_Delete((unsigned char *)szReportFileName);

		inDISP_LogPrintfWithFlag("  FTP Report Open File Err [%d] END -----", inRetVal);
		return(VS_ERROR);
	}
	
	inRetVal = inFILE_Seek(ulFHandle, 0, _SEEK_BEGIN_); /* 從頭開始 */
	if (inRetVal == VS_ERROR)
	{
		inFILE_Close(&ulFHandle);
		inFILE_Delete((unsigned char *)szReportFileName);
		inDISP_LogPrintfWithFlag("  FTP Report Seek File Err [%d] END -----", inRetVal);
		return (VS_ERROR);
	}

	inCnt = 0;
	memset(szReportData, 0x00, sizeof(szReportData));

	memcpy(&szReportData[inCnt], "20", 2);

	inCnt +=2;
	szReportData[inCnt ++] = 0x2c;
        
	memcpy(&szReportData[inCnt], szMerchantID, strlen(szMerchantID));
	inCnt += strlen(szMerchantID);

	szReportData[inCnt ++] = 0x2c;

	memcpy(&szReportData[inCnt], szTerminalID, strlen(szTerminalID));
	inCnt += strlen(szTerminalID);

	szReportData[inCnt ++] = 0x2c;

	memcpy(&szReportData[inCnt], szSN, strlen(szSN));
	inCnt += strlen(szSN);

	szReportData[inCnt ++] = 0x2c;

	memcpy(&szReportData[inCnt], szDate, strlen(szDate));
	inCnt += strlen(szDate);
	memcpy(&szReportData[inCnt], szTime, strlen(szTime));
	inCnt += strlen(szTime);

	szReportData[inCnt ++] = 0x2c;
	memcpy(&szReportData[inCnt], szRespondCode, 2);
	inCnt += 2;

	inRetVal = inFILE_Write(&ulFHandle, (unsigned char *)&szReportData[0], strlen(szReportData));
	if (inRetVal == VS_ERROR)
	{
		inFILE_Close(&ulFHandle);
		inFILE_Delete((unsigned char *)szReportFileName);
		inDISP_LogPrintfWithFlag("  FTP Report Upload Err [%d] END -----", inRetVal);
		return (VS_ERROR);
	}

	inFILE_Close(&ulFHandle);	

	sprintf(srFtpsObj.szFtpsFileName, "%s%s", _FS_DATA_PATH_, szReportFileName); /* 不指定會找不到 */
	
	/* 初始化curl庫 */
	if (inFTPS_Initial(pobTran) != VS_SUCCESS)
	{
		inDISP_Msg_BMP(_ERR_CONNECT_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, 30, "", 0);

		return (VS_ERROR);
	}
        
	/* FTPS上傳 重試3次 */
	for (i = 0 ; i < 3 ; i ++)
	{
		inRetVal = inFTPS_Upload(&srFtpsObj);

		if (inRetVal != VS_SUCCESS)
		{
			/* 提示下載失敗錯誤訊息 */
			if (inRetVal == CURLE_REMOTE_ACCESS_DENIED)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("檔案路徑設定有誤", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
				inDISP_ChineseFont("上傳檔案失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont(szReportFileName, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_BEEP(3, 500);
			}
			else if (inRetVal == CURLE_REMOTE_FILE_NOT_FOUND)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("檔案不存在主機上", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
				inDISP_ChineseFont("上傳檔案失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont(szReportFileName, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_BEEP(3, 500);
			}
			else
			{
				inDISP_Msg_BMP(_ERR_CONNECT_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
			}

			continue;
		}
		else
		{
			inDISP_ChineseFont(szReportFileName, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
			inDISP_ChineseFont("回報檔案完成", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);			
			inDISP_BEEP(3, 500);			
			break;
		}
	}
        
	inFTPS_Deinitial(pobTran);
	
	/* 不論上傳成功或失敗都要刪除回報檔 */
	inFILE_Delete((unsigned char *)szReportFileName);
	
	/* 設定可以改變 FTPDownloadResponseCode 的值  */
	inTMSFTP_OffFileOpationFlag();
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}


/*
Function        : inEDCTMS_FTP_FtpsEsunUpload
Date&Time   : 2021/5/26 下午 3:37
Describe        : 執行FTPS上傳檔案的步驟
* 若使用FTP ，請調整 CURLOPT_USE_SSL = CURLUSESSL_ALL
*若使用FTPS，請調整 CURLOPT_USE_SSL = CURLUSESSL_LAST
*/

int inEDCTMS_FTP_FtpsEsunUpload(TRANSACTION_OBJECT *pobTran)
{
	int	inCnt;
	int	i, inFTPPort, inRetVal = VS_ERROR;
	char	szTime[6 + 1];
	char	szDate[12 + 1];
	char	szTemplate[128 + 1];
	char	szReportFileName[128];
	char	szTerminalID[8 + 1];
	char	szMerchantID[15 + 1];
	char	szSN[20 + 1];
	char	szFTPIPAddress[16 + 1];
	char	szFTPPortNum[6 + 1];
	char	szFTPID[20 + 1];
	char	szFTPPW[20 + 1];
	char	szReportData[100];
	char	szRespondCode[4];
	unsigned long   ulFHandle;                           /* FILE Handle */
	FTPS_REC        srFtpsObj;
	RTC_NEXSYS	srRTC;	
        
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 取得FTPS需要用的IP PORT ID PW */
	memset(&srFtpsObj, 0x00, sizeof(srFtpsObj));
	memset(szFTPIPAddress, 0x00, sizeof(szFTPIPAddress));
	inGetFTPIPAddress(szFTPIPAddress);
	memset(szFTPPortNum, 0x00, sizeof(szFTPPortNum));
	inGetFTPPortNum(szFTPPortNum);
	memset(szFTPID, 0x00, sizeof(szFTPID));
	inGetFTPID(szFTPID);
	memset(szFTPPW, 0x00, sizeof(szFTPPW));
	inGetFTPPW(szFTPPW);
		
	if (inLoadHDTRec(0) != VS_SUCCESS)
		return (VS_ERROR);

	if (inLoadHDPTRec(0) != VS_SUCCESS)
		return (VS_ERROR);
        
    
        if (inLoadTMSFTPRec(0) != VS_SUCCESS)
                return (VS_ERROR);
        
	/* FTPS目錄用TID來命名 */
	memset(szTerminalID, 0x00, sizeof(szTerminalID));
	inGetTerminalID(szTerminalID);

	memset(szMerchantID, 0x00, sizeof(szMerchantID));	
	inGetMerchantID(szMerchantID);

	/* 設定CA憑證名稱 */
	strcpy(srFtpsObj.szCACertFileName, _PEM_FTPS_CREDIT_HOST_FILE_NAME_);
	/* 設定CA憑證路徑 */
	strcpy(srFtpsObj.szCACertFilePath, _CA_DATA_PATH_);
	/* FTPS Port */
	inFTPPort = atoi(szFTPPortNum);
	srFtpsObj.inFtpsPort = inFTPPort;
	/* FTPS ID */
	//strcpy(srFtpsObj.szFtpsID, szFTPID); /* EDC_TMSFtp1_Writer */
	strcpy(srFtpsObj.szFtpsID, _ESUN_READ_FTP_ID_);
	/* FTPS PW */
	//strcpy(srFtpsObj.szFtpsPW, szFTPPW); /* *&!!$!)%!!^!)!!!$ */
	strcpy(srFtpsObj.szFtpsPW, _ESUN_READ_FTP_PW_); /* *&!!$!)%!!^!)!!!$ */
	/* 下載的檔案名 */
	
	memset(&srRTC, 0x00, sizeof(srRTC));

	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	memset(szTime, 0x00, sizeof(szTime));
	sprintf(szTime, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);

	memset(szDate, 0x00, sizeof(szDate));
	sprintf(szDate, "%04d%02d%02d", (srRTC.uszYear+2000), srRTC.uszMonth, srRTC.uszDay);

	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%02d%02d", srRTC.uszMonth, srRTC.uszDay);

        memset(szRespondCode, 0x00, sizeof(szRespondCode));
        inGetFTPDownloadResponseCode(szRespondCode);
        inDISP_LogPrintfWithFlag("  RespCode [%s] ", szRespondCode);
    
	memset(szReportFileName, 0x00, sizeof(szReportFileName));
	sprintf(szReportFileName, "%4s%8s%2s", szTemplate, szTerminalID,szRespondCode);
	
	/* S/N */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	/* 取後8碼，但最後一碼為CheckSum，所以取8~15 */
	inFunc_GetSeriaNumber(szTemplate);
	szTemplate[15] = ' ';
	
	memset(szSN, 0x00, sizeof(szSN));
	memcpy(szSN, &szTemplate[0], strlen(szTemplate));

	
		
	/* 組FTPS URL */
	/* specify target URL, and note that this URL should include a file name, not only a directory */ 
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "ftps://%s/%s", szFTPIPAddress,szReportFileName);
	strcpy(srFtpsObj.szFtpsURL, szTemplate);
	
	inDISP_LogPrintfWithFlag("  FTP FileName[%s] Time[%s] ", szReportFileName, szTime);
	
	/* 新建回報檔案 */
	inRetVal = inFILE_Create(&ulFHandle, (unsigned char *)szReportFileName);
	if (inRetVal != VS_SUCCESS)
	{
		inFILE_Close(&ulFHandle);
		inFILE_Delete((unsigned char *)szReportFileName);

		inDISP_LogPrintfWithFlag("  FTP Report Create File Err [%d] END -----", inRetVal);
		return (VS_ERROR);
	}

	/* 開啟回報檔案 */
	inRetVal = inFILE_Open(&ulFHandle, (unsigned char *)szReportFileName);

	if (inRetVal != VS_SUCCESS)
	{
		inFILE_Close(&ulFHandle);
		inFILE_Delete((unsigned char *)szReportFileName);

		inDISP_LogPrintfWithFlag("  FTP Report Open File Err [%d] END -----", inRetVal);
		return(VS_ERROR);
	}
	
	inRetVal = inFILE_Seek(ulFHandle, 0, _SEEK_BEGIN_); /* 從頭開始 */
	if (inRetVal == VS_ERROR)
	{
		inFILE_Close(&ulFHandle);
		inFILE_Delete((unsigned char *)szReportFileName);
		inDISP_LogPrintfWithFlag("  FTP Report Seek File Err [%d] END -----", inRetVal);
		return (VS_ERROR);
	}

	inCnt = 0;
	memset(szReportData, 0x00, sizeof(szReportData));

	memcpy(&szReportData[inCnt], "28", 2);

	inCnt +=2;
	szReportData[inCnt ++] = 0x2c;
        
	memcpy(&szReportData[inCnt], szMerchantID, strlen(szMerchantID));
	inCnt += strlen(szMerchantID);

	szReportData[inCnt ++] = 0x2c;

	memcpy(&szReportData[inCnt], szTerminalID, strlen(szTerminalID));
	inCnt += strlen(szTerminalID);

	szReportData[inCnt ++] = 0x2c;

	memcpy(&szReportData[inCnt], szSN, strlen(szSN));
	inCnt += strlen(szSN);

	szReportData[inCnt ++] = 0x2c;

	memcpy(&szReportData[inCnt], szDate, strlen(szDate));
	inCnt += strlen(szDate);
	memcpy(&szReportData[inCnt], szTime, strlen(szTime));
	inCnt += strlen(szTime);

	szReportData[inCnt ++] = 0x2c;
	memcpy(&szReportData[inCnt], szRespondCode, 2);
	inCnt += 2;

	inRetVal = inFILE_Write(&ulFHandle, (unsigned char *)&szReportData[0], strlen(szReportData));
	if (inRetVal == VS_ERROR)
	{
		inFILE_Close(&ulFHandle);
		inFILE_Delete((unsigned char *)szReportFileName);
		inDISP_LogPrintfWithFlag("  FTP Report Upload Err [%d] END -----", inRetVal);
		return (VS_ERROR);
	}

	inFILE_Close(&ulFHandle);	

	sprintf(srFtpsObj.szFtpsFileName, "%s%s", _FS_DATA_PATH_, szReportFileName); /* 不指定會找不到 */
	
	/* 初始化curl庫 */
	if (inFTPS_Initial(pobTran) != VS_SUCCESS)
	{
		inDISP_Msg_BMP(_ERR_CONNECT_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, 30, "", 0);

		return (VS_ERROR);
	}
        
	/* FTPS上傳 重試3次 */
	for (i = 0 ; i < 3 ; i ++)
	{
		inRetVal = inFTPS_Upload(&srFtpsObj);

		if (inRetVal != VS_SUCCESS)
		{
			/* 提示下載失敗錯誤訊息 */
			if (inRetVal == CURLE_REMOTE_ACCESS_DENIED)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("檔案路徑設定有誤", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
				inDISP_ChineseFont("上傳檔案失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont(szReportFileName, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_BEEP(3, 500);
			}
			else if (inRetVal == CURLE_REMOTE_FILE_NOT_FOUND)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("檔案不存在主機上", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
				inDISP_ChineseFont("上傳檔案失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont(szReportFileName, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_BEEP(3, 500);
			}
			else
			{
				inDISP_Msg_BMP(_ERR_CONNECT_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
			}

			continue;
		}
		else
		{
			inDISP_ChineseFont(szReportFileName, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
			inDISP_ChineseFont("回報檔案完成", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);			
			inDISP_BEEP(3, 500);			
			break;
		}
	}
        
	inFTPS_Deinitial(pobTran);
	
	/* 不論上傳成功或失敗都要刪除回報檔 */
	inFILE_Delete((unsigned char *)szReportFileName);
	
	/* 設定可以改變 FTPDownloadResponseCode 的值  */
	inTMSFTP_OffFileOpationFlag();
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}



/*
Function        : inEDCTMS_FTP_FtpsEsunFileLogUpload
Date&Time   : 2021/5/26 下午 3:37
Describe        : 執行FTPS上傳檔案的步驟
* 若使用FTP ，請調整 CURLOPT_USE_SSL = CURLUSESSL_ALL
*若使用FTPS，請調整 CURLOPT_USE_SSL = CURLUSESSL_LAST
*/

int inEDCTMS_FTP_FtpsEsunFileLogUpload(TRANSACTION_OBJECT *pobTran)
{
	int	i, inFTPPort, inRetVal = VS_ERROR;
	char	szTime[6 + 1];
	char	szDate[12 + 1];
	char	szTemplate[128 + 1];
	char	szReportFileName[128];
	char	szTerminalID[8 + 1];
	char	szMerchantID[15 + 1];
	char	szFTPIPAddress[16 + 1];
	char	szFTPPortNum[6 + 1];
	char	szFTPID[20 + 1];
	char	szFTPPW[20 + 1];

	FTPS_REC        srFtpsObj;
	RTC_NEXSYS	srRTC;	
        
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 取得FTPS需要用的IP PORT ID PW */
	memset(&srFtpsObj, 0x00, sizeof(srFtpsObj));
	memset(szFTPIPAddress, 0x00, sizeof(szFTPIPAddress));
	inGetFTPIPAddress(szFTPIPAddress);
	memset(szFTPPortNum, 0x00, sizeof(szFTPPortNum));
	inGetFTPPortNum(szFTPPortNum);
	memset(szFTPID, 0x00, sizeof(szFTPID));
	inGetFTPID(szFTPID);
	memset(szFTPPW, 0x00, sizeof(szFTPPW));
	inGetFTPPW(szFTPPW);
		
	if (inLoadHDTRec(0) != VS_SUCCESS)
		return (VS_ERROR);

	if (inLoadHDPTRec(0) != VS_SUCCESS)
		return (VS_ERROR);
        
    
        if (inLoadTMSFTPRec(0) != VS_SUCCESS)
                return (VS_ERROR);
        
	/* FTPS目錄用TID來命名 */
	memset(szTerminalID, 0x00, sizeof(szTerminalID));
	inGetTerminalID(szTerminalID);

	memset(szMerchantID, 0x00, sizeof(szMerchantID));	
	inGetMerchantID(szMerchantID);

	/* 設定CA憑證名稱 */
	strcpy(srFtpsObj.szCACertFileName, _PEM_FTPS_CREDIT_HOST_FILE_NAME_);
	/* 設定CA憑證路徑 */
	strcpy(srFtpsObj.szCACertFilePath, _CA_DATA_PATH_);
	/* FTPS Port */
	inFTPPort = atoi(szFTPPortNum);
	srFtpsObj.inFtpsPort = inFTPPort;
	/* FTPS ID */
	//strcpy(srFtpsObj.szFtpsID, szFTPID); /* EDC_TMSFtp1_Writer */
	strcpy(srFtpsObj.szFtpsID, _ESUN_READ_FTP_ID_);
	/* FTPS PW */
	//strcpy(srFtpsObj.szFtpsPW, szFTPPW); /* *&!!$!)%!!^!)!!!$ */
	strcpy(srFtpsObj.szFtpsPW, _ESUN_READ_FTP_PW_); /* *&!!$!)%!!^!)!!!$ */
	/* 下載的檔案名 */
	
	memset(&srRTC, 0x00, sizeof(srRTC));

	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	memset(szTime, 0x00, sizeof(szTime));
	sprintf(szTime, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
	
	memset(szDate, 0x00, sizeof(szDate));
	sprintf(szDate, "%02d%02d", srRTC.uszMonth, srRTC.uszDay);
    
	memset(szReportFileName, 0x00, sizeof(szReportFileName));
	sprintf(szReportFileName, "%4s%6s%8s", szDate, szTime, szTerminalID);
        
	/* 組FTPS URL */
	/* specify target URL, and note that this URL should include a file name, not only a directory */ 
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "ftps://%s/%s", szFTPIPAddress,szReportFileName);
	strcpy(srFtpsObj.szFtpsURL, szTemplate);
	
	inDISP_LogPrintfWithFlag("  FTP FileName[%s] Time[%s] ", szReportFileName, szTime);
	
	sprintf(srFtpsObj.szFtpsFileName, "%s%s", _FS_DATA_PATH_, szReportFileName); /* 不指定會找不到 */
	
	/* 初始化curl庫 */
	if (inFTPS_Initial(pobTran) != VS_SUCCESS)
	{
		inDISP_Msg_BMP(_ERR_CONNECT_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, 30, "", 0);
		return (VS_ERROR);
	}
        
        if(VS_SUCCESS != inFunc_Copy_Data(_FILE_LOG_NAME_, _FS_DATA_PATH_, szReportFileName, _FS_DATA_PATH_))
        {
            inDISP_DispLogAndWriteFlie("  FTP FILE LOG Copy Data *Error* Line[%d]",  __LINE__);
            inDISP_Msg_BMP(_ERR_CONNECT_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, 30, "", 0);
            return (VS_ERROR);
        }
        
	/* FTPS上傳 重試3次 */
	for (i = 0 ; i < 3 ; i ++)
	{
		inRetVal = inFTPS_Upload(&srFtpsObj);

		if (inRetVal != VS_SUCCESS)
		{
			/* 提示下載失敗錯誤訊息 */
			if (inRetVal == CURLE_REMOTE_ACCESS_DENIED)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("檔案路徑設定有誤", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
				inDISP_ChineseFont("上傳檔案失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont(szReportFileName, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_BEEP(3, 500);
			}
			else if (inRetVal == CURLE_REMOTE_FILE_NOT_FOUND)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("檔案不存在主機上", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
				inDISP_ChineseFont("上傳檔案失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont(szReportFileName, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_BEEP(3, 500);
			}
			else
			{
				inDISP_Msg_BMP(_ERR_CONNECT_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
			}

			continue;
		}
		else
		{
			inDISP_ChineseFont(szReportFileName, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
			inDISP_ChineseFont("回報檔案完成", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);			
			inDISP_BEEP(3, 500);			
			break;
		}
	}
        
	inFTPS_Deinitial(pobTran);
	
	/* 不論上傳成功或失敗都要刪除回報檔 */
	inFILE_Delete((unsigned char *)szReportFileName);
	
	/* 設定可以改變 FTPDownloadResponseCode 的值  */
	inTMSFTP_OffFileOpationFlag();
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}




/*
Function        : inEDCTMS_FTPS_Check_TID_MID
Date&Time   : 2016/2/2 下午 8:02
Describe        :  檢核設定的 TID MID
*	目前沒在用 20190620 [SAM] 
 * inEDCTMS_FTPS_Check_TID_MID
*/
int inEDCTMS_FTP_FtpsCheckTidAndMid(TRANSACTION_OBJECT *pobTran)
{
        char    szTID[8 + 1];
        char    szMID[15 + 1];

        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inEDCTMS_FTPS_Check_TID_MID() START!");
        
        if (inLoadHDTRec(0) < 0)
                return (VS_ERROR);
	if (inLoadHDPTRec(0) < 0)
                return (VS_ERROR);
        
        memset(szTID, 0x00, sizeof(szTID));
        inGetTerminalID(szTID);
        
        memset(szMID, 0x00, sizeof(szMID));
        inGetMerchantID(szMID);
        
        /* 比對是否相同 不同的話提示錯誤 */
        if (memcmp(gsrFTP.szHeader_TID, szTID, strlen(szTID)))
        {
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                inDISP_ChineseFont("檔案格式有誤", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
                inDISP_BEEP(3, 500);
                return (VS_ERROR);
        }
        
        if (memcmp(gsrFTP.szHeader_MID, szMID, strlen(szMID)))
        {
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                inDISP_ChineseFont("檔案格式有誤", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
                inDISP_BEEP(3, 500);
                return (VS_ERROR);
        }
        
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inEDCTMS_FTPS_Check_TID_MID() END!");
        
        return (VS_SUCCESS);
}


/*
Function        : inEDCTMS_FTP_FtpsDownloadFileByFileList
Date&Time   : 2016/2/2 下午 8:02
Describe        : 利用下載的 File List 整理過後的 szFTPFilePath 對每個檔案進行 FTPS 下載
 * inEDCTMS_FTPS_APPARM_Download
*/
int inEDCTMS_FTP_FtpsDownloadFileByFileList(TRANSACTION_OBJECT *pobTran)
{
	int		i, j, k, inFTPPort, inRetVal;
	int             inFileCnt = 0, inSizes = 0;
	char		szTemplate[128 + 1];
//     char		szTerminalID[8 + 1];		/* 端末機代號 */
	char		szFTPFileAttribute[2 + 1];	/* 檔案屬性 */
	char		szFTPFilePath[60 + 1];		/* 檔案路徑 */
	char		szFTPFileName[26 + 1];	/* 檔案名稱 */
	char		szFTPFileSize[10 + 1];		/* 檔案大小 */
	char		szFTPIPAddress[16 + 1];	/* FTP IP Address */
	char		szFTPPortNum[6 + 1];		/* FTP Port Number */
	char		szFTPID[20 + 1];			/* FTP ID */
	char		szFTPPW[20 + 1];		/* FTP PW */

	FTPS_REC        srFtpsObj;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 取得FTPS需要用的IP PORT ID PW */
	memset(&srFtpsObj, 0x00, sizeof(srFtpsObj));
	memset(szFTPIPAddress, 0x00, sizeof(szFTPIPAddress));
	inGetFTPIPAddress(szFTPIPAddress);
	memset(szFTPPortNum, 0x00, sizeof(szFTPPortNum));
	inGetFTPPortNum(szFTPPortNum);
	memset(szFTPID, 0x00, sizeof(szFTPID));
	inGetFTPID(szFTPID);
	memset(szFTPPW, 0x00, sizeof(szFTPPW));
	inGetFTPPW(szFTPPW);
    
	/* FTPS Port */
	inFTPPort = atoi(szFTPPortNum);
	srFtpsObj.inFtpsPort = inFTPPort; /* inFTPPort; */

	/* FTPS ID */
	strcpy(srFtpsObj.szFtpsID, _NEXSYS_READ_FTP_ID_);

	/* FTPS PW */
	strcpy(srFtpsObj.szFtpsPW, _NEXSYS_READ_FTP_PW_);
	
	/* 初始化curl庫 */
	if (inFTPS_Initial(pobTran) != VS_SUCCESS)
	{
		inDISP_Msg_BMP(_ERR_CONNECT_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, 30, "", 0);
		return (VS_ERROR);
	}
	
    

	/* 下載檔案 用Load FTPFLT Record 來下載檔案 */
	for (i = 0 ;; i ++)
	{
		if (inLoadEDCFTPFLTRec(i) < 0)
			break;

		memset(szFTPFileAttribute, 0x00, sizeof(szFTPFileAttribute));
		inGetEDCFTPFileAttribute(szFTPFileAttribute);

		/* 檢查檔案AP屬性的時間條件，人工下載參數及自動下載參數時不下AP */		 
		if(pobTran->inTMSDwdMode  == _EDC_TMS_AUTO_PARM_DLD_MOD_ || 
		    pobTran->inTMSDwdMode == _EDC_TMS_MANUAL_PARM_DLD_MOD_ )
		{
			/* 過濾屬性為 AP下載的值 2019/12/11 下午 5:03 [SAM] */
			if(szFTPFileAttribute[0] == 'A')
				continue;
		}
		
		memset(szFTPFilePath, 0x00, sizeof(szFTPFilePath));
		inGetEDCFTPFilePath(szFTPFilePath);
				
		k = strlen(szFTPFilePath);
		inSizes = strlen(szFTPFilePath);
		inFileCnt = 0;
		/* 因為在讀取時已會把逗號[，] 篩選掉，所以可以直接拿資料來使用 */
		while (1)
		{
			if (szFTPFilePath[inSizes --] == 0x2F)
				break;

			inFileCnt ++;
		}
                
		inFileCnt --;
		
		memset(szFTPFileName, 0x00, sizeof(szFTPFileName));
		memcpy(&szFTPFileName[0], &szFTPFilePath[k - inFileCnt], inFileCnt);
		
		memset(szFTPFileSize, 0x00, sizeof(szFTPFileSize));
		inGetEDCFTPFileSize(szFTPFileSize);
                
		/* 還沒圖片 先用字顯示 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("檔案下載中", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(srFtpsObj.szFtpsURL, 0x00, sizeof(srFtpsObj.szFtpsURL));
		sprintf(szTemplate, "ftps://%s%s", szFTPIPAddress, szFTPFilePath);
		strcpy(srFtpsObj.szFtpsURL, szTemplate);

		/* 下載的檔案名 */
		memset(srFtpsObj.szFtpsFileName, 0x00, sizeof(srFtpsObj.szFtpsFileName));
		strcpy(srFtpsObj.szFtpsFileName, szFTPFileName);
		
		inFTPS_SetMessageForDownload(_DISP_PARA_DLD_MESSAGE_);
		
		/* 下載檔案 失敗重試3次 */
		for (j = 0; j < 3; j++)
		{
			inRetVal = inFTPS_Download(&srFtpsObj);

			if (inRetVal != VS_SUCCESS)
			{
				/* 提示下載失敗錯誤訊息 */
				inDISP_Msg_BMP(_ERR_TMS_DWL_FAILED_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
				continue;
			}
			else
			{
				/* 有下AP，且AP下載成功 */
				if (memcmp(srFtpsObj.szFtpsFileName, _EDC_APPL_NAME_, strlen(_EDC_APPL_NAME_)) == 0)
				{
					/* 使用 TmsCPT，設定要更新APP */
					inSetTMSAPPUpdateStatus("1");
					inSaveTMSCPTRec(0);
				}

				break;
			}
		}
                
		if (inRetVal != VS_SUCCESS)
		{
			/* 不管成功或失敗都要DeInitial */
			inFTPS_Deinitial(pobTran);
			/* 使用 TmsCPT，設定要更新APP */
			inSetTMSAPPUpdateStatus("0");
			inEDCTMS_SCHEDULE_ResetDownloadDateTime(pobTran);
			inTMSFTP_SetDowloadResponseCode(_FTP_DOWNLOAD_REPORT_DOWNLOAD_RETRY_FAILED_);
			inSaveTMSCPTRec(0);
			inSaveTMSFTPRec(0);
			/* 只要有檔案下載失敗，刪除所有下載檔案  2020/1/3 [SAM] */			
			inEDCTMS_DeleteAllDownloadFileMethodFlow(pobTran);			
			return (VS_ERROR);			
		}
			
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}



/*
Function        : inEDCTMS_FTP_FtpsUpdateFinalParameter
Date&Time   : 2016-08-16 下午 02:19:14
Describe        : 檔案下載完成後，AP更新及檔案參數更新
 *	因為檢查的檔案名稱" _EDC_FTPFLT_FILE_NAME_ "會與ISO下載的不同，所以分開更新
 *	inEDCTMS_FTPS_UpdateParam
*/
int inEDCTMS_FTP_FtpsUpdateFinalParameter(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	int	i = 0, j = 0, inRecord;
	int	k = 0, inFileCnt = 0, inSizes = 0;
#ifdef __UPDATE_TMS_AP__ 	/* 先不更新AP，只更新參數  */
	char	szTMSAPPUpdateStatus[2 + 1];
#endif
	char	szFilePath[60 + 1];
	unsigned char   uszFileName[32 + 1];
	char szCPT_HostIPSecond[15 + 1]={0};            /* 第二授權主機 IP Address */
	char szCPT_HostPortNoSecond[5 + 1] ={0};         /* 第二授權主機 Port No. */
	VS_BOOL		fIPASS = VS_FALSE;
//	VS_BOOL		fECC = VS_FALSE, fICASH = VS_FALSE, fHAPPYCASH = VS_FALSE;	/* 用來標示Host有沒有開 */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數下載＞ */
	inDISP_PutGraphic(_TMS_UPDATING_, 0, _COORDINATE_Y_LINE_8_4_);
		
#ifdef __UPDATE_TMS_AP__	
	/* 使用 TmsCPT 列表，先檢查是否有【APPLICATION】要更新【1 = 需要更新AP】 */
	memset(szTMSAPPUpdateStatus, 0x00, sizeof(szTMSAPPUpdateStatus));
	inGetTMSAPPUpdateStatus(szTMSAPPUpdateStatus);
        
	inDISP_DispLogAndWriteFlie("  TMS APP Update [%s]", szTMSAPPUpdateStatus);
      
	if (memcmp(szTMSAPPUpdateStatus, "1", strlen("1")) == 0)
	{
		/* 更新完AP就會系統自動重開，所以後續動作先做 */
		inSetTMSAPPUpdateStatus("0");
		if (inSaveTMSCPTRec(0) < 0)
		{
			inDISP_DispLogAndWriteFlie("  Save TmsCPT AP Status *Error* Line[%d]", __LINE__);
			return (VS_SUCCESS);
		}
		
		/* 更新 TmsSCT 參數  */
//		inEDCTMS_SaveSysDownloadDateTime(pobTran);

		/* 使用 TmsFPT 
		 * 目前設定參數更新完成都要回報，如ISO格式無法回報需要修改 2019/12/4 上午 9:53 [SAM] */
		inSetFTPEffectiveReportBit("Y");
		if (inSaveTMSFTPRec(0) < 0)
		{
			inDISP_DispLogAndWriteFlie("  Save TMSFPT *Error* Line [%d] END -----", __LINE__);
		}
				
		/* 等到參數轉換完才算完成  */
		inSetTMSOK("N");
		/* 重置紀錄TSAM已註冊的開關*/
		inSetTSAMRegisterEnable("N");
		/* 更新TMS，DCC狀態重置 */
		inSetDCCSettleDownload("0");

		if (inSaveEDCRec(0) < 0)
		{
			inDISP_DispLogAndWriteFlie("  Save EDC Table *Error* Line[%d]", __LINE__);
			return (VS_SUCCESS);
		}
		
		inRetVal = inEDCTMS_FUNC_ProcessAutoUpdateAP();
		
		if (inRetVal == VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  AP更新完成");
			inFunc_Reboot();
			return (VS_SUCCESS); /* 要先重新開機一次，不然更新參數檔會有問題 */
		}
		else
		{
			inDISP_DispLogAndWriteFlie(" AP更新失敗 [%d]", inRetVal);
			return (VS_ERROR);
		}
		
	}
	
#endif 
	
	inRetVal = inFILE_Check_Exist((unsigned char *)_EDC_FTPFLT_FILE_NAME_);
	if (inRetVal == VS_SUCCESS)
	{
		inDISP_LogPrintfWithFlag("  %s Exist Line[%d]" , _EDC_FTPFLT_FILE_NAME_, __LINE__ );
	}
	else
	{
		inDISP_DispLogAndWriteFlie("  %s Not Exist Line[%d]", _EDC_FTPFLT_FILE_NAME_, __LINE__);
	}

	for (i = 0 ;; i++)
	{
		/* Load TMS File List 取得檔名 */
		if (inLoadEDCFTPFLTRec(i) < 0)
			break;

		inFileCnt = 0;
		
		memset(uszFileName, 0x00, sizeof(uszFileName));
		memset(szFilePath, 0x00, sizeof(szFilePath));
		inGetEDCFTPFilePath(szFilePath);
		k = strlen(szFilePath);
		inSizes = strlen(szFilePath);
		while (1)
		{
			if (szFilePath[inSizes --] == 0x2F)
				break;

			inFileCnt ++;
		}
                
		inFileCnt --;
		memcpy(&uszFileName[0], &szFilePath[k - inFileCnt], inFileCnt);
		
		inDISP_DispLogAndWriteFlie(" Tms Update Count[%d] ",  i);
		inDISP_DispLogAndWriteFlie(" Tms Update File[%s] ",  uszFileName);
		
		/* 先確認檔案是否存在 避免誤刪檔案 */
		if (inFILE_Check_Exist(uszFileName) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  TSM Check Flie[%s] *Error* Line[%d]", uszFileName, __LINE__ );
			continue;
		}
                
		inRecord = 0;
               
		/* 重新命名成我們使用的檔案名稱 */
		if (!memcmp(&uszFileName[0], "CardDef.txt", 11))
		{
			/* [檢查參數檔] 此檔案有初始設定，會由TMS下載檔案更新 2022/8/12 上午 10:07 [SAM] */
			/* 刪除TMS下載的檔案並改名 */
			inFILE_Delete((unsigned char *)"CDT.dat");
			inFILE_Rename(uszFileName, (unsigned char *)"CDT.dat");

			/* 計算有幾筆Record */
			for (j = 0 ;; j ++)
			{
				if (inLoadCDTRec(j) < 0)
					break;

				inRecord ++;
			}

		}
		else if (!memcmp(&uszFileName[0], "Config.txt", 10))
		{
			/* [檢查參數檔] 此檔案有初始設定，會由TMS下載檔案更新 2022/8/12 上午 10:07 [SAM] */
			
			//inFILE_Delete((unsigned char *)"CFGT.dat");
			inEDCTMS_CFGF_UpdateConfig();
			//inFILE_Rename(uszFileName, (unsigned char *)"CFGT.dat");

		}
		else if (!memcmp(&uszFileName[0], "SysConfig.txt", 13))
		{
			/* [檢查參數檔] 此檔案有初始設定，會由TMS下載檔案更新 2022/8/12 上午 10:07 [SAM] */
			inFILE_Delete((unsigned char *)"TMSSCT.dat");
			inFILE_Rename(uszFileName, (unsigned char *)"TMSSCT.dat");

			/* 計算有幾筆Record */
			for (j = 0 ;; j ++)
			{
				if (inLoadTMSSCTRec(j) < 0)
					break;

				inRecord ++;
			}
		}                        
		else if (!memcmp(&uszFileName[0], "PWDef.txt", 9))
		{
			/* [檢查參數檔] 此檔案有初始設定，會由TMS下載檔案更新 2022/8/12 上午 10:07 [SAM] */
			inFILE_Delete((unsigned char *)"PWD.dat");
			inFILE_Rename(uszFileName, (unsigned char *)"PWD.dat");

			/* 計算有幾筆Record */
			for (j = 0 ;; j ++)
			{
				if (inLoadPWDRec(j) < 0)
					break;

				inRecord ++;
			}
		}                      
		else if (!memcmp(&uszFileName[0], "HostDef.txt", 11))
		{
			/* [檢查參數檔] 此檔案有初始設定，會由TMS下載檔案，下載後會直接使用TMS下載的檔案 2022/8/12 上午 10:07 [SAM] */
			inFILE_Delete((unsigned char *)"HDT.dat");
			inFILE_Rename(uszFileName, (unsigned char *)"HDT.dat");
			
			/* 因為主機下的主要HOST 會命名為HOST，所以需要重寫主機名稱  */
			inLoadHDTRec(0);
			inSetHostLabel(_HOST_NAME_CREDIT_MAINLY_USE_);
			inSaveHDTRec(0);
			
			/* 計算有幾筆Record */
			for (j = 0 ;; j ++)
			{
				if (inLoadHDTRec(j) < 0)
					break;

				inRecord ++;
			}
			inDISP_LogPrintfWithFlag(" HDT COUNT [%d] ", j);

		}
		else if (!memcmp(&uszFileName[0], "CommDef.txt", 11))
		{
			/* [檢查參數檔] 此檔案有初始設定，會由TMS下載檔案更新 2022/8/12 上午 10:07 [SAM] */
			
			/* 因第二組的值都會帶0,0,0,0，所以先讀出已設定的IP 備份 2019/12/23 下午 2:07 [SAM] */
			if (inLoadCPTRec(0) == VS_ERROR)
			{
				inDISP_DispLogAndWriteFlie("  TMS Load CPT[0]  *Error* Line[%d] ",  __LINE__);
			}
			inGetHostIPSecond(szCPT_HostIPSecond);
			inGetHostPortNoSecond(szCPT_HostPortNoSecond);
			
			inDISP_DispLogAndWriteFlie("  TMS CPT Sec Ip[%s] Port[%s] Line[%d] ", szCPT_HostIPSecond, szCPT_HostPortNoSecond, __LINE__);
			
			inFILE_Delete((unsigned char *)_CPT_FILE_NAME_);
			inFILE_Rename(uszFileName, (unsigned char *)_CPT_FILE_NAME_);
			
			/* 備份CPT參數 */
			if (inEDCTMS_CPT_BackupCptFile() != VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf("  CPT Backup Fail");
				}
			}

		}                     
		else if (!memcmp(&uszFileName[0], "EMVDef.txt", 10))
		{
			/* [檢查參數檔] 此檔案有初始設定，會由TMS下載檔案更新 2022/8/12 上午 10:07 [SAM] */
			inFILE_Delete((unsigned char *)"MVT.dat");
			inFILE_Rename(uszFileName, (unsigned char *)"MVT.dat");

			/* 同步MCCCode */
			inEDCTMS_MVT_SyncMccCode();

			/* 計算有幾筆Record */
			for (j = 0 ;; j ++)
			{
				if (inLoadMVTRec(j) < 0)
					break;

				inRecord ++;
			}
		}                   
		else if (!memcmp(&uszFileName[0], "EMVCLDef.txt", 12))
		{
			/* [檢查參數檔] 此檔案有初始設定，會由TMS下載檔案更新 2022/8/12 上午 10:07 [SAM] */
			inFILE_Delete((unsigned char *)"VWT.dat");
			inFILE_Rename(uszFileName, (unsigned char *)"VWT.dat");

			/* 計算有幾筆Record */
			for (j = 0 ;; j ++)
			{
				if (inLoadVWTRec(j) < 0)
					break;

				inRecord ++;
			}
		}                      
		else if (!memcmp(&uszFileName[0], "EMVCAPK.txt", 11))
		{
			/* [檢查參數檔] 此檔案有初始設定，會由TMS下載檔案更新 2022/8/12 上午 10:07 [SAM] */
			inFILE_Delete((unsigned char *)"EST.dat");
			inFILE_Rename(uszFileName, (unsigned char *)"EST.dat");

			/* 計算有幾筆Record */
			for (j = 0 ;; j ++)
			{
				if (inLoadESTRec(j) < 0)
					break;

				inRecord ++;
			}
		}                      
		else if (!memcmp(&uszFileName[0], "SpecCardDef.txt", 15))
		{
			/* [檢查參數檔] 此檔案由TMS下載產生，沒有初始檔案  2022/8/12 上午 10:07 [SAM] */
			inFILE_Delete((unsigned char *)"SCDT.dat");
			inFILE_Rename(uszFileName, (unsigned char *)"SCDT.dat");

			/* 計算有幾筆Record */
			for (j = 0 ;; j ++)
			{
				if (inLoadSCDTRec(j) < 0)
					break;

				inRecord ++;
			}
		}
		else if (!memcmp(&uszFileName[0], "PCodeDef.txt", 12))
		{
			/* [檢查參數檔] 此檔案由TMS下載產生，沒有初始檔案  2022/8/12 上午 10:07 [SAM] */
			inFILE_Delete((unsigned char *)"PCD.dat");
			inFILE_Rename(uszFileName, (unsigned char *)"PCD.dat");

			/* 計算有幾筆Record */
			for (j = 0 ;; j ++)
			{
				if (inLoadPCDRec(j) < 0)
					break;

				inRecord ++;
			}
		}
		else if (!memcmp(&uszFileName[0], "PayItem.txt", 11))
		{
			/* [檢查參數檔] 此檔案由TMS下載產生，沒有初始檔案  2022/8/12 上午 10:07 [SAM] */
			inFILE_Delete((unsigned char *)"PIT.dat");
			inFILE_Rename(uszFileName, (unsigned char *)"PIT.dat");

			/* 計算有幾筆Record */
			for (j = 0 ;; j ++)
			{
				if (inLoadPITRec(j) < 0)
					break;

				inRecord ++;
			}
		}
		else if (!memcmp(&uszFileName[0], "IPASSDef.txt", 12))
		{
			inFILE_Delete((unsigned char *)"IPASSDT.dat");
			inFILE_Rename(uszFileName, (unsigned char *)"IPASSDT.dat");

			/* 計算有幾筆Record */
			for (j = 0 ;; j ++)
			{
				if (inLoadIPASSDTRec(j) < 0)
					break;

				inRecord ++;
			}

			fIPASS = VS_TRUE;
		}
		else if (!memcmp(&uszFileName[0], "BankLogo.bmp", 12))
		{
			inFILE_Delete( (unsigned char *)"FUBONLOGO.bmp");
			inFILE_Rename(uszFileName, (unsigned char *)"FUBONLOGO.bmp");
//			inFILE_Rename(uszFileName, (unsigned char *)"NCCCLOGO.bmp");
//			inFILE_Copy_File((unsigned char *)"BankLogo.bmp", (unsigned char *)"FUBONLOGO.bmp);
		}              
		else if (!memcmp(&uszFileName[0], "BmpName.bmp", 11))
		{
			inFILE_Delete((unsigned char *)"BMPNAME.bmp");
			inFILE_Rename(uszFileName, (unsigned char *)"BMPNAME.bmp");
		}                    
		else if (!memcmp(&uszFileName[0], "BmpLogo.bmp", 11))
		{
			inFILE_Delete((unsigned char *)"BMPLOGO.bmp");
			inFILE_Rename(uszFileName, (unsigned char *)"BMPLOGO.bmp");
		}
		else if (!memcmp(&uszFileName[0], "BmpNotice.bmp", 13))
		{
			inFILE_Delete((unsigned char *)"BMPNOTICE.bmp");
			inFILE_Rename(uszFileName, (unsigned char *)"BMPNOTICE.bmp");
		}
		else if (!memcmp(&uszFileName[0], "BmpLegal.bmp", 12))
		{
			inFILE_Delete((unsigned char *)"BMPLEGAL.bmp");
			inFILE_Rename(uszFileName, (unsigned char *)"BMPLEGAL.bmp");
		}
		else if (!memcmp(&uszFileName[0], "BmpSlogan.bmp", 13))
		{
			inFILE_Delete((unsigned char *)"BMPSLOGAN.bmp");
			inFILE_Rename(uszFileName, (unsigned char *)"BMPSLOGAN.bmp");
		}
		else if (!memcmp(&uszFileName[0], "CUPLegal.bmp", 12))
		{
			inFILE_Delete((unsigned char *)"CUPLEGAL.bmp");
			inFILE_Rename(uszFileName, (unsigned char *)"CUPLEGAL.bmp");
		}
		else if (!memcmp(&uszFileName[0], "Big5Name.txt", 12))
		{

		}
		else
		{
			inFILE_Delete(uszFileName);
		}

		inDISP_DispLogAndWriteFlie("  TMS Update File[%s] Indx[%d] SUCCESS ", uszFileName, inRecord);
		
        }
        
	/* 刪除所有Host Table */
	inFunc_DeleteAllBatchTable();

	/* 初始化HDPT */
#ifdef _MODIFY_HDPT_FUNC_	
	inHDPT_Initial_AllRercord(_DATA_BASE_NAME_NEXSYS_PARAMETER_,_HDPT_TABLE_NAME_);	
#else
	inEDCTMS_HDPT_InitialFileValue();
#endif	

	/* 初始化CFGT(預設好NCCC缺少的欄位值) */
	//inEDCTMS_Inital_CFGTRec();

	/* 更新EMV參數 */
	inEMVXML_Create_EMVConfigXml();
	
	inFunc_Delete_Data("", _EMV_CONFIG_FILENAME_, _EMV_EMVCL_DATA_PATH_);
	inFunc_Make_Dir(_EMV_EMVCL_DIR_NAME_, _FS_DATA_PATH_);
	inFunc_Move_Data(_EMV_CONFIG_FILENAME_, _AP_ROOT_PATH_, "", _EMV_EMVCL_DATA_PATH_);
	
	/* 更新CTLS感應參數 */
	inEMVXML_Create_CTLSConfigXml();
	
	inFunc_Delete_Data("", _EMVCL_CONFIG_FILENAME_, _EMV_EMVCL_DATA_PATH_);
	inFunc_Make_Dir(_EMV_EMVCL_DIR_NAME_, _FS_DATA_PATH_);
	inFunc_Move_Data(_EMVCL_CONFIG_FILENAME_, _AP_ROOT_PATH_, "", _EMV_EMVCL_DATA_PATH_);
	
	/* 初始化 電票使用的 TDT */
	inEDCTMS_TDT_ReInitialTicketTable();
	/* 初始化票證參數 */
	if (fIPASS == VS_TRUE)
	{
		inEDCTMS_FUNC_ReInitialIPASSTable();
	}
	
	/* 更新完成刪除File List */
	inFILE_Delete((unsigned char *)_EDC_FTPFLT_FILE_NAME_);

	inDISP_DispLogAndWriteFlie("  Parameter 更新成功");
        
        /* 客製化參數需要額外處理時這時進行，20230111 Miyano add */
        inEDCTMS_FTP_CustomerIndicator_Process();

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);        
}

/*
Function    : inEDCTMS_FTP_CustomerIndicator_Process
Date&Time   : 2023/1/11 上午 9:38
Describe    : TMS更新尾聲，若客製化參數需要額外處理時用
*/
int inEDCTMS_FTP_CustomerIndicator_Process(void)
{
    char    szCustomerIndicator[3 + 1];
    
    if (ginDebug == VS_TRUE)
    {
            inDISP_LogPrintf("inEDCTMS_FTP_CustomerIndicator_Process Start");
    }
    
    memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
    inGetCustomIndicator(szCustomerIndicator);

    /* Costco 客製化 初始化TSP IP/Port */
    if (vbCheckCostcoCustom(Costco_New))
    {
        inCostcoInitial();
    }
    
    if (ginDebug == VS_TRUE)
    {
            inDISP_LogPrintf("inEDCTMS_FTP_CustomerIndicator_Process End");
    }
    
    return VS_SUCCESS;
}


/*
Function        : inEDCTMS_FTP_FtpsUpdateFinalApp
Date&Time   : 2021/5/21 下午 4:16
Describe        : 檔案下載完成後，AP更新
*/
int inEDCTMS_FTP_FtpsUpdateFinalApp(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	char	szTMSAPPUpdateStatus[2 + 1];
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
   
        if (inLoadTMSCPTRec(0) < 0)
        {
            inDISP_DispLogAndWriteFlie("  Load TmsCPT AP Status *Error* Line[%d]", __LINE__);
            return (VS_SUCCESS);
        }
    
	/* 使用 TmsCPT 列表，先檢查是否有【APPLICATION】要更新【1 = 需要更新AP】 */
	memset(szTMSAPPUpdateStatus, 0x00, sizeof(szTMSAPPUpdateStatus));
	inGetTMSAPPUpdateStatus(szTMSAPPUpdateStatus);
        
	inDISP_DispLogAndWriteFlie("  TMS APP Update [%s]", szTMSAPPUpdateStatus);
      
	if (memcmp(szTMSAPPUpdateStatus, "1", strlen("1")) == 0)
	{
                    inDISP_ClearAll();
                    inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數下載＞ */
                    inDISP_PutGraphic(_TMS_UPDATING_, 0, _COORDINATE_Y_LINE_8_4_);

		/* 更新完AP就會系統自動重開，所以後續動作先做 */
		inSetTMSAPPUpdateStatus("0");
		if (inSaveTMSCPTRec(0) < 0)
		{
			inDISP_DispLogAndWriteFlie("  Save TmsCPT AP Status *Error* Line[%d]", __LINE__);
			return (VS_SUCCESS);
		}
		
		inRetVal = inEDCTMS_FUNC_ProcessAutoUpdateAP();
		
                    /* 因為玉山只有更新AP，所以更新失敗目前測試不影響原來程式，所以還是繼續跑後續行為 2021/5/27 上午 10:39 [SAM] */
		if (inRetVal == VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  AP更新完成");
			inFunc_Reboot();
			return (VS_SUCCESS); /* 要先重新開機一次，不然更新參數檔會有問題 */
		}
		else
		{
			inDISP_DispLogAndWriteFlie(" AP更新失敗 [%d]", inRetVal);
			return (VS_SUCCESS);
		}
		
	}
	
	/* 刪除所有Host Table  這個再看看有沒有需要進行  */
//	inFunc_DeleteAllBatchTable();

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);        
}



/*
Function        :inEDCTMS_FTP_FtpsCheckAllDownloadFileList
Date&Time       :2017/1/25 下午 2:19
Describe        :檢查檔案完整性，若有下載失敗檔案，檔案全刪
 * inEDCTMS_CheckAllDownloadFile_FTPS
*/
int inEDCTMS_FTP_FtpsCheckAllDownloadFileList()
{
	int		i = 0, k = 0, inFileCnt = 0, inSizes = 0;
	long		lnFileSize = 0, lnTMSFLTSize = 0;
	int		inDownloadStatus = VS_SUCCESS;
	char		szTemplate[60 + 1], szFileName[60 + 1], szFilePath[60 + 1];
	unsigned long   ulFile_Handle;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	if (inFILE_Check_Exist((unsigned char *)_EDC_FTPFLT_FILE_NAME_) != VS_SUCCESS)
	{
		inDISP_LogPrintfWithFlag("  Check Err in FTPS [%s] ", _EDC_FTPFLT_FILE_NAME_);
		return (VS_ERROR);
	}
	
	for (i = 0;; i++)
	{
		/* Load TMS File List 取得檔名 */
		if (inLoadEDCFTPFLTRec(i) < 0)
			break;

		memset(szFileName, 0x00, sizeof(szFileName));
		memset(szFilePath, 0x00, sizeof(szFilePath));
		inGetEDCFTPFilePath(szFilePath);
		k = strlen(szFilePath);
		inSizes = strlen(szFilePath);
		while (1)
		{
			if (szFilePath[inSizes --] == 0x2F)
				break;

			inFileCnt ++;
		}
                
		inFileCnt --;
		memcpy(&szFileName[0], &szFilePath[k - inFileCnt], inFileCnt); 
                
		/* AP剛解壓縮完就刪了，所以不檢查 */
		if (memcmp(szFileName, _EDC_APPL_NAME_, strlen(_EDC_APPL_NAME_)) == 0)
		{
			continue;
		}
		
		/* 比對下載的檔案與File List提供的檔案大小是否一致 */
		lnFileSize = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)szFileName);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetEDCFTPFileSize(szTemplate);
		lnTMSFLTSize = atol(szTemplate);
               
		/* 下載成功或失敗結果存在TMSFileIndex */
		if (lnFileSize == lnTMSFLTSize)
		{
			continue;
		}
		else
		{
			inDISP_LogPrintfWithFlag("  FTPS Check File Size Err Name [%s] ", szFileName);
			inDISP_LogPrintfWithFlag("  File Size [%d]  TMSFLT File Size[%d] ", lnFileSize, lnTMSFLTSize);
			
			inDownloadStatus = VS_ERROR;
			inFILE_Delete((unsigned char *)szFileName);
		}
	}
	
	/* 若有下載失敗，回傳錯誤並刪FileList */
	if (inDownloadStatus == VS_ERROR)
	{
		inDISP_LogPrintfWithFlag("  FTPS TMS下載不完全，刪除FileList");
		
		inFILE_Delete((unsigned char *)_EDC_FTPFLT_FILE_NAME_);
		
		return (VS_ERROR);
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        : inEDCTMS_FTP_FtpsDeleteAllDownloadFileList
Date&Time   : 2017/1/25 下午 4:33
Describe        : 下載檔案全刪
 * inEDCTMS_DeleteAllDownloadFile_FTPS
*/
int inEDCTMS_FTP_FtpsDeleteAllDownloadFileList()
{
	int	i = 0, k =0;
	int	inSizes = 0;
	int	inFileCnt = 0;
	char	szFileName[60 + 1];
	char	szFilePath[60 + 1];
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inEDCTMS_DeleteAllDownloadFile_FTPS() START !");
	}

	if (inFILE_Check_Exist((unsigned char *)_EDC_FTPFLT_FILE_NAME_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	for (i = 0;; i++)
	{
		/* Load TMS File List 取得檔名 */
		if (inLoadEDCFTPFLTRec(i) < 0)
				break;

		memset(szFileName, 0x00, sizeof(szFileName));
		memset(szFilePath, 0x00, sizeof(szFilePath));
		inGetEDCFTPFilePath(szFilePath);
		k = strlen(szFilePath);
		inSizes = strlen(szFilePath);
		inFileCnt = 0;

		while (1)
		{
			if (szFilePath[inSizes --] == 0x2F)
			break;

			inFileCnt ++;
		}

		inFileCnt --;
		memcpy(&szFileName[0], &szFilePath[k - inFileCnt], inFileCnt); 

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "刪除%s", szFileName);
			inDISP_LogPrintf(szDebugMsg);
		}
		inFILE_Delete((unsigned char *)szFileName);
	}

		/* 若有下載失敗，回傳錯誤並刪FileList */
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("刪除FileList");
	}

	inFILE_Delete((unsigned char *)_EDC_FTPFLT_FILE_NAME_);
	inFILE_Delete((unsigned char *)_TMS_FILELIST_NAME_);
	
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inEDCTMS_DeleteAllDownloadFile_FTPS() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
	
	return (VS_SUCCESS);
}


/*
Function        : inEDCTMS_FTP_FtpsSendReceiveFlow
Date&Time   : 2019/06/20
Describe        : 目的是使用FTPS功能進行單檔[FileList.txt]檔案下載
 * inEDCTMS_FuncSendReceive
*/
int inEDCTMS_FTP_FtpsSendReceiveFlow(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 執行FTPS下載 */
	inRetVal = inEDCTMS_FTP_FtpsDownloadFileList(pobTran);

	 /* 下載失敗return Error */
	 if (inRetVal != VS_SUCCESS)
	 {
		 inDISP_LogPrintfWithFlag("   FTPS Download Fail [%d] ",inRetVal );
		 return (VS_ERROR);
	 }

	 inDISP_ClearAll();
	 inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	 inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數下載＞ */
	 inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);

	 /* 檢查下載的檔案大小 */
	 //inNCCCTMS_FTPS_CheckFileSize(pobTran);
	 inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	 return (VS_SUCCESS);
}



/*
Function        : inEDCTMS_FuncSendReceive_FTPS
Date&Time   : 2018/6/27 下午 3:27
Describe        : 只處理FTPS下載 FileList.txt 或 檔案
 * inEDCTMS_FuncSendReceive_FTPS
*/
int inEDCTMS_FTP_FtpsSendReceiveFlowByTransCode(TRANSACTION_OBJECT *pobTran)
{
        int	inRetVal = VS_ERROR;
        int inTermDay, inScheduleDay;
        char szVerTemp[12];

        inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

#if defined(_ESUN_MAIN_HOST_)  && (_MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_)
        
        /* 初始化curl庫 */
        if (inFTPS_Initial(pobTran) != VS_SUCCESS)
        {
                inDISP_Msg_BMP(_ERR_CONNECT_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
                return (VS_ERROR);
        }
    
        /* 新增下載VersionDate 檔案，先判斷是否需要下載AP，如果日期不符且大於卡機日期才進行下載 2021/6/22 下午 2:48 [SAM] */
        inRetVal = inEDCTMS_FTP_FtpsDownloadApVersionFile(pobTran);
        if (inRetVal != VS_SUCCESS)
        {
                inDISP_DispLogAndWriteFlie("  ESUN UPT Dld Ap Version File *Error [%d] Line[%d]", inRetVal, __LINE__);
                /* 不管成功或失敗都要DeInitial */
                inRetVal = inFTPS_Deinitial(pobTran);
                if (inRetVal != VS_SUCCESS)
                        inDISP_DispLogAndWriteFlie("  ESUN UPT Dld Ap Version Deinital Fail [%d] Line[%d] ", inRetVal, __LINE__);
                return (VS_ERROR);
        }

        inDISP_ClearAll();
        inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
        inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數下載＞ */
        inDISP_ChineseFont("版本檢查中", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);
        /* 初始化szVerTemp，避免發生日期判斷錯誤 2021/10/27 上午 10:19 [SAM] */
        memset(szVerTemp, 0x00, sizeof(szVerTemp));
        inRetVal = inEDCTMS_ProcessApVersionDateFile(szVerTemp);
        if (inRetVal != VS_SUCCESS)
        {
                inDISP_DispLogAndWriteFlie("  ESUN UPT Process Ap Version File *Error [%d] Line[%d]", inRetVal, __LINE__);
                /* 不管成功或失敗都要DeInitial */
                inRetVal = inFTPS_Deinitial(pobTran);
                if (inRetVal != VS_SUCCESS)
                        inDISP_DispLogAndWriteFlie("  ESUN UPT Process Ap Version Deinital Fail [%d] Line[%d] ", inRetVal, __LINE__);
                inDISP_ClearAll();
                inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
                inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數下載＞ */
                inDISP_ChineseFont("版本日期檔有問題", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);
                return (VS_ERROR);
        }
        
        inTermDay = inFunc_SunDay_Sum(gszTermVersionDate);
        inScheduleDay = inFunc_SunDay_Sum(szVerTemp);
        
        inDISP_DispLogAndWriteFlie("  Terminal Date[%d]  ScheduleDate [%d]  Line[%d] ", inTermDay, inScheduleDay, __LINE__);
        
        if (inTermDay <= inScheduleDay)
        {
                /* 只下載固定AP位置 */    
                inRetVal = inEDCTMS_FTP_FtpsOnlyDownloadAp(pobTran);

                /* 不管成功或失敗都要DeInitial */
                inFTPS_Deinitial(pobTran);

                if (inRetVal != VS_SUCCESS)
                {
                        inDISP_DispLogAndWriteFlie("  Auto Download FileList Data *Error* Line[%d]", __LINE__);
                        return (VS_ERROR);
                }
                /* 下載成功，開始處理*/
                else
                {
                        inDISP_ClearAll();
                        inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
                        inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數下載＞ */
                        inDISP_ChineseFont("AP下載完成", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);
                /* 檢查下載的檔案大小 */
//            inEDCTMS_FTP_FtpsCheckFileListSize(pobTran);
            }
            
        }else{
                /* 新增Deinitial ，可能沒釋放造成玉山UPT連線失敗 2021/10/27 上午 10:19 [SAM] */
                 inRetVal = inFTPS_Deinitial(pobTran);
                if (inRetVal != VS_SUCCESS)
                        inDISP_DispLogAndWriteFlie("  ESUN UPT Dld Ap Version Deinital Fail [%d] Line[%d] ", inRetVal, __LINE__);
                inDISP_ChineseFont("已是最新版本", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);
                return (VS_ERROR);
        }
#else	
	/* 自動詢問，目前流程還沒定，先保留方法 20190620 [SAM] */
	if (pobTran->inTransactionCode == _EDCTMS_FTP_AUTO_INQUIRY_REPORT_)
	{
		inDISP_DispLogAndWriteFlie("  EDCTMS_FTP_AUTO_INQUIRY_REPORT LINE[%d]", __LINE__);
	
		/* 初始化curl庫 */
		if (inFTPS_Initial(pobTran) != VS_SUCCESS)
		{
			inDISP_Msg_BMP(_ERR_CONNECT_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
			return (VS_ERROR);
		}
		
		inRetVal = inEDCTMS_FTP_FtpsDownloadFileList(pobTran);
		
		/* 不管成功或失敗都要DeInitial */
		inFTPS_Deinitial(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_LogPrintfWithFlag("  FTPS Deinital Fail [%d] ", inRetVal);
			return (VS_ERROR);
		}
		/* 下載成功 */
		else
		{
			
		}
		
	}	
	else if (pobTran->inTransactionCode == _EDCTMS_PRAM_DOWNLOAD_ || 
		  pobTran->inTransactionCode == _EDCTMS_AP_DOWNLOAD_)
	{
		inDISP_DispLogAndWriteFlie("  EDCTMS_LOGON LINE[%d]", __LINE__);
		
		/* 先下載 Sysconfig.txt 檔 */
//		if(inEDCTMS_DowloadSpconfigFileFlow(pobTran) != VS_SUCCESS)
//		{
//			inDISP_DispLogAndWriteFlie("  Sysconfig Dowload *Error* Line[%d]", __LINE__);
//			inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
//			return (VS_ERROR);
//		}
		
		/* 更新 TmsSCT 時間資料但因 TmsSCT會再自動下載時被更新
		 * 所以需同時更新到 TmsFPT裏的資料  */
//		if(inEDCTMS_SaveSysDownloadDateTime(pobTran) != VS_SUCCESS)
//		{
//			inDISP_DispLogAndWriteFlie("  Save Sct Time To TmsCpt *Error* Line[%d]", __LINE__);
//			inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
//			return (VS_ERROR);
//		}
		
		
		/* 檢查是否要進行詢問 */
		if(inEDCTMS_SCHEDULE_CheckInquireDateTime() != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Check Schedule Inq Time *Error* Line[%d]", __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
		}
				
		/* 下載公司TMS 的 FileList.txt  */
		inRetVal = inEDCTMS_FTP_FtpsDownloadFileList(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  FTP Donload FileList *Error* Line[%d]", __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
		}
				
		/* FTP有下就要回報 */
		inLoadTMSFTPRec(0);
		inSetFTPEffectiveReportBit("Y");
		inSaveTMSFTPRec(0);
				
		/* 刪除原有的 [FTPFLT.dat] 檔案，並以TMS下載的 [FileList.txt] 檔案改名並取代 */
		inFILE_Delete((unsigned char *)_EDC_FTPFLT_FILE_NAME_);
		inFILE_Rename((unsigned char *)_TMS_FILELIST_NAME_, (unsigned char *)_EDC_FTPFLT_FILE_NAME_);
		
		/* 下載 FTPFLT.dat 內所列出的檔案 */
		inRetVal = inEDCTMS_FTP_FtpsDownloadFileByFileList(pobTran);
		
		/* 不管成功或失敗都要DeInitial */
		inFTPS_Deinitial(pobTran);
		
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Auto Download FileList Data *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		}
		/* 下載成功，開始處理*/
		else
		{
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數下載＞ */
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);
			/* 檢查下載的檔案大小 */
			inEDCTMS_FTP_FtpsCheckFileListSize(pobTran);
		}
	}
	else if (pobTran->inTransactionCode == _EDCTMS_MANUAL_DOWNLOAD_)
	{
		inDISP_DispLogAndWriteFlie("  _EDCTMS_MANUAL_DOWNLOAD_  LINE[%d]", __LINE__);
		
		/* 因人工下載不會先下載 Sysconfig.txt 檔，所以要再這邊下載並轉換為 TmsSCT檔案 */
		if(inEDCTMS_DowloadSpconfigFileFlow(pobTran) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Sysconfig Dowload *Error* Line[%d]", __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
		}
		
//		/* 更新 TmsSCT 時間資料但因 TmsSCT會再自動下載時被更新
//		 * 所以需同時更新到 TmsFPT裏的資料  */
//		if(inEDCTMS_SaveSysDownloadDateTime(pobTran) != VS_SUCCESS)
//		{
//			inDISP_DispLogAndWriteFlie("  Save Sct Time To TmsCpt *Error* Line[%d]", __LINE__);
//			inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
//			return (VS_ERROR);
//		}
		
		/* 下載公司TMS 的 FileList.txt  */
		inRetVal = inEDCTMS_FTP_FtpsDownloadFileList(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Download FileList *Error* Line[%d]", __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
		}
				
		/* FTP有下就要回報 */
		inLoadTMSFTPRec(0);
		inSetFTPEffectiveReportBit("Y");
		inSaveTMSFTPRec(0);
				
		/* 刪除原有的 [FTPFLT.dat] 檔案，並以TMS下載的 [FileList.txt] 檔案改名並取代 */
		inFILE_Delete((unsigned char *)_EDC_FTPFLT_FILE_NAME_);
		inFILE_Rename((unsigned char *)_TMS_FILELIST_NAME_, (unsigned char *)_EDC_FTPFLT_FILE_NAME_);
		
		/* 下載 FTPFLT.dat 內所列出的檔案 */
		inRetVal = inEDCTMS_FTP_FtpsDownloadFileByFileList(pobTran);
		
		/* 不管成功或失敗都要DeInitial */
		inFTPS_Deinitial(pobTran);
		
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Manul Download FileList Data *Error* Line[%d]", __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
		}
		/* 下載成功，開始處理*/
		else
		{
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數下載＞ */
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);

			/* 檢查下載的檔案大小 */
			inEDCTMS_FTP_FtpsCheckFileListSize(pobTran);
		}
	}
	else
	{
		inDISP_LogPrintfWithFlag("FTP 無此流程");

		inDISP_Msg_BMP("", 0, _BEEP_1TIMES_MSG_, 1, "FTP無此TMS流程", _LINE_8_6_);
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d]  FTP Flow *Error* END-----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
#endif	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}


/*
Function        : inEDCTMS_FTP_FtpsCheckFileListSize
Date&Time   : 2019/6/10 
Describe        : 檢查從TMS已下載的檔案大小與檔案內容資料所提供的檔案大小是否相符
 * inEDCTMS_FTPS_CheckFileSize
*/

int inEDCTMS_FTP_FtpsCheckFileListSize(TRANSACTION_OBJECT *pobTran)
{
	int	i = 0;
	int	inFileCnt = 0, inSizes = 0, k = 0;
	long	lnFileSize = 0, lnTMSFLTSize = 0;
	char	szTemplate[60 + 1], szFileName[60 + 1];
	char	szFilePath[60 + 1];
	unsigned long   ulFile_Handle;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);	
	
	inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);
	
	for (i = 0;; i++)
	{
		/* Load TMS File List 取得檔名 */
		if (inLoadEDCFTPFLTRec(i) < 0)
			break;
                
		memset(szFilePath, 0x00, sizeof(szFilePath));
		inGetEDCFTPFilePath(szFilePath);

		k = strlen(szFilePath);
		inSizes = strlen(szFilePath);
		inFileCnt = 0;
		
		while (1)
		{
			if (szFilePath[inSizes --] == 0x2F)
				break;

			inFileCnt ++;
		}

		inFileCnt --;
		memset(szFileName, 0x00, sizeof(szFileName));
		memcpy(&szFileName[0], &szFilePath[k - inFileCnt], inFileCnt);   	

		/* 比對下載的檔案與File List提供的檔案大小是否一致 */
		lnFileSize = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)szFileName);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetEDCFTPFileSize(szTemplate);
		lnTMSFLTSize = atol(szTemplate);
		
		/* AP 不檢查長度，目前不知道為什麼長度會和下載的檔案長度不一致  */
		if(!memcmp(szFileName, _EDC_APPL_NAME_, strlen(_EDC_APPL_NAME_)))
		{
			inSetEDCFTPFileIndex("Y ");
			inSaveEDCFTPFLTRec(i);
			inDISP_DispLogAndWriteFlie("  AP File Name: %s success", szFileName);
			continue;
		}
		
		/* 下載成功或失敗結果存在TMSFileIndex */
		if (lnFileSize == lnTMSFLTSize)
		{
			inDISP_DispLogAndWriteFlie("  File Name[%d]: %s success", i, szFileName);
			inDISP_DispLogAndWriteFlie("  Now size: %ld 等於 Table size: %ld", lnFileSize, lnTMSFLTSize);

			inSetEDCFTPFileIndex("Y ");
			inSaveEDCFTPFLTRec(i);
		}
		else
		{
			inDISP_DispLogAndWriteFlie("  File Name[%d]: %s", i, szFileName);
			inDISP_DispLogAndWriteFlie("  Now size: %ld 不等於 Table size: %ld", lnFileSize, lnTMSFLTSize);

			inSetEDCFTPFileIndex("N ");
			inSaveEDCFTPFLTRec(i);
		}
			
	}
    
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
	
	return (VS_SUCCESS);
}



/*
Function        : inEDCTMS_FTP_FtpsOnlyDownloadAp
Date&Time   : 2021/5/21 下午 3:07
Describe        : 只下載固定AP檔案 目前只要玉山使用UPT下載就用下面規則，如果要使用原本嘉利要使用的規則要修改
    或是開發玉山一般版TMS時也要新增 2021/5/21 下午 1:41 [SAM] 
*/
int inEDCTMS_FTP_FtpsOnlyDownloadAp(TRANSACTION_OBJECT *pobTran)
{
        int		 j,  inFTPPort, inRetVal;
        char		szTemplate[128 + 1];
        char		szFTPIPAddress[16 + 1];	/* FTP IP Address */
        char		szFTPPortNum[6 + 1];		/* FTP Port Number */
        char		szFTPID[20 + 1];			/* FTP ID */
        char		szFTPPW[20 + 1];		/* FTP PW */
        FTPS_REC        srFtpsObj;

        inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

        /* 取得FTPS需要用的IP PORT ID PW */
        memset(&srFtpsObj, 0x00, sizeof(srFtpsObj));
        memset(szFTPIPAddress, 0x00, sizeof(szFTPIPAddress));
        inGetFTPIPAddress(szFTPIPAddress);
        memset(szFTPPortNum, 0x00, sizeof(szFTPPortNum));
        inGetFTPPortNum(szFTPPortNum);
        memset(szFTPID, 0x00, sizeof(szFTPID));
        inGetFTPID(szFTPID);
        memset(szFTPPW, 0x00, sizeof(szFTPPW));
        inGetFTPPW(szFTPPW);
        
        /* 設定CA憑證名稱 */
        strcpy(srFtpsObj.szCACertFileName, _PEM_FTPS_CREDIT_HOST_FILE_NAME_);
        /* 設定CA憑證路徑 */
        strcpy(srFtpsObj.szCACertFilePath, _CA_DATA_PATH_);

        /* FTPS Port */
        inFTPPort = atoi(szFTPPortNum);
        srFtpsObj.inFtpsPort = inFTPPort; /* inFTPPort; */
        /* FTPS ID */
        strcpy(srFtpsObj.szFtpsID, _ESUN_READ_FTP_ID_);
        /* FTPS PW */
        strcpy(srFtpsObj.szFtpsPW, _ESUN_READ_FTP_PW_);
    
        /* 還沒圖片 先用字顯示 */
        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        inDISP_ChineseFont("檔案下載中", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "ftps://%s/%s", szFTPIPAddress,_EDC_APPL_NAME_);
        strcpy(srFtpsObj.szFtpsURL, szTemplate);
        
        memset(srFtpsObj.szFtpsFileName, 0x00, sizeof(srFtpsObj.szFtpsFileName));
        strcpy(srFtpsObj.szFtpsFileName, _EDC_APPL_NAME_);
        inFILE_Delete((unsigned char *)srFtpsObj.szFtpsFileName);
        
        inFTPS_SetMessageForDownload(_DISP_PARA_DLD_MESSAGE_);
       
        /* 下載檔案 失敗重試3次 */
        for (j = 0; j < 3; j++)
        {
            inRetVal = inFTPS_Download(&srFtpsObj);

            if (inRetVal != VS_SUCCESS)
            {
                /* 提示下載失敗錯誤訊息 */
                inDISP_Msg_BMP(_ERR_TMS_DWL_FAILED_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
                continue;
            }
            else
            {
                inDISP_LogPrintfWithFlag(" Donwload App Success Line[%d] ", __LINE__);
                break;
            }
        }
                
        if (inRetVal != VS_SUCCESS)
        {
                /* 使用 TmsCPT，設定要更新APP */
                inSetTMSAPPUpdateStatus("0");
                inTMSFTP_SetDowloadResponseCode("07");
//                inEDCTMS_SCHEDULE_ResetDownloadDateTime(pobTran);
//                inTMSFTP_SetDowloadResponseCode(_FTP_DOWNLOAD_REPORT_DOWNLOAD_RETRY_FAILED_);
                inSaveTMSCPTRec(0);
                inSaveTMSFTPRec(0);
                /* 只要有檔案下載失敗，刪除所有下載檔案  2020/1/3 [SAM] */			
//                inEDCTMS_DeleteAllDownloadFileMethodFlow(pobTran);
                inRetVal = VS_ERROR;
                
        }else{
                /* 使用 TmsCPT，設定要更新APP */
                inSetTMSAPPUpdateStatus("1");
                inTMSFTP_SetDowloadResponseCode("00");
                inSaveTMSCPTRec(0);
                inSaveTMSFTPRec(0);
                inRetVal = VS_SUCCESS;
        }
        
        /* 不管成功或失敗都要DeInitial */
        inFTPS_Deinitial(pobTran);
        
        inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
        return (inRetVal);
}



