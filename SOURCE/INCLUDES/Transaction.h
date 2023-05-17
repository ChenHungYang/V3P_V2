/*
 * File:   Transaction.h
 * Author: user
 *
 * Created on 2015年7月27日, 下午 4:31
 */
#ifndef _TRANSACTION_H_
#define _TRANSACTION_H_

#define _AUTH_CODE_SIZE_ 6		   /* Authorization code size */
#define _PAN_SIZE_ 19			   /* Primary account number */
#define _PAN_UCARD_SIZE_ 21		   /* 加兩個槓槓 */
#define _EXP_DATE_SIZE_ 4		   /* Expiry date MMYY */
#define _SVC_CODE_SIZE_ 4		   /* Size of service code */
#define _CARD_HOLDER_NAME_SIZE_ 40 /* CardHolderName */
#define _MAX_TRACK1_NAME_SIZE_ 48  /* CardHolderName最大長度 */
#define _DATE_SIZE_ 6			   /* Date in YYYYMMDD format */
#define _TIME_SIZE_ 6			   /* Time in HHMMSS format */
#define _DEBIT_ISSUER_ID_SIZE_ 8
#define _DEBIT_CARD_COMMENT_SIZE_ 30
#define _DEBIT_ACCOUNT_SIZE_ 16
#define _FISC_ISSUER_ID_SIZE_ 8		/* FISC 發卡單位代號長度*/
#define _FISC_CARD_COMMENT_SIZE_ 30 /* FISC 備註欄長度*/
#define _FISC_ACCOUNT_SIZE_ 16
#define _FISC_TCC_SIZE_ 8
#define _FISC_MCC_SIZE_ 15
#define _FISC_DATE_AND_TIME_SIZE_ 14
#define _FISC_REFUND_DATE_ 8
#define _ISSUER_NUM_SIZE_ 2
#define _PIN_SIZE_ 8 /* Size of PIN data*/
#define _MAC_SIZE_ 8
#define _ENC_PIN_LENGTH_ 8
#define _VSS_SMALL_BUFF_SIZE_ 64
#define _VSS_MED_BUFF_SIZE_ 128
#define _UID_SIZE_ 10

