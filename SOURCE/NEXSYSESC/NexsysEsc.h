#ifndef _NEXSYS_ESC_FLOW_
#define _NEXSYS_ESC_FLOW_

#define _NEXSYS_ESC_CANCELLED_		0
#define _NEXSYS_ESC_AUTHORIZED_	1
#define _NEXSYS_ESC_REFERRAL_		2
#define _NEXSYS_ESC_SETTLE_UPLOAD_BATCH_                  3
#define _NEXSYS_ESC_DECLINED_			-1
#define _NEXSYS_ESC_COMM_ERROR_		-2
#define _NEXSYS_ESC_GEN2AC_ERR_		-3
#define _NEXSYS_ESC_PACK_ERR_			-4
#define _NEXSYS_ESC_UNPACK_ERR_		-5

/* ESC使用 */
#define _ESC_FILE_RECEIPT_GZ_			"ESC_E1.txt.gz"
#define _ESC_FILE_RECEIPT_GZ_ENCRYPTED_	"ESC_E1_E.txt.gz"
#define _ESC_FILE_BMP_GZ_				"ESC_E2.bmp.gz"
#define _ESC_FILE_BMP_GZ_ENCRYPTED_	"ESC_E2_E.bmp.gz"
#define  _ESC_FILE_BMP_BACKUP_		"ESC_BMP_BACKUP.bmp"
#define _TABLE_E1_			31
#define _TABLE_E2_			32
//#define _ESC_ISO_MAX_LEN_		720		/* ESC上傳時，單筆封包所含的簽單or簽名檔大小，避免整個封包超過單筆極限 */
//#define	_ESC_LIMIT_			5

/* 標示支不支援ESC狀態 */
#define	_ESC_STATUS_NONE_			0		/* 未設定 */
#define	_ESC_STATUS_NOT_SUPPORTED_		1		/* 不支援ESC上傳 */
#define _ESC_STATUS_OVERLIMIT_			2		/* 超過水位而出紙本 */
#define _ESC_STATUS_SUPPORTED_			3		/* 支援ESC上傳 */

/* 這邊改為只用於結帳條統計 已上傳、出紙本、 重試中、 重試失敗不上傳 */
#define _ESC_ACCUM_STATUS_UPLOADED_	1		/* ESC已上傳 */
#define _ESC_ACCUM_STATUS_BYPASS_		2		/* 出紙本 */
#define _ESC_ACCUM_STATUS_AGAIN_		3		/* 重試(過渡狀態，最後會變成已上傳或失敗) */
#define _ESC_ACCUM_STATUS_FAIL_		4		/* 失敗 */
#define _ESC_ACCUM_STATUS_REPRINT_	5		/* 重印時要加總紙本數量，另外統計用 20181220 [SAM] */

/* 標示ESC上傳狀態*/
#define _ESC_UPLOAD_STATUS_UPLOAD_NOT_YET_	0	/* ESC未上傳 */
#define _ESC_UPLOAD_STATUS_UPLOADED_		1	/* ESC已上傳 */
#define _ESC_UPLOAD_STATUS_PAPER_			2	/* 已出紙本 */

/* IDLE下是否上傳 */
#define _ESC_IDLE_NO_NEED_UPLOAD_	0		/* 不需上傳 */
#define _ESC_IDLE_UPLOAD_		1		/* 需要上傳 */

#define _ESC_SEND_RETRY_LIMIT_		1

/* 和esc主機連線狀態 */
#define	_ESC_CONNECT_NOT_CONNECT_	0		/* 沒連 */
#define _ESC_CONNECT_ALREADY_		1		/* 有連 */

/* 在什麼狀態下上傳ESC */
#define _ESC_UPLOAD_TRADE_		1		/* 當筆 */
#define _ESC_UPLOAD_IDLE_			2		/* IDLE */
#define _ESC_UPLOAD_SETTLE_		3		/* 結帳 */

#define _ACQ_ID_		        "00493817"

#define	_ESC_REINFORCE_TXNCODE_SALE_	1
#define	_ESC_REINFORCE_TXNCODE_REFUND_	2




