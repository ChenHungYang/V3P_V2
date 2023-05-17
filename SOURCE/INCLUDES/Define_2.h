#define _FILE_DATA_LENGTH_MAX_	3072
#define _DEBUG_MESSAGE_SIZE_		256

/* Print Options */
#define _PRT_CUST_				0
#define _PRT_MERCH_				1
#define _PRT_MERCH_DUPLICATE_	2

#define _ADD_	0
#define _RESET_	1


#define _TRT_FILE_NAME_CREDIT_	"CREDITTRT"
#define _TRT_FILE_NAME_DINERS_	"DINERSTRT"
#define _TRT_FILE_NAME_HG_		"HGTRT"
#define _TRT_FILE_NAME_VASS_		"VASSTRT"
#define _TRT_FILE_NAME_CLM_		"CLMTRT"
#define _TRT_FILE_NAME_AMEX_		"AMEXTRT"
#define _TRT_FILE_NAME_DCC_		"DCCTRT"
#define _TRT_FILE_NAME_ESC_		"ESCTRT"
#define _TRT_FILE_NAME_ESVC_		"ESVCTRT"
#define _TRT_FILE_NAME_REDEMPTION_	"REDETRT"
#define _TRT_FILE_NAME_INSTALLMENT_	"INSTTRT"
#define _TRT_FILE_NAME_UNION_PAY_		"UNIONPAYTRT"
#define _TRT_FILE_NAME_FISC_			"FISCTRT"
#define _TRT_FILE_NAME_MAIL_ORDER_	"MOTRT"
/* [新增電票悠遊卡功能]  命名TRT的名字 [SAM] 2022/6/8 下午 5:53 */
#define _TRT_FILE_NAME_CMAS_            "CMASTRT" 

#define _TRT_FILE_NAME_SVC_		"SVCTRT"	/* [新增SVC功能]  [SAM] */

/* 因檔案名稱不能過長，所以每個主機會另取對應主機縮寫來命名 */
#define _FILE_NAME_CREDIT_		"CR"
#define _FILE_NAME_AMEX_			"AE"
#define _FILE_NAME_DINERS_		"DN"
#define _FILE_NAME_UNION_PAY_		"UP"
#define _FILE_NAME_INSTALLMENT_	"IN"
#define _FILE_NAME_REDEMPTION_	"RE"
#define _FILE_NAME_DCC_			"DC"
#define _FILE_NAME_ESC_			"ES"
#define _FILE_NAME_HG_			"HG"
#define _FILE_NAME_ESVC_			"EC"
#define _FILE_NAME_FUBON_		"FB"	/* 富邦銀行 */
#define _FILE_NAME_CMAS_			"CM"  /* [新增電票悠遊卡功能] 新增存檔檔名 [SAM] 2022/6/21 下午 4:47 */

#define _FILE_NAME_SVC_			"SV"  /* [新增SVC功能]  [SAM] */

/* 以下為新增主機時要加入的主機名稱 */
/* 程式中主要主機名稱 為 _HOST_NAME_CREDIT_MAINLY_USE_ , TODO: 如果有時間看要不要整理名稱 */

#define _HOST_NAME_CREDIT_MAINLY_USE_	"FUBON"
#define _HOST_NAME_CREDIT_FUBON_		"FUBON"


#define _HOST_NAME_DCC_	"DCC"
#define _HOST_NAME_ESC_		"ESC"
#define _HOST_NAME_DINERS_	"DINERS"
#define _HOST_NAME_AMEX_	"AMEX"
#define _HOST_NAME_ESVC_	"ESVC"
#define _HOST_NAME_INST_	"INST"
#define _HOST_NAME_REDEEM_	"REDE"
#define _HOST_NAME_ESVC_	"ESVC"

/* 因為公司TMS會下CUP主機，因富邦是同批次，所以CUP要跳過 所以新增 20190214 [SAM] */
#define _HOST_NAME_CUP_	"CUP"	

#define _HOST_NAME_SVC_	"SVC"  /* [新增SVC功能]  [SAM] */


/* 票證 */
#define _HOST_NAME_IPASS_		"IPASS"
#define _HOST_NAME_ECC_			"ECC"
#define _HOST_NAME_ICASH_		"ICASH"
#define _HOST_NAME_HAPPYCASH_	"HAPPYCASH"
/* [新增電票悠遊卡功能] [由於財金HDT HOST名稱為CMAS，故改用此HOST CMAS 名稱] 來替代原聯合使用的 ESVC [SAM] 2022/6/8 下午 5:53 */
#define _HOST_NAME_CMAS_                "CMAS" 

/* TSP */ /* 20230109 Miyano add */
#define _HOST_NAME_TSP_   "TSP"


#define _CARD_TYPE_SMARTPAY_	"SMARTPAY"
#define _CARD_TYPE_VISA_			"VISA"
#define _CARD_TYPE_MASTERCARD_	"MASTERCARD"
#define _CARD_TYPE_JCB_			"JCB"
#define _CARD_TYPE_U_CARD_		"U CARD"
#define _CARD_TYPE_AMEX_		"AMEX"
#define _CARD_TYPE_CUP_			"CUP"
#define _CARD_TYPE_DINERS_		"DISCOVER"


#define _ACCUM_FILE_EXTENSION_	".amt"

#define _BATCH_FILE_EXTENSION_		".bat"
#define _BATCH_KEY_FILE_EXTENSION_		".bkey"
#define _PICTURE_FILE_EXTENSION_		".bmp"
#define _DATA_BASE_FILE_EXTENSION_		".db"

#define _ADVICE_FILE_EXTENSION_			".adv"
#define _ADVICE_BACKUP_FILE_EXTENSION_		".adb"
#define _ADVICE_ESC_FILE_EXTENSION_		".adve"
#define _ADVICE_ESC_BACKUP_FILE_EXTENSION_	".adbe"

#define _REVERSAL_FILE_EXTENSION_				".rev"
#define _REVERSAL_DIALBACKUP_FILE_EXTENSION_	".revd"

#define _REVERSAL_ISR_FILE_EXTENSION_	".irev"

#define _GZIP_FILE_EXTENSION_	".gz"

#define _SOUND_FILE_EXTENSION_	".wav"



