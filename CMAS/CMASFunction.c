
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>
#include <libxml/parser.h>

#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/COMM/Comm.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/TDT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"

#include "../SOURCE/COMM/Ethernet.h"
#include "../SOURCE/PRINT/Print.h"
#include "../CTLS/CTLS.h"
//#include "../NCCC/NCCCTicketSrc.h"
//#include "../NCCC/NCCCTicketIso.h"
//#include "../NCCC/NCCCtms.h"
#include "../NCCC/NCCCtSAM.h"

#include "../ECC/ICER/stdAfx.h"
#include "../ECC/ICER/libutil.h"
#include "../ECC/ICER/ICERAPI.h"

#include "../ECC/ECC.h"

#include "CMASAPIISO.h"
//#include "CMASprtByBuffer.h"
#include "CMASFunction.h"

extern int ginDebug; /* Debug使用 extern */
extern char guszECCRetryBit;
extern unsigned long gulDemoTicketPoint; /* DEMO用 */
extern int ginAPVersionType;
extern int ginCMASDirLink; /* 20200406 用來判斷悠遊卡是直連還是間連[Hachi] */
extern int ginFindRunTime;

/*Easy Card api的概念是,程式要先生成
	ICERAPI.REQ與ICERAPI.REQ.OK
	ICERAPI.REQ是真正的檔案,ICERAPI.REQ.OK是個空檔案
	執行ICERApi_exe()進行交易
	會產出ICERAPI.REQ後再產出ICERAPI.REQ.OK表示產生ICERAPI.REQ已完成
	另外ICERINI.xml為ICERApi_exe()的參數*/

/*
Function        :inCMAS_CheckTermStatus
Date&Time       :2020/01/09 下午 3:45
Describe        :檢查是否鎖機、檢查是否下過參數
Author          :Hachi
 */
int inCMAS_CheckTermStatus(TRANSACTION_OBJECT *pobTran) {
	char szTMSOK[2 + 1] = {0};
	char szDemoMode[2 + 1] = {0};

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "----------------------------------------");
		inDISP_LogPrintfAt(AT, "inCMAS_CheckTermStatus() START !");
	}

	/* 檢查是否已經鎖機 */
	if (inFunc_Check_EDCLock() != VS_SUCCESS)
		return (VS_ERROR);

	/* 檢查是否做過【參數下載】 */
	memset(szTMSOK, 0x00, sizeof (szTMSOK));
	inGetTMSOK(szTMSOK);
	if (memcmp(szTMSOK, "Y", 1) != 0) {
		inDISP_Msg_BMP(_ERR_TMS_DWL_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

		return (VS_ERROR);
	}

	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof (szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0) {
		pobTran->srTRec.inTicketType = _TICKET_TYPE_ECC_;
		pobTran->srTRec.uszESVCTransBit = VS_TRUE;

		if (ginDebug == VS_TRUE) {
			inDISP_LogPrintfAt(AT, "inCMAS_CheckTermStatus() END !");
			inDISP_LogPrintfAt(AT, "----------------------------------------");
		}

		return (VS_SUCCESS);
	} else {
		pobTran->srTRec.inTicketType = _TICKET_TYPE_ECC_;
		pobTran->srTRec.uszESVCTransBit = VS_TRUE;

		if (ginDebug == VS_TRUE) {
			inDISP_LogPrintfAt(AT, "inCMAS_CheckTermStatus() END !");
			inDISP_LogPrintfAt(AT, "----------------------------------------");
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inCMAS_Decide_Trans_Type
Date&Time       :2020/01/09 下午 3:45
Describe        :load HDT, HDPT；給incode, inTransactionCode, TRT
Author          :Hachi
 */
int inCMAS_Decide_Trans_Type(TRANSACTION_OBJECT *pobTran) {
	int inESVCIndex = -1;
	char szDebugMsg[100 + 1];

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "----------------------------------------");
		inDISP_LogPrintfAt(AT, "inCMAS_Decide_Trans_Type() START !");
		inDISP_LogPrintfAt(AT, "(pobTran->srTRec.inTicketType is %d", pobTran->srTRec.inTicketType);
	}

	/* 票證 TRT 分開寫 */
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_) {
		/* MFT index 紀錄 HDT index */
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CMAS_, &inESVCIndex);
		pobTran->srBRec.inHDTIndex = inESVCIndex; //這邊要給HDT,否則 inEVENT_Responder 跑完OPT後 跑不進TRT
		if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) < 0)
			return (VS_ERROR);
		
		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) < 0)
			return (VS_ERROR);

		if (pobTran->inTransactionCode == _TICKET_EASYCARD_DEDUCT_) {
			pobTran->inTransactionCode = _TICKET_EASYCARD_DEDUCT_;
			pobTran->srTRec.inCode = _TICKET_EASYCARD_DEDUCT_;
			pobTran->inRunTRTID = _TRT_CMAS_DEDUCT_;
		} else if (pobTran->inTransactionCode == _TICKET_EASYCARD_REFUND_) {
			pobTran->inTransactionCode = _TICKET_EASYCARD_REFUND_;
			pobTran->srTRec.inCode = _TICKET_EASYCARD_REFUND_;
			pobTran->inRunTRTID = _TRT_CMAS_REFUND_;
		} else if (pobTran->inTransactionCode == _TICKET_EASYCARD_INQUIRY_) {
			pobTran->inTransactionCode = _TICKET_EASYCARD_INQUIRY_;
			pobTran->srTRec.inCode = _TICKET_EASYCARD_INQUIRY_;
			pobTran->inRunTRTID = _TRT_CMAS_QUERY_;
		} else if (pobTran->inTransactionCode == _TICKET_EASYCARD_TOP_UP_) {
			pobTran->inTransactionCode = _TICKET_EASYCARD_TOP_UP_;
			pobTran->srTRec.inCode = _TICKET_EASYCARD_TOP_UP_;
			pobTran->inRunTRTID = _TRT_CMAS_ADD_VALUE_;
		} else if (pobTran->inTransactionCode == _TICKET_EASYCARD_VOID_TOP_UP_) {
			pobTran->inTransactionCode = _TICKET_EASYCARD_VOID_TOP_UP_;
			pobTran->srTRec.inCode = _TICKET_EASYCARD_VOID_TOP_UP_;
			pobTran->inRunTRTID = _TRT_CMAS_VOID_ADD_VALUE_;
		} else if (pobTran->inTransactionCode == _SETTLE_) {
			pobTran->inTransactionCode = _SETTLE_;
			pobTran->srTRec.inCode = _SETTLE_;
			pobTran->inRunTRTID = _TRT_CMAS_SETTLE_;
		} else {
			if (ginDebug == VS_TRUE) {
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "EASYCARD 無此交易別");
				inDISP_LogPrintfAt(AT, szDebugMsg);
			}
			return (VS_ERROR);
		}
	}
	if (pobTran->srBRec.lnTxnAmount >= 0)
		pobTran->srTRec.lnTxnAmount = pobTran->srBRec.lnTxnAmount;

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "inCMAS_Decide_Trans_Type() END !");
		inDISP_LogPrintfAt(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_APIFlow
Date&Time   :2020/01/09 下午 3:45
Describe        :決定交易FLOW
Author           :Hachi
 *  [新增電票悠遊卡功能]  主要執行感應卡片及對應的電票流程  [SAM]  2022/6/21 下午 6:01
 */
int inCMAS_APIFlow(TRANSACTION_OBJECT *pobTran) {
	int inRetVal = VS_ERROR;
	int inRetVal2 = VS_ERROR;
	int inOrgCode = _TRANS_TYPE_NULL_;
	char szDebugMsg[100 + 1] = {0};
	char szDemoMode[2 + 1] = {0};
	char szTxnType[20 + 1] = {0};

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "inCMAS_APIFlow start!!!");
		inDISP_LogPrintfAt(AT, "pobTran->srTRec.inCode == %d", pobTran->srTRec.inCode);
	}

	inDISP_ClearAll();
	inRetVal = inCMAS_Fast_Tap_Wait(pobTran, pobTran->srTRec.szUID); /*20200620[Hachi]*/
	if (inRetVal != VS_SUCCESS) {
		return (inRetVal);
	}
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("交易進行中", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
	inDISP_ChineseFont("請勿移動票卡", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);

	memset(szDemoMode, 0x00, sizeof (szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0) {
		memset(pobTran->srTRec.szUID, 0x00, sizeof (pobTran->srTRec.szUID));
		memcpy(&pobTran->srTRec.szUID[0], "B123456789", 10);

		pobTran->srTRec.lnCardRemainAmount = gulDemoTicketPoint;
		pobTran->srTRec.lnFinalBeforeAmt = gulDemoTicketPoint;
		pobTran->srTRec.lnTopUpAmount = 500; /* 自動加值金額(元/次) */

		/* 悠遊卡簽單有RRN及自己的Date Time */
		memcpy(&pobTran->srTRec.srECCRec.szRRN[0], &pobTran->srTRec.szRefNo[0], 12);
		strcpy(pobTran->srTRec.srECCRec.szDate, "20");
		memcpy(&pobTran->srTRec.srECCRec.szDate[2], &pobTran->srTRec.szDate[0], 6);
		memcpy(&pobTran->srTRec.srECCRec.szTime[0], &pobTran->srTRec.szTime[0], 6);

		if (pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_) {
			if ((pobTran->srTRec.lnFinalBeforeAmt - pobTran->srTRec.lnTxnAmount) >= 0) {
				pobTran->srTRec.lnFinalAfterAmt = pobTran->srTRec.lnFinalBeforeAmt - pobTran->srTRec.lnTxnAmount;
				gulDemoTicketPoint = pobTran->srTRec.lnFinalAfterAmt;
				inRetVal = VS_SUCCESS;
			} else {
				//TODO: 目前這個交易開關會在TMS後初始化,應該是沒有用，要再看怎麼改。
				/* 交易開關 */
				memset(szTxnType, 0x00, sizeof (szTxnType));
				inGetTicket_HostTransFunc(szTxnType);

				if (szTxnType[3] == 'Y' && pobTran->srTRec.lnTxnAmount >= 500) {
					inRetVal = VS_SUCCESS;
					/* 先取得自動加值金額 */
					pobTran->srTRec.lnTotalTopUpAmount = 0;

					if (inNCCC_Ticket_Top_Up_Amount_Check(pobTran) != VS_SUCCESS) {
						/* 自動加值金額過大 */
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						inDISP_ChineseFont("餘額不足", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
						inDISP_BEEP(3, 500);

						if (ginDebug == VS_TRUE) {
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "inNCCC_Ticket_IPASS_Query_Flow()_自動加值金額過大");
							inDISP_LogPrintfAt(AT, szDebugMsg);
						}
						inRetVal = VS_ERROR;
					}

					if (inRetVal == VS_SUCCESS) {
						/* 負值不可交易 */
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						inDISP_ChineseFont("餘額不足", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
						inDISP_ChineseFont("進行自動加值", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);

						pobTran->inTransactionCode = _TICKET_EASYCARD_AUTO_TOP_UP_;
						pobTran->srTRec.inCode = _TICKET_EASYCARD_AUTO_TOP_UP_;

						gulDemoTicketPoint += pobTran->srTRec.lnTotalTopUpAmount;
					}

					if (inRetVal == VS_SUCCESS) {
						if (inFLOW_RunFunction(pobTran, _CMAS_UPDATE_ACCUM_) != VS_SUCCESS) {
							inRetVal = VS_ERROR;
						}
					}

					if (inRetVal == VS_SUCCESS) {
						if (inFLOW_RunFunction(pobTran, _CMAS_UPDATE_BATCH_) != VS_SUCCESS) {
							inRetVal = VS_ERROR;
						}
					}

					if (inRetVal == VS_SUCCESS) {
						if (inFLOW_RunFunction(pobTran, _CMAS_UPDATE_INV_) != VS_SUCCESS) {
							inRetVal = VS_ERROR;
						}
					}

					if (inRetVal == VS_SUCCESS) {
						pobTran->inTransactionCode = _TICKET_EASYCARD_DEDUCT_;
						pobTran->srTRec.inCode = _TICKET_EASYCARD_DEDUCT_;

						pobTran->srTRec.lnFinalAfterAmt = gulDemoTicketPoint - pobTran->srTRec.lnTxnAmount;
						gulDemoTicketPoint = pobTran->srTRec.lnFinalAfterAmt;
					}

					if (inRetVal == VS_SUCCESS) {
						if (inFLOW_RunFunction(pobTran, _CMAS_GET_PARM_) != VS_SUCCESS) {
							inRetVal = VS_ERROR;
						}
					}
				} else {
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("餘額不足", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
					inDISP_BEEP(3, 500);
					inRetVal = VS_ERROR;
				}
			}

			memset(pobTran->srTRec.szTicketRefundCode, 0x00, sizeof (pobTran->srTRec.szTicketRefundCode));
			memcpy(&pobTran->srTRec.szTicketRefundCode[0], "333333", 6);
		} else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_) {
			pobTran->srTRec.lnFinalAfterAmt = pobTran->srTRec.lnFinalBeforeAmt + pobTran->srTRec.lnTxnAmount;
			gulDemoTicketPoint = pobTran->srTRec.lnFinalAfterAmt;
			inRetVal = VS_SUCCESS;
		} else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_) {
			if (pobTran->srTRec.lnFinalBeforeAmt - pobTran->srTRec.lnTxnAmount < 0) {
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("餘額不足", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_BEEP(3, 500);
				inRetVal = VS_ERROR;
			} else {
				pobTran->srTRec.lnFinalAfterAmt = pobTran->srTRec.lnFinalBeforeAmt - pobTran->srTRec.lnTxnAmount;
				gulDemoTicketPoint = pobTran->srTRec.lnFinalAfterAmt;
				inRetVal = VS_SUCCESS;
			}
		} else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_) {
			inRetVal = VS_SUCCESS;
		}
	} else {
		if (pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_) {
			pobTran->srTRec.lnTotalTopUpAmount = 0;
			inRetVal = inCMAS_Deduct_Flow(pobTran);

			/*20210407  參考聯合code新增自動加值流程 start [Hachi]*/
			/* 有可能購貨失敗但自動加值成功，所以不能在這裡跳出 */
			/* 檢核是否有自動加值 */
			if (pobTran->srTRec.lnTotalTopUpAmount > 0) {
				/* 存原交易別 */
				inOrgCode = pobTran->srTRec.inCode;
				inRetVal2 = inCMAS_AutoAdd_Flow(pobTran);
				/* 回復原交易別 */
				pobTran->srTRec.inCode = inOrgCode;
				if (inRetVal2 != VS_SUCCESS) {
					inRetVal = VS_ERROR;
				}
			}
			/*20210407  參考聯合code新增自動加值流程 end [Hachi]*/
		} else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_) {
			inRetVal = inCMAS_Refund_Flow(pobTran);
		} else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_) {
			inRetVal = inCMAS_TOP_UP_Flow(pobTran);
		} else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_) {
			inRetVal = inCMAS_Void_TOP_UP_Flow(pobTran);
		} else if (pobTran->srTRec.inCode == _SETTLE_) {
			inRetVal = inCMAS_Settle_Flow(pobTran);
		} else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_) {
			inRetVal = inCMAS_Inquiry_Flow(pobTran);
		} else {
			if (ginDebug == VS_TRUE) {
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "無對應ECC incode: %d", pobTran->srTRec.inCode);
				inDISP_LogPrintfAt(AT, szDebugMsg);
			}

			inRetVal = VS_ERROR;
		}
	}

	if (inRetVal == VS_SUCCESS) {
		inDISP_BEEP(1, 0);
		inCTLS_Set_LED(_CTLS_LIGHT_GREEN_);
	} else {
		/*錯誤畢聲與燈號在inCMAS_ResponeCode裡面顯示，避免畫面與燈號不同步問題*/
	}

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "inCMAS_APIFlow end!!!");
	}

	return (inRetVal);
}

