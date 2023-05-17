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
#include "EDC.h"

static  EDC_REC srEDCRec;	/* construct EDC record */
extern  int	ginDebug;		/* Debug使用 extern */
/* 2018/5/23 下午 3:43
   VersionID和VersionDate改為寫死在程式中
 */

char		gszTermVersionID[16 + 1] = "TFB000_UW";	/* 暫存的TerminalVersionID，重開機要還原(測試導向功能) */
char		gszTermVersionDate[16 + 1] = "20221214";	/* 暫存的TerminalVersionDate，重開機要還原(測試導向功能) */


/*
Function		: inLoadEDCRec
Date&Time	: 2015/8/31 下午 2:00
Describe		: 讀EDC檔案，inEDCRec是要讀哪一筆的紀錄，第一筆為0
 */
int inLoadEDCRec(int inEDCRec)
{
	unsigned long ulFile_Handle; /* File Handle */
	unsigned char *uszReadData; /* 放抓到的record */
	unsigned char *uszTemp; /* 暫存，放整筆EDC檔案 */
	char szEDCRec[_SIZE_EDC_REC_ + 1]; /* 暫存, 放各個欄位檔案 */
//	char szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1]; /* debug message */
	long lnEDCLength = 0; /* EDC總長度 */
	long lnReadLength; /* 記錄每次要從EDC.dat讀多長的資料 */
	int i, k, j = 0, inRec = 0; /* inRec記錄讀到第幾筆, i為目前從EDC讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
	int inSearchResult = -1; /* 判斷有沒有讀到0x0D 0x0A的Flag */
	
	int inAutoSetValue = 0;

	/* inLoadEDCRec()_START */
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] inEDCRec[%d]  START -----",__FILE__, __FUNCTION__, __LINE__, inEDCRec);

	/* 判斷傳進來的inEDCRec是否小於零 大於等於零才是正確值(防呆) */
	if (inEDCRec < 0)
	{
		inDISP_LogPrintfWithFlag(" inEDCRec < 0:(index = %d) *Error* Line[%d]", inEDCRec, __LINE__);
		return (VS_ERROR);
	}

	/*
	 * open EDC.dat file
	 * open失敗時，if條件成立，關檔並回傳VS_ERROR
	 */
	if (inFILE_Open(&ulFile_Handle, (unsigned char *) _EDC_FILE_NAME_) == VS_ERROR)
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
	lnEDCLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *) _EDC_FILE_NAME_);

	if (lnEDCLength == VS_ERROR)
	{
		/* GetSize失敗 ，關檔 */
		inFILE_Close(&ulFile_Handle);
		inDISP_LogPrintfWithFlag(" EdcFile GetSize *Error* Line[%d] ", __LINE__);
		return (VS_ERROR);
	}

	/*
	 * allocate 記憶體
	 * allocate時多分配一個byte以防萬一（ex:換行符號）
	 */
	uszReadData = malloc(lnEDCLength + 1);
	uszTemp = malloc(lnEDCLength + 1);
	/* 初始化 uszTemp uszReadData */
	memset(uszReadData, 0x00, lnEDCLength + 1);
	memset(uszTemp, 0x00, lnEDCLength + 1);

	/* seek 到檔案開頭 & 從檔案開頭開始read */
	if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
	{
		lnReadLength = lnEDCLength;

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
					inDISP_DispLogAndWriteFlie(" EdcFile Read 1024 Bytes *Error* Line[%d] ", __LINE__);
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
					inDISP_DispLogAndWriteFlie(" EdcFile Read [%ld] Bytes *Error* Line[%d] ",lnReadLength, __LINE__);
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
		inDISP_DispLogAndWriteFlie(" EdcFile File Seek *Error* Line[%d] ",lnReadLength, __LINE__);
		/* Seek失敗，所以回傳Error */
		return (VS_ERROR);
	}

	/*
	 *抓取所需要的那筆record
	 *i為目前從EDC讀到的第幾個字元
	 *j為該record的長度
	 */
	j = 0;
	for (i = 0; i <= lnEDCLength; ++i) /* "<=" 是為了抓到最後一個0x00 */
	{
		/* 讀完一筆record或讀到EDC的結尾時  */
		if ((uszTemp[i] == 0x0D && uszTemp[i + 1] == 0x0A) || uszTemp[i] == 0x00)
		{
			/* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
			inSearchResult = 1;

			/* 清空uszReadData */
			memset(uszReadData, 0x00, lnEDCLength + 1);
			/* 把record從temp指定的位置截取出來放到uszReadData */
			memcpy(&uszReadData[0], &uszTemp[i - j], j);
			inRec++;
			/* 因為inEDC_Rec的index從0開始，所以inEDC_Rec要+1 */
			if (inRec == (inEDCRec + 1))
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
	if (inRec < (inEDCRec + 1) || inSearchResult == -1)
	{
		inDISP_DispLogAndWriteFlie(" EdcFile No data or Index[%d] *Error* Line[%d] ", inEDCRec,  __LINE__);

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
		inDISP_DispLogAndWriteFlie(" EdcFile No specific data *Error* Line[%d] ", __LINE__);

		/* 關檔 */
		inFILE_Close(&ulFile_Handle);

		/* Free pointer */
		free(uszReadData);
		free(uszTemp);

		return (VS_ERROR);
	}

	/* 結構初始化 */
	memset(&srEDCRec, 0x00, sizeof (srEDCRec));
	/*
	 * 以下pattern為存入EDC_Rec
	 * i為EDC的第幾個字元
	 * 存入EDC_Rec
	 */
	i = 0;


	/* 01_是否開啟銀聯 */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
			szEDCRec[k - 1] == 0x0D ||
			szEDCRec[k - 1] == 0x0A ||
			szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{

			inDISP_LogPrintfWithFlag("EDC unpack ERROR 1");

			/* 關檔 */
			inFILE_Close(&ulFile_Handle);

			/* Free pointer */
			free(uszReadData);
			free(uszTemp);

			return (VS_ERROR);
		}
	}

	/*  該筆有資料 */
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szCUPFuncEnable[0], &szEDCRec[0], k - 1);
	}

	/* 02_是否開啟SmartPay */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			inDISP_LogPrintfWithFlag("EDC unpack ERROR 2");

			/* 關檔 */
			inFILE_Close(&ulFile_Handle);

			/* Free pointer */
			free(uszReadData);
			free(uszTemp);

			return (VS_ERROR);
		}
	}

	/*  該筆有資料 */
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szFiscFuncEnable[0], &szEDCRec[0], k - 1);
	}

	/* 03_ECRComPort */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			inDISP_LogPrintfWithFlag("EDC unpack ERROR 3");

			/* 關檔 */
			inFILE_Close(&ulFile_Handle);

			/* Free pointer */
			free(uszReadData);
			free(uszTemp);

			return (VS_ERROR);
		}
	}

	/*  該筆有資料 */
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szECRComPort[0], &szEDCRec[0], k - 1);
	}

	/* 04_ECRVersion */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			inDISP_LogPrintfWithFlag("EDC unpack ERROR 4");

			/* 關檔 */
			inFILE_Close(&ulFile_Handle);

			/* Free pointer */
			free(uszReadData);
			free(uszTemp);

			return (VS_ERROR);
		}
	}

	/*  該筆有資料 */
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szECRVersion[0], &szEDCRec[0], k - 1);
	}

	/* 05_PABX Code */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			inDISP_LogPrintfWithFlag("EDC unpack ERROR 5");

			/* 關檔 */
			inFILE_Close(&ulFile_Handle);

			/* Free pointer */
			free(uszReadData);
			free(uszTemp);

			return (VS_ERROR);
		}
	}

	/*  該筆有資料 */
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szPABXCode[0], &szEDCRec[0], k - 1);
	}

	/* 06_TSAM RegisterEnable */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			inDISP_LogPrintfWithFlag("EDC unpack ERROR 6");
			/* 關檔 */
			inFILE_Close(&ulFile_Handle);

			/* Free pointer */
			free(uszReadData);
			free(uszTemp);

			return (VS_ERROR);
		}
	}

	/*  該筆有資料 */
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szTSAMRegisterEnable[0], &szEDCRec[0], k - 1);
	}

	/* 07_TMS下載是否成功 */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 7");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szTMSOK[0], &szEDCRec[0], k - 1);
	}

	/* 08_是否為新裝機 */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 8");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szDCCInit[0], &szEDCRec[0], k - 1);
	}

	/* 09_DCC下載模式 */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 9");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szDCCDownloadMode[0], &szEDCRec[0], k - 1);
	}

	/* 10_上次啟動DCC下載日期 */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 10");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szDCCLastUpdateDate[0], &szEDCRec[0], k - 1);
	}

	/* 11_結帳完是否要再做DCC下載 */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 11");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szDCCSettleDownload[0], &szEDCRec[0], k - 1);
	}

	/* 12_DCCBinTableVersion */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 12");
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
	if (szEDCRec[0] != 0x2C)
	{
		memcpy(&srEDCRec.szDCCBinVer[0], &szEDCRec[0], k - 1);
	}

	/* 13_EDC是否鎖機 */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 13");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szEDCLOCK[0], &szEDCRec[0], k - 1);
	}

	/* 14_Debug列印模式 */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 14");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szISODebug[0], &szEDCRec[0], k - 1);
	}

	/* 15_工程師管理密碼 */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 15");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szManagerPassword[0], &szEDCRec[0], k - 1);
	}

	/* 16_裝機工程師管理密碼 */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 16");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szFunctionPassword[0], &szEDCRec[0], k - 1);
	}

	/* 17_商店管理密碼 */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 17");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szMerchantPassword[0], &szEDCRec[0], k - 1);
	}

	/* 18_??管理密碼 */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 18");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szSuperPassword[0], &szEDCRec[0], k - 1);
	}

	/* 19_MCCCode */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 19");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szMCCCode[0], &szEDCRec[0], k - 1);
	}

	/* 20_IssuerID */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 20");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szIssuerID[0], &szEDCRec[0], k - 1);
	}

	/* 21_輸入逾時時間 */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 21");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szEnterTimeout[0], &szEDCRec[0], k - 1);
	}

	/* 22_IP Send TimeOut時間 */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 22");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szIPSendTimeout[0], &szEDCRec[0], k - 1);
	}

	/* 23_TermVersionID */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 23");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szTermVersionID[0], &szEDCRec[0], k - 1);
	}

	/* 24_TermVersionDate */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 24");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szTermVersionDate[0], &szEDCRec[0], k - 1);
	}

	/* 25_EDC IP */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 25");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szTermIPAddress[0], &szEDCRec[0], k - 1);
	}

	/* 26_TermGetewayAddress */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 26");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szTermGetewayAddress[0], &szEDCRec[0], k - 1);
	}

	/* 27_TermMASKAddress */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 27");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szTermMASKAddress[0], &szEDCRec[0], k - 1);
	}

	/* 28_TermECRPort */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 28");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szTermECRPort[0], &szEDCRec[0], k - 1);
	}

	/* 29 szESCMode */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 29");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szESCMode[0], &szEDCRec[0], k - 1);
	}

	/* 30_MultiComPort1 */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 30");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szMultiComPort1[0], &szEDCRec[0], k - 1);
	}

	/* 31 szMultiComPort1Version */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 31");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szMultiComPort1Version[0], &szEDCRec[0], k - 1);
	}

	/* 32 MultiComPort2 */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 32");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szMultiComPort2[0], &szEDCRec[0], k - 1);
	}

	/* 33 szMultiComPort2Version */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 33");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szMultiComPort2Version[0], &szEDCRec[0], k - 1);
	}

	/* 34 szMultiComPort3 */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 34");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szMultiComPort3[0], &szEDCRec[0], k - 1);
	}

	/* 35 szMultiComPort3Version */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 35");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szMultiComPort3Version[0], &szEDCRec[0], k - 1);
	}

	/* 36 szEMVForceOnline */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 36");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szEMVForceOnline[0], &szEDCRec[0], k - 1);
	}

	/* 37 szAutoConnect */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 37");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szAutoConnect[0], &szEDCRec[0], k - 1);
	}

	/* 38 szLOGONum */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 38");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szLOGONum[0], &szEDCRec[0], k - 1);
	}

	/* 39 szHostSAMSlot */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 39");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szHostSAMSlot[0], &szEDCRec[0], k - 1);
	}

	/* 40 szSAMSlotSN1 */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 40");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szSAMSlotSN1[0], &szEDCRec[0], k - 1);
	}

	/* 41 szSAMSlotSN2 */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 41");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szSAMSlotSN2[0], &szEDCRec[0], k - 1);
	}

	/* 42 szSAMSlotSN3 */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 42");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szSAMSlotSN3[0], &szEDCRec[0], k - 1);
	}

	/* 43 szSAMSlotSN4 */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 43");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szSAMSlotSN4[0], &szEDCRec[0], k - 1);
	}

	/* 44 szPWMEnable */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 44");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szPWMEnable[0], &szEDCRec[0], k - 1);
	}

	/* 45 szPWMMode */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 45");
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
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szPWMMode[0], &szEDCRec[0], k - 1);
	}

	/* 46 szPWMIdleTimeout */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 46");
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
	/* 因為是最後的欄位還要多判斷0x00 */
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szPWMIdleTimeout[0], &szEDCRec[0], k - 1);
	}

	/* 47 szDemoMode */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 47");
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
	/* 因為是最後的欄位還要多判斷0x00 */
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szDemoMode[0], &szEDCRec[0], k - 1);
	}

	/* 48 szTMKOK */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 48");
			}

			/* 關檔 */
			inFILE_Close(&ulFile_Handle);

			/* Free pointer */
			free(uszReadData);
			free(uszTemp);

			return (VS_ERROR);
		}
	}

	/*	該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szTMKOK[0], &szEDCRec[0], k - 1);
	}

	/* 49 szCUPTMKOK */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 49");
			}

			/* 關檔 */
			inFILE_Close(&ulFile_Handle);

			/* Free pointer */
			free(uszReadData);
			free(uszTemp);

			return (VS_ERROR);
		}
	}

	/*該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szCUPTMKOK[0], &szEDCRec[0], k - 1);
	}

	/* 50 szCUPTPKOK */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 50");
			}

			/* 關檔 */
			inFILE_Close(&ulFile_Handle);

			/* Free pointer */
			free(uszReadData);
			free(uszTemp);

			return (VS_ERROR);
		}
	}

	/*	該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szCUPTPKOK[0], &szEDCRec[0], k - 1);
	}

	/* 51 szCUPLOGONOK */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 51");
				inDISP_LogPrintfArea(FALSE, "", 0, uszReadData, lnEDCLength);
			}

			/* 關檔 */
			inFILE_Close(&ulFile_Handle);

			/* Free pointer */
			free(uszReadData);
			free(uszTemp);

			return (VS_ERROR);
		}
	}

	/* 該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szCUPLOGONOK[0], &szEDCRec[0], k - 1);
	}

	/* 52 szCMASFuncEnable */
	/* 初始化 */
	memset(szEDCRec, 0x00, sizeof (szEDCRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szEDCRec[k++] = uszReadData[i++];
		if (szEDCRec[k - 1] == 0x2C ||
			szEDCRec[k - 1] == 0x0D ||
			szEDCRec[k - 1] == 0x0A ||
			szEDCRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnEDCLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("EDC unpack ERROR 52");
				inDISP_LogPrintfArea(FALSE, "", 0, uszReadData, lnEDCLength);
			}

			/* 關檔 */
			inFILE_Close(&ulFile_Handle);

			/* Free pointer */
			free(uszReadData);
			free(uszTemp);

			return (VS_ERROR);
		}
	}

	/*	該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
	if (szEDCRec[0] != 0x2C &&
			szEDCRec[0] != 0x0D &&
			szEDCRec[0] != 0x0A &&
			szEDCRec[0] != 0x00)
	{
		memcpy(&srEDCRec.szCMASFuncEnable[0], &szEDCRec[0], k - 1);
	}else{
		/* 因為舊版的檔案會少資料，所以讀到新增的資料讀不到時，就自動補資料 2022/11/9 [SAM] */
		inAutoSetValue = 1;
	}
	
	/* 因為舊版的檔案會少資料，所以讀到新增的資料讀不到時，就自動補資料 2022/11/9 [SAM] */
	if(inAutoSetValue == 1)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("EDC unpack szCMASFuncEnable Auto Set Value ");
		}
		srEDCRec.szCMASFuncEnable[0] = 'N';
		srEDCRec.szCMASLinkMode[0] = '0';
		srEDCRec.szCMASAddTcpipMode[0] = '0';

	}else
	{
		/* 53 szCMASLinkMode */
		/* 初始化 */
		memset(szEDCRec, 0x00, sizeof (szEDCRec));
		k = 0;

		/* 從Record中讀欄位資料出來 */
		while (1)
		{
			szEDCRec[k++] = uszReadData[i++];
			if (szEDCRec[k - 1] == 0x2C ||
					szEDCRec[k - 1] == 0x0D ||
					szEDCRec[k - 1] == 0x0A ||
					szEDCRec[k - 1] == 0x00)
			{
				break;
			}

			if (i > lnEDCLength)
			{
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf("EDC unpack ERROR 53");
					inDISP_LogPrintfArea(FALSE, "", 0, uszReadData, lnEDCLength);
				}

				/* 關檔 */
				inFILE_Close(&ulFile_Handle);

				/* Free pointer */
				free(uszReadData);
				free(uszTemp);

				return (VS_ERROR);
			}
		}

		/*	該筆有資料 */
		/* 因為是最後的欄位還要多判斷0x00 */
		if (szEDCRec[0] != 0x2C &&
				szEDCRec[0] != 0x0D &&
				szEDCRec[0] != 0x0A &&
				szEDCRec[0] != 0x00)
		{
			memcpy(&srEDCRec.szCMASLinkMode[0], &szEDCRec[0], k - 1);
		}

		/* 54 szCMASAddTcpipMode */
		/* 初始化 */
		memset(szEDCRec, 0x00, sizeof (szEDCRec));
		k = 0;

		/* 從Record中讀欄位資料出來 */
		while (1)
		{
			szEDCRec[k++] = uszReadData[i++];
			if (szEDCRec[k - 1] == 0x2C ||
				szEDCRec[k - 1] == 0x0D ||
				szEDCRec[k - 1] == 0x0A ||
				szEDCRec[k - 1] == 0x00)
			{
				break;
			}

			if (i > lnEDCLength)
			{
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf("EDC unpack ERROR 54");
					inDISP_LogPrintfArea(FALSE, "", 0, uszReadData, lnEDCLength);
				}

				/* 關檔 */
				inFILE_Close(&ulFile_Handle);

				/* Free pointer */
				free(uszReadData);
				free(uszTemp);

				return (VS_ERROR);
			}
		}

		/*	該筆有資料 */
		/* 因為是最後的欄位還要多判斷0x00 */
		if (szEDCRec[0] != 0x2C &&
				szEDCRec[0] != 0x0D &&
				szEDCRec[0] != 0x0A &&
				szEDCRec[0] != 0x00)
		{
			memcpy(&srEDCRec.szCMASAddTcpipMode[0], &szEDCRec[0], k - 1);
		}
			
	}

	/* release */
	/* 關檔 */
	inFILE_Close(&ulFile_Handle);

	/* Free pointer */
	free(uszReadData);
	free(uszTemp);

	/* inLoadEDCRec() END */
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] inEDCRec[%d]  END -----",__FILE__, __FUNCTION__, __LINE__, inEDCRec);

	return (VS_SUCCESS);
}

