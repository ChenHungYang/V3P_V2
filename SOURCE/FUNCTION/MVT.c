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
#include "MVT.h"

static  MVT_REC srMVTRec;	/* construct MVT record */
extern  int	ginDebug;	/* Debug使用 extern */

/*
Function        :inLoadMVTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀MVT檔案，inMVTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadMVTRec(int inMVTRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆MVT檔案 */
        char            szMVTRec[_SIZE_MVT_REC_ + 1];           /* 暫存, 放各個欄位檔案 */
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnMVTLength = 0;                        /* MVT總長度 */
        long            lnReadLength;                           /* 記錄每次要從MVT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;                 /* inRec記錄讀到第幾筆, i為目前從MVT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadMVTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadMVTRec(%d) START!!", inMVTRec);
                inDISP_LogPrintf(szErrorMsg);
        }

        /* 判斷傳進來的inMVTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inMVTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inMVTRec < 0:(index = %d) ERROR!!", inMVTRec);
                        inDISP_LogPrintf(szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open MVT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_MVT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnMVTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_MVT_FILE_NAME_);

	if (lnMVTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnMVTLength + 1);
        uszTemp = malloc(lnMVTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnMVTLength + 1);
        memset(uszTemp, 0x00, lnMVTLength + 1);

         /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnMVTLength;

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
         *i為目前從MVT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnMVTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到MVT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                      	/* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
			inSearchResult = 1;

			/* 清空uszReadData */
                        memset(uszReadData, 0x00, lnMVTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
			memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
			/* 因為inMVT_Rec的index從0開始，所以inMVT_Rec要+1 */
                        if (inRec == (inMVTRec + 1))
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
         * 如果沒有inMVTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inMVTRec + 1) || inSearchResult == -1)
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
        memset(&srMVTRec, 0x00, sizeof(srMVTRec));
	/*
         * 以下pattern為存入MVT_Rec
         * i為MVT的第幾個字元
         * 存入MVT_Rec
         */
        i = 0;


        /* 01_EMV應用程式索引 */
	/* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
	k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("MVT unpack ERROR_1");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szApplicationIndex[0], &szMVTRec[0], k - 1);
        }

        /* 02_應用程式 ID */
	/* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("MVT unpack ERROR_2");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szMVTApplicationId[0], &szMVTRec[0], k - 1);
        }

        /* 03_Terminal_Type */
	/* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("MVT unpack ERROR_3");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szTerminalType[0], &szMVTRec[0], k - 1);
        }

        /* 04_Terminal_Capabilities */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("MVT unpack ERROR_4");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szTerminalCapabilities[0], &szMVTRec[0], k - 1);
        }

        /* 05_Additional_Terminal_Capabilities */
	/* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("MVT unpack ERROR_5");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szAdditionalTerminalCapabilities[0], &szMVTRec[0], k - 1);
        }

        /* 06_Terminal_Country_Code */
	/* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("MVT unpack ERROR_6");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szTerminalCountryCode[0], &szMVTRec[0], k - 1);
        }

        /* 07_Transaction_Currency_Code */
	/* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("MVT unpack ERROR_7");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szTransactionCurrencyCode[0], &szMVTRec[0], k - 1);
        }

        /* 08_Default_TAC */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("MVT unpack ERROR_8");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szDefaultTAC[0], &szMVTRec[0], k - 1);
        }

        /* 09_Online_TAC */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("MVT unpack ERROR_9");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szOnlineTAC[0], &szMVTRec[0], k - 1);
        }

        /* 10_Denial_TAC */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("MVT unpack ERROR_10");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szDenialTAC[0], &szMVTRec[0], k - 1);
        }

        /* 11_Default_TDOL */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("MVT unpack ERROR_11");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szDefaultTDOL[0], &szMVTRec[0], k - 1);
        }

        /* 12_Default_DDOL */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("MVT unpack ERROR_12");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szDefaultDDOL[0], &szMVTRec[0], k - 1);
        }

        /* 13_EMV_Floor_Limit */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("MVT unpack ERROR_13");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szEMVFloorLimit[0], &szMVTRec[0], k - 1);
        }

        /* 14_Random_Selection_Threshol */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("MVT unpack ERROR_14");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szRandomSelectionThreshold[0], &szMVTRec[0], k - 1);
        }

        /* 15_Target_Percent_for_Random_Selection */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("MVT unpack ERROR_15");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szTargetPercentforRandomSelection[0], &szMVTRec[0], k - 1);
        }

        /* 16_Max_Target_Percent_for_Random_Selection */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("MVT unpack ERROR_16");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szMaxTargetPercentforRandomSelection[0], &szMVTRec[0], k - 1);
        }

        /* 17_Merchant Category Code(MCC) */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("MVT unpack ERROR_17");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szMerchantCategoryCode[0], &szMVTRec[0], k - 1);
        }

        /* 18_Transaction Category Code(TCC) */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("MVT unpack ERROR_18");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szTransactionCategoryCode[0], &szMVTRec[0], k - 1);
        }

        /* 19_Transaction_Reference_Currency_Code */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("MVT unpack ERROR_19");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szTransactionReferenceCurrencyCode[0], &szMVTRec[0], k - 1);
        }

        /* 20_Transaction_Reference_Currency_Coversion */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("MVT unpack ERROR_20");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szTransactionReferenceCurrencyCoversion[0], &szMVTRec[0], k - 1);
        }

        /* 21_Transaction_Reference_Currency_Exponent */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("MVT unpack ERROR_21");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szTransactionReferenceCurrencyExponent[0], &szMVTRec[0], k - 1);
        }

        /* 22_等候EMV Offline PIN輸入的Time out時間  */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("MVT unpack ERROR_22");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srMVTRec.szEMVPINEntryTimeout[0], &szMVTRec[0], k - 1);
        }

	/* release */
	/* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadMVTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadMVTRec(%d) END!!", inMVTRec);
                inDISP_LogPrintf(szErrorMsg);
                inDISP_LogPrintf("----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveMVTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveMVTRec(int inMVTRec)
{
	unsigned long   uldat_Handle;   			/* FILE Handle */
	int inRecCount = 0, inTempIndex = 0;
	int     	inRetVal;
	int     	i = 0;

	char szTempTitle[] = "inSaveMVTRec";
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
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inSaveMVTRec INIT" );
#endif
	
	inTempIndex = inMVTRec;
	
	memset(szTempFIleName, 0x00, sizeof(szTempFIleName));
	strcat( szTempFIleName, _MVT_FILE_NAME_);
	
	memset(szTempBakFileName, 0x00, sizeof(szTempBakFileName));
	strcat( szTempBakFileName, _MVT_FILE_NAME_BAK_);
	
	inDISP_LogPrintfWithFlag("  %s Index[%d] ", szTempTitle, inTempIndex);

      	/* 開啟原檔案HDPT.dat */
	inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_MVT_FILE_NAME_);

	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  %s *Error* [%d] ",szTempTitle, inRetVal);
		inFunc_EDCLock();
		return(VS_ERROR);
	}

	usRetVal = CTOS_FileGetSize((unsigned char *)_MVT_FILE_NAME_, &ulFileSize);	
	
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
        uszWriteBuff_Record = malloc(_SIZE_MVT_REC_ + _SIZE_MVT_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_MVT_REC_ + _SIZE_MVT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */
   
        /* uszRead_Total_Buff儲存MVT.dat全部資料 */
        uszRead_Total_Buff = malloc(ulFileSize + 1);
        memset(uszRead_Total_Buff, 0x00, ulFileSize + 1);

        ulPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

        /* ApplicationIndex */
        memcpy(&uszWriteBuff_Record[0], &srMVTRec.szApplicationIndex[0], strlen(srMVTRec.szApplicationIndex));
        ulPackCount += strlen(srMVTRec.szApplicationIndex);
        uszWriteBuff_Record[ulPackCount] = 0x2C;
        ulPackCount++;

        /* ApplicationId */
        memcpy(&uszWriteBuff_Record[ulPackCount], &srMVTRec.szMVTApplicationId[0], strlen(srMVTRec.szMVTApplicationId));
        ulPackCount += strlen(srMVTRec.szMVTApplicationId);
        uszWriteBuff_Record[ulPackCount] = 0x2C;
        ulPackCount++;

        /* TerminalType */
        memcpy(&uszWriteBuff_Record[ulPackCount], &srMVTRec.szTerminalType[0], strlen(srMVTRec.szTerminalType));
        ulPackCount += strlen(srMVTRec.szTerminalType);
        uszWriteBuff_Record[ulPackCount] = 0x2C;
        ulPackCount++;

        /* TerminalCapabilities */
        memcpy(&uszWriteBuff_Record[ulPackCount], &srMVTRec.szTerminalCapabilities[0], strlen(srMVTRec.szTerminalCapabilities));
        ulPackCount += strlen(srMVTRec.szTerminalCapabilities);
        uszWriteBuff_Record[ulPackCount] = 0x2C;
        ulPackCount++;

        /* AdditionalTerminalCapabilities */
        memcpy(&uszWriteBuff_Record[ulPackCount], &srMVTRec.szAdditionalTerminalCapabilities[0], strlen(srMVTRec.szAdditionalTerminalCapabilities));
        ulPackCount += strlen(srMVTRec.szAdditionalTerminalCapabilities);
        uszWriteBuff_Record[ulPackCount] = 0x2C;
        ulPackCount++;

        /* TerminalCountryCode */
        memcpy(&uszWriteBuff_Record[ulPackCount], &srMVTRec.szTerminalCountryCode[0], strlen(srMVTRec.szTerminalCountryCode));
        ulPackCount += strlen(srMVTRec.szTerminalCountryCode);
        uszWriteBuff_Record[ulPackCount] = 0x2C;
        ulPackCount++;

        /* TransactionCurrencyCode */
        memcpy(&uszWriteBuff_Record[ulPackCount], &srMVTRec.szTransactionCurrencyCode[0], strlen(srMVTRec.szTransactionCurrencyCode));
        ulPackCount += strlen(srMVTRec.szTransactionCurrencyCode);
        uszWriteBuff_Record[ulPackCount] = 0x2C;
        ulPackCount++;

        /* DefaultTAC */
        memcpy(&uszWriteBuff_Record[ulPackCount], &srMVTRec.szDefaultTAC[0], strlen(srMVTRec.szDefaultTAC));
        ulPackCount += strlen(srMVTRec.szDefaultTAC);
        uszWriteBuff_Record[ulPackCount] = 0x2C;
        ulPackCount++;

        /* OnlineTAC */
        memcpy(&uszWriteBuff_Record[ulPackCount], &srMVTRec.szOnlineTAC[0], strlen(srMVTRec.szOnlineTAC));
        ulPackCount += strlen(srMVTRec.szOnlineTAC);
        uszWriteBuff_Record[ulPackCount] = 0x2C;
        ulPackCount++;

        /* DenialTAC */
        memcpy(&uszWriteBuff_Record[ulPackCount], &srMVTRec.szDenialTAC[0], strlen(srMVTRec.szDenialTAC));
        ulPackCount += strlen(srMVTRec.szDenialTAC);
        uszWriteBuff_Record[ulPackCount] = 0x2C;
        ulPackCount++;

        /* DefaultTDOL */
        memcpy(&uszWriteBuff_Record[ulPackCount], &srMVTRec.szDefaultTDOL[0], strlen(srMVTRec.szDefaultTDOL));
        ulPackCount += strlen(srMVTRec.szDefaultTDOL);
        uszWriteBuff_Record[ulPackCount] = 0x2C;
        ulPackCount++;

        /* DefaultDDOL */
        memcpy(&uszWriteBuff_Record[ulPackCount], &srMVTRec.szDefaultDDOL[0], strlen(srMVTRec.szDefaultDDOL));
        ulPackCount += strlen(srMVTRec.szDefaultDDOL);
        uszWriteBuff_Record[ulPackCount] = 0x2C;
        ulPackCount++;

        /* EMVFloorLimit */
        memcpy(&uszWriteBuff_Record[ulPackCount], &srMVTRec.szEMVFloorLimit[0], strlen(srMVTRec.szEMVFloorLimit));
        ulPackCount += strlen(srMVTRec.szEMVFloorLimit);
        uszWriteBuff_Record[ulPackCount] = 0x2C;
        ulPackCount++;

        /* RandomSelectionThreshold */
        memcpy(&uszWriteBuff_Record[ulPackCount], &srMVTRec.szRandomSelectionThreshold[0], strlen(srMVTRec.szRandomSelectionThreshold));
        ulPackCount += strlen(srMVTRec.szRandomSelectionThreshold);
        uszWriteBuff_Record[ulPackCount] = 0x2C;
        ulPackCount++;

        /* TargetPercentforRandomSelection */
        memcpy(&uszWriteBuff_Record[ulPackCount], &srMVTRec.szTargetPercentforRandomSelection[0], strlen(srMVTRec.szTargetPercentforRandomSelection));
        ulPackCount += strlen(srMVTRec.szTargetPercentforRandomSelection);
        uszWriteBuff_Record[ulPackCount] = 0x2C;
        ulPackCount++;

        /* MaxTargetPercentforRandomSelection */
        memcpy(&uszWriteBuff_Record[ulPackCount], &srMVTRec.szMaxTargetPercentforRandomSelection[0], strlen(srMVTRec.szMaxTargetPercentforRandomSelection));
        ulPackCount += strlen(srMVTRec.szMaxTargetPercentforRandomSelection);
        uszWriteBuff_Record[ulPackCount] = 0x2C;
        ulPackCount++;

        /* MerchantCategoryCode */
        memcpy(&uszWriteBuff_Record[ulPackCount], &srMVTRec.szMerchantCategoryCode[0], strlen(srMVTRec.szMerchantCategoryCode));
        ulPackCount += strlen(srMVTRec.szMerchantCategoryCode);
        uszWriteBuff_Record[ulPackCount] = 0x2C;
        ulPackCount++;

        /* TransactionCategoryCode */
        memcpy(&uszWriteBuff_Record[ulPackCount], &srMVTRec.szTransactionCategoryCode[0], strlen(srMVTRec.szTransactionCategoryCode));
        ulPackCount += strlen(srMVTRec.szTransactionCategoryCode);
        uszWriteBuff_Record[ulPackCount] = 0x2C;
        ulPackCount++;

        /* TransactionReferenceCurrencyCode */
        memcpy(&uszWriteBuff_Record[ulPackCount], &srMVTRec.szTransactionReferenceCurrencyCode[0], strlen(srMVTRec.szTransactionReferenceCurrencyCode));
        ulPackCount += strlen(srMVTRec.szTransactionReferenceCurrencyCode);
        uszWriteBuff_Record[ulPackCount] = 0x2C;
        ulPackCount++;

        /* TransactionReferenceCurrencyCoversion */
        memcpy(&uszWriteBuff_Record[ulPackCount], &srMVTRec.szTransactionReferenceCurrencyCoversion[0], strlen(srMVTRec.szTransactionReferenceCurrencyCoversion));
        ulPackCount += strlen(srMVTRec.szTransactionReferenceCurrencyCoversion);
        uszWriteBuff_Record[ulPackCount] = 0x2C;
        ulPackCount++;

        /* TransactionReferenceCurrencyExponent */
        memcpy(&uszWriteBuff_Record[ulPackCount], &srMVTRec.szTransactionReferenceCurrencyExponent[0], strlen(srMVTRec.szTransactionReferenceCurrencyExponent));
        ulPackCount += strlen(srMVTRec.szTransactionReferenceCurrencyExponent);
        uszWriteBuff_Record[ulPackCount] = 0x2C;
        ulPackCount++;

        /* EMVPINEntryTimeout */
        memcpy(&uszWriteBuff_Record[ulPackCount], &srMVTRec.szEMVPINEntryTimeout[0], strlen(srMVTRec.szEMVPINEntryTimeout));
        ulPackCount += strlen(srMVTRec.szEMVPINEntryTimeout);

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
	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inSaveMVTRec END");
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
Function        :inGetApplicationIndex
Date&Time       :
Describe        :
*/
int inGetApplicationIndex(char* szApplicationIndex)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szApplicationIndex == NULL || strlen(srMVTRec.szApplicationIndex) <= 0 || strlen(srMVTRec.szApplicationIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetApplicationIndex() ERROR !!");

                        if (szApplicationIndex == NULL)
                        {
                                inDISP_LogPrintf("szApplicationIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szApplicationIndex length = (%d)", (int)strlen(srMVTRec.szApplicationIndex));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }
                return (VS_ERROR);
        }
        memcpy(&szApplicationIndex[0], &srMVTRec.szApplicationIndex[0], strlen(srMVTRec.szApplicationIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetApplicationIndex
Date&Time       :
Describe        :
*/
int inSetApplicationIndex(char* szApplicationIndex)
{
        memset(srMVTRec.szApplicationIndex, 0x00, sizeof(srMVTRec.szApplicationIndex));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szApplicationIndex == NULL || strlen(szApplicationIndex) <= 0 || strlen(szApplicationIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetApplicationIndex() ERROR !!");

                        if (szApplicationIndex == NULL)
                        {
                                inDISP_LogPrintf("szApplicationIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szApplicationIndex length = (%d)", (int)strlen(szApplicationIndex));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szApplicationIndex[0], &szApplicationIndex[0], strlen(szApplicationIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetMVTApplicationId
Date&Time       :
Describe        :
*/
int inGetMVTApplicationId(char* szMVTApplicationId)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szMVTApplicationId == NULL || strlen(srMVTRec.szMVTApplicationId) <= 0 || strlen(srMVTRec.szMVTApplicationId) > 16)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetMVTApplicationId() ERROR !!");

                        if (szMVTApplicationId == NULL)
                        {
                                inDISP_LogPrintf("szApplicationId == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szApplicationId length = (%d)", (int)strlen(srMVTRec.szMVTApplicationId));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMVTApplicationId[0], &srMVTRec.szMVTApplicationId[0], strlen(srMVTRec.szMVTApplicationId));

        return (VS_SUCCESS);

}

/*
Function        :inSetMVTApplicationId
Date&Time       :
Describe        :
*/
int inSetMVTApplicationId(char* szMVTApplicationId)
{
        memset(srMVTRec.szMVTApplicationId, 0x00, sizeof(srMVTRec.szMVTApplicationId));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szMVTApplicationId == NULL || strlen(szMVTApplicationId) <= 0 || strlen(szMVTApplicationId) > 16)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetApplicationId() ERROR !!");

                        if (szMVTApplicationId == NULL)
                        {
                                inDISP_LogPrintf("szApplicationId == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szApplicationId length = (%d)", (int)strlen(szMVTApplicationId));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szMVTApplicationId[0], &szMVTApplicationId[0], strlen(szMVTApplicationId));

        return (VS_SUCCESS);
}

/*
Function        :inGetMVTTerminalType
Date&Time       :
Describe        :
*/
int inGetMVTTerminalType(char* szTerminalType)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTerminalType == NULL || strlen(srMVTRec.szTerminalType) <= 0 || strlen(srMVTRec.szTerminalType) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetMVTTerminalType() ERROR !!");

                        if (szTerminalType == NULL)
                        {
                                inDISP_LogPrintf("szTerminalType == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalType length = (%d)", (int)strlen(srMVTRec.szTerminalType));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTerminalType[0], &srMVTRec.szTerminalType[0], strlen(srMVTRec.szTerminalType));

        return (VS_SUCCESS);
}

/*
Function        :inSetTerminalType
Date&Time       :
Describe        :
*/
int inSetMVTTerminalType(char* szTerminalType)
{
        memset(srMVTRec.szTerminalType, 0x00, sizeof(srMVTRec.szTerminalType));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTerminalType == NULL || strlen(szTerminalType) <= 0 || strlen(szTerminalType) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTerminalType() ERROR !!");

                        if (szTerminalType == NULL)
                        {
                                inDISP_LogPrintf("szTerminalType == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalType length = (%d)", (int)strlen(szTerminalType));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szTerminalType[0], &szTerminalType[0], strlen(szTerminalType));

        return (VS_SUCCESS);
}

/*
Function        :inGetTerminalCapabilities
Date&Time       :
Describe        :
*/
int inGetTerminalCapabilities(char* szTerminalCapabilities)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTerminalCapabilities == NULL || strlen(srMVTRec.szTerminalCapabilities) <= 0 || strlen(srMVTRec.szTerminalCapabilities) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTerminalCapabilities() ERROR !!");

                        if (szTerminalCapabilities == NULL)
                        {
                                inDISP_LogPrintf("szTerminalCapabilities == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalCapabilities length = (%d)", (int)strlen(srMVTRec.szTerminalCapabilities));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTerminalCapabilities[0], &srMVTRec.szTerminalCapabilities[0], strlen(srMVTRec.szTerminalCapabilities));

        return (VS_SUCCESS);
}

/*
Function        :inSetTerminalCapabilities
Date&Time       :
Describe        :
*/
int inSetTerminalCapabilities(char* szTerminalCapabilities)
{
        memset(srMVTRec.szTerminalCapabilities, 0x00, sizeof(srMVTRec.szTerminalCapabilities));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTerminalCapabilities == NULL || strlen(szTerminalCapabilities) <= 0 || strlen(szTerminalCapabilities) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTerminalCapabilities() ERROR !!");

                        if (szTerminalCapabilities == NULL)
                        {
                                inDISP_LogPrintf("szTerminalCapabilities == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalCapabilities length = (%d)", (int)strlen(szTerminalCapabilities));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szTerminalCapabilities[0], &szTerminalCapabilities[0], strlen(szTerminalCapabilities));

        return (VS_SUCCESS);
}

/*
Function        :inGetAdditionalTerminalCapabilities
Date&Time       :
Describe        :
*/
int inGetAdditionalTerminalCapabilities(char* szAdditionalTerminalCapabilities)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szAdditionalTerminalCapabilities == NULL || strlen(srMVTRec.szAdditionalTerminalCapabilities) <= 0 || strlen(srMVTRec.szAdditionalTerminalCapabilities) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetAdditionalTerminalCapabilities() ERROR !!");

                        if (szAdditionalTerminalCapabilities == NULL)
                        {
                                inDISP_LogPrintf("szAdditionalTerminalCapabilities == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szAdditionalTerminalCapabilities length = (%d)", (int)strlen(srMVTRec.szAdditionalTerminalCapabilities));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szAdditionalTerminalCapabilities[0], &srMVTRec.szAdditionalTerminalCapabilities[0], strlen(srMVTRec.szAdditionalTerminalCapabilities));

        return (VS_SUCCESS);
}

/*
Function        :inSetAdditionalTerminalCapabilities
Date&Time       :
Describe        :
*/
int inSetAdditionalTerminalCapabilities(char* szAdditionalTerminalCapabilities)
{
        memset(srMVTRec.szAdditionalTerminalCapabilities, 0x00, sizeof(srMVTRec.szAdditionalTerminalCapabilities));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szAdditionalTerminalCapabilities == NULL || strlen(szAdditionalTerminalCapabilities) <= 0 || strlen(szAdditionalTerminalCapabilities) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetAdditionalTerminalCapabilities() ERROR !!");

                        if (szAdditionalTerminalCapabilities == NULL)
                        {
                                inDISP_LogPrintf("szAdditionalTerminalCapabilities == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szAdditionalTerminalCapabilities length = (%d)", (int)strlen(szAdditionalTerminalCapabilities));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szAdditionalTerminalCapabilities[0], &szAdditionalTerminalCapabilities[0], strlen(szAdditionalTerminalCapabilities));

        return (VS_SUCCESS);
}

/*
Function        :inGetMVTTerminalCountryCode
Date&Time       :
Describe        :
*/
int inGetMVTTerminalCountryCode(char* szTerminalCountryCode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTerminalCountryCode == NULL || strlen(srMVTRec.szTerminalCountryCode) <= 0 || strlen(srMVTRec.szTerminalCountryCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetMVTTerminalCountryCode() ERROR !!");

                        if (szTerminalCountryCode == NULL)
                        {
                                inDISP_LogPrintf("szTerminalCountryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalCountryCode length = (%d)", (int)strlen(srMVTRec.szTerminalCountryCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTerminalCountryCode[0], &srMVTRec.szTerminalCountryCode[0], strlen(srMVTRec.szTerminalCountryCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetMVTTerminalCountryCode
Date&Time       :
Describe        :
*/
int inSetMVTTerminalCountryCode(char* szTerminalCountryCode)
{
        memset(srMVTRec.szTerminalCountryCode, 0x00, sizeof(srMVTRec.szTerminalCountryCode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTerminalCountryCode == NULL || strlen(szTerminalCountryCode) <= 0 || strlen(szTerminalCountryCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTerminalCountryCode() ERROR !!");

                        if (szTerminalCountryCode == NULL)
                        {
                                inDISP_LogPrintf("szTerminalCountryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalCountryCode length = (%d)", (int)strlen(szTerminalCountryCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szTerminalCountryCode[0], &szTerminalCountryCode[0], strlen(szTerminalCountryCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetMVTTransactionCurrencyCode
Date&Time       :
Describe        :
*/
int inGetMVTTransactionCurrencyCode(char* szTransactionCurrencyCode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTransactionCurrencyCode == NULL || strlen(srMVTRec.szTransactionCurrencyCode) <= 0 || strlen(srMVTRec.szTransactionCurrencyCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetMVTTransactionCurrencyCode() ERROR !!");

                        if (szTransactionCurrencyCode == NULL)
                        {
                                inDISP_LogPrintf("szTransactionCurrencyCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionCurrencyCode length = (%d)", (int)strlen(srMVTRec.szTransactionCurrencyCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTransactionCurrencyCode[0], &srMVTRec.szTransactionCurrencyCode[0], strlen(srMVTRec.szTransactionCurrencyCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetMVTTransactionCurrencyCode
Date&Time       :
Describe        :
*/
int inSetMVTTransactionCurrencyCode(char* szTransactionCurrencyCode)
{
        memset(srMVTRec.szTransactionCurrencyCode, 0x00, sizeof(srMVTRec.szTransactionCurrencyCode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTransactionCurrencyCode == NULL || strlen(szTransactionCurrencyCode) <= 0 || strlen(szTransactionCurrencyCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTransactionCurrencyCode() ERROR !!");

                        if (szTransactionCurrencyCode == NULL)
                        {
                                inDISP_LogPrintf("szTransactionCurrencyCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionCurrencyCode length = (%d)", (int)strlen(szTransactionCurrencyCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szTransactionCurrencyCode[0], &szTransactionCurrencyCode[0], strlen(szTransactionCurrencyCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetDefaultTAC
Date&Time       :
Describe        :
*/
int inGetDefaultTAC(char* szDefaultTAC)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szDefaultTAC == NULL || strlen(srMVTRec.szDefaultTAC) <= 0 || strlen(srMVTRec.szDefaultTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetDefaultTAC() ERROR !!");

                        if (szDefaultTAC == NULL)
                        {
                                inDISP_LogPrintf("szDefaultTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDefaultTAC length = (%d)", (int)strlen(srMVTRec.szDefaultTAC));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDefaultTAC[0], &srMVTRec.szDefaultTAC[0], strlen(srMVTRec.szDefaultTAC));

        return (VS_SUCCESS);
}

/*
Function        :inSetDefaultTAC
Date&Time       :
Describe        :
*/
int inSetDefaultTAC(char* szDefaultTAC)
{
        memset(srMVTRec.szDefaultTAC, 0x00, sizeof(srMVTRec.szDefaultTAC));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szDefaultTAC == NULL || strlen(szDefaultTAC) <= 0 || strlen(szDefaultTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetDefaultTAC() ERROR !!");

                        if (szDefaultTAC == NULL)
                        {
                                inDISP_LogPrintf("szDefaultTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDefaultTAC length = (%d)", (int)strlen(szDefaultTAC));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szDefaultTAC[0], &szDefaultTAC[0], strlen(szDefaultTAC));

        return (VS_SUCCESS);
}

/*
Function        :inGetOnlineTAC
Date&Time       :
Describe        :
*/
int inGetOnlineTAC(char* szOnlineTAC)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szOnlineTAC == NULL || strlen(srMVTRec.szOnlineTAC) <= 0 || strlen(srMVTRec.szOnlineTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetOnlineTAC() ERROR !!");

                        if (szOnlineTAC == NULL)
                        {
                                inDISP_LogPrintf("szOnlineTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szOnlineTAC length = (%d)", (int)strlen(srMVTRec.szOnlineTAC));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szOnlineTAC[0], &srMVTRec.szOnlineTAC[0], strlen(srMVTRec.szOnlineTAC));

        return (VS_SUCCESS);
}

/*
Function        :inSetOnlineTAC
Date&Time       :
Describe        :
*/
int inSetOnlineTAC(char* szOnlineTAC)
{
        memset(srMVTRec.szOnlineTAC, 0x00, sizeof(srMVTRec.szOnlineTAC));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szOnlineTAC == NULL || strlen(szOnlineTAC) <= 0 || strlen(szOnlineTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetOnlineTAC() ERROR !!");

                        if (szOnlineTAC == NULL)
                        {
                                inDISP_LogPrintf("szOnlineTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szOnlineTAC length = (%d)", (int)strlen(szOnlineTAC));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szOnlineTAC[0], &szOnlineTAC[0], strlen(szOnlineTAC));

        return (VS_SUCCESS);
}

/*
Function        :inGetDenialTAC
Date&Time       :
Describe        :
*/
int inGetDenialTAC(char* szDenialTAC)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szDenialTAC == NULL || strlen(srMVTRec.szDenialTAC) <= 0 || strlen(srMVTRec.szDenialTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetDenialTAC() ERROR !!");

                        if (szDenialTAC == NULL)
                        {
                                inDISP_LogPrintf("szDenialTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDenialTAC length = (%d)", (int)strlen(srMVTRec.szDenialTAC));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDenialTAC[0], &srMVTRec.szDenialTAC[0], strlen(srMVTRec.szDenialTAC));

        return (VS_SUCCESS);
}

/*
Function        :inSetDenialTAC
Date&Time       :
Describe        :
*/
int inSetDenialTAC(char* szDenialTAC)
{
        memset(srMVTRec.szDenialTAC, 0x00, sizeof(srMVTRec.szDenialTAC));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szDenialTAC == NULL || strlen(szDenialTAC) <= 0 || strlen(szDenialTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetDenialTAC() ERROR !!");

                        if (szDenialTAC == NULL)
                        {
                                inDISP_LogPrintf("szDenialTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDenialTAC length = (%d)", (int)strlen(szDenialTAC));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szDenialTAC[0], &szDenialTAC[0], strlen(szDenialTAC));

        return (VS_SUCCESS);
}

/*
Function        :inGetDefaultTDOL
Date&Time       :
Describe        :
*/
int inGetDefaultTDOL(char* szDefaultTDOL)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szDefaultTDOL == NULL || strlen(srMVTRec.szDefaultTDOL) <= 0 || strlen(srMVTRec.szDefaultTDOL) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetDefaultTDOL() ERROR !!");

                        if (szDefaultTDOL == NULL)
                        {
                                inDISP_LogPrintf("szDefaultTDOL == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDefaultTDOL length = (%d)", (int)strlen(srMVTRec.szDefaultTDOL));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDefaultTDOL[0], &srMVTRec.szDefaultTDOL[0], strlen(srMVTRec.szDefaultTDOL));

        return (VS_SUCCESS);

}

/*
Function        :inSetDefaultTDOL
Date&Time       :
Describe        :
*/
int inSetDefaultTDOL(char* szDefaultTDOL)
{
        memset(srMVTRec.szDefaultTDOL, 0x00, sizeof(srMVTRec.szDefaultTDOL));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szDefaultTDOL == NULL || strlen(szDefaultTDOL) <= 0 || strlen(szDefaultTDOL) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetDefaultTDOL() ERROR !!");

                        if (szDefaultTDOL == NULL)
                        {
                                inDISP_LogPrintf("szDefaultTDOL == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDefaultTDOL length = (%d)", (int)strlen(szDefaultTDOL));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szDefaultTDOL[0], &szDefaultTDOL[0], strlen(szDefaultTDOL));

        return (VS_SUCCESS);
}

/*
Function        :inGetDefaultDDOL
Date&Time       :
Describe        :
*/
int inGetDefaultDDOL(char* szDefaultDDOL)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szDefaultDDOL == NULL || strlen(srMVTRec.szDefaultDDOL) <= 0 || strlen(srMVTRec.szDefaultDDOL) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetDefaultDDOL() ERROR !!");

                        if (szDefaultDDOL == NULL)
                        {
                                inDISP_LogPrintf("szDefaultDDOL == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDefaultDDOL length = (%d)", (int)strlen(srMVTRec.szDefaultDDOL));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDefaultDDOL[0], &srMVTRec.szDefaultDDOL[0], strlen(srMVTRec.szDefaultDDOL));

        return (VS_SUCCESS);
}

/*
Function        :inSetDefaultDDOL
Date&Time       :
Describe        :
*/
int inSetDefaultDDOL(char* szDefaultDDOL)
{
        memset(srMVTRec.szDefaultDDOL, 0x00, sizeof(srMVTRec.szDefaultDDOL));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szDefaultDDOL == NULL || strlen(szDefaultDDOL) <= 0 || strlen(szDefaultDDOL) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetDefaultDDOL() ERROR !!");

                        if (szDefaultDDOL == NULL)
                        {
                                inDISP_LogPrintf("szDefaultDDOL == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDefaultDDOL length = (%d)", (int)strlen(szDefaultDDOL));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szDefaultDDOL[0], &szDefaultDDOL[0], strlen(szDefaultDDOL));

        return (VS_SUCCESS);
}

/*
Function        :inGetEMVFloorLimit
Date&Time       :
Describe        :
*/
int inGetEMVFloorLimit(char* szEMVFloorLimit)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szEMVFloorLimit == NULL || strlen(srMVTRec.szEMVFloorLimit) <= 0 || strlen(srMVTRec.szEMVFloorLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetEMVFloorLimit() ERROR !!");

                        if (szEMVFloorLimit == NULL)
                        {
                                inDISP_LogPrintf("szEMVFloorLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEMVFloorLimit length = (%d)", (int)strlen(srMVTRec.szEMVFloorLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szEMVFloorLimit[0], &srMVTRec.szEMVFloorLimit[0], strlen(srMVTRec.szEMVFloorLimit));

        return (VS_SUCCESS);
}

/*
Function        :inSetEMVFloorLimit
Date&Time       :
Describe        :
*/
int inSetEMVFloorLimit(char* szEMVFloorLimit)
{
        memset(srMVTRec.szEMVFloorLimit, 0x00, sizeof(srMVTRec.szEMVFloorLimit));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szEMVFloorLimit == NULL || strlen(szEMVFloorLimit) <= 0 || strlen(szEMVFloorLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetEMVFloorLimit() ERROR !!");

                        if (szEMVFloorLimit == NULL)
                        {
                                inDISP_LogPrintf("szEMVFloorLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEMVFloorLimit length = (%d)", (int)strlen(szEMVFloorLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szEMVFloorLimit[0], &szEMVFloorLimit[0], strlen(szEMVFloorLimit));

        return (VS_SUCCESS);
}

/*
Function        :inGetRandomSelectionThreshold
Date&Time       :
Describe        :
*/
int inGetRandomSelectionThreshold(char* szRandomSelectionThreshold)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szRandomSelectionThreshold == NULL || strlen(srMVTRec.szRandomSelectionThreshold) <= 0 || strlen(srMVTRec.szRandomSelectionThreshold) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetRandomSelectionThreshold() ERROR !!");

                        if (szRandomSelectionThreshold == NULL)
                        {
                                inDISP_LogPrintf("szRandomSelectionThreshold == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szRandomSelectionThreshold length = (%d)", (int)strlen(srMVTRec.szRandomSelectionThreshold));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szRandomSelectionThreshold[0], &srMVTRec.szRandomSelectionThreshold[0], strlen(srMVTRec.szRandomSelectionThreshold));

        return (VS_SUCCESS);
}

/*
Function        :inSetRandomSelectionThreshold
Date&Time       :
Describe        :
*/
int inSetRandomSelectionThreshold(char* szRandomSelectionThreshold)
{
        memset(srMVTRec.szRandomSelectionThreshold, 0x00, sizeof(srMVTRec.szRandomSelectionThreshold));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szRandomSelectionThreshold == NULL || strlen(szRandomSelectionThreshold) <= 0 || strlen(szRandomSelectionThreshold) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetRandomSelectionThreshold() ERROR !!");

                        if (szRandomSelectionThreshold == NULL)
                        {
                                inDISP_LogPrintf("szRandomSelectionThreshold == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szRandomSelectionThreshold length = (%d)", (int)strlen(szRandomSelectionThreshold));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szRandomSelectionThreshold[0], &szRandomSelectionThreshold[0], strlen(szRandomSelectionThreshold));

        return (VS_SUCCESS);
}

/*
Function        :inGetTargetPercentforRandomSelection
Date&Time       :
Describe        :
*/
int inGetTargetPercentforRandomSelection(char* szTargetPercentforRandomSelection)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTargetPercentforRandomSelection == NULL || strlen(srMVTRec.szTargetPercentforRandomSelection) <= 0 || strlen(srMVTRec.szTargetPercentforRandomSelection) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTargetPercentforRandomSelection() ERROR !!");

                        if (szTargetPercentforRandomSelection == NULL)
                        {
                                inDISP_LogPrintf("szTargetPercentforRandomSelection == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTargetPercentforRandomSelection length = (%d)", (int)strlen(srMVTRec.szTargetPercentforRandomSelection));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTargetPercentforRandomSelection[0], &srMVTRec.szTargetPercentforRandomSelection[0], strlen(srMVTRec.szTargetPercentforRandomSelection));

        return (VS_SUCCESS);
}

/*
Function        :inSetTargetPercentforRandomSelection
Date&Time       :
Describe        :
*/
int inSetTargetPercentforRandomSelection(char* szTargetPercentforRandomSelection)
{
        memset(srMVTRec.szTargetPercentforRandomSelection, 0x00, sizeof(srMVTRec.szTargetPercentforRandomSelection));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTargetPercentforRandomSelection == NULL || strlen(szTargetPercentforRandomSelection) <= 0 || strlen(szTargetPercentforRandomSelection) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTargetPercentforRandomSelection() ERROR !!");

                        if (szTargetPercentforRandomSelection == NULL)
                        {
                                inDISP_LogPrintf("szTargetPercentforRandomSelection == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTargetPercentforRandomSelection length = (%d)", (int)strlen(szTargetPercentforRandomSelection));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szTargetPercentforRandomSelection[0], &szTargetPercentforRandomSelection[0], strlen(szTargetPercentforRandomSelection));

        return (VS_SUCCESS);
}

/*
Function        :inGetMaxTargetPercentforRandomSelection
Date&Time       :
Describe        :
*/
int inGetMaxTargetPercentforRandomSelection(char* szMaxTargetPercentforRandomSelection)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szMaxTargetPercentforRandomSelection == NULL || strlen(srMVTRec.szMaxTargetPercentforRandomSelection) <= 0 || strlen(srMVTRec.szMaxTargetPercentforRandomSelection) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetMaxTargetPercentforRandomSelection() ERROR !!");

                        if (szMaxTargetPercentforRandomSelection == NULL)
                        {
                                inDISP_LogPrintf("szMaxTargetPercentforRandomSelection == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMaxTargetPercentforRandomSelection length = (%d)", (int)strlen(srMVTRec.szMaxTargetPercentforRandomSelection));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMaxTargetPercentforRandomSelection[0], &srMVTRec.szMaxTargetPercentforRandomSelection[0], strlen(srMVTRec.szMaxTargetPercentforRandomSelection));

        return (VS_SUCCESS);
}

/*
Function        :inSetMaxTargetPercentforRandomSelection
Date&Time       :
Describe        :
*/
int inSetMaxTargetPercentforRandomSelection(char* szMaxTargetPercentforRandomSelection)
{
        memset(srMVTRec.szMaxTargetPercentforRandomSelection, 0x00, sizeof(srMVTRec.szMaxTargetPercentforRandomSelection));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szMaxTargetPercentforRandomSelection == NULL || strlen(szMaxTargetPercentforRandomSelection) <= 0 || strlen(szMaxTargetPercentforRandomSelection) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetMaxTargetPercentforRandomSelection() ERROR !!");

                        if (szMaxTargetPercentforRandomSelection == NULL)
                        {
                                inDISP_LogPrintf("szMaxTargetPercentforRandomSelection == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMaxTargetPercentforRandomSelection length = (%d)", (int)strlen(szMaxTargetPercentforRandomSelection));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szMaxTargetPercentforRandomSelection[0], &szMaxTargetPercentforRandomSelection[0], strlen(szMaxTargetPercentforRandomSelection));

        return (VS_SUCCESS);
}

/*
Function        :inGetMVTMerchantCategoryCode
Date&Time       :
Describe        :
*/
int inGetMVTMerchantCategoryCode(char* szMerchantCategoryCode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szMerchantCategoryCode == NULL || strlen(srMVTRec.szMerchantCategoryCode) <= 0 || strlen(srMVTRec.szMerchantCategoryCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetMVTMerchantCategoryCode() ERROR !!");

                        if (szMerchantCategoryCode == NULL)
                        {
                                inDISP_LogPrintf("szMerchantCategoryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMerchantCategoryCode length = (%d)", (int)strlen(srMVTRec.szMerchantCategoryCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMerchantCategoryCode[0], &srMVTRec.szMerchantCategoryCode[0], strlen(srMVTRec.szMerchantCategoryCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetMVTMerchantCategoryCode
Date&Time       :
Describe        :
*/
int inSetMVTMerchantCategoryCode(char* szMerchantCategoryCode)
{
        memset(srMVTRec.szMerchantCategoryCode, 0x00, sizeof(srMVTRec.szMerchantCategoryCode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szMerchantCategoryCode == NULL || strlen(szMerchantCategoryCode) <= 0 || strlen(szMerchantCategoryCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetMerchantCategoryCode() ERROR !!");

                        if (szMerchantCategoryCode == NULL)
                        {
                                inDISP_LogPrintf("szMerchantCategoryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMerchantCategoryCode length = (%d)", (int)strlen(szMerchantCategoryCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szMerchantCategoryCode[0], &szMerchantCategoryCode[0], strlen(szMerchantCategoryCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetMVTTransactionCategoryCode
Date&Time       :
Describe        :
*/
int inGetMVTTransactionCategoryCode(char* szTransactionCategoryCode)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTransactionCategoryCode == NULL || strlen(srMVTRec.szTransactionCategoryCode) <= 0 || strlen(srMVTRec.szTransactionCategoryCode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetMVTTransactionCategoryCode() ERROR !!");

                        if (szTransactionCategoryCode == NULL)
                        {
                                inDISP_LogPrintf("szTransactionCategoryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionCategoryCode length = (%d)", (int)strlen(srMVTRec.szTransactionCategoryCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTransactionCategoryCode[0], &srMVTRec.szTransactionCategoryCode[0], strlen(srMVTRec.szTransactionCategoryCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetMVTTransactionCategoryCode
Date&Time       :
Describe        :
*/
int inSetMVTTransactionCategoryCode(char* szTransactionCategoryCode)
{
        memset(srMVTRec.szTransactionCategoryCode, 0x00, sizeof(srMVTRec.szTransactionCategoryCode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTransactionCategoryCode == NULL || strlen(szTransactionCategoryCode) <= 0 || strlen(szTransactionCategoryCode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szTransactionCategoryCode == NULL)
                        {
                                inDISP_LogPrintf("inSetSloganPrtPositio() ERROR !! szTransactionCategoryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inSetSloganPrtPositio() ERROR !! szTransactionCategoryCode length = (%d)", (int)strlen(szTransactionCategoryCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szTransactionCategoryCode[0], &szTransactionCategoryCode[0], strlen(szTransactionCategoryCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetTransactionReferenceCurrencyCode
Date&Time       :
Describe        :
*/
int inGetTransactionReferenceCurrencyCode(char* szTransactionReferenceCurrencyCode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTransactionReferenceCurrencyCode == NULL || strlen(srMVTRec.szTransactionReferenceCurrencyCode) <= 0 || strlen(srMVTRec.szTransactionReferenceCurrencyCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTransactionReferenceCurrencyCode() ERROR !!");

                        if (szTransactionReferenceCurrencyCode == NULL)
                        {
                                inDISP_LogPrintf("szTransactionReferenceCurrencyCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionReferenceCurrencyCode length = (%d)", (int)strlen(srMVTRec.szTransactionReferenceCurrencyCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTransactionReferenceCurrencyCode[0], &srMVTRec.szTransactionReferenceCurrencyCode[0], strlen(srMVTRec.szTransactionReferenceCurrencyCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetTransactionReferenceCurrencyCode
Date&Time       :
Describe        :
*/
int inSetTransactionReferenceCurrencyCode(char* szTransactionReferenceCurrencyCode)
{
        memset(srMVTRec.szTransactionReferenceCurrencyCode, 0x00, sizeof(srMVTRec.szTransactionReferenceCurrencyCode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTransactionReferenceCurrencyCode == NULL || strlen(szTransactionReferenceCurrencyCode) <= 0 || strlen(szTransactionReferenceCurrencyCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTransactionReferenceCurrencyCode() ERROR !!");

                        if (szTransactionReferenceCurrencyCode == NULL)
                        {
                                inDISP_LogPrintf("szTransactionReferenceCurrencyCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionReferenceCurrencyCode length = (%d)", (int)strlen(szTransactionReferenceCurrencyCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szTransactionReferenceCurrencyCode[0], &szTransactionReferenceCurrencyCode[0], strlen(szTransactionReferenceCurrencyCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetTransactionReferenceCurrencyCoversion
Date&Time       :
Describe        :
*/
int inGetTransactionReferenceCurrencyCoversion(char* szTransactionReferenceCurrencyCoversion)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTransactionReferenceCurrencyCoversion == NULL || strlen(srMVTRec.szTransactionReferenceCurrencyCoversion) <= 0 || strlen(srMVTRec.szTransactionReferenceCurrencyCoversion) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTransactionReferenceCurrencyCoversion() ERROR !!");

                        if (szTransactionReferenceCurrencyCoversion == NULL)
                        {
                                inDISP_LogPrintf("szTransactionReferenceCurrencyCoversion == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionReferenceCurrencyCoversion length = (%d)", (int)strlen(srMVTRec.szTransactionReferenceCurrencyCoversion));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTransactionReferenceCurrencyCoversion[0], &srMVTRec.szTransactionReferenceCurrencyCoversion[0], strlen(srMVTRec.szTransactionReferenceCurrencyCoversion));

        return (VS_SUCCESS);
}

/*
Function        :inSetTransactionReferenceCurrencyCoversion
Date&Time       :
Describe        :
*/
int inSetTransactionReferenceCurrencyCoversion(char* szTransactionReferenceCurrencyCoversion)
{
        memset(srMVTRec.szTransactionReferenceCurrencyCoversion, 0x00, sizeof(srMVTRec.szTransactionReferenceCurrencyCoversion));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTransactionReferenceCurrencyCoversion == NULL || strlen(szTransactionReferenceCurrencyCoversion) <= 0 || strlen(szTransactionReferenceCurrencyCoversion) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTransactionReferenceCurrencyCoversion() ERROR !!");

                        if (szTransactionReferenceCurrencyCoversion == NULL)
                        {
                                inDISP_LogPrintf("szTransactionReferenceCurrencyCoversion == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionReferenceCurrencyCoversion length = (%d)", (int)strlen(szTransactionReferenceCurrencyCoversion));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szTransactionReferenceCurrencyCoversion[0], &szTransactionReferenceCurrencyCoversion[0], strlen(szTransactionReferenceCurrencyCoversion));

        return (VS_SUCCESS);
}

/*
Function        :inGetTransactionReferenceCurrencyExponent
Date&Time       :
Describe        :
*/
int inGetTransactionReferenceCurrencyExponent(char* szTransactionReferenceCurrencyExponent)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTransactionReferenceCurrencyExponent == NULL || strlen(srMVTRec.szTransactionReferenceCurrencyExponent) <= 0 || strlen(srMVTRec.szTransactionReferenceCurrencyExponent) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTransactionReferenceCurrencyExponent() ERROR !!");

                        if (szTransactionReferenceCurrencyExponent == NULL)
                        {
                                inDISP_LogPrintf("szTransactionReferenceCurrencyExponent == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionReferenceCurrencyExponent length = (%d)", (int)strlen(srMVTRec.szTransactionReferenceCurrencyExponent));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTransactionReferenceCurrencyExponent[0], &srMVTRec.szTransactionReferenceCurrencyExponent[0], strlen(srMVTRec.szTransactionReferenceCurrencyExponent));

        return (VS_SUCCESS);
}

/*
Function        :inSetTransactionReferenceCurrencyExponent
Date&Time       :
Describe        :
*/
int inSetTransactionReferenceCurrencyExponent(char* szTransactionReferenceCurrencyExponent)
{
        memset(srMVTRec.szTransactionReferenceCurrencyExponent, 0x00, sizeof(srMVTRec.szTransactionReferenceCurrencyExponent));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTransactionReferenceCurrencyExponent == NULL || strlen(szTransactionReferenceCurrencyExponent) <= 0 || strlen(szTransactionReferenceCurrencyExponent) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTransactionReferenceCurrencyExponent() ERROR !!");

                        if (szTransactionReferenceCurrencyExponent == NULL)
                        {
                                inDISP_LogPrintf("szTransactionReferenceCurrencyExponent == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionReferenceCurrencyExponent length = (%d)", (int)strlen(szTransactionReferenceCurrencyExponent));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szTransactionReferenceCurrencyExponent[0], &szTransactionReferenceCurrencyExponent[0], strlen(szTransactionReferenceCurrencyExponent));

        return (VS_SUCCESS);
}

/*
Function        :inGetEMVPINEntryTimeout
Date&Time       :
Describe        :
*/
int inGetEMVPINEntryTimeout(char* szEMVPINEntryTimeout)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szEMVPINEntryTimeout == NULL || strlen(srMVTRec.szEMVPINEntryTimeout) <= 0 || strlen(srMVTRec.szEMVPINEntryTimeout) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetEMVPINEntryTimeout() ERROR !!");

                        if (szEMVPINEntryTimeout == NULL)
                        {
                                inDISP_LogPrintf("szEMVPINEntryTimeout == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEMVPINEntryTimeout length = (%d)", (int)strlen(srMVTRec.szEMVPINEntryTimeout));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szEMVPINEntryTimeout[0], &srMVTRec.szEMVPINEntryTimeout[0], strlen(srMVTRec.szEMVPINEntryTimeout));

        return (VS_SUCCESS);

}

/*
Function        :inSetEMVPINEntryTimeout
Date&Time       :
Describe        :
*/
int inSetEMVPINEntryTimeout(char* szEMVPINEntryTimeout)
{
        memset(srMVTRec.szEMVPINEntryTimeout, 0x00, sizeof(srMVTRec.szEMVPINEntryTimeout));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szEMVPINEntryTimeout == NULL || strlen(szEMVPINEntryTimeout) <= 0 || strlen(szEMVPINEntryTimeout) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetEMVPINEntryTimeout() ERROR !!");

                        if (szEMVPINEntryTimeout == NULL)
                        {
                                inDISP_LogPrintf("szEMVPINEntryTimeout == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEMVPINEntryTimeout length = (%d)", (int)strlen(szEMVPINEntryTimeout));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szEMVPINEntryTimeout[0], &szEMVPINEntryTimeout[0], strlen(szEMVPINEntryTimeout));

        return (VS_SUCCESS);
}
