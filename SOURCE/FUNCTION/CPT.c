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
#include "CPT.h"

static  CPT_REC srCPTRec;	/* construct CPT record */
extern  int     ginDebug;  	/* Debug使用 extern */

/*
Function        :inLoadCPTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀CPT檔案，inCPT_Rec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadCPTRec(int inCPTRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆CPT檔案 */
        char            szCPTRec[_SIZE_CPT_REC_ + 1];           /* 暫存, 放各個欄位檔案 */
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnCPTLength = 0;                        /* CPT總長度 */
        long            lnReadLength;                           /* 記錄每次要從CPT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;                 /* inRec記錄讀到第幾筆, i為目前從CPT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadCPTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadCPTRec(%d) START!!", inCPTRec);
                inDISP_LogPrintf(szErrorMsg);
        }

        /* 判斷傳進來的inCPTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inCPTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inCPTRec < 0:(index = %d) ERROR!!", inCPTRec);
                        inDISP_LogPrintf(szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open CPT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_CPT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnCPTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_CPT_FILE_NAME_);

	if (lnCPTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnCPTLength + 1);
        uszTemp = malloc(lnCPTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnCPTLength + 1);
        memset(uszTemp, 0x00, lnCPTLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnCPTLength;

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
         *i為目前從CPT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnCPTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到CPT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

			/* 清空uszReadData */
                        memset(uszReadData, 0x00, lnCPTLength + 1);
			/* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
			/* 因為inCPT_Rec的index從0開始，所以inCPT_Rec要+1 */
                        if (inRec == (inCPTRec + 1))
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
         * 如果沒有inCPTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inCPTRec + 1) || inSearchResult == -1)
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
        memset(&srCPTRec, 0x00, sizeof(srCPTRec));
        /*
         * 以下pattern為存入CPT_Rec
         * i為CPT的第幾個字元
         * 存入CPT_Rec
         */
        i = 0;


        /* 01_通訊參數索引 */
	/* 初始化 */
        memset(szCPTRec, 0x00, sizeof(szCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCPTRec[k ++] = uszReadData[i ++];
                if (szCPTRec[k - 1] == 0x2C	||
		    szCPTRec[k - 1] == 0x0D	||
		    szCPTRec[k - 1] == 0x0A	||
		    szCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CPT unpack ERROR");
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
        if (szCPTRec[0] != 0x2C	&&
	    szCPTRec[0] != 0x0D	&&
	    szCPTRec[0] != 0x0A	&&
	    szCPTRec[0] != 0x00)
        {
                memcpy(&srCPTRec.szCommIndex[0], &szCPTRec[0], k - 1);
        }

        /* 02_TPDU */
	/* 初始化 */
        memset(szCPTRec, 0x00, sizeof(szCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCPTRec[k ++] = uszReadData[i ++];
                if (szCPTRec[k - 1] == 0x2C	||
		    szCPTRec[k - 1] == 0x0D	||
		    szCPTRec[k - 1] == 0x0A	||
		    szCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CPT unpack ERROR");
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
        if (szCPTRec[0] != 0x2C	&&
	    szCPTRec[0] != 0x0D	&&
	    szCPTRec[0] != 0x0A	&&
	    szCPTRec[0] != 0x00)
        {
                memcpy(&srCPTRec.szTPDU[0], &szCPTRec[0], k - 1);
        }

        /* 03_網路識別碼 */
	/* 初始化 */
        memset(szCPTRec, 0x00, sizeof(szCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCPTRec[k ++] = uszReadData[i ++];
                if (szCPTRec[k - 1] == 0x2C	||
		    szCPTRec[k - 1] == 0x0D	||
		    szCPTRec[k - 1] == 0x0A	||
		    szCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CPT unpack ERROR");
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
        if (szCPTRec[0] != 0x2C	&&
	    szCPTRec[0] != 0x0D	&&
	    szCPTRec[0] != 0x0A	&&
	    szCPTRec[0] != 0x00)
        {
                memcpy(&srCPTRec.szNII[0], &szCPTRec[0], k - 1);
        }

        /* 04_第一授權撥接電話 */
	/* 初始化 */
        memset(szCPTRec, 0x00, sizeof(szCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCPTRec[k ++] = uszReadData[i ++];
                if (szCPTRec[k - 1] == 0x2C	||
		    szCPTRec[k - 1] == 0x0D	||
		    szCPTRec[k - 1] == 0x0A	||
		    szCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CPT unpack ERROR");
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
        if (szCPTRec[0] != 0x2C	&&
	    szCPTRec[0] != 0x0D	&&
	    szCPTRec[0] != 0x0A	&&
	    szCPTRec[0] != 0x00)
        {
                memcpy(&srCPTRec.szHostTelPrimary[0], &szCPTRec[0], k - 1);
        }

        /* 05_第二授權撥接電話 */
	/* 初始化 */
        memset(szCPTRec, 0x00, sizeof(szCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCPTRec[k ++] = uszReadData[i ++];
                if (szCPTRec[k - 1] == 0x2C	||
		    szCPTRec[k - 1] == 0x0D	||
		    szCPTRec[k - 1] == 0x0A	||
		    szCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CPT unpack ERROR");
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
        if (szCPTRec[0] != 0x2C	&&
	    szCPTRec[0] != 0x0D	&&
	    szCPTRec[0] != 0x0A	&&
	    szCPTRec[0] != 0x00)
        {
                memcpy(&srCPTRec.szHostTelSecond[0], &szCPTRec[0], k - 1);
        }

        /* 06_Call Bank 撥接電話 */
	/* 初始化 */
        memset(szCPTRec, 0x00, sizeof(szCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCPTRec[k ++] = uszReadData[i ++];
                if (szCPTRec[k - 1] == 0x2C	||
		    szCPTRec[k - 1] == 0x0D	||
		    szCPTRec[k - 1] == 0x0A	||
		    szCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CPT unpack ERROR");
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
        if (szCPTRec[0] != 0x2C	&&
	    szCPTRec[0] != 0x0D	&&
	    szCPTRec[0] != 0x0A	&&
	    szCPTRec[0] != 0x00)
        {
                memcpy(&srCPTRec.szReferralTel[0], &szCPTRec[0], k - 1);
        }

        /* 07_第一授權主機 IP Address */
	/* 初始化 */
        memset(szCPTRec, 0x00, sizeof(szCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCPTRec[k ++] = uszReadData[i ++];
                if (szCPTRec[k - 1] == 0x2C	||
		    szCPTRec[k - 1] == 0x0D	||
		    szCPTRec[k - 1] == 0x0A	||
		    szCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CPT unpack ERROR");
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
        if (szCPTRec[0] != 0x2C	&&
	    szCPTRec[0] != 0x0D	&&
	    szCPTRec[0] != 0x0A	&&
	    szCPTRec[0] != 0x00)
        {
                memcpy(&srCPTRec.szHostIPPrimary[0], &szCPTRec[0], k - 1);
        }

        /* 08_第一授權主機 Port No. */
	/* 初始化 */
        memset(szCPTRec, 0x00, sizeof(szCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCPTRec[k ++] = uszReadData[i ++];
                if (szCPTRec[k - 1] == 0x2C	||
		    szCPTRec[k - 1] == 0x0D	||
		    szCPTRec[k - 1] == 0x0A	||
		    szCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CPT unpack ERROR");
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
        if (szCPTRec[0] != 0x2C	&&
	    szCPTRec[0] != 0x0D	&&
	    szCPTRec[0] != 0x0A	&&
	    szCPTRec[0] != 0x00)
        {
                memcpy(&srCPTRec.szHostPortNoPrimary[0], &szCPTRec[0], k - 1);
        }

        /* 09_第二授權主機 IP Address */
	/* 初始化 */
        memset(szCPTRec, 0x00, sizeof(szCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCPTRec[k ++] = uszReadData[i ++];
                if (szCPTRec[k - 1] == 0x2C	||
		    szCPTRec[k - 1] == 0x0D	||
		    szCPTRec[k - 1] == 0x0A	||
		    szCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CPT unpack ERROR");
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
        if (szCPTRec[0] != 0x2C	&&
	    szCPTRec[0] != 0x0D	&&
	    szCPTRec[0] != 0x0A	&&
	    szCPTRec[0] != 0x00)
        {
                memcpy(&srCPTRec.szHostIPSecond[0], &szCPTRec[0], k - 1);
        }

        /* 10_第二授權主機 Port No. */
	/* 初始化 */
        memset(szCPTRec, 0x00, sizeof(szCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCPTRec[k ++] = uszReadData[i ++];
                if (szCPTRec[k - 1] == 0x2C	||
		    szCPTRec[k - 1] == 0x0D	||
		    szCPTRec[k - 1] == 0x0A	||
		    szCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CPT unpack ERROR");
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
        if (szCPTRec[0] != 0x2C	&&
	    szCPTRec[0] != 0x0D	&&
	    szCPTRec[0] != 0x0A	&&
	    szCPTRec[0] != 0x00)
        {
                memcpy(&srCPTRec.szHostPortNoSecond[0], &szCPTRec[0], k - 1);
        }

        /* 11_TCP 電文長度之格式 */
	/* 初始化 */
        memset(szCPTRec, 0x00, sizeof(szCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCPTRec[k ++] = uszReadData[i ++];
                if (szCPTRec[k - 1] == 0x2C	||
		    szCPTRec[k - 1] == 0x0D	||
		    szCPTRec[k - 1] == 0x0A	||
		    szCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CPT unpack ERROR");
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
        if (szCPTRec[0] != 0x2C	&&
	    szCPTRec[0] != 0x0D	&&
	    szCPTRec[0] != 0x0A	&&
	    szCPTRec[0] != 0x00)
        {
                memcpy(&srCPTRec.szTCPHeadFormat[0], &szCPTRec[0], k - 1);
        }

        /* 12_連線等候時間 */
	/* 初始化 */
        memset(szCPTRec, 0x00, sizeof(szCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCPTRec[k ++] = uszReadData[i ++];
                if (szCPTRec[k - 1] == 0x2C	||
		    szCPTRec[k - 1] == 0x0D	||
		    szCPTRec[k - 1] == 0x0A	||
		    szCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CPT unpack ERROR");
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
        if (szCPTRec[0] != 0x2C	&&
	    szCPTRec[0] != 0x0D	&&
	    szCPTRec[0] != 0x0A	&&
	    szCPTRec[0] != 0x00)
        {
                memcpy(&srCPTRec.szCarrierTimeOut[0], &szCPTRec[0], k - 1);
        }

        /* 13_授權等候時間 */
	/* 初始化 */
        memset(szCPTRec, 0x00, sizeof(szCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCPTRec[k ++] = uszReadData[i ++];
                if (szCPTRec[k - 1] == 0x2C	||
		    szCPTRec[k - 1] == 0x0D	||
		    szCPTRec[k - 1] == 0x0A	||
		    szCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CPT unpack ERROR");
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
        if (szCPTRec[0] != 0x2C	&&
	    szCPTRec[0] != 0x0D	&&
	    szCPTRec[0] != 0x0A	&&
	    szCPTRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srCPTRec.szHostResponseTimeOut[0], &szCPTRec[0], k - 1);
        }

	/* 14_第一結帳撥接電話*/
	/* 初始化 */
        memset(szCPTRec, 0x00, sizeof(szCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCPTRec[k ++] = uszReadData[i ++];
                if (szCPTRec[k - 1] == 0x2C	||
		    szCPTRec[k - 1] == 0x0D	||
		    szCPTRec[k - 1] == 0x0A	||
		    szCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CPT unpack Settle Tel 1 ERROR");
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
        if (szCPTRec[0] != 0x2C	&&
	    szCPTRec[0] != 0x0D	&&
	    szCPTRec[0] != 0x0A	&&
	    szCPTRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srCPTRec.szSettleTelPrimary[0], &szCPTRec[0], k - 1);
        }

	/* 13_第二結帳撥接電話 */
	/* 初始化 */
        memset(szCPTRec, 0x00, sizeof(szCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCPTRec[k ++] = uszReadData[i ++];
                if (szCPTRec[k - 1] == 0x2C	||
		    szCPTRec[k - 1] == 0x0D	||
		    szCPTRec[k - 1] == 0x0A	||
		    szCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CPT unpack Settle Tel 2 ERROR");
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
        if (szCPTRec[0] != 0x2C	&&
	    szCPTRec[0] != 0x0D	&&
	    szCPTRec[0] != 0x0A	&&
	    szCPTRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srCPTRec.szSettleTelSecond[0], &szCPTRec[0], k - 1);
        }		
		
        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

	/* inLoadCPTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadCPTRec(%d) END!!", inCPTRec);
                inDISP_LogPrintf(szErrorMsg);
		inDISP_LogPrintf("----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveCPTRec
Date&Time       :
Describe        :
*/
int inSaveCPTRec(int inCPTRec)
{
	unsigned long   uldat_Handle; /* FILE Handle */
	int inRecCount = 0, inTempIndex = 0;
	int     	inRetVal;
	int     	i = 0;

	char szTempTitle[] = "inSaveCPTRec";
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
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inSaveCFGTRec INIT" );
#endif	
	
	inTempIndex = inCPTRec;
	
	
	memset(szTempFIleName, 0x00, sizeof(szTempFIleName));
	strcat( szTempFIleName, _CPT_FILE_NAME_);
	
	memset(szTempBakFileName, 0x00, sizeof(szTempBakFileName));
	strcat( szTempBakFileName, _CPT_FILE_NAME_BAK_);
	
	inDISP_LogPrintfWithFlag("  %s Index[%d] ", szTempTitle, inTempIndex);
	
      
	/* 開啟原檔案HDPT.dat */
	inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_CPT_FILE_NAME_);

	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  %s *Error* [%d] ",szTempTitle, inRetVal);
		inFunc_EDCLock();
		return(VS_ERROR);
	}

	usRetVal = CTOS_FileGetSize((unsigned char *)_CPT_FILE_NAME_, &ulFileSize);	
	
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
	uszWriteBuff_Record = malloc(_SIZE_CPT_REC_ + _SIZE_CPT_COMMA_0D0A_);
	memset(uszWriteBuff_Record, 0x00, _SIZE_CPT_REC_ + _SIZE_CPT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */


	/* uszRead_Total_Buff儲存CPT.dat全部資料 */
	uszRead_Total_Buff = malloc(ulFileSize + 1);
	memset(uszRead_Total_Buff, 0x00, ulFileSize + 1);

	ulPackCount = 0; /* uszWriteBuff_Record的index位置 */
	/* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* CommIndex */
	memcpy(&uszWriteBuff_Record[0], &srCPTRec.szCommIndex[0], strlen(srCPTRec.szCommIndex));
	ulPackCount += strlen(srCPTRec.szCommIndex);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TPDU */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCPTRec.szTPDU[0], strlen(srCPTRec.szTPDU));
	ulPackCount += strlen(srCPTRec.szTPDU);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* NII */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCPTRec.szNII[0], strlen(srCPTRec.szNII));
	ulPackCount += strlen(srCPTRec.szNII);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* HostTelPrimary */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCPTRec.szHostTelPrimary[0], strlen(srCPTRec.szHostTelPrimary));
	ulPackCount += strlen(srCPTRec.szHostTelPrimary);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* HostTelSecond */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCPTRec.szHostTelSecond[0], strlen(srCPTRec.szHostTelSecond));
	ulPackCount += strlen(srCPTRec.szHostTelSecond);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* ReferralTel */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCPTRec.szReferralTel[0], strlen(srCPTRec.szReferralTel));
	ulPackCount += strlen(srCPTRec.szReferralTel);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* HostIPPrimary */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCPTRec.szHostIPPrimary[0], strlen(srCPTRec.szHostIPPrimary));
	ulPackCount += strlen(srCPTRec.szHostIPPrimary);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* HostPortNoPrimary */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCPTRec.szHostPortNoPrimary[0], strlen(srCPTRec.szHostPortNoPrimary));
	ulPackCount += strlen(srCPTRec.szHostPortNoPrimary);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* HostIPSecond */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCPTRec.szHostIPSecond[0], strlen(srCPTRec.szHostIPSecond));
	ulPackCount += strlen(srCPTRec.szHostIPSecond);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* HostPortNoSecond */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCPTRec.szHostPortNoSecond[0], strlen(srCPTRec.szHostPortNoSecond));
	ulPackCount += strlen(srCPTRec.szHostPortNoSecond);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TCPHeadFormat */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCPTRec.szTCPHeadFormat[0], strlen(srCPTRec.szTCPHeadFormat));
	ulPackCount += strlen(srCPTRec.szTCPHeadFormat);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* CarrierTimeOut */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCPTRec.szCarrierTimeOut[0], strlen(srCPTRec.szCarrierTimeOut));
	ulPackCount += strlen(srCPTRec.szCarrierTimeOut);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* HostResponseTimeOut */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCPTRec.szHostResponseTimeOut[0], strlen(srCPTRec.szHostResponseTimeOut));
	ulPackCount += strlen(srCPTRec.szHostResponseTimeOut);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* SettleTelPrimary */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCPTRec.szSettleTelPrimary[0], strlen(srCPTRec.szSettleTelPrimary));
	ulPackCount += strlen(srCPTRec.szSettleTelPrimary);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* SettleTelSecond */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCPTRec.szSettleTelSecond[0], strlen(srCPTRec.szSettleTelSecond));
	ulPackCount += strlen(srCPTRec.szSettleTelSecond);

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
	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inSaveCPTRec END");
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
Function        :inGetCommIndex
Date&Time       :
Describe        :
*/
int inGetCommIndex(char* szCommIndex)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCommIndex == NULL || strlen(srCPTRec.szCommIndex) <= 0 || strlen(srCPTRec.szCommIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCommIndex() ERROR !!");

                        if (szCommIndex == NULL)
                        {
                                inDISP_LogPrintf("szCommIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCommIndex length = (%d)", (int)strlen(srCPTRec.szCommIndex));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCommIndex[0], &srCPTRec.szCommIndex[0], strlen(srCPTRec.szCommIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetCommIndex
Date&Time       :
Describe        :
*/
int inSetCommIndex(char* szCommIndex)
{
        memset(srCPTRec.szCommIndex, 0x00, sizeof(srCPTRec.szCommIndex));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCommIndex == NULL || strlen(szCommIndex) <= 0 || strlen(szCommIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCommIndex() ERROR !!");

                        if (szCommIndex == NULL)
                        {
                                inDISP_LogPrintf("szCommIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCommIndex length = (%d)", (int)strlen(szCommIndex));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCPTRec.szCommIndex[0], &szCommIndex[0], strlen(szCommIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetTPDU
Date&Time       :
Describe        :
*/
int inGetTPDU(char* szTPDU)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTPDU == NULL || strlen(srCPTRec.szTPDU) <= 0 || strlen(srCPTRec.szTPDU) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTPDU() ERROR !!");

                        if (szTPDU == NULL)
                        {
                                inDISP_LogPrintf("szTPDU == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTPDU length = (%d)", (int)strlen(srCPTRec.szTPDU));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTPDU[0], &srCPTRec.szTPDU[0], strlen(srCPTRec.szTPDU));

        return (VS_SUCCESS);

}

/*
Function        :inSetTPDU
Date&Time       :
Describe        :
*/
int inSetTPDU(char* szTPDU)
{
        memset(srCPTRec.szTPDU, 0x00, sizeof(srCPTRec.szTPDU));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTPDU == NULL || strlen(szTPDU) <= 0 || strlen(szTPDU) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTPDU() ERROR !!");

                        if (szTPDU == NULL)
                        {
                                inDISP_LogPrintf("szTPDU == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTPDU length = (%d)", (int)strlen(szTPDU));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCPTRec.szTPDU[0], &szTPDU[0], strlen(szTPDU));

        return (VS_SUCCESS);
}

/*
Function        :inGetNII
Date&Time       :
Describe        :
*/
int inGetNII(char* szNII)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szNII == NULL || strlen(srCPTRec.szNII) <= 0 || strlen(srCPTRec.szNII) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetNII() ERROR !!");

                        if (szNII == NULL)
                        {
                                inDISP_LogPrintf("szNII == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szNII length = (%d)", (int)strlen(szNII));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szNII[0], &srCPTRec.szNII[0], strlen(srCPTRec.szNII));

        return (VS_SUCCESS);
}

/*
Function        :inSetNII
Date&Time       :
Describe        :
*/
int inSetNII(char* szNII)
{
        memset(srCPTRec.szNII, 0x00, sizeof(srCPTRec.szNII));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szNII == NULL || strlen(szNII) <= 0 || strlen(szNII) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetNII() ERROR !!");

                        if (szNII == NULL)
                        {
                                inDISP_LogPrintf("szNII == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szNII length = (%d)", (int)strlen(szNII));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCPTRec.szNII[0], &szNII[0], strlen(szNII));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostTelPrimary
Date&Time       :
Describe        :
*/
int inGetHostTelPrimary(char* szHostTelPrimary)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostTelPrimary == NULL || strlen(srCPTRec.szHostTelPrimary) <= 0 || strlen(srCPTRec.szHostTelPrimary) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetHostTelPrimary() ERROR !!");

                        if (szHostTelPrimary == NULL)
                        {
                                inDISP_LogPrintf("szHostTelPrimary == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHostTelPrimary length = (%d)", (int)strlen(szHostTelPrimary));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szHostTelPrimary[0], &srCPTRec.szHostTelPrimary[0], strlen(srCPTRec.szHostTelPrimary));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostTelPrimary
Date&Time       :
Describe        :
*/
int inSetHostTelPrimary(char* szHostTelPrimary)
{
        memset(srCPTRec.szHostTelPrimary, 0x00, sizeof(srCPTRec.szHostTelPrimary));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostTelPrimary == NULL || strlen(szHostTelPrimary) <= 0 || strlen(szHostTelPrimary) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetHostTelPrimary() ERROR !!");

                        if (szHostTelPrimary == NULL)
                        {
                                inDISP_LogPrintf("szHostTelPrimary == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHostTelPrimary length = (%d)", (int)strlen(szHostTelPrimary));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCPTRec.szHostTelPrimary[0], &szHostTelPrimary[0], strlen(szHostTelPrimary));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostTelSecond
Date&Time       :
Describe        :
*/
int inGetHostTelSecond(char* szHostTelSecond)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostTelSecond == NULL || strlen(srCPTRec.szHostTelSecond) <= 0 || strlen(srCPTRec.szHostTelSecond) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetHostTelSecond() ERROR !!");

                        if (szHostTelSecond == NULL)
                        {
                                inDISP_LogPrintf("szHostTelSecond == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHostTelSecond length = (%d)", (int)strlen(szHostTelSecond));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szHostTelSecond[0], &srCPTRec.szHostTelSecond[0], strlen(srCPTRec.szHostTelSecond));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostTelSecond
Date&Time       :
Describe        :
*/
int inSetHostTelSecond(char* szHostTelSecond)
{
        memset(srCPTRec.szHostTelSecond, 0x00, sizeof(srCPTRec.szHostTelSecond));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostTelSecond == NULL || strlen(szHostTelSecond) <= 0 || strlen(szHostTelSecond) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetHostTelSecond() ERROR !!");

                        if (szHostTelSecond == NULL)
                        {
                                inDISP_LogPrintf("szHostTelSecond == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHostTelSecond length = (%d)", (int)strlen(szHostTelSecond));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCPTRec.szHostTelSecond[0], &szHostTelSecond[0], strlen(szHostTelSecond));

        return (VS_SUCCESS);
}

/*
Function        :inGetReferralTel
Date&Time       :
Describe        :
*/
int inGetReferralTel(char* szReferralTel)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szReferralTel == NULL || strlen(srCPTRec.szReferralTel) <= 0 || strlen(srCPTRec.szReferralTel) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetReferralTel() ERROR !!");

                        if (szReferralTel == NULL)
                        {
                                inDISP_LogPrintf("szReferralTel == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szReferralTel length = (%d)", (int)strlen(szReferralTel));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szReferralTel[0], &srCPTRec.szReferralTel[0], strlen(srCPTRec.szReferralTel));

        return (VS_SUCCESS);
}

/*
Function        :inSetReferralTel
Date&Time       :
Describe        :
*/
int inSetReferralTel(char* szReferralTel)
{
        memset(srCPTRec.szReferralTel, 0x00, sizeof(srCPTRec.szReferralTel));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szReferralTel == NULL || strlen(szReferralTel) <= 0 || strlen(szReferralTel) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetReferralTel() ERROR !!");

                        if (szReferralTel == NULL)
                        {
                                inDISP_LogPrintf("szReferralTel == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szReferralTel length = (%d)", (int)strlen(szReferralTel));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCPTRec.szReferralTel[0], &szReferralTel[0], strlen(szReferralTel));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostIPPrimary
Date&Time       :
Describe        :
*/
int inGetHostIPPrimary(char* szHostIPPrimary)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostIPPrimary == NULL || strlen(srCPTRec.szHostIPPrimary) <= 0 || strlen(srCPTRec.szHostIPPrimary) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetHostIPPrimary() ERROR !!");

                        if (szHostIPPrimary == NULL)
                        {
                                inDISP_LogPrintf("szHostIPPrimary == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHostIPPrimary length = (%d)", (int)strlen(szHostIPPrimary));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szHostIPPrimary[0], &srCPTRec.szHostIPPrimary[0], strlen(srCPTRec.szHostIPPrimary));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostIPPrimary
Date&Time       :
Describe        :
*/
int inSetHostIPPrimary(char* szHostIPPrimary)
{
        memset(srCPTRec.szHostIPPrimary, 0x00, sizeof(srCPTRec.szHostIPPrimary));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostIPPrimary == NULL || strlen(szHostIPPrimary) <= 0 || strlen(szHostIPPrimary) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetHostIPPrimary() ERROR !!");

                        if (szHostIPPrimary == NULL)
                        {
                                inDISP_LogPrintf("szHostIPPrimary == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHostIPPrimary length = (%d)", (int)strlen(szHostIPPrimary));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCPTRec.szHostIPPrimary[0], &szHostIPPrimary[0], strlen(szHostIPPrimary));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostPortNoPrimary
Date&Time       :
Describe        :
*/
int inGetHostPortNoPrimary(char* szHostPortNoPrimary)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostPortNoPrimary == NULL || strlen(srCPTRec.szHostPortNoPrimary) <= 0 || strlen(srCPTRec.szHostPortNoPrimary) > 5)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetHostPortNoPrimary() ERROR !!");

                        if (szHostPortNoPrimary == NULL)
                        {
                                inDISP_LogPrintf("szHostPortNoPrimary == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHostPortNoPrimary length = (%d)", (int)strlen(szHostPortNoPrimary));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szHostPortNoPrimary[0], &srCPTRec.szHostPortNoPrimary[0], strlen(srCPTRec.szHostPortNoPrimary));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostPortNoPrimary
Date&Time       :
Describe        :
*/
int inSetHostPortNoPrimary(char* szHostPortNoPrimary)
{
        memset(srCPTRec.szHostPortNoPrimary, 0x00, sizeof(srCPTRec.szHostPortNoPrimary));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostPortNoPrimary == NULL || strlen(szHostPortNoPrimary) <= 0 || strlen(szHostPortNoPrimary) > 5)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetHostPortNoPrimary() ERROR !!");

                        if (szHostPortNoPrimary == NULL)
                        {
                                inDISP_LogPrintf("szHostPortNoPrimary == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHostPortNoPrimary length = (%d)", (int)strlen(szHostPortNoPrimary));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCPTRec.szHostPortNoPrimary[0], &szHostPortNoPrimary[0], strlen(szHostPortNoPrimary));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostIPSecond
Date&Time       :
Describe        :
*/
int inGetHostIPSecond(char* szHostIPSecond)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostIPSecond == NULL || strlen(srCPTRec.szHostIPSecond) <= 0 || strlen(srCPTRec.szHostIPSecond) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetHostIPSecond() ERROR !!");

                        if (szHostIPSecond == NULL)
                        {
                                inDISP_LogPrintf("szHostIPSecond == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHostIPSecond length = (%d)", (int)strlen(szHostIPSecond));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szHostIPSecond[0], &srCPTRec.szHostIPSecond[0], strlen(srCPTRec.szHostIPSecond));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostIPSecond
Date&Time       :
Describe        :
*/
int inSetHostIPSecond(char* szHostIPSecond)
{
        memset(srCPTRec.szHostIPSecond, 0x00, sizeof(srCPTRec.szHostIPSecond));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostIPSecond == NULL || strlen(szHostIPSecond) <= 0 || strlen(szHostIPSecond) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetHostIPSecond() ERROR !!");

                        if (szHostIPSecond == NULL)
                        {
                                inDISP_LogPrintf("szHostIPSecond == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHostIPSecond length = (%d)", (int)strlen(szHostIPSecond));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCPTRec.szHostIPSecond[0], &szHostIPSecond[0], strlen(szHostIPSecond));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostPortNoSecond
Date&Time       :
Describe        :
*/
int inGetHostPortNoSecond(char* szHostPortNoSecond)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostPortNoSecond == NULL || strlen(srCPTRec.szHostPortNoSecond) <= 0 || strlen(srCPTRec.szHostPortNoSecond) > 5)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetHostPortNoSecond() ERROR !!");

                        if (szHostPortNoSecond == NULL)
                        {
                                inDISP_LogPrintf("szHostPortNoSecond == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHostPortNoSecond length = (%d)", (int)strlen(szHostPortNoSecond));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szHostPortNoSecond[0], &srCPTRec.szHostPortNoSecond[0], strlen(srCPTRec.szHostPortNoSecond));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostPortNoSecond
Date&Time       :
Describe        :
*/
int inSetHostPortNoSecond(char* szHostPortNoSecond)
{
        memset(srCPTRec.szHostPortNoSecond, 0x00, sizeof(srCPTRec.szHostPortNoSecond));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostPortNoSecond == NULL || strlen(szHostPortNoSecond) <= 0 || strlen(szHostPortNoSecond) > 5)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetHostPortNoSecond() ERROR !!");

                        if (szHostPortNoSecond == NULL)
                        {
                                inDISP_LogPrintf("szHostPortNoSecond == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHostPortNoSecond length = (%d)", (int)strlen(szHostPortNoSecond));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCPTRec.szHostPortNoSecond[0], &szHostPortNoSecond[0], strlen(szHostPortNoSecond));

        return (VS_SUCCESS);
}

/*
Function        :inGetTCPHeadFormat
Date&Time       :
Describe        :
*/
int inGetTCPHeadFormat(char* szTCPHeadFormat)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTCPHeadFormat == NULL || strlen(srCPTRec.szTCPHeadFormat) <= 0 || strlen(srCPTRec.szTCPHeadFormat) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTCPHeadFormat() ERROR !!");

                        if (szTCPHeadFormat == NULL)
                        {
                                inDISP_LogPrintf("szTCPHeadFormat == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTCPHeadFormat length = (%d)", (int)strlen(szTCPHeadFormat));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTCPHeadFormat[0], &srCPTRec.szTCPHeadFormat[0], strlen(srCPTRec.szTCPHeadFormat));

        return (VS_SUCCESS);
}

/*
Function        :inSetTCPHeadFormat
Date&Time       :
Describe        :
*/
int inSetTCPHeadFormat(char* szTCPHeadFormat)
{
        memset(srCPTRec.szTCPHeadFormat, 0x00, sizeof(srCPTRec.szTCPHeadFormat));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTCPHeadFormat == NULL || strlen(szTCPHeadFormat) <= 0 || strlen(szTCPHeadFormat) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTCPHeadFormat() ERROR !!");

                        if (szTCPHeadFormat == NULL)
                        {
                                inDISP_LogPrintf("szTCPHeadFormat == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTCPHeadFormat length = (%d)", (int)strlen(szTCPHeadFormat));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCPTRec.szTCPHeadFormat[0], &szTCPHeadFormat[0], strlen(szTCPHeadFormat));

        return (VS_SUCCESS);
}

/*
Function        :inGetCarrierTimeOut
Date&Time       :
Describe        :
*/
int inGetCarrierTimeOut(char* szCarrierTimeOut)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCarrierTimeOut == NULL || strlen(srCPTRec.szCarrierTimeOut) <= 0 || strlen(srCPTRec.szCarrierTimeOut) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCarrierTimeOut() ERROR !!");

                        if (szCarrierTimeOut == NULL)
                        {
                                inDISP_LogPrintf("szCarrierTimeOut == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCarrierTimeOut length = (%d)", (int)strlen(szCarrierTimeOut));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCarrierTimeOut[0], &srCPTRec.szCarrierTimeOut[0], strlen(srCPTRec.szCarrierTimeOut));

        return (VS_SUCCESS);
}

/*
Function        :inSetCarrierTimeOut
Date&Time       :
Describe        :
*/
int inSetCarrierTimeOut(char* szCarrierTimeOut)
{
        memset(srCPTRec.szCarrierTimeOut, 0x00, sizeof(srCPTRec.szCarrierTimeOut));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCarrierTimeOut == NULL || strlen(szCarrierTimeOut) <= 0 || strlen(szCarrierTimeOut) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCarrierTimeOut() ERROR !!");

                        if (szCarrierTimeOut == NULL)
                        {
                                inDISP_LogPrintf("szCarrierTimeOut == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCarrierTimeOut length = (%d)", (int)strlen(szCarrierTimeOut));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCPTRec.szCarrierTimeOut[0], &szCarrierTimeOut[0], strlen(szCarrierTimeOut));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostResponseTimeOut
Date&Time       :
Describe        :
*/
int inGetHostResponseTimeOut(char* szHostResponseTimeOut)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostResponseTimeOut == NULL || strlen(srCPTRec.szHostResponseTimeOut) <= 0 || strlen(srCPTRec.szHostResponseTimeOut) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetHostResponseTimeOut() ERROR !!");

                        if (szHostResponseTimeOut == NULL)
                        {
                                inDISP_LogPrintf("szHostResponseTimeOut == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHostResponseTimeOut length = (%d)", (int)strlen(szHostResponseTimeOut));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szHostResponseTimeOut[0], &srCPTRec.szHostResponseTimeOut[0], strlen(srCPTRec.szHostResponseTimeOut));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostResponseTimeOut
Date&Time       :
Describe        :
*/
int inSetHostResponseTimeOut(char* szHostResponseTimeOut)
{
        memset(srCPTRec.szHostResponseTimeOut, 0x00, sizeof(srCPTRec.szHostResponseTimeOut));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostResponseTimeOut == NULL || strlen(szHostResponseTimeOut) <= 0 || strlen(szHostResponseTimeOut) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetHostResponseTimeOut() ERROR !!");

                        if (szHostResponseTimeOut == NULL)
                        {
                                inDISP_LogPrintf("szHostResponseTimeOut == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHostResponseTimeOut length = (%d)", (int)strlen(szHostResponseTimeOut));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCPTRec.szHostResponseTimeOut[0], &szHostResponseTimeOut[0], strlen(szHostResponseTimeOut));

        return (VS_SUCCESS);
}


/*
Function        :inGetSettleTelPrimary
Date&Time       :
Describe        :
*/
int inGetSettleTelPrimary(char* szSettleTelPrimary)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSettleTelPrimary == NULL || strlen(srCPTRec.szSettleTelPrimary) <= 0 || strlen(srCPTRec.szSettleTelPrimary) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetSettleTelPrimary() ERROR !!");

                        if (szSettleTelPrimary == NULL)
                        {
                                inDISP_LogPrintf("szSettleTelPrimary == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSettleTelPrimary length = (%d)", (int)strlen(szSettleTelPrimary));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSettleTelPrimary[0], &srCPTRec.szSettleTelPrimary[0], strlen(srCPTRec.szSettleTelPrimary));

        return (VS_SUCCESS);
}

/*
Function        :inSetSettleTelPrimary
Date&Time       :
Describe        :
*/
int inSetSettleTelPrimary(char* szSettleTelPrimary)
{
        memset(srCPTRec.szSettleTelPrimary, 0x00, sizeof(srCPTRec.szSettleTelPrimary));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSettleTelPrimary == NULL || strlen(szSettleTelPrimary) <= 0 || strlen(szSettleTelPrimary) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetSettleTelPrimary() ERROR !!");

                        if (szSettleTelPrimary == NULL)
                        {
                                inDISP_LogPrintf("szSettleTelPrimary == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSettleTelPrimary length = (%d)", (int)strlen(szSettleTelPrimary));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCPTRec.szSettleTelPrimary[0], &szSettleTelPrimary[0], strlen(szSettleTelPrimary));

        return (VS_SUCCESS);
}

/*
Function        :inGetSettleTelSecond
Date&Time       :
Describe        :
*/
int inGetSettleTelSecond(char* szSettleTelSecond)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSettleTelSecond == NULL || strlen(srCPTRec.szSettleTelSecond) <= 0 || strlen(srCPTRec.szSettleTelSecond) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetSettleTelSecond() ERROR !!");

                        if (szSettleTelSecond == NULL)
                        {
                                inDISP_LogPrintf("szSettleTelSecond == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSettleTelSecond length = (%d)", (int)strlen(szSettleTelSecond));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSettleTelSecond[0], &srCPTRec.szSettleTelSecond[0], strlen(srCPTRec.szSettleTelSecond));

        return (VS_SUCCESS);
}

/*
Function        :inSetSettleTelSecond
Date&Time       :
Describe        :
*/
int inSetSettleTelSecond(char* szSettleTelSecond)
{
        memset(srCPTRec.szSettleTelSecond, 0x00, sizeof(srCPTRec.szSettleTelSecond));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSettleTelSecond == NULL || strlen(szSettleTelSecond) <= 0 || strlen(szSettleTelSecond) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetSettleTelSecond() ERROR !!");

                        if (szSettleTelSecond == NULL)
                        {
                                inDISP_LogPrintf("szSettleTelSecond == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSettleTelSecond length = (%d)", (int)strlen(szSettleTelSecond));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCPTRec.szSettleTelSecond[0], &szSettleTelSecond[0], strlen(szSettleTelSecond));

        return (VS_SUCCESS);
}



/*
Function        :inCPT_Edit_CPT_Table
Date&Time       :2017/5/15 下午 4:08
Describe        :
*/
int inCPT_Edit_CPT_Table(void)
{
	TABLE_GET_SET_TABLE CPT_FUNC_TABLE[] =
	{
		{"szCommIndex"			,inGetCommIndex			,inSetCommIndex			},
		{"szTPDU"			,inGetTPDU			,inSetTPDU			},
		{"szNII"			,inGetNII			,inSetNII			},
		{"szHostTelPrimary"		,inGetHostTelPrimary		,inSetHostTelPrimary		},
		{"szHostTelSecond"		,inGetHostTelSecond		,inSetHostTelSecond		},
		{"szReferralTel"		,inGetReferralTel		,inSetReferralTel		},
		{"szHostIPPrimary"		,inGetHostIPPrimary		,inSetHostIPPrimary		},
		{"szHostPortNoPrimary"		,inGetHostPortNoPrimary		,inSetHostPortNoPrimary		},
		{"szHostIPSecond"		,inGetHostIPSecond		,inSetHostIPSecond		},
		{"szHostPortNoSecond"		,inGetHostPortNoSecond		,inSetHostPortNoSecond		},
		{"szTCPHeadFormat"		,inGetTCPHeadFormat		,inSetTCPHeadFormat		},
		{"szCarrierTimeOut"		,inGetCarrierTimeOut		,inSetCarrierTimeOut		},
		{"szHostResponseTimeOut"	,inGetHostResponseTimeOut	,inSetHostResponseTimeOut	},
		{"szSettleTelPrimary"	,inGetSettleTelPrimary	,inSetSettleTelPrimary	},
		{"szSettleTelSecond"	,inGetSettleTelSecond	,inSetSettleTelSecond	},
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
	inDISP_ChineseFont_Color("是否更改CPT", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
	inLoadCPTRec(inRecordCnt);
	
	inFunc_Edit_Table_Tag(CPT_FUNC_TABLE);
	inSaveCPTRec(inRecordCnt);
	
	return	(VS_SUCCESS);
}

int inCPT_Test1(void)
{
        inLoadCPTRec(0);
        inSetCommIndex("99");
        inSetTPDU("6666123456");
        inSetNII("338");
        inSetHostTelPrimary("23577711");
        inSetHostTelSecond("23577711");
        inSetReferralTel("1234");
        inSetHostIPPrimary("9999");
        inSetHostPortNoPrimary("555");
        inSetHostIPSecond("123.321.44.2");
        inSetHostPortNoSecond("1234");
        inSetTCPHeadFormat("H");
        inSetCarrierTimeOut("30");
        inSetHostResponseTimeOut("20");
        inSaveCPTRec(0);

        return (VS_SUCCESS);
}
