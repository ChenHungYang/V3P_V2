typedef enum
{
	_OPERATION_NULL_ = 0x00,
	_OPERATION_SALE_,
	_OPERATION_SALE_ICC_,
	_OPERATION_SALE_CTLS_,
	_OPERATION_SALE_CTLS_IDLE_,
	_OPERATION_FISC_SALE_,
	_OPERATION_FISC_SALE_CTLS_,
	_OPERATION_INST_SALE_CTLS_,
	_OPERATION_INST_REFUND_CTLS_,
	_OPERATION_INST_ADJUST_,
	_OPERATION_INST_ADJUST_CTLS_,//10
	_OPERATION_REDEEM_SALE_CTLS_,
	_OPERATION_REDEEM_REFUND_CTLS_,
	_OPERATION_REDEEM_ADJUST_,
	_OPERATION_REDEEM_ADJUST_CTLS_,
	_OPERATION_VOID_,
	_OPERATION_REFUND_,
	_OPERATION_REFUND_AMOUNT_FIRST_,
	_OPERATION_REFUND_CTLS_,
	_OPERATION_REFUND_AMOUNT_FIRST_CUP_,
	_OPERATION_REFUND_CTLS_CUP_,//20
	_OPERATION_FISC_REFUND_,
	_OPERATION_FISC_REFUND_CTLS_,
	_OPERATION_SETTLE_,
	_OPERATION_TIP_,
	_OPERATION_SALE_OFFLINE_,
	_OPERATION_SALE_OFFLINE_CTLS_,
	_OPERATION_ADJUST_,
	_OPERATION_PRE_AUTH_,
	_OPERATION_PRE_AUTH_CTLS_,
	_OPERATION_PRE_COMP_,//30
	_OPERATION_PRE_COMP_CTLS_,
	_OPERATION_MAIL_ORDER_,
	_OPERATION_TOTAL_REPORT_,
	_OPERATION_DETAIL_REPORT_,
	_OPERATION_DELETE_BATCH_,
	_OPERATION_TSAM_,
	_OPERATION_REVIEW_,
	_OPERATION_REVIEW_TOTAL_,
	_OPERATION_REVIEW_DETAIL_,
	_OPERATION_REPRINT_,//40
	_OPERATION_FUN7_PRINT_,
	_OPERATION_FUN7_DCC_PRINT_,
	_OPERATION_FUN8_PRINT_,
	_OPERATION_FUN4_PRINT_,
	_OPERATION_FUN5_TMS_DOWNLOAD_,
	_OPERATION_FUN5_TMS_TASK_REPORT_,
	_OPERATION_FUN5_DCC_DOWNLOAD_,
	_OPERATION_FUN6_TMS_DOWNLOAD_,
	_OPERATION_FUN6_TMS_TRACELOG_UP_,
	_OPERATION_FUN6_DCC_DOWNLOAD_,//50
	_OPERATION_FUN6_DCC_RATE_DOWNLOAD_,
	_OPERATION_FUN1_READERINIT_,
	_OPERATION_ECR_TRANSACTION_,
	_OPERATION_MULTIFUNC_SLAVE_,
	_OPERATION_ESC_IDLE_UPLOAD_,
	_OPERATION_DCC_SCHEDULE_,
	_OPERATION_TMS_DCC_SCHEDULE_,
	_OPERATION_TMS_SCHEDULE_INQUIRE_,
	_OPERATION_TMS_SCHEDULE_DOWNLOAD_,
	_OPERATION_TMS_PROCESS_EFFECTIVE_,//60
	_OPERATION_EDC_BOOTING_, /* 目前有在用 20190619 [SAM] */
	_OPERATION_POWER_SAVING_SLEEP_,
	_OPERATION_POWER_SAVING_STANDBY_,
	_OPERATION_EDC_LOAD_KEY_BOOTING_,
	_OPERATION_EDC_V3UL_BOOTING_,
	_OPERATION_EDC_REBOOT_,
	_OPERATION_LOYALTY_REDEEM_,
	_OPERATION_LOYALTY_REDEEM_CTLS_,
	_OPERATION_BARCODE_,
	_OPERATION_VOID_LOYALTY_REDEEM_,//70
	_OPERATION_LOYALTY_REDEEM_REFUND_,
					
	/* HAPPYGO */
	_OPERATION_HG_,
	_OPERATION_HG_REFUND_,
	_OPERATION_I_R_,
	_OPERATION_HG_INST_SALE_CTLS_,
	_OPERATION_HG_REDEEM_SALE_CTLS_,
	/* 票證 */
	_OPERATION_TICKET_DEDUCT_,
	_OPERATION_TICKET_REFUND_,
	_OPERATION_TICKET_INQUIRY_,
	_OPERATION_TICKET_TOP_UP_,//80
	_OPERATION_TICKET_VOID_TOP_UP_,
	_OPERATION_TICKET_SAM_REGISTER_,
	_OPERATION_LOAD_KEY_FROM_520_,
	_OPERATION_CUP_LOGON_,
	_OPERATION_TMS_DOWNLOAD_, /*  20190131 TMS [SAM] */
	_OPERATION_SDK_READ_CARD_, /* 國泰自助的SDK使用 2020/2/25 上午 11:08 [SAM]  */
         _OPERATION_SELFSERVE_AUTHCOM_, /* KIOSK 共月ID，OTP重新定義由存檔的PAUTH交易抓取資料進行AUTH_COMP交易. 2022/1/27 上午 11:40 [SAM] 補充說明  */
	// TODO: 這個功能看要不要做, 看起來像開機時在初始化, 但原本初始化已有功能
	_OPERATION_EDC_SDK_INITIAL_BOOTING_,/* [新增電票悠遊卡功能]  [SAM] */
	/* [新增電票悠遊卡功能]  新增CMASMenu時需要的執行步驟 [SAM] 2022/6/9 下午 4:37 */
	_OPERATION_EASYCARD_DEDUCT_, /* 悠遊卡購貨、退貨、現金加值、共用 */
	_OPERATION_EASYCARD_VOID_,
	_OPERATION_EASYCARD_NORMAL_, /* 悠遊卡餘額查詢*/   
	/* [新增電票悠遊卡功能]  新增CMASMenu時需要的執行步驟 [SAM] 2022/6/9 下午 4:37 END */
	_OPERATION_SVC_CARD_, 
			
} OPERATION;

