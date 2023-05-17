
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/TDT.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/ECR_FUNC/ECR.h"
#include "../SOURCE/COMM/TLS.h"
#include "../SOURCE/COMM/Comm.h"
#include "../SOURCE/COMM/Ethernet.h"
#include "../EMVSRC/EMVsrc.h"

#include "../ECC/ICER/stdAfx.h"
#include "../ECC/ICER/libutil.h"
#include "../ECC/ICER/ICERAPI.h"

#include "../NCCC/NCCCTicketSrc.h"
#include "../NCCC/NCCCtSAM.h"
#include "CMASAPIISO.h"
//#include "CMASFunction.h"
#include "CMASsrc.h"

extern int ginDebug; /* Debug使用 extern */
extern int ginAPVersionType;
extern int ginCMASDirLink;

int CMAS_DEDUCT_TRT_TABLE[] = {
	_CMAS_GET_PARM_,
	_CMAS_API_FLOW_,
	_CMAS_UPDATE_ACCUM_,
	_CMAS_UPDATE_BATCH_,
	_CMAS_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_FLOW_NULL_,
};

int CMAS_REFUND_TRT_TABLE[] = {
	_CMAS_GET_PARM_,
	_CMAS_API_FLOW_,
	_CMAS_UPDATE_ACCUM_,
	_CMAS_UPDATE_BATCH_,
	_CMAS_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_FLOW_NULL_,
};

int CMAS_QUERY_TRT_TABLE[] = {
	_CMAS_GET_PARM_,
	_CMAS_API_FLOW_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_FLOW_NULL_,
};

int CMAS_ADD_VALUE_TRT_TABLE[] = {
	_CMAS_GET_PARM_,
	_CMAS_API_FLOW_,
	_CMAS_UPDATE_ACCUM_,
	_CMAS_UPDATE_BATCH_,
	_CMAS_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_FLOW_NULL_,
};

int CMAS_VOID_ADD_VALUE_TRT_TABLE[] = {
	_CMAS_GET_PARM_,
	_CMAS_API_FLOW_,
	_CMAS_UPDATE_ACCUM_,
	_CMAS_UPDATE_BATCH_,
	_CMAS_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_FLOW_NULL_,
};

int CMAS_SETTLE_TRT_TABLE[] = {
	_CMAS_SETTLE_FLOW_,
	_FUNCTION_DELETE_BATCH_FLOW_,
	_FUNCTION_PRINT_TOTAL_REPORT_BY_BUFFER_,
	_FUNCTION_DELETE_ACCUM_FLOW_,
	_FUNCTION_REST_BATCH_INV_,
	_FUNCTION_UPDATE_BATCH_NUM_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_CMAS_SET_UPDATE_BATCH_FLAG_,
	_FUNCTION_CHECK_TIME_, /* 20220214,新增確認系統日期與版本日期 */
	_FLOW_NULL_,
};

TRT_TABLE TRANSACTION_CMAS_TABLE[] = {
	{_TRT_CMAS_DEDUCT_, CMAS_DEDUCT_TRT_TABLE},
	{_TRT_CMAS_REFUND_, CMAS_REFUND_TRT_TABLE},
	{_TRT_CMAS_QUERY_, CMAS_QUERY_TRT_TABLE},
	{_TRT_CMAS_ADD_VALUE_, CMAS_ADD_VALUE_TRT_TABLE},
	{_TRT_CMAS_VOID_ADD_VALUE_, CMAS_VOID_ADD_VALUE_TRT_TABLE},
	{_TRT_CMAS_SETTLE_, CMAS_SETTLE_TRT_TABLE},
	{_FLOW_NULL_, NULL}
};

/*
 * Function		: inCMAS_Init_Flow
 * Date&Time	: 2020/2/3 下午 6:06
 * Describe		: 票證初始化，組INI
 * Author		: SAM 
 * Describe		:  加入合庫程式
 *  原作者參照 inNCCC_Ticket_Init_Logon_Flow 功能重寫
 * 
 */
