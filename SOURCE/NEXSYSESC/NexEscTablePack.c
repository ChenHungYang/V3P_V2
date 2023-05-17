#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <ctosapi.h>
#include <sqlite3.h>

#include "../../SOURCE/INCLUDES/Define_1.h"
#include "../../SOURCE/INCLUDES/Define_2.h"
#include "../../SOURCE/INCLUDES/Transaction.h"
#include "../../SOURCE/INCLUDES/TransType.h"

#include "../../SOURCE/PRINT/Print.h"
#include "../../SOURCE/FUNCTION/Sqlite.h"
#include "../../SOURCE/FUNCTION/HDPT.h"
#include "../../SOURCE/FUNCTION/HDT.h"
#include "../../SOURCE/FUNCTION/CDT.h"
#include "../../SOURCE/FUNCTION/Function.h"
#include "../../SOURCE/FUNCTION/Card.h"

#include "../../SOURCE/DISPLAY/Display.h"

#include "NexsysEsc.h"
#include "NexEscTableFn.h"
#include "NexEscTablePack.h"
#include "NexsysEscIso.h"

extern int	ginNEX_EscUploadIndex;	/* 上傳中使用 */
extern NEX_ESC_DATA	 NEX_ESC_UPLOAD_DATA[_ESC_LIMIT_];	/* 總上傳資料 */


int inDispAsciiData(char* szDispStr, unsigned char * szAnlisys, int inProcessLen)
{
	char *szCopyTemp;

	szCopyTemp = malloc(sizeof(char) * (inProcessLen + 1));

	memset(szCopyTemp, 0x00, (inProcessLen + 1));

	memcpy(&szCopyTemp[0], szAnlisys, inProcessLen);
	inESC_DebugLog("  %s[%d] [%s]", szDispStr, inProcessLen, szCopyTemp);

	free(szCopyTemp);
	return 0;
}

int inDispHexData(char* szDispStr, unsigned char * szAnlisys, int inProcessLen)
{
	char *szCopyTemp;

	szCopyTemp = malloc(sizeof(char) * ((inProcessLen *2)  + 1));

	memset(szCopyTemp,0x00, (inProcessLen*2) + 1);

	inFunc_BCD_to_ASCII(szCopyTemp, szAnlisys, inProcessLen);

	if(inProcessLen*2 > 4500)
	{
		inESC_DebugLog("  %s[%d] [%s]", szDispStr, 4500, szCopyTemp);
		inESC_DebugLog("  %s[%d] [%s]", szDispStr, (inProcessLen- 4500), &szCopyTemp[4500]);

	}else{
		inESC_DebugLog("  %s[%d] [%s]", szDispStr, inProcessLen, szCopyTemp);
	}
	free(szCopyTemp);


	return 0;
}

