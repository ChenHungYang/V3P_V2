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
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/FILE_FUNC/File.h"
#include "../SOURCE/FUNCTION/FILE_FUNC/FIleLogFunc.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Batch.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/CPT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/MVT.h"
#include "../SOURCE/FUNCTION/KMS.h"
#include "../SOURCE/FUNCTION/Signpad.h"
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/CARD_FUNC/CardFunction.h"
#include "../SOURCE/COMM/Comm.h"

#include "../SOURCE/FUNCTION/UNIT_FUNC/TimeUint.h"
#include "../SOURCE/FUNCTION/ECR_FUNC/ECR.h"

#include "../SOURCE/KEY/ProcessTmk.h"

#include "../SOURCE/TMS/EDCTmsUnitFunc.h"

#include "../CTLS/CTLS.h"

#include "../EMVSRC/EMVsrc.h"

#include "FUBONfunc.h"
#include "FUBONiso.h"
#include "FUBONtsam.h"
#include "FUBONencryptFunc.h"
#include "FUBONTablePack.h"


extern VS_BOOL fStartCallBank;
extern int inType;
extern int inLoadHostData(int inHostRecNo);
extern int ginDebug; /* Debug使用 extern */
extern int ginISODebug;


VS_BOOL fFubonField_2 = VS_FALSE, fFubonField_35 = VS_FALSE, fFubonField_55 = VS_FALSE;

char szFUBON_ISO_Field03[_FUBON_PCODE_SIZE_ + 1];
char szFESDebug[2];
//extern char szFESDebug[2];

ISO_FIELD_FUBON_TABLE srFUBON_ISOFieldPack[] ={
	{2, inFUBON_ISO_Pack02},
	{3, inFUBON_ISO_Pack03},
	{4, inFUBON_ISO_Pack04},
	{11, inFUBON_ISO_Pack11},
	{12, inFUBON_ISO_Pack12},
	{13, inFUBON_ISO_Pack13},
	{14, inFUBON_ISO_Pack14},
	{22, inFUBON_ISO_Pack22},
	{24, inFUBON_ISO_Pack24},
	{25, inFUBON_ISO_Pack25},
	{35, inFUBON_ISO_Pack35},
	{37, inFUBON_ISO_Pack37},
	{38, inFUBON_ISO_Pack38},
	{39, inFUBON_ISO_Pack39},
	{41, inFUBON_ISO_Pack41},
	{42, inFUBON_ISO_Pack42},
	{48, inFUBON_ISO_Pack48},
	{52, inFUBON_ISO_Pack52},
	{54, inFUBON_ISO_Pack54},
	{55, inFUBON_ISO_Pack55},
	{56, inFUBON_ISO_Pack56},
	{59, inFUBON_ISO_Pack59},
	{60, inFUBON_ISO_Pack60},
	{62, inFUBON_ISO_Pack62},
	{63, inFUBON_ISO_Pack63},
	{64, inFUBON_ISO_Pack64},
	{0, NULL},
};

ISO_FIELD_FUBON_TABLE srFUBON_ISOFieldPack_TSAM[] ={
	{2, inFUBON_ISO_Pack02_tSAM},
	{3, inFUBON_ISO_Pack03},
	{4, inFUBON_ISO_Pack04},
	{11, inFUBON_ISO_Pack11},
	{12, inFUBON_ISO_Pack12},
	{13, inFUBON_ISO_Pack13},
	{14, inFUBON_ISO_Pack14},
	{22, inFUBON_ISO_Pack22},
	{24, inFUBON_ISO_Pack24},
	{25, inFUBON_ISO_Pack25},
	{35, inFUBON_ISO_Pack35_tSAM},
	{37, inFUBON_ISO_Pack37},
	{38, inFUBON_ISO_Pack38},
	{39, inFUBON_ISO_Pack39},
	{41, inFUBON_ISO_Pack41},
	{42, inFUBON_ISO_Pack42},
	{48, inFUBON_ISO_Pack48},
	{52, inFUBON_ISO_Pack52},
	{54, inFUBON_ISO_Pack54},
	{55, inFUBON_ISO_Pack55_tSAM},
	{56, inFUBON_ISO_Pack56},
	{59, inFUBON_ISO_Pack59},
	{60, inFUBON_ISO_Pack60},
	{62, inFUBON_ISO_Pack62},
	{63, inFUBON_ISO_Pack63},
	{64, inFUBON_ISO_Pack64},
	{0, NULL},
};

ISO_FIELD_FUBON_TABLE srFUBON_ISOFieldPack_Software[] ={
	{2, inFUBON_ISO_Pack02_Software},
	{3, inFUBON_ISO_Pack03},
	{4, inFUBON_ISO_Pack04},
	{11, inFUBON_ISO_Pack11},
	{12, inFUBON_ISO_Pack12},
	{13, inFUBON_ISO_Pack13},
	{14, inFUBON_ISO_Pack14_Software},
	{22, inFUBON_ISO_Pack22},
	{24, inFUBON_ISO_Pack24},
	{25, inFUBON_ISO_Pack25_Software},
	{35, inFUBON_ISO_Pack35_Software},
	{37, inFUBON_ISO_Pack37},
	{38, inFUBON_ISO_Pack38},
	{39, inFUBON_ISO_Pack39},
	{41, inFUBON_ISO_Pack41},
	{42, inFUBON_ISO_Pack42},
	{48, inFUBON_ISO_Pack48},
	{52, inFUBON_ISO_Pack52},
	{54, inFUBON_ISO_Pack54},
	{55, inFUBON_ISO_Pack55_Software},
	{56, inFUBON_ISO_Pack56},
	{59, inFUBON_ISO_Pack59},
	{60, inFUBON_ISO_Pack60},
	{62, inFUBON_ISO_Pack62},
	{63, inFUBON_ISO_Pack63},
	{64, inFUBON_ISO_Pack64},
	{0, NULL},
};

ISO_FIELD_FUBON_TABLE srFUBON_ISOFieldUnPack[] ={
	{12, inFUBON_ISO_UnPack12},
	{13, inFUBON_ISO_UnPack13},
	{24, inFUBON_ISO_UnPack24},
	{37, inFUBON_ISO_UnPack37},
	{38, inFUBON_ISO_UnPack38},
	{39, inFUBON_ISO_UnPack39},
	{55, inFUBON_ISO_UnPack55},
	{56, inFUBON_ISO_UnPack56},
	{59, inFUBON_ISO_UnPack59},
	{63, inFUBON_ISO_UnPack63},
	{0, NULL},
};

ISO_CHECK_FUBON_TABLE srFUBON_ISOFieldCheck[] ={
	{3, inFUBON_ISO_Check03},
	{4, inFUBON_ISO_Check04},
	{41, inFUBON_ISO_Check41},
	{0, NULL},
};

ISO_FIELD_TYPE_FUBON_TABLE srFUBON_ISOFieldType[] ={
	{2, _FUBON_ISO_NIBBLE_2_, VS_FALSE, 0},
	{3, _FUBON_ISO_BCD_, VS_FALSE, _FUBON_PCODE_SIZE_ * 2},
	{4, _FUBON_ISO_BCD_, VS_FALSE, 12},
	{11, _FUBON_ISO_BCD_, VS_FALSE, 6},
	{12, _FUBON_ISO_BCD_, VS_FALSE, _TIME_SIZE_},
	{13, _FUBON_ISO_BCD_, VS_FALSE, 4},
	{14, _FUBON_ISO_BCD_, VS_FALSE, 4},
	{22, _FUBON_ISO_BCD_, VS_FALSE, 4},
	{24, _FUBON_ISO_BCD_, VS_FALSE, 4},
	{25, _FUBON_ISO_BCD_, VS_FALSE, 2},
	{27, _FUBON_ISO_BCD_, VS_FALSE, 2},
	{35, _FUBON_ISO_NIBBLE_2_, VS_FALSE, 0},
	{37, _FUBON_ISO_ASC_, VS_FALSE, _FUBON_RRN_SIZE_},
	{38, _FUBON_ISO_ASC_, VS_FALSE, _AUTH_CODE_SIZE_},
	{39, _FUBON_ISO_ASC_, VS_FALSE, 2},
	{41, _FUBON_ISO_ASC_, VS_FALSE, 8},
	{42, _FUBON_ISO_ASC_, VS_FALSE, 15},
	{48, _FUBON_ISO_BYTE_3_, VS_TRUE, 0},
	{52, _FUBON_ISO_BCD_, VS_FALSE, 16},
	{54, _FUBON_ISO_BYTE_3_, VS_TRUE, 0},
	{55, _FUBON_ISO_BYTE_3_, VS_FALSE, 0},
	{56, _FUBON_ISO_BYTE_3_, VS_FALSE, 0},
	{57, _FUBON_ISO_BYTE_3_, VS_FALSE, 0},
	{58, _FUBON_ISO_BYTE_3_, VS_TRUE, 0},
	{59, _FUBON_ISO_BYTE_3_, VS_FALSE, 0},
	{60, _FUBON_ISO_BYTE_3_, VS_TRUE, 0},
	{61, _FUBON_ISO_BYTE_3_, VS_TRUE, 0},
	{62, _FUBON_ISO_BYTE_3_, VS_TRUE, 0},
	{63, _FUBON_ISO_BYTE_3_, VS_TRUE, 0},
	{64, _FUBON_ISO_BCD_, VS_FALSE, 16},
	{0, _FUBON_ISO_BCD_, VS_FALSE, 0},
};


int inFUBON_SALE_[] = {3, 4, 11, 22, 24, 25, 35, 41, 42, 59, 62, 0};
int inFUBON_INST_SALE_[] = {3, 4, 11, 22, 24, 25, 35, 41, 42, 59, 62, 63, 0};
int inFUBON_REDEEM_SALE_[] = {3, 4, 11, 22, 24, 25, 35, 41, 42, 59, 62, 63, 0};
int inFUBON_REFUND_[] = {3, 4, 11, 22, 24, 25, 35, 38, 41, 42, 59, 62, 0};
int inFUBON_INST_REFUND_[] = {3, 4, 11, 22, 24, 25, 35, 38, 41, 42, 59, 62, 63, 0};
int inFUBON_REDEEM_REFUND_[] = {3, 4, 11, 22, 24, 25, 35, 38, 41, 42, 59, 62, 63, 0};
int inFUBON_SALE_ADJUST[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 38, 41, 42, 59, 60, 62, 0};
int inFUBON_REDEEM_ADJUST[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 38, 41, 42, 59, 62, 63, 0};
int inFUBON_INST_ADJUST[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 38, 41, 42, 59, 62, 63, 0};
int inFUBON_PRE_AUTH[] = {3, 4, 11, 22, 24, 25, 35, 41, 42, 59, 62, 0};
int inFUBON_PRE_COMP[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 41, 42, 59, 62, 0};
int inFUBON_ICCTcUpload[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 37, 38, 41, 42, 55, 59, 62, 0};
int inFUBON_SEND_ADVICE[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 38, 41, 42, 59, 62, 0};
int inFUBON_REVERSAL[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 41, 42, 59, 62, 0};
int inFUBON_ISR_UPLOAD[] = {3, 4, 11, 22, 24, 25, 35, 41, 42, 55, 59, 62, 0};
int inFUBON_SETTLE[] = {3, 11, 24, 41, 42, 59, 60, 63, 0};
int inFUBON_CLS_BATCH[] = {3, 11, 24, 41, 42, 60, 63, 0};
int inFUBON_BATCH_UPLOAD[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 37, 38, 39, 41, 42, 59, 60, 62, 0};
int inFUBON_VOID[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 37, 38, 41, 42, 59, 62, 0};
int inFUBON_SALE_TIP[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 38, 41, 42, 48, 54, 59, 60, 62, 0};
int inFUBON_SALE_OFFLINE[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 38, 41, 42, 59, 62, 0};
int inFUBON_CUP_LOGON[] = {3, 11, 24, 25, 41, 42, 59, 0};
int inFUBON_CUP_CHANG_TPK[] = {3, 11, 24, 41, 42, 59, 0};
int inFUBON_CUP_CHANG_TMK[] = {3, 11, 24, 41, 42, 59, 0};
int inFUBON_CUP_SALE[] = {3, 4, 11, 22, 24, 25, 35, 41, 42, 59, 62, 0};
int inFUBON_CUP_REFUND_[] = {3, 4, 11, 22, 24, 25, 35, 38, 41, 42, 59, 62, 0};
int inFUBON_CUP_PRE_AUTH[] = {3, 4, 11, 22, 24, 25, 35, 41, 42, 59, 62, 0};
int inFUBON_CUP_PRE_COMP[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 38, 41, 42, 59, 62, 0};
int inFUBON_CUP_VOID[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 41, 42, 59, 62, 0};
int inFUBON_CUP_PRE_COMP_VOID[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 41, 42, 59, 62, 0};
int inFUBON_CUP_PRE_AUTH_VOID[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 41, 42, 59, 62, 0};

BIT_MAP_FUBON_TABLE srFUBON_ISOBitMap[] ={
	{_SALE_, inFUBON_SALE_, "0200", "000000"},
	{_INST_SALE_, inFUBON_INST_SALE_, "0200", "000000"},
	{_REDEEM_SALE_, inFUBON_REDEEM_SALE_, "0200", "000000"},
	{_REFUND_, inFUBON_REFUND_, "0200", "200000"},
	{_INST_REFUND_, inFUBON_INST_REFUND_, "0200", "200000"},
	{_REDEEM_REFUND_, inFUBON_REDEEM_REFUND_, "0200", "200000"},
	{_VOID_, inFUBON_VOID, "0200", "020000"},
	{_TIP_, inFUBON_SALE_TIP, "0220", "020000"},
	{_ADJUST_, inFUBON_SALE_ADJUST, "0220", "020000"},
	{_INST_ADJUST_, inFUBON_INST_ADJUST, "0200", "700000"},
	{_REDEEM_ADJUST_, inFUBON_REDEEM_ADJUST, "0200", "700000"},
	{_SALE_OFFLINE_, inFUBON_SALE_OFFLINE, "0220", "000000"},
	{_PRE_AUTH_, inFUBON_PRE_AUTH, "0100", "380000"},
	{_PRE_COMP_, inFUBON_PRE_COMP, "0200", "800000"},
	{_TC_UPLOAD_, inFUBON_ICCTcUpload, "0220", "060000"},
	{_ADVICE_, inFUBON_SEND_ADVICE, "0220", "000000"},
	{_REVERSAL_, inFUBON_REVERSAL, "0400", "000000"},
	{_ISR_UPLOAD_, inFUBON_ISR_UPLOAD, "0400", "060000"},
	{_CUP_LOGON_, inFUBON_CUP_LOGON, "0800", "920000"},
	{_CHANGE_TMK_, inFUBON_CUP_CHANG_TMK, "0800", "510000"},
	{_CUP_CHANG_TPK_, inFUBON_CUP_CHANG_TPK, "0800", "500000"},
	{_CUP_SALE_, inFUBON_CUP_SALE, "0200", "000000"},
	{_CUP_REFUND_, inFUBON_CUP_REFUND_, "0200", "200000"},
	{_CUP_PRE_AUTH_, inFUBON_CUP_PRE_AUTH, "0100", "300000"},
	{_CUP_PRE_COMP_, inFUBON_CUP_PRE_COMP, "0200", "400000"},
	{_CUP_VOID_, inFUBON_CUP_VOID, "0200", "020000"},
	{_CUP_PRE_AUTH_VOID_, inFUBON_CUP_PRE_AUTH_VOID, "0100", "320000"},
	{_CUP_PRE_COMP_VOID_, inFUBON_CUP_PRE_COMP_VOID, "0200", "420000"},
	{_SETTLE_, inFUBON_SETTLE, "0500", "920000"},
	{_CLS_BATCH_, inFUBON_CLS_BATCH, "0500", "960000"},
	{_BATCH_UPLOAD_, inFUBON_BATCH_UPLOAD, "0320", "000000"},
	{_TRANS_TYPE_NULL_, NULL, "0000", "000000"},
};

ISO_TYPE_FUBON_TABLE srFUBON_ISOFunc[] ={
	{
		srFUBON_ISOFieldPack,
		srFUBON_ISOFieldUnPack,
		srFUBON_ISOFieldCheck,
		srFUBON_ISOFieldType,
		srFUBON_ISOBitMap,
		inFUBON_ISO_ISOGetBitMapCode,
		inFUBON_ISO_ISOPackMTI,
		vdFUBON_ISO_ISOModifyBitMap,
		vdFUBON_ISO_ISOModifyPackData,
		inFUBON_ISO_ISOCheckHeader,
		inFUBON_ISO_ISOOnlineAnalyse,
		inFUBON_ISO_ISOAdviceAnalyse
	},
	{
		srFUBON_ISOFieldPack_TSAM,
		srFUBON_ISOFieldUnPack,
		srFUBON_ISOFieldCheck,
		srFUBON_ISOFieldType,
		srFUBON_ISOBitMap,
		inFUBON_ISO_ISOGetBitMapCode,
		inFUBON_ISO_ISOPackMTI,
		vdFUBON_ISO_ISOModifyBitMap,
		vdFUBON_ISO_ISOModifyPackData,
		inFUBON_ISO_ISOCheckHeader,
		inFUBON_ISO_ISOOnlineAnalyse,
		inFUBON_ISO_ISOAdviceAnalyse
	},
	{
		srFUBON_ISOFieldPack_Software,
		srFUBON_ISOFieldUnPack,
		srFUBON_ISOFieldCheck,
		srFUBON_ISOFieldType,
		srFUBON_ISOBitMap,
		inFUBON_ISO_ISOGetBitMapCode,
		inFUBON_ISO_ISOPackMTI,
		vdFUBON_ISO_ISOModifyBitMap,
		vdFUBON_ISO_ISOModifyPackData,
		inFUBON_ISO_ISOCheckHeader,
		inFUBON_ISO_ISOOnlineAnalyse,
		inFUBON_ISO_ISOAdviceAnalyse
	},
};

/*
 * App Name		: inFUBON_ISO_CUP_FuncAutoLogon
 * App Builder		: SAM
 * App Date&Time	: 2022/8/3 下午 4:41
 * App Function		: 
 * Input Param		: TRANSACTION_OBJECT *pobTran
 * Output Param		: 
 * 1. -> VS_SUCCESS
 * 2. -> VS_ERROR
 */
