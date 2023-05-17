#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <unistd.h>
#include <sqlite3.h>

#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"
#include "../EVENT/Flow.h"
#include "../EVENT/MenuMsg.h"
#include "Function.h"
#include "Sqlite.h"
#include "Accum.h"
#include "FILE_FUNC/File.h"
#include "FILE_FUNC/FIleLogFunc.h"
#include "HDT.h"
#include "Batch.h"
#include "CDT.h"
#include "CFGT.h"
#include "HDPT.h"
#include "EDC.h"
#include "Card.h"
#include "Signpad.h"
//#include "../../CREDIT/Creditprt.h"	/* 有Accum的結構 */
//#include "../../NCCC/NCCCsrc.h"
#include "../../NCCC/NCCCTicketSrc.h"
//#include "../../NCCC/NCCCesc.h"
//#include "../../NCCC/NCCCloyalty.h"

#include "CARD_FUNC/CardFunction.h"
#include "UNIT_FUNC/TimeUint.h"


extern int ginDebug; /* Debug使用 extern */
unsigned long gulADVHandle, gulBATHandle, gulBKEYHandle;
unsigned char guszBATFileName[20 + 1], guszBKEYFileName[20 + 1], guszADVFileName[20 + 1];

/*
Function        :inBATCH_FuncUpdateTxnRecord
Date&Time       :2015/9/8 下午 16:00
Describe        :update到.bkey檔, 交易存檔檔案名稱【最多 12 Byte】 = (Host Name + Batch Number + .Bkey)
 */
int inBATCH_FuncUpdateTxnRecord(TRANSACTION_OBJECT *pobTran)
{
	unsigned char uszBATFileName[14 + 1]; /* 檔名(含附檔名)最長可到15個字 */
	unsigned long ulBATHandle;
	TRANS_BATCH_KEY srBKeyRec;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_FuncUpdateTxnRecord()_START");
	}

	if (inFunc_ComposeFileName(pobTran, (char*) uszBATFileName, _BATCH_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	memset(&srBKeyRec, 0x00, sizeof (srBKeyRec));

	/* Open BAT File */
	if (inFILE_Open(&ulBATHandle, uszBATFileName) == VS_ERROR)
	{
		/* 開檔失敗時，不必關檔(開檔失敗，handle回傳NULL) */
		/* 若檔案不存在時，Create檔案 */
		if (inFILE_Check_Exist(uszBATFileName) == (VS_ERROR))
		{
			/* Create檔 */
			if (inFILE_Create(&ulBATHandle, uszBATFileName) == (VS_ERROR))
			{
				/* Create檔失敗時 */
				/* Create檔失敗時不關檔，因為create檔失敗handle回傳NULL */
				return (VS_ERROR);
			}
		}/* 檔案存在還是開檔失敗，回傳錯誤跳出 */
		else
		{
			return (VS_ERROR);
		}

		/* create檔成功就繼續(因為create檔已經把handle指向檔案，所以不用在開檔) */
	}

	/* srBKeyRec.lnBatchRecStart儲存寫檔的起始點 */
	srBKeyRec.lnBatchRecStart = lnFILE_GetSize(&ulBATHandle, uszBATFileName);
	/* 計算要寫入的長度 */
	srBKeyRec.lnBatchRecSize = _BATCH_REC_SIZE_;

	/* 算出是否有【EMV】和【Contactless】交易 */
	if (pobTran->srBRec.inChipStatus != 0 || pobTran->srBRec.uszContactlessBit == VS_TRUE || pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
		srBKeyRec.lnBatchRecSize += _EMV_REC_SIZE_; /* 記憶體 -> 要存檔的 Size */

	/* 存檔 */
	/* 先seek到最後的位置，因為【.bat】永遠都是Append */
	if (inFILE_Seek(ulBATHandle, 0, _SEEK_END_) == VS_ERROR)
	{
		/* inFILE_Seek失敗時 */
		/* Seek檔失敗，所以關檔 */
		inFILE_Close(&ulBATHandle);

		/* Seek檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
		return (VS_ERROR);
	}

	if (inFILE_Write(&ulBATHandle, (unsigned char *) &pobTran->srBRec, srBKeyRec.lnBatchRecSize) == VS_ERROR)
	{
		/* 讀檔失敗 */
		/* Read檔失敗，所以關檔 */
		inFILE_Close(&ulBATHandle);

		/* Read檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
		return (VS_ERROR);
	}

	/* 關檔 */
	if (inFILE_Close(&ulBATHandle) == VS_ERROR)
	{
		/* 停1秒再關一次 */
		inDISP_Wait(1000);

		/* 若再關還是失敗，回傳VS_ERROR */
		if (inFILE_Close(&ulBATHandle) == VS_ERROR)
			return (VS_ERROR);

	}

	/* 存TRANS_BATCH_KEY到【.bkey】檔，【.bkey】檔會存交易紀錄的記憶體位置 */
	/* 以後是透過【.bkey】檔去抓取【.bat】檔的交易紀錄 */
	/* 1. 存交易記錄的TRANS_BATCH_KEY
	   2. 先取得TRANS_BATCH_KEY Index 再做存檔 */
	if (inBATCH_StoreBatchKeyFile(pobTran, &srBKeyRec) == VS_ERROR)
	{
		/* 存檔失敗 */
		/* 因為inBATCH_StoreBatchKeyFile失敗，所以回傳VS_ERROR */
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_FuncUpdateTxnRecord()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_StoreBatchKeyFile
Date&Time       :2015/9/8 下午 16:00
Describe        :把srBKeyRec存入.bkey
 */
int inBATCH_StoreBatchKeyFile(TRANSACTION_OBJECT *pobTran, TRANS_BATCH_KEY *srBKeyRec)
{
	long lnSearchStart, lnSearchEnd, lnSearchIndex = 0;
	long lnOffset, lnBatchKeySize;
	long lnRecTotalCnt = 0; /* bkey檔案的total record number */
	long lnBatchRecStart, lnBatchRecSize;
	long lnRecTotalSize; /* bkey檔案的total size */
	char szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1]; /* debug message */
	unsigned char uszBKEYFileName[15 + 1]; /* 檔名(含附檔名)最長可到15個字 */
	unsigned long ulBKEYHandle; /* File Handle */

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_StoreBatchKeyFile()_START");
	}

	if (inFunc_ComposeFileName(pobTran, (char*) uszBKEYFileName, _BATCH_KEY_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* Open Batch Key File */
	if (inFILE_Open(&ulBKEYHandle, uszBKEYFileName) == VS_ERROR)
	{
		/* 開檔失敗時，不必關檔(開檔失敗，handle回傳NULL) */
		/* 若檔案不存在時，Create檔案 */
		if (inFILE_Check_Exist(uszBKEYFileName) == (VS_ERROR))
		{
			if (inFILE_Create(&ulBKEYHandle, uszBKEYFileName) == VS_ERROR)
			{
				/* Create檔失敗時不關檔，因為create檔失敗handle回傳NULL */
				return (VS_ERROR);
			}
		}			/* 檔案存在還是開檔失敗，回傳錯誤跳出 */
		else
		{
			return (VS_ERROR);
		}

	}

	/* 從srBKeyRec->lnBatchRecStart取出記憶體起始位置 */
	lnBatchRecStart = srBKeyRec->lnBatchRecStart;
	/* 算出要存的BATCH_REC的大小 */
	lnBatchRecSize = srBKeyRec->lnBatchRecSize;
	/* 取出【.bkey】的大小 */
	lnRecTotalSize = lnFILE_GetSize(&ulBKEYHandle, uszBKEYFileName);
	/* 算出目前【.bkey】的總筆數 */
	srBKeyRec->lnSearchIndex = lnRecTotalCnt = (lnRecTotalSize / _BATCH_KEY_SIZE_);
	/* Update Invoice Number */
	srBKeyRec->lnInvoiceNo = pobTran->srBRec.lnOrgInvNum;

	/* 判斷【.bkey】是要Update還是Append */
	/* pobTran->uszUpdateBatchBit 表示是要更新記錄 */
	if (pobTran->uszUpdateBatchBit == VS_TRUE)
	{
		lnSearchStart = 1;
		lnSearchEnd = lnRecTotalCnt;

		if (lnRecTotalCnt == 1)
			lnSearchIndex = 0;
		else
			lnSearchIndex = (lnRecTotalCnt / 2); /* 二分法 */

		while (1)
		{
			lnBatchKeySize = _BATCH_KEY_SIZE_;

			/* 表示是要 Get 最後一筆記錄 */
			if (pobTran->srBRec.lnOrgInvNum == _BATCH_LAST_RECORD_)
				lnOffset = ((lnRecTotalCnt - 1) * lnBatchKeySize);
			else
				lnOffset = lnSearchIndex * lnBatchKeySize;

			if (inFILE_Seek(ulBKEYHandle, lnOffset, _SEEK_BEGIN_) == VS_ERROR)
			{
				/* inFILE_Seek失敗時 */
				/* Seek檔失敗，所以關檔 */
				inFILE_Close(&ulBKEYHandle);

				/* Seek檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
				return (VS_ERROR);
			}

			/* 一次讀取BATCH_KEY_SIZE的大小 */
			if (inFILE_Read(&ulBKEYHandle, (unsigned char *) srBKeyRec, _BATCH_KEY_SIZE_) == VS_ERROR)
			{
				/* 讀檔失敗 */
				/* Read檔失敗，所以關檔 */
				inFILE_Close(&ulBKEYHandle);

				/* Read檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
				return (VS_ERROR);
			}

			/* 表示是要 Get 最後一筆記錄 */
			if (pobTran->srBRec.lnOrgInvNum == _BATCH_LAST_RECORD_)
			{
				pobTran->srBRec.lnOrgInvNum = srBKeyRec->lnInvoiceNo;
				break;
			}

			/* 判斷讀到的資料是否正確 */
			if (pobTran->srBRec.lnOrgInvNum == srBKeyRec->lnInvoiceNo)
			{
				break;
			} else
			{
				if (srBKeyRec->lnInvoiceNo > pobTran->srBRec.lnOrgInvNum)
					lnSearchEnd = lnSearchIndex; /* 往左邊找 */
				else
					lnSearchStart = lnSearchIndex; /* 往右邊找 */
			}

			lnSearchIndex = (lnSearchStart + lnSearchEnd) / 2;

			if (lnSearchIndex == 1 && lnSearchEnd == 1)
				lnSearchIndex = 0;

			/* 沒有找到 Record 所以給一個假的lnOrgInvNum */
			if (lnRecTotalCnt < 0)
			{
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf(" BATCH_KEY SEARCH_NOT_FOUND RETURN ERR");
				}

				pobTran->srBRec.lnOrgInvNum = _BATCH_INVALID_RECORD_;
				break;
			}

			lnRecTotalCnt--;
			continue;
		}

		/* 看到錯誤在這裡修正 */
		if (srBKeyRec->lnInvoiceNo != pobTran->srBRec.lnOrgInvNum)
		{
			inFILE_Close(&ulBKEYHandle);
			return (VS_ERROR);
		}

		/* 原交易的資料 */
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf(" BATCH_KEY UPDATE");
		}

		srBKeyRec->lnBatchRecStart = lnBatchRecStart; /* 算出 BATCH_REC 在記憶體的啟始位置 */
		srBKeyRec->lnBatchRecSize = lnBatchRecSize; /* 算出要存 BATCH_REC 的大小 */
	} else
	{
		srBKeyRec->lnBatchRecStart = lnBatchRecStart; /* 算出 BATCH_REC 在記憶體的啟始位置 */
		srBKeyRec->lnBatchRecSize = lnBatchRecSize; /* 算出要存 BATCH_REC 的大小 */
		srBKeyRec->lnSearchIndex++; /* BATCH_REC 交易的總筆數 */

		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf(" BATCH_KEY APPEND");
		}
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szErrorMsg, 0x00, sizeof (szErrorMsg));
		sprintf(szErrorMsg, "InvoiceNo[BKEY : %ld]", srBKeyRec->lnInvoiceNo);
		inDISP_LogPrintf(szErrorMsg);

		memset(szErrorMsg, 0x00, sizeof (szErrorMsg));
		sprintf(szErrorMsg, "[SEARCH : %ld]", pobTran->srBRec.lnOrgInvNum);
		inDISP_LogPrintf(szErrorMsg);

		memset(szErrorMsg, 0x00, sizeof (szErrorMsg));
		sprintf(szErrorMsg, "srBKeyRec->lnBatchRecStart : [%ld]", srBKeyRec->lnBatchRecStart);
		inDISP_LogPrintf(szErrorMsg);
		memset(szErrorMsg, 0x00, sizeof (szErrorMsg));
		sprintf(szErrorMsg, "srBKeyRec->lnBatchRecSize : [%ld]", srBKeyRec->lnBatchRecSize);
		inDISP_LogPrintf(szErrorMsg);
	}

	/* 要先算出 TRANS_BATCH_KEY 在記憶體的大小，因為是從【0】開始 */
	lnOffset = (srBKeyRec->lnSearchIndex - 1) * _BATCH_KEY_SIZE_;

	/* 先指到要存 TRANS_BATCH_KEY 的位置 */
	if (inFILE_Seek(ulBKEYHandle, lnOffset, _SEEK_BEGIN_) == VS_ERROR)
	{
		/* inFILE_Seek失敗時 */
		/* Seek檔失敗，所以關檔 */
		inFILE_Close(&ulBKEYHandle);

		/* Seek檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
		return (VS_ERROR);
	}

	/* 存檔到BAK */
	if (inFILE_Write(&ulBKEYHandle, (unsigned char *) srBKeyRec, _BATCH_KEY_SIZE_) == VS_ERROR)
	{
		/* 寫檔失敗時 */
		/* 關檔  */
		inFILE_Close(&ulBKEYHandle);
		/* 寫檔失敗，所以回傳error。(關檔不論成功與否都要回傳(VS_ERROR)) */
		return (VS_ERROR);
	}

	/* 關檔 */
	if (inFILE_Close(&ulBKEYHandle) == VS_ERROR)
	{
		/* 停1秒再關一次 */
		inDISP_Wait(1000);

		if (inFILE_Close(&ulBKEYHandle) == VS_ERROR)
		{
			/* 若再關還是失敗，回傳VS_ERROR */
			return (VS_ERROR);
		}
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_StoreBatchKeyFile()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_SearchRecord
Date&Time       :2015/9/8 下午 16:00
Describe        :用二分法找到BAK的資料
 */
int inBATCH_SearchRecord(TRANSACTION_OBJECT *pobTran, TRANS_BATCH_KEY *srBatchKeyRec, unsigned long ulBKEYHandle, int inRecTotalCnt)
{
	long lnSearchStart, lnSearchEnd, lnSearchIndex = 0;
	long lnOffset, lnBatchKeySize;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_SearchRecord()_START");
	}

	lnSearchStart = 1;
	lnSearchEnd = inRecTotalCnt;

	if (inRecTotalCnt == 1)
		lnSearchIndex = 0;
	else
		lnSearchIndex = (inRecTotalCnt / 2); /* 二分法 */


	while (1)
	{
		lnBatchKeySize = _BATCH_KEY_SIZE_;

		/* 表示是要 Get 最後一筆記錄 */
		if (pobTran->srBRec.lnOrgInvNum == _BATCH_LAST_RECORD_)
		{
			lnOffset = (long) (((long) (inRecTotalCnt - 1) * lnBatchKeySize));
		} else
		{
			lnOffset = lnSearchIndex * lnBatchKeySize;
		}

		/* seek .bkey */
		if (inFILE_Seek(ulBKEYHandle, lnOffset, _SEEK_BEGIN_) == VS_ERROR)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("inBATCH_SearchRecord() seek bkey error!");
			}

			return (VS_ERROR);
		}

		/* 一次讀取BATCH_KEY_SIZE的大小 */
		if (inFILE_Read(&ulBKEYHandle, (unsigned char *) srBatchKeyRec, _BATCH_KEY_SIZE_) == VS_ERROR)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("inBATCH_SearchRecord() read bkey error!");
			}

			return (VS_ERROR);
		}

		/* 表示是要 Get 最後一筆記錄 */
		if (pobTran->srBRec.lnOrgInvNum == _BATCH_LAST_RECORD_)
		{
			pobTran->srBRec.lnOrgInvNum = srBatchKeyRec->lnInvoiceNo;
			break;
		}

		/* 判斷讀到的資料是否正確 */
		if (pobTran->srBRec.lnOrgInvNum == srBatchKeyRec->lnInvoiceNo)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("SEARCH_BATCH_KEY_REC_OK!");
			}

			break;
		} else
		{
			if (srBatchKeyRec->lnInvoiceNo > pobTran->srBRec.lnOrgInvNum)
			{
				/* 往左邊找 */
				lnSearchEnd = lnSearchIndex;
			} else
			{
				/* 往右邊找 */
				lnSearchStart = lnSearchIndex;
			}
		}

		lnSearchIndex = (lnSearchStart + lnSearchEnd) / 2;

		if (lnSearchIndex == 1 && lnSearchEnd == 1)
			lnSearchIndex = 0;

		/* 沒有找到 Record 所以給一個假的lnOrgInvNum */
		if (inRecTotalCnt < 0)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("inBATCH_SearchRecord() ERROR!");
			}

			pobTran->srBRec.lnOrgInvNum = _BATCH_INVALID_RECORD_;
			break;
		}

		inRecTotalCnt--;
	} /* end while loop */

	if (srBatchKeyRec->lnInvoiceNo != pobTran->srBRec.lnOrgInvNum)
		return (VS_ERROR);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_SearchRecord()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_CheckReport
Date&Time       :2015/9/8 下午 16:00
Describe        :處理 inBATCH_GetTotalCountFromBakFile()的錯誤訊息
 */
int inBATCH_CheckReport(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_CheckReport()_START");
	}

	inRetVal = inBATCH_GetTotalCountFromBakFile(pobTran);
	if (inRetVal == VS_NO_RECORD)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 顯示無交易紀錄 */
		inDISP_Msg_BMP(_ERR_RECORD_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, 2, "", 0);

		return (VS_ERROR);
	} else if (inRetVal == VS_ERROR)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 顯示讀取批次檔錯誤 */
		inDISP_Msg_BMP(_ERR_BATCH_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, 2, "", 0);

		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_CheckReport()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GetTotalCountFromBakFile
Date&Time       :2015/9/8 下午 16:00
Describe        :檢查bkey有幾筆record，回傳值為筆數
 */
