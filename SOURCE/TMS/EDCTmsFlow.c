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

#include "../EVENT/MenuMsg.h"
#include "../EVENT/Flow.h"

#include "../FUNCTION/Function.h"
#include "../FUNCTION/Sqlite.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/HDPT.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/CPT.h"
#include "../FUNCTION/FuncTable.h"
#include "../FUNCTION/FILE_FUNC/File.h"

#include "../COMM/Ftps.h"
#include "../COMM/Comm.h"

#include "TMSTABLE/EDCtmsFTPFLT.h"
#include "TMSTABLE/TmsCPT.h"
#include "TMSTABLE/TmsFTP.h"
#include "TMSTABLE/TmsSCT.h"


#include "EDCTmsDefine.h"
#include "EDCTmsUnitFunc.h"
#include "EDCTmsScheduleFunc.h"
#include "EDCTmsFtpFunc.h"
#include "EDCTmsAnalyseFunc.h"
#include "EDCTmsIsoFunc.h"
#include "EDCTmsPrintFunc.h"
#include "EDCTmsFileProc.h"
#include "EDCTmsFtpFunc.h"

#include "EDCTmsFlow.h"

/*
Function        : inEDCTMS_FuncSelectFlow
Date&Time   : 2019/10/1 下午 4:02
Describe        : 使用功能鍵人工下載 TMS，下載分流 ISO8583或FTPS
 *  原來的名稱 inEDCTMS_TMS_Func5SelectFlow
*/
int inEDCTMS_MenuForUserSetTmsParameterFlow(TRANSACTION_OBJECT *pobTran)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 先LOAD 預設的 HOST */
	inLoadHDTRec(0);
	inLoadHDPTRec(0);
	/* TMS的CPT參數 */
	inLoadTMSCPTRec(0);
	/* EDC參數 */
	inLoadEDCRec(0);
	/* CPT參數 */
	inLoadCPTRec(0);
	/* FTP參數 */
	inLoadTMSFTPRec(0);

	pobTran->inTransactionCode = _EDCTMS_MANUAL_DOWNLOAD_; /* 先Hard code */
	pobTran->inTMSDwdMode = _EDC_TMS_AUTO_PARM_DLD_MOD_; /* 手動下載 */
	
        	/* 輸入端末機代號 */
	if (inEDCTMS_FUNC_GetTID(pobTran) != VS_SUCCESS)
		return (VS_ERROR);
        
	/* 輸入商店代號 */
	if (inEDCTMS_FUNC_GetMID(pobTran) != VS_SUCCESS)
		return (VS_ERROR);

	/* 通訊模式分流, 會決定是否用 FTP 還是 ISO 方式下載  */
	if (inEDCTMS_FUNC_SetCommParm(pobTran) != VS_SUCCESS)
		return (VS_ERROR);
	
//	/* 重新設定自動下載時間 */
//	if (inEDCTMS_SCHEDULE_ResetDownloadDateTime(pobTran) != VS_SUCCESS)
//		return (VS_ERROR);
	
//	if (inEDCTMS_DeleteCompareVersionDateFile() != VS_SUCCESS)
//		return (VS_ERROR);
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}


