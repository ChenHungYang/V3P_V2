#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>
#include <unistd.h>

#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "FILE_FUNC/File.h"
#include "FILE_FUNC/FIleLogFunc.h"
#include "Function.h"
#include "Sqlite.h"

#include "EDC_Para_Table_Func.h"

#include "HDPT.h"
#include "../FUNCTION/UNIT_FUNC/TimeUint.h"

/* 讀成功會設定為讀取的ID,只要有寫入成功才會變動 */
static int inHDPT_TempRecordIndex = -1;
static int inHDPT_HadBeenSave = VS_TRUE;

static  HDPT_REC srHDPTRec;	/* construct HDPT record */
extern  int     ginDebug;  	/* Debug使用 extern */


extern	int	ginFindRunTime;
extern	char	gszParamDBPath[];




SQLITE_TAG_TABLE TABLE_HDPT_TAG[] = 
{
	{"inTableID"			,"INTEGER"	,"PRIMARY KEY"	,""},	/* Table ID Primary key, sqlite table專用避免PRIMARY KEY重複 */
	{"szInvoiceNum"			,"BLOB"		,""		,""},	/* Invoice Number */
	{"szBatchNum"			,"BLOB"		,""		,""},	/* Batch Number */
	{"szSTANNum"			,"BLOB"		,""		,""},	/* STAN Number */
	{"szReversalCnt"			,"BLOB"		,""		,""},	/* Reversal Count */
	{"szTRTFileName"		,"BLOB"		,""		,""},	/* szTRTFileName */
	{"szMustSettleBit"		,"BLOB"		,""		,""},	/* 是否結帳的flag，預設值為N */
	{"szSendReversalBit"		,"BLOB"		,""		,""},	/* 預設值為N */
	{"szCLS_SettleBit"		,"BLOB"		,""		,""},	/* 看是否要續傳批次 */
	{"szTicket_InvNum"		,"BLOB"		,""		,""},	/* 紀錄電票的invoiceNum，簽單的調閱編號用szInvoiceNum的 */
	{"szBatchNumLimit"		,"BLOB"		,""		,""},	/* 單筆Batch的最大數量 */
	{"szMustISRUploadEnableBit"	,"BLOB"		,""		,""},	/* 用來判斷ISR是否需要上傳 */	
	{""},
};



#ifdef _MODIFY_HDPT_FUNC_
/*
Function        :inLoadHDPTRec
Date&Time       :2019/4/24 下午 2:06
Describe        :優化存參數部份
*/
int inLoadHDPTRec(int inHDPTRec)
{
	int	inRetVal = VS_ERROR;
	
	/* 如果沒有覆寫回資料，同個檔案不用重讀 */
	//if(inHDPT_TempRecordIndex == inHDPTRec || inHDPT_HadBeenSave == VS_TRUE)
	if(inHDPT_TempRecordIndex == inHDPTRec && inHDPT_HadBeenSave != VS_TRUE)
	{	
		inDISP_DispLogAndWriteFlie(" Load HDPT *Error* TempId[%d] InputId[%d] BeenSave[%d] Line[%d]",inHDPT_TempRecordIndex,inHDPTRec ,inHDPT_HadBeenSave,__LINE__ );
		return VS_SUCCESS;
	}

	/* 加速用 */
	if (_PARAMETER_SAVE_WAY_DAT_ == _PARAMETER_SAVE_WAY_DAT_SQLITE_)
	{
		inRetVal = inLoadHDPTRec_SQLite(inHDPTRec);
	}
	else
	{
		inRetVal = inLoadHDPTRec_CTOS(inHDPTRec);
	}
	
	return (inRetVal);
}

/*
Function        :inLoadHDPTRec_SQLite
Date&Time       :2019/4/24 下午 2:11
Describe        :讀HDPT檔案，inHDPT_Rec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadHDPTRec_SQLite(int inHDPTRec)
{
	int	inRetVal = VS_ERROR;
	SQLITE_ALL_TABLE srAll = {0};

        /* inLoadHDPTRec_SQLite()_START */
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" HDPT Index(%d)  ", inHDPTRec);


	/* 判斷傳進來的inHDPTRec是否小於零 大於等於零才是正確值(防呆) */
	if (inHDPTRec < 0)
	{
		inDISP_LogPrintfWithFlag(" inHDPTRec < 0:(index = %d) ERROR!!", inHDPTRec);
		return (VS_ERROR);
	}
	
	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inHDPT_Table_Link_HDPTRec(&srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_LockEdcDispLogAndWriteFlie("Table Link 失敗");
		return (VS_ERROR);
	}
	
	inRetVal = inSqlite_Get_Table_ByRecordID_All(gszParamDBPath, _HDPT_TABLE_NAME_, inHDPTRec, &srAll);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_LockEdcDispLogAndWriteFlie("db:%s,table:%s,Rec:%d get data failed", gszParamDBPath, _HDPT_TABLE_NAME_, inHDPTRec);
		return (VS_ERROR);
	}
	
	inHDPT_TempRecordIndex = inHDPTRec;
	inHDPT_HadBeenSave = VS_FALSE;

        /* inLoadHDPTRec_SQLite() END */
	
	inDISP_LogPrintfWithFlag(" inLoadHDPTRec_SQLite(%d) ", inHDPTRec);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
        return (VS_SUCCESS);
}

#endif