int inESC_PackTableE1(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int inPackAddr, int inIncode)
{
	unsigned char 	uszBCD[2 + 1];
	char szTemplate[128];
	char szLRC;
	int inPackTemp = 0, inLRCDataLen, inCount;
	int inSubPackLenAddr;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	//TAG E1
	//sub name
	memcpy(&uszPackBuf[inPackTemp], "E1", 2);
	inPackTemp += 2;
	inSubPackLenAddr = inPackTemp;

	//SubDatalen
	memcpy(&uszPackBuf[inPackTemp], "00", 2);
	inPackTemp += 2;


	/* For ESC電子簽單，上傳流程要使用ESC Host */
	/* 1-1 Terminal id */
	inNEXSYS_ESC_SwitchToESC_Host(pobTran->srBRec.inHDTIndex);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (inGetTerminalID(szTemplate) == VS_ERROR)
                return (VS_ERROR);

	/* 回覆原host */
	if( inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" inESC_PackTableE1 Load HDT[%d] *Error* Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
	}

	memcpy((char *) &uszPackBuf[inPackTemp], szTemplate, strlen(szTemplate));
	inPackTemp += strlen(szTemplate);

	/* 1-2. Batch Number */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%06ld", pobTran->srBRec.lnBatchNum);
	memcpy((char *) &uszPackBuf[inPackTemp], &szTemplate[3], 3);
	inPackTemp += 3;

	/* 1-3. Invoice Number */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
	memcpy((char *) &uszPackBuf[inPackTemp], &szTemplate[3], 3);
	inPackTemp += 3;

	/* 1-4. YYYYMMDDHHmmss */

	/* YYYYMMDD */
	memcpy((char *) &uszPackBuf[inPackTemp], pobTran->srBRec.szEscDate, 8);
	inPackTemp += 8;
	inESC_DebugLog("  E1 Time [%s] ",pobTran->srBRec.szEscDate);

	/* HHMMSS */
	memcpy((char *) &uszPackBuf[inPackTemp], pobTran->srBRec.szEscTime, 6);
	inPackTemp += 6;
	inESC_DebugLog("  E1 Time [%s] ",pobTran->srBRec.szEscTime);

	/* 2. 電子簽名檔封包總數 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%02d", NEX_ESC_UPLOAD_DATA[ginNEX_EscUploadIndex].inTotalPacketCnt);

	memcpy(&uszPackBuf[inPackTemp], &szTemplate[0], 2);
	inPackTemp += 2;

	/* 2. 電子簽名檔目前傳送第幾個封包 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%02d", NEX_ESC_UPLOAD_DATA[ginNEX_EscUploadIndex].inPacketCnt);
	memcpy(&uszPackBuf[inPackTemp], &szTemplate[0], 2);
	inPackTemp += 2;

	inESC_DebugLog("  E1 PackCnt [%d]", NEX_ESC_UPLOAD_DATA[ginNEX_EscUploadIndex].inPacketCnt);

	/* ３. 電子簽名檔完整封包總長度 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%05ld", NEX_ESC_UPLOAD_DATA[ginNEX_EscUploadIndex].lnTotalPacketSize);
	memcpy(&uszPackBuf[inPackTemp], &szTemplate[0], 5);
	inPackTemp += 5;

	/* ４. 本電子簽名檔封包傳送長度 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%04d", NEX_ESC_UPLOAD_DATA[ginNEX_EscUploadIndex].inPacketSize);
	memcpy(&uszPackBuf[inPackTemp], &szTemplate[0], 4);
	inPackTemp += 4;

	/* 5. 電子簽名檔內容檢查碼 */
	inLRCDataLen = NEX_ESC_UPLOAD_DATA[ginNEX_EscUploadIndex].inPacketSize;
	memset(&szLRC, 0x00, 1);

	szLRC = NEX_ESC_UPLOAD_DATA[ginNEX_EscUploadIndex].szPackData[0];

	for (inCount = 0; inCount < (inLRCDataLen -1 ); inCount++)
	{
		szLRC = szLRC ^ NEX_ESC_UPLOAD_DATA[ginNEX_EscUploadIndex].szPackData[inCount+1];
	}

	uszPackBuf[inPackTemp] = szLRC;
	inPackTemp ++;
	inESC_DebugLog("  LRC = %2x", szLRC);

	/* 減去 tag 及 sublen 長度 */
	memset(szTemplate,0x00,sizeof(szTemplate));
	sprintf(&szTemplate[0],"%04x",inPackTemp - 2 - 2);
	inESC_DebugLog("  PackDataLen[%s]",szTemplate);

	inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 2);
	inESC_DebugLog("  BCD Len[%x][%x]",uszBCD[0], uszBCD[1]);

	memcpy(&uszPackBuf[inSubPackLenAddr], &uszBCD[0], 2);

	inESC_DebugLog("  DataCnt[%d]",inPackTemp);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

        return (inPackTemp + inPackAddr);
}


int inESC_PackTableE2(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int inPackAddr, int inIncode)
{
	unsigned char 	uszBCD[2 + 1];
	char szTemplate[42 + 1];
	int	inPackTemp = 0;
	int	inSubPackLenAddr ;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	inESC_DebugLog("  ginEscUpIndex [%d]",ginNEX_EscUploadIndex);
	inESC_DebugLog("  ESC Table Id [%d] ",NEX_ESC_UPLOAD_DATA[ginNEX_EscUploadIndex].inTableIndex);

	//TAG E1
	//sub name
	memcpy(&uszPackBuf[inPackTemp], "E2", 2);
	inPackTemp += 2;
	inSubPackLenAddr = inPackTemp;

	//SubDatalen
	memcpy(&uszPackBuf[inPackTemp], "00", 2);
	inPackTemp += 2;

	/* For ESC電子簽單，上傳流程 */
	if (NEX_ESC_UPLOAD_DATA[ginNEX_EscUploadIndex].inTableIndex == _NEXSYS_TABLE_E2_)
	{
		/* [DEBUG] */
//		inESC_DebugLog("  Pack ESC SIZE  [%d] ",NEX_ESC_UPLOAD_DATA[ginNEX_EscUploadIndex].inPacketSize);
//		inESC_DebugLog("  ISO ESC DATA DISP START----- ");
//		inESC_DisplayData(&NEX_ESC_UPLOAD_DATA[ginNEX_EscUploadIndex].szPackData[0], NEX_ESC_UPLOAD_DATA[ginNEX_EscUploadIndex].inPacketSize);
//		inESC_DebugLog("  ISO ESC DATA DISP END----- ");

		/* 封包內容 */
		memcpy(&uszPackBuf[inPackTemp], &NEX_ESC_UPLOAD_DATA[ginNEX_EscUploadIndex].szPackData[0], NEX_ESC_UPLOAD_DATA[ginNEX_EscUploadIndex].inPacketSize);
		inPackTemp += NEX_ESC_UPLOAD_DATA[ginNEX_EscUploadIndex].inPacketSize;

	}

	/* Packet SubData Length */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(&szTemplate[0],"%04x",(inPackTemp - 2 - 2));
	inESC_DebugLog("  PackDataLen[%s]",szTemplate);

	memset(uszBCD, 0x00, sizeof(uszBCD));
	inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 2);
	inESC_DebugLog("  BCD Len[%x][%x]",uszBCD[0], uszBCD[1]);

	memcpy(&uszPackBuf[inSubPackLenAddr], &uszBCD[0], 2);

	inESC_DebugLog("-----[%s][%s] Cnt[%d] END -----",__FILE__, __FUNCTION__, inPackTemp);
	return (inPackTemp + inPackAddr);
}



