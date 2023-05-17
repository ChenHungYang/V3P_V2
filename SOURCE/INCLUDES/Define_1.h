/*
 * File:   Define.h
 * Author: user
 *
 * Created on 2015年6月7日, 下午 5:02
 */

// #define WORD_LO(xxx) ((byte) ((word)(xxx) & 255))
//
// #define WORD_HI(xxx) ((byte) ((word)(xxx) >> 8))

/* 先放著，要刪除   START */
#define _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_ 10
#define _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_ 2

#define _ESC_STATUS_NONE_ 0			 /* 未設定 */
#define _ESC_STATUS_NOT_SUPPORTED_ 1 /* 不支援ESC上傳 */
#define _ESC_STATUS_OVERLIMIT_ 2	 /* 超過水位而出紙本 */
#define _ESC_STATUS_SUPPORTED_ 3	 /* 支援ESC上傳 */

#define _APP_UPDATE_PATH_ "./fs_data/nexsysap/"

#define _TICKET_TYPE_NONE_ 0
#define _TICKET_TYPE_IPASS_ 1
#define _TICKET_TYPE_ECC_ 2
#define _TICKET_TYPE_ICASH_ 3
#define _TICKET_TYPE_HAPPYCASH_ 4

#define _ESC_UPLOAD_STATUS_UPLOAD_NOT_YET_ 0 /* ESC未上傳 */
#define _ESC_UPLOAD_STATUS_UPLOADED_ 1		 /* ESC已上傳 */
#define _ESC_UPLOAD_STATUS_PAPER_ 2			 /* 已出紙本 */

#define _TMS_INQUIRE_00_SETTLE_ "0"
#define _TMS_INQUIRE_01_SETTLE_POWERON_ "1"
#define _TMS_INQUIRE_02_SCHEDHULE_SETTLE_ "2"

#define _NEXSYS_APP_MODE_SDK_ 7

/* 因為横向簽名檔案會太大,所以要縮放後列印，與上傳的當案是不同一個 2019/07/01 [SAM] */
#define __NEW_SIGN_FUNC__

/* 因為ESC 顯示的計算方法與列印的不同，所以要改動  */
#define __USE_ACCUM_DATA__

/* 計算各功能的時間  */
// #define __TIME_UNIT_COUNT__

/* 聯合DCC 流程，如果要用可以參考 20190731 [SAM] */
// #define __NCCC_DCC_FUNC__

/*  外接設備開發中  */
#define __MUTI_FUCN_TEST_

/* 修改重覆SELECT AID 問題 */
#define _MODIFY_EMV_SELECT_AID_

/* 是否使用 Sqlite 來儲存 HDPT  */
// #define _MODIFY_HDPT_FUNC_

/* 在設定是否要清除畫面後再顯示抬頭  */
// #define _CLER_SCREEN_AND_DISP_TITLE_

#define _CONNECT_DISP_MODIFY

/* SDK 不需要下載AP 所以不更新 VersionDate.txt 2020/3/19 下午 6:30 [SAM]  */
// #if _NEXSYS_APP_MODE_ != _NEXSYS_APP_MODE_SDK_

/* 控制TMS是否要下載 VersionDate.txt 及判斷 2020/1/14 下午 3:50 [SAM]*/
#define _TMS_CHECK_VERSION_DATE_

// #endif
/* 資料庫使用 */
// #define _MODIFY_NEW_ENORMOUS_SERACH_

/* [新增電票悠遊卡功能]  程式中原支援金融卡，所以新增 _SUPPORT_FISC_HOST_
 *  用於判斷是否執行金融卡相對應流程 */
// #define _SUPPORT_FISC_HOST_

/* 要開悠遊卡功能要打開 */
// #define __ENABLE_CMAS_INIT__

#define __SVC_ENABLE__

// #define _DEBUG_9F1F_TAG_

