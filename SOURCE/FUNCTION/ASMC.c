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
#include "ASMC.h"

static  ASMC_REC srASMCRec;	/* construct ASMC record */
extern  int     ginDebug;  	/* Debug使用 extern */

/*
Function        :inLoadASMCRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀ASMC檔案，inASMC_Rec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadASMCRec(int inASMCRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆ASMC檔案 */
        char            szASMCRec[_SIZE_ASMC_REC_ + 1];         /* 暫存, 放各個欄位檔案 */
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnASMCLength = 0;                       /* ASMC總長度 */
        long            lnReadLength;                           /* 記錄每次要從ASMC.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;                 /* inRec記錄讀到第幾筆, i為目前從ASMC讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadASMCRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadASMCRec(%d) START!!", inASMCRec);
                inDISP_LogPrintf(szErrorMsg);
        }

        /* 判斷傳進來的inASMCRec是否小於零 大於等於零才是正確值(防呆) */
        if (inASMCRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inASMCRec < 0:(index = %d) ERROR!!", inASMCRec);
                        inDISP_LogPrintf(szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open ASMC.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_ASMC_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnASMCLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_ASMC_FILE_NAME_);
        
	if (lnASMCLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnASMCLength + 1);
        uszTemp = malloc(lnASMCLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnASMCLength + 1);
        memset(uszTemp, 0x00, lnASMCLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnASMCLength;

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
                } /* end for loop */
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
         *i為目前從ASMC讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnASMCLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到ASMC的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

			/* 清空uszReadData */
                        memset(uszReadData, 0x00, lnASMCLength + 1);
			/* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
			/* 因為inASMC_Rec的index從0開始，所以inASMC_Rec要+1 */
                        if (inRec == (inASMCRec + 1))
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
         * 如果沒有inASMCRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inASMCRec + 1) || inSearchResult == -1)
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
        memset(&srASMCRec, 0x00, sizeof(srASMCRec));
        /*
         * 以下pattern為存入ASMC_Rec
         * i為ASMC的第幾個字元
         * 存入ASMC_Rec
         */
        i = 0;


        /* 01_支援刷卡兌換 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)
        {
                memcpy(&srASMCRec.szCreditCardFlag[0], &szASMCRec[0], k - 1);
        }

        /* 02_刷卡兌換起日參數 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)
        {
                memcpy(&srASMCRec.szCreditCardStartDate[0], &szASMCRec[0], k - 1);
        }

        /* 03_刷卡兌換迄日參數 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)
        {
                memcpy(&srASMCRec.szCreditCardEndDate[0], &szASMCRec[0], k - 1);
        }

        /* 04_支援條碼兑換 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)
        {
                memcpy(&srASMCRec.szBarCodeFlag[0], &szASMCRec[0], k - 1);
        }

        /* 05_條碼兑換起日參數 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)
        {
                memcpy(&srASMCRec.szBarCodeStartDate[0], &szASMCRec[0], k - 1);
        }

        /* 06_條碼兑換迄日參數 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)
        {
                memcpy(&srASMCRec.szBarCodeEndDate[0], &szASMCRec[0], k - 1);
        }

        /* 07_支援條碼兌換取消 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)
        {
                memcpy(&srASMCRec.szVoidRedeemFlag[0], &szASMCRec[0], k - 1);
        }

        /* 08_條碼兌換取消起日參數 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)
        {
                memcpy(&srASMCRec.szVoidRedeemStartDate[0], &szASMCRec[0], k - 1);
        }

        /* 09_條碼兌換取消迄日參數 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)
        {
                memcpy(&srASMCRec.szVoidRedeemEndDate[0], &szASMCRec[0], k - 1);
        }

        /* 10_支援優惠(含請求電文)功能 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)
        {
                memcpy(&srASMCRec.szASMFlag[0], &szASMCRec[0], k - 1);
        }

        /* 11_優惠起日參數 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)
        {
                memcpy(&srASMCRec.szASMStartDate[0], &szASMCRec[0], k - 1);
        }

        /* 12_優惠迄日參數 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srASMCRec.szASMEndDate[0], &szASMCRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

	/* inLoadASMCRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadASMCRec(%d) END!!", inASMCRec);
                inDISP_LogPrintf(szErrorMsg);
		inDISP_LogPrintf("----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveASMCRec
Date&Time       :
Describe        :
*/
int inSaveASMCRec(int inASMCRec)
{
	unsigned long   uldat_Handle; /* FILE Handle */
	int inRecCount = 0, inTempIndex = 0;
	int     	inRetVal;
	int     	i = 0;

	char szTempTitle[] = "inSaveASMCRec";
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
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inSaveASMCRec INIT" );
#endif
	
	inTempIndex = inASMCRec;
	
	memset(szTempFIleName, 0x00, sizeof(szTempFIleName));
	strcat( szTempFIleName, _ASMC_FILE_NAME_);
	
	memset(szTempBakFileName, 0x00, sizeof(szTempBakFileName));
	strcat( szTempBakFileName, _ASMC_FILE_NAME_BAK_);
	
	inDISP_LogPrintfWithFlag("  %s Index[%d] ", szTempTitle, inTempIndex);
	
	/* 開啟原檔案HDPT.dat */
	inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_ASMC_FILE_NAME_);

	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" %s *Error* [%d]  ",szTempTitle, inRetVal);
		inFunc_EDCLock();
		return(VS_ERROR);
	}

	usRetVal = CTOS_FileGetSize((unsigned char *)_ASMC_FILE_NAME_, &ulFileSize);	
	
	if(usRetVal != d_OK)
	{
		inDISP_DispLogAndWriteFlie(" %s Get FileSize *Error* [%d] Size[%ld]",szTempTitle, usRetVal, ulFileSize);
		inFunc_EDCLock();
		/* Close檔案 */
		inFILE_Close(&uldat_Handle);
		return (VS_ERROR);
	}

	/* 組Write Record封包 */
	/* 給WriteBuff記憶體大小 */
	uszWriteBuff_Record = malloc(_SIZE_ASMC_REC_ + _SIZE_ASMC_COMMA_0D0A_);
	memset(uszWriteBuff_Record, 0x00, _SIZE_ASMC_REC_ + _SIZE_ASMC_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

	/* uszRead_Total_Buff儲存ASMC.dat全部資料 */
	uszRead_Total_Buff = malloc(ulFileSize + 1);
	memset(uszRead_Total_Buff, 0x00, ulFileSize + 1);

	ulPackCount = 0; /* uszWriteBuff_Record的index位置 */
	/* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* CreditCardFlag */
	memcpy(&uszWriteBuff_Record[0], &srASMCRec.szCreditCardFlag[0], strlen(srASMCRec.szCreditCardFlag));
	ulPackCount += strlen(srASMCRec.szCreditCardFlag);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* CreditCardStartDate */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srASMCRec.szCreditCardStartDate[0], strlen(srASMCRec.szCreditCardStartDate));
	ulPackCount += strlen(srASMCRec.szCreditCardStartDate);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* CreditCardEndDate */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srASMCRec.szCreditCardEndDate[0], strlen(srASMCRec.szCreditCardEndDate));
	ulPackCount += strlen(srASMCRec.szCreditCardEndDate);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* BarCodeFlag */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srASMCRec.szBarCodeFlag[0], strlen(srASMCRec.szBarCodeFlag));
	ulPackCount += strlen(srASMCRec.szBarCodeFlag);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* BarCodeStartDate */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srASMCRec.szBarCodeStartDate[0], strlen(srASMCRec.szBarCodeStartDate));
	ulPackCount += strlen(srASMCRec.szBarCodeStartDate);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* BarCodeEndDate */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srASMCRec.szBarCodeEndDate[0], strlen(srASMCRec.szBarCodeEndDate));
	ulPackCount += strlen(srASMCRec.szBarCodeEndDate);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* VoidRedeemFlag */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srASMCRec.szVoidRedeemFlag[0], strlen(srASMCRec.szVoidRedeemFlag));
	ulPackCount += strlen(srASMCRec.szVoidRedeemFlag);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* VoidRedeemStartDate */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srASMCRec.szVoidRedeemStartDate[0], strlen(srASMCRec.szVoidRedeemStartDate));
	ulPackCount += strlen(srASMCRec.szVoidRedeemStartDate);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* VoidRedeemEndDate */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srASMCRec.szVoidRedeemEndDate[0], strlen(srASMCRec.szVoidRedeemEndDate));
	ulPackCount += strlen(srASMCRec.szVoidRedeemEndDate);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* ASMFlag */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srASMCRec.szASMFlag[0], strlen(srASMCRec.szASMFlag));
	ulPackCount += strlen(srASMCRec.szASMFlag);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* ASMStartDate */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srASMCRec.szASMStartDate[0], strlen(srASMCRec.szASMStartDate));
	ulPackCount += strlen(srASMCRec.szASMStartDate);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* ASMEndDate */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srASMCRec.szASMEndDate[0], strlen(srASMCRec.szASMEndDate));
	ulPackCount += strlen(srASMCRec.szASMEndDate);

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
		inDISP_DispLogAndWriteFlie(" %s Read Data *Error* [%d] Line[%d] ", szTempTitle, usRetVal, __LINE__);
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
		inDISP_DispLogAndWriteFlie(" %s Serch *Warning*  inTempIndex[%d] ulFileTotalCount [%ld] ulReadSize [%ld] ", szTempTitle, inTempIndex, ulFileTotalCount, ulReadSize);
		inDISP_DispLogAndWriteFlie(" %s Serch *Warning*  ulLineLenth[%d] ulCurrentLen[%d]", szTempTitle, ulLineLenth, ulCurrentLen);
		/* Close檔案 */
		inFILE_Close(&uldat_Handle);

		/* Free pointer */
		free(uszRead_Total_Buff);
		free(uszWriteBuff_Record);
		return (VS_ERROR);
	}
	
	if(ulPackCount != ulCurrentLen){
		inDISP_DispLogAndWriteFlie(" %s Len *Warning* ulFileTotalCount[%d] ulPackCount [%ld] lnLineLenth [%ld] ", szTempTitle, ulFileTotalCount, ulPackCount, ulCurrentLen);
	}
	
	/* 防呆 總record數量小於要存取inHDPTRec Return ERROR */
	if(ulFileTotalCount > ulReadSize)
	{
		inDISP_DispLogAndWriteFlie(" %s Total Len *Warning*  Total Len [%ld] ulReadSize [%ld] ", szTempTitle, ulFileTotalCount, ulReadSize);
	}
	
	/* 因為寫入長度會變動,所以要重寫檔案 */
	if( inMustCreateFile == 1 )
	{
		/* 開啟原檔案HDPT.dat */
		inRetVal = inFILE_Create(&uldat_BakHandle, (unsigned char *)szTempBakFileName);
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" %s Create *Error* [%d] Line[%d] ",szTempTitle, inRetVal, __LINE__);
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
				inDISP_DispLogAndWriteFlie(" %s Write Data *Error*usRetVal[%ld] Line[%d]", szTempTitle, usRetVal ,__LINE__ );
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
				inDISP_DispLogAndWriteFlie(" %s Write Data *Error*usRetVal[%ld] Line[%d]", szTempTitle, usRetVal ,__LINE__ );
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
				inDISP_DispLogAndWriteFlie(" %s Write Data *Error*usRetVal[%ld] Line[%d]", szTempTitle, usRetVal ,__LINE__ );
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
				inDISP_DispLogAndWriteFlie(" %s Write Data *Error*usRetVal[%ld] Line[%d]", szTempTitle, usRetVal ,__LINE__ );
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
			inDISP_DispLogAndWriteFlie(" %s File Close *Error*  Line[%d]", szTempTitle, __LINE__ );
			inFunc_EDCLock();
			/* Close檔案 */
			inFILE_Close(&uldat_BakHandle);
			/* Free pointer */
			free(uszRead_Total_Buff);
			free(uszWriteBuff_Record);
			return (VS_ERROR);
		}
		
		if(VS_SUCCESS != inFILE_Close(&uldat_BakHandle)){
			inDISP_DispLogAndWriteFlie(" %s File Close *Error*  Line[%d]", szTempTitle, __LINE__ );
			inFunc_EDCLock();
			/* Free pointer */
			free(uszRead_Total_Buff);
			free(uszWriteBuff_Record);
			return (VS_ERROR);
		}
		
		/* 刪除原CDT.dat */
		if (inFILE_Delete((unsigned char *)szTempFIleName) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" %s File Del *Error* Name[%s] Line[%d]", szTempTitle,szTempFIleName , __LINE__ );
			/* Free pointer */
			free(uszRead_Total_Buff);
			free(uszWriteBuff_Record);
			return (VS_ERROR);
		}

		/* 將CDT.bak改名字為CDT.dat取代原檔案 */
		if (inFILE_Rename((unsigned char *)szTempBakFileName, (unsigned char *)szTempFIleName) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" %s Rename  *Error* Name[%s] Line[%d]", szTempTitle,szTempFIleName , __LINE__ );
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
			inDISP_DispLogAndWriteFlie(" %s File Seek *Error* usRetVal [%ld] SeekAddr [%ld]", szTempTitle, usRetVal,  (ulLineLenth * inTempIndex));
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
			inDISP_DispLogAndWriteFlie(" %s Write Data *Error*usRetVal[%ld] Line[%d]", szTempTitle, usRetVal ,__LINE__ );
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
	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inSaveASMCRec END");
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
Function        :inGetCreditCardFlag
Date&Time       :
Describe        :
*/
int inGetCreditCardFlag(char* szCreditCardFlag)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCreditCardFlag == NULL || strlen(srASMCRec.szCreditCardFlag) <= 0 || strlen(srASMCRec.szCreditCardFlag) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCreditCardFlag() ERROR !!");

                        if (szCreditCardFlag == NULL)
                        {
                                inDISP_LogPrintf("szCreditCardFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCreditCardFlag length = (%d)", (int)strlen(srASMCRec.szCreditCardFlag));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCreditCardFlag[0], &srASMCRec.szCreditCardFlag[0], strlen(srASMCRec.szCreditCardFlag));

        return (VS_SUCCESS);
}

