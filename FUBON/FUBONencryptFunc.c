
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/FUNCTION/Function.h"

#include "../SOURCE/KEY/ProcessTmk.h"

extern int ginDebug;
extern int ginISODebug;

/*
Function        : inFUBON_TMKKeyExchange
Date&Time   : 2016/12/30 下午 1:38
Describe        :
 */
int inFUBON_TMKKeyExchange(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	int inCnt, inLen, inTMKindex, inPINLen, inMACLen;
	char szLen[2 + 1], szTemplate[96 + 1];
	char szKeyExChange[48 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	/* 初始化 */
	inCnt = 0;
	inLen = 0;
	inTMKindex = 0;
	inPINLen = 0;
	inMACLen = 0;

	memset(szLen, 0x00, sizeof (szLen));
	sprintf(szLen, "%x", uszUnPackBuf[1]);

	inLen = atoi(szLen);
	inCnt += 2;

	inDISP_LogPrintfWithFlag(" Exchange Length[%d][0x%02X][0x%02X]", inLen, uszUnPackBuf[0], uszUnPackBuf[1]);

	/* 因為要列印,所以條件還是留著 */
	if (ginISODebug == VS_TRUE)
	{
		inPRINT_ChineseFont("Field_60 Key Exchange", _PRT_ISO_);
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "Length [%d][0x%02X][0x%02X]", inLen, uszUnPackBuf[0], uszUnPackBuf[1]);
		inPRINT_ChineseFont(szTemplate, _PRT_ISO_);
	}

	/* Key Set : Set = _TMK_KEYSET_*/
	/* Key Index : Index = 0x01~0x0F */
	/* mfes預設為第一把Key() */
	inTMKindex = 1;

	inDISP_LogPrintfWithFlag(" TMK Index [%d]", inTMKindex);

	if (ginISODebug == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "TMK Index [%d]", inTMKindex);
		inPRINT_ChineseFont(szTemplate, _PRT_ISO_);
	}

	while (inCnt < inLen)
	{
		switch (uszUnPackBuf[inCnt])
		{
			case 'P':
				inCnt++; /* Key ID */
				/*
					長度僅含Key Length，不含Chack Value)
					First sub-element length; the value of the “length”
					sub-field is always one;.
					BCD length for 範例 : 32 bytes 0x20
				 */

				inPINLen = uszUnPackBuf[inCnt];
				inCnt++; /* Sub Length */

				inDISP_LogPrintfWithFlag(" PIN KEY BCD Lenght [%d][0x%02X]", inPINLen, uszUnPackBuf[inCnt - 1]);

				if (ginISODebug == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof (szTemplate));
					sprintf(szTemplate, "PIN KEY BCD Lenght [%d][0x%02X]", inPINLen, uszUnPackBuf[inCnt - 1]);
					inPRINT_ChineseFont_Format(szTemplate, "  ", 34, _PRT_ISO_);

				}

				if (inPINLen == 32 || inPINLen == 48)
				{
					/* Unpack TPK under TMK, 16(DES)/32(2DES)/48(3DES) */
					memset(szKeyExChange, 0x00, sizeof (szKeyExChange));
					memcpy(szKeyExChange, (char *) &uszUnPackBuf[inCnt], inPINLen);
					inCnt += inPINLen;

					inDISP_LogPrintfWithFlag(" PIN KEY [%s]", szKeyExChange);


					if (ginISODebug == VS_TRUE)
					{
						memset(szTemplate, 0x00, sizeof (szTemplate));
						sprintf(szTemplate, "PIN KEY [%s]", szKeyExChange);
						inPRINT_ChineseFont_Format(szTemplate, "  ", 34, _PRT_ISO_);
					}

					/* Write PINKey */
					if (inNCCC_TMK_Write_PINKey(inTMKindex, inPINLen / 2, szKeyExChange, "") != VS_SUCCESS)
					{
						inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Write PIN Key *Error* END -----", __FILE__, __FUNCTION__, __LINE__);
						return (VS_ERROR);
					}
				} else
				{
					inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Pin Key Len *Error* END -----", __FILE__, __FUNCTION__, __LINE__);
					return (VS_ERROR);
				}

				break;
			case 'M':
				inCnt++; /* Key ID */
				/*
					Sub Length
					(長度僅含Key Length，不含Chack Value)
					First sub-element length; the value of the “length”
					sub-field is always one;.
					BCD length for 範例 : 32 bytes 0x20
				 */
				inMACLen = uszUnPackBuf[inCnt];
				inCnt++; /* Sub Length */

				inDISP_LogPrintfWithFlag(" MAC KEY BCD Lenght [%d][0x%02X]", inMACLen, uszUnPackBuf[inCnt - 1]);

				if (ginISODebug == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof (szTemplate));
					sprintf(szTemplate, "MAC KEY BCD Lenght [%d][0x%02X]", inMACLen, uszUnPackBuf[inCnt - 1]);
					inPRINT_ChineseFont_Format(szTemplate, "  ", 34, _PRT_ISO_);
				}

				if (inMACLen == 32 || inMACLen == 48)
				{
					/* Unpack TPK under TMK, 16(DES)/32(2DES)/48(3DES) */
					memset(szKeyExChange, 0x00, sizeof (szKeyExChange));
					memcpy(&szKeyExChange[0], (char *) &uszUnPackBuf[inCnt], inMACLen);
					inCnt += inMACLen;

					inDISP_LogPrintfWithFlag(" MAC KEY [%s]", szKeyExChange);

					if (ginISODebug == VS_TRUE)
					{
						memset(szTemplate, 0x00, sizeof (szTemplate));
						sprintf(szTemplate, "MAC KEY [%s]", szKeyExChange);
						inPRINT_ChineseFont_Format(szTemplate, "  ", 34, _PRT_ISO_);
					}

					/* Write MAC Key */
					if (inNCCC_TMK_Write_MACKey(inTMKindex, inMACLen / 2, szKeyExChange, "") != VS_SUCCESS)
					{
						inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Write Mac Key *Error* END -----", __FILE__, __FUNCTION__, __LINE__);
						return (VS_ERROR);
					}
				} else
				{
					inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Mac Key Len *Error* END -----", __FILE__, __FUNCTION__, __LINE__);
					return (VS_ERROR);
				}

				break;
			default:
				return (VS_ERROR);
		}
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}