int inBATCH_GetTotalCountFromBakFile(TRANSACTION_OBJECT *pobTran)
{
	unsigned char uszFileName[15 + 1]; /* 檔名(含附檔名)最長可到15個字 */
	int inBAKTotalCnt = 0; /* 檔案總筆數 */
	long lnBAKTotalFileSize = 0;
	unsigned long ulBKEYHandle; /* File Handle */

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GetTotalCountFromBakFile()_START");
	}

	if (inFunc_ComposeFileName(pobTran, (char*) uszFileName, _BATCH_KEY_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* ingenico 會刪除空白的檔案 */
	if (inFILE_OpenReadOnly(&ulBKEYHandle, uszFileName) == VS_ERROR)
	{
		/* 表示BAK沒有資料 */
		/* 開檔錯誤，確認是否有檔案，若有檔案仍錯誤，則可能是handle的問題 */
		if (inFILE_Check_Exist(uszFileName) != (VS_ERROR))
		{
			/* 回傳error */
			return (VS_ERROR);
		}			/*如果沒檔案時，則為沒有交易紀錄，回傳NO_RECORD並印空簽單*/
		else
		{
			return (VS_NO_RECORD);
		}
	}

	/* 算出 TRANS_BATCH_KEY 總合 */
	lnBAKTotalFileSize = lnFILE_GetSize(&ulBKEYHandle, uszFileName);
	/* 算出交易總筆數，因為lnFILE_GetSize回傳值為long，因為此函式回傳int所以強制轉型，因為筆數不會超過int大小 */
	inBAKTotalCnt = (int) (lnBAKTotalFileSize / _BATCH_KEY_SIZE_);

	/* 關檔 */
	if (inFILE_Close(&ulBKEYHandle) == VS_ERROR)
	{
		/* 沒關掉，再關一次 */
		if (inFILE_Close(&ulBKEYHandle) == VS_ERROR)
		{
			return (VS_ERROR);
		}
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GetTotalCountFromBakFile()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (inBAKTotalCnt);
}

/*
Function        :inBATCH_OpenBatchKeyFile
Date&Time       :2015/9/8 下午 16:00
Describe        :開bak檔案(ReadOnly)，假如bak沒有資料會回傳VS_NO_RECORD，有值的話會回傳handle
 */
int inBATCH_OpenBatchKeyFile(TRANSACTION_OBJECT *pobTran, unsigned long ulBKEYHandle)
{
	unsigned char uszFileName[15 + 1]; /* 檔名(含附檔名)最長可到15個字 */

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_OpenBatchKeyFile()_START");
	}

	if (inFunc_ComposeFileName(pobTran, (char*) uszFileName, _BATCH_KEY_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* ingenico會刪除空白的檔案 */
	/* open bkey file */
	if (inFILE_OpenReadOnly(&ulBKEYHandle, uszFileName) == VS_ERROR)
	{
		/* 表示bkey沒有資料 */
		return (VS_NO_RECORD);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_OpenBatchKeyFile()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_OpenBatchRecFile
Date&Time       :2015/9/8 下午 16:00
Describe        :開bat檔案(ReadOnly)，有值的話會回傳handle
 */
int inBATCH_OpenBatchRecFile(TRANSACTION_OBJECT *pobTran, unsigned long ulBATHandle)
{
	unsigned char uszFileName[15 + 1]; /* 檔名(含附檔名)最長可到15個字 */

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_OpenBatchRecFile()_START");
	}

	if (inFunc_ComposeFileName(pobTran, (char*) uszFileName, _BATCH_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}


	/* ingenico會刪除空白的檔案 */
	/* open bat file */
	if (inFILE_OpenReadOnly(&ulBATHandle, uszFileName) == VS_ERROR)
	{
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_OpenBatchRecFile()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_ReviewReport_Detail
Date&Time       :2015/9/8 下午 16:00
Describe        :交易查詢使用
 */
int inBATCH_ReviewReport_Detail(TRANSACTION_OBJECT *pobTran)
{
	int i = 0;
	int inFileRecCnt = 0;
	int inReviewReportResult = VS_TRUE;
	int inTouchPage = 0;
	char szTemplate1[42 + 1];
	char szTemplate2[42 + 1];
	char szFinalPAN[_PAN_UCARD_SIZE_ + 1];
	char szDispMsg [50 + 1];
	char szDebugMsg[100 + 1];
	char szKey = 0x00;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBatch_ReviewReport()_START");
	}

	/* 算出交易總筆數 */
	inFileRecCnt = inBATCH_GetTotalCountFromBakFile(pobTran);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "筆數: %d", inFileRecCnt);
		inDISP_LogPrintf(szDebugMsg);
	}

	i = 0;
	while (i < inFileRecCnt)
	{
		/* 開始讀取每一筆交易記錄 */
		if (inBATCH_GetDetailRecords(pobTran, i) != VS_SUCCESS)
		{
			inReviewReportResult = VS_FALSE;
			break;
		}

#ifdef _ESUN_MAIN_HOST_

		inDISP_LogPrintfWithFlag("  BATCH_Rv uszVOIDBit[%d]  inOrgCode[%d ] Line[%d] ", pobTran->srBRec.uszVOIDBit, pobTran->srBRec.inOrgCode, __LINE__);

		/* 玉山自助規格，如有做過AUTH COMP，原AUTH不可顯示 2021/5/19 上午 11:14 [SAM] */
		if (pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode == _PRE_AUTH_)
		{
			if (inTouchPage == _DisTouch_Slide_Right_To_Left_ || szKey == _KEY_ENTER_ || szKey == _KEY_DOWN_)
			{
				i++;
				continue;
			} else if (inTouchPage == _DisTouch_Slide_Left_To_Right_ || szKey == _KEY_UP_)
			{
				i--;

				/* 交易紀錄編號不可小於0 */
				if (i < 0)
				{
					i = 0;
					/* 加這行是因為，若第一筆是DCC轉台幣會陷入無限迴圈，所以改滑動方向 */
					inTouchPage = _DisTouch_Slide_Right_To_Left_;
					szKey = _KEY_ENTER_;
				}
				continue;
			}				/* 若第一筆就是要跳過的紀錄，會因為沒有event，而沒有跳過，所以預設為下一頁 */
			else
			{
				i++;
				continue;
			}

		}
#else                
		/* DCC轉台幣的紀錄要特別處理，原先的那筆DCC一般交易不能出現 */
		if (pobTran->srBRec.inCode == _VOID_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)
		{
			if (inTouchPage == _DisTouch_Slide_Right_To_Left_ || szKey == _KEY_ENTER_ || szKey == _KEY_DOWN_)
			{
				i++;
				continue;
			} else if (inTouchPage == _DisTouch_Slide_Left_To_Right_ || szKey == _KEY_UP_)
			{
				i--;

				/* 交易紀錄編號不可小於0 */
				if (i < 0)
				{
					i = 0;
					/* 加這行是因為，若第一筆是DCC轉台幣會陷入無限迴圈，所以改滑動方向 */
					inTouchPage = _DisTouch_Slide_Right_To_Left_;
					szKey = _KEY_ENTER_;
				}
				continue;
			}				/* 若第一筆就是要跳過的紀錄，會因為沒有event，而沒有跳過，所以預設為下一頁 */
			else
			{
				i++;
				continue;
			}

		}
#endif
		inDISP_ClearAll();

		if (inReviewReportResult == VS_FALSE)
			break;

		/* Disp Card Label & Expire date*/
		memset(szDispMsg, 0x00, sizeof (szDispMsg));
		memset(szTemplate1, 0x00, sizeof (szTemplate1));
		inGetTRTFileName(szTemplate1);
		/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
		/* 金融卡沒有效期 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			sprintf(szDispMsg, " %s", pobTran->srBRec.szCardLabel);
		}			/* 其他加密 */
		else
		{
			memset(szTemplate1, 0x00, sizeof (szTemplate1));
			inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
			sprintf(szDispMsg, " %s   %s", pobTran->srBRec.szCardLabel, szTemplate1);
		}

		inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_1_, _DISP_LEFT_);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "CardLabel: %s", pobTran->srBRec.szCardLabel);
			inDISP_LogPrintf(szDebugMsg);
		}

		/* Disp Card Number */
		memset(szDispMsg, 0x00, sizeof (szDispMsg));
		memset(szFinalPAN, 0x00, sizeof (szFinalPAN));
		strcpy(szFinalPAN, pobTran->srBRec.szPAN);

		if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
		{
			inFunc_FormatPAN_UCARD(szFinalPAN);
		}
		sprintf(szDispMsg, " %s", szFinalPAN);
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "szPAN: %s", pobTran->srBRec.szPAN);
			inDISP_LogPrintf(szDebugMsg);
		}

		/* Transaction Type */
		memset(szDispMsg, 0x00, sizeof (szDispMsg));
		memset(szTemplate1, 0x00, sizeof (szTemplate1));
		memset(szTemplate2, 0x00, sizeof (szTemplate2));
		inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
		if (strlen(szTemplate2) > 0)
		{
			sprintf(szDispMsg, " %s", szTemplate1);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_5_, _DISP_LEFT_);
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, " %s", szTemplate2);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_6_, _DISP_LEFT_);
		} else
		{
			sprintf(szDispMsg, " %s %s", szTemplate1, szTemplate2);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		}


		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Tran Type: %s %s", szTemplate1, szTemplate2);
			inDISP_LogPrintf(szDebugMsg);
		}

		/* 授權碼(金卡顯示調單編號) */
		memset(szDispMsg, 0x00, sizeof (szDispMsg));
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			sprintf(szDispMsg, " RRN NO.: %s", pobTran->srBRec.szFiscRRN);
		} else
		{
			sprintf(szDispMsg, " APPR: %s", pobTran->srBRec.szAuthCode);
		}
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_7_, _DISP_LEFT_);

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%s", szDispMsg);
			inDISP_LogPrintf(szDebugMsg);
		}

		/* Reference Number */
		memset(szDispMsg, 0x00, sizeof (szDispMsg));
		sprintf(szDispMsg, " REF: %s", pobTran->srBRec.szRefNo);
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_8_, _DISP_LEFT_);

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%s", szDispMsg);
			inDISP_LogPrintf(szDebugMsg);
		}

		/* Invoice Number */
		memset(szDispMsg, 0x00, sizeof (szDispMsg));
		sprintf(szDispMsg, " INV: %06ld", pobTran->srBRec.lnOrgInvNum);
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%06ld", pobTran->srBRec.lnOrgInvNum);
			inDISP_LogPrintf(szDebugMsg);
		}

		/* Amount */
		memset(szDispMsg, 0x00, sizeof (szDispMsg));

		if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
		{
			switch (pobTran->srBRec.inCode)
			{
				case _SALE_:
				case _INST_SALE_:
				case _REDEEM_SALE_:
				case _SALE_OFFLINE_:
				case _FORCE_CASH_ADVANCE_:
				case _PRE_COMP_:
				case _PRE_AUTH_:
				case _CUP_SALE_:
				case _CUP_PRE_COMP_:
				case _CUP_PRE_AUTH_:
				case _INST_ADJUST_:
				case _REDEEM_ADJUST_:
				case _MAIL_ORDER_:
				case _CUP_MAIL_ORDER_:
				case _CASH_ADVANCE_:
				case _FISC_SALE_:
					if (pobTran->srBRec.lnTipTxnAmount != 0)
						sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
					else
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
					inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
					sprintf(szDispMsg, " %s", szTemplate1);
					break;
				case _ADJUST_:
					sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
					inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
					sprintf(szDispMsg, " %s", szTemplate1);
					break;
				case _TIP_:
					sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
					inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
					sprintf(szDispMsg, " %s", szTemplate1);
					break;
				case _REFUND_:
				case _INST_REFUND_:
				case _REDEEM_REFUND_:
				case _CUP_REFUND_:
				case _CUP_MAIL_ORDER_REFUND_:
				case _FISC_REFUND_:
					sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
					inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
					sprintf(szDispMsg, " %s", szTemplate1);
					break;
				default:
					sprintf(szDispMsg, " AMT_ERR_(%d)", pobTran->srBRec.inCode);
					break;
			} /* End switch () */
		} else
		{
			switch (pobTran->srBRec.inOrgCode)
			{
				case _SALE_:
				case _INST_SALE_:
				case _REDEEM_SALE_:
				case _SALE_OFFLINE_:
				case _FORCE_CASH_ADVANCE_:
				case _PRE_COMP_:
				case _PRE_AUTH_:
				case _CUP_SALE_:
				case _CUP_PRE_COMP_:
				case _CUP_PRE_AUTH_:
				case _INST_ADJUST_:
				case _REDEEM_ADJUST_:
				case _MAIL_ORDER_:
				case _CUP_MAIL_ORDER_:
				case _CASH_ADVANCE_:
				case _FISC_SALE_:
					sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
					inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
					sprintf(szDispMsg, " %s", szTemplate1);
					break;
				case _ADJUST_:
					sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
					inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
					sprintf(szDispMsg, " %s", szTemplate1);
					break;
					/* 目前小費沒有取消 */
				case _REFUND_:
				case _INST_REFUND_:
				case _REDEEM_REFUND_:
				case _CUP_REFUND_:
				case _CUP_MAIL_ORDER_REFUND_:
					/* SmartPay不能取消 */
					sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
					inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
					sprintf(szDispMsg, " %s", szTemplate1);
					break;
				default:
					sprintf(szDispMsg, " AMT_VOID_ERR_(%d)", pobTran->srBRec.inCode);
					break;
			} /* End switch () */
		}
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%s", szDispMsg);
			inDISP_LogPrintf(szDebugMsg);
		}

		/* Store ID */
		memset(szTemplate1, 0x00, sizeof (szTemplate1));
		memset(szDispMsg, 0x00, sizeof (szDispMsg));

		inGetStoreIDEnable(&szTemplate1[0]);

		if ((szTemplate1[0] == 'Y') && (strlen(pobTran->srBRec.szStoreID) > 0))
		{
			if (strlen(pobTran->srBRec.szStoreID) > 42)
				memcpy(&szTemplate1[0], &pobTran->srBRec.szStoreID[0], 42);
			else
				strcpy(szTemplate1, pobTran->srBRec.szStoreID);

			szTemplate1[18] = 0x00;

			sprintf(szDispMsg, " STD:%s", pobTran->srBRec.szStoreID);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
		} else
		{
			/* 沒開櫃號功能， 不印櫃號 */
		}

		memset(szDispMsg, 0x00, sizeof (szDispMsg));
		sprintf(szDispMsg, "%d / %d", i + 1, inFileRecCnt);
		inDISP_ChineseFont_Point_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_8_, _COLOR_BLACK_, _COLOR_LIGHTBLUE_, 10);

		/* 事件初始化 */
		szKey = 0;
		/* 設定Timeout */
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);

		while (1)
		{

			/* 偵測滑動 */
			inTouchPage = inDisTouch_TouchSensor_Click_Slide(_Touch_NEWUI_REVIEW_BATCH_);
			if (inTouchPage == _DisTouch_Slide_Right_To_Left_)
			{
				++i;
				break;
			}
			if (inTouchPage == _DisTouch_Slide_Left_To_Right_)
			{
				--i;

				/* 交易紀錄編號不可小於0 */
				if (i < 0)
				{
					i = 0;
				}
				break;
			}

			/* 偵測按鍵 */
			szKey = uszKBD_Key();

			/* TimeOut */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
			{
				inReviewReportResult = VS_FALSE;
				break;
			} else if (szKey == _KEY_ENTER_ || szKey == _KEY_DOWN_)
			{
				++i;
				break;
			} else if (szKey == _KEY_UP_ || szKey == _KEY_FUNCTION_)
			{
				--i;
				/* 交易紀錄編號不可小於0 */
				if (i < 0)
				{
					i = 0;
				}
				break;
			}

		}

	} /* End for () .... */

	/* 顯示批次結束 */
	inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第一層顯示 <交易查詢> */
	inDISP_Msg_BMP(_ERR_BATCH_END_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBatch_ReviewReport()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GetDetailRecords
Date&Time       :2015/9/8 下午 16:00
Describe        :這邊的抓法是根據Bkey內的紀錄，但因為同步InvNum的關係，第一筆不一定是InvNum = 1
 */
int inBATCH_GetDetailRecords(TRANSACTION_OBJECT *pobTran, int inStartCnt)
{
	int i;
	long lnOffset, lnBKEYSize;
	long lnReadLength; /* 記錄剩餘要從Brec讀多長的資料 */
	char szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1]; /* debug message */
	unsigned char uszBATFileName[16 + 1], uszBKEYFileName[16 + 1];
	unsigned long ulBKEYHandle; /* bkey Handle */
	unsigned long ulBATHandle; /* BAT Handle */
	TRANS_BATCH_KEY srBKEYRec;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GetDetailRecords()_START");
	}

	if (inFunc_ComposeFileName(pobTran, (char*) uszBATFileName, _BATCH_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (inFunc_ComposeFileName(pobTran, (char*) uszBKEYFileName, _BATCH_KEY_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	memset((char *) &srBKEYRec, 0x00, sizeof (srBKEYRec));

	/* 【STEP 1】先找 TRANS_BATCH_KEY 裡的 lnInvoiceNo */
	lnBKEYSize = _BATCH_KEY_SIZE_;
	lnOffset = (long) (((long) inStartCnt * lnBKEYSize)); /* 第幾筆 */

	if (inFILE_OpenReadOnly(&ulBKEYHandle, uszBKEYFileName) == VS_ERROR)
	{
		return (VS_ERROR);
	} else
	{
		if (inFILE_Seek(ulBKEYHandle, lnOffset, _SEEK_BEGIN_) == VS_ERROR)
		{
			inFILE_Close(&ulBKEYHandle);

			return (VS_ERROR);
		} else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szErrorMsg, 0x00, sizeof (szErrorMsg));
				sprintf(szErrorMsg, "%lu,%lu", lnBKEYSize, lnFILE_GetSize(&ulBKEYHandle, uszBKEYFileName));
				inDISP_LogPrintf(szErrorMsg);
			}

			if (inFILE_Read(&ulBKEYHandle, (unsigned char *) &srBKEYRec, sizeof (srBKEYRec)) == VS_ERROR)
			{
				inFILE_Close(&ulBKEYHandle);

				return (VS_ERROR);
			}
		}
	}

	if (inFILE_Close(&ulBKEYHandle) == VS_ERROR)
	{
		return (VS_ERROR);
	}

	/* 【STEP 2】開始搜尋相對應的 BATCH_REC 交易記錄 */
	lnOffset = srBKEYRec.lnBatchRecStart;

	if (inFILE_OpenReadOnly(&ulBATHandle, uszBATFileName) == VS_ERROR)
	{
		return (VS_ERROR);
	}

	if (inFILE_Seek(ulBATHandle, lnOffset, _SEEK_BEGIN_) == VS_ERROR)
	{
		inFILE_Close(&ulBATHandle);

		return (VS_ERROR);
	}

	lnReadLength = srBKEYRec.lnBatchRecSize;

	/* 要從0開始算，所以要用i++，而不是++i */
	for (i = 0;; i++)
	{
		/* 剩餘長度大於或等於inMaxsize */
		if (lnReadLength >= 1024)
		{
			/* read bat，讀的數量為srBKRec.lnBatchRecSize，因為bat每一筆資料長度是浮動的 */
			if (inFILE_Read(&ulBATHandle, (unsigned char *) &pobTran->srBRec + (1024 * i), 1024) == VS_ERROR)
			{
				inFILE_Close(&ulBATHandle);

				return (VS_ERROR);
			} else
			{
				/* 一次讀1024 */
				lnReadLength -= 1024;

				/* 當剩餘長度剛好為1024，會剛好讀完 */
				if (lnReadLength == 0)
					break;
			}

		}			/* 剩餘長度小於1024 */
		else if (lnReadLength < 1024)
		{
			/* 就只讀剩餘長度 */
			if (inFILE_Read(&ulBATHandle, (unsigned char *) &pobTran->srBRec + (1024 * i), lnReadLength) == VS_ERROR)
			{
				/* Close檔案 */
				inFILE_Close(&ulBATHandle);

				return (VS_ERROR);
			} else
			{
				lnReadLength -= lnReadLength;

				if (lnReadLength == 0)
					break;
			}
		}
	}/* end for loop */

	/* 檢查讀出來的srBRec是否為正確的 */
	if (pobTran->srBRec.lnOrgInvNum != srBKEYRec.lnInvoiceNo)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szErrorMsg, 0x00, sizeof (szErrorMsg));
			sprintf(szErrorMsg, "OrgInv:%lu, BkeyInvoiceNo:%lu", pobTran->srBRec.lnOrgInvNum, srBKEYRec.lnInvoiceNo);
			inDISP_LogPrintf(szErrorMsg);
		}

		inFILE_Close(&ulBATHandle);

		return (VS_ERROR);
	}

	if (inFILE_Close(&ulBATHandle) == VS_ERROR)
	{
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GetDetailRecords()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_FuncUserChoice
Date&Time       :2015/9/8 下午 16:00
Describe        :
 */
int inBATCH_FuncUserChoice(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR;
#ifdef __NCCC_DCC_FUNC__	
	int inDCCIndex = -1;
	int inNCCCIndex = -1;
	int inOrgIndex = -1;
#endif

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

#ifdef __NCCC_DCC_FUNC__
	/* NCCC DCC的特殊處理，當端末機收到收銀機要取消DCC HOST交易的電文時，
	 * 如果在DCC的帳務中查不到這一筆交易(表示交易改為台幣支付)端末機就自動在去找NCCC的帳務來解決銀行別錯誤而取消不到的問題。 START
	 */
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_DCC_, &inDCCIndex);
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_MAINLY_USE_, &inNCCCIndex);
	/* NCCC DCC的特殊處理，當端末機收到收銀機要取消DCC HOST交易的電文時，
	 * 如果在DCC的帳務中查不到這一筆交易(表示交易改為台幣支付)端末機就自動在去找NCCC的帳務來解決銀行別錯誤而取消不到的問題。 END
	 */
#endif 

	while (1)
	{
		/* 如果lnOrgInvNum有值，就不需要做輸入的動作(ECR發動) */
		if (pobTran->uszECRBit == VS_TRUE)
		{
			if (pobTran->srBRec.lnOrgInvNum <= 0)
			{
				return (VS_ERROR);
			}
		} else
		{
			if (pobTran->srBRec.lnOrgInvNum <= 0)
			{
				if (inBATCH_GetInvoiceNumber(pobTran) == VS_ERROR)
				{
					return (VS_ERROR);
				}

				/* 因為EDC如果按確認鍵，等於調出上一筆記錄 */
				if (pobTran->srBRec.lnOrgInvNum == 0)
					pobTran->srBRec.lnOrgInvNum = _BATCH_LAST_RECORD_; /* 表示是最後一筆 */
			}
		}

		/* Load Batch Record */
		inRetVal = inBATCH_GetTransRecord(pobTran);

		if (inRetVal == VS_SUCCESS)
		{
			/* NCCC DCC的特殊處理，當端末機收到收銀機要取消DCC HOST交易的電文時，
			 * 如果在DCC的帳務中查不到這一筆交易(表示交易改為台幣支付)端末機就自動在去找NCCC的帳務來解決銀行別錯誤而取消不到的問題。 START
			 */
#ifdef __NCCC_DCC_FUNC__

			if (pobTran->srBRec.inHDTIndex == inDCCIndex &&
					pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)
			{
				/* 這個情形只有在用POS機時才會出現 */
				if (pobTran->uszECRBit == VS_TRUE)
				{
					inOrgIndex = pobTran->srBRec.inHDTIndex;
					pobTran->srBRec.inHDTIndex = inNCCCIndex;
					/* Load Batch Record */
					inRetVal = inBATCH_GetTransRecord(pobTran);

					if (inRetVal == VS_SUCCESS)
					{
						break;
					} else if (inRetVal == VS_NO_RECORD)
					{
						/* 找不到，還原index並顯示無交易紀錄*/
						pobTran->srBRec.inHDTIndex = inOrgIndex;

						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						/* 顯示無交易紀錄 */
						inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

						inDISP_BEEP(3, 500);
						pobTran->srBRec.lnOrgInvNum = -1;

						continue;
					} else
					{
						return (VS_ERROR);
					}
					/* NCCC DCC的特殊處理，當端末機收到收銀機要取消DCC HOST交易的電文時，
					 * 如果在DCC的帳務中查不到這一筆交易(表示交易改為台幣支付)端末機就自動在去找NCCC的帳務來解決銀行別錯誤而取消不到的問題。 END
					 */
				} else
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					/* 顯示無交易紀錄 */
					inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

					inDISP_BEEP(3, 500);
					pobTran->srBRec.lnOrgInvNum = -1;

					continue;
				}

			}				/* 找到紀錄 */
			else
			{
				break;
			}
#else
			break;
#endif

		}			/* 找不到 */
		else if (inRetVal == VS_NO_RECORD)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			/* 顯示無交易紀錄 */
			inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

			inDISP_BEEP(3, 500);
			pobTran->srBRec.lnOrgInvNum = -1;

			continue;
		} else
		{
			return (VS_ERROR);
		}

	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GetInvoiceNumber
Date&Time       :2015/9/8 下午 16:00
Describe        :輸入調閱編號
 */