int inFUBON_ISO_CUP_FuncAutoLogon(TRANSACTION_OBJECT *pobTran)
{

	int inRetVal;
	int inRetry = VS_TRUE;
	char szLogOn[2];
	char TmkOk[1 + 1];
	char szFuncEnable[1 + 1];
	char szCommmode[1 + 1] = {0};
	char szDemoMode[2 + 1] = {0};

	inDISP_LogPrintfWithFlag("----[%s] RunOpId[%d] Line[%d] START----", __FUNCTION__, pobTran->inRunOperationID, __LINE__);

	/* DEMO 不需要安全認證，需跳過  20190123 [SAM]*/
	memset(szDemoMode, 0x00, sizeof (szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		inSetCUPTMKOK("0");
		inSetCUPLOGONOK("0");
		inSaveEDCRec(0);
		inDISP_LogPrintfWithFlag("----[%s] Demo[%d] Line[%d] END----", __FUNCTION__, pobTran->inRunOperationID, __LINE__);
		return (VS_SUCCESS);
	}

	/* 如果不是開機流程，要先詢問 */
	if (pobTran->inRunOperationID != _OPERATION_EDC_BOOTING_ &&
		pobTran->inRunOperationID != _OPERATION_SETTLE_ &&
		pobTran->inRunOperationID != _OPERATION_CUP_LOGON_)
	{
		inRetVal = inFUBON_UserCehckLogon(pobTran);
		if (inRetVal == VS_USER_CANCEL || inRetVal == VS_TIMEOUT)
			return (inRetVal);
	} else
	{
		inDISP_LogPrintfWithFlag("inFUBON_ISO_CUP_FuncAutoLogon() 1!!");
		pobTran->srBRec.uszOfflineBit = VS_FALSE;
	}



	pobTran->srBRec.inHDTIndex = 0;
	inLoadHDPTRec(0);
	inGetTMSOK(szLogOn);

	if (szLogOn[0] == '0')
	{
		inDISP_Msg_BMP(_ERR_TMS_DWL_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		return (VS_SUCCESS);
	}

	inGetCUPFuncEnable(szFuncEnable);
	if (szFuncEnable[0] != 'Y')
	{
		inDISP_LogPrintfWithFlag("inFUBON_ISO_CUP_FuncAutoLogon CUP FuncEnable = off");
		return (VS_SUCCESS);
	}
	
	/* 這個條件要看一下，目前只要執行就會固定重讀HDT，如果有特別需要再個別進行調整 */
	inDISP_LogPrintfWithFlag("--inCUP_LOGON_Process inLoadHDTRec(0)--");
	if (inLoadHDTRec(0) < 0)
	{
		inDISP_DispLogAndWriteFlie(" FU CUP Log On Load HDT[%d] *Error* Line[%d]", 0, __LINE__);
		return (VS_ERROR);
	}

	if (inLoadCDTRec(0) < 0)
	{
		inDISP_DispLogAndWriteFlie(" FU CUP Log On Load CDT[%d] *Error* Line[%d]", 0, __LINE__);
		return (VS_ERROR);
	}
	
	inGetCommMode(szCommmode);


	/* 因為剛啟動 LOGON時,可能沒有抓到時間，所以在執行LOG ON前重抓一次時間 2022/7/15 上午 11:22 */
	//add by green 191226 先get年 否則logon後因為沒有get過 更新時間會造成時間被改為2000
	{
		RTC_NEXSYS srRTC;

		memset(&srRTC, 0x00, sizeof (RTC_NEXSYS));
		if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
			return (VS_ERROR);

		inFunc_Sync_BRec_Date_Time(pobTran, &srRTC);
	}


	while (1)
	{
		//Add by li for check tmk download status
		inGetTMKOK(TmkOk);
		//end
		inDISP_LogPrintfWithFlag("inCUP_LOGON_Process _CHANGE_TMK_ START----------");
		if (TmkOk[0] != 0x31)
		{
			/* 安全認證 */
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_LOGON_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第一層顯示 <安全認證> */
			pobTran->srBRec.inCode = _CHANGE_TMK_;
			pobTran->inTransactionCode = _CHANGE_TMK_;
			pobTran->inISOTxnCode = _CHANGE_TMK_;

			if ((inRetVal = inFUBON_ISO_BuildAndSendPacket(pobTran)) != VS_SUCCESS)
			{
				inSetCUPTMKOK("0");
				inSetCUPLOGONOK("0");
				inSaveEDCRec(0);
				inCOMM_End(pobTran);

				if (memcmp(szCommmode, _COMM_MODEM_MODE_, 1) != 0)
					CTOS_Delay(1500);

				inRetVal = inFUBON_UserCehckLogon(pobTran);

				/* 抓取 KioskFlag的值，如為 TRUE 就不用確認畫面 2020/3/6 下午 4:28 [SAM] */
				if (inFunc_GetKisokFlag() == VS_TRUE)
				{
					if (inRetry == VS_FALSE)
						return (VS_TIMEOUT);

					inRetry = VS_FALSE;
					inDISP_LogPrintfWithFlag(" FU Kiosk Cup Logon Line[%d]", __LINE__);

				} else
				{

					inDISP_LogPrintfWithFlag(" FU UserCehckLogon[%d]", inRetVal);
					if (inRetVal == VS_USER_CANCEL || inRetVal == VS_TIMEOUT)
					{
						if (pobTran->inRunOperationID == _OPERATION_SETTLE_)
						{
							pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
							return (VS_SUCCESS);
						}
						
						return (inRetVal);
					}
				}
				continue;
			} else
			{
				inSetCUPTMKOK("1");
				inSaveEDCRec(0);
			}
		}

		inDISP_PutGraphic(_MENU_LOGON_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第一層顯示 <安全認證> */
		pobTran->srBRec.inCode = _CUP_CHANG_TPK_;
		pobTran->inTransactionCode = _CUP_CHANG_TPK_;
		pobTran->inISOTxnCode = _CUP_CHANG_TPK_;

		if ((inRetVal = inFUBON_ISO_BuildAndSendPacket(pobTran)) != VS_SUCCESS)
		{
			inSetCUPTMKOK("0");
			inSetCUPTPKOK("0");
			inSetCUPLOGONOK("0");
			inSaveEDCRec(0);
			inCOMM_End(pobTran);

			if (memcmp(szCommmode, _COMM_MODEM_MODE_, 1) != 0)
				CTOS_Delay(1500);

			inRetVal = inFUBON_UserCehckLogon(pobTran);

			inDISP_LogPrintfWithFlag("XXX inCUP_LOGON_ISO_BuildAndSendPacket Err XXX,%d", inRetVal);
			if (inRetVal == VS_USER_CANCEL || inRetVal == VS_TIMEOUT)
			{
				if (pobTran->inRunOperationID == _OPERATION_SETTLE_)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
					return (VS_SUCCESS);
				}

				return (inRetVal);
			}
			continue;
		} else
		{
			inSetCUPTPKOK("1");
			inSetCUPLOGONOK("1");
			inSaveEDCRec(0);
			break;
		}
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	inCOMM_End(pobTran);
	return (inRetVal);

}

/*
 * App Name		: inFUBON_ISO_PackSendUnPackReceData
 * App Builder		: 
 * App Date&Time	: 2022/8/3 下午 4:49
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 * 	1. -> VS_SUCCESS
 * 	2. -> VS_ERROR
 */
int inFUBON_ISO_PackSendUnPackReceData(TRANSACTION_OBJECT *pobTran, int inTxnCode)
{
	int inSendLen = 0, inReceLen = 0, inRetVal;
	char szSendPacket[_FUBON_ISO_SEND_ + 1], szRecePacket[_FUBON_ISO_RECV_ + 1];

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inFUBON_ISO_PackSendUnPackReceData INIT");

	memset(szSendPacket, 0x00, sizeof (szSendPacket));
	memset(szRecePacket, 0x00, sizeof (szRecePacket));

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag("  inTxnCode [%d]", inTxnCode);

	if ((inSendLen = inFUBON_ISO_PackISO(pobTran, (unsigned char *) szSendPacket, inTxnCode)) <= 0)
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

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "Aft inFUBON_ISO_PackISO");

	if ((inReceLen = inFUBON_ISO_SendReceData(pobTran,
			(unsigned char *) szSendPacket,
			inSendLen,
			(unsigned char *) szRecePacket)) != VS_SUCCESS)
	{
		if (pobTran->inISOTxnCode != _TC_UPLOAD_ || pobTran->inISOTxnCode != _ADVICE_)
			memset(pobTran->srBRec.szRespCode, 0x00, sizeof (pobTran->srBRec.szRespCode));
		pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_RECV_;
		inDISP_LogPrintfWithFlag(" Fubon SendRece *Error* ReceLen[%d]", inReceLen);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Iso Send *Error* END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "Aft inFUBON_ISO_SendReceData");

	inRetVal = inFUBON_ISO_UnPackISO(pobTran,
			(unsigned char *) szSendPacket,
			inSendLen,
			(unsigned char *) szRecePacket,
			inReceLen);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_LogPrintfWithFlag(" Fubon UnPack *Error* inRetVal[%d]", inRetVal);
		inDISP_Msg_BMP(_ERR_ISO_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		inRetVal = VS_ISO_UNPACK_ERROR;
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inFUBON_ISO_PackSendUnPackReceData END");
	
	inDISP_LogPrintfWithFlag(" Return  inRetVal[%d] ", inRetVal);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
 * App Name		: inFUBON_ISO_PackISO
 * App Builder		: 
 * App Date&Time	: 2022/8/3 下午 4:52
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 * 
 */
int inFUBON_ISO_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode)
{
	int i, inSendLen, inField, inCnt;
	int inBitMap[_FUBON_MAX_BIT_MAP_CNT_];
	int inRetVal, inISOFuncIndex = -1;
	unsigned char bBuf[30];
	ISO_TYPE_FUBON_TABLE srISOFunc;
	char szCommmode[1 + 1] = {0}, szEncryptMode[1 + 1] = {0};

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	inGetCommMode(szCommmode);
	inGetEncryptMode(szEncryptMode);
	
	inDISP_LogPrintfWithFlag(" Fubon ISO Pack COMM_MODE[%02x] ENCRYPT_MODE[%02x]",
			szCommmode[0], szEncryptMode[0]);

	fFubonField_35 = VS_FALSE;
	fFubonField_55 = VS_FALSE;
	inISOFuncIndex = szEncryptMode[0] - '0';

	if (inISOFuncIndex >= 4 || inISOFuncIndex < 0)
	{
		inDISP_Msg_BMP(_ERR_ISO_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		inDISP_LockEdcDispLogAndWriteFlie(" Fubon ISO Index[%d] *Error* ", inISOFuncIndex);
		return (VS_ERROR);
	}

	inSendLen = 0;
	inField = 0;
	/* 決定要執行第幾個 Function Index */
	memset((char *) &srISOFunc, 0x00, sizeof (srISOFunc));
	memcpy((char *) &srISOFunc, (char *) &srFUBON_ISOFunc[inISOFuncIndex], sizeof (srISOFunc));
	memset((char *) inBitMap, 0x00, sizeof (inBitMap));

	/* 開始組 ISO 電文 */
	/* Pack TPDU */
	memset((char *) bBuf, 0x00, sizeof (bBuf));
	inGetTPDU((char *) bBuf);
	inFunc_ASCII_to_BCD(&uszSendBuf[inSendLen], (char *) bBuf, _FUBON_TPDU_SIZE_); /* 5 BYTE */
	inSendLen += _FUBON_TPDU_SIZE_;

	inRetVal = inFUBON_ISO_GetBitMap(pobTran, inTxnCode, &srISOFunc, inBitMap, &uszSendBuf[inSendLen]);
	if (inRetVal <= VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" Fubon ISO GetBitMap[%d] *Error* ", inTxnCode);
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
			inCnt = srISOFunc.srPackISO[inField].inISOLoad(pobTran, &uszSendBuf[inSendLen]);
			if (inCnt <= 0)
			{
				inDISP_DispLogAndWriteFlie(" Fubon ISO FieldNum[%d] Field[%d] *Error* ", srISOFunc.srPackISO[inField].inFieldNum, inField);

				if (srISOFunc.srPackISO[inField].inFieldNum == 35 || srISOFunc.srPackISO[inField].inFieldNum == 55)
				{
					inDISP_DispLogAndWriteFlie(" Fubon inBitMap[%d] inCnt[%d] *Error* ", inBitMap[i], inCnt);
					inDISP_Msg_BMP(_ERR_READ_CARD_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
				}
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
 * App Name		: inFUBON_ISO_UnPackISO
 * App Builder		: 
 * App Date&Time	: 2022/8/3 下午 4:55
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_UnPackISO(TRANSACTION_OBJECT *pobTran,
		unsigned char *uszSendBuf,
		int inSendLen,
		unsigned char *uszReceBuf,
		int inReceLen)
{
	int i, inReceCnt, inCnt, inSendCnt, inSendField, inReceField;
	char szBuf[_FUBON_TPDU_SIZE_ + _FUBON_MTI_SIZE_ + _FUBON_BIT_MAP_SIZE_ + 1];
	unsigned char uszSendMap[_FUBON_BIT_MAP_SIZE_ + 1], uszReceMap[_FUBON_BIT_MAP_SIZE_ + 1];
	ISO_TYPE_FUBON_TABLE srISOFunc;
	char szEncryptMode[1 + 1] = {0};

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, " inFUBON_ISO_UnPackISO INIT");

	inDISP_LogPrintfWithFlag(" Receive_Data_Len[%d] inISOTxnCode[%d]", inReceLen, pobTran->inISOTxnCode);

	inSendField = inReceField = 0;
	inSendCnt = inReceCnt = 0;
	memset((char *) &srISOFunc, 0x00, sizeof (srISOFunc));
	memset((char *) uszSendMap, 0x00, sizeof (uszSendMap));
	memset((char *) uszReceMap, 0x00, sizeof (uszReceMap));
	memset((char *) szBuf, 0x00, sizeof (szBuf));

	inGetEncryptMode(szEncryptMode);
	inDISP_LogPrintfWithFlag(" szEncryptMode[%s] ", szEncryptMode);
	
	memcpy((char *) &srISOFunc, (char *) &srFUBON_ISOFunc[szEncryptMode[0] - '0'], sizeof (srISOFunc));

	if (srISOFunc.inCheckISOHeader != NULL)
	{
		if (srISOFunc.inCheckISOHeader(pobTran, (char *) &uszSendBuf[inReceCnt], (char *) &uszReceBuf[inReceCnt]) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" FU CheckIsoHeader *Error* Line[%d]", __LINE__);
			return (VS_ISO_UNPACK_ERROR);
		}
	}

	inSendCnt += _FUBON_TPDU_SIZE_;
	inReceCnt += _FUBON_TPDU_SIZE_;
	inSendCnt += _FUBON_MTI_SIZE_;
	inReceCnt += _FUBON_MTI_SIZE_;

	memcpy((char *) uszSendMap, (char *) &uszSendBuf[inSendCnt], _FUBON_BIT_MAP_SIZE_);
	memcpy((char *) uszReceMap, (char *) &uszReceBuf[inReceCnt], _FUBON_BIT_MAP_SIZE_);

	inSendCnt += _FUBON_BIT_MAP_SIZE_;
	inReceCnt += _FUBON_BIT_MAP_SIZE_;


	if (!inFUBON_BitMapCheck(uszReceMap, 39))
	{
		inDISP_DispLogAndWriteFlie(" FU CheckIsoHeader Check 39 *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	if (!inFUBON_BitMapCheck(uszReceMap, 41))
	{
		inDISP_DispLogAndWriteFlie(" FU CheckIsoHeader Check 41 *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	for (i = 1; i <= 64; i++)
	{
		if (inFUBON_BitMapCheck(uszSendMap, i) && !inFUBON_BitMapCheck(uszReceMap, i))
		{
			inSendCnt += inFUBON_ISO_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
		} else if (inFUBON_BitMapCheck(uszReceMap, i))
		{
			if (inFUBON_BitMapCheck(uszSendMap, i))
			{
				if ((inSendField = inFUBON_ISO_GetCheckField(i, srISOFunc.srCheckISO)) != VS_ERROR)
				{
					if (srISOFunc.srCheckISO[inSendField].inISOCheck(pobTran, &uszSendBuf[inSendCnt], &uszReceBuf[inReceCnt]) != VS_SUCCESS)
					{
						inDISP_DispLogAndWriteFlie(" FU ISOCHECK *Error* SendId[%d] FieldNum[%d]  ", inSendField, srISOFunc.srCheckISO[inSendField].inFieldNum);
						return (VS_ISO_UNPACK_ERROR);
					}
				}

				inSendCnt += inFUBON_ISO_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
			}

			while (i > srISOFunc.srUnPackISO[inReceField].inFieldNum)
			{
				inReceField++;
			}

			if (i == srISOFunc.srUnPackISO[inReceField].inFieldNum)
			{
				if (inFUBON_ISO_CheckUnPackField(i, srISOFunc.srUnPackISO) == VS_SUCCESS)
				{
					if (srISOFunc.srUnPackISO[inReceField].inISOLoad(pobTran, &uszReceBuf[inReceCnt]) != VS_SUCCESS)
					{
						inDISP_DispLogAndWriteFlie(" FU UnPackISO Rece_id[%d] *Error* ", inReceField);
						return (VS_ERROR);
					}
				}
			}

			inCnt = inFUBON_ISO_GetFieldLen(i, &uszReceBuf[inReceCnt], srISOFunc.srISOFieldType);
			if (inCnt == VS_ERROR)
			{
				inDISP_DispLogAndWriteFlie(" FU ISO Get Field Len i[%d] *Error", i);
				return (VS_ISO_UNPACK_ERROR);
			}

			inReceCnt += inCnt;
		}
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inFUBON_ISO_UnPackISO Aft 64BIT_WHILE");


	if (!memcmp(&pobTran->srBRec.szRespCode[0], "00", 2))
	{
		switch (pobTran->inISOTxnCode)
		{
			case _CHANGE_TMK_:
			case _CUP_CHANG_TPK_:
				break;
			case _CUP_SALE_:
			case _CUP_PRE_AUTH_:
				if (!inFUBON_BitMapCheck(uszReceMap, 04))
				{
					inDISP_DispLogAndWriteFlie(" FU BitMpCheck_1 04 *Eerror* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode, __LINE__);
					return (VS_ERROR);
				}

				if (!inFUBON_BitMapCheck(uszReceMap, 38))
				{
					inDISP_DispLogAndWriteFlie(" FU BitMpCheck_1 38 *Eerror* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode, __LINE__);
					return (VS_ERROR);
				}
				break;
			case _CUP_REFUND_:
			case _CUP_PRE_COMP_:
				if (!inFUBON_BitMapCheck(uszReceMap, 04))
				{
					inDISP_DispLogAndWriteFlie(" FU BitMpCheck_2 04 *Eerror* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode, __LINE__);
					return (VS_ERROR);
				}

				if (!inFUBON_BitMapCheck(uszReceMap, 38))
				{
					inDISP_DispLogAndWriteFlie(" FU BitMpCheck_1 38 *Eerror* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode, __LINE__);
					return (VS_ERROR);
				}
				break;
			case _CUP_VOID_:
			case _CUP_PRE_AUTH_VOID_:
			case _CUP_PRE_COMP_VOID_:

				if (!inFUBON_BitMapCheck(uszReceMap, 04))
				{
					inDISP_DispLogAndWriteFlie(" FU BitMpCheck_3 04 *Eerror* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode, __LINE__);
					return (VS_ERROR);
				}
				break;
			case _INST_SALE_:
			case _INST_ADJUST_:
			case _INST_REFUND_:
			case _REDEEM_SALE_:
			case _REDEEM_ADJUST_:
			case _REDEEM_REFUND_:
				if (!inFUBON_BitMapCheck(uszReceMap, 04))
				{
					inDISP_DispLogAndWriteFlie(" FU BitMpCheck_4 04 *Eerror* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode, __LINE__);
					return (VS_ERROR);
				}

				if (!inFUBON_BitMapCheck(uszReceMap, 38))
				{
					inDISP_DispLogAndWriteFlie(" FU BitMpCheck_4 38 *Eerror* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode, __LINE__);
					return (VS_ERROR);
				}
				break;
			case _SALE_:
			case _PRE_AUTH_:
			case _PRE_COMP_:
			case _REFUND_:
				if (!inFUBON_BitMapCheck(uszReceMap, 04))
				{
					inDISP_DispLogAndWriteFlie(" FU BitMpCheck_5 04 *Eerror* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode, __LINE__);
					return (VS_ERROR);
				}

				if (!inFUBON_BitMapCheck(uszReceMap, 38))
				{
					inDISP_DispLogAndWriteFlie(" FU BitMpCheck_5 38 *Eerror* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode, __LINE__);
					return (VS_ERROR);
				}
				break;
			case _TIP_:
			case _VOID_:
				if (!inFUBON_BitMapCheck(uszReceMap, 04))
				{
					inDISP_DispLogAndWriteFlie(" FU BitMpCheck_6 04 *Eerror* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode, __LINE__);
					return (VS_ERROR);
				}
				break;
			default:
				break;
		} /* End switch() */
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inFUBON_ISO_UnPackISO END");

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
 * App Name		: inFUBON_ISO_GetBitMap
 * App Builder		: 
 * App Date&Time	: 2022/8/5 上午 11:54
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_GetBitMap(TRANSACTION_OBJECT *pobTran,
		int inTxnType,
		ISO_TYPE_FUBON_TABLE *srISOFuncIndex,
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

	/*  BIT_MAP_FUBON_TABLE srFES_ISOBitMap  inTxnID */
	if ((inBitMapIndex = inFUBON_ISO_GetBMapMTIPCode(pobTran, srISOFuncIndex, inBitMapCode)) == VS_ERROR)
		return (VS_ERROR);

	/* BIT_MAP_FUBON_TABLE srFES_ISOBitMap  BitMap */
	vdFUBON_ISO_CopyISOMap(inTxnBitMap, srISOFuncIndex->srBitMap[inBitMapIndex].inBitMap);

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

	/* Process Code */
	memset(szFUBON_ISO_Field03, 0x00, sizeof (szFUBON_ISO_Field03));
	if (inBitMapCode != _BATCH_UPLOAD_)
	{
		if (inBitMapCode == _REVERSAL_)
		{
			inFunc_ASCII_to_BCD((unsigned char *) &szFUBON_ISO_Field03[0], srISOFuncIndex->srBitMap[inBitMapIndex].szPCode, _FUBON_PCODE_SIZE_);

			if (pobTran->inTransactionCode == _VOID_ ||
				pobTran->inTransactionCode == _CUP_VOID_ ||
				pobTran->inTransactionCode == _CUP_PRE_AUTH_VOID_ ||
				pobTran->inTransactionCode == _CUP_PRE_COMP_VOID_)
			{
				if (pobTran->srBRec.inOrgCode == _REFUND_ || pobTran->srBRec.inOrgCode == _INST_REFUND_ ||
					pobTran->srBRec.inOrgCode == _CUP_REFUND_ || pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
					szFUBON_ISO_Field03[0] = 0x22;
				else if (pobTran->srBRec.inOrgCode == _PRE_AUTH_ || pobTran->srBRec.inOrgCode == _CUP_PRE_AUTH_)
					szFUBON_ISO_Field03[0] = 0x32;
				else if (pobTran->srBRec.inOrgCode == _CUP_PRE_COMP_)
					szFUBON_ISO_Field03[0] = 0x42;
				else if (pobTran->srBRec.inOrgCode == _SALE_ || pobTran->srBRec.inOrgCode == _CUP_SALE_ ||
						pobTran->srBRec.inOrgCode == _INST_SALE_ || pobTran->srBRec.inOrgCode == _REDEEM_SALE_ ||
						pobTran->srBRec.inOrgCode == _SALE_OFFLINE_ || pobTran->srBRec.inOrgCode == _ADJUST_)
					szFUBON_ISO_Field03[0] = 0x02;
				else if (pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_ || pobTran->srBRec.inOrgCode == _INST_ADJUST_)
					szFUBON_ISO_Field03[0] = 0x72;
				else if ( pobTran->srBRec.inOrgCode == _PRE_COMP_)/* [新增預授權完成] 修改判斷條件把 _PRE_COMP_ 獨立出來 2022/11/7 [SAM] */
					szFUBON_ISO_Field03[0] = 0x82;
			} else if (pobTran->inTransactionCode == _REFUND_ || pobTran->inTransactionCode == _CUP_REFUND_ ||
					pobTran->inTransactionCode == _REDEEM_REFUND_ || pobTran->inTransactionCode == _INST_REFUND_)
				szFUBON_ISO_Field03[0] = 0x20;
			else if (pobTran->inTransactionCode == _PRE_AUTH_)
				szFUBON_ISO_Field03[0] = 0x38;
			else if (pobTran->inTransactionCode == _CUP_PRE_AUTH_)
				szFUBON_ISO_Field03[0] = 0x30;
			else if (pobTran->inTransactionCode == _PRE_COMP_)/* [新增預授權完成] 修改判斷條件把 _PRE_COMP_ 獨立出來 2022/5/12 [SAM] */
				szFUBON_ISO_Field03[0] = 0x80;
			else if (pobTran->inTransactionCode == _CUP_PRE_COMP_)
				szFUBON_ISO_Field03[0] = 0x40;
			else if (pobTran->inTransactionCode == _TIP_)
				szFUBON_ISO_Field03[0] = 0x02;
			else if (pobTran->inTransactionCode == _REDEEM_ADJUST_ || pobTran->inTransactionCode == _INST_ADJUST_)
				szFUBON_ISO_Field03[0] = 0x70;
		} else
		{
			inFunc_ASCII_to_BCD((unsigned char *) &szFUBON_ISO_Field03[0], srISOFuncIndex->srBitMap[inBitMapIndex].szPCode, _FUBON_PCODE_SIZE_);

			if (pobTran->inTransactionCode == _VOID_ ||
					pobTran->inTransactionCode == _CUP_VOID_ ||
					pobTran->inTransactionCode == _CUP_PRE_AUTH_VOID_ ||
					pobTran->inTransactionCode == _CUP_PRE_COMP_VOID_)
			{
				if (pobTran->srBRec.inOrgCode == _REFUND_ || pobTran->srBRec.inOrgCode == _INST_REFUND_ ||
						pobTran->srBRec.inOrgCode == _CUP_REFUND_ || pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
					szFUBON_ISO_Field03[0] = 0x22;
				else if (pobTran->srBRec.inOrgCode == _PRE_AUTH_ || pobTran->srBRec.inOrgCode == _CUP_PRE_AUTH_)
					szFUBON_ISO_Field03[0] = 0x32;
				else if (pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_ || pobTran->srBRec.inOrgCode == _INST_ADJUST_)
					szFUBON_ISO_Field03[0] = 0x72;
				else if (pobTran->srBRec.inOrgCode == _PRE_COMP_)/* [新增預授權完成] 修改判斷條件把 _PRE_COMP_ 獨立出來 2022/5/12 [SAM] */
					szFUBON_ISO_Field03[0] = 0x82;
			}
		}
	} else
	{
		if (pobTran->inTransactionCode == _SETTLE_ && (pobTran->srBRec.inOrgCode == _REFUND_ ||
			pobTran->srBRec.inOrgCode == _CUP_REFUND_ ||
			pobTran->srBRec.inOrgCode == _INST_REFUND_ ||
			pobTran->srBRec.inOrgCode == _REDEEM_REFUND_))
			szFUBON_ISO_Field03[0] = 0x20;

		if (pobTran->inTransactionCode == _SETTLE_ && (pobTran->srBRec.inOrgCode == _CUP_PRE_COMP_))
			szFUBON_ISO_Field03[0] = 0x40;

		if (pobTran->inTransactionCode == _SETTLE_ && (pobTran->srBRec.inOrgCode == _TIP_))
		{
			szFUBON_ISO_Field03[0] = 0x00;
			szFUBON_ISO_Field03[1] = 0x00;
		}

		if (pobTran->inTransactionCode == _SETTLE_ && (pobTran->srBRec.inOrgCode == _ADJUST_))
		{
			szFUBON_ISO_Field03[0] = 0x00;
			szFUBON_ISO_Field03[1] = 0x00;
		}

		if (pobTran->inTransactionCode == _SETTLE_ && (pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_ ||
				pobTran->srBRec.inOrgCode == _INST_ADJUST_))
		{
			szFUBON_ISO_Field03[0] = 0x70;
		}

		/* [新增預授權完成] 修改判斷條件把 _PRE_COMP_ 獨立出來 2022/5/12 [SAM] */
		if (pobTran->inTransactionCode == _SETTLE_ && (pobTran->srBRec.inOrgCode == _PRE_COMP_))
			szFUBON_ISO_Field03[0] = 0x80;
	}

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

	inDISP_LogPrintfWithFlag("uszSendBuf 3 = [%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x]!!",
			uszSendBuf[0], uszSendBuf[1], uszSendBuf[2], uszSendBuf[3], uszSendBuf[4],
			uszSendBuf[5], uszSendBuf[6], uszSendBuf[7], uszSendBuf[8], uszSendBuf[9],
			uszSendBuf[10], uszSendBuf[11], uszSendBuf[12], uszSendBuf[13], uszSendBuf[14]);

	inCnt += _FUBON_BIT_MAP_SIZE_;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_ISOGetBitMapCode
 * App Builder		: 
 * App Date&Time	: 2022/8/5 上午 11:55
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_ISOGetBitMapCode(TRANSACTION_OBJECT *pobTran, int inTxnType)
{
	int inBitMapCode = -1;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" inTxnType[%d]", inTxnType);

	switch (inTxnType)
	{
		case _REVERSAL_:
		case _SALE_:
		case _INST_SALE_:
		case _REDEEM_SALE_:
		case _TC_UPLOAD_:
		case _ADVICE_:
		case _REFUND_:
		case _INST_REFUND_:
		case _REDEEM_REFUND_:
		case _VOID_:
		case _TIP_:
		case _ADJUST_:
		case _REDEEM_ADJUST_:
		case _INST_ADJUST_:
		case _SALE_OFFLINE_:
		case _PRE_AUTH_:
		case _PRE_COMP_:
		case _CUP_LOGON_:
		case _CHANGE_TMK_:
		case _CUP_CHANG_TPK_:
		case _CUP_SALE_:
		case _CUP_REFUND_:
		case _CUP_PRE_AUTH_:
		case _CUP_PRE_COMP_:
		case _CUP_VOID_:
		case _CUP_PRE_COMP_VOID_:
		case _CUP_PRE_AUTH_VOID_:
		case _SETTLE_:
		case _CLS_BATCH_:
		case _ISR_UPLOAD_:
		case _BATCH_UPLOAD_:
			inBitMapCode = inTxnType;
			break;
		default:
			inDISP_DispLogAndWriteFlie(" FU GetBitMapCode *Eerror* inTxnType[%d] Line[%d]", inTxnType, __LINE__);
			break;
	}
	
	inDISP_LogPrintfWithFlag(" inBitMapCode[%d]", inBitMapCode);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inBitMapCode);
}

/*
 * App Name		: inFUBON_ISO_ISOPackMTI
 * App Builder		: 
 * App Date&Time	: 2022/8/5 上午 11:56
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_ISOPackMTI(TRANSACTION_OBJECT *pobTran, int inTxnCode, unsigned char *bPackData, char *szMTI)
{
	int inCnt = 0;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" inTxnCode[%d] szMTI[%s]", inTxnCode, szMTI);

	if (inTxnCode == _REVERSAL_)
		memcpy((char *) &bPackData[inCnt], "\x04\x00", _FUBON_MTI_SIZE_);
	else if (inTxnCode == _ADVICE_)
		memcpy((char *) &bPackData[inCnt], "\x02\x20", _FUBON_MTI_SIZE_);
	else
		inFunc_ASCII_to_BCD(&bPackData[inCnt], szMTI, _FUBON_MTI_SIZE_);

	inCnt += _FUBON_MTI_SIZE_;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: vdFUBON_ISO_ISOModifyBitMap
 * App Builder		: 
 * App Date&Time	: 2022/8/5 上午 11:57
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
void vdFUBON_ISO_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inTxnType, int *inBitMap)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" P-inTransactionCode[%d] Br-inCode[%d] P-inISOTxnCode[%d] inTxnType[%d] Br-inOrgCode[%d]", pobTran->inTransactionCode, 
											pobTran->srBRec.inCode, 
											pobTran->inISOTxnCode,
											inTxnType,
											pobTran->srBRec.inOrgCode);

	if (pobTran->inISOTxnCode == _CUP_LOGON_ || pobTran->inISOTxnCode == _SETTLE_ || pobTran->inISOTxnCode == _CLS_BATCH_ ||
			pobTran->inISOTxnCode == _CHANGE_TMK_ || pobTran->inISOTxnCode == _CUP_CHANG_TPK_)
	{
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Not Map Set END -----", __FILE__, __FUNCTION__, __LINE__);
		return;
	}

	if (inTxnType == _REVERSAL_)
	{
		inDISP_LogPrintfWithFlag("vdFUBON_ISO_ISOModifyBitMap inTxnType = _REVERSAL_");
		if (pobTran->srBRec.inCode == _TIP_)
			vdFUBON_ISO_MapSet(inBitMap, 54);

		if (pobTran->srBRec.inCode == _VOID_)
			vdFUBON_ISO_MapSet(inBitMap, 37);

		if (strlen(pobTran->srBRec.szAuthCode) > 0)
			vdFUBON_ISO_MapSet(inBitMap, 38);

		if (pobTran->srBRec.uszRedeemBit == VS_TRUE || pobTran->srBRec.uszInstallmentBit == VS_TRUE)
			vdFUBON_ISO_MapSet(inBitMap, 63);
	}

	if (inTxnType == _ISR_UPLOAD_)
	{
		if (strlen(pobTran->srBRec.szTime) > 0)
			vdFUBON_ISO_MapSet(inBitMap, 12);

		if (strlen(pobTran->srBRec.szDate) > 0)
			vdFUBON_ISO_MapSet(inBitMap, 13);

		if (strlen(pobTran->srBRec.szRefNo) > 0)
			vdFUBON_ISO_MapSet(inBitMap, 37);

		if (strlen(pobTran->srBRec.szAuthCode) > 0)
			vdFUBON_ISO_MapSet(inBitMap, 38);

		if (strlen(pobTran->srBRec.szRespCode) > 0)
			vdFUBON_ISO_MapSet(inBitMap, 39);

	}

	if (strlen(pobTran->srBRec.szStoreID) > 0 && inTxnType != _TIP_)
		vdFUBON_ISO_MapSet(inBitMap, 48);

	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		if (inTxnType == _CUP_SALE_ || inTxnType == _CUP_PRE_AUTH_)
		{
			inDISP_LogPrintfWithFlag(" Fu PinLen[%d] FirstByte[%02x] PinStr[%s]", strlen(pobTran->szPIN), pobTran->szPIN[0], pobTran->szPIN);

			if (pobTran->szPIN[0] != 0x00)
				vdFUBON_ISO_MapSet(inBitMap, 52);
		}
	}

	if (pobTran->inISOTxnCode == _BATCH_UPLOAD_ && pobTran->srBRec.inCode == _TIP_)
		vdFUBON_ISO_MapSet(inBitMap, 54);

	if (pobTran->srBRec.inChipStatus == _EMV_CARD_ || pobTran->srBRec.uszContactlessBit == VS_TRUE)
	{
		if (!memcmp(&pobTran->srBRec.szAuthCode[0], "VLP", 3) ||
				!memcmp(&pobTran->srBRec.szAuthCode[0], "JCB", 3) ||
				pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE)
		{

		} else
		{
			vdFUBON_ISO_MapSet(inBitMap, 55);
		}
	}


	if (pobTran->srBRec.inChipStatus == _EMV_CARD_ &&
			pobTran->srBRec.inCode != _CUP_VOID_ &&
			pobTran->srBRec.inCode != _CUP_PRE_AUTH_VOID_)
	{
		vdFUBON_ISO_MapSet(inBitMap, 55);
	}

	if (pobTran->srBRec.inCode == _CUP_VOID_ ||
			pobTran->srBRec.inCode == _CUP_PRE_AUTH_VOID_)
	{
		vdFUBON_ISO_MapReset(inBitMap, 55);
	}


	if (pobTran->srBRec.uszRedeemBit == VS_TRUE || pobTran->srBRec.uszInstallmentBit == VS_TRUE)
		vdFUBON_ISO_MapSet(inBitMap, 63);

	if (pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
	{
		vdFUBON_ISO_MapReset(inBitMap, 55);
	}

	if ((pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _CUP_REFUND_ ||
			(pobTran->srBRec.inOrgCode == _REFUND_ && pobTran->srBRec.inCode == _VOID_)) &&
			(fGetCntlessFuncEnable() == VS_TRUE))
	{
		inDISP_LogPrintfWithFlag("vdFUBON_ISO_ISOModifyBitMap inCode = _REFUND_ ,inCode = _CUP_REFUND_");
		vdFUBON_ISO_MapReset(inBitMap, 55);
	}

	if (pobTran->srBRec.uszManualBit == VS_TRUE)
	{
		/*vdFUBON_ISO_MapSet(inBitMap, 2);
		vdFUBON_ISO_MapSet(inBitMap, 14);
		vdFUBON_ISO_MapReset(inBitMap, 35);*/
		vdFUBON_ISO_MapReset(inBitMap, 55);
	}

	if (!memcmp(szGetCustomerIndicator(), _CUSTOMER_002_DREAM_MALL_, 3)
			&& (pobTran->inISOTxnCode == _SALE_
			|| pobTran->inISOTxnCode == _REDEEM_SALE_
			|| pobTran->inISOTxnCode == _INST_SALE_
			|| pobTran->inISOTxnCode == _PRE_AUTH_
			|| pobTran->inISOTxnCode == _TIP_
			|| pobTran->inISOTxnCode == _SALE_OFFLINE_
			|| (pobTran->inISOTxnCode == _BATCH_UPLOAD_ && pobTran->srBRec.inCode == _SALE_)
			|| (pobTran->inISOTxnCode == _BATCH_UPLOAD_ && pobTran->srBRec.inCode == _TIP_)
			|| (pobTran->inISOTxnCode == _BATCH_UPLOAD_ && pobTran->srBRec.inCode == _SALE_OFFLINE_)
			|| (pobTran->inISOTxnCode == _BATCH_UPLOAD_ && pobTran->srBRec.inCode == _REDEEM_SALE_)
			|| (pobTran->inISOTxnCode == _BATCH_UPLOAD_ && pobTran->srBRec.inCode == _INST_SALE_)
			|| pobTran->inISOTxnCode == _TC_UPLOAD_))
	{
		vdFUBON_ISO_MapSet(inBitMap, 56);
	}

	if (!memcmp(szGetCustomerIndicator(), _CUSTOMER_003_HANKYU_, 3)
			&& (pobTran->inISOTxnCode == _SALE_
			|| pobTran->inISOTxnCode == _REDEEM_SALE_
			|| pobTran->inISOTxnCode == _INST_SALE_
			|| pobTran->inISOTxnCode == _PRE_AUTH_
			|| pobTran->inISOTxnCode == _TIP_
			|| pobTran->inISOTxnCode == _SALE_OFFLINE_
			|| (pobTran->inISOTxnCode == _BATCH_UPLOAD_ && pobTran->srBRec.inCode == _SALE_)
			|| (pobTran->inISOTxnCode == _BATCH_UPLOAD_ && pobTran->srBRec.inCode == _TIP_)
			|| (pobTran->inISOTxnCode == _BATCH_UPLOAD_ && pobTran->srBRec.inCode == _SALE_OFFLINE_)
			|| (pobTran->inISOTxnCode == _BATCH_UPLOAD_ && pobTran->srBRec.inCode == _REDEEM_SALE_)
			|| (pobTran->inISOTxnCode == _BATCH_UPLOAD_ && pobTran->srBRec.inCode == _INST_SALE_)
			|| pobTran->inISOTxnCode == _TC_UPLOAD_))
	{
		vdFUBON_ISO_MapSet(inBitMap, 56);
	}

	if (pobTran->inISOTxnCode == _VOID_)
	{
		if (strlen(pobTran->srBRec.szRefNo) > 0)
			vdFUBON_ISO_MapSet(inBitMap, 37);
	}

	if (pobTran->inISOTxnCode == _CUP_VOID_ || pobTran->inISOTxnCode == _CUP_PRE_AUTH_VOID_ || pobTran->inISOTxnCode == _CUP_PRE_COMP_VOID_)
	{
		vdFUBON_ISO_MapReset(inBitMap, 37);
	}

	/* [新增預授權完成] 新增判斷條件只要是 PRE_COMP 交易，不管是正向或負向交易都不送晶片資料 2022/5/12 [SAM] */
	if (pobTran->srBRec.inCode == _PRE_COMP_ || pobTran->srBRec.inOrgCode == _PRE_COMP_)
		vdFUBON_ISO_MapReset(inBitMap, 55);
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
}

/*
 * App Name		: inFUBON_ISO_CUP_FuncAutoLogon
 * App Builder		: 
 * App Date&Time	: 2022/8/3
 * App Function		: 
 * Input Param		: BYTE usSlot,DWORD *Slot_Type
 * Output Param		: 
 */
void vdFUBON_ISO_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *bPackData, int *inPackLen)
{

}

/*
 * App Name		: inFUBON_ISO_ISOCheckHeader
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 12:01
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 * 
 */
int inFUBON_ISO_ISOCheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader)
{
	int inCnt = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	/* TPDU */
	inCnt += _FUBON_TPDU_SIZE_;
	/* MTI */
	szSendISOHeader[inCnt + 1] += 0x10;
	if (memcmp(&szSendISOHeader[inCnt], &szReceISOHeader[inCnt], _FUBON_MTI_SIZE_))
	{
		inDISP_DispLogAndWriteFlie(" InCnt[%d] Send[%02x%02x] != Rece[%02x%02x] Line[%d] END ", inCnt, szSendISOHeader[inCnt], 
						szSendISOHeader[inCnt + 1], szReceISOHeader[inCnt], szReceISOHeader[inCnt + 1], __LINE__ );
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * App Name		: inFUBON_ISO_ISOOnlineAnalyse
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 12:02
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_ISOOnlineAnalyse(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	if (pobTran->srBRec.inChipStatus == _EMV_CARD_ && (pobTran->inTransactionCode == _SALE_ ||
		pobTran->inTransactionCode == _INST_SALE_ ||
		pobTran->inTransactionCode == _REDEEM_SALE_ ||
		pobTran->inTransactionCode == _PRE_AUTH_ ||
		pobTran->inTransactionCode == _CUP_SALE_ ||
		pobTran->inTransactionCode == _CUP_PRE_AUTH_))
	{
		inDISP_LogPrintfWithFlag(" Fubon Analyse EMV Line[%d]", __LINE__);
		inRetVal = inFUBON_OnlineAnalyseEMV(pobTran);
	} else
	{
		inDISP_LogPrintfWithFlag(" Fubon Analyse Magnetic Line[%d]", __LINE__);
		inRetVal = inFUBON_OnlineAnalyseMagneticManual(pobTran);
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
 * App Name		: inFUBON_ISO_ISOAdviceAnalyse
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 1:04
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_ISOAdviceAnalyse(TRANSACTION_OBJECT *pobTran, VS_BOOL blTcUpload)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	if (!memcmp(&pobTran->srBRec.szRespCode[0], "00", 2))
	{
		if (inFUBON_ISO_CheckAuthCode(pobTran) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" AdviceAnalyse Check AuthCode[%s] *Error* Line[%d]", pobTran->srBRec.szAuthCode, __LINE__);
			return (VS_ERROR);
		}

		if (pobTran->srBRec.inChipStatus == _EMV_CARD_ && pobTran->uszTCUploadBit == VS_TRUE)
		{
			inDISP_LogPrintfWithFlag(" inFUBON_ISO_ISOAdviceAnalyse()_EMV_TC_LOAD_SUCCESS");
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
			//pobTran->srTRTRec.fUpdate = VS_TRUE;

			if (inFLOW_RunFunction(pobTran, _UPDATE_BATCH_) != VS_SUCCESS)
			{
				inDISP_LogPrintfWithFlag(" inFUBON_ISO_ISOAdviceAnalyse()_UPDATE_BATCH_ERROR");
				return (VS_ERROR);
			}

			if (inADVICE_DeleteRecordFlow(pobTran, pobTran->srBRec.lnOrgInvNum) != VS_SUCCESS)
			//if (inADVICE_DeleteOrg(pobTran, pobTran->srBRec.lnOrgInvNum) != VS_SUCCESS)
			{
				inDISP_LogPrintfWithFlag(" inFUBON_ISO_ISOAdviceAnalyse()_DELETE_ERROR");
				return (VS_ERROR);
			}


			inDISP_LogPrintfWithFlag(" Aft Advice pobTran->srBRec.szRefNo[%s]", pobTran->srBRec.szRefNo);
			/* 因為富邦OFFLINE要輸入RRN,所以拿掉 20190222 [SAM] */
//			/* Added Database  20190221 [SAM] */
//			//if(VS_SUCCESS == inBATCH_Get_Batch_ByInvNum_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_, pobTran->srBRec.lnOrgInvNum)
//			if(VS_SUCCESS != inSqlite_Update_ByInvNum_Trans_Rrn_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_, pobTran->srBRec.lnOrgInvNum))
//			{
//				if (ginDebug == VS_TRUE)
//					inDISP_LogPrintf("inFUBON_ISO_ISOAdviceAnalyse()_ Update_ByInvNum_Trans_Rrn");
//			}
//			

			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
			return (VS_SUCCESS);
		}
	} else
	{
		if (pobTran->inTransactionCode == _SETTLE_)
			inFUBON_DispHostResponseCode(pobTran);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] *Error* END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_ERROR);
}

/*
 * App Name		: inFUBON_ISO_GetBMapMTIPCode
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 1:08
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_GetBMapMTIPCode(TRANSACTION_OBJECT *pobTran, ISO_TYPE_FUBON_TABLE *srISOFuncIndex, int inBitMapCode)
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

/*
 * App Name		: vdFUBON_ISO_CopyISOMap
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 1:46
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
void vdFUBON_ISO_CopyISOMap(int *inBitMap, int *inSourceBitMap)
{
	int i;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	for (i = 0; i < _FUBON_MAX_BIT_MAP_CNT_; i++)
	{
		if (inSourceBitMap[i] == 0)
			break;
		else
		{
			inBitMap[i] = inSourceBitMap[i];
//			if (ginDebug == VS_TRUE)
//				inDISP_LogPrintf("  inBitMap(%d)", inBitMap[i]);
		}
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
}

/*
 * App Name		: vdFUBON_ISO_MapSet
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 1:46
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
void vdFUBON_ISO_MapSet(int *inBitMap, int inFeild)
{
	int i, inBMapCnt, inBitMapCnt;
	int inBMap[_FUBON_MAX_BIT_MAP_CNT_];

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
 * App Name		: vdFUBON_ISO_MapReset
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 1:48
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
void vdFUBON_ISO_MapReset(int *inBitMap, int inFeild)
{
	int i, inBMapCnt;
	int inBMap[_FUBON_MAX_BIT_MAP_CNT_];

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
 * App Name		: inFUBON_ISO_MapTest
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 1:47
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_MapTest(int *inBitMap, int inFeild)
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
 * App Name		: inFUBON_BitMapCheck
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 1:48
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_BitMapCheck(unsigned char *inBitMap, int inFeild)
{
	int inByteIndex, inBitIndex;

	inFeild--;
	inByteIndex = inFeild / 8;
	inBitIndex = 7 - (inFeild - inByteIndex * 8);

	if (_FUBON_CHECK_BIT_MAP_(inBitMap[inByteIndex], inBitIndex))
	{
//		inDISP_LogPrintfWithFlag(" inFUBON_BitMapCheck() OK 1,%02x,%d,%d,%d!!", inBitMap[inByteIndex], inFeild, inByteIndex, inBitIndex);
		return (VS_TRUE);
	}

	return (VS_FALSE);
}

/*
 * App Name		: inFUBON_ISO_Pack02
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 1:48
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack02(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;
	int inPANLen;
	char szPAN[_PAN_SIZE_ + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" Field 2 PAN[%s]", pobTran->srBRec.szPAN);

	memset(szPAN, 0x00, sizeof (szPAN));
	inPANLen = strlen(pobTran->srBRec.szPAN);
	uszPackBuf[inCnt++] = (inPANLen / 10 * 16) + (inPANLen % 10);
	memcpy(szPAN, pobTran->srBRec.szPAN, inPANLen);
	if (inPANLen % 2)
		szPAN[inPANLen++] = '0';

	inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], szPAN, inPANLen / 2);
	inCnt += inPANLen / 2;

	inDISP_LogPrintfWithFlag(" Field 2 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack02_tSAM
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 1:49
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack02_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0, /* i,*/ inEncryptLen = 0;
	short shLen;
	char szPAN[_PAN_SIZE_ + 1];
	char szEncrypt[_PAN_SIZE_ + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" Field 2 tSAM PAN[%s]", pobTran->srBRec.szPAN);

	memset(szPAN, 0x00, sizeof (szPAN));

	if (!memcmp(&pobTran->srBRec.szTranAbbrev[0], "NC", 2))
		memcpy(szPAN, &pobTran->srBRec.szPAN[4], 11);
	else
		strcpy(szPAN, pobTran->srBRec.szPAN);

	strcpy(szPAN, pobTran->srBRec.szPAN);

	shLen = strlen(szPAN);

	inFunc_ASCII_to_BCD((unsigned char *) szEncrypt, &szPAN[0], (shLen / 2));

	if ((inEncryptLen = inFUBON_TSAM_EncryptISOData(pobTran, &szEncrypt[0], shLen)) <= 0)
	{
		inDISP_DispLogAndWriteFlie(" FU tSAM Field2 Encrypt *Error* END Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag(" inFUBON_ISO_Pack02_tSAM()_inEncryptLen[%d]", inEncryptLen);

	shLen = (inEncryptLen * 2);
	uszPackBuf[inCnt++] = (shLen / 10 * 16) + (shLen % 10);
	memcpy((char *) &uszPackBuf[inCnt], &szEncrypt[0], inEncryptLen);
	inCnt += (shLen / 2);

	inDISP_LogPrintfWithFlag(" Field 2 tSAM PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack02_Software
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 1:53
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack02_Software(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;
	int inPANLen;
	int inSoftwareKey, i;
	char szPAN[_PAN_SIZE_ + 1];
	char szBCDPanData[10 + 1];
	char szSTAN[6 + 1], szField11[3 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" Field 2 Software PAN[%s]", pobTran->srBRec.szPAN);

	memset(szPAN, 0x00, sizeof (szPAN));
	inPANLen = strlen(pobTran->srBRec.szPAN);
	uszPackBuf[inCnt++] = (inPANLen / 10 * 16) + (inPANLen % 10);
	memcpy(szPAN, pobTran->srBRec.szPAN, inPANLen);
	if (inPANLen % 2)
		szPAN[inPANLen++] = '0';

	memset(szField11, 0x00, sizeof (szField11));
	memset(szSTAN, 0x00, sizeof (szSTAN));

	if (pobTran->srBRec.uszOfflineBit == VS_TRUE &&
		pobTran->srBRec.uszUpload1Bit == VS_TRUE &&
		pobTran->srBRec.uszUpload2Bit == VS_TRUE)
		sprintf(szSTAN, "%06ld", pobTran->srBRec.lnSTAN_Multi);
	else
		sprintf(szSTAN, "%06ld", pobTran->srBRec.lnSTANNum);

	inFunc_ASCII_to_BCD((unsigned char *) &szField11[0], &szSTAN[0], 3);
	inSoftwareKey = (unsigned char) szField11[2] & 0x0f;

	for (i = 0; i < inPANLen; i++)
	{
		szPAN[i] = szPAN[i] ^ inSoftwareKey;
		inSoftwareKey++;

		if (inSoftwareKey > 9)
			inSoftwareKey = 0;
	}

	SoftWareTransferPack(szPAN, inPANLen, szBCDPanData);

	memcpy((char *) &uszPackBuf[inCnt], &szBCDPanData[0], (inPANLen / 2));
	inCnt += (inPANLen / 2);

	inDISP_LogPrintfWithFlag(" Field 2 Software PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack03
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 1:54
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" Field 3 InCode[%d] ", pobTran->srBRec.inCode);
	inDISP_LogPrintfWithFlag(" Field 3 1[%02x] 2[%02x] 3[%02x] ", szFUBON_ISO_Field03[0], szFUBON_ISO_Field03[1], szFUBON_ISO_Field03[3]);
	
	memcpy(&uszPackBuf[inCnt], &szFUBON_ISO_Field03[0], _FUBON_PCODE_SIZE_);
	inCnt += _FUBON_PCODE_SIZE_;

	inDISP_LogPrintfWithFlag(" Field 3 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack04
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 1:55
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;
	char szTemplate[12 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" IsoTxCode[%d] TxnAmt[%ld] TipAmt[%ld] AdjAmt[%ld] InCode[%ld] inOrgCode[%ld]",
			pobTran->inISOTxnCode,
			pobTran->srBRec.lnTxnAmount, 
			pobTran->srBRec.lnTipTxnAmount, 
			pobTran->srBRec.lnAdjustTxnAmount, 
			pobTran->srBRec.inCode, 
			pobTran->srBRec.inOrgCode);

	memset(szTemplate, 0x00, sizeof (szTemplate));

	switch (pobTran->inISOTxnCode)
	{
		case _TIP_:
			sprintf(szTemplate, "%010ld00", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
			break;
		case _BATCH_UPLOAD_:
			if (pobTran->srBRec.inCode == _ADJUST_)
			{
				sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnAdjustTxnAmount);
			} else
			{
				sprintf(szTemplate, "%010ld00", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
			}
			break;
		case _REVERSAL_:
			if (pobTran->srBRec.inCode == _TIP_)
				sprintf(szTemplate, "%010ld00", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
			else
				sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);
			break;
		case _ADJUST_:
			sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnAdjustTxnAmount);
			break;
		default:
			sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);
			break;
	}

	inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], szTemplate, 6);
	inCnt += 6;

	inDISP_LogPrintfWithFlag(" Field 4 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack11
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 1:55
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack11(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;
	char szSTAN[6 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" lnSTANNum[%d] lnSTAN_Multi[%d] ",
			pobTran->srBRec.lnSTANNum, pobTran->srBRec.lnSTAN_Multi);
	
	memset(szSTAN, 0x00, sizeof (szSTAN));

	if (pobTran->srBRec.uszOfflineBit == VS_TRUE &&
		pobTran->srBRec.uszUpload1Bit == VS_TRUE &&
		pobTran->srBRec.uszUpload2Bit == VS_TRUE)
		sprintf(szSTAN, "%06ld", pobTran->srBRec.lnSTAN_Multi);
	else
		sprintf(szSTAN, "%06ld", pobTran->srBRec.lnSTANNum);

	inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], &szSTAN[0], 3);
	inCnt += 3;

	inDISP_LogPrintfWithFlag(" Field 11 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack12
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 1:56
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" szTime[%s]", pobTran->srBRec.szTime);
	
	inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], &pobTran->srBRec.szTime[0], 3);
	inCnt += 3;
	
	inDISP_LogPrintfWithFlag(" Field 12 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack13
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 1:56
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], &pobTran->srBRec.szDate[2 + 2], 2);
	inCnt += 2;

	inDISP_LogPrintfWithFlag(" Field 13 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack14
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 1:58
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack14(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" ExpDate[%s]", pobTran->srBRec.szExpDate);
	
	inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], pobTran->srBRec.szExpDate, _EXP_DATE_SIZE_ / 2);
	inCnt += _EXP_DATE_SIZE_ / 2;

	inDISP_LogPrintfWithFlag(" Field 14 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack14_Software
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 1:58
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack14_Software(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;
	int i = 0;
	char szSTAN[6 + 1], szField11[3 + 1];
	char EncrptyExp[4 + 1];
	short shLen, inSoftwareKey;
	char BcdBuf[2 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" Soft ExpDate[%s]", pobTran->srBRec.szExpDate);

	inCnt = 0;

	memcpy(EncrptyExp, pobTran->srBRec.szExpDate, sizeof (EncrptyExp));
	shLen = strlen(EncrptyExp);

	/* �}�l�[�K */
	memset(szField11, 0x00, sizeof (szField11));
	memset(szSTAN, 0x00, sizeof (szSTAN));
	if (pobTran->srBRec.uszOfflineBit == VS_TRUE &&
			pobTran->srBRec.uszUpload1Bit == VS_TRUE &&
			pobTran->srBRec.uszUpload2Bit == VS_TRUE)
		sprintf(szSTAN, "%06ld", pobTran->srBRec.lnSTAN_Multi);
	else
		sprintf(szSTAN, "%06ld", pobTran->srBRec.lnSTANNum);

	inFunc_ASCII_to_BCD((unsigned char *) &szField11[0], &szSTAN[0], 3);
	inSoftwareKey = (unsigned char) szField11[2] & 0x0f;

	for (i = 0; i < shLen; i++)
	{
		EncrptyExp[i] = EncrptyExp[i] ^ inSoftwareKey;
		inSoftwareKey++;

		if (inSoftwareKey > 9)
			inSoftwareKey = 0;
	}

	SoftWareTransferPack(EncrptyExp, 4, BcdBuf);
	memcpy((char *) &uszPackBuf[inCnt], BcdBuf, 2);

	inCnt += _EXP_DATE_SIZE_ / 2;

	inDISP_LogPrintfWithFlag(" Field 14 Software PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack22
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 2:00
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;
	char szTermCap[6 + 1];
	VS_BOOL fTermPinFlag = VS_FALSE;
	int inIndex = -1;
	char szTerminalCapbilityAscii[6 + 1]; /* TerminalCapbility的Ascii值 */
	char szTerminalCapbility[3 + 1]; /* TerminalCapbility的Hex值 */
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	inDISP_LogPrintfWithFlag(" uszContactlessBit[%02x] inTransactionCode[%d]", pobTran->srBRec.uszContactlessBit, pobTran->inTransactionCode);
	inDISP_LogPrintfWithFlag(" inCode[%d], inISOTxnCode[%d]", pobTran->srBRec.inCode, pobTran->inISOTxnCode);
	inDISP_LogPrintfWithFlag(" inOrgCode[%d] uszCUPTransBit[%d] uszPinEnterBit[%d]", 
			pobTran->srBRec.inOrgCode,
			pobTran->srBRec.uszCUPTransBit,
			pobTran->srBRec.uszPinEnterBit);

	
	memset(szTermCap, 0x00, sizeof (szTermCap));
	/* 1.比對TerminalCapbility */
	memset(szTerminalCapbility, 0x00, sizeof (szTerminalCapbility));
	if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)) == 0)
	{
		inIndex = _VISA_MVT_INDEX_;
	} else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
	{
		inIndex = _MCHIP_MVT_INDEX_;
	} else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)) == 0)
	{
		inIndex = _JSMART_MVT_INDEX_;
	} else if (memcmp(pobTran->srBRec.szCUP_EMVAID, "A000000333010101", strlen("A000000333010101")) == 0)
	{
		inIndex = _CUP_DEBIT_MVT_INDEX_;
	} else if (memcmp(pobTran->srBRec.szCUP_EMVAID, "A000000333010102", strlen("A000000333010102")) == 0)
	{
		inIndex = _CUP_CREDIT_MVT_INDEX_;
	} else if (memcmp(pobTran->srBRec.szCUP_EMVAID, "A000000333010103", strlen("A000000333010103")) == 0)
	{
		inIndex = _CUP_QUASI_CREDIT_MVT_INDEX_;
	} else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
	{
		inIndex = _AEIPS_MVT_INDEX_;
	} else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0)
	{
		inIndex = _D_PAS_MVT_INDEX_;
	} else
	{
		inIndex = -1;
	}
	
	inDISP_LogPrintfWithFlag(" CardIndex[%d] ", inIndex);
	
	memset(szTerminalCapbilityAscii, 0x00, sizeof (szTerminalCapbilityAscii));
	memset(szTerminalCapbility, 0x00, sizeof (szTerminalCapbility));
	if (inIndex != -1)
	{
		inLoadMVTRec(inIndex);
		inGetTerminalCapabilities(szTerminalCapbilityAscii);

		/* 有值才繼續 */
		if (strlen(szTerminalCapbilityAscii) > 0)
		{
			inFunc_ASCII_to_BCD((unsigned char*) szTerminalCapbility, szTerminalCapbilityAscii, 3);

			/* If EMVTermCapabilities enable  a , b , c ,mean PIN Entry Capbility enable
			   a. Plaintext PIN for ICC verification
			   b. Enciphered PIN for online verification
			   c. Enciphered PIN for offline verification
			 */
			if ((szTerminalCapbility[1] & 0xD0) == 0xD0)
				fTermPinFlag = VS_TRUE;
		}

	}

	/* MTIP*/
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		if (pobTran->srBRec.inCode == _CUP_SALE_ || pobTran->srBRec.inCode == _CUP_PRE_AUTH_)
		{
			if (pobTran->srBRec.uszPinEnterBit == VS_TRUE)
				//if (pobTran->szPIN[0] != ' ' && pobTran->szPIN[0] != 0x00)
				fTermPinFlag = VS_TRUE;
			else
				fTermPinFlag = VS_FALSE;
		}
	}

	if (pobTran->srBRec.inChipStatus == _EMV_CARD_ &&
			(pobTran->inTransactionCode == _CUP_VOID_ || pobTran->inTransactionCode == _CUP_PRE_AUTH_VOID_ || pobTran->inISOTxnCode == _REVERSAL_))
	{
		fTermPinFlag = VS_FALSE;
	}

	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE && pobTran->inISOTxnCode == _REVERSAL_)
	{
		fTermPinFlag = VS_FALSE;
	}

	inDISP_LogPrintfWithFlag(" fTermPinFlag[%d] ", fTermPinFlag);
	
	if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
	{
		if (pobTran->srBRec.inCode == _CUP_VOID_ || pobTran->srBRec.inCode == _CUP_PRE_AUTH_VOID_) /* 2013-12-02 AM 10:12:19 add by kakab �s�W���p���ΰh�f��Ӥ@��H�Υd����y�{ */
		{
			if (fTermPinFlag == VS_TRUE)
				inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "5011", 2);
			else
				inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "5012", 2);
		} else
		{
			if (fTermPinFlag == VS_TRUE)
				inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "5051", 2);
			else
				inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "5052", 2);
		}
	} else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
	{
		/* [新增預授權完成] 新增判斷條件 2022/5/12 [SAM] */
		if (pobTran->srBRec.inOrgCode == _PRE_COMP_ && pobTran->srBRec.inCode == _VOID_)
		{
			if (fTermPinFlag == VS_TRUE)
				inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "5011", 2);
			else
				inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "5012", 2);

		} else if (pobTran->srBRec.inCode == _PRE_COMP_)/* [新增預授權完成] 新增判斷條件 2022/5/12 [SAM] */
		{
			if (fTermPinFlag == VS_TRUE)
				inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "5011", 2);
			else
				inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "5012", 2);

		} else if (pobTran->srBRec.inCode == _CUP_VOID_)
		{
			if (fTermPinFlag == VS_TRUE)
				inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "3011", 2);
			else
				inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "3012", 2);
		} else if ((pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _CUP_REFUND_) ||
				(pobTran->srBRec.inOrgCode == _REFUND_ && pobTran->srBRec.inCode == _VOID_))
		{
			/* 富邦通知 原帶 5022 改為帶 5012 因為02後送時會判斷要送完成 TRAC II 所以要改成 01 2022/10/19 [SAM] */
			inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "5012", 2);
		} else if (fTermPinFlag == VS_TRUE)
		{
			if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
				inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "3071", 2); /* 3901 -> 3071 add by sampo for fubon */
			else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE)
				inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "4911", 2);
			else
				inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "3071", 2);
		} else
		{
			if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
				inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "3072", 2);
			else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE)
				inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "4912", 2);
			else
				inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "3072", 2);
		}
	} else if (pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
	{
		if (fTermPinFlag == VS_TRUE)
			inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "5801", 2);
		else
			inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "5802", 2);

		if (pobTran->srBRec.inCode == _CUP_VOID_ || pobTran->srBRec.inCode == _CUP_PRE_AUTH_VOID_) /* 2013-12-02 AM 10:12:19 add by kakab �s�W���p���ΰh�f��Ӥ@��H�Υd����y�{ */
		{
			if (fTermPinFlag == VS_TRUE)
				inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "5011", 2);
			else
				inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "5012", 2);
		}
	} else
	{
		if (pobTran->srBRec.uszManualBit == VS_TRUE ||
			pobTran->srBRec.inCode == _CUP_VOID_ ||
			pobTran->srBRec.inCode == _CUP_PRE_AUTH_VOID_)
		{
			if (fTermPinFlag == VS_TRUE)
				inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "5011", 2);
			else
				inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "5012", 2);
		} else
		{
			if (fTermPinFlag == VS_TRUE)
				inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "5021", 2);
			else
				inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "5022", 2);
		}
	}

	inCnt += 2;

	inDISP_LogPrintfWithFlag(" Field 14 Software PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack24
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 2:00
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;
	char szTemplate[100 + 1];
	unsigned char uszBCD[2 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memset(szTemplate, 0x00, sizeof (szTemplate));
	if (inGetNII(&szTemplate[1]) == VS_ERROR)
	{
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] *Error* GetNii -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	
	/* NII 只有三碼所以前面補0 */
	szTemplate[0] = '0';

	inDISP_LogPrintfWithFlag(" F_024 [NII %s]", szTemplate);
	
	memset(uszBCD, 0x00, sizeof (uszBCD));
	inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 2);

	memcpy((char *) &uszPackBuf[inCnt], (char *) &uszBCD[0], 2);
	inCnt += 2;

	inDISP_LogPrintfWithFlag(" Field 24 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);

}

/*
 * App Name		: inFUBON_ISO_Pack25
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 2:04
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack25(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	if ((pobTran->inTransactionCode == _PRE_COMP_) ||
			(pobTran->inTransactionCode == _CUP_PRE_COMP_) ||
			(pobTran->inTransactionCode == _PRE_AUTH_) ||
			(pobTran->inTransactionCode == _CUP_PRE_AUTH_) ||
			(pobTran->inISOTxnCode == _BATCH_UPLOAD_ && pobTran->srBRec.inCode == _PRE_COMP_) ||
			(pobTran->inISOTxnCode == _BATCH_UPLOAD_ && pobTran->srBRec.inCode == _CUP_PRE_COMP_))
		inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "06", 1);
	else
		inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "00", 1);

	inCnt += 1;
	
	inDISP_LogPrintfWithFlag(" Field 25 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack25_Software
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 2:04
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack25_Software(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	if (pobTran->srBRec.inCode != _CUP_LOGON_ || pobTran->srBRec.inCode != _CHANGE_TMK_ || pobTran->srBRec.inCode != _CUP_CHANG_TPK_)
	{
		/* 20221110，Cardpool所有交易F_25只送[98]，cardpool會依F_3去重組F_25。目前授權完成會送[06]
		     (Tsam版，預先授權、授權完成及授權完成取消，要送[06]) 
		 所以把 06 的條件拿掉  2022/11/16 [SAM] */
//		if ((pobTran->inTransactionCode == _PRE_COMP_) ||
//				(pobTran->inISOTxnCode == _BATCH_UPLOAD_ && pobTran->srBRec.inCode == _PRE_COMP_))
//			/*if ((pobTran->inTransactionCode == _PRE_COMP_) || (pobTran->inTransactionCode == _CUP_PRE_COMP_) ||
//				(pobTran->inISOTxnCode == _BATCH_UPLOAD_ && pobTran->srBRec.inCode == _PRE_COMP_) ||
//				(pobTran->inISOTxnCode == _BATCH_UPLOAD_ && pobTran->srBRec.inCode == _CUP_PRE_COMP_))*/
//			inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "06", 1);
//		else
			inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "98", 1);
	} else
		inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "00", 1);


	inCnt += 1;
	
	inDISP_LogPrintfWithFlag(" Field 25 Software PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack35
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 2:05
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack35(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0, i;
	short shLen;
	char szT2Data[_TRACK2_SIZE_ + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	memset(szT2Data, 0x00, sizeof (szT2Data));

	switch (pobTran->inISOTxnCode)
	{
		case _TIP_:
		case _TC_UPLOAD_:
		case _ADVICE_:
		case _VOID_:
		case _CUP_PRE_COMP_:
		case _REVERSAL_:
		case _ISR_UPLOAD_:
		case _PRE_COMP_:
		case _ADJUST_:
		case _REDEEM_ADJUST_:
		case _INST_ADJUST_:
		case _SALE_OFFLINE_:
		case _CUP_VOID_:
		case _CUP_PRE_AUTH_VOID_:
		case _CUP_PRE_COMP_VOID_:
		case _BATCH_UPLOAD_:
			if (pobTran->srBRec.uszManualBit == VS_TRUE)
			{
				/* Manual keyin -> PAN + 'D' + Expire Date */
				if (!memcmp(&pobTran->srBRec.szTranAbbrev[0], "NC", 2))
				{
					memcpy(szT2Data, &pobTran->srBRec.szPAN[4], 11);
					strcat(szT2Data, "D");
					strcat(szT2Data, pobTran->srBRec.szExpDate);
				} else
				{
					strcpy(szT2Data, pobTran->srBRec.szPAN);
					strcat(szT2Data, "D");
					strcat(szT2Data, pobTran->srBRec.szExpDate);
				}
			} else
			{
				if (!memcmp(&pobTran->srBRec.szTranAbbrev[0], "NC", 2) && !memcmp(&pobTran->srBRec.szServiceCode[0], "701", 3))
				{
					memcpy(szT2Data, &pobTran->srBRec.szPAN[4], 11);
					strcat(szT2Data, "D");
					strcat(szT2Data, pobTran->srBRec.szExpDate);
					strcat(szT2Data, pobTran->srBRec.szServiceCode);
				} else
				{
					strcpy(szT2Data, pobTran->srBRec.szPAN);
					strcat(szT2Data, "D");
					strcat(szT2Data, pobTran->srBRec.szExpDate);
					strcat(szT2Data, pobTran->srBRec.szServiceCode);
				}
			}

			break;
		default:
			
			/* 新增感應退貨，只需送至有效期資料既可 2022/11/9 [SAM] */
			if(pobTran->inISOTxnCode == _REFUND_ && pobTran->srBRec.uszContactlessBit == VS_TRUE)
			{
				/* Manual keyin -> PAN + 'D' + Expire Date */
				if (!memcmp(&pobTran->srBRec.szTranAbbrev[0], "NC", 2)) 
				{
					memcpy(szT2Data, &pobTran->srBRec.szPAN[4], 11);
					strcat(szT2Data, "D");
					strcat(szT2Data, pobTran->srBRec.szExpDate);
				} else
				{
					strcpy(szT2Data, pobTran->srBRec.szPAN);
					strcat(szT2Data, "D");
					strcat(szT2Data, pobTran->srBRec.szExpDate);
				}
				
			}else if (pobTran->srBRec.uszManualBit == VS_TRUE)
			{
				/* Manual keyin -> PAN + 'D' + Expire Date */
				if (!memcmp(&pobTran->srBRec.szTranAbbrev[0], "NC", 2))
				{
					memcpy(szT2Data, &pobTran->srBRec.szPAN[4], 11);
					strcat(szT2Data, "D");
					strcat(szT2Data, pobTran->srBRec.szExpDate);
				} else
				{
					strcpy(szT2Data, pobTran->srBRec.szPAN);
					strcat(szT2Data, "D");
					strcat(szT2Data, pobTran->srBRec.szExpDate);
				}
			} else
			{
				inCARDFUNC_ModifyTrack2(pobTran->szTrack2, strlen(pobTran->szTrack2));
				strcpy(szT2Data, pobTran->szTrack2);

				for (i = 0; i < strlen(szT2Data); i++)
				{
					if (szT2Data[i] == '=')
						szT2Data[i] = 'D';
				}
			}

			break;
	} /* End switch () ... */

	shLen = strlen(szT2Data);
	uszPackBuf[inCnt++] = (shLen / 10 * 16) + (shLen % 10);

	if (shLen % 2)
	{
		shLen++;
		strcat(szT2Data, "0");
	}

	inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], &szT2Data[0], (shLen / 2));
	inCnt += (shLen / 2);
	
	inDISP_LogPrintfWithFlag(" Field 35 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack35_tSAM
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 2:22
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack35_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0, i, inEncryptLen;
	short shLen;
	char szT2Data[64 + 1];
	char szEncrypt[64 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	memset(szT2Data, 0x00, sizeof (szT2Data));

	switch (pobTran->inISOTxnCode)
	{
		case _TIP_:
		case _TC_UPLOAD_:
		case _ADVICE_:
		case _VOID_:
		case _CUP_PRE_COMP_:
		case _REVERSAL_:
		case _ISR_UPLOAD_:
		case _PRE_COMP_:
		case _ADJUST_:
		case _REDEEM_ADJUST_:
		case _INST_ADJUST_:
		case _SALE_OFFLINE_:
		case _CUP_VOID_:
		case _CUP_PRE_AUTH_VOID_:
		case _CUP_PRE_COMP_VOID_:
		case _BATCH_UPLOAD_:
			if (pobTran->srBRec.uszManualBit == VS_TRUE)
			{
				/* Manual keyin -> PAN + 'D' + Expire Date */
				if (!memcmp(&pobTran->srBRec.szTranAbbrev[0], "NC", 2))
				{
					memcpy(szT2Data, &pobTran->srBRec.szPAN[4], 11);
					strcat(szT2Data, "D");
					strcat(szT2Data, pobTran->srBRec.szExpDate);
				} else
				{
					strcpy(szT2Data, pobTran->srBRec.szPAN);
					strcat(szT2Data, "D");
					strcat(szT2Data, pobTran->srBRec.szExpDate);
				}
			} else
			{
				if (!memcmp(&pobTran->srBRec.szTranAbbrev[0], "NC", 2) && !memcmp(&pobTran->srBRec.szServiceCode[0], "701", 3))
				{
					memcpy(szT2Data, &pobTran->srBRec.szPAN[4], 11);
					strcat(szT2Data, "D");
					strcat(szT2Data, pobTran->srBRec.szExpDate);
					strcat(szT2Data, pobTran->srBRec.szServiceCode);
				} else
				{
					strcpy(szT2Data, pobTran->srBRec.szPAN);
					strcat(szT2Data, "D");
					strcat(szT2Data, pobTran->srBRec.szExpDate);
					strcat(szT2Data, pobTran->srBRec.szServiceCode);
				}
			}

			break;
		default:
			if (pobTran->srBRec.uszManualBit == VS_TRUE)
			{
				/* Manual keyin -> PAN + 'D' + Expire Date */
				if (!memcmp(&pobTran->srBRec.szTranAbbrev[0], "NC", 2))
				{
					memcpy(szT2Data, &pobTran->srBRec.szPAN[4], 11);
					strcat(szT2Data, "D");
					strcat(szT2Data, pobTran->srBRec.szExpDate);
				} else
				{
					strcpy(szT2Data, pobTran->srBRec.szPAN);
					strcat(szT2Data, "D");
					strcat(szT2Data, pobTran->srBRec.szExpDate);
				}
			} else
			{
				inCARDFUNC_ModifyTrack2(pobTran->szTrack2, strlen(pobTran->szTrack2));
				strcpy(szT2Data, pobTran->szTrack2);

				for (i = 0; i < strlen(szT2Data); i++)
				{
					if (szT2Data[i] == '=')
						szT2Data[i] = 'D';
				}
			}

			break;
	} /* End switch () ... */

	shLen = strlen(szT2Data);

	inDISP_LogPrintfWithFlag(" Before Encryptation Data Len[%d]", shLen);

	/* Field_35 */
	memset(szEncrypt, 0x00, sizeof (szEncrypt));
	inFunc_ASCII_to_BCD((unsigned char *) szEncrypt, &szT2Data[0], (shLen / 2) + 1);

	if ((inEncryptLen = inFUBON_TSAM_EncryptISOData(pobTran, &szEncrypt[0], shLen)) <= 0)
	{
		inDISP_DispLogAndWriteFlie(" FU tSAM Field35 Encrypt *Error* END Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag(" After Encryptation Result Or Len [%d]", inEncryptLen);


	shLen = (inEncryptLen * 2);
	uszPackBuf[inCnt++] = (shLen / 10 * 16) + (shLen % 10);
	memcpy((char *) &uszPackBuf[inCnt], &szEncrypt[0], inEncryptLen);
	inCnt += (shLen / 2);

	inDISP_LogPrintfWithFlag(" Field 35 tSAM PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack35_Software
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 2:22
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 * 修改條件，感應交易下 _PRE_COMP_  _REFUND_ 不能送Service Code 2022/11/21 [SAM]
 */
int inFUBON_ISO_Pack35_Software(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0, i, inEncryptLen;
	short shLen, inSoftwareKey;
	char szT2Data[_TRACK2_SIZE_ + 1], szBCDT2Data[_TRACK2_SIZE_ + 1];
	char szSTAN[6 + 1], szField11[3 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" Soft inISOTxnCode[%d] ", pobTran->inISOTxnCode);

	memset(szT2Data, 0x00, sizeof (szT2Data));

	switch (pobTran->inISOTxnCode)
	{
		case _PRE_COMP_:
		case _REFUND_:
			/* 新增感應退貨，只需送至有效期資料既可 2022/11/9 [SAM] */
			if(pobTran->srBRec.uszContactlessBit == VS_TRUE || pobTran->srBRec.uszManualBit == VS_TRUE)
			{
				inFUBON_PackField35Data(pobTran, szT2Data , NON_PACK_SERVICE_CODE);

			}else {
				inFUBON_PackField35Data(pobTran, szT2Data , PACK_SERVICE_CODE);
			}
			break;
			
		case _VOID_:
			if (pobTran->srBRec.uszManualBit == VS_TRUE || 
				( (pobTran->srBRec.inOrgCode == _REFUND_ || pobTran->srBRec.inOrgCode == _PRE_COMP_)
				    && pobTran->srBRec.uszContactlessBit == VS_TRUE))
			{
				inFUBON_PackField35Data(pobTran, szT2Data , NON_PACK_SERVICE_CODE);
			} else
			{
				inFUBON_PackField35Data(pobTran, szT2Data , PACK_SERVICE_CODE);
			}
			break;
		case _REVERSAL_:
		case _ISR_UPLOAD_:	
		case _BATCH_UPLOAD_:
			if (pobTran->srBRec.uszManualBit == VS_TRUE || 
				( (pobTran->srBRec.inCode == _REFUND_ || 
					pobTran->srBRec.inCode == _PRE_COMP_ ||
					pobTran->srBRec.inOrgCode == _REFUND_ || 
					pobTran->srBRec.inOrgCode == _PRE_COMP_)
				    && pobTran->srBRec.uszContactlessBit == VS_TRUE))
			{
				inFUBON_PackField35Data(pobTran, szT2Data , NON_PACK_SERVICE_CODE);
			} else
			{
				inFUBON_PackField35Data(pobTran, szT2Data , PACK_SERVICE_CODE);
			}
			break;			
		case _TIP_:
		case _TC_UPLOAD_:
		case _ADVICE_:
		case _CUP_PRE_COMP_:
		case _ADJUST_:
		case _REDEEM_ADJUST_:
		case _INST_ADJUST_:
		case _SALE_OFFLINE_:
		case _CUP_VOID_:
		case _CUP_PRE_AUTH_VOID_:
		case _CUP_PRE_COMP_VOID_:
			
			if (pobTran->srBRec.uszManualBit == VS_TRUE)
			{
				inFUBON_PackField35Data(pobTran, szT2Data , NON_PACK_SERVICE_CODE);
				
			} else
			{
				inFUBON_PackField35Data(pobTran, szT2Data , PACK_SERVICE_CODE);
			}

			break;
		default:
			if (pobTran->srBRec.uszManualBit == VS_TRUE)
			{
				inFUBON_PackField35Data(pobTran, szT2Data , NON_PACK_SERVICE_CODE);
				
			} else
			{
				inCARDFUNC_ModifyTrack2(pobTran->szTrack2, strlen(pobTran->szTrack2));
				strcpy(szT2Data, pobTran->szTrack2);
			}
			break;
	} /* End switch () ... */

	for (i = 0; i < strlen(szT2Data); i++)
	{
		if (szT2Data[i] == 'D')
			szT2Data[i] = '=';
	}

	shLen = strlen(szT2Data);

	inDISP_LogPrintfArea(FALSE, "inFUBON_ISO_Pack35_Software 0", 29, (BYTE *) szT2Data, shLen);

	uszPackBuf[inCnt++] = (shLen / 10 * 16) + (shLen % 10);

	if (shLen % 2)
	{
		shLen++;
		strcat(szT2Data, "0");
	}

	/* �}�l�[�K */
	memset(szBCDT2Data, 0x00, sizeof (szBCDT2Data));
	inFunc_ASCII_to_BCD((unsigned char *) &szBCDT2Data[0], &szT2Data[0], (shLen / 2));
	memset(szField11, 0x00, sizeof (szField11));
	memset(szSTAN, 0x00, sizeof (szSTAN));

	if (pobTran->srBRec.uszOfflineBit == VS_TRUE &&
		pobTran->srBRec.uszUpload1Bit == VS_TRUE &&
		pobTran->srBRec.uszUpload2Bit == VS_TRUE)
		sprintf(szSTAN, "%06ld", pobTran->srBRec.lnSTAN_Multi);
	else
		sprintf(szSTAN, "%06ld", pobTran->srBRec.lnSTANNum);

	inFunc_ASCII_to_BCD((unsigned char *) &szField11[0], &szSTAN[0], 3);
	inSoftwareKey = (unsigned char) szField11[2] & 0x0f;

	for (i = 0; i < shLen; i++)
	{
		szT2Data[i] = szT2Data[i] ^ inSoftwareKey;
		inSoftwareKey++;

		if (inSoftwareKey > 9)
			inSoftwareKey = 0;
	}

	inDISP_LogPrintfArea(TRUE, "inFUBON_ISO_Pack35_Software 1", 29, (BYTE *) szT2Data, shLen);

	inEncryptLen = SoftWareTransferPack(szT2Data, shLen, szBCDT2Data);

	inDISP_LogPrintfArea(TRUE, "inFUBON_ISO_Pack35_Software 2", 29, (BYTE *) szBCDT2Data, inEncryptLen);

	shLen = (inEncryptLen * 2);

	memcpy((char *) &uszPackBuf[inCnt], &szBCDT2Data[0], (shLen / 2));
	inCnt += (shLen / 2);

	inDISP_LogPrintfWithFlag(" Field 35 Software PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);

}

/*
 * App Name		: inFUBON_ISO_Pack37
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 2:24
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memcpy((char *) &uszPackBuf[inCnt], pobTran->srBRec.szRefNo, 12);
	inCnt += 12;

	inDISP_LogPrintfWithFlag(" Field 37 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack38
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 3:26
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0, i;
	char szAuthCode[_AUTH_CODE_SIZE_ + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	for (i = 0; i < strlen(pobTran->srBRec.szAuthCode); i++)
	{
		if (((pobTran->srBRec.szAuthCode[i] >= '0') && (pobTran->srBRec.szAuthCode[i] <= '9')) ||
			((pobTran->srBRec.szAuthCode[i] >= 'A') && (pobTran->srBRec.szAuthCode[i] <= 'Z')) ||
			((pobTran->srBRec.szAuthCode[i] >= 'a') && (pobTran->srBRec.szAuthCode[i] <= 'z')) ||
			(pobTran->srBRec.szAuthCode[i] == 0x20))
		{
			continue;
		} else
		{
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE && (pobTran->srBRec.szAuthCode[i] == 0x30 || pobTran->srBRec.szAuthCode[i] == 0x20))
				continue;
			else
			{
				inDISP_DispLogAndWriteFlie(" FU Pack Field38_1[%s] *Error* Line[%d]", pobTran->srBRec.szAuthCode,  __LINE__);
				return (VS_ERROR);
			}
		}
	}

	/* [新增預授權完成] 因預先授權完成在輸入時可以允許空白授權碼，所以不用檢查 2022/11/7 [SAM] */
	if(pobTran->inTransactionCode != _PRE_COMP_ )
	{
		if ((!memcmp(pobTran->srBRec.szAuthCode, "000000", 6) || !memcmp(pobTran->srBRec.szAuthCode, "      ", 6)) &&
			pobTran->srBRec.uszCUPTransBit != VS_TRUE)
		{
			inDISP_DispLogAndWriteFlie(" FU Pack Field38_2[%s] *Error* Line[%d]", pobTran->srBRec.szAuthCode,  __LINE__);
			return (VS_ERROR);
		}
	}

	memset(szAuthCode, 0x00, sizeof (szAuthCode));
	strcpy(szAuthCode, pobTran->srBRec.szAuthCode);
	inFunc_PAD_ASCII(szAuthCode, szAuthCode, ' ', 6, _PAD_LEFT_FILL_RIGHT_);
	memcpy((char *) &uszPackBuf[inCnt], &szAuthCode[0], _AUTH_CODE_SIZE_);
	inCnt += _AUTH_CODE_SIZE_;

	inDISP_LogPrintfWithFlag(" Field 38 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack39
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 3:30
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	memcpy((char *) &uszPackBuf[inCnt], &pobTran->srBRec.szRespCode[0], 2);
	inCnt += 2;

	inDISP_LogPrintfWithFlag(" Field 39 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack41
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 3:31
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack41(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
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
 * App Name		: inFUBON_ISO_Pack42
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 3:33
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
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
 * App Name		: inFUBON_ISO_Pack48
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 3:37
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	if (!memcmp(szGetCustomerIndicator(), _CUSTOMER_002_DREAM_MALL_, 3) ||
		!memcmp(szGetCustomerIndicator(), _CUSTOMER_003_HANKYU_, 3))
	{
		/* Length */
		uszPackBuf[inCnt++] = 0x00;
		uszPackBuf[inCnt++] = 0x21;
		/* Table ID */
		uszPackBuf[inCnt++] = 0x14;
		/* Value */
		memcpy((char *) &uszPackBuf[inCnt], pobTran->srBRec.szStoreID, 20);
		inCnt += 20;
	} else
	{
		/* Length */
		uszPackBuf[inCnt++] = 0x00;
		uszPackBuf[inCnt++] = 0x19;
		/* Table ID */
		uszPackBuf[inCnt++] = 0x12;
		/* Value */
		memcpy((char *) &uszPackBuf[inCnt], pobTran->srBRec.szStoreID, 18);
		inCnt += 18;
	}

	inDISP_LogPrintfWithFlag(" Field 48 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack52
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 3:37
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack52(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	inDISP_LogPrintfWithFlag("inFUBON_ISO_Pack52(%d) START!!", pobTran->srBRec.uszManualBit);
	inDISP_LogPrintfArea(TRUE, (char *) "szPIN 1:", 8, (unsigned char*) pobTran->szPIN, sizeof (pobTran->szPIN));

	inFunc_ASCII_to_BCD((BYTE *) & uszPackBuf[inCnt], pobTran->szPIN, _PIN_SIZE_);
	inCnt += 8;
	
	inDISP_LogPrintfArea(TRUE, (char *) "szPIN 2:", 8, &uszPackBuf[inCnt - 8], _PIN_SIZE_);
	
	inDISP_LogPrintfWithFlag(" Field 52 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack54
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 3:39
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack54(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	uszPackBuf[inCnt++] = 0x00;
	uszPackBuf[inCnt++] = 0x12;
	sprintf((char *) &uszPackBuf[inCnt], "%010ld00", pobTran->srBRec.lnTipTxnAmount);
	inCnt += 12;

	inDISP_LogPrintfWithFlag(" Field 54 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack55
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 3:39
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack55(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0, inPacketCnt = 0;
	char szASCII[4 + 1], szBCD[2 + 1];
	char szPacket[512 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memset(szPacket, 0x00, sizeof (szPacket));

	if (pobTran->srEMVRec.in5F2A_TransCurrCodeLen > 0)
	{
		szPacket[inPacketCnt++] = 0x5F;
		szPacket[inPacketCnt++] = 0x2A;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz5F2A_TransCurrCode[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);
		inPacketCnt += pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
	}

	if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen > 0)
	{
		szPacket[inPacketCnt++] = 0x5F;
		szPacket[inPacketCnt++] = 0x34;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
		inPacketCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
	}

	if (pobTran->srEMVRec.in82_AIPLen > 0)
	{
		szPacket[inPacketCnt++] = 0x82;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in82_AIPLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz82_AIP[0], pobTran->srEMVRec.in82_AIPLen);
		inPacketCnt += pobTran->srEMVRec.in82_AIPLen;
	}

	if (pobTran->srEMVRec.in84_DFNameLen > 0)
	{
		szPacket[inPacketCnt++] = 0x84;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in84_DFNameLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
		inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;
	}

	if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) ||
			(pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY) ||
			(pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3) ||
			(pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE && (pobTran->inISOTxnCode == _SALE_ || pobTran->inISOTxnCode == _REVERSAL_)) ||
			(pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP && (pobTran->inISOTxnCode == _SALE_ || pobTran->inISOTxnCode == _REVERSAL_)))
	{
		
	} else
	{
		if (pobTran->inISOTxnCode == _TIP_ || pobTran->inISOTxnCode == _TC_UPLOAD_ || pobTran->inISOTxnCode == _ADJUST_ ||
				pobTran->inISOTxnCode == _BATCH_UPLOAD_ ||
				(pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE && pobTran->inISOTxnCode == _ADVICE_) ||
				(pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP && pobTran->inISOTxnCode == _ADVICE_) ||
				(!memcmp(&pobTran->srBRec.szAuthCode[0], "Y1", 2) && pobTran->srBRec.inChipStatus == _EMV_CARD_))
		{
			if (pobTran->srEMVRec.in8A_AuthRespCodeLen > 0)
			{
				szPacket[inPacketCnt++] = 0x8A;
				szPacket[inPacketCnt++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
				memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
				inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
			}
		}
		else if (pobTran->inISOTxnCode == _REVERSAL_ && pobTran->srBRec.inCode == _TIP_)
		{
			if (pobTran->srEMVRec.in8A_AuthRespCodeLen > 0)
			{
				szPacket[inPacketCnt++] = 0x8A;
				szPacket[inPacketCnt++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
				memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
				inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
			}
		}
	}

	/* TAG 91 ARPC
		Issuer Authentication Data, mandatory if online issuer authentication is required.
		for Batch Upload only
	 */
	if (pobTran->inISOTxnCode == _BATCH_UPLOAD_)
	{
		if (pobTran->srEMVRec.in91_IssuerAuthDataLen > 0)
		{
			szPacket[inPacketCnt++] = 0x91;
			szPacket[inPacketCnt++] = pobTran->srEMVRec.in91_IssuerAuthDataLen;
			memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz91_IssuerAuthData[0], pobTran->srEMVRec.in91_IssuerAuthDataLen);
			inPacketCnt += pobTran->srEMVRec.in91_IssuerAuthDataLen;
		}
	}
	if (pobTran->srEMVRec.in95_TVRLen > 0)
	{
		szPacket[inPacketCnt++] = 0x95;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in95_TVRLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz95_TVR[0], pobTran->srEMVRec.in95_TVRLen);
		inPacketCnt += pobTran->srEMVRec.in95_TVRLen;
	}

	if (pobTran->srEMVRec.in9A_TranDateLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9A;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9A_TranDateLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);
		inPacketCnt += pobTran->srEMVRec.in9A_TranDateLen;
	}

	if (pobTran->srEMVRec.in9B_TSILen > 0)
	{
		szPacket[inPacketCnt++] = 0x9B;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9B_TSILen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
		inPacketCnt += pobTran->srEMVRec.in9B_TSILen;
	}

	if (pobTran->srEMVRec.in9C_TranTypeLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9C;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9C_TranTypeLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9C_TranType[0], pobTran->srEMVRec.in9C_TranTypeLen);
		inPacketCnt += pobTran->srEMVRec.in9C_TranTypeLen;
	}

	if (pobTran->srEMVRec.in9F02_AmtAuthNumLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x02;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F02_AmtAuthNumLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F02_AmtAuthNumLen;
	}

	if (pobTran->srEMVRec.in9F03_AmtOtherNumLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x03;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F03_AmtOtherNumLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F03_AmtOtherNum[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F03_AmtOtherNumLen;
	}

	if (pobTran->srEMVRec.in9F09_TermVerNumLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x09;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F09_TermVerNumLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F09_TermVerNum[0], pobTran->srEMVRec.in9F09_TermVerNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F09_TermVerNumLen;
	}

	if (pobTran->srEMVRec.in9F10_IssuerAppDataLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x10;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F10_IssuerAppDataLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F10_IssuerAppData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);
		inPacketCnt += pobTran->srEMVRec.in9F10_IssuerAppDataLen;
	}

	if (pobTran->srEMVRec.in9F1A_TermCountryCodeLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x1A;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F1A_TermCountryCode[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);
		inPacketCnt += pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
	}

	if (pobTran->srEMVRec.in9F1E_IFDNumLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x1E;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F1E_IFDNumLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
	}

	if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x26;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F26_ApplCryptogramLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F26_ApplCryptogram[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);
		inPacketCnt += pobTran->srEMVRec.in9F26_ApplCryptogramLen;
	}

	if (pobTran->srEMVRec.in9F27_CIDLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x27;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F27_CIDLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F27_CID[0], pobTran->srEMVRec.in9F27_CIDLen);
		inPacketCnt += pobTran->srEMVRec.in9F27_CIDLen;
	}

	if (pobTran->srEMVRec.in9F33_TermCapabilitiesLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x33;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F33_TermCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
		inPacketCnt += pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
	}

	if (pobTran->srEMVRec.in9F34_CVMLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x34;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F34_CVMLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
		inPacketCnt += pobTran->srEMVRec.in9F34_CVMLen;
	}

	if (pobTran->srEMVRec.in9F35_TermTypeLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x35;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F35_TermTypeLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
		inPacketCnt += pobTran->srEMVRec.in9F35_TermTypeLen;
	}

	if (pobTran->srEMVRec.in9F36_ATCLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x36;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F36_ATCLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F36_ATC[0], pobTran->srEMVRec.in9F36_ATCLen);
		inPacketCnt += pobTran->srEMVRec.in9F36_ATCLen;
	}

	if (pobTran->srEMVRec.in9F37_UnpredictNumLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x37;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F37_UnpredictNumLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F37_UnpredictNum[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F37_UnpredictNumLen;
	}
	/* [�I��] �W��TC_UPLOAD�~�e9F41 */
	if (pobTran->inISOTxnCode == _TC_UPLOAD_ || pobTran->inISOTxnCode == _ISR_UPLOAD_)
	{
		if (pobTran->srEMVRec.in9F41_TransSeqCounterLen > 0)
		{
			szPacket[inPacketCnt++] = 0x9F;
			szPacket[inPacketCnt++] = 0x41;
			szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F41_TransSeqCounterLen;
			memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
			inPacketCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
		}
	}

	if (pobTran->srEMVRec.in9F5B_ISRLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x5B;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F5B_ISRLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F5B_ISR[0], pobTran->srEMVRec.in9F5B_ISRLen);
		inPacketCnt += pobTran->srEMVRec.in9F5B_ISRLen;
	}

	if (pobTran->srBRec.inChipStatus == _EMV_CARD_ && pobTran->srEMVRec.in9F63_CardProductLabelInformationLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x63;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F63_CardProductLabelInformationLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F63_CardProductLabelInformation[0], pobTran->srEMVRec.in9F63_CardProductLabelInformationLen);
		inPacketCnt += pobTran->srEMVRec.in9F63_CardProductLabelInformationLen;
	}

	/* F_55 �[ 9F6E, Add by sunny 2017/05/09 */
	/* Enhance Contactless Reader Capabilities (START)*/
	/* EMV Contactless Bool C-4 Kernel 4 Spec V2.5 */
	if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
	{
		if (pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen > 0)
		{
			szPacket[inPacketCnt++] = 0x9F;
			szPacket[inPacketCnt++] = 0x6E;
			szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen;
			memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F6E_From_Factor_Indicator[0], pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen);
			inPacketCnt += pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen;
		}
	}
	/* Enhance Contactless Reader Capabilities (END)*/

	/* Packet Data Length */
	memset(szASCII, 0x00, sizeof (szASCII));
	sprintf(szASCII, "%04d", inPacketCnt);
	memset(szBCD, 0x00, sizeof (szBCD));
	inFunc_ASCII_to_BCD((unsigned char *) szBCD, szASCII, 2);
	memcpy((char *) &uszPackBuf[inCnt], &szBCD[0], 2);
	inCnt += 2;
	/* Packet Data */
	memcpy((char *) &uszPackBuf[inCnt], &szPacket[0], inPacketCnt);
	inCnt += inPacketCnt;
	
	inDISP_LogPrintfWithFlag(" Field 55 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack55_tSAM
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 3:42
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack55_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0, inPacketCnt = 0;
	char szASCII[4 + 1], szBCD[2 + 1];
	char szPacket[512 + 1];
	//char	szEncrypt[48 + 1];
	//BYTE	btField37[8 + 1], btField41[4 + 1];
	//BYTE	btTrack2[20];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	memset(szPacket, 0x00, sizeof (szPacket));

	if (pobTran->srEMVRec.in5F2A_TransCurrCodeLen > 0)
	{
		szPacket[inPacketCnt++] = 0x5F;
		szPacket[inPacketCnt++] = 0x2A;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz5F2A_TransCurrCode[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);
		inPacketCnt += pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
	}

	if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen > 0)
	{
		szPacket[inPacketCnt++] = 0x5F;
		szPacket[inPacketCnt++] = 0x34;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
		inPacketCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
	}

	if (pobTran->srEMVRec.in82_AIPLen > 0)
	{
		szPacket[inPacketCnt++] = 0x82;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in82_AIPLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz82_AIP[0], pobTran->srEMVRec.in82_AIPLen);
		inPacketCnt += pobTran->srEMVRec.in82_AIPLen;
	}

	if (pobTran->srEMVRec.in84_DFNameLen > 0)
	{
		szPacket[inPacketCnt++] = 0x84;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in84_DFNameLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
		inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;
	}

	if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) ||
			(pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY) ||
			(pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3) ||
			(pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE && (pobTran->inISOTxnCode == _SALE_ || pobTran->inISOTxnCode == _REVERSAL_)) ||
			(pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP && (pobTran->inISOTxnCode == _SALE_ || pobTran->inISOTxnCode == _REVERSAL_)))
	{

	} else
	{
		if (pobTran->inISOTxnCode == _TIP_ || pobTran->inISOTxnCode == _TC_UPLOAD_ || pobTran->inISOTxnCode == _ADJUST_ ||
				pobTran->inISOTxnCode == _BATCH_UPLOAD_ ||
				(pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE && pobTran->inISOTxnCode == _ADVICE_) ||
				(pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP && pobTran->inISOTxnCode == _ADVICE_) ||
				(!memcmp(&pobTran->srBRec.szAuthCode[0], "Y1", 2) && pobTran->srBRec.inChipStatus == _EMV_CARD_))
		{
			if (pobTran->srEMVRec.in8A_AuthRespCodeLen > 0)
			{
				szPacket[inPacketCnt++] = 0x8A;
				szPacket[inPacketCnt++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
				memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
				inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
			}
		}			/* Online �p�O Reversal�n�P���� by Wei Hsiu - 20130121 */
		else if (pobTran->inISOTxnCode == _REVERSAL_ && pobTran->srBRec.inCode == _TIP_)
		{
			if (pobTran->srEMVRec.in8A_AuthRespCodeLen > 0)
			{
				szPacket[inPacketCnt++] = 0x8A;
				szPacket[inPacketCnt++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
				memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
				inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
			}
		}
	}
	/* TAG 91 ARPC
		Issuer Authentication Data, mandatory if online issuer authentication is required.
		for Batch Upload only
	 */
	if (pobTran->inISOTxnCode == _BATCH_UPLOAD_)
	{
		if (pobTran->srEMVRec.in91_IssuerAuthDataLen > 0)
		{
			szPacket[inPacketCnt++] = 0x91;
			szPacket[inPacketCnt++] = pobTran->srEMVRec.in91_IssuerAuthDataLen;
			memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz91_IssuerAuthData[0], pobTran->srEMVRec.in91_IssuerAuthDataLen);
			inPacketCnt += pobTran->srEMVRec.in91_IssuerAuthDataLen;
		}
	}
	if (pobTran->srEMVRec.in95_TVRLen > 0)
	{
		szPacket[inPacketCnt++] = 0x95;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in95_TVRLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz95_TVR[0], pobTran->srEMVRec.in95_TVRLen);
		inPacketCnt += pobTran->srEMVRec.in95_TVRLen;
	}

	if (pobTran->srEMVRec.in9A_TranDateLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9A;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9A_TranDateLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);
		inPacketCnt += pobTran->srEMVRec.in9A_TranDateLen;
	}

	if (pobTran->srEMVRec.in9B_TSILen > 0)
	{
		szPacket[inPacketCnt++] = 0x9B;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9B_TSILen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
		inPacketCnt += pobTran->srEMVRec.in9B_TSILen;
	}

	if (pobTran->srEMVRec.in9C_TranTypeLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9C;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9C_TranTypeLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9C_TranType[0], pobTran->srEMVRec.in9C_TranTypeLen);
		inPacketCnt += pobTran->srEMVRec.in9C_TranTypeLen;
	}

	if (pobTran->srEMVRec.in9F02_AmtAuthNumLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x02;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F02_AmtAuthNumLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F02_AmtAuthNumLen;
	}

	if (pobTran->srEMVRec.in9F03_AmtOtherNumLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x03;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F03_AmtOtherNumLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F03_AmtOtherNum[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F03_AmtOtherNumLen;
	}

	if (pobTran->srEMVRec.in9F09_TermVerNumLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x09;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F09_TermVerNumLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F09_TermVerNum[0], pobTran->srEMVRec.in9F09_TermVerNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F09_TermVerNumLen;
	}

	if (pobTran->srEMVRec.in9F10_IssuerAppDataLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x10;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F10_IssuerAppDataLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F10_IssuerAppData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);
		inPacketCnt += pobTran->srEMVRec.in9F10_IssuerAppDataLen;
	}

	if (pobTran->srEMVRec.in9F1A_TermCountryCodeLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x1A;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F1A_TermCountryCode[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);
		inPacketCnt += pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
	}

	if (pobTran->srEMVRec.in9F1E_IFDNumLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x1E;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F1E_IFDNumLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
	}

	if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x26;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F26_ApplCryptogramLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F26_ApplCryptogram[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);
		inPacketCnt += pobTran->srEMVRec.in9F26_ApplCryptogramLen;
	}

	if (pobTran->srEMVRec.in9F27_CIDLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x27;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F27_CIDLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F27_CID[0], pobTran->srEMVRec.in9F27_CIDLen);
		inPacketCnt += pobTran->srEMVRec.in9F27_CIDLen;
	}

	if (pobTran->srEMVRec.in9F33_TermCapabilitiesLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x33;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F33_TermCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
		inPacketCnt += pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
	}

	if (pobTran->srEMVRec.in9F34_CVMLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x34;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F34_CVMLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
		inPacketCnt += pobTran->srEMVRec.in9F34_CVMLen;
	}

	if (pobTran->srEMVRec.in9F35_TermTypeLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x35;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F35_TermTypeLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
		inPacketCnt += pobTran->srEMVRec.in9F35_TermTypeLen;
	}

	if (pobTran->srEMVRec.in9F36_ATCLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x36;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F36_ATCLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F36_ATC[0], pobTran->srEMVRec.in9F36_ATCLen);
		inPacketCnt += pobTran->srEMVRec.in9F36_ATCLen;
	}

	if (pobTran->srEMVRec.in9F37_UnpredictNumLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x37;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F37_UnpredictNumLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F37_UnpredictNum[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F37_UnpredictNumLen;
	}

	/* [�I��] TC_UPLOAD�MISR UPLOAD�~�e9F41 */
	if (pobTran->inISOTxnCode == _TC_UPLOAD_ || pobTran->inISOTxnCode == _ISR_UPLOAD_)
	{
		if (pobTran->srEMVRec.in9F41_TransSeqCounterLen > 0)
		{
			szPacket[inPacketCnt++] = 0x9F;
			szPacket[inPacketCnt++] = 0x41;
			szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F41_TransSeqCounterLen;
			memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
			inPacketCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
		}
	}

	if (pobTran->srEMVRec.in9F5B_ISRLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x5B;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F5B_ISRLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F5B_ISR[0], pobTran->srEMVRec.in9F5B_ISRLen);
		inPacketCnt += pobTran->srEMVRec.in9F5B_ISRLen;
	}

	if (pobTran->srBRec.inChipStatus == _EMV_CARD_ && pobTran->srEMVRec.in9F63_CardProductLabelInformationLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x63;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F63_CardProductLabelInformationLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F63_CardProductLabelInformation[0], pobTran->srEMVRec.in9F63_CardProductLabelInformationLen);
		inPacketCnt += pobTran->srEMVRec.in9F63_CardProductLabelInformationLen;
	}

	/* F_55 �[ 9F6E, Add by sunny 2017/05/09 */
	/* Enhance Contactless Reader Capabilities (START)*/
	/* EMV Contactless Bool C-4 Kernel 4 Spec V2.5 */
	if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
	{
		if (pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen > 0)
		{
			szPacket[inPacketCnt++] = 0x9F;
			szPacket[inPacketCnt++] = 0x6E;
			szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen;
			memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F6E_From_Factor_Indicator[0], pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen);
			inPacketCnt += pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen;
		}
	}
	/* Enhance Contactless Reader Capabilities (END)*/

	/* Packet Data Length */
	memset(szASCII, 0x00, sizeof (szASCII));
	sprintf(szASCII, "%04d", inPacketCnt);
	memset(szBCD, 0x00, sizeof (szBCD));
	inFunc_ASCII_to_BCD((unsigned char *) szBCD, szASCII, 2);
	memcpy((char *) &uszPackBuf[inCnt], &szBCD[0], 2);
	inCnt += 2;
	/* Packet Data */
	memcpy((char *) &uszPackBuf[inCnt], &szPacket[0], inPacketCnt);
	inCnt += inPacketCnt;

	inDISP_LogPrintfWithFlag(" Field 55 tSAM PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack55_Software
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 3:43
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack55_Software(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0, inPacketCnt = 0;
	char szASCII[4 + 1], szBCD[2 + 1];
	char szSTAN[6 + 1], szField11[3 + 1];
	char szEncrypt[48 + 1], szPacket[512 + 1];
	short inSoftwareKey;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memset(szPacket, 0x00, sizeof (szPacket));

	memset(szField11, 0x00, sizeof (szField11));
	memset(szSTAN, 0x00, sizeof (szSTAN));

	if (pobTran->srBRec.uszOfflineBit == VS_TRUE &&
		pobTran->srBRec.uszUpload1Bit == VS_TRUE &&
		pobTran->srBRec.uszUpload2Bit == VS_TRUE)
		sprintf(szSTAN, "%06ld", pobTran->srBRec.lnSTAN_Multi);
	else
		sprintf(szSTAN, "%06ld", pobTran->srBRec.lnSTANNum);

	inFunc_ASCII_to_BCD((unsigned char *) &szField11[0], &szSTAN[0], 3);
	inSoftwareKey = (unsigned char) szField11[2] & 0x0f;

	memset(szEncrypt, 0x00, sizeof (szEncrypt));

	if (pobTran->srEMVRec.in5F2A_TransCurrCodeLen > 0)
	{
		szPacket[inPacketCnt++] = 0x5F;
		szPacket[inPacketCnt++] = 0x2A;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz5F2A_TransCurrCode[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);
		inPacketCnt += pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
	}

	if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen > 0)
	{
		szPacket[inPacketCnt++] = 0x5F;
		szPacket[inPacketCnt++] = 0x34;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
		inPacketCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
	}

	if (pobTran->srEMVRec.in82_AIPLen > 0)
	{
		szPacket[inPacketCnt++] = 0x82;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in82_AIPLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz82_AIP[0], pobTran->srEMVRec.in82_AIPLen);
		inPacketCnt += pobTran->srEMVRec.in82_AIPLen;
	}

	if (pobTran->srEMVRec.in84_DFNameLen > 0)
	{
		szPacket[inPacketCnt++] = 0x84;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in84_DFNameLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
		inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;
	}

	if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) ||
		(pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY) ||
		(pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3) ||
		(pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE && (pobTran->inISOTxnCode == _SALE_ || pobTran->inISOTxnCode == _REVERSAL_)) ||
		(pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP && (pobTran->inISOTxnCode == _SALE_ || pobTran->inISOTxnCode == _REVERSAL_)))
	{

	} else
	{
		if (pobTran->inISOTxnCode == _TIP_ || pobTran->inISOTxnCode == _TC_UPLOAD_ || pobTran->inISOTxnCode == _ADJUST_ ||
			pobTran->inISOTxnCode == _BATCH_UPLOAD_ ||
			(pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE && pobTran->inISOTxnCode == _ADVICE_) ||
			(pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP && pobTran->inISOTxnCode == _ADVICE_) ||
			(!memcmp(&pobTran->srBRec.szAuthCode[0], "Y1", 2) && pobTran->srBRec.inChipStatus == _EMV_CARD_))
		{
			if (pobTran->srEMVRec.in8A_AuthRespCodeLen > 0)
			{
				szPacket[inPacketCnt++] = 0x8A;
				szPacket[inPacketCnt++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
				memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
				inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
			}
		}
		else if (pobTran->inISOTxnCode == _REVERSAL_ && pobTran->srBRec.inCode == _TIP_)
		{
			if (pobTran->srEMVRec.in8A_AuthRespCodeLen > 0)
			{
				szPacket[inPacketCnt++] = 0x8A;
				szPacket[inPacketCnt++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
				memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
				inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
			}
		}
	}
	/* TAG 91 ARPC
		Issuer Authentication Data, mandatory if online issuer authentication is required.
		for Batch Upload only
	 */
	if (pobTran->inISOTxnCode == _BATCH_UPLOAD_)
	{
		if (pobTran->srEMVRec.in91_IssuerAuthDataLen > 0)
		{
			szPacket[inPacketCnt++] = 0x91;
			szPacket[inPacketCnt++] = pobTran->srEMVRec.in91_IssuerAuthDataLen;
			memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz91_IssuerAuthData[0], pobTran->srEMVRec.in91_IssuerAuthDataLen);
			inPacketCnt += pobTran->srEMVRec.in91_IssuerAuthDataLen;
		}
	}

	if (pobTran->srEMVRec.in95_TVRLen > 0)
	{
		szPacket[inPacketCnt++] = 0x95;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in95_TVRLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz95_TVR[0], pobTran->srEMVRec.in95_TVRLen);
		inPacketCnt += pobTran->srEMVRec.in95_TVRLen;
	}

	if (pobTran->srEMVRec.in9A_TranDateLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9A;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9A_TranDateLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);
		inPacketCnt += pobTran->srEMVRec.in9A_TranDateLen;
	}

	if (pobTran->srEMVRec.in9B_TSILen > 0)
	{
		szPacket[inPacketCnt++] = 0x9B;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9B_TSILen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
		inPacketCnt += pobTran->srEMVRec.in9B_TSILen;
	}

	if (pobTran->srEMVRec.in9C_TranTypeLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9C;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9C_TranTypeLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9C_TranType[0], pobTran->srEMVRec.in9C_TranTypeLen);
		inPacketCnt += pobTran->srEMVRec.in9C_TranTypeLen;
	}

	if (pobTran->srEMVRec.in9F02_AmtAuthNumLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x02;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F02_AmtAuthNumLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F02_AmtAuthNumLen;
	}

	if (pobTran->srEMVRec.in9F03_AmtOtherNumLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x03;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F03_AmtOtherNumLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F03_AmtOtherNum[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F03_AmtOtherNumLen;
	}

	if (pobTran->srEMVRec.in9F09_TermVerNumLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x09;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F09_TermVerNumLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F09_TermVerNum[0], pobTran->srEMVRec.in9F09_TermVerNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F09_TermVerNumLen;
	}

	if (pobTran->srEMVRec.in9F10_IssuerAppDataLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x10;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F10_IssuerAppDataLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F10_IssuerAppData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);
		inPacketCnt += pobTran->srEMVRec.in9F10_IssuerAppDataLen;
	}

	if (pobTran->srEMVRec.in9F1A_TermCountryCodeLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x1A;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F1A_TermCountryCode[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);
		inPacketCnt += pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
	}

	if (pobTran->srEMVRec.in9F1E_IFDNumLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x1E;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F1E_IFDNumLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
	}

	if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x26;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F26_ApplCryptogramLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F26_ApplCryptogram[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);
		inPacketCnt += pobTran->srEMVRec.in9F26_ApplCryptogramLen;
	}

	if (pobTran->srEMVRec.in9F27_CIDLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x27;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F27_CIDLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F27_CID[0], pobTran->srEMVRec.in9F27_CIDLen);
		inPacketCnt += pobTran->srEMVRec.in9F27_CIDLen;
	}

	if (pobTran->srEMVRec.in9F33_TermCapabilitiesLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x33;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F33_TermCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
		inPacketCnt += pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
	}

	if (pobTran->srEMVRec.in9F34_CVMLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x34;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F34_CVMLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
		inPacketCnt += pobTran->srEMVRec.in9F34_CVMLen;
	}

	if (pobTran->srEMVRec.in9F35_TermTypeLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x35;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F35_TermTypeLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
		inPacketCnt += pobTran->srEMVRec.in9F35_TermTypeLen;
	}

	if (pobTran->srEMVRec.in9F36_ATCLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x36;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F36_ATCLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F36_ATC[0], pobTran->srEMVRec.in9F36_ATCLen);
		inPacketCnt += pobTran->srEMVRec.in9F36_ATCLen;
	}

	if (pobTran->srEMVRec.in9F37_UnpredictNumLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x37;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F37_UnpredictNumLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F37_UnpredictNum[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F37_UnpredictNumLen;
	}

	if (pobTran->inISOTxnCode == _TC_UPLOAD_ || pobTran->inISOTxnCode == _ISR_UPLOAD_)
	{
		if (pobTran->srEMVRec.in9F41_TransSeqCounterLen > 0)
		{
			szPacket[inPacketCnt++] = 0x9F;
			szPacket[inPacketCnt++] = 0x41;
			szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F41_TransSeqCounterLen;
			memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
			inPacketCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
		}
	}

	if (pobTran->srEMVRec.in9F5B_ISRLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x5B;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F5B_ISRLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F5B_ISR[0], pobTran->srEMVRec.in9F5B_ISRLen);
		inPacketCnt += pobTran->srEMVRec.in9F5B_ISRLen;
	}

	if (pobTran->srBRec.inChipStatus == _EMV_CARD_ && pobTran->srEMVRec.in9F63_CardProductLabelInformationLen > 0)
	{
		szPacket[inPacketCnt++] = 0x9F;
		szPacket[inPacketCnt++] = 0x63;
		szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F63_CardProductLabelInformationLen;
		memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F63_CardProductLabelInformation[0], pobTran->srEMVRec.in9F63_CardProductLabelInformationLen);
		inPacketCnt += pobTran->srEMVRec.in9F63_CardProductLabelInformationLen;
	}

	/* F_55 �[ 9F6E, Add by sunny 2017/05/09 */
	/* Enhance Contactless Reader Capabilities (START)*/
	/* EMV Contactless Bool C-4 Kernel 4 Spec V2.5 */
	if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
	{
		if (pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen > 0)
		{
			szPacket[inPacketCnt++] = 0x9F;
			szPacket[inPacketCnt++] = 0x6E;
			szPacket[inPacketCnt++] = pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen;
			memcpy(&szPacket[inPacketCnt], (char *) &pobTran->srEMVRec.usz9F6E_From_Factor_Indicator[0], pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen);
			inPacketCnt += pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen;
		}
	}
	/* Enhance Contactless Reader Capabilities (END)*/

	/* Packet Data Length */
	memset(szASCII, 0x00, sizeof (szASCII));
	sprintf(szASCII, "%04d", inPacketCnt);
	memset(szBCD, 0x00, sizeof (szBCD));
	inFunc_ASCII_to_BCD((unsigned char *) szBCD, szASCII, 2);
	memcpy((char *) &uszPackBuf[inCnt], &szBCD[0], 2);
	inCnt += 2;
	/* Packet Data */
	memcpy((char *) &uszPackBuf[inCnt], &szPacket[0], inPacketCnt);
	inCnt += inPacketCnt;
	
	inDISP_LogPrintfWithFlag(" Field 55 Software PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack56
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 3:46
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack56(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0, inPacketCnt = 0;
	char szASCII[4 + 1], szBCD[2 + 1];
	char szPacket[100 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	memset(szPacket, _NULL_CH_, sizeof (szPacket));

	szPacket[inPacketCnt++] = 0x53; /* Table ID = S0 */
	szPacket[inPacketCnt++] = 0x30;
	
	if (pobTran->srBRec.szStore_DREAM_MALL[0] == 0x31)
	{
		szPacket[inPacketCnt++] = 'M';
		memcpy((char *) &szPacket[inPacketCnt], &pobTran->srBRec.szStore_DREAM_MALL[1], 10);
		inPacketCnt += 10;
		szPacket[inPacketCnt++] = 'C';
		memcpy((char *) &szPacket[inPacketCnt], &pobTran->srBRec.szStore_DREAM_MALL[11], 20);
		inPacketCnt += 20;
	} else
	{
		memset(pobTran->srBRec.szStore_DREAM_MALL, 0x20, sizeof (pobTran->srBRec.szStore_DREAM_MALL));
		szPacket[inPacketCnt++] = 'M';
		memcpy((char *) &szPacket[inPacketCnt], &pobTran->srBRec.szStore_DREAM_MALL[1], 10);
		inPacketCnt += 10;
		szPacket[inPacketCnt++] = 'C';
		memcpy((char *) &szPacket[inPacketCnt], &pobTran->srBRec.szStore_DREAM_MALL[11], 20);
		inPacketCnt += 20;
	}

	/* Packet Data Length */
	memset(szASCII, 0x00, sizeof (szASCII));
	sprintf(szASCII, "%04d", inPacketCnt);
	memset(szBCD, 0x00, sizeof (szBCD));
	inFunc_ASCII_to_BCD((unsigned char *) szBCD, szASCII, 2);
	memcpy((char *) &uszPackBuf[inCnt], &szBCD[0], 2);
	inCnt += 2;
	/* Packet Data */
	memcpy((char *) &uszPackBuf[inCnt], &szPacket[0], inPacketCnt);
	inCnt += inPacketCnt;

	inDISP_LogPrintfWithFlag(" Field 56 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack59
 * App Builder		: 
 * App Date&Time	: 2022/8/9 上午 10:19
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0, inPacketCnt = 0;
	char szASCII[4 + 1], szBCD[2 + 1];
	char szTmpField59[150 + 1];
	char EncrptyCmk[16 + 1];
	char szPinPadSnBuf[20];
	char szSTAN[6 + 1];
	char CupTemp[10];
	char szRRNTemp[12 + 1];
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" TxnCode[%d] ", pobTran->inISOTxnCode);
	
	memset(szTmpField59, _NULL_CH_, sizeof (szTmpField59));
	
	inFUBON_PackTableA1(pobTran, (unsigned char*)&szTmpField59[inPacketCnt], &inPacketCnt);	
	
	/* 只有ECR交易且為當筆正向銷售交易要送，才會有 B1 2022/10/19 [SAM] */
	if(pobTran->uszEcrTransFlag == 'Y')
	{
		if(pobTran->inISOTxnCode == _SALE_ || 
			pobTran->inISOTxnCode == _REDEEM_SALE_ || 
			pobTran->inISOTxnCode == _INST_SALE_ ||
			pobTran->inISOTxnCode == _CUP_SALE_)
		{
			inFUBON_PackTableB1(pobTran, (unsigned char*)&szTmpField59[inPacketCnt], &inPacketCnt);	
		}
	}
	
	switch (pobTran->inISOTxnCode)
	{
		/* [新增預授權完成]  取消要上傳 N1 Table 2022/10/19  [SAM] */
		case _VOID_:
			if(pobTran->srBRec.inOrgCode == _PRE_COMP_)
			{
				inFUBON_PackTableN1(pobTran, (unsigned char*)&szTmpField59[inPacketCnt], &inPacketCnt);
			}
			break;
		/* [新增預授權完成]  N1 Table 2022/5/11 [SAM] */
		case _PRE_COMP_:
			inFUBON_PackTableN1(pobTran, (unsigned char*)&szTmpField59[inPacketCnt], &inPacketCnt);
			break;
			/* Table ID C1 */
		case _CUP_SALE_:
		case _CUP_PRE_AUTH_:
			inFUBON_PackTableC1_SendInitialData(pobTran, (unsigned char*)&szTmpField59[inPacketCnt], &inPacketCnt);
			/* Pin Block Present */
			if ((pobTran->inISOTxnCode == _CUP_SALE_ && (pobTran->szPIN[0] != ' ' && pobTran->szPIN[0] != 0x00)) ||
				(pobTran->inISOTxnCode == _CUP_PRE_AUTH_ && (pobTran->szPIN[0] != ' ' && pobTran->szPIN[0] != 0x00)))
			{
				inFUBON_PackTableP1(pobTran, (unsigned char*)&szTmpField59[inPacketCnt], &inPacketCnt);
			}
			break;
			/* TCUPLOAD Table ID C1*/
		case _ISR_UPLOAD_:
		case _TC_UPLOAD_:
			
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inFUBON_PackTableC1_SendReceivedData(pobTran, (unsigned char*)&szTmpField59[inPacketCnt], &inPacketCnt);
			}
			
			break;
		case _CUP_VOID_:
		case _CUP_PRE_AUTH_VOID_:
		case _CUP_REFUND_:
		case _CUP_PRE_COMP_:
		case _CUP_PRE_COMP_VOID_:
			inFUBON_PackTableC1_SendReceivedData(pobTran, (unsigned char*)&szTmpField59[inPacketCnt], &inPacketCnt);
			break;
			/* TMK Request=P1+P4, Response=P3
			     TPK  Request=P1   , Response=P2
			 */
		case _CHANGE_TMK_:
			inFUBON_PackTableP1(pobTran, (unsigned char*)&szTmpField59[inPacketCnt], &inPacketCnt);
			if(VS_ERROR == inFUBON_PackTableP4(pobTran, (unsigned char*)&szTmpField59[inPacketCnt], &inPacketCnt)){
				return VS_ERROR;
			}
			break;
		case _CUP_CHANG_TPK_:
			inFUBON_PackTableP1(pobTran, (unsigned char*)&szTmpField59[inPacketCnt], &inPacketCnt);
			break;
		case _BATCH_UPLOAD_:

			if (pobTran->srBRec.inCode == _PRE_COMP_ )
			{
				inFUBON_PackTableN1(pobTran, (unsigned char*)&szTmpField59[inPacketCnt], &inPacketCnt);
			}
			
			if (pobTran->srBRec.inCode == _CUP_SALE_ ||
				pobTran->srBRec.inCode == _CUP_REFUND_ ||
				pobTran->srBRec.inCode == _CUP_PRE_COMP_)
			{
				inFUBON_PackTableC1_SendReceivedData(pobTran, (unsigned char*)&szTmpField59[inPacketCnt], &inPacketCnt);
			}
			break;
			/* Table ID A2 */
		case _REVERSAL_:
			/* STAND IN */
			inFUBON_PackTableA2(pobTran, (unsigned char*)&szTmpField59[inPacketCnt], &inPacketCnt);
			
			switch (pobTran->srBRec.inCode)
			{
				/* 修改 20221110，信用卡取消授權完成的Reversal，F_59未上傳Table ID N1，導致沖銷F_39會被回[05]  
				    2022/11/16 [SAM] */
				case _VOID_:
					if(pobTran->srBRec.inOrgCode == _PRE_COMP_)
						inFUBON_PackTableN1(pobTran, (unsigned char*)&szTmpField59[inPacketCnt], &inPacketCnt);
					break;
				case _PRE_COMP_:
					inFUBON_PackTableN1(pobTran, (unsigned char*)&szTmpField59[inPacketCnt], &inPacketCnt);
					break;
				case _CUP_SALE_:
				case _CUP_VOID_:
				case _CUP_REFUND_:
				case _CUP_PRE_COMP_:
				case _CUP_PRE_AUTH_:
				case _CUP_PRE_AUTH_VOID_:
				case _CUP_PRE_COMP_VOID_:
					inFUBON_PackTableC1_ReversalSendReceivedData(pobTran, (unsigned char*)&szTmpField59[inPacketCnt], &inPacketCnt);
					break;
				default:
					break;
			}/* End switch () .... */


		default:
			break;
	} /* End switch () .... */

	/* Packet Data Length */
	memset(szASCII, 0x00, sizeof (szASCII));
	sprintf(szASCII, "%04d", inPacketCnt);
	memset(szBCD, 0x00, sizeof (szBCD));
	inFunc_ASCII_to_BCD((unsigned char *) szBCD, szASCII, 2);
	memcpy((char *) &uszPackBuf[inCnt], &szBCD[0], 2);
	inCnt += 2;
	/* Packet Data */
	memcpy((char *) &uszPackBuf[inCnt], &szTmpField59[0], inPacketCnt);
	inCnt += inPacketCnt;

	inDISP_LogPrintfWithFlag(" Field 59 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack60
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 3:55
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;
	char szMsgTypeId[4 + 1];
	char szScratch[23];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	/* Batch Upload */
	if ((pobTran->inISOTxnCode == _TIP_))
	{
		uszPackBuf[inCnt++] = 0x00;
		uszPackBuf[inCnt++] = 0x12;
		sprintf((char *) &uszPackBuf[inCnt], "%010ld00", pobTran->srBRec.lnTxnAmount);
		inCnt += 12;
	} else if ((pobTran->inISOTxnCode == _ADJUST_))
	{
		uszPackBuf[inCnt++] = 0x00;
		uszPackBuf[inCnt++] = 0x12;
		sprintf((char *) &uszPackBuf[inCnt], "%010ld00", pobTran->srBRec.lnOrgTxnAmount);
		inCnt += 12;

	} else if ((pobTran->inISOTxnCode == _BATCH_UPLOAD_))
	{

		memset(szMsgTypeId, 0x00, sizeof (szMsgTypeId));
		memset(szScratch, 0x00, sizeof (szScratch));

		switch (pobTran->srBRec.inCode)
		{
			case _SALE_:
			case _CUP_SALE_:
			case _INST_SALE_:
			case _REDEEM_SALE_:
			case _REFUND_:
			case _CUP_REFUND_:
			case _INST_REFUND_:
			case _REDEEM_REFUND_:
			case _PRE_COMP_:
			case _CUP_PRE_COMP_:
				memcpy(szMsgTypeId, "\x02\x00", 2);
				break;
			case _TIP_:
			case _SALE_OFFLINE_:
			case _ADJUST_:
			case _INST_ADJUST_:
			case _REDEEM_ADJUST_:
				memcpy(szMsgTypeId, "\x02\x20", 2);
				break;
			default:
				break;
		}

		if (pobTran->srBRec.uszOfflineBit == VS_TRUE &&
				pobTran->srBRec.uszUpload1Bit == VS_TRUE &&
				pobTran->srBRec.uszUpload2Bit == VS_TRUE)
		{
			sprintf(szScratch, "%02x%02x%06ld%12s", szMsgTypeId[0],
					szMsgTypeId[1],
					pobTran->srBRec.lnSTAN_Multi,
					pobTran->srBRec.szRefNo);
		} else
		{
			sprintf(szScratch, "%02x%02x%06ld%12s", szMsgTypeId[0],
					szMsgTypeId[1],
					pobTran->srBRec.lnSTANNum,
					pobTran->srBRec.szRefNo);
		}
		inFunc_PAD_ASCII(szScratch, szScratch, ' ', 22, _PAD_LEFT_FILL_RIGHT_);

		uszPackBuf[inCnt++] = 0x00;
		uszPackBuf[inCnt++] = 0x22;
		memcpy((char *) &uszPackBuf[inCnt], szScratch, 22);
		inCnt += 22;
	} else
	{
		if (pobTran->inISOTxnCode == _REVERSAL_ && pobTran->srBRec.inCode == _TIP_)
		{
			uszPackBuf[inCnt++] = 0x00;
			uszPackBuf[inCnt++] = 0x12;
			sprintf((char *) &uszPackBuf[inCnt], "%010ld00", pobTran->srBRec.lnTxnAmount);
			inCnt += 12;
		}
		else if (pobTran->inISOTxnCode == _REVERSAL_ && pobTran->srBRec.inCode == _ADJUST_)
		{
			uszPackBuf[inCnt++] = 0x00;
			uszPackBuf[inCnt++] = 0x12;
			sprintf((char *) &uszPackBuf[inCnt], "%010ld00", pobTran->srBRec.lnTxnAmount);
			inCnt += 12;
		} else
		{
			char szTemplate[10 + 1];
			long lnBatchNum = 0L;

			uszPackBuf[inCnt++] = 0x00;
			uszPackBuf[inCnt++] = 0x06;
			memset(szTemplate, 0x00, sizeof (szTemplate));
			if (inGetBatchNum(szTemplate) == VS_SUCCESS)
				lnBatchNum = atol(szTemplate);
			sprintf((char *) &uszPackBuf[inCnt], "%06ld", lnBatchNum);
			inCnt += 6;
		}
	}

	inDISP_LogPrintfWithFlag(" Field 60 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack62
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 3:58
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	uszPackBuf[inCnt++] = 0x00;
	uszPackBuf[inCnt++] = 0x06;
	sprintf((char *) &uszPackBuf[inCnt], "%06ld", pobTran->srBRec.lnOrgInvNum);
	inCnt += 6;
	
	inDISP_LogPrintfWithFlag(" Field 62 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
 * App Name		: inFUBON_ISO_Pack63
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 3:59
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack63(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0, inTxnCode = -1;
	int inAccumCnt, inBatchCnt, inPacketCnt = 0;
	char szASCII[4 + 1], szBCD[2 + 1];
	char szTemplate[100], szPacket[100 + 1];
	ACCUM_TOTAL_REC srAccumRec;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	if (pobTran->inISOTxnCode == _REVERSAL_)
	{
		if (pobTran->srBRec.uszInstallmentBit == VS_TRUE && pobTran->inTransactionCode != _INST_SALE_)
			inTxnCode = _INST_DETAIL_;
		else if (pobTran->srBRec.uszRedeemBit == VS_TRUE && pobTran->inTransactionCode != _REDEEM_SALE_)
			inTxnCode = _REDEEM_DETAIL_;
		else
			inTxnCode = pobTran->inTransactionCode;
	} else if (pobTran->inISOTxnCode == _VOID_ ||
			pobTran->inISOTxnCode == _BATCH_UPLOAD_ ||
			pobTran->inISOTxnCode == _TIP_ ||
			pobTran->inISOTxnCode == _TC_UPLOAD_)
	{
		if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
			inTxnCode = _INST_DETAIL_;
		else if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
			inTxnCode = _REDEEM_DETAIL_;
		else
			inTxnCode = pobTran->srBRec.inOrgCode;
	} else
		inTxnCode = pobTran->inISOTxnCode;

	switch (inTxnCode)
	{
		case _SETTLE_:
		case _CLS_BATCH_:
			memset(szPacket, 0x00, sizeof (szPacket));
			inBatchCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
			if (inBatchCnt == VS_ERROR)
			{
				inDISP_DispLogAndWriteFlie(" FU Pack63 GetTotal DB *Error* Line[%d] ", __LINE__);
				return (VS_ERROR);
			}else if (inBatchCnt == VS_NO_RECORD)
			{
				strcpy(szPacket, "000000000000000");
				strcat(szPacket, "000000000000000");
				inPacketCnt += 30;
			} else
			{
				memset(&srAccumRec, 0x00, sizeof (srAccumRec));
				inAccumCnt = inACCUM_GetRecord(pobTran, &srAccumRec);
				if (inAccumCnt == VS_NO_RECORD)
				{
					strcpy(szPacket, "000000000000000");
					strcat(szPacket, "000000000000000");
					inPacketCnt += 30;
				} else if (inAccumCnt == VS_ERROR)
				{
					inDISP_DispLogAndWriteFlie(" FU Pack63 GetTotal AccumCntFile *Error* Line[%d] ", __LINE__);
					return (VS_ERROR);
				}else
				{
					/* _SALE_ Count */
					/* 因為資料庫與儲存金額的檔案不合，所以要造成不平帳上傳
					 * 規則是把筆數加10 2020/5/15 上午 11:40 [SAM] */
					if (1 == inFUBON_GetFuUnBlanceSettleFlag())
					{
						inDISP_DispLogAndWriteFlie(" FU Pack63 UnBlanceSettle *Error* Line[%d]", __LINE__);
						memset(szTemplate, 0x00, sizeof (szTemplate));
						sprintf(szTemplate, "%03lu", (srAccumRec.lnTotalSaleCount + 10));
					} else
					{
						memset(szTemplate, 0x00, sizeof (szTemplate));
						sprintf(szTemplate, "%03lu", srAccumRec.lnTotalSaleCount);
					}
					strcpy(szPacket, szTemplate);
					inPacketCnt += 3;
					/* _SALE_ Amount */
					sprintf(szTemplate, "%010ld00", ((long) srAccumRec.llTotalSaleAmount + (long) srAccumRec.llTotalTipsAmount));
					strcat(szPacket, szTemplate);
					inPacketCnt += 12;
					/* _REFUND_ Count */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					sprintf(szTemplate, "%03lu", srAccumRec.lnTotalRefundCount);
					strcat(szPacket, szTemplate);
					inPacketCnt += 3;
					/* _REFUND_ Amount */
					sprintf(szTemplate, "%010ld00", (0 - (long) srAccumRec.llTotalRefundAmount));
					strcat(szPacket, szTemplate);
					inPacketCnt += 12;
				}
			}

			strcat(szPacket, "000000000000000000000000000000");
			strcat(szPacket, "000000000000000000000000000000");
			inPacketCnt += 60;

			break;

		case _INST_SALE_:
			/* (TX-Indicator) */
//                                memset(szTemplate, 0x20, sizeof(szTemplate));
//                                memset(ch_INST_I_OR_E, _NULL_CH_, sizeof(ch_INST_I_OR_E));
//                                ch_INST_I_OR_E[get_env("#INSTIE", ch_INST_I_OR_E, sizeof(ch_INST_I_OR_E))] = 0x00;
//                                memcpy(&szTemplate[0], &ch_INST_I_OR_E[0], 1);

			/* Indicator & Period */
			/* (Goods ID/Product code)*/
			/* (Period Numbers)*/
			sprintf(szPacket, "I%06lu%02lu", pobTran->srBRec.lnFubonGoodsID,
					pobTran->srBRec.lnInstallmentPeriod);
			inPacketCnt = 9;
			break;
		case _INST_REFUND_:
		case _INST_ADJUST_:
		case _INST_DETAIL_:
			/* Indicator (1 Byte) */
			memcpy(&szPacket[0], &pobTran->srBRec.szInstallmentIndicator[0], 1);
			inPacketCnt++;
			/* Goods ID/Product code)*/
			sprintf(&szPacket[1], "%06lu", pobTran->srBRec.lnFubonGoodsID);
			inPacketCnt += 6;

			memset(szTemplate, 0x00, sizeof (szTemplate));
			/*
			Period Numbers		an	2
			TX-Response code	an	2
			Down Payment		an	8
			_INST_SALE_ Payment	an	8
			_INST_SALE_ Fee		an	8
			 */
			sprintf(szTemplate, "%02lu00%08lu%08lu%08lu",
					pobTran->srBRec.lnInstallmentPeriod,
					pobTran->srBRec.lnInstallmentDownPayment + pobTran->srBRec.lnTipTxnAmount,
					pobTran->srBRec.lnInstallmentPayment,
					pobTran->srBRec.lnInstallmentFormalityFee);
			strcat(szPacket, szTemplate);
			inPacketCnt += 28;
			break;
		case _REDEEM_SALE_:
			/* Indicator */
			strcpy(szPacket, "1");
			inPacketCnt = 1;
			break;
		case _REDEEM_REFUND_:
		case _REDEEM_ADJUST_:
		case _REDEEM_DETAIL_:
			/* Indicator (1 Byte) */
			memcpy(&szPacket[0], &pobTran->srBRec.szRedeemIndicator[0], 1);
			inPacketCnt++;

			/* (Goods ID/Product code)*/
			sprintf(&szPacket[1], "%06lu", pobTran->srBRec.lnFubonGoodsID);
			inPacketCnt += 6;

			/*
			TX-Response code		an	2
			Points of _REDEEM_SALE_	an	8
			Points of Balance		an	8
			Amount of _REDEEM_SALE_	an	8
			Paid credit amount		an	8
			 */

			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "00%08lu%08lu%08lu%08lu",
					pobTran->srBRec.lnRedemptionPoints,
					pobTran->srBRec.lnRedemptionPointsBalance,
					pobTran->srBRec.lnRedemptionPaidAmount,
					(pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
			strcat(szPacket, szTemplate);
			inPacketCnt += 34;
			break;
		default:
			break;
	}


	/* Packet Data Length */
	memset(szASCII, 0x00, sizeof (szASCII));
	sprintf(szASCII, "%04d", inPacketCnt);
	memset(szBCD, 0x00, sizeof (szBCD));
	inFunc_ASCII_to_BCD((unsigned char *) szBCD, szASCII, 2);
	memcpy((char *) &uszPackBuf[inCnt], &szBCD[0], 2);
	inCnt += 2;
	/* Packet Data */
	memcpy((char *) &uszPackBuf[inCnt], &szPacket[0], inPacketCnt);
	inCnt += inPacketCnt;

	inDISP_LogPrintfWithFlag(" Field 63 PackLen[%ld] ", inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inCnt);

}

/*
 * App Name		: inFUBON_ISO_Pack64
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 4:06
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Pack64(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
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
 * App Name		: inFUBON_ISO_Check03
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 4:10
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Check03(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	if (memcmp((char *) uszSendPacket, (char *) uszRecePacket, 3))
	{
		inDISP_DispLogAndWriteFlie(" FU ISO Check03 *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * App Name		: inFUBON_ISO_Check04
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 4:29
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Check04(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	if (memcmp((char *) uszSendPacket, (char *) uszRecePacket, 6))
	{
		inDISP_DispLogAndWriteFlie(" FU ISO Check03 *Error* Line[%d]", __LINE__);
		inDISP_Msg_BMP(_ERR_ISO_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * App Name		: inFUBON_ISO_Check41
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 4:30
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_Check41(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	if (memcmp((char *) uszSendPacket, (char *) uszRecePacket, 8))
	{
		inDISP_DispLogAndWriteFlie(" FU ISO Check41 *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * App Name		: inFUBON_ISO_UnPack12
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 4:31
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_UnPack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	/* 銷售前若有一筆Reversal的RRN交易，則此筆交易Timout的Reversal會帶到前筆RRN的資料 
	  新增  _ISR_UPLOAD_ 條件 2022/11/16 [SAM] */
	if (pobTran->inISOTxnCode == _REVERSAL_ || pobTran->inISOTxnCode ==_ISR_UPLOAD_)
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
 * App Name		: inFUBON_ISO_UnPack13
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 4:33
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_UnPack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	/* 銷售前若有一筆Reversal的RRN交易，則此筆交易Timout的Reversal會帶到前筆RRN的資料 
	  新增  _ISR_UPLOAD_ 條件 2022/11/16 [SAM] */
	if (pobTran->inISOTxnCode == _REVERSAL_ || pobTran->inISOTxnCode ==_ISR_UPLOAD_)
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
 * App Name		: inFUBON_ISO_UnPack24
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 4:33
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_UnPack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	char szNPS[8];
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	/* 銷售前若有一筆Reversal的RRN交易，則此筆交易Timout的Reversal會帶到前筆RRN的資料 
	  新增 Reversal  _ISR_UPLOAD_ 條件 2022/11/16 [SAM] */
	if (pobTran->inISOTxnCode == _REVERSAL_ || pobTran->inISOTxnCode ==_ISR_UPLOAD_)
	{
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Reversal END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}
	else
	{
		memset(szNPS, 0x00, sizeof (szNPS));
		inFunc_BCD_to_ASCII(&szNPS[0], uszUnPackBuf, 2);
		if (szNPS[0] == 0x39)
			pobTran->srBRec.uszField24NPSBit = VS_TRUE;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * App Name		: inFUBON_ISO_UnPack37
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 4:35
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_UnPack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	/* 銷售前若有一筆Reversal的RRN交易，則此筆交易Timout的Reversal會帶到前筆RRN的資料 
	  新增 Reversal  _ISR_UPLOAD_ 條件 2022/11/16 [SAM] */
	if (pobTran->inISOTxnCode == _REVERSAL_ || pobTran->inISOTxnCode ==_ISR_UPLOAD_)
	{
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Reversal END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}
	else
	{
		memcpy(&pobTran->srBRec.szRefNo[0], (char *) uszUnPackBuf, 12);
	}
	
	inDISP_LogPrintfWithFlag(" FU UnPack37 szRefNo[%s]", pobTran->srBRec.szRefNo);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * App Name		: inFUBON_ISO_UnPack38
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 4:37
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	if (pobTran->inISOTxnCode == _REVERSAL_ || pobTran->inISOTxnCode == _CUP_LOGON_ ||
		pobTran->inISOTxnCode == _CHANGE_TMK_ || pobTran->inISOTxnCode == _CUP_CHANG_TPK_ ||
		pobTran->inISOTxnCode == _ISR_UPLOAD_)
	{
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Do Not UnPack38 END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}else
	{
		memcpy(&pobTran->srBRec.szAuthCode[0], (char *) uszUnPackBuf, _AUTH_CODE_SIZE_);
	}
	
	inDISP_LogPrintfWithFlag(" FU ISO UnPack38 ACode[%s]", pobTran->srBRec.szAuthCode);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * App Name		: inFUBON_ISO_UnPack39
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 4:40
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	memcpy(&pobTran->srBRec.szRespCode[0], (char *) &uszUnPackBuf[0], 2);

	if (pobTran->inISOTxnCode != _SALE_ &&
		pobTran->inISOTxnCode != _CUP_SALE_ &&
		((!memcmp(&pobTran->srBRec.szRespCode[0], "01", 2)) || (!memcmp(&pobTran->srBRec.szRespCode[0], "02", 2))))
	{
		memset(pobTran->srBRec.szRespCode, 0x00, sizeof (pobTran->srBRec.szRespCode));
		strcpy(pobTran->srBRec.szRespCode, "05");
	}

	pobTran->srBRec.szRespCode[2] = 0x00;
	
	inDISP_LogPrintfWithFlag(" FU UnPack39 RCode[%s]", pobTran->srBRec.szRespCode);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * App Name		: inFUBON_ISO_UnPack55
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 4:42
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_UnPack55(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	int inLen;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	inDISP_LogPrintfWithFlag(" FU ISO UnPack55 inISOTxnCode[%d] uszUnPackBuf[%02x%02x]", pobTran->inISOTxnCode, uszUnPackBuf[0], uszUnPackBuf[1]);

	/* 銷售前若有一筆Reversal的RRN交易，則此筆交易Timout的Reversal會帶到前筆RRN的資料 
	  新增 Reversal  _ISR_UPLOAD_ 條件 2022/11/16 [SAM] */
	if (pobTran->inISOTxnCode == _REVERSAL_ || pobTran->inISOTxnCode ==_ISR_UPLOAD_)
	{
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Reversal END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}

	inLen = ((uszUnPackBuf[0] / 16 * 10) + uszUnPackBuf[0] % 16)*100;
	inLen += (uszUnPackBuf[1] / 16 * 10) + uszUnPackBuf[1] % 16;

	if (inLen > 0)
	{
		inNCCC_EMVUnPackData55(pobTran, &uszUnPackBuf[2], inLen);
	}
	else
	{
		inDISP_DispLogAndWriteFlie(" FU ISO UnPack55 Len[%ld] *Error* Line[%d]", inLen,  __LINE__);
		return (VS_ERROR);
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * App Name		: inFUBON_ISO_UnPack56
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 4:46
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_UnPack56(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	inDISP_LogPrintfWithFlag(" FU ISO UnPack56 inISOTxnCode[%d]", pobTran->inISOTxnCode);
	inDISP_LogPrintfArea(FALSE, "inFUBON_ISO_UnPack56", 20, uszUnPackBuf, 4);

	if (uszUnPackBuf[2] == 0x53 && uszUnPackBuf[3] == 0x31)
	{
		pobTran->srBRec.uszF56NoSignatureBit = VS_TRUE;

	} else
	{
		pobTran->srBRec.uszF56NoSignatureBit = VS_FALSE;

	}

	inDISP_LogPrintfWithFlag(" FU ISO UnPack56 uszUnPackBuf[%s] uszF56NoSignatureBit[%d]", uszUnPackBuf, pobTran->srBRec.uszF56NoSignatureBit);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * App Name		: inFUBON_ISO_UnPack59
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 4:50
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_UnPack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	char szCurrentDateTime[15 + 1];
	char TempBuffer[100];
	char TempKeyData[32 + 1];
	char TempKcv[8 + 1];
	char Kcv[4 + 1];
	char szMKIndex[1 + 1];
	int inLen = 0;
	int OutLen = 0;
	int inResult = VS_ERROR;
	char szPinpadMode[1 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	inDISP_LogPrintfWithFlag(" FU ISO UnPack59 inISOTxnCode[%d]", pobTran->inISOTxnCode);

	memset(szMKIndex, 0x00, sizeof (szMKIndex));
	memset(szCurrentDateTime, 0x00, sizeof (szCurrentDateTime));
	memset(TempBuffer, 0x00, sizeof (TempBuffer));

	memset(szPinpadMode, 0x00, sizeof (szPinpadMode));
	inGetPinpadMode(szPinpadMode);

	inLen = ((uszUnPackBuf[0] / 16 * 10) + uszUnPackBuf[0] % 16) * 100;
	inLen += (uszUnPackBuf[1] / 16 * 10) + uszUnPackBuf[1] % 16;

	inResult = inFUBON_GetISOTableData("A1", &uszUnPackBuf[2], inLen, TempBuffer, &OutLen);

	inDISP_LogPrintfWithFlag(" FU ISO UnPack59 inResult[%d] OutLen[%d] inLen[%d]", inResult, OutLen, inLen);
	inDISP_LogPrintfWithFlag(" FU ISO UnPack59 TempBuffer[%s] szDate[%s]", TempBuffer, pobTran->srBRec.szDate);

	if (inResult == VS_SUCCESS)
	{
		if (OutLen > 0)
		{
			memcpy(&pobTran->srBRec.szDate[0], &TempBuffer[0], 4);
			memcpy(&szCurrentDateTime[0], pobTran->srBRec.szDate, 8);
			memcpy(&szCurrentDateTime[8], pobTran->srBRec.szTime, 6);

			inDISP_DispLogAndWriteFlie(" UnPack59 Time Update HostDate[%s]  Line[%d]", TempBuffer, __LINE__);
			inDISP_DispLogAndWriteFlie(" UnPack59 Time Update CurrentDateTime[%s] Line[%d]", szCurrentDateTime, __LINE__);

			inFunc_SetEDCDateTime(&szCurrentDateTime[0], &szCurrentDateTime[8]);
		}
	}

	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		OutLen = 0;
		memset(TempBuffer, 0x00, sizeof (TempBuffer));
		inResult = inFUBON_GetISOTableData("C1", &uszUnPackBuf[2], inLen, TempBuffer, &OutLen);

		if (inResult == VS_SUCCESS && pobTran->inISOTxnCode != _REVERSAL_ && pobTran->inISOTxnCode != _ISR_UPLOAD_)
		{
			if (OutLen > 0)
			{
				memcpy(pobTran->srBRec.szCUP_TN, &TempBuffer[1], 6);
				memcpy(pobTran->srBRec.szCUP_STD, &TempBuffer[7], 4);
				memcpy(pobTran->srBRec.szCUP_TD, &TempBuffer[11], 4);
				memcpy(pobTran->srBRec.szCUP_TT, &TempBuffer[15], 6);
			}
		}
	}

	if (!memcmp(pobTran->srBRec.szRespCode, "00", 2))
	{
		/* 
		 *  Table B1 交易卡片發卡銀行代碼，同時滿足以下條件則交易Request要送此欄位。
		 * 刷卡機支援支援信用卡作電子發票載具二代載具加密字串規則。
		 * 交易來源為收銀機發動。交易別為sale、redeem及installment。
		 * 2022/10/14 [SAM]
		 * 只有ECR交易，才會有 B1 2022/10/19 [SAM]
		 */
		if(pobTran->uszEcrTransFlag == 'Y')
		{
			if(pobTran->inISOTxnCode == _SALE_ || 
				pobTran->inISOTxnCode == _REDEEM_SALE_ || 
				pobTran->inISOTxnCode == _INST_SALE_ ||
				pobTran->inISOTxnCode == _CUP_SALE_)
			{
				OutLen = 0;
				memset(TempBuffer, 0x00, sizeof (TempBuffer));
				inResult = inFUBON_GetISOTableData("B1", &uszUnPackBuf[2], inLen, TempBuffer, &OutLen);

				if (inResult == VS_SUCCESS)
				{
					if (OutLen == 0x03 && memcmp(TempBuffer,"   ",3))
					{
						memcpy(pobTran->srBRec.szEI_BankId, TempBuffer, 3);
						pobTran->srBRec.szEI_BankId[4] = 0x00;
					}else
					{
						memcpy(pobTran->srBRec.szEI_BankId, "NNN", 3);
						pobTran->srBRec.szEI_BankId[4] = 0x00;
						inDISP_DispLogAndWriteFlie(" UnPack59 B1 Data OutLen[%ld] *Error* Line[%d]", OutLen, __LINE__);
					}
				} else
				{
					inDISP_DispLogAndWriteFlie(" UnPack59 Get B1 Data *Error* Line[%d]", __LINE__);
					return (VS_ERROR);
				}
			}
		}
				
		if (pobTran->srBRec.inCode == _CHANGE_TMK_)
		{
			/* Table ID = P3 */
			/* Terminal Master key data
				--------------------------------------------------------------------
			Change TMK
			Table ID			An		2	2 bytes
			Length			Binary	1	0x18	Now is 24 bytes
			TMK				Binary	16	TMK under KEK (CMK) (3-DES)
			TMK check Value	Binary	8	check only first 3 bytes
			--------------------------------------------------------------------*/
			OutLen = 0;
			memset(TempBuffer, 0x00, sizeof (TempBuffer));
			inResult = inFUBON_GetISOTableData("P3", &uszUnPackBuf[2], inLen, TempBuffer, &OutLen);

			if (inResult == VS_SUCCESS)
			{
				if (OutLen == 0x18)
				{
					memset(TempKeyData, 0x00, sizeof (TempKeyData));

					inResult = inFUBON_TMK_Write(KMS2_KEYPROTECTIONMODE_KPK_ECB, (BYTE *) TempBuffer, 16, (BYTE *) & TempBuffer[16], 3);
					if (inResult != VS_SUCCESS)
					{
						inDISP_DispLogAndWriteFlie(" UnPack59 TMK Write *Error* Line[%d]", __LINE__);
						return (VS_ERROR);
					}
				}else
				{
					inDISP_DispLogAndWriteFlie(" UnPack59 P3 Data OutLen[%ld] *Error* Line[%d]", OutLen, __LINE__);
				}
			} else
			{
				inDISP_DispLogAndWriteFlie(" UnPack59 Get P3 Data *Error* Line[%d]", __LINE__);
				return (VS_ERROR);
			}

			memset(TempKcv, 0x00, sizeof (TempKcv));
			memset(Kcv, 0x00, sizeof (Kcv));
		}

		if (pobTran->srBRec.inCode == _CUP_CHANG_TPK_)
		{
			OutLen = 0;
			memset(TempBuffer, 0x00, sizeof (TempBuffer));
			inResult = inFUBON_GetISOTableData("P2", &uszUnPackBuf[2], inLen, TempBuffer, &OutLen);

			inDISP_LogPrintfArea(TRUE, "TempBuffer = ", 13, (BYTE *) TempBuffer, OutLen);

			if (inResult == VS_SUCCESS)
			{
				if (OutLen == 0x18)
				{
					memset(TempKeyData, 0x00, sizeof (TempKeyData));
					memset(TempKcv, 0x00, sizeof (TempKcv));
					inFunc_BCD_to_ASCII(TempKeyData, (unsigned char *) &TempBuffer[0], 16);
					inFunc_BCD_to_ASCII(TempKcv, (unsigned char *) &TempBuffer[16], 3);
					inResult = inNCCC_TMK_Write_PINKey(1, 16, TempKeyData, TempKcv);
					if (inResult != VS_SUCCESS)
					{
						inDISP_DispLogAndWriteFlie(" UnPack59 TPK Write *Error* Line[%d]", __LINE__);
						return (VS_ERROR);
					}
				} else
				{
					inDISP_DispLogAndWriteFlie(" UnPack59 P2 Data OutLen[%ld] *Error* Line[%d]", OutLen, __LINE__);
					return (VS_ERROR);
				}

				memset(TempKcv, 0x00, sizeof (TempKcv));
				memset(Kcv, 0x00, sizeof (Kcv));
			}else
			{
				inDISP_DispLogAndWriteFlie(" UnPack59 Get P2 Data *Error* Line[%d]", __LINE__);
			}
		}
	}

	inDISP_LogPrintfWithFlag(" FU UnPack59 inISOTxnCode[%d]", pobTran->inISOTxnCode);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * App Name		: inFUBON_ISO_UnPack63
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 5:07
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_UnPack63(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	int inRetVal = VS_ERROR, inCnt = 0, i;
	int inSwitchTxnCode;
	char szTemplate[100];
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	if (pobTran->inISOTxnCode == _REVERSAL_)
	{
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Reversl END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}

	if (!memcmp(&pobTran->srBRec.szRespCode[0], "00", 2))
	{
		if (pobTran->inISOTxnCode == _CUP_LOGON_)
		{
			inRetVal = inFUBON_TMKKeyExchange(pobTran, uszUnPackBuf);
		} else
		{
			if (pobTran->inTransactionCode == _VOID_)
				inSwitchTxnCode = pobTran->srBRec.inOrgCode;
			else
				inSwitchTxnCode = pobTran->inTransactionCode;

			switch (inSwitchTxnCode)
			{
				case _INST_SALE_:
				case _INST_REFUND_:
				case _INST_ADJUST_:
					inCnt += 2; /* Length (2 Byte) */
					/* Indicator (1 Byte) */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], (char *) &uszUnPackBuf[inCnt], 1);
					if (!memcmp(&szTemplate[0], "I", 1) || !memcmp(&szTemplate[0], "E", 1))
					{
						memset(pobTran->srBRec.szInstallmentIndicator, 0x00, sizeof (pobTran->srBRec.szInstallmentIndicator));
						memcpy(&pobTran->srBRec.szInstallmentIndicator[0], (char *) &uszUnPackBuf[inCnt], 1);
						inCnt++;
					} else
					{
						memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
						pobTran->srBRec.szRespCode[2] = 0x00;
						inDISP_DispLogAndWriteFlie(" FU UnPack63 Indicator *Error* Line[%d]", __LINE__);
						return (inRetVal);
					}
					/* (Product Code)*/
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], (char *) &uszUnPackBuf[inCnt], 6);
					pobTran->srBRec.lnFubonGoodsID = atol(szTemplate);
					inCnt += 6;

					/* (_INST_SALE_ Period) */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], (char *) &uszUnPackBuf[inCnt], 2);

					if (memcmp(&szTemplate[0], "  ", 2))
					{
						for (i = 0; i < strlen(szTemplate); i++)
						{
							if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9'))
								continue;
							else
							{
								memcpy(&pobTran->srBRec.szRespCode[0], "I0", 2);
								pobTran->srBRec.szRespCode[2] = 0x00;
								inDISP_DispLogAndWriteFlie(" FU UnPack63 Period Check 0~9 *Error* Line[%d]", __LINE__);
								return (inRetVal);
							}
						}

						pobTran->srBRec.lnInstallmentPeriod = atol(szTemplate);
						inCnt += 2;
					} else
					{
						memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
						pobTran->srBRec.szRespCode[2] = 0x00;
						inDISP_DispLogAndWriteFlie(" FU UnPack63 Period is blank *Error* Line[%d]", __LINE__);
						return (inRetVal);
					}
					/* (Response Code) */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], (char *) &uszUnPackBuf[inCnt], 2);

					memcpy(&pobTran->srBRec.szRespCode[0], szTemplate, 2);
					pobTran->srBRec.szRespCode[2] = 0x00;
					inCnt += 2;

					/* (Down Payment) */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], (char *) &uszUnPackBuf[inCnt], 8);

					if (memcmp(&szTemplate[0], "        ", 8))
					{
						for (i = 0; i < strlen(szTemplate); i++)
						{
							if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9'))
								continue;
							else
							{
								memcpy(&pobTran->srBRec.szRespCode[0], "I0", 2);
								pobTran->srBRec.szRespCode[2] = 0x00;
								inDISP_DispLogAndWriteFlie(" FU UnPack63 Down Payment Check 0~9 *Error* Line[%d]", __LINE__);
								return (inRetVal);
							}
						}

						pobTran->srBRec.lnInstallmentDownPayment = atol(szTemplate);
						inCnt += 8;
					} else
					{
						memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
						pobTran->srBRec.szRespCode[2] = 0x00;
						inDISP_DispLogAndWriteFlie(" FU UnPack63 Down Payment is blank *Error* Line[%d]", __LINE__);
						return (inRetVal);
					}

					/* (_INST_SALE_ Payment) */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], (char *) &uszUnPackBuf[inCnt], 8);

					if (memcmp(&szTemplate[0], "        ", 8))
					{
						for (i = 0; i < strlen(szTemplate); i++)
						{
							if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9'))
								continue;
							else
							{
								memcpy(&pobTran->srBRec.szRespCode[0], "I0", 2);
								pobTran->srBRec.szRespCode[2] = 0x00;
								inDISP_DispLogAndWriteFlie(" FU UnPack63 Payment Check 0~9 *Error* Line[%d]", __LINE__);
								return (inRetVal);
							}
						}

						pobTran->srBRec.lnInstallmentPayment = atol(szTemplate);
						inCnt += 8;
					} else
					{
						memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
						pobTran->srBRec.szRespCode[2] = 0x00;
						inDISP_DispLogAndWriteFlie(" FU UnPack63 Payment is blank *Error* Line[%d]", __LINE__);
						return (inRetVal);
					}

					/* (Formality Fee) */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], (char *) &uszUnPackBuf[inCnt], 8);

					if (memcmp(&szTemplate[0], "        ", 8))
					{
						for (i = 0; i < strlen(szTemplate); i++)
						{
							if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9'))
								continue;
							else
							{
								memcpy(&pobTran->srBRec.szRespCode[0], "I0", 2);
								pobTran->srBRec.szRespCode[2] = 0x00;
								inDISP_DispLogAndWriteFlie(" FU UnPack63 Fee Check 0~9 *Error* Line[%d]", __LINE__);
								return (inRetVal);
							}
						}

						pobTran->srBRec.lnInstallmentFormalityFee = atol(szTemplate);
						inCnt += 8;
					} else
					{
						memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
						pobTran->srBRec.szRespCode[2] = 0x00;
						inDISP_DispLogAndWriteFlie(" FU UnPack63 Fee is blank *Error* Line[%d]", __LINE__);
						return (inRetVal);
					}

					/* Response Code */
					if (szFESDebug[0] == '2')
					{

						inPRINT_ChineseFont(" ", _PRT_NORMAL_);
						inPRINT_ChineseFont("_INST_SALE_ Relative Date", _PRT_NORMAL_);
						/* Indicator */
						memset(szTemplate, 0x00, sizeof (szTemplate));
						sprintf(szTemplate, "  Indicator = %s", pobTran->srBRec.szInstallmentIndicator);
						inPRINT_ChineseFont(szTemplate, _PRT_NORMAL_);
						/* _INST_SALE_ Period */
						memset(szTemplate, 0x00, sizeof (szTemplate));
						sprintf(szTemplate, "  _INST_SALE_ Period = %ld", pobTran->srBRec.lnInstallmentPeriod);
						inPRINT_ChineseFont(szTemplate, _PRT_NORMAL_);
						/* Down Payment */
						memset(szTemplate, 0x00, sizeof (szTemplate));
						sprintf(szTemplate, "  Down Payment = %ld", pobTran->srBRec.lnInstallmentDownPayment);
						inPRINT_ChineseFont(szTemplate, _PRT_NORMAL_);
						/* _INST_SALE_ Payment */
						memset(szTemplate, 0x00, sizeof (szTemplate));
						sprintf(szTemplate, "  _INST_SALE_ Payment = %ld", pobTran->srBRec.lnInstallmentPayment);
						inPRINT_ChineseFont(szTemplate, _PRT_NORMAL_);
						/* Formality Fee */
						memset(szTemplate, 0x00, sizeof (szTemplate));
						sprintf(szTemplate, "  Formality Fee = %ld", pobTran->srBRec.lnInstallmentFormalityFee);
						inPRINT_ChineseFont(szTemplate, _PRT_NORMAL_);
						inPRINT_ChineseFont(" ", _PRT_NORMAL_);
						inPRINT_ChineseFont("==========================================", _PRT_NORMAL_);
					}

					break;
				case _REDEEM_SALE_:
				case _REDEEM_REFUND_:
				case _REDEEM_ADJUST_:
					/* Length (2 Byte) */
					inCnt += 2;

					inDISP_LogPrintfArea(FALSE, "inFUBON_ISO_UnPack63 1", 22, uszUnPackBuf, 84);

					/* Redeem Indicator */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], (char *) &uszUnPackBuf[inCnt], 1);
					if (!memcmp(&szTemplate[0], "1", 1) || !memcmp(&szTemplate[0], "2", 1))
					{
						memset(pobTran->srBRec.szRedeemIndicator, 0x00, sizeof (pobTran->srBRec.szRedeemIndicator));
						memcpy(&pobTran->srBRec.szRedeemIndicator[0], (char *) &uszUnPackBuf[inCnt], 1);
						inCnt++;
					} else
					{
						memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
						pobTran->srBRec.szRespCode[2] = 0x00;
						inDISP_DispLogAndWriteFlie(" FU UnPack63 Redeem Indicator Check 1~2 *Error* Line[%d]", __LINE__);
						return (inRetVal);
					}

					/* (Product Code)*/
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], (char *) &uszUnPackBuf[inCnt], 6);
					pobTran->srBRec.lnFubonGoodsID = atol(szTemplate);
					inCnt += 6;

					/* TX-Response code*/
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], (char *) &uszUnPackBuf[inCnt], 2);
					memcpy(&pobTran->srBRec.szRespCode[0], szTemplate, 2);
					pobTran->srBRec.szRespCode[2] = 0x00;
					inCnt += 2;

					/* (Points Of _REDEEM_SALE_)*/
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], (char *) &uszUnPackBuf[inCnt], 8);

					if (memcmp(&szTemplate[0], "        ", 8))
					{
						for (i = 0; i < strlen(szTemplate); i++)
						{
							if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9'))
								continue;
							else
							{
								memcpy(&pobTran->srBRec.szRespCode[0], "R0", 2);
								pobTran->srBRec.szRespCode[2] = 0x00;
								inDISP_DispLogAndWriteFlie(" FU UnPack63 Redeem Point Check 0~9 *Error* Line[%d]", __LINE__);
								return (inRetVal);
							}
						}

						pobTran->srBRec.lnRedemptionPoints = atol(szTemplate);
						inCnt += 8;
					} else
					{
						memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
						pobTran->srBRec.szRespCode[2] = 0x00;
						inDISP_DispLogAndWriteFlie(" FU UnPack63 Redeem Point is blank *Error* Line[%d]", __LINE__);
						return (inRetVal);
					}

					/* (Points of Balance) */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], (char *) &uszUnPackBuf[inCnt], 8);

					if (memcmp(&szTemplate[0], "        ", 8))
					{
						for (i = 0; i < strlen(szTemplate); i++)
						{
							if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9'))
								continue;
							else
							{
								memcpy(&pobTran->srBRec.szRespCode[0], "R0", 2);
								pobTran->srBRec.szRespCode[2] = 0x00;
								inDISP_DispLogAndWriteFlie(" FU UnPack63 Redeem Balance Check 0~9 *Error* Line[%d]", __LINE__);
								return (inRetVal);
							}
						}

						pobTran->srBRec.lnRedemptionPointsBalance = atol(szTemplate);
						inCnt += 8;
					} else
					{
						memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
						pobTran->srBRec.szRespCode[2] = 0x00;
						inDISP_DispLogAndWriteFlie(" FU UnPack63 Redeem Balance is blank *Error* Line[%d]", __LINE__);
						return (inRetVal);
					}
					/* (Amount of _REDEEM_SALE_)*/
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], (char *) &uszUnPackBuf[inCnt], 8);
					if (memcmp(&szTemplate[0], "        ", 8))
					{
						for (i = 0; i < strlen(szTemplate); i++)
						{
							if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9'))
								continue;
							else
							{
								memcpy(&pobTran->srBRec.szRespCode[0], "R0", 2);
								pobTran->srBRec.szRespCode[2] = 0x00;
								inDISP_DispLogAndWriteFlie(" FU UnPack63 Redeem PaidAmount Check 0~9 *Error* Line[%d]", __LINE__);
								return (inRetVal);
							}
						}

						pobTran->srBRec.lnRedemptionPaidAmount = atol(szTemplate);
						inCnt += 8;
					} else
					{
						memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
						pobTran->srBRec.szRespCode[2] = 0x00;
						inDISP_DispLogAndWriteFlie(" FU UnPack63 Redeem PaidAmount is blank *Error* Line[%d]", __LINE__);
						return (inRetVal);
					}

					/* (Paid Credit Amount) */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], (char *) &uszUnPackBuf[inCnt], 8);

					if (memcmp(&szTemplate[0], "        ", 8))
					{
						for (i = 0; i < strlen(szTemplate); i++)
						{
							if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9'))
								continue;
							else
							{
								memcpy(&pobTran->srBRec.szRespCode[0], "R0", 2);
								pobTran->srBRec.szRespCode[2] = 0x00;
								inDISP_DispLogAndWriteFlie(" FU UnPack63 Redeem PaidCreditAmount Check 0~9 *Error* Line[%d]", __LINE__);
								return (inRetVal);
							}
						}

						pobTran->srBRec.lnRedemptionPaidCreditAmount = atol(szTemplate);
						inCnt += 8;
					} else
					{
						memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
						pobTran->srBRec.szRespCode[2] = 0x00;
						inDISP_DispLogAndWriteFlie(" FU UnPack63 Redeem PaidCreditAmount is blank *Error* Line[%d]", __LINE__);
						return (inRetVal);
					}

					/* Response Code */
					if (szFESDebug[0] == '2')
					{
						inPRINT_ChineseFont(" ", _PRT_NORMAL_);
						inPRINT_ChineseFont("Online _REDEEM_SALE_ Relative Date", _PRT_NORMAL_);
						/* Indicator */
						memset(szTemplate, 0x00, sizeof (szTemplate));
						sprintf(szTemplate, "  Indicator = %s", pobTran->srBRec.szRedeemIndicator);
						inPRINT_ChineseFont(szTemplate, _PRT_NORMAL_);
						/* Points Of _REDEEM_SALE_ */
						memset(szTemplate, 0x00, sizeof (szTemplate));
						sprintf(szTemplate, "  Points Of _REDEEM_SALE_ = %ld", pobTran->srBRec.lnRedemptionPoints);
						inPRINT_ChineseFont(szTemplate, _PRT_NORMAL_);
						/* Points of Balance */
						memset(szTemplate, 0x00, sizeof (szTemplate));
						sprintf(szTemplate, "  Points of Balance = %ld", pobTran->srBRec.lnRedemptionPointsBalance);
						inPRINT_ChineseFont(szTemplate, _PRT_NORMAL_);
						/* Paid Credit Amount */
						memset(szTemplate, 0x00, sizeof (szTemplate));
						sprintf(szTemplate, "  Paid Credit Amount = %ld", pobTran->srBRec.lnRedemptionPaidCreditAmount);
						inPRINT_ChineseFont(szTemplate, _PRT_NORMAL_);
						inPRINT_ChineseFont(" ", _PRT_NORMAL_);
						inPRINT_ChineseFont("==========================================", _PRT_NORMAL_);
					}
					break;
				default:
					break;
			}
		}
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * App Name		: inFUBON_ISO_SendReceData
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 5:13
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_SendReceData(TRANSACTION_OBJECT *pobTran,
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
	
	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, " inFUBON_ISO_SendReceData INIT");

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
		vdFUBON_ISO_ISOFormatDebug_DISP(&uszSendBuf[0], inSendLen);
	}
	/* 列印ISO Debug */
	if (ginISODebug == VS_TRUE)
	{
		vdFUBON_ISO_ISOFormatDebug_PRINT(&uszSendBuf[0], inSendLen);
	}

	if ((inRetVal = inCOMM_Send(uszSendBuf, inSendLen, inSendTimeout, VS_TRUE)) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" FU inCOMM_Send *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "Aft_inCOMM_Send");

	/* 這裡是用CPT.DAT的東西，所以一定要用 */
	memset(szHostResponseTimeOut, 0x00, sizeof (szHostResponseTimeOut));
	inRetVal = inGetHostResponseTimeOut(szHostResponseTimeOut);

	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" FU GetHostResponseTimeOut *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	inReceiveTimeout = atoi(szHostResponseTimeOut);

	if ((inRetVal = inCOMM_Receive(uszReceBuf, inReceiveSize, inReceiveTimeout, VS_TRUE)) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" FU inCOMM_Receive *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inFUBON_ISO_SendReceData Aft  inCOMM_Receive");

	/* 顯示ISO Debug */
	if (ginDebug == VS_TRUE)
		vdFUBON_ISO_ISOFormatDebug_DISP(&uszReceBuf[0], inRetVal);
	/* 列印ISO Debug */
	if (ginISODebug == VS_TRUE)
		vdFUBON_ISO_ISOFormatDebug_PRINT(&uszReceBuf[0], inRetVal);

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inFUBON_ISO_SendReceData END");
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);

}