/*
Function        :inCMAS_Deduct_Flow
Date&Time       :2020/01/17 下午 3:45
Describe        :悠遊卡購貨流程
Author          :Hachi
 */
int inCMAS_Deduct_Flow(TRANSACTION_OBJECT *pobTran) {
	int inRetVal = VS_ERROR;
	int inRetry = 1;
	char szKey = 0x00;
	char szAPI_RespCode[10];
	char szCMASAddTcpipMode [ 2 + 1 ] = {0};

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "----------------------------------------");
		inDISP_LogPrintfAt(AT, "inCMAS_Deduct_Flow() START !");
	}
	/* 初始化是否已重試 */
	guszECCRetryBit = VS_FALSE;

	memset(szCMASAddTcpipMode, 0x00, sizeof (szCMASAddTcpipMode));
	inGetCMASAddTcpipMode(szCMASAddTcpipMode);

	while (inRetry <= 5) {
		/* 組電文(生成ICERAPI.REQ) */
		inCMAS_PACK_Deduct_ICERAPI(pobTran); /*參考inECC_PACK_Deduct_ICERAPI(pobTran);*/

		/* AdditionalTcpipData */
		/*見規格書2.16 AddtionalTcpipData !=0 時才需要ICERAPI2.REQ、ICERAPI2.REQ*/
		if (szCMASAddTcpipMode[0] == '0') {
			/*直連不帶*/
		} else if (szCMASAddTcpipMode[0] == '1') {
			/*NCCC*/
		} else if (szCMASAddTcpipMode[0] == '2') {
			/*CTCB*/
		} else if (szCMASAddTcpipMode[0] == '3') {
			/*FISC*/
			inCMAS_PACK_Deduct_ICERAPI_FISC(pobTran);
		}

		if (ginDebug == VS_TRUE) {
			inDISP_LogPrintfAt(AT, "ICER Start");
		}

		inRetVal = ICERApi_exe();

		if (ginDebug == VS_TRUE) {
			inDISP_LogPrintfAt(AT, "ICER End");
		}

		/* 解電文(分析ICERAPI.RES) */
		inRetVal = inCMAS_UNPACK_ICERAPI(pobTran, _CMAS_API_RES_FILE_);

		if (inRetVal != VS_SUCCESS) {
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inCTLS_Set_LED(_CTLS_LIGHT_RED_);
			inDISP_ChineseFont("悠遊卡檔案異常", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
			inDISP_BEEP(3, 500);

			return (VS_ERROR);
		}

		/* 檢核回應碼 <T3901>開始 */
		if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "0", 1) != 0) {
			/* -125 = 須執行Retry */
			/* -119 : Call相關DLL回應錯誤 ; 請參考T3904 */
			/* 62 : RC531錯誤，交易中止，請重新交易*/
			if ((memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-125", 4) == 0) ||
					(memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-119", 4) == 0 &&
					memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "62", 2) == 0 &&
					guszECCRetryBit == VS_TRUE)) {
				if (inRetry >= 4) {
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("請重新交易", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
					inDISP_BEEP(3, 500);
					return (VS_ERROR);
				}

				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				//inDISP_ChineseFont("連線失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				sprintf(szAPI_RespCode, "%s", pobTran->srTRec.srECCRec.szAPI_RespCode);
				inDISP_ChineseFont(szAPI_RespCode, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont("請重新感應卡片", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_ChineseFont("並按確認鍵重試", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);

				szKey = 0x00;
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, 60);

				while (1) {
					szKey = uszKBD_Key();
					if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS) {
						szKey = _KEY_TIMEOUT_;
					}

					if (szKey == _KEY_ENTER_) {
						inRetry++;
						guszECCRetryBit = VS_TRUE;
						break;
					} else if (szKey == _KEY_TIMEOUT_) {
						return (VS_ERROR);
					}
				}

				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("交易處理中", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_ChineseFont("請重新感應卡片", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);

				continue;
			} else {
				/* 其他錯誤Display */
				inCMAS_ResponeCode(pobTran);

				return (VS_ERROR);
			}
		}

		/* 抓必要資料*/
		/* 20200415如果是間連，需要額外去UNPACK */
		if (szCMASAddTcpipMode[0] != '0') {
			/* [先移除功能要再加回] 2022/6/8 [SAM] */
			//			if (inECC_UNPACK_ICERAPI_NCCC(pobTran, _CMAS_API_RES_FILE2_, _CMAS_FOLDER_PATH_) != VS_SUCCESS)
			//			{
			//				inDISP_LogPrintfAt(AT,"inECC_UNPACK_ICERAPI_NCCC VSERROR");
			//				return (VS_ERROR);
			//			}
			//			END
		}
		break;
	}

	/*
	 * T5501批次號碼之規則為yymmddss+流水號兩碼
	 * 以當批第一筆成功交易時間為準
	 * 如果NeedNewBatch是'Y'，代表是第一筆，更新狀態為不用更新批號
	 * 如果NeedNewBatch是'N'，代表有做過交易
	 */
	inCMAS_Batch_Update();

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "inCMAS_Deduct_Flow() END !");
		inDISP_LogPrintfAt(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_Refund_Flow
Date&Time       :2020/2/11 下午 5:30
Describe        :退貨流程
Author          :Hachi
 */
int inCMAS_Refund_Flow(TRANSACTION_OBJECT *pobTran) {
	int inRetVal = VS_ERROR;
	int inRetry = 1;
	char szKey = 0x00;
	char szAPI_RespCode[10];
	char szCustomerIndicator[3 + 1] = {0};
	char szDebugMsg[100 + 1] = {0};
	char szCMASAddTcpipMode[2 + 1] = {0};
	// unsigned char uszECRAlreadyBit = VS_FALSE;

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "----------------------------------------");
		inDISP_LogPrintfAt(AT, "inCMAS_Refund_Flow() START !");
	}

	memset(szCustomerIndicator, 0x00, sizeof (szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	guszECCRetryBit = VS_FALSE;

	memset(szCMASAddTcpipMode, 0x00, sizeof (szCMASAddTcpipMode));
	inGetCMASAddTcpipMode(szCMASAddTcpipMode);

	while (inRetry <= 5) {
		/* 組電文(生成ICERAPI.REQ) */
		inCMAS_PACK_Refund_ICERAPI(pobTran);

		/* AdditionalTcpipData */
		if (szCMASAddTcpipMode[0] == '0') {
			/*直連不帶*/
		} else if (szCMASAddTcpipMode[0] == '1') {
			/*NCCC*/
		} else if (szCMASAddTcpipMode[0] == '2') {
			/*CTCB*/
		} else if (szCMASAddTcpipMode[0] == '3') {
			/*FISC*/
			inCMAS_PACK_Deduct_ICERAPI_FISC(pobTran);
		}

		if (ginDebug == VS_TRUE) {
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "ICER Start");
			inDISP_LogPrintfAt(AT, szDebugMsg);
		}

		inRetVal = ICERApi_exe();

		if (ginDebug == VS_TRUE) {
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "ICER End");
			inDISP_LogPrintfAt(AT, szDebugMsg);
		}

		/* 解電文(分析ICERAPI.RES) */
		inRetVal = inCMAS_UNPACK_ICERAPI(pobTran, _CMAS_API_RES_FILE_);

		if (inRetVal != VS_SUCCESS) {
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inCTLS_Set_LED(_CTLS_LIGHT_RED_);
			inDISP_ChineseFont("悠遊卡檔案異常", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
			inDISP_BEEP(3, 1000);

			return (VS_ERROR);
		}

		/* 檢核回應碼 <T3901>開始 */
		if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "0", 1) != 0) {

			/* -125 = Retry */
			/* -119 : Call相關DLL回應錯誤 ; 請參考T3904 */
			/* 62 : RC531錯誤，交易中止，請重新交易*/
			if ((memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-125", 4) == 0) ||
					(memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-119", 4) == 0 &&
					memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "62", 2) == 0 &&
					guszECCRetryBit == VS_TRUE)) {
				if (inRetry >= 4) {
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("請重新交易", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
					inDISP_BEEP(3, 1000);
					return (VS_ERROR);
				}

				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				//inDISP_ChineseFont("連線失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				sprintf(szAPI_RespCode, "%s", pobTran->srTRec.srECCRec.szAPI_RespCode);
				inDISP_ChineseFont(szAPI_RespCode, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont("請重新感應卡片", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_ChineseFont("並按確認鍵重試", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);

				szKey = 0x00;
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, 60);

				while (1) {
					szKey = uszKBD_Key();
					if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS) {
						szKey = _KEY_TIMEOUT_;
					}

					if (szKey == _KEY_ENTER_) {
						inRetry++;
						guszECCRetryBit = VS_TRUE;
						break;
					} else if (szKey == _KEY_TIMEOUT_) {
						return (VS_ERROR);
					}
				}

				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("交易處理中", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_ChineseFont("請重新感應卡片", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);

				continue;
			} else {
				/* 其他錯誤Display */
				inCMAS_ResponeCode(pobTran);

				return (VS_ERROR);
			}

		}
		/* 抓必要資料 */
		/* 2020.03.20 若AdditionalTcpipData = 0 ，則不需要unpack ICERAPI2 [Hachi] */
		if (szCMASAddTcpipMode[0] != '0') {
			/* [先移除功能要再加回] 2022/6/8 [SAM] */
			//			if (inECC_UNPACK_ICERAPI_NCCC(pobTran, _CMAS_API_RES_FILE2_, _CMAS_FOLDER_PATH_) != VS_SUCCESS)
			//				return (VS_ERROR);
			//			END
		}
		break;
	}

	/*
	 * T5501批次號碼之規則為yymmddss+流水號兩碼
	 * 以當批第一筆成功交易時間為準
	 * 如果NeedNewBatch是'Y'，代表是第一筆，更新狀態為不用更新批號
	 * 如果NeedNewBatch是'N'，代表有做過交易
	 */
	inCMAS_Batch_Update();

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "inCMAS_Refund_Flow() END !");
		inDISP_LogPrintfAt(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_TOP_UP_Flow
Date&Time       :2020/2/11 下午 5:30
Describe        :現金加值流程
Author          :Hachi
 */
int inCMAS_TOP_UP_Flow(TRANSACTION_OBJECT *pobTran) {
	int inRetVal = VS_ERROR;
	int inRetry = 1;
	char szKey = 0x00;
	char szAPI_RespCode[10];
	char szCMASAddTcpipMode[2 + 1] = {0};
	// char szDebugMsg[100 + 1] = {0};
	// unsigned char uszECRAlreadyBit = VS_FALSE;

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "----------------------------------------");
		inDISP_LogPrintfAt(AT, "inCMAS_TOP_UP_Flow() START !");
	}

	guszECCRetryBit = VS_FALSE;

	memset(szCMASAddTcpipMode, 0x00, sizeof (szCMASAddTcpipMode));
	inGetCMASAddTcpipMode(szCMASAddTcpipMode);

	while (inRetry <= 5) {
		/* 組電文(生成ICERAPI.REQ) */
		inCMAS_PACK_TOP_UP_ICERAPI(pobTran);

		/* AdditionalTcpipData */
		if (szCMASAddTcpipMode[0] == '0') {
			/*直連不帶*/
		} else if (szCMASAddTcpipMode[0] == '1') {
			/*NCCC*/
		} else if (szCMASAddTcpipMode[0] == '2') {
			/*CTCB*/
		} else if (szCMASAddTcpipMode[0] == '3') {
			/*FISC*/
			inCMAS_PACK_Deduct_ICERAPI_FISC(pobTran);
		}

		if (ginDebug == VS_TRUE) {
			inDISP_LogPrintfAt(AT, "ICER Start");
		}

		inRetVal = ICERApi_exe();


		if (ginDebug == VS_TRUE) {
			inDISP_LogPrintfAt(AT, "ICER End");
		}

		/* 解電文(分析ICERAPI.RES) */
		inRetVal = inCMAS_UNPACK_ICERAPI(pobTran, _CMAS_API_RES_FILE_);

		if (inRetVal != VS_SUCCESS) {
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inCTLS_Set_LED(_CTLS_LIGHT_RED_);
			inDISP_ChineseFont("悠遊卡檔案異常", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
			inDISP_BEEP(3, 1000);

			return (VS_ERROR);
		}

		/* 檢核回應碼 <T3901>開始 */
		if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "0", 1) != 0) {
			/* -125 = Retry */
			/* -119 : Call相關DLL回應錯誤 ; 請參考T3904 */
			/* 62 : RC531錯誤，交易中止，請重新交易*/
			if ((memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-125", 4) == 0) ||
					(memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-119", 4) == 0 &&
					memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "62", 2) == 0 &&
					guszECCRetryBit == VS_TRUE)) {
				if (inRetry >= 4) {
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("請重新交易", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
					inDISP_BEEP(3, 1000);
					return (VS_ERROR);
				}

				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				//inDISP_ChineseFont("連線失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				sprintf(szAPI_RespCode, "%s", pobTran->srTRec.srECCRec.szAPI_RespCode);
				inDISP_ChineseFont(szAPI_RespCode, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont("請重新感應卡片", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_ChineseFont("並按確認鍵重試", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);

				szKey = 0x00;
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, 60);

				while (1) {
					szKey = uszKBD_Key();
					if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS) {
						szKey = _KEY_TIMEOUT_;
					}

					if (szKey == _KEY_ENTER_) {
						inRetry++;
						guszECCRetryBit = VS_TRUE;
						break;
					} else if (szKey == _KEY_TIMEOUT_) {
						return (VS_ERROR);
					}
				}

				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("交易處理中", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_ChineseFont("請重新感應卡片", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);

				continue;
			} else {
				/* 其他錯誤Display */
				inCMAS_ResponeCode(pobTran);

				return (VS_ERROR);
			}
		}

		/* 抓必要資料 */
		/* 2020.03.20 若AdditionalTcpipData = 0 ，則不需要unpack ICERAPI2 [Hachi] */
		if (szCMASAddTcpipMode[0] != '0') {
			/* [先移除功能要再加回] 2022/6/8 [SAM] */
			//			if (inECC_UNPACK_ICERAPI_NCCC(pobTran, _CMAS_API_RES_FILE2_, _CMAS_FOLDER_PATH_) != VS_SUCCESS)
			//				return (VS_ERROR);
			//			END
		}
		break;
	}
	/*
	 * T5501批次號碼之規則為yymmddss+流水號兩碼
	 * 以當批第一筆成功交易時間為準
	 * 如果NeedNewBatch是'Y'，代表是第一筆，更新狀態為不用更新批號
	 * 如果NeedNewBatch是'N'，代表有做過交易
	 */

	inCMAS_Batch_Update();

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "inCMAS_TOP_UP_Flow() END !");
		inDISP_LogPrintfAt(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_Void_TOP_UP_Flow
Date&Time       :2020/2/11 下午 5:30
Describe        :
Author          :Hachi
 */
int inCMAS_Void_TOP_UP_Flow(TRANSACTION_OBJECT *pobTran) {
	int inRetVal = VS_ERROR;
	int inRetry = 1;
	char szKey = 0x00;
	char szAPI_RespCode[10];
	char szCMASAddTcpipMode[2 + 1] = {0};
	//char szDebugMsg[100 + 1] = {0};
	//unsigned char uszECRAlreadyBit = VS_FALSE;

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "----------------------------------------");
		inDISP_LogPrintfAt(AT, "inCMAS_Void_TOP_UP_Flow() START !");
	}

	guszECCRetryBit = VS_FALSE;

	memset(szCMASAddTcpipMode, 0x00, sizeof (szCMASAddTcpipMode));
	inGetCMASAddTcpipMode(szCMASAddTcpipMode);

	while (inRetry <= 5) {
		/* 組電文(生成ICERAPI.REQ) */
		inCMAS_PACK_Void_TOP_UP_ICERAPI(pobTran);

		/* AdditionalTcpipData */
		/*見規格書2.16 AddtionalTcpipData !=0 時才需要ICERAPI2.REQ、ICERAPI2.REQ*/
		if (szCMASAddTcpipMode[0] == '0') {
			/*直連不帶*/
		} else if (szCMASAddTcpipMode[0] == '1') {
			/*NCCC*/
		} else if (szCMASAddTcpipMode[0] == '2') {
			/*CTCB*/
		} else if (szCMASAddTcpipMode[0] == '3') {
			/*FISC*/
			inCMAS_PACK_Deduct_ICERAPI_FISC(pobTran);
		}

		if (ginDebug == VS_TRUE) {
			inDISP_LogPrintfAt(AT, "ICER Start");
		}

		inRetVal = ICERApi_exe();


		if (ginDebug == VS_TRUE) {
			inDISP_LogPrintfAt(AT, "ICER End");
		}

		/* 解電文(分析ICERAPI.RES) */
		inRetVal = inCMAS_UNPACK_ICERAPI(pobTran, _CMAS_API_RES_FILE_);

		if (inRetVal != VS_SUCCESS) {
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inCTLS_Set_LED(_CTLS_LIGHT_RED_);
			inDISP_ChineseFont("悠遊卡檔案異常", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
			inDISP_BEEP(3, 1000);

			return (VS_ERROR);
		}

		/* 檢核回應碼 <T3901>開始 */
		if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "0", 1) != 0) {
			/* -125 = Retry */
			/* -119 : Call相關DLL回應錯誤 ; 請參考T3904 */
			/* 62 : RC531錯誤，交易中止，請重新交易*/
			if ((memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-125", 4) == 0) ||
					(memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-119", 4) == 0 &&
					memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "62", 2) == 0 &&
					guszECCRetryBit == VS_TRUE)) {
				if (inRetry >= 4) {
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("請重新交易", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
					inDISP_BEEP(3, 1000);
					return (VS_ERROR);
				}

				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				//inDISP_ChineseFont("連線失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				sprintf(szAPI_RespCode, "%s", pobTran->srTRec.srECCRec.szAPI_RespCode);
				inDISP_ChineseFont(szAPI_RespCode, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont("請重新感應卡片", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_ChineseFont("並按確認鍵重試", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);

				szKey = 0x00;
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, 60);

				while (1) {
					szKey = uszKBD_Key();
					if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS) {
						szKey = _KEY_TIMEOUT_;
					}

					if (szKey == _KEY_ENTER_) {
						inRetry++;
						guszECCRetryBit = VS_TRUE;
						break;
					} else if (szKey == _KEY_TIMEOUT_) {
						return (VS_ERROR);
					}
				}

				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("交易處理中", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_ChineseFont("請重新感應卡片", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);

				continue;
			} else {
				/* 其他錯誤Display */
				inCMAS_ResponeCode(pobTran);

				return (VS_ERROR);
			}
		}

		/* 抓必要資料 */
		/* 2020.03.20 若AdditionalTcpipData = 0 ，則不需要unpack ICERAPI2 [Hachi] */
		if (szCMASAddTcpipMode[0] != '0') {
			/* [先移除功能要再加回] 2022/6/8 [SAM] */
			//			if (inECC_UNPACK_ICERAPI_NCCC(pobTran, _CMAS_API_RES_FILE2_, _CMAS_FOLDER_PATH_) != VS_SUCCESS)
			//				return (VS_ERROR);
			//			END
		}
		break;
	}
	/*
	 * T5501批次號碼之規則為yymmddss+流水號兩碼
	 * 以當批第一筆成功交易時間為準
	 * 如果NeedNewBatch是'Y'，代表是第一筆，更新狀態為不用更新批號
	 * 如果NeedNewBatch是'N'，代表有做過交易
	 */

	inCMAS_Batch_Update();

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "inCMAS_Void_TOP_UP_Flow() END !");
		inDISP_LogPrintfAt(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_Inquiry_Flow
Date&Time       :2020/2/11 下午 6:00
Describe        :餘額查詢流程
Author          :Hachi
 */
int inCMAS_Inquiry_Flow(TRANSACTION_OBJECT *pobTran) {
	int inRetVal = 0;
	char szDebugMsg[100 + 1] = {0};
	char szCMASAddTcpipMode[ 2 + 1 ] = {0};

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "----------------------------------------");
		inDISP_LogPrintfAt(AT, "inCMAS_Inquiry_Flow() START !");
	}

	memset(szCMASAddTcpipMode, 0x00, sizeof (szCMASAddTcpipMode));
	inGetCMASAddTcpipMode(szCMASAddTcpipMode);

	/* 組電文(生成ICERAPI.REQ) */
	inCMAS_PACK_Inquiry_ICERAPI(pobTran);

	/* AdditionalTcpipData */
	/*見規格書2.16 AddtionalTcpipData !=0 時才需要ICERAPI2.REQ、ICERAPI2.REQ*/
	if (szCMASAddTcpipMode[0] == '0') {
		/*直連不帶*/
	} else if (szCMASAddTcpipMode[0] == '1') {
		/*NCCC*/
	} else if (szCMASAddTcpipMode[0] == '2') {
		/*CTCB*/
	} else if (szCMASAddTcpipMode[0] == '3') {
		/*FISC*/
		inCMAS_PACK_Deduct_ICERAPI_FISC(pobTran);
	}

	if (ginDebug == VS_TRUE) {
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "ICER Start");
		inDISP_LogPrintfAt(AT, szDebugMsg);
	}

	inRetVal = ICERApi_exe();

	if (ginDebug == VS_TRUE) {
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "ICER End");
		inDISP_LogPrintfAt(AT, szDebugMsg);
	}

	/* 解電文(分析ICERAPI.RES) */
	inRetVal = inCMAS_UNPACK_ICERAPI(pobTran, _CMAS_API_RES_FILE_);

	if (inRetVal != VS_SUCCESS) {
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inCTLS_Set_LED(_CTLS_LIGHT_RED_);
		inDISP_ChineseFont("悠遊卡檔案異常", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
		inDISP_BEEP(3, 500);

		return (VS_ERROR);
	}

	/* 檢核回應碼 <T3901>開始 */
	if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "0", 1) != 0) {
		/* 其他錯誤Display */
		inCMAS_ResponeCode(pobTran);

		return (VS_ERROR);
	}

	/* 抓必要資料 */
	/* 2020.03.20 若AdditionalTcpipData = 0 ，則不需要unpack ICERAPI2 [Hachi] */
	if (szCMASAddTcpipMode[0] != '0') {
		/* [先移除功能要再加回] 2022/6/8 [SAM] */
		//		if (inECC_UNPACK_ICERAPI_NCCC(pobTran, _CMAS_API_RES_FILE2_, _CMAS_FOLDER_PATH_) != VS_SUCCESS)
		//			return (VS_ERROR);
		//		END
	}
	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "inCMAS_Inquiry_Flow() END !");
		inDISP_LogPrintfAt(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_Settle_Flow
Date&Time       :2020/2/27 下午 5:37
Describe        :結帳流程
Author          :Hachi
 */
int inCMAS_Settle_Flow(TRANSACTION_OBJECT *pobTran) {
	int inRetVal = VS_ERROR;
	char szDemoMode[2 + 1] = {0};
	char szCMASAddTcpipMode[2 + 1] = {0};

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "----------------------------------------");
		inDISP_LogPrintfAt(AT, "inCMAS_Settle_Flow() START !");
	}

	memset(szDemoMode, 0x00, sizeof (szDemoMode));
	inGetDemoMode(szDemoMode);

	memset(szCMASAddTcpipMode, 0x00, sizeof (szCMASAddTcpipMode));
	inGetCMASAddTcpipMode(szCMASAddTcpipMode);

	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0) {
	} else {
		/* 組電文(生成ICERAPI.REQ) */
		inCMAS_PACK_Settle_ICERAPI(pobTran);

		/* AdditionalTcpipData */
		if (szCMASAddTcpipMode[0] == '0') {
			/*直連不帶*/
		} else if (szCMASAddTcpipMode[0] == '1') {
			/*NCCC*/
		} else if (szCMASAddTcpipMode[0] == '2') {
			/*CTCB*/
		} else if (szCMASAddTcpipMode[0] == '3') {
			/*FISC*/
			inCMAS_PACK_Deduct_ICERAPI_FISC(pobTran);
		}
		if (ginDebug == VS_TRUE) {
			inDISP_LogPrintfAt(AT, "ICER Start");
		}

		inRetVal = ICERApi_exe();

		if (ginDebug == VS_TRUE) {
			inDISP_LogPrintfAt(AT, "ICER End");
		}

		/* 解電文(分析ICERAPI.RES) */
		inRetVal = inCMAS_UNPACK_ICERAPI(pobTran, _CMAS_API_RES_FILE_);

		if (inRetVal != VS_SUCCESS) {
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inCTLS_Set_LED(_CTLS_LIGHT_RED_);
			inDISP_ChineseFont("悠遊卡檔案異常", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
			inDISP_BEEP(3, 500);

			return (VS_ERROR);
		}

		/* 檢核回應碼 <T3901>開始 */
		if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "0", 1) != 0) {
			/* 其他錯誤Display */
			inCMAS_ResponeCode(pobTran);

			return (VS_ERROR);
		}

		/*20200507 Added start [Hachi]*/
		/*
		 * T5501批次號碼之規則為yymmddss+流水號兩碼
		 * 以當批第一筆成功交易時間為準
		 * 如果NeedNewBatch是'Y'，代表是第一筆，更新狀態為不用更新批號
		 * 如果NeedNewBatch是'N'，代表有做過交易
		 */
		/* 結帳，所以要更新批號 */
		// inSetTicket_NeedNewBatch("Y");
		// inSaveTDTRec(_TDT_INDEX_01_ECC_);
		/*20200507 Added end [Hachi]*/


		// else
		// {
		// 	if (inNCCC_TICKET_SetReversalCnt(pobTran, _RESET_) == VS_ERROR)
		// 	{
		// 		return (VS_ERROR);
		// 	}

		// 	/* 結帳成功 把請先結帳的bit關掉 */
		// 	inNCCC_TICKET_SetMustSettleBit(pobTran, "N");
		// }

		/* 抓必要資料 */
		/* 2020.03.20 若AdditionalTcpipData = 0 ，則不需要unpack ICERAPI2 [Hachi] */
		if (szCMASAddTcpipMode[0] != '0') {
			/* [先移除功能要再加回] 2022/6/8 [SAM] */
			//			if (inECC_UNPACK_ICERAPI_NCCC(pobTran, _CMAS_API_RES_FILE2_, _CMAS_FOLDER_PATH_) != VS_SUCCESS)
			//				return (VS_ERROR);
			//			END
		}
	}

	/*20200508 Added start Hachi */
	/*V3 NCCC版 無帳務下結帳批次號不累加，帳還是會送到NCCC，但NCCC主機不會轉送至悠遊卡主機*/
	/*目前採用除了版NCCC 一率累加批次*/
	if (szCMASAddTcpipMode[0] != '1')
		inCMAS_Batch_Update();
	/*20200508 Added start Hachi */

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "inCMAS_Settle_Flow() START !");
		inDISP_LogPrintfAt(AT, "----------------------------------------");
	}
	return (VS_SUCCESS);
}

