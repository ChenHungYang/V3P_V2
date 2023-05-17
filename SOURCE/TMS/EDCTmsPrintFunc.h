#ifndef _EDC_TMS_PRINT_FUNC_H_
#define _EDC_TMS_PRINT_FUNC_H_

int inEDCTMS_PRINT_PrintDownloadStatus(TRANSACTION_OBJECT *pobTran);
int inEDCTMS_PRINT_PrintScheduleMessage(TRANSACTION_OBJECT *pobTran, int inStatus);
int inEDCTMS_PRINT_PrintReturnTask(TRANSACTION_OBJECT *pobTran);

#endif
