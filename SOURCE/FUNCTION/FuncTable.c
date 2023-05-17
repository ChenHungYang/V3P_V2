#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <libxml/parser.h>
#include <sqlite3.h>


#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Transaction.h"

#include "../PRINT/Print.h"

#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"

#include "../EVENT/Flow.h"
#include "../EVENT/Event.h"

#include "../COMM/Comm.h"
#include "../COMM/Modem.h"

//#include "../TMS/EDCTms.h"
#include "../TMS/EDCTmsDefine.h"
#include "../TMS/EDCTmsFlow.h"
#include "../TMS/EDCTmsUnitFunc.h"
#include "../TMS/EDCTmsAnalyseFunc.h"
#include "../TMS/EDCTmsFileProc.h"
#include "../TMS/EDCTmsFtpFunc.h"

#include "../NEXSYSESC/NexsysEsc.h"

#include "../../EMVSRC/EMVsrc.h"
#include "../../CTLS/CTLS.h"


#include "../FUNCTION/Accum.h"

#include "../FUNCTION/CARD_FUNC/CardFunction.h"

#include "../FUNCTION/CREDIT_FUNC/CreditCheckFunc.h"
#include "../FUNCTION/CREDIT_FUNC/CreditFlowDispFunc.h"
#include "../FUNCTION/CREDIT_FUNC/CreditProcessDataFunc.h"
#include "../FUNCTION/CREDIT_FUNC/CreditPrintFunc.h"
#include "../FUNCTION/CREDIT_FUNC/Creditfunc.h"

#include "../FUNCTION/INSTALL_FUNC/InstCheckFunc.h"
#include "../FUNCTION/INSTALL_FUNC/InstProcessDataFunc.h"

#include "../FUNCTION/REDEEM_FUNC/RedeemCheckFunc.h"
#include "../FUNCTION/REDEEM_FUNC/RedeemProcessDataFunc.h"

#include "../FUNCTION/CUP_FUNC/CupCheckFunc.h"
#include "../FUNCTION/CUP_FUNC/CupFlowDispFunc.h"
#include "../FUNCTION/CUP_FUNC/CupProcessDataFunc.h"

/* [新增SVC功能]  [SAM] */
#include "../../SVCSRC/SvcSrc.h"
#include "../../SVCSRC/SvcFunc.h"
#include "../../SVCSRC/SvcAccum.h"
#include "../../SVCSRC/SvcPrint.h"
#include "../../SVCSRC/SvcIso.h"

#include "Accum.h"
#include "Sqlite.h"
#include "Function.h"
#include "FuncTable.h"
#include "Card.h"
#include "Batch.h"
#include "Signpad.h"
#include "ECR_FUNC/ECR.h"
#include "MULTI_FUNC/MultiFunc.h"
#include "PowerManagement.h"

#include "../../FUBON/FUBONfunc.h"
#include "../../FUBON/FUBONiso.h"


#include "../../SOURCE/FUNCTION/AccountFunction/PrintBillInfo.h"

#include "../../CMAS/CMASsrc.h"
#include "../../CMAS/CMASFunction.h"

#include "../../NCCC/NCCCTicketSrc.h"

#include "COSTCO_FUNC/Costco.h"

