#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>

#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"
#include "../FUNCTION/Accum.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/Sqlite.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/HDPT.h"
#include "../FUNCTION/Function.h"
#include "../FUNCTION/FuncTable.h"
#include "../FUNCTION/ECR_FUNC/ECR.h"
#include "../FUNCTION/ECR_FUNC/RS232.h"
#include "../FUNCTION/MULTI_FUNC/MultiFunc.h"
#include "../FUNCTION/PowerManagement.h"
#include "../FUNCTION/AccountFunction/PrintBillInfo.h"
#include "../FUNCTION/UNIT_FUNC/TimeUint.h"
#include "../FUNCTION/FILE_FUNC/File.h"
#include "../FUNCTION/FILE_FUNC/FIleLogFunc.h"

#include "../../EMVSRC/EMVsrc.h"
#include "../../CTLS/CTLS.h"
#include "../COMM/Comm.h"
#include "../COMM/WiFi.h"
#include "Menu.h"
#include "MenuMsg.h"
#include "Event.h"
#include "Flow.h"


#include "../../FUBON/FUBONfunc.h"


/* [新增電票悠遊卡功能] [SAM] 2022/6/8 下午 3:46 */
#include "../../CMAS/CMASsrc.h"

#include "../../SVCSRC/SvcSrc.h"

int	ginEventCode;	/* 用於儲存IdleMenuKeyIn第一次按下的按鍵 */
extern  int		ginIdleDispFlag;
extern	int		ginFindRunTime;
extern	int		ginTouch_Handle;
extern	int		ginDebug;
extern	int		ginMachineType;
extern	int		ginAPVersionType;
extern	ECR_TABLE	gsrECROb;

static char st_szEventDebugBuf[64];

static TRANSACTION_OBJECT* Temp_pobTran = NULL;

int inEVENT_Test(void)
{
        inDISP_LogPrintf("inEVENT_Test()");
        return (VS_SUCCESS);
}

/* [外接設備設定] 新增  tbGetPobTranPoint() inSetPobTranPoint() inSetPobTranPointNull()
 * 2022/11/17 [SAM]
 */
extern MULTI_TABLE	gstMultiOb;

TRANSACTION_OBJECT* tbGetPobTranPoint()
{
	return Temp_pobTran;
}

int  inSetPobTranPoint(TRANSACTION_OBJECT *TempPobtran)
{
	if(Temp_pobTran != NULL)
		Temp_pobTran = TempPobtran;
	return 0;
}

int  inSetPobTranPointNull()
{
	Temp_pobTran = NULL;
	return 0;
}


/*
Function		: chTransferEventKeyToChar
Date&Time	: 2022/7/21 下午 7:22 [SAM]
Describe		: 
 * DEBUG 輸出用 
*/
char* chTransferEventKeyToChar(int inKey)
{
	char szTemp[52];
	EVENT emTempEvent;
	
	emTempEvent = inKey;
	memset(st_szEventDebugBuf, 0x00, sizeof(st_szEventDebugBuf));
	memset(szTemp, 0x00, sizeof(szTemp));
	
	switch(emTempEvent)
	{
		case _NONE_EVENT_:
			strcat(st_szEventDebugBuf, " 無事件");
			break;
		case _SWIPE_EVENT_:
			strcat(st_szEventDebugBuf, " 刷卡事件");
			break;	
		case _EMV_DO_EVENT_:
			strcat(st_szEventDebugBuf, " 晶片插卡事件");
			break;
		case _MENUKEYIN_EVENT_:
			strcat(st_szEventDebugBuf, " 人工輸入事件");
			break;
		case _SENSOR_EVENT_:
			strcat(st_szEventDebugBuf, " 感應觸發事件");
			break;
		case _TICKET_EVENT_:
			strcat(st_szEventDebugBuf, " 票證事件");
			break;
		case _ECR_EVENT_:
			strcat(st_szEventDebugBuf, " ECR事件");
			break;
		case _MULTIFUNC_SLAVE_EVENT_:
			strcat(st_szEventDebugBuf, " 外接設備事件");
			break;
		case _BARCODE_READER_EVENT_:
			strcat(st_szEventDebugBuf, " Barcode 事件");
			break;
		case _POWER_MANAGEMENT_EVENT_:
			strcat(st_szEventDebugBuf, " 省電模式事件");
			break;
		case _ESC_IDLE_UPLOAD_EVENT_:
			strcat(st_szEventDebugBuf, " ESC上傳事件");
			break;
		case _TMS_SCHEDULE_INQUIRE_EVENT_:
			strcat(st_szEventDebugBuf, " TMS排程詢問事件");
			break;
		case _TMS_SCHEDULE_DOWNLOAD_EVENT_:
			strcat(st_szEventDebugBuf, " TMS排程下載事件");
			break;
		case _TMS_PROCESS_EFFECTIVE_EVENT_:
			strcat(st_szEventDebugBuf, " TMS參數生效事件");
			break;
		case _DCC_SCHEDULE_EVENT_:
			strcat(st_szEventDebugBuf, " DCC排程下載事件");
			break;
		case _TMS_DCC_SCHEDULE_EVENT_:
			strcat(st_szEventDebugBuf, " TMS連動DCC下載事件");
			break;
		case _BOOTING_EVENT_:
			strcat(st_szEventDebugBuf, " 開機流程事件");
			break;
		default:
			sprintf(szTemp, "無對應事件[%d]", inKey);
			memcpy(st_szEventDebugBuf, szTemp, strlen(szTemp));
			break;
	}
	
	return st_szEventDebugBuf;
}