/* 檔案輸出的目錄 */
#define _CASTLE_TYPE_NAME_V3C_ "V3C"
#define _CASTLE_TYPE_NAME_V3M_ "V3M"
#define _CASTLE_TYPE_NAME_V3P_ "V3P"
#define _CASTLE_TYPE_NAME_V3UL_ "V3UL"
#define _CASTLE_TYPE_NAME_MP200_ "MP200"

#define _CASTLE_TYPE_NAME_UPT1000_ "UPT"
#define _CASTLE_TYPE_NAME_UPT1000F_ "UPTF"
#define _CASTLE_TYPE_NAME_UNKNOWN_ "UNKNO"

/* 定義Table儲存方式 */
#define _PARAMETER_SAVE_WAY_DAT_CTOS_ 1
#define _PARAMETER_SAVE_WAY_DAT_LINUX_ 2
#define _PARAMETER_SAVE_WAY_DAT_SQLITE_ 3

#define _PARAMETER_SAVE_WAY_DAT_ _PARAMETER_SAVE_WAY_DAT_SQLITE_

#define _LOOP_TEST_

typedef struct
{
	unsigned long ulDCC_BINLen;			  /* BinTable累積的長度 */
	char szDCC_BTMC[8 + 1];				  /* Batch/File transfer message count (8 Byte) */
	char szDCC_BTFI[32 + 1];			  /* Batch/File transfer file identfication (32 Byte) */
	char szDCC_FTFS[6 + 1];				  /* File Transfer file size (6 Byte) */
	char szDCC_FTEDRC[6 + 1];			  /* File transfer elementary data record count (6 Byte) */
	char szDCC_FTREDRC[6 + 1];			  /* File transfer remaining elementary data record count (6 Byte) */
	char szDCC_FC[4 + 1];				  /* Function Code (4 Byte) */
	char szDCC_AP[2 + 1];				  /* Available Parameters (1 Byte) */
	char szDCC_AC[4 + 1];				  /* Action Code (4 Byte) */
	char szDCC_PURC[2 + 1];				  /* Parameter Update Result Code (1 Byte) */
	char szDCC_BTURC[2 + 1];			  /* BIN Table Update Ressult Code (1 Byte) */
	char szDCC_Install[2 + 1];			  /* Installation Indicator (1 Byte) */
	char szDCC_BINVersion[4 + 1];		  /* BIN Table Version (4 Byte) */
	char szDCC_FileList[8 + 1];			  /* 目前總共有7個檔案，若陣列中填入1代表需要下載該檔案 */
	char szDCC_FileDownloadStaus[8 + 1];  /* 目前總共有7個檔案，若陣列中填入1代表該檔案下載成功 */
	unsigned char uszDCC_Record[254 + 1]; /* Data Record (max to 250 Byte) */
} DCC_DATA;

DCC_DATA gsrDCC_Download;

#define _TMSFLT_FILE_NAME_ "TMSFLT.dat" /*file name*/

#define _TMS_DOWNLOAD_FLAG_NO_ "0"
#define _TMS_DOWNLOAD_FLAG_IMMEDIATE_ "1"
#define _TMS_DOWNLOAD_FLAG_SCHEDULE_ "2"

#define _TMS_DOWNLOAD_SCOPE_AP_ "1"
#define _TMS_DOWNLOAD_SCOPE_FULL_ "2"
#define _TMS_DOWNLOAD_SCOPE_PARTIAL_ "3"

/* 標示支不支援ESC狀態 */
#define _ESC_STATUS_NONE_ 0			 /* 未設定 */
#define _ESC_STATUS_NOT_SUPPORTED_ 1 /* 不支援ESC上傳 */
#define _ESC_STATUS_OVERLIMIT_ 2	 /* 超過水位而出紙本 */
#define _ESC_STATUS_SUPPORTED_ 3	 /* 支援ESC上傳 */

