
#ifndef _EDC_TMS_DEFINE_H_
#define _EDC_TMS_DEFINE_H_


/*
 公司FTP
125.227.39.70
21 (走最簡單不加密的ftp)
第一組
EDC_TMSFtp1_Writer
*&!!$!)%!!^!)!!!$
第二組
EDC_TMSFtp2_Reader
*@!)!(&!))!)!!!$


國泰FTP
#define IP_FTPCB_CHT  "192.168.128.71"
#define IP_FTPCB_FET  "172.16.3.12"
#define IP_FTPCB_TFN  "210.17.125.11"
#define IP_FTPCB_4G_CHT  "192.168.128.71"
#define IP_FTPCB_4G_FET  "192.168.128.71"
#define IP_FTPCB_4G_TFN  ""
#define IP_FTPCB_COSMED  "192.168.2.128"

#define PORT_FTPCB_CHT  21
#define PORT_FTPCB_FET  21
#define PORT_FTPCB_TFN  21
#define PORT_FTPCB_4G_CHT 21
#define PORT_FTPCB_4G_FET 21
#define PORT_FTPCB_4G_TFN 0
#define PORT_FTPCB_COSMED 21

第一組
e_nexsys_w1
7YXpVgkD
第二組
e_nexsys_r_edc1
94miUd5w
 */

/* 固定要使用的資料 */
#define _CATHAY_FTP_ 99
#define _NEXSYS_FTP_	88

#define  _NEXSYS_READ_FTP_ID_ "EDC_TMSFtp2_Reader"	/* FTPS的使用者名稱 */
#define  _NEXSYS_READ_FTP_PW_ "*@!)!(&!))!)!!!$"		/* FTPS的密碼 */

#define  _NEXSYS_WRITE_FTP_ID_ "EDC_TMSFtp1_Writer"	/* FTPS的使用者名稱 */
#define  _NEXSYS_WRITE_FTP_PW_ "*&!!$!)%!!^!)!!!$" 	/* FTPS的密碼 */


#define  _ESUN_READ_FTP_ID_ "098001slc"	/* FTPS的使用者名稱 */
#define  _ESUN_READ_FTP_PW_ "0098001slc"		/* FTPS的密碼 */

#define IP_FTPCB_CHT  "192.168.128.71"
//#define IP_FTPCB_CHT  "125.227.39.70"
#define IP_FTPCB_FET  "172.16.3.12"
#define IP_FTPCB_TFN  "210.17.125.11"
#define IP_FTPCB_4G_CHT  "192.168.128.71"
#define IP_FTPCB_4G_FET  "192.168.128.71"
#define IP_FTPCB_4G_TFN  ""
#define IP_FTPCB_COSMED  "192.168.2.128"
#define IP_FTPCB_NULL  ""

#define PORT_FTPCB_CHT	21
//#define PORT_FTPCB_CHT  "990"
#define PORT_FTPCB_FET	21
#define PORT_FTPCB_TFN	21
#define PORT_FTPCB_4G_CHT	21
#define PORT_FTPCB_4G_FET		21
#define PORT_FTPCB_4G_TFN	0
#define PORT_FTPCB_COSMED	21
#define PORT_FTPCB_NONE		0

#define  _CATHAY_READ_FTP_ID_	"e_nexsys_r_edc1"	/* FTPS的使用者名稱 */
#define  _CATHAY_READ_FTP_PW_	"94miUd5w" 		/* FTPS的密碼 */

#define  _CATHAY_WRITE_FTP_ID_	"e_nexsys_w1"	/* FTPS的使用者名稱 */
#define  _CATHAY_WRITE_FTP_PW_ "7YXpVgkD"		/* FTPS的密碼 */


