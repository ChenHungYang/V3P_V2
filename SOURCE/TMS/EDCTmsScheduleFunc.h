#ifndef _EDC_TMS_SCHEDULE_FUNC_H_
#define _EDC_TMS_SCHEDULE_FUNC_H_

int inEDCTMS_SCHDULE_CheckEffectiveDateTime(TRANSACTION_OBJECT * pobTran);
int inEDCTMS_SCHEDULE_CheckInquireDateTime(void);
int inEDCTMS_SCHEDULE_CheckUpdteFptInquireDateTime(char *szTMSInquireStartDate, char *szTMSInquireTime);
int inEDCTMS_SCHEDULE_CheckDownloadDateTime(TRANSACTION_OBJECT * pobTran);
int inEDCTMS_SCHEDULE_ResetDownloadDateTime(TRANSACTION_OBJECT * pobTran);
int inEDCTMS_SCHEDULE_CheckApVersionDate(void);
#endif
