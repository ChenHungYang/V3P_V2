
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>

#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Flow.h"

#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/Sqlite.h"

#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"

#include "../SOURCE/FUNCTION/ECR_FUNC/ECR.h"

#include "../EMVSRC/EMVsrc.h"

#include "SvcSrc.h"

int SVC_ACTIVECARD_TRT_TABLE[] ={
//	_FUNCTION_CHECK_TRANS_FUNCTION_FLOW_, 待補，目前沒功能開關
	_FUBON_FUNCTION_MUST_SETTLE_CHECK_,
	_FUNCTION_DUPLICATE_CHECK_,
	_COMM_MODEM_PREDIAL_,
	_FUBON_SET_ONLINE_OFFLINE_,
	_SVC_ISO_FUNCTION_BUILD_AND_SEND_PACKET_,
	_SVC_UPDATE_ACCUM_,
	_UPDATE_BATCH_,
	_FUNCTION_DUPLICATE_SAVE_,
	_FUNCTION_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_SVC_AUTO_LOAD_,
	_FLOW_NULL_,
};

int SVC_INQUIRY_TRT_TABLE[] ={
//	_FUNCTION_CHECK_TRANS_FUNCTION_FLOW_, 待補，目前沒功能開關
	_FUBON_FUNCTION_MUST_SETTLE_CHECK_,
	_FUNCTION_DUPLICATE_CHECK_,
	_COMM_MODEM_PREDIAL_,
	_FUBON_SET_ONLINE_OFFLINE_,
	_SVC_ISO_FUNCTION_BUILD_AND_SEND_PACKET_,
	_SVC_UPDATE_ACCUM_,
	_UPDATE_BATCH_,
	_FUNCTION_DUPLICATE_SAVE_,
	_FUNCTION_UPDATE_INV_,
	_SVC_DISPLAY_POINT_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_FLOW_NULL_,
};

int SVC_REDEEM_TRT_TABLE[] ={
//	_FUNCTION_CHECK_TRANS_FUNCTION_FLOW_, 待補，目前沒功能開關
	_FUBON_FUNCTION_MUST_SETTLE_CHECK_,
	_FUNCTION_DUPLICATE_CHECK_,
	_COMM_MODEM_PREDIAL_,
	_FUBON_SET_ONLINE_OFFLINE_,
	_SVC_ISO_FUNCTION_BUILD_AND_SEND_PACKET_,
	_SVC_UPDATE_ACCUM_,
	_UPDATE_BATCH_,
	_FUNCTION_DUPLICATE_SAVE_,
	_FUNCTION_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_FLOW_NULL_,
};

int SVC_SETTLE_TRT_TABLE[] ={
	_FUNCTION_CHECK_TRANS_FUNCTION_FLOW_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_, /*  要再確認是否要在這邊回送 20090124 [SAM]*/
	_SVC_ISO_FUNCTION_BUILD_AND_SEND_PACKET_,
	_FUNCTION_PRINT_TOTAL_REPORT_BY_BUFFER_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_, /*  自助結帳回覆 20090124 [SAM]*/
	_FUNCTION_DELETE_BATCH_FLOW_,
	_FUNCTION_DELETE_ACCUM_FLOW_,
	_FUNCTION_REST_BATCH_INV_,
	_FUNCTION_UPDATE_BATCH_NUM_,
	_FLOW_NULL_,
};

int SVC_REFUND_TRT_TABLE[] ={
//	_FUNCTION_CHECK_TRANS_FUNCTION_FLOW_, 待補，目前沒功能開關
	_FUBON_FUNCTION_MUST_SETTLE_CHECK_,
	_FUNCTION_DUPLICATE_CHECK_,
	_COMM_MODEM_PREDIAL_,
	_SVC_ISO_FUNCTION_BUILD_AND_SEND_PACKET_,
	_SVC_UPDATE_ACCUM_,
	_UPDATE_BATCH_,
	_FUNCTION_DUPLICATE_SAVE_,
	_FUNCTION_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_FLOW_NULL_,
};

int SVC_BACKCARD_TRT_TABLE[] ={
//	_FUNCTION_CHECK_TRANS_FUNCTION_FLOW_, 待補，目前沒功能開關
	_FUBON_FUNCTION_MUST_SETTLE_CHECK_,
	_FUNCTION_DUPLICATE_CHECK_,
	_COMM_MODEM_PREDIAL_,
	_SVC_COMPARE_POINT_,
	_SVC_ISO_FUNCTION_BUILD_AND_SEND_PACKET_,
	_SVC_UPDATE_ACCUM_,
	_UPDATE_BATCH_,
	_FUNCTION_DUPLICATE_SAVE_,
	_FUNCTION_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_FLOW_NULL_,
};