/* 這邊改為只用於結帳條統計 已上傳、出紙本、 重試中、 重試失敗不上傳 */
#define _ESC_ACCUM_STATUS_UPLOADED_ 1 /* ESC已上傳 */
#define _ESC_ACCUM_STATUS_BYPASS_ 2	  /* 出紙本 */
#define _ESC_ACCUM_STATUS_AGAIN_ 3	  /* 重試(過渡狀態，最後會變成已上傳或失敗) */
#define _ESC_ACCUM_STATUS_FAIL_ 4	  /* 失敗 */
#define _ESC_ACCUM_STATUS_REPRINT_ 5  /* 重印時要加總紙本數量，另外統計用 20181220 [SAM] */

/* 標示ESC上傳狀態*/
#define _ESC_UPLOAD_STATUS_UPLOAD_NOT_YET_ 0 /* ESC未上傳 */
#define _ESC_UPLOAD_STATUS_UPLOADED_ 1		 /* ESC已上傳 */
#define _ESC_UPLOAD_STATUS_PAPER_ 2			 /* 已出紙本 */

#define _ESC_REINFORCE_TXNCODE_SALE_ 1
#define _ESC_REINFORCE_TXNCODE_REFUND_ 2

int ginDCCHostIndex;

/* 先放著，要刪除   END  */

#define _TMS_TRACE_LOG_ "TraceLog.txt"

/* 宣示是那一種機型 並宣告有哪些能力 START */
#define _CASTLE_TYPE_V3C_ 0
#define _CASTLE_TYPE_V3M_ 1
#define _CASTLE_TYPE_V3P_ 2
#define _CASTLE_TYPE_V3UL_ 3
#define _CASTLE_TYPE_MP200_ 4
#define _CASTLE_TYPE_UPT1000_ 5
#define _CASTLE_TYPE_UPT1000F_ 5

/* 宣告是那一個銀行的版本，由HDT第一個Record的HostLabel判別 */
#define _APVERSION_TYPE_NCCC_ 0
#define _APVERSION_TYPE_TSB_ 1
#define _APVERSION_TYPE_TCB_ 2
#define _APVERSION_TYPE_CUB_ 3
#define _APVERSION_TYPE_FUBON_ 4
#define _APVERSION_TYPE_ESUN_ 5
#define _APVERSION_TYPE_BOT_ 99

#if _MACHINE_TYPE_ == _CASTLE_TYPE_V3C_
#define _TOUCH_CAPBILITY_		  /* 是否有觸控能力 */
#define _COMMUNICATION_CAPBILITY_ /* 是否有通訊能力 */
								  //	#define _HALF_LCD_			/* 320 * 240 */
#define _FULL_LCD_				  /* 320 * 480 */
#elif _MACHINE_TYPE_ == _CASTLE_TYPE_V3M_
#define _TOUCH_CAPBILITY_		  /* 是否有觸控能力 */
#define _COMMUNICATION_CAPBILITY_ /* 是否有通訊能力 */
#define _CRADLE_CAPBILITY_		  /* 有底座 */
#define _FULL_LCD_				  /* 320 * 480 */
#elif _MACHINE_TYPE_ == _CASTLE_TYPE_V3P_
#define _TOUCH_CAPBILITY_		  /* 是否有觸控能力 */
#define _COMMUNICATION_CAPBILITY_ /* 是否有通訊能力 */
//	#define _HALF_LCD_			/* 320 * 240 */
#define _FULL_LCD_				  /* 320 * 480 */
#elif _MACHINE_TYPE_ == _CASTLE_TYPE_V3UL_
#define _HALF_LCD_ /* 320 * 240 */
#elif _MACHINE_TYPE_ == _CASTLE_TYPE_MP200_
//	#define _TOUCH_CAPBILITY_		/* 是否有觸控能力 */
#define _COMMUNICATION_CAPBILITY_ /* 是否有通訊能力 */
#define _HALF_LCD_				  /* 320 * 240 */
#elif _MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_
#define _COMMUNICATION_CAPBILITY_ /* 是否有通訊能力 */
#define _FULL_LCD_				  /* 320 * 480 */
#else
#define _TOUCH_CAPBILITY_		  /* 是否有觸控能力 */
#define _COMMUNICATION_CAPBILITY_ /* 是否有通訊能力 */
//	#define _HALF_LCD_			/* 320 * 240 */
#define _FULL_LCD_				  /* 320 * 480 */
#endif

