#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>

#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../FUNCTION/FILE_FUNC/File.h"
#include "../FUNCTION/FILE_FUNC/FIleLogFunc.h"
#include "../INCLUDES/Transaction.h"
#include "../FUNCTION/Function.h"

#include "../DISPLAY/Display.h"

#include "NexEscTableFn.h"


extern  int     ginDebug;	/* Debug使用 extern */
static NEX_ESCT_REC srESCTRec;

/*  設計用來判斷是哪個機型，但需要在開機時呼叫 inESC_SetMachineTypeFunc */
static int	ginNEX_MachineType = 0;


int inESC_SetMachineTypeFunc(int inSetType)
{
	ginNEX_MachineType = inSetType;
	return 0;
}

int inESC_GetMachineTypeFunc(void)
{
	return ginNEX_MachineType;
}

int inESC_DisplayData(char * szTemp, int inLen)
{
	int inTmpCount;
	char szDispBuf[15306];
	char szCopyTemp[4016];
	long lnTmep, lnAddr;
	
	inTmpCount = 0;
	lnAddr = 0;
	memset(szDispBuf, 0x00, sizeof(szDispBuf));
	inESC_DebugLog("----Disp Start len[%d]----", inLen);
	
	inFunc_BCD_to_ASCII(szDispBuf, (unsigned char *)szTemp, inLen);
	
	lnTmep = inLen*2;
	
	inTmpCount = lnTmep / 4000;
	
	while(inTmpCount)
	{
		memset(szCopyTemp, 0x00, sizeof(szCopyTemp));
		memcpy(szCopyTemp,&szDispBuf[lnAddr], 4000);
		inESC_DebugLog(szCopyTemp);
		inTmpCount--;
		lnAddr+= 4000;
	}
	
	inESC_DebugLog(&szDispBuf[lnAddr]);
		
	return 0;
}
	
int inESC_DebugLog(char *szStr,...)
{
	char szDebugMsg[5000 + 1] = {0};
	va_list list;
	
	inDebugFunction = inDISP_LogPrintfWithFlag;
	
	if (inDebugFunction != 0)
	{
		va_start(list, szStr);
		vsprintf(szDebugMsg, szStr, list);
		va_end(list);
		inDebugFunction(szDebugMsg);
	}
	
	return 0;
}


int inParsingData(unsigned char * szSourceData, char* szDestData, int inTotalDataLen, int* inSourecLen, int *inDestLen)
{
	while (1)
	{
		szDestData[*inDestLen ++] = szSourceData[*inSourecLen ++];
		if (szDestData[*inDestLen - 1] == 0x2C	||
			szDestData[*inDestLen - 1] == 0x0D	||
			szDestData[*inDestLen - 1] == 0x0A	||
			szDestData[*inDestLen - 1] == 0x00)
		{
			inESC_DebugLog(" [%x],[%x]", szDestData[*inDestLen - 1],  szDestData[*inDestLen]);
			break;
		}

		if (*inSourecLen > inTotalDataLen)
		{
			inESC_DebugLog(" ParsingData Error SourecLen[%d] TotalLen[%d]", inSourecLen, inTotalDataLen);
			return (VS_ERROR);
		}
	}

	return VS_SUCCESS;
}
        

