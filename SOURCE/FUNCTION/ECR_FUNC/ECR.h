/* 
 * File:   ECR.h
 * Author: user
 *
 * Created on 2017年6月1日, 下午 5:33
 */
#ifndef __ECR_H__
#define __ECR_H__


#define _ECR_RETRYTIMES_				3
#define _ECR_RECEIVE_TIMER_			_TIMER_NEXSYS_2_	/* 1 */
#define _ECR_RECEIVE_REQUEST_TIMEOUT_	10
#define _ECR_GET_CARD_TIMEOUT_		30
#define _ECR_RECEIVE_ACK_TIMEOUT_		2
#define _ECR_BUFF_SIZE_				5120 + 1

/* 8N1_Standard 長度 */
#define _ECR_8N1_Standard_Data_Size_			400

/* 20230314 Miyano add 8N1 Costco GasStation ECR收最大長度 */
#define _ECR_COSTCO_GASSTATION_MAX_SIZES_	1024    /* 封包最大數 */

/* 交易別 */
#define _ECR_8N1_SALE_				"01"
#define _ECR_8N1_VOID_				"30"
#define _ECR_8N1_REFUND_				"02"
#define _ECR_8N1_OFFLINE_				"03"
#define _ECR_8N1_INSTALLMENT_			"04"
#define _ECR_8N1_REDEEM_				"05"
#define _ECR_8N1_INSTALLMENT_REFUND_		"06"
#define _ECR_8N1_REDEEM_REFUND_			"07"
#define _ECR_8N1_INSTALLMENT_ADJUST_		"97"
#define _ECR_8N1_REDEEM_ADJUST_			"96"
#define _ECR_8N1_PREAUTH_			"19"
#define _ECR_8N1_PREAUTH_CANCEL_		"21"
#define _ECR_8N1_PREAUTH_COMPLETE_		"20"
#define _ECR_8N1_PREAUTH_COMPLETE_CANCEL_	"22"
#define _ECR_8N1_TIP_					"28"
#define _ECR_8N1_SETTLEMENT_			"50"
#define _ECR_8N1_START_CARD_NO_INQUIRY_	"60"
#define _ECR_8N1_END_CARD_NO_INQUIRY_		"70"
#define _ECR_8N1_REPRINT_RECEIPT_			"91"
#define _ECR_8N1_MENU_REVIEW_DETAIL_		"92"
#define _ECR_8N1_MENU_REVIEW_TOTAL_		"93"
#define _ECR_8N1_MENU_REPORT_DETAIL_		"94"
#define _ECR_8N1_MENU_REPORT_TOTAL_		"95"
#define _ECR_8N1_EDC_REBOOT_				"99"
#define _ECR_8N1_AWARD_REDEEM_			"40"
#define _ECR_8N1_VOID_AWARD_REDEEM_		"41"
#define _ECR_8N1_ESVC_TOP_UP_				"65"	/* ESVC加值(現金加值) */
#define _ECR_8N1_ESVC_BALANCE_INQUIRY_		"66"	/* ESVC餘額查詢 */
#define _ECR_8N1_ESVC_VOID_TOP_UP_			"67"	/* ESVC加值取消(現金加值) */
#define _ECR_8N1_HG_VOID_					"30"	/* 快樂購取消 */
#define _ECR_8N1_HG_REWARD_SALE_			"81"	/* 快樂購紅利積點(一般交易) */
#define _ECR_8N1_HG_REWARD_INSTALLMENT_		"82"	/* 快樂購紅利積點 + 信用卡分期付款 */
#define _ECR_8N1_HG_REWARD_REDEMPTION_		"83"	/* 快樂購紅利積點 + 信用卡紅利扣抵 */
#define _ECR_8N1_HG_ONLINE_REDEEMPTION_		"84"	/* 快樂購點數扣抵 */
#define _ECR_8N1_HG_POINT_CERTAIN_		"85"	/* 快樂購加價購 */
#define _ECR_8N1_HG_FULL_REDEEMPTION_		"86"	/* 快樂購點數兌換 */
#define _ECR_8N1_HG_REDEEM_REFUND_		"87"	/* 快樂購扣抵退貨 */
#define _ECR_8N1_HG_REWARD_REFUND_		"88"	/* 快樂購回饋退貨 */
#define _ECR_8N1_HG_POINT_INQUIRY_		"89"	/* 快樂購點數查詢 */

