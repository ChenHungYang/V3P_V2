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
#include "../FUNCTION/Accum.h"
#include "../FUNCTION/Function.h"
#include "../FUNCTION/FuncTable.h"
#include "../FUNCTION/Sqlite.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/CDT.h"
#include "../FUNCTION/CPT.h"
#include "../FUNCTION/CPT_Backup.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/HDPT.h"
#include "../FUNCTION/PWD.h"
#include "../FUNCTION/MVT.h"
#include "../FUNCTION/VWT.h"
#include "../FUNCTION/EST.h"
#include "../FUNCTION/SKM.h"
#include "../FUNCTION/QAT.h"
#include "../FUNCTION/FILE_FUNC/File.h"
#include "../FUNCTION/FILE_FUNC/FIleLogFunc.h"
#include "../FUNCTION/IPASSDT.h"
#include "../FUNCTION/TDT.h"
#include "../FUNCTION/ECCDT.h"
#include "../FUNCTION/ASMC.h"
#include "../FUNCTION/PCD.h"
#include "../FUNCTION/PIT.h"
#include "../FUNCTION/SCDT.h"

#include "../EVENT/MenuMsg.h"
#include "../EVENT/Flow.h"

#include "../COMM/Comm.h"
#include "../COMM/Ethernet.h"
#include "../COMM/Modem.h"
#include "../COMM/Ftps.h"
#include "../COMM/TLS.h"

#include "../../EMVSRC/EMVxml.h"
#include "../../EMVSRC/EMVsrc.h"

#include "../../CTLS/CTLS.h"

#include "../FUNCTION/AccountFunction/PrintBillInfo.h"

#include "TMSTABLE/TmsFTP.h"
#include "TMSTABLE/TmsCPT.h"
#include "TMSTABLE/EDCtmsFTPFLT.h"

#include "EDCTmsDefine.h"

#include "EDCTmsFileProc.h"
#include "EDCTmsUnitFunc.h"

extern	char	gszTermVersionID[15 + 1];
extern	char	gszTermVersionDate[16 + 1];

/*  因為要選擇，如果不是使用嘉利的TMS就使用固定的IP 值 2019/10/8 [SAM]  */
static int st_inUseFtpId = 0;


