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
#include "CFGT.h"
#include "../../CTLS/CTLS.h"	/* For inCFGT_Edit_CFGT_Table 用 */

static  CFGT_REC srCFGTRec;	/* construct CFGT record */
extern  int	ginDebug;	/* Debug使用 extern */

/*
Function        :inLoadCFGTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀CFGT檔案，inCFGTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadCFGTRec(int inCFGTRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆CFGT檔案 */
        char            szCFGTRec[_SIZE_CFGT_REC_ + 1];         /* 暫存, 放各個欄位檔案 */
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnCFGTLength = 0;                       /* CFGT總長度 */
        long            lnReadLength;                           /* 記錄每次要從CFGT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;                 /* inRec記錄讀到第幾筆, i為目前從CFGT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadCFGTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadCFGTRec(%d) START!!", inCFGTRec);
                inDISP_LogPrintf(szErrorMsg);
        }

        /* 判斷傳進來的inCFGTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inCFGTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inCFGTRec < 0:(index = %d) ERROR!!", inCFGTRec);
                        inDISP_LogPrintf(szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open CFGT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_CFGT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnCFGTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_CFGT_FILE_NAME_);

	if (lnCFGTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnCFGTLength + 1);
        uszTemp = malloc(lnCFGTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnCFGTLength + 1);
        memset(uszTemp, 0x00, lnCFGTLength + 1);

         /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnCFGTLength;

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
         *i為目前從CFGT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnCFGTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到CFGT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                      	/* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
			inSearchResult = 1;

			/* 清空uszReadData */
                        memset(uszReadData, 0x00, lnCFGTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
			memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
			/* 因為inCFGT_Rec的index從0開始，所以inCFGT_Rec要+1 */
                        if (inRec == (inCFGTRec + 1))
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
         * 如果沒有inCFGTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inCFGTRec + 1) || inSearchResult == -1)
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
        memset(&srCFGTRec, 0x00, sizeof(srCFGTRec));
	/*
         * 以下pattern為存入CFGT_Rec
         * i為CFGT的第幾個字元
         * 存入CFGT_Rec
         */
        i = 0;


        /* 01_客製化參數 */
	/* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
	k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack1 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szCustomIndicator[0], &szCFGTRec[0], k - 1);
        }

        /* 02_NCCC FES 模式 */
	/* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack2 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szNCCCFESMode[0], &szCFGTRec[0], k - 1);
        }

        /* 03_通訊模式 */
	/* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack3 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szCommMode[0], &szCFGTRec[0], k - 1);
        }

        /* 04_撥接備援 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack4 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szDialBackupEnable[0], &szCFGTRec[0], k - 1);
        }

        /* 05_加密模式 */
	/* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack5 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szEncryptMode[0], &szCFGTRec[0], k - 1);
        }

        /* 06_不可連續刷卡檢核 */
	/* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack6 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szSplitTransCheckEnable[0], &szCFGTRec[0], k - 1);
        }

        /* 07_城市別 */
	/* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack7 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szCityName[0], &szCFGTRec[0], k - 1);
        }

        /* 08_櫃號功能 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack8 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szStoreIDEnable[0], &szCFGTRec[0], k - 1);
        }

        /* 09_櫃號輸入最短位數 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack9 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szMinStoreIDLen[0], &szCFGTRec[0], k - 1);
        }

        /* 10_櫃號輸入最長位數 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack10 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szMaxStoreIDLen[0], &szCFGTRec[0], k - 1);
        }
        
        
        /* 11_輸入櫃號的提示訊息是否由TMS控制 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack11 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szPrompt[0], &szCFGTRec[0], k - 1);
        }

        /* 12_櫃號提示訊息的內容 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack12 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szPromptData[0], &szCFGTRec[0], k - 1);
        }

        /* 13_ECR連線是否啟動 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack13 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szECREnable[0], &szCFGTRec[0], k - 1);
        }

        /* 14_ECR卡號遮掩 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack14 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szECRCardNoTruncateEnable[0], &szCFGTRec[0], k - 1);
        }

        /* 15_ECR卡片有效期回傳 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack15 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szECRExpDateReturnEnable[0], &szCFGTRec[0], k - 1);
        }

        /* 16_列印產品代碼 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack16 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szProductCodeEnable[0], &szCFGTRec[0], k - 1);
        }

        /* 17_列印商店 Slogan */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack17 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szPrtSlogan[0], &szCFGTRec[0], k - 1);
        }

        /* 18_Slogan起始日 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack18 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szSloganStartDate[0], &szCFGTRec[0], k - 1);
        }

        /* 19_Slogan停用日 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack19 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szSloganEndDate[0], &szCFGTRec[0], k - 1);
        }

        /* 20_Slogan列印位置 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack20 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szSloganPrtPosition[0], &szCFGTRec[0], k - 1);
        }

        /* 21_帳單列印模式 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack21 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szPrtMode[0], &szCFGTRec[0], k - 1);
        }

        /* 22_啟動非接觸式讀卡功能 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack22 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szContactlessEnable[0], &szCFGTRec[0], k - 1);
        }

        /* 23_接受 Visa Paywave非接觸式卡片 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack23 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szVISAPaywaveEnable[0], &szCFGTRec[0], k - 1);
        }

        /* 24_接受 JCB Jspeedy非接觸式卡片  */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack24 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szJCBJspeedyEnable[0], &szCFGTRec[0], k - 1);
        }

        /* 25_接受 MC Paypass非接觸式卡片 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack25 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szMCPaypassEnable[0], &szCFGTRec[0], k - 1);
        }

        /* 26_CUP退貨限額 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack26 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szCUPRefundLimit[0], &szCFGTRec[0], k - 1);
        }

        /* 27_CUP自動安全認證次數 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack27 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szCUPKeyExchangeTimes[0], &szCFGTRec[0], k - 1);
        }

        /* 28_交易電文是否上傳MAC驗證 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack28 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szMACEnable[0], &szCFGTRec[0], k - 1);
        }

        /* 29_密碼機模式 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack29 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szPinpadMode[0], &szCFGTRec[0], k - 1);
        }

        /* 30_CVV2功能 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack30 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szFORCECVV2[0], &szCFGTRec[0], k - 1);
        }

        /* 31_4DBC_Enable功能 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack31 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.sz4DBC[0], &szCFGTRec[0], k - 1);
        }
        
        /* 32_啟動特殊卡別參數檔功能 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack32 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szSpecialCardRangeEnable[0], &szCFGTRec[0], k - 1);
        }

        /* 33_列印商店 Logo  */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack33 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szPrtMerchantLogo[0], &szCFGTRec[0], k - 1);
        }

        /* 34_列印商店表頭  */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack34 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szPrtMerchantName[0], &szCFGTRec[0], k - 1);
        }

        /* 35_列印商店提示語 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack35 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szPrtNotice[0], &szCFGTRec[0], k - 1);
        }

        /* 36_列印促銷廣告 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack36 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szPrtPromotionAdv[0], &szCFGTRec[0], k - 1);
        }
        
        /* 37_郵購及定期性行業Indicator */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT37 unpack ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szElecCommerceFlag[0], &szCFGTRec[0], k - 1);
        }

        /* 38_DCC詢價版本 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack38 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szDccFlowVersion[0], &szCFGTRec[0], k - 1);
        }

        /* 39_DCC是否接受VISA卡 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack39 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szSupDccVisa[0], &szCFGTRec[0], k - 1);
        }

        /* 40_DCC是否接受MasterCard卡 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack40 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szSupDccMasterCard[0], &szCFGTRec[0], k - 1);
        }

        /* 41_DHCP Retry 次數 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack41 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szDHCPRetryTimes[0], &szCFGTRec[0], k - 1);
        }

        /* 42_銀行別 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack42 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szBankID[0], &szCFGTRec[0], k - 1);
        }
        
        /* 43_VEPS開關 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack43 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szVEPSFlag[0], &szCFGTRec[0], k - 1);
        }
        
        /* 44_連接埠參數 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack44 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szComportMode[0], &szCFGTRec[0], k - 1);
        }        

        /* 45_FTP模式 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack45 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szFTPMode[0], &szCFGTRec[0], k - 1);
        } 
        
        /* 46_感應設備 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack46 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szContlessMode[0], &szCFGTRec[0], k - 1);
        } 
		
        /* 47_是否開啟BarCode Reader */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack47 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szBarCodeReaderEnable[0], &szCFGTRec[0], k - 1);
        }

        /* 48_是否開啟EMV PIN Bypass功能 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack48 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szEMVPINBypassEnable[0], &szCFGTRec[0], k - 1);
        }

        /* 49_等候CUP Online PIN輸入的Time out時間 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack49 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szCUPOnlinePINEntryTimeout[0], &szCFGTRec[0], k - 1);
        }

        /* 50_簽名板模式 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack50 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szSignPadMode[0], &szCFGTRec[0], k - 1);
        }

        /* 51_有開啟簽名板功能(SignPad_Mode='1' || '2')，是否列印商店商店存根聯 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack51 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szESCPrintMerchantCopy[0], &szCFGTRec[0], k - 1);
        }

        /* 52_開始日會列印商店存根聯 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack52 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szESCPrintMerchantCopyStartDate[0], &szCFGTRec[0], k - 1);
        }

        /* 53_結束日不列印商店存根聯 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack53 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szESCPrintMerchantCopyEndDate[0], &szCFGTRec[0], k - 1);
        }

        /* 54_電子簽帳單之上水位參數 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack54 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szESCReciptUploadUpLimit[0], &szCFGTRec[0], k - 1);
        }

        /* 55_Contactless_Reader_Mode模式 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack55 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szContactlessReaderMode[0], &szCFGTRec[0], k - 1);
        }

        /* 56_端末機依此參數判斷對TMS或Download Server(FTPS)下載 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack56 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szTMSDownloadMode[0], &szCFGTRec[0], k - 1);
        }

        /* 57_接受 AMEX 非接觸式卡片(保留未來使用) */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack57 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szAMEXContactlessEnable[0], &szCFGTRec[0], k - 1);
        }

        /* 58_接受 CUP非接觸式卡片(QuickPass) */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack58 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szCUPContactlessEnable[0], &szCFGTRec[0], k - 1);
        }
	
	/* 59_接受 SmartPay非接觸式卡片 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack59 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szSmartPayContactlessEnable[0], &szCFGTRec[0], k - 1);
        }
	
	/* 60_開啟繳費項目功能 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack60 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szPay_Item_Enable[0], &szCFGTRec[0], k - 1);
        }
	
	/* 61_商店自存聯卡號遮掩 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack61 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szStore_Stub_CardNo_Truncate_Enable[0], &szCFGTRec[0], k - 1);
        }
	
	/* 62_是否為整合型週邊設備 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack62 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szIntegrate_Device[0], &szCFGTRec[0], k - 1);
        }
	
	/* 63_FES請款代碼 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack63 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szFES_ID[0], &szCFGTRec[0], k - 1);
        }
	
	/* 64_整合型周邊設備之AP ID */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack64 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szIntegrate_Device_AP_ID, &szCFGTRec[0], k - 1);
        }
	
	/* 65_短式簽單模式 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack65 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szShort_Receipt_Mode[0], &szCFGTRec[0], k - 1);
        }
	
	/* 66_I-FES(Internet Gateway)模式 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack66 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szI_FES_Mode[0], &szCFGTRec[0], k - 1);
        }
	
	/* 67_是否開啟DHCP模式 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack67 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00) /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srCFGTRec.szDHCP_Mode[0], &szCFGTRec[0], k - 1);
        }
	
	/* 68_電票優先權 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack68 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
	{
                memcpy(&srCFGTRec.szESVC_Priority[0], &szCFGTRec[0], k - 1);
        }
	
	/* 69_接受DFS非接觸式卡片 */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack69 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00)
        {
                memcpy(&srCFGTRec.szDFS_Contactless_Enable[0], &szCFGTRec[0], k - 1);
        }
	
	/* 70_是否為雲端MFES */
        /* 初始化 */
        memset(szCFGTRec, 0x00, sizeof(szCFGTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCFGTRec[k ++] = uszReadData[i ++];
                if (szCFGTRec[k - 1] == 0x2C	||
		    szCFGTRec[k - 1] == 0x0D	||
		    szCFGTRec[k - 1] == 0x0A	||
		    szCFGTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCFGTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("CFGT unpack70 ERROR");
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
        if (szCFGTRec[0] != 0x2C	&&
	    szCFGTRec[0] != 0x0D	&&
	    szCFGTRec[0] != 0x0A	&&
	    szCFGTRec[0] != 0x00) /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srCFGTRec.szCloud_MFES[0], &szCFGTRec[0], k - 1);
        }

        /* release */
	/* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadCFGTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadCFGTRec(%d) END!!", inCFGTRec);
                inDISP_LogPrintf(szErrorMsg);
                inDISP_LogPrintf("----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveCFGTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveCFGTRec(int inCFGTRec)
{
	unsigned long   uldat_Handle;   			/* FILE Handle */
	int inRecCount = 0, inTempIndex = 0;
	int     	inRetVal;
	int     	i = 0;

	char szTempTitle[] = "inSaveCFGTRec";
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
	
	inTempIndex = inCFGTRec;
	
	memset(szTempFIleName, 0x00, sizeof(szTempFIleName));
	strcat( szTempFIleName, _CFGT_FILE_NAME_);
	
	memset(szTempBakFileName, 0x00, sizeof(szTempBakFileName));
	strcat( szTempBakFileName, _CFGT_FILE_NAME_BAK_);
	
	inDISP_LogPrintfWithFlag("  %s Index[%d] ", szTempTitle, inTempIndex);

      	/* 開啟原檔案HDPT.dat */
	inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_CFGT_FILE_NAME_);

	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  %s *Error* [%d] ",szTempTitle, inRetVal);
		inFunc_EDCLock();
		return(VS_ERROR);
	}

	usRetVal = CTOS_FileGetSize((unsigned char *)_CFGT_FILE_NAME_, &ulFileSize);	
	
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
	uszWriteBuff_Record = malloc(_SIZE_CFGT_REC_ + _SIZE_CFGT_COMMA_0D0A_);
	memset(uszWriteBuff_Record, 0x00, _SIZE_CFGT_REC_ + _SIZE_CFGT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

	/* uszRead_Total_Buff儲存CFGT.dat全部資料 */
	uszRead_Total_Buff = malloc(ulFileSize + 1);
	memset(uszRead_Total_Buff, 0x00, ulFileSize + 1);

	ulPackCount = 0; /* uszWriteBuff_Record的index位置 */
	/* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* 1.CustomIndicator */
	memcpy(&uszWriteBuff_Record[0], &srCFGTRec.szCustomIndicator[0], strlen(srCFGTRec.szCustomIndicator));
	ulPackCount += strlen(srCFGTRec.szCustomIndicator);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 2.NCCCFESMode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szNCCCFESMode[0], strlen(srCFGTRec.szNCCCFESMode));
	ulPackCount += strlen(srCFGTRec.szNCCCFESMode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 3.CommMode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szCommMode[0], strlen(srCFGTRec.szCommMode));
	ulPackCount += strlen(srCFGTRec.szCommMode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 4.DialBackupEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szDialBackupEnable[0], strlen(srCFGTRec.szDialBackupEnable));
	ulPackCount += strlen(srCFGTRec.szDialBackupEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 5.EncryptMode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szEncryptMode[0], strlen(srCFGTRec.szEncryptMode));
	ulPackCount += strlen(srCFGTRec.szEncryptMode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 6.SplitTransCheckEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szSplitTransCheckEnable[0], strlen(srCFGTRec.szSplitTransCheckEnable));
	ulPackCount += strlen(srCFGTRec.szSplitTransCheckEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 7.CityName */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szCityName[0], strlen(srCFGTRec.szCityName));
	ulPackCount += strlen(srCFGTRec.szCityName);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 8.StoreIDEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szStoreIDEnable[0], strlen(srCFGTRec.szStoreIDEnable));
	ulPackCount += strlen(srCFGTRec.szStoreIDEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 9.MinStoreIDLen */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szMinStoreIDLen[0], strlen(srCFGTRec.szMinStoreIDLen));
	ulPackCount += strlen(srCFGTRec.szMinStoreIDLen);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 10.MaxStoreIDLen */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szMaxStoreIDLen[0], strlen(srCFGTRec.szMaxStoreIDLen));
	ulPackCount += strlen(srCFGTRec.szMaxStoreIDLen);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 11.Prompt(Nexsys) */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szPrompt[0], strlen(srCFGTRec.szPrompt));
	ulPackCount += strlen(srCFGTRec.szPrompt);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 12.PromptData(Nexsys) */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szPromptData[0], strlen(srCFGTRec.szPromptData));
	ulPackCount += strlen(srCFGTRec.szPromptData);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 13.ECREnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szECREnable[0], strlen(srCFGTRec.szECREnable));
	ulPackCount += strlen(srCFGTRec.szECREnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 14.ECRCardNoTruncateEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szECRCardNoTruncateEnable[0], strlen(srCFGTRec.szECRCardNoTruncateEnable));
	ulPackCount += strlen(srCFGTRec.szECRCardNoTruncateEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 15.ECRExpDateReturnEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szECRExpDateReturnEnable[0], strlen(srCFGTRec.szECRExpDateReturnEnable));
	ulPackCount += strlen(srCFGTRec.szECRExpDateReturnEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 16.ProductCodeEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szProductCodeEnable[0], strlen(srCFGTRec.szProductCodeEnable));
	ulPackCount += strlen(srCFGTRec.szProductCodeEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 17.PrtSlogan */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szPrtSlogan[0], strlen(srCFGTRec.szPrtSlogan));
	ulPackCount += strlen(srCFGTRec.szPrtSlogan);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 18.SloganStartDate */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szSloganStartDate[0], strlen(srCFGTRec.szSloganStartDate));
	ulPackCount += strlen(srCFGTRec.szSloganStartDate);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 19.SloganEndDate */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szSloganEndDate[0], strlen(srCFGTRec.szSloganEndDate));
	ulPackCount += strlen(srCFGTRec.szSloganEndDate);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 20.SloganPrtPosition */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szSloganPrtPosition[0], strlen(srCFGTRec.szSloganPrtPosition));
	ulPackCount += strlen(srCFGTRec.szSloganPrtPosition);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 21.PrtMode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szPrtMode[0], strlen(srCFGTRec.szPrtMode));
	ulPackCount += strlen(srCFGTRec.szPrtMode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 22.ContactlessEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szContactlessEnable[0], strlen(srCFGTRec.szContactlessEnable));
	ulPackCount += strlen(srCFGTRec.szContactlessEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 23.VISAPaywaveEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szVISAPaywaveEnable[0], strlen(srCFGTRec.szVISAPaywaveEnable));
	ulPackCount += strlen(srCFGTRec.szVISAPaywaveEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 24.JCBJspeedyEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szJCBJspeedyEnable[0], strlen(srCFGTRec.szJCBJspeedyEnable));
	ulPackCount += strlen(srCFGTRec.szJCBJspeedyEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 25.MCPaypassEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szMCPaypassEnable[0], strlen(srCFGTRec.szMCPaypassEnable));
	ulPackCount += strlen(srCFGTRec.szMCPaypassEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 26.CUPRefundLimit */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szCUPRefundLimit[0], strlen(srCFGTRec.szCUPRefundLimit));
	ulPackCount += strlen(srCFGTRec.szCUPRefundLimit);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 27.CUPKeyExchangeTimes */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szCUPKeyExchangeTimes[0], strlen(srCFGTRec.szCUPKeyExchangeTimes));
	ulPackCount += strlen(srCFGTRec.szCUPKeyExchangeTimes);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 28.MACEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szMACEnable[0], strlen(srCFGTRec.szMACEnable));
	ulPackCount += strlen(srCFGTRec.szMACEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 29.PinpadMode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szPinpadMode[0], strlen(srCFGTRec.szPinpadMode));
	ulPackCount += strlen(srCFGTRec.szPinpadMode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 30.FORCECVV2 */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szFORCECVV2[0], strlen(srCFGTRec.szFORCECVV2));
	ulPackCount += strlen(srCFGTRec.szFORCECVV2);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 31.4DBCEnable(Nexsys) */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.sz4DBC[0], strlen(srCFGTRec.sz4DBC));
	ulPackCount += strlen(srCFGTRec.sz4DBC);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 32.SpecialCardRangeEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szSpecialCardRangeEnable[0], strlen(srCFGTRec.szSpecialCardRangeEnable));
	ulPackCount += strlen(srCFGTRec.szSpecialCardRangeEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 33.PrtMerchantLogo */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szPrtMerchantLogo[0], strlen(srCFGTRec.szPrtMerchantLogo));
	ulPackCount += strlen(srCFGTRec.szPrtMerchantLogo);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 34.PrtMerchantName */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szPrtMerchantName[0], strlen(srCFGTRec.szPrtMerchantName));
	ulPackCount += strlen(srCFGTRec.szPrtMerchantName);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 35.PrtNotice */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szPrtNotice[0], strlen(srCFGTRec.szPrtNotice));
	ulPackCount += strlen(srCFGTRec.szPrtNotice);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 36.PrtPromotionAdv(Nexsys) */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szPrtPromotionAdv[0], strlen(srCFGTRec.szPrtPromotionAdv));
	ulPackCount += strlen(srCFGTRec.szPrtPromotionAdv);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 37.ElecCommerceFlag */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szElecCommerceFlag[0], strlen(srCFGTRec.szElecCommerceFlag));
	ulPackCount += strlen(srCFGTRec.szElecCommerceFlag);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 38.DccFlowVersion */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szDccFlowVersion[0], strlen(srCFGTRec.szDccFlowVersion));
	ulPackCount += strlen(srCFGTRec.szDccFlowVersion);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 39.SupDccVisa */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szSupDccVisa[0], strlen(srCFGTRec.szSupDccVisa));
	ulPackCount += strlen(srCFGTRec.szSupDccVisa);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 40.SupDccMasterCard */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szSupDccMasterCard[0], strlen(srCFGTRec.szSupDccMasterCard));
	ulPackCount += strlen(srCFGTRec.szSupDccMasterCard);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 41.DHCPRetryTimes */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szDHCPRetryTimes[0], strlen(srCFGTRec.szDHCPRetryTimes));
	ulPackCount += strlen(srCFGTRec.szDHCPRetryTimes);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 42.BANK_Indicator(Nexsys) */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szBankID[0], strlen(srCFGTRec.szBankID));
	ulPackCount += strlen(srCFGTRec.szBankID);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 43.VEPSFlag(Nexsys) */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szVEPSFlag[0], strlen(srCFGTRec.szVEPSFlag));
	ulPackCount += strlen(srCFGTRec.szVEPSFlag);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 44.ComportMode(Nexsys) */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szComportMode[0], strlen(srCFGTRec.szComportMode));
	ulPackCount += strlen(srCFGTRec.szComportMode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 45.FTPMode(Nexsys) */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szFTPMode[0], strlen(srCFGTRec.szFTPMode));
	ulPackCount += strlen(srCFGTRec.szFTPMode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 46.ContlessMode(Nexsys) */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szContlessMode[0], strlen(srCFGTRec.szContlessMode));
	ulPackCount += strlen(srCFGTRec.szContlessMode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* BarCodeReaderEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szBarCodeReaderEnable[0], strlen(srCFGTRec.szBarCodeReaderEnable));
	ulPackCount += strlen(srCFGTRec.szBarCodeReaderEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* EMVPINBypassEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szEMVPINBypassEnable[0], strlen(srCFGTRec.szEMVPINBypassEnable));
	ulPackCount += strlen(srCFGTRec.szEMVPINBypassEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* CUPOnlinePINEntryTimeout */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szCUPOnlinePINEntryTimeout[0], strlen(srCFGTRec.szCUPOnlinePINEntryTimeout));
	ulPackCount += strlen(srCFGTRec.szCUPOnlinePINEntryTimeout);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* SignPadMode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szSignPadMode[0], strlen(srCFGTRec.szSignPadMode));
	ulPackCount += strlen(srCFGTRec.szSignPadMode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* ESCPrintMerchantCopy */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szESCPrintMerchantCopy[0], strlen(srCFGTRec.szESCPrintMerchantCopy));
	ulPackCount += strlen(srCFGTRec.szESCPrintMerchantCopy);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* ESCPrintMerchantCopyStartDate */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szESCPrintMerchantCopyStartDate[0], strlen(srCFGTRec.szESCPrintMerchantCopyStartDate));
	ulPackCount += strlen(srCFGTRec.szESCPrintMerchantCopyStartDate);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* ESCPrintMerchantCopyEndDate */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szESCPrintMerchantCopyEndDate[0], strlen(srCFGTRec.szESCPrintMerchantCopyEndDate));
	ulPackCount += strlen(srCFGTRec.szESCPrintMerchantCopyEndDate);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* ESCReciptUploadUpLimit */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szESCReciptUploadUpLimit[0], strlen(srCFGTRec.szESCReciptUploadUpLimit));
	ulPackCount += strlen(srCFGTRec.szESCReciptUploadUpLimit);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* ContactlessReaderMode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szContactlessReaderMode[0], strlen(srCFGTRec.szContactlessReaderMode));
	ulPackCount += strlen(srCFGTRec.szContactlessReaderMode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TMSDownloadMode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szTMSDownloadMode[0], strlen(srCFGTRec.szTMSDownloadMode));
	ulPackCount += strlen(srCFGTRec.szTMSDownloadMode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* AMEXContactlessEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szAMEXContactlessEnable[0], strlen(srCFGTRec.szAMEXContactlessEnable));
	ulPackCount += strlen(srCFGTRec.szAMEXContactlessEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* CUPContactlessEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szCUPContactlessEnable[0], strlen(srCFGTRec.szCUPContactlessEnable));
	ulPackCount += strlen(srCFGTRec.szCUPContactlessEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* SmartPayContactlessEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szSmartPayContactlessEnable[0], strlen(srCFGTRec.szSmartPayContactlessEnable));
	ulPackCount += strlen(srCFGTRec.szSmartPayContactlessEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* Pay_Item_Enable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szPay_Item_Enable[0], strlen(srCFGTRec.szPay_Item_Enable));
	ulPackCount += strlen(srCFGTRec.szPay_Item_Enable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* Store_Stub_CardNo_Truncate_Enable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szStore_Stub_CardNo_Truncate_Enable[0], strlen(srCFGTRec.szStore_Stub_CardNo_Truncate_Enable));
	ulPackCount += strlen(srCFGTRec.szStore_Stub_CardNo_Truncate_Enable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* Integrate_Device */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szIntegrate_Device[0], strlen(srCFGTRec.szIntegrate_Device));
	ulPackCount += strlen(srCFGTRec.szIntegrate_Device);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* FES_ID */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szFES_ID[0], strlen(srCFGTRec.szFES_ID));
	ulPackCount += strlen(srCFGTRec.szFES_ID);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* Integrate_Device_AP_ID */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szIntegrate_Device_AP_ID[0], strlen(srCFGTRec.szIntegrate_Device_AP_ID));
	ulPackCount += strlen(srCFGTRec.szIntegrate_Device_AP_ID);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* Short_Receipt_Mode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szShort_Receipt_Mode[0], strlen(srCFGTRec.szShort_Receipt_Mode));
	ulPackCount += strlen(srCFGTRec.szShort_Receipt_Mode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* I_FES_Mode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szI_FES_Mode[0], strlen(srCFGTRec.szI_FES_Mode));
	ulPackCount += strlen(srCFGTRec.szI_FES_Mode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* DHCP_Mode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szDHCP_Mode[0], strlen(srCFGTRec.szDHCP_Mode));
	ulPackCount += strlen(srCFGTRec.szDHCP_Mode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* ESVC_Priority */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szESVC_Priority[0], strlen(srCFGTRec.szESVC_Priority));
	ulPackCount += strlen(srCFGTRec.szESVC_Priority);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* DFS_Contactless_Enable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szDFS_Contactless_Enable[0], strlen(srCFGTRec.szDFS_Contactless_Enable));
	ulPackCount += strlen(srCFGTRec.szDFS_Contactless_Enable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* Cloud_MFES */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srCFGTRec.szCloud_MFES[0], strlen(srCFGTRec.szCloud_MFES));
	ulPackCount += strlen(srCFGTRec.szCloud_MFES);

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
	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inSaveCFGTRec END");
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
Function        :inGetCustomIndicator
Date&Time       :
Describe        :
*/
int inGetCustomIndicator(char* szCustomIndicator)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCustomIndicator == NULL || strlen(srCFGTRec.szCustomIndicator) <= 0 || strlen(srCFGTRec.szCustomIndicator) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCustomIndicator() ERROR !!");

                        if (szCustomIndicator == NULL)
                        {
                                inDISP_LogPrintf("szCustomIndicator == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCustomIndicator length = (%d)", (int)strlen(srCFGTRec.szCustomIndicator));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }
                return (VS_ERROR);
        }
        memcpy(&szCustomIndicator[0], &srCFGTRec.szCustomIndicator[0], strlen(srCFGTRec.szCustomIndicator));

        return (VS_SUCCESS);
}

