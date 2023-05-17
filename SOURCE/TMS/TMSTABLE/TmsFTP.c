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

#include "TmsFTP.h"

static  TMSFTP_REC	srTMSFTPRec;	/* construct TMSFTP record */
extern  int		ginDebug;       /* Debug使用 extern */
static int ginHasBeenSet = VS_FALSE;

/*
Function        :inLoadTMSFTPRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀TMSFTP檔案，inTMSFTPRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadTMSFTPRec(int inTMSFTPRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆TMSFTP檔案 */
        char            szTMSFTPRec[_SIZE_TMSFTP_REC_ + 1];           /* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnTMSFTPLength = 0;                        /* TMSFTP總長度 */
        long            lnReadLength;                           /* 記錄每次要從TMSFTP.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從TMSFTP讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */
       
        /* inLoadTMSFTPRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadTMSFTPRec(%d) START!!", inTMSFTPRec);
                inDISP_LogPrintf(szErrorMsg);
        }

        /* 判斷傳進來的inTMSFTPRec是否小於零 大於等於零才是正確值(防呆) */
        if (inTMSFTPRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inTMSFTPRec < 0:(index = %d) ERROR!!", inTMSFTPRec);
                        inDISP_LogPrintf(szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open TMSFTP.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_TMSFTP_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnTMSFTPLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_TMSFTP_FILE_NAME_);
        
        if (lnTMSFTPLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnTMSFTPLength + 1);
        uszTemp = malloc(lnTMSFTPLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnTMSFTPLength + 1);
        memset(uszTemp, 0x00, lnTMSFTPLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnTMSFTPLength;

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
         *i為目前從TMSFTP讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnTMSFTPLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到TMSFTP的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnTMSFTPLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inTMSFTP_Rec的index從0開始，所以inTMSFTP_Rec要+1 */
                        if (inRec == (inTMSFTPRec + 1))
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
         * 如果沒有inTMSFTPRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inTMSFTPRec + 1) || inSearchResult == -1)
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
        memset(&srTMSFTPRec, 0x00, sizeof(srTMSFTPRec));
        /*
         * 以下pattern為存入TMSFTP_Rec
         * i為TMSFTP的第幾個字元
         * 存入TMSFTP_Rec
         */
        i = 0;


        /* 01_TMS IP Address */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSFTP unpack ERROR.");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPIPAddress[0], &szTMSFTPRec[0], k - 1);
        }

        /* 02_TMS Port Number */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPPortNum[0], &szTMSFTPRec[0], k - 1);
        }

        /* 03_TMS FTPID */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPID[0], &szTMSFTPRec[0], k - 1);
        }

        /* 04_結帳後更新參數的開關 */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPPW[0], &szTMSFTPRec[0], k - 1);
        }
        
        /* 05_是否允許自動下載 */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPAutoDownloadFlag[0], &szTMSFTPRec[0], k - 1);
        }
        
        /* 06_允許端末機下載的日期(時間) */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srTMSFTPRec.szFTPInquireStartDate[0], &szTMSFTPRec[0], k - 1);
        }

        /* 07_端末機參數異動日期時間 */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPInquireStartTime[0], &szTMSFTPRec[0], k - 1);
        }
        
        /* 08_端末機是否須檢查有無帳務才可更新 */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPEffectiveCloseBatch[0], &szTMSFTPRec[0], k - 1);
        }
        
        /* 09_作業批號 */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPBatchNum[0], &szTMSFTPRec[0], k - 1);
        }
	
	/* 10_回報回應碼 */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPInquiryResponseCode[0], &szTMSFTPRec[0], k - 1);
        }
	
	/* 11_ DownloadResponseCode*/
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPDownloadResponseCode[0], &szTMSFTPRec[0], k - 1);
        }
	
	/* 12_szDownloadCategory */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPDownloadCategory[0], &szTMSFTPRec[0], k - 1);
        }
	
	/* 12_FTPEffectiveReportBit */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPEffectiveReportBit[0], &szTMSFTPRec[0], k - 1);
        }
	
		
	/* 12_FTPApVersionDate */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPApVersionDate[0], &szTMSFTPRec[0], k - 1);
        }		
		
        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadTMSFTPRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadTMSFTPRec(%d) END!!", inTMSFTPRec);
                inDISP_LogPrintf(szErrorMsg);
                inDISP_LogPrintf("----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveTMSFTPRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveTMSFTPRec(int inTMSFTPRec)
{
  	unsigned long   uldat_Handle;   			/* FILE Handle */
	int inRecCount = 0, inTempIndex = 0;
	int     	inRetVal;
	int     	i = 0;

	char szTempTitle[] = "inSaveTMSFTPRec";
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
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inSaveTMSFTPRec INIT" );
#endif

	inTempIndex = inTMSFTPRec;
	
	memset(szTempFIleName, 0x00, sizeof(szTempFIleName));
	strcat( szTempFIleName, _TMSFTP_FILE_NAME_);
	
	memset(szTempBakFileName, 0x00, sizeof(szTempBakFileName));
	strcat( szTempBakFileName, _TMSFTP_FILE_NAME_BAK_);
	
	inDISP_LogPrintfWithFlag("  %s Index[%d] ", szTempTitle, inTempIndex);

      	/* 開啟原檔案HDPT.dat */
	inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_TMSFTP_FILE_NAME_);

	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  %s *Error* [%d] ",szTempTitle, inRetVal);
		inFunc_EDCLock();
		return(VS_ERROR);
	}

	usRetVal = CTOS_FileGetSize((unsigned char *)_TMSFTP_FILE_NAME_, &ulFileSize);	
	
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
	uszWriteBuff_Record = malloc(_SIZE_TMSFTP_REC_ + _SIZE_TMSFTP_COMMA_0D0A_);
	memset(uszWriteBuff_Record, 0x00, _SIZE_TMSFTP_REC_ + _SIZE_TMSFTP_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

	/* uszRead_Total_Buff儲存TMSFTP.dat全部資料 */
	uszRead_Total_Buff = malloc(ulFileSize + 1);
	memset(uszRead_Total_Buff, 0x00, ulFileSize + 1);

	ulPackCount = 0; /* uszWriteBuff_Record的index位置 */
	/* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* FTPIPAddress */
	memcpy(&uszWriteBuff_Record[0], &srTMSFTPRec.szFTPIPAddress[0], strlen(srTMSFTPRec.szFTPIPAddress));
	ulPackCount += strlen(srTMSFTPRec.szFTPIPAddress);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* FTPPortNum */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSFTPRec.szFTPPortNum[0], strlen(srTMSFTPRec.szFTPPortNum));
	ulPackCount += strlen(srTMSFTPRec.szFTPPortNum);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* FTPID */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSFTPRec.szFTPID[0], strlen(srTMSFTPRec.szFTPID));
	ulPackCount += strlen(srTMSFTPRec.szFTPID);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* FTPPW */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSFTPRec.szFTPPW[0], strlen(srTMSFTPRec.szFTPPW));
	ulPackCount += strlen(srTMSFTPRec.szFTPPW);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szFTPAutoDownloadFlag */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSFTPRec.szFTPAutoDownloadFlag[0], strlen(srTMSFTPRec.szFTPAutoDownloadFlag));
	ulPackCount += strlen(srTMSFTPRec.szFTPAutoDownloadFlag);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szFTPStartDownloadDateTime */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSFTPRec.szFTPInquireStartDate[0], strlen(srTMSFTPRec.szFTPInquireStartDate));
	ulPackCount += strlen(srTMSFTPRec.szFTPInquireStartDate);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szFTPTermParemeterDateTime */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSFTPRec.szFTPInquireStartTime[0], strlen(srTMSFTPRec.szFTPInquireStartTime));
	ulPackCount += strlen(srTMSFTPRec.szFTPInquireStartTime);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szFTPEffectiveCloseBatch */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSFTPRec.szFTPEffectiveCloseBatch[0], strlen(srTMSFTPRec.szFTPEffectiveCloseBatch));
	ulPackCount += strlen(srTMSFTPRec.szFTPEffectiveCloseBatch);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szFTPBatchNum */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSFTPRec.szFTPBatchNum[0], strlen(srTMSFTPRec.szFTPBatchNum));
	ulPackCount += strlen(srTMSFTPRec.szFTPBatchNum);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szFTPInquiryResponseCode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSFTPRec.szFTPInquiryResponseCode[0], strlen(srTMSFTPRec.szFTPInquiryResponseCode));
	ulPackCount += strlen(srTMSFTPRec.szFTPInquiryResponseCode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szFTPDownloadResponseCode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSFTPRec.szFTPDownloadResponseCode[0], strlen(srTMSFTPRec.szFTPDownloadResponseCode));
	ulPackCount += strlen(srTMSFTPRec.szFTPDownloadResponseCode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szFTPDownloadCategory */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSFTPRec.szFTPDownloadCategory[0], strlen(srTMSFTPRec.szFTPDownloadCategory));
	ulPackCount += strlen(srTMSFTPRec.szFTPDownloadCategory);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szFTPEffectiveReportBit */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSFTPRec.szFTPEffectiveReportBit[0], strlen(srTMSFTPRec.szFTPEffectiveReportBit));
	ulPackCount += strlen(srTMSFTPRec.szFTPEffectiveReportBit);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;
	
	/* szFTPApVersionDate */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSFTPRec.szFTPApVersionDate[0], strlen(srTMSFTPRec.szFTPApVersionDate));
	ulPackCount += strlen(srTMSFTPRec.szFTPApVersionDate);

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
		     ( uszRead_Total_Buff[i-1] != 0x0A && uszRead_Total_Buff[i] == 0x00))
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
		/* 先刪除檔案 */
		if (inFILE_Delete((unsigned char *)szTempBakFileName) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  %s File Del *Error* Name[%s] Line[%d]", szTempTitle,szTempFIleName , __LINE__ );
			/* Free pointer */
			free(uszRead_Total_Buff);
			free(uszWriteBuff_Record);
			return (VS_ERROR);
		}
		
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
	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inSaveTMSFTPRec END");