/*
 * App Name		: inFUBON_ISO_GetFieldLen
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 5:18
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_FUBON_TABLE *srFieldType)
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
			case _FUBON_ISO_ASC_:
				inCnt += srFieldType[i].inFieldLen;
				break;
			case _FUBON_ISO_BCD_:
				inCnt += srFieldType[i].inFieldLen / 2;
				break;
			case _FUBON_ISO_NIBBLE_2_:
				inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
				inCnt += ((inLen + 1) / 2) + 1;
				break;
			case _FUBON_ISO_NIBBLE_3_:
				inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
				inCnt += ((inLen + 1) / 2) + 2;
				break;
			case _FUBON_ISO_BYTE_2_:
				inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
				inCnt += inLen + 1;
				break;
			case _FUBON_ISO_BYTE_3_:
				inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
				inCnt += inLen + 2;
				break;
			case _FUBON_ISO_BYTE_2_H_:
				inLen = (int) uszSendData[0];
				inCnt += inLen + 1;
				break;
			case _FUBON_ISO_BYTE_3_H_:
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
 * App Name		: inFUBON_ISO_GetCheckField
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 5:38
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 * 讀取對應的 ISO_CHECK_FUBON_TABLE ，並執行設定需要檢查的函式 
 * 因執行的函式有DEBUG LOG, 這邊可以不顯示  LOG 2022/10/21 
 */