int inESC_PackTableR1(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int inPackAddr, int inIncode)
{
	unsigned char 	uszBCD[2 + 1];
	char szTemplate[12];
	int inPackTemp = 0;
	int inSubPackLenAddr;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	//TAG E1
	//sub name
	memcpy(&uszPackBuf[inPackTemp], "R1", 2);
	inPackTemp += 2;
	inSubPackLenAddr = inPackTemp;

	//SubDatalen
	memcpy(&uszPackBuf[inPackTemp], "00", 2);
	inPackTemp += 2;

	/*  卡別 */
	if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
		memcpy(&uszPackBuf[inPackTemp], "02", 2);
	else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)))
		memcpy(&uszPackBuf[inPackTemp], "01", 2);
	else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)))
		memcpy(&uszPackBuf[inPackTemp], "03", 2);
	else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_)))
		memcpy(&uszPackBuf[inPackTemp], "04", 2);
	else
		memcpy(&uszPackBuf[inPackTemp], "99", 2);
	inPackTemp += 2;

	/*  交易模式 */
	if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
	{
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
			memcpy(&uszPackBuf[inPackTemp], "C", 1);
//		else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
//			memcpy(&uszPackBuf[inPackTemp], "T", 1);
		else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			memcpy(&uszPackBuf[inPackTemp], "W", 1);
		else
		{
			if (pobTran->srBRec.uszManualBit == VS_TRUE)
			{
				/* 電文轉Manual Keyin但是簽單要印感應的W */
				if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
					memcpy(&uszPackBuf[inPackTemp], "W", 1);
				else
					memcpy(&uszPackBuf[inPackTemp], "M", 1);
			}
			else if (pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
				memcpy(&uszPackBuf[inPackTemp], "F", 1);
			else
				memcpy(&uszPackBuf[inPackTemp], "S", 1);
		}
	}
	else
	{
		if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			memcpy(&uszPackBuf[inPackTemp], "W", 1);
		else
			memcpy(&uszPackBuf[inPackTemp], "C", 1);
	}
	inPackTemp += 1;

	/*  交易類別 */
	vdESC_PackTransType(pobTran, (char *)&uszPackBuf[inPackTemp]);
	inPackTemp += 2;

	/*  交易西元年 */
	memcpy((char *) &uszPackBuf[inPackTemp], pobTran->srBRec.szDate, 4);
	inPackTemp += 4;


	/*  免簽名註記 */
	/* 原來是用 uszNoSignatureBit 但在富邦時改為  uszF56NoSignatureBit 來判斷 */
	if (pobTran->srBRec.uszF56NoSignatureBit == VS_TRUE )
		memcpy(&uszPackBuf[inPackTemp], "1", 1);
	else
		memcpy(&uszPackBuf[inPackTemp], "0", 1);
	inPackTemp += 1;

	/* 減去 tag 及 sublen 長度 */
	memset(szTemplate,0x00,sizeof(szTemplate));
	sprintf(&szTemplate[0],"%04x",inPackTemp - 2 - 2);
	inESC_DebugLog("  PackDataLen[%s]",szTemplate);

	inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 2);
	inESC_DebugLog("  BCD Len[%x][%x]",uszBCD[0], uszBCD[1]);

	memcpy(&uszPackBuf[inSubPackLenAddr], &uszBCD[0], 2);

	inESC_DebugLog("  DataCnt[%d]",inPackTemp);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

        return (inPackTemp + inPackAddr);
}

