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
#include "../EVENT/MenuMsg.h"
#include "../FUNCTION/FILE_FUNC/File.h"
#include "../FUNCTION/FILE_FUNC/FIleLogFunc.h"
#include "../FUNCTION/Function.h"
#include "Sqlite.h"
#include "../FUNCTION/HDPT.h"
#include "../FUNCTION/EDC_Para_Table_Func.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
//#include "../../NCCC/NCCCesc.h"

#include "Batch.h"

extern int	ginDebug;		/* Debug使用 extern */
SQLITE_RESULT	gsrResult = {};		/* For特殊情況使用，若一次要Get多筆Record，只Get一次，待使用完後在Call sqlite3_free_table() */

static int st_inSqlitHandleCnt = 0;


int	ginEnormousNowCnt = 0;
unsigned char	guszEnormousNoNeedResetBit = VS_FALSE;	/* 避免每次都從頭找起 */

/* [新增電票悠遊卡功能]  [SAM] */
/* 以下這兩個變數用在原先sqlite3_get_table的時機 */
sqlite3		*gsrDBConnection;	/* 建立到資料庫的connection */
sqlite3_stmt	*gsrSQLStat;

#ifdef _MODIFY_HDPT_FUNC_
char		gszTranDBPath[100 + 1] = {_FS_DATA_PATH_ _DATA_BASE_NAME_NEXSYS_TRANSACTION_};
char		gszParamDBPath[100 + 1] = {_FS_DATA_PATH_ _DATA_BASE_NAME_NEXSYS_PARAMETER_};
#endif

//static int st_inEmvSqlitHandleCnt = 0;

/*
	筆記:

	(1) var因為額外儲存地址，讀取時會先去抓資料，會比非var來的略慢
	效能：(nvarchar, varchar) < (nchar, char)
	(2) n因為1字2Byte，所以正常會花費2倍儲存空間
	儲存體：(nvarchar, nchar) >> (varchar, char)


	確認一定長度，且只會有英數字：char
	確認一定長度，且可能會用非英數以外的字元：nchar
	長度可變動，且只會有英數字：varchar
	長度可變動，且可能會用非英數以外的字元：nvarchar

 *	SQLite 的primary key會自動增長，若已有三筆Record，Delete 第二筆再insert新資料，則primaery key從4繼續往上加
 *	目前儲存型別皆改成BLOB，目的是要儲存0x00部份
 *	目前此份code有兩種查詢方式的程式碼並存，一種是sqlite_exec，第二種是sqlite_prepare and step 因為C語言的特性，建議盡量設計成prepare的形式
 */

/*	關於sqlite3_column_xxx(sqlite3_stmt*, int iCol);
 *	如果對該列使用了不同於該列本身類型適合的數據讀取方式，
 *	得到的數值將是轉換過得結果(所以不知道類型一律用Blob就對了)。*/
SQLITE_TAG_TABLE TABLE_BATCH_TAG[] =
{
	{"inTableID"			,"INTEGER"	,"PRIMARY KEY"	,""},	/* Table ID Primary key, sqlite table專用避免PRIMARY KEY重複 */
	{"inCode"				,"INTEGER"	,""		,""},	/* Trans Code */
	{"inOrgCode"			,"INTEGER"	,""		,""},	/* Original Trans Code  */
	{"inPrintOption"			,"INTEGER"	,""		,""},	/* Print Option Flag */
	{"inHDTIndex"			,"INTEGER"	,""		,""},	/* 紀錄HDTindex */
	{"inCDTIndex"			,"INTEGER"	,""		,""},	/* 紀錄CDTindex */
	{"inCPTIndex"			,"INTEGER"	,""		,""},	/* 紀錄CPTindex */
	{"inTxnResult"			,"INTEGER"	,""		,""},	/* 紀錄交易結果 */
	{"inChipStatus"			,"INTEGER"	,""		,""},	/* 0 NOT_USING_CHIP, 1 EMV_CARD, 2 EMV_EASY_ENTRY_CARD */
	{"inFiscIssuerIDLength"		,"INTEGER"	,""		,""},	/* 金融卡發卡單位代號長度 */
	{"inFiscCardCommentLength"	,"INTEGER"	,""		,""},	/* 金融卡備註欄長度 */
	{"inFiscAccountLength"		,"INTEGER"	,""		,""},	/* 金融卡帳號長度 */
	{"inFiscSTANLength"		,"INTEGER"	,""		,""},	/* 金融卡交易序號長度 */
	{"inESCTransactionCode"	,"INTEGER"	,""		,""},	/* ESC組ISO使用 重新上傳使用 Transaction Code沒辦法存在Batch */
	{"inESCUploadMode"		,"INTEGER"	,""		,""},	/* 標示支不支援ESC */
	{"inESCUploadStatus"		,"INTEGER"	,""		,""},	/* 標示ESC上傳狀態 */
	{"inSignStatus"			,"INTEGER"	,""		,""},	/* 簽名檔狀態(有 免簽 或 Bypass) ESC電文使用 */
	{"inHGCreditHostIndex"	,"INTEGER"	,""		,""},	/* 聯合_HAPPY_GO_信用卡主機 */
	{"inHGCode"			,"INTEGER"	,""		,""},	/* 聯合_HAPPY_GO_交易碼 */
	{"lnTxnAmount"			,"INTEGER"	,""		,""},	/* The transaction amount, such as a SALE */
	{"lnOrgTxnAmount"		,"INTEGER"	,""		,""},	/* The ORG transaction amount, such as a SALE */
	{"lnTipTxnAmount"		,"INTEGER"	,""		,""},	/* The transaction amount, such as a TIP */
	{"lnAdjustTxnAmount"		,"INTEGER"	,""		,""},	/* The transaction amount, such as a ADJUST */
	{"lnTotalTxnAmount"		,"INTEGER"	,""		,""},	/* The transaction amount, such as a TOTAL */
	{"lnInvNum"			,"INTEGER"	,""		,""},	/* 調閱編號  */
	{"lnOrgInvNum"			,"INTEGER"	,""		,""},	/* Original 調閱編號  */
	{"lnBatchNum"			,"INTEGER"	,""		,""},	/* Batch Number */
	{"lnOrgBatchNum"		,"INTEGER"	,""		,""},	/* Original Batch Number */
	{"lnSTANNum"			,"INTEGER"	,""		,""},	/* Stan Number */
	{"lnSTAN_Multi"			,"INTEGER"	,"" 	,""},	/* Stan Number 2 */
	{"lnOrgSTANNum"		,"INTEGER"	,""		,""},	/* Original Stan Number */
	{"lnInstallmentPeriod"		,"INTEGER"	,""		,""},	/* 分期付款_期數 */
	{"lnInstallmentDownPayment"	,"INTEGER"	,""		,""},	/* 分期付款_頭期款 */
	{"lnInstallmentPayment"		,"INTEGER"	,""		,""},	/* 分期付款_每期款 */
	{"lnInstallmentFormalityFee"	,"INTEGER"	,""		,""},	/* 分期付款_手續費 */
	{"lnRedemptionPoints"		,"INTEGER"	,""		,""},	/* 紅利扣抵_扣抵紅利點數 */
	{"lnRedemptionPointsBalance"	,"INTEGER"	,""		,""},	/* 紅利扣抵_剩餘紅利點數 */
	{"lnRedemptionPaidAmount"		,"INTEGER"	,""		,""},	/* 紅利扣抵_抵扣金額 */
	{"lnRedemptionPaidCreditAmount"	,"INTEGER"	,""		,""},	/* 紅利扣抵_支付金額 */
	{"lnHGTransactionType"	,"INTEGER"	,""		,""},	/* 聯合_HAPPY GO_交易類別 */
	{"lnHGPaymentType"		,"INTEGER"	,""		,""},	/* 聯合_HAPPY_GO_支付工具 */
	{"lnHGPaymentTeam"		,"INTEGER"	,""		,""},	/* 聯合_HAPPY_GO_支付工具_主機回_*/
	{"lnHGBalancePoint"		,"INTEGER"	,""		,""},	/* 聯合_HAPPY_GO_剩餘點數 */
	{"lnHGTransactionPoint"	,"INTEGER"	,""		,""},	/* 聯合_HAPPY_GO_交易點數  合計 */
	{"lnHGAmount"			,"INTEGER"	,""		,""},	/* 聯合_HAPPY_GO_扣抵後金額  (商品金額 = lnHGAmount + lnHGRedeemAmt) */
	{"lnHGRedeemAmount"	,"INTEGER"	,""		,""},	/* 聯合_HAPPY_GO_扣抵金額 */
	{"lnHGRefundLackPoint"	,"INTEGER"	,""		,""},	/* 聯合_HAPPY_GO_不足點數 */
	{"lnHGBatchIndex"		,"INTEGER"	,""		,""},	/* 聯合_HAPPY_GO_主機當下批次號碼 */
	{"lnHG_SPDH_OrgInvNum"	,"INTEGER"	,""		,""},	/* HAPPY_GO取消用INV */
	{"lnHGSTAN"			,"INTEGER"	,""		,""},	/* HAPPY_GO STAN */
	{"lnFubonGoodsID"		,"INTEGER"	,"" 	,""},	/* lnFubonGoodsID */
	{"szAuthCode"			,"BLOB"		,""		,""},	/* Auth Code */
	{"szMPASAuthCode"		,"BLOB"		,""		,""},	/* MPAS Auth Code */
	{"szRespCode"			,"BLOB"		,""		,""},	/* Response Code */
	{"szStoreID"			,"BLOB"		,""		,""},	/* StoreID */
	{"szCardLabel"			,"BLOB"		,""		,""},	/* 卡別  */
	{"szPAN"				,"BLOB"		,""		,""},	/* 卡號  */
	{"szDate"				,"BLOB"		,""		,""},	/* YYYYMMDD */
	{"szOrgDate"			,"BLOB"		,""		,""},	/* YYYYMMDD */
	{"szEscDate"			,"BLOB"		,""		,""},	/* YYYYMMDD */
	{"szTime"				,"BLOB"		,""		,""},	/* HHMMSS */
	{"szOrgTime"			,"BLOB"		,""		,""},	/* HHMMSS */
	{"szEscTime"			,"BLOB"		,""		,""},	/* HHMMSS */
	{"szCardTime"			,"BLOB"		,""		,""},	/* 晶片卡讀卡時間 , YYYYMMDDHHMMSS */
	{"szRefNo"				,"BLOB"		,""		,""},	/* 序號  */
	{"szExpDate"			,"BLOB"		,""		,""},	/* Expiration date */
	{"szServiceCode"			,"BLOB"		,""		,""},	/* Service code from track */
	{"szCardHolder"			,"BLOB"		,""		,""},	/* 持卡人名字 */
	{"szAMEX4DBC"			,"BLOB"		,""		,""},
	{"szFiscIssuerID"			,"BLOB"		,""		,""},	/* 發卡單位代號 */
	{"szFiscCardComment"		,"BLOB"		,""		,""},	/* 金融卡備註欄 */
	{"szFiscAccount"			,"BLOB"		,""		,""},	/* 金融卡帳號 */
	{"szFiscOutAccount"		,"BLOB"		,""		,""},	/* 金融卡轉出帳號 */
	{"szFiscSTAN"			,"BLOB"		,""		,""},	/* 金融卡交易序號 */
	{"szFiscTacLength"		,"BLOB"		,""		,""},	/* 金融卡Tac長度 */
	{"szFiscTac"				,"BLOB"		,""		,""},	/* 金融卡Tac */
	{"szFiscTCC"			,"BLOB"		,""		,""},	/* 端末設備查核碼 */
	{"szFiscMCC"			,"BLOB"		,""		,""},	/* 金融卡MCC */
	{"szFiscRRN"			,"BLOB"		,""		,""},	/* 金融卡調單編號 */
	{"szFiscRefundDate"		,"BLOB"		,""		,""},	/* 金融卡退貨原始交易日期(YYYYMMDD) */
	{"szFiscDateTime"		,"BLOB"		,""		,""},	/* 計算TAC(S2)的交易日期時間 */
	{"szFiscPayDevice"		,"BLOB"		,""		,""},	/* 金融卡付款裝置 1 = 手機 2 = 卡片 */
	{"szFiscMobileDevice"		,"BLOB"		,""		,""},	/* SE 類型，0x05：雲端卡片(Cloud-Based) */
	{"szFiscMobileNFType"		,"BLOB"		,""		,""},	/* 行動金融卡是否需輸入密碼 00不需要 01視情況 02一定要 */
	{"szFiscMobileNFSetting"	,"BLOB"		,""		,""},	/* 近端交易類型設定 0x00：Single Issuer Wallet 0x01：國內Third-Party Wallet 0x02~9：保留 0x0A：其他 */
	{"szInstallmentIndicator"	,"BLOB"		,""		,""},
	{"szRedeemIndicator"		,"BLOB"		,""		,""},
	{"szRedeemSignOfBalance"	,"BLOB"		,""		,""},
	{"szHGCardLabel"			,"BLOB"		,""		,""},	/* HAPPY_GO 卡別 */
	{"szHGPAN"			,"BLOB"		,""		,""},	/* HAPPY_GO Account number */
	{"szHGAuthCode"		,"BLOB"		,""		,""},	/* HAPPY_GO 授權碼 */
	{"szHGRefNo"			,"BLOB"		,""		,""},	/* HAPPY_GO Reference Number */
	{"szHGRespCode"		,"BLOB"		,""		,""},	/* HG Response Code */
	{"szCUP_TN"			,"BLOB"		,""		,""},	/* CUP Trace Number (TN) */
	{"szCUP_TD"			,"BLOB"		,""		,""},	/* CUP Transaction Date (MMDD) */
	{"szCUP_TT"			,"BLOB"		,""		,""},	/* CUP Transaction Time (hhmmss) */
	{"szCUP_RRN"			,"BLOB"		,""		,""},	/* CUP Retrieve Reference Number (CRRN) */
	{"szCUP_STD"			,"BLOB"		,""		,""},	/* CUP Settlement Date(MMDD) Of Host Response */
	{"szCUP_EMVAID"		,"BLOB"		,""		,""},	/* CUP晶片交易存AID帳單列印使用 */
	{"szTranAbbrev"			,"BLOB"		,""		,""},	/* Tran abbrev for reports */
	{"szIssueNumber"		,"BLOB"		,""		,""},
	{"szStore_DREAM_MALL"	,"BLOB"		,""		,""},	/* 存Dream_Mall Account Number And Member ID*/
	{"szDCC_FCNFR"			,"BLOB"		,""		,""},	/* Foreign Currency No. For Rate */
	{"szDCC_AC"			,"BLOB"		,""		,""},	/* Action Code */
	{"szDCC_FCN"			,"BLOB"		,""		,""},	/* Foreign Currency Number */
	{"szDCC_FCA"			,"BLOB"		,""		,""},	/* Foreign Currency Amount */
	{"szDCC_FCMU"			,"BLOB"		,""		,""},	/* Foreign Currency Minor Unit */
	{"szDCC_FCAC"			,"BLOB"		,""		,""},	/* Foreign currcncy Alphabetic Code */
	{"szDCC_ERMU"			,"BLOB"		,""		,""},	/* Exchange Rate Minor Unit */
	{"szDCC_ERV"			,"BLOB"		,""		,""},	/* Exchange Rate Value */
	{"szDCC_IRMU"			,"BLOB"		,""		,""},	/* Inverted Rate Minor Unit */
	{"szDCC_IRV"			,"BLOB"		,""		,""},	/* Inverted Rate Value */
	{"szDCC_IRDU"			,"BLOB"		,""		,""},	/* Inverted Rate Display Unit */
	{"szDCC_MPV"			,"BLOB"		,""		,""},	/* Markup Percentage Value */
	{"szDCC_MPDP"			,"BLOB"		,""		,""},	/* Markup Percentage Decimal Point */
	{"szDCC_CVCN"			,"BLOB"		,""		,""},	/* Commissino Value Currency Number */
	{"szDCC_CVCA"			,"BLOB"		,""		,""},	/* Commission Value Currency Amount */
	{"szDCC_CVCMU"			,"BLOB"		,""		,""},	/* Commission Value Currency Minor Unit */
	{"szDCC_TIPFCA"			,"BLOB"		,""		,""},	/* Tip Foreign Currency Amount */
	{"szDCC_OTD"			,"BLOB"		,""		,""},	/* Original Transaction Date & Time (MMDD) */
	{"szDCC_OTA"			,"BLOB"		,""		,""},	/* Original Transaction Amount */
	{"szProductCode"			,"BLOB"		,""		,""},	/* 產品代碼 */
	{"szAwardNum"			,"BLOB"		,""		,""},	/* 優惠個數 */
	{"szAwardSN"			,"BLOB"		,""		,""},	/* 優惠序號(Award S/N) TID(8Bytes)+YYYYMMDDhhmmss(16 Bytes)，共22Bytes */
	{"szTxnNo"				,"BLOB"		,""		,""},	/* 交易編號 */
	{"szMCP_BANKID"		,"BLOB"		,""		,""},	/* 行動支付標記 金融機構代碼 */
	{"szPayItemCode"		,"BLOB"		,""		,""},	/* 繳費項目代碼 */
	{"szTableTD_Data"		,"BLOB"		,""		,""},	/* Table TD的資料 */
	{"szDFSTraceNum"		,"BLOB"		,""		,""},	/* DFS交易系統追蹤號 */
	{"szEI_FLAG"			,"BLOB"		,""		,""},	/* 玉山自助收銀新增欄位 [SAM] */
	{"szEI_BankId"			,"BLOB"		,""		,""},	/* 玉山自助收銀新增欄位 [SAM] */	
	{"uszWAVESchemeID"		,"BLOB"		,""		,""},	/* WAVE 使用用於組電文 Field_22 */
	{"uszVOIDBit"			,"BLOB"		,""		,""},	/* 負向交易 */
	{"uszUpload1Bit"			,"BLOB"		,""		,""},	/* Offline交易使用 (原交易advice是否已上傳)*/
	{"uszUpload2Bit"			,"BLOB"		,""		,""},	/* Offline交易使用 (當前交易是否為advice)*/
	{"uszUpload3Bit"			,"BLOB"		,""		,""},	/* Offline交易使用 */
	{"uszReferralBit"			,"BLOB"		,""		,""},	/* ISO Response Code 【01】【02】使用 */
	{"uszOfflineBit"			,"BLOB"		,""		,""},	/* 離線交易 */
	{"uszManualBit"			,"BLOB"		,""		,""},	/* Manual Keyin */
	{"uszNoSignatureBit"		,"BLOB"		,""		,""},	/* 免簽名使用 (免簽名則為true)*/
	{"uszCUPTransBit"		,"BLOB"		,""		,""},	/* 是否為CUP */
	{"uszFiscTransBit"			,"BLOB"		,""		,""},	/* SmartPay交易，是否為金融卡 */
	{"uszContactlessBit"		,"BLOB"		,""		,""},	/* 是否為非接觸式 */
	{"uszEMVFallBackBit"		,"BLOB"		,""		,""},	/* 是否要啟動fallback */
	{"uszInstallmentBit"		,"BLOB"		,""		,""},	/* Installment */
	{"uszRedeemBit"			,"BLOB"		,""		,""},	/* Redemption */
	{"uszForceOnlineBit"		,"BLOB"		,""		,""},	/* 組電文使用 Field_25 Point of Service Condition Code */
	{"uszMail_OrderBit"		,"BLOB"		,""		,""},	/* 組電文使用 Field_25 Point of Service Condition Code */
	{"uszDCCTransBit"		,"BLOB"		,""		,""},	/* 是否為DCC交易 */
	{"uszNCCCDCCRateBit"		,"BLOB"		,""		,""},
	{"uszCVV2Bit"			,"BLOB"		,""		,""},
	{"uszRewardSuspendBit"	,"BLOB"		,""		,""},
	{"uszRewardL1Bit"		,"BLOB"		,""		,""},	/* 要印L1 */
	{"uszRewardL2Bit"		,"BLOB"		,""		,""},	/* 要印L2 */
	{"uszRewardL5Bit"		,"BLOB"		,""		,""},	/* 要印L5 */
	{"uszField24NPSBit"		,"BLOB"		,""		,""},
	{"uszVEPS_SignatureBit"	,"BLOB"		,""		,""},	/* VEPS 免簽名是否成立 */
	{"uszTCUploadBit"		,"BLOB"		,""		,""},	/* TCUpload是否已上傳 */
	{"uszFiscConfirmBit"		,"BLOB"		,""		,""},	/* SmartPay 0220 是否已上傳 */
	{"uszFiscVoidConfirmBit"	,"BLOB"		,""		,""},	/* SmartPay Void 0220 是否已上傳 */
	{"uszESCMerchantCopyBit"	,"BLOB"		,""		,""},	/* 補欄位 2020/1/10 下午 2:32 [SAM] */
	{"uszPinEnterBit"			,"BLOB"		,""		,""},	/* 此筆交易是否有鍵入密碼(只能確認原交易，若該筆之後的調整或取消不會將此Bit Off) */
	{"uszL2PrintADBit"		,"BLOB"		,""		,""},	/* L2是否印AD，因L2原交易取消要判斷，只好增加 */
	{"uszInstallment"			,"BLOB"		,""		,""},	/* HappyGo分期交易 */
	{"uszRedemption"		,"BLOB"		,""		,""},	/* HappyGo點數兌換 */
	{"uszHappyGoSingle"		,"BLOB"		,""		,""},	/* HappyGo交易 */
	{"uszHappyGoMulti"		,"BLOB"		,""		,""},	/* HappyGo混合交易 */
	{"uszCLSBatchBit"			,"BLOB"		,""		,""},	/* 是否已接續上傳 */
	{"uszTxNoCheckBit"		,"BLOB"		,""		,""},	/* 商店自存聯卡號遮掩開關 */
	{"uszSpecial00Bit"		,"BLOB"		,""		,""},	/* 特殊卡別參數檔，活動代碼00表示免簽(只紀錄，主要看uszNoSignatureBit) */
	{"uszSpecial01Bit"		,"BLOB"		,""		,""},	/* 特殊卡別參數檔，活動代碼01表示ECR回傳明碼(先決條件ECR卡號遮掩有開才做判斷) */
	{"uszRefundCTLSBit"		,"BLOB"		,""		,""},	/* 用在簽單印(W) 因為送電文contactless bit已OFF轉Manual Key in */
	{"uszMPASTransBit"		,"BLOB"		,""		,""},	/* 標示為小額交易 */
	{"uszMPASReprintBit"		,"BLOB"		,""		,""},	/* 標示該小額交易是否可重印 */
	{"uszMobilePayBit"		,"BLOB"		,""		,""},	/* 判斷是不是行動支付 Table NC */
	{"uszF56NoSignatureBit"	,"BLOB"		,""		,""},	/* F56免簽名使用 (免簽名則為true)*/
	{"uszReceiveCupF59"		,"BLOB"		,""		,""},	/* 補欄位 2020/1/10 下午 2:37 [SAM] */
	{"uszUpdated"			,"BLOB"		,""		,"DEFAULT 0"},	/* For SQLite使用，pobTran中不存，若設為1則代表該紀錄已不用，初始值設為0 */
	{""},
};

SQLITE_TAG_TABLE TABLE_EMV_BATCH_TAG[] =
{
	{"inTableID"				,"INTEGER"	,"PRIMARY KEY"	,""},	/* Table ID Primary key, sqlite table專用避免PRIMARY KEY重複 */
	{"inBatchTableID"			,"INTEGER"	,""		,""},	/* Table ID FOREIGN  key, sqlite table專用，用來對應Batch Table的FOREIGN Key */
	{"inEMVCardDecision"			,"INTEGER"	,""		,""},
	{"in50_APLabelLen"			,"INTEGER"	,""		,""},
	{"in5A_ApplPanLen"			,"INTEGER"	,""		,""},
	{"in5F20_CardholderNameLen"	,"INTEGER"	,""		,""},
	{"in5F24_ExpireDateLen"		,"INTEGER"	,""		,""},
	{"in5F2A_TransCurrCodeLen"	,"INTEGER"	,""		,""},
	{"in5F34_ApplPanSeqnumLen"	,"INTEGER"	,""		,""},
	{"in71_IssuerScript1Len"		,"INTEGER"	,""		,""},
	{"in72_IssuerScript2Len"		,"INTEGER"	,""		,""},
	{"in82_AIPLen"				,"INTEGER"	,""		,""},
	{"in84_DFNameLen"			,"INTEGER"	,""		,""},
	{"in8A_AuthRespCodeLen"		,"INTEGER"	,""		,""},
	{"in91_IssuerAuthDataLen"		,"INTEGER"	,""		,""},
	{"in95_TVRLen"				,"INTEGER"	,""		,""},
	{"in9A_TranDateLen"			,"INTEGER"	,""		,""},
	{"in9B_TSILen"				,"INTEGER"	,""		,""},
	{"in9C_TranTypeLen"			,"INTEGER"	,""		,""},
	{"in9F02_AmtAuthNumLen"		,"INTEGER"	,""		,""},
	{"in9F03_AmtOtherNumLen"	,"INTEGER"	,""		,""},
	{"in9F08_AppVerNumICCLen"	,"INTEGER"	,""		,""},
	{"in9F09_TermVerNumLen"		,"INTEGER"	,""		,""},
	{"in9F10_IssuerAppDataLen"		,"INTEGER"	,""		,""},
	{"in9F18_IssuerScriptIDLen"		,"INTEGER"	,""		,""},
	{"in9F1A_TermCountryCodeLen"	,"INTEGER"	,""		,""},
	{"in9F1E_IFDNumLen"			,"INTEGER"	,""		,""},
	{"in9F26_ApplCryptogramLen"	,"INTEGER"	,""		,""},
	{"in9F27_CIDLen"			,"INTEGER"	,""		,""},
	{"in9F33_TermCapabilitiesLen"	,"INTEGER"	,""		,""},
	{"in9F34_CVMLen"			,"INTEGER"	,""		,""},
	{"in9F35_TermTypeLen"		,"INTEGER"	,""		,""},
	{"in9F36_ATCLen"			,"INTEGER"	,""		,""},
	{"in9F37_UnpredictNumLen"	,"INTEGER"	,""		,""},
	{"in9F41_TransSeqCounterLen"	,"INTEGER"	,""		,""},
	{"in9F5A_Application_Program_IdentifierLen"	,"INTEGER"	,""		,""},
	{"in9F5B_ISRLen"						,"INTEGER"	,""		,""},
	{"in9F63_CardProductLabelInformationLen"	,"INTEGER"	,""		,""},
	{"in9F66_QualifiersLen"				,"INTEGER"	,""		,""},
	{"in9F6C_Card_Transaction_QualifiersLen"	,"INTEGER"	,""		,""},
	{"in9F6E_From_Factor_IndicatorLen"		,"INTEGER"	,""		,""},
	{"in9F74_TLVLen"					,"INTEGER"	,""		,""},
	{"inDF69_NewJspeedyModeLen"			,"INTEGER"	,""		,""},
	{"inDF8F4F_TransactionResultLen"	,"INTEGER"	,""		,""},
	{"inDFEC_FallBackIndicatorLen"		,"INTEGER"	,""		,""},
	{"inDFED_ChipConditionCodeLen"	,"INTEGER"	,""		,""},
	{"inDFEE_TerEntryCapLen"			,"INTEGER"	,""		,""},
	{"inDFEF_ReasonOnlineCodeLen"		,"INTEGER"	,""		,""},
	{"usz50_APLabel"				,"BLOB"		,""		,""},
	{"usz5A_ApplPan"				,"BLOB"		,""		,""},
	{"usz5F20_CardholderName"		,"BLOB"		,""		,""},
	{"usz5F24_ExpireDate"				,"BLOB"		,""		,""},
	{"usz5F2A_TransCurrCode"			,"BLOB"		,""		,""},
	{"usz5F34_ApplPanSeqnum"			,"BLOB"		,""		,""},
	{"usz71_IssuerScript1"				,"BLOB"		,""		,""},
	{"usz72_IssuerScript2"				,"BLOB"		,""		,""},
	{"usz82_AIP"					,"BLOB"		,""		,""},
	{"usz84_DF_NAME"				,"BLOB"		,""		,""},
	{"usz8A_AuthRespCode"			,"BLOB"		,""		,""},
	{"usz91_IssuerAuthData"			,"BLOB"		,""		,""},
	{"usz95_TVR"					,"BLOB"		,""		,""},
	{"usz9A_TranDate"				,"BLOB"		,""		,""},
	{"usz9B_TSI"					,"BLOB"		,""		,""},
	{"usz9C_TranType"				,"BLOB"		,""		,""},
	{"usz9F02_AmtAuthNum"			,"BLOB"		,""		,""},
	{"usz9F03_AmtOtherNum"			,"BLOB"		,""		,""},
	{"usz9F08_AppVerNumICC"			,"BLOB"		,""		,""},
	{"usz9F09_TermVerNum"			,"BLOB"		,""		,""},
	{"usz9F10_IssuerAppData"			,"BLOB"		,""		,""},
	{"usz9F18_IssuerScriptID"			,"BLOB"		,""		,""},
	{"usz9F1A_TermCountryCode"		,"BLOB"		,""		,""},
	{"usz9F1E_IFDNum"				,"BLOB"		,""		,""},
	{"usz9F26_ApplCryptogram"			,"BLOB"		,""		,""},
	{"usz9F27_CID"					,"BLOB"		,""		,""},
	{"usz9F33_TermCapabilities"		,"BLOB"		,""		,""},
	{"usz9F34_CVM"					,"BLOB"		,""		,""},
	{"usz9F35_TermType"				,"BLOB"		,""		,""},
	{"usz9F36_ATC"					,"BLOB"		,""		,""},
	{"usz9F37_UnpredictNum"			,"BLOB"		,""		,""},
	{"usz9F41_TransSeqCounter"		,"BLOB"		,""		,""},
	{"usz9F5A_Application_Program_Identifier"	,"BLOB"		,""		,""},
	{"usz9F5B_ISR"						,"BLOB"		,""		,""},
	{"usz9F63_CardProductLabelInformation"	,"BLOB"		,""		,""},
	{"usz9F66_Qualifiers"					,"BLOB"		,""		,""},
	{"usz9F6C_Card_Transaction_Qualifiers"	,"BLOB"		,""		,""},
	{"usz9F6E_From_Factor_Indicator"			,"BLOB"		,""		,""},
	{"usz9F74_TLV"						,"BLOB"		,""		,""},
	{"uszDF69_NewJspeedyMode"			,"BLOB"		,""		,""},
	{"uszDF8F4F_TransactionResult"			,"BLOB"		,""		,""},
	{"uszDFEC_FallBackIndicator"			,"BLOB"		,""		,""},
	{"uszDFED_ChipConditionCode"			,"BLOB"		,""		,""},
	{"uszDFEE_TerEntryCap"				,"BLOB"		,""		,""},
	{"uszDFEF_ReasonOnlineCode"			,"BLOB"		,""		,""},
	{""},
};

/*Note: var1字一byte nvar1字兩byte，空白可隨意插入SQL語句，唯Column（Tag）不能重複(找錯找好久)，*/
/*------------------------------------------------------------------------------------------
 * 語句形式
*
* static char	*szCreateTable = "
*/

/* 如果建立時不設檔案名，則設立暫時使用的資料庫(close 連結即消失)
----------------------------------下面三個選一個---------------------------------------------------------------------------
 * SQLITE_OPEN_READONLY			唯讀
 * SQLITE_OPEN_READWRITE			讀寫
 * SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE	檔案不存在就開檔
------------------------------------如果是單ap應該是不用考慮以下問題--------------------------------------------------------
 * SQLITE_OPEN_NOMUTEX		多線程方式，可以同時一個連結寫，多個連結讀 有可能會失敗   (啟用bCoreMutex禁用bFullMutex的鎖)
 * SQLITE_OPEN_FULLMUTEX	串行化方式，一個連結關閉，另一個連結才能連結，default為此 (啟用bCoreMutex和bFullMutex的鎖）
 * SQLITE_OPEN_SHAREDCACHE
 */

