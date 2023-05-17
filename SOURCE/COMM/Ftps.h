typedef struct
{
	int     inFtpsPort;			/* FTPS的PORT */
	int	inFtpRetryTimes;		/* 要顯示的重試次數 */
	char    szFtpsURL[512 + 1];	/* FTPS的路徑 */
	char    szFtpsID[64 + 1];		/* FTPS的使用者名稱 */
	char    szFtpsPW[64 + 1];	/* FTPS的密碼 */
	char    szFtpsFileName[64 + 1];	/* 要儲存的檔案名稱  FTPS_file.txt轉換成其他檔名移動到fs_data 資料夾 */
	char	szCACertFileName[50 + 1];	/* 憑證名稱 */
	char	szCACertFilePath[100 + 1];	/* 憑證路徑  新增 20190129 [SAM]*/
	int	inSSLModle;			/* 新增使用SSL的模式 2020/2/10 下午 3:46 [SAM]*/
} FTPS_REC;

typedef struct FtpFile
{
	char	szFilename[100 + 1];
	FILE	*pStream;
} FTP_FILE;

#define _PEM_FTPS_CREDIT_HOST_FILE_NAME_	"FTPS.pem"	

#define _DISP_PARA_DLD_MESSAGE_	0
#define _DISP_INQE_MESSAGE_		1


int inFTPS_Initial(TRANSACTION_OBJECT *pobTran);
int inFTPS_EthernetSetConfig(TRANSACTION_OBJECT *pobTran);
int inFTPS_Download(FTPS_REC *srFtpsObj);
int inFTPS_Upload(FTPS_REC *srFtpsObj);
int inFTPS_Deinitial(TRANSACTION_OBJECT *pobTran);
int inFTPS_Test(void);
int inFTPS_SetMessageForDownload( int inMessageMode);
int inFTPS_GetMessageForDownload(void);

double Int64ToDouble(int64_t  in64);
int inFunc_Disp_ProgressBar(char *szDispBar, char *szDispPer, curl_off_t cuTotal, curl_off_t cuNow);

// 非必要function  libcurl版本為7.24
void vdFunc_Check_libCurl_Version(void);