#define _ECR_8N1_SALE_NO_				1	/* 一般交易	/SmartPay消費扣款	*/
#define _ECR_8N1_VOID_NO_			30	/* 取消		/SmartPay消費扣款沖正	*/
#define _ECR_8N1_REFUND_NO_			2	/* 退貨		/SmartPay消費扣款退貨	*/
#define _ECR_8N1_OFFLINE_NO_			3	/* 交易補登 */
#define _ECR_8N1_INSTALLMENT_NO_		4	/* 分期付款 */
#define _ECR_8N1_REDEEM_NO_			5	/* 紅利扣抵 */
#define _ECR_8N1_INSTALLMENT_REFUND_NO_	6	/* 分期退貨 */
#define _ECR_8N1_REDEEM_REFUND_NO_		7	/* 紅利退貨 */
#define _ECR_8N1_INSTALLMENT_ADJUST_NO_	97	/* 分期調帳 */
#define _ECR_8N1_REDEEM_ADJUST_NO_		96	/* 紅利調帳 */
#define _ECR_8N1_PREAUTH_NO_				19	/* 預先授權 */
#define _ECR_8N1_PREAUTH_CANCEL_NO_		21	/* 預先授權取消(保留未來銀聯卡交易使用) */
#define _ECR_8N1_PREAUTH_COMPLETE_NO_		20	/* 預先授權完成 */
#define _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_	22	/* 預先授權完成取消(保留未來銀聯卡交易使用) */
#define _ECR_8N1_TIP_NO_					28	/* 小費交易 */
#define _ECR_8N1_SETTLEMENT_NO_			50	/* 結帳 */
#define _ECR_8N1_START_CARD_NO_INQUIRY_NO_	60	/* 啟動卡號查詢 */
#define _ECR_8N1_END_CARD_NO_INQUIRY_NO_	70	/* 結束卡號查詢 */
#define _ECR_8N1_REPRINT_RECEIPT_NO_		91	/* 重印簽單 */
#define _ECR_8N1_MENU_REVIEW_DETAIL_NO_	92	/* 交易明細查詢 */
#define _ECR_8N1_MENU_REVIEW_TOTAL_NO_		93	/* 交易總額查詢 */
#define _ECR_8N1_MENU_REPORT_DETAIL_NO_	94	/* 交易明細列印 */
#define _ECR_8N1_MENU_REPORT_TOTAL_NO_	95	/* 交易總額列印 */
#define _ECR_8N1_EDC_REBOOT_NO_			99	/* EDC重新開機 */
#define _ECR_8N1_AWARD_REDEEM_NO_		40	/* 優惠兌換 */
#define _ECR_8N1_VOID_AWARD_REDEEM_NO_	41	/* 取消優惠兌換 */
#define _ECR_8N1_ESVC_TOP_UP_NO_			65	/* ESVC加值(現金加值) */
#define _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_	66	/* ESVC餘額查詢 */
#define _ECR_8N1_ESVC_VOID_TOP_UP_NO_		67	/* ESVC加值取消(現金加值) */
#define _ECR_8N1_HG_VOID_NO_				30	/* 快樂購取消(同一般取消) */
#define _ECR_8N1_HG_REWARD_SALE_NO_		81	/* 快樂購紅利積點(一般交易) */
#define _ECR_8N1_HG_REWARD_INSTALLMENT_NO_	82	/* 快樂購紅利積點 + 信用卡分期付款 */
#define _ECR_8N1_HG_REWARD_REDEMPTION_NO_	83	/* 快樂購紅利積點 + 信用卡紅利扣抵 */
#define _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_	84	/* 快樂購點數扣抵 */
#define _ECR_8N1_HG_POINT_CERTAIN_NO_		85	/* 快樂購加價購 */
#define _ECR_8N1_HG_FULL_REDEEMPTION_NO_	86	/* 快樂購點數兌換 */
#define _ECR_8N1_HG_REDEEM_REFUND_NO_		87	/* 快樂購扣抵退貨 */
#define _ECR_8N1_HG_REWARD_REFUND_NO_		88	/* 快樂購回饋退貨 */
#define _ECR_8N1_HG_POINT_INQUIRY_NO_		89	/* 快樂購點數查詢 */

/* ECR_NCCC_HOSTID */
#define	_ECR_8N1_NCCC_HOSTID_NCCC_			"03"	/* U CARD、VISA、MASTER、JCB、CUP、SMART PAY*/
#define	_ECR_8N1_NCCC_HOSTID_DCC_			"04"	/* VISA、MASTER之外幣交易 */
#define	_ECR_8N1_NCCC_HOSTID_DINERS_			"00"	/* DINERS */
#define	_ECR_8N1_NCCC_HOSTID_LOYALTY_REDEEM_		"05"	/* 優惠兌換主機(ASM) */
#define	_ECR_8N1_NCCC_HOSTID_HG_			"13"	/* HappyGo 規格沒寫，照Code抄 */
#define	_ECR_8N1_NCCC_HOSTID_ESVC_			"06"	/* 悠遊卡、一卡通、愛金卡及有錢卡 */

/* ECR_NCCC_CARDTYPE */
#define	_ECR_8N1_NCCC_CARDTYPE_UCARD_			"01"	/* U CARD*/
#define	_ECR_8N1_NCCC_CARDTYPE_VISA_			"02"	/* VISA */
#define	_ECR_8N1_NCCC_CARDTYPE_MASTERCARD_		"03"	/* MASTERCARD */
#define	_ECR_8N1_NCCC_CARDTYPE_JCB_			"04"	/* JCB */
#define	_ECR_8N1_NCCC_CARDTYPE_AMEX_			"05"	/* AMEX */
#define	_ECR_8N1_NCCC_CARDTYPE_CUP_			"06"	/* CUP */
#define	_ECR_8N1_NCCC_CARDTYPE_DINERS_			"07"	/* DINERS */
#define	_ECR_8N1_NCCC_CARDTYPE_SMARTPAY_		"08"	/* SMART PAY */
#define	_ECR_8N1_NCCC_CARDTYPE_ECC_			"11"	/* ECC(悠遊卡) */
#define	_ECR_8N1_NCCC_CARDTYPE_IPASS_			"12"	/* iPASS(一卡通) */
#define	_ECR_8N1_NCCC_CARDTYPE_ICASH_			"13"	/* iCash(愛金卡) */
#define	_ECR_8N1_NCCC_CARDTYPE_HAPPYCASH_		"14"	/* HappyCash(有錢卡) */

/* 7E1_Standard 長度 */
#define _ECR_7E1_Standard_Data_Size_			144