/*
Function        :inSqlite_Initial
Date&Time       :2017/3/14 上午 10:50
Describe        :
*/
int inSqlite_Initial()
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 設定*/
	inSqlite_Initial_Setting();

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Initial_Setting
Date&Time       :2017/3/14 上午 10:33
Describe        :有一些設定要改
*/
int inSqlite_Initial_Setting()
{
	int	inRc;
	int	inRetVal;
	int	inSwitch;
	int	inState;
	char	szDebugMsg[100 + 1];
	sqlite3	*srDBConnection = NULL;/* 建立到資料庫的connection */
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 開啟DataBase檔 */
	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	//inRetVal = inSqlite_DB_Open_Or_Create(gszTranDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq InitialSetting OpenV2 *Error* Rvl[%d] Line[%d]", inRetVal, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		inDISP_LogPrintfWithFlag("Open Database File OK");
		inDISP_LogPrintf(szDebugMsg);
		st_inSqlitHandleCnt ++;
	}

	/* SQLlite 外鍵預設為關閉，改成開啟(不然用外鍵就沒意義了) (1代表開啟 0代表關閉)*/
	inSwitch = 1;
	inRetVal = sqlite3_db_config(srDBConnection, SQLITE_DBCONFIG_ENABLE_FKEY, inSwitch, &inState);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq InitialSetting DB Config *Error* Rvl[%d] Line[%d]", inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  Switch[%d] inState[%d]", inSwitch, inState);
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq InitialSetting DB Config  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "SQLITE_DBCONFIG_ENABLE_FKEY Switch :%d OK, State: %d", inSwitch, inState);
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq InitialSetting DB Config  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}else{
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        : inSqlite_CreateBatchTable
Date&Time   : 2016/3/28 上午 10:47
Describe        : Open Database檔 建立table 這function只負責建batch的table
*/
int inSqlite_Create_Table(char* szDBName, char* szTableName, SQLITE_TAG_TABLE* pobSQLTag)
{
	
#ifdef _MODIFY_HDPT_FUNC_	
	int		i;
	int		inRetVal;
	int		inSqlLength = 0;
	char		szSqlPrefix[100 + 2];		/* CREATE TABLE	szTableName( */
	char		szSqlSuffix[10 + 2];		/* ); */
	char		szDebugMsg[100 + 1];
	char		*szCreateSql;			/* 因為會很長且隨table浮動，所以用pointer */
	char		*szErrorMessage = NULL;
	sqlite3		*srDBConnection;		/* 建立到資料庫的connection */
		
	
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 開啟DataBase檔 */
	inRetVal = inSqlite_DB_Open_Or_Create(szDBName, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("Open Database File *Error* LINE[%d]", __LINE__);
		return (VS_ERROR);
	}
	else
	{		
		inDISP_DispLogAndWriteFlie("Open Database File OK LINE[%d]", __LINE__);
	}

	/* 塞入SQL語句 */
	/* 為了使table name可變動，所以拉出來組 */
	/* 加入了if not exists字串，若已建立不會重複建立 */
	/* 前綴 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "CREATE TABLE if not exists %s(", szTableName);
	/*  "CREATE TABLE	szTableName("的長度 */
	inSqlLength += strlen(szSqlPrefix);
	
	
	/* 計算要分配的記憶體長度 */
	for (i = 0;; i ++)
	{
		/* 碰到Table底部 */
		if (strlen((char*)&pobSQLTag[i].szTag) == 0)
		{
			break;
		}
		
		/* 第一行前面不加逗號，其他都要 */
		if (i > 0)
		{
			inSqlLength += strlen(",");
		}
		
		/* Tag Name */
		inSqlLength += strlen((char*)&pobSQLTag[i].szTag);
		/* Tag 型別 */
		if (strlen((char*)&pobSQLTag[i].szType) > 0)
		{
			inSqlLength += strlen(" ");
			inSqlLength += strlen((char*)&pobSQLTag[i].szType);
		}
		/* Tag 屬性1 */
		if (strlen((char*)&pobSQLTag[i].szAttribute1) > 0)
		{
			inSqlLength += strlen(" ");
			inSqlLength += strlen((char*)&pobSQLTag[i].szAttribute1);
		}
		/* Tag 屬性2 */
		if (strlen((char*)&pobSQLTag[i].szAttribute2) > 0)
		{
			inSqlLength += strlen(" ");
			inSqlLength += strlen((char*)&pobSQLTag[i].szAttribute2);
		}
	}
	
	/* 後綴 */
	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, ");");
	inSqlLength += strlen(szSqlSuffix);
        
	/* inSqlLength: */
	inDISP_DispLogAndWriteFlie("inSqlLength [%d] LINE[%d]", inSqlLength, __LINE__);
	
	/* 配置記憶體(一定要+1，超級重要，不然會overflow) */
	szCreateSql = malloc(inSqlLength + 1);
	memset(szCreateSql, 0x00, inSqlLength);
	
	/* 先丟前綴Table Name */
	strcat(szCreateSql, szSqlPrefix);
	
	/* table要哪些tag */
	for (i = 0 ;; i ++)
        {
		/* 碰到Table底部 */
		if (strlen((char*)&pobSQLTag[i].szTag) == 0)
		{
			break;
		}
		
		/* 第一行前面不加逗號，其他都要 */
		if (i > 0)
		{
			strcat(szCreateSql , ",");
		}
		
		/* Tag Name */
		strcat(szCreateSql, (char*)&pobSQLTag[i].szTag);
		/* Tag 型別 */
		if (strlen((char*)&pobSQLTag[i].szType) > 0)
		{
			strcat(szCreateSql, " ");
			strcat(szCreateSql, (char*)&pobSQLTag[i].szType);
		}
		/* Tag 屬性1 */
		if (strlen((char*)&pobSQLTag[i].szAttribute1) > 0)
		{
			strcat(szCreateSql, " ");
			strcat(szCreateSql, (char*)&pobSQLTag[i].szAttribute1);
		}
		/* Tag 屬性2 */
		if (strlen((char*)&pobSQLTag[i].szAttribute2) > 0)
		{
			strcat(szCreateSql, " ");
			strcat(szCreateSql, (char*)&pobSQLTag[i].szAttribute2);
		}
        }
	
	/* 後綴 */
	strcat(szCreateSql, szSqlSuffix);
	
	/* 建立 Table */
	inRetVal = sqlite3_exec(srDBConnection, szCreateSql, 0, 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{		
		inDISP_DispLogAndWriteFlie("Create Table *Error* Num[%d] LINE[%d]",inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("Reason [%s] LINE[%d]",szErrorMessage, __LINE__);
		free(szCreateSql);
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			inDISP_DispLogAndWriteFlie("Close Database *Error* LINE[%d]",__LINE__);
			return (VS_ERROR);
		}
		else
		{
			inDISP_DispLogAndWriteFlie("Close Database OK LINE[%d]",__LINE__);
		}
		
		return (VS_ERROR);
	}
	else
	{
		inDISP_DispLogAndWriteFlie("Create Table OK LINE[%d]",__LINE__);
	
		/* 釋放記憶體 */
		free(szCreateSql);
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("Create Table *Error* LINE[%d]",__LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			inDISP_DispLogAndWriteFlie("Close Database OK LINE[%d]",__LINE__);
		}

	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
#endif
	return (VS_SUCCESS);
}



/*
Function        :inSqlite_CreateBatchTable
Date&Time       :2016/3/28 上午 10:47
Describe        :Open Database檔 建立table 這function只負責建batch的table
*/
int inSqlite_Create_BatchTable(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int	inRc;
	int	i;
	int	inRetVal;
	int	inSqlLength = 0;
	char	szSqlPrefix[100 + 2];		/* CREATE TABLE	szTableName( */
	char	szSqlSuffix[10 + 2];		/* ); */
	char	szDebugMsg[100 + 1];
	char	*szCreateSql;			/* 因為會很長且隨table浮動，所以用pointer */
	char	*szErrorMessage = NULL;
	sqlite3	*srDBConnection;		/* 建立到資料庫的connection */


	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 開啟DataBase檔 */
	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Create  Open V2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  Table Name [%s] Line[%d]", szTableName, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}

	/* 塞入SQL語句 */
	/* 為了使table name可變動，所以拉出來組 */
	/* 加入了if not exists字串，若已建立不會重複建立 */
	/* 前綴 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "CREATE TABLE if not exists %s(", szTableName);
	/*  "CREATE TABLE	szTableName("的長度 */
	inSqlLength += strlen(szSqlPrefix);


	/* 計算要分配的記憶體長度 */
	for (i = 0;; i ++)
	{
		/* 碰到Table底部 */
		if (strlen((char*)&TABLE_BATCH_TAG[i].szTag) == 0)
		{
			break;
		}

		/* 第一行前面不加逗號，其他都要 */
		if (i > 0)
		{
			inSqlLength += strlen(",");
		}

		/* Tag Name */
		inSqlLength += strlen((char*)&TABLE_BATCH_TAG[i].szTag);
		/* Tag 型別 */
		if (strlen((char*)&TABLE_BATCH_TAG[i].szType) > 0)
		{
			inSqlLength += strlen(" ");
			inSqlLength += strlen((char*)&TABLE_BATCH_TAG[i].szType);
		}
		/* Tag 屬性1 */
		if (strlen((char*)&TABLE_BATCH_TAG[i].szAttribute1) > 0)
		{
			inSqlLength += strlen(" ");
			inSqlLength += strlen((char*)&TABLE_BATCH_TAG[i].szAttribute1);
		}
		/* Tag 屬性2 */
		if (strlen((char*)&TABLE_BATCH_TAG[i].szAttribute2) > 0)
		{
			inSqlLength += strlen(" ");
			inSqlLength += strlen((char*)&TABLE_BATCH_TAG[i].szAttribute2);
		}
	}

	/* 後綴 */
	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, ");");
	inSqlLength += strlen(szSqlSuffix);

	/* inSqlLength: */
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", inSqlLength);
		inDISP_LogPrintf(szDebugMsg);
	}
	/* 配置記憶體(一定要+1，超級重要，不然會overflow) */
	szCreateSql = malloc(inSqlLength + 1);
	memset(szCreateSql, 0x00, inSqlLength);

	/* 先丟前綴Table Name */
	strcat(szCreateSql, szSqlPrefix);

	/* table要哪些tag */
	for (i = 0 ;; i ++)
	{
		/* 碰到Table底部 */
		if (strlen((char*)&TABLE_BATCH_TAG[i].szTag) == 0)
		{
			break;
		}

		/* 第一行前面不加逗號，其他都要 */
		if (i > 0)
		{
			strcat(szCreateSql , ",");
		}

		/* Tag Name */
		strcat(szCreateSql, (char*)&TABLE_BATCH_TAG[i].szTag);
		/* Tag 型別 */
		if (strlen((char*)&TABLE_BATCH_TAG[i].szType) > 0)
		{
			strcat(szCreateSql, " ");
			strcat(szCreateSql, (char*)&TABLE_BATCH_TAG[i].szType);
		}
		/* Tag 屬性1 */
		if (strlen((char*)&TABLE_BATCH_TAG[i].szAttribute1) > 0)
		{
			strcat(szCreateSql, " ");
			strcat(szCreateSql, (char*)&TABLE_BATCH_TAG[i].szAttribute1);
		}
		/* Tag 屬性2 */
		if (strlen((char*)&TABLE_BATCH_TAG[i].szAttribute2) > 0)
		{
			strcat(szCreateSql, " ");
			strcat(szCreateSql, (char*)&TABLE_BATCH_TAG[i].szAttribute2);
		}
	}

	/* 後綴 */
	strcat(szCreateSql, szSqlSuffix);

	/* 建立 Table */
	inRetVal = sqlite3_exec(srDBConnection, szCreateSql, 0, 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq CreateBatchTable exec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  Table Name [%s] Reason Msg[%s] Line[%d]", szTableName, szErrorMessage, __LINE__);

		free(szCreateSql);

		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq CreateBatchTable Close inRc[%d] *Error* Line[%d]", inRc,  __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}

		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Create Table OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}

	/* 釋放記憶體 */
	free(szCreateSql);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq CreateBatchTable Close inRc[%d] *Error* Line[%d]", inRc,  __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}else{
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Create_BatchTable_EMV
Date&Time       :2017/3/14 下午 4:48
Describe        :Open Database檔 建立table 這function只負責建batch的table
 *		szTableName2:用來設定外鍵
*/
int inSqlite_Create_BatchTable_EMV(TRANSACTION_OBJECT *pobTran, char* szTableName1, char* szTableName2)
{
	int	inRc;
	int	i;
	int	inRetVal;
	int	inSqlLength = 0;
	char	szSqlPrefix[100 + 2];		/* CREATE TABLE	szTableName( */
	char	szSqlSuffix[10 + 2];		/* ); */
	char	szAdditionalData[300 + 1];	/* 因為加檔案會超過，所以改大 原值為100 20190924  [SAM] */
	char	szDebugMsg[100 + 1];
	char	*szCreateSql;			/* 因為會很長且隨table浮動，所以用pointer */
	char	*szErrorMessage = NULL;
	sqlite3	*srDBConnection;		/* 建立到資料庫的connection */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 開啟DataBase檔 */
	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Create Emv OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName1[%s] szTableName2[%s] Line[%d]", szTableName1,szTableName2,  __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}

	/* 塞入SQL語句 */
	/* 為了使table name可變動，所以拉出來組 */
	/* 加入了if not exists字串，若已建立不會重複建立 */
	/* 前綴 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "CREATE TABLE if not exists %s(", szTableName1);
	/*  "CREATE TABLE	szTableName("的長度 */
	inSqlLength += strlen(szSqlPrefix);


	/* 計算要分配的記憶體長度 */
	for (i = 0;; i ++)
	{
		/* 碰到Table底部 */
		if (strlen((char*)&TABLE_EMV_BATCH_TAG[i].szTag) == 0)
		{
			break;
		}

		/* 第一行前面不加逗號，其他都要 */
		if (i > 0)
		{
			inSqlLength += strlen(",");
		}

		/* Tag Name */
		inSqlLength += strlen((char*)&TABLE_EMV_BATCH_TAG[i].szTag);
		/* Tag 型別 */
		if (strlen((char*)&TABLE_EMV_BATCH_TAG[i].szType) > 0)
		{
			inSqlLength += strlen(" ");
			inSqlLength += strlen((char*)&TABLE_EMV_BATCH_TAG[i].szType);
		}
		/* Tag 屬性1 */
		if (strlen((char*)&TABLE_EMV_BATCH_TAG[i].szAttribute1) > 0)
		{
			inSqlLength += strlen(" ");
			inSqlLength += strlen((char*)&TABLE_EMV_BATCH_TAG[i].szAttribute1);
		}
		/* Tag 屬性2 */
		if (strlen((char*)&TABLE_EMV_BATCH_TAG[i].szAttribute2) > 0)
		{
			inSqlLength += strlen(" ");
			inSqlLength += strlen((char*)&TABLE_EMV_BATCH_TAG[i].szAttribute2);
		}
	}
	/* 外鍵 ",FOREIGN KEY(inBatchTableID) REFERENCES", szTableName2(inTableID)" */
	memset(szAdditionalData, 0x00, sizeof(szAdditionalData));
	sprintf(szAdditionalData, ",FOREIGN KEY(inBatchTableID) REFERENCES %s(inTableID) ON DELETE CASCADE ON UPDATE CASCADE", szTableName2);
	inSqlLength += strlen(szAdditionalData);

	/* 後綴 */
	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, ");");
	inSqlLength += strlen(szSqlSuffix);

	/* inSqlLength: */
	if (ginDebug == VS_TRUE)
        {
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", inSqlLength);
		inDISP_LogPrintf(szDebugMsg);
	}
	/* 配置記憶體(一定要+1，超級重要，不然會overflow) */
	szCreateSql = malloc(inSqlLength + 1);
	memset(szCreateSql, 0x00, inSqlLength);

	/* 先丟前綴Table Name */
	strcat(szCreateSql, szSqlPrefix);

	/* table要哪些tag */
	for (i = 0 ;; i ++)
        {
		/* 碰到Table底部 */
		if (strlen((char*)&TABLE_EMV_BATCH_TAG[i].szTag) == 0)
		{
			break;
		}

		/* 第一行前面不加逗號，其他都要 */
		if (i > 0)
		{
			strcat(szCreateSql , ",");
		}

		/* Tag Name */
		strcat(szCreateSql, (char*)&TABLE_EMV_BATCH_TAG[i].szTag);
		/* Tag 型別 */
		if (strlen((char*)&TABLE_EMV_BATCH_TAG[i].szType) > 0)
		{
			strcat(szCreateSql, " ");
			strcat(szCreateSql, (char*)&TABLE_EMV_BATCH_TAG[i].szType);
		}
		/* Tag 屬性1 */
		if (strlen((char*)&TABLE_EMV_BATCH_TAG[i].szAttribute1) > 0)
		{
			strcat(szCreateSql, " ");
			strcat(szCreateSql, (char*)&TABLE_EMV_BATCH_TAG[i].szAttribute1);
		}
		/* Tag 屬性2 */
		if (strlen((char*)&TABLE_EMV_BATCH_TAG[i].szAttribute2) > 0)
		{
			strcat(szCreateSql, " ");
			strcat(szCreateSql, (char*)&TABLE_EMV_BATCH_TAG[i].szAttribute2);
		}
        }

	/* 設定外鍵 */
	strcat(szCreateSql, szAdditionalData);

	/* 後綴 */
	strcat(szCreateSql, szSqlSuffix);

	/* 建立 Table */
	inRetVal = sqlite3_exec(srDBConnection, szCreateSql, 0, 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Create Emv exec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName1[%s] szTableName2[%s]  Reason Msg[%s] Line[%d]", szTableName1, szTableName2, szErrorMessage, __LINE__);

		free(szCreateSql);

		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq Create Emv Close inRc[%d] *Error* Line[%d]", inRc, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}

		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Create Table OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}

	/* 釋放記憶體 */
	free(szCreateSql);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Create Emv Close inRc[%d] *Error* Line[%d]", inRc, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}else{
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}


/*
Function        :inSqlite_Drop_Table_Flow
Date&Time       :2017/3/14 下午 3:00
Describe        :在這邊決定名稱並分流
*/
int inSqlite_Drop_Table_Flow(TRANSACTION_OBJECT *pobTran, int inTableType)
{

	int	inRetVal = VS_SUCCESS;
#ifdef _MODIFY_HDPT_FUNC_
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	if (pobTran->uszFileNameNoNeedNumBit == VS_TRUE)
	{
		inFunc_ComposeFileName(pobTran, szTableName, "", 0);
	}
	else
	{
		inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	}
	
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
//		case _TN_BATCH_TABLE_ESC_TEMP_:
//			strcat(szTableName, _BATCH_TABLE_ESC_TEMP_SUFFIX_);
//			break;
//		case _TN_BATCH_TABLE_ESC_ADVICE_:
//			strcat(szTableName, _BATCH_TABLE_ESC_ADVICE_SUFFIX_);
//			break;
//		case _TN_BATCH_TABLE_ESC_ADVICE_EMV_:
//			strcat(szTableName, _BATCH_TABLE_ESC_ADVICE_EMV_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ADVICE_SUFFIX_);
		default :
			break;
	}
	
	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
//		case _TN_BATCH_TABLE_ESC_TEMP_:
		case _TN_EMV_TABLE_:
		case _TN_BATCH_TABLE_TICKET_:
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
//		case _TN_BATCH_TABLE_ESC_ADVICE_:
//		case _TN_BATCH_TABLE_ESC_ADVICE_EMV_:
#ifdef _MODIFY_HDPT_FUNC_			
			inRetVal = inSqlite_Drop_Table(gszTranDBPath, szTableName);
#endif
			break;
		default :
			return (VS_ERROR);
			break;
	}
#endif	
	return (inRetVal);
}

/*
Function        : inSqlite_Drop_Table
Date&Time   : 2016/4/11 下午 3:25
Describe        : delete 該batch的table
*/
int inSqlite_Drop_Table(char* szDBName, char* szTableName)
{
#ifdef _MODIFY_HDPT_FUNC_
	int		inRetVal;
	char		szDeleteSql[80 + 1];	/* INSERT INTO	szTableName( */
	char		*szErrorMessage = NULL;
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */
	
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	inRetVal = inSqlite_DB_Open_Or_Create(szDBName, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != VS_SUCCESS)
	{	
		inDISP_DispLogAndWriteFlie(" Open Database File *Error* LINE[%d]", __LINE__);
		return (VS_ERROR);
	}
	else
	{
		inDISP_DispLogAndWriteFlie(" Open Database File OK LINE[%d]", __LINE__);
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szDeleteSql, 0x00, sizeof(szDeleteSql));
	sprintf(szDeleteSql, "DROP TABLE %s", szTableName);
	
	/* Delete */
	inRetVal = sqlite3_exec(srDBConnection, szDeleteSql, 0, 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie(" Drop Table ERROR Num[%d] LINE[%d]", inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie(" Reason[%s] LINE[%d]", szErrorMessage, __LINE__);
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			inDISP_DispLogAndWriteFlie(" Open Database File OK LINE[%d]", __LINE__);
		}
		
		return (VS_ERROR);
	}
	else
	{
		inDISP_DispLogAndWriteFlie(" Drop OK LINE[%d]", __LINE__);
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			inDISP_DispLogAndWriteFlie(" Close Database File *Error* LINE[%d]", __LINE__);
			return (VS_ERROR);
		}
		else
		{
			inDISP_DispLogAndWriteFlie(" Close Database File OK LINE[%d]", __LINE__);
		}
	}

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
#endif	
	return (VS_SUCCESS);
}



/*
 * Function		: inSqlite_DeleteBatchTable_Flow
 * Date&Time	: 2017/3/14 下午 3:00
 * Describe		: 
 * 在這邊決定名稱並分流，使用 HOST NAME、 BATCH NUMBER 組合 TABLE NAME，
 * 並執行資料表的刪除。
*/
int inSqlite_Delete_Table_Flow(TRANSACTION_OBJECT *pobTran, int inTableType)
{
	int	inRetVal;
	char	szTableName[50 + 1];
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* TableName 會是像 CR000001 的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_EMV_:/* 新增ESC EMV記錄 20190919 [SAM] */
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_EMV_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_EMV_:/* 新增ESC EMV記錄 20190919 [SAM] */
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_EMV_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ADVICE_SUFFIX_);
			break;
		default :
			inDISP_DispLogAndWriteFlie("  TbType[%d] Not Found  *Error* Line[%d]", inTableType, __LINE__ );
			break;
	}

	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_BATCH_TABLE_ESC_AGAIN_EMV_:/* 新增ESC EMV記錄 20190919 [SAM] */
		case _TN_BATCH_TABLE_ESC_FAIL_EMV_:  /* 新增ESC EMV記錄 20190919 [SAM] */
		case _TN_EMV_TABLE_:
		case _TN_BATCH_TABLE_TICKET_:
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			inRetVal = inSqlite_Delete_Table(pobTran, szTableName);
			break;
		default :
			inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] TbType[%d] *Error* END -----",__FILE__, __FUNCTION__, __LINE__,inTableType);
			return (VS_ERROR);
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
 * Function		: inSqlite_Delete_Table
 * Date&Time	: 2016/4/11 下午 3:25
 * Describe		:  
 * 使用 DROP TABLE 指令，進行表的刪除 delete 該batch的table
*/
int inSqlite_Delete_Table(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int	inRc;
	int	inRetVal;
	char	szDeleteSql[80 + 1];	/* INSERT INTO	szTableName( */
	char	*szErrorMessage = NULL;
	sqlite3	*srDBConnection;	/* 建立到資料庫的connection */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Delete  Open V2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  Table Name [%s] Line[%d]", szTableName, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		inDISP_LogPrintfWithFlag("Open Database File OK");
		st_inSqlitHandleCnt ++;
	}

	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szDeleteSql, 0x00, sizeof(szDeleteSql));
	sprintf(szDeleteSql, "DROP TABLE %s", szTableName);

	/* Delete */
	inRetVal = sqlite3_exec(srDBConnection, szDeleteSql, 0, 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Delete exec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  Table Name [%s] Reason Msg[%s] Line[%d]", szTableName, szErrorMessage, __LINE__);
		inDISP_DispLogAndWriteFlie("  szDeleteSql [%s] Line[%d]", szDeleteSql,  __LINE__);
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq Delete Close inRc[%d] *Error* Line[%d]",inRc,  __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			inDISP_LogPrintfWithFlag("Close Database OK");
			st_inSqlitHandleCnt --;
		}
		return (VS_ERROR);
	}
	else
	{
		inDISP_LogPrintfWithFlag("Delete OK");
	}

	/* 關閉 database, close null pointer 是NOP(No Operation) */
	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Delete Close RetVal[%d] *Error* Line[%d]",inRc,  __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		inDISP_LogPrintfWithFlag("Close Database OK");
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * Function		:inSqlite_Table_Link_BRec
 * Date&Time	:2017/3/13 上午 10:59
 * Describe		:
 * 將pobTran變數pointer位置放到Table中(用以解決每一個function都要放一個table的問題)
 * 這邊直接把pobTran的pointer直接指到srAll(之後可能要考慮給動態記憶體)，
 * TagName因為是寫在這個Function內的Table，所以要給實體位置儲存