typedef struct TagBATCH_REC
{
	int inCode;										/* Trans Code */
	int inOrgCode;									/* Original Trans Code  */
	int inPrintOption;								/* Print Option Flag */
	int inHDTIndex;									/* 紀錄HDTindex */
	int inCDTIndex;									/* 紀錄CDTindex */
	int inCPTIndex;									/* 紀錄CPTindex */
	int inTxnResult;								/* 紀錄交易結果 */
	int inChipStatus;								/* 0 NOT_USING_CHIP, 1 EMV_CARD, 2 EMV_EASY_ENTRY_CARD */
	int inFiscIssuerIDLength;						/* 金融卡發卡單位代號長度 */
	int inFiscCardCommentLength;					/* 金融卡備註欄長度 */
	int inFiscAccountLength;						/* 金融卡帳號長度 */
	int inFiscSTANLength;							/* 金融卡交易序號長度 */
	int inESCTransactionCode;						/* ESC組ISO使用 重新上傳使用 Transaction Code沒辦法存在Batch */
	int inESCUploadMode;							/* 標示支不支援ESC */
	int inESCUploadStatus;							/* 標示ESC上傳狀態 */
	int inSignStatus;								/* 簽名檔狀態(有 免簽 或 Bypass) ESC電文使用 */
	int inHGCreditHostIndex;						/* 聯合_HAPPY_GO_信用卡主機 */
	int inHGCode;									/* 聯合_HAPPY_GO_交易碼 */
	long lnTxnAmount;								/* The transaction amount, such as a SALE */
	long lnOrgTxnAmount;							/* The ORG transaction amount, such as a SALE */
	long lnTipTxnAmount;							/* The transaction amount, such as a TIP */
	long lnAdjustTxnAmount;							/* The transaction amount, such as a ADJUST */
	long lnTotalTxnAmount;							/* The transaction amount, such as a TOTAL */
	long lnInvNum;									/* 調閱編號  */
	long lnOrgInvNum;								/* Original 調閱編號  */
	long lnBatchNum;								/* Batch Number */
	long lnOrgBatchNum;								/* Original Batch Number */
	long lnSTANNum;									/* Stan Number */
	long lnSTAN_Multi;								/*  */
	long lnOrgSTANNum;								/* Original Stan Number，離線或混合交易使用 */
	long lnInstallmentPeriod;						/* 分期付款_期數 */
	long lnInstallmentDownPayment;					/* 分期付款_頭期款 */
	long lnInstallmentPayment;						/* 分期付款_每期款 */
	long lnInstallmentFormalityFee;					/* 分期付款_手續費 */
	long lnRedemptionPoints;						/* 紅利扣抵_扣抵紅利點數 */
	long lnRedemptionPointsBalance;					/* 紅利扣抵_剩餘紅利點數 */
	long lnRedemptionPaidCreditAmount;				/* 紅利扣抵_實際支付金額（消費者付的金額） */
	long lnRedemptionPaidAmount;					/* By Ray */
	long lnHGTransactionType;						/* 聯合_HAPPY GO_交易類別 */
	long lnHGPaymentType;							/* 聯合_HAPPY_GO_支付工具 */
	long lnHGPaymentTeam;							/* 聯合_HAPPY_GO_支付工具_主機回_*/
	long lnHGBalancePoint;							/* 聯合_HAPPY_GO_剩餘點數 */
	long lnHGTransactionPoint;						/* 聯合_HAPPY_GO_交易點數  合計 */
	long lnHGAmount;								/* 聯合_HAPPY_GO_扣抵後金額  (商品金額 = lnHGAmount + lnHGRedeemAmt) */
	long lnHGRedeemAmount;							/* 聯合_HAPPY_GO_扣抵金額 */
	long lnHGRefundLackPoint;						/* 聯合_HAPPY_GO_不足點數 */
	long lnHGBatchIndex;							/* 聯合_HAPPY_GO_主機當下批次號碼 */
	long lnHG_SPDH_OrgInvNum;						/* HAPPY_GO取消用INV */
	long lnHGSTAN;									/* HAPPY_GO STAN */
	long lnFubonGoodsID;							// By Ray
	long lnSvcRedeemAmount;							/* [新增SVC功能]  [SAM] */
	char szAuthCode[_AUTH_CODE_SIZE_ + 1];			/* Auth Code */
	char szMPASAuthCode[_AUTH_CODE_SIZE_ + 1];		/* MPAS Auth Code */
	char szRespCode[4 + 1];							/* Response Code */
	char szStoreID[50 + 1];							/* StoreID */
	char szCardLabel[20 + 1];						/* 卡別  */
	char szPAN[20 + 1];								/* 卡號  */
	char szDate[8 + 1];								/* YYYYMMDD */
	char szOrgDate[8 + 1];							/* YYYYMMDD */
	char szEscDate[8 + 1];							/* YYYYMMDD */
	char szTime[6 + 1];								/* HHMMSS */
	char szOrgTime[6 + 1];							/* HHMMSS */
	char szEscTime[6 + 1];							/* HHMMSS */
	char szCardTime[15 + 1];						/* 晶片卡讀卡時間 , YYYYMMDDHHMMSS */
	char szRefNo[12 + 1];							/* 序號  */
	char szExpDate[_EXP_DATE_SIZE_ + 1];			/* Expiration date YYMM */
	char szServiceCode[_SVC_CODE_SIZE_ + 1];		/* Service code from track */
	char szCardHolder[_CARD_HOLDER_NAME_SIZE_ + 1]; /* 持卡人名字 */
	char szAMEX4DBC[4 + 1];
	char szFiscIssuerID[8 + 1];		   /* 發卡單位代號 */
	char szFiscCardComment[30 + 1];	   /* 金融卡備註欄 */
	char szFiscAccount[16 + 1];		   /* 金融卡帳號 */
	char szFiscOutAccount[16 + 1];	   /* 金融卡轉出帳號 */
	char szFiscSTAN[8 + 1];			   /* 金融卡交易序號 */
	char szFiscTacLength[2 + 1];	   /* 金融卡Tac長度 */
	char szFiscTac[32 + 1];			   /* 金融卡Tac */
	char szFiscTCC[8 + 1];			   /* 端末設備查核碼 */
	char szFiscMCC[16 + 1];			   /* 金融卡MCC */
	char szFiscRRN[12 + 1];			   /* 金融卡調單編號 */
	char szFiscRefundDate[8 + 1];	   /* 金融卡退貨原始交易日期(YYYYMMDD) */
	char szFiscDateTime[14 + 1];	   /* 計算TAC(S2)的交易日期時間 */
	char szFiscPayDevice[2 + 1];	   /* 金融卡付款裝置 1 = 手機 2 = 卡片 */
	char szFiscMobileDevice[2 + 1];	   /* SE 類型，0x05：雲端卡片(Cloud-Based) */
	char szFiscMobileNFType[2 + 1];	   /* 近端交易類型，行動金融卡是否需輸入密碼 00不需要 01視情況 02一定要 */
	char szFiscMobileNFSetting[2 + 1]; /* 近端交易類型設定 0x00：Single Issuer Wallet 0x01：國內Third-Party Wallet 0x02~9：保留 0x0A：其他 */
	char szInstallmentIndicator[2 + 1];
	char szRedeemIndicator[2 + 1];
	char szRedeemSignOfBalance[1 + 1];
	char szHGCardLabel[20 + 1];		 /* HAPPY_GO 卡別 */
	char szHGPAN[20 + 1];			 /* HAPPY_GO Account number */
	char szHGAuthCode[6 + 1];		 /* HAPPY_GO 授權碼 */
	char szHGRefNo[12 + 1];			 /* HAPPY_GO Reference Number */
	char szHGRespCode[6 + 1];		 /* HG Response Code */
	char szCUP_TN[6 + 1];			 /* CUP Trace Number (TN) */
	char szCUP_TD[4 + 1];			 /* CUP Transaction Date (MMDD) */
	char szCUP_TT[6 + 1];			 /* CUP Transaction Time (hhmmss) */
	char szCUP_RRN[12 + 1];			 /* CUP Retrieve Reference Number (CRRN) */
	char szCUP_STD[4 + 1];			 /* CUP Settlement Date(MMDD) Of Host Response */
	char szCUP_EMVAID[32 + 1];		 /* CUP晶片交易存AID帳單列印使用 */
	char szTranAbbrev[2 + 1];		 /* Tran abbrev for reports */
	char szIssueNumber[2 + 1];		 /* */
	char szStore_DREAM_MALL[36 + 1]; /* 存Dream_Mall Account Number And Member ID*/
	char szDCC_FCNFR[3 + 1];		 /* Foreign Currency No. For Rate */
	char szDCC_AC[4 + 1];			 /* Action Code */
	char szDCC_FCN[3 + 1];			 /* Foreign Currency Number */
	char szDCC_FCA[12 + 1];			 /* Foreign Currency Amount */
	char szDCC_FCMU[2 + 1];			 /* Foreign Currency Minor Unit */
	char szDCC_FCAC[3 + 1];			 /* Foreign currcncy Alphabetic Code */
	char szDCC_ERMU[2 + 1];			 /* Exchange Rate Minor Unit */
	char szDCC_ERV[9 + 1];			 /* Exchange Rate Value */
	char szDCC_IRMU[2 + 1];			 /* Inverted Rate Minor Unit */
	char szDCC_IRV[9 + 1];			 /* Inverted Rate Value */
	char szDCC_IRDU[2 + 1];			 /* Inverted Rate Display Unit */
	char szDCC_MPV[8 + 1];			 /* Markup Percentage Value */
	char szDCC_MPDP[2 + 1];			 /* Markup Percentage Decimal Point */
	char szDCC_CVCN[3 + 1];			 /* Commissino Value Currency Number */
	char szDCC_CVCA[12 + 1];		 /* Commission Value Currency Amount */
	char szDCC_CVCMU[2 + 1];		 /* Commission Value Currency Minor Unit */
	char szDCC_TIPFCA[12 + 1];		 /* Tip Foreign Currency Amount */
	char szDCC_OTD[4 + 1];			 /* Original Transaction Date & Time (MMDD) */
	char szDCC_OTA[12 + 1];			 /* Original Transaction Amount */
	char szProductCode[42 + 1];		 /* 產品代碼 */
	char szAwardNum[2 + 1];			 /* 優惠個數 */
	char szAwardSN[22 + 1];			 /* 優惠序號(Award S/N) TID(8Bytes)+YYYYMMDDhhmmss(16 Bytes)，共22Bytes */
	char szTxnNo[24 + 1];			 /* 交易編號 */
	char szMCP_BANKID[4 + 1];		 /* 行動支付標記 金融機構代碼 */
	char szPayItemCode[5 + 1];		 /* 繳費項目代碼 */
	char szTableTD_Data[15 + 1];	 /* Table TD的資料， */
	char szDFSTraceNum[6 + 1];		 /* DFS交易系統追蹤號 */
	char szTSPTransactionID[16 + 1]; /* ScanPay使用 */

	char szEI_FLAG[1 + 1];
	char szEI_BankId[6 + 1];
	char szSvcUID[7 + 1];		   /* [新增SVC功能]  [SAM] */
	char szSvcIssueId[5 + 1];	   /* [新增SVC功能]  [SAM] */
	char szSvcCardNumber[12 + 1];  /* [新增SVC功能]  [SAM] */
	char szSvcExpireDay[4 + 1];	   /* [新增SVC功能]  [SAM] */
	char szSvcCardTid[4 + 1];	   /* [新增SVC功能]  [SAM] */
	char szSvcTxnDateTime[14 + 1]; /* [新增SVC功能]  [SAM] */
	char szSvcPointSign[3 + 1];	   /* [新增SVC功能]  [SAM] */
	char szSvcPoint1[5 + 1];	   /* [新增SVC功能]  [SAM] */
	char szSvcPoint2[5 + 1];	   /* [新增SVC功能]  [SAM] */
	char szSvcPoint3[5 + 1];	   /* [新增SVC功能]  [SAM] */
	char szSvcPoint4[5 + 1];	   /* [新增SVC功能]  [SAM] */

	unsigned char uszWAVESchemeID;	   /* WAVE 使用用於組電文 Field_22 */
	unsigned char uszVOIDBit;		   /* 負向交易 */
	unsigned char uszUpload1Bit;	   /* Offline交易使用 (原交易advice是否未上傳)*/
	unsigned char uszUpload2Bit;	   /* Offline交易使用 (原交易advice調帳是否未上傳)*/
	unsigned char uszUpload3Bit;	   /* Offline交易使用 (原交易advice調帳的取消是否未上傳)*/
	unsigned char uszReferralBit;	   /* ISO Response Code 【01】【02】使用 */
	unsigned char uszOfflineBit;	   /* 離線交易 */
	unsigned char uszManualBit;		   /* Manual Keyin */
	unsigned char uszNoSignatureBit;   /* 免簽名使用 (免簽名則為true)*/
	unsigned char uszCUPTransBit;	   /* 是否為CUP */
	unsigned char uszFiscTransBit;	   /* SmartPay交易，是否為金融卡 */
	unsigned char uszContactlessBit;   /* 是否為非接觸式 */
	unsigned char uszEMVFallBackBit;   /* 是否要啟動fallback */
	unsigned char uszInstallmentBit;   /* Installment */
	unsigned char uszRedeemBit;		   /* Redemption */
	unsigned char uszForceOnlineBit;   /* 組電文使用 Field_25 Point of Service Condition Code */
	unsigned char uszMail_OrderBit;	   /* 組電文使用 Field_25 Point of Service Condition Code */
	unsigned char uszDCCTransBit;	   /* 是否為DCC交易 */
	unsigned char uszNCCCDCCRateBit;   /* 詢價後轉台幣 */
	unsigned char uszCVV2Bit;		   /* 有輸入CVV2 */
	unsigned char uszRewardSuspendBit; /* 暫停優惠服務(優惠活動為0且要印補充資訊) */
	unsigned char uszRewardL1Bit;	   /* 要印L1 */
	unsigned char uszRewardL2Bit;	   /* 要印L2 */
	unsigned char uszRewardL5Bit;	   /* 要印L5 */
	unsigned char uszField24NPSBit;
	unsigned char uszVEPS_SignatureBit;	 /* VEPS 免簽名是否成立 */
	unsigned char uszTCUploadBit;		 /* TCUpload是否已上傳 */
	unsigned char uszFiscConfirmBit;	 /* SmartPay 0220 是否已上傳 */
	unsigned char uszFiscVoidConfirmBit; /* SmartPay Void 0220 是否已上傳 */
	unsigned char uszESCMerchantCopyBit; /* 是否加印商店聯 */
	unsigned char uszPinEnterBit;		 /* 此筆交易是否有鍵入密碼(只能確認原交易，若該筆之後的調整或取消不會將此Bit Off) */
	unsigned char uszL2PrintADBit;		 /* L2是否印AD，因L2原交易取消要判斷，只好增加 */
	unsigned char uszInstallment;		 /* HappyGo分期交易 */
	unsigned char uszRedemption;		 /* HappyGo點數兌換 */
	unsigned char uszHappyGoSingle;		 /* HappyGo交易 */
	unsigned char uszHappyGoMulti;		 /* HappyGo混合交易 */
	unsigned char uszCLSBatchBit;		 /* 是否已接續上傳 此Bit設定時值為'1' */
	unsigned char uszTxNoCheckBit;		 /* 商店自存聯卡號遮掩開關 */
	unsigned char uszSpecial00Bit;		 /* 特殊卡別參數檔，活動代碼00表示免簽(只紀錄，主要看uszNoSignatureBit) */
	unsigned char uszSpecial01Bit;		 /* 特殊卡別參數檔，活動代碼01表示ECR回傳明碼(先決條件ECR卡號遮掩有開才做判斷) */
	unsigned char uszRefundCTLSBit;		 /* 用在簽單印(W) 因為送電文contactless bit已OFF轉Manual Key in */
	unsigned char uszMPASTransBit;		 /* 標示為小額交易 */
	unsigned char uszMPASReprintBit;	 /* 標示該小額交易是否可重印 */
	unsigned char uszMobilePayBit;		 /* 判斷是不是行動支付交易 由主機回傳 */
	unsigned char uszF56NoSignatureBit;	 /* F56免簽名使用 (免簽名則為true)*/
	unsigned char uszReceiveCupF59;		 /* 判斷是否有收到欄位59的資料 */
	unsigned char fScanPayForSS : 1;	 /* ScanPayForSS用 */
} BATCH_REC;