typedef enum
{
	_TRT_NULL_ = 0x00,
	_TRT_SALE_,
	_TRT_SALE_ICC_,
	_TRT_SALE_CTLS_,
	_TRT_VOID_,
	_TRT_REFUND_,
	_TRT_REFUND_CTLS_,
	_TRT_SETTLE_,
	_TRT_TIP_,
	_TRT_SALE_OFFLINE_,
	_TRT_SALE_OFFLINE_CTLS_,	/* 10 */ 
	_TRT_ADJUST_,
	_TRT_PRE_AUTH_,
	_TRT_PRE_AUTH_ICC_,
	_TRT_PRE_AUTH_CTLS_,
	_TRT_PRE_COMP_,
	_TRT_PRE_COMP_CTLS_,
	_TRT_INST_SALE_,
	_TRT_INST_SALE_ICC_,
	_TRT_INST_SALE_CTLS_,
	_TRT_INST_REFUND_,		/* 20 */
	_TRT_INST_REFUND_CTLS_,
	_TRT_INST_ADJUST_,
	_TRT_INST_ADJUST_CTLS_,
	_TRT_REDEEM_SALE_,
	_TRT_REDEEM_SALE_ICC_,
	_TRT_REDEEM_SALE_CTLS_,
	_TRT_REDEEM_REFUND_,
	_TRT_REDEEM_REFUND_CTLS_,
	_TRT_REDEEM_ADJUST_,
	_TRT_REDEEM_ADJUST_CTLS_,/* 30 */
	_TRT_CUP_SALE_,
	_TRT_CUP_SALE_ICC_,
	_TRT_CUP_SALE_CTLS_,
	_TRT_CUP_REFUND_,
	_TRT_CUP_REFUND_CTLS_,
	_TRT_CUP_PRE_AUTH_,
	_TRT_CUP_PRE_AUTH_ICC_,
	_TRT_CUP_PRE_AUTH_CTLS_,
	_TRT_CUP_PRE_COMP_,
	_TRT_CUP_PRE_COMP_CTLS_, /* 40 */
	_TRT_CUP_VOID_,
	_TRT_CUP_PRE_AUTH_VOID_,
	_TRT_CUP_PRE_COMP_VOID_,
	_TRT_CUP_INST_SALE_,
	_TRT_CUP_INST_SALE_ICC_,
	_TRT_CUP_INST_SALE_CTLS_,
	_TRT_CUP_INST_REFUND_,
	_TRT_CUP_INST_REFUND_CTLS_,
	_TRT_CUP_REDEEM_SALE_,
	_TRT_CUP_REDEEM_SALE_ICC_, /* 50 */
	_TRT_CUP_REDEEM_SALE_CTLS_,
	_TRT_CUP_REDEEM_REFUND_,
	_TRT_CUP_REDEEM_REFUND_CTLS_,
	_TRT_CUP_LOGON_,
	_TRT_FISC_SALE_ICC_,
	_TRT_FISC_SALE_CTLS_,
	_TRT_FISC_VOID_,
	_TRT_FISC_REFUND_ICC_,
	_TRT_FISC_REFUND_CTLS_,
	_TRT_MAIL_ORDER_,		/* 60 */
	_TRT_MAIL_ORDER_REFUND_,
	_TRT_CUP_MAIL_ORDER_,
	_TRT_CUP_MAIL_ORDER_REFUND_,
	_TRT_CUP_MAIL_ORDER_REFUND_CTLS_,
	_TRT_CASH_ADVANCE_,
	_TRT_FORCE_CASH_ADVANCE_,
	_TRT_LOYALTY_REDEEM_,
	_TRT_VOID_LOYALTY_REDEEM_,
	_TRT_LOYALTY_REDEEM_REFUND_,
	_TRT_HG_INQUIRY_,		/* 70 */ 
	_TRT_HG_FULL_REDEMPTION_,
	_TRT_HG_POINT_CERTAIN_CASH_,
	_TRT_HG_POINT_CERTAIN_GIFT_PAPER_,
	_TRT_HG_ONLINE_REDEEM_CASH_,
	_TRT_HG_ONLINE_REDEEM_GIFT_PAPER_,
	_TRT_HG_REWARD_CASH_,
	_TRT_HG_REWARD_GIFT_PAPER_,
	_TRT_HG_REWARD_CREDIT_,
	_TRT_HG_REWARD_CREDIT_ICC_,
	_TRT_HG_REWARD_CREDIT_CTLS_,
	_TRT_HG_REWARD_CREDIT_INSIDE_,
	_TRT_HG_REWARD_CREDIT_INSIDE_ICC_,
	_TRT_HG_REWARD_CUP_,
	_TRT_HG_REWARD_CUP_ICC_,
	_TRT_HG_REWARD_REDEMPTION_,
	_TRT_HG_REWARD_REDEMPTION_ICC_,
	_TRT_HG_REWARD_REDEMPTION_CTLS_,
	_TRT_HG_REWARD_REDEMPTION_CREDIT_INSIDE_,
	_TRT_HG_REWARD_REDEMPTION_CREDIT_INSIDE_ICC_,
	_TRT_HG_REWARD_INSTALLMENT_,
	_TRT_HG_REWARD_INSTALLMENT_ICC_,
	_TRT_HG_REWARD_INSTALLMENT_CTLS_,
	_TRT_HG_REWARD_INSTALLMENT_CREDIT_INSIDE_,
	_TRT_HG_REWARD_INSTALLMENT_CREDIT_INSIDE_ICC_,
	_TRT_HG_ONLINE_REDEEM_CREDIT_,
	_TRT_HG_ONLINE_REDEEM_CREDIT_ICC_,
	_TRT_HG_ONLINE_REDEEM_CREDIT_CTLS_,
	_TRT_HG_ONLINE_REDEEM_CREDIT_INSIDE_,
	_TRT_HG_ONLINE_REDEEM_CREDIT_INSIDE_ICC_,
	_TRT_HG_ONLINE_REDEEM_CUP_,
	_TRT_HG_ONLINE_REDEEM_CUP_ICC_,
	_TRT_HG_POINT_CERTAIN_CREDIT_,
	_TRT_HG_POINT_CERTAIN_CREDIT_ICC_,
	_TRT_HG_POINT_CERTAIN_CREDIT_CTLS_,
	_TRT_HG_POINT_CERTAIN_CREDIT_INSIDE_,
	_TRT_HG_POINT_CERTAIN_CREDIT_INSIDE_ICC_,
	_TRT_HG_POINT_CERTAIN_CUP_,
	_TRT_HG_POINT_CERTAIN_CUP_ICC_,
	_TRT_HG_REWARD_REFUND_,
	_TRT_HG_REDEEM_REFUND_,
	_TRT_HG_VOID_,
	/* 票證 */
	_TRT_TICKET_IPASS_DEDUCT_,
	_TRT_TICKET_IPASS_INQUIRY_,
	_TRT_TICKET_IPASS_REFUND_,
	_TRT_TICKET_IPASS_TOP_UP_,
	_TRT_TICKET_IPASS_VOID_TOP_UP_,

	_TRT_TICKET_ECC_DEDUCT_,
	_TRT_TICKET_ECC_INQUIRY_,
	_TRT_TICKET_ECC_REFUND_,
	_TRT_TICKET_ECC_TOP_UP_,
	_TRT_TICKET_ECC_VOID_TOP_UP_,
	_TRT_TICKET_ECC_SETTLE_,
          _TRT_VOID_PAUTH_ ,/* 新增玉山自助沖消AUTH交易 2020/12/24 下午 3:05 [SAM] */
/* 新增玉山自助AUTH COMPLETE交易 2020/12/28 下午 3:29 [SAM] 
 * 補充說明，ID會共用在各家自助交易，會在各家TRT中覆寫對應的流程*/
         _TRT_SELFSERVE_AUTHCOM_ ,
	/* [新增電票悠遊卡功能] 新增 TRT FLOW  [SAM] 2022/6/8 下午 5:39 */
	_TRT_CMAS_DEDUCT_,
	_TRT_CMAS_ADD_VALUE_,
	_TRT_CMAS_REFUND_,
	_TRT_CMAS_VOID_DEDUCT_,
	_TRT_CMAS_VOID_ADD_VALUE_,
	_TRT_CMAS_QUERY_,
	_TRT_CMAS_SETTLE_,
	/* [新增電票悠遊卡功能] 新增 TRT FLOW  [SAM] 2022/6/8 下午 5:39  END */
	/* [新增SVC功能]  [SAM] START */
	_TRT_SVC_ACTIVECARD_,
	_TRT_SVC_INQUIRY_,
	_TRT_SVC_REDEEM_,
	_TRT_SVC_SETTLE_,
	_TRT_SVC_REFUND_,
	_TRT_SVC_BACKCARD_,
	_TRT_SVC_RELOAD_,
	/* [新增SVC功能]  [SAM] END */
			
	
} TRANSACTION;

typedef struct
{
        int             inOPTCode;
        int             *inOPTID;
} OPT_TABLE;

typedef struct
{
        int             inTRTCode;
        int             *inTRTID;
} TRT_TABLE;

#define _FLOW_NULL_          -1

int inFLOW_Test(TRANSACTION_OBJECT *pobTran);
int inFLOW_TRT_Test(TRANSACTION_OBJECT *pobTran);
int inFLOW_RunFunction(TRANSACTION_OBJECT *pobTran, int inFuncID);
int inFLOW_RunOperation(TRANSACTION_OBJECT *pobTran, int inOPTCode);
