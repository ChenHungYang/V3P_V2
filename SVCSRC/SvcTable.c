
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>

#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/Transaction.h"

#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/FUNCTION/FILE_FUNC/File.h"
#include "../SOURCE/FUNCTION/FILE_FUNC/FIleLogFunc.h"
#include "../SOURCE/FUNCTION/Function.h"

#include "SvcTable.h"

static  SVC_TABLE_REC srSvcTableRec;

/*
Function	: inLoadSvcTableRec
Date&Time	: 2022/12/15 下午 3:35
Describe	: 
 */
int inLoadSvcTableRec(int inRecIndex)
{
	unsigned long ulFile_Handle; /* File Handle */
	unsigned char *uszReadData; /* 放抓到的record */
	unsigned char *uszTemp; /* 暫存，放整筆檔案 */
	char szTempRec[_SIZE_SVC_TABLE_REC_ + 1]; /* 暫存, 放各個欄位檔案 */
	long lnTempRecLength = 0; /* 總長度 */
	long lnReadLength; /* 記錄每次要從EDC.dat讀多長的資料 */
	int i, k, j = 0, inRec = 0; /* inRec記錄讀到第幾筆, i為目前從檔案資料讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
	int inSearchResult = -1; /* 判斷有沒有讀到0x0D 0x0A的Flag */
	

	inDISP_LogPrintfWithFlag("-----[%s][%s] Line[%d]  START -----",__FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" Input index[%d]", inRecIndex);

	/* 判斷傳進來的inEDCRec是否小於零 大於等於零才是正確值(防呆) */
	if (inRecIndex < 0)
	{
		inDISP_LogPrintfWithFlag(" inRecIndex < 0:(index = %d) *Error* Line[%d]", inRecIndex, __LINE__);
		return (VS_ERROR);
	}

	/*
	 * open  file
	 * open失敗時，if條件成立，關檔並回傳VS_ERROR
	 */
	if (inFILE_Open(&ulFile_Handle, (unsigned char *) _SVC_TABLE_FILE_NAME_) == VS_ERROR)
	{
		/* 開檔失敗 ，不用關檔 */
		/* 開檔失敗，所以回傳error */
		inDISP_LogPrintfWithFlag(" Open EdcFile *Error* Line[%d] ", __LINE__);
		return (VS_ERROR);
	}

	/*
	 * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
	 * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
	 */
	lnTempRecLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *) _SVC_TABLE_FILE_NAME_);

	if (lnTempRecLength == VS_ERROR)
	{
		/* GetSize失敗 ，關檔 */
		inFILE_Close(&ulFile_Handle);
		inDISP_LogPrintfWithFlag(" Fn[%s] GetSize *Error* Line[%d] ",__FUNCTION__, __LINE__);
		return (VS_ERROR);
	}

	/*
	 * allocate 記憶體
	 * allocate時多分配一個byte以防萬一（ex:換行符號）
	 */
	uszReadData = malloc(lnTempRecLength + 1);
	uszTemp = malloc(lnTempRecLength + 1);
	/* 初始化 uszTemp uszReadData */
	memset(uszReadData, 0x00, lnTempRecLength + 1);
	memset(uszTemp, 0x00, lnTempRecLength + 1);
	
	
	/* seek 到檔案開頭 & 從檔案開頭開始read */
	if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
	{
		lnReadLength = lnTempRecLength;

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
				}					/* 讀失敗時 */
				else
				{
					/* Close檔案 */
					inFILE_Close(&ulFile_Handle);
					/* Free pointer */
					free(uszReadData);
					free(uszTemp);
					inDISP_DispLogAndWriteFlie(" Fu[%s] Read 1024 Bytes *Error* Line[%d] ", __FUNCTION__, __LINE__);
					return (VS_ERROR);
				}
			}				/* 剩餘長度小於1024 */
			else if (lnReadLength < 1024)
			{
				/* 就只讀剩餘長度 */
				if (inFILE_Read(&ulFile_Handle, &uszTemp[1024 * i], lnReadLength) == VS_SUCCESS)
				{
					break;
				}					/* 讀失敗時 */
				else
				{
					/* Close檔案 */
					inFILE_Close(&ulFile_Handle);

					/* Free pointer */
					free(uszReadData);
					free(uszTemp);
					inDISP_DispLogAndWriteFlie(" Fu[%s] Read [%ld] Bytes *Error* Line[%d] ", __FUNCTION__, lnReadLength, __LINE__);
					return (VS_ERROR);
				}
			}
		} /* end for loop */
	}		/* seek不成功時 */
	else
	{
		/* 關檔並回傳 */
		inFILE_Close(&ulFile_Handle);
		/* Free pointer */
		free(uszReadData);
		free(uszTemp);
		inDISP_DispLogAndWriteFlie(" Fu[%s] File Seek *Error* Line[%d] ", __FUNCTION__, lnReadLength, __LINE__);
		/* Seek失敗，所以回傳Error */
		return (VS_ERROR);
	}

	
	/*
	 *抓取所需要的那筆record
	 *i為目前從EDC讀到的第幾個字元
	 *j為該record的長度
	 */
	j = 0;
	for (i = 0; i <= lnTempRecLength; ++i) /* "<=" 是為了抓到最後一個0x00 */
	{
		/* 讀完一筆record或讀到EDC的結尾時  */
		if ((uszTemp[i] == 0x0D && uszTemp[i + 1] == 0x0A) || uszTemp[i] == 0x00)
		{
			/* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
			inSearchResult = 1;

			/* 清空uszReadData */
			memset(uszReadData, 0x00, lnTempRecLength + 1);
			/* 把record從temp指定的位置截取出來放到uszReadData */
			memcpy(&uszReadData[0], &uszTemp[i - j], j);
			inRec++;
			/* 因為inEDC_Rec的index從0開始，所以inEDC_Rec要+1 */
			if (inRec == (inRecIndex + 1))
			{
				break;
			}

			/* 為了跳過 0x0D 0x0A */
			i = i + 2;
			/* 每讀完一筆record，j就歸0 */
			j = 0;
		}

		j++;
	}

	/*
	 * 如果沒有inEDCRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
	 * 關檔、釋放記憶體並return VS_ERROR
	 * 如果總record數量小於要存取Record的Index
	 * 特例：有可能會遇到全文都沒有0x0D 0x0A
	 */
	if (inRec < (inRecIndex + 1) || inSearchResult == -1)
	{
		inDISP_DispLogAndWriteFlie(" Fn[%s] No data or Index[%d] *Error* Line[%d] ",__FUNCTION__, inRecIndex,  __LINE__);
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
		inDISP_DispLogAndWriteFlie(" Fn[%s] No specific data *Error* Line[%d] ", __FUNCTION__, __LINE__);
		/* 關檔 */
		inFILE_Close(&ulFile_Handle);
		/* Free pointer */
		free(uszReadData);
		free(uszTemp);
		return (VS_ERROR);
	}
	
	/* 結構初始化 */
	memset(&srSvcTableRec, 0x00, sizeof (srSvcTableRec));
	
	/* 計算讀出來的資料的長度，從0開始計算 */
	i = 0;

	/* 01 參數下載版本 */
	/* 初始化 */
	memset(szTempRec, 0x00, sizeof (szTempRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szTempRec[k++] = uszReadData[i++];		
		if (szTempRec[k - 1] == 0x2C ||
			szTempRec[k - 1] == 0x0D ||
			szTempRec[k - 1] == 0x0A ||
			szTempRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnTempRecLength)
		{
			inDISP_LogPrintfWithFlag(" Fu[%s] unpack ERROR ", __FUNCTION__);
			/* 關檔 */
			inFILE_Close(&ulFile_Handle);
			/* Free pointer */
			free(uszReadData);
			free(uszTemp);
			return (VS_ERROR);
		}
	}

	/*  該筆有資料 */
	if (szTempRec[0] != 0x2C &&
		szTempRec[0] != 0x0D &&
		szTempRec[0] != 0x0A &&
		szTempRec[0] != 0x00)
	{
		memcpy(&srSvcTableRec.szSVCParameterVersion[0], &szTempRec[0], k - 1);
	}
	
	/* 02 參數版本時間 */
	/* 初始化 */
	memset(szTempRec, 0x00, sizeof (szTempRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szTempRec[k++] = uszReadData[i++];		
		if (szTempRec[k - 1] == 0x2C ||
			szTempRec[k - 1] == 0x0D ||
			szTempRec[k - 1] == 0x0A ||
			szTempRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnTempRecLength)
		{
			inDISP_LogPrintfWithFlag(" Fu[%s] unpack ERROR ", __FUNCTION__);
			/* 關檔 */
			inFILE_Close(&ulFile_Handle);
			/* Free pointer */
			free(uszReadData);
			free(uszTemp);
			return (VS_ERROR);
		}
	}

	/*  該筆有資料 */
	if (szTempRec[0] != 0x2C &&
		szTempRec[0] != 0x0D &&
		szTempRec[0] != 0x0A &&
		szTempRec[0] != 0x00)
	{
		memcpy(&srSvcTableRec.szSVCParameterDateTime[0], &szTempRec[0], k - 1);
	}
	
	/* 02 亂碼化資料 */
	/* 初始化 */
	memset(szTempRec, 0x00, sizeof (szTempRec));
	k = 0;
	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szTempRec[k++] = uszReadData[i++];		
		if (szTempRec[k - 1] == 0x2C ||
			szTempRec[k - 1] == 0x0D ||
			szTempRec[k - 1] == 0x0A ||
			szTempRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnTempRecLength)
		{
			inDISP_LogPrintfWithFlag(" Fu[%s] unpack ERROR ", __FUNCTION__);
			/* 關檔 */
			inFILE_Close(&ulFile_Handle);
			/* Free pointer */
			free(uszReadData);
			free(uszTemp);
			return (VS_ERROR);
		}
	}

	/*  該筆有資料 */
	if (szTempRec[0] != 0x2C &&
		szTempRec[0] != 0x0D &&
		szTempRec[0] != 0x0A &&
		szTempRec[0] != 0x00)
	{
		memcpy(&srSvcTableRec.szSVCMaskData[0], &szTempRec[0], k - 1);
	}
	
	
	/* release */
	/* 關檔 */
	inFILE_Close(&ulFile_Handle);
	
	/* Free pointer */
	free(uszReadData);
	free(uszTemp);
	
	inDISP_LogPrintfWithFlag("-----[%s][%s] Line[%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}


/*
Function	: inSaveSvcTableRec
Date&Time	: 2022/12/15 下午 3:28
Describe	: 
*/
int inSaveSvcTableRec(int inRecIndex)
{
	unsigned long   uldat_Handle;   			/* FILE Handle */
	int inRecCount = 0, inTempIndex = 0;
	int     	inRetVal;
	int     	i = 0;

	char szTempTitle[] = "inSaveSvcTableRec";
	char szTempFIleName[32], szTempBakFileName[32];
	unsigned char   *uszRead_Total_Buff;
	unsigned char   *uszWriteBuff_Record;
	unsigned long ulFileSize = 0, ulReadSize = 0, ulPackCount = 0,  ulLineLenth = 0;;
	unsigned long ulCurrentLen = 0, ulFileTotalCount = 0;
	unsigned long  uldat_BakHandle;
	int inOnlyOneRecord, inMustCreateFile, inFundRec;
	unsigned short usRetVal;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	inTempIndex = inRecIndex;
	
	memset(szTempFIleName, 0x00, sizeof(szTempFIleName));
	strcat( szTempFIleName, _SVC_TABLE_FILE_NAME_);
	
	memset(szTempBakFileName, 0x00, sizeof(szTempBakFileName));
	strcat( szTempBakFileName, _SVC_TABLE_FILE_NAME_BAK_);
	
	inDISP_LogPrintfWithFlag("  %s Index[%d] ", szTempTitle, inTempIndex);
	
	/* 開啟原檔案HDPT.dat */
	inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)szTempFIleName);

	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  %s Open *Error* [%d] ",szTempTitle, inRetVal);
		inFunc_EDCLock();
		return(VS_ERROR);
	}

	usRetVal = CTOS_FileGetSize((unsigned char *)szTempFIleName, &ulFileSize);	
	
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
	uszWriteBuff_Record = malloc(_SIZE_SVC_TABLE_REC_ + _SIZE_SVC_TABLE_COMMA_0D0A_);
	memset(uszWriteBuff_Record, 0x00, _SIZE_SVC_TABLE_REC_ + _SIZE_SVC_TABLE_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

	/* uszRead_Total_Buff 儲存讀出檔案的全部資料 */
	uszRead_Total_Buff = malloc(ulFileSize + 1);
	memset(uszRead_Total_Buff, 0x00, ulFileSize + 1);

	ulPackCount = 0; /* uszWriteBuff_Record的index位置 */
	/* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* 1 szSVCParameterVersion */
	memcpy(&uszWriteBuff_Record[0], &srSvcTableRec.szSVCParameterVersion[0], strlen(srSvcTableRec.szSVCParameterVersion));
	ulPackCount += strlen(srSvcTableRec.szSVCParameterVersion);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;
	
	/* 2 szSVCParameterDateTime */
	memcpy(&uszWriteBuff_Record[0], &srSvcTableRec.szSVCParameterDateTime[0], strlen(srSvcTableRec.szSVCParameterDateTime));
	ulPackCount += strlen(srSvcTableRec.szSVCParameterDateTime);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;
	
	/* 3 szSVCMaskData */
	memcpy(&uszWriteBuff_Record[0], &srSvcTableRec.szSVCMaskData[0], strlen(srSvcTableRec.szSVCMaskData));
	ulPackCount += strlen(srSvcTableRec.szSVCMaskData);
		
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
		/* 開啟原檔案EDC.dat */
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
	
	return(VS_SUCCESS);
}

