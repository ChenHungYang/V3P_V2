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

#include "TmsSCT.h"

static  TMSSCT_REC srTMSSCTRec;	/* construct TMSSCT record */
extern  int     ginDebug;       /* Debug使用 extern */

/*
Function        :inLoadTMSSCTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀TMSSCT檔案，inTMSSCTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadTMSSCTRec(int inTMSSCTRec)
{
	unsigned long   ulFile_Handle;                          /* File Handle */
	unsigned char   *uszReadData;                           /* 放抓到的record */
	unsigned char   *uszTemp;                               /* 暫存，放整筆TMSSCT檔案 */
	char            szTMSSCTRec[_SIZE_TMSSCT_REC_ + 1];         /* 暫存, 放各個欄位檔案 */
//	char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
	long            lnTMSSCTLength = 0;                       /* TMSSCT總長度 */
	long            lnReadLength;                           /* 記錄每次要從TMSSCT.dat讀多長的資料 */
	int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從TMSSCT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
	int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */
       
	/* inLoadTMSSCTRec()_START */
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
      
	/* 判斷傳進來的inTMSSCTRec是否小於零 大於等於零才是正確值(防呆) */
	if (inTMSSCTRec < 0)
	{
		inDISP_DispLogAndWriteFlie("  InData Code[%d] *Error* Line[%d]",inTMSSCTRec, __LINE__);
		return (VS_ERROR);
	}

	/*
	 * open TMSSCT.dat file
	 * open失敗時，if條件成立，關檔並回傳VS_ERROR
	 */
	if (inFILE_Open(&ulFile_Handle, (unsigned char *)_TMSSCT_FILE_NAME_) == VS_ERROR)
	{
		/* 開檔失敗 ，不用關檔 */
		/* 開檔失敗，所以回傳error */
		inDISP_DispLogAndWriteFlie("  Open TMSSCT File [%s] *Error* Line[%d]",_TMSSCT_FILE_NAME_, __LINE__);
		return (VS_ERROR);
	}

	/*
	 * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
	 * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
	 */
	lnTMSSCTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_TMSSCT_FILE_NAME_);

	if (lnTMSSCTLength == VS_ERROR)
	{
		/* GetSize失敗 ，關檔 */
		inDISP_DispLogAndWriteFlie("  GetSize TMSSCT Size[%d] *Error* Line[%d]",lnTMSSCTLength, __LINE__);
		inFILE_Close(&ulFile_Handle);
		return (VS_ERROR);
	}

	/*
	 * allocate 記憶體
	 * allocate時多分配一個byte以防萬一（ex:換行符號）
	 */
	uszReadData = malloc(lnTMSSCTLength + 1);
	uszTemp = malloc(lnTMSSCTLength + 1);
	/* 初始化 uszTemp uszReadData */
	memset(uszReadData, 0x00, lnTMSSCTLength + 1);
	memset(uszTemp, 0x00, lnTMSSCTLength + 1);

	/* seek 到檔案開頭 & 從檔案開頭開始read */
	if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
	{
		lnReadLength = lnTMSSCTLength;

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
	 *i為目前從TMSSCT讀到的第幾個字元
	 *j為該record的長度
	 */
	j = 0;
	for (i = 0; i <= lnTMSSCTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
	{
		/* 讀完一筆record或讀到TMSSCT的結尾時  */
		if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
		{
			/* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
			inSearchResult = 1;

			/* 清空uszReadData */
			memset(uszReadData, 0x00, lnTMSSCTLength + 1);
			/* 把record從temp指定的位置截取出來放到uszReadData */
			memcpy(&uszReadData[0], &uszTemp[i-j], j);
			inRec++;
			/* 因為inTMSSCT_Rec的index從0開始，所以inTMSSCT_Rec要+1 */
			if (inRec == (inTMSSCTRec + 1))
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
	 * 如果沒有inTMSSCTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
	 * 關檔、釋放記憶體並return VS_ERROR
	 * 如果總record數量小於要存取Record的Index
	 * 特例：有可能會遇到全文都沒有0x0D 0x0A
	 */
	if (inRec < (inTMSSCTRec + 1) || inSearchResult == -1)
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
	memset(&srTMSSCTRec, 0x00, sizeof(srTMSSCTRec));
	/*
	 * 以下pattern為存入TMSSCT_Rec
	 * i為TMSSCT的第幾個字元
	 * 存入TMSSCT_Rec
	 */
	i = 0;


	/* 01_TMS下載詢問機制 */
	/* 初始化 */
	memset(szTMSSCTRec, 0x00, sizeof(szTMSSCTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szTMSSCTRec[k ++] = uszReadData[i ++];
		if (szTMSSCTRec[k - 1] == 0x2C	||
			szTMSSCTRec[k - 1] == 0x0D	||
			szTMSSCTRec[k - 1] == 0x0A	||
			szTMSSCTRec[k - 1] == 0x00)
		{
				break;
		}

		if (i > lnTMSSCTLength)
		{
			if (ginDebug == VS_TRUE)
			{
					inDISP_LogPrintf("TMSSCT unpack ERROR.");
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
	if (szTMSSCTRec[0] != 0x2C	&&
	szTMSSCTRec[0] != 0x0D	&&
	szTMSSCTRec[0] != 0x0A	&&
	szTMSSCTRec[0] != 0x00)
	{
		memcpy(&srTMSSCTRec.szTMSInquireMode[0], &szTMSSCTRec[0], k - 1);
	}

	/* 02_TMS下載類型 */
	/* 初始化 */
	memset(szTMSSCTRec, 0x00, sizeof(szTMSSCTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szTMSSCTRec[k ++] = uszReadData[i ++];
		if (szTMSSCTRec[k - 1] == 0x2C	||
			szTMSSCTRec[k - 1] == 0x0D	||
			szTMSSCTRec[k - 1] == 0x0A	||
			szTMSSCTRec[k - 1] == 0x00)
		{
				break;
		}

		if (i > lnTMSSCTLength)
		{
			if (ginDebug == VS_TRUE)
			{
					inDISP_LogPrintf("TMSSCT unpack ERROR.");
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
	if (szTMSSCTRec[0] != 0x2C	&&
	szTMSSCTRec[0] != 0x0D	&&
	szTMSSCTRec[0] != 0x0A	&&
	szTMSSCTRec[0] != 0x00)
	{
		memcpy(&srTMSSCTRec.szTMSDownLoadType[0], &szTMSSCTRec[0], k - 1);
	}
	
	
	
        /* 03_TMS 安排時間自動訊問 排程起始日 */
        /* 初始化 */
        memset(szTMSSCTRec, 0x00, sizeof(szTMSSCTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSSCTRec[k ++] = uszReadData[i ++];
                if (szTMSSCTRec[k - 1] == 0x2C	||
		    szTMSSCTRec[k - 1] == 0x0D	||
		    szTMSSCTRec[k - 1] == 0x0A	||
		    szTMSSCTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSSCTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSSCT unpack ERROR");
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
        if (szTMSSCTRec[0] != 0x2C	&&
	    szTMSSCTRec[0] != 0x0D	&&
	    szTMSSCTRec[0] != 0x0A	&&
	    szTMSSCTRec[0] != 0x00)
        {
                memcpy(&srTMSSCTRec.szTMSInquireStartDate[0], &szTMSSCTRec[0], k - 1);
        }

        /* 04_TMS 安排時間自動訊問 指定詢問時間 */
        /* 初始化 */
        memset(szTMSSCTRec, 0x00, sizeof(szTMSSCTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSSCTRec[k ++] = uszReadData[i ++];
                if (szTMSSCTRec[k - 1] == 0x2C	||
		    szTMSSCTRec[k - 1] == 0x0D	||
		    szTMSSCTRec[k - 1] == 0x0A	||
		    szTMSSCTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSSCTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inDISP_LogPrintf("TMSSCT unpack ERROR");
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
        if (szTMSSCTRec[0] != 0x2C	&&
	    szTMSSCTRec[0] != 0x0D	&&
	    szTMSSCTRec[0] != 0x0A	&&
	    szTMSSCTRec[0] != 0x00)
        {
                memcpy(&srTMSSCTRec.szTMSInquireTime[0], &szTMSSCTRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
Function        :inSaveTMSSCTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveTMSSCTRec(int inTMSSCTRec)
{
  	unsigned long   uldat_Handle;   			/* FILE Handle */
	int inRecCount = 0, inTempIndex = 0;
	int     	inRetVal;
	int     	i = 0;

	char szTempTitle[] = "inSaveTMSSCTRec";
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
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inSaveTMSSCTRec INIT" );
#endif

	inTempIndex = inTMSSCTRec;
	
	memset(szTempFIleName, 0x00, sizeof(szTempFIleName));
	strcat( szTempFIleName, _TMSSCT_FILE_NAME_);
	
	memset(szTempBakFileName, 0x00, sizeof(szTempBakFileName));
	strcat( szTempBakFileName, _TMSSCT_FILE_NAME_BAK_);
	
	inDISP_LogPrintfWithFlag("  %s Index[%d] ", szTempTitle, inTempIndex);

      	/* 開啟原檔案HDPT.dat */
	inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_TMSSCT_FILE_NAME_);

	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  %s *Error* [%d] ",szTempTitle, inRetVal);
		inFunc_EDCLock();
		return(VS_ERROR);
	}

	usRetVal = CTOS_FileGetSize((unsigned char *)_TMSSCT_FILE_NAME_, &ulFileSize);	
	
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
	uszWriteBuff_Record = malloc(_SIZE_TMSSCT_REC_ + _SIZE_TMSSCT_COMMA_0D0A_);
	memset(uszWriteBuff_Record, 0x00, _SIZE_TMSSCT_REC_ + _SIZE_TMSSCT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

	/* uszRead_Total_Buff儲存TMSSCT.dat全部資料 */
	uszRead_Total_Buff = malloc(ulFileSize + 1);
	memset(uszRead_Total_Buff, 0x00, ulFileSize + 1);

	ulPackCount = 0; /* uszWriteBuff_Record的index位置 */
	/* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* TMSInquireMode */
	memcpy(&uszWriteBuff_Record[0], &srTMSSCTRec.szTMSInquireMode[0], strlen(srTMSSCTRec.szTMSInquireMode));
	ulPackCount += strlen(srTMSSCTRec.szTMSInquireMode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* szTMSDownLoadType */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSSCTRec.szTMSDownLoadType[0], strlen(srTMSSCTRec.szTMSDownLoadType));
	ulPackCount += strlen(srTMSSCTRec.szTMSDownLoadType);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;
	
	/* TMSInquireStartDate */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSSCTRec.szTMSInquireStartDate[0], strlen(srTMSSCTRec.szTMSInquireStartDate));
	ulPackCount += strlen(srTMSSCTRec.szTMSInquireStartDate);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* TMSInquireTime */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srTMSSCTRec.szTMSInquireTime[0], strlen(srTMSSCTRec.szTMSInquireTime));
	ulPackCount += strlen(srTMSSCTRec.szTMSInquireTime);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

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
		
		/* 刪除原 TMSSCT.dat */
		if (inFILE_Delete((unsigned char *)szTempFIleName) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  %s File Del *Error* Name[%s] Line[%d]", szTempTitle,szTempFIleName , __LINE__ );
			/* Free pointer */
			free(uszRead_Total_Buff);
			free(uszWriteBuff_Record);
			return (VS_ERROR);
		}

		/* 將 TMSSCT.bak改名字為CDT.dat取代原檔案 */
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
	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inSaveTMSSCTRec END");
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
Function        :inGetTMSInquireMode
Date&Time       :
Describe        :
*/
int inGetTMSInquireMode(char* szTMSInquireMode)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSInquireMode == NULL || strlen(srTMSSCTRec.szTMSInquireMode) <= 0 || strlen(srTMSSCTRec.szTMSInquireMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTMSInquireMode() ERROR !!");
                        
			if (szTMSInquireMode == NULL)
                        {
                                inDISP_LogPrintf("szTMSInquireMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSInquireMode length = (%d)", (int)strlen(srTMSSCTRec.szTMSInquireMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSInquireMode[0], &srTMSSCTRec.szTMSInquireMode[0], strlen(srTMSSCTRec.szTMSInquireMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSInquireMode
Date&Time       :
Describe        :
*/
int inSetTMSInquireMode(char* szTMSInquireMode)
{
        memset(srTMSSCTRec.szTMSInquireMode, 0x00, sizeof(srTMSSCTRec.szTMSInquireMode));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTMSInquireMode == NULL || strlen(szTMSInquireMode) <= 0 || strlen(szTMSInquireMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTMSInquireMode() ERROR !!");
                        if (szTMSInquireMode == NULL)
                        {
                                inDISP_LogPrintf("szTMSInquireMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSInquireMode length = (%d)", (int)strlen(szTMSInquireMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTMSSCTRec.szTMSInquireMode[0], &szTMSInquireMode[0], strlen(szTMSInquireMode));

        return (VS_SUCCESS);
}




/*
Function        :  inGetTMSDownLoadType
Date&Time   :
Describe        :
*/
int inGetTMSDownLoadType(char* szTMSDownLoadType)
{
	/* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
	if (szTMSDownLoadType == NULL || strlen(srTMSSCTRec.szTMSDownLoadType) <= 0 || strlen(srTMSSCTRec.szTMSDownLoadType) > 1)
	{
		/* debug */
		if (ginDebug == VS_TRUE)
		{
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inGetTMSInquireMode() ERROR !!");

			if (szTMSDownLoadType == NULL)
			{
				inDISP_LogPrintf("szTMSInquireMode == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szTMSInquireMode length = (%d)", (int)strlen(srTMSSCTRec.szTMSDownLoadType));
				inDISP_LogPrintf(szErrorMsg);
			}
		}

		return (VS_ERROR);
	}

	memcpy(&szTMSDownLoadType[0], &srTMSSCTRec.szTMSDownLoadType[0], strlen(srTMSSCTRec.szTMSDownLoadType));
	return (VS_SUCCESS);
}

/*
Function        : inSetTMSDownLoadType
Date&Time   :
Describe        :
*/
int inSetTMSDownLoadType(char* szTMSDownLoadType)
{
	memset(srTMSSCTRec.szTMSDownLoadType, 0x00, sizeof(srTMSSCTRec.szTMSDownLoadType));
	/* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
	if (szTMSDownLoadType == NULL || strlen(szTMSDownLoadType) <= 0 || strlen(szTMSDownLoadType) > 1)
	{
		/* debug */
		if (ginDebug == VS_TRUE)
		{
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inSetTMSInquireMode() ERROR !!");
			if (szTMSDownLoadType == NULL)
			{
				inDISP_LogPrintf("szTMSInquireMode == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szTMSInquireMode length = (%d)", (int)strlen(szTMSDownLoadType));
				inDISP_LogPrintf(szErrorMsg);
			}
		}

		return (VS_ERROR);
	}
	
	memcpy(&srTMSSCTRec.szTMSDownLoadType[0], &szTMSDownLoadType[0], strlen(szTMSDownLoadType));

	return (VS_SUCCESS);
}





/*
Function        :inGetTMSInquireStartDate
Date&Time       :
Describe        :
*/
int inGetTMSInquireStartDate(char* szTMSInquireStartDate)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSInquireStartDate == NULL || strlen(srTMSSCTRec.szTMSInquireStartDate) <= 0 || strlen(srTMSSCTRec.szTMSInquireStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTMSInquireStartDate() ERROR !!");

                        if (szTMSInquireStartDate == NULL)
                        {
                                inDISP_LogPrintf("szTMSInquireStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSInquireStartDate length = (%d)", (int)strlen(srTMSSCTRec.szTMSInquireStartDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSInquireStartDate[0], &srTMSSCTRec.szTMSInquireStartDate[0], strlen(srTMSSCTRec.szTMSInquireStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSInquireStartDate
Date&Time       :
Describe        :
*/
int inSetTMSInquireStartDate(char* szTMSInquireStartDate)
{
        memset(srTMSSCTRec.szTMSInquireStartDate, 0x00, sizeof(srTMSSCTRec.szTMSInquireStartDate));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSInquireStartDate == NULL || strlen(szTMSInquireStartDate) <= 0 || strlen(szTMSInquireStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTMSInquireStartDate() ERROR !!");

                        if (szTMSInquireStartDate == NULL)
                        {
                                inDISP_LogPrintf("szTMSInquireStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSInquireStartDate length = (%d)", (int)strlen(szTMSInquireStartDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTMSSCTRec.szTMSInquireStartDate[0], &szTMSInquireStartDate[0], strlen(szTMSInquireStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSInquireTime
Date&Time       :
Describe        :
*/
int inGetTMSInquireTime(char* szTMSInquireTime)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSInquireTime == NULL || strlen(srTMSSCTRec.szTMSInquireTime) <= 0 || strlen(srTMSSCTRec.szTMSInquireTime) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTMSInquireTime() ERROR !!");

                        if (szTMSInquireTime == NULL)
                        {
                                inDISP_LogPrintf("szTMSInquireTime == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSInquireTime length = (%d)", (int)strlen(srTMSSCTRec.szTMSInquireTime));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSInquireTime[0], &srTMSSCTRec.szTMSInquireTime[0], strlen(srTMSSCTRec.szTMSInquireTime));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSInquireTime
Date&Time       :
Describe        :
*/
int inSetTMSInquireTime(char* szTMSInquireTime)
{
        memset(srTMSSCTRec.szTMSInquireTime, 0x00, sizeof(srTMSSCTRec.szTMSInquireTime));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSInquireTime == NULL || strlen(szTMSInquireTime) <= 0 || strlen(szTMSInquireTime) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTMSInquireTime() ERROR !!");

                        if (szTMSInquireTime == NULL)
                        {
                                inDISP_LogPrintf("szTMSInquireTime == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSInquireTime length = (%d)", (int)strlen(szTMSInquireTime));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTMSSCTRec.szTMSInquireTime[0], &szTMSInquireTime[0], strlen(szTMSInquireTime));

        return (VS_SUCCESS);
}

/*
Function        : inTMSSCT_Edit_TMSSCT_Table
Date&Time   : 2017/7/21 上午 11:03
Describe        :
*/
int inTMSSCT_Edit_TMSSCT_Table(void)
{
	TABLE_GET_SET_TABLE TMSSCT_FUNC_TABLE[] =
	{
		{"szTMSInquireMode"		,inGetTMSInquireMode		,inSetTMSInquireMode},
		{"szTMSDownLoadType"	,inGetTMSDownLoadType		,inSetTMSDownLoadType},		
		{"szTMSInquireStartDate"	,inGetTMSInquireStartDate	,inSetTMSInquireStartDate	},
		{"szTMSInquireTime"		,inGetTMSInquireTime		,inSetTMSInquireTime		},
		{""},
	};
	int	inRetVal;
	char	szKey;
	
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont_Color("是否更改TMSSCT", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
	
	inSaveTMSSCTRec(0);
	inFunc_Edit_Table_Tag(TMSSCT_FUNC_TABLE);
	inSaveTMSSCTRec(0);
	
	return	(VS_SUCCESS);
}