typedef struct TagEMV_REC
{
	int inEMVCardDecision;
	int in50_APLabelLen;
	int in5A_ApplPanLen;
	int in5F20_CardholderNameLen;
	int in5F24_ExpireDateLen;
	int in5F2A_TransCurrCodeLen;
	int in5F34_ApplPanSeqnumLen;
	int in71_IssuerScript1Len;
	int in72_IssuerScript2Len;
	int in82_AIPLen;
	int in84_DFNameLen;
	int in8A_AuthRespCodeLen;
	int in91_IssuerAuthDataLen;
	int in95_TVRLen;
	int in9A_TranDateLen;
	int in9B_TSILen;
	int in9C_TranTypeLen;
	int in9F02_AmtAuthNumLen;
	int in9F03_AmtOtherNumLen;
	int in9F08_AppVerNumICCLen;
	int in9F09_TermVerNumLen;
	int in9F10_IssuerAppDataLen;
	int in9F18_IssuerScriptIDLen;
	int in9F1A_TermCountryCodeLen;
	int in9F1E_IFDNumLen;
	int in9F26_ApplCryptogramLen;
	int in9F27_CIDLen;
	int in9F33_TermCapabilitiesLen;
	int in9F34_CVMLen;
	int in9F35_TermTypeLen;
	int in9F36_ATCLen;
	int in9F37_UnpredictNumLen;
	int in9F41_TransSeqCounterLen;
	int in9F5A_Application_Program_IdentifierLen;
	int in9F5B_ISRLen;
	int in9F63_CardProductLabelInformationLen;
	int in9F66_QualifiersLen;
	int in9F6C_Card_Transaction_QualifiersLen;
	int in9F6E_From_Factor_IndicatorLen;
	int in9F74_TLVLen;
	int inDF69_NewJspeedyModeLen;
	int inDF8F4F_TransactionResultLen;
	int inDFEC_FallBackIndicatorLen;
	int inDFED_ChipConditionCodeLen;
	int inDFEE_TerEntryCapLen;
	int inDFEF_ReasonOnlineCodeLen;
	unsigned char usz50_APLabel[16 + 1];
	unsigned char usz5A_ApplPan[10 + 1];
	unsigned char usz5F20_CardholderName[26 + 1];
	unsigned char usz5F24_ExpireDate[4 + 1];
	unsigned char usz5F2A_TransCurrCode[2 + 1];
	unsigned char usz5F34_ApplPanSeqnum[2 + 1];
	unsigned char usz71_IssuerScript1[262 + 1];
	unsigned char usz72_IssuerScript2[262 + 1];
	unsigned char usz82_AIP[2 + 1];
	unsigned char usz84_DF_NAME[16 + 1];
	unsigned char usz8A_AuthRespCode[2 + 1];
	unsigned char usz91_IssuerAuthData[16 + 1];
	unsigned char usz95_TVR[6 + 1];
	unsigned char usz9A_TranDate[4 + 1];
	unsigned char usz9B_TSI[2 + 1];
	unsigned char usz9C_TranType[2 + 1];
	unsigned char usz9F02_AmtAuthNum[6 + 1];
	unsigned char usz9F03_AmtOtherNum[6 + 1];
	unsigned char usz9F08_AppVerNumICC[2 + 1];
	unsigned char usz9F09_TermVerNum[2 + 1];
	unsigned char usz9F10_IssuerAppData[32 + 1];
	unsigned char usz9F18_IssuerScriptID[4 + 1];
	unsigned char usz9F1A_TermCountryCode[2 + 1];
	unsigned char usz9F1E_IFDNum[8 + 1];
	unsigned char usz9F26_ApplCryptogram[8 + 1];
	unsigned char usz9F27_CID[2 + 1];
	unsigned char usz9F33_TermCapabilities[4 + 1];
	unsigned char usz9F34_CVM[4 + 1];
	unsigned char usz9F35_TermType[2 + 1];
	unsigned char usz9F36_ATC[2 + 1];
	unsigned char usz9F37_UnpredictNum[4 + 1];
	unsigned char usz9F41_TransSeqCounter[4 + 1];
	unsigned char usz9F5A_Application_Program_Identifier[32 + 1];
	unsigned char usz9F5B_ISR[6 + 1];
	unsigned char usz9F63_CardProductLabelInformation[16 + 1];
	unsigned char usz9F66_Qualifiers[4 + 1];
	unsigned char usz9F6C_Card_Transaction_Qualifiers[2 + 1];
	unsigned char usz9F6E_From_Factor_Indicator[32 + 1];
	unsigned char usz9F74_TLV[6 + 1];
	unsigned char uszDF69_NewJspeedyMode[2 + 1];
	unsigned char uszDF8F4F_TransactionResult[2 + 1];
	unsigned char uszDFEC_FallBackIndicator[2 + 1];
	unsigned char uszDFED_ChipConditionCode[2 + 1];
	unsigned char uszDFEE_TerEntryCap[2 + 1];
	unsigned char uszDFEF_ReasonOnlineCode[4 + 1];

} EMV_REC;