/* 交易別 */
#define _ECR_7E1_SALE_				"01"
#define _ECR_7E1_REFUND_			"02"
#define _ECR_7E1_OFFLINE_			"03"
#define _ECR_7E1_PREAUTH_			"19"
#define _ECR_7E1_INSTALLMENT_			"04"
#define _ECR_7E1_REDEEM_			"05"
#define _ECR_7E1_INSTALLMENT_REFUND_		"22"
#define _ECR_7E1_REDEEM_REFUND_			"32"
#define _ECR_7E1_VOID_				"30"
#define _ECR_7E1_SETTLEMENT_			"50"
#define _ECR_7E1_START_CARD_NO_INQUIRY_		"60"
#define _ECR_7E1_END_CARD_NO_INQUIRY_		"70"
#define _ECR_7E1_INSTALLMENT_ADJUST_		"97"
#define _ECR_7E1_REDEEM_ADJUST_			"96"
#define _ECR_7E1_HG_POINT_INQUIRY_		"65"	/* 點數查詢 */
#define _ECR_7E1_HG_REWARD_SALE_		"81"	/* 紅利積點 */
#define _ECR_7E1_HG_REWARD_INSTALLMENT_		"84"	/* 紅利積點-分期 */
#define _ECR_7E1_HG_REWARD_REDEMPTION_		"85"	/* 紅利積點-紅利 */
#define _ECR_7E1_HG_ONLINE_REDEEMPTION_		"86"	/* 點數扣抵 */
#define _ECR_7E1_HG_POINT_CERTAIN_		"87"	/* 加價購 */
#define _ECR_7E1_HG_FULL_REDEEMPTION_		"88"	/* 點數扣抵 */
#define _ECR_7E1_HG_REDEEM_REFUND_		"91"	/* 扣抵退貨 */
#define _ECR_7E1_HG_REWARD_REFUND_		"92"	/* 回饋退貨 */
#define _ECR_7E1_CASH_SALE_			"141"	/* 預借現金 */

#define _ECR_7E1_SALE_NO_			1	/* 一般交易	/SmartPay消費扣款	*/
#define _ECR_7E1_REFUND_NO_			2	/* 退貨		/SmartPay消費扣款退貨	*/
#define _ECR_7E1_OFFLINE_NO_			3	/* 交易補登 */
#define _ECR_7E1_PREAUTH_NO_			19	/* 預先授權 */
#define _ECR_7E1_INSTALLMENT_NO_		4	/* 分期付款 */
#define _ECR_7E1_REDEEM_NO_			5	/* 紅利扣抵 */
#define _ECR_7E1_FUBON_INST_REFUND_NO_		6	/* 富邦 分期退貨 20191119 [SAM] */
#define _ECR_7E1_FUBON_REDEEM_REFUND_NO_		7	/* 富邦 紅利退貨 20191119 [SAM] */

#define _ECR_7E1_INSTALLMENT_REFUND_NO_		22	/* 分期退貨 */
#define _ECR_7E1_REDEEM_REFUND_NO_		32	/* 紅利退貨 */
#define _ECR_7E1_VOID_NO_			30	/* 取消		/SmartPay消費扣款沖正	*/
#define _ECR_7E1_SETTLEMENT_NO_			50	/* 結帳 */
#define _ECR_7E1_START_CARD_NO_INQUIRY_NO_	60	/* 啟動卡號查詢 */
#define _ECR_7E1_END_CARD_NO_INQUIRY_NO_	70	/* 結束卡號查詢 */
#define _ECR_7E1_INSTALLMENT_ADJUST_NO_		97	/* 分期調帳 */
#define _ECR_7E1_REDEEM_ADJUST_NO_		96	/* 紅利調帳 */
#define _ECR_7E1_HG_POINT_INQUIRY_NO_		65	/* 點數查詢 */
#define _ECR_7E1_HG_REWARD_SALE_NO_		81	/* 紅利積點 */
#define _ECR_7E1_HG_REWARD_INSTALLMENT_NO_	84	/* 紅利積點-分期 */
#define _ECR_7E1_HG_REWARD_REDEMPTION_NO_	85	/* 紅利積點-紅利 */
#define _ECR_7E1_HG_ONLINE_REDEEMPTION_NO_	86	/* 點數扣抵 */
#define _ECR_7E1_HG_POINT_CERTAIN_NO_		87	/* 加價購 */
#define _ECR_7E1_HG_FULL_REDEEMPTION_NO_	88	/* 點數兌換 */
#define _ECR_7E1_HG_REDEEM_REFUND_NO_		91	/* 扣抵退貨 */
#define _ECR_7E1_HG_REWARD_REFUND_NO_		92	/* 回饋退貨 */
#define _ECR_7E1_CASH_SALE_NO_			141	/* 預借現金 */

/* ECR_NCCC_HOSTID */
#define	_ECR_7E1_NCCC_HOSTID_NCCC_			"03"	/* U CARD、VISA、MASTER、JCB、CUP、SMART PAY*/
#define	_ECR_7E1_NCCC_HOSTID_DCC_			"04"	/* VISA、MASTER之外幣交易 */
#define	_ECR_7E1_NCCC_HOSTID_DINERS_			"00"	/* DINERS */

#define _ECR_LOAD_TMK_FROM_520_Standard_Data_Size_	489

/* For ECR Transaction Response Code */
/* 錯誤碼版本00 */
#define _ECR_RESPONSE_CODE_NOT_SET_ERROR_		0	/* 沒設定ECR錯誤碼 */
#define _ECR_RESPONSE_CODE_ERROR_				1	/* 交易失敗 / 主機拒絕或卡片拒絕 */
#define _ECR_RESPONSE_CODE_CALLBANK_			2	/* 請連絡銀行 */
#define _ECR_RESPONSE_CODE_TIMEOUT_			3	/* 交易逾時 */
#define _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_	4	/* 操作錯誤 */
#define _ECR_RESPONSE_CODE_COMM_ERROR_		5	/* 通訊失敗 */
#define _ECR_RESPONSE_CODE_USER_TERMINATE_ERROR_	6	/* 使用者終止交易 */
#define _ECR_RESPONSE_CODE_READER_CARD_ERROR_	7	/* 讀卡失敗 */
#define _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_	8
#define _ECR_RESPONSE_CODE_BYPASS_				9
#define _ECR_RESPONSE_CODE_NOT_MEMBER_CARD_		10
#define _ECR_RESPONSE_CODE_VOID_CHECK_ORG_AMT_ERROR_	11
#define _ECR_RESPONSE_CODE_CTLS_HOST_CANCEL_		12
#define _ECR_RESPONSE_CODE_CTLS_TWO_TAP_			13
#define _ECR_RESPONSE_CODE_SETTLEMENT_ERROR_		14
#define _ECR_RESPONSE_CODE_PIN_BYPASS_			15
#define _ECR_RESPONSE_CODE_AMOUNT_ERROR_		16      /* SKM - 金額檢核錯誤 */
#define _ECR_RESPONSE_CODE_OTHER_ERROR_                     19
#define _ECR_RESPONSE_CODE_SMARTPAY_ERROR_		20	/* 20151014浩瑋新增 *//* 假error，只是標明是 Smartpay */
#define _ECR_RESPONSE_CODE_CTLS_ERROR_			21	

