
#ifndef __MULTI_HOST_FUNC_H__
#define __MULTI_HOST_FUNC_H__

#define V3_VID      0x0CA6
#define V3_PID      0xA020  //for usb standard mode

#define _MULTI_HOST_RS232_MAX_SIZES_	1024    /* 封包最大數 */


unsigned char* szMultiFunc_GetHostData(void);

int inMulti_HOST_PrintReceiveDeBug(char *szDataBuffer, unsigned short usReceiveBufferSize, int inDataSize);
int inMultiFunc_DispTagValue(char fDebugFlag,char *chMsg,int inMsgLen,BYTE *chData,int inDataLen);
int inMultiFunc_AnalysisEmvData(TRANSACTION_OBJECT *pobTran, unsigned char *uszCTLSData , int inDataSizes);
int inCTLS_VIVO_CheckErrorCodes(TRANSACTION_OBJECT *pobTran, unsigned char *uszErrorCodes, int inLen);
int inCTLS_VIVO_CheckStatusCodes(unsigned char btStatusCodes);
int inCTLS_VIVO_PayPassDiscretionaryData(TRANSACTION_OBJECT *pobTran, unsigned char *szPayPassData, int inPayPassLength);

int inMultiFunc_HostInital(MULTI_TABLE *stMultiOb);
int inMultiFunc_Host_DataRecvMulti( MULTI_TABLE *MultiOb, char *szComData);
int inMultiFunc_Host_DataRecv(unsigned char inHandle, int inRespTimeOut, char *szGetMultiData, unsigned char uszSendAck, MULTI_TABLE* stMultiFuncOb);
int inMultiFunc_Host_RecePacket(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *stMultiOb);
int inMultiFunc_Host_DataSend(MULTI_TABLE * stMultiOb, char *szDataBuffer, int inDataSize);
int inMultiFunc_Host_PackResult(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *srMultiHost );
int inMultiFunc_Host_SendPacketWaitACK(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *srMultiHost);
int inMultiFunc_Host_SendPacketDotWaitAck(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *srMultiHost);
int inMultiFunc_Host_CallSendErrorFunc(TRANSACTION_OBJECT * pobTran, int inErrorType);
int inMultiFunc_Host_SendError(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *stMultiOb);
int inMultiFunc_Host_DataSendMultiError(MULTI_TABLE* srMultiHost, char *szECRRespCode);
int inMultiFunc_Host_End(MULTI_TABLE* stMultiOb);

int inMultiFunc_Host_InitialComPort(TRANSACTION_OBJECT *pobTran);
int inMultiFunc_Host_RecvCtlsData(TRANSACTION_OBJECT *pobTran);
int inMultiFunc_CallSlaveCtlsResult(TRANSACTION_OBJECT *pobTran);
int inMultiFunc_CallSlave(TRANSACTION_OBJECT *pobTran, int inTranCode, MULTI_TABLE * srMultiTable);

/* 20230411 Miyano 新增 USB_Host 相關Function */
int inMultiFunc_Host_USB_Initial(void);
int inMultiFunc_Host_Data_Receive_Check(unsigned char uszComPort, unsigned short *usReceiveLen);
int inMultiFunc_Host_Data_Receive(unsigned char uszComPort, unsigned char *uszReceBuff, unsigned short *usReceSize);
int inMultiFunc_Host_Data_Send_Check(unsigned char uszComPort);
int inMultiFunc_Host_Data_Send(unsigned char uszComPort, unsigned char *uszSendBuff, unsigned short usSendSize);
int inMultiFunc_Host_Receive_ACKandNAK(MULTI_TABLE *stMultiOb);
int inMultiFunc_Host_Send_ACKorNAK(MULTI_TABLE *stMultiOb, int inResponse);
int inMultiFunc_Host_FlushRxBuffer(unsigned char uszComPort);
int inMultiFunc_Host_Send(TRANSACTION_OBJECT *pobTran, MULTI_TABLE * stMultiOb, char *szDataBuffer, int inDataSize);


#endif