*/
int inSqlite_Table_Link_BRec(TRANSACTION_OBJECT *pobTran, SQLITE_ALL_TABLE *srAll, int inLinkState)
{
	SQLITE_INT32T_TABLE TABLE_BATCH_INT[] =
	{
		{0	,"inTableID"			,&pobTran->inTableID				},	/* inTableID */
		{0	,"inCode"				,&pobTran->srBRec.inCode				},	/* Trans Code */
		{0	,"inOrgCode"			,&pobTran->srBRec.inOrgCode			},	/* Original Trans Code  */
		{0	,"inPrintOption"			,&pobTran->srBRec.inPrintOption		},	/* Print Option Flag */
		{0	,"inHDTIndex"			,&pobTran->srBRec.inHDTIndex			},	/* 紀錄HDTindex */
		{0	,"inCDTIndex"			,&pobTran->srBRec.inCDTIndex			},	/* 紀錄CDTindex */
		{0	,"inCPTIndex"			,&pobTran->srBRec.inCPTIndex			},	/* 紀錄CPTindex */
		{0	,"inTxnResult"			,&pobTran->srBRec.inTxnResult			},	/* 紀錄交易結果 */
		{0	,"inChipStatus"			,&pobTran->srBRec.inChipStatus			},	/* 0 NOT_USING_CHIP, 1 EMV_CARD, 2 EMV_EASY_ENTRY_CARD */
		{0	,"inFiscIssuerIDLength"		,&pobTran->srBRec.inFiscIssuerIDLength		},	/* 金融卡發卡單位代號長度 */
		{0	,"inFiscCardCommentLength"	,&pobTran->srBRec.inFiscCardCommentLength	},	/* 金融卡備註欄長度 */
		{0	,"inFiscAccountLength"	,&pobTran->srBRec.inFiscAccountLength		},	/* 金融卡帳號長度 */
		{0	,"inFiscSTANLength"		,&pobTran->srBRec.inFiscSTANLength		},	/* 金融卡交易序號長度 */
		{0	,"inESCTransactionCode"	,&pobTran->srBRec.inESCTransactionCode		},	/* ESC組ISO使用 重新上傳使用 Transaction Code沒辦法存在Batch */
		{0	,"inESCUploadMode"		,&pobTran->srBRec.inESCUploadMode		},	/* 標示支不支援ESC */
		{0	,"inESCUploadStatus"		,&pobTran->srBRec.inESCUploadStatus		},	/* 標示ESC上傳狀態 */
		{0	,"inSignStatus"			,&pobTran->srBRec.inSignStatus			},	/* 簽名檔狀態(有 免簽 或 Bypass) ESC電文使用 */
		{0	,"inHGCreditHostIndex"	,&pobTran->srBRec.inHGCreditHostIndex		},	/* 聯合_HAPPY_GO_信用卡主機 */
		{0	,"inHGCode"			,&pobTran->srBRec.inHGCode			},	/* 聯合_HAPPY_GO_交易碼 */
		{0	,"lnTxnAmount"			,&pobTran->srBRec.lnTxnAmount			},	/* The transaction amount, such as a SALE */
		{0	,"lnOrgTxnAmount"		,&pobTran->srBRec.lnOrgTxnAmount		},	/* The ORG transaction amount, such as a SALE */
		{0	,"lnTipTxnAmount"		,&pobTran->srBRec.lnTipTxnAmount		},	/* The transaction amount, such as a TIP */
		{0	,"lnAdjustTxnAmount"		,&pobTran->srBRec.lnAdjustTxnAmount		},	/* The transaction amount, such as a ADJUST */
		{0	,"lnTotalTxnAmount"		,&pobTran->srBRec.lnTotalTxnAmount		},	/* The transaction amount, such as a TOTAL */
		{0	,"lnInvNum"			,&pobTran->srBRec.lnInvNum			},	/* 調閱編號  */
		{0	,"lnOrgInvNum"			,&pobTran->srBRec.lnOrgInvNum			},	/* Original 調閱編號  */
		{0	,"lnBatchNum"			,&pobTran->srBRec.lnBatchNum			},	/* Batch Number */
		{0	,"lnOrgBatchNum"		,&pobTran->srBRec.lnOrgBatchNum			},	/* Original Batch Number */
		{0	,"lnSTANNum"			,&pobTran->srBRec.lnSTANNum			},	/* Stan Number */
		{0	,"lnSTAN_Multi"			,&pobTran->srBRec.lnSTAN_Multi 		},	/* Stan Number 2 */
		{0	,"lnOrgSTANNum"		,&pobTran->srBRec.lnOrgSTANNum			},	/* Original Stan Number */
		{0	,"lnInstallmentPeriod"		,&pobTran->srBRec.lnInstallmentPeriod		},	/* 分期付款_期數 */
		{0	,"lnInstallmentDownPayment"	,&pobTran->srBRec.lnInstallmentDownPayment	},	/* 分期付款_頭期款 */
		{0	,"lnInstallmentPayment"		,&pobTran->srBRec.lnInstallmentPayment		},	/* 分期付款_每期款 */
		{0	,"lnInstallmentFormalityFee"	,&pobTran->srBRec.lnInstallmentFormalityFee	},	/* 分期付款_手續費 */
		{0	,"lnRedemptionPoints"		,&pobTran->srBRec.lnRedemptionPoints		},	/* 紅利扣抵_扣抵紅利點數 */
		{0	,"lnRedemptionPointsBalance",	&pobTran->srBRec.lnRedemptionPointsBalance	},	/* 紅利扣抵_剩餘紅利點數 */
		{0	,"lnRedemptionPaidAmount"	,&pobTran->srBRec.lnRedemptionPaidAmount	},	/* 紅利扣抵_抵扣金額 */
		{0	,"lnRedemptionPaidCreditAmount"	,&pobTran->srBRec.lnRedemptionPaidCreditAmount	},	/* 紅利扣抵_支付金額 */
		{0	,"lnHGTransactionType"	,&pobTran->srBRec.lnHGTransactionType		},	/* 聯合_HAPPY GO_交易類別 */
		{0	,"lnHGPaymentType"		,&pobTran->srBRec.lnHGPaymentType		},	/* 聯合_HAPPY_GO_支付工具 */
		{0	,"lnHGPaymentTeam"		,&pobTran->srBRec.lnHGPaymentTeam		},	/* 聯合_HAPPY_GO_支付工具_主機回_*/
		{0	,"lnHGBalancePoint"		,&pobTran->srBRec.lnHGBalancePoint		},	/* 聯合_HAPPY_GO_剩餘點數 */
		{0	,"lnHGTransactionPoint"	,&pobTran->srBRec.lnHGTransactionPoint		},	/* 聯合_HAPPY_GO_交易點數  合計 */
		{0	,"lnHGAmount"			,&pobTran->srBRec.lnHGAmount			},	/* 聯合_HAPPY_GO_扣抵後金額  (商品金額 = lnHGAmount + lnHGRedeemAmt) */
		{0	,"lnHGRedeemAmount"	,&pobTran->srBRec.lnHGRedeemAmount		},	/* 聯合_HAPPY_GO_扣抵金額 */
		{0	,"lnHGRefundLackPoint"	,&pobTran->srBRec.lnHGRefundLackPoint		},	/* 聯合_HAPPY_GO_不足點數 */
		{0	,"lnHGBatchIndex"		,&pobTran->srBRec.lnHGBatchIndex		},	/* 聯合_HAPPY_GO_主機當下批次號碼 */
		{0	,"lnHG_SPDH_OrgInvNum"	,&pobTran->srBRec.lnHG_SPDH_OrgInvNum		},	/* HAPPY_GO取消用INV */
		{0	,"lnHGSTAN"			,&pobTran->srBRec.lnHGSTAN			},	/* HAPPY_GO STAN */
		{0	,"lnFubonGoodsID"		,&pobTran->srBRec.lnFubonGoodsID			},	/* lnFubonGoodsID */
		{0	,""				,NULL						}	/* 這行用Null用來知道尾端在哪 */

	};

	SQLITE_INT64T_TABLE TABLE_BATCH_INT64T[] =
	{
		{0	,""				,NULL						}	/* 這行用Null用來知道尾端在哪 */
	};

	SQLITE_CHAR_TABLE TABLE_BATCH_CHAR[] =
	{
		{0	,"szAuthCode"			,pobTran->srBRec.szAuthCode			,sizeof(pobTran->srBRec.szAuthCode)		},	/* Auth Code */
		{0	,"szMPASAuthCode"		,pobTran->srBRec.szMPASAuthCode		,sizeof(pobTran->srBRec.szMPASAuthCode)		},	/* MPAS Auth Code */
		{0	,"szRespCode"			,pobTran->srBRec.szRespCode			,sizeof(pobTran->srBRec.szRespCode)		},	/* Response Code */
		{0	,"szStoreID"			,pobTran->srBRec.szStoreID				,sizeof(pobTran->srBRec.szStoreID)		},	/* StoreID */
		{0	,"szCardLabel"			,pobTran->srBRec.szCardLabel			,sizeof(pobTran->srBRec.szCardLabel)		},	/* 卡別  */
		{0	,"szPAN"				,pobTran->srBRec.szPAN				,sizeof(pobTran->srBRec.szPAN)			},	/* 卡號  */
		{0	,"szDate"				,pobTran->srBRec.szDate				,sizeof(pobTran->srBRec.szDate)			},	/* YYYYMMDD */
		{0	,"szOrgDate"			,pobTran->srBRec.szOrgDate			,sizeof(pobTran->srBRec.szOrgDate)		},	/* YYYYMMDD */
		{0	,"szEscDate"			,pobTran->srBRec.szEscDate			,sizeof(pobTran->srBRec.szEscDate)		},	/* YYYYMMDD */
		{0	,"szTime"				,pobTran->srBRec.szTime				,sizeof(pobTran->srBRec.szTime)			},	/* HHMMSS */
		{0	,"szOrgTime"			,pobTran->srBRec.szOrgTime			,sizeof(pobTran->srBRec.szOrgTime)		},	/* HHMMSS */
		{0	,"szEscTime"			,pobTran->srBRec.szEscTime			,sizeof(pobTran->srBRec.szEscTime)		},	/* HHMMSS */
		{0	,"szCardTime"			,pobTran->srBRec.szCardTime			,sizeof(pobTran->srBRec.szCardTime)		},	/* 晶片卡讀卡時間 , YYYYMMDDHHMMSS */
		{0	,"szRefNo"				,pobTran->srBRec.szRefNo				,sizeof(pobTran->srBRec.szRefNo)		},	/* 序號  */
		{0	,"szExpDate"			,pobTran->srBRec.szExpDate			,sizeof(pobTran->srBRec.szExpDate)		},	/* Expiration date */
		{0	,"szServiceCode"			,pobTran->srBRec.szServiceCode			,sizeof(pobTran->srBRec.szServiceCode)		},	/* Service code from track */
		{0	,"szCardHolder"			,pobTran->srBRec.szCardHolder			,sizeof(pobTran->srBRec.szCardHolder)		},	/* 持卡人名字 */
		{0	,"szAMEX4DBC"			,pobTran->srBRec.szAMEX4DBC			,sizeof(pobTran->srBRec.szAMEX4DBC)		},
		{0	,"szFiscIssuerID"			,pobTran->srBRec.szFiscIssuerID			,sizeof(pobTran->srBRec.szFiscIssuerID)		},	/* 發卡單位代號 */
		{0	,"szFiscCardComment"	,pobTran->srBRec.szFiscCardComment		,sizeof(pobTran->srBRec.szFiscCardComment)	},	/* 金融卡備註欄 */
		{0	,"szFiscAccount"			,pobTran->srBRec.szFiscAccount			,sizeof(pobTran->srBRec.szFiscAccount)		},	/* 金融卡帳號 */
		{0	,"szFiscOutAccount"		,pobTran->srBRec.szFiscOutAccount		,sizeof(pobTran->srBRec.szFiscOutAccount)	},	/* 金融卡轉出帳號 */
		{0	,"szFiscSTAN"			,pobTran->srBRec.szFiscSTAN			,sizeof(pobTran->srBRec.szFiscSTAN)		},	/* 金融卡交易序號 */
		{0	,"szFiscTacLength"		,pobTran->srBRec.szFiscTacLength		,sizeof(pobTran->srBRec.szFiscTacLength)	},	/* 金融卡Tac長度 */
		{0	,"szFiscTac"			,pobTran->srBRec.szFiscTac				,sizeof(pobTran->srBRec.szFiscTac)		},	/* 金融卡Tac */
		{0	,"szFiscTCC"			,pobTran->srBRec.szFiscTCC				,sizeof(pobTran->srBRec.szFiscTCC)		},	/* 端末設備查核碼 */
		{0	,"szFiscMCC"			,pobTran->srBRec.szFiscMCC			,sizeof(pobTran->srBRec.szFiscMCC)		},	/* 金融卡MCC */
		{0	,"szFiscRRN"			,pobTran->srBRec.szFiscRRN			,sizeof(pobTran->srBRec.szFiscRRN)		},	/* 金融卡調單編號 */
		{0	,"szFiscRefundDate"		,pobTran->srBRec.szFiscRefundDate		,sizeof(pobTran->srBRec.szFiscRefundDate)	},	/* 金融卡退貨原始交易日期(YYYYMMDD) */
		{0	,"szFiscDateTime"		,pobTran->srBRec.szFiscDateTime		,sizeof(pobTran->srBRec.szFiscDateTime)		},	/* 計算TAC(S2)的交易日期時間 */
		{0	,"szFiscPayDevice"		,pobTran->srBRec.szFiscPayDevice		,sizeof(pobTran->srBRec.szFiscPayDevice)	},	/* 金融卡付款裝置 1 = 手機 2 = 卡片 */
		{0	,"szFiscMobileDevice"		,pobTran->srBRec.szFiscMobileDevice		,sizeof(pobTran->srBRec.szFiscMobileDevice)	},	/* SE 類型，0x05：雲端卡片(Cloud-Based) */
		{0	,"szFiscMobileNFType"		,pobTran->srBRec.szFiscMobileNFType		,sizeof(pobTran->srBRec.szFiscMobileNFType)	},	/* 行動金融卡是否需輸入密碼 00不需要 01視情況 02一定要 */
		{0	,"szFiscMobileNFSetting"	,pobTran->srBRec.szFiscMobileNFSetting	,sizeof(pobTran->srBRec.szFiscMobileNFSetting)	},	/* 近端交易類型設定 0x00：Single Issuer Wallet 0x01：國內Third-Party Wallet 0x02~9：保留 0x0A：其他 */
		{0	,"szInstallmentIndicator"	,pobTran->srBRec.szInstallmentIndicator	,sizeof(pobTran->srBRec.szInstallmentIndicator)	},
		{0	,"szRedeemIndicator"		,pobTran->srBRec.szRedeemIndicator		,sizeof(pobTran->srBRec.szRedeemIndicator)	},
		{0	,"szRedeemSignOfBalance"	,pobTran->srBRec.szRedeemSignOfBalance	,sizeof(pobTran->srBRec.szRedeemSignOfBalance)	},
		{0	,"szHGCardLabel"		,pobTran->srBRec.szHGCardLabel			,sizeof(pobTran->srBRec.szHGCardLabel)		},	/* HAPPY_GO 卡別 */
		{0	,"szHGPAN"			,pobTran->srBRec.szHGPAN				,sizeof(pobTran->srBRec.szHGPAN)		},	/* HAPPY_GO Account number */
		{0	,"szHGAuthCode"		,pobTran->srBRec.szHGAuthCode			,sizeof(pobTran->srBRec.szHGAuthCode)		},	/* HAPPY_GO 授權碼 */
		{0	,"szHGRefNo"			,pobTran->srBRec.szHGRefNo			,sizeof(pobTran->srBRec.szHGRefNo)		},	/* HAPPY_GO Reference Number */
		{0	,"szHGRespCode"		,pobTran->srBRec.szHGRespCode			,sizeof(pobTran->srBRec.szHGRespCode)		},	/* HG Response Code */
		{0	,"szCUP_TN"			,pobTran->srBRec.szCUP_TN			,sizeof(pobTran->srBRec.szCUP_TN)		},	/* CUP Trace Number (TN) */
		{0	,"szCUP_TD"			,pobTran->srBRec.szCUP_TD			,sizeof(pobTran->srBRec.szCUP_TD)		},	/* CUP Transaction Date (MMDD) */
		{0	,"szCUP_TT"			,pobTran->srBRec.szCUP_TT				,sizeof(pobTran->srBRec.szCUP_TT)		},	/* CUP Transaction Time (hhmmss) */
		{0	,"szCUP_RRN"			,pobTran->srBRec.szCUP_RRN			,sizeof(pobTran->srBRec.szCUP_RRN)		},	/* CUP Retrieve Reference Number (CRRN) */
		{0	,"szCUP_STD"			,pobTran->srBRec.szCUP_STD			,sizeof(pobTran->srBRec.szCUP_STD)		},	/* CUP Settlement Date(MMDD) Of Host Response */
		{0	,"szCUP_EMVAID"		,pobTran->srBRec.szCUP_EMVAID		,sizeof(pobTran->srBRec.szCUP_EMVAID)		},	/* CUP晶片交易存AID帳單列印使用 */
		{0	,"szTranAbbrev"			,pobTran->srBRec.szTranAbbrev			,sizeof(pobTran->srBRec.szTranAbbrev)		},	/* Tran abbrev for reports */
		{0	,"szIssueNumber"		,pobTran->srBRec.szIssueNumber			,sizeof(pobTran->srBRec.szIssueNumber)		},
		{0	,"szStore_DREAM_MALL"	,pobTran->srBRec.szStore_DREAM_MALL	,sizeof(pobTran->srBRec.szStore_DREAM_MALL	)	},	/* 存Dream_Mall Account Number And Member ID*/
		{0	,"szDCC_FCNFR"			,pobTran->srBRec.szDCC_FCNFR			,sizeof(pobTran->srBRec.szDCC_FCNFR)		},	/* Foreign Currency No. For Rate */
		{0	,"szDCC_AC"			,pobTran->srBRec.szDCC_AC			,sizeof(pobTran->srBRec.szDCC_AC)		},	/* Action Code */
		{0	,"szDCC_FCN"			,pobTran->srBRec.szDCC_FCN			,sizeof(pobTran->srBRec.szDCC_FCN)		},	/* Foreign Currency Number */
		{0	,"szDCC_FCA"			,pobTran->srBRec.szDCC_FCA			,sizeof(pobTran->srBRec.szDCC_FCA)		},	/* Foreign Currency Amount */
		{0	,"szDCC_FCMU"			,pobTran->srBRec.szDCC_FCMU			,sizeof(pobTran->srBRec.szDCC_FCMU)		},	/* Foreign Currency Minor Unit */
		{0	,"szDCC_FCAC"			,pobTran->srBRec.szDCC_FCAC			,sizeof(pobTran->srBRec.szDCC_FCAC)		},	/* Foreign currcncy Alphabetic Code */
		{0	,"szDCC_ERMU"			,pobTran->srBRec.szDCC_ERMU			,sizeof(pobTran->srBRec.szDCC_ERMU)		},	/* Exchange Rate Minor Unit */
		{0	,"szDCC_ERV"			,pobTran->srBRec.szDCC_ERV			,sizeof(pobTran->srBRec.szDCC_ERV)		},	/* Exchange Rate Value */
		{0	,"szDCC_IRMU"			,pobTran->srBRec.szDCC_IRMU			,sizeof(pobTran->srBRec.szDCC_IRMU)		},	/* Inverted Rate Minor Unit */
		{0	,"szDCC_IRV"			,pobTran->srBRec.szDCC_IRV			,sizeof(pobTran->srBRec.szDCC_IRV)		},	/* Inverted Rate Value */
		{0	,"szDCC_IRDU"			,pobTran->srBRec.szDCC_IRDU			,sizeof(pobTran->srBRec.szDCC_IRDU)		},	/* Inverted Rate Display Unit */
		{0	,"szDCC_MPV"			,pobTran->srBRec.szDCC_MPV			,sizeof(pobTran->srBRec.szDCC_MPV)		},	/* Markup Percentage Value */
		{0	,"szDCC_MPDP"			,pobTran->srBRec.szDCC_MPDP			,sizeof(pobTran->srBRec.szDCC_MPDP)		},	/* Markup Percentage Decimal Point */
		{0	,"szDCC_CVCN"			,pobTran->srBRec.szDCC_CVCN			,sizeof(pobTran->srBRec.szDCC_CVCN)		},	/* Commissino Value Currency Number */
		{0	,"szDCC_CVCA"			,pobTran->srBRec.szDCC_CVCA			,sizeof(pobTran->srBRec.szDCC_CVCA)		},	/* Commission Value Currency Amount */
		{0	,"szDCC_CVCMU"		,pobTran->srBRec.szDCC_CVCMU			,sizeof(pobTran->srBRec.szDCC_CVCMU)		},	/* Commission Value Currency Minor Unit */
		{0	,"szDCC_TIPFCA"			,pobTran->srBRec.szDCC_TIPFCA			,sizeof(pobTran->srBRec.szDCC_TIPFCA)		},	/* Tip Foreign Currency Amount */
		{0	,"szDCC_OTD"			,pobTran->srBRec.szDCC_OTD			,sizeof(pobTran->srBRec.szDCC_OTD)		},	/* Original Transaction Date & Time (MMDD) */
		{0	,"szDCC_OTA"			,pobTran->srBRec.szDCC_OTA			,sizeof(pobTran->srBRec.szDCC_OTA)		},	/* Original Transaction Amount */
		{0	,"szProductCode"		,pobTran->srBRec.szProductCode			,sizeof(pobTran->srBRec.szProductCode)		},	/* 產品代碼 */
		{0	,"szAwardNum"			,pobTran->srBRec.szAwardNum			,sizeof(pobTran->srBRec.szAwardNum)		},	/* 優惠個數 */
		{0	,"szAwardSN"			,pobTran->srBRec.szAwardSN			,sizeof(pobTran->srBRec.szAwardSN)		},	/* 優惠序號(Award S/N) TID(8Bytes)+YYYYMMDDhhmmss(16 Bytes)，共22Bytes */
		{0	,"szTxnNo"				,pobTran->srBRec.szTxnNo				,sizeof(pobTran->srBRec.szTxnNo)		},	/* 交易編號 */
		{0	,"szMCP_BANKID"		,pobTran->srBRec.szMCP_BANKID		,sizeof(pobTran->srBRec.szMCP_BANKID)		},	/* 行動支付標記 金融機構代碼 */
		{0	,"szPayItemCode"		,pobTran->srBRec.szPayItemCode			,sizeof(pobTran->srBRec.szPayItemCode)		},	/* 繳費項目代碼 */
		{0	,"szTableTD_Data"		,pobTran->srBRec.szTableTD_Data		,sizeof(pobTran->srBRec.szTableTD_Data)		},	/* Table TD的資料， */
		{0	,"szDFSTraceNum"		,pobTran->srBRec.szDFSTraceNum		,sizeof(pobTran->srBRec.szDFSTraceNum)		},	/* DFS交易系統追蹤號 */
		{0	,"szEI_FLAG"			,pobTran->srBRec.szEI_FLAG			,sizeof(pobTran->srBRec.szEI_FLAG)	},	/* WAVE 使用用於組電文 Field_22 */
		{0	,"szEI_BankId"			,pobTran->srBRec.szEI_BankId			,sizeof(pobTran->srBRec.szEI_BankId)	},	/* WAVE 使用用於組電文 Field_22 */
		{0	,"uszWAVESchemeID"		,&pobTran->srBRec.uszWAVESchemeID	,sizeof(pobTran->srBRec.uszWAVESchemeID)	},	/* WAVE 使用用於組電文 Field_22 */
		{0	,"uszVOIDBit"			,&pobTran->srBRec.uszVOIDBit			,sizeof(pobTran->srBRec.uszVOIDBit)		},	/* 負向交易 */
		{0	,"uszUpload1Bit"			,&pobTran->srBRec.uszUpload1Bit		,sizeof(pobTran->srBRec.uszUpload1Bit)		},	/* Offline交易使用 (原交易advice是否已上傳)*/
		{0	,"uszUpload2Bit"			,&pobTran->srBRec.uszUpload2Bit		,sizeof(pobTran->srBRec.uszUpload2Bit)		},	/* Offline交易使用 (當前交易是否為advice)*/
		{0	,"uszUpload3Bit"			,&pobTran->srBRec.uszUpload3Bit		,sizeof(pobTran->srBRec.uszUpload3Bit)		},	/* Offline交易使用 */
		{0	,"uszReferralBit"			,&pobTran->srBRec.uszReferralBit		,sizeof(pobTran->srBRec.uszReferralBit)		},	/* ISO Response Code 【01】【02】使用 */
		{0	,"uszOfflineBit"			,&pobTran->srBRec.uszOfflineBit			,sizeof(pobTran->srBRec.uszOfflineBit)		},	/* 離線交易 */
		{0	,"uszManualBit"			,&pobTran->srBRec.uszManualBit		,sizeof(pobTran->srBRec.uszManualBit)		},	/* Manual Keyin */
		{0	,"uszNoSignatureBit"		,&pobTran->srBRec.uszNoSignatureBit		,sizeof(pobTran->srBRec.uszNoSignatureBit)	},	/* 免簽名使用 (免簽名則為true)*/
		{0	,"uszCUPTransBit"		,&pobTran->srBRec.uszCUPTransBit		,sizeof(pobTran->srBRec.uszCUPTransBit)		},	/* 是否為CUP */
		{0	,"uszFiscTransBit"		,&pobTran->srBRec.uszFiscTransBit		,sizeof(pobTran->srBRec.uszFiscTransBit)	},	/* SmartPay交易，是否為金融卡 */
		{0	,"uszContactlessBit"		,&pobTran->srBRec.uszContactlessBit		,sizeof(pobTran->srBRec.uszContactlessBit)	},	/* 是否為非接觸式 */
		{0	,"uszEMVFallBackBit"		,&pobTran->srBRec.uszEMVFallBackBit		,sizeof(pobTran->srBRec.uszEMVFallBackBit)	},	/* 是否要啟動fallback */
		{0	,"uszInstallmentBit"		,&pobTran->srBRec.uszInstallmentBit		,sizeof(pobTran->srBRec.uszInstallmentBit)	},	/* Installment */
		{0	,"uszRedeemBit"			,&pobTran->srBRec.uszRedeemBit		,sizeof(pobTran->srBRec.uszRedeemBit)		},	/* Redemption */
		{0	,"uszForceOnlineBit"		,&pobTran->srBRec.uszForceOnlineBit		,sizeof(pobTran->srBRec.uszForceOnlineBit)	},	/* 組電文使用 Field_25 Point of Service Condition Code */
		{0	,"uszMail_OrderBit"		,&pobTran->srBRec.uszMail_OrderBit		,sizeof(pobTran->srBRec.uszMail_OrderBit)	},	/* 組電文使用 Field_25 Point of Service Condition Code */
		{0	,"uszDCCTransBit"		,&pobTran->srBRec.uszDCCTransBit		,sizeof(pobTran->srBRec.uszDCCTransBit)		},	/* 是否為DCC交易 */
		{0	,"uszNCCCDCCRateBit"	,&pobTran->srBRec.uszNCCCDCCRateBit	,sizeof(pobTran->srBRec.uszNCCCDCCRateBit)	},
		{0	,"uszCVV2Bit"			,&pobTran->srBRec.uszCVV2Bit			,sizeof(pobTran->srBRec.uszCVV2Bit)		},
		{0	,"uszRewardSuspendBit"	,&pobTran->srBRec.uszRewardSuspendBit	,sizeof(pobTran->srBRec.uszRewardSuspendBit)	},
		{0	,"uszRewardL1Bit"		,&pobTran->srBRec.uszRewardL1Bit		,sizeof(pobTran->srBRec.uszRewardL1Bit)		},
		{0	,"uszRewardL2Bit"		,&pobTran->srBRec.uszRewardL2Bit		,sizeof(pobTran->srBRec.uszRewardL2Bit)		},
		{0	,"uszRewardL5Bit"		,&pobTran->srBRec.uszRewardL5Bit		,sizeof(pobTran->srBRec.uszRewardL5Bit)		},	/* 要印L5 */
		{0	,"uszField24NPSBit"		,&pobTran->srBRec.uszField24NPSBit		,sizeof(pobTran->srBRec.uszField24NPSBit)	},
		{0	,"uszVEPS_SignatureBit"	,&pobTran->srBRec.uszVEPS_SignatureBit	,sizeof(pobTran->srBRec.uszVEPS_SignatureBit)	},	/* VEPS 免簽名是否成立 */
		{0	,"uszTCUploadBit"		,&pobTran->srBRec.uszTCUploadBit		,sizeof(pobTran->srBRec.uszTCUploadBit)		},	/* TCUpload是否已上傳 */
		{0	,"uszFiscConfirmBit"		,&pobTran->srBRec.uszFiscConfirmBit		,sizeof(pobTran->srBRec.uszFiscConfirmBit)	},	/* SmartPay 0220 是否已上傳 */
		{0	,"uszFiscVoidConfirmBit"	,&pobTran->srBRec.uszFiscVoidConfirmBit		,sizeof(pobTran->srBRec.uszFiscVoidConfirmBit)	},	/* SmartPay Void 0220 是否已上傳 */
		{0	,"uszESCMerchantCopyBit"	,&pobTran->srBRec.uszESCMerchantCopyBit		,sizeof(pobTran->srBRec.uszESCMerchantCopyBit)	},	/*  補欄位 2020/1/10 下午 2:34 [SAM] */
		{0	,"uszPinEnterBit"			,&pobTran->srBRec.uszPinEnterBit			,sizeof(pobTran->srBRec.uszPinEnterBit)		},	/* 此筆交易是否有鍵入密碼(只能確認原交易，若該筆之後的調整或取消不會將此Bit Off) */
		{0	,"uszL2PrintADBit"		,&pobTran->srBRec.uszL2PrintADBit		,sizeof(pobTran->srBRec.uszL2PrintADBit)	},	/* L2是否印AD，因L2原交易取消要判斷，只好增加 */
		{0	,"uszInstallment"		,&pobTran->srBRec.uszInstallment		,sizeof(pobTran->srBRec.uszInstallment)		},	/* HappyGo分期交易 */
		{0	,"uszRedemption"		,&pobTran->srBRec.uszRedemption		,sizeof(pobTran->srBRec.uszRedemption)		},	/* HappyGo點數兌換 */
		{0	,"uszHappyGoSingle"		,&pobTran->srBRec.uszHappyGoSingle		,sizeof(pobTran->srBRec.uszHappyGoSingle)	},	/* HappyGo交易 */
		{0	,"uszHappyGoMulti"		,&pobTran->srBRec.uszHappyGoMulti		,sizeof(pobTran->srBRec.uszHappyGoMulti)	},	/* HappyGo混合交易 */
		{0	,"uszCLSBatchBit"		,&pobTran->srBRec.uszCLSBatchBit		,sizeof(pobTran->srBRec.uszCLSBatchBit)		},	/* 是否已接續上傳 */
		{0	,"uszTxNoCheckBit"		,&pobTran->srBRec.uszTxNoCheckBit		,sizeof(pobTran->srBRec.uszTxNoCheckBit)	},	/* 商店自存聯卡號遮掩開關 */
		{0	,"uszSpecial00Bit"		,&pobTran->srBRec.uszSpecial00Bit		,sizeof(pobTran->srBRec.uszSpecial00Bit)	},	/* 特殊卡別參數檔，活動代碼00表示免簽(只紀錄，主要看uszNoSignatureBit) */
		{0	,"uszSpecial01Bit"		,&pobTran->srBRec.uszSpecial01Bit		,sizeof(pobTran->srBRec.uszSpecial01Bit)	},	/* 特殊卡別參數檔，活動代碼01表示ECR回傳明碼(先決條件ECR卡號遮掩有開才做判斷) */
		{0	,"uszRefundCTLSBit"		,&pobTran->srBRec.uszRefundCTLSBit		,sizeof(pobTran->srBRec.uszRefundCTLSBit)	},	/* 用在簽單印(W) 因為送電文contactless bit已OFF轉Manual Key in */
		{0	,"uszMPASTransBit"		,&pobTran->srBRec.uszMPASTransBit		,sizeof(pobTran->srBRec.uszMPASTransBit)	},	/* 標示為小額交易 */
		{0	,"uszMPASReprintBit"		,&pobTran->srBRec.uszMPASReprintBit	,sizeof(pobTran->srBRec.uszMPASReprintBit)	},	/* 標示該小額交易是否可重印 */
		{0	,"uszMobilePayBit"		,&pobTran->srBRec.uszMobilePayBit		,sizeof(pobTran->srBRec.uszMobilePayBit)	},	/* 判斷是不是行動支付 Table NC */
		{0	,"uszF56NoSignatureBit"	,&pobTran->srBRec.uszF56NoSignatureBit	,sizeof(pobTran->srBRec.uszF56NoSignatureBit)	},	/* F56免簽名使用 (免簽名則為true)*/
		{0	,"uszReceiveCupF59"		,&pobTran->srBRec.uszReceiveCupF59		,sizeof(pobTran->srBRec.uszReceiveCupF59)	},	/* 補欄位  2020/1/10 下午 2:37 [SAM] */
		{0	,""				,NULL						,0						}	/* 這行用Null用來知道尾端在哪 */
	};

	int	i;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	for (i = 0;; i++)
	{
		/* 碰到Table底部，設定Tag數並跳出 */
		if (strlen(TABLE_BATCH_INT[i].szTag) == 0)
		{
			break;
		}

		/* pointer為空，則跳過 */
		if (TABLE_BATCH_INT[i].pTagValue == NULL)
			continue;

		/* 變數多過於原來設定的Tag數 */
		if (srAll->inIntNum == _TAG_INT_MAX_NUM_)
		{
			inDISP_DispLogAndWriteFlie("  LinkBrec Int 變數過多 [%d]", srAll->inIntNum);
			return (VS_ERROR);
		}

		/* Tag 名稱過長 */
		if (strlen(TABLE_BATCH_INT[i].szTag) > _TAG_MAX_LENGRH_)
		{
			inDISP_DispLogAndWriteFlie("  LinkBrec Int Tag 名稱過長 [%d]", strlen(TABLE_BATCH_INT[i].szTag));
			return (VS_ERROR);
		}

		/* Insert時不用塞TableID */
		if (inLinkState == _LS_INSERT_)
		{
			/* 判斷長度是因為避免相同字首比對錯誤 */
			if ((memcmp(TABLE_BATCH_INT[i].szTag, "inTableID", strlen("inTableID")) == 0) && (strlen(TABLE_BATCH_INT[i].szTag) == strlen("inTableID")))
			{
				continue;
			}
		}

		strcat(srAll->srInt[srAll->inIntNum].szTag, TABLE_BATCH_INT[i].szTag);
		srAll->srInt[srAll->inIntNum].pTagValue = TABLE_BATCH_INT[i].pTagValue;
		srAll->inIntNum++;
	}

	for (i = 0;; i++)
	{
		/* 碰到Table底部，設定Tag數並跳出 */
		if (strlen(TABLE_BATCH_INT64T[i].szTag) == 0)
		{
			break;
		}

		/* pointer為空，則跳過 */
		if (TABLE_BATCH_INT64T[i].pTagValue == NULL)
			continue;

		/* 變數多過於原來設定的Tag數 */
		if (srAll->inInt64tNum == _TAG_INT64T_MAX_NUM_)
		{
			inDISP_DispLogAndWriteFlie("  LinkBrec Int64t 變數過多 [%d]", srAll->inInt64tNum);
			return (VS_ERROR);
		}

		/* Tag 名稱過長 */
		if (strlen(TABLE_BATCH_INT64T[i].szTag) > _TAG_MAX_LENGRH_)
		{
			inDISP_DispLogAndWriteFlie("  LinkBrec Int64t Tag 名稱過長 [%d]", strlen(TABLE_BATCH_INT64T[i].szTag));
			return (VS_ERROR);
		}

		strcat(srAll->srInt64t[srAll->inInt64tNum].szTag, TABLE_BATCH_INT64T[i].szTag);
		srAll->srInt64t[srAll->inInt64tNum].pTagValue = TABLE_BATCH_INT64T[i].pTagValue;
		srAll->inInt64tNum++;
	}

	for (i = 0;; i++)
	{
		/* 碰到Table底部，設定Tag數並跳出 */
		if (strlen(TABLE_BATCH_CHAR[i].szTag) == 0)
		{
			break;
		}

		/* pointer為空，則跳過 */
		if (TABLE_BATCH_CHAR[i].pCharVariable == NULL)
			continue;

		/* 變數多過於原來設定的Tag數 */
		if (srAll->inCharNum == _TAG_CHAR_MAX_NUM_)
		{
			inDISP_DispLogAndWriteFlie("  LinkBrec Char 變數過多 [%d]", srAll->inCharNum);
			return (VS_ERROR);
		}

		/* Tag 名稱過長 */
		if (strlen(TABLE_BATCH_CHAR[i].szTag) > _TAG_MAX_LENGRH_)
		{
			inDISP_DispLogAndWriteFlie("  LinkBrec Char Tag名稱過長 [%d]", strlen(TABLE_BATCH_CHAR[i].szTag));
			return (VS_ERROR);
		}

		strcat(srAll->srChar[srAll->inCharNum].szTag, TABLE_BATCH_CHAR[i].szTag);
		srAll->srChar[srAll->inCharNum].pCharVariable = TABLE_BATCH_CHAR[i].pCharVariable;
		srAll->srChar[srAll->inCharNum].inTagValueLen = TABLE_BATCH_CHAR[i].inTagValueLen;
		srAll->inCharNum++;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * Function		: inSqlite_Table_Link_EMVRec
 * Date&Time	: 2017/3/15 上午 9:49
 * Describe		: 
 * 將pobTran變數pointer位置放到Table中(用以解決每一個function都要放一個table的問題)
 * 這邊直接把pobTran的pointer直接指到srAll(之後可能要考慮給動態記憶體)，
 * TagName因為是寫在這個Function內的Table，所以要給實體位置儲存
*/
int inSqlite_Table_Link_EMVRec(TRANSACTION_OBJECT *pobTran, SQLITE_ALL_TABLE *srAll, int inLinkState)
{
	SQLITE_INT32T_TABLE TABLE_BATCH_INT[] =
	{
		{0	,"inBatchTableID"				,&pobTran->inTableID						},	/* Table ID FOREIGN  key, sqlite table專用，用來對應Batch Table的FOREIGN Key */
		{0	,"inEMVCardDecision"				,&pobTran->srEMVRec.inEMVCardDecision				},
		{0	,"in50_APLabelLen"				,&pobTran->srEMVRec.in50_APLabelLen				},
		{0	,"in5A_ApplPanLen"				,&pobTran->srEMVRec.in5A_ApplPanLen				},
		{0	,"in5F20_CardholderNameLen"		,&pobTran->srEMVRec.in5F20_CardholderNameLen			},
		{0	,"in5F24_ExpireDateLen"			,&pobTran->srEMVRec.in5F24_ExpireDateLen			},
		{0	,"in5F2A_TransCurrCodeLen"		,&pobTran->srEMVRec.in5F2A_TransCurrCodeLen			},
		{0	,"in5F34_ApplPanSeqnumLen"		,&pobTran->srEMVRec.in5F34_ApplPanSeqnumLen			},
		{0	,"in71_IssuerScript1Len"			,&pobTran->srEMVRec.in71_IssuerScript1Len			},
		{0	,"in72_IssuerScript2Len"			,&pobTran->srEMVRec.in72_IssuerScript2Len			},
		{0	,"in82_AIPLen"					,&pobTran->srEMVRec.in82_AIPLen					},
		{0	,"in84_DFNameLen"				,&pobTran->srEMVRec.in84_DFNameLen				},
		{0	,"in8A_AuthRespCodeLen"			,&pobTran->srEMVRec.in8A_AuthRespCodeLen			},
		{0	,"in91_IssuerAuthDataLen"			,&pobTran->srEMVRec.in91_IssuerAuthDataLen			},
		{0	,"in95_TVRLen"					,&pobTran->srEMVRec.in95_TVRLen					},
		{0	,"in9A_TranDateLen"				,&pobTran->srEMVRec.in9A_TranDateLen				},
		{0	,"in9B_TSILen"					,&pobTran->srEMVRec.in9B_TSILen					},
		{0	,"in9C_TranTypeLen"				,&pobTran->srEMVRec.in9C_TranTypeLen				},
		{0	,"in9F02_AmtAuthNumLen"			,&pobTran->srEMVRec.in9F02_AmtAuthNumLen			},
		{0	,"in9F03_AmtOtherNumLen"		,&pobTran->srEMVRec.in9F03_AmtOtherNumLen			},
		{0	,"in9F08_AppVerNumICCLen"		,&pobTran->srEMVRec.in9F08_AppVerNumICCLen			},
		{0	,"in9F09_TermVerNumLen"			,&pobTran->srEMVRec.in9F09_TermVerNumLen			},
		{0	,"in9F10_IssuerAppDataLen"			,&pobTran->srEMVRec.in9F10_IssuerAppDataLen			},
		{0	,"in9F18_IssuerScriptIDLen"			,&pobTran->srEMVRec.in9F18_IssuerScriptIDLen			},
		{0	,"in9F1A_TermCountryCodeLen"		,&pobTran->srEMVRec.in9F1A_TermCountryCodeLen			},
		{0	,"in9F1E_IFDNumLen"				,&pobTran->srEMVRec.in9F1E_IFDNumLen				},
		{0	,"in9F26_ApplCryptogramLen"		,&pobTran->srEMVRec.in9F26_ApplCryptogramLen			},
		{0	,"in9F27_CIDLen"				,&pobTran->srEMVRec.in9F27_CIDLen				},
		{0	,"in9F33_TermCapabilitiesLen"		,&pobTran->srEMVRec.in9F33_TermCapabilitiesLen			},
		{0	,"in9F34_CVMLen"				,&pobTran->srEMVRec.in9F34_CVMLen				},
		{0	,"in9F35_TermTypeLen"			,&pobTran->srEMVRec.in9F35_TermTypeLen				},
		{0	,"in9F36_ATCLen"				,&pobTran->srEMVRec.in9F36_ATCLen				},
		{0	,"in9F37_UnpredictNumLen"			,&pobTran->srEMVRec.in9F37_UnpredictNumLen			},
		{0	,"in9F41_TransSeqCounterLen"			,&pobTran->srEMVRec.in9F41_TransSeqCounterLen			},
		{0	,"in9F5A_Application_Program_IdentifierLen"	,&pobTran->srEMVRec.in9F5A_Application_Program_IdentifierLen	},
		{0	,"in9F5B_ISRLen"						,&pobTran->srEMVRec.in9F5B_ISRLen				},
		{0	,"in9F63_CardProductLabelInformationLen"	,&pobTran->srEMVRec.in9F63_CardProductLabelInformationLen	},
		{0	,"in9F66_QualifiersLen"				,&pobTran->srEMVRec.in9F66_QualifiersLen			},
		{0	,"in9F6C_Card_Transaction_QualifiersLen"	,&pobTran->srEMVRec.in9F6C_Card_Transaction_QualifiersLen	},
		{0	,"in9F6E_From_Factor_IndicatorLen"		,&pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen		},
		{0	,"in9F74_TLVLen"					,&pobTran->srEMVRec.in9F74_TLVLen				},
		{0	,"inDF69_NewJspeedyModeLen"			,&pobTran->srEMVRec.inDF69_NewJspeedyModeLen			},
		{0	,"inDF8F4F_TransactionResultLen"			,&pobTran->srEMVRec.inDF8F4F_TransactionResultLen		},
		{0	,"inDFEC_FallBackIndicatorLen"			,&pobTran->srEMVRec.inDFEC_FallBackIndicatorLen			},
		{0	,"inDFED_ChipConditionCodeLen"		,&pobTran->srEMVRec.inDFED_ChipConditionCodeLen			},
		{0	,"inDFEE_TerEntryCapLen"				,&pobTran->srEMVRec.inDFEE_TerEntryCapLen			},
		{0	,"inDFEF_ReasonOnlineCodeLen"			,&pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen			},
		{0	,""						,NULL								}/* 這行用Null用來知道尾端在哪 */

	};

	SQLITE_INT64T_TABLE TABLE_BATCH_INT64T[] =
	{
		{0	,""						,NULL								}/* 這行用Null用來知道尾端在哪 */
	};

	SQLITE_CHAR_TABLE TABLE_BATCH_CHAR[] =
	{
		{0	,"usz50_APLabel"				,pobTran->srEMVRec.usz50_APLabel				,17	},
		{0	,"usz5A_ApplPan"				,pobTran->srEMVRec.usz5A_ApplPan				,11	},
		{0	,"usz5F20_CardholderName"		,pobTran->srEMVRec.usz5F20_CardholderName			,27	},
		{0	,"usz5F24_ExpireDate"				,pobTran->srEMVRec.usz5F24_ExpireDate				,5	},
		{0	,"usz5F2A_TransCurrCode"			,pobTran->srEMVRec.usz5F2A_TransCurrCode			,3	},
		{0	,"usz5F34_ApplPanSeqnum"			,pobTran->srEMVRec.usz5F34_ApplPanSeqnum			,3	},
		{0	,"usz71_IssuerScript1"				,pobTran->srEMVRec.usz71_IssuerScript1				,263	},
		{0	,"usz72_IssuerScript2"				,pobTran->srEMVRec.usz72_IssuerScript2				,263	},
		{0	,"usz82_AIP"					,pobTran->srEMVRec.usz82_AIP					,3	},
		{0	,"usz84_DF_NAME"				,pobTran->srEMVRec.usz84_DF_NAME				,17	},
		{0	,"usz8A_AuthRespCode"			,pobTran->srEMVRec.usz8A_AuthRespCode				,3	},
		{0	,"usz91_IssuerAuthData"			,pobTran->srEMVRec.usz91_IssuerAuthData				,17	},
		{0	,"usz95_TVR"					,pobTran->srEMVRec.usz95_TVR					,7	},
		{0	,"usz9A_TranDate"				,pobTran->srEMVRec.usz9A_TranDate				,5	},
		{0	,"usz9B_TSI"					,pobTran->srEMVRec.usz9B_TSI					,3	},
		{0	,"usz9C_TranType"				,pobTran->srEMVRec.usz9C_TranType				,3	},
		{0	,"usz9F02_AmtAuthNum"			,pobTran->srEMVRec.usz9F02_AmtAuthNum				,7	},
		{0	,"usz9F03_AmtOtherNum"			,pobTran->srEMVRec.usz9F03_AmtOtherNum				,7	},
		{0	,"usz9F08_AppVerNumICC"			,pobTran->srEMVRec.usz9F08_AppVerNumICC				,3	},
		{0	,"usz9F09_TermVerNum"			,pobTran->srEMVRec.usz9F09_TermVerNum				,3	},
		{0	,"usz9F10_IssuerAppData"			,pobTran->srEMVRec.usz9F10_IssuerAppData			,33	},
		{0	,"usz9F18_IssuerScriptID"			,pobTran->srEMVRec.usz9F18_IssuerScriptID			,5	},
		{0	,"usz9F1A_TermCountryCode"		,pobTran->srEMVRec.usz9F1A_TermCountryCode			,3	},
		{0	,"usz9F1E_IFDNum"				,pobTran->srEMVRec.usz9F1E_IFDNum				,9	},
		{0	,"usz9F26_ApplCryptogram"			,pobTran->srEMVRec.usz9F26_ApplCryptogram			,9	},
		{0	,"usz9F27_CID"					,pobTran->srEMVRec.usz9F27_CID					,3	},
		{0	,"usz9F33_TermCapabilities"			,pobTran->srEMVRec.usz9F33_TermCapabilities			,5	},
		{0	,"usz9F34_CVM"					,pobTran->srEMVRec.usz9F34_CVM					,5	},
		{0	,"usz9F35_TermType"				,pobTran->srEMVRec.usz9F35_TermType				,3	},
		{0	,"usz9F36_ATC"					,pobTran->srEMVRec.usz9F36_ATC					,3	},
		{0	,"usz9F37_UnpredictNum"				,pobTran->srEMVRec.usz9F37_UnpredictNum				,5	},
		{0	,"usz9F41_TransSeqCounter"			,pobTran->srEMVRec.usz9F41_TransSeqCounter			,5	},
		{0	,"usz9F5A_Application_Program_Identifier"	,pobTran->srEMVRec.usz9F5A_Application_Program_Identifier	,33	},
		{0	,"usz9F5B_ISR"						,pobTran->srEMVRec.usz9F5B_ISR					,7	},
		{0	,"usz9F63_CardProductLabelInformation"	,pobTran->srEMVRec.usz9F63_CardProductLabelInformation		,17	},
		{0	,"usz9F66_Qualifiers"					,pobTran->srEMVRec.usz9F66_Qualifiers				,5	},
		{0	,"usz9F6C_Card_Transaction_Qualifiers"		,pobTran->srEMVRec.usz9F6C_Card_Transaction_Qualifiers		,3	},
		{0	,"usz9F6E_From_Factor_Indicator"			,pobTran->srEMVRec.usz9F6E_From_Factor_Indicator		,33	},
		{0	,"usz9F74_TLV"						,pobTran->srEMVRec.usz9F74_TLV					,7	},
		{0	,"uszDF69_NewJspeedyMode"			,pobTran->srEMVRec.uszDF69_NewJspeedyMode			,3	},
		{0	,"uszDF8F4F_TransactionResult"			,pobTran->srEMVRec.uszDF8F4F_TransactionResult			,3	},
		{0	,"uszDFEC_FallBackIndicator"			,pobTran->srEMVRec.uszDFEC_FallBackIndicator			,3	},
		{0	,"uszDFED_ChipConditionCode"			,pobTran->srEMVRec.uszDFED_ChipConditionCode			,3	},
		{0	,"uszDFEE_TerEntryCap"				,pobTran->srEMVRec.uszDFEE_TerEntryCap				,3	},
		{0	,"uszDFEF_ReasonOnlineCode"			,pobTran->srEMVRec.uszDFEF_ReasonOnlineCode			,5	},

		{0	,""						,NULL								,0	}/* 這行用Null用來知道尾端在哪 */
	};

	int	i;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	for (i = 0;; i++)
	{
		/* 碰到Table底部，設定Tag數並跳出 */
		if (strlen(TABLE_BATCH_INT[i].szTag) == 0)
		{
			break;
		}

		/* pointer為空，則跳過 */
		if (TABLE_BATCH_INT[i].pTagValue == NULL)
			continue;

		/* 變數多過於原來設定的Tag數 */
		if (srAll->inIntNum == _TAG_INT_MAX_NUM_)
		{
			inDISP_DispLogAndWriteFlie("  LinkEMVRec Int 變數過多 [%d]", srAll->inIntNum);
			return (VS_ERROR);
		}

		/* Tag 名稱過長 */
		if (strlen(TABLE_BATCH_INT[i].szTag) > _TAG_MAX_LENGRH_)
		{
			inDISP_DispLogAndWriteFlie("  LinkEMVRec Int Tag 名稱過長 [%d]", strlen(TABLE_BATCH_INT[i].szTag));
			return (VS_ERROR);
		}

		strcat(srAll->srInt[srAll->inIntNum].szTag, TABLE_BATCH_INT[i].szTag);
		srAll->srInt[srAll->inIntNum].pTagValue = TABLE_BATCH_INT[i].pTagValue;
		srAll->inIntNum++;
	}

	for (i = 0;; i++)
	{
		/* 碰到Table底部，設定Tag數並跳出 */
		if (strlen(TABLE_BATCH_INT64T[i].szTag) == 0)
		{
			break;
		}

		/* pointer為空，則跳過 */
		if (TABLE_BATCH_INT64T[i].pTagValue == NULL)
			continue;

		/* 變數多過於原來設定的Tag數 */
		if (srAll->inInt64tNum == _TAG_INT64T_MAX_NUM_)
		{
			inDISP_DispLogAndWriteFlie("  LinkEMVRec Int64 變數過多 [%d]", srAll->inInt64tNum);
			return (VS_ERROR);
		}

		/* Tag 名稱過長 */
		if (strlen(TABLE_BATCH_INT64T[i].szTag) > _TAG_MAX_LENGRH_)
		{
			inDISP_DispLogAndWriteFlie("  LinkEMVRec Int64 Tag 名稱過長 [%d]", strlen(TABLE_BATCH_INT64T[i].szTag));
			return (VS_ERROR);
		}

		strcat(srAll->srInt64t[srAll->inInt64tNum].szTag, TABLE_BATCH_INT64T[i].szTag);
		srAll->srInt64t[srAll->inInt64tNum].pTagValue = TABLE_BATCH_INT64T[i].pTagValue;
		srAll->inInt64tNum++;
	}

	for (i = 0;; i++)
	{
		/* 碰到Table底部，設定Tag數並跳出 */
		if (strlen(TABLE_BATCH_CHAR[i].szTag) == 0)
		{
			break;
		}

		/* pointer為空，則跳過 */
		if (TABLE_BATCH_CHAR[i].pCharVariable == NULL)
			continue;

		/* 變數多過於原來設定的Tag數 */
		if (srAll->inCharNum == _TAG_CHAR_MAX_NUM_)
		{
			inDISP_DispLogAndWriteFlie("  LinkEMVRec Char 變數過多 [%d]", srAll->inCharNum);
			return (VS_ERROR);
		}

		/* Tag 名稱過長 */
		if (strlen(TABLE_BATCH_CHAR[i].szTag) > _TAG_MAX_LENGRH_)
		{
			inDISP_DispLogAndWriteFlie("  LinkEMVRec Char Tag 名稱過長 [%d]", strlen(TABLE_BATCH_CHAR[i].szTag));
			return (VS_ERROR);
		}

		strcat(srAll->srChar[srAll->inCharNum].szTag, TABLE_BATCH_CHAR[i].szTag);
		srAll->srChar[srAll->inCharNum].pCharVariable = TABLE_BATCH_CHAR[i].pCharVariable;
		srAll->srChar[srAll->inCharNum].inTagValueLen = TABLE_BATCH_CHAR[i].inTagValueLen;
		srAll->inCharNum++;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * Function		: inSqlite_Insert_All_Batch
 * Date&Time	: 2016/3/30 下午 5:57
 * Describe		:  指令 INSERT INTO
 * Open Database檔  insert batch Record(不含EMV Record)
*/
int inSqlite_Insert_All_Batch(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int	inRc;
	int	inBindingIndex = 1;	/* binding的index從1開始 */
	int	i;
	int	inRetVal;
	int	inSqlLength = 0;	/* 算組SQL語句的長度 */
	char	szSqlPrefix[100 + 1];	/* INSERT INTO	szTableName( */
	char	szSqlSuffix[20 + 1];	/* VALUES ( */
	char	szSqlSuffix2[10 + 1];	/* ); */
	char	szTemplate[100 + 1];	/* 因為最長有到711 */
	char	*szInsertSql;
	sqlite3	*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt	*srSQLStat;
	SQLITE_ALL_TABLE	srAll;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Insert All Batch  Open V2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  Table Name [%s] Line[%d]", szTableName, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		inDISP_LogPrintfWithFlag("Open Database File OK");
		st_inSqlitHandleCnt ++;
	}

	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_BRec(pobTran, &srAll, _LS_INSERT_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Sq Insert All Batch  LinkBRec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq Insert All Batch Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}

		return (VS_ERROR);
	}

	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "INSERT INTO %s (", szTableName);

	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, ")VALUES (");

	memset(szSqlSuffix2, 0x00, sizeof(szSqlSuffix2));
	sprintf(szSqlSuffix2, ");");

	/* 算要配置多少記憶體 */
	inSqlLength += strlen(szSqlPrefix);

	/* LogPort Debug */
	inDISP_LogPrintfWithFlag("COUNT");
	inDISP_LogPrintfWithFlag("IntTag: %d", srAll.inIntNum);
	inDISP_LogPrintfWithFlag("Int64tTag: %d", srAll.inInt64tNum);
	inDISP_LogPrintfWithFlag("CharTag: %d", srAll.inCharNum);

	for (i = 0; i < srAll.inIntNum; i++)
	{
		inSqlLength += strlen(srAll.srInt[i].szTag);
		/* Comma */
		inSqlLength += 2;
	}

	for (i = 0; i < srAll.inInt64tNum; i++)
	{
		inSqlLength += strlen(srAll.srInt64t[i].szTag);
		/* Comma */
		inSqlLength += 2;
	}

	for (i = 0; i < srAll.inCharNum; i++)
	{
		inSqlLength += strlen(srAll.srChar[i].szTag);
		/* Comma */
		inSqlLength += 2;
	}

	/* 第一行最後面的) */
	inSqlLength ++;
	/* 第二行"VALUES ("的長度 */
	inSqlLength += strlen(szSqlSuffix);

	for (i = 0; i < srAll.inIntNum; i++)
	{
		/* 用問號 */
		inSqlLength += 1;
		/* Comma & Space */
		inSqlLength += 2;
	}

	for (i = 0; i < srAll.inInt64tNum; i++)
	{
		/* 用問號 */
		inSqlLength += 1;
		/* Comma & Space */
		inSqlLength += 2;
	}

	for (i = 0; i < srAll.inCharNum; i++)
	{
		/* 用問號 */
		inSqlLength += 1;
		/* Comma & Space & 兩個單引號 */
		inSqlLength += 2;
	}

	/* ); */
	inSqlLength += strlen(szSqlSuffix2);

	/* LogPort Debug */
	inDISP_LogPrintfWithFlag("inSqlLength: %d", inSqlLength);

	/* 配置記憶體 */
	szInsertSql = malloc(inSqlLength + 100);
	memset(szInsertSql, 0x00, inSqlLength + 100);

	/* 先丟Table Name */
	strcat(szInsertSql, szSqlPrefix);

	/* 看上一個table是不是空的，有東西的話，第一項前面要加comma，但這已經是第一個table，所以放0 */
	if (0 > 0)
	{
		strcat(szInsertSql, ", ");
	}

	for (i = 0; i < srAll.inIntNum; i++)
	{
		if (i > 0)
		{
			strcat(szInsertSql, ", ");
		}
		strcat(szInsertSql, srAll.srInt[i].szTag);
	}

	/* 看上一個table是不是空的，有東西的話，第一項前面要加comma */
	if (srAll.inIntNum > 0)
	{
		strcat(szInsertSql, ", ");
	}

	for (i = 0; i < srAll.inInt64tNum; i++)
	{
		if (i > 0)
		{
			strcat(szInsertSql, ", ");
		}
		strcat(szInsertSql, srAll.srInt64t[i].szTag);
	}

	/* 看上一個table是不是空的，有東西的話，第一項前面要加comma */
	if (srAll.inInt64tNum > 0)
	{
		strcat(szInsertSql, ", ");
	}

	for (i = 0; i < srAll.inCharNum; i++)
	{

		if (i > 0)
		{
			strcat(szInsertSql, ", ");
		}
		strcat(szInsertSql, srAll.srChar[i].szTag);
	}

	/* 為了避免有空的table導致多塞, */
	if (memcmp((szInsertSql + strlen(szInsertSql) - 2), ", ", 2) == 0)
	{
		memset(szInsertSql + strlen(szInsertSql) - 2, 0x00, 2);
	}

	/* ")VALUES (" */
	strcat(szInsertSql, szSqlSuffix);
	memset(szTemplate, 0x00, sizeof(szTemplate));

	for (i = 0; i < srAll.inIntNum; i++)
	{
		if (i == 0)
		{
			sprintf(szTemplate, "?");
		}
		else
		{
			sprintf(szTemplate, ", ?");
		}
		strcat(szInsertSql, szTemplate);
		/* 只清自己用過得長度 comma(1) + space(1) + ?(1) */
		memset(szTemplate, 0x00, 3);
	}

	/* 代表上一個table有東西，要加comma */
	if (srAll.inIntNum > 0)
	{
		strcat(szInsertSql, ", ");
	}

	/* LogPort Debug */
	inDISP_LogPrintfWithFlag("Int Insert OK");
	
	for (i = 0; i < srAll.inInt64tNum; i++)
	{
		if (i == 0)
		{
			sprintf(szTemplate, "?");
		}
		else
		{
			sprintf(szTemplate, ", ?");
		}
		strcat(szInsertSql, szTemplate);
		/* 只清自己用過得長度 comma(1) + space(1) + ?(1) */
		memset(szTemplate, 0x00, 3);
	}

	/* 代表上一個table有東西，要加comma */
	if (srAll.inInt64tNum > 0)
	{
		strcat(szInsertSql, ", ");
	}

	inDISP_LogPrintfWithFlag("Int64t Insert OK");

	for (i = 0; i < srAll.inCharNum; i++)
	{
		if (i == 0)
		{
			sprintf(szTemplate, "?");
		}
		else
		{
			sprintf(szTemplate, ", ?");
		}
		strcat(szInsertSql, szTemplate);
		/* 只清自己用過得長度 comma(1) + space(1) + ?(1) */
		memset(szTemplate, 0x00, 3);
	}

	/* 代表上一個table有東西，要加comma */
	if (srAll.inCharNum > 0)
	{
		strcat(szInsertSql, ", ");
	}

	inDISP_LogPrintfWithFlag("Char Insert OK");

	/* 為了避免有空的table導致多塞, */
	if (memcmp((szInsertSql + strlen(szInsertSql) - 2), ", ", 2) == 0)
	{
		memset(szInsertSql + strlen(szInsertSql) - 2, 0x00, 2);
	}

	/* 最後面的); */
	strcat(szInsertSql, szSqlSuffix2);

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szInsertSql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Insert All Batch PrepareV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szInsertSql[%s] Line[%d]",szInsertSql ,  __LINE__);
	}

	/* Binding變數 */
	for (i = 0; i < srAll.inIntNum; i++)
	{
		inRetVal = sqlite3_bind_int(srSQLStat, inBindingIndex, *(int32_t*)srAll.srInt[i].pTagValue);
		if (inRetVal != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq Insert All Batch  Bindint *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
			inDISP_DispLogAndWriteFlie("  inBindingIndex[%d] inIntNum[%d] Line[%d]",inBindingIndex , srAll.inIntNum,  __LINE__);
		}
		else
		{
			inBindingIndex++;
		}

	}

	for (i = 0; i < srAll.inInt64tNum; i++)
	{
		inRetVal = sqlite3_bind_int64(srSQLStat, inBindingIndex, *(int64_t*)srAll.srInt64t[i].pTagValue);
		if (inRetVal != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq Insert All Batch Bind64 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
			inDISP_DispLogAndWriteFlie("  inBindingIndex[%d] inInt64tNum[%d] Line[%d]",inBindingIndex , srAll.inInt64tNum,  __LINE__);
		}
		else
		{
			inBindingIndex++;
		}
	}

	for (i = 0; i < srAll.inCharNum; i++)
	{
		inRetVal = sqlite3_bind_blob(srSQLStat, inBindingIndex, srAll.srChar[i].pCharVariable, srAll.srChar[i].inTagValueLen, NULL);
		if (inRetVal != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq Insert All Batch Bindblob *Error* HDT[%d] Rvl[%d] Line[%d]", pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
			inDISP_DispLogAndWriteFlie("  inBindingIndex[%d] inCharNum[%d] Line[%d]", inBindingIndex, srAll.inCharNum, __LINE__);
		}	
		else
		{
			inBindingIndex++;
		}
	}

	do
	{
		/* Insert */
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW	||
		    inRetVal == SQLITE_DONE)
		{
			inDISP_LogPrintfWithFlag("Insert OK ");
		}
		else
		{
			inDISP_DispLogAndWriteFlie("  Sq Insert All Batch Step *Error* HDT[%d] Rvl[%d] Line[%d]", pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
			inDISP_DispLogAndWriteFlie("  Reason[%s] Line[%d]", sqlite3_errmsg(srDBConnection), __LINE__);
			inDISP_DispLogAndWriteFlie("  Commd[%s] Line[%d]", szInsertSql, __LINE__);
		}

	}while (inRetVal == SQLITE_ROW);

	/* 釋放事務，若要重用則用sqlite3_reset */
	inRc = sqlite3_finalize(srSQLStat);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Insert All Batch  finalize inRc[%d] *Error* Line[%d]", inRc, __LINE__);
	}

	free(szInsertSql);

	/* 關閉 database, close null pointer 是NOP(No Operation) */
	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Insert All Batch Close inRc[%d] *Error* Line[%d]", inRc, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		inDISP_LogPrintfWithFlag("Close Database OK");
		st_inSqlitHandleCnt --;
	}
	
	/* inRetVal 這個需要保留，因為是  sqlite3_step 執行失敗 */
	if (inRetVal == SQLITE_ERROR)
	{
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] Fail[%d] END -----",__FILE__, __FUNCTION__, __LINE__, inRetVal);
		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * Function		: inSqlite_Insert_All_EMV
 * Date&Time	: 2017/3/15 上午 9:47
 * Describe		:
*/
int inSqlite_Insert_All_EMV(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int	inRc;
	int	inBindingIndex = 1;	/* binding的index從1開始 */
	int	i;
	int	inRetVal;
	int	inSqlLength = 0;	/* 算組SQL語句的長度 */
	char	szDebugMsg[84 + 1];
	char	szSqlPrefix[100 + 1];	/* INSERT INTO	szTableName( */
	char	szSqlSuffix[20 + 1];	/* VALUES ( */
	char	szSqlSuffix2[10 + 1];	/* ); */
	char	szTemplate[300 + 1];	/* 因為最長有到266 */
	char	*szInsertSql;
	sqlite3	*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt	*srSQLStat;
	SQLITE_ALL_TABLE	srAll;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Emv Insert All  Open V2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] Line[%d]", szTableName, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		inDISP_LogPrintfWithFlag("Open Database File OK");
		st_inSqlitHandleCnt ++;
	}

	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_EMVRec(pobTran, &srAll, _LS_INSERT_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Sq Emv Insert All  LinkBRec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq Emv Insert All Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
		return (VS_ERROR);
	}

	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "INSERT INTO %s (", szTableName);

	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, ")VALUES (");

	memset(szSqlSuffix2, 0x00, sizeof(szSqlSuffix2));
	sprintf(szSqlSuffix2, ");");

	/* 算要配置多少記憶體 */
	inSqlLength += strlen(szSqlPrefix);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("COUNT");

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "IntTag: %d", srAll.inIntNum);
		inDISP_LogPrintf(szDebugMsg);

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Int64tTag: %d", srAll.inInt64tNum);
		inDISP_LogPrintf(szDebugMsg);

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "CharTag: %d", srAll.inCharNum);
		inDISP_LogPrintf(szDebugMsg);
	}

	for (i = 0; i < srAll.inIntNum; i++)
	{
		inSqlLength += strlen(srAll.srInt[i].szTag);
		/* Comma */
		inSqlLength += 2;
	}

	for (i = 0; i < srAll.inInt64tNum; i++)
	{
		inSqlLength += strlen(srAll.srInt64t[i].szTag);
		/* Comma */
		inSqlLength += 2;
	}

	for (i = 0; i < srAll.inCharNum; i++)
	{
		inSqlLength += strlen(srAll.srChar[i].szTag);
		/* Comma */
		inSqlLength += 2;
	}

	/* 第一行最後面的) */
	inSqlLength ++;
	/* 第二行"VALUES ("的長度 */
	inSqlLength += strlen(szSqlSuffix);

	for (i = 0; i < srAll.inIntNum; i++)
	{
		/* 用問號 */
		inSqlLength += 1;
		/* Comma & Space */
		inSqlLength += 2;
	}

	for (i = 0; i < srAll.inInt64tNum; i++)
	{
		/* 用問號 */
		inSqlLength += 1;
		/* Comma & Space */
		inSqlLength += 2;
	}

	for (i = 0; i < srAll.inCharNum; i++)
	{
		/* 用問號 */
		inSqlLength += 1;
		/* Comma & Space */
		inSqlLength += 2;
	}

	/* ); */
	inSqlLength += strlen(szSqlSuffix2);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", inSqlLength);
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 配置記憶體 */
	szInsertSql = malloc(inSqlLength + 100);
	memset(szInsertSql, 0x00, inSqlLength + 100);

	/* 先丟Table Name */
	strcat(szInsertSql, szSqlPrefix);

	/* 看上一個table是不是空的，有東西的話，第一項前面要加comma，但這已經是第一個table，所以放0 */
	if (0 > 0)
	{
		strcat(szInsertSql, ", ");
	}

	for (i = 0; i < srAll.inIntNum; i++)
	{
		if (i > 0)
		{
			strcat(szInsertSql, ", ");
		}
		strcat(szInsertSql, srAll.srInt[i].szTag);
	}

	/* 看上一個table是不是空的，有東西的話，第一項前面要加comma */
	if (srAll.inIntNum > 0)
	{
		strcat(szInsertSql, ", ");
	}

	for (i = 0; i < srAll.inInt64tNum; i++)
	{
		if (i > 0)
		{
			strcat(szInsertSql, ", ");
		}
		strcat(szInsertSql, srAll.srInt64t[i].szTag);
	}

	/* 看上一個table是不是空的，有東西的話，第一項前面要加comma */
	if (srAll.inInt64tNum > 0)
	{
		strcat(szInsertSql, ", ");
	}

	for (i = 0; i < srAll.inCharNum; i++)
	{

		if (i > 0)
		{
			strcat(szInsertSql, ", ");
		}
		strcat(szInsertSql, srAll.srChar[i].szTag);
	}

	/* 為了避免有空的table導致多塞, */
	if (memcmp((szInsertSql + strlen(szInsertSql) - 2), ", ", 2) == 0)
	{
		memset(szInsertSql + strlen(szInsertSql) - 2, 0x00, 2);
	}

	/* ")VALUES (" */
	strcat(szInsertSql, szSqlSuffix);
	memset(szTemplate, 0x00, sizeof(szTemplate));

	for (i = 0; i < srAll.inIntNum; i++)
	{
		if (i == 0)
		{
			sprintf(szTemplate, "?");
		}
		else
		{
			sprintf(szTemplate, ", ?");
		}
		strcat(szInsertSql, szTemplate);
		/* 只清自己用過得長度 comma(1) + space(1) + ?(1) */
		memset(szTemplate, 0x00, 3);
	}

	/* 代表上一個table有東西，要加comma */
	if (srAll.inIntNum > 0)
	{
		strcat(szInsertSql, ", ");
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Int Insert OK");
		inDISP_LogPrintf(szDebugMsg);
	}

	for (i = 0; i < srAll.inInt64tNum; i++)
	{
		if (i == 0)
		{
			sprintf(szTemplate, "?");
		}
		else
		{
			sprintf(szTemplate, ", ?");
		}
		strcat(szInsertSql, szTemplate);
		/* 只清自己用過得長度 comma(1) + space(1) + ?(1) */
		memset(szTemplate, 0x00, 3);
	}

	/* 代表上一個table有東西，要加comma */
	if (srAll.inInt64tNum > 0)
	{
		strcat(szInsertSql, ", ");
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Int64t Insert OK");
		inDISP_LogPrintf(szDebugMsg);
	}

	for (i = 0; i < srAll.inCharNum; i++)
	{
		if (i == 0)
		{
			sprintf(szTemplate, "?");
		}
		else
		{
			sprintf(szTemplate, ", ?");
		}
		strcat(szInsertSql, szTemplate);
		/* 只清自己用過得長度 comma(1) + space(1) + ?(1) */
		memset(szTemplate, 0x00, 3);
	}

	/* 代表上一個table有東西，要加comma */
	if (srAll.inCharNum > 0)
	{
		strcat(szInsertSql, ", ");
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Char Insert OK");
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 為了避免有空的table導致多塞, */
	if (memcmp((szInsertSql + strlen(szInsertSql) - 2), ", ", 2) == 0)
	{
		memset(szInsertSql + strlen(szInsertSql) - 2, 0x00, 2);
	}

	/* 最後面的); */
	strcat(szInsertSql, szSqlSuffix2);

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szInsertSql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Emv Insert All  PrepareV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szInsertSql[%s] Line[%d]",szInsertSql ,  __LINE__);
	}

	/* Binding變數 */
	for (i = 0; i < srAll.inIntNum; i++)
	{
		inRetVal = sqlite3_bind_int(srSQLStat, inBindingIndex, *(int32_t*)srAll.srInt[i].pTagValue);
		if (inRetVal != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq Emv Insert All  Bindint *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
			inDISP_DispLogAndWriteFlie("  inBindingIndex[%d] inIntNum[%d] Line[%d]",inBindingIndex , srAll.inIntNum,  __LINE__);
		}
		else
		{
			inBindingIndex++;
		}

	}

	for (i = 0; i < srAll.inInt64tNum; i++)
	{
		inRetVal = sqlite3_bind_int64(srSQLStat, inBindingIndex, *(int64_t*)srAll.srInt64t[i].pTagValue);
		if (inRetVal != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq Emv Insert All  Bind64 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
			inDISP_DispLogAndWriteFlie("  inBindingIndex[%d] inInt64tNum[%d] Line[%d]",inBindingIndex , srAll.inInt64tNum,  __LINE__);
		}
		else
		{
			inBindingIndex++;
		}
	}

	for (i = 0; i < srAll.inCharNum; i++)
	{
		inRetVal = sqlite3_bind_blob(srSQLStat, inBindingIndex, srAll.srChar[i].pCharVariable, srAll.srChar[i].inTagValueLen, NULL);
		if (inRetVal != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq Emv Insert All  Bindblob *Error* HDT[%d] Rvl[%d] Line[%d]", pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
			inDISP_DispLogAndWriteFlie("  inBindingIndex[%d] inCharNum[%d] Line[%d]", inBindingIndex, srAll.inCharNum, __LINE__);
		}
		else
		{
			inBindingIndex++;
		}
	}

	do
	{
		/* Insert */
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW	||
		    inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Insert OK");
				inDISP_LogPrintf(szDebugMsg);
			}
		}
		else
		{
			inDISP_DispLogAndWriteFlie("  Sq Emv Insert All  Step *Error* HDT[%d] Rvl[%d] Line[%d]", pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
			inDISP_DispLogAndWriteFlie("  Reason[%s] Line[%d]", sqlite3_errmsg(srDBConnection), __LINE__);
			inDISP_DispLogAndWriteFlie("  Commd[%s] Line[%d]", szInsertSql, __LINE__);
		}

	}while (inRetVal == SQLITE_ROW);

	/* 釋放事務，若要重用則用sqlite3_reset */
	inRc = sqlite3_finalize(srSQLStat);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Emv Insert All Step finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
	}

	free(szInsertSql);

	/* 關閉 database, close null pointer 是NOP(No Operation) */
	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Emv Insert All Step Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt --;
	}
	
	/* inRetVal 這個需要保留，因為是  sqlite3_step 執行失敗 */
	if (inRetVal == SQLITE_ERROR)
	{
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] Fail[%d] END -----",__FILE__, __FUNCTION__, __LINE__, inRetVal);
		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_TagValue_ByInvoiceNumber_Flow
Date&Time       :2017/3/20 下午 5:57
Describe        :在這邊決定名稱並分流
*/
int inSqlite_Get_TagValue_ByInvoiceNumber_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inInvoiceNumber, char *szTagName, char *szTagValue)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		default :
			inDISP_DispLogAndWriteFlie("  TbType Not Found[%d] Line[%d]", inTableType, __LINE__);
			break;
	}

	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_EMV_TABLE_:
			inRetVal = inSqlite_Get_TagValue_ByInvoiceNumber(pobTran, szTableName, inInvoiceNumber, szTagName, szTagValue);
			break;
		default :
			inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] TbType[%d] END -----",__FILE__, __FUNCTION__, __LINE__, inTableType);
			return (VS_ERROR);
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
Function        :inSqlite_Get_TagValue_ByInvoiceNumber
Date&Time       :2016/4/11 下午 3:25
Describe        :利用調閱標號取得指定的Tag，輸入當筆invoiceNumber、Tag名稱的string，然後輸入一個陣列szTagValue來接值
*/
int inSqlite_Get_TagValue_ByInvoiceNumber(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber, char *szTagName, char *szTagValue)
{
	int	inRc;
	int	j = 0;
	int	inRetVal = VS_ERROR;
	int	inDataLen = 0;
	char	szDebugMsg[84 + 1];
	char	szQuerySql[300 + 1];	/* INSERT INTO	szTableName( */
	sqlite3_stmt	*srSQLStat;
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetTagValueByInv OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] Inv[%d] Line[%d]", szTableName, inInvoiceNumber, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTagName[%s] szTagValue[%s] Line[%d]", szTagName, szTagValue, __LINE__);

		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}

	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "SELECT %s FROM %s WHERE lnOrgInvNum = %d", szTagName, szTableName, inInvoiceNumber);

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetTagValueByInv PrepareV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql, __LINE__);
	}

	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inDISP_LogPrintf(szDebugMsg);
		}
		
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetTagValueByInv Step finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}
	
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetTagValueByInv Step Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
		
		return (VS_NO_RECORD);
	}
	else
	{
		inDISP_DispLogAndWriteFlie("  Sq GetTagValue  Step *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  Reason[%s] Line[%d]", sqlite3_errmsg(srDBConnection), __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql, __LINE__);

	
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetTagValueByInv Step finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}
	
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetTagValueByInv Step Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}

		return (VS_ERROR);
	}

	/* 若是成功，將值丟到輸入的位置。 */
	/* 應該只會抓到一列 */
	j = 0;

	inDataLen = sqlite3_column_bytes(srSQLStat, j);
	memcpy(szTagValue, sqlite3_column_blob(srSQLStat, j), inDataLen);

	/* 釋放事務 */
	inRc = sqlite3_finalize(srSQLStat);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetTagValueByInv Step finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
	}

	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetTagValueByInv Step Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Table_Flow
Date&Time       :2017/3/14 下午 3:27
Describe        :
*/
int inSqlite_Get_Table_Count_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int *inTableCount)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ADVICE_SUFFIX_);
		default :
			inDISP_DispLogAndWriteFlie("  TableType [%d] Not Found Line[%d]", inTableType, __LINE__);
			break;
	}

	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_EMV_TABLE_:
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			inRetVal = inSqlite_Get_Table_Count(pobTran, szTableName, inTableCount);
			break;
		default :
			inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] TbTp[%d] END -----",__FILE__, __FUNCTION__, __LINE__, inTableType);
			return (VS_ERROR);
	}
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
Function        :inSqlite_Get_Table_Count
Date&Time       :2016/4/29 下午 1:17
Describe        :可以取得table有幾筆資料
*/
int inSqlite_Get_Table_Count(TRANSACTION_OBJECT *pobTran, char *szTableName, int *inTableCount)
{
	int	inRc;
	int	j = 0;
	int	inRetVal;
	char	szDebugMsg[128 + 1];
	char	szQuerySql[300 + 1];	/* INSERT INTO	szTableName( */
	char	szErrorMessage[100 + 1];
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt	*srSQLStat;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetTabCount OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] inTableCount[%d] Line[%d]", szTableName, inTableCount, __LINE__);
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}
	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));

	/* SQLite COUNT 計算資料庫中 table的行數 */
	sprintf(szQuerySql, "SELECT count(*) FROM %s", szTableName);

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetTabCount PrepareV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql,  __LINE__);
	}

	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inDISP_LogPrintf(szDebugMsg);
		}
		
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetTabCount Step finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}
	
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetTabCount Step Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
			
		return (VS_NO_RECORD);
	}
	else
	{
		
		memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
		strcpy(szErrorMessage, sqlite3_errmsg(srDBConnection));

		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetTabCount Step finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}
	
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetTabCount Step Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}

		/* 因為直接get 不存在的table回傳值是-1，只有在Error Msg才能得知錯誤原因 */
		if (memcmp(szErrorMessage, "no such table", strlen("no such table")) == 0)
		{
			inDISP_DispLogAndWriteFlie(" NoSuchTable Line[%d]", __LINE__);
			inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_NO_RECORD);
		}
		else
		{
			inDISP_DispLogAndWriteFlie("  Sq GetTabCount  Step *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
			inDISP_DispLogAndWriteFlie("  Reason[%s] Line[%d]", szErrorMessage , __LINE__);
			inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
		}
	}

	/* 若是成功，將值丟到輸入的位置。 */
	j = 0;
	*inTableCount = sqlite3_column_int(srSQLStat, j);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Get count(*) OK count:%d", *inTableCount);
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 釋放事務 */
	inRc = sqlite3_finalize(srSQLStat);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetTabCount Step finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
	}

	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetTabCount Step Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Batch_ByInvNum