int SVC_RELOAD_TRT_TABLE[] ={
//	_FUNCTION_CHECK_TRANS_FUNCTION_FLOW_, 待補，目前沒功能開關
	_FUBON_FUNCTION_MUST_SETTLE_CHECK_,
	_FUNCTION_DUPLICATE_CHECK_,
	_COMM_MODEM_PREDIAL_,
	_SVC_ISO_FUNCTION_BUILD_AND_SEND_PACKET_,
	_SVC_UPDATE_ACCUM_,
	_UPDATE_BATCH_,
	_FUNCTION_DUPLICATE_SAVE_,
	_FUNCTION_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_FLOW_NULL_,
};

TRT_TABLE SVC_TRT_TRANS_TABLE[] ={
	{_TRT_SVC_ACTIVECARD_,	SVC_ACTIVECARD_TRT_TABLE},
	{_TRT_SVC_INQUIRY_,		SVC_INQUIRY_TRT_TABLE},
	{_TRT_SVC_REDEEM_,		SVC_REDEEM_TRT_TABLE},
	{_TRT_SVC_SETTLE_,		SVC_SETTLE_TRT_TABLE},
	{_TRT_SVC_REFUND_,		SVC_REFUND_TRT_TABLE},
	{_TRT_SVC_BACKCARD_,	SVC_BACKCARD_TRT_TABLE},
	{_TRT_SVC_RELOAD_,		SVC_RELOAD_TRT_TABLE},	
	{_FLOW_NULL_, NULL}
};

/* SVC感應卡流程會跑 SVC_CARD_OPERATION */

int inSVC_RunTRT(TRANSACTION_OBJECT *pobTran, int inTRTCode)
{
	int *inTRTID = NULL;
	int i, inRetVal = VS_ERROR;

	for (i = 0;; i++)
	{
		if (SVC_TRT_TRANS_TABLE[i].inTRTCode == inTRTCode)
		{
			inTRTID = SVC_TRT_TRANS_TABLE[i].inTRTID;
			break;
		} else if (SVC_TRT_TRANS_TABLE[i].inTRTCode == -1)
			break;
	}

	if (inTRTID == NULL)
	{
		inECR_SendError(pobTran, VS_USER_OPER_ERR);
		return (VS_ERROR);
	}

	/* 補註解:
	 *  因 inTRTID 結構最後一個元素會設定為 _FLOW_NULL_ 
	 * 所以 inRetVal 的值，會以執行的 inTRTID 功能時最後執行結果為回覆值 2022/9/30 [SAM] */
	for (i = 0;; i++)
	{
		/* 正常的結束TRT流程  */
		if (inTRTID[i] == _FLOW_NULL_)
		{
			break;
		}

		inRetVal = inFLOW_RunFunction(pobTran, inTRTID[i]);

		if (inRetVal != VS_SUCCESS)
		{
			
			inDISP_LogPrintfWithFlag(" FU Trt inErrorMsg[%d] RetVal[%s]", pobTran->inErrorMsg, 
							pszDisp_GetResponseV3Message(inRetVal));
			/* 只有出錯才需SendError */
			if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
			{
				inECR_SendError(pobTran, pobTran->inECRErrorMsg);
			} else
			{
				inECR_SendError(pobTran, inRetVal);
			}
			break;
		}

	}

	/* 斷線 */
	inCOMM_End(pobTran);

	/* 要先回傳再顯示錯誤訊息 */
	inFunc_Display_Error(pobTran);

	/* 退回晶片卡 */
	if (inRetVal != VS_SUCCESS)
	{
		inFunc_Check_Card_Still_Exist(pobTran, _REMOVE_CARD_ERROR_);
	} else
	{
		inFunc_Check_Card_Still_Exist(pobTran, _REMOVE_CARD_NOT_ERROR_);
	}

	return (inRetVal);
}


