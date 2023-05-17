#ifndef  _EDC_TMS_FILE_PROCESS_H_
#define _EDC_TMS_FILE_PROCESS_H_

int inEDCTMS_SaveSysDownloadDateTime(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_DeleteCompareVersionDateFile(void);
int inEDCTMS_ReadFileData(unsigned long ulFileHandle, unsigned char *uszOutData, unsigned long ulReadLen);
int inEDCTMS_ProcessApVersionDateFile(char *uszOutData);
#endif