int inBATCH_GetInvoiceNumber(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR, i = 0;
	char szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT srDispObj;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GetInvoiceNumber()_START");
	}

	/* 初始化 */
	memset(&srDispObj, 0x00, sizeof (DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof (szTemplate));

	while (1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_GET_INVOICE_NUM_, 0, _COORDINATE_Y_LINE_8_4_);

		srDispObj.inY = _LINE_8_6_;
		srDispObj.inR_L = _DISP_LEFT_;
		srDispObj.inMaxLen = 3;
		srDispObj.inMask = VS_FALSE;
		srDispObj.inColor = _COLOR_RED_;
		srDispObj.inTouchSensorFunc = _Touch_CUP_LOGON_;
		strcpy(srDispObj.szPromptMsg, "INV#:");
		//strcpy(srDispObj.szPromptMsg, "REF#:");

		strcpy(szTemplate, "INV#:");
		//strcpy(szTemplate, "REF#:");
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_LEFT_);

		memset(srDispObj.szOutput, 0x00, sizeof (srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_Character_Mask_And_DisTouch(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		{
			return (VS_ERROR);
		} else if (inRetVal == 0)
		{
			pobTran->srBRec.lnOrgInvNum = _BATCH_LAST_RECORD_;
			break;
		} else if (inRetVal > 0)
		{
			for (i = 0; i < inRetVal; i++)
			{
				if ((srDispObj.szOutput[i] >= '0') && (srDispObj.szOutput[i] <= '9'))
					continue;
				else
				{
					memset(srDispObj.szOutput, 0x00, sizeof (srDispObj.szOutput));
					break;
				}
			}

			if (strlen(srDispObj.szOutput) == 0)
				continue;

			pobTran->srBRec.lnOrgInvNum = atol(srDispObj.szOutput);

			if (pobTran->srBRec.lnOrgInvNum == 0)
			{
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf("inBATCH_GetInvoiceNumber No OrgInvNum");
				}

				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				/* 顯示無交易紀錄 */
				inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);
				inDISP_Wait(3000);

				continue;
			}
		}

		break;
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GetInvoiceNumber()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GetTransRecord
Date&Time       :2015/9/8 下午 16:00
Describe        :用inBATCH_SearchRecord找到想要的bkey記錄，再找bkey對應的bat記錄
 */
int inBATCH_GetTransRecord(TRANSACTION_OBJECT *pobTran)
{
	long lnBAKFileSize;
	long lnReadLength;
	int inRetVal, inBAKTotalCnt, i;
	char szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1]; /* debug message */
	unsigned char uszBATFileName[15 + 1], uszBKEYFileName[14 + 1]; /* 檔名(含附檔名)最長可到15個字 */
	unsigned long ulBKEYHandle;
	unsigned long ulBATHandle;
	TRANS_BATCH_KEY srBKRec;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GetTransRecord()_START");
	}

	memset(uszBKEYFileName, 0x00, sizeof (uszBKEYFileName));
	memset(uszBATFileName, 0x00, sizeof (uszBATFileName));

	if (inFunc_ComposeFileName(pobTran, (char*) uszBKEYFileName, _BATCH_KEY_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (inFunc_ComposeFileName(pobTran, (char*) uszBATFileName, _BATCH_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* ingenico會刪除空白的檔案 */
	/* open bkey file */
	if (inFILE_OpenReadOnly(&ulBKEYHandle, uszBKEYFileName) == VS_ERROR)
	{
		/* 表示bkey沒有資料 */
		return (VS_NO_RECORD);
	}

	/* 檢查bkey總長度 */
	lnBAKFileSize = lnFILE_GetSize(&ulBKEYHandle, uszBKEYFileName);

	if (lnBAKFileSize > 0)
	{
		inBAKTotalCnt = (int) (lnBAKFileSize / _BATCH_KEY_SIZE_);
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szErrorMsg, 0x00, sizeof (szErrorMsg));
			sprintf(szErrorMsg, "inBATCH_GetTransRecord lnBAKFileSize = %ld", lnBAKFileSize);
			inDISP_LogPrintf(szErrorMsg);
		}

		inFILE_Close(&ulBKEYHandle);

		return (VS_ERROR);
	}

	/* 找到符合pobTran->srBRec.lnOrgInvNum的bkey記錄  */
	memset(&srBKRec, 0x00, sizeof (srBKRec));
	if (inBATCH_SearchRecord(pobTran, &srBKRec, ulBKEYHandle, inBAKTotalCnt) == VS_ERROR)
	{
		inFILE_Close(&ulBKEYHandle);
		return (VS_NO_RECORD);
	}

	/* ingenico會刪除空白的檔案 */
	/* open bat file */
	if (inFILE_OpenReadOnly(&ulBATHandle, uszBATFileName) == VS_ERROR)
	{
		/* 表示BAT沒有資料 */
		inFILE_Close(&ulBKEYHandle);
		return (VS_ERROR);
	}

	/* 開始搜尋相對應的 BATCH_REC 交易記錄 */
	if (inFILE_Seek(ulBATHandle, srBKRec.lnBatchRecStart, _SEEK_BEGIN_) == VS_ERROR)
	{
		inFILE_Close(&ulBKEYHandle);
		inFILE_Close(&ulBATHandle);
		return (VS_ERROR);
	}

	lnReadLength = srBKRec.lnBatchRecSize;

	for (i = 0;; ++i)
	{
		/* 剩餘長度大於或等於inMaxsize */
		if (lnReadLength >= 1024)
		{
			/* read bat，讀的數量為srBKRec.lnBatchRecSize，因為bat每一筆資料長度是浮動的 */
			if ((inRetVal = inFILE_Read(&ulBATHandle, (unsigned char *) &pobTran->srBRec + (1024 * i), 1024)) == VS_ERROR)
			{
				inFILE_Close(&ulBATHandle);

				return (VS_ERROR);
			} else
			{
				/* 一次讀1024 */
				lnReadLength -= 1024;

				/* 當剩餘長度剛好為1024，會剛好讀完 */
				if (lnReadLength == 0)
					break;
			}

		}			/* 剩餘長度小於1024 */
		else if (lnReadLength < 1024)
		{
			/* 就只讀剩餘長度 */
			if ((inRetVal = inFILE_Read(&ulBATHandle, (unsigned char *) &pobTran->srBRec + (1024 * i), lnReadLength)) == VS_ERROR)
			{
				/* Close檔案 */
				inFILE_Close(&ulBATHandle);

				return (VS_ERROR);
			} else
			{
				lnReadLength -= lnReadLength;

				if (lnReadLength == 0)
					break;
			}
		}
	}/* end for loop */

	if (inRetVal == VS_ERROR || pobTran->srBRec.lnOrgInvNum != srBKRec.lnInvoiceNo)
	{
		inFILE_Close(&ulBKEYHandle);
		inFILE_Close(&ulBATHandle);
		return (VS_ERROR);
	}

	inFILE_Close(&ulBKEYHandle);
	inFILE_Close(&ulBATHandle);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GetTransRecord()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GlobalBatchHandleReadOnly
Date&Time       :
Describe        :
 */
int inBATCH_GlobalBatchHandleReadOnly(void)
{
	unsigned char uszBATFileName[16 + 1], uszBKEYFileName[16 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GlobalBatchHandleReadOnly()_START");
	}

	if (inFunc_ComposeFileName_ForGlobal((char*) uszBKEYFileName, _BATCH_KEY_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (inFunc_ComposeFileName_ForGlobal((char*) uszBATFileName, _BATCH_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 為了避免重複關Handle造成當機，先給default value */
	gulBATHandle = VS_HANDLE_NULL;
	gulBKEYHandle = VS_HANDLE_NULL;

	if (inFILE_OpenReadOnly(&gulBATHandle, uszBATFileName) == VS_ERROR)
		return (VS_ERROR);

	if (inFILE_OpenReadOnly(&gulBKEYHandle, uszBKEYFileName) == VS_ERROR)
	{
		inFILE_Close(&gulBATHandle);
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GlobalBatchHandleReadOnly()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GlobalBatchHandleClose
Date&Time       :
Describe        :
 */
int inBATCH_GlobalBatchHandleClose(void)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GlobalBatchHandleClose()_START");
	}

	if (gulBATHandle != VS_HANDLE_NULL)
	{
		inFILE_Close(&gulBATHandle);
	}

	if (gulBKEYHandle != VS_HANDLE_NULL)
	{
		inFILE_Close(&gulBKEYHandle);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GlobalBatchHandleClose()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GlobalAdviceHandleReadOnly
Date&Time       :
Describe        :
 */
int inBATCH_GlobalAdviceHandleReadOnly(void)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GlobalAdviceHandleReadOnly()_START");
	}

	if (inFunc_ComposeFileName_ForGlobal((char*) guszBATFileName, _BATCH_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (inFunc_ComposeFileName_ForGlobal((char*) guszBKEYFileName, _BATCH_KEY_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (inFunc_ComposeFileName_ForGlobal((char*) guszADVFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}



	/* 為了避免重複開Handle造成當機，先將handle值設為default(0x00)(因為用globalHandle無法確保是否已關，重複開會造成錯誤) */
	gulBATHandle = VS_HANDLE_NULL;
	gulBKEYHandle = VS_HANDLE_NULL;
	gulADVHandle = VS_HANDLE_NULL;

	if (inFILE_OpenReadOnly(&gulBATHandle, guszBATFileName) == VS_ERROR)
		return (VS_ERROR);

	if (inFILE_OpenReadOnly(&gulBKEYHandle, guszBKEYFileName) == VS_ERROR)
	{
		inFILE_Close(&gulBATHandle);
		return (VS_ERROR);
	}

	if (inFILE_OpenReadOnly(&gulADVHandle, guszADVFileName) == VS_ERROR)
	{
		inFILE_Close(&gulBATHandle);
		inFILE_Close(&gulBKEYHandle);
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GlobalAdviceHandleReadOnly()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GlobalAdviceHandleClose
Date&Time       :
Describe        :
 */
int inBATCH_GlobalAdviceHandleClose(void)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GlobalAdviceHandleClose()_START");
	}

	if (gulBATHandle != VS_HANDLE_NULL)
	{
		inFILE_Close(&gulBATHandle);
	}

	if (gulBKEYHandle != VS_HANDLE_NULL)
	{
		inFILE_Close(&gulBKEYHandle);
	}

	if (gulADVHandle != VS_HANDLE_NULL)
	{
		inFILE_Close(&gulADVHandle);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GlobalAdviceHandleClose()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GetAdviceDetailRecord
Date&Time       :
Describe        :
 */
int inBATCH_GetAdviceDetailRecord(TRANSACTION_OBJECT *pobTran, int inADVCnt)
{
	int i;
	int inRetVal;
	long lnBKEYSize, lnBKEYTotalCnt, lnOffset;
	long lnSearchStart, lnSearchEnd, lnSearchIndex = 0;
	long lnReadLength = 0;
	unsigned char uszGetAdviceInvoiceNum[6 + 1];
	TRANS_BATCH_KEY srBKEYRec;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GetAdviceDetailRecord()_START");
	}

	/* 讀【.adv】檔 */
	if (inFILE_Seek(gulADVHandle, 0, _SEEK_BEGIN_) == VS_ERROR)
	{
		return (VS_ERROR);
	} else
	{
		memset(uszGetAdviceInvoiceNum, 0x00, sizeof (uszGetAdviceInvoiceNum));
		if ((inRetVal = inFILE_Read(&gulADVHandle, &uszGetAdviceInvoiceNum[0], _ADV_FIELD_SIZE_)) == VS_ERROR)
		{
			return (VS_ERROR);
		} else
		{
			pobTran->srBRec.lnOrgInvNum = atol((char *) uszGetAdviceInvoiceNum);
		}
	}

	/* 讀【.bkey】檔，用Invoice Number比對後，取得Batch在記憶體的位置 */
	lnBKEYTotalCnt = lnFILE_GetSize(&gulBKEYHandle, guszBKEYFileName);
	lnBKEYTotalCnt = (lnBKEYTotalCnt / _BATCH_KEY_SIZE_);

	lnSearchStart = 1;
	lnSearchEnd = lnBKEYTotalCnt;

	if (lnBKEYTotalCnt == 1)
		lnSearchIndex = 0;
	else
		lnSearchIndex = (lnBKEYTotalCnt / 2); /* 二分法 */

	while (1)
	{
		lnBKEYSize = _BATCH_KEY_SIZE_;
		lnOffset = lnSearchIndex * lnBKEYSize;

		if (inFILE_Seek(gulBKEYHandle, lnOffset, _SEEK_BEGIN_) == VS_ERROR)
		{
			return (VS_ERROR);
		} else
		{
			lnReadLength = lnBKEYSize;
			/* 要從0開始算，所以要用i++，而不是++i */
			for (i = 0;; i++)
			{
				/* 剩餘長度大於或等於inMaxsize */
				if (lnReadLength >= 1024)
				{
					/* read bat，讀的數量為srBKRec.lnBatchRecSize，因為bat每一筆資料長度是浮動的 */
					if (inFILE_Read(&gulBKEYHandle, (unsigned char *) &srBKEYRec + (1024 * i), 1024) == VS_ERROR)
					{
						inFILE_Close(&gulBKEYHandle);

						return (VS_ERROR);
					} else
					{
						/* 一次讀1024 */
						lnReadLength -= 1024;

						/* 當剩餘長度剛好為1024，會剛好讀完 */
						if (lnReadLength == 0)
							break;
					}

				}					/* 剩餘長度小於1024 */
				else if (lnReadLength < 1024)
				{
					/* 就只讀剩餘長度 */
					if (inFILE_Read(&gulBKEYHandle, (unsigned char *) &srBKEYRec + (1024 * i), lnReadLength) == VS_ERROR)
					{
						/* Close檔案 */
						inFILE_Close(&gulBKEYHandle);

						return (VS_ERROR);
					} else
					{
						lnReadLength -= lnBKEYSize;

						if (lnReadLength == 0)
							break;
					}
				}
			}/* end for loop */

		}

		/* 判斷讀到的資料是否正確 */
		if (pobTran->srBRec.lnOrgInvNum == srBKEYRec.lnInvoiceNo)
		{
			break;
		} else
		{
			if (srBKEYRec.lnInvoiceNo > pobTran->srBRec.lnOrgInvNum)
				lnSearchEnd = lnSearchIndex; /* 往左邊找 */
			else
				lnSearchStart = lnSearchIndex; /* 往右邊找 */
		}

		lnSearchIndex = (lnSearchStart + lnSearchEnd) / 2;

		if (lnSearchIndex == 1 && lnSearchEnd == 1)
			lnSearchIndex = 0;

		/* 沒有找到TRANS_BATCH_KEY */
		if (lnBKEYTotalCnt < 0)
		{
			return (VS_ERROR);
		}

		lnBKEYTotalCnt--;
		continue;
	}

	/* 讀【.bat】檔BATCH_REC 交易記錄 */
	lnOffset = srBKEYRec.lnBatchRecStart;

	if (inFILE_Seek(gulBATHandle, srBKEYRec.lnBatchRecStart, _SEEK_BEGIN_) == VS_ERROR)
	{
		return (VS_ERROR);
	} else
	{
		lnReadLength = srBKEYRec.lnBatchRecSize;
		/* 讀的數量為srBKRec.lnBatchRecSize，因為【.bat】每一筆資料長度是浮動的 */

		/* 要從0開始算，所以要用i++，而不是++i */
		for (i = 0;; i++)
		{
			/* 剩餘長度大於或等於inMaxsize */
			if (lnReadLength >= 1024)
			{
				/* read bat，讀的數量為srBKRec.lnBatchRecSize，因為bat每一筆資料長度是浮動的 */
				if (inFILE_Read(&gulBATHandle, (unsigned char *) &pobTran->srBRec + (1024 * i), 1024) == VS_ERROR)
				{
					inFILE_Close(&gulBATHandle);

					return (VS_ERROR);
				} else
				{
					/* 一次讀1024 */
					lnReadLength -= 1024;

					/* 當剩餘長度剛好為1024，會剛好讀完 */
					if (lnReadLength == 0)
						break;
				}

			}				/* 剩餘長度小於1024 */
			else if (lnReadLength < 1024)
			{
				/* 就只讀剩餘長度 */
				if (inFILE_Read(&gulBATHandle, (unsigned char *) &pobTran->srBRec + (1024 * i), lnReadLength) == VS_ERROR)
				{
					/* Close檔案 */
					inFILE_Close(&gulBATHandle);

					return (VS_ERROR);
				} else
				{
					lnReadLength -= lnReadLength;

					if (lnReadLength == 0)
						break;
				}
			}
		}/* end for loop */

	}

	/* 檢查讀出來的srBRec是否為正確的 */
	if (pobTran->srBRec.lnOrgInvNum != srBKEYRec.lnInvoiceNo)
	{
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GetAdviceDetailRecord()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_ESC_Save_Advice
Date&Time       :2016/12/19 下午 3:57
Describe        :存ESC advice
 */
int inBATCH_ESC_Save_Advice(TRANSACTION_OBJECT *pobTran)
{
	inADVICE_ESC_SaveAppend(pobTran, pobTran->srBRec.lnOrgInvNum);

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_Update_Sign_Status
Date&Time       :2017/3/22 下午 3:46
Describe        :更新簽名狀態
 */
int inBATCH_Update_Sign_Status(TRANSACTION_OBJECT *pobTran)
{
	pobTran->uszUpdateBatchBit = VS_TRUE;
	inBATCH_FuncUpdateTxnRecord(pobTran);

	return (VS_SUCCESS);
}

/*
Function        :inADVICE_SaveAppend
Date&Time       :
Describe        :
 */
int inADVICE_SaveAppend(TRANSACTION_OBJECT *pobTran, long lnInvNum)
{
	int inRetVal;
	char szAdviceInvNum[10 + 1];
	unsigned char uszFileName[16 + 1];
	unsigned long ulFHandle;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inADVICE_SaveAppend()_START,%d", lnInvNum);
	}

	if (inFunc_ComposeFileName(pobTran, (char*) uszFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inADVICE_SaveAppend() uszFileName=%s", uszFileName);

	inRetVal = inFILE_Open(&ulFHandle, uszFileName);
	if (inRetVal == VS_ERROR)
	{
		/* 檔案不存在 */
		inRetVal = inFILE_Create(&ulFHandle, uszFileName);
		if (inRetVal == VS_ERROR)
		{
			return (VS_ERROR);
		}
	}

	inRetVal = inFILE_Seek(ulFHandle, 0, _SEEK_END_); /* 指到最後第一個 BYTE */
	if (inRetVal == VS_ERROR)
	{
		inFILE_Close(&ulFHandle);
		return (VS_ERROR);
	}

	memset(szAdviceInvNum, 0x00, sizeof (szAdviceInvNum));
	sprintf(szAdviceInvNum, "%06ld", lnInvNum);
	inRetVal = inFILE_Write(&ulFHandle, (unsigned char *) &szAdviceInvNum[0], _ADV_FIELD_SIZE_);
	if (inRetVal == VS_ERROR)
	{
		inFILE_Close(&ulFHandle);
		return (VS_ERROR);
	}

	inFILE_Close(&ulFHandle);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inADVICE_SaveAppend()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inADVICE_SaveTop
Date&Time       :2016/9/20 上午 9:53
Describe        :TC Upload 使用
 */
int inADVICE_SaveTop(TRANSACTION_OBJECT *pobTran, long lnInvNum)
{
	int inRetVal;
	long lnADVCnt;
	char szAdviceInvNum[10 + 1];
	unsigned long ulADVFHandle;
	unsigned char *uszADVMemory, uszADVFileName[16 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inADVICE_SaveTop()_START");
	}

	if (inFunc_ComposeFileName(pobTran, (char*) uszADVFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	inRetVal = inFILE_Open(&ulADVFHandle, uszADVFileName);

	if (inRetVal == VS_ERROR)
	{
		/* 檔案不存在 */
		inRetVal = inFILE_Create(&ulADVFHandle, uszADVFileName);
		if (inRetVal == VS_ERROR)
		{
			return (VS_ERROR);
		}
	}

	/* 取得 Advice 檔案大小 */
	lnADVCnt = lnFILE_GetSize(&ulADVFHandle, uszADVFileName);
	if (lnADVCnt == 0)
	{
		/* 表示是第一筆資料 */
		inRetVal = inFILE_Seek(ulADVFHandle, 0, _SEEK_END_); /* 指到最後第一個 BYTE */
		if (inRetVal == VS_ERROR)
		{
			inFILE_Close(&ulADVFHandle);
			return (VS_ERROR);
		}

		memset(szAdviceInvNum, 0x00, sizeof (szAdviceInvNum));
		sprintf(szAdviceInvNum, "%06ld", lnInvNum);
		inRetVal = inFILE_Write(&ulADVFHandle, (unsigned char *) &szAdviceInvNum[0], _ADV_FIELD_SIZE_);
		if (inRetVal == VS_ERROR)
		{
			inFILE_Close(&ulADVFHandle);
			return (VS_ERROR);
		}

		inFILE_Close(&ulADVFHandle);
	} else
	{
		/* 記憶體 */
		uszADVMemory = malloc(lnADVCnt + 12);
		memset(uszADVMemory, 0x00, (lnADVCnt + 12));
		/* 讀 Advice 檔案資料 */
		inRetVal = inFILE_Seek(ulADVFHandle, 0, _SEEK_BEGIN_); /* 指到第一個 BYTE */
		if (inRetVal == VS_ERROR)
		{
			inFILE_Close(&ulADVFHandle);
			free(uszADVMemory);
			return (VS_ERROR);
		}

		inRetVal = inFILE_Read(&ulADVFHandle, &uszADVMemory[0], lnADVCnt);

		if (inRetVal == VS_ERROR)
		{
			inFILE_Close(&ulADVFHandle);
			free(uszADVMemory);
			return (VS_ERROR);
		}

		/* 關檔案 */
		inFILE_Close(&ulADVFHandle);
		/* 刪除檔案 */
		inFILE_Delete(uszADVFileName);

		/* 重新建立 Advice 檔 */
		inRetVal = inFILE_Create(&ulADVFHandle, uszADVFileName);
		if (inRetVal == VS_ERROR)
		{
			free(uszADVMemory);
			return (VS_ERROR);
		}

		inRetVal = inFILE_Seek(ulADVFHandle, 0, _SEEK_END_); /* 指到最後第一個 BYTE */
		if (inRetVal == VS_ERROR)
		{
			inFILE_Close(&ulADVFHandle);
			free(uszADVMemory);
			return (VS_ERROR);
		}

		memset(szAdviceInvNum, 0x00, sizeof (szAdviceInvNum));
		sprintf(szAdviceInvNum, "%06ld", lnInvNum);
		inRetVal = inFILE_Write(&ulADVFHandle, (unsigned char *) &szAdviceInvNum[0], _ADV_FIELD_SIZE_);
		if (inRetVal == VS_ERROR)
		{
			inFILE_Close(&ulADVFHandle);
			free(uszADVMemory);
			return (VS_ERROR);
		}

		inRetVal = inFILE_Seek(ulADVFHandle, 0, _SEEK_END_); /* 指到最後第一個 BYTE */
		if (inRetVal == VS_ERROR)
		{
			inFILE_Close(&ulADVFHandle);
			free(uszADVMemory);
			return (VS_ERROR);
		}

		inRetVal = inFILE_Write(&ulADVFHandle, &uszADVMemory[0], lnADVCnt);
		if (inRetVal == VS_ERROR)
		{
			inFILE_Close(&ulADVFHandle);
			free(uszADVMemory);
			return (VS_ERROR);
		}

		inFILE_Close(&ulADVFHandle);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inADVICE_SaveTop()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inADVICE_DeleteRecordFlow
Date&Time       :2016/9/20 上午 9:57
Describe        :檢查 lnInvNum 是否在 Advice 檔案中，若是則將 lnInvNum 從 Advice 檔案中刪除
 */
int inADVICE_DeleteRecordFlow(TRANSACTION_OBJECT *pobTran, long lnInvNum)
{
	int inDeleteAdvIndex = 0;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inADVICE_DeleteRecordFlow() START !");
	}

	if (inADVICE_SearchRecord_Index(pobTran, lnInvNum, &inDeleteAdvIndex) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 刪除中間的值 */
	if (inDeleteAdvIndex >= 0)
	{
		if (inADVICE_DeleteRecored(pobTran, inDeleteAdvIndex) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock();

			return (VS_ERROR);
		}
	}


	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inADVICE_DeleteRecordFlow() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inADVICE_SearchRecord_Index
Date&Time       :2016/9/20 下午 2:08
Describe        :
 */
int inADVICE_SearchRecord_Index(TRANSACTION_OBJECT *pobTran, long lnInvNum, int *inADVIndex)
{
	int inRecCnt = 0;
	int inRetVal;
	char szDebugMsg[100 + 1];
	char szReadBuf[_ADV_FIELD_SIZE_ + 1];
	unsigned char uszADVFileName[16 + 1];
	unsigned long ulHandle;

	if (inFunc_ComposeFileName(pobTran, (char*) uszADVFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 找到該advice index */
	if (inFILE_OpenReadOnly(&ulHandle, uszADVFileName) != VS_SUCCESS)
		return (VS_ERROR);

	inRecCnt = (int) (lnFILE_GetSize(&ulHandle, uszADVFileName) / _ADV_FIELD_SIZE_);
	(*inADVIndex) = 0;

	/* 從頭開始搜索 */
	if (inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		if (inFILE_Close(&ulHandle) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock();
		}

		return (VS_ERROR);
	}

	do
	{

		memset(szReadBuf, 0x00, _ADV_FIELD_SIZE_ + 1);
		inRetVal = inFILE_Read(&ulHandle, (unsigned char*) szReadBuf, _ADV_FIELD_SIZE_);

		if (inRetVal != VS_SUCCESS)
		{
			inFILE_Close(&ulHandle);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "inADVICE_SearchRecord_Index ERROR!!");
				inDISP_LogPrintf(szDebugMsg);
			}

			return (VS_ERROR);
		} else
		{
			if (lnInvNum == atol(szReadBuf))
			{
				break;
			}

			/* index + 1 */
			(*inADVIndex)++;
		}

	} while (*inADVIndex <= inRecCnt);

	if (inFILE_Close(&ulHandle) != VS_SUCCESS)
		return (VS_ERROR);

	return (VS_SUCCESS);
}

/*
Function        :inADVICE_DeleteRecored
Date&Time       :2016/9/20 下午 1:56
Describe        :刪除ADV檔中的特定invoice number的紀錄
 */
int inADVICE_DeleteRecored(TRANSACTION_OBJECT *pobTran, int inAdvIndex)
{
	int i;
	int inAdvSize; /* ADV檔總長度 */
	int inRemainLength; /* 後半段需寫入的長度 */
	long lnReadLength; /* 剩餘讀取長度 */
	char szDebugMsg[100 + 1];
	unsigned char *uszAdvBuf;
	unsigned char uszADVFileName[16 + 1], uszADBFileName[16 + 1];
	unsigned long ulADVHandle, ulADBHandle;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inADVICE_DeleteRecored() START !");
	}

	if (inFunc_ComposeFileName(pobTran, (char*) uszADVFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (inFunc_ComposeFileName(pobTran, (char*) uszADBFileName, _ADVICE_BACKUP_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 原始檔案 */
	if (inFILE_OpenReadOnly(&ulADVHandle, uszADVFileName) != VS_SUCCESS)
	{
		inFILE_Close(&ulADVHandle);

		/* 鎖機 */
		inFunc_EDCLock();

		return (VS_ERROR);
	}

	/* 抓檔案長度 */
	inAdvSize = (int) lnFILE_GetSize(&ulADVHandle, uszADVFileName);

	/* 表示只有一筆記錄，直接刪檔即可 */
	if (inAdvSize == _ADV_FIELD_SIZE_)
	{

		if (inFILE_Close(&ulADVHandle) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock();
		}

		if (inFILE_Delete(uszADVFileName) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock();
		}

		return (VS_SUCCESS);
	}

	/* 若有多筆紀錄，從頭開始搜索 */
	if (inFILE_Seek(ulADVHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		if (inFILE_Close(&ulADVHandle) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock();
		}

		return (VS_ERROR);
	}

	uszAdvBuf = malloc(inAdvSize + 1);
	/* 剩餘讀取長度 */
	lnReadLength = inAdvSize;

	/* 先讀出所有adv內容 */
	for (i = 0;; ++i)
	{
		memset(uszAdvBuf, 0x00, inAdvSize + 1);
		inFILE_Read(&ulADVHandle, uszAdvBuf, 1024);
		/* 剩餘長度大於或等於1024 */
		if (lnReadLength >= 1024)
		{
			if (inFILE_Read(&ulADVHandle, &uszAdvBuf[1024 * i], 1024) == VS_SUCCESS)
			{
				/* 一次讀1024 */
				lnReadLength -= 1024;

				/* 當剩餘長度剛好為1024，會剛好讀完 */
				if (lnReadLength == 0)
					break;
			}				/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulADVHandle);

				/* Free pointer */
				free(uszAdvBuf);

				/* 鎖機 */
				inFunc_EDCLock();

				return (VS_ERROR);
			}
		}			/* 剩餘長度小於1024 */
		else if (lnReadLength < 1024)
		{
			/* 就只讀剩餘長度 */
			if (inFILE_Read(&ulADVHandle, &uszAdvBuf[1024 * i], lnReadLength) == VS_SUCCESS)
			{
				break;
			}				/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulADVHandle);

				/* Free pointer */
				free(uszAdvBuf);

				/* 鎖機 */
				inFunc_EDCLock();

				return (VS_ERROR);
			}
		}

	};

	/* 關檔 */
	if (inFILE_Close(&ulADVHandle) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock();
	}

	/* 防呆先刪除ADB */
	inFILE_Delete((unsigned char *) uszADBFileName);
	/* 建新的備份檔案 */
	if (inFILE_Create(&ulADBHandle, uszADBFileName) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock();

		return (VS_ERROR);
	}

	/* 若有多筆紀錄，從頭開始搜索 */
	if (inFILE_Seek(ulADBHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		if (inFILE_Close(&ulADBHandle) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock();
		}

		return (VS_ERROR);
	}

	/* 寫入前半段，inAdvIndex為0則代表前半段沒資料需要寫 */
	if (inAdvIndex != 0)
	{
		if (inFILE_Write(&ulADBHandle, uszAdvBuf, _ADV_FIELD_SIZE_ * inAdvIndex) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock();

			return (VS_ERROR);
		}

	}

	/* 跳過那一筆不寫，並移handle到最後面 */
	if (inFILE_Seek(ulADBHandle, 0, _SEEK_END_) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock();

		return (VS_ERROR);
	}

	/* 寫入後半段，若剩餘資料長度大於0，代表後面需要寫 */
	inRemainLength = inAdvSize - (_ADV_FIELD_SIZE_ * (inAdvIndex + 1));
	if (inRemainLength > 0)
	{
		if (inFILE_Write(&ulADBHandle, &uszAdvBuf[(_ADV_FIELD_SIZE_ * (inAdvIndex + 1))], inRemainLength) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock();

			return (VS_ERROR);
		}
	}

	/* 關檔 */
	if (inFILE_Close(&ulADBHandle) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock();
	}

	free(uszAdvBuf);

	if (inFILE_Delete(uszADVFileName) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock();
	}

	if (inFILE_Rename(uszADBFileName, uszADVFileName) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock();
	}

	if (inFILE_Delete(uszADBFileName) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock();
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inADVICE_DeleteRecored()_END");
		inDISP_LogPrintf(szDebugMsg);
	}

	return (VS_SUCCESS);
}

/*
Function        :inADVICE_DeleteAll
Date&Time       :2016/9/20 上午 9:54
Describe        :刪掉該Host的adv檔
 */
int inADVICE_DeleteAll(TRANSACTION_OBJECT *pobTran)
{
	unsigned char uszFileName[16 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inADVICE_DeleteAll()_START");
	}

	if (inFunc_ComposeFileName(pobTran, (char*) uszFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	inFILE_Delete(uszFileName);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inADVICE_DeleteAll()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inADVICE_Update
Date&Time       :
Describe        :因gulADVHandle 是全域變數, 在外面一起關Handle
 */
int inADVICE_Update(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;
	long lnADVCnt;
	unsigned char *uszADVMemory;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inADVICE_Update()_START");
	}

	/* 取得 Advice 檔案大小 */
	lnADVCnt = lnFILE_GetSize(&gulADVHandle, guszADVFileName);
	if (lnADVCnt == 0)
	{
		return (VS_SUCCESS);
	} else
	{
		/* 記憶體 */
		uszADVMemory = malloc(lnADVCnt + 12);
		memset(uszADVMemory, 0x00, (lnADVCnt + 12));
		/* 讀 Advice 檔案資料，指到第二個 Advice */
		inRetVal = inFILE_Seek(gulADVHandle, _ADV_FIELD_SIZE_, _SEEK_BEGIN_); /* 指到第二個 Advice */
		if (inRetVal == VS_ERROR)
		{
			free(uszADVMemory);
			return (VS_ERROR);
		}

		lnADVCnt = lnADVCnt - 6;

		if (lnADVCnt == 0)
		{
			/* 如果lnADVCnt等於0，表示最後一筆 */
			/* 關檔案 */
			inFILE_Close(&gulADVHandle);
			/* 刪除檔案 */
			inFILE_Delete(guszADVFileName);
		} else
		{
			inRetVal = inFILE_Read(&gulADVHandle, &uszADVMemory[0], lnADVCnt);

			if (inRetVal == VS_ERROR)
			{
				inFILE_Close(&gulADVHandle);
				inFILE_Delete(guszADVFileName);
				free(uszADVMemory);
				if (inFILE_Create(&gulADVHandle, guszADVFileName) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("inFILE_Create gulADVHandle fail-1");
					}
				}

				return (VS_ERROR);
			}

			/* 關檔案 */
			inFILE_Close(&gulADVHandle);
			/* 刪除檔案 */
			inFILE_Delete(guszADVFileName);

			/* 重新建立 Advice 檔 */
			inRetVal = inFILE_Create(&gulADVHandle, guszADVFileName);
			if (inRetVal == VS_ERROR)
			{
				free(uszADVMemory);
				return (VS_ERROR);
			}

			inRetVal = inFILE_Seek(gulADVHandle, 0, _SEEK_END_); /* 指到最後第一個 BYTE */
			if (inRetVal == VS_ERROR)
			{
				free(uszADVMemory);
				return (VS_ERROR);
			}

			inRetVal = inFILE_Write(&gulADVHandle, &uszADVMemory[0], lnADVCnt);
			if (inRetVal == VS_ERROR)
			{
				free(uszADVMemory);
				return (VS_ERROR);
			}
		}
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inADVICE_Update()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inADVICE_GetTotalCount
Date&Time       :
Describe        :
 */
int inADVICE_GetTotalCount(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;
	long lnADVCnt;
	char szDebugMsg[100 + 1];
	unsigned long ulADVFHandle;
	unsigned char uszADVFileName[16 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inADVICE_GetTotalCount()_START");
	}

	if (inFunc_ComposeFileName(pobTran, (char*) uszADVFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inADVICE_GetTotalCount()uszADVFileName=%s", uszADVFileName);

	inRetVal = inFILE_OpenReadOnly(&ulADVFHandle, uszADVFileName);
	if (inRetVal == VS_ERROR)
	{
		return (VS_NO_RECORD);
	}

	/* 取得 Advice 檔案大小 */
	lnADVCnt = lnFILE_GetSize(&ulADVFHandle, uszADVFileName);
	if (lnADVCnt == 0)
	{
		inFILE_Close(&ulADVFHandle);
		inFILE_Delete(uszADVFileName);

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inADVICE_GetTotalCount(%ld)_END", lnADVCnt);
			inDISP_LogPrintf(szDebugMsg);
			inDISP_LogPrintf("----------------------------------------");
		}
		return (VS_NO_RECORD);
	} else
	{
		lnADVCnt = (lnADVCnt / _ADV_FIELD_SIZE_);
		inFILE_Close(&ulADVFHandle);
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inADVICE_GetTotalCount(%ld)_END", lnADVCnt);
		inDISP_LogPrintf(szDebugMsg);
		inDISP_LogPrintf("----------------------------------------");
	}

	return (lnADVCnt);
}

/*
Function        :inADVICE_GetInvoiceNum
Date&Time       :
Describe        :
 */
int inADVICE_GetInvoiceNum(TRANSACTION_OBJECT *pobTran, char *szGetInvoiceNum)
{
	int inRetVal;
	unsigned long ulADVFHandle;
	unsigned char uszADVFileName[16 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inADVICE_GetInvoiceNum()_START");
	}

	if (inFunc_ComposeFileName(pobTran, (char*) uszADVFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	inRetVal = inFILE_OpenReadOnly(&ulADVFHandle, uszADVFileName);
	if (inRetVal == VS_ERROR)
	{
		return (VS_NO_RECORD);
	}

	inRetVal = inFILE_Read(&ulADVFHandle, (unsigned char *) &szGetInvoiceNum[0], _ADV_FIELD_SIZE_);

	if (inRetVal == VS_ERROR)
	{
		inFILE_Close(&ulADVFHandle);
		return (VS_ERROR);
	}

	inFILE_Close(&ulADVFHandle);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inADVICE_GetInvoiceNum()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inADVICE_ESC_SaveAppend
Date&Time       :2017/3/29 上午 10:23
Describe        :For ESC 專用，附檔名為ADVE
 */
int inADVICE_ESC_SaveAppend(TRANSACTION_OBJECT *pobTran, long lnInvNum)
{
	int inRetVal;
	char szAdviceInvNum[10 + 1];
	unsigned char uszFileName[16 + 1] = {0};
	unsigned long ulFHandle;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inADVICE_ESC_SaveAppend()_START");
	}

	memset(uszFileName, 0x00, sizeof (uszFileName));
	if (inFunc_ComposeFileName(pobTran, (char*) uszFileName, _ADVICE_ESC_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	inRetVal = inFILE_Open(&ulFHandle, uszFileName);
	if (inRetVal == VS_ERROR)
	{
		/* 檔案不存在 */
		inRetVal = inFILE_Create(&ulFHandle, uszFileName);
		if (inRetVal == VS_ERROR)
		{
			return (VS_ERROR);
		}
	}

	inRetVal = inFILE_Seek(ulFHandle, 0, _SEEK_END_); /* 指到最後第一個 BYTE */
	if (inRetVal == VS_ERROR)
	{
		inFILE_Close(&ulFHandle);
		return (VS_ERROR);
	}

	memset(szAdviceInvNum, 0x00, sizeof (szAdviceInvNum));
	sprintf(szAdviceInvNum, "%06ld", lnInvNum);
	inRetVal = inFILE_Write(&ulFHandle, (unsigned char *) &szAdviceInvNum[0], _ADV_FIELD_SIZE_);
	if (inRetVal == VS_ERROR)
	{
		inFILE_Close(&ulFHandle);
		return (VS_ERROR);
	}

	inFILE_Close(&ulFHandle);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inADVICE_ESC_SaveAppend()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inADVICE_ESC_GetTotalCount
Date&Time       :2017/3/29 上午 11:01
Describe        :
 */
int inADVICE_ESC_GetTotalCount(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;
	long lnADVCnt;
	char szDebugMsg[100 + 1];
	unsigned long ulADVFHandle;
	unsigned char uszADVFileName[16 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inADVICE_ESC_GetTotalCount()_START");
	}

	if (inFunc_ComposeFileName(pobTran, (char*) uszADVFileName, _ADVICE_ESC_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	inRetVal = inFILE_OpenReadOnly(&ulADVFHandle, uszADVFileName);
	if (inRetVal == VS_ERROR)
	{
		return (VS_NO_RECORD);
	}

	/* 取得 Advice 檔案大小 */
	lnADVCnt = lnFILE_GetSize(&ulADVFHandle, uszADVFileName);
	if (lnADVCnt == 0)
	{
		inFILE_Close(&ulADVFHandle);
		inFILE_Delete(uszADVFileName);

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inADVICE_ESC_GetTotalCount(%ld)_END", lnADVCnt);
			inDISP_LogPrintf(szDebugMsg);
			inDISP_LogPrintf("----------------------------------------");
		}
		return (VS_NO_RECORD);
	} else
	{
		lnADVCnt = (lnADVCnt / _ADV_FIELD_SIZE_);
		inFILE_Close(&ulADVFHandle);
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inADVICE_ESC_GetTotalCount(%ld)_END", lnADVCnt);
		inDISP_LogPrintf(szDebugMsg);
		inDISP_LogPrintf("----------------------------------------");
	}

	return (lnADVCnt);
}

/*
Function        :inADVICE_ESC_DeleteRecordFlow
Date&Time       :2016/9/20 上午 9:57
Describe        :檢查 lnInvNum 是否在 Advice 檔案中，若是則將 lnInvNum 從 Advice 檔案中刪除
 */
int inADVICE_ESC_DeleteRecordFlow(TRANSACTION_OBJECT *pobTran, long lnInvNum)
{
	int inDeleteAdvIndex = 0;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inADVICE_ESC_DeleteRecordFlow() START !");
	}

	if (inADVICE_ESC_SearchRecord_Index(pobTran, lnInvNum, &inDeleteAdvIndex) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 刪除中間的值 */
	if (inDeleteAdvIndex >= 0)
	{
		if (inADVICE_ESC_DeleteRecored(pobTran, inDeleteAdvIndex) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock();

			return (VS_ERROR);
		}
	}


	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inADVICE_ESC_DeleteRecordFlow() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inADVICE_ESC_SearchRecord_Index
Date&Time       :2017/3/29 下午 1:23
Describe        :ESC專用
 */
int inADVICE_ESC_SearchRecord_Index(TRANSACTION_OBJECT *pobTran, long lnInvNum, int *inADVIndex)
{
	int inRecCnt = 0;
	int inRetVal;
	char szDebugMsg[100 + 1];
	char szReadBuf[_ADV_FIELD_SIZE_ + 1];
	unsigned char uszADVFileName[16 + 1];
	unsigned long ulHandle;

	if (inFunc_ComposeFileName(pobTran, (char*) uszADVFileName, _ADVICE_ESC_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 找到該advice index */
	if (inFILE_OpenReadOnly(&ulHandle, uszADVFileName) != VS_SUCCESS)
		return (VS_ERROR);

	inRecCnt = (int) (lnFILE_GetSize(&ulHandle, uszADVFileName) / _ADV_FIELD_SIZE_);

	/* 從頭開始搜索 */
	if (inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		if (inFILE_Close(&ulHandle) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock();
		}

		return (VS_ERROR);
	}

	do
	{
		memset(szReadBuf, 0x00, _ADV_FIELD_SIZE_ + 1);
		inRetVal = inFILE_Read(&ulHandle, (unsigned char*) szReadBuf, _ADV_FIELD_SIZE_);

		if (inRetVal != VS_SUCCESS)
		{
			inFILE_Close(&ulHandle);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "inADVICE_ESC_SearchRecord_Index ERROR!!");
				inDISP_LogPrintf(szDebugMsg);
			}

			return (VS_ERROR);
		} else
		{
			if (lnInvNum == atol(szReadBuf))
			{
				break;
			}

			/* index + 1 */
			*inADVIndex += 1;
		}

	} while (*inADVIndex <= inRecCnt);

	if (inFILE_Close(&ulHandle) != VS_SUCCESS)
		return (VS_ERROR);

	return (VS_SUCCESS);
}

/*
Function        :inADVICE_ESC_DeleteRecored
Date&Time       :2017/3/29 下午 1:24
Describe        :ESC專用，刪除ADV檔中的特定invoice number的紀錄
 */
int inADVICE_ESC_DeleteRecored(TRANSACTION_OBJECT *pobTran, int inAdvIndex)
{
	int i;
	int inAdvSize; /* ADV檔總長度 */
	int inRemainLength; /* 後半段需寫入的長度 */
	long lnReadLength; /* 剩餘讀取長度 */
	char szDebugMsg[100 + 1];
	unsigned char *uszAdvBuf;
	unsigned char uszADVFileName[16 + 1], uszADBFileName[16 + 1];
	unsigned long ulADVHandle, ulADBHandle;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inADVICE_ESC_DeleteRecored() START !");
	}

	if (inFunc_ComposeFileName(pobTran, (char*) uszADVFileName, _ADVICE_ESC_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (inFunc_ComposeFileName(pobTran, (char*) uszADBFileName, _ADVICE_ESC_BACKUP_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 原始檔案 */
	if (inFILE_OpenReadOnly(&ulADVHandle, uszADVFileName) != VS_SUCCESS)
	{
		inFILE_Close(&ulADVHandle);

		/* 鎖機 */
		inFunc_EDCLock();

		return (VS_ERROR);
	}

	/* 抓檔案長度 */
	inAdvSize = (int) lnFILE_GetSize(&ulADVHandle, uszADVFileName);

	/* 表示只有一筆記錄，直接刪檔即可 */
	if (inAdvSize == _ADV_FIELD_SIZE_)
	{

		if (inFILE_Close(&ulADVHandle) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock();
		}

		if (inFILE_Delete(uszADVFileName) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock();
		}

		return (VS_SUCCESS);
	}

	/* 若有多筆紀錄，從頭開始搜索 */
	if (inFILE_Seek(ulADVHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		if (inFILE_Close(&ulADVHandle) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock();
		}

		return (VS_ERROR);
	}

	uszAdvBuf = malloc(inAdvSize + 1);
	/* 剩餘讀取長度 */
	lnReadLength = inAdvSize;

	/* 先讀出所有adv內容 */
	for (i = 0;; ++i)
	{
		memset(uszAdvBuf, 0x00, inAdvSize + 1);
		inFILE_Read(&ulADVHandle, uszAdvBuf, 1024);
		/* 剩餘長度大於或等於1024 */
		if (lnReadLength >= 1024)
		{
			if (inFILE_Read(&ulADVHandle, &uszAdvBuf[1024 * i], 1024) == VS_SUCCESS)
			{
				/* 一次讀1024 */
				lnReadLength -= 1024;

				/* 當剩餘長度剛好為1024，會剛好讀完 */
				if (lnReadLength == 0)
					break;
			}				/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulADVHandle);

				/* Free pointer */
				free(uszAdvBuf);

				/* 鎖機 */
				inFunc_EDCLock();

				return (VS_ERROR);
			}
		}			/* 剩餘長度小於1024 */
		else if (lnReadLength < 1024)
		{
			/* 就只讀剩餘長度 */
			if (inFILE_Read(&ulADVHandle, &uszAdvBuf[1024 * i], lnReadLength) == VS_SUCCESS)
			{
				break;
			}				/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulADVHandle);

				/* Free pointer */
				free(uszAdvBuf);

				/* 鎖機 */
				inFunc_EDCLock();

				return (VS_ERROR);
			}
		}

	};

	/* 關檔 */
	if (inFILE_Close(&ulADVHandle) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock();
	}

	/* 防呆先刪除ADB */
	inFILE_Delete((unsigned char *) uszADBFileName);
	/* 建新的備份檔案 */
	if (inFILE_Create(&ulADBHandle, uszADBFileName) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock();

		return (VS_ERROR);
	}

	/* 若有多筆紀錄，從頭開始搜索 */
	if (inFILE_Seek(ulADBHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		if (inFILE_Close(&ulADBHandle) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock();
		}

		return (VS_ERROR);
	}

	/* 寫入前半段，inAdvIndex為0則代表前半段沒資料需要寫 */
	if (inAdvIndex != 0)
	{
		if (inFILE_Write(&ulADBHandle, uszAdvBuf, _ADV_FIELD_SIZE_ * inAdvIndex) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock();

			return (VS_ERROR);
		}

	}

	/* 跳過那一筆不寫，並移handle到最後面 */
	if (inFILE_Seek(ulADBHandle, 0, _SEEK_END_) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock();

		return (VS_ERROR);
	}

	/* 寫入後半段，若剩餘資料長度大於0，代表後面需要寫 */
	inRemainLength = inAdvSize - (_ADV_FIELD_SIZE_ * (inAdvIndex + 1));
	if (inRemainLength > 0)
	{
		if (inFILE_Write(&ulADBHandle, &uszAdvBuf[(_ADV_FIELD_SIZE_ * (inAdvIndex + 1))], inRemainLength) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock();

			return (VS_ERROR);
		}
	}

	/* 關檔 */
	if (inFILE_Close(&ulADBHandle) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock();
	}

	free(uszAdvBuf);

	if (inFILE_Delete(uszADVFileName) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock();
	}

	if (inFILE_Rename(uszADBFileName, uszADVFileName) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock();
	}

	if (inFILE_Delete(uszADBFileName) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock();
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inADVICE_ESC_DeleteRecored()_END");
		inDISP_LogPrintf(szDebugMsg);
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_FuncUpdateTxnRecord_By_Sqlite
Date&Time       :2017/3/14 下午 5:33
Describe        :insert資料到資料庫
 */
int inBATCH_FuncUpdateTxnRecord_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_SUCCESS;
	char szTableID[12 + 1] = {0};
	char szDebugMsg[100 + 1] = {0};

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_DispLogAndWriteFlie(" uszUpdateBatchBit[%d] inChipStatus[%d] uszContactlessBit[%d] uszEMVFallBackBit[%d]",
			pobTran->uszUpdateBatchBit,
			pobTran->srBRec.inChipStatus,
			pobTran->srBRec.uszContactlessBit,
			pobTran->srBRec.uszEMVFallBackBit);

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inBATCH_FuncUpdateTxnRecord_By_Sqlite INIT");

	/* 如果是update，不用再試圖創Table */
	if (pobTran->uszUpdateBatchBit == VS_TRUE)
	{
		inRetVal = inSqlite_Check_Table_Exist_Flow(pobTran, _TN_BATCH_TABLE_);
		/* 如果要update卻沒有table，直接跳出 */
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" Update Rec Check Table *Error* Line[%d]", __LINE__);
			inFunc_EDCLock();
		}
	} else
	{
		inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_);
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" Update Rec Create Rec Batch Table *Error* Line[%d]", __LINE__);
			inFunc_EDCLock();
		}
		inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_EMV_TABLE_);
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" Update Rec Create Emv Batch Table *Error* Line[%d]", __LINE__);
			inFunc_EDCLock();
		}
	}

	/* 如果uszUpdate設為1 */
	if (pobTran->uszUpdateBatchBit == VS_TRUE)
	{
		inRetVal = inSqlite_Update_ByInvNum_TranState_Flow(pobTran, _TN_BATCH_TABLE_, pobTran->srBRec.lnOrgInvNum);
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" Update Rec Update By Inv *Error* Line[%d]", __LINE__);
			inFunc_EDCLock();
		}
	}

	/* Insert批次 */
	inRetVal = inBATCH_Insert_All_Flow(pobTran, _TN_BATCH_TABLE_);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" Update Rec Insert Batch Table *Error* Line[%d]", __LINE__);
		inFunc_EDCLock();
	}

	memset(szTableID, 0x00, sizeof (szTableID));
	/* 抓出TableID */
	inRetVal = inSqlite_Get_Max_TableID_Flow(pobTran, _TN_BATCH_TABLE_, szTableID);
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" Update Rec Get Max Table Id *Error* Line[%d]", __LINE__);
		inFunc_EDCLock();
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inTableID : %d", atoi(szTableID));
			inDISP_LogPrintf(szDebugMsg);
		}
		pobTran->inTableID = atoi(szTableID);
	}

	/* 算出是否有【EMV】和【Contactless】交易 */
	if (pobTran->srBRec.inChipStatus != 0 ||
		pobTran->srBRec.uszContactlessBit == VS_TRUE ||
		pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
	{
		inRetVal = inBATCH_Insert_All_Flow(pobTran, _TN_EMV_TABLE_);
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" Update Rec Insert Emv Table *Error* Line[%d]", __LINE__);
			inFunc_EDCLock();
		}
	}

	/* 強制同步
	 * Linux 系統中，為了加快資料的讀取速度，寫入硬盤的資料有的時候為了效能，會寫到 filesystem buffer 中，
	 * 這個 buffer 是一塊記憶體空間，如果欲寫入硬碟的資料存此 buffer 中，而系統又突然斷電的話，那資料就會流失!
	 * 可以透sync 指令，將存 buffer 中的資料強制寫入disk中；
	 * 或是在程式中利用fsync() function
	 * 虹堡文件有提到，這個指令會減少NAND壽命，但為了不掉帳還是要用
	 *
	 */
	sync();

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inBATCH_FuncUpdateTxnRecord_By_Sqlite END");
	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inBATCH_CheckReport_By_Sqlite
Date&Time       :2017/3/14 下午 5:34
Describe        :處理 inBATCH_GetTotalCountFromBakFile()的錯誤訊息
 */
int inBATCH_CheckReport_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int inRetCnt = -1;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_CheckReport_By_Sqlite()_START");
	}

	inRetCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
	if (inRetCnt == 0)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 顯示無交易紀錄 */
		inDISP_Msg_BMP(_ERR_RECORD_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, 2, "", 0);

		return (VS_ERROR);
	} else if (inRetCnt == VS_ERROR)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 顯示讀取批次檔錯誤 */
		inDISP_Msg_BMP(_ERR_BATCH_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, 2, "", 0);

		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_CheckReport_By_Sqlite()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GetTotalCountFromBakFile_By_Sqlite
Date&Time       :2017/3/15 下午 5:49
Describe        :有幾筆record，回傳值為筆數
 */
int inBATCH_GetTotalCountFromBakFile_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;
	int inCnt = -1;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GetTotalCountFromBakFile_By_Sqlite()_START");
	}

	inRetVal = inSqlite_Get_Batch_Count_Flow(pobTran, _TN_BATCH_TABLE_, &inCnt);
	if (inRetVal == VS_NO_RECORD)
	{
		return (0);
	} else if (inRetVal != VS_SUCCESS)
	{
		return (-1);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GetTotalCountFromBakFile_By_Sqlite()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (inCnt);
}

/*
Function        :inBATCH_GetTotalCountFromBakFile_By_Sqlite_ESVC
Date&Time       :2018/1/31 下午 1:20
Describe        :有幾筆record，回傳值為筆數
 */
int inBATCH_GetTotalCountFromBakFile_By_Sqlite_ESVC(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;
	int inCnt = -1;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GetTotalCountFromBakFile_By_Sqlite_ESVC()_START");
	}

	inRetVal = inSqlite_Get_Batch_Count_Flow(pobTran, _TN_BATCH_TABLE_TICKET_, &inCnt);
	if (inRetVal == VS_NO_RECORD)
	{
		return (VS_NO_RECORD);
	} else if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GetTotalCountFromBakFile_By_Sqlite_ESVC()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (inCnt);
}

/*
Function        :inBATCH_ReviewReport_Detail_Flow_By_Sqlite
Date&Time       :2017/6/12 下午 5:42
Describe        :交易查詢分流
 */
int inBATCH_ReviewReport_Detail_Flow_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_SUCCESS;
	char szFunEnable[2 + 1];

	memset(szFunEnable, 0x00, sizeof (szFunEnable));
	inGetStore_Stub_CardNo_Truncate_Enable(szFunEnable);

	if (memcmp(szFunEnable, "Y", strlen("Y")) == 0)
	{
		inRetVal = inBATCH_ReviewReport_Detail_Txno_By_Sqlite(pobTran);
	} else
	{
		inRetVal = inBATCH_ReviewReport_Detail_By_Sqlite(pobTran);
	}

	return (inRetVal);
}

/*
Function        :inBATCH_ReviewReport_Detail_By_Sqlite
Date&Time       :2017/3/14 下午 5:48
Describe        :交易查詢使用
 */
int inBATCH_ReviewReport_Detail_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int inNowindex = 0;
	int inFileRecCnt = -1;
	int inTouchPage = 0;
	char szTemplate1[42 + 1];
	char szTemplate2[42 + 1];
	char szFinalPAN[_PAN_UCARD_SIZE_ + 1];
	char szDispMsg [50 + 1];
	char szDebugMsg[100 + 1];
	char szKey = 0x00;
	char szTRTFileName[16 + 1];
	unsigned char uszBatchEndBit = VS_FALSE;
	unsigned char uszBatchCancelBit = VS_FALSE;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_ReviewReport_Detail_By_Sqlite()_START");
	}

	/* 算出交易總筆數 */
	inFileRecCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "筆數: %d", inFileRecCnt);
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 沒交易紀錄 */
	if (inFileRecCnt == 0)
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第一層顯示 <交易查詢> */
		return (VS_NO_RECORD);
	}

	/* 先抓出總資料 */
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);

	while (inNowindex < inFileRecCnt)
	{
		/* 開始讀取每一筆交易記錄 */
		if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, inNowindex) != VS_SUCCESS)
		{
			uszBatchEndBit = VS_TRUE;
			break;
		}

		inDISP_ClearAll();

		if (uszBatchEndBit == VS_TRUE ||
				uszBatchCancelBit == VS_TRUE)
		{
			break;
		}

		inGetTRTFileName(szTRTFileName);

		/* DCC轉台幣的紀錄要特別處理，原先的那筆DCC一般交易不能出現 */
		if (pobTran->srBRec.inCode == _VOID_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)
		{
			if (inTouchPage == _DisTouch_Slide_Right_To_Left_ || szKey == _KEY_ENTER_ || szKey == _KEY_DOWN_)
			{
				inNowindex++;
				continue;
			} else if (inTouchPage == _DisTouch_Slide_Left_To_Right_ || szKey == _KEY_UP_)
			{
				inNowindex--;

				/* 交易紀錄編號不可小於0 */
				if (inNowindex < 0)
				{
					inNowindex = 0;
					/* 加這行是因為，若第一筆是DCC轉台幣會陷入無限迴圈，所以改滑動方向 */
					inTouchPage = _DisTouch_Slide_Right_To_Left_;
					szKey = _KEY_ENTER_;
				}
				continue;
			}				/* 若第一筆就是要跳過的紀錄，會因為沒有event，而沒有跳過，所以預設為下一頁 */
			else
			{
				inNowindex++;
				continue;
			}

		}

		/* 優惠兌換的顯示比較特別 */
		if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_ ||
				pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
		{
			/* Invoice Number */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_)
			{
				sprintf(szDispMsg, "優惠兌換     %03ld", pobTran->srBRec.lnOrgInvNum);
			} else if (pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
			{
				sprintf(szDispMsg, "兌換取消     %03ld", pobTran->srBRec.lnOrgInvNum);
			}
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_1_, _DISP_LEFT_);

			/* Data & Time */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, "%.2s/%.2s/%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_2_, _DISP_LEFT_);

			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, "%.2s:%.2s:%.2s", &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2], &pobTran->srBRec.szTime[4]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_3_, _DISP_LEFT_);
		} else
		{


			/* Disp Card Label & Expire date*/
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			memset(szTemplate1, 0x00, sizeof (szTemplate1));
			inGetTRTFileName(szTemplate1);
			/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
			/* 金融卡沒有效期 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				sprintf(szDispMsg, " %s", pobTran->srBRec.szCardLabel);
			}				/* 其他加密 */
			else
			{
				memset(szTemplate1, 0x00, sizeof (szTemplate1));
				inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
				sprintf(szDispMsg, " %s   %s", pobTran->srBRec.szCardLabel, szTemplate1);
			}

			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_1_, _DISP_LEFT_);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "CardLabel: %s", pobTran->srBRec.szCardLabel);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Disp Card Number */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			memset(szFinalPAN, 0x00, sizeof (szFinalPAN));
			strcpy(szFinalPAN, pobTran->srBRec.szPAN);

			if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
			{
				inFunc_FormatPAN_UCARD(szFinalPAN);
			}
			sprintf(szDispMsg, " %s", szFinalPAN);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "szPAN: %s", pobTran->srBRec.szPAN);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Transaction Type */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			memset(szTemplate1, 0x00, sizeof (szTemplate1));
			memset(szTemplate2, 0x00, sizeof (szTemplate2));
			inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
			if (strlen(szTemplate2) > 0)
			{
				sprintf(szDispMsg, " %s", szTemplate1);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_5_, _DISP_LEFT_);
				memset(szDispMsg, 0x00, sizeof (szDispMsg));
				sprintf(szDispMsg, " %s", szTemplate2);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_6_, _DISP_LEFT_);
			} else
			{
				sprintf(szDispMsg, " %s %s", szTemplate1, szTemplate2);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
			}


			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Tran Type: %s %s", szTemplate1, szTemplate2);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* 授權碼(金卡顯示調單編號) */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				sprintf(szDispMsg, " RRN NO.: %s", pobTran->srBRec.szFiscRRN);
			} else
			{
				sprintf(szDispMsg, " APPR: %s", pobTran->srBRec.szAuthCode);
			}
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_7_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Reference Number */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, " REF: %s", pobTran->srBRec.szRefNo);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_8_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Invoice Number */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, " INV: %06ld", pobTran->srBRec.lnOrgInvNum);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%06ld", pobTran->srBRec.lnOrgInvNum);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* 是否有優惠兌換資訊 */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			if (pobTran->srBRec.uszVOIDBit == VS_FALSE &&
					atoi(pobTran->srBRec.szAwardNum) > 0)
			{
				strcpy(szDispMsg, "award");
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_RIGHT_);
			} else if (pobTran->srBRec.uszVOIDBit == VS_TRUE &&
					atoi(pobTran->srBRec.szAwardNum) > 0)
			{
				strcpy(szDispMsg, "void awd");
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_RIGHT_);
			}

			/* Amount */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));

			if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
			{
				switch (pobTran->srBRec.inCode)
				{
					case _SALE_:
					case _INST_SALE_:
					case _REDEEM_SALE_:
					case _SALE_OFFLINE_:
					case _FORCE_CASH_ADVANCE_:
					case _PRE_COMP_:
					case _PRE_AUTH_:
					case _CUP_SALE_:
					case _CUP_PRE_COMP_:
					case _CUP_PRE_AUTH_:
					case _INST_ADJUST_:
					case _REDEEM_ADJUST_:
					case _MAIL_ORDER_:
					case _CUP_MAIL_ORDER_:
					case _CASH_ADVANCE_:
					case _FISC_SALE_:
						if (pobTran->srBRec.lnTipTxnAmount != 0)
							sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						else
							sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _ADJUST_:
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _TIP_:
						sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _REFUND_:
					case _INST_REFUND_:
					case _REDEEM_REFUND_:
					case _CUP_REFUND_:
					case _CUP_MAIL_ORDER_REFUND_:
					case _FISC_REFUND_:
						sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					default:
						sprintf(szDispMsg, " AMT_ERR_(%d)", pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			} else
			{
				switch (pobTran->srBRec.inOrgCode)
				{
					case _SALE_:
					case _INST_SALE_:
					case _REDEEM_SALE_:
					case _SALE_OFFLINE_:
					case _FORCE_CASH_ADVANCE_:
					case _PRE_COMP_:
					case _PRE_AUTH_:
					case _CUP_SALE_:
					case _CUP_PRE_COMP_:
					case _CUP_PRE_AUTH_:
					case _INST_ADJUST_:
					case _REDEEM_ADJUST_:
					case _MAIL_ORDER_:
					case _CUP_MAIL_ORDER_:
					case _CASH_ADVANCE_:
					case _FISC_SALE_:
						sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _ADJUST_:
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
						/* 目前小費沒有取消 */
					case _REFUND_:
					case _INST_REFUND_:
					case _REDEEM_REFUND_:
					case _CUP_REFUND_:
					case _CUP_MAIL_ORDER_REFUND_:
						/* SmartPay不能取消 */
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					default:
						sprintf(szDispMsg, " AMT_VOID_ERR_(%d)", pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Store ID */
			memset(szTemplate1, 0x00, sizeof (szTemplate1));
			memset(szDispMsg, 0x00, sizeof (szDispMsg));

			inGetStoreIDEnable(&szTemplate1[0]);

			if ((szTemplate1[0] == 'Y') && (strlen(pobTran->srBRec.szStoreID) > 0))
			{
				if (strlen(pobTran->srBRec.szStoreID) > 42)
					memcpy(&szTemplate1[0], &pobTran->srBRec.szStoreID[0], 42);
				else
					strcpy(szTemplate1, pobTran->srBRec.szStoreID);

				szTemplate1[18] = 0x00;

				sprintf(szDispMsg, " STD:%s", pobTran->srBRec.szStoreID);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
			} else
			{
				/* 沒開櫃號功能， 不印櫃號 */
			}

			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, "%d / %d", inNowindex + 1, inFileRecCnt);
			inDISP_ChineseFont_Point_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_8_, _COLOR_BLACK_, _COLOR_LIGHTBLUE_, 10);
		}

		/* 事件初始化 */
		szKey = 0;
		/* 設定Timeout */
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);

		while (1)
		{

			/* 偵測滑動 */
			inTouchPage = inDisTouch_TouchSensor_Click_Slide(_Touch_NEWUI_REVIEW_BATCH_);
			if (inTouchPage == _DisTouch_Slide_Right_To_Left_)
			{
				++inNowindex;
				break;
			}
			if (inTouchPage == _DisTouch_Slide_Left_To_Right_)
			{
				--inNowindex;

				/* 交易紀錄編號不可小於0 */
				if (inNowindex < 0)
				{
					inNowindex = 0;
				}
				break;
			}

			/* 偵測按鍵 */
			szKey = uszKBD_Key();

			/* TimeOut */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
			{
				uszBatchCancelBit = VS_TRUE;
				break;
			} else if (szKey == _KEY_ENTER_ || szKey == _KEY_DOWN_)
			{
				++inNowindex;
				break;
			} else if (szKey == _KEY_UP_ || szKey == _KEY_FUNCTION_)
			{
				--inNowindex;
				/* 交易紀錄編號不可小於0 */
				if (inNowindex < 0)
				{
					inNowindex = 0;
				}
				break;
			}

		}

	} /* End for () .... */

	inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);

	if (uszBatchEndBit == VS_TRUE)
	{
		/* 顯示批次結束 */
		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第一層顯示 <交易查詢> */
		inDISP_Msg_BMP(_ERR_BATCH_END_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_ReviewReport_Detail_By_Sqlite()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_ReviewReport_Detail_Txno_By_Sqlite
Date&Time       :2017/6/12 下午 5:54
Describe        :交易查詢使用，for卡號遮掩
 */
int inBATCH_ReviewReport_Detail_Txno_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int i = 0;
	int inNowindex = 0;
	int inFileRecCnt = -1;
	int inTouchPage = 0;
	char szTemplate1[42 + 1];
	char szTemplate2[42 + 1];
	char szFinalPAN[_PAN_UCARD_SIZE_ + 1];
	char szDispMsg [50 + 1];
	char szDebugMsg[100 + 1];
	char szKey = 0x00;
	char szTRTFileName[16 + 1];
	unsigned char uszBatchEndBit = VS_FALSE;
	unsigned char uszBatchCancelBit = VS_FALSE;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_ReviewReport_Detail_Txno_By_Sqlite()_START");
	}

	/* 算出交易總筆數 */
	inFileRecCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "筆數: %d", inFileRecCnt);
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 沒交易紀錄 */
	if (inFileRecCnt == 0)
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第一層顯示 <交易查詢> */
		return (VS_NO_RECORD);
	}

	/* 先抓出總資料 */
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);

	while (inNowindex < inFileRecCnt)
	{
		/* 開始讀取每一筆交易記錄 */
		if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, inNowindex) != VS_SUCCESS)
		{
			uszBatchEndBit = VS_TRUE;
			break;
		}

		inDISP_ClearAll();

		if (uszBatchEndBit == VS_TRUE ||
				uszBatchCancelBit == VS_TRUE)
			break;

		inGetTRTFileName(szTRTFileName);

		/* DCC轉台幣的紀錄要特別處理，原先的那筆DCC一般交易不能出現 */
		if (pobTran->srBRec.inCode == _VOID_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)
		{
			if (inTouchPage == _DisTouch_Slide_Right_To_Left_ || szKey == _KEY_ENTER_ || szKey == _KEY_DOWN_)
			{
				inNowindex++;
				continue;
			} else if (inTouchPage == _DisTouch_Slide_Left_To_Right_ || szKey == _KEY_UP_)
			{
				inNowindex--;

				/* 交易紀錄編號不可小於0 */
				if (i < 0)
				{
					i = 0;
					/* 加這行是因為，若第一筆是DCC轉台幣會陷入無限迴圈，所以改滑動方向 */
					inTouchPage = _DisTouch_Slide_Right_To_Left_;
					szKey = _KEY_ENTER_;
				}
				continue;
			}				/* 若第一筆就是要跳過的紀錄，會因為沒有event，而沒有跳過，所以預設為下一頁 */
			else
			{
				inNowindex++;
				continue;
			}

		}

		/* 優惠兌換的顯示比較特別 */
		if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_ ||
				pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
		{
			/* Invoice Number */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_)
			{
				sprintf(szDispMsg, "優惠兌換     %03ld", pobTran->srBRec.lnOrgInvNum);
			} else if (pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
			{
				sprintf(szDispMsg, "兌換取消     %03ld", pobTran->srBRec.lnOrgInvNum);
			}
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_1_, _DISP_LEFT_);

			/* Data & Time */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, "%.2s/%.2s/%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_2_, _DISP_LEFT_);

			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, "%.2s:%.2s:%.2s", &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2], &pobTran->srBRec.szTime[4]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_3_, _DISP_LEFT_);
		} else
		{
			/* Invoice Number */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, " INV# %06ld", pobTran->srBRec.lnOrgInvNum);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_1_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%06ld", pobTran->srBRec.lnOrgInvNum);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Transaction Type */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			memset(szTemplate1, 0x00, sizeof (szTemplate1));
			memset(szTemplate2, 0x00, sizeof (szTemplate2));
			inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
			if (strlen(szTemplate2) > 0)
			{
				sprintf(szDispMsg, " %s", szTemplate1);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_3_, _DISP_LEFT_);
				memset(szDispMsg, 0x00, sizeof (szDispMsg));
				sprintf(szDispMsg, " %s", szTemplate2);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_4_, _DISP_LEFT_);
			} else
			{
				sprintf(szDispMsg, " %s %s", szTemplate1, szTemplate2);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
			}


			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Tran Type: %s %s", szTemplate1, szTemplate2);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Disp Card Number */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			memset(szFinalPAN, 0x00, sizeof (szFinalPAN));
			strcpy(szFinalPAN, pobTran->srBRec.szPAN);

			if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
			{
				inFunc_FormatPAN_UCARD(szFinalPAN);
			}

			/* 卡號遮掩 */
			/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
			if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
			{
				for (i = 3; i < (strlen(szFinalPAN) - 5); i++)
					szFinalPAN[i] = 0x2A;
			} else
			{
				for (i = 6; i < (strlen(szFinalPAN) - 4); i++)
					szFinalPAN[i] = 0x2A;
			}

			sprintf(szDispMsg, " %s", szFinalPAN);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "szPAN: %s", pobTran->srBRec.szPAN);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Check Number or Exp Date*/
			memset(szTemplate1, 0x00, sizeof (szTemplate1));
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			inGetTRTFileName(szTemplate1);

			/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				memset(szTemplate1, 0x00, sizeof (szTemplate1));
				inGetCardLabel(szTemplate1);
				sprintf(szDispMsg, "%s", szTemplate1);
			} else
			{
				/* Check Number */
				memset(szTemplate1, 0x00, sizeof (szTemplate1));
				inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
				sprintf(szDispMsg, " Check# %s", szTemplate1);
			}
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);

			/* Data & Time */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, " 20%.2s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);

			/* 授權碼(金卡顯示調單編號) */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				sprintf(szDispMsg, " RRN NO.: %s", pobTran->srBRec.szFiscRRN);
			} else
			{
				sprintf(szDispMsg, " APP# %s", pobTran->srBRec.szAuthCode);
			}
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);

			/* 是否有優惠兌換資訊 */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			if (pobTran->srBRec.uszVOIDBit == VS_FALSE &&
					atoi(pobTran->srBRec.szAwardNum) > 0)
			{
				strcpy(szDispMsg, "award ");
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _DISP_RIGHT_);
			} else if (pobTran->srBRec.uszVOIDBit == VS_TRUE &&
					atoi(pobTran->srBRec.szAwardNum) > 0)
			{
				strcpy(szDispMsg, "void awd ");
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _DISP_RIGHT_);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Amount */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));

			if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
			{
				switch (pobTran->srBRec.inCode)
				{
					case _SALE_:
					case _INST_SALE_:
					case _REDEEM_SALE_:
					case _SALE_OFFLINE_:
					case _FORCE_CASH_ADVANCE_:
					case _PRE_COMP_:
					case _PRE_AUTH_:
					case _CUP_SALE_:
					case _CUP_PRE_COMP_:
					case _CUP_PRE_AUTH_:
					case _INST_ADJUST_:
					case _REDEEM_ADJUST_:
					case _MAIL_ORDER_:
					case _CUP_MAIL_ORDER_:
					case _CASH_ADVANCE_:
					case _FISC_SALE_:
						if (pobTran->srBRec.lnTipTxnAmount != 0)
							sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						else
							sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "AMT $ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _ADJUST_:
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "AMT $ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _TIP_:
						sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "AMT $ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _REFUND_:
					case _INST_REFUND_:
					case _REDEEM_REFUND_:
					case _CUP_REFUND_:
					case _CUP_MAIL_ORDER_REFUND_:
					case _FISC_REFUND_:
						sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "AMT $ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					default:
						sprintf(szDispMsg, " AMT_ERR_(%d)", pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			} else
			{
				switch (pobTran->srBRec.inOrgCode)
				{
					case _SALE_:
					case _INST_SALE_:
					case _REDEEM_SALE_:
					case _SALE_OFFLINE_:
					case _FORCE_CASH_ADVANCE_:
					case _PRE_COMP_:
					case _PRE_AUTH_:
					case _CUP_SALE_:
					case _CUP_PRE_COMP_:
					case _CUP_PRE_AUTH_:
					case _INST_ADJUST_:
					case _REDEEM_ADJUST_:
					case _MAIL_ORDER_:
					case _CUP_MAIL_ORDER_:
					case _CASH_ADVANCE_:
					case _FISC_SALE_:
						sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "AMT $ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _ADJUST_:
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "AMT $ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
						/* 目前小費沒有取消 */
					case _REFUND_:
					case _INST_REFUND_:
					case _REDEEM_REFUND_:
					case _CUP_REFUND_:
					case _CUP_MAIL_ORDER_REFUND_:
						/* SmartPay不能取消 */
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "AMT $ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					default:
						sprintf(szDispMsg, " AMT_VOID_ERR_(%d)", pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Store ID */
			memset(szTemplate1, 0x00, sizeof (szTemplate1));
			memset(szDispMsg, 0x00, sizeof (szDispMsg));

			inGetStoreIDEnable(&szTemplate1[0]);

			if ((szTemplate1[0] == 'Y') && (strlen(pobTran->srBRec.szStoreID) > 0))
			{
				if (strlen(pobTran->srBRec.szStoreID) > 42)
					memcpy(&szTemplate1[0], &pobTran->srBRec.szStoreID[0], 42);
				else
					strcpy(szTemplate1, pobTran->srBRec.szStoreID);

				szTemplate1[18] = 0x00;

				sprintf(szDispMsg, " STD:%s", pobTran->srBRec.szStoreID);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
			} else
			{
				/* 沒開櫃號功能， 不印櫃號 */
			}

			//			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			//			sprintf(szDispMsg, "%d / %d", inNowindex + 1, inFileRecCnt);
			//			inDISP_ChineseFont_Point_Color(szDispMsg, _FONESIZE_8X22_, _LINE_8X22_8_, _COLOR_BLACK_, _COLOR_LIGHTBLUE_, 10);
		}

		/* 事件初始化 */
		szKey = 0;
		/* 設定Timeout */
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);

		while (1)
		{

			/* 偵測滑動 */
			inTouchPage = inDisTouch_TouchSensor_Click_Slide(_Touch_NEWUI_REVIEW_BATCH_);
			if (inTouchPage == _DisTouch_Slide_Right_To_Left_)
			{
				++inNowindex;
				break;
			}
			if (inTouchPage == _DisTouch_Slide_Left_To_Right_)
			{
				--inNowindex;

				/* 交易紀錄編號不可小於0 */
				if (inNowindex < 0)
				{
					inNowindex = 0;
				}
				break;
			}

			/* 偵測按鍵 */
			szKey = uszKBD_Key();

			/* TimeOut */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
			{
				uszBatchCancelBit = VS_TRUE;
				break;
			} else if (szKey == _KEY_ENTER_ || szKey == _KEY_DOWN_)
			{
				++inNowindex;
				break;
			} else if (szKey == _KEY_UP_ || szKey == _KEY_FUNCTION_)
			{
				--inNowindex;
				/* 交易紀錄編號不可小於0 */
				if (inNowindex < 0)
				{
					inNowindex = 0;
				}
				break;
			}

		}

	} /* End for () .... */

	inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);

	/* 顯示批次結束 */
	if (uszBatchEndBit == VS_TRUE)
	{
		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第一層顯示 <交易查詢> */
		inDISP_Msg_BMP(_ERR_BATCH_END_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_ReviewReport_Detail_Txno_By_Sqlite()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_ReviewReport_Detail_NEWUI_Flow_By_Sqlite
Date&Time       :2017/6/12 下午 5:42
Describe        :交易查詢分流
 */
int inBATCH_ReviewReport_Detail_NEWUI_Flow_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_SUCCESS;
	char szFunEnable[2 + 1];

	memset(szFunEnable, 0x00, sizeof (szFunEnable));
	inGetStore_Stub_CardNo_Truncate_Enable(szFunEnable);

	if (memcmp(szFunEnable, "Y", strlen("Y")) == 0)
	{
		inRetVal = inBATCH_ReviewReport_Detail_Txno_NEWUI_By_Sqlite(pobTran);
	} else
	{
		inRetVal = inBATCH_ReviewReport_Detail_NEWUI_By_Sqlite(pobTran);
	}

	if (inRetVal != VS_SUCCESS)
	{
		inBATCH_CheckReport_By_Sqlite(pobTran);
	}

	return (inRetVal);
}

/*
Function        :inBATCH_ReviewReport_Detail_NEWUI_By_Sqlite_NEWUI
Date&Time       :2017/3/14 下午 5:48
Describe        :交易查詢使用
 */
int inBATCH_ReviewReport_Detail_NEWUI_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int inLen = 0;
	int inESVCIndex = -1;
	int inRetVal = VS_SUCCESS;
	int inNowindex = 0;
	int inFileRecCnt = -1;
	int inTouchPage = 0;
	char szTemplate1[42 + 1] = {0};
	char szTemplate2[42 + 1] = {0};
	char szFinalPAN[_PAN_UCARD_SIZE_ + 1] = {0};
	char szDispMsg [50 + 1] = {0};
	char szDebugMsg[100 + 1] = {0};
	char szKey = 0x00;
	char szTRTFileName[16 + 1] = {0};
	char szFuncEnable[2 + 1] = {0};
	unsigned char uszBatchEndBit = VS_FALSE;
	unsigned char uszBatchCancelBit = VS_FALSE;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_ReviewReport_Detail_NEWUI_By_Sqlite_NEWUI()_START");
	}

	/* 算出交易總筆數 */
	inFileRecCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "筆數: %d", inFileRecCnt);
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 沒交易紀錄 */
	if (inFileRecCnt == 0)
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第一層顯示 <交易查詢> */
		return (VS_NO_RECORD);
	}

	/* [新增電票悠遊卡功能] 電票功能取代此條件 [SAM] 2022/6/22 上午 11:05 */
	//	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_ESVC_, &inESVCIndex);

	/* [新增電票悠遊卡功能] 新增顯示明細報表功能條件 [SAM] 2022/6/22 上午 11:05 */
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CMAS_, &inESVCIndex);
		inDISP_LogPrintfWithFlag(" Batch Review ESVCIndex[%d] Br_HostId[%d] Line[%d] ", inESVCIndex, pobTran->srBRec.inHDTIndex, __LINE__);
	}


	/* 先抓出總資料 */
	if (pobTran->srBRec.inHDTIndex == inESVCIndex)
	{
		inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_START(pobTran);
	} else
	{
		inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);
	}

	inDISP_ClearAll();
	inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0, _COORDINATE_Y_LINE_8_1_); /* 第一層顯示 <交易查詢> */
	inDISP_PutGraphic(_REVIEW_DETAIL_, 0, _COORDINATE_Y_LINE_8_8_); /* 上一筆 下一筆 */

	while (1)
	{
		if (inNowindex >= inFileRecCnt)
		{
			uszBatchEndBit = VS_TRUE;
		}

		if (uszBatchEndBit == VS_TRUE ||
			uszBatchCancelBit == VS_TRUE)
		{
			break;
		}

		/* 開始讀取每一筆交易記錄 */
		if (pobTran->srBRec.inHDTIndex == inESVCIndex)
		{
			if (inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_Read(pobTran, inNowindex) != VS_SUCCESS)
			{
				uszBatchEndBit = VS_TRUE;
				break;
			}
		} else
		{
			if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, inNowindex) != VS_SUCCESS)
			{
				uszBatchEndBit = VS_TRUE;
				break;
			}
		}

		inDISP_Clear_Line(_LINE_8_2_, _LINE_8_7_);



		inGetTRTFileName(szTRTFileName);
