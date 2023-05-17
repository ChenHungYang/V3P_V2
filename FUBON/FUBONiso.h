#ifndef __FUBONISO__
#define __FUBONISO__



/*#include <iso8583.h>
#include <amexhost.h>
#include <transact.h>
#include <protocol.h>
#include <packet.h>
#include <isoload.h>*/

/* Iso Debug Use 2022/10/20 [SAM] */
#define _DISPLAY_DEBUG_ISO	99
#define _PRINT_DEBUG_ISO		98

#define _FUBON_TMK_KEY_SET_INDEX_		2

#define fGetCntlessFuncEnable()			TRUE//By Ray
#define _TRACK2_SIZE_                     40              /* Size of track two data       */

#define _FUBON_CHECK_BIT_MAP_(x, b)	((x) & (1U<<(b)))

#define _FUBON_NULL_TX_                  0
#define _FUBON_MAX_BIT_MAP_CNT_          40
#define _FUBON_MTI_SIZE_                 2
#define _FUBON_PCODE_SIZE_               3
#define _FUBON_TPDU_SIZE_                /*11*/5
#define _FUBON_BIT_MAP_SIZE_             8
#define _FUBON_RRN_SIZE_                 12 /* RRN */
#define _FUBON_ISO_SEND_                1536
#define _FUBON_ISO_RECV_                 1536
#define _FUBON_ISO_ASC_                  1 /* a */
#define _FUBON_ISO_BCD_                  2
#define _FUBON_ISO_NIBBLE_2_             3 /* ..nibble */
#define _FUBON_ISO_NIBBLE_3_             4 /* ...nibble */
#define _FUBON_ISO_BYTE_2_               5 /* ..ans */
#define _FUBON_ISO_BYTE_3_               6 /* ...ans */
#define _FUBON_ISO_BYTE_2_H_             7 /* ..ans */
#define _FUBON_ISO_BYTE_3_H_             8 /* ...ans */
#define _FUBON_ISO_ASC_1_                9 /* a */
#define _FUBON_ISO_BYTE_1_               10/* ..ans */

typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOLoad)(TRANSACTION_OBJECT *, unsigned char *); /* 組 Field 的功能 */
} ISO_FIELD_FUBON_TABLE;

typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOCheck)(TRANSACTION_OBJECT *, unsigned char *, unsigned char *); /* 檢查 Field 的功能 */
} ISO_CHECK_FUBON_TABLE;

typedef struct
{
        int inFieldNum; /* Field Number */
        int inFieldType; /* Field Type */
        unsigned char uszDispAscii; /* 是否顯示其 ASCII 字元 */
        int inFieldLen; /* Field Len */
} ISO_FIELD_TYPE_FUBON_TABLE;

typedef struct
{
        int inTxnID; /* 交易類別 */
        int *inBitMap; /* Bit Map */
        char szMTI[_FUBON_MTI_SIZE_ * 2 + 1]; /* Message Type */
        char szPCode[_FUBON_PCODE_SIZE_ * 2 + 1]; /* Processing Code */
} BIT_MAP_FUBON_TABLE;

typedef struct
{
        ISO_FIELD_FUBON_TABLE *srPackISO; /* 組封包的功能結構 */
        ISO_FIELD_FUBON_TABLE *srUnPackISO; /* 解封包的功能結構 */
        ISO_CHECK_FUBON_TABLE *srCheckISO; /* 檢查封包的功能結構 */
        ISO_FIELD_TYPE_FUBON_TABLE *srISOFieldType; /* ISO Field 型態結構 */
        BIT_MAP_FUBON_TABLE *srBitMap; /* Bit Map的陣列 */
        int (*inGetBitMapCode)(TRANSACTION_OBJECT *, int);/*  Bit Map */
        int (*inPackMTI)(TRANSACTION_OBJECT *, int , unsigned char *, char *); /* 組 Message Type 的功能 */
        void (*vdModifyBitMap)(TRANSACTION_OBJECT *, int , int *);/* 組封包前修改 Bit Map 的功能 */
        void (*vdModifyPackData)(TRANSACTION_OBJECT *, unsigned char *, int *); /* 組封包後修改整個封包的功能 */
        int (*inCheckISOHeader)(TRANSACTION_OBJECT *, char *, char *); /* 檢查 Message Type 的功能 */
        int (*inOnAnalyse)(TRANSACTION_OBJECT *); /* Online交易分析 */
        int (*inAdviceAnalyse)(TRANSACTION_OBJECT *, VS_BOOL); /* 收到 Advice 後的分析 */
} ISO_TYPE_FUBON_TABLE;

