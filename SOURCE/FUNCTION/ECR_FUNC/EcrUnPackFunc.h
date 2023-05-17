#ifndef __ECR_UNPACK_FUNCTION__
#define __ECR_UNPACK_FUNCTION__

#define ECR_UNP_RL_SUCCESS      0
#define ECR_UNP_RL_ERROR         -1

int inECR_UNPACK_TimeoutResponseCode(TRANSACTION_OBJECT *pobTran, char* szUnPackBuf, int* inUnPackAddr);


#endif

