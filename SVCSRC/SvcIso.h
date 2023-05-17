/* 
 * File:   SvcIso.h
 * Author: Sam chang
 *
 * Created on 2022年9月14日, 上午 9:40
 */

#ifndef SVCISO_H
#define	SVCISO_H

#ifdef __cplusplus
extern "C" {
#endif

#define _SVC_CHECK_BIT_MAP_(x, b)	((x) & (1U<<(b)))

#define _SVC_ISO_TEMPLATE_SIZE		1024	
	
#define _SVC_NULL_TX_			0
#define _SVC_MAX_BIT_MAP_CNT_		40
#define _SVC_MTI_SIZE_			2
#define _SVC_PCODE_SIZE_			3
#define _SVC_TPDU_SIZE_			5/*11*/
#define _SVC_BIT_MAP_SIZE_			8
#define _SVC_RRN_SIZE_			12 /* RRN */
#define _SVC_ISO_SEND_			1536
#define _SVC_ISO_RECV_			1536
#define _SVC_ISO_ASC_				1 /* a */
#define _SVC_ISO_BCD_				2
#define _SVC_ISO_NIBBLE_2_			3 /* ..nibble */
#define _SVC_ISO_NIBBLE_3_			4 /* ...nibble */
#define _SVC_ISO_BYTE_2_			5 /* ..ans */
#define _SVC_ISO_BYTE_3_			6 /* ...ans */
#define _SVC_ISO_BYTE_2_H_			7 /* ..ans */
#define _SVC_ISO_BYTE_3_H_			8 /* ...ans */
#define _SVC_ISO_ASC_1_			9 /* a */
#define _SVC_ISO_BYTE_1_			10/* ..ans */


typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOLoad)(TRANSACTION_OBJECT *, unsigned char *); /* 組 Field 的功能 */
} ISO_FIELD_SVC_TABLE;	
	
typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOCheck)(TRANSACTION_OBJECT *, unsigned char *, unsigned char *); /* 檢查 Field 的功能 */
} ISO_CHECK_SVC_TABLE;	


typedef struct
{
        int inFieldNum; /* Field Number */
        int inFieldType; /* Field Type */
        unsigned char uszDispAscii; /* 是否顯示其 ASCII 字元 */
        int inFieldLen; /* Field Len */
} ISO_FIELD_TYPE_SVC_TABLE;

typedef struct
{
        int inTxnID; /* 交易類別 */
        int *inBitMap; /* Bit Map */
        char szMTI[_SVC_MTI_SIZE_ * 2 + 1]; /* Message Type */
        char szPCode[_SVC_PCODE_SIZE_ * 2 + 1]; /* Processing Code */
} BIT_MAP_SVC_TABLE;

typedef struct
{
        ISO_FIELD_SVC_TABLE *srPackISO; /* 組封包的功能結構 */
        ISO_FIELD_SVC_TABLE *srUnPackISO; /* 解封包的功能結構 */
        ISO_CHECK_SVC_TABLE *srCheckISO; /* 檢查封包的功能結構 */
        ISO_FIELD_TYPE_SVC_TABLE *srISOFieldType; /* ISO Field 型態結構 */
        BIT_MAP_SVC_TABLE *srBitMap; /* Bit Map的陣列 */
        int (*inGetBitMapCode)(TRANSACTION_OBJECT *, int);/*  Bit Map */
        int (*inPackMTI)(TRANSACTION_OBJECT *, int , unsigned char *, char *); /* 組 Message Type 的功能 */
        void (*vdModifyBitMap)(TRANSACTION_OBJECT *, int , int *);/* 組封包前修改 Bit Map 的功能 */
        void (*vdModifyPackData)(TRANSACTION_OBJECT *, unsigned char *, int *); /* 組封包後修改整個封包的功能 */
        int (*inCheckISOHeader)(TRANSACTION_OBJECT *, char *, char *); /* 檢查 Message Type 的功能 */
        int (*inOnAnalyse)(TRANSACTION_OBJECT *); /* Online交易分析 */
        int (*inAdviceAnalyse)(TRANSACTION_OBJECT *, VS_BOOL); /* 收到 Advice 後的分析 */
} ISO_TYPE_SVC_TABLE;