/*
Function        :inLoadNexEscDataRec
Date&Time       :2015/8/31 
Describe        : 用來讀取ESC檔案
 *	20181227 原本想分開資料，目前使用原本NCCC的方法使用HDT資料		
*/
int inLoadNexEscDataRec(int inESCRec)
{
	unsigned long ulFile_Handle;		/* File Handle */
	unsigned char *uszReadData;	/* record */
	unsigned char *uszTemp;
	char szESCRec[_SIZE_NEX_ESC_REC_ + 1];
	long lnESCLength = 0; 
	long lnReadLength;	
	int i, k, j = 0, inRec = 0;
	int inSearchResult = -1;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	inESC_DebugLog("  inLoadESCRec(%d) ", inESCRec);

	if (inESCRec < 0)
	{
		inESC_DebugLog("  inESCRec < 0:(index = %d) ERROR!!", inESCRec);
		return VS_ERROR;
	}

	if (inFILE_Open(&ulFile_Handle, (unsigned char *)_NEX_ESC_FILE_NAME_) == VS_ERROR)
	{
		inESC_DebugLog("  File Open Error (1)");
		return (VS_ERROR);
	}

	lnESCLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_NEX_ESC_FILE_NAME_);

	if (lnESCLength == VS_ERROR)
	{
		inESC_DebugLog("  FileGetSize Error (1)");
		inFILE_Close(&ulFile_Handle);
		return (VS_ERROR);
	}

	
	uszReadData = malloc(lnESCLength + 1);
	uszTemp = malloc(lnESCLength + 1);

	memset(uszReadData, 0x00, lnESCLength + 1);
	memset(uszTemp, 0x00, lnESCLength + 1);

        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnESCLength;

                for (i = 0;; ++i)
                {
                        /* ?��??�度大於?��???024 */
                        if (lnReadLength >= 1024)
                        {
                                if (inFILE_Read(&ulFile_Handle, &uszTemp[1024*i], 1024) == VS_SUCCESS)
                                {
                                        /* 一次�?1024 */
                                        lnReadLength -= 1024;

                                        /* ?�剩餘長度�?好為1024，�??�好讀�?*/
                                        if (lnReadLength == 0)
                                                break;
                                }
                                /* 讀失�???*/
                                else
                                {
                                        /* Close檔�? */
                                        inFILE_Close(&ulFile_Handle);

                                        /* Free pointer */
                                        free(uszReadData);
                                        free(uszTemp);

                                        return (VS_ERROR);
                                }
                        }
                        /* ?��??�度小於1024 */
                        else if (lnReadLength < 1024)
                        {
                                /* 就只讀?��??�度 */
                                if (inFILE_Read(&ulFile_Handle, &uszTemp[1024*i], lnReadLength) == VS_SUCCESS)
                                {
                                        break;
                                }
                                /* 讀失�???*/
                                else
                                {
                                        /* Close檔�? */
                                        inFILE_Close(&ulFile_Handle);

                                        /* Free pointer */
                                        free(uszReadData);
                                        free(uszTemp);

                                        return (VS_ERROR);
                                }
                        }
                } /* end for loop */
        }
        /* seek不�??��? */
        else
        {
                /* ?��?並�???*/
                inFILE_Close(&ulFile_Handle);
                /* Free pointer */
                free(uszReadData);
                free(uszTemp);

                /* Seek失�?，�?以�??�Error */
                return (VS_ERROR);
        }

        /*
         *?��??�?�要�????record
         *i?�目?��? ESC 讀?��?第幾?��???
         *j?�該record?�長�?
         */
        j = 0;
        for (i = 0; i <= lnESCLength; ++i)      /* "<="?�為了�??��?後�???x00 */
        {
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* ?��?讀??x0D 0x0A，Flag?�為1(表示讀?�record?��?�? */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnESCLength + 1);
                        /* ?�record從temp?��??��?置截?�出來放?�uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        
                        /* ?�為 inESCRec ?�index�??��?，�?�?inESCRec �?1 */
                        if (inRec == (inESCRec + 1))
                        {
                                break;
                        }

                        /* ?��?跳�? 0x0D 0x0A */
                        i = i + 2;
                        /* 每�?完�?筆record，j就歸0 */
                        j = 0;
                }

                j ++;
        }

        /*
         * 如�?沒�? inLoadNexEscDataRec ?�那筆�??��??�整筆�??�都沒�?0x0D 0x0A就�??�Error
         * ?��??��??��??��?並return VS_ERROR
         * 如�?總record?��?小於要�??�Record?�Index
         * ?��?：�??�能?��??�全?�都沒�?0x0D 0x0A
         */
        if (inRec < (inESCRec + 1) || inSearchResult == -1)
        {
                inESC_DebugLog("  No data or Index ERROR");
                
                /* ?��? */
                inFILE_Close(&ulFile_Handle);

                /* Free pointer */
                free(uszReadData);
                free(uszTemp);

                return (VS_ERROR);
        }

        /* uszReadData沒�??��??��??��??��??��??��?並return (VS_ERROR) */
        if (*uszReadData == 0x00)
        {
                inESC_DebugLog("  No specific data.");
                /* ?��? */
                inFILE_Close(&ulFile_Handle);

                /* Free pointer */
                free(uszReadData);
                free(uszTemp);

                return (VS_ERROR);
        }

        /* 結�??��???*/
        memset(&srESCTRec, 0x00, sizeof(srESCTRec));
        /*
         * 以�?pattern?��??�HDT_Rec
         * i?�HDT?�第幾個�???
         * 存入HDT_Rec
         */
        i = 0;


        /* 01_主�?索�? */
        /* ?��???*/
        memset(szESCRec, 0x00, sizeof(szESCRec));
        k = 0;
          
        /* 從Record中�?欄�?資�??��? */
        if(VS_SUCCESS == inParsingData(uszReadData, szESCRec, lnESCLength, &k, &i))
        {
                inESC_DebugLog("  Aft Parsing k[%d] i[%d]",k ,i);
                inESC_DebugLog("  Aft Parsing ESCDATA[%s] ",szESCRec );
                
                memcpy(&srESCTRec.szESCHostIndex[0], &szESCRec[0], k - 1);
        }

        /* 02_主�??�能?�否?��? */
        /* ?��???*/
        memset(szESCRec, 0x00, sizeof(szESCRec));
        k = 0;

        /* 從Record中�?欄�?資�??��? */
       if(VS_SUCCESS == inParsingData(uszReadData, szESCRec, lnESCLength, &k, &i))
        {
                inESC_DebugLog("  Aft Parsing k[%d] i[%d]",k ,i);
                inESC_DebugLog("  Aft Parsing ESCDATA[%s] ",szESCRec );
                
                memcpy(&srESCTRec.szESCULPort[0], &szESCRec[0], k - 1);
        }

        /* 03_主�??�稱 */
	/* ?��???*/
        memset(szESCRec, 0x00, sizeof(szESCRec));
        k = 0;

        /* 從Record中�?欄�?資�??��? */
        if(VS_SUCCESS == inParsingData(uszReadData, szESCRec, lnESCLength, &k, &i))
        {
                inESC_DebugLog("  Aft Parsing k[%d] i[%d]",k ,i);
                inESC_DebugLog("  Aft Parsing ESCDATA[%s] ",szESCRec );
                
                memcpy(&srESCTRec.szESCULIp[0], &szESCRec[0], k - 1);
        }


        /* 04_?��?�?? */
        /* ?��???*/
        memset(szESCRec, 0x00, sizeof(szESCRec));
        k = 0;

        /* 從Record中�?欄�?資�??��? */
        if(VS_SUCCESS == inParsingData(uszReadData, szESCRec, lnESCLength, &k, &i))
        {
                inESC_DebugLog("  Aft Parsing k[%d] i[%d]",k ,i);
                inESC_DebugLog("  Aft Parsing ESCDATA[%s] ",szESCRec );
                
                memcpy(&srESCTRec.szESCIdleTime[0], &szESCRec[0], k - 1);
        }

   
        /* 05_端末機代??*/
        /* ?��???*/
        memset(szESCRec, 0x00, sizeof(szESCRec));
        k = 0;

        /* 從Record中�?欄�?資�??��? */
         if(VS_SUCCESS == inParsingData(uszReadData, szESCRec, lnESCLength, &k, &i))
        {
                inESC_DebugLog("  Aft Parsing k[%d] i[%d]",k ,i);
                inESC_DebugLog("  Aft Parsing ESCDATA[%s] ",szESCRec );
                
                memcpy(&srESCTRec.szESCMode[0], &szESCRec[0], k - 1);
        }

        /* 06_對�??��??�數索�? */
        /* ?��???*/
        memset(szESCRec, 0x00, sizeof(szESCRec));
        k = 0;

        /* 從Record中�?欄�?資�??��? */
        if(VS_SUCCESS == inParsingData(uszReadData, szESCRec, lnESCLength, &k, &i))
        {
                inESC_DebugLog("  Aft Parsing k[%d] i[%d]",k ,i);
                inESC_DebugLog("  Aft Parsing ESCDATA[%s] ",szESCRec );
                
                memcpy(&srESCTRec.szESCUploadMode[0], &szESCRec[0], k - 1);
        }

      
        /* 07_交�??�能?�數 */
        /* ?��???*/
        memset(szESCRec, 0x00, sizeof(szESCRec));
        k = 0;

        /* 從Record中�?欄�?資�??��? */
        if(VS_SUCCESS == inParsingData(uszReadData, szESCRec, lnESCLength, &k, &i))
        {
                inESC_DebugLog("  Aft Parsing k[%d] i[%d]",k ,i);
                inESC_DebugLog("  Aft Parsing ESCDATA[%s] ",szESCRec );
                
                memcpy(&srESCTRec.szESCReceiptUploadUpLimit[0], &szESCRec[0], k - 1);
        }

        /* 08_人工輸入?��??�能 */
        /* ?��???*/
        memset(szESCRec, 0x00, sizeof(szESCRec));
        k = 0;

        /* 從Record中�?欄�?資�??��? */
        if(VS_SUCCESS == inParsingData(uszReadData, szESCRec, lnESCLength, &k, &i))
        {
                inESC_DebugLog("  Aft Parsing k[%d] i[%d]",k ,i);
                inESC_DebugLog("  Aft Parsing ESCDATA[%s] ",szESCRec );
                
                memcpy(&srESCTRec.szESCUploadCnt[0], &szESCRec[0], k - 1);
        }

        /* 09_Call Bank?�能 */
        /* ?��???*/
        memset(szESCRec, 0x00, sizeof(szESCRec));
        k = 0;

        /* 從Record中�?欄�?資�??��? */
        if(VS_SUCCESS == inParsingData(uszReadData, szESCRec, lnESCLength, &k, &i))
        {
                inESC_DebugLog("  Aft Parsing k[%d] i[%d]",k ,i);
                inESC_DebugLog("  Aft Parsing ESCDATA[%s] ",szESCRec );
                
                memcpy(&srESCTRec.szESCUploadFailCnt[0], &szESCRec[0], k - 1);
        }

        /* 10_小費檢�??��?�?*/
        /* ?��???*/
        memset(szESCRec, 0x00, sizeof(szESCRec));
        k = 0;

        /* 從Record中�?欄�?資�??��? */
        if(VS_SUCCESS == inParsingData(uszReadData, szESCRec, lnESCLength, &k, &i))
        {
                inESC_DebugLog("  Aft Parsing k[%d] i[%d]",k ,i);
                inESC_DebugLog("  Aft Parsing ESCDATA[%s] ",szESCRec );
                
                memcpy(&srESCTRec.szESCFailHostIndex[0], &szESCRec[0], k - 1);
        }
        
        
        /* 11_小費檢�??��?�?*/
        /* ?��???*/
        memset(szESCRec, 0x00, sizeof(szESCRec));
        k = 0;

        /* 從Record中�?欄�?資�??��? */
        if(VS_SUCCESS == inParsingData(uszReadData, szESCRec, lnESCLength, &k, &i))
        {
                inESC_DebugLog("  Aft Parsing k[%d] i[%d]",k ,i);
                inESC_DebugLog("  Aft Parsing ESCDATA[%s] ",szESCRec );
                
                memcpy(&srESCTRec.szESCSettleTimeoutCnt[0], &szESCRec[0], k - 1);
        }

        /* 12_小費檢�??��?�?*/
        /* ?��???*/
        memset(szESCRec, 0x00, sizeof(szESCRec));
        k = 0;

        /* 從Record中�?欄�?資�??��? */
        if(VS_SUCCESS == inParsingData(uszReadData, szESCRec, lnESCLength, &k, &i))
        {
                inESC_DebugLog("  Aft Parsing k[%d] i[%d]",k ,i);
                inESC_DebugLog("  Aft Parsing ESCDATA[%s] ",szESCRec );
                
                memcpy(&srESCTRec.szESCDataHead[0], &szESCRec[0], k - 1);
        }
        
        /* release */
        /* ?��? */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadHDTRec() END */
        inESC_DebugLog("  inLoadNexEscDataRec(%d) ", inESCRec);
        inESC_DebugLog("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
        return (VS_SUCCESS);
}