/*
Function        : inLoadHDPTRec_CTOS
Date&Time   : 2015/8/31 下午 2:00
Describe        : 讀HDPT檔案，inHDPT_Rec是要讀哪一筆的紀錄，第一筆為0
*/
#ifdef _MODIFY_HDPT_FUNC_
int inLoadHDPTRec_CTOS(int inHDPTRec)
#else
int inLoadHDPTRec(int inHDPTRec)
#endif
{
	unsigned long   ulFile_Handle;		/* File Handle */
	unsigned char   *uszReadData;		/* 放抓到的record */
	unsigned char   *uszTemp;			/* 暫存，放整筆HDPT檔案 */
	char            szHDPTRec[_SIZE_HDPT_REC_ + 1];		/* 暫存, 放各個欄位檔案 */
	long            lnHDPTLength = 0;      	/* HDPT總長度 */
	long            lnReadLength;           		/* 記錄每次要從HDPT.dat讀多長的資料 */
	int             i, k, j = 0, inRec = 0; 		/* inRec記錄讀到第幾筆, i為目前從HDPT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
	int             inSearchResult = -1;    		/* 判斷有沒有讀到0x0D 0x0A的Flag */

	/* inLoadHDPTRec()_START */
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 用區域的時間計算 */
//	unsigned long ulRunTime;
//	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "HDPT_27 inLoadHDPTRec INIT" );
	
	inDISP_LogPrintfWithFlag(" inHDPTRec(%d) ", inHDPTRec);
        

	/* 判斷傳進來的inHDPTRec是否小於零 大於等於零才是正確值(防呆) */
	if (inHDPTRec < 0)
	{
		/* debug */
		inDISP_LogPrintfWithFlag(" inHDPTRec < 0:(index = %d) ERROR!!", inHDPTRec);
		return (VS_ERROR);
	}

	/*
	 * open HDPT.dat file
	 * open失敗時，if條件成立，關檔並回傳VS_ERROR
	 */
	if (inFILE_Open(&ulFile_Handle, (unsigned char *)_HDPT_FILE_NAME_) == VS_ERROR)
	{
		/* 開檔失敗 ，不用關檔 */
		/* 開檔失敗，所以回傳error */
		return (VS_ERROR);
	}

	/*
	 * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
	 * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
	 */
	lnHDPTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_HDPT_FILE_NAME_);

	if (lnHDPTLength == VS_ERROR)
	{
		/* GetSize失敗 ，關檔 */
		inFILE_Close(&ulFile_Handle);
		return (VS_ERROR);
	}

	/*
	 * allocate 記憶體
	 * allocate時多分配一個byte以防萬一（ex:換行符號）
	 */
	uszReadData = malloc(lnHDPTLength + 1);
	uszTemp = malloc(lnHDPTLength + 1);
	/* 初始化 uszTemp uszReadData */
	memset(uszReadData, 0x00, lnHDPTLength + 1);
	memset(uszTemp, 0x00, lnHDPTLength + 1);

	/* seek 到檔案開頭 & 從檔案開頭開始read */
	if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
	{
		lnReadLength = lnHDPTLength;

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
	 *i為目前從HDPT讀到的第幾個字元
	 *j為該record的長度
	 */
	j = 0;
	for (i = 0; i <= lnHDPTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
	{
		/* 讀完一筆record或讀到HDPT的結尾時  */
		if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
		{
			/* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
			inSearchResult = 1;

			/* 清空uszReadData */
			memset(uszReadData, 0x00, lnHDPTLength + 1);
			/* 把record從temp指定的位置截取出來放到uszReadData */
			memcpy(&uszReadData[0], &uszTemp[i-j], j);
			inRec++;
			/* 因為inHDPT_Rec的index從0開始，所以inHDPT_Rec要+1 */
			if (inRec == (inHDPTRec + 1))
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
	 * 如果沒有inHDPTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
	 * 關檔、釋放記憶體並return VS_ERROR
	 * 如果總record數量小於要存取Record的Index
	 * 特例：有可能會遇到全文都沒有0x0D 0x0A
	 */
	if (inRec < (inHDPTRec + 1) || inSearchResult == -1)
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
		inDISP_LogPrintfWithFlag("  No specific data.");

		/* 關檔 */
		inFILE_Close(&ulFile_Handle);

		/* Free pointer */
		free(uszReadData);
		free(uszTemp);

		return (VS_ERROR);
	}

	/* 結構初始化 */
	memset(&srHDPTRec, 0x00, sizeof(srHDPTRec));
	/*
	 * 以下pattern為存入HDPT_Rec
	 * i為HDPT的第幾個字元
	 * 存入HDPT_Rec
	 */
	i = 0;


	/* 01_Invoice Number */
	/* 初始化 */
	memset(szHDPTRec, 0x00, sizeof(szHDPTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szHDPTRec[k++] = uszReadData[i++];
		if (szHDPTRec[k - 1] == 0x2C	||
			szHDPTRec[k - 1] == 0x0D	||
			szHDPTRec[k - 1] == 0x0A	||
			szHDPTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnHDPTLength)
		{
			inDISP_LogPrintfWithFlag("  HDPT unpack ERROR");

			/* 關檔 */
			inFILE_Close(&ulFile_Handle);

			/* Free pointer */
			free(uszReadData);
			free(uszTemp);

			return (VS_ERROR);
		}
	}

	/*  該筆有資料 */
	if (szHDPTRec[0] != 0x2C	&&
		szHDPTRec[0] != 0x0D	&&
		szHDPTRec[0] != 0x0A	&&
		szHDPTRec[0] != 0x00)
	{
		memcpy(&srHDPTRec.szInvoiceNum[0], &szHDPTRec[0], k - 1);
	}

        /* 02_Batch Number */
        /* 初始化 */
        memset(szHDPTRec, 0x00, sizeof(szHDPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDPTRec[k++] = uszReadData[i++];
                if (szHDPTRec[k - 1] == 0x2C	||
		    szHDPTRec[k - 1] == 0x0D	||
		    szHDPTRec[k - 1] == 0x0A	||
		    szHDPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("HDPT unpack ERROR");
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
        if (szHDPTRec[0] != 0x2C	&&
	    szHDPTRec[0] != 0x0D	&&
	    szHDPTRec[0] != 0x0A	&&
	    szHDPTRec[0] != 0x00)
        {
                memcpy(&srHDPTRec.szBatchNum[0], &szHDPTRec[0], k - 1);
        }

        /* 03_STAN Number */
        /* 初始化 */
        memset(szHDPTRec, 0x00, sizeof(szHDPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDPTRec[k++] = uszReadData[i++];
                if (szHDPTRec[k - 1] == 0x2C	||
		    szHDPTRec[k - 1] == 0x0D	||
		    szHDPTRec[k - 1] == 0x0A	||
		    szHDPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("HDPT unpack ERROR");
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
        if (szHDPTRec[0] != 0x2C	&&
	    szHDPTRec[0] != 0x0D	&&
	    szHDPTRec[0] != 0x0A	&&
	    szHDPTRec[0] != 0x00)
        {
                memcpy(&srHDPTRec.szSTANNum[0], &szHDPTRec[0], k - 1);
        }

        /* 04_Reversal Count */
        /* 初始化 */
        memset(szHDPTRec, 0x00, sizeof(szHDPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDPTRec[k++] = uszReadData[i++];
                if (szHDPTRec[k - 1] == 0x2C	||
		    szHDPTRec[k - 1] == 0x0D	||
		    szHDPTRec[k - 1] == 0x0A	||
		    szHDPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("HDPT unpack ERROR");
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
        if (szHDPTRec[0] != 0x2C	&&
	    szHDPTRec[0] != 0x0D	&&
	    szHDPTRec[0] != 0x0A	&&
	    szHDPTRec[0] != 0x00)
        {
                memcpy(&srHDPTRec.szReversalCnt[0], &szHDPTRec[0], k - 1);
        }

        /* 05_TRTFILENAME */
        /* 初始化 */
        memset(szHDPTRec, 0x00, sizeof(szHDPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDPTRec[k++] = uszReadData[i++];
                if (szHDPTRec[k - 1] == 0x2C	||
		    szHDPTRec[k - 1] == 0x0D	||
		    szHDPTRec[k - 1] == 0x0A	||
		    szHDPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("HDPT unpack ERROR");
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
        if (szHDPTRec[0] != 0x2C	&&
	    szHDPTRec[0] != 0x0D	&&
	    szHDPTRec[0] != 0x0A	&&
	    szHDPTRec[0] != 0x00)
        {
                memcpy(&srHDPTRec.szTRTFileName[0], &szHDPTRec[0], k - 1);
        }

        /* 06_szMustSettleBit */
        /* 初始化 */
        memset(szHDPTRec, 0x00, sizeof(szHDPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDPTRec[k++] = uszReadData[i++];
                if (szHDPTRec[k - 1] == 0x2C	||
		    szHDPTRec[k - 1] == 0x0D	||
		    szHDPTRec[k - 1] == 0x0A	||
		    szHDPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("HDPT unpack error");
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
        if (szHDPTRec[0] != 0x2C	&&
	    szHDPTRec[0] != 0x0D	&&
	    szHDPTRec[0] != 0x0A	&&
	    szHDPTRec[0] != 0x00)
        {
                memcpy(&srHDPTRec.szMustSettleBit[0], &szHDPTRec[0], k - 1);
        }

        /* 07_szSendReversalBit */
        /* 初始化 */
        memset(szHDPTRec, 0x00, sizeof(szHDPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDPTRec[k++] = uszReadData[i++];
                if (szHDPTRec[k - 1] == 0x2C	||
		    szHDPTRec[k - 1] == 0x0D	||
		    szHDPTRec[k - 1] == 0x0A	||
		    szHDPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("HDPT unpack ERROR");
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
        if (szHDPTRec[0] != 0x2C	&&
	    szHDPTRec[0] != 0x0D	&&
	    szHDPTRec[0] != 0x0A	&&
	    szHDPTRec[0] != 0x00)
        {
                memcpy(&srHDPTRec.szSendReversalBit[0], &szHDPTRec[0], k - 1);
        }

	/* 08_szCLS_SettleBit */
        /* 初始化 */
        memset(szHDPTRec, 0x00, sizeof(szHDPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDPTRec[k++] = uszReadData[i++];
                if (szHDPTRec[k - 1] == 0x2C	||
		    szHDPTRec[k - 1] == 0x0D	||
		    szHDPTRec[k - 1] == 0x0A	||
		    szHDPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("HDPT unpack ERROR");
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
        if (szHDPTRec[0] != 0x2C	&&
	    szHDPTRec[0] != 0x0D	&&
	    szHDPTRec[0] != 0x0A	&&
	    szHDPTRec[0] != 0x00)
        {
                memcpy(&srHDPTRec.szCLS_SettleBit[0], &szHDPTRec[0], k - 1);
        }

	/* 09_szTicket_InvNum */
        /* 初始化 */
        memset(szHDPTRec, 0x00, sizeof(szHDPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDPTRec[k++] = uszReadData[i++];
                if (szHDPTRec[k - 1] == 0x2C	||
		    szHDPTRec[k - 1] == 0x0D	||
		    szHDPTRec[k - 1] == 0x0A	||
		    szHDPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("HDPT unpack ERROR");
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
        if (szHDPTRec[0] != 0x2C	&&
	    szHDPTRec[0] != 0x0D	&&
	    szHDPTRec[0] != 0x0A	&&
	    szHDPTRec[0] != 0x00)
        {
                memcpy(&srHDPTRec.szTicket_InvNum[0], &szHDPTRec[0], k - 1);
        }

	/* 10_szBatchNumLimit */
        /* 初始化 */
        memset(szHDPTRec, 0x00, sizeof(szHDPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDPTRec[k++] = uszReadData[i++];
                if (szHDPTRec[k - 1] == 0x2C	||
		    szHDPTRec[k - 1] == 0x0D	||
		    szHDPTRec[k - 1] == 0x0A	||
		    szHDPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("HDPT unpack ERROR");
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
        if (szHDPTRec[0] != 0x2C	&&
	    szHDPTRec[0] != 0x0D	&&
	    szHDPTRec[0] != 0x0A	&&
	    szHDPTRec[0] != 0x00) /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srHDPTRec.szBatchNumLimit[0], &szHDPTRec[0], k - 1);
        }

	/* 11_szMustISRUploadEnableBit */
        /* 初始化 */
        memset(szHDPTRec, 0x00, sizeof(szHDPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDPTRec[k++] = uszReadData[i++];
                if (szHDPTRec[k - 1] == 0x2C	||
		    szHDPTRec[k - 1] == 0x0D	||
		    szHDPTRec[k - 1] == 0x0A	||
		    szHDPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("HDPT unpack ERROR");
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
	if (szHDPTRec[0] != 0x2C	&&
		szHDPTRec[0] != 0x0D	&&
		szHDPTRec[0] != 0x0A	&&
		szHDPTRec[0] != 0x00) /* 因為是最後的欄位還要多判斷0x00 */
	{
		memcpy(&srHDPTRec.szMustISRUploadEnableBit[0], &szHDPTRec[0], k - 1);
	}

	/* release */
	/* 關檔 */
	inFILE_Close(&ulFile_Handle);

	/* Free pointer */
	free(uszReadData);
	free(uszTemp);

	inHDPT_TempRecordIndex = inHDPTRec;
	inHDPT_HadBeenSave = VS_FALSE;
	
	/* inLoadHDPTRec() END */
	inDISP_LogPrintfWithFlag("  inHDPTRec[%d] TempIdx[%d]END!!", inHDPTRec ,inHDPT_TempRecordIndex);

//	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "HDPT_27 inLoadHDPTRec END");

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}


#ifdef _MODIFY_HDPT_FUNC_
/*
Function        :inSaveHDPTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :寫入HDPT.dat，inHDPTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inSaveHDPTRec(int inHDPTRec)
{
	int	inRetVal = VS_ERROR;
	
	/* 因為沒讀取過檔案，不應該存檔，所以鎖機 2020/1/9 下午 3:03 [SAM] */
	if(inHDPT_TempRecordIndex != inHDPTRec)
	{
		inDISP_DispLogAndWriteFlie(" Save HDPT *Error* TempId[%d] InputId[%d] Line[%d]",inHDPT_TempRecordIndex,inHDPTRec ,__LINE__ );
//		inFunc_EDCLock();
//		return VS_SUCCESS;
	}
	
	/* 加速用 */
	if (_PARAMETER_SAVE_WAY_DAT_ == _PARAMETER_SAVE_WAY_DAT_SQLITE_)
	{
		inRetVal = inSaveHDPTRec_SQLite(inHDPTRec);
	}
	else if (_PARAMETER_SAVE_WAY_DAT_ == _PARAMETER_SAVE_WAY_DAT_LINUX_)
	{
		inRetVal = inSaveHDPTRec_Linux(inHDPTRec);
	}
	else
	{
		inRetVal = inSaveHDPTRec_CTOS(inHDPTRec);
	}
	
	if (inRetVal != VS_SUCCESS)
	{
		inFunc_EDCLock();
	}

        return (inRetVal);
}


/*
Function        :inSaveHDPTRec_Linux
Date&Time       :2019/1/17 上午 11:10
Describe        :寫入HDPT.dat，inHDPTRec是要讀哪一筆的紀錄，第一筆為0
 *		 改用原生Linux Function
*/
int inSaveHDPTRec_Linux(int inHDPTRec)
{
        int     	inPackCount = 0;        		/* uszWriteBuff_Record的index */
        int     	inRecIndex = 0;         		/* uszRead_Total_Buff的index */
        int     	inHDPT_Total_Rec = 0;    		/* HDPT.dat的總筆數 */
        int     	inRetVal;
        int     	i = 0, j = 0;
	int		inDatFd = 0;
	int		inBakFd = 0;
	int		inHDPTLength = 0;         		/* HDPT.dat檔案總長度 */
	int		inReadCnt = 0;
	int		inNowReadOffset = 0;
	char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];	/* debug message */
	char		*szRead_Total_Buff;
	char		*szWriteBuff_Record, *szWriteBuff_Org;
        long    	lnReadLength = 0;       		/* 每次要從HDPT.dat讀多少byte出來 */

	/* inSaveHDPTRec_Linux()_START */
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag("  inSaveHDPTRec_Linux(%d) ", inHDPTRec);

	/* 防呆先刪除HDPT.bak  */
	inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

        /* 新建HDPT.bak檔案 */
        inRetVal = inFile_Linux_Create_In_Fs_Data(&inBakFd, _HDPT_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFile_Linux_Close(inBakFd);

                return (VS_ERROR);
        }

        /* 開啟原檔案HDPT.dat */
        inRetVal = inFile_Linux_Open_In_Fs_Data(&inDatFd, _HDPT_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* HDPT.dat開檔失敗 ，不用關檔HDPT.dat */
                /* HDPT.bak仍要關並刪除 */
                inFile_Linux_Close(inBakFd);
                inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

                return(VS_ERROR);
        }

        /* 取得HDPT.dat檔案大小 */
	inRetVal = inFile_Linux_Get_FileSize(inDatFd, &inHDPTLength);

        /* 取得檔案大小失敗 */
        if (inRetVal == VS_ERROR)
        {
                /* HDPT.bak和HDPT.dat要關並刪除 */
                inFile_Linux_Close(inBakFd);
                inFile_Linux_Close(inDatFd);
                inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

                return (VS_ERROR);
        }
	
        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        szWriteBuff_Record = malloc(_SIZE_HDPT_REC_ + _SIZE_HDPT_COMMA_0D0A_);
        memset(szWriteBuff_Record, 0x00, _SIZE_HDPT_REC_ + _SIZE_HDPT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原HDPT.dat的每筆Record 初始化 */
        szWriteBuff_Org = malloc(_SIZE_HDPT_REC_ + _SIZE_HDPT_COMMA_0D0A_);
        memset(szWriteBuff_Org, 0x00, _SIZE_HDPT_REC_ + _SIZE_HDPT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存HDPT.dat全部資料 */
        szRead_Total_Buff = malloc(inHDPTLength + 1);
        memset(szRead_Total_Buff, 0x00, inHDPTLength + 1);
	
        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* InvoiceNum */
        memcpy(&szWriteBuff_Record[0], &srHDPTRec.szInvoiceNum[0], strlen(srHDPTRec.szInvoiceNum));
        inPackCount += strlen(srHDPTRec.szInvoiceNum);
        szWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* BatchNum */
        memcpy(&szWriteBuff_Record[inPackCount], &srHDPTRec.szBatchNum[0], strlen(srHDPTRec.szBatchNum));
        inPackCount += strlen(srHDPTRec.szBatchNum);
        szWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* STANNum */
        memcpy(&szWriteBuff_Record[inPackCount], &srHDPTRec.szSTANNum[0], strlen(srHDPTRec.szSTANNum));
        inPackCount += strlen(srHDPTRec.szSTANNum);
	szWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* ReversalCnt */
        memcpy(&szWriteBuff_Record[inPackCount], &srHDPTRec.szReversalCnt[0], strlen(srHDPTRec.szReversalCnt));
        inPackCount += strlen(srHDPTRec.szReversalCnt);
        szWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* TRTFileName */
        memcpy(&szWriteBuff_Record[inPackCount], &srHDPTRec.szTRTFileName[0], strlen(srHDPTRec.szTRTFileName));
	inPackCount += strlen(srHDPTRec.szTRTFileName);
        szWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* MustSettleBit */
	memcpy(&szWriteBuff_Record[inPackCount], &srHDPTRec.szMustSettleBit[0], strlen(srHDPTRec.szMustSettleBit));
        inPackCount += strlen(srHDPTRec.szMustSettleBit);
        szWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* SendReversalBit */
	memcpy(&szWriteBuff_Record[inPackCount], &srHDPTRec.szSendReversalBit[0], strlen(srHDPTRec.szSendReversalBit));
        inPackCount += strlen(srHDPTRec.szSendReversalBit);
        szWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* CLS_SettleBit */
	memcpy(&szWriteBuff_Record[inPackCount], &srHDPTRec.szCLS_SettleBit[0], strlen(srHDPTRec.szCLS_SettleBit));
        inPackCount += strlen(srHDPTRec.szCLS_SettleBit);
        szWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szTicket_InvNum */
	memcpy(&szWriteBuff_Record[inPackCount], &srHDPTRec.szTicket_InvNum[0], strlen(srHDPTRec.szTicket_InvNum));
        inPackCount += strlen(srHDPTRec.szTicket_InvNum);
        szWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* szBatchNumLimit */
        memcpy(&szWriteBuff_Record[inPackCount], &srHDPTRec.szBatchNumLimit[0], strlen(srHDPTRec.szBatchNumLimit));
        inPackCount += strlen(srHDPTRec.szBatchNumLimit);
	szWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* szMustISRUploadEnableBit */
	memcpy(&szWriteBuff_Record[inPackCount], &srHDPTRec.szMustISRUploadEnableBit[0], strlen(srHDPTRec.szMustISRUploadEnableBit));
	inPackCount += strlen(srHDPTRec.szMustISRUploadEnableBit);		
		
        /* 最後的data不用逗號 */
	/* 補上換行符號 */

        /* 0D是移至行首 */
        szWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        szWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;
	
        /* 讀HDPT.dat檔案存到uszRead_Total_Buff */
        if (inFile_Linux_Seek(inDatFd, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = inHDPTLength;
		inNowReadOffset = 0;
		
                while(1)
                {
			inReadCnt = lnReadLength;
			
			/* 就只讀剩餘長度 */
			if (inFile_Linux_Read(inDatFd, (char*)&szRead_Total_Buff[inNowReadOffset], &inReadCnt) == VS_SUCCESS)
			{
				lnReadLength -= inReadCnt;
				inNowReadOffset += inReadCnt;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFile_Linux_Close(inDatFd);
				inFile_Linux_Close(inBakFd);
				inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

				/* Free pointer */
				free(szRead_Total_Buff);
				free(szWriteBuff_Record);
				free(szWriteBuff_Org);

				return (VS_ERROR);
			}
                        
			if (lnReadLength == 0)
				break;
                } /* end for loop */
        }
        else
        {
                /* Close檔案 */
                inFile_Linux_Close(inDatFd);
                inFile_Linux_Close(inBakFd);
                inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

                /* Free pointer */
                free(szRead_Total_Buff);
                free(szWriteBuff_Record);
                free(szWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }
	
        /* 防呆 總record數量小於要存取inHDPTRec Return ERROR */
        /* 算總Record數 */
        for (i = 0; i < (inHDPTLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (szRead_Total_Buff[i] == 0x0A)
                        inHDPT_Total_Rec++;
        }

        /* 防呆 總record數量小於要存取inHDPTRec Return ERROR */
        if ((inHDPTRec + 1) > inHDPT_Total_Rec)
        {
		inDISP_LogPrintfWithFlag("No data or Index ERROR");

		/* 關檔 */
                inFile_Linux_Close(inDatFd);
                inFile_Linux_Close(inBakFd);
                inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

                /* Free pointer */
                free(szRead_Total_Buff);
                free(szWriteBuff_Record);
                free(szWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inHDPTRec決定要先存幾筆Record到HDPT.bak，ex:inHDPTRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inHDPTRec; i ++)
        {
                j = 0;

                while (1)
                {
                        /* 存Record */
                        szWriteBuff_Org[j] = szRead_Total_Buff[inRecIndex];
                        j++;

                        /* 每讀完一筆record，就把baffer中record的資料寫入bak */
                        if (szRead_Total_Buff[inRecIndex] == 0x0A)
                        {
                                /* 為防止寫入位置錯誤，先移動到bak的檔案結尾 */
                                inRetVal = inFile_Linux_Seek(inBakFd, 0, _SEEK_END_);

                                if (inRetVal != VS_SUCCESS)
                                {
                                        /* 關檔 */
                                        inFile_Linux_Close(inDatFd);
                                        inFile_Linux_Close(inBakFd);
                                        inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

                                        /* Free pointer */
                                        free(szWriteBuff_Record);
                                        free(szWriteBuff_Org);
                                        free(szRead_Total_Buff);

                                        return (VS_ERROR);
                                }

                                /* 寫入bak檔案 */
                                inRetVal = inFile_Linux_Write(inBakFd, szWriteBuff_Org, &j);

                                if (inRetVal != VS_SUCCESS)
                                {
                                        /* 關檔 */
                                        inFile_Linux_Close(inDatFd);
                                        inFile_Linux_Close(inBakFd);
                                        inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

                                        /* Free pointer */
                                        free(szWriteBuff_Record);
                                        free(szWriteBuff_Org);
                                        free(szRead_Total_Buff);

                                        return (VS_ERROR);
                                }

                                /* 清空buffer為下一筆寫入做準備 */
                                memset(szWriteBuff_Org, 0x00, sizeof(szWriteBuff_Org));
                                inRecIndex++;

                                break;
                        }

                        /* 讀下一個字元 */
                        inRecIndex++;
                }
        }

        /* 存組好的該HDPTRecord 到 HDPT.bak */
        inRetVal = inFile_Linux_Seek(inBakFd, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFile_Linux_Close(inDatFd);
                inFile_Linux_Close(inBakFd);
                inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

                /* Free pointer */
                free(szWriteBuff_Record);
                free(szWriteBuff_Org);
                free(szRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 將組好的record寫入bak檔 */
        inRetVal = inFile_Linux_Write(inBakFd, szWriteBuff_Record, &inPackCount);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFile_Linux_Close(inDatFd);
                inFile_Linux_Close(inBakFd);
                inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

                /* Free pointer */
                free(szWriteBuff_Record);
                free(szWriteBuff_Org);
                free(szRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原HDPT.dat Record 到 HDPT.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
        /* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
        inHDPTRec = inHDPTRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < inHDPTLength; i++)
        {
                if (szRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inHDPTRec)
                        {
                                inRecIndex = i + 1;
                                break;
                        }
                }
        }
	
        i = 0;
        memset(szWriteBuff_Org, 0x00, sizeof(szWriteBuff_Org));

        while (1)
        {
                /* 接續存原HDPT.dat的Record */
                szWriteBuff_Org[i] = szRead_Total_Buff[inRecIndex];
                i++;

                if (szRead_Total_Buff[inRecIndex] == 0x0A)
                {
                        /* 移動到bak的檔案結尾 */
                        inRetVal = inFile_Linux_Seek(inBakFd, 0, _SEEK_END_);
			
                        if (inRetVal != VS_SUCCESS)
                        {
                                /* 關檔 */
                                inFile_Linux_Close(inDatFd);
                                inFile_Linux_Close(inBakFd);
                                inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

                                /* Free pointer */
                                free(szWriteBuff_Record);
                                free(szWriteBuff_Org);
                                free(szRead_Total_Buff);

                                return (VS_ERROR);
                        }
		
                        inRetVal = inFile_Linux_Write(inBakFd, szWriteBuff_Org, &i);
			
                        if (inRetVal != VS_SUCCESS)
                        {
                                /* 關檔 */
                                inFile_Linux_Close(inDatFd);
                                inFile_Linux_Close(inBakFd);
                                inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

                                /* Free pointer */
                                free(szWriteBuff_Record);
                                free(szWriteBuff_Org);
                                free(szRead_Total_Buff);

                                return (VS_ERROR);
                        }

                        memset(szWriteBuff_Org, 0x00, sizeof(szWriteBuff_Org));
                        i = 0;
                }
                else if (szRead_Total_Buff[inRecIndex] == 0x00)
                {
                        /* 讀到0x00結束 */
                        break;
                }

                inRecIndex++;
        }
	
        /* 關檔 */
	fdatasync(inBakFd);
        inFile_Linux_Close(inDatFd);
        inFile_Linux_Close(inBakFd);

        /* Free pointer */
        free(szWriteBuff_Record);
        free(szWriteBuff_Org);
        free(szRead_Total_Buff);
	
        /* 刪除原HDPT.dat */
        inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_);

        /* 將HDPT.bak改名字為HDPT.dat取代原檔案 */
        if (inFile_Linux_Rename_In_Fs_Data(_HDPT_FILE_NAME_BAK_, _HDPT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }
	
	/* rwxr--r--*/
	inFunc_Data_Chmod("744", _HDPT_FILE_NAME_, _FS_DATA_PATH_);
	
	if (ginFindRunTime == VS_TRUE)
	{
		//inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
//	inHDPT_TempRecordIndex = -1;
	inHDPT_HadBeenSave = VS_TRUE;
	
        /* inSaveHDPTRec_Linux() END */
	inDISP_LogPrintfWithFlag("  inSaveHDPTRec_Linux(%d) ", inHDPTRec - 1);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	

        return(VS_SUCCESS);
}

#endif
/*
Function        : inSaveHDPTRec_CTOS
Date&Time   : 2015/8/31 下午 2:00
Describe        : 寫入HDPT.dat，inHDPTRec 是要讀哪一筆的紀錄，第一筆為0
 * ulFileSize 讀取檔案的長度
 * ulReadSize 實際讀取檔案的長度
 * ulPackCount 需寫入的資料長度
 * ulLineLenth  計算每行的暫存長度
 * ulCurrentLen 實際讀到要寫入的行數長度
*/
#ifdef _MODIFY_HDPT_FUNC_
int inSaveHDPTRec_CTOS(int inHDPTRec)
#else
int inSaveHDPTRec(int inHDPTRec)
#endif
{
	unsigned long   uldat_Handle;   			/* FILE Handle */
	int inRecCount = 0, inTempIndex = 0;
	int     	inRetVal;
	int     	i = 0;

	char szTempTitle[] = "inSaveHDPTRec";
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
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inSaveHDPTRec INIT" );
#endif
	
	inTempIndex = inHDPTRec;
		
	memset(szTempFIleName, 0x00, sizeof(szTempFIleName));
	strcat( szTempFIleName, _HDPT_FILE_NAME_);
	
	memset(szTempBakFileName, 0x00, sizeof(szTempBakFileName));
	strcat( szTempBakFileName, _HDPT_FILE_NAME_BAK_);
	
	inDISP_LogPrintfWithFlag("  %s Index[%d] ", szTempTitle, inTempIndex);
	
      
	/* 開啟原檔案HDPT.dat */
	inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_HDPT_FILE_NAME_);

	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  %s *Error* [%d] ",szTempTitle, inRetVal);
		inFunc_EDCLock();
//		inHDPT_TempRecordIndex = -1;
		return(VS_ERROR);
	}

	usRetVal = CTOS_FileGetSize((unsigned char *)_HDPT_FILE_NAME_, &ulFileSize);	
	
	if(usRetVal != d_OK)
	{
		inDISP_DispLogAndWriteFlie("  %s Get FileSize *Error* [%d] Size[%ld]",szTempTitle, usRetVal, ulFileSize);
		inFunc_EDCLock();
		/* Close檔案 */
		inFILE_Close(&uldat_Handle);
//		inHDPT_TempRecordIndex = -1;
		return (VS_ERROR);
	}		
	
	
	/* 組Write Record封包 */
	/* 給WriteBuff記憶體大小 */
	uszWriteBuff_Record = malloc(_SIZE_HDPT_REC_ + _SIZE_HDPT_COMMA_0D0A_);
	memset(uszWriteBuff_Record, 0x00, _SIZE_HDPT_REC_ + _SIZE_HDPT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */


	/* uszRead_Total_Buff儲存HDPT.dat全部資料 */
	uszRead_Total_Buff = malloc(ulFileSize + 1);
	memset(uszRead_Total_Buff, 0x00, ulFileSize + 1);

	ulPackCount = 0; /* uszWriteBuff_Record的index位置 */
	/* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* InvoiceNum */
	memcpy(&uszWriteBuff_Record[0], &srHDPTRec.szInvoiceNum[0], strlen(srHDPTRec.szInvoiceNum));
	ulPackCount += strlen(srHDPTRec.szInvoiceNum);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* BatchNum */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srHDPTRec.szBatchNum[0], strlen(srHDPTRec.szBatchNum));
	ulPackCount += strlen(srHDPTRec.szBatchNum);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* STANNum */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srHDPTRec.szSTANNum[0], strlen(srHDPTRec.szSTANNum));
	ulPackCount += strlen(srHDPTRec.szSTANNum);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* ReversalCnt */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srHDPTRec.szReversalCnt[0], strlen(srHDPTRec.szReversalCnt));
	ulPackCount += strlen(srHDPTRec.szReversalCnt);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TRTFileName */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srHDPTRec.szTRTFileName[0], strlen(srHDPTRec.szTRTFileName));
	ulPackCount += strlen(srHDPTRec.szTRTFileName);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* MustSettleBit */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srHDPTRec.szMustSettleBit[0], strlen(srHDPTRec.szMustSettleBit));
	ulPackCount += strlen(srHDPTRec.szMustSettleBit);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* SendReversalBit */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srHDPTRec.szSendReversalBit[0], strlen(srHDPTRec.szSendReversalBit));
	ulPackCount += strlen(srHDPTRec.szSendReversalBit);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* CLS_SettleBit */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srHDPTRec.szCLS_SettleBit[0], strlen(srHDPTRec.szCLS_SettleBit));
	ulPackCount += strlen(srHDPTRec.szCLS_SettleBit);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szTicket_InvNum */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srHDPTRec.szTicket_InvNum[0], strlen(srHDPTRec.szTicket_InvNum));
	ulPackCount += strlen(srHDPTRec.szTicket_InvNum);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szBatchNumLimit */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srHDPTRec.szBatchNumLimit[0], strlen(srHDPTRec.szBatchNumLimit));
	ulPackCount += strlen(srHDPTRec.szBatchNumLimit);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szMustISRUploadEnableBit */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srHDPTRec.szMustISRUploadEnableBit[0], strlen(srHDPTRec.szMustISRUploadEnableBit));
	ulPackCount += strlen(srHDPTRec.szMustISRUploadEnableBit);

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
//		inHDPT_TempRecordIndex = -1;
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
//		inHDPT_TempRecordIndex = -1;
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
//			inHDPT_TempRecordIndex = -1;
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
//				inHDPT_TempRecordIndex = -1;
				return (VS_ERROR);
			}
		}else{
			inDISP_DispLogAndWriteFlie("  First write len[%ld]", (ulReadSize -  ulFileTotalCount));
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
//				inHDPT_TempRecordIndex = -1;
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
//				inHDPT_TempRecordIndex = -1;
				return (VS_ERROR);
			}
			
			inDISP_DispLogAndWriteFlie("  Sec write len[%ld]", (ulReadSize -  (ulFileTotalCount+ulCurrentLen)));
			inDISP_DispLogAndWriteFlie("  Sec Addr [%ld]", ((ulFileTotalCount+ulCurrentLen)));
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
//				inHDPT_TempRecordIndex = -1;
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
//			inHDPT_TempRecordIndex = -1;
			return (VS_ERROR);
		}
		
		if(VS_SUCCESS != inFILE_Close(&uldat_BakHandle)){
			inDISP_DispLogAndWriteFlie("  %s File Close *Error*  Line[%d]", szTempTitle, __LINE__ );
			inFunc_EDCLock();
			/* Free pointer */
			free(uszRead_Total_Buff);
			free(uszWriteBuff_Record);
//			inHDPT_TempRecordIndex = -1;
			return (VS_ERROR);
		}
		
		/* 刪除原CDT.dat */
		if (inFILE_Delete((unsigned char *)szTempFIleName) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  %s File Del *Error* Name[%s] Line[%d]", szTempTitle,szTempFIleName , __LINE__ );
			/* Free pointer */
			free(uszRead_Total_Buff);
			free(uszWriteBuff_Record);
//			inHDPT_TempRecordIndex = -1;
			return (VS_ERROR);
		}

		/* 將CDT.bak改名字為CDT.dat取代原檔案 */
		if (inFILE_Rename((unsigned char *)szTempBakFileName, (unsigned char *)szTempFIleName) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  %s Rename  *Error* Name[%s] Line[%d]", szTempTitle,szTempFIleName , __LINE__ );
			/* Free pointer */
			free(uszRead_Total_Buff);
			free(uszWriteBuff_Record);
//			inHDPT_TempRecordIndex = -1;
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
			
//			inHDPT_TempRecordIndex = -1;
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
			
//			inHDPT_TempRecordIndex = -1;
			return (VS_ERROR);
		}
	
		/* 關檔 */
		inFILE_Close(&uldat_Handle);
	}
	
	/* Free pointer */
	free(uszWriteBuff_Record);
	free(uszRead_Total_Buff);

#ifdef __COUNT_FUNC_TIME__
	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inSaveHDPTRec END");
#endif
//	inHDPT_TempRecordIndex = -1;
	inHDPT_HadBeenSave = VS_TRUE;
	
	inDISP_LogPrintfWithFlag( "  End %s [%d] END!!", szTempTitle, inTempIndex);	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
        return(VS_SUCCESS);
}


#ifdef _MODIFY_HDPT_FUNC_
/*
Function        :inSaveHDPTRec_SQLite
Date&Time       :2019/4/23 下午 6:00
Describe        :寫入HDPT.dat，inHDPTRec是要讀哪一筆的紀錄，第一筆為0
 *		 改用SQLite
*/
int inSaveHDPTRec_SQLite(int inHDPTRec)
{
        int			inRetVal = VS_ERROR;
	SQLITE_ALL_TABLE	srAll = {0};

        /* inSaveHDPTRec_Linux()_START */
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);	
	inDISP_LogPrintfWithFlag("  inSaveHDPTRec_SQLite(%d) ", inHDPTRec);

	
	if (ginFindRunTime == VS_TRUE)
	{
//		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inHDPT_Table_Link_HDPTRec(&srAll, _LS_UPDATE_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_LockEdcDispLogAndWriteFlie("Table Link 失敗");
//		inHDPT_TempRecordIndex = -1;
		return (VS_ERROR);
	}
	
	inRetVal = inSqlite_Insert_Or_Replace_ByRecordID_All(gszParamDBPath, _HDPT_TABLE_NAME_, inHDPTRec, &srAll);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_LockEdcDispLogAndWriteFlie("db:%s,table:%s,Rec:%d update failed", gszParamDBPath, _HDPT_TABLE_NAME_, inHDPTRec);
//		inHDPT_TempRecordIndex = -1;
		return (VS_ERROR);
	}
	
	/* 同步 */
	sync();

	if (ginFindRunTime == VS_TRUE)
	{
//		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
//	inHDPT_TempRecordIndex = -1;
	inHDPT_HadBeenSave = VS_TRUE;
        /* inSaveHDPTRec_Linux() END */
	inDISP_LogPrintfWithFlag("inSaveHDPTRec_SQLite(%d)", inHDPTRec);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
        return(VS_SUCCESS);
}

/*
Function        :inHDPT_Table_Link_HDPTRec
Date&Time       :2019/4/24 上午 11:00
Describe        :將FunctionPointer放至對應位置
*/
int inHDPT_Table_Link_HDPTRec(SQLITE_ALL_TABLE *srAll, int inLinkState)
{
	SQLITE_INT32T_TABLE TABLE_BATCH_INT[] = 
	{
		{0	,"inTableID"		,&srHDPTRec.inRecordRowID							},	/* inTableID */
		{0	,""				,NULL										}	/* 這行用Null用來知道尾端在哪 */	
	};
	
	SQLITE_INT32T_TABLE TABLE_BATCH_INT64T[] = 
	{
		{0	,""				,NULL										}	/* 這行用Null用來知道尾端在哪 */
	};
	
	SQLITE_CHAR_TABLE TABLE_BATCH_CHAR[] =
	{
		{0	,""				,NULL				,0						}	/* 這行用Null用來知道尾端在哪 */
	};
	
	SQLITE_CHAR_TABLE TABLE_BATCH_TEXT[] =
	{
		
		{0	,"szInvoiceNum"		,&srHDPTRec.szInvoiceNum	,strlen(srHDPTRec.szInvoiceNum)			},	/* Invoice Number */
		{0	,"szBatchNum"		,&srHDPTRec.szBatchNum		,strlen(srHDPTRec.szBatchNum)			},	/* Batch Number */
		{0	,"szSTANNum"		,&srHDPTRec.szSTANNum		,strlen(srHDPTRec.szSTANNum)			},	/* STAN Number */
		{0	,"szReversalCnt"		,&srHDPTRec.szReversalCnt	,strlen(srHDPTRec.szReversalCnt)		},	/* Reversal Count */
		{0	,"szTRTFileName"		,&srHDPTRec.szTRTFileName	,strlen(srHDPTRec.szTRTFileName)		},	/* szTRTFileName */
		{0	,"szMustSettleBit"		,&srHDPTRec.szMustSettleBit	,strlen(srHDPTRec.szMustSettleBit)		},	/* 是否結帳的flag，預設值為N */
		{0	,"szSendReversalBit"	,&srHDPTRec.szSendReversalBit	,strlen(srHDPTRec.szSendReversalBit)		},	/* 預設值為N */
		{0	,"szCLS_SettleBit"		,&srHDPTRec.szCLS_SettleBit	,strlen(srHDPTRec.szCLS_SettleBit)		},	/* 看是否要續傳批次 */
		{0	,"szTicket_InvNum"	,&srHDPTRec.szTicket_InvNum	,strlen(srHDPTRec.szTicket_InvNum)		},	/* 紀錄電票的invoiceNum，簽單的調閱編號用szInvoiceNum的 */
		{0	,"szBatchNumLimit"	,&srHDPTRec.szBatchNumLimit	,strlen(srHDPTRec.szBatchNumLimit)		},	/* 單筆Batch的最大數量 */
		{0	,"szMustISRUploadEnableBit"	,&srHDPTRec.szMustISRUploadEnableBit	,strlen(srHDPTRec.szMustISRUploadEnableBit)		},	/* 用來判斷ISR是否要上傳 */
		{0	,""				,NULL				,0						}	/* 這行用Null用來知道尾端在哪 */
	};
	
	int	i;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);	
	
	for (i = 0;; i++)
	{
		/* 碰到Table底部，設定Tag數並跳出 */
		if (strlen(TABLE_BATCH_INT[i].szTag) == 0)
		{
			break;
		}
		
		/* pointer為空，則跳過 */
		if (TABLE_BATCH_INT[i].pTagValue == NULL)
			continue;
		
		/* 變數多過於原來設定的Tag數 */
		if (srAll->inIntNum == _TAG_INT_MAX_NUM_)
		{
			inDISP_LogPrintfWithFlag("Int變數過多");
			return (VS_ERROR);
		}
		
		/* Tag 名稱過長 */
		if (strlen(TABLE_BATCH_INT[i].szTag) > _TAG_MAX_LENGRH_)
		{
			inDISP_LogPrintfWithFlag("Tag 名稱過長");
			return (VS_ERROR);
		}
		
		/* Insert時不用塞TableID */
		if (inLinkState == _LS_INSERT_)
		{
			/* 判斷長度是因為避免相同字首比對錯誤 */
			if ((memcmp(TABLE_BATCH_INT[i].szTag, "inTableID", strlen("inTableID")) == 0) && (strlen(TABLE_BATCH_INT[i].szTag) == strlen("inTableID")))
			{
				continue;
			}
		}
		
		strcat(srAll->srInt[srAll->inIntNum].szTag, TABLE_BATCH_INT[i].szTag);
		srAll->srInt[srAll->inIntNum].pTagValue = TABLE_BATCH_INT[i].pTagValue;
		srAll->inIntNum++;
	}
	
	for (i = 0;; i++)
	{
		/* 碰到Table底部，設定Tag數並跳出 */
		if (strlen(TABLE_BATCH_INT64T[i].szTag) == 0)
		{
			break;
		}
		
		/* pointer為空，則跳過 */
		if (TABLE_BATCH_INT64T[i].pTagValue == NULL)
			continue;
		
		/* 變數多過於原來設定的Tag數 */
		if (srAll->inInt64tNum == _TAG_INT64T_MAX_NUM_)
		{
			inDISP_LogPrintfWithFlag("Int64t變數過多");
			return (VS_ERROR);
		}
		
		/* Tag 名稱過長 */
		if (strlen(TABLE_BATCH_INT64T[i].szTag) > _TAG_MAX_LENGRH_)
		{
			inDISP_LogPrintfWithFlag("Tag 名稱過長");
			return (VS_ERROR);
		}
		
		strcat(srAll->srInt64t[srAll->inInt64tNum].szTag, TABLE_BATCH_INT64T[i].szTag);
		srAll->srInt64t[srAll->inInt64tNum].pTagValue = TABLE_BATCH_INT64T[i].pTagValue;
		srAll->inInt64tNum++;
	}
	
	for (i = 0;; i++)
	{
		/* 碰到Table底部，設定Tag數並跳出 */
		if (strlen(TABLE_BATCH_CHAR[i].szTag) == 0)
		{
			break;
		}
		
		/* pointer為空，則跳過 */
		if (TABLE_BATCH_CHAR[i].pCharVariable == NULL)
			continue;
		
		/* 變數多過於原來設定的Tag數 */
		if (srAll->inCharNum == _TAG_CHAR_MAX_NUM_)
		{
			inDISP_LogPrintfWithFlag("Char變數過多");
			return (VS_ERROR);
		}
		
		/* Tag 名稱過長 */
		if (strlen(TABLE_BATCH_CHAR[i].szTag) > _TAG_MAX_LENGRH_)
		{
			inDISP_LogPrintfWithFlag("Tag 名稱過長");
			return (VS_ERROR);
		}
		
		strcat(srAll->srChar[srAll->inCharNum].szTag, TABLE_BATCH_CHAR[i].szTag);
		srAll->srChar[srAll->inCharNum].pCharVariable = TABLE_BATCH_CHAR[i].pCharVariable;
		srAll->srChar[srAll->inCharNum].inTagValueLen = TABLE_BATCH_CHAR[i].inTagValueLen;
		srAll->inCharNum++;
	}
	
	for (i = 0;; i++)
	{
		/* 碰到Table底部，設定Tag數並跳出 */
		if (strlen(TABLE_BATCH_TEXT[i].szTag) == 0)
		{
			break;
		}
		
		/* pointer為空，則跳過 */
		if (TABLE_BATCH_TEXT[i].pCharVariable == NULL)
			continue;
		
		/* 變數多過於原來設定的Tag數 */
		if (srAll->inTextNum == _TAG_TEXT_MAX_NUM_)
		{
			inDISP_LogPrintfWithFlag( "TEXT變數過多");
			return (VS_ERROR);
		}
		
		/* Tag 名稱過長 */
		if (strlen(TABLE_BATCH_TEXT[i].szTag) > _TAG_MAX_LENGRH_)
		{
			inDISP_LogPrintfWithFlag( "Tag 名稱過長");
			return (VS_ERROR);
		}
		
		strcat(srAll->srText[srAll->inTextNum].szTag, TABLE_BATCH_TEXT[i].szTag);
		srAll->srText[srAll->inTextNum].pCharVariable = TABLE_BATCH_TEXT[i].pCharVariable;
		srAll->srText[srAll->inTextNum].inTagValueLen = TABLE_BATCH_TEXT[i].inTagValueLen;
		srAll->inTextNum++;
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
	
	return (VS_SUCCESS);
}


/*
Function        :inHDPT_Initial_AllRercord
Date&Time       :2019/4/26 下午 4:35
Describe        :若參數生效，初始化HDPT
*/
int inHDPT_Initial_AllRercord(char* szDBName, char* szTableName)
{
	int	i = 0;
	
	/* 資料庫先清Table再重建*/
	if (_PARAMETER_SAVE_WAY_DAT_ == _PARAMETER_SAVE_WAY_DAT_SQLITE_)
	{
		inSqlite_Drop_Table(szDBName, szTableName);
		inEDCPara_Create_Table_Flow(szDBName, szTableName, TABLE_HDPT_TAG);
	}
	
	for (i = 0; ; i++)
	{
		/* 目前只有14個default record */
		if (i >= 14)
		{
			break;
		}
		
		inSetInvoiceNum("000001");
		inSetBatchNum("000001");
		inSetSTANNum("000001");
		inSetReversalCnt("000001");
		inSetMustSettleBit("N");
		inSetSendReversalBit("N");
		inSetCLS_SettleBit("N");
		inSetTicket_InvNum("000001");
		inSetBatchNumLimit("0600");
		inSetTicket_InvNum("000001");
		inSetBatchNumLimit("0600");
		inSetMustISRUploadEnableBit("0");/* 富邦新增的參數 20190130 [SAM] */
		
		
		switch (i)
		{
			case 0:
				inSetTRTFileName(_TRT_FILE_NAME_CREDIT_);
				break;
			case 1:
				inSetTRTFileName(_TRT_FILE_NAME_DINERS_);
				break;
			case 2:
				inSetTRTFileName(_TRT_FILE_NAME_HG_);
				break;
			case 3:
				inSetTRTFileName(_TRT_FILE_NAME_VASS_);
				break;
			case 4:
				inSetTRTFileName(_TRT_FILE_NAME_CLM_);
				break;
			case 5:
				inSetTRTFileName(_TRT_FILE_NAME_AMEX_);
				break;
			case 6:
				inSetTRTFileName(_TRT_FILE_NAME_DCC_);
				break;
			case 7:
				inSetTRTFileName(_TRT_FILE_NAME_ESC_);
				break;
			case 8:
				inSetTRTFileName(_TRT_FILE_NAME_ESVC_);
				break;
			case 9:
				inSetTRTFileName(_TRT_FILE_NAME_REDEMPTION_);
				break;
			case 10:
				inSetTRTFileName(_TRT_FILE_NAME_INSTALLMENT_);
				break;
			case 11:
				inSetTRTFileName(_TRT_FILE_NAME_UNION_PAY_);
				break;
			case 12:
				inSetTRTFileName(_TRT_FILE_NAME_FISC_);
				break;
			case 13:
				inSetTRTFileName(_TRT_FILE_NAME_MAIL_ORDER_);
				break;
			default:
				break;
		}
		
		inSaveHDPTRec(i);
	}
	
	return (VS_SUCCESS);
}

#endif


/*
set和get等價於相反的操作
各欄位的set和get function
*/

/*
Function        :inGetInvoiceNum
Date&Time       :
Describe        :
*/
int inGetInvoiceNum(char* szInvoiceNum)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szInvoiceNum == NULL || strlen(srHDPTRec.szInvoiceNum) <= 0 || strlen(srHDPTRec.szInvoiceNum) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetInvoiceNum() ERROR !!");

                        if (szInvoiceNum == NULL)
                        {
                                inDISP_LogPrintf("szInvoiceNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szInvoiceNum length = (%d)", (int)strlen(srHDPTRec.szInvoiceNum));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szInvoiceNum[0], &srHDPTRec.szInvoiceNum[0], strlen(srHDPTRec.szInvoiceNum));

        return (VS_SUCCESS);
}

/*
Function        :inSetInvoiceNum
Date&Time       :
Describe        :
*/
int inSetInvoiceNum(char* szInvoiceNum)
{
        memset(srHDPTRec.szInvoiceNum, 0x00, sizeof(srHDPTRec.szInvoiceNum));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szInvoiceNum == NULL || strlen(szInvoiceNum) <= 0 || strlen(szInvoiceNum) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetInvoiceNum() ERROR !!");

                        if (szInvoiceNum == NULL)
                        {
                                inDISP_LogPrintf("szInvoiceNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szInvoiceNum length = (%d)", (int)strlen(szInvoiceNum));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srHDPTRec.szInvoiceNum[0], &szInvoiceNum[0], strlen(szInvoiceNum));

        return (VS_SUCCESS);
}

/*
Function        :inGetBatchNum
Date&Time       :
Describe        :
*/
int inGetBatchNum(char* szBatchNum)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szBatchNum == NULL || strlen(srHDPTRec.szBatchNum) <= 0 || strlen(srHDPTRec.szBatchNum) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetBatchNum() ERROR !!");

                        if (szBatchNum == NULL)
                        {
                                inDISP_LogPrintf("szBatchNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBatchNum length = (%d) ", (int)strlen(srHDPTRec.szBatchNum));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }

        memcpy(&szBatchNum[0], &srHDPTRec.szBatchNum[0], strlen(srHDPTRec.szBatchNum));

        return (VS_SUCCESS);
}

/*
Function        :inSetBatchNum
Date&Time       :
Describe        :
*/
int inSetBatchNum(char* szBatchNum)
{
        memset(srHDPTRec.szBatchNum, 0x00, sizeof(srHDPTRec.szBatchNum));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szBatchNum == NULL || strlen(szBatchNum) <= 0 || strlen(szBatchNum) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetBatchNum() ERROR !!");

                        if (szBatchNum == NULL)
                        {
                                inDISP_LogPrintf("szBatchNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBatchNum length = (%d)", (int)strlen(szBatchNum));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srHDPTRec.szBatchNum[0], &szBatchNum[0], strlen(szBatchNum));

        return (VS_SUCCESS);
}

/*
Function        :inGetSTANNum
Date&Time       :
Describe        :
*/
int inGetSTANNum(char* szSTANNum)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSTANNum == NULL || strlen(srHDPTRec.szSTANNum) <= 0 || strlen(srHDPTRec.szSTANNum) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetSTANNum() ERROR !!");

                        if (szSTANNum == NULL)
                        {
                                inDISP_LogPrintf("szSTANNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSTANNum length = (%d) ", (int)strlen(srHDPTRec.szSTANNum));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSTANNum[0], &srHDPTRec.szSTANNum[0], strlen(srHDPTRec.szSTANNum));

        return (VS_SUCCESS);
}

/*
Function        :inSetSTANNum
Date&Time       :
Describe        :
*/
int inSetSTANNum(char* szSTANNum)
{
        memset(srHDPTRec.szSTANNum, 0x00, sizeof(srHDPTRec.szSTANNum));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSTANNum == NULL || strlen(szSTANNum) <= 0 || strlen(szSTANNum) > 6)
        {
		/* debug */
		if (ginDebug == VS_TRUE)
		{
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inSetSTANNum() ERROR !!");

			if (szSTANNum == NULL)
			{
				inDISP_LogPrintf("szSTANNum == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szSTANNum length = (%d)", (int)strlen(szSTANNum));
				inDISP_LogPrintf(szErrorMsg);
			}
		}

		return (VS_ERROR);
	}
        memcpy(&srHDPTRec.szSTANNum[0], &szSTANNum[0], strlen(szSTANNum));

        return (VS_SUCCESS);
}

/*
Function        :inGetReversalCnt
Date&Time       :
Describe        :
*/
int inGetReversalCnt(char* szReversalCnt)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szReversalCnt == NULL || strlen(srHDPTRec.szReversalCnt) <= 0 || strlen(srHDPTRec.szReversalCnt) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetReversalCnt() ERROR !!");

                        if (szReversalCnt == NULL)
                        {
                                inDISP_LogPrintf("szReversalCnt == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szReversalCnt length = (%d) ", (int)strlen(srHDPTRec.szReversalCnt));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szReversalCnt[0], &srHDPTRec.szReversalCnt[0], strlen(srHDPTRec.szReversalCnt));

        return (VS_SUCCESS);
}

/*
Function        :inSetReversalCnt
Date&Time       :
Describe        :
*/
int inSetReversalCnt(char* szReversalCnt)
{
        memset(srHDPTRec.szReversalCnt, 0x00, sizeof(srHDPTRec.szReversalCnt));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szReversalCnt == NULL || strlen(szReversalCnt) <= 0 || strlen(szReversalCnt) > 6)
        {
		/* debug */
		if (ginDebug == VS_TRUE)
		{
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inSetReversalCnt() ERROR !!");

			if (szReversalCnt == NULL)
			{
				inDISP_LogPrintf("szReversalCnt == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szReversalCnt length = (%d)", (int)strlen(szReversalCnt));
				inDISP_LogPrintf(szErrorMsg);
			}
		}

		return (VS_ERROR);
	}
        memcpy(&srHDPTRec.szReversalCnt[0], &szReversalCnt[0], strlen(szReversalCnt));

        return (VS_SUCCESS);
}

/*
Function        :inGetTRTFileName
Date&Time       :
Describe        :
*/
int inGetTRTFileName(char* szTRTFileName)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTRTFileName == NULL || strlen(srHDPTRec.szTRTFileName) <= 0 || strlen(srHDPTRec.szTRTFileName) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inGetTRTFileName() ERROR !!");

                        if (szTRTFileName == NULL)
                        {
                                inDISP_LogPrintf("szTRTFileName == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "(szTRTFileName length = %d) ", (int)strlen(srHDPTRec.szTRTFileName));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTRTFileName[0], &srHDPTRec.szTRTFileName[0], strlen(srHDPTRec.szTRTFileName));

        return (VS_SUCCESS);
}

/*
Function        :inSetTRTFileName
Date&Time       :
Describe        :
*/
int inSetTRTFileName(char* szTRTFileName)
{
        memset(srHDPTRec.szTRTFileName, 0x00, sizeof(srHDPTRec.szTRTFileName));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTRTFileName == NULL || strlen(szTRTFileName) <= 0 || strlen(szTRTFileName) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTRTFileName() ERROR !!");

                        if (szTRTFileName == NULL)
                        {
                                inDISP_LogPrintf("szTRTFileName == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTRTFileName length = (%d) ", (int)strlen(srHDPTRec.szTRTFileName));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srHDPTRec.szTRTFileName[0], &szTRTFileName[0], strlen(szTRTFileName));

        return (VS_SUCCESS);
}

/*
Function        :inGetMustSettleBit
Date&Time       :
Describe        :
*/
int inGetMustSettleBit(char* szMustSettleBit)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMustSettleBit == NULL || strlen(srHDPTRec.szMustSettleBit) <= 0 || strlen(srHDPTRec.szMustSettleBit) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inGetMustSettleBit() ERROR !!");

                        if (szMustSettleBit == NULL)
                        {
                                inDISP_LogPrintf("szMustSettleBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "(szMustSettleBit length = %d) ", (int)strlen(srHDPTRec.szMustSettleBit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMustSettleBit[0], &srHDPTRec.szMustSettleBit[0], strlen(srHDPTRec.szMustSettleBit));

        return (VS_SUCCESS);
}

/*
Function        :inSetMustSettleBit
Date&Time       :
Describe        :
*/
int inSetMustSettleBit(char* szMustSettleBit)
{
        memset(srHDPTRec.szMustSettleBit, 0x00, sizeof(srHDPTRec.szMustSettleBit));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMustSettleBit == NULL || strlen(szMustSettleBit) <= 0 || strlen(szMustSettleBit) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetMustSettleBit() ERROR !!");

                        if (szMustSettleBit == NULL)
                        {
                                inDISP_LogPrintf("szMustSettleBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMustSettleBit length = (%d) ", (int)strlen(srHDPTRec.szMustSettleBit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srHDPTRec.szMustSettleBit[0], &szMustSettleBit[0], strlen(szMustSettleBit));

        return (VS_SUCCESS);
}

/*
Function        :inGetSendReversalBit
Date&Time       :
Describe        :
*/
int inGetSendReversalBit(char* szSendReversalBit)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSendReversalBit == NULL || strlen(srHDPTRec.szSendReversalBit) <= 0 || strlen(srHDPTRec.szSendReversalBit) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inGetSendReversalBit() ERROR !!");

                        if (szSendReversalBit == NULL)
                        {
                                inDISP_LogPrintf("szSendReversalBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "(szSendReversalBit length = %d) ", (int)strlen(srHDPTRec.szSendReversalBit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSendReversalBit[0], &srHDPTRec.szSendReversalBit[0], strlen(srHDPTRec.szSendReversalBit));
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inGetSendReversalBit(%s) 1!!",szSendReversalBit);

        return (VS_SUCCESS);
}

/*
Function        :inSetSendReversalBit
Date&Time       :
Describe        :
*/
int inSetSendReversalBit(char* szSendReversalBit)
{
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inSetSendReversalBit(%s) 1!!",szSendReversalBit);
        memset(srHDPTRec.szSendReversalBit, 0x00, sizeof(srHDPTRec.szSendReversalBit));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSendReversalBit == NULL || strlen(szSendReversalBit) <= 0 || strlen(szSendReversalBit) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetSendReversalBit() ERROR !!");

                        if (szSendReversalBit == NULL)
                        {
                                inDISP_LogPrintf("szSendReversalBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSendReversalBit length = (%d) ", (int)strlen(srHDPTRec.szSendReversalBit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srHDPTRec.szSendReversalBit[0], &szSendReversalBit[0], strlen(szSendReversalBit));

        return (VS_SUCCESS);
}

/*
Function        :inGetCLS_SettleBit
Date&Time       :2017/4/10 下午 3:29
Describe        :
*/
int inGetCLS_SettleBit(char* szCLS_SettleBit)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCLS_SettleBit == NULL || strlen(srHDPTRec.szCLS_SettleBit) <= 0 || strlen(srHDPTRec.szCLS_SettleBit) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inGetCLS_SettleBit() ERROR !!");

                        if (szCLS_SettleBit == NULL)
                        {
                                inDISP_LogPrintf("szCLS_SettleBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "(szCLS_SettleBit length = %d) ", (int)strlen(srHDPTRec.szCLS_SettleBit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCLS_SettleBit[0], &srHDPTRec.szCLS_SettleBit[0], strlen(srHDPTRec.szCLS_SettleBit));

        return (VS_SUCCESS);
}

/*
Function        :inSetCLS_SettleBit
Date&Time       :2017/4/10 下午 3:29
Describe        :
*/
int inSetCLS_SettleBit(char* szCLS_SettleBit)
{
        memset(srHDPTRec.szCLS_SettleBit, 0x00, sizeof(srHDPTRec.szCLS_SettleBit));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCLS_SettleBit == NULL || strlen(szCLS_SettleBit) <= 0 || strlen(szCLS_SettleBit) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCLS_SettleBit() ERROR !!");

                        if (szCLS_SettleBit == NULL)
                        {
                                inDISP_LogPrintf("szCLS_SettleBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCLS_SettleBit length = (%d) ", (int)strlen(srHDPTRec.szCLS_SettleBit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srHDPTRec.szCLS_SettleBit[0], &szCLS_SettleBit[0], strlen(szCLS_SettleBit));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_InvNum
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inGetTicket_InvNum(char* szTicket_InvNum)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_InvNum == NULL || strlen(srHDPTRec.szTicket_InvNum) <= 0 || strlen(srHDPTRec.szTicket_InvNum) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTicket_InvNum() ERROR !!");

			if (szTicket_InvNum == NULL)
                        {
                                inDISP_LogPrintf("szTicket_InvNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_InvNum length = (%d)", (int)strlen(srHDPTRec.szTicket_InvNum));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_InvNum[0], &srHDPTRec.szTicket_InvNum[0], strlen(srHDPTRec.szTicket_InvNum));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_InvNum
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inSetTicket_InvNum(char* szTicket_InvNum)
{
        memset(srHDPTRec.szTicket_InvNum, 0x00, sizeof(srHDPTRec.szTicket_InvNum));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_InvNum == NULL || strlen(szTicket_InvNum) <= 0 || strlen(szTicket_InvNum) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTicket_InvNum() ERROR !!");
                        if (szTicket_InvNum == NULL)
                        {
                                inDISP_LogPrintf("szTicket_InvNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_InvNum length = (%d)", (int)strlen(szTicket_InvNum));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srHDPTRec.szTicket_InvNum[0], &szTicket_InvNum[0], strlen(szTicket_InvNum));

        return (VS_SUCCESS);
}

/*
Function        :inGetBatchNumLimit
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inGetBatchNumLimit(char* szBatchNumLimit)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szBatchNumLimit == NULL || strlen(srHDPTRec.szBatchNumLimit) <= 0 || strlen(srHDPTRec.szBatchNumLimit) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetBatchNumLimit() ERROR !!");

			if (szBatchNumLimit == NULL)
                        {
                                inDISP_LogPrintf("szBatchNumLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBatchNumLimit length = (%d)", (int)strlen(srHDPTRec.szBatchNumLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szBatchNumLimit[0], &srHDPTRec.szBatchNumLimit[0], strlen(srHDPTRec.szBatchNumLimit));

        return (VS_SUCCESS);
}

/*
Function        :inSetBatchNumLimit
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inSetBatchNumLimit(char* szBatchNumLimit)
{
        memset(srHDPTRec.szBatchNumLimit, 0x00, sizeof(srHDPTRec.szBatchNumLimit));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szBatchNumLimit == NULL || strlen(szBatchNumLimit) <= 0 || strlen(szBatchNumLimit) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetBatchNumLimit() ERROR !!");
                        if (szBatchNumLimit == NULL)
                        {
                                inDISP_LogPrintf("szBatchNumLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBatchNumLimit length = (%d)", (int)strlen(szBatchNumLimit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srHDPTRec.szBatchNumLimit[0], &szBatchNumLimit[0], strlen(szBatchNumLimit));

        return (VS_SUCCESS);
}

/*
Function        :szMustISRUploadEnableBit
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inGetMustISRUploadEnableBit(char* szMustISRUploadEnableBit)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szMustISRUploadEnableBit == NULL || strlen(srHDPTRec.szMustISRUploadEnableBit) <= 0 || strlen(srHDPTRec.szMustISRUploadEnableBit) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("szMustISRUploadEnableBit() ERROR 1!!");

			if (szMustISRUploadEnableBit == NULL)
                        {
                                inDISP_LogPrintf("szMustISRUploadEnableBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMustISRUploadEnableBit length = (%d)", (int)strlen(srHDPTRec.szMustISRUploadEnableBit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMustISRUploadEnableBit[0], &srHDPTRec.szMustISRUploadEnableBit[0], strlen(srHDPTRec.szMustISRUploadEnableBit));
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("inGetMustISRUploadEnableBit(%s) 1!!",szMustISRUploadEnableBit);

        return (VS_SUCCESS);
}

/*
Function        :szMustISRUploadEnableBit
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inSetMustISRUploadEnableBit(char* szMustISRUploadEnableBit)
{
	inDISP_LogPrintfWithFlag("  szMustISRUploadEnableBit [%s]",szMustISRUploadEnableBit);

        memset(srHDPTRec.szMustISRUploadEnableBit, 0x00, sizeof(srHDPTRec.szMustISRUploadEnableBit));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szMustISRUploadEnableBit == NULL || strlen(szMustISRUploadEnableBit) <= 0 || strlen(szMustISRUploadEnableBit) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("szMustISRUploadEnableBit() ERROR 2!!");
                        if (szMustISRUploadEnableBit == NULL)
                        {
                                inDISP_LogPrintf("szBatchNumLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBatchNumLimit length = (%d)", (int)strlen(szMustISRUploadEnableBit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srHDPTRec.szMustISRUploadEnableBit[0], &szMustISRUploadEnableBit[0], strlen(szMustISRUploadEnableBit));

        return (VS_SUCCESS);
}

/*
Function        :inHDPT_Edit_HDPT_Table
Date&Time       :2017/4/28 下午 5:18
Describe        :
*/
int inHDPT_Edit_HDPT_Table(void)
{
	TABLE_GET_SET_TABLE HDPT_FUNC_TABLE[] =
	{
		{"szInvoiceNum"				,inGetInvoiceNum				,inSetInvoiceNum},
		{"szBatchNum"				,inGetBatchNum					,inSetBatchNum},
		{"szSTANNum"				,inGetSTANNum				,inSetSTANNum},
		{"szReversalCnt"				,inGetReversalCnt				,inSetReversalCnt},
		{"szTRTFileName"			,inGetTRTFileName				,inSetTRTFileName},
		{"szMustSettleBit"			,inGetMustSettleBit				,inSetMustSettleBit},
		{"szSendReversalBit"			,inGetSendReversalBit			,inSetSendReversalBit},
		{"szCLS_SettleBit"			,inGetCLS_SettleBit				,inSetCLS_SettleBit},
		{"szTicket_InvNum"			,inGetTicket_InvNum				,inSetTicket_InvNum},
		{"szBatchNumLimit"			,inGetBatchNumLimit				,inSetBatchNumLimit},
		{"szMustISRUploadEnableBit"	,inGetMustISRUploadEnableBit		,inSetMustISRUploadEnableBit},
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
	inDISP_ChineseFont_Color("是否更改HDPT", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
	inLoadHDPTRec(inRecordCnt);
	inFunc_Edit_Table_Tag(HDPT_FUNC_TABLE);
	inSaveHDPTRec(inRecordCnt);

	return	(VS_SUCCESS);
}

/*
Function        :inHDPT_CheckCopyHDPT
Date&Time       :2017/8/18 下午 1:26
Describe        :因有些流程錯誤會讓DCC覆蓋NCCC的TRT，所以用這個檢查
*/
int inHDPT_CheckCopyHDPT()
{
	int	inIndex = 0;
	char	szTRTFileName[12 + 1];
	char	szDebugMsg[100 + 1];

	inIndex = 0;
	inLoadHDPTRec(inIndex);
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	inDISP_ChineseFont(szTRTFileName, _FONTSIZE_8X16_, _LINE_8_1_, _DISP_RIGHT_);
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("---------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "HDPT %d:%s", inIndex, szTRTFileName);
		inDISP_LogPrintf(szDebugMsg);
		inDISP_LogPrintf("---------------------------");
	}

	inIndex = 1;
	inLoadHDPTRec(inIndex);
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	inDISP_ChineseFont(szTRTFileName, _FONTSIZE_8X16_, _LINE_8_2_, _DISP_RIGHT_);
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("---------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "HDPT %d:%s", inIndex, szTRTFileName);
		inDISP_LogPrintf(szDebugMsg);
		inDISP_LogPrintf("---------------------------");
	}

	inIndex = 2;
	inLoadHDPTRec(inIndex);
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	inDISP_ChineseFont(szTRTFileName, _FONTSIZE_8X16_, _LINE_8_3_, _DISP_RIGHT_);
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("---------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "HDPT %d:%s", inIndex, szTRTFileName);
		inDISP_LogPrintf(szDebugMsg);
		inDISP_LogPrintf("---------------------------");
	}

	inIndex = 3;
	inLoadHDPTRec(inIndex);
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	inDISP_ChineseFont(szTRTFileName, _FONTSIZE_8X16_, _LINE_8_4_, _DISP_RIGHT_);
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("---------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "HDPT %d:%s", inIndex, szTRTFileName);
		inDISP_LogPrintf(szDebugMsg);
		inDISP_LogPrintf("---------------------------");
	}

	inIndex = 7;
	inLoadHDPTRec(inIndex);
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	inDISP_ChineseFont(szTRTFileName, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_RIGHT_);
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("---------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "HDPT %d:%s", inIndex, szTRTFileName);
		inDISP_LogPrintf(szDebugMsg);
		inDISP_LogPrintf("---------------------------");
	}

	inIndex = 8;
	inLoadHDPTRec(inIndex);
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	inDISP_ChineseFont(szTRTFileName, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_);
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("---------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "HDPT %d:%s", inIndex, szTRTFileName);
		inDISP_LogPrintf(szDebugMsg);
		inDISP_LogPrintf("---------------------------");
	}

	inIndex = 9;
	inLoadHDPTRec(inIndex);
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	inDISP_ChineseFont(szTRTFileName, _FONTSIZE_8X16_, _LINE_8_7_, _DISP_RIGHT_);
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("---------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "HDPT %d:%s", inIndex, szTRTFileName);
		inDISP_LogPrintf(szDebugMsg);
		inDISP_LogPrintf("---------------------------");
	}

	return (VS_SUCCESS);
}

int inHDPT_Test1(void)
{
        unsigned long   ulHandle;
        unsigned char   szFile_Buffer[_FILE_DATA_LENGTH_MAX_ + 1];
        int     inRetVal = 0;
        char    szTemplate[_FILE_DATA_LENGTH_MAX_ + 1];


        memset(szFile_Buffer, 0x00, sizeof(szFile_Buffer));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        /* 開啟已存在檔案 */
        inRetVal = inFILE_Open(&ulHandle, (unsigned char*)"HDPT.dat");
        /* 檔案不存在，建立新檔案 */
        //inRetVal = inFILE_Create(&ulHandle, "HDPT.dat");
        /* 尋找檔案測試 */
        //inFILE_Seek(ulHandle, 0, _SEEK_END_);
        /* 寫入檔案測試 不用seek END的話會把原資料改寫掉 */
        //inFILE_Write(&ulHandle, "12341234", sizeof("12341234"));
        /* 寫入檔案位置會到最尾巴，需要用seek回到開頭 */
        inFILE_Seek(ulHandle, 0, _SEEK_BEGIN_);
        /* 讀取檔案測試 */
        inFILE_Read(&ulHandle , szFile_Buffer , _FILE_DATA_LENGTH_MAX_);

        strcpy(szTemplate, (char *)szFile_Buffer);
        inDISP_LogPrintf(szTemplate);
        inFILE_Close(&ulHandle);

        return (VS_SUCCESS);
}

int inHDPT_Test2(void)
{
//        char    szTemplate[_FILE_DATA_LENGTH_MAX_ + 1];

        inLoadHDPTRec(0);
        inSetInvoiceNum("000002");
        inSetBatchNum("000003");
        inSetSTANNum("000004");
        inSaveHDPTRec(0);

        return (VS_SUCCESS);
}