int inSVC_Tms_Download(TRANSACTION_OBJECT *pobTran)
{
	char szTmsOK[3] = {0};
	int	inTempTransactionCode = 0;
	int	inTempinCode = 0;
	int	inRetVal = 0;
	
	pobTran->srBRec.inHDTIndex = _SVC_HDT_INDEX_21_;
	inLoadHDTRec(pobTran->srBRec.inHDTIndex);
	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	inGetSVCTmsOK(szTmsOK);// TODO: 這個是SVC使用的 "SVCTMSOK" 要改

	if (szTmsOK[0] == 'Y')
	{
		inDISP_Msg_BMP(_ERR_TMS_DWL_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		return (VS_SUCCESS);
	}

	/* 因為可能在沒抓時間的地方使用，所以還是重抓 */
	{
		RTC_NEXSYS srRTC;

		memset(&srRTC, 0x00, sizeof (RTC_NEXSYS));
		if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
			return (VS_ERROR);

		inFunc_Sync_BRec_Date_Time(pobTran, &srRTC);
		
		/* 設定 SVC 交易時間 */
		memcpy(pobTran->srBRec.szSvcTxnDateTime, pobTran->srBRec.szDate, 8);
		memcpy(&pobTran->srBRec.szSvcTxnDateTime[8], pobTran->srBRec.szTime, 8);
		inDISP_LogPrintfWithFlag("Svc Tms Time[%s]", pobTran->srBRec.szSvcTxnDateTime);
		
	}
	
	inTempTransactionCode = pobTran->inTransactionCode;
	inTempinCode = pobTran->srBRec.inCode;
	
	pobTran->inTransactionCode = _SVC_TMS_;
	pobTran->srBRec.inCode = _SVC_TMS_;
	if ((inRetVal = inSVC_ISO_BuildAndSendPacket(pobTran)) != VS_SUCCESS)
	{
		pobTran->inTransactionCode = inTempTransactionCode;
		pobTran->srBRec.inCode = inTempinCode;
		inSetSVCTmsOK("N");
		return (VS_ERROR);
	}
	else
	{
		pobTran->inTransactionCode = inTempTransactionCode;
		pobTran->srBRec.inCode = inTempinCode;
		inSetSVCTmsOK("Y");
		/* 把功能轉成啟動卡 */
		pobTran->srBRec.inCode = _SVC_ACTIVECARD_;
		pobTran->inTransactionCode = _SVC_ACTIVECARD_;
//		pobTran->lnCheckTxnFunction = _SVC_ACTIVECARD_;
		
	}
	
	return VS_SUCCESS;
}

/*
 * 依照 Vx520 開卡自動儲值 1000 元
 **/
int inSVC_AutoReload(TRANSACTION_OBJECT *pobTran)
{
	if ( pobTran->fSvcActivedCardAuto == VS_FALSE)
	{
		return (VS_SUCCESS);
	}

	pobTran->inTransactionCode = _SVC_RELOAD_;
	pobTran->srBRec.inCode = _SVC_RELOAD_;
	pobTran->srBRec.lnTxnAmount = 1000;

	if (inFLOW_RunFunction(pobTran, _SVC_ISO_FUNCTION_BUILD_AND_SEND_PACKET_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (inFLOW_RunFunction(pobTran, _SVC_UPDATE_ACCUM_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (inFLOW_RunFunction(pobTran, _UPDATE_BATCH_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (inFLOW_RunFunction(pobTran, _FUNCTION_DUPLICATE_SAVE_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (inFLOW_RunFunction(pobTran, _FUNCTION_UPDATE_INV_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (inFLOW_RunFunction(pobTran, _FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}


int inSVC_ComparePoint(TRANSACTION_OBJECT *pobTran)
{
	char	szSvcKey;
	char	szTemplate[42 + 1];
	long	lnBalaceAmount = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	if (inLoadHDTRec(_SVC_HDT_INDEX_21_) < 0)
	{
		inDISP_LogPrintfWithFlag("inSVC_Compare_Point()_LOAD_HDT_ERROR");
		return (VS_ERROR);
	}

	pobTran->srBRec.inCode = _SVC_INQUIRY_;
	if (inFLOW_RunFunction(pobTran, _SVC_ISO_FUNCTION_BUILD_AND_SEND_PACKET_) != VS_SUCCESS)
	{
		inDISP_LogPrintfWithFlag("inSVC_Compare_Point()_ISO_ERROR");
		return (VS_ERROR);
	}

	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(szTemplate, (unsigned char*)pobTran->srBRec.szSvcPoint1, 5);
	lnBalaceAmount = atol(szTemplate)/100;

	if (lnBalaceAmount !=  pobTran->srBRec.lnTxnAmount)
	{
		inDISP_LogPrintfWithFlag("inSVC_Compare_Point_(%lu)(%lu)", lnBalaceAmount, pobTran->srBRec.lnTxnAmount);

		if (pobTran->uszEcrTransFlag == 'Y')
		{
			inECR_SendError(pobTran, VS_USER_OPER_ERR);
		}

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("退卡金額錯誤", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
		inDISP_ChineseFont("請按清除鍵", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		szSvcKey = 0x00;
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		inDISP_BEEP(3, 100);

		while (1) 
		{
			inDISP_BEEP(1, 100);
			
			szSvcKey = uszKBD_Key();
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS) {
				szSvcKey = _KEY_TIMEOUT_;
			}

			if (szSvcKey == _KEY_TIMEOUT_ || szSvcKey == _KEY_CLEAR_) {
				return (VS_ERROR);
			}
		}
		/* 因為已顯示過訊息，所以設定為無錯誤碼 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		return (VS_ERROR);
	}

//	vdDisplayTranTitle(TWN_SVC_BACKCARD_TITLE_MSG1); /* 第一行顯示 */
	pobTran->inTransactionCode = _SVC_BACKCARD_;
	pobTran->srBRec.inCode = _SVC_BACKCARD_;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}