typedef struct TagIPASS_REC
{
	int inStepNum;				 /* 紀錄交易階段 */
	long lnCardInvNum;			 /* Card Transaction Invoice # */
	long lnUnixTime;			 /* GMT Unix Time */
	long lnSign_Len;			 /* 簽章長度 */
	char szDAVTITxn[280];		 /* Advice用，API回傳取得 */
	char szSign_Data[300];		 /* 收送用簽章資料 */
	char szCardInfo[100];		 /* 詢卡結果 */
	unsigned char uszCB_CardBit; /* 是否為聯名卡 */
	unsigned char uszQueryBit;	 /* 是否為詢卡 */
} IPASS_REC;

typedef struct TagECC_REC
{
	long lnCardInvNum;				  /* Card Transaction Invoice # */
	char szRespCode[2 + 1];			  /* Response Code */
	char szDate[8 + 1];				  /* YYYYMMDD */
	char szTime[6 + 1];				  /* HHMMSS */
	char szICER_RespCode[4 + 1];	  /*T3900  ICER respond code， 00 表示交易成功*/
	char szAPI_RespCode[4 + 1];		  /*T3901 ICERAPI return code， 0 表示交易成功 */
	char szEZ_RespCode[4 + 1];		  /*T3902 EZHost respond code， 00 表示交易成功*/
	char szSVCS_RespCode[6 + 1];	  /*T3903 SVCS respond code， 0 表示交易成功*/
	char szReader_RespCode[4 + 1];	  /*T3904 Reader Response Code 讀卡指令的回應 */
	char szPurseVersionNumber[2 + 1]; /* Response Code T4800 */
	char szCardID[20 + 1];			  /* T0200 */
	char szPurseID[16 + 1];			  /* T0211 */
	char szRRN[15 + 1];
	unsigned char uszCB_CardBit; /* 是否為聯名卡 */
} ECC_REC;

