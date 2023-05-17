
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>


#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/Transaction.h"

#include "../SOURCE/EVENT/MenuMsg.h"

#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/DISPLAY/DisTouch.h"

#include "../SOURCE/COMM/Comm.h"

#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/FILE_FUNC/File.h"
#include "../SOURCE/FUNCTION/FILE_FUNC/FIleLogFunc.h"
#include "../SOURCE/FUNCTION/Batch.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/CFGT.h"

#include "../EMVSRC/EMVsrc.h"

#include "SvcFunc.h"
#include "SvcPackTagFunc.h"
#include "SvcIso.h"
#include "SvcTable.h"


extern EMV_CONFIG EMVGlobConfig;

extern int ginDebug; /* Debug使用 extern */
extern int ginISODebug;

char szSVC_ISO_Field03[_SVC_PCODE_SIZE_ + 1];

/* 
 *  *  inSVC_ISO_BuildAndSendPacket ()   綁定 FUNCTION ID 會在TRT流程中執行
 * 
 */

static long lnSVC_BalaceAmount = 10000000L; /* 好市多 */


/*[組封包的功能結構]*/
ISO_FIELD_SVC_TABLE srSVC_FieldPack[] =
{
	{3,	inSVC_ISO_Pack03},
	{11,	inSVC_ISO_Pack11},
	{41,	inSVC_ISO_Pack41},
	{42,	inSVC_ISO_Pack42},
	{48,	inSVC_ISO_Pack48},
	{58,	inSVC_ISO_Pack58},
	{61,	inSVC_ISO_Pack61},
	{64,	inSVC_ISO_Pack64},
	{0,	NULL}, /* 最後一組一定要放【0】!! */
};

/*[解封包的功能結構]*/
ISO_FIELD_SVC_TABLE srSVC_FieldUnPack[] =
{
        {12,	inSVC_ISO_UnPack12},
        {13,	inSVC_ISO_UnPack13},
        {24,	inSVC_ISO_UnPack24},
        {37,	inSVC_ISO_UnPack37},
        {38,	inSVC_ISO_UnPack38},
        {39,	inSVC_ISO_UnPack39},
        {48,	inSVC_ISO_UnPack48},
        {58,	inSVC_ISO_UnPack58},
        {61,	inSVC_ISO_UnPack61},
        {0,	NULL}, /* 最後一組一定要放【0】!! */
};

/* [檢查封包的功能結構] */
ISO_CHECK_SVC_TABLE srSVC_FieldCheck[] =
{
        {41,	inSVC_ISO_Check41},
        {0,	NULL}, /* 最後一組一定要放【0】!! */
};

/* [ISO Field 型態結構] */
ISO_FIELD_TYPE_SVC_TABLE srSVC_DataElementsFormat[] =
{
	{2,	_SVC_ISO_NIBBLE_2_,	VS_FALSE,	0},
	{3,	_SVC_ISO_BCD_,		VS_FALSE,	_SVC_PCODE_SIZE_ * 2},
	{4, 	_SVC_ISO_BCD_,		VS_FALSE,	12},
	{11,	_SVC_ISO_BCD_,		VS_FALSE,	6},
	{12,	_SVC_ISO_BCD_,		VS_FALSE,	_TIME_SIZE_},
	{13,	_SVC_ISO_BCD_,		VS_FALSE,	4},
	{14,	_SVC_ISO_BCD_,		VS_FALSE,	4},
	{14,	_SVC_ISO_BCD_,		VS_FALSE,	4},
	{22,	_SVC_ISO_BCD_,		VS_FALSE,	4},
	{24,	_SVC_ISO_BCD_,		VS_FALSE,	4},
	{25,	_SVC_ISO_BCD_,		VS_FALSE,	2},
	{32,	_SVC_ISO_BYTE_1_,	VS_FALSE,	0},
	{35,	_SVC_ISO_NIBBLE_2_,	VS_FALSE,	0},
	{37,	_SVC_ISO_ASC_,		VS_FALSE,	_SVC_RRN_SIZE_},
	{38,	_SVC_ISO_ASC_,		VS_FALSE,	_AUTH_CODE_SIZE_},
	{39,	_SVC_ISO_ASC_,		VS_FALSE,	2},
	{41,	_SVC_ISO_ASC_,		VS_FALSE,	8},
	{42,	_SVC_ISO_ASC_,		VS_FALSE,	15},
	{48,	_SVC_ISO_BYTE_3_,	VS_FALSE,	 0},
	{52,	_SVC_ISO_BCD_,		VS_FALSE,	16},
	{54,	_SVC_ISO_BYTE_3_,	VS_FALSE,	 0},
	{55,	_SVC_ISO_BYTE_3_,	VS_FALSE,	0},
	{57,	_SVC_ISO_BYTE_3_,	VS_FALSE,	0},
	{58,	_SVC_ISO_BYTE_3_,	VS_FALSE,	 0},
	{59,	_SVC_ISO_BYTE_3_,	VS_FALSE,	 0},
	{60,	_SVC_ISO_BYTE_3_,	VS_FALSE,	 0},
	{61,	_SVC_ISO_BYTE_3_,	VS_FALSE,	 0},
	{62,	_SVC_ISO_BYTE_3_,	VS_FALSE,	 0},
	{63,	_SVC_ISO_BYTE_3_,	VS_FALSE,	0},
	{64,	_SVC_ISO_BCD_,		VS_FALSE,	16},
	{0,	_SVC_ISO_BCD_,		VS_FALSE,	0}, /* 最後一組一定要放【0】 !! */
};

/* [交易要使用的BIT MAP]  */
int inSVC_TMS[] = {3, 11, 41, 42, 48, 58, 0}; /* 最後一組一定要放 0!! */
int inSVC_TMS_FINISH[] = {3, 11, 41, 42, 48, 58, 0}; /* 最後一組一定要放 0!! */
int inSVC_REDEEM[] = {3, 11, 41, 42, 48, 61, 64, 0}; /* 最後一組一定要放 0!! */
int inSVC_RELOAD[] = {3, 11, 41, 42, 48, 64, 0}; /* 最後一組一定要放 0!! */
int inSVC_ACTIVECARD[] = {3, 11, 41, 42, 48, 64, 0}; /* 最後一組一定要放 0!! */
int inSVC_VOID[] = {3, 11, 41, 42, 48, 64, 0}; /* 最後一組一定要放 0!! */
int inSVC_REFUND[] = {3, 11, 41, 42, 48, 64, 0}; /* 最後一組一定要放 0!! */
int inSVC_SETTLE[] = {3, 41, 42, 48, 58, 64, 0}; /* 最後一組一定要放 0!! */
int inSVC_CLS_BATCH[] = {3, 41, 42, 48, 58, 64, 0}; /* 最後一組一定要放 0!! */
int inSVC_REVERSAL[] = {3, 11, 41, 42, 48, 64, 0}; /* 最後一組一定要放 0!! */
int inSVC_BATCHUPLOAD[] = {3, 11, 41, 42, 48, 64, 0}; /* 最後一組一定要放 0!! */
int inSVC_INQUIRY[] = {3, 11, 41, 42, 48, 64, 0}; /* 最後一組一定要放 0!! */
int inSVC_BACKCARD[] = {3, 11, 41, 42, 48, 61, 64, 0}; /* 最後一組一定要放 0!! */


BIT_MAP_SVC_TABLE srSVC_BitMap[] =
{
	{_SVC_TMS_,			inSVC_TMS,				"0300",	"930000"},
	{_SVC_TMS_FINISH_,		inSVC_TMS_FINISH,		"0300",	"930001"},
	{_SVC_REDEEM_,			inSVC_REDEEM,			"0200",	"888888"},
	{_SVC_RELOAD_,			inSVC_RELOAD,			"0200",	"888888"},
	{_SVC_ACTIVECARD_,		inSVC_ACTIVECARD,		"0200",	"888888"},
	{_SVC_VOID_,			inSVC_VOID,			"0200",	"888888"},
	{_SVC_REFUND_,			inSVC_REFUND,			"0200",	"888888"},
	{_SVC_SETTLE_,			inSVC_SETTLE,			"0500",	"920000"},
	{_SVC_CLS_BATCH_,		inSVC_CLS_BATCH,		"0500",	"960000"},
	{_SVC_REVERSAL_,		inSVC_REVERSAL,			"0400",	"888888"},
	{_SVC_BATCHUPLOAD_,		inSVC_BATCHUPLOAD,		"0320",	"900000"},
	{_SVC_INQUIRY_,			inSVC_INQUIRY,			"0200",	"888888"},
	{_SVC_BACKCARD_,		inSVC_BACKCARD,		"0200",	"888888"},
	{_TRANS_TYPE_NULL_,		NULL,				"0000",	"000000"}
};

/* [ISO流程架構] 用在組 ISO 流程時步驟會使用  */
ISO_TYPE_SVC_TABLE srSVC_ISOFunc[] =
{
        {
                srSVC_FieldPack,/* srPackISO */
                srSVC_FieldUnPack,/* srUnPackISO */
                srSVC_FieldCheck,/* srCheckISO */
                srSVC_DataElementsFormat, /* srISOFieldType */
                srSVC_BitMap,/* srBitMap */
                inSVC_ISO_GetBitMapCode,/* inGetBitMapCode */
                inSVC_ISO_PackMTI,/* inPackMTI */
                vdSVC_ISO_ModifyBitMap,/* vdModifyBitMap */
                vdSVC_ISO_ModifyPackData,/* vdModifyPackData */
                inSVC_ISO_CheckHeader,/* inCheckISOHeader */
                inSVC_ISO_OnlineAnalyse, /* inOnAnalyse */
                inSVC_ISO_AdviceAnalyse /* inAdviceAnalyse */
        },
};


/*
 * Function		: inSVC_ISO_Pack03
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * 組成 ISO電文欄位使用
 */
int inSVC_ISO_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" Field 3 InCode[%d] ", pobTran->srBRec.inCode);
	inDISP_LogPrintfWithFlag(" Field 3 1[%02x] 2[%02x] 3[%02x] ", szSVC_ISO_Field03[0], szSVC_ISO_Field03[1], szSVC_ISO_Field03[3]);
	
	memcpy(&uszPackBuf[inCnt], &szSVC_ISO_Field03[0], _SVC_PCODE_SIZE_);
	inCnt += _SVC_PCODE_SIZE_;

	inDISP_LogPrintfWithFlag(" Field 3 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
	
}


/*
 * Function		: inSVC_ISO_Pack11
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * 組成 ISO電文欄位使用
 */
