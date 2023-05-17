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
#include "TDT.h"

static  TDT_REC	srTDTRec;	/* construct TDT record */
extern  int	ginDebug;       /* Debug使用 extern */

/*
Function        :inLoadTDTRec
Date&Time       :2017/12/18 上午 9:54
Describe        :讀TDT檔案，inTDTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadTDTRec(int inTDTRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆TDT檔案 */
        char            szTDTRec[_SIZE_TDT_REC_ + 1];		/* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnTDTLength = 0;			/* TDT總長度 */
        long            lnReadLength;                           /* 記錄每次要從TDT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從TDT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadTDTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadTDTRec(%d) START!!", inTDTRec);
                inDISP_LogPrintf(szErrorMsg);
        }

        /* 判斷傳進來的inTDTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inTDTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inTDTRec < 0:(index = %d) ERROR!!", inTDTRec);
                        inDISP_LogPrintf(szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open TDT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_TDT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnTDTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_TDT_FILE_NAME_);

        if (lnTDTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnTDTLength + 1);
        uszTemp = malloc(lnTDTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnTDTLength + 1);
        memset(uszTemp, 0x00, lnTDTLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnTDTLength;

                for (i = 0;; ++i)
                {
                        /* 剩餘長度大於或等於1024 */
                        if (lnReadLength >= 1024)
                        {
                                if (inFILE_Read(&ulFile_Handle, &uszTemp[1024 * i], 1024) == VS_SUCCESS)
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
                                if (inFILE_Read(&ulFile_Handle, &uszTemp[1024 * i], lnReadLength) == VS_SUCCESS)
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
         *i為目前從TDT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnTDTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到TDT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnTDTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inTDT_Rec的index從0開始，所以inTDT_Rec要+1 */
                        if (inRec == (inTDTRec + 1))
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
         * 如果沒有inTDTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inTDTRec + 1) || inSearchResult == -1)
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
        memset(&srTDTRec, 0x00, sizeof(srTDTRec));
        /*
         * 以下pattern為存入TDT_Rec
         * i為TDT的第幾個字元
         * 存入TDT_Rec
         */
        i = 0;


        /* 01_szTicket_HostIndex */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TDT unpack ERROR.");
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
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_HostIndex[0], &szTDTRec[0], k - 1);
        }

        /* 02_szTicket_HostName */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TDT unpack ERROR");
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
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_HostName[0], &szTDTRec[0], k - 1);
        }

        /* 03_szTicket_HostTransFunc */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TDT unpack ERROR");
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
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_HostTransFunc[0], &szTDTRec[0], k - 1);
        }

        /* 04_szTicket_HostEnable */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TDT unpack ERROR");
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
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_HostEnable[0], &szTDTRec[0], k - 1);
        }
	
	/* 05_szTicket_LogOnOK */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TDT unpack ERROR");
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
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_LogOnOK[0], &szTDTRec[0], k - 1);
        }
	
	/* 06_szTicket_SAM_Slot */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TDT unpack ERROR");
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
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_SAM_Slot[0], &szTDTRec[0], k - 1);
        }
	
	/* 07_szTicket_ReaderID */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TDT unpack ERROR");
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
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_ReaderID[0], &szTDTRec[0], k - 1);
        }
	
	/* 08_szTicket_STAN */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TDT unpack ERROR");
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
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_STAN[0], &szTDTRec[0], k - 1);
        }
	
	/* 09_szTicket_LastTransDate */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TDT unpack ERROR");
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
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_LastTransDate[0], &szTDTRec[0], k - 1);
        }
	
	/* 10.szTicket_LastRRN */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TDT unpack ERROR");
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
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_LastRRN[0], &szTDTRec[0], k - 1);
        }
	
	/* 11_szTicket_Device1 */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TDT unpack ERROR");
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
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_Device1[0], &szTDTRec[0], k - 1);
        }
	
	/* 12_szTicket_Device2 */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TDT unpack ERROR");
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
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_Device2[0], &szTDTRec[0], k - 1);
        }
	
	/* 13_szTicket_Batch */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TDT unpack ERROR");
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
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_Batch[0], &szTDTRec[0], k - 1);
        }
	
	/* 14_szTicket_NeedNewBatch */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TDT unpack ERROR");
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
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_NeedNewBatch[0], &szTDTRec[0], k - 1);
        }
	
	/* 15_悠遊卡Dongle Device ID */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TDT unpack ERROR");
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
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_Device3[0], &szTDTRec[0], k - 1);
        }
	
        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadTDTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadTDTRec(%d) END!!", inTDTRec);
                inDISP_LogPrintf(szErrorMsg);
                inDISP_LogPrintf("----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveTDTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveTDTRec(int inTDTRec)
{
 	unsigned long   uldat_Handle;   			/* FILE Handle */
	int inRecCount = 0, inTempIndex = 0;
	int     	inRetVal;
	int     	i = 0;

	char szTempTitle[] = "inSaveTDTRec";
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
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inSaveTDTRec INIT" );
#endif

	inTempIndex = inTDTRec;
	
	memset(szTempFIleName, 0x00, sizeof(szTempFIleName));
	strcat( szTempFIleName, _TDT_FILE_NAME_);
	
	memset(szTempBakFileName, 0x00, sizeof(szTempBakFileName));
	strcat( szTempBakFileName, _TDT_FILE_NAME_BAK_);
	
	inDISP_LogPrintfWithFlag("  %s Index[%d] ", szTempTitle, inTempIndex);

      	/* 開啟原檔案HDPT.dat */
	inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_TDT_FILE_NAME_);

	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  %s *Error* [%d] ",szTempTitle, inRetVal);
		inFunc_EDCLock();
		return(VS_ERROR);
	}

	usRetVal = CTOS_FileGetSize((unsigned char *)_TDT_FILE_NAME_, &ulFileSize);	
	
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
	uszWriteBuff_Record = malloc(_SIZE_TDT_REC_ + _SIZE_TDT_COMMA_0D0A_);
	memset(uszWriteBuff_Record, 0x00, _SIZE_TDT_REC_ + _SIZE_TDT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

	/* uszRead_Total_Buff儲存TDT.dat全部資料 */
	uszRead_Total_Buff = malloc(ulFileSize + 1);
	memset(uszRead_Total_Buff, 0x00, ulFileSize + 1);

	ulPackCount = 0; /* uszWriteBuff_Record的index位置 */
	/* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* szTicket_HostIndex */
	memcpy(&uszWriteBuff_Record[0], &srTDTRec.szTicket_HostIndex[0], strlen(srTDTRec.szTicket_HostIndex));
	ulPackCount += strlen(srTDTRec.szTicket_HostIndex);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szTicket_HostName */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTDTRec.szTicket_HostName[0], strlen(srTDTRec.szTicket_HostName));
	ulPackCount += strlen(srTDTRec.szTicket_HostName);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szTicket_HostTransFunc */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTDTRec.szTicket_HostTransFunc[0], strlen(srTDTRec.szTicket_HostTransFunc));
	ulPackCount += strlen(srTDTRec.szTicket_HostTransFunc);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szTicket_HostEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTDTRec.szTicket_HostEnable[0], strlen(srTDTRec.szTicket_HostEnable));
	ulPackCount += strlen(srTDTRec.szTicket_HostEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szTicket_LogOnOK */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTDTRec.szTicket_LogOnOK[0], strlen(srTDTRec.szTicket_LogOnOK));
	ulPackCount += strlen(srTDTRec.szTicket_LogOnOK);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szTicket_SAM_Slot */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTDTRec.szTicket_SAM_Slot[0], strlen(srTDTRec.szTicket_SAM_Slot));
	ulPackCount += strlen(srTDTRec.szTicket_SAM_Slot);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szTicket_ReaderID */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTDTRec.szTicket_ReaderID[0], strlen(srTDTRec.szTicket_ReaderID));
	ulPackCount += strlen(srTDTRec.szTicket_ReaderID);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szTicket_STAN */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTDTRec.szTicket_STAN[0], strlen(srTDTRec.szTicket_STAN));
	ulPackCount += strlen(srTDTRec.szTicket_STAN);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szTicket_LastTransDate */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTDTRec.szTicket_LastTransDate[0], strlen(srTDTRec.szTicket_LastTransDate));
	ulPackCount += strlen(srTDTRec.szTicket_LastTransDate);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szTicket_LastRRN */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTDTRec.szTicket_LastRRN[0], strlen(srTDTRec.szTicket_LastRRN));
	ulPackCount += strlen(srTDTRec.szTicket_LastRRN);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szTicket_Device1 */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTDTRec.szTicket_Device1[0], strlen(srTDTRec.szTicket_Device1));
	ulPackCount += strlen(srTDTRec.szTicket_Device1);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szTicket_Device2 */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTDTRec.szTicket_Device2[0], strlen(srTDTRec.szTicket_Device2));
	ulPackCount += strlen(srTDTRec.szTicket_Device2);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szTicket_Batch */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTDTRec.szTicket_Batch[0], strlen(srTDTRec.szTicket_Batch));
	ulPackCount += strlen(srTDTRec.szTicket_Batch);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szTicket_NeedNewBatch */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTDTRec.szTicket_NeedNewBatch[0], strlen(srTDTRec.szTicket_NeedNewBatch));
	ulPackCount += strlen(srTDTRec.szTicket_NeedNewBatch);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 悠遊卡Dongle Device ID */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTDTRec.szTicket_Device3[0], strlen(srTDTRec.szTicket_Device3));
	ulPackCount += strlen(srTDTRec.szTicket_Device3);

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
	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inSaveTDTRec END");
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
Function        :inGetTicket_HostIndex
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inGetTicket_HostIndex(char* szTicket_HostIndex)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_HostIndex == NULL || strlen(srTDTRec.szTicket_HostIndex) <= 0 || strlen(srTDTRec.szTicket_HostIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTicket_HostIndex() ERROR !!");

			if (szTicket_HostIndex == NULL)
                        {
                                inDISP_LogPrintf("szTicket_HostIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_HostIndex length = (%d)", (int)strlen(srTDTRec.szTicket_HostIndex));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_HostIndex[0], &srTDTRec.szTicket_HostIndex[0], strlen(srTDTRec.szTicket_HostIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_HostIndex
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inSetTicket_HostIndex(char* szTicket_HostIndex)
{
        memset(srTDTRec.szTicket_HostIndex, 0x00, sizeof(srTDTRec.szTicket_HostIndex));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_HostIndex == NULL || strlen(szTicket_HostIndex) <= 0 || strlen(szTicket_HostIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTicket_HostIndex() ERROR !!");
                        if (szTicket_HostIndex == NULL)
                        {
                                inDISP_LogPrintf("szTicket_HostIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_HostIndex length = (%d)", (int)strlen(szTicket_HostIndex));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_HostIndex[0], &szTicket_HostIndex[0], strlen(szTicket_HostIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_HostName
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inGetTicket_HostName(char* szTicket_HostName)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_HostName == NULL || strlen(srTDTRec.szTicket_HostName) <= 0 || strlen(srTDTRec.szTicket_HostName) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTicket_HostName() ERROR !!");

			if (szTicket_HostName == NULL)
                        {
                                inDISP_LogPrintf("szTicket_HostName == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_HostName length = (%d)", (int)strlen(srTDTRec.szTicket_HostName));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_HostName[0], &srTDTRec.szTicket_HostName[0], strlen(srTDTRec.szTicket_HostName));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_HostName
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inSetTicket_HostName(char* szTicket_HostName)
{
        memset(srTDTRec.szTicket_HostName, 0x00, sizeof(srTDTRec.szTicket_HostName));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_HostName == NULL || strlen(szTicket_HostName) <= 0 || strlen(szTicket_HostName) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTicket_HostName() ERROR !!");
                        if (szTicket_HostName == NULL)
                        {
                                inDISP_LogPrintf("szTicket_HostName == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_HostName length = (%d)", (int)strlen(szTicket_HostName));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_HostName[0], &szTicket_HostName[0], strlen(szTicket_HostName));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_HostTransFunc
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inGetTicket_HostTransFunc(char* szTicket_HostTransFunc)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_HostTransFunc == NULL || strlen(srTDTRec.szTicket_HostTransFunc) <= 0 || strlen(srTDTRec.szTicket_HostTransFunc) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTicket_HostTransFunc() ERROR !!");

			if (szTicket_HostTransFunc == NULL)
                        {
                                inDISP_LogPrintf("szTicket_HostTransFunc == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_HostTransFunc length = (%d)", (int)strlen(srTDTRec.szTicket_HostTransFunc));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_HostTransFunc[0], &srTDTRec.szTicket_HostTransFunc[0], strlen(srTDTRec.szTicket_HostTransFunc));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_HostTransFunc
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inSetTicket_HostTransFunc(char* szTicket_HostTransFunc)
{
        memset(srTDTRec.szTicket_HostTransFunc, 0x00, sizeof(srTDTRec.szTicket_HostTransFunc));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_HostTransFunc == NULL || strlen(szTicket_HostTransFunc) <= 0 || strlen(szTicket_HostTransFunc) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTicket_HostTransFunc() ERROR !!");
                        if (szTicket_HostTransFunc == NULL)
                        {
                                inDISP_LogPrintf("szTicket_HostTransFunc == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_HostTransFunc length = (%d)", (int)strlen(szTicket_HostTransFunc));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_HostTransFunc[0], &szTicket_HostTransFunc[0], strlen(szTicket_HostTransFunc));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_HostEnable
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inGetTicket_HostEnable(char* szTicket_HostEnable)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_HostEnable == NULL || strlen(srTDTRec.szTicket_HostEnable) <= 0 || strlen(srTDTRec.szTicket_HostEnable) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTicket_HostEnable() ERROR !!");

			if (szTicket_HostEnable == NULL)
                        {
                                inDISP_LogPrintf("szTicket_HostEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_HostEnable length = (%d)", (int)strlen(srTDTRec.szTicket_HostEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_HostEnable[0], &srTDTRec.szTicket_HostEnable[0], strlen(srTDTRec.szTicket_HostEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_HostEnable
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inSetTicket_HostEnable(char* szTicket_HostEnable)
{
        memset(srTDTRec.szTicket_HostEnable, 0x00, sizeof(srTDTRec.szTicket_HostEnable));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_HostEnable == NULL || strlen(szTicket_HostEnable) <= 0 || strlen(szTicket_HostEnable) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTicket_HostEnable() ERROR !!");
                        if (szTicket_HostEnable == NULL)
                        {
                                inDISP_LogPrintf("szTicket_HostEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_HostEnable length = (%d)", (int)strlen(szTicket_HostEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_HostEnable[0], &szTicket_HostEnable[0], strlen(szTicket_HostEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_LogOnOK
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inGetTicket_LogOnOK(char* szTicket_LogOnOK)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_LogOnOK == NULL || strlen(srTDTRec.szTicket_LogOnOK) <= 0 || strlen(srTDTRec.szTicket_LogOnOK) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTicket_LogOnOK() ERROR !!");

			if (szTicket_LogOnOK == NULL)
                        {
                                inDISP_LogPrintf("szTicket_LogOnOK == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_LogOnOK length = (%d)", (int)strlen(srTDTRec.szTicket_LogOnOK));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_LogOnOK[0], &srTDTRec.szTicket_LogOnOK[0], strlen(srTDTRec.szTicket_LogOnOK));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_LogOnOK
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inSetTicket_LogOnOK(char* szTicket_LogOnOK)
{
        memset(srTDTRec.szTicket_LogOnOK, 0x00, sizeof(srTDTRec.szTicket_LogOnOK));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_LogOnOK == NULL || strlen(szTicket_LogOnOK) <= 0 || strlen(szTicket_LogOnOK) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTicket_LogOnOK() ERROR !!");
                        if (szTicket_LogOnOK == NULL)
                        {
                                inDISP_LogPrintf("szTicket_LogOnOK == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_LogOnOK length = (%d)", (int)strlen(szTicket_LogOnOK));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_LogOnOK[0], &szTicket_LogOnOK[0], strlen(szTicket_LogOnOK));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_SAM_Slot
Date&Time       :2018/1/4 下午 3:18
Describe        :
*/
int inGetTicket_SAM_Slot(char* szTicket_SAM_Slot)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_SAM_Slot == NULL || strlen(srTDTRec.szTicket_SAM_Slot) <= 0 || strlen(srTDTRec.szTicket_SAM_Slot) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTicket_SAM_Slot() ERROR !!");

			if (szTicket_SAM_Slot == NULL)
                        {
                                inDISP_LogPrintf("szTicket_SAM_Slot == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_SAM_Slot length = (%d)", (int)strlen(srTDTRec.szTicket_SAM_Slot));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_SAM_Slot[0], &srTDTRec.szTicket_SAM_Slot[0], strlen(srTDTRec.szTicket_SAM_Slot));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_SAM_Slot
Date&Time       :2018/1/4 下午 3:18
Describe        :
*/
int inSetTicket_SAM_Slot(char* szTicket_SAM_Slot)
{
        memset(srTDTRec.szTicket_SAM_Slot, 0x00, sizeof(srTDTRec.szTicket_SAM_Slot));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_SAM_Slot == NULL || strlen(szTicket_SAM_Slot) <= 0 || strlen(szTicket_SAM_Slot) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTicket_SAM_Slot() ERROR !!");
                        if (szTicket_SAM_Slot == NULL)
                        {
                                inDISP_LogPrintf("szTicket_SAM_Slot == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_SAM_Slot length = (%d)", (int)strlen(szTicket_SAM_Slot));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_SAM_Slot[0], &szTicket_SAM_Slot[0], strlen(szTicket_SAM_Slot));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_ReaderID
Date&Time       :2018/1/4 下午 3:19
Describe        :
*/
int inGetTicket_ReaderID(char* szTicket_ReaderID)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_ReaderID == NULL || strlen(srTDTRec.szTicket_ReaderID) <= 0 || strlen(srTDTRec.szTicket_ReaderID) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTicket_ReaderID() ERROR !!");

			if (szTicket_ReaderID == NULL)
                        {
                                inDISP_LogPrintf("szTicket_ReaderID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_ReaderID length = (%d)", (int)strlen(srTDTRec.szTicket_ReaderID));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_ReaderID[0], &srTDTRec.szTicket_ReaderID[0], strlen(srTDTRec.szTicket_ReaderID));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_ReaderID
Date&Time       :2018/1/4 下午 3:20
Describe        :
*/
int inSetTicket_ReaderID(char* szTicket_ReaderID)
{
        memset(srTDTRec.szTicket_ReaderID, 0x00, sizeof(srTDTRec.szTicket_ReaderID));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_ReaderID == NULL || strlen(szTicket_ReaderID) <= 0 || strlen(szTicket_ReaderID) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTicket_ReaderID() ERROR !!");
                        if (szTicket_ReaderID == NULL)
                        {
                                inDISP_LogPrintf("szTicket_ReaderID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_ReaderID length = (%d)", (int)strlen(szTicket_ReaderID));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_ReaderID[0], &szTicket_ReaderID[0], strlen(szTicket_ReaderID));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_STAN
Date&Time       :2018/3/22 下午 1:28
Describe        :
*/
int inGetTicket_STAN(char* szTicket_STAN)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_STAN == NULL || strlen(srTDTRec.szTicket_STAN) <= 0 || strlen(srTDTRec.szTicket_STAN) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTicket_STAN() ERROR !!");

			if (szTicket_STAN == NULL)
                        {
                                inDISP_LogPrintf("szTicket_STAN == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_STAN length = (%d)", (int)strlen(srTDTRec.szTicket_STAN));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_STAN[0], &srTDTRec.szTicket_STAN[0], strlen(srTDTRec.szTicket_STAN));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_STAN
Date&Time       :2018/3/22 下午 1:28
Describe        :
*/
int inSetTicket_STAN(char* szTicket_STAN)
{
        memset(srTDTRec.szTicket_STAN, 0x00, sizeof(srTDTRec.szTicket_STAN));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_STAN == NULL || strlen(szTicket_STAN) <= 0 || strlen(szTicket_STAN) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTicket_STAN() ERROR !!");
                        if (szTicket_STAN == NULL)
                        {
                                inDISP_LogPrintf("szTicket_STAN == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_STAN length = (%d)", (int)strlen(szTicket_STAN));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_STAN[0], &szTicket_STAN[0], strlen(szTicket_STAN));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_LastTransDate
Date&Time       :2018/3/22 下午 1:28
Describe        :
*/
int inGetTicket_LastTransDate(char* szTicket_LastTransDate)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_LastTransDate == NULL || strlen(srTDTRec.szTicket_LastTransDate) <= 0 || strlen(srTDTRec.szTicket_LastTransDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTicket_LastTransDate() ERROR !!");

			if (szTicket_LastTransDate == NULL)
                        {
                                inDISP_LogPrintf("szTicket_LastTransDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_LastTransDate length = (%d)", (int)strlen(srTDTRec.szTicket_LastTransDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_LastTransDate[0], &srTDTRec.szTicket_LastTransDate[0], strlen(srTDTRec.szTicket_LastTransDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_LastTransDate
Date&Time       :2018/3/22 下午 1:28
Describe        :
*/
int inSetTicket_LastTransDate(char* szTicket_LastTransDate)
{
        memset(srTDTRec.szTicket_LastTransDate, 0x00, sizeof(srTDTRec.szTicket_LastTransDate));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_LastTransDate == NULL || strlen(szTicket_LastTransDate) <= 0 || strlen(szTicket_LastTransDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTicket_LastTransDate() ERROR !!");
                        if (szTicket_LastTransDate == NULL)
                        {
                                inDISP_LogPrintf("szTicket_LastTransDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_LastTransDate length = (%d)", (int)strlen(szTicket_LastTransDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_LastTransDate[0], &szTicket_LastTransDate[0], strlen(szTicket_LastTransDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_LastRRN
Date&Time       :2018/3/28 下午 1:16
Describe        :
*/
int inGetTicket_LastRRN(char* szTicket_LastRRN)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_LastRRN == NULL || strlen(srTDTRec.szTicket_LastRRN) <= 0 || strlen(srTDTRec.szTicket_LastRRN) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTicket_LastRRN() ERROR !!");

			if (szTicket_LastRRN == NULL)
                        {
                                inDISP_LogPrintf("szTicket_LastRRN == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_LastRRN length = (%d)", (int)strlen(srTDTRec.szTicket_LastRRN));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_LastRRN[0], &srTDTRec.szTicket_LastRRN[0], strlen(srTDTRec.szTicket_LastRRN));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_LastRRN
Date&Time       :2018/3/28 下午 1:16
Describe        :
*/
int inSetTicket_LastRRN(char* szTicket_LastRRN)
{
        memset(srTDTRec.szTicket_LastRRN, 0x00, sizeof(srTDTRec.szTicket_LastRRN));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_LastRRN == NULL || strlen(szTicket_LastRRN) <= 0 || strlen(szTicket_LastRRN) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTicket_LastRRN() ERROR !!");
                        if (szTicket_LastRRN == NULL)
                        {
                                inDISP_LogPrintf("szTicket_LastRRN == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_LastRRN length = (%d)", (int)strlen(szTicket_LastRRN));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_LastRRN[0], &szTicket_LastRRN[0], strlen(szTicket_LastRRN));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_Device1
Date&Time       :2018/3/28 下午 1:16
Describe        :
*/
int inGetTicket_Device1(char* szTicket_Device1)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_Device1 == NULL || strlen(srTDTRec.szTicket_Device1) <= 0 || strlen(srTDTRec.szTicket_Device1) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTicket_Device1() ERROR !!");

			if (szTicket_Device1 == NULL)
                        {
                                inDISP_LogPrintf("szTicket_Device1 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_Device1 length = (%d)", (int)strlen(srTDTRec.szTicket_Device1));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_Device1[0], &srTDTRec.szTicket_Device1[0], strlen(srTDTRec.szTicket_Device1));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_Device1
Date&Time       :2018/3/28 下午 1:16
Describe        :
*/
int inSetTicket_Device1(char* szTicket_Device1)
{
        memset(srTDTRec.szTicket_Device1, 0x00, sizeof(srTDTRec.szTicket_Device1));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_Device1 == NULL || strlen(szTicket_Device1) <= 0 || strlen(szTicket_Device1) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTicket_Device1() ERROR !!");
                        if (szTicket_Device1 == NULL)
                        {
                                inDISP_LogPrintf("szTicket_Device1 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_Device1 length = (%d)", (int)strlen(szTicket_Device1));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_Device1[0], &szTicket_Device1[0], strlen(szTicket_Device1));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_Device2
Date&Time       :2018/3/28 下午 1:16
Describe        :
*/
int inGetTicket_Device2(char* szTicket_Device2)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_Device2 == NULL || strlen(srTDTRec.szTicket_Device2) <= 0 || strlen(srTDTRec.szTicket_Device2) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTicket_Device2() ERROR !!");

			if (szTicket_Device2 == NULL)
                        {
                                inDISP_LogPrintf("szTicket_Device2 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_Device2 length = (%d)", (int)strlen(srTDTRec.szTicket_Device2));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_Device2[0], &srTDTRec.szTicket_Device2[0], strlen(srTDTRec.szTicket_Device2));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_Device2
Date&Time       :2018/3/28 下午 1:16
Describe        :
*/
int inSetTicket_Device2(char* szTicket_Device2)
{
        memset(srTDTRec.szTicket_Device2, 0x00, sizeof(srTDTRec.szTicket_Device2));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_Device2 == NULL || strlen(szTicket_Device2) <= 0 || strlen(szTicket_Device2) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTicket_Device2() ERROR !!");
                        if (szTicket_Device2 == NULL)
                        {
                                inDISP_LogPrintf("szTicket_Device2 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_Device2 length = (%d)", (int)strlen(szTicket_Device2));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_Device2[0], &szTicket_Device2[0], strlen(szTicket_Device2));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_Batch
Date&Time       :2018/3/28 下午 1:16
Describe        :
*/
int inGetTicket_Batch(char* szTicket_Batch)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_Batch == NULL || strlen(srTDTRec.szTicket_Batch) <= 0 || strlen(srTDTRec.szTicket_Batch) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTicket_Batch() ERROR !!");

			if (szTicket_Batch == NULL)
                        {
                                inDISP_LogPrintf("szTicket_Batch == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_Batch length = (%d)", (int)strlen(srTDTRec.szTicket_Batch));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_Batch[0], &srTDTRec.szTicket_Batch[0], strlen(srTDTRec.szTicket_Batch));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_Batch
Date&Time       :2018/3/28 下午 1:16
Describe        :
*/
int inSetTicket_Batch(char* szTicket_Batch)
{
        memset(srTDTRec.szTicket_Batch, 0x00, sizeof(srTDTRec.szTicket_Batch));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_Batch == NULL || strlen(szTicket_Batch) <= 0 || strlen(szTicket_Batch) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTicket_Batch() ERROR !!");
                        if (szTicket_Batch == NULL)
                        {
                                inDISP_LogPrintf("szTicket_Batch == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_Batch length = (%d)", (int)strlen(szTicket_Batch));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_Batch[0], &szTicket_Batch[0], strlen(szTicket_Batch));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_FirstTxnDate
Date&Time       :2018/3/28 下午 1:16
Describe        :
*/
int inGetTicket_NeedNewBatch(char* szTicket_NeedNewBatch)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_NeedNewBatch == NULL || strlen(srTDTRec.szTicket_NeedNewBatch) <= 0 || strlen(srTDTRec.szTicket_NeedNewBatch) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTicket_FirstTxnDate() ERROR !!");

			if (szTicket_NeedNewBatch == NULL)
                        {
                                inDISP_LogPrintf("szTicket_FirstTxnDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_FirstTxnDate length = (%d)", (int)strlen(srTDTRec.szTicket_NeedNewBatch));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_NeedNewBatch[0], &srTDTRec.szTicket_NeedNewBatch[0], strlen(srTDTRec.szTicket_NeedNewBatch));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_FirstTxnDate
Date&Time       :2018/3/28 下午 1:16
Describe        :
*/
int inSetTicket_NeedNewBatch(char* szTicket_NeedNewBatch)
{
        memset(srTDTRec.szTicket_NeedNewBatch, 0x00, sizeof(srTDTRec.szTicket_NeedNewBatch));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_NeedNewBatch == NULL || strlen(szTicket_NeedNewBatch) <= 0 || strlen(szTicket_NeedNewBatch) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTicket_FirstTxnDate() ERROR !!");
                        if (szTicket_NeedNewBatch == NULL)
                        {
                                inDISP_LogPrintf("szTicket_FirstTxnDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_FirstTxnDate length = (%d)", (int)strlen(szTicket_NeedNewBatch));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_NeedNewBatch[0], &szTicket_NeedNewBatch[0], strlen(szTicket_NeedNewBatch));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_Device3
Date&Time       :2018/7/10 下午 3:44
Describe        :
*/
int inGetTicket_Device3(char* szTicket_Device3)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_Device3 == NULL || strlen(srTDTRec.szTicket_Device3) <= 0 || strlen(srTDTRec.szTicket_Device3) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTicket_Device3() ERROR !!");

			if (szTicket_Device3 == NULL)
                        {
                                inDISP_LogPrintf("szTicket_Device3 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_Device3 length = (%d)", (int)strlen(srTDTRec.szTicket_Device3));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_Device3[0], &srTDTRec.szTicket_Device3[0], strlen(srTDTRec.szTicket_Device3));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_Device3
Date&Time       :2018/7/10 下午 3:44
Describe        :
*/
int inSetTicket_Device3(char* szTicket_Device3)
{
        memset(srTDTRec.szTicket_Device3, 0x00, sizeof(srTDTRec.szTicket_Device3));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_Device3 == NULL || strlen(szTicket_Device3) <= 0 || strlen(szTicket_Device3) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTicket_Device3() ERROR !!");
                        if (szTicket_Device3 == NULL)
                        {
                                inDISP_LogPrintf("szTicket_Device3 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_Device3 length = (%d)", (int)strlen(szTicket_Device3));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_Device3[0], &szTicket_Device3[0], strlen(szTicket_Device3));

        return (VS_SUCCESS);
}

/*
Function        :inTDT_Edit_TDT_Table
Date&Time       :2017/5/15 下午 4:08
Describe        :
*/
int inTDT_Edit_TDT_Table(void)
{
	TABLE_GET_SET_TABLE TDT_FUNC_TABLE[] =
	{
		{"szTicket_HostIndex"			,inGetTicket_HostIndex			,inSetTicket_HostIndex			},
		{"szTicket_HostName"			,inGetTicket_HostName			,inSetTicket_HostName			},
		{"szTicket_HostTransFunc"		,inGetTicket_HostTransFunc		,inSetTicket_HostTransFunc		},
		{"szTicket_HostEnable"			,inGetTicket_HostEnable			,inSetTicket_HostEnable			},
		{"szTicket_LogOnOK"			,inGetTicket_LogOnOK			,inSetTicket_LogOnOK			},
		{"szTicket_SAM_Slot"			,inGetTicket_SAM_Slot			,inSetTicket_SAM_Slot			},
		{"szTicket_ReaderID"			,inGetTicket_ReaderID			,inSetTicket_ReaderID			},
		{"szTicket_STAN"			,inGetTicket_STAN			,inSetTicket_STAN			},
		{"szTicket_LastTransDate"		,inGetTicket_LastTransDate		,inSetTicket_LastTransDate		},
		{"szTicket_LastRRN"			,inGetTicket_LastRRN			,inSetTicket_LastRRN			},
		{"szTicket_Device1"			,inGetTicket_Device1			,inSetTicket_Device1			},
		{"szTicket_Device2"			,inGetTicket_Device2			,inSetTicket_Device2			},
		{"szTicket_Batch"			,inGetTicket_Batch			,inSetTicket_Batch			},
		{"szTicket_NeedNewBatch"		,inGetTicket_NeedNewBatch		,inSetTicket_NeedNewBatch		},
		{"szTicket_Device3"			,inGetTicket_Device3			,inSetTicket_Device3			},
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
	inDISP_ChineseFont_Color("是否更改TDT", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
	inLoadTDTRec(inRecordCnt);
	
	inFunc_Edit_Table_Tag(TDT_FUNC_TABLE);
	inSaveTDTRec(inRecordCnt);
	
	return	(VS_SUCCESS);
}