void *pvdGetESCTRec(void)
{
        return &srESCTRec;

}

int inGetESCRecord(char * szTempData, int inMode)
{
	int inResult = -1;
	if(szTempData != NULL)
	{
		switch(inMode)
		{
			case _ESC_HOST_IDEX_:
				if ( strlen(srESCTRec.szESCHostIndex) <= 0 || strlen(srESCTRec.szESCHostIndex) > 2)
				{
					inESC_DebugLog("szEscHostIndex length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCHostIndex[0], strlen(srESCTRec.szESCHostIndex));
					inResult = 0;
				}
				break;
			case _ESC_UL_PORT_:
				if ( strlen(srESCTRec.szESCULPort) <= 0 || strlen(srESCTRec.szESCULPort) > 2)
				{
					inESC_DebugLog("szESCULPort length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCULPort[0], strlen(srESCTRec.szESCULPort));
					inResult = 0;
				}
				break;
			case _ESC_UL_IP_:
				if ( strlen(srESCTRec.szESCULIp) <= 0 || strlen(srESCTRec.szESCULIp) > 2)
				{
					inESC_DebugLog("szESCULIp length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCULIp[0], strlen(srESCTRec.szESCULIp));
					inResult = 0;
				}
				break;
			case _ESC_IDLE_TIME_:
				if ( strlen(srESCTRec.szESCIdleTime) <= 0 || strlen(srESCTRec.szESCIdleTime) > 2)
				{
					inESC_DebugLog("szESCIdleTime length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCIdleTime[0], strlen(srESCTRec.szESCIdleTime));
					inResult = 0;
				}
				break;
			case _ESC_MODE_:
				if ( strlen(srESCTRec.szESCMode) <= 0 || strlen(srESCTRec.szESCMode) > 2)
				{
					inESC_DebugLog("szESCMode length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCMode[0], strlen(srESCTRec.szESCMode));
					inResult = 0;
				}
				break;
			case _ESC_UPLOAD_MODE_:
				if ( strlen(srESCTRec.szESCUploadMode) <= 0 || strlen(srESCTRec.szESCUploadMode) > 2)
				{
					inESC_DebugLog("szESCUploadMode length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCUploadMode[0], strlen(srESCTRec.szESCUploadMode));
					inResult = 0;
				}
				break;
			case _ESC_RECPIT_UPLOAD_LIMIT_:
				if ( strlen(srESCTRec.szESCReceiptUploadUpLimit) <= 0 || strlen(srESCTRec.szESCReceiptUploadUpLimit) > 2)
				{
					inESC_DebugLog("szESCReceiptUploadUpLimit length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCReceiptUploadUpLimit[0], strlen(srESCTRec.szESCReceiptUploadUpLimit));
					inResult = 0;
				}
				break;
			case _ESC_UPLOAD_COUNT_:
				if ( strlen(srESCTRec.szESCUploadCnt) <= 0 || strlen(srESCTRec.szESCUploadCnt) > 2)
				{
					inESC_DebugLog("szESCUploadCnt length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCUploadCnt[0], strlen(srESCTRec.szESCUploadCnt));
					inResult = 0;
				}
				break;
			case _ESC_UPLOAD_FAIL_COUNT_:
				if ( strlen(srESCTRec.szESCUploadFailCnt) <= 0 || strlen(srESCTRec.szESCUploadFailCnt) > 2)
				{
					inESC_DebugLog("szESCUploadFailCnt length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCUploadFailCnt[0], strlen(srESCTRec.szESCUploadFailCnt));
					inResult = 0;
				}
				break;
			case _ESC_FAIL_HOST_INDEX_:
				if ( strlen(srESCTRec.szESCFailHostIndex) <= 0 || strlen(srESCTRec.szESCFailHostIndex) > 2)
				{
					inESC_DebugLog("szESCFailHostIndex length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCFailHostIndex[0], strlen(srESCTRec.szESCFailHostIndex));
					inResult = 0;
				}
				break;
			case _ESC_SETTLE_TIMEOUT_COUNT_:
				if ( strlen(srESCTRec.szESCSettleTimeoutCnt) <= 0 || strlen(srESCTRec.szESCSettleTimeoutCnt) > 2)
				{
					inESC_DebugLog("szESCSettleTimeoutCnt length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCSettleTimeoutCnt[0], strlen(srESCTRec.szESCSettleTimeoutCnt));
					inResult = 0;
				}
				break;
			case _ESC_DATA_HEAD_:
				if ( strlen(srESCTRec.szESCDataHead) <= 0 || strlen(srESCTRec.szESCDataHead) > 2)
				{
					inESC_DebugLog("szESCDataHead length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCDataHead[0], strlen(srESCTRec.szESCDataHead));
					inResult = 0;
				}
				break;
			default:
				break;
		}
		
	}else
	{
		inESC_DebugLog("  Mode[%d] InData == NULL",inMode);
	}
	
	return inResult;
}


int inSetESCRecord(char * szTempData, int inMode)
{
	int inResult = -1;
	if(szTempData != NULL)
	{
		switch(inMode)
		{
			case _ESC_HOST_IDEX_:
				if ( strlen(srESCTRec.szESCHostIndex) <= 0 || strlen(srESCTRec.szESCHostIndex) > 2)
				{
					inESC_DebugLog("szEscHostIndex length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&srESCTRec.szESCHostIndex[0], &szTempData[0], strlen(szTempData));
					inResult = 0;
				}
				break;
			case _ESC_UL_PORT_:
				if ( strlen(srESCTRec.szESCULPort) <= 0 || strlen(srESCTRec.szESCULPort) > 2)
				{
					inESC_DebugLog("szESCULPort length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCULPort[0], strlen(srESCTRec.szESCULPort));
					inResult = 0;
				}
				break;
			case _ESC_UL_IP_:
				if ( strlen(srESCTRec.szESCULIp) <= 0 || strlen(srESCTRec.szESCULIp) > 2)
				{
					inESC_DebugLog("szESCULIp length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCULIp[0], strlen(srESCTRec.szESCULIp));
					inResult = 0;
				}
				break;
			case _ESC_IDLE_TIME_:
				if ( strlen(srESCTRec.szESCIdleTime) <= 0 || strlen(srESCTRec.szESCIdleTime) > 2)
				{
					inESC_DebugLog("szESCIdleTime length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCIdleTime[0], strlen(srESCTRec.szESCIdleTime));
					inResult = 0;
				}
				break;
			case _ESC_MODE_:
				if ( strlen(srESCTRec.szESCMode) <= 0 || strlen(srESCTRec.szESCMode) > 2)
				{
					inESC_DebugLog("szESCMode length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCMode[0], strlen(srESCTRec.szESCMode));
					inResult = 0;
				}
				break;
			case _ESC_UPLOAD_MODE_:
				if ( strlen(srESCTRec.szESCUploadMode) <= 0 || strlen(srESCTRec.szESCUploadMode) > 2)
				{
					inESC_DebugLog("szESCUploadMode length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCUploadMode[0], strlen(srESCTRec.szESCUploadMode));
					inResult = 0;
				}
				break;
			case _ESC_RECPIT_UPLOAD_LIMIT_:
				if ( strlen(srESCTRec.szESCReceiptUploadUpLimit) <= 0 || strlen(srESCTRec.szESCReceiptUploadUpLimit) > 2)
				{
					inESC_DebugLog("szESCReceiptUploadUpLimit length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCReceiptUploadUpLimit[0], strlen(srESCTRec.szESCReceiptUploadUpLimit));
					inResult = 0;
				}
				break;
			case _ESC_UPLOAD_COUNT_:
				if ( strlen(srESCTRec.szESCUploadCnt) <= 0 || strlen(srESCTRec.szESCUploadCnt) > 2)
				{
					inESC_DebugLog("szESCUploadCnt length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCUploadCnt[0], strlen(srESCTRec.szESCUploadCnt));
					inResult = 0;
				}
				break;
			case _ESC_UPLOAD_FAIL_COUNT_:
				if ( strlen(srESCTRec.szESCUploadFailCnt) <= 0 || strlen(srESCTRec.szESCUploadFailCnt) > 2)
				{
					inESC_DebugLog("szESCUploadFailCnt length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCUploadFailCnt[0], strlen(srESCTRec.szESCUploadFailCnt));
					inResult = 0;
				}
				break;
			case _ESC_FAIL_HOST_INDEX_:
				if ( strlen(srESCTRec.szESCFailHostIndex) <= 0 || strlen(srESCTRec.szESCFailHostIndex) > 2)
				{
					inESC_DebugLog("szESCFailHostIndex length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCFailHostIndex[0], strlen(srESCTRec.szESCFailHostIndex));
					inResult = 0;
				}
				break;
			case _ESC_SETTLE_TIMEOUT_COUNT_:
				if ( strlen(srESCTRec.szESCSettleTimeoutCnt) <= 0 || strlen(srESCTRec.szESCSettleTimeoutCnt) > 2)
				{
					inESC_DebugLog("szESCSettleTimeoutCnt length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCSettleTimeoutCnt[0], strlen(srESCTRec.szESCSettleTimeoutCnt));
					inResult = 0;
				}
				break;
			case _ESC_DATA_HEAD_:
				if ( strlen(srESCTRec.szESCDataHead) <= 0 || strlen(srESCTRec.szESCDataHead) > 2)
				{
					inESC_DebugLog("szESCDataHead length = (%d)", (int)strlen(szTempData));
				}else
				{
					memcpy(&szTempData[0], &srESCTRec.szESCDataHead[0], strlen(srESCTRec.szESCDataHead));
					inResult = 0;
				}
				break;
			default:
				break;
		}
		
	}else
	{
		inESC_DebugLog("  Mode[%d] InData == NULL",inMode);
	}
	
	return inResult;
}

/* DEMO 測試功能用  */
int inGetPrintEscCountData(TRANSACTION_OBJECT *pobTran)
{

	char	szKey;

	inESC_DebugLog("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	inDISP_ChineseFont_Color("1 要出紙本 ", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
	inDISP_ChineseFont_Color("2 逾時", _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);
	inDISP_ChineseFont_Color("3 解封包失敗", _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);
	inDISP_ChineseFont_Color("成功按確認", _FONTSIZE_8X22_, _LINE_8_8_, _COLOR_BLACK_, _DISP_LEFT_);
	
	
	while (1)
	{
		szKey = uszKBD_GetKey(_EDC_TIMEOUT_);
		if (szKey == _KEY_1_)
		{
			inESC_DebugLog("  Key_1 ");
			return (VS_ESCAPE);
		}else  if (szKey == _KEY_2_)
		{
			inESC_DebugLog("  Key_2 ");
			return (VS_ERROR);
		}else  if (szKey == _KEY_3_)
		{
			inESC_DebugLog("  Key_3 ");
			return (-4);
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inESC_DebugLog("  Key_Timeout ");
			return (VS_SUCCESS);
		}
		else if (szKey == _KEY_ENTER_)
		{
			inESC_DebugLog("  Key_Enter ");
			return (VS_SUCCESS);
		}

	}
	
	return 0;
}




