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

#include "../../EMVSRC/EMVsrc.h"
#include "../../EMVSRC/EMVxml.h"

#include "../../CTLS/CTLS.h"

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
#include "../FUNCTION/SCDT.h"
#include "../FUNCTION/PCD.h"
#include "../FUNCTION/PIT.h"


#include "../FUNCTION/FILE_FUNC/File.h"

#include "../FUNCTION/ASMC.h"
#include "../FUNCTION/IPASSDT.h"

#include "../EVENT/MenuMsg.h"

#include "TMSTABLE_FUNC/EDCTmsCPTFunc.h"
#include "TMSTABLE_FUNC/EDCTmsMVTFunc.h"
#include "TMSTABLE_FUNC/EDCTmsHDPTFunc.h"
#include "TMSTABLE_FUNC/EDCTmsTDTFunc.h"

#include "EDCTmsDefine.h"
#include "TMSTABLE/EDCtmsFTPFLT.h"
#include "TMSTABLE/TmsCPT.h"
#include "TMSTABLE/TmsFLT.h"
#include "TMSTABLE/TmsSCT.h"


#include "EDCTmsFlow.h"
#include "EDCTmsUnitFunc.h"

#include "EDCTmsIsoFunc.h"

/*
Function        : inEDCTMS_ISO_CheckAllDownloadFile
Date&Time   : 2016/12/13 下午 2:47
Describe        : 檢查檔案完整性，若有下載失敗檔案，檔案全刪
 * inEDCTMS_CheckAllDownloadFile_ISO8583
*/
int inEDCTMS_ISO_CheckAllDownloadFile(void)
{
	int		i = 0, j = 0, inFileNameLen = 0, inSlash = 0;
	long		lnFileSize = 0, lnTMSFLTSize = 0;
	int		inDownloadStatus = VS_SUCCESS;
	char		szTemplate[60 + 1], szFileName[60 + 1];
	unsigned long   ulFile_Handle;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	if (inFILE_Check_Exist((unsigned char *)_TMSFLT_FILE_NAME_) != VS_SUCCESS)
	{
		inDISP_LogPrintfWithFlag("  Check Err in Iso [%s] ", _TMSFLT_FILE_NAME_);
		return (VS_ERROR);
	}
	
	for (i = 0;; i++)
	{
		/* Load TMS File List 取得檔名 */
		if (inLoadTMSFLTRec(i) < 0)
			break;

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSFilePathName(szTemplate);

		/* 取得檔案路徑後 抓取檔名儲存到FileName */
		for (j = 0; j < 60 ; j++)
		{
			if (szTemplate[j] == '/')
				inSlash = j + 1;

			if (szTemplate[j] == 0x00)
				break;
		}

		/* 斜線後的檔名長度*/
		inFileNameLen = j - inSlash;
		memset(szFileName, 0x00, sizeof(szFileName));
		memcpy(&szFileName[0], &szTemplate[inSlash], inFileNameLen);

		/* AP剛解壓縮完就刪了，所以不檢查 */
		if (memcmp(szFileName, _EDC_APPL_NAME_, strlen(_EDC_APPL_NAME_)) == 0)
		{
			continue;
		}

		/* 比對下載的檔案與File List提供的檔案大小是否一致 */
		lnFileSize = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)szFileName);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSFileSize(szTemplate);
		lnTMSFLTSize = atol(szTemplate);

		/* 下載成功或失敗結果存在TMSFileIndex */
		if (lnFileSize == lnTMSFLTSize)
		{
			continue;
		}
		else
		{
			inDISP_LogPrintfWithFlag("  ISO Check File Size Err Name [%s] ", szFileName);
			inDISP_LogPrintfWithFlag("  File Size [%d]  TMSFLT File Size[%d] ", lnFileSize, lnTMSFLTSize);
			
			inDownloadStatus = VS_ERROR;
			inFILE_Delete((unsigned char *)szFileName);
		}
	}
	
	/* 若有下載失敗，回傳錯誤並刪FileList */
	if (inDownloadStatus == VS_ERROR)
	{
		inDISP_LogPrintfWithFlag("  TMS下載不完全，刪除FileList");
		inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_);

		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (VS_SUCCESS);
}

