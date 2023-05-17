#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>

#include "../../SOURCE/INCLUDES/Define_1.h"
#include "../../SOURCE/INCLUDES/Define_2.h"
#include "../../SOURCE/INCLUDES/Transaction.h"
#include "../../SOURCE/INCLUDES/TransType.h"

#include "../../SOURCE/FUNCTION/Function.h"
#include "../../SOURCE/FUNCTION/Sqlite.h"
#include "../../SOURCE/FUNCTION/CPT.h"
#include "../../SOURCE/FUNCTION/HDT.h"
#include "../../SOURCE/FUNCTION/HDPT.h"
#include "../../SOURCE/FUNCTION/MVT.h"
#include "../../SOURCE/FUNCTION/CFGT.h"
#include "../../SOURCE/FUNCTION/Signpad.h"


#include "../../SOURCE/DISPLAY/Display.h"

#include "../../SOURCE/EVENT/MenuMsg.h"

#include "../../SOURCE/COMM/Comm.h"

#include "../../SOURCE/PRINT/Print.h"

#include "../../SOURCE/KEY/ProcessTmk.h"

#include "../FUNCTION/FILE_FUNC/File.h"
#include "../FUNCTION/FILE_FUNC/FIleLogFunc.h"

#include "../../CTLS/CTLS.h"

#include "NexEscTableFn.h"
#include "NexsysEscIso.h"
#include "NexsysEsc.h"
#include "NexEscTablePack.h"


unsigned char	guszNEXSYS_ESC_ISO_Field03[_NEXSYS_ESC_PCODE_SIZE_ + 1];
unsigned char	guszNEX_tSAMCheckSum_ESC35[4 + 1];
unsigned char	guszNEX_FiscBit = VS_FALSE;			/* Fisc要用ascii壓 */

extern unsigned char	guszNEX_tSAMKeyIndex_ESC;
extern int	ginNEX_EscUploadIndex;	/* 上傳中使用 */
extern int	ginNEX_ESC_Connect_flag ;	/* idle畫面是否與ESC主機連線 一直重連會耗時*/
extern int	ginNEX_ESC_Send_Cnt;	/* 電子簽單上傳計數 超過三次不上傳 */

extern char gszNEX_ESC_Date;		/* yyyymmdd */
extern char gszNEX_ESC_Time;		/* hhmmss */

NEX_ESC_DATA	 NEX_ESC_UPLOAD_DATA[_ESC_LIMIT_];			/* 總上傳資料 */

int inNEXSYS_ESC_ISOGetBitMapCode(TRANSACTION_OBJECT *pobTran, int inTxnType)
{
        int 	inBitMapTxnCode = -1;

        switch (inTxnType)
        {
                case _ESC_UPLOAD_ :
                        inBitMapTxnCode = inTxnType;
                        break;
                default:
                        break;
        }

        return (inBitMapTxnCode);
}


/*
Function        :inNEXSYS_ESC_ISOPackMessageType
Date&Time       :
Describe        :
*/
int inNEXSYS_ESC_ISOPackMessageType(TRANSACTION_OBJECT *pobTran, int inTxnCode, unsigned char *uszPackData, char *szMTI)
{
        int 		inCnt = 0;
        unsigned char 	uszBCD[10 + 1];
	
	/* Force Sale(強制授權交易)，Field_25=”06”。Force交易定義為Online的補登交易。
	   分期調帳及紅利調帳處理邏輯比照【Force Sale(強制授權交易)】 */
	if ((pobTran->srBRec.uszForceOnlineBit == VS_TRUE) &&
	    (inTxnCode == _SALE_OFFLINE_ || inTxnCode == _FORCE_CASH_ADVANCE_ ||
	     inTxnCode == _REDEEM_ADJUST_ || inTxnCode == _INST_ADJUST_))
	{
		memset(szMTI, 0x00, sizeof(szMTI));
		strcpy(szMTI, "0200"); /* Default */
	}

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szMTI[0], _NEXSYS_ESC_MTI_SIZE_);
        memcpy((char *) &uszPackData[inCnt], (char *) &uszBCD[0], _NEXSYS_ESC_MTI_SIZE_);
        inCnt += _NEXSYS_ESC_MTI_SIZE_;

        return (inCnt);
}

/*
Function        :inNEXSYS_ESC_ISOModifyBitMap
Date&Time       :
Describe        :inTxnType from pobTran->inISOTxnCode
*/
int inNEXSYS_ESC_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inTxnType, int *inBitMap)
{
	char	szEncryptMode[2 + 1];
	
	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	inESC_DebugLog("  InCode[%d]",pobTran->srBRec.inCode);
	
	if (strlen(pobTran->srBRec.szStoreID) > 0)
		inNEXSYS_ESC_BitMapSet(inBitMap, 48);
	
	memset(szEncryptMode, 0x00, sizeof(szEncryptMode));
	inGetEncryptMode(szEncryptMode);
	if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_TSAM_, strlen(_NCCC_ENCRYPTION_TSAM_)) == 0)
	{
		inNEXSYS_ESC_BitMapSet(inBitMap, 57);
	}

	if (pobTran->srBRec.inCode == _TIP_)
		inNEXSYS_ESC_BitMapSet(inBitMap, 54);
	
	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE || 
	     pobTran->srBRec.uszRedeemBit == VS_TRUE)
			inNEXSYS_ESC_BitMapSet(inBitMap, 63);
	
	/* ESC_UPLOAD 不用送 MAC */
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
        return (VS_SUCCESS);
}

int inNEXSYS_ESC_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackData, int *inPackLen)
{
	char	szCommMode[2 + 1];
	char	szEncryptionMode[2 + 1];
	
	/* 用TSAM加密，要放TSam key index到最後一個Bit */
	memset(szCommMode, 0x00, sizeof(szCommMode));
	inGetCommMode(szCommMode);
	if (memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0 || memcmp(szCommMode, _COMM_GPRS_MODE_, 1) == 0)
	{
		memset(szEncryptionMode, 0x00, sizeof(szEncryptionMode));
		inGetEncryptMode(szEncryptionMode);

		if (memcmp(szEncryptionMode, _NCCC_ENCRYPTION_TSAM_, 1) == 0)
			uszPackData[4] = guszNEX_tSAMKeyIndex_ESC; /* TPDU 最後一個 Byte */
	}
	
        return (VS_SUCCESS);
}

int inNEXSYS_ESC_ISOCheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader)
{
	int	inCnt = 0;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 檢查TPDU */
	inCnt += _NEXSYS_ESC_TPDU_SIZE_;

	/* 檢查MTI */
	szSendISOHeader[inCnt + 1] += 0x10;
	if (memcmp(&szSendISOHeader[inCnt], &szReceISOHeader[inCnt], _NEXSYS_ESC_MTI_SIZE_))
		return (VS_ERROR);

	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}


int inNEXSYS_ESC_BitMapSet(int *inBitMap, int inFeild)
{
        int 	i, inBMapCnt, inBitMapCnt;
        int 	inBMap[_NEXSYS_ESC_MAX_BIT_MAP_CNT_];

        memset((char *) inBMap, 0x00, sizeof(inBMap));
        inBMapCnt = inBitMapCnt = 0;

        for (i = 0; i < 64; i++)
        {
                if (inBitMap[inBitMapCnt] > inFeild)
                {
                        if (i == 0)
                        {
                                /* 第一個BitMap */
                                inBMap[inBMapCnt++] = inFeild;
                                break;
                        }
                }
                else if (inBitMap[inBitMapCnt] < inFeild)
                {
                        inBMap[inBMapCnt++] = inBitMap[inBitMapCnt++];
                        if (inBitMap[inBitMapCnt] == 0 || inBitMap[inBitMapCnt] > inFeild) /* 最後一個 BitMap */
                        {
                                inBMap[inBMapCnt++] = inFeild;
                                break;
                        }
                }
                else
                        break; /* if (*inBitMap == inFeild) */
        }

        for (i = 0;; i++)
        {
                if (inBitMap[inBitMapCnt] != 0)
                        inBMap[inBMapCnt++] = inBitMap[inBitMapCnt++];
                else
                        break;
        }

        memcpy((char *) inBitMap, (char *) inBMap, sizeof(inBMap));

        return (VS_SUCCESS);
}