/* 宣示是那一種機型 並宣告有哪些能力 END */

/* 定義回傳值 */
#define VS_TRUE 1
#define VS_FALSE 0

#define VS_SUCCESS 0		/* 成功回傳值 */
#define VS_ERROR (-1)		/* 錯誤回傳值 */
#define VS_FAILURE (-2)		/* 失敗回傳值 */
#define VS_ESCAPE (-3)		/* 空白回傳值 */
#define VS_TIMEOUT (-4)		/* TIME OUT */
#define VS_HANDLE_NULL (-1) /* Ingenico機型定義為0x00，V5為-1 */

/* 新增外接感應設備回應碼  START 20190816[SAM]*/
#define VS_SMARTPAY_SUCCESS 2
#define VS_QUICKPASS_SUCCESS 3

#define VS_CTLS_ERROR -2
#define VS_CTLS_INSERT_CARD -3
#define VS_CTLE_OFFLINE_AUTH 0 /* Offline Authorization */
#define VS_CTLS_ONLINE_AUTH 23 /* Request Online Authorization */
#define VS_WAVE_MAC_ERR (-50)  /* 感應卡使用 */

#define VS_TAP_AGAIN (-18)
#define VS_TWO_TAP (-22)

/* 新增外接感應設備回應碼  END 20190816[SAM]*/

/* 定義按鍵值 */
#define _KEY_0_ d_KBD_0
#define _KEY_1_ d_KBD_1
#define _KEY_2_ d_KBD_2
#define _KEY_3_ d_KBD_3
#define _KEY_4_ d_KBD_4
#define _KEY_5_ d_KBD_5
#define _KEY_6_ d_KBD_6
#define _KEY_7_ d_KBD_7
#define _KEY_8_ d_KBD_8
#define _KEY_9_ d_KBD_9
#define _KEY_F1_ d_KBD_F1

#if (_MACHINE_TYPE_ == _CASTLE_TYPE_V3C_) || (_MACHINE_TYPE_ == _CASTLE_TYPE_V3P_) || (_MACHINE_TYPE_ == _CASTLE_TYPE_V3UL_)
#define _KEY_F1_ d_KBD_F1
#define _KEY_F2_ d_KBD_F2
#define _KEY_F3_ d_KBD_F3
#define _KEY_UP_ d_KBD_00
#define _KEY_DOWN_ d_KBD_DOT
#define _KEY_F4_ d_KBD_F4
#define _KEY_CLEAR_ d_KBD_CLEAR
#define _KEY_ENTER_ d_KBD_ENTER
#define _KEY_CANCEL_ d_KBD_CANCEL
#define _KEY_DOT_ d_KBD_DOT
#define _KEY_FUNCTION_ d_KBD_00
#define _KEY_ALPHA_ d_KBD_DOT
#define _KEY_INVALID_ d_KBD_INVALID
#elif _MACHINE_TYPE_ == _CASTLE_TYPE_V3M_
#define _KEY_F2_ d_KBD_F2
#define _KEY_F3_ d_KBD_F3
#define _KEY_UP_ d_KBD_0
#define _KEY_DOWN_ d_KBD_DOT
#define _KEY_F4_ d_KBD_F4
#define _KEY_CLEAR_ d_KBD_CLEAR
#define _KEY_ENTER_ d_KBD_ENTER
#define _KEY_CANCEL_ d_KBD_CANCEL
#define _KEY_DOT_ d_KBD_DOT
#define _KEY_FUNCTION_ d_KBD_00
#define _KEY_ALPHA_ d_KBD_DOT
#define _KEY_INVALID_ d_KBD_INVALID
#elif _MACHINE_TYPE_ == _CASTLE_TYPE_MP200_
#define _KEY_F2_ d_KBD_F2
#define _KEY_F3_ d_KBD_F3
#define _KEY_UP_ d_KBD_UP
#define _KEY_DOWN_ d_KBD_DOWN
#define _KEY_F4_ d_KBD_F4
#define _KEY_CLEAR_ d_KBD_CLEAR
#define _KEY_ENTER_ d_KBD_ENTER
#define _KEY_CANCEL_ d_KBD_CANCEL
#define _KEY_DOT_ d_KBD_DOWN
#define _KEY_FUNCTION_ d_KBD_00
#define _KEY_ALPHA_ d_KBD_DOWN
#define _KEY_INVALID_ d_KBD_INVALID
#else
#define _KEY_F2_ d_KBD_F2
#define _KEY_F3_ d_KBD_F3
#define _KEY_UP_ d_KBD_0
#define _KEY_DOWN_ d_KBD_DOT
#define _KEY_F4_ d_KBD_F4
#define _KEY_CLEAR_ d_KBD_CLEAR
#define _KEY_ENTER_ d_KBD_ENTER
#define _KEY_CANCEL_ d_KBD_CANCEL
#define _KEY_DOT_ d_KBD_DOT
#define _KEY_FUNCTION_ d_KBD_00
#define _KEY_ALPHA_ d_KBD_DOT
#define _KEY_INVALID_ d_KBD_INVALID
#endif