/*
Function        :inSetCreditCardFlag
Date&Time       :
Describe        :
*/
int inSetCreditCardFlag(char* szCreditCardFlag)
{
        memset(srASMCRec.szCreditCardFlag, 0x00, sizeof(srASMCRec.szCreditCardFlag));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCreditCardFlag == NULL || strlen(szCreditCardFlag) <= 0 || strlen(szCreditCardFlag) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCreditCardFlag() ERROR !!");

                        if (szCreditCardFlag == NULL)
                        {
                                inDISP_LogPrintf("szCreditCardFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCreditCardFlag length = (%d)", (int)strlen(szCreditCardFlag));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szCreditCardFlag[0], &szCreditCardFlag[0], strlen(szCreditCardFlag));

        return (VS_SUCCESS);
}

/*
Function        :inGetCreditCardStartDate
Date&Time       :
Describe        :
*/
int inGetCreditCardStartDate(char* szCreditCardStartDate)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCreditCardStartDate == NULL || strlen(srASMCRec.szCreditCardStartDate) <= 0 || strlen(srASMCRec.szCreditCardStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCreditCardStartDate() ERROR !!");

                        if (szCreditCardStartDate == NULL)
                        {
                                inDISP_LogPrintf("szCreditCardStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCreditCardStartDate length = (%d)", (int)strlen(srASMCRec.szCreditCardStartDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCreditCardStartDate[0], &srASMCRec.szCreditCardStartDate[0], strlen(srASMCRec.szCreditCardStartDate));

        return (VS_SUCCESS);

}

/*
Function        :inSetCreditCardStartDate
Date&Time       :
Describe        :
*/
int inSetCreditCardStartDate(char* szCreditCardStartDate)
{
        memset(srASMCRec.szCreditCardStartDate, 0x00, sizeof(srASMCRec.szCreditCardStartDate));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCreditCardStartDate == NULL || strlen(szCreditCardStartDate) <= 0 || strlen(szCreditCardStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCreditCardStartDate() ERROR !!");

                        if (szCreditCardStartDate == NULL)
                        {
                                inDISP_LogPrintf("szCreditCardStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCreditCardStartDate length = (%d)", (int)strlen(szCreditCardStartDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szCreditCardStartDate[0], &szCreditCardStartDate[0], strlen(szCreditCardStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetCreditCardEndDate
Date&Time       :
Describe        :
*/
int inGetCreditCardEndDate(char* szCreditCardEndDate)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCreditCardEndDate == NULL || strlen(srASMCRec.szCreditCardEndDate) <= 0 || strlen(srASMCRec.szCreditCardEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCreditCardEndDate() ERROR !!");

                        if (szCreditCardEndDate == NULL)
                        {
                                inDISP_LogPrintf("szCreditCardEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCreditCardEndDate length = (%d)", (int)strlen(szCreditCardEndDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCreditCardEndDate[0], &srASMCRec.szCreditCardEndDate[0], strlen(srASMCRec.szCreditCardEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetCreditCardEndDate
Date&Time       :
Describe        :
*/
int inSetCreditCardEndDate(char* szCreditCardEndDate)
{
        memset(srASMCRec.szCreditCardEndDate, 0x00, sizeof(srASMCRec.szCreditCardEndDate));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCreditCardEndDate == NULL || strlen(szCreditCardEndDate) <= 0 || strlen(szCreditCardEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCreditCardEndDate() ERROR !!");

                        if (szCreditCardEndDate == NULL)
                        {
                                inDISP_LogPrintf("szCreditCardEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCreditCardEndDate length = (%d)", (int)strlen(szCreditCardEndDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szCreditCardEndDate[0], &szCreditCardEndDate[0], strlen(szCreditCardEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetBarCodeFlag
Date&Time       :
Describe        :
*/
int inGetBarCodeFlag(char* szBarCodeFlag)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szBarCodeFlag == NULL || strlen(srASMCRec.szBarCodeFlag) <= 0 || strlen(srASMCRec.szBarCodeFlag) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetBarCodeFlag() ERROR !!");

                        if (szBarCodeFlag == NULL)
                        {
                                inDISP_LogPrintf("szBarCodeFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBarCodeFlag length = (%d)", (int)strlen(szBarCodeFlag));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szBarCodeFlag[0], &srASMCRec.szBarCodeFlag[0], strlen(srASMCRec.szBarCodeFlag));

        return (VS_SUCCESS);
}

/*
Function        :inSetBarCodeFlag
Date&Time       :
Describe        :
*/
int inSetBarCodeFlag(char* szBarCodeFlag)
{
        memset(srASMCRec.szBarCodeFlag, 0x00, sizeof(srASMCRec.szBarCodeFlag));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szBarCodeFlag == NULL || strlen(szBarCodeFlag) <= 0 || strlen(szBarCodeFlag) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetBarCodeFlag() ERROR !!");

                        if (szBarCodeFlag == NULL)
                        {
                                inDISP_LogPrintf("szBarCodeFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBarCodeFlag length = (%d)", (int)strlen(szBarCodeFlag));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szBarCodeFlag[0], &szBarCodeFlag[0], strlen(szBarCodeFlag));

        return (VS_SUCCESS);
}

/*
Function        :inGetBarCodeStartDate
Date&Time       :
Describe        :
*/
int inGetBarCodeStartDate(char* szBarCodeStartDate)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szBarCodeStartDate == NULL || strlen(srASMCRec.szBarCodeStartDate) <= 0 || strlen(srASMCRec.szBarCodeStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetBarCodeStartDate() ERROR !!");

                        if (szBarCodeStartDate == NULL)
                        {
                                inDISP_LogPrintf("szBarCodeStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBarCodeStartDate length = (%d)", (int)strlen(szBarCodeStartDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szBarCodeStartDate[0], &srASMCRec.szBarCodeStartDate[0], strlen(srASMCRec.szBarCodeStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetBarCodeStartDate
Date&Time       :
Describe        :
*/
int inSetBarCodeStartDate(char* szBarCodeStartDate)
{
        memset(srASMCRec.szBarCodeStartDate, 0x00, sizeof(srASMCRec.szBarCodeStartDate));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szBarCodeStartDate == NULL || strlen(szBarCodeStartDate) <= 0 || strlen(szBarCodeStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetBarCodeStartDate() ERROR !!");

                        if (szBarCodeStartDate == NULL)
                        {
                                inDISP_LogPrintf("szBarCodeStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBarCodeStartDate length = (%d)", (int)strlen(szBarCodeStartDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szBarCodeStartDate[0], &szBarCodeStartDate[0], strlen(szBarCodeStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetBarCodeEndDate
Date&Time       :
Describe        :
*/
int inGetBarCodeEndDate(char* szBarCodeEndDate)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szBarCodeEndDate == NULL || strlen(srASMCRec.szBarCodeEndDate) <= 0 || strlen(srASMCRec.szBarCodeEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetBarCodeEndDate() ERROR !!");

                        if (szBarCodeEndDate == NULL)
                        {
                                inDISP_LogPrintf("szBarCodeEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBarCodeEndDate length = (%d)", (int)strlen(szBarCodeEndDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szBarCodeEndDate[0], &srASMCRec.szBarCodeEndDate[0], strlen(srASMCRec.szBarCodeEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetBarCodeEndDate
Date&Time       :
Describe        :
*/
int inSetBarCodeEndDate(char* szBarCodeEndDate)
{
        memset(srASMCRec.szBarCodeEndDate, 0x00, sizeof(srASMCRec.szBarCodeEndDate));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szBarCodeEndDate == NULL || strlen(szBarCodeEndDate) <= 0 || strlen(szBarCodeEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetBarCodeEndDate() ERROR !!");

                        if (szBarCodeEndDate == NULL)
                        {
                                inDISP_LogPrintf("szBarCodeEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBarCodeEndDate length = (%d)", (int)strlen(szBarCodeEndDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szBarCodeEndDate[0], &szBarCodeEndDate[0], strlen(szBarCodeEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetVoidRedeemFlag
Date&Time       :
Describe        :
*/
int inGetVoidRedeemFlag(char* szVoidRedeemFlag)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szVoidRedeemFlag == NULL || strlen(srASMCRec.szVoidRedeemFlag) <= 0 || strlen(srASMCRec.szVoidRedeemFlag) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetVoidRedeemFlag() ERROR !!");

                        if (szVoidRedeemFlag == NULL)
                        {
                                inDISP_LogPrintf("szVoidRedeemFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szVoidRedeemFlag length = (%d)", (int)strlen(szVoidRedeemFlag));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szVoidRedeemFlag[0], &srASMCRec.szVoidRedeemFlag[0], strlen(srASMCRec.szVoidRedeemFlag));

        return (VS_SUCCESS);
}

/*
Function        :inSetVoidRedeemFlag
Date&Time       :
Describe        :
*/
int inSetVoidRedeemFlag(char* szVoidRedeemFlag)
{
        memset(srASMCRec.szVoidRedeemFlag, 0x00, sizeof(srASMCRec.szVoidRedeemFlag));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szVoidRedeemFlag == NULL || strlen(szVoidRedeemFlag) <= 0 || strlen(szVoidRedeemFlag) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetVoidRedeemFlag() ERROR !!");

                        if (szVoidRedeemFlag == NULL)
                        {
                                inDISP_LogPrintf("szVoidRedeemFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szVoidRedeemFlag length = (%d)", (int)strlen(szVoidRedeemFlag));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szVoidRedeemFlag[0], &szVoidRedeemFlag[0], strlen(szVoidRedeemFlag));

        return (VS_SUCCESS);
}

/*
Function        :inGetVoidRedeemStartDate
Date&Time       :
Describe        :
*/
int inGetVoidRedeemStartDate(char* szVoidRedeemStartDate)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szVoidRedeemStartDate == NULL || strlen(srASMCRec.szVoidRedeemStartDate) <= 0 || strlen(srASMCRec.szVoidRedeemStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetVoidRedeemStartDate() ERROR !!");

                        if (szVoidRedeemStartDate == NULL)
                        {
                                inDISP_LogPrintf("szVoidRedeemStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szVoidRedeemStartDate length = (%d)", (int)strlen(szVoidRedeemStartDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szVoidRedeemStartDate[0], &srASMCRec.szVoidRedeemStartDate[0], strlen(srASMCRec.szVoidRedeemStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetVoidRedeemStartDate
Date&Time       :
Describe        :
*/
int inSetVoidRedeemStartDate(char* szVoidRedeemStartDate)
{
        memset(srASMCRec.szVoidRedeemStartDate, 0x00, sizeof(srASMCRec.szVoidRedeemStartDate));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szVoidRedeemStartDate == NULL || strlen(szVoidRedeemStartDate) <= 0 || strlen(szVoidRedeemStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetVoidRedeemStartDate() ERROR !!");

                        if (szVoidRedeemStartDate == NULL)
                        {
                                inDISP_LogPrintf("szVoidRedeemStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szVoidRedeemStartDate length = (%d)", (int)strlen(szVoidRedeemStartDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szVoidRedeemStartDate[0], &szVoidRedeemStartDate[0], strlen(szVoidRedeemStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetVoidRedeemEndDate
Date&Time       :
Describe        :
*/
int inGetVoidRedeemEndDate(char* szVoidRedeemEndDate)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szVoidRedeemEndDate == NULL || strlen(srASMCRec.szVoidRedeemEndDate) <= 0 || strlen(srASMCRec.szVoidRedeemEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetVoidRedeemEndDate() ERROR !!");

                        if (szVoidRedeemEndDate == NULL)
                        {
                                inDISP_LogPrintf("szVoidRedeemEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szVoidRedeemEndDate length = (%d)", (int)strlen(szVoidRedeemEndDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szVoidRedeemEndDate[0], &srASMCRec.szVoidRedeemEndDate[0], strlen(srASMCRec.szVoidRedeemEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetVoidRedeemEndDate
Date&Time       :
Describe        :
*/
int inSetVoidRedeemEndDate(char* szVoidRedeemEndDate)
{
        memset(srASMCRec.szVoidRedeemEndDate, 0x00, sizeof(srASMCRec.szVoidRedeemEndDate));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szVoidRedeemEndDate == NULL || strlen(szVoidRedeemEndDate) <= 0 || strlen(szVoidRedeemEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetVoidRedeemEndDate() ERROR !!");

                        if (szVoidRedeemEndDate == NULL)
                        {
                                inDISP_LogPrintf("szVoidRedeemEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szVoidRedeemEndDate length = (%d)", (int)strlen(szVoidRedeemEndDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szVoidRedeemEndDate[0], &szVoidRedeemEndDate[0], strlen(szVoidRedeemEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetASMFlag
Date&Time       :
Describe        :
*/
int inGetASMFlag(char* szASMFlag)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szASMFlag == NULL || strlen(srASMCRec.szASMFlag) <= 0 || strlen(srASMCRec.szASMFlag) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetASMFlag() ERROR !!");

                        if (szASMFlag == NULL)
                        {
                                inDISP_LogPrintf("szASMFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szASMFlag length = (%d)", (int)strlen(szASMFlag));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szASMFlag[0], &srASMCRec.szASMFlag[0], strlen(srASMCRec.szASMFlag));

        return (VS_SUCCESS);
}

/*
Function        :inSetASMFlag
Date&Time       :
Describe        :
*/
int inSetASMFlag(char* szASMFlag)
{
        memset(srASMCRec.szASMFlag, 0x00, sizeof(srASMCRec.szASMFlag));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szASMFlag == NULL || strlen(szASMFlag) <= 0 || strlen(szASMFlag) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetASMFlag() ERROR !!");

                        if (szASMFlag == NULL)
                        {
                                inDISP_LogPrintf("szASMFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szASMFlag length = (%d)", (int)strlen(szASMFlag));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szASMFlag[0], &szASMFlag[0], strlen(szASMFlag));

        return (VS_SUCCESS);
}

/*
Function        :inGetASMStartDate
Date&Time       :
Describe        :
*/
int inGetASMStartDate(char* szASMStartDate)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szASMStartDate == NULL || strlen(srASMCRec.szASMStartDate) <= 0 || strlen(srASMCRec.szASMStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetASMStartDate() ERROR !!");

                        if (szASMStartDate == NULL)
                        {
                                inDISP_LogPrintf("szASMStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szASMStartDate length = (%d)", (int)strlen(szASMStartDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szASMStartDate[0], &srASMCRec.szASMStartDate[0], strlen(srASMCRec.szASMStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetASMStartDate
Date&Time       :
Describe        :
*/
int inSetASMStartDate(char* szASMStartDate)
{
        memset(srASMCRec.szASMStartDate, 0x00, sizeof(srASMCRec.szASMStartDate));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szASMStartDate == NULL || strlen(szASMStartDate) <= 0 || strlen(szASMStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetASMStartDate() ERROR !!");

                        if (szASMStartDate == NULL)
                        {
                                inDISP_LogPrintf("szASMStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szASMStartDate length = (%d)", (int)strlen(szASMStartDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szASMStartDate[0], &szASMStartDate[0], strlen(szASMStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetASMEndDate
Date&Time       :
Describe        :
*/
int inGetASMEndDate(char* szASMEndDate)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szASMEndDate == NULL || strlen(srASMCRec.szASMEndDate) <= 0 || strlen(srASMCRec.szASMEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetASMEndDate() ERROR !!");

                        if (szASMEndDate == NULL)
                        {
                                inDISP_LogPrintf("szASMEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szASMEndDate length = (%d)", (int)strlen(szASMEndDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szASMEndDate[0], &srASMCRec.szASMEndDate[0], strlen(srASMCRec.szASMEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetASMEndDate
Date&Time       :
Describe        :
*/
int inSetASMEndDate(char* szASMEndDate)
{
        memset(srASMCRec.szASMEndDate, 0x00, sizeof(srASMCRec.szASMEndDate));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szASMEndDate == NULL || strlen(szASMEndDate) <= 0 || strlen(szASMEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetASMEndDate() ERROR !!");

                        if (szASMEndDate == NULL)
                        {
                                inDISP_LogPrintf("szASMEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szASMEndDate length = (%d)", (int)strlen(szASMEndDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szASMEndDate[0], &szASMEndDate[0], strlen(szASMEndDate));

        return (VS_SUCCESS);
}