int inNEXSYS_ESC_BitMapReset(int *inBitMap, int inFeild)
{
        int 	i, inBMapCnt;
        int 	inBMap[_NEXSYS_ESC_MAX_BIT_MAP_CNT_];

        memset((char *) inBMap, 0x00, sizeof(inBMap));
        inBMapCnt = 0;

        for (i = 0; i < 64; i++)
        {
                if (inBitMap[i] == 0)
                        break;
                else if (inBitMap[i] != inFeild)
                        inBMap[inBMapCnt++] = inBitMap[i];
        }

        memcpy((char *) inBitMap, (char *) inBMap, sizeof(inBMap));

        return (VS_SUCCESS);
}

int inNEXSYS_ESC_BitMapCheck(unsigned char *inBitMap, int inFeild)
{
        int 	inByteIndex, inBitIndex;

        inFeild--;
        inByteIndex = inFeild / 8;
        inBitIndex = 7 - (inFeild - inByteIndex * 8);

        if (_NEXSYS_ESC_CHECK_BIT_MAP_(inBitMap[inByteIndex], inBitIndex))
        {
                return (VS_TRUE);
        }

        return (VS_FALSE);
}

/*
Function        :inNEXSYS_ESC_CopyBitMap
Date&Time       :
Describe        :
*/
int inNEXSYS_ESC_CopyBitMap(int *inBitMap, int *inSearchBitMap)
{
        int 	i;

        for (i = 0; i < _NEXSYS_ESC_MAX_BIT_MAP_CNT_; i++)
        {
                if (inSearchBitMap[i] == 0)
                        break;
                else
                        inBitMap[i] = inSearchBitMap[i];
        }

        return (VS_SUCCESS);
}

/*
Function        :inNEXSYS_ESC_GetBitMapTableIndex
Date&Time       :
Describe        :
*/
int inNEXSYS_ESC_GetBitMapTableIndex(ISO_TYPE_NEXSYS_ESC_TABLE *srISOFunc, int inBitMapTxnCode)
{
        int 	inBitMapIndex;

        for (inBitMapIndex = 0;; inBitMapIndex++)
        {
                if (srISOFunc->srBitMap[inBitMapIndex].inTxnID == _NEXSYS_ESC_NULL_TX_)
                {
	           inESC_DebugLog("  Ｃan Not Found BIT MAP [%d] ", inBitMapIndex);
                        return (VS_ERROR); /* 找不到相對應交易別的Bit Map */
                }

                if (srISOFunc->srBitMap[inBitMapIndex].inTxnID == inBitMapTxnCode)
                        break; /* 找到一樣的交易類別 */
        }

        return (inBitMapIndex);
}


int inNEXSYS_ESC_GetBitMapMessagegTypeField03(TRANSACTION_OBJECT *pobTran, ISO_TYPE_NEXSYS_ESC_TABLE *srISOFunc, int inTxnType, int *inTxnBitMap,
                unsigned char *uszSendBuf)
{
	int 	i, j, k, inBitMapIndex, *inBitMap, inCnt, inBitMapTxnCode = -1;
	unsigned char 	uszBuf;

	/* 設定交易別 */
	inBitMapTxnCode = srISOFunc->inGetBitMapCode(pobTran, inTxnType);
	if (inBitMapTxnCode == -1)
		return (VS_ERROR);

	/* 要搜尋 BIT_MAP_NEXSYS_ESC_TABLE srNEXSYS_ESC_ISOBitMap 相對應的 inTxnID */
	if ((inBitMapIndex = inNEXSYS_ESC_GetBitMapTableIndex(srISOFunc, inBitMapTxnCode)) == VS_ERROR)
		return (VS_ERROR);

	/* Pack Message Type */
	inCnt = 0;
	inCnt += srISOFunc->inPackMTI(pobTran, inTxnType, &uszSendBuf[inCnt], srISOFunc->srBitMap[inBitMapIndex].szMTI);


	for (i = 0; i < 20; i++)
	{
		inESC_DebugLog("1.inBitMap = %d", srISOFunc->srBitMap[inBitMapIndex].inBitMap[i]);
	}

	/* 要搜尋 BIT_MAP_NEXSYS_ESC_TABLE srNEXSYS_ESC_ISOBitMap 相對應的 inBitMap */
	inNEXSYS_ESC_CopyBitMap(inTxnBitMap, srISOFunc->srBitMap[inBitMapIndex].inBitMap);

	for (i = 0; i < 20; i++)
	{
		inESC_DebugLog("2.inBitMap = %d", srISOFunc->srBitMap[inBitMapIndex].inBitMap[i]);
	}
        
	/* 修改 Bit Map */
	srISOFunc->inModifyBitMap(pobTran, inTxnType, inTxnBitMap);

	/* Process Code */
	memset(guszNEXSYS_ESC_ISO_Field03, 0x00, sizeof(guszNEXSYS_ESC_ISO_Field03));
	inESC_DebugLog("  Get szPCode [%d]", inBitMapIndex);
	inESC_DebugLog("  szPCode [%x][%x][%x][%x]", srISOFunc->srBitMap[inBitMapIndex].szPCode[0],
				     srISOFunc->srBitMap[inBitMapIndex].szPCode[1], srISOFunc->srBitMap[inBitMapIndex].szPCode[2], srISOFunc->srBitMap[inBitMapIndex].szPCode[3]);
	inFunc_ASCII_to_BCD(&guszNEXSYS_ESC_ISO_Field03[0], srISOFunc->srBitMap[inBitMapIndex].szPCode, 3);
	
	
	/* Pack Bit Map */
	inBitMap = inTxnBitMap;
	for (i = 0;; i++)
	{
		uszBuf = 0x80;
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
			uszBuf = uszBuf >> 1;
			k--;
		}

		uszSendBuf[inCnt + j] += uszBuf;
		inBitMap++;
	}

        inCnt += _NEXSYS_ESC_BIT_MAP_SIZE_;

        return (inCnt);
}


int inNEXSYS_ESC_Pack02(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;
	int inPANLen;
	char szPAN[20 + 1], szPackPAN[20 + 1];

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	memset(szPAN, 0x00, sizeof(szPAN));
	inPANLen = strlen(pobTran->srBRec.szPAN);
	uszPackBuf[inCnt ++] = (inPANLen / 10 * 16) + (inPANLen % 10);/*bcd的長度,例如16碼卡號,這邊會變22,但其實bcd的0x16就是22*/
	memcpy(szPAN, pobTran->srBRec.szPAN, inPANLen);

	//若卡號為奇數 前補 0
	memset(szPackPAN, 0x00, sizeof(szPackPAN));
	if (inPANLen % 2)
	{
		szPackPAN[0] = '0';
		memcpy(&szPackPAN[1], szPAN, inPANLen);
		inFunc_ASCII_to_BCD((unsigned char *)&uszPackBuf[inCnt], szPackPAN,  (inPANLen + 1)/ 2);
		inCnt += (inPANLen + 1 )/ 2;
	}
	else
	{
		memcpy(&szPackPAN[0], szPAN, inPANLen);
		inFunc_ASCII_to_BCD((unsigned char *)&uszPackBuf[inCnt], szPackPAN,  inPANLen / 2);
		inCnt += inPANLen / 2;
	}
	
	inESC_DebugLog("  DataCnt[%d]",inCnt);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
Function        :inNEXSYS_ESC_Pack03
Date&Time       :
Describe        :Field_3:       Processing Code
*/
int inNEXSYS_ESC_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int 	inCnt = 0;
	char	szTemplate[42 + 1];

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	memcpy(&uszPackBuf[inCnt], &guszNEXSYS_ESC_ISO_Field03[0], _NEXSYS_ESC_PCODE_SIZE_);

	inCnt += _NEXSYS_ESC_PCODE_SIZE_;


	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(szTemplate, guszNEXSYS_ESC_ISO_Field03, _NEXSYS_ESC_PCODE_SIZE_);
	inESC_DebugLog("F_03 [P_CODE %s]", szTemplate);


	inESC_DebugLog("  DataCnt[%d]",inCnt);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
Function        :inNEXSYS_ESC_Pack04
Date&Time       :
Describe        :Field_4:       Amount, Transaction
*/
int inNEXSYS_ESC_Pack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int 		inCnt = 0;
	char 		szTemplate[42 + 1];
	unsigned char 	uszBCD[6 + 1];

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	memset(szTemplate, 0x00, sizeof(szTemplate));

	switch (pobTran->inISOTxnCode)
	{
		case _ESC_UPLOAD_:
			if (pobTran->srBRec.inCode == _TIP_)
				sprintf(szTemplate, "%010ld00", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
			else if (pobTran->srBRec.inCode == _ADJUST_)
				sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnAdjustTxnAmount);
			else
				sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);
			break;
		default:
			inESC_DebugLog("  inESC_Pack04()_TxnCode = %d ERROR", pobTran->inISOTxnCode);
			return (VS_ERROR);
	}

	memset(uszBCD, 0x00, sizeof(uszBCD));
	inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 6);
	memcpy((char *) &uszPackBuf[inCnt], (char *) &uszBCD[0], 6);
	inCnt += 6;

	
	inESC_DebugLog("F_04 [AMT %s]", szTemplate);
	inESC_DebugLog("  DataCnt[%d]",inCnt);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (inCnt);
}

