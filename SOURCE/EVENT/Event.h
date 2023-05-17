
#ifndef __EVENT_H__
#define __EVENT_H__

TRANSACTION_OBJECT* tbGetPobTranPoint(void);
int  inSetPobTranPoint(TRANSACTION_OBJECT *TempPobtran);
int  inSetPobTranPointNull(void);

int inEVENT_Test(void);
int inEVENT_Responder(int inKey);
int inEVENT_AutoSettle(TRANSACTION_OBJECT *pobTran);

#endif