typedef struct TagICASH_REC
{
	long lnCardInvNum;	  /* Card Transaction Invoice # */
	char szCardInfo[100]; /* 詢卡結果 */
} ICASH_REC;

typedef struct TagHAPPYCASH_REC
{
	long lnCardInvNum;	  /* Card Transaction Invoice # */
	char szCardInfo[100]; /* 詢卡結果 */
} HAPPYCASH_REC;

typedef struct TagTicket_BATCH_REC
{
	int inCode;				 /* 交易類別 */
	int inPrintOption;		 /* Print Option Flag (also in TCT) */
	int inTicketType;		 /* 票證種類 - 交易或明細使用 */
	int inTDTIndex;			 /* 存TDT的index */
	long lnTxnAmount;		 /* 交易金額 */
	long lnTopUpAmount;		 /* 加值金額(基底) */
	long lnTotalTopUpAmount; /* 加值金額 */
	long lnCardRemainAmount; /* 卡片餘額 */
	long lnInvNum;			 /* For簽單使用，簽單序號 */
	long lnECCInvNum;		 /* ECC Transaction Invoice # */
	long lnSTAN;
	long lnFinalBeforeAmt;				/* 最後交易結構，交易前卡片餘額 */
	long lnFinalAfterAmt;				/* 最後交易結構，交易後卡片餘額 */
	long lnMainInvNum;					/* Confirm use 電文序號，因電票中一筆交易可能有數筆電文，所以簽單序號和電文序號分開 */
	long lnCountInvNum;					/* Confirm use 若有advice，要預先跳過的電文序號 */
	long lnTicketCTN;					/* [新增電票悠遊卡功能]  電子票證卡片交易序號  for IPASS 2.0  [SAM] 2022/6/14 上午 10:20 */
	char szUID[_UID_SIZE_ + 1];			/* 卡號 or UID number */
	char szDate[8 + 1]; /* YYMMDD */	/* [新增電票悠遊卡功能] 因使用上需要，長度由6改動為8  [SAM] 2022/6/22 上午 10:03 */
	char szOrgDate[8 + 1]; /* YYMMDD */ /* [新增電票悠遊卡功能] 因使用上需要，長度由6改動為8  [SAM] 2022/6/22 上午 10:03 */
	char szTime[6 + 1];					/* HHMMSS */
	char szOrgTime[6 + 1];				/* HHMMSS */
	char szAuthCode[_AUTH_CODE_SIZE_ + 1];
	char szECCAuthCode[_AUTH_CODE_SIZE_ + 1];
	char szRespCode[4 + 1];			 /* Response Code */
	char szProductCode[42 + 1];		 /* 產品代碼 */
	char szTicketRefundCode[12 + 1]; /* 退貨序號 */
	char szTicketRefundDate[4 + 1];	 /* MMDD */
	char szStoreID[50 + 1];
	char szRefNo[12 + 1];				/* Reference Number(RRN) */
	char szAwardNum[2 + 1];				/* 優惠個數 */
	char szAwardSN[22 + 1];				/* 優惠序號(Award S/N) TID(8Bytes)+YYYYMMDDhhmmss(16 Bytes)，共22Bytes */
	unsigned char uszAutoTopUpBit;		/* 是否自動加值 */
	unsigned char uszBlackListBit;		/* 是否在黑名單中 */
	unsigned char uszOfflineBit;		/* 離線交易 */
	unsigned char uszTicketConnectBit;	/* 是否連線中 */
	unsigned char uszResponseBit;		/* 票值回覆用 */
	unsigned char uszCloseAutoTopUpBit; /* 關閉自動加值用 */
	unsigned char uszStopPollBit;		/* Mifare Stop */
	unsigned char uszConfirmBit;		/* IPASS Confirm Inv use */
	unsigned char uszESVCTransBit;		/* 代表是電票交易 */
	IPASS_REC srIPASSRec;
	ECC_REC srECCRec;
	ICASH_REC srICASHRec;
	HAPPYCASH_REC srHAPPYCASHRec;
} TICKET_REC;