/*
Function        :inNEXSYS_ESC_Pack11
Date&Time       :
Describe        :Field_11:      System Trace Audit Number
*/
int inNEXSYS_ESC_Pack11(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inESC_Hostindex;
	int 		inCnt = 0;
	char 		szSTAN[6 + 1];
	unsigned char 	uszBCD[3 + 1];
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

//	/* 找到 ESC HOST 並存 ginNEX_ESCHostIndex 參數並讀取 HDT 不會影響 srBRec.inHDTIndex */
//	inNEXSYS_ESC_SwitchToESC_Host(pobTran->srBRec.inHDTIndex);
	
	if (inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_ESC_, &inESC_Hostindex) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Esc Pack 11 *Error* Name[%s] Line[%d] ",_HOST_NAME_ESC_, __LINE__);
		inFunc_EDCLock();
		return (VS_ERROR);
	}
	
	if( inLoadHDTRec(inESC_Hostindex) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" Esc Pack 11 Load ESC HDT[%d] *Error* Line[%d]",inESC_Hostindex,__LINE__);
	}

	memset(szSTAN, 0x00, sizeof(szSTAN));
	inGetSTANNum(szSTAN);

	/* 記得切回來 */
	if( inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" Esc Pack 11 Recover Load HDT[%d] *Error* Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
	}
	
	if( inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" Esc Pack 11 Recover Load HDPT[%d] *Error* Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
	}

	memset(uszBCD, 0x00, sizeof(uszBCD));
	inFunc_ASCII_to_BCD(&uszBCD[0], &szSTAN[0], 3);
	memcpy((char *) &uszPackBuf[inCnt], (char *) &uszBCD[0], 3);
	inCnt += 3;


	inESC_DebugLog("F_011 [STAN %s]", szSTAN);
	inESC_DebugLog("  DataCnt[%d]",inCnt);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (inCnt);
}

/*
Function        :inNEXSYS_ESC_Pack12
Date&Time       :
Describe        :Field_12:      Time, Local Transaction
*/
int inNEXSYS_ESC_Pack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int 		inCnt = 0;
	unsigned char 	uszBCD[3 + 1];
	
	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	memset(uszBCD, 0x00, sizeof(uszBCD));
	inFunc_ASCII_to_BCD(&uszBCD[0], &pobTran->srBRec.szTime[0], 3);
	memcpy((char *) &uszPackBuf[inCnt], (char *) &uszBCD[0], 3);
	inCnt += 3;
	
	
	inESC_DebugLog("  F_012 [TIME %s]", pobTran->srBRec.szTime);
	inESC_DebugLog("  DataCnt[%d]",inCnt);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
        return (inCnt);
}

/*
Function        :inNEXSYS_ESC_Pack13
Date&Time       :
Describe        :Field_13:      Date, Local Transaction
*/
int inNEXSYS_ESC_Pack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int 		inCnt = 0;
	char		szTemplate[42 + 1];
	unsigned char 	uszBCD[2 + 1];
	
	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	memset(uszBCD, 0x00, sizeof(uszBCD));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &pobTran->srBRec.szDate[4], 4);
	inFunc_ASCII_to_BCD(&uszBCD[0], szTemplate, 2);
	memcpy((char *) &uszPackBuf[inCnt], (char *) &uszBCD[0], 2);
	inCnt += 2;
	
		
	inESC_DebugLog("  F_013 [DATE %s]", szTemplate);
	inESC_DebugLog("  DataCnt[%d]",inCnt);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

        return (inCnt);
}