int inFUBON_ISO_GetCheckField(int inField, ISO_CHECK_FUBON_TABLE *srChkISO)
{
	int i;
	for (i = 0; i < 64; i++)
	{
		if (srChkISO[i].inFieldNum == 0)
			break;
		else if (srChkISO[i].inFieldNum == inField)
		{
			return (i);
		}
	}
	return (VS_ERROR);
}

/*
 * App Name		: inFUBON_ISO_CheckUnPackField
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 5:39
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_CheckUnPackField(int inField, ISO_FIELD_FUBON_TABLE *srCheckUnPackField)
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
			inDISP_LogPrintfWithFlag(" FU ISO CheckUnPackFiled ReturnVal[%d] ", i);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
			return (VS_SUCCESS);
		}
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] *Error* END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_ERROR);
}

/*
 * App Name		: inFUBON_CheckIncSTAN
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 5:40
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_CheckIncSTAN(void)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);	
#if 0//By Ray
	long lnSTANNum;
	TRANSACTION_OBJECT *pobTran;

	//pdebug(("--inIncSTAN--"));

	if ((pobTran = pstGet_pobTran()) == _NULL_CH_)
		return VS_ERROR;

	lnSTANNum = lnGetHostSTAN(); //lnGetMHTSTAN kelvin_f2 upgrade v2.2

	lnSTANNum++;
	if (lnSTANNum % 100 == 0)
		lnSTANNum = 1;

	if (lnSTANNum > lnMaxValue(inGetSTANSize())) //v2.2
		lnSTANNum = 1L;

	vdSetHostSTAN(lnSTANNum); //vdSetMHTSTAN(lnSTANNum); version 1.45

	return (inSaveHDTRec(inGetBatchIndex())); //inGetHDTBatchIndex()version 1.45
#else
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return 0;
#endif
}

/*
 * App Name		: inFUBON_ISO_GetFieldIndex
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 5:40
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_GetFieldIndex(int inField, ISO_FIELD_TYPE_FUBON_TABLE *srFieldType)
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
 * Function		: vdProcessDebugOutPut
 * Date&Time	: 2022/10/20 下午 4:56 
 * Describe		: 
 * 統一處理 Iso Debug 輸出訊息方式
 * 2022/10/20 [SAM]
 */