/* TMS inTransactionCode 使用 */
#define _EDCTMS_NULL_TX_					0
#define _EDCTMS_SCHEDULE_				1
#define _EDCTMS_LOGON_					2
#define _EDCTMS_RECORD_					3
#define _EDCTMS_TASK_					4
#define _EDCTMS_TRACE_LOG_				5
#define _EDCTMS_LOGOFF_					6
#define _EDCTMS_FTP_AUTO_INQUIRY_REPORT_	7       /* 開機自動詢問FTP回報用 */
#define _EDCTMS_FTP_DOWNLOAD_TMS_REPORT_	8	/* 下載 TMS回報用 */
#define _EDCTMS_LOGON_FTPIDPW_			9	/* 取得ID、PW */
#define _EDCTMS_AP_DOWNLOAD_			10	/* 下載AP及參數 */
#define _EDCTMS_MANUAL_DOWNLOAD_		11	/* 使用人工下載TMS參數及AP */
#define _EDCTMS_PRAM_DOWNLOAD_			12	/* 下載參數 */

#define _EDCTMS_MAX_BIT_MAP_CNT_	40

#define _EDCTMS_MTI_SIZE_		2
#define _EDCTMS_PCODE_SIZE_	3
#define _EDCTMS_TPDU_SIZE_	5
#define _EDCTMS_BIT_MAP_SIZE_	8
#define _EDCTMS_RRN_SIZE_		12 /* RRN */
#define _EDCTMS_ISO_SEND_		1536
#define _EDCTMS_ISO_RECV_		1536
#define _EDCTMS_ISO_ASC_		1 /* a */
#define _EDCTMS_ISO_BCD_		2
#define _EDCTMS_ISO_NIBBLE_2_	3 /* ..nibble */
#define _EDCTMS_ISO_NIBBLE_3_	4 /* ...nibble */
#define _EDCTMS_ISO_BYTE_2_	5 /* ..ans */
#define _EDCTMS_ISO_BYTE_3_	6 /* ...ans */
#define _EDCTMS_ISO_BYTE_2_H_	7 /* ..ans */
#define _EDCTMS_ISO_BYTE_3_H_	8 /* ...ans */
#define _EDCTMS_ISO_ASC_1_	9 /* a */

#define _EDCTMS_CANCELLED_	0
#define _EDCTMS_AUTHORIZED_	1
#define _EDCTMS_DECLINED_	-1
#define _EDCTMS_COMM_ERROR_	-2
#define _EDCTMS_PACK_ERR_	-3
#define _EDCTMS_UNPACK_ERR_	-4

/* inTMSDwdMode 使用*/
#define _EDC_TMS_MANUAL_PARM_AP_DLD_MOD_	99	/* 使用人工下載TMS參數及AP */
#define _EDC_TMS_MANUAL_PARM_DLD_MOD_	98	/* 使用人工下載TMS參數 */
#define _EDC_TMS_AUTO_PARM_DLD_MOD_		97	/* 自動下載參數 */
#define _EDC_TMS_AUTO_PARM_AP_DLD_MOD_	96	/* 自動下載參數AP */
#define _EDC_TMS_REPORT_SEND_MOD_			95	/* 回應報表 */

#define _EDC_TMS_DOWNLOAD_SCOPE_FULL_AP_			"0"
#define _EDC_TMS_DOWNLOAD_SCOPE_FULL_PARAMETER_	"1"
#define _EDC_TMS_DOWNLOAD_SCOPE_PARTIAL_PARAMETER_	"2"

/* 自動排程提示單用 START */
#define _EDC_TMS_PRT_SCHEDULE_NOFTFY_			1		/* 表示要印自動排程時間 */
#define _EDC_TMS_PRT_SCHEDULE_SUCCESS_			2		/* 表示要印自動排程下載完成 */
/* 自動排程提示單用 END */

#define _EDC_TMS_TRACE_LOG_			"TraceLog.txt"

#if defined(_CATHAY_MAIN_HOST_)
	#define _EDC_APPL_NAME_			"CBAPPL.zip"	/*  20190131 TMS [SAM] */
#elif defined(_ESUN_MAIN_HOST_)

        #define _EDC_APPL_NAME_			"EBAPPL.zip"	/*  TMS [SAM] */
#else
	#define _EDC_APPL_NAME_			"FUBONAPPL.zip"	/*  20190131 TMS [SAM] */
#endif 