/* 定義Function Table只要有跑Function Index都要來這裡抓，另外這裡放的位置要與FuncID.h放的一致 */
FUNC_TABLE FunctionTable[] ={
	/* inFuncId,                                 	inFunctionPoint */

	/* MultiFunc slave */
	{_MULTIFUNC_RC_COMMAND_, inMultiFunc_Receive_Command},
	{_MULTIFUNC_SEND_RESULT_, inMultiFunc_SendResult},

	/* FLOW */
	{_FUNCTION_GET_CARD_FIELDS_FLOW_, inFunc_GetCardFields_Flow},
	{_FUNCTION_GET_CARD_FIELDS_, inFunc_GetCardFields},
	{_FUNCTION_GET_CARD_FIELDS_ICC_, inFunc_GetCardFields_ICC},
	{_FUNCTION_GET_CARD_FIELDS_CTLS_, inFunc_GetCardFields_CTLS},
	{_FUNCTION_GET_CARD_FIELDS_REFUND_CTLS_FLOW_, inFunc_GetCardFields_Refund_CTLS_Flow},
	{_FUNCTION_GET_CARD_FIELDS_REFUND_CTLS_, inFunc_GetCardFields_Refund_CTLS},

	/* EMV PROCESS FLOW */
	{_EMV_PROCESS_, inEMV_Process},

	/* 富邦 FLOW*/
	{_FUBON_FUNCTION_MUST_SETTLE_CHECK_, inFUBON_Func_Must_SETTLE},
	{_FUBON_ISO_FUNCTION_BUILD_AND_SEND_PACKET_, inFUBON_ISO_BuildAndSendPacket},
	{_FUBON_SET_ONLINE_OFFLINE_, inFUBON_FuncSetOnlineOffline},
	{_FUBON_FUNCTION_MAKE_REFNO_, inFUBON_ISO_MakeReferenceNumber},
	{_FUBON_TURN_OFF_SETTLE_FLAG_, inFUBON_DisableFuUnBlanceSettleFlag},


	{_COMM_START_, inCOMM_ConnectStart},
	{_COMM_END_, inCOMM_End},

	/* [SAM] 電簽新增  */
	{_NEXSYS_ESC_CHECK_, inNEXSYS_ESC_Check},
	{_NEXSYS_ESC_UPLOAD_, inNEXSYS_ESC_Func_Upload}, /* 已寫LOG */
	{_NEXSYS_ESC_UPLOAD_IDLE_, inNEXSYS_ESC_Func_Upload_Idle},
	{_NEXSYS_ESC_UPLOAD_SETTLE_, inNEXSYS_ESC_Func_Upload_Settle},
	{_NEXSYS_ESC_UPDATE_RE_PRINT_, inNEXSYS_ESC_Func_UpdateReprintCount}, /* 重印簽單ESC紙本個數 20181220[SAM]  */


	{_FUNCTION_SELECT_I_R_PAYMENT_TYPE_, inFunc_SelectInstRedeemPaymentType},

	{_FUNCTION_GET_SIGNPAD_, inSIGN_TouchSignature_Flow},
	{_FUNCTION_PRINT_RECEIPT_, inFunc_PrintReceipt},/* 目前沒在用 2022/8/19 下午 1:49 */

	{_FUNCTION_CL_POWER_OFF_, inFunc_CL_Power_Off},

	/*CREDIT */
	{_CREDIT_FUNCTION_GET_AMOUNT_, inCREDIT_Func_GetAmount},
	{_CREDIT_FUNCTION_GET_TIP_AMOUNT_, inCREDIT_Func_GetTipAmount},
	{_CREDIT_FUNCTION_GET_REFUND_AMOUNT_, inCREDIT_Func_GetRefundAmount},
	{_CREDIT_FUNCTION_GET_ADJUST_AMOUNT_, inCREDIT_Func_GetAdjustAmount},
	{_CREDIT_FUNCTION_GET_STOREID_, inCREDIT_Func_GetStoreID},
	{_CREDIT_FUNCTION_GET_AUTHCODE_, inCREDIT_Func_GetAuthCode},
	{_CREDIT_FUNCTION_CHECKRESULT_, inCREDIT_Func_CheckResult},
	{_CREDIT_FUNCTION_CHECK_RESULT_SETTLE_, inACCUM_ReviewReport_Total_Settle},
	{_CREDIT_FUNCTION_GET_OPT_REFUND_AMOUNT_, inCREDIT_Func_Get_OPT_RefundAmount},
	//		{_CREDIT_FUNCTION_SET_SETTLE_FLAG_ON_,		inEDCTMS_FUNC_SetTmsFtpSettleFlag},

	/* IDLE FLOW */
	{_CREDIT_FUNCTION_GET_OPT_AMOUNT_, inCREDIT_Func_Get_OPT_Amount},
	{_CREDIT_FUNCTION_GET_OPT_AUTHCODE_, inCREDIT_Func_Get_OPT_AuthCode},
	{_CREDIT_FUNCTION_GET_OPT_STOREID_, inCREDIT_Func_Get_OPT_StoreID},
	{_CREDIT_FUNCTION_GET_OPT_PERIOD_, inCREDIT_Func_Get_OPT_Period},

	/* ECR SALE FLOW */
	{_FUNCTION_ECR_RECEIVE_TRANSACTION_REQUEST_, inECR_Receive_Transaction},
	{_FUNCTION_ECR_SEND_TRANSACTION_RESULT_, inECR_Send_Transaction_Result},
	{_FUNCTION_ECR_SEND_INQUIRY_RESULT_, inECR_Send_Inquiry_Result},
	{_FUNCTION_ECR_RECEIVE_SECOND_TRANSACTION_REQUEST_, inECR_Receive_Second_Transaction},
	{_FUNCTION_ECR_LOAD_TMK_, inECR_Load_TMK},
	{_FUNCTION_ECR_CUSTOMER_FLOW_, inECR_Customer_Flow},
        {_FUNCTION_ECR_CUSTOMER_FLOW2_, inECR_Customer_Flow2},  /* 20230502 Miyano 好市多加油站授權完成 */

	{_FUNCTION_GET_CARD_FIELDS_FISC_, inFunc_GetCardFields_FISC},
	{_FUNCTION_GET_CARD_FIELDS_FISC_CTLS_, inFunc_GetCardFields_FISC_CTLS},
	{_FUNCTION_GET_CARD_FIELDS_FISC_REDEND_CTLS_, inFunc_GetCardFields_FISC_CTLS_Refund},
	{_FUNCTION_GET_CARD_FIELDS_LOYALTY_REDEEM_, inFunc_GetCardFields_Loyalty_Redeem_Swipe},
	{_FUNCTION_GET_CARD_FIELDS_LOYALTY_REDEEM_CTLS_, inFunc_GetCardFields_Loyalty_Redeem_CTLS},
	{_FUNCTION_GET_BARCODE_, inFunc_GetBarCodeFlow},
	{_FUNCTION_GET_CARD_FIELDS_HG_, inFunc_GetCardFields_HG},
	{_FUNCTION_GET_CREDIT_CARD_FIELDS_HG_, inFunc_GetCreditCardFields_HG},
	{_FUNCTION_GET_CREDIT_CARD_FIELDS_MAIL_ORDER_, inFunc_GetCardFields_MailOrder},

	{_FUNCTION_PRINT_TOTAL_REPORT_, inFunc_PrintTotalReport},/* 目前沒在用 2022/8/19 下午 2:50 */
	{_FUNCTION_PRINT_DETAIL_REPORT_, inFunc_PrintDetailReport},/* 目前沒在用 2022/8/19 下午 2:50 */
	{_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_, inFunc_PrintReceipt_ByBuffer_Flow}, /* 已寫LOG */
	{_FUNCTION_REPRINT_RECEIPT_BY_BUFFER_, inFunc_RePrintReceipt_ByBuffer_Flow},
	{_FUNCTION_PRINT_TOTAL_REPORT_BY_BUFFER_, inFunc_PrintTotalReport_ByBuffer},// 2022/10/21 有在使用
	{_FUNCTION_PRINT_DETAIL_REPORT_BY_BUFFER_, inFunc_PrintDetailReport_ByBuffer},// 2022/10/21 有在使用
	{_FUNCTION_REVIEW_, inFunc_ReviewReport},
	{_FUNCTION_TOTAL_REVIEW_, inACCUM_ReviewReport_Total},
	{_FUNCTION_DETAIL_REVIEW_, inBATCH_ReviewReport_Detail_Flow_By_Sqlite},
	{_FUNCTION_DETAIL_REVIEW_NEWUI_, inBATCH_ReviewReport_Detail_NEWUI_Flow_By_Sqlite},
	{_FUNCTION_GET_HOST_NUM_, inFunc_GetHostNum},
	{_FUNCTION_GET_HOST_NUM_NEWUI_, inFunc_GetHostNum_NewUI},
	{_FUNCTION_USER_CHOICE_, inBATCH_FuncUserChoice_By_Sqlite},
	{_FUNCTION_CHECK_PAN_EXPDATE_, inFunc_CheckPAN_EXP},
	{_FUNCTION_CHECK_PAN_EXPDATE_LOYALTY_REDEEM_, inFunc_CheckPAN_EXP_Loyalty_Redeem},
	{_FUNCTION_CHECK_BARCODE_LOYALTY_REDEEM_, inFunc_CheckBarcode_Loyalty_Redeem},
	{_FUNCTION_CHECK_HG_PAN_EXPDATE_, inFunc_CheckHGPAN_EXP},
	{_FUNCTION_UPDATE_INV_, inFunc_UpdateInvNum},
	{_FUNCTION_UPDATE_BATCH_NUM_, inFunc_UpdateBatchNum},
	{_FUNCTION_DELETE_BATCH_, inFunc_DeleteBatch},
	{_FUNCTION_DELETE_BATCH_FLOW_, inFunc_DeleteBatch_Flow},
	{_FUNCTION_DELETE_ACCUM_, inFunc_DeleteAccum},
	{_FUNCTION_DELETE_ACCUM_FLOW_, inFunc_DeleteAccum_Flow},
	{_FUNCTION_REST_BATCH_INV_, inFunc_ResetBatchInvNum},

	{_FUNCTION_All_HOST_MUST_SETTLE, inFunc_All_Host_Must_Settle_Check},
	{_FUNCTION_CHECK_TERM_STATUS_, inFunc_CheckTermStatus},/* 檢查端末機狀態、例如是否有做過TMS */
	{_FUNCTION_CHECK_TERM_STATUS_TICKET_, inFunc_CheckTermStatus_Ticket},
	{_FUNCTION_DUPLICATE_CHECK_, inFunc_DuplicateCheck},
	{_FUNCTION_DUPLICATE_SAVE_, inFunc_DuplicateSave},
	{_FUNCTION_IDLE_CHECKCUSTOMPASSWORD_FLOW_, inFunc_Idle_CheckCustomPassword_Flow},
	{_UPDATE_ACCUM_, inACCUM_UpdateFlow},
	{_UPDATE_BATCH_, inBATCH_FuncUpdateTxnRecord_By_Sqlite},
	{_COMM_MODEM_PREDIAL_, inModem_Predial},
	{_POWER_SAVING_FLOW_SLEEP_, inPWM_Enter_Sleep_Mode},
	{_POWER_SAVING_FLOW_STANDBY_, inPWM_Enter_StandBy_Mode},
	/* 由 NCCC_FUNCTION_CHECK_TRANS_FUNCTION_FLOW_  修改來的 */
	{_FUNCTION_CHECK_TRANS_FUNCTION_FLOW_, inCREDIT_CheckTransactionFunctionFlow},
	/* 由 NCCC_FUNCTION_VOID_CHECK_  修改來的 */
	{_FUNCTION_VOID_CHECK_, inCREDIT_CheckVoidFunc},
	/* 由 NCCC_FUNCTION_VOID_CONFIRM_  修改來的 */
	{_FUNCTION_VOID_CONFIRM_, inCREDIT_ConfirmToRunVoidTrans},
	/* 由 NCCC_FUNCTION_DISPLAY_PLEASE_LOGON_FIRST_  修改來的 */
	{_FUNCTION_DISPLAY_PLEASE_LOGON_FIRST_, inCUP_DisplayPleaseLogonFirst},
	/* _NCCC_FUNCTION_MAKE_INSTALLMENT_DATA_ */
	{_FUNCTION_MAKE_INSTALLMENT_DATA_, inINST_MakeInstDataFunc},
	/* _NCCC_FUNCTION_MAKE_REDEEM_DATA_ */
	{_FUNCTION_MAKE_REDEEM_DATA_, inREDEEM_MakeRedeemDataFunc},
	/* _NCCC_FUNCTION_CUP_VOID_CHECK_ */
	{_FUNCTION_CUP_VOID_CHECK_, inCUP_CheckVoidFunc},
	/* _NCCC_FUNCTION_CUP_VOID_CONFIRM_ */
	{_FUNCTION_CUP_VOID_CONFIRM_, inCUP_ConfirmToRunVoidTrans},
	/* _NCCC_FUNCTION_TIP_CHECK_ */
	{_FUNCTION_TIP_CHECK_, inCREDIT_CheckTipFunc},
	/* _NCCC_FUNCTION_ADJUST_CHECK_ */
	{_FUNCTION_ADJUST_CHECK_, inCREDIT_CheckAdjustFunc},
	/* _NCCC_FUNCTION_GET_CUP_PIN  */
	{_FUNCTION_GET_CUP_PIN_, inCUP_GetPinFunc},

	/* _NCCC_FUNCTION_CUP_REFUND_LIMIT_CHECK_ */
	{_FUNCTION_CUP_REFUND_LIMIT_CHECK_, inCUP_CheckRefuneLimitFunc},

	/* _NCCC_FUNCTION_GET_TRANSACTION_NO_FROM_PAN_ */
	{_FUNCTION_GET_TRANSACTION_NO_FROM_PAN_, inCARDFUNC_GetTransactionNoFromPANByNcccRule},


	/*CREDIT */
	{_CREDIT_FUNCTION_GET_PERIOD_, inCREDIT_Func_GetPeriod},
	{_CREDIT_FUNCTION_GET_DOWNPAYMENT_, inCREDIT_Func_GetDownPayment},
	{_CREDIT_FUNCTION_GET_INSTPAYMENT_, inCREDIT_Func_GetInstPayment},
	{_CREDIT_FUNCTION_GET_INSTFEE_, inCREDIT_Func_GetInstFee},
	{_CREDIT_FUNCTION_GET_REDEEM_PAID_AMT_, inCREDIT_Func_GetPaidAmount},
	{_CREDIT_FUNCTION_GET_PAY_AMOUNT_, inCREDIT_Func_GetPayAmount},
	{_CREDIT_FUNCTION_GET_REDEEM_POINT_, inCREDIT_Func_GetRedeemPoint},
	{_CREDIT_FUNCTION_PRINT_PARAM_TERM_, inCREDIT_Func7PrintParamTerm},/* 目前沒在用 2022/8/19 下午 2:50 */
	{_CREDIT_FUNCTION_GET_ORI_TRANSDATE_, inCREDIT_Func_GetOriTransDate},
	{_CREDIT_FUNCTION_GET_ORI_AMOUNT_, inCREDIT_Func_GetOriAmount},
	{_CREDIT_FUNCTION_GET_ORI_PREAUTH_AMOUNT_, inCREDIT_Func_GetOriPreAuthAmount},
	{_CREDIT_FUNCTION_GET_PRECOMP_AMOUNT_, inCREDIT_Func_GetPreCompAmount},
	{_CREDIT_FUNCTION_GET_PRODUCT_CODE_, inCREDIT_Func_GetProductCode},
	{_CREDIT_FUNCTION_GET_CVV2_, inCREDIT_Func_GetCVV2},
	{_CREDIT_FUNCTION_GET_DCC_ORI_TRANSDATE_, inCREDIT_Func_GetDCCOriTransDate},
	{_CREDIT_FUNCTION_GET_DCC_ORI_AMOUNT_, inCREDIT_Func_GetDCCOriAmount},
	{_CREDIT_FUNCTION_GET_DCC_TIP_AMOUNT_, inCREDIT_Func_GetDCCTipAmount}, /* 輸入外幣小費 */
	/* SMARTPAY專用 */
	{_CREDIT_FUNCTION_GET_FISC_RRN_, inCREDIT_Func_GetFiscRRN},
	{_CREDIT_FUNCTION_GET_FISC_REFUND_, inCREDIT_Func_GetFiscRefund},
	{_CREDIT_FUNCTION_GET_FISC_ORI_TRANSDATE_, inCREDIT_Func_GetFiscOriTransDate},
	/* 票證使用 */
	{_CREDIT_FUNCTION_GET_RF_NUMBER_, inCREDIT_Func_Get_OPT_RFNumber}, /* 請輸入RFN序號 */

	/* IDLE FLOW */
	{_CREDIT_FUNCTION_GET_OPT_DOWNPAYMENT_, inCREDIT_Func_Get_OPT_DownPayment},
	{_CREDIT_FUNCTION_GET_OPT_INSTPAYMENT_, inCREDIT_Func_Get_OPT_InstPayment},
	{_CREDIT_FUNCTION_GET_OPT_INSTFEE_, inCREDIT_Func_Get_OPT_InstFee},
	{_CREDIT_FUNCTION_GET_OPT_PRODUCT_CODE_, inCREDIT_Func_Get_OPT_ProductCode},
	{_CREDIT_FUNCTION_GET_OPT_PAY_AMOUNT_, inCREDIT_Func_Get_OPT_PayAmount},
	{_CREDIT_FUNCTION_GET_OPT_REDEEM_POINT_, inCREDIT_Func_Get_OPT_RedeemPoint},
	{_CREDIT_FUNCTION_GET_OPT_ORI_TRANSDATE_, inCREDIT_Func_Get_OPT_OriTransDate},
	{_CREDIT_FUNCTION_GET_OPT_ORI_TRANSDATE_ESVC_, inCREDIT_Func_Get_OPT_OriTransDate_ESVC},

	{_CREDIT_FUNCTION_GET_OPT_ORI_PREAUTH_AMOUNT_, inCREDIT_Func_Get_OPT_OriPreAuthAmount},
	{_CREDIT_FUNCTION_GET_OPT_PRECOMP_AMOUNT_, inCREDIT_Func_Get_OPT_PreCompAmount},
	{_CREDIT_FUNCTION_GET_OPT_FISC_RRN_, inCREDIT_Func_Get_OPT_FiscRRN},
	{_CREDIT_FUNCTION_GET_OPT_FISC_REFUND_, inCREDIT_Func_Get_OPT_FiscRefund},
	{_CREDIT_FUNCTION_GET_OPT_FISC_ORI_TRANSDATE_, inCREDIT_Func_Get_OPT_FiscOriTransDate},

	{_CREDIT_FUNCTION_PRINT_PARAM_TERM_BY_BUFFER_, inCREDIT_PRINT_Func7PrintParamTerm_ByBuffer},
//	{_CREDIT_FUNCTION_PRINT_DCC_PARAM_BY_BUFFER_,	inCREDIT_PRINT_Func7PrintDCCParamTerm_ByBuffer},
#if 0
	/* AMEX FLOW */
	{_AMEX_FUNCTION_GET_4DBC_, inAMEX_Func_Get4DBC},
	{_AMEX_FUNCTION_SET_TXN_ONLINE_OFFLINE_, inAMEX_Func_SetTxnOnlineOffline},
	{_AMEX_FUNCTION_BUILD_AND_SEND_PACKET_, inAMEX_Func_BuildAndSendPacket},
	{_AMEX_FUNCTION_VOID_CONFIRM_, inAMEX_Func_VOID_Confirm},
	{_AMEX_FUNCTION_VOID_CHECK_, inAMEX_Func_VOID_Check},
	{_AMEX_FUNCTION_TIP_CHECK_, inAMEX_Func_TIP_Check},
	{_AMEX_FUNCTION_ADJUST_CHECK_, inAMEX_Func_ADJUST_Check},
	{_AMEX_FUNCTION_MUST_SETTLE_CHECK_, inAMEX_Func_Must_SETTLE},

	/* DINERS FLOW */
	{_DINERS_FUNCTION_SET_TXN_ONLINE_OFFLINE_, inDINERS_Func_SetTxnOnlineOffline},
	{_DINERS_FUNCTION_BUILD_AND_SEND_PACKET_, inDINERS_Func_BuildAndSendPacket},
	{_DINERS_FUNCTION_VOID_CONFIRM_, inDINERS_Func_VOID_Confirm},
	{_DINERS_FUNCTION_VOID_CHECK_, inDINERS_Func_VOID_Check},
	{_DINERS_FUNCTION_TIP_CHECK_, inDINERS_Func_TIP_Check},
	{_DINERS_FUNCTION_ADJUST_CHECK_, inDINERS_Func_ADJUST_Check},
	{_DINERS_FUNCTION_MUST_SETTLE_CHECK_, inDINERS_Func_Must_SETTLE},
	{_DINERS_FUNCTION_CHECK_TRANS_FUNCTION_, inDINERS_Func_CheckTransactionFunction},

#endif		

#if 0
	/* FISC */
	{_FISC_CARD_PROCESS_TRAN_, inFISC_CardProcess},
	{_FISC_CARD_ENTER_PIN_, inFISC_GetUserPin},
	{_FISC_CARD_VERIFY_PIN_, inFISC_VerifyPin},

	/* TMS */
	{_FUNC5_TMS_SELECT_FLOW_, inNCCCTMS_TMS_Func5SelectFlow},
	{_FUNC5_TASK_REPORT_, inNCCCTMS_Func5ReturnTaskReport},
	{_FUNCTION_TMS_CONNECT_, inNCCCTMS_ConnectToServer},
	{_FUNCTION_TMS_DISCONNECT_, inNCCCTMS_DisConnect_From_Server},
	{_FUNCTION_TMS_SEND_RECEIVE_FLOW_, inNCCCTMS_FuncSendReceive_Flow},
	{_FUNCTION_TMS_SEND_RECEIVE_, inNCCCTMS_FuncSendReceive},
	{_FUNCTION_TMS_SEND_RECEIVE_FTPS_, inNCCCTMS_FuncSendReceive_FTPS},
	{_FUNCTION_TMS_RESULT_HANDLE_, inNCCCTMS_FuncResultHandle},
	{_FUN6_TMS_DOWNLOAD_, inNCCCTMS_Func6TMSDownload},
	{_FUN6_TRACELOG_UPLOAD_, inNCCCTMS_Func6TraceLog_Upload},

	{_FUNCTION_TMS_SCHEDULE_DOWNLOAD_, inNCCCTMS_Schedule_Download},
	{_FUNCTION_TMS_DOWNLOAD_SETTLE_, inNCCCTMS_Download_Settle},
	{_FUNC5_DCC_DOWNLOAD_PARAMETER_, inNCCCTMS_DCC_FuncDownloadParameter},
	{_FUNCTION_TMS_IDLE_UPDATE_, inNCCCTMS_IdleUpdate},
	{_FUNCTION_TMS_PRINT_RETURN_TASK_, inNCCCTMS_PRINT_Return_Task},
#endif 

	/* TMS */
	{_FUNCTION_TMS_DOWNLOAD_SETTLE_, inEDCTMS_DownloadTmsDataAfterSettle},
	{_FUNCTION_TMS_SCHEDULE_INQUIRE_, inEDCTMS_ScheduleInquireFlow},
	{_FUNCTION_TMS_REBOOT_, inEDCTMS_FUNC_Reboot},
	{_FUNCTION_NEXSYS_RENEW_FPT_TIME_, inEDCTMS_SaveSysDownloadDateTime},


#if 0
	/* 票證DEMO */
	{_TICKET_DECIDE_TRANS_TYPE_, inNCCC_Ticket_Decide_Trans_Type},
	{_TICKET_CHECK_TRANS_ENABLE_, inNCCC_Ticket_Func_Check_Transaction_Function_Flow},
	{_TICKET_GET_PARM_, inNCCC_Ticket_Get_ParamValue},
	{_TICKET_GET_ESVC_CARD_FLOW_, inNCCC_Ticket_Get_Card_Flow},
	{_TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_, inNCCC_TICKET_Func_SetTxnOnlineOffline},
	{_TICKET_FUNCTION_BUILD_AND_SEND_PACKET_, inNCCC_TICKET_Func_BuildAndSendPacket},
	{_TICKET_CHECK_ACK_HOST_, inNCCC_Ticket_Check_AckHost},
	{_TICKET_CONNECT_, inNCCC_Ticket_Connect},
	{_TICKET_DISCONNECT_, inNCCC_Ticket_DisConnect},
	{_TICKET_GET_DAVTI_PARM_, inNCCC_Ticket_Get_DAVTI_Data},
	{_TICKET_UPDATE_ACCUM_, inACCUM_Update_Ticket_Flow},
	{_TICKET_UPDATE_BATCH_, inBATCH_FuncUpdateTxnRecord_Ticket_By_Sqlite},
	{_TICKET_UPDATE_INV_, inNCCC_Ticket_Func_UpdateTermInvNum},
	{_TICKET_IPASS_TAPCARD_FIRST_, inIPASS_First_Tap},
	{_TICKET_IPASS_TAPCARD_SECOND_, inIPASS_Second_Tap},
	{_TICKET_IPASS_QUERYING_FLOW_, inIPASS_Query_Flow},
	{_TICKET_ECC_API_FLOW_, inECC_APIflow},
	{_TICKET_ECC_SETTLE_FLOW_, inECC_Settle_Flow},
	{_TICKET_GET_VOID_TOP_UP_AMOUNT_FROM_BATCH_, inNCCC_Ticket_Get_Void_Top_Up_Amount_From_Batch},
#endif

	/* 開機流程使用  */
	{_EDC_BOOTING_DISPLAY_INITIAL_, inFunc_Booting_Flow_Display_Initial}, /* 目前有在用 20190619 [SAM] */
	{_EDC_BOOTING_PRINT_INITIAL_, inFunc_Booting_Flow_Print_Initial}, /* 目前有在用 20190619 [SAM] */
	{_EDC_BOOTING_PRINT_IMAGE_INITIAL_, inFunc_Booting_Flow_Print_Image_Initial},
	{_EDC_BOOTING_CASTLE_LIBRARY_INITIAL_, inFunc_Booting_Flow_Castle_library_Initial},
	{_EDC_BOOTING_LOAD_TABLE_, inFunc_Booting_Flow_Load_Table},
	{_EDC_BOOTING_SYNC_DEBUG_, inFunc_Booting_Flow_Sync_Debug},
	{_EDC_BOOTING_COMM_INITIAL_, inFunc_Booting_Flow_Communication_Initial},
	{_EDC_BOOTING_ECR_INITIAL_, inFunc_Booting_Flow_ECR_Initial},
	{_EDC_BOOTING_MULTI_FIRST_INITIAL_, inFunc_Booting_Flow_MultiFunc_First_Initial}, /* 目前用在 3UL*/
	{_EDC_BOOTING_EMV_INITIAL_, inFunc_Booting_Flow_EMV_Initial},
	{_EDC_BOOTING_TMSOK_CHECK_INITIAL_, inFunc_Booting_Flow_TMSOK_Check_Initial},
	{_EDC_BOOTING_ETICKET_INITIAL_, inFunc_Booting_Flow_Eticket_Initial},
	{_EDC_BOOTING_CTLS_INITIAL_, inFunc_Booting_Flow_CTLS_Initial},
	{_EDC_BOOTING_SQLITE_INITIAL_, inFunc_Booting_Flow_SQLite_Initial},
	{_EDC_BOOTING_CUP_LOGON_, inFunc_Booting_Flow_CUP_LOGON},
	{_EDC_BOOTING_TSAM_INITIAL_, inFunc_Booting_Flow_TSAM_Initial},
	{_EDC_BOOTING_TMS_INQUIRE_, inFunc_Booting_Flow_TMS_Parameter_Inquire},
	{_EDC_BOOTING_TMS_DCC_SCHEDULE_, inFunc_Booting_Flow_TMS_DCC_Schedule},
	{_EDC_BOOTING_POWER_ON_PASSWORD_, inFunc_Booting_Flow_Enter_PowerOn_Password},
	{_EDC_BOOTING_CLEAR_AP_DUMP_, inFunc_Booting_Flow_Clear_AP_Dump},
	{_EDC_BOOTING_PROCESS_CRADLE_, inFunc_Booting_Flow_Process_Cradle},
	{_EDC_BOOTING_POWER_MANAGEMENT_, inFunc_Booting_Flow_PowerManagement}, /* 目前沒在用 */
	{_EDC_BOOTING_REPRINT_POWEROFF_, inFunc_Booting_Flow_Reprint_Poweroff},

	{_EDC_BOOTING_UPDATE_PARAMETER_POWER_ON_, inFunc_Booting_Flow_Update_Parameter}, /* 目前有在用 20190619 [SAM] */


	{_EDC_BOOTING_UPDATE_SUCCESS_REPORT_, inFunc_Booting_Flow_Update_Success_Report},
	{_EDC_BOOTING_TMS_DOWNLOAD_, inFunc_Booting_Flow_TMS_Download},
	{_EDC_BOOTING_CHECK_BATCH_AMT_DATA_, inFunc_Booting_Flow_TMS_Download},

	/* 彥均加的TMS 需全部加入 20190108 [SAM] START*/
	{_FUNCTION_NEXSYS_TMS_SELECT_FLOW_, inEDCTMS_MenuForUserSetTmsParameterFlow},
	{_FUNCTION_NEXSYS_TMS_CONNECT_, inEDCTMS_FUNC_ConnectToServer},
	{_FUNCTION_NEXSYS_TMS_SEND_RECEIVE_FLOW_, inEDCTMS_SendAndReceiveFlow},
	{_FUNCTION_NEXSYS_TMS_RESULT_HANDLE_, inEDCTMS_ANALYSE_ResultHandle},
	/* 彥均加的TMS 需全部加入 20190108 [SAM]  END*/

	/* [新增預授權完成] 為了預先授權完成要輸入年月日，新增功能 2022/5/12 [SAM] */
	{_FUNCTION_GET_ORIG_TRANSDATE_, inFunc_InputYearDateTime},

	/* [新增電票悠遊卡功能]   新增會使用的功能 [SAM] 2022/6/8 下午 6:00 */
	{_TICKET_GET_VOID_TOP_UP_AMOUNT_FROM_BATCH_, inNCCC_Ticket_Get_Void_Top_Up_Amount_From_Batch},
	{_FUNCTION_CHECK_TERM_STATUS_CMAS_, inCMAS_CheckTermStatus},
	{_CMAS_DECIDE_TRANS_TYPE_, inCMAS_Decide_Trans_Type},
	{_CMAS_API_FLOW_, inCMAS_APIFlow},
	{_CMAS_UPDATE_INV_, inCMAS_Func_UpdateTermInvNum},
	{_CMAS_UPDATE_BATCH_, inBATCH_FuncUpdateTxnRecord_Ticket_By_Sqlite},
	{_CMAS_GET_PARM_, inCMAS_Get_ParamValue},
	{_FUNCTION_USER_CHOICE_CMAS_, inCMAS_Decide_Trans_Type},
	{_CMAS_SETTLE_FLOW_, inCMAS_Settle_Flow},
	{_CMAS_SET_UPDATE_BATCH_FLAG_, inCMAS_Set_Update_Batch_Flag},
	{_CMAS_UPDATE_ACCUM_, inACCUM_Update_CMAS_Flow},
	{_CMAS_CHECK_TRANS_ENABLE_, inCMAS_Func_Check_Transaction_Function_Flow},
	/* [新增電票悠遊卡功能]  END */
	
	/* [新增SVC功能]  [SAM]  START*/
	{_SVC_TMS_DOWNLOAD_, inSVC_Tms_Download},
	{_SVC_AUTO_LOAD_, inSVC_AutoReload},
	{_SVC_COMPARE_POINT_, inSVC_ComparePoint},
	{_FUNCTION_GET_SVC_CARD_FIELDS_CTLS_, inSVC_GetSvcCardFieldsFunction_CTLS},
	{_SVC_UPDATE_ACCUM_, inSVC_ACCUM_Update_Flow},
	{_SVC_ISO_FUNCTION_BUILD_AND_SEND_PACKET_, inSVC_ISO_BuildAndSendPacket},
	/* [新增SVC功能]  [SAM]  END*/
        
        /* 20230201 Miyano Costco 相關功能 Start */
        {_COSTCO_SCANPAY_, inCostco_TSP_ProcessScanPay},
        /* 20230201 Miyano Costco 相關功能 End */
        
};

int inFUNC_TABLE_Test(void) {
	inDISP_LogPrintf("inFUNC_TABLE_Test()");
	return (VS_SUCCESS);
}