int inESC_PackTableR2(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int inPackAddr, int inIncode)
{
	unsigned char 	uszBCD[2 + 1];
	char szTemplate[128];
	int inPackTemp = 0;
	int inSubPackLenAddr;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	//TAG E1
	//sub name
	memcpy(&uszPackBuf[inPackTemp], "R2", 2);
	inPackTemp += 2;
	inSubPackLenAddr = inPackTemp;

	//SubDatalen
	memcpy(&uszPackBuf[inPackTemp], "00", 2);
	inPackTemp += 2;

	memset(szTemplate, 0x00, sizeof(szTemplate));
	inCARD_ExpDateEncryptAndDecrypt(pobTran, szTemplate, szTemplate, _EXP_ENCRYPT_);

	inESC_DebugLog("   Enc Data Len [%d]",strlen(szTemplate));

	if(strlen(szTemplate) <= 0)
	{
		inESC_DebugLog("   Enc Data Len 0");
		memcpy(&uszPackBuf[inPackTemp], "      ", 6);
		inPackTemp += 6;
	}else
	{
		memcpy(&uszPackBuf[inPackTemp], szTemplate, strlen(szTemplate));
		inPackTemp += strlen(szTemplate);

		inESC_DebugLog("   Enc Data [%s]", szTemplate);

		if(strlen(szTemplate) < 6 )
		{
			memcpy(&uszPackBuf[inPackTemp], "      ", (6 - strlen(szTemplate)));
			inPackTemp += (6 - strlen(szTemplate));
		}
	}


	/* 減去 tag 及 sublen 長度 */
	memset(szTemplate,0x00,sizeof(szTemplate));
	sprintf(&szTemplate[0],"%04x",inPackTemp - 2 - 2);
	inESC_DebugLog("  PackDataLen[%s]",szTemplate);

	inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 2);
	inESC_DebugLog("  BCD Len[%x][%x]",uszBCD[0], uszBCD[1]);

	memcpy(&uszPackBuf[inSubPackLenAddr], &uszBCD[0], 2);

	inESC_DebugLog("  DataCnt[%d]",inPackTemp);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

        return (inPackTemp + inPackAddr);
}


int inESC_PackTableR3(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int inPackAddr, int inIncode)
{
	unsigned char 	uszBCD[2 + 1];
	char szTemplate[128];
	int inPackTemp = 0;
	int inSubPackLenAddr;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	//TAG E1
	//sub name
	memcpy(&uszPackBuf[inPackTemp], "R3", 2);
	inPackTemp += 2;
	inSubPackLenAddr = inPackTemp;

	//SubDatalen
	memcpy(&uszPackBuf[inPackTemp], "00", 2);
	inPackTemp += 2;


	memcpy(&uszPackBuf[inPackTemp], pobTran->srBRec.szCUP_TN, strlen(pobTran->srBRec.szCUP_TN));
	inPackTemp += strlen(pobTran->srBRec.szCUP_TN);

	inESC_DebugLog("   CUP STAN[%s]", pobTran->srBRec.szCUP_TN);

	/* 減去 tag 及 sublen 長度 */
	memset(szTemplate,0x00,sizeof(szTemplate));
	sprintf(&szTemplate[0],"%04x",inPackTemp - 2 - 2);
	inESC_DebugLog("  PackDataLen[%s]",szTemplate);

	inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 2);
	inESC_DebugLog("  BCD Len[%x][%x]",uszBCD[0], uszBCD[1]);

	memcpy(&uszPackBuf[inSubPackLenAddr], &uszBCD[0], 2);

	inESC_DebugLog("  DataCnt[%d]",inPackTemp);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

        return (inPackTemp + inPackAddr);
}

int inESC_PackTableR4(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int inPackAddr, int inIncode)
{
	unsigned char 	uszBCD[2 + 1];
	char szTemplate[128];
	int inPackTemp = 0;
	int inSubPackLenAddr;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	//TAG E1
	//sub name
	memcpy(&uszPackBuf[inPackTemp], "R4", 2);
	inPackTemp += 2;
	inSubPackLenAddr = inPackTemp;

	//SubDatalen
	memcpy(&uszPackBuf[inPackTemp], "00", 2);
	inPackTemp += 2;


	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%02X%02X%02X%02X%02X%02X%02X%02X",
		pobTran->srEMVRec.usz9F26_ApplCryptogram[0],
		pobTran->srEMVRec.usz9F26_ApplCryptogram[1],
		pobTran->srEMVRec.usz9F26_ApplCryptogram[2],
		pobTran->srEMVRec.usz9F26_ApplCryptogram[3],
		pobTran->srEMVRec.usz9F26_ApplCryptogram[4],
		pobTran->srEMVRec.usz9F26_ApplCryptogram[5],
		pobTran->srEMVRec.usz9F26_ApplCryptogram[6],
		pobTran->srEMVRec.usz9F26_ApplCryptogram[7]);

	inESC_DebugLog("   R4DAte [%s] ",szTemplate);

	memcpy(&uszPackBuf[inPackTemp], szTemplate,  strlen(szTemplate));
	inPackTemp += strlen(szTemplate);

	/* 減去 tag 及 sublen 長度 */
	memset(szTemplate,0x00,sizeof(szTemplate));
	sprintf(&szTemplate[0],"%04x",inPackTemp - 2 - 2);
	inESC_DebugLog("  PackDataLen[%s]",szTemplate);
	/* 因為不要轉BCD 所以*/
	inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 2);
	inESC_DebugLog("  BCD Len[%x][%x]",uszBCD[0], uszBCD[1]);

	memcpy(&uszPackBuf[inSubPackLenAddr], &uszBCD[0], 2);

	inESC_DebugLog("  DataCnt[%d]",inPackTemp);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (inPackTemp + inPackAddr);
}

