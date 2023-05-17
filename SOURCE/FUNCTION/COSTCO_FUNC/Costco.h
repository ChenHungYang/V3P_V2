/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Costco.h
 * Author: Miyano
 *
 * Created on 2023年1月5日, 下午 5:09
 */

#ifndef COSTCO_H
#define COSTCO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../ECR_FUNC/ECR.h"
        
#define _File_Costco_TSP_Info_          "TSPInfo.txt"

#define _Costco_TSP_ISO_SEND_		2048
#define _Costco_TSP_ISO_RECV_		2048

#define DISP_STR_LEN                    26    
#define ATS_ISO_SEND                    1536
#define ATS_ISO_RECE                    1536

#define _Costco_TSP_NULL_TX_		0
#define _Costco_TSP_MAX_BIT_MAP_CNT_	40
#define _Costco_TSP_MTI_SIZE_		2
#define _Costco_TSP_PCODE_SIZE_         3
#define _Costco_TSP_TPDU_SIZE_		5
#define _Costco_TSP_BIT_MAP_SIZE_	8

#define VS_SEND_CONTINUE		10
#define VS_RECE_CONTINUE		11        

/* Actions’ Messages */
#define ACTION_UPDATE_TRANSACTION	200
#define ACTION_KEEP_ALIVE		201
#define ACTION_CHECK_UPDATE_STATUS	202
#define ACTION_DOWNLOAD_BINARY		203
#define ACTION_DOWNLOAD_PARAMETER	204
#define ACTION_SATUS_REPORT		205
#define ACTION_DIAGNOSTIC_REPORT	206
#define ACTION_DOWNLOAD_BLACKLIST	207
#define ACTION_INSTALL_BINARY		-208
#define ACTION_INSTALL_PARAMETER	-209
#define ACTION_INSTALL_BLACKLIST	-210
#define ACTION_DETOKEN_TRANSACTION	211
#define ACTION_COSTCOPAY_TRANSACTION	212
    
#define _KeyTest_Product_               1
#define _KeyTest_Test_                  0
    
#define D_Default			0
#define	D_HostValue			1

#ifndef BYTE
typedef unsigned char   BYTE;
#endif

typedef int (*PFI)();
typedef int (*PFS)();
typedef int (*PFI_VD)(void *);
typedef int (*PFI_PSZ)(char *);
typedef int (*PFI_VOID)(void);

typedef void  (*PFV_VD)(void *);
typedef int (*PFI_PFOR_SEND)(void *, char *, int , VS_BOOL, int , long );
typedef int (*PFI_PFOR_RES)(void *, char *, int ,  int , long );
typedef int (*PFI_PFOR_BEGIN)(void *, int , VS_BOOL);
typedef short (*PFI_VOID_CHAR)(void * , char *);
typedef int (*PFI_PVOID_INT)(void * , int);
typedef int (*PFI_SH_PSZ)(short, char *);
typedef int (*PFI_VD_INT)(void *, int);
typedef int (*PFI_INT_SH) (int ,unsigned short);
typedef int (*PFI_INT_INT) (int ,unsigned int);
typedef int (*PFI_VD_CHAR_BOOL) (void *, char *, VS_BOOL);

typedef struct
{
	int inTraceFileHandle;
	int inTraceTotalSizes;
	int inTraceSearchIndex;
	int inTraceReadSizes;
	int inCode;
	int inDwnStatus;
	int inErrorType;
	int inListTotalCount;
	int inListIndex; 		/* 要下載的索引 */
	int inReceTimeOut;
	int inPackNo;
	int inTransactionResult;
	int inHDTCount;
	int inERMActionflag;
	long lnReceSizes;
	long lnReceFileSize;
	long lnTotalFileSize;

	char szTPDU[10 + 1];
	char szNII[4 + 1];
	char szFileName[15 + 1];
	char szRespCode[5 + 1];
	char szOrgRespCode[2 + 1];
	char szPCode[6 + 1];
	char szTotalPacketSize[4 + 1];
	char szPackNo[4 + 1];
	char szErrFilePathName[60 + 1];
	char szDataHead[2 + 1];
	unsigned fRequest : 1;
	unsigned fAppDownload : 1;
	unsigned fParamListNotFound : 1;
        unsigned fMultiResp: 1;
} COSTCOPAY_OBJECT;    
 