typedef struct
{
	int	inESCSaleUploadCnt;		/* Sale已上傳筆數 */
	int	inESCRefundUploadCnt;		/* Refund已上傳筆數 */
	int	inESCSalePaperCnt;		/* Sale出紙本筆數 */
	int	inESCRefundPaperCnt;		/* Refund出紙本筆數 */
	long	inESCSaleUploadAmt;		/* Sale已上傳金額 */
	long	inESCRefundUploadAmt;		/* Refund已上傳金額 */
	long	inESCSalePaperAmt;		/* Sale出紙本金額 */
	long	inESCRefundPaperAmt;		/* Refund出紙本金額 */
} NEX_ESC_REINFORCE;

int inNEXSYS_ESC_Check(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_MerchantCopy_Check(TRANSACTION_OBJECT *pobTran);

int inNEXSYS_ESC_Func_Upload(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_Func_Upload_Idle(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_Func_Upload_AfterTrans(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_Func_Upload_Settle(TRANSACTION_OBJECT *pobTran);



int inNEXSYS_ESC_ISOOnlineAnalyse(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_ISOAdviceAnalyse(TRANSACTION_OBJECT *pobTran, unsigned char *uszTcUpload);

int inNEXSYS_ESC_GetSTAN(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_SetSTAN(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_ProcessReversal(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_AdviceSendRecvPacket(TRANSACTION_OBJECT *pobTran, int inAdvCnt);
int inNEXSYS_ESC_ProcessAdvice(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_ProcessOnline(TRANSACTION_OBJECT *pobTran, int inType);
int inNEXSYS_ESC_ProcessOffline(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_AnalysePacket(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_ReversalSendRecvPacket(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_ReversalSave(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_CommSendRecvToHost(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket);
int inNEXSYS_ESC_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_CheckRespCode(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_CheckAuthCode(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_OnlineAnalyseMagneticManual(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_ProcessSettleBatchUpload(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_DispHostResponseCode(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_ProcessReferral(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_GetReversalCnt(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_SetReversalCnt(TRANSACTION_OBJECT *pobTran, long lnSetMode);
int inNEXSYS_ESC_GetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit);
int inNEXSYS_ESC_SetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit);
int inNEXSYS_ESC_CUP_PowerOnLogon(void);
int inNEXSYS_ESC_TMKKeyExchange(TRANSACTION_OBJECT *pobTran,unsigned char *uszUnPackBuf);
void vdNEXSYS_ESC_ISO_FormatDebug(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen);

int inNEXSYS_ESC_GetESC_Enable(int inOrgHDTIndex, char* szHostEnable);
int inNEXSYS_ESC_SwitchToESC_Host(int inOrgHDTIndex);
int inNEXSYS_ESC_Data_Compress_Encryption(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_Data_Packet(char *szFileName, int inTableIndex);
int inNEXSYS_ESC_Insert_Again_Record(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_Insert_Fail_Record(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_Get_Again_Record_Most_TOP(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_Get_Fail_Record_Most_TOP(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_Delete_Again_Record_Most_TOP(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_Delete_Fail_Record_Most_TOP(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_Again_Table_Count(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_Fail_Table_Count(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_Process_PowerOff_When_Signing(TRANSACTION_OBJECT *pobTran);
int inNEXSYS_ESC_Func_Upload_Idle_Check_Time(void);

void vdNEXSYS_ESC_ISO_FormatDebug_DISP(unsigned char *uszSendBuf, int inSendLen);
void vdNEXSYS_ESC_ISO_FormatDebug_DISP_59(unsigned char *uszSendBuf, int inFieldLen);
void vdNEXSYS_ESC_ISO_FormatDebug_PRINT(unsigned char *uszSendBuf, int inSendLen);
void vdNEXSYS_ESC_ISO_FormatDebug_PRINT_59(unsigned char *uszSendBuf, int inFieldLen, unsigned char *uszBuffer,FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);

int inNEXSYS_ESC_Func_UpdateReprintCount(TRANSACTION_OBJECT *pobTran);
#endif