#define _SAM_SLOT_1_ d_SC_SAM1
#define _SAM_SLOT_2_ d_SC_SAM2
#define _SAM_SLOT_3_ d_SC_SAM3
#define _SAM_SLOT_4_ d_SC_SAM4

#define _KEY_LEFT_ -1	  /* 目前沒用到的就塞-1 */
#define _KEY_RIGHT_ -1	  /* 目前沒用到的就塞-1 */
#define _KEY_STR_ -1	  /* 目前沒用到的就塞-1 */
#define _KEY_PND_ -1	  /* 目前沒用到的就塞-1 */
#define _KEY_TIMEOUT_ 127 /* 原先為80，會與Function Key重複，但char只能放-128到127 */

/* 注意:Event類用的是Unsigned char 最多到255 */
/* 沒觸發EVENT */
typedef enum
{
	_NONE_EVENT_ = 0,			   /* 沒有觸發事件 */
	_SWIPE_EVENT_ = 140,		   /* 磁條觸發 */
	_EMV_DO_EVENT_,				   /* 晶片觸發 */
	_MENUKEYIN_EVENT_,			   /* MenuKeyIn */
	_SENSOR_EVENT_,				   /* 感應觸發 */
	_TICKET_EVENT_,				   /* 票證流程 */
	_COSTCO_SCAN_EVENT_,		   /* Costco掃碼流程 */
	_ECR_EVENT_,				   /* ECR觸發 */
	_MULTIFUNC_SLAVE_EVENT_,	   /* 當外接設備時 */
	_BARCODE_READER_EVENT_,		   /* 端末機接Barcode Reader Event */
	_POWER_MANAGEMENT_EVENT_,	   /* 進入省電模式 */
	_ESC_IDLE_UPLOAD_EVENT_,	   /* Idle上傳ESC */
	_TMS_SCHEDULE_INQUIRE_EVENT_,  /* 150 TMS 排程詢問 */
	_TMS_SCHEDULE_DOWNLOAD_EVENT_, /* TMS 排程下載 */
	_TMS_PROCESS_EFFECTIVE_EVENT_, /* TMS 參數生效 */
	_DCC_SCHEDULE_EVENT_,		   /* DCC排程下載 */
	_TMS_DCC_SCHEDULE_EVENT_,	   /* TMS連動DCC下載 */
	_BOOTING_EVENT_,			   /* 開機流程 */
} EVENT;

