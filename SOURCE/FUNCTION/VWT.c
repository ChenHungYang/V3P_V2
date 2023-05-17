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
#include "VWT.h"

static  VWT_REC srVWTRec;	/* construct VWT record */
extern  int	ginDebug;	/* Debug使用 extern */

/*
Function        :inLoadVWTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀VWT檔案，inVWTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadVWTRec(int inVWTRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆VWT檔案 */
        char            szVWTRec[_SIZE_VWT_REC_ + 1];           /* 暫存, 放各個欄位檔案 */
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnVWTLength = 0;                        /* VWT總長度 */
        long            lnReadLength;                           /* 記錄每次要從VWT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;                 /* inRec記錄讀到第幾筆, i為目前從VWT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadVWTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadVWTRec(%d) START!!", inVWTRec);
                inDISP_LogPrintf(szErrorMsg);
        }

        /* 判斷傳進來的inVWTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inVWTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inVWTRec < 0:(index = %d) ERROR!!", inVWTRec);
                        inDISP_LogPrintf(szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open VWT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_VWT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnVWTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_VWT_FILE_NAME_);

	if (lnVWTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnVWTLength + 1);
        uszTemp = malloc(lnVWTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnVWTLength + 1);
        memset(uszTemp, 0x00, lnVWTLength + 1);

         /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnVWTLength;

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
         *i為目前從VWT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnVWTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到VWT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                      	/* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
			inSearchResult = 1;

			/* 清空uszReadData */
                        memset(uszReadData, 0x00, lnVWTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
			memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
			/* 因為inVWT_Rec的index從0開始，所以inVWT_Rec要+1 */
                        if (inRec == (inVWTRec + 1))
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
         * 如果沒有inVWTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inVWTRec + 1) || inSearchResult == -1)
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
        memset(&srVWTRec, 0x00, sizeof(srVWTRec));
	/*
         * 以下pattern為存入VWT_Rec
         * i為VWT的第幾個字元
         * 存入VWT_Rec
         */
        i = 0;


        /* 01_各非接觸式卡片索引 */
	/* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
	k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szContactlessSchemeIndex[0], &szVWTRec[0], k - 1);
        }

        /* 02_Terminal Type */
	/* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szTerminalType[0], &szVWTRec[0], k - 1);
        }

        /* 03_CVM Requirement */
	/* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szCVMRequirement[0], &szVWTRec[0], k - 1);
        }

        /* 04_VLP Support Indicator */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szVLPSupportIndicator[0], &szVWTRec[0], k - 1);
        }

        /* 05_Terminal Country Code */
	/* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szTerminalCountryCode[0], &szVWTRec[0], k - 1);
        }

        /* 06_Transaction Currency Code */
	/* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szTransactionCurrencyCode[0], &szVWTRec[0], k - 1);
        }

        /* 07_Transaction Type */
	/* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szTransactionType[0], &szVWTRec[0], k - 1);
        }

        /* 08_Contactless Transaction Limit */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szContactlessTransactionLimit[0], &szVWTRec[0], k - 1);
        }

        /* 09_CVM Required Limit */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szCVMRequiredLimit[0], &szVWTRec[0], k - 1);
        }

        /* 10_Contactless Floor Limit */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szContactlessFloorLimit[0], &szVWTRec[0], k - 1);
        }

        /* 11_Enhanced DDA Version Num */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szEnhancedDDAVersionNum[0], &szVWTRec[0], k - 1);
        }

        /* 12_Display Offline Funds */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szDisplayOfflineFunds[0], &szVWTRec[0], k - 1);
        }

        /* 13_Terminal Transaction Qualifier */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szTerminalTransactionQualifier[0], &szVWTRec[0], k - 1);
        }

        /* 14_Paypass Terminal Type */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassTerminalType[0], &szVWTRec[0], k - 1);
        }

        /* 15_Paypass Terminal Capabilities */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassTerminalCapabilities[0], &szVWTRec[0], k - 1);
        }

        /* 16_Paypass Terminal Country Code */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassTerminalCountryCode[0], &szVWTRec[0], k - 1);
        }

        /* 17_Paypass Transaction Currency Code */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassTransactionCurrencyCode[0], &szVWTRec[0], k - 1);
        }

        /* 18_Paypass Default TAC */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassDefaultTAC[0], &szVWTRec[0], k - 1);
        }

        /* 19_Paypass Denial TAC */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassDenialTAC[0], &szVWTRec[0], k - 1);
        }

        /* 20_Paypass Online TAC */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassOnlineTAC[0], &szVWTRec[0], k - 1);
        }

        /* 21_Paypass Default TDOL */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassDefaultTDOL[0], &szVWTRec[0], k - 1);
        }

        /* 22_Paypass EMV Floor Limit */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassEMVFloorLimit[0], &szVWTRec[0], k - 1);
        }

        /* 23_Paypass Random Selection Threshold */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassRandomSelectionThreshold[0], &szVWTRec[0], k - 1);
        }

        /* 24_Paypass Target Percent for Random Selection */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassTargetPercentforRandomSelection[0], &szVWTRec[0], k - 1);
        }

        /* 25_Paypass Max Target Percent for Random Selection */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassMaxTargetPercentforRandomSelection[0], &szVWTRec[0], k - 1);
        }

        /* 26_Paypass CVM Required Limit */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassCVMRequiredLimit[0], &szVWTRec[0], k - 1);
        }

        /* 27_Merchant Category Code */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szMerchantCategoryCode[0], &szVWTRec[0], k - 1);
        }

        /* 28_Transaction Category Code */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szTransactionCategoryCode[0], &szVWTRec[0], k - 1);
        }

        /* 29_Combination Option */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szCombinationOption[0], &szVWTRec[0], k - 1);
        }

        /* 30_Terminal Interchange Profile  */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szTerminalInterchangeProfile[0], &szVWTRec[0], k - 1);
        }

        /* 31_PayWave AP ID */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPayWaveAPID[0], &szVWTRec[0], k - 1);
        }

        /* 32_PayWave AP ID Contactless Transaction Limit */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPayWaveAPIDContactlessTransactionLimit[0], &szVWTRec[0], k - 1);
        }

        /* 33_PayWave AP ID CVM Required Limit */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPayWaveAPIDCVMRequiredLimit[0], &szVWTRec[0], k - 1);
        }

        /* 34_PayWave AP ID Contactless Floor Limit */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srVWTRec.szPayWaveAPIDContactlessFloorLimit[0], &szVWTRec[0], k - 1);
        }

        /* release */
	/* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadVWTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadVWTRec(%d) END!!", inVWTRec);
                inDISP_LogPrintf(szErrorMsg);
                inDISP_LogPrintf("----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveVWTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveVWTRec(int inVWTRec)
{
 	unsigned long   uldat_Handle;   			/* FILE Handle */
	int inRecCount = 0, inTempIndex = 0;
	int     	inRetVal;
	int     	i = 0;

	char szTempTitle[] = "inSaveVWTRec";
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
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inSaveVWTRec INIT" );
#endif

	inTempIndex = inVWTRec;
	
	memset(szTempFIleName, 0x00, sizeof(szTempFIleName));
	strcat( szTempFIleName, _VWT_FILE_NAME_);
	
	memset(szTempBakFileName, 0x00, sizeof(szTempBakFileName));
	strcat( szTempBakFileName, _VWT_FILE_NAME_BAK_);
	
	inDISP_LogPrintfWithFlag("  %s Index[%d] ", szTempTitle, inTempIndex);

      	/* 開啟原檔案HDPT.dat */
	inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_VWT_FILE_NAME_);

	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  %s *Error* [%d] ",szTempTitle, inRetVal);
		inFunc_EDCLock();
		return(VS_ERROR);
	}

	usRetVal = CTOS_FileGetSize((unsigned char *)_VWT_FILE_NAME_, &ulFileSize);	
	
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
	uszWriteBuff_Record = malloc(_SIZE_VWT_REC_ + _SIZE_VWT_COMMA_0D0A_);
	memset(uszWriteBuff_Record, 0x00, _SIZE_VWT_REC_ + _SIZE_VWT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

	/* uszRead_Total_Buff儲存VWT.dat全部資料 */
	uszRead_Total_Buff = malloc(ulFileSize + 1);
	memset(uszRead_Total_Buff, 0x00, ulFileSize + 1);

	ulPackCount = 0; /* uszWriteBuff_Record的index位置 */
	/* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* ContactlessSchemeIndex */
	memcpy(&uszWriteBuff_Record[0], &srVWTRec.szContactlessSchemeIndex[0], strlen(srVWTRec.szContactlessSchemeIndex));
	ulPackCount += strlen(srVWTRec.szContactlessSchemeIndex);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TerminalType */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szTerminalType[0], strlen(srVWTRec.szTerminalType));
	ulPackCount += strlen(srVWTRec.szTerminalType);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* CVMRequirement */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szCVMRequirement[0], strlen(srVWTRec.szCVMRequirement));
	ulPackCount += strlen(srVWTRec.szCVMRequirement);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* VLPSupportIndicator */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szVLPSupportIndicator[0], strlen(srVWTRec.szVLPSupportIndicator));
	ulPackCount += strlen(srVWTRec.szVLPSupportIndicator);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TerminalCountryCode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szTerminalCountryCode[0], strlen(srVWTRec.szTerminalCountryCode));
	ulPackCount += strlen(srVWTRec.szTerminalCountryCode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TransactionCurrencyCode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szTransactionCurrencyCode[0], strlen(srVWTRec.szTransactionCurrencyCode));
	ulPackCount += strlen(srVWTRec.szTransactionCurrencyCode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TransactionType */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szTransactionType[0], strlen(srVWTRec.szTransactionType));
	ulPackCount += strlen(srVWTRec.szTransactionType);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* ContactlessTransactionLimit */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szContactlessTransactionLimit[0], strlen(srVWTRec.szContactlessTransactionLimit));
	ulPackCount += strlen(srVWTRec.szContactlessTransactionLimit);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* CVMRequiredLimit */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szCVMRequiredLimit[0], strlen(srVWTRec.szCVMRequiredLimit));
	ulPackCount += strlen(srVWTRec.szCVMRequiredLimit);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* ContactlessFloorLimit */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szContactlessFloorLimit[0], strlen(srVWTRec.szContactlessFloorLimit));
	ulPackCount += strlen(srVWTRec.szContactlessFloorLimit);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* EnhancedDDAVersionNum */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szEnhancedDDAVersionNum[0], strlen(srVWTRec.szEnhancedDDAVersionNum));
	ulPackCount += strlen(srVWTRec.szEnhancedDDAVersionNum);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* DisplayOfflineFunds */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szDisplayOfflineFunds[0], strlen(srVWTRec.szDisplayOfflineFunds));
	ulPackCount += strlen(srVWTRec.szDisplayOfflineFunds);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TerminalTransactionQualifier */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szTerminalTransactionQualifier[0], strlen(srVWTRec.szTerminalTransactionQualifier));
	ulPackCount += strlen(srVWTRec.szTerminalTransactionQualifier);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* PaypassTerminalType */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szPaypassTerminalType[0], strlen(srVWTRec.szPaypassTerminalType));
	ulPackCount += strlen(srVWTRec.szPaypassTerminalType);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* PaypassTerminalCapabilities */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szPaypassTerminalCapabilities[0], strlen(srVWTRec.szPaypassTerminalCapabilities));
	ulPackCount += strlen(srVWTRec.szPaypassTerminalCapabilities);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* PaypassTerminalCountryCode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szPaypassTerminalCountryCode[0], strlen(srVWTRec.szPaypassTerminalCountryCode));
	ulPackCount += strlen(srVWTRec.szPaypassTerminalCountryCode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* PaypassTransactionCurrencyCode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szPaypassTransactionCurrencyCode[0], strlen(srVWTRec.szPaypassTransactionCurrencyCode));
	ulPackCount += strlen(srVWTRec.szPaypassTransactionCurrencyCode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* PaypassDefaultTAC */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szPaypassDefaultTAC[0], strlen(srVWTRec.szPaypassDefaultTAC));
	ulPackCount += strlen(srVWTRec.szPaypassDefaultTAC);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* PaypassDenialTAC */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szPaypassDenialTAC[0], strlen(srVWTRec.szPaypassDenialTAC));
	ulPackCount += strlen(srVWTRec.szPaypassDenialTAC);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* PaypassOnlineTAC */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szPaypassOnlineTAC[0], strlen(srVWTRec.szPaypassOnlineTAC));
	ulPackCount += strlen(srVWTRec.szPaypassOnlineTAC);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* PaypassDefaultTDOL */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szPaypassDefaultTDOL[0], strlen(srVWTRec.szPaypassDefaultTDOL));
	ulPackCount += strlen(srVWTRec.szPaypassDefaultTDOL);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* PaypassEMVFloorLimit */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szPaypassEMVFloorLimit[0], strlen(srVWTRec.szPaypassEMVFloorLimit));
	ulPackCount += strlen(srVWTRec.szPaypassEMVFloorLimit);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* PaypassRandomSelectionThreshold */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szPaypassRandomSelectionThreshold[0], strlen(srVWTRec.szPaypassRandomSelectionThreshold));
	ulPackCount += strlen(srVWTRec.szPaypassRandomSelectionThreshold);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* PaypassTargetPercentforRandomSelection */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szPaypassTargetPercentforRandomSelection[0], strlen(srVWTRec.szPaypassTargetPercentforRandomSelection));
	ulPackCount += strlen(srVWTRec.szPaypassTargetPercentforRandomSelection);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* PaypassMaxTargetPercentforRandomSelection */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szPaypassMaxTargetPercentforRandomSelection[0], strlen(srVWTRec.szPaypassMaxTargetPercentforRandomSelection));
	ulPackCount += strlen(srVWTRec.szPaypassMaxTargetPercentforRandomSelection);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* PaypassCVMRequiredLimit */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szPaypassCVMRequiredLimit[0], strlen(srVWTRec.szPaypassCVMRequiredLimit));
	ulPackCount += strlen(srVWTRec.szPaypassCVMRequiredLimit);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* MerchantCategoryCode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szMerchantCategoryCode[0], strlen(srVWTRec.szMerchantCategoryCode));
	ulPackCount += strlen(srVWTRec.szMerchantCategoryCode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TransactionCategoryCode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szTransactionCategoryCode[0], strlen(srVWTRec.szTransactionCategoryCode));
	ulPackCount += strlen(srVWTRec.szTransactionCategoryCode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* CombinationOption */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szCombinationOption[0], strlen(srVWTRec.szCombinationOption));
	ulPackCount += strlen(srVWTRec.szCombinationOption);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TerminalInterchangeProfile */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szTerminalInterchangeProfile[0], strlen(srVWTRec.szTerminalInterchangeProfile));
	ulPackCount += strlen(srVWTRec.szTerminalInterchangeProfile);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* PayWaveAPID */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szPayWaveAPID[0], strlen(srVWTRec.szPayWaveAPID));
	ulPackCount += strlen(srVWTRec.szPayWaveAPID);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* PayWaveAPIDContactlessTransactionLimit */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szPayWaveAPIDContactlessTransactionLimit[0], strlen(srVWTRec.szPayWaveAPIDContactlessTransactionLimit));
	ulPackCount += strlen(srVWTRec.szPayWaveAPIDContactlessTransactionLimit);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* PayWaveAPIDCVMRequiredLimit */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szPayWaveAPIDCVMRequiredLimit[0], strlen(srVWTRec.szPayWaveAPIDCVMRequiredLimit));
	ulPackCount += strlen(srVWTRec.szPayWaveAPIDCVMRequiredLimit);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* PayWaveAPIDContactlessFloorLimit */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srVWTRec.szPayWaveAPIDContactlessFloorLimit[0], strlen(srVWTRec.szPayWaveAPIDContactlessFloorLimit));
	ulPackCount += strlen(srVWTRec.szPayWaveAPIDContactlessFloorLimit);

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
	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inSaveVWTRec END");
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
Function        :inGetContactlessSchemeIndex
Date&Time       :
Describe        :
*/
int inGetContactlessSchemeIndex(char* szContactlessSchemeIndex)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szContactlessSchemeIndex == NULL || strlen(srVWTRec.szContactlessSchemeIndex) <= 0 || strlen(srVWTRec.szContactlessSchemeIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetContactlessSchemeIndex() ERROR !!");

                        if (szContactlessSchemeIndex == NULL)
                        {
                                inDISP_LogPrintf("szContactlessSchemeIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szContactlessSchemeIndex length = (%d)", (int)strlen(srVWTRec.szContactlessSchemeIndex));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }
                return (VS_ERROR);
        }
        memcpy(&szContactlessSchemeIndex[0], &srVWTRec.szContactlessSchemeIndex[0], strlen(srVWTRec.szContactlessSchemeIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetContactlessSchemeIndex
Date&Time       :
Describe        :
*/
int inSetContactlessSchemeIndex(char* szContactlessSchemeIndex)
{
        memset(srVWTRec.szContactlessSchemeIndex, 0x00, sizeof(srVWTRec.szContactlessSchemeIndex));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szContactlessSchemeIndex == NULL || strlen(szContactlessSchemeIndex) <= 0 || strlen(szContactlessSchemeIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetContactlessSchemeIndex() ERROR !!");

                        if (szContactlessSchemeIndex == NULL)
                        {
                                inDISP_LogPrintf("szContactlessSchemeIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szContactlessSchemeIndex length = (%d)", (int)strlen(szContactlessSchemeIndex));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szContactlessSchemeIndex[0], &szContactlessSchemeIndex[0], strlen(szContactlessSchemeIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetVWTTerminalType
Date&Time       :
Describe        :
*/
int inGetVWTTerminalType(char* szTerminalType)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTerminalType == NULL || strlen(srVWTRec.szTerminalType) <= 0 || strlen(srVWTRec.szTerminalType) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTerminalType() ERROR !!");

                        if (szTerminalType == NULL)
                        {
                                inDISP_LogPrintf("szTerminalType == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalType length = (%d)", (int)strlen(srVWTRec.szTerminalType));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTerminalType[0], &srVWTRec.szTerminalType[0], strlen(srVWTRec.szTerminalType));

        return (VS_SUCCESS);

}

/*
Function        :inSetVWTTerminalType
Date&Time       :
Describe        :
*/
int inSetVWTTerminalType(char* szTerminalType)
{
        memset(srVWTRec.szTerminalType, 0x00, sizeof(srVWTRec.szTerminalType));
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
        memcpy(&srVWTRec.szTerminalType[0], &szTerminalType[0], strlen(szTerminalType));

        return (VS_SUCCESS);
}

/*
Function        :inGetCVMRequirement
Date&Time       :
Describe        :
*/
int inGetCVMRequirement(char* szCVMRequirement)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCVMRequirement == NULL || strlen(srVWTRec.szCVMRequirement) <= 0 || strlen(srVWTRec.szCVMRequirement) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCVMRequirement() ERROR !!");

                        if (szCVMRequirement == NULL)
                        {
                                inDISP_LogPrintf("szCVMRequirement == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCVMRequirement length = (%d)", (int)strlen(srVWTRec.szCVMRequirement));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCVMRequirement[0], &srVWTRec.szCVMRequirement[0], strlen(srVWTRec.szCVMRequirement));

        return (VS_SUCCESS);
}

/*
Function        :inSetCVMRequirement
Date&Time       :
Describe        :
*/
int inSetCVMRequirement(char* szCVMRequirement)
{
        memset(srVWTRec.szCVMRequirement, 0x00, sizeof(srVWTRec.szCVMRequirement));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCVMRequirement == NULL || strlen(szCVMRequirement) <= 0 || strlen(szCVMRequirement) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCVMRequirement() ERROR !!");

                        if (szCVMRequirement == NULL)
                        {
                                inDISP_LogPrintf("szCVMRequirement == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCVMRequirement length = (%d)", (int)strlen(szCVMRequirement));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szCVMRequirement[0], &szCVMRequirement[0], strlen(szCVMRequirement));

        return (VS_SUCCESS);
}

/*
Function        :inGetVLPSupportIndicator
Date&Time       :
Describe        :
*/
int inGetVLPSupportIndicator(char* szVLPSupportIndicator)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szVLPSupportIndicator == NULL || strlen(srVWTRec.szVLPSupportIndicator) <= 0 || strlen(srVWTRec.szVLPSupportIndicator) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetVLPSupportIndicator() ERROR !!");

                        if (szVLPSupportIndicator == NULL)
                        {
                                inDISP_LogPrintf("szVLPSupportIndicator == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szVLPSupportIndicator length = (%d)", (int)strlen(srVWTRec.szVLPSupportIndicator));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szVLPSupportIndicator[0], &srVWTRec.szVLPSupportIndicator[0], strlen(srVWTRec.szVLPSupportIndicator));

        return (VS_SUCCESS);
}

/*
Function        :inSetVLPSupportIndicator
Date&Time       :
Describe        :
*/
int inSetVLPSupportIndicator(char* szVLPSupportIndicator)
{
        memset(srVWTRec.szVLPSupportIndicator, 0x00, sizeof(srVWTRec.szVLPSupportIndicator));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szVLPSupportIndicator == NULL || strlen(szVLPSupportIndicator) <= 0 || strlen(szVLPSupportIndicator) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetVLPSupportIndicator() ERROR !!");

                        if (szVLPSupportIndicator == NULL)
                        {
                                inDISP_LogPrintf("szVLPSupportIndicator == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szVLPSupportIndicator length = (%d)", (int)strlen(szVLPSupportIndicator));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szVLPSupportIndicator[0], &szVLPSupportIndicator[0], strlen(szVLPSupportIndicator));

        return (VS_SUCCESS);
}

/*
Function        :inGetTerminalCountryCode
Date&Time       :
Describe        :
*/
int inGetTerminalCountryCode(char* szTerminalCountryCode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTerminalCountryCode == NULL || strlen(srVWTRec.szTerminalCountryCode) <= 0 || strlen(srVWTRec.szTerminalCountryCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTerminalCountryCode() ERROR !!");

                        if (szTerminalCountryCode == NULL)
                        {
                                inDISP_LogPrintf("szTerminalCountryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalCountryCode length = (%d)", (int)strlen(srVWTRec.szTerminalCountryCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTerminalCountryCode[0], &srVWTRec.szTerminalCountryCode[0], strlen(srVWTRec.szTerminalCountryCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetTerminalCountryCode
Date&Time       :
Describe        :
*/
int inSetTerminalCountryCode(char* szTerminalCountryCode)
{
        memset(srVWTRec.szTerminalCountryCode, 0x00, sizeof(srVWTRec.szTerminalCountryCode));
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
        memcpy(&srVWTRec.szTerminalCountryCode[0], &szTerminalCountryCode[0], strlen(szTerminalCountryCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetVWTTransactionCurrencyCode
Date&Time       :
Describe        :
*/
int inGetVWTTransactionCurrencyCode(char* szTransactionCurrencyCode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTransactionCurrencyCode == NULL || strlen(srVWTRec.szTransactionCurrencyCode) <= 0 || strlen(srVWTRec.szTransactionCurrencyCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTransactionCurrencyCode() ERROR !!");

                        if (szTransactionCurrencyCode == NULL)
                        {
                                inDISP_LogPrintf("szTransactionCurrencyCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionCurrencyCode length = (%d)", (int)strlen(srVWTRec.szTransactionCurrencyCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTransactionCurrencyCode[0], &srVWTRec.szTransactionCurrencyCode[0], strlen(srVWTRec.szTransactionCurrencyCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetVWTTransactionCurrencyCode
Date&Time       :
Describe        :
*/
int inSetVWTTransactionCurrencyCode(char* szTransactionCurrencyCode)
{
        memset(srVWTRec.szTransactionCurrencyCode, 0x00, sizeof(srVWTRec.szTransactionCurrencyCode));
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
        memcpy(&srVWTRec.szTransactionCurrencyCode[0], &szTransactionCurrencyCode[0], strlen(szTransactionCurrencyCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetTransactionType
Date&Time       :
Describe        :
*/
int inGetTransactionType(char* szTransactionType)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTransactionType == NULL || strlen(srVWTRec.szTransactionType) <= 0 || strlen(srVWTRec.szTransactionType) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTransactionType() ERROR !!");

                        if (szTransactionType == NULL)
                        {
                                inDISP_LogPrintf("szTransactionType == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionType length = (%d)", (int)strlen(srVWTRec.szTransactionType));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTransactionType[0], &srVWTRec.szTransactionType[0], strlen(srVWTRec.szTransactionType));

        return (VS_SUCCESS);
}

/*
Function        :inSetTransactionType
Date&Time       :
Describe        :
*/
int inSetTransactionType(char* szTransactionType)
{
        memset(srVWTRec.szTransactionType, 0x00, sizeof(srVWTRec.szTransactionType));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTransactionType == NULL || strlen(szTransactionType) <= 0 || strlen(szTransactionType) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTransactionType() ERROR !!");

                        if (szTransactionType == NULL)
                        {
                                inDISP_LogPrintf("szTransactionType == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionType length = (%d)", (int)strlen(szTransactionType));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szTransactionType[0], &szTransactionType[0], strlen(szTransactionType));

        return (VS_SUCCESS);
}

/*
Function        :inGetContactlessTransactionLimit
Date&Time       :
Describe        :
*/
int inGetContactlessTransactionLimit(char* szContactlessTransactionLimit)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szContactlessTransactionLimit == NULL || strlen(srVWTRec.szContactlessTransactionLimit) <= 0 || strlen(srVWTRec.szContactlessTransactionLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetContactlessTransactionLimit() ERROR !!");

                        if (szContactlessTransactionLimit == NULL)
                        {
                                inDISP_LogPrintf("szContactlessTransactionLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szContactlessTransactionLimit length = (%d)", (int)strlen(srVWTRec.szContactlessTransactionLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szContactlessTransactionLimit[0], &srVWTRec.szContactlessTransactionLimit[0], strlen(srVWTRec.szContactlessTransactionLimit));

        return (VS_SUCCESS);
}

/*
Function        :inSetContactlessTransactionLimit
Date&Time       :
Describe        :
*/
int inSetContactlessTransactionLimit(char* szContactlessTransactionLimit)
{
        memset(srVWTRec.szContactlessTransactionLimit, 0x00, sizeof(srVWTRec.szContactlessTransactionLimit));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szContactlessTransactionLimit == NULL || strlen(szContactlessTransactionLimit) <= 0 || strlen(szContactlessTransactionLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetContactlessTransactionLimit() ERROR !!");

                        if (szContactlessTransactionLimit == NULL)
                        {
                                inDISP_LogPrintf("szContactlessTransactionLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szContactlessTransactionLimit length = (%d)", (int)strlen(szContactlessTransactionLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szContactlessTransactionLimit[0], &szContactlessTransactionLimit[0], strlen(szContactlessTransactionLimit));

        return (VS_SUCCESS);
}

/*
Function        :inGetCVMRequiredLimit
Date&Time       :
Describe        :
*/
int inGetCVMRequiredLimit(char* szCVMRequiredLimit)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCVMRequiredLimit == NULL || strlen(srVWTRec.szCVMRequiredLimit) <= 0 || strlen(srVWTRec.szCVMRequiredLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetCVMRequiredLimit() ERROR !!");

                        if (szCVMRequiredLimit == NULL)
                        {
                                inDISP_LogPrintf("szCVMRequiredLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCVMRequiredLimit length = (%d)", (int)strlen(srVWTRec.szCVMRequiredLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCVMRequiredLimit[0], &srVWTRec.szCVMRequiredLimit[0], strlen(srVWTRec.szCVMRequiredLimit));

        return (VS_SUCCESS);
}

/*
Function        :inSetCVMRequiredLimit
Date&Time       :
Describe        :
*/
int inSetCVMRequiredLimit(char* szCVMRequiredLimit)
{
        memset(srVWTRec.szCVMRequiredLimit, 0x00, sizeof(srVWTRec.szCVMRequiredLimit));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCVMRequiredLimit == NULL || strlen(szCVMRequiredLimit) <= 0 || strlen(szCVMRequiredLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCVMRequiredLimit() ERROR !!");

                        if (szCVMRequiredLimit == NULL)
                        {
                                inDISP_LogPrintf("szCVMRequiredLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCVMRequiredLimit length = (%d)", (int)strlen(szCVMRequiredLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szCVMRequiredLimit[0], &szCVMRequiredLimit[0], strlen(szCVMRequiredLimit));

        return (VS_SUCCESS);
}

/*
Function        :inGetContactlessFloorLimit
Date&Time       :
Describe        :
*/
int inGetContactlessFloorLimit(char* szContactlessFloorLimit)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szContactlessFloorLimit == NULL || strlen(srVWTRec.szContactlessFloorLimit) <= 0 || strlen(srVWTRec.szContactlessFloorLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetContactlessFloorLimit() ERROR !!");

                        if (szContactlessFloorLimit == NULL)
                        {
                                inDISP_LogPrintf("szContactlessFloorLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szContactlessFloorLimit length = (%d)", (int)strlen(srVWTRec.szContactlessFloorLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szContactlessFloorLimit[0], &srVWTRec.szContactlessFloorLimit[0], strlen(srVWTRec.szContactlessFloorLimit));

        return (VS_SUCCESS);
}

/*
Function        :inSetContactlessFloorLimit
Date&Time       :
Describe        :
*/
int inSetContactlessFloorLimit(char* szContactlessFloorLimit)
{
        memset(srVWTRec.szContactlessFloorLimit, 0x00, sizeof(srVWTRec.szContactlessFloorLimit));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szContactlessFloorLimit == NULL || strlen(szContactlessFloorLimit) <= 0 || strlen(szContactlessFloorLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetContactlessFloorLimit() ERROR !!");

                        if (szContactlessFloorLimit == NULL)
                        {
                                inDISP_LogPrintf("szContactlessFloorLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szContactlessFloorLimit length = (%d)", (int)strlen(szContactlessFloorLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szContactlessFloorLimit[0], &szContactlessFloorLimit[0], strlen(szContactlessFloorLimit));

        return (VS_SUCCESS);
}

/*
Function        :inGetEnhancedDDAVersionNum
Date&Time       :
Describe        :
*/
int inGetEnhancedDDAVersionNum(char* szEnhancedDDAVersionNum)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szEnhancedDDAVersionNum == NULL || strlen(srVWTRec.szEnhancedDDAVersionNum) <= 0 || strlen(srVWTRec.szEnhancedDDAVersionNum) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetEnhancedDDAVersionNum() ERROR !!");

                        if (szEnhancedDDAVersionNum == NULL)
                        {
                                inDISP_LogPrintf("szEnhancedDDAVersionNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEnhancedDDAVersionNum length = (%d)", (int)strlen(srVWTRec.szEnhancedDDAVersionNum));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szEnhancedDDAVersionNum[0], &srVWTRec.szEnhancedDDAVersionNum[0], strlen(srVWTRec.szEnhancedDDAVersionNum));

        return (VS_SUCCESS);

}

/*
Function        :inSetEnhancedDDAVersionNum
Date&Time       :
Describe        :
*/
int inSetEnhancedDDAVersionNum(char* szEnhancedDDAVersionNum)
{
        memset(srVWTRec.szEnhancedDDAVersionNum, 0x00, sizeof(srVWTRec.szEnhancedDDAVersionNum));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szEnhancedDDAVersionNum == NULL || strlen(szEnhancedDDAVersionNum) <= 0 || strlen(szEnhancedDDAVersionNum) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetEnhancedDDAVersionNum() ERROR !!");

                        if (szEnhancedDDAVersionNum == NULL)
                        {
                                inDISP_LogPrintf("szEnhancedDDAVersionNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEnhancedDDAVersionNum length = (%d)", (int)strlen(szEnhancedDDAVersionNum));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szEnhancedDDAVersionNum[0], &szEnhancedDDAVersionNum[0], strlen(szEnhancedDDAVersionNum));

        return (VS_SUCCESS);
}

/*
Function        :inGetDisplayOfflineFunds
Date&Time       :
Describe        :
*/
int inGetDisplayOfflineFunds(char* szDisplayOfflineFunds)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szDisplayOfflineFunds == NULL || strlen(srVWTRec.szDisplayOfflineFunds) <= 0 || strlen(srVWTRec.szDisplayOfflineFunds) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetDisplayOfflineFunds() ERROR !!");

                        if (szDisplayOfflineFunds == NULL)
                        {
                                inDISP_LogPrintf("szDisplayOfflineFunds == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDisplayOfflineFunds length = (%d)", (int)strlen(srVWTRec.szDisplayOfflineFunds));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDisplayOfflineFunds[0], &srVWTRec.szDisplayOfflineFunds[0], strlen(srVWTRec.szDisplayOfflineFunds));

        return (VS_SUCCESS);
}

/*
Function        :inSetDisplayOfflineFunds
Date&Time       :
Describe        :
*/
int inSetDisplayOfflineFunds(char* szDisplayOfflineFunds)
{
        memset(srVWTRec.szDisplayOfflineFunds, 0x00, sizeof(srVWTRec.szDisplayOfflineFunds));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szDisplayOfflineFunds == NULL || strlen(szDisplayOfflineFunds) <= 0 || strlen(szDisplayOfflineFunds) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetDisplayOfflineFunds() ERROR !!");

                        if (szDisplayOfflineFunds == NULL)
                        {
                                inDISP_LogPrintf("szDisplayOfflineFunds == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDisplayOfflineFunds length = (%d)", (int)strlen(szDisplayOfflineFunds));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szDisplayOfflineFunds[0], &szDisplayOfflineFunds[0], strlen(szDisplayOfflineFunds));

        return (VS_SUCCESS);
}

/*
Function        :inGetTerminalTransactionQualifier
Date&Time       :
Describe        :
*/
int inGetTerminalTransactionQualifier(char* szTerminalTransactionQualifier)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTerminalTransactionQualifier == NULL || strlen(srVWTRec.szTerminalTransactionQualifier) <= 0 || strlen(srVWTRec.szTerminalTransactionQualifier) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTerminalTransactionQualifier() ERROR !!");

                        if (szTerminalTransactionQualifier == NULL)
                        {
                                inDISP_LogPrintf("szTerminalTransactionQualifier == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalTransactionQualifier length = (%d)", (int)strlen(srVWTRec.szTerminalTransactionQualifier));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTerminalTransactionQualifier[0], &srVWTRec.szTerminalTransactionQualifier[0], strlen(srVWTRec.szTerminalTransactionQualifier));

        return (VS_SUCCESS);
}

/*
Function        :inSetTerminalTransactionQualifier
Date&Time       :
Describe        :
*/
int inSetTerminalTransactionQualifier(char* szTerminalTransactionQualifier)
{
        memset(srVWTRec.szTerminalTransactionQualifier, 0x00, sizeof(srVWTRec.szTerminalTransactionQualifier));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTerminalTransactionQualifier == NULL || strlen(szTerminalTransactionQualifier) <= 0 || strlen(szTerminalTransactionQualifier) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTerminalTransactionQualifier() ERROR !!");

                        if (szTerminalTransactionQualifier == NULL)
                        {
                                inDISP_LogPrintf("szTerminalTransactionQualifier == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalTransactionQualifier length = (%d)", (int)strlen(szTerminalTransactionQualifier));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szTerminalTransactionQualifier[0], &szTerminalTransactionQualifier[0], strlen(szTerminalTransactionQualifier));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassTerminalType
Date&Time       :
Describe        :
*/
int inGetPaypassTerminalType(char* szPaypassTerminalType)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassTerminalType == NULL || strlen(srVWTRec.szPaypassTerminalType) <= 0 || strlen(srVWTRec.szPaypassTerminalType) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPaypassTerminalType() ERROR !!");

                        if (szPaypassTerminalType == NULL)
                        {
                                inDISP_LogPrintf("szPaypassTerminalType == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassTerminalType length = (%d)", (int)strlen(srVWTRec.szPaypassTerminalType));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassTerminalType[0], &srVWTRec.szPaypassTerminalType[0], strlen(srVWTRec.szPaypassTerminalType));

        return (VS_SUCCESS);
}

/*
Function        :inSetPaypassTerminalType
Date&Time       :
Describe        :
*/
int inSetPaypassTerminalType(char* szPaypassTerminalType)
{
        memset(srVWTRec.szPaypassTerminalType, 0x00, sizeof(srVWTRec.szPaypassTerminalType));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassTerminalType == NULL || strlen(szPaypassTerminalType) <= 0 || strlen(szPaypassTerminalType) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPaypassTerminalType() ERROR !!");

                        if (szPaypassTerminalType == NULL)
                        {
                                inDISP_LogPrintf("szPaypassTerminalType == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassTerminalType length = (%d)", (int)strlen(szPaypassTerminalType));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassTerminalType[0], &szPaypassTerminalType[0], strlen(szPaypassTerminalType));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassTerminalCapabilities
Date&Time       :
Describe        :
*/
int inGetPaypassTerminalCapabilities(char* szPaypassTerminalCapabilities)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassTerminalCapabilities == NULL || strlen(srVWTRec.szPaypassTerminalCapabilities) <= 0 || strlen(srVWTRec.szPaypassTerminalCapabilities) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPaypassTerminalCapabilities() ERROR !!");

                        if (szPaypassTerminalCapabilities == NULL)
                        {
                                inDISP_LogPrintf("szPaypassTerminalCapabilities == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassTerminalCapabilities length = (%d)", (int)strlen(srVWTRec.szPaypassTerminalCapabilities));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassTerminalCapabilities[0], &srVWTRec.szPaypassTerminalCapabilities[0], strlen(srVWTRec.szPaypassTerminalCapabilities));

        return (VS_SUCCESS);
}

/*
Function        :inSetPaypassTerminalCapabilities
Date&Time       :
Describe        :
*/
int inSetPaypassTerminalCapabilities(char* szPaypassTerminalCapabilities)
{
        memset(srVWTRec.szPaypassTerminalCapabilities, 0x00, sizeof(srVWTRec.szPaypassTerminalCapabilities));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassTerminalCapabilities == NULL || strlen(szPaypassTerminalCapabilities) <= 0 || strlen(szPaypassTerminalCapabilities) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPaypassTerminalCapabilities() ERROR !!");

                        if (szPaypassTerminalCapabilities == NULL)
                        {
                                inDISP_LogPrintf("szPaypassTerminalCapabilities == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassTerminalCapabilities length = (%d)", (int)strlen(szPaypassTerminalCapabilities));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassTerminalCapabilities[0], &szPaypassTerminalCapabilities[0], strlen(szPaypassTerminalCapabilities));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassTerminalCountryCode
Date&Time       :
Describe        :
*/
int inGetPaypassTerminalCountryCode(char* szPaypassTerminalCountryCode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassTerminalCountryCode == NULL || strlen(srVWTRec.szPaypassTerminalCountryCode) <= 0 || strlen(srVWTRec.szPaypassTerminalCountryCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPaypassTerminalCountryCode() ERROR !!");

                        if (szPaypassTerminalCountryCode == NULL)
                        {
                                inDISP_LogPrintf("szPaypassTerminalCountryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassTerminalCountryCode length = (%d)", (int)strlen(srVWTRec.szPaypassTerminalCountryCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassTerminalCountryCode[0], &srVWTRec.szPaypassTerminalCountryCode[0], strlen(srVWTRec.szPaypassTerminalCountryCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetPaypassTerminalCountryCode
Date&Time       :
Describe        :
*/
int inSetPaypassTerminalCountryCode(char* szPaypassTerminalCountryCode)
{
        memset(srVWTRec.szPaypassTerminalCountryCode, 0x00, sizeof(srVWTRec.szPaypassTerminalCountryCode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassTerminalCountryCode == NULL || strlen(szPaypassTerminalCountryCode) <= 0 || strlen(szPaypassTerminalCountryCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPaypassTerminalCountryCode() ERROR !!");

                        if (szPaypassTerminalCountryCode == NULL)
                        {
                                inDISP_LogPrintf("szPaypassTerminalCountryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassTerminalCountryCode length = (%d)", (int)strlen(szPaypassTerminalCountryCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassTerminalCountryCode[0], &szPaypassTerminalCountryCode[0], strlen(szPaypassTerminalCountryCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassTransactionCurrencyCode
Date&Time       :
Describe        :
*/
int inGetPaypassTransactionCurrencyCode(char* szPaypassTransactionCurrencyCode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassTransactionCurrencyCode == NULL || strlen(srVWTRec.szPaypassTransactionCurrencyCode) <= 0 || strlen(srVWTRec.szPaypassTransactionCurrencyCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPaypassTransactionCurrencyCode() ERROR !!");

                        if (szPaypassTransactionCurrencyCode == NULL)
                        {
                                inDISP_LogPrintf("szPaypassTransactionCurrencyCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassTransactionCurrencyCode length = (%d)", (int)strlen(srVWTRec.szPaypassTransactionCurrencyCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassTransactionCurrencyCode[0], &srVWTRec.szPaypassTransactionCurrencyCode[0], strlen(srVWTRec.szPaypassTransactionCurrencyCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetPaypassTransactionCurrencyCode
Date&Time       :
Describe        :
*/
int inSetPaypassTransactionCurrencyCode(char* szPaypassTransactionCurrencyCode)
{
        memset(srVWTRec.szPaypassTransactionCurrencyCode, 0x00, sizeof(srVWTRec.szPaypassTransactionCurrencyCode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassTransactionCurrencyCode == NULL || strlen(szPaypassTransactionCurrencyCode) <= 0 || strlen(szPaypassTransactionCurrencyCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPaypassTransactionCurrencyCode() ERROR !!");

                        if (szPaypassTransactionCurrencyCode == NULL)
                        {
                                inDISP_LogPrintf("szPaypassTransactionCurrencyCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassTransactionCurrencyCode length = (%d)", (int)strlen(szPaypassTransactionCurrencyCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassTransactionCurrencyCode[0], &szPaypassTransactionCurrencyCode[0], strlen(szPaypassTransactionCurrencyCode));

        return (VS_SUCCESS);
}

int inGetPaypassDefaultTAC(char* szPaypassDefaultTAC)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassDefaultTAC == NULL || strlen(srVWTRec.szPaypassDefaultTAC) <= 0 || strlen(srVWTRec.szPaypassDefaultTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPaypassDefaultTAC() ERROR !!");

                        if (szPaypassDefaultTAC == NULL)
                        {
                                inDISP_LogPrintf("szPaypassDefaultTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassDefaultTAC length = (%d)", (int)strlen(srVWTRec.szPaypassDefaultTAC));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassDefaultTAC[0], &srVWTRec.szPaypassDefaultTAC[0], strlen(srVWTRec.szPaypassDefaultTAC));

        return (VS_SUCCESS);
}

int inSetPaypassDefaultTAC(char* szPaypassDefaultTAC)
{
        memset(srVWTRec.szPaypassDefaultTAC, 0x00, sizeof(srVWTRec.szPaypassDefaultTAC));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassDefaultTAC == NULL || strlen(szPaypassDefaultTAC) <= 0 || strlen(szPaypassDefaultTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szPaypassDefaultTAC == NULL)
                        {
                                inDISP_LogPrintf("inSetSloganPrtPositio() ERROR !! szPaypassDefaultTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inSetSloganPrtPositio() ERROR !! szPaypassDefaultTAC length = (%d)", (int)strlen(szPaypassDefaultTAC));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassDefaultTAC[0], &szPaypassDefaultTAC[0], strlen(szPaypassDefaultTAC));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassDenialTAC
Date&Time       :
Describe        :
*/
int inGetPaypassDenialTAC(char* szPaypassDenialTAC)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassDenialTAC == NULL || strlen(srVWTRec.szPaypassDenialTAC) <= 0 || strlen(srVWTRec.szPaypassDenialTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPaypassDenialTAC() ERROR !!");

                        if (szPaypassDenialTAC == NULL)
                        {
                                inDISP_LogPrintf("szPaypassDenialTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassDenialTAC length = (%d)", (int)strlen(srVWTRec.szPaypassDenialTAC));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassDenialTAC[0], &srVWTRec.szPaypassDenialTAC[0], strlen(srVWTRec.szPaypassDenialTAC));

        return (VS_SUCCESS);
}

/*
Function        :inSetPaypassDenialTAC
Date&Time       :
Describe        :
*/
int inSetPaypassDenialTAC(char* szPaypassDenialTAC)
{
        memset(srVWTRec.szPaypassDenialTAC, 0x00, sizeof(srVWTRec.szPaypassDenialTAC));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassDenialTAC == NULL || strlen(szPaypassDenialTAC) <= 0 || strlen(szPaypassDenialTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPaypassDenialTAC() ERROR !!");

                        if (szPaypassDenialTAC == NULL)
                        {
                                inDISP_LogPrintf("szPaypassDenialTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassDenialTAC length = (%d)", (int)strlen(szPaypassDenialTAC));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassDenialTAC[0], &szPaypassDenialTAC[0], strlen(szPaypassDenialTAC));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassOnlineTAC
Date&Time       :
Describe        :
*/
int inGetPaypassOnlineTAC(char* szPaypassOnlineTAC)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassOnlineTAC == NULL || strlen(srVWTRec.szPaypassOnlineTAC) <= 0 || strlen(srVWTRec.szPaypassOnlineTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPaypassOnlineTAC() ERROR !!");

                        if (szPaypassOnlineTAC == NULL)
                        {
                                inDISP_LogPrintf("szPaypassOnlineTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassOnlineTAC length = (%d)", (int)strlen(srVWTRec.szPaypassOnlineTAC));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassOnlineTAC[0], &srVWTRec.szPaypassOnlineTAC[0], strlen(srVWTRec.szPaypassOnlineTAC));

        return (VS_SUCCESS);
}

/*
Function        :inSetPaypassOnlineTAC
Date&Time       :
Describe        :
*/
int inSetPaypassOnlineTAC(char* szPaypassOnlineTAC)
{
        memset(srVWTRec.szPaypassOnlineTAC, 0x00, sizeof(srVWTRec.szPaypassOnlineTAC));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassOnlineTAC == NULL || strlen(szPaypassOnlineTAC) <= 0 || strlen(szPaypassOnlineTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPaypassOnlineTAC() ERROR !!");

                        if (szPaypassOnlineTAC == NULL)
                        {
                                inDISP_LogPrintf("szPaypassOnlineTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassOnlineTAC length = (%d)", (int)strlen(szPaypassOnlineTAC));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassOnlineTAC[0], &szPaypassOnlineTAC[0], strlen(szPaypassOnlineTAC));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassDefaultTDOL
Date&Time       :
Describe        :
*/
int inGetPaypassDefaultTDOL(char* szPaypassDefaultTDOL)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassDefaultTDOL == NULL || strlen(srVWTRec.szPaypassDefaultTDOL) <= 0 || strlen(srVWTRec.szPaypassDefaultTDOL) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPaypassDefaultTDOL() ERROR !!");

                        if (szPaypassDefaultTDOL == NULL)
                        {
                                inDISP_LogPrintf("szPaypassDefaultTDOL == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassDefaultTDOL length = (%d)", (int)strlen(srVWTRec.szPaypassDefaultTDOL));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassDefaultTDOL[0], &srVWTRec.szPaypassDefaultTDOL[0], strlen(srVWTRec.szPaypassDefaultTDOL));

        return (VS_SUCCESS);
}

/*
Function        :inSetPaypassDefaultTDOL
Date&Time       :
Describe        :
*/
int inSetPaypassDefaultTDOL(char* szPaypassDefaultTDOL)
{
        memset(srVWTRec.szPaypassDefaultTDOL, 0x00, sizeof(srVWTRec.szPaypassDefaultTDOL));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassDefaultTDOL == NULL || strlen(szPaypassDefaultTDOL) <= 0 || strlen(szPaypassDefaultTDOL) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPaypassDefaultTDOL() ERROR !!");

                        if (szPaypassDefaultTDOL == NULL)
                        {
                                inDISP_LogPrintf("szPaypassDefaultTDOL == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassDefaultTDOL length = (%d)", (int)strlen(szPaypassDefaultTDOL));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassDefaultTDOL[0], &szPaypassDefaultTDOL[0], strlen(szPaypassDefaultTDOL));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassEMVFloorLimit
Date&Time       :
Describe        :
*/
int inGetPaypassEMVFloorLimit(char* szPaypassEMVFloorLimit)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassEMVFloorLimit == NULL || strlen(srVWTRec.szPaypassEMVFloorLimit) <= 0 || strlen(srVWTRec.szPaypassEMVFloorLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPaypassEMVFloorLimit() ERROR !!");

                        if (szPaypassEMVFloorLimit == NULL)
                        {
                                inDISP_LogPrintf("szPaypassEMVFloorLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassEMVFloorLimit length = (%d)", (int)strlen(srVWTRec.szPaypassEMVFloorLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassEMVFloorLimit[0], &srVWTRec.szPaypassEMVFloorLimit[0], strlen(srVWTRec.szPaypassEMVFloorLimit));

        return (VS_SUCCESS);

}

/*
Function        :inSetPaypassEMVFloorLimit
Date&Time       :
Describe        :
*/
int inSetPaypassEMVFloorLimit(char* szPaypassEMVFloorLimit)
{
        memset(srVWTRec.szPaypassEMVFloorLimit, 0x00, sizeof(srVWTRec.szPaypassEMVFloorLimit));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassEMVFloorLimit == NULL || strlen(szPaypassEMVFloorLimit) <= 0 || strlen(szPaypassEMVFloorLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPaypassEMVFloorLimit() ERROR !!");

                        if (szPaypassEMVFloorLimit == NULL)
                        {
                                inDISP_LogPrintf("szPaypassEMVFloorLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassEMVFloorLimit length = (%d)", (int)strlen(szPaypassEMVFloorLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassEMVFloorLimit[0], &szPaypassEMVFloorLimit[0], strlen(szPaypassEMVFloorLimit));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassRandomSelectionThreshold
Date&Time       :
Describe        :
*/
int inGetPaypassRandomSelectionThreshold(char* szPaypassRandomSelectionThreshold)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassRandomSelectionThreshold == NULL || strlen(srVWTRec.szPaypassRandomSelectionThreshold) <= 0 || strlen(srVWTRec.szPaypassRandomSelectionThreshold) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPaypassRandomSelectionThreshold() ERROR !!");

                        if (szPaypassRandomSelectionThreshold == NULL)
                        {
                                inDISP_LogPrintf("szPaypassRandomSelectionThreshold == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassRandomSelectionThreshold length = (%d)", (int)strlen(srVWTRec.szPaypassRandomSelectionThreshold));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassRandomSelectionThreshold[0], &srVWTRec.szPaypassRandomSelectionThreshold[0], strlen(srVWTRec.szPaypassRandomSelectionThreshold));

        return (VS_SUCCESS);
}

/*
Function        :inSetPaypassRandomSelectionThreshold
Date&Time       :
Describe        :
*/
int inSetPaypassRandomSelectionThreshold(char* szPaypassRandomSelectionThreshold)
{
        memset(srVWTRec.szPaypassRandomSelectionThreshold, 0x00, sizeof(srVWTRec.szPaypassRandomSelectionThreshold));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassRandomSelectionThreshold == NULL || strlen(szPaypassRandomSelectionThreshold) <= 0 || strlen(szPaypassRandomSelectionThreshold) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPaypassRandomSelectionThreshold() ERROR !!");

                        if (szPaypassRandomSelectionThreshold == NULL)
                        {
                                inDISP_LogPrintf("szPaypassRandomSelectionThreshold == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassRandomSelectionThreshold length = (%d)", (int)strlen(szPaypassRandomSelectionThreshold));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassRandomSelectionThreshold[0], &szPaypassRandomSelectionThreshold[0], strlen(szPaypassRandomSelectionThreshold));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassTargetPercentforRandomSelection
Date&Time       :
Describe        :
*/
int inGetPaypassTargetPercentforRandomSelection(char* szPaypassTargetPercentforRandomSelection)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassTargetPercentforRandomSelection == NULL || strlen(srVWTRec.szPaypassTargetPercentforRandomSelection) <= 0 || strlen(srVWTRec.szPaypassTargetPercentforRandomSelection) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPaypassTargetPercentforRandomSelection() ERROR !!");

                        if (szPaypassTargetPercentforRandomSelection == NULL)
                        {
                                inDISP_LogPrintf("szPaypassTargetPercentforRandomSelection == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassTargetPercentforRandomSelection length = (%d)", (int)strlen(srVWTRec.szPaypassTargetPercentforRandomSelection));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassTargetPercentforRandomSelection[0], &srVWTRec.szPaypassTargetPercentforRandomSelection[0], strlen(srVWTRec.szPaypassTargetPercentforRandomSelection));

        return (VS_SUCCESS);
}

/*
Function        :inSetPaypassTargetPercentforRandomSelection
Date&Time       :
Describe        :
*/
int inSetPaypassTargetPercentforRandomSelection(char* szPaypassTargetPercentforRandomSelection)
{
        memset(srVWTRec.szPaypassTargetPercentforRandomSelection, 0x00, sizeof(srVWTRec.szPaypassTargetPercentforRandomSelection));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassTargetPercentforRandomSelection == NULL || strlen(szPaypassTargetPercentforRandomSelection) <= 0 || strlen(szPaypassTargetPercentforRandomSelection) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPaypassTargetPercentforRandomSelection() ERROR !!");

                        if (szPaypassTargetPercentforRandomSelection == NULL)
                        {
                                inDISP_LogPrintf("szPaypassTargetPercentforRandomSelection == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassTargetPercentforRandomSelection length = (%d)", (int)strlen(szPaypassTargetPercentforRandomSelection));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassTargetPercentforRandomSelection[0], &szPaypassTargetPercentforRandomSelection[0], strlen(szPaypassTargetPercentforRandomSelection));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassMaxTargetPercentforRandomSelection
Date&Time       :
Describe        :
*/
int inGetPaypassMaxTargetPercentforRandomSelection(char* szPaypassMaxTargetPercentforRandomSelection)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassMaxTargetPercentforRandomSelection == NULL || strlen(srVWTRec.szPaypassMaxTargetPercentforRandomSelection) <= 0 || strlen(srVWTRec.szPaypassMaxTargetPercentforRandomSelection) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPaypassMaxTargetPercentforRandomSelection() ERROR !!");

                        if (szPaypassMaxTargetPercentforRandomSelection == NULL)
                        {
                                inDISP_LogPrintf("szPaypassMaxTargetPercentforRandomSelection == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassMaxTargetPercentforRandomSelection length = (%d)", (int)strlen(srVWTRec.szPaypassMaxTargetPercentforRandomSelection));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassMaxTargetPercentforRandomSelection[0], &srVWTRec.szPaypassMaxTargetPercentforRandomSelection[0], strlen(srVWTRec.szPaypassMaxTargetPercentforRandomSelection));

        return (VS_SUCCESS);
}

/*
Function        :inSetPaypassMaxTargetPercentforRandomSelection
Date&Time       :
Describe        :
*/
int inSetPaypassMaxTargetPercentforRandomSelection(char* szPaypassMaxTargetPercentforRandomSelection)
{
        memset(srVWTRec.szPaypassMaxTargetPercentforRandomSelection, 0x00, sizeof(srVWTRec.szPaypassMaxTargetPercentforRandomSelection));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassMaxTargetPercentforRandomSelection == NULL || strlen(szPaypassMaxTargetPercentforRandomSelection) <= 0 || strlen(szPaypassMaxTargetPercentforRandomSelection) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPaypassMaxTargetPercentforRandomSelection() ERROR !!");

                        if (szPaypassMaxTargetPercentforRandomSelection == NULL)
                        {
                                inDISP_LogPrintf("szPaypassMaxTargetPercentforRandomSelection == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassMaxTargetPercentforRandomSelection length = (%d)", (int)strlen(szPaypassMaxTargetPercentforRandomSelection));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassMaxTargetPercentforRandomSelection[0], &szPaypassMaxTargetPercentforRandomSelection[0], strlen(szPaypassMaxTargetPercentforRandomSelection));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassCVMRequiredLimit
Date&Time       :
Describe        :
*/
int inGetPaypassCVMRequiredLimit(char* szPaypassCVMRequiredLimit)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassCVMRequiredLimit == NULL || strlen(srVWTRec.szPaypassCVMRequiredLimit) <= 0 || strlen(srVWTRec.szPaypassCVMRequiredLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPaypassCVMRequiredLimit() ERROR !!");

                        if (szPaypassCVMRequiredLimit == NULL)
                        {
                                inDISP_LogPrintf("szPaypassCVMRequiredLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassCVMRequiredLimit length = (%d)", (int)strlen(srVWTRec.szPaypassCVMRequiredLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassCVMRequiredLimit[0], &srVWTRec.szPaypassCVMRequiredLimit[0], strlen(srVWTRec.szPaypassCVMRequiredLimit));

        return (VS_SUCCESS);
}

/*
Function        :inSetPaypassCVMRequiredLimit
Date&Time       :
Describe        :
*/
int inSetPaypassCVMRequiredLimit(char* szPaypassCVMRequiredLimit)
{
        memset(srVWTRec.szPaypassCVMRequiredLimit, 0x00, sizeof(srVWTRec.szPaypassCVMRequiredLimit));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassCVMRequiredLimit == NULL || strlen(szPaypassCVMRequiredLimit) <= 0 || strlen(szPaypassCVMRequiredLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPaypassCVMRequiredLimit() ERROR !!");

                        if (szPaypassCVMRequiredLimit == NULL)
                        {
                                inDISP_LogPrintf("szPaypassCVMRequiredLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassCVMRequiredLimit length = (%d)", (int)strlen(szPaypassCVMRequiredLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassCVMRequiredLimit[0], &szPaypassCVMRequiredLimit[0], strlen(szPaypassCVMRequiredLimit));

        return (VS_SUCCESS);
}

/*
Function        :inGetVWTMerchantCategoryCode
Date&Time       :
Describe        :
*/
int inGetVWTMerchantCategoryCode(char* szMerchantCategoryCode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szMerchantCategoryCode == NULL || strlen(srVWTRec.szMerchantCategoryCode) <= 0 || strlen(srVWTRec.szMerchantCategoryCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetMerchantCategoryCode() ERROR !!");

                        if (szMerchantCategoryCode == NULL)
                        {
                                inDISP_LogPrintf("szMerchantCategoryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMerchantCategoryCode length = (%d)", (int)strlen(srVWTRec.szMerchantCategoryCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMerchantCategoryCode[0], &srVWTRec.szMerchantCategoryCode[0], strlen(srVWTRec.szMerchantCategoryCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetVWTMerchantCategoryCode
Date&Time       :
Describe        :
*/
int inSetVWTMerchantCategoryCode(char* szMerchantCategoryCode)
{
        memset(srVWTRec.szMerchantCategoryCode, 0x00, sizeof(srVWTRec.szMerchantCategoryCode));
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
        memcpy(&srVWTRec.szMerchantCategoryCode[0], &szMerchantCategoryCode[0], strlen(szMerchantCategoryCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetVWTTransactionCategoryCode
Date&Time       :
Describe        :
*/
int inGetVWTTransactionCategoryCode(char* szTransactionCategoryCode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTransactionCategoryCode == NULL || strlen(srVWTRec.szTransactionCategoryCode) <= 0 || strlen(srVWTRec.szTransactionCategoryCode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTransactionCategoryCode() ERROR !!");

                        if (szTransactionCategoryCode == NULL)
                        {
                                inDISP_LogPrintf("szTransactionCategoryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionCategoryCode length = (%d)", (int)strlen(srVWTRec.szTransactionCategoryCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTransactionCategoryCode[0], &srVWTRec.szTransactionCategoryCode[0], strlen(srVWTRec.szTransactionCategoryCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetVWTTransactionCategoryCode
Date&Time       :
Describe        :
*/
int inSetVWTTransactionCategoryCode(char* szTransactionCategoryCode)
{
        memset(srVWTRec.szTransactionCategoryCode, 0x00, sizeof(srVWTRec.szTransactionCategoryCode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTransactionCategoryCode == NULL || strlen(szTransactionCategoryCode) <= 0 || strlen(szTransactionCategoryCode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTransactionCategoryCode() ERROR !!");

                        if (szTransactionCategoryCode == NULL)
                        {
                                inDISP_LogPrintf("szTransactionCategoryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionCategoryCode length = (%d)", (int)strlen(szTransactionCategoryCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szTransactionCategoryCode[0], &szTransactionCategoryCode[0], strlen(szTransactionCategoryCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetCombinationOption
Date&Time       :
Describe        :
*/
int inGetCombinationOption(char* szCombinationOption)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCombinationOption == NULL || strlen(srVWTRec.szCombinationOption) <= 0 || strlen(srVWTRec.szCombinationOption) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetszCombinationOption() ERROR !!");

                        if (szCombinationOption == NULL)
                        {
                                inDISP_LogPrintf("szCombinationOption == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCombinationOption length = (%d)", (int)strlen(srVWTRec.szCombinationOption));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCombinationOption[0], &srVWTRec.szCombinationOption[0], strlen(srVWTRec.szCombinationOption));

        return (VS_SUCCESS);
}

/*
Function        :inSetCombinationOption
Date&Time       :
Describe        :
*/
int inSetCombinationOption(char* szCombinationOption)
{
        memset(srVWTRec.szCombinationOption, 0x00, sizeof(srVWTRec.szCombinationOption));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCombinationOption == NULL || strlen(szCombinationOption) <= 0 || strlen(szCombinationOption) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCombinationOption() ERROR !!");

                        if (szCombinationOption == NULL)
                        {
                                inDISP_LogPrintf("szCombinationOption == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCombinationOption length = (%d)", (int)strlen(szCombinationOption));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szCombinationOption[0], &szCombinationOption[0], strlen(szCombinationOption));

        return (VS_SUCCESS);
}

/*
Function        :inGetTerminalInterchangeProfile
Date&Time       :
Describe        :
*/
int inGetTerminalInterchangeProfile(char* szTerminalInterchangeProfile)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTerminalInterchangeProfile == NULL || strlen(srVWTRec.szTerminalInterchangeProfile) <= 0 || strlen(srVWTRec.szTerminalInterchangeProfile) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTerminalInterchangeProfile() ERROR !!");

                        if (szTerminalInterchangeProfile == NULL)
                        {
                                inDISP_LogPrintf("szTerminalInterchangeProfile == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalInterchangeProfile length = (%d)", (int)strlen(srVWTRec.szTerminalInterchangeProfile));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTerminalInterchangeProfile[0], &srVWTRec.szTerminalInterchangeProfile[0], strlen(srVWTRec.szTerminalInterchangeProfile));

        return (VS_SUCCESS);
}

/*
Function        :inSetTerminalInterchangeProfile
Date&Time       :
Describe        :
*/
int inSetTerminalInterchangeProfile(char* szTerminalInterchangeProfile)
{
        memset(srVWTRec.szTerminalInterchangeProfile, 0x00, sizeof(srVWTRec.szTerminalInterchangeProfile));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTerminalInterchangeProfile == NULL || strlen(szTerminalInterchangeProfile) <= 0 || strlen(szTerminalInterchangeProfile) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTerminalInterchangeProfile() ERROR !!");

                        if (szTerminalInterchangeProfile == NULL)
                        {
                                inDISP_LogPrintf("szTerminalInterchangeProfile == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalInterchangeProfile length = (%d)", (int)strlen(szTerminalInterchangeProfile));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szTerminalInterchangeProfile[0], &szTerminalInterchangeProfile[0], strlen(szTerminalInterchangeProfile));

        return (VS_SUCCESS);
}

/*
Function        :inGetPayWaveAPID
Date&Time       :
Describe        :
*/
int inGetPayWaveAPID(char* szPayWaveAPID)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPayWaveAPID == NULL || strlen(srVWTRec.szPayWaveAPID) <= 0 || strlen(srVWTRec.szPayWaveAPID) > 32)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPayWaveAPID() ERROR !!");

                        if (szPayWaveAPID == NULL)
                        {
                                inDISP_LogPrintf("szPayWaveAPID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPayWaveAPID length = (%d)", (int)strlen(srVWTRec.szPayWaveAPID));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPayWaveAPID[0], &srVWTRec.szPayWaveAPID[0], strlen(srVWTRec.szPayWaveAPID));

        return (VS_SUCCESS);
}

/*
Function        :inSetPayWaveAPID
Date&Time       :
Describe        :
*/
int inSetPayWaveAPID(char* szPayWaveAPID)
{
        memset(srVWTRec.szPayWaveAPID, 0x00, sizeof(srVWTRec.szPayWaveAPID));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPayWaveAPID == NULL || strlen(szPayWaveAPID) <= 0 || strlen(szPayWaveAPID) > 32)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPayWaveAPID() ERROR !!");

                        if (szPayWaveAPID == NULL)
                        {
                                inDISP_LogPrintf("szPayWaveAPID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPayWaveAPID length = (%d)", (int)strlen(szPayWaveAPID));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPayWaveAPID[0], &szPayWaveAPID[0], strlen(szPayWaveAPID));

        return (VS_SUCCESS);
}

/*
Function        :inGetPayWaveAPIDContactlessTransactionLimit
Date&Time       :
Describe        :
*/
int inGetPayWaveAPIDContactlessTransactionLimit(char* szPayWaveAPIDContactlessTransactionLimit)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPayWaveAPIDContactlessTransactionLimit == NULL || strlen(srVWTRec.szPayWaveAPIDContactlessTransactionLimit) <= 0 || strlen(srVWTRec.szPayWaveAPIDContactlessTransactionLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPayWaveAPIDContactlessTransactionLimit() ERROR !!");

                        if (szPayWaveAPIDContactlessTransactionLimit == NULL)
                        {
                                inDISP_LogPrintf("szPayWaveAPIDContactlessTransactionLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPayWaveAPIDContactlessTransactionLimit length = (%d)", (int)strlen(srVWTRec.szPayWaveAPIDContactlessTransactionLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPayWaveAPIDContactlessTransactionLimit[0], &srVWTRec.szPayWaveAPIDContactlessTransactionLimit[0], strlen(srVWTRec.szPayWaveAPIDContactlessTransactionLimit));

        return (VS_SUCCESS);

}

/*
Function        :inSetPayWaveAPIDContactlessTransactionLimit
Date&Time       :
Describe        :
*/
int inSetPayWaveAPIDContactlessTransactionLimit(char* szPayWaveAPIDContactlessTransactionLimit)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPayWaveAPIDContactlessTransactionLimit == NULL || strlen(szPayWaveAPIDContactlessTransactionLimit) <= 0 || strlen(szPayWaveAPIDContactlessTransactionLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPayWaveAPIDContactlessTransactionLimit() ERROR !!");

                        if (szPayWaveAPIDContactlessTransactionLimit == NULL)
                        {
                                inDISP_LogPrintf("szPayWaveAPIDContactlessTransactionLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPayWaveAPIDContactlessTransactionLimit length = (%d)", (int)strlen(szPayWaveAPIDContactlessTransactionLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPayWaveAPIDContactlessTransactionLimit[0], &szPayWaveAPIDContactlessTransactionLimit[0], strlen(szPayWaveAPIDContactlessTransactionLimit));

        return (VS_SUCCESS);
}

/*
Function        :inGetPayWaveAPIDCVMRequiredLimit
Date&Time       :
Describe        :
*/
int inGetPayWaveAPIDCVMRequiredLimit(char* szPayWaveAPIDCVMRequiredLimit)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPayWaveAPIDCVMRequiredLimit == NULL || strlen(srVWTRec.szPayWaveAPIDCVMRequiredLimit) <= 0 || strlen(srVWTRec.szPayWaveAPIDCVMRequiredLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPayWaveAPIDCVMRequiredLimit() ERROR !!");

                        if (szPayWaveAPIDCVMRequiredLimit == NULL)
                        {
                                inDISP_LogPrintf("szPayWaveAPIDCVMRequiredLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPayWaveAPIDCVMRequiredLimit length = (%d)", (int)strlen(srVWTRec.szPayWaveAPIDCVMRequiredLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPayWaveAPIDCVMRequiredLimit[0], &srVWTRec.szPayWaveAPIDCVMRequiredLimit[0], strlen(srVWTRec.szPayWaveAPIDCVMRequiredLimit));

        return (VS_SUCCESS);
}

/*
Function        :inSetPayWaveAPIDCVMRequiredLimit
Date&Time       :
Describe        :
*/
int inSetPayWaveAPIDCVMRequiredLimit(char* szPayWaveAPIDCVMRequiredLimit)
{
        memset(srVWTRec.szPayWaveAPIDCVMRequiredLimit, 0x00, sizeof(srVWTRec.szPayWaveAPIDCVMRequiredLimit));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPayWaveAPIDCVMRequiredLimit == NULL || strlen(szPayWaveAPIDCVMRequiredLimit) <= 0 || strlen(szPayWaveAPIDCVMRequiredLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPayWaveAPIDCVMRequiredLimit() ERROR !!");

                        if (szPayWaveAPIDCVMRequiredLimit == NULL)
                        {
                                inDISP_LogPrintf("szPayWaveAPIDCVMRequiredLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPayWaveAPIDCVMRequiredLimit length = (%d)", (int)strlen(szPayWaveAPIDCVMRequiredLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPayWaveAPIDCVMRequiredLimit[0], &szPayWaveAPIDCVMRequiredLimit[0], strlen(szPayWaveAPIDCVMRequiredLimit));

        return (VS_SUCCESS);
}

/*
Function        :inGetPayWaveAPIDContactlessFloorLimit
Date&Time       :
Describe        :
*/
int inGetPayWaveAPIDContactlessFloorLimit(char* szPayWaveAPIDContactlessFloorLimit)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPayWaveAPIDContactlessFloorLimit == NULL || strlen(srVWTRec.szPayWaveAPIDContactlessFloorLimit) <= 0 || strlen(srVWTRec.szPayWaveAPIDContactlessFloorLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPayWaveAPIDContactlessFloorLimit() ERROR !!");

                        if (szPayWaveAPIDContactlessFloorLimit == NULL)
                        {
                                inDISP_LogPrintf("szPayWaveAPIDContactlessFloorLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPayWaveAPIDContactlessFloorLimit length = (%d)", (int)strlen(srVWTRec.szPayWaveAPIDContactlessFloorLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPayWaveAPIDContactlessFloorLimit[0], &srVWTRec.szPayWaveAPIDContactlessFloorLimit[0], strlen(srVWTRec.szPayWaveAPIDContactlessFloorLimit));

        return (VS_SUCCESS);
}

/*
Function        :inSetPayWaveAPIDContactlessFloorLimit
Date&Time       :
Describe        :
*/
int inSetPayWaveAPIDContactlessFloorLimit(char* szPayWaveAPIDContactlessFloorLimit)
{
        memset(srVWTRec.szPayWaveAPIDContactlessFloorLimit, 0x00, sizeof(srVWTRec.szPayWaveAPIDContactlessFloorLimit));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPayWaveAPIDContactlessFloorLimit == NULL || strlen(szPayWaveAPIDContactlessFloorLimit) <= 0 || strlen(szPayWaveAPIDContactlessFloorLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPayWaveAPIDContactlessFloorLimit() ERROR !!");

                        if (szPayWaveAPIDContactlessFloorLimit == NULL)
                        {
                                inDISP_LogPrintf("szPayWaveAPIDContactlessFloorLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPayWaveAPIDContactlessFloorLimit length = (%d)", (int)strlen(szPayWaveAPIDContactlessFloorLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPayWaveAPIDContactlessFloorLimit[0], &szPayWaveAPIDContactlessFloorLimit[0], strlen(szPayWaveAPIDContactlessFloorLimit));

        return (VS_SUCCESS);
}