int inCMAS_Init_Flow(TRANSACTION_OBJECT *pobTran)
{
	int inESVCIndex = -1;
	int inRetVal = VS_SUCCESS;
	char szSAMSlot[2 + 1] = {0};
	char szCMASSAMSlot [1 + 1] = {0};
	char szTMSOK[2 + 1] = {0};
	char szHostEnable[2 + 1] = {0};
	char szDemoMode[2 + 1] = {0};
	char szDebugMsg[100 + 1] = {0};
	char szAscii[6 + 1] = {0};
	char szCMASLinkMode[ 2 + 1 ] = {0};
	char szTemplate[50 + 1] = {0};
	unsigned char uszECCVersion[3 + 1] = {0};

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT, "inCMAS_Init_Flow START");
	}

	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof (szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		if (inLoadTDTRec(_TDT_INDEX_01_ECC_) < 0)
		{
			return (VS_SUCCESS);
		}
		if (inLoadHDTRec(_HDT_INDEX_13_CMAS_) < 0)
		{
			return (VS_SUCCESS);
		}
		memset(szHostEnable, 0x00, sizeof (szHostEnable));
		inGetHostEnable(szHostEnable);

		if (memcmp(szHostEnable, "Y", strlen("Y")) == 0)
		{
			inSetTicket_LogOnOK("Y");
			/* 一代設備編號 */
			inSetTicket_Device1("221234567890");
			/* 二代設備編號 */
			inSetTicket_Device2("09101234567890");
			/* 悠遊卡批次號碼 */
			inSetTicket_Batch("18010101");
			/* DEMO好像不用更新 */
			inSetTicket_NeedNewBatch("N");
		} else
		{
			inSetTicket_LogOnOK("N");
		}

		inSaveTDTRec(_TDT_INDEX_01_ECC_);

		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintfAt(AT, "inCMAS_Init_Flow (demo) end");
		}
		return (VS_SUCCESS);
	} else
	{
		/* 檢查是否做過【參數下載】 */
		memset(szTMSOK, 0x00, sizeof (szTMSOK));
		inGetTMSOK(szTMSOK);
		if (szTMSOK[0] != 'Y')
			return (VS_SUCCESS);

		/* SamSlot */
		inLoadTDTRec(_TDT_INDEX_01_ECC_);
		memset(szSAMSlot, 0x00, sizeof (szSAMSlot));
		inGetTicket_SAM_Slot(szSAMSlot);
		switch (atoi(szSAMSlot))
		{
			case 1:
				szCMASSAMSlot[0] = 0x11;
				break;
			case 2:
				szCMASSAMSlot[0] = 0x22;
				break;
			case 3:
				szCMASSAMSlot[0] = 0x33;
				break;
			case 4:
				szCMASSAMSlot[0] = 0x44;
				break;
			default:
				szCMASSAMSlot[0] = 0x22;
				break;
		}

		/* 悠遊卡硬體初始化只執行一次 */
		inFunc_Find_Specific_HDTindex(-1, _HOST_NAME_CMAS_, &inESVCIndex);

		if (inESVCIndex != -1)/* 如果有找到CMAS Host index*/
		{
			if (inLoadHDTRec(inESVCIndex) != VS_SUCCESS)
				return (VS_ERROR);

			memset(szHostEnable, 0x00, sizeof (szHostEnable));
			inGetHostEnable(szHostEnable);
			if (memcmp(szHostEnable, "Y", strlen("Y")) == 0)
			{
				inDISP_ClearAll();
				inICERAPI_InitialReader2(szCMASSAMSlot[0]);
			}
		}

		/* 需新增 NCCCtSAM.c ，使用NCCC TSAM功能  */
		inNCCC_tSAM_PowerOn_ECC_Flow();
		pobTran->srTRec.inTicketType = _TICKET_TYPE_ECC_;

		/* 電子票證 */
		/* 刪LOG */
		// inECC_Delete_Log();
		inCMAS_Delete_Log();

		/* 建立資料夾 */
		inFunc_Make_Dir(_CMAS_FOLDER_NAME_, "./");

		/* 2020.04.06 將ESUN SSL連線憑證複製到ICERDATA資料夾底下。*/
		/* 2020.04.06 因為目前直連只有玉山，這邊載入玉山測試憑證。*/
		/* 2020.04.14 只有玉山會用到[Hachi]*/
		memset(szCMASLinkMode, 0x00, sizeof (szCMASLinkMode));
		inGetCMASLinkMode(szCMASLinkMode);
		/* [新增電票悠遊卡功能]  這個區段是在設定是否要使用 SSL憑證, 目前不會使用外部憑證,所以先註解掉以下程式 [SAM] 2022/6/14 下午 4:52 */
		//		if (szCMASLinkMode[0] == '0') /*1 = 間連  0 = 直連*/ {
		//			if (ginAPVersionType == _APVERSION_TYPE_TCB_ || ginAPVersionType == _APVERSION_TYPE_BOT_) {
		//				/* 20201203 除了玉山之外不需要額外複製測試憑證*/
		//			} else {
		//
		//				inFunc_Data_Copy(_PEM_CA_CMAS_ESUN_FILE_NAME_, _CA_DATA_PATH_, _PEM_CA_CMAS_ESUN_FILE_NAME_, _CMAS_FOLDER_PATH_);
		//			}
		//		}
		/* [新增電票悠遊卡功能]  這個區段是在設定是否要使用 SSL憑證, 目前不會使用外部憑證,所以先註解掉以下程式 END [SAM] 2022/6/14 下午 4:52 */

		/* 初始化 */
		inDISP_ClearAll();
		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
		inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /*電子票證title*/
		inDISP_ChineseFont("悠遊卡初始化", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);

		/* 2020.03.05.新增 確認version */
		ECC_ReadLibVersion_Lib(uszECCVersion);
		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof (szAscii));
			inFunc_BCD_to_ASCII(szAscii, uszECCVersion, 3);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "ECC Version: %s", szAscii);
			inDISP_LogPrintfAt(AT, szDebugMsg);
		}

		/* 產生參數檔 */
		inCMAS_PACK_ICERINI(pobTran);

		/*20200508 組電文(生成ICERAPI.REQ)*/
		inCMAS_PACK_LogOn_ICERAPI(pobTran);

		/* 20200415組電文(生成ICERAPI2.REQ)*/
		if (szCMASLinkMode[0] == '1')
		{
			inCMAS_PACK_Deduct_ICERAPI_FISC(pobTran);
		}

		/*20200520 Call ICERApi.exe前，新增撥接與連線檢核，不然會造成程式跳出 */
		/* 票證沒撥接，自行防呆 */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		inGetCommMode(szTemplate);
		if (memcmp(szTemplate, _COMM_MODEM_MODE_, strlen(_COMM_MODEM_MODE_)) == 0)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("此交易不支援撥接", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
			inDISP_Wait(2000);

			inRetVal = VS_ERROR;
		} else
		{
			/* CMAS_API_REQ_CHECK 要有 */
			/* 因悠遊卡不會檢核是否連線成功就直接送會造成死機(EDC IP沒設定好不會回傳Socket錯誤)，所以在這邊防呆 */
			if (inETHERNET_Check_Ethernet_Config_Correct() == VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintfAt(AT, "ICER Start");
				}

				inRetVal = ICERApi_exe();

				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintfAt(AT, "ICER End");
				}
			} else
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
				inFunc_Display_Error(pobTran); /* 通訊失敗 */
				pobTran->inErrorMsg = 0x00;
				inRetVal = VS_ERROR;
			}
		}

		if (inRetVal != VS_SUCCESS)
		{
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
			inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("悠遊卡認證失敗", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
			inDISP_BEEP(3, 500);
			inLoadTDTRec(_TDT_INDEX_01_ECC_);
			inSetTicket_LogOnOK("N");
			inSaveTDTRec(_TDT_INDEX_01_ECC_);

			//inECC_UNPACK_ICERAPI(pobTran, _CMAS_API_RES_FILE_);
			// return (VS_ERROR);
		} else
		{
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
			inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
			inDISP_ChineseFont_Color("悠遊卡授權已完成", _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_BLACK_, _DISP_CENTER_);
			inDISP_Wait(500);
			//inECC_UNPACK_ICERAPI(pobTran, _CMAS_API_RES_FILE_);
			inLoadTDTRec(_TDT_INDEX_01_ECC_);
			inSetTicket_LogOnOK("Y");
			inSaveTDTRec(_TDT_INDEX_01_ECC_);

			// return (VS_SUCCESS);
		}

		/* 清空錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;

		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintfAt(AT, "inCMAS_Init_Flow END");
		}
		return (VS_SUCCESS);
	}
}

/*
 * Function		: inCMAS_Func_UpdateTermInvNum
 * Date&Time	: 2020/2/3 下午 6:06
 * Describe		: 票證初始化，組INI
 * Author		: SAM 
 * Describe		:  加入合庫程式   
 *  更新簽單InvNum和電文InvNum，原作者參照 參考inNCCC_Ticket_Func_UpdateTermInvNum
 * 
 */
int inCMAS_Func_UpdateTermInvNum(TRANSACTION_OBJECT *pobTran)
{
	int inESVC_HostIndex = -1;
	char szInvNum[6 + 1];
	char szTicketInvNum[6 + 1];
	char szHostBatchNumLimit[6 + 1];
	long lnInvNum = 0;
	long lnTicketInvNum = 0;
	long lnHostBatchNumLimit = 0;
	unsigned char uszSettleBit = VS_FALSE;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT, "----------------------------------------");
		inDISP_LogPrintfAt(AT, "inCMAS_Func_UpdateTermInvNum() START !");
	}

	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CMAS_, &inESVC_HostIndex);

	if (inLoadHDPTRec(inESVC_HostIndex) == VS_ERROR)
	{
		/* 鎖機 */
		inFunc_EDCLock();

		return (VS_ERROR);
	}

	/* InvNum增加 */
	memset(szInvNum, 0x00, sizeof (szInvNum));
	inGetInvoiceNum(szInvNum);
	lnInvNum = atol(szInvNum);
	lnInvNum++;

	memset(szInvNum, 0x00, sizeof (szInvNum));
	sprintf(szInvNum, "%06ld", lnInvNum);
	inSetInvoiceNum(szInvNum);

	memset(szHostBatchNumLimit, 0x00, sizeof (szHostBatchNumLimit));
	inGetBatchNumLimit(szHostBatchNumLimit);
	lnHostBatchNumLimit = atol(szHostBatchNumLimit);

	/* 超過交易筆數，表示要結帳 */
	if (lnInvNum >= lnHostBatchNumLimit)
	{
		uszSettleBit = VS_TRUE;
	}

	if (uszSettleBit == VS_TRUE)
	{
		inSetMustSettleBit("Y");
	}

	/*20210427Hachi [補] start*/
	if (inLoadTDTRec(pobTran->srTRec.inTDTIndex) == VS_ERROR)
	{
		/* 鎖機 */
		inFunc_EDCLock();

		return (VS_ERROR);
	}
	/*20210427Hachi [補] End*/

	memset(szTicketInvNum, 0x00, sizeof (szTicketInvNum));
	inGetTicket_InvNum(szTicketInvNum);
	lnTicketInvNum = atol(szTicketInvNum);
	/* 檢核Inv是否合法(不能小於原本的值) */
	if (pobTran->srTRec.lnCountInvNum >= lnTicketInvNum)
	{
		//pobTran->srTRec.lnCountInvNum++;  /*20210427 [Hachi ]mark*/
		memset(szTicketInvNum, 0x00, sizeof (szTicketInvNum));
		sprintf(szTicketInvNum, "%06ld", pobTran->srTRec.lnCountInvNum);
		inSetTicket_InvNum(szTicketInvNum);
	}

	/* int limit 32767 */
	if (pobTran->srTRec.lnCountInvNum > 32000)
	{
		inSetTicket_InvNum("000001");
	}

	if (inSaveHDPTRec(inESVC_HostIndex) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		//	        /* 悠遊卡特有 */
		//	        inLoadCMASTRec(0);
		//	        if (lnGetCMAS_STAN() > 999990)
		//	        {
		//	                vdSetCMAS_STAN(1);
		//                        inSaveCMASTRec(0);
		//                }
	}

	pobTran->uszTicketADVOnBit = VS_FALSE;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT, "inCMAS_Func_UpdateTermInvNum() END !");
		inDISP_LogPrintfAt(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}


