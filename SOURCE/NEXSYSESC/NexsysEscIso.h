#ifndef _NEXSYS_ESC_ISO_
#define _NEXSYS_ESC_ISO_

#define _NEXSYS_ESC_CHECK_BIT_MAP_(x, b)	((x) & (1U<<(b)))

#define _NEXSYS_ESC_NULL_TX_		0
#define _NEXSYS_ESC_MAX_BIT_MAP_CNT_	40
#define _NEXSYS_ESC_MTI_SIZE_		2
#define _NEXSYS_ESC_PCODE_SIZE_	3
#define _NEXSYS_ESC_TPDU_SIZE_		5
#define _NEXSYS_ESC_BIT_MAP_SIZE_	8
#define _NEXSYS_ESC_RRN_SIZE_		12 /* RRN */
#define _NEXSYS_ESC_ISO_SEND_		3072
#define _NEXSYS_ESC_ISO_RECV_		3072
#define _NEXSYS_ESC_ISO_ASC_		1 /* a */
#define _NEXSYS_ESC_ISO_BCD_		2
#define _NEXSYS_ESC_ISO_NIBBLE_2_	3 /* ..nibble */
#define _NEXSYS_ESC_ISO_NIBBLE_3_	4 /* ...nibble */
#define _NEXSYS_ESC_ISO_BYTE_2_		5 /* ..ans */
#define _NEXSYS_ESC_ISO_BYTE_3_		6 /* ...ans */
#define _NEXSYS_ESC_ISO_BYTE_2_H_             7 /* ..ans */
#define _NEXSYS_ESC_ISO_BYTE_3_H_             8 /* ...ans */
#define _NEXSYS_ESC_ISO_ASC_1_              9 /* a */
#define _NEXSYS_ESC_ISO_BYTE_1_           10/* ..ans */

/*  有重覆使用，與 _TABLE_E1_    */
#define _NEXSYS_TABLE_E1_			31
#define _NEXSYS_TABLE_E2_			32

#define	_ESC_LIMIT_			5
//#define _ESC_ISO_MAX_LEN_		1000		/* ESC上傳時，單筆封包所含的簽單or簽名檔大小，避免整個封包超過單筆極限 */
#define _ESC_ISO_MAX_LEN_		1500		/* ESC上傳時，單筆封包所含的簽單or簽名檔大小，避免整個封包超過單筆極限 */

/* ESC電子簽單，上傳使用結構 Start */

typedef struct NEX_ESC_E1E2_DATA
{
	int	inTableIndex;				/* 用來表示E1 or E2 */
	int     inTotalPacketCnt;			/* 總封包數 */
	int     inPacketCnt;				/* 第幾個封包 */
	long  	lnTotalPacketSize;		/* 總封包大小 */
	int     inPacketSize;				/* 該封包大小 */
	char	szPackData[_ESC_ISO_MAX_LEN_ + 1];	/* 封包內容 */
	char    szLRC[1 + 1];				/* 檢查碼 = 內容長度到內容每個byteXOR */
} NEX_ESC_DATA;




/* ESC電子簽單，上傳使用結構 End */

typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOLoad)(TRANSACTION_OBJECT *, unsigned char *); /* 組 Field 的功能 */
} ISO_FIELD_NEXSYS_ESC_TABLE;

typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOCheck)(TRANSACTION_OBJECT *, unsigned char *, unsigned char *); /* 檢查 Field 的功能 */
} ISO_CHECK_NEXSYS_ESC_TABLE;

typedef struct
{
        int inFieldNum; /* Field Number */
        int inFieldType; /* Field Type */
        unsigned char uszDispAscii; /* 是否顯示其 ASCII 字元 */
        int inFieldLen; /* Field Len */
} ISO_FIELD_TYPE_NEXSYS_ESC_TABLE;

typedef struct
{
        int inTxnID; /* 交易類別 */
        int *inBitMap; /* Bit Map */
        char szMTI[_NEXSYS_ESC_MTI_SIZE_ * 2 + 1]; /* Message Type */
        char szPCode[_NEXSYS_ESC_PCODE_SIZE_ * 2 + 1]; /* Processing Code */
} BIT_MAP_NEXSYS_ESC_TABLE;