//typedef struct
//{
//        int inFieldNum; /* Field Number */
//        int (*inISOLoad)(TRANSACTION_OBJECT *, unsigned char *); /* 組 Field 的功能 */
//} ISO_FIELD_COSTCO_TSP_TABLE;    
//
//typedef struct
//{
//        int inFieldNum; /* Field Number */
//        int (*inISOCheck)(TRANSACTION_OBJECT *, unsigned char *, unsigned char *); /* 檢查 Field 的功能 */
//} ISO_CHECK_COSTCO_TSP_TABLE;
//
//typedef struct
//{
//        int inFieldNum; /* Field Number */
//        int inFieldType; /* Field Type */
//        unsigned char uszDispAscii; /* 是否顯示其 ASCII 字元 */
//        int inFieldLen; /* Field Len */
//} ISO_FIELD_TYPE_COSTCO_TSP_TABLE;
//
//typedef struct
//{
//        int inTxnID; /* 交易類別 */
//        int *inBitMap; /* Bit Map */
//        char szMTI[_Costco_TSP_MTI_SIZE_ * 2 + 1]; /* Message Type */
//        char szPCode[_Costco_TSP_PCODE_SIZE_ * 2 + 1]; /* Processing Code */
//} BIT_MAP_COSTCO_TSP_TABLE;
//
//typedef struct
//{
//        ISO_FIELD_COSTCO_TSP_TABLE *srPackISO; /* 組封包的功能結構 */
//        ISO_FIELD_COSTCO_TSP_TABLE *srUnPackISO; /* 解封包的功能結構 */
//        ISO_CHECK_COSTCO_TSP_TABLE *srCheckISO; /* 檢查封包的功能結構 */
//        ISO_FIELD_TYPE_COSTCO_TSP_TABLE *srISOFieldType; /* ISO Field 型態結構 */
//        BIT_MAP_COSTCO_TSP_TABLE *srBitMap; /* Bit Map的陣列 */
//        int (*inGetBitMapCode)(TRANSACTION_OBJECT *, int);/* 取得組 Bit Map 的交易類別 */
//        int (*inPackMTI)(TRANSACTION_OBJECT *, int , unsigned char *, char *); /* 組 Message Type 的功能 */
//        int (*inModifyBitMap)(TRANSACTION_OBJECT *, int , int *);/* 組封包前修改 Bit Map 的功能 */
//        int (*inModifyPackData)(TRANSACTION_OBJECT *, unsigned char *, int *); /* 組封包後修改整個封包的功能 */
//        int (*inCheckISOHeader)(TRANSACTION_OBJECT *, char *, char *); /* 檢查 Message Type 的功能 */
//        int (*inOnAnalyse)(TRANSACTION_OBJECT *); /* Online交易分析 */
//        int (*inAdviceAnalyse)(TRANSACTION_OBJECT *, unsigned char *); /* 收到 Advice 後的分析 */
//} ISO_TYPE_COSTCO_TSP_TABLE;
   
typedef struct
{
        char TSP_IP[15+1];
        char TSP_Port[5+1];
        
        int inStatusCode;
} TSP_Info;

int CAL_LRC(char *Data);
int inStoreBCDBin(char *store1,char *store2,int data);
long lnHexStringToLong( char *szHex );

int LoadCreditScanHost(void);
int LoadRedeemScanHost(void);
int LoadInstScanHost(void);

VS_BOOL vbCheckCostcoCustom(int inVersion);

int inCostcoInitial(void);
int inSetTSP_IP(char* szTSPIPAddress);
int inSetTSP_Port(char* szTSPPortNum);
int inGetTSP_IP(char* szTSPIPAddress);
int inGetTSP_Port(char* szTSPPortNum);