/*
Function		: inEVENT_Responder
Date&Time	: 2017/10/2 下午 3:46
Describe		: 
 * 主要跑事件流程的地方，
*/
int inEVENT_Responder(int inKey)
{
	int     inMENU_EVENT, inRetVal = VS_ERROR;
	char    szTRTFileName[12 + 1];
	char    szHostName[8 + 1];
	RTC_NEXSYS		srRTC; 		/* Date & Time */
	EventMenuItem		srEventMenuItem;
	TRANSACTION_OBJECT	pobTran;

	inDISP_LogPrintfWithFlag("----[%s] Key[%s] Line[%d] START----",  __FUNCTION__, chTransferEventKeyToChar(inKey), __LINE__);

	/* [DEBUG] */
	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	if(inKey  == 53){
		
		inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "EVENT_62 inEVENT_Responder INIT" );
	}
		
#ifdef _TOUCH_CAPBILITY_
	/* 觸發事件關閉觸控檔案 */
	inDisTouch_Flush_TouchFile();
#endif

	memset((char *)&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
	memset((char *)&srEventMenuItem, 0x00, sizeof(EventMenuItem));
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));

	inSetPobTranPoint(&pobTran);
	
	/* 取得EDC時間日期 */
	memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" EVT Responder GetSysTime *Error* Line[%d] ", __LINE__);
		return (VS_ERROR);
	}
	
	inFunc_Sync_BRec_Date_Time(&pobTran, &srRTC);

	/* 因ESC需用新的日期為索引,所以重新取得 20181220 [SAM]*/
	inFunc_Sync_BRec_ESC_Date_Time(&pobTran, &srRTC);
	
	pobTran.srBRec.inHDTIndex = -1 ;/* reset 為-1 ,因為LoadRec 是從0開始,避免load錯 */
	
	/* 預設先讀取固定的HDT設定 */
	if (inLoadHDTRec(0) < 0)
	{
		inDISP_DispLogAndWriteFlie(" EVT Responder Load HDT 0 *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	srEventMenuItem.inEventCode = inKey;

	/* 用於儲存IdleMenuKeyIn第一個鍵 */
	ginEventCode = inKey;
	
	switch (inMENU_EVENT)
	{
		default:
#if defined	_LOAD_KEY_AP_
		inRetVal = inMENU_000_MenuFlow_LoadKeyUI(&srEventMenuItem);
#else
		if (ginMachineType == _CASTLE_TYPE_V3C_)
		{
                        inRetVal = inMENU_000_MenuFlow_NEWUI(&srEventMenuItem);
		}
		else if (ginMachineType == _CASTLE_TYPE_V3M_)
		{
			inRetVal = inMENU_000_MenuFlow_NEWUI(&srEventMenuItem);
		}
		else if (ginMachineType == _CASTLE_TYPE_V3P_)
		{
			inRetVal = inMENU_000_MenuFlow_NEWUI(&srEventMenuItem);
		}
		else if (ginMachineType == _CASTLE_TYPE_V3UL_)
		{
			inRetVal = inMENU_000_MenuFlow_V3UL(&srEventMenuItem);
		}
		else if (ginMachineType == _CASTLE_TYPE_MP200_)
		{
			inRetVal = inMENU_000_MenuFlow_MP200(&srEventMenuItem);
		}
		else if (ginMachineType == _CASTLE_TYPE_UPT1000_)
		{
			inRetVal = inMENU_000_MenuFlow_NEWUI(&srEventMenuItem);
		}
		else
		{
			inRetVal = inMENU_000_MenuFlow_NEWUI(&srEventMenuItem);
		}
#endif
		break;
	}
	
	/* 這個步驟會跑設定的OPT 流程，並且會設定部份參數值，以便判斷交易進行 */
	if (inRetVal == VS_SUCCESS)
	{
		pobTran.inFunctionID = srEventMenuItem.inCode;
		pobTran.inRunOperationID = srEventMenuItem.inRunOperationID;
		pobTran.inRunTRTID = srEventMenuItem.inRunTRTID;

		pobTran.inTransactionCode = srEventMenuItem.inCode;
		/* 電子票證的條件 */
		if (srEventMenuItem.inCode == _TICKET_DEDUCT_	||
			srEventMenuItem.inCode == _TICKET_REFUND_	||
			srEventMenuItem.inCode == _TICKET_INQUIRY_	||
			srEventMenuItem.inCode == _TICKET_TOP_UP_	||
			srEventMenuItem.inCode == _TICKET_VOID_TOP_UP_||
			/* [新增電票悠遊卡功能] 新增電票條件，重新設定 inCode [SAM] 2022/6/23 下午 6:49 START */
			srEventMenuItem.inCode == _TICKET_EASYCARD_DEDUCT_ ||
			srEventMenuItem.inCode == _TICKET_EASYCARD_REFUND_ ||
			srEventMenuItem.inCode == _TICKET_EASYCARD_TOP_UP_ ||
			srEventMenuItem.inCode == _TICKET_EASYCARD_VOID_TOP_UP_ ||
			srEventMenuItem.inCode == _TICKET_EASYCARD_INQUIRY_
			/* [新增電票悠遊卡功能] 新增電票條件，重新設定 inCode [SAM] 2022/6/23 下午 6:49 END */
			)
		{
			pobTran.srTRec.inCode = srEventMenuItem.inCode;
		}
		else
		{
			pobTran.srBRec.inCode = srEventMenuItem.inCode;
		}
		
		/* [新增SVC功能]  [SAM] */
		pobTran.fSvcActivedCardAuto = VS_TRUE;
		
		pobTran.srBRec.inOrgCode = srEventMenuItem.inCode;

		pobTran.srBRec.uszCUPTransBit = srEventMenuItem.uszCUPTransBit;
		pobTran.srBRec.uszInstallmentBit = srEventMenuItem.uszInstallmentBit;
		pobTran.srBRec.uszRedeemBit = srEventMenuItem.uszRedeemBit;
		pobTran.uszECRBit = srEventMenuItem.uszECRBit;
		pobTran.uszAutoSettleBit = srEventMenuItem.uszAutoSettleBit;
		pobTran.srBRec.uszFiscTransBit = srEventMenuItem.uszFISCTransBit;	/* 確認是金融卡才on */
		pobTran.srBRec.uszMail_OrderBit = srEventMenuItem.uszMailOrderBit;
		pobTran.uszMultiFuncSlaveBit = srEventMenuItem.uszMultiFuncSlaveBit;
		pobTran.srTRec.uszESVCTransBit = srEventMenuItem.uszESVCTransBit;

		pobTran.srBRec.lnHGTransactionType = srEventMenuItem.lnHGTransactionType;

		/* SmartPay不用簽名 */
		if (pobTran.srBRec.uszFiscTransBit == VS_TRUE)
		{
			pobTran.srBRec.uszNoSignatureBit = VS_TRUE;
		}
		
		inDISP_DispLogAndWriteFlie(" EVT Bef Run OPT inFunctionID[%d] inRunOperationID[%d] inRunTRTID[%d] inTransactionCode[%d]",
					pobTran.inFunctionID, pobTran.inRunOperationID, pobTran.inRunTRTID, pobTran.inTransactionCode);
		inDISP_DispLogAndWriteFlie(" EVT inOrgCode[%d] uszCUPTransBit[%u] uszInstallmentBit[%u] uszRedeemBit[%u]", 
					pobTran.srBRec.inOrgCode, pobTran.srBRec.uszCUPTransBit, pobTran.srBRec.uszInstallmentBit, pobTran.srBRec.uszRedeemBit);
		inDISP_DispLogAndWriteFlie(" EVT uszECRBit[%u] uszAutoSettleBit[%u] uszFiscTransBit[%u] uszMail_OrderBit[%u]",
					pobTran.uszECRBit, pobTran.uszAutoSettleBit, pobTran.srBRec.uszFiscTransBit, pobTran.srBRec.uszMail_OrderBit );
		inDISP_DispLogAndWriteFlie(" EVT uszMultiFuncSlaveBit[%u] uszESVCTransBit[%u] lnHGTransactionType[%ld] ",
					pobTran.uszMultiFuncSlaveBit, pobTran.srTRec.uszESVCTransBit, pobTran.srBRec.lnHGTransactionType);
	
		
		inRetVal = inFLOW_RunOperation(&pobTran, srEventMenuItem.inRunOperationID);

		inDISP_DispLogAndWriteFlie(" EVT Aft Run OPT  inFunctionID[%d] inRunOperationID[%d] inRunTRTID[%d] inTransactionCode[%d]",
					pobTran.inFunctionID, pobTran.inRunOperationID, pobTran.inRunTRTID, pobTran.inTransactionCode);
		inDISP_DispLogAndWriteFlie(" EVT inOrgCode[%d] uszCUPTransBit[%u] uszInstallmentBit[%u] uszRedeemBit[%u]",
					pobTran.srBRec.inOrgCode, pobTran.srBRec.uszCUPTransBit, pobTran.srBRec.uszInstallmentBit, pobTran.srBRec.uszRedeemBit);
		inDISP_DispLogAndWriteFlie(" EVT uszECRBit[%u] uszAutoSettleBit[%u] uszFiscTransBit[%u] uszMail_OrderBit[%u]", 
					pobTran.uszECRBit, pobTran.uszAutoSettleBit, pobTran.srBRec.uszFiscTransBit, pobTran.srBRec.uszMail_OrderBit );
		inDISP_DispLogAndWriteFlie(" EVT uszMultiFuncSlaveBit[%u] uszESVCTransBit[%u] lnHGTransactionType[%ld] ",
					pobTran.uszMultiFuncSlaveBit, pobTran.srTRec.uszESVCTransBit, pobTran.srBRec.lnHGTransactionType);
		
		
		/* [DEBUG] */
		if(inKey  == 53)
		{
			inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVENT_62 After Run Operation");
		}
		
		inDISP_LogPrintfWithFlag(" EVT Over Run OPT RetVal [%d] Line[%d]",inRetVal ,__LINE__);

		if (inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" EVT OPT Result[%d] is *Error* Line[%d]", inRetVal, __LINE__ );
			/* 如果有插SmartPay錯誤要Power Off */
			if (pobTran.uszFISCBit == VS_TRUE)
			{
//				inFISC_PowerOFF(&pobTran);
			}

			if (ginMachineType == _CASTLE_TYPE_V3UL_ ||
                            ginMachineType == _CASTLE_TYPE_V3P_  ||
                            ginMachineType == _CASTLE_TYPE_V3C_)
			{
				/* 被外接設備出錯時回傳 */
				inMultiFunc_SendError(&pobTran, inRetVal);
			}
			else
			{
				
				if (pobTran.inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
				{
					inDISP_LogPrintfWithFlag(" EVT Ecr Send Step Use MsgId[%d] Line[%d]", pobTran.inECRErrorMsg,  __LINE__ );
					inECR_SendError(&pobTran, pobTran.inECRErrorMsg);
				}
				else
				{	
					inDISP_LogPrintfWithFlag(" EVT Ecr Send Step Use Retval[%d] Line[%d]", inRetVal,  __LINE__ );
					inECR_SendError(&pobTran, inRetVal);
				}
			}

			/* 斷線 */
			inCOMM_End(&pobTran);

			/* 要先回傳再顯示錯誤訊息 */
			inFunc_Display_Error(&pobTran);

			/* 退回晶片卡 */
			inFunc_Check_Card_Still_Exist(&pobTran, _REMOVE_CARD_ERROR_);
			
			inSqlite_ShowDbHandle();
			inFILE_ShowFileHandleCnt();
		}
	}

	if (inRetVal == VS_SUCCESS)
	{
		/* 將OPT重新設定 pobTran.inRunTRTID 給 srEventMenuItem.inRunTRTID */
		srEventMenuItem.inRunTRTID = pobTran.inRunTRTID;

		/* 如果當筆要連動結帳 */
		if (pobTran.uszAutoSettleBit == VS_TRUE)
		{
			/* 連動結帳流程 */
			inEVENT_AutoSettle(&pobTran);
		}
		else if (srEventMenuItem.inRunTRTID != 0)
		{
			inDISP_LogPrintfWithFlag(" EVT Run TRT HDT[%d] Line[%d]", pobTran.srBRec.inHDTIndex, __LINE__);

			/* 預防萬一再LoadHDPT一次 */
			if (inLoadHDPTRec(pobTran.srBRec.inHDTIndex) == VS_SUCCESS)
			{
				/* 在這裡決定跑那一個host的TRT */
				if(VS_SUCCESS != inGetTRTFileName(szTRTFileName))
				{
					inDISP_DispLogAndWriteFlie(" EVT GetTRTName[%s] *Error* Line[%d]",szTRTFileName, __LINE__ );
				}

				inDISP_LogPrintfWithFlag(" TRT_Name[%s] Len[%d] Line[%d]",szTRTFileName, strlen(szTRTFileName), __LINE__);

				/* ResetTitle */
				inFunc_ResetTitle(&pobTran);
				
				/* 補上資料處理中的訊息，Resetitle 會把畫面清空 */
//				inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_); /* 處理中... */
				
				memset(szHostName, 0x00, sizeof(szHostName));
				if(VS_SUCCESS != inGetHostLabel(szHostName))
				{
					inDISP_DispLogAndWriteFlie(" EVT GetHostLabel[%d] *Error* Line[%d]",pobTran.srBRec.inHDTIndex,__LINE__ );
				}
				
				inDISP_LogPrintfWithFlag("  inEVENT_Responder HostNm[%s] Lenth[%d] Line[%d]",szHostName , strlen(szHostName), __LINE__);
				/*TODO: 看能不能寫成可變動功能 */
				if (memcmp(szHostName, _HOST_NAME_CREDIT_FUBON_, strlen(_HOST_NAME_CREDIT_FUBON_)) == 0)
				{
					if (!memcmp(_TRT_FILE_NAME_CREDIT_, szTRTFileName, strlen(_TRT_FILE_NAME_CREDIT_)))
					{
						inRetVal = inFUBON_RunTRT(&pobTran, srEventMenuItem.inRunTRTID);
						/* [DEBUG] */
						if(inKey  == 53){
							inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVENT_62 After Run Fubon Trt");
						}
		
						inDISP_LogPrintfWithFlag(" Run Fubon Trt End ");
					}else{
						inDISP_DispLogAndWriteFlie(" EVT TrtName[%s] *Error*  Line[%d]",szTRTFileName, __LINE__);
					}
						
				} 
				else if (!memcmp(_TRT_FILE_NAME_CMAS_, szTRTFileName, strlen(_TRT_FILE_NAME_CMAS_)))
				{	/* [新增電票悠遊卡功能] 新增悠遊卡 TRT執行功能 [SAM] 2022/6/8 下午 3:44 */
					inRetVal = inCMAS_RunTRT(&pobTran, srEventMenuItem.inRunTRTID); // modify by  for CMAS
				}else if (!memcmp(_TRT_FILE_NAME_SVC_, szTRTFileName, strlen(_TRT_FILE_NAME_SVC_)))
				{	/* [新增SVC功能]  [SAM] */
					inRetVal = inSVC_RunTRT(&pobTran, srEventMenuItem.inRunTRTID); 
				}
				else 
				{
					inDISP_DispLogAndWriteFlie(" EVT Can't found host Name[%s] Line[%d]",szHostName, __LINE__);
				}

			}else{
				inDISP_DispLogAndWriteFlie(" EVT LoadHdt *Error* HID[%d]", pobTran.srBRec.inHDTIndex);
			}
		}
		
		/* [DEBUG] */
		if(inKey  == 53){
			inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVENT_62 After Run TRT TABLE");
		}

		/* 顯示所用的檔案個數 */
		inSqlite_ShowDbHandle();
		inFILE_ShowFileHandleCnt();
		
		inDISP_LogPrintfWithFlag(" Aft  Run  Trt ");
	}

#ifdef _COMMUNICATION_CAPBILITY_
	/* 如果有撥接備援，在這裡回復Ethernet */
	if(inCOMM_MODEM_DialBackUpOff(&pobTran) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" EVT Change DialBack Etherent *Error* Line[%d]", __LINE__);
	}
#endif
	
	/* 回IDLE前，清LED燈號 */
	inCTLS_Clear_LED();
	/* 要回Idle，顯示idle圖片ON */
	ginIdleDispFlag = VS_TRUE;

	/* 計時器初始化 */
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 0);
	inDISP_Timer_Start(_TIMER_NEXSYS_2_, 0);
	inDISP_Timer_Start(_TIMER_NEXSYS_3_, 0);
	inDISP_Timer_Start(_TIMER_NEXSYS_4_, 0);

	/* 清鍵盤buffer */
	inFlushKBDBuffer();

	if (ginMachineType == _CASTLE_TYPE_V3UL_ ||
            ginMachineType == _CASTLE_TYPE_V3P_  |
            ginMachineType == _CASTLE_TYPE_V3C_)
	{
		inMultiFunc_FlushRxBuffer(gstMultiOb.srSetting.uszComPort);
	}
	else
	{
		/* ECR清buffer */
		inECR_FlushRxBuffer();
	}

	/* 關閉感應天線，若沒關閉感應天線會和觸控板衝突(有悠遊卡時) */
	inCTLS_Power_Off();
	