void vdProcessDebugOutPut(char *szSouce, int inDebugType)
{
	if(inDebugType == _PRINT_DEBUG_ISO)
	{
		inPRINT_ChineseFont(szSouce, _PRT_NORMAL_);
	}else{
		inDISP_LogPrintfWithFlag(szSouce);
	}
}

/*
 * App Name		: vdFUBON_ISO_ISOFormatDebug_DISP
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 5:41
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
void vdFUBON_ISO_ISOFormatDebug_DISP(unsigned char *bSendBuf, int inSendLen)
{
	char szPrtBuf[50], szBuf[500], szBitMap[_FUBON_BIT_MAP_SIZE_ + 1];
	int i, j, inCnt = 0, inField, inLen, inFieldLen;
	int inBitMapCnt1, inBitMapCnt2;
	unsigned char uszBitMap[9], uszBitMapDisp[66];
	ISO_TYPE_FUBON_TABLE srISOTypeTableDisp;
	char szEncryptMode[1 + 1] = {0};

	inGetEncryptMode(szEncryptMode);
	memset((char *) &srISOTypeTableDisp, 0x00, sizeof (srISOTypeTableDisp));
	memcpy((char *) &srISOTypeTableDisp, (char *) &srFUBON_ISOFunc[szEncryptMode[0] - '0'], sizeof (srISOTypeTableDisp));

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("==========================================");

	if (bSendBuf[_FUBON_TPDU_SIZE_ + 1] & 0x10)
	{
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("ISO8583 Format  <<Recvive Data>>");
	} else
	{
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("ISO8583 Format  <<Send Data>>");
	}

	memset(szBuf, 0x00, sizeof (szBuf));
	sprintf(szBuf, "TPDU = [");
	inFunc_BCD_to_ASCII(&szBuf[8], &bSendBuf[inCnt], _FUBON_TPDU_SIZE_);
	strcat(szBuf, "]");
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("%s", szBuf);
	inCnt += _FUBON_TPDU_SIZE_;

	memset(szBuf, 0x00, sizeof (szBuf));
	sprintf(szBuf, "MTI  = [");
	inFunc_BCD_to_ASCII(&szBuf[8], &bSendBuf[inCnt], _FUBON_MTI_SIZE_);
	strcat(szBuf, "]");
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("%s", szBuf);
	inCnt += _FUBON_MTI_SIZE_;

	memset(szBuf, 0x00, sizeof (szBuf));
	memset(szBitMap, 0x00, sizeof (szBitMap));
	memcpy(szBitMap, (char *) &bSendBuf[inCnt], _FUBON_BIT_MAP_SIZE_);
	sprintf(szBuf, "BMap = [%02X %02X %02X %02X %02X %02X %02X %02X]",
			bSendBuf[inCnt + 0],
			bSendBuf[inCnt + 1],
			bSendBuf[inCnt + 2],
			bSendBuf[inCnt + 3],
			bSendBuf[inCnt + 4],
			bSendBuf[inCnt + 5],
			bSendBuf[inCnt + 6],
			bSendBuf[inCnt + 7]);
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("%s", szBuf);
	memset(uszBitMapDisp, 0x00, sizeof (uszBitMapDisp));
	memset(uszBitMap, 0x00, sizeof (uszBitMap));
	memcpy(uszBitMap, (char *) &bSendBuf[inCnt], _FUBON_BIT_MAP_SIZE_);

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


	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("   123456789A    123456789A    123456789A");
	memset(szBuf, 0x00, sizeof (szBuf));
	sprintf(szBuf, " 0>%10.10s  1>%10.10s  2>%10.10s", &uszBitMapDisp[1], &uszBitMapDisp[11], &uszBitMapDisp[21]);
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("%s", szBuf);
	sprintf(szBuf, " 3>%10.10s  4>%10.10s  5>%10.10s", &uszBitMapDisp[31], &uszBitMapDisp[41], &uszBitMapDisp[51]);
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("%s", szBuf);
	sprintf(szBuf, " 6>%4.4s", &uszBitMapDisp[61]);
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("%s", szBuf);

	inCnt += _FUBON_BIT_MAP_SIZE_;

	for (i = 1; i <= 64; i++)
	{
		if (!inFUBON_BitMapCheck((unsigned char *) szBitMap, i))
			continue;

		inLen = 0;
		inFieldLen = 0;
		memset(szBuf, 0x00, sizeof (szBuf));
		sprintf(szBuf, "F_%02d ", i);
		inLen = 5;
		inField = inFUBON_ISO_GetFieldIndex(i, srISOTypeTableDisp.srISOFieldType);
		inFieldLen = inFUBON_ISO_GetFieldLen(i, &bSendBuf[inCnt], srISOTypeTableDisp.srISOFieldType);
		switch (srISOTypeTableDisp.srISOFieldType[inField].inFieldType)
		{
			case _FUBON_ISO_ASC_:
				memcpy(&szBuf[inLen], &bSendBuf[inCnt], inFieldLen);
				break;
			case _FUBON_ISO_NIBBLE_2_:
			case _FUBON_ISO_BYTE_2_:
			case _FUBON_ISO_BYTE_3_:
			case _FUBON_ISO_BYTE_2_H_:
			case _FUBON_ISO_BYTE_3_H_:
				strcat(&szBuf[inLen++], "[");
				inFunc_BCD_to_ASCII(&szBuf[inLen], &bSendBuf[inCnt++], 1);
				inLen += 2;
				inFieldLen--;
				if (srISOTypeTableDisp.srISOFieldType[inField].inFieldType == _FUBON_ISO_BYTE_3_ ||
						srISOTypeTableDisp.srISOFieldType[inField].inFieldType == _FUBON_ISO_BYTE_3_H_)
				{
					inFunc_BCD_to_ASCII(&szBuf[inLen], &bSendBuf[inCnt++], 1);
					inLen += 2;
					inFieldLen--;
				}
				strcat(&szBuf[inLen++], "]");
			case _FUBON_ISO_BCD_:
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
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("%s", szPrtBuf);
			j++;
		} while ((j * 42) < strlen(szBuf));

		if (i == 55)
			vdFUBON_ISO_ISOFormatDebug_EMV(&bSendBuf[inCnt], inFieldLen, _DISPLAY_DEBUG_ISO);
		else if (i == 59)
			vdFUBON_ISO_ISOFormatDebug_59(&bSendBuf[inCnt], inFieldLen, _DISPLAY_DEBUG_ISO);		
		
		inCnt += inFieldLen;
	}

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("==========================================");
}


/*
 * App Name		: vdFUBON_ISO_ISOFormatDebug_PRINT
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 5:43
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
void vdFUBON_ISO_ISOFormatDebug_PRINT(unsigned char *bSendBuf, int inSendLen)
{
	char szBuf[512], szBitMap[_FUBON_BIT_MAP_SIZE_ + 1];
	char szPIN[50];
	int i, inCnt = 0, inField, inLen, inFieldLen, inOneLineLen = 48, inPrintLineCnt;
	int inBitMapCnt1, inBitMapCnt2;
	unsigned char uszBitMap[9], uszBitMapDisp[66];
	ISO_TYPE_FUBON_TABLE srISOTypeTablePrt;
	char szEncryptMode[1 + 1] = {0};
	char szPrintLineData[50 + 1], szPrtBuf[70 + 1];
	//	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];

	inGetEncryptMode(szEncryptMode);
	memset(szPIN, 0x2a, sizeof (szPIN));
	memset((char *) &srISOTypeTablePrt, 0x00, sizeof (srISOTypeTablePrt));
	memcpy((char *) &srISOTypeTablePrt, (char *) &srFUBON_ISOFunc[szEncryptMode[0] - '0'], sizeof (srISOTypeTablePrt));

	if (bSendBuf[6] == 0x00 || bSendBuf[6] == 0x20 || bSendBuf[6] == 0x40)
	{
		inPRINT_ChineseFont(" ", _PRT_NORMAL_);
		inPRINT_ChineseFont("==========================================", _PRT_NORMAL_);
		inPRINT_ChineseFont("ISO8583 Format  <<Send Data>>", _PRT_NORMAL_);
	} else
		inPRINT_ChineseFont("ISO8583 Format  <<Recvive Data>>", _PRT_NORMAL_);

	memset(szBuf, 0x00, sizeof (szBuf));
	sprintf(szBuf, "TPDU = [");
	inFunc_BCD_to_ASCII(&szBuf[8], &bSendBuf[inCnt], _FUBON_TPDU_SIZE_);
	strcat(szBuf, "]");
	inPRINT_ChineseFont(szBuf, _PRT_NORMAL_);
	inCnt += _FUBON_TPDU_SIZE_;

	memset(szBuf, 0x00, sizeof (szBuf));
	sprintf(szBuf, "MTI  = [");
	inFunc_BCD_to_ASCII(&szBuf[8], &bSendBuf[inCnt], _FUBON_MTI_SIZE_);
	strcat(szBuf, "]");
	inPRINT_ChineseFont(szBuf, _PRT_NORMAL_);
	inCnt += _FUBON_MTI_SIZE_;

	memset(szBuf, 0x00, sizeof (szBuf));
	memset(szBitMap, 0x00, sizeof (szBitMap));
	memcpy(szBitMap, (char *) &bSendBuf[inCnt], _FUBON_BIT_MAP_SIZE_);
	sprintf(szBuf, "BMap = [%02X %02X %02X %02X %02X %02X %02X %02X]",
			bSendBuf[inCnt + 0],
			bSendBuf[inCnt + 1],
			bSendBuf[inCnt + 2],
			bSendBuf[inCnt + 3],
			bSendBuf[inCnt + 4],
			bSendBuf[inCnt + 5],
			bSendBuf[inCnt + 6],
			bSendBuf[inCnt + 7]);

	inPRINT_ChineseFont(szBuf, _PRT_NORMAL_);
	memset(uszBitMapDisp, 0x00, sizeof (uszBitMapDisp));
	memset(uszBitMap, 0x00, sizeof (uszBitMap));
	memcpy(uszBitMap, (char *) &bSendBuf[inCnt], _FUBON_BIT_MAP_SIZE_);

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

	inPRINT_ChineseFont("   1234567890    1234567890    1234567890", _PRT_NORMAL_);
	memset(szBuf, 0x00, sizeof (szBuf));
	sprintf(szBuf, " 0>%10.10s  1>%10.10s  2>%10.10s", &uszBitMapDisp[1], &uszBitMapDisp[11], &uszBitMapDisp[21]);
	inPRINT_ChineseFont(szBuf, _PRT_NORMAL_);
	sprintf(szBuf, " 3>%10.10s  4>%10.10s  5>%10.10s", &uszBitMapDisp[31], &uszBitMapDisp[41], &uszBitMapDisp[51]);
	inPRINT_ChineseFont(szBuf, _PRT_NORMAL_);
	sprintf(szBuf, " 6>%4.4s", &uszBitMapDisp[61]);
	inPRINT_ChineseFont(szBuf, _PRT_NORMAL_);

	inCnt += _FUBON_BIT_MAP_SIZE_;

	for (i = 1; i <= 64; i++)
	{
		if (!inFUBON_BitMapCheck((unsigned char *) szBitMap, i))
			continue;

		inLen = 0;
		inFieldLen = 0;
		memset(szBuf, 0x00, sizeof (szBuf));
		sprintf(szBuf, "F_%02d ", i);
		inLen = 5;
		strcat(&szBuf[inLen++], "[");

		switch (i)
		{
			case 2:
				strcat(szBuf, "PAN ");
				inLen += 4;
				break;
			case 3:
				strcat(szBuf, "P_CODE ");
				inLen += 7;
				break;
			case 4:
				strcat(szBuf, "AMT ");
				inLen += 4;
				break;
			case 11:
				strcat(szBuf, "STAN ");
				inLen += 5;
				break;
			case 12:
				strcat(szBuf, "TIME ");
				inLen += 5;
				break;
			case 13:
				strcat(szBuf, "DATE ");
				inLen += 5;
				break;
			case 24:
				strcat(szBuf, "NII ");
				inLen += 4;
				break;
			case 37:
				strcat(szBuf, "RRN ");
				inLen += 4;
				break;
			case 38:
				strcat(szBuf, "AUTH_ID ");
				inLen += 8;
				break;
			case 39:
				strcat(szBuf, "RESP_CODE ");
				inLen += 10;
				break;
			case 41:
				strcat(szBuf, "TID ");
				inLen += 4;
				break;
			case 42:
				strcat(szBuf, "MID ");
				inLen += 4;
				break;
			case 48:
				strcat(szBuf, "STORE_ID ");
				inLen += 9;
				break;
			case 52:
				strcat(szBuf, "PIN_BLOCK ");
				inLen += 10;
				break;
			case 54:
				strcat(szBuf, "TIP_AMT ");
				inLen += 8;
				break;
			case 55:
				strcat(szBuf, "EMV ");
				inLen += 4;
				break;
			case 56:
				strcat(szBuf, "EMV ");
				inLen += 4;
				break;
			case 57:
				strcat(szBuf, "CSUM ");
				inLen += 5;
				break;
			case 59:
				strcat(szBuf, "TABLE ");
				inLen += 6;
				break;
			case 64:
				strcat(szBuf, "MAC ");
				inLen += 4;
				break;
			default:
				break;
		}

		inField = inFUBON_ISO_GetFieldIndex(i, srISOTypeTablePrt.srISOFieldType);
		inFieldLen = inFUBON_ISO_GetFieldLen(i, &bSendBuf[inCnt], srISOTypeTablePrt.srISOFieldType);

		switch (srISOTypeTablePrt.srISOFieldType[inField].inFieldType)
		{
			case _FUBON_ISO_ASC_:
				memcpy(&szBuf[inLen], &bSendBuf[inCnt], inFieldLen);
				break;
			case _FUBON_ISO_BYTE_3_:
				inFunc_BCD_to_ASCII(&szBuf[inLen], &bSendBuf[inCnt], 2);
				inCnt += 2;
				inLen += 4;
				strcat(szBuf, " ");
				inLen++;
				inFieldLen--;
				inFieldLen--;

				if (srISOTypeTablePrt.srISOFieldType[inField].uszDispAscii == VS_TRUE)
				{
					if (i != 59)
						memcpy(&szBuf[inLen], (char *) &bSendBuf[inCnt], inFieldLen);
				} else
				{
					if (i != 55)
						inFunc_BCD_to_ASCII(&szBuf[inLen], &bSendBuf[inCnt], inFieldLen);
				}
				break;
			case _FUBON_ISO_NIBBLE_2_:
			case _FUBON_ISO_BYTE_2_:
			case _FUBON_ISO_BYTE_2_H_:
			case _FUBON_ISO_BYTE_3_H_:

				inFunc_BCD_to_ASCII(&szBuf[inLen], &bSendBuf[inCnt++], 1);
				inLen += 2;
				strcat(szBuf, " ");
				inLen++;

				inFieldLen--;
				if (srISOTypeTablePrt.srISOFieldType[inField].inFieldType == _FUBON_ISO_BYTE_3_ ||
						srISOTypeTablePrt.srISOFieldType[inField].inFieldType == _FUBON_ISO_BYTE_3_H_)
				{
					inFunc_BCD_to_ASCII(&szBuf[inLen], &bSendBuf[inCnt++], 1);
					inLen += 2;
					inFieldLen--;
				}

			case _FUBON_ISO_BCD_:

				if (srISOTypeTablePrt.srISOFieldType[inField].uszDispAscii == VS_TRUE)
				{

					//if (i == 52 )
					//    memcpy(&szBuf[inLen], szPIN, inFieldLen);
					//else
					memcpy(&szBuf[inLen], (char *) &bSendBuf[inCnt], inFieldLen);
				} else
				{
					//if (i == 52 )
					//    memcpy(&szBuf[inLen], szPIN, inFieldLen);
					//else
					inFunc_BCD_to_ASCII(&szBuf[inLen], &bSendBuf[inCnt], inFieldLen);
				}
				break;
			default:
				break;
		}

		strcat(&szBuf[inLen++], "]");

		inPrintLineCnt = 0;
//		if (ginDebug == VS_TRUE)
//			inDISP_LogPrintf("vdFUBON_ISO_ISOFormatDebug_PRINT 0,%d,%d,%d!!", inPrintLineCnt, inOneLineLen, strlen(szBuf));
		while ((inPrintLineCnt * inOneLineLen) < strlen(szBuf))
		{
			memset(szPrintLineData, 0x00, sizeof (szPrintLineData));
			memset(szPrtBuf, 0x00, sizeof (szPrtBuf));
			if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szBuf))
			{
				strcat(szPrintLineData, &szBuf[inPrintLineCnt * inOneLineLen]);
			} else
			{
				memcpy(szPrintLineData, &szBuf[inPrintLineCnt * inOneLineLen], inOneLineLen);
			}

			sprintf(szPrtBuf, "  %s", szPrintLineData);

			inPRINT_ChineseFont(szPrtBuf, _PRT_NORMAL_);
			//inPRINT_Buffer_PutIn(szPrtBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPrintLineCnt++;
//			if (ginDebug == VS_TRUE)
//				inDISP_LogPrintf("vdFUBON_ISO_ISOFormatDebug_PRINT 1,%d,%d,%d!!", inPrintLineCnt, inOneLineLen, strlen(szBuf));
		};

		if (i == 55)
			vdFUBON_ISO_ISOFormatDebug_EMV(&bSendBuf[inCnt], inFieldLen, _PRINT_DEBUG_ISO);
		else if (i == 59)
			vdFUBON_ISO_ISOFormatDebug_59(&bSendBuf[inCnt], inFieldLen, _PRINT_DEBUG_ISO);
		inCnt += inFieldLen;
	}

	inPRINT_ChineseFont("==========================================", _PRT_NORMAL_);
	inPRINT_ChineseFont(" ", _PRT_NORMAL_);
}

/*
 * App Name		: vdFUBON_ISO_ISOFormatDebug_EMV
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 5:43
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
void vdFUBON_ISO_ISOFormatDebug_EMV(unsigned char *bSendBuf, int inFieldLen, int inDebugType)
{
	int i, inTagLen;
	char szTag[128], szPrintTag[128];

	for (i = 0; i < inFieldLen; i++)
	{
		if ((bSendBuf[i] & 0x1F) == 0x1F)
		{
			/* 2-Byte Tag */
			memset(szTag, 0x00, sizeof (szTag));
			memset(szPrintTag, 0x00, sizeof (szPrintTag));
			inTagLen = bSendBuf[i + 2];
			inFunc_BCD_to_ASCII(&szTag[0], &bSendBuf[i + 3], inTagLen);
			sprintf(&szPrintTag[0], "  Tag %02X", bSendBuf[i]);
			i++;
			sprintf(&szPrintTag[8], "%02X", bSendBuf[i]);
			i++;
			sprintf(&szPrintTag[10], " (%02X = %d) %s", bSendBuf[i], inTagLen, szTag);
			i += inTagLen;
			vdProcessDebugOutPut(szPrintTag, inDebugType);
		} else
		{
			/* 1-Byte Tag */
			memset(szTag, 0x00, sizeof (szTag));
			memset(szPrintTag, 0x00, sizeof (szPrintTag));
			inTagLen = bSendBuf[i + 1];
			inFunc_BCD_to_ASCII(&szTag[0], &bSendBuf[i + 2], inTagLen);
			sprintf(&szPrintTag[0], "  Tag %02X", bSendBuf[i]);
			i++;
			sprintf(&szPrintTag[8], " (%02X = %d)(%s)", bSendBuf[i], inTagLen, szTag);
			i += inTagLen;
			
			vdProcessDebugOutPut(szPrintTag, inDebugType);
		}
	}
}