#ifdef _ESUN_MAIN_HOST_

		inDISP_LogPrintfWithFlag("  NEWUI_By_Sqlite uszVOIDBit[%d]  inOrgCode[%d ] Line[%d] ", pobTran->srBRec.uszVOIDBit, pobTran->srBRec.inOrgCode, __LINE__);

		/* 玉山自助規格，如有做過AUTH COMP，原AUTH不可顯示 2021/5/19 上午 11:14 [SAM] */
		if (pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode == _PRE_AUTH_)
		{
			if (inTouchPage == _DisTouch_Slide_Right_To_Left_ || szKey == _KEY_ENTER_ || szKey == _KEY_DOWN_)
			{
				inNowindex++;
				continue;
			} else if (inTouchPage == _DisTouch_Slide_Left_To_Right_ || szKey == _KEY_UP_)
			{
				inNowindex--;

				/* 交易紀錄編號不可小於0 */
				if (inNowindex < 0)
				{
					inNowindex = 0;
					/* 加這行是因為，若第一筆是DCC轉台幣會陷入無限迴圈，所以改滑動方向 */
					inTouchPage = _DisTouch_Slide_Right_To_Left_;
					szKey = _KEY_ENTER_;
				}
				continue;
			}				/* 若第一筆就是要跳過的紀錄，會因為沒有event，而沒有跳過，所以預設為下一頁 */
			else
			{
				inNowindex++;
				continue;
			}

		}