typedef struct
{
        ISO_FIELD_NEXSYS_ESC_TABLE *srPackISO; /* 組封包的功能結構 */
        ISO_FIELD_NEXSYS_ESC_TABLE *srUnPackISO; /* 解封包的功能結構 */
        ISO_CHECK_NEXSYS_ESC_TABLE *srCheckISO; /* 檢查封包的功能結構 */
        ISO_FIELD_TYPE_NEXSYS_ESC_TABLE *srISOFieldType; /* ISO Field 型態結構 */
        BIT_MAP_NEXSYS_ESC_TABLE *srBitMap; /* Bit Map的陣列 */
        int (*inGetBitMapCode)(TRANSACTION_OBJECT *, int);/* 取得組 Bit Map 的交易類別 */
        int (*inPackMTI)(TRANSACTION_OBJECT *, int , unsigned char *, char *); /* 組 Message Type 的功能 */
        int (*inModifyBitMap)(TRANSACTION_OBJECT *, int , int *);/* 組封包前修改 Bit Map 的功能 */
        int (*inModifyPackData)(TRANSACTION_OBJECT *, unsigned char *, int *); /* 組封包後修改整個封包的功能 */
        int (*inCheckISOHeader)(TRANSACTION_OBJECT *, char *, char *); /* 檢查 Message Type 的功能 */
        int (*inOnAnalyse)(TRANSACTION_OBJECT *); /* Online交易分析 */
        int (*inAdviceAnalyse)(TRANSACTION_OBJECT *, unsigned char *); /* 收到 Advice 後的分析 */
} ISO_TYPE_NEXSYS_ESC_TABLE;




int inNEXSYS_ESC_BitMapSet(int *inBitMap, int inFeild);
int inNEXSYS_ESC_BitMapReset(int *inBitMap, int inFeild);
int inNEXSYS_ESC_BitMapCheck(unsigned char *inBitMap, int inFeild);
int inNEXSYS_ESC_CopyBitMap(int *inBitMap, int *inSearchBitMap);
int inNEXSYS_ESC_GetBitMapTableIndex(ISO_TYPE_NEXSYS_ESC_TABLE *srISOFunc, int inBitMapCode);
int inNEXSYS_ESC_GetBitMapMessagegTypeField03(TRANSACTION_OBJECT *pobTran, ISO_TYPE_NEXSYS_ESC_TABLE *srISOFunc, int inTxnType, int *inTxnBitMap, unsigned char *uszSendBuf);
int inNEXSYS_ESC_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode);
int inNEXSYS_ESC_CheckUnPackField(int inField, ISO_FIELD_NEXSYS_ESC_TABLE *srCheckUnPackField);
int inNEXSYS_ESC_GetCheckField(int inField, ISO_CHECK_NEXSYS_ESC_TABLE *ISOFieldCheck);
int inNEXSYS_ESC_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_NEXSYS_ESC_TABLE *srFieldType);
int inNEXSYS_ESC_GetFieldIndex(int inField, ISO_FIELD_TYPE_NEXSYS_ESC_TABLE *srFieldType);
int inNEXSYS_ESC_UnPackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, unsigned char *uszRecvBuf);


int inNEXSYS_ESC_ISOGetBitMapCode(TRANSACTION_OBJECT *pobTran, int inTxnType);
int inNEXSYS_ESC_ISOPackMessageType(TRANSACTION_OBJECT *pobTran, int inTxnCode, unsigned char *uszPackData, char *szMTI);
int inNEXSYS_ESC_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inTxnType, int *inBitMap);
int inNEXSYS_ESC_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackData, int *inPackLen);
int inNEXSYS_ESC_ISOCheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader);

int inNEXSYS_ESC_Pack02(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack11(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack14(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack15(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack25(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack32(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack35(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack35_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack41(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack44(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack45(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack52(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack54(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack55(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack57_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack58(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack63(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNEXSYS_ESC_Pack64(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);

int inNEXSYS_ESC_Check03(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);
int inNEXSYS_ESC_Check04(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);
int inNEXSYS_ESC_Check41(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);

int inNEXSYS_ESC_UnPack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNEXSYS_ESC_UnPack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNEXSYS_ESC_UnPack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNEXSYS_ESC_UnPack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNEXSYS_ESC_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNEXSYS_ESC_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNEXSYS_ESC_UnPack55(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNEXSYS_ESC_UnPack58(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNEXSYS_ESC_UnPack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNEXSYS_ESC_UnPack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNEXSYS_ESC_UnPack63(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);

#endif