#define _ECR_RESPONSE_CODE_NODATA_                                  22 /* 新增外接感應設備回應碼 20190816[SAM]*/
#define _ECR_RESPONSE_CODE_EXCESSIVE_TRANS_AMT_        23 /* 交易金額超過限額 2020/1/30 下午 2:37 [SAM] */
#define _ECR_RESPONSE_CODE_PLESE_SETTLE_                          24 /* 請先結帳的訊息 2020/1/30 下午 5:10 [SAM] */
#define _ECR_RESPONSE_CODE_NEAR_MAX_SETTLE_CNT_	 25 /* 請先結帳的訊息 2020/1/30 下午 5:10 [SAM] */

#define _ECR_RESPONSE_CODE_ECHO_TEST_SUCCESS_             26 /* ECHO TEST 使用  2020/12/22 [SAM] */
#define _ECR_RESPONSE_CODE_CARD_TYPE_NOT_MACH_      27  /* 卡片型態不同 2020/12/22 [SAM] */
#define _ECR_RESPONSE_CODE_CAR_NUM_NOT_MACH_        28  /* 車號不相同 2020/12/22 [SAM] */
#define _ECR_RESPONSE_CODE_FALLBACK_                                29  /* FALL BACK 訊息 2020/12/22 [SAM] */
#define _ECR_RESPONSE_CODE_TRANS_NOT_FOUND_              30  /* 找不到交易  2020/12/22 [SAM] */
#define _ECR_RESPONSE_CODE_TRANS_AUTHCOMP_IS_DONE_              31  /*  已做過預先授權 2020/12/22 [SAM] */
#define _ECR_RESPONSE_CODE_TRANS_IS_VOID_              32  /* 交易已取消 2020/12/22 [SAM] */
#define _ECR_RESPONSE_CODE_USE_CHIP_CARD_              33  /*  2020/12/22 [SAM] */
#define _ECR_RESPONSE_CODE_CELL_PHONE_TWO_TAP_   34  /*  2020/12/22 [SAM] */

#define _ECR_RESPONSE_CODE_ESVC_RE_TAP_CARD_		35 /* [新增電票悠遊卡功能] 請重新感應卡片 [SAM] 2022/6/8 下午 5:51 */

#define _ECR_RESPONSE_CODE_NODATA			36

#define _ECR_RESPONSE_CODE_SUCCESS_				99	/* 成功 */



/* 交易別 */
#define _ECR_8N1_TSB_KIOSK_SALE_MULTI_			"01"
#define _ECR_8N1_TSB_KIOSK_SALE_SINGLE_			"11"
#define _ECR_8N1_TSB_KIOSK_REFUND_			"02"	/* 目前不開放 */
#define _ECR_8N1_TSB_KIOSK_OFFLINE_			"03"	/* 目前不開放 */
#define _ECR_8N1_TSB_KIOSK_INSTALLMENT_		"04"
#define _ECR_8N1_TSB_KIOSK_REDEEM_			"05"
#define _ECR_8N1_TSB_KIOSK_VOID_				"30"
#define _ECR_8N1_TSB_KIOSK_SETTLEMENT_			"50"
#define _ECR_8N1_TSB_KIOSK_INQUIRE_TRANSACTION_STAUS_	"62"
#define _ECR_8N1_TSB_KIOSK_REPRINT_RECEIPT_		"63"
#define _ECR_8N1_TSB_KIOSK_SALE_FALLBACK_		"91"
#define _ECR_8N1_TSB_KIOSK_INSTALLMENT_FALLBACK_	"94"
#define _ECR_8N1_TSB_KIOSK_REDEEM_FALLBACK_		"95"


#define _ECR_8N1_TSB_KIOSK_SALE_MULTI_NO_			1	/* 一般交易(多合一)		*/
#define _ECR_8N1_TSB_KIOSK_SALE_SINGLE_NO_			11	/* 一般交易(僅只有一般交易)	*/
#define _ECR_8N1_TSB_KIOSK_REFUND_NO_				2	/* 退貨		目前不開放	*/
#define _ECR_8N1_TSB_KIOSK_OFFLINE_NO_				3	/* 交易補登	目前不開放	*/
#define _ECR_8N1_TSB_KIOSK_INSTALLMENT_NO_			4	/* 分期付款			*/
#define _ECR_8N1_TSB_KIOSK_REDEEM_NO_				5	/* 紅利扣抵			*/
#define _ECR_8N1_TSB_KIOSK_VOID_NO_					30	/* 取消				*/
#define _ECR_8N1_TSB_KIOSK_SETTLEMENT_NO_			50	/* 結帳				*/
#define _ECR_8N1_TSB_KIOSK_INQUIRE_TRANSACTION_STAUS_NO_	62	/* 詢問當筆交易狀態		*/
#define _ECR_8N1_TSB_KIOSK_REPRINT_RECEIPT_NO_			63	/* 重印簽單			*/
#define _ECR_8N1_TSB_KIOSK_SALE_FALLBACK_NO_			91	/* 銷售(Fall Back)		*/
#define _ECR_8N1_TSB_KIOSK_INSTALLMENT_FALLBACK_NO_	94	/* 分期付款(Fall Back)		*/
#define _ECR_8N1_TSB_KIOSK_REDEEM_FALLBACK_NO_		95	/* 紅利抵用(Fall Back)		*/