/*
 * App Name		: vdFUBON_ISO_ISOFormatDebug_59
 * App Builder		: 
 * App Date&Time	: 
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
void vdFUBON_ISO_ISOFormatDebug_59(unsigned char *bSendBuf, int inFieldLen, int inDebugType)
{
	int inCnt = 0, inTableLen;
	char szPrintBuf[128], szTemplate[42 + 1];
	int i;
	char Debugbuf[1024];
		
	while (inFieldLen > inCnt)
	{
		switch (bSendBuf[inCnt])
		{
			case 'N':
				/* Table ID */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %.2s", &bSendBuf[inCnt]);
				vdProcessDebugOutPut(szPrintBuf, inDebugType);
				inCnt += 2;
				
				/* Table Length */
				inTableLen = bSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %x / %d", bSendBuf[inCnt], inTableLen);
				vdProcessDebugOutPut(szPrintBuf, inDebugType);
				inCnt++;
				
				/* Ori Pre-Auth Trans Date  (8 Byte) */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(&szTemplate[0], &bSendBuf[inCnt], 8);
				memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
				sprintf(szPrintBuf, "   Ori Pre-Auth Trans Date = %s", szTemplate);
				vdProcessDebugOutPut(szPrintBuf, inDebugType);
				inCnt += 8;
				
				/* Ori Pre-Auth Trans Date  (8 Byte) */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(&szTemplate[0], &bSendBuf[inCnt], 12);
				memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
				sprintf(szPrintBuf, "   Ori Pre-Auth Amount = %s", szTemplate);
				vdProcessDebugOutPut(szPrintBuf, inDebugType);
				inCnt += 12;
				
				/* Ori Pre-Auth Trans Approval code  (6 Byte) */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(&szTemplate[0], &bSendBuf[inCnt], 6);
				memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
				sprintf(szPrintBuf, "   Ori Pre-Auth Trans Approval code = %s", szTemplate);
				vdProcessDebugOutPut(szPrintBuf, inDebugType);
				inCnt += 6;
				
				break;
			case 'B':
				/* Table ID */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %.2s", &bSendBuf[inCnt]);
				vdProcessDebugOutPut(szPrintBuf, inDebugType);
				inCnt += 2;
				
				/* Table Length */
				inTableLen = bSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %x / %d", bSendBuf[inCnt], inTableLen);
				vdProcessDebugOutPut(szPrintBuf, inDebugType);
				inCnt++;
				
				/* BANK ID  (3 Byte) */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(&szTemplate[0], &bSendBuf[inCnt], 3);
				memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
				sprintf(szPrintBuf, "  BANK ID = %s", szTemplate);
				vdProcessDebugOutPut(szPrintBuf, inDebugType);
				inCnt += 3;
				break;
			case 'P':
				/* Table ID */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %.2s", &bSendBuf[inCnt]);
				vdProcessDebugOutPut(szPrintBuf, inDebugType);
				inCnt += 2;
				/* Table Length */
				inTableLen = bSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %x / %d", bSendBuf[inCnt], inTableLen);
				vdProcessDebugOutPut(szPrintBuf, inDebugType);
				inCnt++;

				if (bSendBuf[inCnt - 2] == '1') /* 3->2 */
				{
					vdProcessDebugOutPut("  Pinpad SN", inDebugType);
				} else if (bSendBuf[inCnt - 2] == '4') /* 3->2 */
				{
					vdProcessDebugOutPut("  CMK", inDebugType);
				} else
				{
					vdProcessDebugOutPut("  Table Data", inDebugType);
				}

				for (i = 0; i < inTableLen; i++)
					sprintf(&Debugbuf[i * 2], "%02X", bSendBuf[inCnt + i]);
				vdProcessDebugOutPut(Debugbuf, inDebugType);
				inCnt += inTableLen;

				break;
			case 'C':
				/* Table ID */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %.2s", &bSendBuf[inCnt]);
				vdProcessDebugOutPut(szPrintBuf, inDebugType);
				inCnt += 2;
				/* Table Length */
				inTableLen = bSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %x / %d", bSendBuf[inCnt], inTableLen);
				vdProcessDebugOutPut(szPrintBuf, inDebugType);
				inCnt++;
				/* Acquire Indicator / Terminal Hot Key Control (1 Byte) */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				szTemplate[0] = bSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
				sprintf(szPrintBuf, "    CUP Indicator = %s", szTemplate);
				vdProcessDebugOutPut(szPrintBuf, inDebugType);
				inCnt++;

				if (szTemplate[0] == '1')
				{
					/* CUP Trace Number (6 Byte) */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], &bSendBuf[inCnt], 6);
					memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
					sprintf(szPrintBuf, "    CUP Trace Number = %s", szTemplate);
					vdProcessDebugOutPut(szPrintBuf, inDebugType);
					inCnt += 6;
					/* Settlement Date(MMDD) (4 Byte) */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], &bSendBuf[inCnt], 4);
					memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
					sprintf(szPrintBuf, "    Settlement Date(MMDD) = %s", szTemplate);
					vdProcessDebugOutPut(szPrintBuf, inDebugType);
					inCnt += 4;
					/* CUP Transaction Date(MMDD) (4 Byte) */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], &bSendBuf[inCnt], 4);
					memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
					sprintf(szPrintBuf, "    CUP Transaction Date(MMDD) = %s", szTemplate);
					vdProcessDebugOutPut(szPrintBuf, inDebugType);
					inCnt += 4;
					/* CUP Transaction Time(hhmmss) (6 Byte) */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], &bSendBuf[inCnt], 6);
					memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
					sprintf(szPrintBuf, "    CUP Transaction Time(hhmmss) = %s", szTemplate);
					vdProcessDebugOutPut(szPrintBuf, inDebugType);
					inCnt += 6;
				}

				break;
			case 'A':
				/* Table ID */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %.2s", &bSendBuf[inCnt]);
				vdProcessDebugOutPut(szPrintBuf, inDebugType);
				inCnt += 2;
				/* Table Length */
				inTableLen = bSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %x / %d", bSendBuf[inCnt], inTableLen);
				vdProcessDebugOutPut(szPrintBuf, inDebugType);
				inCnt++;

				if (bSendBuf[inCnt - 2] == '1')
				{
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], &bSendBuf[inCnt], 4);
					memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
					sprintf(szPrintBuf, "   Host YYYY = %s", szTemplate);
					vdProcessDebugOutPut(szPrintBuf, inDebugType);
					inCnt += 4;

					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], &bSendBuf[inCnt], 10);
					memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
					sprintf(szPrintBuf, "   Host MMDDHHMMSS = %s", szTemplate);
					vdProcessDebugOutPut(szPrintBuf, inDebugType);
					inCnt += 10;
				} else if (bSendBuf[inCnt - 2] == '2')
				{
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], &bSendBuf[inCnt], 6);
					memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
					sprintf(szPrintBuf, "   Origonal STAN = %s", szTemplate);
					vdProcessDebugOutPut(szPrintBuf, inDebugType);
					inCnt += 6;
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], &bSendBuf[inCnt], 14);
					memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
					sprintf(szPrintBuf, "   Origonal Date = %s", szTemplate);
					vdProcessDebugOutPut(szPrintBuf, inDebugType);
					inCnt += 14;
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(&szTemplate[0], &bSendBuf[inCnt], 12);
					memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
					sprintf(szPrintBuf, "   Origonal RRN = %s", szTemplate);
					vdProcessDebugOutPut(szPrintBuf, inDebugType);
					inCnt += 12;

				}
				break;
			default:
				memset(szTemplate, 0x00, sizeof (szTemplate));
				szTemplate[0] = bSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
				sprintf(szPrintBuf, "     OTHER = (%s)", szTemplate);
				vdProcessDebugOutPut(szPrintBuf, inDebugType);
				inCnt++;
				break;
		}
	}
}