/*
Function		: inSaveEDCRec
Date&Time	: 2015/8/31 下午 2:00
Describe		: 寫入EDC.dat，inEDCRec是要讀哪一筆的紀錄，第一筆為0
*/
int inSaveEDCRec(int inEDCRec)
{
	unsigned long   uldat_Handle;   			/* FILE Handle */
	int inRecCount = 0, inTempIndex = 0;
	int     	inRetVal;
	int     	i = 0;

	char szTempTitle[] = "inSaveEDCRec";
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
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inSaveEDCRec INIT" );
#endif
		
	inTempIndex = inEDCRec;
	
	memset(szTempFIleName, 0x00, sizeof(szTempFIleName));
	strcat( szTempFIleName, _EDC_FILE_NAME_);
	
	memset(szTempBakFileName, 0x00, sizeof(szTempBakFileName));
	strcat( szTempBakFileName, _EDC_FILE_NAME_BAK_);
	
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
	uszWriteBuff_Record = malloc(_SIZE_EDC_REC_ + _SIZE_EDC_COMMA_0D0A_);
	memset(uszWriteBuff_Record, 0x00, _SIZE_EDC_REC_ + _SIZE_EDC_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

	/* uszRead_Total_Buff儲存EDC.dat全部資料 */
	uszRead_Total_Buff = malloc(ulFileSize + 1);
	memset(uszRead_Total_Buff, 0x00, ulFileSize + 1);

	ulPackCount = 0; /* uszWriteBuff_Record的index位置 */
	/* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* 1 CUPFuncEnable */
	memcpy(&uszWriteBuff_Record[0], &srEDCRec.szCUPFuncEnable[0], strlen(srEDCRec.szCUPFuncEnable));
	ulPackCount += strlen(srEDCRec.szCUPFuncEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 2 FiscFuncEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szFiscFuncEnable[0], strlen(srEDCRec.szFiscFuncEnable));
	ulPackCount += strlen(srEDCRec.szFiscFuncEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 3 szECRComPort */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szECRComPort[0], strlen(srEDCRec.szECRComPort));
	ulPackCount += strlen(srEDCRec.szECRComPort);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 4 ECRVersion */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szECRVersion[0], strlen(srEDCRec.szECRVersion));
	ulPackCount += strlen(srEDCRec.szECRVersion);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 5 PABXCODE */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szPABXCode[0], strlen(srEDCRec.szPABXCode));
	ulPackCount += strlen(srEDCRec.szPABXCode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 6 TSAMRegisterEnable*/
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szTSAMRegisterEnable[0], strlen(srEDCRec.szTSAMRegisterEnable));
	ulPackCount += strlen(srEDCRec.szTSAMRegisterEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 7 TMSOK */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szTMSOK[0], strlen(srEDCRec.szTMSOK));
	ulPackCount += strlen(srEDCRec.szTMSOK);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 8 DCCInit */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szDCCInit[0], strlen(srEDCRec.szDCCInit));
	ulPackCount += strlen(srEDCRec.szDCCInit);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 9 DCCDownloadMode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szDCCDownloadMode[0], strlen(srEDCRec.szDCCDownloadMode));
	ulPackCount += strlen(srEDCRec.szDCCDownloadMode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 10 DCCLastUpdateDate */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szDCCLastUpdateDate[0], strlen(srEDCRec.szDCCLastUpdateDate));
	ulPackCount += strlen(srEDCRec.szDCCLastUpdateDate);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 11 DCCSettleDownload */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szDCCSettleDownload[0], strlen(srEDCRec.szDCCSettleDownload));
	ulPackCount += strlen(srEDCRec.szDCCSettleDownload);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 12 DCCBinTableVersion */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szDCCBinVer[0], strlen(srEDCRec.szDCCBinVer));
	ulPackCount += strlen(srEDCRec.szDCCBinVer);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 13 EDCLOCK */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szEDCLOCK[0], strlen(srEDCRec.szEDCLOCK));
	ulPackCount += strlen(srEDCRec.szEDCLOCK);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 14 ISODebug */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szISODebug[0], strlen(srEDCRec.szISODebug));
	ulPackCount += strlen(srEDCRec.szISODebug);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 15 szManagerPassword */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szManagerPassword[0], strlen(srEDCRec.szManagerPassword));
	ulPackCount += strlen(srEDCRec.szManagerPassword);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 16 szFunctionPassword */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szFunctionPassword[0], strlen(srEDCRec.szFunctionPassword));
	ulPackCount += strlen(srEDCRec.szFunctionPassword);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 17 szMerchantPassword */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szMerchantPassword[0], strlen(srEDCRec.szMerchantPassword));
	ulPackCount += strlen(srEDCRec.szMerchantPassword);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 18 szSuperPassword */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szSuperPassword[0], strlen(srEDCRec.szSuperPassword));
	ulPackCount += strlen(srEDCRec.szSuperPassword);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 19 MCCCode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szMCCCode[0], strlen(srEDCRec.szMCCCode));
	ulPackCount += strlen(srEDCRec.szMCCCode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 20 IssuerID */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szIssuerID[0], strlen(srEDCRec.szIssuerID));
	ulPackCount += strlen(srEDCRec.szIssuerID);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 21 EnterTimeout */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szEnterTimeout[0], strlen(srEDCRec.szEnterTimeout));
	ulPackCount += strlen(srEDCRec.szEnterTimeout);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 22 IPSendTimeout */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szIPSendTimeout[0], strlen(srEDCRec.szIPSendTimeout));
	ulPackCount += strlen(srEDCRec.szIPSendTimeout);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 23 TermVersionID */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szTermVersionID[0], strlen(srEDCRec.szTermVersionID));
	ulPackCount += strlen(srEDCRec.szTermVersionID);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 24 TermVersionDate */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szTermVersionDate[0], strlen(srEDCRec.szTermVersionDate));
	ulPackCount += strlen(srEDCRec.szTermVersionDate);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 25 TermIPAddress */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szTermIPAddress[0], strlen(srEDCRec.szTermIPAddress));
	ulPackCount += strlen(srEDCRec.szTermIPAddress);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 26 TermGetewayAddress */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szTermGetewayAddress[0], strlen(srEDCRec.szTermGetewayAddress));
	ulPackCount += strlen(srEDCRec.szTermGetewayAddress);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 27 TermMASKAddress */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szTermMASKAddress[0], strlen(srEDCRec.szTermMASKAddress));
	ulPackCount += strlen(srEDCRec.szTermMASKAddress);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 28 TermECRPort */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szTermECRPort[0], strlen(srEDCRec.szTermECRPort));
	ulPackCount += strlen(srEDCRec.szTermECRPort);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 29 ESCMode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szESCMode[0], strlen(srEDCRec.szESCMode));
	ulPackCount += strlen(srEDCRec.szESCMode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 30 szMultiComPort1 */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szMultiComPort1[0], strlen(srEDCRec.szMultiComPort1));
	ulPackCount += strlen(srEDCRec.szMultiComPort1);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 31 szMultiComPort1Version */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szMultiComPort1Version[0], strlen(srEDCRec.szMultiComPort1Version));
	ulPackCount += strlen(srEDCRec.szMultiComPort1Version);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 32 szMultiComPort2 */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szMultiComPort2[0], strlen(srEDCRec.szMultiComPort2));
	ulPackCount += strlen(srEDCRec.szMultiComPort2);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 33 szMultiComPort2Version */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szMultiComPort2Version[0], strlen(srEDCRec.szMultiComPort2Version));
	ulPackCount += strlen(srEDCRec.szMultiComPort2Version);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 34 szMultiComPort3 */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szMultiComPort3[0], strlen(srEDCRec.szMultiComPort3));
	ulPackCount += strlen(srEDCRec.szMultiComPort3);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 35 szMultiComPort3Version */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szMultiComPort3Version[0], strlen(srEDCRec.szMultiComPort3Version));
	ulPackCount += strlen(srEDCRec.szMultiComPort3Version);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 36 szEMVForceOnline */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szEMVForceOnline[0], strlen(srEDCRec.szEMVForceOnline));
	ulPackCount += strlen(srEDCRec.szEMVForceOnline);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 37 szAutoConnect */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szAutoConnect[0], strlen(srEDCRec.szAutoConnect));
	ulPackCount += strlen(srEDCRec.szAutoConnect);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 38 szLOGONum */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szLOGONum[0], strlen(srEDCRec.szLOGONum));
	ulPackCount += strlen(srEDCRec.szLOGONum);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 39 szHostSAMSlot */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szHostSAMSlot[0], strlen(srEDCRec.szHostSAMSlot));
	ulPackCount += strlen(srEDCRec.szHostSAMSlot);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 40 szSAMSlotSN1 */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szSAMSlotSN1[0], strlen(srEDCRec.szSAMSlotSN1));
	ulPackCount += strlen(srEDCRec.szSAMSlotSN1);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 41 szSAMSlotSN2 */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szSAMSlotSN2[0], strlen(srEDCRec.szSAMSlotSN2));
	ulPackCount += strlen(srEDCRec.szSAMSlotSN2);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 42 szSAMSlotSN3 */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szSAMSlotSN3[0], strlen(srEDCRec.szSAMSlotSN3));
	ulPackCount += strlen(srEDCRec.szSAMSlotSN3);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 43 szSAMSlotSN4 */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szSAMSlotSN4[0], strlen(srEDCRec.szSAMSlotSN4));
	ulPackCount += strlen(srEDCRec.szSAMSlotSN4);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 44 szPWMEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szPWMEnable[0], strlen(srEDCRec.szPWMEnable));
	ulPackCount += strlen(srEDCRec.szPWMEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 45 szPWMMode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szPWMMode[0], strlen(srEDCRec.szPWMMode));
	ulPackCount += strlen(srEDCRec.szPWMMode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 46 szPWMIdleTimeout */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szPWMIdleTimeout[0], strlen(srEDCRec.szPWMIdleTimeout));
	ulPackCount += strlen(srEDCRec.szPWMIdleTimeout);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 47 szDemoMode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szDemoMode[0], strlen(srEDCRec.szDemoMode));
	ulPackCount += strlen(srEDCRec.szDemoMode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 48 szTMKOK */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szTMKOK[0], strlen(srEDCRec.szTMKOK));
	ulPackCount += strlen(srEDCRec.szTMKOK);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 49 szCUPTMKOK */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szCUPTMKOK[0], strlen(srEDCRec.szCUPTMKOK));
	ulPackCount += strlen(srEDCRec.szCUPTMKOK);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 50 szCUPTPKOK */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szCUPTPKOK[0], strlen(srEDCRec.szCUPTPKOK));
	ulPackCount += strlen(srEDCRec.szCUPTPKOK);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 51 szCUPLOGONOK */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szCUPLOGONOK[0], strlen(srEDCRec.szCUPLOGONOK));
	ulPackCount += strlen(srEDCRec.szCUPLOGONOK);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;

	/* 52 szCMASFuncEnable */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szCMASFuncEnable[0], strlen(srEDCRec.szCMASFuncEnable));
	ulPackCount += strlen(srEDCRec.szCMASFuncEnable);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;
	
	/* 53 szCMASLinkMode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szCMASLinkMode[0], strlen(srEDCRec.szCMASLinkMode));
	ulPackCount += strlen(srEDCRec.szCMASLinkMode);
	uszWriteBuff_Record[ulPackCount] = 0x2C;
	ulPackCount++;
	
	/* 54 szCMASAddTcpipMode */
	memcpy(&uszWriteBuff_Record[ulPackCount], &srEDCRec.szCMASAddTcpipMode[0], strlen(srEDCRec.szCMASAddTcpipMode));
	ulPackCount += strlen(srEDCRec.szCMASAddTcpipMode);
	
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

