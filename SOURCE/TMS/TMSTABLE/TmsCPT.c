#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>

#include "../../INCLUDES/Define_1.h"
#include "../../INCLUDES/Define_2.h"
#include "../../INCLUDES/Transaction.h"
#include "../../PRINT/Print.h"
#include "../../DISPLAY/Display.h"
#include "../../FUNCTION/FILE_FUNC/File.h"
#include "../../FUNCTION/FILE_FUNC/FIleLogFunc.h"
#include "../../FUNCTION/Function.h"

#include "TmsCPT.h"

static  TMSCPT_REC	srTMSCPTRec;	/* construct TMSCPT record */
extern  int		ginDebug;       /* Debug使用 extern */

/*
Function        :inLoadTMSCPTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀TMSCPT檔案，inTMSCPTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadTMSCPTRec(int inTMSCPTRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆TMSCPT檔案 */
        char            szTMSCPTRec[_SIZE_TMSCPT_REC_ + 1];           /* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnTMSCPTLength = 0;                        /* TMSCPT總長度 */
        long            lnReadLength;                           /* 記錄每次要從TMSCPT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從TMSCPT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */
       
        /* inLoadTMSCPTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadTMSCPTRec(%d) START!!", inTMSCPTRec);
                inDISP_LogPrintf(szErrorMsg);
        }

        /* 判斷傳進來的inTMSCPTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inTMSCPTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inTMSCPTRec < 0:(index = %d) ERROR!!", inTMSCPTRec);
                        inDISP_LogPrintf(szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open TMSCPT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_TMSCPT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnTMSCPTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_TMSCPT_FILE_NAME_);
        
        if (lnTMSCPTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnTMSCPTLength + 1);
        uszTemp = malloc(lnTMSCPTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnTMSCPTLength + 1);
        memset(uszTemp, 0x00, lnTMSCPTLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnTMSCPTLength;

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
         *i為目前從TMSCPT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnTMSCPTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到TMSCPT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnTMSCPTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inTMSCPT_Rec的index從0開始，所以inTMSCPT_Rec要+1 */
                        if (inRec == (inTMSCPTRec + 1))
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
         * 如果沒有inTMSCPTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inTMSCPTRec + 1) || inSearchResult == -1)
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
        memset(&srTMSCPTRec, 0x00, sizeof(srTMSCPTRec));
        /*
         * 以下pattern為存入TMSCPT_Rec
         * i為TMSCPT的第幾個字元
         * 存入TMSCPT_Rec
         */
        i = 0;


        /* 01_TMS IP Address */
        /* 初始化 */
        memset(szTMSCPTRec, 0x00, sizeof(szTMSCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSCPTRec[k ++] = uszReadData[i ++];
                if (szTMSCPTRec[k - 1] == 0x2C	||
		    szTMSCPTRec[k - 1] == 0x0D	||
		    szTMSCPTRec[k - 1] == 0x0A	||
		    szTMSCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSCPT unpack ERROR.");
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
        if (szTMSCPTRec[0] != 0x2C	&&
	    szTMSCPTRec[0] != 0x0D	&&
	    szTMSCPTRec[0] != 0x0A	&&
	    szTMSCPTRec[0] != 0x00)
        {
                memcpy(&srTMSCPTRec.szTMSIPAddress[0], &szTMSCPTRec[0], k - 1);
        }

        /* 02_TMS Port Number */
        /* 初始化 */
        memset(szTMSCPTRec, 0x00, sizeof(szTMSCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSCPTRec[k ++] = uszReadData[i ++];
                if (szTMSCPTRec[k - 1] == 0x2C	||
		    szTMSCPTRec[k - 1] == 0x0D	||
		    szTMSCPTRec[k - 1] == 0x0A	||
		    szTMSCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSCPT unpack ERROR");
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
        if (szTMSCPTRec[0] != 0x2C	&&
	    szTMSCPTRec[0] != 0x0D	&&
	    szTMSCPTRec[0] != 0x0A	&&
	    szTMSCPTRec[0] != 0x00)
        {
                memcpy(&srTMSCPTRec.szTMSPortNum[0], &szTMSCPTRec[0], k - 1);
        }

        /* 03_TMS Phone Number */
        /* 初始化 */
        memset(szTMSCPTRec, 0x00, sizeof(szTMSCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSCPTRec[k ++] = uszReadData[i ++];
                if (szTMSCPTRec[k - 1] == 0x2C	||
		    szTMSCPTRec[k - 1] == 0x0D	||
		    szTMSCPTRec[k - 1] == 0x0A	||
		    szTMSCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSCPT unpack ERROR");
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
        if (szTMSCPTRec[0] != 0x2C	&&
	    szTMSCPTRec[0] != 0x0D	&&
	    szTMSCPTRec[0] != 0x0A	&&
	    szTMSCPTRec[0] != 0x00)
        {
                memcpy(&srTMSCPTRec.szTMSPhoneNumber[0], &szTMSCPTRec[0], k - 1);
        }

        /* 04_結帳後更新參數的開關 */
        /* 初始化 */
        memset(szTMSCPTRec, 0x00, sizeof(szTMSCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSCPTRec[k ++] = uszReadData[i ++];
                if (szTMSCPTRec[k - 1] == 0x2C	||
		    szTMSCPTRec[k - 1] == 0x0D	||
		    szTMSCPTRec[k - 1] == 0x0A	||
		    szTMSCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSCPT unpack ERROR");
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
        if (szTMSCPTRec[0] != 0x2C	&&
	    szTMSCPTRec[0] != 0x0D	&&
	    szTMSCPTRec[0] != 0x0A	&&
	    szTMSCPTRec[0] != 0x00)
        {
                memcpy(&srTMSCPTRec.szTMSEffectiveCloseBatch[0], &szTMSCPTRec[0], k - 1);
        }
        
        /* 05_參數生效的日期 */
        /* 初始化 */
        memset(szTMSCPTRec, 0x00, sizeof(szTMSCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSCPTRec[k ++] = uszReadData[i ++];
                if (szTMSCPTRec[k - 1] == 0x2C	||
		    szTMSCPTRec[k - 1] == 0x0D	||
		    szTMSCPTRec[k - 1] == 0x0A	||
		    szTMSCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSCPT unpack ERROR");
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
        if (szTMSCPTRec[0] != 0x2C	&&
	    szTMSCPTRec[0] != 0x0D	&&
	    szTMSCPTRec[0] != 0x0A	&&
	    szTMSCPTRec[0] != 0x00)
        {
                memcpy(&srTMSCPTRec.szTMSEffectiveDate[0], &szTMSCPTRec[0], k - 1);
        }
        
        /* 06_參數生效的時間 */
        /* 初始化 */
        memset(szTMSCPTRec, 0x00, sizeof(szTMSCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSCPTRec[k ++] = uszReadData[i ++];
                if (szTMSCPTRec[k - 1] == 0x2C	||
		    szTMSCPTRec[k - 1] == 0x0D	||
		    szTMSCPTRec[k - 1] == 0x0A	||
		    szTMSCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSCPT unpack ERROR");
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
        if (szTMSCPTRec[0] != 0x2C	&&
	    szTMSCPTRec[0] != 0x0D	&&
	    szTMSCPTRec[0] != 0x0A	&&
	    szTMSCPTRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srTMSCPTRec.szTMSEffectiveTime[0], &szTMSCPTRec[0], k - 1);
        }

	/* 07_排程下載的時機 */
        /* 初始化 */
        memset(szTMSCPTRec, 0x00, sizeof(szTMSCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSCPTRec[k ++] = uszReadData[i ++];
                if (szTMSCPTRec[k - 1] == 0x2C	||
		    szTMSCPTRec[k - 1] == 0x0D	||
		    szTMSCPTRec[k - 1] == 0x0A	||
		    szTMSCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSCPT unpack ERROR");
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
        if (szTMSCPTRec[0] != 0x2C	&&
	    szTMSCPTRec[0] != 0x0D	&&
	    szTMSCPTRec[0] != 0x0A	&&
	    szTMSCPTRec[0] != 0x00)
        {
                memcpy(&srTMSCPTRec.szTMSDownloadFlag[0], &szTMSCPTRec[0], k - 1);
        }
	
        /* 08_排程下載的類型 */
        /* 初始化 */
        memset(szTMSCPTRec, 0x00, sizeof(szTMSCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSCPTRec[k ++] = uszReadData[i ++];
                if (szTMSCPTRec[k - 1] == 0x2C	||
		    szTMSCPTRec[k - 1] == 0x0D	||
		    szTMSCPTRec[k - 1] == 0x0A	||
		    szTMSCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSCPT unpack ERROR");
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
        if (szTMSCPTRec[0] != 0x2C	&&
	    szTMSCPTRec[0] != 0x0D	&&
	    szTMSCPTRec[0] != 0x0A	&&
	    szTMSCPTRec[0] != 0x00)
        {
                memcpy(&srTMSCPTRec.szTMSDownloadScope[0], &szTMSCPTRec[0], k - 1);
        }
        
        /* 09_排程下載日期 */
        /* 初始化 */
        memset(szTMSCPTRec, 0x00, sizeof(szTMSCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSCPTRec[k ++] = uszReadData[i ++];
                if (szTMSCPTRec[k - 1] == 0x2C	||
		    szTMSCPTRec[k - 1] == 0x0D	||
		    szTMSCPTRec[k - 1] == 0x0A	||
		    szTMSCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSCPT unpack ERROR");
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
        if (szTMSCPTRec[0] != 0x2C	&&
	    szTMSCPTRec[0] != 0x0D	&&
	    szTMSCPTRec[0] != 0x0A	&&
	    szTMSCPTRec[0] != 0x00)
        {
                memcpy(&srTMSCPTRec.szTMSScheduleDate[0], &szTMSCPTRec[0], k - 1);
        }
        
        /* 10_排程下載時間 */
        /* 初始化 */
        memset(szTMSCPTRec, 0x00, sizeof(szTMSCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSCPTRec[k ++] = uszReadData[i ++];
                if (szTMSCPTRec[k - 1] == 0x2C	||
		    szTMSCPTRec[k - 1] == 0x0D	||
		    szTMSCPTRec[k - 1] == 0x0A	||
		    szTMSCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSCPT unpack ERROR");
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
        if (szTMSCPTRec[0] != 0x2C	&&
	    szTMSCPTRec[0] != 0x0D	&&
	    szTMSCPTRec[0] != 0x0A	&&
	    szTMSCPTRec[0] != 0x00)
        {
                memcpy(&srTMSCPTRec.szTMSScheduleTime[0], &szTMSCPTRec[0], k - 1);
        }
	
	/* 11_排程下載重試次數 */
        /* 初始化 */
        memset(szTMSCPTRec, 0x00, sizeof(szTMSCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSCPTRec[k ++] = uszReadData[i ++];
                if (szTMSCPTRec[k - 1] == 0x2C	||
		    szTMSCPTRec[k - 1] == 0x0D	||
		    szTMSCPTRec[k - 1] == 0x0A	||
		    szTMSCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSCPT unpack ERROR");
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
        if (szTMSCPTRec[0] != 0x2C	&&
	    szTMSCPTRec[0] != 0x0D	&&
	    szTMSCPTRec[0] != 0x0A	&&
	    szTMSCPTRec[0] != 0x00)
        {
                memcpy(&srTMSCPTRec.szTMSScheduleRetry[0], &szTMSCPTRec[0], k - 1);
        }
	
	/* 12_是否需要更新AP */
        /* 初始化 */
        memset(szTMSCPTRec, 0x00, sizeof(szTMSCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSCPTRec[k ++] = uszReadData[i ++];
                if (szTMSCPTRec[k - 1] == 0x2C	||
		    szTMSCPTRec[k - 1] == 0x0D	||
		    szTMSCPTRec[k - 1] == 0x0A	||
		    szTMSCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSCPT unpack ERROR");
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
        if (szTMSCPTRec[0] != 0x2C	&&
	    szTMSCPTRec[0] != 0x0D	&&
	    szTMSCPTRec[0] != 0x0A	&&
	    szTMSCPTRec[0] != 0x00)
        {
                memcpy(&srTMSCPTRec.szTMSAPPUpdateStatus[0], &szTMSCPTRec[0], k - 1);
        }
	
	/* 13_TMSNII */
        /* 初始化 */
        memset(szTMSCPTRec, 0x00, sizeof(szTMSCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSCPTRec[k ++] = uszReadData[i ++];
                if (szTMSCPTRec[k - 1] == 0x2C	||
		    szTMSCPTRec[k - 1] == 0x0D	||
		    szTMSCPTRec[k - 1] == 0x0A	||
		    szTMSCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSCPT unpack ERROR");
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
        if (szTMSCPTRec[0] != 0x2C	&&
	    szTMSCPTRec[0] != 0x0D	&&
	    szTMSCPTRec[0] != 0x0A	&&
	    szTMSCPTRec[0] != 0x00)
        {
                memcpy(&srTMSCPTRec.szTMSNII[0], &szTMSCPTRec[0], k - 1);
        }
	
	/* 14_是否需要參數回報 */
        /* 初始化 */
        memset(szTMSCPTRec, 0x00, sizeof(szTMSCPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSCPTRec[k ++] = uszReadData[i ++];
                if (szTMSCPTRec[k - 1] == 0x2C	||
		    szTMSCPTRec[k - 1] == 0x0D	||
		    szTMSCPTRec[k - 1] == 0x0A	||
		    szTMSCPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSCPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSCPT unpack ERROR");
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
        if (szTMSCPTRec[0] != 0x2C	&&
	    szTMSCPTRec[0] != 0x0D	&&
	    szTMSCPTRec[0] != 0x0A	&&
	    szTMSCPTRec[0] != 0x00)
        {
                memcpy(&srTMSCPTRec.szTMSEffectiveReportBit[0], &szTMSCPTRec[0], k - 1);
        }
	
        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadTMSCPTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadTMSCPTRec(%d) END!!", inTMSCPTRec);
                inDISP_LogPrintf(szErrorMsg);
                inDISP_LogPrintf("----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveTMSCPTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveTMSCPTRec(int inTMSCPTRec)
{
 	unsigned long   uldat_Handle;   			/* FILE Handle */
	int inRecCount = 0, inTempIndex = 0;
	int     	inRetVal;
	int     	i = 0;

	char szTempTitle[] = "inSaveTMSCPTRec";
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
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inSaveTMSCPTRec INIT" );
#endif

	inTempIndex = inTMSCPTRec;
	
	memset(szTempFIleName, 0x00, sizeof(szTempFIleName));
	strcat( szTempFIleName, _TMSCPT_FILE_NAME_);
	
	memset(szTempBakFileName, 0x00, sizeof(szTempBakFileName));
	strcat( szTempBakFileName, _TMSCPT_FILE_NAME_BAK_);
	
	inDISP_LogPrintfWithFlag("  %s Index[%d] ", szTempTitle, inTempIndex);

      	/* 開啟原檔案HDPT.dat */
	inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_TMSCPT_FILE_NAME_);

	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  %s *Error* [%d] ",szTempTitle, inRetVal);
		inFunc_EDCLock();
		return(VS_ERROR);
	}

	usRetVal = CTOS_FileGetSize((unsigned char *)_TMSCPT_FILE_NAME_, &ulFileSize);	
	
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
	uszWriteBuff_Record = malloc(_SIZE_TMSCPT_REC_ + _SIZE_TMSCPT_COMMA_0D0A_);
	memset(uszWriteBuff_Record, 0x00, _SIZE_TMSCPT_REC_ + _SIZE_TMSCPT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

	/* uszRead_Total_Buff儲存TMSCPT.dat全部資料 */
	uszRead_Total_Buff = malloc(ulFileSize + 1);
	memset(uszRead_Total_Buff, 0x00, ulFileSize + 1);

	ulPackCount = 0; /* uszWriteBuff_Record的index位置 */
	/* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* TMSIPAddress */
	memcpy(&uszWriteBuff_Record[0], &srTMSCPTRec.szTMSIPAddress[0], strlen(srTMSCPTRec.szTMSIPAddress));
	ulPackCount += strlen(srTMSCPTRec.szTMSIPAddress);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TMSPortNum */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSCPTRec.szTMSPortNum[0], strlen(srTMSCPTRec.szTMSPortNum));
	ulPackCount += strlen(srTMSCPTRec.szTMSPortNum);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TMSPhoneNumber */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSCPTRec.szTMSPhoneNumber[0], strlen(srTMSCPTRec.szTMSPhoneNumber));
	ulPackCount += strlen(srTMSCPTRec.szTMSPhoneNumber);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szTMSEffectiveCloseBatch */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSCPTRec.szTMSEffectiveCloseBatch[0], strlen(srTMSCPTRec.szTMSEffectiveCloseBatch));
	ulPackCount += strlen(srTMSCPTRec.szTMSEffectiveCloseBatch);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szTMSEffectiveDate */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSCPTRec.szTMSEffectiveDate[0], strlen(srTMSCPTRec.szTMSEffectiveDate));
	ulPackCount += strlen(srTMSCPTRec.szTMSEffectiveDate);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szTMSEffectiveTime */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSCPTRec.szTMSEffectiveTime[0], strlen(srTMSCPTRec.szTMSEffectiveTime));
	ulPackCount += strlen(srTMSCPTRec.szTMSEffectiveTime);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TMSDownloadFlag */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSCPTRec.szTMSDownloadFlag[0], strlen(srTMSCPTRec.szTMSDownloadFlag));
	ulPackCount += strlen(srTMSCPTRec.szTMSDownloadFlag);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TMSDownloadScope */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSCPTRec.szTMSDownloadScope[0], strlen(srTMSCPTRec.szTMSDownloadScope));
	ulPackCount += strlen(srTMSCPTRec.szTMSDownloadScope);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TMSScheduleDate */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSCPTRec.szTMSScheduleDate[0], strlen(srTMSCPTRec.szTMSScheduleDate));
	ulPackCount += strlen(srTMSCPTRec.szTMSScheduleDate);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TMSScheduleTime */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSCPTRec.szTMSScheduleTime[0], strlen(srTMSCPTRec.szTMSScheduleTime));
	ulPackCount += strlen(srTMSCPTRec.szTMSScheduleTime);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TMSScheduleRetry */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSCPTRec.szTMSScheduleRetry[0], strlen(srTMSCPTRec.szTMSScheduleRetry));
	ulPackCount += strlen(srTMSCPTRec.szTMSScheduleRetry);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TMSScheduleRetry */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSCPTRec.szTMSAPPUpdateStatus[0], strlen(srTMSCPTRec.szTMSAPPUpdateStatus));
	ulPackCount += strlen(srTMSCPTRec.szTMSAPPUpdateStatus);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szTMSNII */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSCPTRec.szTMSNII[0], strlen(srTMSCPTRec.szTMSNII));
	ulPackCount += strlen(srTMSCPTRec.szTMSNII);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szTMSEffectiveReportBit */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSCPTRec.szTMSEffectiveReportBit[0], strlen(srTMSCPTRec.szTMSEffectiveReportBit));
	ulPackCount += strlen(srTMSCPTRec.szTMSEffectiveReportBit);

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
	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inSaveTMSCPTRec END");
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
Function        :inGetTMSIPAddress
Date&Time       :
Describe        :
*/
int inGetTMSIPAddress(char* szTMSIPAddress)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSIPAddress == NULL || strlen(srTMSCPTRec.szTMSIPAddress) <= 0 || strlen(srTMSCPTRec.szTMSIPAddress) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTMSIPAddress() ERROR !!");
                        
			if (szTMSIPAddress == NULL)
                        {
                                inDISP_LogPrintf("szTMSIPAddress == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSIPAddress length = (%d)", (int)strlen(srTMSCPTRec.szTMSIPAddress));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSIPAddress[0], &srTMSCPTRec.szTMSIPAddress[0], strlen(srTMSCPTRec.szTMSIPAddress));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSIPAddress
Date&Time       :
Describe        :
*/
int inSetTMSIPAddress(char* szTMSIPAddress)
{
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTMSIPAddress == NULL || strlen(szTMSIPAddress) <= 0 || strlen(szTMSIPAddress) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTMSIPAddress() ERROR !!");
                        if (szTMSIPAddress == NULL)
                        {
                                inDISP_LogPrintf("szTMSIPAddress == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSIPAddress length = (%d)", (int)strlen(szTMSIPAddress));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }

        memset(srTMSCPTRec.szTMSIPAddress, 0x00, sizeof(srTMSCPTRec.szTMSIPAddress));
        memcpy(&srTMSCPTRec.szTMSIPAddress[0], &szTMSIPAddress[0], strlen(szTMSIPAddress));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSPortNum
Date&Time       :
Describe        :
*/
int inGetTMSPortNum(char* szTMSPortNum)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSPortNum == NULL || strlen(srTMSCPTRec.szTMSPortNum) <= 0 || strlen(srTMSCPTRec.szTMSPortNum) > 5)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTMSPortNum() ERROR !!");

                        if (szTMSPortNum == NULL)
                        {
                                inDISP_LogPrintf("szTMSPortNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSPortNum length = (%d)", (int)strlen(srTMSCPTRec.szTMSPortNum));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSPortNum[0], &srTMSCPTRec.szTMSPortNum[0], strlen(srTMSCPTRec.szTMSPortNum));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSPortNum
Date&Time       :
Describe        :
*/
int inSetTMSPortNum(char* szTMSPortNum)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSPortNum == NULL || strlen(szTMSPortNum) <= 0 || strlen(szTMSPortNum) > 5)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTMSPortNum() ERROR !!");

                        if (szTMSPortNum == NULL)
                        {
                                inDISP_LogPrintf("szTMSPortNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSPortNum length = (%d)", (int)strlen(szTMSPortNum));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }

        memset(srTMSCPTRec.szTMSPortNum, 0x00, sizeof(srTMSCPTRec.szTMSPortNum));
        memcpy(&srTMSCPTRec.szTMSPortNum[0], &szTMSPortNum[0], strlen(szTMSPortNum));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSPhoneNumber
Date&Time       :
Describe        :
*/
int inGetTMSPhoneNumber(char* szTMSPhoneNumber)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSPhoneNumber == NULL || strlen(srTMSCPTRec.szTMSPhoneNumber) <= 0 || strlen(srTMSCPTRec.szTMSPhoneNumber) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTMSPhoneNumber() ERROR !!");

                        if (szTMSPhoneNumber == NULL)
                        {
                                inDISP_LogPrintf("szTMSPhoneNumber == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSPhoneNumber length = (%d)", (int)strlen(srTMSCPTRec.szTMSPhoneNumber));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSPhoneNumber[0], &srTMSCPTRec.szTMSPhoneNumber[0], strlen(srTMSCPTRec.szTMSPhoneNumber));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSPhoneNumber
Date&Time       :
Describe        :
*/
int inSetTMSPhoneNumber(char* szTMSPhoneNumber)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSPhoneNumber == NULL || strlen(szTMSPhoneNumber) <= 0 || strlen(szTMSPhoneNumber) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTMSPhoneNumber() ERROR !!");

                        if (szTMSPhoneNumber == NULL)
                        {
                                inDISP_LogPrintf("szTMSPhoneNumber == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSPhoneNumber length = (%d)", (int)strlen(szTMSPhoneNumber));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSCPTRec.szTMSPhoneNumber, 0x00, sizeof(srTMSCPTRec.szTMSPhoneNumber));
        memcpy(&srTMSCPTRec.szTMSPhoneNumber[0], &szTMSPhoneNumber[0], strlen(szTMSPhoneNumber));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSEffectiveCloseBatch
Date&Time       :
Describe        :
*/
int inGetTMSEffectiveCloseBatch(char* szTMSEffectiveCloseBatch)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSEffectiveCloseBatch == NULL || strlen(srTMSCPTRec.szTMSEffectiveCloseBatch) <= 0 || strlen(srTMSCPTRec.szTMSEffectiveCloseBatch) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTMSEffectiveCloseBatch() ERROR !!");

                        if (szTMSEffectiveCloseBatch == NULL)
                        {
                                inDISP_LogPrintf("szTMSEffectiveCloseBatch == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSEffectiveCloseBatch length = (%d)", (int)strlen(srTMSCPTRec.szTMSEffectiveCloseBatch));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSEffectiveCloseBatch[0], &srTMSCPTRec.szTMSEffectiveCloseBatch[0], strlen(srTMSCPTRec.szTMSEffectiveCloseBatch));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSEffectiveCloseBatch
Date&Time       :
Describe        :
*/
int inSetTMSEffectiveCloseBatch(char* szTMSEffectiveCloseBatch)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSEffectiveCloseBatch == NULL || strlen(szTMSEffectiveCloseBatch) <= 0 || strlen(szTMSEffectiveCloseBatch) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTMSEffectiveCloseBatch() ERROR !!");

                        if (szTMSEffectiveCloseBatch == NULL)
                        {
                                inDISP_LogPrintf("szTMSEffectiveCloseBatch == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSEffectiveCloseBatch length = (%d)", (int)strlen(szTMSEffectiveCloseBatch));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSCPTRec.szTMSEffectiveCloseBatch, 0x00, sizeof(srTMSCPTRec.szTMSEffectiveCloseBatch));
        memcpy(&srTMSCPTRec.szTMSEffectiveCloseBatch[0], &szTMSEffectiveCloseBatch[0], strlen(szTMSEffectiveCloseBatch));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSEffectiveDate
Date&Time       :
Describe        :
*/
int inGetTMSEffectiveDate(char* szTMSEffectiveDate)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSEffectiveDate == NULL || strlen(srTMSCPTRec.szTMSEffectiveDate) <= 0 || strlen(srTMSCPTRec.szTMSEffectiveDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTMSEffectiveDate() ERROR !!");

                        if (szTMSEffectiveDate == NULL)
                        {
                                inDISP_LogPrintf("szTMSEffectiveDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSEffectiveDate length = (%d)", (int)strlen(srTMSCPTRec.szTMSEffectiveDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSEffectiveDate[0], &srTMSCPTRec.szTMSEffectiveDate[0], strlen(srTMSCPTRec.szTMSEffectiveDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSEffectiveDate
Date&Time       :
Describe        :
*/
int inSetTMSEffectiveDate(char* szTMSEffectiveDate)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSEffectiveDate == NULL || strlen(szTMSEffectiveDate) <= 0 || strlen(szTMSEffectiveDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTMSEffectiveDate() ERROR !!");

                        if (szTMSEffectiveDate == NULL)
                        {
                                inDISP_LogPrintf("szTMSEffectiveDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSEffectiveDate length = (%d)", (int)strlen(szTMSEffectiveDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSCPTRec.szTMSEffectiveDate, 0x00, sizeof(srTMSCPTRec.szTMSEffectiveDate));
        memcpy(&srTMSCPTRec.szTMSEffectiveDate[0], &szTMSEffectiveDate[0], strlen(szTMSEffectiveDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSEffectiveTime
Date&Time       :
Describe        :
*/
int inGetTMSEffectiveTime(char* szTMSEffectiveTime)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSEffectiveTime == NULL || strlen(srTMSCPTRec.szTMSEffectiveTime) <= 0 || strlen(srTMSCPTRec.szTMSEffectiveTime) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTMSEffectiveTime() ERROR !!");

                        if (szTMSEffectiveTime == NULL)
                        {
                                inDISP_LogPrintf("szTMSEffectiveTime == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSEffectiveTime length = (%d)", (int)strlen(srTMSCPTRec.szTMSEffectiveTime));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSEffectiveTime[0], &srTMSCPTRec.szTMSEffectiveTime[0], strlen(srTMSCPTRec.szTMSEffectiveTime));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSEffectiveTime
Date&Time       :
Describe        :
*/
int inSetTMSEffectiveTime(char* szTMSEffectiveTime)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSEffectiveTime == NULL || strlen(szTMSEffectiveTime) <= 0 || strlen(szTMSEffectiveTime) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTMSEffectiveTime() ERROR !!");

                        if (szTMSEffectiveTime == NULL)
                        {
                                inDISP_LogPrintf("szTMSEffectiveTime == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSEffectiveTime length = (%d)", (int)strlen(szTMSEffectiveTime));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSCPTRec.szTMSEffectiveTime, 0x00, sizeof(srTMSCPTRec.szTMSEffectiveTime));
        memcpy(&srTMSCPTRec.szTMSEffectiveTime[0], &szTMSEffectiveTime[0], strlen(szTMSEffectiveTime));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSDownloadFlag
Date&Time       :2017/1/17 上午 11:40
Describe        :
*/
int inGetTMSDownloadFlag(char* szTMSDownloadFlag)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSDownloadFlag == NULL || strlen(srTMSCPTRec.szTMSDownloadFlag) <= 0 || strlen(srTMSCPTRec.szTMSDownloadFlag) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTMSDownloadFlag() ERROR !!");

                        if (szTMSDownloadFlag == NULL)
                        {
                                inDISP_LogPrintf("szTMSDownloadFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSDownloadFlag length = (%d)", (int)strlen(srTMSCPTRec.szTMSDownloadFlag));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSDownloadFlag[0], &srTMSCPTRec.szTMSDownloadFlag[0], strlen(srTMSCPTRec.szTMSDownloadFlag));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSDownloadFlag
Date&Time       :
Describe        :
*/
int inSetTMSDownloadFlag(char* szTMSDownloadFlag)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSDownloadFlag == NULL || strlen(szTMSDownloadFlag) <= 0 || strlen(szTMSDownloadFlag) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTMSDownloadFlag() ERROR !!");

                        if (szTMSDownloadFlag == NULL)
                        {
                                inDISP_LogPrintf("szTMSDownloadFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSDownloadFlag length = (%d)", (int)strlen(szTMSDownloadFlag));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSCPTRec.szTMSDownloadFlag, 0x00, sizeof(srTMSCPTRec.szTMSDownloadFlag));
        memcpy(&srTMSCPTRec.szTMSDownloadFlag[0], &szTMSDownloadFlag[0], strlen(szTMSDownloadFlag));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSDownloadScope
Date&Time       :
Describe        :
*/
int inGetTMSDownloadScope(char* szTMSDownloadScope)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSDownloadScope == NULL || strlen(srTMSCPTRec.szTMSDownloadScope) <= 0 || strlen(srTMSCPTRec.szTMSDownloadScope) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTMSDownloadScope() ERROR !!");

                        if (szTMSDownloadScope == NULL)
                        {
                                inDISP_LogPrintf("szTMSDownloadScope == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSDownloadScope length = (%d)", (int)strlen(srTMSCPTRec.szTMSDownloadScope));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSDownloadScope[0], &srTMSCPTRec.szTMSDownloadScope[0], strlen(srTMSCPTRec.szTMSDownloadScope));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSDownloadScope
Date&Time       :
Describe        :
*/
int inSetTMSDownloadScope(char* szTMSDownloadScope)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSDownloadScope == NULL || strlen(szTMSDownloadScope) <= 0 || strlen(szTMSDownloadScope) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTMSDownloadScope() ERROR !!");

                        if (szTMSDownloadScope == NULL)
                        {
                                inDISP_LogPrintf("szTMSDownloadScope == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSDownloadScope length = (%d)", (int)strlen(szTMSDownloadScope));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSCPTRec.szTMSDownloadScope, 0x00, sizeof(srTMSCPTRec.szTMSDownloadScope));
        memcpy(&srTMSCPTRec.szTMSDownloadScope[0], &szTMSDownloadScope[0], strlen(szTMSDownloadScope));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSScheduleDate
Date&Time       :
Describe        :
*/
int inGetTMSScheduleDate(char* szTMSScheduleDate)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSScheduleDate == NULL || strlen(srTMSCPTRec.szTMSScheduleDate) <= 0 || strlen(srTMSCPTRec.szTMSScheduleDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTMSScheduleDate() ERROR !!");

                        if (szTMSScheduleDate == NULL)
                        {
                                inDISP_LogPrintf("szTMSScheduleDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSScheduleDate length = (%d)", (int)strlen(srTMSCPTRec.szTMSScheduleDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSScheduleDate[0], &srTMSCPTRec.szTMSScheduleDate[0], strlen(srTMSCPTRec.szTMSScheduleDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSScheduleDate
Date&Time       :
Describe        :
*/
int inSetTMSScheduleDate(char* szTMSScheduleDate)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSScheduleDate == NULL || strlen(szTMSScheduleDate) <= 0 || strlen(szTMSScheduleDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTMSScheduleDate() ERROR !!");

                        if (szTMSScheduleDate == NULL)
                        {
                                inDISP_LogPrintf("szTMSScheduleDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSScheduleDate length = (%d)", (int)strlen(szTMSScheduleDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSCPTRec.szTMSScheduleDate, 0x00, sizeof(srTMSCPTRec.szTMSScheduleDate));
        memcpy(&srTMSCPTRec.szTMSScheduleDate[0], &szTMSScheduleDate[0], strlen(szTMSScheduleDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSScheduleTime
Date&Time       :
Describe        :
*/
int inGetTMSScheduleTime(char* szTMSScheduleTime)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSScheduleTime == NULL || strlen(srTMSCPTRec.szTMSScheduleTime) <= 0 || strlen(srTMSCPTRec.szTMSScheduleTime) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTMSScheduleTime() ERROR !!");

                        if (szTMSScheduleTime == NULL)
                        {
                                inDISP_LogPrintf("szTMSScheduleTime == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSScheduleTime length = (%d)", (int)strlen(srTMSCPTRec.szTMSScheduleTime));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSScheduleTime[0], &srTMSCPTRec.szTMSScheduleTime[0], strlen(srTMSCPTRec.szTMSScheduleTime));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSScheduleTime
Date&Time       :
Describe        :
*/
int inSetTMSScheduleTime(char* szTMSScheduleTime)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSScheduleTime == NULL || strlen(szTMSScheduleTime) <= 0 || strlen(szTMSScheduleTime) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTMSScheduleTime() ERROR !!");

                        if (szTMSScheduleTime == NULL)
                        {
                                inDISP_LogPrintf("szTMSScheduleTime == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSScheduleTime length = (%d)", (int)strlen(szTMSScheduleTime));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSCPTRec.szTMSScheduleTime, 0x00, sizeof(srTMSCPTRec.szTMSScheduleTime));        
        memcpy(&srTMSCPTRec.szTMSScheduleTime[0], &szTMSScheduleTime[0], strlen(szTMSScheduleTime));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSScheduleRetry
Date&Time       :
Describe        :
*/
int inGetTMSScheduleRetry(char* szTMSScheduleRetry)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSScheduleRetry == NULL || strlen(srTMSCPTRec.szTMSScheduleRetry) <= 0 || strlen(srTMSCPTRec.szTMSScheduleRetry) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTMSScheduleRetry() ERROR !!");

                        if (szTMSScheduleRetry == NULL)
                        {
                                inDISP_LogPrintf("szTMSScheduleRetry == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSScheduleRetry length = (%d)", (int)strlen(srTMSCPTRec.szTMSScheduleRetry));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSScheduleRetry[0], &srTMSCPTRec.szTMSScheduleRetry[0], strlen(srTMSCPTRec.szTMSScheduleRetry));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSScheduleRetry
Date&Time       :
Describe        :
*/
int inSetTMSScheduleRetry(char* szTMSScheduleRetry)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSScheduleRetry == NULL || strlen(szTMSScheduleRetry) <= 0 || strlen(szTMSScheduleRetry) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTMSScheduleRetry() ERROR !!");

                        if (szTMSScheduleRetry == NULL)
                        {
                                inDISP_LogPrintf("szTMSScheduleRetry == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSScheduleRetry length = (%d)", (int)strlen(szTMSScheduleRetry));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSCPTRec.szTMSScheduleRetry, 0x00, sizeof(srTMSCPTRec.szTMSScheduleRetry));        
        memcpy(&srTMSCPTRec.szTMSScheduleRetry[0], &szTMSScheduleRetry[0], strlen(szTMSScheduleRetry));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSAPPUpdateStatus
Date&Time       :2017/5/8 下午 2:49
Describe        :
*/
int inGetTMSAPPUpdateStatus(char* szTMSAPPUpdateStatus)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSAPPUpdateStatus == NULL || strlen(srTMSCPTRec.szTMSAPPUpdateStatus) <= 0 || strlen(srTMSCPTRec.szTMSAPPUpdateStatus) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTMSAPPUpdateStatus() ERROR !!");

                        if (szTMSAPPUpdateStatus == NULL)
                        {
                                inDISP_LogPrintf("szTMSAPPUpdateStatus == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSAPPUpdateStatus length = (%d)", (int)strlen(srTMSCPTRec.szTMSAPPUpdateStatus));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSAPPUpdateStatus[0], &srTMSCPTRec.szTMSAPPUpdateStatus[0], strlen(srTMSCPTRec.szTMSAPPUpdateStatus));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSAPPUpdateStatus
Date&Time       :2017/5/8 下午 2:49
Describe        :
*/
int inSetTMSAPPUpdateStatus(char* szTMSAPPUpdateStatus)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSAPPUpdateStatus == NULL || strlen(szTMSAPPUpdateStatus) <= 0 || strlen(szTMSAPPUpdateStatus) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTMSAPPUpdateStatus() ERROR !!");

                        if (szTMSAPPUpdateStatus == NULL)
                        {
                                inDISP_LogPrintf("szTMSAPPUpdateStatus == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSAPPUpdateStatus length = (%d)", (int)strlen(szTMSAPPUpdateStatus));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSCPTRec.szTMSAPPUpdateStatus, 0x00, sizeof(srTMSCPTRec.szTMSAPPUpdateStatus));        
        memcpy(&srTMSCPTRec.szTMSAPPUpdateStatus[0], &szTMSAPPUpdateStatus[0], strlen(szTMSAPPUpdateStatus));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSNII
Date&Time       :2017/7/21 上午 11:24
Describe        :
*/
int inGetTMSNII(char* szTMSNII)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSNII == NULL || strlen(srTMSCPTRec.szTMSNII) <= 0 || strlen(srTMSCPTRec.szTMSNII) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTMSNII() ERROR !!");

                        if (szTMSNII == NULL)
                        {
                                inDISP_LogPrintf("szTMSNII == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSNII length = (%d)", (int)strlen(srTMSCPTRec.szTMSNII));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSNII[0], &srTMSCPTRec.szTMSNII[0], strlen(srTMSCPTRec.szTMSNII));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSNII
Date&Time       :2017/7/21 上午 11:24
Describe        :
*/
int inSetTMSNII(char* szTMSNII)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSNII == NULL || strlen(szTMSNII) <= 0 || strlen(szTMSNII) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTMSNII() ERROR !!");

                        if (szTMSNII == NULL)
                        {
                                inDISP_LogPrintf("szTMSNII == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSNII length = (%d)", (int)strlen(szTMSNII));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSCPTRec.szTMSNII, 0x00, sizeof(srTMSCPTRec.szTMSNII));        
        memcpy(&srTMSCPTRec.szTMSNII[0], &szTMSNII[0], strlen(szTMSNII));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSEffectiveReportBit
Date&Time       :2018/6/5 上午 10:25
Describe        :
*/
int inGetTMSEffectiveReportBit(char* szTMSEffectiveReportBit)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSEffectiveReportBit == NULL || strlen(srTMSCPTRec.szTMSEffectiveReportBit) <= 0 || strlen(srTMSCPTRec.szTMSEffectiveReportBit) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTMSEffectiveReportBit() ERROR !!");

                        if (szTMSEffectiveReportBit == NULL)
                        {
                                inDISP_LogPrintf("szTMSEffectiveReportBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSEffectiveReportBit length = (%d)", (int)strlen(srTMSCPTRec.szTMSEffectiveReportBit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSEffectiveReportBit[0], &srTMSCPTRec.szTMSEffectiveReportBit[0], strlen(srTMSCPTRec.szTMSEffectiveReportBit));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSEffectiveReportBit
Date&Time       :2018/6/5 上午 10:25
Describe        :
*/
int inSetTMSEffectiveReportBit(char* szTMSEffectiveReportBit)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSEffectiveReportBit == NULL || strlen(szTMSEffectiveReportBit) <= 0 || strlen(szTMSEffectiveReportBit) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTMSEffectiveReportBit() ERROR !!");

                        if (szTMSEffectiveReportBit == NULL)
                        {
                                inDISP_LogPrintf("szTMSEffectiveReportBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSEffectiveReportBit length = (%d)", (int)strlen(szTMSEffectiveReportBit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSCPTRec.szTMSEffectiveReportBit, 0x00, sizeof(srTMSCPTRec.szTMSEffectiveReportBit));        
        memcpy(&srTMSCPTRec.szTMSEffectiveReportBit[0], &szTMSEffectiveReportBit[0], strlen(szTMSEffectiveReportBit));

        return (VS_SUCCESS);
}

/*
Function        :inResetTMSCPT_Schedule
Date&Time       :2017/1/19 上午 10:39
Describe        :
*/
int inResetTMSCPT_Schedule()
{
	inLoadTMSCPTRec(0);

	inSetTMSDownloadFlag("0");
	inSetTMSScheduleDate("00000000");
	inSetTMSScheduleTime("000000");
	inSetTMSPhoneNumber("               ");
	inSetTMSScheduleRetry("00");
	inSetTMSAPPUpdateStatus("N");
	inSetTMSEffectiveReportBit("N");
	
	inSaveTMSCPTRec(0);
	
	return (VS_SUCCESS);
}

/*
Function        :inTMSCPT_Edit_TMSCPT_Table
Date&Time       :2017/7/21 上午 11:03
Describe        :
*/
int inTMSCPT_Edit_TMSCPT_Table(void)
{
	TABLE_GET_SET_TABLE TMSCPT_FUNC_TABLE[] =
	{
		{"szTMSIPAddress"		,inGetTMSIPAddress			,inSetTMSIPAddress			},
		{"szTMSPortNum"		,inGetTMSPortNum			,inSetTMSPortNum			},
		{"szTMSPhoneNumber"	,inGetTMSPhoneNumber		,inSetTMSPhoneNumber		},
		{"szTMSEffectiveCloseBatch",inGetTMSEffectiveCloseBatch	,inSetTMSEffectiveCloseBatch	},
		{"szTMSEffectiveDate"		,inGetTMSEffectiveDate		,inSetTMSEffectiveDate		},
		{"szTMSEffectiveTime"		,inGetTMSEffectiveTime		,inSetTMSEffectiveTime		},
		{"szTMSDownloadFlag"	,inGetTMSDownloadFlag		,inSetTMSDownloadFlag		},
		{"szTMSDownloadScope"	,inGetTMSDownloadScope		,inSetTMSDownloadScope		},
		{"szTMSScheduleDate"	,inGetTMSScheduleDate		,inSetTMSScheduleDate		},
		{"szTMSScheduleTime"	,inGetTMSScheduleTime		,inSetTMSScheduleTime		},
		{"szTMSScheduleRetry"	,inGetTMSScheduleRetry		,inSetTMSScheduleRetry		},
		{"szTMSAPPUpdateStatus"	,inGetTMSAPPUpdateStatus	,inSetTMSAPPUpdateStatus	},
		{"szTMSNII"			,inGetTMSNII				,inSetTMSNII				},
		{"szTMSEffectiveReportBit"	,inGetTMSEffectiveReportBit	,inSetTMSEffectiveReportBit	},
		{""},
	};
	int	inRetVal;
	char	szKey;
	
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont_Color("是否更改TMSCPT", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
	
	inLoadTMSCPTRec(0);
	inFunc_Edit_Table_Tag(TMSCPT_FUNC_TABLE);
	inSaveTMSCPTRec(0);
	
	return	(VS_SUCCESS);
}