/*
Function        :inNEXSYS_ESC_Pack14
Date&Time       :2018/8/16 下午 5:17
Describe        :Expire date
*/
int inNEXSYS_ESC_Pack14(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int 		inCnt = 0;
	char		szTemplate[42 + 1];
	unsigned char 	uszBCD[(_EXP_DATE_SIZE_ / 2) + 1] = {0};

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	memset(uszBCD, 0x00, sizeof(uszBCD));

	/* 目前是使用固定的值 20190328 [SAM] */
	inFunc_ASCII_to_BCD(&uszBCD[0], "1299", _EXP_DATE_SIZE_ / 2);
	memcpy((char *) &uszPackBuf[inCnt], (char *) &uszBCD[0], _EXP_DATE_SIZE_ / 2);
	inCnt += _EXP_DATE_SIZE_ / 2;

	inESC_DebugLog("  F_014 [TIME %s]", szTemplate);
	inESC_DebugLog("  DataCnt[%d]",inCnt);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
Function        :inNEXSYS_ESC_Pack22
Date&Time       :
Describe        :Field_22:   Point of Service Entry Mode
*/
int inNEXSYS_ESC_Pack22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int 	inCnt = 0;
	char	szTermCap[6 + 1];
	VS_BOOL fTermPinFlag = VS_FALSE;
	int		inIndex = -1;
	char	szTerminalCapbilityAscii[6 + 1];	/* TerminalCapbility的Ascii值 */
	char	szTerminalCapbility[3 + 1];		/* TerminalCapbility的Hex值 */
	char szPOSmode[4+1];
	
	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	inESC_DebugLog("inFUBON_ISO_Pack22 inTransactionCode = %d", pobTran->inTransactionCode);
	inESC_DebugLog("inFUBON_ISO_Pack22 inCode = %d", pobTran->srBRec.inCode);
	inESC_DebugLog("inFUBON_ISO_Pack22 inISOTxnCode = %d", pobTran->inISOTxnCode);
	inESC_DebugLog("inFUBON_ISO_Pack22 inOrgCode = %d,%d,%02x,%d", 
					pobTran->srBRec.inOrgCode,pobTran->srBRec.uszCUPTransBit,
					pobTran->szPIN[0],pobTran->srBRec.uszPinEnterBit);

	memset(szTermCap, 0x00, sizeof(szTermCap));
	memset(szPOSmode, 0x00, sizeof(szPOSmode));

	/* 1.比對TerminalCapbility */
	memset(szTerminalCapbility, 0x00, sizeof(szTerminalCapbility));
	if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)) == 0)
	{
		inIndex = _VISA_MVT_INDEX_;
	}
	else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
	{
		inIndex = _MCHIP_MVT_INDEX_;
	}
	else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)) == 0)
	{
		inIndex = _JSMART_MVT_INDEX_;
	}
	else if (memcmp(pobTran->srBRec.szCUP_EMVAID, "A000000333010101", strlen("A000000333010101")) == 0)
	{
		inIndex = _CUP_DEBIT_MVT_INDEX_;
	}
	else if (memcmp(pobTran->srBRec.szCUP_EMVAID, "A000000333010102", strlen("A000000333010102")) == 0)
	{
		inIndex = _CUP_CREDIT_MVT_INDEX_;
	}
	else if (memcmp(pobTran->srBRec.szCUP_EMVAID, "A000000333010103", strlen("A000000333010103")) == 0)
	{
		inIndex = _CUP_QUASI_CREDIT_MVT_INDEX_;
	}
	else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
	{
		inIndex = _AEIPS_MVT_INDEX_;
	}
	else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0)
	{
		inIndex = _D_PAS_MVT_INDEX_;
	}
	else
	{
		inIndex = -1;
	}

	memset(szTerminalCapbilityAscii, 0x00, sizeof(szTerminalCapbilityAscii));
	memset(szTerminalCapbility, 0x00, sizeof(szTerminalCapbility));
	if (inIndex != -1)
	{
		inLoadMVTRec(inIndex);
		inGetTerminalCapabilities(szTerminalCapbilityAscii);

		/* 有值才繼續 */
		if (strlen(szTerminalCapbilityAscii) > 0)
		{
			inFunc_ASCII_to_BCD((unsigned char*)szTerminalCapbility, szTerminalCapbilityAscii, 3);

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
			if(pobTran->srBRec.uszPinEnterBit == VS_TRUE)
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

	if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
	{
        if ( pobTran->srBRec.inCode == _CUP_VOID_ || pobTran->srBRec.inCode == _CUP_PRE_AUTH_VOID_) /* 2013-12-02 AM 10:12:19 add by kakab �s�W���p���ΰh�f��Ӥ@��H�Υd����y�{ */
		{
			if (fTermPinFlag == VS_TRUE)
				memcpy(szPOSmode, "5011", 2);
			else
				memcpy(szPOSmode, "5012", 2);
		}
		else
		{
			if (fTermPinFlag == VS_TRUE)
				memcpy(szPOSmode, "5051", 2);
			else
				memcpy(szPOSmode, "5052", 2);
		}
	}
	else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
	{
		if (pobTran->srBRec.inCode == _CUP_VOID_)
		{
			if (fTermPinFlag == VS_TRUE)
				memcpy(szPOSmode, "3011", 2);
			else
				memcpy(szPOSmode, "3012", 2);
		}
		else if ((pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _CUP_REFUND_) ||
			 (pobTran->srBRec.inOrgCode == _REFUND_ && pobTran->srBRec.inCode == _VOID_))
		{
			inESC_DebugLog("inFUBON_ISO_Pack22 inCode = _REFUND_ ,inCode = _CUP_REFUND_, inOrgCode = _REFUND_ & inCode = _VOID_");
			memcpy(szPOSmode, "5022", 2);
		}
		else if (fTermPinFlag == VS_TRUE)
		{
			if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
				memcpy(szPOSmode, "3071", 2);
			else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE)
			        memcpy(szPOSmode, "4911", 2);
			else
				memcpy(szPOSmode, "3071", 2);
		}
		else
		{
		        if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
				memcpy(szPOSmode, "3072", 2);
			else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE)
			        memcpy(szPOSmode, "4912", 2);
			else
				memcpy(szPOSmode, "3072", 2);
		}
	}
	else if (pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
	{
		if (fTermPinFlag == VS_TRUE)
			memcpy(szPOSmode, "5801", 2);
		else
			memcpy(szPOSmode, "5802", 2);

		if ( pobTran->srBRec.inCode == _CUP_VOID_ || pobTran->srBRec.inCode == _CUP_PRE_AUTH_VOID_) 
		{
			if (fTermPinFlag == VS_TRUE)
				memcpy(szPOSmode, "5011", 2);
			else
				memcpy(szPOSmode, "5012", 2);
		}
	}
	else
	{
		if (pobTran->srBRec.uszManualBit == VS_TRUE  ||
		    pobTran->srBRec.inCode == _CUP_VOID_ ||
		    pobTran->srBRec.inCode == _CUP_PRE_AUTH_VOID_)
		{
			if (fTermPinFlag == VS_TRUE)
				memcpy(szPOSmode, "5011", 2);
			else
				memcpy(szPOSmode, "5012", 2);
		}
		else
		{
			if (fTermPinFlag == VS_TRUE)
				memcpy(szPOSmode, "5021", 2);
			else
				memcpy(szPOSmode, "5022", 2);
		}
	}

	inCnt += 2;

	inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], szPOSmode, 2);
	inESC_DebugLog("  F_022 [POSMode %s]", szPOSmode);
	inESC_DebugLog("  DataCnt[%d]",inCnt);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (inCnt);
}

/*
Function        :inNEXSYS_ESC_Pack24
Date&Time       :
Describe        :Field_24:      Network International Identifier (NII)
*/
int inNEXSYS_ESC_Pack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int 		inCnt = 0;
	char 		szTemplate[42 + 1];
	unsigned char 	uszBCD[2 + 1];
	
	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* For ESC電子簽單，上傳流程要使用ESC Host */
	inNEXSYS_ESC_SwitchToESC_Host(pobTran->srBRec.inHDTIndex);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (inGetNII(&szTemplate[1]) == VS_ERROR)
		return (VS_ERROR);
	
	/* 記得load回來 */
	if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie("  inNEXSYS_ESC_Pack24 Recover HDT[%d] *Error* Line[%d]",pobTran->srBRec.inHDTIndex, __LINE__);
	}

	/* NII 只有三碼所以前面補0 */
	szTemplate[0] = '0' ;

	memset(uszBCD, 0x00, sizeof(uszBCD));
	inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 2);

	memcpy((char *) &uszPackBuf[inCnt], (char *) &uszBCD[0], 2);
	inCnt += 2;
		
	inESC_DebugLog("  F_024 [NII %s]", szTemplate);
	inESC_DebugLog("  DataCnt[%d]",inCnt);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}

/*
Function        :inNEXSYS_ESC_Pack25
Date&Time       :
Describe        :Field_25:   Point of Service Condition Code
*/
int inNEXSYS_ESC_Pack25(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int 		inCnt = 0;
	char		szTemplate[42 + 1];
	char		szSignStatus[2 + 1];
	char		szEncryptMode[2 + 1];
	unsigned char 	uszBCD[1 + 1];
	
	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* For ESC電子簽單，上傳流程 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	/*
		第一個Nibble表示加密方式，
		0  tSAM加密
		9  3DES軟體加密

		第二個Nibble表示本交易有無電子簽名檔。
		0  有上傳電子簽名檔
		1  無上傳電子簽名檔(免簽名)
		(刪除線(是否改為一律帶1))2  無上傳電子簽名檔(持卡人使用紙本簽名) 
	*/
	
	memset(szEncryptMode, 0x00, sizeof(szEncryptMode));
	inGetEncryptMode(szEncryptMode);
	if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_TSAM_, strlen(_NCCC_ENCRYPTION_TSAM_)) == 0)
	{
		strcpy(szTemplate, "0");
	}
	else
	{
		strcpy(szTemplate, "9");
	}
	
	memset(szSignStatus, 0x00, sizeof(szSignStatus));
	if (pobTran->srBRec.inSignStatus == _SIGN_SIGNED_)
	{
		sprintf(szSignStatus, "%d", 0);
	}
	else
	{
		sprintf(szSignStatus, "%d", 1);
	}
	memcpy(&szTemplate[1], szSignStatus, 1);
	
	inFunc_ASCII_to_BCD(uszBCD, szTemplate, 1);
        memcpy((char *) &uszPackBuf[inCnt], (char *)uszBCD, 1);
        inCnt++;
	
	inESC_DebugLog("F_025 [POSCondition code %s]", szTemplate);
	inESC_DebugLog("  DataCnt[%d]",inCnt);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

        return (inCnt);
}

