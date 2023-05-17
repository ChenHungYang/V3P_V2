
#ifndef __CARD_FUNCTION_H__
#define __CARD_FUNCTION_H__

int inFunc_FormatPAN_UCARD(char *szPAN);
int inFunc_CardNumber_Hash(char *szInputData, char *szOutputData);

int inCARDFUNC_GetTransactionNoFromPANByNcccRule(TRANSACTION_OBJECT *pobTran);
int inCARDFUNC_GetPANFromTransactionNo(TRANSACTION_OBJECT *pobTran);

int  inCARDFUNC_ModifyTrack2(char *szTrack2,int inTrack2Len);
#endif