/* Added by Hachi 2019/6/25 下午 2:59  IPASS 2.0  start*/

/*
Function        :inCMAS_RunTRT
Date&Time       :2016/9/19 下午 3:44
Describe        :執行悠遊卡交易流程
Author		:Hachi
 */
int inCMAS_RunTRT(TRANSACTION_OBJECT *pobTran, int inTRTCode)
{
	int *inTRTID = NULL;
	int i, inRetVal = VS_ERROR;
	char szTemplate[100 + 1];

	if (ginDebug == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "TRT CODE = %d", inTRTCode);
		inDISP_LogPrintfAt(AT, "inCMAS_RunTRT() START!");
		inDISP_LogPrintfAt(AT, szTemplate);
	}

	for (i = 0;; i++)
	{
		if (1)
		{
			if (TRANSACTION_CMAS_TABLE[i].inTRTCode == inTRTCode)
			{
				inTRTID = TRANSACTION_CMAS_TABLE[i].inTRTID;
				break;
			}
		} else
		{
			inTRTID = NULL;
		}
	}

	if (inTRTID == NULL)
	{
		return (VS_ERROR);
	}

	for (i = 0;; i++)
	{
		if (inTRTID[i] == VS_ERROR)
			break;

		inRetVal = inFLOW_RunFunction(pobTran, inTRTID[i]);

		if (inRetVal != VS_SUCCESS)
		{

#ifdef _SUPPORT_FISC_HOST_
			/* 如果有插SmartPay錯誤要Power Off */
			if (pobTran->uszFISCBit == VS_TRUE)
			{
				inFISC_PowerOFF(pobTran);
			}
#endif
			/* 只有出錯才需SendError */
			inECR_SendError(pobTran, inRetVal);
			inFunc_Display_Error(pobTran);
			break;
		}
	}

	/* 斷線 */
	inCOMM_End(pobTran); //20190725 Hachi

	/* 要先回傳再顯示錯誤訊息 */
	// inFunc_Display_Error(pobTran);

	/* 退回晶片卡 */
	if (inRetVal != VS_SUCCESS)
	{
		inFunc_Check_Card_Still_Exist(pobTran, _REMOVE_CARD_ERROR_);
	} else
	{
		if (pobTran->uszECRBit == VS_TRUE && pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_)
		{
		} else
		{
			inFunc_Check_Card_Still_Exist(pobTran, _REMOVE_CARD_NOT_ERROR_);
		}
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT, "inCMAS_RunTRT() END!");
	}
	return (inRetVal);
}