#ifdef _TOUCH_CAPBILITY_
	/* 觸發事件關閉觸控檔案 */
	inDisTouch_Flush_TouchFile();
#endif
	
	if (ginMachineType == _CASTLE_TYPE_V3M_)
	{
		/* 電量管理用 */
		inPWM_StandBy_Mode_Timer_Start();
	}

	/* [DEBUG] */
	if(inKey  == 53){
		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "EVENT_62 inEVENT_Responder  END");
	}
	
	inDISP_LogPrintfWithFlag("----[%s] Line[%d] END----", __FUNCTION__, __LINE__);
	
	return (inRetVal);
}

/*
Function        :inEVENT_AutoSettle
Date&Time       :2016/6/15 上午 10:29
Describe        :連動結帳流程
*/
int inEVENT_AutoSettle(TRANSACTION_OBJECT *pobTran)
{
	int	i = 0;
	int	inRetVal = VS_SUCCESS;
	char	szHostName[42 + 1] = {0};
	char	szHostEnable[2 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};
	char	szTRTFileName[12 + 1] = {0};
	char	szBatchNum[6 + 1] = {0};
	unsigned char	uszSettleFailBit = VS_FALSE;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* ECR結帳立即回傳 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		inFLOW_RunFunction(pobTran, _FUNCTION_ECR_SEND_TRANSACTION_RESULT_);
	}

	for (i = 0;; ++i)
	{
		/* 先LoadHDT */
		if (inLoadHDTRec(i) == VS_ERROR)
		{
			/* 當找不到第i筆資料會回傳VS_ERROR */
			inDISP_LogPrintfWithFlag(" EVT Auto Settle Load HDT[%d] *Error* Line[%d]", i, __LINE__);
			break;
		}

		/* ESC要跳過 */
		memset(szHostName, 0x00, sizeof(szHostName));
		inGetHostLabel(szHostName);
		if (memcmp(szHostName, _HOST_NAME_ESC_, strlen(_HOST_NAME_ESC_)) == 0)
		{
			continue;
		}
		/* 因為公司TMS會下CUP主機，因富邦是同批次，所以CUP要跳過 所以新增 20190214 [SAM] */
		if (memcmp(szHostName, _HOST_NAME_CUP_, strlen(_HOST_NAME_CUP_)) == 0)
		{
			continue;
		}
		
		/* GET HOST Enable */
		/* 結帳沒開也不跑 */
		memset(szHostEnable, 0x00, sizeof(szHostEnable));
		if (inGetHostEnable(szHostEnable) == VS_ERROR)
		{
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%d HostEnable: %s", i, szHostEnable);
				inDISP_LogPrintf(szDebugMsg);
			}
		}

		/* 看結帳開關 */

		if (memcmp(szHostEnable, "Y", 1) != 0)
		{
			/* 如果HostEnable != Y，就continue */
			continue;
		}
		else
		{
			pobTran->srBRec.inHDTIndex = i;
		}

		/* 如果主機有開，才loadHDPT */
		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_SUCCESS)
		{
			memset(szBatchNum, 0x00, sizeof(szBatchNum));
			inGetBatchNum(szBatchNum);
			pobTran->srBRec.lnBatchNum = atol(szBatchNum);

			/* 在這裡決定跑那一個host的TRT */
			memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
			inGetTRTFileName(szTRTFileName);

			/* 跑各家TRT流程,如有新增主機需要再此進行修改 */
			if (!memcmp(_TRT_FILE_NAME_CREDIT_, szTRTFileName, strlen(_TRT_FILE_NAME_CREDIT_)))
			{
				/*TODO: 看能不能寫成可變動功能 */
				inRetVal = inFUBON_RunTRT(pobTran, pobTran->inRunTRTID);

			}else if (!memcmp(_TRT_FILE_NAME_CMAS_, szTRTFileName, strlen(_TRT_FILE_NAME_CMAS_)))
			{	/* [新增電票悠遊卡功能] 新增悠遊卡 TRT執行功能  移除原來 _TRT_FILE_NAME_ESVC_ 的  inNCCC_Ticket_RunTRT [SAM] 2022/6/8 下午 3:44 */
				/* [先移除功能要再加回] 2022/6/8 [SAM] */
//				gsrECROb.szAccountData.inCMAS_Enable = 1; /*  此參數目前看起來為 KIOSK 參數，看要不要  */
//				END				
				inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_CHECK_TERM_STATUS_CMAS_);
				inRetVal = inFLOW_RunFunction(pobTran, _CMAS_DECIDE_TRANS_TYPE_);
				inRetVal = inCMAS_RunTRT(pobTran, pobTran->inRunTRTID);
			}
			else if (!memcmp(_TRT_FILE_NAME_SVC_, szTRTFileName, strlen(_TRT_FILE_NAME_SVC_)))
			{	/* [新增SVC功能]  [SAM] */
				inRetVal = inSVC_RunTRT(pobTran, pobTran->inRunTRTID);
			}
			else{
				inDISP_DispLogAndWriteFlie(" EVT AutoSettle *Error* B_HID[%d] Line[%d]", pobTran->srBRec.inHDTIndex,  __LINE__);
			}

			if (inRetVal != VS_SUCCESS)
			{
				/* 列印XXXHost結帳失敗 */
				while (inRetVal != VS_SUCCESS)
				{
					uszSettleFailBit = VS_TRUE;
					inRetVal = inCREDIT_PRINT_AutoSettle_Failed_ByBuffer();
				}
			}
		}else{
			inDISP_DispLogAndWriteFlie(" EVT AutoSettle Load HDPT *Error* HName[%s] TRTName[%s] Line[%d]", szHostName, szTRTFileName, __LINE__);
		}

	}

	/* 連動結帳跑完，把Bit Off，做TMS下載 */
	pobTran->uszAutoSettleBit = VS_FALSE;
	
/* 富邦目前不做結帳TMS自動詢問的功能，先拿掉 20190213 [SAM]*/
#if 0 	
	/* 如果結帳失敗，就不跑下載 */
	/* 分ISO和FTP，若是ISO，則有結帳失敗就不下，
	   FTP則是不論結帳是否成功，一律執行 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTMSDownloadMode(szTemplate);
	if (memcmp(szTemplate, _TMS_DLMODE_FTPS_, strlen(_TMS_DLMODE_FTPS_)) == 0)
	{
		inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_TMS_SCHEDULE_INQUIRE_);
		inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_TMS_DOWNLOAD_SETTLE_);
		inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_TMS_REBOOT_);
	}
	else
	{
		/* 結帳失敗，不下載 */
		if (uszSettleFailBit == VS_TRUE)
		{

		}
		else
		{
			/* ISO不用詢問 */
			inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_TMS_DOWNLOAD_SETTLE_);
			inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_TMS_REBOOT_);
		}
	}
#endif
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (VS_SUCCESS);
}