/* 交易pobTran參數，會儲存以下三個結構參數 srBRec存交易設定值，srEMVRec存EMV設定值，srTRTRec存TRT設定值
 * 其餘資料交易完成後皆會重罝
 */
typedef struct
{
	short shTrack1Len;	   /* track 1 len */
	short shTrack2Len;	   /* track 2 len */
	short shTrack3Len;	   /* track 3 len */
	int inFunctionID;	   /* 交易的類型 */
	int inRunOperationID;  /* 執行 OPT.txt */
	int inRunTRTID;		   /* 執行 xxxxTRT.txt */
	int inTransactionCode; /* Transaction Code */
	int inTransactionResult;
	int inISOTxnCode;			/* ISO code */
	int inCreditCard;			/* 判斷是否為信用卡 */
	int inUICCCard;				/* 判斷是否為銀聯卡 */
	int inFISCCard;				/* 判斷是否為金融卡 */
	int in57_Track2Len;			/* track 2 Len */
	int inTMSDwdMode;			/* 判斷自動下載或手動下載 */
	int inEFID;					/* 金融卡EFID */
	int inMenuKeyin;			/* 手動輸入顯示的第一個字 */
	int inHGTransactionCode;	/* HappyGo用 */
	int inTableID;				/* SQLite使用，在插入資料庫後從資料庫中抓出的PrimaryKey(主要用於Foreign Key) */
	int inEMVResult;			/* EMV kernel approve or decline(沒有分first失敗還是second 失敗) 目前沒用這個值判斷 */
	int inCVMResult;			/* EMV kernel判斷是否要簽名 目前沒用這個值判斷 */
	int inMultiTransactionCode; /* 只用在當外接設備時，因兩種機型之incode定義不同，所以特別開一個參數來存 */
	int inMultiNormalPayRule;	/* 只用在當外接設備時，手續費 for NP */
	int inErrorMsg;				/* 錯誤訊息代碼 */
	int inECRErrorMsgVersion;	/* 預設為0，若做不同的銀行ECR版本時，可能會遇上定義的錯誤碼差很遠難以整合，可以用版本區分 */
	int inECRErrorMsg;			/* 錯誤訊息代碼 */
	int inEMVDecision;			/* 用來分EMV Online or Offline */
	int inESC_Sale_UploadCnt;	/* 結帳前儲存用 */
	int inESC_Sale_PaperCnt;	/* 結帳前儲存用 */
	int inESC_Refund_UploadCnt; /* 結帳前儲存用 */
	int inESC_Refund_PaperCnt;	/* 結帳前儲存用 */
	int inFisc1004Len;			/* 只用在外接設備時使用，1004資料長度 */
	int inSvcTotalCnt;			/* [SVC新增欄位]  */
	int inSvcSendCnt;			/* [SVC新增欄位]  */
	int inSvcReceiveCnt;		/* [SVC新增欄位]  */
	int inSvcRecordCnt;			/* [SVC新增欄位]  */
								/* 20230116 Miyano add Start */
	int inRandomLen;
	int inQrDataLen;
	int inEncryptPanLen;
	int inTransSendLen;
	int inTransReceLen;
	int inEncryptExpireDateLen;
	int inActionCode;
	/* 20230116 Miyano add End */

	long lnMultiFdtFeeAmt;		 /* 只用在外接設備時使用，手續費 for NP */
	long lnESC_Sale_UploadAmt;	 /* 結帳前儲存用 */
	long lnESC_Sale_PaperAmt;	 /* 結帳前儲存用 */
	long lnESC_Refund_UploadAmt; /* 結帳前儲存用 */
	long lnESC_Refund_PaperAmt;	 /* 結帳前儲存用 */

	char szTransCRC; /* 20230116 Miyano add */

	char szTrack1[128 + 1]; /* track 1 buffer */
	char szTrack2[128 + 1]; /* track 2 buffer */
	char szTrack3[128 + 1]; /* track 3 buffer */
	char szT2Data[256 + 1]; /* 通常應該是存TRACK 2的ASCII 型態，但目前沒用到 */
	char szCVV2Value[4 + 1];
	char szL3_AwardWay[2 + 1];			/* 兌換方式， ‘1’=於收銀機上輸入條碼‘2’=於端末機上接BarcodeReader‘3’=卡號於端末機上輸入兌換條碼‘4’=卡號於端末機上刷卡‘5’=卡號於端末機上手動輸入 */
	char szL3_Barcode1[20 + 1];			/* 優惠兌換用一維條碼 (一)*/
	char szL3_Barcode1Len[2 + 1];		/* 優惠兌換用一維條碼長度 (一)*/
	char szL3_Barcode2[20 + 1];			/* 優惠兌換用一維條碼 (二)*/
	char szL3_Barcode2Len[2 + 1];		/* 優惠兌換用一維條碼長度 (二)*/
	char szReferralPhoneNum[30 + 1];	/* Call Bank 的電話號碼 */
	char szPIN[16 + 1];					/* Pin data pin size = 16 */
	char szFiscPin[16 + 1];				/* 金融卡Pin */
	char szMAC_HEX[8 + 1];				/* CUP 交易 */
	char szMultiFuncTID[10 + 1];		/* 只用在當外接設備時，因存進HDT太花時間 */
	char szMultiFuncMID[15 + 1];		/* 只用在當外接設備時，因存進HDT太花時間 */
	char szMultiNaitionalPay[2 + 1];	/* 只用在當外接設備時，是否有全國繳 */
	char szMultiEdcFiscIssuerId[8 + 1]; /* 只用在當外接設備時，發卡單位代號 for NP */
	char szMultiEdcFdtFeeFlag[2 + 1];	/* 只用在當外接設備時，手續費開關 for NP */
	char szMultiEdcTccCode[8 + 1];		/* 只用在當外接設備時，Tcc code for NP */
	char szFisc1004Data[256 + 1];		/* 只用在外接設備時使用，1004資料 */
	char szTicket_ErrorCode[10 + 1];	/* [新增電票悠遊卡功能] 票證使用錯誤碼 [SAM] 2022/6/8 下午 5:53 */
	char szTicketSettleBlance[1 + 1];	/* [新增電票悠遊卡功能]  悠遊卡結帳狀態 [SAM] 2022/6/30 下午 3:56 */
	char szTicketBankId[2 + 1];			/* [新增電票悠遊卡功能]  BankId  [SAM] 2022/6/30 下午 3:56 */
										/* 20230131 Miyano add Start */
	char szTerminalMasterKey[32 + 1];	/* [新增CostcoPay] Key經加密暫存 [Miyano] 2023/1/16 */
	char szTransKey[32 + 1];
	char szRandom[29 + 1]; /* 29 randoms bytes */ /* 20230116 Miyano add */
	char szTransSendData[1536 + 1];				  /* ISOload.h  ATS_ISO_SEND */
	char szTransReceData[1536 + 1];				  /* ISOload.h  ATS_ISO_RECE */
	char szEncryptPan[32 + 1];
	char szEncryptExpireDate[4 + 1];
	char szQrData[150 + 1];
	/* 20230131 Miyano add End */

	unsigned char uszUpdateBatchBit;
	unsigned char uszReversalBit;
	unsigned char uszLastBatchUploadBit;	 /* 最後一筆 Batch Upload 使用 */
	unsigned char uszConnectionBit;			 /* 判斷是否已經是連結狀態 */
	unsigned char uszTCUploadBit;			 /* EMV 0220 上傳使用(標示為當筆TC UPLOAD) */
	unsigned char uszFiscConfirmBit;		 /* SmartPay 0220 使用 */
	unsigned char uszManualUCardBit;		 /* 因為U card在manualkeyin 可能會和其他卡的卡號重複 */
	unsigned char uszCreditBit;				 /* 判斷是否為信用卡 */
	unsigned char uszUICCBit;				 /* 判斷是否為銀聯卡 */
	unsigned char uszFISCBit;				 /* 判斷是否為金融卡 */
	unsigned char uszPayWave3Tag55Bit;		 /* Wave3免簽名條件 */
	unsigned char usz57_Track2[40 + 1];		 /* track 2 buffer */
	unsigned char uszFTP_TMS_Download;		 /* 判斷是否為FTP下載 */
	unsigned char uszMACBit;				 /* 是否要送 MAC */
	unsigned char uszECRBit;				 /* 當筆是否ECR發動 */
	unsigned char uszAutoSettleBit;			 /* 當筆是否連動結帳 */
	unsigned char uszReferralBit;			 /* 當筆是否CallBank(在當筆送非advice) */
	unsigned char uszQuickPassTag99;		 /* QuickPass OnlinePIN */
	unsigned char uszDialBackup;			 /* 撥接備援使用 */
	unsigned char uszEMVProcessDisconectBit; /* 此Bit用以判斷是否等到TC UPload後才斷線 */
	unsigned char uszHGManualBit;			 /* 表示集點用的HG卡使用人工輸入方式 */
	unsigned char uszMultiFuncSlaveBit;		 /* 表示是被外接設備 */
	unsigned char uszTwoTapBit;				 /* 表示是TwoTap流程 */
	unsigned char uszTicketADVOnBit;		 /* 票證發生票值回覆 */
	unsigned char uszNotBeepBit;			 /* 這個bit On起來代表，票證交易還在進行，不響嗶聲 */
	unsigned char uszInputCheckNoBit;		 /* 這個bit On起來代表，有輸入檢查碼，確認卡號時要顯示檢查碼 */
	unsigned char uszInputTxnoBit;			 /* 這個bit On起來代表，有輸入交易編號 */
	unsigned char uszSelfServiceBit;		 /* 這個Bit On起來表示是自助式設備，不能隨便讓人按按鍵或取消(先不使用) */
	unsigned char uszSettleLOGONFailedBit;	 /* 表示結帳完的安全認證失敗，要顯示請先安全認證 */
	unsigned char uszN4TableBit;			 /* 代表有回傳Table ID "N4", for 修改分期及紅利交易防呆機制 */
	unsigned char uszN5TableBit;			 /* 代表有回傳Table ID "N5", for 修改分期及紅利交易防呆機制 */
	unsigned char uszCardInquiryFirstBit;	 /* 詢卡第一次流程 */
	unsigned char uszCardInquirysSecondBit;	 /* 詢卡第二次流程 */
	unsigned char uszECRPreferCreditBit;	 /* 只用在當外接設備時，ECR預設COMBO時，信用卡優先 */
	unsigned char uszECRPreferFiscBit;		 /* 只用在當外接設備時，ECR預設COMBO時，金融卡優先(育德說不會和uszECRPreferCreditBit同時On，湘信他) */
	unsigned char uszTMSDownloadRebootBit;	 /* 表示TMS下載完要重開機，因為可能下載還有後續要處理，所以用Bit紀錄，直到回Idle前再重開機 */
	unsigned char uszContinueTxnAgainBit;	 /* 表示TMS下載完要重開機，因為可能下載還有後續要處理，所以用Bit紀錄，直到回Idle前再重開機 */
	unsigned char fMultiMinus;
	unsigned char uszFileNameNoNeedHostBit;			/* On起來時，組成檔案名不參考Host，而以All代替 */
	unsigned char uszFileNameNoNeedNumBit;			/* On起來時，組成檔案名不參考批次號碼 */
	unsigned char uszKioskFlag;						/* Fubon 自助用 2020/1/22 上午 11:11 [SAM] */
	unsigned char uszKioskSendFlag;					/* Fubon 自助用 2020/1/22 上午 11:11 [SAM] */
	unsigned char uszEcrHasBeenAnalyseBit;			/* 已做過第一次解析的交易 */
	unsigned char uszEcrTransFlag;					/* 收銀機交易 */
	unsigned char fSvcActivedCardAuto;				/* [SVC新增欄位]  */
	unsigned char fSendToCostcoPayHost : 1;			/* 需要連線至CostcoPay主機時On起來 */
	unsigned char uszMenuSelectCancelBit;			/* 當外接設備時，出感應選單時取消，不再進行感應流程 */
	char szSVC_UID[_PAN_UCARD_SIZE_ + 1];			/* SVC UID Number */
	char szSVC_CardNum[_PAN_UCARD_SIZE_ + 1];		/* SVC CARD Number */
	char szSVC_CardExp[_PAN_UCARD_SIZE_ + 1];		/* SVC CARD EXP Number */
	char szSVC_CardIssuerID[_PAN_UCARD_SIZE_ + 1];	/* SVC CARD ISSUER ID */
	char szSVC_CardATID[_PAN_UCARD_SIZE_ + 1];		/* SVC CARD ATID */
	char szSVC_CardOtherData[_PAN_UCARD_SIZE_ + 1]; /* SVC CARD OtherData */
	BATCH_REC srBRec;
	EMV_REC srEMVRec;
	TICKET_REC srTRec;
} TRANSACTION_OBJECT;

/* TRANS_BATCH_KEY存batch資料 */
typedef struct TagTRANS_BATCH_KEY
{
	long lnSearchIndex;
	long lnInvoiceNo;	  /* 原交易的 Invoice */
	long lnBatchRecStart; /* 要搜尋 BATCH_REC 的啟始位置 */
	long lnBatchRecSize;  /* 要存的檔案大小 */
	long lnAdviceOrgSTAN; /* Advice交易的【STAN】 */
} TRANS_BATCH_KEY;

#define _TRANSACTION_OBJECT_SIZE_ sizeof(TRANSACTION_OBJECT)
#define _BATCH_REC_SIZE_ sizeof(BATCH_REC)
#define _BATCH_KEY_SIZE_ sizeof(TRANS_BATCH_KEY)
#define _EMV_REC_SIZE_ sizeof(EMV_REC)
#define _ADV_FIELD_SIZE_ 6

typedef int (*FUNC_POINT)(TRANSACTION_OBJECT *);

#endif