/*
Function        :inCMASSrc_Display_Review
Date&Time       :2020/4/17 上午 10:11
Describe        :悠遊卡交易查詢
Author		:Hachi
 */
int inCMASSrc_Display_Review(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_SUCCESS;
	int inChoice = 0;
	int inTouchSensorFunc = _Touch_NEWUI_IDLE_;
	char szTemplate1[44 + 1] = {0};
	char szTemplate2[44 + 1] = {0};
	char szDispBuffer1[44 + 1] = {0};
	char szDispBuffer2[44 + 1] = {0};
	long long llSum = 0;
	long long llDeductSum = 0;
	long long llRefundSum = 0;
	long long llVoidDeductSum = 0;
	//long long llAddReturnSum = 0;
	long long llAutoAddSum = 0;
	unsigned char uszKey = 0x00;
	TICKET_ACCUM_TOTAL_REC srAccumRec = {0};

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT, "inCMASSrc_Display_Review start");
	}

	/* 讀交易資料，並放交易查詢Title */
	memset(&srAccumRec, 0x00, sizeof (TICKET_ACCUM_TOTAL_REC));
	inACCUM_GetRecord_ESVC(pobTran, &srAccumRec);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont_Color("購貨", _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_, _DISP_LEFT_);
	inDISP_ChineseFont_Color("退貨", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
	inDISP_ChineseFont_Color("現金加值", _FONTSIZE_8X44_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);
	/* 購貨筆數 */
	memset(szDispBuffer1, 0x00, sizeof (szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof (szTemplate1));

	llSum = srAccumRec.lnEASYCARD_DeductTotalCount;

	sprintf(szTemplate1, "%03lld", llSum);

	/* 購貨金額 */
	memset(szDispBuffer1, 0x00, sizeof (szDispBuffer1));
	memset(szTemplate2, 0x00, sizeof (szTemplate2));

	llSum = srAccumRec.llEASYCARD_DeductTotalAmount;
	llDeductSum = llSum;

	sprintf(szTemplate2, "%01lld", llSum);
	inFunc_Amount_Comma(szTemplate2, "$", 0x00, _SIGNED_NONE_, 11, _PADDING_RIGHT_);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_, _COLOR_WHITE_, 7);
	memset(szDispBuffer2, 0x00, sizeof (szDispBuffer2));
	sprintf(szDispBuffer2, "%s", szTemplate2);
	inDISP_EnglishFont_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_, _DISP_RIGHT_);

	/* 退貨筆數 */
	memset(szDispBuffer1, 0x00, sizeof (szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof (szTemplate1));

	llSum = srAccumRec.lnEASYCARD_RefundTotalCount;

	sprintf(szTemplate1, "%03lld", llSum);

	/* 退貨金額 */
	memset(szDispBuffer1, 0x00, sizeof (szDispBuffer1));
	memset(szTemplate2, 0x00, sizeof (szTemplate2));

	llSum = srAccumRec.llEASYCARD_RefundTotalAmount;
	llRefundSum = llSum;

	sprintf(szTemplate2, "%01lld", (0 - llSum));
	inFunc_Amount_Comma(szTemplate2, "$", 0x00, _SIGNED_NONE_, 11, _PADDING_RIGHT_);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _COLOR_WHITE_, 7);
	memset(szDispBuffer2, 0x00, sizeof (szDispBuffer2));
	sprintf(szDispBuffer2, "%s", szTemplate2);
	inDISP_EnglishFont_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_RIGHT_);

	/* 現金加值筆數 */
	memset(szDispBuffer1, 0x00, sizeof (szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof (szTemplate1));

	llSum = srAccumRec.lnEASYCARD_ADDTotalCount;

	sprintf(szTemplate1, "%03lld", llSum);

	/* 現金加值金額 */
	memset(szDispBuffer1, 0x00, sizeof (szDispBuffer1));
	memset(szTemplate2, 0x00, sizeof (szTemplate2));

	llSum = srAccumRec.llEASYCARD_ADDTotalAmount;

	sprintf(szTemplate2, "%01lld", llSum);
	inFunc_Amount_Comma(szTemplate2, "$", 0x00, _SIGNED_NONE_, 11, _PADDING_RIGHT_);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _COLOR_WHITE_, 7);
	memset(szDispBuffer2, 0x00, sizeof (szDispBuffer2));
	sprintf(szDispBuffer2, "%s", szTemplate2);
	inDISP_EnglishFont_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_RIGHT_);

	uszKey = 0x00;
	inTouchSensorFunc = _Touch_NEWUI_IDLE_;
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		uszKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		} else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		} else if (inChoice == _NEWUI_IDLE_Touch_KEY_FUNCTIONKEY_ ||
				uszKey == _KEY_ENTER_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	if (inRetVal == VS_USER_CANCEL ||
			inRetVal == VS_TIMEOUT)
	{
		return (inRetVal);
	}
	/*---------------------------------page2---------------------------------------*/
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont_Color("取消加值", _FONTSIZE_8X44_, _LINE_8_4_, _COLOR_BLACK_, _DISP_LEFT_);
	inDISP_ChineseFont_Color("自動加值", _FONTSIZE_8X44_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);

	/* 取消加值筆數 */
	memset(szDispBuffer1, 0x00, sizeof (szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof (szTemplate1));

	llSum = srAccumRec.lnEASYCARD_VoidADDTotalCount;
	sprintf(szTemplate1, "%03lld", llSum);

	/*取消加值金額*/
	memset(szDispBuffer1, 0x00, sizeof (szDispBuffer1));
	memset(szTemplate2, 0x00, sizeof (szTemplate2));

	llSum = srAccumRec.llEASYCARD_VoidADDTotalAmount;
	llVoidDeductSum = llSum;

	sprintf(szTemplate2, "%01lld", (0 - llSum));
	inFunc_Amount_Comma(szTemplate2, "$", 0x00, _SIGNED_NONE_, 11, _PADDING_RIGHT_);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_, _COLOR_WHITE_, 7);
	memset(szDispBuffer2, 0x00, sizeof (szDispBuffer2));
	sprintf(szDispBuffer2, "%s", szTemplate2);
	inDISP_EnglishFont_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_, _DISP_RIGHT_);

	/* 自動加值筆數 */
	memset(szDispBuffer1, 0x00, sizeof (szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof (szTemplate1));

	llSum = srAccumRec.lnEASYCARD_AutoADDTotalCount;
	sprintf(szTemplate1, "%03lld", llSum);

	/* 自動加值金額*/
	memset(szDispBuffer1, 0x00, sizeof (szDispBuffer1));
	memset(szTemplate2, 0x00, sizeof (szTemplate2));

	llSum = srAccumRec.llEASYCARD_AutoADDTotalAmount;
	llAutoAddSum = llSum;

	sprintf(szTemplate2, "%01lld", llSum);
	inFunc_Amount_Comma(szTemplate2, "$", 0x00, _SIGNED_NONE_, 11, _PADDING_RIGHT_);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _COLOR_WHITE_, 7);
	memset(szDispBuffer2, 0x00, sizeof (szDispBuffer2));
	sprintf(szDispBuffer2, "%s", szTemplate2);
	inDISP_EnglishFont_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_RIGHT_);

	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		uszKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		} else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		} else if (inChoice == _NEWUI_IDLE_Touch_KEY_FUNCTIONKEY_ ||
				uszKey == _KEY_ENTER_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	if (inRetVal == VS_USER_CANCEL ||
			inRetVal == VS_TIMEOUT)
	{
		return (inRetVal);
	}
	/*---------------------------------page3---------------------------------------*/
	/* 購貨總額 加值總額 回上一頁 回主畫面 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	// inDISP_PutGraphic(_REVIEW_TOTAL_3_, 0, _COORDINATE_Y_LINE_8_4_);
	inDISP_ChineseFont_Color("購貨總額", _FONTSIZE_8X44_, _LINE_8_4_, _COLOR_BLACK_, _DISP_LEFT_);
	inDISP_ChineseFont_Color("加值總額", _FONTSIZE_8X44_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);

	/* 購貨總額筆數 */
	memset(szDispBuffer1, 0x00, sizeof (szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof (szTemplate1));

	llSum = srAccumRec.lnEASYCARD_DeductTotalCount + srAccumRec.lnEASYCARD_RefundTotalCount;
	sprintf(szTemplate1, "%03lld", llSum);

	/* 購貨總額金額 */
	memset(szDispBuffer1, 0x00, sizeof (szDispBuffer1));
	memset(szTemplate2, 0x00, sizeof (szTemplate2));

	llSum = srAccumRec.llEASYCARD_DeductTotalAmount - srAccumRec.llEASYCARD_RefundTotalAmount;

	sprintf(szTemplate2, "%01lld", llSum);
	inFunc_Amount_Comma(szTemplate2, "$", 0x00, _SIGNED_NONE_, 11, _PADDING_RIGHT_);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_, _COLOR_WHITE_, 7);
	memset(szDispBuffer2, 0x00, sizeof (szDispBuffer2));
	sprintf(szDispBuffer2, "%s", szTemplate2);
	inDISP_EnglishFont_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_, _DISP_RIGHT_);

	/* 加值總額筆數 */
	llSum = srAccumRec.lnEASYCARD_ADDTotalCount + srAccumRec.lnEASYCARD_VoidADDTotalCount;
	sprintf(szTemplate1, "%03lld", llSum);

	memset(szDispBuffer1, 0x00, sizeof (szDispBuffer1));
	memset(szTemplate2, 0x00, sizeof (szTemplate2));

	/* 加值總額 */
	llSum = srAccumRec.llEASYCARD_ADDTotalAmount - srAccumRec.llEASYCARD_VoidADDTotalAmount;
	sprintf(szTemplate2, "%01lld", llSum);
	inFunc_Amount_Comma(szTemplate2, "$", 0x00, _SIGNED_NONE_, 11, _PADDING_RIGHT_);

	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _COLOR_WHITE_, 7);
	memset(szDispBuffer2, 0x00, sizeof (szDispBuffer2));
	sprintf(szDispBuffer2, "%s", szTemplate2);
	inDISP_EnglishFont_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_RIGHT_);

	uszKey = 0x00;
	inTouchSensorFunc = _Touch_NEWUI_REVIEW_TOTAL_;
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		uszKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (inChoice == _NEWUI_REVIEW_TOTAL_Touch_RETURN_IDLE_BUTTON_ ||
				uszKey == _KEY_ENTER_ ||
				uszKey == _KEY_CANCEL_ ||
				uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_SUCCESS;
			break;
		} else if (inChoice == _NEWUI_REVIEW_TOTAL_Touch_LAST_PAGE_BUTTON_ ||
				uszKey == _KEY_CLEAR_)
		{
			inRetVal = VS_LAST_PAGE;
			pobTran->srBRec.inHDTIndex = -1;
			break;
		}
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT, "inCMASSrc_Display_Review End(%d)", inRetVal);
	}

	return (inRetVal);
}

