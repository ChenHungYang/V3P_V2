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
#include "CPT_Backup.h"

static  CPT_REC srCPT_BACKUPRec;	/* construct CPT record */
extern  int     ginDebug;		/* Debug使用 extern */

/*
Function        :inLoadCPT_BACKUPRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀CPT檔案，inCPT_Rec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadCPT_BackupRec(int inCPTRec)
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

        /* inLoadCPT_BACKUPRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadCPT_BACKUPRec(%d) START!!", inCPTRec);
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
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_CPT_BACKUP_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnCPTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_CPT_BACKUP_FILE_NAME_);

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
        memset(&srCPT_BACKUPRec, 0x00, sizeof(srCPT_BACKUPRec));
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
                memcpy(&srCPT_BACKUPRec.szCommIndex[0], &szCPTRec[0], k - 1);
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
                memcpy(&srCPT_BACKUPRec.szTPDU[0], &szCPTRec[0], k - 1);
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
                memcpy(&srCPT_BACKUPRec.szNII[0], &szCPTRec[0], k - 1);
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
                memcpy(&srCPT_BACKUPRec.szHostTelPrimary[0], &szCPTRec[0], k - 1);
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
                memcpy(&srCPT_BACKUPRec.szHostTelSecond[0], &szCPTRec[0], k - 1);
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
                memcpy(&srCPT_BACKUPRec.szReferralTel[0], &szCPTRec[0], k - 1);
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
                memcpy(&srCPT_BACKUPRec.szHostIPPrimary[0], &szCPTRec[0], k - 1);
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
                memcpy(&srCPT_BACKUPRec.szHostPortNoPrimary[0], &szCPTRec[0], k - 1);
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
                memcpy(&srCPT_BACKUPRec.szHostIPSecond[0], &szCPTRec[0], k - 1);
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
                memcpy(&srCPT_BACKUPRec.szHostPortNoSecond[0], &szCPTRec[0], k - 1);
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
                memcpy(&srCPT_BACKUPRec.szTCPHeadFormat[0], &szCPTRec[0], k - 1);
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
                memcpy(&srCPT_BACKUPRec.szCarrierTimeOut[0], &szCPTRec[0], k - 1);
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
                memcpy(&srCPT_BACKUPRec.szHostResponseTimeOut[0], &szCPTRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

	/* inLoadCPT_BACKUPRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadCPT_BACKUPRec(%d) END!!", inCPTRec);
                inDISP_LogPrintf(szErrorMsg);
		inDISP_LogPrintf("----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveCPT_BACKUPRec
Date&Time       :
Describe        :
*/
int inSaveCPT_BackupRec(int inCPTRec)
{
        unsigned long   uldat_Handle;   			/* FILE Handle */
        unsigned long   ulbak_Handle;   			/* FILE Handle */
        int     	inPackCount = 0;        		/* uszWriteBuff_Record的index */
        int     	inRecIndex = 0;         		/* uszRead_Total_Buff的index */
        int     	inCPT_Total_Rec = 0;    		/* CPT.dat的總筆數 */
        int     	inRetVal;
        int     	i = 0, j = 0;
	char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];	/* debug message */
        long    	lnReadLength = 0;       		/* 每次要從CPT.dat讀多少byte出來 */
        long    	lnCPTLength = 0;         		/* CPT.dat檔案總長度 */
        unsigned char   *uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSaveCPT_BACKUPRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveCPT_BACKUPRec(%d) START!!", inCPTRec);
                inDISP_LogPrintf(szErrorMsg);
        }

        /* 防呆先刪除CPT.bak  */
        inFILE_Delete((unsigned char *)_CPT_BACKUP_BAK_FILE_NAME_);

        /* 新建CPT.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_CPT_BACKUP_BAK_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案CPT.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_CPT_BACKUP_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* CPT.dat開檔失敗 ，不用關檔CPT.dat */
                /* CPT.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_CPT_BACKUP_BAK_FILE_NAME_);

                return(VS_ERROR);
        }

        /* 取得CPT.dat檔案大小 */
        lnCPTLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_CPT_BACKUP_FILE_NAME_);

	/* 取得檔案大小失敗 */
        if (lnCPTLength == VS_ERROR)
        {
		/* CPT.bak和CPT.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_CPT_BACKUP_BAK_FILE_NAME_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_CPT_REC_ + _SIZE_CPT_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_CPT_REC_ + _SIZE_CPT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原CPT.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_CPT_REC_ + _SIZE_CPT_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_CPT_REC_ + _SIZE_CPT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存CPT.dat全部資料 */
        uszRead_Total_Buff = malloc(lnCPTLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnCPTLength + 1);

	inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* CommIndex */
        memcpy(&uszWriteBuff_Record[0], &srCPT_BACKUPRec.szCommIndex[0], strlen(srCPT_BACKUPRec.szCommIndex));
        inPackCount += strlen(srCPT_BACKUPRec.szCommIndex);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* TPDU */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCPT_BACKUPRec.szTPDU[0], strlen(srCPT_BACKUPRec.szTPDU));
        inPackCount += strlen(srCPT_BACKUPRec.szTPDU);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* NII */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCPT_BACKUPRec.szNII[0], strlen(srCPT_BACKUPRec.szNII));
        inPackCount += strlen(srCPT_BACKUPRec.szNII);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* HostTelPrimary */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCPT_BACKUPRec.szHostTelPrimary[0], strlen(srCPT_BACKUPRec.szHostTelPrimary));
        inPackCount += strlen(srCPT_BACKUPRec.szHostTelPrimary);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* HostTelSecond */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCPT_BACKUPRec.szHostTelSecond[0], strlen(srCPT_BACKUPRec.szHostTelSecond));
        inPackCount += strlen(srCPT_BACKUPRec.szHostTelSecond);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* ReferralTel */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCPT_BACKUPRec.szReferralTel[0], strlen(srCPT_BACKUPRec.szReferralTel));
        inPackCount += strlen(srCPT_BACKUPRec.szReferralTel);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* HostIPPrimary */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCPT_BACKUPRec.szHostIPPrimary[0], strlen(srCPT_BACKUPRec.szHostIPPrimary));
        inPackCount += strlen(srCPT_BACKUPRec.szHostIPPrimary);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* HostPortNoPrimary */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCPT_BACKUPRec.szHostPortNoPrimary[0], strlen(srCPT_BACKUPRec.szHostPortNoPrimary));
        inPackCount += strlen(srCPT_BACKUPRec.szHostPortNoPrimary);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* HostIPSecond */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCPT_BACKUPRec.szHostIPSecond[0], strlen(srCPT_BACKUPRec.szHostIPSecond));
        inPackCount += strlen(srCPT_BACKUPRec.szHostIPSecond);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* HostPortNoSecond */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCPT_BACKUPRec.szHostPortNoSecond[0], strlen(srCPT_BACKUPRec.szHostPortNoSecond));
        inPackCount += strlen(srCPT_BACKUPRec.szHostPortNoSecond);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* TCPHeadFormat */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCPT_BACKUPRec.szTCPHeadFormat[0], strlen(srCPT_BACKUPRec.szTCPHeadFormat));
        inPackCount += strlen(srCPT_BACKUPRec.szTCPHeadFormat);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* CarrierTimeOut */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCPT_BACKUPRec.szCarrierTimeOut[0], strlen(srCPT_BACKUPRec.szCarrierTimeOut));
        inPackCount += strlen(srCPT_BACKUPRec.szCarrierTimeOut);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* HostResponseTimeOut */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCPT_BACKUPRec.szHostResponseTimeOut[0], strlen(srCPT_BACKUPRec.szHostResponseTimeOut));
        inPackCount += strlen(srCPT_BACKUPRec.szHostResponseTimeOut);

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀CPT.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnCPTLength;

                for (i = 0;; ++i)
                {
                        /* 剩餘長度大於或等於1024 */
                        if (lnReadLength >= 1024)
                        {
                                if (inFILE_Read(&uldat_Handle, &uszRead_Total_Buff[1024 * i], 1024) == VS_SUCCESS)
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
                                        inFILE_Close(&uldat_Handle);
                                        inFILE_Close(&ulbak_Handle);
                                        inFILE_Delete((unsigned char *)_CPT_BACKUP_BAK_FILE_NAME_);

                                        /* Free pointer */
					free(uszRead_Total_Buff);
                                        free(uszWriteBuff_Record);
                                        free(uszWriteBuff_Org);

                                        return (VS_ERROR);
                                }
                        }
                        /* 剩餘長度小於1024 */
                        else if (lnReadLength < 1024)
                        {
                                /* 就只讀剩餘長度 */
                                if (inFILE_Read(&uldat_Handle, &uszRead_Total_Buff[1024 * i], lnReadLength) == VS_SUCCESS)
                                {
                                        break;
                                }
                                /* 讀失敗時 */
                                else
                                {
                                        /* Close檔案 */
                                        inFILE_Close(&uldat_Handle);
                                        inFILE_Close(&ulbak_Handle);
                                        inFILE_Delete((unsigned char *)_CPT_BACKUP_BAK_FILE_NAME_);

                                        /* Free pointer */
					free(uszRead_Total_Buff);
                                        free(uszWriteBuff_Record);
                                        free(uszWriteBuff_Org);

                                        return (VS_ERROR);
                                }
                        }
                } /* end for loop */
        }
        else
        {
                /* Close檔案 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_CPT_BACKUP_BAK_FILE_NAME_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inCPTRec Return ERROR */
        /* 算總Record數 */
	for (i = 0; i < (lnCPTLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inCPT_Total_Rec++;
        }

	/* 防呆 總record數量小於要存取inCPTRec Return ERROR */
        if ((inCPTRec + 1) > inCPT_Total_Rec)
        {
                if (ginDebug == VS_TRUE)
                {
                        inDISP_LogPrintf("No data or Index ERROR");
                }

                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_CPT_BACKUP_BAK_FILE_NAME_);

                /* Free pointer */
		free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inCPTRec決定要先存幾筆Record到CPT.bak，ex:inCPTRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inCPTRec; i ++)
        {
                j = 0;

                while (1)
                {
                        /* 存Record */
                        uszWriteBuff_Org[j] = uszRead_Total_Buff[inRecIndex];
                        j++;

                        /* 每讀完一筆record，就把baffer中record的資料寫入bak */
                        if (uszRead_Total_Buff[inRecIndex] == 0x0A)
                        {
                                /* 為防止寫入位置錯誤，先移動到bak的檔案結尾 */
                                inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

                                if (inRetVal != VS_SUCCESS)
                                {
                                        /* 關檔 */
                                        inFILE_Close(&uldat_Handle);
                                        inFILE_Close(&ulbak_Handle);
                                        inFILE_Delete((unsigned char *)_CPT_BACKUP_BAK_FILE_NAME_);

                                        /* Free pointer */
					free(uszWriteBuff_Record);
                                        free(uszWriteBuff_Org);
                                        free(uszRead_Total_Buff);

                                        return (VS_ERROR);
                                }

                                /* 寫入bak檔案 */
                                inRetVal = inFILE_Write(&ulbak_Handle, uszWriteBuff_Org, j);

                                if (inRetVal != VS_SUCCESS)
                                {
                                        /* 關檔 */
                                        inFILE_Close(&uldat_Handle);
                                        inFILE_Close(&ulbak_Handle);
                                        inFILE_Delete((unsigned char *)_CPT_BACKUP_BAK_FILE_NAME_);

                                        /* Free pointer */
					free(uszWriteBuff_Record);
                                        free(uszWriteBuff_Org);
                                        free(uszRead_Total_Buff);

                                        return (VS_ERROR);
                                }

                                /* 清空buffer為下一筆寫入做準備 */
                                memset(uszWriteBuff_Org, 0x00, sizeof(uszWriteBuff_Org));
                                inRecIndex++;

                                break;
                        }

                        /* 讀下一個字元 */
                        inRecIndex++;
                }
        }

        /* 存組好的該CPTRecord 到 CPT.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_CPT_BACKUP_BAK_FILE_NAME_);

		/* Free pointer */
		free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 將組好的record寫入bak檔 */
        inRetVal = inFILE_Write(&ulbak_Handle, uszWriteBuff_Record, inPackCount);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_CPT_BACKUP_BAK_FILE_NAME_);

                /* Free pointer */
		free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原CPT.dat Record 到 CPT.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
	/* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
	inCPTRec = inCPTRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnCPTLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inCPTRec)
                        {
                                inRecIndex = i + 1;
                                break;
                        }
                }
        }

        i = 0;
        memset(uszWriteBuff_Org, 0x00, sizeof(uszWriteBuff_Org));

        while (1)
        {
                /* 接續存原CPT.dat的Record */
                uszWriteBuff_Org[i] = uszRead_Total_Buff[inRecIndex];
                i++;

                if (uszRead_Total_Buff[inRecIndex] == 0x0A)
                {
                        /* 移動到bak的檔案結尾 */
                        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

                        if (inRetVal != VS_SUCCESS)
                        {
                                /* 關檔 */
                                inFILE_Close(&uldat_Handle);
                                inFILE_Close(&ulbak_Handle);
                                inFILE_Delete((unsigned char *)_CPT_BACKUP_BAK_FILE_NAME_);

                                /* Free pointer */
				free(uszWriteBuff_Record);
                                free(uszWriteBuff_Org);
                                free(uszRead_Total_Buff);

                                return (VS_ERROR);
                        }

                        inRetVal = inFILE_Write(&ulbak_Handle, uszWriteBuff_Org, i);

                        if (inRetVal != VS_SUCCESS)
                        {
                                /* 關檔 */
                                inFILE_Close(&uldat_Handle);
                                inFILE_Close(&ulbak_Handle);
                                inFILE_Delete((unsigned char *)_CPT_BACKUP_BAK_FILE_NAME_);

                                /* Free pointer */
				free(uszWriteBuff_Record);
                                free(uszWriteBuff_Org);
                                free(uszRead_Total_Buff);

                                return (VS_ERROR);
                        }

                        memset(uszWriteBuff_Org, 0x00, sizeof(uszWriteBuff_Org));
                        i = 0;
                }
                else if (uszRead_Total_Buff[inRecIndex] == 0x00)
                {
                        /* 讀到0x00結束 */
                        break;
                }

                inRecIndex++;
        }

        /* 關檔 */
        inFILE_Close(&uldat_Handle);
        inFILE_Close(&ulbak_Handle);

        /* Free pointer */
        free(uszWriteBuff_Record);
        free(uszWriteBuff_Org);
        free(uszRead_Total_Buff);

        /* 刪除原CPT.dat */
        if (inFILE_Delete((unsigned char *)_CPT_BACKUP_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將CPT.bak改名字為CPT.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_CPT_BACKUP_BAK_FILE_NAME_, (unsigned char *)_CPT_BACKUP_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* inSaveCPT_BACKUPRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveCPT_BACKUPRec(%d) END!!", inCPTRec - 1);
                inDISP_LogPrintf(szErrorMsg);
		inDISP_LogPrintf("----------------------------------------");
        }

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
int inGetCommIndex_Backup(char* szCommIndex)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCommIndex == NULL || strlen(srCPT_BACKUPRec.szCommIndex) <= 0 || strlen(srCPT_BACKUPRec.szCommIndex) > 2)
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
                                sprintf(szErrorMsg, "szCommIndex length = (%d)", (int)strlen(srCPT_BACKUPRec.szCommIndex));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCommIndex[0], &srCPT_BACKUPRec.szCommIndex[0], strlen(srCPT_BACKUPRec.szCommIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetCommIndex
Date&Time       :
Describe        :
*/
int inSetCommIndex_Backup(char* szCommIndex)
{
        memset(srCPT_BACKUPRec.szCommIndex, 0x00, sizeof(srCPT_BACKUPRec.szCommIndex));
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
        memcpy(&srCPT_BACKUPRec.szCommIndex[0], &szCommIndex[0], strlen(szCommIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetTPDU
Date&Time       :
Describe        :
*/
int inGetTPDU_Backup(char* szTPDU)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTPDU == NULL || strlen(srCPT_BACKUPRec.szTPDU) <= 0 || strlen(srCPT_BACKUPRec.szTPDU) > 10)
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
                                sprintf(szErrorMsg, "szTPDU length = (%d)", (int)strlen(srCPT_BACKUPRec.szTPDU));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTPDU[0], &srCPT_BACKUPRec.szTPDU[0], strlen(srCPT_BACKUPRec.szTPDU));

        return (VS_SUCCESS);

}

/*
Function        :inSetTPDU
Date&Time       :
Describe        :
*/
int inSetTPDU_Backup(char* szTPDU)
{
        memset(srCPT_BACKUPRec.szTPDU, 0x00, sizeof(srCPT_BACKUPRec.szTPDU));
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
        memcpy(&srCPT_BACKUPRec.szTPDU[0], &szTPDU[0], strlen(szTPDU));

        return (VS_SUCCESS);
}

/*
Function        :inGetNII
Date&Time       :
Describe        :
*/
int inGetNII_Backup(char* szNII)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szNII == NULL || strlen(srCPT_BACKUPRec.szNII) <= 0 || strlen(srCPT_BACKUPRec.szNII) > 3)
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
        memcpy(&szNII[0], &srCPT_BACKUPRec.szNII[0], strlen(srCPT_BACKUPRec.szNII));

        return (VS_SUCCESS);
}