/*
Function        :inNEXSYS_ESC_Pack35
Date&Time       :2017/5/4 下午 6:02
Describe        :Field_35:   TRACK 2 Data
*/
int inNEXSYS_ESC_Pack35(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int     inCnt = 0, inPacketCnt = 0;
	char    szT2Data[100 + 1];
	char	szFinalPAN[20 + 1];		/* 為了U CARD */
	char	szEncrypt[16 + 1];
	char	szResult[16 + 1];
	char	szAscii[16 + 1];
	short   shLen;
	
	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

        /*
        1.	本系統所有卡號一律透過ISO8583 Field_35上傳，不使用Field_02及Field_14。
	2.	所有交易只上傳【卡號】=【有效期】
	3.	U Card卡號上傳規則【11碼卡號】=【有效期】。
	4.	本欄位為敏感性資料須符合PCI規範，若以TCP/IP 協定上傳之交易電文，本欄位須依照本中心規範進行卡號加密。

        */
        
	/* PAN + 'D' + Expire Date */
	memset(szT2Data, 0x00, sizeof(szT2Data));

	/* Field 35 raw data */
	/* 備註：U CARD */
	memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
	if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
	{
		memcpy(szFinalPAN, &pobTran->srBRec.szPAN[4], 11);
	}
	else
	{
		strcpy(szFinalPAN, pobTran->srBRec.szPAN);
	}
	
	/* 初始化 */
	guszNEX_FiscBit = VS_FALSE;
	
	/* 金融卡（SmartPay） */
        if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
        {
		guszNEX_FiscBit = VS_TRUE;
		
                uszPackBuf[inCnt ++] = 0x16;
		
                /* 帳號（PAN） */
		/* 開始加密 */
		memset(szEncrypt, 0x00, sizeof(szEncrypt));
		memset(szResult, 0x00, sizeof(szResult));
		/* 轉hex */
		inFunc_ASCII_to_BCD((unsigned char*)szEncrypt, pobTran->srBRec.szFiscAccount, 8);
		inNCCC_TMK_ESC_3DES_Encrypt(szEncrypt, 8, szResult);

		/* 轉回 ascii 塞回去 */
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szResult, 8);

		/* 直接塞回去 */
                memcpy(&uszPackBuf[inCnt], szAscii, _FISC_ACCOUNT_SIZE_);
                inCnt += pobTran->srBRec.inFiscAccountLength;
        }
        /* 信用卡 */
        else
        {
		strcpy(szT2Data, szFinalPAN);
		inPacketCnt += strlen(szFinalPAN);

		strcat(szT2Data, "D");
		inPacketCnt ++;

		memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szExpDate[0], 4);
		inPacketCnt += 4;

		/* 卡號 + 有效期不滿16碼要補F */
		if (strlen(szT2Data) < 16)
		{
			inFunc_PAD_ASCII(szT2Data, szT2Data, 'F', 16, _PAD_RIGHT_FILL_LEFT_);
			inPacketCnt = 16;
		}
		/* F35最前面放長度*/
		shLen = strlen(szT2Data);

		uszPackBuf[inCnt] = (shLen / 10 * 16) + (shLen % 10);
		inCnt ++;

		/* 如果長度不為偶數，需補0*/
		if (shLen % 2)
		{
			shLen ++;
			strcat(szT2Data, "0");
		}

		/* 開始加密 */
		memset(szEncrypt, 0x00, sizeof(szEncrypt));
		memset(szResult, 0x00, sizeof(szResult));
		/* 轉hex */
		inFunc_ASCII_to_BCD((unsigned char*)szEncrypt, szT2Data, 8);
		inNCCC_TMK_ESC_3DES_Encrypt(szEncrypt, 8, szResult);

		/* 轉回 ascii 塞回去 */
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szResult, 8);

		memcpy(szT2Data, szAscii, 16);

		/* 一起轉回HEX並塞到packet中*/
		inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], &szT2Data[0], (shLen / 2));
		inCnt += (shLen / 2);
	}
	
	
	inESC_DebugLog("  F_35 [T2DATA  %s]", szT2Data);
	inESC_DebugLog("  DataCnt[%d]",inCnt);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
        return (inCnt);
}

/*
Function        :inNEXSYS_ESC_Pack35_tSAM
Date&Time       :2017/5/4 下午 6:02
Describe        :Field_35:   TRACK 2 Data
*/
int inNEXSYS_ESC_Pack35_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;
//	int		inCnt = 0, inPacketCnt = 0;
//	int		inLen = 0;
//	int		i, inRetVal = -1;
//	char		szT2Data[100 + 1];
//	char		szFinalPAN[20 + 1];		/* 為了U CARD */
//	char		szEncrypt[16 + 1];
//	char		szAscii[84 + 1];
//	char		szTemplate[100 + 1];
//	char		szField37[8 + 1], szField41[4 + 1];
//	unsigned char	uszSlot = 0;
	
	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
/* 目前沒在用，先拿掉 2019/03/13 [SAM] */
#if 0	
	/* 抓tSAM Slot */
	inRetVal = inNCCC_tSAM_Decide_tSAM_Slot(&uszSlot);
	
	if (inRetVal != VS_SUCCESS)
	{
		inESC_DebugLog("  inNCCC_tSAM_Decide_tSAM_Slot Failed");
	}

        /*
        1.	本系統所有卡號一律透過ISO8583 Field_35上傳，不使用Field_02及Field_14。
	2.	所有交易只上傳【卡號】=【有效期】
	3.	U Card卡號上傳規則【11碼卡號】=【有效期】。
	4.	本欄位為敏感性資料須符合PCI規範，若以TCP/IP 協定上傳之交易電文，本欄位須依照本中心規範進行卡號加密。

        */
        
	/* PAN + 'D' + Expire Date */
	memset(szT2Data, 0x00, sizeof(szT2Data));

	/* Field 35 raw data */
	/* 備註：U CARD */
	memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
	if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
	{
		memcpy(szFinalPAN, &pobTran->srBRec.szPAN[4], 11);
	}
	else
	{
		strcpy(szFinalPAN, pobTran->srBRec.szPAN);
	}
	
	/* 初始化 */
	guszNEX_FiscBit = VS_FALSE;
	
	/* 金融卡（SmartPay） */
        if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
        {
		guszNEX_FiscBit = VS_TRUE;
		
                uszPackBuf[inCnt ++] = 0x16;
		
		/* 金融卡帳號開始加密 */
		/* 組 Field_37 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, pobTran->srBRec.szRefNo, 11);
		inFunc_PAD_ASCII(szTemplate, szTemplate, 'F', 16, _PAD_LEFT_FILL_RIGHT_); /* 補【F】 */
		memset(szField37, 0x00, sizeof(szField37));
		inFunc_ASCII_to_BCD((unsigned char*)szField37, szTemplate, 8);
		/* 組 Field_41 */
		/* Load回原Host */
		inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTerminalID(szTemplate);
		memset(szField41, 0x00, sizeof(szField41));
		inFunc_ASCII_to_BCD((unsigned char*)szField41, szTemplate, 4);
		/* Field_35 */
		memset(guszNEX_tSAMCheckSum_ESC35, 0x00, sizeof(guszNEX_tSAMCheckSum_ESC35));
		memset(szEncrypt, 0x00, sizeof(szEncrypt));

		/* 這裡32是指32 nibble = 16 Bytes */
		inLen = 32;
		
		memcpy(szEncrypt, pobTran->srBRec.szFiscAccount, inLen / 2);
		
		inRetVal = inNCCC_tSAM_Encrypt(uszSlot, 
					       inLen,
					       szEncrypt,
					       (unsigned char*)&szField37,
					       (unsigned char*)&szField41,
					       &guszNEX_tSAMKeyIndex_ESC,
					       guszNEX_tSAMCheckSum_ESC35);

		if (inRetVal == VS_ERROR)
			return (VS_ERROR);

                /* 帳號（PAN） */
                memcpy(&uszPackBuf[inCnt], szEncrypt, inRetVal);
                inCnt += inRetVal;
        }
        /* 信用卡 */
        else
        {
		/* Data = 後面Data總長+長度+卡號+D+有效期 */
                /* 修改ESC F_35加密規則，移除長度兩碼 by Wei Hsiu - 2014/6/23 上午 10:35:37 */
		
		strcpy(szT2Data, szFinalPAN);
		inPacketCnt += strlen(szFinalPAN);

		strcat(szT2Data, "D");
		inPacketCnt ++;

		memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szExpDate[0], 4);
		inPacketCnt += 4;

		/* 卡號 + 有效期不滿16碼要補F */
		if (strlen(szT2Data) < 16)
		{
			inFunc_PAD_ASCII(szT2Data, szT2Data, 'F', 16, _PAD_LEFT_FILL_RIGHT_);
			inPacketCnt = 16;
		}
		/* F35最前面放長度*/
		inLen = strlen(szT2Data);

		uszPackBuf[inCnt] = (inLen / 10 * 16) + (inLen % 10);
		inCnt ++;

		/* 如果長度不為偶數，需補0*/
		if (inLen % 2)
		{
			inLen ++;
			strcat(szT2Data, "0");
		}
		
		/* Field_35 */
		memset(guszNEX_tSAMCheckSum_ESC35, 0x00, sizeof(guszNEX_tSAMCheckSum_ESC35));
		memset(szEncrypt, 0x00, sizeof(szEncrypt));

		/* 空的部份全補F */
		if (inLen < 16)
		{
			for (i = inLen; i < 16; i ++)
			{
				szT2Data[i] = 0x46;
			}
			inLen = 16;
			
			inFunc_ASCII_to_BCD((unsigned char*)szEncrypt, szT2Data, inLen / 2);
		}
		else
			inFunc_ASCII_to_BCD((unsigned char*)szEncrypt, szT2Data, inLen / 2);
		
		
		inESC_DebugLog("  F_35 [szEncrypt ] [%s] L:[%d]", szAscii, inLen);

		/* 開始加密 */
		/* 組 Field_37 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, pobTran->srBRec.szRefNo, 11);
		inFunc_PAD_ASCII(szTemplate, szTemplate, 'F', 16, _PAD_LEFT_FILL_RIGHT_); /* 補【F】 */
		memset(szField37, 0x00, sizeof(szField37));
		inFunc_ASCII_to_BCD((unsigned char*)szField37, szTemplate, 8);
		
		
		inESC_DebugLog("  F_35 [field37: %s] %d", szAscii, 8);
		
		/* 組 Field_41 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTerminalID(szTemplate);
		memset(szField41, 0x00, sizeof(szField41));
		inFunc_ASCII_to_BCD((unsigned char*)szField41, szTemplate, 4);
		

		inESC_DebugLog("  F_35 [field41: %s] %d", szAscii, 4);
		
		memcpy((char *)&uszPackBuf[inCnt], szEncrypt, (inLen / 2));

		inESC_DebugLog("  inNEXSYS_ESC_Pack35_tSAM() 開始加密");
		
		inRetVal = inNCCC_tSAM_Encrypt(uszSlot, 
					       inLen,
					       szEncrypt,
					       (unsigned char*)&szField37,
					       (unsigned char*)&szField41,
					       &guszNEX_tSAMKeyIndex_ESC,
					       guszNEX_tSAMCheckSum_ESC35);

		if (inRetVal == VS_ERROR)
			return (VS_ERROR);
		else
		{
			/* 塞到packet中*/
			memcpy((char *)&uszPackBuf[inCnt], szEncrypt, inRetVal);
			inCnt += (inLen / 2);
			
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, (unsigned char*)uszPackBuf, strlen((char*)uszPackBuf));
			inESC_DebugLog("  F_35 [szEncrypted  %s]", szAscii);

		}
		
	}
	

	inESC_DebugLog("  F_35 [T2DATA  %s]", szT2Data);
	inESC_DebugLog("  DataCnt[%d]",inCnt);