int inESC_PackTableR5(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int inPackAddr, int inIncode)
{
	unsigned char 	uszBCD[2 + 1];
	char szTemplate[128];
	int inPackTemp = 0;
	int inSubPackLenAddr;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	//TAG E1
	//sub name
	memcpy(&uszPackBuf[inPackTemp], "R5", 2);
	inPackTemp += 2;
	inSubPackLenAddr = inPackTemp;

	//SubDatalen
	memcpy(&uszPackBuf[inPackTemp], "00", 2);
	inPackTemp += 2;

	memset(szTemplate,0x00,sizeof(szTemplate));
	sprintf(szTemplate, "%s", pobTran->srBRec.szCardHolder);
	if(strlen(szTemplate) > 30 )
	{
		memcpy(&uszPackBuf[inPackTemp], szTemplate, 30);
		inPackTemp += 30;
	}else{
		memcpy(&uszPackBuf[inPackTemp], szTemplate, strlen(szTemplate));
		inPackTemp += strlen(szTemplate);
	}

	/* 減去 tag 及 sublen 長度 */
	memset(szTemplate,0x00,sizeof(szTemplate));
	sprintf(&szTemplate[0],"%04x",inPackTemp - 2 - 2);
	inESC_DebugLog("  PackDataLen[%s]",szTemplate);

	inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 2);
	inESC_DebugLog("  BCD Len[%x][%x]",uszBCD[0], uszBCD[1]);

	memcpy(&uszPackBuf[inSubPackLenAddr], &uszBCD[0], 2);

	inESC_DebugLog("  DataCnt[%d]",inPackTemp);
	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

        return (inPackTemp + inPackAddr);
}


int inESC_UnPackTableHeader(TRANSACTION_OBJECT *pobTran, unsigned char *szUnPackBuf, int inPackAddr,int inSubLen)
{
	int inUnPackCnt = 0, inPackTableId ;
	char szErrorLogTemp[64];

	if (!memcmp(&szUnPackBuf[inUnPackCnt], "\x35\x38", 2))
	{
		inPackTableId = 58;
	}else if (!memcmp(&szUnPackBuf[inUnPackCnt], "\x35\x39", 2))
	{
		inPackTableId = 59;
	}else if (!memcmp(&szUnPackBuf[inUnPackCnt], "\x36\x34", 2))
	{
		inPackTableId = 64;
	}else if (!memcmp(&szUnPackBuf[inUnPackCnt], "\x36\x35", 2))
	{
		inPackTableId = 65;
	}else if (!memcmp(&szUnPackBuf[inUnPackCnt], "\x36\x38", 2))
	{
		inPackTableId = 68;
	}else if (!memcmp(&szUnPackBuf[inUnPackCnt], "\x37\x35", 2))
	{
		inPackTableId = 75;
	}else if (!memcmp(&szUnPackBuf[inUnPackCnt], "\x37\x37", 2))
	{
		inPackTableId = 77;
	}else if (!memcmp(&szUnPackBuf[inUnPackCnt], "\x38\x34", 2))
	{
		inPackTableId = 84;
	}else if (!memcmp(&szUnPackBuf[inUnPackCnt], "\x38\x35", 2))
	{
		inPackTableId = 85;
	}else if (!memcmp(&szUnPackBuf[inUnPackCnt], "\x38\x39", 2))
	{
		inPackTableId = 89;
	}else if (!memcmp(&szUnPackBuf[inUnPackCnt], "\x39\x31", 2))
	{
		inPackTableId = 91;
	}else if (!memcmp(&szUnPackBuf[inUnPackCnt], "\x39\x30", 2))
	{
		inPackTableId = 90;
	}else if (!memcmp(&szUnPackBuf[inUnPackCnt], "\x39\x37", 2))
	{
		inPackTableId = 97;
	}else
		inPackTableId = -1;

	if(inPackTableId == -1)
	{
		memset(szErrorLogTemp,0x00,sizeof(szErrorLogTemp));
		sprintf(szErrorLogTemp,"UnPack Header [%c][%c] ", szUnPackBuf[inUnPackCnt], szUnPackBuf[inUnPackCnt+1]);
		inESC_DebugLog(szErrorLogTemp);
	}

        return inPackTableId;
}

