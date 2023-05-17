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
#include "SCDT.h"

static  SCDT_REC srSCDTRec;	/* construct SCDT record */
extern  int     ginDebug;       /* Debug使用 extern */

/*
Function        :inLoadSCDTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀SCDT檔案，inSCDTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadSCDTRec(int inSCDTRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆SCDT檔案 */
        char            szSCDTRec[_SIZE_SCDT_REC_ + 1];           /* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnSCDTLength = 0;                        /* SCDT總長度 */
        long            lnReadLength;                           /* 記錄每次要從SCDT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從SCDT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */
       
        /* inLoadSCDTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadSCDTRec(%d) START!!", inSCDTRec);
                inDISP_LogPrintf(szErrorMsg);
        }

        /* 判斷傳進來的inSCDTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inSCDTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inSCDTRec < 0:(index = %d) ERROR!!", inSCDTRec);
                        inDISP_LogPrintf(szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open SCDT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_SCDT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnSCDTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_SCDT_FILE_NAME_);
        
        if (lnSCDTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnSCDTLength + 1);
        uszTemp = malloc(lnSCDTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnSCDTLength + 1);
        memset(uszTemp, 0x00, lnSCDTLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnSCDTLength;

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
         *i為目前從SCDT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnSCDTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到SCDT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnSCDTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inSCDT_Rec的index從0開始，所以inSCDT_Rec要+1 */
                        if (inRec == (inSCDTRec + 1))
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
         * 如果沒有inSCDTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inSCDTRec + 1) || inSearchResult == -1)
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
        memset(&srSCDTRec, 0x00, sizeof(srSCDTRec));
        /*
         * 以下pattern為存入SCDT_Rec
         * i為SCDT的第幾個字元
         * 存入SCDT_Rec
         */
        i = 0;


        /* 01_卡別索引 */
        /* 初始化 */
        memset(szSCDTRec, 0x00, sizeof(szSCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSCDTRec[k ++] = uszReadData[i ++];
                if (szSCDTRec[k - 1] == 0x2C	||
		    szSCDTRec[k - 1] == 0x0D	||
		    szSCDTRec[k - 1] == 0x0A	||
		    szSCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("SCDT unpack ERROR.");
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
        if (szSCDTRec[0] != 0x2C	&&
	    szSCDTRec[0] != 0x0D	&&
	    szSCDTRec[0] != 0x0A	&&
	    szSCDTRec[0] != 0x00)
        {
                memcpy(&srSCDTRec.szSCDTCardIndex[0], &szSCDTRec[0], k - 1);
        }

        /* 02_低卡號範圍 */
        /* 初始化 */
        memset(szSCDTRec, 0x00, sizeof(szSCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSCDTRec[k ++] = uszReadData[i ++];
                if (szSCDTRec[k - 1] == 0x2C	||
		    szSCDTRec[k - 1] == 0x0D	||
		    szSCDTRec[k - 1] == 0x0A	||
		    szSCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("SCDT unpack ERROR");
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
        if (szSCDTRec[0] != 0x2C	&&
	    szSCDTRec[0] != 0x0D	&&
	    szSCDTRec[0] != 0x0A	&&
	    szSCDTRec[0] != 0x00)
        {
                memcpy(&srSCDTRec.szSCDTLowBinRange[0], &szSCDTRec[0], k - 1);
        }

        /* 03_高卡號範圍 */
        /* 初始化 */
        memset(szSCDTRec, 0x00, sizeof(szSCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSCDTRec[k ++] = uszReadData[i ++];
                if (szSCDTRec[k - 1] == 0x2C	||
		    szSCDTRec[k - 1] == 0x0D	||
		    szSCDTRec[k - 1] == 0x0A	||
		    szSCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("SCDT unpack ERROR");
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
        if (szSCDTRec[0] != 0x2C	&&
	    szSCDTRec[0] != 0x0D	&&
	    szSCDTRec[0] != 0x0A	&&
	    szSCDTRec[0] != 0x00)
        {
                memcpy(&srSCDTRec.szSCDTHighBinRange[0], &szSCDTRec[0], k - 1);
        }

        /* 04_活動代碼 */
        /* 初始化 */
        memset(szSCDTRec, 0x00, sizeof(szSCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSCDTRec[k ++] = uszReadData[i ++];
                if (szSCDTRec[k - 1] == 0x2C	||
		    szSCDTRec[k - 1] == 0x0D	||
		    szSCDTRec[k - 1] == 0x0A	||
		    szSCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("SCDT unpack ERROR");
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
        if (szSCDTRec[0] != 0x2C	&&
	    szSCDTRec[0] != 0x0D	&&
	    szSCDTRec[0] != 0x0A	&&
	    szSCDTRec[0] != 0x00)
        {
                memcpy(&srSCDTRec.szCampaignNumber[0], &szSCDTRec[0], k - 1);
        }

        /* 05_活動起始日 */
        /* 初始化 */
        memset(szSCDTRec, 0x00, sizeof(szSCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSCDTRec[k ++] = uszReadData[i ++];
                if (szSCDTRec[k - 1] == 0x2C	||
		    szSCDTRec[k - 1] == 0x0D	||
		    szSCDTRec[k - 1] == 0x0A	||
		    szSCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("SCDT unpack ERROR");
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
        if (szSCDTRec[0] != 0x2C	&&
	    szSCDTRec[0] != 0x0D	&&
	    szSCDTRec[0] != 0x0A	&&
	    szSCDTRec[0] != 0x00)
        {
                memcpy(&srSCDTRec.szCampaignStartDate[0], &szSCDTRec[0], k - 1);
        }

        /* 06_活動結束日 */
        /* 初始化 */
        memset(szSCDTRec, 0x00, sizeof(szSCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSCDTRec[k ++] = uszReadData[i ++];
                if (szSCDTRec[k - 1] == 0x2C	||
		    szSCDTRec[k - 1] == 0x0D	||
		    szSCDTRec[k - 1] == 0x0A	||
		    szSCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("SCDT unpack ERROR");
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
        if (szSCDTRec[0] != 0x2C	&&
	    szSCDTRec[0] != 0x0D	&&
	    szSCDTRec[0] != 0x0A	&&
	    szSCDTRec[0] != 0x00)
        {
                memcpy(&srSCDTRec.szCampaignEndDate[0], &szSCDTRec[0], k - 1);
        }

        /* 07_活動限額 */
        /* 初始化 */
        memset(szSCDTRec, 0x00, sizeof(szSCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSCDTRec[k ++] = uszReadData[i ++];
                if (szSCDTRec[k - 1] == 0x2C	||
		    szSCDTRec[k - 1] == 0x0D	||
		    szSCDTRec[k - 1] == 0x0A	||
		    szSCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("SCDT unpack ERROR");
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
        if (szSCDTRec[0] != 0x2C	&&
	    szSCDTRec[0] != 0x0D	&&
	    szSCDTRec[0] != 0x0A	&&
	    szSCDTRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srSCDTRec.szCampaignAmount[0], &szSCDTRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadSCDTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadSCDTRec(%d) END!!", inSCDTRec);
                inDISP_LogPrintf(szErrorMsg);
                inDISP_LogPrintf("----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveSCDTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveSCDTRec(int inSCDTRec)
{
 	unsigned long   uldat_Handle;   			/* FILE Handle */
	int inRecCount = 0, inTempIndex = 0;
	int     	inRetVal;
	int     	i = 0;

	char szTempTitle[] = "inSaveSCDTRec";
	char szTempFIleName[32], szTempBakFileName[32];
	unsigned char   *uszRead_Total_Buff;
	unsigned char   *uszWriteBuff_Record;
	unsigned long ulFileSize = 0, ulReadSize = 0, ulPackCount = 0,  ulLineLenth = 0;
	unsigned long ulCurrentLen = 0, ulFileTotalCount = 0;
	unsigned long  uldat_BakHandle;
	int inOnlyOneRecord, inMustCreateFile, inFundRec;
	unsigned short usRetVal;


	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	#ifdef __COUNT_FUNC_TIME__
	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inSaveSCDTRec INIT" );
	#endif
	
	inTempIndex = inSCDTRec;
	
	memset(szTempFIleName, 0x00, sizeof(szTempFIleName));
	strcat( szTempFIleName, _SCDT_FILE_NAME_);
	
	memset(szTempBakFileName, 0x00, sizeof(szTempBakFileName));
	strcat( szTempBakFileName, _SCDT_FILE_NAME_BAK_);
	
	inDISP_LogPrintfWithFlag("  %s Index[%d] ", szTempTitle, inTempIndex);

      	/* 開啟原檔案HDPT.dat */
	inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_SCDT_FILE_NAME_);

	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  %s *Error* [%d] ",szTempTitle, inRetVal);
		inFunc_EDCLock();
		return(VS_ERROR);
	}

	usRetVal = CTOS_FileGetSize((unsigned char *)_SCDT_FILE_NAME_, &ulFileSize);	
	
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
	uszWriteBuff_Record = malloc(_SIZE_SCDT_REC_ + _SIZE_SCDT_COMMA_0D0A_);
	memset(uszWriteBuff_Record, 0x00, _SIZE_SCDT_REC_ + _SIZE_SCDT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */


	/* uszRead_Total_Buff儲存SCDT.dat全部資料 */
	uszRead_Total_Buff = malloc(ulFileSize + 1);
	memset(uszRead_Total_Buff, 0x00, ulFileSize + 1);

	ulPackCount = 0; /* uszWriteBuff_Record的index位置 */
	/* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* SCDTCardIndex */
	memcpy(&uszWriteBuff_Record[0], &srSCDTRec.szSCDTCardIndex[0], strlen(srSCDTRec.szSCDTCardIndex));
	ulPackCount += strlen(srSCDTRec.szSCDTCardIndex);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* SCDTLowBinRange */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srSCDTRec.szSCDTLowBinRange[0], strlen(srSCDTRec.szSCDTLowBinRange));
	ulPackCount += strlen(srSCDTRec.szSCDTLowBinRange);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* SCDTHighBinRange */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srSCDTRec.szSCDTHighBinRange[0], strlen(srSCDTRec.szSCDTHighBinRange));
	ulPackCount += strlen(srSCDTRec.szSCDTHighBinRange);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* CampaignNumber */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srSCDTRec.szCampaignNumber[0], strlen(srSCDTRec.szCampaignNumber));
	ulPackCount += strlen(srSCDTRec.szCampaignNumber);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* CampaignStartDate */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srSCDTRec.szCampaignStartDate[0], strlen(srSCDTRec.szCampaignStartDate));
	ulPackCount += strlen(srSCDTRec.szCampaignStartDate);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* CampaignEndDate */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srSCDTRec.szCampaignEndDate[0], strlen(srSCDTRec.szCampaignEndDate));
	ulPackCount += strlen(srSCDTRec.szCampaignEndDate);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* CampaignAmount */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srSCDTRec.szCampaignAmount[0], strlen(srSCDTRec.szCampaignAmount));
	ulPackCount += strlen(srSCDTRec.szCampaignAmount);

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
	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inSaveSCDTRec END");
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
Function        :inGetSCDTCardIndex
Date&Time       :
Describe        :
*/
int inGetSCDTCardIndex(char* szSCDTCardIndex)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szSCDTCardIndex == NULL || strlen(srSCDTRec.szSCDTCardIndex) <= 0 || strlen(srSCDTRec.szSCDTCardIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetSCDTCardIndex() ERROR !!");
                        
			if (szSCDTCardIndex == NULL)
                        {
                                inDISP_LogPrintf("szSCDTCardIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSCDTCardIndex length = (%d)", (int)strlen(srSCDTRec.szSCDTCardIndex));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSCDTCardIndex[0], &srSCDTRec.szSCDTCardIndex[0], strlen(srSCDTRec.szSCDTCardIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetSCDTCardIndex
Date&Time       :
Describe        :
*/
int inSetSCDTCardIndex(char* szSCDTCardIndex)
{
        memset(srSCDTRec.szSCDTCardIndex, 0x00, sizeof(srSCDTRec.szSCDTCardIndex));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szSCDTCardIndex == NULL || strlen(szSCDTCardIndex) <= 0 || strlen(szSCDTCardIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetSCDTCardIndex() ERROR !!");
                        if (szSCDTCardIndex == NULL)
                        {
                                inDISP_LogPrintf("szSCDTCardIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSCDTCardIndex length = (%d)", (int)strlen(szSCDTCardIndex));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSCDTRec.szSCDTCardIndex[0], &szSCDTCardIndex[0], strlen(szSCDTCardIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetSCDTLowBinRange
Date&Time       :
Describe        :
*/
int inGetSCDTLowBinRange(char* szSCDTLowBinRange)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szSCDTLowBinRange == NULL || strlen(srSCDTRec.szSCDTLowBinRange) <= 0 || strlen(srSCDTRec.szSCDTLowBinRange) > 11)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetSCDTLowBinRange() ERROR !!");

                        if (szSCDTLowBinRange == NULL)
                        {
                                inDISP_LogPrintf("szSCDTLowBinRange == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSCDTLowBinRange length = (%d)", (int)strlen(srSCDTRec.szSCDTLowBinRange));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSCDTLowBinRange[0], &srSCDTRec.szSCDTLowBinRange[0], strlen(srSCDTRec.szSCDTLowBinRange));

        return (VS_SUCCESS);
}

/*
Function        :inSetSCDTLowBinRange
Date&Time       :
Describe        :
*/
int inSetSCDTLowBinRange(char* szSCDTLowBinRange)
{
        memset(srSCDTRec.szSCDTLowBinRange, 0x00, sizeof(srSCDTRec.szSCDTLowBinRange));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSCDTLowBinRange == NULL || strlen(szSCDTLowBinRange) <= 0 || strlen(szSCDTLowBinRange) > 11)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetSCDTLowBinRange() ERROR !!");

                        if (szSCDTLowBinRange == NULL)
                        {
                                inDISP_LogPrintf("szSCDTLowBinRange == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSCDTLowBinRange length = (%d)", (int)strlen(szSCDTLowBinRange));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSCDTRec.szSCDTLowBinRange[0], &szSCDTLowBinRange[0], strlen(szSCDTLowBinRange));

        return (VS_SUCCESS);
}

/*
Function        :inGetSCDTHighBinRange
Date&Time       :
Describe        :
*/
int inGetSCDTHighBinRange(char* szSCDTHighBinRange)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szSCDTHighBinRange == NULL || strlen(srSCDTRec.szSCDTHighBinRange) <= 0 || strlen(srSCDTRec.szSCDTHighBinRange) > 11)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetSCDTHighBinRange() ERROR !!");

                        if (szSCDTHighBinRange == NULL)
                        {
                                inDISP_LogPrintf("szSCDTHighBinRange == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSCDTHighBinRange length = (%d)", (int)strlen(srSCDTRec.szSCDTHighBinRange));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSCDTHighBinRange[0], &srSCDTRec.szSCDTHighBinRange[0], strlen(srSCDTRec.szSCDTHighBinRange));

        return (VS_SUCCESS);
}

/*
Function        :inSetSCDTHighBinRange
Date&Time       :
Describe        :
*/
int inSetSCDTHighBinRange(char* szSCDTHighBinRange)
{
        memset(srSCDTRec.szSCDTHighBinRange, 0x00, sizeof(srSCDTRec.szSCDTHighBinRange));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSCDTHighBinRange == NULL || strlen(szSCDTHighBinRange) <= 0 || strlen(szSCDTHighBinRange) > 11)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetSCDTHighBinRange() ERROR !!");

                        if (szSCDTHighBinRange == NULL)
                        {
                                inDISP_LogPrintf("szSCDTHighBinRange == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSCDTHighBinRange length = (%d)", (int)strlen(szSCDTHighBinRange));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSCDTRec.szSCDTHighBinRange[0], &szSCDTHighBinRange[0], strlen(szSCDTHighBinRange));

        return (VS_SUCCESS);
}

/*
Function        :inGetCampaignNumber
Date&Time       :
Describe        :
*/
int inGetCampaignNumber(char* szCampaignNumber)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCampaignNumber == NULL || strlen(srSCDTRec.szCampaignNumber) <= 0 || strlen(srSCDTRec.szCampaignNumber) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetHostCampaignNumber() ERROR !!");

                        if (szCampaignNumber == NULL)
                        {
                                inDISP_LogPrintf("szCampaignNumber == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCampaignNumber length = (%d)", (int)strlen(srSCDTRec.szCampaignNumber));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCampaignNumber[0], &srSCDTRec.szCampaignNumber[0], strlen(srSCDTRec.szCampaignNumber));

        return (VS_SUCCESS);
}

/*
Function        :inSetCampaignNumber
Date&Time       :
Describe        :
*/
int inSetCampaignNumber(char* szCampaignNumber)
{
        memset(srSCDTRec.szCampaignNumber, 0x00, sizeof(srSCDTRec.szCampaignNumber));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCampaignNumber == NULL || strlen(szCampaignNumber) <= 0 || strlen(szCampaignNumber) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetHostCampaignNumber() ERROR !!");

                        if (szCampaignNumber == NULL)
                        {
                                inDISP_LogPrintf("szCampaignNumber == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCampaignNumber length = (%d)", (int)strlen(szCampaignNumber));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSCDTRec.szCampaignNumber[0], &szCampaignNumber[0], strlen(szCampaignNumber));

        return (VS_SUCCESS);
}

/*
Function        :inGetCampaignStartDate
Date&Time       :
Describe        :
*/
int inGetCampaignStartDate(char* szCampaignStartDate)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCampaignStartDate == NULL || strlen(srSCDTRec.szCampaignStartDate) <= 0 || strlen(srSCDTRec.szCampaignStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCampaignStartDate() ERROR !!");

                        if (szCampaignStartDate == NULL)
                        {
                                inDISP_LogPrintf("szCampaignStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCampaignStartDate length = (%d)", (int)strlen(srSCDTRec.szCampaignStartDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCampaignStartDate[0], &srSCDTRec.szCampaignStartDate[0], strlen(srSCDTRec.szCampaignStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetCampaignStartDate
Date&Time       :
Describe        :
*/
int inSetCampaignStartDate(char* szCampaignStartDate)
{
        memset(srSCDTRec.szCampaignStartDate, 0x00, sizeof(srSCDTRec.szCampaignStartDate));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCampaignStartDate == NULL || strlen(szCampaignStartDate) <= 0 || strlen(szCampaignStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCampaignStartDate() ERROR !!");

                        if (szCampaignStartDate == NULL)
                        {
                                inDISP_LogPrintf("szCampaignStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCampaignStartDate length = (%d)", (int)strlen(szCampaignStartDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSCDTRec.szCampaignStartDate[0], &szCampaignStartDate[0], strlen(szCampaignStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetCampaignEndDate
Date&Time       :
Describe        :
*/
int inGetCampaignEndDate(char* szCampaignEndDate)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCampaignEndDate == NULL || strlen(srSCDTRec.szCampaignEndDate) <= 0 || strlen(srSCDTRec.szCampaignEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCampaignEndDate() ERROR !!");

                        if (szCampaignEndDate == NULL)
                        {
                                inDISP_LogPrintf("szCampaignEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCampaignEndDate length = (%d)", (int)strlen(srSCDTRec.szCampaignEndDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCampaignEndDate[0], &srSCDTRec.szCampaignEndDate[0], strlen(srSCDTRec.szCampaignEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetCampaignEndDate
Date&Time       :
Describe        :
*/
int inSetCampaignEndDate(char* szCampaignEndDate)
{
        memset(srSCDTRec.szCampaignEndDate, 0x00, sizeof(srSCDTRec.szCampaignEndDate));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCampaignEndDate == NULL || strlen(szCampaignEndDate) <= 0 || strlen(szCampaignEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCampaignEndDate() ERROR !!");

                        if (szCampaignEndDate == NULL)
                        {
                                inDISP_LogPrintf("szCampaignEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCampaignEndDate length = (%d)", (int)strlen(szCampaignEndDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSCDTRec.szCampaignEndDate[0], &szCampaignEndDate[0], strlen(szCampaignEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetCampaignAmount
Date&Time       :
Describe        :
*/
int inGetCampaignAmount(char* szCampaignAmount)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCampaignAmount == NULL || strlen(srSCDTRec.szCampaignAmount) <= 0 || strlen(srSCDTRec.szCampaignAmount) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCampaignAmount() ERROR !!");

                        if (szCampaignAmount == NULL)
                        {
                                inDISP_LogPrintf("szCampaignAmount == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCampaignAmount length = (%d)", (int)strlen(srSCDTRec.szCampaignAmount));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCampaignAmount[0], &srSCDTRec.szCampaignAmount[0], strlen(srSCDTRec.szCampaignAmount));

        return (VS_SUCCESS);
}

/*
Function        :inSetCampaignAmount
Date&Time       :
Describe        :
*/
int inSetCampaignAmount(char* szCampaignAmount)
{
        memset(srSCDTRec.szCampaignAmount, 0x00, sizeof(srSCDTRec.szCampaignAmount));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCampaignAmount == NULL || strlen(szCampaignAmount) <= 0 || strlen(szCampaignAmount) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCampaignAmount() ERROR !!");

                        if (szCampaignAmount == NULL)
                        {
                                inDISP_LogPrintf("szCampaignAmount == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCampaignAmount length = (%d)", (int)strlen(szCampaignAmount));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSCDTRec.szCampaignAmount[0], &szCampaignAmount[0], strlen(szCampaignAmount));

        return (VS_SUCCESS);
}