#endif	
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

        return (inCnt);
}


/*
Function        :inNEXSYS_ESC_Pack37
Date&Time       :
Describe        :Field_37:	Retrieval Reference Number (RRN)
*/
int inNEXSYS_ESC_Pack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int 	inCnt = 0;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	if (strlen(pobTran->srBRec.szRefNo) > 0)
	{
		memcpy(&uszPackBuf[inCnt], pobTran->srBRec.szRefNo, 12);
	}else
		memcpy((char *) &uszPackBuf[inCnt], "            ", 12);
	inCnt += 12;

	
	inESC_DebugLog("  F_37 [RRN %s]", pobTran->srBRec.szRefNo);
	inESC_DebugLog("  DataCnt[%d]",inCnt);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
        return (inCnt);
}

/*
Function        :inNEXSYS_ESC_Pack38
Date&Time       :
Describe        :Field_38:	Authorization Identification Response（SmartPay交易無授權碼）
*/
int inNEXSYS_ESC_Pack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int 	inCnt = 0;
	char  szAuthCode[_AUTH_CODE_SIZE_ + 1];
	
	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 確認是否授權碼是合法字元 */
//	if (inFunc_Check_AuthCode_Validate(pobTran->srBRec.szAuthCode) != VS_SUCCESS)
//	{
//		inDISP_Msg_BMP(_ERR_AUTHCODE_, _COORDINATE_Y_LINE_8_6_, _NO_KEY_MSG_, 2, "", 0);
//		return (VS_ERROR);
//	}

	/* 授權碼不能全0或全空白，除了CUP */
	if ((!memcmp(pobTran->srBRec.szAuthCode, "000000", 6) || 
		!memcmp(pobTran->srBRec.szAuthCode, "      ", 6)) && pobTran->srBRec.uszCUPTransBit != VS_TRUE)
	{
			return (VS_ERROR);
	}

	memset(szAuthCode, 0x00, sizeof(szAuthCode));
	strcpy(szAuthCode, pobTran->srBRec.szAuthCode);
	/* 若沒輸入滿6個字元，則右邊用空白填滿*/
	inFunc_PAD_ASCII(szAuthCode, szAuthCode, ' ', 6, _PAD_LEFT_FILL_RIGHT_);

	memcpy((char *) &uszPackBuf[inCnt], &szAuthCode[0], _AUTH_CODE_SIZE_);
	inCnt += _AUTH_CODE_SIZE_;

	
	inESC_DebugLog("  F_38 [AUTH CODE %s]", szAuthCode);
	inESC_DebugLog("  DataCnt[%d]",inCnt);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
        return (inCnt);
}

/*
Function        :inNEXSYS_ESC_Pack41
Date&Time       :
Describe        :Field_41:	Card Acceptor Terminal Identification（TID）補充說明：端末機代號需可支援英、數字。
*/
int inNEXSYS_ESC_Pack41(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int 	inCnt = 0;
	char 	szTemplate[8 + 1]; /* szTemplate for TID */
	
	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* For ESC電子簽單，上傳流程要使用ESC Host */
	inNEXSYS_ESC_SwitchToESC_Host(pobTran->srBRec.inHDTIndex);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (inGetTerminalID(szTemplate) == VS_ERROR)
                return (VS_ERROR);

	/* 回覆原host */
	if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie("  inNEXSYS_ESC_Pack41 Recover HDT[%d] *Error* Line[%d]",pobTran->srBRec.inHDTIndex, __LINE__);
	}

	memcpy((char *) &uszPackBuf[inCnt], szTemplate, strlen(szTemplate));
	inCnt += strlen(szTemplate);
	
	
	inESC_DebugLog("  F_41 [TID %s]", szTemplate);
	inESC_DebugLog("  DataCnt[%d]",inCnt);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

        return (inCnt);
}

/*
Function        :inNEXSYS_ESC_Pack42
Date&Time       :
Describe        :Field_42:	Card Acceptor Identification Code(MID)
 		補充說明：
		1. 商店代號需左靠右補空白。
		2. 語音開卡交易須使用固定商店代號 0122900410 。
*/
int inNEXSYS_ESC_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int 	inCnt = 0;
	char 	szTemplate[16 + 1]; /* szTemplate for MID */

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* For ESC電子簽單，上傳流程要使用ESC Host */
	inNEXSYS_ESC_SwitchToESC_Host(pobTran->srBRec.inHDTIndex);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (inGetMerchantID(szTemplate) == VS_ERROR)
		return (VS_ERROR);

	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PAD_LEFT_FILL_RIGHT_);

	/* 回覆原host */
	if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie("  inNEXSYS_ESC_Pack42 Recover HDT[%d] *Error* Line[%d]",pobTran->srBRec.inHDTIndex, __LINE__);
	}

	memcpy((char *) &uszPackBuf[inCnt], szTemplate, strlen(szTemplate));
	inCnt += strlen(szTemplate);


	inESC_DebugLog("  F_42 [MID %s]", szTemplate);
	inESC_DebugLog("  DataCnt[%d]",inCnt);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (inCnt);
}