/*
 * App Name		: inFUBON_ISO_BuildAndSendPacket
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 5:44
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_BuildAndSendPacket(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;
	char szDemoMode[2 + 1] = {0};
	char szTemplate[100 + 1] = {0}, szTemplate1[100 + 1] = {0}, szTemplate2[42 + 1] = {0};

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inFUBON_ISO_BuildAndSendPacket INIT");

	inDISP_LogPrintfWithFlag(" uszOfflineBit[%d] ", pobTran->srBRec.uszOfflineBit);

	pobTran->srBRec.uszNoSignatureBit = VS_FALSE;

	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof (szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		/* 步驟 1 Get System Trans Number */
		inFUBON_GetSTAN(pobTran);

		if (pobTran->inTransactionCode == _INST_SALE_)
		{
			if (pobTran->srBRec.lnInstallmentPeriod == 0)
			{
				/* 分期付款_頭期款 */
				pobTran->srBRec.lnInstallmentDownPayment = pobTran->srBRec.lnTxnAmount;
				pobTran->srBRec.lnInstallmentPayment = 0L; /* 分期付款_每期款 */
			} else
			{
				/* 分期付款_頭期款 */
				pobTran->srBRec.lnInstallmentDownPayment = (pobTran->srBRec.lnTxnAmount / pobTran->srBRec.lnInstallmentPeriod);
				/* 分期付款_每期款 */
				pobTran->srBRec.lnInstallmentPayment = pobTran->srBRec.lnInstallmentDownPayment;
				/* 餘數加到首期 */
				pobTran->srBRec.lnInstallmentDownPayment += (pobTran->srBRec.lnTxnAmount % pobTran->srBRec.lnInstallmentPeriod);
			}

			pobTran->srBRec.lnInstallmentFormalityFee = 0L; /* 聯合分期付款_手續費 */

		} else if (pobTran->inTransactionCode == _REDEEM_SALE_)
		{
			/* 紅利扣抵_扣抵紅利點數 */
			pobTran->srBRec.lnRedemptionPoints = (pobTran->srBRec.lnTxnAmount / 10);
			/* 紅利扣抵_支付金額 */
			pobTran->srBRec.lnRedemptionPaidCreditAmount = (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPoints);
			/* 紅利扣抵_剩餘紅利點數 */
			pobTran->srBRec.lnRedemptionPointsBalance = (pobTran->srBRec.lnTxnAmount * 999);
			if (pobTran->srBRec.lnRedemptionPointsBalance > 9999999)
				pobTran->srBRec.lnRedemptionPointsBalance = 9999999;
		}

		/* 授權碼 = Batch Number + Invoice Number */
		if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
		{
			if (strlen(pobTran->srBRec.szAuthCode) > 0)
			{

			} else
			{
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memset(szTemplate1, 0x00, sizeof (szTemplate1));
				memset(szTemplate2, 0x00, sizeof (szTemplate2));

				inGetBatchNum(szTemplate1);
				inGetInvoiceNum(szTemplate2);

				sprintf(szTemplate, "%03d%03d", atoi(szTemplate1), atoi(szTemplate2));
				strcpy(pobTran->srBRec.szAuthCode, szTemplate);
			}

		}

		/* 主機回應碼 */
		strcpy(pobTran->srBRec.szRespCode, "00");

		memset(pobTran->srBRec.szRefNo, 0x00, sizeof (pobTran->srBRec.szRefNo));
		strcpy(pobTran->srBRec.szRefNo, "9");

		memset(szTemplate, 0x00, 12);
		inGetTerminalID(szTemplate);
		memcpy(&pobTran->srBRec.szRefNo[1], &szTemplate[3], 5);

		memset(szTemplate, 0x00, sizeof (szTemplate));
		inGetBatchNum(szTemplate);
		sprintf(szTemplate, "%ld", pobTran->srBRec.lnBatchNum);
		memcpy(&pobTran->srBRec.szRefNo[6], &szTemplate[4], 2);

		memset(szTemplate, 0x00, sizeof (szTemplate));
		inGetInvoiceNum(szTemplate);
		sprintf(szTemplate, "%ld", pobTran->srBRec.lnOrgInvNum);
		memcpy(&pobTran->srBRec.szRefNo[8], &szTemplate[3], 3);

		memset(&pobTran->srBRec.szRefNo[11], 0x20, 1);

		/* Set STAN */
		inFUBON_SetSTAN(pobTran);

		if(pobTran->srBRec.lnTxnAmount % 2)
		{
			memcpy(pobTran->srBRec.szEI_BankId, "NNN", 3);
		}else{
			memcpy(pobTran->srBRec.szEI_BankId, "012", 3);
		}
			
		
		inDISP_PutGraphic(_SEND_, 0, _COORDINATE_Y_LINE_8_7_); /* 傳送中... */
		inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_); /* 處理中... */
		inDISP_PutGraphic(_RECEIVE_, 0, _COORDINATE_Y_LINE_8_7_); /* 接收中... */
		inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_); /* 處理中... */
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] DEMO END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	} else
	{
		if (inFUBON_GetSTAN(pobTran) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" FU BuildAndSend Get Stan *Error* Line[%d]", __LINE__);
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

			inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inFUBON_ISO_BuildAndSendPacket Aft Comm Start");

			if (pobTran->inTransactionCode != _CHANGE_TMK_ && pobTran->inTransactionCode != _CUP_CHANG_TPK_)
			{
				if ((inRetVal = inFUBON_ISO_ProcessReversal(pobTran)) != VS_SUCCESS)
				{
					inDISP_DispLogAndWriteFlie(" FU Process Reversal *Error* Val[%d] Line[%d]", inRetVal, __LINE__);
					/* 修改錯誤訊息，如果沒有設定，就定為連線失敗 2022/11/7 [SAM] */
					if(pobTran->inErrorMsg == _ERROR_CODE_V3_NONE_)
						pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
					return (VS_ERROR);
				}
			}

			if (pobTran->inTransactionCode == _SETTLE_)
			{
				inSetMustSettleBit("Y");
				if (VS_SUCCESS != inSaveHDPTRec(pobTran->srBRec.inHDTIndex))
				{
					inDISP_DispLogAndWriteFlie(" FU BuildAndSend Save HDPT *Error* HID[%d] Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
				}

				/* 開啟 TmsFPT 下載TMS要結帳的參數 */
				inEDCTMS_FUNC_SetTmsFtpSettleFlagOn();

				if ((inRetVal = inFUBON_ISO_ProcessAdvice(pobTran)) != VS_SUCCESS)
				{
					inDISP_DispLogAndWriteFlie(" FU BuildAndSend Porcess Advice *Error* Val[%d] Line[%d]", inRetVal, __LINE__);
					/* 修改錯誤訊息，如果沒有設定，就定為連線失敗 2022/11/7 [SAM] */
					if(pobTran->inErrorMsg == _ERROR_CODE_V3_NONE_)
						pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
					return (VS_ERROR);
				}
			}
		}

		pobTran->uszContinueTxnAgainBit = VS_FALSE;

		do
		{
			if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
			{
				if ((inRetVal = inFUBON_ISO_ProcessOnline(pobTran)) != VS_SUCCESS)
				{
					if ((pobTran->srBRec.inChipStatus == _EMV_CARD_) &&
						(pobTran->inTransactionCode == _SALE_ || pobTran->inTransactionCode == _PRE_AUTH_) &&
						(inRetVal != VS_ISO_PACK_ERR) &&
						(inRetVal != VS_ISO_UNPACK_ERROR))
					{
						pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
						inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inFUBON_ISO_BuildAndSendPacket Aft Process Online Comm *Error*");
						inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] Aft Process Online Comm *Error* END -----", __FILE__, __FUNCTION__, __LINE__);
					} else
					{
						inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inFUBON_ISO_BuildAndSendPacket Aft Process Online *Error*");
						inFUBON_SetSTAN(pobTran);
						inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] Aft Process Online *Error* END -----", __FILE__, __FUNCTION__, __LINE__);
						return (inRetVal);
					}
				}

				inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inFUBON_ISO_BuildAndSendPacket Aft Process Online Success");
			} else
			{
				if (inFUBON_ISO_ProcessOffline(pobTran) != VS_SUCCESS)
				{
					inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] ProcessOffline Err END -----", __FILE__, __FUNCTION__, __LINE__);
					return (VS_ERROR);
				} else
					break;
			}

			pobTran->uszContinueTxnAgainBit = VS_FALSE;

			if (inFUBON_ISO_AnalysePack(pobTran) != VS_SUCCESS)
			{
				inFUBON_SetSTAN(pobTran);
				inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inFUBON_ISO_BuildAndSendPacket  AnalysePack *Error*");
				inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] Analyse *Error* END -----", __FILE__, __FUNCTION__, __LINE__);
				return (VS_ERROR);
			}

		} while (pobTran->uszContinueTxnAgainBit);

		inFUBON_SetSTAN(pobTran);
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inFUBON_ISO_BuildAndSendPacket END");

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * App Name		: inFUBON_ISO_ProcessReversal
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 5:46
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_ProcessReversal(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_SUCCESS;
	int inResult = VS_SUCCESS;
	char szTraceLog[100];
	char szSendReversalBit[2 + 1];
	char szMustISRUploadEnableBit[1 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memset(szSendReversalBit, 0x00, sizeof (szSendReversalBit));
	if (inGetSendReversalBit(szSendReversalBit) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" FU GetSendReversalBit *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	if (!memcmp(szSendReversalBit, "Y", 1))
	{
		if ((inRetVal = inFUBON_ISO_ReversalSend(pobTran)) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" FU Send Reversal *Error* Val[%d] Line[%d]", inRetVal, __LINE__);
			return (VS_ERROR);
		}
	}

	
	memset(szMustISRUploadEnableBit, 0x00, sizeof (szMustISRUploadEnableBit));
	if (inGetMustISRUploadEnableBit(szMustISRUploadEnableBit) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" FU GetMustISRUploadEnableBit *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	if (!memcmp(szMustISRUploadEnableBit, "1", 1))
	{
		inDISP_LogPrintfWithFlag(" ISR inFUBON_ISO_ProcessReversal() START!!");
		if ((inResult = inFUBON_ISO_ISR_ReversalSend(pobTran)) != VS_SUCCESS)
		{
			memset(szTraceLog, 0x00, sizeof (szTraceLog));
			inDISP_DispLogAndWriteFlie(" FU ISR Reversal Send *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		}
	}
	
	if (pobTran->uszReversalBit == VS_TRUE && inRetVal == VS_SUCCESS)
	{
		if ((inRetVal = inFUBON_ISO_ReversalSave_Flow(pobTran, _REVERSAL_)) != VS_SUCCESS)
		{
			memset(szTraceLog, 0x00, sizeof (szTraceLog));
			inDISP_DispLogAndWriteFlie(" FU Reversal Save Flow *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		}
	}

	if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
	{
		if (pobTran->uszReversalBit == VS_TRUE && inResult == VS_SUCCESS)
		{
			if ((inResult = inFUBON_ISO_ISR_ReversalSave(pobTran)) != VS_SUCCESS)
			{
				memset(szTraceLog, 0x00, sizeof (szTraceLog));
				inDISP_DispLogAndWriteFlie(" FU ISR Reversal Save *Error* Line[%d]", __LINE__);
				return (VS_ERROR);
			}
		}
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * App Name		: inFUBON_ISO_ProcessAdvice
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 5:52
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_ProcessAdvice(TRANSACTION_OBJECT *pobTran)
{
	int i, inAdvCnt;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	if ((inAdvCnt = inADVICE_GetTotalCount(pobTran)) == VS_NO_RECORD)
		return (VS_SUCCESS);

	for (i = 0; i < inAdvCnt; i++)
	{
		if (inFUBON_ISO_SendAdvice(pobTran, 1, VS_FALSE) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" FU ISO ProcessAdvice Send *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		}
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * App Name		: inFUBON_ISO_ProcessSettleBatchUpload
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 5:52
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_ProcessSettleBatchUpload(TRANSACTION_OBJECT *pobTran)
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
		inDISP_DispLogAndWriteFlie(" FU ISO inBatchTotalCnt[%d] *Error* Line[%d]", inBatchTotalCnt, __LINE__);
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
		inFUBON_SetSTAN(pobTran);

		inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);

		for (i = 0; i < inBatchTotalCnt; i++)
		{
			pobTran->inISOTxnCode = _BATCH_UPLOAD_;
			if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, i) != VS_SUCCESS)
			{
				inRunCLS_BATCH = VS_FALSE;
				break;
			}

			if (pobTran->srBRec.uszCLSBatchBit == '1')
			{
				continue;
			}

			if (pobTran->srBRec.inCode == _VOID_ || pobTran->srBRec.inCode == _PRE_AUTH_ ||
				pobTran->srBRec.inCode == _CUP_PRE_AUTH_ || pobTran->srBRec.inCode == _CUP_VOID_ ||
				pobTran->srBRec.inCode == _CUP_PRE_AUTH_VOID_ || pobTran->srBRec.inCode == _CUP_PRE_COMP_VOID_)
				continue;

			inBatchValidCnt++;

			if (inBatchValidCnt == inBatchValidTotalCnt)
				pobTran->uszLastBatchUploadBit = VS_TRUE;

			inFUBON_GetSTAN(pobTran);

			inRetVal = inFUBON_ISO_PackSendUnPackReceData(pobTran, pobTran->inISOTxnCode);
			inFUBON_SetSTAN(pobTran);

			if (inRetVal != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie(" FU Adv Send inISOTxnCode[%d] *Error* Line[%d]",  pobTran->inISOTxnCode, __LINE__);
				inRunCLS_BATCH = VS_FALSE;
				break;
			} else
			{
				if (memcmp(pobTran->srBRec.szRespCode, "00", 2))
				{
					inFUBON_DispHostResponseCode(pobTran);
					inRunCLS_BATCH = VS_FALSE;
					inDISP_DispLogAndWriteFlie(" FU Adv Send inISOTxnCode[%d] RespCode[%s] *Error* Line[%d]",  pobTran->inISOTxnCode, pobTran->srBRec.szRespCode, __LINE__);
					break;
				}

				inBATCH_Update_CLS_SettleBit_By_Sqlite(pobTran);
			}

		}

		inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);

		memcpy((char *) pobTran, (char *) &OrgpobTran, sizeof (TRANSACTION_OBJECT));
	}


	inFUBON_GetSTAN(pobTran);

	if (inRunCLS_BATCH == VS_TRUE)
	{
		inSetCLS_SettleBit("N");
		inSaveHDPTRec(pobTran->srBRec.inHDTIndex);

		pobTran->inISOTxnCode = _CLS_BATCH_;

		if (inFUBON_ISO_PackSendUnPackReceData(pobTran, pobTran->inISOTxnCode) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" FU Send _CLS_BATCH_ *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		}else
		{
			if (memcmp(pobTran->srBRec.szRespCode, "00", 2))
			{
				inDISP_DispLogAndWriteFlie(" FU Send _CLS_BATCH_ inISOTxnCode[%d] RespCode[%s] *Error* Line[%d]",  pobTran->inISOTxnCode, pobTran->srBRec.szRespCode, __LINE__);
				return (VS_ERROR);
			}
		}
	} else
	{
		inSetCLS_SettleBit("Y");
		inSaveHDPTRec(pobTran->srBRec.inHDTIndex);
		inDISP_DispLogAndWriteFlie(" FU _CLS_BATCH_ != VS_TRUE *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * App Name		: inFUBON_ISO_ProcessSettleCupLogon
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 5:58
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_ProcessSettleCupLogon(TRANSACTION_OBJECT *pobTran)
{
	return VS_SUCCESS;
}

/*
 * App Name		: inFUBON_ISO_ReversalSend
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 5:58
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_ReversalSend(TRANSACTION_OBJECT *pobTran)
{
	return inFUBON_ISO_ReversalSendRecvPacket(pobTran, _REVERSAL_);
}

/*
 * App Name		: inFUBON_ISO_ReversalSendRecvPacket
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 5:59
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_ReversalSendRecvPacket(TRANSACTION_OBJECT *pobTran, int inISOTxnCode)
{
	int inRetVal, inSendCnt;
	long lnREVCnt;
	char szDialBackupEnable[2 + 1];
	char szCommMode[2 + 1];
	unsigned char uszFileName[20 + 1];
	unsigned char uszSendPacket[_FUBON_ISO_SEND_ + 1], uszRecvPacket[_FUBON_ISO_RECV_ + 1];
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
			inDISP_DispLogAndWriteFlie(" RSRP ComposeFileName_1 *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		}
	} else
	{
		memset(uszFileName, 0x00, sizeof (uszFileName));
		if (inFunc_ComposeFileName(pobTran, (char*) uszFileName, (inISOTxnCode == _REVERSAL_) ? _REVERSAL_FILE_EXTENSION_ : _REVERSAL_ISR_FILE_EXTENSION_, 6) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" RSRP ComposeFileName_2 *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		}
	}

	/* 開檔案 */
	if ((inRetVal = inFILE_OpenReadOnly(&ulREVFHandle, uszFileName)) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" RSRP FILE_OpenReadOnly *Error* Line[%d]", __LINE__);
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

	inDISP_LogPrintfWithFlag("inFUBON_ISO_ReversalSendRecvPacket() Before");

	if ((inRetVal = inFUBON_ISO_SendReceData(pobTran, uszSendPacket, inSendCnt, uszRecvPacket)) != VS_SUCCESS)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;

		inDISP_DispLogAndWriteFlie(" RSRP ISO_SendReceData *Error* Line[%d]", __LINE__);
		return (VS_COMM_ERROR);
	}

	/* 解 ISO 電文 */
	pobTran->inISOTxnCode = inISOTxnCode;
	inRetVal = inFUBON_ISO_UnPackISO(pobTran, uszSendPacket, inSendCnt, uszRecvPacket, inRetVal);
	if (inRetVal != VS_SUCCESS)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_UNPACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
		inDISP_DispLogAndWriteFlie(" RSRP ISO_UnPackISO *Error* Line[%d]", __LINE__);
		return (VS_ISO_UNPACK_ERROR);
	}

	/* memcmp兩字串相同回傳0 */
	if (memcmp(pobTran->srBRec.szRespCode, "00", 2) != 0)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_CANCELLED_;
		return (VS_ERROR);
	} else
	{
		if (inISOTxnCode == _REVERSAL_)
			inSetSendReversalBit("N");
		else
			inSetMustISRUploadEnableBit("0");
		if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" RSRP SaveHDPTRec *Error* Line[%d]", __LINE__);
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
				inDISP_DispLogAndWriteFlie(" RSRP ComposeFileName_3 *Error* Line[%d]", __LINE__);
				return (VS_ERROR);
			}

			inFILE_Delete(uszFileName);

			/* 刪除非撥接Reversal */
			memset(uszFileName, 0x00, sizeof (uszFileName));
			if (inFunc_ComposeFileName(pobTran, (char*) uszFileName, (inISOTxnCode == _REVERSAL_) ? _REVERSAL_FILE_EXTENSION_ : _REVERSAL_ISR_FILE_EXTENSION_, 6) != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie(" RSRP ComposeFileName_4 *Error* Line[%d]", __LINE__);
				return (VS_ERROR);
			}

			inFILE_Delete(uszFileName);
		} else
		{
			memset(uszFileName, 0x00, sizeof (uszFileName));
			if (inFunc_ComposeFileName(pobTran, (char*) uszFileName, (inISOTxnCode == _REVERSAL_) ? _REVERSAL_FILE_EXTENSION_ : _REVERSAL_ISR_FILE_EXTENSION_, 6) != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie(" RSRP ComposeFileName_5 *Error* Line[%d]", __LINE__);
				return (VS_ERROR);
			}

			inFILE_Delete(uszFileName);
		}
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * App Name		: inFUBON_ISO_ReversalSave
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 6:05
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_ReversalSave(TRANSACTION_OBJECT *pobTran, int inISOTxnCode)
{
	int inRetVal, inPacketCnt;
	unsigned char uszReversalPacket[_FUBON_ISO_RECV_ + 1];
	unsigned char uszFileName[20 + 1];
	unsigned long srFHandle;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" RS inHDTIndex[%d] inISOTxnCode[%d]", pobTran->srBRec.inHDTIndex, inISOTxnCode);

	inDISP_LogPrintfArea(TRUE, "", 0, uszReversalPacket, 300);

	memset(uszReversalPacket, 0x00, sizeof (uszReversalPacket));
	pobTran->inISOTxnCode = inISOTxnCode; /* 沖銷 */
	//pobTran->inISOTxnCode = _REVERSAL_; /* 沖銷 */

	/* 組_REVERSAL_ 封包 */
	inPacketCnt = inFUBON_ISO_PackISO(pobTran, &uszReversalPacket[0], pobTran->inISOTxnCode);
	if (inPacketCnt <= 0)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_PACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_PACK_;
		inDISP_DispLogAndWriteFlie(" RS ISO_PackISO *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	memset(uszFileName, 0x00, sizeof (uszFileName));
	if (inFunc_ComposeFileName(pobTran, (char*) uszFileName, (inISOTxnCode == _REVERSAL_) ? _REVERSAL_FILE_EXTENSION_ : _REVERSAL_ISR_FILE_EXTENSION_, 6) != VS_SUCCESS)
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
	if (inISOTxnCode == _REVERSAL_)
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
 * App Name		: inFUBON_ISO_ProcessOnline
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 6:12
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_ProcessOnline(TRANSACTION_OBJECT *pobTran)
{
	char szTraceLog[100];
	int inRetVal;

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, " inFUBON_ISO_ProcessOnline INIT");

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	if (pobTran->uszContinueTxnAgainBit == VS_TRUE)
	{
		if (pobTran->uszReversalBit == VS_TRUE)
		{
			if ((inRetVal = inFUBON_ISO_ReversalSave_Flow(pobTran, _REVERSAL_)) != VS_SUCCESS)
			{
				memset(szTraceLog, 0x00, sizeof (szTraceLog));
				inDISP_DispLogAndWriteFlie(" P_Oline ISO_ReversalSave_Flow *Error* Line[%d]", __LINE__);
				return (VS_ERROR);
			}

			/* ISR UPLOAD */
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
			{
				if ((inRetVal = inFUBON_ISO_ISR_ReversalSave(pobTran)) != VS_SUCCESS)
				{
					inDISP_DispLogAndWriteFlie(" P_Oline ISO_ISR_ReversalSave *Error* Line[%d]", __LINE__);
					return (VS_ERROR);
				}
			}
		}
	}


	pobTran->inISOTxnCode = pobTran->inTransactionCode;
	inRetVal = inFUBON_ISO_PackSendUnPackReceData(pobTran, pobTran->inISOTxnCode);

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "AFT_inFUBON_ISO_PackSendUnPackReceData");

	if (inRetVal == VS_ERROR)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
		inDISP_DispLogAndWriteFlie(" P_Oline ISO_PackSendUnPackReceData *Error* Line[%d]", __LINE__);
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
		}
		inDISP_DispLogAndWriteFlie(" P_Oline Unpack *Error* [%d]  Line[%d]", inRetVal, __LINE__);
		return (inRetVal);
	} else
	{
		pobTran->inTransactionResult = inFUBON_ISO_CheckRespCode(pobTran);

		inDISP_LogPrintfWithFlag(" Fubon Aft Check Resp Code [%d] ", pobTran->inTransactionResult);

		if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
		{
			if (pobTran->srEMVRec.in71_IssuerScript1Len > 0 || pobTran->srEMVRec.in72_IssuerScript2Len > 0)
			{
				if ((pobTran->inTransactionCode == _PRE_AUTH_ || pobTran->inTransactionCode == _CUP_PRE_AUTH_))
				{
					inDISP_DispLogAndWriteFlie(" P_Oline ISR Upload With PRE_AUTH Line[%d]", __LINE__);
					inSetMustISRUploadEnableBit("1");
					if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) < 0)
					{
						inDISP_DispLogAndWriteFlie(" P_Oline inSaveHDPTRec_1[%d] *Error* Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
						return (VS_ERROR);
					}
				} else if (pobTran->inTransactionResult != _TRAN_RESULT_AUTHORIZED_)
				{
					inDISP_DispLogAndWriteFlie(" P_Oline ISR Upload With AUTH *Error* Line[%d]", __LINE__);
					inSetMustISRUploadEnableBit("1");
					if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) < 0)
					{
						inDISP_DispLogAndWriteFlie(" P_Oline inSaveHDPTRec_2[%d] *Error* Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
						return (VS_ERROR);
					}
				}
			}
		}

		if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_)
		{
			if (inFUBON_ISO_CheckAuthCode(pobTran) != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie(" P_Oline ISO_CheckAuthCode *Error* Line[%d]", __LINE__);
				return (VS_ISO_UNPACK_ERROR);
			}
		} else if (pobTran->inTransactionResult == VS_ISO_UNPACK_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" P_Oline VS_ISO_UNPACK_ERROR *Error* Line[%d]", __LINE__);
			return (VS_ISO_UNPACK_ERROR);
		}
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inFUBON_ISO_ProcessOnline_END");

	inDISP_LogPrintfWithFlag(" TxnResult[%d] ", pobTran->inTransactionResult);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
 * App Name		: inFUBON_ISO_ProcessOffline
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 6:19
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_ProcessOffline(TRANSACTION_OBJECT *pobTran)
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
				inDISP_DispLogAndWriteFlie(" P_Offline Ad DeleteRec *Error* Line[%d]", __LINE__);
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
				inDISP_DispLogAndWriteFlie(" P_Offline Ad SaveAddend *Error* Line[%d]", __LINE__);
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
		inDISP_DispLogAndWriteFlie(" P_Offline Other Fail *Error* Line[%d]", __LINE__);
		inRetVal = VS_ERROR;
	}
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
 * App Name		: inFUBON_ISO_CheckRespCode
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 6:24
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_CheckRespCode(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = _TRAN_RESULT_COMM_ERROR_;
	char szCommmode[1 + 1] = {0}, szCallBankEnable[2 + 1] = {0};

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);	
	
	inDISP_LogPrintfWithFlag(" CheckRespCode RCode[%s] ", pobTran->srBRec.szRespCode);

	if (!memcmp(pobTran->srBRec.szRespCode, "00", 2))
	{
		inRetVal = _TRAN_RESULT_AUTHORIZED_;
	} else if (!memcmp(pobTran->srBRec.szRespCode, "01", 2) ||
			!memcmp(pobTran->srBRec.szRespCode, "02", 2))
	{
		switch (pobTran->inISOTxnCode)
		{
			case _SALE_:
				inGetCommMode(szCommmode);
				inGetCallBankEnable(szCallBankEnable);
				if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				{
					strcpy(pobTran->srBRec.szRespCode, "05");
					inRetVal = _TRAN_RESULT_CANCELLED_;
				}/* GPRS Call Bank */
				else if ((memcmp(szCommmode, _COMM_GPRS_MODE_, 1) != 0) && szCallBankEnable[0] == 'Y')
					inRetVal = _TRAN_RESULT_REFERRAL_;
				else
				{
					inRetVal = _TRAN_RESULT_CANCELLED_;
				}
				break;
			default:
				inRetVal = _TRAN_RESULT_CANCELLED_;
				break;
		}
	} else if (!memcmp(pobTran->srBRec.szRespCode, "  ", 2))
	{
		inRetVal = VS_ISO_UNPACK_ERROR;
	} else if (pobTran->srBRec.szRespCode[0] == 0x00)
	{
		inRetVal = _TRAN_RESULT_COMM_ERROR_;
	} else if (!memcmp(pobTran->srBRec.szRespCode, "DD", 2))
	{
		inDISP_LogPrintfWithFlag("inFUBON_ISO_CheckRespCode 2!!");

		inSetMustSettleBit("Y");
		/* 開啟 TmsFPT 下載TMS要結帳的參數 */
		inEDCTMS_FUNC_SetTmsFtpSettleFlagOn();

		if (VS_SUCCESS != inSaveHDPTRec(pobTran->srBRec.inHDTIndex))
		{
			inDISP_DispLogAndWriteFlie(" FU BuildAndSend Save HDPT *Error* HID[%d] Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
		}

		/* 富邦ECR 回覆 "DD" 時, ECR回覆需回 "請先結帳"的訊息，所以要新增此判斷 2020/1/30 下午 4:44 [SAM] */
		if (pobTran->uszKioskFlag == 'Y')
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_PLESE_SETTLE_;

		inRetVal = _TRAN_RESULT_CANCELLED_;
	} else if (!memcmp(&pobTran->srBRec.szRespCode[0], "D1", 2) ||
			!memcmp(&pobTran->srBRec.szRespCode[0], "D2", 2) ||
			!memcmp(&pobTran->srBRec.szRespCode[0], "L1", 2) ||
			!memcmp(&pobTran->srBRec.szRespCode[0], "CK", 2) ||
			!memcmp(&pobTran->srBRec.szRespCode[0], "CD", 2) ||
			!memcmp(&pobTran->srBRec.szRespCode[0], "RA", 2))
	{
		inRetVal = _TRAN_RESULT_CANCELLED_;
	} else
	{
		if ((pobTran->srBRec.szRespCode[0] >= '0' && pobTran->srBRec.szRespCode[0] <= '9') &&
				(pobTran->srBRec.szRespCode[1] >= '0' && pobTran->srBRec.szRespCode[1] <= '9'))
		{
			if (pobTran->inISOTxnCode == _SETTLE_ && !memcmp(&pobTran->srBRec.szRespCode[0], "95", 2))
				inRetVal = _TRAN_RESULT_SETTLE_UPLOAD_BATCH_;
			else
				inRetVal = _TRAN_RESULT_CANCELLED_;
		} else
		{
			inRetVal = VS_ISO_UNPACK_ERROR;
		}
	}

	inDISP_LogPrintfWithFlag(" Return Val[%d]", inRetVal);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}