/* ECR_NCCC_HOSTID */
#define	_ECR_8N1_TSB_KIOSK_HOSTID_TSB_				"03"	/* U CARD、VISA、MASTER、JCB、CUP */
#define	_ECR_8N1_TSB_KIOSK_HOSTID_INST_				"04"	/* 分期 */
#define	_ECR_8N1_TSB_KIOSK_HOSTID_REDEEM_			"05"	/* 紅利 */
#define	_ECR_8N1_TSB_KIOSK_HOSTID_SMARTPAY_			"06"	/* Samrtpay */
#define	_ECR_8N1_TSB_KIOSK_HOSTID_DCC_				"07"	/* VISA、MASTER之外幣交易 */
#define	_ECR_8N1_TSB_KIOSK_HOSTID_DINERS_			"00"	/* DINERS */
#define	_ECR_8N1_TSB_KIOSK_HOSTID_AMEX_				"01"	/* AMEX */

/* ECR_NCCC_CARDTYPE */
#define	_ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_VISA_			"04"	/* VISA */
#define	_ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_MASTERCARD_	"05"	/* MASTERCARD */
#define	_ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_JCB_			"03"	/* JCB */
#define	_ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_SMARTPAY_		"09"	/* SMART PAY */
#define	_ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_UCARD_		"08"	/* U CARD*/
#define	_ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_AMEX_		"06"	/* AMEX */
#define	_ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_DINERS_		"07"	/* DINERS */
#define	_ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_CUP_			"01"	/* CUP */


/* FBON ECR_HOSTID */
#define	_ECR_7E1_FUBON_HOSTID_			"03"	/* U CARD、VISA、MASTER、JCB、CUP */
#define	_ECR_7E1_FUBON_HOSTID_DINERS_	"00"	/* DINERS */
#define	_ECR_7E1_FUBON_HOSTID_AMEX_	"01"	/* AMEX */


/* 7E1_VIESHOW 長度 */
#define _ECR_7E1_VIESHOW_ReturnData_Size_			185

/* 7E1_VIESHOW 交易類別編號(數字) */
#define _ECR_7E1_VIESHOW_SALE_NO_			1	/* 一般交易	 */
#define _ECR_7E1_VIESHOW_VOID_NO_			2	/* 取消 */
#define _ECR_7E1_VIESHOW_REFUND_NO_                     3	/* 退貨 */
#define _ECR_7E1_VIESHOW_SETTLEMENT_NO_	4	/* 結帳 */
#define _ECR_7E1_VIESHOW_PICKUP_CARD_NO_	31	/* 目前是解卡，但指令叫PICUP CARD */
#define _ECR_7E1_VIESHOW_GET_BAND_CARD_NO_	220	/* 更新要判斷的卡號 */

/* 7E1_VIESHOW 交易類別編號(字串) */
#define _ECR_7E1_VIESHOW_SALE_			"01"	/* 一般交易 */
#define _ECR_7E1_VIESHOW_VOID_			"02"  /* 取消 */
#define _ECR_7E1_VIESHOW_REFUND_			"03"	/* 退貨 */
#define _ECR_7E1_VIESHOW_SETTLEMENT_		"04"	/* 結帳 */
#define _ECR_7E1_VIESHOW_PICKUP_CARD_		"31"	/* 目前是解卡，但指令叫PICUP CARD */
#define _ECR_7E1_VIESHOW_GET_BAND_CARD_	"DC"	/* 更新要判斷的卡號 */


/* 7E1_VIESHOW 主機編號 */
#define	_ECR_7E1_VIESHOW_HOSTID_		"01"	/* U CARD、VISA、MASTER、JCB、CUP */



/* 新增車麻吉的參數 2021/12/29 下午 4:27 [SAM] */
/* 7E1_MOCHI_PAY_ 交易類別編號(數字) */
#define _ECR_7E1_MOCHI_PAY_SALE_NO_                         1	/* 一般交易	 */
#define _ECR_7E1_MOCHI_PAY_REFUND_NO_                   2	/* 退貨 */
#define _ECR_7E1_MOCHI_PAY_PRE_AUTH_NO_                3	/* 預先授權 */
#define _ECR_7E1_MOCHI_PAY_PRE_AUTH_COMPLETED_NO_                4	/* 預先授權 */
#define _ECR_7E1_MOCHI_PAY_SETTLEMENT_NO_           31	/* 結帳 */
#define _ECR_7E1_MOCHI_PAY_EZCARD_SALE_NO_         41	/* 悠遊卡扣值 */
#define _ECR_7E1_MOCHI_PAY_EZCARD_INQUIRY_NO_   44	/* 悠遊卡查詢 */
#define _ECR_7E1_MOCHI_PAY_EZCARD_CASH_NO_         45	/* 悠遊卡現金加值 */

#define _ECR_7E1_MOCHI_PAY_IPASS_SALE_NO_         51            /* 悠遊卡扣值 */
#define _ECR_7E1_MOCHI_PAY_IPASS_INQUIRY_NO_   54           /* 悠遊卡查詢 */
#define _ECR_7E1_MOCHI_PAY_IPASS_CASH_NO_         55            /* 悠遊卡現金加值 */

#define _ECR_7E1_MOCHI_PAY_START_CARD_NO_INQUIRY_NO_            60      /*  啟動卡號查詢  */
#define _ECR_7E1_MOCHI_PAY_SECOND_SALE_NO_                                  62      /* 啟動第二段交易	 */
#define _ECR_7E1_MOCHI_PAY_END_CARD_NO_INQUIRY_NO_            70      /*  結束卡號查詢  */


#define _ECR_7E1_MOCHI_PAY_ECHO_TEST_NO_                                80      /* 連線測試	 */
#define _ECR_7E1_MOCHI_PAY_TERMINATE_TRANS_NO_                   82      /* 終止交易 */