/* Added Table 58 2017/7/13 */
int inESC_UnPackTable58(TRANSACTION_OBJECT *pobTran, unsigned char *szUnPackBuf, int inPackAddr,int inSubLen)
{
	int inUnPackCnt = 0;
	char szTemplate[32];


	if(inSubLen != _DEFAULT_TABLE_58_LENTH_)
	{
		memset(szTemplate,0x00,sizeof(szTemplate));
		sprintf(szTemplate,"UnPack58 Err [%d]", inSubLen);
		return VS_ERROR;
	}else{

		/* RPAN partial 10 */
		//memcpy(pobTran->srBRec.szRpanPartial, &szUnPackBuf[inUnPackCnt], 10);
		inUnPackCnt += 10;

		/* Wallet type 1 */
		//memcpy(pobTran->srBRec.szWalletType, &szUnPackBuf[inUnPackCnt], 1);
		inUnPackCnt += 1;

		/* Filler reserved 9 */
		inUnPackCnt += 9;
	}

	return (inUnPackCnt + inPackAddr);
}


void vdESC_PackTransType(TRANSACTION_OBJECT *pobTran, char *szPrintBuf1)
{
	if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
	{
		switch (pobTran->srBRec.inCode)
		{
			case _SALE_ :
			case _CUP_SALE_ :
				memcpy(szPrintBuf1, "01",2);
				break;
			case _SALE_OFFLINE_ :
				memcpy(szPrintBuf1, "03",2);
				break;
			case _REFUND_ :
			case _CUP_REFUND_ :
				memcpy(szPrintBuf1, "02",2);
				break;
			case _INST_SALE_ :
				memcpy(szPrintBuf1, "21",2);
				break;
			case _INST_REFUND_ :
				memcpy(szPrintBuf1, "22",2);
				break;
			case _REDEEM_SALE_ :
				memcpy(szPrintBuf1, "31",2);
				break;
			case _REDEEM_REFUND_ :
				memcpy(szPrintBuf1, "32",2);
				break;
			case _TIP_ :
				memcpy(szPrintBuf1, "04",2);
				break;
			case _ADJUST_ :
				memcpy(szPrintBuf1, "05",2);
				break;
			case _REDEEM_ADJUST_ :
				memcpy(szPrintBuf1, "33",2);
				break;
			case _INST_ADJUST_ :
				memcpy(szPrintBuf1, "23",2);
				break;
			case _PRE_AUTH_ :
			case _CUP_PRE_AUTH_ :
				memcpy(szPrintBuf1, "11",2);

				break;
			case _PRE_COMP_ :
			case _CUP_PRE_COMP_ :
				memcpy(szPrintBuf1, "12",2);
				break;
			default :
				memcpy(szPrintBuf1, "01",2);
				break;
		}
	}
	else
	{
		switch (pobTran->srBRec.inOrgCode)
		{
			case _SALE_ :
			case _CUP_SALE_ :
				memcpy(szPrintBuf1, "41",2);
				break;
			case _SALE_OFFLINE_ :
				memcpy(szPrintBuf1, "43",2);
				break;
			case _REFUND_ :
			case _CUP_REFUND_ :
				memcpy(szPrintBuf1, "42",2);
				break;
			case _INST_SALE_ :
				memcpy(szPrintBuf1, "61",2);
				break;
			case _REDEEM_SALE_ :
				memcpy(szPrintBuf1, "71",2);
				break;
			case _ADJUST_ :
				memcpy(szPrintBuf1, "41",2);
				break;
			case _REDEEM_ADJUST_ :
				memcpy(szPrintBuf1, "73",2);
				break;
			case _INST_ADJUST_ :
				memcpy(szPrintBuf1, "63",2);
				break;
			case _REDEEM_REFUND_ :
				memcpy(szPrintBuf1, "72",2);
				break;
			case _INST_REFUND_ :
				memcpy(szPrintBuf1, "62",2);
				break;
			case _CUP_PRE_AUTH_ :
			case _PRE_AUTH_ :
				memcpy(szPrintBuf1, "51",2);
				break;
			case _CUP_PRE_COMP_ :
			case _PRE_COMP_ :
				memcpy(szPrintBuf1, "52",2);
				break;
			default :
				memcpy(szPrintBuf1, "41",2);
				break;
		}
	}
}