/*
 * App Name		: inFUBON_ISO_CheckAuthCode
 * App Builder		: 
 * App Date&Time	: 2022/8/5 下午 6:28
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_CheckAuthCode(TRANSACTION_OBJECT *pobTran)
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
			if (!memcmp(&pobTran->srBRec.szAuthCode[0], "000000", 6) || !memcmp(&pobTran->srBRec.szAuthCode[0], "      ", 6))
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
 * App Name		: inFUBON_ISO_AnalysePack
 * App Builder		: 
 * App Date&Time	: 2022/8/9 上午 10:43
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_AnalysePack(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_SUCCESS;
	ISO_TYPE_FUBON_TABLE srISOFunc;
	char szEncryptMode[1 + 1] = {0};

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" B.inChipStatus[%d] ", pobTran->srBRec.inChipStatus);

	if (pobTran->inTransactionCode == _SALE_ &&
		pobTran->inEMVDecision == _EMV_DECESION_OFFLINE &&
		//pobTran->inEMVCardDecision == EMV_OFFLINE_AUTH &&
		pobTran->srBRec.inChipStatus == _EMV_CARD_ &&
		pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_ &&
		!memcmp(&pobTran->srBRec.szAuthCode[0], "Y1", 2))
	{
		inDISP_LogPrintfWithFlag(" inFUBON_ISO_AnalysePack()_Y1_END!!");
		return (VS_SUCCESS);
	}

	if (pobTran->srBRec.uszOfflineBit == VS_TRUE)
	{
		inDISP_LogPrintfWithFlag(" inFUBON_ISO_AnalysePack()_OFFLINE_END!!");
		return (inRetVal);
	}

	inGetEncryptMode(szEncryptMode);
	memset((char *) &srISOFunc, 0x00, sizeof (srISOFunc));
	memcpy((char *) &srISOFunc, (char *) &srFUBON_ISOFunc[szEncryptMode[0] - '0'], sizeof (srISOFunc));

	if (srISOFunc.inOnAnalyse != NULL)
	{
		inRetVal = srISOFunc.inOnAnalyse(pobTran);
	}

	inDISP_LogPrintfWithFlag(" AnalysePack inRetVal[%d]", inRetVal);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}


/*
 * App Name		: inFUBON_ISO_SendAdvice
 * App Builder		: 
 * App Date&Time	: 2022/8/9 上午 10:45
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_SendAdvice(TRANSACTION_OBJECT *pobTran, int inAdvCnt, VS_BOOL blTcUpload)
{

	int inCnt, inRetVal = VS_ERROR, inISOFuncIndex = -1;
	TRANSACTION_OBJECT AdvpobTran;
	ISO_TYPE_FUBON_TABLE srISOFunc;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	inISOFuncIndex = 0;
	memset((char *) &srISOFunc, 0x00, sizeof (srISOFunc));
	memcpy((char *) &srISOFunc, (char *) &srFUBON_ISOFunc[inISOFuncIndex], sizeof (srISOFunc));

	memset((char *) &AdvpobTran, 0x00, sizeof (TRANSACTION_OBJECT));
	memcpy((char *) &AdvpobTran, (char *) pobTran, sizeof (TRANSACTION_OBJECT));

	if (AdvpobTran.uszTCUploadBit != VS_TRUE && AdvpobTran.uszFiscConfirmBit != VS_TRUE)
	{
		if (inBATCH_GlobalAdviceHandleReadOnly_By_Sqlite() == VS_ERROR)
		{
			inDISP_LogPrintfWithFlag(" inFUBON_ISO_SendAdvice() fail 1!!");
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

		if ((blTcUpload == VS_TRUE) &&
				(AdvpobTran.srBRec.uszTCUploadBit == VS_FALSE) &&
				(AdvpobTran.srBRec.inChipStatus == _EMV_CARD_) &&
				(AdvpobTran.inTransactionCode == _SALE_ ||
				AdvpobTran.inTransactionCode == _INST_SALE_ || AdvpobTran.inTransactionCode == _REDEEM_SALE_))
		{
			AdvpobTran.inISOTxnCode = _TC_UPLOAD_;
		} else
		{
			if (AdvpobTran.srBRec.inChipStatus == _EMV_CARD_ && AdvpobTran.srBRec.uszTCUploadBit == VS_FALSE)
			{
				inDISP_DispLogAndWriteFlie(" EMV_TCUPLOAD ERR B.InCode[%d] ", AdvpobTran.srBRec.inCode);
				inDISP_DispLogAndWriteFlie(" B.uszUpload1Bit[%d] B.uszUpload2Bit[%d]", AdvpobTran.srBRec.uszUpload1Bit, AdvpobTran.srBRec.uszUpload2Bit);
				if (!memcmp(AdvpobTran.srBRec.szAuthCode, "Y1", 2) ||
						!memcmp(AdvpobTran.srBRec.szAuthCode, "Y3", 2) ||
						AdvpobTran.srBRec.uszReferralBit == VS_TRUE)
				{
					if (AdvpobTran.srBRec.inCode == _TIP_)
					{
						if (AdvpobTran.srBRec.uszUpload1Bit == VS_TRUE && AdvpobTran.srBRec.uszUpload2Bit == VS_TRUE)
							AdvpobTran.inISOTxnCode = _ADVICE_;
						else
							AdvpobTran.inISOTxnCode = _TIP_;
					} else if (AdvpobTran.srBRec.inCode == _ADJUST_)
					{
						if (AdvpobTran.srBRec.uszUpload1Bit == VS_TRUE && AdvpobTran.srBRec.uszUpload2Bit == VS_TRUE)
							AdvpobTran.inISOTxnCode = _ADVICE_;
						else
							AdvpobTran.inISOTxnCode = _ADJUST_;
					} else
						AdvpobTran.inISOTxnCode = _ADVICE_;
				} else
				{
					AdvpobTran.inISOTxnCode = _TC_UPLOAD_;
				}
			} else
			{
				inDISP_DispLogAndWriteFlie(" NON_EMV_TCUPLOAD ERR B.InCode[%d] ", AdvpobTran.srBRec.inCode);
				inDISP_DispLogAndWriteFlie(" B.uszUpload1Bit[%d] B.uszUpload2Bit[%d]", AdvpobTran.srBRec.uszUpload1Bit, AdvpobTran.srBRec.uszUpload2Bit);
				if (AdvpobTran.srBRec.inCode == _TIP_)
				{
					if (AdvpobTran.srBRec.uszUpload1Bit == VS_TRUE && AdvpobTran.srBRec.uszUpload2Bit == VS_TRUE)
						AdvpobTran.inISOTxnCode = _ADVICE_;
					else
						AdvpobTran.inISOTxnCode = _TIP_;
				} else if (AdvpobTran.srBRec.inCode == _ADJUST_)
				{
					if (AdvpobTran.srBRec.uszUpload1Bit == VS_TRUE && AdvpobTran.srBRec.uszUpload2Bit == VS_TRUE)
						AdvpobTran.inISOTxnCode = _ADVICE_;
					else
						AdvpobTran.inISOTxnCode = _ADJUST_;
				} else
					AdvpobTran.inISOTxnCode = _ADVICE_;
			}
		}

		if (inRetVal == VS_SUCCESS)
			inRetVal = inFUBON_ISO_PackSendUnPackReceData(&AdvpobTran, AdvpobTran.inISOTxnCode);

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

/*
 * App Name		: inFUBON_ISO_ReversalSave_For_DialBeckUp
 * App Builder		: 
 * App Date&Time	: 2022/8/9 上午 11:04
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_ReversalSave_For_DialBeckUp(TRANSACTION_OBJECT *pobTran)
{
	inDISP_LogPrintfWithFlag("inFUBON_ISO_ReversalSave_For_DialBeckUp() END!!");
	return (VS_SUCCESS);
}

/*
 * App Name		: inFUBON_ISO_PackISO_FOR_DialBackUp
 * App Builder		: 
 * App Date&Time	: 2022/8/9 上午 11:04
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_PackISO_FOR_DialBackUp(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode)
{
	int i, inSendLen, inField, inCnt;
	int inBitMap[_FUBON_MAX_BIT_MAP_CNT_];
	int inRetVal, inISOFuncIndex = -1;
	unsigned char bBuf[100];
	ISO_TYPE_FUBON_TABLE srISOFunc;
	char szCommmode[1 + 1] = {0}, szEncryptMode[1 + 1] = {0};
	unsigned char uszBCD[20 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	inGetCommMode(szCommmode);
	inGetEncryptMode(szEncryptMode);
	inDISP_LogPrintfWithFlag(" FIPIFD COMM_MODE[%02x] ENCRYPT_MODE[%02x] ",
			szCommmode[0], szEncryptMode[0]);
	if (memcmp(szCommmode, _COMM_MODEM_MODE_, 1) == 0)
	{
		fFubonField_35 = VS_FALSE;
		fFubonField_55 = VS_FALSE;
		inISOFuncIndex = szEncryptMode[0] - '0';
	} else
		inISOFuncIndex = 0;

	if (inISOFuncIndex >= 3 || inISOFuncIndex < 0)
	{
		inDISP_Msg_BMP(_ERR_ISO_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		return (VS_ERROR);
	}

	inSendLen = 0;
	inField = 0;
	memset((char *) &srISOFunc, 0x00, sizeof (srISOFunc));
	
	memcpy((char *) &srISOFunc, (char *) &srFUBON_ISOFunc[inISOFuncIndex], sizeof (srISOFunc));
	memset((char *) inBitMap, 0x00, sizeof (inBitMap));
	memset((char *) bBuf, 0x00, sizeof (bBuf));

	if (inGetTPDU((char *) bBuf) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" FIPIFD inGetTPDU() ERROR!");
		return (VS_ERROR);
	}

	memset(uszBCD, 0x00, sizeof (uszBCD));
	inFunc_ASCII_to_BCD(uszBCD, (char *) bBuf, 5);
	memcpy((char *) &uszSendBuf[inSendLen], (char *) uszBCD, _FUBON_TPDU_SIZE_);
	inSendLen += _FUBON_TPDU_SIZE_;
	memset(bBuf, 0x00, sizeof (bBuf));

	inRetVal = inFUBON_ISO_GetBitMap(pobTran, inTxnCode, &srISOFunc, inBitMap, &uszSendBuf[inSendLen]);
	if (inRetVal <= VS_ERROR)
	{
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] GetBitMap *Error* END-----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
	else
		inSendLen += inRetVal;

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
			inCnt = srISOFunc.srPackISO[inField].inISOLoad(pobTran, &uszSendBuf[inSendLen]);
			if (inCnt <= 0)
			{
				inDISP_LogPrintfWithFlag(" FIPIFD ISO_Field_Index[%d] *Error* END", srISOFunc.srPackISO[inField].inFieldNum);

				if (srISOFunc.srPackISO[inField].inFieldNum == 35 || srISOFunc.srPackISO[inField].inFieldNum == 55)
				{
					inDISP_Msg_BMP(_ERR_READ_CARD_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
				}
				return (VS_ERROR);
			} else
				inSendLen += inCnt;
		}
	}

	if (srISOFunc.vdModifyPackData != NULL)
		srISOFunc.vdModifyPackData(pobTran, uszSendBuf, &inSendLen);

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END-----", __FILE__, __FUNCTION__, __LINE__);
	return (inSendLen);
}

/*
 * App Name		: inFUBON_ISO_ISR_ReversalSave
 * App Builder		: 
 * App Date&Time	: 2022/8/9 上午 11:54
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_ISR_ReversalSave(TRANSACTION_OBJECT *pobTran)
{
	return inFUBON_ISO_ReversalSave_Flow(pobTran, _ISR_UPLOAD_);
}

/*
 * App Name		: inFUBON_ISO_ISR_ReversalSave_For_DialBeckUp
 * App Builder		: 
 * App Date&Time	: 
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_ISR_ReversalSave_For_DialBeckUp(TRANSACTION_OBJECT *pobTran)
{
	inDISP_LogPrintfWithFlag("inFUBON_ISO_ISR_ReversalSave_For_DialBeckUp() END!!");
	return (VS_SUCCESS);
}

/*
 * App Name		: inFUBON_ISO_ReversalSendRecvPacket
 * App Builder		: 
 * App Date&Time	: 2022/8/9 上午 11:55
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_ISR_ReversalSend(TRANSACTION_OBJECT *pobTran)
{
	return inFUBON_ISO_ReversalSendRecvPacket(pobTran, _ISR_UPLOAD_);
}

/*
 * App Name		: inFUBON_ISO_MakeReferenceNumber
 * App Builder		: 
 * App Date&Time	: 2022/8/9 上午 11:55
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_MakeReferenceNumber(TRANSACTION_OBJECT *pobTran)
{
	char szTID[8 + 1];
	char szBatchNum[6 + 1];
	char szST[6 + 1];
	char szTemp[6 + 1];
	long lnSTANNum;
	short inHostInvNum;
	long lnBatchNum = 0L;
	char szTemplate[10 + 1], szTemplate2[42 + 1] = {0}, szSTANNum[12 + 1] = {0};

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, " inFUBON_ISO_MakeReferenceNumber INIT");

	memset(szTemplate, 0x00, sizeof (szTemplate));
	memset(szTemplate2, 0x00, sizeof (szTemplate2));
	if (inGetBatchNum(szTemplate) == VS_SUCCESS)
	{
		lnBatchNum = atol(szTemplate);
	} else
	{
		inDISP_DispLogAndWriteFlie(" FU inGetBatchNum *Error* Line[%d]", __LINE__);
	}

	inGetInvoiceNum(szTemplate2);
	inHostInvNum = atoi(szTemplate2);

	if ((inHostInvNum >= 999) || (inHostInvNum == 0))
		inHostInvNum = 1;

	sprintf(szTemplate2, "%06d", inHostInvNum);
	inSetInvoiceNum(szTemplate2);
	inSaveHDTRec(0);

	inGetTerminalID(szTID);
	sprintf(szTemp, "%ld", lnBatchNum);
	inFunc_PAD_ASCII(szBatchNum, szTemp, '0', 6, _PAD_RIGHT_FILL_LEFT_);

	if (inGetSTANNum(szSTANNum) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] inGetSTANNum_1 END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	lnSTANNum = atol(szSTANNum);

	if ((lnSTANNum % 100 == 99) && (lnSTANNum >= 99))
	{
		inFUBON_SetSTAN(pobTran);
		memset(szSTANNum, 0x00, sizeof (szSTANNum));
		if (inGetSTANNum(szSTANNum) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] inGetSTANNum_2 END -----", __FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
		}
		lnSTANNum = atol(szSTANNum);
	}

	pobTran->srBRec.lnSTANNum = lnSTANNum;
	sprintf(szTemp, "%ld", pobTran->srBRec.lnSTANNum);
	inDISP_LogPrintfWithFlag(" Make TEMP  [%s]", szTemp);

	inFunc_PAD_ASCII(szST, szTemp, '0', 6, _PAD_RIGHT_FILL_LEFT_);
	inDISP_LogPrintfWithFlag(" Make ST  [%s]", szST);


	memset(pobTran->srBRec.szRefNo, 0x00, sizeof (pobTran->srBRec.szRefNo));
	strcpy(pobTran->srBRec.szRefNo, "9");
	memcpy(&pobTran->srBRec.szRefNo[1], &szTID[3], 5);
	memcpy(&pobTran->srBRec.szRefNo[6], &szBatchNum[4], 2);
	memcpy(&pobTran->srBRec.szRefNo[8], &szST[3], 3);
	memset(&pobTran->srBRec.szRefNo[11], 0x20, 1);


	inDISP_LogPrintfWithFlag(" Make RRN [%s]", pobTran->srBRec.szRefNo);

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inFUBON_ISO_MakeReferenceNumber END");

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
 * App Name		: inFunc_REFERRAL_GetManualApproval
 * App Builder		: 
 * App Date&Time	: 2022/8/9 上午 11:57
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_REFERRAL_GetManualApproval(TRANSACTION_OBJECT *pobTran)
{

	if (inFunc_REFERRAL_GetManualApproval(pobTran) != VS_SUCCESS)
		return (VS_ERROR);

	return (VS_SUCCESS);
}


/*
 * App Name		: SoftWareTransferPack
 * App Builder		: 
 * App Date&Time	: 2022/8/9 上午 11:59
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int SoftWareTransferPack(char *Data, int DataLen, char *OutBcdData)
{
	unsigned char temp[100];
	int i, j, BcdLen;

	if (DataLen > (sizeof (temp)))
	{
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Datalen[%d] *Error* END -----", __FILE__, __FUNCTION__, __LINE__, DataLen);
		return (VS_FALSE);
	}

	memcpy(temp, Data, DataLen);

	/* Length is not Even added a charcacter '0' */
	if (DataLen % 2)
	{
		temp[DataLen] = '0';
		DataLen++;
	}

	temp[DataLen] = 0x00;

	for (i = 0; i < DataLen; i++)
	{
		if ((temp[i] >= 0x30) && (temp[i] <= 0x3f))
			temp[i] = temp[i] - '0';
		else if ((temp[i] >= 'A') && (temp[i] <= 'F'))
			temp[i] = temp[i] - 'A' + 10;
		else if ((temp[i] >= 'a') && (temp[i] <= 'f'))
			temp[i] = temp[i] - 'a' + 10;
		else
		{
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Data Format *Error* END -----", __FILE__, __FUNCTION__, __LINE__);
			return (VS_FALSE);
		}
	}

	j = 0;

	for (i = 0; i < ((DataLen + 1) / 2); i++)
	{
		OutBcdData[i] = (temp[j] << 4) + temp[j + 1];
		j += 2;
	}
	BcdLen = ((DataLen + 1) / 2);

	return (BcdLen);
}
//end

/*
 * App Name		: inFUBON_GetISOTableData
 * App Builder		: 
 * App Date&Time	: 2022/8/9 上午 11:59
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_GetISOTableData(char* TableId, unsigned char* InputData, int InputLen, char* OutData, int* Outlen)
{
	char TempData[200];
	int i = 0;

	memset(TempData, 0x00, sizeof (TempData));
	if (InputLen > 0)
	{
		memcpy(TempData, InputData, InputLen);
	} else
	{
		return (VS_ERROR);
	}

	for (i = 0; i < InputLen; i++)
	{
		if (memcmp(&TempData[i], TableId, 2) == 0)
		{
			if (TempData[i + 2] > 0)
			{
				memcpy(OutData, &TempData[i + 3], TempData[i + 2]);
				*Outlen = TempData[i + 2];
				return (VS_SUCCESS);
			} else
			{
				return (VS_ERROR);
			}
		}
	}

	return (VS_ERROR);
}


/*
 * App Name		: inFUBON_ISO_ReversalSave_Flow
 * App Builder		: 
 * App Date&Time	: 2022/8/9 下午 12:04
 * App Function		: 
 * Input Param		: 
 * Output Param		: 
 */
int inFUBON_ISO_ReversalSave_Flow(TRANSACTION_OBJECT *pobTran, int inISOTxnCode)
{
	int inRetVal;
	char szDialBackupEnable[2 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	inRetVal = inFUBON_ISO_ReversalSave(pobTran, inISOTxnCode);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] ReversalSave *Error* END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	memset(szDialBackupEnable, 0x00, sizeof (szDialBackupEnable));
	inGetDialBackupEnable(szDialBackupEnable);

	if (memcmp(szDialBackupEnable, "Y", strlen("Y")) == 0)
	{

		inRetVal = inFUBON_ISO_ReversalSave_For_DialBeckUp(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] Dial_Bek ReversalSave *Error* END -----", __FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
		}

	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