/*
Function        : inCMAS_Batch_Check
Date&Time   : 2020/3/17 下午 6:03
Describe        : 確認是否進批，並回傳批號 參考令雄版的inECC_Batch_Check
Author          : Hachi
 * 批次號碼說明:
 * T5501(8 碼數字)：
 * YYMMDDXX， YY 為西元年末兩碼， XX 為 2 碼流水號， 01~99。
 * 所有交易皆需代入 T5501 欄位
 * 若結帳未成功，則批次號碼延用至結帳成功。需待結帳成功後，再進行跳號
 */
int inCMAS_Batch_Check(char *szECCBatchOut) {
	int inCount = 0;
	char szECCBatch[8 + 1] = {0};
	char szTermDateTime[8 + 1] = {0};
	char szTicket_NeedNewBatch[2 + 1] = {0};
	char szTemplate[2 + 1] = {0};
	RTC_NEXSYS srRTC = {0};

	memset(szTicket_NeedNewBatch, 0x00, sizeof (szTicket_NeedNewBatch));
	inGetTicket_NeedNewBatch(szTicket_NeedNewBatch);
	/* 先檢查是否是第一筆，不是就沿用 */
	if (memcmp(szTicket_NeedNewBatch, "Y", strlen("Y")) == 0) {
		if (ginDebug == VS_TRUE) {
			inDISP_LogPrintfAt(AT, "szTicket_NeedNewBatch is Y");
		}
		memset(&srRTC, 0x00, sizeof (srRTC));
		inFunc_GetSystemDateAndTime(&srRTC);
		sprintf(szTermDateTime, "%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		inLoadTDTRec(_TDT_INDEX_01_ECC_);
		memset(szECCBatch, 0x00, sizeof (szECCBatch));
		inGetTicket_Batch(szECCBatch);

		if (ginDebug == VS_TRUE) {
			inDISP_LogPrintfAt(AT, "szTermDateTime is %s(Y)", szTermDateTime);
			inDISP_LogPrintfAt(AT, "inGetTicket_Batch is %s(Y)", szECCBatch);
		}

		/* 是否是同一天 */
		if (memcmp(szTermDateTime, szECCBatch, 6) == 0) {
			/* 同一天流水號+1 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szECCBatch[6], 2);
			inCount = atoi(szTemplate) + 1;

			if (inCount >= 100) {
				inCount = 1;
			}

			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "%02d", inCount);

			if (ginDebug == VS_TRUE) {
				inDISP_LogPrintfAt(AT, "Same Day Num (%02d)", inCount);
			}
		} else {
			/* 不同天更新日期，並回到第一批 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "%02d", 1);
		}

		memcpy(&szTermDateTime[6], szTemplate, 2);
		strcpy(szECCBatchOut, szTermDateTime);
	} else {
		if (ginDebug == VS_TRUE) {
			inDISP_LogPrintfAt(AT, "szTicket_NeedNewBatch is N");
		}
		inLoadTDTRec(_TDT_INDEX_01_ECC_);
		memset(szECCBatch, 0x00, sizeof (szECCBatch));
		inGetTicket_Batch(szECCBatch);

		strcpy(szECCBatchOut, szECCBatch);
	}

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "inCMAS_Batch_Check the szECCBatchOut is %s", szECCBatchOut);
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_Batch_Update
Date&Time       :2020/3/19 下午 04:50
Describe        :更新Batch Number至TDT
Author          :Hachi
 */
int inCMAS_Batch_Update(void) {
	char szECCBatch[8 + 1] = {0};
	char szTicket_NeedNewBatch[2 + 1] = {0};

	memset(szTicket_NeedNewBatch, 0x00, sizeof (szTicket_NeedNewBatch));
	inGetTicket_NeedNewBatch(szTicket_NeedNewBatch);
	/* 檢查是否要更新批次 */
	if (memcmp(szTicket_NeedNewBatch, "Y", strlen("Y")) == 0) {
		memset(szECCBatch, 0x00, sizeof (szECCBatch));
		inCMAS_Batch_Check(szECCBatch);
		inSetTicket_Batch(szECCBatch);
		inSetTicket_NeedNewBatch("N");
		inSaveTDTRec(_TDT_INDEX_01_ECC_);
	} else {
		/* 不更新 */
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_Set_Update_Batch_Flag
Date&Time       :2020/5/6 下午 6:24
Describe        :將悠遊卡更新批次的Flag放到和NCCC批次同樣的地方，以求印單時批號邏輯一致，參考inECC_Set_Update_Batch_Flag
Author          :Hachi
 */
int inCMAS_Set_Update_Batch_Flag(TRANSACTION_OBJECT *pobTran) {
	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "----------------------------------------");
		inDISP_LogPrintfAt(AT, "inCMAS_Set_Update_Batch_Flag() START !");
	}

	/*
	 * T5501批次號碼之規則為yymmddss+流水號兩碼
	 * 以當批第一筆成功交易時間為準
	 * 如果NeedNewBatch是'Y'，代表是第一筆，更新狀態為不用更新批號
	 * 如果NeedNewBatch是'N'，代表有做過交易
	 */
	inLoadTDTRec(_TDT_INDEX_01_ECC_);
	/* 結帳，所以要更新批號 */
	inSetTicket_NeedNewBatch("Y");
	inSaveTDTRec(_TDT_INDEX_01_ECC_);

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "inCMAS_Set_Update_Batch_Flag() END !");
		inDISP_LogPrintfAt(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_ResponeCode
Date&Time       :2020/5/14 下午 4:19
Describe        :CAMS主機回應碼
Author          :Hachi
 */
int inCMAS_ResponeCode(TRANSACTION_OBJECT *pobTran) {
	int inRespCodeLen = 0;
	char szLine1[40] = {0}, szLine2[40] = {0}, szLine3[40] = {0}; /*20220119,浩瑋標記fix*/
	char szOther[10 + 1] = {0};

	inDISP_LogPrintfAt(AT, "inCMAS_ResponeCode start");

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	memset(szLine1, 0x00, sizeof (szLine1));
	memset(szLine2, 0x00, sizeof (szLine2));
	memset(szLine3, 0x00, sizeof (szLine3));

	inRespCodeLen = atoi(pobTran->srTRec.srECCRec.szAPI_RespCode);

	if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-101", 4)) {
		/* ICER RespCode */
		if (!memcmp(&pobTran->srTRec.srECCRec.szICER_RespCode[0], "51", 2)) {
			sprintf(szLine3, "%s", "票卡餘額不足");
			sprintf(pobTran->szTicket_ErrorCode, "%s", "E0016");
		} else if (!memcmp(&pobTran->srTRec.srECCRec.szICER_RespCode[0], "61", 2)) {
			sprintf(szLine3, "%s", "電票金額超過上限");
			sprintf(pobTran->szTicket_ErrorCode, "%s", "E0012");
		} else if (!memcmp(&pobTran->srTRec.srECCRec.szICER_RespCode[0], "76", 2) ||
				!memcmp(&pobTran->srTRec.srECCRec.szICER_RespCode[0], "77", 2)) {
			sprintf(szLine3, "%s", "無法比對原交易");
			sprintf(pobTran->szTicket_ErrorCode, "%s", "E0014");
		} else if (!memcmp(&pobTran->srTRec.srECCRec.szICER_RespCode[0], "17", 2))/*20211130,悠遊卡要求新增*/ {
			sprintf(szLine3, "%s", "超出次限額");
			sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
		else {
			sprintf(szLine3, "%s", "交易失敗");
			sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}

		sprintf(szOther, "%s", pobTran->srTRec.srECCRec.szICER_RespCode);
	} else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-102", 4)) {
		sprintf(szLine3, "%s", "交易失敗");
		sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
	} else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-123", 4)) {
		sprintf(szLine3, "%s", "票卡餘額不足");
		sprintf(pobTran->szTicket_ErrorCode, "%s", "E0016");
	} else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-130", 4)) {
		sprintf(szLine3, "%s", "票卡已退");
		sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
	} else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-134", 4)) {
		sprintf(szLine3, "%s", "票卡未開卡");
		sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
	} else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-132", 4) ||
			!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-133", 4)) {
		inDISP_LogPrintfAt(AT, "log -132 || -133");
		//inDISP_LogPrintfAt(AT,"pobTran->srTRec.srECCRec.szReader_RespCode == %s",pobTran->srTRec.srECCRec.szReader_RespCode );
		//inDISP_LogPrintfAt(AT,"pobTran->srTRec.srECCRec.szAPI_RespCode == %s",pobTran->srTRec.srECCRec.szAPI_RespCode );
		//sprintf(szLine2, "%s", pobTran->srTRec.srECCRec.szReader_RespCode); 
		//sprintf(szOther, "%s", pobTran->srTRec.srECCRec.szReader_RespCode);
		sprintf(szLine3, "%s", "票卡已鎖洽悠遊卡");
		sprintf(pobTran->szTicket_ErrorCode, "%s", "E0013");
	} else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-110", 4) ||
			!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-111", 4) ||
			!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-120", 4) ||
			!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-121", 4)) {
		sprintf(szLine3, "%s", "交易失敗，請重試");
		sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
	} else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-103", 4) ||
			!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-104", 4) ||
			!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-105", 4) ||
			!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-106", 4) ||
			!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-107", 4) ||
			!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-108", 4) ||
			!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-109", 4) ||
			!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-122", 4) ||
			!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-124", 4) ||
			!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-125", 4) ||
			!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-126", 4) ||
			!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-128", 4)) {
		sprintf(szLine3, "%s", "交易異常，請報修");
		sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
	} else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-119", 4)) {
		/* Reader RespCode */
		if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "9000", 4) ||
				!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6001", 4) ||
				!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6002", 4) ||
				!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6003", 4) ||
				!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6004", 4) ||
				!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6005", 4) ||
				!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6088", 4)) {
			sprintf(szLine3, "%s", "交易失敗，請重試");
			sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		} else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "60A1", 4) ||
				!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "60A2", 4)) {
			sprintf(szLine3, "%s", "交易異常，請報修");
			sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		} else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "61", 2)) {
			if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6108", 4)) {
				sprintf(szLine3, "%s", "票卡過期");
				sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
			} else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6109", 4)) {
				inDISP_LogPrintfAt(AT, "log 6109");
				//inDISP_LogPrintfAt(AT,"pobTran->srTRec.srECCRec.szReader_RespCode == %s",pobTran->srTRec.srECCRec.szReader_RespCode );
				//inDISP_LogPrintfAt(AT,"pobTran->srTRec.srECCRec.szAPI_RespCode == %s",pobTran->srTRec.srECCRec.szAPI_RespCode );                            
				sprintf(szLine3, "%s", "票卡已鎖洽悠遊卡");
				sprintf(pobTran->szTicket_ErrorCode, "%s", "E0013");
			} else {
				sprintf(szLine3, "%s", "票卡異常"); /*20211130,依照悠遊卡修改*/
				sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
			}
		} else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "64", 2)) {
			if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6401", 4)) {
				sprintf(szLine3, "%s", "取消與上一筆不符");
				sprintf(pobTran->szTicket_ErrorCode, "%s", "E0014");
			} else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6402", 4)) {
				sprintf(szLine3, "%s", "交易金額超過額度");
				sprintf(pobTran->szTicket_ErrorCode, "%s", "E0012");
			} else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6403", 4)) {
				sprintf(szLine3, "%s", "票卡餘額不足");
				sprintf(pobTran->szTicket_ErrorCode, "%s", "E0016");
			}
			else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6418", 4)) {
				sprintf(szLine3, "%s", "限制購貨"); /*20211130,依照悠遊卡修改*/
				sprintf(pobTran->szTicket_ErrorCode, "%s", "E0016");
			}
			else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "640C", 4)) {
				sprintf(szLine3, "%s", "超出日限額"); /*20211130,依照悠遊卡修改*/
				sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
			}
			else {
				sprintf(szLine3, "%s", "票卡異常"); /*20211130,依照悠遊卡修改*/
				sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
			}
		} else {
			sprintf(szLine3, "%s", "交易異常，請報修");
			sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
		sprintf(szOther, "%s", pobTran->srTRec.srECCRec.szReader_RespCode);
	} else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-138", 4)) {
		if (!memcmp(&pobTran->srTRec.szRespCode[0], "XT", 2)) {
			sprintf(szLine3, "%s", " 無法比對原交易");
			sprintf(pobTran->szTicket_ErrorCode, "%s", "E0014");
		} else {
			sprintf(szLine3, "%s", "交易失敗");
			sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
	} else if (inRespCodeLen == 3) {
		if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-11", 2) ||
				!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-18", 2) ||
				!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-19", 2) ||
				!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-21", 2) ||
				!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-22", 2) ||
				!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-23", 2)) {
			sprintf(szLine3, "%s", "交易失敗，請重試");
			sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		} else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-32", 2) ||
				!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-34", 2) ||
				!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-36", 2)) {
			sprintf(szLine3, "%s", "交易異常，請報修");
			sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		} else {
			sprintf(szLine3, "%s", "    交易失敗");
			sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
	} else if (inRespCodeLen == 2) {
		if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-1", 2) ||
				!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-2", 2) ||
				!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-3", 2) ||
				!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-4", 2) ||
				!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-5", 2) ||
				!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-6", 2) ||
				!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-7", 2) ||
				!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-8", 2) ||
				!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-9", 2)) {
			sprintf(szLine3, "%s", "交易失敗，請重試");
			sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		} else {
			sprintf(szLine3, "%s", "    交易失敗");
			sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
	}
	else {
		sprintf(szLine3, "%s", "    交易失敗");
		sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
	}

	if (strlen(szOther) > 0) {
		if (!memcmp(&pobTran->srTRec.szRespCode[0], "XT", 2)) {
			sprintf(szLine1, "[%s][%s]", pobTran->srTRec.szRespCode, szOther);
		} else {
			sprintf(szLine1, "[%s][%s]", pobTran->srTRec.srECCRec.szAPI_RespCode, szOther);
		}
	}
	else {
		if (!memcmp(&pobTran->srTRec.szRespCode[0], "XT", 2)) {
			sprintf(szLine1, "[%s]", pobTran->srTRec.szRespCode);
		} else {
			sprintf(szLine1, "[%s]", pobTran->srTRec.srECCRec.szAPI_RespCode);
		}
	}

	inDISP_ChineseFont(szLine1, _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
	inDISP_ChineseFont(szLine2, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
	inDISP_ChineseFont(szLine3, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
	// inDISP_ChineseFont("請按任意鍵", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);

	/*20200514[Hachi]*/
	inCTLS_Set_LED(_CTLS_LIGHT_RED_);
	inDISP_BEEP(3, 500);
	inDISP_Wait(2000);
	// uszKBD_GetKey(60);

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_Func_Check_Transaction_Function_Flow
Date&Time       :2020/5/19 下午 5:45
Describe        :悠遊卡檢查個別交易功能FLOW
Author          :Hachi
 */
int inCMAS_Func_Check_Transaction_Function_Flow(TRANSACTION_OBJECT *pobTran) {
	int inRetVal = VS_SUCCESS;
	unsigned char uszTxnEnable = VS_TRUE;

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "----------------------------------------");
		inDISP_LogPrintfAt(AT, "inCMAS_Func_Check_Transaction_Function_Flow() START !");
	}

	/* 20200617 [Hachi]*/
	if (inLoadHDTRec(_HDT_INDEX_13_CMAS_) < 0) {
		return (VS_SUCCESS);
	}

	if (ginFindRunTime == VS_TRUE) {
		/* [先移除功能要再加回] 2022/6/8 [SAM] */
		//		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
		//		END
	}

	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_) {
		inRetVal = inCMAS_Func_Check_Transaction_Function(pobTran->srTRec.inCode);
		if (inRetVal != VS_SUCCESS) {
			uszTxnEnable = VS_FALSE;
		}
	} else {
		uszTxnEnable = VS_FALSE;
	}

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "inCMAS_Func_Check_Transaction_Function_Flow() END !");
		inDISP_LogPrintfAt(AT, "----------------------------------------");
	}

	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE) {
		pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;
		return (VS_ERROR);
	} else {
		return (VS_SUCCESS);
	}
}