#else                
		/* DCC轉台幣的紀錄要特別處理，原先的那筆DCC一般交易不能出現 */
		if (pobTran->srBRec.inCode == _VOID_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)
		{
			if (inTouchPage == _DisTouch_Slide_Right_To_Left_ || szKey == _KEY_ENTER_ || szKey == _KEY_DOWN_)
			{
				inNowindex++;
				continue;
			} else if (inTouchPage == _DisTouch_Slide_Left_To_Right_ || szKey == _KEY_UP_)
			{
				inNowindex--;

				/* 交易紀錄編號不可小於0 */
				if (inNowindex < 0)
				{
					inNowindex = 0;
					/* 加這行是因為，若第一筆是DCC轉台幣會陷入無限迴圈，所以改滑動方向 */
					inTouchPage = _DisTouch_Slide_Right_To_Left_;
					szKey = _KEY_ENTER_;
				}
				continue;
			}				/* 若第一筆就是要跳過的紀錄，會因為沒有event，而沒有跳過，所以預設為下一頁 */
			else
			{
				inNowindex++;
				continue;
			}

		}
#endif
		/* 優惠兌換的顯示比較特別 */
		if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_ ||
				pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
		{
			/* Invoice Number */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_)
			{
				sprintf(szDispMsg, "優惠兌換     %03ld", pobTran->srBRec.lnOrgInvNum);
			} else if (pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
			{
				sprintf(szDispMsg, "兌換取消     %03ld", pobTran->srBRec.lnOrgInvNum);
			}
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_2_, _DISP_LEFT_);

			/* Data & Time */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, "%.2s/%.2s/%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_3_, _DISP_LEFT_);

			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, "%.2s:%.2s:%.2s", &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2], &pobTran->srBRec.szTime[4]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
		} else if (pobTran->srBRec.inHDTIndex == inESVCIndex)
		{
			/* Disp Card Label & Expire date*/
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
			{
				strcpy(szDispMsg, " 一卡通");
			} else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
			{
				strcpy(szDispMsg, " 悠遊卡");
			}

			inDISP_ChineseFont_Point_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_2_, _COLOR_BLACK_, _COLOR_WHITE_, 1);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "CardLabel: %s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Disp Card Number */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			memset(szFinalPAN, 0x00, sizeof (szFinalPAN));
			/* 卡號值 */
			if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
			{
				memset(szFinalPAN, 0x00, sizeof (szFinalPAN));
				inLen = strlen(pobTran->srTRec.szUID);
				memcpy(szFinalPAN, pobTran->srTRec.szUID, inLen);
				szFinalPAN[inLen - 1] = 0x2A;
				szFinalPAN[inLen - 2] = 0x2A;
			} else
			{
				memset(szFinalPAN, 0x00, sizeof (szFinalPAN));
				inLen = strlen(pobTran->srTRec.szUID);
				memcpy(szFinalPAN, pobTran->srTRec.szUID, inLen);
			}

			sprintf(szDispMsg, " %s", szFinalPAN);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "szUID: %s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Transaction Type */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			memset(szTemplate1, 0x00, sizeof (szTemplate1));
			if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_)
			{
				/* [新增電票悠遊卡功能] 修改顯示條件，新增自動加值項目 [SAM] 2022/6/22 上午 11:08 */
				if (pobTran->srTRec.lnTotalTopUpAmount > 0)
				{
					strcpy(szTemplate1, "購貨、自動加值");
				} else
					strcpy(szTemplate1, "購貨　　");

				//				strcpy(szTemplate1, "購貨　　");
			} else if (pobTran->srTRec.inCode == _TICKET_IPASS_REFUND_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_)
				strcpy(szTemplate1, "退貨　　");
			else if (pobTran->srTRec.inCode == _TICKET_IPASS_TOP_UP_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_)
				strcpy(szTemplate1, "現金加值");
			else if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_)
				strcpy(szTemplate1, "加值取消");
			else if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_)
				strcpy(szTemplate1, "餘額查詢");
			else if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_AUTO_TOP_UP_)
				strcpy(szTemplate1, "自動加值");
			else
				strcpy(szTemplate1, "　　");

			sprintf(szDispMsg, " %s", szTemplate1);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Tran Type: %s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Data & Time */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			/* [新增電票悠遊卡功能] 因日期格式有進行調整所以替代以下原有顯示格式 [SAM] 2022/6/22 上午 11:10
			 *  sprintf(szDispMsg, " 20%.2s/%.2s/%.2s %.2s:%.2s", &pobTran->srTRec.szDate[0], &pobTran->srTRec.szDate[2], &pobTran->srTRec.szDate[4], &pobTran->srTRec.szTime[0], &pobTran->srTRec.szTime[2]);
			 *  */
			sprintf(szDispMsg, " %.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srTRec.szDate[0], &pobTran->srTRec.szDate[4], &pobTran->srTRec.szDate[6], &pobTran->srTRec.szTime[0], &pobTran->srTRec.szTime[2]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_9_, _DISP_LEFT_);

			/* RF序號 */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, " RF序號: %s", pobTran->srTRec.szTicketRefundCode);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_10_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Reference Number */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			/* [新增電票悠遊卡功能] 新增電票條件  [SAM] 2022/6/22 上午 11:16 */
			if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
			{
				sprintf(szDispMsg, " RRN: %s", pobTran->srTRec.srECCRec.szRRN);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_11_, _DISP_LEFT_);
			} else
			{
				sprintf(szDispMsg, " REF: %s", pobTran->srTRec.szRefNo);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_11_, _DISP_LEFT_);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Invoice Number */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, " INV: %06ld", pobTran->srTRec.lnInvNum);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_12_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Amount */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			switch (pobTran->srTRec.inCode)
			{
				case _TICKET_IPASS_DEDUCT_:
				case _TICKET_EASYCARD_DEDUCT_:
				case _TICKET_IPASS_REFUND_:
				case _TICKET_EASYCARD_REFUND_:
				case _TICKET_IPASS_TOP_UP_:
				case _TICKET_EASYCARD_TOP_UP_:
				case _TICKET_IPASS_INQUIRY_:
				case _TICKET_IPASS_AUTO_TOP_UP_:
				case _TICKET_EASYCARD_AUTO_TOP_UP_:
					sprintf(szTemplate1, "%ld", pobTran->srTRec.lnTxnAmount);
					inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
					sprintf(szDispMsg, " %s", szTemplate1);
					break;
					/* [新增電票悠遊卡功能] 調整 _TICKET_IPASS_VOID_TOP_UP_ 及  _TICKET_EASYCARD_VOID_TOP_UP_ 
					 * 顯示方式   [SAM] 2022/6/22 上午 11:13 */
				case _TICKET_IPASS_VOID_TOP_UP_:
				case _TICKET_EASYCARD_VOID_TOP_UP_: /*20200417 Added[Hachi]*/
					sprintf(szTemplate1, "%ld", (0 - pobTran->srTRec.lnTxnAmount));
					inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
					sprintf(szDispMsg, " %s", szTemplate1);
					break;
					/*20191231 Added by Hachi end*/
				default:
					sprintf(szDispMsg, " AMT_VOID_ERR_(%d)", pobTran->srTRec.inCode);
					break;
			} /* End switch () */
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_13_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Store ID */
			memset(szTemplate1, 0x00, sizeof (szTemplate1));
			memset(szDispMsg, 0x00, sizeof (szDispMsg));

			inGetStoreIDEnable(&szTemplate1[0]);

			if ((szTemplate1[0] == 'Y') && (strlen(pobTran->srTRec.szStoreID) > 0))
			{
				if (strlen(pobTran->srTRec.szStoreID) > 42)
					memcpy(&szTemplate1[0], &pobTran->srTRec.szStoreID[0], 42);
				else
					strcpy(szTemplate1, pobTran->srTRec.szStoreID);

				szTemplate1[18] = 0x00;

				sprintf(szDispMsg, " STD:%s", pobTran->srTRec.szStoreID);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_14_, _DISP_LEFT_);
			} else
			{
				/* 沒開櫃號功能， 不印櫃號 */
			}
		} else
		{
			/* Disp Card Label & Expire date */
			/* Disp Card Label */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, " %s", pobTran->srBRec.szCardLabel);
			inDISP_ChineseFont_Point_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_2_, _COLOR_BLACK_, _COLOR_WHITE_, 1);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "CardLabel: %s", pobTran->srBRec.szCardLabel);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Expire date */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			memset(szTemplate1, 0x00, sizeof (szTemplate1));
			/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
			/* 金融卡沒有效期 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{

			}/* 其他加密 */
			else
			{
				memset(szTemplate1, 0x00, sizeof (szTemplate1));
				inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
				sprintf(szDispMsg, "%s", szTemplate1);
			}
			inDISP_ChineseFont_Point_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_2_, _COLOR_BLACK_, _COLOR_WHITE_, 13);


			/* Disp Card Number */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			memset(szFinalPAN, 0x00, sizeof (szFinalPAN));
			strcpy(szFinalPAN, pobTran->srBRec.szPAN);

			if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
			{
				inFunc_FormatPAN_UCARD(szFinalPAN);
			}
			sprintf(szDispMsg, " %s", szFinalPAN);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "szPAN: %s", pobTran->srBRec.szPAN);
				inDISP_LogPrintf(szDebugMsg);
			}


			/* Transaction Type */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			memset(szTemplate1, 0x00, sizeof (szTemplate1));
			memset(szTemplate2, 0x00, sizeof (szTemplate2));
			inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
			if (strlen(szTemplate2) > 0)
			{
				sprintf(szDispMsg, " %s", szTemplate1);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_7_, _DISP_LEFT_);
				memset(szDispMsg, 0x00, sizeof (szDispMsg));
				sprintf(szDispMsg, " %s", szTemplate2);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_8_, _DISP_LEFT_);
			} else
			{
				sprintf(szDispMsg, " %s %s", szTemplate1, szTemplate2);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
			}


			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Tran Type: %s %s", szTemplate1, szTemplate2);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Data & Time */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, " 20%.2s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_9_, _DISP_LEFT_);

			/* 授權碼(金卡顯示調單編號) */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				sprintf(szDispMsg, " 交易序號: %s", pobTran->srBRec.szFiscRRN);
			} else
			{
				sprintf(szDispMsg, " 授權碼: %s", pobTran->srBRec.szAuthCode);
			}
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_10_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Reference Number */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, " 序號: %s", pobTran->srBRec.szRefNo);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_11_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Invoice Number */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, " 調閱編號: %06ld", pobTran->srBRec.lnOrgInvNum);

			/* 【需求單 - 105259】總額明細報表及總額明細查詢補強機制 商店聯出紙本要印<P> */
			memset(szFuncEnable, 0x00, sizeof (szFuncEnable));
			inGetESCMode(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0 && pobTran->srBRec.inESCUploadStatus == _ESC_UPLOAD_STATUS_PAPER_)
			{
				strcat(szDispMsg, " <P>");
			}

			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_12_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* 顯示是否有優惠資訊 */
			//			if (inNCCC_Loyalty_ASM_Flag() != VS_SUCCESS)
			//			{
			//				memset(szDispMsg, 0x00, sizeof(szDispMsg));
			//				sprintf(szDispMsg, " ");
			//				inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X44_, _LINE_16_12_, _DISP_RIGHT_);
			//			}
			//			else
			{
				/* 原交易有優惠資訊，取消交易後，非小費及暫停優惠服務，則列印取消優惠資訊 */
				if (pobTran->srBRec.uszVOIDBit == VS_TRUE &&
					(pobTran->srBRec.uszRewardL1Bit == VS_TRUE ||
					pobTran->srBRec.uszRewardL2Bit == VS_TRUE ||
					pobTran->srBRec.uszRewardL5Bit == VS_TRUE) &&
					atoi(pobTran->srBRec.szAwardNum) > 0 &&
					pobTran->srBRec.uszRewardSuspendBit != VS_TRUE)
				{
					memset(szDispMsg, 0x00, sizeof (szDispMsg));
					sprintf(szDispMsg, "VOID AWD ");
					inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X44_, _LINE_16_12_, _DISP_RIGHT_);
				} else if (pobTran->srBRec.uszVOIDBit != VS_TRUE &&
						(pobTran->srBRec.uszRewardL1Bit == VS_TRUE ||
						pobTran->srBRec.uszRewardL2Bit == VS_TRUE ||
						pobTran->srBRec.uszRewardL5Bit == VS_TRUE))
				{
					memset(szDispMsg, 0x00, sizeof (szDispMsg));
					sprintf(szDispMsg, "AWARD ");
					inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X44_, _LINE_16_12_, _DISP_RIGHT_);
				} else
				{
					memset(szDispMsg, 0x00, sizeof (szDispMsg));
					sprintf(szDispMsg, " ");
					inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X44_, _LINE_16_12_, _DISP_RIGHT_);
				}

			}

			/* Amount */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));

			if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
			{
				switch (pobTran->srBRec.inCode)
				{
					case _SALE_:
					case _INST_SALE_:
					case _REDEEM_SALE_:
					case _SALE_OFFLINE_:
					case _FORCE_CASH_ADVANCE_:
					case _PRE_COMP_:
					case _PRE_AUTH_:
					case _CUP_SALE_:
					case _CUP_PRE_COMP_:
					case _CUP_PRE_AUTH_:
					case _INST_ADJUST_:
					case _REDEEM_ADJUST_:
					case _MAIL_ORDER_:
					case _CUP_MAIL_ORDER_:
					case _CASH_ADVANCE_:
					case _FISC_SALE_:
						if (pobTran->srBRec.lnTipTxnAmount != 0)
							sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						else
							sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "金額 NT$ ", ' ', _SIGNED_NONE_, 22, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _ADJUST_:
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "金額 NT$ ", ' ', _SIGNED_NONE_, 22, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _TIP_:
						sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "金額 NT$ ", ' ', _SIGNED_NONE_, 22, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _REFUND_:
					case _INST_REFUND_:
					case _REDEEM_REFUND_:
					case _CUP_REFUND_:
					case _CUP_MAIL_ORDER_REFUND_:
					case _FISC_REFUND_:
						sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "金額 NT$ ", ' ', _SIGNED_NONE_, 22, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					default:
						sprintf(szDispMsg, " AMT_ERR_(%d)", pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			} else
			{
				switch (pobTran->srBRec.inOrgCode)
				{
					case _SALE_:
					case _INST_SALE_:
					case _REDEEM_SALE_:
					case _SALE_OFFLINE_:
					case _FORCE_CASH_ADVANCE_:
					case _PRE_COMP_:
					case _PRE_AUTH_:
					case _CUP_SALE_:
					case _CUP_PRE_COMP_:
					case _CUP_PRE_AUTH_:
					case _INST_ADJUST_:
					case _REDEEM_ADJUST_:
					case _MAIL_ORDER_:
					case _CUP_MAIL_ORDER_:
					case _CASH_ADVANCE_:
					case _FISC_SALE_:
						sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "金額 NT$ ", ' ', _SIGNED_NONE_, 22, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _ADJUST_:
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "金額 NT$ ", ' ', _SIGNED_NONE_, 22, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
						/* 目前小費沒有取消 */
					case _REFUND_:
					case _INST_REFUND_:
					case _REDEEM_REFUND_:
					case _CUP_REFUND_:
					case _CUP_MAIL_ORDER_REFUND_:
						/* SmartPay不能取消 */
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "金額 NT$ ", ' ', _SIGNED_NONE_, 22, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					default:
						sprintf(szDispMsg, " AMT_VOID_ERR_(%d)", pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_13_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Store ID */
			memset(szTemplate1, 0x00, sizeof (szTemplate1));
			memset(szDispMsg, 0x00, sizeof (szDispMsg));

			inGetStoreIDEnable(&szTemplate1[0]);

			if ((szTemplate1[0] == 'Y') && (strlen(pobTran->srBRec.szStoreID) > 0))
			{
				if (strlen(pobTran->srBRec.szStoreID) > 42)
					memcpy(&szTemplate1[0], &pobTran->srBRec.szStoreID[0], 42);
				else
					strcpy(szTemplate1, pobTran->srBRec.szStoreID);

				szTemplate1[18] = 0x00;

				sprintf(szDispMsg, " 櫃號:%s", pobTran->srBRec.szStoreID);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_14_, _DISP_LEFT_);
			} else
			{
				/* 沒開櫃號功能， 不印櫃號 */
			}
		}

		/* 事件初始化 */
		szKey = 0;
		/* 設定Timeout */
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);

		while (1)
		{

			/* 偵測滑動 */
			inTouchPage = inDisTouch_TouchSensor_Click_Slide(_Touch_NEWUI_REVIEW_BATCH_);

			/* 偵測按鍵 */
			szKey = uszKBD_Key();

			/* TimeOut */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_SUCCESS;
				uszBatchCancelBit = VS_TRUE;
				break;
			} else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_SUCCESS;
				uszBatchCancelBit = VS_TRUE;
				break;
			} else if (inTouchPage == _DisTouch_Slide_Right_To_Left_ ||
					inTouchPage == _NEWUI_REVIEW_DETAIL_Touch_NEXT_DATA_BUTTON_ ||
					szKey == _KEY_ENTER_ ||
					szKey == _KEY_DOWN_)
			{
				++inNowindex;
				break;
			} else if (inTouchPage == _DisTouch_Slide_Left_To_Right_ ||
					inTouchPage == _NEWUI_REVIEW_DETAIL_Touch_LAST_DATA_BUTTON_ ||
					szKey == _KEY_UP_ ||
					szKey == _KEY_FUNCTION_)
			{
				--inNowindex;
				/* 交易紀錄編號不可小於0 */
				if (inNowindex < 0)
				{
					inNowindex = 0;
				}
				break;
			}

		}

	} /* End for () .... */

	if (pobTran->srBRec.inHDTIndex == inESVCIndex)
	{
		inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_END(pobTran);
	} else
	{
		inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);
	}

	if (uszBatchEndBit == VS_TRUE)
	{
		/* 顯示批次結束 */
		inDISP_ClearAll();
		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第一層顯示 <交易查詢> */
		/* 清下排 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_ERR_BATCH_END_, 0, _COORDINATE_Y_LINE_8_6_);
		inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);
		inDISP_BEEP(1, 0);

		/* 事件初始化 */
		szKey = 0;
		/* 設定Timeout */
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);

		while (1)
		{
			/* 偵測滑動、觸控 */
			inTouchPage = inDisTouch_TouchSensor_Click_Slide(_Touch_BATCH_END_);
			/* 按鍵 */
			szKey = uszKBD_Key();

			/* TimeOut */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (inTouchPage == _BATCH_END_Touch_ENTER_BUTTON_ ||
					szKey == _KEY_ENTER_)
			{
				break;
			} else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				break;
			} else
			{
				continue;
			}
		}
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_ReviewReport_Detail_NEWUI_By_Sqlite_NEWUI()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inBATCH_ReviewReport_Detail_Txno_NEWUI_By_Sqlite
Date&Time       :2017/6/12 下午 5:54
Describe        :交易查詢使用，for卡號遮掩
 */
