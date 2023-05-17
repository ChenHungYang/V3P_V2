#ifndef _EDC_TMS_FTP_FUNC_H_
#define _EDC_TMS_FTP_FUNC_H_

int inEDCTMS_FTP_FtpsDownloadApVersionFile(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_FTP_FtpsDownloadSpconfig(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_FTP_FtpsDownloadFileList(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_FTP_FtpsUpload(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_FTP_FtpsCheckTidAndMid(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_FTP_FtpsDownloadFileByFileList(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_FTP_FtpsUpdateFinalParameter(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_FTP_FtpsCheckAllDownloadFileList(void);
int inEDCTMS_FTP_FtpsDeleteAllDownloadFileList(void);
int inEDCTMS_FTP_FtpsSendReceiveFlow(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_FTP_FtpsSendReceiveFlowByTransCode(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_FTP_FtpsCheckFileListSize(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_FTP_FtpsOnlyDownloadAp(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_FTP_FtpsUpdateFinalApp(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_FTP_FtpsEsunUpload(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_FTP_CustomerIndicator_Process(void);

#endif