/*
Function        :inCMAS_Delete_Log
Date&Time       :2020/6/8 上午 9:37
Describe        :參考inECC_Delete_Log
Author          :Hachi
 */
int inCMAS_Delete_Log(void) {
	char szFileName[63 + 1] = "EcrLog.txt";
	char szTimeTemp[18 + 1]; /* 取得目前日期時間 (YYYYMMDDhhmmssw):*/
	char szUNIXDATE[30 + 1];

	RTC_NEXSYS srRTC = {0};

	inDISP_LogPrintfAt(AT, "inCMAS_Delete_Log start");

	/*檢核並清除7天前的log*/
	memset(&srRTC, 0x00, sizeof (CTOS_RTC));
	/* 取得EDC時間日期 */
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS) {
		if (ginDebug == VS_TRUE) {
			memset(szFileName, 0x00, sizeof (szFileName));
			sprintf(szFileName, "inFunc_GetDateAndTime ERROR");
			inDISP_LogPrintfAt(AT, szFileName);
		}

		/* 感應燈號及聲響 */
		return (VS_ERROR);
	}

	memset(szTimeTemp, 0x00, sizeof (szTimeTemp));
	memset(szUNIXDATE, 0x00, sizeof (szUNIXDATE));

	sprintf(szTimeTemp, "20%02d%02d%02d%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond); /*yyyymmddhhmmss*/
	inDISP_LogPrintfAt(AT, "NOW TIME : %s", szTimeTemp);
	/* 先換成unix time */
	ulgetUnixTimeCnt((unsigned char *) szUNIXDATE, (unsigned char *) szTimeTemp);
	inDISP_LogPrintfAt(AT, "UNIX TIME :");
	PrintHexToStringFix((unsigned char *) &szTimeTemp[0], 4); /*debug用*/

	/*減去七天,且轉回來*/
	memset(szTimeTemp, 0x00, sizeof (szTimeTemp));
	ECC_UnixToDateTime2((unsigned char *) szUNIXDATE, (unsigned char *) szTimeTemp, 14, 7);
	inDISP_LogPrintfAt(AT, "NOW TIME 2: %s", szTimeTemp);

	memset(szFileName, 0x00, sizeof (szFileName));
	strcpy(szFileName, "ICER");
	memcpy(&szFileName[4], &szTimeTemp[0], 8);
	strcat(szFileName, ".log");

	inDISP_LogPrintfAt(AT, "del file name: %s", szFileName);

	/* 刪ICERLog ,格式如下 ICER20211130.log*/
	inFunc_Delete_Data("", szFileName, _ECC_ICER_LOG_FOLDER_PATH_);
	//inFunc_Data_Delete("", "*", _ECC_ICER_LOG_FOLDER_PATH_);        


	memset(szFileName, 0x00, sizeof (szFileName));
	strcpy(szFileName, "RDebug");
	memcpy(&szFileName[6], &szTimeTemp[2], 6);
	strcat(szFileName, ".log");

	inDISP_LogPrintfAt(AT, "del file name: %s", szFileName);

	/* 刪Rdebug ,格式如下 RDebug211130.log*/
	inFunc_Delete_Data("", szFileName, _ECC_ICER_LOG_FOLDER_PATH_);
	//        /* 這是原本的寫法,我不知道是否會產生在這..但有確認肯定會存在.\ICERData\ICERLog */
	//	inFunc_GetSystemDateAndTime(&srRTC);
	//	memset(szFileName, 0x00, sizeof(szFileName));
	//	sprintf(szFileName, "RDebug%02d%02d%02d.log", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
	//	inFunc_Data_Delete("", szFileName, _FS_DATA_PATH_);

	inDISP_LogPrintfAt(AT, "inCMAS_Delete_Log end");

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_Func_PrintReceipt_ByBuffer
Date&Time       :2020/6/11 下午 5:45
Describe        :悠遊卡簽單
Author          :Hachi 
 */
int inCMAS_Func_PrintReceipt_ByBuffer(TRANSACTION_OBJECT *pobTran) {
	int inRetVal = VS_ERROR;
	int inChoice = 0;
	int inTouchSensorFunc = _Touch_CUST_RECEIPT_;
	char szPrtMode[2 + 1];
	unsigned char uszKey = 0x00;

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "inCMAS_Func_PrintReceipt_ByBuffer() START");
	}

	/* 為了恢復被signpad清掉的title */
	inFunc_ResetTitle(pobTran);

	memset(szPrtMode, 0x00, sizeof (szPrtMode));
	inGetPrtMode(szPrtMode);

	/* 列印商店聯 */
	/* 簽單模式不是0就印商店聯 */
	if (memcmp(szPrtMode, "0", strlen("0")) != 0) {
		/* [新增電票悠遊卡功能] 因列印格式關係，修改顯示圖片及顯示欄位，取代 _TOUCH_TCBUI_PRT_RECEIPT_ 圖片 [SAM] 2022/6/23 上午 10:52 */
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);/* 列印帳單中 */

		while (1) {
			pobTran->srTRec.inPrintOption = _PRT_MERCH_;
			
			inRetVal = inCMAS_PRINT_Receipt_ByBuffer(pobTran); 
			
			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
				inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else {
				/* 成功就砍簽名圖檔 */
				inFunc_Delete_Signature(pobTran);

				break;
			}
		}
	}

	/* 三聯式簽單，中間要多印一聯客戶聯 */
	if (memcmp(szPrtMode, "3", strlen("3")) == 0) {
		/* [新增電票悠遊卡功能] 因列印格式關係，修改顯示圖片及顯示欄位，取代 _TOUCH_TCBUI_PRT_RECEIPT_ 圖片 [SAM] 2022/6/23 上午 10:52 */
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);/* 列印帳單中 */

		/* 列印商店存根聯 */
		while (1) {
			pobTran->srTRec.inPrintOption = _PRT_MERCH_DUPLICATE_;
			
			inRetVal = inCMAS_PRINT_Receipt_ByBuffer(pobTran); 
			
			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
					inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
				break;
		}
	}

	/* [新增電票悠遊卡功能] 重印簽單應該不能顯示金額，因為會不準確 [SAM] 2022/6/24 上午 10:28 */
	if (pobTran->inRunOperationID != _OPERATION_REPRINT_)	
	{
		inDISP_Clear_Line(_LINE_8_1_, _LINE_8_8_);
		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_8_1_); /* 第一層顯示 LOGO */

		/* 提示訊息 */
		inNCCC_Ticket_Display_Transaction_Result(pobTran);


		/* 第一聯印完後15秒未按確認或清除鍵則端末機嗶嗶聲提示，響15秒後自動出第二聯 */
		inDISP_Timer_Start(_TIMER_NEXSYS_2_, 30);
		inDISP_Timer_Start(_TIMER_NEXSYS_3_, 15);
		while (1) {
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			uszKey = uszKBD_Key();

			if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS) {
				uszKey = _KEY_TIMEOUT_;
			}

			if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS) {
				inDISP_BEEP(1, 250);
			}

			if (uszKey == _KEY_ENTER_ ||
				uszKey == _KEY_TIMEOUT_ ||
				inChoice == _CUSTReceipt_Touch_ENTER_) {
				inRetVal = VS_SUCCESS;
				break;
			} else if (uszKey == _KEY_CANCEL_ ||
					inChoice == _CUSTReceipt_Touch_CANCEL_) {
				inRetVal = VS_USER_CANCEL;
				break;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();

		/* 因為這裡交易已完成，所以一定回傳成功*/
		if (inRetVal == VS_USER_CANCEL) {
			return (VS_SUCCESS);
		}
	}

	/* 列印顧客聯 */
	/* 三聯式簽單，兩聯以上就要印顧客聯 */
	if (memcmp(szPrtMode, "2", strlen("2")) == 0 ||
		memcmp(szPrtMode, "3", strlen("3")) == 0) {
		while (1) {
			inDISP_Clear_Line(_LINE_8_1_, _LINE_8_8_);
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_8_1_); /* 第一層顯示 LOGO */
			
			/* [新增電票悠遊卡功能] 重印簽單應該不能顯示金額，因為會不準確 [SAM] 2022/6/24 上午 10:28 */
			if (pobTran->inRunOperationID != _OPERATION_REPRINT_)	
			{
				/* 提示訊息 */
				inNCCC_Ticket_Display_Transaction_Result(pobTran);
			}
			
			pobTran->srTRec.inPrintOption = _PRT_CUST_;
			inRetVal = inCMAS_PRINT_Receipt_ByBuffer(pobTran); 
			
			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
					inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
				break;
		}
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_Fast_Tap_Wait
Date&Time       :2020/6/20 上午 10:36
Describe        :用來判斷是否有卡片放置在刷卡機上
Author          :Hachi
 */
