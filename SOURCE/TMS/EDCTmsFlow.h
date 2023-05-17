#ifndef _EDC_TMS_FLOW_H_
#define _EDC_TMS_FLOW_H_

int inEDCTMS_MenuForUserSetTmsParameterFlow(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_IdleUpdateParamDataFlow(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_UpdateParamFlowForIdleUpdate(TRANSACTION_OBJECT *pobTran);

int inEDCTMS_ScheduleInquireFlow(TRANSACTION_OBJECT *pobTran);

int inEDCTMS_DownloadTmsDataAfterSettle(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_ReturnTheTmsReport(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_CheckAllDownloadFileMethodFlow(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_DeleteAllDownloadFileMethodFlow(TRANSACTION_OBJECT * pobTran);
int inEDCTMS_CheckTmsFileListExistFlow(TRANSACTION_OBJECT * pobTran);
int inEDCTMS_DeleteTmsFileListExistFlow(TRANSACTION_OBJECT * pobTran);
int inEDCTMS_SendAndReceiveFlow(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_DowloadSpconfigFileFlow(TRANSACTION_OBJECT *pobTran);

#endif