/*
*Function        : inEDCTMS_IdleUpdateParamData
*Date&Time   : 2019/10/4 上午 11:26
*Describe        : 在IDLE時更新參數，需參考參數生效(看參數生效日) 
 * 2021/5/26 下午 5:24 目前沒在用 [SAM]
	inEDCTMS_IdleUpdate
*/
int inEDCTMS_IdleUpdateParamDataFlow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	
	inRetVal = inEDCTMS_CheckTmsFileListExistFlow(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	inRetVal = inEDCTMS_UpdateParamFlowForIdleUpdate(pobTran);

	if (inRetVal == VS_SUCCESS)
	{
		inSetTMSOK("Y");
		inSaveEDCRec(0);
	}
	else
	{
		/* 更新失敗 刪除File List */
		inEDCTMS_DeleteTmsFileListExistFlow(pobTran);
		inSetTMSOK("N");
		/* 端末機資料錯誤請報修 */
		inDISP_Clear_Line(_LINE_8_3_, _LINE_8_8_);
		inDISP_Msg_BMP(_ERR_EDC_LOCK_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, 30, "", 0);

		inFunc_EDCLock();
		inSaveEDCRec(0);
	}
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
Function        : inEDCTMS_UpdateParamFlowForIdleUpdate
Date&Time   : 2019/10/4 上午 11:30
Describe        : 檔案更新分流  
 * 2021/5/26 下午 5:24 目前沒在用 [SAM]
 *  inEDCTMS_UpdateParam_Flow
*/
int inEDCTMS_UpdateParamFlowForIdleUpdate(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
		
	if (pobTran->uszFTP_TMS_Download == 'Y')
	{
		inDISP_LogPrintfWithFlag("  UPDATE CHOOSE FTPS");
		inRetVal = inEDCTMS_FTP_FtpsUpdateFinalParameter(pobTran);
	}
	else
	{
		inDISP_LogPrintfWithFlag("  UPDATE CHOOSE ISO8583");
		inRetVal = inEDCTMS_ISO_UpdateParam(pobTran);
	}
	
	if (inRetVal == VS_SUCCESS)
	{
		/* TMS更新成功 */
		inSetTMSOK("Y");

		/* 重置紀錄TSAM已註冊的開關*/
		inSetTSAMRegisterEnable("N");

		inSaveEDCRec(0);
		
		/* 開啟銀聯及電簽功能 20190213 [SAM] */
		inFunc_FUBON_Check_Linkage_Function_Enable(pobTran);
		
		inEDCTMS_SaveSysDownloadDateTime(pobTran);
		/* 初始記錄狀態，用在回報時判斷  2020/1/14 下午 3:32 [SAM] */
		inSetFTPAutoDownloadFlag("00");
		
		/* 目前設定參數更新完成都要回報，如ISO格式無法回報需要修改 2019/12/4 上午 9:53 [SAM] */			
		inSetFTPEffectiveReportBit("Y");
		inTMSFTP_SetDowloadResponseCode(_FTP_DOWNLOAD_REPORT_EFFECT_SUCCESS_);
		if (inSaveTMSFTPRec(0) < 0)
		{
			inDISP_DispLogAndWriteFlie("  Save TMSFPT *Error* Line [%d] END -----", __LINE__);
		}
#if 0
		/* 富邦目前沒有SMART PAY，所以先刪除不用 20190108 [SAM]  */
		/* 確認是否接受CUP、SMARTPAY */
		inFunc_Check_Linkage_Function_Enable(pobTran);

		/* TMS連動DCC下載檢查，開機後根據DCCDownloadMode立即DCC下載 */
		if (inNCCC_DCC_TMS_Schedule_Check(pobTran) == VS_SUCCESS)
		{
			inSetDCCDownloadMode(_NCCC_DCC_DOWNLOAD_MODE_NOW_);
			inSaveEDCRec(0);
		}

		/* 更新TMS，DCC狀態重置 */
		inSetDCCSettleDownload("0");
		inSaveEDCRec(0);
#endif
		/* 更新參數完重開機 */
		inFunc_Reboot();
	}
	else
	{
		/* 更新失敗 刪除File List */
		inFILE_Delete((unsigned char *)_EDC_FTPFLT_FILE_NAME_);
		inSetTMSOK("N");
		inSaveEDCRec(0);                     
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (inRetVal);
}


/*
Function        : inEDCTMS_ScheduleInquireFlow
Date&Time   : 2019/12/6 下午 3:51
Describe        : TMS開機詢問檢查
 *  目前回傳值都會是 VS_SUCCESS 不然在TRT中使用會中斷後續流程
 *  原來使用的名稱 inEDCTMS_Schedule_Inquire
*/
int inEDCTMS_ScheduleInquireFlow(TRANSACTION_OBJECT *pobTran)
{
/* 沒連線能力 */
#ifndef _COMMUNICATION_CAPBILITY_
	return (VS_SUCCESS);
#endif
	
	char szDemoMode[4];
	char    szTemplate[16 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{	
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] DEMO END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}
	
	/* 先查看是哪一種下載模式，
	 * 若為ISO8583，由結帳電文帶下，
	 * 若為FTP，則要下載FileList看是否有差異
	 */
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inGetTMSDownloadMode(szTemplate);
//	
//	inDISP_DispLogAndWriteFlie("  Dowload Mode[%s] Line[%d]", szTemplate, __LINE__);
//	
//	/* ISO8583*/
//	if (memcmp(szTemplate, _TMS_DLMODE_ISO8583_, strlen(_TMS_DLMODE_ISO8583_)) == 0)
//	{
//		/* 設定不是FTP 下載*/
//		pobTran->uszFTP_TMS_Download = 'N';
//	}
//	else if (memcmp(szTemplate, _TMS_DLMODE_FTPS_, strlen(_TMS_DLMODE_FTPS_)) == 0)
//	{
//		/* 設定是FTP 下載*/
//		pobTran->uszFTP_TMS_Download = 'Y';		
//	}
//	else
//	{
//		inDISP_LockEdcDispLogAndWriteFlie("  找不到此TMS下載模式 :%s", szTemplate);
//		return (VS_ERROR);
//	}

	/* 固定讀入第0個 主機  */
	if (inLoadCPTRec(0) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie("  Sysconfig Load CPT *Error* Line[%d]",  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}

	/* 固定讀入第0個記錄  */
	if (inLoadTMSFTPRec(0) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie("  Sysconfig Load TMSFTP *Error* Line[%d]",  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}
	
	/* 目前是固定用FTP下載  */
	pobTran->uszFTP_TMS_Download = 'Y';
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetCommMode(szTemplate);
	if (memcmp(szTemplate, _COMM_ETHERNET_MODE_, strlen(_COMM_ETHERNET_MODE_)))
	{
		inDISP_DispLogAndWriteFlie("  Sysconfig Dowload Mode[%s] *Error* Line[%d]", szTemplate, __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return VS_SUCCESS;
	}
	
	/* 先下載 Sysconfig.txt 檔 */
	if(inEDCTMS_DowloadSpconfigFileFlow(pobTran) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Sysconfig Dowload *Error* Line[%d]", __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 因為自動下載時卡機已有參數，如下載失敗可先不需理會 */
		return (VS_SUCCESS);
	}
	
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        : inEDCTMS_DownloadTmsDataAfterSettle
Date&Time   : 2019/10/31 下午 5:55
Describe        : 使用在 結帳後TMS下載或是 開機時下載參數及AP
 *  因為詢問失敗還是可以進行交易
 * inEDCTMS_Download_Settle
*/
int inEDCTMS_DownloadTmsDataAfterSettle(TRANSACTION_OBJECT *pobTran)
{

/* 目前只有國泰需要進行AP更新及自動參數更新 2019/12/9 [SAM] */

	return (VS_SUCCESS);
}


/*
Function        : inEDCTMS_ReturnTheTmsReport
Date&Time   : 2016/2/2 下午 2:32
Describe        : TMS參數生效回報，目前只加入 FTPS 上傳功能，如要在OPT或分流使用需要再修改
 * inEDCTMS_TMS_Return_Report
*/
int inEDCTMS_ReturnTheTmsReport(TRANSACTION_OBJECT *pobTran)
{
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 參數生效回報 */
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_TMS_REPORT_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);			/* 第三層顯示 參數生效回報 */
	pobTran->inTransactionCode = _EDCTMS_TRACE_LOG_;
	pobTran->inTMSDwdMode = _EDC_TMS_REPORT_SEND_MOD_;			/* 自動下載 */
	pobTran->uszFTP_TMS_Download = 'Y';	 /* 公司TMS回報都用 FTP模式 */

/* 玉山UPT回報重新寫 2021/5/26 下午 3:36 [SAM]  */
#if defined(_ESUN_MAIN_HOST_)  && (_MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_)    
        inEDCTMS_FTP_FtpsEsunUpload(pobTran); 
        
//        inEDCTMS_FTP_FtpsEsunFileLogUpload(pobTran); 
#else    
	/*  目前不看參數直接用 FTPS 模式上傳  */
	inEDCTMS_FTP_FtpsUpload(pobTran); 
#endif
	
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}



/*
Function        : inEDCTMS_CheckAllDownloadFileMethodFlow
Date&Time   : 2017/1/25 下午 2:44
Describe        : 確認下載檔案方式是ISO還是FTPS的分流
 * inEDCTMS_CheckAllDownloadFile_Flow
*/
int inEDCTMS_CheckAllDownloadFileMethodFlow(TRANSACTION_OBJECT * pobTran)
{
	int	inRetVal;
	
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* FTPS */
	if (pobTran->uszFTP_TMS_Download == 'Y')
	{
		inDISP_LogPrintfWithFlag("  inEDCTMS_CheckAllDownloadFile Choose FTPS");
		
		inRetVal = inEDCTMS_FTP_FtpsCheckAllDownloadFileList();
	}
	/* ISO8583 */
	else
	{
		inDISP_LogPrintfWithFlag("  inEDCTMS_CheckAllDownloadFile Choose ISO8583");

		inRetVal = inEDCTMS_ISO_CheckAllDownloadFile();
	}
	
	inDISP_DispLogAndWriteFlie("  FinalResult [%d] DldMode [%s]", inRetVal ,  pobTran->uszFTP_TMS_Download );
	
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (inRetVal);
}

/*
Function        : inEDCTMS_DeleteAllDownloadFileMethodFlow
Date&Time   : 2017/1/25 下午 4:40
Describe        : 確認刪除檔案的方式是ISO還是FTPS的分流
 * inEDCTMS_DeleteAllDownloadFile_Flow
*/
int inEDCTMS_DeleteAllDownloadFileMethodFlow(TRANSACTION_OBJECT * pobTran)
{	
	int	inRetVal = VS_SUCCESS;
	
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* FTPS */
	if (pobTran->uszFTP_TMS_Download == 'Y')
	{
		inDISP_LogPrintfWithFlag(" inEDCTMS_DeleteAllDownloadFile_Flow Choose FTPS");
		inRetVal = inEDCTMS_FTP_FtpsDeleteAllDownloadFileList();
	}
	else
	{
		inDISP_LogPrintfWithFlag("  inEDCTMS_DeleteAllDownloadFile_Flow Choose ISO8583");
		inRetVal = inEDCTMS_ISO_DeleteAllDownloadFile();
	}
	
	inDISP_DispLogAndWriteFlie("  Del Tms All Data Result [%d] ", inRetVal );
	
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (inRetVal);
}


/*
Function        : inEDCTMS_CheckTmsFileListExistFlow
Date&Time   : 2017/1/25 下午 2:53
Describe        : 確認FileList，通訊分流，檢查存在哪一個FileList，就切成哪一種，
 *  這是為了避免設定為CFGT設定FTPS卻跑ISO8583流程，唯一有疑慮為若有FileList沒清乾淨會跑錯流程
 * inEDCTMS_Check_FileList_Flow
*/
int inEDCTMS_CheckTmsFileListExistFlow(TRANSACTION_OBJECT * pobTran)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* ISO8583 */
	if (inFILE_Check_Exist((unsigned char *)_TMSFLT_FILE_NAME_) == VS_SUCCESS)
	{
		pobTran->uszFTP_TMS_Download = 'N';
		inDISP_LogPrintfWithFlag("  inEDCTMS_Check_FileList Choose ISO8583");
	}
	/* FTPS */
	else if (inFILE_Check_Exist((unsigned char *)_EDC_FTPFLT_FILE_NAME_) == VS_SUCCESS)
	{
		pobTran->uszFTP_TMS_Download = 'Y';
		inDISP_LogPrintfWithFlag("  inEDCTMS_Check_FileList Choose FTPS");
	}
	else
	{
		inDISP_LogPrintfWithFlag("  inEDCTMS_Check_FileList No Filelist");
		inRetVal = VS_ERROR;
	}
	
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (inRetVal);
}

/*
Function        : inEDCTMS_Delete_FileList_Flow
Date&Time   : 2017/1/25 下午 2:53
Describe        : 刪除FileList，通訊分流
 * inEDCTMS_Delete_FileList_Flow
*/
int inEDCTMS_DeleteTmsFileListExistFlow(TRANSACTION_OBJECT * pobTran)
{
	int	inRetVal;

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* FTPS */
	if (pobTran->uszFTP_TMS_Download == 'Y')
	{
		inDISP_LogPrintfWithFlag("  inEDCTMS_Delete_FileList_Flow Choose FTPS");
		inRetVal = inFILE_Delete((unsigned char *)_EDC_FTPFLT_FILE_NAME_);
	}
	else
	{
		inDISP_LogPrintfWithFlag("  inEDCTMS_Delete_FileList_Flow Choose ISO8583");
		inRetVal = inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_);
	}
	
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}


/*
Function        : inEDCTMS_FuncSendReceive_Flow
Date&Time   : 2018/6/28 下午 2:12
Describe        : 決定使用ISO或FTPS方式下載的分流，目前使用在OPT上
 * inEDCTMS_FuncSendReceive_Flow
*/

int inEDCTMS_SendAndReceiveFlow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	if (pobTran->uszFTP_TMS_Download == 'Y')
	{
		inRetVal = inEDCTMS_FTP_FtpsSendReceiveFlowByTransCode(pobTran);
		inDISP_LogPrintfWithFlag("  Run SendReceiveWithFTPS Reult [%d]",inRetVal );
	}
	else
	{
		//inRetVal = inEDCTMS_ISO_UpdateParam(pobTran);
		inDISP_LogPrintfWithFlag("  Run SendReceive Reult [%d]",inRetVal );
	}

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
	return (inRetVal);
}

/*
Function        : inEDCTMS_DowloadSpconfigFileFlow
Date&Time   : 2019/12/8 下午 3:17
Describe        : 下載TMS Sysconfig ，目前只使用FTP，如有別的模式可在此分流
 * 目前不用判斷條件，因為 TmsSCT 檔案只要每次下載都會更新 
*/

int inEDCTMS_DowloadSpconfigFileFlow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
		
	if (pobTran->uszFTP_TMS_Download == 'Y')
	{
		/* 初始化curl庫 */
		if (inFTPS_Initial(pobTran) != VS_SUCCESS)
		{
			inDISP_Msg_BMP(_ERR_CONNECT_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, 30, "", 0);
			inDISP_DispLogAndWriteFlie("  Dld Spconfig FTPS INIT *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		}

		inRetVal = inEDCTMS_FTP_FtpsDownloadSpconfig(pobTran);
	
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Dld Spconfig File *Error [%d] Line[%d]", inRetVal, __LINE__);
			/* 不管成功或失敗都要DeInitial */
			inRetVal = inFTPS_Deinitial(pobTran);
			if (inRetVal != VS_SUCCESS)
				inDISP_DispLogAndWriteFlie("  Dld SpconfigFile Deinital Fail [%d] Line[%d] ", inRetVal, __LINE__);
			return (VS_ERROR);
		}
		
		/* 不管成功或失敗都要DeInitial */
		inRetVal = inFTPS_Deinitial(pobTran);
		if (inRetVal != VS_SUCCESS)
			inDISP_DispLogAndWriteFlie("  Dld SpconfigFile Deinital Fail [%d] Line[%d] ", inRetVal, __LINE__);

		/* 刪除 TmsSCT  */
		inRetVal = inFILE_Delete((unsigned char *)_TMSSCT_FILE_NAME_);
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Dld Spconfig DelFile *Error* Fn[%d] Line[%d]", _TMSSCT_FILE_NAME_, __LINE__);
			return (VS_ERROR);
		}
		
		/* 用下載檔 sysconfig 整個取代 TmsSCT */
		inRetVal = inFILE_Rename((unsigned char *)_TMS_SYSCONFIG_NAME_, (unsigned char *)_TMSSCT_FILE_NAME_);
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Dld Spconfig  Rename *Error* Fn[%d] Line[%d]", _TMSSCT_FILE_NAME_, __LINE__);
			return (VS_ERROR);
		}
	}
	else
	{
		inDISP_DispLogAndWriteFlie(" Dld Sconfig *Error* Not Ftp Mode[%d] Line[%d]",pobTran->uszFTP_TMS_Download, __LINE__);
		return (VS_ERROR);
	}

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (VS_SUCCESS);
}




