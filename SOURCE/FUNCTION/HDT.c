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
#include "HDT.h"

static  HDT_REC srHDTRec;	/* construct HDT record */
extern  int     ginDebug;	/* Debug使用 extern */
static int inRealDataCount = _SIZE_HDT_COMMA_0D0A_ ;

/*
Function        :inLoadHDTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀HDT檔案，inHDTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadHDTRec(int inHDTRec)
{
	unsigned long   ulFile_Handle;          		/* File Handle */
	unsigned char   *uszReadData;           		/* 放抓到的record */
	unsigned char   *uszTemp;               		/* 暫存，放整筆HDT檔案 */
	char            szHDTRec[_SIZE_HDT_REC_ + 1];      	/* 暫存, 放各個欄位檔案 */
	char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1]; 	/* debug message */
	long            lnHDTLength = 0;        		/* HDT總長度 */
	long            lnReadLength;           		/* 記錄每次要從HDT.dat讀多長的資料 */
	int             i, k, j = 0, inRec = 0; 		/* inRec記錄讀到第幾筆, i為目前從HDT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
	int             inSearchResult = -1;    		/* 判斷有沒有讀到0x0D 0x0A的Flag */

	/* inLoadHDTRec()_START */
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 判斷傳進來的inHDTRec是否小於零 大於等於零才是正確值(防呆) */
	if (inHDTRec < 0)
	{
		/* debug */
		inDISP_LogPrintfWithFlag("  inHDTRec < 0:(index = %d) ERROR!!", inHDTRec);
		return (VS_ERROR);
	}

        /*
         * open HDT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_HDT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

	/*
	 * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
	 * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
	 */
	lnHDTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_HDT_FILE_NAME_);

	if (lnHDTLength == VS_ERROR)
	{
		/* GetSize失敗 ，關檔 */
		inFILE_Close(&ulFile_Handle);
		return (VS_ERROR);
	}

	/*
	 * allocate 記憶體
	 * allocate時多分配一個byte以防萬一（ex:換行符號）
	 */
	uszReadData = malloc(lnHDTLength + 1);
	uszTemp = malloc(lnHDTLength + 1);
	/* 初始化 uszTemp uszReadData */
	memset(uszReadData, 0x00, lnHDTLength + 1);
	memset(uszTemp, 0x00, lnHDTLength + 1);

	/* seek 到檔案開頭 & 從檔案開頭開始read */
	if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
	{
		lnReadLength = lnHDTLength;

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
	 *i為目前從HDT讀到的第幾個字元
	 *j為該record的長度
	 */
	j = 0;
	for (i = 0; i <= lnHDTLength; ++i)      /* "<="是為了抓到最後一個0x00 */
	{
		if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
		{
			/* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
			inSearchResult = 1;

			/* 清空uszReadData */
			memset(uszReadData, 0x00, lnHDTLength + 1);
			/* 把record從temp指定的位置截取出來放到uszReadData */
			memcpy(&uszReadData[0], &uszTemp[i-j], j);
			inRec++;
			/* 因為inHDT_Rec的index從0開始，所以inHDT_Rec要+1 */
			if (inRec == (inHDTRec + 1))
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
	 * 如果沒有inHDTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
	 * 關檔、釋放記憶體並return VS_ERROR
	 * 如果總record數量小於要存取Record的Index
	 * 特例：有可能會遇到全文都沒有0x0D 0x0A
	 */
	if (inRec < (inHDTRec + 1) || inSearchResult == -1)
	{
		inDISP_LogPrintfWithFlag("  No data or Index ERROR");

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
		inDISP_LogPrintfWithFlag("No specific data.");

		/* 關檔 */
		inFILE_Close(&ulFile_Handle);

		/* Free pointer */
		free(uszReadData);
		free(uszTemp);

		return (VS_ERROR);
	}

	/* 結構初始化 */
	memset(&srHDTRec, 0x00, sizeof(srHDTRec));
	/*
	 * 以下pattern為存入HDT_Rec
	 * i為HDT的第幾個字元
	 * 存入HDT_Rec
	 */
	i = 0;


	/* 01_主機索引 */
	/* 初始化 */
	memset(szHDTRec, 0x00, sizeof(szHDTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szHDTRec[k ++] = uszReadData[i ++];
		if (szHDTRec[k - 1] == 0x2C	||
			szHDTRec[k - 1] == 0x0D	||
			szHDTRec[k - 1] == 0x0A	||
			szHDTRec[k - 1] == 0x00)
		{
				break;
		}

		if (i > lnHDTLength)
		{
			inDISP_LogPrintfWithFlag("  HDT unpack ERROR");

			/* 關檔 */
			inFILE_Close(&ulFile_Handle);

			/* Free pointer */
			free(uszReadData);
			free(uszTemp);

			return (VS_ERROR);
		}
	}

	/*  該筆有資料 */
	if (szHDTRec[0] != 0x2C	&&
	szHDTRec[0] != 0x0D	&&
	szHDTRec[0] != 0x0A	&&
	szHDTRec[0] != 0x00)
	{
		memcpy(&srHDTRec.szHostIndex[0], &szHDTRec[0], k - 1);
	}

	/* 02_主機功能是否開啟 */
	/* 初始化 */
	memset(szHDTRec, 0x00, sizeof(szHDTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szHDTRec[k ++] = uszReadData[i ++];
		
		if (szHDTRec[k - 1] == 0x2C	||
			szHDTRec[k - 1] == 0x0D	||
			szHDTRec[k - 1] == 0x0A	||
			szHDTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnHDTLength)
		{
			inDISP_LogPrintfWithFlag("  HDT unpack ERROR");

			/* 關檔 */
			inFILE_Close(&ulFile_Handle);

			/* Free pointer */
			free(uszReadData);
			free(uszTemp);

			return (VS_ERROR);
		}
	}

	/*  該筆有資料 */
	if (szHDTRec[0] != 0x2C	&&
		szHDTRec[0] != 0x0D	&&
		szHDTRec[0] != 0x0A	&&
		szHDTRec[0] != 0x00)
	{
		memcpy(&srHDTRec.szHostEnable[0], &szHDTRec[0], k - 1);
	}

        /* 03_主機名稱 */
	/* 初始化 */
        memset(szHDTRec, 0x00, sizeof(szHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDTRec[k ++] = uszReadData[i ++];
                if (szHDTRec[k - 1] == 0x2C	||
		    szHDTRec[k - 1] == 0x0D	||
		    szHDTRec[k - 1] == 0x0A	||
		    szHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("HDT unpack ERROR");
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
        if (szHDTRec[0] != 0x2C	&&
	    szHDTRec[0] != 0x0D	&&
	    szHDTRec[0] != 0x0A	&&
	    szHDTRec[0] != 0x00)
        {
                memcpy(&srHDTRec.szHostLabel[0], &szHDTRec[0], k - 1);
        }

        /* 04_商店代號 */
        /* 初始化 */
        memset(szHDTRec, 0x00, sizeof(szHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDTRec[k ++] = uszReadData[i ++];
                if (szHDTRec[k - 1] == 0x2C	||
		    szHDTRec[k - 1] == 0x0D	||
		    szHDTRec[k - 1] == 0x0A	||
		    szHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("HDT unpack ERROR");
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
        if (szHDTRec[0] != 0x2C	&&
	    szHDTRec[0] != 0x0D	&&
	    szHDTRec[0] != 0x0A	&&
	    szHDTRec[0] != 0x00)
        {
                memcpy(&srHDTRec.szMerchantID[0], &szHDTRec[0], k - 1);
        }

        /* 05_端末機代號 */
        /* 初始化 */
        memset(szHDTRec, 0x00, sizeof(szHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDTRec[k ++] = uszReadData[i ++];
                if (szHDTRec[k - 1] == 0x2C	||
		    szHDTRec[k - 1] == 0x0D	||
		    szHDTRec[k - 1] == 0x0A	||
		    szHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("HDT unpack ERROR");
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
        if (szHDTRec[0] != 0x2C	&&
	    szHDTRec[0] != 0x0D	&&
	    szHDTRec[0] != 0x0A	&&
	    szHDTRec[0] != 0x00)
        {
                memcpy(&srHDTRec.szTerminalID[0], &szHDTRec[0], k - 1);
        }

        /* 06_對應通訊參數索引 */
        /* 初始化 */
        memset(szHDTRec, 0x00, sizeof(szHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDTRec[k ++] = uszReadData[i ++];
                if (szHDTRec[k - 1] == 0x2C	||
		    szHDTRec[k - 1] == 0x0D	||
		    szHDTRec[k - 1] == 0x0A	||
		    szHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("HDT unpack ERROR");
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
        if (szHDTRec[0] != 0x2C	&&
	    szHDTRec[0] != 0x0D	&&
	    szHDTRec[0] != 0x0A	&&
	    szHDTRec[0] != 0x00)
        {
                memcpy(&srHDTRec.szCommunicationIndex[0], &szHDTRec[0], k - 1);
        }

        /* 07_對應通訊參數索引(Nexsys) */
        /* 初始化 */
        memset(szHDTRec, 0x00, sizeof(szHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDTRec[k ++] = uszReadData[i ++];
                if (szHDTRec[k - 1] == 0x2C	||
		    szHDTRec[k - 1] == 0x0D	||
		    szHDTRec[k - 1] == 0x0A	||
		    szHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("HDT unpack ERROR");
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
        if (szHDTRec[0] != 0x2C	&&
	    szHDTRec[0] != 0x0D	&&
	    szHDTRec[0] != 0x0A	&&
	    szHDTRec[0] != 0x00)
        {
                memcpy(&srHDTRec.szPasswordIndex[0], &szHDTRec[0], k - 1);
        }
        
        /* 08_交易功能參數 */
        /* 初始化 */
        memset(szHDTRec, 0x00, sizeof(szHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDTRec[k ++] = uszReadData[i ++];
                if (szHDTRec[k - 1] == 0x2C	||
		    szHDTRec[k - 1] == 0x0D	||
		    szHDTRec[k - 1] == 0x0A	||
		    szHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("HDT unpack ERROR");
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
        if (szHDTRec[0] != 0x2C	&&
	    szHDTRec[0] != 0x0D	&&
	    szHDTRec[0] != 0x0A	&&
	    szHDTRec[0] != 0x00)
        {
                memcpy(&srHDTRec.szTransFunc[0], &szHDTRec[0], k - 1);
        }

        /* 09_人工輸入卡號功能 */
        /* 初始化 */
        memset(szHDTRec, 0x00, sizeof(szHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDTRec[k ++] = uszReadData[i ++];
                if (szHDTRec[k - 1] == 0x2C	||
		    szHDTRec[k - 1] == 0x0D	||
		    szHDTRec[k - 1] == 0x0A	||
		    szHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("HDT unpack ERROR");
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
        if (szHDTRec[0] != 0x2C	&&
	    szHDTRec[0] != 0x0D	&&
	    szHDTRec[0] != 0x0A	&&
	    szHDTRec[0] != 0x00)
        {
                memcpy(&srHDTRec.szManualKeyin[0], &szHDTRec[0], k - 1);
        }

        /* 10_Call Bank功能 */
        /* 初始化 */
        memset(szHDTRec, 0x00, sizeof(szHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDTRec[k ++] = uszReadData[i ++];
                if (szHDTRec[k - 1] == 0x2C	||
		    szHDTRec[k - 1] == 0x0D	||
		    szHDTRec[k - 1] == 0x0A	||
		    szHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("HDT unpack ERROR");
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
        if (szHDTRec[0] != 0x2C	&&
	    szHDTRec[0] != 0x0D	&&
	    szHDTRec[0] != 0x0A	&&
	    szHDTRec[0] != 0x00)
        {
                memcpy(&srHDTRec.szCallBankEnable[0], &szHDTRec[0], k - 1);
        }

        /* 11_小費檢合百分比 */
        /* 初始化 */
        memset(szHDTRec, 0x00, sizeof(szHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDTRec[k ++] = uszReadData[i ++];
                if (szHDTRec[k - 1] == 0x2C	||
		    szHDTRec[k - 1] == 0x0D	||
		    szHDTRec[k - 1] == 0x0A	||
		    szHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("HDT unpack ERROR");
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
        if (szHDTRec[0] != 0x2C	&&
	    szHDTRec[0] != 0x0D	&&
	    szHDTRec[0] != 0x0A	&&
	    szHDTRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srHDTRec.szTipPercent[0], &szHDTRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadHDTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadHDTRec(%d) END!!", inHDTRec);
                inDISP_LogPrintf(szErrorMsg);
                inDISP_LogPrintf("----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveHDTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :寫入HDT.dat，inHDTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inSaveHDTRec(int inHDTRec)
{
	unsigned long   uldat_Handle;   			/* FILE Handle */
	int inRecCount = 0, inTempIndex = 0;
	int     	inRetVal;
	int     	i = 0;

	char szTempTitle[] = "inSaveHDTRec";
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
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "HDT_732 inSaveHDTRec INIT" );
#endif
	
	inTempIndex = inHDTRec;
	
	
	memset(szTempFIleName, 0x00, sizeof(szTempFIleName));
	strcat( szTempFIleName, _HDT_FILE_NAME_);
	
	memset(szTempBakFileName, 0x00, sizeof(szTempBakFileName));
	strcat( szTempBakFileName, _HDT_FILE_NAME_BAK_);
	
	inDISP_LogPrintfWithFlag("  %s Index[%d] ", szTempTitle, inTempIndex);
	
	/* 開啟原檔案HDPT.dat */
	inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_HDT_FILE_NAME_);

	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  %s *Error* [%d] ",szTempTitle, inRetVal);
		inFunc_EDCLock();
		return(VS_ERROR);
	}

	usRetVal = CTOS_FileGetSize((unsigned char *)_HDT_FILE_NAME_, &ulFileSize);	
	
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
	uszWriteBuff_Record = malloc(_SIZE_HDT_REC_ + _SIZE_HDT_COMMA_0D0A_);
	memset(uszWriteBuff_Record, 0x00, _SIZE_HDT_REC_ + _SIZE_HDT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

	/* uszRead_Total_Buff儲存HDT.dat全部資料 */
	uszRead_Total_Buff = malloc(ulFileSize + 1);
	memset(uszRead_Total_Buff, 0x00, ulFileSize + 1);

	ulPackCount = 0; /* uszWriteBuff_Record的index位置 */
	/* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* HostIndex */
	memcpy(&uszWriteBuff_Record[0], &srHDTRec.szHostIndex[0], strlen(srHDTRec.szHostIndex));
	ulPackCount += strlen(srHDTRec.szHostIndex);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* HostEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srHDTRec.szHostEnable[0], strlen(srHDTRec.szHostEnable));
	ulPackCount += strlen(srHDTRec.szHostEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* HostLabel */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srHDTRec.szHostLabel[0], strlen(srHDTRec.szHostLabel));
	ulPackCount += strlen(srHDTRec.szHostLabel);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* MerchantID */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srHDTRec.szMerchantID[0], strlen(srHDTRec.szMerchantID));
	ulPackCount += strlen(srHDTRec.szMerchantID);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TerminalID */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srHDTRec.szTerminalID[0], strlen(srHDTRec.szTerminalID));
	ulPackCount += strlen(srHDTRec.szTerminalID);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* CommunicationIndex */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srHDTRec.szCommunicationIndex[0], strlen(srHDTRec.szCommunicationIndex));
	ulPackCount += strlen(srHDTRec.szCommunicationIndex);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* PasswordIndex(Nexsys) */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srHDTRec.szPasswordIndex[0], strlen(srHDTRec.szPasswordIndex));
	ulPackCount += strlen(srHDTRec.szPasswordIndex);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TransFunc */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srHDTRec.szTransFunc[0], strlen(srHDTRec.szTransFunc));
	ulPackCount += strlen(srHDTRec.szTransFunc);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* ManualKeyin */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srHDTRec.szManualKeyin[0], strlen(srHDTRec.szManualKeyin));
	ulPackCount += strlen(srHDTRec.szManualKeyin);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* CallBankEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srHDTRec.szCallBankEnable[0], strlen(srHDTRec.szCallBankEnable));
	ulPackCount += strlen(srHDTRec.szCallBankEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TipPercent */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srHDTRec.szTipPercent[0], strlen(srHDTRec.szTipPercent));
	ulPackCount += strlen(srHDTRec.szTipPercent);

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
	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "HDT_732 inSaveHDTRec END");
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
Function        :inGetHostIndex
Date&Time       :
Describe        :
*/
int inGetHostIndex(char* szHostIndex)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostIndex == NULL || strlen(srHDTRec.szHostIndex) <= 0 || strlen(srHDTRec.szHostIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                	inDISP_LogPrintf("inGetHostIndex() ERROR !!");

			if (szHostIndex == NULL)
			{
				inDISP_LogPrintf("szHostIndex == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szHostIndex length = (%d)", (int)strlen(srHDTRec.szHostIndex));
				inDISP_LogPrintf(szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&szHostIndex[0], &srHDTRec.szHostIndex[0], strlen(srHDTRec.szHostIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostIndex
Date&Time       :
Describe        :
*/
int inSetHostIndex(char* szHostIndex)
{
        memset(srHDTRec.szHostIndex, 0x00, sizeof(srHDTRec.szHostIndex));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostIndex == NULL || strlen(szHostIndex) <= 0 || strlen(szHostIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inSetHostIndex() ERROR !!");

			if (szHostIndex == NULL)
			{
				inDISP_LogPrintf("szHostIndex == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szHostIndex length = (%d)", (int)strlen(szHostIndex));
				inDISP_LogPrintf(szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srHDTRec.szHostIndex[0], &szHostIndex[0], strlen(szHostIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostEnable
Date&Time       :
Describe        :
*/
int inGetHostEnable(char* szHostEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostEnable == NULL || strlen(srHDTRec.szHostEnable) <= 0 || strlen(srHDTRec.szHostEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                	inDISP_LogPrintf("inGetHostEnable() ERROR !!");

                	if (szHostEnable == NULL)
                	{
                	        inDISP_LogPrintf("szHostEnable == NULL");
                	}
                	else
                	{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szHostEnable length = (%d)", (int)strlen(srHDTRec.szHostEnable));
                	        inDISP_LogPrintf(szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szHostEnable[0], &srHDTRec.szHostEnable[0], strlen(srHDTRec.szHostEnable));

        return (VS_SUCCESS);

}

/*
Function        :inSetHostEnable
Date&Time       :
Describe        :
*/
int inSetHostEnable(char* szHostEnable)
{
        memset(srHDTRec.szHostEnable, 0x00, sizeof(srHDTRec.szHostEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostEnable == NULL || strlen(szHostEnable) <= 0 || strlen(szHostEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inSetHostEnable() ERROR !! ");

			if (szHostEnable == NULL)
			{
				inDISP_LogPrintf("szHostEnable == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szHostEnable length = (%d)", (int)strlen(szHostEnable));
				inDISP_LogPrintf(szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srHDTRec.szHostEnable[0], &szHostEnable[0], strlen(szHostEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostLabel
Date&Time       :
Describe        :
*/
int inGetHostLabel(char* szHostLabel)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostLabel == NULL || strlen(srHDTRec.szHostLabel) < 0 || strlen(srHDTRec.szHostLabel) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inGetHostLabel() ERROR !!");

			if (szHostLabel == NULL)
			{
				inDISP_LogPrintf("szHostLabel == NULL");
			}
			else if (strlen(srHDTRec.szHostLabel) < 0 || strlen(srHDTRec.szHostLabel) > 8)
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szHostLabel length = (%d) ", (int)strlen(srHDTRec.szHostLabel));
				inDISP_LogPrintf(szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&szHostLabel[0], &srHDTRec.szHostLabel[0], strlen(srHDTRec.szHostLabel));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostLabel
Date&Time       :
Describe        :
*/
int inSetHostLabel(char* szHostLabel)
{
        memset(srHDTRec.szHostLabel, 0x00, sizeof(srHDTRec.szHostLabel));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostLabel == NULL || strlen(szHostLabel) <= 0 || strlen(szHostLabel) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inSetHostLabel() ERROR !!");

			if (szHostLabel == NULL)
			{
				inDISP_LogPrintf("szHostLabel == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szHostLabel length = (%d)", (int)strlen(szHostLabel));
				inDISP_LogPrintf(szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srHDTRec.szHostLabel[0], &szHostLabel[0], strlen(szHostLabel));

        return (VS_SUCCESS);
}

/*
Function        :inGetMerchantID
Date&Time       :
Describe        :
*/
int inGetMerchantID(char* szMerchantID)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMerchantID == NULL || strlen(srHDTRec.szMerchantID) <= 0 || strlen(srHDTRec.szMerchantID) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inGetMerchantID() ERROR !!");

			if (szMerchantID == NULL)
			{
				inDISP_LogPrintf("szMerchantID == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szMerchantID length = (%d)", (int)strlen(srHDTRec.szMerchantID));
				inDISP_LogPrintf(szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&szMerchantID[0], &srHDTRec.szMerchantID[0], strlen(srHDTRec.szMerchantID));

        return (VS_SUCCESS);
}

/*
Function        :inSetMerchantID
Date&Time       :
Describe        :
*/
int inSetMerchantID(char* szMerchantID)
{
        memset(srHDTRec.szMerchantID, 0x00, sizeof(srHDTRec.szMerchantID));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMerchantID == NULL || strlen(szMerchantID) <= 0 || strlen(szMerchantID) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inSetMerchantID() ERROR !!");

			if (szMerchantID == NULL)
			{
				inDISP_LogPrintf("szMerchantID == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szMerchantID length =  (%d)", (int)strlen(szMerchantID));
				inDISP_LogPrintf(szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srHDTRec.szMerchantID[0], &szMerchantID[0], strlen(szMerchantID));

        return (VS_SUCCESS);
}

/*
Function        :inGetTerminalID
Date&Time       :
Describe        :
*/
int inGetTerminalID(char* szTerminalID)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTerminalID == NULL || strlen(srHDTRec.szTerminalID) <= 0 || strlen(srHDTRec.szTerminalID) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                	inDISP_LogPrintf("inGetTerminalID() ERROR !!");

                	if (szTerminalID == NULL)
                	{
                	        inDISP_LogPrintf("szTerminalID == NULL");
                	}
                	else
                	{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szTerminalID length = (%d)", (int)strlen(srHDTRec.szTerminalID));
                	        inDISP_LogPrintf(szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szTerminalID[0], &srHDTRec.szTerminalID[0], strlen(srHDTRec.szTerminalID));

        return (VS_SUCCESS);
}

/*
Function        :inSetTerminalID
Date&Time       :
Describe        :
*/
int inSetTerminalID(char* szTerminalID)
{
        memset(srHDTRec.szTerminalID, 0x00, sizeof(srHDTRec.szTerminalID));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTerminalID == NULL || strlen(szTerminalID) <= 0 || strlen(szTerminalID) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inSetTerminalID() ERROR !! ");

			if (szTerminalID == NULL)
			{
				inDISP_LogPrintf("szTerminalID == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szTerminalID length = (%d)", (int)strlen(szTerminalID));
				inDISP_LogPrintf(szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srHDTRec.szTerminalID[0], &szTerminalID[0], strlen(szTerminalID));

        return (VS_SUCCESS);
}

/*
Function        :inGetCommunicationIndex
Date&Time       :
Describe        :
*/
int inGetCommunicationIndex(char* szCommunicationIndex)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCommunicationIndex == NULL || strlen(srHDTRec.szCommunicationIndex) < 0 || strlen(srHDTRec.szCommunicationIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                	inDISP_LogPrintf("inGetCommunicationIndex() ERROR !!");

                        if (szCommunicationIndex == NULL)
                        {
				inDISP_LogPrintf("szCommunicationIndex == NULL");
                        }
                        else if (strlen(srHDTRec.szCommunicationIndex) < 0 || strlen(srHDTRec.szCommunicationIndex) > 2)
                        {
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szCommunicationIndex length = (%d) ", (int)strlen(srHDTRec.szCommunicationIndex));
				inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCommunicationIndex[0], &srHDTRec.szCommunicationIndex[0], strlen(srHDTRec.szCommunicationIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetCommunicationIndex
Date&Time       :
Describe        :
*/
int inSetCommunicationIndex(char* szCommunicationIndex)
{
        memset(srHDTRec.szCommunicationIndex, 0x00, sizeof(srHDTRec.szCommunicationIndex));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCommunicationIndex == NULL || strlen(szCommunicationIndex) <= 0 || strlen(szCommunicationIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inSetCommunicationIndex() ERROR !!");

			if (szCommunicationIndex == NULL)
			{
				inDISP_LogPrintf("szCommunicationIndex == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szCommunicationIndex length = (%d)", (int)strlen(szCommunicationIndex));
				inDISP_LogPrintf(szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srHDTRec.szCommunicationIndex[0], &szCommunicationIndex[0], strlen(szCommunicationIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetPasswordIndex
Date&Time       :
Describe        :
*/
int inGetPasswordIndex(char* szPasswordIndex)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPasswordIndex == NULL || strlen(srHDTRec.szPasswordIndex) < 0 || strlen(srHDTRec.szPasswordIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                	inDISP_LogPrintf("inGetPasswordIndex() ERROR !!");

                        if (szPasswordIndex == NULL)
                        {
				inDISP_LogPrintf("szPasswordIndex == NULL");
                        }
                        else if (strlen(srHDTRec.szPasswordIndex) < 0 || strlen(srHDTRec.szPasswordIndex) > 2)
                        {
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szPasswordIndex length = (%d) ", (int)strlen(srHDTRec.szPasswordIndex));
				inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPasswordIndex[0], &srHDTRec.szPasswordIndex[0], strlen(srHDTRec.szPasswordIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetCommunicationIndex
Date&Time       :
Describe        :
*/
int inSetPasswordIndex(char* szPasswordIndex)
{
        memset(srHDTRec.szPasswordIndex, 0x00, sizeof(srHDTRec.szPasswordIndex));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPasswordIndex == NULL || strlen(szPasswordIndex) <= 0 || strlen(szPasswordIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inSetPasswordIndex() ERROR !!");

			if (szPasswordIndex == NULL)
			{
				inDISP_LogPrintf("szPasswordIndex == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szPasswordIndex length = (%d)", (int)strlen(szPasswordIndex));
				inDISP_LogPrintf(szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srHDTRec.szPasswordIndex[0], &szPasswordIndex[0], strlen(szPasswordIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetTransFunc
Date&Time       :
Describe        :
*/
int inGetTransFunc(char* szTransFunc)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTransFunc == NULL || strlen(srHDTRec.szTransFunc) <= 0 || strlen(srHDTRec.szTransFunc) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                	inDISP_LogPrintf("inGetTransFunc() ERROR !!");

                	if (szTransFunc == NULL)
                	{
                	        inDISP_LogPrintf("szTransFunc == NULL");
                	}
                	else
                	{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szTransFunc length = (%d)", (int)strlen(srHDTRec.szTransFunc));
                	        inDISP_LogPrintf(szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szTransFunc[0], &srHDTRec.szTransFunc[0], strlen(srHDTRec.szTransFunc));

        return (VS_SUCCESS);
}

/*
Function        :inSetTransFunc
Date&Time       :
Describe        :
*/
int inSetTransFunc(char* szTransFunc)
{
        memset(srHDTRec.szTransFunc, 0x00, sizeof(srHDTRec.szTransFunc));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTransFunc == NULL || strlen(szTransFunc) <= 0 || strlen(szTransFunc) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inSetTransFunc() ERROR !!");

			if (szTransFunc == NULL)
			{
				inDISP_LogPrintf("szTransFunc == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szTransFunc length = (%d)", (int)strlen(szTransFunc));
				inDISP_LogPrintf(szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srHDTRec.szTransFunc[0], &szTransFunc[0], strlen(szTransFunc));

        return (VS_SUCCESS);
}

/*
Function        :inGetManualKeyin
Date&Time       :
Describe        :
*/
int inGetManualKeyin(char* szManualKeyin)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szManualKeyin == NULL || strlen(srHDTRec.szManualKeyin) <= 0 || strlen(srHDTRec.szManualKeyin) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                	inDISP_LogPrintf("inGetManualKeyin() ERROR !!");

                	if (szManualKeyin == NULL)
                	{
                	        inDISP_LogPrintf("szManualKeyin == NULL");
                	}
                	else
                	{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szManualKeyin length = (%d)", (int)strlen(srHDTRec.szManualKeyin));
                	        inDISP_LogPrintf(szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szManualKeyin[0], &srHDTRec.szManualKeyin[0], strlen(srHDTRec.szManualKeyin));

        return (VS_SUCCESS);
}

/*
Function        :inSetManualKeyin
Date&Time       :
Describe        :
*/
int inSetManualKeyin(char* szManualKeyin)
{
        memset(srHDTRec.szManualKeyin, 0x00, sizeof(srHDTRec.szManualKeyin));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (strlen(szManualKeyin) <= 0 || strlen(szManualKeyin) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inSetManualKeyin() ERROR !!");

			if (szManualKeyin == NULL)
			{
				inDISP_LogPrintf("szTransFunc == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szManualKeyin length = (%d)", (int)strlen(szManualKeyin));
				inDISP_LogPrintf(szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srHDTRec.szManualKeyin[0], &szManualKeyin[0], strlen(szManualKeyin));

        return (VS_SUCCESS);
}

/*
Function        :inGetCallBankEnable
Date&Time       :
Describe        :
*/
int inGetCallBankEnable(char* szCallBankEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCallBankEnable == NULL || strlen(srHDTRec.szCallBankEnable) <= 0 || strlen(srHDTRec.szCallBankEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                	inDISP_LogPrintf("inGetCallBankEnable() ERROR !!");

                	if (szCallBankEnable == NULL)
                	{
                	        inDISP_LogPrintf("szCallBankEnable == NULL");
                	}
                	else
                	{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szCallBankEnable length = (%d)", (int)strlen(srHDTRec.szCallBankEnable));
                	        inDISP_LogPrintf(szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szCallBankEnable[0], &srHDTRec.szCallBankEnable[0], strlen(srHDTRec.szCallBankEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetCallBankEnable
Date&Time       :
Describe        :
*/
int inSetCallBankEnable(char* szCallBankEnable)
{
        memset(srHDTRec.szCallBankEnable, 0x00, sizeof(srHDTRec.szCallBankEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCallBankEnable == NULL || strlen(szCallBankEnable) <= 0 || strlen(szCallBankEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inSetCallBankEnable() ERROR !!");

			if (szCallBankEnable == NULL)
			{
				inDISP_LogPrintf("szCallBankEnable == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szCallBankEnable length = (%d)", (int)strlen(szCallBankEnable));
				inDISP_LogPrintf(szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srHDTRec.szCallBankEnable[0], &szCallBankEnable[0], strlen(szCallBankEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetTipPercent
Date&Time       :
Describe        :
*/
int inGetTipPercent(char* szTipPercent)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTipPercent == NULL || strlen(srHDTRec.szTipPercent) <= 0 || strlen(srHDTRec.szTipPercent) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                	inDISP_LogPrintf("inGetTipPercent() ERROR !!");

                	if (szTipPercent == NULL)
                	{
                		inDISP_LogPrintf("szTipPercent == NULL");
                	}
                	else
                	{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szTipPercent length = (%d)", (int)strlen(srHDTRec.szTipPercent));
                	        inDISP_LogPrintf(szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szTipPercent[0], &srHDTRec.szTipPercent[0], strlen(srHDTRec.szTipPercent));

        return (VS_SUCCESS);
}

/*
Function        :inSetTipPercent
Date&Time       :
Describe        :
*/
int inSetTipPercent(char* szTipPercent)
{
        memset(srHDTRec.szTipPercent, 0x00, sizeof(srHDTRec.szTipPercent));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTipPercent == NULL || strlen(szTipPercent) <= 0 || strlen(szTipPercent) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inSetTipPercent() ERROR !!");

			if (szTipPercent == NULL)
			{
				inDISP_LogPrintf("szTipPercent == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szTipPercent length = (%d)", (int)strlen(szTipPercent));
				inDISP_LogPrintf(szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srHDTRec.szTipPercent[0], &szTipPercent[0], strlen(szTipPercent));

        return (VS_SUCCESS);
}

/*
Function        :inHDT_Edit_HDT_Table
Date&Time       :2017/5/15 下午 4:08
Describe        :
*/
int inHDT_Edit_HDT_Table(void)
{
	TABLE_GET_SET_TABLE HDT_FUNC_TABLE[] =
	{
		{"szHostIndex"			,inGetHostIndex			,inSetHostIndex			},
		{"szHostEnable"			,inGetHostEnable		,inSetHostEnable		},
		{"szHostLabel"			,inGetHostLabel			,inSetHostLabel			},
		{"szMerchantID"			,inGetMerchantID		,inSetMerchantID		},
		{"szTerminalID"			,inGetTerminalID		,inSetTerminalID		},
		{"szCommunicationIndex"	,inGetCommunicationIndex	,inSetCommunicationIndex	},
		{"szPasswordIndex"		,inGetPasswordIndex		,inSetPasswordIndex	},
		{"szTransFunc"			,inGetTransFunc			,inSetTransFunc	},
		{"szManualKeyin"		,inGetManualKeyin		,inSetManualKeyin		},
		{"szCallBankEnable"		,inGetCallBankEnable		,inSetCallBankEnable		},
		{"szTipPercent"			,inGetTipPercent		,inSetTipPercent		},
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
	inDISP_ChineseFont_Color("是否更改HDT", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
	inLoadHDTRec(inRecordCnt);
	
	inFunc_Edit_Table_Tag(HDT_FUNC_TABLE);
	inSaveHDTRec(inRecordCnt);
	
	return	(VS_SUCCESS);
}

int inHDT_Test1(void)
{
//        inLoadHDTRec(0);
//        inSetHostIndex("QQ");
//        inSetHostEnable("Y");
//        inSetHostLabel("TT");
//        inSetMerchantID("Q23456789012345");
//        inSetTerminalID("P2109876");
//        inSetCommunicationIndex("01");
//        inSetPasswordIndex("05");
//        inSetTransFunc("11111111111111111111");
//        inSetManualKeyin("2");
//        inSetCallBankEnable("4");
//        inSetTipPercent("999");
//        inSaveHDTRec(0);
//
//        inLoadHDTRec(5);
//        inSetHostIndex("QQ");
//        inSetHostEnable("Y");
//        inSetHostLabel("TT");
//        inSetMerchantID("Q23456789012345");
//        inSetTerminalID("P2109876");
//        inSetCommunicationIndex("01");
//        inSetPasswordIndex("05");
//        inSetTransFunc("11111111111111111111");
//        inSetManualKeyin("2");
//        inSetCallBankEnable("4");
//        inSetTipPercent("999");
//        inSaveHDTRec(5);


        inLoadHDTRec(17);
        inSetHostIndex("QQ");
        inSetHostEnable("Y");
        inSetHostLabel("TT");
        inSetMerchantID("Q23456789012345");
        inSetTerminalID("P2109876");
        inSetCommunicationIndex("01");
        inSetTransFunc("11111111111111111111");
        inSetManualKeyin("2");
        inSetCallBankEnable("4");
        inSetTipPercent("999");
        inSaveHDTRec(17);

        return (VS_SUCCESS);
}


int inHDT_Test2(void)
{

        if (inLoadHDTRec(0) == VS_SUCCESS)
        {

                inSetHostIndex("02");
                inSaveHDTRec(0);
        }
        if (inLoadHDTRec(1)== VS_SUCCESS)
        {
                inSetHostEnable("Y");
                inSaveHDTRec(1);
        }

        if (inLoadHDTRec(2))
        {
                inSetHostLabel("CTCB");
                inSaveHDTRec(2);
        }
        if (inLoadHDTRec(3))
        {
                inSetMerchantID("123456789012345");
                inSaveHDTRec(3);
        }
        if (inLoadHDTRec(4))
        {
                inSetTerminalID("12345678");
                inSaveHDTRec(4);
        }
        if (inLoadHDTRec(5))
        {
                inSetCommunicationIndex("01");
                inSaveHDTRec(5);
        }
        if (inLoadHDTRec(6))
        {
                inSetTransFunc("ABCDEFGHIJKLMNOPQRST");
                inSaveHDTRec(7);
        }
        if (inLoadHDTRec(7))
        {
                inSetManualKeyin("2");
                inSetCallBankEnable("4");
                inSetTipPercent("200");
                inSaveHDTRec(8);
        }
        return (VS_SUCCESS);
}

int inSetHdtRealDataCount( int inRealCount)
{
	inRealDataCount = inRealCount;
	return inRealDataCount;
}

/*
Function        : inHDT_Edit_Part_Of_HDT_Table
Date&Time   : 2021/1/20 下午 3:22
Describe        : 因為傳入的HOST 已讀取過，所以不需要再重覆讀取，如果要使用在單獨功能上，那需要重讀HOST
*/
int inHDT_Edit_Part_Of_HDT_Table(int inHostId)
{
	TABLE_GET_SET_TABLE HDT_FUNC_TABLE[] =
	{
		{"MerchantID"			,inGetMerchantID		,inSetMerchantID		},
		{"TerminalID"			,inGetTerminalID		,inSetTerminalID		},
		{""},
	};

	inFunc_Edit_Table_Tag(HDT_FUNC_TABLE);
	inSaveHDTRec(inHostId);	
	return  (VS_SUCCESS);
}

