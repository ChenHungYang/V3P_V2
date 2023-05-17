#ifndef _EDC_TMS_UNIT_FUNC_H_
#define _EDC_TMS_UNIT_FUNC_H_

typedef struct {
	unsigned char *uszSourceData;
	unsigned char *uszCopyData;
	int inSourceAddr;
	int inCopyAddr;
	int inCheckLen;	
}EDC_TMS_DATA_CONIG_REC;

int inEDCTMS_FUNC_ChooseManualTmsMode(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_FUNC_GetTID(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_FUNC_GetMID(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_FUNC_CheckVersion(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_FUNC_SetCommParm(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_FUNC_ConnectToServer(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_FUNC_CheckTMSOK(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_FUNC_CheckBatchFileForSetMustSettleFlag(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_FUNC_ProcessAutoUpdateAP(void);
int inEDCTMS_FUNC_ReInitialIPASSTable(void);

int inEDCTMS_FUNC_GetDataFormFileData(unsigned char *uszInData, unsigned char *uszOutData, long *InLen, long lnInDataTotalLen);
int inEDCTMS_FUNC_Reboot(TRANSACTION_OBJECT * pobTran);
int inEDCTMS_FUNC_ConfirmTheTmsWitchHost(TRANSACTION_OBJECT * pobTran);

/* 2019/10/5  因為要走不同的FTP下載，所以要分開處理 [SAM]*/
int inEDCTMS_FUNC_GetUserChooseFtpId(void);
int inEDCTMS_FUNC_SetUserChooseFtpId(int inFtpid);

int inEDCTMS_FUNC_FindAndCopyDataWhitComma(char* szSource, char* szDestn, int inMaxLen, int* inSerchCnt);

int inEDCTMS_FUNC_SetTmsFtpSettleFlagOn(void);
int inEDCTMS_FUNC_SetTmsFtpSettleFlagOff(void);

#endif