int inEDCTMS_FUNC_ChooseManualTmsMode(TRANSACTION_OBJECT *pobTran)
{
	char szKey;
	char szTemp[4] = {0};
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);	
	
	inDISP_ChineseFont("TMS下載模式：", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
	inDISP_ChineseFont("1: AP、參數下載", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
	inDISP_ChineseFont("2: 參數下載", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
	
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 15);
	
	while (1)
	{
		szKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		if (szKey == _KEY_TIMEOUT_ || szKey == _KEY_CANCEL_)
		{
			return (VS_ERROR);
		}
		else if (szKey == _KEY_1_)
		{
			pobTran->inTransactionCode = _EDCTMS_MANUAL_DOWNLOAD_;
			pobTran->inTMSDwdMode = _EDC_TMS_MANUAL_PARM_AP_DLD_MOD_;
			inDISP_DispLogAndWriteFlie(" K1 T_Code[%d] T_DL_MODE[%d]",pobTran->inTransactionCode, pobTran->inTMSDwdMode);
			break;
		}
		else if (szKey == _KEY_2_)
		{
			pobTran->inTransactionCode = _EDCTMS_MANUAL_DOWNLOAD_;
			pobTran->inTMSDwdMode = _EDC_TMS_MANUAL_PARM_DLD_MOD_;
			inDISP_DispLogAndWriteFlie(" K2 T_Code[%d] T_DL_MODE[%d]",pobTran->inTransactionCode, pobTran->inTMSDwdMode);
			break;
		}

	}/* 重新初始化迴圈 */
	
	/* 只有人工TMS要先記錄狀態，在FTP回報時判斷  2020/1/14 下午 3:32 [SAM] */
	sprintf(szTemp, "%02d", pobTran->inTMSDwdMode);
	inSetFTPAutoDownloadFlag(szTemp);
	if((VS_SUCCESS != inSaveTMSFTPRec(0))) 
	{
		inDISP_DispLogAndWriteFlie("  Manual Tms Save TMSFTP *Eerror* ID[0]  Line [%d]", __LINE__);
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}



/*
Function        : inEDCTMS_FUNC_GetTID
Date&Time   : 2019/10/1 下午 4:05
Describe        : 輸入Terminal ID
 * 原來的名稱 inEDCTMS_TID_GET
*/
int inEDCTMS_FUNC_GetTID(TRANSACTION_OBJECT *pobTran)
{
	char	szTerminalID[16 + 1];
	int	inRetVal = 0;
	DISPLAY_OBJECT  srDispObj;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_TMS_ENTER_TID_, 0, _COORDINATE_Y_LINE_8_4_);

	/* 顯示目前的TID */
	memset(szTerminalID, 0x00, sizeof(szTerminalID));
	inGetTerminalID(szTerminalID);
	inFunc_PAD_ASCII(szTerminalID, szTerminalID, ' ', 16, _PADDING_LEFT_);
	inDISP_EnglishFont(szTerminalID, _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
        
	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	srDispObj.inMaxLen = 8;
	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inColor = _COLOR_RED_;

	inRetVal = inDISP_Enter8x16(&srDispObj);

	if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (VS_ERROR);

	if (strlen(srDispObj.szOutput) > 0)
	{
		inDISP_DispLogAndWriteFlie("  TMS Input TID[%s] Line[%d]",srDispObj.szOutput,  __LINE__);
		inSetTerminalID(srDispObj.szOutput);
		inSaveHDTRec(0);
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}


/*
Function        : inEDCTMS_FUNC_GetMID
Date&Time   : 2019/10/1 下午 4:09
Describe        : 輸入Merchant ID
 * inEDCTMS_MID_GET
*/
int inEDCTMS_FUNC_GetMID(TRANSACTION_OBJECT *pobTran)
{
	char    szMerchantID[16 + 1];
	int	inRetVal = 0;
	DISPLAY_OBJECT  srDispObj;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_TMS_ENTER_MID_, 0, _COORDINATE_Y_LINE_8_4_);

	/* 顯示目前的MID */
	memset(szMerchantID, 0x00, sizeof(szMerchantID));
	inGetMerchantID(szMerchantID);
	inFunc_PAD_ASCII(szMerchantID, szMerchantID, ' ', 16, _PADDING_LEFT_);
	inDISP_EnglishFont(szMerchantID, _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	srDispObj.inMaxLen = 15;
	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inColor = _COLOR_RED_;

	inRetVal = inDISP_Enter8x16(&srDispObj);
        
	if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (VS_ERROR);

	if (strlen(srDispObj.szOutput) > 0)
	{
		inDISP_DispLogAndWriteFlie("  TMS Input MID[%s] Line[%d]",srDispObj.szOutput,  __LINE__);
		inSetMerchantID(srDispObj.szOutput);
		inSaveHDTRec(0);
	}        
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
Function        : inEDCTMS_FUNC_CheckVersion
Date&Time   : 2019/10/1 下午 4:11
Describe        : 確認版本名稱及版本日期
 * inEDCTMS_Version_Check
*/
int inEDCTMS_FUNC_CheckVersion(TRANSACTION_OBJECT *pobTran)
{
	char		szDispMsg[16 + 1];
	unsigned char   uszKey;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_CHECK_VERSION_, 0, _COORDINATE_Y_LINE_8_4_);

	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	if (strlen(gszTermVersionID) > 0)
	{
		memcpy(szDispMsg, gszTermVersionID, strlen(gszTermVersionID));
	}
	else
	{
		inGetTermVersionID(szDispMsg);
	}
	
	inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	inGetTermVersionDate(szDispMsg);
	inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
        
	while (1)
	{
		uszKey = uszKBD_GetKey(30);

		if (uszKey == _KEY_ENTER_)
		{
			break;
		}
		else if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
		{
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] *Error* END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
		}
		else
		{
			continue;
		}
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}



/*
Function        : inEDCTMS_FUNC_SetCommParm
Date&Time   : 2019/10/1 下午 4:14
Describe        : 設定TMS HOST IP及TMS PORT
 * inEDCTMS_TMS_SetCommParm
*/
int inEDCTMS_FUNC_SetCommParm(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = 0;
	char	szDispMsg[16 + 1];
	char	szTemplate[20 + 1];
	DISPLAY_OBJECT  srDispObj;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetCommMode(szTemplate);
	
	inDISP_LogPrintfWithFlag("  CommMode[%s] Line[%d]", szTemplate, __LINE__);
	
	/* 若是撥接，則輸入TMS電話號碼 */
	if (memcmp(szTemplate, _COMM_MODEM_MODE_, strlen(_COMM_MODEM_MODE_)) == 0)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* TMS下載電話號碼 */
		inDISP_ChineseFont("TMS下載電話號碼", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

		pobTran->uszFTP_TMS_Download = 'N';

		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetTMSPhoneNumber(szDispMsg);
		inFunc_PAD_ASCII(szDispMsg, szDispMsg, ' ', 16, _PADDING_LEFT_);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		srDispObj.inMaxLen = 15;
		srDispObj.inY = _LINE_8_6_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;

		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		{
			inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] Com Phone[%d] *Error* END -----",__FILE__, __FUNCTION__, __LINE__, inRetVal);
			return (VS_ERROR);
		}

		if (strlen(srDispObj.szOutput) > 0)
		{
			inSetTMSPhoneNumber(srDispObj.szOutput);
			inSaveTMSCPTRec(0);
		}
	}
	/* Ethernet才要選iso8583或FTP */
	else
	{
		
#ifdef _EDC_CHOOSE_MODEL_
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 選擇FTP下載或ISO8583下載 */
		inDISP_ChineseFont("1.FTP下載", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
		inDISP_ChineseFont("2.ISO8583下載", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		while (1)
		{
			uszKey = uszKBD_GetKey(30);

			if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
			{
				pobTran->uszFTP_TMS_Download = 'N';
				return (VS_ERROR);
			}
			else if (uszKey == _KEY_1_)
			{
				pobTran->uszFTP_TMS_Download = 'Y';
				break;
			}
			else if (uszKey == _KEY_2_)
			{
				pobTran->uszFTP_TMS_Download = 'N';
				break;
			}
			else
			{
				continue;
			}
		}
#else  /* _EDC_CHOOSE_MODEL_  */

		pobTran->uszFTP_TMS_Download = 'Y';
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
#endif
		
		if (pobTran->uszFTP_TMS_Download == 'Y')
		{
			inDISP_ChineseFont("FTP HOST IP?", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetFTPIPAddress(szDispMsg);
			inFunc_PAD_ASCII(szDispMsg, szDispMsg, ' ', 16, _PADDING_LEFT_);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 15;
			srDispObj.inY = _LINE_8_6_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;

			inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			{
				inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] FTP IP[%d] *Error* END -----",__FILE__, __FUNCTION__, __LINE__, inRetVal);
				return (VS_ERROR);
			}

			if (strlen(srDispObj.szOutput) > 0)
			{
				inSetFTPIPAddress(srDispObj.szOutput);
				inSaveTMSFTPRec(0);
			}

			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("FTP HOST PORT?", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetFTPPortNum(szDispMsg);
			inFunc_PAD_ASCII(szDispMsg, szDispMsg, ' ', 16, _PADDING_LEFT_);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 15;
			srDispObj.inY = _LINE_8_6_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;

			inRetVal = inDISP_Enter8x16(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			{
				inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] FTP Port[%d] *Error* END -----",__FILE__, __FUNCTION__, __LINE__, inRetVal);
				return (VS_ERROR);
			}

			if (strlen(srDispObj.szOutput) > 0)
			{
				inSetFTPPortNum(srDispObj.szOutput);
				inSaveTMSFTPRec(0);
			}
			
		}else
		{
#ifdef _EDC_CHOOSE_MODEL_
			/* 輸入TMS IP */
			/* 目前不使用 ISO 下載方法  */

			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_TMS_ENTER_IP_, 0, _COORDINATE_Y_LINE_8_4_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetTMSIPAddress(szDispMsg);
			inFunc_PAD_ASCII(szDispMsg, szDispMsg, ' ', 16, _PADDING_LEFT_);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 15;
			srDispObj.inY = _LINE_8_6_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;

			inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				return (VS_ERROR);

			if (strlen(srDispObj.szOutput) > 0)
			{
				inSetTMSIPAddress(srDispObj.szOutput);
				inSaveTMSCPTRec(0);
			}

			/* 輸入TMS PORT */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_TMS_ENTER_PORT_, 0, _COORDINATE_Y_LINE_8_4_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetTMSPortNum(szDispMsg);
			inFunc_PAD_ASCII(szDispMsg, szDispMsg, ' ', 16, _PADDING_LEFT_);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 5;
			srDispObj.inY = _LINE_8_6_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;

			inRetVal = inDISP_Enter8x16(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				return (VS_ERROR);

			if (strlen(srDispObj.szOutput) > 0)
			{
				inSetTMSPortNum(srDispObj.szOutput);
				inSaveTMSCPTRec(0);
			}
#endif
		}
	}
        
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}


/*
Function        : inEDCTMS_FUNC_ConnectToServer
Date&Time   : 2019/10/1 下午 4:23
Describe        : 連線到TMS主機, 目前只能使用FTP下載
 * inEDCTMS_ConnectToServer
*/
int inEDCTMS_FUNC_ConnectToServer(TRANSACTION_OBJECT *pobTran)
{
	char szCommmode[2 + 1];
	char szUserName[20 + 1];
	char szPassWord[20 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	memset(szCommmode, 0x00, sizeof(szCommmode));
	inGetCommMode(szCommmode);
        
	if (memcmp(szCommmode, _COMM_MODEM_MODE_, 1) == 0)
	{
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] Modem Mode *Error* END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 撥接 */
		/* 此功能已關閉 */
		inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, 30, "", 0);
		return (VS_ERROR);
	}
	else if (memcmp(szCommmode, _COMM_ETHERNET_MODE_, 1) == 0 ||
		 memcmp(szCommmode, _COMM_GPRS_MODE_, 1) == 0  ||
		 memcmp(szCommmode, _COMM_WIFI_MODE_, 1) == 0)
	{
                
		inDISP_LogPrintfWithFlag("COMM MODE = [%s]", szCommmode);

		if (inLoadTMSFTPRec(0) != VS_SUCCESS)
		{
			inDISP_Msg_BMP(_ERR_CONNECT_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, 30, "", 0);
			return (VS_ERROR);
		}
                
		if (memcmp(szCommmode, _COMM_ETHERNET_MODE_, 1) == 0)
		{        
			/* 設定EthernetConfig */
			if (inFTPS_EthernetSetConfig(pobTran) != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] Ethernet Config *Error* END -----",__FILE__, __FUNCTION__, __LINE__);
				inDISP_Msg_BMP(_ERR_CONNECT_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, 30, "", 0);
				return (VS_ERROR);
			}
		}        

		/* 避免關閉之後連原本的FTPS有問題 設回預設ID和PW */
		memset(szUserName, 0x00, sizeof(szUserName));
		strcpy(szUserName, "EDC_TMSFtp1_Writer");
		inSetFTPID(szUserName);
		memset(szPassWord, 0x00, sizeof(szPassWord));
		strcpy(szPassWord, "*@!)!(&!))!)!!!$");
		inSetFTPPW(szPassWord);
	}
	else
	{       
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] Comm[%s] *Error* END -----",__FILE__, __FUNCTION__, __LINE__, szCommmode);
		return (VS_ERROR);
	}        

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}


/*
Function        : inEDCTMS_FUNC_CheckTMSOK
Date&Time   : 2019/10/4 上午 11:31
Describe        :確認TMSOK
 * inEDCTMS_CheckTMSOK
*/
int inEDCTMS_FUNC_CheckTMSOK(TRANSACTION_OBJECT *pobTran)
{
        char szTMSOK[2 + 1];

        memset(szTMSOK, 0x00, sizeof(szTMSOK));
        
        inGetTMSOK(szTMSOK);
        
        if (!memcmp(&szTMSOK[0], "Y", 1))
                return (VS_SUCCESS);
        else
                return (VS_ERROR);
}

/*
Function        :inEDCTMS_Settle_Check
Date&Time       :2016/2/2 下午 8:02
Describe        :判斷是否要先結帳
 *	目前沒在用 20190620 [SAM]
 * inEDCTMS_Settle_Check
*/
int inEDCTMS_FUNC_CheckBatchFileForSetMustSettleFlag(TRANSACTION_OBJECT *pobTran)
{
	int     i;
	int     inBAKTotalCnt = 0;      /* 檔案總筆數 */
	char    szTemplate[16 + 1];
	unsigned char   uszFileName[15 + 1];
	unsigned long   ulBKEYHandle, lnBAKTotalFileSize = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
        
	for (i = 0 ;; i ++)
	{
		if (inLoadHDTRec(i) < 0)
				break;

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetHostEnable(szTemplate);

		if (!memcmp(&szTemplate[0], "Y", 1))
		{
			if (inLoadHDPTRec(i) < 0)
				return (VS_ERROR);
		}
		else
		{
			continue;
		}

		if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _BATCH_KEY_FILE_EXTENSION_, 6) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
                
		if (inFILE_Check_Exist(uszFileName) == VS_ERROR)
			continue;
		else
		{
			/* 算出 TRANS_BATCH_KEY 總合 */
			lnBAKTotalFileSize = lnFILE_GetSize(&ulBKEYHandle, uszFileName);
			/* 算出交易總筆數，因為lnFILE_GetSize回傳值為long，因為此函式回傳int所以強制轉型，因為筆數不會超過int大小 */
			inBAKTotalCnt = (int)(lnBAKTotalFileSize / _BATCH_KEY_SIZE_);

			if (inBAKTotalCnt > 0)
			{
				inSetMustSettleBit("Y");
				inSaveHDPTRec(i);
			}
		}
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}


/*
Function        : inEDCTMS_FUNC_ProcessAutoUpdateAP
Date&Time   : 2017/5/8 下午 4:22
Describe        : 目的在移動檔案，由"./fs_data/"的APP.ZIP 移動到 "./fs_data/nexsysap/" 下並解壓縮
 * inEDCTMS_Process_AP
*/
int inEDCTMS_FUNC_ProcessAutoUpdateAP()
{
	int		inRetVal = VS_ERROR;
	char		szPathName[100 + 1] = {0};

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintf("inEDCTMS_FUNC_ProcessAutoUpdateAP Start");
	inFunc_Clear_AP_Dump();
	
	if (inFILE_Check_Exist((unsigned char*)_EDC_APPL_NAME_) != VS_SUCCESS)
	{
		inDISP_LogPrintf("inFILE_Check No Exist");
                inDISP_DispLogAndWriteFlie("  _EDC_APPL_NAME_ Is Not Exist Line[%d]", __LINE__);
		return (VS_ERROR);
	}
        inDISP_LogPrintf("inFILE_Check Exist");
	/* 建立一個新的資料匣，方便清除及解壓檔案  */
	inFunc_Make_Dir(_EDC_APP_UPDATE_DIR_NAME_, _FS_DATA_PATH_);
        inDISP_LogPrintf("Flag A");
	inFunc_Move_Data(_EDC_APPL_NAME_, _FS_DATA_PATH_, "", _EDC_APP_UPDATE_PATH_);
	inDISP_LogPrintf("Flag B");
	inFunc_Unzip("-o", _EDC_APPL_NAME_, _EDC_APP_UPDATE_PATH_, "-d", _EDC_APP_UPDATE_PATH_);
        inDISP_LogPrintf("Flag C");
	inFunc_Delete_Data("", _EDC_APPL_NAME_, _EDC_APP_UPDATE_PATH_);
        inDISP_LogPrintf("Flag D");
	
	/* 以下在測試預計的目錄是否存在，目前不做處理  */
	//inFunc_ls("", _EDC_APP_UPDATE_PATH_);
	//inFunc_ls("", "./fs_data/nexsysap/AP");
	
	memset(szPathName, 0x00, sizeof(szPathName));
	sprintf(szPathName, "%s%s", _EDC_APP_UPDATE_PATH_, _EDC_ESUN_APP_UPDATE_LIST_);
	
	inRetVal = inFunc_Update_AP(szPathName);
	inDISP_LogPrintf("inFunc_Update_AP End");
	inDISP_LogPrintfWithFlag("   inFunc_Update_AP Result [%d]", inRetVal);
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (inRetVal);
}


/*
Function        : inEDCTMS_FUNC_ReInitialIPASSTable
Date&Time   : 2018/1/29 下午 6:02
Describe        : 初始 IPASSDT Table 
 *  inEDCTMS_Initial_IPASS
*/
int inEDCTMS_FUNC_ReInitialIPASSTable()
{
	int	inESVC_HostIndex = -1;
	char	szTemplate[20 + 1];
	char	szTemplate2[20 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	inLoadIPASSDTRec(0);
	/* 有檔案代表有開Host */
	inLoadTDTRec(_TDT_INDEX_00_IPASS_);

	/* Ticket_HostIndex */
	inFunc_Find_Specific_HDTindex(0, _HOST_NAME_ESVC_, &inESVC_HostIndex);
	if (inESVC_HostIndex != -1)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d", inESVC_HostIndex);
		inSetTicket_HostIndex(szTemplate);
	}

	/* TransFunc */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetIPASS_Transaction_Function(szTemplate);
	inSetTicket_HostTransFunc(szTemplate);

	/* HostEnable */
	inSetTicket_HostEnable("Y");

	/* LogOn OK */
	inSetTicket_LogOnOK("N");

	/* 將SAM slot更新到TDT */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetIPASS_SAM_Slot(szTemplate);

	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	sprintf(szTemplate2, "%02d", atoi(szTemplate));
	inSetTicket_SAM_Slot(szTemplate2);

	/* ReaderID*/
	inSetTicket_ReaderID("    ");

	inSaveTDTRec(_TDT_INDEX_00_IPASS_);
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (VS_SUCCESS);
}




int inEDCTMS_FUNC_GetDataFormFileData(unsigned char *uszInData, unsigned char *uszOutData, long *InLen, long lnInDataTotalLen)
{
	int	inCnt = 0 ;
	int	inDataLen;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	while (1)
	{
		/* 讀取總長如果超過傳入的字串總長度則視為資料錯誤 */
		/* 預先把長度加1後判斷是否超過總長度 */
		if ((*InLen+1) > lnInDataTotalLen)
		{
			inDISP_LogPrintfWithFlag("  GetData[%ld] Over DataLen[%d] *Error* Line[%d]",*InLen, inDataLen,  __LINE__);
			return (VS_ERROR);
		}
		
		/* 判斷資料是否為逗號 */
		if (uszInData[inCnt] == ',')
		{
			/* 因為需要連符號都算入，因為不會再使用，所以要加1 */
			inCnt ++;
			/* 計算現有長度位置 */
			*InLen += inCnt;
			/* 因為需要連符號都算入，因為不會再使用，拷備時要減去 */
			memcpy(uszOutData, &uszInData[0], (inCnt -1));
			break;
		}
		else if (uszInData[inCnt - 1] == 0x0D && uszInData[inCnt] == 0x0A)/* 判斷資料是否為換行 */
		{
			/* 因為需要連符號都算入，因為不會再使用，所以要加1 */
			inCnt ++;
			/* 計算現有長度位置 */
			*InLen += inCnt;
			/* 因為需要連符號都算入，因為不會再使用，拷備時要減去 */
			memcpy(uszOutData, &uszInData[0], (inCnt - 2));
			break;
		}
		
		inCnt ++;
		/* 計算現有長度位置 */
		*InLen += inCnt;
		
	}
	
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] inInLen [%ld] END -----",__FILE__, __FUNCTION__, __LINE__, InLen);

	return (VS_SUCCESS);
}


/*
Function        : inEDCTMS_Reboot
Date&Time   : 
Describe        :
 * 
 * inEDCTMS_Reboot
*/
int inEDCTMS_FUNC_Reboot(TRANSACTION_OBJECT * pobTran)
{
	if (pobTran->uszTMSDownloadRebootBit == VS_TRUE)
	{
		inFunc_Reboot();
	}

	return (VS_SUCCESS);
}

/*
Function        : inEDCTMS_FUNC_ConfirmTheTmsWitchHost
Date&Time   : 2019/10/5 上午 11:03
Describe        : 讓操作者選擇要使用哪個FTP的IP做下載
 *
*/
int inEDCTMS_FUNC_ConfirmTheTmsWitchHost(TRANSACTION_OBJECT * pobTran)

{

	return (VS_SUCCESS);
}

/*
Function        : inEDCTMS_FUNC_GetUserChooseFtpId
Date&Time   : 
Describe        :
 *  因為要選擇，如果不是使用嘉利的TMS就使用固定的IP 值 2019/10/8 [SAM] 
 *  inEDCTMS_GetUserChooseFtpId
*/
int inEDCTMS_FUNC_GetUserChooseFtpId()
{
	return (st_inUseFtpId);
}

/*
Function        : inEDCTMS_FUNC_SetUserChooseFtpId
Date&Time   : 
Describe        :
  *  因為要選擇，如果不是使用嘉利的TMS就使用固定的IP 值 2019/10/8 [SAM] 
 *  inEDCTMS_SetUserChooseFtpId
*/
int inEDCTMS_FUNC_SetUserChooseFtpId(int inFtpid)
{
	st_inUseFtpId = inFtpid;
	
	return (st_inUseFtpId);
}

//
//int inEDCTMS_FUNC_Check(TRANSACTION_OBJECT * pobTran)
//{
//	int i;
//	BOOL fDWLSuccess;
//	char szTemplate[32];
//	/* 自動詢問和下載的不用印 */
//	if (pobTran->inTMSDwdMode == _EDC_TMS_AUTO_DOWNLOAD_)
//	{
//		for (i = 0 ;; i++)
//		{
//			/* 下載結果已經存在File Index */
//			if (inLoadEDCFTPFLTRec(i) < 0)
//			{
//				inDISP_DispLogAndWriteFlie("  TMS Load FTPFLT Break [%d] *Error* LINE[%d]", i, __LINE__);
//				break;
//			}
//
//			memset(szTemplate, 0x00, sizeof(szTemplate));
//			/* 取得下載結果 Y or N */
//			/* 這個參數會在下載檔案成功時被改為 "Y "，所以可以用來判斷該檔是否有下載完成 */
//			inGetEDCFTPFileIndex(szTemplate);
//				
//			if (!memcmp(&szTemplate[0], "Y", 1))
//			{
//				/* 下載成功且是排程下載，將指定詢問時間初始化*/
//				memset(szTemplate, 0x00, sizeof(szTemplate));
//				inGetTMSInquireMode(szTemplate);
//
//				if (!memcmp(&szTemplate[0], _TMS_INQUIRE_02_SCHEDHULE_SETTLE_, 1))
//				{
//					inSetTMSInquireStartDate("00000000");
//					inSetTMSInquireTime("000000");
//					inSaveTMSSCTRec(0);
//				}                                
//			}
//			else
//			{
//				inDISP_DispLogAndWriteFlie("  TMS Print Check  FN[%s] *Error* LINE[%d]",_EDC_FTPFLT_FILE_NAME_, __LINE__);
//				fDWLSuccess = VS_ERROR;
//			}                
//		}
//
//		if (fDWLSuccess != VS_SUCCESS)
//		{
//			/* 刪除File List */
//			inFILE_Delete((unsigned char *)_TMSFTP_FILE_NAME_);
//		}
//
//		/* 若是排程下載，則回idle再檢查生效時間 */
//		return (VS_SUCCESS);
//	}
//	
//}

int inEDCTMS_FUNC_FindAndCopyDataWhitComma(char* szSource, char* szDestn, int inMaxLen, int* inSerchCnt) 
{
	do
	{
		if(szSource[*inSerchCnt] == ',' )
		{
			memcpy(szDestn, szSource, *inSerchCnt);
			/* 需要跳過"," */
			*inSerchCnt  += 1;
			return VS_SUCCESS;
		}
		
		*inSerchCnt += 1 ;
		
	}while(*inSerchCnt < inMaxLen);
	
	return VS_ERROR;
}

int inEDCTMS_FUNC_SetTmsFtpSettleFlagOn()
{
	inLoadTMSFTPRec(0);
	inSetFTPEffectiveCloseBatch("Y");
	inSaveTMSFTPRec(0);
	return VS_SUCCESS;
}

int inEDCTMS_FUNC_SetTmsFtpSettleFlagOff()
{
	inLoadTMSFTPRec(0);
	inSetFTPEffectiveCloseBatch("N");
	inSaveTMSFTPRec(0);
	return VS_SUCCESS;
}