int inESC_UuPackTableE1(unsigned char *uszPackBuf , int *inLen , int inUnPackMode)
{
	int inStartLen,inSubDataLen;
	int inAddr = 0;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inStartLen = *inLen;

	inDispHexData("  E1 Sub Len  ",  &uszPackBuf[inAddr], 2);
	inSubDataLen = ((int) uszPackBuf[inAddr] * 256) + (int) uszPackBuf[inAddr+1];
	inESC_DebugLog(" Sub Len[%x] ", inSubDataLen);
	inAddr+=2;

	/* For ESC電子簽單，上傳流程要使用ESC Host */
	/* 1-1 Terminal id */
	inDispAsciiData("  商店代號  ",  &uszPackBuf[inAddr], 8);
	inAddr+= 8;

	/* 1-2. Batch Number */
	inDispAsciiData("  批次號碼 ",  &uszPackBuf[inAddr], 3);
	inAddr+= 3;
	/* 1-3. Invoice Number */
	inDispAsciiData("  調閱編號 ",  &uszPackBuf[inAddr], 3);
	inAddr+= 3;
	/* 1-4. YYYYMMDDHHmmss */
	/* YYYYMMDD */
	inDispAsciiData("  年月日 ",  &uszPackBuf[inAddr], 8);
	inAddr+= 8;

	/* HHMMSS */
	inDispAsciiData("  時分秒 ",  &uszPackBuf[inAddr], 6);
	inAddr+= 6;

	/* 1. 電子簽名檔封包總數 */
	inDispAsciiData("  ESC Pack Total  ",  &uszPackBuf[inAddr], 2);
	inAddr+= 2;
	/* 2. 電子簽名檔目前傳送第幾個封包 */
	inDispAsciiData("  ESC Pack Cnt  ",  &uszPackBuf[inAddr], 2);
	inAddr+= 2;

	/* ３. 電子簽名檔完整封包總長度 */
	inDispAsciiData("  ESC Pack Total Len ",  &uszPackBuf[inAddr], 5);
	inAddr+= 5;

	/* ４. 本電子簽名檔封包傳送長度 */
	inDispAsciiData("  ESC UpLoad Pack Size ",  &uszPackBuf[inAddr], 4);
	inAddr+= 4;
	/* 5. 電子簽名檔內容檢查碼 */
	inDispHexData("  ESC Check Code  ",  &uszPackBuf[inAddr], 1);
	inAddr+= 1;

	*inLen += inAddr;

//	inESC_DebugLog("  Addr [%d]", inAddr);
//	inESC_DebugLog("  Start Len [%d]", inStartLen);
//	inESC_DebugLog("  DataCnt[%d]",*inLen);

	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

        return (0);
}


int inESC_UuPackTableE2(unsigned char *uszPackBuf , int *inLen , int inUnPackMode)
{
	int inStartLen, inSubDataLen;
	int inAddr = 0;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inStartLen = *inLen;

	inSubDataLen = ((int) uszPackBuf[inAddr] * 256) + (int) uszPackBuf[inAddr+1];

	inESC_DebugLog(" Sublen [%d] ",inSubDataLen );

	inDispHexData(" E2 Sub Len ",  &uszPackBuf[inAddr], 2);
	inAddr+= 2;

	inDispHexData(" ESC Picture Data",  &uszPackBuf[inAddr], inSubDataLen );
	inAddr+= inSubDataLen;

	*inLen += inAddr;

	inESC_DebugLog("  Addr [%d]", inAddr);
	inESC_DebugLog("  Start Len [%d]", inStartLen);
	inESC_DebugLog("  DataCnt[%d]",*inLen );

	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

        return  (0);
}


