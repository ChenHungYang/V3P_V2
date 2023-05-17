
/*
 * [檢查參數檔] 此檔案有初始檔案，會有預設值 2022/8/12 上午 10:07 [SAM]
 */

typedef struct
{
	char	szCUPFuncEnable[2 + 1];		/* 是否開啟CUP功能，該參數值為(Y/N)，預設為N */
	char	szFiscFuncEnable[2 + 1];		/* 是否開啟SmartPay功能，該參數值為(Y/N)，預設為N*/
	char	szECRComPort[4 + 1];			/* 決定ECR用哪一個ComPort */
	char	szECRVersion[2 + 1];			/* 決定哪一個ECR版本 */
	char	szPABXCode[2 + 1];			/* 撥外線的PABXCode */
	char	szTSAMRegisterEnable[2 + 1];	/* 是否已註冊tSAM，該參數值為(Y/N)，預設為N */
	char  szTMSOK[2 + 1];				/* TMS下載是否成功，該參數值為(Y/N)，預設為N */
	char	szDCCInit[2 + 1];			/* 是否還沒下過第一次DCC參數，該參數值為(1/0)，預設為1，新裝機或重裝機設為1，只要執行DCC下載過後無論成功與否，都設為0，若此bit為1則主機吐FullFileList (Installation Indicator, Request Message: If the terminal is newly installed, or re-installed, or re-initialized, the value is ‘1’ Else, the value is ‘0’)*/
	char	szDCCDownloadMode[2 + 1];	/* DCC排程下載模式 0:除非手動否則不下載 1:整點提示(如果沒有舊參數又下載失敗) 2:立即下載(用於TMS連動DCC重開機立即下載，在TMS更新完後從0設為2)*/
	char	szDCCLastUpdateDate[6 + 1];	/* YY/MM/DD 上次啟動DCC下載的時間 */
	char	szDCCSettleDownload[2 + 1];	/* 是否要在結帳後，再做一次DCC參數下載(DCC排程下載失敗時觸發，只有在結帳完或下一次自動結帳會重置) */
	char    szDCCBinVer[4 + 1];			/* DCC BinTable Version */
	char    szEDCLOCK[2 + 1];			/* EDC鎖機 */
	char    szISODebug[10 + 1];			/* Debug列印模式(有開則設為1)，[0]＝ginDebug,[1]=ginISODebug,[2]=ginDisplayDebug,[3]ginEngineerDebug,[4]TraceLogDebug */
	char    szManagerPassword[12 + 1];	/* 工程師管理密碼 */
	char    szFunctionPassword[6 + 1];	/* 裝機工程師管理密碼 */
	char    szMerchantPassword[6 + 1];	/* 商店管理密碼 */
	char    szSuperPassword[6 + 1];		/* 管理密碼 */
	char    szMCCCode[4 + 1];			
	char    szIssuerID[4 + 1];
	char    szEnterTimeout[4 + 1];		/* 輸入逾時時間 */
	char    szIPSendTimeout[2 + 1];		/* IP Send TimeOut時間 */
	char    szTermVersionID[16 + 1];		/* TermVersionID (目前不使用)*/
	char    szTermVersionDate[16 + 1];	/* TermVersionDate (目前不使用)*/
	char    szTermIPAddress[16 + 1];		/* EDC IP */
	char    szTermGetewayAddress[16 + 1];	/* TermGetewayAddress */
	char    szTermMASKAddress[16 + 1];	/* TermMASKAddress */
	char	szTermECRPort[6 + 1];		/* 當Server端時的port */
	char	szESCMode[2 + 1];			/* 是否要加上NE這個欄位 */
	char	szMultiComPort1[4 + 1];		/* 多接設備接口1(目前最多同時四個port，所以先開好欄位) */
	char	szMultiComPort1Version[2 + 1];	/* 決定第一個外接設備版本 */
	char	szMultiComPort2[4 + 1];		/* 多接設備接口2 */
	char	szMultiComPort2Version[2 + 1];	/* 決定第二個外接設備版本 */
	char	szMultiComPort3[4 + 1];		/* 多接設備接口3 */
	char	szMultiComPort3Version[2 + 1];	/* 決定第三個外接設備版本 */
	char	szEMVForceOnline[2 + 1];		/* 是否強制EMV kernel Online */
	char	szAutoConnect[10 + 1];		/* 是否自動連接(有開則設為1)，[0]＝Ethernet,[1]=WiFi, */
	char	szLOGONum[4 + 1];			/* LOGO編號 */
	char	szHostSAMSlot[2 + 1];		/* 信用卡SAM放的位置，一般來說固定為1 */
	char	szSAMSlotSN1[16 + 1];		/* 第一個SAM的SN */
	char	szSAMSlotSN2[16 + 1];		/* 第二個SAM的SN */
	char	szSAMSlotSN3[16 + 1];		/* 第三個SAM的SN */
	char	szSAMSlotSN4[16 + 1];		/* 第四個SAM的SN */
	char	szPWMEnable[2 + 1];			/* 是否開啟電源管理功能 */
	char	szPWMMode[2 + 1];			/* 設為0是Standby，設為1是Sleep */
	char	szPWMIdleTimeout[4 + 1];		/* 進省電模式Timeout */
	char	szDemoMode[2 + 1];			/* 是否開啟教育訓練模式 */
	char   szTMKOK[2 + 1];
	char   szCUPTMKOK[2 + 1];
	char   szCUPTPKOK[2 + 1];
	char   szCUPLOGONOK[2 + 1];
	char szCMASFuncEnable[2 + 1];		/* [新增電票悠遊卡功能] 是否開啟悠遊卡功能，該參數值為(Y/N)，預設為N  [SAM] 2022/6/14 下午 2:28 */
	char szCMASLinkMode[2 + 1];		/* [新增電票悠遊卡功能] 悠遊卡連線模式 1 = 間連  0 = 直連 [SAM] 2022/6/14 下午 2:28  */
	char szCMASAddTcpipMode[2 + 1];	/* [新增電票悠遊卡功能] 悠遊卡AddtionalTcpipData，0 = 不帶、 1 = NCCC、2 = CTCB、 3 = FISC [SAM] 2022/6/14 下午 2:37 */
} EDC_REC;

