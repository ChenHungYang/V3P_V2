
#ifndef __CREDIT_FUNC_H__
#define __CREDIT_FUNC_H__

int inFunc_Dial_VoiceLine(unsigned char *uszNumber, unsigned short usLen);
int inFunc_Disclaim_Auth(TRANSACTION_OBJECT *pobTran);
int inFunc_CL_Power_Off(TRANSACTION_OBJECT* pobTran);
int inFunc_SelectInstRedeemPaymentType(TRANSACTION_OBJECT *pobTran);
int inFunc_InputYearDateTime(TRANSACTION_OBJECT *pobTran);
#endif