#define _EDC_ESUN_APP_UPDATE_LIST_	"FileList.mmci"
#define _EDC_APP_UPDATE_LIST_			"CBAPPL/FileListDL.mmci"
#define _EDC_APP_UPDATE_LIST_V3_MFES_	"FileList_V3_MFES.mmci"

/* 下載檔案的暫存檔的路徑 */
#define _EDC_APP_UPDATE_DIR_NAME_		"nexsysap"
#define _EDC_APP_UPDATE_PATH_			"./fs_data/nexsysap/"

#define _TMS_SYSCONFIG_NAME_			"SysConfig.txt"
#define _TMS_FILELIST_NAME_			"FileList.txt"
#define _TMS_AP_VERSION_DATE_NAME_	"VersionDate.txt"

#define _TMS_SYS_BACKUP_FILE_NAME_		"SysConfig.Bak"


//#define _EDC_FTP_INFO_2_					"TermInfo2.txt"
//#define _EDC_APPL_UPDATE_CHECK_NAME_		"APPLOK"
//#define _EDC_APPL_UPDATE_CHECK_DIR_NAME_	"APPLCheck"
//#define _EDC_APPL_UPDATE_CHECK_PATH_		"./fs_data/APPLCheck/"
#define _EDC_TMS_TREMINAL_PACKET_SIZE_		"1024"

/* 控制是否要讓操作者選擇設定 */
//#define _EDC_CHOOSE_MODEL_		1


typedef struct
{
	int inCode;
	int inListTotalCount;           /* File List總共有幾個檔案要下載 */
	int inListIndex; 		/* 要下載的索引 */
	int inTotalPacketCount;
	int inPackNo;
	int inTransactionResult;
	int inDownloadFlag;             /* 0 : 無需下載  1 : 立即下載  2 : 排程下載 */
	int inDownlaodScope;            /* 1 : AP下載 2 : Full參數下載 3 : Partial參數下載 */
	/* Trace Log使用 */
	int inTraceTotalSizes;
	int inTraceSearchIndex;
	int inTraceReadSizes;
        
	long    lnFileSize;
	unsigned long   ulFile_Handle;

	char szTPDU[10 + 1];
	char szNII[4 + 1];
	char szFileName[24 + 1];
	char szRespCode[2 + 1];
	char szProcessingCode[6 + 1];
	char szTotalPacketCount[4 + 1];	/* 總封包數 */
	char szPacketNo[4 + 1];			/* 從第幾個封包開始下載 */
	char szErrFilePathName[60 + 1];	/* 記錄下載失敗的檔案路徑 */
	unsigned char uszField61[1536 + 1];	/* 下載的檔案 / Trace Log */
	unsigned char uszRequest;
	unsigned char uszDownlodType;	/* 1:Application Downlaod 2:Parameter Download */
} EDC_TMS_OBJECT;


typedef struct
{
	short shFTPDownloadFileCount;		/* 需下載之檔案總數*/
	int	inHeader_length;                        
	char	szIndex[2 + 1];				/* 該record 是在 TermInfo.txt 第幾筆 record */
	char	szFile_Attribute[1 + 1];			/* 該 record 的屬性 分 H(頭定義) , A(AP) , P(參數) */
	char	szHeader_TID[8 + 1]; 			/* TID 資料 */
	char	szHeader_MID[15 + 1]; 			/* MID 資料 */
	char	szHeader_MFES_ID[4 + 1]; 		/* MFES ID 資料 */
	char	szHeader_AutoDownloadFlag[1 + 1]; 	/* 可否自動下載 */
	char	szHeader_StartDownloadDate[12 + 1]; 	/* EDC可以開始下載的日期 */
	char	szHeader_TermApVersion[15 + 1];		/* 下載AP 的 AP ID */
	char	szHeader_TermApVersionDate[15 + 1];	/* 下載AP 的 AP 日期 */
	char	szHeader_TermParmDateTime[12 + 1];	/* EDC 參數異動日期時間 */
	char 	szHeader_CloseBatchFlag[1 + 1];		/* EDC是否須檢查有無帳務才可更新 */
	char	szHeader_BatchNumber[8 + 1];		/* 作業批號 */
} EDC_FTP_OBJECT;




#endif