int inSVC_ISO_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inSVC_ISO_Pack11(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inSVC_ISO_Pack41(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inSVC_ISO_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inSVC_ISO_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inSVC_ISO_Pack58(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inSVC_ISO_Pack61(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inSVC_ISO_Pack64(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);

int inSVC_ISO_UnPack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inSVC_ISO_UnPack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inSVC_ISO_UnPack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inSVC_ISO_UnPack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inSVC_ISO_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inSVC_ISO_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inSVC_ISO_UnPack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inSVC_ISO_UnPack58(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inSVC_ISO_UnPack61(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);

int inSVC_ISO_Check41(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);

/* Field 功能 */
int inSVC_ISO_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_SVC_TABLE *srFieldType);
int inSVC_ISO_GetCheckField(int inField, ISO_CHECK_SVC_TABLE *srChkISO);
int inSVC_ISO_CheckUnPackField(int inField, ISO_FIELD_SVC_TABLE *srCheckUnPackField);
int inSVC_BitMapCheck(unsigned char *inBitMap, int inFeild);

int inSVC_ISO_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode);
int inSVC_ISO_UnPackISO(TRANSACTION_OBJECT *pobTran,
		unsigned char *uszSendBuf,
		int inSendLen,
		unsigned char *uszReceBuf,
		int inReceLen);

int inSVC_ISO_PackSendUnPackReceData(TRANSACTION_OBJECT *pobTran, int inTxnCode);

int inSVC_ISO_GetBMapMTIPCode(TRANSACTION_OBJECT *pobTran, ISO_TYPE_SVC_TABLE *srISOFuncIndex, int inBitMapCode);
void vdSVC_ISO_CopyISOMap(int *inBitMap, int *inSourceBitMap);
int inSVC_ISO_GetBitMap(TRANSACTION_OBJECT *pobTran,
		int inTxnType,
		ISO_TYPE_SVC_TABLE *srISOFuncIndex,		
		int *inTxnBitMap,
		unsigned char *uszSendBuf);
int inSVC_ISO_GetBitMapCode(TRANSACTION_OBJECT *pobTran, int inTxnType);
int inSVC_ISO_PackMTI(TRANSACTION_OBJECT *pobTran, int inTxnCode, unsigned char *bPackData, char *szMTI);
void vdSVC_ISO_ModifyBitMap(TRANSACTION_OBJECT *pobTran, int inTxnType, int *inBitMap);
void vdSVC_ISO_ModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *bPackData, int *inPackLen);
int inSVC_ISO_CheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader);
int inSVC_ISO_OnlineAnalyse(TRANSACTION_OBJECT *pobTran);
int inSVC_OnlineAnalyse(TRANSACTION_OBJECT *pobTran);
int inSVC_ISO_AdviceAnalyse(TRANSACTION_OBJECT *pobTran, VS_BOOL blTcUpload);
int inSVC_ISO_BuildAndSendPacket(TRANSACTION_OBJECT *pobTran);
int inSVC_ISO_ProcessReversal(TRANSACTION_OBJECT *pobTran);
int inSVC_ISO_ProcessAdvice(TRANSACTION_OBJECT *pobTran);
int inSVC_ISO_ProcessSettleBatchUpload(TRANSACTION_OBJECT *pobTran);
int inSVC_ISO_ReversalSend(TRANSACTION_OBJECT *pobTran);
int inSVC_ISO_ReversalSendRecvPacket(TRANSACTION_OBJECT *pobTran, int inISOTxnCode);
int inSVC_ISO_ReversalSave(TRANSACTION_OBJECT *pobTran, int inISOTxnCode);

int inSVC_ISO_ProcessOnline(TRANSACTION_OBJECT *pobTran);
int inSVC_ISO_ProcessOffline(TRANSACTION_OBJECT *pobTran);
int inSVC_ISO_CheckRespCode(TRANSACTION_OBJECT *pobTran);
int inSVC_ISO_CheckAuthCode(TRANSACTION_OBJECT *pobTran);
int inSVC_ISO_AnalysePack(TRANSACTION_OBJECT *pobTran);

int inSVC_ISO_SendAdvice(TRANSACTION_OBJECT *pobTran, int inAdvCnt, VS_BOOL blTcUpload);
int inSVC_ISO_ReversalSave_For_DialBeckUp(TRANSACTION_OBJECT *pobTran);
int inSVC_ISO_ReversalSave_Flow(TRANSACTION_OBJECT *pobTran, int inISOTxnCode);

int inSVC_ISO_SendReceData(TRANSACTION_OBJECT *pobTran,
		unsigned char *uszSendBuf,
		int inSendLen,
		unsigned char *uszReceBuf);

int inSVC_DispHostResponseCode(TRANSACTION_OBJECT *pobTran);

void vdSVC_ISO_ISOFormatDebug_DISP(unsigned char *bSendBuf, int inSendLen);
void vdSVC_ISO_ISOFormatDebug_PRINT(unsigned char *bSendBuf, int inSendLen);
void vdSVC_ISO_ISOFormatDebug_EMV(unsigned char *bSendBuf, int inFieldLen);
void vdSVC_ISO_ISOFormatDebug_59(unsigned char *bSendBuf, int inFieldLen);

#ifdef __cplusplus
}
#endif

#endif	/* SVCISO_H */