#endif

	inDISP_LogPrintfWithFlag( "  End %s [%d] END!!", szTempTitle, inTempIndex );	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	

	return(VS_SUCCESS);
}

/*
set和get等價於相反的操作
各欄位的set和get function
*/

/*
Function        :inGetFTPIPAddress
Date&Time       :
Describe        :
*/
int inGetFTPIPAddress(char* szFTPIPAddress)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPIPAddress == NULL || strlen(srTMSFTPRec.szFTPIPAddress) <= 0 || strlen(srTMSFTPRec.szFTPIPAddress) > 16)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetFTPIPAddress() ERROR !!");
                        
			if (szFTPIPAddress == NULL)
                        {
                                inDISP_LogPrintf("szFTPIPAddress == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPIPAddress length = (%d)", (int)strlen(srTMSFTPRec.szFTPIPAddress));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPIPAddress[0], &srTMSFTPRec.szFTPIPAddress[0], strlen(srTMSFTPRec.szFTPIPAddress));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPIPAddress
Date&Time       :
Describe        :
*/
int inSetFTPIPAddress(char* szFTPIPAddress)
{
        memset(srTMSFTPRec.szFTPIPAddress, 0x00, sizeof(srTMSFTPRec.szFTPIPAddress));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szFTPIPAddress == NULL || strlen(szFTPIPAddress) <= 0 || strlen(szFTPIPAddress) > 16)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetFTPIPAddress() ERROR !!");
                        if (szFTPIPAddress == NULL)
                        {
                                inDISP_LogPrintf("szFTPIPAddress == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPIPAddress length = (%d)", (int)strlen(szFTPIPAddress));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTMSFTPRec.szFTPIPAddress[0], &szFTPIPAddress[0], strlen(szFTPIPAddress));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPPortNum
Date&Time       :
Describe        :
*/
int inGetFTPPortNum(char* szFTPPortNum)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPPortNum == NULL || strlen(srTMSFTPRec.szFTPPortNum) <= 0 || strlen(srTMSFTPRec.szFTPPortNum) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetFTPPortNum() ERROR !!");

                        if (szFTPPortNum == NULL)
                        {
                                inDISP_LogPrintf("szFTPPortNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPPortNum length = (%d)", (int)strlen(srTMSFTPRec.szFTPPortNum));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPPortNum[0], &srTMSFTPRec.szFTPPortNum[0], strlen(srTMSFTPRec.szFTPPortNum));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPPortNum
Date&Time       :
Describe        :
*/
int inSetFTPPortNum(char* szFTPPortNum)
{
        memset(srTMSFTPRec.szFTPPortNum, 0x00, sizeof(srTMSFTPRec.szFTPPortNum));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPPortNum == NULL || strlen(szFTPPortNum) <= 0 || strlen(szFTPPortNum) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetFTPPortNum() ERROR !!");

                        if (szFTPPortNum == NULL)
                        {
                                inDISP_LogPrintf("szFTPPortNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPPortNum length = (%d)", (int)strlen(szFTPPortNum));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTMSFTPRec.szFTPPortNum[0], &szFTPPortNum[0], strlen(szFTPPortNum));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPID
Date&Time       :
Describe        :
*/
int inGetFTPID(char* szFTPID)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPID == NULL || strlen(srTMSFTPRec.szFTPID) <= 0 || strlen(srTMSFTPRec.szFTPID) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetFTPID() ERROR !!");

                        if (szFTPID == NULL)
                        {
                                inDISP_LogPrintf("szFTPID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPID length = (%d)", (int)strlen(srTMSFTPRec.szFTPID));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPID[0], &srTMSFTPRec.szFTPID[0], strlen(srTMSFTPRec.szFTPID));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPID
Date&Time       :
Describe        :
*/
int inSetFTPID(char* szFTPID)
{
        memset(srTMSFTPRec.szFTPID, 0x00, sizeof(srTMSFTPRec.szFTPID));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPID == NULL || strlen(szFTPID) <= 0 || strlen(szFTPID) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetFTPID() ERROR !!");

                        if (szFTPID == NULL)
                        {
                                inDISP_LogPrintf("szFTPID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPID length = (%d)", (int)strlen(szFTPID));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTMSFTPRec.szFTPID[0], &szFTPID[0], strlen(szFTPID));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPPW
Date&Time       :
Describe        :
*/
int inGetFTPPW(char* szFTPPW)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPPW == NULL || strlen(srTMSFTPRec.szFTPPW) <= 0 || strlen(srTMSFTPRec.szFTPPW) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetFTPPW() ERROR !!");

                        if (szFTPPW == NULL)
                        {
                                inDISP_LogPrintf("szFTPPW == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPPW length = (%d)", (int)strlen(srTMSFTPRec.szFTPPW));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPPW[0], &srTMSFTPRec.szFTPPW[0], strlen(srTMSFTPRec.szFTPPW));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPPW
Date&Time       :
Describe        :
*/
int inSetFTPPW(char* szFTPPW)
{
        memset(srTMSFTPRec.szFTPPW, 0x00, sizeof(srTMSFTPRec.szFTPPW));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPPW == NULL || strlen(szFTPPW) <= 0 || strlen(szFTPPW) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetFTPPW() ERROR !!");

                        if (szFTPPW == NULL)
                        {
                                inDISP_LogPrintf("szFTPPW == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPPW length = (%d)", (int)strlen(szFTPPW));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTMSFTPRec.szFTPPW[0], &szFTPPW[0], strlen(szFTPPW));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPAutoDownloadFlag
Date&Time       :2018/6/27 上午 11:22
Describe        :
*/
int inGetFTPAutoDownloadFlag(char* szFTPAutoDownloadFlag)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPAutoDownloadFlag == NULL || strlen(srTMSFTPRec.szFTPAutoDownloadFlag) <= 0 || strlen(srTMSFTPRec.szFTPAutoDownloadFlag) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetFTPAutoDownloadFlag() ERROR !!");

                        if (szFTPAutoDownloadFlag == NULL)
                        {
                                inDISP_LogPrintf("szFTPAutoDownloadFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPAutoDownloadFlag length = (%d)", (int)strlen(srTMSFTPRec.szFTPAutoDownloadFlag));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPAutoDownloadFlag[0], &srTMSFTPRec.szFTPAutoDownloadFlag[0], strlen(srTMSFTPRec.szFTPAutoDownloadFlag));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPAutoDownloadFlag
Date&Time       :2018/6/27 上午 11:22
Describe        :
*/
int inSetFTPAutoDownloadFlag(char* szFTPAutoDownloadFlag)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPAutoDownloadFlag == NULL || strlen(szFTPAutoDownloadFlag) <= 0 || strlen(szFTPAutoDownloadFlag) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetFTPAutoDownloadFlag() ERROR !!");

                        if (szFTPAutoDownloadFlag == NULL)
                        {
                                inDISP_LogPrintf("szFTPAutoDownloadFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPAutoDownloadFlag length = (%d)", (int)strlen(szFTPAutoDownloadFlag));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFTPRec.szFTPAutoDownloadFlag, 0x00, sizeof(srTMSFTPRec.szFTPAutoDownloadFlag));
        memcpy(&srTMSFTPRec.szFTPAutoDownloadFlag[0], &szFTPAutoDownloadFlag[0], strlen(szFTPAutoDownloadFlag));

        return (VS_SUCCESS);
}

/*
Function        : inGetFTPInquireStartDate
Date&Time       :2018/6/27 上午 11:23
Describe        :
*/
int inGetFTPInquireStartDate(char* szFTPInquireStartDate)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPInquireStartDate == NULL || strlen(srTMSFTPRec.szFTPInquireStartDate) <= 0 || strlen(srTMSFTPRec.szFTPInquireStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetFTPInquireStartDate() ERROR !!");

                        if (szFTPInquireStartDate == NULL)
                        {
                                inDISP_LogPrintf("szFTPStartDownloadDateTime == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPStartDownloadDateTime length = (%d)", (int)strlen(srTMSFTPRec.szFTPInquireStartTime));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPInquireStartDate[0], &srTMSFTPRec.szFTPInquireStartDate[0], strlen(srTMSFTPRec.szFTPInquireStartDate));

        return (VS_SUCCESS);
}

/*
Function        : inSetFTPInquireStartDatee
Date&Time       :2018/6/27 上午 11:23
Describe        :
*/
int inSetFTPInquireStartDate(char* szFTPInquireStartDate)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPInquireStartDate == NULL || strlen(szFTPInquireStartDate) <= 0 || strlen(szFTPInquireStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetFTPInquireStartDate() ERROR !!");

                        if (szFTPInquireStartDate == NULL)
                        {
                                inDISP_LogPrintf("szFTPInquireStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPInquireStartDate length = (%d)", (int)strlen(szFTPInquireStartDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFTPRec.szFTPInquireStartDate, 0x00, sizeof(srTMSFTPRec.szFTPInquireStartDate));
        memcpy(&srTMSFTPRec.szFTPInquireStartDate[0], &szFTPInquireStartDate[0], strlen(szFTPInquireStartDate));

        return (VS_SUCCESS);
}

/*
Function        : inGetFTPInquireStartTime
Date&Time       :2018/6/27 上午 11:25
Describe        :
*/
int inGetFTPInquireStartTime(char* szFTPInquireStartTime)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPInquireStartTime == NULL || strlen(srTMSFTPRec.szFTPInquireStartTime) <= 0 || strlen(srTMSFTPRec.szFTPInquireStartTime) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetFTPInquireStartTime() ERROR !!");

                        if (szFTPInquireStartTime == NULL)
                        {
                                inDISP_LogPrintf("szFTPInquireStartTime == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPInquireStartTime length = (%d)", (int)strlen(srTMSFTPRec.szFTPInquireStartTime));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPInquireStartTime[0], &srTMSFTPRec.szFTPInquireStartTime[0], strlen(srTMSFTPRec.szFTPInquireStartTime));

        return (VS_SUCCESS);
}

/*
Function        : inSetFTPInquireStartTime
Date&Time       :2018/6/27 上午 11:25
Describe        :
*/
int inSetFTPInquireStartTime(char* szFTPInquireStartTime)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPInquireStartTime == NULL || strlen(szFTPInquireStartTime) <= 0 || strlen(szFTPInquireStartTime) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetFTPInquireStartTime() ERROR !!");

                        if (szFTPInquireStartTime == NULL)
                        {
                                inDISP_LogPrintf("szFTPInquireStartTime == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPInquireStartTime length = (%d)", (int)strlen(szFTPInquireStartTime));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFTPRec.szFTPInquireStartTime, 0x00, sizeof(srTMSFTPRec.szFTPInquireStartTime));
        memcpy(&srTMSFTPRec.szFTPInquireStartTime[0], &szFTPInquireStartTime[0], strlen(szFTPInquireStartTime));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPEffectiveCloseBatch
Date&Time       :2018/6/27 上午 11:25
Describe        :
*/
int inGetFTPEffectiveCloseBatch(char* szFTPEffectiveCloseBatch)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPEffectiveCloseBatch == NULL || strlen(srTMSFTPRec.szFTPEffectiveCloseBatch) <= 0 || strlen(srTMSFTPRec.szFTPEffectiveCloseBatch) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetFTPEffectiveCloseBatch() ERROR !!");

                        if (szFTPEffectiveCloseBatch == NULL)
                        {
                                inDISP_LogPrintf("szFTPEffectiveCloseBatch == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPEffectiveCloseBatch length = (%d)", (int)strlen(srTMSFTPRec.szFTPEffectiveCloseBatch));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPEffectiveCloseBatch[0], &srTMSFTPRec.szFTPEffectiveCloseBatch[0], strlen(srTMSFTPRec.szFTPEffectiveCloseBatch));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPEffectiveCloseBatch
Date&Time       :2018/6/27 上午 11:25
Describe        :
*/
int inSetFTPEffectiveCloseBatch(char* szFTPEffectiveCloseBatch)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPEffectiveCloseBatch == NULL || strlen(szFTPEffectiveCloseBatch) <= 0 || strlen(szFTPEffectiveCloseBatch) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetFTPEffectiveCloseBatch() ERROR !!");

                        if (szFTPEffectiveCloseBatch == NULL)
                        {
                                inDISP_LogPrintf("szFTPEffectiveCloseBatch == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPEffectiveCloseBatch length = (%d)", (int)strlen(szFTPEffectiveCloseBatch));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFTPRec.szFTPEffectiveCloseBatch, 0x00, sizeof(srTMSFTPRec.szFTPEffectiveCloseBatch));
        memcpy(&srTMSFTPRec.szFTPEffectiveCloseBatch[0], &szFTPEffectiveCloseBatch[0], strlen(szFTPEffectiveCloseBatch));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPBatchNum
Date&Time       :2018/6/27 上午 11:26
Describe        :
*/
int inGetFTPBatchNum(char* szFTPBatchNum)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPBatchNum == NULL || strlen(srTMSFTPRec.szFTPBatchNum) <= 0 || strlen(srTMSFTPRec.szFTPBatchNum) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetFTPBatchNum() ERROR !!");

                        if (szFTPBatchNum == NULL)
                        {
                                inDISP_LogPrintf("szFTPBatchNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPBatchNum length = (%d)", (int)strlen(srTMSFTPRec.szFTPBatchNum));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPBatchNum[0], &srTMSFTPRec.szFTPBatchNum[0], strlen(srTMSFTPRec.szFTPBatchNum));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPBatchNum
Date&Time       :
Describe        :
*/
int inSetFTPBatchNum(char* szFTPBatchNum)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPBatchNum == NULL || strlen(szFTPBatchNum) <= 0 || strlen(szFTPBatchNum) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetFTPBatchNum() ERROR !!");

                        if (szFTPBatchNum == NULL)
                        {
                                inDISP_LogPrintf("szFTPBatchNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPBatchNum length = (%d)", (int)strlen(szFTPBatchNum));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFTPRec.szFTPBatchNum, 0x00, sizeof(srTMSFTPRec.szFTPBatchNum));        
        memcpy(&srTMSFTPRec.szFTPBatchNum[0], &szFTPBatchNum[0], strlen(szFTPBatchNum));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPInquiryResponseCode
Date&Time       :2018/6/27 上午 11:26
Describe        :
*/
int inGetFTPInquiryResponseCode(char* szFTPInquiryResponseCode)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPInquiryResponseCode == NULL || strlen(srTMSFTPRec.szFTPInquiryResponseCode) <= 0 || strlen(srTMSFTPRec.szFTPInquiryResponseCode) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetFTPInquiryResponseCode() ERROR !!");

                        if (szFTPInquiryResponseCode == NULL)
                        {
                                inDISP_LogPrintf("szFTPInquiryResponseCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPInquiryResponseCode length = (%d)", (int)strlen(srTMSFTPRec.szFTPInquiryResponseCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPInquiryResponseCode[0], &srTMSFTPRec.szFTPInquiryResponseCode[0], strlen(srTMSFTPRec.szFTPInquiryResponseCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPInquiryResponseCode
Date&Time       :
Describe        :
*/
int inSetFTPInquiryResponseCode(char* szFTPInquiryResponseCode)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPInquiryResponseCode == NULL || strlen(szFTPInquiryResponseCode) <= 0 || strlen(szFTPInquiryResponseCode) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetFTPInquiryResponseCode() ERROR !!");

                        if (szFTPInquiryResponseCode == NULL)
                        {
                                inDISP_LogPrintf("szFTPInquiryResponseCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPInquiryResponseCode length = (%d)", (int)strlen(szFTPInquiryResponseCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFTPRec.szFTPInquiryResponseCode, 0x00, sizeof(srTMSFTPRec.szFTPInquiryResponseCode));        
        memcpy(&srTMSFTPRec.szFTPInquiryResponseCode[0], &szFTPInquiryResponseCode[0], strlen(szFTPInquiryResponseCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPDownloadResponseCode
Date&Time       :2018/6/27 上午 11:26
Describe        :
*/
int inGetFTPDownloadResponseCode(char* szFTPDownloadResponseCode)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPDownloadResponseCode == NULL || strlen(srTMSFTPRec.szFTPDownloadResponseCode) <= 0 || strlen(srTMSFTPRec.szFTPDownloadResponseCode) > 2)
        {
                
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetFTPDownloadResponseCode() ERROR !!");

                        if (szFTPDownloadResponseCode == NULL)
                        {
                                inDISP_LogPrintf("szFTPDownloadResponseCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPDownloadResponseCode length = (%d)", (int)strlen(srTMSFTPRec.szFTPDownloadResponseCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }

        memcpy(&szFTPDownloadResponseCode[0], &srTMSFTPRec.szFTPDownloadResponseCode[0], strlen(srTMSFTPRec.szFTPDownloadResponseCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPDownloadResponseCode
Date&Time       :
Describe        :
*/
int inSetFTPDownloadResponseCode(char* szFTPDownloadResponseCode)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPDownloadResponseCode == NULL || strlen(szFTPDownloadResponseCode) <= 0 || strlen(szFTPDownloadResponseCode) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetFTPDownloadResponseCode() ERROR !!");

                        if (szFTPDownloadResponseCode == NULL)
                        {
                                inDISP_LogPrintf("szFTPDownloadResponseCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPDownloadResponseCode length = (%d)", (int)strlen(szFTPDownloadResponseCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFTPRec.szFTPDownloadResponseCode, 0x00, sizeof(srTMSFTPRec.szFTPDownloadResponseCode));        
        memcpy(&srTMSFTPRec.szFTPDownloadResponseCode[0], &szFTPDownloadResponseCode[0], strlen(szFTPDownloadResponseCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPDownloadCategory
Date&Time       :2018/6/27 上午 11:26
Describe        :
*/
int inGetFTPDownloadCategory(char* szFTPDownloadCategory)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPDownloadCategory == NULL || strlen(srTMSFTPRec.szFTPDownloadCategory) <= 0 || strlen(srTMSFTPRec.szFTPDownloadCategory) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetFTPDownloadCategory() ERROR !!");

                        if (szFTPDownloadCategory == NULL)
                        {
                                inDISP_LogPrintf("szFTPDownloadCategory == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPDownloadCategory length = (%d)", (int)strlen(srTMSFTPRec.szFTPDownloadCategory));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPDownloadCategory[0], &srTMSFTPRec.szFTPDownloadCategory[0], strlen(srTMSFTPRec.szFTPDownloadCategory));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPDownloadCategory
Date&Time       :
Describe        :
*/
int inSetFTPDownloadCategory(char* szFTPDownloadCategory)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPDownloadCategory == NULL || strlen(szFTPDownloadCategory) <= 0 || strlen(szFTPDownloadCategory) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetFTPDownloadCategory() ERROR !!");

                        if (szFTPDownloadCategory == NULL)
                        {
                                inDISP_LogPrintf("szFTPDownloadCategory == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPDownloadCategory length = (%d)", (int)strlen(szFTPDownloadCategory));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFTPRec.szFTPDownloadCategory, 0x00, sizeof(srTMSFTPRec.szFTPDownloadCategory));        
        memcpy(&srTMSFTPRec.szFTPDownloadCategory[0], &szFTPDownloadCategory[0], strlen(szFTPDownloadCategory));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPEffectiveReportBit
Date&Time       :2018/6/27 上午 11:26
Describe        :
*/
int inGetFTPEffectiveReportBit(char* szFTPEffectiveReportBit)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPEffectiveReportBit == NULL || strlen(srTMSFTPRec.szFTPEffectiveReportBit) <= 0 || strlen(srTMSFTPRec.szFTPEffectiveReportBit) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetFTPEffectiveReportBit() ERROR !!");

                        if (szFTPEffectiveReportBit == NULL)
                        {
                                inDISP_LogPrintf("szFTPEffectiveReportBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPEffectiveReportBit length = (%d)", (int)strlen(srTMSFTPRec.szFTPEffectiveReportBit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPEffectiveReportBit[0], &srTMSFTPRec.szFTPEffectiveReportBit[0], strlen(srTMSFTPRec.szFTPEffectiveReportBit));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPEffectiveReportBit
Date&Time       :
Describe        :
*/
int inSetFTPEffectiveReportBit(char* szFTPEffectiveReportBit)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPEffectiveReportBit == NULL || strlen(szFTPEffectiveReportBit) <= 0 || strlen(szFTPEffectiveReportBit) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetFTPEffectiveReportBit() ERROR !!");

                        if (szFTPEffectiveReportBit == NULL)
                        {
                                inDISP_LogPrintf("szFTPEffectiveReportBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPEffectiveReportBit length = (%d)", (int)strlen(szFTPEffectiveReportBit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFTPRec.szFTPEffectiveReportBit, 0x00, sizeof(srTMSFTPRec.szFTPEffectiveReportBit));        
        memcpy(&srTMSFTPRec.szFTPEffectiveReportBit[0], &szFTPEffectiveReportBit[0], strlen(szFTPEffectiveReportBit));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPEffectiveReportBit
Date&Time       :2018/6/27 上午 11:26
Describe        :
*/
int inGetFTPApVersionDate(char* szFTPApVersionDate)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPApVersionDate == NULL || strlen(srTMSFTPRec.szFTPApVersionDate) <= 0 || strlen(srTMSFTPRec.szFTPApVersionDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetFTPApVersionDate() ERROR !!");

                        if (szFTPApVersionDate == NULL)
                        {
                                inDISP_LogPrintf("szFTPApVersionDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPApVersionDate length = (%d)", (int)strlen(srTMSFTPRec.szFTPApVersionDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPApVersionDate[0], &srTMSFTPRec.szFTPApVersionDate[0], strlen(srTMSFTPRec.szFTPApVersionDate));

        return (VS_SUCCESS);
}

/*
Function        : inSetFTPApVersionDate
Date&Time   :
Describe        :
*/
int inSetFTPApVersionDate(char* szFTPApVersionDate)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPApVersionDate == NULL || strlen(szFTPApVersionDate) <= 0 || strlen(szFTPApVersionDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetFTPApVersionDate() ERROR !!");

                        if (szFTPApVersionDate == NULL)
                        {
                                inDISP_LogPrintf("szFTPApVersionDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPApVersionDate length = (%d)", (int)strlen(szFTPApVersionDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFTPRec.szFTPApVersionDate, 0x00, sizeof(srTMSFTPRec.szFTPApVersionDate));        
        memcpy(&srTMSFTPRec.szFTPApVersionDate[0], &szFTPApVersionDate[0], strlen(szFTPApVersionDate));

        return (VS_SUCCESS);
}

/*
Function        :inTMSFTP_Edit_TMSFTP_Table
Date&Time       :2017/7/21 上午 11:03
Describe        :
*/
int inTMSFTP_Edit_TMSFTP_Table(void)
{
	TABLE_GET_SET_TABLE TMSFTP_FUNC_TABLE[] =
	{
		{"szFTPIPAddress"		,inGetFTPIPAddress		,inSetFTPIPAddress		},
		{"szFTPPortNum"			,inGetFTPPortNum		,inSetFTPPortNum		},
		{"szFTPID"				,inGetFTPID			,inSetFTPID			},
		{"szFTPPW"				,inGetFTPPW			,inSetFTPPW			},
		{"szFTPAutoDownloadFlag"	,inGetFTPAutoDownloadFlag		,inSetFTPAutoDownloadFlag	},
		{"szFTPInquireStartDate"	,inGetFTPInquireStartDate	,inSetFTPInquireStartDate	},
		{"szFTPInquireStartTime"	,inGetFTPInquireStartTime	,inSetFTPInquireStartTime	},
		{"szFTPEffectiveCloseBatch"	,inGetFTPEffectiveCloseBatch	,inSetFTPEffectiveCloseBatch	},
		{"szFTPBatchNum"		,inGetFTPBatchNum		,inSetFTPBatchNum		},
		{"szFTPInquiryResponseCode"	,inGetFTPInquiryResponseCode	,inSetFTPInquiryResponseCode	},
		{"szFTPDownloadResponseCode"	,inGetFTPDownloadResponseCode	,inSetFTPDownloadResponseCode	},
		{"szFTPDownloadCategory"	,inGetFTPDownloadCategory	,inSetFTPDownloadCategory	},
		{"szFTPEffectiveReportBit"	,inGetFTPEffectiveReportBit	,inSetFTPEffectiveReportBit	},
		{"szFTPApVersionDate"	,	inGetFTPApVersionDate	,inSetFTPApVersionDate	},
		{""},
	};
	int	inRetVal;
	char	szKey;
	
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont_Color("是否更改TMSFTP", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
	
	inSaveTMSFTPRec(0);
	inFunc_Edit_Table_Tag(TMSFTP_FUNC_TABLE);
	inSaveTMSFTPRec(0);
	
	return	(VS_SUCCESS);
}

/*
Function        : inResetTMSFTP_Schedule
Date&Time   : 2019/12/8 下午 2:38
Describe        : 初始化檔案資料
*/
int inResetTMSFTP_Schedule()
{
	inLoadTMSFTPRec(0);

	inSetFTPEffectiveReportBit("N");
	inSetFTPEffectiveCloseBatch("N");
	
	inSaveTMSFTPRec(0);
	
	return (VS_SUCCESS);
}

int inTMSFTP_SetDowloadResponseCode(char * szRespCode)
{	
	if(ginHasBeenSet != VS_TRUE)
	{
		ginHasBeenSet = VS_TRUE;
		inSetFTPDownloadResponseCode(szRespCode);
	}
	return VS_SUCCESS;
}

int inTMSFTP_OffFileOpationFlag()
{
	ginHasBeenSet = VS_FALSE;
	return VS_SUCCESS;
}


