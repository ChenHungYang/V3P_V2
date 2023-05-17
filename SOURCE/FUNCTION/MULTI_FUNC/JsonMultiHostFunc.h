/* 
 * File:   JsonMultiHostFunc.h
 * Author: Sam chang
 *
 * Created on 2022年11月17日, 下午 4:49
 */

#ifndef	__JSONMULTIHOSTFUNC_H__
#define	__JSONMULTIHOSTFUNC_H__

#ifdef	__cplusplus
extern "C" {
#endif

int inMultiFunc_Host_JsonDataRecv(unsigned char inHandle, int inRespTimeOut, 
				char *szGetMultiData, unsigned char uszSendAck, MULTI_TABLE* stMultiFuncOb);
int inMultiFunc_Host_JsonRecePacket(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *stMultiOb);
int inMultiFunc_Host_JsonPackData(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *srMultiHost );
int inMultiFunc_Host_JasonWaitRecvDataForCallSlve(TRANSACTION_OBJECT *pobTran);
int inMultiFunc_Host_JsonSendPacketWaitACK(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *srMultiHost);
int inMultiFunc_Host_JsonSendError(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *stMultiOb);
int inMultiFunc_Host_JsonUnpackCallSlaveCtlsResult(TRANSACTION_OBJECT *pobTran);
int inMultiFunc_JsonCallSlave(TRANSACTION_OBJECT *pobTran, int inTranCode, MULTI_TABLE * srMultiTable);

#ifdef	__cplusplus
}
#endif

#endif	/* JSONMULTIHOSTFUNC_H */