typedef enum
{
	VS_USER_CANCEL = (-1000),	/* 使用者取消交易 */
	VS_WAVE_INVALID_SCHEME_ERR, /* 感應卡使用 */
	VS_WAVE_AMOUNT_ERR,			/* 感應卡使用 */
	VS_WAVE_ERROR,				/* 感應卡使用 */

	VS_NO_CARD_BIN,		   /* 找不到cardbin */
	VS_CARD_PAN_ERROR,	   /* 卡號錯誤 */
	VS_CARD_EXP_ERR,	   /* 有效期錯誤 */
	VS_LAST_PAGE,		   /* 回上一層 */
	VS_PREVIOUS_PAGE,	   /* 上一頁 */
	VS_NEXT_PAGE,		   /* 下一頁 */
	VS_FUNC_CLOSE_ERR,	   /* 功能未開，回上一頁 */
	VS_HG_REWARD_COMM_ERR, /* 紅利積點失敗 */
	VS_PRINTER_OVER_HEAT,  /* 印表機過熱 */
	VS_PRINTER_PAPER_OUT,  /* 列印沒紙 */
	VS_CALLBANK,		   /* call bank */
	VS_ISO_PACK_ERR,	   /* ISO PACK 失敗 */
	VS_ISO_UNPACK_ERROR,   /* ISO UNPACK 失敗 */
	VS_COMM_ERROR,		   /* 通訊失敗 */
	VS_ICC_INSERT_ERROR,   /* 插卡失敗 */
	VS_SWIPE_ERROR,		   /* 刷卡失敗 */
	VS_FILE_ERROR,		   /* 檔案失敗 */
	VS_READ_ERROR,		   /* 讀檔失敗 */
	VS_WRITE_ERROR,		   /* 寫檔失敗 */
	VS_USER_OPER_ERR,	   /* 使用者操作錯誤 */
	VS_CLOSE_ERROR,		   /* 檔案關閉失敗 */
	VS_OPEN_ERROR,		   /* 檔案開啟失敗 */
	VS_NO_RECORD,		   /* 沒有交易記錄 */
	VS_ABORT,			   /* Operation Aborted (obsolete) */
	VS_EMV_CARD_OUT,	   /* 晶片卡被取出 */
	VS_WAVE_NO_DATA,	   /* 沒收到感應資料 */
} RESPONSE_V3;

#define VS_BOOL BOOL

#define _NULL_CH_ ((char)0)

/* Values for chip status flag */
#define _NOT_USING_CHIP_ 0
#define _EMV_CARD_ 1
#define _EMV_TABLE_NOT_USED_ -1

/* 密碼機模式 */
#define _PINPAD_MODE_0_NO_ "0"		 /* 未使用密碼機 */
#define _PINPAD_MODE_1_INTERNAL_ "1" /* 使用內建密碼機 */
#define _PINPAD_MODE_2_EXTERNAL_ "2" /* 使用外接密碼機 */

/* 感應模式 */
#define _CTLS_MODE_0_NO_ "0"	   /* 未使用感應 */
#define _CTLS_MODE_1_INTERNAL_ "1" /* 使用內建感應 */
// #define _CTLS_MODE_2_EXTERNAL_	"2"	/* 使用外接感應 */

/* 外接設備 */					  /* 20230307 Miyano add */
#define _EXTERNAL_DEVICE_A30_ "2" /* 使用A30外接 */
#define _EXTERNAL_DEVICE_V3C_ "3" /* 使用V3C外接 */

/* 簽名版模式 */
#define _SIGNPAD_MODE_0_NO_ "0"		  /* 不使用簽名板 */
#define _SIGNPAD_MODE_1_INTERNAL_ "1" /* 使用內建簽名板 */
#define _SIGNPAD_MODE_2_EXTERNAL_ "2" /* 使用外接簽名板 */

/* TMS 下載模式 */
#define _TMS_MODE_1_ISO_ "1"
#define _TMS_MODE_2_FTPS_ "2"

/* 加密模式 */
#define _NCCC_ENCRYPTION_NONE_ "0"
#define _NCCC_ENCRYPTION_TSAM_ "1"
#define _NCCC_ENCRYPTION_SOFTWARE_ "2"