int inESC_UuPackTableR1(unsigned char *uszPackBuf , int *inLen , int inUnPackMode)
{
	int inStartLen,inSubDataLen;
	int inAddr = 0;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inStartLen = *inLen;

	inDispHexData("  R1 長度 ",  &uszPackBuf[inAddr], 2);
	inSubDataLen = ((int) uszPackBuf[inAddr] * 256) + (int) uszPackBuf[inAddr+1];
	inESC_DebugLog(" Sub Len[%x] ", inSubDataLen);
	inAddr+=2;

	/* 卡別  */
	inDispAsciiData("  卡別  ",  &uszPackBuf[inAddr], 2);
	inAddr+= 2;

	/* 交易模式 */
	inDispAsciiData("  交易模式  ",  &uszPackBuf[inAddr], 1);
	inAddr+= 1;

	/* 交易類別 */
	inDispAsciiData("  交易類別  ",  &uszPackBuf[inAddr], 2);
	inAddr+= 2;

	/* 交易日期西元年 */
	inDispAsciiData("  交易日期西元年  ",  &uszPackBuf[inAddr], 4);
	inAddr+= 4;

	/* 免簽名註記 */
	inDispAsciiData("  免簽名註記  ",  &uszPackBuf[inAddr], 1);
	inAddr+= 1;


	*inLen += inAddr;

	inESC_DebugLog("  Addr [%d]", inAddr);
	inESC_DebugLog("  Start Len [%d]", inStartLen);
	inESC_DebugLog("  DataCnt[%d]",*inLen);

	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

        return (0);
}

int inESC_UuPackTableR2(unsigned char *uszPackBuf , int *inLen , int inUnPackMode)
{
	int inStartLen,inSubDataLen;
	int inAddr = 0;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inStartLen = *inLen;

	inDispHexData("  R2 長度 ",  &uszPackBuf[inAddr], 2);
	inSubDataLen = ((int) uszPackBuf[inAddr] * 256) + (int) uszPackBuf[inAddr+1];
	inESC_DebugLog(" Sub Len[%x] ", inSubDataLen);
	inAddr+=2;

	/* 檢查碼  */
	inDispAsciiData("  檢查碼  ",  &uszPackBuf[inAddr], 6);
	inAddr+= 6;

	*inLen += inAddr;

	inESC_DebugLog("  Addr [%d]", inAddr);
	inESC_DebugLog("  Start Len [%d]", inStartLen);
	inESC_DebugLog("  DataCnt[%d]",*inLen);

	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

        return (0);
}

int inESC_UuPackTableR3(unsigned char *uszPackBuf , int *inLen , int inUnPackMode)
{
	int inStartLen,inSubDataLen;
	int inAddr = 0;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inStartLen = *inLen;

	inDispHexData("  R3 長度 ",  &uszPackBuf[inAddr], 2);
	inSubDataLen = ((int) uszPackBuf[inAddr] * 256) + (int) uszPackBuf[inAddr+1];
	inESC_DebugLog("  Sub Len[%x] ", inSubDataLen);
	inAddr+=2;

	/* CUP交易序號  */
	inDispAsciiData("  CUP交易序號  ",  &uszPackBuf[inAddr], 6);
	inAddr+= 6;

	*inLen += inAddr;

	inESC_DebugLog("  Addr [%d]", inAddr);
	inESC_DebugLog("  Start Len [%d]", inStartLen);
	inESC_DebugLog("  DataCnt[%d]",*inLen);

	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

        return (0);
}

int inESC_UuPackTableR4(unsigned char *uszPackBuf , int *inLen , int inUnPackMode)
{
	int inStartLen,inSubDataLen;
	int inAddr = 0;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inStartLen = *inLen;

	inDispHexData("  R3 長度 ",  &uszPackBuf[inAddr], 2);
	inSubDataLen = ((int) uszPackBuf[inAddr] * 256) + (int) uszPackBuf[inAddr+1];
	inESC_DebugLog("  Sub Len[%x] ", inSubDataLen);
	inAddr+=2;

	/* 晶片碼  */
	inDispAsciiData("  晶片碼  ",  &uszPackBuf[inAddr], 16);
	inAddr+= 16;

	*inLen += inAddr;

	inESC_DebugLog("  Addr [%d]", inAddr);
	inESC_DebugLog("  Start Len [%d]", inStartLen);
	inESC_DebugLog("  DataCnt[%d]",*inLen);

	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

        return (0);
}

int inESC_UuPackTableR5(unsigned char *uszPackBuf , int *inLen , int inUnPackMode)
{
	int inStartLen,inSubDataLen;
	int inAddr = 0;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inStartLen = *inLen;

	inDispHexData("  R5 長度 ",  &uszPackBuf[inAddr], 2);
	inSubDataLen = ((int) uszPackBuf[inAddr] * 256) + (int) uszPackBuf[inAddr+1];
	inESC_DebugLog("  Sub Len[%x] ", inSubDataLen);
	inAddr+=2;

	/* 持卡人姓名  */
	inDispHexData("  持卡人姓名 ",  &uszPackBuf[inAddr], inSubDataLen);
	inAddr+= inSubDataLen;

	*inLen += inAddr;

	inESC_DebugLog(" Addr [%d]", inAddr);
	inESC_DebugLog(" Start Len [%d]", inStartLen);
	inESC_DebugLog(" DataCnt[%d]",*inLen);

	inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

        return (0);
}