Date&Time       :2016/4/12 下午 2:53
Describe        :利用調閱標號來將該筆資料全塞回pobTran中的BRec、會取最新狀態(如取消、調帳)
*/
int inSqlite_Get_Batch_ByInvNum(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber)
{
	int	inRc;
	int	j = 0;
	int	inIntIndex = 0, inInt64tIndex = 0, inCharIndex = 0;
	int	inCols = 0, inDataLen = 0;
	int	inRetVal = VS_SUCCESS;
	int	inFind = VS_FALSE;
	char	szDebugMsg[128 + 1];
	char	szQuerySql[200 + 1];	/* INSERT INTO	szTableName( */
	char	szTagName[_TAG_WIDTH_ + 1];
	char	szErrorMessage[100 + 1];
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_ALL_TABLE	srAll;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatch Inv OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] Inv[%d] Line[%d]", szTableName, inInvoiceNumber, __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}

	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_BRec(pobTran, &srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatch Inv LinkBRec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] Inv[%d] Line[%d]", szTableName, inInvoiceNumber, __LINE__);
		
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq InitialSetting DB Config  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	if (inInvoiceNumber > 0)
	{
		sprintf(szQuerySql, "SELECT * FROM %s WHERE lnOrgInvNum = %d ORDER BY inTableID DESC LIMIT 1", szTableName, inInvoiceNumber);
	}
	else if (inInvoiceNumber == _BATCH_LAST_RECORD_)
	{
		/* sqlite3_last_insert_rowid 只有在同一connection才有用 所以這邊的邏輯是最後一筆理論上invoiceNumber會最大
		   若是同一筆，可能有調帳等操作，加上用max(inTableID)來判斷 */


		/* 再重新組查詢語句，把剛剛查到的invoiceNumber放進去 第一列第0行是所查的值 */
		memset(szQuerySql, 0x00, sizeof(szQuerySql));
		sprintf(szQuerySql, "SELECT * FROM %s WHERE lnOrgInvNum = (SELECT MAX(lnOrgInvNum) FROM %s) ORDER BY inTableID DESC LIMIT 1", szTableName, szTableName);
	}
	else
	{

		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBatch Inv Close *Error* inRetVal[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}

		return (VS_ERROR);
	}

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatch Inv Prepare *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql, __LINE__);
	}

	inFunc_CalculateRunTimeGlobal_Start();
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		/* 替換資料前先清空srBRec */
		memset(&pobTran->srBRec, 0x00, sizeof(pobTran->srBRec));
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inDISP_LogPrintf(szDebugMsg);
		}
		
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBatch Inv  Step finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}
	
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBatch Inv  Step Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_NO_RECORD);
	}
	else
	{
		memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
		strcpy(szErrorMessage, sqlite3_errmsg(srDBConnection));

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
			inDISP_LogPrintf(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", szErrorMessage);
			inDISP_LogPrintf(szDebugMsg);

			inDISP_LogPrintf(szQuerySql);
		}

		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBatch Inv  Step finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}
	
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBatch Inv  Step Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}

		/* 因為直接get 不存在的table回傳值是-1，只有在Error Msg才能得知錯誤原因 */
		if (memcmp(szErrorMessage, "no such table", strlen("no such table")) == 0)
		{
			inDISP_DispLogAndWriteFlie("  No Such Table Line[%d] ", __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_NO_RECORD);
		}
		else
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBatch Inv  Step *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
			inDISP_DispLogAndWriteFlie("  Reason[%s] Line[%d]", szErrorMessage , __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
		}
	}

	inCols = sqlite3_column_count(srSQLStat);

	/* binding 取得值 */
	for (j = 0; j < inCols; j++)
	{
		inFind = VS_FALSE;
		memset(szTagName, 0x00, sizeof(szTagName));
		strcat(szTagName, sqlite3_column_name(srSQLStat, j));

		for (inIntIndex = 0; inIntIndex < srAll.inIntNum; inIntIndex++)
		{
			if (srAll.srInt[inIntIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}


			/* 比對Tag Name */
			if (memcmp(szTagName, srAll.srInt[inIntIndex].szTag, strlen(srAll.srInt[inIntIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srInt[inIntIndex].szTag))
			{
				*(int32_t*)srAll.srInt[inIntIndex].pTagValue = sqlite3_column_int(srSQLStat, j);
				srAll.srInt[inIntIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inInt64tIndex = 0; inInt64tIndex < srAll.inInt64tNum; inInt64tIndex++)
		{
			if (srAll.srInt64t[inInt64tIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll.srInt64t[inInt64tIndex].szTag, strlen(srAll.srInt64t[inInt64tIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srInt64t[inInt64tIndex].szTag))
			{
				*(int64_t*)srAll.srInt64t[inInt64tIndex].pTagValue = sqlite3_column_int64(srSQLStat, j);
				srAll.srInt64t[inInt64tIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inCharIndex = 0; inCharIndex < srAll.inCharNum; inCharIndex++)
		{
			if (srAll.srChar[inCharIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll.srChar[inCharIndex].szTag, strlen(srAll.srChar[inCharIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srChar[inCharIndex].szTag))
			{
				inDataLen = sqlite3_column_bytes(srSQLStat, j);
				memcpy(srAll.srChar[inCharIndex].pCharVariable, sqlite3_column_blob(srSQLStat, j), inDataLen);
				srAll.srChar[inCharIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Find no variable to insert:Tag: %s", szTagName);
			inDISP_LogPrintf(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Value: %s", (char*)sqlite3_column_blob(srSQLStat, j));
			inDISP_LogPrintf(szDebugMsg);
		}

	}

	inFunc_WatchRunTime();

	/* 釋放事務 */
	inRc = sqlite3_finalize(srSQLStat);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatch Inv  Step finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
	}

	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatch Inv  Step Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_EMV_ByInvNum
Date&Time       :2017/3/20 下午 12:02
Describe        :利用調閱標號來將該筆資料全塞回pobTran中的BRec、會取最新狀態(如取消、調帳)
*/
int inSqlite_Get_EMV_ByInvNum(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int	inRc;
	int	j = 0;
	int	inIntIndex = 0, inInt64tIndex = 0, inCharIndex = 0;
	int	inCols = 0, inDataLen = 0;
	int	inRetVal = VS_SUCCESS;
	int	inFind = VS_FALSE;
	char	szDebugMsg[128 + 1];
	char	szQuerySql[200 + 1];	/* INSERT INTO	szTableName( */
	char	szTagName[_TAG_WIDTH_ + 1];
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_ALL_TABLE	srAll;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Emv GetBatch Inv OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] Line[%d]", szTableName,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}

	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_EMVRec(pobTran, &srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Sq Emv GetBatch Inv LinkBRec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] Line[%d]", szTableName,  __LINE__);
		
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq InitialSetting DB Config  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
		
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	if (pobTran->inTableID >= 0)
	{
		sprintf(szQuerySql, "SELECT * FROM %s WHERE (inBatchTableID = %d)", szTableName, pobTran->inTableID);
	}
	else
	{
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq Emv GetBatch Inv Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}
		inDISP_DispLogAndWriteFlie("  inTableID[%d] *Error* Line[%d]",pobTran->inTableID,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Emv GetBatch Inv Prepare *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql, __LINE__);
	}

	inFunc_CalculateRunTimeGlobal_Start();
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		/* 替換資料前先清空srBRec */
		memset(&pobTran->srEMVRec, 0x00, sizeof(pobTran->srEMVRec));
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inDISP_LogPrintf(szDebugMsg);
		}
		
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq Emv GetBatch Inv Stpe finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}

		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq Emv GetBatch Inv Stpe  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
		inDISP_DispLogAndWriteFlie("  SQLITE_DONE Line[%d]",  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_NO_RECORD);
	}
	else
	{
		inDISP_DispLogAndWriteFlie("  Sq Emv GetBatch Inv  Step *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  Reason[%s] Line[%d]", sqlite3_errmsg(srDBConnection) , __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql, __LINE__);		
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq Emv GetBatch Inv Stpe finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}

		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq Emv GetBatch Inv Stpe  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}
		
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	inCols = sqlite3_column_count(srSQLStat);

	/* binding 取得值 */
	for (j = 0; j < inCols; j++)
	{
		inFind = VS_FALSE;
		memset(szTagName, 0x00, sizeof(szTagName));
		strcat(szTagName, sqlite3_column_name(srSQLStat, j));


		for (inIntIndex = 0; inIntIndex < srAll.inIntNum; inIntIndex++)
		{
			if (srAll.srInt[inIntIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}


			/* 比對Tag Name */
			if (memcmp(szTagName, srAll.srInt[inIntIndex].szTag, strlen(srAll.srInt[inIntIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srInt[inIntIndex].szTag))
			{
				*(int32_t*)srAll.srInt[inIntIndex].pTagValue = sqlite3_column_int(srSQLStat, j);
				srAll.srInt[inIntIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inInt64tIndex = 0; inInt64tIndex < srAll.inInt64tNum; inInt64tIndex++)
		{
			if (srAll.srInt64t[inInt64tIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll.srInt64t[inInt64tIndex].szTag, strlen(srAll.srInt64t[inInt64tIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srInt64t[inInt64tIndex].szTag))
			{
				*(int64_t*)srAll.srInt64t[inInt64tIndex].pTagValue = sqlite3_column_int(srSQLStat, j);
				srAll.srInt64t[inInt64tIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inCharIndex = 0; inCharIndex < srAll.inCharNum; inCharIndex++)
		{
			if (srAll.srChar[inCharIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll.srChar[inCharIndex].szTag, strlen(srAll.srChar[inCharIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srChar[inCharIndex].szTag))
			{
				inDataLen = sqlite3_column_bytes(srSQLStat, j);
				memcpy(srAll.srChar[inCharIndex].pCharVariable, sqlite3_column_blob(srSQLStat, j), inDataLen);
				srAll.srChar[inCharIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Find no variable to insert:Tag: %s", szTagName);
			inDISP_LogPrintf(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Value: %s", (char*)sqlite3_column_blob(srSQLStat, j));
			inDISP_LogPrintf(szDebugMsg);
		}

	}

	inFunc_WatchRunTime();

	/* 釋放事務 */
	inRc = sqlite3_finalize(srSQLStat);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Emv GetBatch Inv Stpe finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
	}

	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Emv GetBatch Inv Stpe  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Update_ByInvoiceNumber_All
Date&Time       :2016/4/13 上午 10:42
Describe        :利用調閱標號來更新資料庫中的所有欄位，注意:update keytag不能重複 這隻function暫時用不到
 *		因為要保存每次交易狀態，而不是只紀錄帳
*/
int inSqlite_Update_ByInvoiceNumber_All(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber)
{
	int	inRc;
	int	inBindingIndex = 1;	/* binding的index從1開始 */
	int	i = 0;
	int	inRetVal = VS_SUCCESS;
	int	inSqlLength = 0;
	char	szDebugMsg[84 + 1];
	char	szSqlPrefix[100 + 1];	/* Update INTO	szTableName( */
	char	szSqlSuffix[40 + 1];	/* VALUES ( */
	char	szSqlSuffix2[40 + 1];	/* ); */
	char	szTemplate[720 + 1];	/* 因為最長有到711 */
	char	*szUpdateSql;
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_ALL_TABLE	srAll;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	pobTran->srBRec.lnOrgInvNum = inInvoiceNumber;

	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq UpdateByInvAll OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] Inv[%d] Line[%d]", szTableName, inInvoiceNumber, __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}

	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_BRec(pobTran, &srAll, _LS_UPDATE_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Sq UpdateByInvAll LinkBrec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] Inv[%d] Line[%d]", szTableName, inInvoiceNumber, __LINE__);
		
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq InitialSetting DB Config  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "UPDATE %s", szTableName);

	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, " SET ");

	memset(szSqlSuffix2, 0x00, sizeof(szSqlSuffix2));
	sprintf(szSqlSuffix2, " WHERE (lnOrgInvNum = %d) AND (inTableID = (SELECT MAX(inTableID) FROM %s))", inInvoiceNumber, szTableName);

	/* 算要配置多少記憶體 */
	inSqlLength += strlen(szSqlPrefix);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("COUNT");

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "IntTag: %d", srAll.inIntNum);
		inDISP_LogPrintf(szDebugMsg);

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Int64tTag: %d", srAll.inInt64tNum);
		inDISP_LogPrintf(szDebugMsg);

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "CharTag: %d", srAll.inCharNum);
		inDISP_LogPrintf(szDebugMsg);
	}

	for (i = 0; i < srAll.inIntNum; i++)
	{
		/* Comma + space （i = 0時不放", "，但這裡只是分配空間，所以可以忽略 ） */
		inSqlLength += 2;
		inSqlLength += strlen(srAll.srInt[i].szTag);
		/* space + " = " + space*/
		inSqlLength += 3;
		/* " ? " */
		inSqlLength += 1;
	}

	for (i = 0; i < srAll.inInt64tNum; i++)
	{
		/* Comma + space */
		inSqlLength += 2;
		inSqlLength += strlen(srAll.srInt64t[i].szTag);
		/* space + " = " + space*/
		inSqlLength += 3;
		/* " ? " */
		inSqlLength += 1;
	}

	for (i = 0; i < srAll.inCharNum; i++)
	{
		/* Comma + space */
		inSqlLength += 2;
		inSqlLength += strlen(srAll.srChar[i].szTag);
		/* space + " = " + space */
		inSqlLength += 3;
		/* 根據字串長度 */
		inSqlLength += strlen(srAll.srChar[i].pCharVariable);
		/* " ? " */
		inSqlLength++;
	}

	/* "SET "的長度 */
	inSqlLength += strlen(szSqlSuffix);


	/* "WHERE lnOrgInvNum = %d" */
	inSqlLength += strlen(szSqlSuffix2);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", inSqlLength);
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 配置記憶體 */
	szUpdateSql = malloc(inSqlLength+1);
	memset(szUpdateSql, 0x00, inSqlLength);

	/* 先丟Table Name */
	strcat(szUpdateSql, szSqlPrefix);
	strcat(szUpdateSql, szSqlSuffix);

	memset(szTemplate, 0x00, sizeof(szTemplate));

	for (i = 0; i < srAll.inIntNum; i++)
	{
		if (i == 0)
		{
			/* 第一個table的第一項不處理 */
		}
		else
		{
			strcat(szUpdateSql, ", ");
		}
		strcat(szUpdateSql, srAll.srInt[i].szTag);
		strcat(szUpdateSql, " = ");

		sprintf(szTemplate, "?");
		strcat(szUpdateSql, szTemplate);
		memset(szTemplate, 0x00, 1);
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Int update OK");
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 代表上一個table有東西，要加comma */
	if (srAll.inIntNum > 0)
	{
		strcat(szUpdateSql, ", ");
	}

	for (i = 0; i < srAll.inInt64tNum; i++)
	{
		if (i == 0)
		{
			/* 第一個table的第一項不處理 */
		}
		else
		{
			strcat(szUpdateSql, ", ");
		}
		strcat(szUpdateSql, srAll.srInt64t[i].szTag);
		strcat(szUpdateSql, " = ");

		sprintf(szTemplate, "?");
		strcat(szUpdateSql, szTemplate);
		memset(szTemplate, 0x00, 1);
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Int64t update OK");
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 代表上一個table有東西，要加comma */
	if (srAll.inInt64tNum > 0)
	{
		strcat(szUpdateSql, ", ");
	}

	for (i = 0; i < srAll.inCharNum; i++)
	{

		if (i == 0)
		{
			/* 第一個table的第一項不處理 */
		}
		else
		{
			strcat(szUpdateSql, ", ");
		}
		strcat(szUpdateSql, srAll.srChar[i].szTag);
		strcat(szUpdateSql, " = ");

		sprintf(szTemplate, "?");
		strcat(szUpdateSql, szTemplate);
		/* 只清自己用過得長度 問號 */
		memset(szTemplate, 0x00, 1);

	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Char update OK");
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 為了避免有空的table導致多塞", "，若最後為", "清回0x00 */
	if (memcmp((szUpdateSql + strlen(szUpdateSql) - 2), ", ", 2) == 0)
	{
		memset(szUpdateSql + strlen(szUpdateSql) - 2, 0x00, 2);
	}

	/* 最後面的"WHERE lnOrgInvNum = %d" */
	strcat(szUpdateSql, szSqlSuffix2);

	if (ginDebug == VS_TRUE)
	{
		/* 因為值很多，所以用teraterm確認 */
		printf(szUpdateSql);
	}

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szUpdateSql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq UpdateByInvAll PrepareV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szUpdateSql[%s]  inSqlLength[%d] Line[%d]", szUpdateSql, inSqlLength,  __LINE__);
	
	}

	/* Binding變數 */
	for (i = 0; i < srAll.inIntNum; i++)
	{
		inRetVal = sqlite3_bind_int(srSQLStat, inBindingIndex, *(int32_t*)srAll.srInt[i].pTagValue);
		if (inRetVal != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq UpdateByInvAll BindInt *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
			inDISP_DispLogAndWriteFlie("  inBindingIndex[%d]  inIntNum[%d] Line[%d]", inBindingIndex, srAll.inIntNum, __LINE__);
		}
		else
		{
			inBindingIndex++;
		}

	}

	for (i = 0; i < srAll.inInt64tNum; i++)
	{
		inRetVal = sqlite3_bind_int64(srSQLStat, inBindingIndex, *(int64_t*)srAll.srInt64t[i].pTagValue);
		if (inRetVal != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq UpdateByInvAll Bind64 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
			inDISP_DispLogAndWriteFlie("  inBindingIndex[%d]  inInt64tNum[%d] Line[%d]", inBindingIndex, srAll.inInt64tNum, __LINE__);
		}
		else
		{
			inBindingIndex++;
		}
	}

	for (i = 0; i < srAll.inCharNum; i++)
	{
		inRetVal = sqlite3_bind_blob(srSQLStat, inBindingIndex, srAll.srChar[i].pCharVariable, srAll.srChar[i].inTagValueLen, NULL);
		if (inRetVal != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq UpdateByInvAll Blob *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
			inDISP_DispLogAndWriteFlie("  inBindingIndex[%d]  inCharNum[%d] Line[%d]", inBindingIndex, srAll.inCharNum, __LINE__);
		}
		else
		{
			inBindingIndex++;
		}
	}

	/* Update */
	do
	{
		/* Update */
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW	||
		    inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Update OK");
				inDISP_LogPrintf(szDebugMsg);
			}
		}
		else
		{
			inDISP_DispLogAndWriteFlie("  Sq UpdateByInvAll Step *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
			inDISP_DispLogAndWriteFlie("  Reason[%s] Line[%d]", sqlite3_errmsg(srDBConnection) , __LINE__);
			inDISP_DispLogAndWriteFlie("  szUpdateSql[%s]  inSqlLength[%d] Line[%d]", szUpdateSql, inSqlLength,  __LINE__);
		}

	}while (inRetVal == SQLITE_ROW);

	/* 釋放事務，若要重用則用sqlite3_reset */
	inRc = sqlite3_finalize(srSQLStat);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq UpdateByInvAll Stpe finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
	}

	free(szUpdateSql);

	/* 關閉 database, close null pointer 是NOP(No Operation) */
	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq UpdateByInvAll Stpe  Close *Error* Hdt[%d] inRc[%d] Line[%d]", pobTran->srBRec.inHDTIndex, inRc,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Delete_Record_ByInvoiceNumber
Date&Time       :2016/4/11 下午 3:25
Describe        :利用調閱標號刪除Record
*/
int inSqlite_Delete_Record_ByInvoiceNumber(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber)
{
	int	inRc;
	int	inRetVal;
	char	szDebugMsg[84 + 1];
	char	szQuerySql[300 + 1];	/* INSERT INTO	szTableName( */
	char	*szErrorMessage = NULL;
	sqlite3	*srDBConnection;	/* 建立到資料庫的connection */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq DelRecByInvl OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] Inv[%d] Line[%d]", szTableName, inInvoiceNumber, __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}

	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "DELETE FROM %s WHERE lnOrgInvNum = %d", szTableName, inInvoiceNumber);

	/* 取得 database 裡所有的資料 */
	inRetVal = sqlite3_exec(srDBConnection , szQuerySql, 0 , 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq DelRecByInvl exec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName [%s] Reason Msg[%s] Line[%d]", szTableName, szErrorMessage, __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql [%s] Line[%d]", szQuerySql,  __LINE__);

		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq DelRecByInvl Close *Error* Hdt[%d] inRc[%d] Line[%d]", pobTran->srBRec.inHDTIndex, inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Delete Record OK");
			inDISP_LogPrintf(szDebugMsg);
		}

	}

	/* 關閉 database, close null pointer 是NOP(No Operation) */
	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq DelRecByInvl Close *Error* Hdt[%d] inRc[%d] Line[%d]", pobTran->srBRec.inHDTIndex, inRc,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_ESC_Get_BRec_Top_Flow
Date&Time       :2017/3/14 下午 3:49
Describe        :
*/
int inSqlite_ESC_Get_BRec_Top_Flow(TRANSACTION_OBJECT *pobTran, int inTableType)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		default :
			inDISP_DispLogAndWriteFlie("   TableName Not Found[%d] Line[%d]",inTableType, __LINE__);
			break;
	}

	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_EMV_TABLE_:
			inRetVal = inSqlite_ESC_Get_BRec_Top(pobTran, szTableName);
			break;
		default :
			inDISP_DispLogAndWriteFlie("  Sql Table Not Found[%d] Line[%d]",inTableType, __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
	}
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
Function        : inSqlite_ESC_Get_EMVRec_Top_Flow
Date&Time   : 20190919 
Describe        : 新增ESC EMV記錄
*/
int inSqlite_ESC_Get_EMVRec_Top_Flow(TRANSACTION_OBJECT *pobTran, int inTableType)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_ESC_AGAIN_EMV_:/* 新增ESC EMV記錄 20190919 [SAM] */
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_EMV_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_EMV_:/* 新增ESC EMV記錄 20190919 [SAM] */
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_EMV_SUFFIX_);
			break;
		default :
			inDISP_DispLogAndWriteFlie("   TableName Not Found[%d] Line[%d]",inTableType, __LINE__);
			break;
	}

	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_ESC_AGAIN_EMV_:
		case _TN_BATCH_TABLE_ESC_FAIL_EMV_:
			inRetVal = inSqlite_ESC_Get_EMVRec_Top(pobTran, szTableName);
			break;
		default :
			inDISP_DispLogAndWriteFlie("  Sql Table Not Found[%d] Line[%d]",inTableType, __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}



/*
Function        :inSqlite_ESC_Get_BRec_Top
Date&Time       :2016/4/12 下午 2:53
Describe        :ESC會取table中Primary Key最小的值
*/
int inSqlite_ESC_Get_BRec_Top(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int	inRc;
	int	j = 0;
	int	inIntIndex = 0, inInt64tIndex = 0, inCharIndex = 0;
	int	inCols = 0, inDataLen = 0;
	int	inRetVal = VS_SUCCESS;
	int	inFind = VS_FALSE;
	char	szDebugMsg[128 + 1];
	char	szQuerySql[300 + 1];	/* INSERT INTO	szTableName( */
	char	szTagName[_TAG_WIDTH_ + 1];
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_ALL_TABLE	srAll;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Esc GetRecTop OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] Line[%d]", szTableName,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}

	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_BRec(pobTran, &srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Sq Esc GetRecTop LinkBrec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] Line[%d]", szTableName,  __LINE__);
		
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq InitialSetting DB Config  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
		
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "SELECT * FROM %s WHERE (inTableID = (SELECT MIN(inTableID) FROM %s))", szTableName, szTableName);

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Esc GetRecTop Prepare *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql,  __LINE__);
	}

	inFunc_CalculateRunTimeGlobal_Start();
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		/* 替換資料前先清空srBRec */
		//memset(&pobTran->srBRec, 0x00, sizeof(pobTran->srBRec));
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inDISP_LogPrintf(szDebugMsg);
		}
		
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBRecTop Stpe finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}

		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBRecTop Stpe  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
		inDISP_DispLogAndWriteFlie("  VS_NO_RECORD Line[%d]", __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_NO_RECORD);
	}
	else
	{
		inDISP_DispLogAndWriteFlie("  Sq GetRecTop Step *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  Reason[%s] Line[%d]", sqlite3_errmsg(srDBConnection) , __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql, __LINE__);		
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBRecTop Stpe finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}

		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBRecTop Stpe  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			
			st_inSqlitHandleCnt --;
		}
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	inCols = sqlite3_column_count(srSQLStat);

	/* binding 取得值 */
	for (j = 0; j < inCols; j++)
	{
		inFind = VS_FALSE;
		memset(szTagName, 0x00, sizeof(szTagName));
		strcat(szTagName, sqlite3_column_name(srSQLStat, j));


		for (inIntIndex = 0; inIntIndex < srAll.inIntNum; inIntIndex++)
		{
			if (srAll.srInt[inIntIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}


			/* 比對Tag Name */
			if (memcmp(szTagName, srAll.srInt[inIntIndex].szTag, strlen(srAll.srInt[inIntIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srInt[inIntIndex].szTag))
			{
				*(int32_t*)srAll.srInt[inIntIndex].pTagValue = sqlite3_column_int(srSQLStat, j);
				srAll.srInt[inIntIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inInt64tIndex = 0; inInt64tIndex < srAll.inInt64tNum; inInt64tIndex++)
		{
			if (srAll.srInt64t[inInt64tIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll.srInt64t[inInt64tIndex].szTag, strlen(srAll.srInt64t[inInt64tIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srInt64t[inInt64tIndex].szTag))
			{
				*(int64_t*)srAll.srInt64t[inInt64tIndex].pTagValue = sqlite3_column_int64(srSQLStat, j);
				srAll.srInt64t[inInt64tIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inCharIndex = 0; inCharIndex < srAll.inCharNum; inCharIndex++)
		{
			if (srAll.srChar[inCharIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll.srChar[inCharIndex].szTag, strlen(srAll.srChar[inCharIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srChar[inCharIndex].szTag))
			{
				inDataLen = sqlite3_column_bytes(srSQLStat, j);
				memcpy(srAll.srChar[inCharIndex].pCharVariable, sqlite3_column_blob(srSQLStat, j), inDataLen);
				srAll.srChar[inCharIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Find no variable to insert:Tag: %s", szTagName);
			inDISP_LogPrintf(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Value: %s", (char*)sqlite3_column_blob(srSQLStat, j));
			inDISP_LogPrintf(szDebugMsg);
		}

	}

	inFunc_WatchRunTime();

	/* 釋放事務 */
	inRc = sqlite3_finalize(srSQLStat);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBRecTop Stpe finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
	}

	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBRecTop Stpe  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}


/*
Function        : inSqlite_ESC_Get_EMVRec_Top
Date&Time   : 20190919 
Describe        :  
 * 新增ESC EMV記錄 
 * ESC會取table中Primary Key最小的值
*/
int inSqlite_ESC_Get_EMVRec_Top(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int	inRc;
	int	j = 0;
	int	inIntIndex = 0, inInt64tIndex = 0, inCharIndex = 0;
	int	inCols = 0, inDataLen = 0;
	int	inRetVal = VS_SUCCESS;
	int	inFind = VS_FALSE;
	char	szDebugMsg[128 + 1];
	char	szQuerySql[300 + 1];	/* INSERT INTO	szTableName( */
	char	szTagName[_TAG_WIDTH_ + 1];
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_ALL_TABLE	srAll;

	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Esc GetRecTop OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] Line[%d]", szTableName,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}

	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_EMVRec(pobTran, &srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Sq Esc GetRecTop LinkBrec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] Line[%d]", szTableName,  __LINE__);
		
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq InitialSetting DB Config  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	if (pobTran->inTableID >= 0)
	{
		sprintf(szQuerySql, "SELECT * FROM %s WHERE (inBatchTableID = %d)", szTableName, pobTran->inTableID);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, " Sq Cmd [%s]",szQuerySql );
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else
	{

		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  ESC Sq Emv GetBatch Inv Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "  ESC Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}
		inDISP_DispLogAndWriteFlie("  inTableID[%d] *Error* Line[%d]", pobTran->inTableID, __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Esc EVM GetRecTop Prepare *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql,  __LINE__);
	}

	inFunc_CalculateRunTimeGlobal_Start();
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		/* 替換資料前先清空srBRec */
		memset(&pobTran->srEMVRec, 0x00, sizeof(pobTran->srEMVRec));
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inDISP_LogPrintf(szDebugMsg);
		}
		
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq Emv Sq GetRecTop Stpe finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}

		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq Emv Sq GetRecTop Stpe  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
		inDISP_DispLogAndWriteFlie("  VS_NO_RECORD Line[%d]", __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_NO_RECORD);
	}
	else
	{
		inDISP_DispLogAndWriteFlie("  ESC Emv Sq GetRecTop Step *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  Reason[%s] Line[%d]", sqlite3_errmsg(srDBConnection) , __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql, __LINE__);		
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq Emv Sq GetRecTop Stpe finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}

		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq Emv Sq GetRecTop Stpe  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			
			st_inSqlitHandleCnt --;
		}
		
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	inCols = sqlite3_column_count(srSQLStat);

	/* binding 取得值 */
	for (j = 0; j < inCols; j++)
	{
		inFind = VS_FALSE;
		memset(szTagName, 0x00, sizeof(szTagName));
		strcat(szTagName, sqlite3_column_name(srSQLStat, j));


		for (inIntIndex = 0; inIntIndex < srAll.inIntNum; inIntIndex++)
		{
			if (srAll.srInt[inIntIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}


			/* 比對Tag Name */
			if (memcmp(szTagName, srAll.srInt[inIntIndex].szTag, strlen(srAll.srInt[inIntIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srInt[inIntIndex].szTag))
			{
				*(int32_t*)srAll.srInt[inIntIndex].pTagValue = sqlite3_column_int(srSQLStat, j);
				srAll.srInt[inIntIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inInt64tIndex = 0; inInt64tIndex < srAll.inInt64tNum; inInt64tIndex++)
		{
			if (srAll.srInt64t[inInt64tIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll.srInt64t[inInt64tIndex].szTag, strlen(srAll.srInt64t[inInt64tIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srInt64t[inInt64tIndex].szTag))
			{
				*(int64_t*)srAll.srInt64t[inInt64tIndex].pTagValue = sqlite3_column_int64(srSQLStat, j);
				srAll.srInt64t[inInt64tIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inCharIndex = 0; inCharIndex < srAll.inCharNum; inCharIndex++)
		{
			if (srAll.srChar[inCharIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll.srChar[inCharIndex].szTag, strlen(srAll.srChar[inCharIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srChar[inCharIndex].szTag))
			{
				inDataLen = sqlite3_column_bytes(srSQLStat, j);
				memcpy(srAll.srChar[inCharIndex].pCharVariable, sqlite3_column_blob(srSQLStat, j), inDataLen);
				srAll.srChar[inCharIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Find no variable to insert:Tag: %s", szTagName);
			inDISP_LogPrintf(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Value: %s", (char*)sqlite3_column_blob(srSQLStat, j));
			inDISP_LogPrintf(szDebugMsg);
		}

	}

	inFunc_WatchRunTime();

	/* 釋放事務 */
	inRc = sqlite3_finalize(srSQLStat);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Emv Sq GetRecTop Stpe finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
	}

	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Emv Sq GetRecTop Stpe  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}


/*
Function        :inSqlite_ESC_Delete_Record_Flow
Date&Time       :2017/3/14 下午 3:57
Describe        :
*/
int inSqlite_ESC_Delete_Record_Flow(TRANSACTION_OBJECT *pobTran, int inTableType)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_EMV_:/* 新增ESC EMV記錄 20190919 [SAM] */
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_EMV_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_EMV_:/* 新增ESC EMV記錄 20190919 [SAM] */
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_EMV_SUFFIX_);
			break;	
		default :
			inDISP_DispLogAndWriteFlie("   TableName Not Found[%d] Line[%d]",inTableType, __LINE__);
			break;
	}

	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_BATCH_TABLE_ESC_AGAIN_EMV_:/* 新增ESC EMV記錄 20190919 [SAM] */
		case _TN_BATCH_TABLE_ESC_FAIL_EMV_:/* 新增ESC EMV記錄 20190919 [SAM] */
		case _TN_EMV_TABLE_:
			inRetVal = inSqlite_ESC_Delete_Record(pobTran, szTableName);
			break;
		default :
			inDISP_DispLogAndWriteFlie("  Sql Table Not Found[%d] Line[%d]",inTableType, __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
Function        :inSqlite_ESC_Delete_Record
Date&Time       :2016/4/11 下午 3:25
Describe        :ESC會刪除table中Primary Key最小的值
*/
int inSqlite_ESC_Delete_Record(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int	inRc;
	int	inRetVal;
	char	szDebugMsg[84 + 1];
	char	szQuerySql[300 + 1];	/* INSERT INTO	szTableName( */
	char	*szErrorMessage = NULL;
	sqlite3	*srDBConnection;	/* 建立到資料庫的connection */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Esc DelRec OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] Line[%d]", szTableName,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}

	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "DELETE FROM %s WHERE inTableID = (SELECT MIN(inTableID) FROM %s)", szTableName, szTableName);

	/* 取得 database 裡所有的資料 */
	inRetVal = sqlite3_exec(srDBConnection , szQuerySql, 0 , 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Esc DelRec Exec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szErrorMessage[%s] Line[%d]", szErrorMessage , __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql, __LINE__);		

		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq Esc DelRec Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Delete Record OK");
			inDISP_LogPrintf(szDebugMsg);
		}

	}

	/* 關閉 database, close null pointer 是NOP(No Operation) */
	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Esc DelRec Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Max_TableID_Flow
Date&Time       :2017/3/15 下午 2:43
Describe        :
*/
int inSqlite_Get_Max_TableID_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, char *szTagValue)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		default :
			inDISP_DispLogAndWriteFlie("   TableName Not Found[%d] Line[%d]",inTableType, __LINE__);
			break;
	}

	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_EMV_TABLE_:
			inRetVal = inSqlite_Get_Max_TableID(pobTran, szTableName, szTagValue);
//			inRetVal = inSqlite_Get_Max_TableID(pobTran, szTableName, szMaxTableID);
			break;
		default :
			inDISP_DispLogAndWriteFlie("  Sql Table Not Found[%d] Line[%d]",inTableType, __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
Function        :inSqlite_Get_Max_TableID
Date&Time       :2017/3/15 下午 1:29
Describe        :抓最大的Primary Key值
*/
int inSqlite_Get_Max_TableID(TRANSACTION_OBJECT *pobTran, char* szTableName, char *szTagValue)
//int inSqlite_Get_Max_TableID(char* szDBPath, char* szTableName, char *szTagValue)
{
	int	inRc;
	int		j = 0;
	int		inDataLen = 0;
	int		inRetVal = VS_ERROR;
	char		szDebugMsg[84 + 1];
	char		szQuerySql[300 + 1];	/* INSERT INTO	szTableName( */
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt	*srSQLStat;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
//	inRetVal = inSqlite_DB_Open_Or_Create(szDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetMaxTableID OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] szTagValue[%s] Line[%d]", szTableName, szTagValue,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}

	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "SELECT MAX(%s) FROM %s", "inTableID", szTableName);

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetMaxTableID Prepare *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql,  __LINE__);
	}

	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inDISP_LogPrintf(szDebugMsg);
		}
		
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetMaxTableID finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}

		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetMaxTableID  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
					
		inDISP_DispLogAndWriteFlie("  VS_NO_RECORD Line[%d]", __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_NO_RECORD);
	}
	else
	{
		inDISP_DispLogAndWriteFlie("  Sq GetMaxTableID Step *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szErrorMessage[%s] Line[%d]", sqlite3_errmsg(srDBConnection) , __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql, __LINE__);		

		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetMaxTableID finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}

		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetMaxTableID  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	/* 若是成功，將值丟到輸入的位置。 */
	j = 0;
	inDataLen = sqlite3_column_bytes(srSQLStat, j);
	memcpy(szTagValue, sqlite3_column_blob(srSQLStat, j), inDataLen);

	/* 釋放事務 */
	inRc = sqlite3_finalize(srSQLStat);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetMaxTableID finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
	}

	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetMaxTableID  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Update_ByInvoiceNumber_TranState_Flow
Date&Time       :2017/3/15 下午 2:43
Describe        :用來更新批次狀態時，把舊的紀錄作廢
*/
int inSqlite_Update_ByInvNum_TranState_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inInvoiceNumber)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		default :
			inDISP_DispLogAndWriteFlie("  TableName Not Found[%d] Line[%d]",inTableType, __LINE__);
			break;
	}

	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_EMV_TABLE_:
			inRetVal = inSqlite_Update_ByInvNum_TranState(pobTran, szTableName, inInvoiceNumber);
			break;
		case _TN_BATCH_TABLE_TICKET_:
			inRetVal = inSqlite_Update_ByInvNum_TranState_Ticket(pobTran, szTableName, inInvoiceNumber);
			break;
		default :
			inDISP_DispLogAndWriteFlie("  Sql Table Not Found[%d] Line[%d]",inTableType, __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);

	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
Function        :inSqlite_Update_ByInvNum_TranState
Date&Time       :2017/3/15 下午 2:46
Describe        : UPDATE table_name SET column1 = value1, column2 = value2...., columnN = valueN WHERE [condition];
 *
*/
int inSqlite_Update_ByInvNum_TranState(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber)
{
	int	inRc;
	int	i;
	int	inRetVal;
	int	inSqlLength = 0;
	char	szDebugMsg[84 + 1];
	char	szSqlPrefix[100 + 1];	/* Update INTO	szTableName( */
	char	szSqlSuffix[40 + 1];	/* VALUES ( */
	char	szSqlSuffix2[200 + 1];	/* ); */
	char	szTemplate[720 + 1];	/* 因為最長有到711 */
	char	szTag[_TAG_WIDTH_ + 1];
	char	szTagValue[2 + 1];
	char	*szErrorMessage = NULL;
	char	*szUpdateSql;
	sqlite3	*srDBConnection;	/* 建立到資料庫的connection */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	pobTran->srBRec.lnOrgInvNum = inInvoiceNumber;

	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq UpdateByInvTranState OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] Inv[%d] Line[%d]", szTableName, inInvoiceNumber, __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}

	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "UPDATE %s", szTableName);

	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, " SET ");

	memset(szSqlSuffix2, 0x00, sizeof(szSqlSuffix2));
	sprintf(szSqlSuffix2, " WHERE inTableID IN (SELECT inTableID FROM %s WHERE lnOrgInvNum = %d ORDER BY inTableID DESC LIMIT 1)", szTableName, inInvoiceNumber);

	memset(szTag, 0x00, sizeof(szTag));
	sprintf(szTag, "uszUpdated");

	memset(szTagValue, 0x00, sizeof(szTagValue));
	sprintf(szTagValue, "1");

	/* 算要配置多少記憶體 */
	inSqlLength += strlen(szSqlPrefix);

	for (i = 0; i < 1; i++)
	{
		/* Comma + space */
		inSqlLength += 2;
		inSqlLength += strlen(szTag);
		/* space + " = " + space + " ' " */
		inSqlLength += 4;
		/* 根據字串長度 */
		inSqlLength += strlen(szTagValue);
		/* " ' " */
		inSqlLength++;
	}

	/* "SET "的長度 */
	inSqlLength += strlen(szSqlSuffix);

	/* "WHERE lnOrgInvNum = %d" */
	inSqlLength += strlen(szSqlSuffix2);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", inSqlLength);
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 配置記憶體 */
	szUpdateSql = malloc(inSqlLength+1);
	memset(szUpdateSql, 0x00, inSqlLength);

	/* 先丟Table Name */
	strcat(szUpdateSql, szSqlPrefix);
	strcat(szUpdateSql, szSqlSuffix);

	memset(szTemplate, 0x00, sizeof(szTemplate));

	for (i = 0; i < 1; i++)
	{
		if (i == 0)
		{
			/* 第一個table的第一項不處理 */
		}
		else
		{
			strcat(szUpdateSql, ", ");
		}
		strcat(szUpdateSql, szTag);
		strcat(szUpdateSql, " = ");

		sprintf(szTemplate, "'%s'", szTagValue);
		strcat(szUpdateSql, szTemplate);
		/* 只清自己用過得長度 兩個單引號*/
		memset(szTemplate, 0x00, strlen(szTagValue) + 2);
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Char update OK");
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 為了避免有空的table導致多塞", "，若最後為", "清回0x00 */
	if (memcmp((szUpdateSql + strlen(szUpdateSql) - 2), ", ", 2) == 0)
	{
		memset(szUpdateSql + strlen(szUpdateSql) - 2, 0x00, 2);
	}

	/* 最後面的"WHERE lnOrgInvNum = %d" */
	strcat(szUpdateSql, szSqlSuffix2);

	if (ginDebug == VS_TRUE)
	{
		/* 因為值很多，所以用teraterm確認 */
		printf(szUpdateSql);
	}

	/* Update */
	inRetVal = sqlite3_exec(srDBConnection, szUpdateSql, 0, 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq UpdateByInvTranState Exec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szErrorMessage[%s] Line[%d]", szErrorMessage , __LINE__);
		inDISP_DispLogAndWriteFlie("  szUpdateSql[%s] inSqlLength[%d] Line[%d]", szUpdateSql, inSqlLength, __LINE__);		
		
		free(szUpdateSql);
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq UpdateByInvTranState Close Rc[%d] *Error* Line[%d]", inRc, __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Update OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	free(szUpdateSql);

	/* 關閉 database, close null pointer 是NOP(No Operation) */
	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq UpdateByInvTranState Close Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Update_ByInvNum_TranState_Ticket
Date&Time       :2018/1/12 下午 5:05
Describe        :UPDATE table_name SET column1 = value1, column2 = value2...., columnN = valueN WHERE [condition];
 *		 用於更新batch時，需把上一筆紀錄作廢
 *
*/
int inSqlite_Update_ByInvNum_TranState_Ticket(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber)
{
	int	inRc;
	int	i;
	int	inRetVal;
	int	inSqlLength = 0;
	char	szDebugMsg[84 + 1];
	char	szSqlPrefix[100 + 1];	/* Update INTO	szTableName( */
	char	szSqlSuffix[40 + 1];	/* VALUES ( */
	char	szSqlSuffix2[200 + 1];	/* ); */
	char	szTemplate[720 + 1];	/* 因為最長有到711 */
	char	szTag[_TAG_WIDTH_ + 1];
	char	szTagValue[2 + 1];
	char	*szErrorMessage = NULL;
	char	*szUpdateSql;
	sqlite3	*srDBConnection;	/* 建立到資料庫的connection */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}

		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}

	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "UPDATE %s", szTableName);

	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, " SET ");

	memset(szSqlSuffix2, 0x00, sizeof(szSqlSuffix2));
	sprintf(szSqlSuffix2, " WHERE inTableID IN (SELECT inTableID FROM %s WHERE lnInvNum = %d ORDER BY inTableID DESC LIMIT 1)", szTableName, inInvoiceNumber);

	memset(szTag, 0x00, sizeof(szTag));
	sprintf(szTag, "uszUpdated");

	memset(szTagValue, 0x00, sizeof(szTagValue));
	sprintf(szTagValue, "1");

	/* 算要配置多少記憶體 */
	inSqlLength += strlen(szSqlPrefix);

	for (i = 0; i < 1; i++)
	{
		/* Comma + space */
		inSqlLength += 2;
		inSqlLength += strlen(szTag);
		/* space + " = " + space + " ' " */
		inSqlLength += 4;
		/* 根據字串長度 */
		inSqlLength += strlen(szTagValue);
		/* " ' " */
		inSqlLength++;
	}

	/* "SET "的長度 */
	inSqlLength += strlen(szSqlSuffix);

	/* "WHERE lnInvNum = %d" */
	inSqlLength += strlen(szSqlSuffix2);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", inSqlLength);
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 配置記憶體 */
	szUpdateSql = malloc(inSqlLength+1);
	memset(szUpdateSql, 0x00, inSqlLength);

	/* 先丟Table Name */
	strcat(szUpdateSql, szSqlPrefix);
	strcat(szUpdateSql, szSqlSuffix);

	memset(szTemplate, 0x00, sizeof(szTemplate));

	for (i = 0; i < 1; i++)
	{
		if (i == 0)
		{
			/* 第一個table的第一項不處理 */
		}
		else
		{
			strcat(szUpdateSql, ", ");
		}
		strcat(szUpdateSql, szTag);
		strcat(szUpdateSql, " = ");

		sprintf(szTemplate, "'%s'", szTagValue);
		strcat(szUpdateSql, szTemplate);
		/* 只清自己用過得長度 兩個單引號*/
		memset(szTemplate, 0x00, strlen(szTagValue) + 2);
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "UChar update OK");
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 為了避免有空的table導致多塞", "，若最後為", "清回0x00 */
	if (memcmp((szUpdateSql + strlen(szUpdateSql) - 2), ", ", 2) == 0)
	{
		memset(szUpdateSql + strlen(szUpdateSql) - 2, 0x00, 2);
	}

	/* 最後面的"WHERE lnInvNum = %d" */
	strcat(szUpdateSql, szSqlSuffix2);

	if (ginDebug == VS_TRUE)
	{
		/* 因為值很多，所以用teraterm確認 */
		printf(szUpdateSql);
	}

	/* Update */
	inRetVal = sqlite3_exec(srDBConnection, szUpdateSql, 0, 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq UpdateByInvTranState Ticket Exec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szErrorMessage[%s] Line[%d]", szErrorMessage , __LINE__);
		inDISP_DispLogAndWriteFlie("  szUpdateSql[%s] Line[%d]", szUpdateSql, __LINE__);	
		
		free(szUpdateSql);
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq UpdateByInvTranState Ticket Close Rc[%d] *Error* Line[%d]", inRc, __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}		
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Update OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}

	
	free(szUpdateSql);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq UpdateByInvTranState Ticket Close Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Update_ByInvoiceNumber_CLS_SettleBit_Flow
Date&Time       :2017/3/15 下午 2:43
Describe        :
*/
int inSqlite_Update_ByInvNum_CLS_SettleBit_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inInvoiceNumber)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		default :
			inDISP_DispLogAndWriteFlie("  TableName Not Found[%d] Line[%d]",inTableType, __LINE__);
			break;
	}

	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_EMV_TABLE_:
			inRetVal = inSqlite_Update_ByInvNum_CLS_SettleBit(pobTran, szTableName, inInvoiceNumber);
			break;
		default :
			inDISP_DispLogAndWriteFlie("  Sql Table Not Found[%d] Line[%d]",inTableType, __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
Function        :inSqlite_Update_ByInvNum_CLS_SettleBit
Date&Time       :2017/3/15 下午 2:46
Describe        :UPDATE table_name SET column1 = value1, column2 = value2...., columnN = valueN WHERE [condition];
 *
*/
int inSqlite_Update_ByInvNum_CLS_SettleBit(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber)
{
	int	inRc;
	int	i;
	int	inRetVal;
	int	inSqlLength = 0;
	char	szDebugMsg[500 + 1];
	char	szSqlPrefix[100 + 1];	/* Update INTO	szTableName( */
	char	szSqlSuffix[40 + 1];	/* VALUES ( */
	char	szSqlSuffix2[200 + 1];	/* ); */
	char	szTemplate[720 + 1];	/* 因為最長有到711 */
	char	szTag[_TAG_WIDTH_ + 1];
	char	szTagValue[10 + 1];
	char	*szErrorMessage = NULL;
	char	*szUpdateSql;
	sqlite3	*srDBConnection;	/* 建立到資料庫的connection */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	pobTran->srBRec.lnOrgInvNum = inInvoiceNumber;

	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq UpdateByInvCtlsSettle OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] Inv[%d] Line[%d]", szTableName, inInvoiceNumber, __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}

	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "UPDATE %s", szTableName);

	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, " SET ");

	memset(szSqlSuffix2, 0x00, sizeof(szSqlSuffix2));
	sprintf(szSqlSuffix2, " WHERE inTableID IN (SELECT inTableID FROM %s WHERE lnOrgInvNum = %d ORDER BY inTableID DESC LIMIT 1)", szTableName, inInvoiceNumber);

	memset(szTag, 0x00, sizeof(szTag));
	sprintf(szTag, "uszCLSBatchBit");

	memset(szTagValue, 0x00, sizeof(szTagValue));
	sprintf(szTagValue, "1");

	/* 算要配置多少記憶體 */
	inSqlLength += strlen(szSqlPrefix);

	for (i = 0; i < 1; i++)
	{
		/* Comma + space */
		inSqlLength += 2;
		inSqlLength += strlen(szTag);
		/* space + " = " + space + " ' " */
		inSqlLength += 4;
		/* 根據字串長度 */
		inSqlLength += strlen(szTagValue);
		/* " ' " */
		inSqlLength++;
	}

	/* "SET "的長度 */
	inSqlLength += strlen(szSqlSuffix);

	/* "WHERE lnOrgInvNum = %d" */
	inSqlLength += strlen(szSqlSuffix2);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", inSqlLength);
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 配置記憶體 */
	szUpdateSql = malloc(inSqlLength+1);
	memset(szUpdateSql, 0x00, inSqlLength);

	/* 先丟Table Name */
	strcat(szUpdateSql, szSqlPrefix);
	strcat(szUpdateSql, szSqlSuffix);

	memset(szTemplate, 0x00, sizeof(szTemplate));

	for (i = 0; i < 1; i++)
	{
		if (i == 0)
		{
			/* 第一個table的第一項不處理 */
		}
		else
		{
			strcat(szUpdateSql, ", ");
		}
		strcat(szUpdateSql, szTag);
		strcat(szUpdateSql, " = ");

		sprintf(szTemplate, "'%s'", szTagValue);
		strcat(szUpdateSql, szTemplate);
		/* 只清自己用過得長度 兩個單引號*/
		memset(szTemplate, 0x00, strlen(szTagValue) + 2);
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "UChar update OK");
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 為了避免有空的table導致多塞", "，若最後為", "清回0x00 */
	if (memcmp((szUpdateSql + strlen(szUpdateSql) - 2), ", ", 2) == 0)
	{
		memset(szUpdateSql + strlen(szUpdateSql) - 2, 0x00, 2);
	}

	/* 最後面的"WHERE lnOrgInvNum = %d" */
	strcat(szUpdateSql, szSqlSuffix2);

	/* Update */
	inRetVal = sqlite3_exec(srDBConnection, szUpdateSql, 0, 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq UpdateByInvCtlsSettle Exec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szErrorMessage[%s] Line[%d]", szErrorMessage , __LINE__);
		inDISP_DispLogAndWriteFlie("  szUpdateSql[%s] Line[%d]", szUpdateSql, __LINE__);		
		
		free(szUpdateSql);
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq UpdateByInvCtlsSettle Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Update OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}

	free(szUpdateSql);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq UpdateByInvCtlsSettle Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}



/*
Function        :inSqlite_Update_ByInvNum_Trans_Rrn_Flow
Date&Time       : 20190221
Describe        :
*/
int inSqlite_Update_ByInvNum_Trans_Rrn_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inInvoiceNumber)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		default :
			inDISP_DispLogAndWriteFlie("  TableName Not Found[%d] Line[%d]",inTableType, __LINE__);
			break;
	}

	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
			inRetVal = inSqlite_Update_ByInvNum_Trans_Rrn(pobTran, szTableName, inInvoiceNumber);
			break;
		default :
			inDISP_DispLogAndWriteFlie("  Sql Table Not Found[%d] Line[%d]",inTableType, __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
Function        :inSqlite_Update_ByInvNum_Trans_Rrn
Date&Time       : 20190221
Describe        :UPDATE table_name SET column1 = value1, column2 = value2...., columnN = valueN WHERE [condition];
 *
*/
int inSqlite_Update_ByInvNum_Trans_Rrn(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber)
{
	int	inRc;
	int	i;
	int	inRetVal;
	int	inSqlLength = 0;
	char	szDebugMsg[500 + 1];
	char	szSqlPrefix[100 + 1];	/* Update INTO	szTableName( */
	char	szSqlSuffix[40 + 1];	/* VALUES ( */
	char	szSqlSuffix2[200 + 1];	/* ); */
	char	szTemplate[720 + 1];	/* 因為最長有到711 */
	char	szTag[_TAG_WIDTH_ + 1];
	char	szTagValue[10 + 1];
	char	*szErrorMessage = NULL;
	char	*szUpdateSql;
	sqlite3	*srDBConnection;	/* 建立到資料庫的connection */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	pobTran->srBRec.lnOrgInvNum = inInvoiceNumber;

	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq UpdateByInvTransRrn OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] Inv[%d] Line[%d]", szTableName, inInvoiceNumber, __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}

	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "UPDATE %s", szTableName);

	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, " SET ");

	memset(szSqlSuffix2, 0x00, sizeof(szSqlSuffix2));
	sprintf(szSqlSuffix2, " WHERE inTableID IN (SELECT inTableID FROM %s WHERE lnOrgInvNum = %d ORDER BY inTableID DESC LIMIT 1)", szTableName, inInvoiceNumber);

	memset(szTag, 0x00, sizeof(szTag));
	sprintf(szTag, "szRefNo");

	memset(szTagValue, 0x00, sizeof(szTagValue));
	strcat(szTagValue, pobTran->srBRec.szRefNo);

	/* 算要配置多少記憶體 */
	inSqlLength += strlen(szSqlPrefix);

	for (i = 0; i < 1; i++)
	{
		/* Comma + space */
		inSqlLength += 2;
		inSqlLength += strlen(szTag);
		/* space + " = " + space + " ' " */
		inSqlLength += 4;
		/* 根據字串長度 */
		inSqlLength += strlen(szTagValue);
		/* " ' " */
		inSqlLength++;
	}

	/* "SET "的長度 */
	inSqlLength += strlen(szSqlSuffix);

	/* "WHERE lnOrgInvNum = %d" */
	inSqlLength += strlen(szSqlSuffix2);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", inSqlLength);
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 配置記憶體 */
	szUpdateSql = malloc(inSqlLength+1);
	memset(szUpdateSql, 0x00, inSqlLength);

	/* 先丟Table Name */
	strcat(szUpdateSql, szSqlPrefix);
	strcat(szUpdateSql, szSqlSuffix);

	memset(szTemplate, 0x00, sizeof(szTemplate));

	for (i = 0; i < 1; i++)
	{
		if (i == 0)
		{
			/* 第一個table的第一項不處理 */
		}
		else
		{
			strcat(szUpdateSql, ", ");
		}
		strcat(szUpdateSql, szTag);
		strcat(szUpdateSql, " = ");

		sprintf(szTemplate, "'%s'", szTagValue);
		strcat(szUpdateSql, szTemplate);
		/* 只清自己用過得長度 兩個單引號*/
		memset(szTemplate, 0x00, strlen(szTagValue) + 2);
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "UChar update OK");
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 為了避免有空的table導致多塞", "，若最後為", "清回0x00 */
	if (memcmp((szUpdateSql + strlen(szUpdateSql) - 2), ", ", 2) == 0)
	{
		memset(szUpdateSql + strlen(szUpdateSql) - 2, 0x00, 2);
	}

	/* 最後面的"WHERE lnOrgInvNum = %d" */
	strcat(szUpdateSql, szSqlSuffix2);

	/* Update */
	inRetVal = sqlite3_exec(srDBConnection, szUpdateSql, 0, 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq UpdateByInvTransRrn Exec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szErrorMessage[%s] Line[%d]", szErrorMessage , __LINE__);
		inDISP_DispLogAndWriteFlie("  szUpdateSql[%s] Line[%d]", szUpdateSql, __LINE__);		
		
		free(szUpdateSql);
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq UpdateByInvTransRrn Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Update OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}

	free(szUpdateSql);
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq UpdateByInvTransRrn Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}


