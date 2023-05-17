#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "FILE_FUNC/File.h"
#include "FILE_FUNC/FIleLogFunc.h"
#include "Function.h"
#include "EST.h"

static  EST_REC srESTRec;	/* construct EST record */
extern  int     ginDebug;       /* Debug使用 extern */

/*
Function        :inLoadESTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀EST檔案，inESTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadESTRec(int inESTRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆EST檔案 */
        char            szESTRec[_SIZE_EST_REC_ + 1];           /* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnESTLength = 0;                        /* EST總長度 */
        long            lnReadLength;                           /* 記錄每次要從EST.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從EST讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadESTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadESTRec(%d) START!!", inESTRec);
                inDISP_LogPrintf(szErrorMsg);
        }

        /* 判斷傳進來的inESTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inESTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inESTRec < 0:(index = %d) ERROR!!", inESTRec);
                        inDISP_LogPrintf(szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open EST.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_EST_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnESTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_EST_FILE_NAME_);

        if (lnESTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnESTLength + 1);
        uszTemp = malloc(lnESTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnESTLength + 1);
        memset(uszTemp, 0x00, lnESTLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnESTLength;

                for (i = 0;; ++i)
                {
                        /* 剩餘長度大於或等於1024 */
                        if (lnReadLength >= 1024)
                        {
                                if (inFILE_Read(&ulFile_Handle, &uszTemp[1024*i], 1024) == VS_SUCCESS)
                                {
                                        /* 一次讀1024 */
                                        lnReadLength -= 1024;

                                        /* 當剩餘長度剛好為1024，會剛好讀完 */
                                        if (lnReadLength == 0)
                                                break;
                                }
                                /* 讀失敗時 */
                                else
                                {
                                        /* Close檔案 */
                                        inFILE_Close(&ulFile_Handle);

                                        /* Free pointer */
                                        free(uszReadData);
                                        free(uszTemp);

                                        return (VS_ERROR);
                                }
                        }
                        /* 剩餘長度小於1024 */
                        else if (lnReadLength < 1024)
                        {
                                /* 就只讀剩餘長度 */
                                if (inFILE_Read(&ulFile_Handle, &uszTemp[1024*i], lnReadLength) == VS_SUCCESS)
                                {
                                        break;
                                }
                                /* 讀失敗時 */
                                else
                                {
                                        /* Close檔案 */
                                        inFILE_Close(&ulFile_Handle);

                                        /* Free pointer */
                                        free(uszReadData);
                                        free(uszTemp);

                                        return (VS_ERROR);
                                }
                        }
                }/* end for loop */
        }
        /* seek不成功時 */
        else
        {
                /* 關檔並回傳 */
                inFILE_Close(&ulFile_Handle);
                /* Free pointer */
                free(uszReadData);
                free(uszTemp);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /*
         *抓取所需要的那筆record
         *i為目前從EST讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnESTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到EST的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnESTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inEST_Rec的index從0開始，所以inEST_Rec要+1 */
                        if (inRec == (inESTRec + 1))
                        {
                                break;
                        }

                        /* 為了跳過 0x0D 0x0A */
                        i = i + 2;
                        /* 每讀完一筆record，j就歸0 */
                        j = 0;
                }

                j ++;
        }

        /*
         * 如果沒有inESTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inESTRec + 1) || inSearchResult == -1)
        {
                if (ginDebug == VS_TRUE)
                {
                        inDISP_LogPrintf("No data or Index ERROR");
                }

		/* 關檔 */
                inFILE_Close(&ulFile_Handle);

		/* Free pointer */
                free(uszReadData);
                free(uszTemp);

                return (VS_ERROR);
        }

        /* uszReadData沒抓到資料，關檔、釋放記憶體並return (VS_ERROR) */
        if (*uszReadData == 0x00)
        {
                if (ginDebug == VS_TRUE)
                {
                        inDISP_LogPrintf("No specific data.");
                }

                /* 關檔 */
                inFILE_Close(&ulFile_Handle);

                /* Free pointer */
                free(uszReadData);
                free(uszTemp);

                return (VS_ERROR);
        }

        /* 結構初始化 */
        memset(&srESTRec, 0x00, sizeof(srESTRec));
        /*
         * 以下pattern為存入EST_Rec
         * i為EST的第幾個字元
         * 存入EST_Rec
         */
        i = 0;


        /* 01_EMVCAPK索引 */
        /* 初始化 */
        memset(szESTRec, 0x00, sizeof(szESTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szESTRec[k ++] = uszReadData[i ++];
                if (szESTRec[k - 1] == 0x2C	||
		    szESTRec[k - 1] == 0x0D	||
		    szESTRec[k - 1] == 0x0A	||
		    szESTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnESTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("EST unpack ERROR.");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szESTRec[0] != 0x2C	&&
	    szESTRec[0] != 0x0D	&&
	    szESTRec[0] != 0x0A	&&
	    szESTRec[0] != 0x00)
        {
                memcpy(&srESTRec.szEMVCAPKIndex[0], &szESTRec[0], k - 1);
        }

        /* 02_應用程式 ID */
        /* 初始化 */
        memset(szESTRec, 0x00, sizeof(szESTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szESTRec[k ++] = uszReadData[i ++];
                if (szESTRec[k - 1] == 0x2C	||
		    szESTRec[k - 1] == 0x0D	||
		    szESTRec[k - 1] == 0x0A	||
		    szESTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnESTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("EST unpack ERROR");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szESTRec[0] != 0x2C	&&
	    szESTRec[0] != 0x0D	&&
	    szESTRec[0] != 0x0A	&&
	    szESTRec[0] != 0x00)
        {
                memcpy(&srESTRec.szCAPKApplicationId[0], &szESTRec[0], k - 1);
        }

        /* 03_CAPK索引值 */
        /* 初始化 */
        memset(szESTRec, 0x00, sizeof(szESTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szESTRec[k ++] = uszReadData[i ++];
                if (szESTRec[k - 1] == 0x2C	||
		    szESTRec[k - 1] == 0x0D	||
		    szESTRec[k - 1] == 0x0A	||
		    szESTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnESTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("EST unpack ERROR");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szESTRec[0] != 0x2C	&&
	    szESTRec[0] != 0x0D	&&
	    szESTRec[0] != 0x0A	&&
	    szESTRec[0] != 0x00)
        {
                memcpy(&srESTRec.szCAPKIndex[0], &szESTRec[0], k - 1);
        }

        /* 04_CAPK Key值有效期 */
        /* 初始化 */
        memset(szESTRec, 0x00, sizeof(szESTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szESTRec[k ++] = uszReadData[i ++];
                if (szESTRec[k - 1] == 0x2C	||
		    szESTRec[k - 1] == 0x0D	||
		    szESTRec[k - 1] == 0x0A	||
		    szESTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnESTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("EST unpack ERROR");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szESTRec[0] != 0x2C	&&
	    szESTRec[0] != 0x0D	&&
	    szESTRec[0] != 0x0A	&&
	    szESTRec[0] != 0x00)
        {
                memcpy(&srESTRec.szCAPKKeyExpireDate[0], &szESTRec[0], k - 1);
        }

        /* 05_CAPK Key值實際長度 */
        /* 初始化 */
        memset(szESTRec, 0x00, sizeof(szESTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szESTRec[k ++] = uszReadData[i ++];
                if (szESTRec[k - 1] == 0x2C	||
		    szESTRec[k - 1] == 0x0D	||
		    szESTRec[k - 1] == 0x0A	||
		    szESTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnESTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("EST unpack ERROR");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szESTRec[0] != 0x2C	&&
	    szESTRec[0] != 0x0D	&&
	    szESTRec[0] != 0x0A	&&
	    szESTRec[0] != 0x00)
        {
                memcpy(&srESTRec.szCAPKKeyLength[0], &szESTRec[0], k - 1);
        }

        /* 06_CAPK Key值 */
        /* 初始化 */
        memset(szESTRec, 0x00, sizeof(szESTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szESTRec[k ++] = uszReadData[i ++];
                if (szESTRec[k - 1] == 0x2C	||
		    szESTRec[k - 1] == 0x0D	||
		    szESTRec[k - 1] == 0x0A	||
		    szESTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnESTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("EST unpack ERROR");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szESTRec[0] != 0x2C	&&
	    szESTRec[0] != 0x0D	&&
	    szESTRec[0] != 0x0A	&&
	    szESTRec[0] != 0x00)
        {
                memcpy(&srESTRec.szCAPKKeyModulus[0], &szESTRec[0], k - 1);
        }

        /* 07_CAPK Key值 Exponent */
        /* 初始化 */
        memset(szESTRec, 0x00, sizeof(szESTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szESTRec[k ++] = uszReadData[i ++];
                if (szESTRec[k - 1] == 0x2C	||
		    szESTRec[k - 1] == 0x0D	||
		    szESTRec[k - 1] == 0x0A	||
		    szESTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnESTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("EST unpack ERROR");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szESTRec[0] != 0x2C	&&
	    szESTRec[0] != 0x0D	&&
	    szESTRec[0] != 0x0A	&&
	    szESTRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srESTRec.szCAPKExponent[0], &szESTRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadESTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadESTRec(%d) END!!", inESTRec);
                inDISP_LogPrintf(szErrorMsg);
                inDISP_LogPrintf("----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveESTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveESTRec(int inESTRec)
{
 
	unsigned long   uldat_Handle;   			/* FILE Handle */
	int inRecCount = 0, inTempIndex = 0;
	int     	inRetVal;
	int     	i = 0;

	char szTempTitle[] = "inSaveESTRec";
	char szTempFIleName[32], szTempBakFileName[32];
	unsigned char   *uszRead_Total_Buff;
	unsigned char   *uszWriteBuff_Record;
	unsigned long ulFileSize = 0, ulReadSize = 0, ulPackCount = 0,  ulLineLenth = 0;;
	unsigned long ulCurrentLen = 0, ulFileTotalCount = 0;
	unsigned long  uldat_BakHandle;
	int inOnlyOneRecord, inMustCreateFile, inFundRec;
	unsigned short usRetVal;


	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

#ifdef __COUNT_FUNC_TIME__
	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inSaveESTRec INIT" );
#endif
	
	inTempIndex = inESTRec;
	
	
	memset(szTempFIleName, 0x00, sizeof(szTempFIleName));
	strcat( szTempFIleName, _EST_FILE_NAME_);
	
	memset(szTempBakFileName, 0x00, sizeof(szTempBakFileName));
	strcat( szTempBakFileName, _EST_FILE_NAME_BAK_);
	
	inDISP_LogPrintfWithFlag("  %s Index[%d] ", szTempTitle, inTempIndex);
	
      
	/* 開啟原檔案HDPT.dat */
	inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_EST_FILE_NAME_);

	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  %s *Error* [%d] ",szTempTitle, inRetVal);
		inFunc_EDCLock();
		return(VS_ERROR);
	}

	usRetVal = CTOS_FileGetSize((unsigned char *)_EST_FILE_NAME_, &ulFileSize);	
	
	if(usRetVal != d_OK)
	{
		inDISP_DispLogAndWriteFlie("  %s Get FileSize *Error* [%d] Size[%ld]",szTempTitle, usRetVal, ulFileSize);
		inFunc_EDCLock();
		/* Close檔案 */
		inFILE_Close(&uldat_Handle);
		return (VS_ERROR);
	}	

     
	/* 組Write Record封包 */
	/* 給WriteBuff記憶體大小 */
	uszWriteBuff_Record = malloc(_SIZE_EST_REC_ + _SIZE_EST_COMMA_0D0A_);
	memset(uszWriteBuff_Record, 0x00, _SIZE_EST_REC_ + _SIZE_EST_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */


	/* uszRead_Total_Buff儲存EST.dat全部資料 */
	uszRead_Total_Buff = malloc(ulFileSize + 1);
	memset(uszRead_Total_Buff, 0x00, ulFileSize + 1);

	ulPackCount = 0; /* uszWriteBuff_Record的index位置 */
	/* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* EMVCAPKIndex */
	memcpy(&uszWriteBuff_Record[0], &srESTRec.szEMVCAPKIndex[0], strlen(srESTRec.szEMVCAPKIndex));
	ulPackCount += strlen(srESTRec.szEMVCAPKIndex);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* CAPKApplicationId */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srESTRec.szCAPKApplicationId[0], strlen(srESTRec.szCAPKApplicationId));
	ulPackCount += strlen(srESTRec.szCAPKApplicationId);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* CAPKIndex */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srESTRec.szCAPKIndex[0], strlen(srESTRec.szCAPKIndex));
	ulPackCount += strlen(srESTRec.szCAPKIndex);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* CAPKKeyExpireDate */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srESTRec.szCAPKKeyExpireDate[0], strlen(srESTRec.szCAPKKeyExpireDate));
	ulPackCount += strlen(srESTRec.szCAPKKeyExpireDate);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* CAPKKeyLength */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srESTRec.szCAPKKeyLength[0], strlen(srESTRec.szCAPKKeyLength));
	ulPackCount += strlen(srESTRec.szCAPKKeyLength);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* CAPKKeyModulus */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srESTRec.szCAPKKeyModulus[0], strlen(srESTRec.szCAPKKeyModulus));
	ulPackCount += strlen(srESTRec.szCAPKKeyModulus);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* CAPKExponent */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srESTRec.szCAPKExponent[0], strlen(srESTRec.szCAPKExponent));
	ulPackCount += strlen(srESTRec.szCAPKExponent);

	/* 最後的data不用逗號 */
	/* 補上換行符號 */

	/* 0D是移至行首 */
	uszWriteBuff_Record[ulPackCount] = 0x0D;
	ulPackCount++;
	/* 0A是移至下一行 */
	uszWriteBuff_Record[ulPackCount] = 0x0A;
	ulPackCount++;
	
	/*  利用讀取到的檔案長度來讀取整個FILE，但因為 ulReadSize 讀出來的長度會被更改，所以需要二個變數  */
	ulReadSize = ulFileSize;
	
	usRetVal = CTOS_FileRead(uldat_Handle, uszRead_Total_Buff, &ulReadSize);
	
	if(usRetVal != d_OK)
	{
		inDISP_DispLogAndWriteFlie("  %s Read Data *Error* [%d] Line[%d] ", szTempTitle, usRetVal, __LINE__);
		inFunc_EDCLock();
		/* Close檔案 */
		inFILE_Close(&uldat_Handle);

		/* Free pointer */
		free(uszRead_Total_Buff);
		free(uszWriteBuff_Record);
		return (VS_ERROR);
	}
	
	inDISP_LogPrintfWithFlag("  Get ulFileSize[%ld]  Read ulReadSize [%ld] Line[%d] ", ulFileSize, ulReadSize, __LINE__);
 
	/* 計算每行的暫存長度 */
	ulLineLenth = 0;
	/* 實際讀到要寫入的行數長度 */
	ulCurrentLen = 0;
	/* 計算實際讀到的記錄筆數 */
	inRecCount = 0;
	/* 計算實際長度，用來判斷是否一致 */
	ulFileTotalCount = 0;
	
	/* 因為寫入長度會變動,所以要重寫檔案 */
	inMustCreateFile = 0;
	
	/* 用來判斷是否只有一行記錄  */
	inOnlyOneRecord = 0;
	
	inFundRec = VS_ERROR;
	
	/* 計算一行長度為多長  */
	for (i = 0; i < (ulReadSize+1)  ; i++)
	{
		/* 一個換行，record數就+1 */
		/* 第一個判斷 只要是0d 0a 就代表一行 */
		/* 第二個判斷，讀到 0x00 時，前面一個BYTE 如果是0A 就代表是讀過最後一行又讀了一個BYTE */
		if ((uszRead_Total_Buff[i-1] == 0x0D && uszRead_Total_Buff[i] == 0x0A) || 
		     ( uszRead_Total_Buff[i-1] != 0x0A &&uszRead_Total_Buff[i] == 0x00))
		{
			/* 因為不需要算0x00的長度，所以不用加  */
			if(uszRead_Total_Buff[i] != 0x00)
			{
				ulLineLenth++;
				ulFileTotalCount ++;
			}
				
			inDISP_LogPrintfWithFlag("  Find Rec i[%d] [0x%x] ",i, uszRead_Total_Buff[i]);
			inDISP_LogPrintfWithFlag("  Find Rec ulLineLenth[%d] ulFileTotalCount[%d] ",ulLineLenth, ulFileTotalCount);
			
			if( inTempIndex == inRecCount)
			{
				/* 當前行數的位元 */
				ulCurrentLen = ulLineLenth;
				
				/* 如果讀出來的長度與計算一行的長度一致，而且又是第0個記錄，就只要寫入預組好的記錄既可  */
				if(ulFileTotalCount == ulReadSize && inRecCount == 0)
				{
					inDISP_LogPrintfWithFlag("  OnlyOneRecord ");
					inOnlyOneRecord = 1;
				}
				
				if(ulFileTotalCount <= ulReadSize)
				{
					inDISP_LogPrintfWithFlag("  Found Rec ");
					inFundRec = VS_TRUE;
				}
				
				/* 設定檔案定位的地方  */
				ulFileTotalCount -= ulCurrentLen;
			
				/* 因為資料長度可能會變動，如果與原本檔案不同，則以輸入資料為準,需要重寫檔案 */
				if(ulPackCount != ulCurrentLen)
				{
					inMustCreateFile = 1;
				}
				
				inDISP_LogPrintfWithFlag("  Break Rec ");
				break;
			}
			/* 如果不為當前寫入記錄，重算記錄長度，記錄的行數加1*/
			ulLineLenth = 0;
			inRecCount ++;
		}else{
			ulLineLenth++;
			ulFileTotalCount ++;
		}
	}
	
	inDISP_LogPrintfWithFlag("  inTempIndex[%d] inRecCount [%d] ",  inTempIndex, inRecCount);
	
	if(inFundRec != VS_TRUE )
	{
		inDISP_DispLogAndWriteFlie("  %s Serch *Warning*  inTempIndex[%d] ulFileTotalCount [%ld] ulReadSize [%ld] ", szTempTitle, inTempIndex, ulFileTotalCount, ulReadSize);
		inDISP_DispLogAndWriteFlie("  %s Serch *Warning*  ulLineLenth[%d] ulCurrentLen[%d]", szTempTitle, ulLineLenth, ulCurrentLen);
		/* Close檔案 */
		inFILE_Close(&uldat_Handle);

		/* Free pointer */
		free(uszRead_Total_Buff);
		free(uszWriteBuff_Record);
		return (VS_ERROR);
	}
	
	if(ulPackCount != ulCurrentLen){
		inDISP_DispLogAndWriteFlie("  %s Len *Warning* ulFileTotalCount[%d] ulPackCount [%ld] lnLineLenth [%ld] ", szTempTitle, ulFileTotalCount, ulPackCount, ulCurrentLen);
	}
	
	/* 防呆 總record數量小於要存取inHDPTRec Return ERROR */
	if(ulFileTotalCount > ulReadSize)
	{
		inDISP_DispLogAndWriteFlie("  %s Total Len *Warning*  Total Len [%ld] ulReadSize [%ld] ", szTempTitle, ulFileTotalCount, ulReadSize);
	}
	
	/* 因為寫入長度會變動,所以要重寫檔案 */
	if( inMustCreateFile == 1 )
	{
		/* 開啟原檔案HDPT.dat */
		inRetVal = inFILE_Create(&uldat_BakHandle, (unsigned char *)szTempBakFileName);
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  %s Create *Error* [%d] Line[%d] ",szTempTitle, inRetVal, __LINE__);
			inFunc_EDCLock();
			/* Close檔案 */
			inFILE_Close(&uldat_Handle);
				
			/* Free pointer */
			free(uszRead_Total_Buff);
			free(uszWriteBuff_Record);
			return(VS_ERROR);
		}
		
		/* 如果讀出來的長度與計算一行的長度一致，而且又是第0個記錄，就只要寫入預組好的記錄既可  */
		if(inOnlyOneRecord == 1)
		{
			usRetVal = CTOS_FileWrite(uldat_BakHandle, uszWriteBuff_Record, ulPackCount);
			if(usRetVal != d_OK)
			{
				inDISP_DispLogAndWriteFlie("  %s Write Data *Error*usRetVal[%ld] Line[%d]", szTempTitle, usRetVal ,__LINE__ );
				inFunc_EDCLock();
				/* Close檔案 */
				inFILE_Close(&uldat_Handle);
				inFILE_Close(&uldat_BakHandle);

				/* Free pointer */
				free(uszRead_Total_Buff);
				free(uszWriteBuff_Record);
				return (VS_ERROR);
			}
		}else{
			/* 先寫入檔案資料直到算出來的那行資料為止  */
			usRetVal = CTOS_FileWrite(uldat_BakHandle, uszRead_Total_Buff,  ulFileTotalCount);
			if(usRetVal != d_OK)
			{
				inDISP_DispLogAndWriteFlie("  %s Write Data *Error*usRetVal[%ld] Line[%d]", szTempTitle, usRetVal ,__LINE__ );
				inFunc_EDCLock();
				/* Close檔案 */
				inFILE_Close(&uldat_Handle);
				inFILE_Close(&uldat_BakHandle);

				/* Free pointer */
				free(uszRead_Total_Buff);
				free(uszWriteBuff_Record);
				return (VS_ERROR);
			}
			
			/* 寫入要修改的記錄資料  */
			usRetVal = CTOS_FileWrite(uldat_BakHandle, uszWriteBuff_Record, ulPackCount);
			if(usRetVal != d_OK)
			{
				inDISP_DispLogAndWriteFlie("  %s Write Data *Error*usRetVal[%ld] Line[%d]", szTempTitle, usRetVal ,__LINE__ );
				inFunc_EDCLock();
				/* Close檔案 */
				inFILE_Close(&uldat_Handle);
				inFILE_Close(&uldat_BakHandle);
				/* Free pointer */
				free(uszRead_Total_Buff);
				free(uszWriteBuff_Record);
				return (VS_ERROR);
			}
			
			/* 跳過原檔資料至讀取出來的那行資料，再將原後續資料補寫  */
			usRetVal = CTOS_FileWrite(uldat_BakHandle, &uszRead_Total_Buff[(ulFileTotalCount+ ulCurrentLen)],  (ulReadSize -  (ulFileTotalCount+ulCurrentLen)));
			if(usRetVal != d_OK)
			{
				inDISP_DispLogAndWriteFlie("  %s Write Data *Error*usRetVal[%ld] Line[%d]", szTempTitle, usRetVal ,__LINE__ );
				inFunc_EDCLock();
				/* Close檔案 */
				inFILE_Close(&uldat_Handle);
				inFILE_Close(&uldat_BakHandle);
				/* Free pointer */
				free(uszRead_Total_Buff);
				free(uszWriteBuff_Record);
				return (VS_ERROR);
			}
		}
		
		if(VS_SUCCESS != inFILE_Close(&uldat_Handle)){
			inDISP_DispLogAndWriteFlie("  %s File Close *Error*  Line[%d]", szTempTitle, __LINE__ );
			inFunc_EDCLock();
			/* Close檔案 */
			inFILE_Close(&uldat_BakHandle);
			/* Free pointer */
			free(uszRead_Total_Buff);
			free(uszWriteBuff_Record);
			return (VS_ERROR);
		}
		
		if(VS_SUCCESS != inFILE_Close(&uldat_BakHandle)){
			inDISP_DispLogAndWriteFlie("  %s File Close *Error*  Line[%d]", szTempTitle, __LINE__ );
			inFunc_EDCLock();
			/* Free pointer */
			free(uszRead_Total_Buff);
			free(uszWriteBuff_Record);
			return (VS_ERROR);
		}
		
		/* 刪除原CDT.dat */
		if (inFILE_Delete((unsigned char *)szTempFIleName) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  %s File Del *Error* Name[%s] Line[%d]", szTempTitle,szTempFIleName , __LINE__ );
			/* Free pointer */
			free(uszRead_Total_Buff);
			free(uszWriteBuff_Record);
			return (VS_ERROR);
		}

		/* 將CDT.bak改名字為CDT.dat取代原檔案 */
		if (inFILE_Rename((unsigned char *)szTempBakFileName, (unsigned char *)szTempFIleName) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  %s Rename  *Error* Name[%s] Line[%d]", szTempTitle,szTempFIleName , __LINE__ );
			/* Free pointer */
			free(uszRead_Total_Buff);
			free(uszWriteBuff_Record);
			return (VS_ERROR);
		}
		
	}else{
		
		/* 如預先入長度與計算出的行數長度一致，則直接使用定位點寫入既可 */
		
		usRetVal = CTOS_FileSeek(uldat_Handle, ulFileTotalCount, _SEEK_BEGIN_);
	
		if(usRetVal != d_OK)
		{
			inDISP_DispLogAndWriteFlie("  %s File Seek *Error* usRetVal [%ld] SeekAddr [%ld]", szTempTitle, usRetVal,  (ulLineLenth * inTempIndex));
			inFunc_EDCLock();
			/* Close檔案 */
			inFILE_Close(&uldat_Handle);

			/* Free pointer */
			free(uszRead_Total_Buff);
			free(uszWriteBuff_Record);
			return (VS_ERROR);
		}
		
		usRetVal = CTOS_FileWrite(uldat_Handle, uszWriteBuff_Record, ulPackCount);
		if(usRetVal != d_OK)
		{
			inDISP_DispLogAndWriteFlie("  %s Write Data *Error*usRetVal[%ld] Line[%d]", szTempTitle, usRetVal ,__LINE__ );
			inFunc_EDCLock();
			/* Close檔案 */
			inFILE_Close(&uldat_Handle);

			/* Free pointer */
			free(uszRead_Total_Buff);
			free(uszWriteBuff_Record);
			return (VS_ERROR);
		}
	
		/* 關檔 */
		inFILE_Close(&uldat_Handle);
	}
	
	/* Free pointer */
	free(uszWriteBuff_Record);
	free(uszRead_Total_Buff);

#ifdef __COUNT_FUNC_TIME__
	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inSaveESTRec END");
#endif

	inDISP_LogPrintfWithFlag( "  End %s [%d] END!!", szTempTitle, inTempIndex - 1);	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
        return(VS_SUCCESS);
}

/*
set和get等價於相反的操作
各欄位的set和get function
*/

/*
Function        :inGetEMVCAPKIndex
Date&Time       :
Describe        :
*/
int inGetEMVCAPKIndex(char* szEMVCAPKIndex)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szEMVCAPKIndex == NULL || strlen(srESTRec.szEMVCAPKIndex) <= 0 || strlen(srESTRec.szEMVCAPKIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetEMVCAPKIndex() ERROR !!");

			if (szEMVCAPKIndex == NULL)
                        {
                                inDISP_LogPrintf("szEMVCAPKIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEMVCAPKIndex length = (%d)", (int)strlen(srESTRec.szEMVCAPKIndex));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szEMVCAPKIndex[0], &srESTRec.szEMVCAPKIndex[0], strlen(srESTRec.szEMVCAPKIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetEMVCAPKIndex
Date&Time       :
Describe        :
*/
int inSetEMVCAPKIndex(char* szEMVCAPKIndex)
{
        memset(srESTRec.szEMVCAPKIndex, 0x00, sizeof(srESTRec.szEMVCAPKIndex));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szEMVCAPKIndex == NULL || strlen(szEMVCAPKIndex) <= 0 || strlen(szEMVCAPKIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetEMVCAPKIndex() ERROR !!");
                        if (szEMVCAPKIndex == NULL)
                        {
                                inDISP_LogPrintf("szEMVCAPKIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEMVCAPKIndex length = (%d)", (int)strlen(szEMVCAPKIndex));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srESTRec.szEMVCAPKIndex[0], &szEMVCAPKIndex[0], strlen(szEMVCAPKIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetCAPKApplicationId
Date&Time       :
Describe        :
*/
int inGetCAPKApplicationId(char* szCAPKApplicationId)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCAPKApplicationId == NULL || strlen(srESTRec.szCAPKApplicationId) <= 0 || strlen(srESTRec.szCAPKApplicationId) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCAPKApplicationId() ERROR !!");

                        if (szCAPKApplicationId == NULL)
                        {
                                inDISP_LogPrintf("szCAPKApplicationId == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKApplicationId length = (%d)", (int)strlen(srESTRec.szCAPKApplicationId));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCAPKApplicationId[0], &srESTRec.szCAPKApplicationId[0], strlen(srESTRec.szCAPKApplicationId));

        return (VS_SUCCESS);
}

/*
Function        :inSetCAPKApplicationId
Date&Time       :
Describe        :
*/
int inSetCAPKApplicationId(char* szCAPKApplicationId)
{
        memset(srESTRec.szCAPKApplicationId, 0x00, sizeof(srESTRec.szCAPKApplicationId));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCAPKApplicationId == NULL || strlen(szCAPKApplicationId) <= 0 || strlen(szCAPKApplicationId) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCAPKApplicationId() ERROR !!");

                        if (szCAPKApplicationId == NULL)
                        {
                                inDISP_LogPrintf("szCAPKApplicationId == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKApplicationId length = (%d)", (int)strlen(szCAPKApplicationId));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srESTRec.szCAPKApplicationId[0], &szCAPKApplicationId[0], strlen(szCAPKApplicationId));

        return (VS_SUCCESS);
}

/*
Function        :inGetCAPKIndex
Date&Time       :
Describe        :
*/
int inGetCAPKIndex(char* szCAPKIndex)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCAPKIndex == NULL || strlen(srESTRec.szCAPKIndex) <= 0 || strlen(srESTRec.szCAPKIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCAPKIndex() ERROR !!");

                        if (szCAPKIndex == NULL)
                        {
                                inDISP_LogPrintf("szCAPKIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKIndex length = (%d)", (int)strlen(srESTRec.szCAPKIndex));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCAPKIndex[0], &srESTRec.szCAPKIndex[0], strlen(srESTRec.szCAPKIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetCAPKIndex
Date&Time       :
Describe        :
*/
int inSetCAPKIndex(char* szCAPKIndex)
{
        memset(srESTRec.szCAPKIndex, 0x00, sizeof(srESTRec.szCAPKIndex));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCAPKIndex == NULL || strlen(szCAPKIndex) <= 0 || strlen(szCAPKIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCAPKIndex() ERROR !!");

                        if (szCAPKIndex == NULL)
                        {
                                inDISP_LogPrintf("szCAPKIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKIndex length = (%d)", (int)strlen(szCAPKIndex));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srESTRec.szCAPKIndex[0], &szCAPKIndex[0], strlen(szCAPKIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostCAPKKeyExpireDate
Date&Time       :
Describe        :
*/
int inGetHostCAPKKeyExpireDate(char* szCAPKKeyExpireDate)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCAPKKeyExpireDate == NULL || strlen(srESTRec.szCAPKKeyExpireDate) <= 0 || strlen(srESTRec.szCAPKKeyExpireDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetHostCAPKKeyExpireDate() ERROR !!");

                        if (szCAPKKeyExpireDate == NULL)
                        {
                                inDISP_LogPrintf("szCAPKKeyExpireDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKKeyExpireDate length = (%d)", (int)strlen(srESTRec.szCAPKKeyExpireDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCAPKKeyExpireDate[0], &srESTRec.szCAPKKeyExpireDate[0], strlen(srESTRec.szCAPKKeyExpireDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostCAPKKeyExpireDate
Date&Time       :
Describe        :
*/
int inSetHostCAPKKeyExpireDate(char* szCAPKKeyExpireDate)
{
        memset(srESTRec.szCAPKKeyExpireDate, 0x00, sizeof(srESTRec.szCAPKKeyExpireDate));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCAPKKeyExpireDate == NULL || strlen(szCAPKKeyExpireDate) <= 0 || strlen(szCAPKKeyExpireDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetHostCAPKKeyExpireDate() ERROR !!");

                        if (szCAPKKeyExpireDate == NULL)
                        {
                                inDISP_LogPrintf("szCAPKKeyExpireDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKKeyExpireDate length = (%d)", (int)strlen(szCAPKKeyExpireDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srESTRec.szCAPKKeyExpireDate[0], &szCAPKKeyExpireDate[0], strlen(szCAPKKeyExpireDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetCAPKKeyLength
Date&Time       :
Describe        :
*/
int inGetCAPKKeyLength(char* szCAPKKeyLength)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCAPKKeyLength == NULL || strlen(srESTRec.szCAPKKeyLength) <= 0 || strlen(srESTRec.szCAPKKeyLength) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCAPKKeyLength() ERROR !!");

                        if (szCAPKKeyLength == NULL)
                        {
                                inDISP_LogPrintf("szCAPKKeyLength == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKKeyLength length = (%d)", (int)strlen(srESTRec.szCAPKKeyLength));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCAPKKeyLength[0], &srESTRec.szCAPKKeyLength[0], strlen(srESTRec.szCAPKKeyLength));

        return (VS_SUCCESS);
}

/*
Function        :inSetCAPKKeyLength
Date&Time       :
Describe        :
*/
int inSetCAPKKeyLength(char* szCAPKKeyLength)
{
        memset(srESTRec.szCAPKKeyLength, 0x00, sizeof(srESTRec.szCAPKKeyLength));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCAPKKeyLength == NULL || strlen(szCAPKKeyLength) <= 0 || strlen(szCAPKKeyLength) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCAPKKeyLength() ERROR !!");

                        if (szCAPKKeyLength == NULL)
                        {
                                inDISP_LogPrintf("szCAPKKeyLength == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKKeyLength length = (%d)", (int)strlen(szCAPKKeyLength));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srESTRec.szCAPKKeyLength[0], &szCAPKKeyLength[0], strlen(szCAPKKeyLength));

        return (VS_SUCCESS);
}

/*
Function        :inGetCAPKKeyModulus
Date&Time       :
Describe        :
*/
int inGetCAPKKeyModulus(char* szCAPKKeyModulus)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCAPKKeyModulus == NULL || strlen(srESTRec.szCAPKKeyModulus) <= 0 || strlen(srESTRec.szCAPKKeyModulus) > 496)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCAPKKeyModulus() ERROR !!");

                        if (szCAPKKeyModulus == NULL)
                        {
                                inDISP_LogPrintf("szCAPKKeyModulus == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKKeyModulus length = (%d)", (int)strlen(srESTRec.szCAPKKeyModulus));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCAPKKeyModulus[0], &srESTRec.szCAPKKeyModulus[0], strlen(srESTRec.szCAPKKeyModulus));

        return (VS_SUCCESS);
}

/*
Function        :inSetCAPKKeyModulus
Date&Time       :
Describe        :
*/
int inSetCAPKKeyModulus(char* szCAPKKeyModulus)
{
        memset(srESTRec.szCAPKKeyModulus, 0x00, sizeof(srESTRec.szCAPKKeyModulus));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCAPKKeyModulus == NULL || strlen(szCAPKKeyModulus) <= 0 || strlen(szCAPKKeyModulus) > 496)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCAPKKeyModulus() ERROR !!");

                        if (szCAPKKeyModulus == NULL)
                        {
                                inDISP_LogPrintf("szCAPKKeyModulus == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKKeyModulus length = (%d)", (int)strlen(szCAPKKeyModulus));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srESTRec.szCAPKKeyModulus[0], &szCAPKKeyModulus[0], strlen(szCAPKKeyModulus));

        return (VS_SUCCESS);
}

/*
Function        :inGetCAPKExponent
Date&Time       :
Describe        :
*/
int inGetCAPKExponent(char* szCAPKExponent)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCAPKExponent == NULL || strlen(srESTRec.szCAPKExponent) <= 0 || strlen(srESTRec.szCAPKExponent) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCAPKExponent() ERROR !!");

                        if (szCAPKExponent == NULL)
                        {
                                inDISP_LogPrintf("szCAPKExponent == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKExponent length = (%d)", (int)strlen(srESTRec.szCAPKExponent));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCAPKExponent[0], &srESTRec.szCAPKExponent[0], strlen(srESTRec.szCAPKExponent));

        return (VS_SUCCESS);
}

/*
Function        :inSetCAPKExponent
Date&Time       :
Describe        :
*/
int inSetCAPKExponent(char* szCAPKExponent)
{
        memset(srESTRec.szCAPKExponent, 0x00, sizeof(srESTRec.szCAPKExponent));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCAPKExponent == NULL || strlen(szCAPKExponent) <= 0 || strlen(szCAPKExponent) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCAPKExponent() ERROR !!");

                        if (szCAPKExponent == NULL)
                        {
                                inDISP_LogPrintf("szCAPKExponent == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKExponent length = (%d)", (int)strlen(szCAPKExponent));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srESTRec.szCAPKExponent[0], &szCAPKExponent[0], strlen(szCAPKExponent));

        return (VS_SUCCESS);
}