int inBATCH_ReviewReport_Detail_Txno_NEWUI_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int inLen = 0;
	int inESVCIndex = -1;
	int inRetVal = VS_SUCCESS;
	int i = 0;
	int inNowindex = 0;
	int inFileRecCnt = -1;
	int inTouchPage = 0;
	char szTemplate1[42 + 1] = {0};
	char szTemplate2[42 + 1] = {0};
	char szFinalPAN[_PAN_UCARD_SIZE_ + 1] = {0};
	char szDispMsg [50 + 1] = {0};
	char szDebugMsg[100 + 1] = {0};
	char szKey = 0x00;
	char szTRTFileName[16 + 1] = {0};
	char szFuncEnable[2 + 1] = {0};
	unsigned char uszBatchEndBit = VS_FALSE;
	unsigned char uszBatchCancelBit = VS_FALSE;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_ReviewReport_Detail_Txno_NEWUI_By_Sqlite()_START");
	}

	/* 算出交易總筆數 */
	inFileRecCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "筆數: %d", inFileRecCnt);
		inDISP_LogPrintf(szDebugMsg);
	}

	/* 沒交易紀錄 */
	if (inFileRecCnt == 0)
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第一層顯示 <交易查詢> */
		return (VS_NO_RECORD);
	}

	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_ESVC_, &inESVCIndex);

	/* 先抓出總資料 */
	if (pobTran->srBRec.inHDTIndex == inESVCIndex)
	{
		inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_START(pobTran);
	} else
	{
		inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);
	}

	inDISP_ClearAll();
	inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0, _COORDINATE_Y_LINE_8_1_); /* 第一層顯示 <交易查詢> */
	inDISP_PutGraphic(_REVIEW_DETAIL_, 0, _COORDINATE_Y_LINE_8_8_); /* 上一筆 下一筆 */

	while (1)
	{
		if (inNowindex >= inFileRecCnt)
		{
			uszBatchEndBit = VS_TRUE;
		}

		if (uszBatchEndBit == VS_TRUE ||
				uszBatchCancelBit == VS_TRUE)
		{
			break;
		}

		/* 開始讀取每一筆交易記錄 */
		if (pobTran->srBRec.inHDTIndex == inESVCIndex)
		{
			if (inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_Read(pobTran, inNowindex) != VS_SUCCESS)
			{
				uszBatchEndBit = VS_TRUE;
				break;
			}
		} else
		{
			if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, inNowindex) != VS_SUCCESS)
			{
				uszBatchEndBit = VS_TRUE;
				break;
			}
		}

		inDISP_Clear_Line(_LINE_8_2_, _LINE_8_7_);

		inGetTRTFileName(szTRTFileName);

		/* DCC轉台幣的紀錄要特別處理，原先的那筆DCC一般交易不能出現 */
		if (pobTran->srBRec.inCode == _VOID_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)
		{
			if (inTouchPage == _DisTouch_Slide_Right_To_Left_ || szKey == _KEY_ENTER_ || szKey == _KEY_DOWN_)
			{
				inNowindex++;
				continue;
			} else if (inTouchPage == _DisTouch_Slide_Left_To_Right_ || szKey == _KEY_UP_)
			{
				inNowindex--;

				/* 交易紀錄編號不可小於0 */
				if (i < 0)
				{
					i = 0;
					/* 加這行是因為，若第一筆是DCC轉台幣會陷入無限迴圈，所以改滑動方向 */
					inTouchPage = _DisTouch_Slide_Right_To_Left_;
					szKey = _KEY_ENTER_;
				}
				continue;
			}				/* 若第一筆就是要跳過的紀錄，會因為沒有event，而沒有跳過，所以預設為下一頁 */
			else
			{
				inNowindex++;
				continue;
			}

		}

		/* 優惠兌換的顯示比較特別 */
		if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_ ||
				pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
		{
			/* Invoice Number */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_)
			{
				sprintf(szDispMsg, "優惠兌換     %03ld", pobTran->srBRec.lnOrgInvNum);
			} else if (pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
			{
				sprintf(szDispMsg, "兌換取消     %03ld", pobTran->srBRec.lnOrgInvNum);
			}
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_1_, _DISP_LEFT_);

			/* Data & Time */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, "%.2s/%.2s/%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_2_, _DISP_LEFT_);

			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, "%.2s:%.2s:%.2s", &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2], &pobTran->srBRec.szTime[4]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_3_, _DISP_LEFT_);
		} else if (pobTran->srBRec.inHDTIndex == inESVCIndex)
		{
			/* Disp Card Label & Expire date*/
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
			{
				strcpy(szDispMsg, " 一卡通");
			} else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
			{
				strcpy(szDispMsg, " 悠遊卡");
			}

			inDISP_ChineseFont_Point_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_2_, _COLOR_BLACK_, _COLOR_WHITE_, 1);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "CardLabel: %s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Disp Card Number */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			memset(szFinalPAN, 0x00, sizeof (szFinalPAN));
			/* 卡號值 */
			if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
			{
				memset(szFinalPAN, 0x00, sizeof (szFinalPAN));
				inLen = strlen(pobTran->srTRec.szUID);
				memcpy(szFinalPAN, pobTran->srTRec.szUID, inLen);
				szFinalPAN[inLen - 1] = 0x2A;
				szFinalPAN[inLen - 2] = 0x2A;
			} else
			{
				memset(szFinalPAN, 0x00, sizeof (szFinalPAN));
				inLen = strlen(pobTran->srTRec.szUID);
				memcpy(szFinalPAN, pobTran->srTRec.szUID, inLen);
			}

			sprintf(szDispMsg, " %s", szFinalPAN);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "szUID: %s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Transaction Type */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			memset(szTemplate1, 0x00, sizeof (szTemplate1));
			if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_)
				strcpy(szTemplate1, "購貨　　");
			else if (pobTran->srTRec.inCode == _TICKET_IPASS_REFUND_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_)
				strcpy(szTemplate1, "退貨　　");
			else if (pobTran->srTRec.inCode == _TICKET_IPASS_TOP_UP_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_)
				strcpy(szTemplate1, "現金加值");
			else if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_)
				strcpy(szTemplate1, "加值取消");
			else if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_)
				strcpy(szTemplate1, "餘額查詢");
			else if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_AUTO_TOP_UP_)
				strcpy(szTemplate1, "自動加值");
			else
				strcpy(szTemplate1, "　　");

			sprintf(szDispMsg, " %s", szTemplate1);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Tran Type: %s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Data & Time */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, " 20%.2s/%.2s/%.2s %.2s:%.2s", &pobTran->srTRec.szDate[0], &pobTran->srTRec.szDate[2], &pobTran->srTRec.szDate[4], &pobTran->srTRec.szTime[0], &pobTran->srTRec.szTime[2]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_9_, _DISP_LEFT_);

			/* RF序號 */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, " RF序號: %s", pobTran->srTRec.szTicketRefundCode);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_10_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Reference Number */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, " REF: %s", pobTran->srTRec.szRefNo);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_11_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Invoice Number */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, " INV: %06ld", pobTran->srTRec.lnInvNum);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_12_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Amount */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			switch (pobTran->srTRec.inCode)
			{
				case _TICKET_IPASS_DEDUCT_:
				case _TICKET_EASYCARD_DEDUCT_:
				case _TICKET_IPASS_REFUND_:
				case _TICKET_EASYCARD_REFUND_:
				case _TICKET_IPASS_TOP_UP_:
				case _TICKET_EASYCARD_TOP_UP_:
				case _TICKET_IPASS_VOID_TOP_UP_:
				case _TICKET_EASYCARD_VOID_TOP_UP_:
				case _TICKET_IPASS_INQUIRY_:
				case _TICKET_IPASS_AUTO_TOP_UP_:
				case _TICKET_EASYCARD_AUTO_TOP_UP_:
					sprintf(szTemplate1, "%ld", pobTran->srTRec.lnTxnAmount);
					inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
					sprintf(szDispMsg, " %s", szTemplate1);
					break;
				default:
					sprintf(szDispMsg, " AMT_VOID_ERR_(%d)", pobTran->srTRec.inCode);
					break;
			} /* End switch () */
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_13_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Store ID */
			memset(szTemplate1, 0x00, sizeof (szTemplate1));
			memset(szDispMsg, 0x00, sizeof (szDispMsg));

			inGetStoreIDEnable(&szTemplate1[0]);

			if ((szTemplate1[0] == 'Y') && (strlen(pobTran->srTRec.szStoreID) > 0))
			{
				if (strlen(pobTran->srTRec.szStoreID) > 42)
					memcpy(&szTemplate1[0], &pobTran->srTRec.szStoreID[0], 42);
				else
					strcpy(szTemplate1, pobTran->srTRec.szStoreID);

				szTemplate1[18] = 0x00;

				sprintf(szDispMsg, " STD:%s", pobTran->srTRec.szStoreID);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_14_, _DISP_LEFT_);
			} else
			{
				/* 沒開櫃號功能， 不印櫃號 */
			}
		} else
		{
			/* Disp Card Label & Expire date */
			/* Disp Card Label */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, " %s", pobTran->srBRec.szCardLabel);
			inDISP_ChineseFont_Point_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_2_, _COLOR_BLACK_, _COLOR_WHITE_, 1);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "CardLabel: %s", pobTran->srBRec.szCardLabel);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Expire date */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			memset(szTemplate1, 0x00, sizeof (szTemplate1));
			/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
			/* 金融卡沒有效期 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{

			}				/* 其他加密 */
			else
			{
				memset(szTemplate1, 0x00, sizeof (szTemplate1));
				inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
				sprintf(szDispMsg, "%s", szTemplate1);
			}
			inDISP_ChineseFont_Point_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_2_, _COLOR_BLACK_, _COLOR_WHITE_, 13);

			/* Disp Card Number */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			memset(szFinalPAN, 0x00, sizeof (szFinalPAN));
			strcpy(szFinalPAN, pobTran->srBRec.szPAN);

			if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
			{
				inFunc_FormatPAN_UCARD(szFinalPAN);
			}

			/* 卡號遮掩 */
			/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
			if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
			{
				for (i = 3; i < (strlen(szFinalPAN) - 5); i++)
					szFinalPAN[i] = 0x2A;
			} else
			{
				for (i = 6; i < (strlen(szFinalPAN) - 4); i++)
					szFinalPAN[i] = 0x2A;
			}

			sprintf(szDispMsg, " %s", szFinalPAN);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "szPAN: %s", pobTran->srBRec.szPAN);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Transaction Type */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			memset(szTemplate1, 0x00, sizeof (szTemplate1));
			memset(szTemplate2, 0x00, sizeof (szTemplate2));
			inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
			if (strlen(szTemplate2) > 0)
			{
				sprintf(szDispMsg, " %s", szTemplate1);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_7_, _DISP_LEFT_);
				memset(szDispMsg, 0x00, sizeof (szDispMsg));
				sprintf(szDispMsg, " %s", szTemplate2);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_8_, _DISP_LEFT_);
			} else
			{
				sprintf(szDispMsg, " %s %s", szTemplate1, szTemplate2);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
			}


			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Tran Type: %s %s", szTemplate1, szTemplate2);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Data & Time */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, " 20%.2s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_9_, _DISP_LEFT_);

			/* 授權碼(金卡顯示調單編號) */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				sprintf(szDispMsg, " 交易序號: %s", pobTran->srBRec.szFiscRRN);
			} else
			{
				sprintf(szDispMsg, " 授權碼: %s", pobTran->srBRec.szAuthCode);
			}
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_10_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Reference Number */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, " 序號: %s", pobTran->srBRec.szRefNo);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_11_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Invoice Number */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));
			sprintf(szDispMsg, " 調閱編號: %06ld", pobTran->srBRec.lnOrgInvNum);

			/* 【需求單 - 105259】總額明細報表及總額明細查詢補強機制 商店聯出紙本要印<P> */
			memset(szFuncEnable, 0x00, sizeof (szFuncEnable));
			inGetESCMode(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0 && pobTran->srBRec.inESCUploadStatus == _ESC_UPLOAD_STATUS_PAPER_)
			{
				strcat(szDispMsg, " <P>");
			}

			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_12_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* 顯示是否有優惠資訊 */
			//			if (inNCCC_Loyalty_ASM_Flag() != VS_SUCCESS)
			if (1)
			{
				memset(szDispMsg, 0x00, sizeof (szDispMsg));
				sprintf(szDispMsg, " ");
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X44_, _LINE_16_12_, _DISP_RIGHT_);
			} else
			{
				/* 原交易有優惠資訊，取消交易後，非小費及暫停優惠服務，則列印取消優惠資訊 */
				if (pobTran->srBRec.uszVOIDBit == VS_TRUE &&
						(pobTran->srBRec.uszRewardL1Bit == VS_TRUE ||
						pobTran->srBRec.uszRewardL2Bit == VS_TRUE ||
						pobTran->srBRec.uszRewardL5Bit == VS_TRUE) &&
						atoi(pobTran->srBRec.szAwardNum) > 0 &&
						pobTran->srBRec.uszRewardSuspendBit != VS_TRUE)
				{
					memset(szDispMsg, 0x00, sizeof (szDispMsg));
					sprintf(szDispMsg, "VOID AWD ");
					inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X44_, _LINE_16_12_, _DISP_RIGHT_);
				} else if (pobTran->srBRec.uszVOIDBit != VS_TRUE &&
						(pobTran->srBRec.uszRewardL1Bit == VS_TRUE ||
						pobTran->srBRec.uszRewardL2Bit == VS_TRUE ||
						pobTran->srBRec.uszRewardL5Bit == VS_TRUE))
				{
					memset(szDispMsg, 0x00, sizeof (szDispMsg));
					sprintf(szDispMsg, "AWARD ");
					inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X44_, _LINE_16_12_, _DISP_RIGHT_);
				} else
				{
					memset(szDispMsg, 0x00, sizeof (szDispMsg));
					sprintf(szDispMsg, " ");
					inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X44_, _LINE_16_12_, _DISP_RIGHT_);
				}

			}

			/* Amount */
			memset(szDispMsg, 0x00, sizeof (szDispMsg));

			if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
			{
				switch (pobTran->srBRec.inCode)
				{
					case _SALE_:
					case _INST_SALE_:
					case _REDEEM_SALE_:
					case _SALE_OFFLINE_:
					case _FORCE_CASH_ADVANCE_:
					case _PRE_COMP_:
					case _PRE_AUTH_:
					case _CUP_SALE_:
					case _CUP_PRE_COMP_:
					case _CUP_PRE_AUTH_:
					case _INST_ADJUST_:
					case _REDEEM_ADJUST_:
					case _MAIL_ORDER_:
					case _CUP_MAIL_ORDER_:
					case _CASH_ADVANCE_:
					case _FISC_SALE_:
						if (pobTran->srBRec.lnTipTxnAmount != 0)
							sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						else
							sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "金額 NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _ADJUST_:
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "金額 NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _TIP_:
						sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "金額 NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _REFUND_:
					case _INST_REFUND_:
					case _REDEEM_REFUND_:
					case _CUP_REFUND_:
					case _CUP_MAIL_ORDER_REFUND_:
					case _FISC_REFUND_:
						sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "金額 NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					default:
						sprintf(szDispMsg, " AMT_ERR_(%d)", pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			} else
			{
				switch (pobTran->srBRec.inOrgCode)
				{
					case _SALE_:
					case _INST_SALE_:
					case _REDEEM_SALE_:
					case _SALE_OFFLINE_:
					case _FORCE_CASH_ADVANCE_:
					case _PRE_COMP_:
					case _PRE_AUTH_:
					case _CUP_SALE_:
					case _CUP_PRE_COMP_:
					case _CUP_PRE_AUTH_:
					case _INST_ADJUST_:
					case _REDEEM_ADJUST_:
					case _MAIL_ORDER_:
					case _CUP_MAIL_ORDER_:
					case _CASH_ADVANCE_:
					case _FISC_SALE_:
						sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "金額 NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _ADJUST_:
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "金額 NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
						/* 目前小費沒有取消 */
					case _REFUND_:
					case _INST_REFUND_:
					case _REDEEM_REFUND_:
					case _CUP_REFUND_:
					case _CUP_MAIL_ORDER_REFUND_:
						/* SmartPay不能取消 */
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "金額 NT$ ", ' ', _SIGNED_NONE_, 17, _PAD_LEFT_FILL_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					default:
						sprintf(szDispMsg, " AMT_VOID_ERR_(%d)", pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_13_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* Store ID */
			memset(szTemplate1, 0x00, sizeof (szTemplate1));
			memset(szDispMsg, 0x00, sizeof (szDispMsg));

			inGetStoreIDEnable(&szTemplate1[0]);

			if ((szTemplate1[0] == 'Y') && (strlen(pobTran->srBRec.szStoreID) > 0))
			{
				if (strlen(pobTran->srBRec.szStoreID) > 42)
					memcpy(&szTemplate1[0], &pobTran->srBRec.szStoreID[0], 42);
				else
					strcpy(szTemplate1, pobTran->srBRec.szStoreID);

				szTemplate1[18] = 0x00;

				sprintf(szDispMsg, " 櫃號:%s", pobTran->srBRec.szStoreID);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_14_, _DISP_LEFT_);
			} else
			{
				/* 沒開櫃號功能， 不印櫃號 */
			}
		}

		/* 事件初始化 */
		szKey = 0;
		/* 設定Timeout */
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);

		while (1)
		{

			/* 偵測滑動 */
			inTouchPage = inDisTouch_TouchSensor_Click_Slide(_Touch_NEWUI_REVIEW_BATCH_);
			if (inTouchPage == _DisTouch_Slide_Right_To_Left_)
			{
				++inNowindex;
				break;
			}
			if (inTouchPage == _DisTouch_Slide_Left_To_Right_)
			{
				--inNowindex;

				/* 交易紀錄編號不可小於0 */
				if (inNowindex < 0)
				{
					inNowindex = 0;
				}
				break;
			}

			/* 偵測按鍵 */
			szKey = uszKBD_Key();

			/* TimeOut */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_SUCCESS;
				uszBatchCancelBit = VS_TRUE;
				break;
			} else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_SUCCESS;
				uszBatchCancelBit = VS_TRUE;
				break;
			} else if (inTouchPage == _DisTouch_Slide_Right_To_Left_ ||
					inTouchPage == _NEWUI_REVIEW_DETAIL_Touch_NEXT_DATA_BUTTON_ ||
					szKey == _KEY_ENTER_ ||
					szKey == _KEY_DOWN_)
			{
				++inNowindex;
				break;
			} else if (inTouchPage == _DisTouch_Slide_Left_To_Right_ ||
					inTouchPage == _NEWUI_REVIEW_DETAIL_Touch_LAST_DATA_BUTTON_ ||
					szKey == _KEY_UP_ ||
					szKey == _KEY_FUNCTION_)
			{
				--inNowindex;
				/* 交易紀錄編號不可小於0 */
				if (inNowindex < 0)
				{
					inNowindex = 0;
				}
				break;
			}
		}

	} /* End for () .... */

	/* 開始讀取每一筆交易記錄 */
	if (pobTran->srBRec.inHDTIndex == inESVCIndex)
	{
		inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_END(pobTran);
	} else
	{
		inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);
	}


	if (uszBatchEndBit == VS_TRUE)
	{
		/* 顯示批次結束 */
		inDISP_ClearAll();
		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第一層顯示 <交易查詢> */
		/* 清下排 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_ERR_BATCH_END_, 0, _COORDINATE_Y_LINE_8_6_);
		inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);
		inDISP_BEEP(1, 0);

		/* 事件初始化 */
		szKey = 0;
		/* 設定Timeout */
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);

		while (1)
		{
			/* 偵測滑動、觸控 */
			inTouchPage = inDisTouch_TouchSensor_Click_Slide(_Touch_BATCH_END_);
			/* 按鍵 */
			szKey = uszKBD_Key();

			/* TimeOut */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (inTouchPage == _BATCH_END_Touch_ENTER_BUTTON_ ||
					szKey == _KEY_ENTER_)
			{
				break;
			} else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				break;
			} else
			{
				continue;
			}
		}
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_ReviewReport_Detail_Txno_NEWUI_By_Sqlite()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GetDetailRecords_By_Sqlite
Date&Time       :2017/3/15 下午 3:20
Describe        :抓資料庫中uszUpdate為0的資料
 */