/*
Function        :inSqlite_Get_Batch_ByCnt_Flow
Date&Time       :2017/3/15 下午 4:03
Describe        :
*/
int inSqlite_Get_Batch_ByCnt_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inRecCnt)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		default :
			inDISP_DispLogAndWriteFlie("  TableName Not Found[%d] Line[%d]",inTableType, __LINE__);
			break;
	}

	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
			inRetVal = inSqlite_Get_Batch_ByCnt(pobTran, szTableName, inRecCnt);
			break;
		case _TN_EMV_TABLE_:
			inRetVal = inSqlite_Get_EMV_ByCnt(pobTran, szTableName);
			break;
		default :
			inDISP_DispLogAndWriteFlie("  Sql Table Not Found[%d] Line[%d]",inTableType, __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
Function        :inSqlite_Get_Batch_ByCnt
Date&Time       :2017/3/15 下午 4:02
Describe        :利用調閱標號來將該筆資料全塞回pobTran中的BRec、會取最新狀態(如取消、調帳)，只抓最後一筆，
 *		因為原本API inRecCnt當成offset，所以這邊做成一樣。
*/
int inSqlite_Get_Batch_ByCnt(TRANSACTION_OBJECT *pobTran, char* szTableName, int inRecCnt)
{
	int	inRc;
	int	i = 0, j = 0;
	int	inIntIndex = 0, inInt64tIndex = 0, inCharIndex = 0;
	int	inCols = 0, inDataLen = 0;
	int	inRetVal = VS_SUCCESS;
	int	inFind = VS_FALSE;
	char	szDebugMsg[128 + 1];
	char	szQuerySql[200 + 1];	/* INSERT INTO	szTableName( */
	char	szTagName[_TAG_WIDTH_ + 1];
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_ALL_TABLE	srAll;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	inFunc_CalculateRunTimeGlobal_Start();

	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchByCnt OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] inRecCnt[%d] Line[%d]", szTableName, inRecCnt, __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}

	inFunc_WatchRunTime();

	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_BRec(pobTran, &srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchByCnt LinkBRec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] inRecCnt[%d] Line[%d]", szTableName, inRecCnt, __LINE__);
		
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq InitialSetting DB Config  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "SELECT * FROM %s WHERE (uszUpdated = 0)", szTableName);

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchByCnt Prepare *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] inRecCnt[%d] Line[%d]", szQuerySql, inRecCnt, __LINE__);
	}

	inFunc_CalculateRunTimeGlobal_Start();
	/* 取得 database 裡所有的資料 */

	/* 跳到要選的那一筆 */
	for (i = 0; i < inRecCnt; i++)
	{
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Get Table OK");
				inDISP_LogPrintf(szDebugMsg);
			}

		}
		else if (inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "NO DATA");
				inDISP_LogPrintf(szDebugMsg);
			}
			
			inRc = sqlite3_finalize(srSQLStat);
			if (inRc != SQLITE_OK)
			{
				inDISP_DispLogAndWriteFlie("  Sq InitialSetting DB Config  finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
			}

			inRc = sqlite3_close(srDBConnection);
			if (inRc != SQLITE_OK)
			{
				inDISP_DispLogAndWriteFlie("  Sq InitialSetting DB Config  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
				inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
				/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
				return (VS_ERROR);
			}else{
				st_inSqlitHandleCnt --;
			}
			return (VS_NO_RECORD);
		}
		else
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBatchByCnt Step *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
			inDISP_DispLogAndWriteFlie("  szErrorMessage[%s] Line[%d]", sqlite3_errmsg(srDBConnection) , __LINE__);
			inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql, __LINE__);		

			/* 關閉 database, close null pointer 是NOP(No Operation) */
			inRc = sqlite3_finalize(srSQLStat);
			if (inRc != SQLITE_OK)
			{
				inDISP_DispLogAndWriteFlie("  Sq InitialSetting DB Config  finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
			}

			inRc = sqlite3_close(srDBConnection);
			if (inRc != SQLITE_OK)
			{
				inDISP_DispLogAndWriteFlie("  Sq InitialSetting DB Config  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
				inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
				/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
				return (VS_ERROR);
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "Close Database OK");
					inDISP_LogPrintf(szDebugMsg);
				}
				st_inSqlitHandleCnt --;
			}
			
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
		}
	}
	inFunc_WatchRunTime();

	/* 替換資料前先清空srBRec */
	memset(&pobTran->srBRec, 0x00, sizeof(pobTran->srBRec));

	inFunc_CalculateRunTimeGlobal_Start();
	inCols = sqlite3_column_count(srSQLStat);

	/* binding 取得值 */
	for (j = 0; j < inCols; j++)
	{
		inFind = VS_FALSE;
		memset(szTagName, 0x00, sizeof(szTagName));
		strcat(szTagName, sqlite3_column_name(srSQLStat, j));


		for (inIntIndex = 0; inIntIndex < srAll.inIntNum; inIntIndex++)
		{
			if (srAll.srInt[inIntIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}


			/* 比對Tag Name */
			if (memcmp(szTagName, srAll.srInt[inIntIndex].szTag, strlen(srAll.srInt[inIntIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srInt[inIntIndex].szTag))
			{
				*(int32_t*)srAll.srInt[inIntIndex].pTagValue = sqlite3_column_int(srSQLStat, j);
				srAll.srInt[inIntIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inInt64tIndex = 0; inInt64tIndex < srAll.inInt64tNum; inInt64tIndex++)
		{
			if (srAll.srInt64t[inInt64tIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll.srInt64t[inInt64tIndex].szTag, strlen(srAll.srInt64t[inInt64tIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srInt64t[inInt64tIndex].szTag))
			{
				*(int64_t*)srAll.srInt64t[inInt64tIndex].pTagValue = sqlite3_column_int64(srSQLStat, j);
				srAll.srInt64t[inInt64tIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inCharIndex = 0; inCharIndex < srAll.inCharNum; inCharIndex++)
		{
			if (srAll.srChar[inCharIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll.srChar[inCharIndex].szTag, strlen(srAll.srChar[inCharIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srChar[inCharIndex].szTag))
			{
				inDataLen = sqlite3_column_bytes(srSQLStat, j);
				memcpy(srAll.srChar[inCharIndex].pCharVariable, sqlite3_column_blob(srSQLStat, j), inDataLen);
				srAll.srChar[inCharIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Find no variable to insert:Tag: %s", szTagName);
			inDISP_LogPrintf(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Value: %s", (char*)sqlite3_column_blob(srSQLStat, j));
			inDISP_LogPrintf(szDebugMsg);
		}

	}
	inFunc_WatchRunTime();

	inRc = sqlite3_finalize(srSQLStat);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq InitialSetting DB Config  finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
	}

	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq InitialSetting DB Config  Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_EMV_ByCnt
Date&Time       :2017/3/20 下午 3:10
Describe        :利用調閱標號來將該筆資料全塞回pobTran中的BRec、會取最新狀態(如取消、調帳)，只抓最後一筆，
 *		因為原本API inRecCnt當成offset，所以這邊做成一樣。
*/
int inSqlite_Get_EMV_ByCnt(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int	inRc;
	int	j = 0;
	int	inIntIndex = 0, inInt64tIndex = 0, inCharIndex = 0;
	int	inCols = 0, inDataLen = 0;
	int	inRetVal = VS_SUCCESS;
	int	inFind = VS_FALSE;
	char	szDebugMsg[128 + 1];
	char	szQuerySql[200 + 1];	/* INSERT INTO	szTableName( */
	char	szTagName[_TAG_WIDTH_ + 1];
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_ALL_TABLE	srAll;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq EMV GetBatchByCnt OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] Line[%d]", szTableName,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}

	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_EMVRec(pobTran, &srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Sq EMV GetBatchByCnt LinkEmvRec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] Line[%d]", szTableName,  __LINE__);
		
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq EMV GetBatchByCnt Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	if (pobTran->inTableID >= 0)
	{
		sprintf(szQuerySql, "SELECT * FROM %s WHERE (inBatchTableID = %d)", szTableName, pobTran->inTableID);
	}
	else
	{

		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq EMV GetBatchByCnt Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}		
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}
		inDISP_DispLogAndWriteFlie("  inTableID[%d] *Error* Line[%d]", pobTran->inTableID, __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq EMV GetBatchByCnt Prepare *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql, __LINE__);
	}

	inFunc_CalculateRunTimeGlobal_Start();
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		/* 替換資料前先清空srBRec */
		memset(&pobTran->srEMVRec, 0x00, sizeof(pobTran->srEMVRec));
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inDISP_LogPrintf(szDebugMsg);
		}
		
		/* 釋放事務 */
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq EMV GetBatchByCnt finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}

		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq EMV GetBatchByCnt Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
		
		return (VS_NO_RECORD);
	}
	else
	{
		inDISP_DispLogAndWriteFlie("  Sq EMV GetBatchByCnt Step *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szErrorMessage[%s] Line[%d]", sqlite3_errmsg(srDBConnection) , __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql, __LINE__);		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq EMV GetBatchByCnt finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}

		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq EMV GetBatchByCnt Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}
		
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	inCols = sqlite3_column_count(srSQLStat);

	/* binding 取得值 */
	for (j = 0; j < inCols; j++)
	{
		inFind = VS_FALSE;
		memset(szTagName, 0x00, sizeof(szTagName));
		strcat(szTagName, sqlite3_column_name(srSQLStat, j));


		for (inIntIndex = 0; inIntIndex < srAll.inIntNum; inIntIndex++)
		{
			if (srAll.srInt[inIntIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}


			/* 比對Tag Name */
			if (memcmp(szTagName, srAll.srInt[inIntIndex].szTag, strlen(srAll.srInt[inIntIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srInt[inIntIndex].szTag))
			{
				*(int32_t*)srAll.srInt[inIntIndex].pTagValue = sqlite3_column_int(srSQLStat, j);
				srAll.srInt[inIntIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inInt64tIndex = 0; inInt64tIndex < srAll.inInt64tNum; inInt64tIndex++)
		{
			if (srAll.srInt64t[inInt64tIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll.srInt64t[inInt64tIndex].szTag, strlen(srAll.srInt64t[inInt64tIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srInt64t[inInt64tIndex].szTag))
			{
				*(int64_t*)srAll.srInt64t[inInt64tIndex].pTagValue = sqlite3_column_int64(srSQLStat, j);
				srAll.srInt64t[inInt64tIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inCharIndex = 0; inCharIndex < srAll.inCharNum; inCharIndex++)
		{
			if (srAll.srChar[inCharIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll.srChar[inCharIndex].szTag, strlen(srAll.srChar[inCharIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srChar[inCharIndex].szTag))
			{
				inDataLen = sqlite3_column_bytes(srSQLStat, j);
				memcpy(srAll.srChar[inCharIndex].pCharVariable, sqlite3_column_blob(srSQLStat, j), inDataLen);
				srAll.srChar[inCharIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Find no variable to insert:Tag: %s", szTagName);
			inDISP_LogPrintf(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Value: %s", (char*)sqlite3_column_blob(srSQLStat, j));
			inDISP_LogPrintf(szDebugMsg);
		}

	}

	inFunc_WatchRunTime();

	inRc = sqlite3_finalize(srSQLStat);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq EMV GetBatchByCnt finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
	}

	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq EMV GetBatchByCnt Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}


#ifdef _MODIFY_NEW_ENORMOUS_SERACH_

/*
Function        :inSqlite_Get_Batch_ByCnt_Enormous_Search
Date&Time       :2017/3/21 下午 1:53
Describe        :
*/
int inSqlite_Get_Batch_ByCnt_Enormous_Search(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int	inRc;
	int	inRetVal;
	char	szDebugMsg[128 + 1];
	char	szQuerySql[200 + 1];	/* INSERT INTO	szTableName( */
	char	*szErrorMessage = NULL;
	sqlite3	*srDBConnection;	/* 建立到資料庫的connection */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inFunc_CalculateRunTimeGlobal_Start();

	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchByCnt Enor_Search OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] Line[%d]", szTableName,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}

	inFunc_WatchRunTime();

	/* 塞入SQL語句 */
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "SELECT * FROM %s WHERE (uszUpdated = 0)ORDER BY lnOrgInvNum ASC", szTableName);

	memset(&gsrResult, 0x00, sizeof(gsrResult));

	inFunc_CalculateRunTimeGlobal_Start();
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(gsrDBConnection, szQuerySql, -1, &gsrSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	/* 初始化計數 */
	ginEnormousNowCnt = 0;
	
	inFunc_WatchRunTime();
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Batch_ByCnt_Enormous_Get
Date&Time       :2017/3/21 下午 1:53
Describe        :
*/
int inSqlite_Get_Batch_ByCnt_Enormous_Get(TRANSACTION_OBJECT *pobTran, char* szTableName, int inRecCnt)
{
	int	inRetVal;
	int	i = 0, j = 0;
	int	inIntIndex = 0, inInt64tIndex = 0, inCharIndex = 0;
	int	inCols = 0, inDataLen = 0;
	int	inFind = VS_FALSE;
	char	szDebugMsg[100 + 1];
	char	szTagName[_TAG_WIDTH_ + 1] = {0};
	SQLITE_ALL_TABLE	srAll;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 新增需要逐筆列印時不用重新讀取DB問題  START 2020/3/5 下午 1:27 [SAM] */
	/* 延續使用，不重新搜尋 */
	if (guszEnormousNoNeedResetBit == VS_TRUE)
	{
		
	}
	else
	{
		/* 重置前一次結果 */
		sqlite3_reset(gsrSQLStat);
		ginEnormousNowCnt = 0;
	}
	
	/* 取得 database 裡所有的資料 */
	for (; ginEnormousNowCnt <= inRecCnt; ginEnormousNowCnt++)
	{
		/* Qerry */
		inRetVal = sqlite3_step(gsrSQLStat);
		if (inRetVal == SQLITE_ROW)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Get Table OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else if (inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "NO DATA");
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 釋放事務 */
			inSqlite_SQL_Finalize(&gsrSQLStat);
			inSqlite_DB_Close(&gsrDBConnection);
			return (VS_NO_RECORD);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
				inLogPrintf(AT, szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 釋放事務 */
			inSqlite_SQL_Finalize(&gsrSQLStat);
			/* 關閉 database, close null pointer 是NOP(No Operation) */
			if (inSqlite_DB_Close(&gsrDBConnection) != VS_SUCCESS)
			{
				/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
				return (VS_ERROR);
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "Close Database OK");
					inLogPrintf(AT, szDebugMsg);
				}
			}

			return (VS_ERROR);
		}
	}
	/* 新增需要逐筆列印時不用重新讀取DB問題  END 2020/3/5 下午 1:27 [SAM] */
	
	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_BRec(pobTran, &srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchByCnt Enor_Get LinkBRec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] inRecCnt[%d] Line[%d]", szTableName, inRecCnt,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	/* 替換資料前先清空srBRec */
	memset(&pobTran->srBRec, 0x00, sizeof(pobTran->srBRec));

	/* 因為只會抓到一筆資料，所以inRows值會是1(表示只有兩行資料) 只做i = 0搜尋TagName, 從i = 1開始 */
	i = inRecCnt + 1;
	inFunc_CalculateRunTimeGlobal_Start();
	for (j = 0; j < gsrResult.inCols; j++)
	{
		inFind = VS_FALSE;
		/* 如果database中的值是NULL，用此function規避，不然會溢位 */
		if (gsrResult.szResult[i * gsrResult.inCols + j] == NULL)
		{
			continue;
		}

		for (inIntIndex = 0; inIntIndex < srAll.inIntNum; inIntIndex++)
		{
			if (srAll.srInt[inIntIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(gsrResult.szResult[0 * gsrResult.inCols + j], srAll.srInt[inIntIndex].szTag, strlen(srAll.srInt[inIntIndex].szTag)) == 0	||
			    strlen(gsrResult.szResult[0 * gsrResult.inCols + j]) == strlen(srAll.srInt[inIntIndex].szTag))
			{
				*(int32_t*)srAll.srInt[inIntIndex].pTagValue = atoi(gsrResult.szResult[i * gsrResult.inCols + j]);
				srAll.srInt[inIntIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

//					if (ginDebug == VS_TRUE)
//					{
//						char szDebugMsg[100 + 1];
//
//						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
//						sprintf(szDebugMsg, "%d", *srAll.srInt[inIntIndex].inTagValue);
//						inDISP_LogPrintf(szDebugMsg);
//					}
				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inInt64tIndex = 0; inInt64tIndex < srAll.inInt64tNum; inInt64tIndex++)
		{
			if (srAll.srInt64t[inInt64tIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(gsrResult.szResult[0 * gsrResult.inCols + j], srAll.srInt64t[inInt64tIndex].szTag, strlen(srAll.srInt64t[inInt64tIndex].szTag)) == 0	&&
			    strlen(gsrResult.szResult[0 * gsrResult.inCols + j]) == strlen(srAll.srInt64t[inInt64tIndex].szTag))
			{
				*(int64_t*)srAll.srInt64t[inInt64tIndex].pTagValue = strtoll(gsrResult.szResult[i * gsrResult.inCols + j], NULL, 10);
				srAll.srInt64t[inInt64tIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

//					if (ginDebug == VS_TRUE)
//					{
//						char szDebugMsg[100 + 1];
//
//						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
//						sprintf(szDebugMsg, "%ld", *TABLE_srBRec_GetInt64tAll[inInt64tIndex].lnTagValue);
//						inDISP_LogPrintf(szDebugMsg);
//					}
				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inCharIndex = 0; inCharIndex < srAll.inCharNum; inCharIndex++)
		{
			if (srAll.srChar[inCharIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(gsrResult.szResult[0 * gsrResult.inCols + j], srAll.srChar[inCharIndex].szTag, strlen(srAll.srChar[inCharIndex].szTag)) == 0	&&
			    strlen(gsrResult.szResult[0 * gsrResult.inCols + j]) == strlen(srAll.srChar[inCharIndex].szTag))
			{
				if (strlen(gsrResult.szResult[i * gsrResult.inCols + j]) > 0)
				{
					memcpy(srAll.srChar[inCharIndex].pCharVariable, gsrResult.szResult[i * gsrResult.inCols + j], strlen(gsrResult.szResult[i * gsrResult.inCols + j]));
				}
				else
				{
					memset(srAll.srChar[inCharIndex].pCharVariable, 0x00, 1);
				}
				srAll.srChar[inCharIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

//					if (ginDebug == VS_TRUE)
//					{
//						char szDebugMsg[100 + 1];
//
//						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
//						sprintf(szDebugMsg, "%s:%s", TABLE_srBRec_GetCharAll[inCharIndex].szTag, TABLE_srBRec_GetCharAll[inCharIndex].pCharVariable);
//						inDISP_LogPrintf(szDebugMsg);
//					}
				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Find no variable to insert:Tag: %s", gsrResult.szResult[0 * gsrResult.inCols + j]);
			inDISP_LogPrintf(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Value: %s", gsrResult.szResult[i * gsrResult.inCols + j]);
			inDISP_LogPrintf(szDebugMsg);
		}

	}
	inFunc_WatchRunTime();

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_EMV_ByCnt_Enormous_Get
Date&Time       :2017/3/21 下午 1:53
Describe        :
*/
int inSqlite_Get_EMV_ByCnt_Enormous_Get(TRANSACTION_OBJECT *pobTran, char* szTableName, int inRecCnt)
{
	int	inRetVal;
	int	i, j;
	int	inIntIndex = 0, inInt64tIndex = 0, inCharIndex = 0;
	int	inFind = VS_FALSE;
	char	szDebugMsg[100 + 1];
	SQLITE_ALL_TABLE	srAll;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 新增需要逐筆列印時不用重新讀取DB問題  START 2020/3/5 下午 1:27 [SAM] */
	/* 延續使用，不重新搜尋 */
	if (guszEnormousNoNeedResetBit == VS_TRUE)
	{
		
	}
	else
	{
		/* 重置前一次結果 */
		sqlite3_reset(gsrSQLStat);
		ginEnormousNowCnt = 0;
	}
	
	/* 取得 database 裡所有的資料 */
	for (; ginEnormousNowCnt <= inRecCnt; ginEnormousNowCnt++)
	{
		/* Qerry */
		inRetVal = sqlite3_step(gsrSQLStat);
		if (inRetVal == SQLITE_ROW)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Get Table OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else if (inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "NO DATA");
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 釋放事務 */
			inSqlite_SQL_Finalize(&gsrSQLStat);
			inSqlite_DB_Close(&gsrDBConnection);
			return (VS_NO_RECORD);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
				inLogPrintf(AT, szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
				inLogPrintf(AT, szDebugMsg);
			}

			/* 釋放事務 */
			inSqlite_SQL_Finalize(&gsrSQLStat);
			/* 關閉 database, close null pointer 是NOP(No Operation) */
			if (inSqlite_DB_Close(&gsrDBConnection) != VS_SUCCESS)
			{
				/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
				return (VS_ERROR);
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "Close Database OK");
					inLogPrintf(AT, szDebugMsg);
				}
			}

			return (VS_ERROR);
		}
	}
	/* 新增需要逐筆列印時不用重新讀取DB問題  END 2020/3/5 下午 1:27 [SAM] */
	
	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_EMVRec(pobTran, &srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Sq EMV GetBatchByCnt Enor_Get LinkEMVRec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] inRecCnt[%d] Line[%d]", szTableName, inRecCnt, __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	/* 因為只會抓到一筆資料，所以inRows值會是1(表示只有兩行資料) 只做i = 0搜尋TagName, 從i = 1開始 */
	i = inRecCnt + 1;
	inFunc_CalculateRunTimeGlobal_Start();
	for (j = 0; j < gsrResult.inCols; j++)
	{
		inFind = VS_FALSE;
		/* 如果database中的值是NULL，用此function規避，不然會溢位 */
		if (gsrResult.szResult[i * gsrResult.inCols + j] == NULL)
		{
			continue;
		}

		for (inIntIndex = 0; inIntIndex < srAll.inIntNum; inIntIndex++)
		{
			if (srAll.srInt[inIntIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(gsrResult.szResult[0 * gsrResult.inCols + j], srAll.srInt[inIntIndex].szTag, strlen(srAll.srInt[inIntIndex].szTag)) == 0	||
			    strlen(gsrResult.szResult[0 * gsrResult.inCols + j]) == strlen(srAll.srInt[inIntIndex].szTag))
			{
				*(int32_t*)srAll.srInt[inIntIndex].pTagValue = atoi(gsrResult.szResult[i * gsrResult.inCols + j]);
				srAll.srInt[inIntIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

//					if (ginDebug == VS_TRUE)
//					{
//						char szDebugMsg[100 + 1];
//
//						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
//						sprintf(szDebugMsg, "%d", *srAll.srInt[inIntIndex].inTagValue);
//						inDISP_LogPrintf(szDebugMsg);
//					}
				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inInt64tIndex = 0; inInt64tIndex < srAll.inInt64tNum; inInt64tIndex++)
		{
			if (srAll.srInt64t[inInt64tIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(gsrResult.szResult[0 * gsrResult.inCols + j], srAll.srInt64t[inInt64tIndex].szTag, strlen(srAll.srInt64t[inInt64tIndex].szTag)) == 0	&&
			    strlen(gsrResult.szResult[0 * gsrResult.inCols + j]) == strlen(srAll.srInt64t[inInt64tIndex].szTag))
			{
				*(int64_t*)srAll.srInt64t[inInt64tIndex].pTagValue = strtoll(gsrResult.szResult[i * gsrResult.inCols + j], NULL, 10);
				srAll.srInt64t[inInt64tIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

//					if (ginDebug == VS_TRUE)
//					{
//						char szDebugMsg[100 + 1];
//
//						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
//						sprintf(szDebugMsg, "%ld", *TABLE_srBRec_GetInt64tAll[inInt64tIndex].lnTagValue);
//						inDISP_LogPrintf(szDebugMsg);
//					}
				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inCharIndex = 0; inCharIndex < srAll.inCharNum; inCharIndex++)
		{
			if (srAll.srChar[inCharIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(gsrResult.szResult[0 * gsrResult.inCols + j], srAll.srChar[inCharIndex].szTag, strlen(srAll.srChar[inCharIndex].szTag)) == 0	&&
			    strlen(gsrResult.szResult[0 * gsrResult.inCols + j]) == strlen(srAll.srChar[inCharIndex].szTag))
			{
				if (strlen(gsrResult.szResult[i * gsrResult.inCols + j]) > 0)
				{
					memcpy(srAll.srChar[inCharIndex].pCharVariable, gsrResult.szResult[i * gsrResult.inCols + j], strlen(gsrResult.szResult[i * gsrResult.inCols + j]));
				}
				else
				{
					memset(srAll.srChar[inCharIndex].pCharVariable, 0x00, 1);
				}
				srAll.srChar[inCharIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

//					if (ginDebug == VS_TRUE)
//					{
//						char szDebugMsg[100 + 1];
//
//						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
//						sprintf(szDebugMsg, "%s:%s", TABLE_srBRec_GetCharAll[inCharIndex].szTag, TABLE_srBRec_GetCharAll[inCharIndex].pCharVariable);
//						inDISP_LogPrintf(szDebugMsg);
//					}
				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Find no variable to insert:Tag: %s", gsrResult.szResult[0 * gsrResult.inCols + j]);
			inDISP_LogPrintf(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Value: %s", gsrResult.szResult[i * gsrResult.inCols + j]);
			inDISP_LogPrintf(szDebugMsg);
		}

	}
	inFunc_WatchRunTime();

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Batch_ByCnt_Enormous_Free
Date&Time       :2017/3/21 下午 1:55
Describe        :用完資料Free
*/
int inSqlite_Get_Batch_ByCnt_Enormous_Free()
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inFunc_CalculateRunTimeGlobal_Start();

	sqlite3_free_table(gsrResult.szResult);

	/* 新增需要逐筆列印時不用重新讀取DB問題  START 2020/3/5 下午 1:27 [SAM] */
	/* 計數歸0 */
	ginEnormousNowCnt = 0;
	guszEnormousNoNeedResetBit = VS_FALSE;
	/* 新增需要逐筆列印時不用重新讀取DB問題  END 2020/3/5 下午 1:27 [SAM] */
	
	
	/* 初始化結構 */
	memset(&gsrResult, 0x00, sizeof(gsrResult));

	inFunc_WatchRunTime();

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

#else
/*
Function        :inSqlite_Get_Batch_ByCnt_Enormous_Search
Date&Time       :2017/3/21 下午 1:53
Describe        :
*/
int inSqlite_Get_Batch_ByCnt_Enormous_Search(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int	inRc;
	int	inRetVal;
	char	szDebugMsg[128 + 1];
	char	szQuerySql[200 + 1];	/* INSERT INTO	szTableName( */
	char	*szErrorMessage = NULL;
	sqlite3	*srDBConnection;	/* 建立到資料庫的connection */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inFunc_CalculateRunTimeGlobal_Start();

	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchByCnt Enor_Search OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] Line[%d]", szTableName,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}

	inFunc_WatchRunTime();

	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "SELECT * FROM %s WHERE (uszUpdated = 0)ORDER BY lnOrgInvNum ASC", szTableName);

	memset(&gsrResult, 0x00, sizeof(gsrResult));

	inFunc_CalculateRunTimeGlobal_Start();
	/* 取得 database 裡所有的資料 */
	inRetVal = sqlite3_get_table(srDBConnection , szQuerySql, &(gsrResult.szResult) , &(gsrResult.inRows), &(gsrResult.inCols), &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchByCnt Enor_Search GetTable *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szErrorMessage[%s] Line[%d]", szErrorMessage,  __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql,  __LINE__);
		
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBatchByCnt Enor_Search Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
		
		/* 因為直接get 不存在的table回傳值是-1，只有在Error Msg才能得知錯誤原因 */
		if (memcmp(szErrorMessage, "no such table", strlen("no such table")) == 0)
		{
			inDISP_DispLogAndWriteFlie("  No Such Table Line[%d] ", __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_NO_RECORD);
		}
		else
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBatchByCnt Enor_Search GetTable *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
			inDISP_DispLogAndWriteFlie("  Reason[%s] Line[%d]", szErrorMessage , __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get table OK");
			inDISP_LogPrintf(szDebugMsg);
		}

	}
	inFunc_WatchRunTime();

	/* 如果沒有符合搜尋條件的資料，仍會get table OK，但抓不到資料，不符目的 */
	if (gsrResult.inRows < 1)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchByCnt Enor_Search GetTable *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  NoData inRows[%d] Line[%d]", gsrResult.inRows,  __LINE__);
		
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBatchByCnt Enor_Search Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_NO_RECORD);
	}

	/* 關閉 database, close null pointer 是NOP(No Operation) */
	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchByCnt Enor_Search Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Batch_ByCnt_Enormous_Get
Date&Time       :2017/3/21 下午 1:53
Describe        :
*/
int inSqlite_Get_Batch_ByCnt_Enormous_Get(TRANSACTION_OBJECT *pobTran, char* szTableName, int inRecCnt)
{
	int	inRetVal;
	int	i = 0, j = 0;
	int	inIntIndex = 0, inInt64tIndex = 0, inCharIndex = 0;
	int	inFind = VS_FALSE;
	char	szDebugMsg[100 + 1];
	SQLITE_ALL_TABLE	srAll;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 如果database中的值是NULL，用此function規避，不然會溢位 */
	if (gsrResult.szResult[(inRecCnt + 1) * gsrResult.inCols + 0] == NULL)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchByCnt Enor_Get Result NULL *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] inRecCnt[%d] inCols[d%] Line[%d]", szTableName, inRecCnt, gsrResult.inCols, __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
		
	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_BRec(pobTran, &srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchByCnt Enor_Get LinkBRec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] inRecCnt[%d] Line[%d]", szTableName, inRecCnt,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	/* 替換資料前先清空srBRec */
	memset(&pobTran->srBRec, 0x00, sizeof(pobTran->srBRec));

	/* 因為只會抓到一筆資料，所以inRows值會是1(表示只有兩行資料) 只做i = 0搜尋TagName, 從i = 1開始 */
	i = inRecCnt + 1;
	inFunc_CalculateRunTimeGlobal_Start();
	for (j = 0; j < gsrResult.inCols; j++)
	{
		inFind = VS_FALSE;
		/* 如果database中的值是NULL，用此function規避，不然會溢位 */
		if (gsrResult.szResult[i * gsrResult.inCols + j] == NULL)
		{
			continue;
		}

		for (inIntIndex = 0; inIntIndex < srAll.inIntNum; inIntIndex++)
		{
			if (srAll.srInt[inIntIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(gsrResult.szResult[0 * gsrResult.inCols + j], srAll.srInt[inIntIndex].szTag, strlen(srAll.srInt[inIntIndex].szTag)) == 0	||
			    strlen(gsrResult.szResult[0 * gsrResult.inCols + j]) == strlen(srAll.srInt[inIntIndex].szTag))
			{
				*(int32_t*)srAll.srInt[inIntIndex].pTagValue = atoi(gsrResult.szResult[i * gsrResult.inCols + j]);
				srAll.srInt[inIntIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

//					if (ginDebug == VS_TRUE)
//					{
//						char szDebugMsg[100 + 1];
//
//						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
//						sprintf(szDebugMsg, "%d", *srAll.srInt[inIntIndex].inTagValue);
//						inDISP_LogPrintf(szDebugMsg);
//					}
				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inInt64tIndex = 0; inInt64tIndex < srAll.inInt64tNum; inInt64tIndex++)
		{
			if (srAll.srInt64t[inInt64tIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(gsrResult.szResult[0 * gsrResult.inCols + j], srAll.srInt64t[inInt64tIndex].szTag, strlen(srAll.srInt64t[inInt64tIndex].szTag)) == 0	&&
			    strlen(gsrResult.szResult[0 * gsrResult.inCols + j]) == strlen(srAll.srInt64t[inInt64tIndex].szTag))
			{
				*(int64_t*)srAll.srInt64t[inInt64tIndex].pTagValue = strtoll(gsrResult.szResult[i * gsrResult.inCols + j], NULL, 10);
				srAll.srInt64t[inInt64tIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

//					if (ginDebug == VS_TRUE)
//					{
//						char szDebugMsg[100 + 1];
//
//						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
//						sprintf(szDebugMsg, "%ld", *TABLE_srBRec_GetInt64tAll[inInt64tIndex].lnTagValue);
//						inDISP_LogPrintf(szDebugMsg);
//					}
				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inCharIndex = 0; inCharIndex < srAll.inCharNum; inCharIndex++)
		{
			if (srAll.srChar[inCharIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(gsrResult.szResult[0 * gsrResult.inCols + j], srAll.srChar[inCharIndex].szTag, strlen(srAll.srChar[inCharIndex].szTag)) == 0	&&
			    strlen(gsrResult.szResult[0 * gsrResult.inCols + j]) == strlen(srAll.srChar[inCharIndex].szTag))
			{
				if (strlen(gsrResult.szResult[i * gsrResult.inCols + j]) > 0)
				{
					memcpy(srAll.srChar[inCharIndex].pCharVariable, gsrResult.szResult[i * gsrResult.inCols + j], strlen(gsrResult.szResult[i * gsrResult.inCols + j]));
				}
				else
				{
					memset(srAll.srChar[inCharIndex].pCharVariable, 0x00, 1);
				}
				srAll.srChar[inCharIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

//					if (ginDebug == VS_TRUE)
//					{
//						char szDebugMsg[100 + 1];
//
//						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
//						sprintf(szDebugMsg, "%s:%s", TABLE_srBRec_GetCharAll[inCharIndex].szTag, TABLE_srBRec_GetCharAll[inCharIndex].pCharVariable);
//						inDISP_LogPrintf(szDebugMsg);
//					}
				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Find no variable to insert:Tag: %s", gsrResult.szResult[0 * gsrResult.inCols + j]);
			inDISP_LogPrintf(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Value: %s", gsrResult.szResult[i * gsrResult.inCols + j]);
			inDISP_LogPrintf(szDebugMsg);
		}

	}
	inFunc_WatchRunTime();

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_EMV_ByCnt_Enormous_Get
Date&Time       :2017/3/21 下午 1:53
Describe        :
*/
int inSqlite_Get_EMV_ByCnt_Enormous_Get(TRANSACTION_OBJECT *pobTran, char* szTableName, int inRecCnt)
{
	int	inRetVal;
	int	i, j;
	int	inIntIndex = 0, inInt64tIndex = 0, inCharIndex = 0;
	int	inFind = VS_FALSE;
	char	szDebugMsg[100 + 1];
	SQLITE_ALL_TABLE	srAll;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
		
	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_EMVRec(pobTran, &srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Sq EMV GetBatchByCnt Enor_Get LinkEMVRec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] inRecCnt[%d] Line[%d]", szTableName, inRecCnt, __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	/* 因為只會抓到一筆資料，所以inRows值會是1(表示只有兩行資料) 只做i = 0搜尋TagName, 從i = 1開始 */
	i = inRecCnt + 1;
	inFunc_CalculateRunTimeGlobal_Start();
	for (j = 0; j < gsrResult.inCols; j++)
	{
		inFind = VS_FALSE;
		/* 如果database中的值是NULL，用此function規避，不然會溢位 */
		if (gsrResult.szResult[i * gsrResult.inCols + j] == NULL)
		{
			continue;
		}

		for (inIntIndex = 0; inIntIndex < srAll.inIntNum; inIntIndex++)
		{
			if (srAll.srInt[inIntIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(gsrResult.szResult[0 * gsrResult.inCols + j], srAll.srInt[inIntIndex].szTag, strlen(srAll.srInt[inIntIndex].szTag)) == 0	||
			    strlen(gsrResult.szResult[0 * gsrResult.inCols + j]) == strlen(srAll.srInt[inIntIndex].szTag))
			{
				*(int32_t*)srAll.srInt[inIntIndex].pTagValue = atoi(gsrResult.szResult[i * gsrResult.inCols + j]);
				srAll.srInt[inIntIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

//					if (ginDebug == VS_TRUE)
//					{
//						char szDebugMsg[100 + 1];
//
//						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
//						sprintf(szDebugMsg, "%d", *srAll.srInt[inIntIndex].inTagValue);
//						inDISP_LogPrintf(szDebugMsg);
//					}
				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inInt64tIndex = 0; inInt64tIndex < srAll.inInt64tNum; inInt64tIndex++)
		{
			if (srAll.srInt64t[inInt64tIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(gsrResult.szResult[0 * gsrResult.inCols + j], srAll.srInt64t[inInt64tIndex].szTag, strlen(srAll.srInt64t[inInt64tIndex].szTag)) == 0	&&
			    strlen(gsrResult.szResult[0 * gsrResult.inCols + j]) == strlen(srAll.srInt64t[inInt64tIndex].szTag))
			{
				*(int64_t*)srAll.srInt64t[inInt64tIndex].pTagValue = strtoll(gsrResult.szResult[i * gsrResult.inCols + j], NULL, 10);
				srAll.srInt64t[inInt64tIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

//					if (ginDebug == VS_TRUE)
//					{
//						char szDebugMsg[100 + 1];
//
//						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
//						sprintf(szDebugMsg, "%ld", *TABLE_srBRec_GetInt64tAll[inInt64tIndex].lnTagValue);
//						inDISP_LogPrintf(szDebugMsg);
//					}
				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inCharIndex = 0; inCharIndex < srAll.inCharNum; inCharIndex++)
		{
			if (srAll.srChar[inCharIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(gsrResult.szResult[0 * gsrResult.inCols + j], srAll.srChar[inCharIndex].szTag, strlen(srAll.srChar[inCharIndex].szTag)) == 0	&&
			    strlen(gsrResult.szResult[0 * gsrResult.inCols + j]) == strlen(srAll.srChar[inCharIndex].szTag))
			{
				if (strlen(gsrResult.szResult[i * gsrResult.inCols + j]) > 0)
				{
					memcpy(srAll.srChar[inCharIndex].pCharVariable, gsrResult.szResult[i * gsrResult.inCols + j], strlen(gsrResult.szResult[i * gsrResult.inCols + j]));
				}
				else
				{
					memset(srAll.srChar[inCharIndex].pCharVariable, 0x00, 1);
				}
				srAll.srChar[inCharIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

//					if (ginDebug == VS_TRUE)
//					{
//						char szDebugMsg[100 + 1];
//
//						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
//						sprintf(szDebugMsg, "%s:%s", TABLE_srBRec_GetCharAll[inCharIndex].szTag, TABLE_srBRec_GetCharAll[inCharIndex].pCharVariable);
//						inDISP_LogPrintf(szDebugMsg);
//					}
				break;
			}

		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Find no variable to insert:Tag: %s", gsrResult.szResult[0 * gsrResult.inCols + j]);
			inDISP_LogPrintf(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Value: %s", gsrResult.szResult[i * gsrResult.inCols + j]);
			inDISP_LogPrintf(szDebugMsg);
		}

	}
	inFunc_WatchRunTime();

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Batch_ByCnt_Enormous_Free
Date&Time       :2017/3/21 下午 1:55
Describe        :用完資料Free
*/
int inSqlite_Get_Batch_ByCnt_Enormous_Free()
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inFunc_CalculateRunTimeGlobal_Start();

	sqlite3_free_table(gsrResult.szResult);
	
	/* 初始化結構 */
	memset(&gsrResult, 0x00, sizeof(gsrResult));

	inFunc_WatchRunTime();

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}
#endif
/*
Function        :inSqlite_Get_Batch_Count_Flow
Date&Time       :2017/3/14 下午 3:27
Describe        :這邊是抓有效交易紀錄的筆數
*/
int inSqlite_Get_Batch_Count_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int *inTableCount)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		default :
			inDISP_DispLogAndWriteFlie("  TableName Not Found[%d] Line[%d]",inTableType, __LINE__);
			break;
	}

	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_EMV_TABLE_:
		case _TN_BATCH_TABLE_TICKET_:
			inRetVal = inSqlite_Get_Batch_Count(pobTran, szTableName, inTableCount);
			break;
		default :
			inDISP_DispLogAndWriteFlie("  Sql Table Not Found[%d] Line[%d]",inTableType, __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
Function        :inSqlite_Get_Batch_Count
Date&Time       :2017/3/15 下午 4:36
Describe        :這邊是抓有效交易紀錄的筆數
*/
int inSqlite_Get_Batch_Count(TRANSACTION_OBJECT *pobTran, char *szTableName, int *inTableCount)
{
	int	inRc;
	int	j = 0;
	int	inRetVal;
	char	szDebugMsg[128 + 1];
	char	szQuerySql[300 + 1];	/* INSERT INTO	szTableName( */
	char	szErrorMessage[100 + 1];
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt	*srSQLStat;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchCnt OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] inTableCount[%d] Line[%d]", szTableName, *inTableCount,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}
	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));

	/* SQLite COUNT 計算資料庫中 table的行數 */
	sprintf(szQuerySql, "SELECT count(*) FROM %s WHERE (uszUpdated = 0)", szTableName);

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchCnt Prepare *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql, __LINE__);
	}

	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inDISP_LogPrintf(szDebugMsg);
		}
		
		/* 釋放事務 */
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBatchCnt finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}
			
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBatchCnt Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
		
		inDISP_DispLogAndWriteFlie("  VS_NO_RECORD Line[%d]", __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_NO_RECORD);
	}
	else
	{
		memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
		strcpy(szErrorMessage, sqlite3_errmsg(srDBConnection));

		inDISP_DispLogAndWriteFlie("  Sq GetBatchCnt Step *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szErrorMessage[%s] Line[%d]", szErrorMessage , __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql, __LINE__);		
		
		/* 釋放事務 */
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBatchCnt finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}
			
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBatchCnt Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}

		/* 因為直接get 不存在的table回傳值是-1，只有在Error Msg才能得知錯誤原因 */
		if (memcmp(szErrorMessage, "no such table", strlen("no such table")) == 0)
		{
			inDISP_DispLogAndWriteFlie("  No Such Table Line[%d] ", __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_NO_RECORD);
		}
		else
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBatchByCnt Enor_Search GetTable *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
			inDISP_DispLogAndWriteFlie("  Reason[%s] Line[%d]", szErrorMessage , __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
		}
	}

	/* 若是成功，將值丟到輸入的位置。 */
	j = 0;
	*inTableCount =sqlite3_column_int(srSQLStat, j);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Get count(*) OK count:%d", *inTableCount);
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 釋放事務 */
	inRc = sqlite3_finalize(srSQLStat);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchCnt finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
	}

	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchCnt Close *Error* inRc[%d] Line[%d]", inRc,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Batch_Upload_Count_Flow
Date&Time       :2017/3/14 下午 3:27
Describe        :這邊是抓BathcUploaf有效交易紀錄的筆數
*/
int inSqlite_Get_Batch_Upload_Count_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int *inTableCount)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		default :
			inDISP_DispLogAndWriteFlie("  TableName Not Found[%d] Line[%d]",inTableType, __LINE__);
			break;
	}

	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_EMV_TABLE_:
			inRetVal = inSqlite_Get_Batch_Count(pobTran, szTableName, inTableCount);
			break;
		default :
			inDISP_DispLogAndWriteFlie("  Sql Table Not Found[%d] Line[%d]",inTableType, __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
	}
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
Function        :inSqlite_Get_Batch_Upload_Count
Date&Time       :2017/3/15 下午 4:36
Describe        :這邊是抓有效交易紀錄的筆數
*/
int inSqlite_Get_Batch_Upload_Count(TRANSACTION_OBJECT *pobTran, char *szTableName, int *inTableCount)
{
	int	inRc;
	int	inBindingIndex = 1;
	int	inCols;
	int	inRetVal;
	char	szDebugMsg[128 + 1];
	char	szQuerySql[300 + 1];	/* INSERT INTO	szTableName( */
	sqlite3_stmt	*srSQLStat;
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchUploadCnt OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] inTableCount[%d] Line[%d]", szTableName, *inTableCount,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}
	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));

	/* SQLite COUNT 計算資料庫中 table的行數 */
	/* 注意：X'01'這個寫法相當於0x01 */
	/* 注意：但X'00' 不等於0x00 */
	sprintf(szQuerySql, "SELECT count(*) FROM %s WHERE (uszUpdated = 0) "
		"AND (uszCLSBatchBit <> X'01') "
		"AND (uszVOIDBit <> X'01') "
		"AND (inCode <> ?) "
		"AND (inCode <> ?) "
		"AND (inCode <> ?) "
		"AND (inCode <> ?)", szTableName);

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchUploadCnt Prepare *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql, __LINE__);
	}

	/* index 從1開始 */
	inBindingIndex = 1;
	inRetVal = sqlite3_bind_int(srSQLStat, inBindingIndex, _PRE_AUTH_);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchUploadCnt BindInt *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  _PRE_AUTH_ inBindingIndex[%d]  Line[%d]", inBindingIndex,  __LINE__);
	}
	else
	{
		inBindingIndex++;
	}

	inRetVal = sqlite3_bind_int(srSQLStat, inBindingIndex, _CUP_PRE_AUTH_);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchUploadCnt BindInt *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  _CUP_PRE_AUTH_ inBindingIndex[%d]  Line[%d]", inBindingIndex,  __LINE__);
	}
	else
	{
		inBindingIndex++;
	}

	inRetVal = sqlite3_bind_int(srSQLStat, inBindingIndex, _LOYALTY_REDEEM_);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchUploadCnt BindInt *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  _LOYALTY_REDEEM_ inBindingIndex[%d]  Line[%d]", inBindingIndex,  __LINE__);
	}
	else
	{
		inBindingIndex++;
	}

	inRetVal = sqlite3_bind_int(srSQLStat, inBindingIndex, _VOID_LOYALTY_REDEEM_);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchUploadCnt BindInt *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  _VOID_LOYALTY_REDEEM_ inBindingIndex[%d]  Line[%d]", inBindingIndex,  __LINE__);
	}
	else
	{
		inBindingIndex++;
	}

	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		/* 替換資料前先清空srBRec */
		memset(&pobTran->srEMVRec, 0x00, sizeof(pobTran->srEMVRec));
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inDISP_LogPrintf(szDebugMsg);
		}
		
		/* 釋放事務 */
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBatchUploadCnt finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}
		
		inRc = sqlite3_close(srDBConnection);
		if (inRetVal != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBatchUploadCnt Close *Error* inRetVal[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
		return (VS_NO_RECORD);
	}
	else
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchUploadCnt Step *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szErrorMessage[%s] Line[%d]", sqlite3_errmsg(srDBConnection) , __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql, __LINE__);
		
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBatchUploadCnt finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}
				
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_close(srDBConnection);
		if (inRetVal != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq GetBatchUploadCnt Close *Error* inRetVal[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	inCols = sqlite3_column_count(srSQLStat);

	/* 若是成功，將值丟到輸入的位置。 */
	/* The leftmost column of the result set has the index 0.*/
	*inTableCount = sqlite3_column_int(srSQLStat, 0);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Get count(*) OK count:%d", *inTableCount);
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 釋放事務，若要重用則用sqlite3_reset */
	inRc = sqlite3_finalize(srSQLStat);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchUploadCnt finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
	}

	/* 關閉 database, close null pointer 是NOP(No Operation) */
	inRc = sqlite3_close(srDBConnection);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq GetBatchUploadCnt Close *Error* inRetVal[%d] Line[%d]", inRc,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Check_Table_Exist_Flow
Date&Time       :2017/3/17 上午 9:36
Describe        :By query "SELECT name FROM sqlite_master WHERE type='table' AND name='table_name';" 確認Table是否存在
*/
int inSqlite_Check_Table_Exist_Flow(TRANSACTION_OBJECT *pobTran, int inTableType)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		default :
			inDISP_DispLogAndWriteFlie("  TableName Not Found[%d] Line[%d]",inTableType, __LINE__);
			break;
	}

	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_EMV_TABLE_:
		case _TN_BATCH_TABLE_TICKET_:
			inRetVal = inSqlite_Check_Table_Exist(pobTran, szTableName);
			break;
		default :
			inDISP_DispLogAndWriteFlie("  Sql Table Not Found[%d] Line[%d]",inTableType, __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
	}
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
Function        :inSqlite_Check_Table_Exist
Date&Time       :2017/3/17 上午 10:00
Describe        :By query "SELECT name FROM sqlite_master WHERE type='table' AND name='table_name';" 確認Table是否存在
 *		sqlite_master 是 Sqlite的系統表，裡面存所有表的名稱
 *		需要另外注意的是暫存表存在sqlite_temp_master
*/
int inSqlite_Check_Table_Exist(TRANSACTION_OBJECT *pobTran, char *szTableName)
{
	int	inRc;
	int	inRetVal = VS_ERROR;
	char	szDebugMsg[128 + 1];
	char	szQuerySql[300 + 1];	/* INSERT INTO	szTableName( */
	char	szErrorMessage[100 + 1];
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt	*srSQLStat;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq CheckTableExist OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName[%s] Line[%d]", szTableName,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}
	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));

	/* SQLite COUNT 計算資料庫中 table的行數 */
	sprintf(szQuerySql, "SELECT name FROM sqlite_master WHERE type = 'table' AND name = '%s'", szTableName);

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq CheckTableExist Prepare *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql,  __LINE__);
	}

	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Exist : %s", szTableName);
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Not Exist : %s", szTableName);
			inDISP_LogPrintf(szDebugMsg);
		}
		
		/* 釋放事務 */
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq CheckTableExist finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}
		
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq CheckTableExist Close *Error* inRetVal[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
		
		return (VS_NO_RECORD);
	}
	else
	{
		memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
		strcpy(szErrorMessage, sqlite3_errmsg(srDBConnection));

		inDISP_DispLogAndWriteFlie("  Sq CheckTableExist Step *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szErrorMessage[%s] Line[%d]", szErrorMessage , __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql, __LINE__);		
		
		/* 釋放事務 */
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq CheckTableExist finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq CheckTableExist Close *Error* inRetVal[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}

		/* 因為直接get 不存在的table回傳值是-1，只有在Error Msg才能得知錯誤原因 */
		if (memcmp(szErrorMessage, "no such table", strlen("no such table")) == 0)
		{
			inDISP_DispLogAndWriteFlie("  No Such Table Line[%d] ", __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_NO_RECORD);
		}
		else
		{
			inDISP_DispLogAndWriteFlie("  Sq CheckTableExist *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
			inDISP_DispLogAndWriteFlie("  Reason[%s] Line[%d]", szErrorMessage , __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
		}
	}

	/* 釋放事務 */
	inRc = sqlite3_finalize(srSQLStat);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq CheckTableExist finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
	}

	/* 關閉 database, close null pointer 是NOP(No Operation) */
	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq CheckTableExist Close *Error* inRetVal[%d] Line[%d]", inRc,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Vacuum_Flow
Date&Time       :2017/3/17 下午 3:31
Describe        :Vacuum
*/
int inSqlite_Vacuum_Flow(TRANSACTION_OBJECT *pobTran, int inTableType)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		default :
			inDISP_DispLogAndWriteFlie("  TableName Not Found[%d] Line[%d]",inTableType, __LINE__);
			break;
	}

	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_EMV_TABLE_:
			inRetVal = inSqlite_Vacuum(pobTran, szTableName);
			break;
		default :
			inDISP_DispLogAndWriteFlie("  Sql Table Not Found[%d] Line[%d]",inTableType, __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
	}
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
Function        :inSqlite_Vacuum
Date&Time       :2017/3/17 下午 3:30
Describe        :Vacuum 指令不只可以對Table使用，也能對資料庫使用
*/
int inSqlite_Vacuum(TRANSACTION_OBJECT *pobTran, char* szTableName1)
{
	int	inRc;
	int	inRetVal;
	int	inSqlLength = 0;
	char	szSqlPrefix[40 + 2];		/* CREATE TABLE	szTableName( */
	char	szDebugMsg[100 + 1];
	char	*szVacummSql;			/* 因為會很長且隨table浮動，所以用pointer */
	char	*szErrorMessage = NULL;
	sqlite3	*srDBConnection;		/* 建立到資料庫的connection */


	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 開啟DataBase檔 */
	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Vacuum OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szTableName1[%s] Line[%d]", szTableName1,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}

	/* 塞入SQL語句 */
	/* 為了使table name可變動，所以拉出來組 */
	/* 加入了if not exists字串，若已建立不會重複建立 */
	/* 前綴 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "VACUUM %s;", szTableName1);
	/*  "CREATE TABLE	szTableName("的長度 */
	inSqlLength += strlen(szSqlPrefix);

	/* inSqlLength: */
	if (ginDebug == VS_TRUE)
        {
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", inSqlLength);
		inDISP_LogPrintf(szDebugMsg);
	}
	/* 配置記憶體(一定要+1，超級重要，不然會overflow) */
	szVacummSql = malloc(inSqlLength + 1);
	memset(szVacummSql, 0x00, inSqlLength);

	/* 先丟前綴Table Name */
	strcat(szVacummSql, szSqlPrefix);

	/* 建立 Table */
	inRetVal = sqlite3_exec(srDBConnection, szVacummSql, 0, 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Vacuum Exec *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szErrorMessage[%s] Line[%d]", szErrorMessage , __LINE__);
		inDISP_DispLogAndWriteFlie("  szVacummSql[%s] Line[%d]", szVacummSql, __LINE__);		

		free(szVacummSql);

		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq Vacuum Close *Error* inRetVal[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}

		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Vacuum Table OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}

	/* 釋放記憶體 */
	free(szVacummSql);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq Vacuum Close *Error* inRetVal[%d] Line[%d]", inRc,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}else{
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Check_Escape_Cnt
Date&Time       :2017/3/21 下午 4:23
Describe        :
*/
int inSqlite_Check_Escape_Cnt(char* szString)
{
	int	i;
	int	inCnt = 0;

	for (i = 0; i < strlen(szString); i++)
	{
		if (szString[i] == '\'')
		{
			inCnt++;
		}
	}


	return (inCnt);
}

/*
Function        :inSqlite_Process_Escape
Date&Time       :2017/3/21 下午 4:23
Describe        :
*/
int inSqlite_Process_Escape(char* szOldString, char* szNewString)
{
	int	i;
	int	inNewStringIndex = 0;

	for (i = 0; i < strlen(szOldString); i++)
	{
		szNewString[inNewStringIndex] = szOldString[i];
		inNewStringIndex++;

		if (szOldString[i] == '\'')
		{
			strcat(szNewString, "'");
			inNewStringIndex++;
		}

	}


	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_ESC_Sale_Upload_Count
Date&Time       :2016/4/29 下午 1:17
Describe        :可以取得table有幾筆資料
*/
int inSqlite_Get_ESC_Sale_Upload_Count(TRANSACTION_OBJECT *pobTran, char *szTableName, int inTxnType, unsigned char uszPaperBit, int *inTxnTotalCnt, long *lnTxnTotalAmt)
{
	int	inRc;
	int	j = 0;
	int	inRetVal = VS_ERROR;
	char	szDebugMsg[128 + 1] = {0};
	char	szQuerySql[3000 + 1] = {0};	/* INSERT INTO	szTableName( */
	char	szErrorMessage[100 + 1];
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt	*srSQLStat;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	inRetVal = sqlite3_open_v2(_DATA_BASE_NAME_NEXSYS_, &srDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq ESC GetSaleUpladCnt OpenV2 *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  TableName[%s] TxnType[%d] TxnTotalCnt[%d] TxnTotalAmt[%d] Line[%d]", szTableName, inTxnType, *inTxnTotalCnt, *lnTxnTotalAmt,  __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt ++;
	}
	/* 塞入SQL語句 */

	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));

	/* SQLite COUNT 計算資料庫中 table的行數 */
//先刪掉	
#ifdef _MODIFY_HDPT_FUNC_
	if (inTxnType == _ESC_REINFORCE_TXNCODE_SALE_)
	{
		if (uszPaperBit == VS_TRUE)
		{
			sprintf(szQuerySql, "SELECT count(*), sum(lnTotalTxnAmount) FROM %s WHERE "
				"((uszUpdated = %d) AND "
				"(inESCUploadStatus = %d OR "
				"inESCUploadStatus = %d) AND "
				"(incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d))",
				szTableName,
				0,
				_ESC_UPLOAD_STATUS_PAPER_,
				_ESC_UPLOAD_STATUS_UPLOAD_NOT_YET_,
				_SALE_,
				_CUP_SALE_,
				_INST_SALE_,
				_REDEEM_SALE_,
				_SALE_OFFLINE_,
				_INST_ADJUST_,
				_REDEEM_ADJUST_,
				_PRE_COMP_,
				_CUP_PRE_COMP_,
				_MAIL_ORDER_,
				_CUP_MAIL_ORDER_,
				_CASH_ADVANCE_,
				_FORCE_CASH_ADVANCE_,
				_FISC_SALE_,
				_TIP_);
		}
		else
		{
			sprintf(szQuerySql, "SELECT count(*), sum(lnTotalTxnAmount) FROM %s WHERE "
				"((uszUpdated = %d) AND "
				"(inESCUploadStatus = %d) AND "
				"(incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d))",
				szTableName,
				0,
				_ESC_UPLOAD_STATUS_UPLOADED_,
				_SALE_,
				_CUP_SALE_,
				_INST_SALE_,
				_REDEEM_SALE_,
				_SALE_OFFLINE_,
				_INST_ADJUST_,
				_REDEEM_ADJUST_,
				_PRE_COMP_,
				_CUP_PRE_COMP_,
				_MAIL_ORDER_,
				_CUP_MAIL_ORDER_,
				_CASH_ADVANCE_,
				_FORCE_CASH_ADVANCE_,
				_FISC_SALE_,
				_TIP_);
		}
	}
	else if (inTxnType == _ESC_REINFORCE_TXNCODE_REFUND_)
	{
		if (uszPaperBit == VS_TRUE)
		{
			sprintf(szQuerySql, "SELECT count(*), sum(lnTotalTxnAmount) FROM %s WHERE "
				"((uszUpdated = %d) AND "
				"(inESCUploadStatus = %d OR "
				"inESCUploadStatus = %d) AND "
				"(incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d))",
				szTableName,
				0,
				_ESC_UPLOAD_STATUS_PAPER_,
				_ESC_UPLOAD_STATUS_UPLOAD_NOT_YET_,
				_REFUND_,
				_REDEEM_REFUND_,
				_INST_REFUND_,
				_CUP_REFUND_,
				_CUP_MAIL_ORDER_REFUND_,
				_FISC_REFUND_);
		}
		else
		{
			sprintf(szQuerySql, "SELECT count(*), sum(lnTotalTxnAmount) FROM %s WHERE "
				"((uszUpdated = %d) AND "
				"(inESCUploadStatus = %d) AND "
				"(incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d))",
				szTableName,
				0,
				_ESC_UPLOAD_STATUS_UPLOADED_,
				_REFUND_,
				_REDEEM_REFUND_,
				_INST_REFUND_,
				_CUP_REFUND_,
				_CUP_MAIL_ORDER_REFUND_,
				_FISC_REFUND_);
		}
	}
	else
	{
		return (VS_ERROR);
	}
#endif
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq ESC GetSaleUpladCnt Prepare *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql, __LINE__);		

	}

	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inDISP_LogPrintf(szDebugMsg);
		}
		
		/* 釋放事務 */
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq ESC GetSaleUpladCnt finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}
		
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq ESC GetSaleUpladCnt Close *Error* inRetVal[%d] Line[%d]", inRc,  __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}else{
			st_inSqlitHandleCnt --;
		}
		
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_NO_RECORD);
	}
	else
	{
		memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
		strcpy(szErrorMessage, sqlite3_errmsg(srDBConnection));

		inDISP_DispLogAndWriteFlie("  Sq ESC GetSaleUpladCnt Step *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  szErrorMessage[%s] Line[%d]", szErrorMessage , __LINE__);
		inDISP_DispLogAndWriteFlie("  szQuerySql[%s] Line[%d]", szQuerySql, __LINE__);		
		
		/* 釋放事務 */
		inRc = sqlite3_finalize(srSQLStat);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq ESC GetSaleUpladCnt finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		}
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		inRc = sqlite3_close(srDBConnection);
		if (inRc != SQLITE_OK)
		{
			inDISP_DispLogAndWriteFlie("  Sq ESC GetSaleUpladCnt Close Rc[%d] *Error* Line[%d]", inRc, __LINE__);
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintf(szDebugMsg);
			}
			st_inSqlitHandleCnt --;
		}

		/* 因為直接get 不存在的table回傳值是-1，只有在Error Msg才能得知錯誤原因 */
		if (memcmp(szErrorMessage, "no such table", strlen("no such table")) == 0)
		{
			inDISP_DispLogAndWriteFlie(" NoSuchTable Line[%d]", __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_NO_RECORD);
		}
		else
		{
			inDISP_DispLogAndWriteFlie("  Sq ESC GetSaleUpladCnt  Step *Error* HDT[%d] Rvl[%d] Line[%d]",pobTran->srBRec.inHDTIndex, inRetVal, __LINE__);
			inDISP_DispLogAndWriteFlie("  Reason[%s] Line[%d]", szErrorMessage , __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
		}
	}

	/* 若是成功，將值丟到輸入的位置。 */
	j = 0;
	*inTxnTotalCnt = sqlite3_column_int(srSQLStat, j);
	j = 1;
	*lnTxnTotalAmt = sqlite3_column_int(srSQLStat, j);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Get count(*) OK count:%d Amt:%ld", *inTxnTotalCnt, *lnTxnTotalAmt);
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 釋放事務 */
	inRc = sqlite3_finalize(srSQLStat);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq ESC GetSaleUpladCnt finalize Rc[%d] *Error* Line[%d]", inRc, __LINE__);
	}

	/* 關閉 database, close null pointer 是NOP(No Operation) */
	inRc = sqlite3_close(srDBConnection);
	if (inRc != SQLITE_OK)
	{
		inDISP_DispLogAndWriteFlie("  Sq ESC GetSaleUpladCnt Close Rc[%d] *Error* Line[%d]", inRc, __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		st_inSqlitHandleCnt --;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Batch_To_DB_Test
Date&Time       :2016/4/11 下午 5:33
Describe        :將現有的batch檔轉成改以db紀錄，測試用
*/
int inSqlite_Batch_To_DB_Test(void)
{
//	int			i;
//	int			inRetVal;
//	long			lnCurrentInvoiceNumber;
//	char			szTable[16 + 1];
//	TRANSACTION_OBJECT	pobTran;
//
//	memset(&pobTran, 0x00, sizeof(pobTran));
//	pobTran.srBRec.inHDTIndex = -1;
//
//	inFunc_GetHostNum(&pobTran);
//
//	pobTran.srBRec.lnOrgInvNum = -1;
//	inBATCH_FuncUserChoice_By_Sqlite(&pobTran);
//	lnCurrentInvoiceNumber = pobTran.srBRec.lnOrgInvNum;
//
//	for (i = 1; i <= lnCurrentInvoiceNumber; i++)
//	{
//		inBATCH_GetTransRecord_By_Sqlite(&pobTran);
//		memcpy(pobTran.srBRec.szCardHolder, "'", strlen("'"));
//		inRetVal = inSqlite_Insert_All_Flow(&pobTran, _TN_BATCH_TABLE_);
//		if (inRetVal != VS_SUCCESS)
//		{
//			inDISP_Msg_BMP(_ERR_BATCH_, _COORDINATE_Y_LINE_8X16_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
//			break;
//		}
//		pobTran.srBRec.lnOrgInvNum++;
//	}


//	inFunc_AP_Folder_Copy_To_SD(_DATA_BASE_NAME_NEXSYS_);

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Table_Count_Test
Date&Time       :2016/5/4 上午 11:28
Describe        :算出table 資料筆數，測試用
*/
int inSqlite_Get_Table_Count_Test()
{
	int			inCnt = 0;
	TRANSACTION_OBJECT	pobTran;

	memset(&pobTran, 0x00, sizeof(pobTran));

	inSqlite_Get_Table_Count(&pobTran, _BATCH_TABLE_ESC_AGAIN_SUFFIX_, &inCnt);

	if (ginDebug == VS_TRUE)
	{
		char szDebugMsg[100 + 1];

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "%d", inCnt);
		inDISP_LogPrintf(szDebugMsg);
	}

	inSqlite_Get_Table_Count(&pobTran, _BATCH_TABLE_ESC_FAIL_SUFFIX_, &inCnt);

	if (ginDebug == VS_TRUE)
	{
		char szDebugMsg[100 + 1];

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "%d", inCnt);
		inDISP_LogPrintf(szDebugMsg);
	}

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Vacuum_Flow_Test
Date&Time       :2017/3/17 下午 3:40
Describe        :測試壓縮指令
*/
int inSqlite_Vacuum_Flow_Test()
{
	TRANSACTION_OBJECT	pobTran;

	memset(&pobTran, 0x00, sizeof(pobTran));
	pobTran.srBRec.inHDTIndex = -1;

	inFunc_GetHostNum(&pobTran);
	inSqlite_Vacuum_Flow(&pobTran, _TN_BATCH_TABLE_);

	return (VS_SUCCESS);
}

int inSqlite_ShowDbHandle()
{
	if (ginDebug == VS_TRUE)
	{		
		char szTempFile[34], szLinuxTempFile[34];
	
		memset(szTempFile, 0x00, sizeof(szTempFile));
		memset(szLinuxTempFile, 0x00, sizeof(szLinuxTempFile));
		sprintf(szTempFile, "  Ex Sqlit Handle[%d]",st_inSqlitHandleCnt);

		inDISP_LogPrintfWithFlag(szTempFile);

		inPRINT_ChineseFont("=====================", _PRT_HEIGHT_);
		inPRINT_ChineseFont(szTempFile, _PRT_HEIGHT_);
		inPRINT_ChineseFont("=====================", _PRT_HEIGHT_);
	}
	return VS_SUCCESS;
}

#ifdef _MODIFY_HDPT_FUNC_

/*
Function        :inSqlite_Update_ByTableID_All
Date&Time       :2019/4/24 上午 9:44
Describe        :利用Table ID來更新資料庫中的所有欄位，注意:update keytag不能重複（若已沒有Record則會update失敗，改用insert or update更符合以前SaveRec的使用情況）
*/
int inSqlite_Update_ByRecordID_All(char* szDBName, char* szTableName, int inRecordID, SQLITE_ALL_TABLE* srAll)
{
	int			inRowID = 0;
	int			inBindingIndex = 1;	/* binding的index從1開始 */
	int			i = 0;
	int			inRetVal = VS_SUCCESS;
	int			inSqlLength = 0;
	char			szDebugMsg[84 + 1];
	char			szSqlPrefix[100 + 1];	/* Update INTO	szTableName( */
	char			szSqlSuffix[40 + 1];	/* VALUES ( */
	char			szSqlSuffix2[40 + 1];	/* ); */
	char			szTemplate[720 + 1];	/* 因為最長有到711 */
	char			*szUpdateSql;
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	if (inRecordID < 0)
	{
		inDISP_DispLogAndWriteFlie("inRecordID不合法:%d", inRecordID);
		return (VS_ERROR);
	}
	
	/* 程式內的RecorID都是從0開始，RowID從1開始，所以這邊加一 */
	inRowID = inRecordID + 1;
	
	inRetVal = inSqlite_DB_Open_Or_Create(szDBName, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		inDISP_LogPrintfWithFlag("Open Database File OK");

	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "UPDATE %s", szTableName);
	
	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, " SET ");
	
	memset(szSqlSuffix2, 0x00, sizeof(szSqlSuffix2));
	sprintf(szSqlSuffix2, " WHERE inTableID = %d FROM %s", inRowID, szTableName);
	
	/* 算要配置多少記憶體 */
	inSqlLength += strlen(szSqlPrefix);
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfWithFlag( "COUNT");
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "IntTag: %d", srAll->inIntNum);
		inDISP_LogPrintfWithFlag(szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Int64tTag: %d", srAll->inInt64tNum);
		inDISP_LogPrintfWithFlag(szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "CharTag: %d", srAll->inCharNum);
		inDISP_LogPrintfWithFlag(szDebugMsg);
	}
	
	for (i = 0; i < srAll->inIntNum; i++)
	{
		/* Comma + space （i = 0時不放", "，但這裡只是分配空間，所以可以忽略 ） */
		inSqlLength += 2;
		inSqlLength += strlen(srAll->srInt[i].szTag);
		/* space + " = " + space*/
		inSqlLength += 3;
		/* " ? " */
		inSqlLength += 1;
	}
	
	for (i = 0; i < srAll->inInt64tNum; i++)
	{
		/* Comma + space */
		inSqlLength += 2;
		inSqlLength += strlen(srAll->srInt64t[i].szTag);
		/* space + " = " + space*/
		inSqlLength += 3;
		/* " ? " */
		inSqlLength += 1;
	}
	
	for (i = 0; i < srAll->inCharNum; i++)
	{
		/* Comma + space */
		inSqlLength += 2;
		inSqlLength += strlen(srAll->srChar[i].szTag);
		/* space + " = " + space */
		inSqlLength += 3;
		/* 根據字串長度 */
		inSqlLength += strlen(srAll->srChar[i].pCharVariable);
		/* " ? " */
		inSqlLength++;
	}
	
	/* "SET "的長度 */
	inSqlLength += strlen(szSqlSuffix);
	

	/* "WHERE lnOrgInvNum = %d" */
	inSqlLength += strlen(szSqlSuffix2);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", inSqlLength);
		inDISP_LogPrintfWithFlag(szDebugMsg);
	}
	
	/* 配置記憶體 */
	szUpdateSql = malloc(inSqlLength);
	memset(szUpdateSql, 0x00, inSqlLength);
	
	/* 先丟Table Name */
	strcat(szUpdateSql, szSqlPrefix);
	strcat(szUpdateSql, szSqlSuffix);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	
	for (i = 0; i < srAll->inIntNum; i++)
	{
		if (i == 0)
		{
			/* 第一個table的第一項不處理 */
		}
		else
		{
			strcat(szUpdateSql, ", ");
		}
		strcat(szUpdateSql, srAll->srInt[i].szTag);
		strcat(szUpdateSql, " = ");
		
		sprintf(szTemplate, "?");
		strcat(szUpdateSql, szTemplate);
		memset(szTemplate, 0x00, 1);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Int update OK");
		inDISP_LogPrintfWithFlag(szDebugMsg);
	}
	
	/* 代表上一個table有東西，要加comma */
	if (srAll->inIntNum > 0)
	{
		strcat(szUpdateSql, ", ");
	}
	
	for (i = 0; i < srAll->inInt64tNum; i++)
	{
		if (i == 0)
		{
			/* 第一個table的第一項不處理 */
		}
		else
		{
			strcat(szUpdateSql, ", ");
		}
		strcat(szUpdateSql, srAll->srInt64t[i].szTag);
		strcat(szUpdateSql, " = ");
		
		sprintf(szTemplate, "?");
		strcat(szUpdateSql, szTemplate);
		memset(szTemplate, 0x00, 1);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Int64t update OK");
		inDISP_LogPrintfWithFlag(szDebugMsg);
	}
	
	/* 代表上一個table有東西，要加comma */
	if (srAll->inInt64tNum > 0)
	{
		strcat(szUpdateSql, ", ");
	}
	
	for (i = 0; i < srAll->inCharNum; i++)
	{
		
		if (i == 0)
		{
			/* 第一個table的第一項不處理 */
		}
		else
		{
			strcat(szUpdateSql, ", ");
		}
		strcat(szUpdateSql, srAll->srChar[i].szTag);
		strcat(szUpdateSql, " = ");
		
		sprintf(szTemplate, "?");
		strcat(szUpdateSql, szTemplate);
		/* 只清自己用過得長度 問號 */
		memset(szTemplate, 0x00, 1);
		
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Char update OK");
		inDISP_LogPrintfWithFlag(szDebugMsg);
	}
	
	/* 為了避免有空的table導致多塞", "，若最後為", "清回0x00 */
	if (memcmp((szUpdateSql + strlen(szUpdateSql) - 2), ", ", 2) == 0)
	{
		memset(szUpdateSql + strlen(szUpdateSql) - 2, 0x00, 2);
	}

	/* 最後面的"WHERE lnOrgInvNum = %d" */
	strcat(szUpdateSql, szSqlSuffix2);
	
	if (ginDebug == VS_TRUE)
	{
		/* 因為值很多，所以用teraterm確認 */
		printf(szUpdateSql);
	}
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szUpdateSql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
		
	}
	
	/* Binding變數 */
	for (i = 0; i < srAll->inIntNum; i++)
	{
		inRetVal = sqlite3_bind_int(srSQLStat, inBindingIndex, *(int32_t*)srAll->srInt[i].pTagValue);
		if (inRetVal != SQLITE_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Binging Int Fail: %d", inRetVal);
				inDISP_LogPrintfWithFlag(szDebugMsg);
			}
		}
		else
		{
			inBindingIndex++;
		}
		
	}
	
	for (i = 0; i < srAll->inInt64tNum; i++)
	{
		inRetVal = sqlite3_bind_int64(srSQLStat, inBindingIndex, *(int64_t*)srAll->srInt64t[i].pTagValue);
		if (inRetVal != SQLITE_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Binging Int64t Fail: %d", inRetVal);
				inDISP_LogPrintfWithFlag(szDebugMsg);
			}
		}
		else
		{
			inBindingIndex++;
		}
	}
	
	for (i = 0; i < srAll->inCharNum; i++)
	{
		inRetVal = sqlite3_bind_blob(srSQLStat, inBindingIndex, srAll->srChar[i].pCharVariable, srAll->srChar[i].inTagValueLen, NULL);
		if (inRetVal != SQLITE_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Binging Char Fail: %d", inRetVal);
				inDISP_LogPrintfWithFlag(szDebugMsg);
			}
		}
		else
		{
			inBindingIndex++;
		}
	}
	
	/* Update */
	do
	{
		/* Update */
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW	||
		    inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Update OK");
				inDISP_LogPrintfWithFlag(szDebugMsg);
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Update ERROR Num:%d", inRetVal);
				inDISP_LogPrintfWithFlag(szDebugMsg);
				
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
				inDISP_LogPrintfWithFlag(szDebugMsg);
				
				inDISP_LogPrintfWithFlag( szUpdateSql);
			}

		}
		
	}while (inRetVal == SQLITE_ROW);
	
	/* 釋放事務，若要重用則用sqlite3_reset */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	free(szUpdateSql);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintfWithFlag( "inSqlite_Update_ByRecordID_All()_END");
                inDISP_LogPrintfWithFlag( "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Table_ByRecordID_All
Date&Time       :2019/4/24 下午 2:22
Describe        :利用RecordID、取得最新參數
*/
int inSqlite_Get_Table_ByRecordID_All(char* szDBName, char* szTableName, int inRecordID, SQLITE_ALL_TABLE* srAll)
{
	int			inRowID = 0;
	int			inRetVal = VS_SUCCESS;
	char			szDebugMsg[128 + 1] = {0};
	char			szQuerySql[200 + 1];	/* INSERT INTO	szTableName( */
	char			szErrorMessage[100 + 1];
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	
	if (ginDebug == VS_TRUE)
        {
        	inDISP_LogPrintfWithFlag( "----------------------------------------");
                inDISP_LogPrintfWithFlag( "inSqlite_Get_Table_ByRecordID_All()_START");
        }
	
	if (inRecordID < 0)
	{
		inDISP_DispLogAndWriteFlie( "inRecordID不合法:%d", inRecordID);
		return (VS_ERROR);
	}
	
	inRowID = inRecordID + 1;
	
	inRetVal = inSqlite_DB_Open_Or_Create(szDBName, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
	}
	
	/* 塞入SQL語句 */
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "SELECT * FROM %s WHERE inTableID = %d ORDER BY inTableID DESC LIMIT 1", szTableName, inRowID);

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
	}
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_NO_RECORD);
	}
	else
	{
		memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
		strcpy(szErrorMessage, sqlite3_errmsg(srDBConnection));
			
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
			inDISP_LogPrintfWithFlag(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", szErrorMessage);
			inDISP_LogPrintfWithFlag(szDebugMsg);

			inDISP_LogPrintfWithFlag( szQuerySql);
		}

		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintfWithFlag(szDebugMsg);
			}
		}

		/* 因為直接get 不存在的table回傳值是-1，只有在Error Msg才能得知錯誤原因 */
		if (memcmp(szErrorMessage, "no such table", strlen("no such table")) == 0)
		{
			return (VS_NO_RECORD);
		}
		else
		{
			return (VS_ERROR);
		}
	}
	
	/* binding 取得值 */
	inSqlite_Get_Binding_Value(&srSQLStat, srAll);
		
	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintfWithFlag( "inSqlite_Get_Table_ByRecordID_All()_END");
                inDISP_LogPrintfWithFlag( "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Insert_Or_Replace_ByRecordID_All
Date&Time       :2019/4/26 下午 5:10
Describe        :Open Database檔 insert or replace(若有此紀錄，則更新(若該欄位未指定值，則會更新為NULL)，若無此紀錄，則插入)
*/
int inSqlite_Insert_Or_Replace_ByRecordID_All(char* szDBName, char* szTableName, int inRecordID , SQLITE_ALL_TABLE* srAll)
{
	int			inRowID = 0;
	int			inBindingIndex = 1;	/* binding的index從1開始 */
	int			i;
	int			inRetVal;
	int			inSqlLength = 0;	/* 算組SQL語句的長度 */
	char			szDebugMsg[84 + 1];
	char			szSqlPrefix[100 + 1];	/* INSERT INTO	szTableName( */
	char			szSqlSuffix[20 + 1];	/* VALUES ( */
	char			szSqlSuffix2[10 + 1];	/* ); */
	char			szTemplate[100 + 1];	/* 因為最長有到711 */
	char			*szInsertSql;
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	
	if (ginDebug == VS_TRUE)
        {
        	inDISP_LogPrintfWithFlag( "----------------------------------------");
                inDISP_LogPrintfWithFlag( "inSqlite_Insert_Or_Replace_ByRecordID_All()_START");
        }
	
	if (inRecordID < 0)
	{
		inDISP_DispLogAndWriteFlie( "inRecordID不合法:%d", inRecordID);
		return (VS_ERROR);
	}
	
	/* 程式內的RecorID都是從0開始，RowID從1開始，所以這邊加一 */
	inRowID = inRecordID + 1;
	/* 強制改成rowID的pointer */
	srAll->srInt[0].pTagValue = &inRowID;
	inRetVal = inSqlite_DB_Open_Or_Create(szDBName, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "INSERT OR REPLACE INTO %s (", szTableName);
	
	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, ")VALUES (");
	
	memset(szSqlSuffix2, 0x00, sizeof(szSqlSuffix2));
	sprintf(szSqlSuffix2, ");");
	
	/* 算要配置多少記憶體 */
	inSqlLength += strlen(szSqlPrefix);
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfWithFlag( "COUNT");
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "IntTag: %d", srAll->inIntNum);
		inDISP_LogPrintfWithFlag(szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Int64tTag: %d", srAll->inInt64tNum);
		inDISP_LogPrintfWithFlag(szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "CharTag: %d", srAll->inCharNum);
		inDISP_LogPrintfWithFlag(szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "TextTag: %d", srAll->inTextNum);
		inDISP_LogPrintfWithFlag(szDebugMsg);
	}
	
	for (i = 0; i < srAll->inIntNum; i++)
	{
		inSqlLength += strlen(srAll->srInt[i].szTag);
		/* Comma */
		inSqlLength += 2
	}
	
	for (i = 0; i < srAll->inInt64tNum; i++)
	{
		inSqlLength += strlen(srAll->srInt64t[i].szTag);
		/* Comma */
		inSqlLength += 2;
	}
	
	for (i = 0; i < srAll->inCharNum; i++)
	{
		inSqlLength += strlen(srAll->srChar[i].szTag);
		/* Comma */
		inSqlLength += 2;
	}
	
	for (i = 0; i < srAll->inTextNum; i++)
	{
		inSqlLength += strlen(srAll->srText[i].szTag);
		/* Comma */
		inSqlLength += 2;
	}
	
	/* 第一行最後面的) */
	inSqlLength ++;
	/* 第二行"VALUES ("的長度 */
	inSqlLength += strlen(szSqlSuffix);
	
	for (i = 0; i < srAll->inIntNum; i++)
	{
		/* 用問號 */
		inSqlLength += 1;
		/* Comma & Space */
		inSqlLength += 2;
	}
	
	for (i = 0; i < srAll->inInt64tNum; i++)
	{
		/* 用問號 */
		inSqlLength += 1;
		/* Comma & Space */
		inSqlLength += 2;
	}
	
	for (i = 0; i < srAll->inCharNum; i++)
	{
		/* 用問號 */
		inSqlLength += 1;
		/* Comma & Space & 兩個單引號 */
		inSqlLength += 2;
	}
	
	for (i = 0; i < srAll->inTextNum; i++)
	{
		/* 用問號 */
		inSqlLength += 1;
		/* Comma & Space & 兩個單引號 */
		inSqlLength += 2;
	}
	
	/* ); */
	inSqlLength += strlen(szSqlSuffix2);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", inSqlLength);
		inDISP_LogPrintfWithFlag(szDebugMsg);
	}
	
	/* 配置記憶體 */
	szInsertSql = malloc(inSqlLength + 100);
	memset(szInsertSql, 0x00, inSqlLength + 100);
	
	/* 先丟Table Name */
	strcat(szInsertSql, szSqlPrefix);
	
	/* 看上一個table是不是空的，有東西的話，第一項前面要加comma，但這已經是第一個table，所以放0 */
	if (0 > 0)
	{
		strcat(szInsertSql, ", ");
	}
	
	for (i = 0; i < srAll->inIntNum; i++)
	{
		if (i > 0)
		{
			strcat(szInsertSql, ", ");
		}
		strcat(szInsertSql, srAll->srInt[i].szTag);
	}
	
	/* 看上一個table是不是空的，有東西的話，第一項前面要加comma */
	if (srAll->inIntNum > 0)
	{
		strcat(szInsertSql, ", ");
	}
	
	for (i = 0; i < srAll->inInt64tNum; i++)
	{
		if (i > 0)
		{
			strcat(szInsertSql, ", ");
		}
		strcat(szInsertSql, srAll->srInt64t[i].szTag);
	}
	
	/* 看上一個table是不是空的，有東西的話，第一項前面要加comma */
	if (srAll->inInt64tNum > 0)
	{
		strcat(szInsertSql, ", ");
	}
	
	for (i = 0; i < srAll->inCharNum; i++)
	{
		
		if (i > 0)
		{
			strcat(szInsertSql, ", ");
		}
		strcat(szInsertSql, srAll->srChar[i].szTag);
	}
	
	/* 看上一個table是不是空的，有東西的話，第一項前面要加comma */
	if (srAll->inCharNum > 0)
	{
		strcat(szInsertSql, ", ");
	}
	
	for (i = 0; i < srAll->inTextNum; i++)
	{
		
		if (i > 0)
		{
			strcat(szInsertSql, ", ");
		}
		strcat(szInsertSql, srAll->srText[i].szTag);
	}
	
	/* 為了避免有空的table導致多塞, */
	if (memcmp((szInsertSql + strlen(szInsertSql) - 2), ", ", 2) == 0)
	{
		memset(szInsertSql + strlen(szInsertSql) - 2, 0x00, 2);
	}
	
	/* ")VALUES (" */
	strcat(szInsertSql, szSqlSuffix);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	
	for (i = 0; i < srAll->inIntNum; i++)
	{
		if (i == 0)
		{
			sprintf(szTemplate, "?");
		}
		else
		{
			sprintf(szTemplate, ", ?");
		}
		strcat(szInsertSql, szTemplate);
		/* 只清自己用過得長度 comma(1) + space(1) + ?(1) */
		memset(szTemplate, 0x00, 3);
	}
	
	/* 代表上一個table有東西，要加comma */
	if (srAll->inIntNum > 0)
	{
		strcat(szInsertSql, ", ");
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Int Insert OK");
		inDISP_LogPrintfWithFlag(szDebugMsg);
	}
	
	for (i = 0; i < srAll->inInt64tNum; i++)
	{
		if (i == 0)
		{
			sprintf(szTemplate, "?");
		}
		else
		{
			sprintf(szTemplate, ", ?");
		}
		strcat(szInsertSql, szTemplate);
		/* 只清自己用過得長度 comma(1) + space(1) + ?(1) */
		memset(szTemplate, 0x00, 3);
	}
	
	/* 代表上一個table有東西，要加comma */
	if (srAll->inInt64tNum > 0)
	{
		strcat(szInsertSql, ", ");
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Int64t Insert OK");
		inDISP_LogPrintfWithFlag(szDebugMsg);
	}
	
	for (i = 0; i < srAll->inCharNum; i++)
	{
		if (i == 0)
		{
			sprintf(szTemplate, "?");
		}
		else
		{
			sprintf(szTemplate, ", ?");
		}
		strcat(szInsertSql, szTemplate);
		/* 只清自己用過得長度 comma(1) + space(1) + ?(1) */
		memset(szTemplate, 0x00, 3);
	}
	
	/* 代表上一個table有東西，要加comma */
	if (srAll->inCharNum > 0)
	{
		strcat(szInsertSql, ", ");
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Char Insert OK");
		inDISP_LogPrintfWithFlag(szDebugMsg);
	}
	
	for (i = 0; i < srAll->inTextNum; i++)
	{
		if (i == 0)
		{
			sprintf(szTemplate, "?");
		}
		else
		{
			sprintf(szTemplate, ", ?");
		}
		strcat(szInsertSql, szTemplate);
		/* 只清自己用過得長度 comma(1) + space(1) + ?(1) */
		memset(szTemplate, 0x00, 3);
	}
	
	/* 代表上一個table有東西，要加comma */
	if (srAll->inTextNum > 0)
	{
		strcat(szInsertSql, ", ");
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Text Insert OK");
		inDISP_LogPrintfWithFlag(szDebugMsg);
	}
	
	/* 為了避免有空的table導致多塞, */
	if (memcmp((szInsertSql + strlen(szInsertSql) - 2), ", ", 2) == 0)
	{
		memset(szInsertSql + strlen(szInsertSql) - 2, 0x00, 2);
	}
	
	/* 最後面的); */
	strcat(szInsertSql, szSqlSuffix2);
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szInsertSql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
		
	}
	
	/* Binding變數 */
	for (i = 0; i < srAll->inIntNum; i++)
	{
		inRetVal = sqlite3_bind_int(srSQLStat, inBindingIndex, *(int32_t*)srAll->srInt[i].pTagValue);
		if (inRetVal != SQLITE_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Binging Int Fail: %d", inRetVal);
				inDISP_LogPrintfWithFlag(szDebugMsg);
			}
		}
		else
		{
			inBindingIndex++;
		}
		
	}
	
	for (i = 0; i < srAll->inInt64tNum; i++)
	{
		inRetVal = sqlite3_bind_int64(srSQLStat, inBindingIndex, *(int64_t*)srAll->srInt64t[i].pTagValue);
		if (inRetVal != SQLITE_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Binging Int64t Fail: %d", inRetVal);
				inDISP_LogPrintfWithFlag(szDebugMsg);
			}
		}
		else
		{
			inBindingIndex++;
		}
	}
	
	for (i = 0; i < srAll->inCharNum; i++)
	{
		inRetVal = sqlite3_bind_blob(srSQLStat, inBindingIndex, srAll->srChar[i].pCharVariable, srAll->srChar[i].inTagValueLen, NULL);
		if (inRetVal != SQLITE_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Binging Char Fail: %d", inRetVal);
				inDISP_LogPrintfWithFlag(szDebugMsg);
			}
		}
		else
		{
			inBindingIndex++;
		}
	}
	
	for (i = 0; i < srAll->inTextNum; i++)
	{
		inRetVal = sqlite3_bind_text(srSQLStat, inBindingIndex, srAll->srText[i].pCharVariable, srAll->srText[i].inTagValueLen, NULL);
		if (inRetVal != SQLITE_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Binging Text Fail: %d", inRetVal);
				inDISP_LogPrintfWithFlag(szDebugMsg);
			}
		}
		else
		{
			inBindingIndex++;
		}
	}
	
	do
	{
		/* Insert */
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW	||
		    inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Insert OK");
				inDISP_LogPrintfWithFlag(szDebugMsg);
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Insert ERROR Num:%d", inRetVal);
				inDISP_LogPrintfWithFlag(szDebugMsg);
				
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
				inDISP_LogPrintfWithFlag(szDebugMsg);
				
				inDISP_LogPrintfWithFlag( szInsertSql);
			}

		}
		
	}while (inRetVal == SQLITE_ROW);
	
	/* 釋放事務，若要重用則用sqlite3_reset */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	free(szInsertSql);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
	}
	
	if (inRetVal == SQLITE_ERROR)
	{
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintfWithFlag( "inSqlite_Insert_Or_Replace_ByRecordID_All()_END");
                inDISP_LogPrintfWithFlag( "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Data_By_External_SQL
Date&Time       :2019/5/28 下午 2:22
Describe        :嘗試不將SQL寫死達到重複使用的效果
*/
int inSqlite_Get_Data_By_External_SQL(char* szDBName, char* szTableName, SQLITE_ALL_TABLE* srAll, char* szSQLStatement)
{
	int			j = 0;
	int			inIntIndex = 0, inInt64tIndex = 0, inCharIndex = 0;
	int			inCols = 0, inDataLen = 0;
	int			inRetVal = VS_SUCCESS;
	int			inFind = VS_FALSE;
	char			szDebugMsg[128 + 1];
	char			szQuerySql[200 + 1];	/* INSERT INTO	szTableName( */
	char			szTagName[_TAG_WIDTH_ + 1];
	char			szErrorMessage[100 + 1];
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	
	if (ginDebug == VS_TRUE)
        {
        	inDISP_LogPrintfWithFlag( "----------------------------------------");
                inDISP_LogPrintfWithFlag( "inSqlite_Get_Table_ByRecordID_All()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(szDBName, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
	}
	
	/* 塞入SQL語句 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	strcpy(szQuerySql, szSQLStatement);

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
	}
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_NO_RECORD);
	}
	else
	{
		memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
		strcpy(szErrorMessage, sqlite3_errmsg(srDBConnection));
			
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
			inDISP_LogPrintfWithFlag(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", szErrorMessage);
			inDISP_LogPrintfWithFlag(szDebugMsg);

			inDISP_LogPrintfWithFlag( szQuerySql);
		}

		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintfWithFlag(szDebugMsg);
			}
		}

		/* 因為直接get 不存在的table回傳值是-1，只有在Error Msg才能得知錯誤原因 */
		if (memcmp(szErrorMessage, "no such table", strlen("no such table")) == 0)
		{
			return (VS_NO_RECORD);
		}
		else
		{
			return (VS_ERROR);
		}
	}
	
	inCols = sqlite3_column_count(srSQLStat);
	
	/* binding 取得值 */
	for (j = 0; j < inCols; j++)
	{
		inFind = VS_FALSE;
		memset(szTagName, 0x00, sizeof(szTagName));
		strcat(szTagName, sqlite3_column_name(srSQLStat, j));
		
		
		for (inIntIndex = 0; inIntIndex < srAll->inIntNum; inIntIndex++)
		{
			if (srAll->srInt[inIntIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}
			
			
			/* 比對Tag Name */
			if (memcmp(szTagName, srAll->srInt[inIntIndex].szTag, strlen(srAll->srInt[inIntIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll->srInt[inIntIndex].szTag))
			{
				*(int32_t*)srAll->srInt[inIntIndex].pTagValue = sqlite3_column_int(srSQLStat, j);
				srAll->srInt[inIntIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}
		
		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inInt64tIndex = 0; inInt64tIndex < srAll->inInt64tNum; inInt64tIndex++)
		{
			if (srAll->srInt64t[inInt64tIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll->srInt64t[inInt64tIndex].szTag, strlen(srAll->srInt64t[inInt64tIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll->srInt64t[inInt64tIndex].szTag))
			{
				*(int64_t*)srAll->srInt64t[inInt64tIndex].pTagValue = sqlite3_column_int64(srSQLStat, j);
				srAll->srInt64t[inInt64tIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}
			
		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}
		
		for (inCharIndex = 0; inCharIndex < srAll->inCharNum; inCharIndex++)
		{
			if (srAll->srChar[inCharIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}
			
			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll->srChar[inCharIndex].szTag, strlen(srAll->srChar[inCharIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll->srChar[inCharIndex].szTag))
			{
				inDataLen = sqlite3_column_bytes(srSQLStat, j);
				memcpy(srAll->srChar[inCharIndex].pCharVariable, sqlite3_column_blob(srSQLStat, j), inDataLen);
				srAll->srChar[inCharIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}
		
		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Find no variable to insert:Tag: %s", szTagName);
			inDISP_LogPrintfWithFlag(szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Value: %s", (char*)sqlite3_column_blob(srSQLStat, j));
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
		
	}
	/* 釋放事務 */
	sqlite3_finalize(srSQLStat);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintfWithFlag( "inSqlite_Get_Table_ByRecordID_All()_END");
                inDISP_LogPrintfWithFlag( "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Binding_Value
Date&Time       :2019/6/3 下午 5:19
Describe        :取得查詢後的結果
*/
int inSqlite_Get_Binding_Value(sqlite3_stmt** srSQLStat, SQLITE_ALL_TABLE* srAll)
{
	int	j = 0;
	int	inCols = 0;
	int	inFind = VS_FALSE;
	int	inIntIndex = 0, inInt64tIndex = 0, inCharIndex = 0, inTextIndex = 0;
	int	inDataLen = 0;
	char	szTagName[_TAG_WIDTH_ + 1] = {0};
	char	szDebugMsg[128 + 1] = {0};
	
	inCols = sqlite3_column_count(*srSQLStat);
	
	/* binding 取得值 */
	for (j = 0; j < inCols; j++)
	{
		inFind = VS_FALSE;
		memset(szTagName, 0x00, sizeof(szTagName));
		strcat(szTagName, sqlite3_column_name(*srSQLStat, j));
		
		
		for (inIntIndex = 0; inIntIndex < srAll->inIntNum; inIntIndex++)
		{
			if (srAll->srInt[inIntIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}
			
			
			/* 比對Tag Name */
			if (memcmp(szTagName, srAll->srInt[inIntIndex].szTag, strlen(srAll->srInt[inIntIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll->srInt[inIntIndex].szTag))
			{
				*(int32_t*)srAll->srInt[inIntIndex].pTagValue = sqlite3_column_int(*srSQLStat, j);
				srAll->srInt[inIntIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}
		
		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inInt64tIndex = 0; inInt64tIndex < srAll->inInt64tNum; inInt64tIndex++)
		{
			if (srAll->srInt64t[inInt64tIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll->srInt64t[inInt64tIndex].szTag, strlen(srAll->srInt64t[inInt64tIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll->srInt64t[inInt64tIndex].szTag))
			{
				*(int64_t*)srAll->srInt64t[inInt64tIndex].pTagValue = sqlite3_column_int64(*srSQLStat, j);
				srAll->srInt64t[inInt64tIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}
			
		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}
		
		for (inCharIndex = 0; inCharIndex < srAll->inCharNum; inCharIndex++)
		{
			if (srAll->srChar[inCharIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}
			
			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll->srChar[inCharIndex].szTag, strlen(srAll->srChar[inCharIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll->srChar[inCharIndex].szTag))
			{
				inDataLen = sqlite3_column_bytes(*srSQLStat, j);
				/* 放之前先清空 */
				memset(srAll->srText[inTextIndex].pCharVariable, 0x00, inDataLen + 1);
				memcpy(srAll->srChar[inCharIndex].pCharVariable, sqlite3_column_blob(*srSQLStat, j), inDataLen);
				srAll->srChar[inCharIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}
		
		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inTextIndex = 0; inTextIndex < srAll->inTextNum; inTextIndex++)
		{
			if (srAll->srText[inTextIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}
			
			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll->srText[inTextIndex].szTag, strlen(srAll->srText[inTextIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll->srText[inTextIndex].szTag))
			{
				inDataLen = sqlite3_column_bytes(*srSQLStat, j);
				/* 放之前先清空 */
				memset(srAll->srText[inTextIndex].pCharVariable, 0x00, inDataLen + 1);
				memcpy(srAll->srText[inTextIndex].pCharVariable, sqlite3_column_blob(*srSQLStat, j), inDataLen);
				srAll->srText[inTextIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}
		
		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Find no variable to insert:Tag: %s", szTagName);
			inDISP_LogPrintfWithFlag(szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Value: %s", (char*)sqlite3_column_blob(*srSQLStat, j));
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
		
	}
	
	
	return (VS_SUCCESS);
}

/*
Function        :
Date&Time       :
Describe        :
*/
int inSqlite_Calculate_SQLLength(char* szSqlPrefix, char* szSqlSuffix, char* szSqlSuffix2, SQLITE_ALL_TABLE* srAll, int* inSqlLength)
{
	int	i = 0;
	char	szDebugMsg[84 + 1] = {0};
	
	/* 算要配置多少記憶體 */
	*inSqlLength += strlen(szSqlPrefix);
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfWithFlag( "COUNT");
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "IntTag: %d", srAll->inIntNum);
		inDISP_LogPrintfWithFlag(szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Int64tTag: %d", srAll->inInt64tNum);
		inDISP_LogPrintfWithFlag(szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "CharTag: %d", srAll->inCharNum);
		inDISP_LogPrintfWithFlag(szDebugMsg);
	}
	
	for (i = 0; i < srAll->inIntNum; i++)
	{
		*inSqlLength += strlen(srAll->srInt[i].szTag);
		/* Comma */
		*inSqlLength += 2;;
	}
	
	for (i = 0; i < srAll->inInt64tNum; i++)
	{
		*inSqlLength += strlen(srAll->srInt64t[i].szTag);
		/* Comma */
		*inSqlLength += 2;;
	}
	
	for (i = 0; i < srAll->inCharNum; i++)
	{
		*inSqlLength += strlen(srAll->srChar[i].szTag);
		/* Comma */
		*inSqlLength += 2;;
	}
	
	/* 第一行最後面的) */
	(*inSqlLength) ++;
	/* 第二行"VALUES ("的長度 */
	*inSqlLength += strlen(szSqlSuffix);
	
	for (i = 0; i < srAll->inIntNum; i++)
	{
		/* 用問號 */
		*inSqlLength += 1;
		/* Comma & Space */
		*inSqlLength += 2;
	}
	
	for (i = 0; i < srAll->inInt64tNum; i++)
	{
		/* 用問號 */
		*inSqlLength += 1;
		/* Comma & Space */
		*inSqlLength += 2;
	}
	
	for (i = 0; i < srAll->inCharNum; i++)
	{
		/* 用問號 */
		*inSqlLength += 1;
		/* Comma & Space & 兩個單引號 */
		*inSqlLength += 2;
	}
	
	/* ); */
	*inSqlLength += strlen(szSqlSuffix2);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", *inSqlLength);
		inDISP_LogPrintfWithFlag(szDebugMsg);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_DB_Open_Or_Create
Date&Time       :2019/6/3 下午 6:26
Describe        :開啟DB，」若無則創建
*/
int inSqlite_DB_Open_Or_Create(char* szDBPath, sqlite3** srDBConnection, int inFlags, char* szVfs)
{
	int	inRetVal = SQLITE_OK;
	char	szDebugMsg[84 + 1] = {0};
	
	inRetVal = sqlite3_open_v2(szDBPath, srDBConnection, inFlags, szVfs);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File Failed");
			inDISP_LogPrintfWithFlag(szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ErrorNum:%d", inRetVal);
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
		inFile_Open_File_Cnt_Increase();
		
		return (VS_SUCCESS);
	}
	
}

/*
Function        :inSqlite_Copy_Table_Data
Date&Time       :2019/6/4 上午 10:41
Describe        :複製Table
 *		uszPreserveData:是否拋棄table內的資料
*/
int inSqlite_Copy_Table_Data(char* szDBPath, char* szOldTableName, char* szNewTableName)
{
	int		inRetVal = VS_ERROR;
	int		inSqlLength = 0;
	char		szSqlPrefix[200 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		*szCreateSql;			/* 因為會很長且隨table浮動，所以用pointer */
	char		*szErrorMessage = NULL;
	sqlite3		*srDBConnection;		/* 建立到資料庫的connection */
		
	
	if (ginDebug == VS_TRUE)
        {
        	inDISP_LogPrintfWithFlag( "----------------------------------------");
                inDISP_LogPrintfWithFlag( "inSqlite_Copy_Table_Data()_START");
        }
	
	/* 開啟DataBase檔 */
	inRetVal = inSqlite_DB_Open_Or_Create(szDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	if (inRetVal != VS_SUCCESS)
	{	
		return (VS_ERROR);
	}

	/* 塞入SQL語句 */
	/* 為了使table name可變動，所以拉出來組 */
	/* 加入了if not exists字串，若已建立不會重複建立 */
	/* 前綴 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "INSERT INTO %s SELECT * FROM %s;", szNewTableName, szOldTableName);

	/*  "CREATE TABLE	szTableName("的長度 */
	inSqlLength += strlen(szSqlPrefix);
        
	/* inSqlLength: */
	if (ginDebug == VS_TRUE) 
        {
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", inSqlLength);
		inDISP_LogPrintfWithFlag(szDebugMsg);
	}
	/* 配置記憶體(一定要+1，超級重要，不然會overflow) */
	szCreateSql = malloc(inSqlLength + 1);
	memset(szCreateSql, 0x00, inSqlLength);
	
	/* 先丟前綴Table Name */
	strcat(szCreateSql, szSqlPrefix);
	
	/* 建立 Table */
	inRetVal = sqlite3_exec(srDBConnection, szCreateSql, 0, 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Copy Table ERROR Num:%d", inRetVal);
			inDISP_LogPrintfWithFlag(szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason:%s", szErrorMessage);
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
		
		free(szCreateSql);
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintfWithFlag(szDebugMsg);
			}
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Copy Table OK");
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
	}
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	
	/* 釋放記憶體 */
	free(szCreateSql);
	
	if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintfWithFlag( "inSqlite_Copy_Table_Data()_END");
                inDISP_LogPrintfWithFlag( "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Rename_Table
Date&Time       :2019/6/4 下午 1:13
Describe        :重新命名
*/
int inSqlite_Rename_Table(char* szDBPath, char* szOldTableName, char* szNewTableName)
{
	int		inRetVal = VS_ERROR;
	int		inSqlLength = 0;
	char		szSqlPrefix[100 + 2] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		*szCreateSql;			/* 因為會很長且隨table浮動，所以用pointer */
	char		*szErrorMessage = NULL;
	sqlite3		*srDBConnection;		/* 建立到資料庫的connection */
		
	
	if (ginDebug == VS_TRUE)
        {
        	inDISP_LogPrintfWithFlag( "----------------------------------------");
                inDISP_LogPrintfWithFlag( "inSqlite_Rename_Table()_START");
        }
	
	/* 開啟DataBase檔 */
	inRetVal = inSqlite_DB_Open_Or_Create(szDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	if (inRetVal != VS_SUCCESS)
	{	
		return (VS_ERROR);
	}

	/* 塞入SQL語句 */
	/* 為了使table name可變動，所以拉出來組 */
	/* 加入了if not exists字串，若已建立不會重複建立 */
	/* 前綴 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "ALTER TABLE %s RENAME TO %s;", szOldTableName, szNewTableName);
	/*  "CREATE TABLE	szTableName("的長度 */
	inSqlLength += strlen(szSqlPrefix);
        
	/* inSqlLength: */
	if (ginDebug == VS_TRUE) 
        {
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", inSqlLength);
		inDISP_LogPrintfWithFlag(szDebugMsg);
	}
	/* 配置記憶體(一定要+1，超級重要，不然會overflow) */
	szCreateSql = malloc(inSqlLength + 1);
	memset(szCreateSql, 0x00, inSqlLength);
	
	/* 先丟前綴Table Name */
	strcat(szCreateSql, szSqlPrefix);
	
	/* 建立 Table */
	inRetVal = sqlite3_exec(srDBConnection, szCreateSql, 0, 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Rename Table ERROR Num:%d", inRetVal);
			inDISP_LogPrintfWithFlag(szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason:%s", szErrorMessage);
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
		
		free(szCreateSql);
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintfWithFlag(szDebugMsg);
			}
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Rename Table OK");
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
	}
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	
	/* 釋放記憶體 */
	free(szCreateSql);
	
	if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintfWithFlag( "inSqlite_Rename_Table()_END");
                inDISP_LogPrintfWithFlag( "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Table_Relist_SQLite
Date&Time       :2019/6/4 下午 1:25
Describe        :
*/
int inSqlite_Table_Relist_SQLite(char *szDBPath, char *szTableName, int inOldIndex, int inNewIndex)
{
	int			i = 0;
	int			inBindingIndex = 1;
	int			inRetVal;
	int			inTempTableID = 0;		/* 暫時最大+1 */
	char			szDebugMsg[100 + 1] = {0};
	char			szMaxTableID[10 + 1] = {0};
	char			szQuerySql[200 + 1] = {0};
	sqlite3			*srDBConnection;		/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfWithFlag( "----------------------------------------");
		inDISP_LogPrintfWithFlag( "inSqlite_Table_Relist_SQLite() START !");
	}
	
	/* index不合法 */
	if (inOldIndex == -1 || inNewIndex == -1)
	{
		return (VS_ERROR);
	}
	
	memset(szMaxTableID, 0x00, sizeof(szMaxTableID));
	inRetVal = inSqlite_Get_Max_TableID(szDBPath, szTableName, szMaxTableID);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	else
	{
		inTempTableID = atoi(szMaxTableID) + 1;
	}
	
	/* 開啟DataBase檔 */
	inRetVal = inSqlite_DB_Open_Or_Create(szDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	if (inRetVal != VS_SUCCESS)
	{	
		return (VS_ERROR);
	}
	
	/* 第一步驟，先將要調整的RECORD塞到最後面 */
	
	/* 塞入SQL語句 */
	/* 為了使table name可變動，所以拉出來組 */
	/* 前綴 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "UPDATE %s SET inTableID = %d WHERE (inTableID = %d);", szTableName, inTempTableID, inOldIndex);
	/*  "CREATE TABLE	szTableName("的長度 */
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
	}
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	do
	{
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table OK");
				inDISP_LogPrintfWithFlag(szDebugMsg);
			}
		}
		else if (inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table OK");
				inDISP_LogPrintfWithFlag(szDebugMsg);
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table ERROR Num:%d", inRetVal);
				inDISP_LogPrintfWithFlag(szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
				inDISP_LogPrintfWithFlag(szDebugMsg);

				inDISP_LogPrintfWithFlag( szQuerySql);
			}

			/* 釋放事務 */
			inSqlite_SQL_Finalize(&srSQLStat);
			/* 關閉 database, close null pointer 是NOP(No Operation) */
			if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
			{
				/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
				return (VS_ERROR);
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "Close Database OK");
					inDISP_LogPrintfWithFlag(szDebugMsg);
				}
			}

			return (VS_ERROR);
		}
	
	}while (inRetVal != SQLITE_DONE);
	
	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	/* 第二步驟，將中間的TableID加一，以空出空間 */
	/* 塞入SQL語句 */
	/* 為了使table name可變動，所以拉出來組 */
	/* 前綴 */
	/* 往前插入的狀況 */
	if (inNewIndex < inOldIndex)
	{
		memset(szQuerySql, 0x00, sizeof(szQuerySql));
		sprintf(szQuerySql, "UPDATE %s SET inTableID = inTableID + 1 WHERE inTableID = ? ;", szTableName);
		/*  "CREATE TABLE	szTableName("的長度 */

		/* prepare語句 */
		inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
		if (inRetVal != SQLITE_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
				inDISP_LogPrintfWithFlag(szDebugMsg);
			}
		}
		
		for (i = (inOldIndex - 1); i >= inNewIndex; i--)
		{
			inBindingIndex = 1;
			
			inRetVal = sqlite3_bind_int(srSQLStat, inBindingIndex, i);
			if (inRetVal != SQLITE_OK)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Binging Int Fail: %d", inRetVal);
					inDISP_LogPrintfWithFlag(szDebugMsg);
				}
			}
			else
			{
				inBindingIndex++;
			}
			
			/* 取得 database 裡所有的資料 */
			/* Qerry */
			do
			{
				inRetVal = sqlite3_step(srSQLStat);
				if (inRetVal == SQLITE_ROW)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "UPDATE Table OK");
						inDISP_LogPrintfWithFlag(szDebugMsg);
					}
				}
				else if (inRetVal == SQLITE_DONE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "UPDATE Table OK");
						inDISP_LogPrintfWithFlag(szDebugMsg);
					}
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "UPDATE Table ERROR Num:%d", inRetVal);
						inDISP_LogPrintfWithFlag(szDebugMsg);

						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
						inDISP_LogPrintfWithFlag(szDebugMsg);

						inDISP_LogPrintfWithFlag( szQuerySql);
					}

					/* 釋放事務 */
					inSqlite_SQL_Finalize(&srSQLStat);
	
					/* 關閉 database, close null pointer 是NOP(No Operation) */
					if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
					{
						/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
						return (VS_ERROR);
					}
					else
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "Close Database OK");
							inDISP_LogPrintfWithFlag(szDebugMsg);
						}
					}

					return (VS_ERROR);
				}

			}while (inRetVal != SQLITE_DONE);
			
			sqlite3_reset(srSQLStat);
		}

		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
	}
	else
	{
		memset(szQuerySql, 0x00, sizeof(szQuerySql));
		sprintf(szQuerySql, "UPDATE %s SET inTableID = inTableID + 1 WHERE inTableID = ? ;", szTableName);
		/*  "CREATE TABLE	szTableName("的長度 */

		/* prepare語句 */
		inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
		if (inRetVal != SQLITE_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
				inDISP_LogPrintfWithFlag(szDebugMsg);
			}
		}
		
		for (i = (inOldIndex + 1); i <= inNewIndex; i++)
		{
			inBindingIndex = 1;
			
			inRetVal = sqlite3_bind_int(srSQLStat, inBindingIndex, i);
			if (inRetVal != SQLITE_OK)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Binging Int Fail: %d", inRetVal);
					inDISP_LogPrintfWithFlag(szDebugMsg);
				}
			}
			else
			{
				inBindingIndex++;
			}
			
			/* 取得 database 裡所有的資料 */
			/* Qerry */
			do
			{
				inRetVal = sqlite3_step(srSQLStat);
				if (inRetVal == SQLITE_ROW)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "UPDATE Table OK");
						inDISP_LogPrintfWithFlag(szDebugMsg);
					}
				}
				else if (inRetVal == SQLITE_DONE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "UPDATE Table OK");
						inDISP_LogPrintfWithFlag(szDebugMsg);
					}
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "UPDATE Table ERROR Num:%d", inRetVal);
						inDISP_LogPrintfWithFlag(szDebugMsg);

						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
						inDISP_LogPrintfWithFlag(szDebugMsg);

						inDISP_LogPrintfWithFlag( szQuerySql);
					}

					/* 釋放事務 */
					inSqlite_SQL_Finalize(&srSQLStat);
					
					/* 關閉 database, close null pointer 是NOP(No Operation) */
					if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
					{
						/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
						return (VS_ERROR);
					}
					else
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "Close Database OK");
							inDISP_LogPrintfWithFlag(szDebugMsg);
						}
					}

					return (VS_ERROR);
				}

			}while (inRetVal != SQLITE_DONE);
			
			sqlite3_reset(srSQLStat);
		}

		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
	}
	
	/* 第三步驟 將Record放回去 */
	/* 塞入SQL語句 */
	/* 為了使table name可變動，所以拉出來組 */
	/* 前綴 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "UPDATE %s SET inTableID = %d WHERE (inTableID = %d);", szTableName, inNewIndex, inTempTableID);
	/*  "CREATE TABLE	szTableName("的長度 */
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
	}
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	do
	{
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table OK");
				inDISP_LogPrintfWithFlag(szDebugMsg);
			}
		}
		else if (inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table OK");
				inDISP_LogPrintfWithFlag(szDebugMsg);
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table ERROR Num:%d", inRetVal);
				inDISP_LogPrintfWithFlag(szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
				inDISP_LogPrintfWithFlag(szDebugMsg);

				inDISP_LogPrintfWithFlag( szQuerySql);
			}

			/* 釋放事務 */
			inSqlite_SQL_Finalize(&srSQLStat);
			
			/* 關閉 database, close null pointer 是NOP(No Operation) */
			if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
			{
				/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
				return (VS_ERROR);
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "Close Database OK");
					inDISP_LogPrintfWithFlag(szDebugMsg);
				}
			}

			return (VS_ERROR);
		}
	
	}while (inRetVal != SQLITE_DONE);
	
	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfWithFlag( "inSqlite_Table_Relist_SQLite() END !");
		inDISP_LogPrintfWithFlag( "----------------------------------------");
	}

        return (VS_SUCCESS);
}

