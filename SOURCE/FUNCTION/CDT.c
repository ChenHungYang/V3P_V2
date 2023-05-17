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
#include "CDT.h"

static  CDT_REC srCDTRec;	/* construct CDT record */
extern  int     ginDebug;       /* Debug使用 extern */
static int inRealDataCount = _SIZE_CDT_COMMA_0D0A_ ;
/*
Function        :inLoadCDTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀CDT檔案，inCDTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadCDTRec(int inCDTRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆CDT檔案 */
        char            szCDTRec[_SIZE_CDT_REC_ + 1];           /* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnCDTLength = 0;                        /* CDT總長度 */
        long            lnReadLength;                           /* 記錄每次要從CDT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從CDT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadCDTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadCDTRec(%d) START!!", inCDTRec);
                inDISP_LogPrintf(szErrorMsg);
        }

        /* 判斷傳進來的inCDTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inCDTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inCDTRec < 0:(index = %d) ERROR!!", inCDTRec);
                        inDISP_LogPrintf(szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open CDT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_CDT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnCDTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_CDT_FILE_NAME_);

        if (lnCDTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnCDTLength + 1);
        uszTemp = malloc(lnCDTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnCDTLength + 1);
        memset(uszTemp, 0x00, lnCDTLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnCDTLength;

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
         *i為目前從CDT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnCDTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到CDT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnCDTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inCDT_Rec的index從0開始，所以inCDT_Rec要+1 */
                        if (inRec == (inCDTRec + 1))
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
         * 如果沒有inCDTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inCDTRec + 1) || inSearchResult == -1)
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
        memset(&srCDTRec, 0x00, sizeof(srCDTRec));
        /*
         * 以下pattern為存入CDT_Rec
         * i為CDT的第幾個字元
         * 存入CDT_Rec
         */
        i = 0;


        /* 01_卡別索引 */
        /* 初始化 */
        memset(szCDTRec, 0x00, sizeof(szCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTRec[k ++] = uszReadData[i ++];
                
                if (szCDTRec[k - 1] == 0x2C	||
		    szCDTRec[k - 1] == 0x0D	||
		    szCDTRec[k - 1] == 0x0A	||
		    szCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CDT unpack ERROR.");
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
        if (szCDTRec[0] != 0x2C	&&
	    szCDTRec[0] != 0x0D	&&
	    szCDTRec[0] != 0x0A	&&
	    szCDTRec[0] != 0x00)
        {
                memcpy(&srCDTRec.szCardIndex[0], &szCDTRec[0], k - 1);
        }

        /* 02_低卡號範圍 */
        /* 初始化 */
        memset(szCDTRec, 0x00, sizeof(szCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTRec[k ++] = uszReadData[i ++];
                if (szCDTRec[k - 1] == 0x2C	||
		    szCDTRec[k - 1] == 0x0D	||
		    szCDTRec[k - 1] == 0x0A	||
		    szCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CDT unpack ERROR");
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
        if (szCDTRec[0] != 0x2C	&&
	    szCDTRec[0] != 0x0D	&&
	    szCDTRec[0] != 0x0A	&&
	    szCDTRec[0] != 0x00)
        {
                memcpy(&srCDTRec.szLowBinRange[0], &szCDTRec[0], k - 1);
        }

        /* 03_高卡號範圍 */
        /* 初始化 */
        memset(szCDTRec, 0x00, sizeof(szCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTRec[k ++] = uszReadData[i ++];
                if (szCDTRec[k - 1] == 0x2C	||
		    szCDTRec[k - 1] == 0x0D	||
		    szCDTRec[k - 1] == 0x0A	||
		    szCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CDT unpack ERROR");
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
        if (szCDTRec[0] != 0x2C	&&
	    szCDTRec[0] != 0x0D	&&
	    szCDTRec[0] != 0x0A	&&
	    szCDTRec[0] != 0x00)
        {
                memcpy(&srCDTRec.szHighBinRange[0], &szCDTRec[0], k - 1);
        }

        /* 04_對應交易主機索引 */
        /* 初始化 */
        memset(szCDTRec, 0x00, sizeof(szCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTRec[k ++] = uszReadData[i ++];
                if (szCDTRec[k - 1] == 0x2C	||
		    szCDTRec[k - 1] == 0x0D	||
		    szCDTRec[k - 1] == 0x0A	||
		    szCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CDT unpack ERROR");
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
        if (szCDTRec[0] != 0x2C	&&
	    szCDTRec[0] != 0x0D	&&
	    szCDTRec[0] != 0x0A	&&
	    szCDTRec[0] != 0x00)
        {
                memcpy(&srCDTRec.szHostCDTIndex[0], &szCDTRec[0], k - 1);
        }

        /* 05_檢查最短卡號長度 */
        /* 初始化 */
        memset(szCDTRec, 0x00, sizeof(szCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTRec[k ++] = uszReadData[i ++];
                if (szCDTRec[k - 1] == 0x2C	||
		    szCDTRec[k - 1] == 0x0D	||
		    szCDTRec[k - 1] == 0x0A	||
		    szCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CDT unpack ERROR");
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
        if (szCDTRec[0] != 0x2C	&&
	    szCDTRec[0] != 0x0D	&&
	    szCDTRec[0] != 0x0A	&&
	    szCDTRec[0] != 0x00)
        {
                memcpy(&srCDTRec.szMinPANLength[0], &szCDTRec[0], k - 1);
        }

        /* 06_檢查最長卡號長度 */
        /* 初始化 */
        memset(szCDTRec, 0x00, sizeof(szCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTRec[k ++] = uszReadData[i ++];
                if (szCDTRec[k - 1] == 0x2C	||
		    szCDTRec[k - 1] == 0x0D	||
		    szCDTRec[k - 1] == 0x0A	||
		    szCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CDT unpack ERROR");
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
        if (szCDTRec[0] != 0x2C	&&
	    szCDTRec[0] != 0x0D	&&
	    szCDTRec[0] != 0x0A	&&
	    szCDTRec[0] != 0x00)
        {
                memcpy(&srCDTRec.szMaxPANLength[0], &szCDTRec[0], k - 1);
        }

        /* 07_檢查碼查核(U CARD以11碼卡號，依U CARD檢查碼邏輯進行查核) */
        /* 初始化 */
        memset(szCDTRec, 0x00, sizeof(szCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTRec[k ++] = uszReadData[i ++];
                if (szCDTRec[k - 1] == 0x2C	||
		    szCDTRec[k - 1] == 0x0D	||
		    szCDTRec[k - 1] == 0x0A	||
		    szCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CDT unpack ERROR");
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
        if (szCDTRec[0] != 0x2C	&&
	    szCDTRec[0] != 0x0D	&&
	    szCDTRec[0] != 0x0A	&&
	    szCDTRec[0] != 0x00)
        {
                memcpy(&srCDTRec.szModule10Check[0], &szCDTRec[0], k - 1);
        }

        /* 08_有效期查核 */
        /* 初始化 */
        memset(szCDTRec, 0x00, sizeof(szCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTRec[k ++] = uszReadData[i ++];
                if (szCDTRec[k - 1] == 0x2C	||
		    szCDTRec[k - 1] == 0x0D	||
		    szCDTRec[k - 1] == 0x0A	||
		    szCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CDT unpack ERROR");
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
        if (szCDTRec[0] != 0x2C	&&
	    szCDTRec[0] != 0x0D	&&
	    szCDTRec[0] != 0x0A	&&
	    szCDTRec[0] != 0x00)
        {
                memcpy(&srCDTRec.szExpiredDateCheck[0], &szCDTRec[0], k - 1);
        }

        /* 09_輸入AMEX 4DBC或MASTER/VISA CVV2。 */
        /* 初始化 */
        memset(szCDTRec, 0x00, sizeof(szCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTRec[k ++] = uszReadData[i ++];
                if (szCDTRec[k - 1] == 0x2C	||
		    szCDTRec[k - 1] == 0x0D	||
		    szCDTRec[k - 1] == 0x0A	||
		    szCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CDT unpack ERROR");
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
        if (szCDTRec[0] != 0x2C	&&
	    szCDTRec[0] != 0x0D	&&
	    szCDTRec[0] != 0x0A	&&
	    szCDTRec[0] != 0x00)
        {
                memcpy(&srCDTRec.sz4DBCEnable[0], &szCDTRec[0], k - 1);
        }

	/* 10_卡別名稱(VISA, MASTERCARD, JCB, U CARD, AMEX, DINERS) */
        /* 初始化 */
        memset(szCDTRec, 0x00, sizeof(szCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTRec[k ++] = uszReadData[i ++];
                if (szCDTRec[k - 1] == 0x2C	||
		    szCDTRec[k - 1] == 0x0D	||
		    szCDTRec[k - 1] == 0x0A	||
		    szCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CDT unpack ERROR");
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
        if (szCDTRec[0] != 0x2C	&&
	    szCDTRec[0] != 0x0D	&&
	    szCDTRec[0] != 0x0A	&&
	    szCDTRec[0] != 0x00)
        {
                memcpy(&srCDTRec.szCardLabel[0], &szCDTRec[0], k - 1);
                inDISP_LogPrintf("Miyano flag inCDT szCardLabel = %s", srCDTRec.szCardLabel);
        }

#ifdef _TX_CHECK_NO_
        /* 11_列印交易編號及遮掩商店存根聯之卡號 */
        /* 初始化 */
        memset(szCDTRec, 0x00, sizeof(szCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTRec[k ++] = uszReadData[i ++];
                if (szCDTRec[k - 1] == 0x2C	||
		    szCDTRec[k - 1] == 0x0D	||
		    szCDTRec[k - 1] == 0x0A	||
		    szCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CDT unpack ERROR");
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
        if (szCDTRec[0] != 0x2C	&&
	    szCDTRec[0] != 0x0D	&&
	    szCDTRec[0] != 0x0A	&&
	    szCDTRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srCDTRec.szPrint_Tx_No_Check_No[0], &szCDTRec[0], k - 1);
        }
#endif
        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadCDTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadCDTRec(%d) END!!", inCDTRec);
                inDISP_LogPrintf(szErrorMsg);
                inDISP_LogPrintf("----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveCDTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveCDTRec(int inCDTRec)
{
	unsigned long   uldat_Handle;   			/* FILE Handle */
	int inRecCount = 0, inTempIndex = 0;
	int     	inRetVal;
	int     	i = 0;

	char szTempTitle[] = "inSaveCDTRec";
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
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inSaveCDTRec INIT" );
#endif
	
	inTempIndex = inCDTRec;

	
	memset(szTempFIleName, 0x00, sizeof(szTempFIleName));
	strcat( szTempFIleName, _CDT_FILE_NAME_);
	
	memset(szTempBakFileName, 0x00, sizeof(szTempBakFileName));
	strcat( szTempBakFileName, _CDT_FILE_NAME_BAK_);
	
	inDISP_LogPrintfWithFlag("  %s Index[%d] ", szTempTitle, inTempIndex);

      	/* 開啟原檔案HDPT.dat */
	inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_CDT_FILE_NAME_);

	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  %s *Error* [%d] ",szTempTitle, inRetVal);
		inFunc_EDCLock();
		return(VS_ERROR);
	}

	usRetVal = CTOS_FileGetSize((unsigned char *)_CDT_FILE_NAME_, &ulFileSize);	
	
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
	uszWriteBuff_Record = malloc(_SIZE_CDT_REC_ + _SIZE_CDT_COMMA_0D0A_);
	memset(uszWriteBuff_Record, 0x00, _SIZE_CDT_REC_ + _SIZE_CDT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */


	/* uszRead_Total_Buff儲存CDT.dat全部資料 */
	uszRead_Total_Buff = malloc(ulFileSize + 1);
	memset(uszRead_Total_Buff, 0x00, ulFileSize + 1);

	ulPackCount = 0; /* uszWriteBuff_Record的index位置 */
	/* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* CardIndex */
	memcpy(&uszWriteBuff_Record[0], &srCDTRec.szCardIndex[0], strlen(srCDTRec.szCardIndex));
	ulPackCount += strlen(srCDTRec.szCardIndex);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* LowBinRange */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCDTRec.szLowBinRange[0], strlen(srCDTRec.szLowBinRange));
	ulPackCount += strlen(srCDTRec.szLowBinRange);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* HighBinRange */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCDTRec.szHighBinRange[0], strlen(srCDTRec.szHighBinRange));
	ulPackCount += strlen(srCDTRec.szHighBinRange);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* HostCDTIndex */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCDTRec.szHostCDTIndex[0], strlen(srCDTRec.szHostCDTIndex));
	ulPackCount += strlen(srCDTRec.szHostCDTIndex);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* MinPANLength */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCDTRec.szMinPANLength[0], strlen(srCDTRec.szMinPANLength));
	ulPackCount += strlen(srCDTRec.szMinPANLength);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* MaxPANLength */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCDTRec.szMaxPANLength[0], strlen(srCDTRec.szMaxPANLength));
	ulPackCount += strlen(srCDTRec.szMaxPANLength);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* Module10Check */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCDTRec.szModule10Check[0], strlen(srCDTRec.szModule10Check));
	ulPackCount += strlen(srCDTRec.szModule10Check);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* ExpiredDateCheck */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCDTRec.szExpiredDateCheck[0], strlen(srCDTRec.szExpiredDateCheck));
	ulPackCount += strlen(srCDTRec.szExpiredDateCheck);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 4DBCEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCDTRec.sz4DBCEnable[0], strlen(srCDTRec.sz4DBCEnable));
	ulPackCount += strlen(srCDTRec.sz4DBCEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* CardLabel */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCDTRec.szCardLabel[0], strlen(srCDTRec.szCardLabel));
	ulPackCount += strlen(srCDTRec.szCardLabel);

#ifdef _TX_CHECK_NO_
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;
	
	/* Print_Tx_No_Check_No */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCDTRec.szPrint_Tx_No_Check_No[0], strlen(srCDTRec.szPrint_Tx_No_Check_No));
	ulPackCount += strlen(srCDTRec.szPrint_Tx_No_Check_No);
#endif
	
	
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
	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inSaveCDTRec END");
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
Function        :inGetCardIndex
Date&Time       :
Describe        :
*/
int inGetCardIndex(char* szCardIndex)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCardIndex == NULL || strlen(srCDTRec.szCardIndex) <= 0 || strlen(srCDTRec.szCardIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCardIndex() ERROR !!");

			if (szCardIndex == NULL)
                        {
                                inDISP_LogPrintf("szCardIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCardIndex length = (%d)", (int)strlen(srCDTRec.szCardIndex));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCardIndex[0], &srCDTRec.szCardIndex[0], strlen(srCDTRec.szCardIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetCardIndex
Date&Time       :
Describe        :
*/
int inSetCardIndex(char* szCardIndex)
{
        memset(srCDTRec.szCardIndex, 0x00, sizeof(srCDTRec.szCardIndex));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szCardIndex == NULL || strlen(szCardIndex) <= 0 || strlen(szCardIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCardIndex() ERROR !!");
                        if (szCardIndex == NULL)
                        {
                                inDISP_LogPrintf("szCardIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCardIndex length = (%d)", (int)strlen(szCardIndex));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCDTRec.szCardIndex[0], &szCardIndex[0], strlen(szCardIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetLowBinRange
Date&Time       :
Describe        :
*/
int inGetLowBinRange(char* szLowBinRange)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szLowBinRange == NULL || strlen(srCDTRec.szLowBinRange) <= 0 || strlen(srCDTRec.szLowBinRange) > 11)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetLowBinRange() ERROR !!");

                        if (szLowBinRange == NULL)
                        {
                                inDISP_LogPrintf("szLowBinRange == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szLowBinRange length = (%d)", (int)strlen(srCDTRec.szLowBinRange));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szLowBinRange[0], &srCDTRec.szLowBinRange[0], strlen(srCDTRec.szLowBinRange));

        return (VS_SUCCESS);
}

/*
Function        :inSetLowBinRange
Date&Time       :
Describe        :
*/
int inSetLowBinRange(char* szLowBinRange)
{
        memset(srCDTRec.szLowBinRange, 0x00, sizeof(srCDTRec.szLowBinRange));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szLowBinRange == NULL || strlen(szLowBinRange) <= 0 || strlen(szLowBinRange) > 11)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetLowBinRange() ERROR !!");

                        if (szLowBinRange == NULL)
                        {
                                inDISP_LogPrintf("szLowBinRange == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szLowBinRange length = (%d)", (int)strlen(szLowBinRange));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCDTRec.szLowBinRange[0], &szLowBinRange[0], strlen(szLowBinRange));

        return (VS_SUCCESS);
}

/*
Function        :inGetHighBinRange
Date&Time       :
Describe        :
*/
int inGetHighBinRange(char* szHighBinRange)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szHighBinRange == NULL || strlen(srCDTRec.szHighBinRange) <= 0 || strlen(srCDTRec.szHighBinRange) > 11)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetHighBinRange() ERROR !!");

                        if (szHighBinRange == NULL)
                        {
                                inDISP_LogPrintf("szHighBinRange == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHighBinRange length = (%d)", (int)strlen(srCDTRec.szHighBinRange));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szHighBinRange[0], &srCDTRec.szHighBinRange[0], strlen(srCDTRec.szHighBinRange));

        return (VS_SUCCESS);
}

/*
Function        :inSetHighBinRange
Date&Time       :
Describe        :
*/
int inSetHighBinRange(char* szHighBinRange)
{
        memset(srCDTRec.szHighBinRange, 0x00, sizeof(srCDTRec.szHighBinRange));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szHighBinRange == NULL || strlen(szHighBinRange) <= 0 || strlen(szHighBinRange) > 11)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetHighBinRange() ERROR !!");

                        if (szHighBinRange == NULL)
                        {
                                inDISP_LogPrintf("szHighBinRange == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHighBinRange length = (%d)", (int)strlen(szHighBinRange));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCDTRec.szHighBinRange[0], &szHighBinRange[0], strlen(szHighBinRange));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostCDTIndex
Date&Time       :
Describe        :
*/
int inGetHostCDTIndex(char* szHostCDTIndex)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szHostCDTIndex == NULL || strlen(srCDTRec.szHostCDTIndex) <= 0 || strlen(srCDTRec.szHostCDTIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetHostCDTIndex() ERROR !!");

                        if (szHostCDTIndex == NULL)
                        {
                                inDISP_LogPrintf("szHostCDTIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHostCDTIndex length = (%d)", (int)strlen(srCDTRec.szHostCDTIndex));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szHostCDTIndex[0], &srCDTRec.szHostCDTIndex[0], strlen(srCDTRec.szHostCDTIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostCDTIndex
Date&Time       :
Describe        :
*/
int inSetHostCDTIndex(char* szHostCDTIndex)
{
        memset(srCDTRec.szHostCDTIndex, 0x00, sizeof(srCDTRec.szHostCDTIndex));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szHostCDTIndex == NULL || strlen(szHostCDTIndex) <= 0 || strlen(szHostCDTIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetHostCDTIndex() ERROR !!");

                        if (szHostCDTIndex == NULL)
                        {
                                inDISP_LogPrintf("szHostCDTIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHostCDTIndex length = (%d)", (int)strlen(szHostCDTIndex));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCDTRec.szHostCDTIndex[0], &szHostCDTIndex[0], strlen(szHostCDTIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetMinPANLength
Date&Time       :
Describe        :
*/
int inGetMinPANLength(char* szMinPANLength)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szMinPANLength == NULL || strlen(srCDTRec.szMinPANLength) <= 0 || strlen(srCDTRec.szMinPANLength) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetMinPANLength() ERROR !!");

                        if (szMinPANLength == NULL)
                        {
                                inDISP_LogPrintf("szMinPANLength == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMinPANLength length = (%d)", (int)strlen(srCDTRec.szMinPANLength));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMinPANLength[0], &srCDTRec.szMinPANLength[0], strlen(srCDTRec.szMinPANLength));

        return (VS_SUCCESS);
}

/*
Function        :inSetMinPANLength
Date&Time       :
Describe        :
*/
int inSetMinPANLength(char* szMinPANLength)
{
        memset(srCDTRec.szMinPANLength, 0x00, sizeof(srCDTRec.szMinPANLength));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szMinPANLength == NULL || strlen(szMinPANLength) <= 0 || strlen(szMinPANLength) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetMinPANLength() ERROR !!");

                        if (szMinPANLength == NULL)
                        {
                                inDISP_LogPrintf("szMinPANLength == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMinPANLength length = (%d)", (int)strlen(szMinPANLength));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCDTRec.szMinPANLength[0], &szMinPANLength[0], strlen(szMinPANLength));

        return (VS_SUCCESS);
}

/*
Function        :inGetMaxPANLength
Date&Time       :
Describe        :
*/
int inGetMaxPANLength(char* szMaxPANLength)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szMaxPANLength == NULL || strlen(srCDTRec.szMaxPANLength) <= 0 || strlen(srCDTRec.szMaxPANLength) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetMaxPANLength() ERROR !!");

                        if (szMaxPANLength == NULL)
                        {
                                inDISP_LogPrintf("szMaxPANLength == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMaxPANLength length = (%d)", (int)strlen(srCDTRec.szMaxPANLength));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMaxPANLength[0], &srCDTRec.szMaxPANLength[0], strlen(srCDTRec.szMaxPANLength));

        return (VS_SUCCESS);
}

/*
Function        :inSetMaxPANLength
Date&Time       :
Describe        :
*/
int inSetMaxPANLength(char* szMaxPANLength)
{
        memset(srCDTRec.szMaxPANLength, 0x00, sizeof(srCDTRec.szMaxPANLength));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szMaxPANLength == NULL || strlen(szMaxPANLength) <= 0 || strlen(szMaxPANLength) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetMaxPANLength() ERROR !!");

                        if (szMaxPANLength == NULL)
                        {
                                inDISP_LogPrintf("szMaxPANLength == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMaxPANLength length = (%d)", (int)strlen(szMaxPANLength));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCDTRec.szMaxPANLength[0], &szMaxPANLength[0], strlen(szMaxPANLength));

        return (VS_SUCCESS);
}

/*
Function        :inGetModule10Check
Date&Time       :
Describe        :
*/
int inGetModule10Check(char* szModule10Check)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szModule10Check == NULL || strlen(srCDTRec.szModule10Check) <= 0 || strlen(srCDTRec.szModule10Check) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetModule10Check() ERROR !!");

                        if (szModule10Check == NULL)
                        {
                                inDISP_LogPrintf("szModule10Check == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szModule10Check length = (%d)", (int)strlen(srCDTRec.szModule10Check));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szModule10Check[0], &srCDTRec.szModule10Check[0], strlen(srCDTRec.szModule10Check));

        return (VS_SUCCESS);
}

/*
Function        :inSetModule10Check
Date&Time       :
Describe        :
*/
int inSetModule10Check(char* szModule10Check)
{
        memset(srCDTRec.szModule10Check, 0x00, sizeof(srCDTRec.szModule10Check));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szModule10Check == NULL || strlen(szModule10Check) <= 0 || strlen(szModule10Check) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetModule10Check() ERROR !!");

                        if (szModule10Check == NULL)
                        {
                                inDISP_LogPrintf("szModule10Check == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szModule10Check length = (%d)", (int)strlen(szModule10Check));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCDTRec.szModule10Check[0], &szModule10Check[0], strlen(szModule10Check));

        return (VS_SUCCESS);
}

/*
Function        :inGetExpiredDateCheck
Date&Time       :
Describe        :
*/
int inGetExpiredDateCheck(char* szExpiredDateCheck)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szExpiredDateCheck == NULL || strlen(srCDTRec.szExpiredDateCheck) <= 0 || strlen(srCDTRec.szExpiredDateCheck) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetExpiredDateCheck() ERROR !!");

                        if (szExpiredDateCheck == NULL)
                        {
                                inDISP_LogPrintf("szExpiredDateCheck == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szExpiredDateCheck length = (%d)", (int)strlen(srCDTRec.szExpiredDateCheck));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szExpiredDateCheck[0], &srCDTRec.szExpiredDateCheck[0], strlen(srCDTRec.szExpiredDateCheck));

        return (VS_SUCCESS);
}

/*
Function        :inSetExpiredDateCheck
Date&Time       :
Describe        :
*/
int inSetExpiredDateCheck(char* szExpiredDateCheck)
{
        memset(srCDTRec.szExpiredDateCheck, 0x00, sizeof(srCDTRec.szExpiredDateCheck));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szExpiredDateCheck == NULL || strlen(szExpiredDateCheck) <= 0 || strlen(szExpiredDateCheck) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetExpiredDateCheck() ERROR !!");

                        if (szExpiredDateCheck == NULL)
                        {
                                inDISP_LogPrintf("szExpiredDateCheck == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szExpiredDateCheck length = (%d)", (int)strlen(szExpiredDateCheck));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCDTRec.szExpiredDateCheck[0], &szExpiredDateCheck[0], strlen(szExpiredDateCheck));

        return (VS_SUCCESS);
}

/*
Function        :inGet4DBCEnable
Date&Time       :
Describe        :
*/
int inGet4DBCEnable(char* sz4DBCEnable)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (sz4DBCEnable == NULL || strlen(srCDTRec.sz4DBCEnable) <= 0 || strlen(srCDTRec.sz4DBCEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGet4DBCEnable() ERROR !!");

                        if (sz4DBCEnable == NULL)
                        {
                                inDISP_LogPrintf("sz4DBCEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "sz4DBCEnable length = (%d)", (int)strlen(srCDTRec.sz4DBCEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&sz4DBCEnable[0], &srCDTRec.sz4DBCEnable[0], strlen(srCDTRec.sz4DBCEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetCVV4DBCEnable
Date&Time       :
Describe        :
*/
int inSet4DBCEnable(char* sz4DBCEnable)
{
        memset(srCDTRec.sz4DBCEnable, 0x00, sizeof(srCDTRec.sz4DBCEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (sz4DBCEnable == NULL || strlen(sz4DBCEnable) <= 0 || strlen(sz4DBCEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSet4DBCEnable() ERROR !!");

                        if (sz4DBCEnable == NULL)
                        {
                                inDISP_LogPrintf("sz4DBCEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "sz4DBCEnable length = (%d)", (int)strlen(sz4DBCEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCDTRec.sz4DBCEnable[0], &sz4DBCEnable[0], strlen(sz4DBCEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetCardLabel
Date&Time       :2016/11/25 下午 1:21
Describe        :卡別
*/
int inGetCardLabel(char* szCardLabel)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCardLabel == NULL || strlen(srCDTRec.szCardLabel) <= 0 || strlen(srCDTRec.szCardLabel) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCardLabel() ERROR !!");

                        if (szCardLabel == NULL)
                        {
                                inDISP_LogPrintf("szCardLabel == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCardLabel length = (%d)", (int)strlen(srCDTRec.szCardLabel));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCardLabel[0], &srCDTRec.szCardLabel[0], strlen(srCDTRec.szCardLabel));

        return (VS_SUCCESS);
}

/*
Function        :inSetCardLabel
Date&Time       :2016/11/25 下午 1:22
Describe        :卡別
*/
int inSetCardLabel(char* szCardLabel)
{
        memset(srCDTRec.szCardLabel, 0x00, sizeof(srCDTRec.szCardLabel));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCardLabel == NULL || strlen(szCardLabel) <= 0 || strlen(szCardLabel) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCardLabel() ERROR !!");

                        if (szCardLabel == NULL)
                        {
                                inDISP_LogPrintf("szCardLabel == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCardLabel length = (%d)", (int)strlen(szCardLabel));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCDTRec.szCardLabel[0], &szCardLabel[0], strlen(szCardLabel));

        return (VS_SUCCESS);
}

#ifdef _TX_CHECK_NO_
/*
Function        :inGetPrint_Tx_No_Check_No
Date&Time       :2017/5/15 下午 4:02
Describe        :卡別
*/
int inGetPrint_Tx_No_Check_No(char* szPrint_Tx_No_Check_No)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szPrint_Tx_No_Check_No == NULL || strlen(srCDTRec.szPrint_Tx_No_Check_No) <= 0 || strlen(srCDTRec.szPrint_Tx_No_Check_No) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPrint_Tx_No_Check_No() ERROR !!");

                        if (szPrint_Tx_No_Check_No == NULL)
                        {
                                inDISP_LogPrintf("szPrint_Tx_No_Check_No == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrint_Tx_No_Check_No length = (%d)", (int)strlen(srCDTRec.szPrint_Tx_No_Check_No));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPrint_Tx_No_Check_No[0], &srCDTRec.szPrint_Tx_No_Check_No[0], strlen(srCDTRec.szPrint_Tx_No_Check_No));

        return (VS_SUCCESS);
}


/*
Function        :inSetPrint_Tx_No_Check_No
Date&Time       :2017/5/15 下午 4:02
Describe        :卡別
*/
int inSetPrint_Tx_No_Check_No(char* szPrint_Tx_No_Check_No)
{
        memset(srCDTRec.szPrint_Tx_No_Check_No, 0x00, sizeof(srCDTRec.szPrint_Tx_No_Check_No));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPrint_Tx_No_Check_No == NULL || strlen(szPrint_Tx_No_Check_No) <= 0 || strlen(szPrint_Tx_No_Check_No) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPrint_Tx_No_Check_No() ERROR !!");

                        if (szPrint_Tx_No_Check_No == NULL)
                        {
                                inDISP_LogPrintf("szPrint_Tx_No_Check_No == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrint_Tx_No_Check_No length = (%d)", (int)strlen(szPrint_Tx_No_Check_No));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCDTRec.szPrint_Tx_No_Check_No[0], &szPrint_Tx_No_Check_No[0], strlen(szPrint_Tx_No_Check_No));

        return (VS_SUCCESS);
}
#endif
/*
Function        :inCDT_Edit_CDT_Table
Date&Time       :2017/5/15 下午 4:08
Describe        :
*/
int inCDT_Edit_CDT_Table(void)
{
	TABLE_GET_SET_TABLE CDT_FUNC_TABLE[] =
	{
		{"szCardIndex"			,inGetCardIndex			,inSetCardIndex			},
		{"szLowBinRange"		,inGetLowBinRange		,inSetLowBinRange		},
		{"szHighBinRange"		,inGetHighBinRange		,inSetHighBinRange		},
		{"szHostCDTIndex"		,inGetHostCDTIndex		,inSetHostCDTIndex		},
		{"szMinPANLength"		,inGetMinPANLength		,inSetMinPANLength		},
		{"szMaxPANLength"		,inGetMaxPANLength		,inSetMaxPANLength		},
		{"szModule10Check"		,inGetModule10Check		,inSetModule10Check		},
		{"szExpiredDateCheck"	,inGetExpiredDateCheck		,inSetExpiredDateCheck		},
		{"sz4DBCEnable"			,inGet4DBCEnable		,inSet4DBCEnable		},
		{"szCardLabel"			,inGetCardLabel			,inSetCardLabel			},
#ifdef _TX_CHECK_NO_		
		{"szPrint_Tx_No_Check_No"	,inGetPrint_Tx_No_Check_No	,inSetPrint_Tx_No_Check_No	},
#endif
		{""},
	};
	int		inRetVal;
	int		inRecordCnt = 0;
	char		szKey;
	DISPLAY_OBJECT  srDispObj;
	
	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	srDispObj.inY = _LINE_8_8_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 2;
	srDispObj.inColor = _COLOR_BLACK_;
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont_Color("是否更改CDT", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
	inDISP_ChineseFont_Color("列印請按0 更改請按Enter", _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);
	inDISP_ChineseFont_Color("跳過請按取消鍵", _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);
	while (1)
	{
		szKey = uszKBD_GetKey(_EDC_TIMEOUT_);
		if (szKey == _KEY_0_)
		{
			inRetVal = VS_SUCCESS;
			/* 這裡放列印的Function */
			return	(inRetVal);
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
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont_Color("請輸入Record Index?", _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);
			while (1)
			{
				memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
				srDispObj.inOutputLen = 0;
	
				inRetVal = inDISP_Enter8x16(&srDispObj);
				if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				{
					return (inRetVal);
				}
				else if (srDispObj.inOutputLen > 0)
				{
					inRecordCnt = atoi(srDispObj.szOutput);
					break;
				}
			}

			break;
		}
		
	}
	inLoadCDTRec(inRecordCnt);
	
	inFunc_Edit_Table_Tag(CDT_FUNC_TABLE);
	inSaveCDTRec(inRecordCnt);
	
	return	(VS_SUCCESS);
}

int inCDT_Test (void)
{
	char    CDT_Data[1024];
	//int     i = 0;
	memset(CDT_Data, 0x00, sizeof(CDT_Data));

	inLoadCDTRec(0);
	inSetCardIndex("01");
	inSetLowBinRange("4000000000");
	inSetHighBinRange("4000999999");
	inSetHostCDTIndex("01");
	inSetMinPANLength("11");
	inSetMaxPANLength("16");
	inSetModule10Check("Y");
	inSetExpiredDateCheck("Y");
	inSet4DBCEnable("N");
	inSetCardLabel("U CARD");
	inSaveCDTRec(0);


	inLoadCDTRec(1);
	inSetCardIndex("02");
	inSetLowBinRange("4000000000");
	inSetHighBinRange("4999999999");
	inSetHostCDTIndex("01");
	inSetMinPANLength("01");
	inSetMaxPANLength("19");
	inSetModule10Check("Y");
	inSetExpiredDateCheck("Y");
	inSet4DBCEnable("N");
	inSetCardLabel("VISA");
	inSaveCDTRec(1);

	inLoadCDTRec(2);
	inSetCardIndex("03");
	inSetLowBinRange("5100000000");
	inSetHighBinRange("5599999999");
	inSetHostCDTIndex("01");
	inSetMinPANLength("13");
	inSetMaxPANLength("19");
	inSetModule10Check("Y");
	inSetExpiredDateCheck("Y");
	inSet4DBCEnable("N");
	inSetCardLabel("VISA");
	inSaveCDTRec(2);

	inLoadCDTRec(3);
	inSetCardIndex("04");
	inSetLowBinRange("3528000000");
	inSetHighBinRange("3589999999");
	inSetHostCDTIndex("01");
	inSetMinPANLength("13");
	inSetMaxPANLength("19");
	inSetModule10Check("Y");
	inSetExpiredDateCheck("Y");
	inSet4DBCEnable("N");
	inSetCardLabel("JCB");
	inSaveCDTRec(3);

	inLoadCDTRec(4);
	inSetCardIndex("05");
	inSetLowBinRange("1040000000");
	inSetHighBinRange("1040999999");
	inSetHostCDTIndex("02");
	inSetMinPANLength("04");
	inSetMaxPANLength("14");
	inSetModule10Check("Y");
	inSetExpiredDateCheck("Y");
	inSet4DBCEnable("N");
	inSetCardLabel("DINERS");
	inSaveCDTRec(4);

	inLoadCDTRec(5);
	inSetCardIndex("06");
	inSetLowBinRange("3400000000");
	inSetHighBinRange("3499999999");
	inSetHostCDTIndex("01");
	inSetMinPANLength("15");
	inSetMaxPANLength("15");
	inSetModule10Check("Y");
	inSetExpiredDateCheck("Y");
	inSet4DBCEnable("Y");
	inSetCardLabel("AMEX");
	inSaveCDTRec(5);

	inLoadCDTRec(6);
	inSetCardIndex("07");
	inSetLowBinRange("3700000000");
	inSetHighBinRange("3799999999");
	inSetHostCDTIndex("01");
	inSetMinPANLength("15");
	inSetMaxPANLength("15");
	inSetModule10Check("Y");
	inSetExpiredDateCheck("Y");
	inSet4DBCEnable("Y");
	inSetCardLabel("AMEX");
	inSaveCDTRec(6);

	inLoadCDTRec(7);
	inSetCardIndex("08");
	inSetLowBinRange("9999999999");
	inSetHighBinRange("9999999999");
	inSetHostCDTIndex("01");
	inSetMinPANLength("01");
	inSetMaxPANLength("19");
	inSetModule10Check("Y");
	inSetExpiredDateCheck("Y");
	inSet4DBCEnable("N");
	inSetCardLabel("VISA");
	inSaveCDTRec(6);


	return 0;
}

int inSetCdtRealDataCount( int inRealCount)
{
	inRealDataCount = inRealCount;
	return inRealDataCount;
}