/* 7E1_MOCHI_PAY_ 交易類別編號(字串) */
#define _ECR_7E1_MOCHI_PAY_SALE_                                                "1"	/* 一般交易	 */
#define _ECR_7E1_MOCHI_PAY_REFUND_                                          "2"	/* 退貨 */
#define _ECR_7E1_MOCHI_PAY_PRE_AUTH_                                       "3"	/* 預先授權 */
#define _ECR_7E1_MOCHI_PAY_PRE_AUTH_COMPLETED_                "4"	/* 預先授權 */
#define _ECR_7E1_MOCHI_PAY_SETTLEMENT_                                  "31"	/* 結帳 */
#define _ECR_7E1_MOCHI_PAY_EZCARD_SALE_                                 "41"	/* 悠遊卡扣值 */
#define _ECR_7E1_MOCHI_PAY_EZCARD_INQUIRY_                          "44"	/* 悠遊卡查詢 */
#define _ECR_7E1_MOCHI_PAY_EZCARD_CASH_                                 "45"	/* 悠遊卡現金加值 */

#define _ECR_7E1_MOCHI_PAY_IPASS_SALE_                                      "51"            /* 悠遊卡扣值 */
#define _ECR_7E1_MOCHI_PAY_IPASS_INQUIRY_                               "54"           /* 悠遊卡查詢 */
#define _ECR_7E1_MOCHI_PAY_IPASS_CASH_                                      "55"            /* 悠遊卡現金加值 */

#define _ECR_7E1_MOCHI_PAY_START_CARD_INQUIRY_                  "60"      /*  啟動卡號查詢  */
#define _ECR_7E1_MOCHI_PAY_SECOND_SALE_                                 "62"      /* 啟動第二段交易	 */
#define _ECR_7E1_MOCHI_PAY_END_CARD_NO_INQUIRY_             "70"      /*  結束卡號查詢  */


#define _ECR_7E1_MOCHI_PAY_ECHO_TEST_                                       "80"      /* 連線測試	 */
#define _ECR_7E1_MOCHI_PAY_TERMINATE_TRANS_                         "82"      /* 終止交易 */

/* 7E1_MOCHI_PAY_ 主機編號 */
#define _ECR_7E1_MOCHI_PAY_HOSTID_		"01"	/* U CARD、VISA、MASTER、JCB、CUP */

#define _ECR_7E1_MOCHI_PAY_RETURN_DATA_SIZE_        600



/* 7E1_ESUN SelfServe 長度 */
#define _ECR_7E1_SELF_SERVE_GAS_DATA_SIZE_              400

#define _ECR_7E1_SELF_SERVE_GAS_SALE_NO_                            1	/* 一般交易 Auth Complete	 */
#define _ECR_7E1_SELF_SERVE_GAS_GET_PAN_NO_                    60	/* 取得卡號功能 */
#define _ECR_7E1_SELF_SERVE_GAS_SETTLEMENT_NO_              50	/* 結帳 */
#define _ECR_7E1_SELF_SERVE_GAS_END_GET_PAN_NO_           70	/* 結束卡號查詢 */
#define _ECR_7E1_SELF_SERVE_GAS_ECHO_TEST_NO_                80	/* 連線測試功能 */

#define _ECR_7E1_SELF_SERVE_GAS_SALE_                        "01"	/* 預先授權完成 */
#define _ECR_7E1_SELF_SERVE_GAS_GET_PAN_                "60"	/* 進行預先授權並取得卡號功能 */
#define _ECR_7E1_SELF_SERVE_GAS_SETTLEMENT_          "50"	/* 結帳 */
#define _ECR_7E1_SELF_SERVE_GAS_END_GET_PAN_       "70"	/* 結束預先授權卡號查詢並需沖銷預先授權交易 */
#define _ECR_7E1_SELF_SERVE_GAS_ECHO_TEST_            "80" /* 連線測試功能 */

/* 7E1_ESUN SelfServe ECR_HOSTID */
#define	_ECR_7E1_SELF_SERVE_HOSTID_			"00"	/* U CARD、VISA、MASTER、JCB、CUP */
#define	_ECR_7E1_SELF_SERVE_HOSTID_DINERS_	"02"	/* DINERS */
#define	_ECR_7E1_SELF_SERVE_HOSTID_AMEX_           "03"	/* AMEX */

/* 7E1_ESUN SelfServe Card Type */
#define	_ECR_7E1_SELF_SERVE_CARDTYPE_UCARD_			"1"	/* U CARD*/
#define	_ECR_7E1_SELF_SERVE_CARDTYPE_VISA_			"3"	/* VISA */
#define	_ECR_7E1_SELF_SERVE_CARDTYPE_MASTERCARD_		"4"	/* MASTERCARD */
#define	_ECR_7E1_SELF_SERVE_CARDTYPE_JCB_                              "2"	/* JCB */
#define	_ECR_7E1_SELF_SERVE_CARDTYPE_AMEX_			"5"	/* AMEX */
#define	_ECR_7E1_SELF_SERVE_CARDTYPE_CUP_			"7"	/* CUP */
#define	_ECR_7E1_SELF_SERVE_CARDTYPE_DINERS_			"6"	/* DINERS */

/* 20230331 Miyano add Costco_GasStation Card Type */
#define _ECR_Costco_GasStation_CardType_Cobrand_		"01"	/* 聯名卡 */
#define _ECR_Costco_GasStation_CardType_Credit_			"02"	/* 信用卡 */
#define _ECR_Costco_GasStation_CardType_SVC_			"11"	/* 儲值卡 */
#define _ECR_Costco_GasStation_CardType_Debit_			"31"	/* 金融卡 */
#define _ECR_Costco_GasStation_CardType_Supervisor_		"50"	/* 主管卡 */
#define _ECR_Costco_GasStation_CardType_Staff_			"51"	/* 員工卡 */
#define _ECR_Costco_GasStation_CardType_Member_			"52"	/* 會員卡 */
#define _ECR_Costco_GasStation_CardType_CounterChargeAuth1_	"53"	/* 櫃檯簽帳授權卡 */
#define _ECR_Costco_GasStation_CardType_Charge_			"54"	/* 車隊卡(簽帳) */
#define _ECR_Costco_GasStation_CardType_CounterChargeAuth2_	"55"	/* 櫃檯簽帳授權卡 */

