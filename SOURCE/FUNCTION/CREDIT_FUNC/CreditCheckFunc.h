
#ifndef __CREDIT_CHECK_FUNC_H__
#define __CREDIT_CHECK_FUNC_H__

int inCREDIT_CheckTransactionFunction(int inCode);
int inCREDIT_CheckTransactionFunctionFlow(TRANSACTION_OBJECT *pobTran);
int inCREDIT_CheckVoidFunc(TRANSACTION_OBJECT *pobTran);
int inCREDIT_CheckTipFunc(TRANSACTION_OBJECT *pobTran);
int inCREDIT_CheckAdjustFunc(TRANSACTION_OBJECT *pobTran);

#endif