/* DCC FLOW VERSION */
#define _NCCC_DCC_FLOW_VER_NOT_SUPORTED_ "0" /* 不支援DCC */
#define _NCCC_DCC_FLOW_VER_BY_CARD_BIN_ "1"	 /* 直接於詢價時由 DCC 依 Card Bin 回覆其外幣幣別及金額 */
#define _NCCC_DCC_FLOW_VER_BY_MANUAL_ "2"	 /* 於EDC選擇交易幣別詢價 */

/* Transaction Result (避開0，避免沒設定以為是cancel)*/
#define _TRAN_RESULT_CANCELLED_ 1				 /* 被主機拒絕 */
#define _TRAN_RESULT_AUTHORIZED_ 2				 /* 授權(不論online or offline)*/
#define _TRAN_RESULT_REFERRAL_ 3				 /* Call Bank */
#define _TRAN_RESULT_SETTLE_UPLOAD_BATCH_ 4		 /* Batch Upload */
#define _TRAN_RESULT_DECLINED_ -1				 /* 卡片拒絕 */
#define _TRAN_RESULT_COMM_ERROR_ -2				 /* 通訊失敗 */
#define _TRAN_RESULT_GEN2AC_ERR_ -3				 /* Second Gen AC 失敗 */
#define _TRAN_RESULT_PACK_ERR_ -4				 /* ISO PACK 失敗 */
#define _TRAN_RESULT_UNPACK_ERR_ -5				 /* ISO UNPACK 失敗*/
#define _TRAN_RESULT_HG_REWARD_COMM_ERR_ -6		 /* HG通訊失敗 */
#define _TRAN_RESULT_HG_REWARD_CANCELLED_ERR_ -7 /* HG主機拒絕 */

#define _AP_ROOT_PATH_ "./"
#define _FS_DATA_PATH_ "./fs_data/"
#define _CA_DIR_NAME_ "CA"						  /* 20090129 [SAM] */
#define _CA_DATA_PATH_ "./fs_data/CA/"			  /* 20090129 [SAM] */
#define _EMV_EMVCL_DIR_NAME_ "EMVxml"			  /* 20090129 [SAM] */
#define _EMV_EMVCL_DATA_PATH_ "./fs_data/EMVxml/" /* 20090129 [SAM] */
#define _SD_PATH_ "/media/mdisk/"
#define _USB_PATH_ "/media/udisk/"
#define _AP_PUB_PATH_ "/home/ap/pub/"

#define _AP_ROOT_DIR_NAME_ "."
#define _FS_DIR_NAME_ "fs_data"

/* [新增SVC功能]  [SAM] */
#define _SVC_HDT_INDEX_21_ 21

/* 2023/05/15 HungYang 新增SVC卡片錯誤代號*/
#define _ERRORMSG_SVC_CARD_NOT_OPEN_ -10 // 卡片未開卡
#define _ERRORMSG_NOT_SVC_CARD -11		 // 非SVC卡片