/*
Function		: inFunc_Booting_Flow_Eticket_Initial
Date&Time	: 2022/6/9 上午 11:22
Describe		: 一卡通與悠遊卡初始化
 * [新增電票悠遊卡功能]  [SAM] 2022/6/9 上午 11:26
 *  修改合庫執行流程，用 inFunc_Booting_Eticket_Initial_FISC 的內容來進行覆寫，
 *  並刪除原 Funcion 裏的 inFunc_Booting_Flow_Eticket_Initial 功能。
 */

/* 原功能會有一卡通功能，要使用請打把 0設為1 */
#if 0

int inFunc_Booting_Flow_Eticket_Initial(TRANSACTION_OBJECT *pobTran)
{
	char szTMSOK[2 + 1] = {0};
	char szCTLSEnable[2 + 1] = {0}; /* 觸控是否打開 */
	char szIPASSFlag[2 + 1];
	char szCMASFlag[2 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT, "----------------------------------------");
		inDISP_LogPrintfAt(AT, "inFunc_Booting_Flow_Eticket_Initial() START !");
	}

	memset(szIPASSFlag, 0x00, sizeof (szIPASSFlag));
	memset(szCMASFlag, 0x00, sizeof (szCMASFlag));

	inGetIPASSFuncEnable(szIPASSFlag);
	inGetCMASFuncEnable(szCMASFlag);

	if (memcmp(szIPASSFlag, "Y", 1) != 0 && memcmp(szCMASFlag, "Y", 1) != 0)
	{
		return (VS_SUCCESS);
	}

	/* 確認是否鎖機 */
	/* TMS有下才跑下面*/
	/* 判斷是否有開感應 */
	memset(szTMSOK, 0x00, sizeof (szTMSOK));
	inGetTMSOK(szTMSOK);
	if ((memcmp(szTMSOK, "Y", 1) == 0 && inFunc_Check_EDCLock() == VS_SUCCESS))
	{
		/* 目前裡面不設定網路，由端末機控制 */
		/* 若找不到該票證的SAM卡，則不能感應 */

		memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
		inGetContactlessEnable(szCTLSEnable);

		if (!memcmp(&szCTLSEnable[0], "Y", 1))
		{
			/*一卡通HOST有開*/
			if (memcmp(szIPASSFlag, "Y", 1) == 0)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
				inDISP_PutGraphic(_MENU_IPASS_LOGON_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
				inFunc_Booting_Flow_IPASS2_AUTO_SIGNON(pobTran);
			}
			/*悠遊卡HOST有開*/
			if (memcmp(szCMASFlag, "Y", 1) == 0)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
				inDISP_PutGraphic(_MENU_CMAS_LOGON_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
				inCMAS_Init_Flow(pobTran);
			}
		}
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT, "inFunc_Booting_Flow_Eticket_Initial() END !");
		inDISP_LogPrintfAt(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}
