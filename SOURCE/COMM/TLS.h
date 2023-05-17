/* 
 * File:   TLS.h
 * Author: user
 *
 * Created on 2017年7月28日, 下午 4:53
 */

#define _PEM_CA_CREDIT_NCCC_FILE_NAME_          "./fs_data/CA/nccc.com.tw.pem"
#define _PEM_CA_CREDIT_TSB_FILE_NAME_		"tsb.pem"

/* 玉山自助加油 VPN要使用SSL 2021/5/3 下午 4:27 [SAM]*/
#define _PEM_CA_CREDIT_ESUN_FILE_NAME_		"./fs_data/CA/ESUN.pem"

int inTLS_Init(void);

/* context 相關 */
int inTLS_CTX_New(int inMethod, unsigned int *inCTX_ID);
int inTLS_CTX_LoadCACertificationFromFile(unsigned int uiCTX_ID,unsigned char *uszFileName);
int inTLS_CTX_LoadCertificationFromFile(unsigned int uiCTX_ID,unsigned char *uszFileName, int inFiletype);
int inTLS_CTX_LoadCertificationFromFile(unsigned int uiCTX_ID,unsigned char *uszFileName, int inFiletype);
int inTLS_CTX_LoadPrivateKeyFromFile(unsigned int uiCTX_ID,unsigned char *uszFileName, int inFiletype);
int inTLS_CTX_SetVerificationMode(unsigned int uiCTX_ID,unsigned int uiMode);
int inTLS_CTX_Free(unsigned int *uiCTX_ID);

/* 連線相關 */
int inTLS_TLS_New(unsigned int uiCTX, unsigned int *uiSSLID);
int inTLS_SetVerificationMode(unsigned int uiSSLID, unsigned int uiMode);
int inTLS_SetCipherList(unsigned int uiSSLID,unsigned char *uszList);
int inTLS_SetSocket(unsigned int uiSSL_ID, int inSocketHandle);
int inTLS_SetProtocolVersion(unsigned int uiSSL_ID, unsigned int uiProtocolVersion);
int inTLS_TLS2_Connect(unsigned int uiSSL_ID);
int inTLS_Send_Data(unsigned int uiSSL_ID, unsigned char *uszData, int *inDataLen);
int inTLS_Read_Data(unsigned int uiSSL_ID, unsigned char *uszData, int *inDataLen);
int inTLS_TLS2_Disconnect(unsigned int uiSSL_ID, unsigned int uiFlag);
int inTLS_TLS2_Free(unsigned int *uiSSL_ID);
int inTLS_TLS2_GetSession(unsigned int uiSSL_ID);
int inTLS_TLS_SetTls2TimeOut(unsigned int uiSSLID, int inTimeOut);


/* 流程 */
int inTLS_Process_CTX_Flow(unsigned int *uiCTX_ID);
int inTLS_Process_TLS_Flow(unsigned int *uiCTX_ID, unsigned int *uiSSLID);

/* Test */
int inTLS_Test(void);