/* 錯誤訊息代碼 自定義 */
typedef enum
{
	_ERROR_CODE_V3_NONE_ = 0,			/* 0，代表無錯誤碼 */
	_ERROR_CODE_V3_USER_CANCEL_,		/* 使用者取消 */
	_ERROR_CODE_V3_COMM_,				/* 連線失敗 */
	_ERROR_CODE_V3_ISO_PACK_,			/* 電文PACK錯誤 */
	_ERROR_CODE_V3_ISO_UNPACK_,			/* 電文UNPACK錯誤 */
	_ERROR_CODE_V3_SETTLE_NOT_SUCCESS_, /* 結帳未成功 */

	_ERROR_CODE_V3_WAVE_ERROR_,					/* 感應失敗 請改插卡或刷卡 */
	_ERROR_CODE_V3_WAVE_ERROR_NOT_ONE_CARD_,	/* 感應失敗 超過一張卡 */
	_ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC, /* 不接受此感應卡 請改刷卡或插卡 */
	_ERROR_CODE_V3_FUNC_CLOSE_,					/* 此功能已關閉 */
	_ERROR_CODE_V3_CTLS_DATA_SHORT_,			/* 感應資料不足 */
	_ERROR_CODE_V3_WAVE_ERROR_Z1_,				/* 感應資料不足 */

	_ERROR_CODE_V3_TICKET_AMOUNT_TOO_MUCH_IN_ONE_TRANSACTION_, /* 金額超過單筆上限 */
	_ERROR_CODE_V3_TICKET_AMOUNT_NOT_ENOUGH_,				   /* 餘額不足 */

	_ERROR_CODE_V3_TRT_NOT_FOUND_, /* 找不到對應的TRT */

	_ERROR_CODE_V3_ECR_INST_FEE_NOT_0_, /* 分期手續費不為0 */
	_ERROR_CODE_V3_EMV_CARD_OUT_,		/* 晶片卡被取出 */
	_ERROR_CODE_V3_EMV_FALLBACK_,		/* 請改刷磁條 */

	/* Smartpay錯誤，為了統一顯示 */
	_ERROR_CODE_V3_FISC_01_READ_FAIL_,		  /* 顯示讀卡失敗 */
	_ERROR_CODE_V3_FISC_02_6982_,			  /* 顯示卡片失效 */
	_ERROR_CODE_V3_FISC_03_TIME_ERROR_,		  /* 目前無錯誤訊息(端末機抓不到時間)*/
	_ERROR_CODE_V3_FISC_04_MAC_TAC_,		  /* 目前無錯誤訊息(MAC和TAC產生失敗)*/
	_ERROR_CODE_V3_FISC_05_NO_INCODE_,		  /* 目前無錯誤訊息(沒有此交易別)*/
	_ERROR_CODE_V3_FISC_06_TMS_NOT_SUPPORT_,  /* 不接受此感應卡 請改插卡 */
	_ERROR_CODE_V3_FISC_07_AMT_OVERLIMIT_,	  /* 超過感應限額 請改插卡 */
	_ERROR_CODE_V3_FISC_08_NO_CARD_BIN_,	  /* 目前無錯誤訊息(CDT找不到Smartpay) */
	_ERROR_CODE_V3_FISC_09_NOT_RIGHT_INCODE_, /* 請依正確卡別操作 */
	_ERROR_CODE_V3_FISC_10_LOGON_FAIL_,		  /* 目前無錯誤訊息(安全認證失敗) */
	_ERROR_CODE_V3_FISC_11_FISC_FALLBACK_,	  /* 請改插金融卡 */
	_ERROR_CODE_V3_FISC_12_SEND_APDU_FAIL_,	  /* SEND APDU ERROR */

	/* 外接設備錯誤 */
	_ERROR_CODE_V3_MULTI_FUNC_CTLS_,				 /* 外接設備時，感應有問題 */
	_ERROR_CODE_V3_MULTI_FUNC_TERMINAL_,			 /* Terminal設定導致錯誤問題 */
	_ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_REAL_,		 /* 非感應SmartPay錯誤時 */
	_ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_CTLS_REAL_,	 /* 感應時，Smartpay錯誤要自己亮燈號 */
	_ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_OVER_AMOUNT_, /* 感應時，Smartpay超過感應限額，要自己亮燈號 */
	_ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_TIP_TOO_BIG_, /* 感應時，Smartpay金額小於手續費，要自己亮燈號 */

	_ERROR_CODE_V3_RECV_, /* 接收失敗 */
	/* UPT SDK用  2020/2/26 上午 10:47 [SAM] */
	_ERROR_CODE_V3_GET_HDPT_TAG_FAIL_,
	_ERROR_CODE_V3_EMV_PLS_READ_EMV_,
	_ERROR_CODE_V3_EMV_PLS_READ_EMV_CUP_,

} ERROR_CODE_V3;