/* 以欄位數決定 ex:欄位數54個，理論上會有53個comma和兩個byte的0x0D 0X0A */
#define _SIZE_EDC_COMMA_0D0A_           55
#define _SIZE_EDC_REC_                  (sizeof(EDC_REC))
#define _EDC_FILE_NAME_                 "EDC.dat"
#define _EDC_FILE_NAME_BAK_        "EDC.bak"

#define _PWM_MODE_00_STANDBY_	"0"
#define _PWM_MODE_01_SLEEP_		"1"

/* Load & Save function */
int inLoadEDCRec(int inEDCRec);
int inSaveEDCRec(int inEDCRec);
int inEDC_Edit_EDC_Table(void);

/* Set function */
int inSetCUPFuncEnable(char* szCUPFuncEnable);
int inSetFiscFuncEnable(char* szFiscFuncEnable);
int inSetECRComPort(char* szECRComPort);
int inSetECRVersion(char* szECRVersion);
int inSetPABXCode(char* szPABXCode);
int inSetTSAMRegisterEnable(char* szTSAMRegisterEnable);
int inSetTMSOK(char* szTMSOK);
int inSetDCCInit(char* szDCCInit);
int inSetDCCDownloadMode(char* szDCCDownloadMode);
int inSetDCCLastUpdateDate(char* szDCCLastUpdateDate);
int inSetDCCSettleDownload(char* szDCCSettleDownload);
int inSetDCCBinVer(char* szDCCBinVer);
int inSetEDCLOCK(char* szEDCLOCK);
int inSetISODebug(char* szISODebug);
int inSetManagerPassword(char* szManagerPassword);
int inSetFunctionPassword(char* szFunctionPassword);
int inSetMerchantPassword(char* szMerchantPassword);
int inSetSuperPassword(char* szSuperPassword);
int inSetMCCCode(char* szMCCCode);
int inSetIssuerID(char* szIssuerID);
int inSetEnterTimeout(char* szEnterTimeout);
int inSetIPSendTimeout(char* szIPSendTimeout);
int inSetTermVersionID(char* szTermVersionID);
int inSetTermVersionDate(char* szTermVersionDate);
int inSetTermIPAddress(char* szTermIPAddress);
int inSetTermGetewayAddress(char* szTermGetewayAddress);
int inSetTermMASKAddress(char* szTermMASKAddress);
int inSetTermECRPort(char* szTermECRPort);
int inSetESCMode(char* szESCMode);
int inSetMultiComPort1(char* szMultiComPort1);
int inSetMultiComPort1Version(char* szMultiComPort1Version);
int inSetMultiComPort2(char* szMultiComPort2);
int inSetMultiComPort2Version(char* szMultiComPort2Version);
int inSetMultiComPort3(char* szMultiComPort3);
int inSetMultiComPort3Version(char* szMultiComPort3Version);
int inSetEMVForceOnline(char* szEMVForceOnline);
int inSetAutoConnect(char* szAutoConnect);
int inSetLOGONum(char* szLOGONum);
int inSetHostSAMSlot(char* szSAMSlot);
int inSetSAMSlotSN1(char* szSAMSlotSN1);
int inSetSAMSlotSN2(char* szSAMSlotSN2);
int inSetSAMSlotSN3(char* szSAMSlotSN3);
int inSetSAMSlotSN4(char* szSAMSlotSN4);
int inSetPWMEnable(char* szPWMEnable);
int inSetPWMMode(char* szPWMMode);
int inSetPWMIdleTimeout(char* szPWMIdleTimeout);
int inSetDemoMode(char* szDemoMode);

