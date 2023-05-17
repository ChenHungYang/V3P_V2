
#ifndef __FUBON_TABLE_PACK_H__
#define __FUBON_TABLE_PACK_H__

int inFUBON_PackTableN1(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inFUBON_PackTableB1(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inFUBON_PackTableC1_SendInitialData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inFUBON_PackTableC1_SendReceivedData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inFUBON_PackTableC1_ReversalSendReceivedData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inFUBON_PackTableP1(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inFUBON_PackTableP4(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inFUBON_PackTableA1(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inFUBON_PackTableA2(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
#endif