int inCMAS_Fast_Tap_Wait(TRANSACTION_OBJECT *pobTran, char *szUID) {
	int inRetVal = VS_SUCCESS;
	char szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
	long lnTimeout = 0;
	unsigned char uszKey = 0x00;

	inDISP_ClearAll();
	/* 顯示對應交易別的感應畫面 */
	inCTLS_Decide_Display_Image(pobTran);

	/* 顯示虛擬LED(不閃爍)，但inCTLS_ReceiveReadyForSales(V3機型)也會顯示LED（閃爍），所以沒有實質作用 */
	inCTLS_LED_Wait_Start();

	/* 進入畫面時先顯示金額 */
	if (pobTran->srBRec.lnTxnAmount > 0) {
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%ld", pobTran->srBRec.lnTxnAmount);
		inFunc_Amount_Comma(szTemplate, "NT$ ", ' ', _SIGNED_NONE_, 16, VS_TRUE);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_LEFT_);
	}

	if (pobTran->uszECRBit == VS_TRUE) {
		lnTimeout = 30;
	} else {
		lnTimeout = 30;
	}

	/* inSecond剩餘倒數時間 */
	inDISP_TimeoutStart(lnTimeout);

	while (1) {
		uszKey = uszKBD_Key();

		/* 感應倒數時間 && Display Countdown */
		if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT) {
			/* 感應時間到Timeout */
			uszKey = _KEY_TIMEOUT_;
		}

		if (uszKey == _KEY_TIMEOUT_) {
			inRetVal = VS_TIMEOUT;
			break;
		} else if (uszKey == _KEY_CANCEL_) {
			inRetVal = VS_USER_CANCEL;
			break;
		} else if (inCTLS_Check_TypeACard() == VS_SUCCESS) {
			inCTLS_Get_TypeACardSN(szUID);
			inRetVal = VS_SUCCESS;
			inCTLS_Set_LED(_CTLS_LIGHT_YELLOW_);
			inDISP_BEEP(1, 0);

			break;
		}
	}

	return (inRetVal);
}