/*
Function        :inSetNII
Date&Time       :
Describe        :
*/
int inSetNII_Backup(char* szNII)
{
        memset(srCPT_BACKUPRec.szNII, 0x00, sizeof(srCPT_BACKUPRec.szNII));
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
        memcpy(&srCPT_BACKUPRec.szNII[0], &szNII[0], strlen(szNII));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostTelPrimary
Date&Time       :
Describe        :
*/
int inGetHostTelPrimary_Backup(char* szHostTelPrimary)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostTelPrimary == NULL || strlen(srCPT_BACKUPRec.szHostTelPrimary) <= 0 || strlen(srCPT_BACKUPRec.szHostTelPrimary) > 15)
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
        memcpy(&szHostTelPrimary[0], &srCPT_BACKUPRec.szHostTelPrimary[0], strlen(srCPT_BACKUPRec.szHostTelPrimary));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostTelPrimary
Date&Time       :
Describe        :
*/
int inSetHostTelPrimary_Backup(char* szHostTelPrimary)
{
        memset(srCPT_BACKUPRec.szHostTelPrimary, 0x00, sizeof(srCPT_BACKUPRec.szHostTelPrimary));
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
        memcpy(&srCPT_BACKUPRec.szHostTelPrimary[0], &szHostTelPrimary[0], strlen(szHostTelPrimary));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostTelSecond
Date&Time       :
Describe        :
*/
int inGetHostTelSecond_Backup(char* szHostTelSecond)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostTelSecond == NULL || strlen(srCPT_BACKUPRec.szHostTelSecond) <= 0 || strlen(srCPT_BACKUPRec.szHostTelSecond) > 15)
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
        memcpy(&szHostTelSecond[0], &srCPT_BACKUPRec.szHostTelSecond[0], strlen(srCPT_BACKUPRec.szHostTelSecond));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostTelSecond
Date&Time       :
Describe        :
*/
int inSetHostTelSecond_Backup(char* szHostTelSecond)
{
        memset(srCPT_BACKUPRec.szHostTelSecond, 0x00, sizeof(srCPT_BACKUPRec.szHostTelSecond));
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
        memcpy(&srCPT_BACKUPRec.szHostTelSecond[0], &szHostTelSecond[0], strlen(szHostTelSecond));

        return (VS_SUCCESS);
}

/*
Function        :inGetReferralTel
Date&Time       :
Describe        :
*/
int inGetReferralTel_Backup(char* szReferralTel)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szReferralTel == NULL || strlen(srCPT_BACKUPRec.szReferralTel) <= 0 || strlen(srCPT_BACKUPRec.szReferralTel) > 15)
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
        memcpy(&szReferralTel[0], &srCPT_BACKUPRec.szReferralTel[0], strlen(srCPT_BACKUPRec.szReferralTel));

        return (VS_SUCCESS);
}

/*
Function        :inSetReferralTel
Date&Time       :
Describe        :
*/
int inSetReferralTel_Backup(char* szReferralTel)
{
        memset(srCPT_BACKUPRec.szReferralTel, 0x00, sizeof(srCPT_BACKUPRec.szReferralTel));
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
        memcpy(&srCPT_BACKUPRec.szReferralTel[0], &szReferralTel[0], strlen(szReferralTel));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostIPPrimary
Date&Time       :
Describe        :
*/
int inGetHostIPPrimary_Backup(char* szHostIPPrimary)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostIPPrimary == NULL || strlen(srCPT_BACKUPRec.szHostIPPrimary) <= 0 || strlen(srCPT_BACKUPRec.szHostIPPrimary) > 15)
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
        memcpy(&szHostIPPrimary[0], &srCPT_BACKUPRec.szHostIPPrimary[0], strlen(srCPT_BACKUPRec.szHostIPPrimary));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostIPPrimary
Date&Time       :
Describe        :
*/
int inSetHostIPPrimary_Backup(char* szHostIPPrimary)
{
        memset(srCPT_BACKUPRec.szHostIPPrimary, 0x00, sizeof(srCPT_BACKUPRec.szHostIPPrimary));
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
        memcpy(&srCPT_BACKUPRec.szHostIPPrimary[0], &szHostIPPrimary[0], strlen(szHostIPPrimary));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostPortNoPrimary
Date&Time       :
Describe        :
*/
int inGetHostPortNoPrimary_Backup(char* szHostPortNoPrimary)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostPortNoPrimary == NULL || strlen(srCPT_BACKUPRec.szHostPortNoPrimary) <= 0 || strlen(srCPT_BACKUPRec.szHostPortNoPrimary) > 5)
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
        memcpy(&szHostPortNoPrimary[0], &srCPT_BACKUPRec.szHostPortNoPrimary[0], strlen(srCPT_BACKUPRec.szHostPortNoPrimary));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostPortNoPrimary
Date&Time       :
Describe        :
*/
int inSetHostPortNoPrimary_Backup(char* szHostPortNoPrimary)
{
        memset(srCPT_BACKUPRec.szHostPortNoPrimary, 0x00, sizeof(srCPT_BACKUPRec.szHostPortNoPrimary));
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
        memcpy(&srCPT_BACKUPRec.szHostPortNoPrimary[0], &szHostPortNoPrimary[0], strlen(szHostPortNoPrimary));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostIPSecond
Date&Time       :
Describe        :
*/
int inGetHostIPSecond_Backup(char* szHostIPSecond)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostIPSecond == NULL || strlen(srCPT_BACKUPRec.szHostIPSecond) <= 0 || strlen(srCPT_BACKUPRec.szHostIPSecond) > 15)
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
        memcpy(&szHostIPSecond[0], &srCPT_BACKUPRec.szHostIPSecond[0], strlen(srCPT_BACKUPRec.szHostIPSecond));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostIPSecond
Date&Time       :
Describe        :
*/
int inSetHostIPSecond_Backup(char* szHostIPSecond)
{
        memset(srCPT_BACKUPRec.szHostIPSecond, 0x00, sizeof(srCPT_BACKUPRec.szHostIPSecond));
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
        memcpy(&srCPT_BACKUPRec.szHostIPSecond[0], &szHostIPSecond[0], strlen(szHostIPSecond));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostPortNoSecond
Date&Time       :
Describe        :
*/
int inGetHostPortNoSecond_Backup(char* szHostPortNoSecond)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostPortNoSecond == NULL || strlen(srCPT_BACKUPRec.szHostPortNoSecond) <= 0 || strlen(srCPT_BACKUPRec.szHostPortNoSecond) > 5)
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
        memcpy(&szHostPortNoSecond[0], &srCPT_BACKUPRec.szHostPortNoSecond[0], strlen(srCPT_BACKUPRec.szHostPortNoSecond));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostPortNoSecond
Date&Time       :
Describe        :
*/
int inSetHostPortNoSecond_Backup(char* szHostPortNoSecond)
{
        memset(srCPT_BACKUPRec.szHostPortNoSecond, 0x00, sizeof(srCPT_BACKUPRec.szHostPortNoSecond));
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
        memcpy(&srCPT_BACKUPRec.szHostPortNoSecond[0], &szHostPortNoSecond[0], strlen(szHostPortNoSecond));

        return (VS_SUCCESS);
}