#else

int inFunc_Booting_Flow_Eticket_Initial(TRANSACTION_OBJECT *pobTran)
{
	char szTMSOK[2 + 1] = {0};
	char szCTLSEnable[2 + 1] = {0}; /* 觸控是否打開 */
	char szCMASFlag[2 + 1];

#ifndef __ENABLE_CMAS_INIT__	
	inDISP_LogPrintfAt(AT, "Eticket Function Not Enable ");
	return VS_SUCCESS;
#endif

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT, "----------------------------------------");
		inDISP_LogPrintfAt(AT, "inFunc_Booting_Flow_Eticket_Initial() START !");
	}

	memset(szCMASFlag, 0x00, sizeof (szCMASFlag));

	inGetCMASFuncEnable(szCMASFlag);
	if (memcmp(szCMASFlag, "Y", 1) != 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintfAt(AT, " inFunc_Booting_Flow_Eticket_Initial *Error* Cmas[%s] END", szCMASFlag);
			inDISP_LogPrintfAt(AT, "----------------------------------------");
		}
		return (VS_SUCCESS);
	}

	/* 確認是否鎖機 */
	/* TMS有下才跑下面*/
	/* 判斷是否有開感應 */
	memset(szTMSOK, 0x00, sizeof (szTMSOK));
	inGetTMSOK(szTMSOK);
	if ((memcmp(szTMSOK, "Y", 1) == 0 && inFunc_Check_EDCLock() == VS_SUCCESS))
	{
		/* 目前裡面不設定網路，由端末機控制 */
		/* 若找不到該票證的SAM卡，則不能感應 */

		memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
		inGetContactlessEnable(szCTLSEnable);

		if (!memcmp(&szCTLSEnable[0], "Y", 1))
		{
			/*悠遊卡HOST有開*/
			if (memcmp(szCMASFlag, "Y", 1) == 0)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
				//TODO: 先拿掉顯示的圖示,看不要要補回				
//				inDISP_PutGraphic(_MENU_CMAS_LOGON_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
				inCMAS_Init_Flow(pobTran);
			}
		}
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT, "inFunc_Booting_Flow_Eticket_Initial() END !");
		inDISP_LogPrintfAt(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}
#endif


