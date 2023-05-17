#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <ctosapi.h>
#include <ctos_qrcode.h>
#include <emv_cl.h>
#include <iconv.h>
#include <pthread.h>
#include <fcntl.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <sqlite3.h>

#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/Transaction.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DispMsg.h"
#include "../DISPLAY/DisTouch.h"
#include "../EVENT/MenuMsg.h"
#include "../EVENT/Menu.h"
#include "../EVENT/Flow.h"
#include "../EVENT/Event.h"


#include "CARD_FUNC/CardFunction.h"

#include "Accum.h"
#include "Batch.h"
#include "Card.h"
#include "CDT.h"
#include "CFGT.h"
#include "CPT.h"
#include "EDC.h"
#include "FILE_FUNC/File.h"
#include "FILE_FUNC/FIleLogFunc.h"
#include "Function.h"
#include "FuncTable.h"
#include "Sqlite.h"
#include "HDT.h"
#include "HDPT.h"
#include "KMS.h"
#include "MVT.h"
#include "PWD.h"
#include "ECR_FUNC/ECR.h"
#include "ECR_FUNC/RS232.h"
#include "MULTI_FUNC/MultiFunc.h"
#include "MULTI_FUNC/MultiHostFunc.h"
#include "MULTI_FUNC/JsonMultiHostFunc.h"
#include "PowerManagement.h"
#include "XML.h"
#include "TDT.h"
#include "Signpad.h"
#include "IPASSDT.h"
#include "ECCDT.h"
#include "USB.h"

#include "../TMS/TMSTABLE/TmsFTP.h"
#include "../TMS/TMSTABLE/TmsCPT.h"
#include "../TMS/TMSTABLE/TmsSCT.h"

#include "../COMM/Comm.h"
#include "../COMM/TLS.h"
#include "../COMM/Bluetooth.h"
#include "../COMM/Ethernet.h"
#include "../COMM/GSM.h"
#include "../COMM/WiFi.h"

#include "../../EMVSRC/EMVsrc.h"

#include "../../CTLS/CTLS.h"

#include "../FUNCTION/AccountFunction/PrintBillInfo.h"

#include "../FUNCTION/UNIT_FUNC/TimeUint.h"

#include "../../SOURCE/KEY/ProcessTmk.h"

/* _USE_NEW_TMS_FUNC_ */
#include "../TMS/EDCTmsFlow.h"
#include "../TMS/EDCTmsScheduleFunc.h"
#include "../TMS/EDCTmsUnitFunc.h"
#include "../TMS/EDCTmsFtpFunc.h"
#include "../TMS/EDCTmsUnitFunc.h"
#include "../TMS/EDCTmsFileProc.h"


#include "CREDIT_FUNC/CreditProcessDataFunc.h"
#include "CREDIT_FUNC/Creditfunc.h"
#include "CREDIT_FUNC/CreditPrintFunc.h"

#include "../NEXSYSESC/NexsysEsc.h"

#include "../../FUBON/FUBONiso.h"

/* [新增電票悠遊卡功能]  新增引用功能 [SAM] 2022/6/8 下午 5:27 */
#include "../../CMAS/CMASFunction.h"
#include "../../CMAS/CMASprtByBuffer.h"
#include "../../ECC/ECC.h"

extern  int     ginIdleMSRStatus, ginMenuKeyIn, ginIdleICCStatus;
extern  int     ginEventCode;
extern  int     ginDebug;  			/* Debug使用 extern */
extern  int     ginISODebug;  			/* Debug使用 extern */
extern	int	ginDisplayDebug;		/* Debug使用 extern */
extern	int	ginEngineerDebug;
extern	int	ginESCHostIndex;
extern	int	ginFallback;
extern	int	ginRecordTime[2][10];
extern	char	gszTermVersionID[16 + 1];
extern	char	gszTermVersionDate[16 + 1];
extern	int	ginMachineType;

extern MULTI_TABLE	gstMultiOb;	/* [修改外接感應設備] 2022/11/22 [SAM] */

char		gszDuplicatePAN[_PAN_SIZE_ + 1] = {};
unsigned long	gulRunTime;

/* 抓取設定此交易是否為Kiosk要跳過停留步驟的參數 2020/3/6 下午 4:21 [SAM] */
static char  st_chSkipPressButton = 'N';

EMV_CONFIG      EMVGlobConfig;			/* EMVsrc 有call back function 無法在CTLS include */

extern int	ginAPVersionType;

/* 因為需要ECR參數，所以定義全域變數 2022/2/18 [SAM] */
extern ECR_TABLE	gsrECROb;

char b64string[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
#define badchar(c,p) ((p = memchr(b64string, c, 64))) /* compiler有warning 改寫 */

#ifdef _DEBUG_9F1F_TAG_
	extern unsigned char gusz9F1F[129];
	extern long gln9F1FLen;
#endif

/*
Function		:inFunc_PAD_ASCII
Date&Time	:2015/6/24 下午 4:14
Describe		:靠左靠右補字或空白
 *注意		:若靠右，且pad 0x00會抓不到字串長度，建議不要包含中文字串，中文字串會算三個字元
*/
int inFunc_PAD_ASCII(char *szStr_out, char *szStr_in, char szPad_char, int inPad_size, int inAlign)
{
	int inPADlen = 0;
	char szTemplate[256 + 1];
	/* 如果 PAD 資料有錯再開啟 */
//	inDISP_LogPrintfWithFlag(" StrLen[%d] inPad_size [%d]", strlen(szStr_in),  inPad_size );	

	/* 第一步:計算要補多少字元 */
	inPADlen = inPad_size - strlen(szStr_in);

	/* 第二步:靠左靠右 */
	if (inPADlen > 0)
	{
		/* 將szTemplate初始化成要補的字元 */
		memset(szTemplate, szPad_char, sizeof(szTemplate));

		if (inAlign == _PAD_RIGHT_FILL_LEFT_)
			memcpy(&szTemplate[inPADlen], &szStr_in[0], inPad_size); /* 字靠右 */
		else if (inAlign == _PAD_LEFT_FILL_RIGHT_)
			memcpy(&szTemplate[0], &szStr_in[0], strlen(szStr_in)); /* 字靠左 */

		/* 補結束字元 & 傳回szStr_out */
		szTemplate[inPad_size] = '\0';
		memset(szStr_out, 0x00, sizeof(szStr_out));
		strcpy(szStr_out, szTemplate);
	}
	else if (inPADlen == 0)
	{
		/* inPADlen = 0 的狀況，szStr_in將字串給szStr_out */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &szStr_in[0], strlen(szStr_in));
		memset(szStr_out, 0x00, sizeof(szStr_out));
		strcpy(szStr_out, szTemplate);
	}
	else
	{
		/* inPADlen < 0 清空szStr_out Return Error */
		memset(szStr_out, 0x00, sizeof(szStr_out));
		inDISP_LogPrintfWithFlag("inPADlen < 0");
		return(VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function	:inFunc_ASCII_to_BCD
Date&Time	:2015/7/13 下午 2:02
Describe        :ASCII轉BCD，inLength指要轉換成的byte數(ASCII字串的1/2長度)
		注意:BCD的空間長度至少要為ASCII的1/2倍，否則會溢位。(2byte ascii可轉換成1byte bcd)

        void  inFunc_ASCII_to_BCD (const char *dsp, char *hex, int pairs);
                                         INPUT      OUTPUT
            Converts ASCII hexadecimal data to binary
            For example, if dsp points to the four bytes { 0x34, 0x41, 0x32, 0x65 } or the equivalent
         { '4', 'A', '2', 'e' }, and n = 2, then the two bytes { 0x4A, 0x2E } are stored in hex.
*/
int inFunc_ASCII_to_BCD(unsigned char *uszBCD, char *szASCII, int inLength)
{
        unsigned char uszTemplate = 0;
        int	i;

        /* 防呆 inLength須大於零 */
        if (inLength <= 0)
                return (VS_ERROR);

        for (i = 0; i < inLength ; i ++)
        {
                /* 字元做運算 */
                switch (*szASCII)
                {
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                                uszTemplate = *szASCII - '0';
                                break;
                        case 'A':
                        case 'B':
                        case 'C':
                        case 'D':
                        case 'E':
                        case 'F':
                                uszTemplate = *szASCII - 'A' + 10;
                                break;
                        case 'a':
                        case 'b':
                        case 'c':
                        case 'd':
                        case 'e':
                        case 'f':
                                uszTemplate = *szASCII - 'a' + 10;
                                break;
                        default:
                                break;
                }

                *uszBCD = (unsigned char) (uszTemplate << 4); /* 向左移動四位元 */
                szASCII ++;

                switch (*szASCII)
                {
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                                uszTemplate = *szASCII - '0';
                                break;
                        case 'A':
                        case 'B':
                        case 'C':
                        case 'D':
                        case 'E':
                        case 'F':
                                uszTemplate = *szASCII - 'A' + 10;
                                break;
                        case 'a':
                        case 'b':
                        case 'c':
                        case 'd':
                        case 'e':
                        case 'f':
                                uszTemplate = *szASCII - 'a' + 10;
                                break;
                        default:
                                break;
                }

                szASCII ++;
                *uszBCD = *uszBCD | uszTemplate;
                uszBCD ++;
        }

        return (VS_SUCCESS);
}

/*
Function	:inFunc_BCD_to_ASCII
Date&Time	:2015/7/13 下午 2:02
Describe	:BCD轉ASCII 注意:ASCII的空間長度要為BCD的兩倍，否則會溢位。
*/
int inFunc_BCD_to_ASCII(char *szASCII, unsigned char *uszBCD, int inLength)
{
	unsigned char uszTemplate, uszNibble;
	int i;

	for(i = 0; i < inLength; ++i)
	{
		uszTemplate = *uszBCD++;
		uszNibble = (uszTemplate / 16);         /* 第一個字  */

		/* 16進位A = 10進位的10，做運算 */
		if (uszNibble < 10)
		{
			*szASCII = (char)('0' + uszNibble);
			szASCII++;
		}
		else/* A~F */
		{
			*szASCII = (char)('A' + (uszNibble - 10));
			szASCII++;
		}

		uszNibble = (uszTemplate % 16);         /* 第二個字  */

		if (uszNibble < 10)
		{
			*szASCII = (char)('0' + uszNibble);
			szASCII++;
		}
		else
		{
			*szASCII = (char)('A' + (uszNibble - 10));
			szASCII++;
		}
	}

	/* 不要補0比較好用 */
//	*szASCII = 0;

	return (VS_SUCCESS);
}

/*
Function	:inFunc_BCD_to_INT
Date&Time	:2015/7/24 下午 17:31
Describe	:BCD轉INT
*/
int inFunc_BCD_to_INT(int *inINT, unsigned char *uszBCD, int inLength)
{
        char szASCII[10 + 1];

        memset(szASCII, 0x00, sizeof(szASCII));
        /* 使用BCD轉ASCII的Function */
        inFunc_BCD_to_ASCII(&szASCII[0], &uszBCD[0], inLength);

        *inINT = atoi(szASCII); /* 用atoi將String轉成Int */

        return (VS_SUCCESS);
}

int inBCD_ASCII_test(void)
{
	int		i;
        char		szASCII[24 + 1] = {'\0'};
	char		szTemplate[256 + 1];
        unsigned char	uszBCD[24 + 1] = {0x31,0x32,0x33,0x34,0x35,0xaB,0xCD,0xef};

        memset(szTemplate, 0x00, sizeof(szTemplate));

        inDISP_LogPrintf("BCD Initial Value");
        for(i = 0; i < 8; ++i)
        {
                sprintf(szTemplate, "0x%02x", uszBCD[i]);
                inDISP_LogPrintf(szTemplate);
        }

        inDISP_LogPrintf("-------");
        inDISP_LogPrintf("BCD to  ASCII");
        inFunc_BCD_to_ASCII(&szASCII[0], &uszBCD[0], 10);
        memcpy(szTemplate, szASCII, 20);
        inDISP_LogPrintf(szTemplate);
        inDISP_LogPrintf("-------");
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(uszBCD, 0x00, sizeof(uszBCD));
        inDISP_LogPrintf("ASCII to BCD");
        inFunc_ASCII_to_BCD (&uszBCD[0], &szASCII[0], 10);

        for(i = 0; i < 10; ++i)
        {
                sprintf(szTemplate, "0x%x", uszBCD[i]);
                inDISP_LogPrintf(szTemplate);
        }
        inDISP_LogPrintf("-----------------");

        strcpy(szASCII, "1234567890ABCDEF");

        for(i = 0; i < 20; ++i)
        {
                sprintf(szTemplate, "0x%x", szASCII[i]);
                inDISP_LogPrintf(szTemplate);
        }

        inDISP_LogPrintf("-----------------");
        inDISP_LogPrintf("BCD to INT");
        inFunc_BCD_to_INT(&i, (unsigned char*)"\x17\x39\x58", 3);
        sprintf(szTemplate, "%d", i);
        inDISP_LogPrintf(szTemplate);

        return (VS_SUCCESS);
}

int inBCD_ASCII_test2(void)
{
        char	szASCII[100 + 1];
        char	szBCD[100 + 1];
        char	szTemplate[256 + 1];

        memset(szTemplate, 0x00, sizeof(szTemplate));

	memset(szASCII, 0x00, sizeof(szASCII));
	sprintf(szASCII, "5430450000001517D181220115646974");
	inDISP_LogPrintf(szASCII);

	memset(szBCD, 0x00, sizeof(szBCD));
	inFunc_ASCII_to_BCD((unsigned char*)szBCD, szASCII, 32);
	memset(szASCII, 0x00, sizeof(szASCII));
	inFunc_BCD_to_ASCII(szASCII, (unsigned char*)szBCD, 32);
	inDISP_LogPrintf(szASCII);

        return (VS_SUCCESS);
}

/*
Function        :inFunc_Amount
Date&Time       :2016/9/5 下午 4:03
Describe        :
 *szAmt:	金額
 *szCurSymbol:	金額的符號 ex: ＄、 ￥
 *szPad_char:	要Pad的字元
 *inSigned:	如果有需要印出負0的需求
 *inWide:	最後字串的寬度
 *inAlign:	右靠左靠，True的話，右靠左補空白；False的話，左靠右補空白
 *注意:		若靠右，且pad 0x00會抓不到字串長度
*/
int inFunc_Amount(char *szAmt, char *szCurSymbol, char szPad_char, int inSigned, int inWide, int inAlign)
{
	int	inOffset = 0;
        int	inLen;	/* inPoint:現在szComma的長度 */
        char	szComma[48 + 1];
	char	szTemplate[48 + 1];
	char	szUnsignedAmt[20 + 1];


	/* 輸入數字是負數 */
        memset(szUnsignedAmt, 0x00, sizeof(szUnsignedAmt));
	if (szAmt[0] == '-')
		strcpy(szUnsignedAmt, &szAmt[1]);
	else
		strcpy(szUnsignedAmt, &szAmt[0]);

	inLen = strlen(szUnsignedAmt);
	inOffset = inLen % 3;
	memset(szComma, 0x00, sizeof(szComma));
	strcpy(szComma,szUnsignedAmt);

#if 0
	for (i = 0; i < inLen; i ++)
	{
		szComma[inPoint ++] = szUnsignedAmt[i];
		inNumberLen ++;			/* 數字長度 */

		/* 每第三個數字加comma ，若已經是最後一位也不加comma */
		if (((inNumberLen - inOffset) % 3 == 0) &&
		     (i != inLen - 1))
			szComma[inPoint ++] = 0x2C; /* 補【,】 */
	}
#endif

	memset(szTemplate, 0x00, sizeof(szTemplate));

	/* Flag有On或是原傳入金額為負 */
	if((inSigned == _SIGNED_MINUS_ || szAmt[0] == '-') && (szComma[0] != '0' || szComma[1] != 0x00))
		sprintf(szTemplate, "-%s", szComma);
	else
		sprintf(szTemplate, "%s", szComma);

	/* 補空白 */
	if (inAlign == _PAD_RIGHT_FILL_LEFT_)
		inFunc_PAD_ASCII(szTemplate, szTemplate, szPad_char, inWide - strlen(szCurSymbol), _PAD_RIGHT_FILL_LEFT_);	/* 右靠左補空白 */
	else
		inFunc_PAD_ASCII(szTemplate, szTemplate, szPad_char, inWide - strlen(szCurSymbol), _PAD_LEFT_FILL_RIGHT_);	/* 左靠右補空白 */

	/* 金額符號 */
	if (strlen(szCurSymbol) != 0)
		sprintf(szAmt, "%s%s", szCurSymbol, szTemplate);
	else
		sprintf(szAmt, "%s", szTemplate);

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Amount_Comma
Date&Time       :2016/9/5 下午 4:03
Describe        :
 *szAmt:	金額
 *szCurSymbol:	金額的符號 ex: ＄、 ￥
 *szPad_char:	要Pad的字元
 *inSigned:	如果有需要印出負0的需求
 *inWide:	最後字串的寬度
 *inAlign:	右靠左靠，True的話，右靠左補空白；False的話，左靠右補空白
 *注意:		若靠右，且pad 0x00會抓不到字串長度
*/
int inFunc_Amount_Comma(char *szAmt, char *szCurSymbol, char szPad_char, int inSigned, int inWide, int inAlign)
{
	int	inOffset = 0;
	int	inNumberLen = 0;	/* 已放進szComma的數字數量 */
        int	inLen, inPoint = 0, i;	/* inPoint:現在szComma的長度 */
        char	szComma[48 + 1];
	char	szTemplate[48 + 1];
	char	szUnsignedAmt[20 + 1];


	/* 輸入數字是負數 */
        memset(szUnsignedAmt, 0x00, sizeof(szUnsignedAmt));
	if (szAmt[0] == '-')
		strcpy(szUnsignedAmt, &szAmt[1]);
	else
		strcpy(szUnsignedAmt, &szAmt[0]);

	inLen = strlen(szUnsignedAmt);
	inOffset = inLen % 3;
	memset(szComma, 0x00, sizeof(szComma));

	for (i = 0; i < inLen; i ++)
	{
		szComma[inPoint ++] = szUnsignedAmt[i];
		inNumberLen ++;			/* 數字長度 */

		/* 每第三個數字加comma ，若已經是最後一位也不加comma */
		if (((inNumberLen - inOffset) % 3 == 0) &&
		     (i != inLen - 1))
			szComma[inPoint ++] = 0x2C; /* 補【,】 */
	}


	memset(szTemplate, 0x00, sizeof(szTemplate));

	/* Flag有On或是原傳入金額為負 */
	if((inSigned == _SIGNED_MINUS_ || szAmt[0] == '-') && (szComma[0] != '0' || szComma[1] != 0x00))
		sprintf(szTemplate, "-%s", szComma);
	else
		sprintf(szTemplate, "%s", szComma);

	if(inAlign != _DO_NOT_NEED_PAD_)
	{
		/* 補空白 */
		if (inAlign == _PAD_RIGHT_FILL_LEFT_)
			inFunc_PAD_ASCII(szTemplate, szTemplate, szPad_char, inWide - strlen(szCurSymbol), _PAD_RIGHT_FILL_LEFT_);	/* 右靠左補空白 */
		else
			inFunc_PAD_ASCII(szTemplate, szTemplate, szPad_char, inWide - strlen(szCurSymbol), _PAD_LEFT_FILL_RIGHT_);	/* 左靠右補空白 */
	}

	/* 金額符號 */
	if (strlen(szCurSymbol) != 0)
		sprintf(szAmt, "%s%s", szCurSymbol, szTemplate);
	else
		sprintf(szAmt, "%s", szTemplate);

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Amount_Comma_DCC
Date&Time       :2016/9/5 下午 2:30
Describe        :根據minor unit來決定小數點
 *szAmt:	金額
 *szCurSymbol:	金額的符號 ex: ＄、 ￥(不輸入的話放"")
 *szPad_char:	要Pad的字元
 *inWide:	最後字串的寬度
 *inAlign:	右靠左靠，True的話，右靠左補空白；False的話，左靠右補空白
 *szMinorUnit:	有幾位是小數點以下的數字
 *szCurrCode:	幣別碼(不輸入的話放"")
 *szOutput:	輸出
 *注意:		若靠右，且pad 0x00會抓不到字串長度
*/
int inFunc_Amount_Comma_DCC(char *szAmt, char* szCurSymbol, char szPad_char, int inSigned, int inWide, int inAlign, char *szMinorUnit, char *szCurrCode, char* szOutput)
{
	int	i, inMUIndex, inCnt, inFCMU;
	int	inComma	;			/* 有幾位數中間要放comma */
	char	szTemplate[50 + 1], szTemplate2[50 + 1];
	char	szFCA[12 + 1], szComma[42 + 1];
	char	szAmount_Minus[13 + 1] = {};	/* 確認是否有負數用 */

	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcpy(szTemplate, szAmt);
	/* 把金額移到最右並把其他欄位放數字0(為了去除金額前面的空白) */
	inFunc_PAD_ASCII(szTemplate, szTemplate, '0' , 12, _PAD_RIGHT_FILL_LEFT_);

	/* 去除金額前面的空白 */
	for (i = 0; i < 12; i ++)
	{
		/* 加入i == 12 - 1 是為了避免0元不會複製金額的情況 */
		if ((szTemplate[i] != 0x30 && szTemplate[i] != 0x20) ||
		     i == 12 - 1)
		{
			memset(szAmount_Minus, 0x00, sizeof(szAmount_Minus));
			memcpy(szAmount_Minus, &szTemplate[i], (12 - i)); /* Currency Amount */
			break;
		}
	}

	memset(szFCA, 0x00, sizeof(szFCA));
	/* 輸入數字是負數 */
	if (szAmount_Minus[0] == '-')
	{
		memcpy(szFCA, &szAmount_Minus[1], 12);
	}
	else
	{
		memcpy(szFCA, &szAmount_Minus[0], 12);
	}

	inFCMU = atoi(szMinorUnit); /* Currency Minor Unit(表示有幾位數字是在小數點後) */

	/* 沒有小數點時(這一段在加小數點) */
	if (inFCMU == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, szFCA);
		/* 算有幾位數要塞comma */
		inComma = strlen(szTemplate);
	}
	else
	{
		/* 如果有數字的位數小於小數點位數(Ex:0.03 strlen(szFCA) = 1, inFCMU = 2) */
		if (strlen(szFCA) <= inFCMU)
			inFunc_PAD_ASCII(szFCA, szFCA, '0', (inFCMU + 1), _PAD_RIGHT_FILL_LEFT_);

		inMUIndex = (strlen(szFCA) - inFCMU); /* 第幾位放小數點 */
		inCnt = 0;/* inCnt:szTemplate中的index  i:szFCA中的index */
		memset(szTemplate, 0x00, sizeof(szTemplate));

		for (i = 0; i < strlen(szFCA); i ++)
		{
			szTemplate[inCnt ++] = szFCA[i];

			/* 要放小數點 */
			if (inCnt == inMUIndex)
				szTemplate[inCnt ++] = 0x2E; /* 補【.】 */
		}

		/* 算有幾位數要塞comma */
		for (inComma = 0; inComma < strlen(szTemplate); inComma ++)
		{
			/* 小數點以下不加comma */
			if (szTemplate[inComma] == 0x2E)
				break;
		}

	}

	/* 這一段在加comma */
	memset(szComma, 0x00, sizeof(szComma));

	if (inComma <= 3)
		strcpy(szComma, szTemplate);
	else if (inComma >= 4 && inComma <= 6)
	{
		memset(szComma, 0x00, sizeof(szComma));
		inCnt = 0;

		for (i = 0; i < strlen(szTemplate); i ++)
		{
			szComma[inCnt ++] = szTemplate[i];

			if (i == (inComma - 3 - 1))
				szComma[inCnt ++] = 0x2C;	/* 補【,】 */
		}

	}
	else if (inComma >= 7)
	{
		memset(szComma, 0x00, sizeof(szComma));
		inCnt = 0;

		for (i = 0; i < strlen(szTemplate); i ++)
		{
			szComma[inCnt ++] = szTemplate[i];

			if (i == (inComma - 3 - 1))
				szComma[inCnt ++] = 0x2C;		/* 補【,】 */
			else if (i == (inComma - 3 - 3 - 1))
				szComma[inCnt ++] = 0x2C;		/* 補【,】 */
		}
	}

	/* 塞完comma和dot後，若原本有負號，這個時候塞回來 */
	/* Flag有On或是原傳入金額為負 */
	memset(szTemplate2, 0x00, sizeof(szTemplate));
	if (inSigned == _SIGNED_MINUS_ || szAmount_Minus[0] == '-')
		sprintf(szTemplate2, "-%s", szComma);
	else
		sprintf(szTemplate2, "%s", szComma);

	memset(szTemplate, 0x00, sizeof(szTemplate));
	/* 金額符號 */
	if (strlen(szCurSymbol) != 0)
		sprintf(szTemplate, "%s%s", szCurSymbol, szTemplate2);
	else
		sprintf(szTemplate, "%s",szTemplate2);

	/* 幣別碼 */
	if (strlen(szCurrCode) != 0)
		sprintf(szOutput, "%s %s", szCurrCode, szTemplate);	/* Currcncy Alphabetic Code */
	else
		sprintf(szOutput, "%s", szTemplate);			/* Currcncy Alphabetic Code */

	/* 補空白 */
	if (inAlign == _PAD_RIGHT_FILL_LEFT_)
		inFunc_PAD_ASCII(szOutput, szOutput, szPad_char, inWide, _PAD_RIGHT_FILL_LEFT_);	/* 右靠左補空白 */
	else
		inFunc_PAD_ASCII(szOutput, szOutput, szPad_char, inWide, _PAD_LEFT_FILL_RIGHT_);	/* 左靠右補空白 */

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Currency_Conversion_Fee
Date&Time       :2016/9/7 下午 4:01
Describe        :根據minor unit來決定保留到小數點下第幾位
 *szMPV_In:	轉換費率值(Markup Percentage Value)
 *szMinorUnit:	有幾位是小數點以下的數字
 *szOutput:	輸出
*/
int inFunc_Currency_Conversion_Fee(char *szMPV_In, char *szMinorUnit, char* szOutput)
{
	int	i;
	int	inMPV;				/* 要保留到小數點下第幾位 */
	char	szMPV[10 + 1];			/* Markup Percentage Value */

	/* 轉換費率 */
	memset(szMPV, 0x00, sizeof(szMPV));
	strcpy(szMPV, szMPV_In);		/* Markup Percentage Value */

	/* 要保留到小數點下第幾位 */
	inMPV = atoi(szMinorUnit);		/* Markup Percentage Decimal Point */

	/* 小數點後不保留的情況 */
	if (inMPV == 0)
	{
		for (i = 0; i < strlen(szMPV); i ++)
		{
			if (szMPV[i] == 0x2E)
			{
				szMPV[i] = 0x00;
				break;
			}
		}

		memset(szOutput, 0x00, sizeof(szOutput));
		strcpy(szOutput, szMPV);
	}
	else
	{
		memset(szOutput, 0x00, sizeof(szOutput));

		for (i = 0; i < strlen(szMPV); i ++)
		{
			if (szMPV[i] == 0x2E)
			{
				szMPV[i + 1 + inMPV] = 0x00;	/* 幾位小數點(中間的1是小數點) */
				break;
			}
		}

		strcpy(szOutput, szMPV);
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_GetCardFields
Date&Time       :2017/6/13 下午 6:09
Describe        :只接收刷卡和人工輸入的分流
*/
int inFunc_GetCardFields_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szFunEnable[2 + 1];

	memset(szFunEnable, 0x00, sizeof(szFunEnable));
	inGetStore_Stub_CardNo_Truncate_Enable(szFunEnable);
/*  先刪掉，因為富邦在刷卡時不用此參數 20190211 [SAM] */
//	if (memcmp(szFunEnable, "Y", strlen("Y")) == 0)
//	{
//		inRetVal = inFunc_GetCardFields_Txno(pobTran);
//	}
//	else
	{
		inRetVal = inFunc_GetCardFields(pobTran);
	}

	return (inRetVal);
}

/*
Function        :inFunc_GetCardFields
Date&Time       :2017/1/20 下午 4:29
Describe        :
*/
int inFunc_GetCardFields(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	int	inMSR_RetVal = -1;	/* 磁條事件的反應，怕和其他用到inRetVal的搞混所以獨立出來 */
        char	szKey = 0;
	char	szFuncEnable[2 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	long	lnTimeout = 0;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_GetCardFields() START !");
	}

	/* 如果有卡號就直接跳走 */
	if (strlen(pobTran->srBRec.szPAN) > 0)
	{
		return (VS_SUCCESS);
	}

        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;

        /* 如果ginEventCode為數字鍵流程即為IDLE MenuKeyIn */
        if (ginEventCode >= '0' && ginEventCode <= '9')
        {
                /* 表示是idle手動輸入，這邊才on flag是因為有可能idle輸入金額 */
                ginMenuKeyIn = VS_TRUE;
        }

	/* 設定Timeout */
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	if (pobTran->uszECRBit == VS_TRUE)
	{
		lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
	}
	else
	{
		lnTimeout = 30;
	}

	inDISP_Timer_Start(_TIMER_GET_CARD_, lnTimeout);
        /* 先設定為讀卡失敗，有成功再設定回沒設定錯誤碼  */
        inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_READER_CARD_ERROR_);
        while (1)
        {
                /* idle 刷卡 或 一般刷卡 */
	        if (ginIdleMSRStatus == VS_TRUE  || ginEventCode == _SWIPE_EVENT_)
	        {
	                /* 第一步驟GetTrack123 */
	                inCARD_GetTrack123(pobTran);

	                /* 第二步驟unPackTrack資料 */
	                if (inCARD_unPackCard(pobTran) != VS_SUCCESS)
	                {
				/* Unpack失敗，把flag關掉可以再手動輸入 */
	                        ginIdleMSRStatus = VS_FALSE;
	                        continue;
	                }

                        /* 第三步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);

                        /* 第四步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* U CARD 有自己的檢核法 */
				if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
				{
					if (inCARD_ValidTrack2_UCard_PAN(pobTran->srBRec.szPAN) != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}
			}

  	                /* 第六步驟檢核SeviceCode */
                        if (inCheckFallbackFlag(pobTran) != VS_SUCCESS)
	                        return (VS_ERROR);

			/* 第七步驟檢核ExpDate */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetExpiredDateCheck(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}
			
			/* 富邦測試列印用 */
			inFunc_Print9F1FData(pobTran);
			
			/* 決定TRT */
			inRetVal = inFunc_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

                        /* 先設定回無錯誤狀態碼 2022/2/18 [SAM] */
                            inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_NOT_SET_ERROR_);
                        return (VS_SUCCESS);
	        }
                /* idle手動輸入或一般輸入 */
                else if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
                {
			/* 表示是手動輸入 */
                        pobTran->srBRec.uszManualBit = VS_TRUE;

			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			/* V3 Enter key 代碼為 'A' 所以要限制 */
			if (szKey >= '0' && szKey <= '9')
			{
				pobTran->inMenuKeyin = (int)szKey;
			}

			/* 人工輸入卡號開關 */
			/* 退貨交易及預先授權/預先授權完成交易部參考人工輸入功能開關 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetManualKeyin(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0		&&
			   (pobTran->inTransactionCode != _REFUND_		&&
			    pobTran->inTransactionCode != _INST_REFUND_		&&
			    pobTran->inTransactionCode != _REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _PRE_AUTH_		&&
			    pobTran->inTransactionCode != _PRE_COMP_		&&
			    pobTran->inTransactionCode != _REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_REFUND_		&&
			    pobTran->inTransactionCode != _CUP_INST_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_PRE_AUTH_	&&
			    pobTran->inTransactionCode != _CUP_PRE_COMP_))
			{
				inRetVal = inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_4_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

				return (inRetVal);
			}

			inRetVal = inCREDIT_Func_Get_Card_Number(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			if (inCARD_Generate_Special_Card(pobTran->srBRec.szPAN) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}

                        /* 第一步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);

			/* 第二步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* U CARD 有自己的檢核法 */
				if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
				{
					if (inCARD_ValidTrack2_UCard_PAN_MenuKeyIn(pobTran->srBRec.szPAN) != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}
			}

                        /* 第三步選擇輸入檢查碼或有效期 */
                        inRetVal = inCREDIT_Func_Get_CheckNO_ExpDate_Flow(pobTran);
			if (inRetVal != VS_SUCCESS)
				return (inRetVal);

                        /* 第四步驟檢核ExpDate */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetExpiredDateCheck(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}

			/* 決定TRT */
			inRetVal = inFunc_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
                        /* 先設定回無錯誤狀態碼 2022/2/18 [SAM] */
                            inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_NOT_SET_ERROR_);
                        return (VS_SUCCESS);
                }

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 顯示請刷卡 */
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			inDISP_PutGraphic(_GET_CARD_CUP_, 0, _COORDINATE_Y_LINE_8_4_);
		}
		else
		{
			inDISP_PutGraphic(_GET_CARD_, 0, _COORDINATE_Y_LINE_8_4_);
		}

		/* 進迴圈前先清MSR BUFFER */
		inCARD_Clean_MSR_Buffer();

		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測刷卡------------------*/
			inMSR_RetVal = inCARD_MSREvent();
			if (inMSR_RetVal == VS_SUCCESS)
			{
				/* 刷卡事件 */
				ginEventCode = _SWIPE_EVENT_;
			}
			/* 回復錯誤訊息蓋掉的圖 */
			else if (inMSR_RetVal == VS_SWIPE_ERROR)
			{
				inFunc_ResetTitle(pobTran);
				inDISP_Msg_BMP(_ERR_READ_CARD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);

				/* 顯示請刷卡 */
				if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
				{
					inDISP_PutGraphic(_GET_CARD_CUP_, 0, _COORDINATE_Y_LINE_8_4_);
				}
				else
				{
					inDISP_PutGraphic(_GET_CARD_, 0, _COORDINATE_Y_LINE_8_4_);
				}
			}

         
			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 檢查TIMEOUT */
			if (inTimerGet(_TIMER_GET_CARD_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* Timeout or Cancel */
                                        inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_TIMEOUT_);
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* Timeout or Cancel */
				return (VS_USER_CANCEL);
			}
			/* ManuKeyin 為數字鍵時 */
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* key in事件 */
				ginEventCode = _MENUKEYIN_EVENT_;
			}

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				/* 銀聯一般交易不能輸入卡號 */
				if (pobTran->inTransactionCode == _CUP_SALE_ && ginEventCode == _MENUKEYIN_EVENT_)
					continue;
				else
					break;
			}

		}/* while (1) 偵測事件迴圈...*/

	}/* while (1) 對事件做回應迴圈...*/

        return (VS_SUCCESS);
}

/*
Function        :inFunc_GetCardFields_Txno
Date&Time       :2017/6/13 下午 6:14
Describe        :
*/
int inFunc_GetCardFields_Txno(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	int	inMSR_RetVal = -1;	/* 磁條事件的反應，怕和其他用到inRetVal的搞混所以獨立出來 */
	char	szKey = 0;
	char	szFuncEnable[2 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	long	lnTimeout = 0;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_GetCardFields_Txno() START !");
	}

	/* 如果有卡號就直接跳走 */
	if (strlen(pobTran->srBRec.szPAN) > 0)
	{
		return (VS_SUCCESS);
	}

	/* 過卡方式參數初始化  */
	pobTran->srBRec.uszManualBit = VS_FALSE;

	/* 如果ginEventCode為數字鍵流程即為IDLE MenuKeyIn */
	if (ginEventCode >= '0' && ginEventCode <= '9')
	{
		/* 表示是idle手動輸入，這邊才on flag是因為有可能idle輸入金額 */
		ginMenuKeyIn = VS_TRUE;
	}

	/* 設定Timeout */
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	if (pobTran->uszECRBit == VS_TRUE)
	{
		lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
	}
	else
	{
		lnTimeout = 30;
	}

	inDISP_Timer_Start(_TIMER_GET_CARD_, lnTimeout);

        while (1)
        {
                /* idle 刷卡 或 一般刷卡 */
	        if (ginIdleMSRStatus == VS_TRUE  || ginEventCode == _SWIPE_EVENT_)
	        {
	                /* 第一步驟GetTrack123 */
	                inCARD_GetTrack123(pobTran);

	                /* 第二步驟unPackTrack資料 */
	                if (inCARD_unPackCard(pobTran) != VS_SUCCESS)
	                {
				/* Unpack失敗，把flag關掉可以再手動輸入 */
	                        ginIdleMSRStatus = VS_FALSE;
	                        continue;
	                }

                        /* 第三步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);

                        /* 第四步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* U CARD 有自己的檢核法 */
				if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
				{
					if (inCARD_ValidTrack2_UCard_PAN(pobTran->srBRec.szPAN) != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}
			}

  	                /* 第六步驟檢核SeviceCode */
                        if (inCheckFallbackFlag(pobTran) != VS_SUCCESS)
	                        return (VS_ERROR);

			/* 第七步驟檢核ExpDate */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetExpiredDateCheck(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}

			/* 決定TRT */
			inRetVal = inFunc_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

                        return (VS_SUCCESS);
	        }
                /* idle手動輸入或一般輸入 */
                else if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
                {
			/* 表示是手動輸入 */
                        pobTran->srBRec.uszManualBit = VS_TRUE;

			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			/* 人工輸入卡號開關 */
			/* 退貨交易及預先授權/預先授權完成交易部參考人工輸入功能開關 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetManualKeyin(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0		&&
			   (pobTran->inTransactionCode != _REFUND_		&&
			    pobTran->inTransactionCode != _INST_REFUND_		&&
			    pobTran->inTransactionCode != _REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _PRE_AUTH_		&&
			    pobTran->inTransactionCode != _PRE_COMP_		&&
			    pobTran->inTransactionCode != _REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_REFUND_		&&
			    pobTran->inTransactionCode != _CUP_INST_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_PRE_AUTH_	&&
			    pobTran->inTransactionCode != _CUP_PRE_COMP_))
			{
				inRetVal = inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_4_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

				return (inRetVal);
			}

			inRetVal = inCREDIT_Func_Get_Card_Number_Txno_Flow(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			if (inCARD_Generate_Special_Card(pobTran->srBRec.szPAN) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}

                        /* 第一步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);



			/* 第二步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* U CARD 有自己的檢核法 */
				if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
				{
					if (inCARD_ValidTrack2_UCard_PAN_MenuKeyIn(pobTran->srBRec.szPAN) != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}
			}

                        /* 第三步選擇輸入檢查碼或有效期 */
                        /* 部份交易不能允許輸入檢查碼 */
			if (pobTran->srBRec.inCode == _SALE_		||
			    pobTran->srBRec.inCode == _CUP_SALE_	||
			    pobTran->srBRec.inCode == _INST_SALE_	||
			    pobTran->srBRec.inCode == _REDEEM_SALE_	||
			    pobTran->srBRec.inCode == _PRE_AUTH_	||
			    pobTran->srBRec.inCode == _CUP_INST_SALE_	||
			    pobTran->srBRec.inCode == _CUP_REDEEM_SALE_	||
			    pobTran->srBRec.inCode == _CUP_PRE_AUTH_)
			{
				inRetVal = inCREDIT_Func_Get_Exp_Date(pobTran);
			}
			else
			{
				inRetVal = inCREDIT_Func_Get_CheckNO_ExpDate_Flow(pobTran);
			}
			if (inRetVal != VS_SUCCESS)
				return (inRetVal);

                        /* 第四步驟檢核ExpDate */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetExpiredDateCheck(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}

			/* 決定TRT */
			inRetVal = inFunc_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

                        return (VS_SUCCESS);
                }

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 顯示請刷銀聯卡 人工輸入按0 */
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			inDISP_PutGraphic(_GET_CARD_CUP_TXNO_, 0, _COORDINATE_Y_LINE_8_4_);
		}
		/* 顯示請刷卡 人工輸入按0 */
		else
		{
			inDISP_PutGraphic(_GET_CARD_TXNO_, 0, _COORDINATE_Y_LINE_8_4_);
		}

		/* 進迴圈前先清MSR BUFFER */
		inCARD_Clean_MSR_Buffer();

		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測刷卡------------------*/
			inMSR_RetVal = inCARD_MSREvent();
			if (inMSR_RetVal == VS_SUCCESS)
			{
				/* 刷卡事件 */
				ginEventCode = _SWIPE_EVENT_;
			}
			/* 回復錯誤訊息蓋掉的圖 */
			else if (inMSR_RetVal == VS_SWIPE_ERROR)
			{
				inFunc_ResetTitle(pobTran);
				inDISP_Msg_BMP(_ERR_READ_CARD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);

				/* 顯示請刷銀聯卡 人工輸入按0 */
				if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
				{
					inDISP_PutGraphic(_GET_CARD_CUP_TXNO_, 0, _COORDINATE_Y_LINE_8_4_);
				}
				/* 顯示請刷卡 人工輸入按0 */
				else
				{
					inDISP_PutGraphic(_GET_CARD_TXNO_, 0, _COORDINATE_Y_LINE_8_4_);
				}
			}

			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 檢查TIMEOUT */
			if (inTimerGet(_TIMER_GET_CARD_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* Cancel */
				return (VS_USER_CANCEL);
			}
			/* ManuKeyin 為數字鍵時 */
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* key in事件 人工輸入請按0*/
				if (szKey == '0')
				{
					ginEventCode = _MENUKEYIN_EVENT_;
				}
			}

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				/* 銀聯一般交易不能輸入卡號 */
				if (pobTran->inTransactionCode == _CUP_SALE_ && ginEventCode == _MENUKEYIN_EVENT_)
					continue;
				else
					break;
			}

		}/* while (1) 偵測事件迴圈...*/

	}/* while (1) 對事件做回應迴圈...*/

        return (VS_SUCCESS);
}

/*
Function        :inFunc_GetCardFields_ICC
Date&Time       :2018/5/29 下午 3:50
Describe        :
*/
int inFunc_GetCardFields_ICC(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	int	inMSR_RetVal = -1;	/* 磁條事件的反應，怕和其他用到inRetVal的搞混所以獨立出來 */
	int	inEMV_RetVal = -1;	/* 晶片卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
	char	szKey = 0;
	char	szFuncEnable[2 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	long	lnTimeout = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 如果有卡號就直接跳走 */
	if (strlen(pobTran->srBRec.szPAN) > 0)
	{
		return (VS_SUCCESS);
	}

	/* 過卡方式參數初始化  */
	pobTran->srBRec.uszManualBit = VS_FALSE;

	/* 如果ginEventCode為數字鍵流程即為IDLE MenuKeyIn */
	if (ginEventCode >= '0' && ginEventCode <= '9')
	{
		/* 表示是idle手動輸入，這邊才on flag是因為有可能idle輸入金額 */
		ginMenuKeyIn = VS_TRUE;
	}

	/* 設定Timeout */
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	if (pobTran->uszECRBit == VS_TRUE)
	{
		lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
	}
	else
	{
		lnTimeout = 30;
	}

        /* 先設定為讀卡失敗，有成功再設定回沒設定錯誤碼 2022/2/18 [SAM] */
        inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_READER_CARD_ERROR_);
    
	inDISP_Timer_Start(_TIMER_GET_CARD_, lnTimeout);

	while (1)
	{

		/* idle 刷卡 或 一般刷卡 */
		if (ginIdleMSRStatus == VS_TRUE || ginEventCode == _SWIPE_EVENT_)
		{
			/* 第一步驟GetTrack123 */
			inCARD_GetTrack123(pobTran);

			/* 第二步驟unPackTrack資料 */
			if (inCARD_unPackCard(pobTran) != VS_SUCCESS)
			{
				/* Unpack失敗，把flag關掉可以再手動輸入 */
				ginIdleMSRStatus = VS_FALSE;
				continue;
			}

			/* 第三步驟 判斷card bin 讀HDT */
			if (inCARD_GetBin(pobTran) != VS_SUCCESS)
					return(VS_ERROR);

			/* 第四步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* U CARD 有自己的檢核法 */
				if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
				{
					if (inCARD_ValidTrack2_UCard_PAN(pobTran->srBRec.szPAN) != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}
			}

			/* 第六步驟檢核SeviceCode */
			if (inCheckFallbackFlag(pobTran) != VS_SUCCESS)
				return (VS_ERROR);

			/* 第七步驟檢核ExpDate */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetExpiredDateCheck(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}
			/* 富邦測試列印用 */
			inFunc_Print9F1FData(pobTran);
			
			/* 決定TRT */
			inRetVal = inFunc_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
                            /* 先設定回無錯誤狀態碼 2022/2/18 [SAM] */
                            inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_NOT_SET_ERROR_);
			return (VS_SUCCESS);
		}
		/* idle手動輸入或一般輸入 */
		else if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
		{
			/* 表示是手動輸入 */
			pobTran->srBRec.uszManualBit = VS_TRUE;

			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			/* V3 Enter key 代碼為 'A' 所以要限制 */
			if (szKey >= '0' && szKey <= '9')
			{
				pobTran->inMenuKeyin = (int)szKey;
			}

			/* 人工輸入卡號開關 */
			/* 退貨交易及預先授權/預先授權完成交易部參考人工輸入功能開關 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetManualKeyin(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0		&&
			   (pobTran->inTransactionCode != _REFUND_		&&
			    pobTran->inTransactionCode != _INST_REFUND_		&&
			    pobTran->inTransactionCode != _REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _PRE_AUTH_		&&
			    pobTran->inTransactionCode != _PRE_COMP_		&&
			    pobTran->inTransactionCode != _REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_REFUND_		&&
			    pobTran->inTransactionCode != _CUP_INST_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_PRE_AUTH_	&&
			    pobTran->inTransactionCode != _CUP_PRE_COMP_))
			{
				inRetVal = inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

				return (inRetVal);
			}

			inRetVal = inCREDIT_Func_Get_Card_Number(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			if (inCARD_Generate_Special_Card(pobTran->srBRec.szPAN) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}

			/* 第一步驟 判斷card bin 讀HDT */
			if (inCARD_GetBin(pobTran) != VS_SUCCESS)
				return(VS_ERROR);

			/* 第二步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* U CARD 有自己的檢核法 */
				if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
				{
					if (inCARD_ValidTrack2_UCard_PAN_MenuKeyIn(pobTran->srBRec.szPAN) != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}
			}

			/* 第三步選擇輸入檢查碼或有效期 */
			/* 部份交易不能允許輸入檢查碼 */
			if (pobTran->srBRec.inCode == _SALE_		||
			    pobTran->srBRec.inCode == _CUP_SALE_	||
			    pobTran->srBRec.inCode == _INST_SALE_	||
			    pobTran->srBRec.inCode == _REDEEM_SALE_	||
			    pobTran->srBRec.inCode == _PRE_AUTH_	||
			    pobTran->srBRec.inCode == _CUP_INST_SALE_	||
			    pobTran->srBRec.inCode == _CUP_REDEEM_SALE_	||
			    pobTran->srBRec.inCode == _CUP_PRE_AUTH_)
			{
				inRetVal = inCREDIT_Func_Get_Exp_Date(pobTran);
			}
			else
			{
				inRetVal = inCREDIT_Func_Get_CheckNO_ExpDate_Flow(pobTran);
			}
			
			if (inRetVal != VS_SUCCESS)
				return (inRetVal);

			/* 第四步驟檢核ExpDate */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetExpiredDateCheck(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}

			/* 決定TRT */
			inRetVal = inFunc_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
                            
                            /* 先設定回無錯誤狀態碼 2022/2/18 [SAM] */
                            inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_NOT_SET_ERROR_);
			return (VS_SUCCESS);
		}
		/* idle 插晶片卡 */
		else if  (ginIdleICCStatus == VS_TRUE || ginEventCode == _EMV_DO_EVENT_)
		{
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE); /* 關閉FALL BACK */
			}

			/* 取得晶片卡資料 */
			if (inEMV_GetEMVCardData(pobTran) != VS_SUCCESS)
				return (VS_ERROR);
			else
			{
				if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				{
					if (inEMV_GetCardNoFlow(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
#ifdef _MODIFY_EMV_SELECT_AID_
					/* 第三步驟 判斷card bin 讀HDT */
					if (inCARD_GetBin(pobTran) != VS_SUCCESS)
						return (VS_ERROR);

					/* 第四步驟檢核PAN module 10 */
					memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
					inGetModule10Check(szFuncEnable);
					if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
					{
						/* U CARD 有自己的檢核法 */
						if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
						{
							if (inCARD_ValidTrack2_UCard_PAN(pobTran->srBRec.szPAN) != VS_SUCCESS){
								inDISP_DispLogAndWriteFlie("  Emv Process Valid Ucard Pan Bin *Error* Line[%d]", __LINE__);
								return (VS_ERROR);
							}
						}
						else
						{
							if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
							{
								inDISP_DispLogAndWriteFlie("  Emv Process Valid Track2 Pan Bin *Error* Line[%d]", __LINE__);
								return (VS_ERROR);
							}
						}
					}

					/* 第五步驟檢核ExpDate */
					memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
					inGetExpiredDateCheck(szFuncEnable);
					if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
					{
						if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
							return (VS_ERROR);
					}
#endif
				}
				/* TRT在inEMV_GetEMVCardData裡面selectAID時設定 */
				/* 這邊是HardCode，如果之後有不同的收單行，可能要用AID來分 (EX:AE晶片卡)*/
			}

			/* 決定TRT */
			inRetVal = inFunc_Decide_ICC_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
                            /* 先設定回無錯誤狀態碼 2022/2/18 [SAM] */
                            inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_NOT_SET_ERROR_);
			return (VS_SUCCESS);
		}
                
                /* 顯示請刷銀聯卡或插卡 */
		else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			inDISP_PutGraphic(_GET_CARD_CUP_ICC_, 0, _COORDINATE_Y_LINE_8_4_);
		}
		/* 顯示請刷卡或插卡 */
		else
		{
			inDISP_PutGraphic(_GET_CARD_ICC_, 0, _COORDINATE_Y_LINE_8_4_);
		}

		/* 進迴圈前先清MSR BUFFER */
		inCARD_Clean_MSR_Buffer();
		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測刷卡------------------*/
			inMSR_RetVal = inCARD_MSREvent();
			if (inMSR_RetVal == VS_SUCCESS)
			{
				/* 刷卡事件 */
				ginEventCode = _SWIPE_EVENT_;
			}
			/* 回復錯誤訊息蓋掉的圖 */
			else if (inMSR_RetVal == VS_SWIPE_ERROR)
			{
				inFunc_ResetTitle(pobTran);
				inDISP_Msg_BMP(_ERR_READ_CARD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);

				/* 顯示請刷銀聯卡或插卡 */
				if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
				{
					inDISP_PutGraphic(_GET_CARD_CUP_ICC_, 0, _COORDINATE_Y_LINE_8_4_);
				}
				/* 顯示請刷卡或插卡 */
				else
				{
					inDISP_PutGraphic(_GET_CARD_ICC_, 0, _COORDINATE_Y_LINE_8_4_);
				}
			}

			/* ------------偵測晶片卡---------------- */
			inEMV_RetVal = inEMV_ICCEvent();
			if (inEMV_RetVal == VS_SUCCESS)
			{
				/* 晶片卡事件 */
				ginEventCode = _EMV_DO_EVENT_;
			}

			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 檢查TIMEOUT */
			if (inTimerGet(_TIMER_GET_CARD_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* Timeout */
                                    inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_TIMEOUT_);
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* Cancel */
				return (VS_USER_CANCEL);
			}
			/* ManuKeyin 為數字鍵時 */
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* key in事件 */
				ginEventCode = _MENUKEYIN_EVENT_;
			}
                        
			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				/* 銀聯一般交易不能輸入卡號 */
				if (pobTran->inTransactionCode == _CUP_SALE_ && ginEventCode == _MENUKEYIN_EVENT_)
					continue;
				else
					break;
			}

		}/* while (1) 偵測事件迴圈...*/

	}/* while (1) 對事件做回應迴圈...*/

}

/*
Function        :inFunc_GetCardFields_CTLS
Date&Time       :2016/11/2 上午 11:40
Describe        :進這隻必定CTLS Enable為開
*/
int inFunc_GetCardFields_CTLS(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inMSR_RetVal = -1;	/* 磁條事件的反應，怕和其他用到inRetVal的搞混所以獨立出來 */
	int		inEMV_RetVal = -1;	/* 晶片卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
	int		inCTLS_RetVal = -1;	/* 感應卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
	char		szKey = -1;
	char		szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
	char		szFuncEnable[2 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	long		lnTimeout = 0;
	unsigned long   ulCTLS_RetVal = -1;
	
	int		inChoice = 0;		/* [修改外接感應設備] 2022/11/22 [SAM] */
	char		szCTLSMode[1 + 1] = {0};/* [修改外接感應設備] 2022/11/22 [SAM] */
	unsigned short	uszKey;			/* [修改外接感應設備] 2022/11/22 [SAM] */
//	VS_BOOL fCTLS_Send = VS_FALSE;	/* [修改外接感應設備] 2022/11/22 [SAM] */
	

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "GETCARD_1742 GetCardFields CTLS INIT" );
	inTIME_UNIT_CalculateRunTimeGlobalStart("CTLS FLOW START ");
	/* 如果有卡號就直接跳走 */
	if (strlen(pobTran->srBRec.szPAN) > 0)
	{
		inDISP_DispLogAndWriteFlie("  Card Num Exist[%s]",pobTran->srBRec.szPAN);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}

	/* [修改外接感應設備] 2022/11/22 [SAM] */
	memset(szCTLSMode, 0x00, sizeof(szCTLSMode));
	inGetContactlessReaderMode(szCTLSMode);
	
	/* 過卡方式參數初始化  */
	pobTran->srBRec.uszManualBit = VS_FALSE;
	/* 初始化 ginEventCode */
	ginEventCode = -1;

	/* Send CTLS Readly for Sale Command */
	if (inCTLS_SendReadyForSale_Flow(pobTran) != VS_SUCCESS)
	{
		/* 如果Send 失敗，轉成沒感應的界面 */
		inRetVal = inFunc_GetCardFields_ICC(pobTran);

		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] Send CTLS *Error* END -----",__FILE__, __FUNCTION__, __LINE__);		
		return (inRetVal);
	}
	
	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "GETCARD_1742 Aft SendRedyForSaleFlow");
	
	/* 顯示對應交易別的感應畫面 */
	inCTLS_Decide_Display_Image(pobTran);
        
	/* [修改外接感應設備] 2022/11/22 [SAM] */
	/* 因為不使用內建時呼叫會當機 */
	if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
            !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)) )
	{
		/* 顯示虛擬LED(不閃爍)，但inCTLS_ReceiveReadyForSales(V3機型)也會顯示LED（閃爍），所以沒有實質作用 */
                inCTLS_LED_Wait_Start();
	}
	
	/* 進入畫面時先顯示金額 */
	if (pobTran->srBRec.lnTxnAmount > 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 16, VS_TRUE);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_LEFT_);
	}

	/* 設定Timeout */
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	if (pobTran->uszECRBit == VS_TRUE)
	{
		lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
	}
	else
	{
		lnTimeout = 30;
	}

	/* inSecond剩餘倒數時間 */
	inDISP_TimeoutStart(lnTimeout);

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "GETCARD_1742 Bef While Loop ");

	/* 先設定為讀卡失敗，有成功再設定回沒設定錯誤碼 2022/2/18 [SAM] */
	inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_READER_CARD_ERROR_);
    
	while (1)
	{                  
		/* （idle畫面刷卡此function不會發生） or 刷卡事件發生 */
		if (ginIdleMSRStatus == VS_TRUE || ginEventCode == _SWIPE_EVENT_)
		{
			/* [修改外接感應設備] 2022/11/22 [SAM] */
			if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
                            !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)) )
			{
                                if (gstMultiOb.stMulti_TransData.unzAlreadySend == VS_TRUE)
                                {
					inMultiFunc_Host_CallSendErrorFunc(pobTran, _ECR_RESPONSE_CODE_CTLS_HOST_CANCEL_);
                                }
			} else
			{
				/* 取消感應交易 */
				inCTLS_CancelTransacton_Flow();
			}

			inFunc_ResetTitle(pobTran);

			/* 第一步驟GetTrack123 */
			inCARD_GetTrack123(pobTran);

			/* 第二步驟unPackTrack資料 */
			if (inCARD_unPackCard(pobTran) != VS_SUCCESS)
			{
				/* Unpack失敗，再開感應太麻煩 */
				return(VS_ERROR);
			}

			/* 第三步驟 判斷card bin 讀HDT */
			if (inCARD_GetBin(pobTran) != VS_SUCCESS)
				return(VS_ERROR);

			/* 第四步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* U CARD 有自己的檢核法 */
				if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
				{
					if (inCARD_ValidTrack2_UCard_PAN(pobTran->srBRec.szPAN) != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}
			}

			/* 第六步驟檢核SeviceCode */
			if (inCheckFallbackFlag(pobTran) != VS_SUCCESS)
				return (VS_ERROR);

			/* 第七步驟檢核ExpDate */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetExpiredDateCheck(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}
			
			/* 富邦測試列印用 */
			inFunc_Print9F1FData(pobTran);
			
			/* 決定TRT */
			inRetVal = inFunc_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
                            
			/* 先設定回無錯誤狀態碼 2022/2/18 [SAM] */
			inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_NOT_SET_ERROR_);
			return (VS_SUCCESS);
		}
		/* （idle畫面人工輸入此function不會發生）or Menu Keyin */
		else if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
		{
			/* 表示是手動輸入 */
			pobTran->srBRec.uszManualBit = VS_TRUE;

			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			/* [修改外接感應設備] 2022/11/22 [SAM] */
			if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
                            !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)) )
			{
                                if (gstMultiOb.stMulti_TransData.unzAlreadySend == VS_TRUE)
                                {
					inMultiFunc_Host_CallSendErrorFunc(pobTran, _ECR_RESPONSE_CODE_CTLS_HOST_CANCEL_);
                                }
			} else
			{
				/* 取消感應交易 */
				inCTLS_CancelTransacton_Flow();
			}

			/* V3 Enter key 代碼為 'A' 所以要限制 */
			if (szKey >= '0' && szKey <= '9')
			{
				pobTran->inMenuKeyin = (int)szKey;
			}

			/* 人工輸入卡號開關 */
			/* 退貨交易及預先授權/預先授權完成交易部參考人工輸入功能開關 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetManualKeyin(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0		&&
			   (pobTran->inTransactionCode != _REFUND_		&&
			    pobTran->inTransactionCode != _INST_REFUND_		&&
			    pobTran->inTransactionCode != _REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _PRE_AUTH_		&&
			    pobTran->inTransactionCode != _PRE_COMP_		&&
			    pobTran->inTransactionCode != _REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_REFUND_		&&
			    pobTran->inTransactionCode != _CUP_INST_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_PRE_AUTH_	&&
			    pobTran->inTransactionCode != _CUP_PRE_COMP_))
			{
				inRetVal = inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

				return (inRetVal);
			}

			inRetVal = inCREDIT_Func_Get_Card_Number(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			if (inCARD_Generate_Special_Card(pobTran->srBRec.szPAN) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}

			/* 第一步驟 判斷card bin 讀HDT */
			if (inCARD_GetBin(pobTran) != VS_SUCCESS)
				return(VS_ERROR);

			/* 第二步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* U CARD 有自己的檢核法 */
				if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
				{
					if (inCARD_ValidTrack2_UCard_PAN_MenuKeyIn(pobTran->srBRec.szPAN) != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}
			}

			/* 第三步選擇輸入檢查碼或有效期 */
			/* 部份交易不能允許輸入檢查碼 */
			if (pobTran->srBRec.inCode == _SALE_		||
			    pobTran->srBRec.inCode == _CUP_SALE_	||
			    pobTran->srBRec.inCode == _INST_SALE_	||
			    pobTran->srBRec.inCode == _REDEEM_SALE_	||
			    pobTran->srBRec.inCode == _PRE_AUTH_	||
			    pobTran->srBRec.inCode == _CUP_INST_SALE_	||
			    pobTran->srBRec.inCode == _CUP_REDEEM_SALE_	||
			    pobTran->srBRec.inCode == _CUP_PRE_AUTH_)
			{
				inRetVal = inCREDIT_Func_Get_Exp_Date(pobTran);
			}
			else
			{
				inRetVal = inCREDIT_Func_Get_CheckNO_ExpDate_Flow(pobTran);
			}
			
			if (inRetVal != VS_SUCCESS)
				return (inRetVal);

			/* 第四步驟檢核ExpDate */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetExpiredDateCheck(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}

			/* 決定TRT */
			inRetVal = inFunc_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			/* 先設定回無錯誤狀態碼 2022/2/18 [SAM] */
			inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_NOT_SET_ERROR_);
			return (VS_SUCCESS);
		}
		/* （idle 插晶片卡此function不會發生）or 晶片卡事件 */
		else if  (ginIdleICCStatus == VS_TRUE || ginEventCode == _EMV_DO_EVENT_)
		{
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "GETCARD_1742 Bef Cancel Ctls Command");
			
			/* [修改外接感應設備] 2022/11/22 [SAM] */
			if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
                            !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)) )
			{
                                if (gstMultiOb.stMulti_TransData.unzAlreadySend == VS_TRUE)
                                {
					inMultiFunc_Host_CallSendErrorFunc(pobTran, _ECR_RESPONSE_CODE_CTLS_HOST_CANCEL_);
                                }
			} else
			{
				/* 取消感應交易 */
				inCTLS_CancelTransacton_Flow();
			}
			
			inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "GETCARD_1742 Aft Cancel Ctls Command");
			
			inFunc_ResetTitle(pobTran);

			inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "GETCARD_1742 Bef Get Emv Data");
			/* 取得晶片卡資料 */
			if (inEMV_GetEMVCardData(pobTran) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}
			else
			{
				inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "GETCARD_1742 Aft  Get Emv Data");
				
				if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				{
					inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "GETCARD_1742 Bef Get CardNo Flow");
					if (inEMV_GetCardNoFlow(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
					inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "GETCARD_1742 Aft Get CardNo Flow");
#ifdef _MODIFY_EMV_SELECT_AID_	
					/* 第三步驟 判斷card bin 讀HDT */
					if (inCARD_GetBin(pobTran) != VS_SUCCESS)
						return (VS_ERROR);

					/* 第四步驟檢核PAN module 10 */
					memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
					inGetModule10Check(szFuncEnable);
					if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
					{
						/* U CARD 有自己的檢核法 */
						if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
						{
							if (inCARD_ValidTrack2_UCard_PAN(pobTran->srBRec.szPAN) != VS_SUCCESS){
								inDISP_DispLogAndWriteFlie("  Emv Process Valid Ucard Pan Bin *Error* Line[%d]", __LINE__);
								return (VS_ERROR);
							}
						}
						else
						{
							if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
							{
								inDISP_DispLogAndWriteFlie("  Emv Process Valid Track2 Pan Bin *Error* Line[%d]", __LINE__);
								return (VS_ERROR);
							}
						}
					}

					/* 第五步驟檢核ExpDate */
					memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
					inGetExpiredDateCheck(szFuncEnable);
					if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
					{
						if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
							return (VS_ERROR);
					}
#endif					
				}
				/* Host、TRT在inEMV_GetEMVCardData裡面selectAID時設定 */
				/* 如果之後有不同的收單行，可能要用AID來分 (EX:AE晶片卡)*/
			}
			
			/* 富邦測試列印用 */
			inFunc_Print9F1FData(pobTran);
			
			/* 決定TRT */
			inRetVal = inFunc_Decide_ICC_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "GETCARD_1742 Aft Decide Icc Trt");
			/* 先設定回無錯誤狀態碼 2022/2/18 [SAM] */
			inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_NOT_SET_ERROR_);
			return (VS_SUCCESS);
		}
		/* 感應事件 */
		else if (ginEventCode == _SENSOR_EVENT_)
		{
                        /* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			/* [修改外接感應設備]  新增外接條件 2022/11/22 [SAM] */
			if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
                            !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)) )
			{
                                inDISP_LogPrintfWithFlag("CTLS MODE :External");
				
				inCTLS_RetVal = inMultiFunc_CallSlaveCtlsResult(pobTran);
				
				switch (inCTLS_RetVal)
				{
					case VS_SUCCESS:
						pobTran->srBRec.uszContactlessBit = VS_TRUE;

						 /* 取卡號有效期 */
						if (strlen(pobTran->szTrack2) > 0)
						{
							inRetVal = inCARD_unPackCard(pobTran);
							if (inRetVal != VS_SUCCESS)
								return (VS_CARD_PAN_ERROR);
						}
						else
						{
							return (VS_NO_CARD_BIN);
						}
						inDISP_LogPrintfWithFlag(" MultiFunc Receive Success Data");

						/* 判斷card bin 讀HDT */
						if (inCARD_GetBin(pobTran) != VS_SUCCESS)
							return (VS_NO_CARD_BIN);

						/* 檢核PAN module 10 */
						memset(szFuncEnable, 0x00, sizeof (szFuncEnable));
						inGetModule10Check(szFuncEnable);
						if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
						{
							if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
								return (VS_CARD_PAN_ERROR);
						}

						/* 檢核ExpDate */
						memset(szFuncEnable, 0x00, sizeof (szFuncEnable));
						inGetExpiredDateCheck(szFuncEnable);
						if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
						{
							if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
								return (VS_CARD_EXP_ERR);
						}

						/* 新增免簽名判斷  */
						//TODO 要看一下要不要留這個判斷
//						if (pobTran->srBRec.lnTxnAmount > 3000)
//							pobTran->srBRec.uszNoSignatureBit = VS_FALSE;
//						else
//							pobTran->srBRec.uszNoSignatureBit = VS_TRUE;

						/* 例外狀況 Tag檢核 感應限額等等 */
						if (inCTLS_ExceptionCheck_External(pobTran) != VS_SUCCESS)
						{
							return (VS_ERROR);
						}
						break;
					
					default:
						inDISP_LogPrintfWithFlag("感應失敗[ %d] Line[%d]", inRetVal, __LINE__);
						if (pobTran->inErrorMsg == _ERROR_CODE_V3_NONE_)
						{
							pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
						}

						return (VS_WAVE_ERROR);
				}

			} else
			{
				inDISP_LogPrintfWithFlag("CTLS MODE :Internal");
				/* 收到的retuen不是等待資料的狀態就去解析 */
				ulCTLS_RetVal = ulCTLS_CheckResponseCode_SALE(pobTran);

				if (ulCTLS_RetVal == d_EMVCL_RC_NO_CARD)
				{
					/* Timeout沒卡 */
					return (VS_TIMEOUT);
				}
				else if (ulCTLS_RetVal == d_EMVCL_RC_MORE_CARDS)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_NOT_ONE_CARD_;

					return (VS_WAVE_ERROR);
				}
				/* 這邊要切SmartPay */
				else if (ulCTLS_RetVal == d_EMVCL_NON_EMV_CARD)
				{
					if (pobTran->srBRec.inTxnResult != VS_SUCCESS)
						return (VS_WAVE_ERROR);

					/* 轉 FISC_SALE */
					/* FISC incode 在 inFISC_CTLSProcess內設定 */
				}
				/* 走信用卡流程 */
				else if (ulCTLS_RetVal == d_EMVCL_RC_DATA)
				{
					/* 判斷card bin 讀HDT */
					if (inCARD_GetBin(pobTran) != VS_SUCCESS)
						return (VS_NO_CARD_BIN);

					/* 檢核PAN module 10 */
					memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
					inGetModule10Check(szFuncEnable);
					if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
					{
						/* U CARD 有自己的檢核法 */
						if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
						{
							if (inCARD_ValidTrack2_UCard_PAN(pobTran->srBRec.szPAN) != VS_SUCCESS)
								return (VS_CARD_PAN_ERROR);
						}
						else
						{
							if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
								return (VS_CARD_PAN_ERROR);
						}
					}

					/* 檢核ExpDate */
					memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
					inGetExpiredDateCheck(szFuncEnable);
					if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
					{
						if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
							return (VS_CARD_EXP_ERR);
					}
				}
				/*  Two Tap 流程 */
				else if (ulCTLS_RetVal == d_EMVCL_RC_SEE_PHONE)
				{
					/* 重Send命令，等第二次感應 */
					/* Send CTLS Readly for Sale Command */
					if (inCTLS_SendReadyForSale_Flow(pobTran) != VS_SUCCESS)
						return (VS_WAVE_ERROR);

					ginEventCode = -1;
					continue;
				}
				/* 感應錯誤 */
				else
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "感應失敗,0x%08lX", ulCTLS_RetVal);
						inDISP_LogPrintf(szDebugMsg);
					}

					if (pobTran->inErrorMsg == _ERROR_CODE_V3_NONE_)
					{
						pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
					}

					return (VS_WAVE_ERROR);
				}
			}
			
			/* 富邦測試列印用 */
			inFunc_Print9F1FData(pobTran);
			
			/* 決定TRT */
			inRetVal = inFunc_Decide_CTLS_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			inFunc_ResetTitle(pobTran);
			/* 先設定回無錯誤狀態碼 2022/2/18 [SAM] */
			inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_NOT_SET_ERROR_);
			return (VS_SUCCESS);
		}
		/* 走票證流程 */
		else if (ginEventCode == _TICKET_EVENT_)
		{
			if (inMENU_Check_ETICKET_Enable(0) == VS_SUCCESS)
			{
				pobTran->inTransactionCode = _TICKET_DEDUCT_;
				pobTran->srTRec.inCode = pobTran->inTransactionCode;
				pobTran->srTRec.uszESVCTransBit = VS_TRUE;
				inFunc_ResetTitle(pobTran);
				inRetVal = inFLOW_RunOperation(pobTran, _OPERATION_TICKET_DEDUCT_);

				return (inRetVal);
			}
			else
			{
				/* 沒開票證，不做反應 */
			}
		}
                /* 20230206 Miyano add Costco掃碼付流程 */
                else if (ginEventCode == _COSTCO_SCAN_EVENT_)
		{
			pobTran->srTRec.inCode = pobTran->inTransactionCode;
			inRetVal = inFLOW_RunFunction(pobTran, _COSTCO_SCANPAY_);
                        inDISP_LogPrintf("Miyano Test inRetVal = %d", inRetVal);
			return (inRetVal);
		}
                
                
		/* 進迴圈前先清MSR BUFFER */
		inCARD_Clean_MSR_Buffer();
		while (1)
		{
			ginEventCode = -1;

			/* ------------偵測刷卡------------------*/
			inMSR_RetVal = inCARD_MSREvent();

			if (inMSR_RetVal == VS_SUCCESS)
			{
				/* 刷卡事件 */
				ginEventCode = _SWIPE_EVENT_;
			}
			/* 回復錯誤訊息蓋掉的圖 */
			else if (inMSR_RetVal == VS_SWIPE_ERROR)
			{
				inFunc_ResetTitle(pobTran);
				inDISP_Msg_BMP(_ERR_READ_CARD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);

				/* 顯示對應交易別的感應畫面 */
				inCTLS_Decide_Display_Image(pobTran);

				/* 回復虛擬燈號 */
				EMVCL_ShowVirtualLED(NULL);

				/* 回復顯示金額 */
				if (pobTran->srBRec.lnTxnAmount > 0)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
					inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 16, VS_TRUE);
					inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_LEFT_);
				}

				/* 抓取 KioskFlag的值，如為 TRUE 就不用確認畫面 2020/3/6 下午 4:28 [SAM] */
				if(inFunc_GetKisokFlag() == VS_TRUE)
				{	
					inDISP_LogPrintfWithFlag(" FU Kiosk GetCardFields Ctls Line[%d]", __LINE__);
					inCTLS_CancelTransacton_Flow();
					/* Timeout */
					return (VS_TIMEOUT);
				}
			}

			/* ------------偵測晶片卡---------------- */
			inEMV_RetVal = inEMV_ICCEvent();
			if (inEMV_RetVal == VS_SUCCESS)
			{
				/* 晶片卡事件 */
				ginEventCode = _EMV_DO_EVENT_;
			}

			/* ------------偵測感應卡------------------ */
			inCTLS_RetVal = inCTLS_ReceiveReadyForSales_Flow(pobTran);
			inDISP_LogPrintfWithFlag(" FU GetCardFields inCTLS_RetVal[%d] Line[%d]", inCTLS_RetVal, __LINE__);
			
			/* [修改外接感應設備] 2022/11/22 [SAM] */
			if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
                            !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)) )
			{
				if (inCTLS_RetVal != VS_WAVE_NO_DATA)
				{	
					/* 感應卡事件 */
					ginEventCode = _SENSOR_EVENT_;					
				}
			} else
			{
				if (inCTLS_RetVal == VS_SUCCESS)
				{
					/* 感應卡事件 */
					ginEventCode = _SENSOR_EVENT_;
				}
			}
			
			
			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 感應倒數時間 && Display Countdown */
			if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
			{
				/* 感應時間到Timeout */
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
				/* [修改外接感應設備] 2022/11/22 [SAM] */
				if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
                                    !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)))
				{
                                        if (gstMultiOb.stMulti_TransData.unzAlreadySend == VS_TRUE)
						inMultiFunc_Host_CallSendErrorFunc(pobTran, _ECR_RESPONSE_CODE_CTLS_HOST_CANCEL_);
				} else
				{
					/* 取消感應交易 */
					inCTLS_CancelTransacton_Flow();
				}
				/* Timeout */
				inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_TIMEOUT_);
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
                                /* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
				/* [修改外接感應設備] 2022/11/22 [SAM] */
				if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
                                    !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)) )
				{
                                        if (gstMultiOb.stMulti_TransData.unzAlreadySend == VS_TRUE)
                                        {
                                                inMultiFunc_Host_CallSendErrorFunc(pobTran, _ECR_RESPONSE_CODE_CTLS_HOST_CANCEL_);
                                        }
				} else
				{
					/* 取消感應交易 */
					inCTLS_CancelTransacton_Flow();
				}
				/* Cancel */
				return (VS_USER_CANCEL);
			}
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* key in事件 */
				ginEventCode = _MENUKEYIN_EVENT_;
			}
			else if (szKey == _KEY_DOT_)
			{
				/* 分期、紅利、銀聯、HappyGo混合 不能轉票證 */
				if (pobTran->srBRec.uszInstallmentBit == VS_TRUE	||
					pobTran->srBRec.uszRedeemBit == VS_TRUE		||
					pobTran->srBRec.uszCUPTransBit == VS_TRUE		||
					pobTran->srBRec.uszHappyGoMulti == VS_TRUE		||
					pobTran->srBRec.inCode == _PRE_AUTH_)
				{

				}
				else
				{
					/* 走票證流程 */
					ginEventCode = _TICKET_EVENT_;
				}
			}
                        else if (szKey == _KEY_FUNCTION_ && vbCheckCostcoCustom(Costco_New))
			{
				/* 好市多Costco 掃碼交易 */
				ginEventCode = _COSTCO_SCAN_EVENT_;
			}
                        
			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				/* 銀聯一般交易不能輸入卡號 */
				if (pobTran->inTransactionCode == _CUP_SALE_ && ginEventCode == _MENUKEYIN_EVENT_)
					continue;
				else
					break;
			}

		}/* while (1) 偵測事件迴圈...*/
	} /* while (1) 對事件做回應迴圈...*/

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_GetCardFields_Refund_CTLS_Flow
Date&Time       :2017/6/27 下午 4:40
Describe        :感應退貨的分流
*/
int inFunc_GetCardFields_Refund_CTLS_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szFunEnable[2 + 1] = {0};

	memset(szFunEnable, 0x00, sizeof(szFunEnable));
	inGetStore_Stub_CardNo_Truncate_Enable(szFunEnable);

	if (memcmp(szFunEnable, "Y", strlen("Y")) == 0)
	{
		inRetVal = inFunc_GetCardFields_Refund_CTLS_Txno(pobTran);
	}
	else
	{
		inRetVal = inFunc_GetCardFields_Refund_CTLS(pobTran);
	}

	return (inRetVal);
}

/*
Function        :inFunc_GetCardFields_Refund_CTLS
Date&Time       :2017/6/27 下午 4:26
Describe        :進這隻必定CTLS Enable為開，感應退貨
*/
int inFunc_GetCardFields_Refund_CTLS(TRANSACTION_OBJECT *pobTran)
{
int		inRetVal = VS_ERROR;
int		inMSR_RetVal = -1;	/* 磁條事件的反應，怕和其他用到inRetVal的搞混所以獨立出來 */
int		inCTLS_RetVal = -1;	/* 感應卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
char		szKey = -1;
char		szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
char		szFuncEnable[2 + 1] = {0};
char		szCustomerIndicator[3 + 1] = {0};
long		lnTimeout = 0;
unsigned long   ulCTLS_RetVal = 0x00;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_GetCardFields_Refund_CTLS() START !");
	}

	/* 如果有卡號就直接跳走 */
	if (strlen(pobTran->srBRec.szPAN) > 0)
	{
		return (VS_SUCCESS);
	}

	/* 過卡方式參數初始化  */
	pobTran->srBRec.uszManualBit = VS_FALSE;
	/* 初始化 ginEventCode */
	ginEventCode = -1;

	/* Send CTLS Readly for Refund Command */
	if (inCTLS_SendReadyForRefund_Flow(pobTran) != VS_SUCCESS)
	{
		/* 如果Send 失敗，轉成沒感應的界面 */
		inRetVal = inFunc_GetCardFields(pobTran);
		return (inRetVal);
	}

	/* 顯示對應交易別的感應畫面 */
	inCTLS_Decide_Display_Image(pobTran);

	/* 顯示虛擬LED(不閃爍)，但inCTLS_ReceiveReadyForSales(V3機型)也會顯示LED（閃爍），所以沒有實質作用 */
        inCTLS_LED_Wait_Start();

	/* 進入畫面時先顯示金額 */
	if (pobTran->srBRec.lnTxnAmount > 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 16, VS_TRUE);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_LEFT_);
	}



	/* 設定Timeout */
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	if (pobTran->uszECRBit == VS_TRUE)
	{
		lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
	}
	else
	{
		lnTimeout = 30;
	}

	/* inSecond剩餘倒數時間 */
	inDISP_TimeoutStart(lnTimeout);

        /* 先設定為讀卡失敗，有成功再設定回沒設定錯誤碼 2022/2/18 [SAM] */
        inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_READER_CARD_ERROR_);
    
	while (1)
	{
		/* （idle畫面刷卡此function不會發生） or 刷卡事件發生 */
		if (ginIdleMSRStatus == VS_TRUE || ginEventCode == _SWIPE_EVENT_)
		{
			/* 取消感應交易 */
			inCTLS_CancelTransacton_Flow();
			inFunc_ResetTitle(pobTran);

			/* 第一步驟GetTrack123 */
			inCARD_GetTrack123(pobTran);

			/* 第二步驟unPackTrack資料 */
			if (inCARD_unPackCard(pobTran) != VS_SUCCESS)
			{
				/* Unpack失敗，再開感應太麻煩 */
				return(VS_ERROR);
			}

			/* 第三步驟 判斷card bin 讀HDT */
			if (inCARD_GetBin(pobTran) != VS_SUCCESS)
				return(VS_ERROR);

			/* 第四步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* U CARD 有自己的檢核法 */
				if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
				{
					if (inCARD_ValidTrack2_UCard_PAN(pobTran->srBRec.szPAN) != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}
			}

			/* 第六步驟檢核SeviceCode */
			if (inCheckFallbackFlag(pobTran) != VS_SUCCESS)
				return (VS_ERROR);

			/* 第七步驟檢核ExpDate */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetExpiredDateCheck(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}

			/* 決定TRT */
			inRetVal = inFunc_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
                            /* 先設定回無錯誤狀態碼 2022/2/18 [SAM] */
                            inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_NOT_SET_ERROR_);
			return (VS_SUCCESS);
		}
		/* （idle畫面人工輸入此function不會發生）or Menu Keyin */
		else if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
		{
			/* 表示是手動輸入 */
			pobTran->srBRec.uszManualBit = VS_TRUE;

			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			/* 取消感應交易 */
			inCTLS_CancelTransacton_Flow();

			/* V3 Enter key 代碼為 'A' 所以要限制 */
			if (szKey >= '0' && szKey <= '9')
			{
				pobTran->inMenuKeyin = (int)szKey;
			}

			/* 人工輸入卡號開關 */
			/* 退貨交易及預先授權/預先授權完成交易部參考人工輸入功能開關 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetManualKeyin(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0		&&
				(pobTran->inTransactionCode != _REFUND_		&&
				pobTran->inTransactionCode != _INST_REFUND_		&&
				pobTran->inTransactionCode != _REDEEM_REFUND_	&&
				pobTran->inTransactionCode != _PRE_AUTH_		&&
				pobTran->inTransactionCode != _PRE_COMP_		&&
				pobTran->inTransactionCode != _REDEEM_REFUND_	&&
				pobTran->inTransactionCode != _CUP_REFUND_		&&
				pobTran->inTransactionCode != _CUP_INST_REFUND_	&&
				pobTran->inTransactionCode != _CUP_REDEEM_REFUND_	&&
				pobTran->inTransactionCode != _CUP_PRE_AUTH_	&&
				pobTran->inTransactionCode != _CUP_PRE_COMP_))
			{
				inRetVal = inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

				return (inRetVal);
			}

			inRetVal = inCREDIT_Func_Get_Card_Number(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			if (inCARD_Generate_Special_Card(pobTran->srBRec.szPAN) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}

			/* 第一步驟 判斷card bin 讀HDT */
			if (inCARD_GetBin(pobTran) != VS_SUCCESS)
				return(VS_ERROR);

			/* 第二步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* U CARD 有自己的檢核法 */
				if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
				{
					if (inCARD_ValidTrack2_UCard_PAN_MenuKeyIn(pobTran->srBRec.szPAN) != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}
			}

			/* 第三步選擇輸入檢查碼或有效期 */
			inRetVal = inCREDIT_Func_Get_CheckNO_ExpDate_Flow(pobTran);
			if (inRetVal != VS_SUCCESS)
				return (inRetVal);

			/* 第四步驟檢核ExpDate */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetExpiredDateCheck(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}

			/* 決定TRT */
			inRetVal = inFunc_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
                            /* 先設定回無錯誤狀態碼 2022/2/18 [SAM] */
                            inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_NOT_SET_ERROR_);
			return (VS_SUCCESS);
		}
		/* 感應事件 */
		else if (ginEventCode == _SENSOR_EVENT_)
		{
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			/* 收到的retuen不是等待資料的狀態就去解析 */
			ulCTLS_RetVal = ulCTLS_CheckResponseCode_Refund(pobTran);

			if (ulCTLS_RetVal == d_EMVCL_RC_NO_CARD)
			{
				/* Timeout沒卡 */
				return (VS_ERROR);
			}
			else if (ulCTLS_RetVal == d_EMVCL_RC_MORE_CARDS)
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_NOT_ONE_CARD_;

				return (VS_WAVE_ERROR);
			}
			/* 這邊要切SmartPay */
			else if (ulCTLS_RetVal == d_EMVCL_NON_EMV_CARD)
			{
				if (pobTran->srBRec.inTxnResult != VS_SUCCESS)
					return (VS_ERROR);

				/* 轉 FISC_Refund */
				/* FISC incode 在 inFISC_CTLSProcess內設定 */
			}
			/* 走信用卡流程 */
			else if (ulCTLS_RetVal == d_EMVCL_RC_DATA)
			{
				/* 判斷card bin 讀HDT */
				if (inCARD_GetBin(pobTran) != VS_SUCCESS)
					return (VS_ERROR);

				/* 檢核PAN module 10 */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetModule10Check(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					/* U CARD 有自己的檢核法 */
					if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
					{
						if (inCARD_ValidTrack2_UCard_PAN(pobTran->srBRec.szPAN) != VS_SUCCESS)
							return (VS_ERROR);
					}
					else
					{
						if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
							return (VS_ERROR);
					}
				}

				/* 檢核ExpDate */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetExpiredDateCheck(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}
			}
			/*  Two Tap 流程 */
			else if (ulCTLS_RetVal == d_EMVCL_RC_SEE_PHONE)
			{
				/* 重Send命令，等第二次感應 */
				/* Send CTLS Readly for Refund Command */
				if (inCTLS_SendReadyForRefund_Flow(pobTran) != VS_SUCCESS)
				return (VS_ERROR);

				ginEventCode = -1;
				continue;
			}
			/* 感應錯誤 */
			else
			{
				if (pobTran->inErrorMsg == _ERROR_CODE_V3_NONE_)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
				}

				return (VS_ERROR);
			}

			/* 決定TRT */
			inRetVal = inFunc_Decide_CTLS_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			inFunc_ResetTitle(pobTran);
                            /* 先設定回無錯誤狀態碼 2022/2/18 [SAM] */
                            inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_NOT_SET_ERROR_);
			return (VS_SUCCESS);
		}
                /* 20230206 Miyano add Costco掃碼付 */
                else if (ginEventCode == _COSTCO_SCAN_EVENT_)
		{
			pobTran->srTRec.inCode = pobTran->inTransactionCode;
			inRetVal = inFLOW_RunFunction(pobTran, _COSTCO_SCANPAY_);
                        inDISP_LogPrintf("Miyano Test inRetVal = %d", inRetVal);
			return (inRetVal);
		}

		/* 進迴圈前先清MSR BUFFER */
		inCARD_Clean_MSR_Buffer();
		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測刷卡------------------*/
			inMSR_RetVal = inCARD_MSREvent();
			if (inMSR_RetVal == VS_SUCCESS)
			{
				/* 刷卡事件 */
				ginEventCode = _SWIPE_EVENT_;
			}
			/* 回復錯誤訊息蓋掉的圖 */
			else if (inMSR_RetVal == VS_SWIPE_ERROR)
			{
				inFunc_ResetTitle(pobTran);
				inDISP_Msg_BMP(_ERR_READ_CARD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);

				/* 顯示對應交易別的感應畫面 */
				inCTLS_Decide_Display_Image(pobTran);

				/* 回復虛擬燈號 */
				EMVCL_ShowVirtualLED(NULL);

				/* 回復顯示金額 */
				if (pobTran->srBRec.lnTxnAmount > 0)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
					inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 16, VS_TRUE);
					inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_LEFT_);
				}

			}

			/* ------------偵測感應卡------------------ */
			inCTLS_RetVal = inCTLS_ReceiveReadyForRefund_Flow(pobTran);
			if (inCTLS_RetVal == VS_SUCCESS)
			{
				/* 感應卡事件 */
				ginEventCode = _SENSOR_EVENT_;
			}


			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 感應倒數時間 && Display Countdown */
			if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
			{
				/* 感應時間到Timeout */
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
				inCTLS_CancelTransacton_Flow();
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
				inCTLS_CancelTransacton_Flow();
				/* Cancel */
				return (VS_USER_CANCEL);
			}
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* key in事件 */
				ginEventCode = _MENUKEYIN_EVENT_;
			}
                        else if (szKey == _KEY_FUNCTION_ && vbCheckCostcoCustom(Costco_New))
			{
				/* 好市多Costco 掃碼交易 */
				ginEventCode = _COSTCO_SCAN_EVENT_;
			}

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}

		}/* while (1) 偵測事件迴圈...*/


	} /* while (1) 對事件做回應迴圈...*/

	return (VS_SUCCESS);
}

/*
Function        :inFunc_GetCardFields_Refund_CTLS_Txno
Date&Time       :2017/6/27 下午 4:26
Describe        :進這隻必定CTLS Enable為開，感應退貨加交易編號
*/
int inFunc_GetCardFields_Refund_CTLS_Txno(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inMSR_RetVal = -1;	/* 磁條事件的反應，怕和其他用到inRetVal的搞混所以獨立出來 */
	int		inCTLS_RetVal = -1;	/* 感應卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
        char		szKey = -1;
        char		szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
	char		szFuncEnable[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	long		lnTimeout = 0;
        unsigned long   ulCTLS_RetVal = 0;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_GetCardFields_Refund_CTLS_Txno() START !");
	}

	/* 如果有卡號就直接跳走 */
	if (strlen(pobTran->srBRec.szPAN) > 0)
	{
		return (VS_SUCCESS);
	}

        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;
	/* 初始化 ginEventCode */
        ginEventCode = -1;

	/* 顯示對應交易別的感應畫面 */
	inCTLS_Decide_Display_Image(pobTran);

	/* 顯示虛擬LED(不閃爍)，但inCTLS_ReceiveReadyForSales(V3機型)也會顯示LED（閃爍），所以沒有實質作用 */
        inCTLS_LED_Wait_Start();

        /* 進入畫面時先顯示金額 */
	if (pobTran->srBRec.lnTxnAmount > 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 16, VS_TRUE);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_LEFT_);
	}

	/* Send CTLS Readly for Refund Command */
	if (inCTLS_SendReadyForRefund_Flow(pobTran) != VS_SUCCESS)
	{
		/* 如果Send 失敗，轉成沒感應的界面 */
		inRetVal = inFunc_GetCardFields_Txno(pobTran);
		return (inRetVal);
	}

        /* 設定Timeout */
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, strlen(_CUSTOMER_INDICATOR_107_BUMPER_)))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}

	/* 剩餘倒數時間 開始 */
	inDISP_TimeoutStart(lnTimeout);

        /* 先設定為讀卡失敗，有成功再設定回沒設定錯誤碼 2022/2/18 [SAM] */
        inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_READER_CARD_ERROR_);
    
	while (1)
        {
                /* （idle畫面刷卡此function不會發生） or 刷卡事件發生 */
	        if (ginIdleMSRStatus == VS_TRUE || ginEventCode == _SWIPE_EVENT_)
	        {
                        /* 取消感應交易 */
                        inCTLS_CancelTransacton_Flow();
                        inFunc_ResetTitle(pobTran);

	                /* 第一步驟GetTrack123 */
	                inCARD_GetTrack123(pobTran);

	                /* 第二步驟unPackTrack資料 */
	                if (inCARD_unPackCard(pobTran) != VS_SUCCESS)
	                {
				/* Unpack失敗，再開感應太麻煩 */
	                        return(VS_ERROR);
	                }

                        /* 第三步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);

                        /* 第四步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* U CARD 有自己的檢核法 */
				if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
				{
					if (inCARD_ValidTrack2_UCard_PAN(pobTran->srBRec.szPAN) != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}
			}

	                /* 第六步驟檢核SeviceCode */
                        if (inCheckFallbackFlag(pobTran) != VS_SUCCESS)
	                        return (VS_ERROR);

	                /* 第七步驟檢核ExpDate */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetExpiredDateCheck(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}

			/* 決定TRT */
			inRetVal = inFunc_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
                        /* 先設定回無錯誤狀態碼 2022/2/18 [SAM] */
                            inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_NOT_SET_ERROR_);
                        return (VS_SUCCESS);
	        }
                /* （idle畫面人工輸入此function不會發生）or Menu Keyin */
                else if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
                {
			/* 表示是手動輸入 */
                        pobTran->srBRec.uszManualBit = VS_TRUE;

			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			/* 取消感應交易 */
                        inCTLS_CancelTransacton_Flow();

			/* 人工輸入卡號開關 */
			/* 退貨交易及預先授權/預先授權完成交易部參考人工輸入功能開關 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetManualKeyin(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0		&&
			   (pobTran->inTransactionCode != _REFUND_		&&
			    pobTran->inTransactionCode != _INST_REFUND_		&&
			    pobTran->inTransactionCode != _REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _PRE_AUTH_		&&
			    pobTran->inTransactionCode != _PRE_COMP_		&&
			    pobTran->inTransactionCode != _REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_REFUND_		&&
			    pobTran->inTransactionCode != _CUP_INST_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_PRE_AUTH_	&&
			    pobTran->inTransactionCode != _CUP_PRE_COMP_))
			{
				inRetVal = inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_4_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

				return (inRetVal);
			}

			inRetVal = inCREDIT_Func_Get_Card_Number_Txno_Flow(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			if (inCARD_Generate_Special_Card(pobTran->srBRec.szPAN) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}

                        /* 第一步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);

			/* 第二步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* U CARD 有自己的檢核法 */
				if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
				{
					if (inCARD_ValidTrack2_UCard_PAN_MenuKeyIn(pobTran->srBRec.szPAN) != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}
			}

                        /* 第三步選擇輸入檢查碼或有效期 */
                        inRetVal = inCREDIT_Func_Get_CheckNO_ExpDate_Flow(pobTran);
			if (inRetVal != VS_SUCCESS)
				return (inRetVal);

                        /* 第四步驟檢核ExpDate */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetExpiredDateCheck(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}

			/* 決定TRT */
			inRetVal = inFunc_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
                        /* 先設定回無錯誤狀態碼 2022/2/18 [SAM] */
                            inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_NOT_SET_ERROR_);
                        return (VS_SUCCESS);
                }
		/* 感應事件 */
		else if (ginEventCode == _SENSOR_EVENT_)
		{
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			/* 收到的retuen不是等待資料的狀態就去解析 */
			ulCTLS_RetVal = ulCTLS_CheckResponseCode_Refund(pobTran);

			if (ulCTLS_RetVal == d_EMVCL_RC_NO_CARD)
                        {
				/* Timeout沒卡 */
				return (VS_ERROR);
			}
			else if (ulCTLS_RetVal == d_EMVCL_RC_MORE_CARDS)
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_NOT_ONE_CARD_;

				return (VS_WAVE_ERROR);
			}
                        /* 這邊要切SmartPay */
                        else if (ulCTLS_RetVal == d_EMVCL_NON_EMV_CARD)
                        {
				if (pobTran->srBRec.inTxnResult != VS_SUCCESS)
					 return (VS_ERROR);

				/* 轉 FISC_Refund */
				/* FISC incode 在 inFISC_CTLSProcess內設定 */
                        }
                        /* 走信用卡流程 */
                        else if (ulCTLS_RetVal == d_EMVCL_RC_DATA)
                        {
				/* 判斷card bin 讀HDT */
				if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                       return (VS_ERROR);

				/* 檢核PAN module 10 */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetModule10Check(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					/* U CARD 有自己的檢核法 */
					if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
					{
						if (inCARD_ValidTrack2_UCard_PAN(pobTran->srBRec.szPAN) != VS_SUCCESS)
							return (VS_ERROR);
					}
					else
					{
						if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
							return (VS_ERROR);
					}
				}

				/* 檢核ExpDate */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetExpiredDateCheck(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}
                        }
			/*  Two Tap 流程 */
			else if (ulCTLS_RetVal == d_EMVCL_RC_SEE_PHONE)
			{
				/* 重Send命令，等第二次感應 */
				/* Send CTLS Readly for Refund Command */
				if (inCTLS_SendReadyForRefund_Flow(pobTran) != VS_SUCCESS)
					return (VS_ERROR);

				ginEventCode = -1;
				continue;
			}
			/* 感應錯誤 */
			else
			{
				if (pobTran->inErrorMsg == _ERROR_CODE_V3_NONE_)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
				}

				return (VS_ERROR);
			}

			/* 決定TRT */
			inRetVal = inFunc_Decide_CTLS_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			inFunc_ResetTitle(pobTran);
                        /* 先設定回無錯誤狀態碼 2022/2/18 [SAM] */
                            inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_NOT_SET_ERROR_);
			return (VS_SUCCESS);
		}
                /* 20230206 Miyano 新增Costco掃碼付 */
                else if (ginEventCode == _COSTCO_SCAN_EVENT_)
		{
			pobTran->srTRec.inCode = pobTran->inTransactionCode;
			inRetVal = inFLOW_RunFunction(pobTran, _COSTCO_SCANPAY_);
                        inDISP_LogPrintf("Miyano Test inRetVal = %d", inRetVal);
			return (inRetVal);
		}

		/* 進迴圈前先清MSR BUFFER */
		inCARD_Clean_MSR_Buffer();
		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測刷卡------------------*/
			inMSR_RetVal = inCARD_MSREvent();
			if (inMSR_RetVal == VS_SUCCESS)
			{
				/* 刷卡事件 */
				ginEventCode = _SWIPE_EVENT_;
			}
			/* 回復錯誤訊息蓋掉的圖 */
			else if (inMSR_RetVal == VS_SWIPE_ERROR)
			{
				inFunc_ResetTitle(pobTran);
				inDISP_Msg_BMP(_ERR_READ_CARD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);

				/* 顯示對應交易別的感應畫面 */
				inCTLS_Decide_Display_Image(pobTran);

				/* 回復虛擬燈號 */
				EMVCL_ShowVirtualLED(NULL);

				/* 回復顯示金額 */
				if (pobTran->srBRec.lnTxnAmount > 0)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
					inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 16, VS_TRUE);
					inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_LEFT_);
				}

			}

			/* ------------偵測感應卡------------------ */
			inCTLS_RetVal = inCTLS_ReceiveReadyForRefund_Flow(pobTran);
			if (inCTLS_RetVal == VS_SUCCESS)
			{
				/* 感應卡事件 */
				ginEventCode = _SENSOR_EVENT_;
			}


			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 感應倒數時間 && Display Countdown */
			if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
			{
				/* 感應時間到Timeout */
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
				inCTLS_CancelTransacton_Flow();
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
				inCTLS_CancelTransacton_Flow();
				/* Cancel */
				return (VS_USER_CANCEL);
			}
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* key in事件 人工輸入請按0*/
				if (szKey == '0')
				{
					ginEventCode = _MENUKEYIN_EVENT_;
				}
			}
                        else if (szKey == _KEY_FUNCTION_ && vbCheckCostcoCustom(Costco_New))
			{
				/* 好市多Costco 掃碼交易 */
				ginEventCode = _COSTCO_SCAN_EVENT_;
			}
            
			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				/* 銀聯一般交易不能輸入卡號 */
				if (pobTran->inTransactionCode == _CUP_SALE_ && ginEventCode == _MENUKEYIN_EVENT_)
					continue;
				else
					break;
			}

		}/* while (1) 偵測事件迴圈...*/


	} /* while (1) 對事件做回應迴圈...*/

        return (VS_SUCCESS);
}

/*
Function        :inFunc_GetCardFields_FISC
Date&Time       :2017/4/5 上午 11:43
Describe        :SmartPay選單進入使用 只能用晶片
*/
int inFunc_GetCardFields_FISC(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	int	inEMV_RetVal = -1;	/* 晶片卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
        char	szKey = 0;
	char	szCustomerIndicator[3 + 1] = {0};
	long	lnTimeout = 0;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_GetCardFields_FISC() START !");
	}

	/* 如果有卡號就直接跳走 */
	if (strlen(pobTran->srBRec.szPAN) > 0)
	{
		return (VS_SUCCESS);
	}

        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;

        /* 如果ginEventCode為數字鍵流程即為IDLE MenuKeyIn */
        if (ginEventCode >= '0' && ginEventCode <= '9')
        {
                /* 表示是idle手動輸入，這邊才on flag是因為有可能idle輸入金額 */
                ginMenuKeyIn = VS_TRUE;
        }

	/* 設定Timeout */
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, strlen(_CUSTOMER_INDICATOR_107_BUMPER_)))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}

	inDISP_Timer_Start(_TIMER_GET_CARD_, lnTimeout);

        while (1)
        {
                /* 插晶片卡 */
                if  (ginEventCode == _EMV_DO_EVENT_)
                {
                        /* 取得晶片卡資料 */
                        if (inEMV_GetEMVCardData(pobTran) != VS_SUCCESS)
                                return (VS_ERROR);
                        else
                        {
                                /* SmartPay晶片卡 */
                                if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
                                {
                                        inDISP_Msg_BMP(_ERR_NOT_SUP_CARD_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
                                        return (VS_ERROR);
                                }

                                pobTran->srBRec.inHDTIndex = 0;
                        }

			/* 決定TRT */
			inRetVal = inFunc_Decide_ICC_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

                        return (VS_SUCCESS);
                }


		/* 顯示請刷卡或插卡 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_GET_FISC_CARD_, 0, _COORDINATE_Y_LINE_8_4_);

		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測晶片卡---------------- */
			inEMV_RetVal = inEMV_ICCEvent();
			if (inEMV_RetVal == VS_SUCCESS)
			{
				/* 晶片卡事件 */
				ginEventCode = _EMV_DO_EVENT_;
			}

			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 檢查TIMEOUT */
			if (inTimerGet(_TIMER_GET_CARD_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* Cancel */
				return (VS_USER_CANCEL);
			}
			/* ManuKeyin 為數字鍵時 */
			else if(szKey >= '0' && szKey <= '9')
			{
				/* 清鍵盤buffer */
				inFlushKBDBuffer();
			}

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}

		}/* while (1) 偵測事件迴圈...*/

	}/* while (1) 對事件做回應迴圈...*/

}
/*
Function        :inFunc_GetCardFields_FISC_CTLS
Date&Time       :2017/9/8 下午 2:16
Describe        :SmartPay選單進入 感應
*/
int inFunc_GetCardFields_FISC_CTLS(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inEMV_RetVal = -1;	/* 晶片卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
	int		inCTLS_RetVal = -1;	/* 感應卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
	long		lnTimeout = 0;
	char		szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
        char		szKey = 0;
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned long   ulCTLS_RetVal = 0;

        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;

        /* 如果ginEventCode為數字鍵流程即為IDLE MenuKeyIn */
        if (ginEventCode >= '0' && ginEventCode <= '9')
        {
                /* 表示是idle手動輸入，這邊才on flag是因為有可能idle輸入金額 */
                ginMenuKeyIn = VS_TRUE;
        }

	/* 顯示請插金融卡或感應 */
	inDISP_ClearAll();
	inDISP_PutGraphic(_MENU_FISC_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_);	/* 第二層顯示 ＜消費扣款＞ */
	inDISP_PutGraphic(_CTLS_FISCICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);		/* 顯示請插金融卡或感應 */

	/* 顯示虛擬LED(不閃爍)，但inCTLS_ReceiveReadyForSales(V3機型)也會顯示LED（閃爍），所以沒有實質作用 */
        inCTLS_LED_Wait_Start();

        /* 進入畫面時先顯示金額 */
	if (pobTran->srBRec.lnTxnAmount > 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 16, VS_TRUE);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_LEFT_);
	}

	/* Send CTLS Readly for Sale Command */
	if (inCTLS_SendReadyForSale_Flow(pobTran) != VS_SUCCESS)
	{
		/* 如果Send 失敗，轉成沒感應的界面 */
		inRetVal = inFunc_GetCardFields_FISC(pobTran);
		return (inRetVal);
	}

	/* 設定Timeout */
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, strlen(_CUSTOMER_INDICATOR_107_BUMPER_)))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}

	/* inSecond剩餘倒數時間 */
	inDISP_TimeoutStart(lnTimeout);

        while (1)
        {
                /* 插晶片卡 */
                if  (ginEventCode == _EMV_DO_EVENT_)
                {
			/* 取消感應交易 */
                        inCTLS_CancelTransacton_Flow();

                        /* 取得晶片卡資料 */
                        if (inEMV_GetEMVCardData(pobTran) != VS_SUCCESS)
			{
                                return (VS_ERROR);
			}
                        else
                        {
                                /* SmartPay晶片卡 */
                                if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
                                {
                                        inDISP_Msg_BMP(_ERR_NOT_SUP_CARD_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
                                        return (VS_ERROR);
                                }

                                pobTran->srBRec.inHDTIndex = 0;
                        }

			/* 決定TRT */
			inRetVal = inFunc_Decide_ICC_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

                        return (VS_SUCCESS);
                }
		/* 感應事件 */
		else if (ginEventCode == _SENSOR_EVENT_)
		{
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			/* 收到的retuen不是等待資料的狀態就去解析 */
			ulCTLS_RetVal = ulCTLS_CheckResponseCode_SALE(pobTran);

			if (ulCTLS_RetVal == d_EMVCL_RC_NO_CARD)
                        {
				/* Timeout沒卡 */
				return (VS_ERROR);
			}
			else if (ulCTLS_RetVal == d_EMVCL_RC_MORE_CARDS)
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_NOT_ONE_CARD_;

				return (VS_WAVE_ERROR);
			}
                        /* 這邊要切SmartPay */
                        else if (ulCTLS_RetVal == d_EMVCL_NON_EMV_CARD)
                        {
				if (pobTran->srBRec.inTxnResult != VS_SUCCESS)
					 return (VS_ERROR);

				/* 轉 FISC_Refund */
				/* FISC incode 在 inFISC_CTLSProcess內設定 */
                        }
                        /* 走信用卡流程 */
                        else if (ulCTLS_RetVal == d_EMVCL_RC_DATA)
                        {
				/* SmartPay卡 */
                                if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
                                {
                                        inDISP_Msg_BMP(_ERR_NOT_SUP_CARD_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
                                        return (VS_ERROR);
                                }
                        }
			/*  Two Tap 流程 */
			else if (ulCTLS_RetVal == d_EMVCL_RC_SEE_PHONE)
			{
				/* 重Send命令，等第二次感應 */
				/* Send CTLS Readly for Refund Command */
				if (inCTLS_SendReadyForRefund_Flow(pobTran) != VS_SUCCESS)
					return (VS_ERROR);

				ginEventCode = -1;
				continue;
			}
			/* 感應錯誤 */
			else
			{
				if (pobTran->inErrorMsg == _ERROR_CODE_V3_NONE_)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
				}
				return (VS_WAVE_ERROR);
			}

			/* 決定TRT */
			inRetVal = inFunc_Decide_CTLS_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			inFunc_ResetTitle(pobTran);

			return (VS_SUCCESS);
		}

		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測晶片卡---------------- */
			inEMV_RetVal = inEMV_ICCEvent();
			if (inEMV_RetVal == VS_SUCCESS)
			{
				/* 晶片卡事件 */
				ginEventCode = _EMV_DO_EVENT_;
			}

			/* ------------偵測感應卡------------------ */
			inCTLS_RetVal = inCTLS_ReceiveReadyForSales_Flow(pobTran);
			if (inCTLS_RetVal == VS_SUCCESS)
			{
				/* 感應卡事件 */
				ginEventCode = _SENSOR_EVENT_;
			}

			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 感應倒數時間 && Display Countdown */
			if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
			{
				/* 感應時間到Timeout */
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* Cancel */
				return (VS_USER_CANCEL);
			}
			/* ManuKeyin 為數字鍵時 */
			else if(szKey >= '0' && szKey <= '9')
			{
				/* 清鍵盤buffer */
				inFlushKBDBuffer();
			}

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}

		}/* while (1) 偵測事件迴圈...*/

	}/* while (1) 對事件做回應迴圈...*/

}

/*
Function        :inFunc_GetCardFields_FISC_CTLS_Refund
Date&Time       :2017/9/8 下午 2:16
Describe        :SmartPay選單進入 感應退貨
*/
int inFunc_GetCardFields_FISC_CTLS_Refund(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inEMV_RetVal = -1;	/* 晶片卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
	int		inCTLS_RetVal = -1;	/* 感應卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
	long		lnTimeout = 0;
	char		szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
        char		szKey = 0;
	unsigned long   ulCTLS_RetVal = 0;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_GetCardFields_FISC_CTLS_Refund() START !");
	}

	/* 如果有卡號就直接跳走 */
	if (strlen(pobTran->srBRec.szPAN) > 0)
	{
		return (VS_SUCCESS);
	}

        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;

        /* 如果ginEventCode為數字鍵流程即為IDLE MenuKeyIn */
        if (ginEventCode >= '0' && ginEventCode <= '9')
        {
                /* 表示是idle手動輸入，這邊才on flag是因為有可能idle輸入金額 */
                ginMenuKeyIn = VS_TRUE;
        }

	/* 顯示請插金融卡或感應 */
	inDISP_ClearAll();
	inDISP_PutGraphic(_MENU_FISC_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_);	/* 第二層顯示 ＜退費交易＞ */
	inDISP_PutGraphic(_CTLS_FISCICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);		/* 顯示請插金融卡或感應 */

	/* 顯示虛擬LED(不閃爍)，但inCTLS_ReceiveReadyForSales(V3機型)也會顯示LED（閃爍），所以沒有實質作用 */
        inCTLS_LED_Wait_Start();

        /* 進入畫面時先顯示金額 */
	if (pobTran->srBRec.lnTxnAmount > 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 16, VS_TRUE);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_LEFT_);
	}

	/* Send CTLS Readly for Refund Command */
	if (inCTLS_SendReadyForRefund_Flow(pobTran) != VS_SUCCESS)
	{
		/* 如果Send 失敗，轉成沒感應的界面 */
		inRetVal = inFunc_GetCardFields_FISC(pobTran);
		return (inRetVal);
	}

	/* 設定Timeout */
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, strlen(_CUSTOMER_INDICATOR_107_BUMPER_)))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}

	/* inSecond剩餘倒數時間 */
	inDISP_TimeoutStart(lnTimeout);

        while (1)
        {
                /* 插晶片卡 */
                if  (ginEventCode == _EMV_DO_EVENT_)
                {
			/* 取消感應交易 */
                        inCTLS_CancelTransacton_Flow();

                        /* 取得晶片卡資料 */
                        if (inEMV_GetEMVCardData(pobTran) != VS_SUCCESS)
			{
                                return (VS_ERROR);
			}
                        else
                        {
                                /* SmartPay晶片卡 */
                                if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
                                {
                                        inDISP_Msg_BMP(_ERR_NOT_SUP_CARD_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
                                        return (VS_ERROR);
                                }

                                pobTran->srBRec.inHDTIndex = 0;
                        }

			/* 決定TRT */
			inRetVal = inFunc_Decide_ICC_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

                        return (VS_SUCCESS);
                }
		/* 感應事件 */
		else if (ginEventCode == _SENSOR_EVENT_)
		{
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			/* 收到的retuen不是等待資料的狀態就去解析 */
			ulCTLS_RetVal = ulCTLS_CheckResponseCode_Refund(pobTran);

			if (ulCTLS_RetVal == d_EMVCL_RC_NO_CARD)
                        {
				/* Timeout沒卡 */
				return (VS_ERROR);
			}
			else if (ulCTLS_RetVal == d_EMVCL_RC_MORE_CARDS)
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_NOT_ONE_CARD_;

				return (VS_WAVE_ERROR);
			}
                        /* 這邊要切SmartPay */
                        else if (ulCTLS_RetVal == d_EMVCL_NON_EMV_CARD)
                        {
				if (pobTran->srBRec.inTxnResult != VS_SUCCESS)
					 return (VS_ERROR);

				/* 轉 FISC_Refund */
				/* FISC incode 在 inFISC_CTLSProcess內設定 */
                        }
                        /* 走信用卡流程 */
                        else if (ulCTLS_RetVal == d_EMVCL_RC_DATA)
                        {
				/* SmartPay卡 */
                                if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
                                {
                                        inDISP_Msg_BMP(_ERR_NOT_SUP_CARD_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
                                        return (VS_ERROR);
                                }
                        }
			/*  Two Tap 流程 */
			else if (ulCTLS_RetVal == d_EMVCL_RC_SEE_PHONE)
			{
				/* 重Send命令，等第二次感應 */
				/* Send CTLS Readly for Refund Command */
				if (inCTLS_SendReadyForRefund_Flow(pobTran) != VS_SUCCESS)
					return (VS_ERROR);

				ginEventCode = -1;
				continue;
			}
			/* 感應錯誤 */
			else
			{
				if (pobTran->inErrorMsg == _ERROR_CODE_V3_NONE_)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
				}
				return (VS_ERROR);
			}

			/* 決定TRT */
			inRetVal = inFunc_Decide_CTLS_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			inFunc_ResetTitle(pobTran);

			return (VS_SUCCESS);
		}

		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測晶片卡---------------- */
			inEMV_RetVal = inEMV_ICCEvent();
			if (inEMV_RetVal == VS_SUCCESS)
			{
				/* 晶片卡事件 */
				ginEventCode = _EMV_DO_EVENT_;
			}

			/* ------------偵測感應卡------------------ */
			inCTLS_RetVal = inCTLS_ReceiveReadyForRefund_Flow(pobTran);
			if (inCTLS_RetVal == VS_SUCCESS)
			{
				/* 感應卡事件 */
				ginEventCode = _SENSOR_EVENT_;
			}

			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 感應倒數時間 && Display Countdown */
			if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
			{
				/* 感應時間到Timeout */
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* Cancel */
				return (VS_USER_CANCEL);
			}
			/* ManuKeyin 為數字鍵時 */
			else if(szKey >= '0' && szKey <= '9')
			{
				/* 清鍵盤buffer */
				inFlushKBDBuffer();
			}

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}

		}/* while (1) 偵測事件迴圈...*/

	}/* while (1) 對事件做回應迴圈...*/

}

/*
Function        :inFunc_GetCardFields_Loyalty_Redeem_Swipe
Date&Time       :2017/1/20 下午 4:29
Describe        :優惠兌換顯示刷卡及請輸入卡號，僅手動輸入檢核卡號，刷卡不檢核
*/
int inFunc_GetCardFields_Loyalty_Redeem_Swipe(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	int	inNCCCIndex = -1;
	int	inMSR_RetVal = -1;	/* 磁條事件的反應，怕和其他用到inRetVal的搞混所以獨立出來 */
        char	szKey = 0;
	char	szBatchNum [6 + 1] = {0}, szInvoiceNum [6 + 1] = {0}, szSTANNum [6 + 1] = {0};
	char	szFuncEnable[2 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	long	lnTimeout = 0;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_GetCardFields_Loyalty_Redeem_Swipe() START !");
	}

	/* 如果有卡號就直接跳走 */
	if (strlen(pobTran->srBRec.szPAN) > 0)
	{
		return (VS_SUCCESS);
	}

        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 顯示請刷卡或輸入卡號 */
	inDISP_PutGraphic(_GET_CARD_AWARD_, 0, _COORDINATE_Y_LINE_8_4_);

	/* 設定Timeout */
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, strlen(_CUSTOMER_INDICATOR_107_BUMPER_)))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}

	inDISP_Timer_Start(_TIMER_GET_CARD_, lnTimeout);

        while (1)
        {
                /* idle 刷卡 或 一般刷卡 */
	        if (ginIdleMSRStatus == VS_TRUE  || ginEventCode == _SWIPE_EVENT_)
	        {
	                /* 第一步驟GetTrack123 */
	                inCARD_GetTrack123(pobTran);

	                /* 第二步驟unPackTrack資料 */
	                if (inCARD_unPackCard(pobTran) != VS_SUCCESS)
	                {
				/* Unpack失敗，把flag關掉可以再手動輸入 */
	                        ginIdleMSRStatus = VS_FALSE;
	                        continue;
	                }

                        /* 第三步驟 不讀CDT 直接指定Host */
			inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_MAINLY_USE_, &inNCCCIndex);
                        pobTran->srBRec.inHDTIndex = inNCCCIndex;

			inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
			memset(szBatchNum, 0x00, sizeof(szBatchNum));
			inGetBatchNum(szBatchNum);
			pobTran->srBRec.lnBatchNum = atol(szBatchNum);

			memset(szSTANNum, 0x00, sizeof(szSTANNum));
			inGetSTANNum(szSTANNum);
			pobTran->srBRec.lnSTANNum = atol(szSTANNum);

			memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
			inGetInvoiceNum(szInvoiceNum);
			pobTran->srBRec.lnOrgInvNum = atol(szInvoiceNum);

			memcpy(pobTran->szL3_AwardWay, "4", 1);

			/* 決定TRT */
			inRetVal = inFunc_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

                        return (VS_SUCCESS);
	        }
                /* idle手動輸入或一般輸入 */
                else if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
                {
			/* 表示是手動輸入 */
                        pobTran->srBRec.uszManualBit = VS_TRUE;

			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			/* V3 Enter key 代碼為 'A' 所以要限制 */
			if (szKey >= '0' && szKey <= '9')
			{
				pobTran->inMenuKeyin = (int)szKey;
			}

			/* 人工輸入卡號開關 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetManualKeyin(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0)
			{
				inRetVal = inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

				return (inRetVal);
			}

			inRetVal = inCREDIT_Func_Get_Card_Number(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			if (inCARD_Generate_Special_Card(pobTran->srBRec.szPAN) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}

                        /* 第一步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);

                        /* 第二步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* U CARD 有自己的檢核法 */
				if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
				{
					if (inCARD_ValidTrack2_UCard_PAN_MenuKeyIn(pobTran->srBRec.szPAN) != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}
			}

			memset(pobTran->srBRec.szExpDate, 0x00, sizeof(pobTran->srBRec.szExpDate));
			inRetVal = inCREDIT_Func_Get_Exp_Date(pobTran);
			if (inRetVal != VS_SUCCESS)
				return (inRetVal);

			/* ‘5’=以卡號當作兌換資訊，於端末機上手動輸入 */
			memcpy(pobTran->szL3_AwardWay, "5", 1);

			/* 決定TRT */
			inRetVal = inFunc_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

                        return (VS_SUCCESS);
                }

		/* 進迴圈前先清MSR BUFFER */
		inCARD_Clean_MSR_Buffer();

		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測刷卡------------------*/
			inMSR_RetVal = inCARD_MSREvent();
			if (inMSR_RetVal == VS_SUCCESS)
			{
				/* 刷卡事件 */
				ginEventCode = _SWIPE_EVENT_;
			}
			/* 回復錯誤訊息蓋掉的圖 */
			else if (inMSR_RetVal == VS_SWIPE_ERROR)
			{
				inFunc_ResetTitle(pobTran);
				inDISP_Msg_BMP(_ERR_READ_CARD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);

				/* 顯示請刷卡或輸入卡號 */
				inDISP_PutGraphic(_GET_CARD_AWARD_, 0, _COORDINATE_Y_LINE_8_4_);
			}

			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 檢查TIMEOUT */
			if (inTimerGet(_TIMER_GET_CARD_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* Cancel */
				return (VS_USER_CANCEL);
			}
			/* ManuKeyin 為數字鍵時 */
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* key in事件 */
				ginEventCode = _MENUKEYIN_EVENT_;
			}

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				/* 銀聯一般交易不能輸入卡號 */
				if (pobTran->inTransactionCode == _CUP_SALE_ && ginEventCode == _MENUKEYIN_EVENT_)
					continue;
				else
					break;
			}

		}/* while (1) 偵測事件迴圈...*/

	}/* while (1) 對事件做回應迴圈...*/

        return (VS_SUCCESS);
}

/*
Function        :inFunc_GetCardFields_Loyalty_Redeem_CTLS
Date&Time       :2017/11/9 下午 2:51
Describe        :優惠兌換顯示請刷卡、感應卡片或輸入卡號
*/
int inFunc_GetCardFields_Loyalty_Redeem_CTLS(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inNCCCIndex = -1;
	int		inMSR_RetVal = -1;	/* 磁條事件的反應，怕和其他用到inRetVal的搞混所以獨立出來 */
	int		inCTLS_RetVal = -1;	/* 感應卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
        char		szKey = 0;
	char		szBatchNum [6 + 1] = {0}, szInvoiceNum [6 + 1] = {0}, szSTANNum [6 + 1] = {0};
	char		szFuncEnable[2 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	long		lnTimeout = 0;
        unsigned long   ulCTLS_RetVal = 0x00;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_GetCardFields_Loyalty_Redeem_CTLS() START !");
	}

	/* 如果有卡號就直接跳走 */
	if (strlen(pobTran->srBRec.szPAN) > 0)
	{
		return (VS_SUCCESS);
	}

        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;

	/* Send CTLS Readly for Sale Command */
	if (inCTLS_SendReadyForSale_Flow(pobTran) != VS_SUCCESS)
	{
		/* 如果Send 失敗，轉成沒感應的界面 */
		inRetVal = inFunc_GetCardFields_ICC(pobTran);
		return (inRetVal);
	}

	inDISP_ClearAll();
	inDISP_PutGraphic(_MENU_LOYALTY_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_2_);	/* 第二層顯示 ＜HG卡分期付款＞ */
	/* 顯示請刷卡或輸入卡號 */
	inDISP_PutGraphic(_CTLS_AWARD_, 0, _COORDINATE_Y_LINE_8_3_);

	/* 顯示虛擬LED(不閃爍)，但inCTLS_ReceiveReadyForSales(V3機型)也會顯示LED（閃爍），所以沒有實質作用 */
        inCTLS_LED_Wait_Start();

	/* 只抓卡號，不用金額 */

	/* 設定Timeout */
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, strlen(_CUSTOMER_INDICATOR_107_BUMPER_)))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}

	/* inSecond剩餘倒數時間 */
	inDISP_TimeoutStart(lnTimeout);

        while (1)
        {
                /* idle 刷卡 或 一般刷卡 */
	        if (ginIdleMSRStatus == VS_TRUE  || ginEventCode == _SWIPE_EVENT_)
	        {
			/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
			inCTLS_CancelTransacton_Flow();

	                /* 第一步驟GetTrack123 */
	                inCARD_GetTrack123(pobTran);

	                /* 第二步驟unPackTrack資料 */
	                if (inCARD_unPackCard(pobTran) != VS_SUCCESS)
	                {
				/* Unpack失敗，把flag關掉可以再手動輸入 */
	                        ginIdleMSRStatus = VS_FALSE;
	                        continue;
	                }

                        /* 第三步驟 不讀CDT 直接指定Host */
			inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_MAINLY_USE_, &inNCCCIndex);
                        pobTran->srBRec.inHDTIndex = inNCCCIndex;

			inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
			memset(szBatchNum, 0x00, sizeof(szBatchNum));
			inGetBatchNum(szBatchNum);
			pobTran->srBRec.lnBatchNum = atol(szBatchNum);

			memset(szSTANNum, 0x00, sizeof(szSTANNum));
			inGetSTANNum(szSTANNum);
			pobTran->srBRec.lnSTANNum = atol(szSTANNum);

			memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
			inGetInvoiceNum(szInvoiceNum);
			pobTran->srBRec.lnOrgInvNum = atol(szInvoiceNum);

			/* ‘4’=以卡號當作兌換資訊，於端末機上刷卡輸入 */
			memcpy(pobTran->szL3_AwardWay, "4", 1);

			/* 決定TRT */
			inRetVal = inFunc_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

                        return (VS_SUCCESS);
	        }
                /* idle手動輸入或一般輸入 */
                else if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
                {
			/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
			inCTLS_CancelTransacton_Flow();

			/* 表示是手動輸入 */
                        pobTran->srBRec.uszManualBit = VS_TRUE;

			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			/* V3 Enter key 代碼為 'A' 所以要限制 */
			if (szKey >= '0' && szKey <= '9')
			{
				pobTran->inMenuKeyin = (int)szKey;
			}

			/* 人工輸入卡號開關 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetManualKeyin(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0)
			{
				inRetVal = inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

				return (inRetVal);
			}

			inRetVal = inCREDIT_Func_Get_Card_Number(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			if (inCARD_Generate_Special_Card(pobTran->srBRec.szPAN) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}

                        /* 第一步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);

                        /* 第二步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* U CARD 有自己的檢核法 */
				if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
				{
					if (inCARD_ValidTrack2_UCard_PAN_MenuKeyIn(pobTran->srBRec.szPAN) != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}
			}

			memset(pobTran->srBRec.szExpDate, 0x00, sizeof(pobTran->srBRec.szExpDate));
			inRetVal = inCREDIT_Func_Get_Exp_Date(pobTran);
			if (inRetVal != VS_SUCCESS)
				return (inRetVal);

			/* ‘5’=以卡號當作兌換資訊，於端末機上手動輸入 */
			memcpy(pobTran->szL3_AwardWay, "5", 1);

			/* 決定TRT */
			inRetVal = inFunc_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

                        return (VS_SUCCESS);
                }
		/* 感應事件 */
		else if (ginEventCode == _SENSOR_EVENT_)
		{
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			/* 收到的retuen不是等待資料的狀態就去解析 */
			ulCTLS_RetVal = ulCTLS_CheckResponseCode_SALE(pobTran);

			if (ulCTLS_RetVal == d_EMVCL_RC_NO_CARD)
                        {
				/* Timeout沒卡 */
				return (VS_ERROR);
			}
			else if (ulCTLS_RetVal == d_EMVCL_RC_MORE_CARDS)
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_NOT_ONE_CARD_;

				return (VS_WAVE_ERROR);
			}
                        /* 這邊要切SmartPay */
                        else if (ulCTLS_RetVal == d_EMVCL_NON_EMV_CARD)
                        {
				if (pobTran->srBRec.inTxnResult != VS_SUCCESS)
					 return (VS_ERROR);

				/* 轉 FISC_SALE */
				/* FISC incode 在 inFISC_CTLSProcess內設定 */
                        }
                        /* 走信用卡流程 */
                        else if (ulCTLS_RetVal == d_EMVCL_RC_DATA)
                        {
				/* 判斷card bin 讀HDT */
				if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                       return (VS_ERROR);

				/* 檢核PAN module 10 */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetModule10Check(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					/* U CARD 有自己的檢核法 */
					if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
					{
						if (inCARD_ValidTrack2_UCard_PAN(pobTran->srBRec.szPAN) != VS_SUCCESS)
							return (VS_ERROR);
					}
					else
					{
						if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
							return (VS_ERROR);
					}
				}

				/* 檢核ExpDate */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetExpiredDateCheck(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}
                        }
			/*  Two Tap 流程 */
			else if (ulCTLS_RetVal == d_EMVCL_RC_SEE_PHONE)
			{
				/* 重Send命令，等第二次感應 */
				/* Send CTLS Readly for Sale Command */
				if (inCTLS_SendReadyForSale_Flow(pobTran) != VS_SUCCESS)
					return (VS_ERROR);

				ginEventCode = -1;
				continue;
			}
			/* 感應錯誤 */
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "感應失敗,0x%08lX", ulCTLS_RetVal);
					inDISP_LogPrintf(szDebugMsg);
				}

				if (pobTran->inErrorMsg == _ERROR_CODE_V3_NONE_)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
				}

				return (VS_WAVE_ERROR);
			}

			/* 決定TRT */
			inRetVal = inFunc_Decide_CTLS_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			inFunc_ResetTitle(pobTran);

			/* ‘ˋ’=以卡號當作兌換資訊，於端末機上刷卡輸入 */
			/* 因為NCCC沒有定義感應，所以先帶4 */
			memcpy(pobTran->szL3_AwardWay, "4", 1);

			return (VS_SUCCESS);
		}



		/* 進迴圈前先清MSR BUFFER */
		inCARD_Clean_MSR_Buffer();

		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測刷卡------------------*/
			inMSR_RetVal = inCARD_MSREvent();
			if (inMSR_RetVal == VS_SUCCESS)
			{
				/* 刷卡事件 */
				ginEventCode = _SWIPE_EVENT_;
			}
			/* 回復錯誤訊息蓋掉的圖 */
			else if (inMSR_RetVal == VS_SWIPE_ERROR)
			{
				inFunc_ResetTitle(pobTran);
				inDISP_Msg_BMP(_ERR_READ_CARD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);

				inDISP_ClearAll();
				inDISP_PutGraphic(_MENU_LOYALTY_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_2_);	/* 第二層顯示 ＜HG卡分期付款＞ */
				/* 顯示請刷卡或輸入卡號 */
				inDISP_PutGraphic(_CTLS_AWARD_, 0, _COORDINATE_Y_LINE_8_3_);
			}

			/* ------------偵測感應卡------------------ */
			inCTLS_RetVal = inCTLS_ReceiveReadyForSales_Flow(pobTran);
			if (inCTLS_RetVal == VS_SUCCESS)
			{
				/* 感應卡事件 */
				ginEventCode = _SENSOR_EVENT_;
			}

			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 感應倒數時間 && Display Countdown */
			if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
			{
				/* 感應時間到Timeout */
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
				inCTLS_CancelTransacton_Flow();
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
				inCTLS_CancelTransacton_Flow();
				/* Cancel */
				return (VS_USER_CANCEL);
			}
			/* ManuKeyin 為數字鍵時 */
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* key in事件 */
				ginEventCode = _MENUKEYIN_EVENT_;
			}

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				/* 銀聯一般交易不能輸入卡號 */
				if (pobTran->inTransactionCode == _CUP_SALE_ && ginEventCode == _MENUKEYIN_EVENT_)
					continue;
				else
					break;
			}

		}/* while (1) 偵測事件迴圈...*/

	}/* while (1) 對事件做回應迴圈...*/

        return (VS_SUCCESS);
}

/*
Function        :inFunc_GetBarCodeFlow
Date&Time       :2017/2/18 下午 1:58
Describe        :取得優惠條碼
*/
int inFunc_GetBarCodeFlow(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inNCCCIndex = -1;
        char		szKey = 0;
	char		szBatchNum [6 + 1] = {0}, szInvoiceNum [6 + 1] = {0}, szSTANNum [6 + 1] = {0};
	char		szBarCodeReaderEnable[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	long		lnTimeout = 0;
	unsigned char	uszBarCodeReaderEnable = VS_FALSE;

	/* 看是否有開啟BarcodeReader，在這邊看TMS開關而不在迴圈中檢查，增進效率 */
	inGetBarCodeReaderEnable(szBarCodeReaderEnable);
	if (memcmp(szBarCodeReaderEnable, "Y", strlen("Y")) == 0)
	{
		uszBarCodeReaderEnable = VS_TRUE;
	}

	/* 不讀CDT 直接指定Host */
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_MAINLY_USE_, &inNCCCIndex);
	pobTran->srBRec.inHDTIndex = inNCCCIndex;

	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	memset(szBatchNum, 0x00, sizeof(szBatchNum));
	inGetBatchNum(szBatchNum);
	pobTran->srBRec.lnBatchNum = atol(szBatchNum);

	memset(szSTANNum, 0x00, sizeof(szSTANNum));
	inGetSTANNum(szSTANNum);
	pobTran->srBRec.lnSTANNum = atol(szSTANNum);

	memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
	inGetInvoiceNum(szInvoiceNum);
	pobTran->srBRec.lnOrgInvNum = atol(szInvoiceNum);

	if (ginEventCode == _ECR_EVENT_)
	{
		/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
		if (ginFallback == VS_TRUE)
		{
			inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
		}

		/* ‘1’=以條碼當作兌換資訊，透過收銀機條碼資訊 */
		memcpy(pobTran->szL3_AwardWay, "1", 1);

		if (strlen(pobTran->szL3_Barcode1) > 0	||
		    strlen(pobTran->szL3_Barcode2) > 0)
		{
			return (VS_SUCCESS);
		}
	}

	/* 設定Timeout */
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, strlen(_CUSTOMER_INDICATOR_107_BUMPER_)))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			ginEventCode = _ECR_EVENT_;
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}

	inDISP_Timer_Start(_TIMER_GET_CARD_, lnTimeout);

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 顯示請掃描條碼或輸入條碼 */
	inDISP_PutGraphic(_GET_BARCODE_, 0, _COORDINATE_Y_LINE_8_4_);

	while (memcmp(pobTran->szL3_Barcode1, "11", strlen("11")) != 0	&&
	       memcmp(pobTran->szL3_Barcode1, "21", strlen("21")) != 0)	/* 輸入Barcode1迴圈(若輸入值不合法，就繼續輸入) */
        {
                /* idle手動輸入或一般輸入 */
                if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
                {
			/* 表示是手動輸入 */
			pobTran->srBRec.uszManualBit = VS_TRUE;

			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			/* V3 Enter key 代碼為 'A' 所以要限制 */
			if (szKey >= '0' && szKey <= '9')
			{
				pobTran->inMenuKeyin = (int)szKey;
			}

			/* 抓BarCode1 */
			inRetVal = inCREDIT_Func_Get_Barcode1(pobTran);

			/* 重置輸入時第一個字元 */
			szKey = -1;
			pobTran->inMenuKeyin = 0;

			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			/* ‘3’=以條碼當作兌換資訊，手動於端末機輸入兌換(核銷)條碼。 */
			memcpy(pobTran->szL3_AwardWay, "3", 1);

			/* 決定TRT */
			inRetVal = inFunc_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			/* 跳出Barcode1 迴圈 */
			continue;
                }
		else if (ginEventCode == _BARCODE_READER_EVENT_)
		{
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			/* ‘2’=以條碼當作兌換資訊，端末機接BarCode Reader掃描兌換(核銷)條碼。 */
			memcpy(pobTran->szL3_AwardWay, "2", 1);

			/* 決定TRT */
			inRetVal = inFunc_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			/* 跳出Barcode1 迴圈 */
			continue;
		}

		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 檢查TIMEOUT */
			if (inTimerGet(_TIMER_GET_CARD_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* Cancel */
				return (VS_USER_CANCEL);
			}
			/* ManuKeyin 為數字鍵時 */
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* key in事件 */
				ginEventCode = _MENUKEYIN_EVENT_;
			}

			/* ------------偵測Barcode Reader------------------ */
			/* 之後有接Barcode Reader用 */
			if (0/* 這裡放Barcode Reader讀成功的條件 */)
			{
				/* 端末機收到BarCode Reader */
				ginEventCode = _BARCODE_READER_EVENT_;
			}

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}

		}/* while (1) 偵測事件迴圈...*/

	}/* 對事件做回應迴圈... */


	/* 判斷BarCode1後決定是否抓BarCode 2 */
	if (memcmp(pobTran->szL3_Barcode1, "11", strlen("11")) == 0)
	{
		/* 若"11"，則只有一段條碼*/
		return (VS_SUCCESS);
	}

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請掃描或輸入第二段條碼？ */
	inDISP_PutGraphic(_GET_BARCODE_2_, 0, _COORDINATE_Y_LINE_8_4_);

	/* 抓BarCode2 */
        while (memcmp(pobTran->szL3_Barcode2, "22", strlen("22")) != 0)		/* 輸入Barcode2迴圈(若輸入值不合法，就繼續輸入) */
        {
                /* idle手動輸入或一般輸入 */
                if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
                {
			/* 表示是手動輸入 */
                        pobTran->srBRec.uszManualBit = VS_TRUE;

			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			/* V3 Enter key 代碼為 'A' 所以要限制 */
			if (szKey >= '0' && szKey <= '9')
			{
				pobTran->inMenuKeyin = (int)szKey;
			}

			/* 抓BarCode2 */
			inRetVal = inCREDIT_Func_Get_Barcode2(pobTran);

			/* 重置輸入時第一個字元 */
			szKey = -1;
			pobTran->inMenuKeyin = 0;

			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			/* ‘3’=以條碼當作兌換資訊，手動於端末機輸入兌換(核銷)條碼。 */
			memcpy(pobTran->szL3_AwardWay, "3", 1);

			/* 決定TRT */
			inRetVal = inFunc_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			/* 跳出Barcode1 迴圈 */
			continue;
                }
		else if (ginEventCode == _BARCODE_READER_EVENT_)
		{
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			/* ‘2’=以條碼當作兌換資訊，端末機接BarCode Reader掃描兌換(核銷)條碼。 */
			memcpy(pobTran->szL3_AwardWay, "2", 1);

			/* 決定TRT */
			inRetVal = inFunc_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			/* 跳出Barcode1 迴圈 */
			continue;
		}

		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 檢查TIMEOUT */
			if (inTimerGet(_TIMER_GET_CARD_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* Cancel */
				return (VS_USER_CANCEL);
			}
			/* ManuKeyin 為數字鍵時 */
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* key in事件 */
				ginEventCode = _MENUKEYIN_EVENT_;
			}

			/* ------------偵測Barcode Reader------------------ */
			/* 之後有接Barcode Reader用 */
			if (uszBarCodeReaderEnable == VS_TRUE)
			{
				if (0/* 這裡放Barcode Reader讀成功的條件 */)
				{
					/* 端末機收到BarCode Reader */
					ginEventCode = _BARCODE_READER_EVENT_;
				}

			}

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}

		}/* while (1) 偵測事件迴圈...*/

	}/* 對事件做回應迴圈... */


        return (VS_SUCCESS);
}

/*
Function        :inFunc_GetCardFields_HG
Date&Time       :2017/3/1 下午 1:14
Describe        :HG選單進入使用 只能用晶片或磁條
*/
int inFunc_GetCardFields_HG(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = 0;
        int	inMSR_RetVal = -1;	/* 磁條事件的反應，怕和其他用到inRetVal的搞混所以獨立出來 */
	int	inEMV_RetVal = -1;	/* 晶片卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
        char	szKey = 0;
	char	szTransFunc[20 + 1] = {0};
	char	szFuncEnable[2 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	long	lnTimeout = 0;

        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;

	/* 設定Timeout */
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, strlen(_CUSTOMER_INDICATOR_107_BUMPER_)))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}

        inDISP_Timer_Start(_TIMER_GET_CARD_, lnTimeout);

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        if (pobTran->inRunOperationID == _OPERATION_HG_REFUND_)
        {
                /* 顯示請刷HG卡 */
                inDISP_PutGraphic(_GET_HG_CARD_, 0, _COORDINATE_Y_LINE_8_4_);
        }
        else
        {
                /* 顯示請刷或插HG卡 */
                inDISP_PutGraphic(_GET_HG_CARD_ICC_, 0, _COORDINATE_Y_LINE_8_4_);
        }

        /* 進迴圈前先清MSR BUFFER */
        inCARD_Clean_MSR_Buffer();

        while (1)
        {
                /* 一般刷卡 */
                if (ginEventCode == _SWIPE_EVENT_)
                {
                        /* 第一步驟GetTrack123 */
	                inCARD_GetTrack123(pobTran);

	                /* 第二步驟unPackTrack資料 */
	                if (inCARD_unPackCard(pobTran) != VS_SUCCESS)
	                {
				/* Unpack失敗，把flag關掉可以再手動輸入 */
	                        ginIdleMSRStatus = VS_FALSE;
	                        continue;
	                }
			/* 因HG混合交易如果HG非手輸但信用卡手輸 帳單匯印HG卡的持卡人姓名*/
			memset(pobTran->srBRec.szCardHolder, 0x00, sizeof(pobTran->srBRec.szCardHolder));

                        /* 將收到的卡號複製到szHGPAN中 */
                        strcpy(pobTran->srBRec.szHGPAN, pobTran->srBRec.szPAN);

			/* 第三步驟 判斷HG card bin */
                        if (inCARD_GetBin_HG(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);

			/* 純HG不檢核 */
//			if (memcmp(pobTran->srBRec.szHGCardLabel, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_)) == 0)
//			{
//
//			}
//			else
			{
				/* 第二步驟檢核PAN module 10 */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetModule10Check(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					/* U CARD 有自己的檢核法 */
					if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
					{
						if (inCARD_ValidTrack2_UCard_PAN_MenuKeyIn(pobTran->srBRec.szPAN) != VS_SUCCESS)
							return (VS_ERROR);
					}
					else
					{
						if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
							return (VS_ERROR);
					}
				}

				/* 第四步驟檢核ExpDate */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetExpiredDateCheck(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}
			}

                        return (VS_SUCCESS);
                }
		/* Menu Keyin */
                else if (ginEventCode == _MENUKEYIN_EVENT_)
                {
			memset(szTransFunc, 0x00, sizeof(szTransFunc));
			if (inGetTransFunc(szTransFunc) != VS_SUCCESS)
			    return (VS_ERROR);

			/* 人工輸入卡號開關 */
			/* 這裡是HG所以可以在輸入卡號前就判別 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetManualKeyin(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0)
			{
				/* 紅利積點人工輸入卡號 */
				if (pobTran->srBRec.lnHGTransactionType == _HG_REWARD_ && memcmp(&szTransFunc[7], "Y", 1) == 0)
				{

				}
				/* 其他都擋下來 */
				else
				{
					/* 請依正確卡別操作 */
					inRetVal = inDISP_Msg_BMP(_ERR_PLEASE_FOLLOW_RIGHT_OPT_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

					return (inRetVal);
				}
			}
			/* 有開人工輸入 */
			else
			{

			}

			/* 表示是手動輸入 */
			pobTran->uszHGManualBit = VS_TRUE;

			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			/* V3 Enter key 代碼為 'A' 所以要限制 */
			if (szKey >= '0' && szKey <= '9')
			{
				pobTran->inMenuKeyin = (int)szKey;
			}

			inRetVal = inCREDIT_Func_Get_Card_Number(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			if (inCARD_Generate_Special_Card(pobTran->srBRec.szPAN) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}

			/* 將收到的卡號複製到szHGPAN中 */
			strcpy(pobTran->srBRec.szHGPAN, pobTran->srBRec.szPAN);

			/* 第一步驟 判斷HG card bin 讀HDT */
			if (inCARD_GetBin_HG(pobTran) != VS_SUCCESS)
				return(VS_ERROR);

			/* 純HG不檢核 */
//			if (memcmp(pobTran->srBRec.szHGCardLabel, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_)) == 0)
//			{
//
//			}
//			else
			{
				/* 第二步驟檢核PAN module 10 */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetModule10Check(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					/* U CARD 有自己的檢核法 */
					if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
					{
						if (inCARD_ValidTrack2_UCard_PAN_MenuKeyIn(pobTran->srBRec.szHGPAN) != VS_SUCCESS)
							return (VS_ERROR);
					}
					else
					{
						if (inCARD_ValidTrack2_HGPAN(pobTran) != VS_SUCCESS)
							return (VS_ERROR);
					}
				}

				/* 第三步輸入有效期 */
				inRetVal = inCREDIT_Func_Get_Exp_Date(pobTran);
				if (inRetVal != VS_SUCCESS)
					return (inRetVal);

				/* 第四步驟檢核ExpDate */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetExpiredDateCheck(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}
			}

			return (VS_SUCCESS);
                }
                /* 晶片卡事件 */
                else if (ginEventCode == _EMV_DO_EVENT_)
                {
                        /* 在這裡做所有HG退貨插卡檢核 */
                        if (pobTran->inRunOperationID == _OPERATION_HG_REFUND_)
                        {
                                return (VS_ERROR);
                        }

                        /* 取得晶片卡資料 */
                        if (inEMV_GetEMVCardData(pobTran) != VS_SUCCESS)
                                return (VS_ERROR);

			/* 目前只有點數查詢需要先取卡號，此流程可以取代inEMV_Process中分析卡號的那段，但暫時不全面更換 */
			if (inEMV_GetCardNoFlow(pobTran) != VS_SUCCESS)
				return (VS_ERROR);

			/* 因HG混合交易如果HG非手輸但信用卡手輸 帳單匯印HG卡的持卡人姓名*/
			memset(pobTran->srBRec.szCardHolder, 0x00, sizeof(pobTran->srBRec.szCardHolder));

                        /* 將收到的卡號複製到szHGPAN中 */
                        strcpy(pobTran->srBRec.szHGPAN, pobTran->srBRec.szPAN);

			/* 第三步驟 判斷HG card bin */
                        if (inCARD_GetBin_HG(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);

			/* 純HG不檢核 */
//			if (memcmp(pobTran->srBRec.szHGCardLabel, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_)) == 0)
//			{
//
//			}
//			else
			{
				/* 第二步驟檢核PAN module 10 */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetModule10Check(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					/* U CARD 有自己的檢核法 */
					if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
					{
						if (inCARD_ValidTrack2_UCard_PAN_MenuKeyIn(pobTran->srBRec.szPAN) != VS_SUCCESS)
							return (VS_ERROR);
					}
					else
					{
						if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
							return (VS_ERROR);
					}
				}

				/* 第四步驟檢核ExpDate */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetExpiredDateCheck(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}
			}

                        return (VS_SUCCESS);
                }

                while (1)
                {
                        ginEventCode = -1;
                        /* ------------偵測刷卡------------------*/
                        inMSR_RetVal = inCARD_MSREvent();
                        if (inMSR_RetVal == VS_SUCCESS)
                        {
                                /* 刷卡事件 */
                                ginEventCode = _SWIPE_EVENT_;
                        }
                        /* 回復錯誤訊息蓋掉的圖 */
                        else if (inMSR_RetVal == VS_SWIPE_ERROR)
                        {
                                inFunc_ResetTitle(pobTran);
                                inDISP_Msg_BMP(_ERR_READ_CARD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);

                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                if (pobTran->inRunOperationID == _OPERATION_HG_REFUND_)
                                {
                                        /* 顯示請刷HG卡 */
                                        inDISP_PutGraphic(_GET_HG_CARD_, 0, _COORDINATE_Y_LINE_8_4_);
                                }
                                else
                                {
                                        /* 顯示請刷或插HG卡 */
                                        inDISP_PutGraphic(_GET_HG_CARD_ICC_, 0, _COORDINATE_Y_LINE_8_4_);
                                }
                        }

                        /* ------------偵測晶片卡---------------- */
                        inEMV_RetVal = inEMV_ICCEvent();
                        if (inEMV_RetVal == VS_SUCCESS)
                        {
                                /* 晶片卡事件 */
                                ginEventCode = _EMV_DO_EVENT_;
                        }

                        /* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 檢查TIMEOUT */
			if (inTimerGet(_TIMER_GET_CARD_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* Cancel */
				return (VS_USER_CANCEL);
			}
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* key in事件 */
				ginEventCode = _MENUKEYIN_EVENT_;
			}

                        /* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}
                }
        }
}

/*
Function        :inFunc_GetCreditCardFields_HG
Date&Time       :2017/1/20 下午 07:14
Describe        :HG交易時取得信用卡資訊
*/
int inFunc_GetCreditCardFields_HG(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal;
        char    szCTLSEnable[2 + 1];		/* 觸控是否打開 */
        char	szTMSOK[2 + 1];
	char	szFuncEnable[2 + 1];

        if (ginDebug == VS_TRUE)
	{
                inDISP_LogPrintf("inFunc_GetCreditCardFields_HG START");
        }

        if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
	{
                if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CREDIT_ || pobTran->srBRec.lnHGPaymentType == _HG_PAY_CUP_)
                {
			/* 不是選HGI，需要重置信用卡狀態 */
			pobTran->srBRec.inChipStatus = 0;


			if (inCREDIT_Func_GetAmount(pobTran) != VS_SUCCESS)			// 輸入金額
				return (VS_ERROR);

			//if (pobTran->srBRec.lnHGTransactionType == _HG_POINT_CERTAIN_)
			//{
			//	if (inHG_Func_GetPoint(pobTran) != VS_SUCCESS)			// 輸入使用點數
			//		return (VS_ERROR);
			//}

			if (inCREDIT_Func_GetProductCode(pobTran) != VS_SUCCESS)		// 輸入櫃號
				return (VS_ERROR);

			if (inCREDIT_Func_GetStoreID(pobTran) != VS_SUCCESS)			// 輸入櫃號
				return (VS_ERROR);

                        /* 如果lnHGPaymentType不是_HG_PAY_CREDIT_INSIDE_，則清除信用卡號 */
                        memset(pobTran->srBRec.szPAN, 0x00, sizeof(pobTran->srBRec.szPAN));
                        ginEventCode = -1;

                        memset(szTMSOK, 0x00, sizeof(szTMSOK));
                        inGetTMSOK(szTMSOK);

                        memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
                        inGetContactlessEnable(szCTLSEnable);

                        if (!memcmp(szCTLSEnable, "Y", 1) && !memcmp(szTMSOK, "Y", 1))
                        {
                                inRetVal = inFunc_GetCardFields_CTLS(pobTran);
                        }
                        else
                        {
                                inRetVal = inFunc_GetCardFields_ICC(pobTran);
                        }

                        if (inRetVal == VS_SUCCESS) // 插卡、刷卡、感應
                        {
                                pobTran->srBRec.inHGCreditHostIndex = pobTran->srBRec.inHDTIndex;
                        }
                        else
                        {
                                return (VS_ERROR);
                        }

                }
		/* HGI不過卡，但仍要獲得卡片資訊 */
		else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CREDIT_INSIDE_)
		{
			/* 第二步驟unPackTrack資料 */
			if (inCARD_unPackCard(pobTran) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}

			/* 第三步驟 判斷card bin 讀HDT */
			if (inCARD_GetBin(pobTran) != VS_SUCCESS)
				return(VS_ERROR);

			/* 第四步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* U CARD 有自己的檢核法 */
				if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
				{
					if (inCARD_ValidTrack2_UCard_PAN(pobTran->srBRec.szPAN) != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}
			}

			/* 第六步驟檢核SeviceCode */
			/* 如果過HG不是是用插卡，而且用HGI就需要檢核SeviceCode (避免晶片卡用刷卡過交易) */
			if (pobTran->srBRec.inChipStatus != _EMV_CARD_)
			{
				if (inCheckFallbackFlag(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}

			/* 第七步驟檢核ExpDate */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetExpiredDateCheck(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}
		}

        }

        if (ginDebug == VS_TRUE)
	{
                inDISP_LogPrintf("inFunc_GetCreditCardFields_HG END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inFunc_GetCardFields_MailOrder
Date&Time       :2017/6/7 下午 6:22
Describe        :
*/
int inFunc_GetCardFields_MailOrder(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
        char	szKey = 0;
	char	szFuncEnable[2 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	long	lnTimeout = 0;

        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;

        /* 如果ginEventCode為數字鍵流程即為IDLE MenuKeyIn */
        if (ginEventCode >= '0' && ginEventCode <= '9')
        {
                /* 表示是idle手動輸入，這邊才on flag是因為有可能idle輸入金額 */
                ginMenuKeyIn = VS_TRUE;
        }

	/* 設定Timeout */
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, strlen(_CUSTOMER_INDICATOR_107_BUMPER_)))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}

	inDISP_Timer_Start(_TIMER_GET_CARD_, lnTimeout);

        while (1)
        {

                /* idle手動輸入或一般輸入 */
                if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
                {
			/* 表示是手動輸入 */
                        pobTran->srBRec.uszManualBit = VS_TRUE;

			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			/* V3 Enter key 代碼為 'A' 所以要限制 */
			if (szKey >= '0' && szKey <= '9')
			{
				pobTran->inMenuKeyin = (int)szKey;
			}

			/* 人工輸入卡號開關 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetManualKeyin(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0)
			{
				inRetVal = inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_4_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

				return (inRetVal);
			}

			inRetVal = inCREDIT_Func_Get_Card_Number(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			if (inCARD_Generate_Special_Card(pobTran->srBRec.szPAN) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}

                        /* 第一步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);

			/* 第二步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* U CARD 有自己的檢核法 */
				if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
				{
					if (inCARD_ValidTrack2_UCard_PAN_MenuKeyIn(pobTran->srBRec.szPAN) != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}
			}

                        /* 第三步有效期 */
                        inRetVal = inCREDIT_Func_Get_Exp_Date(pobTran);
			if (inRetVal != VS_SUCCESS)
				return (inRetVal);

                        /* 第四步驟檢核ExpDate */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetExpiredDateCheck(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}

			/* 決定TRT */
			inRetVal = inFunc_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

                        return (VS_SUCCESS);
                }

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

		while (1)
		{
			ginEventCode = -1;


			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 檢查TIMEOUT */
			if (inTimerGet(_TIMER_GET_CARD_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* Cancel */
				return (VS_USER_CANCEL);
			}

			/* key in事件 */
			ginEventCode = _MENUKEYIN_EVENT_;

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}

		}/* while (1) 偵測事件迴圈...*/

	}/* while (1) 對事件做回應迴圈...*/

        return (VS_SUCCESS);
}

/*
Function        :inFunc_PrintReceipt
Date&Time       :2015/8/10 上午 10:24
Describe        :FuncTable.c列印信用卡指標
*/
int inFunc_PrintReceipt(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CUST_RECEIPT_;
	char		szDispBuf[_DISP_MSG_SIZE_ + 1] = {0};
	char		szPrtMode[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char   uszKey = 0x00;

	/* 為了恢復被signpad清掉的title */
	inFunc_ResetTitle(pobTran);

	memset(szPrtMode, 0x00, sizeof(szPrtMode));
	inGetPrtMode(szPrtMode);

	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

        /* 列印商店聯 */
	/* 簽單模式不是0就印商店聯 */
	if (memcmp(szPrtMode, "0", strlen("0")) != 0)
	{
		while (1)
		{
			pobTran->srBRec.inPrintOption = _PRT_MERCH_;
			inRetVal = inCREDIT_PRINT_Receipt(pobTran);

			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
				break;
		}
	}

	/* 三聯式簽單，中間要多印一聯客戶聯 */
	if (memcmp(szPrtMode, "3", strlen("3")) == 0)
	{
		/* 列印帳單中 */
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

		/* 列印商店存根聯 */
		while (1)
		{
			pobTran->srBRec.inPrintOption = _PRT_MERCH_DUPLICATE_;
			inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
				break;
		}
	}

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	if (pobTran->srBRec.uszF56NoSignatureBit == VS_TRUE)
	//ifif (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
	{
		/* 提示免簽名和授權碼 */
		inDISP_PutGraphic(_CHECK_SIGNATURE_5_, 0, _COORDINATE_Y_LINE_8_4_);
	}
	else
	{
		/* 提示檢核簽名和授權碼 */
		inDISP_PutGraphic(_CHECK_SIGNATURE_1_, 0, _COORDINATE_Y_LINE_8_4_);
	}

        memset(szDispBuf, 0x00, sizeof(szDispBuf));
        sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
        inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _COLOR_WHITE_, 11);

	/* 客製化107顯示訊息TimeOut 2秒 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, strlen(_CUSTOMER_INDICATOR_107_BUMPER_)))
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
	}
	else
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 180);
	}
        while (1)
        {
                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                uszKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

                if (uszKey == _KEY_ENTER_			||
		    uszKey == _KEY_TIMEOUT_			||
		    inChoice == _CUSTReceipt_Touch_ENTER_)
		{
			inRetVal = VS_SUCCESS;
                        break;
		}
                else if (uszKey == _KEY_CANCEL_			||
			 inChoice == _CUSTReceipt_Touch_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
                        break;
		}
        }
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	/* 因為這裡交易已完成，所以一定回傳成功*/
	if (inRetVal == VS_USER_CANCEL)
	{
		return (VS_SUCCESS);
	}

        /* 列印顧客聯 */
        while (1)
        {
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		if (pobTran->srBRec.uszF56NoSignatureBit == VS_TRUE)
		//if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
		{
			/* 提示免簽名和授權碼 */
			inDISP_PutGraphic(_CHECK_SIGNATURE_6_, 0, _COORDINATE_Y_LINE_8_4_);
		}
		else
		{
			/* 提示檢核簽名和授權碼 */
			inDISP_PutGraphic(_CHECK_SIGNATURE_2_, 0, _COORDINATE_Y_LINE_8_4_);
		}

                memset(szDispBuf, 0x00, sizeof(szDispBuf));
                sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
                inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _COLOR_WHITE_,11);

                pobTran->srBRec.inPrintOption = _PRT_CUST_;
                inRetVal = inCREDIT_PRINT_Receipt(pobTran);

                if (inRetVal == VS_PRINTER_PAPER_OUT ||
		    inRetVal == VS_PRINTER_OVER_HEAT)
			continue;
		else
			break;
        }

        return (VS_SUCCESS);
}

/*
Function        :inFunc_PrintTotalReport
Date&Time       :2015/8/20 上午 10:24
Describe        :FuncTable.c列印總額帳單
*/
int inFunc_PrintTotalReport(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;

	while (1)
        {
		inRetVal = inCREDIT_PRINT_TotalReport(pobTran);

		if (inRetVal == VS_PRINTER_PAPER_OUT ||
		    inRetVal == VS_PRINTER_OVER_HEAT)
			continue;
		else
			break;
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_PrintDetailReport
Date&Time       :2015/8/20 上午 10:24
Describe        :FuncTable.c列印明細帳單
*/
int inFunc_PrintDetailReport(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;

	while (1)
        {
		inRetVal = inCREDIT_PRINT_DetailReport(pobTran);

		if (inRetVal == VS_PRINTER_PAPER_OUT ||
		    inRetVal == VS_PRINTER_OVER_HEAT)
			continue;
		else
			break;
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_PrintReceipt_ByBuffer_Flow
Date&Time       :2017/3/22 上午 11:07
Describe        :印帳單分流
*/
int inFunc_PrintReceipt_ByBuffer_Flow(TRANSACTION_OBJECT *pobTran)
{
	char	szESCMode[2 + 1] = {0};
	char	szTRTFileName[16 + 1] = {0};
//	char	szTxnType[20 + 1] = {0};
//	char	szFESMode[2 + 1] = {0};
//	char	szDispBuf[50 + 1] = {0};
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "PRINT_5667 inFunc_PrintReceipt_ByBuffer_Flow INIT" );	

	inDISP_LogPrintfWithFlag("  Func Print ByBuffer RRN [%s]", pobTran->srBRec.szRefNo );
	
	/* 抓取 KioskFlag的值，如為 TRUE 就不列印簽單 2020/3/27 下午 4:23 [SAM] */
	if(inFunc_GetKisokFlag() != VS_TRUE)
	{
		/* 是否有列印功能 */
		if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
		{
			inDISP_Msg_BMP(_TRANSACTION_SUCCESS_, _COORDINATE_Y_LINE_8_4_, _NO_KEY_MSG_, 2, "", 0);
			inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "PRINT_5667  Ckeck Capability END");
		}
		else
		{
			memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
			inGetTRTFileName(szTRTFileName);

			inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "PRINT_5667  Bef Print Receipt  END");
			if (memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)) == 0	)
			{
				/* ESC開關 */
				memset(szESCMode, 0x00, sizeof(szESCMode));
				inGetESCMode(szESCMode);

				/* 沒ESC的狀況，直接印紙本，不須另外判斷 */
				if (szESCMode[0] != 'Y')
				{
					inFunc_NCCC_PrintReceipt_ByBuffer(pobTran);
				}
				/* 有ESC的狀況 */
				else
				{
					inFunc_NCCC_PrintReceipt_ByBuffer_ESC(pobTran);
				}

			}		
			/* 電票交易 */
			else if (memcmp(szTRTFileName, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)) == 0)
			{
				inFunc_NCCC_PrintReceipt_ByBuffer_ESVC(pobTran);
			}else if (memcmp(szTRTFileName, _TRT_FILE_NAME_CMAS_, strlen(_TRT_FILE_NAME_CMAS_)) == 0)
			{	/* [新增電票悠遊卡功能]  新加入悠遊卡條件及功能 [SAM] 2022/6/8 下午 5:06  */
				inCMAS_Func_PrintReceipt_ByBuffer(pobTran);
			}/* 其他Host，如DINERS */
			else if (memcmp(szTRTFileName, _TRT_FILE_NAME_SVC_, strlen(_TRT_FILE_NAME_SVC_)) == 0)
			{
				/* [新增SVC功能]  [SAM] */
				inSVC_PRINT_ReceiptByBuffer(pobTran);
			}else
			{
				inFunc_PrintReceipt_ByBuffer(pobTran);
			}

			inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "PRINT_5667  Aft Print Receipt  END");
		}
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "PRINT_5667 inFunc_PrintReceipt_ByBuffer_Flow END");
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
        return (VS_SUCCESS);
}

/*
Function        :inFunc_PrintReceipt_ByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :標準，For Diners使用
*/
int inFunc_PrintReceipt_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CUST_RECEIPT_;
	char		szDispBuf[_DISP_MSG_SIZE_ + 1] = {0};
	char		szPrtMode[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char   uszKey = 0x00;

	/* 為了恢復被signpad清掉的title */
	inFunc_ResetTitle(pobTran);

	memset(szPrtMode, 0x00, sizeof(szPrtMode));
	inGetPrtMode(szPrtMode);

	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/* 列印商店聯 */
	/* 簽單模式不是0就印商店聯 */
	if (memcmp(szPrtMode, "0", strlen("0")) != 0)
	{
		/* 列印帳單中 */
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

		while (1)
		{
			pobTran->srBRec.inPrintOption = _PRT_MERCH_;
			inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
			{
				/* 成功就砍簽名圖檔 */
				inFunc_Delete_Signature(pobTran);

				break;
			}
		}
	}

	/* 三聯式簽單，中間要多印一聯客戶聯 */
	if (memcmp(szPrtMode, "3", strlen("3")) == 0)
	{
		/* 列印帳單中 */
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

		/* 列印商店存根聯 */
		while (1)
		{
			pobTran->srBRec.inPrintOption = _PRT_MERCH_DUPLICATE_;
			inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
				break;
		}
	}

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	if (pobTran->srBRec.uszF56NoSignatureBit == VS_TRUE)
	//ifif (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
	{
		/* 提示免簽名和授權碼 */
		inDISP_PutGraphic(_CHECK_SIGNATURE_5_, 0, _COORDINATE_Y_LINE_8_4_);
	}
	else
	{
		/* 提示檢核簽名和授權碼 */
		inDISP_PutGraphic(_CHECK_SIGNATURE_1_, 0, _COORDINATE_Y_LINE_8_4_);
	}

	memset(szDispBuf, 0x00, sizeof(szDispBuf));
	sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
	inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 11);


        /* 第一聯印完後15秒未按確認或清除鍵則端末機嗶嗶聲提示，響15秒後自動出第二聯 */
	/* 客製化107顯示訊息TimeOut 2秒 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, strlen(_CUSTOMER_INDICATOR_107_BUMPER_)))
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
		inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
	}
	else
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_2_, 30);
		inDISP_Timer_Start(_TIMER_NEXSYS_3_, 15);
	}

        while (1)
        {
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                uszKey = uszKBD_Key();

		if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
		{
			inDISP_BEEP(1, 250);
		}

                if (uszKey == _KEY_ENTER_			||
		    uszKey == _KEY_TIMEOUT_			||
		    inChoice == _CUSTReceipt_Touch_ENTER_)
		{
			inRetVal = VS_SUCCESS;
                        break;
		}
                else if (uszKey == _KEY_CANCEL_			||
			 inChoice == _CUSTReceipt_Touch_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
                        break;
		}
        }
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	/* 因為這裡交易已完成，所以一定回傳成功*/
	if (inRetVal == VS_USER_CANCEL)
	{
		return (VS_SUCCESS);
	}

        /* 列印顧客聯 */
	/* 三聯式簽單，兩聯以上就要印顧客聯 */
	if (memcmp(szPrtMode, "2", strlen("2")) == 0	||
	    memcmp(szPrtMode, "3", strlen("3")) == 0)
	{
		while (1)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			if (pobTran->srBRec.uszF56NoSignatureBit == VS_TRUE)
			//if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
			{
				/* 提示免簽名和授權碼 */
				inDISP_PutGraphic(_CHECK_SIGNATURE_6_, 0, _COORDINATE_Y_LINE_8_4_);
			}
			else
			{
				if (pobTran->srBRec.uszF56NoSignatureBit == VS_TRUE)
				//if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
				{
					/* 提示免簽名和授權碼 */
					inDISP_PutGraphic(_CHECK_SIGNATURE_6_, 0, _COORDINATE_Y_LINE_8_4_);
				}
				else
				{
					/* 提示檢核簽名和授權碼 */
					inDISP_PutGraphic(_CHECK_SIGNATURE_2_, 0, _COORDINATE_Y_LINE_8_4_);
				}
			}

			memset(szDispBuf, 0x00, sizeof(szDispBuf));
			sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
			inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 11);

			pobTran->srBRec.inPrintOption = _PRT_CUST_;
			inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
				break;
		}
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_NCCC_PrintReceipt_ByBuffer_Loyalty_Redeem
Date&Time       :2017/2/21 下午 5:01
Describe        :優惠兌換專用
*/
int inFunc_NCCC_PrintReceipt_ByBuffer_Loyalty_Redeem(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	char	szPrtMode[2 + 1];

	/* 為了恢復被signpad清掉的title */
	inFunc_ResetTitle(pobTran);

	memset(szPrtMode, 0x00, sizeof(szPrtMode));
	inGetPrtMode(szPrtMode);

	/* 列印商店聯 */
	/* 簽單模式不是0就印商店聯 */
	if (memcmp(szPrtMode, "0", strlen("0")) != 0)
	{
		/* 列印帳單中 */
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

		while (1)
		{
			pobTran->srBRec.inPrintOption = _PRT_MERCH_;
			inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
			{
				/* 成功就砍簽名圖檔 */
				inFunc_Delete_Signature(pobTran);

				break;
			}
		}
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_NCCC_PrintReceipt_ByBuffer
Date&Time       :2017/3/22 下午 1:46
Describe        :NCCC專用
*/
int inFunc_NCCC_PrintReceipt_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CUST_RECEIPT_;
	char		szDispBuf[_DISP_MSG_SIZE_ + 1] = {0};
	char		szPrtMode[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char   uszKey = 0x00;

	/* 為了恢復被signpad清掉的title */
	inFunc_ResetTitle(pobTran);

	memset(szPrtMode, 0x00, sizeof(szPrtMode));
	inGetPrtMode(szPrtMode);

	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/* 列印商店聯 */
	/* 簽單模式不是0就印商店聯 */
	if (memcmp(szPrtMode, "0", strlen("0")) != 0)
	{
		/* 列印帳單中 */
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

		while (1)
		{
			pobTran->srBRec.inPrintOption = _PRT_MERCH_;
			inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
			{
				/* 成功就砍簽名圖檔 */
				inFunc_Delete_Signature(pobTran);

				break;
			}

		}
	}

	/* 三聯式簽單，中間要多印一聯客戶聯 */
	if (memcmp(szPrtMode, "3", strlen("3")) == 0)
	{
		/* 列印帳單中 */
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

		/* 列印商店存根聯 */
		while (1)
		{
			pobTran->srBRec.inPrintOption = _PRT_MERCH_DUPLICATE_;
			inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
			{
				break;
			}
		}
	}

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		/* SmartPay交易要提示調閱編號FiscRRN */
		inDISP_PutGraphic(_CHECK_SIGNATURE_3_, 0, _COORDINATE_Y_LINE_8_4_);

		memset(szDispBuf, 0x00, sizeof(szDispBuf));
		sprintf(szDispBuf, "%s", pobTran->srBRec.szFiscRRN);
		inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 9);
	}
	else
	{
		if (pobTran->srBRec.uszF56NoSignatureBit == VS_TRUE)
		//if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
		{
			/* 提示免簽名和授權碼 */
			inDISP_PutGraphic(_CHECK_SIGNATURE_5_, 0, _COORDINATE_Y_LINE_8_4_);
		}
		else
		{
			/* 提示檢核簽名和授權碼 */
			inDISP_PutGraphic(_CHECK_SIGNATURE_1_, 0, _COORDINATE_Y_LINE_8_4_);
		}

		memset(szDispBuf, 0x00, sizeof(szDispBuf));
		sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
		inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 11);
	}

	/* 第一聯印完後15秒未按確認或清除鍵則端末機嗶嗶聲提示，響15秒後自動出第二聯 */
	/* 客製化107顯示訊息TimeOut 2秒 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, strlen(_CUSTOMER_INDICATOR_107_BUMPER_)))
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
		inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
	}
	else
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_2_, 30);
		inDISP_Timer_Start(_TIMER_NEXSYS_3_, 15);
	}
	
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		uszKey = uszKBD_Key();

		if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
		{
			inDISP_BEEP(1, 250);
		}

		if (uszKey == _KEY_ENTER_			||
		    uszKey == _KEY_TIMEOUT_			||
		    inChoice == _CUSTReceipt_Touch_ENTER_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (uszKey == _KEY_CANCEL_			||
			 inChoice == _CUSTReceipt_Touch_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	/* 因為這裡交易已完成，所以一定回傳成功*/
	if (inRetVal == VS_USER_CANCEL)
	{
		return (VS_SUCCESS);
	}

        /* 列印顧客聯 */
	/* 三聯式簽單，兩聯以上就要印顧客聯 */
	if (memcmp(szPrtMode, "2", strlen("2")) == 0	||
	    memcmp(szPrtMode, "3", strlen("3")) == 0)
	{
		while (1)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				/* SmartPay交易要提示調閱編號FiscRRN */
				inDISP_PutGraphic(_CHECK_SIGNATURE_4_, 0, _COORDINATE_Y_LINE_8_4_);

				memset(szDispBuf, 0x00, sizeof(szDispBuf));
				sprintf(szDispBuf, "%s", pobTran->srBRec.szFiscRRN);
				inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 9);
			}
			else
			{
				if (pobTran->srBRec.uszF56NoSignatureBit == VS_TRUE)
				//if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
				{
					/* 提示免簽名和授權碼 */
					inDISP_PutGraphic(_CHECK_SIGNATURE_6_, 0, _COORDINATE_Y_LINE_8_4_);
				}
				else
				{
					/* 提示檢核簽名和授權碼 */
					inDISP_PutGraphic(_CHECK_SIGNATURE_2_, 0, _COORDINATE_Y_LINE_8_4_);
				}

				memset(szDispBuf, 0x00, sizeof(szDispBuf));
				sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
				inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 11);
			}

			pobTran->srBRec.inPrintOption = _PRT_CUST_;
			
			inDISP_LogPrintfWithFlag(" Print ByBuffer  Cust RRN [%s]", pobTran->srBRec.szRefNo );
			
			inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
			{
				break;
			}
		}
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_NCCC_PrintReceipt_ByBuffer_ESC
Date&Time       :2017/3/22 下午 1:45
Describe        :有ESC判斷
*/
int inFunc_NCCC_PrintReceipt_ByBuffer_ESC(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CUST_RECEIPT_;
	char		szDispBuf[_DISP_MSG_SIZE_ + 1];
	char		szPrtMode[2 + 1];
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char   uszKey = 0x00;

	/* 為了恢復被signpad清掉的title */
	inFunc_ResetTitle(pobTran);

	memset(szPrtMode, 0x00, sizeof(szPrtMode));
	inGetPrtMode(szPrtMode);

	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	if (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_NOT_SUPPORTED_	||
	    pobTran->srBRec.inESCUploadMode == _ESC_STATUS_OVERLIMIT_		||
	   (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_SUPPORTED_		&&
	    pobTran->srBRec.inSignStatus == _SIGN_BYPASS_)			||
	    inNEXSYS_ESC_MerchantCopy_Check(pobTran) == VS_SUCCESS)
	{
		/* 列印商店聯 */
		/* 簽單模式不是0就印商店聯 */
		if (memcmp(szPrtMode, "0", strlen("0")) != 0)
		{
			/* 列印帳單中 */
			inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

			while (1)
			{
				pobTran->srBRec.inPrintOption = _PRT_MERCH_;
				inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(pobTran);

				/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
				if (inRetVal == VS_PRINTER_PAPER_OUT ||
				    inRetVal == VS_PRINTER_OVER_HEAT)
					continue;
				else
				{
					/* 如果不是因為加印(之後還要上傳ESC用)才出紙本，印完紙本就砍簽名圖檔 */
					if (inNEXSYS_ESC_MerchantCopy_Check(pobTran) != VS_SUCCESS)
					{
						inFunc_Delete_Signature(pobTran);
					}

					break;
				}

			}
		}

		if (inNEXSYS_ESC_MerchantCopy_Check(pobTran) == VS_SUCCESS	||
		   (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_SUPPORTED_	&&
		    pobTran->srBRec.inSignStatus == _SIGN_BYPASS_))
		{
			/* 三聯式簽單，中間要多印一聯客戶聯 */
			if (memcmp(szPrtMode, "3", strlen("3")) == 0)
			{
				/* 列印帳單中 */
				inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

				/* 列印商店存根聯 */
				while (1)
				{
					pobTran->srBRec.inPrintOption = _PRT_MERCH_DUPLICATE_;
					inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(pobTran);

					/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
					if (inRetVal == VS_PRINTER_PAPER_OUT ||
					    inRetVal == VS_PRINTER_OVER_HEAT)
						    continue;
					else
					{
						break;
					}
				}
			}
		}

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			/* SmartPay交易要提示調閱編號FiscRRN */
			inDISP_PutGraphic(_CHECK_SIGNATURE_3_, 0, _COORDINATE_Y_LINE_8_4_);

			memset(szDispBuf, 0x00, sizeof(szDispBuf));
			sprintf(szDispBuf, "%s", pobTran->srBRec.szFiscRRN);
			inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 9);
		}
		else
		{
			if (pobTran->srBRec.uszF56NoSignatureBit == VS_TRUE)
			//if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
			{
				/* 提示免簽名和授權碼 */
				inDISP_PutGraphic(_CHECK_SIGNATURE_5_, 0, _COORDINATE_Y_LINE_8_4_);
			}
			else
			{
				/* 提示檢核簽名和授權碼 */
				inDISP_PutGraphic(_CHECK_SIGNATURE_1_, 0, _COORDINATE_Y_LINE_8_4_);
			}

			memset(szDispBuf, 0x00, sizeof(szDispBuf));
			sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
			inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 11);
		}

		/* 第一聯印完後15秒未按確認或清除鍵則端末機嗶嗶聲提示，響15秒後自動出第二聯 */
		/* 客製化107顯示訊息TimeOut 2秒 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, strlen(_CUSTOMER_INDICATOR_107_BUMPER_)))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
			inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
		}
		else
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_2_, 30);
			inDISP_Timer_Start(_TIMER_NEXSYS_3_, 15);
		}
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			uszKey = uszKBD_Key();

			if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
			{
				uszKey = _KEY_TIMEOUT_;
			}

			if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
			{
				inDISP_BEEP(1, 250);
			}

			if (uszKey == _KEY_ENTER_			||
			    uszKey == _KEY_TIMEOUT_			||
			    inChoice == _CUSTReceipt_Touch_ENTER_)
			{
				inRetVal = VS_SUCCESS;
				break;
			}
			else if (uszKey == _KEY_CANCEL_			||
			     inChoice == _CUSTReceipt_Touch_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				break;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();

		/* 因為這裡交易已完成，所以一定回傳成功*/
		if (inRetVal == VS_USER_CANCEL)
		{
			return (VS_SUCCESS);
		}

	}

	/* 三聯式簽單，兩聯以上就要印顧客聯 */
	if (memcmp(szPrtMode, "2", strlen("2")) == 0	||
	    memcmp(szPrtMode, "3", strlen("3")) == 0)
	{
		/* 列印顧客聯 */
		while (1)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_PRINTING_, 0, _COORDINATE_Y_LINE_8_7_);

			pobTran->srBRec.inPrintOption = _PRT_CUST_;
			inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
			{
				break;
			}
		}
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_PrintReceipt_ByBuffer_HappyGo_Single
Date&Time       :2017/3/22 上午 11:08
Describe        :
*/
int inFunc_NCCC_PrintReceipt_ByBuffer_HappyGo_Single(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CUST_RECEIPT_;
	char		szDispBuf[_DISP_MSG_SIZE_ + 1] = {0};
	char		szPrtMode[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char   uszKey = 0x00;

	/* 為了恢復被signpad清掉的title */
	inFunc_ResetTitle(pobTran);

	memset(szPrtMode, 0x00, sizeof(szPrtMode));
	inGetPrtMode(szPrtMode);

	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	/* 列印商店聯 */
	/* 簽單模式不是0就印商店聯 */
	if (memcmp(szPrtMode, "0", strlen("0")) != 0)
	{
		/* 列印帳單中 */
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

		while (1)
		{
			pobTran->srBRec.inPrintOption = _PRT_MERCH_;
			inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
			{
				/* 成功就砍簽名圖檔 */
				inFunc_Delete_Signature(pobTran);

				break;
			}
		}
	}

	if (inNEXSYS_ESC_MerchantCopy_Check(pobTran) == VS_SUCCESS	||
	   (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_SUPPORTED_	&&
	    pobTran->srBRec.inSignStatus == _SIGN_BYPASS_))
	{
		/* 三聯式簽單，中間要多印一聯客戶聯 */
		if (memcmp(szPrtMode, "3", strlen("3")) == 0)
		{
			/* 列印帳單中 */
			inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

			/* 列印商店存根聯 */
			while (1)
			{
				pobTran->srBRec.inPrintOption = _PRT_MERCH_DUPLICATE_;
				inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(pobTran);

				/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
				if (inRetVal == VS_PRINTER_PAPER_OUT ||
				    inRetVal == VS_PRINTER_OVER_HEAT)
					continue;
				else
				{
					break;
				}
			}
		}
	}

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 提示列印客戶存根 */
	inDISP_PutGraphic(_CHECK_PRINT_CUST_RECEIPT_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 第一聯印完後15秒未按確認或清除鍵則端末機嗶嗶聲提示，響15秒後自動出第二聯 */
	/* 客製化107顯示訊息TimeOut 2秒 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, strlen(_CUSTOMER_INDICATOR_107_BUMPER_)))
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
		inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
	}
	else
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_2_, 30);
		inDISP_Timer_Start(_TIMER_NEXSYS_3_, 15);
	}
        while (1)
        {
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		uszKey = uszKBD_Key();

		if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
		{
			inDISP_BEEP(1, 250);
		}

                if (uszKey == _KEY_ENTER_			||
		    uszKey == _KEY_TIMEOUT_			||
		    inChoice == _CUSTReceipt_Touch_ENTER_)
                        break;
                else if (uszKey == _KEY_CANCEL_			||
			 inChoice == _CUSTReceipt_Touch_CANCEL_)
                        return (VS_SUCCESS);
                else
                        continue;
        }

        /* 列印顧客聯 */
	/* 三聯式簽單，兩聯以上就要印顧客聯 */
	if (memcmp(szPrtMode, "2", strlen("2")) == 0	||
	    memcmp(szPrtMode, "3", strlen("3")) == 0)
	{
		while (1)
		{
			if (pobTran->srBRec.uszHappyGoSingle != TRUE)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				if (pobTran->srBRec.uszF56NoSignatureBit == VS_TRUE)
				//if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
				{
					/* 提示免簽名和授權碼 */
					inDISP_PutGraphic(_CHECK_SIGNATURE_6_, 0, _COORDINATE_Y_LINE_8_4_);
				}
				else
				{
					/* 提示檢核簽名和授權碼 */
					inDISP_PutGraphic(_CHECK_SIGNATURE_2_, 0, _COORDINATE_Y_LINE_8_4_);
				}
			}
			else
			{
				/* 列印中 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_PRINTING_, 0, _COORDINATE_Y_LINE_8_7_);
			}

			memset(szDispBuf, 0x00, sizeof(szDispBuf));
			sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
			inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 11);

			pobTran->srBRec.inPrintOption = _PRT_CUST_;
			inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
			{
				break;
			}
		}
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_NCCC_PrintReceipt_ByBuffer_ESVC
Date&Time       :2018/1/8 下午 3:02
Describe        :
*/
int inFunc_NCCC_PrintReceipt_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_NONE_;
	char		szDispBuf[_DISP_MSG_SIZE_ + 1] = {0};
	char		szPrtMode[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char   uszKey = 0x00;

	/* 為了恢復被signpad清掉的title */
	inFunc_ResetTitle(pobTran);

	memset(szPrtMode, 0x00, sizeof(szPrtMode));
	inGetPrtMode(szPrtMode);

	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/* 列印商店聯 */
	/* 簽單模式不是0就印商店聯 */
	if (memcmp(szPrtMode, "0", strlen("0")) != 0)
	{
		if (pobTran->srTRec.inCode != _TICKET_IPASS_INQUIRY_	&&
		    pobTran->srTRec.inCode != _TICKET_EASYCARD_INQUIRY_)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			/* 列印帳單中 */
			inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);
		}

		while (1)
		{
			pobTran->srTRec.inPrintOption = _PRT_MERCH_;
			inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
			{
				break;
			}
		}
	}

	/* 三聯式簽單，中間要多印一聯客戶聯 */
	if (memcmp(szPrtMode, "3", strlen("3")) == 0)
	{
		if (pobTran->srTRec.inCode != _TICKET_IPASS_INQUIRY_	&&
		    pobTran->srTRec.inCode != _TICKET_EASYCARD_INQUIRY_)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			/* 列印帳單中 */
			inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);
		}

		/* 列印商店存根聯 */
		while (1)
		{
			pobTran->srTRec.inPrintOption = _PRT_MERCH_DUPLICATE_;
			inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
			{
				break;
			}
		}
	}

	/* 提示訊息 */
/* 目前沒有用 20190327 [SAM] */	
//	inNCCC_Ticket_Display_Transaction_Result(pobTran);

        /* 第一聯印完後15秒未按確認或清除鍵則端末機嗶嗶聲提示，響15秒後自動出顧客聯 */
	/* 客製化107顯示訊息TimeOut 2秒 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, strlen(_CUSTOMER_INDICATOR_107_BUMPER_)))
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
		inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
	}
	else
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_2_, 30);
		inDISP_Timer_Start(_TIMER_NEXSYS_3_, 15);
	}

	/* 因為查詢不印，所以只顯示確認鍵 */
	if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ ||
	    pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_)
	{
		inTouchSensorFunc = _Touch_BATCH_END_;

		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			uszKey = uszKBD_Key();

			if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
			{
				uszKey = _KEY_TIMEOUT_;
			}

			if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
			{
				inDISP_BEEP(1, 250);
			}

			if (uszKey == _KEY_ENTER_			||
			    uszKey == _KEY_TIMEOUT_			||
			    inChoice == _BATCH_END_Touch_ENTER_BUTTON_)
				break;
			else
				continue;
		}
		/* 清掉觸控殘值 */
		inDisTouch_Flush_TouchFile();
	}
	else
	{
		inTouchSensorFunc = _Touch_CUST_RECEIPT_;

		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			uszKey = uszKBD_Key();

			if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
			{
				uszKey = _KEY_TIMEOUT_;
			}

			if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
			{
				inDISP_BEEP(1, 250);
			}

			if (uszKey == _KEY_ENTER_			||
			    uszKey == _KEY_TIMEOUT_			||
			    inChoice == _CUSTReceipt_Touch_ENTER_)
			{
				break;
			}
			else if (uszKey == _KEY_CANCEL_			||
				 inChoice == _CUSTReceipt_Touch_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				break;
			}
			else
			{
				continue;
			}
		}

		/* 清掉觸控殘值 */
		inDisTouch_Flush_TouchFile();

		/* 按取消不印顧客聯 */
		if (inRetVal == VS_USER_CANCEL)
		{
			return (VS_SUCCESS);
		}
	}

	/* 列印顧客聯 */
	/* 三聯式簽單，兩聯以上就要印顧客聯 */
	if (memcmp(szPrtMode, "2", strlen("2")) == 0	||
	    memcmp(szPrtMode, "3", strlen("3")) == 0)
	{
		while (1)
		{
			/* 列印中 */
			/* 詢卡不用印 */
			if (pobTran->srTRec.inCode != _TICKET_IPASS_INQUIRY_	&&
			    pobTran->srTRec.inCode != _TICKET_EASYCARD_INQUIRY_)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_PRINTING_, 0, _COORDINATE_Y_LINE_8_7_);
			}

			memset(szDispBuf, 0x00, sizeof(szDispBuf));
			sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
			inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 11);

			pobTran->srTRec.inPrintOption = _PRT_CUST_;
			inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
			{
				break;
			}
		}
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_RePrintReceipt_ByBuffer_Flow
Date&Time       :2017/2/21 下午 4:52
Describe        :重印帳單分流
*/
int inFunc_RePrintReceipt_ByBuffer_Flow(TRANSACTION_OBJECT *pobTran)
{
//	char	szTemplate[6 + 1];
	char	szESCMode[2 + 1];
	char	szTRTFileName[16 + 1];
//	char	szTxnType[20 + 1];
	
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);

	if (memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)) == 0	||
	    memcmp(szTRTFileName, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)) == 0)
	{
		/* (6)	ESC功能僅包含中心NCCC Host(含HappyGo混合交易)及DCC Host之交易(不含大來卡) */
		/* ESC開關 */
		memset(szESCMode, 0x00, sizeof(szESCMode));
		inGetESCMode(szESCMode);

		/* 沒ESC的狀況，直接印紙本，不須另外判斷 */
		if (szESCMode[0] != 'Y')
		{
			inFunc_NCCC_PrintReceipt_ByBuffer(pobTran);
		}
		/* 有ESC的狀況 */
		else
		{
			inFunc_NCCC_RePrintReceipt_ByBuffer_ESC(pobTran);
		}

	}
	else if (memcmp(szTRTFileName, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)) == 0)
	{
		inFunc_NCCC_PrintReceipt_ByBuffer_ESVC(pobTran);
	}
	else if (memcmp(szTRTFileName, _TRT_FILE_NAME_CMAS_, strlen(_TRT_FILE_NAME_CMAS_)) == 0)
	{	/* [新增電票悠遊卡功能] 新增列印條件 [SAM] 2022/6/21 下午 4:40 */
		inCMAS_Func_PrintReceipt_ByBuffer(pobTran);
	}
	/* 其他Host，如DINERS */
	else
	{
		inFunc_PrintReceipt_ByBuffer(pobTran);
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_NCCC_RePrintReceipt_ByBuffer_ESC
Date&Time       :2017/3/22 下午 1:45
Describe        :和inFunc_NCCC_PrintReceipt_ByBuffer_ESC相同，但拿掉商店聯Copy的判斷
*/
int inFunc_NCCC_RePrintReceipt_ByBuffer_ESC(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CUST_RECEIPT_;
	char		szDispBuf[_DISP_MSG_SIZE_ + 1] = {0};
	char		szPrtMode[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char   uszKey = 0x00;

	/* 為了恢復被signpad清掉的title */
	inFunc_ResetTitle(pobTran);

	memset(szPrtMode, 0x00, sizeof(szPrtMode));
	inGetPrtMode(szPrtMode);

	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	if (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_NOT_SUPPORTED_	||
	    pobTran->srBRec.inESCUploadMode == _ESC_STATUS_OVERLIMIT_		||
	   (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_SUPPORTED_		&&
	    pobTran->srBRec.inSignStatus == _SIGN_BYPASS_))
	{
		/* 列印商店聯 */
		/* 簽單模式不是0就印商店聯 */
		if (memcmp(szPrtMode, "0", strlen("0")) != 0)
		{
			/* 列印帳單中 */
			inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

			while (1)
			{
				pobTran->srBRec.inPrintOption = _PRT_MERCH_;
				inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(pobTran);

				/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
				if (inRetVal == VS_PRINTER_PAPER_OUT ||
				    inRetVal == VS_PRINTER_OVER_HEAT)
					continue;
				else
				{
					/* 成功就砍簽名圖檔 */
					inFunc_Delete_Signature(pobTran);

					break;
				}

			}
		}

		if (inNEXSYS_ESC_MerchantCopy_Check(pobTran) == VS_SUCCESS	||
		   (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_SUPPORTED_	&&
		    pobTran->srBRec.inSignStatus == _SIGN_BYPASS_))
		{
			/* 列印帳單中 */
			inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

			/* 三聯式簽單，中間要多印一聯客戶聯 */
			if (memcmp(szPrtMode, "3", strlen("3")) == 0)
			{
				/* 列印商店存根聯 */
				while (1)
				{
					pobTran->srBRec.inPrintOption = _PRT_MERCH_DUPLICATE_;
					inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(pobTran);

					/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
					if (inRetVal == VS_PRINTER_PAPER_OUT ||
					    inRetVal == VS_PRINTER_OVER_HEAT)
					    continue;
					else
					{
						break;
					}
				}
			}
		}

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			/* SmartPay交易要提示調閱編號FiscRRN */
			inDISP_PutGraphic(_CHECK_SIGNATURE_3_, 0, _COORDINATE_Y_LINE_8_4_);

			memset(szDispBuf, 0x00, sizeof(szDispBuf));
			sprintf(szDispBuf, "%s", pobTran->srBRec.szFiscRRN);
			inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 9);
		}
		else
		{
			if (pobTran->srBRec.uszF56NoSignatureBit == VS_TRUE)
			//if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
			{
				/* 提示免簽名和授權碼 */
				inDISP_PutGraphic(_CHECK_SIGNATURE_5_, 0, _COORDINATE_Y_LINE_8_4_);
			}
			else
			{
				/* 提示檢核簽名和授權碼 */
				inDISP_PutGraphic(_CHECK_SIGNATURE_1_, 0, _COORDINATE_Y_LINE_8_4_);
			}

			memset(szDispBuf, 0x00, sizeof(szDispBuf));
			sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
			inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 11);
		}

		/* 第一聯印完後15秒未按確認或清除鍵則端末機嗶嗶聲提示，響15秒後自動出第二聯 */
		/* 客製化107顯示訊息TimeOut 2秒 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, strlen(_CUSTOMER_INDICATOR_107_BUMPER_)))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
			inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
		}
		else
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_2_, 30);
			inDISP_Timer_Start(_TIMER_NEXSYS_3_, 15);
		}
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			uszKey = uszKBD_Key();

			if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
			{
				uszKey = _KEY_TIMEOUT_;
			}

			if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
			{
				inDISP_BEEP(1, 250);
			}

			if (uszKey == _KEY_ENTER_			||
			    uszKey == _KEY_TIMEOUT_			||
			    inChoice == _CUSTReceipt_Touch_ENTER_)
			{
				inRetVal = VS_SUCCESS;
				break;
			}
			else if (uszKey == _KEY_CANCEL_			||
			     inChoice == _CUSTReceipt_Touch_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				break;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();

		/* 因為這裡交易已完成，所以一定回傳成功*/
		if (inRetVal == VS_USER_CANCEL)
		{
			return (VS_SUCCESS);
		}
	}

	/* 三聯式簽單，兩聯以上就要印顧客聯 */
	if (memcmp(szPrtMode, "2", strlen("2")) == 0	||
	    memcmp(szPrtMode, "3", strlen("3")) == 0)
	{
		/* 列印顧客聯 */
		while (1)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_PRINTING_, 0, _COORDINATE_Y_LINE_8_7_);

			pobTran->srBRec.inPrintOption = _PRT_CUST_;
			inRetVal = inCREDIT_PRINT_Receipt_Flow_ByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
			{
				break;
			}
		}
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_PrintTotalReport_ByBuffer
Date&Time       :2016/2/24 下午 4:26
Describe        :FuncTable.c列印總額帳單
*/
int inFunc_PrintTotalReport_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	char	szTRTFileName[16 + 1];

	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);

#ifdef _FUBON_MAIN_HOST_
	/* 富邦無人自助不需要列印  2020/5/20 上午 9:12 [SAM] */
	if(inFunc_GetKisokFlag() == VS_TRUE)
	{	
		return (VS_SUCCESS);
	}	
#endif	
	
	
	while (1)
	{
		if (memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)) == 0)
		{
			inRetVal = inCREDIT_PRINT_TotalReport_ByBuffer_NCCC(pobTran);
		}
		else if (memcmp(szTRTFileName, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)) == 0)
		{
//			inRetVal = inCREDIT_PRINT_TotalReport_ByBuffer_ESVC(pobTran);
		} else if (memcmp(szTRTFileName, _TRT_FILE_NAME_CMAS_, strlen(_TRT_FILE_NAME_CMAS_)) == 0)
		{
			/* [新增電票悠遊卡功能] 因為新增電票，所以列印方式採用合庫的格式
			 * 如果有需要不同格式再進行分流  [SAM] 2022/6/16 下午 1:58 */
			inRetVal = inCMAS_PRINT_TotalReport_ByBuffer(pobTran);
		}
		else
			
		{
			inRetVal = inCREDIT_PRINT_TotalReport_ByBuffer(pobTran);
		}

		/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
		if (inRetVal == VS_PRINTER_PAPER_OUT ||
		    inRetVal == VS_PRINTER_OVER_HEAT)
			continue;
		else
		{
			break;
		}
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_PrintDetailReport_ByBuffer
Date&Time       :2015/8/20 上午 10:24
Describe        : FuncTable.c列印明細帳單
 *  目前使用中 2022/10/21 [SAM]
*/
int inFunc_PrintDetailReport_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	char	szTRTFileName[16 + 1];

	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);

	while (1)
        {
		if (memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)) == 0)
		{
			inRetVal = inCREDIT_PRINT_NCCC_DetailReport_ByBuffer(pobTran);
		}
		else if (memcmp(szTRTFileName, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)) == 0)
		{
//			inRetVal = inCREDIT_PRINT_DetailReport_ByBuffer_ESVC(pobTran);
		}
		else if (memcmp(szTRTFileName, _TRT_FILE_NAME_CMAS_, strlen(_TRT_FILE_NAME_CMAS_)) == 0)
		{
			/* [新增電票悠遊卡功能] 新增明細列印條件  [SAM] 2022/6/21 下午 4:41 */
			inRetVal = inCMAS_PRINT_DetailReport_ByBuffer(pobTran);
		}
		else
		{
			inRetVal = inCREDIT_PRINT_DetailReport_ByBuffer(pobTran);
		}

		/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
		if (inRetVal == VS_PRINTER_PAPER_OUT ||
		    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
		else
		{
			break;
		}
	}

        return (VS_SUCCESS);
}


/*
Function        :inGetTransType
Date&Time       :2015/8/10 上午 10:24
Describe        :取得交易別
*/
int inFunc_GetTransType(TRANSACTION_OBJECT *pobTran, char *szPrintBuf1, char* szPrintBuf2)
{

	if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
	{
		switch (pobTran->srBRec.inCode)
		{
			case _SALE_ :
			case _CUP_SALE_ :
				strcpy(szPrintBuf1, "銷售");
				break;
			case _INST_SALE_ :
				strcpy(szPrintBuf1, "分期付款");
				break;
			case _REDEEM_SALE_ :
				strcpy(szPrintBuf1, "紅利抵扣");
				break;
			case _PRE_AUTH_ :
			case _CUP_PRE_AUTH_ :
				strcpy(szPrintBuf1, "預先授權");
				break;
			case _PRE_COMP_ :
				strcpy(szPrintBuf1, "預先授權完成");
				break;
			case _CUP_PRE_AUTH_VOID_ :
				strcpy(szPrintBuf1, "預先授權取消");
				break;
			case _CUP_PRE_COMP_ :
				strcpy(szPrintBuf1, "預先授權完成");
				break;
			case _CUP_PRE_COMP_VOID_ :
				strcpy(szPrintBuf1, "預先授權完成取消");
				break;
			case _SALE_OFFLINE_ :
				strcpy(szPrintBuf1, "交易補登");
				break;
			case _REFUND_ :
			case _CUP_REFUND_ :
				strcpy(szPrintBuf1, "退貨");
				break;
			case _REDEEM_REFUND_ :
				strcpy(szPrintBuf1, "紅利抵扣退貨");
				break;
			case _INST_REFUND_ :
				strcpy(szPrintBuf1, "分期付款退貨");
				break;
			case _TIP_ :
				strcpy(szPrintBuf1, "小費交易");
				break;
			case _ADJUST_ :
				strcpy(szPrintBuf1, "調帳");
				break;
			case _INST_ADJUST_ :
				strcpy(szPrintBuf1, "分期調帳");
				break;
			case _REDEEM_ADJUST_ :
				strcpy(szPrintBuf1, "紅利調帳");
				break;
			default :
				strcpy(szPrintBuf1, "!正向中文類別錯誤!");
				break;
		}
	}
	else
	{
		switch (pobTran->srBRec.inOrgCode)
		{
			case _SALE_ :
			case _CUP_SALE_ :
				strcpy(szPrintBuf1, "取消");
				break;
			case _INST_SALE_ :
				strcpy(szPrintBuf1, "分期付款取消");
				break;
			case _REDEEM_SALE_ :
				strcpy(szPrintBuf1, "紅利抵扣取消");
				break;
			case _CUP_PRE_AUTH_ :
				strcpy(szPrintBuf1, "預先授權取消");
				break;
			case _CUP_PRE_COMP_ :
				strcpy(szPrintBuf1, "預先授權完成取消");
				break;
			case _SALE_OFFLINE_ :
				strcpy(szPrintBuf1, "取消-交易補登");
				break;
			case _TIP_ :
				strcpy(szPrintBuf1, "取消-小費交易");
				break;
			case _ADJUST_ :
				strcpy(szPrintBuf1, "取消-調帳");
				break;
			case _INST_ADJUST_ :
				strcpy(szPrintBuf1, "取消-後台調帳-分期");
				//strcpy(szPrintBuf1, "取消-分期調帳");
				break;
			case _REDEEM_ADJUST_ :
				strcpy(szPrintBuf1, "取消-後台調帳-紅利");
				//strcpy(szPrintBuf1, "取消-紅利調帳");
				break;
			case _REFUND_ :
			case _CUP_REFUND_:
				strcpy(szPrintBuf1, "取消-退貨");
				break;
			case _REDEEM_REFUND_:
				strcpy(szPrintBuf1, "取消-退貨-紅利抵扣");
				break;
			case _INST_REFUND_:
				strcpy(szPrintBuf1, "取消-退貨-分期付款");
				break;
			case _PRE_AUTH_ :
				strcpy(szPrintBuf1, "取消-預先授權");
				break;
			case _PRE_COMP_ :/* [新增預授權完成] 修改判斷條件把 _PRE_COMP_ 獨立出來 2022/11/7 [SAM] */
				strcpy(szPrintBuf1, "取消-預先授權完成");			
				break;
			default :
				strcpy(szPrintBuf1, "!負向中文類別錯誤!");
				break;
		}
	}

	return (VS_SUCCESS);
}

void vdEDC_GetTransType(TRANSACTION_OBJECT *pobTran, char *szPrintBuf1, char* szPrintBuf2)
{
	if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
	{
		switch (pobTran->srBRec.inCode)
		{
			case _SALE_ :
			case _CUP_SALE_ :
				strcpy(szPrintBuf1, "銷售 SALE");
				strcpy(szPrintBuf2, "");
				break;
			case _SALE_OFFLINE_ :
				strcpy(szPrintBuf1, "交易補登 OFFLINE");
				strcpy(szPrintBuf2, "");
				break;
			case _REFUND_ :
			case _CUP_REFUND_ :
				strcpy(szPrintBuf1, "退貨 REFUND");
				strcpy(szPrintBuf2, "");
				break;
			case _INST_SALE_ :
				strcpy(szPrintBuf1, "分期付款 SALE INSTALLMENT");
				strcpy(szPrintBuf2, "");
				break;
			case _INST_REFUND_ :
				strcpy(szPrintBuf1, "分期付款退貨 REFUND INSTALLMENT");
				strcpy(szPrintBuf2, "");
				break;
			case _REDEEM_SALE_ :
				strcpy(szPrintBuf1, "紅利抵扣 SALE REDEMPTION");
				strcpy(szPrintBuf2, "");
				break;
			case _REDEEM_REFUND_ :
				strcpy(szPrintBuf1, "紅利抵扣退貨 REFUND REDEMPTION");
				strcpy(szPrintBuf2, "");
				break;
			case _TIP_ :
				strcpy(szPrintBuf1, "小費交易 TIPS");
				strcpy(szPrintBuf2, "");
				break;
			case _ADJUST_ :
				strcpy(szPrintBuf1, "調帳 ADJUST");
				strcpy(szPrintBuf2, "");
				break;
			case _REDEEM_ADJUST_ :
				strcpy(szPrintBuf1, "紅利抵扣調帳 ADJUST REDEMPTION");
				strcpy(szPrintBuf2, "");
				break;
			case _INST_ADJUST_ :
				strcpy(szPrintBuf1, "分期付款調帳 ADJUST INSTALLMENT");
				strcpy(szPrintBuf2, "");
				break;
			case _PRE_AUTH_ :
			case _CUP_PRE_AUTH_ :
				strcpy(szPrintBuf1, "預先授權 PREAUTH");
				strcpy(szPrintBuf2, "");
				break;
			case _PRE_COMP_ :
			case _CUP_PRE_COMP_ :
				strcpy(szPrintBuf1, "預先授權完成 PREAUTH COMPLETE");
				strcpy(szPrintBuf2, "");
				break;
			default :
				strcpy(szPrintBuf1, "!!!FUBON_正向交易!!!");
				strcpy(szPrintBuf2, "!!!交易類別錯誤!!!");
				break;
		}
	}
	else
	{
		switch (pobTran->srBRec.inOrgCode)
		{
			case _SALE_ :
			case _CUP_SALE_ :
				strcpy(szPrintBuf1, "取消 VOID");
				strcpy(szPrintBuf2, "");
				break;
			case _SALE_OFFLINE_ :
				strcpy(szPrintBuf1, "取消-交易補登 VOID OFFLINE");
				strcpy(szPrintBuf2, "");
				break;
			case _REFUND_ :
			case _CUP_REFUND_ :
				strcpy(szPrintBuf1, "取消-退貨 VOID REFUND");
				strcpy(szPrintBuf2, "");
				break;
			case _INST_SALE_ :
				strcpy(szPrintBuf1, "分期付款取消 VOID INSTALLMENT");
				strcpy(szPrintBuf2, "");
				break;
			case _REDEEM_SALE_ :
				strcpy(szPrintBuf1, "紅利抵扣取消 VOID REDEMPTION");
				strcpy(szPrintBuf2, "");
				break;
			/* added by sampo for Fubon */
			case _ADJUST_ :
				strcpy(szPrintBuf1, "取消-調帳 VOID ADJUST");
				strcpy(szPrintBuf2, "");
				break;
			case _REDEEM_ADJUST_ :
				strcpy(szPrintBuf1, "取消-後台調帳-紅利");
				strcpy(szPrintBuf2, "VOID ADJUST REDEEM");
				break;
			case _INST_ADJUST_ :
				strcpy(szPrintBuf1, "取消-後台調帳-分期");
				strcpy(szPrintBuf2, "VOID ADJUST INST.");
				break;
			case _REDEEM_REFUND_ :
				strcpy(szPrintBuf1, "取消-退貨-紅利抵扣");
				strcpy(szPrintBuf2, "VOID REFUND REDEEM");
				break;
			case _INST_REFUND_ :
				strcpy(szPrintBuf1, "取消-退貨-分期付款");
				strcpy(szPrintBuf2, "VOID REFUND INST.");
				break;
			case _CUP_PRE_AUTH_ :
				strcpy(szPrintBuf1, "預先授權取消 PREAUTH VOID");
				strcpy(szPrintBuf2, "");
				break;
			case _PRE_COMP_:// 新增類別 2022/10/21 [SAM]
			case _CUP_PRE_COMP_ :
				strcpy(szPrintBuf1, "預先授權完成取消");
				strcpy(szPrintBuf2, "PREAUTH COMPLETE VOID");
				break;
			default :
				strcpy(szPrintBuf1, "!!!FUBON_負向交易!!!");
				strcpy(szPrintBuf2, "!!!交易類別錯誤!!!");
				break;
		}
	}
}

/*
Function        :inFunc_GetTransType_ESVC
Date&Time       :2018/1/31 下午 1:44
Describe        :取得交易別
*/
int inFunc_GetTransType_ESVC(TRANSACTION_OBJECT *pobTran, char *szPrintBuf)
{
	char	szDebugMsg[100 + 1];

	if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_AUTO_TOP_UP_)
		strcat(szPrintBuf, "自動加值");
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_TOP_UP_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_)
		strcat(szPrintBuf, "手動加值");
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_REFUND_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_)
		strcat(szPrintBuf, "退貨　　");
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_)
		strcat(szPrintBuf, "購貨　　");
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_)
		strcpy(szPrintBuf, "加值取消");
	else
	{
		strcat(szPrintBuf, "NO Incode");

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inCode = %d", pobTran->srTRec.inCode);
			inDISP_LogPrintf(szDebugMsg);
		}
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_UpdateInvNum
Date&Time       :2015/10/15 上午 10:00
Describe        :Invoice Number加1，並且寫入HDPT.dat
*/
int inFunc_UpdateInvNum(TRANSACTION_OBJECT *pobTran)
{
//	int	inNCCC_HostIndex = -1;
//	int	inDCC_HostIndex = -1;
//	int	inHG_HostIndex = -1;
	char    szInvNum[6 + 1];
	char	szDemoMode[2 + 1] = {0};
	long    lnInvNum;

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inFunc_UpdateInvNum INIT");
	
	
	if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie("  UpdateInvNubm Load HDPT *Error* HID[%d]  Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
		/* 鎖機 */
		inFunc_EDCLock();
		return (VS_ERROR);
	}

	lnInvNum = pobTran->srBRec.lnOrgInvNum;

	lnInvNum ++;
	
	
#ifdef _FUBON_MAIN_HOST_
	/* 新增富邦Kiosk Demo使用功能 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	inDISP_LogPrintfWithFlag(" UpInv Demo[%s] Kiosk[%u] Line[%d]",szDemoMode, pobTran->uszKioskFlag, __LINE__);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		/* 富邦結帳設為500 筆 結帳Flag打開  2020/1/30 下午 4:33 [SAM] */
		if (lnInvNum >= 13)
		{
			inSetMustSettleBit("Y"); /* 有交易筆數限制，表示要結帳 */
			/* 開啟 TmsFPT 下載TMS要結帳的參數 */
			inEDCTMS_FUNC_SetTmsFtpSettleFlagOn();

			/* 因為這邊是成功交易，所以 pobTran->inECRErrorMsg 不應該有設定值 
			 * 所以利用 gsrECROb.srTransData.inErrorType 來斷交易成功時的回應碼 2020/1/30 下午 5:13 [SAM]
			 */
			if(pobTran->uszKioskFlag == 'Y')
				inECR_ECROB_SetErrorCode(_ECR_RESPONSE_CODE_NEAR_MAX_SETTLE_CNT_);
		}else{


			/* 因為這邊是成功交易，所以 pobTran->inECRErrorMsg 不應該有設定值 
			 * 所以利用 gsrECROb.srTransData.inErrorType 來斷交易成功時的回應碼
			 * 差距50筆時就提示  2020/1/30 下午 5:13 [SAM]
			 */
			if(pobTran->uszKioskFlag == 'Y')
			{
				if((13 - lnInvNum) <= 3 )
					inECR_ECROB_SetErrorCode(_ECR_RESPONSE_CODE_NEAR_MAX_SETTLE_CNT_);
			}
		}
	}else
	{
		/* 富邦結帳設為500 筆 結帳Flag打開  2020/1/30 下午 4:33 [SAM] */
		if (lnInvNum >= 501)
		{
			inSetMustSettleBit("Y"); /* 有交易筆數限制，表示要結帳 */
			/* 開啟 TmsFPT 下載TMS要結帳的參數 */
			inEDCTMS_FUNC_SetTmsFtpSettleFlagOn();

			/* 因為這邊是成功交易，所以 pobTran->inECRErrorMsg 不應該有設定值 
			 * 所以利用 gsrECROb.srTransData.inErrorType 來斷交易成功時的回應碼 2020/1/30 下午 5:13 [SAM]
			 */
			if(pobTran->uszKioskFlag == 'Y')
				inECR_ECROB_SetErrorCode(_ECR_RESPONSE_CODE_PLESE_SETTLE_);
		}else{


			/* 因為這邊是成功交易，所以 pobTran->inECRErrorMsg 不應該有設定值 
			 * 所以利用 gsrECROb.srTransData.inErrorType 來斷交易成功時的回應碼
			 * 差距50筆時就提示  2020/1/30 下午 5:13 [SAM]
			 */
			if(pobTran->uszKioskFlag == 'Y')
			{
				if((501- lnInvNum) <= 50 )
					inECR_ECROB_SetErrorCode(_ECR_RESPONSE_CODE_NEAR_MAX_SETTLE_CNT_);
			}
		}
	}
#else
	/* 結帳Flag打開 */
	if (lnInvNum >= 999999)
	{
		inSetMustSettleBit("Y"); /* 有交易筆數限制，表示要結帳 */
		/* 開啟 TmsFPT 下載TMS要結帳的參數 */
		inEDCTMS_FUNC_SetTmsFtpSettleFlagOn();
	}
#endif
	memset(szInvNum, 0x00, sizeof(szInvNum));
	sprintf(szInvNum, "%06ld", lnInvNum);   /* 因為HDPT要補滿6位數，因此補0 */
	if (inSetInvoiceNum(szInvNum) == VS_ERROR)
	{	
		inDISP_DispLogAndWriteFlie("  UpdateInvNubm Set InvoiceNum *Error* szInvNum[%s]  Line[%d]", szInvNum, __LINE__);
		/* 鎖機 */
		inFunc_EDCLock();
		return (VS_ERROR);
	}

	if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie("  UpdateInvNubm Save HDPT *Error* HID[%d]  Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
		/* 鎖機 */
		inFunc_EDCLock();
		return (VS_ERROR);
	}

/* 因為公司的TMS不會下HG 所以不需要同步 20190211 [SAM]*/
#if 0 		
	/* 如果是HG混合交易，則原先Host先在上面累加，接這在下面累加HG 的invoice Number */
	if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
	{
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_HG_, &inHG_HostIndex);

		if (inLoadHDPTRec(inHG_HostIndex) == VS_ERROR)
		{
			/* 鎖機 */
			inFunc_EDCLock();

			return (VS_ERROR);
		}

		memset(szInvNum, 0x00, sizeof(szInvNum));
		inGetInvoiceNum(szInvNum);

		lnInvNum = atol(szInvNum);
		lnInvNum ++;

		/* 結帳Flag打開 */
		if (lnInvNum >= 999999)
			inSetMustSettleBit("Y"); /* 有交易筆數限制，表示要結帳 */

		sprintf(szInvNum, "%06ld", lnInvNum);   /* 因為HDPT要補滿6位數，因此補0 */

		if (inSetInvoiceNum(szInvNum) == VS_ERROR)
		{
			/* 鎖機 */
			inFunc_EDCLock();

			return (VS_ERROR);
		}

		if (inSaveHDPTRec(inHG_HostIndex) == VS_ERROR)
		{
			/* 鎖機 */
			inFunc_EDCLock();

			return (VS_ERROR);
		}

		/* Load回來 */
		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			/* 鎖機 */
			inFunc_EDCLock();

			return (VS_ERROR);
		}

	}
#endif
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inFunc_UpdateInvNum 1!!");

/* 因為公司的TMS不會下HG 所以不需要同步 20190211 [SAM]*/
#ifdef __NCCC_DCC_FUNC__
	/* NCCC DCC HG 三個Host index同步 */
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_FUBON_, &inNCCC_HostIndex);
	//inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_MAINLY_USE_, &inNCCC_HostIndex);
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_DCC_, &inDCC_HostIndex);
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_HG_, &inHG_HostIndex);


	/* NCCC 或 DCC 或 HG 更新invoice number就需要同步，
	 * 如果HG混合交易(含NCCC、DINERS)，則原Host Update Invoice Number，HG在接著Update，
	 * 如果為純HG交易，則會選HG Update*/
	if (pobTran->srBRec.inHDTIndex == inNCCC_HostIndex	||
	    pobTran->srBRec.inHDTIndex == inDCC_HostIndex	||
	    pobTran->srBRec.inHDTIndex == inHG_HostIndex	||
	    pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
	{
		if (inNCCC_Func_Sync_InvoiceNumber(pobTran) != VS_SUCCESS)
		{
			/* 應同步而未同步要鎖機 */
			inFunc_EDCLock();

			return (VS_ERROR);
		}

	}
	
#endif

	/* 之後補上強制結帳的動作 */

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inFunc_UpdateInvNum END");
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inFunc_UpdateBatchNum
Date&Time       :2015/10/15 上午 10:00
Describe        :Batch Number加1，並且寫入HDPT.dat
*/
int inFunc_UpdateBatchNum(TRANSACTION_OBJECT *pobTran)
{
#ifdef  __NCCC_DCC_FUNC__	
	int	inNCCC_HostIndex = -1;
	int	inDCC_HostIndex = -1;
#endif
	char    szBatchNum[6 + 1];
	long    lnBatchNum;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	memset(szBatchNum, 0x00, sizeof(szBatchNum));
	if (inGetBatchNum(szBatchNum) == VS_ERROR)
	{
		inDISP_LogPrintfWithFlag("  inGetBatchNum Error ");
		return (VS_ERROR);
	}

	lnBatchNum = atol(szBatchNum);

	lnBatchNum ++;

	/* 超過999999，歸一 */
	if (lnBatchNum > 999999)
		lnBatchNum = 1;

	memset(szBatchNum, 0x00, sizeof(szBatchNum));
	sprintf(szBatchNum, "%06ld", lnBatchNum);   /* 因為HDPT要補滿6位數，因此補0 */
	if (inSetBatchNum(szBatchNum) == VS_ERROR)
	{
		inDISP_LogPrintfWithFlag("  inSetBatchNum Error ");
		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag("  HDTIndex [%d] ", pobTran->srBRec.inHDTIndex);
	
	if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
	{
		inDISP_LogPrintfWithFlag("  inSaveHDPTRec Error ");
		return (VS_ERROR);
	}

/* 目前用不到 20190327 [SAM] */
#ifdef  __NCCC_DCC_FUNC__
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_MAINLY_USE_, &inNCCC_HostIndex);
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_DCC_, &inDCC_HostIndex);


	/* 現在需要同步的Host有NCCC DCC HG(因為HG不結帳，所以只要NCCC或DCC變動，直接跟NCCC) */
	if (pobTran->srBRec.inHDTIndex == inNCCC_HostIndex	||
	    pobTran->srBRec.inHDTIndex == inDCC_HostIndex)
	{
		/* 如果是DCC or NCCC 就同步Batch Number */
		if (inNCCC_DCC_Sync_BatchNumber(pobTran) != VS_SUCCESS)
		{
			/* 應同步而未同步要鎖機 */
			inFunc_EDCLock();

			return (VS_ERROR);
		}
	}
#endif 
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inFunc_DeleteBatch
Date&Time       :2016/10/4 上午 11:49
Describe        :
*/
int inFunc_DeleteBatch(TRANSACTION_OBJECT *pobTran)
{
	int		i;
	int		inCode;
	int		inInvoiceNum;
	char            szHostName[8 + 1];
	unsigned char   uszFileName[20 + 1];
	TRANSACTION_OBJECT	pobTranTemp = {0};

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_RESET_BATCH_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <清除批次> */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	memset(szHostName, 0x00, sizeof(szHostName));
	if (inGetHostLabel(szHostName) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 顯示刪除批次請勿關機 */
	inDISP_PutGraphic(_DELETE_BATCH_, 0, _COORDINATE_Y_LINE_8_6_);
	/* 第二行中文提示 */
	inDISP_ChineseFont(szHostName, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
	/* 延遲一秒 */
	inDISP_Wait(1000);

	/* 刪除該host所有的簽名圖檔 因為調帳的簽名圖檔不一樣所以要多砍 */
	/* 這邊改incode只是為了組檔名用的， 所以砍完要設定回來 */
	inCode = pobTran->srBRec.inCode;
	memset(uszFileName, 0x00, sizeof(uszFileName));

	/* Load Batch Record 修改 20191029 [SAM]*/
	/* 這邊用pobTranTemp是避免影響後面流程(連動結帳)，會把之前的資料讀出來 */
	memset(&pobTranTemp, 0x00, sizeof(pobTranTemp));
	memcpy(&pobTranTemp, pobTran, sizeof(TRANSACTION_OBJECT));
	pobTranTemp.srBRec.lnOrgInvNum = _BATCH_LAST_RECORD_; /* 表示是最後一筆 */
	inBATCH_GetTransRecord_By_Sqlite(&pobTranTemp);
	inInvoiceNum = (int)pobTranTemp.srBRec.lnOrgInvNum;
	
	for (i = 1; i < inInvoiceNum; i++)
	{
		pobTran->srBRec.lnOrgInvNum = i;
		
		if (inFunc_ComposeFileName_InvoiceNumber(pobTran, (char*)uszFileName, _PICTURE_FILE_EXTENSION_, 6) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
		{
			inFILE_Delete(uszFileName);
		}
		
		/* VOID */
		pobTran->srBRec.inCode = _VOID_;
		if (inFunc_ComposeFileName_InvoiceNumber(pobTran, (char*)uszFileName, _PICTURE_FILE_EXTENSION_, 6) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
		{
			inFILE_Delete(uszFileName);
		}

		/* TIP */
		pobTran->srBRec.inCode = _TIP_;
		if (inFunc_ComposeFileName_InvoiceNumber(pobTran, (char*)uszFileName, _PICTURE_FILE_EXTENSION_, 6) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
		{
			inFILE_Delete(uszFileName);
		}

		/* ADJUST */
		pobTran->srBRec.inCode = _ADJUST_;
		if (inFunc_ComposeFileName_InvoiceNumber(pobTran, (char*)uszFileName, _PICTURE_FILE_EXTENSION_, 6) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
		{
			inFILE_Delete(uszFileName);
		}

	
	}
	pobTran->srBRec.inCode = inCode;

	/* Delete REVERSAL */
	memset(uszFileName, 0x00, sizeof(uszFileName));
	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
       
	if (inFILE_Delete(uszFileName) == VS_SUCCESS)
	{
		inSetSendReversalBit("N");
		inSaveHDPTRec(pobTran->srBRec.inHDTIndex);
	}
		
	/* Delete ISR REVERSAL 20190214 [SAM] */
	 memset(uszFileName, 0x00, sizeof(uszFileName));
	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_ISR_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
        
	if (inFILE_Delete(uszFileName) == VS_SUCCESS)
	{
		inSetMustISRUploadEnableBit("0");
		inSaveHDPTRec(pobTran->srBRec.inHDTIndex);
	}

	/* Delete ADVICE */
	memset(uszFileName, 0x00, sizeof(uszFileName));
	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	inFILE_Delete(uszFileName);

	/* Delete ADVICE ESC */
	memset(uszFileName, 0x00, sizeof(uszFileName));
	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _ADVICE_ESC_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	inFILE_Delete(uszFileName);

	/* 刪除該host的Table */
	inFunc_DeleteBatchTable(pobTran);

	/* 特例處理 START */
	/* 如果是NCCC要多砍優惠平台檔案和HappyGo的帳 */
/* 目前用不到 20190327 [SAM] */	
#if 0	
	if (memcmp(szHostName, _HOST_NAME_CREDIT_MAINLY_USE_, strlen(_HOST_NAME_CREDIT_MAINLY_USE_)) == 0)
	{
		/* 優惠平台檔案 */
		if (inFILE_Check_Exist((unsigned char*)_REWARD_FILE_NAME_) == VS_SUCCESS)
		{
			inFILE_Delete((unsigned char*)_REWARD_FILE_NAME_);
		}
	}
	/* 票證要刪除殘存檔 */
	else if (memcmp(szHostName, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)) == 0)
	{
		inFunc_Delete_Data("", "ICERAPI_CMAS.adv", _ECC_FOLDER_PATH_);
		inFunc_Delete_Data("", "ICERAPI_CMAS.rev", _ECC_FOLDER_PATH_);
		inFunc_Delete_Data("", "ICERAPI_CMAS.rev.bak", _ECC_FOLDER_PATH_);
	}
#endif 	
	
	/* [新增電票悠遊卡功能] 新增砍票證檔案的條件 [SAM] 2022/6/21 下午 4:56 */
	/* 票證要刪除殘存檔 */
	if (memcmp(szHostName, _HOST_NAME_CMAS_, strlen(_HOST_NAME_CMAS_)) == 0)
	{
		inFunc_Delete_Data("", "ICERAPI_CMAS.adv", _ECC_FOLDER_PATH_);
		inFunc_Delete_Data("", "ICERAPI_CMAS.rev", _ECC_FOLDER_PATH_);
		inFunc_Delete_Data("", "ICERAPI_CMAS.rev.bak", _ECC_FOLDER_PATH_);
	}
	/* 特例處理 END */

	/* 關閉Settle Bit */
	inSetMustSettleBit("N");
	/* 關閉續傳 Bit */
	inSetCLS_SettleBit("N");
	inSaveHDPTRec(pobTran->srBRec.inHDTIndex);

	/* 關閉 TmsFPT 下載TMS要結帳的參數 */
	inEDCTMS_FUNC_SetTmsFtpSettleFlagOff();
	
        return (VS_SUCCESS);
}

/*
Function        :inFunc_DeleteBatch_Flow
Date&Time       :2017/3/7 下午 1:53
Describe        :刪除批次，處理例外
*/
int inFunc_DeleteBatch_Flow(TRANSACTION_OBJECT *pobTran)
{
//	int	inHGIndex = -1;
//	int	inOrgIndex = -1;
	char    szHostName[8 + 1] = {0};
//	char	szFuncEable[2 + 1] = {0};

	/* 刪除批次 */
	inFunc_DeleteBatch(pobTran);

	memset(szHostName, 0x00, sizeof(szHostName));
	if (inGetHostLabel(szHostName) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

/* 因為公司的TMS不會下HG 所以不需要多砍 20190211 [SAM]*/	
#if 0	
	/* 特例處理 START */
	/* 如果是NCCC要多砍優惠平台檔案和HappyGo的帳 */
	if (memcmp(szHostName, _HOST_NAME_CREDIT_MAINLY_USE_, strlen(_HOST_NAME_CREDIT_MAINLY_USE_)) == 0)
	{
		/* HappyGo的帳 */
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_HG_, &inHGIndex);
		if (inHGIndex == -1)
		{
			return (VS_ERROR);
		}

		inOrgIndex = pobTran->srBRec.inHDTIndex;
		pobTran->srBRec.inHDTIndex = inHGIndex;

		inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

		memset(szFuncEable, 0x00, sizeof(szFuncEable));
		inGetHostEnable(szFuncEable);
		if (memcmp(szFuncEable, "Y", strlen("Y")) == 0)
		{
			inFunc_DeleteBatch(pobTran);
		}

		pobTran->srBRec.inHDTIndex = inOrgIndex;
		inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	}
#endif
	/* 特例處理 END */
        return (VS_SUCCESS);
}

/*
Function        :inFunc_DeleteAccum
Date&Time       :2016/10/4 上午 11:49
Describe        :
*/
int inFunc_DeleteAccum(TRANSACTION_OBJECT *pobTran)
{
        char            szHostName[8 + 1];
        unsigned char   uszFileName[20 + 1];

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_RESET_BATCH_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <清除批次> */

	memset(szHostName, 0x00, sizeof(szHostName));
        if (inGetHostLabel(szHostName) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 顯示刪除批次請勿關機 */
        inDISP_PutGraphic(_DELETE_BATCH_, 0, _COORDINATE_Y_LINE_8_6_);
        /* 第二行中文提示 */
        inDISP_ChineseFont(szHostName, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

        /* 延遲一秒 */
        inDISP_Wait(1000);

        /* Delete amt */
        memset(uszFileName, 0x00, sizeof(uszFileName));
	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _ACCUM_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
        inFILE_Delete(uszFileName);

        return (VS_SUCCESS);
}

/*
Function        :inFunc_DeleteAccum_Flow
Date&Time       :2017/3/7 下午 2:00
Describe        :刪除金額檔，並處理例外
*/
int inFunc_DeleteAccum_Flow(TRANSACTION_OBJECT *pobTran)
{
//	int	inHGIndex = -1;
//	int	inOrgIndex = -1;
	char    szHostName[8 + 1] = {0};
//	char	szFuncEable[2 + 1] = {0};

	inFunc_DeleteAccum(pobTran);

	memset(szHostName, 0x00, sizeof(szHostName));
	if (inGetHostLabel(szHostName) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
/* 因為公司的TMS不會下HG 所以不需要同步 20190211 [SAM]*/
#if 0	
	/* 特例處理 START */
	/* 如果是NCCC要多砍HappyGo總帳檔案 */
	if (memcmp(szHostName, _HOST_NAME_CREDIT_MAINLY_USE_, strlen(_HOST_NAME_CREDIT_MAINLY_USE_)) == 0)
	{
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_HG_, &inHGIndex);
		if (inHGIndex == -1)
		{
			return (VS_ERROR);
		}

		inOrgIndex = pobTran->srBRec.inHDTIndex;
		pobTran->srBRec.inHDTIndex = inHGIndex;

		inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

		memset(szFuncEable, 0x00, sizeof(szFuncEable));
		inGetHostEnable(szFuncEable);
		if (memcmp(szFuncEable, "Y", strlen("Y")) == 0)
		{
			inFunc_DeleteAccum(pobTran);
		}

		pobTran->srBRec.inHDTIndex = inOrgIndex;
		inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	}
	/* 特例處理 END */
#endif
        return (VS_SUCCESS);
}

/*
Function        :inFunc_CreateAllBatchTable
Date&Time       :2016/12/7 上午 9:54
Describe        :建立所有Host有開的Table
*/
int inFunc_CreateAllBatchTable(void)
{
	int			i;
	int			inRetVal;
	char			szHostEnable[2 + 1];
	TRANSACTION_OBJECT	pobTran;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_CreateAllBatchTable() START !");
	}

	memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));

	for (i = 0; ; i++)
	{
		if (inLoadHDTRec(i) < 0)
		{
			inDISP_LogPrintfWithFlag(" Func Create All Batch Table Load HDT[%d] *Error* Line[%d]", i, __LINE__);
			break;
		}
		pobTran.srBRec.inHDTIndex = i;

		inGetHostEnable(szHostEnable);

		if (memcmp(szHostEnable, "Y", 1) == 0)
		{
			inRetVal = inFunc_CreateNewBatchTable(&pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie(" Func Create All Batch Table New Batch *Error* Line[%d]", __LINE__);
				inFunc_EDCLock();
				return (VS_ERROR);
			}

		}
		else
		{
			/* 沒開Host跳過 */
			continue;
		}

	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
Function        :inFunc_DeleteAllBatchTable
Date&Time       :2016/12/7 上午 9:54
Describe        :刪除所有Host的Table
*/
int inFunc_DeleteAllBatchTable(void)
{
	int			i;
	TRANSACTION_OBJECT	pobTran;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));

	for (i = 0; ; i++)
	{
		if (inLoadHDTRec(i) < 0)
		{
			inDISP_LogPrintfWithFlag(" Func Delete All Batch Table Load HDT[%d] *Error* Line[%d]", i, __LINE__);
			break;
		}

		pobTran.srBRec.inHDTIndex = i;

		inFunc_DeleteBatchTable(&pobTran);

	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
Function        :inFunc_CreateNewBatchTable
Date&Time       :2016/12/7 上午 9:54
Describe        :結帳完建新的Batch Table
*/
int inFunc_CreateNewBatchTable(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;

	/* 結帳完建新的Batch Table */
	inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" Func CreateNewBathcTable Create  _TN_BATCH_TABLE_ *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}
	inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_EMV_TABLE_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" Func CreateNewBathcTable Create  _TN_EMV_TABLE_ *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" Func CreateNewBathcTable Create  _TN_BATCH_TABLE_ESC_AGAIN_ *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" Func CreateNewBathcTable Create  _TN_BATCH_TABLE_ESC_FAIL_ *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_DeleteBatchTable
Date&Time       :2016/12/7 上午 9:54
Describe        :刪除Batch Table
*/
int inFunc_DeleteBatchTable(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;

	inRetVal = inSqlite_Delete_Table_Flow(pobTran, _TN_BATCH_TABLE_);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	inRetVal = inSqlite_Delete_Table_Flow(pobTran, _TN_EMV_TABLE_);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	inRetVal = inSqlite_Delete_Table_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	inRetVal = inSqlite_Delete_Table_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_EMV_);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	inRetVal = inSqlite_Delete_Table_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	inRetVal = inSqlite_Delete_Table_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_EMV_);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	inRetVal = inSqlite_Delete_Table_Flow(pobTran, _TN_BATCH_TABLE_TICKET_);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	inRetVal = inSqlite_Delete_Table_Flow(pobTran, _TN_BATCH_TABLE_TICKET_ADVICE_);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_ResetBatchInvNum
Date&Time       :2016/10/4 上午 11:50
Describe        :結完帳時用或手動砍批
*/
int inFunc_ResetBatchInvNum(TRANSACTION_OBJECT *pobTran)
{
//	int	inNCCC_HostIndex = -1;
//	int	inDCC_HostIndex = -1;
//	int	inHG_HostIndex = -1;
        char    szInvNum[6 + 1];

        /* GET_HOST_NUM時就已經load HDPT了 */

        /* 結完帳，InvNum歸一 */
        memset(szInvNum, 0x00, sizeof(szInvNum));
        sprintf(szInvNum, "%06d", 1);   /* 因為HDPT要補滿6位數，因此補0 */

        if (inSetInvoiceNum(szInvNum) == VS_ERROR)
        {
                return (VS_ERROR);
        }

        if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
        {
                return (VS_ERROR);
        }

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inFunc_ResetBatchInvNum 1!!");
/* 因為公司的TMS不會下HG 所以不需要同步 20190211 [SAM]*/
#ifdef __NCCC_DCC_FUNC__
	/* NCCC DCC HG 三個Host index同步 */
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_MAINLY_USE_, &inNCCC_HostIndex);
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_DCC_, &inDCC_HostIndex);

	/* NCCC 或 DCC 或 HG 更新invoice number就需要同步，
	 * 如果HG混合交易(含NCCC、DINERS)，則原Host Update Invoice Number，HG在接著Update，
	 * 如果為純HG交易，則會選HG Update*/
	if (pobTran->srBRec.inHDTIndex == inNCCC_HostIndex	||
	    pobTran->srBRec.inHDTIndex == inDCC_HostIndex	||
	    pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
	{
		if (inNCCC_Func_Sync_Reset_InvoiceNumber(pobTran) != VS_SUCCESS)
		{
			/* 應同步而未同步要鎖機 */
			inFunc_EDCLock();

			return (VS_ERROR);
		}

	}
#endif
        return (VS_SUCCESS);
}

/*
Function        :inFunc_GetSystemDateAndTime
Date&Time       :2016/10/21 上午 11:42
Describe        :Get出端末機系統日期時間，放到自定義的結構
*/
int inFunc_GetSystemDateAndTime(RTC_NEXSYS *srRTC)
{
	int		inRetVal = 0;
	CTOS_RTC	srSysRTC;

	memset(&srSysRTC, 0x00, sizeof(srSysRTC));
	inRetVal = CTOS_RTCGet(&srSysRTC);

	if (inRetVal != d_OK)
	{
		inDISP_DispLogAndWriteFlie("  FuncGetSystemTime *Error* RetVal[0x%04X] Line[%d]", inRetVal, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		srRTC->uszYear = srSysRTC.bYear;
		srRTC->uszMonth = srSysRTC.bMonth;
		srRTC->uszDay = srSysRTC.bDay;
		srRTC->uszHour = srSysRTC.bHour;
		srRTC->uszMinute = srSysRTC.bMinute;
		srRTC->uszSecond = srSysRTC.bSecond;

		/* 不一定每一種機型都Get的到星期幾，所以可以斟酌是否要這個 */
		srRTC->uszDoW = srSysRTC.bDoW;

		return (VS_SUCCESS);
	}

}

/*
Function        :inFunc_SyncPobTran_Date_Include_Year
Date&Time       :2018/2/8 上午 9:40
Describe        :將傳進的RTC的時間部份，傳進放進來的Buffer,因為只傳pointer，所以要求傳進長度避免爆掉
 *		:這個會連20一起填
*/
int inFunc_SyncPobTran_Date_Include_Year(char *szDate,int inTimeBufferLen, RTC_NEXSYS *srRTC)
{

	if (inTimeBufferLen >= 8 + 1)
	{
		sprintf(szDate, "20%02d%02d%02d",  srRTC->uszYear, srRTC->uszMonth, srRTC->uszDay);
		return (VS_SUCCESS);
	}
	else
	{
		inDISP_DispLogAndWriteFlie("  FuncSyncPobtrans Date *Error* inTimeBufferLen[%d] Line[%d]", inTimeBufferLen, __LINE__);
		return (VS_ERROR);
	}
}

/*
Function        :inFunc_SyncPobTran_Date
Date&Time       :2018/2/8 上午 9:40
Describe        :將傳進的RTC的時間部份，傳進放進來的Buffer,因為只傳pointer，所以要求傳進長度避免爆掉
*/
int inFunc_SyncPobTran_Date(char *szDate,int inTimeBufferLen, RTC_NEXSYS *srRTC)
{
	if (inTimeBufferLen >= 6 + 1)
	{
		sprintf(szDate, "%02d%02d%02d",  srRTC->uszYear, srRTC->uszMonth, srRTC->uszDay);
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

/*
Function        :inFunc_SyncPobTran_Time
Date&Time       :2018/2/8 上午 9:40
Describe        :將傳進的RTC的時間部份，傳進放進來的Buffer,因為只傳pointer，所以要求傳進長度避免爆掉
*/
int inFunc_SyncPobTran_Time(char *szTime,int inTimeBufferLen, RTC_NEXSYS *srRTC)
{
	if (inTimeBufferLen >= 6 + 1)
	{
		sprintf(szTime, "%02d%02d%02d",  srRTC->uszHour, srRTC->uszMinute, srRTC->uszSecond);
		return (VS_SUCCESS);
	}
	else
	{
		inDISP_DispLogAndWriteFlie("  FuncSyncPobtrans Time *Error* inTimeBufferLen[%d] Line[%d]", inTimeBufferLen, __LINE__);
		return (VS_ERROR);
	}
}

/*
Function        :inFunc_Sync_BRec_Date_Time
Date&Time       :2018/2/8 上午 9:44
Describe        :同步BRec日期和時間
*/
int inFunc_Sync_BRec_Date_Time(TRANSACTION_OBJECT *pobTran, RTC_NEXSYS *srRTC)
{
	/* 同步到pobTran */
	memset(pobTran->srBRec.szDate, 0x00, sizeof(pobTran->srBRec.szDate));
	memset(pobTran->srBRec.szOrgDate, 0x00, sizeof(pobTran->srBRec.szOrgDate));
	memset(pobTran->srBRec.szTime, 0x00, sizeof(pobTran->srBRec.szTime));
	memset(pobTran->srBRec.szOrgTime, 0x00, sizeof(pobTran->srBRec.szOrgTime));

	if (inFunc_SyncPobTran_Date_Include_Year(pobTran->srBRec.szDate, sizeof(pobTran->srBRec.szDate),  srRTC) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" Sync Brec Year Date *Error* Line[%d] ", __LINE__);
		return (VS_ERROR);
	}

	if (inFunc_SyncPobTran_Date_Include_Year(pobTran->srBRec.szOrgDate, sizeof(pobTran->srBRec.szOrgDate),  srRTC) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" Sync Brec OrgDate Year *Error* Line[%d] ", __LINE__);
		return (VS_ERROR);
	}

	if (inFunc_SyncPobTran_Time(pobTran->srBRec.szTime, sizeof(pobTran->srBRec.szTime),  srRTC) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" Sync Brec Time *Error* Line[%d] ", __LINE__);
		return (VS_ERROR);
	}

	if (inFunc_SyncPobTran_Time(pobTran->srBRec.szOrgTime, sizeof(pobTran->srBRec.szOrgTime),  srRTC) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" Sync Brec OrgTime *Error* Line[%d] ", __LINE__);
		return (VS_ERROR);
	}
	
	inDISP_DispLogAndWriteFlie("  szDate[%s] szTime[%s] ", pobTran->srBRec.szDate, pobTran->srBRec.szTime);
	inDISP_DispLogAndWriteFlie("  szOrgDate[%s] szOrgTime[%s] ", pobTran->srBRec.szOrgDate, pobTran->srBRec.szOrgTime);
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Sync_TRec_Date_Time
Date&Time       :2018/2/8 上午 9:44
Describe        :同步TRec日期和時間
*/
int inFunc_Sync_TRec_Date_Time(TRANSACTION_OBJECT *pobTran, RTC_NEXSYS *srRTC)
{
	/* 同步到pobTran */
	memset(pobTran->srTRec.szDate, 0x00, sizeof(pobTran->srTRec.szDate));
	memset(pobTran->srTRec.szOrgDate, 0x00, sizeof(pobTran->srTRec.szOrgDate));
	memset(pobTran->srTRec.szTime, 0x00, sizeof(pobTran->srTRec.szTime));
	memset(pobTran->srTRec.szOrgTime, 0x00, sizeof(pobTran->srTRec.szOrgTime));

	if (inFunc_SyncPobTran_Date(pobTran->srTRec.szDate, sizeof(pobTran->srTRec.szDate),  srRTC) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" Sync Trec Date *Error* Line[%d] ", __LINE__);
		return (VS_ERROR);
	}

	if (inFunc_SyncPobTran_Date(pobTran->srTRec.szOrgDate, sizeof(pobTran->srTRec.szOrgDate),  srRTC) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" Sync Trec OrgDate *Error* Line[%d] ", __LINE__);
		return (VS_ERROR);
	}

	if (inFunc_SyncPobTran_Time(pobTran->srTRec.szTime, sizeof(pobTran->srTRec.szTime),  srRTC) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" Sync Trec Time *Error* Line[%d] ", __LINE__);
		return (VS_ERROR);
	}

	if (inFunc_SyncPobTran_Time(pobTran->srTRec.szOrgTime, sizeof(pobTran->srTRec.szOrgTime),  srRTC) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" Sync Trec OrgTime *Error* Line[%d] ", __LINE__);
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_SetEDCDateTime
Date&Time       :2016/11/25 下午 1:50
Describe        :更新端末機日期時間 輸入YYYYMMDD HHMMSS
*/
int inFunc_SetEDCDateTime(char *szDate, char *szTime)
{
	int	inYear, inMonth, inDay, inHour, inMinute, inSecond;
	char	szTemplate[8 + 1];
	unsigned short	usRet;
	CTOS_RTC	SetRTC;

	inDISP_LogPrintfWithFlag(" inFunc_SetEDCDateTime Input EDC Date[%s] Time[%s]",szDate, szTime);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	/* 年 */
	memcpy(&szTemplate[0], &szDate[2], 2);
	inYear = atoi(szTemplate);
	SetRTC.bYear = inYear;
	/* 月 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szDate[4], 2);
	inMonth = atoi(szTemplate);
	SetRTC.bMonth = inMonth;
	/* 日 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szDate[6], 2);
	inDay = atoi(szTemplate);
	SetRTC.bDay = inDay;
	/* 小時 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szTime[0], 2);
	inHour = atoi(szTemplate);
	SetRTC.bHour = inHour;
	/* 分鐘 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szTime[2], 2);
	inMinute = atoi(szTemplate);
	SetRTC.bMinute = inMinute;
	/* 秒 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szTime[4], 2);
	inSecond = atoi(szTemplate);
	SetRTC.bSecond = inSecond;

	/* 更改EDC時間 */
	usRet = CTOS_RTCSet(&SetRTC);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf(" inFunc_SetEDCDateTime (%04d/%02d/%02d %02d:%02d:%02d)=%d",SetRTC.bYear,SetRTC.bMonth,SetRTC.bDay,SetRTC.bHour,SetRTC.bMinute,SetRTC.bSecond,usRet);
	}

	if (usRet != d_OK)
	{
		inDISP_DispLogAndWriteFlie(" CTOS_RTCSet *Error* Ret[%x] Line[%d] ",usRet,  __LINE__);
		return (VS_ERROR);
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_Fun3EditDateTime
Date&Time       :2017/7/6 上午 10:28
Describe        :
*/
int inFunc_Fun3EditDateTime(void)
{
        int		inRetVal;
        char		szDate[8 + 1], szTime[6 + 1];
	char		szDispayMsg[8 + 1];
        unsigned char   uszKey;
        DISPLAY_OBJECT  srDispObj;
	RTC_NEXSYS	stRTC;

	memset(&stRTC, 0x00, sizeof(stRTC));
	inFunc_GetSystemDateAndTime(&stRTC);

        /* 日期設定 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        inDISP_PutGraphic(_SET_DATE_, 0, _COORDINATE_Y_LINE_8_4_);

	memset(szDispayMsg, 0x00, sizeof(szDispayMsg));
	sprintf(szDispayMsg, "20%02d%02d%02d", stRTC.uszYear, stRTC.uszMonth, stRTC.uszDay);
	inDISP_ChineseFont(szDispayMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);

        while (1)
        {
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                srDispObj.inMaxLen = 8;
                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

                inRetVal = inDISP_Enter8x16(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                        return (VS_ERROR);

                if (strlen(srDispObj.szOutput) != 8)
                {
                        inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
                        continue;
                }
                else
                {
                        memset(szDate, 0x00, sizeof(szDate));
                        memcpy(&szDate[0], &srDispObj.szOutput[0], 8);
                        break;
                }
        }

        /* 檢核日期 */
        inRetVal = inFunc_CheckValidDate_Include_Year(szDate);

        if (inRetVal != VS_SUCCESS)
        {
                /* 提示錯誤訊息 */
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                inDISP_PutGraphic(_ERR_DATE_TIME_, 0, _COORDINATE_Y_LINE_8_4_);
                inDISP_BEEP(1, 0);

                while (1)
                {
                        uszKey = uszKBD_GetKey(30);

                        if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
                                return (VS_ERROR);
                        else
                                continue;
                }
        }

        /* 時間設定 */
        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        inDISP_PutGraphic(_SET_TIME_, 0, _COORDINATE_Y_LINE_8_4_);

	memset(szDispayMsg, 0x00, sizeof(szDispayMsg));
	sprintf(szDispayMsg, "%02d%02d%02d", stRTC.uszHour, stRTC.uszMinute, stRTC.uszSecond);
	inDISP_ChineseFont(szDispayMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);

        while (1)
        {
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                srDispObj.inMaxLen = 6;
                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

                inRetVal = inDISP_Enter8x16(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                        return (VS_ERROR);

                if (strlen(srDispObj.szOutput) != 6)
                {
                        inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
                        continue;
                }
                else
                {
                        memset(szTime, 0x00, sizeof(szTime));
                        memcpy(&szTime[0], &srDispObj.szOutput[0], 6);
                        break;
                }
        }

        /* 檢核時間 */
        inRetVal = inFunc_CheckValidTime(szTime);

        if (inRetVal != VS_SUCCESS)
        {
                /* 提示錯誤訊息 */
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                inDISP_PutGraphic(_ERR_DATE_TIME_, 0, _COORDINATE_Y_LINE_8_4_);
                inDISP_BEEP(1, 0);

                while (1)
                {
                        uszKey = uszKBD_GetKey(30);

                        if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
                                return (VS_ERROR);
                        else
                                continue;
                }
        }

        inFunc_SetEDCDateTime(szDate, szTime);

        return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckValidDate_Include_Year
Date&Time       :2016/11/25 下午 2:59
Describe        :確認日期是否合法
*/
int inFunc_CheckValidDate_Include_Year(char *szValidDate)
{
    	char	szTemplate[14 + 1];
    	int	inDay = 0, inMon = 0, inYear = 0;

    	/* Get 4-Digit Year */
    	memset(szTemplate, 0x00, sizeof(szTemplate));
    	memcpy(&szTemplate[0], &szValidDate[0], 4);
    	inYear = atoi(szTemplate);
    	/*  Get 2-Digit Month */
    	memset(szTemplate, 0x00, sizeof(szTemplate));
    	memcpy(&szTemplate[0], &szValidDate[4], 2);
    	inMon = atoi(szTemplate);
    	/*  Get 2-Digit Day */
    	memset(szTemplate, 0x00, sizeof(szTemplate));
    	memcpy(&szTemplate[0], &szValidDate[6], 2);
    	inDay = atoi(szTemplate);

    	if ((inYear < 2000) || (inYear > 2088))
        	return (VS_ERROR);

    	if ((inMon < 1) || (inMon > 12))
        	return (VS_ERROR);

    	if ((inDay < 1) || (inDay > 31))
        	return (VS_ERROR);

        switch (inMon)
        {
                case 4 :
                case 6 :
                case 9 :
                case 11 :
                        if (inDay > 30)
                            return (VS_ERROR);
                        break;
                case 2:
                        if (inYear % 4 == 0) /* 判斷閏年 */
                        {
                                if (inDay > 29)
                                {
                                        return (VS_ERROR);
                                }
                        }
                        else
                        {
                                if (inDay > 28)
                                {
                                        return (VS_ERROR);
                                }
                        }
                        break;
                default :
                        break;
        }

    	return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckValidOriDate
Date&Time       :2018/2/13 下午 3:36
Describe        :確認原交易日期是否合法
*/
int inFunc_CheckValidOriDate(char *szValidDate)
{
	int		inDay = 0, inMon = 0, inYear = 0;
    	char		szTemplate[8 + 1];
	RTC_NEXSYS	srRTC = {0};

    	/*  Get 2-Digit Month */
    	memset(szTemplate, 0x00, sizeof(szTemplate));
    	memcpy(&szTemplate[0], &szValidDate[0], 2);
    	inMon = atoi(szTemplate);
    	/*  Get 2-Digit Day */
    	memset(szTemplate, 0x00, sizeof(szTemplate));
    	memcpy(&szTemplate[0], &szValidDate[2], 2);
    	inDay = atoi(szTemplate);

	/* 判斷去年、今年 */
	inFunc_GetSystemDateAndTime(&srRTC);
	if (inMon <= srRTC.uszMonth)
	{
		if (inDay > srRTC.uszDay)
		{
			/* 去年 */
			inYear = srRTC.uszYear - 1;
		}
		else
		{
			/* 今年 */
			inYear = srRTC.uszYear;
		}
	}
	else
	{
		/* 去年 */
		inYear = srRTC.uszYear - 1;
	}

    	if ((inMon < 1) || (inMon > 12))
        	return (VS_ERROR);

    	if ((inDay < 1) || (inDay > 31))
        	return (VS_ERROR);

        switch (inMon)
        {
                case 4 :
                case 6 :
                case 9 :
                case 11 :
                        if (inDay > 30)
                            return (VS_ERROR);
                        break;
                case 2:
                        if (inYear % 4 == 0) /* 判斷閏年 */
                        {
                                if (inDay > 29)
                                {
                                        return (VS_ERROR);
                                }
                        }
                        else
                        {
                                if (inDay > 28)
                                {
                                        return (VS_ERROR);
                                }
                        }
                        break;
                default :
                        break;
        }

    	return (VS_SUCCESS);
}

int inFunc_CheckValidTime(char *szTime)
{
	char	szTemplate[4 + 1];
	int 	inMins = 0, inHours = 0, inSecs = 0;

   	/*  Get 2-Digit Hours */
   	memset(szTemplate, 0x00, sizeof(szTemplate));
    	memcpy(&szTemplate[0], &szTime[0], 2);
    	inHours = atoi(szTemplate);
    	/*  Get 2-Digit Minutes */
   	memset(szTemplate, 0x00, sizeof(szTemplate));
    	memcpy(&szTemplate[0], &szTime[2], 2);
    	inMins = atoi(szTemplate);
    	/*  Get 2-Digit Seconds */
   	memset(szTemplate, 0x00, sizeof(szTemplate));
    	memcpy(&szTemplate[0], &szTime[4], 2);
    	inSecs = atoi(szTemplate);

	if ((inHours < 0) || (inHours > 23))
		return (VS_ERROR);

	if ((inMins < 0) || (inMins > 59))
		return (VS_ERROR);

	if ((inSecs < 0) || (inSecs > 59))
		return (VS_ERROR);

	return (VS_SUCCESS);
}

/*
Function        :inFunc_SunDay_Sum
Date&Time       :2016/2/24 下午 1:13
Describe        :輸入Date，算出太陽日，2000年起算，YYYYMMDD
*/
int inFunc_SunDay_Sum(char *szDate)
{
        int     inSunDay = 0;
        int     inYear, inMonth, inDay, inLeap = 0;
        char    szYear[2 + 1], szMonth[2 + 1], szDay[2 + 1];
        VS_BOOL fLeap_Year = VS_FALSE;

        /* 年份 */
        memset(szYear, 0x00, sizeof(szYear));
        memcpy(&szYear[0], &szDate[2], 2);
        inYear = atoi(szYear);

        if (inYear > 0)
        {
                inLeap = inYear / 4;
                /* 算到前一年日數總和 */
                inSunDay = 365 * (inYear - 1) + inLeap;
        }

        /* 判斷閏年 */
	if (inYear % 4 == 0)
		fLeap_Year = VS_TRUE;
	else
		fLeap_Year = VS_FALSE;

        /* 月份 */
        memset(szMonth, 0x00, sizeof(szMonth));
        memcpy(&szMonth[0], &szDate[4], 2);
        inMonth = atoi(szMonth);

        switch (inMonth)
        {
                case 1 :
                        inSunDay = inSunDay + 0;
                        break;
                case 2 :
                        inSunDay = inSunDay + 31;
                        break;
                case 3 :
                        if (fLeap_Year == VS_TRUE)
                                inSunDay = inSunDay + 60;
                        else
                                inSunDay = inSunDay + 59;
                        break;
                case 4 :
                        if (fLeap_Year == VS_TRUE)
                                inSunDay = inSunDay + 91;
                        else
                                inSunDay = inSunDay + 90;
                        break;
                case 5 :
                        if (fLeap_Year == VS_TRUE)
                                inSunDay = inSunDay + 121;
                        else
                                inSunDay = inSunDay + 120;
                        break;
                case 6 :
                        if (fLeap_Year == VS_TRUE)
                                inSunDay = inSunDay + 152;
                        else
                                inSunDay = inSunDay + 151;
                        break;
                case 7 :
                        if (fLeap_Year == VS_TRUE)
                                inSunDay = inSunDay + 182;
                        else
                                inSunDay = inSunDay + 181;
                        break;
                case 8 :
                        if (fLeap_Year == VS_TRUE)
                                inSunDay = inSunDay + 213;
                        else
                                inSunDay = inSunDay + 212;
                        break;
                case 9 :
                        if (fLeap_Year == VS_TRUE)
                                inSunDay = inSunDay + 244;
                        else
                                inSunDay = inSunDay + 243;
                        break;
                case 10 :
                        if (fLeap_Year == VS_TRUE)
                                inSunDay = inSunDay + 274;
                        else
                                inSunDay = inSunDay + 273;
                        break;
                case 11 :
                        if (fLeap_Year == VS_TRUE)
                                inSunDay = inSunDay + 305;
                        else
                                inSunDay = inSunDay + 304;
                        break;
                case 12 :
                        if (fLeap_Year == VS_TRUE)
                                inSunDay = inSunDay + 335;
                        else
                                inSunDay = inSunDay + 334;
                        break;
                default :
                        return (VS_ERROR);
        } /* end switch 當年度太陽日 */

        /* 日期 */
        memset(szDay, 0x00, sizeof(szDay));
        memcpy(&szDay[0], &szDate[6], 2);
        inDay = atoi(szDate);
        inSunDay = inSunDay + inDay;

        return (inSunDay);
}

/*
Function        :inFunc_SunDay_Sum_Check_In_Range
Date&Time       :2017/2/2 下午 3:27
Describe        :輸入Date，算出太陽日，2000年起算，YYYYMMDD，若Date1在Date2和Date3內(含那一天)，Return VS_SUCCESS，否則回傳VS_ERROR
*/
int inFunc_SunDay_Sum_Check_In_Range(char *szDate1, char *szDate2, char *szDate3)
{
	int	i;
	int	inSumDay1, inSumDay2, inSumDay3;
	char	szYear[4 + 1];

	/* 先檢核日期是否為合法字元 */
	for (i = 0; i < 8; i ++)
	{
		if (szDate1[i] < '0' || szDate1[i] > '9')
		{
			return (VS_ERROR);
		}

	}

	for (i = 0; i < 8; i ++)
	{
		if (szDate2[i] < '0' || szDate2[i] > '9')
		{
			return (VS_ERROR);
		}

	}

	for (i = 0; i < 8; i ++)
	{
		if (szDate3[i] < '0' || szDate3[i] > '9')
		{
			return (VS_ERROR);
		}

	}

	/* 檢核年是否小於2000年 */
	memset(szYear, 0x00, sizeof(szYear));
	memcpy(szYear, szDate1, 4);
	if (atoi(szYear) < 2000)
	{
		return (VS_ERROR);
	}

	memset(szYear, 0x00, sizeof(szYear));
	memcpy(szYear, szDate2, 4);
	if (atoi(szYear) < 2000)
	{
		return (VS_ERROR);
	}

	memset(szYear, 0x00, sizeof(szYear));
	memcpy(szYear, szDate3, 4);
	if (atoi(szYear) < 2000)
	{
		return (VS_ERROR);
	}

	/* 算出太陽日 */
	inSumDay1 = inFunc_SunDay_Sum(szDate1);
	inSumDay2 = inFunc_SunDay_Sum(szDate2);
	inSumDay3 = inFunc_SunDay_Sum(szDate3);

	if ((inSumDay1 < inSumDay2) || (inSumDay1 > inSumDay3))
	{
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :lnFuncGetTimeToUnix
Date&Time       :2017/12/21 下午 4:43
Describe        :輸出格林威治的時間，從1970開始算的秒數
*/
int lnFuncGetTimeToUnix(RTC_NEXSYS *srRTC, char *szOutput)
{
	int		inDaysOfMonth[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
        int		inTemp = 0, inDiffTimeZone = 0;
        int		inLeapYearCount = 0;
        char		szTemp[12] = {0};
	unsigned long	ulSec = 0;

        /* 台灣與格林威治時差 */
        inDiffTimeZone = +8;

        /* 計算閏年 */
        inTemp = (2000 + srRTC->uszYear) - 1970;
        inLeapYearCount = 0;

        inLeapYearCount = ((inTemp + 2) / 4);		/* 經過幾個閏年 */

        if ((inTemp + 2) % 4 == 0)			/* 是閏年 */
        {
            if (srRTC->uszMonth <= 2)    /* 尚未超過02/29,要減1天回來 */
            {
                inLeapYearCount--;
            }
            else if((srRTC->uszMonth == 2) && (srRTC->uszDay == 29)) /* 剛好02/29,要加1天 */
            {
                inLeapYearCount++;
            }
        }

        ulSec = inLeapYearCount * 1+ inTemp * 365 + inDaysOfMonth[srRTC->uszMonth - 1] + srRTC->uszDay - 1;

	/* 換算成小時，把時差減回來 */
        ulSec = ulSec * 24 + srRTC->uszHour - inDiffTimeZone;
	/* 換算成分鐘，並把分鐘加上去 */
        ulSec = ulSec * 60 + srRTC->uszMinute;
	/* 換算成秒數，並把秒數加上去 */
        ulSec = ulSec * 60 + srRTC->uszSecond;

        memset(szTemp, 0x00, sizeof(szTemp));
        sprintf(szTemp, "%lu", ulSec);
        memcpy(&szOutput[0], &szTemp[0], 10);

        return (ulSec);
}

/*
Function        :inFunc_String_Dec_to_Hex_Little2Little
Date&Time       :2018/1/2 下午 6:23
Describe        :把十進位的數字字串轉成16進位的字串
*/
int inFunc_String_Dec_to_Hex_Little2Little(char *szInput, char *szOutput)
{
        char		szTemp[20 + 1] = {0};
	char		szDebugMsg[100 + 1];
	unsigned long	ulData = 0;

        ulData = strtoul(szInput, NULL, 10);

        memset(szTemp, 0x00, sizeof(szTemp));
        sprintf(szTemp, "%02X%02X%02X%02X", ((unsigned char*)&ulData)[0], ((unsigned char*)&ulData)[1], ((unsigned char*)&ulData)[2], ((unsigned char*)&ulData)[3]);
        memcpy(&szOutput[0], &szTemp[0], 8);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Unix time:%lu", ulData);
		inDISP_LogPrintf(szDebugMsg);

		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Output:%s", szOutput);
		inDISP_LogPrintf(szDebugMsg);
	}

        return (VS_SUCCESS);
}


/*
Function        :inFunc_String_Hex_to_Dec_Big2Little
Date&Time       :2018/1/17 上午 10:29
Describe        :把16進位的數字字串轉成十進位的字串
*/
int inFunc_String_Hex_to_Dec_Big2Little(char *szInput, char *szOutput, int inInputLen)
{
	int		i = 0;
	int		inTemp = 0;
        char		szTemp[100 + 1] = {0};
	char		szDebugMsg[100 + 1];
	unsigned long	ulData = 0;

	for (i = 0; *(szInput + i) != 0x00; i++)
	{
		if (*(szInput + i) >= '0' && *(szInput + i) <= '9')
		{
			inTemp = *(szInput + i) - '0';
		}
		else if (*(szInput + i) >= 'A' && *(szInput + i) <= 'F')
		{
			inTemp = *(szInput + i) - 'A';
			inTemp += 10;
		}
		else if (*(szInput + i) >= 'a' && *(szInput + i) <= 'f')
		{
			inTemp = *(szInput + i) - 'a';
			inTemp += 10;
		}
		else
		{
			return (VS_ERROR);
		}
		ulData = (16 * ulData) + inTemp;
		
	};
	
        memset(szTemp, 0x00, sizeof(szTemp));
        sprintf(szTemp, "%lu", ulData);
        strcpy(szOutput, szTemp);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Input:%s", szInput);
		inDISP_LogPrintf(szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Output:%s", szOutput);
		inDISP_LogPrintf (szDebugMsg);
	}

        return (VS_SUCCESS);
}



/*
Function        :inFunc_String_Hex_to_Dec_Little2Little
Date&Time       :2018/1/17 上午 10:29
Describe        :把16進位的數字字串轉成十進位的字串
*/
	
int inFunc_String_Hex_to_Dec_Little2Little(char *szInput, char *szOutput)
{
	int		i = 0;
	int		inTemp = 0;
        char		szTemp[100 + 1] = {0};
	char		szDebugMsg[100 + 1];
	unsigned long	ulData = 0;

	for (i = 0; *(szInput + i) != 0x00; i++)
	{
		if (*(szInput + i) >= '0' && *(szInput + i) <= '9')
		{
			inTemp = *(szInput + i) - '0';
		}
		else if (*(szInput + i) >= 'A' && *(szInput + i) <= 'F')
		{
			inTemp = *(szInput + i) - 'A';
			inTemp += 10;
		}
		else if (*(szInput + i) >= 'a' && *(szInput + i) <= 'f')
		{
			inTemp = *(szInput + i) - 'a';
			inTemp += 10;
		}
		else
		{
			return (VS_ERROR);
		}
		ulData = (16 * ulData) + inTemp;

	};

        memset(szTemp, 0x00, sizeof(szTemp));
        sprintf(szTemp, "%lu", ulData);
        strcpy(szOutput, szTemp);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Input:%s", szInput);
		inDISP_LogPrintf(szDebugMsg);

		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Output:%s", szOutput);
		inDISP_LogPrintf(szDebugMsg);
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_Big5toUTF8
Date&Time       :2016/3/8 下午 1:48
Describe        :BIG5編碼中文字轉UTF8編碼 (至現回報訊息用到)
*/
int inFunc_Big5toUTF8(char *szUTF8, char *szBIG5)
{
#ifndef	_LOAD_KEY_AP_
	iconv_t cd;
	size_t  stUTF8Len, stBIG5Len;
	char    szInput[4096 + 1]= {0}, *szInp = szInput;
	char    szOutput[2 * sizeof(szInput) + 1], *szOutp = szOutput;
	char    szTemplate[64 + 1];
	int conv = 0;

        if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inFunc_Big5toUTF8() START!");

        cd = iconv_open("UTF-8", "BIG5");

	if (cd == NULL)
	{
        if (ginDebug == VS_TRUE)
        {
			sprintf(szTemplate, "iconv_open Error !! cd = %d", (int)cd);
			inDISP_LogPrintf(szTemplate);
		}
		iconv_close(cd);
		
		return (VS_ERROR);
        }

        memset(szInput, 0x00, sizeof(szInput));
        memcpy(&szInput[0], &szBIG5[0], strlen(szBIG5));
        memset(szOutput, 0x00, sizeof(szOutput));

        stBIG5Len = strlen(szInput);
        stUTF8Len = stBIG5Len * 2;

        if (ginDebug == VS_TRUE)
        {
                sprintf(szTemplate, "stBIG5Len = %d", stBIG5Len);
                inDISP_LogPrintf(szTemplate);
                sprintf(szTemplate, "stUTF8Len = %d", stUTF8Len);
                inDISP_LogPrintf(szTemplate);
        }

        conv = iconv(cd, &szInp, &stBIG5Len, &szOutp, &stUTF8Len);

        if (ginDebug == VS_TRUE)
        {
                sprintf(szTemplate, "iconv = %d", conv);
                inDISP_LogPrintf(szTemplate);
                inDISP_LogPrintf(szOutput);
        }

        memcpy(&szUTF8[0], &szOutput[0], strlen(szOutput));

        iconv_close(cd);
#endif

	return (VS_SUCCESS);
}

/*
Function        :inFunc_UTF8toBig5
Date&Time       :2017/4/27 下午 5:27
Describe        :UTF8編碼轉BIG5編碼中文字
*/
int inFunc_UTF8toBig5(char *szBIG5, char *szUTF8)
{
#ifndef	_LOAD_KEY_AP_
        iconv_t cd;
        size_t  stUTF8Len, stBIG5Len;
        char    szOutput[1024 + 1], *szOutp = szOutput;
        char    szInput[1024 + 1], *szInp = szInput;
        char    szTemplate[64 + 1];
        int conv;

        if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inFunc_Big5toUTF8() START!");

        cd = iconv_open("BIG5", "UTF-8");

        if (cd == NULL)
	{
        if (ginDebug == VS_TRUE)
        {
                sprintf(szTemplate, "iconv_open Error !! cd = %d", (int)cd);
			inDISP_LogPrintf(szTemplate);
		}
		iconv_close(cd);
		
		return (VS_ERROR);
        }

        memset(szInput, 0x00, sizeof(szInput));
        memcpy(&szInput[0], &szUTF8[0], strlen(szUTF8));
        memset(szOutput, 0x00, sizeof(szOutput));

        stUTF8Len = strlen(szInput);
        stBIG5Len = stUTF8Len;

        if (ginDebug == VS_TRUE)
        {
                sprintf(szTemplate, "stBIG5Len = %d", stBIG5Len);
                inDISP_LogPrintf(szTemplate);
                sprintf(szTemplate, "stUTF8Len = %d", stUTF8Len);
                inDISP_LogPrintf(szTemplate);
        }

        conv = iconv(cd, &szInp, &stUTF8Len, &szOutp, &stBIG5Len);

        if (ginDebug == VS_TRUE)
        {
                sprintf(szTemplate, "iconv = %d", conv);
                inDISP_LogPrintf(szTemplate);
                inDISP_LogPrintf(szOutput);
        }

        memcpy(&szBIG5[0], &szOutput[0], strlen(szOutput));

        iconv_close(cd);
#endif

	return (VS_SUCCESS);
}

void callbackFun(UINT inTotalProgress, UINT inCapProgress, BYTE *uszCurMCI, BYTE *uszCurCAP)
{
	char strBuf[64] = {0};

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	if (strlen((char*)uszCurMCI) > 0)
	{
		memset(strBuf, 0x00, sizeof(strBuf));
		sprintf(strBuf, "%s", uszCurMCI);
		inDISP_ChineseFont(strBuf, _FONTSIZE_16X22_, _LINE_16_13_, _DISP_LEFT_);
	}

	if (strlen((char*)uszCurCAP) > 0)
	{
		memset(strBuf, 0x00, sizeof(strBuf));
		sprintf(strBuf, "%s", uszCurCAP);
		inDISP_ChineseFont(strBuf, _FONTSIZE_16X22_, _LINE_16_14_, _DISP_LEFT_);
	}

	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);

	inDISP_ChineseFont("總進度/單檔進度", _FONTSIZE_16X22_, _LINE_16_15_, _DISP_LEFT_);
	memset(strBuf, 0x00, sizeof(strBuf));
	sprintf(strBuf, "%3d%% / %3d%%", inTotalProgress, inCapProgress);
	inDISP_ChineseFont(strBuf, _FONTSIZE_16X22_, _LINE_16_16_, _DISP_LEFT_);
}

/*
Function	  : inFunc_CalculateRunTimeGlobal_Start
Date&Time : 2016/3/7 上午 10:43
Describe	  : 用來看精度秒以下的RunTime
*/
int inFunc_CalculateRunTimeGlobal_Start(void)
{
	gulRunTime = CTOS_TickGet();

	return (VS_SUCCESS);
}

/*
Function	: inFunc_GetRunTimeGlobal
Date&Time: 2016/3/7 上午 10:43
Describe	: 用來看精度秒以下的RunTime
*/
int inFunc_GetRunTimeGlobal(unsigned long *ulSecond, unsigned long *ulMilliSecond)
{
	char		szDebugMsg[100 + 1];
	unsigned long	ulRunTimeStart, ulRunTimeEnd, ulInterval;

	ulRunTimeStart = 0;
	ulRunTimeEnd = 0;
	ulInterval = 0;
	*ulMilliSecond = 0;
	*ulSecond = 0;


	ulRunTimeStart = gulRunTime;
	ulRunTimeEnd = CTOS_TickGet();

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "%lu %lu", ulRunTimeStart, ulRunTimeEnd);
		inDISP_LogPrintf(szDebugMsg);
	}


	ulInterval = ulRunTimeEnd - ulRunTimeStart;
	*ulSecond = ulInterval / 100;
	*ulMilliSecond = 10 * (ulInterval % 100);

	return (VS_SUCCESS);
}

/*
Function	:inFunc_WatchRunTime
Date&Time	:2017/3/17 上午 11:18
Describe	:用來看精度秒以下的RunTime
*/
int inFunc_WatchRunTime(void)
{
	char		szDebugMsg[100 + 1];
	unsigned long	ulSecond = 0;
	unsigned long 	ulMilliSecond = 0;

	inFunc_GetRunTimeGlobal(&ulSecond, &ulMilliSecond);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Run Time %lu.%03lu", ulSecond, ulMilliSecond);
		inDISP_LogPrintf(szDebugMsg);
	}

	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Run Time %lu.%03lu", ulSecond, ulMilliSecond);
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_SaveRecordTime
Date&Time       :2017/8/29 上午 9:42
Describe        :
*/
int inFunc_SaveRecordTime(int inIndex)
{
	unsigned long	ulSecond = 0;
	unsigned long 	ulMilliSecond = 0;

	inFunc_GetRunTimeGlobal(&ulSecond, &ulMilliSecond);
	ginRecordTime[0][inIndex] = ulSecond;
	ginRecordTime[1][inIndex] = ulMilliSecond;

	return (VS_SUCCESS);
}

/*
Function        :inFunc_WatchRecordTime
Date&Time       :2017/8/29 上午 9:42
Describe        :
*/
int inFunc_WatchRecordTime(int inIndex)
{
	char	szDebugMsg[100 + 1];

	memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
	sprintf(szDebugMsg, "%d Time %d.%03d", inIndex, ginRecordTime[0][inIndex], ginRecordTime[1][inIndex]);
	inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
	
	inDISP_LogPrintfWithFlag(szDebugMsg);
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_CalculateRunTime_Start
Date&Time	:2017/4/27 下午 1:18
Describe	:用來看精度秒以下的RunTime
*/
unsigned long ulFunc_CalculateRunTime_Start()
{	unsigned long	ulRunTime;

	ulRunTime = CTOS_TickGet();

	return (ulRunTime);
}

/*
Function	:inFunc_GetRunTime
Date&Time	:2017/4/27 下午 1:18
Describe	:用來看精度秒以下的RunTime
*/
int inFunc_GetRunTime(unsigned long ulRunTime, unsigned long *ulSecond, unsigned long *ulMilliSecond)
{
	unsigned long	ulRunTimeStart, ulRunTimeEnd, ulInterval;

	ulRunTimeStart = 0;
	ulRunTimeEnd = 0;
	ulInterval = 0;
	*ulMilliSecond = 0;
	*ulSecond = 0;


	ulRunTimeStart = ulRunTime;
	ulRunTimeEnd = CTOS_TickGet();

	ulInterval = ulRunTimeEnd - ulRunTimeStart;
	*ulSecond = ulInterval / 100;
	*ulMilliSecond = 10 * (ulInterval % 100);

	return (VS_SUCCESS);
}

/*
Function        :inFunc_How_Many_Not_Ascii
Date&Time       :2017/2/15 下午 4:56
Describe        :判斷字串中不是英數字符號的數目 小心0x00
*/
int inFunc_How_Many_Not_Ascii(char *szData)
{
	int	i = 0;
	int	inLen = 0;
	int	inNotAscii = 0;

	inLen = strlen(szData);

	for (i = 0; i < inLen; i++)
	{
		if (szData[i] > 127)
		{
			inNotAscii++;
		}
	}

	return inNotAscii;
}

/*
Function	:inFunc_ChooseLoadFileWay
Date&Time	:2016/3/24 下午 2:02
Describe        :
*/
int inFunc_ChooseLoadFileWay(void)
{
	/* 用USB(待開發) */

	/* 用SD卡 */
	inFunc_SDLoadFile();

	return (VS_SUCCESS);
}

/*
Function	:inFunc_SDLoadFile
Date&Time	:2016/3/24 下午 2:02
Describe        :用SD卡載資料的特性:一次可load多個檔案，可以在程式運行中再插入SD卡
 *		目前無法做錯誤判斷
*/
int inFunc_SDLoadFile(void)
{
	int		inRetVal;
	char		szFileName[15];	/* 檔案名稱最多15字*/
        DISPLAY_OBJECT  srDispObj;

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 15;
	srDispObj.inColor = _COLOR_RED_;

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 輸入檔案名稱 */
        inDISP_ChineseFont("輸入檔案名稱:", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

        inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (VS_ERROR);
	memset(szFileName, 0x00, sizeof(szFileName));
	memcpy(szFileName, &srDispObj.szOutput[0], srDispObj.inMaxLen);

	/* fs_data資料夾內移到pub(若資料夾內沒資料則開檔失敗回傳錯誤) */
	inRetVal = inFunc_Copy_Data(szFileName, _FS_DATA_PATH_, "", _SD_PATH_);

	if (ginDebug == VS_TRUE)
	{
		/* 用teraterm可用來看資料夾內狀態 */
		inFunc_ShellCommand_Popen("ls");
	}

	return (VS_SUCCESS);
}

/*
Function	:inFunc_Move
Date&Time	:2018/4/23 下午 4:26
Describe        :此function將主程式資料夾中的特定檔案存到
*/
int inFunc_Move_Data(char *szSrcFileName, char* szSource, char* szDesFileName, char* szDestination)
{
	int	inRetVal = VS_SUCCESS;
	char	szCommand[100 + 1] = {0};	/* Shell Command*/

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inDISP_LogPrintfWithFlag(  "  szSrcFileName[%s]", szSrcFileName);
	inDISP_LogPrintfWithFlag(  "  szDesFileName[%s]", szDesFileName);
	

	/* mv 來源檔(source) 目標檔(destination) (等同rename)*/
	/* mv source1 source2 source3 .... directory (複製到目標資料夾)*/
	/* 可用來變更檔名 */
	/* 預設會直接覆蓋 */
	/* 組命令 */
	memset(szCommand, 0x00, sizeof(szCommand));
	if (szSrcFileName != NULL)
	{
		memset(szCommand, 0x00, sizeof(szCommand));
		sprintf(szCommand, "mv ");
		if (szSource != NULL)
		{
			strcat(szCommand, szSource);
		}

		strcat(szCommand, szSrcFileName);

		/* 空格 */
		strcat(szCommand, " ");

		if (szDestination != NULL)
		{
			strcat(szCommand, szDestination);
		}

		if (szDesFileName != NULL)
		{
			strcat(szCommand, szDesFileName);
		}
	}
	/* 沒有來源檔 */
	else
	{
		inDISP_DispLogAndWriteFlie("  Move Data No Source *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	/* 執行命令 */
	inRetVal = inFunc_ShellCommand_Popen(szCommand);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Func Move Data *Error* RetVal[%s] Line[%d] ",inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  Func Move Data *Error* Cmd[%s]", szCommand);
		return (inRetVal);
	}
	
	inDISP_LogPrintfWithFlag("  Command Success [%s]", szCommand);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
Function	:inFunc_Make_Dir
Date&Time	:2018/4/23 下午 3:48
Describe        :建立資料夾
*/
int inFunc_Make_Dir(char *szDirName, char* szSource)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1] = {0};
	char	szCommand[100 + 1] = {0};	/* Shell Command*/

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inFunc_Make_Dir START!");

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "DirName: %s", szDirName);
		inDISP_LogPrintf(szDebugMsg);
	}

	/* mkdir 資料夾路徑 */
	/* 組命令 */
	memset(szCommand, 0x00, sizeof(szCommand));
	if (szDirName != NULL)
	{
		memset(szCommand, 0x00, sizeof(szCommand));
		sprintf(szCommand, "mkdir -p  ");
		if (szSource != NULL)
		{
			strcat(szCommand, szSource);
		}

		strcat(szCommand, szDirName);
	}
	/* 沒有來源檔 */
	else
	{
		return (VS_ERROR);
	}

	/* 執行命令 */
        inRetVal = inFunc_ShellCommand_Popen(szCommand);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Func Make Dir *Error* RetVal[%s] Line[%d] ",inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  Func Make Dir *Error* Cmd[%s]",szCommand);
		return (inRetVal);
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "DirName : %s", szDirName);
		inDISP_LogPrintf(szDebugMsg);

		inDISP_LogPrintf("inFunc_Make_Dir END!");
	}

	return (VS_SUCCESS);
}

/*
Function	:inFunc_Delete_Data
Date&Time	:2018/4/23 下午 3:48
Describe        :刪除檔案
*/
int inFunc_Delete_Data(char* szParameter, char *szFileName, char* szSource)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1] = {0};
	char	szCommand[100 + 1] = {0};	/* Shell Command*/

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inFunc_Delete_Data START!");

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme: %s", szFileName);
		inDISP_LogPrintf(szDebugMsg);
	}

	/* rm 資料夾路徑/檔名 */
	/* 組命令 */
	memset(szCommand, 0x00, sizeof(szCommand));
	if (szFileName != NULL)
	{
		sprintf(szCommand, "rm  ");

		if (szParameter != NULL)
		{
			strcat(szCommand, szParameter);
			strcat(szCommand, " ");
		}

		if (szSource != NULL)
		{
			strcat(szCommand, szSource);
		}

		/* 目標檔名 */
		strcat(szCommand, szFileName);
	}
	else
	{
		return (VS_ERROR);
	}

	/* 執行命令 */
        inRetVal = inFunc_ShellCommand_Popen(szCommand);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Func Del *Error* RetVal[%s] Line[%d] ",inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  Func Del *Error* Cmd[%s]", szCommand);
		return (inRetVal);
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme : %s", szFileName);
		inDISP_LogPrintf(szDebugMsg);

		inDISP_LogPrintf("inFunc_Delete_Data END!");
	}

	return (VS_SUCCESS);
}

/*
Function	:inFunc_Copy_Data
Date&Time	:2018/4/24 上午 10:30
Describe        :複製檔案
*/
int inFunc_Copy_Data(char *szSrcFileName, char* szSource, char* szDesFileName, char* szDestination)
{
	int	inRetVal = VS_SUCCESS;
	char	szCommand[100 + 1] = {0};	/* Shell Command*/

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inDISP_LogPrintfWithFlag("  Source FNaeme[%s]", szSrcFileName);

	/* 組命令 */
	if (szSrcFileName != NULL)
	{
		/* cp szSource/szSrcFileName szDestination/szDesFileName */
		memset(szCommand, 0x00, sizeof(szCommand));
		sprintf(szCommand, "cp ");
		if (szSource != NULL)
		{
			strcat(szCommand, szSource);
		}

		strcat(szCommand, szSrcFileName);

		/* 空格 */
		strcat(szCommand, " ");

		if (szDestination != NULL)
		{
			strcat(szCommand, szDestination);
		}

		if (szDesFileName != NULL)
		{
			strcat(szCommand, szDesFileName);
		}
	}
	/* 沒有來源檔 */
	else
	{
		inDISP_LogPrintfWithFlag("  No szSrcFileName ");
		return (VS_ERROR);
	}

	/* 執行命令 */
	inRetVal = inFunc_ShellCommand_Popen(szCommand);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Func Copy *Error* RetVal[%s] Line[%d] ",inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  Func Copy *Error* Cmd[%s]", szCommand);
		return (inRetVal);
	}

	inDISP_LogPrintfWithFlag("  Command Success [%s]", szCommand);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function	:inFunc_Rename
Date&Time	:2018/5/25 下午 4:02
Describe        :更改檔名
*/
int inFunc_Rename_Data(char *szOldFileName, char* szSource, char *szNewFileName, char* szDestination)
{
	int	inRetVal = VS_SUCCESS;
	char	szCommand[100 + 1] = {0};	/* Shell Command*/

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inDISP_LogPrintfWithFlag(  "  szOldFileName[%s]", szOldFileName);
	inDISP_LogPrintfWithFlag(  "  szNewFileName[%s]", szNewFileName);

	/* 組命令 */
	if (szOldFileName != NULL)
	{
		/* mv szSource/szOldFileName szDestination/szNewFileName */
		memset(szCommand, 0x00, sizeof(szCommand));
		sprintf(szCommand, "mv ");
		if (szSource != NULL)
		{
			strcat(szCommand, szSource);
		}

		strcat(szCommand, szOldFileName);

		/* 空格 */
		strcat(szCommand, " ");

		if (szDestination != NULL)
		{
			strcat(szCommand, szDestination);
		}

		if (szNewFileName != NULL)
		{
			strcat(szCommand, szNewFileName);
		}
	}
	/* 沒有來源檔 */
	else
	{
		inDISP_LogPrintfWithFlag("  No szOldFileName");
		return (VS_ERROR);
	}

	/* 執行命令 */
	inRetVal = inFunc_ShellCommand_Popen(szCommand);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Func ReName *Error* RetVal[%s] Line[%d] ",inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  Func ReName *Error* Cmd[%s]", szCommand);
		return (inRetVal);
	}

	inDISP_LogPrintfWithFlag("  Command Success [%s]", szCommand);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_GZip_Data
Date&Time	:2018/6/19 下午 2:13
Describe        :GZip檔案
*/
int inFunc_GZip_Data(char* szParameter, char *szFileName, char* szSource)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1] = {0};
	char	szCommand[100 + 1] = {0};	/* Shell Command*/

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inFunc_GZip_Data START!");

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme: %s", szFileName);
		inDISP_LogPrintf(szDebugMsg);
	}

	/* rm 資料夾路徑/檔名 */
	/* 組命令 */
	memset(szCommand, 0x00, sizeof(szCommand));
	if (szFileName != NULL)
	{
		sprintf(szCommand, "gzip  ");

		if (szParameter != NULL)
		{
			strcat(szCommand, szParameter);
			strcat(szCommand, " ");
		}

		if (szSource != NULL)
		{
			strcat(szCommand, szSource);
		}

		/* 目標檔名 */
		strcat(szCommand, szFileName);
	}
	else
	{
		return (VS_ERROR);
	}

	/* 執行命令 */
        inRetVal = inFunc_ShellCommand_Popen(szCommand);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Func GZip Data *Error* RetVal[%s] Line[%d] ",inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  Func GZip Data *Error* Cmd[%s]", szCommand);
		return (inRetVal);
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme : %s", szFileName);
		inDISP_LogPrintf(szDebugMsg);

		inDISP_LogPrintf("inFunc_GZip_Data END!");
	}

	return (VS_SUCCESS);
}

/*
Function	:inFunc_GUnZip_Data
Date&Time	:2018/6/19 下午 2:29
Describe        :GUnZip檔案
*/
int inFunc_GUnZip_Data(char* szParameter, char *szFileName, char* szSource)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1] = {0};
	char	szCommand[100 + 1] = {0};	/* Shell Command*/

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inFunc_GUnZip_Data START!");

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme: %s", szFileName);
		inDISP_LogPrintf(szDebugMsg);
	}

	/* rm 資料夾路徑/檔名 */
	/* 組命令 */
	memset(szCommand, 0x00, sizeof(szCommand));
	if (szFileName != NULL)
	{
		sprintf(szCommand, "gunzip  ");

		if (szParameter != NULL)
		{
			strcat(szCommand, szParameter);
			strcat(szCommand, " ");
		}

		if (szSource != NULL)
		{
			strcat(szCommand, szSource);
		}

		/* 目標檔名 */
		strcat(szCommand, szFileName);
	}
	else
	{
		return (VS_ERROR);
	}

	/* 執行命令 */
        inRetVal = inFunc_ShellCommand_Popen(szCommand);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Func GUnZip Data *Error* RetVal[%s] Line[%d] ",inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  Func GUnZip Data *Error* Cmd[%s]", szCommand);
		return (inRetVal);
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme : %s", szFileName);
		inDISP_LogPrintf(szDebugMsg);

		inDISP_LogPrintf("inFunc_GUnZip_Data END!");
	}

	return (VS_SUCCESS);
}

/*
Function	:inFunc_Unzip
Date&Time	:2018/6/19 下午 2:36
Describe        :解壓縮
 	-c：将解压缩的结果显示到屏幕上，并对字符做适当的转换；
	-f：更新现有的文件；
	-l：显示压缩文件内所包含的文件；
	-p：与-c参数类似，会将解压缩的结果显示到屏幕上，但不会执行任何的转换；
	-t：检查压缩文件是否正确；
	-u：与-f参数类似，但是除了更新现有的文件外，也会将压缩文件中的其他文件解压缩到目录中；
	-v：执行时显示详细的信息；
	-z：仅显示压缩文件的备注文字；
	-a：对文本文件进行必要的字符转换；
	-b：不要对文本文件进行字符转换；
	-C：压缩文件中的文件名称区分大小写；
	-j：不处理压缩文件中原有的目录路径；
	-L：将压缩文件中的全部文件名改为小写；
	-M：将输出结果送到more程序处理；
	-n：解压缩时不要覆盖原有的文件；
	-o：不必先询问用户，unzip执行后覆盖原有的文件；
	-P<密码>：使用zip的密码选项；
	-q：执行时不显示任何信息；
	-s：将文件名中的空白字符转换为底线字符；
	-V：保留VMS的文件版本信息；
	-X：解压缩时同时回存文件原来的UID/GID；
	-d<目录>：指定文件解压缩后所要存储的目录；
	-x<文件>：指定不要处理.zip压缩文件中的哪些文件；
	-Z：unzip-Z等于执行zipinfo指令。

	将压缩文件text.zip在当前目录下解压缩。

	unzip test.zip
	将压缩文件text.zip在指定目录/tmp下解压缩，如果已有相同的文件存在，要求unzip命令不覆盖原先的文件。
	************重要*******************
	這裡指的當前目錄為主程式根目錄，非壓縮檔所在目錄

	unzip -n test.zip -d /tmp
	查看压缩文件目录，但不解压。

	unzip -v test.zip
	将压缩文件test.zip在指定目录/tmp下解压缩，如果已有相同的文件存在，要求unzip命令覆盖原先的文件。

	unzip -o test.zip -d tmp/
 */
int inFunc_Unzip(char* szParameter1, char* szOldFileName, char* szSource, char* szParameter2, char* szDestination)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1] = {0};
	char	szCommand[100 + 1] = {0};	/* Shell Command*/

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inFunc_Unzip START!");

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme: %s", szOldFileName);
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 組命令 */
	if (szOldFileName != NULL)
	{
		/* mv szSource/szOldFileName szDestination/szNewFileName */
		memset(szCommand, 0x00, sizeof(szCommand));
		sprintf(szCommand, "unzip ");

		if (szParameter1 != NULL)
		{
			strcat(szCommand, szParameter1);
			strcat(szCommand, " ");
		}

		if (szSource != NULL)
		{
			strcat(szCommand, szSource);
		}

		strcat(szCommand, szOldFileName);

		/* 空格 */
		strcat(szCommand, " ");

		if (szParameter2 != NULL)
		{
			strcat(szCommand, szParameter2);
			strcat(szCommand, " ");
		}

		if (szDestination != NULL)
		{
			strcat(szCommand, szDestination);
		}

	}
	/* 沒有來源檔 */
	else
	{
		return (VS_ERROR);
	}

	/* 執行命令 */
        inRetVal = inFunc_ShellCommand_Popen(szCommand);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Func UnZip Data *Error* RetVal[%s] Line[%d] ",inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  Func UnZip Data *Error* Cmd[%s]", szCommand);
		return (inRetVal);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inFunc_Unzip END!");
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_ls
Date&Time       :2018/6/29 上午 9:43
Describe        :
*/
int inFunc_ls(char* szParameter1, char* szSource)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1] = {0};
	char	szCommand[100 + 1] = {0};	/* Shell Command*/

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inFunc_ls START!");

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Path: %s", szSource);
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 組命令 */
	if (szSource != NULL)
	{
		memset(szCommand, 0x00, sizeof(szCommand));
		sprintf(szCommand, "ls ");

		if (szParameter1 != NULL)
		{
			strcat(szCommand, szParameter1);
			strcat(szCommand, " ");
		}

		if (szSource != NULL)
		{
			strcat(szCommand, szSource);
		}
	}
	/* 沒有來源檔 */
	else
	{
		return (VS_ERROR);
	}

	/* 執行命令 */
        inRetVal = inFunc_ShellCommand_System(szCommand);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Func Ls *Error* RetVal[%s] Line[%d] ",inRetVal, __LINE__);
		inDISP_DispLogAndWriteFlie("  Func Ls *Error* Cmd[%s]",szCommand);
		return (inRetVal);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inFunc_ls END!");
	}

	return (VS_SUCCESS);
}



/*
Function        :inFunc_CheckFile_In_SD
Date&Time       :2017/1/9 下午 3:01
Describe        :
*/
int inFunc_CheckFile_In_SD()
{
	char		szPath[100 + 1] = {0};
	char		szPath2[100 + 1] = {0};
	char		szPath3[100 + 1] = {0};
	char		szPath4[100 + 1] = {0};
	char		szPath5[100 + 1] = {0};
	char		szSrcPath2[100 + 1] = {0};
	char		szSrcPath3[100 + 1] = {0};
	char		szSrcPath4[100 + 1] = {0};
	char		szSrcPath5[100 + 1] = {0};
	char		szDirName[100 + 1] = {0};
	char		szDirName2[100 + 1] = {0};
	char		szDirName3[100 + 1] = {0};
	char		szDirName4[100 + 1] = {0};
	char		szDirName5[100 + 1] = {0};
	RTC_NEXSYS	srRTC = {0};
	
	if (inFunc_Check_SDCard_Mounted() != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	inFunc_GetSystemDateAndTime(&srRTC);
	sprintf(szPath, "%sV3Check/", _SD_PATH_);
	sprintf(szDirName, "%02d%02d%02d_%02d%02d%02d_NexsysV3", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
	sprintf(szDirName2, "%s/%s", szDirName, _FS_DIR_NAME_);
	sprintf(szDirName3, "%s/%s", szDirName, "ICERData");
	sprintf(szDirName4, "%s/%s/%s", szDirName, _FS_DIR_NAME_, _CA_DIR_NAME_);
	sprintf(szDirName5, "%s/%s/%s", szDirName, _FS_DIR_NAME_, _EMV_EMVCL_DIR_NAME_);
	
	inFunc_Make_Dir(szDirName, szPath);
	inFunc_Make_Dir(szDirName2, szPath);
	inFunc_Make_Dir(szDirName3, szPath);
	inFunc_Make_Dir(szDirName4, szPath);
	inFunc_Make_Dir(szDirName5, szPath);

	memset(szSrcPath2, 0x00, sizeof(szPath2));
	memset(szSrcPath3, 0x00, sizeof(szPath3));
	memset(szSrcPath4, 0x00, sizeof(szPath4));
	memset(szSrcPath5, 0x00, sizeof(szPath5));
	
	sprintf(szSrcPath2, "%s/%s/", _AP_ROOT_DIR_NAME_, _FS_DIR_NAME_);
	sprintf(szSrcPath3, "-r %s/%s/", _AP_ROOT_DIR_NAME_, "ICERData");
	sprintf(szSrcPath4, "%s/%s/%s/", _AP_ROOT_DIR_NAME_, _FS_DIR_NAME_, _CA_DIR_NAME_);
	sprintf(szSrcPath5, "%s/%s/%s/", _AP_ROOT_DIR_NAME_, _FS_DIR_NAME_, _EMV_EMVCL_DIR_NAME_);
	
	memset(szPath, 0x00, sizeof(szPath));
	memset(szPath2, 0x00, sizeof(szPath2));
	memset(szPath3, 0x00, sizeof(szPath3));
	memset(szPath4, 0x00, sizeof(szPath4));
	memset(szPath5, 0x00, sizeof(szPath5));
	
	sprintf(szPath, "%sV3Check/%02d%02d%02d_%02d%02d%02d_NexsysV3/", _SD_PATH_, srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
	sprintf(szPath2, "%s%s/", szPath, _FS_DIR_NAME_);
	sprintf(szPath3, "%s%s/", szPath, "ICERData");
	sprintf(szPath4, "%s%s/%s/", szPath, _FS_DIR_NAME_, _CA_DIR_NAME_);
	sprintf(szPath5, "%s%s/%s/", szPath, _FS_DIR_NAME_, _EMV_EMVCL_DIR_NAME_);
	inFunc_Copy_Data("*", _AP_ROOT_PATH_, "", szPath);
	inFunc_Copy_Data("*", szSrcPath2, "", szPath2);
	inFunc_Copy_Data("*", szSrcPath3, "", szPath3);
	inFunc_Copy_Data("*", szSrcPath4, "", szPath4);
	inFunc_Copy_Data("*", szSrcPath5, "", szPath5);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckFile_In_SD_ALL
Date&Time       :2018/6/4 下午 1:58
Describe        :
*/
int inFunc_CheckFile_In_SD_ALL()
{
	char		szPath[100 + 1] = {0};
	char		szDirName[100 + 1] = {0};
	RTC_NEXSYS	srRTC = {0};

	if (inFunc_Check_SDCard_Mounted() != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	inFunc_GetSystemDateAndTime(&srRTC);
	sprintf(szPath, "%sV3Check/", _SD_PATH_);
	sprintf(szDirName, "%02d%02d%02d_%02d%02d%02d_NexsysV3", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);

	inFunc_Make_Dir(szDirName, szPath);

	memset(szPath, 0x00, sizeof(szPath));
	sprintf(szPath, "%sV3Check/%02d%02d%02d_%02d%02d%02d_NexsysV3/", _SD_PATH_, srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
	inFunc_Copy_Data("* -r", _AP_ROOT_PATH_, "", szPath);

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Copy_All_File_To_SD
Date&Time       :2018/2/7 上午 10:17
Describe        :把程式內所有檔案抓出來
*/
int inFunc_Copy_All_File_To_SD()
{
	inFunc_Copy_Data("*", "-r ./", "", _SD_PATH_);

	return (VS_SUCCESS);
}



/*
Function        :inFunc_CheckFile_In_SD
Date&Time       :2017/1/9 下午 3:01
Describe        :
*/
int inFunc_CheckFile_In_SD_Partial()
{
	char		szPath[100 + 1] = {0};
	char		szPath2[100 + 1] = {0};
	char		szPath3[100 + 1] = {0};
	char		szPath4[100 + 1] = {0};
	char		szPath5[100 + 1] = {0};
	char		szSrcPath2[100 + 1] = {0};
	char		szSrcPath3[100 + 1] = {0};
	char		szSrcPath4[100 + 1] = {0};
	char		szSrcPath5[100 + 1] = {0};
	char		szDirName[100 + 1] = {0};
	char		szDirName2[100 + 1] = {0};
	char		szDirName3[100 + 1] = {0};
	char		szDirName4[100 + 1] = {0};
	char		szDirName5[100 + 1] = {0};
	char		szModelName[20 + 1] = {0};
	RTC_NEXSYS	srRTC = {0};
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	if (inFunc_Check_SDCard_Mounted() != VS_SUCCESS)
	{
		inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_1_, _NO_KEY_MSG_, 2,  "未掛載SD", _LINE_8_7_);
		return (VS_ERROR);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		inDISP_ChineseFont("SD已掛載", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
	}
	
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont("複製檔案中", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
	memset(szModelName, 0x00, sizeof(szModelName));
	inFunc_Get_Termial_Model_Name(szModelName);

	inFunc_GetSystemDateAndTime(&srRTC);
	sprintf(szPath, "%s%sCheck/", _SD_PATH_, szModelName);
	sprintf(szDirName, "%02d%02d%02d_%02d%02d%02d_Nexsys%s", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond, szModelName);
	sprintf(szDirName2, "%s/%s", szDirName, _FS_DIR_NAME_);
	sprintf(szDirName3, "%s/%s", szDirName, "ICERData");
	sprintf(szDirName4, "%s/%s/%s", szDirName, _FS_DIR_NAME_, _CA_DIR_NAME_);
	sprintf(szDirName5, "%s/%s/%s", szDirName, _FS_DIR_NAME_, _EMV_EMVCL_DIR_NAME_);
	
	inFunc_Make_Dir(szDirName, szPath);
	inFunc_Make_Dir(szDirName2, szPath);
	inFunc_Make_Dir(szDirName3, szPath);
	inFunc_Make_Dir(szDirName4, szPath);
	inFunc_Make_Dir(szDirName5, szPath);

	memset(szSrcPath2, 0x00, sizeof(szPath2));
	memset(szSrcPath3, 0x00, sizeof(szPath3));
	memset(szSrcPath4, 0x00, sizeof(szPath4));
	memset(szSrcPath5, 0x00, sizeof(szPath5));
	
	sprintf(szSrcPath2, "%s/%s/", _AP_ROOT_DIR_NAME_, _FS_DIR_NAME_);
	sprintf(szSrcPath3, "-r %s/%s/", _AP_ROOT_DIR_NAME_, "ICERData");
	sprintf(szSrcPath4, "%s/%s/%s/", _AP_ROOT_DIR_NAME_, _FS_DIR_NAME_, _CA_DIR_NAME_);
	sprintf(szSrcPath5, "%s/%s/%s/", _AP_ROOT_DIR_NAME_, _FS_DIR_NAME_, _EMV_EMVCL_DIR_NAME_);
	
	memset(szPath, 0x00, sizeof(szPath));
	memset(szPath2, 0x00, sizeof(szPath2));
	memset(szPath3, 0x00, sizeof(szPath3));
	memset(szPath4, 0x00, sizeof(szPath4));
	memset(szPath5, 0x00, sizeof(szPath5));
	
	sprintf(szPath, "%s%sCheck/%02d%02d%02d_%02d%02d%02d_Nexsys%s/", _SD_PATH_, szModelName, srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond, szModelName);
	sprintf(szPath2, "%s%s/", szPath, _FS_DIR_NAME_);
	sprintf(szPath3, "%s%s/", szPath, "ICERData");
	sprintf(szPath4, "%s%s/%s/", szPath, _FS_DIR_NAME_, _CA_DIR_NAME_);
	sprintf(szPath5, "%s%s/%s/", szPath, _FS_DIR_NAME_, _EMV_EMVCL_DIR_NAME_);
	inFunc_Copy_Data("*", _AP_ROOT_PATH_, "", szPath);
	inFunc_Copy_Data("*", szSrcPath2, "", szPath2);
	inFunc_Copy_Data("*", szSrcPath3, "", szPath3);
	inFunc_Copy_Data("*", szSrcPath4, "", szPath4);
	inFunc_Copy_Data("*", szSrcPath5, "", szPath5);
	
	sync();
	
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont("複製檔案完成", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckFile_In_USB_Partial
Date&Time       :2019/2/18 上午 10:35
Describe        :
*/
int inFunc_CheckFile_In_USB_Partial(void)
{
	int		inOrgUSBMode = 0;
	int		inRetVal = VS_ERROR;
	int		inTimeout = 30;		/* 大約8秒即掛載 */
	char		szPath[100 + 1] = {0};
	char		szPath2[100 + 1] = {0};
	char		szPath3[100 + 1] = {0};
	char		szPath4[100 + 1] = {0};
	char		szPath5[100 + 1] = {0};
	char		szSrcPath2[100 + 1] = {0};
	char		szSrcPath3[100 + 1] = {0};
	char		szSrcPath4[100 + 1] = {0};
	char		szSrcPath5[100 + 1] = {0};
	char		szDirName[100 + 1] = {0};
	char		szDirName2[100 + 1] = {0};
	char		szDirName3[100 + 1] = {0};
	char		szDirName4[100 + 1] = {0};
	char		szDirName5[100 + 1] = {0};
	char		szModelName[20 + 1] = {0};
	unsigned char	uszKey = 0x00;
	unsigned char	uszCheckUSBBit = VS_FALSE;
	unsigned char	uszCheckTimeoutBit = VS_FALSE;
	unsigned char	uszCheckKeyBit = VS_FALSE;
	RTC_NEXSYS	srRTC = {0};

	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("等待掛載中 Timeout:", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
	
	inUSB_Get_Host_Device_Mode(&inOrgUSBMode);
	inUSB_SelectMode(_USB_MODE_HOST_);
	inDISP_TimeoutStart(inTimeout);
	
	while (1)
	{
		inRetVal = inDISP_TimeoutCheck(_FONTSIZE_8X22_, _LINE_8_7_, _DISP_RIGHT_);
		if (inRetVal == VS_TIMEOUT)
		{
			uszCheckTimeoutBit = VS_TRUE;
		}
		
		uszKey = uszKBD_Key();
		if (uszKey == _KEY_CANCEL_)
		{
			uszCheckKeyBit = VS_TRUE;
		}
		
		inRetVal = inFunc_Check_USB_Mounted();
		if (inRetVal == VS_SUCCESS)
		{
			uszCheckUSBBit = VS_TRUE;
		}
		
		if (uszCheckTimeoutBit == VS_TRUE	|| 
		    uszCheckUSBBit == VS_TRUE		||
		    uszCheckKeyBit == VS_TRUE)
		{
			break;
		}
	}
	
	if (uszCheckUSBBit != VS_TRUE)
	{
		inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_1_, _NO_KEY_MSG_, 2,  "未掛載USB", _LINE_8_7_);
		inUSB_SelectMode(inOrgUSBMode);
		return (VS_ERROR);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		inDISP_ChineseFont("USB已掛載", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
	}
	
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont("複製檔案中，請稍後", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
	memset(szModelName, 0x00, sizeof(szModelName));
	inFunc_Get_Termial_Model_Name(szModelName);

	inFunc_GetSystemDateAndTime(&srRTC);
	sprintf(szPath, "%s%sCheck/", _USB_PATH_, szModelName);
	sprintf(szDirName, "%02d%02d%02d_%02d%02d%02d_Nexsys%s", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond, szModelName);
	sprintf(szDirName2, "%s/%s", szDirName, _FS_DIR_NAME_);
	sprintf(szDirName3, "%s/%s", szDirName, "ICERData");
	sprintf(szDirName4, "%s/%s/%s", szDirName, _FS_DIR_NAME_, _CA_DIR_NAME_);
	sprintf(szDirName5, "%s/%s/%s", szDirName, _FS_DIR_NAME_, _EMV_EMVCL_DIR_NAME_);
	
	inFunc_Make_Dir(szDirName, szPath);
	inFunc_Make_Dir(szDirName2, szPath);
	inFunc_Make_Dir(szDirName3, szPath);
	inFunc_Make_Dir(szDirName4, szPath);
	inFunc_Make_Dir(szDirName5, szPath);

	memset(szSrcPath2, 0x00, sizeof(szPath2));
	memset(szSrcPath3, 0x00, sizeof(szPath3));
	memset(szSrcPath4, 0x00, sizeof(szPath4));
	memset(szSrcPath5, 0x00, sizeof(szPath5));
	
	sprintf(szSrcPath2, "%s/%s/", _AP_ROOT_DIR_NAME_, _FS_DIR_NAME_);
	sprintf(szSrcPath3, "-r %s/%s/", _AP_ROOT_DIR_NAME_, "ICERData");
	sprintf(szSrcPath4, "%s/%s/%s/", _AP_ROOT_DIR_NAME_, _FS_DIR_NAME_, _CA_DIR_NAME_);
	sprintf(szSrcPath5, "%s/%s/%s/", _AP_ROOT_DIR_NAME_, _FS_DIR_NAME_, _EMV_EMVCL_DIR_NAME_);
	
	memset(szPath, 0x00, sizeof(szPath));
	memset(szPath2, 0x00, sizeof(szPath2));
	memset(szPath3, 0x00, sizeof(szPath3));
	memset(szPath4, 0x00, sizeof(szPath4));
	memset(szPath5, 0x00, sizeof(szPath5));
	
	sprintf(szPath, "%s%sCheck/%02d%02d%02d_%02d%02d%02d_Nexsys%s/", _USB_PATH_, szModelName, srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond, szModelName);
	sprintf(szPath2, "%s%s/", szPath, _FS_DIR_NAME_);
	sprintf(szPath3, "%s%s/", szPath, "ICERData");
	sprintf(szPath4, "%s%s/%s/", szPath, _FS_DIR_NAME_, _CA_DIR_NAME_);
	sprintf(szPath5, "%s%s/%s/", szPath, _FS_DIR_NAME_, _EMV_EMVCL_DIR_NAME_);
	inFunc_Copy_Data("*", _AP_ROOT_PATH_, "", szPath);
	inFunc_Copy_Data("*", szSrcPath2, "", szPath2);
	inFunc_Copy_Data("*", szSrcPath3, "", szPath3);
	inFunc_Copy_Data("*", szSrcPath4, "", szPath4);
	inFunc_Copy_Data("*", szSrcPath5, "", szPath5);
	
	sync();
	
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont("複製檔案完成", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
	inUSB_SelectMode(inOrgUSBMode);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckFile_In_USB_ALL
Date&Time       :2019/2/18 上午 10:35
Describe        :
*/
int inFunc_CheckFile_In_USB_ALL(void)
{
	int		inOrgUSBMode = 0;
	int		inRetVal = VS_ERROR;
	int		inTimeout = 30;
	char		szPath[100 + 1] = {0};
	char		szDirName[100 + 1] = {0};
	char		szModelName[20 + 1] = {0};
	unsigned char	uszKey = 0x00;
	unsigned char	uszCheckUSBBit = VS_FALSE;
	unsigned char	uszCheckTimeoutBit = VS_FALSE;
	unsigned char	uszCheckKeyBit = VS_FALSE;
	RTC_NEXSYS	srRTC = {0};
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("等待掛載中 Timeout:", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
	
	inUSB_Get_Host_Device_Mode(&inOrgUSBMode);
	inUSB_SelectMode(_USB_MODE_HOST_);
	inDISP_TimeoutStart(inTimeout);
	
	while (1)
	{
		inRetVal = inDISP_TimeoutCheck(_FONTSIZE_8X22_, _LINE_8_7_, _DISP_RIGHT_);
		if (inRetVal == VS_TIMEOUT)
		{
			uszCheckTimeoutBit = VS_TRUE;
		}
		
		uszKey = uszKBD_Key();
		if (uszKey == _KEY_CANCEL_)
		{
			uszCheckKeyBit = VS_TRUE;
		}
		
		inRetVal = inFunc_Check_USB_Mounted();
		if (inRetVal == VS_SUCCESS)
		{
			uszCheckUSBBit = VS_TRUE;
		}
		
		if (uszCheckTimeoutBit == VS_TRUE	|| 
		    uszCheckUSBBit == VS_TRUE		||
		    uszCheckKeyBit == VS_TRUE)
		{
			break;
		}
	}
	
	if (uszCheckUSBBit != VS_TRUE)
	{
		inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_1_, _NO_KEY_MSG_, 2,  "未掛載USB", _LINE_8_7_);		
		inUSB_SelectMode(inOrgUSBMode);
		
		return (VS_ERROR);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("USB已掛載", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
	}
	
	memset(szModelName, 0x00, sizeof(szModelName));
	inFunc_Get_Termial_Model_Name(szModelName);
	
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont("複製檔案中，請稍後", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
	inFunc_GetSystemDateAndTime(&srRTC);
	sprintf(szPath, "%s%sCheck/", _USB_PATH_, szModelName);
	sprintf(szDirName, "%04d%02d%02d_%02d%02d%02d_Nexsys%s", (srRTC.uszYear+2000), srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond, szModelName);
	
	inFunc_Make_Dir(szDirName, szPath);

	memset(szPath, 0x00, sizeof(szPath));
	//sprintf(szPath, "%s%sCheck/%02d%02d%02d_%02d%02d%02d_Nexsys%s/", _USB_PATH_, szModelName, srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond, szModelName);
	sprintf(szPath, "%s%sCheck/%s/", _USB_PATH_, szModelName, szDirName);
	inFunc_Copy_Data("* -r", _AP_ROOT_PATH_, "", szPath);
	
	sync();
	
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont("複製檔案完成", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
	inUSB_SelectMode(inOrgUSBMode);
	
	return (VS_SUCCESS);
}



/*
Function	:inFunc_ShellCommand_System
Date&Time	:2016/3/25 上午 10:45
Describe        :可輸入字串來執行shell指令，利用system函數，另一種方法使用popen，能比較有效的抓取錯誤訊息(待開發)
*/
int inFunc_ShellCommand_System(char *szCommand)
{
	int	inRetVal;
	char	szDebugMsg[100 + 1];

	if (szCommand == NULL)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("command 為空pointer");

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%s", szCommand);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}

	inRetVal = system(szCommand);

	/* 在C 程式裡其中 ret 值要除以256才會得到與shell 傳回相符的值 */
	inRetVal >>= 8;

	if (inRetVal != 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("Failed :");

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%s", szCommand);
			inDISP_LogPrintf(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Error Number : %d", errno);
			inDISP_LogPrintf(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "ReturnValue : %d", inRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}

		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("Success :");

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%s", szCommand);
			inDISP_LogPrintf(szDebugMsg);
		}
	}

	return (VS_SUCCESS);
}

/*
Function	:inFunc_ShellCommand_Popen
Date&Time	:2016/12/30 上午 9:24
Describe        :使用popen，能比較有效的抓取錯誤訊息
*/
int inFunc_ShellCommand_Popen(char *szCommand)
{
	char	szDebugMsg[1000 + 1];
	FILE	*pFilePointer;

	if (szCommand == NULL)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("command 為空pointer");

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%s", szCommand);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}

	pFilePointer = popen(szCommand, "r");

	if (pFilePointer == NULL)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("Failed :");

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%s", szCommand);
			inDISP_LogPrintf(szDebugMsg);
		}

		return (VS_ERROR);
	}
	else
	{
		while (fgets(szDebugMsg, sizeof(szDebugMsg), pFilePointer))
		{
			inDISP_LogPrintf(szDebugMsg);
		}

		pclose(pFilePointer);

		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("Success :");

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%s", szCommand);
			inDISP_LogPrintf(szDebugMsg);
		}
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_ShellCommand_TextFile_UTF8toBig5
Date&Time       :2017/4/27 下午 4:16
Describe        :
*/
int inFunc_ShellCommand_TextFile_UTF8toBig5(char *szUTF8FileName, char *szBIG5FileName)
{
	int	inReVal = VS_ERROR;
	char	szCommand[100 + 1];

	memset(szCommand, 0x00, sizeof(szCommand));
	/* example: iconv -f UTF-8 -t BIG-5 utf8.txt > big5.txt */
	sprintf(szCommand, "%s %s %s %s%s %s%s", "convmv", "-f UTF-8 -t BIG-5", "fs_data/", szUTF8FileName, ">", "fs_data/", szBIG5FileName);

	inReVal = inFunc_ShellCommand_Popen(szCommand);

	if (inReVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_ClearAP
Date&Time       :2017/5/8 下午 5:20
Describe        :
*/
int inFunc_ClearAP(char *szFileName)
{
	int	inReVal;
	char	szCommand[100 + 1];

	memset(szCommand, 0x00, sizeof(szCommand));
	sprintf(szCommand, "%s %s%s", "rm", "../pub/", szFileName);

	inReVal = inFunc_ShellCommand_Popen(szCommand);

	if (inReVal != (VS_SUCCESS))
	{
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        : inFunc_Clear_AP_Dump
Date&Time   : 2018/6/19 下午 1:56
Describe        : 這裡會刪除原本暫存的APP檔，TMS下載的APP會在FS目錄下，所以在安裝時的檔案會移動至此目錄
 *		      目錄位置是可以修改的
*/
int inFunc_Clear_AP_Dump(void)
{
	int	inReVal = VS_ERROR;

	inReVal = inFunc_Delete_Data("-r", "*", _APP_UPDATE_PATH_);
	if (inReVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function	:inFunc_DiscardSpace
Date&Time	:2016/3/29 下午 3:59
Describe        :去除host後面的空白
*/
int inFunc_DiscardSpace(char *szTemplate)
{
	int	i;
	char	szHost[8 + 1];

	memset(szHost, 0x00, sizeof(szHost));
	strcat(szHost, szTemplate);

	for (i = 7; i >= 0; i --)
	{
		if (szHost[i] != ' ')
			break;
	}

	/* Host全空白，不做改動 */
	if (i == 0)
	{
		return (VS_SUCCESS);
	}

	memset(szTemplate, 0x00, 8);
	memcpy(szTemplate, szHost, i + 1);

	return (VS_SUCCESS);
}

/*
Function	:inFunc_Align_Center
Date&Time	:2016/3/29 下午 3:59
Describe        :使未滿八個字的Host，前後補空白置中，Display用
*/
int inFunc_Align_Center(char *szTemplate)
{
	int	i, j;
	int	inSpaceNum = 0;
	int	inHalfSpaceNum = 0;
	char	szHost[8 + 1];

	memset(szHost, 0x00, sizeof(szHost));
	strcat(szHost, szTemplate);

	for (i = 7; i >= 0; i --)
	{
		if (szHost[i] != ' ')
			break;
	}

	/* Host全空白，不做改動 */
	if (i == 0)
	{
		return (VS_SUCCESS);
	}

	memset(szTemplate, 0x00, 8);
	if (i + 1 < 8)
	{
		inSpaceNum = 8 - (i + 1);
		inHalfSpaceNum = inSpaceNum / 2;
		/* 補前面空白 */
		for (j = 0; j < inHalfSpaceNum; j++)
		{
			strcat(szTemplate, "0");
		}

		/* 塞中間Host */
		strcat(szTemplate, szHost);

		/* 塞後面空白 */
		for (j = 0; j < inSpaceNum - inHalfSpaceNum; j++)
		{
			strcat(szTemplate, "0");
		}
	}
	else
	{
		memcpy(szTemplate, szHost, i + 1);
	}

	return (VS_SUCCESS);
}

/*
Function	:inFunc_CheckFullSpace
Date&Time	:2016/7/5 下午 5:07
Describe        :確認陣列中有值的地方是否全是0X20，確認電文中的值使用
*/
int inFunc_CheckFullSpace(char* szString)
{
	int	i;

	for (i = 0; i < strlen(szString); i++)
	{
		if (szString[i] != 0x20)
		{
			return (VS_FALSE);
		}
	}

	return	(VS_TRUE);
}

/*
Function	 : inFunc_HostName_DecideByTRT
Date&Time : 2016/4/14 下午 3:19
Describe	 : 用TRT決定 HostName
*/
int inFunc_HostName_DecideByTRT(char *szHostName)
{
	char	szTRTFName[16 + 1];
	char	szHostLabel[8 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 取出TRT FileName */
	memset(szTRTFName, 0x00, sizeof(szTRTFName));
	if (inGetTRTFileName(szTRTFName) == VS_ERROR)
		return (VS_ERROR);

	inDISP_LogPrintfWithFlag("  TrtFileName[%s]",szTRTFName);

	/* 組檔名，交易存檔檔案名稱 = (Host Name + Batch Number + .bat) */
	memset(szHostName, 0x00, sizeof(szHostName));
	if (!memcmp(szTRTFName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
	{
		memset(szHostLabel, 0x00, sizeof(szHostLabel));
		inGetHostLabel(szHostLabel);

		inDISP_LogPrintfWithFlag("  HostLabel Name[%s]",szHostLabel);
		
		/* 因為有15碼的限制，這邊需要依照HOST 組檔名，檔案名稱會另取 */
		if (memcmp(szHostLabel, _HOST_NAME_CREDIT_FUBON_, strlen(_HOST_NAME_CREDIT_FUBON_)) == 0)
		{	/*TODO: 看能不能寫成可變動功能 */
			sprintf(szHostName, "%s", _FILE_NAME_FUBON_);
		}
		else
		{
			inDISP_DispLogAndWriteFlie("  HostLabel *Error* TRTName[%s] Line[%d]", szHostLabel, __LINE__);
			inDISP_Msg_BMP("", 0, _BEEP_1TIMES_MSG_, 1, "檔名流程錯誤", _LINE_8_6_);
			return (VS_ERROR);
		}
	}
	else if (!memcmp(szTRTFName, _TRT_FILE_NAME_REDEMPTION_, strlen(_TRT_FILE_NAME_REDEMPTION_)))
		sprintf(szHostName, "%s", _FILE_NAME_REDEMPTION_);
	else if (!memcmp(szTRTFName, _TRT_FILE_NAME_INSTALLMENT_, strlen(_TRT_FILE_NAME_INSTALLMENT_)))
		sprintf(szHostName, "%s", _FILE_NAME_INSTALLMENT_);
	else if (!memcmp(szTRTFName, _TRT_FILE_NAME_AMEX_, strlen(_TRT_FILE_NAME_AMEX_)))
		sprintf(szHostName, "%s", _FILE_NAME_AMEX_);
	else if (!memcmp(szTRTFName, _TRT_FILE_NAME_DINERS_, strlen(_TRT_FILE_NAME_DINERS_)))
		sprintf(szHostName, "%s", _FILE_NAME_DINERS_);
	else if (!memcmp(szTRTFName, _TRT_FILE_NAME_UNION_PAY_, strlen(_TRT_FILE_NAME_UNION_PAY_)))
		sprintf(szHostName, "%s", _FILE_NAME_UNION_PAY_);
	else if (!memcmp(szTRTFName, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)))
		sprintf(szHostName, "%s", _FILE_NAME_DCC_);
	else if (!memcmp(szTRTFName, _TRT_FILE_NAME_ESC_, strlen(_TRT_FILE_NAME_ESC_)))
		sprintf(szHostName, "%s", _FILE_NAME_ESC_);
	else if (!memcmp(szTRTFName, _TRT_FILE_NAME_HG_, strlen(_TRT_FILE_NAME_HG_)))
		sprintf(szHostName, "%s", _FILE_NAME_HG_);
	else if (!memcmp(szTRTFName, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)))
		sprintf(szHostName, "%s", _FILE_NAME_ESVC_);
	else if (!memcmp(szTRTFName, _TRT_FILE_NAME_CMAS_, strlen(_TRT_FILE_NAME_CMAS_))) /* [新增電票悠遊卡功能] 新增組TRT File條件 [SAM] 2022/6/21 下午 4:38 */
		sprintf(szHostName, "%s", _FILE_NAME_CMAS_);
	else if (!memcmp(szTRTFName, _TRT_FILE_NAME_SVC_, strlen(_TRT_FILE_NAME_SVC_))) /* [新增SVC功能]  [SAM] */
		sprintf(szHostName, "%s", _FILE_NAME_SVC_);
	else
	{
		inDISP_DispLogAndWriteFlie("  Compose FileName *Error* TRTName[%s] Line[%d]", szTRTFName, __LINE__);
		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return VS_SUCCESS;
}

/*
Function	:inFunc_ComposeFileName
Date&Time	:2016/4/14 下午 1:13
Describe        :覺得很常用，而且方便之後維護這部份
 *  使用 HDT 及 HDPT 中的資料組合檔名
*/
int inFunc_ComposeFileName(TRANSACTION_OBJECT *pobTran, char *szFileName, char *szFileExtension, int inBatchNumWidth)
{
	char    szHostName[8 + 1];
	char	szSprintfArgument[10 + 1];
	char	szTemplate[16 + 1];
	long    lnBatchNum;                             /* 放HDPT中的Batch number */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (inGetBatchNum(szTemplate) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie("  Compose Get B_Num*Error* HDT[%d] Line[%d]",pobTran->srBRec.inHDTIndex, __LINE__);
		return (VS_ERROR);
	}

	lnBatchNum = atol(szTemplate);

	/* 若BatchNum小於1，則不合法 */
	if (lnBatchNum < 1)
	{		
		inDISP_DispLogAndWriteFlie("  Compose *Error* BatchNum[%d] Line[%d]",lnBatchNum, __LINE__);
		return (VS_ERROR);
	}

	/* "%s%06lu%s" */
	memset(szSprintfArgument, 0x00, sizeof(szSprintfArgument));
	sprintf(szSprintfArgument, "%s%d%s", "%s%0", inBatchNumWidth, "ld%s");

	/* 藉由TRT_FileName比對來組出amt的檔名 */
	memset(szHostName, 0x00, sizeof(szHostName));
	inFunc_HostName_DecideByTRT(szHostName);

	inDISP_LogPrintfWithFlag("  Compose Trt Name[%s]", szHostName);

	/* 組成FileName */
	sprintf(szFileName, szSprintfArgument, szHostName, lnBatchNum, szFileExtension);
	
	inDISP_LogPrintfWithFlag("  Compose Full Name[%s]", szFileName);
		
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function	:inFunc_ComposeFileName_ForGlobal
Date&Time	:2016/4/14 下午 1:13
Describe        :For 沒傳pobTran的
*/
int inFunc_ComposeFileName_ForGlobal(char *szFileName, char *szFileExtension, int inBatchNumWidth)
{
	char	szDebugMsg[84 + 1];
	char    szHostName[8 + 1];                     /* 跑那一個TRT流程 */
	char	szSprintfArgument[10 + 1];
	char	szTemplate[16 + 1];
	long    lnBatchNum;                             /* 放HDPT中的Batch number */

	if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inFunc_ComposeFileName_ForGlobal()_START");
        }

	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (inGetBatchNum(szTemplate) == VS_ERROR)
		return (VS_ERROR);

	lnBatchNum = atol(szTemplate);

	/* 若BatchNum小於1，則不合法 */
        if (lnBatchNum < 1)
        {
                if (ginDebug == VS_TRUE)
                {
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "illegal BatchNum.(batchNum = %ld)ERROR!", lnBatchNum);
                        inDISP_LogPrintf(szDebugMsg);
                }
                return (VS_ERROR);
        }

	/* "%s%06lu%s" */
	memset(szSprintfArgument, 0x00, sizeof(szSprintfArgument));
	sprintf(szSprintfArgument, "%s%d%s", "%s%0", inBatchNumWidth, "lu%s");

	/* 藉由TRT_FileName比對來組出amt的檔名 */
	memset(szHostName, 0x00, sizeof(szHostName));
	inFunc_HostName_DecideByTRT(szHostName);

	/* 組成FileName */
        sprintf(szFileName, szSprintfArgument, szHostName, lnBatchNum, szFileExtension);

	if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("inFunc_ComposeFileName_ForGlobal()_END");
		inDISP_LogPrintf("----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function	    : inFunc_ComposeFileName_InvoiceNumber
Date&Time   : 2016/5/4 上午 10:38
Describe        : 考慮到同一InvoiceNumber會有取消、調帳等操作，會有多個簽名圖檔
*/
int inFunc_ComposeFileName_InvoiceNumber(TRANSACTION_OBJECT *pobTran, char *szFileName, char *szFileExtension, int inInvNumWidth)
{
	char    szHostName[8 + 1];
	char	szSprintfArgument[18 + 1];
	long    lnInvNum; /* 放HDPT中的Batch number */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 將pobTran中的BatchNum取出 */
	lnInvNum = pobTran->srBRec.lnOrgInvNum;

	/* 若BatchNum小於1，則不合法 */
	if (lnInvNum < 1)
	{
		inDISP_DispLogAndWriteFlie("  illegal InvoiceNum.(InvoiceNum = %ld)ERROR!", lnInvNum);
		return (VS_ERROR);
	}

	/* "%s%06lu%s" */
	memset(szSprintfArgument, 0x00, sizeof(szSprintfArgument));
	switch (pobTran->srBRec.inCode)
	{
		case _VOID_:
			sprintf(szSprintfArgument, "%s%d%s%s%s", "%s%0", inInvNumWidth, "ld", "1", "%s");
			break;
		case _TIP_ :
			sprintf(szSprintfArgument, "%s%d%s%s%s", "%s%0", inInvNumWidth, "ld", "2", "%s");
			break;
		case _ADJUST_ :
			sprintf(szSprintfArgument, "%s%d%s%s%s", "%s%0", inInvNumWidth, "ld", "3", "%s");
			break;
		default :
			sprintf(szSprintfArgument, "%s%d%s%s%s", "%s%0", inInvNumWidth, "ld", "", "%s");
			break;
	}


	/* 藉由TRT_FileName比對來組出amt的檔名 */
	memset(szHostName, 0x00, sizeof(szHostName));
	inFunc_HostName_DecideByTRT(szHostName);

	inDISP_LogPrintfWithFlag( " TRT NAME [%s]", szHostName);

		/* printf遇到szSprintfArgument內的字串，會認錯，以為後面有引數 */
//		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
//		sprintf(szDebugMsg, "%s", szSprintfArgument);
//		inDISP_LogPrintf(szDebugMsg);

	/* 組成FileName */
	/* 檔名最長15，所以清15 */
	memset(szFileName, 0x00, 15);
	sprintf(szFileName, szSprintfArgument, szHostName, lnInvNum, szFileExtension);
	
	
	inDISP_LogPrintfWithFlag("  FullComposeFileName [%s]", szFileName);	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_REFERRAL_GetManualApproval
Date&Time	:2016/4/19 下午 8:08
Describe        :Call Bank使用 輸入授權碼
*/
int inFunc_REFERRAL_GetManualApproval(TRANSACTION_OBJECT *pobTran)
{
/* 因為UPT不應該有確認畫面  20190523  [SAM] */		
#if _MACHINE_TYPE_ != _CASTLE_TYPE_UPT1000_	
	int		inRetVal = 0;
	int		inChoice = 0;
	int		i = 0;
	char		szDispMsg[128 + 1];
	char		szTemplate[100];
	char		szLocalPhoneNum[30 + 1];
	char		szCommMode[2 + 1], szCallBankEnable[2 + 1];
	unsigned char   uszKey;
	unsigned char	uszCheckReferralNo = VS_TRUE;		/* 確認電話號碼是否合法 */
	unsigned char	uszRetryAgain = VS_FALSE;		/* 表示要重試 */
#endif
	
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inFunc_REFERRAL_GetManualApproval START!!");

//	vdEMVSetICCReadFailure(VS_FALSE);
//	vdEMVSetFallbackToMSR(VS_FALSE);

	/*
		交易在這裡結束，交易不論成功或失敗都在這裡斷線
		交易以前做法是要全部斷網路線，新的做法只要關【SOCKET】，【GPRS】同樣的做法
	*/
	inCOMM_End(pobTran);

	if (pobTran->uszECRBit == VS_TRUE)
	{
		inECR_SendError(pobTran, _ECR_RESPONSE_CODE_CALLBANK_);
	}

/* 因為UPT不應該有確認畫面  20190523  [SAM] */		
#if _MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_ERR_CALL_BANK_, 0, _COORDINATE_Y_LINE_8_5_);		/* 請聯絡銀行 */
	inDISP_BEEP(1, 0);
	inDISP_Wait(1000);
	/* 使用者終止交易 */
	pobTran->inErrorMsg = _ERROR_CODE_V3_USER_CANCEL_;
	return (VS_USER_CANCEL);
#else	
	
	/* 抓取 KioskFlag的值，如為 TRUE 就不用確認畫面 2020/3/6 下午 4:28 [SAM] */
	if(inFunc_GetKisokFlag() == VS_TRUE)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_ERR_CALL_BANK_, 0, _COORDINATE_Y_LINE_8_5_);		/* 請聯絡銀行 */
		inDISP_BEEP(1, 0);
		inDISP_Wait(1000);
		/* 使用者終止交易 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_USER_CANCEL_;
		inDISP_LogPrintfWithFlag("  Func Kiosk Referral ManualAppoval Line[%d]", __LINE__);
		return (VS_USER_CANCEL);
	}
	
	
	if (inLoadCPTRec(0) < 0)
	{
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "inLoadCPTRec(0)_ERR");
                        inDISP_LogPrintf(szTemplate);
                }

		return (VS_ERROR);
	}

	memset(szLocalPhoneNum, 0x00, sizeof(szLocalPhoneNum));
	/* 聯合版本只給區域號碼 */
	//inGetReferralTel(szLocalPhoneNum);pobTran->szReferralPhoneNum
	inGetReferralTel(pobTran->szReferralPhoneNum);

	/*
		含區號的電話號碼。(左靠右補空白)
		3碼區域號+12碼電話號碼
		例：
		台北〈0227191919〉
		端末機依TMS下載的第三支電話中區域碼判斷是否需要 將Referral Telephone no.前的區碼過濾掉
		Table ID”NA”(Call Bank Referral Telephone no.)下載的電話號碼中間不會 有空白或’-’
	*/
	if (!memcmp(&pobTran->szReferralPhoneNum[0], &szLocalPhoneNum[0], strlen(szLocalPhoneNum)))
	{
		/* 表示區碼相同 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, &pobTran->szReferralPhoneNum[strlen(szLocalPhoneNum)]);
		memset(pobTran->szReferralPhoneNum, 0x00, sizeof(pobTran->szReferralPhoneNum));
		strcpy(pobTran->szReferralPhoneNum, szTemplate);
	}



	/* CallBank Enable */
	memset(szCallBankEnable, 0x00, sizeof(szCallBankEnable));
        inGetCallBankEnable(szCallBankEnable);
	if (szCallBankEnable[0] != 'Y')
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_ERR_CALL_BANK_, 0, _COORDINATE_Y_LINE_8_5_);		/* 請聯絡銀行 */

		inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);			/* 請按確認鍵 */
		inDISP_BEEP(1, 0);
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(_Touch_O_LINE8_8_);
			uszKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				uszKey = _KEY_TIMEOUT_;
			}

			if (inChoice == _Touch_O_LINE8_8_ENTER_BUTTON_	||
			    uszKey == _KEY_ENTER_			||
			    uszKey == _KEY_TIMEOUT_)
			{
				break;
			}
			else
			{
				continue;
			}
		}

		/* 使用者終止交易 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_USER_CANCEL_;
		return (VS_USER_CANCEL);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_ERR_CALL_BANK_, 0, _COORDINATE_Y_LINE_8_5_);			/* 請聯絡銀行 */

		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "TEL : %s", pobTran->szReferralPhoneNum);
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _DISP_CENTER_);		/* 顯示電話號碼 */

		/* （十一）
		 * CUP 之 Preauth 交易，若主機回覆 Call bank 時，端末機請轉為 “拒絕交易” 訊息，
		 * CUP 之一般交易，若主機回覆 Call bank 時，端末機請提示完撥號訊息後，
		 * 繼續顯示請聯絡銀行，按【清除】回主畫面，且不可接續以補登流程完成交易。
		 */
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			inDISP_PutGraphic(_ERR_CLEAR_, 0, _COORDINATE_Y_LINE_8_7_); /* 請按清除鍵 */
			inDISP_BEEP(1, 0);
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

			while (1)
			{
				inChoice = inDisTouch_TouchSensor_Click_Slide(_Touch_X_LINE8_8_);
				uszKey = uszKBD_Key();

				/* Timeout */
				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					uszKey = _KEY_TIMEOUT_;
				}

				if (inChoice == _Touch_X_LINE8_8_CANCEL_BUTTON_	||
				    uszKey == _KEY_CANCEL_			||
				    uszKey == _KEY_TIMEOUT_)
				{
					break;
				}
				else
				{
					continue;
				}
			}

			/* 使用者終止交易 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_USER_CANCEL_;
			return (VS_USER_CANCEL);
		}
		else
		{
			/* 請按確認或清除鍵 */
			inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);
			inDISP_BEEP(3, 500);
			inFlushKBDBuffer();
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

			while (1)
			{
				inChoice = inDisTouch_TouchSensor_Click_Slide(_Touch_OX_LINE8_8_);
				uszKey = uszKBD_Key();

				/* Timeout */
				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					uszKey = _KEY_TIMEOUT_;
				}

				if (inChoice == _Touch_OX_LINE8_8_CANCEL_BUTTON_	||
				    uszKey == _KEY_CANCEL_				||
				    uszKey == _KEY_TIMEOUT_)
				{
					/* 使用者終止交易 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_USER_CANCEL_;
					return (VS_USER_CANCEL);
				}
				else if (inChoice == _Touch_OX_LINE8_8_ENTER_BUTTON_	||
					 uszKey == _KEY_ENTER_)
				{
					break;
				}
				else
				{
					continue;
				}
			}
		}
		/* 去除電話號碼中非數字的字元 */
		vdFunc_FilterTel(pobTran->szReferralPhoneNum);

		memset(szCommMode, 0x00, sizeof(szCommMode));
		inGetCommMode(szCommMode);

		if (memcmp(szCommMode, _COMM_GPRS_MODE_, 1) != 0)
		{
			/* 沒電話號碼 */
			if (strlen(pobTran->szReferralPhoneNum) == 0)
				uszCheckReferralNo = VS_FALSE;

			/* 檢查【Referral】號碼 */
			for (i = 0; i < strlen(pobTran->szReferralPhoneNum); i ++)
			{
				/* 如果號碼不是數字 */
				if ((pobTran->szReferralPhoneNum[i] >= 0x3A) || (pobTran->szReferralPhoneNum[i] <= 0x2F))
				{
					uszCheckReferralNo = VS_FALSE;
					break;
				}
			}

			if (uszCheckReferralNo == VS_TRUE)
			{
				/* 撥號 */
				inRetVal = inFunc_Dial_VoiceLine((unsigned char*)pobTran->szReferralPhoneNum, strlen(pobTran->szReferralPhoneNum));
				if (inRetVal == VS_USER_CANCEL)
				{
					/* 使用者終止交易 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_USER_CANCEL_;
					return (VS_USER_CANCEL);
				}
				else
				{
					/* 繼續執行*/
				}
			}
		}

		/* 顯示授權碼補登 */
		inDISP_ClearAll();
		inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_8_1_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_CALL_BANK_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 授權碼補登 */
		/* 輸入管理號碼 */
		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(_ACCESS_WITH_CUSTOM_, _CALL_BANK_) != VS_SUCCESS)
			return (VS_ERROR);

		/* 提示防詐騙訊息 */
		inRetVal = inFunc_Disclaim_Auth(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			/* 使用者終止交易 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_USER_CANCEL_;
			return (VS_USER_CANCEL);
		}

		if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
		{
			/* 偵測晶片插卡 */
			if (inEMV_ICCEvent() != VS_SUCCESS)
			{
				/* 晶片卡被取出 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;

				return (VS_ERROR);
			}

			inFunc_REFERRAL_DisplayPan(pobTran);
			inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_8_); /* 請按確認建 */
			inDISP_BEEP(1, 0);
			inFlushKBDBuffer();

			while (1)
			{
				uszKey = uszKBD_GetKey(30);

				if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
				{
					/* 使用者終止交易 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_USER_CANCEL_;
					return (VS_USER_CANCEL);
				}
				else if (uszKey == _KEY_ENTER_)
				{
					break;
				}
				else
				{
					continue;
				}
			}
		}

		while(1)
		{
			//���M��,���M�������|�n�D��J!!By Ray For FUBON!!
			memset(pobTran->srBRec.szAuthCode,0x00,sizeof(pobTran->srBRec.szAuthCode));
			/* 輸入授權碼 */
			if (inCREDIT_Func_GetAuthCode(pobTran) != VS_SUCCESS)
				return (VS_ERROR);

			/* 檢核授權碼字元是否合法 */
			for (i = 0; i < strlen(pobTran->srBRec.szAuthCode); i ++)
			{
				if (((pobTran->srBRec.szAuthCode[i] >= '0') && (pobTran->srBRec.szAuthCode[i] <= '9')) ||
				    ((pobTran->srBRec.szAuthCode[i] >= 'A') && (pobTran->srBRec.szAuthCode[i] <= 'Z')) ||
				    ((pobTran->srBRec.szAuthCode[i] >= 'a') && (pobTran->srBRec.szAuthCode[i] <= 'z')))
				{
					continue;
				}
				else
				{
					inDISP_BEEP(1, 0);
					uszRetryAgain = VS_TRUE;
					break;
				}
			}

			/* 2012-05-24 AM 11:38:23 add by kakab 修正call bank授權碼輸入時須檢查是否為全0，以免後續錯誤 */
			if (uszRetryAgain == VS_TRUE ||
			    (pobTran->srBRec.uszCUPTransBit != VS_TRUE && !memcmp(pobTran->srBRec.szAuthCode, "000000", 6)))
			{
				/* 2011-04-30 PM 04:47:08 有亂碼要有提示語  在回到輸入畫面 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("授權碼檢查錯誤", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
				inDISP_BEEP(2, 500);

				continue;
			}

			if (strlen(pobTran->srBRec.szAuthCode) >= 2)
			{
				/* 授權碼輸入六碼後 按消字鍵再按【ENTER】數字後有亂碼 */
				pobTran->srBRec.szAuthCode[6] = 0x00;
				break; /* 授權碼最少要 2 碼 */
			}
		} /* End while () .... */

		if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
		{
			/* 偵測晶片插卡 */
			if (inEMV_ICCEvent() != VS_SUCCESS)
			{
				/* 晶片卡被取出 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;

				return (VS_ERROR);
			}
		}
	}/* 授權碼補登開啟 END */
#endif
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inFunc_REFERRAL_GetManualApproval END!!");

	return (VS_SUCCESS);
}

/*
Function	:vdFunc_FilterTel
Date&Time	:2016/7/25 上午 11:44
Describe        :去除電話號碼中非數字之字元
*/
void vdFunc_FilterTel(char *tel)
{
    	int  i , j ;
    	char temp_tel[17] ;

    	memset ( temp_tel , 0x00 , 17 ) ;
    	j = 0 ;

    	for ( i = 0 ; i < 17 ; i ++ )
    	{
        	if ( ( *( tel + i ) >= '0' ) && (  *( tel + i ) <= '9' ) )
        	{
            		temp_tel[j] = *( tel + i ) ;
            		j ++ ;
        	}
    	}

    	for ( i = 0 ; i < 17 ; i ++ )
    	{
        	*( tel + i ) = temp_tel[i] ;
    	}
}

/*
Function	:inFunc_REFERRAL_DisplayPan
Date&Time	:2016/7/25 上午 11:44
Describe        :顯示卡號
*/
int inFunc_REFERRAL_DisplayPan(TRANSACTION_OBJECT *pobTran)
{
	char	szDispMsg[21 + 1];
	char	szExpDate[4 + 3];
	char	szFinalPAN[_PAN_UCARD_SIZE_ + 1];

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("請確認卡號及有效期", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
	/* Disp Card Number */
	if (strlen(pobTran->srBRec.szExpDate) > 0)
	{
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
		strcpy(szFinalPAN, pobTran->srBRec.szPAN);

		if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
		{
			inFunc_FormatPAN_UCARD(szFinalPAN);
		}
		sprintf(szDispMsg, " %s",szFinalPAN);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);
	}

	if (strlen(pobTran->srBRec.szExpDate) > 0)
	{
		memset(szExpDate, 0x00, sizeof(szExpDate));
		strcpy(szExpDate, pobTran->srBRec.szExpDate);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "MM/'YY = %.2s/'%.2s", szExpDate + 2, szExpDate);
                inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _DISP_CENTER_);
	}

	return (VS_SUCCESS);
}

/*
Function	:inFunc_DebugSwitch
Date&Time	:2016/4/22 下午 2:08
Describe        :功能鍵進入 Debug的開關設定
*/
int inFunc_DebugSwitch(void)
{
	int		inRetVal;
	char		szTemplate[64 + 1];
	DISPLAY_OBJECT  srDispObj;

	/* Debug */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	memset(szTemplate, 0x00, sizeof(szTemplate));

	if (ginDebug == VS_TRUE)
		strcpy(szTemplate, "Debug 開關 : ON");
	else
		strcpy(szTemplate, "Debug 開關 : OFF");

        inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
        inDISP_ChineseFont("1 = ON , 0 = OFF", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);

        while (1)
        {
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                srDispObj.inMaxLen = 1;
                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

                inRetVal = inDISP_Enter8x16(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                        return (VS_ERROR);

                if (strlen(srDispObj.szOutput) > 0)
                {
                        if (srDispObj.szOutput[0] == '0')
                        {
				/* 只設定第一個位置 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetISODebug(szTemplate);
				szTemplate[0] = '0';

				inSetISODebug(szTemplate);
				inSaveEDCRec(0);
				inFunc_Sync_Debug_Switch();

                                break;
                        }
                        else if (srDispObj.szOutput[0] == '1')
                        {
                                /* 只設定第一個位置 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetISODebug(szTemplate);
				szTemplate[0] = '1';

				inSetISODebug(szTemplate);
				inSaveEDCRec(0);
				inFunc_Sync_Debug_Switch();

                                break;
                        }
                        else
                        {
                                inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
                                continue;
                        }
                }
                else
                {
                        break;
                }
        }

        /* ISODebug */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        memset(szTemplate, 0x00, sizeof(szTemplate));

        if (ginISODebug == VS_TRUE)
                strcpy(szTemplate, "ISODebug: ON");
        else
                strcpy(szTemplate, "ISODebug: OFF");

        inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
        inDISP_ChineseFont("1 = ON , 0 = OFF", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);

        while (1)
        {
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                srDispObj.inMaxLen = 1;
                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

                inRetVal = inDISP_Enter8x16(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                        return (VS_ERROR);

                if (strlen(srDispObj.szOutput) > 0)
                {
                        if (srDispObj.szOutput[0] == '0')
                        {
				/* 只設定第二個位置 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetISODebug(szTemplate);
				szTemplate[1] = '0';

				inSetISODebug(szTemplate);
				inSaveEDCRec(0);
				inFunc_Sync_ISODebug_Switch();

                                break;
                        }
                        else if (srDispObj.szOutput[0] == '1')
                        {
				/* 只設定第二個位置 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetISODebug(szTemplate);
				szTemplate[1] = '1';

				inSetISODebug(szTemplate);
				inSaveEDCRec(0);
				inFunc_Sync_ISODebug_Switch();

                                break;
                        }
                        else
                        {
                                inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
                                continue;
                        }
                }
                else
                {
                        break;
                }
        }

	/* DisplayDebug */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        memset(szTemplate, 0x00, sizeof(szTemplate));

        if (ginDisplayDebug == VS_TRUE)
                strcpy(szTemplate, "DisplayDebug: ON");
        else
                strcpy(szTemplate, "DisplayDebug: OFF");

        inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
        inDISP_ChineseFont("1 = ON , 0 = OFF", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);

        while (1)
        {
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                srDispObj.inMaxLen = 1;
                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

                inRetVal = inDISP_Enter8x16(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                        return (VS_ERROR);

                if (strlen(srDispObj.szOutput) > 0)
                {
                        if (srDispObj.szOutput[0] == '0')
                        {
				/* 只設定第三個位置 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetISODebug(szTemplate);
				szTemplate[2] = '0';

				inSetISODebug(szTemplate);
				inSaveEDCRec(0);
				inFunc_Sync_DisplayDebug_Switch();

                                break;
                        }
                        else if (srDispObj.szOutput[0] == '1')
                        {
				/* 只設定第二個位置 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetISODebug(szTemplate);
				szTemplate[2] = '1';

				inSetISODebug(szTemplate);
				inSaveEDCRec(0);
				inFunc_Sync_DisplayDebug_Switch();

                                break;
                        }
                        else
                        {
                                inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
                                continue;
                        }
                }
                else
                {
                        break;
                }
        }

	/* EngineerDebug */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        memset(szTemplate, 0x00, sizeof(szTemplate));

        if (ginISODebug == VS_TRUE)
                strcpy(szTemplate, "EngineerDebug: ON");
        else
                strcpy(szTemplate, "EngineerDebug: OFF");

        inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
        inDISP_ChineseFont("1 = ON , 0 = OFF", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);

        while (1)
        {
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                srDispObj.inMaxLen = 1;
                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

                inRetVal = inDISP_Enter8x16(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                        return (VS_ERROR);

                if (strlen(srDispObj.szOutput) > 0)
                {
                        if (srDispObj.szOutput[0] == '0')
                        {
				/* 只設定第四個位置 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetISODebug(szTemplate);
				szTemplate[3] = '0';

				inSetISODebug(szTemplate);
				inSaveEDCRec(0);
				inFunc_Sync_EngineerDebug_Switch();

                                break;
                        }
                        else if (srDispObj.szOutput[0] == '1')
                        {
				/* 只設定第二個位置 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetISODebug(szTemplate);
				szTemplate[3] = '1';

				inSetISODebug(szTemplate);
				inSaveEDCRec(0);
				inFunc_Sync_EngineerDebug_Switch();

                                break;
                        }
                        else
                        {
                                inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
                                continue;
                        }
                }
                else
                {
                        break;
                }
        }

        return (VS_SUCCESS);
}

/*
Function	:inFunc_EDCLock
Date&Time	:2016/9/1 下午 4:36
Describe        :設定EDCLOCK為1，並重開機
*/
int inFunc_EDCLock(void)
{
	inSetEDCLOCK("Y");
	inSaveEDCRec(0);

	inDISP_LogPrintfWithFlag("！！鎖機！！");

	/* 抓資料 */
	inFunc_Copy_Data(_DATA_BASE_NAME_NEXSYS_, _AP_ROOT_PATH_, "", _SD_PATH_);

	inDISP_Msg_BMP(_ERR_EDC_LOCK_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

	inFunc_Reboot();

	return (VS_SUCCESS);
}

/*
Function		: inFunc_Check_EDCLock
Date&Time	: 2016/6/2 上午 11:54
Describe		: 確認是否鎖機
 * [整理功能並加註解]  [SAM] 
*/
int inFunc_Check_EDCLock(void)
{
	int	i;
	char	szEDCLock[2 + 1];
	char	szHostName[8 + 1];

	memset(szEDCLock, 0x00, sizeof(szEDCLock));
	inLoadEDCRec(0);
	inGetEDCLOCK(szEDCLock);
	if (szEDCLock[0] == 'Y')
	{
		for (i = 0 ;; i ++)
		{
			if (inLoadHDTRec(i) < 0)	/* 主機參數檔【HostDef.txt】 */
			{
				inDISP_LogPrintfWithFlag(" Func Check EDC Lock Load HDT[%d] *Error* Line[%d]", i, __LINE__);
				break;
			}

			memset(szHostName, 0x00, sizeof(szHostName));
			inGetHostLabel(szHostName);
			/* 因為公司TMS會下CUP主機，因富邦是同批次，所以CUP要跳過 所以新增 20190214 [SAM] */
			if ( memcmp(szHostName, _HOST_NAME_ESC_, strlen(_HOST_NAME_ESC_)) == 0 ||
			     memcmp(szHostName, _HOST_NAME_CUP_, strlen(_HOST_NAME_CUP_)) == 0)
			{
				continue;
			}

			inLoadHDPTRec(i);
			inSetMustSettleBit("Y");
			inSaveHDPTRec(i);
		}
		
		inEDCTMS_FUNC_SetTmsFtpSettleFlagOn();
		
		inDISP_Msg_BMP(_ERR_EDC_LOCK_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

		return (VS_ERROR);
	}
	else
	{
		return(VS_SUCCESS);
	}

}

/*
Function        :inFunc_Unlock_EDCLock_Flow
Date&Time       :2017/3/28 下午 2:07
Describe        :
*/
int inFunc_Unlock_EDCLock_Flow()
{
	char	szKey;

	while(1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont_Color("1.只解鎖EDC", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
		inDISP_ChineseFont_Color("2.解鎖EDC並清除全部批次", _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);

		szKey = uszKBD_GetKey(_EDC_TIMEOUT_);
		if (szKey == _KEY_1_)
		{
			inFunc_Unlock_EDCLock();
			break;
		}
		else if (szKey == _KEY_2_)
		{
			inFunc_Unlock_EDCLock_And_Delete_Batch();
			break;
		}
		else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
		{
			break;
		}

	}

	return (VS_SUCCESS);
}

/*
Function	:inFunc_Unlock_EDCLock
Date&Time	:2016/8/16 下午 3:14
Describe        :解鎖鎖機狀態
*/
int inFunc_Unlock_EDCLock(void)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey;

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("EDC UNLOCK？", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
	inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
	inDISP_BEEP(1, 0);

	while (1)
	{
		uszKey = uszKBD_GetKey(_EDC_TIMEOUT_);

		if (uszKey == _KEY_0_)
		{
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			return (VS_USER_CANCEL);
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			return (VS_TIMEOUT);
		}

	}


	inSetEDCLOCK("N");
	inSaveEDCRec(0);

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("EDC UNLOCK OK", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);

	inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);
        inDISP_BEEP(1, 0);

	while (1)
	{
		uszKey = uszKBD_GetKey(_EDC_TIMEOUT_);

		if (uszKey == _KEY_ENTER_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}

	}

	return	(inRetVal);
}

/*
Function	:inFunc_Unlock_EDCLock_And_Delete_Batch
Date&Time	:2017/3/28 下午 2:11
Describe        :解鎖鎖機狀態並清除全部批次
*/
int inFunc_Unlock_EDCLock_And_Delete_Batch(void)
{
	int	i;
	int	inRetVal = VS_SUCCESS;
	char	szHostEnable[2 + 1];
	char	szHostName[42 + 1];
	unsigned char	uszKey;
	TRANSACTION_OBJECT	pobTran;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	memset(&pobTran, 0x00, sizeof(pobTran));

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("EDC UNLOCK &", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
	inDISP_ChineseFont("DELETE BATCH？", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
	inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
	inDISP_BEEP(1, 0);

	while (1)
	{
		uszKey = uszKBD_GetKey(_EDC_TIMEOUT_);

		if (uszKey == _KEY_0_)
		{
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			return (VS_USER_CANCEL);
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			return (VS_TIMEOUT);
		}

	}

	inSetEDCLOCK("N");
	inSaveEDCRec(0);

	for (i = 0;; ++i)
	{
		/* 先LoadHDT */
		if (inLoadHDTRec(i) == VS_ERROR)
		{
			inDISP_LogPrintfWithFlag(" Func Unlock_EDCLock_And_Delete_Batch Load HDT[%d] *Error* Line[%d]", i, __LINE__);
			/* 當找不到第i筆資料會回傳VS_ERROR */
			break;
		}

		/* ESC不該出現在選單上 */
		memset(szHostName, 0x00, sizeof(szHostName));
		inGetHostLabel(szHostName);
		if (memcmp(szHostName, _HOST_NAME_ESC_, strlen(_HOST_NAME_ESC_)) == 0)
		{
			continue;
		}
		/* 因為公司TMS會下CUP主機，因富邦是同批次，所以CUP要跳過 所以新增 20190214 [SAM] */
		if (memcmp(szHostName, _HOST_NAME_CUP_, strlen(_HOST_NAME_CUP_)) == 0)
		{
			continue;
		}
		
		/* GET HOST Enable */
		memset(szHostEnable, 0x00, sizeof(szHostEnable));
		if (inGetHostEnable(szHostEnable) == VS_ERROR)
		{
			return (VS_ERROR);
		}
		else
		{
			if (memcmp(szHostEnable, "Y", 1) != 0)
			{
				/* 如果HostEnable != Y，就continue */
				continue;
			}

			pobTran.srBRec.inHDTIndex = i;

			/* 如果主機有開，才loadHDPT */
			if (inLoadHDPTRec(pobTran.srBRec.inHDTIndex) == VS_ERROR)
			{
				/* 當找不到第i筆資料會回傳VS_ERROR */
				return (VS_ERROR);
			}

			inFLOW_RunOperation(&pobTran, _OPERATION_DELETE_BATCH_);
		}

	}/* End of For loop */

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("EDC UNLOCK &", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
	inDISP_ChineseFont("DELETE BATCH OK", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
	inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);
	inDISP_BEEP(1, 0);

	while (1)
	{
		uszKey = uszKBD_GetKey(_EDC_TIMEOUT_);

		if (uszKey == _KEY_ENTER_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}

	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return(inRetVal);
}

/*
Function        :inFunc_Set_TMSOK_Flow
Date&Time       :2017/3/28 下午 2:54
Describe        :
*/
int inFunc_Set_TMSOK_Flow()
{
	char	szKey;

	while(1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont_Color("1.設定TMSOK", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
		inDISP_ChineseFont_Color("2.設定Table 參數", _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);

		szKey = uszKBD_GetKey(_EDC_TIMEOUT_);
		if (szKey == _KEY_1_)
		{
			inFunc_Set_TMSOK();
		}
		else if (szKey == _KEY_2_)
		{
			inFunc_Edit_Table();
		}
		else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
		{
			break;
		}

	}

	return (VS_SUCCESS);
}

/*
Function	:inFunc_Set_TMSOK
Date&Time	:2016/8/16 下午 4:14
Describe        :設定TMSOK
*/
int inFunc_Set_TMSOK(void)
{
	int		inRetVal;
        char		szTemplate[64 + 1];
	char		szTMSOK[1 + 1];
        DISPLAY_OBJECT  srDispObj;

	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);

	/* Debug */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        memset(szTemplate, 0x00, sizeof(szTemplate));

        sprintf(szTemplate, "TMSOK : %s", szTMSOK);

        inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
	inDISP_ChineseFont("1 = Y , 0 = N", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
        while (1)
        {
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                srDispObj.inMaxLen = 1;
                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

                inRetVal = inDISP_Enter8x16(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                        return (VS_ERROR);

                if (strlen(srDispObj.szOutput) > 0)
                {
                        if (srDispObj.szOutput[0] == '0')
                        {
                                inSetTMSOK("N");
			inSaveEDCRec(0);
                                break;
                        }
                        else if (srDispObj.szOutput[0] == '1')
                        {
                                inSetTMSOK("Y");
			inSaveEDCRec(0);
                                break;
                        }
                        else
                        {
                                inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
                                continue;
                        }
                }
                else
                {
                        break;
                }
        }

	return	(VS_SUCCESS);
}

/*
Function	:inFunc_Edit_Table
Date&Time	:2017/3/28 下午 3:00
Describe        :設定Table
*/
int inFunc_Edit_Table(void)
{
	int	inRetVal;

	do
	{
		inRetVal = inCFGT_Edit_CFGT_Table();
	} while (inRetVal != VS_USER_CANCEL);

	do
	{
		inRetVal = inEDC_Edit_EDC_Table();
	} while (inRetVal != VS_USER_CANCEL);

	do
	{
		inRetVal = inHDPT_Edit_HDPT_Table();
	} while (inRetVal != VS_USER_CANCEL);

	do
	{
		inRetVal = inCDT_Edit_CDT_Table();
	} while (inRetVal != VS_USER_CANCEL);

	do
	{
		inRetVal = inHDT_Edit_HDT_Table();
	} while (inRetVal != VS_USER_CANCEL);

	do
	{
		inRetVal = inCPT_Edit_CPT_Table();
	} while (inRetVal != VS_USER_CANCEL);

	do
	{
		inRetVal = inTMSCPT_Edit_TMSCPT_Table();
	} while (inRetVal != VS_USER_CANCEL);

	do
	{
		inRetVal = inTMSSCT_Edit_TMSSCT_Table();
	} while (inRetVal != VS_USER_CANCEL);

	do
	{
		inRetVal = inTMSFTP_Edit_TMSFTP_Table();
	} while (inRetVal != VS_USER_CANCEL);

	do
	{
		inRetVal = inTDT_Edit_TDT_Table();
	} while (inRetVal != VS_USER_CANCEL);

	do
	{
		inRetVal = inIPASSDT_Edit_IPASSDT_Table();
	} while (inRetVal != VS_USER_CANCEL);

	do
	{
		inRetVal = inECCDT_Edit_ECCDT_Table();
	} while (inRetVal != VS_USER_CANCEL);

	do
	{
		inRetVal = inPWD_Edit_PWD_Table();
	} while (inRetVal != VS_USER_CANCEL);

	/* [新增SVC功能]  2022/12/28 下午 6:59 [SAM] */
	do
	{
		inRetVal = inSVC_TABLE_EditTable();
	} while (inRetVal != VS_USER_CANCEL);
	
	return	(VS_SUCCESS);
}

/*
Function        :inFunc_Edit_Table_Tag
Date&Time       :2017/3/28 下午 4:29
Describe        :
*/
int inFunc_Edit_Table_Tag(TABLE_GET_SET_TABLE* srTable)
{
	int		i;
	int		inRetVal;
	char		szKey;
	char		szTemplate[22 + 1];
        DISPLAY_OBJECT  srDispObj;

	for (i = 0; strlen(srTable[i].szTag) > 0; i++)
	{
		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		srDispObj.inY = _LINE_8_8_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMaxLen = 18;
		srDispObj.inColor = _COLOR_BLACK_;
		inRetVal = srTable[i].inGetFunctionPoint(szTemplate);

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont_Color(srTable[i].szTag, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
		inDISP_ChineseFont_Color(szTemplate, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);
		inDISP_ChineseFont_Color("更改按0 跳過按Enter", _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);
		while (1)
		{
			szKey = uszKBD_GetKey(_EDC_TIMEOUT_);
			if (szKey == _KEY_0_)
			{
				while (1)
				{
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					inDISP_ChineseFont_Color("確認按Enter放棄按Cancel", _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);

					srDispObj.inOutputLen = strlen(szTemplate);
					memcpy(srDispObj.szOutput, szTemplate, srDispObj.inOutputLen);
					inDISP_ChineseFont_Color(srDispObj.szOutput, _FONTSIZE_8X16_, _LINE_8_8_, _COLOR_BLACK_, _DISP_RIGHT_);

					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
					{
						i --;
						if (i < 0)
						{
							i = 0;
						}

						break;
					}
					else if (srDispObj.inOutputLen >= 0)
					{
						inRetVal = srTable[i].inSetFunctionPoint(srDispObj.szOutput);
						if (inRetVal != VS_SUCCESS)
						{
							return (inRetVal);
						}

						i --;
						break;
					}

				}

				break;
			}
			else if (szKey == _KEY_CANCEL_ )
			{
				inRetVal = VS_USER_CANCEL;

				return	(inRetVal);
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;

				return	(inRetVal);
			}
			else if (szKey == _KEY_ENTER_)
			{
				break;
			}
			else if (szKey == _KEY_CLEAR_)
			{
				if ((i - 2 + 1) >= 0)
				{
					i -= 2;
				}
				else
				{
					i = 0;
					do
					{
						i++;
					}
					while (strlen(srTable[i + 2].szTag) > 0);
				}
				break;
			}

		}

		/* 到Table底端時，循環從第一個繼續 */
		if (strlen(srTable[i + 1].szTag) == 0)
		{
			i = -1;
		}
	}

	return (VS_SUCCESS);
}

/*
Function	:inFunc_Reboot
Date&Time	:2016/6/3 下午 2:42
Describe        :重新開機
*/
int inFunc_Reboot()
{
	CTOS_SystemReset();

	return	(VS_SUCCESS);
}

/*
Function	:inFunc_Exit_AP
Date&Time	:2017/9/29 上午 9:30
Describe        :離開程式
*/
int inFunc_Exit_AP()
{
	inDISP_ChineseFont("請稍候...", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);

	exit(0);

	return	(VS_SUCCESS);
}

/*
Function        :inFunc_GetSeriaNumber
Date&Time       :2016/10/11 下午 6:28
Describe        :szSerialNumber sizr一定要大於16
*/
int inFunc_GetSeriaNumber(char* szSerialNumber)
{
	int		inRetVal = VS_ERROR;
	char		szDebugMsg[100 + 1] = {0};
	unsigned char	uszTemplate[16 + 1] = {0};

	memset(uszTemplate, 0x00, sizeof(uszTemplate));
	/* The last digit is the check code of factory serial number. The check code is calculated by exclusive-OR (XOR) the first 15 digits. */
	inRetVal = CTOS_GetFactorySN(uszTemplate);
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inFunc_GetSeriaNumber(%d,%d) START,%s!!",inRetVal, strlen((char*)uszTemplate), uszTemplate);
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inFunc_GetSeriaNumber() ERROR!0x%04X", inRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}

		return (VS_ERROR);
	}
	memset(szSerialNumber, 0x00, 16);
	memcpy(szSerialNumber, uszTemplate, 16);

	return (VS_SUCCESS);
}

/*
Function        :inFunc_GetSystemInfo
Date&Time       :2018/1/11 下午 3:37
Describe        :Note Minimum output buffer size is 17 bytes.
*/
int inFunc_GetSystemInfo(unsigned char uszID, unsigned char *uszBuffer)
{
	int		inRetVal = 0;
	char		szDebugMsg[100 + 1];

	memset(uszBuffer, 0x00, 17);
	inRetVal = CTOS_GetSystemInfo(uszID, uszBuffer);
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inFunc_GetSystemInfo() ERROR!0x%04X", inRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}

		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_GetOSVersion
Date&Time       :2018/1/11 下午 3:37
Describe        :
*/
int inFunc_GetOSVersion(unsigned char *uszBuffer)
{
	int	inRetVal = 0;
	char	szDebugMsg[100 + 1];

	inRetVal = inFunc_GetSystemInfo(ID_ROOTFS, uszBuffer);
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inFunc_GetOSVersion() ERROR!");
			inDISP_LogPrintf(szDebugMsg);
		}

		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Sync_Debug_Switch
Date&Time       :2016/12/6 上午 10:37
Describe        :
*/
int inFunc_Sync_Debug_Switch(void)
{
	char	szISODebug[100 + 1];

	memset(szISODebug, 0x00, sizeof(szISODebug));
	inGetISODebug(szISODebug);
	if (memcmp(&szISODebug[0], "1", 1) == 0)
	{
		ginDebug = VS_TRUE;
	}
	else
	{
		ginDebug = VS_FALSE;
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Sync_ISODebug_Switch
Date&Time       :2017/3/28 下午 2:49
Describe        :
*/
int inFunc_Sync_ISODebug_Switch(void)
{
	char	szISODebug[100 + 1];

	memset(szISODebug, 0x00, sizeof(szISODebug));
	inGetISODebug(szISODebug);

	if (memcmp(&szISODebug[1], "1", 1) == 0)
	{
		ginISODebug = VS_TRUE;
	}
	else
	{
		ginISODebug = VS_FALSE;
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Sync_DisplayDebug_Switch
Date&Time       :2017/7/6 上午 10:11
Describe        :
*/
int inFunc_Sync_DisplayDebug_Switch(void)
{
	char	szDisplayDebug[100 + 1];

	memset(szDisplayDebug, 0x00, sizeof(szDisplayDebug));
	inGetISODebug(szDisplayDebug);

	if (memcmp(&szDisplayDebug[2], "1", 1) == 0)
	{
		ginDisplayDebug = VS_TRUE;
	}
	else
	{
		ginDisplayDebug = VS_FALSE;
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Sync_EngineerDebug_Switch
Date&Time       :2018/6/14 下午 6:36
Describe        :
*/
int inFunc_Sync_EngineerDebug_Switch(void)
{
	char	szEngineerDebug[100 + 1];

	memset(szEngineerDebug, 0x00, sizeof(szEngineerDebug));
	inGetISODebug(szEngineerDebug);

	if (memcmp(&szEngineerDebug[3], "1", 1) == 0)
	{
		ginEngineerDebug = VS_TRUE;
	}
	else
	{
		ginEngineerDebug = VS_FALSE;
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Is_Portable_Type
Date&Time       :2016/12/20 上午 11:43
Describe        :確認是否為Portable機型
*/
int inFunc_Is_Portable_Type(void)
{
	char		szDebugMsg[100 + 1];
	unsigned char	uszPortable;
	unsigned char	uszPCI;
	unsigned short	usHWSupport;
	unsigned short	usRetVal;

	usRetVal = CTOS_HWSettingGet(&uszPortable, &uszPCI, &usHWSupport);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_HWSettingGet Err: 0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}

		return (VS_ERROR);
	}

	if (uszPortable == d_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "This is portable type");
			inDISP_LogPrintf(szDebugMsg);
		}

		return (VS_TRUE);
	}
	else
	{
		return (VS_FALSE);
	}

}

/*
Function        :inFunc_Is_Cradle_Attached
Date&Time       :2016/12/20 上午 11:43
Describe        :確認是否Portable機型有接在底座上
*/
int inFunc_Is_Cradle_Attached(void)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;

	usRetVal = CTOS_CradleAttached();
	if (usRetVal == d_YES)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_Cradle is Attached");
			inDISP_LogPrintf(szDebugMsg);
		}

		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_Cradle is not Attached");
			inDISP_LogPrintf(szDebugMsg);
		}

		return (VS_ERROR);
	}
}

/*
Function	:inFunc_Find_Specific_HDTindex
Date&Time	:2016/9/1 上午 11:10
Describe        :傳進特定HostName，找這個Host在HDT.dat中的index，如果沒找到，
 * 則inHostIndex回傳-1，不用切Host，也不抓HostEnable時用
 *  只抓取對應 szHostName 的Index 並設定到 inHostIndex ,之後 會重LOAD inOrgIndex 對應的 HDT 
*/
int inFunc_Find_Specific_HDTindex(int inOrgIndex, char *szHostName, int *inHostIndex)
{
	int	i = 0;
	char	szTemplate[42 + 1];

	do
	{
		/* 按順序load每一個Host */
		if (inLoadHDTRec(i) < 0)
		{
			/* 沒找到index，設為-1 */
			*inHostIndex = -1;

			/* 代表有可回復的Host(保險機制) */
			if (inOrgIndex >= 0)
			{
				/* 回覆原本的Host */
				if (inLoadHDTRec(inOrgIndex) == VS_ERROR)
				{
					inDISP_DispLogAndWriteFlie(" Func Find_Specific_HDTindex Load HDT OrgId[%d] *Error* Line[%d]", inOrgIndex, __LINE__);
				}
			}

			inDISP_DispLogAndWriteFlie(" inFunc_Find_Specific_HDTindex Fail :[%d] [%s]",inOrgIndex, szHostName);
			/* 如果return VS_ERROR 代表table沒有該Host */
			return (VS_ERROR);
		}

		/* 理論上只用8byte位置 */
		memset(szTemplate, 0x00, 8);
		inGetHostLabel(szTemplate);
		if (!memcmp(szTemplate, szHostName, strlen(szHostName)))
		{
			/* 找到的index */
			*inHostIndex = i;

			/* 代表有可回復的Host(保險機制) */
			if(inOrgIndex >= 0)
			{
				/* 回覆原本的Host */
				if (inLoadHDTRec(inOrgIndex) == VS_ERROR)
				{
					inDISP_DispLogAndWriteFlie(" Func Find_Specific_HDTindex Load HDT OrgId[%d] *Error* Line[%d]", inOrgIndex, __LINE__);
				}
			}
			return (VS_SUCCESS);

		}
		
		i++;
	} while (1);

	return (VS_SUCCESS);
}

/*
Function	:inFunc_Find_Specific_HDTindex
Date&Time	:2016/9/1 上午 11:10
Describe        :傳進特定HDTIndex，找這個Index在HDT.dat中的真正的index(因為DCC所以會調動順序)，如果沒找到，則inHostIndex回傳-1，不用切Host，也不抓HostEnable時用
*/
int inFunc_Find_Specific_HDTindex_ByCDTIndex(int inOrgIndex, char *szHDTIndex, int *inHostIndex)
{
	int	i = 0;
	char	szTemplate[42 + 1];

	do
	{
		/* 按順序load每一個Host */
		if (inLoadHDTRec(i) < 0)
		{
			/* 沒找到index，設為-1 */
			*inHostIndex = -1;

			/* 代表有可回復的Host(保險機制) */
			if(inOrgIndex >= 0)
			{
				/* 回覆原本的Host */
				if (inLoadHDTRec(inOrgIndex) == VS_ERROR)
				{
					inDISP_DispLogAndWriteFlie(" inFunc_Find_Specific_HDTindex_ByCDTIndex Load HDT OrgId[%d] *Error* Line[%d]", inOrgIndex, __LINE__);
				}
			}

			inDISP_DispLogAndWriteFlie(" inFunc_Find_Specific_HDTindex_ByCDTIndex Fail :[%d] [%s]",inOrgIndex, szHDTIndex);
			/* 如果return VS_ERROR 代表table沒有該Host */
			return (VS_ERROR);
		}

		/* 理論上只用2byte位置 */
		memset(szTemplate, 0x00, 2);
		inGetHostIndex(szTemplate);
		if (!memcmp(szTemplate, szHDTIndex, strlen(szHDTIndex)))
		{
			/* 找到的index */
			*inHostIndex = i;

			/* 代表有可回復的Host(保險機制) */
			if(inOrgIndex >= 0)
			{
				/* 回覆原本的Host */
				if (inLoadHDTRec(inOrgIndex) == VS_ERROR)
				{
					inDISP_DispLogAndWriteFlie(" inFunc_Find_Specific_HDTindex_ByCDTIndex Load HDT OrgId[%d] *Error* Line[%d]", inOrgIndex, __LINE__);
				}
			}

			return (VS_SUCCESS);
		}
		
		i++;
	} while (1);

	return (VS_SUCCESS);
}

/*
Function	:inFunc_Find_Specific_HDPTindex
Date&Time	:2016/9/1 上午 11:10
Describe        :傳進特定HostName，找這個Host在HDT.dat中的index，如果沒找到，則inHostIndex回傳-1，不用切Host，也不抓HostEnable時用
*/
int inFunc_Find_Specific_HDPTindex(int inOrgIndex, char *szTRTName, int *inTRTIndex)
{
	int	i = 0;
	char	szTemplate[42 + 1];

	do
	{
		/* 按順序load每一個Host */
		if (inLoadHDPTRec(i) < 0)
		{
			/* 沒找到index，設為-1 */
			*inTRTIndex = -1;

			/* 代表有可回復的Host(保險機制) */
			if(inOrgIndex >= 0)
			{
				/* 回覆原本的Host */
				if (inLoadHDTRec(inOrgIndex) == VS_ERROR)
				{
					inDISP_DispLogAndWriteFlie(" inFunc_Find_Specific_HDPTindex Load HDT OrgId[%d] *Error* Line[%d]", inOrgIndex, __LINE__);
				}
			}
			
			inDISP_DispLogAndWriteFlie(" inFunc_Find_Specific_HDPTindex Fail :[%d] [%s]",inOrgIndex, szTRTName);
			/* 如果return VS_ERROR 代表table沒有該Host */
			return (VS_ERROR);
		}

		/* 理論上只用12byte位置 */
		memset(szTemplate, 0x00, 12);
		inGetTRTFileName(szTemplate);
		if (!memcmp(szTemplate, szTRTName, strlen(szTRTName)))
		{
			/* 找到的index */
			*inTRTIndex = i;

			/* 代表有可回復的Host(保險機制) */
			if(inOrgIndex >= 0)
			{
				/* 回覆原本的Host */
				if (inLoadHDTRec(inOrgIndex) == VS_ERROR)
				{
					inDISP_DispLogAndWriteFlie(" inFunc_Find_Specific_HDPTindex Load HDT OrgId[%d] *Error* Line[%d]", inOrgIndex, __LINE__);
				}
			}
			return (VS_SUCCESS);

		}
		
		i++;
	} while (1);

	return (VS_SUCCESS);
}

/*
Function	:inFunc_Find_Specific_CDTindex
Date&Time	:2016/11/25 下午 1:08
Describe        :傳進特定卡別，找這個卡別在CDT.dat中的index，如果沒找到，則inCDTIndex回傳-1
*/
int inFunc_Find_Specific_CDTindex(int inOrgIndex, char *szCardLabel, int *inCDTIndex)
{
	int	i = 0;
	char	szTemplate[42 + 1];

	inDISP_LogPrintfWithFlag("  Compare Specific_CDT[%s]", szCardLabel);
	do
	{
		/* 按順序load每一個CDTRec */
		if (inLoadCDTRec(i) < 0)
		{
			/* 沒找到index，設為-1 */
			*inCDTIndex = -1;

			/* 代表有可回復的CDT(保險機制) */
			if(inOrgIndex >= 0)
			{
				/* 回覆原本的CDT */
				inLoadCDTRec(inOrgIndex);
			}
			inDISP_DispLogAndWriteFlie(" inFunc_Find_Specific_CDTindex Fail :[%d] [%s]",inOrgIndex, szCardLabel);
			/* 如果return VS_ERROR 代表table沒有該Host */
			return (VS_ERROR);
		}

		/* 理論上只用8byte位置 */
		memset(szTemplate, 0x00, 20);
		inGetCardLabel(szTemplate);
		inDISP_LogPrintfWithFlag("  Specific_CDT[%s]",szTemplate);
		
		if (!memcmp(szTemplate, szCardLabel, strlen(szCardLabel)))
		{
			/* 找到的index */
			*inCDTIndex = i;

			/* 代表有可回復的CDT(保險機制) */
			if(inOrgIndex >= 0)
			{
				/* 回覆原本的CDT */
				inLoadCDTRec(inOrgIndex);
			}
			return (VS_SUCCESS);

		}
		
		i++;
	} while (1);

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Get_HDPT_General_Data
Date&Time       :2017/2/6 下午 1:08
Describe        :Load到Host必Load的Batch Number, STAN Number, Invoice Number
*/
int inFunc_Get_HDPT_General_Data(TRANSACTION_OBJECT * pobTran)
{
	int	inRetVal;
	char	szBatchNum [6 + 1], szInvoiceNum [6 + 1], szSTANNum [6 + 1];

	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	memset(szBatchNum, 0x00, sizeof(szBatchNum));
	inRetVal = inGetBatchNum(szBatchNum);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" inFunc_Get_HDPT_General_Data GetBatchNum *Error* Line[%d]", __LINE__);
		return (inRetVal);
	}
	pobTran->srBRec.lnBatchNum = atol(szBatchNum);

	memset(szSTANNum, 0x00, sizeof(szSTANNum));
	inGetSTANNum(szSTANNum);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" inFunc_Get_HDPT_General_Data GetStanNum *Error* Line[%d]", __LINE__);
		return (inRetVal);
	}
	pobTran->srBRec.lnSTANNum = atol(szSTANNum);

	memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
	inGetInvoiceNum(szInvoiceNum);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" inFunc_Get_HDPT_General_Data GetInvNum *Error* Line[%d]", __LINE__);
		return (inRetVal);
	}
	pobTran->srBRec.lnOrgInvNum = atol(szInvoiceNum);

	return (VS_SUCCESS);
}

/*
Function        : inFunc_Check_Linkage_Function_Enable
Date&Time   : 2016/11/25 下午 1:28
Describe        : 確認該TMS的連動開關
 *		 確認該卡別是否有在CDT並連動到EDC開關
*/
int inFunc_Check_Linkage_Function_Enable(TRANSACTION_OBJECT * pobTran)
{
	int	inCDTIndex = -1;
	char	szHostEnable[2 + 1];
	char	szESCReciptUploadUpLimit[4 + 1];
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 是否接收CUP卡 */
	if (inFunc_Find_Specific_CDTindex(pobTran->srBRec.inCDTIndex, _CARD_TYPE_CUP_, &inCDTIndex) == VS_SUCCESS)
	{
		if (inSetCUPFuncEnable("Y") != VS_SUCCESS)
		{
			inFunc_EDCLock();
		}

		if (inSaveEDCRec(0) != VS_SUCCESS)
		{
			inFunc_EDCLock();
		}

	}
	else
	{
		if (inSetCUPFuncEnable("N") != VS_SUCCESS)
		{
			inFunc_EDCLock();
		}
		if (inSaveEDCRec(0) != VS_SUCCESS)
		{
			inFunc_EDCLock();
		}
		
		
		
	}

	/* 是否接受金融卡 */
	if (inFunc_Find_Specific_CDTindex(pobTran->srBRec.inCDTIndex, _CARD_TYPE_SMARTPAY_, &inCDTIndex) == VS_SUCCESS)
	{
		if (inSetFiscFuncEnable("Y") != VS_SUCCESS)
		{
			inFunc_EDCLock();
		}
		if (inSaveEDCRec(0) != VS_SUCCESS)
		{
			inFunc_EDCLock();
		}
	}
	else
	{
		if (inSetFiscFuncEnable("N") != VS_SUCCESS)
		{
			inFunc_EDCLock();
		}
		if (inSaveEDCRec(0) != VS_SUCCESS)
		{
			inFunc_EDCLock();
		}

	}

	/* 確認是否要送NE欄位 */
	/* ESC沒開，不送NE */
	memset(szHostEnable, 0x00, sizeof(szHostEnable));
	if (inNEXSYS_ESC_GetESC_Enable(pobTran->srBRec.inHDTIndex, szHostEnable) != VS_SUCCESS)
	{
		inDISP_LogPrintfWithFlag( "  ESC Mode : N, 找不到該Host");

		if (inSetESCMode("N") != VS_SUCCESS)
		{
			inFunc_EDCLock();
		}
		if (inSaveEDCRec(0) != VS_SUCCESS)
		{
			inFunc_EDCLock();
		}

	}
	else
	{
		if (szHostEnable[0] != 'Y')
		{
			inDISP_LogPrintfWithFlag("  ESC Mode : N, Host未開");

			if (inSetESCMode("N") != VS_SUCCESS)
			{
				inFunc_EDCLock();
			}
			if (inSaveEDCRec(0) != VS_SUCCESS)
			{
				inFunc_EDCLock();
			}

		}
		else
		{
			inDISP_LogPrintfWithFlag("  ESC Mode : Y");

			if (inSetESCMode("Y") != VS_SUCCESS)
			{
				inFunc_EDCLock();
			}
			
			if (inSaveEDCRec(0) != VS_SUCCESS)
			{
				inFunc_EDCLock();
			}

		}

	}

	memset(szESCReciptUploadUpLimit, 0x00, sizeof(szESCReciptUploadUpLimit));
	inGetESCReciptUploadUpLimit(szESCReciptUploadUpLimit);

	/* 若水位為0，也當成不開ESC */
	if (atoi(szESCReciptUploadUpLimit) == 0)
	{
		inDISP_LogPrintfWithFlag("  水位為0，不送NE");

		if (inSetESCMode("N") != VS_SUCCESS)
		{
			inFunc_EDCLock();
		}
		if (inSaveEDCRec(0) != VS_SUCCESS)
		{
			inFunc_EDCLock();
		}
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckCustomizePassword
Date&Time       :
Describe        :
*/
int inFunc_CheckCustomizePassword(int inPasswordLevel, int inCode)
{
	char		szTerminalPwd[16 + 1] = {0};
	char		szPWDEnable[2 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	int		inRetVal = 0;
	DISPLAY_OBJECT  srDispObj;

	if (inPasswordLevel == _ACCESS_FREELY_)
		return (VS_SUCCESS);

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTerminalPwd, 0x00, sizeof(szTerminalPwd));
	memset(szPWDEnable, 0x00, sizeof(szPWDEnable));

	if (inPasswordLevel == _ACCESS_WITH_CUSTOM_)
	{
		if (inLoadPWDRec(0) < 0)
			return (VS_SUCCESS);

		switch (inCode)
		{
			case _SALE_ :
			case _CUP_SALE_:
			case _FISC_SALE_:
				inGetSalePwdEnable(szPWDEnable);

				if (!memcmp(szPWDEnable, "Y", 1))
				{
					inGetSalePwd(szTerminalPwd);
					break;
				}
				else
				{
					return (VS_SUCCESS);
				}
			case _INST_SALE_ :
			case _CUP_INST_SALE_ :
				inGetInstallmentPwdEnable(szPWDEnable);

				if (!memcmp(szPWDEnable, "Y", 1))
				{
					inGetInstallmentPwd(szTerminalPwd);
					break;
				}
				else
				{
					return (VS_SUCCESS);
				}
			case _REDEEM_SALE_ :
			case _CUP_REDEEM_SALE_ :
				inGetRedeemPwdEnable(szPWDEnable);

				if (!memcmp(szPWDEnable, "Y", 1))
				{
					inGetRedeemPwd(szTerminalPwd);
					break;
				}
				else
				{
					return (VS_SUCCESS);
				}
			case _PRE_AUTH_ :
			case _CUP_PRE_AUTH_ :
			case _PRE_COMP_ :
			case _CUP_PRE_COMP_ :
				inGetPreauthPwdEnable(szPWDEnable);

				if (!memcmp(szPWDEnable, "Y", 1))
				{
					inGetPreauthPwd(szTerminalPwd);
					break;
				}
				else
				{
					return (VS_SUCCESS);
				}
			case _VOID_ :
			case _CUP_VOID_ :
			case _CUP_PRE_AUTH_VOID_ :
			case _CUP_PRE_COMP_VOID_ :
			case _FISC_VOID_ :
				inGetVoidPwdEnable(szPWDEnable);

				if (!memcmp(szPWDEnable, "Y", 1))
				{
					inGetVoidPwd(szTerminalPwd);
					break;
				}
				else
				{
					return (VS_SUCCESS);
				}
			case _REFUND_ :
			case _INST_REFUND_ :
			case _REDEEM_REFUND_ :
			case _CUP_REFUND_ :
			case _FISC_REFUND_:
			case _CUP_MAIL_ORDER_REFUND_:
				inGetRefundPwdEnable(szPWDEnable);

				if (!memcmp(szPWDEnable, "Y", 1))
				{
					inGetRefundPwd(szTerminalPwd);
					break;
				}
				else
				{
					return (VS_SUCCESS);
				}
			case _SETTLE_ :
				inGetSettlementPwdEnable(szPWDEnable);

				if (!memcmp(szPWDEnable, "Y", 1))
				{
					inGetSettlementPwd(szTerminalPwd);
					break;
				}
				else
				{
					return (VS_SUCCESS);
				}
			case _SALE_OFFLINE_ :
				inGetOfflinePwdEnable(szPWDEnable);

				if (!memcmp(szPWDEnable, "Y", 1))
				{
					inGetOfflinePwd(szTerminalPwd);
					break;
				}
				else
				{
					return (VS_SUCCESS);
				}
			case _INST_ADJUST_ :
				inGetInstallmentAdjustPwdEnable(szPWDEnable);

				if (!memcmp(szPWDEnable, "Y", 1))
				{
					inGetInstallmentAdjustPwd(szTerminalPwd);
					break;
				}
				else
				{
					return (VS_SUCCESS);
				}
			case _REDEEM_ADJUST_ :
				inGetRedeemAdjustPwdEnable(szPWDEnable);

				if (!memcmp(szPWDEnable, "Y", 1))
				{
					inGetRedeemAdjustPwd(szTerminalPwd);
					break;
				}
				else
				{
					return (VS_SUCCESS);
				}
			case _POWER_ON_ :
				inGetRebootPwdEnale(szPWDEnable);

				if (!memcmp(szPWDEnable, "Y", 1))
				{
					inDISP_ClearAll();
					inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
					inDISP_PutGraphic(_MENU_POWER_ON_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 開機 */
					inGetRebootPwd(szTerminalPwd);
					break;
				}
				else
				{
					return (VS_SUCCESS);
				}
			case _MAIL_ORDER_:
			case _CUP_MAIL_ORDER_:
				inGetMailOrderPwdEnable(szPWDEnable);

				if (!memcmp(szPWDEnable, "Y", 1))
				{
					inGetMailOrderPwd(szTerminalPwd);
					break;
				}
				else
				{
					return (VS_SUCCESS);
				}
			case _CALL_BANK_:
				inGetCallBankForcePwdEnable(szPWDEnable);

				if (!memcmp(szPWDEnable, "Y", 1))
				{
					inGetCallBankForcePwd(szTerminalPwd);
					break;
				}
				else
				{
					return (VS_SUCCESS);
				}
				break;
			case _TIP_:
				return (VS_SUCCESS);
			case _ADJUST_:
				inGetAdjustPwdEnable(szPWDEnable);

				if (!memcmp(szPWDEnable, "Y", 1))
				{
					inGetAdjustPwd(szTerminalPwd);
					break;
				}
				else
				{
					return (VS_SUCCESS);
				}
				
				break;
			case _LOYALTY_REDEEM_:
				return (VS_SUCCESS);
				break;
			case _VOID_LOYALTY_REDEEM_:
				return (VS_SUCCESS);
				break;
			case _LOYALTY_REDEEM_REFUND_:
				return (VS_SUCCESS);
				break;
			default :
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Not Set this PW Incode : %d", inCode);
					inDISP_LogPrintf(szDebugMsg);
				}
				return (VS_ERROR);
				break;
		}
	}
	else if (inPasswordLevel == _ACCESS_WITH_MANAGER_PASSWORD_ ||
		 inPasswordLevel == _ACCESS_WITH_FUNC_PASSWORD_ ||
		 inPasswordLevel == _ACCESS_WITH_MERCHANT_PASSWORD_ ||
		 inPasswordLevel == _ACCESS_WITH_SUPER_PASSWORD_)
	{
		if (inPasswordLevel == _ACCESS_WITH_MANAGER_PASSWORD_)
		{
			inGetManagerPassword(szTerminalPwd);
		}
		else if (inPasswordLevel == _ACCESS_WITH_MERCHANT_PASSWORD_)
		{
			inGetMerchantPassword(szTerminalPwd);
		}
		else if (inPasswordLevel == _ACCESS_WITH_FUNC_PASSWORD_)
		{
			inGetFunctionPassword(szTerminalPwd);
		}
		else if (inPasswordLevel == _ACCESS_WITH_SUPER_PASSWORD_)
		{
			inGetSuperPassword(szTerminalPwd);
		}
	}
        else if(inPasswordLevel == _ACCESS_WITH_EDC_UNLOACK_PASSWORD_)
	{
		strcpy(szTerminalPwd, "332865625");
	}
	else
        {
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("No such PasswordLevel.");
		}

		return (VS_ERROR);
        }

	if (strlen(szTerminalPwd) <= 0) /* 表示沒有密碼 */
		return (VS_SUCCESS);

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	if (inPasswordLevel == _ACCESS_WITH_CUSTOM_)
		inDISP_PutGraphic(_GET_TXN_PWD_, 0, _COORDINATE_Y_LINE_8_4_); /* 請輸入管理號碼 */
	else
	{
		inDISP_PutGraphic(_GET_SYSTEM_PWD_, 0, _COORDINATE_Y_LINE_8_4_); /* 輸入系統管理號碼 */
	}

	while (1)
	{
		inDISP_BEEP(1, 0);
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

		/* 設定顯示變數 */
		srDispObj.inMaxLen = 16;
		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMask = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
		srDispObj.inTimeout = 30;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (VS_ERROR);

		if (srDispObj.inOutputLen == strlen(szTerminalPwd))
		{
			if (!memcmp(szTerminalPwd, srDispObj.szOutput, strlen(szTerminalPwd)))
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				break;
			}
			else
			{
				inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
			}
		}
		else
		{
			inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		}
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_GetHostNum
Date&Time       :2015/10/06 上午 10:00
Describe        :Host選單，如果只有一個主機就直接LoadHDT和HDPT，選定HostNum，並load HDT和HDPT
*/
int inFunc_GetHostNum(TRANSACTION_OBJECT *pobTran)
{
	int		inOpenHostCnt = 0;      /* 記錄有幾個Host有開 */
	int		inLine = 0;             /* 第幾行 */
	int		i, j = 0;               /* j是inHostIndex陣列索引 */
	int		inHostIndex[12 + 1];    /* 記錄HostEnable為Y的HostIndex */
	int		inLine1Index = 0;       /* szLine1的index */
	int		inLine2Index = 0;       /* szLine2的index */
	int		inLine3Index = 0;       /* szLine3的index */
	int		inLine4Index = 0;
	int		inKey = 0;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_12X19_OPT_;
	int		inRetVal = VS_SUCCESS;
	char		szKey;
	char		szHostEnable[2 + 1];
	char		szHostName[42 + 1];
	char		szTemp[48 + 1];
	char		szLine1[48 + 1];        /* 存第一行要顯示的Host */	/* linux系統中文字length一個字為3，小心爆掉 */
	char		szLine2[48 + 1];        /* 存第二行要顯示的Host */
	char		szLine3[48 + 1];        /* 存第三行要顯示的Host */
	char		szLine4[48 + 1];
	char		szBatchNum[6 + 1];
	char		szTimeout[4 + 1];
	DISPLAY_OBJECT  srDispObj;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 如果要連動結帳，跳過選Host流程 */
	if (pobTran->uszAutoSettleBit == VS_TRUE)
	{
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] AutoSettel END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}

	/* 銀聯選NCCC */
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		pobTran->srBRec.inHDTIndex = 0;

		if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie("  GetHostNum LoadHDT *Error* HID[%d] Line[%d]",pobTran->srBRec.inHDTIndex, __LINE__);
			return (VS_ERROR);
		}

		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie("  GetHostNum LoadHDPT *Error* HID[%d] Line[%d]",pobTran->srBRec.inHDTIndex, __LINE__);
			return (VS_ERROR);
		}

		memset(szBatchNum, 0x00, sizeof(szBatchNum));
		inGetBatchNum(szBatchNum);
		pobTran->srBRec.lnBatchNum = atol(szBatchNum);
	}

	/* SmartPay選NCCC */
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		pobTran->srBRec.inHDTIndex = 0;

		if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie("  GetHostNum LoadHDT *Error* HID[%d] Line[%d]",pobTran->srBRec.inHDTIndex, __LINE__);
			return (VS_ERROR);
		}

		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie("  GetHostNum LoadHDPT *Error* HID[%d] Line[%d]",pobTran->srBRec.inHDTIndex, __LINE__);
			return (VS_ERROR);
		}

		memset(szBatchNum, 0x00, sizeof(szBatchNum));
		inGetBatchNum(szBatchNum);
		pobTran->srBRec.lnBatchNum = atol(szBatchNum);
	}


	/* 如果已經有HostNum，跳過選Host流程(ECR發動) */
	if (pobTran->srBRec.inHDTIndex != -1)
	{
		return (VS_SUCCESS);
	}

	/* 以上是特例，如果都沒有就手動選Host */
	memset(szLine1, 0x00, sizeof(szLine1));
	memset(szLine2, 0x00, sizeof(szLine2));
	memset(szLine3, 0x00, sizeof(szLine3));
	memset(szLine4, 0x00, sizeof(szLine4));
	memset(szTimeout, 0x00, sizeof(szTimeout));
	memset(inHostIndex, 0x00, sizeof(inHostIndex));
	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));

	for (i = 0;; ++i)
	{
		/* 先LoadHDT */
		if (inLoadHDTRec(i) == VS_ERROR)
		{
			/* 當找不到第i筆資料會回傳VS_ERROR */
			break;
		}

		/* ESC不該出現在選單上 */
		memset(szHostName, 0x00, sizeof(szHostName));
		inGetHostLabel(szHostName);
		if (memcmp(szHostName, _HOST_NAME_ESC_, strlen(_HOST_NAME_ESC_)) == 0)
		{
			continue;
		}
		/* 因為公司TMS會下CUP主機，因富邦是同批次，所以CUP要跳過 所以新增 20190214 [SAM] */
		if (memcmp(szHostName, _HOST_NAME_CUP_, strlen(_HOST_NAME_CUP_)) == 0)
		{
			continue;
		}

		/* GET HOST Enable */
		memset(szHostEnable, 0x00, sizeof(szHostEnable));
		if (inGetHostEnable(szHostEnable) == VS_ERROR)
		{
			return (VS_ERROR);
		}
		else
		{
			inDISP_LogPrintfWithFlag("  HID[%d] HostEnable: %s", i, szHostEnable);

			if (memcmp(szHostEnable, "Y", 1) != 0)
			{
				/* 如果HostEnable != Y，就continue */
				continue;
			}

			/* 如果主機有開，才loadHDPT */
			if (inLoadHDPTRec(i) == VS_ERROR)
			{
				inDISP_DispLogAndWriteFlie("  GetHostNum LoadHDT *Error* HID[%d] Line[%d]", i, __LINE__);
				/* 當找不到第i筆資料會回傳VS_ERROR */
				return (VS_ERROR);
			}

			inOpenHostCnt ++;       /* 記錄有幾個Host有開 */
			inLine ++;              /* 第幾行 */

			/* 記錄HostEnable為Y的HostIndex，減1是因為HostIndex從01開始 */
			inHostIndex[j] = i;
			j++;

			memset(szHostName, 0x00, sizeof(szHostName));
			inGetHostLabel(szHostName);

			/* 用szTRTFileName來決定要顯示的Host Name */
			memset(szTemp, 0x00, sizeof(szTemp));
			sprintf(szTemp, "%d. %s ", inOpenHostCnt, szHostName);

			/* 每一行顯示的內容先存在陣列裡 */
			switch (inLine)
			{
				case 1:
					memcpy(&szLine1[inLine1Index], szTemp, strlen(szTemp));
					inLine1Index += strlen(szTemp);
					break;
				case 2:
					memcpy(&szLine2[inLine2Index], szTemp, strlen(szTemp));
					inLine2Index += strlen(szTemp);
					break;
				case 3:
					memcpy(&szLine3[inLine3Index], szTemp, strlen(szTemp));
					inLine3Index += strlen(szTemp);
					break;
				case 4:
					memcpy(&szLine4[inLine4Index], szTemp, strlen(szTemp));
					inLine4Index += strlen(szTemp);
					break;
				default:
					break;
			}

		}

	}/* End of For loop */

	/* 當inOpenHostCnt = 0，表示主機都沒開或者inLoadHDT有問題 */
	if (inOpenHostCnt == 0)
	{
		/* 主機選擇錯誤 */
		inDISP_Msg_BMP(_ERR_CHOOSE_HOST_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		return (VS_ERROR);
	}

	if (inOpenHostCnt == 1)
	{
		/* 只有開一個Host */
		if (inLoadHDTRec(inHostIndex[0]) == VS_ERROR)
		{
			/* 主機選擇錯誤 */
			inDISP_Msg_BMP(_ERR_CHOOSE_HOST_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
			return (VS_ERROR);
		}

		if (inLoadHDPTRec(inHostIndex[0]) == VS_ERROR)
		{
			/* 主機選擇錯誤 */
			inDISP_Msg_BMP(_ERR_CHOOSE_HOST_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
			return (VS_ERROR);
		}
		else
		{
			pobTran->srBRec.inHDTIndex = inHostIndex[0];
			memset(szBatchNum, 0x00, sizeof(szBatchNum));
			inGetBatchNum(szBatchNum);
			pobTran->srBRec.lnBatchNum = atol(szBatchNum);
		}

		return (VS_SUCCESS);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 顯示請選擇中心? */
		inDISP_PutGraphic(_CHOCE_HOST_, 0, _COORDINATE_Y_LINE_8_4_);

		/*有開多個Host */
		inDISP_ChineseFont(szLine1, _FONTSIZE_12X19_, _LINE_12_6_, _DISP_LEFT_);
		inDISP_ChineseFont(szLine2, _FONTSIZE_12X19_, _LINE_12_7_, _DISP_LEFT_);
		inDISP_ChineseFont(szLine3, _FONTSIZE_12X19_, _LINE_12_8_, _DISP_LEFT_);
		inDISP_ChineseFont(szLine4, _FONTSIZE_12X19_, _LINE_12_9_, _DISP_LEFT_);

		inDISP_LogPrintfWithFlag(szLine1);
		inDISP_LogPrintfWithFlag(szLine2);
		inDISP_LogPrintfWithFlag(szLine3);
		inDISP_LogPrintfWithFlag(szLine4);
	}

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();

		/* 轉成數字判斷是否在inOpenHostCnt的範圍內 */
		inKey = 0;
		/* 有觸摸*/
		if (inChoice != _DisTouch_No_Event_)
		{
			switch (inChoice)
			{
				case _OPTTouch12X19_LINE_6_:
					inKey = 1;
					inDisTouch_Reverse_Back_Key(inTouchSensorFunc, inChoice);
					break;
				case _OPTTouch12X19_LINE_7_:
					inKey = 2;
					inDisTouch_Reverse_Back_Key(inTouchSensorFunc, inChoice);
					break;
				case _OPTTouch12X19_LINE_8_:
					inKey = 3;
					inDisTouch_Reverse_Back_Key(inTouchSensorFunc, inChoice);
					break;
				case _OPTTouch12X19_LINE_9_:
					inKey = 4;
					inDisTouch_Reverse_Back_Key(inTouchSensorFunc, inChoice);
					break;
				default:
					inKey = 0;
					break;
			}
		}
		/* 有按按鍵 */
		else if (szKey != 0)
		{
			switch (szKey)
			{
				case _KEY_1_:
					inKey = 1;
					break;
				case _KEY_2_:
					inKey = 2;
					break;
				case _KEY_3_:
					inKey = 3;
					break;
				case _KEY_4_:
					inKey = 4;
					break;
				default:
					inKey = 0;
					break;
			}
		}

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (inKey >= 1 && inKey <= inOpenHostCnt)
		{
			if (inLoadHDTRec(inHostIndex[inKey - 1]) == VS_ERROR)
			{
				/* 主機選擇錯誤 */
				inDISP_Msg_BMP(_ERR_CHOOSE_HOST_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
				inRetVal = VS_ERROR;
				break;
			}

			if (inLoadHDPTRec(inHostIndex[inKey - 1]) == VS_ERROR)
			{
				/* 主機選擇錯誤 */
				inDISP_Msg_BMP(_ERR_CHOOSE_HOST_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
				inRetVal = VS_ERROR;
				break;
			}
			else
			{
				inDISP_LogPrintfWithFlag("  HDI[%d]", inHostIndex[inKey - 1]);

				pobTran->srBRec.inHDTIndex = inHostIndex[inKey - 1];
				memset(szBatchNum, 0x00, sizeof(szBatchNum));
				inGetBatchNum(szBatchNum);
				pobTran->srBRec.lnBatchNum = atol(szBatchNum);

				inRetVal = VS_SUCCESS;
				break;
			}
                }
        }

	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}


/*
Function		: inFunc_GetHostNum_NewUI
Date&Time	: 2017/10/31 下午 5:09
Describe		: Host選單，如果只有一個主機就直接 Load HDT 和 HDPT，選定HostNum，並load HDT 和 HDPT
 * [整理功能並加註解] 此功能有在使用 2022/9/1 下午 4:08 [SAM] 
 */
int inFunc_GetHostNum_NewUI(TRANSACTION_OBJECT *pobTran)
{
	int inOpenHostCnt = 0; /* 記錄有幾個Host有開 */
	int inLine = 0; /* 第幾行 */
	int i, j = 0; /* j是inHostIndex陣列索引 */
	int inHostIndex[12 + 1]; /* 記錄HostEnable為Y的HostIndex */
	int inLine1Index = 0; /* szLine1的index */
	int inLine2Index = 0; /* szLine2的index */
	int inLine3Index = 0; /* szLine3的index */
	int inLine4Index = 0;
	int inLine5Index = 0;
	int inLine6Index = 0;
	int inKey = 0;
	int inChoice = 0;
	int inTouchSensorFunc = _Touch_NEWUI_CHOOSE_HOST_;
	int inRetVal = VS_SUCCESS;
	char szKey = 0;
	char szHostEnable[2 + 1] = {0};
	char szHostName[42 + 1] = {0};
	char szTemp[48 + 1] = {0};
	char szLine1[48 + 1] = {0}; /* 存第一行要顯示的Host */ /* linux系統中文字length一個字為3，小心爆掉 */
	char szLine2[48 + 1] = {0}; /* 存第二行要顯示的Host */
	char szLine3[48 + 1] = {0}; /* 存第三行要顯示的Host */
	char szLine4[48 + 1] = {0};
	char szLine5[48 + 1] = {0};
	char szLine6[48 + 1] = {0};
	char szTemp2[48 + 1] = {0};
	char szLine1_2[48 + 1] = {0}; /* 存第一行要顯示的Host */ /* linux系統中文字length一個字為3，小心爆掉 */
	char szLine2_2[48 + 1] = {0}; /* 存第二行要顯示的Host */
	char szLine3_2[48 + 1] = {0}; /* 存第三行要顯示的Host */
	char szLine4_2[48 + 1] = {0};
	char szLine5_2[48 + 1] = {0};
	char szLine6_2[48 + 1] = {0};
	char szBatchNum[6 + 1] = {0};
	char szTimeout[4 + 1] = {0};
	char szDebugMsg[42 + 1] = {0};
	DISPLAY_OBJECT srDispObj;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	/* 如果要連動結帳，跳過選Host流程 */
	if (pobTran->uszAutoSettleBit == VS_TRUE)
	{
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] AutoSettle END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}

	/* 銀聯卡別歸類在主要使用的主機內 */
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		pobTran->srBRec.inHDTIndex = 0;

		if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" FUNC GetHostNum_New CUP Flag=TRUE HDT[%d] *Error* Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
			return (VS_ERROR);
		}

		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" FUNC GetHostNum_New CUP Flag=TRUE HDPT[%d] *Error* Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
			return (VS_ERROR);
		}

		memset(szBatchNum, 0x00, sizeof (szBatchNum));
		inGetBatchNum(szBatchNum);
		pobTran->srBRec.lnBatchNum = atol(szBatchNum);
	}

	/* SmartPay 目前沒在用，先暫時註解掉程式碼 START 2022/9/1 下午 4:16 [SAM] */
//	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
//	{
//		pobTran->srBRec.inHDTIndex = 0;
//
//		if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
//		{
//			inDISP_DispLogAndWriteFlie(" FUNC GetHostNum_New FISC Flag=TRUE HDPT[%d] *Error* Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
//			return (VS_ERROR);
//		}
//
//		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
//		{
//			inDISP_DispLogAndWriteFlie(" FUNC GetHostNum_New FISC Flag=TRUE HDPT[%d] *Error* Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
//			return (VS_ERROR);
//		}
//
//		memset(szBatchNum, 0x00, sizeof (szBatchNum));
//		inGetBatchNum(szBatchNum);
//		pobTran->srBRec.lnBatchNum = atol(szBatchNum);
//	}
	/* SmartPay 目前沒在用，先暫時註解掉程式碼 END 2022/9/1 下午 4:16 [SAM] */

	/* 如果已經有HostNum，跳過選Host流程(ECR發動) */
	if (pobTran->srBRec.inHDTIndex != -1)
	{
		return (VS_SUCCESS);
	}

	/* 以上是特例，如果都沒有就手動選Host */

	memset(szLine1, 0x00, sizeof (szLine1));
	memset(szLine2, 0x00, sizeof (szLine2));
	memset(szLine3, 0x00, sizeof (szLine3));
	memset(szLine4, 0x00, sizeof (szLine4));
	memset(szLine5, 0x00, sizeof (szLine5));
	memset(szLine6, 0x00, sizeof (szLine6));
	memset(szLine1_2, 0x00, sizeof (szLine1_2));
	memset(szLine2_2, 0x00, sizeof (szLine2_2));
	memset(szLine3_2, 0x00, sizeof (szLine3_2));
	memset(szLine4_2, 0x00, sizeof (szLine4_2));
	memset(szLine5_2, 0x00, sizeof (szLine5_2));
	memset(szLine6_2, 0x00, sizeof (szLine6_2));
	memset(szTimeout, 0x00, sizeof (szTimeout));
	memset(inHostIndex, 0x00, sizeof (inHostIndex));
	memset(&srDispObj, 0x00, sizeof (DISPLAY_OBJECT));

	for (i = 0;; ++i)
	{
		/* 先LoadHDT */
		if (inLoadHDTRec(i) == VS_ERROR)
		{
			/* 當找不到第i筆資料會回傳VS_ERROR */
			inDISP_DispLogAndWriteFlie(" FUNC GetHostNum_New Can't Find Mach HDT[%d] *Error* Line[%d]",i ,__LINE__);
			break;
		}

		/* 因為公司TMS會下CUP主機，因富邦是同批次，所以CUP要跳過 所以新增 20190214 [SAM] */
		memset(szHostName, 0x00, sizeof (szHostName));
		inGetHostLabel(szHostName);
		if (memcmp(szHostName, _HOST_NAME_CUP_, strlen(_HOST_NAME_CUP_)) == 0)
		{
			continue;
		}

		/* ESC不該出現在選單上 */
		if (memcmp(szHostName, _HOST_NAME_ESC_, strlen(_HOST_NAME_ESC_)) == 0)
		{
			continue;
		}

		/* ESVC不該出現在一般交易選單上`，目前只有結帳需要，採正向表列 */
		if (memcmp(szHostName, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)) == 0)
		{
			if (pobTran->inRunOperationID != _OPERATION_SETTLE_ &&
					pobTran->inRunOperationID != _OPERATION_DELETE_BATCH_ &&
					pobTran->inRunOperationID != _OPERATION_REVIEW_ &&
					pobTran->inRunOperationID != _OPERATION_REVIEW_TOTAL_ &&
					pobTran->inRunOperationID != _OPERATION_REVIEW_DETAIL_ &&
					pobTran->inRunOperationID != _OPERATION_TOTAL_REPORT_ &&
					pobTran->inRunOperationID != _OPERATION_DETAIL_REPORT_ &&
					pobTran->inRunOperationID != _OPERATION_REPRINT_)
			{
				continue;
			}
		}

		/* [新增電票悠遊卡功能]  新增電票重印功能 [SAM] 2022/6/22 上午 10:35 */
		if ((memcmp(szHostName, _HOST_NAME_CMAS_, strlen(_HOST_NAME_CMAS_)) == 0))
		{
			if (pobTran->inRunOperationID != _OPERATION_SETTLE_ &&
					//pobTran->inRunOperationID != _OPERATION_DELETE_BATCH_	&&
					pobTran->inRunOperationID != _OPERATION_REVIEW_ &&
					pobTran->inRunOperationID != _OPERATION_REVIEW_TOTAL_ &&
					pobTran->inRunOperationID != _OPERATION_REVIEW_DETAIL_ &&
					pobTran->inRunOperationID != _OPERATION_TOTAL_REPORT_ &&
					pobTran->inRunOperationID != _OPERATION_DETAIL_REPORT_ &&
					pobTran->inRunOperationID != _OPERATION_REPRINT_)
			{
				continue;
			}
		}

		/* GET HOST Enable */
		memset(szHostEnable, 0x00, sizeof (szHostEnable));
		if (inGetHostEnable(szHostEnable) == VS_ERROR)
		{
			return (VS_ERROR);
		} else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%d HostEnable: %s", i, szHostEnable);
				inDISP_LogPrintf(szDebugMsg);
			}

			if (memcmp(szHostEnable, "Y", 1) != 0)
			{
				/* 如果HostEnable != Y，就continue */
				continue;
			}

			/* 如果主機有開，才loadHDPT */
			if (inLoadHDPTRec(i) == VS_ERROR)
			{
				/* 當找不到第i筆資料會回傳VS_ERROR */
				return (VS_ERROR);
			}

			inOpenHostCnt++; /* 記錄有幾個Host有開 */
			inLine++; /* 第幾行 */

			/* 記錄HostEnable為Y的HostIndex，減1是因為HostIndex從01開始 */
			inHostIndex[j] = i;
			j++;

			memset(szHostName, 0x00, sizeof (szHostName));
			inGetHostLabel(szHostName);

			/* 用szTRTFileName來決定要顯示的Host Name */
			memset(szTemp, 0x00, sizeof (szTemp));
			inFunc_DiscardSpace(szHostName);
			/* ESVC要顯示電子票證 */
			 if (memcmp(szHostName, _HOST_NAME_CMAS_, strlen(_HOST_NAME_CMAS_)) == 0)
			{ /* [新增電票悠遊卡功能] 新增重印功能，修改提示選項 [SAM] 2022/6/22 上午 10:36 */
				sprintf(szTemp, "%s", "悠遊卡");
				memset(szTemp2, 0x00, sizeof (szTemp2));
				sprintf(szTemp2, "     %d", inOpenHostCnt);
			} else
			{
				sprintf(szTemp, "%s", szHostName);
				memset(szTemp2, 0x00, sizeof (szTemp2));
				sprintf(szTemp2, "     %d", inOpenHostCnt);
			}

			/* 每一行顯示的內容先存在陣列裡 */
			switch (inLine)
			{
				case 1:
					memcpy(&szLine1[inLine1Index], szTemp, strlen(szTemp));
					inLine1Index += strlen(szTemp);

					memcpy(szLine1_2, szTemp2, strlen(szTemp2));
					break;
				case 2:
					memcpy(&szLine2[inLine2Index], szTemp, strlen(szTemp));
					inLine2Index += strlen(szTemp);

					memcpy(szLine2_2, szTemp2, strlen(szTemp2));
					break;
				case 3:
					memcpy(&szLine3[inLine3Index], szTemp, strlen(szTemp));
					inLine3Index += strlen(szTemp);

					memcpy(szLine3_2, szTemp2, strlen(szTemp2));
					break;
				case 4:
					memcpy(&szLine4[inLine4Index], szTemp, strlen(szTemp));
					inLine4Index += strlen(szTemp);

					memcpy(szLine4_2, szTemp2, strlen(szTemp2));
					break;
				case 5:
					memcpy(&szLine5[inLine5Index], szTemp, strlen(szTemp));
					inLine5Index += strlen(szTemp);

					memcpy(szLine5_2, szTemp2, strlen(szTemp2));
					break;
				case 6:
					memcpy(&szLine6[inLine6Index], szTemp, strlen(szTemp));
					inLine6Index += strlen(szTemp);

					memcpy(szLine6_2, szTemp2, strlen(szTemp2));
					break;
				default:
					break;
			}
		}

	}/* End of For loop */

	/* 當inOpenHostCnt = 0，表示主機都沒開或者inLoadHDT有問題 */
	if (inOpenHostCnt == 0)
	{
		/* 主機選擇錯誤 */
		inDISP_Msg_BMP(_ERR_CHOOSE_HOST_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		return (VS_ERROR);
	}

	if (inOpenHostCnt == 1)
	{
		/* 只有開一個Host */
		if (inLoadHDTRec(inHostIndex[0]) == VS_ERROR)
		{
			/* 主機選擇錯誤 */
			inDISP_Msg_BMP(_ERR_CHOOSE_HOST_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
			return (VS_ERROR);
		}

		if (inLoadHDPTRec(inHostIndex[0]) == VS_ERROR)
		{
			/* 主機選擇錯誤 */
			inDISP_Msg_BMP(_ERR_CHOOSE_HOST_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
			return (VS_ERROR);
		} else
		{
			pobTran->srBRec.inHDTIndex = inHostIndex[0];
			memset(szBatchNum, 0x00, sizeof (szBatchNum));
			inGetBatchNum(szBatchNum);
			pobTran->srBRec.lnBatchNum = atol(szBatchNum);
		}

		inRetVal = VS_SUCCESS;
	} else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

		switch (inOpenHostCnt)
		{
			case 2:
				inDISP_PutGraphic(_CHOOSE_HOST_2_, 0, _COORDINATE_Y_LINE_8_4_);
				break;

			case 3:
				inDISP_PutGraphic(_CHOOSE_HOST_3_, 0, _COORDINATE_Y_LINE_8_4_);
				break;

			case 4:
				inDISP_PutGraphic(_CHOOSE_HOST_4_, 0, _COORDINATE_Y_LINE_8_4_);
				break;

			case 5:
				inDISP_PutGraphic(_CHOOSE_HOST_5_, 0, _COORDINATE_Y_LINE_8_4_);
				break;

			case 6:
				inDISP_PutGraphic(_CHOOSE_HOST_6_, 0, _COORDINATE_Y_LINE_8_4_);
				break;

			default:
				inDISP_PutGraphic(_CHOOSE_HOST_6_, 0, _COORDINATE_Y_LINE_8_4_);
				break;
		}

		/*有開多個Host */
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine1, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine1_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);

		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine2_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);

		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine3, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine3_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);

		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine4, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_13_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine4_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_14_, VS_FALSE);

		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine5, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_13_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine5_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_14_, VS_FALSE);

		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine6, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_16_13_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine6_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_16_14_, VS_FALSE);

		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf(szLine1);
			inDISP_LogPrintf(szLine2);
			inDISP_LogPrintf(szLine3);
			inDISP_LogPrintf(szLine4);
			inDISP_LogPrintf(szLine5);
			inDISP_LogPrintf(szLine6);
		}

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* 轉成數字判斷是否在inOpenHostCnt的範圍內 */
			inKey = 0;
			/* 有觸摸*/
			if (inChoice != _DisTouch_No_Event_)
			{
				switch (inChoice)
				{
					case _NEWUI_CHOOSE_HOST_Touch_HOST_1_:
						inKey = 1;
						break;
					case _NEWUI_CHOOSE_HOST_Touch_HOST_2_:
						inKey = 2;
						break;
					case _NEWUI_CHOOSE_HOST_Touch_HOST_3_:
						inKey = 3;
						break;
					case _NEWUI_CHOOSE_HOST_Touch_HOST_4_:
						inKey = 4;
						break;
					case _NEWUI_CHOOSE_HOST_Touch_HOST_5_:
						inKey = 5;
						break;
					case _NEWUI_CHOOSE_HOST_Touch_HOST_6_:
						inKey = 6;
						break;
					default:
						inKey = 0;
						break;
				}
			}				/* 有按按鍵 */
			else if (szKey != 0)
			{
				switch (szKey)
				{
					case _KEY_1_:
						inKey = 1;
						break;
					case _KEY_2_:
						inKey = 2;
						break;
					case _KEY_3_:
						inKey = 3;
						break;
					case _KEY_4_:
						inKey = 4;
						break;
					case _KEY_5_:
						inKey = 5;
						break;
					case _KEY_6_:
						inKey = 6;
						break;
					default:
						inKey = 0;
						break;
				}
			}

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				break;
			} else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				break;
			} else if (inKey >= 1 && inKey <= inOpenHostCnt)
			{
				if (inLoadHDTRec(inHostIndex[inKey - 1]) == VS_ERROR)
				{
					/* 主機選擇錯誤 */
					inDISP_Msg_BMP(_ERR_CHOOSE_HOST_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
					inRetVal = VS_ERROR;
					break;
				}

				if (inLoadHDPTRec(inHostIndex[inKey - 1]) == VS_ERROR)
				{
					/* 主機選擇錯誤 */
					inDISP_Msg_BMP(_ERR_CHOOSE_HOST_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

					inRetVal = VS_ERROR;
					break;
				} else
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szTemp, 0x00, sizeof (szTemp));
						sprintf(szTemp, "Host %d", inHostIndex[inKey - 1]);
						inDISP_LogPrintf(szTemp);
					}
					pobTran->srBRec.inHDTIndex = inHostIndex[inKey - 1];
					memset(szBatchNum, 0x00, sizeof (szBatchNum));
					inGetBatchNum(szBatchNum);
					pobTran->srBRec.lnBatchNum = atol(szBatchNum);

					/* 票證 */
					memset(szHostName, 0x00, sizeof (szHostName));
					inGetHostLabel(szHostName);

					/* [新增電票悠遊卡功能] 新增電票重印條件, [SAM] 2022/6/22 上午 10:40 */
					if (memcmp(szHostName, _HOST_NAME_CMAS_, strlen(_HOST_NAME_CMAS_)) == 0)
					{

						if (pobTran->inRunOperationID == _OPERATION_SETTLE_)
						{
							inDISP_LogPrintfWithFlag(" CMAS Run _OPERATION_SETTLE_ In Get Host Line[%d] ", __LINE__);
/* IPASS 先留著 */
//							if ((memcmp(szHostName, _HOST_NAME_IPASS_, strlen(_HOST_NAME_IPASS_)) == 0))
//							{
//								pobTran->inRunTRTID = _TRT_IPASS_SETTLE_;
//								pobTran->srTRec.inTicketType = _TICKET_TYPE_IPASS_;
//								pobTran->srTRec.inCode = _SETTLE_;
//							}
//							/* 20200417 added  start[Hachi]*/
//						 	else 
							if ((memcmp(szHostName, _HOST_NAME_CMAS_, strlen(_HOST_NAME_CMAS_)) == 0))
							{
								pobTran->inRunTRTID = _TRT_CMAS_SETTLE_;
								pobTran->srTRec.inTicketType = _TICKET_TYPE_ECC_;
								pobTran->srTRec.inCode = _SETTLE_;
							} else
							{
								inDISP_DispLogAndWriteFlie(" CMAS Settle Name [%s] *Error* Line[%d] ", szHostName, __LINE__);
								return (VS_ERROR);
							}
						}							/* 20200417 added  start[Hachi]*/
						else
							if (pobTran->inRunOperationID == _OPERATION_REVIEW_ ||
								pobTran->inRunOperationID == _OPERATION_REVIEW_DETAIL_ || /* [新增電票悠遊卡功能] 因有使用 _OPERATION_REVIEW_DETAIL_  [SAM] 2022/6/22 下午 12:09 */
								pobTran->inRunOperationID == _OPERATION_REVIEW_TOTAL_ || /* [新增電票悠遊卡功能] 因有使用 _OPERATION_REVIEW_TOTAL_  [SAM] 2022/6/22 下午 12:09 */
								pobTran->inRunOperationID == _OPERATION_REPRINT_ ||
								pobTran->inRunOperationID == _OPERATION_TOTAL_REPORT_ || /*20200420 new*/
								pobTran->inRunOperationID == _OPERATION_DETAIL_REPORT_) /*20200420 new*/
						{
							if ((memcmp(szHostName, _HOST_NAME_IPASS_, strlen(_HOST_NAME_IPASS_)) == 0))
								pobTran->srTRec.inTicketType = _TICKET_TYPE_IPASS_;
							else if ((memcmp(szHostName, _HOST_NAME_CMAS_, strlen(_HOST_NAME_CMAS_)) == 0))
								pobTran->srTRec.inTicketType = _TICKET_TYPE_ECC_;
							inDISP_LogPrintfWithFlag(" CMAS Get Host TicketType[%d] Line[%d] ", pobTran->srTRec.inTicketType, __LINE__);
						}
						pobTran->srTRec.uszESVCTransBit = VS_TRUE;
						inDISP_LogPrintfWithFlag(" CMAS Get Host by Invoice Num BatchNum[%d] Line[%d] ", pobTran->srBRec.lnBatchNum, __LINE__);
					}

					inRetVal = VS_SUCCESS;
					break;
				}
			}
		}

		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}


/*
Function        :inFunc_All_Host_Must_Settle
Date&Time       :2016/10/4 上午 11:59
Describe        :確認全部Host是否要先結帳，要下TMS前檢查用
*/
int inFunc_All_Host_Must_Settle_Check(TRANSACTION_OBJECT *pobTran)
{
	int		i = 0;
        char		szMustSettleBit[2 + 1] = {};
	char		szHostEnable[2 + 1] = {};
	char		szHostName[40 + 1] = {};
	unsigned char	uszFileName[15 + 1] = {};

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	for (i = 0 ;; i ++)
	{
		if (inLoadHDTRec(i) < 0)	/* 主機參數檔【HostDef.txt】 */
			break;

		pobTran->srBRec.inHDTIndex = i;
		memset(szHostEnable, 0x00, sizeof(szHostEnable));
		inGetHostEnable(szHostEnable);
		if (!memcmp(szHostEnable, "N", 1))
		{
			/* 基本上只有有開的Host才會有是否要結帳的問題，不過為了預防萬一，多檢查該Host是否有開 */
			continue;
		}

		memset(szHostName, 0x00, sizeof(szHostName));
		inGetHostLabel(szHostName);
		if (memcmp(szHostName, _HOST_NAME_ESC_, strlen(_HOST_NAME_ESC_)) == 0)
		{
			continue;
		}
		
		/* 因為公司TMS會下CUP主機，因富邦是同批次，所以CUP要跳過 所以新增 20190214 [SAM] */
		if (memcmp(szHostName, _HOST_NAME_CUP_, strlen(_HOST_NAME_CUP_)) == 0)
		{
			continue;
		}

		inLoadHDPTRec(i);
		memset(szMustSettleBit, 0x00, sizeof(szMustSettleBit));
		inGetMustSettleBit(szMustSettleBit);

		if (!memcmp(szMustSettleBit, "Y", 1))
		{
			/* 開機詢問檢查時不需要顯示錯誤訊息 2019/12/20 上午 10:35 [SAM] */
			if(pobTran->inRunOperationID != _OPERATION_EDC_BOOTING_)
			{
				/* 因為自動TMS詢問主畫面的圖會包含到TITLE的部份，所以要再清一次 2019/12/19 下午 12:00 [SAM] */
				if(pobTran->inRunOperationID == _OPERATION_TMS_SCHEDULE_DOWNLOAD_)
				{	
					inDISP_ClearAll();
					inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);
				}
				/* 表示要結帳 */
				inFunc_DiscardSpace(szHostName);
				inDISP_Msg_BMP(_ERR_MUST_SETTLE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, szHostName, _LINE_8_5_);
			}
			return (VS_ERROR);
		}

		 /* Check REVERSAL */
		memset(uszFileName, 0x00, sizeof(uszFileName));
		if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
		{
			/* 開機詢問檢查時不需要顯示錯誤訊息 2019/12/20 上午 10:35 [SAM] */
			if(pobTran->inRunOperationID != _OPERATION_EDC_BOOTING_)
			{
				/* 因為自動TMS詢問主畫面的圖會包含到TITLE的部份，所以要再清一次 2019/12/19 下午 12:00 [SAM] */
				if(pobTran->inRunOperationID == _OPERATION_TMS_SCHEDULE_DOWNLOAD_)
				{
					inDISP_ClearAll();
					inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					
				}
				/* 表示要結帳 */
				memset(szHostName, 0x00, sizeof(szHostName));
				inGetHostLabel(szHostName);
				inFunc_DiscardSpace(szHostName);
				inDISP_Msg_BMP(_ERR_MUST_SETTLE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, szHostName, _LINE_8_5_);
			}
			return (VS_ERROR);
		}

		/* Check ADVICE */
		memset(uszFileName, 0x00, sizeof(uszFileName));
		if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
		{
			/* 開機詢問檢查時不需要顯示錯誤訊息 2019/12/20 上午 10:35 [SAM] */
			if(pobTran->inRunOperationID != _OPERATION_EDC_BOOTING_)
			{
				/* 因為自動TMS詢問主畫面的圖會包含到TITLE的部份，所以要再清一次 2019/12/19 下午 12:00 [SAM] */
				if(pobTran->inRunOperationID == _OPERATION_TMS_SCHEDULE_DOWNLOAD_)
				{
					inDISP_ClearAll();
					inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);
				}
				/* 表示要結帳 */
				memset(szHostName, 0x00, sizeof(szHostName));
				inGetHostLabel(szHostName);
				inFunc_DiscardSpace(szHostName);
				inDISP_Msg_BMP(_ERR_MUST_SETTLE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, szHostName, _LINE_8_5_);
			}
			return (VS_ERROR);
		}

		/* Check ADVICE ESC */
		memset(uszFileName, 0x00, sizeof(uszFileName));
		if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _ADVICE_ESC_FILE_EXTENSION_, 6) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
		{
			/* 開機詢問檢查時不需要顯示錯誤訊息 2019/12/20 上午 10:35 [SAM] */
			if(pobTran->inRunOperationID != _OPERATION_EDC_BOOTING_)
			{
				/* 因為自動TMS詢問主畫面的圖會包含到TITLE的部份，所以要再清一次 2019/12/19 下午 12:00 [SAM] */
				if(pobTran->inRunOperationID == _OPERATION_TMS_SCHEDULE_DOWNLOAD_)
				{
					inDISP_ClearAll();
					inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);
				}
				/* 表示要結帳 */
				memset(szHostName, 0x00, sizeof(szHostName));
				inGetHostLabel(szHostName);
				inFunc_DiscardSpace(szHostName);
				inDISP_Msg_BMP(_ERR_MUST_SETTLE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, szHostName, _LINE_8_5_);
			}
			return (VS_ERROR);
		}

		/* Check Table Exist */
		if (inSqlite_Check_Table_Exist_Flow(pobTran, _TN_BATCH_TABLE_) == VS_SUCCESS)
		{
			/* 開機詢問檢查時不需要顯示錯誤訊息 2019/12/20 上午 10:35 [SAM] */
			if(pobTran->inRunOperationID != _OPERATION_EDC_BOOTING_)
			{
				/* 因為自動TMS詢問主畫面的圖會包含到TITLE的部份，所以要再清一次 2019/12/19 下午 12:00 [SAM] */
				if(pobTran->inRunOperationID == _OPERATION_TMS_SCHEDULE_DOWNLOAD_)
				{
					inDISP_ClearAll();
					inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);
				}
				/* 表示要結帳 */
				memset(szHostName, 0x00, sizeof(szHostName));
				inGetHostLabel(szHostName);
				inFunc_DiscardSpace(szHostName);
				inDISP_Msg_BMP(_ERR_MUST_SETTLE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, szHostName, _LINE_8_5_);
			}
			return (VS_ERROR);
		}

	}
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Display_Error
Date&Time       :2017/9/20 下午 12:00
Describe        :為了只顯示一次錯誤訊息
*/
int inFunc_Display_Error(TRANSACTION_OBJECT *pobTran)
{
	char	szTemplate[50 + 1];

	/* 沒有設定Error code 跳過 */
	if (pobTran->inErrorMsg == 0x00)
	{
		return (VS_SUCCESS);
	}

	inDISP_LogPrintfWithFlag(" Func Disp inErrorMsg : %d", pobTran->inErrorMsg);

	/* 統一在這裡顯示錯誤訊息 */
	/* 結帳出錯一律顯示結帳失敗 */
	if (pobTran->inRunTRTID == _TRT_SETTLE_ ||
	    pobTran->inErrorMsg == _ERROR_CODE_V3_SETTLE_NOT_SUCCESS_)
	{
		/* 連動結帳時，不顯示請按清除鍵 */
		if (pobTran->uszAutoSettleBit == VS_TRUE)
		{
			inDISP_LogPrintfWithFlag(" Func Disp Error Msg in Auto Settle");
			inDISP_Msg_BMP(_ERR_SETTLE_FAILED_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 2, "", 0);
		}
		else
		{
			inDISP_LogPrintfWithFlag(" Func Disp Error Msg in Single Settle");
			inDISP_Msg_BMP(_ERR_SETTLE_FAILED_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}

	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_USER_CANCEL_)
	{
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_USER_TERMINATE_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 2, "", 0);
		}
		else
		{
			inDISP_Msg_BMP(_ERR_USER_TERMINATE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_COMM_)
	{
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_CONNECT_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 2, "", 0);			/* 通訊失敗 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_CONNECT_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_ISO_PACK_	||
		 pobTran->inErrorMsg == _ERROR_CODE_V3_ISO_UNPACK_)
	{
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_ISO_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 2, "", 0);			/* 電文錯誤 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_ISO_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);				/* 電文錯誤 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_01_READ_FAIL_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&&
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
//			inFISC_CTLS_LED_TONE(VS_ERROR);
		}

		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_READ_CARD_, _COORDINATE_Y_LINE_8_6_, _NO_KEY_MSG_, 2, "", 0);			/* 讀卡失敗 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_READ_CARD_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);			/* 讀卡失敗 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_02_6982_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&&
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
//			inFISC_CTLS_LED_TONE(VS_ERROR);
		}

		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_CARD_FAIL_, _COORDINATE_Y_LINE_8_6_, _NO_KEY_MSG_, 2, "", 0);			/* 卡片失效 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_CARD_FAIL_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);			/* 卡片失效 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_06_TMS_NOT_SUPPORT_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&&
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
//			inFISC_CTLS_LED_TONE(VS_ERROR);
		}

		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_NOT_SUP_SMARTPAY_CTLS_CARD_, _COORDINATE_Y_LINE_8_5_, _NO_KEY_MSG_, 2, "", 0);		/* 不接受此感應卡 請改插卡 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_NOT_SUP_SMARTPAY_CTLS_CARD_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);		/* 不接受此感應卡 請改插卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_07_AMT_OVERLIMIT_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&&
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
//			inFISC_CTLS_LED_TONE(VS_ERROR);
		}

		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_OVER_SMARTPAY_CTLS_LIMIT_, _COORDINATE_Y_LINE_8_5_, _NO_KEY_MSG_, 2, "", 0);	/* 超過感應限額 請改插卡 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_OVER_SMARTPAY_CTLS_LIMIT_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);	/* 超過感應限額 請改插卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_09_NOT_RIGHT_INCODE_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&&
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
//			inFISC_CTLS_LED_TONE(VS_ERROR);
		}

		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_PLEASE_FOLLOW_RIGHT_OPT_, _COORDINATE_Y_LINE_8_5_, _NO_KEY_MSG_, 2, "", 0);		/* 請依正確卡別操作 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_PLEASE_FOLLOW_RIGHT_OPT_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);	/* 請依正確卡別操作 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_03_TIME_ERROR_	||
		 pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_04_MAC_TAC_		||
		 pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_05_NO_INCODE_	||
		 pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_08_NO_CARD_BIN_	||
		 pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_10_LOGON_FAIL_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&&
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
//			inFISC_CTLS_LED_TONE(VS_ERROR);
		}

		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_CTLS_RECEIVE_, _COORDINATE_Y_LINE_8_5_, _NO_KEY_MSG_, 2, "", 0);			/* 感應失敗 請改插卡或刷卡 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_CTLS_RECEIVE_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);		/* 感應失敗 請改插卡或刷卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_11_FISC_FALLBACK_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&&
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
//			inFISC_CTLS_LED_TONE(VS_ERROR);
		}

		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP("", 0, _NO_KEY_MSG_, 2, "請改插金融卡", _LINE_8_6_);			/* 請改插金融卡 */
		}
		else
		{
			inDISP_Msg_BMP("", 0, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "請改插金融卡", _LINE_8_6_);			/* 請改插金融卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_12_SEND_APDU_FAIL_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&&
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
//			inFISC_CTLS_LED_TONE(VS_ERROR);
		}

		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_CTLS_RECEIVE_, _COORDINATE_Y_LINE_8_5_, _NO_KEY_MSG_, 2, "", 0);			/* 感應失敗 請改插卡或刷卡 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_CTLS_RECEIVE_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);		/* 感應失敗 請改插卡或刷卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_MULTI_FUNC_CTLS_)
	{
		/* 感應失敗 請改插卡或刷卡 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_CTLS_RECEIVE_, _COORDINATE_Y_LINE_8_5_, _NO_KEY_MSG_, 2, "", 0);			/* 感應失敗 請改插卡或刷卡 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_CTLS_RECEIVE_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);		/* 感應失敗 請改插卡或刷卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_CTLS_REAL_)
	{
		/* 亮紅燈 */
//		inFISC_CTLS_LED_TONE(VS_ERROR);
		/* 不接受此感應卡 請改刷卡或插卡 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_NOT_SUP_CTLS_CARD_, _COORDINATE_Y_LINE_8_5_, _NO_KEY_MSG_, 2, "", 0);		/* 不接受此感應卡 請改刷卡或插卡 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_NOT_SUP_CTLS_CARD_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);		/* 不接受此感應卡 請改刷卡或插卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_OVER_AMOUNT_)
	{
		/* 亮紅燈 */
//		inFISC_CTLS_LED_TONE(VS_ERROR);
		/* 超過感應限額 請改插卡或刷卡 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_OVER_SMARTPAY_CTLS_LIMIT_, _COORDINATE_Y_LINE_8_5_, _NO_KEY_MSG_, 2, "", 0);		/* 超過感應限額 請改插卡 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_OVER_SMARTPAY_CTLS_LIMIT_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);		/* 超過感應限額 請改插卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_TIP_TOO_BIG_)
	{
		/* 亮紅燈 */
//		inFISC_CTLS_LED_TONE(VS_ERROR);
		/* 金額小於手續費 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_1_, _BEEP_1TIMES_MSG_, 2, "金額小於手續費", _LINE_8_6_);
		}
		else
		{
			inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_1_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "金額小於手續費", _LINE_8_6_);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_WAVE_ERROR_)
	{
		/* 感應失敗 請改插卡或刷卡 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_CTLS_RECEIVE_, _COORDINATE_Y_LINE_8_5_, _BEEP_1TIMES_MSG_, 2, "", 0);
		}
		else
		{
			inDISP_Msg_BMP(_ERR_CTLS_RECEIVE_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_WAVE_ERROR_NOT_ONE_CARD_)
	{
		/* 感應失敗 超過一張卡 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_CTLS_NOT_ONE_CARD_, _COORDINATE_Y_LINE_8_5_, _BEEP_1TIMES_MSG_, 2, "", 0);
		}
		else
		{
			inDISP_Msg_BMP(_ERR_CTLS_NOT_ONE_CARD_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC)
	{
		/* 亮紅燈 */
//		inFISC_CTLS_LED_TONE(VS_ERROR);
		/* 不接受此感應卡 請改刷卡或插卡 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_NOT_SUP_CTLS_CARD_, _COORDINATE_Y_LINE_8_5_, _NO_KEY_MSG_, 2, "", 0);		/* 不接受此感應卡 請改刷卡或插卡 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_NOT_SUP_CTLS_CARD_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);		/* 不接受此感應卡 請改刷卡或插卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FUNC_CLOSE_)
	{
		/* 此功能已關閉 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 2, "", 0);
		}
		else
		{
			inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_CTLS_DATA_SHORT_)
	{
		/* 感應資料不足 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_CTLS_DATA_SHORT_, _COORDINATE_Y_LINE_8_5_, _BEEP_1TIMES_MSG_, 2, "", 0);
		}
		else
		{
			inDISP_Msg_BMP(_ERR_CTLS_DATA_SHORT_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_WAVE_ERROR_Z1_)
	{
		/* 拒絕交易 Z1 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_USE_MS_OR_ICC_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 2, "拒絕交易 Z1", _LINE_8_5_);
		}
		else
		{
			inDISP_Msg_BMP(_ERR_USE_MS_OR_ICC_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "拒絕交易 Z1", _LINE_8_5_);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_TICKET_AMOUNT_TOO_MUCH_IN_ONE_TRANSACTION_)
	{
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_4_, _BEEP_3TIMES_MSG_, 2, "金額超過單筆上限", _LINE_8_5_);
		}
		else
		{
			inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_4_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "金額超過單筆上限", _LINE_8_5_);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_TICKET_AMOUNT_NOT_ENOUGH_)
	{
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_4_, _BEEP_3TIMES_MSG_, 2, "餘額不足", _LINE_8_5_);
		}
		else
		{
			inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_4_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "餘額不足", _LINE_8_5_);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_TRT_NOT_FOUND_)
	{
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_4_, _BEEP_3TIMES_MSG_, 2, "找不到TRT", _LINE_8_5_);
		}
		else
		{
			inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_4_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "找不到TRT", _LINE_8_5_);
		}
	}
	/* 手續費金額有誤 */
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_ECR_INST_FEE_NOT_0_)
	{
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_INSTFEE_NOT_0_, _COORDINATE_Y_LINE_8_4_, _BEEP_3TIMES_MSG_, 2, "", 0);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{

			inDISP_Msg_BMP(_ERR_INSTFEE_NOT_0_, _COORDINATE_Y_LINE_8_4_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}
		else
		{
			inDISP_Msg_BMP(_ERR_INSTFEE_NOT_0_, _COORDINATE_Y_LINE_8_4_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}
	}
	/* 晶片卡被取出 */
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_EMV_CARD_OUT_)
	{
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_CARD_REMOVED_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 2, "", 0);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_CARD_REMOVED_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 1, "", 0);
		}
		else
		{
			inDISP_Msg_BMP(_ERR_CARD_REMOVED_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 1, "", 0);
		}
	}
	/* 請改刷磁條 */
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_EMV_FALLBACK_)
	{
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_USE_MS_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 2, "", 0);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_USE_MS_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 1, "", 0);
		}
		else
		{
			inDISP_Msg_BMP(_ERR_USE_MS_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 1, "", 0);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_RECV_)
	{
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_RECV_FAILED_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 2, "", 0);			/* 通訊失敗 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_RECV_FAILED_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}
	}
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "未定義錯誤 : %d", pobTran->inErrorMsg);

		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_4_, _BEEP_1TIMES_MSG_, 2, "未定義錯誤", 0);
		}
		else
		{
			inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_4_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "未定義錯誤", 0);
		}
	}

	/* 為了強調Timeout時間 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	/* 顯示完就清空(為了只顯示一次) */
	pobTran->inErrorMsg = 0x00;

	return (VS_SUCCESS);
}

/*
Function        : inFunc_IdleCheckALL_DateAndTime
Date&Time   : 2016/10/21 下午 1:47
Describe        : 在 Idle下Check時間觸發的事件,進入func檢查排程時間，
 *		若時間合法(沒過期)，將日期（沒有的部份要補0）傳出，若比szEarlyDateTime還要早，
 *		則替換，如此可得到最接近的排程
*/
int inFunc_IdleCheckALL_DateAndTime(int *inEvent)
{
	char			szTemplate[16 + 1] = {0};
#if 0		
	char			szDCCDownloadMode[2 + 1] = {0};
	char			szESCMode[2 + 1] = {0};
	char			szCloseBatchBit[2 + 1] = {0};
	unsigned char	uszUpdateBit = VS_FALSE;
	TRANSACTION_OBJECT	pobTran;
	memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
#endif

	/* 預設為_NONE_EVENT_ */
	*inEvent = _NONE_EVENT_;
	/* TMS參數詢問 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTMSOK(szTemplate);

	/* TMSOK 表示已下載過TMS */
	if (!memcmp(&szTemplate[0], "Y", 1))
	{
		/* 公司TMS在IDLE時不需要再次詢問Sysconfig, 因為內容不會自動重置
		 *  所以這邊只會有下載參數的行為 2019/12/10 下午 4:24 [SAM]
		 */
		if(inEDCTMS_SCHEDULE_CheckInquireDateTime() == VS_SUCCESS)
		{
			*inEvent = _TMS_SCHEDULE_DOWNLOAD_EVENT_;
		}
	}
	
#if 0	
	/* TMS參數詢問 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTMSOK(szTemplate);

	/* TMSOK 表示已下載過TMS */
	if (!memcmp(&szTemplate[0], "Y", 1))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSInquireMode(szTemplate);
		/* TMS Inquire Mode = 2 由TMS安排時間自動詢問 */
		if (!memcmp(&szTemplate[0], _TMS_INQUIRE_02_SCHEDHULE_SETTLE_, 1))
		{
			/* 參數詢問 */
			if (inEDCTMS_SCHDULE_CheckEffectiveDateTime() == VS_SUCCESS)
			{
				*inEvent = _TMS_SCHEDULE_INQUIRE_EVENT_;

				return (VS_SUCCESS);
			}
		}
	}


	/* TMS排程下載 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTMSOK(szTemplate);

	/* TMSOK 表示已下載過TMS */
	if (!memcmp(&szTemplate[0], "Y", 1))
	{
		inLoadTMSCPTRec(0);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSDownloadFlag(szTemplate);
		/* TMS Download Flag = 2 由TMS排程下載 */
		if (!memcmp(szTemplate, _TMS_DOWNLOAD_FLAG_SCHEDULE_, strlen(_TMS_DOWNLOAD_FLAG_SCHEDULE_)))
		{
			/* 排程時間檢查 */
			if (inNCCCTMS_Schedule_Download_Date_Time_Check(&pobTran) == VS_SUCCESS)
			{
				*inEvent = _TMS_SCHEDULE_DOWNLOAD_EVENT_;

				return (VS_SUCCESS);
			}
		}
	}

	
	/* TMS參數生效 */
	/* 有File List檔案代表有參數要更新 */
	/* 代表有參數列表，有機會更新，接著檢查時間 */
	/* 只有走ISO才要排程更新 */
	if (inNCCCTMS_Check_FileList_Flow(&pobTran) == VS_SUCCESS)
	{
		/* ISO8583 */
		if (pobTran.uszFTP_TMS_Download != 'Y')
		{
			if (inEDCTMS_SCHEDULE_CheckDownloadDateTime(&pobTran) == VS_SUCCESS)
			{
				/* 檢查時間到時，檢查FileList 及 下載檔案是否合法 */
				if (inNCCCTMS_CheckAllDownloadFile_Flow(&pobTran) != VS_SUCCESS)
				{

				}
				else
				{
					/* 檢查是否有帳 */
					inLoadTMSCPTRec(0);
					inGetTMSEffectiveCloseBatch(szCloseBatchBit);
					if (memcmp(szCloseBatchBit, "Y", strlen("Y")) == 0)
					{
						if (inFLOW_RunFunction(&pobTran, _FUNCTION_All_HOST_MUST_SETTLE) != VS_SUCCESS)
						{

						}
						else
						{
							uszUpdateBit = VS_TRUE;
						}
					}
					else
					{
						uszUpdateBit = VS_TRUE;
					}

					if (uszUpdateBit == VS_TRUE)
					{
						inDISP_ClearAll();
						inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
						inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數下載＞ */
						inDISP_PutGraphic(_TMS_UPDATING_, 0, _COORDINATE_Y_LINE_8_4_);

						/* 重開機更新 */
						inFunc_Reboot();
					}
				}
			}
		}
		/* FTPS */
		else
		{

		}
	}
#endif	
	/* DCC排程下載時間檢查 */
/* 目前用不到 20190327 [SAM] */	
#if 0	
	if (inNCCC_DCC_AutoDownload_Check() == VS_SUCCESS)
	{
		*inEvent = _DCC_SCHEDULE_EVENT_;

		return (VS_SUCCESS);
	}


	/* 整點提示DCC下載 */
	memset(szDCCDownloadMode, 0x00, sizeof(szDCCDownloadMode));
	inGetDCCDownloadMode(szDCCDownloadMode);
	if (memcmp(szDCCDownloadMode, "2", 1) == 0)
	{
		if (inNCCC_DCC_TMS_Schedule_Hour_Check() == VS_SUCCESS)
		{
			*inEvent = _TMS_DCC_SCHEDULE_EVENT_;

			return (VS_SUCCESS);
		}
	}
#endif

	/* 每五分鐘檢查一次是否有ESC未上傳 */
	/* 先確認ESC功能有沒有開 */
/* 目前用不到 20190327 [SAM] */	
#if 0	
	memset(szESCMode, 0x00, sizeof(szESCMode));
	inGetESCMode(szESCMode);
	if (memcmp(&szESCMode[0], "Y", 1) == 0)
	{
		/* 檢查是否已離上次上傳超過五分鐘 */
		if (inNCCC_ESC_Func_Upload_Idle_Check_Time() == VS_SUCCESS)
		{
			*inEvent = _ESC_IDLE_UPLOAD_EVENT_;

			return (VS_SUCCESS);
		}
	}
#endif
	
	return (VS_SUCCESS);
}

/* 
Function        : inFunc_CheckTermStatus
Date&Time   : 2019/04/03
Describe        : 確認機器狀態，例如否已下TMS參數和DCC參數等。
 * 會使用到 EDC.C  CFGT.C 參數
 * [整理功能並加註解]  [SAM] 
*/
int inFunc_CheckTermStatus(TRANSACTION_OBJECT *pobTran)
{
	char		szTMSOK[2 + 1] = {0};
	char		szContactlessReaderMode[2 + 1] = {0};
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 檢查是否已經鎖機 */
	if (inFunc_Check_EDCLock() != VS_SUCCESS)
		return (VS_ERROR);

	inDISP_LogPrintfWithFlag("  inFunc_CheckTermStatus 2!!");

	/* 檢查是否做過【參數下載】 */
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (memcmp(szTMSOK, "Y", 1) != 0)
	{
		inDISP_Msg_BMP(_ERR_TMS_DWL_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		return (VS_ERROR);
	}

#ifdef __MUTI_FUCN_TEST_
	memset(szContactlessReaderMode, 0x00, sizeof(szContactlessReaderMode));
	inGetContactlessReaderMode(szContactlessReaderMode);
	if (memcmp(szContactlessReaderMode, _CTLS_MODE_0_NO_, strlen(_CTLS_MODE_0_NO_)) == 0)
	{
		inDISP_Msg_BMP("", 0, _0_KEY_MSG_, _EDC_TIMEOUT_, "請重新參數下載", _LINE_8_6_);
		inDISP_LogPrintfWithFlag("  inFunc_CheckTermStatus 13!!");
		return (VS_ERROR);
	}
#else
	/* 因為現在沒有外接感應，若下載為外接感應顯示"請重新下載參數" */
	memset(szContactlessReaderMode, 0x00, sizeof(szContactlessReaderMode));
	inGetContactlessReaderMode(szContactlessReaderMode);
	if (memcmp(szContactlessReaderMode, _CTLS_MODE_0_NO_, strlen(_CTLS_MODE_0_NO_)) == 0	||
	    !memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
            !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)) )
	{
		inDISP_Msg_BMP("", 0, _0_KEY_MSG_, _EDC_TIMEOUT_, "請重新參數下載", _LINE_8_6_);
		inDISP_LogPrintfWithFlag("  inFunc_CheckTermStatus 13!!");
		return (VS_ERROR);
	}
#endif
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckTermStatus_Ticket
Date&Time       :2018/1/17 下午 2:18
Describe        :確認機器狀態，包括是否已下TMS參數
*/
int inFunc_CheckTermStatus_Ticket(TRANSACTION_OBJECT *pobTran)
{
	int	inESVCIndex = -1;
	char	szTMSOK[2 + 1] = {0};
	char	szHostIndex[2 + 1] = {0};
	char	szSettleBit[2 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_CheckTermStatus_Ticket() START !");
	}

	/* 檢查是否已經鎖機 */
	if (inFunc_Check_EDCLock() != VS_SUCCESS)
		return (VS_ERROR);

	/* 檢查是否做過【參數下載】 */
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (memcmp(szTMSOK, "Y", 1) != 0)
	{
		inDISP_Msg_BMP(_ERR_TMS_DWL_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

		return (VS_ERROR);
	}

	if (inLoadTDTRec(_TDT_INDEX_00_IPASS_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	memset(szHostIndex, 0x00, sizeof(szHostIndex));
	inGetTicket_HostIndex(szHostIndex);

	inESVCIndex = atoi(szHostIndex);

	if (inLoadHDTRec(inESVCIndex) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" FUNC Check Term Status Tick Load HDT[%d] *Error* Line[%d]", inESVCIndex, __LINE__);
		return (VS_ERROR);
	}

	if (inLoadHDPTRec(inESVCIndex) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" FUNC Check Term Status Tick Load HDPT[%d] *Error* Line[%d]", inESVCIndex, __LINE__);
		return (VS_ERROR);
	}
	/* 檢查是否要結帳 */
	memset(szSettleBit, 0x00, sizeof(szSettleBit));
	inGetMustSettleBit(szSettleBit);
	if (memcmp(szSettleBit, "Y", strlen("Y")) == 0)
	{
		/* 表示要結帳 */
                inDISP_Msg_BMP(_ERR_MUST_SETTLE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "電子票證", _LINE_8_5_);

		return (VS_ERROR);
	}

	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
/* 目前用不到 20190327 [SAM] */	
#if 0 
		pobTran->srTRec.inTicketType = _TICKET_TYPE_NONE_;
#endif
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inFunc_CheckTermStatus_Ticket() END !");
			inDISP_LogPrintf("----------------------------------------");
		}

		return (VS_SUCCESS);
	}
	else
	{
/* 目前用不到 20190327 [SAM] */	
#if 0
		pobTran->srTRec.inTicketType = _TICKET_TYPE_NONE_;
#endif
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inFunc_CheckTermStatus_Ticket() END !");
			inDISP_LogPrintf("----------------------------------------");
		}

		return (VS_SUCCESS);
	}
}



/*
Function        :inFunc_ResetTitle
Date&Time       :2016/9/7 下午 1:02
Describe        :重新顯示交易別標題
 */
int inFunc_ResetTitle(TRANSACTION_OBJECT *pobTran)
{
	int inDisplay = VS_FALSE;

	inDISP_ClearAll();

	/* 考慮到_TRT_SALE_ICC_和_TRT_SALE_CTLS_無法區分交易別，改以inTransactionCode來判斷 */
	if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
	{
		/* 如果OPT是重印，要顯示重印才對 */
		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REPRINT_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜重印簽單＞ */
		inDisplay = VS_TRUE;
		return (VS_SUCCESS);
	} else if (pobTran->inTransactionCode == _TICKET_DEDUCT_)
	{
		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜消費扣款＞ */
		inDisplay = VS_TRUE;
		return (VS_SUCCESS);
	} else if (pobTran->inRunOperationID == _OPERATION_TICKET_INQUIRY_)
	{
		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_TICKET_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜餘額查詢＞ */
		inDisplay = VS_TRUE;

		return (VS_SUCCESS);
	} else if (pobTran->inRunOperationID == _OPERATION_TICKET_TOP_UP_)
	{
		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_TICKET_TOP_UP_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜餘額加值＞ */
		inDisplay = VS_TRUE;

		return (VS_SUCCESS);
	} else if (pobTran->inRunOperationID == _OPERATION_HG_)
	{
		switch (pobTran->srBRec.lnHGTransactionType)
		{
			case _HG_REWARD_:
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡紅利積點＞ */
				inDisplay = VS_TRUE;
				break;
			case _HG_ONLINE_REDEEM_:
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_HAPPYGO_ONLINE_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡加價購＞ */
				inDisplay = VS_TRUE;
				break;
			case _HG_POINT_CERTAIN_:
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_HAPPYGO_POINT_CERTAIN_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡點數抵扣＞ */
				inDisplay = VS_TRUE;
				break;
			case _HG_FULL_REDEMPTION_:
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_HAPPYGO_REDEMPTION_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡點數兌換＞ */
				inDisplay = VS_TRUE;
				break;
			case _HG_INQUIRY_:
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_HAPPYGO_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡點數查詢＞ */
				inDisplay = VS_TRUE;
				break;
			default:
				break;
		}
		/* 有Display就跳出否則繼續跑下面 */
		if (inDisplay == VS_TRUE)
			return (VS_SUCCESS);
	} else if (pobTran->inRunOperationID == _OPERATION_I_R_ && pobTran->srBRec.lnHGTransactionType == _HG_REWARD_)
	{
		switch (pobTran->inTransactionCode)
		{
			case _REDEEM_SALE_:
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡紅利扣抵＞ */
				inDisplay = VS_TRUE;
				break;
			case _INST_SALE_:
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_INS_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡分期付款＞ */
				inDisplay = VS_TRUE;
				break;
			default:
				break;
		}
		/* 有Display就跳出否則繼續跑下面 */
		if (inDisplay == VS_TRUE)
			return (VS_SUCCESS);
	} else if (pobTran->inRunOperationID == _OPERATION_HG_REFUND_)
	{
		switch (pobTran->srBRec.lnHGTransactionType)
		{
			case _HG_REWARD_REFUND_:
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡回饋退貨＞ */
				inDisplay = VS_TRUE;
				break;
			case _HG_REDEEM_REFUND_:
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_HAPPYGO_REDEEM_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡扣抵退貨＞ */
				inDisplay = VS_TRUE;
				break;
			default:
				break;
		}
		/* 有Display就跳出否則繼續跑下面 */
		if (inDisplay == VS_TRUE)
			return (VS_SUCCESS);
	}

	if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
	{
		switch (pobTran->inTransactionCode)
		{
			case _SALE_:
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_DCC_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜DCC一般交易＞ */
				inDisplay = VS_TRUE;
				break;
			case _VOID_:
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_DCC_VOID_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜DCC取消交易＞ */
				inDisplay = VS_TRUE;
				break;
			case _REFUND_:
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_DCC_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜DCC退貨交易＞ */
				inDisplay = VS_TRUE;
				break;
			case _TIP_:
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_DCC_TIP_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜DCC小費交易＞ */
				inDisplay = VS_TRUE;
				break;
			case _DCC_RATE_:
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_DCC_RATE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜DCC詢價交易＞ */
				inDisplay = VS_TRUE;
				break;
			case _PRE_COMP_:
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜預先授權完成＞ */
				inDisplay = VS_TRUE;
				break;
			case _SETTLE_:
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜結帳交易＞ */
				inDisplay = VS_TRUE;
				break;
			default:
				break;

		}
		/* 有Display就跳出否則繼續跑下面 */
		if (inDisplay == VS_TRUE)
			return (VS_SUCCESS);
	}

	/* DCC轉台幣，明明流程是_SALE_OFFLINE_，但需要顯示一般交易 */
	if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)
	{
		switch (pobTran->inTransactionCode)
		{
			case _SALE_OFFLINE_:
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜一般交易＞ */
				inDisplay = VS_TRUE;
				break;
			case _VOID_:
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜取消交易＞ */
				inDisplay = VS_TRUE;
				break;
			case _TIP_:
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TIP_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜小費交易＞ */
				inDisplay = VS_TRUE;
				break;
			default:
				break;
		}

		/* 有Display就跳出否則繼續跑下面 */
		if (inDisplay == VS_TRUE)
			return (VS_SUCCESS);
	}

	switch (pobTran->inTransactionCode)
	{
		case _CHANGE_TMK_:
		case _CUP_CHANG_TPK_:
		case _CUP_LOGON_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_LOGON_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第一層顯示 <安全認證> */
			inDisplay = VS_TRUE;
			break;
		case _SALE_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜一般交易＞ */
			inDisplay = VS_TRUE;
			break;
		case _VOID_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_VOID_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜取消交易＞ */
			inDisplay = VS_TRUE;
			break;
		case _REFUND_:
		case _CUP_MAIL_ORDER_REFUND_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜一般退貨＞ */
			inDisplay = VS_TRUE;
			break;
		case _REDEEM_SALE_:
		case _CUP_REDEEM_SALE_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜紅利扣抵＞ */
			inDisplay = VS_TRUE;
			break;
		case _REDEEM_REFUND_:
		case _CUP_REDEEM_REFUND_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜紅利退貨＞ */
			inDisplay = VS_TRUE;
			break;
		case _REDEEM_ADJUST_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REDEEM_ADJUST_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜紅利調帳＞ */
			inDisplay = VS_TRUE;
			break;
		case _INST_SALE_:
		case _CUP_INST_SALE_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜分期付款＞ */
			inDisplay = VS_TRUE;
			break;
		case _INST_REFUND_:
		case _CUP_INST_REFUND_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜分期退貨＞ */
			inDisplay = VS_TRUE;
			break;
		case _INST_ADJUST_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_INST_ADJUST_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜分期調帳＞ */
			inDisplay = VS_TRUE;
			break;
		case _SETTLE_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜結帳交易＞ */
			inDisplay = VS_TRUE;
			break;
		case _SALE_OFFLINE_:
			if (pobTran->srBRec.uszReferralBit == VS_TRUE)
			{
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_CALL_BANK_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 授權碼補登 */
			} else
			{
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜交易補登＞ */
			}
			inDisplay = VS_TRUE;
			break;
		case _TIP_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TIP_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜小費交易＞ */
			inDisplay = VS_TRUE;
			break;
		case _PRE_AUTH_:
		case _CUP_PRE_AUTH_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜預先授權＞ */
			inDisplay = VS_TRUE;
			break;
		case _ADJUST_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_ADJUST_1_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜後台調帳＞ */
			//            inDISP_PutGraphic(_MENU_ADJUST_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜後台調帳＞ */
			inDisplay = VS_TRUE;
			break;
		case _CUP_SALE_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜銀聯一般交易＞ */
			inDisplay = VS_TRUE;
			break;
		case _CUP_VOID_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_CUP_VOID_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜銀聯取消＞ */
			inDisplay = VS_TRUE;
			break;
		case _CUP_REFUND_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_CUP_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜銀聯退貨> */
			inDisplay = VS_TRUE;
			break;
		case _CUP_PRE_AUTH_VOID_:
		case _VOID_AUTH_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_AUTH_VOID_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜預先授權取消＞ */
			inDisplay = VS_TRUE;
			break;
		case _PRE_COMP_:
		case _CUP_PRE_COMP_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜預先授權完成＞ */
			inDisplay = VS_TRUE;
			break;
		case _CUP_PRE_COMP_VOID_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_COMP_VOID_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜預先授權完成取消＞ */
			inDisplay = VS_TRUE;
			break;
		case _FISC_SALE_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_FISC_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜消費扣款＞ */
			inDisplay = VS_TRUE;
			break;
		case _FISC_VOID_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_FISC_VOID_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜消費扣款沖正＞ */
			inDisplay = VS_TRUE;
			break;
		case _FISC_REFUND_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_FISC_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜退費交易＞ */
			inDisplay = VS_TRUE;
			break;
		case _LOYALTY_REDEEM_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_LOYALTY_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜優惠兌換＞ */
			inDisplay = VS_TRUE;
			break;
		case _VOID_LOYALTY_REDEEM_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_VOID_LOYALTY_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜兌換取消＞ */
			inDisplay = VS_TRUE;
			break;
		case _MAIL_ORDER_:
		case _CUP_MAIL_ORDER_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_MAIL_ORDER_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜郵購交易＞ */
			inDisplay = VS_TRUE;
			break;
		default:
			break;
	}

	/* 票證相關*/
	switch (pobTran->inTransactionCode)
	{
		case _TICKET_IPASS_LOGON_:
		case _TICKET_IPASS_REGISTER_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜安全認證＞ */
			inDisplay = VS_TRUE;
			break;
		case _TICKET_IPASS_DEDUCT_:
		case _TICKET_EASYCARD_DEDUCT_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜購貨交易＞ */
			inDisplay = VS_TRUE;
			break;
		case _TICKET_IPASS_REFUND_:
		case _TICKET_EASYCARD_REFUND_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜退貨交易＞ */
			inDisplay = VS_TRUE;
			break;
		case _TICKET_IPASS_INQUIRY_:
		case _TICKET_EASYCARD_INQUIRY_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜餘額查詢＞ */
			inDisplay = VS_TRUE;
			break;
		case _TICKET_IPASS_TOP_UP_:
		case _TICKET_EASYCARD_TOP_UP_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_TOP_UP_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜加值交易＞ */
			inDisplay = VS_TRUE;
			break;
		case _TICKET_IPASS_VOID_TOP_UP_:
		case _TICKET_EASYCARD_VOID_TOP_UP_:
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_VOID_TOP_UP_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜加值取消＞ */
			inDisplay = VS_TRUE;
			break;
		default:
			break;
	}

	/* 如果真的找不到至少可以顯示一般交易 */
	if (inDisplay == VS_TRUE)
	{
		return (VS_SUCCESS);
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			char szDebugMsg[100 + 1];

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Reset Title Fail");
			inDISP_LogPrintf(szDebugMsg);
		}

		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜一般交易＞ */

		return (VS_SUCCESS);
	}
}


/*
Function        :inFunc_EditPWD_Flow
Date&Time       :2016/11/25 下午 2:03
Describe        :功能6的設定管理號碼
*/
int inFunc_EditPWD_Flow(void)
{
	int	inRetVal = VS_SUCCESS;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char	szKey = 0x00;

        if (inLoadPWDRec(0) < 0)
        {
                /* 此功能已關閉 */
                inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
                return (VS_ERROR);
        }

        inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
        inDISP_PutGraphic(_MENU_SET_PWD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 設定管理號碼 */
        inDISP_PutGraphic(_FUNC6_PWD_MENU_, 0, _COORDINATE_Y_LINE_8_4_);
        inDISP_BEEP(1, 0);

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
        while (1)
        {
                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		/* 變更管理號碼 */
                if (szKey == _KEY_1_			||
		    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
                {
			inRetVal = inFunc_Edit_Manager_Pwd();
			break;
                }
		/* 交易功能管理 */
                else if (szKey == _KEY_2_		||
			 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
                {
			inRetVal = inFunc_Edit_TransFuc_Pwd();
			break;
                }
                else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
        }
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

        return (VS_SUCCESS);
}

/*
Function        :inFunc_Edit_Manager_Pwd
Date&Time       :2017/8/25 下午 5:24
Describe        :
*/
int inFunc_Edit_Manager_Pwd()
{
	int		inRetVal = VS_SUCCESS;
	char		szPWDOld[4 + 1], szPWDNew[4 + 1];
	char		szInitialManagerEnable[2 + 1];
	DISPLAY_OBJECT  srDispObj;

	memset(szInitialManagerEnable, 0x00, sizeof(szInitialManagerEnable));
        inGetInitialManagerEnable(szInitialManagerEnable);
	/* 變更管理者號碼 */
	if (memcmp(szInitialManagerEnable, "Y", 1) != 0)
	{
		/* 此功能已關閉 */
		inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, 30, "", 0);
		return (VS_ERROR);
	}

	/* 請輸入預設管理者號碼 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_GET_MANAGER_ORG_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
	memset(szPWDOld, 0x00, sizeof(szPWDOld));
	inGetInitialManagerPwd(szPWDOld);

	while (1)
	{
		inDISP_BEEP(1, 0);
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

		/* 設定顯示變數 */
		srDispObj.inMaxLen = 4;
		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMask = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (VS_ERROR);

		if (srDispObj.inOutputLen == 4)
			break;
		else
			continue;
	}

	if (!memcmp(&srDispObj.szOutput[0], &szPWDOld[0], 4))
	{
		/* 輸入正確 */
	}
	else
	{
		/* 輸入錯誤 */
		inDISP_Msg_BMP(_ERR_PWD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
		return (VS_ERROR);
	}

	while (1)
	{
		/* 原管理者號碼  新管理者號碼? */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_SET_NEW_MANAGER_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
		/* 提示原始密碼 */
		inDISP_EnglishFont(szPWDOld, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

		while (1)
		{
			inDISP_BEEP(1, 0);
			inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
			memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

			/* 設定顯示變數 */
			srDispObj.inMaxLen = 4;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inMask = VS_TRUE;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;

			inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				return (VS_ERROR);

			if (srDispObj.inOutputLen == 4)
				break;
			else
				continue;
		}

		memset(szPWDNew, 0x00, sizeof(szPWDNew));
		memcpy(&szPWDNew[0], &srDispObj.szOutput[0], 4);

		/* 請再次輸入新管理者號碼 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_SET_MANAGER_PWD_AGAIN_, 0, _COORDINATE_Y_LINE_8_4_);

		while (1)
		{
			inDISP_BEEP(1, 0);
			inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
			memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

			/* 設定顯示變數 */
			srDispObj.inMaxLen = 4;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inMask = VS_TRUE;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;

			inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				return (VS_ERROR);

			if (srDispObj.inOutputLen == 4)
				break;
			else
				continue;
		}

		if (!memcmp(&srDispObj.szOutput[0], &szPWDNew[0], 4))
		{
			/* 修改成功 圖片 */
			inDISP_Msg_BMP(_ERR_SET_PWD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
			/* 儲存管理者號碼 */
			inSetInitialManagerPwd(szPWDNew);
			inSavePWDRec(0);

			return (VS_SUCCESS);
		}
		else
		{
			/* 輸入錯誤 */
			inDISP_Msg_BMP(_ERR_PWD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
			continue;
		}
	}

	return (VS_SUCCESS);
}

int inFunc_Edit_TransFuc_Pwd()
{
	int		inRetVal = VS_SUCCESS;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CHECK_PWD_EDIT_;
	char		szPWDOld[4 + 1], szPWDNew[4 + 1];
        char		szManagerPWD[4 + 1];
        char		szKey = 0x00;
	char		szInitialManagerEnable[2 + 1];
        char		szRebootPwdEnale[2 + 1],
			szSalePwdEnable[2 + 1],
			szPreauthPwdEnable[2 + 1],
			szInstallmentPwdEnable[2 + 1],
			szRedeemPwdEnable[2 + 1],
			szOfflinePwdEnable[2 + 1],
			szInstallmentAdjustPwdEnable[2 + 1],
			szRedeemAdjustPwdEnable[2 + 1],
			szVoidPwdEnable[2 + 1],
			szSettlementPwdEnable[2 + 1],
			szRefundPwdEnable[2 + 1],
			szCallBankForcePwdEnable[2 + 1];
	VS_BOOL		fEditPWD = VS_FALSE;
        DISPLAY_OBJECT  srDispObj;

	/* Get 各交易密碼開關 */
        memset(szInitialManagerEnable, 0x00, sizeof(szInitialManagerEnable));
        inGetInitialManagerEnable(szInitialManagerEnable);
        memset(szRebootPwdEnale, 0x00, sizeof(szRebootPwdEnale));
        inGetRebootPwdEnale(szRebootPwdEnale);
        memset(szSalePwdEnable, 0x00, sizeof(szSalePwdEnable));
        inGetSalePwdEnable(szSalePwdEnable);
        memset(szPreauthPwdEnable, 0x00, sizeof(szPreauthPwdEnable));
        inGetPreauthPwdEnable(szPreauthPwdEnable);
        memset(szInstallmentPwdEnable, 0x00, sizeof(szInstallmentPwdEnable));
        inGetInstallmentPwdEnable(szInstallmentPwdEnable);
        memset(szRedeemPwdEnable, 0x00, sizeof(szRedeemPwdEnable));
        inGetRedeemPwdEnable(szRedeemPwdEnable);
        memset(szOfflinePwdEnable, 0x00, sizeof(szOfflinePwdEnable));
        inGetOfflinePwdEnable(szOfflinePwdEnable);
        memset(szInstallmentAdjustPwdEnable, 0x00, sizeof(szInstallmentAdjustPwdEnable));
        inGetInstallmentAdjustPwdEnable(szInstallmentAdjustPwdEnable);
        memset(szRedeemAdjustPwdEnable, 0x00, sizeof(szRedeemAdjustPwdEnable));
        inGetRedeemAdjustPwdEnable(szRedeemAdjustPwdEnable);
        memset(szVoidPwdEnable, 0x00, sizeof(szVoidPwdEnable));
        inGetVoidPwdEnable(szVoidPwdEnable);
        memset(szSettlementPwdEnable, 0x00, sizeof(szSettlementPwdEnable));
        inGetSettlementPwdEnable(szSettlementPwdEnable);
        memset(szRefundPwdEnable, 0x00, sizeof(szRefundPwdEnable));
        inGetRefundPwdEnable(szRefundPwdEnable);
	memset(szCallBankForcePwdEnable, 0x00, sizeof(szCallBankForcePwdEnable));
        inGetCallBankForcePwdEnable(szCallBankForcePwdEnable);

	if (memcmp(szInitialManagerEnable, "Y", 1) != 0)
	{
		/* 此功能已關閉 */
		inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, 30, "", 0);
		return (VS_ERROR);
	}

	/* 請輸入管理者號碼 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_GET_MANAGER_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
	memset(szManagerPWD, 0x00, sizeof(szManagerPWD));
	inGetInitialManagerPwd(szManagerPWD);

	while (1)
	{
		inDISP_BEEP(1, 0);
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

		/* 設定顯示變數 */
		srDispObj.inMaxLen = 4;
		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMask = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (VS_ERROR);

		if (srDispObj.inOutputLen == 4)
			break;
		else
			continue;
	}

	if (!memcmp(&srDispObj.szOutput[0], &szManagerPWD[0], 4))
	{
		 /* 輸入正確 */
	}
	else
	{
		/* 輸入錯誤 */
		inDISP_Msg_BMP(_ERR_PWD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
		return (VS_ERROR);
	}

	if (!memcmp(&szSalePwdEnable[0], "Y", 1) ||
	    !memcmp(&szPreauthPwdEnable[0], "Y", 1) ||
	    !memcmp(&szInstallmentPwdEnable[0], "Y", 1) ||
	    !memcmp(&szRedeemPwdEnable[0], "Y", 1))
	{
		/* 正項交易 */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_SALE_PWD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 正向交易 */
		/* 按0修改按確認跳下一步 */
		inDISP_PutGraphic(_CHECK_EDIT_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
		inDISP_BEEP(1, 0);

		inTouchSensorFunc = _Touch_CHECK_PWD_EDIT_;
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_0_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_TRUE;
				break;
			}
			else if (szKey == _KEY_ENTER_			||
				 inChoice == _CHECKEditPWD_Touch_ENTER_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_FALSE;
				break;
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_ERROR;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();

		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		/* 修改正項交易密碼 */
		if (fEditPWD == VS_TRUE)
		{
			memset(szPWDOld, 0x00, sizeof(szPWDOld));
			inGetSalePwd(szPWDOld);

			while (1)
			{
				/* 原管理號碼  新管理號碼? */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
				/* 提示原始密碼 */
				inDISP_EnglishFont(szPWDOld, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;

					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else
						continue;
				}

				memset(szPWDNew, 0x00, sizeof(szPWDNew));
				memcpy(&szPWDNew[0], &srDispObj.szOutput[0], 4);

				/* 請再次輸入新管理號碼 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_AGAIN_, 0, _COORDINATE_Y_LINE_8_4_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;

					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else
						continue;
				}

				if (!memcmp(&srDispObj.szOutput[0], &szPWDNew[0], 4))
				{
					/* 修改成功 圖片 */
					inDISP_Msg_BMP(_ERR_SET_PWD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
					/* 儲存管理號碼 */
					inSetSalePwd(szPWDNew);
					inSetPreauthPwd(szPWDNew);
					inSetInstallmentPwd(szPWDNew);
					inSetRedeemPwd(szPWDNew);
					inSavePWDRec(0);

					break;
				}
				else
				{
					/* 輸入錯誤 */
					inDISP_Msg_BMP(_ERR_PWD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
					continue;
				}
			}
		} /* if (Edit == true) end */
	}

	if (!memcmp(&szRefundPwdEnable[0], "Y", 1))
	{
		/* 退貨交易 */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */
		inDISP_PutGraphic(_CHECK_EDIT_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
		inDISP_BEEP(1, 0);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_0_)
			{
				fEditPWD = VS_TRUE;
				inRetVal = VS_SUCCESS;
				break;
			}
			else if (szKey == _KEY_ENTER_			||
				 inChoice == _CHECKEditPWD_Touch_ENTER_)
			{
				fEditPWD = VS_FALSE;
				inRetVal = VS_SUCCESS;
				break;
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				break;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();

		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		/* 修改退貨交易密碼 */
		if (fEditPWD == VS_TRUE)
		{
			memset(szPWDOld, 0x00, sizeof(szPWDOld));
			inGetRefundPwd(szPWDOld);

			while (1)
			{
				/* 原管理號碼  新管理號碼? */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
				/* 提示原始密碼 */
				inDISP_EnglishFont(szPWDOld, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;

					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else
						continue;
				}

				memset(szPWDNew, 0x00, sizeof(szPWDNew));
				memcpy(&szPWDNew[0], &srDispObj.szOutput[0], 4);

				/* 請再次輸入新管理號碼 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_AGAIN_, 0, _COORDINATE_Y_LINE_8_4_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;

					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else
						continue;
				}

				if (!memcmp(&srDispObj.szOutput[0], &szPWDNew[0], 4))
				{
					/* 修改成功 圖片 */
					inDISP_Msg_BMP(_ERR_SET_PWD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
					/* 儲存管理號碼 */
					inSetRefundPwd(szPWDNew);
					inSavePWDRec(0);

					break;
				}
				else
				{
					/* 輸入錯誤 */
					inDISP_Msg_BMP(_ERR_PWD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
					continue;
				}
			}
		} /* if (Edit == true) end */
	}

	if (!memcmp(&szVoidPwdEnable[0], "Y", 1))
	{
		/* 取消交易 */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */
		inDISP_PutGraphic(_CHECK_EDIT_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
		inDISP_BEEP(1, 0);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_0_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_TRUE;
				break;
			}
			else if (szKey == _KEY_ENTER_			||
				 inChoice == _CHECKEditPWD_Touch_ENTER_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_FALSE;
				break;
			}
			else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_ERROR;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();

		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		/* 修改取消交易密碼 */
		if (fEditPWD == VS_TRUE)
		{
			memset(szPWDOld, 0x00, sizeof(szPWDOld));
			inGetVoidPwd(szPWDOld);

			while (1)
			{
				/* 原管理號碼  新管理號碼? */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
				/* 提示原始密碼 */
				inDISP_EnglishFont(szPWDOld, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;

					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else
						continue;
				}

				memset(szPWDNew, 0x00, sizeof(szPWDNew));
				memcpy(&szPWDNew[0], &srDispObj.szOutput[0], 4);

				/* 請再次輸入新管理號碼 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_AGAIN_, 0, _COORDINATE_Y_LINE_8_4_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;

					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else
						continue;
				}

				if (!memcmp(&srDispObj.szOutput[0], &szPWDNew[0], 4))
				{
					/* 修改成功 圖片 */
					inDISP_Msg_BMP(_ERR_SET_PWD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
					/* 儲存管理號碼 */
					inSetVoidPwd(szPWDNew);
					inSavePWDRec(0);

					break;
				}
				else
				{
					/* 輸入錯誤 */
					inDISP_Msg_BMP(_ERR_PWD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
					continue;
				}
			}
		} /* if (Edit == true) end */
	}

	if (!memcmp(&szOfflinePwdEnable[0], "Y", 1) ||
	    !memcmp(&szInstallmentAdjustPwdEnable[0], "Y", 1) ||
	    !memcmp(&szRedeemAdjustPwdEnable[0], "Y", 1))
	{
		/* 交易補登 */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜交易補登＞ */
		inDISP_PutGraphic(_CHECK_EDIT_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
		inDISP_BEEP(1, 0);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_0_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_TRUE;
				break;
			}
			else if (szKey == _KEY_ENTER_			||
				 inChoice == _CHECKEditPWD_Touch_ENTER_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_FALSE;
				break;
			}
			else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_ERROR;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();

		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		/* 修改交易補登密碼 */
		if (fEditPWD == VS_TRUE)
		{
			memset(szPWDOld, 0x00, sizeof(szPWDOld));
			inGetOfflinePwd(szPWDOld);

			while (1)
			{
				/* 原管理號碼  新管理號碼? */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
				/* 提示原始密碼 */
				inDISP_EnglishFont(szPWDOld, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;

					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else
						continue;
				}

				memset(szPWDNew, 0x00, sizeof(szPWDNew));
				memcpy(&szPWDNew[0], &srDispObj.szOutput[0], 4);

				/* 請再次輸入新管理號碼 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_AGAIN_, 0, _COORDINATE_Y_LINE_8_4_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;

					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else
						continue;
				}

				if (!memcmp(&srDispObj.szOutput[0], &szPWDNew[0], 4))
				{
					/* 修改成功 圖片 */
					inDISP_Msg_BMP(_ERR_SET_PWD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
					/* 儲存管理號碼 */
					inSetOfflinePwd(szPWDNew);
					inSetInstallmentAdjustPwd(szPWDNew);
					inSetRedeemAdjustPwd(szPWDNew);
					inSavePWDRec(0);

					break;
				}
				else
				{
					/* 輸入錯誤 */
					inDISP_Msg_BMP(_ERR_PWD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
					continue;
				}
			}
		} /* if (Edit == true) end */
	}

	if (!memcmp(&szCallBankForcePwdEnable[0], "Y", 1))
	{
		/* 交易補登 */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_CALL_BANK_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜授權碼補登＞ */
		inDISP_PutGraphic(_CHECK_EDIT_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
		inDISP_BEEP(1, 0);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_0_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_TRUE;
				break;
			}
			else if (szKey == _KEY_ENTER_			||
				 inChoice == _CHECKEditPWD_Touch_ENTER_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_FALSE;
				break;
			}
			else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_ERROR;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();

		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		/* 修改授權碼補登密碼 */
		if (fEditPWD == VS_TRUE)
		{
			memset(szPWDOld, 0x00, sizeof(szPWDOld));
			inGetCallBankForcePwd(szPWDOld);

			while (1)
			{
				/* 原管理號碼  新管理號碼? */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
				/* 提示原始密碼 */
				inDISP_EnglishFont(szPWDOld, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;

					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else
						continue;
				}

				memset(szPWDNew, 0x00, sizeof(szPWDNew));
				memcpy(&szPWDNew[0], &srDispObj.szOutput[0], 4);

				/* 請再次輸入新管理號碼 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_AGAIN_, 0, _COORDINATE_Y_LINE_8_4_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;

					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else
						continue;
				}

				if (!memcmp(&srDispObj.szOutput[0], &szPWDNew[0], 4))
				{
					/* 修改成功 圖片 */
					inDISP_Msg_BMP(_ERR_SET_PWD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
					/* 儲存管理號碼 */
					inSetOfflinePwd(szPWDNew);
					inSetInstallmentAdjustPwd(szPWDNew);
					inSetRedeemAdjustPwd(szPWDNew);
					inSavePWDRec(0);

					break;
				}
				else
				{
					/* 輸入錯誤 */
					inDISP_Msg_BMP(_ERR_PWD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
					continue;
				}
			}
		} /* if (Edit == true) end */
	}

	if (!memcmp(&szSettlementPwdEnable[0], "Y", 1))
	{
		/* 結帳交易 */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);			/* 第三層顯示 ＜結帳交易＞ */
		inDISP_PutGraphic(_CHECK_EDIT_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
		inDISP_BEEP(1, 0);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_0_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_TRUE;
				break;
			}
			else if (szKey == _KEY_ENTER_			||
				 inChoice == _CHECKEditPWD_Touch_ENTER_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_FALSE;
				break;
			}
			else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_ERROR;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();

		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		/* 修改結帳交易密碼 */
		if (fEditPWD == VS_TRUE)
		{
			memset(szPWDOld, 0x00, sizeof(szPWDOld));
			inGetSettlementPwd(szPWDOld);

			while (1)
			{
				/* 原管理號碼  新管理號碼? */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
				/* 提示原始密碼 */
				inDISP_EnglishFont(szPWDOld, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;

					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else
						continue;
				}

				memset(szPWDNew, 0x00, sizeof(szPWDNew));
				memcpy(&szPWDNew[0], &srDispObj.szOutput[0], 4);

				/* 請再次輸入新管理號碼 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_AGAIN_, 0, _COORDINATE_Y_LINE_8_4_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;

					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else
						continue;
				}

				if (!memcmp(&srDispObj.szOutput[0], &szPWDNew[0], 4))
				{
					/* 修改成功 圖片 */
					inDISP_Msg_BMP(_ERR_SET_PWD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
					/* 儲存管理號碼 */
					inSetSettlementPwd(szPWDNew);
					inSavePWDRec(0);

					break;
				}
				else
				{
					/* 輸入錯誤 */
					inDISP_Msg_BMP(_ERR_PWD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
					continue;
				}
			}
		} /* if (Edit == true) end */
	}

	if (!memcmp(&szRebootPwdEnale[0], "Y", 1))
	{
		/* 開機密碼 */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_POWER_ON_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 開機 */
		inDISP_PutGraphic(_CHECK_EDIT_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
		inDISP_BEEP(1, 0);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_0_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_TRUE;
				break;
			}
			else if (szKey == _KEY_ENTER_			||
				 inChoice == _CHECKEditPWD_Touch_ENTER_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_FALSE;
				break;
			}
			else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_ERROR;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();

		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		/* 修改開機密碼 */
		if (fEditPWD == VS_TRUE)
		{
			memset(szPWDOld, 0x00, sizeof(szPWDOld));
			inGetRebootPwd(szPWDOld);

			while (1)
			{
				/* 原管理號碼  新管理號碼? */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
				/* 提示原始密碼 */
				inDISP_EnglishFont(szPWDOld, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;

					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else
						continue;
				}

				memset(szPWDNew, 0x00, sizeof(szPWDNew));
				memcpy(&szPWDNew[0], &srDispObj.szOutput[0], 4);

				/* 請再次輸入新管理號碼 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_AGAIN_, 0, _COORDINATE_Y_LINE_8_4_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;

					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else
						continue;
				}

				if (!memcmp(&srDispObj.szOutput[0], &szPWDNew[0], 4))
				{
					/* 修改成功 圖片 */
					inDISP_Msg_BMP(_ERR_SET_PWD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
					/* 儲存管理號碼 */
					inSetRebootPwd(szPWDNew);
					inSavePWDRec(0);

					break;
				}
				else
				{
					/* 輸入錯誤 */
					inDISP_Msg_BMP(_ERR_PWD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);
					continue;
				}
			}
		} /* if (Edit == true) end */
	}
	return (VS_SUCCESS);
}

/*
Function        :inFunc_ReviewReport
Date&Time       :2016/2/24 上午 9:33
Describe        :交易查詢使用
*/
int inFunc_ReviewReport(TRANSACTION_OBJECT *pobTran)
{
	/* inACCUM_ReviewReport_Total */
	if (inFLOW_RunFunction(pobTran, _FUNCTION_TOTAL_REVIEW_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* inBATCH_ReviewReport_Detail_Flow_By_Sqlite */
	if (inFLOW_RunFunction(pobTran, _FUNCTION_DETAIL_REVIEW_) != VS_SUCCESS)
	{
		inBATCH_CheckReport_By_Sqlite(pobTran);

		return (VS_ERROR);
	}


	return	(VS_SUCCESS);
}

/*
Function        :inFunc_ReviewReport
Date&Time       :2016/2/24 上午 9:33
Describe        :交易查詢使用
*/
int inFunc_ReviewReport_NEWUI(TRANSACTION_OBJECT *pobTran)
{
	/* inACCUM_ReviewReport_Total */
	if (inFLOW_RunFunction(pobTran, _FUNCTION_TOTAL_REVIEW_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* inBATCH_ReviewReport_Detail_Flow_By_Sqlite */
	if (inFLOW_RunFunction(pobTran, _FUNCTION_DETAIL_REVIEW_NEWUI_) != VS_SUCCESS)
	{
		inBATCH_CheckReport_By_Sqlite(pobTran);

		return (VS_ERROR);
	}


	return	(VS_SUCCESS);
}

/*
Function        :inFun_CheckPAN_EXP
Date&Time       :2016/3/1 上午 10:32
Describe        :確認卡號及有效期，相當於Verifone inNCCC_FuncDisplaySwipeCardInfo
*/
int inFunc_CheckPAN_EXP(TRANSACTION_OBJECT *pobTran)
{
	
/*  UPT 因為不需要出現，要先拿掉,要再利用 20190529 [SAM]*/
#if _MACHINE_TYPE_ != _CASTLE_TYPE_UPT1000_		
	char		szDispMsg [40 + 1];
	char		szFinalPAN[_PAN_UCARD_SIZE_ + 1];
	char		szTemplate[20 + 1];
	unsigned char	uszKey;

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inFunc_CheckPAN_EXP 1!");

	
	/* 晶片卡不需要顯示有效期 */
	/* 感應卡不需要顯示有效期 */
	/* HGI卡不需要顯示有效期，跳過第二次讀取信用卡流程 */
	/* 一般HG交易不需要顯示有效期，跳過第二次讀取信用卡流程 */
	/* HG卡不需要顯示有效期 */
	/* MPAS刷卡不顯示有效期 */
	/* 票證無法顯示有效期 */
	/* 第二段收銀機連線不用顯示 */
	if (pobTran->srBRec.inChipStatus == _EMV_CARD_				||
	    pobTran->srBRec.uszFiscTransBit == VS_TRUE					||
	    pobTran->srBRec.uszContactlessBit == VS_TRUE				||
	    pobTran->srBRec.uszRefundCTLSBit == VS_TRUE				||
	    pobTran->srBRec.lnHGPaymentType == _HG_PAY_CREDIT_INSIDE_	||
	    pobTran->srBRec.uszHappyGoSingle == VS_TRUE				||
	    memcmp(pobTran->srBRec.szPAN, "9552", 4) == 0				||
	    pobTran->srTRec.uszESVCTransBit == VS_TRUE					||
	    pobTran->uszCardInquirysSecondBit == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	/* 顯示檢查碼 */
	else if (pobTran->uszInputCheckNoBit == VS_TRUE)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_CHECK_PAN_EXP_3_, 0, _COORDINATE_Y_LINE_8_4_);
		/* Disp Card Number */
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
		strcpy(szFinalPAN, pobTran->srBRec.szPAN);

		if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
		{
			inFunc_FormatPAN_UCARD(szFinalPAN);
		}
		sprintf(szDispMsg, " %s",szFinalPAN);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);

		/* Disp CheckNo */
		if (strlen(pobTran->srBRec.szExpDate) > 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate, _EXP_ENCRYPT_);

			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, " Check No.%s", szTemplate);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
		}
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_CHECK_PAN_EXP_, 0, _COORDINATE_Y_LINE_8_4_);
		/* Disp Card Number */
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
		strcpy(szFinalPAN, pobTran->srBRec.szPAN);

		if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
		{
			inFunc_FormatPAN_UCARD(szFinalPAN);
		}
		sprintf(szDispMsg, " %s",szFinalPAN);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);

		/* Disp Card Label */
		if (strlen(pobTran->srBRec.szExpDate) > 0)
		{
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, " MM/'YY = %.2s/'%.2s", pobTran->srBRec.szExpDate + 2, pobTran->srBRec.szExpDate);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
		}
	}

	uszKey = 0x00;
	while (1)
	{
		uszKey = uszKBD_GetKey(_ECR_RS232_GET_CARD_TIMEOUT_);

		if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
		{
			return (VS_ERROR);
		}
		else if (uszKey == _KEY_ENTER_)
		{
			break;
		}

	}
#endif
	return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckPAN_EXP_Loyalty_Redeem
Date&Time       :2017/2/3 下午 1:13
Describe        :確認卡號及有效期，優惠兌換用，請核對輸入資訊
*/
int inFunc_CheckPAN_EXP_Loyalty_Redeem(TRANSACTION_OBJECT *pobTran)
{
	char		szDispMsg [40 + 1];
	char		szFinalPAN[_PAN_UCARD_SIZE_ + 1];
	unsigned char	uszKey;

	/* 晶片卡不需要顯示有效期 */
	/* 第二段收銀機連線不用顯示 */
	if (pobTran->srBRec.inChipStatus == _EMV_CARD_	||
	    pobTran->srBRec.uszFiscTransBit == VS_TRUE	||
	    pobTran->srBRec.uszContactlessBit == VS_TRUE||
	    pobTran->uszCardInquirysSecondBit == VS_TRUE)
	{
		return (VS_SUCCESS);
	}

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_CHECK_PAN_EXP_2_, 0, _COORDINATE_Y_LINE_8_4_);
	/* Disp Card Number */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
	strcpy(szFinalPAN, pobTran->srBRec.szPAN);

	if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
	{
		inFunc_FormatPAN_UCARD(szFinalPAN);
	}
	sprintf(szDispMsg, " %s",szFinalPAN);
	inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);

	/* Disp Card Label */
	if (strlen(pobTran->srBRec.szExpDate) > 0)
	{
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, " MM/'YY = %.2s/'%.2s", pobTran->srBRec.szExpDate + 2, pobTran->srBRec.szExpDate);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	}

	uszKey = 0x00;
	while (1)
	{
		uszKey = uszKBD_GetKey(_ECR_RS232_GET_CARD_TIMEOUT_);

		if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
		{
			return (VS_ERROR);
		}
		else if (uszKey == _KEY_ENTER_)
		{
			break;
		}

	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckBarcode_Loyalty_Redeem
Date&Time       :2017/2/20 上午 10:06
Describe        :確認優惠兌換條碼，優惠兌換用，請核對輸入資訊
*/
int inFunc_CheckBarcode_Loyalty_Redeem(TRANSACTION_OBJECT *pobTran)
{
	char		szDispMsg [40 + 1];
	unsigned char	uszKey;


	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_CHECK_PAN_EXP_2_, 0, _COORDINATE_Y_LINE_8_4_);
	/* Disp Barcode1 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "%s",pobTran->szL3_Barcode1);
	inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_RED_, _DISP_LEFT_);

	/* Disp Barcode2 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "%s", pobTran->szL3_Barcode2);
	inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_8_, _COLOR_RED_, _DISP_LEFT_);

	uszKey = 0x00;
	while (1)
	{
		uszKey = uszKBD_GetKey(_ECR_RS232_GET_CARD_TIMEOUT_);

		if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
		{
			return (VS_ERROR);
		}
		else if (uszKey == _KEY_ENTER_)
		{
			break;
		}

	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckHGPAN_EXP
Date&Time       :2017/2/21 下午 5:09
Describe        :確認HG卡號及有效期
*/
int inFunc_CheckHGPAN_EXP(TRANSACTION_OBJECT *pobTran)
{
	char		szDispMsg [26 + 1];
	unsigned char	uszKey;

	/* 晶片卡不需要顯示有效期 */
        /* 感應卡不需要顯示有效期 */
        /* HGI卡不需要顯示有效期，跳過第二次讀取信用卡流程 */
        /* HG卡不需要顯示有效期 */
	if (pobTran->srBRec.inChipStatus == _EMV_CARD_ ||
            !memcmp(&pobTran->srBRec.szPAN[0], "9552", 4))
	{
		return (VS_SUCCESS);
	}

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_CHECK_PAN_EXP_, 0, _COORDINATE_Y_LINE_8_4_);
	/* Disp Card Number */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, " %s",pobTran->srBRec.szPAN);
	inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);

	/* Disp Card Label */
	if (strlen(pobTran->srBRec.szExpDate) > 0)
	{
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, " MM/'YY = %.2s/'%.2s", pobTran->srBRec.szExpDate + 2, pobTran->srBRec.szExpDate);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	}

	uszKey = 0x00;
	while (1)
	{
		uszKey = uszKBD_GetKey(_ECR_RS232_GET_CARD_TIMEOUT_);

		if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
		{
			return (VS_ERROR);
		}
		else if (uszKey == _KEY_ENTER_)
		{
			break;
		}

	}

	return (VS_SUCCESS);
}

/*
Function        : inFunc_Booting_Flow_Display_Initial
Date&Time   : 2017/10/2 下午 5:48
Describe        : 開機流程Display(螢幕)設定初始化
*/
int inFunc_Booting_Flow_Display_Initial(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = inDISP_Initial();

	inDISP_LogPrintfWithFlag("   DISP_INIT_REULT [%d] ", inRetVal);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Print_Initial
Date&Time       :2017/10/2 下午 5:48
Describe        :開機流程Print初始化
*/
int inFunc_Booting_Flow_Print_Initial(TRANSACTION_OBJECT *pobTran)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) == VS_SUCCESS)
	{
		int	inRetVal = VS_SUCCESS;

		/* 列印參數初始化 */
		inRetVal = inPRINT_Initial();
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_LogPrintfWithFlag("   inPRINT_Initial Result[%d] ", inRetVal);
			return (inRetVal);
		}

		inRetVal = inPRINT_TTF_SetFont(_PRT_CHINESE_1_, _FONT_REGULAR_);
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_LogPrintfWithFlag("   inPRINT_TTF_SetFont Result[%d] ", inRetVal);
			return (inRetVal);
		}
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Print_Image_Initial
Date&Time       :2018/6/5 下午 6:27
Describe        :開機流程列印圖片初始化，為了避免開機更新導致抓不到圖片高度
*/
int inFunc_Booting_Flow_Print_Image_Initial(TRANSACTION_OBJECT *pobTran)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 若沒下TMS會沒圖片抓高度，所以不判斷成功或失敗 */
	inPRINT_Buffer_GetHeightFlow();

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Castle_library_Initial
Date&Time       :2017/10/2 下午 5:48
Describe        :開機流程虹堡library初始化，虹堡有一些library要開機初始化
*/
int inFunc_Booting_Flow_Castle_library_Initial(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_Booting_Flow_Castle_library_Initial() START !");
	}

	/* KMS library initial */
	inRetVal = inKMS_Initial();
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}

	/* TLS library initial*/
	inRetVal = inTLS_Init();
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inFunc_Booting_Flow_Castle_library_Initial() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        : inFunc_Booting_Flow_Update_Parameter
Date&Time   : 2018/6/4 下午 8:45
Describe        : 可能是APP更新而進行重開機，
 * 所以重新開機時需要檢查是否有檔案要更新
 */
int inFunc_Booting_Flow_Update_Parameter(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	char		szCloseBatchBit[2 + 1] = {0};
//	char		szTemplate[2 + 1] = {0};
	unsigned char	uszUpdateBit = VS_FALSE;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 目前都先固定為FTP下載 2019/12/6 下午 2:40 [SAM] */
	pobTran->uszFTP_TMS_Download = 'Y';
	
	/* TMS參數生效 */
	/* 檢查 _TMSFLT_FILE_NAME_ 及  _FTPFLT_FILE_NAME_ 檔案是否存在  */
	/* 如果存在就設定對應的下載模式 */
	/* 代表有參數列表，有機會更新，接著檢查時間 */
	if (inEDCTMS_CheckTmsFileListExistFlow(pobTran) == VS_SUCCESS)
	{
		/* ISO8583 */
		if (pobTran->uszFTP_TMS_Download != 'Y')
		{
			/* 有File List檔案代表有參數要更新 */
			if (inEDCTMS_SCHDULE_CheckEffectiveDateTime(pobTran) == VS_SUCCESS)
			{
				/* 檢查時間到時，檢查FileList 及 下載檔案是否合法 */
				if (inEDCTMS_CheckAllDownloadFileMethodFlow(pobTran) != VS_SUCCESS)
					return (VS_SUCCESS);

				/* 檢查是否有帳 */
//				inLoadTMSCPTRec(0);
//				inGetTMSEffectiveCloseBatch(szCloseBatchBit);
//				if (memcmp(szCloseBatchBit, "Y", strlen("Y")) == 0)
//				{
					if (inFLOW_RunFunction(pobTran, _FUNCTION_All_HOST_MUST_SETTLE) != VS_SUCCESS)
					{
						return (VS_SUCCESS);
					}
//				}

				uszUpdateBit = VS_TRUE;
			}
		}
		/* FTPS */
		else
		{
//			if (inEDCTMS_SCHEDULE_CheckInquireDateTime() == VS_SUCCESS)
			{
				/* 檢查是否有帳 */
				/* 目前條件進不去，要再看 20190619 [SAM] */
				inLoadTMSFTPRec(0);
				inGetFTPEffectiveCloseBatch(szCloseBatchBit);
				if (memcmp(szCloseBatchBit, "Y", strlen("Y")) == 0)
				{
					if (inFLOW_RunFunction(pobTran, _FUNCTION_All_HOST_MUST_SETTLE) != VS_SUCCESS)
					{
						inTMSFTP_SetDowloadResponseCode(_FTP_DOWNLOAD_REPORT_EFFECT_FAIED_FOR_NOT_SETTLE_);
						inSaveTMSFTPRec(0);
						return (VS_SUCCESS);
					}
				}

				/* 會走到這邊代表已經有檔案了 */
				uszUpdateBit = VS_TRUE;
			}
		}
	}
	else
	{
		return (VS_SUCCESS);
	}

	/* 進行更新 */
	if (uszUpdateBit == VS_TRUE)
	{
		/* 這邊比Load Table還前面，所以先讀 */
		if((VS_SUCCESS != inLoadEDCRec(0)))
		{
			inDISP_DispLogAndWriteFlie("  Boot Updat Par Load Edc *Eerror* Hdt [0]  Line [%d]", __LINE__);
		}

		/* 因為公司TMS目前只有FTPS的功能，如果有別的下載方法，這邊需要進行分流 20190619 [SAM] */		
		inRetVal = inEDCTMS_FTP_FtpsUpdateFinalParameter(pobTran);
		
		if (inRetVal == VS_SUCCESS)
		{
			/* TMS更新成功 */
			inSetTMSOK("Y");
			/* 重置紀錄TSAM已註冊的開關*/
			inSetTSAMRegisterEnable("N");
			
#if _MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_
			inSetESCMode("N");
#endif		
			if((VS_SUCCESS != inSaveEDCRec(0))) 
			{
				inDISP_DispLogAndWriteFlie("  Boot Updat Par Save Edc *Eerror* Hdt [0]  Line [%d]", __LINE__);
			}
			
			/* 開啟銀聯及電簽功能 20190213 [SAM] */
			inFunc_FUBON_Check_Linkage_Function_Enable(pobTran);
			
			/* 確認是否接受CUP、SMARTPAY */
//			inFunc_Check_Linkage_Function_Enable(pobTran);

/* 目前用不到 20190327 [SAM] */
#if 0
			/* TMS連動DCC下載檢查，開機後根據DCCDownloadMode立即DCC下載 */
			if (inNCCC_DCC_TMS_Schedule_Check(pobTran) == VS_SUCCESS)
			{
				inSetDCCDownloadMode(_NCCC_DCC_DOWNLOAD_MODE_NOW_);
				inSaveEDCRec(0);
			}
#endif
			/* 更新TMS，DCC狀態重置 */
			inSetDCCSettleDownload("0");
			inSaveEDCRec(0);

			/* 要執行生效回報 */
			if (pobTran->uszFTP_TMS_Download == 'Y')
			{
				inLoadTMSFTPRec(0);
				inTMSFTP_SetDowloadResponseCode(_FTP_DOWNLOAD_REPORT_EFFECT_SUCCESS_);
				inEDCTMS_SaveSysDownloadDateTime(pobTran);
				/* 初始記錄狀態，用在回報時判斷  2020/1/14 下午 3:32 [SAM] */
				inSetFTPAutoDownloadFlag("00");
				/* 目前設定參數更新完成都要回報，如ISO格式無法回報需要修改 2019/12/4 上午 9:53 [SAM] */			
				inSetFTPEffectiveReportBit("Y");
				if (inSaveTMSFTPRec(0) < 0)
				{
					inDISP_DispLogAndWriteFlie("  Save TMSFPT *Error* Line [%d] END -----", __LINE__);
				}
			}
			else
			{
				inLoadTMSCPTRec(0);
				inSetTMSEffectiveReportBit("Y");
				inSaveTMSCPTRec(0);
			}
		}
		else
		{
			/* 更新失敗 刪除File List */
			inEDCTMS_DeleteTmsFileListExistFlow(pobTran);

			inSetTMSOK("N");

			if((VS_SUCCESS != inSaveEDCRec(0))) 
			{
				inDISP_DispLogAndWriteFlie("  Boot Updat Par Save Edc *Eerror* EDC [0]  Line [%d]", __LINE__);
			}
			
			
			/* 參數生效失敗 */
			if((VS_SUCCESS != inLoadTMSFTPRec(0))) 
			{
				inDISP_DispLogAndWriteFlie("  Boot Updat Par Load TMSFTP *Eerror*  ID[0]  Line [%d]", __LINE__);
			}
			
			
			inTMSFTP_SetDowloadResponseCode(_FTP_DOWNLOAD_REPORT_EFFECT_FAILED_);

			if((VS_SUCCESS != inSaveTMSFTPRec(0))) 
			{
				inDISP_DispLogAndWriteFlie("  Boot Updat Par Save TMSFTP *Eerror*  ID[0]  Line [%d]", __LINE__);
			}
		}
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}


/*
Function        : inFunc_Booting_Flow_Load_Table
Date&Time   : 2017/10/2 下午 5:48
Describe        : 開機流程load Table內的參數，以便執行接下來的流程
*/
int inFunc_Booting_Flow_Load_Table(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szHostLabel[8 + 1];
	char  szTmsOk[2];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* CFGT.dat */
	inRetVal = inLoadCFGTRec(0);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" Boot Load CFGT *Error* Line[%d]",  __LINE__);
	}

	/* EDC.dat */
	inRetVal = inLoadEDCRec(0);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" Boot Load EDC *Error* Line[%d]",  __LINE__);
	}
	
	inGetTMSOK(szTmsOk);
	
	if (szTmsOk[0] == 'Y')
	{
		/* 檢查版本是否為設定版本  */
		inRetVal = inLoadHDTRec(0);
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" Boot Load HDT *Error* Line[%d]",  __LINE__);
		}
	
		memset(szHostLabel, 0x00, sizeof(szHostLabel));
		inGetHostLabel(szHostLabel);

		if (memcmp(szHostLabel, _HOST_NAME_CREDIT_FUBON_, strlen(_HOST_NAME_CREDIT_FUBON_)))
		{
			inDISP_DispLogAndWriteFlie(" Boot Load Lable Fubon *Error* szHostLabel[%s] Line[%d]", szHostLabel,  __LINE__);
			inFunc_Check_EDCLock();
			return VS_ERROR;
		}	
	}

	/* 因為 FTP 上傳需要使用到 CPT 參數，所以預讀主要主機的參數 2019/12/18 下午 4:08 [SAM]  */
	inRetVal = inLoadCPTRec(0);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" Boot Load CPT *Error* Line[%d]",  __LINE__);
	}

#ifdef __SVC_ENABLE__	
	/* [新增SVC功能] PreLoad Data  [SAM] */
	inRetVal = inLoadSvcTableRec(0);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" Boot Load SvcTable *Error* Line[%d]",  __LINE__);
	}
#endif	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Sync_Debug
Date&Time       :2017/10/3 上午 9:13
Describe        :開機流程同步Table內的Debug Flag
*/
int inFunc_Booting_Flow_Sync_Debug(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_Booting_Flow_Sync_Debug() START !");
	}

	/* 將ISODebug和Table同步 */
	inRetVal = inFunc_Sync_Debug_Switch();
	inRetVal = inFunc_Sync_ISODebug_Switch();
	inRetVal = inFunc_Sync_DisplayDebug_Switch();
	inRetVal = inFunc_Sync_EngineerDebug_Switch();

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inFunc_Booting_Flow_Sync_Debug() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Communication_Initial
Date&Time       :2017/10/3 上午 9:42
Describe        :開機流程同步Table內的Debug Flag
*/
int inFunc_Booting_Flow_Communication_Initial(TRANSACTION_OBJECT *pobTran)
{
#ifdef _COMMUNICATION_CAPBILITY_
	int	inRetVal = VS_SUCCESS;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_Booting_Flow_Communication_Initial() START !");
	}

        /* 網路初始化，根據table選擇撥接或是Ethernet的初始化 */
	inRetVal = inCOMM_InitCommDevice();
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inFunc_Booting_Flow_Communication_Initial() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
#endif
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_ECR_Initial
Date&Time       :2017/10/3 上午 9:42
Describe        :開機流程初始化ECR用的port
*/
int inFunc_Booting_Flow_ECR_Initial(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_Booting_Flow_ECR_Initial() START !");
	}

	if (ginMachineType == _CASTLE_TYPE_V3UL_ ||
            ginMachineType == _CASTLE_TYPE_V3P_  ||
            ginMachineType == _CASTLE_TYPE_V3C_)
	{

	}
	else
	{
		/* ECR initial */
		inRetVal = inECR_Initial();
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inFunc_Booting_Flow_ECR_Initial() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_MultiFunc_Initial
Date&Time       :2017/10/3 上午 10:17
Describe        :開機流程初始化外接設備的port
*/
int inFunc_Booting_Flow_MultiFunc_First_Initial(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_Booting_Flow_MultiFunc_Initial() START !");
	}

	if (ginMachineType == _CASTLE_TYPE_V3UL_ ||
            ginMachineType == _CASTLE_TYPE_V3P_  ||
            ginMachineType == _CASTLE_TYPE_V3C_)
	{
		/* 多接設備initial */
		inRetVal = inMultiFunc_First_Initial();
	}


	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inFunc_Booting_Flow_MultiFunc_Initial() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_EMV_Initial
Date&Time       :2017/10/3 上午 10:28
Describe        :開機流程初始化EMV
*/
int inFunc_Booting_Flow_EMV_Initial(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szTMSOK[2 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_Booting_Flow_EMV_Initial() START !");
	}

	/* 確認是否鎖機 */
	/* TMS有下才跑下面*/
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (memcmp(szTMSOK, "Y", 1) == 0 && inFunc_Check_EDCLock() == VS_SUCCESS)
	{
		/* EMV Initial */
		inRetVal = inEMV_Initial();
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inFunc_Booting_Flow_EMV_Initial() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_TMSOK_Check_Initial
Date&Time       :2017/10/3 上午 11:05
Describe        :開機流程確認是否有下TMS或鎖機，只有這個開機流程會回傳錯誤，只要在這流程後的都可以視為一定要下TMS才能執行
*/
int inFunc_Booting_Flow_TMSOK_Check_Initial(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szTMSOK[2 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_Booting_Flow_TMSOK_Check_Initial() START !");
	}

	/* 確認是否鎖機 */
	/* TMS有下才跑下面*/
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (memcmp(szTMSOK, "Y", 1) == 0 && inFunc_Check_EDCLock() == VS_SUCCESS)
	{
                        inDISP_LogPrintf("TMS Not OK, or EDC LOCK");
	}
	else
	{
		if (memcmp(szTMSOK, "Y", 1) != 0)
		{
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("TMSOK != Y");

//			inDISP_ClearAll();
//			inDISP_Msg_BMP(_ERR_TMS_DWL_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}
		inRetVal = VS_ERROR;

	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("  InRetVal[%d]", inRetVal);
		inDISP_LogPrintf("inFunc_Booting_Flow_TMSOK_Check_Initial() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inFunc_Booting_Flow_CTLS_Initial
Date&Time       :2017/10/3 上午 11:05
Describe        :開機流程感應初始化
*/
int inFunc_Booting_Flow_CTLS_Initial(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szTMSOK[2 + 1];
	char	szCTLSEnable[2 + 1];		/* 觸控是否打開 */

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_Booting_Flow_CTLS_Initial() START !");
	}

	/* 確認是否鎖機 */
	/* TMS有下才跑下面*/
	/* 判斷是否有開感應 */
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (memcmp(szTMSOK, "Y", 1) == 0 && inFunc_Check_EDCLock() == VS_SUCCESS)
	{
		memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
		inGetContactlessEnable(szCTLSEnable);
		if (!memcmp(&szCTLSEnable[0], "Y", 1))
		{
			/* Contactless initial */
			inRetVal = inCTLS_InitReader_Flow();
		}
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inFunc_Booting_Flow_CTLS_Initial() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Sqlite_Initial
Date&Time       :2017/10/3 上午 11:05
Describe        :開機流程Sqlite初始化
*/
int inFunc_Booting_Flow_SQLite_Initial(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_Booting_Flow_Sqlite_Initial() START !");
	}

	/* SQLite Initial */
	inRetVal = inSqlite_Initial();

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inFunc_Booting_Flow_Sqlite_Initial() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_TSAM_Initial
Date&Time       :2017/10/3 下午 1:55
Describe        :開機流程tSAM初始化
*/
int inFunc_Booting_Flow_TSAM_Initial(TRANSACTION_OBJECT *pobTran)
{
//	int	inRetVal = VS_SUCCESS;
//	char	szTMSOK[2 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_Booting_Flow_TSAM_Initial() START !");
	}

	/* 確認是否鎖機 */
	/* TMS有下才跑下面*/
/* 目前用不到 有使用時再參考 20190327 [SAM] */	
#if 0	
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (memcmp(szTMSOK, "Y", 1) == 0 && inFunc_Check_EDCLock() == VS_SUCCESS)
	{
		/* tSAM initial */
		inRetVal = inNCCC_tSAM_InitialSLOT(pobTran);
	}
#endif

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inFunc_Booting_Flow_TSAM_Initial() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        : inFunc_Booting_Flow_TMS_Parameter_Inquire
Date&Time   : 2017/10/3 下午 2:38
Describe        : 開機流程TMS詢問
*/
int inFunc_Booting_Flow_TMS_Parameter_Inquire(TRANSACTION_OBJECT *pobTran)
{
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_TMS_DCC_Schedule
Date&Time       :2017/10/3 下午 2:59
Describe        :TMS 連動DCC下載
*/
int inFunc_Booting_Flow_TMS_DCC_Schedule(TRANSACTION_OBJECT *pobTran)
{
#ifdef	_COMMUNICATION_CAPBILITY_
//	int	inRetVal = VS_SUCCESS;
	char	szTMSOK[2 + 1];
//	char    szTemplate[16 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_Booting_Flow_TMS_DCC_Schedule() START !");
	}
	//vdDispHDPTRec(0);

	/* 確認是否鎖機 */
	/* TMS有下才跑下面*/
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (memcmp(szTMSOK, "Y", 1) == 0 && inFunc_Check_EDCLock() == VS_SUCCESS)
	{
		/* TMS連動DCC下載 */
/* 目前用不到 20190327 [SAM] */		
#if 0		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetDCCDownloadMode(szTemplate);
		if (memcmp(szTemplate, _NCCC_DCC_DOWNLOAD_MODE_NOW_, 1) == 0)
		{
			inRetVal = inEVENT_Responder(_TMS_DCC_SCHEDULE_EVENT_);
		}
#endif
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inFunc_Booting_Flow_TMS_DCC_Schedule() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

#endif
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Enter_PowerOn_Password
Date&Time       :2017/10/3 下午 3:33
Describe        :輸入開機密碼
*/
int inFunc_Booting_Flow_Enter_PowerOn_Password(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 輸入開機密碼 */
	while (1)
	{
		inRetVal = inFunc_CheckCustomizePassword(_ACCESS_WITH_CUSTOM_, _POWER_ON_);
		if (inRetVal != VS_SUCCESS)
			continue;
		else
			break;
	}
	
	/* 為了清除開機前送的資料 2019/0807 [SAM] */
	inECR_FlushRxBuffer();
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_CUP_LOGON
Date&Time       :2017/10/3 下午 3:53
Describe        :安全認證
*/
int inFunc_Booting_Flow_CUP_LOGON(TRANSACTION_OBJECT *pobTran)
{
        inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
        inDISP_LogPrintfWithFlag("  RunOperationId [%d]",pobTran->inRunOperationID);
	
#ifdef _COMMUNICATION_CAPBILITY_

        int	inRetVal = VS_SUCCESS;
        char	szHostName[8 + 1];

	
	if (inLoadHDTRec(0) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" FUNC Bootiong Cup Logon Load HDT[0] *Error* Line[%d]", __LINE__);
	}

	memset(szHostName, 0x00, sizeof(szHostName));
	inGetHostLabel(szHostName);
        inDISP_LogPrintfWithFlag("  HostName [%s]", szHostName);        
        
#warning " 寫入 master key 的地方 "
        
	/*TODO: 看能不能寫成可變動功能 */
	if (memcmp(szHostName, _HOST_NAME_CREDIT_FUBON_, strlen(_HOST_NAME_CREDIT_FUBON_)) == 0)
	{
		inRetVal = inFUBON_ISO_CUP_FuncAutoLogon(pobTran);
		
	}else
	{
		inDISP_DispLogAndWriteFlie("  Boot FUBON Logon *Error* szHostName[%s] Line[%d]", szHostName,  __LINE__);
	}

#endif /* END _COMMUNICATION_CAPBILITY_ */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Clear_AP_Dump
Date&Time       :2018/6/19 下午 1:57
Describe        :清空AP下載的暫存檔案
*/
int inFunc_Booting_Flow_Clear_AP_Dump(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_Booting_Flow_Clear_AP_Dump() START !");
	}

	inRetVal = inFunc_Clear_AP_Dump();

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inFunc_Booting_Flow_Clear_AP_Dump() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Process_Cradle
Date&Time       :2018/3/8 上午 10:50
Describe        :測試中
*/
int inFunc_Booting_Flow_Process_Cradle(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_Booting_Flow_Process_Cradle() START !");
	}

	if (ginMachineType == _CASTLE_TYPE_V3M_)
	{
		inRetVal = inBluetooth_Default_Cradle_Connect();
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inFunc_Booting_Flow_Process_Cradle() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_PowerManagement
Date&Time       :2018/3/14 上午 10:40
Describe        :經測試V3M沒辦法用auto power management，廢棄(2018/3/14 下午 5:32)
*/
int inFunc_Booting_Flow_PowerManagement(TRANSACTION_OBJECT *pobTran)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inFunc_Booting_Flow_PowerManagement() START !");
	}

//#if	_MACHINE_TYPE_ == _CASTLE_TYPE_V3M_
//
//	inPWM_PowerAutoModeEnable();
//	/* 避免進入Standby Mode */
//	inPWM_PowerAutoMode_Standby_Time_Set(0);
//	inPWM_PowerAutoMode_Sleep_Time_Set(60);
//
//#endif

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inFunc_Booting_Flow_PowerManagement() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Reprint_Poweroff
Date&Time       :2018/3/14 上午 10:40
Describe        :檢查是否有未簽交易，要重印簽單
*/
int inFunc_Booting_Flow_Reprint_Poweroff(TRANSACTION_OBJECT *pobTran)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) == VS_SUCCESS)
	{
		inNEXSYS_ESC_Process_PowerOff_When_Signing(pobTran);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Update_Success_Report
Date&Time       :2018/6/5 上午 9:26
Describe        :一定要放最後一個流程，生效回報
*/
int inFunc_Booting_Flow_Update_Success_Report(TRANSACTION_OBJECT *pobTran)
{
	char 	szTemplate[3], szTMSReportBit[3];
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTMSDownloadMode(szTemplate);
	
	inDISP_DispLogAndWriteFlie("  TMS DonwloadMode [%s] Line[%d]",szTemplate, __LINE__);
	
	if (memcmp(szTemplate, _TMS_DLMODE_FTPS_, strlen(_TMS_DLMODE_FTPS_)) == 0)
	{
                    
		inLoadTMSFTPRec(0);
		memset(szTMSReportBit, 0x00, sizeof(szTMSReportBit));
		inGetFTPEffectiveReportBit(szTMSReportBit);
                    inDISP_DispLogAndWriteFlie("  FTP ReportBit [%s] Line[%d]",szTMSReportBit, __LINE__);
		if (memcmp(szTMSReportBit, "Y", strlen("Y")) == 0)
		{
			/* 執行生效回報 */
			/* 參數生效狀態回報 */
			inEDCTMS_ReturnTheTmsReport(pobTran);
			inSetFTPEffectiveReportBit("N");
			inSaveTMSFTPRec(0);
		}
	}
	else if (memcmp(szTemplate, _TMS_DLMODE_ISO8583_, strlen(_TMS_DLMODE_ISO8583_)) == 0)
	{
		inLoadTMSCPTRec(0);
                   memset(szTMSReportBit, 0x00, sizeof(szTMSReportBit));
		inGetTMSEffectiveReportBit(szTMSReportBit);
		if (memcmp(szTMSReportBit, "Y", strlen("Y")) == 0)
		{
			/* 執行生效回報 */
			/* 參數生效狀態回報 */
			inEDCTMS_ReturnTheTmsReport(pobTran);

			inSetTMSEffectiveReportBit("N");
			inSaveTMSCPTRec(0);
		}
	}
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        : inFunc_Booting_Flow_TMS_Download
Date&Time   : 2018/6/25 上午 4:58
Describe        : 開機執行，TMS下載
*/
int inFunc_Booting_Flow_TMS_Download(TRANSACTION_OBJECT *pobTran)
{
	return (VS_SUCCESS);
}


/*
Function        :inFunc_Booting_Flow_RTC_Millisecond_Correction
Date&Time       :2019/6/26 上午 11:26
Describe        :開機流程校正RTC毫秒
 *  新增電票 log使用方法 2022/6/7  [SAM] 
*/
int inFunc_Booting_Flow_RTC_Millisecond_Correction(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_RTC_Millisecond_Correction() START !");
	}
	
	inRetVal = inDISP_RTC_Tick_Correction();
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_RTC_Millisecond_Correction() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}


int inFunc_TWNAddDataToEMVPacket(TRANSACTION_OBJECT *pobTran, unsigned short usTag, unsigned char *pbtPackBuff)
{
	int	offset = 0;
	BYTE	btTmp[128 + 1];
	BYTE	btTVR[5 + 1];
	char	szDefaultTAC[10 + 1];
	char	szDenialTAC[10 + 1];
	char	szOnlineTAC[10 + 1];
//        char    szNCCCFESMode[2 + 1];
        char	szDebugMsg[100 + 1];
	unsigned short usLen;

        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("inFunc_TWNAddDataToEMVPacket START!!");
                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                sprintf(szDebugMsg, "inFunc_TWNAddDataToEMVPacket(0x%02X)", usTag);
                inDISP_LogPrintf(szDebugMsg);
        }

	if ((usTag & 0xDF00) == 0xDF00)
	{
		pbtPackBuff[offset++] = (BYTE) (usTag >> 8);
		pbtPackBuff[offset++] = (BYTE) (usTag & 0x00FF);

		if ((usTag & 0x00FF) == 0x0091)
		{
			memcpy(&pbtPackBuff[offset], &ISS_SCRIPT_RES, ISS_SCRIPT_RES[0] + 1);
			offset += (ISS_SCRIPT_RES[0] + 1);

			return (offset);
		}

		/* Tag DFEC */
		if ((usTag & 0x00FF) == 0x00EC)
		{
			pbtPackBuff[offset ++] = 0x01;
			pbtPackBuff[offset ++] = 0x01;

			return (offset);
		}

		/* Tag DFED */
		if ((usTag & 0x00FF) == 0x00ED)
		{
			pbtPackBuff[offset ++] = 0x01;
			pbtPackBuff[offset ++] = 0x02;
			return (offset);
		}

		/* Tag DFEE */
		if ((usTag & 0x00FF) == 0x00EE)
		{
			pbtPackBuff[offset++] = 0x01;
			pbtPackBuff[offset++] = 0x05;

			return (offset);
		}

		/*  Tag DFEF */
		if ((usTag & 0x00FF) == 0x00EF)
		{
                        memcpy(btTVR, pobTran->srEMVRec.usz95_TVR, pobTran->srEMVRec.in95_TVRLen);
                        inGetDefaultTAC(szDefaultTAC);
                        inGetOnlineTAC(szOnlineTAC);
                        inGetDenialTAC(szDenialTAC);

			if (strlen(szDefaultTAC))
			{
                                inFunc_ASCII_to_BCD((BYTE *)TACDEFAULT, szDefaultTAC, 5);
                                inFunc_ASCII_to_BCD((BYTE *)TACDEFAULT, szDenialTAC, 5);
                                inFunc_ASCII_to_BCD((BYTE *)TACDEFAULT, szOnlineTAC, 5);
			}

			pbtPackBuff[offset ++] = 0x02;

			/*******************************************************************************
			1. 1504: fallback tx.
			2. 1511: merchant suspicious.  Merchant force online.
			3. 1503: random selection.  If both random selection bit of TVR and TAC are set.
			4. 1510: over floor limit.  If both floor limit bit of TVR and TAC are set.
			5. 1508: any other bits of TVR and TAC are set
			6. 1505: icc.  If ARQC return from card.
			********************************************************************************/

			if ((btTVR[3] & 0x10) && (TACONLINE[3] & 0x10))
			{
				/* Random select online */
				pbtPackBuff[offset ++] = 0x15;
				pbtPackBuff[offset ++] = 0x03;
			}
			else if (pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
			{
				pbtPackBuff[offset ++] = 0x15;
				pbtPackBuff[offset ++] = 0x04;
			}
			else if ((btTVR[3] & 0x80) && (TACONLINE[3] & 0x80))
			{
				/* Over floor limit.  If both floor limit bit of TVR and TAC are set. */
				pbtPackBuff[offset ++] = 0x15;
				pbtPackBuff[offset ++] = 0x10;
			}
			else if (btTVR[3] & 0x01)
			{
				/* (Online force by terminal)Merchant force online */
				pbtPackBuff[offset ++] = 0x15;
				pbtPackBuff[offset ++] = 0x11;
			}
			else if ((btTVR[0] & TACONLINE[0]) &&
				 (btTVR[1] & TACONLINE[1]) &&
				 (btTVR[2] & TACONLINE[2]) &&
				 (btTVR[3] & TACONLINE[3]) &&
				 (btTVR[4] & TACONLINE[4]))
			{
				/* any other bits of TVR and TAC are set */
				pbtPackBuff[offset ++] = 0x15;
				pbtPackBuff[offset ++] = 0x08;
			}
			else
			{
				/* (Online force by card ARQC) */
				pbtPackBuff[offset ++] = 0x15;
				pbtPackBuff[offset ++] = 0x05;
			}

			return (offset);
		}

		return (0);
	}
	else
	{
		/* 9FXX */
		if ((usTag & 0xFFFF) == 0x9F5B)
		{
			memcpy( &pbtPackBuff[offset], &ISS_SCRIPT_RES, ISS_SCRIPT_RES[0] + 1);
			offset += (ISS_SCRIPT_RES[0] + 1);

			return (offset);
		}
		else
		{
                        if (EMV_DataGet(usTag, &usLen, btTmp) != d_EMVAPLIB_OK)
                        {
                                return (0);
                        }

			if ((usTag & 0xFFFF) == 0x4F00)
				pbtPackBuff[offset ++] = 0x84;
			else
			{
				if ((usTag & 0x1F00) == 0x1F00)
				{	/* 2-byte tag */
					pbtPackBuff[offset ++] = (BYTE) (usTag >> 8);
					pbtPackBuff[offset ++] = (BYTE) (usTag & 0x00FF);
				}
				else
				{
					pbtPackBuff[offset ++] = (BYTE) (usTag >> 8);
				}
			}

			pbtPackBuff[offset++] = (BYTE) usLen;
			memcpy(pbtPackBuff + offset, btTmp, usLen);

			return (offset + (int )usLen);
		}
	}
}

/*
Function        :inFunc_Set_Temp_VersionID
Date&Time       :2017/1/12 上午 10:25
Describe        :設定臨時的Terminal Version ID
*/
int inFunc_Set_Temp_VersionID()
{
	int		inRetVal;
	char		szTerminalVersionID[15 + 1];	/* 檔案名稱最多15字*/
        DISPLAY_OBJECT  srDispObj;

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 16;
	srDispObj.inColor = _COLOR_RED_;
	srDispObj.inTimeout = 30;

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	memset(szTerminalVersionID, 0x00, sizeof(szTerminalVersionID));
	if (strlen(gszTermVersionID) > 0)
	{
		memcpy(szTerminalVersionID, gszTermVersionID, strlen(gszTermVersionID));
	}
	else
	{
		inGetTermVersionID(szTerminalVersionID);
	}
	inDISP_ChineseFont_Color("原VersonID:", _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_, _DISP_LEFT_);
	inDISP_ChineseFont_Color(szTerminalVersionID, _FONTSIZE_16X22_, _LINE_16_9_, _COLOR_RED_, _DISP_LEFT_);
	/* Set Version ID */
        inDISP_ChineseFont_Color("設定 Version ID:", _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);

	srDispObj.inOutputLen = strlen(szTerminalVersionID);
	memcpy(srDispObj.szOutput, szTerminalVersionID, srDispObj.inOutputLen);
	inDISP_EnglishFont_Color(srDispObj.szOutput, _FONTSIZE_8X22_, _LINE_8_7_, srDispObj.inColor, _DISP_RIGHT_);

        inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (VS_ERROR);

	memset(gszTermVersionID, 0x00, sizeof(gszTermVersionID));
	memcpy(gszTermVersionID, &srDispObj.szOutput[0], srDispObj.inMaxLen);

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Check_Version_ID
Date&Time       :2017/1/18 上午 11:42
Describe        :可確認Version
 *	端帶
	商代
	s/n
	日期
*/
int inFunc_Check_Version_ID()
{
	char		szDispMsg[50 + 1];
	unsigned char	uszKey;
#if _MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_
	int inVerLen, inLen, inLine = _LINE_8_4_;
	char szTempBuf[32];
		
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        memset(szTempBuf, 0x00, sizeof(szTempBuf));	
    
/* 20230210 Miyano marked 不知道用途，暫時先註解掉 */
#ifdef _ESUN_MAIN_HOST_
//        memcpy(szTempBuf,gszEsunTermVersionId,strlen(gszEsunTermVersionId));
#else
//        memcpy(szTempBuf,gszCathayTermVersionId,strlen(gszCathayTermVersionId));
#endif    
    
	if(strlen(szTempBuf) > 16)
	{
		
		inVerLen = strlen(szTempBuf);
		
		inDISP_ChineseFont("版本名稱", _FONTSIZE_8X16_, inLine, _DISP_LEFT_);
		inLine ++;
		
		for( inLen = 0; inLen < inVerLen ; inLen += 16 )
		{
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memcpy(szDispMsg, &szTempBuf[inLen], 16); 
			inDISP_EnglishFont(szDispMsg , _FONTSIZE_8X16_, inLine, _DISP_LEFT_);
			inLine ++;
		}

	}else
	{
		inDISP_ChineseFont("版本名稱", _FONTSIZE_8X16_, inLine, _DISP_LEFT_);
		inLine ++;
		inDISP_EnglishFont(szTempBuf, _FONTSIZE_8X16_, inLine, _DISP_LEFT_);
	}
	
	while (1)
	{
		uszKey = uszKBD_GetKey(30);

		if (uszKey == _KEY_ENTER_)
		{
			break;
		}
		else if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
		{
			return (VS_ERROR);
		}
		else
		{
			continue;
		}
	}
	
	inLine = _LINE_8_4_;
		
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	
	inLoadHDTRec(0);
	
	memset(szTempBuf, 0x00, sizeof(szTempBuf));
	inGetTerminalID(szTempBuf);
	inDISP_ChineseFont("端未機代號", _FONTSIZE_8X16_, inLine, _DISP_LEFT_);
	inLine ++;
	inDISP_EnglishFont(szTempBuf, _FONTSIZE_8X16_, inLine, _DISP_LEFT_);
	inLine ++;
	
	memset(szTempBuf, 0x00, sizeof(szTempBuf));
	inGetMerchantID(szTempBuf);
	inDISP_ChineseFont("商店代號", _FONTSIZE_8X16_, inLine, _DISP_LEFT_);
	inLine ++;
	inDISP_EnglishFont(szTempBuf, _FONTSIZE_8X16_, inLine, _DISP_LEFT_);
	inLine ++;
	
	while (1)
	{
		uszKey = uszKBD_GetKey(30);

		if (uszKey == _KEY_ENTER_)
		{
			break;
		}
		else if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
		{
			return (VS_ERROR);
		}
		else
		{
			continue;
		}
	}
	
	inLine = _LINE_8_4_;
		
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	
	memset(szTempBuf, 0x00, sizeof(szTempBuf));
	inFunc_GetSeriaNumber(szTempBuf);
	szTempBuf[15]= 0x00;
	inDISP_ChineseFont("S/N", _FONTSIZE_8X16_, inLine, _DISP_LEFT_);
	inLine ++;
	inDISP_EnglishFont(szTempBuf, _FONTSIZE_8X16_, inLine, _DISP_LEFT_);
	inLine ++;
	inDISP_ChineseFont("嘉利版本日期", _FONTSIZE_8X16_, inLine, _DISP_LEFT_);
	inLine ++;
	inDISP_EnglishFont(gszTermVersionDate, _FONTSIZE_8X16_, inLine, _DISP_LEFT_);
	
	while (1)
	{
		uszKey = uszKBD_GetKey(30);

		if (uszKey == _KEY_ENTER_)
		{
			break;
		}
		else if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
		{
			return (VS_ERROR);
		}
		else
		{
			continue;
		}
	}
	
#else
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	
	inDISP_PutGraphic(_CHECK_VERSION_, 0, _COORDINATE_Y_LINE_8_4_);

	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	if (strlen(gszTermVersionID) > 0)
	{
		memcpy(szDispMsg, gszTermVersionID, strlen(gszTermVersionID));
	}
	else
	{
		inGetTermVersionID(szDispMsg);
	}
	inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	if (strlen(gszTermVersionDate) > 0)
	{
		memcpy(szDispMsg, gszTermVersionDate, strlen(gszTermVersionDate));
	}
	else
	{
		inGetTermVersionDate(szDispMsg);
	}
	inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
	
	while (1)
	{
		uszKey = uszKBD_GetKey(30);

		if (uszKey == _KEY_ENTER_)
		{
			break;
		}
		else if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
		{
			return (VS_ERROR);
		}
		else
		{
			continue;
		}
	}

#endif	
	
	

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Delete_Signature
Date&Time       :2017/3/22 上午 10:21
Describe        :砍簽名圖檔
*/
int inFunc_Delete_Signature(TRANSACTION_OBJECT *pobTran)
{
	char	szSignature[16 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 商店聯印成功就砍簽名圖檔 */
	memset(szSignature, 0x00, sizeof(szSignature));
	/* 因為用invoice所以不用inFunc_ComposeFileName */
	inFunc_ComposeFileName_InvoiceNumber(pobTran, szSignature, _PICTURE_FILE_EXTENSION_, 6);
	/* 砍簽名圖檔 */
	if(VS_SUCCESS != inFILE_Delete((unsigned char*)szSignature))
	{
		inDISP_DispLogAndWriteFlie("  Func Delete Sign *Error*");
	}

	inDISP_LogPrintfWithFlag("  Delete Signature[%s]", szSignature);

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inFunc_DuplicateSave
Date&Time       :2017/3/30 下午 4:23
Describe        :檢核是否重複刷卡
*/
int inFunc_DuplicateSave(TRANSACTION_OBJECT* pobTran)
{
	char	szSplitTxnCheckEnable[2 + 1];
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inFunc_DuplicateSave INIT" );

	memset(szSplitTxnCheckEnable, 0x00, sizeof(szSplitTxnCheckEnable));
	inGetSplitTransCheckEnable(szSplitTxnCheckEnable);
	if (memcmp(szSplitTxnCheckEnable, "Y", strlen("Y")) != 0)
	{
		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inFunc_DuplicateSave NoEnable END");
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d]  NoEnable END -----",__FILE__, __FUNCTION__, __LINE__);
   		return (VS_SUCCESS);
	}

	/* 我覺得應該是不會有人刷了之後馬上重開機再刷一次，所以直接存global */
	if (pobTran->inTransactionCode == _VOID_			||
	    pobTran->inTransactionCode == _REFUND_			||
	    pobTran->inTransactionCode == _INST_REFUND_			||
	    pobTran->inTransactionCode == _REDEEM_REFUND_		||
	    pobTran->inTransactionCode == _CUP_REFUND_			||
	    pobTran->inTransactionCode == _CUP_MAIL_ORDER_REFUND_	||
	    pobTran->inTransactionCode == _CUP_VOID_)
	{

	}
	else
	{
		memcpy(gszDuplicatePAN, pobTran->srBRec.szPAN, sizeof(pobTran->srBRec.szPAN));
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inFunc_DuplicateSave END");

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inFunc_DuplicateCheck
Date&Time       :2017/3/30 下午 4:23
Describe        :檢核是否重複刷卡
*/
int inFunc_DuplicateCheck(TRANSACTION_OBJECT* pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szSplitTxnCheckEnable[2 + 1];

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 功能有開才檢核 */
	if (memcmp(szSplitTxnCheckEnable, "Y", strlen("Y")) == 0)
	{
		if (pobTran->inTransactionCode == _SALE_		||
		    pobTran->inTransactionCode == _SALE_OFFLINE_	||
		    pobTran->inTransactionCode == _FORCE_CASH_ADVANCE_	||
		    pobTran->inTransactionCode == _INST_SALE_		||
		    pobTran->inTransactionCode == _REDEEM_SALE_		||
		    pobTran->inTransactionCode == _CUP_SALE_		||
		    pobTran->inTransactionCode == _REDEEM_ADJUST_	||
		    pobTran->inTransactionCode == _INST_ADJUST_)
		{
			if (!memcmp(gszDuplicatePAN, pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN)))
			{
				inDISP_Msg_BMP("", 0, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "此卡號與前筆重覆", _LINE_8_6_);	/* 127 = 此卡號與前筆重覆 */
				inRetVal = VS_ERROR;
			}

		}

	}

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] Rval[%d] END -----", __FILE__, __FUNCTION__, __LINE__, inRetVal);
	return (inRetVal);
}

/*
Function        :inFunc_SHA256
Date&Time       :2017/4/5 上午 9:19
Describe        :
*/
int inFunc_SHA256(unsigned char* uszInData, unsigned int uiInDataLen, unsigned char* uszOutData)
{
	CTOS_SHA256_CTX	srSHA256;

	CTOS_SHA256Init(&srSHA256);

	CTOS_SHA256Update(&srSHA256, uszInData, uiInDataLen);

	CTOS_SHA256Final(&srSHA256, uszOutData);

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Base64_Encryption
Date&Time       :2017/4/5 上午 10:25
Describe        :(Verifone NCCCfunc.c:31114 base64_encode)
*/
int inFunc_Base64_Encryption(char* szInData, int inInDataLen, char* szOutData)
{
	char		*szInDatap = szInData;
	char		*top = szOutData;
	char		end[3];
	char		b64string[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	unsigned char	cbyte;
	unsigned char	obyte;

	for(; inInDataLen >= 3; inInDataLen -= 3)
	{
		cbyte = *szInDatap++;
		*top++ = b64string[(int)(cbyte >> 2)];
		obyte = (cbyte << 4) & 0x30;		/* 0011 0000 */

		cbyte = *szInDatap++;
		obyte |= (cbyte >> 4);			/* 0000 1111 */
		*top++ = b64string[(int)obyte];
		obyte = (cbyte << 2) & 0x3C;		/* 0011 1100 */

		cbyte = *szInDatap++;
		obyte |= (cbyte >> 6);			/* 0000 0011 */
		*top++ = b64string[(int)obyte];
		*top++ = b64string[(int)(cbyte & 0x3F)];/* 0011 1111 */
	}

	if(inInDataLen)
	{
		end[0] = *szInDatap++;
		if (--inInDataLen) end[1] = *szInDatap++; else end[1] = 0;
		end[2] = 0;

		cbyte = end[0];
		*top++ = b64string[(int)(cbyte >> 2)];
		obyte = (cbyte << 4) & 0x30;		/* 0011 0000 */

		cbyte = end[1];
		obyte |= (cbyte >> 4);
		*top++ = b64string[(int)obyte];
		obyte = (cbyte << 2) & 0x3C;		/* 0011 1100 */

		if (inInDataLen) *top++ = b64string[(int)obyte];
		else *top++ = '=';
		*top++ = '=';
	}

	*top = 0;

	return (VS_SUCCESS);
}


long base64_decode(char *to, char *from, int len)
{
	char *fromp = from;
	char *top = to;
	char *p;
	unsigned char cbyte;
	unsigned char obyte;
	int padding = 0;

	for (; len >= 4; len -= 4)
	{
		if ((cbyte = *fromp++) == '=')
			cbyte = 0;
		else
		{
			badchar(cbyte, p);

			if (!p)
				return -1;

			cbyte = (p - b64string);
		}

		obyte = cbyte << 2;		/* 1111 1100 */

		if ((cbyte = *fromp++) == '=')
			cbyte = 0;
		else
		{
			badchar(cbyte, p);

			if (!p)
				return -1;

			cbyte = p - b64string;
		}

		obyte |= cbyte >> 4;		/* 0000 0011 */
		*top++ = obyte;

		obyte = cbyte << 4;
				/* 1111 0000 */
		if ((cbyte = *fromp++) == '=')
		{
			cbyte = 0; padding++;
		}
		else
		{
			padding = 0;
			badchar (cbyte, p);

			if (!p)
				return -1;

			cbyte = p - b64string;
		}

		obyte |= cbyte >> 2;		/* 0000 1111 */
		*top++ = obyte;

		obyte = cbyte << 6;		/* 1100 0000 */

		if ((cbyte = *fromp++) == '=')
		{
			cbyte = 0; padding++;
		}
		else
		{
			padding = 0;
			badchar(cbyte, p);

			if (!p)
				return -1;

			cbyte = p - b64string;
		}

		obyte |= cbyte;			/* 0011 1111 */
		*top++ = obyte;
	}

	*top = 0;

	if (len)
		return -1;

	return (top - to) - padding;
}

int inFunc_Base64_Decryption(int inLenth, char *szInputData, char *szOutputData)
{
	int  inLen;
	char szOutData[1024];

	memset(szOutData, 0x00, sizeof(szOutData));
	inLen = base64_decode(szOutData, szInputData, inLenth);

	memcpy(&szOutputData[0], &szOutData[0], inLen);
	return (inLen);
}

/*
Function        :inFunc_Update_AP
Date&Time       :2017/5/8 下午 2:54
Describe        :
*/
int inFunc_Update_AP(char* szPathName)
{
	int	inRetVal = VS_ERROR;
	PRESERVE_NEXSYS	srPreserve;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
		
	inDISP_LogPrintfWithFlag("  Upap Name[%s] ", szPathName);
	
	/* 保存資料 */
	inFunc_UpdateAP_Preserve(&srPreserve);

	inRetVal = CTOS_UpdateFromMMCIEx((unsigned char *)szPathName, &callbackFun);

	inDISP_LogPrintfWithFlag("  MMCIex inRetVal = 0x%04X", inRetVal);

	inRetVal = CTOS_UpdateGetResult();
	if (inRetVal != d_CAP_UPDATE_FINISHED)
	{
		inDISP_DispLogAndWriteFlie("  UpdateGet inRetVal[0x%04X] Line[%d]", inRetVal, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		/* 更新成功 */
		/* 回覆資料 */
		inFunc_UpdateAP_Recover(&srPreserve);

	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Idle_CheckCustomPassword_Flow
Date&Time       :2017/6/8 下午 2:51
Describe        :
*/
int inFunc_Idle_CheckCustomPassword_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;

	/* 第一層輸入密碼 */
	inRetVal = inFunc_CheckCustomizePassword(_ACCESS_WITH_CUSTOM_, pobTran->srBRec.inCode);

	return (inRetVal);
}

/*
Function        :inFunc_Change_EMVForceOnline
Date&Time       :2017/9/4 上午 10:58
Describe        :
*/
int inFunc_Change_EMVForceOnline()
{
	char	szEMVForceOnline[2 + 1];
	char	szDispMsg[50 + 1];

	memset(szEMVForceOnline, 0x00, sizeof(szEMVForceOnline));
	inGetEMVForceOnline(szEMVForceOnline);

	if (memcmp(szEMVForceOnline, "N", strlen("N")) == 0)
	{
		memcpy(szEMVForceOnline, "Y", strlen("Y"));
	}
	else
	{
		memcpy(szEMVForceOnline, "N", strlen("N"));
	}

	inSetEMVForceOnline(szEMVForceOnline);
	inSaveEDCRec(0);

	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "Force Online: %s", szEMVForceOnline);
	inDISP_Msg_BMP("", 0, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, szDispMsg, _LINE_8_6_);

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Check_LRC
Date&Time       :2017/9/19 下午 4:05
Describe        :
 *		szbuff填陣列位置
 *		inLen填字串長度
 *		szOutLRC輸出的字元
*/
int inFunc_Check_LRC(char *szbuff, int inLen, char *szOutLRC)
{
	int	i = 0;

	/* 先清為0 */
	*szOutLRC = 0x00;

	for (i = 0; i < inLen; i++)
	{
		*szOutLRC = *szOutLRC ^ szbuff[i];
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Start_Display_Time_Thread
Date&Time       :2017/12/8 下午 4:57
Describe        :廢棄（2017/12/8 下午 5:42）
*/
int inFunc_Start_Display_Time_Thread()
{
//	pthread_t		uliId;
//
//	pthread_create(&uliId, NULL, (void*)vdFunc_Display_Time, NULL);

	return (VS_SUCCESS);
}

/*
Function		:vdFunc_Display_All_Status
Date&Time	:2018/3/9 上午 11:47
Describe		:
 * 控制顯示在刷卡機第一行的狀態列，
 * szFunEnable 欄位表示功能開關
 * 1:日期時間 2:連線狀態 3.電量 4.Wifi通訊狀態
*/
int inFunc_Display_All_Status(char* szFunEnable)
{
	int	inOffset = 0;
	int	inStatusNum = 3;

	if (strlen(szFunEnable) < inStatusNum)
	{
		return (VS_ERROR);
	}

	/* 檢查是否需更新時間 */
	/* 檢查TIMEOUT，至少一秒才顯示差別 */
	if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
	{
		/* 顯示時間 */
		if (strlen(szFunEnable) >= 1)
		{
			inOffset = 0;
			if (*(szFunEnable + inOffset) == '1')
			{
				inFunc_Display_Time();
			}
		}

		/* 顯示連線狀態 */
		if (strlen(szFunEnable) >= 2)
		{
			inOffset = 1;
			if (*(szFunEnable + inOffset) == '1')
			{
				vdFunc_Display_Ethernet_Status();
			}
		}

		/* 顯示電量 */
		if (strlen(szFunEnable) >= 3)
		{
			inOffset = 2;
			if (*(szFunEnable + inOffset) == '1')
			{
				vdFunc_Display_Battery_Status();
			}
		}

		/* 顯示WiFi收訊 260~ 290 */
		if (strlen(szFunEnable) >= 4)
		{
			inOffset = 3;
			if (*(szFunEnable + inOffset) == '1')
			{
				vdFunc_Display_WiFi_Quality();
			}
		}

		/* 顯示SIM卡收訊 290 ~ 320*/
		if (strlen(szFunEnable) >= 5)
		{
			inOffset = 4;
			if (*(szFunEnable + inOffset) == '1')
			{
				vdFunc_Display_SIM_Quality();
			}
		}

		inDISP_Timer_Start(_TIMER_NEXSYS_2_, 1);
	}

	return (VS_SUCCESS);
}

/*
Function		: inFunc_Display_All_Status_By_Machine_Type
Date&Time	: 2018/5/21 上午 10:51
Describe		:
*/
int inFunc_Display_All_Status_By_Machine_Type(void)
{
	int	inRetVal = VS_ERROR;

	if (ginMachineType == _CASTLE_TYPE_V3C_)
	{
		inRetVal = inFunc_Display_All_Status("11011");
	}
	else if (ginMachineType == _CASTLE_TYPE_V3UL_)
	{
		inRetVal = inFunc_Display_All_Status("00000");
	}
	else if (ginMachineType == _CASTLE_TYPE_MP200_)
	{
		inRetVal = inFunc_Display_All_Status("00110");
	}
	else
	{
		inRetVal = inFunc_Display_All_Status("11111");
	}

	return (inRetVal);
}

/*
Function        :inFunc_Display_Time
Date&Time       :2017/10/25 下午 3:27
Describe        :
*/
int inFunc_Display_Time()
{
        char		szTemplate[42 + 1] = {0};
        RTC_NEXSYS	srRTC;

        memset(szTemplate, 0x00, sizeof(szTemplate));

        memset(&srRTC, 0x00, sizeof(srRTC));
        if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        sprintf(szTemplate, "20%02d/%02d/%02d %02d:%02d:%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
        inDISP_ChineseFont_Point_Color(szTemplate, _FONTSIZE_16X44_, _LINE_16_1_, _COLOR_BLACK_, _COLOR_WHITE_, 1);

        return (VS_SUCCESS);
}

/*
Function        :vdFunc_Display_Ethernet_Status
Date&Time       :2018/3/9 上午 11:18
Describe        :icon 30*30
*/
void vdFunc_Display_Ethernet_Status()
{
	unsigned int	uiXPos = 165;

	if (inEthernet_IsPhysicalOnine() == VS_SUCCESS)
	{
		inDISP_PutGraphic(_ICON_ETHERNET_CONNECTED_, uiXPos, _COORDINATE_Y_LINE_16_1_);
	}
	else
	{
		inDISP_PutGraphic(_ICON_ETHERNET_DISCONNECTED_, uiXPos, _COORDINATE_Y_LINE_16_1_);
	}
}

/*
Function        :vdFunc_Display_Battery_Status
Date&Time       :2018/3/9 下午 1:32
Describe        :電池icon 20*30
*/
void vdFunc_Display_Battery_Status()
{
	int		inRetVal = VS_ERROR;
	char		szTemplate[42 + 1] = {0};
	char		szPercentage[4 + 1] = {0};
	unsigned int	uiXPos = 200;
	unsigned char	uszPercentage = 0;

	/* 顯示沒電池 */
	if (inFunc_Is_Battery_Exist() != VS_SUCCESS)
	{
//		inDISP_PutGraphic(_ICON_BATTERY_NOT_EXIST_, uiXPos, _COORDINATE_Y_LINE_16_1_);
	}
	/* 有電池 */
	else
	{
		/* 是否充電中 */
		inRetVal = inFunc_Is_Battery_Charging();
		if (inRetVal == VS_SUCCESS)
		{
			inDISP_PutGraphic(_ICON_BATTERY_CHARGING_, uiXPos, _COORDINATE_Y_LINE_16_1_);
			inRetVal = inFunc_Get_Battery_Capacity(&uszPercentage);
		}
		else
		{
			inRetVal = inFunc_Get_Battery_Capacity(&uszPercentage);
			/* 0% */
			if (inRetVal != VS_SUCCESS	||
			    (uszPercentage >= 0		&&
			     uszPercentage < 10))
			{
				inDISP_PutGraphic(_ICON_BATTERY_CAPACITY_000_, uiXPos, _COORDINATE_Y_LINE_16_1_);
			}
			else if (uszPercentage >= 10	&&
				 uszPercentage < 20)
			{
				inDISP_PutGraphic(_ICON_BATTERY_CAPACITY_020_, uiXPos, _COORDINATE_Y_LINE_16_1_);
			}
			else if (uszPercentage >= 20	&&
				 uszPercentage < 40)
			{
				inDISP_PutGraphic(_ICON_BATTERY_CAPACITY_040_, uiXPos, _COORDINATE_Y_LINE_16_1_);
			}
			else if (uszPercentage >= 40	&&
				 uszPercentage < 60)
			{
				inDISP_PutGraphic(_ICON_BATTERY_CAPACITY_060_, uiXPos, _COORDINATE_Y_LINE_16_1_);
			}
			else if (uszPercentage >= 60	&&
				 uszPercentage < 80)
			{
				inDISP_PutGraphic(_ICON_BATTERY_CAPACITY_080_, uiXPos, _COORDINATE_Y_LINE_16_1_);
			}
			else if (uszPercentage >= 80	&&
				 uszPercentage <= 1000)
			{
				inDISP_PutGraphic(_ICON_BATTERY_CAPACITY_100_, uiXPos, _COORDINATE_Y_LINE_16_1_);
			}
		}

		/* 顯示電量比 */
		/* 清空，不然數字可能殘留 */
		inDISP_ChineseFont_Point_Color("    ", _FONTSIZE_16X44_, _LINE_16_1_, _COLOR_BLACK_, _COLOR_WHITE_, 28);
		if (uszPercentage > 0)
		{
			memset(szPercentage, 0x00, sizeof(szPercentage));
			sprintf(szPercentage, "%d%%", uszPercentage);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inFunc_PAD_ASCII(szTemplate, szPercentage, ' ', 4, _PAD_RIGHT_FILL_LEFT_);
			inDISP_ChineseFont_Point_Color(szTemplate, _FONTSIZE_16X44_, _LINE_16_1_, _COLOR_BLACK_, _COLOR_WHITE_, 28);
		}
	}
}

/*
Function        :vdFunc_Display_WiFi_Quality
Date&Time       :2018/3/13 下午 4:18
Describe        :icon 30 * 30
*/
void vdFunc_Display_WiFi_Quality(void)
{
	unsigned int	uiXPos = 260;
	unsigned char	uszQuality = 0;

	/* 顯示沒訊號 */
	if (inWiFi_Get_Quality(&uszQuality) != VS_SUCCESS)
	{
//		inDISP_PutGraphic(_ICON_WIFI_NO_SIGNAL_, uiXPos, _COORDINATE_Y_LINE_16_1_);
	}
	/* 有訊號 */
	else
	{

		/* 待測試 */
		/* 無法測定 */
		if (uszQuality == 0)
		{
			inDISP_PutGraphic(_ICON_WIFI_NO_SIGNAL_, uiXPos, _COORDINATE_Y_LINE_16_1_);
		}
		/* 一格 */
		else if (uszQuality == 1)
		{
			inDISP_PutGraphic(_ICON_WIFI_SIGNAL_1_, uiXPos, _COORDINATE_Y_LINE_16_1_);
		}
		/* 兩格 */
		else if (uszQuality == 2)
		{
			inDISP_PutGraphic(_ICON_WIFI_SIGNAL_2_, uiXPos, _COORDINATE_Y_LINE_16_1_);
		}
		/* 三格 */
		else if (uszQuality == 3)
		{
			inDISP_PutGraphic(_ICON_WIFI_SIGNAL_3_, uiXPos, _COORDINATE_Y_LINE_16_1_);
		}
		/* 四格 */
		else if (uszQuality == 4)
		{
			inDISP_PutGraphic(_ICON_WIFI_SIGNAL_4_, uiXPos, _COORDINATE_Y_LINE_16_1_);
		}
		/* 五格 */
		else if (uszQuality == 5)
		{
			inDISP_PutGraphic(_ICON_WIFI_SIGNAL_5_, uiXPos, _COORDINATE_Y_LINE_16_1_);
		}
	}
}

/*
Function        :vdFunc_Display_SIM_Quality
Date&Time       :2018/3/13 下午 2:13
Describe        :icon 30 * 30
*/
void vdFunc_Display_SIM_Quality(void)
{
	unsigned char	uszQuality = 0;
	unsigned int	uiXPos = 290;

	/* 顯示沒訊號 */
	if (inGSM_GetSignalQuality(&uszQuality) != VS_SUCCESS)
	{
//		inDISP_PutGraphic(_ICON_SIM_NO_SIGNAL_, uiXPos, _COORDINATE_Y_LINE_16_1_);
	}
	/* 有訊號 */
	else
	{

		/* 無法測定 */
		if (uszQuality == 99)
		{
			inDISP_PutGraphic(_ICON_SIM_NO_SIGNAL_, uiXPos, _COORDINATE_Y_LINE_16_1_);
		}
		/* 1格 -113 ~ -111 */
		else if (uszQuality >= 0	&&
			 uszQuality <= 1)
		{
			inDISP_PutGraphic(_ICON_SIM_SIGNAL_1_, uiXPos, _COORDINATE_Y_LINE_16_1_);
		}
		/* 兩格 -110 ~ -91 */
		else if (uszQuality > 1		&&
			 uszQuality <= 11)
		{
			inDISP_PutGraphic(_ICON_SIM_SIGNAL_2_, uiXPos, _COORDINATE_Y_LINE_16_1_);
		}
		/* 三格 -90 ~ -76*/
		else if (uszQuality > 11		&&
			 uszQuality <= 17)
		{
			inDISP_PutGraphic(_ICON_SIM_SIGNAL_3_, uiXPos, _COORDINATE_Y_LINE_16_1_);
		}
		/* 四格 */
		else if (uszQuality > 17	&&
			 uszQuality <= 31)
		{
			inDISP_PutGraphic(_ICON_SIM_SIGNAL_4_, uiXPos, _COORDINATE_Y_LINE_16_1_);
		}
	}
}

/*
Function        :inFunc_Check_Card_Still_Exist
Date&Time       :2017/11/6 上午 10:40
Describe        :
*/
int inFunc_Check_Card_Still_Exist(TRANSACTION_OBJECT *pobTran, int inIsError)
{
	int	inRetVal = VS_SUCCESS;

        /* 晶片卡 */
        inRetVal = inEMV_CheckRemoveCard(pobTran, inIsError);
        /* 感應卡 */
        inRetVal = inCTLS_CheckRemoveCard(pobTran, inIsError);

        return (inRetVal);
}

/*
Function        :inFunc_Display_LOGO
Date&Time       :2017/11/21 下午 5:03
Describe        :
*/
int inFunc_Display_LOGO(int inX, int inY)
{
	int	inLOGONum = 0;
	char	szFileName[100 + 1];
	char	szLOGONum[4 + 1];
	char	szDebugMsg[100 + 1];

	memset(szLOGONum, 0x00, sizeof(szLOGONum));
	inGetLOGONum(szLOGONum);
	memset(szFileName, 0x00, sizeof(szFileName));

	if (strlen(szLOGONum) <= 0)
	{

	}
	else
	{
		memset(szFileName, 0x00, sizeof(szFileName));

		inLOGONum = atoi(szLOGONum);
		switch(inLOGONum)
		{
			case	1:
				sprintf(szFileName, "%s", _MENU_HOST_001_TAIWAN_COOPERATIVE_BANK_);
				break;
			case	2:
				sprintf(szFileName, "%s", _MENU_HOST_002_SHANGHAI_COMMERCIAL_SAVINGS_BANK_);
				break;
			case	3:
				sprintf(szFileName, "%s", _MENU_HOST_003_LAND_BANK_OF_TAIWAN_);
				break;
			case	4:
				sprintf(szFileName, "%s", _MENU_HOST_004_CHINATRUST_COMMERCIAL_BANK_);
				break;
			case	5:
				sprintf(szFileName, "%s", _MENU_HOST_005_JIH_SUN_INTERNATIONAL_BANK_);
				break;
			case	6:
				sprintf(szFileName, "%s", _MENU_HOST_006_TAIPEI_FUBON_COMMERCIAL_BANK_);
				break;
			case	7:
				sprintf(szFileName, "%s", _MENU_HOST_007_TAISHIN_INTERNATIONAL_BANK_);
				break;
			case	8:
				sprintf(szFileName, "%s", _MENU_HOST_008_Bank_SINO_PAC_);
				break;
			case	9:
				sprintf(szFileName, "%s", _MENU_HOST_009_ESUN_COMMERCIAL_BANK_);
				break;
			case	10:
				sprintf(szFileName, "%s", _MENU_HOST_010_MEGA_INTERNAIONAL_COMMERCIAL_BANK_);
				break;
			case	11:
				sprintf(szFileName, "%s", _MENU_HOST_011_ENTIE_COMMERCIAL_BANK_);
				break;
			case	12:
				sprintf(szFileName, "%s", _MENU_HOST_012_BANK_OF_KAOHSIUNG_);
				break;
			case	13:
				sprintf(szFileName, "%s", _MENU_HOST_013_CATHAY_UNITED_BANK_);
				break;
			case	14:
				sprintf(szFileName, "%s", _MENU_HOST_014_FIRST_COMMERCIAL_BANK_);
				break;
			case	15:
				sprintf(szFileName, "%s", _MENU_HOST_015_KGI_BANK_);
				break;
			case	16:
				sprintf(szFileName, "%s", _MENU_HOST_016_HUA_NAN_COMMERCIAL_BANK_);
				break;
			case	17:
				sprintf(szFileName, "%s", _MENU_HOST_017_SHIN_KONG_COMMERCIAL_BANK_);
				break;
			case	18:
				sprintf(szFileName, "%s", _MENU_HOST_018_CHANG_HWA_COMMERCIAL_BANK_);
				break;
			case	19:
				sprintf(szFileName, "%s", _MENU_HOST_019_TAIWAN_BUSINESS_BANK_);
				break;


			case	20:
				sprintf(szFileName, "%s", _MENU_HOST_020_FAR_EASTERN_INTERNATIONAL_BANK_);
				break;
			case	21:
				sprintf(szFileName, "%s", _MENU_HOST_021_UNION_BANK_);
				break;
			case	22:
				sprintf(szFileName, "%s", _MENU_HOST_022_COTA_COMMERCIAL_BANK_);
				break;
			case	23:
				sprintf(szFileName, "%s", _MENU_HOST_023_TA_CHONG_BANK_);
				break;
			case	24:
				sprintf(szFileName, "%s", _MENU_HOST_024_YUANTA_COMMERCIAL_BANK_);
				break;
			case	25:
				sprintf(szFileName, "%s", _MENU_HOST_025_TAICHUNG_COMMERCIAL_BANK_);
				break;
			case	26:
				sprintf(szFileName, "%s", _MENU_HOST_026_AEON_);
				break;
			case	27:
				sprintf(szFileName, "%s", _MENU_HOST_027_RAKUTEN_);
				break;
			case	28:
				sprintf(szFileName, "%s", _MENU_HOST_028_CITIBANK_);
				break;
			case	29:
				sprintf(szFileName, "%s", _MENU_HOST_029_DBS_BANK_);
				break;


			case	30:
				sprintf(szFileName, "%s", _MENU_HOST_030_CHARTERED_BANK_);
				break;
			case	31:
				sprintf(szFileName, "%s", _MENU_HOST_031_HWATAI_BANK_);
				break;
			case	32:
				sprintf(szFileName, "%s", _MENU_HOST_032_SUNNY_BANK_);
				break;
			case	33:
				sprintf(szFileName, "%s", _MENU_HOST_033_HSBC_BANK_);
				break;
			case	34:
				sprintf(szFileName, "%s", _MENU_HOST_034_ABN_AMRO_BANK_);
				break;
                    
                            case	94:
				sprintf(szFileName, "%s", _MENU_HOST_094_ESUN_BANK_);
                                       break;
			case	95:
				sprintf(szFileName, "%s", _MENU_HOST_095_TAIPEI_FUBON_COMMERCIAL_BANK_);
				break;
			case	96:
				sprintf(szFileName, "%s", _MENU_HOST_096_CATHY_UNITED_BANK_);
				break;
			case	97:
				sprintf(szFileName, "%s", _MENU_HOST_097_TAISHIN_);
				break;
			case	98:
				sprintf(szFileName, "%s", _MENU_HOST_098_NCCC_);
				break;
			case	99:
				sprintf(szFileName, "%s", _MENU_HOST_099_NEXSYS_);
				break;

			default:
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "No Such LOGONum");
					inDISP_LogPrintf(szDebugMsg);
				}
				return (VS_ERROR);
				break;
		}
	}

	inDISP_PutGraphic(szFileName, inX,  inY);		/* 顯示 LOGO */

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Edit_LOGONum
Date&Time       :2017/11/22 上午 9:27
Describe        :修改logo編號
*/
int inFunc_Edit_LOGONum()
{
	int		inRetVal = VS_SUCCESS;
	char		szTemplate[22 + 1];
        DISPLAY_OBJECT  srDispObj;

	inDISP_ClearAll();

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	srDispObj.inY = _LINE_8_8_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 4;
	srDispObj.inColor = _COLOR_BLACK_;
	inRetVal = inGetLOGONum(szTemplate);

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont_Color("LOGO編號", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
	inDISP_ChineseFont_Color(szTemplate, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);

	while (1)
	{
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		inDISP_ChineseFont_Color("輸入數字後按OK,放棄按X", _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16(&srDispObj);
		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		{
			break;
		}
		else if (srDispObj.inOutputLen >= 0)
		{
			inRetVal = inSetLOGONum(srDispObj.szOutput);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			inRetVal = inSaveEDCRec(0);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			break;
		}

	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_IP_Transform
Date&Time       :2018/3/2 下午 5:52
Describe        :轉換IP表示形式
*/
int inFunc_IP_Transform(char* szAsciiIP, char* szDecimalIP)
{
	int	i = 0;
	int	inCnt = 0;
	int	inTemp = 0;

	for (i = 0; i < strlen(szAsciiIP); i++)
	{
		inTemp = 0;
		while (1)
		{
			if (szAsciiIP[inCnt] == '.'	||
			    szAsciiIP[inCnt] == 0x00)
			{
				inCnt++;
				break;
			}
			else
			{
				inTemp = inTemp * 10 + (szAsciiIP[inCnt] - '0');
				inCnt++;
			}
		}

		szDecimalIP[i] = inTemp;
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Get_Battery_Status
Date&Time       :2018/3/9 下午 1:52
Describe        :
*/
int inFunc_Get_Battery_Status(unsigned int* uiStatus)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_BatteryStatus(uiStatus);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BatteryStatus() OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else if (usRetVal ==d_BATTERY_NOT_SUPPORT)
	{
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BatteryStatus Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Is_Battery_Exist
Date&Time       :2018/3/9 下午 1:58
Describe        :
*/
int inFunc_Is_Battery_Exist()
{
	int		inRetVal = VS_ERROR;
	unsigned int	uiStatus = 0;

	inRetVal = inFunc_Get_Battery_Status(&uiStatus);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	else
	{
		if ((uiStatus & d_MK_BATTERY_EXIST) == d_MK_BATTERY_EXIST)
		{
			return (VS_SUCCESS);
		}
		else
		{
			return (VS_ERROR);
		}
	}
}

/*
Function        :inFunc_Is_Battery_Charging
Date&Time       :2018/3/12 下午 5:37
Describe        :
*/
int inFunc_Is_Battery_Charging()
{
	int		inRetVal = VS_ERROR;
	unsigned int	uiStatus = 0;

	inRetVal = inFunc_Get_Battery_Status(&uiStatus);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	else
	{
		if ((uiStatus & d_MK_BATTERY_CHARGE) == d_MK_BATTERY_CHARGE)
		{
			return (VS_SUCCESS);
		}
		else
		{
			return (VS_ERROR);
		}
	}
}

/*
Function        :inFunc_Get_Battery_Capacity
Date&Time       :2018/3/9 下午 2:10
Describe        :
*/
int inFunc_Get_Battery_Capacity(unsigned char* uszPercentage)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_BatteryGetCapacity(uszPercentage);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BatteryGetCapacity Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		*uszPercentage = 0;

		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BatteryGetCapacity () OK, (%d%%)", *uszPercentage);
			inDISP_LogPrintf(szDebugMsg);
		}
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Device_Model_Get
Date&Time       :2018/5/8 下午 9:00
Describe        :
*/
int inFunc_Device_Model_Get(unsigned char* uszModel)
{
	char    szDebugMsg[100 + 1] = {0};
	unsigned short  usRetVal = VS_ERROR;

	usRetVal = CTOS_DeviceModelGet(uszModel);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_DeviceModelGet() OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}

	return (VS_SUCCESS);
}

/*
Function        : inFunc_Decide_Machine_Type
Date&Time   : 2018/5/8 下午 9:09
Describe        : 抓取機器型號，並設定在全域變數內
*/
int inFunc_Decide_Machine_Type(int* inType)
{
	unsigned char	uszModel = 0;

	inFunc_Device_Model_Get(&uszModel);
	if (uszModel == d_MODEL_VEGA3000)
	{
		if (inFunc_Is_Portable_Type() == VS_TRUE)
		{
			*inType = _CASTLE_TYPE_V3M_;
			inDISP_LogPrintfWithFlag(" Model ( _CASTLE_TYPE_V3M_ )");
		}
		else
		{
			*inType = _CASTLE_TYPE_V3C_;
			inDISP_LogPrintfWithFlag(" Model ( _CASTLE_TYPE_V3C_ )");
		}
	}
	else if (uszModel == d_MODEL_VEGA3000P)
	{
		*inType = _CASTLE_TYPE_V3P_;
		inDISP_LogPrintfWithFlag(" Model ( _CASTLE_TYPE_V3P_ )");
	}
	else if (uszModel == d_MODEL_V3UL)
	{
		*inType = _CASTLE_TYPE_V3UL_;
		inDISP_LogPrintfWithFlag(" Model ( _CASTLE_TYPE_V3UL_ )");
	}
	else if (uszModel == d_MODEL_MP200)
	{
		*inType = _CASTLE_TYPE_MP200_;
		inDISP_LogPrintfWithFlag(" Model ( _CASTLE_TYPE_MP200_ )");
	}
	else if (uszModel == d_MODEL_UPT1000)
	{
		*inType = _CASTLE_TYPE_UPT1000_;
		inDISP_LogPrintfWithFlag(" Model ( _CASTLE_TYPE_UPT1000_ )");
	}
	else
	{
		*inType = _CASTLE_TYPE_V3C_;
		inDISP_LogPrintfWithFlag(" Model Not Match Use Default  ( _CASTLE_TYPE_V3C_ )");
	}

	return (VS_SUCCESS);
}


/*
Function        :inFunc_Get_Termial_Model_Name
Date&Time       :2019/11/15 上午 11:41
Describe        :
*/
int inFunc_Get_Termial_Model_Name(char *szModelName)
{
	int	inType = _CASTLE_TYPE_V3C_;
	
	inFunc_Decide_Machine_Type(&inType);
	
	if (inType == _CASTLE_TYPE_V3C_)
	{
		strcpy(szModelName, _CASTLE_TYPE_NAME_V3C_);
	}
	else if (inType == _CASTLE_TYPE_V3M_)
	{
		strcpy(szModelName, _CASTLE_TYPE_NAME_V3M_);
	}
	else if (inType == _CASTLE_TYPE_V3P_)
	{
		strcpy(szModelName, _CASTLE_TYPE_NAME_V3P_);
	}
	else if (inType == _CASTLE_TYPE_V3UL_)
	{
		strcpy(szModelName, _CASTLE_TYPE_NAME_V3UL_);
	}
	else if (inType == _CASTLE_TYPE_MP200_)
	{
		strcpy(szModelName, _CASTLE_TYPE_NAME_MP200_);
	}
	else if (inType == _CASTLE_TYPE_UPT1000_)
	{
		strcpy(szModelName, _CASTLE_TYPE_NAME_UPT1000_);
	}
	else if (inType == _CASTLE_TYPE_UPT1000F_)
	{
		strcpy(szModelName, _CASTLE_TYPE_NAME_UPT1000F_);
	}
	else
	{
		strcpy(szModelName, _CASTLE_TYPE_NAME_UNKNOWN_);
	}
	
	return (VS_SUCCESS);
}



/*
Function        :inFunc_Check_Print_Capability
Date&Time       :2018/5/8 下午 9:05
Describe        :
*/
int inFunc_Check_Print_Capability(int inType)
{
	if (inType == _CASTLE_TYPE_V3C_	||
	    inType == _CASTLE_TYPE_V3M_)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

/*
Function        :inFunc_UpdateAP_Preserve
Date&Time       :2018/6/23 下午 10:07
Describe        :更新AP前先儲存會被清掉的東西，之後可以考慮用XML存
*/
int inFunc_UpdateAP_Preserve(PRESERVE_NEXSYS* srPreserve)
{
    char szTemp[32];
	/* 清空 */
	memset(srPreserve, 0x00, sizeof(PRESERVE_NEXSYS));

	inLoadEDCRec(0);
	/* IP */
	inGetTermIPAddress(srPreserve->szTermIPAddress);

	/* MASK */
	inGetTermMASKAddress(srPreserve->szTermMASKAddress);

	/* Gateway */
	inGetTermGetewayAddress(srPreserve->szTermGetewayAddress);

	inLoadTMSFTPRec(0);
	/* FTP IP */
	inGetFTPIPAddress(srPreserve->szFTPIPAddress);
          inDISP_LogPrintfWithFlag("  UpAp Ip[%s]", srPreserve->szFTPIPAddress);
    
	/*FTP PORT */
	inGetFTPPortNum(srPreserve->szFTPPortNum);
        inDISP_LogPrintfWithFlag("  UpAp Port[%s]", srPreserve->szFTPPortNum);
        
        memset(szTemp, 0x00, sizeof(szTemp));
        inGetFTPDownloadResponseCode(szTemp);
        inDISP_LogPrintfWithFlag("  UpAp Resp Code[%s]", szTemp);

	return (VS_SUCCESS);
}

/*
Function        :inFunc_UpdateAP_Recover
Date&Time       :2018/6/23 下午 10:07
Describe        :更新AP後回復會被清掉的東西，之後可以考慮用XML
*/
int inFunc_UpdateAP_Recover(PRESERVE_NEXSYS* srRecover)
{
	/* IP */
	inSetTermIPAddress(srRecover->szTermIPAddress);

	/* MASK */
	inSetTermMASKAddress(srRecover->szTermMASKAddress);

	/* Gateway */
	inGetTermGetewayAddress(srRecover->szTermGetewayAddress);

	inSaveEDCRec(0);

	/* FTP IP */
	inSetFTPIPAddress(srRecover->szFTPIPAddress);

	/*FTP PORT */
	inSetFTPPortNum(srRecover->szFTPPortNum);

	inSaveTMSFTPRec(0);

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Table_Delete_Record
Date&Time       :2018/7/5 上午 9:15
Describe        :
*/
int inFunc_Table_Delete_Record(char *szFileName, int inRecordIndex)
{
	/* lnTotalLen : 該檔案全長度 ,		inRecLen : 一個Record的長度 */
	/* lnFinalToalLen :Resort完的長度，用來確認檔案完整性 */
        /* szFile : 該檔案全部內容 ,		szRecord : 該Record全部內容 */

	int			i;
	int			inOldRecLen = 0;		/* Old Record 長度 */
	int			inRecCnt = 0;
	int			inOldRecStartOffset = 0;	/* Old Record 起始位置 */
	int			inRetVal;
        char			*szFile, *szRecord;
	long			lnTotalLen = 0;
	long			lnLeftSize = 0;			/* 剩餘長度 */
	unsigned long		ulHandle;

	/* index不合法 */
	if (inRecordIndex == -1)
	{
		return (VS_ERROR);
	}

        inRetVal = inFILE_Open(&ulHandle, (unsigned char*)szFileName);
        if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 算總長度 */
        lnTotalLen = lnFILE_GetSize(&ulHandle, (unsigned char*)szFileName);

	/* 讀出HDT所有資料 */
        inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_);
	szFile = malloc((lnTotalLen + 1) * sizeof(char));
	memset(szFile, 0x00, lnTotalLen * sizeof(char));
	lnLeftSize = lnTotalLen;

	for (i = 0;; ++i)
	{
		/* 剩餘長度大於或等於1024 */
		if (lnLeftSize >= 1024)
		{
			if (inFILE_Read(&ulHandle, (unsigned char*)&szFile[1024 * i], 1024) == VS_SUCCESS)
			{
				/* 一次讀1024 */
				lnLeftSize -= 1024;

				/* 當剩餘長度剛好為1024，會剛好讀完 */
				if (lnLeftSize == 0)
					break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulHandle);

				/* Free pointer */
				free(szFile);

				return (VS_ERROR);
			}
		}
		/* 剩餘長度小於1024 */
		else if (lnLeftSize < 1024)
		{
			/* 就只讀剩餘長度 */
			if (inFILE_Read(&ulHandle, (unsigned char*)&szFile[1024 * i], lnLeftSize) == VS_SUCCESS)
			{
				break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulHandle);

				/* Free pointer */
				free(szFile);

				return (VS_ERROR);
			}

		}

	}

	/* 算出Old Record的長度 */
	for (i = 0, inRecCnt = 0; i < lnTotalLen; i++)
	{
		/* Record 長度 */
		if (inRecCnt == inRecordIndex)
		{
			inOldRecLen++;
		}
		/* 若大於，代表已經算完，要跳出 */
		else if (inRecCnt > inRecordIndex)
		{
			break;
		}

		/* Record 結尾 */
		if (szFile[i] == 0x0A && szFile[i - 1] == 0x0D)
		{
			inRecCnt++;

			/* 算出Record起始位置 若是第0 Record，永遠不會進去，則inOldRecStartOffset 為 0 */
			if (inRecCnt == inRecordIndex)
			{
				inOldRecStartOffset = i + 1;
			}
		}

	}

	/* 移到Record的起始位置 */
	if (inFILE_Seek(ulHandle, inOldRecStartOffset, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		inFILE_Close(&ulHandle);
		free(szFile);

		return (VS_ERROR);
	}

	/* 讀出Record的資料 */
	szRecord = malloc((inOldRecLen + 1) * sizeof(char));
	memset(szRecord, 0x00, inOldRecLen * sizeof(char));
	if (inFILE_Read(&ulHandle, (unsigned char*)szRecord, inOldRecLen) != VS_SUCCESS)
	{
		inFILE_Close(&ulHandle);
		free(szRecord);
		free(szFile);

		return (VS_ERROR);
	}

	/* 因這裡V3模仿Verifone的insert和delete，但用自己的方式實做，所以需要先關檔再由該function操作*/
	if (inFILE_Close(&ulHandle) != VS_SUCCESS)
	{
		free(szRecord);
		free(szFile);

		return (VS_ERROR);
	}

	/* 釋放記憶體 */
        free(szFile);

	if (inFILE_Data_Delete((unsigned char*)szFileName, inOldRecStartOffset, inOldRecLen) != VS_SUCCESS)	/* 刪除DCC那一段 */
	{
		/* 釋放記憶體 */
		free(szRecord);

		return (VS_ERROR);
	}

	/* 釋放記憶體 */
	free(szRecord);

        return (VS_SUCCESS);
}

/*
Function        :inFunc_Check_SDCard_Mounted
Date&Time       :2018/7/24 下午 1:34
Describe        :
*/
int inFunc_Check_SDCard_Mounted(void)
{
	int	inRetVal = VS_ERROR;
	char	szCommand[100 + 1] = {0};

	memset(szCommand, 0x00, sizeof(szCommand));
	sprintf(szCommand, "ls -l /sys/block/ | grep 'mmc'");

	inRetVal = inFunc_ShellCommand_System(szCommand);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("SD Not Mounted");
		}

		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("SD Mounted");
		}

		return (VS_SUCCESS);
	}
}



/*
Function        :inFunc_Check_USB_Mounted
Date&Time       :2019/2/23 上午 11:50
Describe        :參考條件：
 *		system("ls /sys/block/ | grep 'sd'") == 0 &&
 *		system("find /dev/ -name 'sd*'") == 0
*/
int inFunc_Check_USB_Mounted(void)
{
	int	inRetVal = VS_ERROR;
	char	szCommand[100 + 1] = {0};
	
	/* 有掛載udisk */
	memset(szCommand, 0x00, sizeof(szCommand));
	sprintf(szCommand, "mount | grep '/media/udisk'");

	inRetVal = inFunc_ShellCommand_System(szCommand);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("SD Not Mounted");
		}

		inRetVal = VS_ERROR;
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("SD Mounted");
		}

		inRetVal = VS_SUCCESS;
	}
	
	return (inRetVal);
}


/*
Function	:inFunc_ECR_Comport_Switch
Date&Time	:2018/7/27 上午 9:50
Describe        :
*/
int inFunc_ECR_Comport_Switch(void)
{
        char		szTemplate[64 + 1] = {0};
	char		szDispMsg[64 + 1] = {0};
	unsigned char	uszKey = 0x00;

        /* Comport */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetECRComPort(szTemplate);

	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "ECR Comport: %s", szTemplate);

        inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
        inDISP_ChineseFont("0 = USB1", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
	inDISP_ChineseFont("1 = COM1", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
	inDISP_ChineseFont("2 = COM2", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);

        while (1)
        {
                uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_0_)
		{
			inSetECRComPort("USB1");
			inSaveEDCRec(0);
			break;
		}
		else if (uszKey == _KEY_1_)
		{
			inSetECRComPort("COM1");
			inSaveEDCRec(0);
			break;
		}
		else if (uszKey == _KEY_2_)
		{
			inSetECRComPort("COM2");
			inSaveEDCRec(0);
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			break;
		}
		else
		{
			continue;
		}
        }


        return (VS_SUCCESS);
}

/*
Function        : inFunc_Decide_APVersion_Type
Date&Time   : 2018/8/13 下午 2:23
Describe        : 使用 HDT 中的 Label 判斷對應的主機是哪個，可用來判斷要使用哪個主機的規則
*/
int inFunc_Decide_APVersion_Type(int* inType)
{
	char	szHostLabel[8 + 1] = {0};
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	memset(szHostLabel, 0x00, sizeof(szHostLabel));
	if (inLoadHDTRec(0) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" FUNC Decide Ap Version Load HDT[0] *Error* Line[%d]", __LINE__);
	}
	inGetHostLabel(szHostLabel);

	if (memcmp(szHostLabel, _HOST_NAME_CREDIT_FUBON_, strlen(_HOST_NAME_CREDIT_FUBON_)) == 0)
	{
		*inType = _APVERSION_TYPE_FUBON_;
	}
	else
	{
		/*TODO: 要測試一下失敗狀況 2022/7/21 上午 11:51 */
		/* 改成讀出的host如果有問題就鎖機，不再執行下個步驟 [SAM] 2022/7/21 上午 11:50 */		
		inDISP_Initial();
		inFunc_EDCLock();
		return(VS_ERROR);

	}
	
	inDISP_DispLogAndWriteFlie("  Boot VersionType [%s] Line[%d]", szHostLabel, __LINE__);
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

int inCountEnableHost(void)
{
int i,inOpenHostCnt = 0;
char	szHostName[8 + 1],szHostEnable[2 + 1] = {0},szDebugMsg[100 + 1] = {0};

	for (i = 0;; ++i)
	{
		/* 先LoadHDT */
		if (inLoadHDTRec(i) == VS_ERROR)
		{
			inDISP_LogPrintfWithFlag(" Count Enagle Host Load HDT[%d] *Error* Line[%d]", i, __LINE__);
			/* 當找不到第i筆資料會回傳VS_ERROR */
			break;
		}

		/* ESC不該出現在選單上 */
		memset(szHostName, 0x00, sizeof(szHostName));
		inGetHostLabel(szHostName);
		
		if (memcmp(szHostName, _HOST_NAME_ESC_, strlen(_HOST_NAME_ESC_)) == 0)
		{
			continue;
		}
		
		/* 因為公司TMS會下CUP主機，因富邦是同批次，所以CUP要跳過 所以新增 20190214 [SAM] */
		/* [新增主機要注意] [SAM] 2022/7/21 下午 2:08*/
		if (memcmp(szHostName, _HOST_NAME_CUP_, strlen(_HOST_NAME_CUP_)) == 0)
		{
			continue;
		}	
		/* GET HOST Enable */
		memset(szHostEnable, 0x00, sizeof(szHostEnable));
		if (inGetHostEnable(szHostEnable) == VS_ERROR)
		{
			break;
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%d HostEnable: %s", i, szHostEnable);
				inDISP_LogPrintf(szDebugMsg);
			}

			if (memcmp(szHostEnable, "Y", 1) != 0)
			{
				/* 如果HostEnable != Y，就continue */
				continue;
			}

			/* 如果主機有開，才loadHDPT */
			if (inLoadHDPTRec(i) == VS_ERROR)
			{
				/* 當找不到第i筆資料會回傳VS_ERROR */
				break;
			}

			inOpenHostCnt ++;       /* 記錄有幾個Host有開 */
		}
	}

	return inOpenHostCnt;
}

void vdDispHDPTRec(int inNum)
{
unsigned long   ulFile_Handle;
long            lnHDPTLength = 0;
unsigned char uszTemp[200],uszTemp2[500];
//int i;

	inFILE_Open(&ulFile_Handle, (unsigned char *)_HDPT_FILE_NAME_);
	lnHDPTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_HDPT_FILE_NAME_);
	inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_);
	memset(uszTemp,0x00,sizeof(uszTemp));
	memset(uszTemp2,0x00,sizeof(uszTemp2));
	inFILE_Read(&ulFile_Handle, &uszTemp[0], sizeof(uszTemp));
	sprintf((char *)uszTemp2,"HDPT %d=%s",inNum,uszTemp);
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf((char *)uszTemp2);
}


/*
Function        : inFunc_Sync_BRec_ESC_Date_Time
Date&Time   : 2018/12/20 [SAM]
Describe        : 同步BRec日期和時間
*/
int inFunc_Sync_BRec_ESC_Date_Time(TRANSACTION_OBJECT *pobTran, RTC_NEXSYS *srRTC)
{
	/* 同步到pobTran */
	memset(pobTran->srBRec.szEscDate, 0x00, sizeof(pobTran->srBRec.szDate));
	memset(pobTran->srBRec.szEscTime, 0x00, sizeof(pobTran->srBRec.szTime));

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	if (inFunc_SyncPobTran_Date_Include_Year(pobTran->srBRec.szEscDate, sizeof(pobTran->srBRec.szEscDate),  srRTC) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" Sync ESC Year Date *Error* Line[%d] ", __LINE__);
		return (VS_ERROR);
	}


	if (inFunc_SyncPobTran_Time(pobTran->srBRec.szEscTime, sizeof(pobTran->srBRec.szEscTime),  srRTC) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" Sync ESC Time *Error* Line[%d] ", __LINE__);
		return (VS_ERROR);
	}

	inDISP_DispLogAndWriteFlie(" Sync szEscDate[%s] szEscTime[%s] ", pobTran->srBRec.szEscDate, pobTran->srBRec.szEscTime);

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}



/*
Function        :inFunc_FUBON_Check_Linkage_Function_Enable
Date&Time       :2019/02/13
Describe        :確認該TMS的連動開關
 *		 確認該卡別是否有在CDT並連動到EDC開關
*/
int inFunc_FUBON_Check_Linkage_Function_Enable(TRANSACTION_OBJECT * pobTran)
{
	int	inCDTIndex = -1;
	char	szHostEnable[2 + 1];
//	char	szESCReciptUploadUpLimit[4 + 1];
	char	szDebugMsg[100 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 不需要重LOAD CDT 只是要檢查 TABLE 是否接收CUP卡 2019/0807 [SAM] */
	if (inFunc_Find_Specific_CDTindex(pobTran->srBRec.inCDTIndex, _CARD_TYPE_CUP_, &inCDTIndex) == VS_SUCCESS)
	{
		inDISP_LogPrintfWithFlag(" TMS Set CUP ON");
		if (inSetCUPFuncEnable("Y") != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Fubon Check Link  SetCupFunc *Error*  Line[%d]",  __LINE__);
			inFunc_EDCLock();
		}
		
		if (inSaveEDCRec(0) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Fubon Check Link  Save EdcRec *Error*  Line[%d]",  __LINE__);
			inFunc_EDCLock();
		}

		/* 因為目前銀聯開啟就開MAC */
		inSetMACEnable("Y");
		
		if (inSaveCFGTRec(0) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Fubon Check Link  Save CFGTRec *Error*  Line[%d]",  __LINE__);
			inFunc_EDCLock();
		}	
		
	}
	else
	{
		inDISP_LogPrintfWithFlag(" TMS Set CUP OFF");
		if (inSetCUPFuncEnable("N") != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Fubon Check Link  SetCupFunc *Error*  Line[%d]",  __LINE__);
			inFunc_EDCLock();
		}
		if (inSaveEDCRec(0) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Fubon Check Link  Save EdcRec *Error*  Line[%d]",  __LINE__);
			inFunc_EDCLock();
		}
		/* 因為目前銀聯關閉就不開MAC */
		inSetMACEnable("N");
		
		if (inSaveCFGTRec(0) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  Fubon Check Link  Save CFGTRec *Error*  Line[%d]",  __LINE__);
			inFunc_EDCLock();
		}	
	}

	
	/* 確認是否要送NE欄位 */
	/* ESC沒開，不送NE */
	memset(szHostEnable, 0x00, sizeof(szHostEnable));
	if (inNEXSYS_ESC_GetESC_Enable(pobTran->srBRec.inHDTIndex, szHostEnable) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "ESC Mode : N, 找不到該Host");
			inDISP_LogPrintf(szDebugMsg);
		}

		if (inSetESCMode("N") != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  ESC Fubon Check Link  inSetESCMode  *Error*  Line[%d]",  __LINE__);
			inFunc_EDCLock();
		}
		if (inSaveEDCRec(0) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  ESC Fubon Check Link  inSaveEDCRec  *Error*  Line[%d]",  __LINE__);
			inFunc_EDCLock();
		}

	}
	else
	{
		/* 因為HOST 可能會沒塞值，所以還是要依照判斷條  20190807 [SAM]*/
		if (szHostEnable[0] != 'Y')
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "ESC Mode : N, Host未開");
				inDISP_LogPrintf(szDebugMsg);
			}

			if (inSetESCMode("N") != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie("  ESC Fubon Check Link  inSetESCMode  *Error*  Line[%d]",  __LINE__);
				inFunc_EDCLock();
			}
			if (inSaveEDCRec(0) != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie("  ESC Fubon Check Link  inSaveEDCRec  *Error*  Line[%d]",  __LINE__);
				inFunc_EDCLock();
			}

		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "ESC Mode : Y");
				inDISP_LogPrintf(szDebugMsg);
			}

			if (inSetESCMode("Y") != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie("  ESC Fubon Check Link  inSetESCMode  *Error*  Line[%d]",  __LINE__);
				inFunc_EDCLock();
			}
			
			if (inSaveEDCRec(0) != VS_SUCCESS)
			{
				inDISP_DispLogAndWriteFlie("  ESC Fubon Check Link  inSaveEDCRec  *Error*  Line[%d]",  __LINE__);
				inFunc_EDCLock();
			}

		}

	}
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}


/*
Function        : inFunc_CustomerSetting
Date&Time   : 2019/03/13
Describe        :
*/
int inFunc_CustomerSetting(TRANSACTION_OBJECT* pobTran)
{
	char	szCustomerIndicator[3 + 1] = {0};

	inLoadEDCRec(0);

	inGetCustomIndicator(szCustomerIndicator);
	
	if (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, 3) == 0)
	{
		inSetECRVersion("02");
		inSetECRComPort("USB1");
		inSetEnterTimeout("002");
	}
	else
	{
		inSetECRVersion("01");
		inSetECRComPort("COM1");
		inSetEnterTimeout("030");
	}

	inSaveEDCRec(0);

	return (VS_SUCCESS);
}


/*
Function        : inFUNC_SyncHostTerminalDateTime
Date&Time       :2016/9/14 上午 10:14
Describe        :
*/
int inFUNC_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran)
{
	char	szDebugMsg[100 + 1];

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "%s", pobTran->srBRec.szDate);
		inDISP_LogPrintf(szDebugMsg);

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "%s", pobTran->srBRec.szTime);
		inDISP_LogPrintf(szDebugMsg);
	}

	inFunc_SetEDCDateTime(pobTran->srBRec.szDate, pobTran->srBRec.szTime);

        return (VS_SUCCESS);
}


/*
Function        : inFunc_Decide_MEG_TRT
Date&Time   : 2019/03/27
Describe        : 根據交易別決定TRT 由 inNCCC_Func_Decide_MEG_TRT 修改過來
*/
int inFunc_Decide_MEG_TRT(TRANSACTION_OBJECT *pobTran)
{
	char	szDebug[100 + 1];

	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		if (pobTran->inTransactionCode == _CUP_SALE_)
		{
			pobTran->inRunTRTID = _TRT_CUP_SALE_;
		}
		else if (pobTran->inTransactionCode == _CUP_REFUND_)
		{
			pobTran->inRunTRTID = _TRT_CUP_REFUND_;
		}
		else if (pobTran->inTransactionCode == _CUP_PRE_AUTH_)
		{
			pobTran->inRunTRTID = _TRT_CUP_PRE_AUTH_;
		}
		else if (pobTran->inTransactionCode == _CUP_PRE_COMP_)
		{
			pobTran->inRunTRTID = _TRT_CUP_PRE_COMP_;
		}
		else if (pobTran->inTransactionCode == _CUP_INST_SALE_)
		{
			pobTran->inRunTRTID = _TRT_CUP_INST_SALE_;
		}
		else if (pobTran->inTransactionCode == _CUP_INST_REFUND_)
		{
			pobTran->inRunTRTID = _TRT_CUP_INST_REFUND_;
		}
		else if (pobTran->inTransactionCode == _CUP_REDEEM_SALE_)
		{
			pobTran->inRunTRTID = _TRT_CUP_REDEEM_SALE_;
		}
		else if (pobTran->inTransactionCode == _CUP_REDEEM_REFUND_)
		{
			pobTran->inRunTRTID = _TRT_CUP_REDEEM_REFUND_;
		}
		else if (pobTran->inTransactionCode == _CUP_MAIL_ORDER_)
		{
			pobTran->inRunTRTID = _TRT_CUP_MAIL_ORDER_;
		}
		else if (pobTran->inTransactionCode == _CUP_MAIL_ORDER_REFUND_)
		{
			pobTran->inRunTRTID = _TRT_CUP_MAIL_ORDER_REFUND_;
		}
		else if (pobTran->inTransactionCode == _LOYALTY_REDEEM_)
		{
			pobTran->inRunTRTID = _TRT_LOYALTY_REDEEM_;
		}
		else if (pobTran->inTransactionCode == _LOYALTY_REDEEM_REFUND_)
		{
			pobTran->inRunTRTID = _TRT_LOYALTY_REDEEM_REFUND_;
		}
		else if (pobTran->inTransactionCode == _VOID_LOYALTY_REDEEM_)
		{
			pobTran->inRunTRTID = _TRT_VOID_LOYALTY_REDEEM_;
		}
		else
		{
			/* 找不到對應的TRT */
			pobTran->inErrorMsg = _ERROR_CODE_V3_TRT_NOT_FOUND_;

			if (ginDebug == VS_TRUE)
			{
				memset(szDebug, 0x00, sizeof(szDebug));
				sprintf(szDebug, "AID_Select But Cat not Select TRT 7, inCode: %d", pobTran->inTransactionCode);
				inDISP_LogPrintf(szDebug);
			}

			return (VS_ERROR);
		}
	}
	else
	{
		if (pobTran->inTransactionCode == _SALE_)
		{
			pobTran->inRunTRTID = _TRT_SALE_;
		}
		else if (pobTran->inTransactionCode == _REFUND_)
		{
			pobTran->inRunTRTID = _TRT_REFUND_;
		}
		else if (pobTran->inTransactionCode == _SALE_OFFLINE_)
		{
			pobTran->inRunTRTID = _TRT_SALE_OFFLINE_;
		}
		else if (pobTran->inTransactionCode == _PRE_AUTH_)
		{
			pobTran->inRunTRTID = _TRT_PRE_AUTH_;
		}
		else if (pobTran->inTransactionCode == _PRE_COMP_)
		{
			pobTran->inRunTRTID = _TRT_PRE_COMP_;
		}
		else if (pobTran->inTransactionCode == _INST_SALE_)
		{
			pobTran->inRunTRTID = _TRT_INST_SALE_;
		}
		else if (pobTran->inTransactionCode == _INST_REFUND_)
		{
			pobTran->inRunTRTID = _TRT_INST_REFUND_;
		}
		else if (pobTran->inTransactionCode == _INST_ADJUST_)
		{
			pobTran->inRunTRTID = _TRT_INST_ADJUST_;
		}
		else if (pobTran->inTransactionCode == _REDEEM_SALE_)
		{
			pobTran->inRunTRTID = _TRT_REDEEM_SALE_;
		}
		else if (pobTran->inTransactionCode == _REDEEM_REFUND_)
		{
			pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;
		}
		else if (pobTran->inTransactionCode == _REDEEM_ADJUST_)
		{
			pobTran->inRunTRTID = _TRT_REDEEM_ADJUST_;
		}
		else if (pobTran->inTransactionCode == _MAIL_ORDER_)
		{
			pobTran->inRunTRTID = _TRT_MAIL_ORDER_;
		}
		else if (pobTran->inTransactionCode == _LOYALTY_REDEEM_)
		{
			pobTran->inRunTRTID = _TRT_LOYALTY_REDEEM_;
		}
		else if (pobTran->inTransactionCode == _LOYALTY_REDEEM_REFUND_)
		{
			pobTran->inRunTRTID = _TRT_LOYALTY_REDEEM_REFUND_;
		}
		else if (pobTran->inTransactionCode == _VOID_LOYALTY_REDEEM_)
		{
			pobTran->inRunTRTID = _TRT_VOID_LOYALTY_REDEEM_;
		}
		else
		{
			/* 找不到對應的TRT */
			pobTran->inErrorMsg = _ERROR_CODE_V3_TRT_NOT_FOUND_;

			if (ginDebug == VS_TRUE)
			{
				memset(szDebug, 0x00, sizeof(szDebug));
				sprintf(szDebug, "AID_Select But Cat not Select TRT 8, inCode: %d", pobTran->inTransactionCode);
				inDISP_LogPrintf(szDebug);
			}

			return (VS_ERROR);
		}
	}

	return (VS_SUCCESS);
}


/*
Function        : inFunc_Decide_ICC_TRT
Date&Time   : 2019/03/27
Describe        : 根據交易別決定TRT 由  inFunc_Decide_ICC_TRT 修改來
*/
int inFunc_Decide_ICC_TRT(TRANSACTION_OBJECT *pobTran)
{
	char	szDebug[100 + 1];

	/* 決定TRT */
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		if (pobTran->inTransactionCode == _FISC_SALE_)
		{
			pobTran->inRunTRTID = _TRT_FISC_SALE_ICC_;
		}
		else if (pobTran->inTransactionCode == _FISC_REFUND_)
		{
			pobTran->inRunTRTID = _TRT_FISC_REFUND_ICC_;
		}
		else
		{
			/* 找不到對應的TRT */
			pobTran->inErrorMsg = _ERROR_CODE_V3_TRT_NOT_FOUND_;

			if (ginDebug == VS_TRUE)
			{
				memset(szDebug, 0x00, sizeof(szDebug));
				sprintf(szDebug, "AID_Select But Cat not Select TRT 9, inCode: %d", pobTran->inTransactionCode);
				inDISP_LogPrintf(szDebug);
			}

			return (VS_ERROR);
		}
	}
	else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		if (pobTran->inTransactionCode == _CUP_SALE_)
		{
			pobTran->inRunTRTID = _TRT_CUP_SALE_ICC_;
		}
		else if (pobTran->inTransactionCode == _CUP_PRE_AUTH_)
		{
			pobTran->inRunTRTID = _TRT_CUP_PRE_AUTH_ICC_;
		}
		else if (pobTran->inTransactionCode == _CUP_INST_SALE_)
		{
			pobTran->inRunTRTID = _TRT_CUP_INST_SALE_ICC_;
		}
		else if (pobTran->inTransactionCode == _CUP_REDEEM_SALE_)
		{
			pobTran->inRunTRTID = _TRT_CUP_REDEEM_SALE_ICC_;
		}
		else
		{
			/* 找不到對應的TRT */
			pobTran->inErrorMsg = _ERROR_CODE_V3_TRT_NOT_FOUND_;

			if (ginDebug == VS_TRUE)
			{
				memset(szDebug, 0x00, sizeof(szDebug));
				sprintf(szDebug, "AID_Select But Cat not Select TRT 10, inCode: %d", pobTran->inTransactionCode);
				inDISP_LogPrintf(szDebug);
			}

			return (VS_ERROR);
		}
	}
	else
	{
		if (pobTran->inTransactionCode == _SALE_)
		{
			pobTran->inRunTRTID = _TRT_SALE_ICC_;
		}
		else if (pobTran->inTransactionCode == _REDEEM_SALE_)
		{
			pobTran->inRunTRTID = _TRT_REDEEM_SALE_ICC_;
		}
		else if (pobTran->inTransactionCode == _PRE_AUTH_)
		{
			pobTran->inRunTRTID = _TRT_PRE_AUTH_ICC_;
		}
		else if (pobTran->inTransactionCode == _INST_SALE_)
		{
			pobTran->inRunTRTID = _TRT_INST_SALE_ICC_;
		}
		else if (pobTran->inTransactionCode == _REDEEM_SALE_)
		{
			pobTran->inRunTRTID = _TRT_REDEEM_SALE_ICC_;
		}
		else
		{
			/* 找不到對應的TRT */
			pobTran->inErrorMsg = _ERROR_CODE_V3_TRT_NOT_FOUND_;

			if (ginDebug == VS_TRUE)
			{
				memset(szDebug, 0x00, sizeof(szDebug));
				sprintf(szDebug, "AID_Select But Cat not Select TRT 11, inCode: %d", pobTran->inTransactionCode);
				inDISP_LogPrintf(szDebug);
			}

			return (VS_ERROR);
		}
	}

	return (VS_SUCCESS);
}

/*
Function        : inFunc_Decide_CTLS_TRT
Date&Time   : 2019/03/27
Describe        : 根據交易別決定TRT 由 inFunc_Decide_CTLS_TRT 修改過來
*/
int inFunc_Decide_CTLS_TRT(TRANSACTION_OBJECT *pobTran)
{
	char	szDebug[100 + 1];

	/* 決定TRT */
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		if (pobTran->inTransactionCode == _FISC_SALE_)
		{
			pobTran->inRunTRTID = _TRT_FISC_SALE_CTLS_;
		}
		else if (pobTran->inTransactionCode == _FISC_REFUND_)
		{
			pobTran->inRunTRTID = _TRT_FISC_REFUND_CTLS_;
		}
		else
		{
			/* 找不到對應的TRT */
			pobTran->inErrorMsg = _ERROR_CODE_V3_TRT_NOT_FOUND_;

			if (ginDebug == VS_TRUE)
			{
				memset(szDebug, 0x00, sizeof(szDebug));
				sprintf(szDebug, "AID_Select But Cat not Select TRT 12, inCode: %d", pobTran->inTransactionCode);
				inDISP_LogPrintf(szDebug);
			}

			return (VS_ERROR);
		}
	}
	else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		if (pobTran->inTransactionCode == _CUP_SALE_)
		{
			pobTran->inRunTRTID = _TRT_CUP_SALE_CTLS_;
		}
		else if (pobTran->inTransactionCode == _CUP_REFUND_)
		{
			pobTran->inRunTRTID = _TRT_CUP_REFUND_CTLS_;
		}
		else if (pobTran->inTransactionCode == _CUP_PRE_AUTH_)
		{
			pobTran->inRunTRTID = _TRT_CUP_PRE_AUTH_CTLS_;
		}
		else if (pobTran->inTransactionCode == _CUP_PRE_COMP_)
		{
			pobTran->inRunTRTID = _TRT_CUP_PRE_COMP_CTLS_;
		}
		else if (pobTran->inTransactionCode == _CUP_INST_SALE_)
		{
			pobTran->inRunTRTID = _TRT_CUP_INST_SALE_CTLS_;
		}
		else if (pobTran->inTransactionCode == _CUP_INST_REFUND_)
		{
			pobTran->inRunTRTID = _TRT_CUP_INST_REFUND_CTLS_;
		}
		else if (pobTran->inTransactionCode == _CUP_REDEEM_SALE_)
		{
			pobTran->inRunTRTID = _TRT_CUP_REDEEM_SALE_CTLS_;
		}
		else if (pobTran->inTransactionCode == _CUP_REDEEM_REFUND_)
		{
			pobTran->inRunTRTID = _TRT_CUP_REDEEM_REFUND_CTLS_;
		}
		else if (pobTran->inTransactionCode == _CUP_MAIL_ORDER_REFUND_)
		{
			pobTran->inRunTRTID = _TRT_CUP_MAIL_ORDER_REFUND_CTLS_;
		}
		else if (pobTran->inTransactionCode == _LOYALTY_REDEEM_)
		{
			pobTran->inRunTRTID = _TRT_LOYALTY_REDEEM_;
		}
		else if (pobTran->inTransactionCode == _LOYALTY_REDEEM_REFUND_)
		{
			pobTran->inRunTRTID = _TRT_LOYALTY_REDEEM_REFUND_;
		}
		else if (pobTran->inTransactionCode == _VOID_LOYALTY_REDEEM_)
		{
			pobTran->inRunTRTID = _TRT_VOID_LOYALTY_REDEEM_;
		}
		else
		{
			/* 找不到對應的TRT */
			pobTran->inErrorMsg = _ERROR_CODE_V3_TRT_NOT_FOUND_;

			if (ginDebug == VS_TRUE)
			{
				memset(szDebug, 0x00, sizeof(szDebug));
				sprintf(szDebug, "AID_Select But Cat not Select TRT 13, inCode: %d", pobTran->inTransactionCode);
				inDISP_LogPrintf(szDebug);
			}

			return (VS_ERROR);
		}
	}
	else
	{
		if (pobTran->inTransactionCode == _SALE_)
		{
			pobTran->inRunTRTID = _TRT_SALE_CTLS_;
		}
		else if (pobTran->inTransactionCode == _REFUND_)
		{
			pobTran->inRunTRTID = _TRT_REFUND_CTLS_;
		}
		else if (pobTran->inTransactionCode == _PRE_AUTH_)
		{
			pobTran->inRunTRTID = _TRT_PRE_AUTH_CTLS_;
		}
		else if (pobTran->inTransactionCode == _PRE_COMP_)
		{
			pobTran->inRunTRTID = _TRT_PRE_COMP_CTLS_;
		}
		else if (pobTran->inTransactionCode == _SALE_OFFLINE_)
		{
			pobTran->inRunTRTID = _TRT_SALE_OFFLINE_CTLS_;
		}
		else if (pobTran->inTransactionCode == _INST_SALE_)
		{
			pobTran->inRunTRTID = _TRT_INST_SALE_CTLS_;
		}
		else if (pobTran->inTransactionCode == _INST_REFUND_)
		{
			pobTran->inRunTRTID = _TRT_INST_REFUND_CTLS_;
		}
		else if (pobTran->inTransactionCode == _INST_ADJUST_)
		{
			pobTran->inRunTRTID = _TRT_INST_ADJUST_CTLS_;
		}
		else if (pobTran->inTransactionCode == _REDEEM_SALE_)
		{
			pobTran->inRunTRTID = _TRT_REDEEM_SALE_CTLS_;
		}
		else if (pobTran->inTransactionCode == _REDEEM_REFUND_)
		{
			pobTran->inRunTRTID = _TRT_REDEEM_REFUND_CTLS_;
		}
		else if (pobTran->inTransactionCode == _REDEEM_ADJUST_)
		{
			pobTran->inRunTRTID = _TRT_REDEEM_ADJUST_CTLS_;
		}
		else if (pobTran->inTransactionCode == _LOYALTY_REDEEM_)
		{
			pobTran->inRunTRTID = _TRT_LOYALTY_REDEEM_;
		}
		else if (pobTran->inTransactionCode == _LOYALTY_REDEEM_REFUND_)
		{
			pobTran->inRunTRTID = _TRT_LOYALTY_REDEEM_REFUND_;
		}
		else if (pobTran->inTransactionCode == _VOID_LOYALTY_REDEEM_)
		{
			pobTran->inRunTRTID = _TRT_VOID_LOYALTY_REDEEM_;
		}
		else
		{
			/* 找不到對應的TRT */
			pobTran->inErrorMsg = _ERROR_CODE_V3_TRT_NOT_FOUND_;

			if (ginDebug == VS_TRUE)
			{
				memset(szDebug, 0x00, sizeof(szDebug));
				sprintf(szDebug, "AID_Select But Cat not Select TRT 14, inCode: %d", pobTran->inTransactionCode);
				inDISP_LogPrintf(szDebug);
			}

			return (VS_ERROR);
		}
	}

	return (VS_SUCCESS);
}


/*
Function        : inFunc_CheckCathayHostEnable
Date&Time   : 2019/06/25
Describe        : 因為版本會使用用主機名稱分流，如果檔案沒更新會有問題所以新增判斷是否有定義版本 判斷 
*/

int inFunc_CheckCathayHostEnable( void )
{
	int inResult = VS_ERROR;
	
	return inResult;
}


/*
Function	:inFunc_Data_Chmod
Date&Time	:2018/9/5 上午 11:45
Describe        :更改檔案權限
*/
int inFunc_Data_Chmod(char* szParameter, char *szFileName, char* szSource)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1] = {0};
	char	szCommand[100 + 1] = {0};	/* Shell Command*/
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfWithFlag("inFunc_Data_Chmod START!");
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme: %s", szFileName);
		inDISP_LogPrintfWithFlag(szDebugMsg);
	}
		
	/* rm 資料夾路徑/檔名 */
	/* 組命令 */
	memset(szCommand, 0x00, sizeof(szCommand));
	if (szFileName != NULL)
	{
		sprintf(szCommand, "chmod  ");
		
		if (szParameter != NULL)
		{
			strcat(szCommand, szParameter);
			strcat(szCommand, " ");
		}
		
		if (szSource != NULL)
		{
			strcat(szCommand, szSource);
		}

		/* 目標檔名 */
		strcat(szCommand, szFileName);
	}
	else
	{
		return (VS_ERROR);
	}
	
	/* 執行命令 */
        inRetVal = inFunc_ShellCommand_Popen(szCommand);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme : %s", szFileName);
		inDISP_LogPrintfWithFlag(szDebugMsg);
		
		inDISP_LogPrintfWithFlag("inFunc_Data_Chmod END!");
	}
	
	return (VS_SUCCESS);
}


/*
Function	:inFunc_Data_Rename
Date&Time	:2018/5/25 下午 4:02
Describe        :更改檔名
*/
int inFunc_Data_Rename(char *szOldFileName, char* szSource, char *szNewFileName, char* szDestination)
{
	int	inRetVal = VS_SUCCESS;
	char	szCommand[100 + 1] = {0};	/* Shell Command*/
	
	inDISP_LogPrintfWithFlag("inFunc_Rename START!");
	inDISP_LogPrintfWithFlag("FNaeme: %s", szOldFileName);

	/* 組命令 */
	if (strlen(szOldFileName) != 0)
	{
		/* mv szSource/szOldFileName szDestination/szNewFileName */
		memset(szCommand, 0x00, sizeof(szCommand));
		sprintf(szCommand, "mv ");
		if (strlen(szSource) != 0)
		{
			strcat(szCommand, szSource);
		}
		
		strcat(szCommand, szOldFileName);
		
		/* 空格 */
		strcat(szCommand, " ");
		
		if (strlen(szDestination) != 0)
		{
			strcat(szCommand, szDestination);
		}
		
		if (strlen(szNewFileName) != 0)
		{
			strcat(szCommand, szNewFileName);
		}
	}
	/* 沒有來源檔 */
	else
	{
		return (VS_ERROR);
	}
	
	/* 執行命令 */
	inRetVal = inFunc_ShellCommand_Popen(szCommand);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	inDISP_LogPrintfWithFlag("NewFNaeme : %s", szNewFileName);

	inDISP_LogPrintfWithFlag("inFunc_Data_Rename END!");

	return (VS_SUCCESS);
}


/*
Function        : inFunc_Check_AuthCode_Validate
Date&Time   : 2020/2/25 下午 1:29
Describe        :
*/
int inFunc_Check_AuthCode_Validate(char* szAuthCode)
{
	int	i = 0;
	int	inRetVal = VS_SUCCESS;
	
	if (szAuthCode == NULL)
	{
		return (VS_SUCCESS);
	}
	
	for (i = 0; i < strlen(szAuthCode); i++)
	{
		if (((szAuthCode[i] >= '0') && (szAuthCode[i] <= '9')) ||
		    ((szAuthCode[i] >= 'A') && (szAuthCode[i] <= 'Z')) ||
		    ((szAuthCode[i] >= 'a') && (szAuthCode[i] <= 'z')) ||
		    (szAuthCode[i] == 0x20))
		{
			continue;
		}
		else
		{
			inRetVal = VS_ERROR;
			break;
		}
	}
	
	return (inRetVal);
}



/*
Function        : inFunc_Log_All_Firmware_Version
Date&Time   : 2020/2/24 下午 3:12
Describe        : 使用在自助刷卡機 SDK
*/
int inFunc_Log_All_Firmware_Version(void)
{
	int	inRetVal = VS_SUCCESS;
	int	i = 0;
	int	inID = 0;
	char	szIDName[20 + 1] = {0};
	char	szBuffer[50 + 1] = {0};
	
	for (i = 0; i <= ID_MAXIMUM; i++)
	{
		inID = i;
		memset(szIDName, 0x00, sizeof(szIDName));
		switch (inID)
		{
			case ID_BOOTSULD:
				sprintf(szIDName, "%s", "BOOTSULD");
				break;
			case ID_CRYPTO_HAL:
				sprintf(szIDName, "%s", "CRYPTO_HAL");
				break;
			case ID_LINUX_KERNEL:
				sprintf(szIDName, "%s", "LINUX_KERNEL");
				break;
			case ID_SECURITY_KO:
				sprintf(szIDName, "%s", "SECURITY_KO");
				break;
			case ID_SYSUPD_KO:
				sprintf(szIDName, "%s", "SYSUPD_KO");
				break;
			case ID_KMS:
				sprintf(szIDName, "%s", "KMS");
				break;
			case ID_CADRV_KO:
				sprintf(szIDName, "%s", "CADRV_KO");
				break;
			case ID_CAUSB_KO:
				sprintf(szIDName, "%s", "CAUSB_KO");
				break;
			case ID_LIBCAUART_SO:
				sprintf(szIDName, "%s", "LIBCAUART_SO");
				break;
			case ID_LIBCAUSBH_SO:
				sprintf(szIDName, "%s", "LIBCAUSBH_SO");
				break;
			case ID_LIBCAMODEM_SO:
				sprintf(szIDName, "%s", "LIBCAMODEM_SO");
				break;
			case ID_LIBCAETHERNET_SO:
				sprintf(szIDName, "%s", "LIBCAETHERNET_SO");
				break;
			case ID_LIBCAFONT_SO:
				sprintf(szIDName, "%s", "LIBCAFONT_SO");
				break;
			case ID_LIBCALCD_SO:
				sprintf(szIDName, "%s", "LIBCALCD_SO");
				break;
			case ID_LIBCAPRT_SO:
				sprintf(szIDName, "%s", "LIBCAPRT_SO");
				break;
			case ID_LIBCARTC_SO:
				sprintf(szIDName, "%s", "LIBCARTC_SO");
				break;
			case ID_LIBCAULDPM_SO:
				sprintf(szIDName, "%s", "LIBCAULDPM_SO");
				break;
			case ID_LIBCAPMODEM_SO:
				sprintf(szIDName, "%s", "LIBCAPMODEM_SO");
				break;
			case ID_LIBCAGSM_SO:
				sprintf(szIDName, "%s", "LIBCAGSM_SO");
				break;
			case ID_LIBCAEMVL2_SO:
				sprintf(szIDName, "%s", "LIBCAEMVL2_SO");
				break;
			case ID_LIBCAKMS_SO:
				sprintf(szIDName, "%s", "LIBCAKMS_SO");
				break;
			case ID_LIBCAFS_SO:
				sprintf(szIDName, "%s", "LIBCAFS_SO");
				break;
			case ID_LIBCABARCODE_SO:
				sprintf(szIDName, "%s", "LIBCABARCODE_SO");
				break;
			case ID_CRADLE_MP:
				sprintf(szIDName, "%s", "CRADLE_MP");
				break;
			case ID_LIBTLS_SO:
				sprintf(szIDName, "%s", "LIBTLS_SO");
				break;
			case ID_LIBCLVW_SO:
				sprintf(szIDName, "%s", "LIBCLVW_SO");
				break;
			case ID_LIBCTOSAPI_SO:
				sprintf(szIDName, "%s", "LIBCTOSAPI_SO");
				break;
			case ID_SAM_KO:
				sprintf(szIDName, "%s", "SAM_KO");
				break;
			case ID_CLVWM_MP:
				sprintf(szIDName, "%s", "CLVWM_MP");
				break;
			case ID_ROOTFS:
				sprintf(szIDName, "%s", "ROOTFS");
				break;
			case ID_BIOS:
				sprintf(szIDName, "%s", "BIOS");
				break;
			case ID_CIF_KO:
				sprintf(szIDName, "%s", "CIF_KO");
				break;
			case ID_CLDRV_KO:
				sprintf(szIDName, "%s", "CLDRV_KO");
				break;
			case ID_TMS:
				sprintf(szIDName, "%s", "TMS");
				break;
			case ID_ULDPM:
				sprintf(szIDName, "%s", "ULDPM");
				break;
			case ID_SC_KO:
				sprintf(szIDName, "%s", "SC_KO");
				break;
			case ID_EMV_SO:
				sprintf(szIDName, "%s", "EMV_SO");
				break;
			case ID_EMVCL_SO:
				sprintf(szIDName, "%s", "EMVCL_SO");
				break;
			default	:
				break;
		}
		
		memset(szBuffer, 0x00, sizeof(szBuffer));
		inRetVal = inFunc_GetSystemInfo(inID, (unsigned char*)szBuffer);
		if (inRetVal == VS_SUCCESS)
		{
			inDISP_LogPrintfWithFlag("%s:%s", szIDName, szBuffer);
		}
	}
	
	return (VS_SUCCESS);
}

/* 抓取設定此交易是否為Kiosk要跳過停留步驟的參數 2020/3/6 下午 4:21 [SAM] */
int inFunc_GetKisokFlag()
{	
//	inDISP_LogPrintfWithFlag("  st_chSkipPressButton [%c]", st_chSkipPressButton);
	if(st_chSkipPressButton == 'Y')
		return VS_TRUE;
	else
		return VS_FALSE;
}

/* 設定此交易是否為Kiosk要跳過停留步驟的參數  2020/3/6 下午 4:21 [SAM] */
int inFunc_SetKisokFlag(char* chSetValue)
{
/* 只有富邦版本才能設定，不用都算不啟用 2020/3/9 下午 4:03 [SAM] */
#ifdef _FUBON_MAIN_HOST_	
	//inDISP_LogPrintfWithFlag("  SetVale[%u]", chSetValue[0]);
	st_chSkipPressButton =  chSetValue[0];
#endif
	return VS_SUCCESS;
}

/*
Function        : inFUNC_SetEcrErrorMag
Date&Time   : 2021/1/24 下午 7:37
Describe        : 設定要在對應的 Pack_ResponseCode 裏使用，設定值請用 
 *                      ECR.h 裏的 "For ECR Transaction Response Code" 列表
*/
int inFUNC_SetEcrErrorMag(TRANSACTION_OBJECT *pobTran, int inErrorType)
{   
        pobTran->inECRErrorMsg = inErrorType;
        return VS_SUCCESS;
}



/*
Function        : inFUNC_MeunUiSetAllHdtDataInLoop
Date&Time   :  新增設定主機功能 2021/1/20 下午 12:09 [SAM] 
Describe        : 選擇Host選單，如果只有一個主機就直接LoadHDT和HDPT，選定HostNum，並load HDT和HDPT
 *                      並設定HDT的資料
*/
int inFUNC_MeunUiSetAllHdtDataInLoop()
{
        int inOpenHostCnt = 0;      /* 記錄有幾個Host有開 */
        int inLine = 0;             /* 第幾行 */
        int i, j = 0;               /* j是inHostIndex陣列索引 */
        int inHostIndex[12 + 1];    /* 記錄HostEnable為Y的HostIndex */
        int inLine1Index = 0;       /* szLine1的index */
        int inLine2Index = 0;       /* szLine2的index */
        int inLine3Index = 0;       /* szLine3的index */
        int inLine4Index = 0;
        int inLine5Index = 0;
        int inLine6Index = 0;
        int inKey = 0;
        int inChoice = 0;
        int inTouchSensorFunc = _Touch_NEWUI_CHOOSE_HOST_;
        int inRetVal = VS_SUCCESS;
        char    szKey = 0;
        char    szHostEnable[2 + 1] = {0};
        char	szHostName[42 + 1] = {0};
        char	szTemp[48 + 1] = {0};
        char	szLine1[48 + 1] = {0};		/* 存第一行要顯示的Host */	/* linux系統中文字length一個字為3，小心爆掉 */
        char	szLine2[48 + 1] = {0};		/* 存第二行要顯示的Host */
        char	szLine3[48 + 1] = {0};		/* 存第三行要顯示的Host */
        char	szLine4[48 + 1] = {0};
        char	szLine5[48 + 1] = {0};
        char	szLine6[48 + 1] = {0};
        char	szTemp2[48 + 1] = {0};
        char	szLine1_2[48 + 1] = {0};	/* 存第一行要顯示的Host */	/* linux系統中文字length一個字為3，小心爆掉 */
        char	szLine2_2[48 + 1] = {0};	/* 存第二行要顯示的Host */
        char	szLine3_2[48 + 1] = {0};	/* 存第三行要顯示的Host */
        char	szLine4_2[48 + 1] = {0};
        char	szLine5_2[48 + 1] = {0};
        char	szLine6_2[48 + 1] = {0};
        char	szTimeout[4 + 1] = {0};
//        char	szDebugMsg[42 + 1] = {0};
        char    szTidTemp[32+1] = {0};
        DISPLAY_OBJECT  srDispObj;
	     
        /* 以上是特例，如果都沒有就手動選Host */
        memset(szLine1, 0x00, sizeof(szLine1));
        memset(szLine2, 0x00, sizeof(szLine2));
        memset(szLine3, 0x00, sizeof(szLine3));
        memset(szLine4, 0x00, sizeof(szLine4));
        memset(szLine5, 0x00, sizeof(szLine5));
        memset(szLine6, 0x00, sizeof(szLine6));
        memset(szLine1_2, 0x00, sizeof(szLine1_2));
        memset(szLine2_2, 0x00, sizeof(szLine2_2));
        memset(szLine3_2, 0x00, sizeof(szLine3_2));
        memset(szLine4_2, 0x00, sizeof(szLine4_2));
        memset(szLine5_2, 0x00, sizeof(szLine5_2));
        memset(szLine6_2, 0x00, sizeof(szLine6_2));
        memset(szTimeout, 0x00, sizeof(szTimeout));
        memset(inHostIndex, 0x00, sizeof(inHostIndex));
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        
        inDISP_ClearAll();
        inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
        inDISP_PutGraphic(_MENU_STE_TERMINAL_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);/* 第一層顯示 端末機設定 */

        for (i = 0;; ++i)
        {
                /* 先LoadHDT */
                if (inLoadHDTRec(i) == VS_ERROR)
                {
                        /* 當找不到第i筆資料會回傳VS_ERROR */
                        break;
                }
		
                /* 修正未支援的主機不應顯示在選單上 */
                memset(szHostName, 0x00, sizeof(szHostName));
                inGetHostLabel(szHostName);
                if (memcmp(szHostName, _HOST_NAME_ESC_, strlen(_HOST_NAME_ESC_)) == 0 ||
                    memcmp(szHostName, "MO", 2) == 0 ||
                    memcmp(szHostName, "OILCASH", 7) == 0 ||
                    memcmp(szHostName, "SC", 2) == 0 ||
                    memcmp(szHostName, "SCINST", 6) == 0 ||
                    memcmp(szHostName, "EGIFT", 5) == 0 ||
                    memcmp(szHostName, "CMAS", 4) == 0 ||
                    memcmp(szHostName, "ESUNPARK", 8) == 0 ||    
                    memcmp(szHostName, "TSBPARK", 7) == 0 ||
                    memcmp(szHostName, "CBPARK", 6) == 0 ||    
                    memcmp(szHostName, "ALIPAY", 6) == 0 ||
                    memcmp(szHostName, "DINERS", 6) == 0) /* 調整DINERS尚未開發，不顯示在主機別上面。*/    
                {
                        continue;
                }
		
                if (ginAPVersionType == _APVERSION_TYPE_CUB_ ||
                    ginAPVersionType == _APVERSION_TYPE_TCB_ ||                        
                    ginAPVersionType == _APVERSION_TYPE_ESUN_ )
                {
                        if (memcmp(szHostName, _HOST_NAME_CUP_, strlen(_HOST_NAME_CUP_)) == 0)
                        {
                                continue;
                        }
                }
            
                if (ginAPVersionType == _APVERSION_TYPE_TCB_ ||
                    ginAPVersionType == _APVERSION_TYPE_ESUN_ )
                {
                        memset(szHostName, 0x00, sizeof(szHostName));
                        inGetHostLabel(szHostName);
                        if (memcmp(szHostName, _HOST_NAME_REDEEM_, strlen(_HOST_NAME_REDEEM_)) == 0)
                                continue;
                }
                
                /* GET HOST Enable */
                memset(szHostEnable, 0x00, sizeof(szHostEnable));
                if (inGetHostEnable(szHostEnable) == VS_ERROR)
                {	
                        return (VS_ERROR);
                }
                else
                {
                    
                        inDISP_LogPrintfWithFlag(" [%d] HostEnable: [%s] Line[%d]", i, szHostEnable, __LINE__);
                        
                        if (memcmp(szHostEnable, "Y", 1) != 0)
                        {
                                /* 如果HostEnable != Y，就continue */
                                continue;
                        }

                        /* 如果主機有開，才loadHDPT */
                        if (inLoadHDPTRec(i) == VS_ERROR)
                        {
                                /* 當找不到第i筆資料會回傳VS_ERROR */
                                return (VS_ERROR);
                        }

                        inOpenHostCnt ++;       /* 記錄有幾個Host有開 */
                        inLine ++;              /* 第幾行 */

                        /* 記錄HostEnable為Y的HostIndex，減1是因為HostIndex從01開始 */
                        inHostIndex[j] = i;
                        j++;

                        memset(szHostName, 0x00, sizeof(szHostName));
                        inGetHostLabel(szHostName);
			
                        /* 用szTRTFileName來決定要顯示的Host Name */
                        memset(szTemp, 0x00, sizeof(szTemp));
                        inFunc_DiscardSpace(szHostName);
                        /* ESVC要顯示電子票證 */
                        if (memcmp(szHostName, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)) == 0)
                        {
                                sprintf(szTemp, "%s", "電子票");
                                memset(szTemp2, 0x00, sizeof(szTemp2));
                                sprintf(szTemp2, "證   %d", inOpenHostCnt);
                        }
                        else
                        {
                                if (ginAPVersionType == _APVERSION_TYPE_TCB_ ||
                                    ginAPVersionType == _APVERSION_TYPE_ESUN_ )
                                {
                                        if (memcmp(szHostName, _HOST_NAME_CREDIT_MAINLY_USE_, strlen(_HOST_NAME_CREDIT_MAINLY_USE_)) == 0)
                                        {
                                                sprintf(szTemp, "%s", "信用卡");
                                                memset(szTemp2, 0x00, sizeof(szTemp2));
                                                sprintf(szTemp2, "     %d", inOpenHostCnt);
                                        }
                                        else if (memcmp(szHostName, _HOST_NAME_INST_, strlen(_HOST_NAME_INST_)) == 0)
                                        {
                                                sprintf(szTemp, "%s", "分期");
                                                memset(szTemp2, 0x00, sizeof(szTemp2));
                                                sprintf(szTemp2, "付款 %d", inOpenHostCnt);
                                        }        
                                        else if (memcmp(szHostName, _HOST_NAME_REDEEM_, strlen(_HOST_NAME_REDEEM_)) == 0)
                                        {
                                                sprintf(szTemp, "%s", "紅利");
                                                memset(szTemp2, 0x00, sizeof(szTemp2));
                                                sprintf(szTemp2, "扣抵 %d", inOpenHostCnt);
                                        }
                                        else if (memcmp(szHostName, _HOST_NAME_AMEX_, strlen(_HOST_NAME_AMEX_)) == 0)
                                        {
                                                sprintf(szTemp, "%s", "美國");
                                                memset(szTemp2, 0x00, sizeof(szTemp2));
                                                sprintf(szTemp2, "運通 %d", inOpenHostCnt);
                                        }
                                        else if (memcmp(szHostName, _HOST_NAME_DINERS_, strlen(_HOST_NAME_DINERS_)) == 0)
                                        {
                                                sprintf(szTemp, "%s", "大來");
                                                memset(szTemp2, 0x00, sizeof(szTemp2));
                                                sprintf(szTemp2, "     %d", inOpenHostCnt);
                                        }
                                        else
                                        {
                                                sprintf(szTemp, "%s", szHostName);
                                                memset(szTemp2, 0x00, sizeof(szTemp2));
                                                sprintf(szTemp2, "     %d", inOpenHostCnt);
                                        }        
                                }
                                else
                                {        
                                        sprintf(szTemp, "%s", szHostName);
                                        memset(szTemp2, 0x00, sizeof(szTemp2));
                                        sprintf(szTemp2, "     %d", inOpenHostCnt);
                                }        
                        }
			
                        /* 每一行顯示的內容先存在陣列裡 */
                        switch (inLine)
                        {
                                case 1:
                                        memcpy(&szLine1[inLine1Index], szTemp, strlen(szTemp));
                                        inLine1Index += strlen(szTemp);
                                        memcpy(szLine1_2, szTemp2, strlen(szTemp2));
                                        break;
                                case 2:
                                        memcpy(&szLine2[inLine2Index], szTemp, strlen(szTemp));
                                        inLine2Index += strlen(szTemp);
                                        memcpy(szLine2_2, szTemp2, strlen(szTemp2));
                                        break;
                                case 3:
                                        memcpy(&szLine3[inLine3Index], szTemp, strlen(szTemp));
                                        inLine3Index += strlen(szTemp);
                                        memcpy(szLine3_2, szTemp2, strlen(szTemp2));
                                        break;
                                case 4:
                                        memcpy(&szLine4[inLine4Index], szTemp, strlen(szTemp));
                                        inLine4Index += strlen(szTemp);
                                        memcpy(szLine4_2, szTemp2, strlen(szTemp2));
                                        break;
                                case 5:
                                        memcpy(&szLine5[inLine5Index], szTemp, strlen(szTemp));
                                        inLine5Index += strlen(szTemp);
                                        memcpy(szLine5_2, szTemp2, strlen(szTemp2));
                                        break;
                                case 6:
                                        memcpy(&szLine6[inLine6Index], szTemp, strlen(szTemp));
                                        inLine6Index += strlen(szTemp);
                                        memcpy(szLine6_2, szTemp2, strlen(szTemp2));
                                        break;
                                default:
                                        break;
                        }
                }
		
        }/* End of For loop */

        /* 當inOpenHostCnt = 0，表示主機都沒開或者inLoadHDT有問題 */
        if (inOpenHostCnt == 0)
        {
                /* 主機選擇錯誤 */
                inDISP_Msg_BMP(_ERR_CHOOSE_HOST_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
                return (VS_ERROR);
        }

        /* 只有開一個Host */
        if (inOpenHostCnt == 1)
        {
                if (inLoadHDTRec(inHostIndex[0]) == VS_ERROR)
                {
                        /* 主機選擇錯誤 */
                        inDISP_Msg_BMP(_ERR_CHOOSE_HOST_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
                        return (VS_ERROR);
                }
                
                inHDT_Edit_Part_Of_HDT_Table(0);
                
                inGetTerminalID(szTidTemp);
                inDISP_LogPrintfWithFlag(" Menu Set Flag TID[%s] Line[%d] ", szTidTemp, __LINE__);
                /* 判斷信用卡主機是否為初始值，如果已修改，就可以進行交易 2021/2/17 下午 2:17 [SAM] */
                if(memcmp(szTidTemp, "'00000001", 8 ))
                {
                        inSetTMSOK("Y");
                        inSaveEDCRec(0);
                }
                
                inRetVal = VS_SUCCESS;
        }
        else
        {   
                do
                {
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

                        switch (inOpenHostCnt)
                        {
                                case	2:
                                    inDISP_PutGraphic(_CHOOSE_HOST_2_, 0,_COORDINATE_Y_LINE_8_4_);
                                    break;

                                case	3:
                                    inDISP_PutGraphic(_CHOOSE_HOST_3_, 0,_COORDINATE_Y_LINE_8_4_);
                                    break;

                                case	4:
                                    inDISP_PutGraphic(_CHOOSE_HOST_4_, 0,_COORDINATE_Y_LINE_8_4_);
                                    break;

                                case	5:
                                    inDISP_PutGraphic(_CHOOSE_HOST_5_, 0,_COORDINATE_Y_LINE_8_4_);
                                    break;

                                case	6:
                                    inDISP_PutGraphic(_CHOOSE_HOST_6_, 0,_COORDINATE_Y_LINE_8_4_);
                                    break;

                                default:
                                    inDISP_PutGraphic(_CHOOSE_HOST_6_, 0,_COORDINATE_Y_LINE_8_4_);
                                    break;
                        }

                        /*有開多個Host */
                        inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine1, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
                        inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine1_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);

                        inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
                        inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine2_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);

                        inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine3, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
                        inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine3_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);

                        inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine4, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_13_, VS_FALSE);
                        inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine4_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_14_, VS_FALSE);

                        inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine5, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_13_, VS_FALSE);
                        inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine5_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_14_, VS_FALSE);

                        inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine6, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_16_13_, VS_FALSE);
                        inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine6_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_16_14_, VS_FALSE);

                        
                        
                        inDISP_LogPrintfWithFlag(" Line1[%s] Line[%d] ", szLine1, __LINE__);
                        inDISP_LogPrintfWithFlag(" Line2[%s] Line[%d] ", szLine2, __LINE__);
                        inDISP_LogPrintfWithFlag(" Line3[%s] Line[%d] ", szLine3, __LINE__);
                        inDISP_LogPrintfWithFlag(" Line4[%s] Line[%d] ", szLine4, __LINE__);
                        inDISP_LogPrintfWithFlag(" Line5[%s] Line[%d] ", szLine5, __LINE__);
                        inDISP_LogPrintfWithFlag(" Line6[%s] Line[%d] ", szLine6, __LINE__);
                        
                        inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

                        while (1)
                        {
                                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                                szKey = uszKBD_Key();

                                /* 轉成數字判斷是否在inOpenHostCnt的範圍內 */
                                inKey = 0;
                                /* 有觸摸*/
                                if (inChoice != _DisTouch_No_Event_)
                                {
                                        switch (inChoice)
                                        {
                                            case _NEWUI_CHOOSE_HOST_Touch_HOST_1_:
                                                inKey = 1;
                                                break;
                                            case _NEWUI_CHOOSE_HOST_Touch_HOST_2_:
                                                inKey = 2;
                                                break;
                                            case _NEWUI_CHOOSE_HOST_Touch_HOST_3_:
                                                inKey = 3;
                                                break;
                                            case _NEWUI_CHOOSE_HOST_Touch_HOST_4_:
                                                inKey = 4;
                                                break;
                                            case _NEWUI_CHOOSE_HOST_Touch_HOST_5_:
                                                inKey = 5;
                                                break;
                                            case _NEWUI_CHOOSE_HOST_Touch_HOST_6_:
                                                inKey = 6;
                                                break;
                                            default:
                                                inKey = 0;
                                                break;
                                            }
                                }
                                /* 有按按鍵 */
                                else if (szKey != 0)
                                {
                                        switch (szKey)
                                        {
                                                case _KEY_1_:
                                                    inKey = 1;
                                                    break;
                                                case _KEY_2_:
                                                    inKey = 2;
                                                    break;
                                                case _KEY_3_:
                                                    inKey = 3;
                                                    break;
                                                case _KEY_4_:
                                                    inKey = 4;
                                                    break;
                                                case _KEY_5_:
                                                    inKey = 5;
                                                    break;
                                                case _KEY_6_:
                                                    inKey = 6;
                                                    break;
                                                default:
                                                    inKey = 0;
                                                    break;
                                        }
                                }

                                /* Timeout */
                                if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
                                {
                                        szKey = _KEY_TIMEOUT_;
                                }

                                if (szKey == _KEY_CANCEL_)
                                {
                                        inRetVal = VS_USER_CANCEL;
                                        break;
                                }
                                else if (szKey == _KEY_TIMEOUT_)
                                {
                                        inRetVal = VS_TIMEOUT;
                                        break;
                                }
                                else if (inKey >= 1 && inKey <= inOpenHostCnt)
                                {
                                        if (inLoadHDTRec(inHostIndex[inKey - 1]) == VS_ERROR)
                                        {
                                                /* 主機選擇錯誤 */
                                                inDISP_Msg_BMP(_ERR_CHOOSE_HOST_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
                                                inRetVal = VS_ERROR;
                                                break;
                                        }

                                        if (inLoadHDPTRec(inHostIndex[inKey - 1]) == VS_ERROR)
                                        {
                                                /* 主機選擇錯誤 */
                                                inDISP_Msg_BMP(_ERR_CHOOSE_HOST_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

                                                inRetVal = VS_ERROR;
                                                break;
                                        }
                                        else
                                        {
                                                inHDT_Edit_Part_Of_HDT_Table(inHostIndex[inKey - 1]);
                                                inDISP_LogPrintfWithFlag(" Menu Set Flag ID[%d] Line[%d] ", inHostIndex[inKey - 1], __LINE__);
                                                /* 判斷信用卡主機是否為初始值，如果已修改，就可以進行交易 2021/2/17 下午 2:17 [SAM] */
                                                /* 因為信用卡主機都會是第0個位置，所以一定會開啟 */
                                                if(inHostIndex[inKey - 1] == 0)
                                                {
                                                        memset(szTidTemp, 0x00, sizeof(szTidTemp));
                                                        inGetTerminalID(szTidTemp);
                                                        inDISP_LogPrintfWithFlag(" Menu Set Flag TID[%s] Line[%d] ", szTidTemp, __LINE__);
                                                        if(memcmp(szTidTemp, "'00000001", 8 ))
                                                        {
                                                                inSetTMSOK("Y");
                                                                inSaveEDCRec(0);
                                                        }
                                                }
                                                
                                                inRetVal = VS_SUCCESS;
                                                break;
                                        }
                                }
                        }/* end while (1) */
                        
                        /* 清空Touch資料 */
                        inDisTouch_Flush_TouchFile();
                        
                }while(inRetVal == VS_SUCCESS);
        }

        return (inRetVal);
}

/*
Function        : inFunc_Sync_TRec_Date_Time_CMAS
Date&Time   : 2022/6/22 上午 9:38
Describe        : 同步TRec日期和時間
 * [新增電票悠遊卡功能]  [SAM] 
*/
int inFunc_Sync_TRec_Date_Time_CMAS(TRANSACTION_OBJECT *pobTran, RTC_NEXSYS *srRTC)
{
	/* 同步到pobTran */
	memset(pobTran->srTRec.szDate, 0x00, sizeof(pobTran->srTRec.szDate));
	memset(pobTran->srTRec.szOrgDate, 0x00, sizeof(pobTran->srTRec.szOrgDate));
	memset(pobTran->srTRec.szTime, 0x00, sizeof(pobTran->srTRec.szTime));
	memset(pobTran->srTRec.szOrgTime, 0x00, sizeof(pobTran->srTRec.szOrgTime));

	if (inFunc_SyncPobTran_Date_Include_Year(pobTran->srTRec.szDate, sizeof(pobTran->srTRec.szDate), srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (inFunc_SyncPobTran_Date_Include_Year(pobTran->srTRec.szOrgDate, sizeof(pobTran->srTRec.szOrgDate), srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (inFunc_SyncPobTran_Time(pobTran->srTRec.szTime, sizeof(pobTran->srTRec.szTime), srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (inFunc_SyncPobTran_Time(pobTran->srTRec.szOrgTime, sizeof(pobTran->srTRec.szOrgTime), srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}


int inFunc_TestBase64()
{
#if 0
	char *szTestData, *szOut;
	char szFileName[32] = {0};
	char szInput[128] = {0};
	char b64string[] = "1234567890-=poiuytrewqasghjkl;";
	unsigned long ulFileHandle = NULL;
	int inFileSize = 0;
			
	memcpy(szFileName, "Testsign.bmp", 12);
	
	inDISP_PutGraphic("./fs_data/Testsign.bmp", 0,  _COORDINATE_Y_LINE_8_3_);
	
	if (inFILE_Check_Exist((unsigned char*)szFileName) == VS_SUCCESS){
	
		inFunc_Copy_Data(szFileName, _FS_DATA_PATH_, "Testsign_BK.bmp", _FS_DATA_PATH_);
		inDISP_LogPrintfWithFlag("  Aft Copy Backup BMP F_NAME [%s] ",szFileName );

		inFILE_Delete("Testsign.bmp.gz");
		
		if (inFunc_GZip_Data("", szFileName, _FS_DATA_PATH_) != VS_SUCCESS)
		{
			inDISP_LogPrintfWithFlag("  Compress GZip *Error* FileName[%s]  Line[%d]",szFileName, __LINE__);
			return (VS_ERROR);
		}
		
		if (inFILE_Rename((unsigned char*)"Testsign.bmp", (unsigned char*)"Testsign.bmp.gz") != VS_SUCCESS)
		{
			inDISP_LogPrintfWithFlag("  Compress Reame Gz *Error*  Line[%d]",  __LINE__);
			return (VS_ERROR);
		}
		
		
		inFILE_Open(&ulFileHandle, "Testsign.bmp.gz");
		
		inFileSize = lnFILE_GetSize(&ulFileHandle, "Testsign.bmp.gz");
		
		inDISP_LogPrintfWithFlag("  File Len[%d]   Line[%d]",inFileSize,  __LINE__);
		
		szTestData = malloc(inFileSize+ 1);		
		szOut = malloc((2* inFileSize)+ 1);
		
		if (inFILE_Seek(ulFileHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
		{
			inDISP_LogPrintfWithFlag("  Compress Seek 0 *Error*  Line[%d]", __LINE__);
			inFILE_Close(&ulFileHandle);
			free(szTestData);
			free(szOut);
			inFILE_Delete("Testsign.bmp.gz");
			inFILE_Delete("Testsign.bmp");
			inFILE_Rename((unsigned char*)"Testsign_BK.bmp", (unsigned char*)"Testsign.bmp");
			return (VS_ERROR);
		}
		
		if (inFILE_Read(&ulFileHandle, (unsigned char*)szTestData, inFileSize) != VS_SUCCESS)
		{
			inDISP_LogPrintfWithFlag("  Compress File Read *Error* ulHandle[%lu] Line[%d]",ulFileHandle,  __LINE__);
			inFILE_Close(&ulFileHandle);
			free(szTestData);
			free(szOut);
			inFILE_Delete("Testsign.bmp.gz");
			inFILE_Delete("Testsign.bmp");
			inFILE_Rename((unsigned char*)"Testsign_BK.bmp", (unsigned char*)"Testsign.bmp");
			return (VS_ERROR);
		}
		
		inDISP_LogPrintfArea(TRUE, "Testsign.gz = ", 14, (BYTE *) szTestData, inFileSize);
		
		inFunc_Base64_Encryption(szTestData, inFileSize, szOut );
	}
	
	inFILE_Close(&ulFileHandle);
	free(szTestData);
//	inFILE_Delete("Testsign.bmp.gz");
//	inFILE_Delete("Testsign.bmp");
	
	inDISP_LogPrintfWithFlag(" 64Enc [%s]", szOut);
	free(szOut);
	
	if (inFILE_Rename((unsigned char*)"Testsign_BK.bmp", (unsigned char*)"Testsign.bmp") != VS_SUCCESS)
	{
		inDISP_LogPrintfWithFlag("  Compress Reame Gz *Error*  Line[%d]",  __LINE__);
		return (VS_ERROR);
	}
#endif
	return VS_SUCCESS;
}

int inFunc_Print9F1FData(TRANSACTION_OBJECT *pobTran)
{
#if 0
	int inPinrtMode = 0;
	if(pobTran->srBRec.inChipStatus == _EMV_CARD_){
		inPRINT_ChineseFont("晶片交易", _PRT_NORMAL_);
	}
	else if(pobTran->srBRec.uszContactlessBit == VS_TRUE){
		inPRINT_ChineseFont("感應交易", _PRT_NORMAL_);
	}else{
		inPRINT_ChineseFont("磁條交易", _PRT_NORMAL_);
		inPinrtMode = 1;
	}
	if (inPinrtMode == 1)
	{
		inPRINT_LogPrintfArea(FALSE, "CarNo", 5, pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
		inPRINT_LogPrintfArea(FALSE, "9F1F", 4, pobTran->szTrack1, pobTran->shTrack1Len);
	}else{
		inPRINT_LogPrintfArea(FALSE, "CarNo", 5, pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
		inPRINT_LogPrintfArea(FALSE, "9F1F", 4, gusz9F1F, gln9F1FLen);
	}
#endif
	return VS_SUCCESS;		
}