/*
Function        :inNEXSYS_ESC_Pack48
Date&Time       :
Describe        :Field_48:	Additional Data - Private Use
*/
int inNEXSYS_ESC_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int	inCnt = 0;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* Length */
	uszPackBuf[inCnt ++] = 0x00;
	uszPackBuf[inCnt ++] = 0x19;
	/* Table ID */
	uszPackBuf[inCnt ++] = 0x12;
	/* Value */
	memcpy((char *)&uszPackBuf[inCnt], pobTran->srBRec.szStoreID, 20);
	inCnt += 18;

	inESC_DebugLog("  F_48 [StoreID 001912%s]", pobTran->srBRec.szStoreID);
	inESC_DebugLog("  DataCnt[%d]",inCnt);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (inCnt);
}

int inNEXSYS_ESC_Pack54(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int	inCnt = 0;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	uszPackBuf[inCnt++] = 0x00;
	uszPackBuf[inCnt++] = 0x12;
	sprintf((char *)&uszPackBuf[inCnt], "%010ld00", pobTran->srBRec.lnTipTxnAmount);
	inCnt += 12;

	inESC_DebugLog("  DataCnt[%d]",inCnt);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inCnt);
}


/*
Function        :inNEXSYS_ESC_Pack57_tSAM
Date&Time       :
Describe        :Field_57:	T-SAM Decryption Relation Data
		加密之交易，EDC需送Filed_57，Decryption Server藉此判斷為加密交易。
		前8 bytes(64 bits)為BitMap，將加密之欄位Bit On起來；
		另每個加密之欄位資料需有相對應4 bytes的檢查碼，依序置放於Field_57 BitMap之後。

		範例：
		Field_35 加密，則資料內容應為：
		00 16 XX XX XX XX XX XX XX XX YY YY YY YY ZZ ZZ ZZ ZZ
		其中1~2 bytes為欄位長度，3~10 bytes為BitMap，11~14 bytes為Field_35之檢查碼，15~18 bytes為Field_55之檢查碼。	
*/
int inNEXSYS_ESC_Pack57_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inCnt = 0, inPacketCnt = 0;
	char		szASCII[4 + 1];
	char		szPacket[100 + 1];
	char		szPacket_Ascii[100 + 1];
	unsigned char	uszBCD[2 + 1];
	
	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	
	/* BitMap 8 Byte */
	memset(szPacket, 0x00, sizeof(szPacket));
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x20;
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x20;
	
	memcpy((char *)&szPacket[inPacketCnt], (char *)&guszNEX_tSAMCheckSum_ESC35[0], 4);
	inPacketCnt += 4;

	/* 有重覆，要看一下怎麼寫　*/
//	if (NEX_ESC_UPLOAD_DATA[ginEscUploadIndex].inTableIndex == _NEXSYS_TABLE_E1_)
//	        memcpy(&szPacket[inPacketCnt], guszNEX_tSAMCheckSum_ESC57_E1, 4);
//	else
//	        memcpy(&szPacket[inPacketCnt], guszNEX_tSAMCheckSum_ESC57_E2, 4);

	inPacketCnt += 4;

	/* Packet Data Length */
	memset(szASCII, 0x00, sizeof(szASCII));
	sprintf(szASCII, "%04d", inPacketCnt);
	memset(uszBCD, 0x00, sizeof(uszBCD));
	inFunc_ASCII_to_BCD(uszBCD, szASCII, 2);
	memcpy((char *)&uszPackBuf[inCnt], &uszBCD[0], 2);
	inCnt += 2;
	
	/* Packet Data */
	memcpy((char *)&uszPackBuf[inCnt], &szPacket[0], inPacketCnt);
	inCnt += inPacketCnt;
		
	inESC_DebugLog("  F_57 [CheckSum] Len[%s] Data[%s]", szASCII, szPacket_Ascii);
	inESC_DebugLog("  DataCnt[%d]",inCnt);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
        return (inCnt);
}

/*
Function        :inNEXSYS_ESC_Pack59
Date&Time       :2015/12/3 上午 11:00
Describe        : 
*/

int inNEXSYS_ESC_Pack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int 	inCnt = 0, inPacketCnt = 0;
	char	szASCII[4 + 1], szBCD[2 + 1];
	unsigned char  szPacket[2048 + 1];

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	inESC_DebugLog("  IosTxnCode [%d] ",pobTran->inISOTxnCode );
	switch (pobTran->inISOTxnCode)
	{
		case _ESC_UPLOAD_ :
			
			inPacketCnt = inESC_PackTableE1(pobTran, &szPacket[inPacketCnt], inPacketCnt, pobTran->inISOTxnCode);

			inPacketCnt = inESC_PackTableE2(pobTran, &szPacket[inPacketCnt], inPacketCnt, pobTran->inISOTxnCode);
			
			inPacketCnt = inESC_PackTableR1(pobTran, &szPacket[inPacketCnt], inPacketCnt, pobTran->inISOTxnCode);

			inPacketCnt = inESC_PackTableR2(pobTran, &szPacket[inPacketCnt], inPacketCnt, pobTran->inISOTxnCode);
			
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
				inPacketCnt = inESC_PackTableR3(pobTran, &szPacket[inPacketCnt], inPacketCnt, pobTran->inISOTxnCode);
		
			if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
				inPacketCnt = inESC_PackTableR4(pobTran, &szPacket[inPacketCnt], inPacketCnt, pobTran->inISOTxnCode);
		
			if (strlen(pobTran->srBRec.szCardHolder)> 0) 
				inPacketCnt = inESC_PackTableR5(pobTran, &szPacket[inPacketCnt], inPacketCnt, pobTran->inISOTxnCode);
//			inPacketCnt = inESC_PackTableE3(pobTran, &szPacket[inPacketCnt], inPacketCnt, pobTran->inISOTxnCode);

//			inPacketCnt = inESC_PackTableE5(pobTran, &szPacket[inPacketCnt], inPacketCnt, pobTran->inISOTxnCode);
//
//			inPacketCnt = inESC_PackTableE6(pobTran, &szPacket[inPacketCnt], inPacketCnt, pobTran->inISOTxnCode);
//
//			inPacketCnt = inESC_PackTableE7(pobTran, &szPacket[inPacketCnt], inPacketCnt, pobTran->inISOTxnCode);

//			inPacketCnt = inESC_PackTableE9(pobTran, &szPacket[inPacketCnt], inPacketCnt, pobTran->inISOTxnCode);

			break;
		default:
			
			break;
	}
	
	/* Packet Data Length */
	memset(szASCII, 0x00, sizeof(szASCII));
	sprintf(szASCII, "%04d", inPacketCnt);
	memset(szBCD, 0x00, sizeof(szBCD));
	inFunc_ASCII_to_BCD((unsigned char*)szBCD,szASCII, 2);
	memcpy((char *)&uszPackBuf[inCnt], &szBCD[0], 2);
	inCnt += 2;

	/* Packet Data */
	memcpy((char *)&uszPackBuf[inCnt], &szPacket[0], inPacketCnt);
	inCnt += inPacketCnt;
	
	inESC_DebugLog("  FileDataCnt[%d]", inPacketCnt);
	inESC_DebugLog("  TotalDataCnt[%d]", inCnt);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

        return (inCnt);
}

/*
Function        :inNEXSYS_ESC_Pack60
Date&Time       :
Describe        :Field_60:	Reserved-Private Data
		Record of Count must be Unique in the batch.(Batch Number)
*/
int inNEXSYS_ESC_Pack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	
	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	uszPackBuf[inCnt++] = 0x00;
	uszPackBuf[inCnt++] = 0x06;
	sprintf((char *) &uszPackBuf[inCnt], "%06ld", pobTran->srBRec.lnBatchNum);
	inCnt += 6;
	
	inESC_DebugLog("  F_60 [BatchNumber %06ld]", pobTran->srBRec.lnBatchNum);
	inESC_DebugLog("  DataCnt[%d]",inCnt);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

        return (inCnt);
}