int inSVC_ISO_Pack11(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;
	char szSTAN[6 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" lnSTANNum[%d] lnSTAN_Multi[%d] ",
			pobTran->srBRec.lnSTANNum, pobTran->srBRec.lnSTAN_Multi);
	
	memset(szSTAN, 0x00, sizeof (szSTAN));
	sprintf(szSTAN, "%06ld", pobTran->srBRec.lnSTANNum);

	inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], &szSTAN[0], 3);
	inCnt += 3;

	inDISP_LogPrintfWithFlag(" Field 11 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * Function		: inSVC_ISO_Pack41
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * 組成 ISO電文欄位使用
 */
int inSVC_ISO_Pack41(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;
	char szBuf[8 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memset(szBuf, 0x00, sizeof (szBuf));
	inGetTerminalID(szBuf);

	inDISP_LogPrintfWithFlag(" Field 41 TID[%s] ", szBuf);

	memcpy((char *) &uszPackBuf[inCnt], szBuf, strlen(szBuf));
	inCnt += strlen(szBuf);

	inDISP_LogPrintfWithFlag(" Field 41 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
	
}

/*
 * Function		: inSVC_ISO_Pack42
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * 組成 ISO電文欄位使用
 */
int inSVC_ISO_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;
	char szBuf[15 + 1];
	char szTemplate[16 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	memset(szTemplate, 0x00, sizeof (szTemplate));
	if (inGetMerchantID(szTemplate) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" FU Field42 GetMerchantId *Error* Line[%d]",   __LINE__);
		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag(" FU Field42 MID[%s]", szTemplate);

	memset(szBuf, 0x00, sizeof (szBuf));
	strcpy(szBuf, szTemplate);
	inFunc_PAD_ASCII(szBuf, szBuf, ' ', 15, _PAD_LEFT_FILL_RIGHT_);
	//pad(szBuf, szBuf, ' ', 15, LEFT);
	memcpy((char *) &uszPackBuf[inCnt], szBuf, strlen(szBuf));
	inCnt += strlen(szBuf);
	
	inDISP_LogPrintfWithFlag(" Field 42 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * Function		: inSVC_ISO_Pack48
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * 組成 ISO電文欄位使用
 */
int inSVC_ISO_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int	inCnt = 0;
	int	inTotalTagLen = 0;
	unsigned char uszField48[400];
	char	szASCII[4 + 1], szBCD[2 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memset(uszField48,0x00,sizeof(uszField48));

	switch (pobTran->srBRec.inCode)
	{
		case _SVC_TMS_:
			inSVC_PackTagFF21(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF29(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF64(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			break;
		case _SVC_REDEEM_:
		case _SVC_RELOAD_:
		case _SVC_REFUND_:
			inSVC_PackTagFF21(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF22(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF23(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF25(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF26(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF29(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF2B(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF2E(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF2F(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF31(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF45(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF65(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			break;
		case _SVC_ACTIVECARD_:
			inSVC_PackTagFF21(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF22(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF29(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF2E(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF2F(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF45(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF65(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			break;
		case _SVC_SETTLE_:
			inSVC_PackTagFF21(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF22(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF29(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF2E(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF2F(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF61(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF63(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			break;
		case _SVC_INQUIRY_:
			inSVC_PackTagFF21(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF22(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF23(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF25(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF26(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF29(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF2F(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF45(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF4B(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF65(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			break;
		case _SVC_BACKCARD_:
			inSVC_PackTagFF21(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF22(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF23(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF25(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF26(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF29(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF2E(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF2F(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF31(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF45(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			inSVC_PackTagFF65(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
			break;
		default:
			break;
	}

	if (pobTran->inISOTxnCode == _SVC_BATCHUPLOAD_)
	{
		inSVC_PackTagFF6E(pobTran, &uszField48[inTotalTagLen], &inTotalTagLen);
	}

       	/* Packet Data Length */
	memset(szASCII, 0x00, sizeof (szASCII));
	sprintf(szASCII, "%04d", inTotalTagLen);
	memset(szBCD, 0x00, sizeof (szBCD));
	inFunc_ASCII_to_BCD((unsigned char *) szBCD, szASCII, 2);
	memcpy((char *) &uszPackBuf[inCnt], &szBCD[0], 2);
	inCnt += 2;
	/* Packet Data */
	memcpy((char *) &uszPackBuf[inCnt], &uszField48[0], inTotalTagLen);
	inCnt += inTotalTagLen;

	inDISP_LogPrintfWithFlag(" Field 48 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
	
}

/*
 * Function		: inSVC_ISO_Pack58
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * 組成 ISO電文欄位使用
 */
int inSVC_ISO_Pack58(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int	inCnt = 0;
	int	inTotalTagLen = 0;
	unsigned char uszField58[400];
	char	szASCII[4 + 1], szBCD[2 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memset(uszField58,0x00,sizeof(uszField58));

	switch (pobTran->srBRec.inCode)
	{
		case _SVC_TMS_:
		case _SVC_RELOAD_:
		case _SVC_REFUND_:
		case _SVC_ACTIVECARD_:
		case _SVC_VOID_:
		case _SVC_SETTLE_:
		case _SVC_CLS_BATCH_:
			inSVC_PackTagDF66(pobTran, &uszField58[inTotalTagLen], &inTotalTagLen);
			break;
		default:
			break;
	}

       	/* Packet Data Length */
	memset(szASCII, 0x00, sizeof (szASCII));
	sprintf(szASCII, "%04d", inTotalTagLen);
	memset(szBCD, 0x00, sizeof (szBCD));
	inFunc_ASCII_to_BCD((unsigned char *) szBCD, szASCII, 2);
	memcpy((char *) &uszPackBuf[inCnt], &szBCD[0], 2);
	inCnt += 2;
	/* Packet Data */
	memcpy((char *) &uszPackBuf[inCnt], &uszField58[0], inTotalTagLen);
	inCnt += inTotalTagLen;

	inDISP_LogPrintfWithFlag(" Field 58 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * Function		: inSVC_ISO_Pack61
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * 組成 ISO電文欄位使用
 */
int inSVC_ISO_Pack61(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int	inCnt = 0;
	int	inTotalTagLen = 0;
	unsigned char uszField61[400];
	char	szASCII[4 + 1], szBCD[2 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memset(uszField61,0x00,sizeof(uszField61));

	switch (pobTran->srBRec.inCode)
	{
		case _SVC_REDEEM_:
		case _SVC_BACKCARD_:
			inSVC_PackTagFF3C(pobTran, &uszField61[inTotalTagLen], &inTotalTagLen);
			break;
		default:
			break;
	}

       	/* Packet Data Length */
	memset(szASCII, 0x00, sizeof (szASCII));
	sprintf(szASCII, "%04d", inTotalTagLen);
	memset(szBCD, 0x00, sizeof (szBCD));
	inFunc_ASCII_to_BCD((unsigned char *) szBCD, szASCII, 2);
	memcpy((char *) &uszPackBuf[inCnt], &szBCD[0], 2);
	inCnt += 2;
	/* Packet Data */
	memcpy((char *) &uszPackBuf[inCnt], &uszField61[0], inTotalTagLen);
	inCnt += inTotalTagLen;

	inDISP_LogPrintfWithFlag(" Field 61 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
	
}

/*
 * Function		: inSVC_ISO_Pack64
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * 組成 ISO電文欄位使用
 */
int inSVC_ISO_Pack64(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memcpy((char *) &uszPackBuf[inCnt], &pobTran->szMAC_HEX[0], 8);
	inCnt += 8;

	inDISP_LogPrintfWithFlag(" Field 64 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
	
}

/*
 * Function		: inSVC_ISO_UnPack12
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * 解析主機回覆 ISO電文欄位使用
 */
int inSVC_ISO_UnPack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	if (pobTran->inISOTxnCode == _SVC_REVERSAL_)
	{
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Reversal END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}
	else
		inFunc_BCD_to_ASCII(pobTran->srBRec.szTime, uszUnPackBuf, _TIME_SIZE_ / 2);

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * Function		: inSVC_ISO_UnPack13
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * 解析主機回覆 ISO電文欄位使用
 */
int inSVC_ISO_UnPack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	if (pobTran->inISOTxnCode == _SVC_REVERSAL_)
	{
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Reversal END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}
	else
	{
		memset(&pobTran->srBRec.szDate[4], 0x00, 4);
		inFunc_BCD_to_ASCII(&pobTran->srBRec.szDate[4], &uszUnPackBuf[0], 2);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
	
}

/*
 * Function		: inSVC_ISO_UnPack24
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * 解析主機回覆 ISO電文欄位使用
 */
int inSVC_ISO_UnPack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
	
}

/*
 * Function		: inSVC_ISO_UnPack37
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * 解析主機回覆 ISO電文欄位使用
 */
int inSVC_ISO_UnPack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	memcpy(&pobTran->srBRec.szRefNo[0], (char *) uszUnPackBuf, 12);
	
	inDISP_LogPrintfWithFlag(" UnPack37 szRefNo[%s]", pobTran->srBRec.szRefNo);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * Function		: inSVC_ISO_UnPack38
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * 解析主機回覆 ISO電文欄位使用
 */
int inSVC_ISO_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	if (pobTran->inISOTxnCode == _SVC_REVERSAL_)
	{
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Do Not UnPack38 END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}else
	{
		memcpy(&pobTran->srBRec.szAuthCode[0], (char *) uszUnPackBuf, _AUTH_CODE_SIZE_);
	}
	
	inDISP_LogPrintfWithFlag(" ISO UnPack38 ACode[%s]", pobTran->srBRec.szAuthCode);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
	
}

/*
 * Function		: inSVC_ISO_UnPack39
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * 解析主機回覆 ISO電文欄位使用
 */
int inSVC_ISO_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	memcpy(&pobTran->srBRec.szRespCode[0], (char *) &uszUnPackBuf[0], 2);
	pobTran->srBRec.szRespCode[2] = 0x00;
	
	inDISP_LogPrintfWithFlag(" UnPack39 RCode[%s]", pobTran->srBRec.szRespCode);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);	
}

/*
 * Function		: inSVC_ISO_UnPack48
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * 解析主機回覆 ISO電文欄位使用
 */
int inSVC_ISO_UnPack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	int	inCnt = 0;
	int	inTagCnt, inLength, inLengthCnt, i, inFieldLen = 0;
	int	inCardBcdLen = 0;
	char	szPrintBuf[_SVC_ISO_TEMPLATE_SIZE + 1], szTemplate[_SVC_ISO_TEMPLATE_SIZE + 1];
	char	szTag[20 + 1];
	char	szMaskAsc[20 + 1];
	char	szMask[10 + 1];
	char	szMaskResult[10 + 1];
	char	MaskCardNumber[10 + 1];
	char	CardLenAsc[2 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	inFieldLen = ((uszUnPackBuf[0] / 16 * 1000 + uszUnPackBuf[0] % 16) * 100);
	inFieldLen += (uszUnPackBuf[1] / 16 * 10) + uszUnPackBuf[1] % 16;

	inDISP_LogPrintfWithFlag("inFieldLen : [%d]", inFieldLen);

	inCnt = inCnt + 2;
	while (inFieldLen > inCnt)
	{
		/* 解析Tag */
		inTagCnt = 0;
		while (VS_TRUE)
		{
			inTagCnt++; /* 表示此Tag的長度 */

			if (inTagCnt == 1) /* 檢查第一個byte */
			{
				if ((uszUnPackBuf[inCnt] & 0x1F) == 0x1F) /* bit 1~5 表示下一個byte也是Tag */
				{
					inCnt++;
				}
				else
				{
					inCnt++;
					break;
				}
			}
			else /* 檢查第二個以後的byte */
			{
				if ((uszUnPackBuf[inCnt] & 0x80) == 0x80) /* bit 8 表示下一個byte也是Tag */
				{
					inCnt++;
				}
				else
				{
					inCnt++;
					break;
				}
			}
		}

		memset(szTag, 0x00, sizeof(szTag));
		inFunc_BCD_to_ASCII(szTag, &uszUnPackBuf[inCnt - inTagCnt], inTagCnt);

		/* 解析Length */
		inLength = 0;
		inLengthCnt = 0;
		if ((uszUnPackBuf[inCnt] & 0x80) == 0x80) /* bit 8 表示後續幾個byte來表示長度 */
		{
			i = 0;
			inLengthCnt = uszUnPackBuf[inCnt] & 0x7F; /* 表示後續要幾個byte來表示長度 */
			while (i < inLengthCnt)
			{
				inLength += uszUnPackBuf[inCnt + inLengthCnt - i] << (8 * i); /* 真正長度 */
				i++;
			}
		}
		else /* bit 8 表示長度只有一個byte 01~7F */
		{
			inLength = uszUnPackBuf[inCnt]; /* 真正的長度 */
		}

		inCnt = inCnt + 1 + inLengthCnt;

		if (memcmp(szTag, "FF23", 4) == 0)//Issuer ID
		{
			memcpy(pobTran->srBRec.szSvcIssueId, &uszUnPackBuf[inCnt], inLength);
		}
		else if (memcmp(szTag, "FF25", 4) == 0)//Card Number
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszUnPackBuf[inCnt], inLength);

			memset(CardLenAsc, 0x00, sizeof(CardLenAsc));
			inFunc_BCD_to_ASCII(CardLenAsc, (unsigned char *)&szTemplate[0], 1);

			if (atoi(CardLenAsc) % 2 == 0)
				inCardBcdLen = atoi(CardLenAsc) / 2;
			else
				inCardBcdLen = (atoi(CardLenAsc) / 2) + 1;

			memcpy(MaskCardNumber, &szTemplate[1] ,inCardBcdLen);
			memset(szMaskAsc, 0x00, sizeof(szMaskAsc));
			inGetSVCMaskData(szMaskAsc);
			memset(szMask, 0x00, sizeof(szMask));
			inFunc_ASCII_to_BCD( (unsigned char*)szMask, szMaskAsc, 10);

			for ( i = 0 ;i < 10 ; i++)
			{
				szMaskResult[i] = MaskCardNumber[i] ^ szMask[i + (10 - inCardBcdLen)];
			}

			memset(pobTran->srBRec.szSvcCardNumber, 0xFF, 12);
			memcpy(pobTran->srBRec.szSvcCardNumber, szMaskResult, inCardBcdLen);
		}
		else if (memcmp(szTag, "FF26", 4) == 0)//Card Expire day
		{
			memcpy(pobTran->srBRec.szSvcExpireDay,&uszUnPackBuf[inCnt],inLength);
		}
		else if (memcmp(szTag, "DF54", 4) == 0)//Remain Transaction Amount
		{
			memset(szTemplate,0x00,sizeof(szTemplate));
			memcpy(szTemplate,&uszUnPackBuf[inCnt],inLength);

			memset(szPrintBuf,0x00,sizeof(szPrintBuf));
			inFunc_BCD_to_ASCII(szPrintBuf, (unsigned char*)szTemplate, 5);
			inDISP_LogPrintfWithFlag("inSVC_UnPack48_lnSvcRedeemAmount(%s)", szPrintBuf);
			pobTran->srBRec.lnSvcRedeemAmount = atol(szPrintBuf);
			inDISP_LogPrintfWithFlag("inSVC_UnPack48_lnSvcRedeemAmount(%ld)", pobTran->srBRec.lnSvcRedeemAmount);
			pobTran->srBRec.lnSvcRedeemAmount = pobTran->srBRec.lnSvcRedeemAmount/100;
			inDISP_LogPrintfWithFlag("inSVC_UnPack48lnSvcRedeemAmount100(%ld)", pobTran->srBRec.lnSvcRedeemAmount);
		}
		else if (memcmp(szTag, "FF2A", 4) == 0) /* Time Date */
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszUnPackBuf[inCnt], inLength);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_BCD_to_ASCII(szPrintBuf,(unsigned char *) szTemplate, 7);

			inDISP_LogPrintfWithFlag("inSVC_UnPack48_FF2A(%s)", szPrintBuf);
			memcpy(&pobTran->srBRec.szTime[0], &szPrintBuf[8], 6);
			inDISP_LogPrintfWithFlag("inSVC_UnPack48_Time(%s)", pobTran->srBRec.szTime);
		}

		inCnt += inLength;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * Function		: inSVC_ISO_UnPack58
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * 解析主機回覆 ISO電文欄位使用
 */
int inSVC_ISO_UnPack58(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	int	inCnt = 0;
	int	inTagCnt, inLength, inLengthCnt, i, inDF66Len = 0, inFieldLen = 0;
	char	szPrintBuf[_SVC_ISO_TEMPLATE_SIZE + 1], szTemplate[_SVC_ISO_TEMPLATE_SIZE + 1];
	char	szDF66Data[512];
	char	szTag[20 + 1];
	char	szBatchNum[6 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	inFieldLen = ((uszUnPackBuf[0] / 16 * 1000 + uszUnPackBuf[0] % 16) * 100);
	inFieldLen += (uszUnPackBuf[1] / 16 * 10) + uszUnPackBuf[1] % 16;

	inDISP_LogPrintfWithFlag(" Unpack58 inFieldLen : [%d]", inFieldLen);

	inCnt = inCnt + 2;
	while (inFieldLen > inCnt)
	{
		/* 解析Tag */
		inTagCnt = 0;
		while (VS_TRUE)
		{
			inTagCnt++; /* 表示此Tag的長度 */

			if (inTagCnt == 1) /* 檢查第一個byte */
			{
				if ((uszUnPackBuf[inCnt] & 0x1F) == 0x1F) /* bit 1~5 表示下一個byte也是Tag */
				{
					inCnt++;
				}
				else
				{
					inCnt++;
					break;
				}
			}
			else /* 檢查第二個以後的byte */
			{
				if ((uszUnPackBuf[inCnt] & 0x80) == 0x80) /* bit 8 表示下一個byte也是Tag */
				{
					inCnt++;
				}
				else
				{
					inCnt++;
					break;
				}
			}
		}
		memset(szTag, 0x00, sizeof(szTag));
		inFunc_BCD_to_ASCII(szTag, &uszUnPackBuf[inCnt - inTagCnt], inTagCnt);

		/* 解析Length */
		inLength = 0;
		inLengthCnt = 0;
		if ((uszUnPackBuf[inCnt] & 0x80) == 0x80) /* bit 8 表示後續幾個byte來表示長度 */
		{
			i = 0;
			inLengthCnt = uszUnPackBuf[inCnt] & 0x7F; /* 表示後續要幾個byte來表示長度 */
			while (i < inLengthCnt)
			{
				inLength += uszUnPackBuf[inCnt + inLengthCnt - i] << (8 * i); /* 真正長度 */
				i++;
			}
		}
		else /* bit 8 表示長度只有一個byte 01~7F */
		{
			inLength = uszUnPackBuf[inCnt]; /* 真正的長度 */
		}

		inCnt = inCnt + 1 + inLengthCnt;

		if (memcmp(szTag, "DF66", 4) == 0)
		{
			inDISP_LogPrintfWithFlag("Get DF66 ");
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_BCD_to_ASCII( szTemplate, &uszUnPackBuf[inCnt], 3);
			inSetSVCParameterVersion(szTemplate);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_BCD_to_ASCII( szTemplate, &uszUnPackBuf[inCnt + 3], 7);
			inSetSVCParameterDateTime(szTemplate);

			inDF66Len = inLength - 10;
			memset(szDF66Data, 0x00, sizeof(szDF66Data));
			memcpy(szDF66Data, &uszUnPackBuf[inCnt + 10], inDF66Len);
			
			inSaveSvcTableRec(0);
		}

		inCnt += inLength;
	}

	inCnt = 0; //重新歸零來處理DF66
	while (inDF66Len > inCnt)
	{
		/* 解析Tag */
		inTagCnt = 0;
		while (VS_TRUE)
		{
			inTagCnt++; /* 表示此Tag的長度 */

			if (inTagCnt == 1) /* 檢查第一個byte */
			{
				if ((szDF66Data[inCnt] & 0x1F) == 0x1F) /* bit 1~5 表示下一個byte也是Tag */
				{
					inCnt++;
				}
				else
				{
					inCnt++;
					break;
				}
			}
			else /* 檢查第二個以後的byte */
			{
				if ((szDF66Data[inCnt] & 0x80) == 0x80) /* bit 8 表示下一個byte也是Tag */
				{
					inCnt++;
				}
				else
				{
					inCnt++;
					break;
				}
			}
		}
		memset(szTag, 0x00, sizeof(szTag));
		inFunc_BCD_to_ASCII( szTag, (unsigned char *)&szDF66Data[inCnt - inTagCnt], inTagCnt);

		/* 解析Length */
		inLength = 0;
		inLengthCnt = 0;
		if ((szDF66Data[inCnt] & 0x80) == 0x80) /* bit 8 表示後續幾個byte來表示長度 */
		{
			i = 0;
			inLengthCnt = szDF66Data[inCnt] & 0x7F; /* 表示後續要幾個byte來表示長度 */
			while (i < inLengthCnt)
			{
				inLength += szDF66Data[inCnt + inLengthCnt - i] << (8 * i); /* 真正長度 */
				i++;
			}
		}
		else /* bit 8 表示長度只有一個byte 01~7F */
		{
			inLength = szDF66Data[inCnt]; /* 真正的長度 */
		}

		inCnt = inCnt + 1 + inLengthCnt;

		/* Table Data */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memcpy(szTemplate, &szDF66Data[inCnt], inLength);

		if (memcmp(szTag, "08", 2) == 0)
		{
			inDISP_LogPrintfWithFlag("MaskLen : [%d]", inLength);
			/* 亂碼化使用 */
			inSetSVCMaskData(szTemplate);
		}
		else if (memcmp(szTag, "E1", 2) == 0)
		{
			memset(szBatchNum,0x00,sizeof(szBatchNum));
			memcpy(szBatchNum,szTemplate,inLength);
			if (inSetBatchNum(szBatchNum) == VS_ERROR)
			{
				inDISP_LogPrintfWithFlag("  inSetBatchNum Error ");
				return (VS_ERROR);
			}
		}
		inCnt += inLength;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * Function		: inSVC_ISO_UnPack61
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * 解析主機回覆 ISO電文欄位使用
 */
int inSVC_ISO_UnPack61(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	int	inCnt = 0;
	int	inTagCnt, inLength, inLengthCnt, i, inFieldLen = 0;
	char	szFF3EData[22+1];
	char	szTag[20 + 1];

        	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	inFieldLen = ((uszUnPackBuf[0] / 16 * 1000 + uszUnPackBuf[0] % 16) * 100);
	inFieldLen += (uszUnPackBuf[1] / 16 * 10) + uszUnPackBuf[1] % 16;

	inDISP_LogPrintfWithFlag("inFieldLen : [%d]", inFieldLen);

	inCnt = inCnt + 2;
	while (inFieldLen > inCnt)
	{
		/* 解析Tag */
		inTagCnt = 0;
		while (VS_TRUE)
		{
			inTagCnt++; /* 表示此Tag的長度 */

			if (inTagCnt == 1) /* 檢查第一個byte */
			{
				if ((uszUnPackBuf[inCnt] & 0x1F) == 0x1F) /* bit 1~5 表示下一個byte也是Tag */
				{
					inCnt++;
				}
				else
				{
					inCnt++;
					break;
				}
			}
			else /* 檢查第二個以後的byte */
			{
				if ((uszUnPackBuf[inCnt] & 0x80) == 0x80) /* bit 8 表示下一個byte也是Tag */
				{
					inCnt++;
				}
				else
				{
					inCnt++;
					break;
				}
			}
		}

		memset(szTag, 0x00, sizeof(szTag));
		inFunc_BCD_to_ASCII(szTag, (unsigned char *)&uszUnPackBuf[inCnt - inTagCnt], inTagCnt);

		/* 解析Length */
		inLength = 0;
		inLengthCnt = 0;
		if ((uszUnPackBuf[inCnt] & 0x80) == 0x80) /* bit 8 表示後續幾個byte來表示長度 */
		{
			i = 0;
			inLengthCnt = uszUnPackBuf[inCnt] & 0x7F; /* 表示後續要幾個byte來表示長度 */
			while (i < inLengthCnt)
			{
				inLength += uszUnPackBuf[inCnt + inLengthCnt - i] << (8 * i); /* 真正長度 */
				i++;
			}
		}
		else /* bit 8 表示長度只有一個byte 01~7F */
		{
			inLength = uszUnPackBuf[inCnt]; /* 真正的長度 */
		}

		inCnt = inCnt + 1 + inLengthCnt;

		if (memcmp(szTag, "FF3E", 4) == 0)
		{
			inDISP_LogPrintfWithFlag("Get FF3E ");
			memset(szFF3EData, 0x00, sizeof(szFF3EData));
			memcpy(szFF3EData, &uszUnPackBuf[inCnt], inLength);

			memcpy(pobTran->srBRec.szSvcPointSign, &szFF3EData[0], 2);
			inDISP_LogPrintfWithFlag("PointSign %02X%02X",pobTran->srBRec.szSvcPointSign[0], pobTran->srBRec.szSvcPointSign[1]);

			memcpy(pobTran->srBRec.szSvcPoint1, &szFF3EData[2], 5);
			inDISP_LogPrintfWithFlag("szSvcPoint1 %02X%02X", pobTran->srBRec.szSvcPoint1[0], pobTran->srBRec.szSvcPoint1[1]);
			memcpy(pobTran->srBRec.szSvcPoint2, &szFF3EData[7], 5);
			memcpy(pobTran->srBRec.szSvcPoint3, &szFF3EData[12], 5);
			memcpy(pobTran->srBRec.szSvcPoint4, &szFF3EData[17], 5);
		}
		else if (memcmp(szTag, "FF41", 4) == 0)
		{

		}

		inCnt += inLength;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);	
}

/*
 * Function		: inSVC_ISO_Check41
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * 檢查主機回覆 ISO電文，如不符檢查內容，需停止交易。
 * exist
 */
int inSVC_ISO_Check41(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
	return VS_SUCCESS;
}

/*
 * Function		: inSVC_ISO_GetFieldLen
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * 會使用選用的 [ISO Field 型態結構]，使用Loop執行，比對 inFieldNum 等於傳入的 inField後
 * 再利用 inFieldType 的格式組出相對應的欄位長度 。
 */
int inSVC_ISO_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_SVC_TABLE *srFieldType)
{
	int inCnt = 0, i, inLen;

	for (i = 0;; i++)
	{
		if (srFieldType[i].inFieldNum == 0)
			break;

		if (srFieldType[i].inFieldNum != inField)
			continue;

		switch (srFieldType[i].inFieldType)
		{
			case _SVC_ISO_ASC_:
				inCnt += srFieldType[i].inFieldLen;
				break;
			case _SVC_ISO_BCD_:
				inCnt += srFieldType[i].inFieldLen / 2;
				break;
			case _SVC_ISO_NIBBLE_2_:
				inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
				inCnt += ((inLen + 1) / 2) + 1;
				break;
			case _SVC_ISO_NIBBLE_3_:
				inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
				inCnt += ((inLen + 1) / 2) + 2;
				break;
			case _SVC_ISO_BYTE_2_:
				inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
				inCnt += inLen + 1;
				break;
			case _SVC_ISO_BYTE_3_:
				inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
				inCnt += inLen + 2;
				break;
			case _SVC_ISO_BYTE_2_H_:
				inLen = (int) uszSendData[0];
				inCnt += inLen + 1;
				break;
			case _SVC_ISO_BYTE_3_H_:
				inLen = ((int) uszSendData[0] * 0xFF) + (int) uszSendData[1];
				inCnt += inLen + 1;
				break;
			default:
				break;
		}

		break;
	}

	return (inCnt);
}

/*
 * Function		: inSVC_ISO_GetCheckField
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * 利用 [檢查封包的功能結構] 定義的項目及函式，檢查傳入的 inField 是否與定義的 inFieldNum 有相同。
 * 如符合條件，則回傳比對結構項目的位置 [i]，如不符合條件則回傳 [VS_ERROR]
 */
int inSVC_ISO_GetCheckField(int inField, ISO_CHECK_SVC_TABLE *srChkISO)
{
	int i;

//	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);	
//	inDISP_LogPrintfWithFlag(" GetCheck Search inField(%d)", inField);

	for (i = 0; i < 64; i++)
	{
		if (srChkISO[i].inFieldNum == 0)
			break;
		else if (srChkISO[i].inFieldNum == inField)
		{
//			inDISP_LogPrintfWithFlag(" SVC_ISO GetCheckField ReturnVal[%d] ", i);			
//			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END-----", __FILE__, __FUNCTION__, __LINE__);
			return (i);
		}
	}

//	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] *Warning* END-----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_ERROR);
}

/*
 * Function		: inSVC_ISO_CheckUnPackField
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * 檢查指定的欄位是否有符合要執行定義的解析封包欄位。
 * 符合，回傳 VS_SUCCESS
 * 不符合，回傳 VS_ERROR
 */
int inSVC_ISO_CheckUnPackField(int inField, ISO_FIELD_SVC_TABLE *srCheckUnPackField)
{
	int i;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" Unpack Search inField(%d)", inField);

	for (i = 0; i < 64; i++)
	{
		if (srCheckUnPackField[i].inFieldNum == 0)
			break;
		else if (srCheckUnPackField[i].inFieldNum == inField)
		{
			inDISP_LogPrintfWithFlag(" SVC_ISO CheckUnPackFiled ReturnVal[%d] ", i);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
			return (VS_SUCCESS);
		}
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] *Error* END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_ERROR);
}


/*
 * Function		: inSVC_BitMapCheck
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * 檢查指定的欄位是否有開啟。
 * Input		:
 * inBitMap: 傳入要檢查的BibMap陣列
 * inFeild : 傳入要比對的欄位
 * 
 */
int inSVC_BitMapCheck(unsigned char *inBitMap, int inFeild)
{
	int inByteIndex, inBitIndex;

	inFeild--;
	inByteIndex = inFeild / 8;
	inBitIndex = 7 - (inFeild - inByteIndex * 8);

	if (_SVC_CHECK_BIT_MAP_(inBitMap[inByteIndex], inBitIndex))
	{
		return (VS_TRUE);
	}

	return (VS_FALSE);
}



/*
 * Function		: inSVC_ISO_PackISO
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * [組ISO電文流程] 控制 Pack ISO 流程，依照 [ISO流程架構] 執行對應的功能
 */
int inSVC_ISO_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode)
{
	int i, inSendLen, inField, inCnt;
	int inBitMap[_SVC_MAX_BIT_MAP_CNT_];
	int inRetVal, inISOFuncIndex = -1;
	unsigned char bBuf[30];
	ISO_TYPE_SVC_TABLE srISOFunc;
	char szCommmode[1 + 1] = {0}, szEncryptMode[1 + 1] = {0};

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	inGetCommMode(szCommmode);
	inGetEncryptMode(szEncryptMode);
	
	inDISP_LogPrintfWithFlag(" SVC_ISO Pack COMM_MODE[%02x] ENCRYPT_MODE[%02x]",
			szCommmode[0], szEncryptMode[0]);

	/* 目前只有一個結構，所以設定為 0  */
	inISOFuncIndex = 0; 

	inSendLen = 0;
	inField = 0;
	/* 決定要執行第幾個 Function Index */
	memset((char *) &srISOFunc, 0x00, sizeof (srISOFunc));
	memcpy((char *) &srISOFunc, (char *) &srSVC_ISOFunc[inISOFuncIndex], sizeof (srISOFunc));
	memset((char *) inBitMap, 0x00, sizeof (inBitMap));

	/* 開始組 ISO 電文 */
	/* Pack TPDU */
	memset((char *) bBuf, 0x00, sizeof (bBuf));
	inGetTPDU((char *) bBuf);
	inFunc_ASCII_to_BCD(&uszSendBuf[inSendLen], (char *) bBuf, _SVC_TPDU_SIZE_); /* 5 BYTE */
	inSendLen += _SVC_TPDU_SIZE_;

	inRetVal = inSVC_ISO_GetBitMap(pobTran, inTxnCode, &srISOFunc, inBitMap, &uszSendBuf[inSendLen]);
	if (inRetVal <= VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" SVC_ISO GetBitMap[%d] *Error* ", inTxnCode);
		return (VS_ERROR);
	}
	else
	{
		inSendLen += inRetVal; /* Bit Map 長度 */
	}

	for (i = 0;; i++)
	{
		if (inBitMap[i] == 0)
			break;

		while (inBitMap[i] > srISOFunc.srPackISO[inField].inFieldNum)
		{
			inField++;
		}

		if (inBitMap[i] == srISOFunc.srPackISO[inField].inFieldNum)
		{
			/*  */
			if ( srISOFunc.srPackISO[inField].inFieldNum == 64)
			{
				inSVC_CalMac(pobTran, &uszSendBuf[5], (inSendLen - 5));
			}
			
			inCnt = srISOFunc.srPackISO[inField].inISOLoad(pobTran, &uszSendBuf[inSendLen]);
			if (inCnt <= 0)
			{
				inDISP_DispLogAndWriteFlie(" SVC_ISO FieldNum[%d] Field[%d] *Error* ", srISOFunc.srPackISO[inField].inFieldNum, inField);
				return (VS_ERROR);
			} else
				inSendLen += inCnt;
		}
	}

	if (srISOFunc.vdModifyPackData != NULL)
		srISOFunc.vdModifyPackData(pobTran, uszSendBuf, &inSendLen);

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inSendLen);
}

/*
 * Function		: inSVC_ISO_UnPackISO
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * [解主機回覆ISO電文流程] 控制 UnPack ISO 流程，依照 [ISO流程架構] 執行對應的功能
 */
int inSVC_ISO_UnPackISO(TRANSACTION_OBJECT *pobTran,
		unsigned char *uszSendBuf,
		int inSendLen,
		unsigned char *uszReceBuf,
		int inReceLen)
{
	int i, inReceCnt, inCnt, inSendCnt, inSendField, inReceField;
	char szBuf[_SVC_TPDU_SIZE_ + _SVC_MTI_SIZE_ + _SVC_BIT_MAP_SIZE_ + 1];
	unsigned char uszSendMap[_SVC_BIT_MAP_SIZE_ + 1], uszReceMap[_SVC_BIT_MAP_SIZE_ + 1];
	ISO_TYPE_SVC_TABLE srISOFunc;
	char szEncryptMode[1 + 1] = {0};
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	inDISP_LogPrintfWithFlag(" Receive_Data_Len[%d] inISOTxnCode[%d]", inReceLen, pobTran->inISOTxnCode);
	
	inSendField = inReceField = 0;
	inSendCnt = inReceCnt = 0;
	memset((char *) &srISOFunc, 0x00, sizeof (srISOFunc));
	memset((char *) uszSendMap, 0x00, sizeof (uszSendMap));
	memset((char *) uszReceMap, 0x00, sizeof (uszReceMap));
	memset((char *) szBuf, 0x00, sizeof (szBuf));
	
	inGetEncryptMode(szEncryptMode);
	inDISP_LogPrintfWithFlag(" szEncryptMode[%s] ", szEncryptMode);
	
	/* 拷貝對應的 [ISO流程架構] 來使用 */
	/* 目前只有一個結構，所以設定為 0  */
	memcpy((char *) &srISOFunc, (char *) &srSVC_ISOFunc[0], sizeof (srISOFunc));
	//memcpy((char *) &srISOFunc, (char *) &srSVC_ISOFunc[szEncryptMode[0] - '0'], sizeof (srISOFunc));

	if (srISOFunc.inCheckISOHeader != NULL)
	{
		if (srISOFunc.inCheckISOHeader(pobTran, (char *) &uszSendBuf[inReceCnt], (char *) &uszReceBuf[inReceCnt]) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" SVC_CheckIsoHeader *Error* Line[%d]", __LINE__);
			return (VS_ISO_UNPACK_ERROR);
		}
	}

	inSendCnt += _SVC_TPDU_SIZE_;
	inReceCnt += _SVC_TPDU_SIZE_;
	inSendCnt += _SVC_MTI_SIZE_;
	inReceCnt += _SVC_MTI_SIZE_;

	memcpy((char *) uszSendMap, (char *) &uszSendBuf[inSendCnt], _SVC_BIT_MAP_SIZE_);
	memcpy((char *) uszReceMap, (char *) &uszReceBuf[inReceCnt], _SVC_BIT_MAP_SIZE_);

	inSendCnt += _SVC_BIT_MAP_SIZE_;
	inReceCnt += _SVC_BIT_MAP_SIZE_;


	if (!inSVC_BitMapCheck(uszReceMap, 39))
	{
		inDISP_DispLogAndWriteFlie(" SVC_CheckIsoHeader Check 39 *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	if (!inSVC_BitMapCheck(uszReceMap, 41))
	{
		inDISP_DispLogAndWriteFlie(" SVC_CheckIsoHeader Check 41 *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	
	for (i = 1; i <= 64; i++)
	{	
		/* 如傳送的BIT有 ON 但接收的BIT沒 ON 就不需接收資料比對及解析，但要增加傳送資料的長度 */
		if (inSVC_BitMapCheck(uszSendMap, i) && !inSVC_BitMapCheck(uszReceMap, i))
		{
			inSendCnt += inSVC_ISO_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
		} else if (inSVC_BitMapCheck(uszReceMap, i))
		{
			/* 檢查傳送主機的BitMap有沒有設定 */
			if (inSVC_BitMapCheck(uszSendMap, i))
			{	/* 比對是否要進行欄位檢查 */
				if ((inSendField = inSVC_ISO_GetCheckField(i, srISOFunc.srCheckISO)) != VS_ERROR)
				{
					/* 執行接收資料與傳送資料進行比對 */
					if (srISOFunc.srCheckISO[inSendField].inISOCheck(pobTran, &uszSendBuf[inSendCnt], &uszReceBuf[inReceCnt]) != VS_SUCCESS)
					{
						inDISP_DispLogAndWriteFlie(" SVC_ISOCHECK *Error* SendId[%d] FieldNum[%d]  ", inSendField, srISOFunc.srCheckISO[inSendField].inFieldNum);
						return (VS_ISO_UNPACK_ERROR);
					}
				}

				inSendCnt += inSVC_ISO_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
			}

			while (i > srISOFunc.srUnPackISO[inReceField].inFieldNum)
			{
				inReceField++;
			}

			if (i == srISOFunc.srUnPackISO[inReceField].inFieldNum)
			{
				if (inSVC_ISO_CheckUnPackField(i, srISOFunc.srUnPackISO) == VS_SUCCESS)
				{
					if (srISOFunc.srUnPackISO[inReceField].inISOLoad(pobTran, &uszReceBuf[inReceCnt]) != VS_SUCCESS)
					{
						inDISP_DispLogAndWriteFlie(" SVC_UnPackISO Rece_id[%d] *Error* ", inReceField);
						return (VS_ERROR);
					}
				}
			}

			inCnt = inSVC_ISO_GetFieldLen(i, &uszReceBuf[inReceCnt], srISOFunc.srISOFieldType);
			if (inCnt == VS_ERROR)
			{
				inDISP_DispLogAndWriteFlie(" SVC_ISO Get Field Len i[%d] *Error", i);
				return (VS_ISO_UNPACK_ERROR);
			}

			inReceCnt += inCnt;
		}
	}

	
#if 0// TODO: 這個檢核看要不要補回  2022/12/26 
	/* 如果欄位39回 "00" 就進行客制的檢查項目 */
	if (!memcmp(&pobTran->srBRec.szRespCode[0], "00", 2))
	{
		switch (pobTran->inISOTxnCode)
		{
			case _CUP_SALE_:
			case _CUP_PRE_AUTH_:
				if (!inSVC_BitMapCheck(uszReceMap, 04))
				{
					inDISP_DispLogAndWriteFlie(" SVC_BitMpCheck inISOTxnCode[%d] 04 *Eerror* Line[%d]", pobTran->inISOTxnCode, __LINE__);
					return (VS_ERROR);
				}

				if (!inSVC_BitMapCheck(uszReceMap, 38))
				{
					inDISP_DispLogAndWriteFlie(" SVC_BitMpCheck inISOTxnCode[%d] 38 *Eerror* Line[%d]", pobTran->inISOTxnCode, __LINE__);
					return (VS_ERROR);
				}
				break;
			default:
				inDISP_DispLogAndWriteFlie(" SVC_UnPackISO inISOTxnCode[%d] *Eerror* Line[%d]", pobTran->inISOTxnCode, __LINE__);
				break;
		} /* End switch() */
	}
#endif
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * Function		: inSVC_ISO_PackSendUnPackReceData
 * Date&Time	: 2022/9/14 下午 6:41
 * Describe		: 
 * 執行 ISO 電文的組、解、收、送功能。
 */
int inSVC_ISO_PackSendUnPackReceData(TRANSACTION_OBJECT *pobTran, int inTxnCode)
{
	int inSendLen = 0, inReceLen = 0, inRetVal;
	char szSendPacket[_SVC_ISO_SEND_ + 1], szRecePacket[_SVC_ISO_RECV_ + 1];

	memset(szSendPacket, 0x00, sizeof (szSendPacket));
	memset(szRecePacket, 0x00, sizeof (szRecePacket));

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag("  SVC_inTxnCode [%d]", inTxnCode);

	if ((inSendLen = inSVC_ISO_PackISO(pobTran, (unsigned char *) szSendPacket, inTxnCode)) <= 0)
	{
		inDISP_Msg_BMP(_ERR_ISO_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		return (VS_ISO_PACK_ERR);
	}

	if (pobTran->inISOTxnCode == _SETTLE_ ||
		pobTran->inTransactionCode == _CLS_SETTLE_)
	{
		inSetMustSettleBit("Y");
		/* 開啟 TmsFPT 下載TMS要結帳的參數 */
		inEDCTMS_FUNC_SetTmsFtpSettleFlagOn();
	}

	if ((inReceLen = inSVC_ISO_SendReceData(pobTran,
			(unsigned char *) szSendPacket,
			inSendLen,
			(unsigned char *) szRecePacket)) != VS_SUCCESS)
	{
		if (pobTran->inISOTxnCode != _TC_UPLOAD_ || pobTran->inISOTxnCode != _ADVICE_)
			memset(pobTran->srBRec.szRespCode, 0x00, sizeof (pobTran->srBRec.szRespCode));
		pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_RECV_;
		inDISP_LogPrintfWithFlag(" SVC_SendRece *Error* ReceLen[%d]", inReceLen);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Iso Send *Error* END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	inRetVal = inSVC_ISO_UnPackISO(pobTran,
			(unsigned char *) szSendPacket,
			inSendLen,
			(unsigned char *) szRecePacket,
			inReceLen);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_LogPrintfWithFlag(" SVC_UnPack *Error* inRetVal[%d]", inRetVal);
		inDISP_Msg_BMP(_ERR_ISO_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		inRetVal = VS_ISO_UNPACK_ERROR;
	}

	inDISP_LogPrintfWithFlag(" Return  inRetVal[%d] ", inRetVal);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
	
}



int inSVC_ISO_GetBMapMTIPCode(TRANSACTION_OBJECT *pobTran, ISO_TYPE_SVC_TABLE *srISOFuncIndex, int inBitMapCode)
{
	int inBitMapIndex;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" Input inBitMapCode[%d] ", inBitMapCode);
	
	for (inBitMapIndex = 0;; inBitMapIndex++)
	{
		if (srISOFuncIndex->srBitMap[inBitMapIndex].inTxnID == _TRANS_TYPE_NULL_)
		{
			inDISP_LogPrintfWithFlag(" Compare inBitMapCode *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		}

		if (srISOFuncIndex->srBitMap[inBitMapIndex].inTxnID == inBitMapCode)
			break;
	}

	inDISP_LogPrintfWithFlag(" Return inBitMapIndex[%d]", inBitMapIndex);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inBitMapIndex);
}


void vdSVC_ISO_CopyISOMap(int *inBitMap, int *inSourceBitMap)
{
	int i;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	for (i = 0; i < _SVC_MAX_BIT_MAP_CNT_; i++)
	{
		if (inSourceBitMap[i] == 0)
			break;
		else
		{
			inBitMap[i] = inSourceBitMap[i];
		}
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
}

/*
 * App Name		: vdSVC_ISO_MapSet
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 1:46
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
void vdSVC_ISO_MapSet(int *inBitMap, int inFeild)
{
	int i, inBMapCnt, inBitMapCnt;
	int inBMap[_SVC_MAX_BIT_MAP_CNT_];

	memset((char *) inBMap, 0x00, sizeof (inBMap));
	inBMapCnt = inBitMapCnt = 0;

	for (i = 0; i < 64; i++)
	{
		if (inBitMap[inBitMapCnt] > inFeild)
		{
			if (i == 0)
			{
				inBMap[inBMapCnt++] = inFeild;
				break;
			}
		} else if (inBitMap[inBitMapCnt] < inFeild)
		{
			inBMap[inBMapCnt++] = inBitMap[inBitMapCnt++];
			if (inBitMap[inBitMapCnt] == 0 || inBitMap[inBitMapCnt] > inFeild)
			{
				inBMap[inBMapCnt++] = inFeild;
				break;
			}
		} else /* if (*inBitMap == inFeild) */
			break;
	}

	for (i = 0;; i++)
	{
		if (inBitMap[inBitMapCnt] != 0)
			inBMap[inBMapCnt++] = inBitMap[inBitMapCnt++];
		else
			break;
	}

	memcpy((char *) inBitMap, (char *) inBMap, sizeof (inBMap));
}

/*
 * App Name		: vdSVC_ISO_MapReset
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 1:48
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
void vdSVC_ISO_MapReset(int *inBitMap, int inFeild)
{
	int i, inBMapCnt;
	int inBMap[_SVC_MAX_BIT_MAP_CNT_];

	memset((char *) inBMap, 0x00, sizeof (inBMap));
	inBMapCnt = 0;

	for (i = 0; i < 64; i++)
	{
		if (inBitMap[i] == 0)
			break;
		else if (inBitMap[i] != inFeild)
			inBMap[inBMapCnt++] = inBitMap[i];
	}

	memcpy((char *) inBitMap, (char *) inBMap, sizeof (inBMap));
}

/*
 * App Name		: inSVC_ISO_MapTest
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 1:47
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inSVC_ISO_MapTest(int *inBitMap, int inFeild)
{
	int i;

	for (i = 0; i < 64; i++)
	{
		if (inBitMap[i] == 0)
			return (VS_ERROR);

		if (inBitMap[i] == inFeild)
			return (VS_SUCCESS);
	}

	return (VS_SUCCESS);
}


/*
 * Function		: inSVC_ISO_GetBitMap
 * Date&Time	: 2022/9/14 下午 1:49
 * Describe		: 
 * int *inTxnBitMap : 外部傳入的 int 陣列，主要會拷貝 [交易要使用的BIT MAP] 的結構來進行修改 
 * unsigned char * uszSendBuf : 組封包結構的指標，實際要傳送主機電文的陣列	 
 * 
 * 利用外部丟入的 [ISO流程架構] srISOFuncIndex 執行對應的 GetBitBmp 功能 並回傳
  */
int inSVC_ISO_GetBitMap(TRANSACTION_OBJECT *pobTran,
		int inTxnType,
		ISO_TYPE_SVC_TABLE *srISOFuncIndex,		
		int *inTxnBitMap,
		unsigned char *uszSendBuf)
{
	
	int i, j, k, inBitMapIndex, *inBitMap, inCnt, inBitMapCode = _TRANS_TYPE_NULL_;
	unsigned char bBuf;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" inTxnType[%d]", inTxnType);
	
	inCnt = 0;

	if (srISOFuncIndex->inGetBitMapCode != NULL)
		inBitMapCode = srISOFuncIndex->inGetBitMapCode(pobTran, inTxnType);

	if (inBitMapCode == _TRANS_TYPE_NULL_)
		return (VS_ERROR);

	/*  BIT_MAP_SVC_TABLE srSVC_BitMap  inTxnID */
	if ((inBitMapIndex = inSVC_ISO_GetBMapMTIPCode(pobTran, srISOFuncIndex, inBitMapCode)) == VS_ERROR)
		return (VS_ERROR);

	/* BIT_MAP_SVC_TABLE srSVC_BitMap  BitMap */
	vdSVC_ISO_CopyISOMap(inTxnBitMap, srISOFuncIndex->srBitMap[inBitMapIndex].inBitMap);

	inDISP_LogPrintfWithFlag("uszSendBuf 1 = [%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x]!!",
			uszSendBuf[0], uszSendBuf[1], uszSendBuf[2], uszSendBuf[3], uszSendBuf[4],
			uszSendBuf[5], uszSendBuf[6], uszSendBuf[7], uszSendBuf[8], uszSendBuf[9],
			uszSendBuf[10], uszSendBuf[11], uszSendBuf[12], uszSendBuf[13], uszSendBuf[14]);

	if (srISOFuncIndex->inPackMTI != NULL)
		inCnt += srISOFuncIndex->inPackMTI(pobTran, inTxnType, &uszSendBuf[inCnt], srISOFuncIndex->srBitMap[inBitMapIndex].szMTI);

	inDISP_LogPrintfWithFlag("uszSendBuf 2 = [%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x]!!",
			uszSendBuf[0], uszSendBuf[1], uszSendBuf[2], uszSendBuf[3], uszSendBuf[4],
			uszSendBuf[5], uszSendBuf[6], uszSendBuf[7], uszSendBuf[8], uszSendBuf[9],
			uszSendBuf[10], uszSendBuf[11], uszSendBuf[12], uszSendBuf[13], uszSendBuf[14]);

	/* Bit Map */
	if (srISOFuncIndex->vdModifyBitMap != NULL)
		srISOFuncIndex->vdModifyBitMap(pobTran, inTxnType, inTxnBitMap);

	inDISP_LogPrintfWithFlag("uszSendBuf 3 = [%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x]!!",
			uszSendBuf[0], uszSendBuf[1], uszSendBuf[2], uszSendBuf[3], uszSendBuf[4],
			uszSendBuf[5], uszSendBuf[6], uszSendBuf[7], uszSendBuf[8], uszSendBuf[9],
			uszSendBuf[10], uszSendBuf[11], uszSendBuf[12], uszSendBuf[13], uszSendBuf[14]);
	
	/* Process Code */
	memset(szSVC_ISO_Field03, 0x00, sizeof (szSVC_ISO_Field03));
	
	if (inBitMapCode != _BATCH_UPLOAD_)
	{
		if (inBitMapCode == _SVC_REVERSAL_)
		{
			inFunc_ASCII_to_BCD((unsigned char *) &szSVC_ISO_Field03[0], srISOFuncIndex->srBitMap[inBitMapIndex].szPCode, _SVC_PCODE_SIZE_);

			if (pobTran->inTransactionCode == _VOID_ ||
				pobTran->inTransactionCode == _CUP_VOID_ ||
				pobTran->inTransactionCode == _CUP_PRE_AUTH_VOID_ ||
				pobTran->inTransactionCode == _CUP_PRE_COMP_VOID_)
			{
				if (pobTran->srBRec.inOrgCode == _REFUND_ || pobTran->srBRec.inOrgCode == _INST_REFUND_ ||
					pobTran->srBRec.inOrgCode == _CUP_REFUND_ || pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
					szSVC_ISO_Field03[0] = 0x22;
				
			} 
		} else
		{
			inFunc_ASCII_to_BCD((unsigned char *) &szSVC_ISO_Field03[0], srISOFuncIndex->srBitMap[inBitMapIndex].szPCode, _SVC_PCODE_SIZE_);

			if (pobTran->inTransactionCode == _VOID_ ||
					pobTran->inTransactionCode == _CUP_VOID_ ||
					pobTran->inTransactionCode == _CUP_PRE_AUTH_VOID_ ||
					pobTran->inTransactionCode == _CUP_PRE_COMP_VOID_)
			{
				if (pobTran->srBRec.inOrgCode == _REFUND_ || pobTran->srBRec.inOrgCode == _INST_REFUND_ ||
						pobTran->srBRec.inOrgCode == _CUP_REFUND_ || pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
					szSVC_ISO_Field03[0] = 0x22;
			}
		}
	} else
	{
		if (pobTran->inTransactionCode == _SETTLE_ && (pobTran->srBRec.inOrgCode == _REFUND_ ||
			pobTran->srBRec.inOrgCode == _CUP_REFUND_ ||
			pobTran->srBRec.inOrgCode == _INST_REFUND_ ||
			pobTran->srBRec.inOrgCode == _REDEEM_REFUND_))
			szSVC_ISO_Field03[0] = 0x20;

		if (pobTran->inTransactionCode == _SETTLE_ && (pobTran->srBRec.inOrgCode == _CUP_PRE_COMP_))
			szSVC_ISO_Field03[0] = 0x40;
	}

	inDISP_LogPrintfWithFlag("uszSendBuf 4 = [%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x]!!",
			uszSendBuf[0], uszSendBuf[1], uszSendBuf[2], uszSendBuf[3], uszSendBuf[4],
			uszSendBuf[5], uszSendBuf[6], uszSendBuf[7], uszSendBuf[8], uszSendBuf[9],
			uszSendBuf[10], uszSendBuf[11], uszSendBuf[12], uszSendBuf[13], uszSendBuf[14]);
	
	inBitMap = inTxnBitMap;
	for (i = 0;; i++)
	{
		bBuf = 0x80;
		if (*inBitMap == 0)
			break;

		j = *inBitMap / 8;
		k = *inBitMap % 8;

		if (k == 0)
		{
			j--;
			k = 8;
		}

		k--;

		while (k)
		{
			bBuf = bBuf >> 1;
			k--;
		}

		uszSendBuf[inCnt + j] += bBuf;
		inBitMap++;
	}

	inDISP_LogPrintfWithFlag("uszSendBuf 5 = [%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x]!!",
			uszSendBuf[0], uszSendBuf[1], uszSendBuf[2], uszSendBuf[3], uszSendBuf[4],
			uszSendBuf[5], uszSendBuf[6], uszSendBuf[7], uszSendBuf[8], uszSendBuf[9],
			uszSendBuf[10], uszSendBuf[11], uszSendBuf[12], uszSendBuf[13], uszSendBuf[14]);

	inCnt += _SVC_BIT_MAP_SIZE_;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
	
}

int inSVC_ISO_GetBitMapCode(TRANSACTION_OBJECT *pobTran, int inTxnType)
{
	int inBitMapCode = -1;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" inTxnType[%d]", inTxnType);

	switch (inTxnType)
	{
		case _SVC_TMS_:
		case _SVC_TMS_FINISH_:
		case _SVC_REDEEM_:
		case _SVC_RELOAD_:
		case _SVC_ACTIVECARD_:
		case _SVC_VOID_:
		case _SVC_REFUND_:
		case _SVC_SETTLE_:
		case _SVC_CLS_BATCH_:
		case _SVC_REVERSAL_:
		case _SVC_BATCHUPLOAD_:
		case _SVC_INQUIRY_:
		case _SVC_BACKCARD_:
			inBitMapCode = inTxnType;
			break;
		default:
			break;
	}

	return (inBitMapCode);
}






int inSVC_ISO_PackMTI(TRANSACTION_OBJECT *pobTran, int inTxnCode, unsigned char *bPackData, char *szMTI)
{
	int inCnt = 0;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" inTxnCode[%d] szMTI[%s]", inTxnCode, szMTI);

	inFunc_ASCII_to_BCD(&bPackData[inCnt], szMTI, _SVC_MTI_SIZE_);
	
	inCnt += _SVC_MTI_SIZE_;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

void vdSVC_ISO_ModifyBitMap(TRANSACTION_OBJECT *pobTran, int inTxnType, int *inBitMap)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" P-inTransactionCode[%d] Br-inCode[%d] P-inISOTxnCode[%d] inTxnType[%d] Br-inOrgCode[%d]", pobTran->inTransactionCode, 
											pobTran->srBRec.inCode, 
											pobTran->inISOTxnCode,
											inTxnType,
											pobTran->srBRec.inOrgCode);

	
	
//	vdSVC_ISO_MapReset(inBitMap, 37);
//	vdSVC_ISO_MapSet(inBitMap, 63);
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return;
}

void vdSVC_ISO_ModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *bPackData, int *inPackLen)
{
	return;
}

int inSVC_ISO_CheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader)
{
	int inCnt = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	/* TPDU */
	inCnt += _SVC_TPDU_SIZE_;
	/* �ˬdMTI */
	szSendISOHeader[inCnt + 1] += 0x10;
	if (memcmp(&szSendISOHeader[inCnt], &szReceISOHeader[inCnt], _SVC_MTI_SIZE_))
	{
		inDISP_DispLogAndWriteFlie(" InCnt[%d] Send[%02x%02x] != Rece[%02x%02x] Line[%d] END ", inCnt, szSendISOHeader[inCnt], 
						szSendISOHeader[inCnt + 1], szReceISOHeader[inCnt], szReceISOHeader[inCnt + 1], __LINE__ );
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
	
}

int inSVC_ISO_OnlineAnalyse(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	inDISP_LogPrintfWithFlag(" SVC_Analyse Magnetic Line[%d]", __LINE__);
	inRetVal = inSVC_OnlineAnalyse(pobTran);
		
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
	
}


/*
 App Name      :        inSVC_OnlineAnalyse
 App Function  : 1. 分析交易是否成功
 2. pobTran->srBRec.inTransactionResult 只會有
 【AUTHORIZED】【REFERRAL】【CANCELLED】【COMM_ERROR】
 3. 如果 pobTran->srBRec.szRespCode != 00 要顯示錯誤訊息
 Input Param   :
 Output Param  : 成功 --> VS_SUCCESS
 失敗 --> VS_ERROR
 */
int inSVC_OnlineAnalyse(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_SUCCESS;
//	RTC_NEXSYS srRTC; /* Date & Time */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	inDISP_LogPrintfWithFlag(" SVC_P-TxnResult[%d] B.inHDTIndex[%d] ", pobTran->inTransactionResult, pobTran->srBRec.inHDTIndex);

	if (pobTran->inTransactionCode == _SVC_SETTLE_ || pobTran->inTransactionCode == _SVC_CLS_BATCH_)
	{
		if (memcmp(pobTran->srBRec.szRespCode, "95", 2) && memcmp(pobTran->srBRec.szRespCode, "00", 2))
			inRetVal = VS_ERROR;
		else
		{
			if (pobTran->inTransactionResult == _TRAN_RESULT_SETTLE_UPLOAD_BATCH_ && !memcmp(pobTran->srBRec.szRespCode, "95", 2))
			{
				inRetVal = inSVC_ISO_ProcessSettleBatchUpload(pobTran);
			}
		}
	} else
	{
		if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_)
		{
			if (inSVC_SyncHostTerminalDateTime(pobTran) != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie(" SVC_OnlineAnalyse MSG Auth Sucess Sync Time *Error* Line[%d]", __LINE__);
				return (VS_ERROR);
			}

			if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
			{
				inSetSendReversalBit("N");
				if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
				{
					inDISP_DispLogAndWriteFlie(" SVC_OnlineAnalyse MSG Auth Sucess HDPT[%d] *Error* Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
					return (VS_ERROR);
				}

				inSVC_ISO_SendAdvice(pobTran, 1, VS_FALSE);
			}
		} else if (pobTran->inTransactionResult == _TRAN_RESULT_CANCELLED_)
		{
			if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
			{
				inSetSendReversalBit("N");
				if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
				{
					inDISP_DispLogAndWriteFlie(" SVC_OnlineAnalyse MSG Cancel HDPT[%d] *Error* Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
					return (VS_ERROR);
				}
			}

			inSVC_DispHostResponseCode(pobTran);
			inRetVal = VS_ERROR;
		} else
		{
			memset(pobTran->srBRec.szRespCode, 0x00, sizeof (pobTran->srBRec.szRespCode));
			inSVC_DispHostResponseCode(pobTran);
			inRetVal = VS_ERROR;
		}
	}

	inDISP_LogPrintfWithFlag("  inRetVal[%d]", inRetVal);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}



/*
 * Function		: inSVC_ISO_AdviceAnalyse
 * Date&Time	: 2022/9/15 上午 11:30
 * Describe		: 
 * 
 */
int inSVC_ISO_AdviceAnalyse(TRANSACTION_OBJECT *pobTran, VS_BOOL blTcUpload)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	if (!memcmp(&pobTran->srBRec.szRespCode[0], "00", 2))
	{
		if (inSVC_ISO_CheckAuthCode(pobTran) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" AdviceAnalyse Check AuthCode[%s] *Error* Line[%d]", pobTran->srBRec.szAuthCode, __LINE__);
			return (VS_ERROR);
		}

		if (pobTran->srBRec.inChipStatus == _EMV_CARD_ && pobTran->uszTCUploadBit == VS_TRUE)
		{
			inDISP_LogPrintfWithFlag(" %s()_EMV_TC_LOAD_SUCCESS", __FUNCTION__);
			return (VS_SUCCESS);
		} else
		{
			if (pobTran->srBRec.inCode == _TIP_)
			{
				if (pobTran->srBRec.uszUpload1Bit == VS_TRUE && pobTran->srBRec.uszUpload2Bit == VS_TRUE)
					pobTran->srBRec.uszUpload1Bit = VS_FALSE;
				else
				{
					pobTran->srBRec.uszUpload1Bit = VS_FALSE;
					pobTran->srBRec.uszUpload2Bit = VS_FALSE;
				}
			} else if (pobTran->srBRec.inCode == _VOID_ &&
					pobTran->srBRec.inOrgCode == _SALE_)
			{
				if (pobTran->srBRec.uszUpload1Bit == VS_TRUE && pobTran->srBRec.uszUpload2Bit == VS_TRUE)
					pobTran->srBRec.uszUpload1Bit = VS_FALSE; 
				else
				{
					pobTran->srBRec.uszUpload1Bit = VS_FALSE; 
					pobTran->srBRec.uszUpload2Bit = VS_FALSE; 
				}
			} else
			{
				if (!memcmp(pobTran->srBRec.szAuthCode, "Y1", 2) || !memcmp(pobTran->srBRec.szAuthCode, "Y3", 2) ||
					!memcmp(pobTran->srBRec.szAuthCode, "VLP", 3) || !memcmp(pobTran->srBRec.szAuthCode, "JCB", 3) ||
					pobTran->srBRec.uszReferralBit == VS_TRUE || pobTran->srBRec.uszOfflineBit == VS_TRUE)
				{
					if (pobTran->srBRec.uszUpload1Bit == VS_TRUE)
						pobTran->srBRec.uszUpload1Bit = VS_FALSE;
				}
			}

			pobTran->uszUpdateBatchBit = VS_TRUE;

			if (inFLOW_RunFunction(pobTran, _UPDATE_BATCH_) != VS_SUCCESS)
			{
				inDISP_LogPrintfWithFlag(" %s()_UPDATE_BATCH_ERROR", __FUNCTION__);
				return (VS_ERROR);
			}

			if (inADVICE_DeleteRecordFlow(pobTran, pobTran->srBRec.lnOrgInvNum) != VS_SUCCESS)
			//if (inADVICE_DeleteOrg(pobTran, pobTran->srBRec.lnOrgInvNum) != VS_SUCCESS)
			{
				inDISP_LogPrintfWithFlag(" %s()_DELETE_ERROR", __FUNCTION__);
				return (VS_ERROR);
			}

			inDISP_LogPrintfWithFlag(" Aft Advice pobTran->srBRec.szRefNo[%s]", pobTran->srBRec.szRefNo);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
			return (VS_SUCCESS);
		}
	} else
	{
		if (pobTran->inTransactionCode == _SETTLE_)
			inSVC_DispHostResponseCode(pobTran);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] *Error* END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_ERROR);
	
}

/*
 * Function		: inSVC_ISO_BuildAndSendPacket
 * Date&Time	: 2022/9/14 下午 3:08
 * Describe		: 
 * 處理項目包含通訊建立、Reversal處埋、Advice資料處理及 ISO傳送及解析。
 * 決定是否要 Online 或 Offline 處理交易。
 */
int inSVC_ISO_BuildAndSendPacket(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;
	char szDemoMode[2 + 1] = {0};

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	inDISP_LogPrintfWithFlag(" uszOfflineBit[%d] ", pobTran->srBRec.uszOfflineBit);

	pobTran->srBRec.uszNoSignatureBit = VS_FALSE;

	/* 教育訓練模式 */
	/* TODO: 目前DEMO還不知道要不要做? */
	memset(szDemoMode, 0x00, sizeof (szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		{
			long	lnAmount = 0L;
			char    szTemplate[20];
			char    szAmount[13 + 1];
			
			inDISP_PutGraphic(_SEND_, 0, _COORDINATE_Y_LINE_8_7_); /* 傳送中... */
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_); /* 處理中... */
			inDISP_PutGraphic(_RECEIVE_, 0, _COORDINATE_Y_LINE_8_7_); /* 接收中... */
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_); /* 處理中... */
			
			/* 日期 & 時間 */
			RTC_NEXSYS srRTC;
			memset(&srRTC, 0x00, sizeof (RTC_NEXSYS));
			if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
				return (VS_ERROR);
			inFunc_Sync_BRec_Date_Time(pobTran, &srRTC);
			/* 設定 SVC 交易時間 */
			memcpy(pobTran->srBRec.szSvcTxnDateTime, pobTran->srBRec.szDate, 8);
			memcpy(&pobTran->srBRec.szSvcTxnDateTime[8], pobTran->srBRec.szTime, 8);
			inDISP_LogPrintfWithFlag("Svc Tms Time[%s]", pobTran->srBRec.szSvcTxnDateTime);

			/* 序號 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "123456%03d%03d", pobTran->srBRec.lnBatchNum, pobTran->srBRec.lnOrgInvNum);
			strcpy(pobTran->srBRec.szRefNo, szTemplate);
			/* 主機回應碼 */
			strcpy(pobTran->srBRec.szRespCode, "00");

			memset(szAmount, 0x00, sizeof(szAmount));

			if (pobTran->srBRec.inCode == _SVC_BACKCARD_)
				inFunc_ASCII_to_BCD(pobTran->srBRec.szSvcPoint1, "0000000000",  5);
			else if (pobTran->srBRec.inCode == _SVC_INQUIRY_)
			{
				sprintf(szAmount, "%08ld00", lnSVC_BalaceAmount);
				inFunc_ASCII_to_BCD(pobTran->srBRec.szSvcPoint1, szAmount,  5);
			}
			else if (pobTran->srBRec.inCode == _SVC_REDEEM_)
			{
				lnAmount = lnSVC_BalaceAmount - pobTran->srBRec.lnTxnAmount;
				sprintf(szAmount, "%08ld00", lnAmount);
				inFunc_ASCII_to_BCD(pobTran->srBRec.szSvcPoint1, szAmount,  5);
			}
			else if (pobTran->srBRec.inCode == _SVC_REFUND_)
			{
				lnAmount = lnSVC_BalaceAmount + pobTran->srBRec.lnTxnAmount;
				sprintf(szAmount, "%08ld00", lnAmount);
				inFunc_ASCII_to_BCD(pobTran->srBRec.szSvcPoint1, szAmount,  5);
			}
			else if (pobTran->srBRec.inCode == _SVC_ACTIVECARD_ || pobTran->srBRec.inCode == _SVC_ACTIVECARD_NOT_AUTORELOAD_)
				inFunc_ASCII_to_BCD(pobTran->srBRec.szSvcPoint1, "0000000000",  5);
		}

		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] DEMO END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}else
	{
		if (inSVC_GetSTAN(pobTran) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" SVC_BuildAndSend Get Stan *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		}
		
		if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
		{
			if (pobTran->uszConnectionBit != VS_TRUE)
			{
				inRetVal = inFLOW_RunFunction(pobTran, _COMM_START_);
				if (inRetVal != VS_SUCCESS)
				{
					/* 通訊失敗‧‧‧‧ */
					pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
					pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
					inFunc_Display_Error(pobTran); /* 通訊失敗 */
					return (VS_COMM_ERROR);
				}
			}
			
			/* 餘額查詢不支援 Reversal 誤送會卡帳 */
			if (pobTran->inTransactionCode != _SVC_INQUIRY_)
			{
				if ((inRetVal = inSVC_ISO_ProcessReversal(pobTran)) != VS_SUCCESS)
				{
					inDISP_DispLogAndWriteFlie(" SVC_Process Reversal *Error* Val[%d] Line[%d]", inRetVal, __LINE__);
					pobTran->inErrorMsg = _ERROR_CODE_V3_SETTLE_NOT_SUCCESS_;
					return (VS_ERROR);
				}
			}

			if (pobTran->inTransactionCode == _SVC_SETTLE_)
			{
				inSetMustSettleBit("Y");
				if (VS_SUCCESS != inSaveHDPTRec(pobTran->srBRec.inHDTIndex))
				{
					inDISP_DispLogAndWriteFlie(" SVC_BuildAndSend Save HDPT *Error* HID[%d] Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
				}

				/* 開啟 TmsFPT 下載TMS要結帳的參數 */
				inEDCTMS_FUNC_SetTmsFtpSettleFlagOn();

				if ((inRetVal = inSVC_ISO_ProcessAdvice(pobTran)) != VS_SUCCESS)
				{
					inDISP_DispLogAndWriteFlie(" SVC_BuildAndSend Porcess Advice *Error* Val[%d] Line[%d]", inRetVal, __LINE__);
					pobTran->inErrorMsg = _ERROR_CODE_V3_SETTLE_NOT_SUCCESS_;
					return (VS_ERROR);
				}
#if 0//TODO: 要補做
				/*
				1. 930000 – 前次不平帳上傳失敗
				2. 這裡須要修改
				pobTran->inTransactionCode
				pobTran->srBRec.inCode

				*/
				if (fGetCLS_SETTLEBit() == VS_TRUE)
				{
					   pobTran->inTransactionCode = SVC_CLS_BATCH;
					   pobTran->srBRec.inCode = SVC_CLS_BATCH;
				}
				else
				{
					   pobTran->inTransactionCode = SVC_SETTLE;
					   pobTran->srBRec.inCode = SVC_SETTLE;
				}
#endif
				
			}
		}

		pobTran->uszContinueTxnAgainBit = VS_FALSE;

		do
		{
			if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
			{
				if ((inRetVal = inSVC_ISO_ProcessOnline(pobTran)) != VS_SUCCESS)
				{
					if ((pobTran->srBRec.inChipStatus == _EMV_CARD_) &&
						(pobTran->inTransactionCode == _SALE_ || pobTran->inTransactionCode == _PRE_AUTH_) &&
						(inRetVal != VS_ISO_PACK_ERR) &&
						(inRetVal != VS_ISO_UNPACK_ERROR))
					{
						pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
						inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] Aft Process Online Comm *Error* END -----", __FILE__, __FUNCTION__, __LINE__);
					} else
					{
						inSVC_SetSTAN(pobTran);
						inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] Aft Process Online *Error* END -----", __FILE__, __FUNCTION__, __LINE__);
						return (inRetVal);
					}
				}
			} else
			{
				if (inSVC_ISO_ProcessOffline(pobTran) != VS_SUCCESS)
				{
					inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] ProcessOffline Err END -----", __FILE__, __FUNCTION__, __LINE__);
					return (VS_ERROR);
				} else
					break;
			}

			pobTran->uszContinueTxnAgainBit = VS_FALSE;

			if (inSVC_ISO_AnalysePack(pobTran) != VS_SUCCESS)
			{
				inSVC_SetSTAN(pobTran);
				inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] Analyse *Error* END -----", __FILE__, __FUNCTION__, __LINE__);
				return (VS_ERROR);
			}

		} while (pobTran->uszContinueTxnAgainBit);

		inSVC_SetSTAN(pobTran);
	}
	
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}


/*
 * Function		: inSVC_ISO_ProcessReversal
 * Date&Time	: 2022/9/14 下午 3:08
 * Describe		: 
 * 先檢查是否要進行上筆 Reversal 的傳送，如無檔案需傳送，接著判斷是否要建立當筆 Reversal 檔案
  */
int inSVC_ISO_ProcessReversal(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_SUCCESS;
	char szSendReversalBit[2 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memset(szSendReversalBit, 0x00, sizeof (szSendReversalBit));
	if (inGetSendReversalBit(szSendReversalBit) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" SVC_GetSendReversalBit *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	if (!memcmp(szSendReversalBit, "Y", 1))
	{
		if ((inRetVal = inSVC_ISO_ReversalSend(pobTran)) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" SVC_Send Reversal *Error* Val[%d] Line[%d]", inRetVal, __LINE__);
			return (VS_ERROR);
		}
	}
	

	if (pobTran->uszReversalBit == VS_TRUE && inRetVal == VS_SUCCESS)
	{
		if ((inRetVal = inSVC_ISO_ReversalSave_Flow(pobTran, _SVC_REVERSAL_)) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" SVC Reversal Save Flow *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		}
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * Function		: inSVC_ISO_ProcessAdvice
 * Date&Time	: 2022/9/14 下午 3:08
 * Describe		: 
 * 抓取是否有未上傳資料的檔案，如有記錄就進行資料上傳。
 * 會把所有的記錄以單筆上傳的方法傳送。
 */
int inSVC_ISO_ProcessAdvice(TRANSACTION_OBJECT *pobTran)
{	
	int i, inAdvCnt;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	if ((inAdvCnt = inADVICE_GetTotalCount(pobTran)) == VS_NO_RECORD)
		return (VS_SUCCESS);

	for (i = 0; i < inAdvCnt; i++)
	{
		if (inSVC_ISO_SendAdvice(pobTran, 1, VS_FALSE) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" SVC_ISO ProcessAdvice Send *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		}
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}


/*
 * Function		: inSVC_ISO_ProcessSettleBatchUpload
 * Date&Time	: 2022/9/15 下午 12:12
 * Describe		: 
 * 主機回傳結帳交易不平帳時，要回傳端未機內的所有資料。
 */
int inSVC_ISO_ProcessSettleBatchUpload(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;
	int inBatchTotalCnt, i;
	int inBatchValidCnt = 0, inBatchValidTotalCnt = 0;
	int inRunCLS_BATCH = VS_TRUE;
	TRANSACTION_OBJECT OrgpobTran;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	inBatchTotalCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
	if (inBatchTotalCnt == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" SVC_ISO inBatchTotalCnt[%d] *Error* Line[%d]", inBatchTotalCnt, __LINE__);
		return (VS_ERROR);
	}
	else if (inBatchTotalCnt == 0)
	{
		inRunCLS_BATCH = VS_TRUE;
	} else if (inBatchTotalCnt > 0)
	{
		inBatchValidTotalCnt = inBATCH_GetTotalCount_BatchUpload_By_Sqlite(pobTran);

		/* BATCH UPLOAD */
		pobTran->uszLastBatchUploadBit = VS_FALSE;

		memset((char *) &OrgpobTran, 0x00, sizeof (TRANSACTION_OBJECT));
		memcpy((char *) &OrgpobTran, (char *) pobTran, sizeof (TRANSACTION_OBJECT));
		inSVC_SetSTAN(pobTran);

		inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);

		for (i = 0; i < inBatchTotalCnt; i++)
		{
			pobTran->inISOTxnCode = _SVC_BATCHUPLOAD_;
			if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, i) != VS_SUCCESS)
			{
				inRunCLS_BATCH = VS_FALSE;
				break;
			}

			if (pobTran->srBRec.uszCLSBatchBit == '1')
			{
				continue;
			}
			
			inBatchValidCnt++;

			if (inBatchValidCnt == inBatchValidTotalCnt)
				pobTran->uszLastBatchUploadBit = VS_TRUE;

			inSVC_GetSTAN(pobTran);

			inRetVal = inSVC_ISO_PackSendUnPackReceData(pobTran, pobTran->inISOTxnCode);
			
			inSVC_SetSTAN(pobTran);

			if (inRetVal != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie(" SVC_Adv Send inISOTxnCode[%d] *Error* Line[%d]",  pobTran->inISOTxnCode, __LINE__);
				inRunCLS_BATCH = VS_FALSE;
				break;
			} else
			{
				if (memcmp(pobTran->srBRec.szRespCode, "00", 2))
				{
					inSVC_DispHostResponseCode(pobTran);
					inRunCLS_BATCH = VS_FALSE;
					inDISP_DispLogAndWriteFlie(" SVC_Adv Send inISOTxnCode[%d] RespCode[%s] *Error* Line[%d]",  pobTran->inISOTxnCode, pobTran->srBRec.szRespCode, __LINE__);
					break;
				}

				inBATCH_Update_CLS_SettleBit_By_Sqlite(pobTran);
			}

		}

		inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);

		memcpy((char *) pobTran, (char *) &OrgpobTran, sizeof (TRANSACTION_OBJECT));
	}


	inSVC_GetSTAN(pobTran);

	if (inRunCLS_BATCH == VS_TRUE)
	{
		inSetCLS_SettleBit("N");
		inSaveHDPTRec(pobTran->srBRec.inHDTIndex);

		pobTran->inISOTxnCode = _SVC_BATCHUPLOAD_;

		if (inSVC_ISO_PackSendUnPackReceData(pobTran, pobTran->inISOTxnCode) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" SVC_Send _CLS_BATCH_ *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		}else
		{
			if (memcmp(pobTran->srBRec.szRespCode, "00", 2))
			{
				inDISP_DispLogAndWriteFlie(" SVC_Send _CLS_BATCH_ inISOTxnCode[%d] RespCode[%s] *Error* Line[%d]",  pobTran->inISOTxnCode, pobTran->srBRec.szRespCode, __LINE__);
				return (VS_ERROR);
			}
		}
	} else
	{
		inSetCLS_SettleBit("Y");
		inSaveHDPTRec(pobTran->srBRec.inHDTIndex);
		inDISP_DispLogAndWriteFlie(" SVC_CLS_BATCH_ != VS_TRUE *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}


/*
 * Function		: inSVC_ISO_ReversalSend
 * Date&Time	: 2022/9/14 下午 3:08
 * Describe		: 
 * 主要指定執行  _SVC_REVERSAL_  的傳送。
 * 如果要使用 inSVC_ISO_ReversalSendRecvPacket 傳送別種的 Reversal 檔案，可另外指定。
  */
int inSVC_ISO_ReversalSend(TRANSACTION_OBJECT *pobTran)
{
	return inSVC_ISO_ReversalSendRecvPacket(pobTran, _SVC_REVERSAL_);
}

/*
 * Function		: inSVC_ISO_ReversalSendRecvPacket
 * Date&Time	: 2022/9/14 下午 3:08
 * Describe		: 
 * 處理 Reversal 檔案的讀檔、傳送、接收及刪除。
 * 用檔名讀出Reversal檔案資料，進行傳送及接收，檢查完成後既刪除檔案。
  */
int inSVC_ISO_ReversalSendRecvPacket(TRANSACTION_OBJECT *pobTran, int inISOTxnCode)
{
	int inRetVal, inSendCnt;
	long lnREVCnt;
	char szDialBackupEnable[2 + 1];
	char szCommMode[2 + 1];
	unsigned char uszFileName[20 + 1];
	unsigned char uszSendPacket[_SVC_ISO_SEND_ + 1], uszRecvPacket[_SVC_ISO_RECV_ + 1];
	unsigned long ulREVFHandle;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	memset(uszSendPacket, 0x00, sizeof (uszSendPacket));
	memset(uszRecvPacket, 0x00, sizeof (uszRecvPacket));

	/* 如果有DialBackup */
	memset(szDialBackupEnable, 0x00, sizeof (szDialBackupEnable));
	inGetDialBackupEnable(szDialBackupEnable);
	memset(szCommMode, 0x00, sizeof (szCommMode));
	inGetCommMode(szCommMode);

	if (memcmp(szDialBackupEnable, "Y", strlen("Y")) == 0 && memcmp(szCommMode, _COMM_MODEM_MODE_, strlen(_COMM_MODEM_MODE_)) == 0)
	{
		memset(uszFileName, 0x00, sizeof (uszFileName));
		if (inFunc_ComposeFileName(pobTran, (char*) uszFileName, _REVERSAL_DIALBACKUP_FILE_EXTENSION_, 6) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" SVC_RSRP ComposeFileName_1 *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		}
	} else
	{
		memset(uszFileName, 0x00, sizeof (uszFileName));
		if (inFunc_ComposeFileName(pobTran, (char*) uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" SVC_RSRP ComposeFileName_2 *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		}
	}

	/* 開檔案 */
	if ((inRetVal = inFILE_OpenReadOnly(&ulREVFHandle, uszFileName)) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" SVC_RSRP FILE_OpenReadOnly *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	/* 取得 Reversal 檔案大小 */
	lnREVCnt = lnFILE_GetSize(&ulREVFHandle, uszFileName);
	if (lnREVCnt == 0)
	{
		inFILE_Close(&ulREVFHandle);
		return (VS_ERROR);
	}

	/* 指到第一個 BYTE */
	if ((inRetVal = inFILE_Seek(ulREVFHandle, 0, _SEEK_BEGIN_)) == VS_ERROR)
	{
		inFILE_Close(&ulREVFHandle);
		return (VS_ERROR);
	}

	/* 讀檔案 */
	if ((inRetVal = inFILE_Read(&ulREVFHandle, &uszSendPacket[0], lnREVCnt)) == VS_ERROR)
	{
		inFILE_Close(&ulREVFHandle);
		return (VS_ERROR);
	}

	/* 關檔案 */
	inFILE_Close(&ulREVFHandle);

	/* 傳送及接收 ISO 電文 */
	inSendCnt = lnREVCnt % 65536; /* Add by li for ln2int */

	if ((inRetVal = inSVC_ISO_SendReceData(pobTran, uszSendPacket, inSendCnt, uszRecvPacket)) != VS_SUCCESS)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;

		inDISP_DispLogAndWriteFlie(" SVC_ISO_SendReceData *Error* Line[%d]", __LINE__);
		return (VS_COMM_ERROR);
	}

	/* 解 ISO 電文 */
	pobTran->inISOTxnCode = inISOTxnCode;
	inRetVal = inSVC_ISO_UnPackISO(pobTran, uszSendPacket, inSendCnt, uszRecvPacket, inRetVal);
	if (inRetVal != VS_SUCCESS)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_UNPACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
		inDISP_DispLogAndWriteFlie(" SVC_ISO_UnPackISO *Error* Line[%d]", __LINE__);
		return (VS_ISO_UNPACK_ERROR);
	}

	/* memcmp兩字串相同回傳0 */
	if (memcmp(pobTran->srBRec.szRespCode, "00", 2) != 0)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_CANCELLED_;
		return (VS_ERROR);
	} else
	{
		inSetSendReversalBit("N");

		if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" SVC_RSRP SaveHDPTRec *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		}

		/* 如果有DialBackup，要砍兩個Reversal */
		memset(szDialBackupEnable, 0x00, sizeof (szDialBackupEnable));
		inGetDialBackupEnable(szDialBackupEnable);
		memset(szCommMode, 0x00, sizeof (szCommMode));
		inGetCommMode(szCommMode);

		if (memcmp(szDialBackupEnable, "Y", strlen("Y")) == 0 && memcmp(szCommMode, _COMM_MODEM_MODE_, strlen(_COMM_MODEM_MODE_)) == 0)
		{
			/* 刪除撥接reersal */
			memset(uszFileName, 0x00, sizeof (uszFileName));
			if (inFunc_ComposeFileName(pobTran, (char*) uszFileName, _REVERSAL_DIALBACKUP_FILE_EXTENSION_, 6) != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie(" SVC_RSRP ComposeFileName_3 *Error* Line[%d]", __LINE__);
				return (VS_ERROR);
			}

			inFILE_Delete(uszFileName);

			/* 刪除非撥接Reversal */
			memset(uszFileName, 0x00, sizeof (uszFileName));
			if (inFunc_ComposeFileName(pobTran, (char*) uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie(" SVC_RSRP ComposeFileName_4 *Error* Line[%d]", __LINE__);
				return (VS_ERROR);
			}

			inFILE_Delete(uszFileName);
		} else
		{
			memset(uszFileName, 0x00, sizeof (uszFileName));
			if (inFunc_ComposeFileName(pobTran, (char*) uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie(" SVC_RSRP ComposeFileName_5 *Error* Line[%d]", __LINE__);
				return (VS_ERROR);
			}

			inFILE_Delete(uszFileName);
		}
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
	
}


/*
 * Function		: inSVC_ISO_ReversalSave
 * Date&Time	: 2022/9/14 下午 3:08
 * Describe		: 
 * 處理 Reversal 檔案的建立，利用組 ISO 電文的API 進行資料組合至字元陣列中，
 * 再把字元陣列的資料寫入檔案中。
  */
int inSVC_ISO_ReversalSave(TRANSACTION_OBJECT *pobTran, int inISOTxnCode)
{
		int inRetVal, inPacketCnt;
	unsigned char uszReversalPacket[_SVC_ISO_RECV_ + 1];
	unsigned char uszFileName[20 + 1];
	unsigned long srFHandle;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" RS inHDTIndex[%d] inISOTxnCode[%d]", pobTran->srBRec.inHDTIndex, inISOTxnCode);

	inDISP_LogPrintfArea(TRUE, "", 0, uszReversalPacket, 300);

	memset(uszReversalPacket, 0x00, sizeof (uszReversalPacket));
	pobTran->inISOTxnCode = inISOTxnCode; /* 沖銷 */

	/* 組_REVERSAL_ 封包 */
	inPacketCnt = inSVC_ISO_PackISO(pobTran, &uszReversalPacket[0], pobTran->inISOTxnCode);
	if (inPacketCnt <= 0)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_PACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_PACK_;
		inDISP_DispLogAndWriteFlie(" RS ISO_PackISO *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	memset(uszFileName, 0x00, sizeof (uszFileName));
	if (inFunc_ComposeFileName(pobTran, (char*) uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" RS ComposeFileName_1 *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	/* 先刪除上一筆 Reversal，確保是當筆的 Reversal */
	inFILE_Delete(uszFileName);

	inRetVal = inFILE_Create(&srFHandle, uszFileName);
	if (inRetVal == VS_ERROR)
	{
		inFILE_Close(&srFHandle);
		inFILE_Delete(uszFileName);
		inDISP_DispLogAndWriteFlie(" RS inFILE_Create *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	inRetVal = inFILE_Seek(srFHandle, 0, _SEEK_BEGIN_); /* 從頭開始 */
	if (inRetVal == VS_ERROR)
	{
		inFILE_Close(&srFHandle);
		inFILE_Delete(uszFileName);
		inDISP_DispLogAndWriteFlie(" RS inFILE_Seek *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	inRetVal = inFILE_Write(&srFHandle, &uszReversalPacket[0], inPacketCnt);
	if (inRetVal == VS_ERROR)
	{
		inFILE_Close(&srFHandle);
		inFILE_Delete(uszFileName);
		inDISP_DispLogAndWriteFlie(" RS inFILE_Write *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	inFILE_Close(&srFHandle);
	if (inISOTxnCode == _SVC_REVERSAL_)
	{
		inSetSendReversalBit("Y");

		if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inFILE_Close(&srFHandle);
			inFILE_Delete(uszFileName);
			inDISP_DispLogAndWriteFlie(" RS inSaveHDPTRec[%d] *Error* Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
			return (VS_ERROR);
		}
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
	
}


/*
 * Function		: inSVC_ISO_ProcessOnline
 * Date&Time	: 2022/9/15 上午 10:51
 * Describe		: 
 * 執行當筆交易資料上傳的處理，
 * 1. 檢查是否要建立檔筆交易 Reversal 檔案
 * 2. 傳送資料到主機及接收主機回傳資料
 * 3. 如主機資料接收成功，針對制定[解封包的功能結構]的欄位，
 *     進行主機回傳資料的拷問到相對應的 srBRec-> 欄位中
 * 4. 如主機接收資料失敗或其它錯誤，就回傳錯誤回應碼
 */
int inSVC_ISO_ProcessOnline(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	if (pobTran->uszContinueTxnAgainBit == VS_TRUE)
	{
		if (pobTran->uszReversalBit == VS_TRUE)
		{
			if ((inRetVal = inSVC_ISO_ReversalSave_Flow(pobTran, _SVC_REVERSAL_)) != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie(" SVC_P_Oline ISO_ReversalSave_Flow *Error* Line[%d]", __LINE__);
				return (VS_ERROR);
			}
		}
	}

	pobTran->inISOTxnCode = pobTran->inTransactionCode;
	inRetVal = inSVC_ISO_PackSendUnPackReceData(pobTran, pobTran->inISOTxnCode);

	if (inRetVal == VS_ERROR)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
		inDISP_DispLogAndWriteFlie(" SVC_P_Oline ISO_PackSendUnPackReceData *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
		
	} else if (inRetVal == VS_ISO_PACK_ERR || inRetVal == VS_ISO_UNPACK_ERROR)
	{
		if (inRetVal == VS_ISO_PACK_ERR)
		{
			pobTran->inTransactionResult = _TRAN_RESULT_PACK_ERR_;
			pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_PACK_;
		} else if (inRetVal == VS_ISO_UNPACK_ERROR)
		{
			pobTran->inTransactionResult = _TRAN_RESULT_UNPACK_ERR_;
			pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
		}
		inDISP_DispLogAndWriteFlie(" SVC_P_Oline Unpack *Error* [%d]  Line[%d]", inRetVal, __LINE__);
		return (inRetVal);
	} else
	{
		pobTran->inTransactionResult = inSVC_ISO_CheckRespCode(pobTran);

		inDISP_LogPrintfWithFlag(" SVC_Aft Check Resp Code [%d] ", pobTran->inTransactionResult);

		if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_)
		{
			/* 目前交易完成沒有要判斷，留著項目 */
		} else if (pobTran->inTransactionResult == VS_ISO_UNPACK_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" SVC_P_Oline VS_ISO_UNPACK_ERROR *Error* Line[%d]", __LINE__);
			return (VS_ISO_UNPACK_ERROR);
		}
	}

	inDISP_LogPrintfWithFlag(" TxnResult[%d] ", pobTran->inTransactionResult);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
	
}

/*
 * Function		: inSVC_ISO_ProcessOffline
 * Date&Time	: 2022/9/15 上午 10:53
 * Describe		: 
 * 進行離線交易處理，檢查離線處理條件是否成立
 * 1. 條件成立，判斷是否為取消交易:
 *   1.1取消交易，需刪除此筆交易序號的記錄
 *   1.2正常交易，需建立一筆未上傳離線交易記錄
 * 2. 條作不成立，既回覆交易失敗
 */
int inSVC_ISO_ProcessOffline(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_SUCCESS;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	inDISP_LogPrintfWithFlag(" Process Offline RRN [%s]", pobTran->srBRec.szRefNo);

	if (!memcmp(pobTran->srBRec.szAuthCode, "Y1", 2) ||
		!memcmp(pobTran->srBRec.szAuthCode, "VLP", 3) ||
		!memcmp(pobTran->srBRec.szAuthCode, "JCB", 3) ||
		pobTran->srBRec.uszReferralBit == VS_TRUE ||
		pobTran->srBRec.uszOfflineBit == VS_TRUE)
	{
		if (pobTran->inTransactionCode == _VOID_ && pobTran->srBRec.uszOfflineBit == VS_TRUE &&
			pobTran->srBRec.uszUpload1Bit == VS_TRUE && pobTran->srBRec.uszUpload2Bit == VS_TRUE)
		{
			if (inADVICE_DeleteRecordFlow(pobTran, pobTran->srBRec.lnOrgInvNum) != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie(" SVC_P_Offline Ad DeleteRec *Error* Line[%d]", __LINE__);
				inRetVal = VS_ERROR;
			} else
			{
				memset(pobTran->srBRec.szRespCode, 0x00, sizeof (pobTran->srBRec.szRespCode));
				strcpy(pobTran->srBRec.szRespCode, "00");
				pobTran->srBRec.szRespCode[2] = 0x00;
				pobTran->srBRec.uszUpload1Bit = VS_FALSE;
				pobTran->srBRec.uszUpload2Bit = VS_FALSE;
			}
		} else
		{
			/* Store Advice */
			if (inADVICE_SaveAppend(pobTran, pobTran->srBRec.lnOrgInvNum) != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie(" SVC_P_Offline Ad SaveAddend *Error* Line[%d]", __LINE__);
				inRetVal = VS_ERROR;
			} else
			{
				memset(pobTran->srBRec.szRespCode, 0x00, sizeof (pobTran->srBRec.szRespCode));
				strcpy(pobTran->srBRec.szRespCode, "00");
				pobTran->srBRec.szRespCode[2] = 0x00;
			}
		}
	}
	else
	{
		inDISP_DispLogAndWriteFlie(" SVC_P_Offline Other Fail *Error* Line[%d]", __LINE__);
		inRetVal = VS_ERROR;
	}
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);	
}



/*
 * Function		: inSVC_ISO_CheckRespCode
 * Date&Time	: 2022/9/15 上午 10:38
 * Describe		: 
 * 檢查 srBRec.szRespCode 的對應值後設定對應的回傳值，
 * 目的是用來判斷交易是否繼續進行或顯示對應的訊息。
 */
int inSVC_ISO_CheckRespCode(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = _TRAN_RESULT_COMM_ERROR_;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);	
	
	inDISP_LogPrintfWithFlag(" SVC CheckRespCode RCode[%s] ", pobTran->srBRec.szRespCode);

	if (!memcmp(pobTran->srBRec.szRespCode, "00", 2))
	{
		inRetVal = _TRAN_RESULT_AUTHORIZED_;
	} else if (!memcmp(pobTran->srBRec.szRespCode, "  ", 2))
	{
		inRetVal = VS_ISO_UNPACK_ERROR;
	} else if (pobTran->srBRec.szRespCode[0] == 0x00)
	{
		inRetVal = _TRAN_RESULT_COMM_ERROR_;
	} else
	{
		if( (pobTran->srBRec.szRespCode[0] >= 0x30 && pobTran->srBRec.szRespCode[0] <= 0x39) ||
			(pobTran->srBRec.szRespCode[0] >= 0x41 && pobTran->srBRec.szRespCode[0] <= 0x5A))
		{
			if( (pobTran->srBRec.szRespCode[1] >= 0x30 && pobTran->srBRec.szRespCode[1] <= 0x39) ||
				(pobTran->srBRec.szRespCode[1] >= 0x41 && pobTran->srBRec.szRespCode[1] <= 0x5A))
			{
				if ((pobTran->inISOTxnCode == _SVC_SETTLE_ || pobTran->inISOTxnCode == _SVC_CLS_BATCH_) && (!memcmp(&pobTran->srBRec.szRespCode[0], "95", 2)))
				{
					inRetVal = _TRAN_RESULT_SETTLE_UPLOAD_BATCH_;
				}
				else
				{
					inRetVal = _TRAN_RESULT_CANCELLED_;
				}
			}else
			{
				inRetVal = VS_ISO_UNPACK_ERROR;
			}
		}else
		{
			inRetVal = VS_ISO_UNPACK_ERROR;
		}
	}

	inDISP_LogPrintfWithFlag(" Return Val[%d]", inRetVal);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
	
}


/*
 * Function		: inSVC_ISO_CheckAuthCode
 * Date&Time	: 2022/9/15 上午 11:06
 * Describe		: 
 * 設定條件，判斷是否要檢查主機回傳的授權碼[srBRec.szAuthCode]
 * 如為空白或'0' ,需拒絕交易。
 */
int inSVC_ISO_CheckAuthCode(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_SUCCESS;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);	
	
	inDISP_LogPrintfWithFlag(" ISOTCode[%d] ACode[%s] ", pobTran->inISOTxnCode, pobTran->srBRec.szAuthCode);

	switch (pobTran->inTransactionCode)
	{ /* 不檢核 */
		case _SETTLE_:
		case _BATCH_UPLOAD_:
		case _CLS_BATCH_:
		case _CUP_SALE_:
		case _CUP_REFUND_:
		case _CUP_PRE_AUTH_:
		case _CUP_PRE_COMP_:
		case _CUP_VOID_:
		case _CUP_PRE_AUTH_VOID_:
		case _CUP_PRE_COMP_VOID_:
		case _CHANGE_TMK_:
		case _CUP_CHANG_TPK_:
			break;
		default:
			if (!memcmp(&pobTran->srBRec.szAuthCode[0], "000000", 6) || 
				!memcmp(&pobTran->srBRec.szAuthCode[0], "      ", 6))
			{
				inDISP_DispLogAndWriteFlie(" FU AuthCode is 0 or blank  *Error* Line[%d]", __LINE__);
				inRetVal = VS_ERROR;
			}
			break;
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);	
	return (inRetVal);
}


/*
 * Function		: inSVC_ISO_AnalysePack
 * Date&Time	: 2022/9/15 上午 11:19
 * Describe		: 
 * 進行接收主機資料後的分析。
 * 利用定義 [ISO流程架構] 的解析功能進行，如無定義功能則不執行。
 */
int inSVC_ISO_AnalysePack(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_SUCCESS;
	ISO_TYPE_SVC_TABLE srISOFunc;
//	char szEncryptMode[1 + 1] = {0};

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" B.inChipStatus[%d] ", pobTran->srBRec.inChipStatus);

	if (pobTran->srBRec.uszOfflineBit == VS_TRUE)
	{
		inDISP_LogPrintfWithFlag(" inSVC_ISO_AnalysePack()_OFFLINE_END!!");
		return (inRetVal);
	}

	/* 拷貝對應的 [ISO流程架構] 來使用 */
	/* 目前只有一個結構，所以設定為 0  */
	memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
	memcpy((char *) &srISOFunc, (char *) &srSVC_ISOFunc[0], sizeof (srISOFunc));
		
	if (srISOFunc.inOnAnalyse != NULL)
	{
		inRetVal = srISOFunc.inOnAnalyse(pobTran);
	}

	inDISP_LogPrintfWithFlag(" AnalysePack inRetVal[%d]", inRetVal);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
	
}


/*
 * Function		: inSVC_ISO_SendAdvice
 * Date&Time	: 2022/9/14 下午 3:08
 * Describe		: 
 * 讀出資料庫的資料，會建立暫存的 TRANSACTION_OBJECT，再進行 ISO PACK 的重組及解析
 * 如果資料上傳成功，會在 inAdviceAnalyse 裏進行檢查及資料刪除
  */
int inSVC_ISO_SendAdvice(TRANSACTION_OBJECT *pobTran, int inAdvCnt, VS_BOOL blTcUpload)
{
	int inCnt, inRetVal = VS_ERROR, inISOFuncIndex = -1;
	TRANSACTION_OBJECT AdvpobTran;
	ISO_TYPE_SVC_TABLE srISOFunc;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	/* 目前只有一個結構，所以設定為 0  */
	inISOFuncIndex = 0;
	memset((char *) &srISOFunc, 0x00, sizeof (srISOFunc));
	memcpy((char *) &srISOFunc, (char *) &srSVC_ISOFunc[inISOFuncIndex], sizeof (srISOFunc));

	memset((char *) &AdvpobTran, 0x00, sizeof (TRANSACTION_OBJECT));
	memcpy((char *) &AdvpobTran, (char *) pobTran, sizeof (TRANSACTION_OBJECT));

	if (AdvpobTran.uszTCUploadBit != VS_TRUE && AdvpobTran.uszFiscConfirmBit != VS_TRUE)
	{
		if (inBATCH_GlobalAdviceHandleReadOnly_By_Sqlite() == VS_ERROR)
		{
			inDISP_LogPrintfWithFlag(" inSVC_ISO_SendAdvice() fail 1!!");
			return (VS_ERROR);
		}
	}

	for (inCnt = 0; inCnt < inAdvCnt; inCnt++)
	{
		if (AdvpobTran.uszTCUploadBit != VS_TRUE && AdvpobTran.uszFiscConfirmBit != VS_TRUE)
		{
			AdvpobTran.srBRec.lnOrgInvNum = _BATCH_LAST_RECORD_;
			inRetVal = inBATCH_GetAdviceDetailRecord_By_Sqlite(&AdvpobTran, inCnt);
			if (inRetVal == VS_SUCCESS)
			{
				inRetVal = VS_SUCCESS;
			} else if (inRetVal == VS_NO_RECORD)
			{
				inRetVal = VS_NO_RECORD;
				break;
			} else
			{
				inRetVal = VS_ERROR;
				break;
			}
		} else
		{
			inRetVal = VS_SUCCESS;
		}

		/* SVC 沒有別的條件，所以只留原本的設定  */
		AdvpobTran.inISOTxnCode = _ADVICE_;
		

		if (inRetVal == VS_SUCCESS)
			inRetVal = inSVC_ISO_PackSendUnPackReceData(&AdvpobTran, AdvpobTran.inISOTxnCode);

		if (inRetVal == VS_SUCCESS)
			if (srISOFunc.inAdviceAnalyse != NULL)
				inRetVal = srISOFunc.inAdviceAnalyse(&AdvpobTran, blTcUpload);

		if (inRetVal != VS_SUCCESS)
			break;
	}

	inDISP_LogPrintfWithFlag(" inRetVal[%d]", inRetVal);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
	
}

int inSVC_ISO_ReversalSave_For_DialBeckUp(TRANSACTION_OBJECT *pobTran)
{
	inDISP_LogPrintfWithFlag(" inSVC_ISO_ReversalSave_For_DialBeckUp() END!!");
	return (VS_SUCCESS);
}

/*
 * Function		: inSVC_ISO_ReversalSave_Flow
 * Date&Time	: 2022/9/14 下午 5:36
 * Describe		: 
 * 執行需要建立當筆交易 Reversal 檔案 的流程，
 * 可能會有不同的Reversal 檔案在此建立。
 */
int inSVC_ISO_ReversalSave_Flow(TRANSACTION_OBJECT *pobTran, int inISOTxnCode)
{
	int inRetVal;
	char szDialBackupEnable[2 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	inRetVal = inSVC_ISO_ReversalSave(pobTran, inISOTxnCode);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] ReversalSave *Error* END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	memset(szDialBackupEnable, 0x00, sizeof (szDialBackupEnable));
	inGetDialBackupEnable(szDialBackupEnable);

	if (memcmp(szDialBackupEnable, "Y", strlen("Y")) == 0)
	{
		inRetVal = inSVC_ISO_ReversalSave_For_DialBeckUp(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] Dial_Bek ReversalSave *Error* END -----", __FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
		}

	}
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
	
}


/*
 * Function		: inSVC_ISO_SendReceData
 * Date&Time	: 2022/9/14 下午 3:35
 * Describe		: 
 * 將傳入資料 [uszSendBuf] ，利用通訊的API傳送到主機，並接收主機資料回填[uszReceBuf]
 * 
 */
int inSVC_ISO_SendReceData(TRANSACTION_OBJECT *pobTran,
		unsigned char *uszSendBuf,
		int inSendLen,
		unsigned char *uszReceBuf)
{
	int inRetVal;
	int inSendTimeout = 10;
	int inReceiveTimeout = 10;
	int inReceiveSize = _COMM_RECEIVE_MAX_LENGTH_; /* 配合Ingenico */
	char szTimeOut[2 + 1] = {0};
	char szHostResponseTimeOut[2 + 1] = {0};

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	/* 如果沒設定TimeOut，就用EDC.dat內的TimeOut */
	if (inSendTimeout <= 0)
	{
		memset(szTimeOut, 0x00, sizeof (szTimeOut));
		inGetIPSendTimeout(szTimeOut);
		inSendTimeout = atoi(szTimeOut);
	}

	/* 顯示ISO Debug */
	if (ginDebug == VS_TRUE)
	{
		vdSVC_ISO_ISOFormatDebug_DISP(&uszSendBuf[0], inSendLen);
	}
//	/* 列印ISO Debug */
//	if (ginISODebug == VS_TRUE)
//	{
//		vdSVC_ISO_ISOFormatDebug_PRINT(&uszSendBuf[0], inSendLen);
//	}

	if ((inRetVal = inCOMM_Send(uszSendBuf, inSendLen, inSendTimeout, VS_TRUE)) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" SVC_inCOMM_Send *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	/* 這裡是用CPT.DAT的東西，所以一定要用 */
	memset(szHostResponseTimeOut, 0x00, sizeof (szHostResponseTimeOut));
	inRetVal = inGetHostResponseTimeOut(szHostResponseTimeOut);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" SVC_GetHostResponseTimeOut *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	inReceiveTimeout = atoi(szHostResponseTimeOut);
	
	inDISP_LogPrintfWithFlag(" SVC_ReceiveTimout [%d] LINE[%d]", inReceiveTimeout, __LINE__);
	
	if ((inRetVal = inCOMM_Receive(uszReceBuf, inReceiveSize, inReceiveTimeout, VS_TRUE)) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" SVC_inCOMM_Receive [%d] *Error*  Line[%d]", inRetVal, __LINE__);
		return (VS_ERROR);
	}


	/* 顯示ISO Debug */
	if (ginDebug == VS_TRUE)
		vdSVC_ISO_ISOFormatDebug_DISP(&uszReceBuf[0], inRetVal);
//	/* 列印ISO Debug */
//	if (ginISODebug == VS_TRUE)
//		vdSVC_ISO_ISOFormatDebug_PRINT(&uszReceBuf[0], inRetVal);

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);	
}

/*
 * Function		: inSVC_DispHostResponseCode
 * Date&Time	: 2022/9/15 上午 11:53
 * Describe		: 
 * 比對主機回應碼，顯示符合銀行規格的訊息及回應碼。
 * 
 */
int inSVC_DispHostResponseCode(TRANSACTION_OBJECT *pobTran)
{
	int inChoice = _DisTouch_No_Event_;
	int inRetVal = VS_SUCCESS;
	char szResponseCode[10 + 1] = {0};
	char szMsg[42 + 1] = {0};
	unsigned char uszKey = 0x00;

	memset(szMsg, 0x00, sizeof (szMsg));
	inDISP_LogPrintfWithFlag(" inSVC_DispHostResponseCode 1,%s!!", pobTran->srBRec.szRespCode);

	if ((pobTran->srBRec.szRespCode[0] >= '0' && pobTran->srBRec.szRespCode[0] <= '9') && (pobTran->srBRec.szRespCode[1] >= '0' && pobTran->srBRec.szRespCode[1] <= '9'))
	{
		strcat(szMsg, "拒絕交易");
	}else
	{
		if (!memcmp(&pobTran->srBRec.szRespCode[0], "CB", 2))
			strcat(szMsg, "超過上限拒絕交易");
		else
			strcat(szMsg, "拒絕交易");
	}
		
	memset(szResponseCode, 0x00, sizeof (szResponseCode));
	sprintf(szResponseCode, "%s", pobTran->srBRec.szRespCode); /* 錯誤代碼 */

	/* 清下排 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 顯示主機回覆訊息 */
	inDISP_ChineseFont(szResponseCode, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
	inDISP_ChineseFont(szMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);

	/* 不需要使用按鍵，等待一秒就離開 20190523  [SAM] */
#if _MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_
	inDISP_BEEP(1, 0);
	inDISP_Wait(1000);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
#else	

	/* 抓取 KioskFlag的值，如為 TRUE 就不用確認畫面 2020/3/6 下午 4:28 [SAM] */
	if (inFunc_GetKisokFlag() == VS_TRUE)
	{
		inDISP_BEEP(1, 0);
		inDISP_Wait(500);
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_LogPrintfWithFlag(" SVC_Kiosk DispHostResp Line[%d]", __LINE__);
	} else
	{

		inDISP_PutGraphic(_ERR_CLEAR_, 0, _COORDINATE_Y_LINE_8_7_);
		inDISP_BEEP(1, 0);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(_Touch_BATCH_END_);
			uszKey = uszKBD_Key();
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				uszKey = _KEY_TIMEOUT_;
			}

			if (uszKey == _KEY_CANCEL_ ||
				inChoice == _BATCH_END_Touch_ENTER_BUTTON_)
			{
				inRetVal = VS_SUCCESS;
				break;
			} else if (uszKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_SUCCESS;
				break;
			} else
			{
				continue;
			}
		}
		/* 為了強調Timeout時間 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDisTouch_Flush_TouchFile();
	}
#endif

	return (inRetVal);
}


/*
 * App Name		: inSVC_ISO_GetFieldIndex
 * App Builder		: 
 * App Date&Time	: 2022/12/14 下午 6:01
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inSVC_ISO_GetFieldIndex(int inField, ISO_FIELD_TYPE_SVC_TABLE *srFieldType)
{
	int i;

	for (i = 0;; i++)
	{
		if (srFieldType[i].inFieldNum == 0)
			break;

		if (srFieldType[i].inFieldNum == inField)
		{
			return (i);
		}
	}

	return (VS_ERROR);
}


/*
 * App Name		: vdSVC_ISO_ISOFormatDebug_DISP
 * App Builder		: 
 * App Date&Time	: 2022/12/14 下午 5:54
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
void vdSVC_ISO_ISOFormatDebug_DISP(unsigned char *bSendBuf, int inSendLen)
{
	char szPrtBuf[50], szBuf[500], szBitMap[_SVC_BIT_MAP_SIZE_ + 1];
	int i, j, inCnt = 0, inField, inLen, inFieldLen;
	int inBitMapCnt1, inBitMapCnt2;
	unsigned char uszBitMap[9], uszBitMapDisp[66];
	ISO_TYPE_SVC_TABLE srISOTypeTableDisp;
	char szEncryptMode[1 + 1] = {0};

	inGetEncryptMode(szEncryptMode);
	memset((char *) &srISOTypeTableDisp, 0x00, sizeof (srISOTypeTableDisp));
	memcpy((char *) &srISOTypeTableDisp, (char *) &srSVC_ISOFunc[0], sizeof (srISOTypeTableDisp));

	inDISP_LogPrintfWithFlag("==========================================");

	if (bSendBuf[_SVC_TPDU_SIZE_ + 1] & 0x10)
	{
		inDISP_LogPrintfWithFlag("ISO8583 Format  <<Recvive Data>>");
	} else
	{
		inDISP_LogPrintfWithFlag("ISO8583 Format  <<Send Data>>");
	}

	memset(szBuf, 0x00, sizeof (szBuf));
	sprintf(szBuf, "TPDU = [");
	inFunc_BCD_to_ASCII(&szBuf[8], &bSendBuf[inCnt], _SVC_TPDU_SIZE_);
	strcat(szBuf, "]");
	inDISP_LogPrintfWithFlag("%s", szBuf);
	inCnt += _SVC_TPDU_SIZE_;

	memset(szBuf, 0x00, sizeof (szBuf));
	sprintf(szBuf, "MTI  = [");
	inFunc_BCD_to_ASCII(&szBuf[8], &bSendBuf[inCnt], _SVC_MTI_SIZE_);
	strcat(szBuf, "]");
	inDISP_LogPrintfWithFlag("%s", szBuf);
	inCnt += _SVC_MTI_SIZE_;

	memset(szBuf, 0x00, sizeof (szBuf));
	memset(szBitMap, 0x00, sizeof (szBitMap));
	memcpy(szBitMap, (char *) &bSendBuf[inCnt], _SVC_BIT_MAP_SIZE_);
	sprintf(szBuf, "BMap = [%02X %02X %02X %02X %02X %02X %02X %02X]",
			bSendBuf[inCnt + 0],
			bSendBuf[inCnt + 1],
			bSendBuf[inCnt + 2],
			bSendBuf[inCnt + 3],
			bSendBuf[inCnt + 4],
			bSendBuf[inCnt + 5],
			bSendBuf[inCnt + 6],
			bSendBuf[inCnt + 7]);

	inDISP_LogPrintfWithFlag("%s", szBuf);
	memset(uszBitMapDisp, 0x00, sizeof (uszBitMapDisp));
	memset(uszBitMap, 0x00, sizeof (uszBitMap));
	memcpy(uszBitMap, (char *) &bSendBuf[inCnt], _SVC_BIT_MAP_SIZE_);

	for (inBitMapCnt1 = 0; inBitMapCnt1 < 8; inBitMapCnt1++)
	{
		for (inBitMapCnt2 = 0; inBitMapCnt2 < 8; inBitMapCnt2++)
		{
			if (uszBitMap[inBitMapCnt1] & 0x80)
				uszBitMapDisp[(inBitMapCnt1 * 8) + inBitMapCnt2 + 1] = '1';
			else
				uszBitMapDisp[(inBitMapCnt1 * 8) + inBitMapCnt2 + 1] = '0';

			uszBitMap[inBitMapCnt1] = uszBitMap[inBitMapCnt1] << 1;
		}
	}


	inDISP_LogPrintfWithFlag("   123456789A    123456789A    123456789A");
	memset(szBuf, 0x00, sizeof (szBuf));
	sprintf(szBuf, " 0>%10.10s  1>%10.10s  2>%10.10s", &uszBitMapDisp[1], &uszBitMapDisp[11], &uszBitMapDisp[21]);
	inDISP_LogPrintfWithFlag("%s", szBuf);
	sprintf(szBuf, " 3>%10.10s  4>%10.10s  5>%10.10s", &uszBitMapDisp[31], &uszBitMapDisp[41], &uszBitMapDisp[51]);
	inDISP_LogPrintfWithFlag("%s", szBuf);
	sprintf(szBuf, " 6>%4.4s", &uszBitMapDisp[61]);
	inDISP_LogPrintfWithFlag("%s", szBuf);

	inCnt += _SVC_BIT_MAP_SIZE_;

	for (i = 1; i <= 64; i++)
	{
		if (!inSVC_BitMapCheck((unsigned char *) szBitMap, i))
			continue;

		inLen = 0;
		inFieldLen = 0;
		memset(szBuf, 0x00, sizeof (szBuf));
		sprintf(szBuf, "F_%02d ", i);
		inLen = 5;
		inField = inSVC_ISO_GetFieldIndex(i, srISOTypeTableDisp.srISOFieldType);
		inFieldLen = inSVC_ISO_GetFieldLen(i, &bSendBuf[inCnt], srISOTypeTableDisp.srISOFieldType);
		switch (srISOTypeTableDisp.srISOFieldType[inField].inFieldType)
		{
			case _SVC_ISO_ASC_:
				memcpy(&szBuf[inLen], &bSendBuf[inCnt], inFieldLen);
				break;
			case _SVC_ISO_NIBBLE_2_:
			case _SVC_ISO_BYTE_2_:
			case _SVC_ISO_BYTE_3_:
			case _SVC_ISO_BYTE_2_H_:
			case _SVC_ISO_BYTE_3_H_:
				strcat(&szBuf[inLen++], "[");
				inFunc_BCD_to_ASCII(&szBuf[inLen], &bSendBuf[inCnt++], 1);
				inLen += 2;
				inFieldLen--;
				if (srISOTypeTableDisp.srISOFieldType[inField].inFieldType == _SVC_ISO_BYTE_3_ ||
						srISOTypeTableDisp.srISOFieldType[inField].inFieldType == _SVC_ISO_BYTE_3_H_)
				{
					inFunc_BCD_to_ASCII(&szBuf[inLen], &bSendBuf[inCnt++], 1);
					inLen += 2;
					inFieldLen--;
				}
				strcat(&szBuf[inLen++], "]");
			case _SVC_ISO_BCD_:
				if (srISOTypeTableDisp.srISOFieldType[inField].uszDispAscii == VS_TRUE)
					memcpy(&szBuf[inLen], (char *) &bSendBuf[inCnt], inFieldLen);
				else
					inFunc_BCD_to_ASCII(&szBuf[inLen], &bSendBuf[inCnt], inFieldLen);
				break;
			default:
				break;
		}

		j = 0;

		do
		{
			memset(szPrtBuf, 0x00, sizeof (szPrtBuf));
			memcpy(szPrtBuf, &szBuf[j * 42], 42);
			inDISP_LogPrintfWithFlag("%s", szPrtBuf);
			j++;
		} while ((j * 42) < strlen(szBuf));

		inCnt += inFieldLen;
	}

	inDISP_LogPrintfWithFlag("==========================================");
}



void vdSVC_ISO_ISOFormatDebug_PRINT(unsigned char *bSendBuf, int inSendLen)
{
	
}

void vdSVC_ISO_ISOFormatDebug_EMV(unsigned char *bSendBuf, int inFieldLen)
{
	
}

void vdSVC_ISO_ISOFormatDebug_48(unsigned char *bSendBuf, int inFieldLen)
{
	
}

void vdSVC_ISO_ISOFormatDebug_58(unsigned char *bSendBuf, int inFieldLen)
{
	
}