int inBATCH_GetDetailRecords_By_Sqlite(TRANSACTION_OBJECT *pobTran, int inStartCnt)
{
	int inRetVal;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GetDetailRecords_By_Sqlite()_START");
	}


	inRetVal = inSqlite_Get_Batch_ByCnt_Flow(pobTran, _TN_BATCH_TABLE_, inStartCnt);
	if (inRetVal != VS_SUCCESS)
		return (inRetVal);

	if (pobTran->srBRec.inChipStatus != 0 ||
			pobTran->srBRec.uszContactlessBit == VS_TRUE ||
			pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
	{
		inRetVal = inSqlite_Get_Batch_ByCnt_Flow(pobTran, _TN_EMV_TABLE_, pobTran->srBRec.lnOrgInvNum);
		if (inRetVal != VS_SUCCESS)
			return (inRetVal);
	}


	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GetDetailRecords_By_Sqlite()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inBATCH_Get_Batch_ByCnt_Enormous_Flow
Date&Time       :2017/3/21 下午 1:18
Describe        :查詢大量紀錄使用，最後一定要call free
 */
int inBATCH_Get_Batch_ByCnt_Enormous_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inRecCnt, int inState)
{
	int inRetVal;
	char szTableName[50 + 1]; /* 若傳進的TableName1為空字串，則用szTableName組TableName */

	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof (szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		default:
			break;
	}

	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
			if (inState == _BYCNT_ENORMMOUS_SEARCH_)
			{
				inRetVal = inSqlite_Get_Batch_ByCnt_Enormous_Search(pobTran, szTableName);
			} else if (inState == _BYCNT_ENORMMOUS_READ_)
			{
				inRetVal = inSqlite_Get_Batch_ByCnt_Enormous_Get(pobTran, szTableName, inRecCnt);
			} else if (inState == _BYCNT_ENORMMOUS_FREE_)
			{
				inRetVal = inSqlite_Get_Batch_ByCnt_Enormous_Free();
			} else
			{
				return (VS_ERROR);
			}
			break;
		case _TN_BATCH_TABLE_TICKET_:
			if (inState == _BYCNT_ENORMMOUS_SEARCH_)
			{
				/* [新增電票悠遊卡功能] 重新啟用電票功能 [SAM] 2022/6/14 下午 12:06 */
				inRetVal = inNCCC_Ticket_Get_TRec_ByCnt_Enormous_Search(pobTran, szTableName);
			} else if (inState == _BYCNT_ENORMMOUS_READ_)
			{
				/* [新增電票悠遊卡功能] 重新啟用電票功能 [SAM] 2022/6/14 下午 12:06 */
				inRetVal = inNCCC_Ticket_Get_TRec_ByCnt_Enormous_Get(pobTran, szTableName, inRecCnt);
			} else if (inState == _BYCNT_ENORMMOUS_FREE_)
			{
				inRetVal = inSqlite_Get_Batch_ByCnt_Enormous_Free();
			} else
			{
				return (VS_ERROR);
			}
			break;
		default:
			return (VS_ERROR);
			break;
	}

	return (inRetVal);
}

/*
Function        :inBATCH_GetDetailRecords_By_Sqlite_Enormous_START
Date&Time       :2017/3/21 下午 5:16
Describe        :抓資料庫中uszUpdate為0的資料
 */
int inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GetDetailRecords_By_Sqlite_Enormous_START()_START");
	}

	inRetVal = inBATCH_Get_Batch_ByCnt_Enormous_Flow(pobTran, _TN_BATCH_TABLE_, 0, _BYCNT_ENORMMOUS_SEARCH_);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GetDetailRecords_By_Sqlite_Enormous_START()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read
Date&Time       :2017/3/21 下午 5:16
Describe        :抓資料庫中uszUpdate為0的資料
 */
int inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(TRANSACTION_OBJECT *pobTran, int inStartCnt)
{
	int inRetVal;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read()_START");
	}

	inRetVal = inBATCH_Get_Batch_ByCnt_Enormous_Flow(pobTran, _TN_BATCH_TABLE_, inStartCnt, _BYCNT_ENORMMOUS_READ_);
	if (inRetVal != VS_SUCCESS)
		return (inRetVal);

	if (pobTran->srBRec.inChipStatus != 0 ||
		pobTran->srBRec.uszContactlessBit == VS_TRUE ||
		pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
	{
		inRetVal = inSqlite_Get_Batch_ByCnt_Flow(pobTran, _TN_EMV_TABLE_, 0);
		if (inRetVal != VS_SUCCESS)
			return (inRetVal);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inBATCH_GetDetailRecords_By_Sqlite_Enormous_END
Date&Time       :2017/3/21 下午 5:16
Describe        :抓資料庫中uszUpdate為0的資料
 */
int inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GetDetailRecords_By_Sqlite_Enormous_END()_START");
	}

	inRetVal = inBATCH_Get_Batch_ByCnt_Enormous_Flow(pobTran, _TN_BATCH_TABLE_, 0, _BYCNT_ENORMMOUS_FREE_);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GetDetailRecords_By_Sqlite_Enormous_END()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_START
Date&Time       :2017/3/21 下午 5:16
Describe        :抓資料庫中uszUpdate為0的資料
 */
int inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_START(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_START()_START");
	}

	inRetVal = inBATCH_Get_Batch_ByCnt_Enormous_Flow(pobTran, _TN_BATCH_TABLE_TICKET_, 0, _BYCNT_ENORMMOUS_SEARCH_);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_START()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_Read
Date&Time       :2017/3/21 下午 5:16
Describe        :抓資料庫中uszUpdate為0的資料
 */
int inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_Read(TRANSACTION_OBJECT *pobTran, int inStartCnt)
{
	int inRetVal;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_Read()_START");
	}

	inRetVal = inBATCH_Get_Batch_ByCnt_Enormous_Flow(pobTran, _TN_BATCH_TABLE_TICKET_, inStartCnt, _BYCNT_ENORMMOUS_READ_);
	if (inRetVal != VS_SUCCESS)
		return (inRetVal);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_Read()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_END
Date&Time       :2017/3/21 下午 5:16
Describe        :抓資料庫中uszUpdate為0的資料
 */
int inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_END(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_END()_START");
	}

	inRetVal = inBATCH_Get_Batch_ByCnt_Enormous_Flow(pobTran, _TN_BATCH_TABLE_TICKET_, 0, _BYCNT_ENORMMOUS_FREE_);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_END()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inBATCH_FuncUserChoice_By_Sqlite_ESVC
Date&Time       :2017/3/15 下午 4:48
Describe        :
 */
int inBATCH_FuncUserChoice_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{

	int inRetVal;
	int inCMASIndex = -1; /* [新增電票悠遊卡功能] 新增電票尋找資料的條件 [SAM] 2022/6/22 上午 10:51 */
#ifdef __NCCC_DCC_FUNC__	
	int inDCCIndex = -1;
	int inNCCCIndex = -1;
	int inESVCIndex = -1;
	int inOrgIndex = -1;
#endif
	char szDate[8 + 1], szTime[6 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	if (pobTran->inRunOperationID == _OPERATION_TIP_ || pobTran->inRunOperationID == _OPERATION_ADJUST_)
	{
		memset(szDate, 0x00, sizeof (szDate));
		memset(szTime, 0x00, sizeof (szTime));
		memcpy(szDate, pobTran->srBRec.szDate, sizeof (pobTran->srBRec.szDate));
		memcpy(szTime, pobTran->srBRec.szTime, sizeof (pobTran->srBRec.szTime));

		inDISP_LogPrintfWithFlag("szDate + szTime 1,%s,%s", pobTran->srBRec.szDate, pobTran->srBRec.szTime);
	}

	/* NCCC DCC的特殊處理，當端末機收到收銀機要取消DCC HOST交易的電文時，
	 * 如果在DCC的帳務中查不到這一筆交易(表示交易改為台幣支付)端末機就自動在去找NCCC的帳務來解決銀行別錯誤而取消不到的問題。 START
	 */
#ifdef __NCCC_DCC_FUNC__
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_DCC_, &inDCCIndex);
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_MAINLY_USE_, &inNCCCIndex);
#endif	
#ifdef __NCCC_ESVC_FUNC__
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_ESVC_, &inESVCIndex);
#endif

	/* [新增電票悠遊卡功能] 新增電票尋找資料的條件 [SAM] 2022/6/22 上午 10:51 */
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CMAS_, &inCMASIndex);

	/* NCCC DCC的特殊處理，當端末機收到收銀機要取消DCC HOST交易的電文時，
	 * 如果在DCC的帳務中查不到這一筆交易(表示交易改為台幣支付)端末機就自動在去找NCCC的帳務來解決銀行別錯誤而取消不到的問題。 END
	 */

#ifdef __NCCC_ESVC_FUNC__
	if (pobTran->srBRec.inHDTIndex == inESVCIndex)
	{
		while (1)
		{
			/* 如果lnOrgInvNum有值，就不需要做輸入的動作(ECR發動) */
			if (pobTran->srBRec.lnOrgInvNum <= 0)
			{
				if (inBATCH_GetInvoiceNumber(pobTran) == VS_ERROR)
				{
					return (VS_ERROR);
				}

				/* 因為EDC如果按確認鍵，等於調出上一筆記錄 */
				if (pobTran->srBRec.lnOrgInvNum == 0)
					pobTran->srBRec.lnOrgInvNum = _BATCH_LAST_RECORD_; /* 表示是最後一筆 */
			}

			/* Load Batch Record */
			inRetVal = inBATCH_GetTransRecord_By_Sqlite_ESVC(pobTran);

			if (inRetVal == VS_SUCCESS)
			{
				break;

			}				/* 找不到 */
			else if (inRetVal == VS_NO_RECORD)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				/* 顯示無交易紀錄 */
				inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

				inDISP_BEEP(3, 500);
				pobTran->srBRec.lnOrgInvNum = -1;

				continue;
			} else
			{
				return (VS_ERROR);
			}
		}
	}
#endif
#ifdef __NCCC_DCC_FUNC__
		/* DCC有特殊處理 */
	else if (pobTran->srBRec.inHDTIndex == inDCCIndex)
	{
		while (1)
		{
			/* 如果lnOrgInvNum有值，就不需要做輸入的動作(ECR發動) */
			if (pobTran->srBRec.lnOrgInvNum <= 0)
			{
				if (inBATCH_GetInvoiceNumber(pobTran) == VS_ERROR)
				{
					return (VS_ERROR);
				}

				/* 因為EDC如果按確認鍵，等於調出上一筆記錄 */
				if (pobTran->srBRec.lnOrgInvNum == 0)
					pobTran->srBRec.lnOrgInvNum = _BATCH_LAST_RECORD_; /* 表示是最後一筆 */
			}

			/* Load Batch Record */
			inRetVal = inBATCH_GetTransRecord_By_Sqlite(pobTran);

			if (inRetVal == VS_SUCCESS)
			{
				/* NCCC DCC的特殊處理，當端末機收到收銀機要取消DCC HOST交易的電文時，
				 * 如果在DCC的帳務中查不到這一筆交易(表示交易改為台幣支付)端末機就自動在去找NCCC的帳務來解決銀行別錯誤而取消不到的問題。 START
				 */
				if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)
				{
					/* 這個情形只有在用POS機時才會出現 */
					if (pobTran->uszECRBit == VS_TRUE)
					{
						inOrgIndex = pobTran->srBRec.inHDTIndex;
						pobTran->srBRec.inHDTIndex = inNCCCIndex;
						/* Load Batch Record */
						inRetVal = inBATCH_GetTransRecord_By_Sqlite(pobTran);

						if (inRetVal == VS_SUCCESS)
						{
							break;
						} else if (inRetVal == VS_NO_RECORD)
						{
							/* 找不到，還原index並顯示無交易紀錄*/
							pobTran->srBRec.inHDTIndex = inOrgIndex;

							inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
							/* 顯示無交易紀錄 */
							inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

							inDISP_BEEP(3, 500);
							pobTran->srBRec.lnOrgInvNum = -1;

							continue;
						} else
						{
							return (VS_ERROR);
						}
						/* NCCC DCC的特殊處理，當端末機收到收銀機要取消DCC HOST交易的電文時，
						 * 如果在DCC的帳務中查不到這一筆交易(表示交易改為台幣支付)端末機就自動在去找NCCC的帳務來解決銀行別錯誤而取消不到的問題。 END
						 */
					}						/* 如果是單機應該直接從NCCC帳裡撈 */
					else
					{
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						/* 顯示無交易紀錄 */
						inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

						inDISP_BEEP(3, 500);
						pobTran->srBRec.lnOrgInvNum = -1;

						continue;
					}

				}					/* 找到紀錄 */
				else
				{
					break;
				}

			}				/* 找不到 */
			else if (inRetVal == VS_NO_RECORD)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				/* 顯示無交易紀錄 */
				inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

				inDISP_BEEP(3, 500);
				pobTran->srBRec.lnOrgInvNum = -1;

				continue;
			} else
			{
				return (VS_ERROR);
			}

		}
	} else