int inCostco_Decide_Display_Image(TRANSACTION_OBJECT *pobTran);

int inCostco_TSP_Upload_Flow(TRANSACTION_OBJECT *pobTran);
int inCostco_SwitchToTSP_Host(int inOrgHDTIndex);
int inCostco_TSP_ProcessOnline(TRANSACTION_OBJECT *pobTran);

int inCostco_TSP_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran, COSTCOPAY_OBJECT *srPack);
int inCostco_TSP_PackISO(TRANSACTION_OBJECT *pobTran, COSTCOPAY_OBJECT *srPack, unsigned char *uszSendBuf);
int inCostco_TSP_UnPackISO(TRANSACTION_OBJECT *pobTran, COSTCOPAY_OBJECT *srTMS, unsigned char *uszUnPackBuf, int inReceLen);

int inCostco_Pack_Object_CostcoPay_UpdateTransaction(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inCostco_Pack_Object_DetokenTransaction(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);

int inCostco_TSP_Generate_TerminalMasterKey(TRANSACTION_OBJECT *pobTran);
int inCostco_TSP_Generate_TransactionKey(TRANSACTION_OBJECT *pobTran, int inTranMode);
int inCostco_Check_Key(void);

int inCostco_TSP_AES_Encryption(unsigned char *szKey, unsigned char *szPlainText, unsigned char *szCipherText);
int inCostco_TSP_AES_DataEncrypt(unsigned char *szKey, unsigned char *szPlainText, int inPlainTextLen ,unsigned char *szCipherText);
int inCostco_TSP_AES_DataDecryption(unsigned char *szKey, unsigned char *szCipherText, int inCipherTextLen, unsigned char *szPlainText, int *inPlainTextLen);

void vdCostco_TSP_PrintHexToString1(unsigned char *szReadBuffer, int inBufferLen);
void vdCostco_TSP_PrintHexToString(unsigned char *szReadBuffer);

int inCostco_TSP_CommSendRecvToHost(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket);

void vdGetTSP_TID_Data(char *szTID, int inLen);
int inCostco_TSP_ISO_CheckCRC(COSTCOPAY_OBJECT *srTMS, unsigned char *szCheckData, int inSizes);

int inCostco_TSP_ProcessObject_TLV(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf, int inDataSizes);

int inCostco_TSP_ProcessScanPay(TRANSACTION_OBJECT *pobTran);
int inCostco_ScanPay_Online_Flow(TRANSACTION_OBJECT *pobTran);

int inCostco_TSP_Pack_Object_FileParamUpdate(TRANSACTION_OBJECT *pobTran, COSTCOPAY_OBJECT *srTMS, unsigned char *uszPackBuf);

/* 20230314 Miyano add GasStation ECR */
int inCostco_GasStation_RS232_ECR_8N1_Initial(ECR_TABLE *srECROb);
int inCostco_GasStation_RS232_ECR_8N1_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inCostco_GasStation_RS232_ECR_8N1_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inCostco_GasStation_RS232_ECR_8N1_Send_Packet2(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inCostco_GasStation_RS232_ECR_8N1_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);

/* 20230411 Miyano 好市多加油站，等著收第二段P01用，有到收資料 or Timeout再Break */
int inCostco_GasStation_ECR_2nd_Receive_P01(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);

/* 20230424 Miyano add */
int inCostco_GasStation_Select_TransType(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inCostco_GasStation_Run_TransType(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);

int inCostco_GasStation_Handle_CommandResult(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, int inCommandResult);
int inCostco_GasStation_Handle_ReadCard(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, int inCommandResult);
int inCostco_GasStation_Handle_PreAuth(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, int inCommandResult);
int inCostco_GasStation_Handle_AuthComplete(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, int inCommandResult);
int inCostco_GasStation_Handle_Settle(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, int inCommandResult);
int inCostco_GasStation_Handle_PreAuth_Complete_ERR(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, int inCommandResult);
int inCostco_GasStation_Handle_ScanPay(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, int inCommandResult);
#ifdef __cplusplus
}
#endif

#endif /* COSTCO_H */