/*
Function        :inNEXSYS_ESC_Pack62
Date&Time       :
Describe        :Field_62:	Reserved-Private Data(Invoice Number)
*/
int inNEXSYS_ESC_Pack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int 	inCnt = 0;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	inESC_DebugLog("  IosTxnCode [%d] ",pobTran->inISOTxnCode );

	uszPackBuf[inCnt++] = 0x00;
	uszPackBuf[inCnt++] = 0x06;
	sprintf((char *) &uszPackBuf[inCnt], "%06ld", pobTran->srBRec.lnOrgInvNum);
	inCnt += 6;

	inESC_DebugLog("  OrgInv[%d]", pobTran->srBRec.lnOrgInvNum);
	inESC_DebugLog("  DataCnt[%d]", inCnt);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (inCnt);
}


int inNEXSYS_ESC_Pack63(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int	inCnt = 0;
	int	inPacketCnt = 0;
	char	szASCII[4 + 1], szBCD[2 + 1];
	char	szTemplate[100], szPacket[100 + 1];
	
	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	inESC_DebugLog("  IosTxnCode [%d] ",pobTran->inISOTxnCode );
	
	memset(szPacket, 0x00, sizeof(szPacket));
	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		/* Indicator (1 Byte) */
		memcpy(&szPacket[0], &pobTran->srBRec.szInstallmentIndicator[0], 1);
		inPacketCnt ++;
		/* (Goods ID/Product code)*/
		sprintf(&szPacket[1], "%06lu",pobTran->srBRec.lnFubonGoodsID);
		inPacketCnt +=6;

		memset(szTemplate, 0x00, sizeof(szTemplate));

		sprintf(szTemplate, "%02lu00%08lu%08lu%08lu",
					pobTran->srBRec.lnInstallmentPeriod,
					pobTran->srBRec.lnInstallmentDownPayment + pobTran->srBRec.lnTipTxnAmount,  /* 2012-05-18 PM 03:26:47 add by kakab �������B���[�p�O */
					pobTran->srBRec.lnInstallmentPayment,
					pobTran->srBRec.lnInstallmentFormalityFee);
		strcat(szPacket, szTemplate);
		inPacketCnt += 28;
	} else if(pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
           
		/* Indicator (1 Byte) */
		memcpy(&szPacket[0], &pobTran->srBRec.szRedeemIndicator[0], 1);
		inPacketCnt ++;

		/* (Goods ID/Product code)*/
		sprintf(&szPacket[1], "%06lu",pobTran->srBRec.lnFubonGoodsID);
		inPacketCnt +=6;

				

		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "00%08lu%08lu%08lu%08lu",
					pobTran->srBRec.lnRedemptionPoints,
					pobTran->srBRec.lnRedemptionPointsBalance,
					pobTran->srBRec.lnRedemptionPaidAmount,
					(pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));

		strcat(szPacket, szTemplate);
		inPacketCnt += 34;
	}
	
	
	/* Packet Data Length */
	memset(szASCII, 0x00, sizeof(szASCII));
	sprintf(szASCII, "%04d", inPacketCnt);
	memset(szBCD, 0x00, sizeof(szBCD));
	inFunc_ASCII_to_BCD((unsigned char *)szBCD, szASCII, 2);
	memcpy((char *)&uszPackBuf[inCnt], &szBCD[0], 2);
	inCnt += 2;
	/* Packet Data */
	memcpy((char *)&uszPackBuf[inCnt], &szPacket[0], inPacketCnt);
	inCnt += inPacketCnt;
	
	inESC_DebugLog("  DataCnt[%d]", inCnt);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inCnt);

}


/*
Function        :inNEXSYS_ESC_Check03
Date&Time       :
Describe        :
*/
int inNEXSYS_ESC_Check03(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
	char szDebugTemp[18];
	memset(szDebugTemp,0x00,sizeof(szDebugTemp));
	memcpy(szDebugTemp, uszSendPacket, 3);
	inESC_DisplayData(szDebugTemp,3);
	
	memset(szDebugTemp,0x00,sizeof(szDebugTemp));
	memcpy(szDebugTemp, uszRecePacket, 3);
	inESC_DisplayData(szDebugTemp,3);
	
        if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 3))
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inNEXSYS_ESC_Check04
Date&Time       :
Describe        :
*/
int inNEXSYS_ESC_Check04(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
	char szDebugTemp[18];
	memset(szDebugTemp,0x00,sizeof(szDebugTemp));
	memcpy(szDebugTemp, uszSendPacket, 6);
	inESC_DisplayData(szDebugTemp,6);
	
	memset(szDebugTemp,0x00,sizeof(szDebugTemp));
	memcpy(szDebugTemp, uszRecePacket, 6);
	inESC_DisplayData(szDebugTemp,6);
	
        if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 6))
        {
                //vdSGErrorMessage(NEXSYS_CHECK_ISO_FILED04_ERROR); /* 140 = 電文錯誤請重試 */
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inNEXSYS_ESC_Check41
Date&Time       :
Describe        :
*/
int inNEXSYS_ESC_Check41(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
	char szDebugTemp[18];
	memset(szDebugTemp,0x00,sizeof(szDebugTemp));
	memcpy(szDebugTemp, uszSendPacket, 8);
	inESC_DisplayData(szDebugTemp,8);
	
	memset(szDebugTemp,0x00,sizeof(szDebugTemp));
	memcpy(szDebugTemp, uszRecePacket, 8);
	inESC_DisplayData(szDebugTemp,8);
	
        if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 8))
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inNEXSYS_ESC_UnPack24
Date&Time       :
Describe        : 1. NPS 需求要判斷 Field_24 第一個 Byte 是否為【9】
		2. 此 Flag 是由主機做控管，端末機不管例外處理
*/
int inNEXSYS_ESC_UnPack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        char	szNPS[8];

	memset(szNPS, 0x00, sizeof(szNPS));
	inFunc_BCD_to_ASCII(&szNPS[0], uszUnPackBuf, 2);
	if (szNPS[0] == 0x39)
		pobTran->srBRec.uszField24NPSBit = VS_TRUE;
	
        return (VS_SUCCESS);
}

int inNEXSYS_ESC_UnPack32(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{

	return (VS_SUCCESS);
}

/*
Function        :inNEXSYS_ESC_UnPack38
Date&Time       :
Describe        :
*/
int inNEXSYS_ESC_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        if (pobTran->inISOTxnCode == _REVERSAL_ || pobTran->inISOTxnCode == _CUP_LOGON_)
                return (VS_SUCCESS);
	else
	{
		memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
		memcpy(&pobTran->srBRec.szAuthCode[0], (char *) &uszUnPackBuf[0], _AUTH_CODE_SIZE_);
	}

        return (VS_SUCCESS);
}

/*
Function        :inNEXSYS_ESC_UnPack39
Date&Time       :
Describe        :Response code
*/
int inNEXSYS_ESC_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{	
	char	szTemplate[6 + 1];

	memset(&szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], &guszNEXSYS_ESC_ISO_Field03[0], 3);

	memcpy(&pobTran->srBRec.szRespCode[0], (char *)&uszUnPackBuf[0], 2);

        /* 2011-05-24 PM 05:45:19 銀聯一般交易 01不能被改成05 */
	if (pobTran->inISOTxnCode != _SALE_ &&
	    pobTran->inISOTxnCode != _CUP_SALE_ &&
	    ((!memcmp(&pobTran->srBRec.szRespCode[0], "01", 2)) || (!memcmp(&pobTran->srBRec.szRespCode[0], "02", 2))))
	{
		memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
		strcpy(pobTran->srBRec.szRespCode, "05");
	}
	else if (pobTran->inISOTxnCode == _CLS_BATCH_ &&
		 !memcmp(&pobTran->srBRec.szRespCode[0], "95", 2) &&
		 !memcmp(&szTemplate[0], "960000", 6))
	{
	        	inSaveHDTRec(0);

	}

	pobTran->srBRec.szRespCode[2] = 0x00;
	
        return (VS_SUCCESS);
}