/*
Function        :inSetCustomIndicator
Date&Time       :
Describe        :
*/
int inSetCustomIndicator(char* szCustomIndicator)
{
        memset(srCFGTRec.szCustomIndicator, 0x00, sizeof(srCFGTRec.szCustomIndicator));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCustomIndicator == NULL || strlen(szCustomIndicator) <= 0 || strlen(szCustomIndicator) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCustomIndicator() ERROR !!");

                        if (szCustomIndicator == NULL)
                        {
                                inDISP_LogPrintf("szCustomIndicator == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCustomIndicator length = (%d)", (int)strlen(szCustomIndicator));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szCustomIndicator[0], &szCustomIndicator[0], strlen(szCustomIndicator));

        return (VS_SUCCESS);
}

/*
Function        :inGetNCCCFESMode
Date&Time       :
Describe        :
*/
int inGetNCCCFESMode(char* szNCCCFESMode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szNCCCFESMode == NULL || strlen(srCFGTRec.szNCCCFESMode) <= 0 || strlen(srCFGTRec.szNCCCFESMode) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetNCCCFESMode() ERROR !!");

                        if (szNCCCFESMode == NULL)
                        {
                                inDISP_LogPrintf("szNCCCFESMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szNCCCFESMode length = (%d)", (int)strlen(srCFGTRec.szNCCCFESMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szNCCCFESMode[0], &srCFGTRec.szNCCCFESMode[0], strlen(srCFGTRec.szNCCCFESMode));

        return (VS_SUCCESS);

}

/*
Function        :inSetNCCCFESMode
Date&Time       :
Describe        :
*/
int inSetNCCCFESMode(char* szNCCCFESMode)
{
        memset(srCFGTRec.szNCCCFESMode, 0x00, sizeof(srCFGTRec.szNCCCFESMode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szNCCCFESMode == NULL || strlen(szNCCCFESMode) <= 0 || strlen(szNCCCFESMode) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetNCCCFESMode() ERROR !!");

                        if (szNCCCFESMode == NULL)
                        {
                                inDISP_LogPrintf("szNCCCFESMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szNCCCFESMode length = (%d)", (int)strlen(szNCCCFESMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szNCCCFESMode[0], &szNCCCFESMode[0], strlen(szNCCCFESMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetCommMode
Date&Time       :
Describe        :
*/
int inGetCommMode(char* szCommMode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCommMode == NULL || strlen(srCFGTRec.szCommMode) <= 0 || strlen(srCFGTRec.szCommMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCommMode() ERROR !!");

                        if (szCommMode == NULL)
                        {
                                inDISP_LogPrintf("szCommMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCommMode length = (%d)", (int)strlen(srCFGTRec.szCommMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCommMode[0], &srCFGTRec.szCommMode[0], strlen(srCFGTRec.szCommMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetCommMode
Date&Time       :
Describe        :
*/
int inSetCommMode(char* szCommMode)
{
        memset(srCFGTRec.szCommMode, 0x00, sizeof(srCFGTRec.szCommMode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCommMode == NULL || strlen(szCommMode) <= 0 || strlen(szCommMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCommMode() ERROR !!");

                        if (szCommMode == NULL)
                        {
                                inDISP_LogPrintf("szCommMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCommMode length = (%d)", (int)strlen(szCommMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szCommMode[0], &szCommMode[0], strlen(szCommMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetDialBackupEnable
Date&Time       :
Describe        :
*/
int inGetDialBackupEnable(char* szDialBackupEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szDialBackupEnable == NULL || strlen(srCFGTRec.szDialBackupEnable) <= 0 || strlen(srCFGTRec.szDialBackupEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetDialBackupEnable() ERROR !!");

                        if (szDialBackupEnable == NULL)
                        {
                                inDISP_LogPrintf("szDialBackupEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDialBackupEnable length = (%d)", (int)strlen(srCFGTRec.szDialBackupEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDialBackupEnable[0], &srCFGTRec.szDialBackupEnable[0], strlen(srCFGTRec.szDialBackupEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetDialBackupEnable
Date&Time       :
Describe        :
*/
int inSetDialBackupEnable(char* szDialBackupEnable)
{
        memset(srCFGTRec.szDialBackupEnable, 0x00, sizeof(srCFGTRec.szDialBackupEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szDialBackupEnable == NULL || strlen(szDialBackupEnable) <= 0 || strlen(szDialBackupEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetDialBackupEnable() ERROR !!");

                        if (szDialBackupEnable == NULL)
                        {
                                inDISP_LogPrintf("szDialBackupEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDialBackupEnable length = (%d)", (int)strlen(szDialBackupEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szDialBackupEnable[0], &szDialBackupEnable[0], strlen(szDialBackupEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetEncryptMode
Date&Time       :
Describe        :
*/
int inGetEncryptMode(char* szEncryptMode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szEncryptMode == NULL || strlen(srCFGTRec.szEncryptMode) <= 0 || strlen(srCFGTRec.szEncryptMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetEncryptMode() ERROR !!");

                        if (szEncryptMode == NULL)
                        {
                                inDISP_LogPrintf("szEncryptMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEncryptMode length = (%d)", (int)strlen(srCFGTRec.szEncryptMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szEncryptMode[0], &srCFGTRec.szEncryptMode[0], strlen(srCFGTRec.szEncryptMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetEncryptMode
Date&Time       :
Describe        :
*/
int inSetEncryptMode(char* szEncryptMode)
{
        memset(srCFGTRec.szEncryptMode, 0x00, sizeof(srCFGTRec.szEncryptMode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szEncryptMode == NULL || strlen(szEncryptMode) <= 0 || strlen(szEncryptMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetEncryptMode() ERROR !!");

                        if (szEncryptMode == NULL)
                        {
                                inDISP_LogPrintf("szEncryptMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEncryptMode length = (%d)", (int)strlen(szEncryptMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szEncryptMode[0], &szEncryptMode[0], strlen(szEncryptMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetSplitTransCheckEnable
Date&Time       :
Describe        :
*/
int inGetSplitTransCheckEnable(char* szSplitTransCheckEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szSplitTransCheckEnable == NULL || strlen(srCFGTRec.szSplitTransCheckEnable) <= 0 || strlen(srCFGTRec.szSplitTransCheckEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetSplitTransCheckEnable() ERROR !!");

                        if (szSplitTransCheckEnable == NULL)
                        {
                                inDISP_LogPrintf("szSplitTransCheckEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSplitTransCheckEnable length = (%d)", (int)strlen(srCFGTRec.szSplitTransCheckEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSplitTransCheckEnable[0], &srCFGTRec.szSplitTransCheckEnable[0], strlen(srCFGTRec.szSplitTransCheckEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetSplitTransCheckEnable
Date&Time       :
Describe        :
*/
int inSetSplitTransCheckEnable(char* szSplitTransCheckEnable)
{
        memset(srCFGTRec.szSplitTransCheckEnable, 0x00, sizeof(srCFGTRec.szSplitTransCheckEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSplitTransCheckEnable == NULL || strlen(szSplitTransCheckEnable) <= 0 || strlen(szSplitTransCheckEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetSplitTransCheckEnable() ERROR !!");

                        if (szSplitTransCheckEnable == NULL)
                        {
                                inDISP_LogPrintf("szSplitTransCheckEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSplitTransCheckEnable length = (%d)", (int)strlen(szSplitTransCheckEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szSplitTransCheckEnable[0], &szSplitTransCheckEnable[0], strlen(szSplitTransCheckEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetCityName
Date&Time       :
Describe        :
*/
int inGetCityName(char* szCityName)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCityName == NULL || strlen(srCFGTRec.szCityName) <= 0 || strlen(srCFGTRec.szCityName) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCityName() ERROR !!");

                        if (szCityName == NULL)
                        {
                                inDISP_LogPrintf("szCityName == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCityName length = (%d)", (int)strlen(srCFGTRec.szCityName));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCityName[0], &srCFGTRec.szCityName[0], strlen(srCFGTRec.szCityName));

        return (VS_SUCCESS);
}

/*
Function        :inSetCityName
Date&Time       :
Describe        :
*/
int inSetCityName(char* szCityName)
{
        memset(srCFGTRec.szCityName, 0x00, sizeof(srCFGTRec.szCityName));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCityName == NULL || strlen(szCityName) <= 0 || strlen(szCityName) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCityName() ERROR !!");

                        if (szCityName == NULL)
                        {
                                inDISP_LogPrintf("szCityName == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCityName length = (%d)", (int)strlen(szCityName));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szCityName[0], &szCityName[0], strlen(szCityName));

        return (VS_SUCCESS);
}

/*
Function        :inGetStoreIDEnable
Date&Time       :
Describe        :
*/
int inGetStoreIDEnable(char* szStoreIDEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szStoreIDEnable == NULL || strlen(srCFGTRec.szStoreIDEnable) <= 0 || strlen(srCFGTRec.szStoreIDEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetStoreIDEnable() ERROR !!");

                        if (szStoreIDEnable == NULL)
                        {
                                inDISP_LogPrintf("szStoreIDEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szStoreIDEnable length = (%d)", (int)strlen(srCFGTRec.szStoreIDEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szStoreIDEnable[0], &srCFGTRec.szStoreIDEnable[0], strlen(srCFGTRec.szStoreIDEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetStoreIDEnable
Date&Time       :
Describe        :
*/
int inSetStoreIDEnable(char* szStoreIDEnable)
{
        memset(srCFGTRec.szStoreIDEnable, 0x00, sizeof(srCFGTRec.szStoreIDEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szStoreIDEnable == NULL || strlen(szStoreIDEnable) <= 0 || strlen(szStoreIDEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetStoreIDEnable() ERROR !!");

                        if (szStoreIDEnable == NULL)
                        {
                                inDISP_LogPrintf("szStoreIDEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szStoreIDEnable length = (%d)", (int)strlen(szStoreIDEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szStoreIDEnable[0], &szStoreIDEnable[0], strlen(szStoreIDEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetMinStoreIDLen
Date&Time       :
Describe        :
*/
int inGetMinStoreIDLen(char* szMinStoreIDLen)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szMinStoreIDLen == NULL || strlen(srCFGTRec.szMinStoreIDLen) <= 0 || strlen(srCFGTRec.szMinStoreIDLen) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetMinStoreIDLen() ERROR !!");

                        if (szMinStoreIDLen == NULL)
                        {
                                inDISP_LogPrintf("szMinStoreIDLen == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMinStoreIDLen length = (%d)", (int)strlen(srCFGTRec.szMinStoreIDLen));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMinStoreIDLen[0], &srCFGTRec.szMinStoreIDLen[0], strlen(srCFGTRec.szMinStoreIDLen));

        return (VS_SUCCESS);
}

/*
Function        :inSetMinStoreIDLen
Date&Time       :
Describe        :
*/
int inSetMinStoreIDLen(char* szMinStoreIDLen)
{
        memset(srCFGTRec.szMinStoreIDLen, 0x00, sizeof(srCFGTRec.szMinStoreIDLen));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szMinStoreIDLen == NULL || strlen(szMinStoreIDLen) <= 0 || strlen(szMinStoreIDLen) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetMinStoreIDLen() ERROR !!");

                        if (szMinStoreIDLen == NULL)
                        {
                                inDISP_LogPrintf("szMinStoreIDLen == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMinStoreIDLen length = (%d)", (int)strlen(szMinStoreIDLen));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szMinStoreIDLen[0], &szMinStoreIDLen[0], strlen(szMinStoreIDLen));

        return (VS_SUCCESS);
}

/*
Function        :inGetMaxStoreIDLen
Date&Time       :
Describe        :
*/
int inGetMaxStoreIDLen(char* szMaxStoreIDLen)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szMaxStoreIDLen == NULL || strlen(srCFGTRec.szMaxStoreIDLen) <= 0 || strlen(srCFGTRec.szMaxStoreIDLen) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetMaxStoreIDLen() ERROR !!");

                        if (szMaxStoreIDLen == NULL)
                        {
                                inDISP_LogPrintf("szMaxStoreIDLen == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMaxStoreIDLen length = (%d)", (int)strlen(srCFGTRec.szMaxStoreIDLen));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMaxStoreIDLen[0], &srCFGTRec.szMaxStoreIDLen[0], strlen(srCFGTRec.szMaxStoreIDLen));

        return (VS_SUCCESS);
}

/*
Function        :inSetMaxStoreIDLen
Date&Time       :
Describe        :
*/
int inSetMaxStoreIDLen(char* szMaxStoreIDLen)
{
        memset(srCFGTRec.szMaxStoreIDLen, 0x00, sizeof(srCFGTRec.szMaxStoreIDLen));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szMaxStoreIDLen == NULL || strlen(szMaxStoreIDLen) <= 0 || strlen(szMaxStoreIDLen) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetMaxStoreIDLen() ERROR !!");

                        if (szMaxStoreIDLen == NULL)
                        {
                                inDISP_LogPrintf("szMaxStoreIDLen == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMaxStoreIDLen length = (%d)", (int)strlen(szMaxStoreIDLen));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szMaxStoreIDLen[0], &szMaxStoreIDLen[0], strlen(szMaxStoreIDLen));

        return (VS_SUCCESS);
}

/*
Function        :inGetPrompt
Date&Time       :
Describe        :
*/
int inGetPrompt(char* szPrompt)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPrompt == NULL || strlen(srCFGTRec.szPrompt) <= 0 || strlen(srCFGTRec.szPrompt) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPrompt() ERROR !!");

                        if (szPrompt == NULL)
                        {
                                inDISP_LogPrintf("szPrompt == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrompt length = (%d)", (int)strlen(srCFGTRec.szPrompt));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPrompt[0], &srCFGTRec.szPrompt[0], strlen(srCFGTRec.szPrompt));

        return (VS_SUCCESS);
}

/*
Function        :inSetPrompt
Date&Time       :
Describe        :
*/
int inSetPrompt(char* szPrompt)
{
        memset(srCFGTRec.szPrompt, 0x00, sizeof(srCFGTRec.szPrompt));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPrompt == NULL || strlen(szPrompt) <= 0 || strlen(szPrompt) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPrompt() ERROR !!");

                        if (szPrompt == NULL)
                        {
                                inDISP_LogPrintf("szPrompt == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrompt length = (%d)", (int)strlen(szPrompt));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szPrompt[0], &szPrompt[0], strlen(szPrompt));

        return (VS_SUCCESS);
}

/*
Function        :inGetPromptData
Date&Time       :
Describe        :
*/
int inGetPromptData(char* szPromptData)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPromptData == NULL || strlen(srCFGTRec.szPromptData) <= 0 || strlen(srCFGTRec.szPromptData) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPromptData() ERROR !!");

                        if (szPromptData == NULL)
                        {
                                inDISP_LogPrintf("szPromptData == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPromptData length = (%d)", (int)strlen(srCFGTRec.szPromptData));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPromptData[0], &srCFGTRec.szPromptData[0], strlen(srCFGTRec.szPromptData));

        return (VS_SUCCESS);
}

/*
Function        :inSetPromptData
Date&Time       :
Describe        :
*/
int inSetPromptData(char* szPromptData)
{
        memset(srCFGTRec.szPromptData, 0x00, sizeof(srCFGTRec.szPromptData));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPromptData == NULL || strlen(szPromptData) <= 0 || strlen(szPromptData) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPromptData() ERROR !!");

                        if (szPromptData == NULL)
                        {
                                inDISP_LogPrintf("szPromptData == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPromptData length = (%d)", (int)strlen(szPromptData));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szPromptData[0], &szPromptData[0], strlen(szPromptData));

        return (VS_SUCCESS);
}

/*
Function        :inGetECREnable
Date&Time       :
Describe        :
*/
int inGetECREnable(char* szECREnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szECREnable == NULL || strlen(srCFGTRec.szECREnable) <= 0 || strlen(srCFGTRec.szECREnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetECREnable() ERROR !!");

                        if (szECREnable == NULL)
                        {
                                inDISP_LogPrintf("szECREnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECREnable length = (%d)", (int)strlen(srCFGTRec.szECREnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szECREnable[0], &srCFGTRec.szECREnable[0], strlen(srCFGTRec.szECREnable));

        return (VS_SUCCESS);

}

/*
Function        :inSetECREnable
Date&Time       :
Describe        :
*/
int inSetECREnable(char* szECREnable)
{
        memset(srCFGTRec.szECREnable, 0x00, sizeof(srCFGTRec.szECREnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szECREnable == NULL || strlen(szECREnable) <= 0 || strlen(szECREnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetECREnable() ERROR !!");

                        if (szECREnable == NULL)
                        {
                                inDISP_LogPrintf("szECREnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECREnable length = (%d)", (int)strlen(szECREnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szECREnable[0], &szECREnable[0], strlen(szECREnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetECRCardNoTruncateEnable
Date&Time       :
Describe        :
*/
int inGetECRCardNoTruncateEnable(char* szECRCardNoTruncateEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szECRCardNoTruncateEnable == NULL || strlen(srCFGTRec.szECRCardNoTruncateEnable) <= 0 || strlen(srCFGTRec.szECRCardNoTruncateEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetECRCardNoTruncateEnable() ERROR !!");

                        if (szECRCardNoTruncateEnable == NULL)
                        {
                                inDISP_LogPrintf("szECRCardNoTruncateEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECRCardNoTruncateEnable length = (%d)", (int)strlen(srCFGTRec.szECRCardNoTruncateEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szECRCardNoTruncateEnable[0], &srCFGTRec.szECRCardNoTruncateEnable[0], strlen(srCFGTRec.szECRCardNoTruncateEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetECRCardNoTruncateEnable
Date&Time       :
Describe        :
*/
int inSetECRCardNoTruncateEnable(char* szECRCardNoTruncateEnable)
{
        memset(srCFGTRec.szECRCardNoTruncateEnable, 0x00, sizeof(srCFGTRec.szECRCardNoTruncateEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szECRCardNoTruncateEnable == NULL || strlen(szECRCardNoTruncateEnable) <= 0 || strlen(szECRCardNoTruncateEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetECRCardNoTruncateEnable() ERROR !!");

                        if (szECRCardNoTruncateEnable == NULL)
                        {
                                inDISP_LogPrintf("szECRCardNoTruncateEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECRCardNoTruncateEnable length = (%d)", (int)strlen(szECRCardNoTruncateEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szECRCardNoTruncateEnable[0], &szECRCardNoTruncateEnable[0], strlen(szECRCardNoTruncateEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetECRExpDateReturnEnable
Date&Time       :
Describe        :
*/
int inGetECRExpDateReturnEnable(char* szECRExpDateReturnEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szECRExpDateReturnEnable == NULL || strlen(srCFGTRec.szECRExpDateReturnEnable) <= 0 || strlen(srCFGTRec.szECRExpDateReturnEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetECRExpDateReturnEnable() ERROR !!");

                        if (szECRExpDateReturnEnable == NULL)
                        {
                                inDISP_LogPrintf("szECRExpDateReturnEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECRExpDateReturnEnable length = (%d)", (int)strlen(srCFGTRec.szECRExpDateReturnEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szECRExpDateReturnEnable[0], &srCFGTRec.szECRExpDateReturnEnable[0], strlen(srCFGTRec.szECRExpDateReturnEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetECRExpDateReturnEnable
Date&Time       :
Describe        :
*/
int inSetECRExpDateReturnEnable(char* szECRExpDateReturnEnable)
{
        memset(srCFGTRec.szECRExpDateReturnEnable, 0x00, sizeof(srCFGTRec.szECRExpDateReturnEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szECRExpDateReturnEnable == NULL || strlen(szECRExpDateReturnEnable) <= 0 || strlen(szECRExpDateReturnEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetECRExpDateReturnEnable() ERROR !!");

                        if (szECRExpDateReturnEnable == NULL)
                        {
                                inDISP_LogPrintf("szECRExpDateReturnEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECRExpDateReturnEnable length = (%d)", (int)strlen(szECRExpDateReturnEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szECRExpDateReturnEnable[0], &szECRExpDateReturnEnable[0], strlen(szECRExpDateReturnEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetProductCodeEnable
Date&Time       :
Describe        :
*/
int inGetProductCodeEnable(char* szProductCodeEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szProductCodeEnable == NULL || strlen(srCFGTRec.szProductCodeEnable) <= 0 || strlen(srCFGTRec.szProductCodeEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetProductCodeEnable() ERROR !!");

                        if (szProductCodeEnable == NULL)
                        {
                                inDISP_LogPrintf("szProductCodeEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szProductCodeEnable length = (%d)", (int)strlen(srCFGTRec.szProductCodeEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szProductCodeEnable[0], &srCFGTRec.szProductCodeEnable[0], strlen(srCFGTRec.szProductCodeEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetProductCodeEnable
Date&Time       :
Describe        :
*/
int inSetProductCodeEnable(char* szProductCodeEnable)
{
        memset(srCFGTRec.szProductCodeEnable, 0x00, sizeof(srCFGTRec.szProductCodeEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szProductCodeEnable == NULL || strlen(szProductCodeEnable) <= 0 || strlen(szProductCodeEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetProductCodeEnable() ERROR !!");

                        if (szProductCodeEnable == NULL)
                        {
                                inDISP_LogPrintf("szProductCodeEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szProductCodeEnable length = (%d)", (int)strlen(szProductCodeEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szProductCodeEnable[0], &szProductCodeEnable[0], strlen(szProductCodeEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetPrtSlogan
Date&Time       :
Describe        :
*/
int inGetPrtSlogan(char* szPrtSlogan)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPrtSlogan == NULL || strlen(srCFGTRec.szPrtSlogan) <= 0 || strlen(srCFGTRec.szPrtSlogan) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPrtSlogan() ERROR !!");

                        if (szPrtSlogan == NULL)
                        {
                                inDISP_LogPrintf("szPrtSlogan == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrtSlogan length = (%d)", (int)strlen(srCFGTRec.szPrtSlogan));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPrtSlogan[0], &srCFGTRec.szPrtSlogan[0], strlen(srCFGTRec.szPrtSlogan));

        return (VS_SUCCESS);
}

/*
Function        :inSetPrtSlogan
Date&Time       :
Describe        :
*/
int inSetPrtSlogan(char* szPrtSlogan)
{
        memset(srCFGTRec.szPrtSlogan, 0x00, sizeof(srCFGTRec.szPrtSlogan));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPrtSlogan == NULL || strlen(szPrtSlogan) <= 0 || strlen(szPrtSlogan) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPrtSlogan() ERROR !!");

                        if (szPrtSlogan == NULL)
                        {
                                inDISP_LogPrintf("szPrtSlogan == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrtSlogan length = (%d)", (int)strlen(szPrtSlogan));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szPrtSlogan[0], &szPrtSlogan[0], strlen(szPrtSlogan));

        return (VS_SUCCESS);
}

/*
Function        :inGetSloganStartDate
Date&Time       :
Describe        :
*/
int inGetSloganStartDate(char* szSloganStartDate)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szSloganStartDate == NULL || strlen(srCFGTRec.szSloganStartDate) <= 0 || strlen(srCFGTRec.szSloganStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetSloganStartDate() ERROR !!");

                        if (szSloganStartDate == NULL)
                        {
                                inDISP_LogPrintf("szSloganStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSloganStartDate length = (%d)", (int)strlen(srCFGTRec.szSloganStartDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSloganStartDate[0], &srCFGTRec.szSloganStartDate[0], strlen(srCFGTRec.szSloganStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetSloganStartDate
Date&Time       :
Describe        :
*/
int inSetSloganStartDate(char* szSloganStartDate)
{
        memset(srCFGTRec.szSloganStartDate, 0x00, sizeof(srCFGTRec.szSloganStartDate));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSloganStartDate == NULL || strlen(szSloganStartDate) <= 0 || strlen(szSloganStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetSloganStartDate() ERROR !!");

                        if (szSloganStartDate == NULL)
                        {
                                inDISP_LogPrintf("szSloganStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSloganStartDate length = (%d)", (int)strlen(szSloganStartDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szSloganStartDate[0], &szSloganStartDate[0], strlen(szSloganStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetSloganEndDate
Date&Time       :
Describe        :
*/
int inGetSloganEndDate(char* szSloganEndDate)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szSloganEndDate == NULL || strlen(srCFGTRec.szSloganEndDate) <= 0 || strlen(srCFGTRec.szSloganEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetSloganEndDate() ERROR !!");

                        if (szSloganEndDate == NULL)
                        {
                                inDISP_LogPrintf("szSloganEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSloganEndDate length = (%d)", (int)strlen(srCFGTRec.szSloganEndDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSloganEndDate[0], &srCFGTRec.szSloganEndDate[0], strlen(srCFGTRec.szSloganEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetSloganEndDate
Date&Time       :
Describe        :
*/
int inSetSloganEndDate(char* szSloganEndDate)
{
        memset(srCFGTRec.szSloganEndDate, 0x00, sizeof(srCFGTRec.szSloganEndDate));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSloganEndDate == NULL || strlen(szSloganEndDate) <= 0 || strlen(szSloganEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetSloganEndDate() ERROR !!");

                        if (szSloganEndDate == NULL)
                        {
                                inDISP_LogPrintf("szSloganEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSloganEndDate length = (%d)", (int)strlen(szSloganEndDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szSloganEndDate[0], &szSloganEndDate[0], strlen(szSloganEndDate));

        return (VS_SUCCESS);
}

int inGetSloganPrtPosition(char* szSloganPrtPosition)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSloganPrtPosition == NULL || strlen(srCFGTRec.szSloganPrtPosition) <= 0 || strlen(srCFGTRec.szSloganPrtPosition) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetSloganPrtPosition() ERROR !!");

                        if (szSloganPrtPosition == NULL)
                        {
                                inDISP_LogPrintf("szSloganPrtPosition == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSloganPrtPosition length = (%d)", (int)strlen(srCFGTRec.szSloganPrtPosition));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSloganPrtPosition[0], &srCFGTRec.szSloganPrtPosition[0], strlen(srCFGTRec.szSloganPrtPosition));

        return (VS_SUCCESS);
}

int inSetSloganPrtPosition(char* szSloganPrtPosition)
{
        memset(srCFGTRec.szSloganPrtPosition, 0x00, sizeof(srCFGTRec.szSloganPrtPosition));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSloganPrtPosition == NULL || strlen(szSloganPrtPosition) <= 0 || strlen(szSloganPrtPosition) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szSloganPrtPosition == NULL)
                        {
                                inDISP_LogPrintf("inSetSloganPrtPositio() ERROR !! szSloganPrtPosition == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inSetSloganPrtPositio() ERROR !! szSloganPrtPosition length = (%d)", (int)strlen(szSloganPrtPosition));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szSloganPrtPosition[0], &szSloganPrtPosition[0], strlen(szSloganPrtPosition));

        return (VS_SUCCESS);
}

/*
Function        :inGetPrtMode
Date&Time       :
Describe        :
*/
int inGetPrtMode(char* szPrtMode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPrtMode == NULL || strlen(srCFGTRec.szPrtMode) <= 0 || strlen(srCFGTRec.szPrtMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPrtMode() ERROR !!");

                        if (szPrtMode == NULL)
                        {
                                inDISP_LogPrintf("szPrtMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrtMode length = (%d)", (int)strlen(srCFGTRec.szPrtMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPrtMode[0], &srCFGTRec.szPrtMode[0], strlen(srCFGTRec.szPrtMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetPrtMode
Date&Time       :
Describe        :
*/
int inSetPrtMode(char* szPrtMode)
{
        memset(srCFGTRec.szPrtMode, 0x00, sizeof(srCFGTRec.szPrtMode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPrtMode == NULL || strlen(szPrtMode) <= 0 || strlen(szPrtMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPrtMode() ERROR !!");

                        if (szPrtMode == NULL)
                        {
                                inDISP_LogPrintf("szPrtMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrtMode length = (%d)", (int)strlen(szPrtMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szPrtMode[0], &szPrtMode[0], strlen(szPrtMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetContactlessEnable
Date&Time       :
Describe        :
*/
int inGetContactlessEnable(char* szContactlessEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szContactlessEnable == NULL || strlen(srCFGTRec.szContactlessEnable) <= 0 || strlen(srCFGTRec.szContactlessEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetContactlessEnable() ERROR !!");

                        if (szContactlessEnable == NULL)
                        {
                                inDISP_LogPrintf("szContactlessEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szContactlessEnable length = (%d)", (int)strlen(srCFGTRec.szContactlessEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szContactlessEnable[0], &srCFGTRec.szContactlessEnable[0], strlen(srCFGTRec.szContactlessEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetContactlessEnable
Date&Time       :
Describe        :
*/
int inSetContactlessEnable(char* szContactlessEnable)
{
        memset(srCFGTRec.szContactlessEnable, 0x00, sizeof(srCFGTRec.szContactlessEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szContactlessEnable == NULL || strlen(szContactlessEnable) <= 0 || strlen(szContactlessEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetContactlessEnable() ERROR !!");

                        if (szContactlessEnable == NULL)
                        {
                                inDISP_LogPrintf("szContactlessEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szContactlessEnable length = (%d)", (int)strlen(szContactlessEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szContactlessEnable[0], &szContactlessEnable[0], strlen(szContactlessEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetVISAPaywaveEnable
Date&Time       :
Describe        :
*/
int inGetVISAPaywaveEnable(char* szVISAPaywaveEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szVISAPaywaveEnable == NULL || strlen(srCFGTRec.szVISAPaywaveEnable) <= 0 || strlen(srCFGTRec.szVISAPaywaveEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetVISAPaywaveEnable() ERROR !!");

                        if (szVISAPaywaveEnable == NULL)
                        {
                                inDISP_LogPrintf("szVISAPaywaveEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szVISAPaywaveEnable length = (%d)", (int)strlen(srCFGTRec.szVISAPaywaveEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szVISAPaywaveEnable[0], &srCFGTRec.szVISAPaywaveEnable[0], strlen(srCFGTRec.szVISAPaywaveEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetVISAPaywaveEnable
Date&Time       :
Describe        :
*/
int inSetVISAPaywaveEnable(char* szVISAPaywaveEnable)
{
        memset(srCFGTRec.szVISAPaywaveEnable, 0x00, sizeof(srCFGTRec.szVISAPaywaveEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szVISAPaywaveEnable == NULL || strlen(szVISAPaywaveEnable) <= 0 || strlen(szVISAPaywaveEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetVISAPaywaveEnable() ERROR !!");

                        if (szVISAPaywaveEnable == NULL)
                        {
                                inDISP_LogPrintf("szVISAPaywaveEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szVISAPaywaveEnable length = (%d)", (int)strlen(szVISAPaywaveEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szVISAPaywaveEnable[0], &szVISAPaywaveEnable[0], strlen(szVISAPaywaveEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetJCBJspeedyEnable
Date&Time       :
Describe        :
*/
int inGetJCBJspeedyEnable(char* szJCBJspeedyEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szJCBJspeedyEnable == NULL || strlen(srCFGTRec.szJCBJspeedyEnable) <= 0 || strlen(srCFGTRec.szJCBJspeedyEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetJCBJspeedyEnable() ERROR !!");

                        if (szJCBJspeedyEnable == NULL)
                        {
                                inDISP_LogPrintf("szJCBJspeedyEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szJCBJspeedyEnable length = (%d)", (int)strlen(srCFGTRec.szJCBJspeedyEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szJCBJspeedyEnable[0], &srCFGTRec.szJCBJspeedyEnable[0], strlen(srCFGTRec.szJCBJspeedyEnable));

        return (VS_SUCCESS);

}

/*
Function        :inSetJCBJspeedyEnable
Date&Time       :
Describe        :
*/
int inSetJCBJspeedyEnable(char* szJCBJspeedyEnable)
{
        memset(srCFGTRec.szJCBJspeedyEnable, 0x00, sizeof(srCFGTRec.szJCBJspeedyEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szJCBJspeedyEnable == NULL || strlen(szJCBJspeedyEnable) <= 0 || strlen(szJCBJspeedyEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetJCBJspeedyEnable() ERROR !!");

                        if (szJCBJspeedyEnable == NULL)
                        {
                                inDISP_LogPrintf("szJCBJspeedyEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szJCBJspeedyEnable length = (%d)", (int)strlen(szJCBJspeedyEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szJCBJspeedyEnable[0], &szJCBJspeedyEnable[0], strlen(szJCBJspeedyEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetMCPaypassEnable
Date&Time       :
Describe        :
*/
int inGetMCPaypassEnable(char* szMCPaypassEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szMCPaypassEnable == NULL || strlen(srCFGTRec.szMCPaypassEnable) <= 0 || strlen(srCFGTRec.szMCPaypassEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetMCPaypassEnable() ERROR !!");

                        if (szMCPaypassEnable == NULL)
                        {
                                inDISP_LogPrintf("szMCPaypassEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMCPaypassEnable length = (%d)", (int)strlen(srCFGTRec.szMCPaypassEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMCPaypassEnable[0], &srCFGTRec.szMCPaypassEnable[0], strlen(srCFGTRec.szMCPaypassEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetMCPaypassEnable
Date&Time       :
Describe        :
*/
int inSetMCPaypassEnable(char* szMCPaypassEnable)
{
        memset(srCFGTRec.szMCPaypassEnable, 0x00, sizeof(srCFGTRec.szMCPaypassEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szMCPaypassEnable == NULL || strlen(szMCPaypassEnable) <= 0 || strlen(szMCPaypassEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetMCPaypassEnable() ERROR !!");

                        if (szMCPaypassEnable == NULL)
                        {
                                inDISP_LogPrintf("szMCPaypassEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMCPaypassEnable length = (%d)", (int)strlen(szMCPaypassEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szMCPaypassEnable[0], &szMCPaypassEnable[0], strlen(szMCPaypassEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetCUPRefundLimit
Date&Time       :
Describe        :
*/
int inGetCUPRefundLimit(char* szCUPRefundLimit)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCUPRefundLimit == NULL || strlen(srCFGTRec.szCUPRefundLimit) <= 0 || strlen(srCFGTRec.szCUPRefundLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCUPRefundLimit() ERROR !!");

                        if (szCUPRefundLimit == NULL)
                        {
                                inDISP_LogPrintf("szCUPRefundLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCUPRefundLimit length = (%d)", (int)strlen(srCFGTRec.szCUPRefundLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCUPRefundLimit[0], &srCFGTRec.szCUPRefundLimit[0], strlen(srCFGTRec.szCUPRefundLimit));

        return (VS_SUCCESS);
}

/*
Function        :inSetCUPRefundLimit
Date&Time       :
Describe        :
*/
int inSetCUPRefundLimit(char* szCUPRefundLimit)
{
        memset(srCFGTRec.szCUPRefundLimit, 0x00, sizeof(srCFGTRec.szCUPRefundLimit));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCUPRefundLimit == NULL || strlen(szCUPRefundLimit) <= 0 || strlen(szCUPRefundLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCUPRefundLimit() ERROR !!");

                        if (szCUPRefundLimit == NULL)
                        {
                                inDISP_LogPrintf("szCUPRefundLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCUPRefundLimit length = (%d)", (int)strlen(szCUPRefundLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szCUPRefundLimit[0], &szCUPRefundLimit[0], strlen(szCUPRefundLimit));

        return (VS_SUCCESS);
}

/*
Function        :inGetCUPKeyExchangeTimes
Date&Time       :
Describe        :
*/
int inGetCUPKeyExchangeTimes(char* szCUPKeyExchangeTimes)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCUPKeyExchangeTimes == NULL || strlen(srCFGTRec.szCUPKeyExchangeTimes) <= 0 || strlen(srCFGTRec.szCUPKeyExchangeTimes) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCUPKeyExchangeTimes() ERROR !!");

                        if (szCUPKeyExchangeTimes == NULL)
                        {
                                inDISP_LogPrintf("szCUPKeyExchangeTimes == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCUPKeyExchangeTimes length = (%d)", (int)strlen(srCFGTRec.szCUPKeyExchangeTimes));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCUPKeyExchangeTimes[0], &srCFGTRec.szCUPKeyExchangeTimes[0], strlen(srCFGTRec.szCUPKeyExchangeTimes));

        return (VS_SUCCESS);
}

/*
Function        :inSetCUPKeyExchangeTimes
Date&Time       :
Describe        :
*/
int inSetCUPKeyExchangeTimes(char* szCUPKeyExchangeTimes)
{
        memset(srCFGTRec.szCUPKeyExchangeTimes, 0x00, sizeof(srCFGTRec.szCUPKeyExchangeTimes));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCUPKeyExchangeTimes == NULL || strlen(szCUPKeyExchangeTimes) <= 0 || strlen(szCUPKeyExchangeTimes) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCUPKeyExchangeTimes() ERROR !!");

                        if (szCUPKeyExchangeTimes == NULL)
                        {
                                inDISP_LogPrintf("szCUPKeyExchangeTimes == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCUPKeyExchangeTimes length = (%d)", (int)strlen(szCUPKeyExchangeTimes));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szCUPKeyExchangeTimes[0], &szCUPKeyExchangeTimes[0], strlen(szCUPKeyExchangeTimes));

        return (VS_SUCCESS);
}

/*
Function        :inGetMACEnable
Date&Time       :
Describe        :
*/
int inGetMACEnable(char* szMACEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szMACEnable == NULL || strlen(srCFGTRec.szMACEnable) <= 0 || strlen(srCFGTRec.szMACEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetMACEnable() ERROR !!");

                        if (szMACEnable == NULL)
                        {
                                inDISP_LogPrintf("szMACEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMACEnable length = (%d)", (int)strlen(srCFGTRec.szMACEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMACEnable[0], &srCFGTRec.szMACEnable[0], strlen(srCFGTRec.szMACEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetMACEnable
Date&Time       :
Describe        :
*/
int inSetMACEnable(char* szMACEnable)
{
        memset(srCFGTRec.szMACEnable, 0x00, sizeof(srCFGTRec.szMACEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szMACEnable == NULL || strlen(szMACEnable) <= 0 || strlen(szMACEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetMACEnable() ERROR !!");

                        if (szMACEnable == NULL)
                        {
                                inDISP_LogPrintf("szMACEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMACEnable length = (%d)", (int)strlen(szMACEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szMACEnable[0], &szMACEnable[0], strlen(szMACEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetPinpadMode
Date&Time       :
Describe        :
*/
int inGetPinpadMode(char* szPinpadMode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPinpadMode == NULL || strlen(srCFGTRec.szPinpadMode) <= 0 || strlen(srCFGTRec.szPinpadMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPinpadMode() ERROR !!");

                        if (szPinpadMode == NULL)
                        {
                                inDISP_LogPrintf("szPinpadMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPinpadMode length = (%d)", (int)strlen(srCFGTRec.szPinpadMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPinpadMode[0], &srCFGTRec.szPinpadMode[0], strlen(srCFGTRec.szPinpadMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetPinpadMode
Date&Time       :
Describe        :
*/
int inSetPinpadMode(char* szPinpadMode)
{
        memset(srCFGTRec.szPinpadMode, 0x00, sizeof(srCFGTRec.szPinpadMode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPinpadMode == NULL || strlen(szPinpadMode) <= 0 || strlen(szPinpadMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPinpadMode() ERROR !!");

                        if (szPinpadMode == NULL)
                        {
                                inDISP_LogPrintf("szPinpadMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPinpadMode length = (%d)", (int)strlen(szPinpadMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szPinpadMode[0], &szPinpadMode[0], strlen(szPinpadMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetFORCECVV2
Date&Time       :
Describe        :
*/
int inGetFORCECVV2(char* szFORCECVV2)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szFORCECVV2 == NULL || strlen(srCFGTRec.szFORCECVV2) <= 0 || strlen(srCFGTRec.szFORCECVV2) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetFORCECVV2() ERROR !!");

                        if (szFORCECVV2 == NULL)
                        {
                                inDISP_LogPrintf("szFORCECVV2 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFORCECVV2 length = (%d)", (int)strlen(srCFGTRec.szFORCECVV2));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFORCECVV2[0], &srCFGTRec.szFORCECVV2[0], strlen(srCFGTRec.szFORCECVV2));

        return (VS_SUCCESS);
}

/*
Function        :inSetFORCECVV2
Date&Time       :
Describe        :
*/
int inSetFORCECVV2(char* szFORCECVV2)
{
        memset(srCFGTRec.szFORCECVV2, 0x00, sizeof(srCFGTRec.szFORCECVV2));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFORCECVV2 == NULL || strlen(szFORCECVV2) <= 0 || strlen(szFORCECVV2) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetFORCECVV2() ERROR !!");

                        if (szFORCECVV2 == NULL)
                        {
                                inDISP_LogPrintf("szFORCECVV2 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFORCECVV2 length = (%d)", (int)strlen(szFORCECVV2));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szFORCECVV2[0], &szFORCECVV2[0], strlen(szFORCECVV2));

        return (VS_SUCCESS);
}

/*
Function        :inGet4DBC
Date&Time       :
Describe        :
*/
int inGet4DBC(char* sz4DBC)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (sz4DBC == NULL || strlen(srCFGTRec.sz4DBC) <= 0 || strlen(srCFGTRec.sz4DBC) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGet4DBC() ERROR !!");

                        if (sz4DBC == NULL)
                        {
                                inDISP_LogPrintf("sz4DBC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "sz4DBC length = (%d)", (int)strlen(srCFGTRec.sz4DBC));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&sz4DBC[0], &srCFGTRec.sz4DBC[0], strlen(srCFGTRec.sz4DBC));

        return (VS_SUCCESS);
}

/*
Function        :inSet4DBCEnable
Date&Time       :
Describe        :
*/
int inSet4DBC(char* sz4DBC)
{
        memset(srCFGTRec.sz4DBC, 0x00, sizeof(srCFGTRec.sz4DBC));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (sz4DBC == NULL || strlen(sz4DBC) <= 0 || strlen(sz4DBC) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSet4DBC() ERROR !!");

                        if (sz4DBC == NULL)
                        {
                                inDISP_LogPrintf("sz4DBC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "sz4DBC length = (%d)", (int)strlen(sz4DBC));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.sz4DBC[0], &sz4DBC[0], strlen(sz4DBC));

        return (VS_SUCCESS);
}

/*
Function        :inGetSpecialCardRangeEnable
Date&Time       :
Describe        :
*/
int inGetSpecialCardRangeEnable(char* szSpecialCardRangeEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szSpecialCardRangeEnable == NULL || strlen(srCFGTRec.szSpecialCardRangeEnable) <= 0 || strlen(srCFGTRec.szSpecialCardRangeEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetszSpecialCardRangeEnable() ERROR !!");

                        if (szSpecialCardRangeEnable == NULL)
                        {
                                inDISP_LogPrintf("szSpecialCardRangeEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSpecialCardRangeEnable length = (%d)", (int)strlen(srCFGTRec.szSpecialCardRangeEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSpecialCardRangeEnable[0], &srCFGTRec.szSpecialCardRangeEnable[0], strlen(srCFGTRec.szSpecialCardRangeEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetSpecialCardRangeEnable
Date&Time       :
Describe        :
*/
int inSetSpecialCardRangeEnable(char* szSpecialCardRangeEnable)
{
        memset(srCFGTRec.szSpecialCardRangeEnable, 0x00, sizeof(srCFGTRec.szSpecialCardRangeEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSpecialCardRangeEnable == NULL || strlen(szSpecialCardRangeEnable) <= 0 || strlen(szSpecialCardRangeEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetSpecialCardRangeEnable() ERROR !!");

                        if (szSpecialCardRangeEnable == NULL)
                        {
                                inDISP_LogPrintf("szSpecialCardRangeEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSpecialCardRangeEnable length = (%d)", (int)strlen(szSpecialCardRangeEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szSpecialCardRangeEnable[0], &szSpecialCardRangeEnable[0], strlen(szSpecialCardRangeEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetPrtMerchantLogo
Date&Time       :
Describe        :
*/
int inGetPrtMerchantLogo(char* szPrtMerchantLogo)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPrtMerchantLogo == NULL || strlen(srCFGTRec.szPrtMerchantLogo) <= 0 || strlen(srCFGTRec.szPrtMerchantLogo) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPrtMerchantLogo() ERROR !!");

                        if (szPrtMerchantLogo == NULL)
                        {
                                inDISP_LogPrintf("szPrtMerchantLogo == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrtMerchantLogo length = (%d)", (int)strlen(srCFGTRec.szPrtMerchantLogo));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPrtMerchantLogo[0], &srCFGTRec.szPrtMerchantLogo[0], strlen(srCFGTRec.szPrtMerchantLogo));

        return (VS_SUCCESS);
}

/*
Function        :inSetPrtMerchantLogo
Date&Time       :
Describe        :
*/
int inSetPrtMerchantLogo(char* szPrtMerchantLogo)
{
        memset(srCFGTRec.szPrtMerchantLogo, 0x00, sizeof(srCFGTRec.szPrtMerchantLogo));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPrtMerchantLogo == NULL || strlen(szPrtMerchantLogo) <= 0 || strlen(szPrtMerchantLogo) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPrtMerchantLogo() ERROR !!");

                        if (szPrtMerchantLogo == NULL)
                        {
                                inDISP_LogPrintf("szPrtMerchantLogo == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrtMerchantLogo length = (%d)", (int)strlen(szPrtMerchantLogo));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szPrtMerchantLogo[0], &szPrtMerchantLogo[0], strlen(szPrtMerchantLogo));

        return (VS_SUCCESS);
}

/*
Function        :inGetPrtMerchantName
Date&Time       :
Describe        :
*/
int inGetPrtMerchantName(char* szPrtMerchantName)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPrtMerchantName == NULL || strlen(srCFGTRec.szPrtMerchantName) <= 0 || strlen(srCFGTRec.szPrtMerchantName) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPrtMerchantName() ERROR !!");

                        if (szPrtMerchantName == NULL)
                        {
                                inDISP_LogPrintf("szPrtMerchantName == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrtMerchantName length = (%d)", (int)strlen(srCFGTRec.szPrtMerchantName));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPrtMerchantName[0], &srCFGTRec.szPrtMerchantName[0], strlen(srCFGTRec.szPrtMerchantName));

        return (VS_SUCCESS);
}

/*
Function        :inSetPrtMerchantName
Date&Time       :
Describe        :
*/
int inSetPrtMerchantName(char* szPrtMerchantName)
{
        memset(srCFGTRec.szPrtMerchantName, 0x00, sizeof(srCFGTRec.szPrtMerchantName));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPrtMerchantName == NULL || strlen(szPrtMerchantName) <= 0 || strlen(szPrtMerchantName) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPrtMerchantName() ERROR !!");

                        if (szPrtMerchantName == NULL)
                        {
                                inDISP_LogPrintf("szPrtMerchantName == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrtMerchantName length = (%d)", (int)strlen(szPrtMerchantName));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szPrtMerchantName[0], &szPrtMerchantName[0], strlen(szPrtMerchantName));

        return (VS_SUCCESS);
}

/*
Function        :inGetPrtNotice
Date&Time       :
Describe        :
*/
int inGetPrtNotice(char* szPrtNotice)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPrtNotice == NULL || strlen(srCFGTRec.szPrtNotice) <= 0 || strlen(srCFGTRec.szPrtNotice) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPrtNotice() ERROR !!");

                        if (szPrtNotice == NULL)
                        {
                                inDISP_LogPrintf("szPrtNotice == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrtNotice length = (%d)", (int)strlen(srCFGTRec.szPrtNotice));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPrtNotice[0], &srCFGTRec.szPrtNotice[0], strlen(srCFGTRec.szPrtNotice));

        return (VS_SUCCESS);

}

/*
Function        :inSetPrtNotice
Date&Time       :
Describe        :
*/
int inSetPrtNotice(char* szPrtNotice)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPrtNotice == NULL || strlen(szPrtNotice) <= 0 || strlen(szPrtNotice) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPrtNotice() ERROR !!");

                        if (szPrtNotice == NULL)
                        {
                                inDISP_LogPrintf("szPrtNotice == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrtNotice length = (%d)", (int)strlen(szPrtNotice));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szPrtNotice[0], &szPrtNotice[0], strlen(szPrtNotice));

        return (VS_SUCCESS);
}

/*
Function        :inGetPrtPromotionAdv
Date&Time       :
Describe        :
*/
int inGetPrtPromotionAdv(char* szPrtPromotionAdv)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPrtPromotionAdv == NULL || strlen(srCFGTRec.szPrtPromotionAdv) <= 0 || strlen(srCFGTRec.szPrtPromotionAdv) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPrtPromotionAdv() ERROR !!");

                        if (szPrtPromotionAdv == NULL)
                        {
                                inDISP_LogPrintf("szPrtPromotionAdv == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrtPromotionAdv length = (%d)", (int)strlen(srCFGTRec.szPrtPromotionAdv));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPrtPromotionAdv[0], &srCFGTRec.szPrtPromotionAdv[0], strlen(srCFGTRec.szPrtPromotionAdv));

        return (VS_SUCCESS);

}

/*
Function        :inSetPrtPromotionAdv
Date&Time       :
Describe        :
*/
int inSetPrtPromotionAdv(char* szPrtPromotionAdv)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPrtPromotionAdv == NULL || strlen(szPrtPromotionAdv) <= 0 || strlen(szPrtPromotionAdv) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPrtPromotionAdv() ERROR !!");

                        if (szPrtPromotionAdv == NULL)
                        {
                                inDISP_LogPrintf("szPrtPromotionAdv == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrtPromotionAdv length = (%d)", (int)strlen(szPrtPromotionAdv));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szPrtPromotionAdv[0], &szPrtPromotionAdv[0], strlen(szPrtPromotionAdv));

        return (VS_SUCCESS);
}

/*
Function        :inGetElecCommerceFlag
Date&Time       :
Describe        :
*/
int inGetElecCommerceFlag(char* szElecCommerceFlag)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szElecCommerceFlag == NULL || strlen(srCFGTRec.szElecCommerceFlag) <= 0 || strlen(srCFGTRec.szElecCommerceFlag) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetElecCommerceFlag() ERROR !!");

                        if (szElecCommerceFlag == NULL)
                        {
                                inDISP_LogPrintf("szElecCommerceFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szElecCommerceFlag length = (%d)", (int)strlen(srCFGTRec.szElecCommerceFlag));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szElecCommerceFlag[0], &srCFGTRec.szElecCommerceFlag[0], strlen(srCFGTRec.szElecCommerceFlag));

        return (VS_SUCCESS);
}

/*
Function        :inSetElecCommerceFlag
Date&Time       :
Describe        :
*/
int inSetElecCommerceFlag(char* szElecCommerceFlag)
{
        memset(srCFGTRec.szElecCommerceFlag, 0x00, sizeof(srCFGTRec.szElecCommerceFlag));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szElecCommerceFlag == NULL || strlen(szElecCommerceFlag) <= 0 || strlen(szElecCommerceFlag) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetElecCommerceFlag() ERROR !!");

                        if (szElecCommerceFlag == NULL)
                        {
                                inDISP_LogPrintf("szElecCommerceFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szElecCommerceFlag length = (%d)", (int)strlen(szElecCommerceFlag));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szElecCommerceFlag[0], &szElecCommerceFlag[0], strlen(szElecCommerceFlag));

        return (VS_SUCCESS);
}

/*
Function        :inGetDccFlowVersion
Date&Time       :
Describe        :
*/
int inGetDccFlowVersion(char* szDccFlowVersion)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szDccFlowVersion == NULL || strlen(srCFGTRec.szDccFlowVersion) <= 0 || strlen(srCFGTRec.szDccFlowVersion) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetDccFlowVersion() ERROR !!");

                        if (szDccFlowVersion == NULL)
                        {
                                inDISP_LogPrintf("szDccFlowVersion == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDccFlowVersion length = (%d)", (int)strlen(srCFGTRec.szDccFlowVersion));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDccFlowVersion[0], &srCFGTRec.szDccFlowVersion[0], strlen(srCFGTRec.szDccFlowVersion));

        return (VS_SUCCESS);
}

/*
Function        :inSetDccFlowVersion
Date&Time       :
Describe        :
*/
int inSetDccFlowVersion(char* szDccFlowVersion)
{
        memset(srCFGTRec.szDccFlowVersion, 0x00, sizeof(srCFGTRec.szDccFlowVersion));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szDccFlowVersion == NULL || strlen(szDccFlowVersion) <= 0 || strlen(szDccFlowVersion) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetDccFlowVersion() ERROR !!");

                        if (szDccFlowVersion == NULL)
                        {
                                inDISP_LogPrintf("szDccFlowVersion == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDccFlowVersion length = (%d)", (int)strlen(szDccFlowVersion));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szDccFlowVersion[0], &szDccFlowVersion[0], strlen(szDccFlowVersion));

        return (VS_SUCCESS);
}

/*
Function        :inGetSupDccVisa
Date&Time       :
Describe        :
*/
int inGetSupDccVisa(char* szSupDccVisa)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szSupDccVisa == NULL || strlen(srCFGTRec.szSupDccVisa) <= 0 || strlen(srCFGTRec.szSupDccVisa) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetSupDccVisa() ERROR !!");

                        if (szSupDccVisa == NULL)
                        {
                                inDISP_LogPrintf("szSupDccVisa == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSupDccVisa length = (%d)", (int)strlen(srCFGTRec.szSupDccVisa));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSupDccVisa[0], &srCFGTRec.szSupDccVisa[0], strlen(srCFGTRec.szSupDccVisa));

        return (VS_SUCCESS);
}

/*
Function        :inSetSupDccVisa
Date&Time       :
Describe        :
*/
int inSetSupDccVisa(char* szSupDccVisa)
{
        memset(srCFGTRec.szSupDccVisa, 0x00, sizeof(srCFGTRec.szSupDccVisa));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSupDccVisa == NULL || strlen(szSupDccVisa) <= 0 || strlen(szSupDccVisa) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetSupDccVisa() ERROR !!");

                        if (szSupDccVisa == NULL)
                        {
                                inDISP_LogPrintf("szSupDccVisa == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSupDccVisa length = (%d)", (int)strlen(szSupDccVisa));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szSupDccVisa[0], &szSupDccVisa[0], strlen(szSupDccVisa));

        return (VS_SUCCESS);
}

/*
Function        :inGetSupDccMasterCard
Date&Time       :
Describe        :
*/
int inGetSupDccMasterCard(char* szSupDccMasterCard)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szSupDccMasterCard == NULL || strlen(srCFGTRec.szSupDccMasterCard) <= 0 || strlen(srCFGTRec.szSupDccMasterCard) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetSupDccMasterCard() ERROR !!");

                        if (szSupDccMasterCard == NULL)
                        {
                                inDISP_LogPrintf("szSupDccMasterCard == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSupDccMasterCard length = (%d)", (int)strlen(srCFGTRec.szSupDccMasterCard));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSupDccMasterCard[0], &srCFGTRec.szSupDccMasterCard[0], strlen(srCFGTRec.szSupDccMasterCard));

        return (VS_SUCCESS);
}

/*
Function        :inSetSupDccMasterCard
Date&Time       :
Describe        :
*/
int inSetSupDccMasterCard(char* szSupDccMasterCard)
{
        memset(srCFGTRec.szSupDccMasterCard, 0x00, sizeof(srCFGTRec.szSupDccMasterCard));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSupDccMasterCard == NULL || strlen(szSupDccMasterCard) <= 0 || strlen(szSupDccMasterCard) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetSupDccMasterCard() ERROR !!");

                        if (szSupDccMasterCard == NULL)
                        {
                                inDISP_LogPrintf("szSupDccMasterCard == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSupDccMasterCard length = (%d)", (int)strlen(szSupDccMasterCard));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szSupDccMasterCard[0], &szSupDccMasterCard[0], strlen(szSupDccMasterCard));

        return (VS_SUCCESS);
}

/*
Function        :inGetDHCPRetryTimes
Date&Time       :
Describe        :
*/
int inGetDHCPRetryTimes(char* szDHCPRetryTimes)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szDHCPRetryTimes == NULL || strlen(srCFGTRec.szDHCPRetryTimes) <= 0 || strlen(srCFGTRec.szDHCPRetryTimes) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetDHCPRetryTimes() ERROR !!");

                        if (szDHCPRetryTimes == NULL)
                        {
                                inDISP_LogPrintf("szDHCPRetryTimes == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDHCPRetryTimes length = (%d)", (int)strlen(srCFGTRec.szDHCPRetryTimes));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDHCPRetryTimes[0], &srCFGTRec.szDHCPRetryTimes[0], strlen(srCFGTRec.szDHCPRetryTimes));

        return (VS_SUCCESS);
}

/*
Function        :inSetDHCPRetryTimes
Date&Time       :
Describe        :
*/
int inSetDHCPRetryTimes(char* szDHCPRetryTimes)
{
        memset(srCFGTRec.szDHCPRetryTimes, 0x00, sizeof(srCFGTRec.szDHCPRetryTimes));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szDHCPRetryTimes == NULL || strlen(szDHCPRetryTimes) <= 0 || strlen(szDHCPRetryTimes) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetDHCPRetryTimes() ERROR !!");

                        if (szDHCPRetryTimes == NULL)
                        {
                                inDISP_LogPrintf("szDHCPRetryTimes == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDHCPRetryTimes length = (%d)", (int)strlen(szDHCPRetryTimes));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szDHCPRetryTimes[0], &szDHCPRetryTimes[0], strlen(szDHCPRetryTimes));

        return (VS_SUCCESS);
}

/*
Function        :inGetBankID
Date&Time       :
Describe        :
*/
int inGetBankID(char* szBankID)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szBankID == NULL || strlen(srCFGTRec.szBankID) <= 0 || strlen(srCFGTRec.szBankID) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetBankID() ERROR !!");

                        if (szBankID == NULL)
                        {
                                inDISP_LogPrintf("szBankID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBankID length = (%d)", (int)strlen(srCFGTRec.szBankID));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szBankID[0], &srCFGTRec.szBankID[0], strlen(srCFGTRec.szBankID));

        return (VS_SUCCESS);        
}

/*
Function        :inSetBankID
Date&Time       :
Describe        :
*/
int inSetBankID(char* szBankID)
{
        memset(srCFGTRec.szBankID, 0x00, sizeof(srCFGTRec.szBankID));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szBankID == NULL || strlen(szBankID) <= 0 || strlen(szBankID) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetBankID() ERROR !!");

                        if (szBankID == NULL)
                        {
                                inDISP_LogPrintf("szBankID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBankID length = (%d)", (int)strlen(szBankID));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szBankID[0], &szBankID[0], strlen(szBankID));

        return (VS_SUCCESS);
}

/*
Function        :inGetVEPSFlag
Date&Time       :
Describe        :
*/
int inGetVEPSFlag(char* szVEPSFlag)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szVEPSFlag == NULL || strlen(srCFGTRec.szVEPSFlag) <= 0 || strlen(srCFGTRec.szVEPSFlag) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetVEPSFlag() ERROR !!");

                        if (szVEPSFlag == NULL)
                        {
                                inDISP_LogPrintf("szVEPSFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szVEPSFlag length = (%d)", (int)strlen(srCFGTRec.szVEPSFlag));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szVEPSFlag[0], &srCFGTRec.szVEPSFlag[0], strlen(srCFGTRec.szVEPSFlag));

        return (VS_SUCCESS);           
}

/*
Function        :inSetVEPSFlag
Date&Time       :
Describe        :
*/
int inSetVEPSFlag(char* szVEPSFlag)
{
        memset(srCFGTRec.szVEPSFlag, 0x00, sizeof(srCFGTRec.szVEPSFlag));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szVEPSFlag == NULL || strlen(szVEPSFlag) <= 0 || strlen(szVEPSFlag) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetVEPSFlag() ERROR !!");

                        if (szVEPSFlag == NULL)
                        {
                                inDISP_LogPrintf("szVEPSFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szVEPSFlag length = (%d)", (int)strlen(szVEPSFlag));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szVEPSFlag[0], &szVEPSFlag[0], strlen(szVEPSFlag));

        return (VS_SUCCESS);
}

/*
Function        :inGetComportMode
Date&Time       :
Describe        :
*/
int inGetComportMode(char* szComportMode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szComportMode == NULL || strlen(srCFGTRec.szComportMode) <= 0 || strlen(srCFGTRec.szComportMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetComportMode() ERROR !!");

                        if (szComportMode == NULL)
                        {
                                inDISP_LogPrintf("szComportMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szComportMode length = (%d)", (int)strlen(srCFGTRec.szComportMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szComportMode[0], &srCFGTRec.szComportMode[0], strlen(srCFGTRec.szComportMode));

        return (VS_SUCCESS);  
}

/*
Function        :inSetComportMode
Date&Time       :
Describe        :
*/
int inSetComportMode(char* szComportMode)
{
        memset(srCFGTRec.szComportMode, 0x00, sizeof(srCFGTRec.szComportMode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szComportMode == NULL || strlen(szComportMode) <= 0 || strlen(szComportMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetComportMode() ERROR !!");

                        if (szComportMode == NULL)
                        {
                                inDISP_LogPrintf("szComportMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szComportMode length = (%d)", (int)strlen(szComportMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szComportMode[0], &szComportMode[0], strlen(szComportMode));

        return (VS_SUCCESS);        
}

/*
Function        :inGetFTPMode
Date&Time       :
Describe        :
*/
int inGetFTPMode(char* szFTPMode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPMode == NULL || strlen(srCFGTRec.szFTPMode) <= 0 || strlen(srCFGTRec.szFTPMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetFTPMode() ERROR !!");

                        if (szFTPMode == NULL)
                        {
                                inDISP_LogPrintf("szFTPMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPMode length = (%d)", (int)strlen(srCFGTRec.szFTPMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPMode[0], &srCFGTRec.szFTPMode[0], strlen(srCFGTRec.szFTPMode));

        return (VS_SUCCESS);         
}

/*
Function        :inSetFTPMode
Date&Time       :
Describe        :
*/
int inSetFTPMode(char* szFTPMode)
{
        memset(srCFGTRec.szFTPMode, 0x00, sizeof(srCFGTRec.szFTPMode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPMode == NULL || strlen(szFTPMode) <= 0 || strlen(szFTPMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetFTPMode() ERROR !!");

                        if (szFTPMode == NULL)
                        {
                                inDISP_LogPrintf("szFTPMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPMode length = (%d)", (int)strlen(szFTPMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szFTPMode[0], &szFTPMode[0], strlen(szFTPMode));

        return (VS_SUCCESS);    
}

/*
Function        :inGetContlessMode
Date&Time       :
Describe        :
*/
int inGetContlessMode(char* szContlessMode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szContlessMode == NULL || strlen(srCFGTRec.szContlessMode) <= 0 || strlen(srCFGTRec.szContlessMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetContlessMode() ERROR !!");

                        if (szContlessMode == NULL)
                        {
                                inDISP_LogPrintf("szContlessMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szContlessMode length = (%d)", (int)strlen(srCFGTRec.szContlessMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szContlessMode[0], &srCFGTRec.szContlessMode[0], strlen(srCFGTRec.szContlessMode));

        return (VS_SUCCESS);    
}
/*
Function        :inSetContlessMode
Date&Time       :
Describe        :
*/
int inSetContlessMode(char* szContlessMode)
{
        memset(srCFGTRec.szContlessMode, 0x00, sizeof(srCFGTRec.szContlessMode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szContlessMode == NULL || strlen(szContlessMode) <= 0 || strlen(szContlessMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetContlessMode() ERROR !!");

                        if (szContlessMode == NULL)
                        {
                                inDISP_LogPrintf("szContlessMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szContlessMode length = (%d)", (int)strlen(szContlessMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szContlessMode[0], &szContlessMode[0], strlen(szContlessMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetBarCodeReaderEnable
Date&Time       :
Describe        :
*/
int inGetBarCodeReaderEnable(char* szBarCodeReaderEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szBarCodeReaderEnable == NULL || strlen(srCFGTRec.szBarCodeReaderEnable) <= 0 || strlen(srCFGTRec.szBarCodeReaderEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetBarCodeReaderEnable() ERROR !!");

                        if (szBarCodeReaderEnable == NULL)
                        {
                                inDISP_LogPrintf("szBarCodeReaderEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBarCodeReaderEnable length = (%d)", (int)strlen(srCFGTRec.szBarCodeReaderEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szBarCodeReaderEnable[0], &srCFGTRec.szBarCodeReaderEnable[0], strlen(srCFGTRec.szBarCodeReaderEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetBarCodeReaderEnable
Date&Time       :
Describe        :
*/
int inSetBarCodeReaderEnable(char* szBarCodeReaderEnable)
{
        memset(srCFGTRec.szBarCodeReaderEnable, 0x00, sizeof(srCFGTRec.szBarCodeReaderEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szBarCodeReaderEnable == NULL || strlen(szBarCodeReaderEnable) <= 0 || strlen(szBarCodeReaderEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetBarCodeReaderEnable() ERROR !!");

                        if (szBarCodeReaderEnable == NULL)
                        {
                                inDISP_LogPrintf("szBarCodeReaderEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBarCodeReaderEnable length = (%d)", (int)strlen(szBarCodeReaderEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szBarCodeReaderEnable[0], &szBarCodeReaderEnable[0], strlen(szBarCodeReaderEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetEMVPINBypassEnable
Date&Time       :
Describe        :
*/
int inGetEMVPINBypassEnable(char* szEMVPINBypassEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szEMVPINBypassEnable == NULL || strlen(srCFGTRec.szEMVPINBypassEnable) <= 0 || strlen(srCFGTRec.szEMVPINBypassEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetEMVPINBypassEnable() ERROR !!");

                        if (szEMVPINBypassEnable == NULL)
                        {
                                inDISP_LogPrintf("szEMVPINBypassEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEMVPINBypassEnable length = (%d)", (int)strlen(srCFGTRec.szEMVPINBypassEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szEMVPINBypassEnable[0], &srCFGTRec.szEMVPINBypassEnable[0], strlen(srCFGTRec.szEMVPINBypassEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetEMVPINBypassEnable
Date&Time       :
Describe        :
*/
int inSetEMVPINBypassEnable(char* szEMVPINBypassEnable)
{
        memset(srCFGTRec.szEMVPINBypassEnable, 0x00, sizeof(srCFGTRec.szEMVPINBypassEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szEMVPINBypassEnable == NULL || strlen(szEMVPINBypassEnable) <= 0 || strlen(szEMVPINBypassEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetEMVPINBypassEnable() ERROR !!");

                        if (szEMVPINBypassEnable == NULL)
                        {
                                inDISP_LogPrintf("szEMVPINBypassEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEMVPINBypassEnable length = (%d)", (int)strlen(szEMVPINBypassEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szEMVPINBypassEnable[0], &szEMVPINBypassEnable[0], strlen(szEMVPINBypassEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetCUPOnlinePINEntryTimeout
Date&Time       :
Describe        :
*/
int inGetCUPOnlinePINEntryTimeout(char* szCUPOnlinePINEntryTimeout)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCUPOnlinePINEntryTimeout == NULL || strlen(srCFGTRec.szCUPOnlinePINEntryTimeout) <= 0 || strlen(srCFGTRec.szCUPOnlinePINEntryTimeout) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCUPOnlinePINEntryTimeout() ERROR !!");

                        if (szCUPOnlinePINEntryTimeout == NULL)
                        {
                                inDISP_LogPrintf("szCUPOnlinePINEntryTimeout == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCUPOnlinePINEntryTimeout length = (%d)", (int)strlen(srCFGTRec.szCUPOnlinePINEntryTimeout));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCUPOnlinePINEntryTimeout[0], &srCFGTRec.szCUPOnlinePINEntryTimeout[0], strlen(srCFGTRec.szCUPOnlinePINEntryTimeout));

        return (VS_SUCCESS);
}

/*
Function        :inSetCUPOnlinePINEntryTimeout
Date&Time       :
Describe        :
*/
int inSetCUPOnlinePINEntryTimeout(char* szCUPOnlinePINEntryTimeout)
{
        memset(srCFGTRec.szCUPOnlinePINEntryTimeout, 0x00, sizeof(srCFGTRec.szCUPOnlinePINEntryTimeout));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCUPOnlinePINEntryTimeout == NULL || strlen(szCUPOnlinePINEntryTimeout) <= 0 || strlen(szCUPOnlinePINEntryTimeout) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCUPOnlinePINEntryTimeout() ERROR !!");

                        if (szCUPOnlinePINEntryTimeout == NULL)
                        {
                                inDISP_LogPrintf("szCUPOnlinePINEntryTimeout == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCUPOnlinePINEntryTimeout length = (%d)", (int)strlen(szCUPOnlinePINEntryTimeout));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szCUPOnlinePINEntryTimeout[0], &szCUPOnlinePINEntryTimeout[0], strlen(szCUPOnlinePINEntryTimeout));

        return (VS_SUCCESS);
}

/*
Function        :inGetSignPadMode
Date&Time       :
Describe        :
*/
int inGetSignPadMode(char* szSignPadMode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szSignPadMode == NULL || strlen(srCFGTRec.szSignPadMode) <= 0 || strlen(srCFGTRec.szSignPadMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetSignPadMode() ERROR !!");

                        if (szSignPadMode == NULL)
                        {
                                inDISP_LogPrintf("szSignPadMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSignPadMode length = (%d)", (int)strlen(srCFGTRec.szSignPadMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSignPadMode[0], &srCFGTRec.szSignPadMode[0], strlen(srCFGTRec.szSignPadMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetSignPadMode
Date&Time       :
Describe        :
*/
int inSetSignPadMode(char* szSignPadMode)
{
        memset(srCFGTRec.szSignPadMode, 0x00, sizeof(srCFGTRec.szSignPadMode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSignPadMode == NULL || strlen(szSignPadMode) <= 0 || strlen(szSignPadMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetSignPadMode() ERROR !!");

                        if (szSignPadMode == NULL)
                        {
                                inDISP_LogPrintf("szSignPadMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSignPadMode length = (%d)", (int)strlen(szSignPadMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szSignPadMode[0], &szSignPadMode[0], strlen(szSignPadMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetESCPrintMerchantCopy
Date&Time       :
Describe        :
*/
int inGetESCPrintMerchantCopy(char* szESCPrintMerchantCopy)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szESCPrintMerchantCopy == NULL || strlen(srCFGTRec.szESCPrintMerchantCopy) <= 0 || strlen(srCFGTRec.szESCPrintMerchantCopy) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetESCPrintMerchantCopy() ERROR !!");

                        if (szESCPrintMerchantCopy == NULL)
                        {
                                inDISP_LogPrintf("szESCPrintMerchantCopy == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESCPrintMerchantCopy length = (%d)", (int)strlen(srCFGTRec.szESCPrintMerchantCopy));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szESCPrintMerchantCopy[0], &srCFGTRec.szESCPrintMerchantCopy[0], strlen(srCFGTRec.szESCPrintMerchantCopy));

        return (VS_SUCCESS);
}

/*
Function        :inSetESCPrintMerchantCopy
Date&Time       :
Describe        :
*/
int inSetESCPrintMerchantCopy(char* szESCPrintMerchantCopy)
{
        memset(srCFGTRec.szESCPrintMerchantCopy, 0x00, sizeof(srCFGTRec.szESCPrintMerchantCopy));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szESCPrintMerchantCopy == NULL || strlen(szESCPrintMerchantCopy) <= 0 || strlen(szESCPrintMerchantCopy) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetESCPrintMerchantCopy() ERROR !!");

                        if (szESCPrintMerchantCopy == NULL)
                        {
                                inDISP_LogPrintf("szESCPrintMerchantCopy == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESCPrintMerchantCopy length = (%d)", (int)strlen(szESCPrintMerchantCopy));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szESCPrintMerchantCopy[0], &szESCPrintMerchantCopy[0], strlen(szESCPrintMerchantCopy));

        return (VS_SUCCESS);
}

/*
Function        :inGetESCPrintMerchantCopyStartDate
Date&Time       :
Describe        :
*/
int inGetESCPrintMerchantCopyStartDate(char* szESCPrintMerchantCopyStartDate)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szESCPrintMerchantCopyStartDate == NULL || strlen(srCFGTRec.szESCPrintMerchantCopyStartDate) <= 0 || strlen(srCFGTRec.szESCPrintMerchantCopyStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetESCPrintMerchantCopyStartDate() ERROR !!");

                        if (szESCPrintMerchantCopyStartDate == NULL)
                        {
                                inDISP_LogPrintf("szESCPrintMerchantCopyStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESCPrintMerchantCopyStartDate length = (%d)", (int)strlen(srCFGTRec.szESCPrintMerchantCopyStartDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szESCPrintMerchantCopyStartDate[0], &srCFGTRec.szESCPrintMerchantCopyStartDate[0], strlen(srCFGTRec.szESCPrintMerchantCopyStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetESCPrintMerchantCopyStartDate
Date&Time       :
Describe        :
*/
int inSetESCPrintMerchantCopyStartDate(char* szESCPrintMerchantCopyStartDate)
{
        memset(srCFGTRec.szESCPrintMerchantCopyStartDate, 0x00, sizeof(srCFGTRec.szESCPrintMerchantCopyStartDate));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szESCPrintMerchantCopyStartDate == NULL || strlen(szESCPrintMerchantCopyStartDate) <= 0 || strlen(szESCPrintMerchantCopyStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetESCPrintMerchantCopyStartDate() ERROR !!");

                        if (szESCPrintMerchantCopyStartDate == NULL)
                        {
                                inDISP_LogPrintf("szESCPrintMerchantCopyStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESCPrintMerchantCopyStartDate length = (%d)", (int)strlen(szESCPrintMerchantCopyStartDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szESCPrintMerchantCopyStartDate[0], &szESCPrintMerchantCopyStartDate[0], strlen(szESCPrintMerchantCopyStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetESCPrintMerchantCopyStartDate
Date&Time       :
Describe        :
*/
int inGetESCPrintMerchantCopyEndDate(char* szESCPrintMerchantCopyEndDate)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szESCPrintMerchantCopyEndDate == NULL || strlen(srCFGTRec.szESCPrintMerchantCopyEndDate) <= 0 || strlen(srCFGTRec.szESCPrintMerchantCopyEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetESCPrintMerchantCopyEndDate() ERROR !!");

                        if (szESCPrintMerchantCopyEndDate == NULL)
                        {
                                inDISP_LogPrintf("szESCPrintMerchantCopyEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESCPrintMerchantCopyEndDate length = (%d)", (int)strlen(srCFGTRec.szESCPrintMerchantCopyEndDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szESCPrintMerchantCopyEndDate[0], &srCFGTRec.szESCPrintMerchantCopyEndDate[0], strlen(srCFGTRec.szESCPrintMerchantCopyEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetESCPrintMerchantCopyStartDate
Date&Time       :
Describe        :
*/
int inSetESCPrintMerchantCopyEndDate(char* szESCPrintMerchantCopyEndDate)
{
        memset(srCFGTRec.szESCPrintMerchantCopyEndDate, 0x00, sizeof(srCFGTRec.szESCPrintMerchantCopyEndDate));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szESCPrintMerchantCopyEndDate == NULL || strlen(szESCPrintMerchantCopyEndDate) <= 0 || strlen(szESCPrintMerchantCopyEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetESCPrintMerchantCopyEndDate() ERROR !!");

                        if (szESCPrintMerchantCopyEndDate == NULL)
                        {
                                inDISP_LogPrintf("szESCPrintMerchantCopyEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESCPrintMerchantCopyEndDate length = (%d)", (int)strlen(szESCPrintMerchantCopyEndDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szESCPrintMerchantCopyEndDate[0], &szESCPrintMerchantCopyEndDate[0], strlen(szESCPrintMerchantCopyEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetESCReciptUploadUpLimit
Date&Time       :
Describe        :
*/
int inGetESCReciptUploadUpLimit(char* szESCReciptUploadUpLimit)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szESCReciptUploadUpLimit == NULL || strlen(srCFGTRec.szESCReciptUploadUpLimit) <= 0 || strlen(srCFGTRec.szESCReciptUploadUpLimit) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetESCReciptUploadUpLimit() ERROR !!");

                        if (szESCReciptUploadUpLimit == NULL)
                        {
                                inDISP_LogPrintf("szESCReciptUploadUpLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESCReciptUploadUpLimit length = (%d)", (int)strlen(srCFGTRec.szESCReciptUploadUpLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szESCReciptUploadUpLimit[0], &srCFGTRec.szESCReciptUploadUpLimit[0], strlen(srCFGTRec.szESCReciptUploadUpLimit));

        return (VS_SUCCESS);
}

/*
Function        :inSetESCReciptUploadUpLimit
Date&Time       :
Describe        :
*/
int inSetESCReciptUploadUpLimit(char* szESCReciptUploadUpLimit)
{
        memset(srCFGTRec.szESCReciptUploadUpLimit, 0x00, sizeof(srCFGTRec.szESCReciptUploadUpLimit));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szESCReciptUploadUpLimit == NULL || strlen(szESCReciptUploadUpLimit) <= 0 || strlen(szESCReciptUploadUpLimit) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetESCReciptUploadUpLimit() ERROR !!");

                        if (szESCReciptUploadUpLimit == NULL)
                        {
                                inDISP_LogPrintf("szESCReciptUploadUpLimite == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESCReciptUploadUpLimit length = (%d)", (int)strlen(szESCReciptUploadUpLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szESCReciptUploadUpLimit[0], &szESCReciptUploadUpLimit[0], strlen(szESCReciptUploadUpLimit));

        return (VS_SUCCESS);
}

/*
Function        :inGetContactlessReaderMode
Date&Time       :
Describe        :
*/
int inGetContactlessReaderMode(char* szContactlessReaderMode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szContactlessReaderMode == NULL || strlen(srCFGTRec.szContactlessReaderMode) <= 0 || strlen(srCFGTRec.szContactlessReaderMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetContactlessReaderMode() ERROR !!");

                        if (szContactlessReaderMode == NULL)
                        {
                                inDISP_LogPrintf("szContactlessReaderMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szContactlessReaderMode length = (%d)", (int)strlen(srCFGTRec.szContactlessReaderMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szContactlessReaderMode[0], &srCFGTRec.szContactlessReaderMode[0], strlen(srCFGTRec.szContactlessReaderMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetContactlessReaderMode
Date&Time       :
Describe        :
*/
int inSetContactlessReaderMode(char* szContactlessReaderMode)
{
        memset(srCFGTRec.szContactlessReaderMode, 0x00, sizeof(srCFGTRec.szContactlessReaderMode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szContactlessReaderMode == NULL || strlen(szContactlessReaderMode) <= 0 || strlen(szContactlessReaderMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetContactlessReaderMode() ERROR !!");

                        if (szContactlessReaderMode == NULL)
                        {
                                inDISP_LogPrintf("szContactlessReaderMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szContactlessReaderMode length = (%d)", (int)strlen(szContactlessReaderMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szContactlessReaderMode[0], &szContactlessReaderMode[0], strlen(szContactlessReaderMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSDownloadMode
Date&Time       :
Describe        :
*/
int inGetTMSDownloadMode(char* szTMSDownloadMode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSDownloadMode == NULL || strlen(srCFGTRec.szTMSDownloadMode) <= 0 || strlen(srCFGTRec.szTMSDownloadMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTMSDownloadMode() ERROR !!");

                        if (szTMSDownloadMode == NULL)
                        {
                                inDISP_LogPrintf("szTMSDownloadMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSDownloadMode length = (%d)", (int)strlen(srCFGTRec.szTMSDownloadMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSDownloadMode[0], &srCFGTRec.szTMSDownloadMode[0], strlen(srCFGTRec.szTMSDownloadMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSDownloadMode
Date&Time       :
Describe        :
*/
int inSetTMSDownloadMode(char* szTMSDownloadMode)
{
        memset(srCFGTRec.szTMSDownloadMode, 0x00, sizeof(srCFGTRec.szTMSDownloadMode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSDownloadMode == NULL || strlen(szTMSDownloadMode) <= 0 || strlen(szTMSDownloadMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTMSDownloadMode() ERROR !!");

                        if (szTMSDownloadMode == NULL)
                        {
                                inDISP_LogPrintf("szTMSDownloadMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSDownloadMode length = (%d)", (int)strlen(szTMSDownloadMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szTMSDownloadMode[0], &szTMSDownloadMode[0], strlen(szTMSDownloadMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetAMEXContactlessEnable
Date&Time       :
Describe        :
*/
int inGetAMEXContactlessEnable(char* szAMEXContactlessEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szAMEXContactlessEnable == NULL || strlen(srCFGTRec.szAMEXContactlessEnable) <= 0 || strlen(srCFGTRec.szAMEXContactlessEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetAMEXContactlessEnable() ERROR !!");

                        if (szAMEXContactlessEnable == NULL)
                        {
                                inDISP_LogPrintf("szAMEXContactlessEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szAMEXContactlessEnable length = (%d)", (int)strlen(srCFGTRec.szAMEXContactlessEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szAMEXContactlessEnable[0], &srCFGTRec.szAMEXContactlessEnable[0], strlen(srCFGTRec.szAMEXContactlessEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetAMEXContactlessEnable
Date&Time       :
Describe        :
*/
int inSetAMEXContactlessEnable(char* szAMEXContactlessEnable)
{
        memset(srCFGTRec.szAMEXContactlessEnable, 0x00, sizeof(srCFGTRec.szAMEXContactlessEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szAMEXContactlessEnable == NULL || strlen(szAMEXContactlessEnable) <= 0 || strlen(szAMEXContactlessEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetAMEXContactlessEnable() ERROR !!");

                        if (szAMEXContactlessEnable == NULL)
                        {
                                inDISP_LogPrintf("szAMEXContactlessEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szAMEXContactlessEnable length = (%d)", (int)strlen(szAMEXContactlessEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szAMEXContactlessEnable[0], &szAMEXContactlessEnable[0], strlen(szAMEXContactlessEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetCUPContactlessEnable
Date&Time       :
Describe        :
*/
int inGetCUPContactlessEnable(char* szCUPContactlessEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCUPContactlessEnable == NULL || strlen(srCFGTRec.szCUPContactlessEnable) <= 0 || strlen(srCFGTRec.szCUPContactlessEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCUPContactlessEnable() ERROR !!");

                        if (szCUPContactlessEnable == NULL)
                        {
                                inDISP_LogPrintf("szCUPContactlessEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCUPContactlessEnable length = (%d)", (int)strlen(srCFGTRec.szCUPContactlessEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCUPContactlessEnable[0], &srCFGTRec.szCUPContactlessEnable[0], strlen(srCFGTRec.szCUPContactlessEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetCUPContactlessEnable
Date&Time       :
Describe        :
*/
int inSetCUPContactlessEnable(char* szCUPContactlessEnable)
{
        memset(srCFGTRec.szCUPContactlessEnable, 0x00, sizeof(srCFGTRec.szCUPContactlessEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCUPContactlessEnable == NULL || strlen(szCUPContactlessEnable) <= 0 || strlen(szCUPContactlessEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCUPContactlessEnable() ERROR !!");

                        if (szCUPContactlessEnable == NULL)
                        {
                                inDISP_LogPrintf("szCUPContactlessEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCUPContactlessEnable length = (%d)", (int)strlen(szCUPContactlessEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szCUPContactlessEnable[0], &szCUPContactlessEnable[0], strlen(szCUPContactlessEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetSmartPayContactlessEnable
Date&Time       :
Describe        :
*/
int inGetSmartPayContactlessEnable(char* szSmartPayContactlessEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szSmartPayContactlessEnable == NULL || strlen(srCFGTRec.szSmartPayContactlessEnable) <= 0 || strlen(srCFGTRec.szSmartPayContactlessEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetSmartPayContactlessEnable() ERROR !!");

                        if (szSmartPayContactlessEnable == NULL)
                        {
                                inDISP_LogPrintf("szSmartPayContactlessEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSmartPayContactlessEnable length = (%d)", (int)strlen(srCFGTRec.szSmartPayContactlessEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSmartPayContactlessEnable[0], &srCFGTRec.szSmartPayContactlessEnable[0], strlen(srCFGTRec.szSmartPayContactlessEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetSmartPayContactlessEnable
Date&Time       :
Describe        :
*/
int inSetSmartPayContactlessEnable(char* szSmartPayContactlessEnable)
{
        memset(srCFGTRec.szSmartPayContactlessEnable, 0x00, sizeof(srCFGTRec.szSmartPayContactlessEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSmartPayContactlessEnable == NULL || strlen(szSmartPayContactlessEnable) <= 0 || strlen(szSmartPayContactlessEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetSmartPayContactlessEnable() ERROR !!");

                        if (szSmartPayContactlessEnable == NULL)
                        {
                                inDISP_LogPrintf("szSmartPayContactlessEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSmartPayContactlessEnable length = (%d)", (int)strlen(szSmartPayContactlessEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szSmartPayContactlessEnable[0], &szSmartPayContactlessEnable[0], strlen(szSmartPayContactlessEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetPayItemEnable
Date&Time       :2017/4/12 下午 1:31
Describe        :
*/
int inGetPayItemEnable(char* szPay_Item_Enable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPay_Item_Enable == NULL || strlen(srCFGTRec.szPay_Item_Enable) <= 0 || strlen(srCFGTRec.szPay_Item_Enable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPay_Item_Enable() ERROR !!");

                        if (szPay_Item_Enable == NULL)
                        {
                                inDISP_LogPrintf("szPay_Item_Enable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPay_Item_Enable length = (%d)", (int)strlen(srCFGTRec.szPay_Item_Enable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPay_Item_Enable[0], &srCFGTRec.szPay_Item_Enable[0], strlen(srCFGTRec.szPay_Item_Enable));

        return (VS_SUCCESS);
}

/*
Function        :inSetPayItemEnable
Date&Time       :2017/4/12 下午 1:32
Describe        :
*/
int inSetPayItemEnable(char* szPay_Item_Enable)
{
        memset(srCFGTRec.szPay_Item_Enable, 0x00, sizeof(srCFGTRec.szPay_Item_Enable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPay_Item_Enable == NULL || strlen(szPay_Item_Enable) <= 0 || strlen(szPay_Item_Enable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPay_Item_Enable() ERROR !!");

                        if (szPay_Item_Enable == NULL)
                        {
                                inDISP_LogPrintf("szPay_Item_Enable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPay_Item_Enable length = (%d)", (int)strlen(szPay_Item_Enable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szPay_Item_Enable[0], &szPay_Item_Enable[0], strlen(szPay_Item_Enable));

        return (VS_SUCCESS);
}

/*
Function        :inGetStore_Stub_CardNo_Truncate_Enable
Date&Time       :2017/4/12 下午 1:32
Describe        :
*/
int inGetStore_Stub_CardNo_Truncate_Enable(char* szStore_Stub_CardNo_Truncate_Enable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szStore_Stub_CardNo_Truncate_Enable == NULL || strlen(srCFGTRec.szStore_Stub_CardNo_Truncate_Enable) <= 0 || strlen(srCFGTRec.szStore_Stub_CardNo_Truncate_Enable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetStore_Stub_CardNo_Truncate_Enable() ERROR !!");

                        if (szStore_Stub_CardNo_Truncate_Enable == NULL)
                        {
                                inDISP_LogPrintf("szStore_Stub_CardNo_Truncate_Enable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szStore_Stub_CardNo_Truncate_Enable length = (%d)", (int)strlen(srCFGTRec.szStore_Stub_CardNo_Truncate_Enable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szStore_Stub_CardNo_Truncate_Enable[0], &srCFGTRec.szStore_Stub_CardNo_Truncate_Enable[0], strlen(srCFGTRec.szStore_Stub_CardNo_Truncate_Enable));

        return (VS_SUCCESS);
}

/*
Function        :inSetStore_Stub_CardNo_Truncate_Enable
Date&Time       :2017/4/12 下午 1:32
Describe        :
*/
int inSetStore_Stub_CardNo_Truncate_Enable(char* szStore_Stub_CardNo_Truncate_Enable)
{
        memset(srCFGTRec.szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(srCFGTRec.szStore_Stub_CardNo_Truncate_Enable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szStore_Stub_CardNo_Truncate_Enable == NULL || strlen(szStore_Stub_CardNo_Truncate_Enable) <= 0 || strlen(szStore_Stub_CardNo_Truncate_Enable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetStore_Stub_CardNo_Truncate_Enable() ERROR !!");

                        if (szStore_Stub_CardNo_Truncate_Enable == NULL)
                        {
                                inDISP_LogPrintf("szStore_Stub_CardNo_Truncate_Enable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szStore_Stub_CardNo_Truncate_Enable length = (%d)", (int)strlen(szStore_Stub_CardNo_Truncate_Enable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szStore_Stub_CardNo_Truncate_Enable[0], &szStore_Stub_CardNo_Truncate_Enable[0], strlen(szStore_Stub_CardNo_Truncate_Enable));

        return (VS_SUCCESS);
}

/*
Function        :inGetIntegrate_Device
Date&Time       :2017/4/12 下午 1:34
Describe        :
*/
int inGetIntegrate_Device(char* szIntegrate_Device)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szIntegrate_Device == NULL || strlen(srCFGTRec.szIntegrate_Device) <= 0 || strlen(srCFGTRec.szIntegrate_Device) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetIntegrate_Device() ERROR !!");

                        if (szIntegrate_Device == NULL)
                        {
                                inDISP_LogPrintf("szIntegrate_Device == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIntegrate_Device length = (%d)", (int)strlen(srCFGTRec.szIntegrate_Device));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szIntegrate_Device[0], &srCFGTRec.szIntegrate_Device[0], strlen(srCFGTRec.szIntegrate_Device));

        return (VS_SUCCESS);
}

/*
Function        :inSetIntegrate_Device
Date&Time       :2017/4/12 下午 1:34
Describe        :
*/
int inSetIntegrate_Device(char* szIntegrate_Device)
{
        memset(srCFGTRec.szIntegrate_Device, 0x00, sizeof(srCFGTRec.szIntegrate_Device));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szIntegrate_Device == NULL || strlen(szIntegrate_Device) <= 0 || strlen(szIntegrate_Device) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetIntegrate_Device() ERROR !!");

                        if (szIntegrate_Device == NULL)
                        {
                                inDISP_LogPrintf("szIntegrate_Device == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIntegrate_Device length = (%d)", (int)strlen(szIntegrate_Device));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szIntegrate_Device[0], &szIntegrate_Device[0], strlen(szIntegrate_Device));

        return (VS_SUCCESS);
}

/*
Function        :inGetFES_ID
Date&Time       :2017/4/12 下午 1:35
Describe        :
*/
int inGetFES_ID(char* szFES_ID)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szFES_ID == NULL || strlen(srCFGTRec.szFES_ID) <= 0 || strlen(srCFGTRec.szFES_ID) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetFES_ID() ERROR !!");

                        if (szFES_ID == NULL)
                        {
                                inDISP_LogPrintf("szFES_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFES_ID length = (%d)", (int)strlen(srCFGTRec.szFES_ID));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFES_ID[0], &srCFGTRec.szFES_ID[0], strlen(srCFGTRec.szFES_ID));

        return (VS_SUCCESS);
}

/*
Function        :inSetFES_ID
Date&Time       :2017/4/12 下午 1:35
Describe        :
*/
int inSetFES_ID(char* szFES_ID)
{
        memset(srCFGTRec.szFES_ID, 0x00, sizeof(srCFGTRec.szFES_ID));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFES_ID == NULL || strlen(szFES_ID) <= 0 || strlen(szFES_ID) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetFES_ID() ERROR !!");

                        if (szFES_ID == NULL)
                        {
                                inDISP_LogPrintf("szFES_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFES_ID length = (%d)", (int)strlen(szFES_ID));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szFES_ID[0], &szFES_ID[0], strlen(szFES_ID));

        return (VS_SUCCESS);
}

/*
Function        :inGetIntegrate_Device_AP_ID
Date&Time       :2017/4/12 下午 1:35
Describe        :
*/
int inGetIntegrate_Device_AP_ID(char* szIntegrate_Device_AP_ID)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szIntegrate_Device_AP_ID == NULL || strlen(srCFGTRec.szIntegrate_Device_AP_ID) <= 0 || strlen(srCFGTRec.szIntegrate_Device_AP_ID) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetIntegrate_Device_AP_ID() ERROR !!");

                        if (szIntegrate_Device_AP_ID == NULL)
                        {
                                inDISP_LogPrintf("szIntegrate_Device_AP_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIntegrate_Device_AP_ID length = (%d)", (int)strlen(srCFGTRec.szIntegrate_Device_AP_ID));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szIntegrate_Device_AP_ID[0], &srCFGTRec.szIntegrate_Device_AP_ID[0], strlen(srCFGTRec.szIntegrate_Device_AP_ID));

        return (VS_SUCCESS);
}

/*
Function        :inSetIntegrate_Device_AP_ID
Date&Time       :2017/4/12 下午 1:35
Describe        :
*/
int inSetIntegrate_Device_AP_ID(char* szIntegrate_Device_AP_ID)
{
        memset(srCFGTRec.szIntegrate_Device_AP_ID, 0x00, sizeof(srCFGTRec.szIntegrate_Device_AP_ID));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szIntegrate_Device_AP_ID == NULL || strlen(szIntegrate_Device_AP_ID) <= 0 || strlen(szIntegrate_Device_AP_ID) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetIntegrate_Device_AP_ID() ERROR !!");

                        if (szIntegrate_Device_AP_ID == NULL)
                        {
                                inDISP_LogPrintf("szIntegrate_Device_AP_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIntegrate_Device_AP_ID length = (%d)", (int)strlen(szIntegrate_Device_AP_ID));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szIntegrate_Device_AP_ID[0], &szIntegrate_Device_AP_ID[0], strlen(szIntegrate_Device_AP_ID));

        return (VS_SUCCESS);
}

/*
Function        :inGetShort_Receipt_Mode
Date&Time       :2017/4/12 下午 1:36
Describe        :
*/
int inGetShort_Receipt_Mode(char* szShort_Receipt_Mode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szShort_Receipt_Mode == NULL || strlen(srCFGTRec.szShort_Receipt_Mode) <= 0 || strlen(srCFGTRec.szShort_Receipt_Mode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetShort_Receipt_Mode() ERROR !!");

                        if (szShort_Receipt_Mode == NULL)
                        {
                                inDISP_LogPrintf("szShort_Receipt_Mode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szShort_Receipt_Mode length = (%d)", (int)strlen(srCFGTRec.szShort_Receipt_Mode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szShort_Receipt_Mode[0], &srCFGTRec.szShort_Receipt_Mode[0], strlen(srCFGTRec.szShort_Receipt_Mode));

        return (VS_SUCCESS);
}

/*
Function        :inSetShort_Receipt_Mode
Date&Time       :2017/4/12 下午 1:36
Describe        :
*/
int inSetShort_Receipt_Mode(char* szShort_Receipt_Mode)
{
        memset(srCFGTRec.szShort_Receipt_Mode, 0x00, sizeof(srCFGTRec.szShort_Receipt_Mode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szShort_Receipt_Mode == NULL || strlen(szShort_Receipt_Mode) <= 0 || strlen(szShort_Receipt_Mode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetShort_Receipt_Mode() ERROR !!");

                        if (szShort_Receipt_Mode == NULL)
                        {
                                inDISP_LogPrintf("szShort_Receipt_Mode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szShort_Receipt_Mode length = (%d)", (int)strlen(szShort_Receipt_Mode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szShort_Receipt_Mode[0], &szShort_Receipt_Mode[0], strlen(szShort_Receipt_Mode));

        return (VS_SUCCESS);
}

/*
Function        :inGetI_FES_Mode
Date&Time       :2017/4/12 下午 1:36
Describe        :
*/
int inGetI_FES_Mode(char* szI_FES_Mode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szI_FES_Mode == NULL || strlen(srCFGTRec.szI_FES_Mode) <= 0 || strlen(srCFGTRec.szI_FES_Mode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetI_FES_Mode() ERROR !!");

                        if (szI_FES_Mode == NULL)
                        {
                                inDISP_LogPrintf("szI_FES_Mode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szI_FES_Mode length = (%d)", (int)strlen(srCFGTRec.szI_FES_Mode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szI_FES_Mode[0], &srCFGTRec.szI_FES_Mode[0], strlen(srCFGTRec.szI_FES_Mode));

        return (VS_SUCCESS);
}

/*
Function        :inSetI_FES_Mode
Date&Time       :2017/4/12 下午 1:36
Describe        :
*/
int inSetI_FES_Mode(char* szI_FES_Mode)
{
        memset(srCFGTRec.szI_FES_Mode, 0x00, sizeof(srCFGTRec.szI_FES_Mode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szI_FES_Mode == NULL || strlen(szI_FES_Mode) <= 0 || strlen(szI_FES_Mode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetI_FES_Mode() ERROR !!");

                        if (szI_FES_Mode == NULL)
                        {
                                inDISP_LogPrintf("szI_FES_Mode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szI_FES_Mode length = (%d)", (int)strlen(szI_FES_Mode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szI_FES_Mode[0], &szI_FES_Mode[0], strlen(szI_FES_Mode));

        return (VS_SUCCESS);
}

/*
Function        :inGetDHCP_Mode
Date&Time       :2017/4/12 下午 1:36
Describe        :
*/
int inGetDHCP_Mode(char* szDHCP_Mode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szDHCP_Mode == NULL || strlen(srCFGTRec.szDHCP_Mode) <= 0 || strlen(srCFGTRec.szDHCP_Mode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetDHCP_Mode() ERROR !!");

                        if (szDHCP_Mode == NULL)
                        {
                                inDISP_LogPrintf("szDHCP_Mode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDHCP_Mode length = (%d)", (int)strlen(srCFGTRec.szDHCP_Mode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDHCP_Mode[0], &srCFGTRec.szDHCP_Mode[0], strlen(srCFGTRec.szDHCP_Mode));

        return (VS_SUCCESS);
}

/*
Function        :inSetDHCP_Mode
Date&Time       :2017/4/12 下午 1:36
Describe        :
*/
int inSetDHCP_Mode(char* szDHCP_Mode)
{
        memset(srCFGTRec.szDHCP_Mode, 0x00, sizeof(srCFGTRec.szDHCP_Mode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szDHCP_Mode == NULL || strlen(szDHCP_Mode) <= 0 || strlen(szDHCP_Mode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetDHCP_Mode() ERROR !!");

                        if (szDHCP_Mode == NULL)
                        {
                                inDISP_LogPrintf("szDHCP_Mode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDHCP_Mode length = (%d)", (int)strlen(szDHCP_Mode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szDHCP_Mode[0], &szDHCP_Mode[0], strlen(szDHCP_Mode));

        return (VS_SUCCESS);
}

/*
Function        :inGetESVC_Priority
Date&Time       :2018/1/17 下午 6:28
Describe        :
*/
int inGetESVC_Priority(char* szESVC_Priority)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szESVC_Priority == NULL || strlen(srCFGTRec.szESVC_Priority) <= 0 || strlen(srCFGTRec.szESVC_Priority) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetESVC_Priority() ERROR !!");

                        if (szESVC_Priority == NULL)
                        {
                                inDISP_LogPrintf("szESVC_Priority == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESVC_Priority length = (%d)", (int)strlen(srCFGTRec.szESVC_Priority));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szESVC_Priority[0], &srCFGTRec.szESVC_Priority[0], strlen(srCFGTRec.szESVC_Priority));

        return (VS_SUCCESS);
}

/*
Function        :inSetESVC_Priority
Date&Time       :2018/1/17 下午 6:28
Describe        :
*/
int inSetESVC_Priority(char* szESVC_Priority)
{
        memset(srCFGTRec.szESVC_Priority, 0x00, sizeof(srCFGTRec.szESVC_Priority));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szESVC_Priority == NULL || strlen(szESVC_Priority) <= 0 || strlen(szESVC_Priority) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetESVC_Priority() ERROR !!");

                        if (szESVC_Priority == NULL)
                        {
                                inDISP_LogPrintf("szESVC_Priority == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESVC_Priority length = (%d)", (int)strlen(szESVC_Priority));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szESVC_Priority[0], &szESVC_Priority[0], strlen(szESVC_Priority));

        return (VS_SUCCESS);
}

/*
Function        :inGetDFS_Contactless_Enable
Date&Time       :2017/4/12 下午 1:36
Describe        :
*/
int inGetDFS_Contactless_Enable(char* szDFS_Contactless_Enable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szDFS_Contactless_Enable == NULL || strlen(srCFGTRec.szDFS_Contactless_Enable) <= 0 || strlen(srCFGTRec.szDFS_Contactless_Enable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetDFS_Contactless_Enable() ERROR !!");

                        if (szDFS_Contactless_Enable == NULL)
                        {
                                inDISP_LogPrintf("szDFS_Contactless_Enable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDFS_Contactless_Enable length = (%d)", (int)strlen(srCFGTRec.szDFS_Contactless_Enable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDFS_Contactless_Enable[0], &srCFGTRec.szDFS_Contactless_Enable[0], strlen(srCFGTRec.szDFS_Contactless_Enable));

        return (VS_SUCCESS);
}

/*
Function        :inSetDFS_Contactless_Enable
Date&Time       :2017/4/12 下午 1:36
Describe        :
*/
int inSetDFS_Contactless_Enable(char* szDFS_Contactless_Enable)
{
        memset(srCFGTRec.szDFS_Contactless_Enable, 0x00, sizeof(srCFGTRec.szDFS_Contactless_Enable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szDFS_Contactless_Enable == NULL || strlen(szDFS_Contactless_Enable) <= 0 || strlen(szDFS_Contactless_Enable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetDFS_Contactless_Enable() ERROR !!");

                        if (szDFS_Contactless_Enable == NULL)
                        {
                                inDISP_LogPrintf("szDFS_Contactless_Enable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDFS_Contactless_Enable length = (%d)", (int)strlen(szDFS_Contactless_Enable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szDFS_Contactless_Enable[0], &szDFS_Contactless_Enable[0], strlen(szDFS_Contactless_Enable));

        return (VS_SUCCESS);
}

/*
Function        :inGetCloud_MFES
Date&Time       :2018/5/8 下午 3:04
Describe        :
*/
int inGetCloud_MFES(char* szCloud_MFES)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCloud_MFES == NULL || strlen(srCFGTRec.szCloud_MFES) <= 0 || strlen(srCFGTRec.szCloud_MFES) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCloud_MFES() ERROR !!");

                        if (szCloud_MFES == NULL)
                        {
                                inDISP_LogPrintf("szCloud_MFES == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCloud_MFES length = (%d)", (int)strlen(srCFGTRec.szCloud_MFES));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCloud_MFES[0], &srCFGTRec.szCloud_MFES[0], strlen(srCFGTRec.szCloud_MFES));

        return (VS_SUCCESS);
}

/*
Function        :inSetCloud_MFES
Date&Time       :2018/5/8 下午 3:05
Describe        :
*/
int inSetCloud_MFES(char* szCloud_MFES)
{
        memset(srCFGTRec.szCloud_MFES, 0x00, sizeof(srCFGTRec.szCloud_MFES));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCloud_MFES == NULL || strlen(szCloud_MFES) <= 0 || strlen(szCloud_MFES) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCloud_MFES() ERROR !!");

                        if (szCloud_MFES == NULL)
                        {
                                inDISP_LogPrintf("szCloud_MFES == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCloud_MFES length = (%d)", (int)strlen(szCloud_MFES));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szCloud_MFES[0], &szCloud_MFES[0], strlen(szCloud_MFES));

        return (VS_SUCCESS);
}

/*
Function        :inFunc_Edit_CFGT_Table
Date&Time       :2017/3/28 下午 3:09
Describe        :
*/
int inCFGT_Edit_CFGT_Table(void)
{
	TABLE_GET_SET_TABLE CFGT_FUNC_TABLE[] =
	{
		{"szCustomIndicator"			,inGetCustomIndicator			,inSetCustomIndicator			},
		{"szNCCCFESMode"			,inGetNCCCFESMode			,inSetNCCCFESMode			},
		{"szCommMode"				,inGetCommMode				,inSetCommMode				},
		{"szDialBackupEnable"			,inGetDialBackupEnable			,inSetDialBackupEnable			},
		{"szEncryptMode"			,inGetEncryptMode			,inSetEncryptMode			},
		{"szSplitTransCheckEnable"		,inGetSplitTransCheckEnable		,inSetSplitTransCheckEnable		},
		{"szCityName"				,inGetCityName				,inSetCityName				},
		{"szStoreIDEnable"			,inGetStoreIDEnable			,inSetStoreIDEnable			},
		{"szMinStoreIDLen"			,inGetMinStoreIDLen			,inSetMinStoreIDLen			},		
		{"szMaxStoreIDLen"			,inGetMaxStoreIDLen			,inSetMaxStoreIDLen			},		
		{"szPrompt"			,inGetPrompt			,inSetPrompt			},
		{"szPromptData"			,inGetPromptData			,inSetPromptData			},
		{"szECREnable"				,inGetECREnable				,inSetECREnable				},
		{"szECRCardNoTruncateEnable"		,inGetECRCardNoTruncateEnable		,inSetECRCardNoTruncateEnable		},
		{"szECRExpDateReturnEnable"		,inGetECRExpDateReturnEnable		,inSetECRExpDateReturnEnable		},
		{"szProductCodeEnable"			,inGetProductCodeEnable			,inSetProductCodeEnable			},
		{"szPrtSlogan"				,inGetPrtSlogan				,inSetPrtSlogan				},
		{"szSloganStartDate"			,inGetSloganStartDate			,inSetSloganStartDate			},
		{"szSloganEndDate"			,inGetSloganEndDate			,inSetSloganEndDate			},
		{"szSloganPrtPosition"			,inGetSloganPrtPosition			,inSetSloganPrtPosition			},
		{"szPrtMode"				,inGetPrtMode				,inSetPrtMode				},
		{"szContactlessEnable"			,inGetContactlessEnable			,inSetContactlessEnable			},
		{"szVISAPaywaveEnable"			,inGetVISAPaywaveEnable			,inSetVISAPaywaveEnable			},
		{"szJCBJspeedyEnable"			,inGetJCBJspeedyEnable			,inSetJCBJspeedyEnable			},
		{"szMCPaypassEnable"			,inGetMCPaypassEnable			,inSetMCPaypassEnable			},
		{"szCUPRefundLimit"			,inGetCUPRefundLimit			,inSetCUPRefundLimit			},
		{"szCUPKeyExchangeTimes"		,inGetCUPKeyExchangeTimes		,inSetCUPKeyExchangeTimes		},
		{"szMACEnable"				,inGetMACEnable				,inSetMACEnable				},
		{"szPinpadMode"				,inGetPinpadMode			,inSetPinpadMode			},
		{"szFORCECVV2"				,inGetFORCECVV2				,inSetFORCECVV2				},
		{"sz4DBC"				,inGet4DBC				,inSet4DBC				},		
		{"szSpecialCardRangeEnable"		,inGetSpecialCardRangeEnable		,inSetSpecialCardRangeEnable		},
		{"szPrtMerchantLogo"			,inGetPrtMerchantLogo			,inSetPrtMerchantLogo			},
		{"szPrtMerchantName"			,inGetPrtMerchantName			,inSetPrtMerchantName			},
		{"szPrtNotice"				,inGetPrtNotice				,inSetPrtNotice				},
		{"szPrtPromotionAdv"			,inGetPrtPromotionAdv			,inSetPrtPromotionAdv				},
		{"szElecCommerceFlag"			,inGetElecCommerceFlag			,inSetElecCommerceFlag			},
		{"szDccFlowVersion"			,inGetDccFlowVersion			,inSetDccFlowVersion			},
		{"szSupDccVisa"				,inGetSupDccVisa			,inSetSupDccVisa			},
		{"szSupDccMasterCard"			,inGetSupDccMasterCard			,inSetSupDccMasterCard			},
		{"szDHCPRetryTimes"			,inGetDHCPRetryTimes			,inSetDHCPRetryTimes			},		
		{"szBankID"				,inGetBankID			,inSetBankID			},
		{"szVEPSFlag"			,inGetVEPSFlag			,inSetVEPSFlag			},
		{"szComportMode"		,inGetComportMode		,inSetComportMode			},
		{"szFTPMode"			,inGetFTPMode			,inSetFTPMode			},
		{"szContlessMode"		,inGetContlessMode		,inSetContlessMode			},
		{"szBarCodeReaderEnable"		,inGetBarCodeReaderEnable		,inSetBarCodeReaderEnable		},
		{"szEMVPINBypassEnable"			,inGetEMVPINBypassEnable		,inSetEMVPINBypassEnable		},
		{"szCUPOnlinePINEntryTimeout"		,inGetCUPOnlinePINEntryTimeout		,inSetCUPOnlinePINEntryTimeout		},
		{"szSignPadMode"			,inGetSignPadMode			,inSetSignPadMode			},
		{"szESCPrintMerchantCopy"		,inGetESCPrintMerchantCopy		,inSetESCPrintMerchantCopy		},
		{"szESCPrintMerchantCopyStartDate"	,inGetESCPrintMerchantCopyStartDate	,inSetESCPrintMerchantCopyStartDate	},
		{"szESCPrintMerchantCopyEndDate"	,inGetESCPrintMerchantCopyEndDate	,inSetESCPrintMerchantCopyEndDate	},
		{"szESCReciptUploadUpLimit"		,inGetESCReciptUploadUpLimit		,inSetESCReciptUploadUpLimit		},
		{"szContactlessReaderMode"		,inGetContactlessReaderMode		,inSetContactlessReaderMode		},
		{"szTMSDownloadMode"			,inGetTMSDownloadMode			,inSetTMSDownloadMode			},
		{"szAMEXContactlessEnable"		,inGetAMEXContactlessEnable		,inSetAMEXContactlessEnable		},
		{"szCUPContactlessEnable"		,inGetCUPContactlessEnable		,inSetCUPContactlessEnable		},
		{"szSmartPayContactlessEnable"		,inGetSmartPayContactlessEnable		,inSetSmartPayContactlessEnable		},
		{"szPayItemEnable"			,inGetPayItemEnable			,inSetPayItemEnable			},
		{"szStore_Stub_CardNo_Truncate_Enable"	,inGetStore_Stub_CardNo_Truncate_Enable	,inSetStore_Stub_CardNo_Truncate_Enable	},
		{"szIntegrate_Device"			,inGetIntegrate_Device			,inSetIntegrate_Device			},
		{"szFES_ID"				,inGetFES_ID				,inSetFES_ID				},
		{"szIntegrate_Device_AP_ID"		,inGetIntegrate_Device_AP_ID		,inSetIntegrate_Device_AP_ID		},
		{"szShort_Receipt_Mode"			,inGetShort_Receipt_Mode		,inSetShort_Receipt_Mode		},
		{"szI_FES_Mode"				,inGetI_FES_Mode			,inSetI_FES_Mode			},
		{"szDHCP_Mode"				,inGetDHCP_Mode				,inSetDHCP_Mode				},
		{"szESVC_Priority"			,inGetESVC_Priority			,inSetESVC_Priority			},
		{"szDFS_Contactless_Enable"		,inGetDFS_Contactless_Enable		,inSetDFS_Contactless_Enable		},
		{"szCloud_MFES"				,inGetCloud_MFES			,inSetCloud_MFES			},
		{""},
	};
	int	inRetVal;
	char	szKey;
	
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont_Color("是否更改CFGT", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
			break;
		}
		
	}
	
	/* 如果改到感應開關，而原先開機時沒initial感應器，遇到關LED燈時會crash START */
	char		szContactlessEnable[2 + 1];
	char		szContactlessReaderMode[2 + 1];
	unsigned char	usCTLSInitFlag = VS_FALSE;
	
	memset(szContactlessEnable, 0x00, sizeof(szContactlessEnable));
	memset(szContactlessReaderMode, 0x00, sizeof(szContactlessReaderMode));
	
	inGetContactlessEnable(szContactlessEnable);
	inGetContactlessReaderMode(szContactlessReaderMode);
	
	if (memcmp(szContactlessEnable, "Y", strlen("Y")) != 0 || memcmp(szContactlessReaderMode, "0", strlen("0")) == 0)
	{
		usCTLSInitFlag = VS_TRUE;
	}
	/* 如果改到感應開關，而原先開機時沒initial感應器，遇到關LED燈時會crash END */
	
	inFunc_Edit_Table_Tag(CFGT_FUNC_TABLE);
	inSaveCFGTRec(0);
	
	/* 如果改到感應開關，而原先開機時沒initial感應器，遇到關LED燈時會crash START */
	memset(szContactlessEnable, 0x00, sizeof(szContactlessEnable));
	memset(szContactlessReaderMode, 0x00, sizeof(szContactlessReaderMode));
	
	inGetContactlessEnable(szContactlessEnable);
	inGetContactlessReaderMode(szContactlessReaderMode);
	
	if (memcmp(szContactlessEnable, "Y", strlen("Y")) == 0 && memcmp(szContactlessReaderMode, "0", strlen("0")) != 0 && usCTLSInitFlag == VS_TRUE)
	{
		inCTLS_InitReader_Flow();
	}
	/* 如果改到感應開關，而原先開機時沒initial感應器，遇到關LED燈時會crash END */
	
	return	(VS_SUCCESS);
}