#ifdef __COUNT_FUNC_TIME__
	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inSaveEDCRec END");
#endif
	
	inDISP_LogPrintfWithFlag( " TempIndex[%d] Line[%d] ", inTempIndex, __LINE__ );	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	

	return(VS_SUCCESS);
}

/*
set和get等價於相反的操作
各欄位的set和get function
*/

/*
Function        :inGetCUPFuncEnable
Date&Time       :
Describe        :
*/
int inGetCUPFuncEnable(char* szCUPFuncEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCUPFuncEnable == NULL || strlen(srEDCRec.szCUPFuncEnable) <= 0 || strlen(srEDCRec.szCUPFuncEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szCUPFuncEnable == NULL)
                        {
                                inDISP_LogPrintf("inGetCUPFuncEnable() ERROR !! szCUPFuncEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inGetCUPFuncEnable() ERROR !! szCUPFuncEnable length = (%d)", (int)strlen(srEDCRec.szCUPFuncEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCUPFuncEnable[0], &srEDCRec.szCUPFuncEnable[0], strlen(srEDCRec.szCUPFuncEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetCUPFuncEnable
Date&Time       :
Describe        :
*/
int inSetCUPFuncEnable(char* szCUPFuncEnable)
{
        memset(srEDCRec.szCUPFuncEnable, 0x00, sizeof(srEDCRec.szCUPFuncEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCUPFuncEnable == NULL || strlen(szCUPFuncEnable) <= 0 || strlen(szCUPFuncEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetCUPFuncEnable() ERROR !!");

                        if (szCUPFuncEnable == NULL)
                        {
                                inDISP_LogPrintf("szCUPFuncEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCUPFuncEnable length = (%d)", (int)strlen(szCUPFuncEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szCUPFuncEnable[0], &szCUPFuncEnable[0], strlen(szCUPFuncEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetFiscFuncEnable
Date&Time       :2016/11/25 下午 12:00
Describe        :是否開啟SmartPay開關
*/
int inGetFiscFuncEnable(char* szFiscFuncEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szFiscFuncEnable == NULL || strlen(srEDCRec.szFiscFuncEnable) <= 0 || strlen(srEDCRec.szFiscFuncEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szFiscFuncEnable == NULL)
                        {
                                inDISP_LogPrintf("inGetFiscFuncEnable() ERROR !! szFiscFuncEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inGetFiscFuncEnable() ERROR !! szFiscFuncEnable length = (%d)", (int)strlen(srEDCRec.szFiscFuncEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFiscFuncEnable[0], &srEDCRec.szFiscFuncEnable[0], strlen(srEDCRec.szFiscFuncEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetFiscFuncEnable
Date&Time       :2016/11/25 下午 12:00
Describe        :是否開啟SmartPay開關
*/
int inSetFiscFuncEnable(char* szFiscFuncEnable)
{
        memset(srEDCRec.szFiscFuncEnable, 0x00, sizeof(srEDCRec.szFiscFuncEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szFiscFuncEnable == NULL || strlen(szFiscFuncEnable) <= 0 || strlen(szFiscFuncEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetFiscFuncEnable() ERROR !!");

                        if (szFiscFuncEnable == NULL)
                        {
                                inDISP_LogPrintf("szFiscFuncEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFiscFuncEnable length = (%d)", (int)strlen(szFiscFuncEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szFiscFuncEnable[0], &szFiscFuncEnable[0], strlen(szFiscFuncEnable));

        return (VS_SUCCESS);
}


/*
Function        :inGetECRComPort
Date&Time       :
Describe        :
*/
int inGetECRComPort(char* szECRComPort)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szECRComPort == NULL || strlen(srEDCRec.szECRComPort) <= 0 || strlen(srEDCRec.szECRComPort) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szECRComPort == NULL)
                        {
                                inDISP_LogPrintf("inGetECRComPort() ERROR !! szECRComPort == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inGetECRComPort() ERROR !! szECRComPort length = (%d)", (int)strlen(srEDCRec.szECRComPort));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szECRComPort[0], &srEDCRec.szECRComPort[0], strlen(srEDCRec.szECRComPort));

        return (VS_SUCCESS);
}

/*
Function        :inSetECRComPort
Date&Time       :
Describe        :
*/
int inSetECRComPort(char* szECRComPort)
{
        memset(srEDCRec.szECRComPort, 0x00, sizeof(srEDCRec.szECRComPort));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szECRComPort == NULL || strlen(szECRComPort) <= 0 || strlen(szECRComPort) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetECRComPort() ERROR !!");

                        if (szECRComPort == NULL)
                        {
                                inDISP_LogPrintf("szECRComPort == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECRComPort length = (%d)", (int)strlen(szECRComPort));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szECRComPort[0], &szECRComPort[0], strlen(szECRComPort));

        return (VS_SUCCESS);
}

/*
Function        :inGetECRVersion
Date&Time       :
Describe        :
*/
int inGetECRVersion(char* szECRVersion)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szECRVersion == NULL || strlen(srEDCRec.szECRVersion) <= 0 || strlen(srEDCRec.szECRVersion) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szECRVersion == NULL)
                        {
                                inDISP_LogPrintf("inGetECRVersion() ERROR !! szECRVersion == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inGetECRVersion() ERROR !! szECRVersion length = (%d)", (int)strlen(srEDCRec.szECRVersion));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szECRVersion[0], &srEDCRec.szECRVersion[0], strlen(srEDCRec.szECRVersion));
	
        return (VS_SUCCESS);
}

/*
Function        :inSetECRVersion
Date&Time       :
Describe        :
*/
int inSetECRVersion(char* szECRVersion)
{
        memset(srEDCRec.szECRVersion, 0x00, sizeof(srEDCRec.szECRVersion));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szECRVersion == NULL || strlen(szECRVersion) <= 0 || strlen(szECRVersion) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetECRVersion() ERROR !!");

                        if (szECRVersion == NULL)
                        {
                                inDISP_LogPrintf("szECRVersion == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECRVersion length = (%d)", (int)strlen(szECRVersion));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szECRVersion[0], &szECRVersion[0], strlen(szECRVersion));

        return (VS_SUCCESS);
}

/*
Function        :inGetPABXCode
Date&Time       :
Describe        :
*/
int inGetPABXCode(char* szPABXCode)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPABXCode == NULL || strlen(srEDCRec.szPABXCode) <= 0 || strlen(srEDCRec.szPABXCode) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szPABXCode == NULL)
                        {
                                inDISP_LogPrintf("inGetPABXCode() ERROR !! szPABXCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inGetPABXCode() ERROR !! szPABXCode length = (%d)", (int)strlen(srEDCRec.szPABXCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPABXCode[0], &srEDCRec.szPABXCode[0], strlen(srEDCRec.szPABXCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetPABXCode
Date&Time       :
Describe        :
*/
int inSetPABXCode(char* szPABXCode)
{
        memset(srEDCRec.szPABXCode, 0x00, sizeof(srEDCRec.szPABXCode));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPABXCode == NULL || strlen(szPABXCode) <= 0 || strlen(szPABXCode) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPABXCode() ERROR !!");

                        if (szPABXCode == NULL)
                        {
                                inDISP_LogPrintf("szPABXCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPABXCode length = (%d)", (int)strlen(szPABXCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szPABXCode[0], &szPABXCode[0], strlen(szPABXCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetTSAMRegisterEnable
Date&Time       :
Describe        :
*/
int inGetTSAMRegisterEnable(char* szTSAMRegisterEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTSAMRegisterEnable == NULL || strlen(srEDCRec.szTSAMRegisterEnable) <= 0 || strlen(srEDCRec.szTSAMRegisterEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szTSAMRegisterEnable == NULL)
                        {
                                inDISP_LogPrintf("inGetTSAMRegisterEnable() ERROR !! szTSAMRegisterEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inGetTSAMRegisterEnable() ERROR !! szTSAMRegisterEnable length = (%d)", (int)strlen(srEDCRec.szTSAMRegisterEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTSAMRegisterEnable[0], &srEDCRec.szTSAMRegisterEnable[0], strlen(srEDCRec.szTSAMRegisterEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetTSAMRegisterEnable
Date&Time       :
Describe        :
*/
int inSetTSAMRegisterEnable(char* szTSAMRegisterEnable)
{
        memset(srEDCRec.szTSAMRegisterEnable, 0x00, sizeof(srEDCRec.szTSAMRegisterEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTSAMRegisterEnable == NULL || strlen(szTSAMRegisterEnable) <= 0 || strlen(szTSAMRegisterEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTSAMRegisterEnable() ERROR !!");

                        if (szTSAMRegisterEnable == NULL)
                        {
                                inDISP_LogPrintf("szTSAMRegisterEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTSAMRegisterEnable length = (%d)", (int)strlen(szTSAMRegisterEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szTSAMRegisterEnable[0], &szTSAMRegisterEnable[0], strlen(szTSAMRegisterEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSOK
Date&Time       :
Describe        :
*/
int inGetTMSOK(char* szTMSOK)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTMSOK == NULL || strlen(srEDCRec.szTMSOK) <= 0 || strlen(srEDCRec.szTMSOK) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTMSOK() ERROR !!");

                        if (szTMSOK == NULL)
                        {
                                inDISP_LogPrintf("szTMSOK == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSOK length = (%d)", (int)strlen(srEDCRec.szTMSOK));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&szTMSOK[0], &srEDCRec.szTMSOK[0], strlen(srEDCRec.szTMSOK));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSOK
Date&Time       :
Describe        :
*/
int inSetTMSOK(char* szTMSOK)
{
        memset(srEDCRec.szTMSOK, 0x00, sizeof(srEDCRec.szTMSOK));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTMSOK == NULL || strlen(szTMSOK) <= 0 || strlen(szTMSOK) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTMSOK() ERROR !!");

                        if (szTMSOK == NULL)
                        {
                                inDISP_LogPrintf("szTMSOK == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSOK length = (%d)", (int)strlen(szTMSOK));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szTMSOK[0], &szTMSOK[0], strlen(szTMSOK));

        return (VS_SUCCESS);
}

/*
Function        :inGetDCCInit
Date&Time       :2016/10/4 下午 1:53
Describe        :
*/
int inGetDCCInit(char* szDCCInit)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDCCInit == NULL || strlen(srEDCRec.szDCCInit) <= 0 || strlen(srEDCRec.szDCCInit) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetDCCFullDownload() ERROR !!");

                        if (szDCCInit == NULL)
                        {
                                inDISP_LogPrintf("szDCCFullDownload == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCFullDownload length = (%d)", (int)strlen(srEDCRec.szDCCInit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&szDCCInit[0], &srEDCRec.szDCCInit[0], strlen(srEDCRec.szDCCInit));

        return (VS_SUCCESS);
}

/*
Function        :inSetDCCInit
Date&Time       :2016/10/4 下午 1:53
Describe        :
*/
int inSetDCCInit(char* szDCCInit)
{
        memset(srEDCRec.szDCCInit, 0x00, sizeof(srEDCRec.szDCCInit));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDCCInit == NULL || strlen(szDCCInit) <= 0 || strlen(szDCCInit) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetDCCFullDownload() ERROR !!");

                        if (szDCCInit == NULL)
                        {
                                inDISP_LogPrintf("szDCCFullDownload == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCFullDownload length = (%d)", (int)strlen(szDCCInit));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szDCCInit[0], &szDCCInit[0], strlen(szDCCInit));

        return (VS_SUCCESS);
}

/*
Function        :inGetDCCDownloadMode
Date&Time       :2016/10/20 下午 12:02
Describe        :
*/
int inGetDCCDownloadMode(char* szDCCDownloadMode)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDCCDownloadMode == NULL || strlen(srEDCRec.szDCCDownloadMode) <= 0 || strlen(srEDCRec.szDCCDownloadMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetDCCFullDownload() ERROR !!");

                        if (szDCCDownloadMode == NULL)
                        {
                                inDISP_LogPrintf("szDCCFullDownload == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCFullDownload length = (%d)", (int)strlen(srEDCRec.szDCCDownloadMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&szDCCDownloadMode[0], &srEDCRec.szDCCDownloadMode[0], strlen(srEDCRec.szDCCDownloadMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetDCCDownloadMode
Date&Time       :2016/10/20 下午 12:02
Describe        :
*/
int inSetDCCDownloadMode(char* szDCCDownloadMode)
{
        memset(srEDCRec.szDCCDownloadMode, 0x00, sizeof(srEDCRec.szDCCDownloadMode));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDCCDownloadMode == NULL || strlen(szDCCDownloadMode) <= 0 || strlen(szDCCDownloadMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetDCCFullDownload() ERROR !!");

                        if (szDCCDownloadMode == NULL)
                        {
                                inDISP_LogPrintf("szDCCFullDownload == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCFullDownload length = (%d)", (int)strlen(szDCCDownloadMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szDCCDownloadMode[0], &szDCCDownloadMode[0], strlen(szDCCDownloadMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetDCCLastUpdateDate
Date&Time       :2016/10/4 下午 1:53
Describe        :
*/
int inGetDCCLastUpdateDate(char* szDCCLastUpdateDate)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDCCLastUpdateDate == NULL || strlen(srEDCRec.szDCCLastUpdateDate) <= 0 || strlen(srEDCRec.szDCCLastUpdateDate) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetDCCFullDownload() ERROR !!");

                        if (szDCCLastUpdateDate == NULL)
                        {
                                inDISP_LogPrintf("szDCCFullDownload == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCFullDownload length = (%d)", (int)strlen(srEDCRec.szDCCLastUpdateDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&szDCCLastUpdateDate[0], &srEDCRec.szDCCLastUpdateDate[0], strlen(srEDCRec.szDCCLastUpdateDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetDCCLastUpdateDate
Date&Time       :2016/10/4 下午 1:53
Describe        :
*/
int inSetDCCLastUpdateDate(char* szDCCLastUpdateDate)
{
        memset(srEDCRec.szDCCLastUpdateDate, 0x00, sizeof(srEDCRec.szDCCLastUpdateDate));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDCCLastUpdateDate == NULL || strlen(szDCCLastUpdateDate) <= 0 || strlen(szDCCLastUpdateDate) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetDCCFullDownload() ERROR !!");

                        if (szDCCLastUpdateDate == NULL)
                        {
                                inDISP_LogPrintf("szDCCFullDownload == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCFullDownload length = (%d)", (int)strlen(szDCCLastUpdateDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szDCCLastUpdateDate[0], &szDCCLastUpdateDate[0], strlen(szDCCLastUpdateDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetDCCSettleDownload
Date&Time       :2016/10/20 下午 4:47
Describe        :
*/
int inGetDCCSettleDownload(char* szDCCSettleDownload)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDCCSettleDownload == NULL || strlen(srEDCRec.szDCCSettleDownload) <= 0 || strlen(srEDCRec.szDCCSettleDownload) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetDCCFullDownload() ERROR !!");

                        if (szDCCSettleDownload == NULL)
                        {
                                inDISP_LogPrintf("szDCCFullDownload == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCFullDownload length = (%d)", (int)strlen(srEDCRec.szDCCSettleDownload));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&szDCCSettleDownload[0], &srEDCRec.szDCCSettleDownload[0], strlen(srEDCRec.szDCCSettleDownload));

        return (VS_SUCCESS);
}

/*
Function        :inSetDCCSettleDownload
Date&Time       :2016/10/20 下午 4:47
Describe        :
*/
int inSetDCCSettleDownload(char* szDCCSettleDownload)
{
        memset(srEDCRec.szDCCSettleDownload, 0x00, sizeof(srEDCRec.szDCCSettleDownload));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
	/* Modify szDCCSettleDownload condition 20190220 [SAM] */
        if (szDCCSettleDownload == NULL || strlen(szDCCSettleDownload) <= 0 || strlen(szDCCSettleDownload) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetDCCFullDownload() ERROR !!");

                        if (szDCCSettleDownload == NULL)
                        {
                                inDISP_LogPrintf("szDCCFullDownload == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCFullDownload length = (%d)", (int)strlen(szDCCSettleDownload));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szDCCSettleDownload[0], &szDCCSettleDownload[0], strlen(szDCCSettleDownload));

        return (VS_SUCCESS);
}

/*
Function        :inGetDCCBinVer
Date&Time       :2016/10/20 上午 11:57
Describe        :
*/
int inGetDCCBinVer(char* szDCCBinVer)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDCCBinVer == NULL || strlen(srEDCRec.szDCCBinVer) <=0 || strlen(srEDCRec.szDCCBinVer) > 4)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetDCCBinVer() ERROR !!");

                        if (szDCCBinVer == NULL)
                        {
                                inDISP_LogPrintf("szDCCBinVer == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCBinVer length = (%d)", (int)strlen(srEDCRec.szDCCBinVer));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szDCCBinVer[0], &srEDCRec.szDCCBinVer[0], strlen(srEDCRec.szDCCBinVer));

        return (VS_SUCCESS);
}

/*
Function        :inSetDCCBinVer
Date&Time       :2016/10/20 上午 11:57
Describe        :
*/
int inSetDCCBinVer(char* szDCCBinVer)
{
        memset(srEDCRec.szDCCBinVer, 0x00, sizeof(srEDCRec.szDCCBinVer));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDCCBinVer == NULL || strlen(szDCCBinVer) <= 0 || strlen(szDCCBinVer) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetDCCBinVer() ERROR !!");

                        if (szDCCBinVer == NULL)
                        {
                                inDISP_LogPrintf("szDCCBinVer == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCBinVer length = (%d)", (int)strlen(szDCCBinVer));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szDCCBinVer[0], &szDCCBinVer[0], strlen(szDCCBinVer));

        return (VS_SUCCESS);
}

/*
Function        :inGetEDCLOCK
Date&Time       :
Describe        :
*/
int inGetEDCLOCK(char* szEDCLOCK)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szEDCLOCK == NULL || strlen(srEDCRec.szEDCLOCK) <= 0 || strlen(srEDCRec.szEDCLOCK) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetEDCLOCK() ERROR !!");

                        if (szEDCLOCK == NULL)
                        {
                                inDISP_LogPrintf("szEDCLOCK == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEDCLOCK length = (%d)", (int)strlen(srEDCRec.szEDCLOCK));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&szEDCLOCK[0], &srEDCRec.szEDCLOCK[0], strlen(srEDCRec.szEDCLOCK));

        return (VS_SUCCESS);
}

/*
Function        :inSetEDCLOCK
Date&Time       :
Describe        :
*/
int inSetEDCLOCK(char* szEDCLOCK)
{
        memset(srEDCRec.szEDCLOCK, 0x00, sizeof(srEDCRec.szEDCLOCK));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szEDCLOCK == NULL || strlen(szEDCLOCK) <= 0 || strlen(szEDCLOCK) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetEDCLOCK() ERROR !!");

                        if (szEDCLOCK == NULL)
                        {
                                inDISP_LogPrintf("szEDCLOCK == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEDCLOCK length = (%d)", (int)strlen(szEDCLOCK));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szEDCLOCK[0], &szEDCLOCK[0], strlen(szEDCLOCK));

        return (VS_SUCCESS);
}

/*
Function        :inGetISODebug
Date&Time       :
Describe        :
*/
int inGetISODebug(char* szISODebug)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szISODebug == NULL || strlen(srEDCRec.szISODebug) <= 0 || strlen(srEDCRec.szISODebug) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetISODebug() ERROR !!");

                        if (szISODebug == NULL)
                        {
                                inDISP_LogPrintf("szISODebug == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szISODebug length = (%d)", (int)strlen(srEDCRec.szISODebug));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szISODebug[0], &srEDCRec.szISODebug[0], strlen(srEDCRec.szISODebug));

        return (VS_SUCCESS);
}

/*
Function        :inSetISODebug
Date&Time       :
Describe        :
*/
int inSetISODebug(char* szISODebug)
{
        memset(srEDCRec.szISODebug, 0x00, sizeof(srEDCRec.szISODebug));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szISODebug == NULL || strlen(szISODebug) <= 0 || strlen(szISODebug) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetISODebug() ERROR !!");

                        if (szISODebug == NULL)
                        {
                                inDISP_LogPrintf("szISODebug == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szISODebug length = (%d)", (int)strlen(szISODebug));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szISODebug[0], &szISODebug[0], strlen(szISODebug));

        return (VS_SUCCESS);
}

/*
Function        :inGetManagerPassword
Date&Time       :2017/11/17 下午 2:13
Describe        :
*/
int inGetManagerPassword(char* szManagerPassword)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szManagerPassword == NULL || strlen(srEDCRec.szManagerPassword) <= 0 || strlen(srEDCRec.szManagerPassword) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPassWord() ERROR !!");

                        if (szManagerPassword == NULL)
                        {
                                inDISP_LogPrintf("szPassWord == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPassWord length = (%d)", (int)strlen(srEDCRec.szManagerPassword));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szManagerPassword[0], &srEDCRec.szManagerPassword[0], strlen(srEDCRec.szManagerPassword));

        return (VS_SUCCESS);
}

/*
Function        :inSetManagerPassword
Date&Time       :2017/11/17 下午 2:12
Describe        :
*/
int inSetManagerPassword(char* szManagerPassword)
{
        memset(srEDCRec.szManagerPassword, 0x00, sizeof(srEDCRec.szManagerPassword));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szManagerPassword == NULL || strlen(szManagerPassword) <= 0 || strlen(szManagerPassword) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPassWord() ERROR !!");

                        if (szManagerPassword == NULL)
                        {
                                inDISP_LogPrintf("szPassWord == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPassWord length = (%d)", (int)strlen(szManagerPassword));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szManagerPassword[0], &szManagerPassword[0], strlen(szManagerPassword));

        return (VS_SUCCESS);
}

/*
Function        :inGetFunctionPassword
Date&Time       :2017/11/17 下午 2:13
Describe        :
*/
int inGetFunctionPassword(char* szFunctionPassword)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szFunctionPassword == NULL || strlen(srEDCRec.szFunctionPassword) <= 0 || strlen(srEDCRec.szFunctionPassword) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPassWord() ERROR !!");

                        if (szFunctionPassword == NULL)
                        {
                                inDISP_LogPrintf("szPassWord == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPassWord length = (%d)", (int)strlen(srEDCRec.szFunctionPassword));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFunctionPassword[0], &srEDCRec.szFunctionPassword[0], strlen(srEDCRec.szFunctionPassword));

        return (VS_SUCCESS);
}

/*
Function        :inSetFunctionPassword
Date&Time       :2017/11/17 下午 2:12
Describe        :
*/
int inSetFunctionPassword(char* szFunctionPassword)
{
        memset(srEDCRec.szFunctionPassword, 0x00, sizeof(srEDCRec.szFunctionPassword));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szFunctionPassword == NULL || strlen(szFunctionPassword) <= 0 || strlen(szFunctionPassword) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPassWord() ERROR !!");

                        if (szFunctionPassword == NULL)
                        {
                                inDISP_LogPrintf("szPassWord == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPassWord length = (%d)", (int)strlen(szFunctionPassword));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szFunctionPassword[0], &szFunctionPassword[0], strlen(szFunctionPassword));

        return (VS_SUCCESS);
}

/*
Function        :inGetMerchantPassword
Date&Time       :2017/11/17 下午 2:13
Describe        :
*/
int inGetMerchantPassword(char* szMerchantPassword)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMerchantPassword == NULL || strlen(srEDCRec.szMerchantPassword) <= 0 || strlen(srEDCRec.szMerchantPassword) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPassWord() ERROR !!");

                        if (szMerchantPassword == NULL)
                        {
                                inDISP_LogPrintf("szPassWord == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPassWord length = (%d)", (int)strlen(srEDCRec.szMerchantPassword));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMerchantPassword[0], &srEDCRec.szMerchantPassword[0], strlen(srEDCRec.szMerchantPassword));

        return (VS_SUCCESS);
}

/*
Function        :inSetMerchantPassword
Date&Time       :2017/11/17 下午 2:12
Describe        :
*/
int inSetMerchantPassword(char* szMerchantPassword)
{
        memset(srEDCRec.szMerchantPassword, 0x00, sizeof(srEDCRec.szMerchantPassword));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMerchantPassword == NULL || strlen(szMerchantPassword) <= 0 || strlen(szMerchantPassword) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPassWord() ERROR !!");

                        if (szMerchantPassword == NULL)
                        {
                                inDISP_LogPrintf("szPassWord == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPassWord length = (%d)", (int)strlen(szMerchantPassword));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szMerchantPassword[0], &szMerchantPassword[0], strlen(szMerchantPassword));

        return (VS_SUCCESS);
}

/*
Function        :inGetSuperPassword
Date&Time       :2017/11/17 下午 2:13
Describe        :
*/
int inGetSuperPassword(char* szSuperPassword)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSuperPassword == NULL || strlen(srEDCRec.szSuperPassword) <= 0 || strlen(srEDCRec.szSuperPassword) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPassWord() ERROR !!");

                        if (szSuperPassword == NULL)
                        {
                                inDISP_LogPrintf("szPassWord == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPassWord length = (%d)", (int)strlen(srEDCRec.szSuperPassword));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSuperPassword[0], &srEDCRec.szSuperPassword[0], strlen(srEDCRec.szSuperPassword));

        return (VS_SUCCESS);
}

/*
Function        :inSetSuperPassword
Date&Time       :2017/11/17 下午 2:12
Describe        :
*/
int inSetSuperPassword(char* szSuperPassword)
{
        memset(srEDCRec.szSuperPassword, 0x00, sizeof(srEDCRec.szSuperPassword));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSuperPassword == NULL || strlen(szSuperPassword) <= 0 || strlen(szSuperPassword) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPassWord() ERROR !!");

                        if (szSuperPassword == NULL)
                        {
                                inDISP_LogPrintf("szPassWord == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPassWord length = (%d)", (int)strlen(szSuperPassword));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szSuperPassword[0], &szSuperPassword[0], strlen(szSuperPassword));

        return (VS_SUCCESS);
}

/*
Function        :inGetMCCCode
Date&Time       :
Describe        :
*/
int inGetMCCCode(char* szMCCCode)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMCCCode == NULL || strlen(srEDCRec.szMCCCode) <= 0 || strlen(srEDCRec.szMCCCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetMCCCode() ERROR !!");

                        if (szMCCCode == NULL)
                        {
                                inDISP_LogPrintf("szMCCCode == NULL");
                        }
                        else
                        {
                                sprintf(szErrorMsg, "szMCCCode length = (%d)", (int)strlen(srEDCRec.szMCCCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMCCCode[0], &srEDCRec.szMCCCode[0], strlen(srEDCRec.szMCCCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetMCCCode
Date&Time       :
Describe        :
*/
int inSetMCCCode(char* szMCCCode)
{
        memset(srEDCRec.szMCCCode, 0x00, sizeof(srEDCRec.szMCCCode));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMCCCode == NULL || strlen(szMCCCode) <= 0 || strlen(szMCCCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetMCCCode() ERROR !!");

                        if (szMCCCode == NULL)
                        {
                                inDISP_LogPrintf("szMCCCode == NULL");
                        }
                        else
                        {
                                sprintf(szErrorMsg, "szMCCCode length = (%d)", (int)strlen(szMCCCode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szMCCCode[0], &szMCCCode[0], strlen(szMCCCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetIssuerID
Date&Time       :
Describe        :
*/
int inGetIssuerID(char* szIssuerID)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szIssuerID == NULL || strlen(srEDCRec.szIssuerID) <= 0 || strlen(srEDCRec.szIssuerID) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetIssuerID() ERROR !!");

                        if (szIssuerID == NULL)
                        {
                                inDISP_LogPrintf("szIssuerID == NULL");
                        }
                        else
                        {
                                sprintf(szErrorMsg, "szIssuerID length = (%d)", (int)strlen(srEDCRec.szIssuerID));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szIssuerID[0], &srEDCRec.szIssuerID[0], strlen(srEDCRec.szIssuerID));

        return (VS_SUCCESS);
}

/*
Function        :inSetIssuerID
Date&Time       :
Describe        :
*/
int inSetIssuerID(char* szIssuerID)
{
        memset(srEDCRec.szIssuerID, 0x00, sizeof(srEDCRec.szIssuerID));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szIssuerID == NULL || strlen(szIssuerID) <= 0 || strlen(szIssuerID) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetIssuerID() ERROR !!");

                        if (szIssuerID == NULL)
                        {
                                inDISP_LogPrintf("szIssuerID == NULL");
                        }
                        else
                        {
                                sprintf(szErrorMsg, "szIssuerID length = (%d)", (int)strlen(szIssuerID));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szIssuerID[0], &szIssuerID[0], strlen(szIssuerID));

        return (VS_SUCCESS);
}

/*
Function        :inGetEnterTimeout
Date&Time       :
Describe        :
*/
int inGetEnterTimeout(char* szEnterTimeout)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szEnterTimeout == NULL || strlen(srEDCRec.szEnterTimeout) <= 0 || strlen(srEDCRec.szEnterTimeout) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetEnterTimeout() ERROR !!");

                        if (szEnterTimeout == NULL)
                        {
                                inDISP_LogPrintf("szEnterTimeout == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEnterTimeout length = (%d)", (int)strlen(srEDCRec.szEnterTimeout));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szEnterTimeout[0], &srEDCRec.szEnterTimeout[0], strlen(srEDCRec.szEnterTimeout));

        return (VS_SUCCESS);
}

/*
Function        :inSetEnterTimeout
Date&Time       :
Describe        :
*/
int inSetEnterTimeout(char* szEnterTimeout)
{
        memset(srEDCRec.szEnterTimeout, 0x00, sizeof(srEDCRec.szEnterTimeout));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szEnterTimeout == NULL || strlen(szEnterTimeout) <= 0 || strlen(szEnterTimeout) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetEnterTimeout() ERROR !!");

                        if (szEnterTimeout == NULL)
                        {
                                inDISP_LogPrintf("szEnterTimeout == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEnterTimeout length = (%d)", (int)strlen(szEnterTimeout));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szEnterTimeout[0], &szEnterTimeout[0], strlen(szEnterTimeout));

        return (VS_SUCCESS);
}

/*
Function        :inGetIPSendTimeout
Date&Time       :
Describe        :
*/
int inGetIPSendTimeout(char* szIPSendTimeout)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szIPSendTimeout == NULL || strlen(srEDCRec.szIPSendTimeout) <= 0 || strlen(srEDCRec.szIPSendTimeout) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetIPSendTimeout() ERROR !!");

                        if (szIPSendTimeout == NULL)
                        {
                                inDISP_LogPrintf("szIPSendTimeout == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIPSendTimeout length = (%d)", (int)strlen(srEDCRec.szIPSendTimeout));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szIPSendTimeout[0], &srEDCRec.szIPSendTimeout[0], strlen(srEDCRec.szIPSendTimeout));

        return (VS_SUCCESS);
}

/*
Function        :inSetIPSendTimeout
Date&Time       :
Describe        :
*/
int inSetIPSendTimeout(char* szIPSendTimeout)
{
        memset(srEDCRec.szIPSendTimeout, 0x00, sizeof(srEDCRec.szIPSendTimeout));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szIPSendTimeout == NULL || strlen(szIPSendTimeout) <= 0 || strlen(szIPSendTimeout) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetIPSendTimeout() ERROR !!");

                        if (szIPSendTimeout == NULL)
                        {
                                inDISP_LogPrintf("szIPSendTimeout == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIPSendTimeout length = (%d)", (int)strlen(szIPSendTimeout));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szIPSendTimeout[0], &szIPSendTimeout[0], strlen(szIPSendTimeout));

        return (VS_SUCCESS);
}

/*
Function        :inGetTermVersionID
Date&Time       :
Describe        :
*/
int inGetTermVersionID(char* szTermVersionID)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermVersionID == NULL || strlen(srEDCRec.szTermVersionID) <=0 || strlen(srEDCRec.szTermVersionID) > 15)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTermVersionID() ERROR !!");

                        if (szTermVersionID == NULL)
                        {
                                inDISP_LogPrintf("szTermVersionID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTermVersionID length = (%d)", (int)strlen(srEDCRec.szTermVersionID));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szTermVersionID[0], &srEDCRec.szTermVersionID[0], strlen(srEDCRec.szTermVersionID));

        return (VS_SUCCESS);
}

/*
Function        :inSetTermVersionID
Date&Time       :
Describe        :
*/
int inSetTermVersionID(char* szTermVersionID)
{
        memset(srEDCRec.szTermVersionID, 0x00, sizeof(srEDCRec.szTermVersionID));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermVersionID == NULL || strlen(szTermVersionID) <= 0 || strlen(szTermVersionID) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTermVersionID() ERROR !!");

                        if (szTermVersionID == NULL)
                        {
                                inDISP_LogPrintf("szTermVersionID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTermVersionID length = (%d)", (int)strlen(szTermVersionID));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szTermVersionID[0], &szTermVersionID[0], strlen(szTermVersionID));

        return (VS_SUCCESS);
}

/*
Function        :inGetTermVersionDate
Date&Time       :
Describe        :
*/
int inGetTermVersionDate(char* szTermVersionDate)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermVersionDate == NULL || strlen(srEDCRec.szTermVersionDate) <=0 || strlen(srEDCRec.szTermVersionDate) > 15)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTermVersionDate() ERROR !!");

                        if (szTermVersionDate == NULL)
                        {
                                inDISP_LogPrintf("szTermVersionDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTermVersionDate length = (%d)", (int)strlen(srEDCRec.szTermVersionDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szTermVersionDate[0], &srEDCRec.szTermVersionDate[0], strlen(srEDCRec.szTermVersionDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetTermVersionDate
Date&Time       :
Describe        :
*/
int inSetTermVersionDate(char* szTermVersionDate)
{
        memset(srEDCRec.szTermVersionDate, 0x00, sizeof(srEDCRec.szTermVersionDate));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermVersionDate == NULL || strlen(szTermVersionDate) <= 0 || strlen(szTermVersionDate) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTermVersionDate() ERROR !!");

                        if (szTermVersionDate == NULL)
                        {
                                inDISP_LogPrintf("szTermVersionDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTermVersionDate length = (%d)", (int)strlen(szTermVersionDate));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szTermVersionDate[0], &szTermVersionDate[0], strlen(szTermVersionDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetTermIPAddress
Date&Time       :
Describe        :
*/
int inGetTermIPAddress(char* szTermIPAddress)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermIPAddress == NULL || strlen(srEDCRec.szTermIPAddress) <=0 || strlen(srEDCRec.szTermIPAddress) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szTermIPAddress == NULL)
                        {
                                inDISP_LogPrintf("inGetTermIPAddress() ERROR !! szTermIPAddress == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inGetTermIPAddress() ERROR !! szTermIPAddress length = (%d)", (int)strlen(srEDCRec.szTermIPAddress));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTermIPAddress[0], &srEDCRec.szTermIPAddress[0], strlen(srEDCRec.szTermIPAddress));

        return (VS_SUCCESS);
}

/*
Function        :inSetTermIPAddress
Date&Time       :
Describe        :
*/
int inSetTermIPAddress(char* szTermIPAddress)
{
        memset(srEDCRec.szTermIPAddress, 0x00, sizeof(srEDCRec.szTermIPAddress));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermIPAddress == NULL || strlen(szTermIPAddress) <= 0 || strlen(szTermIPAddress) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTermIPAddress() ERROR !!");

                        if (szTermIPAddress == NULL)
                        {
                                inDISP_LogPrintf("szTermIPAddress == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTermIPAddress length = (%d)", (int)strlen(szTermIPAddress));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szTermIPAddress[0], &szTermIPAddress[0], strlen(szTermIPAddress));

        return (VS_SUCCESS);
}

/*
Function        :inGetTermGetewayAddress
Date&Time       :
Describe        :
*/
int inGetTermGetewayAddress(char* szTermGetewayAddress)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermGetewayAddress == NULL || strlen(srEDCRec.szTermGetewayAddress) <=0 || strlen(srEDCRec.szTermGetewayAddress) > 15)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTermGetewayAddress() ERROR !!");

                        if (szTermGetewayAddress == NULL)
                        {
                                inDISP_LogPrintf("szTermGetewayAddress == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTermGetewayAddress length = (%d)", (int)strlen(srEDCRec.szTermGetewayAddress));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }

                return (VS_ERROR);
        }
        memcpy(&szTermGetewayAddress[0], &srEDCRec.szTermGetewayAddress[0], strlen(srEDCRec.szTermGetewayAddress));

        return (VS_SUCCESS);
}

/*
Function        :inSetTermGetewayAddress
Date&Time       :
Describe        :
*/
int inSetTermGetewayAddress(char* szTermGetewayAddress)
{
        memset(srEDCRec.szTermGetewayAddress, 0x00, sizeof(srEDCRec.szTermGetewayAddress));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermGetewayAddress == NULL || strlen(szTermGetewayAddress) <= 0 || strlen(szTermGetewayAddress) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTermGetewayAddress() ERROR !!");

                        if (szTermGetewayAddress == NULL)
                        {
                                inDISP_LogPrintf("szTermGetewayAddress == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTermGetewayAddress length = (%d)", (int)strlen(szTermGetewayAddress));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szTermGetewayAddress[0], &szTermGetewayAddress[0], strlen(szTermGetewayAddress));

        return (VS_SUCCESS);
}

/*
Function        :inGetTermMASKAddress
Date&Time       :
Describe        :
*/
int inGetTermMASKAddress(char* szTermMASKAddress)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermMASKAddress == NULL || strlen(srEDCRec.szTermMASKAddress) <=0 || strlen(srEDCRec.szTermMASKAddress) > 15)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTermMASKAddress() ERROR !!");

                        if (szTermMASKAddress == NULL)
                        {
                                inDISP_LogPrintf("szTermMASKAddress == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTermMASKAddress length = (%d)", (int)strlen(srEDCRec.szTermMASKAddress));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szTermMASKAddress[0], &srEDCRec.szTermMASKAddress[0], strlen(srEDCRec.szTermMASKAddress));

        return (VS_SUCCESS);
}

/*
Function        :inSetTermMASKAddress
Date&Time       :
Describe        :
*/
int inSetTermMASKAddress(char* szTermMASKAddress)
{
        memset(srEDCRec.szTermMASKAddress, 0x00, sizeof(srEDCRec.szTermMASKAddress));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermMASKAddress == NULL || strlen(szTermMASKAddress) <= 0 || strlen(szTermMASKAddress) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTermMASKAddress() ERROR !!");

                        if (szTermMASKAddress == NULL)
                        {
                                inDISP_LogPrintf("szTermMASKAddress == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTermMASKAddress length = (%d)", (int)strlen(szTermMASKAddress));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szTermMASKAddress[0], &szTermMASKAddress[0], strlen(szTermMASKAddress));

        return (VS_SUCCESS);
}

/*
Function        :inGetTermECRPort
Date&Time       :2017/6/5 下午 2:24
Describe        :
*/
int inGetTermECRPort(char* szTermECRPort)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermECRPort == NULL || strlen(srEDCRec.szTermECRPort) <=0 || strlen(srEDCRec.szTermECRPort) > 6)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetTermECRPort() ERROR !!");

                        if (szTermECRPort == NULL)
                        {
                                inDISP_LogPrintf("szTermECRPort == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTermECRPort length = (%d)", (int)strlen(srEDCRec.szTermECRPort));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szTermECRPort[0], &srEDCRec.szTermECRPort[0], strlen(srEDCRec.szTermECRPort));

        return (VS_SUCCESS);
}

/*
Function        :inSetTermECRPort
Date&Time       :2017/6/5 下午 2:24
Describe        :
*/
int inSetTermECRPort(char* szTermECRPort)
{
        memset(srEDCRec.szTermECRPort, 0x00, sizeof(srEDCRec.szTermECRPort));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermECRPort == NULL || strlen(szTermECRPort) <= 0 || strlen(szTermECRPort) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetTermECRPort() ERROR !!");

                        if (szTermECRPort == NULL)
                        {
                                inDISP_LogPrintf("szTermECRPort == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTermECRPort length = (%d)", (int)strlen(szTermECRPort));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szTermECRPort[0], &szTermECRPort[0], strlen(szTermECRPort));

        return (VS_SUCCESS);
}

/*
Function        :inGetESCMode
Date&Time       :2017/4/11 下午 1:25
Describe        :
*/
int inGetESCMode(char* szESCMode)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szESCMode == NULL || strlen(srEDCRec.szESCMode) <=0 || strlen(srEDCRec.szESCMode) > 2)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetESCMode() ERROR !!");

                        if (szESCMode == NULL)
                        {
                                inDISP_LogPrintf("szESCMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESCMode length = (%d)", (int)strlen(srEDCRec.szESCMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szESCMode[0], &srEDCRec.szESCMode[0], strlen(srEDCRec.szESCMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetESCMode
Date&Time       :2017/4/11 下午 1:25
Describe        :
*/
int inSetESCMode(char* szESCMode)
{
        memset(srEDCRec.szESCMode, 0x00, sizeof(srEDCRec.szESCMode));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szESCMode == NULL || strlen(szESCMode) <= 0 || strlen(szESCMode) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetESCMode() ERROR !!");

                        if (szESCMode == NULL)
                        {
                                inDISP_LogPrintf("szESCMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESCMode length = (%d)", (int)strlen(szESCMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szESCMode[0], &szESCMode[0], strlen(szESCMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetMultiComPort1
Date&Time       :2017/6/30 下午 3:53
Describe        :
*/
int inGetMultiComPort1(char* szMultiComPort1)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort1 == NULL || strlen(srEDCRec.szMultiComPort1) <=0 || strlen(srEDCRec.szMultiComPort1) > 4)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetMultiComPort1() ERROR !!");

                        if (szMultiComPort1 == NULL)
                        {
                                inDISP_LogPrintf("szMultiComPort1 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort1 length = (%d)", (int)strlen(srEDCRec.szMultiComPort1));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szMultiComPort1[0], &srEDCRec.szMultiComPort1[0], strlen(srEDCRec.szMultiComPort1));

        return (VS_SUCCESS);
}

/*
Function        :inSetMultiComPort1
Date&Time       :2017/6/30 下午 3:53
Describe        :
*/
int inSetMultiComPort1(char* szMultiComPort1)
{
        memset(srEDCRec.szMultiComPort1, 0x00, sizeof(srEDCRec.szMultiComPort1));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort1 == NULL || strlen(szMultiComPort1) <= 0 || strlen(szMultiComPort1) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetMultiComPort1() ERROR !!");

                        if (szMultiComPort1 == NULL)
                        {
                                inDISP_LogPrintf("szMultiComPort1 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort1 length = (%d)", (int)strlen(szMultiComPort1));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szMultiComPort1[0], &szMultiComPort1[0], strlen(szMultiComPort1));

        return (VS_SUCCESS);
}

/*
Function        :inGetMultiComPort1Version
Date&Time       :2017/7/3 上午 10:50
Describe        :
*/
int inGetMultiComPort1Version(char* szMultiComPort1Version)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort1Version == NULL || strlen(srEDCRec.szMultiComPort1Version) <=0 || strlen(srEDCRec.szMultiComPort1Version) > 2)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetMultiComPort1Version() ERROR !!");

                        if (szMultiComPort1Version == NULL)
                        {
                                inDISP_LogPrintf("szMultiComPort1Version == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort1Version length = (%d)", (int)strlen(srEDCRec.szMultiComPort1Version));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szMultiComPort1Version[0], &srEDCRec.szMultiComPort1Version[0], strlen(srEDCRec.szMultiComPort1Version));

        return (VS_SUCCESS);
}

/*
Function        :inSetMultiComPort1Version
Date&Time       :2017/7/3 上午 10:50
Describe        :
*/
int inSetMultiComPort1Version(char* szMultiComPort1Version)
{
        memset(srEDCRec.szMultiComPort1Version, 0x00, sizeof(srEDCRec.szMultiComPort1Version));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort1Version == NULL || strlen(szMultiComPort1Version) <= 0 || strlen(szMultiComPort1Version) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetMultiComPort1Version() ERROR !!");

                        if (szMultiComPort1Version == NULL)
                        {
                                inDISP_LogPrintf("szMultiComPort1Version == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort1Version length = (%d)", (int)strlen(szMultiComPort1Version));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szMultiComPort1Version[0], &szMultiComPort1Version[0], strlen(szMultiComPort1Version));

        return (VS_SUCCESS);
}

/*
Function        :inGetMultiComPort2
Date&Time       :2017/6/30 下午 3:53
Describe        :
*/
int inGetMultiComPort2(char* szMultiComPort2)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort2 == NULL || strlen(srEDCRec.szMultiComPort2) <=0 || strlen(srEDCRec.szMultiComPort2) > 4)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetMultiComPort2() ERROR !!");

                        if (szMultiComPort2 == NULL)
                        {
                                inDISP_LogPrintf("szMultiComPort2 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort2 length = (%d)", (int)strlen(srEDCRec.szMultiComPort2));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szMultiComPort2[0], &srEDCRec.szMultiComPort2[0], strlen(srEDCRec.szMultiComPort2));

        return (VS_SUCCESS);
}

/*
Function        :inSetMultiComPort2
Date&Time       :2017/6/30 下午 3:53
Describe        :
*/
int inSetMultiComPort2(char* szMultiComPort2)
{
        memset(srEDCRec.szMultiComPort2, 0x00, sizeof(srEDCRec.szMultiComPort2));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort2 == NULL || strlen(szMultiComPort2) <= 0 || strlen(szMultiComPort2) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetMultiComPort2() ERROR !!");

                        if (szMultiComPort2 == NULL)
                        {
                                inDISP_LogPrintf("szMultiComPort2 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort2 length = (%d)", (int)strlen(szMultiComPort2));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szMultiComPort2[0], &szMultiComPort2[0], strlen(szMultiComPort2));

        return (VS_SUCCESS);
}

/*
Function        :inGetMultiComPort2Version
Date&Time       :2017/7/3 上午 10:50
Describe        :
*/
int inGetMultiComPort2Version(char* szMultiComPort2Version)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort2Version == NULL || strlen(srEDCRec.szMultiComPort2Version) <=0 || strlen(srEDCRec.szMultiComPort2Version) > 2)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetMultiComPort2Version() ERROR !!");

                        if (szMultiComPort2Version == NULL)
                        {
                                inDISP_LogPrintf("szMultiComPort2Version == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort2Version length = (%d)", (int)strlen(srEDCRec.szMultiComPort2Version));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szMultiComPort2Version[0], &srEDCRec.szMultiComPort2Version[0], strlen(srEDCRec.szMultiComPort2Version));

        return (VS_SUCCESS);
}

/*
Function        :inSetMultiComPort2Version
Date&Time       :2017/7/3 上午 10:50
Describe        :
*/
int inSetMultiComPort2Version(char* szMultiComPort2Version)
{
        memset(srEDCRec.szMultiComPort2Version, 0x00, sizeof(srEDCRec.szMultiComPort2Version));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort2Version == NULL || strlen(szMultiComPort2Version) <= 0 || strlen(szMultiComPort2Version) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetMultiComPort2Version() ERROR !!");

                        if (szMultiComPort2Version == NULL)
                        {
                                inDISP_LogPrintf("szMultiComPort2Version == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort2Version length = (%d)", (int)strlen(szMultiComPort2Version));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szMultiComPort2Version[0], &szMultiComPort2Version[0], strlen(szMultiComPort2Version));

        return (VS_SUCCESS);
}

/*
Function        :inGetMultiComPort3
Date&Time       :2017/6/30 下午 3:53
Describe        :
*/
int inGetMultiComPort3(char* szMultiComPort3)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort3 == NULL || strlen(srEDCRec.szMultiComPort3) <=0 || strlen(srEDCRec.szMultiComPort3) > 4)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetMultiComPort3() ERROR !!");

                        if (szMultiComPort3 == NULL)
                        {
                                inDISP_LogPrintf("szMultiComPort3 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort3 length = (%d)", (int)strlen(srEDCRec.szMultiComPort3));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szMultiComPort3[0], &srEDCRec.szMultiComPort3[0], strlen(srEDCRec.szMultiComPort3));

        return (VS_SUCCESS);
}

/*
Function        :inSetMultiComPort3
Date&Time       :2017/6/30 下午 3:53
Describe        :
*/
int inSetMultiComPort3(char* szMultiComPort3)
{
        memset(srEDCRec.szMultiComPort3, 0x00, sizeof(srEDCRec.szMultiComPort3));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort3 == NULL || strlen(szMultiComPort3) <= 0 || strlen(szMultiComPort3) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetMultiComPort3() ERROR !!");

                        if (szMultiComPort3 == NULL)
                        {
                                inDISP_LogPrintf("szMultiComPort3 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort3 length = (%d)", (int)strlen(szMultiComPort3));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szMultiComPort3[0], &szMultiComPort3[0], strlen(szMultiComPort3));

        return (VS_SUCCESS);
}

/*
Function        :inGetMultiComPort3Version
Date&Time       :2017/7/3 上午 10:50
Describe        :
*/
int inGetMultiComPort3Version(char* szMultiComPort3Version)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort3Version == NULL || strlen(srEDCRec.szMultiComPort3Version) <=0 || strlen(srEDCRec.szMultiComPort3Version) > 2)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetMultiComPort3Version() ERROR !!");

                        if (szMultiComPort3Version == NULL)
                        {
                                inDISP_LogPrintf("szMultiComPort3Version == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort3Version length = (%d)", (int)strlen(srEDCRec.szMultiComPort3Version));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szMultiComPort3Version[0], &srEDCRec.szMultiComPort3Version[0], strlen(srEDCRec.szMultiComPort3Version));

        return (VS_SUCCESS);
}

/*
Function        :inSetMultiComPort3Version
Date&Time       :2017/7/3 上午 10:50
Describe        :
*/
int inSetMultiComPort3Version(char* szMultiComPort3Version)
{
        memset(srEDCRec.szMultiComPort3Version, 0x00, sizeof(srEDCRec.szMultiComPort3Version));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort3Version == NULL || strlen(szMultiComPort3Version) <= 0 || strlen(szMultiComPort3Version) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetMultiComPort3Version() ERROR !!");

                        if (szMultiComPort3Version == NULL)
                        {
                                inDISP_LogPrintf("szMultiComPort3Version == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort3Version length = (%d)", (int)strlen(szMultiComPort3Version));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szMultiComPort3Version[0], &szMultiComPort3Version[0], strlen(szMultiComPort3Version));

        return (VS_SUCCESS);
}

/*
Function        :inGetEMVForceOnline
Date&Time       :2017/9/4 上午 10:42
Describe        :
*/
int inGetEMVForceOnline(char* szEMVForceOnline)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szEMVForceOnline == NULL || strlen(srEDCRec.szEMVForceOnline) <=0 || strlen(srEDCRec.szEMVForceOnline) > 2)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetEMVForceOnline() ERROR !!");

                        if (szEMVForceOnline == NULL)
                        {
                                inDISP_LogPrintf("szEMVForceOnline == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEMVForceOnline length = (%d)", (int)strlen(srEDCRec.szEMVForceOnline));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szEMVForceOnline[0], &srEDCRec.szEMVForceOnline[0], strlen(srEDCRec.szEMVForceOnline));

        return (VS_SUCCESS);
}

/*
Function        :inSetEMVForceOnline
Date&Time       :2017/7/3 上午 10:50
Describe        :
*/
int inSetEMVForceOnline(char* szEMVForceOnline)
{
        memset(srEDCRec.szEMVForceOnline, 0x00, sizeof(srEDCRec.szEMVForceOnline));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szEMVForceOnline == NULL || strlen(szEMVForceOnline) <= 0 || strlen(szEMVForceOnline) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetEMVForceOnline() ERROR !!");

                        if (szEMVForceOnline == NULL)
                        {
                                inDISP_LogPrintf("szEMVForceOnline == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEMVForceOnline length = (%d)", (int)strlen(szEMVForceOnline));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szEMVForceOnline[0], &szEMVForceOnline[0], strlen(szEMVForceOnline));

        return (VS_SUCCESS);
}

/*
Function        :inGetAutoConnect
Date&Time       :2017/10/2 上午 11:47
Describe        :
*/
int inGetAutoConnect(char* szAutoConnect)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szAutoConnect == NULL || strlen(srEDCRec.szAutoConnect) <=0 || strlen(srEDCRec.szAutoConnect) > 10)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetAutoConnect() ERROR !!");

                        if (szAutoConnect == NULL)
                        {
                                inDISP_LogPrintf("szAutoConnect == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szAutoConnect length = (%d)", (int)strlen(srEDCRec.szAutoConnect));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szAutoConnect[0], &srEDCRec.szAutoConnect[0], strlen(srEDCRec.szAutoConnect));

        return (VS_SUCCESS);
}

/*
Function        :inSetAutoConnect
Date&Time       :2017/10/2 上午 11:47
Describe        :
*/
int inSetAutoConnect(char* szAutoConnect)
{
        memset(srEDCRec.szAutoConnect, 0x00, sizeof(srEDCRec.szAutoConnect));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szAutoConnect == NULL || strlen(szAutoConnect) <= 0 || strlen(szAutoConnect) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetAutoConnect() ERROR !!");

                        if (szAutoConnect == NULL)
                        {
                                inDISP_LogPrintf("szAutoConnect == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szAutoConnect length = (%d)", (int)strlen(szAutoConnect));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szAutoConnect[0], &szAutoConnect[0], strlen(szAutoConnect));

        return (VS_SUCCESS);
}

/*
Function        :inGetLOGONum
Date&Time       :2017/10/2 上午 11:47
Describe        :
*/
int inGetLOGONum(char* szLOGONum)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szLOGONum == NULL || strlen(srEDCRec.szLOGONum) <=0 || strlen(srEDCRec.szLOGONum) > 4)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetLOGONum() ERROR !!");

                        if (szLOGONum == NULL)
                        {
                                inDISP_LogPrintf("szLOGONum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szLOGONum length = (%d)", (int)strlen(srEDCRec.szLOGONum));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szLOGONum[0], &srEDCRec.szLOGONum[0], strlen(srEDCRec.szLOGONum));

        return (VS_SUCCESS);
}

/*
Function        :inSetLOGONum
Date&Time       :2017/10/2 上午 11:47
Describe        :
*/
int inSetLOGONum(char* szLOGONum)
{
        memset(srEDCRec.szLOGONum, 0x00, sizeof(srEDCRec.szLOGONum));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szLOGONum == NULL || strlen(szLOGONum) <= 0 || strlen(szLOGONum) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetLOGONum() ERROR !!");

                        if (szLOGONum == NULL)
                        {
                                inDISP_LogPrintf("szLOGONum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szLOGONum length = (%d)", (int)strlen(szLOGONum));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szLOGONum[0], &szLOGONum[0], strlen(szLOGONum));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostSAMSlot
Date&Time       :2018/1/11 下午 4:30
Describe        :
*/
int inGetHostSAMSlot(char* szHostSAMSlot)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostSAMSlot == NULL || strlen(srEDCRec.szHostSAMSlot) <=0 || strlen(srEDCRec.szHostSAMSlot) > 2)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetHostSAMSlot() ERROR !!");

                        if (szHostSAMSlot == NULL)
                        {
                                inDISP_LogPrintf("szHostSAMSlot == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHostSAMSlot length = (%d)", (int)strlen(srEDCRec.szHostSAMSlot));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szHostSAMSlot[0], &srEDCRec.szHostSAMSlot[0], strlen(srEDCRec.szHostSAMSlot));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostSAMSlot
Date&Time       :2018/1/11 下午 4:30
Describe        :
*/
int inSetHostSAMSlot(char* szHostSAMSlot)
{
        memset(srEDCRec.szHostSAMSlot, 0x00, sizeof(srEDCRec.szHostSAMSlot));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostSAMSlot == NULL || strlen(szHostSAMSlot) <= 0 || strlen(szHostSAMSlot) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetHostSAMSlot() ERROR !!");

                        if (szHostSAMSlot == NULL)
                        {
                                inDISP_LogPrintf("szHostSAMSlot == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHostSAMSlot length = (%d)", (int)strlen(szHostSAMSlot));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szHostSAMSlot[0], &szHostSAMSlot[0], strlen(szHostSAMSlot));

        return (VS_SUCCESS);
}

/*
Function        :inGetSAMSlotSN1
Date&Time       :2018/1/11 下午 4:30
Describe        :
*/
int inGetSAMSlotSN1(char* szSAMSlotSN1)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSAMSlotSN1 == NULL || strlen(srEDCRec.szSAMSlotSN1) <=0 || strlen(srEDCRec.szSAMSlotSN1) > 16)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetSAMSlotSN1() ERROR !!");

                        if (szSAMSlotSN1 == NULL)
                        {
                                inDISP_LogPrintf("szSAMSlotSN1 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSAMSlotSN1 length = (%d)", (int)strlen(srEDCRec.szSAMSlotSN1));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szSAMSlotSN1[0], &srEDCRec.szSAMSlotSN1[0], strlen(srEDCRec.szSAMSlotSN1));

        return (VS_SUCCESS);
}

/*
Function        :inSetSAMSlotSN1
Date&Time       :2018/1/11 下午 4:30
Describe        :
*/
int inSetSAMSlotSN1(char* szSAMSlotSN1)
{
        memset(srEDCRec.szSAMSlotSN1, 0x00, sizeof(srEDCRec.szSAMSlotSN1));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSAMSlotSN1 == NULL || strlen(szSAMSlotSN1) <= 0 || strlen(szSAMSlotSN1) > 16)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetSAMSlotSN1() ERROR !!");

                        if (szSAMSlotSN1 == NULL)
                        {
                                inDISP_LogPrintf("szSAMSlotSN1 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSAMSlotSN1 length = (%d)", (int)strlen(szSAMSlotSN1));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szSAMSlotSN1[0], &szSAMSlotSN1[0], strlen(szSAMSlotSN1));

        return (VS_SUCCESS);
}

/*
Function        :inGetSAMSlotSN2
Date&Time       :2018/1/11 下午 4:30
Describe        :
*/
int inGetSAMSlotSN2(char* szSAMSlotSN2)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSAMSlotSN2 == NULL || strlen(srEDCRec.szSAMSlotSN2) <=0 || strlen(srEDCRec.szSAMSlotSN2) > 16)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetSAMSlotSN2() ERROR !!");

                        if (szSAMSlotSN2 == NULL)
                        {
                                inDISP_LogPrintf("szSAMSlotSN2 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSAMSlotSN2 length = (%d)", (int)strlen(srEDCRec.szSAMSlotSN2));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szSAMSlotSN2[0], &srEDCRec.szSAMSlotSN2[0], strlen(srEDCRec.szSAMSlotSN2));

        return (VS_SUCCESS);
}

/*
Function        :inSetSAMSlotSN2
Date&Time       :2018/1/11 下午 4:30
Describe        :
*/
int inSetSAMSlotSN2(char* szSAMSlotSN2)
{
        memset(srEDCRec.szSAMSlotSN2, 0x00, sizeof(srEDCRec.szSAMSlotSN2));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSAMSlotSN2 == NULL || strlen(szSAMSlotSN2) <= 0 || strlen(szSAMSlotSN2) > 16)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetSAMSlotSN2() ERROR !!");

                        if (szSAMSlotSN2 == NULL)
                        {
                                inDISP_LogPrintf("szSAMSlotSN2 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSAMSlotSN2 length = (%d)", (int)strlen(szSAMSlotSN2));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szSAMSlotSN2[0], &szSAMSlotSN2[0], strlen(szSAMSlotSN2));

        return (VS_SUCCESS);
}

/*
Function        :inGetSAMSlotSN3
Date&Time       :2018/1/11 下午 4:30
Describe        :
*/
int inGetSAMSlotSN3(char* szSAMSlotSN3)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSAMSlotSN3 == NULL || strlen(srEDCRec.szSAMSlotSN3) <=0 || strlen(srEDCRec.szSAMSlotSN3) > 16)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetSAMSlotSN3() ERROR !!");

                        if (szSAMSlotSN3 == NULL)
                        {
                                inDISP_LogPrintf("szSAMSlotSN3 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSAMSlotSN3 length = (%d)", (int)strlen(srEDCRec.szSAMSlotSN3));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szSAMSlotSN3[0], &srEDCRec.szSAMSlotSN3[0], strlen(srEDCRec.szSAMSlotSN3));

        return (VS_SUCCESS);
}

/*
Function        :inSetSAMSlotSN3
Date&Time       :2018/1/11 下午 4:30
Describe        :
*/
int inSetSAMSlotSN3(char* szSAMSlotSN3)
{
        memset(srEDCRec.szSAMSlotSN3, 0x00, sizeof(srEDCRec.szSAMSlotSN3));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSAMSlotSN3 == NULL || strlen(szSAMSlotSN3) <= 0 || strlen(szSAMSlotSN3) > 16)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetSAMSlotSN3() ERROR !!");

                        if (szSAMSlotSN3 == NULL)
                        {
                                inDISP_LogPrintf("szSAMSlotSN3 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSAMSlotSN3 length = (%d)", (int)strlen(szSAMSlotSN3));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szSAMSlotSN3[0], &szSAMSlotSN3[0], strlen(szSAMSlotSN3));

        return (VS_SUCCESS);
}

/*
Function        :inGetSAMSlotSN4
Date&Time       :2018/1/11 下午 4:30
Describe        :
*/
int inGetSAMSlotSN4(char* szSAMSlotSN4)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSAMSlotSN4 == NULL || strlen(srEDCRec.szSAMSlotSN4) <=0 || strlen(srEDCRec.szSAMSlotSN4) > 16)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetSAMSlotSN4() ERROR !!");

                        if (szSAMSlotSN4 == NULL)
                        {
                                inDISP_LogPrintf("szSAMSlotSN4 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSAMSlotSN4 length = (%d)", (int)strlen(srEDCRec.szSAMSlotSN4));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szSAMSlotSN4[0], &srEDCRec.szSAMSlotSN4[0], strlen(srEDCRec.szSAMSlotSN4));

        return (VS_SUCCESS);
}

/*
Function        :inSetSAMSlotSN4
Date&Time       :2018/1/11 下午 4:30
Describe        :
*/
int inSetSAMSlotSN4(char* szSAMSlotSN4)
{
        memset(srEDCRec.szSAMSlotSN4, 0x00, sizeof(srEDCRec.szSAMSlotSN4));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSAMSlotSN4 == NULL || strlen(szSAMSlotSN4) <= 0 || strlen(szSAMSlotSN4) > 16)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetSAMSlotSN4() ERROR !!");

                        if (szSAMSlotSN4 == NULL)
                        {
                                inDISP_LogPrintf("szSAMSlotSN4 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSAMSlotSN4 length = (%d)", (int)strlen(szSAMSlotSN4));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szSAMSlotSN4[0], &szSAMSlotSN4[0], strlen(szSAMSlotSN4));

        return (VS_SUCCESS);
}

/*
Function        :inGetPWMEnable
Date&Time       :2018/4/11 下午 3:00
Describe        :
*/
int inGetPWMEnable(char* szPWMEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPWMEnable == NULL || strlen(srEDCRec.szPWMEnable) <=0 || strlen(srEDCRec.szPWMEnable) > 2)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPWMEnable() ERROR !!");

                        if (szPWMEnable == NULL)
                        {
                                inDISP_LogPrintf("szPWMEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPWMEnable length = (%d)", (int)strlen(srEDCRec.szPWMEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szPWMEnable[0], &srEDCRec.szPWMEnable[0], strlen(srEDCRec.szPWMEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetPWMEnable
Date&Time       :2018/4/11 下午 3:00
Describe        :
*/
int inSetPWMEnable(char* szPWMEnable)
{
        memset(srEDCRec.szPWMEnable, 0x00, sizeof(srEDCRec.szPWMEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPWMEnable == NULL || strlen(szPWMEnable) <= 0 || strlen(szPWMEnable) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPWMEnable() ERROR !!");

                        if (szPWMEnable == NULL)
                        {
                                inDISP_LogPrintf("szPWMEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPWMEnable length = (%d)", (int)strlen(szPWMEnable));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szPWMEnable[0], &szPWMEnable[0], strlen(szPWMEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetPWMMode
Date&Time       :2018/4/11 下午 3:04
Describe        :
*/
int inGetPWMMode(char* szPWMMode)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPWMMode == NULL || strlen(srEDCRec.szPWMMode) <=0 || strlen(srEDCRec.szPWMMode) > 2)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPWMMode() ERROR !!");

                        if (szPWMMode == NULL)
                        {
                                inDISP_LogPrintf("szPWMMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPWMMode length = (%d)", (int)strlen(srEDCRec.szPWMMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szPWMMode[0], &srEDCRec.szPWMMode[0], strlen(srEDCRec.szPWMMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetPWMMode
Date&Time       :2018/4/11 下午 3:05
Describe        :
*/
int inSetPWMMode(char* szPWMMode)
{
        memset(srEDCRec.szPWMMode, 0x00, sizeof(srEDCRec.szPWMMode));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPWMMode == NULL || strlen(szPWMMode) <= 0 || strlen(szPWMMode) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPWMMode() ERROR !!");

                        if (szPWMMode == NULL)
                        {
                                inDISP_LogPrintf("szPWMMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPWMMode length = (%d)", (int)strlen(szPWMMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szPWMMode[0], &szPWMMode[0], strlen(szPWMMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetPWMIdleTimeout
Date&Time       :2018/4/11 下午 3:06
Describe        :
*/
int inGetPWMIdleTimeout(char* szPWMIdleTimeout)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPWMIdleTimeout == NULL || strlen(srEDCRec.szPWMIdleTimeout) <=0 || strlen(srEDCRec.szPWMIdleTimeout) > 4)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetPWMIdleTimeout() ERROR !!");

                        if (szPWMIdleTimeout == NULL)
                        {
                                inDISP_LogPrintf("szPWMIdleTimeout == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPWMIdleTimeout length = (%d)", (int)strlen(srEDCRec.szPWMIdleTimeout));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szPWMIdleTimeout[0], &srEDCRec.szPWMIdleTimeout[0], strlen(srEDCRec.szPWMIdleTimeout));

        return (VS_SUCCESS);
}

/*
Function        :inSetPWMIdleTimeout
Date&Time       :2018/4/11 下午 3:06
Describe        :
*/
int inSetPWMIdleTimeout(char* szPWMIdleTimeout)
{
        memset(srEDCRec.szPWMIdleTimeout, 0x00, sizeof(srEDCRec.szPWMIdleTimeout));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPWMIdleTimeout == NULL || strlen(szPWMIdleTimeout) <= 0 || strlen(szPWMIdleTimeout) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetPWMIdleTimeout() ERROR !!");

                        if (szPWMIdleTimeout == NULL)
                        {
                                inDISP_LogPrintf("szPWMIdleTimeout == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPWMIdleTimeout length = (%d)", (int)strlen(szPWMIdleTimeout));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szPWMIdleTimeout[0], &szPWMIdleTimeout[0], strlen(szPWMIdleTimeout));

        return (VS_SUCCESS);
}

/*
Function        :inGetDemoMode
Date&Time       :2018/8/22 上午 11:16
Describe        :
*/
int inGetDemoMode(char* szDemoMode)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDemoMode == NULL || strlen(srEDCRec.szDemoMode) <=0 || strlen(srEDCRec.szDemoMode) > 2)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inGetDemoMode() ERROR !!");

                        if (szDemoMode == NULL)
                        {
                                inDISP_LogPrintf("szDemoMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDemoMode length = (%d)", (int)strlen(srEDCRec.szDemoMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szDemoMode[0], &srEDCRec.szDemoMode[0], strlen(srEDCRec.szDemoMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetDemoMode
Date&Time       :2018/4/11 下午 3:06
Describe        :
*/
int inSetDemoMode(char* szDemoMode)
{
        memset(srEDCRec.szDemoMode, 0x00, sizeof(srEDCRec.szDemoMode));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDemoMode == NULL || strlen(szDemoMode) <= 0 || strlen(szDemoMode) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inDISP_LogPrintf("inSetDemoMode() ERROR !!");

                        if (szDemoMode == NULL)
                        {
                                inDISP_LogPrintf("szDemoMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDemoMode length = (%d)", (int)strlen(szDemoMode));
                                inDISP_LogPrintf(szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szDemoMode[0], &szDemoMode[0], strlen(szDemoMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMKOK
Date&Time       :2018/8/22 上午 11:16
Describe        :
*/
int inGetTMKOK(char* szTMKOK)
{
    /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
    if (szTMKOK == NULL || strlen(srEDCRec.szTMKOK) <=0 || strlen(srEDCRec.szTMKOK) > 2)
    {
           /* debug */
           if (ginDebug == VS_TRUE)
           {
                    char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                    inDISP_LogPrintf("inGetTMKOK() ERROR !!");

                    if (szTMKOK == NULL)
                    {
                            inDISP_LogPrintf("szTMKOK == NULL");
                    }
                    else
                    {
                            memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                            sprintf(szErrorMsg, "szTMKOK length = (%d)", (int)strlen(srEDCRec.szTMKOK));
                            inDISP_LogPrintf(szErrorMsg);
                    }
           }
           return (VS_ERROR);
    }
    memcpy(&szTMKOK[0], &srEDCRec.szTMKOK[0], strlen(srEDCRec.szTMKOK));

    return (VS_SUCCESS);
}

/*
Function        :inSetTMKOK
Date&Time       :2018/4/11 下午 3:06
Describe        :
*/
int inSetTMKOK(char* szTMKOK)
{
    memset(srEDCRec.szTMKOK, 0x00, sizeof(srEDCRec.szTMKOK));
    /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
    if (szTMKOK == NULL || strlen(szTMKOK) <= 0 || strlen(szTMKOK) > 2)
    {
            /* debug */
            if (ginDebug == VS_TRUE)
            {
                    char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                    inDISP_LogPrintf("inSetTMKOK() ERROR !!");

                    if (szTMKOK == NULL)
                    {
                            inDISP_LogPrintf("szTMKOK == NULL");
                    }
                    else
                    {
                            memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                            sprintf(szErrorMsg, "szTMKOK length = (%d)", (int)strlen(szTMKOK));
                            inDISP_LogPrintf(szErrorMsg);
                    }
            }

            return (VS_ERROR);
    }
    memcpy(&srEDCRec.szTMKOK[0], &szTMKOK[0], strlen(szTMKOK));

    return (VS_SUCCESS);
}

/*
Function        :szCUPCUPTMKOK
Date&Time       :2018/8/22 上午 11:16
Describe        :
*/
int inGetCUPTMKOK(char* szCUPTMKOK)
{
    /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
    if (szCUPTMKOK == NULL || strlen(srEDCRec.szCUPTMKOK) <=0 || strlen(srEDCRec.szCUPTMKOK) > 2)
    {
           /* debug */
           if (ginDebug == VS_TRUE)
           {
                    char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                    inDISP_LogPrintf("inGetCUPTMKOK() ERROR !!");

                    if (szCUPTMKOK == NULL)
                    {
                            inDISP_LogPrintf("szCUPTMKOK == NULL");
                    }
                    else
                    {
                            memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                            sprintf(szErrorMsg, "szCUPTMKOK length = (%d)", (int)strlen(srEDCRec.szCUPTMKOK));
                            inDISP_LogPrintf(szErrorMsg);
                    }
           }
           return (VS_ERROR);
    }
    memcpy(&szCUPTMKOK[0], &srEDCRec.szCUPTMKOK[0], strlen(srEDCRec.szCUPTMKOK));

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inGetCUPTMKOK (%s)",szCUPTMKOK);

    return (VS_SUCCESS);
}

/*
Function        :inSetCUPTMKOK
Date&Time       :2018/4/11 下午 3:06
Describe        :
*/
int inSetCUPTMKOK(char* szCUPTMKOK)
{

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inSetCUPTMKOK (%s)",szCUPTMKOK);

    memset(srEDCRec.szCUPTMKOK, 0x00, sizeof(srEDCRec.szCUPTMKOK));
    /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
    if (szCUPTMKOK == NULL || strlen(szCUPTMKOK) <= 0 || strlen(szCUPTMKOK) > 2)
    {
            /* debug */
            if (ginDebug == VS_TRUE)
            {
                    char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                    inDISP_LogPrintf("inSetCUPTMKOK() ERROR !!");

                    if (szCUPTMKOK == NULL)
                    {
                            inDISP_LogPrintf("szCUPTMKOK == NULL");
                    }
                    else
                    {
                            memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                            sprintf(szErrorMsg, "szCUPTMKOK length = (%d)", (int)strlen(szCUPTMKOK));
                            inDISP_LogPrintf(szErrorMsg);
                    }
            }

            return (VS_ERROR);
    }
    memcpy(&srEDCRec.szCUPTMKOK[0], &szCUPTMKOK[0], strlen(szCUPTMKOK));

    return (VS_SUCCESS);
}


/*
Function        :inGetCUPTPKOK
Date&Time       :2018/8/22 上午 11:16
Describe        :
*/
int inGetCUPTPKOK(char* szCUPTPKOK)
{
    /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
    if (szCUPTPKOK == NULL || strlen(srEDCRec.szCUPTPKOK) <=0 || strlen(srEDCRec.szCUPTPKOK) > 2)
    {
           /* debug */
           if (ginDebug == VS_TRUE)
           {
                    char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                    inDISP_LogPrintf("inGetCUPTPKOK() ERROR !!");

                    if (szCUPTPKOK == NULL)
                    {
                            inDISP_LogPrintf("szCUPTPKOK == NULL");
                    }
                    else
                    {
                            memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                            sprintf(szErrorMsg, "szCUPTPKOK length = (%d)", (int)strlen(srEDCRec.szCUPTPKOK));
                            inDISP_LogPrintf(szErrorMsg);
                    }
           }
           return (VS_ERROR);
    }
    memcpy(&szCUPTPKOK[0], &srEDCRec.szCUPTPKOK[0], strlen(srEDCRec.szCUPTPKOK));

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inGetCUPTPKOK (%s)",szCUPTPKOK);

    return (VS_SUCCESS);
}

/*
Function        :inSetCUPTPKOK
Date&Time       :2018/4/11 下午 3:06
Describe        :
*/
int inSetCUPTPKOK(char* szCUPTPKOK)
{

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inSetCUPTPKOK (%s)",szCUPTPKOK);

    memset(srEDCRec.szCUPTPKOK, 0x00, sizeof(srEDCRec.szCUPTPKOK));
    /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
    if (szCUPTPKOK == NULL || strlen(szCUPTPKOK) <= 0 || strlen(szCUPTPKOK) > 2)
    {
            /* debug */
            if (ginDebug == VS_TRUE)
            {
                    char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                    inDISP_LogPrintf("inSetCUPTPKOK() ERROR !!");

                    if (szCUPTPKOK == NULL)
                    {
                            inDISP_LogPrintf("szCUPTPKOK == NULL");
                    }
                    else
                    {
                            memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                            sprintf(szErrorMsg, "szCUPTPKOK length = (%d)", (int)strlen(szCUPTPKOK));
                            inDISP_LogPrintf(szErrorMsg);
                    }
            }

            return (VS_ERROR);
    }
    memcpy(&srEDCRec.szCUPTPKOK[0], &szCUPTPKOK[0], strlen(szCUPTPKOK));

    return (VS_SUCCESS);
}

/*
Function        :inGetCUPTPKOK
Date&Time       :2018/8/22 上午 11:16
Describe        :
*/
int inGetCUPLOGONOK(char* szCUPLOGONOK)
{
    /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
    if (szCUPLOGONOK == NULL || strlen(srEDCRec.szCUPLOGONOK) <=0 || strlen(srEDCRec.szCUPLOGONOK) > 2)
    {
           /* debug */
           if (ginDebug == VS_TRUE)
           {
                    char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                    inDISP_LogPrintf("inGetCUPLOGONOK() ERROR !!");

                    if (szCUPLOGONOK == NULL)
                    {
                            inDISP_LogPrintf("szCUPLOGONOK == NULL");
                    }
                    else
                    {
                            memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                            sprintf(szErrorMsg, "szCUPLOGONOK length = (%d)", (int)strlen(srEDCRec.szCUPLOGONOK));
                            inDISP_LogPrintf(szErrorMsg);
                    }
           }
           return (VS_ERROR);
    }
    memcpy(&szCUPLOGONOK[0], &srEDCRec.szCUPLOGONOK[0], strlen(srEDCRec.szCUPLOGONOK));

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inGetCUPLOGONOK (%s)",szCUPLOGONOK);

    return (VS_SUCCESS);
}

/*
Function        :inSetCUPLOGONOK
Date&Time       :2018/4/11 下午 3:06
Describe        :
*/
int inSetCUPLOGONOK(char* szCUPLOGONOK)
{

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inSetCUPLOGONOK (%s)",szCUPLOGONOK);

    memset(srEDCRec.szCUPLOGONOK, 0x00, sizeof(srEDCRec.szCUPLOGONOK));
    /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
    if (szCUPLOGONOK == NULL || strlen(szCUPLOGONOK) <= 0 || strlen(szCUPLOGONOK) > 2)
    {
            /* debug */
            if (ginDebug == VS_TRUE)
            {
                    char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                    inDISP_LogPrintf("inSetCUPLOGONOK() ERROR !!");

                    if (szCUPLOGONOK == NULL)
                    {
                            inDISP_LogPrintf("szCUPLOGONOK == NULL");
                    }
                    else
                    {
                            memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                            sprintf(szErrorMsg, "szCUPLOGONOK length = (%d)", (int)strlen(szCUPLOGONOK));
                            inDISP_LogPrintf(szErrorMsg);
                    }
            }

            return (VS_ERROR);
    }
    memcpy(&srEDCRec.szCUPLOGONOK[0], &szCUPLOGONOK[0], strlen(szCUPLOGONOK));

    return (VS_SUCCESS);
}


/*
Function        : inGetCMASFuncEnable
Date&Time   : 2022/6/14 下午 2:42
Describe        :
 * [新增電票悠遊卡功能]  是否開啟悠遊卡功能，該參數值為(Y/N)，預設為N  [SAM]
*/
int inGetCMASFuncEnable(char* szCMASFuncEnable)
{
	/* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
	if (szCMASFuncEnable == NULL || strlen(srEDCRec.szCMASFuncEnable) <=0 || strlen(srEDCRec.szCMASFuncEnable) > 2)
	{
		/* debug */
		if (ginDebug == VS_TRUE)
		{
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inGetCMASFuncEnable() ERROR !!");

			if (szCMASFuncEnable == NULL)
			{
				inDISP_LogPrintf("szCMASFuncEnable == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szCMASFuncEnable length = (%d)", (int)strlen(srEDCRec.szCMASFuncEnable));
				inDISP_LogPrintf(szErrorMsg);
			}
		}
		return (VS_ERROR);
	}
	
	memcpy(&szCMASFuncEnable[0], &srEDCRec.szCMASFuncEnable[0], strlen(srEDCRec.szCMASFuncEnable));

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inGetCMASFuncEnable (%s)",szCMASFuncEnable);

    return (VS_SUCCESS);
}

/*
Function        : inSetCMASFuncEnable
Date&Time   : 2022/6/14 下午 2:43
Describe        :
*/
int inSetCMASFuncEnable(char* szCMASFuncEnable)
{

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inSetCMASFuncEnable (%s)",szCMASFuncEnable);

	memset(srEDCRec.szCMASFuncEnable, 0x00, sizeof(srEDCRec.szCMASFuncEnable));
	/* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
	if (szCMASFuncEnable == NULL || strlen(szCMASFuncEnable) <= 0 || strlen(szCMASFuncEnable) > 2)
	{
		/* debug */
		if (ginDebug == VS_TRUE)
		{
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inSetCMASFuncEnable() ERROR !!");

			if (szCMASFuncEnable == NULL)
			{
				inDISP_LogPrintf("szCMASFuncEnable == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szCMASFuncEnable length = (%d)", (int)strlen(szCMASFuncEnable));
				inDISP_LogPrintf(szErrorMsg);
			}
		}

		return (VS_ERROR);
	}
    
	memcpy(&srEDCRec.szCMASFuncEnable[0], &szCMASFuncEnable[0], strlen(szCMASFuncEnable));

	return (VS_SUCCESS);
}

/*
Function        : inGetCMASLinkMode
Date&Time   : 2022/6/14 下午 2:42
Describe        :
 *  [新增電票悠遊卡功能] 悠遊卡連線模式 1 = 間連  0 = 直連 [SAM]
*/
int inGetCMASLinkMode(char* szCMASLinkMode)
{
	/* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
	if (szCMASLinkMode == NULL || strlen(srEDCRec.szCMASLinkMode) <=0 || strlen(srEDCRec.szCMASLinkMode) > 2)
	{
		/* debug */
		if (ginDebug == VS_TRUE)
		{
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inGetCMASLinkMode() ERROR !!");

			if (szCMASLinkMode == NULL)
			{
				inDISP_LogPrintf("szCMASLinkMode == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szCMASLinkMode length = (%d)", (int)strlen(srEDCRec.szCMASLinkMode));
				inDISP_LogPrintf(szErrorMsg);
			}
		}
		return (VS_ERROR);
	}
	
	memcpy(&szCMASLinkMode[0], &srEDCRec.szCMASLinkMode[0], strlen(srEDCRec.szCMASLinkMode));

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inGetCMASLinkMode (%s)",szCMASLinkMode);

    return (VS_SUCCESS);
}

/*
Function        : inSetCMASLinkMode
Date&Time   : 2022/6/14 下午 2:43
Describe        :
*/
int inSetCMASLinkMode(char* szCMASLinkMode)
{

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inSetCMASLinkMode (%s)",szCMASLinkMode);

	memset(srEDCRec.szCMASLinkMode, 0x00, sizeof(srEDCRec.szCMASLinkMode));
	/* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
	if (szCMASLinkMode == NULL || strlen(szCMASLinkMode) <= 0 || strlen(szCMASLinkMode) > 2)
	{
		/* debug */
		if (ginDebug == VS_TRUE)
		{
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inSetCMASLinkMode() ERROR !!");

			if (szCMASLinkMode == NULL)
			{
				inDISP_LogPrintf("szCMASLinkMode == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szCMASLinkMode length = (%d)", (int)strlen(szCMASLinkMode));
				inDISP_LogPrintf(szErrorMsg);
			}
		}

		return (VS_ERROR);
	}
    
	memcpy(&srEDCRec.szCMASLinkMode[0], &szCMASLinkMode[0], strlen(szCMASLinkMode));

	return (VS_SUCCESS);
}


/*
Function        : inGetCMASAddTcpipMode
Date&Time   : 2022/6/14 下午 2:42
Describe        :
 *  [新增電票悠遊卡功能] 悠遊卡連線模式 1 = 間連  0 = 直連 [SAM]
*/
int inGetCMASAddTcpipMode(char* szCMASAddTcpipMode)
{
	/* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
	if (szCMASAddTcpipMode == NULL || strlen(srEDCRec.szCMASAddTcpipMode) <=0 || strlen(srEDCRec.szCMASAddTcpipMode) > 2)
	{
		/* debug */
		if (ginDebug == VS_TRUE)
		{
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inGetCMASAddTcpipMode() ERROR !!");

			if (szCMASAddTcpipMode == NULL)
			{
				inDISP_LogPrintf("szCMASAddTcpipMode == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szCMASAddTcpipMode length = (%d)", (int)strlen(srEDCRec.szCMASAddTcpipMode));
				inDISP_LogPrintf(szErrorMsg);
			}
		}
		return (VS_ERROR);
	}
	
	memcpy(&szCMASAddTcpipMode[0], &srEDCRec.szCMASAddTcpipMode[0], strlen(srEDCRec.szCMASAddTcpipMode));

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inGetCMASAddTcpipMode (%s)",szCMASAddTcpipMode);

	return (VS_SUCCESS);
}

/*
Function        : inSetCMASAddTcpipMode
Date&Time   : 2022/6/14 下午 2:43
Describe        :
*/
int inSetCMASAddTcpipMode(char* szCMASAddTcpipMode)
{

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inSetCMASAddTcpipMode (%s)",szCMASAddTcpipMode);

	memset(srEDCRec.szCMASAddTcpipMode, 0x00, sizeof(srEDCRec.szCMASAddTcpipMode));
	/* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
	if (szCMASAddTcpipMode == NULL || strlen(szCMASAddTcpipMode) <= 0 || strlen(szCMASAddTcpipMode) > 2)
	{
		/* debug */
		if (ginDebug == VS_TRUE)
		{
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inDISP_LogPrintf("inSetCMASAddTcpipMode() ERROR !!");

			if (szCMASAddTcpipMode == NULL)
			{
				inDISP_LogPrintf("szCMASAddTcpipMode == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szCMASAddTcpipMode length = (%d)", (int)strlen(szCMASAddTcpipMode));
				inDISP_LogPrintf(szErrorMsg);
			}
		}

		return (VS_ERROR);
	}
    
	memcpy(&srEDCRec.szCMASAddTcpipMode[0], &szCMASAddTcpipMode[0], strlen(szCMASAddTcpipMode));

	return (VS_SUCCESS);
}


/*
Function        :inEDC_Edit_EDC_Table
Date&Time       :2017/5/9 下午 4:21
Describe        :
*/
int inEDC_Edit_EDC_Table(void)
{
	TABLE_GET_SET_TABLE EDC_FUNC_TABLE[] =
	{
		{"szCUPFuncEnable"		,inGetCUPFuncEnable			,inSetCUPFuncEnable			},
		{"szFiscFuncEnable"		,inGetFiscFuncEnable			,inSetFiscFuncEnable			},
		{"szECRComPort"		,inGetECRComPort			,inSetECRComPort			},
		{"szECRVersion"			,inGetECRVersion			,inSetECRVersion			},
		{"szPABXCode"			,inGetPABXCode				,inSetPABXCode				},
		{"szTSAMRegisterEnable"	,inGetTSAMRegisterEnable		,inSetTSAMRegisterEnable		},
		{"szTMSOK"			,inGetTMSOK				,inSetTMSOK				},
		{"szDCCInit"			,inGetDCCInit				,inSetDCCInit				},
		{"szDCCDownloadMode"	,inGetDCCDownloadMode		,inSetDCCDownloadMode			},
		{"szDCCLastUpdateDate"	,inGetDCCLastUpdateDate		,inSetDCCLastUpdateDate			},
		{"szDCCSettleDownload"	,inGetDCCSettleDownload		,inSetDCCSettleDownload			},
		{"szDCCBinVer"			,inGetDCCBinVer			,inSetDCCBinVer				},
		{"szEDCLOCK"			,inGetEDCLOCK				,inSetEDCLOCK				},
		{"szISODebug"			,inGetISODebug				,inSetISODebug				},
		{"szManagerPassword"	,inGetManagerPassword		,inSetManagerPassword			},
		{"szFunctionPassword"	,inGetFunctionPassword		,inSetFunctionPassword			},
		{"szMerchantPassword"	,inGetMerchantPassword		,inSetMerchantPassword			},
		{"szSuperPassword"		,inGetSuperPassword			,inSetSuperPassword			},
		{"szMCCCode"			,inGetMCCCode				,inSetMCCCode				},
		{"szIssuerID"			,inGetIssuerID				,inSetIssuerID				},
		{"szEnterTimeout"		,inGetEnterTimeout			,inSetEnterTimeout			},
		{"szIPSendTimeout"		,inGetIPSendTimeout			,inSetIPSendTimeout			},
		{"szTermVersionID"		,inGetTermVersionID			,inSetTermVersionID			},
		{"szTermVersionDate"		,inGetTermVersionDate		,inSetTermVersionDate			},
		{"szTermIPAddress"		,inGetTermIPAddress			,inSetTermIPAddress			},
		{"szTermGetewayAddress"	,inGetTermGetewayAddress		,inSetTermGetewayAddress		},
		{"szTermMASKAddress"	,inGetTermMASKAddress		,inSetTermMASKAddress			},
		{"szTermECRPort"		,inGetTermECRPort			,inSetTermECRPort			},
		{"szESCMode"			,inGetESCMode				,inSetESCMode				},
		{"szMultiComPort1"		,inGetMultiComPort1			,inSetMultiComPort1			},
		{"szMultiComPort1Version"	,inGetMultiComPort1Version	,inSetMultiComPort1Version		},
		{"szMultiComPort2"		,inGetMultiComPort2			,inSetMultiComPort2			},
		{"szMultiComPort2Version"	,inGetMultiComPort2Version	,inSetMultiComPort2Version		},
		{"szMultiComPort3"		,inGetMultiComPort3			,inSetMultiComPort3			},
		{"szMultiComPort3Version"	,inGetMultiComPort3Version	,inSetMultiComPort3Version		},
		{"szEMVForceOnline"		,inGetEMVForceOnline		,inSetEMVForceOnline			},
		{"szAutoConnect"		,inGetAutoConnect			,inSetAutoConnect			},
		{"szLOGONum"			,inGetLOGONum			,inSetLOGONum				},
		{"szHostSAMSlot"		,inGetHostSAMSlot			,inSetHostSAMSlot			},
		{"szSAMSlotSN1"		,inGetSAMSlotSN1			,inSetSAMSlotSN1			},
		{"szSAMSlotSN2"		,inGetSAMSlotSN2			,inSetSAMSlotSN2			},
		{"szSAMSlotSN3"		,inGetSAMSlotSN3			,inSetSAMSlotSN3			},
		{"szSAMSlotSN4"		,inGetSAMSlotSN4			,inSetSAMSlotSN4			},
		{"szPWMEnable"			,inGetPWMEnable			,inSetPWMEnable				},
		{"szPWMMode"			,inGetPWMMode			,inSetPWMMode				},
		{"szPWMIdleTimeout"		,inGetPWMIdleTimeout		,inSetPWMIdleTimeout			},
		{"szDemoMode"			,inGetDemoMode			,inSetDemoMode				},
		{"szTMKOK"			,inGetTMKOK				,inSetTMKOK					},
		{"szCUPTMKOK"			,inGetCUPTMKOK			,inSetCUPTMKOK				},
		{"szCUPTPKOK"			,inGetCUPTPKOK			,inSetCUPTPKOK				},
		{"szCMASFuncEnable"		,inGetCMASFuncEnable		,inSetCMASFuncEnable			},
		{"szCMASLinkMode"		,inGetCMASLinkMode			,inSetCMASLinkMode			},
		{"szCMASAddTcpipMode"	,inGetCMASAddTcpipMode		,inSetCMASAddTcpipMode			},
		{""},
	};
	int	inRetVal;
	char	szKey;


	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont_Color("是否更改EDC", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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

	inFunc_Edit_Table_Tag(EDC_FUNC_TABLE);
	inSaveEDCRec(0);

	return	(VS_SUCCESS);
}