/* Get function */
int inGetCUPFuncEnable(char* szCUPFuncEnable);
int inGetFiscFuncEnable(char* szFiscFuncEnable);
int inGetECRComPort(char* szECRComPort);
int inGetECRVersion(char* szECRVersion);
int inGetPABXCode(char* szPABXCode);
int inGetTSAMRegisterEnable(char* szTSAMRegisterEnable);
int inGetTMSOK(char* szTMSOK);
int inGetDCCInit(char* szDCCInit);
int inGetDCCDownloadMode(char* szDCCDownloadMode);
int inGetDCCLastUpdateDate(char* szDCCLastUpdateDate);
int inGetDCCSettleDownload(char* szDCCSettleDownload);
int inGetDCCBinVer(char* szDCCBinVer);
int inGetEDCLOCK(char* szEDCLOCK);
int inGetISODebug(char* szISODebug);
int inGetManagerPassword(char* szManagerPassword);
int inGetFunctionPassword(char* szFunctionPassword);
int inGetMerchantPassword(char* szMerchantPassword);
int inGetSuperPassword(char* szSuperPassword);
int inGetMCCCode(char* szMCCCode);
int inGetIssuerID(char* szIssuerID);
int inGetEnterTimeout(char* szEnterTimeout);
int inGetIPSendTimeout(char* szIPSendTimeout);
int inGetTermVersionID(char* szTermVersionID);
int inGetTermVersionDate(char* szTermVersionDate);
int inGetTermIPAddress(char* szTermIPAddress);
int inGetTermGetewayAddress(char* szTermGetewayAddress);
int inGetTermMASKAddress(char* szTermMASKAddress);
int inGetTermECRPort(char* szTermECRPort);
int inGetESCMode(char* szESCMode);
int inGetMultiComPort1(char* szMultiComPort1);
int inGetMultiComPort1Version(char* szMultiComPort1Version);
int inGetMultiComPort2(char* szMultiComPort2);
int inGetMultiComPort2Version(char* szMultiComPort2Version);
int inGetMultiComPort3(char* szMultiComPort3);
int inGetMultiComPort3Version(char* szMultiComPort3Version);
int inGetEMVForceOnline(char* szEMVForceOnline);
int inGetAutoConnect(char* szAutoConnect);
int inGetLOGONum(char* szLOGONum);
int inGetHostSAMSlot(char* szSAMSlot);
int inGetSAMSlotSN1(char* szSAMSlotSN1);
int inGetSAMSlotSN2(char* szSAMSlotSN2);
int inGetSAMSlotSN3(char* szSAMSlotSN3);
int inGetSAMSlotSN4(char* szSAMSlotSN4);
int inGetPWMEnable(char* szPWMEnable);
int inGetPWMMode(char* szPWMMode);
int inGetPWMIdleTimeout(char* szPWMIdleTimeout);
int inGetDemoMode(char* szDemoMode);
int inGetTMKOK(char* szTMKOK);
int inSetTMKOK(char* szTMKOK);
int inGetCUPTMKOK(char* szCUPTMKOK);
int inSetCUPTMKOK(char* szCUPTMKOK);
int inGetCUPTPKOK(char* szCUPTPKOK);
int inSetCUPTPKOK(char* szCUPTPKOK);
int inGetCUPLOGONOK(char* szCUPLOGONOK);
int inSetCUPLOGONOK(char* szCUPLOGONOK);

/* [新增電票悠遊卡功能]  新增電票欄位設定功能 [SAM] 2022/6/21 下午 2:38 */
int inGetCMASFuncEnable(char* szCMASFuncEnable);
int inSetCMASFuncEnable(char* szCMASFuncEnable);
int inGetCMASLinkMode(char* szCMASLinkMode);
int inSetCMASLinkMode(char* szCMASLinkMode);
int inGetCMASAddTcpipMode(char* szCMASAddTcpipMode);
int inSetCMASAddTcpipMode(char* szCMASAddTcpipMode);