/*
Function        : inGetSVCTmsOK
Date&Time   : 2022/12/14 下午 4:57
Describe        :
 *  [新增SVC功能] 下載SVC參數檔完成 [SAM] 
*/
int inGetSVCTmsOK(char* szSVCTmsOK)
{
	/* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
	if (szSVCTmsOK == NULL || strlen(srSvcTableRec.szSVCTmsOK) <=0 || strlen(srSvcTableRec.szSVCTmsOK) > 1)
	{
		inDISP_LogPrintfWithFlag("inGetSVCParameterVersion() ERROR !!");

		if (szSVCTmsOK == NULL)
		{
			inDISP_LogPrintfWithFlag("szSVCTmsOK == NULL");
		}
		else
		{
			inDISP_LogPrintfWithFlag("szSVCTmsOK length = (%d)", (int)strlen(srSvcTableRec.szSVCTmsOK));
		}
		return (VS_ERROR);
	}
	
	memcpy(&szSVCTmsOK[0], &srSvcTableRec.szSVCTmsOK[0], strlen(srSvcTableRec.szSVCTmsOK));

	inDISP_LogPrintfWithFlag("inGetSVCParameterVersion (%s)",szSVCTmsOK);

	return (VS_SUCCESS);
}

/*
Function        : inSetSVCParameterVersion
Date&Time   : 2022/12/14 下午 4:41
Describe        :
 *  [新增SVC功能] 下載SVC參數檔完成 [SAM] 
*/
int inSetSVCTmsOK(char* szSVCTmsOK)
{

	inDISP_LogPrintfWithFlag("inSetSVCTmsOK (%s)",szSVCTmsOK);

	memset(srSvcTableRec.szSVCTmsOK, 0x00, sizeof(srSvcTableRec.szSVCTmsOK));
	/* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
	if (szSVCTmsOK == NULL || strlen(szSVCTmsOK) <= 0 || strlen(szSVCTmsOK) > 1)
	{
		inDISP_LogPrintfWithFlag("inSetSVCTmsOK() ERROR !!");

		if (szSVCTmsOK == NULL)
		{
			inDISP_LogPrintfWithFlag("szSVCTmsOK == NULL");
		}
		else
		{
			inDISP_LogPrintfWithFlag("szSVCTmsOK length = (%d)", (int)strlen(szSVCTmsOK));
		}
		return (VS_ERROR);
	}
    
	memcpy(&srSvcTableRec.szSVCTmsOK[0], &szSVCTmsOK[0], strlen(szSVCTmsOK));

	return (VS_SUCCESS);
}



/*
Function        : inGetSVCParameterVersion
Date&Time   : 2022/12/14 下午 4:57
Describe        :
 *  [新增SVC功能] 下載SVC參數檔的版本 [SAM] 
*/
int inGetSVCParameterVersion(char* szSVCParameterVersion)
{
	/* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
	if (szSVCParameterVersion == NULL || strlen(srSvcTableRec.szSVCParameterVersion) <=0 || strlen(srSvcTableRec.szSVCParameterVersion) > 6)
	{
		inDISP_LogPrintfWithFlag("inGetSVCParameterVersion() ERROR !!");

		if (szSVCParameterVersion == NULL)
		{
			inDISP_LogPrintfWithFlag("szSVCParameterVersion == NULL");
		}
		else
		{
			inDISP_LogPrintfWithFlag("szSVCParameterVersion length = (%d)", (int)strlen(srSvcTableRec.szSVCParameterVersion));
		}
		return (VS_ERROR);
	}
	
	memcpy(&szSVCParameterVersion[0], &srSvcTableRec.szSVCParameterVersion[0], strlen(srSvcTableRec.szSVCParameterVersion));

	inDISP_LogPrintfWithFlag("inGetSVCParameterVersion (%s)",szSVCParameterVersion);

	return (VS_SUCCESS);
}

/*
Function        : inSetSVCParameterVersion
Date&Time   : 2022/12/14 下午 4:41
Describe        :
 *  [新增SVC功能] 下載SVC參數檔的版本 [SAM] 
*/
int inSetSVCParameterVersion(char* szSVCParameterVersion)
{

	inDISP_LogPrintfWithFlag("inSetSVCParameterVersion (%s)",szSVCParameterVersion);

	memset(srSvcTableRec.szSVCParameterVersion, 0x00, sizeof(srSvcTableRec.szSVCParameterVersion));
	/* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
	if (szSVCParameterVersion == NULL || strlen(szSVCParameterVersion) <= 0 || strlen(szSVCParameterVersion) > 6)
	{
		inDISP_LogPrintfWithFlag("inSetCMASAddTcpipMode() ERROR !!");

		if (szSVCParameterVersion == NULL)
		{
			inDISP_LogPrintfWithFlag("szSVCParameterVersion == NULL");
		}
		else
		{
			inDISP_LogPrintfWithFlag("szSVCParameterVersion length = (%d)", (int)strlen(szSVCParameterVersion));
		}
		return (VS_ERROR);
	}
    
	memcpy(&srSvcTableRec.szSVCParameterVersion[0], &szSVCParameterVersion[0], strlen(szSVCParameterVersion));

	return (VS_SUCCESS);
}

/*
Function        : inGetSVCParameterDateTime
Date&Time   : 2022/12/14 下午 4:55
Describe        :
 *  [新增SVC功能] 下載SVC參數檔的時間 [SAM] 
*/
int inGetSVCParameterDateTime(char* szSVCParameterDateTime)
{
	/* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
	if (szSVCParameterDateTime == NULL || strlen(srSvcTableRec.szSVCParameterDateTime) <=0 || strlen(srSvcTableRec.szSVCParameterDateTime) > 14)
	{
		inDISP_LogPrintfWithFlag("inGetSVCParameterDateTime() ERROR !!");

		if (szSVCParameterDateTime == NULL)
		{
			inDISP_LogPrintfWithFlag("szSVCParameterDateTime == NULL");
		}
		else
		{
			inDISP_LogPrintfWithFlag("szSVCParameterDateTime length = (%d)", (int)strlen(srSvcTableRec.szSVCParameterDateTime));
		}
		return (VS_ERROR);
	}
	
	memcpy(&szSVCParameterDateTime[0], &srSvcTableRec.szSVCParameterDateTime[0], strlen(srSvcTableRec.szSVCParameterDateTime));

	inDISP_LogPrintfWithFlag("inGetSVCParameterDateTime (%s)",szSVCParameterDateTime);

	return (VS_SUCCESS);
}

/*
Function        : inSetSVCParameterDateTime
Date&Time   : 2022/12/14 下午 4:56
Describe        :
 * [新增SVC功能] 下載SVC參數檔的時間 [SAM] 
*/
int inSetSVCParameterDateTime(char* szSVCParameterDateTime)
{

	inDISP_LogPrintfWithFlag("inSetSVCParameterDateTime (%s)",szSVCParameterDateTime);

	memset(srSvcTableRec.szSVCParameterDateTime, 0x00, sizeof(srSvcTableRec.szSVCParameterDateTime));
	/* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
	if (szSVCParameterDateTime == NULL || strlen(szSVCParameterDateTime) <= 0 || strlen(szSVCParameterDateTime) > 14)
	{
		inDISP_LogPrintfWithFlag("inSetSVCParameterDateTime() ERROR !!");

		if (szSVCParameterDateTime == NULL)
		{
			inDISP_LogPrintfWithFlag("szSVCParameterDateTime == NULL");
		}
		else
		{
			inDISP_LogPrintfWithFlag("szSVCParameterDateTime length = (%d)", (int)strlen(szSVCParameterDateTime));
		}
		return (VS_ERROR);
	}
    
	memcpy(&srSvcTableRec.szSVCParameterDateTime[0], &szSVCParameterDateTime[0], strlen(szSVCParameterDateTime));

	return (VS_SUCCESS);
}

/*
Function        : inGetSVCMaskData
Date&Time   : 2022/12/15 下午 2:01
Describe        :
 *  [新增SVC功能] 下載亂碼化 MASK [SAM] 
*/
int inGetSVCMaskData(char* szSVCMaskData)
{
	/* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
	if (szSVCMaskData == NULL || strlen(srSvcTableRec.szSVCMaskData) <=0 || strlen(srSvcTableRec.szSVCMaskData) > 20)
	{
		inDISP_LogPrintfWithFlag("inGetSVCMaskData() ERROR !!");

		if (szSVCMaskData == NULL)
		{
			inDISP_LogPrintfWithFlag("szSVCMaskData == NULL");
		}
		else
		{
			inDISP_LogPrintfWithFlag("szSVCMaskData length = (%d)", (int)strlen(srSvcTableRec.szSVCMaskData));
		}
		return (VS_ERROR);
	}
	
	memcpy(&szSVCMaskData[0], &srSvcTableRec.szSVCMaskData[0], strlen(srSvcTableRec.szSVCMaskData));

	inDISP_LogPrintfWithFlag("inGetSVCMaskData (%s)",szSVCMaskData);

	return (VS_SUCCESS);
}

/*
Function        : inSetSVCParameterDateTime
Date&Time   : 2022/12/14 下午 4:56
Describe        :
 * [新增SVC功能] 下載亂碼化 MASK [SAM] 
*/
int inSetSVCMaskData(char* szSVCMaskData)
{

	inDISP_LogPrintfWithFlag("inSetSVCMaskData (%s)",szSVCMaskData);

	memset(srSvcTableRec.szSVCMaskData, 0x00, sizeof(srSvcTableRec.szSVCMaskData));
	/* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
	if (szSVCMaskData == NULL || strlen(szSVCMaskData) <= 0 || strlen(szSVCMaskData) > 20)
	{
		inDISP_LogPrintfWithFlag("inSetSVCMaskData() ERROR !!");

		if (szSVCMaskData == NULL)
		{
			inDISP_LogPrintfWithFlag("szSVCMaskData == NULL");
		}
		else
		{
			inDISP_LogPrintfWithFlag("szSVCMaskData length = (%d)", (int)strlen(szSVCMaskData));
		}
		return (VS_ERROR);
	}
    
	memcpy(&srSvcTableRec.szSVCMaskData[0], &szSVCMaskData[0], strlen(szSVCMaskData));

	return (VS_SUCCESS);
}



/*
Function	: inSVC_TABLE_EditTable
Date&Time	:2017/5/9 下午 4:21
Describe	:
*/
int inSVC_TABLE_EditTable(void)
{
	TABLE_GET_SET_TABLE SVC_TABLE_REC_FUNC_TABLE[] =
	{
		{"szSVCParameterVersion"	,inGetSVCParameterVersion		,inSetSVCParameterVersion			},
		{"szSVCParameterDateTime"	,inGetSVCParameterDateTime	,inSetSVCParameterDateTime		},
		{"szSVCMaskData"		,inGetSVCMaskData			,inSetSVCMaskData				},
		{""},
	};
	int	inRetVal;
	char	szKey;


	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont_Color("是否更改SVC_TAB", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
	inDISP_ChineseFont_Color("列印請按0 更改請按Enter", _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);
	inDISP_ChineseFont_Color("跳過請按取消鍵", _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);
	while (1)
	{
		szKey = uszKBD_GetKey(_EDC_TIMEOUT_);
		if (szKey == _KEY_0_)
		{
			inRetVal = VS_SUCCESS;
			/* 這裡放列印的Function */
			return(inRetVal);
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

	inFunc_Edit_Table_Tag(SVC_TABLE_REC_FUNC_TABLE);
	inSaveSvcTableRec(0);

	return	(VS_SUCCESS);
}