int inFUBON_ISO_Pack02(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack11(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack14(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack25(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack35(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack41(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack52(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack54(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack55(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack56(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack63(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack64(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);

int inFUBON_ISO_Pack02_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack35_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack55_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack57_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);

int inFUBON_ISO_Pack02_Software(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack14_Software(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack25_Software(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack35_Software(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inFUBON_ISO_Pack55_Software(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);

int inFUBON_ISO_Check03(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);
int inFUBON_ISO_Check04(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);
int inFUBON_ISO_Check41(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);

int inFUBON_ISO_UnPack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inFUBON_ISO_UnPack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inFUBON_ISO_UnPack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inFUBON_ISO_UnPack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inFUBON_ISO_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inFUBON_ISO_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inFUBON_ISO_UnPack55(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inFUBON_ISO_UnPack56(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inFUBON_ISO_UnPack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inFUBON_ISO_UnPack63(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);

int inFUBON_ISO_GetFieldIndex(int inField, ISO_FIELD_TYPE_FUBON_TABLE *srFieldType);
int inFUBON_ISO_CUP_FuncAutoLogon(TRANSACTION_OBJECT *pobTran);
int inFUBON_ISO_PackSendUnPackReceData(TRANSACTION_OBJECT *pobTran, int inTxnCode);
int inFUBON_ISO_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode);
int inFUBON_ISO_PackISO_FOR_DialBackUp(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode);
int inFUBON_ISO_UnPackISO(TRANSACTION_OBJECT *pobTran,
			 unsigned char *uszSendBuf,
			 int inSendLen,
			 unsigned char *uszReceBuf,
			 int inReceLen);
int inFUBON_ISO_GetBitMap(TRANSACTION_OBJECT *pobTran,
			 int inTxnType,
			 ISO_TYPE_FUBON_TABLE *srISOFuncIndex,
			 int *inTxnBitMap,
			 unsigned char *uszSendBuf);
int inFUBON_ISO_GetBMapMTIPCode(TRANSACTION_OBJECT *pobTran, ISO_TYPE_FUBON_TABLE *srISOFuncIndex, int inBitMapCode);
int inFUBON_ISO_ISOGetBitMapCode(TRANSACTION_OBJECT *pobTran, int inTxnType);
int inFUBON_ISO_ISOPackMTI(TRANSACTION_OBJECT *pobTran, int inTxnCode, unsigned char *bPackData, char *szMTI);
int inFUBON_ISO_ISOCheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader);
int inFUBON_ISO_ISOOnlineAnalyse(TRANSACTION_OBJECT *pobTran);
int inFUBON_ISO_ISOAdviceAnalyse(TRANSACTION_OBJECT *pobTran, VS_BOOL blTcUpload);
int inFUBON_ISO_MapTest(int *inBitMap, int inFeild);
int inFUBON_ISO_SendReceData(TRANSACTION_OBJECT *pobTran,
	                    unsigned char *uszSendBuf,
	                    int inSendLen,
	                    unsigned char *uszReceBuf);
int inFUBON_ISO_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_FUBON_TABLE *srFieldType);
int inFUBON_ISO_GetCheckField(int inField, ISO_CHECK_FUBON_TABLE *srChkISO);
int inFUBON_ISO_CheckUnPackField(int inField, ISO_FIELD_FUBON_TABLE *srCheckUnPackField);

int inFUBON_ISO_BuildAndSendPacket(TRANSACTION_OBJECT *pobTran);
int inFUBON_ISO_ProcessReversal(TRANSACTION_OBJECT *pobTran);
int inFUBON_ISO_ProcessOnline(TRANSACTION_OBJECT *pobTran);
int inFUBON_ISO_ProcessOffline(TRANSACTION_OBJECT *pobTran);
int inFUBON_ISO_ProcessAdvice(TRANSACTION_OBJECT *pobTran);
int inFUBON_ISO_ProcessSettleBatchUpload(TRANSACTION_OBJECT *pobTran);
int inFUBON_ISO_ProcessSettleCupLogon(TRANSACTION_OBJECT *pobTran);

int inFUBON_ISO_ReversalSend(TRANSACTION_OBJECT *pobTran);
int inFUBON_ISO_ReversalSave(TRANSACTION_OBJECT *pobTran,int inISOTxnCode);
int inFUBON_ISO_ReversalSave_For_DialBeckUp(TRANSACTION_OBJECT *pobTran);
int inFUBON_ISO_CheckRespCode(TRANSACTION_OBJECT *pobTran);
int inFUBON_ISO_CheckAuthCode(TRANSACTION_OBJECT *pobTran);
int inFUBON_ISO_AnalysePack(TRANSACTION_OBJECT *pobTran);
int inFUBON_ISO_SendAdvice(TRANSACTION_OBJECT *pobTran, int inAdvCnt, VS_BOOL blTcUpload);


void vdFUBON_ISO_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inTxnType, int *inBitMap);
void vdFUBON_ISO_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *bPackData, int *inPackLen);
void vdFUBON_ISO_CopyISOMap(int *inBitMap, int *inSourceBitMap);
void vdFUBON_ISO_MapSet(int *inBitMap, int inFeild);
void vdFUBON_ISO_MapReset(int *inBitMap, int inFeild);
int inFUBON_BitMapCheck(unsigned char *inBitMap, int inFeild);

void vdProcessDebugOutPut(char *szSouce, int inDebugType);
void vdFUBON_ISO_ISOFormatDebug_DISP(unsigned char *bSendBuf, int inSendLen);
void vdFUBON_ISO_ISOFormatDebug_PRINT(unsigned char *bSendBuf, int inSendLen);
void vdFUBON_ISO_ISOFormatDebug_EMV(unsigned char *bSendBuf, int inFieldLen, int inDebugType);
void vdFUBON_ISO_ISOFormatDebug_59(unsigned char *bSendBuf, int inFieldLen, int inDebugType);

int inFUBON_CheckIncSTAN (void);
int inFUBON_ISO_ISR_ReversalSave(TRANSACTION_OBJECT *pobTran);
int inFUBON_ISO_ISR_ReversalSave_For_DialBeckUp(TRANSACTION_OBJECT *pobTran);
int inFUBON_ISO_ISR_ReversalSend(TRANSACTION_OBJECT *pobTran);
int inFUBON_ISO_MakeReferenceNumber(TRANSACTION_OBJECT *pobTran);
int inFUBON_ISO_REFERRAL_GetManualApproval(TRANSACTION_OBJECT *pobTran);

int SoftWareTransferPack(char *Data, int DataLen, char *OutBcdData);
int inFUBON_GetISOTableData(char* TableId, unsigned char* InputData,int InputLen,char* OutData,int* Outlen);
int inFUBON_ISO_ReversalSendRecvPacket(TRANSACTION_OBJECT *pobTran,int inISOTxnCode);
int inFUBON_ISO_ReversalSave_Flow(TRANSACTION_OBJECT *pobTran,int inISOTxnCode);

#endif