/*
Function        :inCMAS_Get_ParamValue
Date&Time       :2020/6/20 上午 10:36
Describe        :確認悠遊卡交易參數
Author          :Hachi
 */
int inCMAS_Get_ParamValue(TRANSACTION_OBJECT *pobTran) {
	char szInvNum[6 + 1] = {0};
	char szTicketInvNum[6 + 1] = {0};
	//char szOutput[10 + 1] = {0};
	char szFuncEnable[2 + 1] = {0};
	RTC_NEXSYS srRTC;

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "----------------------------------------");
		inDISP_LogPrintfAt(AT, "inCMAS_Get_ParamValue() START !");
	}

	memset(szInvNum, 0x00, sizeof (szInvNum));
	inGetInvoiceNum(szInvNum);

	memset(szTicketInvNum, 0x00, sizeof (szTicketInvNum));
	inGetTicket_InvNum(szTicketInvNum);

	pobTran->srTRec.lnInvNum = atol(szInvNum);
	pobTran->srTRec.lnMainInvNum = atol(szTicketInvNum);
	pobTran->srTRec.lnCountInvNum = pobTran->srTRec.lnMainInvNum;

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "--------------------------------------------");
		inDISP_LogPrintfAt(AT, "pobTran->srTRec.lnInvNum is %ld", pobTran->srTRec.lnInvNum);
		inDISP_LogPrintfAt(AT, "pobTran->srTRec.lnMainInvNum is %ld", pobTran->srTRec.lnMainInvNum);
		inDISP_LogPrintfAt(AT, "pobTran->srTRec.lnCountInvNum is %ld", pobTran->srTRec.lnCountInvNum);
		inDISP_LogPrintfAt(AT, "--------------------------------------------");
	}


	 inNCCC_Ticket_Func_MakeRefNo(pobTran);

	/* Time */
	memset(&srRTC, 0x00, sizeof (srRTC));
	inFunc_GetSystemDateAndTime(&srRTC);

	// if (inFunc_Sync_TRec_Date_Time_IPASS(pobTran, &srRTC) != VS_SUCCESS)
	if (inFunc_Sync_TRec_Date_Time_CMAS(pobTran, &srRTC) != VS_SUCCESS) //20200620[Hachi]
	{
		return (VS_ERROR);
	}


	/* 櫃號 */
	memset(pobTran->srTRec.szStoreID, 0x00, sizeof (pobTran->srTRec.szStoreID));
	memset(szFuncEnable, 0x00, sizeof (szFuncEnable));
	inGetStoreIDEnable(szFuncEnable);

	if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0) {
		memcpy(pobTran->srTRec.szStoreID, pobTran->srBRec.szStoreID, 50);
	}

	/* 產品代碼 */
	memset(pobTran->srTRec.szProductCode, 0x00, sizeof (pobTran->srTRec.szProductCode));
	memset(szFuncEnable, 0x00, sizeof (szFuncEnable));
	inGetProductCodeEnable(szFuncEnable);

	if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0) {
		memcpy(pobTran->srTRec.szProductCode, pobTran->srBRec.szProductCode, 42);
	}

	/* 退貨碼預設為000000 */
	if (pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_) {
		if (strlen(pobTran->srTRec.szTicketRefundCode) == 0) {
			memset(pobTran->srTRec.szTicketRefundCode, 0x00, sizeof (pobTran->srTRec.szTicketRefundCode));
			memcpy(pobTran->srTRec.szTicketRefundCode, "000000", 6);
		}
	} else {
		memset(pobTran->srTRec.szTicketRefundCode, 0x00, sizeof (pobTran->srTRec.szTicketRefundCode));
		memcpy(pobTran->srTRec.szTicketRefundCode, "000000", 6);
	}

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "inCMAS_Get_ParamValue() END !");
		inDISP_LogPrintfAt(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_Func_Check_Transaction_Function
Date&Time       :2020/6/20 上午 10:36
Describe        :確認悠遊卡個別交易功能是否打開
Author          :Hachi
 */
int inCMAS_Func_Check_Transaction_Function(int inCode) {
	char szTransFunc[20 + 1];
	unsigned char uszTxnEnable = VS_TRUE;

	memset(szTransFunc, 0x00, sizeof (szTransFunc));
	if (inGetTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);

	/* 1. 加值功能檢查 */
	if (inCode == _TICKET_EASYCARD_TOP_UP_) {
		if (memcmp(&szTransFunc[0], "1", 1) != 0) {
			uszTxnEnable = VS_FALSE;
		}
	}		/* 2. 購貨功能檢查 */
	else if (inCode == _TICKET_EASYCARD_DEDUCT_) {
		if (memcmp(&szTransFunc[1], "1", 1) != 0) {
			uszTxnEnable = VS_FALSE;
		}
	}		/* 3. 退貨功能檢查 */
	else if (inCode == _TICKET_EASYCARD_REFUND_) {
		if (memcmp(&szTransFunc[2], "1", 1) != 0) {
			uszTxnEnable = VS_FALSE;
		}
	}		/* 4. 取消購貨功能檢查 */
	else if (inCode == _TICKET_EASYCARD_VOID_DEDUCT_) {
		if (memcmp(&szTransFunc[3], "1", 1) != 0) {
			uszTxnEnable = VS_FALSE;
		}
	}		/* 5. 讀取餘額功能檢查 */
	else if (inCode == _TICKET_EASYCARD_INQUIRY_) {
		if (memcmp(&szTransFunc[4], "1", 1) != 0) {
			uszTxnEnable = VS_FALSE;
		}
	}		/* 6. 結帳功能檢查 */
	else if (inCode == _SETTLE_) {
		if (memcmp(&szTransFunc[5], "1", 1) != 0) {
			uszTxnEnable = VS_FALSE;
		}
	}
		/* 7. 取消加值功能檢查 */
	else if (inCode == _TICKET_EASYCARD_VOID_TOP_UP_) {
		if (memcmp(&szTransFunc[6], "1", 1) != 0) {
			uszTxnEnable = VS_FALSE;
		}
	}

	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE) {
		return (VS_ERROR);
	} else {
		return (VS_SUCCESS);
	}
}