/* 20230424 Miyano 好市多加油站 ECR發動的交易別 */
#define _Costco_ReadCard_		1	/* 讀卡(信用卡) */
#define _Costco_PreAuth_		2	/* 預先授權 */
#define _Costco_AuthComplete_		3	/* 授權完成 */
#define _Costco_Settle_			4	/* 結帳 */
#define _Costco_Reboot_			5	/* 重新開機 */
#define _Costco_ScanPay_		6	/* 掃碼 */

#define _Costco_szCommand_ReadCard_		"01"	/* 讀卡(信用卡) */
#define _Costco_szCommand_PreAuth_		"02"	/* 預先授權 */
#define _Costco_szCommand_AuthComplete_ 	"03"	/* 授權完成 */
#define _Costco_szCommand_Settle_               "04"	/* 結帳 */
#define _Costco_szCommand_Reboot_	        "05"	/* 重新開機 */
#define _Costco_szCommand_ScanPay_		"06"	/* 掃碼 */

#define _Costco_szCommand_PreAuth_Complete_Err  "-1"    /* 預先授權或授權完成 Error */






typedef struct
{
	int		inVersion;
	int		inTimeout;
	unsigned char	uszComPort;		/* Verifone用handle紀錄，Castle用Port紀錄 */
	unsigned char	uszSettingOK;		/* 用來標注是否已設定好RS232 */
}ECR_SETTING;

typedef struct
{
	int		inCode;
	int		inErrorType;
	long		lnAmt;
	long		lnTipAmt;
	char		szTransIndicator[2 + 1];
	char		szTransType[2 + 1];
	char		szStartTransType[2 + 1];
	char		szCUPIndicator[2 + 1];
	char		szOrgCUPindicator[2 + 1];
	unsigned char	uszIsResponce;			/* 用來確認是否回過ECR，避免送兩次回覆(例如:Call Bank是Send Error卻繼續跑流程) */
	unsigned char	uszECRResponsePayitem;	
}ECR_TRANSACTION_DATA;

typedef struct
{
	int		inErrorType;
	char		szTransType[2 + 1];
	char		szDataLen[4 + 1];
	char		szCRC[4 + 1];
	char		szSQNo[1];
	unsigned char	uszIsResponce;			/* 用來確認是否回過ECR，避免送兩次回覆(例如:Call Bank是Send Error卻繼續跑流程) */
}ECR_TRANSACTION_DATA_TSB;

typedef struct
{
	char		szCommand[10 + 1];	/* 指令 */
	char		szPumpNo[1+1];		/* 加油泵號碼 */
        char            szPumpStatus[1+1];      /* 加油泵狀態 */
	char            szEDCStatus[1+1];       /* EDC狀態 */
        char            szResponse[1+1];        /* 執行結果 */
	char		szCardType[2+1];	/* 卡片種類，信用卡/會員/員工... */
        char            szCardKind[20+1];       /* 卡片類型，Visa/Master/JCB... */
	char		szPan[20+1];		/* 卡號 */
	char		szCardExpDate[4+1];	/* 卡片有效期 */
        char            szPresetAmount[8+1];    /* 暫時，預設金額 */
        char            szTransactionType[2+1]; /* 暫時，未知用途的交易別 */
	char		szApproveNo[6+1];	/* 授權碼 Approve No. */
	char		szInvoice[6+1];		/* 調閱編號 */
	char		szBatch[6+1];		/* 批次編號 */
	char		szCity[21+1];		/* 城市名 */
	char		szWayPassCard[1+1];	/* 過卡方式 */
	char		szTxRespCode[2+1];	/* 暫時，未知東西，用在回復PreAuth */
	char		szEInvoiceCarrier[50+1];/* 電子發票載具 */
	char		szEDCMode[1+1];		/* 要切換的模式(讀卡、結帳、掃碼...) */
	char		szSVCType[4+1];		/* SVC要執行的相關功能 */
	char		szTID[8+1];		/* TID */
	char		szMID[15+1];		/* MID */
        char            szDate[8+1];            /* Date */
        char            szTime[6+1];            /* Time */
        
        int             inTransTotalCount;      /* 結帳計算交易筆數 */
        int             inTransTotalAmount;     /* 結帳計算交易金額 */
        
        int             inTransType;            /* 解析ECR後，卡機要執行的動作 */
        int             inFlag_P01;             /* 要收P01:1，不收P01:0 */

	float		fUnitPrice;		/* UnitPrice */
	float		fQunity;		/* 數量 */
	
	long		lnAmount;		/* 金額 */
	long		lnDiscount;		/* 折扣金額 */
	long		lnPresetAmount;		/* 預授凍結額度 */
	
	unsigned char	usz95_TVR[6 + 1];		/* Tag95_TVR */
	unsigned char	usz9B_TSI[2 + 1];		/* usz9B_TSI */
	unsigned char	usz82_AIP[2 + 1];		/* usz82_AIP */
	unsigned char	usz9F27_CID[2 + 1];		/* usz9F27_CID */
	unsigned char	usz9F26_ApplCryptogram[8 + 1];	/* usz9F26_ApplCryptogram */
        
        unsigned char   uszErrString[100+1];    /* Error String */
	unsigned char	uszArgement1[100+1];	/* 其他陳述1 */
	unsigned char	uszArgement2[100+1];	/* 其他陳述2 */
	unsigned char	uszArgement3[100+1];	/* 其他陳述3 */
	unsigned char	uszArgement4[100+1];	/* 其他陳述4 */

}ECR_TRANSACTION_DATA_COSTCO_GAS;	/* 20230325 Miyano add for Costco GasStation */

typedef struct
{
	unsigned char	uszNotSendAck;			/* 若On起來，代表接收完對方Request不用送ACK給對方 */
}ECR_OPTIONAL_SETTING;