/*
Function        :inSqlite_Delete_Table_Data_Flow
Date&Time       :2019/6/5 上午 11:59
Describe        :在這邊決定名稱並分流
*/
int inSqlite_Delete_Table_Data_Flow(TRANSACTION_OBJECT *pobTran, int inTableType)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	if (pobTran->uszFileNameNoNeedNumBit == VS_TRUE)
	{
		inFunc_ComposeFileName(pobTran, szTableName, "", 0);
	}
	else
	{
		inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	}
	
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
//		case _TN_BATCH_TABLE_ESC_TEMP_:
//			strcat(szTableName, _BATCH_TABLE_ESC_TEMP_SUFFIX_);
//			break;
//		case _TN_BATCH_TABLE_ESC_ADVICE_:
//			strcat(szTableName, _BATCH_TABLE_ESC_ADVICE_SUFFIX_);
//			break;
//		case _TN_BATCH_TABLE_ESC_ADVICE_EMV_:
//			strcat(szTableName, _BATCH_TABLE_ESC_ADVICE_EMV_SUFFIX_);
//			break;
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ADVICE_SUFFIX_);
			break;
		default :
			break;
	}
	
	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
//		case _TN_BATCH_TABLE_ESC_TEMP_:
		case _TN_EMV_TABLE_:
		case _TN_BATCH_TABLE_TICKET_:
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
//		case _TN_BATCH_TABLE_ESC_ADVICE_:
//		case _TN_BATCH_TABLE_ESC_ADVICE_EMV_:
			inRetVal = inSqlite_Delete_Table_Data(gszTranDBPath, szTableName);
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inSqlite_Delete_Table_Data
Date&Time       :2019/6/5 上午 11:59
Describe        :僅刪除Data，不刪除Table
*/
int inSqlite_Delete_Table_Data(char* szDBPath, char* szTableName)
{
	int		inRetVal;
	char		szDebugMsg[84 + 1];
	char		szDeleteSql[80 + 1];	/* INSERT INTO	szTableName( */
	char		*szErrorMessage = NULL;
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */
	
	if (ginDebug == VS_TRUE)
        {
        	inDISP_LogPrintfWithFlag( "----------------------------------------");
                inDISP_LogPrintfWithFlag( "inSqlite_Delete_Table_Data()_START");
        }
	
	/* 開啟DataBase檔 */
	inRetVal = inSqlite_DB_Open_Or_Create(szDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	if (inRetVal != VS_SUCCESS)
	{	
		return (VS_ERROR);
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szDeleteSql, 0x00, sizeof(szDeleteSql));
	sprintf(szDeleteSql, "DELETE FROM %s", szTableName);
	
	/* Delete */
	inRetVal = sqlite3_exec(srDBConnection, szDeleteSql, 0, 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Delete Table Data ERROR Num:%d", inRetVal);
			inDISP_LogPrintfWithFlag(szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason:%s", szErrorMessage);
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inDISP_LogPrintfWithFlag(szDebugMsg);
			}
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "DELETE TABLE DATA OK");
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
	}
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintfWithFlag( "inSqlite_Delete_Table_Data()_END");
                inDISP_LogPrintfWithFlag( "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Update_ByInvNum_SignState
Date&Time       :2019/6/5 下午 3:44
Describe        :UPDATE table_name SET column1 = value1, column2 = value2...., columnN = valueN WHERE [condition];
 *
*/
int inSqlite_Update_ByInvNum_SignState(TRANSACTION_OBJECT * pobTran, char* szDBPath, char* szTableName, int inInvoiceNumber)
{
	int			i = 0;
	int			inRetVal = VS_ERROR;
	int			inSqlLength = 0;
	char			szDebugMsg[84 + 1] = {0};
	char			szSqlPrefix[100 + 1] = {0};	/* Update INTO	szTableName( */
	char			szSqlSuffix[40 + 1] = {0};	/* VALUES ( */
	char			szSqlSuffix2[200 + 1] = {0};	/* ); */
	char			szTemplate[720 + 1] = {0};	/* 因為最長有到711 */
	char			szTag[_TAG_WIDTH_ + 1] = {0};
	char			szTagValue[2 + 1] = {0};
	char			*szUpdateSql = NULL;
	sqlite3			*srDBConnection = NULL;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat = NULL;
	
	if (ginDebug == VS_TRUE)
        {
        	inDISP_LogPrintfWithFlag( "----------------------------------------");
                inDISP_LogPrintfWithFlag( "inSqlite_Update_ByInvNum_TranState()_START");
        }
	
	/* 開啟DataBase檔 */
	inRetVal = inSqlite_DB_Open_Or_Create(szDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	if (inRetVal != VS_SUCCESS)
	{	
		return (VS_ERROR);
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "UPDATE %s", szTableName);
	
	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, " SET ");
	
	memset(szSqlSuffix2, 0x00, sizeof(szSqlSuffix2));
	sprintf(szSqlSuffix2, " WHERE inTableID IN (SELECT inTableID FROM %s WHERE lnOrgInvNum = %d ORDER BY inTableID DESC LIMIT 1)", szTableName, inInvoiceNumber);
	
	memset(szTag, 0x00, sizeof(szTag));
	sprintf(szTag, "inSignStatus");
	
	memset(szTagValue, 0x00, sizeof(szTagValue));
	sprintf(szTagValue, "%d", pobTran->srBRec.inSignStatus);
	
	/* 算要配置多少記憶體 */
	inSqlLength += strlen(szSqlPrefix);
	
	for (i = 0; i < 1; i++)
	{
		/* Comma + space */
		inSqlLength += 2;
		inSqlLength += strlen(szTag);
		/* space + " = " + space + " ' " */
		inSqlLength += 4;
		/* 根據字串長度 */
		inSqlLength += strlen(szTagValue);
		/* " ' " */
		inSqlLength++;
	}
	
	/* "SET "的長度 */
	inSqlLength += strlen(szSqlSuffix);
	
	/* "WHERE lnOrgInvNum = %d" */
	inSqlLength += strlen(szSqlSuffix2);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", inSqlLength);
		inDISP_LogPrintfWithFlag(szDebugMsg);
	}
	
	/* 配置記憶體 */
	szUpdateSql = malloc(inSqlLength);
	memset(szUpdateSql, 0x00, inSqlLength);
	
	/* 先丟Table Name */
	strcat(szUpdateSql, szSqlPrefix);
	strcat(szUpdateSql, szSqlSuffix);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	
	for (i = 0; i < 1; i++)
	{
		if (i == 0)
		{
			/* 第一個table的第一項不處理 */
		}
		else
		{
			strcat(szUpdateSql, ", ");
		}
		strcat(szUpdateSql, szTag);
		strcat(szUpdateSql, " = ");
		
		sprintf(szTemplate, "'%s'", szTagValue);
		strcat(szUpdateSql, szTemplate);
		/* 只清自己用過得長度 兩個單引號*/
		memset(szTemplate, 0x00, strlen(szTagValue) + 2);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Char update OK");
		inDISP_LogPrintfWithFlag(szDebugMsg);
	}
	
	/* 為了避免有空的table導致多塞", "，若最後為", "清回0x00 */
	if (memcmp((szUpdateSql + strlen(szUpdateSql) - 2), ", ", 2) == 0)
	{
		memset(szUpdateSql + strlen(szUpdateSql) - 2, 0x00, 2);
	}

	/* 最後面的"WHERE lnOrgInvNum = %d" */
	strcat(szUpdateSql, szSqlSuffix2);
	
	if (ginDebug == VS_TRUE)
	{
		/* 因為值很多，所以用teraterm確認 */
		printf(szUpdateSql);
	}
	
	/* Update */
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szUpdateSql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
	}
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	do
	{
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table OK");
				inDISP_LogPrintfWithFlag(szDebugMsg);
			}
		}
		else if (inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table OK");
				inDISP_LogPrintfWithFlag(szDebugMsg);
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table ERROR Num:%d", inRetVal);
				inDISP_LogPrintfWithFlag(szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
				inDISP_LogPrintfWithFlag(szDebugMsg);

				inDISP_LogPrintfWithFlag( szUpdateSql);
			}

			/* 釋放事務 */
			inSqlite_SQL_Finalize(&srSQLStat);
			
			/* 關閉 database, close null pointer 是NOP(No Operation) */
			if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
			{
				/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
				return (VS_ERROR);
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "Close Database OK");
					inDISP_LogPrintfWithFlag(szDebugMsg);
				}
			}

			return (VS_ERROR);
		}
	
	}while (inRetVal != SQLITE_DONE);
	
	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
	}
	
	free(szUpdateSql);
	
	if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintfWithFlag( "inSqlite_Update_ByInvNum_TranState()_END");
                inDISP_LogPrintfWithFlag( "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_DB_Close
Date&Time       :2019/9/6 上午 10:00
Describe        :關閉DB
*/
int inSqlite_DB_Close(sqlite3** srDBConnection)
{
	int	inRetVal = SQLITE_OK;
	char	szDebugMsg[84 + 1] = {0};
	
	inRetVal = sqlite3_close(*srDBConnection);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Close Database File Failed");
			inDISP_LogPrintfWithFlag(szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ErrorNum:%d", inRetVal);
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Close Database File OK");
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
		inFile_Open_File_Cnt_Decrease();
		
		return (VS_SUCCESS);
	}
	
}

/*
Function        :inSqlite_SQL_Finalize
Date&Time       :2019/10/24 下午 5:55
Describe        :
*/
int inSqlite_SQL_Finalize(sqlite3_stmt **srSQLStat)
{
	int	inRetVal = SQLITE_OK;
	char	szDebugMsg[84 + 1] = {0};
	
	inRetVal = sqlite3_finalize(*srSQLStat);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "sqlite3_finalize Failed");
			inDISP_LogPrintfWithFlag(szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ErrorNum:%d", inRetVal);
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "sqlite3_finalize OK");
			inDISP_LogPrintfWithFlag(szDebugMsg);
		}
		
		return (VS_SUCCESS);
	}
	
}

#endif
