
#ifndef __FUBON_ECR_H__
#define __FUBON_ECR_H__

int inFUBON_RS232_ECR_8N1_Standard_Initial(ECR_TABLE *srECROb);
int inFUBON_RS232_ECR_8N1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inFUBON_RS232_ECR_8N1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inFUBON_RS232_ECR_8N1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inFUBON_RS232_ECR_8N1_Standard_Close(ECR_TABLE* srECRob);

int inFUBON_ECR_8N1_Standard_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);
int inFUBON_ECR_8N1_Standard_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);
int inFUBON_ECR_8N1_Standard_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);
#endif