typedef struct
{
	ECR_SETTING			srSetting;	/* initial完不會動的 */
	ECR_TRANSACTION_DATA		srTransData;	/* 每次接收都要清空 */
	ECR_TRANSACTION_DATA_TSB	srTransDataTSB;	/* 台新專用結構 */
	ECR_TRANSACTION_DATA_COSTCO_GAS	srTransDataGas;	/* Costco加油站 */
	ECR_OPTIONAL_SETTING		srOptionalSetting;
}ECR_TABLE;

/* 20230420 Miyano add */
typedef struct
{
        char		szCommand[3 + 1];	/* 指令 */
	char		szPumpNo[1+1];		/* 加油泵號碼 */
        char            szPumpStatus[1+1];      /* 加油泵狀態 */
	char            szEDCStatus[1+1];       /* EDC狀態 */
        
        unsigned char   uszErrorCode[100+1];    /* EDC錯誤碼，外部 */
	
	int             inFlag_NextState;             /* 要第二段:1，不要第二段:0 */

}ECR_GASSTATION_GLOBAL_TABLE;


/* 用來決定要跑那一個ECR版本 */
typedef struct
{
	int (*inEcrInitial)(ECR_TABLE *srECROb);
	int (*inEcrRece)(TRANSACTION_OBJECT *, ECR_TABLE *srECROb);
	int (*inEcrSend)(TRANSACTION_OBJECT *, ECR_TABLE *srECROb);
	int (*inEcrSendError)(TRANSACTION_OBJECT *, ECR_TABLE *srECROb);
	int (*inEcrEnd)(ECR_TABLE *);
}ECR_TRANS_TABLE;

int inECR_ECROB_GetTransType(char* szTempTransType);
int inECR_ECROB_GetErrorCode(void);
int inECR_ECROB_SetErrorCode(int inErrorType);

int inECR_Initial(void);
int inECR_FlushRxBuffer(void);
int inECR_Send_Check(void);
int inECR_Receive_Check(unsigned short* usLen);
int inECR_Receive_Transaction(TRANSACTION_OBJECT *pobTran);
int inECR_Receive_Second_Transaction(TRANSACTION_OBJECT *pobTran);
int inECR_Load_TMK(TRANSACTION_OBJECT *pobTran);
int inECR_Send_Transaction_Result(TRANSACTION_OBJECT *pobTran);
int inECR_Send_Inquiry_Result(TRANSACTION_OBJECT *pobTran);
int inECR_SendError(TRANSACTION_OBJECT * pobTran, int inErrorType);
int inECR_DeInitial(void);
int inECR_Print_Receive_ISODeBug(char *szDataBuffer, unsigned short usReceiveBufferSize, int inDataSize);
int inECR_Print_Send_ISODeBug(char *szDataBuffer, unsigned short usReceiveBufferSize, int inDataSize);
int inECR_CardNoTruncateDecision(TRANSACTION_OBJECT * pobTran);
int inECR_CardNoTruncateDecision_HG(TRANSACTION_OBJECT * pobTran);
int inECR_Customer_Flow(TRANSACTION_OBJECT* pobTran);
int inECR_Customer_Flow2(TRANSACTION_OBJECT* pobTran);

/* 整合function */
int inECR_Receive(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer, int inDataSize);
int inECR_ReceiveWithoutCheckLenth(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer, int inDataSize);
int inECR_Not_Check_Sizes_Receive(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer, int inDataSize);
int inECR_Send(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer, int inDataSize);
int inECR_Data_Receive(unsigned char *uszReceBuff, unsigned short *usReceSize);
int inECR_Send_ACKorNAK(ECR_TABLE * srECROb, int inAckorNak);

int inECR_8N1_Standard_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer);
int inECR_8N1_Standard_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);
int inECR_8N1_Standard_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);

int inECR_7E1_Standard_Unpack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Standard_Pack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Standard_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);

int inECR_LOAD_TMK_FROM_520_Standard_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer);
int inECR_LOAD_TMK_FROM_520_Standard_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);

int inECR_8N1_Customer_107_Bumper_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer );
int inECR_8N1_Customer_107_Bumper_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);
int inECR_8N1_Customer_107_Bumper_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);

int inECR_8N1_FOBUN_Standard_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);
int inECR_8N1_FOBUN_Standard_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);
int inECR_8N1_FOBUN_Standard_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);

int inECR_SecondReceiveTest(TRANSACTION_OBJECT *pobTran);

/* 國泰威秀用 */
//int inECR_7E1_VIESHOW_Unpack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
//int inECR_7E1_VIESHOW_Pack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
//int inECR_7E1_VIESHOW_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);

/* 20230314 Miyano add Costco 加油站用 */
int inECR_Costco_GasStation_Receive(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer);
int inECR_Costco_GasStation_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer);
int inECR_Costco_GasStation_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer);

int inECR_Costco_GasStation_Unpack_Poll(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer);
int inECR_Costco_GasStation_Unpack_AuthComplete(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer);
int inECR_Costco_GasStation_Unpack_ChangeMode(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer);
int inECR_Costco_GasStation_Unpack_ConfigSetting(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer);
int inECR_Costco_GasStation_Unpack_Inquery(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer);
int inECR_Costco_GasStation_Unpack_Inquery_CostcoPay(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer);
int inECR_Costco_GasStation_Unpack_ShowData(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer);
int inECR_Costco_GasStation_Unpack_SVC(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer);

int inECR_Costco_Get_DataSegment(char *szResult, char *szDataBuffer);
int inECR_Costco_CutData(int inCut, char *szDataBuffer);

int inECR_Costco_GasStation_Pack_EDCStatus(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer);
int inECR_Costco_GasStation_Pack_PreAuth(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer);
int inECR_Costco_GasStation_Pack_ReadCard(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer);
int inECR_Costco_GasStation_Pack_AuthComplete(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer);
int inECR_Costco_GasStation_Pack_Settle(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer);
int inECR_Costco_GasStation_Pack_PreAuth_Complete_ERR(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer);
int inECR_Costco_GasStation_Pack_ScanPay(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer);

int inECR_Costco_GasStation_Unpack_P01(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer);

int inECR_Costco_GasStation_Pack2(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer);

#endif