/*
Function        :inGetTCPHeadFormat
Date&Time       :
Describe        :
*/
int inGetTCPHeadFormat_Backup(char* szTCPHeadFormat)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTCPHeadFormat == NULL || strlen(srCPT_BACKUPRec.szTCPHeadFormat) <= 0 || strlen(srCPT_BACKUPRec.szTCPHeadFormat) > 1)
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
        memcpy(&szTCPHeadFormat[0], &srCPT_BACKUPRec.szTCPHeadFormat[0], strlen(srCPT_BACKUPRec.szTCPHeadFormat));

        return (VS_SUCCESS);
}

/*
Function        :inSetTCPHeadFormat
Date&Time       :
Describe        :
*/
int inSetTCPHeadFormat_Backup(char* szTCPHeadFormat)
{
        memset(srCPT_BACKUPRec.szTCPHeadFormat, 0x00, sizeof(srCPT_BACKUPRec.szTCPHeadFormat));
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
        memcpy(&srCPT_BACKUPRec.szTCPHeadFormat[0], &szTCPHeadFormat[0], strlen(szTCPHeadFormat));

        return (VS_SUCCESS);
}

/*
Function        :inGetCarrierTimeOut
Date&Time       :
Describe        :
*/
int inGetCarrierTimeOut_Backup(char* szCarrierTimeOut)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCarrierTimeOut == NULL || strlen(srCPT_BACKUPRec.szCarrierTimeOut) <= 0 || strlen(srCPT_BACKUPRec.szCarrierTimeOut) > 2)
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
        memcpy(&szCarrierTimeOut[0], &srCPT_BACKUPRec.szCarrierTimeOut[0], strlen(srCPT_BACKUPRec.szCarrierTimeOut));

        return (VS_SUCCESS);
}