#endif
	//if (pobTran->srBRec.inHDTIndex == inIPASSIndex || pobTran->srBRec.inHDTIndex == inCMASIndex) 
	if (pobTran->srBRec.inHDTIndex == inCMASIndex)
	{ /* [新增電票悠遊卡功能] 新增電票搜尋條件 [SAM] 2022/6/22 上午 10:55 */
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintfAt(AT, " function is %s ,Line is %d", __FUNCTION__, __LINE__);
		}
		while (1)
		{
			/*20190722 added by Hachi for IPASS 2.0 reprint start 增加輸入InvNum*/
			/* 如果lnOrgInvNum有值，就不需要做輸入的動作(ECR發動) */
			if (pobTran->srBRec.lnOrgInvNum <= 0)
			{
				if (inBATCH_GetInvoiceNumber(pobTran) == VS_ERROR)
				{
					return (VS_ERROR);
				}
				/*20190722 added by Hachi for IPASS 2.0 reprint end*/
				if (pobTran->srBRec.lnOrgInvNum == 0)
				{
					pobTran->srBRec.lnOrgInvNum = _BATCH_LAST_RECORD_; /* 表示是最後一筆 */
				}
			} //20190722 added by Hachi

			/* Load Batch Record */
			inRetVal = inBATCH_GetTransRecord_By_Sqlite_ESVC(pobTran);

			if (inRetVal == VS_SUCCESS)
			{
				break;
			}				/* 找不到 */
			else if (inRetVal == VS_NO_RECORD)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				/* 顯示無交易紀錄 */
				inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

				inDISP_BEEP(3, 500);
				pobTran->srBRec.lnOrgInvNum = -1;

				continue;
			} else
			{
				return (VS_ERROR);
			}
		}

	} else
	{
		while (1)
		{
			/* 如果lnOrgInvNum有值，就不需要做輸入的動作(ECR發動) */
			if (pobTran->srBRec.lnOrgInvNum <= 0)
			{
				/* 如果使用ECR交易找不到資料就回傳錯誤，自助不能使用按鍵 2021/5/12 上午 10:12 [SAM]  */
#if (_MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_) && (defined(_ESUN_MAIN_HOST_))
				if (pobTran->uszKioskFlag == 'Y')
				{
					return (VS_ERROR);
				} else
				{
					if (inBATCH_GetInvoiceNumber(pobTran) == VS_ERROR)
					{
						return (VS_ERROR);
					}
				}
#else                

				if (inBATCH_GetInvoiceNumber(pobTran) == VS_ERROR)
				{
					return (VS_ERROR);
				}
#endif
				/* 因為EDC如果按確認鍵，等於調出上一筆記錄 */
				if (pobTran->srBRec.lnOrgInvNum == 0)
					pobTran->srBRec.lnOrgInvNum = _BATCH_LAST_RECORD_; /* 表示是最後一筆 */
			}

			/* Load Batch Record */
			inRetVal = inBATCH_GetTransRecord_By_Sqlite(pobTran);

			if (inRetVal == VS_SUCCESS)
			{
				break;

			}				/* 找不到 */
			else if (inRetVal == VS_NO_RECORD)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				/* 顯示無交易紀錄 */
				inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

				inDISP_BEEP(3, 500);
				pobTran->srBRec.lnOrgInvNum = -1;
				/* 如果使用ECR交易找不到資料就回傳錯誤，自助不能使用按鍵 2021/5/12 上午 10:12 [SAM]  */
#if (_MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_) && (defined(_ESUN_MAIN_HOST_))
				if (pobTran->uszKioskFlag == 'Y')
				{
					return (VS_ERROR);
				} else
				{
					continue;
				}
#else                

				continue;
#endif                
			} else
			{
				return (VS_ERROR);
			}
		}
	}

	/* 簽名狀態要重置，取消、小費這種讀批次的會紀錄到上一次的狀態 */
	/* 是否輸入Pin狀態重置 */
	/* 手動輸入調閱編號的情況中，只有重印是要保留原狀態 */
	if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
	{

	} else
	{
		pobTran->srBRec.inESCUploadMode = _ESC_STATUS_NONE_;
		pobTran->srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_UPLOAD_NOT_YET_;
		pobTran->srBRec.inSignStatus = _SIGN_NONE_;
		pobTran->srBRec.uszPinEnterBit = VS_FALSE;
	}

	if (pobTran->inRunOperationID == _OPERATION_TIP_ || pobTran->inRunOperationID == _OPERATION_ADJUST_)
	{
		memcpy(pobTran->srBRec.szDate, szDate, sizeof (pobTran->srBRec.szDate));
		memcpy(pobTran->srBRec.szTime, szTime, sizeof (pobTran->srBRec.szTime));

		inDISP_LogPrintfWithFlag("szDate + szTime 2,%s,%s", pobTran->srBRec.szDate, pobTran->srBRec.szTime);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GetTransRecord_By_Sqlite
Date&Time       :2017/3/15 下午 4:57
Describe        :讀出pobTran->srBRec.lnOrgInvNum的紀錄
 */
int inBATCH_GetTransRecord_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GetTransRecord_By_Sqlite()_START");
	}

	inRetVal = inBATCH_Get_Batch_ByInvNum_Flow(pobTran, _TN_BATCH_TABLE_, pobTran->srBRec.lnOrgInvNum);
	if (inRetVal != VS_SUCCESS)
		return (inRetVal);

	if (pobTran->srBRec.inChipStatus != 0 ||
		pobTran->srBRec.uszContactlessBit == VS_TRUE ||
		pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
	{
		inRetVal = inBATCH_Get_Batch_ByInvNum_Flow(pobTran, _TN_EMV_TABLE_, pobTran->srBRec.lnOrgInvNum);
		if (inRetVal != VS_SUCCESS)
			return (inRetVal);
	}

	/* 儲存原STAN，離線交易軟加要用原STAN */
	pobTran->srBRec.lnOrgSTANNum = pobTran->srBRec.lnSTANNum;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GetTransRecord_By_Sqlite()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inBATCH_GetTransRecord_By_Sqlite_ESVC
Date&Time       :2018/1/30 下午 6:30
Describe        :讀出pobTran->srBRec.lnOrgInvNum的紀錄
 */
int inBATCH_GetTransRecord_By_Sqlite_ESVC(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GetTransRecord_By_Sqlite_ESVC()_START");
	}

	inRetVal = inBATCH_Get_Batch_ByInvNum_Flow(pobTran, _TN_BATCH_TABLE_TICKET_, pobTran->srBRec.lnOrgInvNum);
	if (inRetVal != VS_SUCCESS)
		return (inRetVal);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GetTransRecord_By_Sqlite_ESVC()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inBATCH_GlobalBatchHandleReadOnly_By_Sqlite
Date&Time       :2017/3/15 下午 5:20
Describe        :
 */
int inBATCH_GlobalBatchHandleReadOnly_By_Sqlite(void)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GlobalBatchHandleReadOnly_By_Sqlite()_START");
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GlobalBatchHandleReadOnly_By_Sqlite()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GlobalBatchHandleClose_By_Sqlite
Date&Time       :2017/3/15 下午 5:20
Describe        :
 */
int inBATCH_GlobalBatchHandleClose_By_Sqlite(void)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GlobalBatchHandleClose_By_Sqlite()_START");
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GlobalBatchHandleClose_By_Sqlite()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GlobalAdviceHandleReadOnly_By_Sqlite
Date&Time       :2017/3/15 下午 5:19
Describe        :
 */
int inBATCH_GlobalAdviceHandleReadOnly_By_Sqlite(void)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GlobalAdviceHandleReadOnly_By_Sqlite()_START");
	}

	if (inFunc_ComposeFileName_ForGlobal((char*) guszADVFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 為了避免重複開Handle造成當機，先將handle值設為default(0x00)(因為用globalHandle無法確保是否已關，重複開會造成錯誤) */
	gulADVHandle = VS_HANDLE_NULL;

	if (inFILE_OpenReadOnly(&gulADVHandle, guszADVFileName) == VS_ERROR)
	{
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GlobalAdviceHandleReadOnly_By_Sqlite()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GlobalAdviceHandleClose_By_Sqlite
Date&Time       :2017/3/15 下午 5:18
Describe        :
 */
int inBATCH_GlobalAdviceHandleClose_By_Sqlite(void)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GlobalAdviceHandleClose_By_Sqlite()_START");
	}

	if (gulADVHandle != VS_HANDLE_NULL)
	{
		inFILE_Close(&gulADVHandle);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GlobalAdviceHandleClose_By_Sqlite()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GetAdviceDetailRecord_By_Sqlite
Date&Time       :2017/3/15 下午 5:14
Describe        :
 */
int inBATCH_GetAdviceDetailRecord_By_Sqlite(TRANSACTION_OBJECT *pobTran, int inADVCnt)
{
	int inRetVal;
	unsigned char uszGetAdviceInvoiceNum[6 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GetAdviceDetailRecord_By_Sqlite()_START");
	}

	/* 讀【.adv】檔 */
	if (inFILE_Seek(gulADVHandle, 0, _SEEK_BEGIN_) == VS_ERROR)
	{
		return (VS_NO_RECORD);
	} else
	{
		memset(uszGetAdviceInvoiceNum, 0x00, sizeof (uszGetAdviceInvoiceNum));
		if ((inRetVal = inFILE_Read(&gulADVHandle, &uszGetAdviceInvoiceNum[0], _ADV_FIELD_SIZE_)) == VS_ERROR)
		{
			return (VS_NO_RECORD);
		} else
		{
			pobTran->srBRec.lnOrgInvNum = atol((char *) uszGetAdviceInvoiceNum);
		}
	}

	inRetVal = inBATCH_GetTransRecord_By_Sqlite(pobTran);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GetAdviceDetailRecord_By_Sqlite()_END,%s", uszGetAdviceInvoiceNum);
		inDISP_LogPrintf("----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inBATCH_GlobalAdvice_ESC_HandleReadOnly_By_Sqlite
Date&Time       :2017/3/15 下午 5:19
Describe        :
 */
int inBATCH_GlobalAdvice_ESC_HandleReadOnly_By_Sqlite(void)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GlobalAdvice_ESC_HandleReadOnly_By_Sqlite()_START");
	}

	if (inFunc_ComposeFileName_ForGlobal((char*) guszADVFileName, _ADVICE_ESC_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 為了避免重複開Handle造成當機，先將handle值設為default(0x00)(因為用globalHandle無法確保是否已關，重複開會造成錯誤) */
	gulADVHandle = VS_HANDLE_NULL;

	if (inFILE_OpenReadOnly(&gulADVHandle, guszADVFileName) == VS_ERROR)
	{
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GlobalAdvice_ESC_HandleReadOnly_By_Sqlite()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GlobalAdvice_ESC_HandleClose_By_Sqlite
Date&Time       :2017/3/15 下午 5:18
Describe        :
 */
int inBATCH_GlobalAdvice_ESC_HandleClose_By_Sqlite(void)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GlobalAdvice_ESC_HandleClose_By_Sqlite()_START");
	}

	if (gulADVHandle != VS_HANDLE_NULL)
	{
		inFILE_Close(&gulADVHandle);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GlobalAdvice_ESC_HandleClose_By_Sqlite()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GetAdvice_ESC_DetailRecord_By_Sqlite
Date&Time       :2017/3/29 上午 11:18
Describe        :目前和inBATCH_GetAdvice_DetailRecord_By_Sqlite完全相同，但為了保險所以也分開
 */
int inBATCH_GetAdvice_ESC_DetailRecord_By_Sqlite(TRANSACTION_OBJECT *pobTran, int inADVCnt)
{
	int inRetVal;
	unsigned char uszGetAdviceInvoiceNum[6 + 1];

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GetAdvice_ESC_DetailRecord_By_Sqlite()_START");
	}

	/* 讀【.adv】檔 */
	if (inFILE_Seek(gulADVHandle, 0, _SEEK_BEGIN_) == VS_ERROR)
	{
		return (VS_ERROR);
	} else
	{
		memset(uszGetAdviceInvoiceNum, 0x00, sizeof (uszGetAdviceInvoiceNum));
		if ((inRetVal = inFILE_Read(&gulADVHandle, &uszGetAdviceInvoiceNum[0], _ADV_FIELD_SIZE_)) == VS_ERROR)
		{
			return (VS_ERROR);
		} else
		{
			pobTran->srBRec.lnOrgInvNum = atol((char *) uszGetAdviceInvoiceNum);
		}
	}

	inRetVal = inBATCH_GetTransRecord_By_Sqlite(pobTran);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GetAdvice_ESC_DetailRecord_By_Sqlite()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inBATCH_ESC_Save_Advice_By_Sqlite
Date&Time       :2017/3/15 下午 5:15
Describe        :存ESC advice
 */
int inBATCH_ESC_Save_Advice_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_ESC_Save_Advice_By_Sqlite() START !");
	}

	inADVICE_ESC_SaveAppend(pobTran, pobTran->srBRec.lnOrgInvNum);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_ESC_Save_Advice_By_Sqlite() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_Update_Sign_Status_By_Sqlite
Date&Time       :2017/3/22 下午 3:46
Describe        :更新簽名狀態
 */
int inBATCH_Update_Sign_Status_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_Update_Sign_Status_By_Sqlite() START !");
	}

	pobTran->uszUpdateBatchBit = VS_TRUE;
	inBATCH_FuncUpdateTxnRecord_By_Sqlite(pobTran);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_Update_Sign_Status_By_Sqlite() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_Update_MPAS_Reprint_By_Sqlite
Date&Time       :2017/9/5 下午 4:34
Describe        :更新MPAS重印狀態
 */
int inBATCH_Update_MPAS_Reprint_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_Update_MPAS_Reprint_By_Sqlite() START !");
	}

	pobTran->uszUpdateBatchBit = VS_TRUE;
	inBATCH_FuncUpdateTxnRecord_By_Sqlite(pobTran);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_Update_MPAS_Reprint_By_Sqlite() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_Update_CLS_SettleBit_By_Sqlite
Date&Time       :2017/4/11 上午 11:15
Describe        :
 */
int inBATCH_Update_CLS_SettleBit_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_Update_CLS_SettleBit_By_Sqlite() START !");
	}

	inSqlite_Update_ByInvNum_CLS_SettleBit_Flow(pobTran, _TN_BATCH_TABLE_, pobTran->srBRec.lnOrgInvNum);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_Update_CLS_SettleBit_By_Sqlite() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_Update_ESC_Uploaded_By_Sqlite
Date&Time       :2018/4/27 下午 5:49
Describe        :更新ESC上傳狀態
 */
int inBATCH_Update_ESC_Uploaded_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_Update_ESC_Uploaded_By_Sqlite() START !");
	}

	pobTran->uszUpdateBatchBit = VS_TRUE;
	inBATCH_FuncUpdateTxnRecord_By_Sqlite(pobTran);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_Update_ESC_Uploaded_By_Sqlite() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GetTotalCount_BatchUpload_By_Sqlite
Date&Time       :2017/4/11 下午 3:13
Describe        :取得BatchUpload有效筆數，以避免最後一筆為取消時沒有改processing code
 */
int inBATCH_GetTotalCount_BatchUpload_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;
	int inCnt = -1;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GetTotalCount_BatchUpload_By_Sqlite()_START");
	}

	inRetVal = inSqlite_Get_Batch_Count_Flow(pobTran, _TN_BATCH_TABLE_, &inCnt);
	if (inRetVal == VS_NO_RECORD)
	{
		return (VS_NO_RECORD);
	} else if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GetTotalCountFromBakFile_By_Sqlite()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (inCnt);
}

/*
Function        :inBATCH_FuncUpdateTxnRecord_Ticket_By_Sqlite
Date&Time       :2018/1/12 下午 4:29
Describe        :insert資料到資料庫
 */
int inBATCH_FuncUpdateTxnRecord_Ticket_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_FuncUpdateTxnRecord_Ticket_By_Sqlite()_START");
	}

	/* 如果是update，不用再試圖創Table */
	if (pobTran->uszUpdateBatchBit == VS_TRUE)
	{
		inRetVal = inSqlite_Check_Table_Exist_Flow(pobTran, _TN_BATCH_TABLE_TICKET_);
		/* 如果要update卻沒有table，直接跳出 */
		if (inRetVal != VS_SUCCESS)
		{
			inFunc_EDCLock();
		}
	} else
	{
		inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_TICKET_);
		if (inRetVal != VS_SUCCESS)
		{
			inFunc_EDCLock();
		}
	}

	/* 如果uszUpdate設為1 */
	if (pobTran->uszUpdateBatchBit == VS_TRUE)
	{
		inSqlite_Update_ByInvNum_TranState_Flow(pobTran, _TN_BATCH_TABLE_TICKET_, pobTran->srTRec.lnInvNum);
	}

	/* Insert批次 */
	inRetVal = inBATCH_Insert_All_Flow(pobTran, _TN_BATCH_TABLE_TICKET_);
	if (inRetVal != VS_SUCCESS)
	{
		inFunc_EDCLock();
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_FuncUpdateTxnRecord_Ticket_By_Sqlite()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_Create_BatchTable_Flow
Date&Time       :2017/3/14 下午 3:00
Describe        :在這邊決定名稱並分流
 */
int inBATCH_Create_BatchTable_Flow(TRANSACTION_OBJECT *pobTran, int inTableType)
{
	int inRetVal = VS_SUCCESS;
	char szTableName1[50 + 1]; /* 若傳進的TableName1為空字串，則用szTableName組TableName，理論上Table Name不限制長度 */
	char szTableName2[50 + 1]; /* 用來設定外鍵 */

	/* 由function決定TableName */
	memset(szTableName1, 0x00, sizeof (szTableName1));
	inFunc_ComposeFileName(pobTran, szTableName1, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName1, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			memset(szTableName2, 0x00, sizeof (szTableName2));
			memcpy(szTableName2, szTableName1, strlen(szTableName1));
			strcat(szTableName1, _EMV_TABLE_SUFFIX_);
			strcat(szTableName2, _BATCH_TABLE_SUFFIX_);
			break;

			/* ESC */
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName1, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;

		case _TN_BATCH_TABLE_ESC_AGAIN_EMV_:/* 新增ESC EMV記錄 20190919 [SAM] */
			memset(szTableName2, 0x00, sizeof (szTableName2));
			memcpy(szTableName2, szTableName1, strlen(szTableName1));
			strcat(szTableName1, _BATCH_TABLE_ESC_AGAIN_EMV_SUFFIX_);
			strcat(szTableName2, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;

		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName1, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;

		case _TN_BATCH_TABLE_ESC_FAIL_EMV_:/* 新增ESC EMV記錄 20190919 [SAM] */
			memset(szTableName2, 0x00, sizeof (szTableName2));
			memcpy(szTableName2, szTableName1, strlen(szTableName1));
			strcat(szTableName1, _BATCH_TABLE_ESC_FAIL_EMV_SUFFIX_);
			strcat(szTableName2, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
			/* 票證 */
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName1, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			strcat(szTableName1, _BATCH_TABLE_ADVICE_SUFFIX_);
			break;
		default:
			break;
	}

	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
			inRetVal = inSqlite_Create_BatchTable(pobTran, szTableName1);
			break;
		case _TN_EMV_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_EMV_:/* 新增ESC EMV記錄 20190919 [SAM] */
		case _TN_BATCH_TABLE_ESC_FAIL_EMV_:/* 新增ESC EMV記錄 20190919 [SAM] */
			inRetVal = inSqlite_Create_BatchTable_EMV(pobTran, szTableName1, szTableName2);
			break;
		case _TN_BATCH_TABLE_TICKET_:
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			/* [新增電票悠遊卡功能] 重新啟用電票功能 [SAM] 2022/6/14 上午 11:07 */
			inRetVal = inNCCC_Ticket_CreateBatchTable_Ticket(pobTran, szTableName1);
			break;
		default:
			return (VS_ERROR);
			break;
	}

	return (inRetVal);
}

/*
Function        :inBATCH_Get_Batch_ByInvNum_Flow
Date&Time       :2018/1/31 下午 1:58
Describe        :
 */
int inBATCH_Get_Batch_ByInvNum_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inInvoiceNumber)
{
	int inRetVal;
	char szTableName[50 + 1]; /* 若傳進的TableName1為空字串，則用szTableName組TableName */

	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof (szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);

	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		default:
			break;
	}

	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
			inRetVal = inSqlite_Get_Batch_ByInvNum(pobTran, szTableName, inInvoiceNumber);
			break;
		case _TN_EMV_TABLE_:
			inRetVal = inSqlite_Get_EMV_ByInvNum(pobTran, szTableName);
			break;
		case _TN_BATCH_TABLE_TICKET_:
			/* [新增電票悠遊卡功能] 重新啟用電票功能 [SAM] 2022/6/14 下午 12:06 */
			inRetVal = inNCCC_Ticket_ESVC_Get_Batch_ByInvNum(pobTran, szTableName, inInvoiceNumber);
			break;
		default:
			return (VS_ERROR);
			break;
	}

	return (inRetVal);
}

/*
Function        :inBATCH_Insert_All_Flow
Date&Time       :2017/3/14 下午 3:00
Describe        :在這邊決定名稱並分流
 */
int inBATCH_Insert_All_Flow(TRANSACTION_OBJECT *pobTran, int inTableType)
{
	int inRetVal;
	char szTableName[50 + 1]; /* 若傳進的TableName1為空字串，則用szTableName組TableName */

	inFunc_CalculateRunTimeGlobal_Start();

	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof (szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
			/* ESC */
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_EMV_:/* 新增ESC EMV記錄 20190919 [SAM] */
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_EMV_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_EMV_:/* 新增ESC EMV記錄 20190919 [SAM] */
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_EMV_SUFFIX_);
			break;
			/* 票證 */
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ADVICE_SUFFIX_);
			break;

		default:
			break;
	}

	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
			inRetVal = inSqlite_Insert_All_Batch(pobTran, szTableName);
			break;
		case _TN_EMV_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_EMV_:/* 新增ESC EMV記錄 20190919 [SAM] */
		case _TN_BATCH_TABLE_ESC_FAIL_EMV_:/* 新增ESC EMV記錄 20190919 [SAM] */
			inRetVal = inSqlite_Insert_All_EMV(pobTran, szTableName);
			break;
		case _TN_BATCH_TABLE_TICKET_:
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			/* [新增電票悠遊卡功能] 重新啟用電票功能 [SAM] 2022/6/14 下午 12:06 */
			inRetVal = inNCCC_Ticket_Insert_All_Batch(pobTran, szTableName);
			break;
		default:
			return (VS_ERROR);
			break;
	}

	inFunc_WatchRunTime();

	return (inRetVal);
}

/*
Function        :inBATCH_Get_ESC_Upload_Count_Flow
Date&Time       :2018/5/31 下午 7:07
Describe        :在這邊決定名稱並分流
 *		inTxnType:sale、refund
 *		uszPaperBit:是否已出紙本
 *
 */
int inBATCH_Get_ESC_Upload_Count_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inTxnType, unsigned char uszPaperBit, int *inTxnTotalCnt, long *lnTxnTotalAmt)
{
	int inRetVal;
	char szTableName[50 + 1]; /* 若傳進的TableName1為空字串，則用szTableName組TableName */

	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof (szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;

			/* ESC */
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;

			/* 票證 */
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ADVICE_SUFFIX_);
			break;

		default:
			break;
	}

	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			inRetVal = inSqlite_Get_ESC_Sale_Upload_Count(pobTran, szTableName, inTxnType, uszPaperBit, inTxnTotalCnt, lnTxnTotalAmt);
			break;
		default:
			return (VS_ERROR);
			break;
	}

	return (inRetVal);
}

/*
Function        :inBATCH_GetAdvice_ESVC_DetailRecord_By_Sqlite
Date&Time       :2018/1/26 下午 2:28
Describe        :目前和inBATCH_GetAdvice_DetailRecord_By_Sqlite完全相同，但為了保險所以也分開
 */
int inBATCH_GetAdvice_ESVC_DetailRecord_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inBATCH_GetAdvice_ESC_DetailRecord_By_Sqlite()_START");
	}
	/* [新增電票悠遊卡功能] 重新啟用電票功能 [SAM] 2022/6/14 下午 12:06 */
	inRetVal = inNCCC_Ticket_ESVC_Get_TRec_Top_Flow(pobTran, _TN_BATCH_TABLE_TICKET_ADVICE_);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inBATCH_GetAdvice_ESC_DetailRecord_By_Sqlite()_END");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inBATCH_Presssure_Test
Date&Time       :2017/10/17 下午 2:17
Describe        :
 */
int inBATCH_Presssure_Test()
{

	return (VS_SUCCESS);
}
