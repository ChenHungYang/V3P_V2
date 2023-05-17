/*
 * File:   TransType.h
 * Author: user
 *
 * Created on 2015年7月27日, 下午 5:05
 */

typedef enum
{
	_TRANS_TYPE_NULL_ = 0x00,
	_SALE_,
	_VOID_,
	_REFUND_,
	_SETTLE_,
	_TIP_,
	_SALE_OFFLINE_,
	_ADJUST_,
	_PRE_AUTH_,
	_PRE_COMP_,
	_REVERSAL_,//10
	_ADVICE_,
	_CLS_BATCH_,
	_CLS_SETTLE_,
	_BATCH_UPLOAD_,
	_INST_SALE_,
	_INST_REFUND_,
	_INST_ADJUST_,
	_REDEEM_SALE_,
	_REDEEM_REFUND_,
	_REDEEM_ADJUST_,//20
	_CUP_SALE_,
	_CUP_VOID_,
	_CUP_REFUND_,
	_CUP_INST_SALE_,
	_CUP_INST_REFUND_,
	_CUP_REDEEM_SALE_,
	_CUP_REDEEM_REFUND_,
	_CUP_PRE_AUTH_,
	_CUP_PRE_AUTH_VOID_,
	_CUP_PRE_COMP_,//30
	_CUP_PRE_COMP_VOID_,
	_FISC_SALE_,
	_FISC_VOID_,
	_FISC_REFUND_,
	_FISC_ADVICE_,
	_FISC_BALANCE_INQ_,
	_FISC_FUND_TRANSFER_,
	_FISC_REFUND_TRANSFER_,
	_MAIL_ORDER_,
	_CUP_MAIL_ORDER_,//40
	_CUP_MAIL_ORDER_REFUND_,
	_CASH_ADVANCE_,
	_FORCE_CASH_ADVANCE_,
	_LOYALTY_REDEEM_,
	_VOID_LOYALTY_REDEEM_,
	_LOYALTY_REDEEM_REFUND_,
	_TC_UPLOAD_,
	_CUP_LOGON_,
	_CHANGE_TMK_,
	_CHANGE_DUKPT_,//50
	_CHANGE_WK_,
	_POWER_ON_,
	_ESC_UPLOAD_,
	_INQUIRY_ISSUER_BANK_,
	/* DCC 相關 */
	_DCC_RATE_,
	_DCC_UPDATE_INF_,
	_DCC_BIN_,
	_DCC_UPDATE_CLOSE_,
	_DCC_EX_RATE_,
	_SEND_ESC_ADVICE_,//60
	/* MFES規格 F63 用 */
	_INST_DETAIL_,
	_REDEEM_DETAIL_,
	_CALL_BANK_,

	/* HG使用 */
	/* HG分類用 START */
	_HG_PAY_CREDIT_,			/* 信用卡 */
	_HG_PAY_CASH_,				/* 現金 */
	_HG_PAY_GIFT_PAPER_,			/* 禮券 */
	_HG_PAY_CREDIT_INSIDE_,			/* HGI */
	_HG_PAY_CUP_,				/* 銀聯卡 */
	_HG_REWARD_,				/* 紅利積點 */
	_HG_ONLINE_REDEEM_,/*70*/	/* 點數扣抵 */
	_HG_POINT_CERTAIN_,			/* 加價購 */
	/* HG分類用 END */
	_HG_INQUIRY_,				/* 點數查詢 */
	_HG_FULL_REDEMPTION_,			/* 點數兌換 */

	_HG_REWARD_CASH_,			/* 紅利積點 + 現金 */
	_HG_REWARD_GIFT_PAPER_,			/* 紅利積點 + 禮券 */
	_HG_ONLINE_REDEEM_CASH_,		/* 點數扣抵 + 現金 */
	_HG_ONLINE_REDEEM_GIFT_PAPER_,		/* 點數扣抵 + 禮券 */
	_HG_POINT_CERTAIN_CASH_,		/* 加價購 + 現金 */
	_HG_POINT_CERTAIN_GIFT_PAPER_,		/* 加價購 + 禮券	 */

	_HG_REWARD_CREDIT_,/*80*/		/* 紅利積點 + 信用卡 */
	_HG_REWARD_CREDIT_INSIDE_,		/* 紅利積點 + HGI */
	_HG_REWARD_CUP_,			/* 紅利積點 + 銀聯卡 */

	_HG_ONLINE_REDEEM_CREDIT_,		/* 點數扣抵 + 信用卡 */
	_HG_ONLINE_REDEEM_CREDIT_INSIDE_,	/* 點數扣抵 + HGI */
	_HG_ONLINE_REDEEM_CUP_,			/* 點數扣抵 + 銀聯卡 */

	_HG_POINT_CERTAIN_CREDIT_,		/* 加價購 + 信用卡 */
	_HG_POINT_CERTAIN_CREDIT_INSIDE_,	/* 加價購 + HGI */
	_HG_POINT_CERTAIN_CUP_,			/* 加價購 + 銀聯卡 */

	_HG_REWARD_REDEMPTION_CREDIT_,          /* 紅利積點 + 紅利扣抵 + 信用卡 */
	_HG_REWARD_REDEMPTION_CREDIT_INSIDE_,/*90*/   /* 紅利積點 + 紅利扣抵 + HGI */

	_HG_REWARD_INSTALLMENT_CREDIT_,         /* 紅利積點 + 分期付款 + 信用卡 */
	_HG_REWARD_INSTALLMENT_CREDIT_INSIDE_,  /* 紅利積點 + 分期付款 + HGI */

	_HG_REWARD_REFUND_,			/* 回饋退貨 */
	_HG_REDEEM_REFUND_,			/* 扣抵退貨 */

	/* For 票證使用 */
	_TICKET_DEDUCT_,			/* 票證扣款 */
	_TICKET_REFUND_,			/* 票證退貨 */
	_TICKET_INQUIRY_,			/* 票證查詢 */
	_TICKET_TOP_UP_,			/* 票證加值 */
	_TICKET_VOID_TOP_UP_,			/* 票證取消加值 */
	_TICKET_VOID_DEDUCT_,/*100*/			/* 票證取消扣款 */

	_TICKET_IPASS_DEDUCT_,			/* 一卡通扣款 */
	_TICKET_IPASS_REFUND_,			/* 一卡通退貨 */
	_TICKET_IPASS_INQUIRY_,			/* 一卡通查詢 */
	_TICKET_IPASS_TOP_UP_,			/* 一卡通加值 */
	_TICKET_IPASS_AUTO_TOP_UP_,		/* 一卡通自動加值 */
	_TICKET_IPASS_VOID_TOP_UP_,		/* 一卡通取消加值 */
	_TICKET_IPASS_VOID_DEDUCT_,		/* 一卡通取消扣款 */
	_TICKET_IPASS_QUERY_,
	_TICKET_IPASS_LOGON_,
	_TICKET_IPASS_REGISTER_,//110
	_TICKET_IPASS_ADD_RETURN_, /* [新增電票悠遊卡功能]  新增 ID 留著 IPASS 用 [SAM] 2022/6/14 上午 11:58 */

	_TICKET_EASYCARD_DEDUCT_,		/* 悠遊卡扣款 */
	_TICKET_EASYCARD_REFUND_,		/* 悠遊卡退貨 */
	_TICKET_EASYCARD_INQUIRY_,		/* 悠遊卡查詢 */
	_TICKET_EASYCARD_TOP_UP_,		/* 悠遊卡加值 */
	_TICKET_EASYCARD_AUTO_TOP_UP_,		/* 悠遊卡自動加值 */
	_TICKET_EASYCARD_VOID_TOP_UP_,		/* 悠遊卡取消加值 */
	_TICKET_EASYCARD_VOID_DEDUCT_,		/* 悠遊卡取消扣款 */
	_TICKET_EASYCARD_LOGON_,
	_TICKET_EASYCARD_REGISTER_,//120
	_TICKET_EASYCARD_REG_REV_,

	_CUP_CHANGE_KEY_,
	_CUP_CHANG_TPK_,
	_CUP_SAVE_REVERSAL_,
	_ISR_UPLOAD_,
	_VOID_AUTH_,/* 原PREAUTH交易改送Reversal格式上傳   新增沖消AUTH交易 2020/12/24 下午 3:05 [SAM] */
	_CONF_CHANGE_KEY_,
	
	_SVC_TMS_,
	_SVC_TMS_FINISH_,	/* 目前看520沒有進行交易 2022/12/29 下午 4:00 */
	_SVC_REDEEM_,//130
	_SVC_RELOAD_,
	_SVC_ACTIVECARD_,
	_SVC_VOID_,		/* 目前看520沒有進行交易 2022/12/29 下午 4:00 */
	_SVC_REFUND_,
	_SVC_SETTLE_,
	_SVC_CLS_BATCH_,
	_SVC_REVERSAL_,
	_SVC_BATCHUPLOAD_,
	_SVC_INQUIRY_,
	_SVC_ACTIVECARD_AUTORELOAD_,//140
	_SVC_ACTIVECARD_NOT_AUTORELOAD_,
	_SVC_BACKCARD_,
                
        _COSTCO_TSP_    /* Costco TSP, 20230110 Miyano add */ 
			
} TRANS_TYPE;
