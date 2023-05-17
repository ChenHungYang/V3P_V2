#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include "curl/curl.h"
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/Transaction.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DispMsg.h"
#include "../DISPLAY/DisTouch.h"
#include "../PRINT/Print.h"
#include "../PRINT/PrtMsg.h"

#include "../../SOURCE/FUNCTION/CFGT.h"

#include "../FUNCTION/Function.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/FILE_FUNC/File.h"

#include "EDCTmsDefine.h"
#include "EDCTmsFileProc.h"

#include "TMSTABLE/TmsFTP.h"
#include "TMSTABLE/EDCtmsFTPFLT.h"

#include "EDCTmsFtpFunc.h"
#include "EDCTmsScheduleFunc.h"
#include "EDCTmsPrintFunc.h"
#include "EDCTmsAnalyseFunc.h"



/*
Function        :  inEDCTMS_ANALYSE_ResultHandle
Date&Time   :  2019/12/6 下午 4:41
Describe        :  用在人工下載TMS的結果分析,使用在OPT上，所以都回傳VS_SUCCESS
  */
int inEDCTMS_ANALYSE_ResultHandle(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	if(pobTran->inTMSDwdMode == _EDC_TMS_MANUAL_PARM_AP_DLD_MOD_ || 
	    pobTran->inTMSDwdMode == _EDC_TMS_MANUAL_PARM_DLD_MOD_)
	{
		inDISP_DispLogAndWriteFlie("  TMS Print Parameter Start  LINE[%d]", __LINE__);
		/* 檢查檔案Filelist 是否有下載完成，之後再列印帳單 */
		if (inFILE_Check_Exist((unsigned char *)_EDC_FTPFLT_FILE_NAME_) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  TMS Print Check  FN[%s] *Error* LINE[%d]",_EDC_FTPFLT_FILE_NAME_, __LINE__);
			return (VS_SUCCESS);
		}else{

			/* 印出下載參數表 */
			if (inEDCTMS_PRINT_PrintDownloadStatus(pobTran) != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie("  TMS Print FN[%s] *Error* LINE[%d]",_EDC_FTPFLT_FILE_NAME_, __LINE__);
				return (VS_SUCCESS);
			}
		}
	}

	inRetVal = inEDCTMS_FTP_FtpsUpdateFinalParameter(pobTran);

	if (inRetVal == VS_SUCCESS)
	{
		/* TMS更新成功 */
		inSetTMSOK("Y");
//		/* 重置紀錄TSAM已註冊的開關*/
		inSetTSAMRegisterEnable("N");

		/* 更新TMS，DCC狀態重置 */
		inSetDCCSettleDownload("0");
		inSaveEDCRec(0);
		
		/* 開啟銀聯及電簽功能 20190213 [SAM] */
		inFunc_FUBON_Check_Linkage_Function_Enable(pobTran);
		

		/* 重設時間 */
		inEDCTMS_SaveSysDownloadDateTime(pobTran);
//		inSetFTPInquireStartDate("00000000");
//		inSetFTPInquireStartTime("000000");
		
		/* 初始記錄狀態，用在回報時判斷  2020/1/14 下午 3:32 [SAM] */
		inSetFTPAutoDownloadFlag("00");
		
		/* 使用 TmsFPT 
		 * 目前設定參數更新完成都要回報，如ISO格式無法回報需要修改 2019/12/4 上午 9:53 [SAM] */
		inSetFTPEffectiveReportBit("Y");
		inTMSFTP_SetDowloadResponseCode(_FTP_DOWNLOAD_REPORT_EFFECT_SUCCESS_);
		if (inSaveTMSFTPRec(0) < 0)
		{
			inDISP_DispLogAndWriteFlie("  Save TMSFPT *Error* Line [%d] END -----", __LINE__);
		}
		
		inDISP_DispLogAndWriteFlie("  TMS Update Success Line[%d]", __LINE__);

		/* 更新參數完重開機 */
		inFunc_Reboot();
	}
	else
	{
		inDISP_DispLogAndWriteFlie("  TMS UpDate Fail Line[%d]", __LINE__);
		/* 更新失敗 刪除File List */
		inFILE_Delete((unsigned char *)_EDC_FTPFLT_FILE_NAME_);
		inSetTMSOK("N");
		inSaveEDCRec(0);                     
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
	
}

/*
Function        :  inEDCTMS_ANAL_ResultHandle
Date&Time   :  2016/1/6 下午 2:27
Describe        :  判斷 TMS 更新日期
 *	因為 FUNCTION 會使用在OPT,所以回傳值只能回傳成功，否則會影響到後續的流程
 * inEDCTMS_FuncResultHandle
*/
int inEDCTMS_ANALYSE_SysconfigData(TRANSACTION_OBJECT *pobTran)
{
	
	if (inFILE_Check_Exist((unsigned char *)_TMS_SYSCONFIG_NAME_) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  TMS ANALSYE Check  FN[%s] *Error* LINE[%d]", _TMS_SYSCONFIG_NAME_, __LINE__);
		return (VS_SUCCESS);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :  inEDCTMS_ANALYSE_EsunResultHandle
Date&Time   :  2021/5/21 上午 9:35
Describe        :  這Function只會用在玉山版本上，用在人工下載TMS的結果分析,使用在OPT上，所以都回傳VS_SUCCESS
  */
int inEDCTMS_ANALYSE_EsunResultHandle(TRANSACTION_OBJECT *pobTran)
{

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
#if (_MACHINE_TYPE_ != _CASTLE_TYPE_UPT1000_)
	int inRetVal;    
    
	if(pobTran->inTMSDwdMode == _EDC_TMS_MANUAL_PARM_AP_DLD_MOD_ || 
	    pobTran->inTMSDwdMode == _EDC_TMS_MANUAL_PARM_DLD_MOD_)
	{
		inDISP_DispLogAndWriteFlie("  TMS Print Parameter Start  LINE[%d]", __LINE__);
		/* 檢查檔案Filelist 是否有下載完成，之後再列印帳單 */
		if (inFILE_Check_Exist((unsigned char *)_EDC_FTPFLT_FILE_NAME_) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  TMS Print Check  FN[%s] *Error* LINE[%d]",_EDC_FTPFLT_FILE_NAME_, __LINE__);
			return (VS_SUCCESS);
		}else{

			/* 印出下載參數表 */
			if (inEDCTMS_PRINT_PrintDownloadStatus(pobTran) != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie("  TMS Print FN[%s] *Error* LINE[%d]",_EDC_FTPFLT_FILE_NAME_, __LINE__);
				return (VS_SUCCESS);
			}
		}
	}

	inRetVal = inEDCTMS_FTP_FtpsUpdateFinalParameter(pobTran);

	if (inRetVal == VS_SUCCESS)
	{
		/* TMS更新成功 */
		inSetTMSOK("Y");
                   /* 重置紀錄TSAM已註冊的開關*/
		inSetTSAMRegisterEnable("N");
		
		inSetESCMode("N");

		/* 更新TMS，DCC狀態重置 */
		inSetDCCSettleDownload("0");
		inSaveEDCRec(0);
		
		/* 開啟銀聯及電簽功能 20190213 [SAM] */
		inFunc_FUBON_Check_Linkage_Function_Enable(pobTran);

		/* 重設時間 */
		inEDCTMS_SaveSysDownloadDateTime(pobTran);
		
		/* 初始記錄狀態，用在回報時判斷  2020/1/14 下午 3:32 [SAM] */
		inSetFTPAutoDownloadFlag("00");
		
		/* 使用 TmsFPT 
		 * 目前設定參數更新完成都要回報，如ISO格式無法回報需要修改 2019/12/4 上午 9:53 [SAM] */
		inSetFTPEffectiveReportBit("Y");
		inTMSFTP_SetDowloadResponseCode(_FTP_DOWNLOAD_REPORT_EFFECT_SUCCESS_);
		if (inSaveTMSFTPRec(0) < 0)
		{
			inDISP_DispLogAndWriteFlie("  Save TMSFPT *Error* Line [%d] END -----", __LINE__);
		}
		
		inDISP_DispLogAndWriteFlie("  TMS Update Success Line[%d]", __LINE__);

		/* 更新參數完重開機 */
		inFunc_Reboot();
	}
	else
	{
		inDISP_DispLogAndWriteFlie("  TMS UpDate Fail Line[%d]", __LINE__);
		/* 更新失敗 刪除File List */
		inFILE_Delete((unsigned char *)_EDC_FTPFLT_FILE_NAME_);
		inSetTMSOK("N");
		inSaveEDCRec(0);                     
	}
#else
        /* 因為玉山的UPT TMS目前不更新參數只更新AP，只要下載AP成功就算完成，所以只要回報就好，
         * 如果要改回更新參數再參考上面程式碼進行修改 2021/5/26 下午 3:12 [SAM]*/
   
        inSetFTPEffectiveReportBit("Y");
        inTMSFTP_SetDowloadResponseCode(_FTP_DOWNLOAD_REPORT_EFFECT_SUCCESS_);
        if (inSaveTMSFTPRec(0) < 0)
        {
            inDISP_DispLogAndWriteFlie("  Save TMSFPT *Error* Line [%d] END -----", __LINE__);
        }

        inDISP_DispLogAndWriteFlie("  TMS Update Success Line[%d]", __LINE__);

        /* 更新參數完重開機 */
        inFunc_Reboot();
    
#endif	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
	
}