/*
Function        : inEDCTMS_ISO_DeleteAllDownloadFile
Date&Time   : 2019/10/5 下午 5:02
Describe        :下載檔案全刪
 * inEDCTMS_DeleteAllDownloadFile_ISO8583
*/
int inEDCTMS_ISO_DeleteAllDownloadFile()
{
	int	i = 0, j = 0, inFileNameLen = 0, inSlash = 0;
	char	szTemplate[60 + 1], szFileName[60 + 1];
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	if (inFILE_Check_Exist((unsigned char *)_TMSFLT_FILE_NAME_) != VS_SUCCESS)
	{
		inDISP_LogPrintfWithFlag("  NO TMSFLT FILE EXSIST  Line[%d] END", __LINE__);
		return (VS_ERROR);
	}
	
	for (i = 0;; i++)
	{
		/* Load TMS File List 取得檔名 */
		if (inLoadTMSFLTRec(i) < 0)
			break;

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSFilePathName(szTemplate);

		/* 取得檔案路徑後 抓取檔名儲存到FileName */
		for (j = 0; j < 60 ; j++)
		{
			if (szTemplate[j] == '/')
				inSlash = j + 1;

			if (szTemplate[j] == 0x00)
				break;
		}

		/* 斜線後的檔名長度*/
		inFileNameLen = j - inSlash;
		memset(szFileName, 0x00, sizeof(szFileName));
		memcpy(&szFileName[0], &szTemplate[inSlash], inFileNameLen);

		inFILE_Delete((unsigned char *)szFileName);
		
		inDISP_LogPrintfWithFlag("  刪除[%s] Line[%d]", szFileName, __LINE__);
	}
	

	inDISP_LogPrintfWithFlag("  開始刪除[%s] Line[%d]", _TMSFLT_FILE_NAME_, __LINE__);

	inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_);
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}