/*
Function        :inSetCarrierTimeOut
Date&Time       :
Describe        :
*/
int inSetCarrierTimeOut_Backup(char* szCarrierTimeOut)
{
        memset(srCPT_BACKUPRec.szCarrierTimeOut, 0x00, sizeof(srCPT_BACKUPRec.szCarrierTimeOut));
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
        memcpy(&srCPT_BACKUPRec.szCarrierTimeOut[0], &szCarrierTimeOut[0], strlen(szCarrierTimeOut));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostResponseTimeOut
Date&Time       :
Describe        :
*/
int inGetHostResponseTimeOut_Backup(char* szHostResponseTimeOut)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostResponseTimeOut == NULL || strlen(srCPT_BACKUPRec.szHostResponseTimeOut) <= 0 || strlen(srCPT_BACKUPRec.szHostResponseTimeOut) > 2)
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
        memcpy(&szHostResponseTimeOut[0], &srCPT_BACKUPRec.szHostResponseTimeOut[0], strlen(srCPT_BACKUPRec.szHostResponseTimeOut));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostResponseTimeOut
Date&Time       :
Describe        :
*/
int inSetHostResponseTimeOut_Backup(char* szHostResponseTimeOut)
{
        memset(srCPT_BACKUPRec.szHostResponseTimeOut, 0x00, sizeof(srCPT_BACKUPRec.szHostResponseTimeOut));
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
        memcpy(&srCPT_BACKUPRec.szHostResponseTimeOut[0], &szHostResponseTimeOut[0], strlen(szHostResponseTimeOut));

        return (VS_SUCCESS);
}

/*
Function        :inCPT_Edit_CPT_Table
Date&Time       :2017/5/15 下午 4:08
Describe        :
*/
int inCPT_Backup_Edit_CPT_Backup_Table(void)
{
	TABLE_GET_SET_TABLE CPT_BACKUP_FUNC_TABLE[] =
	{
		{"szCommIndex"			,inGetCommIndex_Backup			,inSetCommIndex_Backup			},
		{"szTPDU"			,inGetTPDU_Backup			,inSetTPDU_Backup			},
		{"szNII"			,inGetNII_Backup			,inSetNII_Backup			},
		{"szHostTelPrimary"		,inGetHostTelPrimary_Backup		,inSetHostTelPrimary_Backup		},
		{"szHostTelSecond"		,inGetHostTelSecond_Backup		,inSetHostTelSecond_Backup		},
		{"szReferralTel"		,inGetReferralTel_Backup		,inSetReferralTel_Backup		},
		{"szHostIPPrimary"		,inGetHostIPPrimary_Backup		,inSetHostIPPrimary_Backup		},
		{"szHostPortNoPrimary"		,inGetHostPortNoPrimary_Backup		,inSetHostPortNoPrimary_Backup		},
		{"szHostIPSecond"		,inGetHostIPSecond_Backup		,inSetHostIPSecond_Backup		},
		{"szHostPortNoSecond"		,inGetHostPortNoSecond_Backup		,inSetHostPortNoSecond_Backup		},
		{"szTCPHeadFormat"		,inGetTCPHeadFormat_Backup		,inSetTCPHeadFormat_Backup		},
		{"szCarrierTimeOut"		,inGetCarrierTimeOut_Backup		,inSetCarrierTimeOut_Backup		},
		{"szHostResponseTimeOut"	,inGetHostResponseTimeOut_Backup	,inSetHostResponseTimeOut_Backup	},
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
	inDISP_ChineseFont_Color("是否更改CPT_BACKUP", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
					inLoadCPT_BackupRec(inRecordCnt);
	
					inFunc_Edit_Table_Tag(CPT_BACKUP_FUNC_TABLE);
					inSaveCPT_BackupRec(inRecordCnt);
					break;
				}
			}

			break;
		}
		
	}
	
	return	(VS_SUCCESS);
}