/*
Function        :inCMAS_AutoAdd_Flow
Date&Time       :2021/4/7 上午 9:22
Describe        :
 */
int inCMAS_AutoAdd_Flow(TRANSACTION_OBJECT *pobTran) {
	int inType = 0;
	long lnAmt = 0l;
	char szTemplate[7] = {0};
	char szInvNum[6 + 1] = {0}; /*2021/4/27 下午 5:13*/

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "----------------------------------------");
		inDISP_LogPrintfAt(AT, "inCMAS_AutoAdd_Flow() START !");
	}

	inType = pobTran->srTRec.inCode;
	pobTran->srTRec.inCode = _TICKET_EASYCARD_AUTO_TOP_UP_;

	/* 金額要調整成自動加值的 */
	lnAmt = pobTran->srTRec.lnTxnAmount;
	pobTran->srTRec.lnTxnAmount = pobTran->srTRec.lnTotalTopUpAmount;

	/* AuthCode要調整成自動加值的 */
	memset(szTemplate, 0x00, sizeof (szTemplate));
	memcpy(&szTemplate[0], &pobTran->srTRec.szAuthCode[0], 6);
	memset(pobTran->srTRec.szAuthCode, 0x00, sizeof (pobTran->srTRec.szAuthCode));
	memcpy(&pobTran->srTRec.szAuthCode[0], &pobTran->srTRec.szECCAuthCode[0], 6);

	if (inFLOW_RunFunction(pobTran, _TICKET_UPDATE_ACCUM_) != VS_SUCCESS)
		return (VS_ERROR);

	/*20210429[Hachi] mark start*/
	//	if (inFLOW_RunFunction(pobTran, _TICKET_UPDATE_BATCH_) != VS_SUCCESS) 
	//		return (VS_ERROR);
	//
	//	if (inFLOW_RunFunction(pobTran, _CMAS_UPDATE_INV_) != VS_SUCCESS)
	//		return (VS_ERROR);
	/*20210429[Hachi] mark end*/

	/* 還原成購貨 */
	pobTran->srTRec.inCode = inType;
	pobTran->srTRec.lnTxnAmount = lnAmt;

	memset(pobTran->srTRec.szAuthCode, 0x00, sizeof (pobTran->srTRec.szAuthCode));
	memcpy(&pobTran->srTRec.szAuthCode[0], &szTemplate[0], 6);

	/*20210427 [Hachi ]mark start*/
	//	/* 購貨帳務問題，組的時候是n，ECC API吐n+2，所以購貨要修成n+1 */
	//	pobTran->srTRec.lnInvNum = pobTran->srTRec.lnECCInvNum - 1;
	/*20210427 [Hachi ]mark End*/

	/*20210427 [Hachi] start*/
	/* 購貨的簽單序號要在自動加值後加一號，上面已累加簽單調閱編號，所以直接再取出 */
	memset(szInvNum, 0x00, sizeof (szInvNum));
	inGetInvoiceNum(szInvNum);
	//pobTran->srTRec.lnInvNum = atol(szInvNum);
	pobTran->srTRec.lnInvNum = atol(szInvNum);
	/*20210427 [Hachi] End*/

	/* RRN - 購貨會帶成n，要重組成n+1 */
	inNCCC_Ticket_Func_MakeRefNo(pobTran);

	//        inCMAS_Batch_Update(); /*20210426 補[Hachi]*/

	if (ginDebug == VS_TRUE) {
		inDISP_LogPrintfAt(AT, "inCMAS_AutoAdd_Flow() END !");
		inDISP_LogPrintfAt(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}