/*
Function        : inEDCTMS_UpdateParam
Date&Time   : 2016/1/6 下午 2:27
Describe        : 用在ISO下載時的檔案更新
 *  會放在 inEDCTMS_UpdateParam_Flow 使用 20190619 目前沒在用 [SAM]
*/
int inEDCTMS_ISO_UpdateParam(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	int		i = 0, j = 0, inFileNameLen = 0, inSlash = 0,  inRecord;
	char		szTemplate[60 + 1];
	char		szTMSAPPUpdateStatus[2 + 1];
	unsigned char   uszFileName[32 + 1];
//	unsigned long   ulFile_Handle; /* File Handle */
	VS_BOOL		fEMVCLDef1 = VS_FALSE, fEMVCLDef2 = VS_FALSE;
	VS_BOOL		fCFGT1 = VS_FALSE, fCFGT2 = VS_FALSE;
	VS_BOOL		fCDT1 = VS_FALSE, fCDT2 = VS_FALSE;
	VS_BOOL		fSCDT1 = VS_FALSE, fSCDT2 = VS_FALSE;
	VS_BOOL		fIPASS = VS_FALSE;
//	VS_BOOL		fECC = VS_FALSE, fICASH = VS_FALSE, fHAPPYCASH = VS_FALSE;	/* 用來標示Host有沒有開 */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數下載＞ */
	inDISP_PutGraphic(_TMS_UPDATING_, 0, _COORDINATE_Y_LINE_8_4_);
        
	
	/* 先檢查是否有【APPLICATION】要更新【1 = 需要更新AP】 */
	memset(szTMSAPPUpdateStatus, 0x00, sizeof(szTMSAPPUpdateStatus));
	inGetTMSAPPUpdateStatus(szTMSAPPUpdateStatus);
	if (memcmp(szTMSAPPUpdateStatus, "1", strlen("1")) == 0)
	{
		/* 更新完AP就會系統自動重開，所以後續動作先做 */
		inSetTMSAPPUpdateStatus("0");
		if (inSaveTMSCPTRec(0) < 0)
		{
			inResetTMSCPT_Schedule();
			inFunc_Reboot();
			return (VS_SUCCESS);
		}
		/* 等到參數轉換完才算完成 */
		inSetTMSOK("N");
		inSaveEDCRec(0);

		/* 重置紀錄TSAM已註冊的開關*/
		inSetTSAMRegisterEnable("N");
		inSaveEDCRec(0);

		/* 更新TMS，DCC狀態重置 */
		inSetDCCSettleDownload("0");
		inSaveEDCRec(0);
		
		inRetVal = inEDCTMS_FUNC_ProcessAutoUpdateAP();
		
		if (inRetVal == VS_SUCCESS)
		{
			inFunc_Reboot();
			
			return (VS_SUCCESS); /* 要先重新開機一次，不然更新參數檔會有問題 */
		}
		else
		{
			return (VS_ERROR);
		}
		
	}
        
	/* 需要先判斷要用哪個檔案來轉換 Config2.txt EMVCLDef2.txt */
	for (i = 0 ;; i++)
	{
		/* Load TMS File List 取得檔名 */
		if (inLoadTMSFLTRec(i) < 0)
			break;
            
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSFilePathName(szTemplate);
                
		/* 取得檔案路徑後 抓取檔名儲存到FileName */
		for (j = 0; j < 60 ; j++)
		{
			if (szTemplate[j] == '/')
				inSlash = j + 1;

			if (szTemplate[j] == 0x00)
				break;
		}
                
		inFileNameLen = j - inSlash;
		memset(uszFileName, 0x00, sizeof(uszFileName));
		memcpy(&uszFileName[0], &szTemplate[inSlash], inFileNameLen);
                
		if (!memcmp(&uszFileName[0], "EMVCLDef.txt", 12))
			fEMVCLDef1 = VS_TRUE;
		else if (!memcmp(&uszFileName[0], "EMVCLDef2.txt", 13))
			fEMVCLDef2 = VS_TRUE;
		else if (!memcmp(&uszFileName[0], "Config.txt", 10))
			fCFGT1 = VS_TRUE;
		else if (!memcmp(&uszFileName[0], "Config2.txt", 11))
			fCFGT2 = VS_TRUE;
		else if (!memcmp(&uszFileName[0], "CardDef.txt", 11))
			fCDT1 = VS_TRUE;
		else if (!memcmp(&uszFileName[0], "CardDef2.txt", 12))
			fCDT2 = VS_TRUE;
		else if (!memcmp(&uszFileName[0], "SpecCardDef.txt", 15))
			fSCDT1 = VS_TRUE;
		else if (!memcmp(&uszFileName[0], "SpecCardDef2.txt", 16))
			fSCDT2 = VS_TRUE;
        }
        
	if (fEMVCLDef2 == VS_TRUE && fEMVCLDef1 == VS_TRUE)
		fEMVCLDef1 = VS_FALSE;

	if (fCFGT2 == VS_TRUE && fCFGT1 == VS_TRUE)
		fCFGT1 = VS_FALSE;

	if (fCDT2 == VS_TRUE && fCDT1 == VS_TRUE)
		fCDT1 = VS_FALSE;

	if (fSCDT2 == VS_TRUE && fSCDT1 == VS_TRUE)
		fSCDT1 = VS_FALSE;
        
	for (i = 0 ;; i++)
	{       
		/* Load TMS File List 取得檔名 */
		if (inLoadTMSFLTRec(i) < 0)
			break;
                
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSFilePathName(szTemplate);

		/* 取得檔案路徑後 抓取檔名儲存到FileName */
		for (j = 0; j < 60 ; j++)
		{
			if (szTemplate[j] == '/')
				inSlash = j + 1;

			if (szTemplate[j] == 0x00)
				break;
		}
                
		inFileNameLen = j - inSlash;
		memset(uszFileName, 0x00, sizeof(uszFileName));
		memcpy(&uszFileName[0], &szTemplate[inSlash], inFileNameLen);
                
		/* 先確認檔案是否存在 避免誤刪檔案 */
		if (inFILE_Check_Exist(uszFileName) != VS_SUCCESS)
		{
			//inTMSFILE_WriteTmsTraceLog("%s_ERROR", uszFileName);
			continue;
		}

	
		inRecord = 0;

		/* 重新命名成我們使用的檔案名稱 */
		if (!memcmp(&uszFileName[0], "CardDef.txt", 11) && (fCDT1 == VS_TRUE))
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
				
		if (!memcmp(&uszFileName[0], "CardDef2.txt", 12) && (fCDT2 == VS_TRUE))
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
		else if ((!memcmp(&uszFileName[0], "Config.txt", 10)) && (fCFGT1 == VS_TRUE))
		{
			/* [檢查參數檔] 此檔案有初始設定，會由TMS下載檔案更新 2022/8/12 上午 10:07 [SAM] */
			inFILE_Delete((unsigned char *)"CFGT.dat");
			inFILE_Rename(uszFileName, (unsigned char *)"CFGT.dat");

			/* 計算有幾筆Record */
			for (j = 0 ;; j ++)
			{
				if (inLoadCFGTRec(j) < 0)
					break;

				inRecord ++;
			}
		}
		else if ((!memcmp(&uszFileName[0], "Config2.txt", 11)) && (fCFGT2 == VS_TRUE))
		{
			/* [檢查參數檔] 此檔案有初始設定，會由TMS下載檔案更新 2022/8/12 上午 10:07 [SAM] */
			inFILE_Delete((unsigned char *)"CFGT.dat");
			inFILE_Rename(uszFileName, (unsigned char *)"CFGT.dat");

			/* 計算有幾筆Record */
			for (j = 0 ;; j ++)
			{
				if (inLoadCFGTRec(j) < 0)
					break;

				inRecord ++;
			}
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
			/* [檢查參數檔] 此檔案有初始設定，會由TMS下載檔案更新 2022/8/12 上午 10:07 [SAM] */
			inFILE_Delete((unsigned char *)"HDT.dat");
			inFILE_Rename(uszFileName, (unsigned char *)"HDT.dat");

			/* 計算有幾筆Record */
			for (j = 0 ;; j ++)
			{
				if (inLoadHDTRec(j) < 0)
					break;

				inRecord ++;
			}
		}
		else if (!memcmp(&uszFileName[0], "CommDef.txt", 11))
		{
			/* [檢查參數檔] 此檔案有初始設定，會由TMS下載檔案更新 2022/8/12 上午 10:07 [SAM] */
			/* 備份CPT參數 */
			if (inEDCTMS_CPT_BackupCptFile() != VS_SUCCESS)
			{
				inDISP_LogPrintfWithFlag("CPT Backup Fail");
			}

			inFILE_Delete((unsigned char *)_CPT_FILE_NAME_);
			inFILE_Rename(uszFileName, (unsigned char *)_CPT_FILE_NAME_);

			/* 計算有幾筆Record */
			for (j = 0 ;; j ++)
			{
				if (inLoadCPTRec(j) < 0)
					break;

				inRecord ++;
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
		else if ((!memcmp(&uszFileName[0], "EMVCLDef.txt", 12)) && (fEMVCLDef1 == VS_TRUE))
		{
			/* [檢查參數檔] 此檔案有初始設定，會由TMS下載檔案更新 2022/8/12 上午 10:07 [SAM] */
			/* TODO: 要檢查一下,因為有二個檔  */
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
		else if ((!memcmp(&uszFileName[0], "EMVCLDef2.txt", 13)) && (fEMVCLDef2 == VS_TRUE))
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
		else if (!memcmp(&uszFileName[0], "SpecCardDef.txt", 15) && fSCDT1 == VS_TRUE)
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
		else if (!memcmp(&uszFileName[0], "SpecCardDef2.txt", 16) && fSCDT2 == VS_TRUE)
		{
			/* [檢查參數檔] 此檔案由TMS下載產生，沒有初始檔案  2022/8/12 上午 10:07 [SAM] */
			inFILE_Delete((unsigned char *)"SCDT.dat");
			inFILE_Rename(uszFileName, (unsigned char *)"SCDT.dat");

			/* 計算有幾筆Record */
			for (j = 0 ;; j ++)
			{
				if (inLoadASMCRec(j) < 0)
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
			inFILE_Delete((unsigned char *)"NCCCLOGO.bmp");
			inFILE_Rename(uszFileName, (unsigned char *)"NCCCLOGO.bmp");
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
		else
		{
			inFILE_Delete(uszFileName);
		}
	}
	
	/* 刪除所有Host Table */
	inFunc_DeleteAllBatchTable();
	
	/* 初始化HDPT */
	inEDCTMS_HDPT_InitialFileValue();
	
	/* 更新EMV參數 */
	inEMVXML_Create_EMVConfigXml();
	inFunc_Delete_Data("", _EMV_CONFIG_FILENAME_, _EMV_EMVCL_DIR_NAME_);
	inFunc_Make_Dir(_EMV_EMVCL_DIR_NAME_, _FS_DATA_PATH_);
	inFunc_Move_Data(_EMV_CONFIG_FILENAME_, _AP_ROOT_PATH_, "", _EMV_EMVCL_DATA_PATH_);

	/* 更新CTLS感應參數 */
	inEMVXML_Create_CTLSConfigXml();
	inFunc_Delete_Data("", _EMVCL_CONFIG_FILENAME_, _EMV_EMVCL_DIR_NAME_);
	inFunc_Make_Dir(_EMV_EMVCL_DIR_NAME_, _FS_DATA_PATH_);
	inFunc_Move_Data(_EMVCL_CONFIG_FILENAME_, _AP_ROOT_PATH_, "", _EMV_EMVCL_DATA_PATH_);
        
	/* 初始化TDT */
	inEDCTMS_TDT_ReInitialTicketTable();
	/* 初始化票證參數 */
	if (fIPASS == VS_TRUE)
	{
		inEDCTMS_FUNC_ReInitialIPASSTable();
	}
	
	
	                  
        /* 更新完成刪除File List */
        inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_);

        inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
        
        return (VS_SUCCESS);        
}


