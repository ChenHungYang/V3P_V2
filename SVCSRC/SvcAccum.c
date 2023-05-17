
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>

#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/TransType.h"

#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/EVENT/MenuMsg.h"

#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/Card.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/FILE_FUNC/File.h"
#include "../SOURCE/FUNCTION/FILE_FUNC/FIleLogFunc.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/Accum.h"

#include "SvcAccum.h"

/*
Function	: inSVC_ACCUM_StoreRecord
Date&Time	: 2022/12/26 下午 8:16
Describe	:
*/
int inSVC_ACCUM_StoreRecord(SVC_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszFileName)
{
        unsigned long   ulHandlePtr;    /* File Handle，type為pointer */
        int		inRetVal; /* return value，來判斷是否回傳error */

        inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

        /*開檔*/
        inRetVal = inFILE_Open(&ulHandlePtr, uszFileName);
        /*開檔成功*/
        if (inRetVal != VS_SUCCESS)
        {
                /* 開檔失敗時，不必關檔(開檔失敗，handle回傳NULL) */
                /* 開檔失敗，所以回傳error */
                return(VS_ERROR);
        }

        /* 把指針指到開頭*/
        inRetVal = inFILE_Seek(ulHandlePtr, 0, _SEEK_BEGIN_);

        /* seek不成功時 */
        if (inRetVal != VS_SUCCESS)
        {
                /* inFILE_Seek失敗時 */
                /* 關檔並回傳VS_ERROR */
                inFILE_Close(&ulHandlePtr);
                /* seek失敗，所以回傳error。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }

        /* 寫檔 */
        if (inFILE_Write(&ulHandlePtr,(unsigned char*)srAccumRec, SVC_ACCUM_TOTAL_REC_SIZE) != VS_SUCCESS)
        {
                /*寫檔失敗時*/
                /* 關檔  */
                inFILE_Close(&ulHandlePtr);
                /* 寫檔失敗，所以回傳error。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }

        /* 關檔  */
        if (inFILE_Close(&ulHandlePtr) != VS_SUCCESS)
        {
                /*關檔失敗時*/
                /* 關檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);

        return (VS_SUCCESS);
}

/*
Function	: inSVC_ACCUM_UpdateTotalAmount
Date&Time	: 2022/12/26 下午 7:57
Describe	: 每當有交易時，到此fuction更新筆數和總額，回傳(VS_SUCCESS) or (VS_ERROR)
 */
int inSVC_ACCUM_UpdateTotalAmount(TRANSACTION_OBJECT *pobTran, SVC_ACCUM_TOTAL_REC *srAccumRec)
{
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	switch (pobTran->srBRec.inCode)
	{
		case _SVC_REDEEM_:
			srAccumRec->lnTotalRedeemCount++;
			srAccumRec->lnTotalCount++;

			if ((pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnSvcRedeemAmount) >= 0)
			{
				srAccumRec->dbTotalRedeemAmount += (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnSvcRedeemAmount);
				srAccumRec->dbTotalAmount += (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnSvcRedeemAmount);
			} else
			{
				srAccumRec->dbTotalRedeemAmount += pobTran->srBRec.lnTxnAmount;
				srAccumRec->dbTotalAmount += pobTran->srBRec.lnTxnAmount;
			}
			break;
		case _SVC_REFUND_:
			srAccumRec->lnTotalRefundCount++;
			srAccumRec->dbTotalRefundAmount += pobTran->srBRec.lnTxnAmount;

			srAccumRec->lnTotalReloadCount++;
			srAccumRec->lnTotalCount++;

			srAccumRec->dbTotalReloadAmount += pobTran->srBRec.lnTxnAmount;
			srAccumRec->dbTotalAmount += pobTran->srBRec.lnTxnAmount;
			break;
		case _SVC_RELOAD_:
			srAccumRec->lnTotalReloadCount++;
			srAccumRec->lnTotalCount++;

			srAccumRec->dbTotalReloadAmount += pobTran->srBRec.lnTxnAmount;
			srAccumRec->dbTotalAmount += pobTran->srBRec.lnTxnAmount;
			break;
		case _SVC_ACTIVECARD_:
			srAccumRec->lnTotalActiveCardCount++;
			srAccumRec->lnTotalCount++;

			srAccumRec->dbTotalActiveCardAmount += pobTran->srBRec.lnTxnAmount;
			srAccumRec->dbTotalAmount += pobTran->srBRec.lnTxnAmount;
			break;
		case _SVC_VOID_:
			srAccumRec->lnTotalVoidCount++;
			srAccumRec->lnTotalCount++;

			srAccumRec->dbTotalVoidAmount -= pobTran->srBRec.lnTxnAmount;
			srAccumRec->dbTotalAmount += pobTran->srBRec.lnTxnAmount;
			break;

		case _SVC_BACKCARD_:
			srAccumRec->lnTotalBackcardCount++;
			srAccumRec->lnTotalCount++;
			srAccumRec->dbTotalBackcardAmount += pobTran->srBRec.lnTxnAmount;
			break;
		default:
			inDISP_DispLogAndWriteFlie("  Accum Update Total Amt *Error* inOrgCode[%d] Line[%d]", pobTran->srBRec.inOrgCode, __LINE__);
			break;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        : inSVC_ACCUM_Update_Flow
Date&Time   : 2022/12/26 下午 8:18
Describe        : 
 *
*/
int inSVC_ACCUM_Update_Flow(TRANSACTION_OBJECT *pobTran)
{
	unsigned long ulHandlePtr = 0;	 /* File Handle，type為pointer */
	int inRetVal;					   /* return value，來判斷是否回傳error */
	unsigned char uszFileName[50 + 1]; /* 儲存交易金額檔案的檔案名稱(最大為15) */
	SVC_ACCUM_TOTAL_REC srAccumRec; /*用來放總筆數、總金額的結構體*/

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memset(uszFileName, 0x00, sizeof(uszFileName));
	if (inFunc_ComposeFileName(pobTran, (char *)uszFileName, _ACCUM_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		/* Update Accum失敗 鎖機*/
		inFunc_EDCLock();
		return (VS_ERROR);
	}

	/* 開檔 create檔(若沒有檔案則創建檔案) */
	if (inFILE_Open(&ulHandlePtr, uszFileName) == (VS_ERROR))
	{
		/* 開檔失敗時，不必關檔(開檔失敗，handle回傳NULL) */
		/* 若檔案不存在時，Create檔案 */
		if (inFILE_Check_Exist(uszFileName) == (VS_ERROR))
		{
			inRetVal = inFILE_Create(&ulHandlePtr, uszFileName);
		}
		/* 檔案存在還是開檔失敗，回傳錯誤跳出 */
		else
		{
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock();
			return (VS_ERROR);
		}

		/* Create檔 */
		if (inRetVal != VS_SUCCESS)
		{
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock();
			/* Create檔失敗時不關檔，因為create檔失敗handle回傳NULL */
			return (VS_ERROR);
		}
		/* create檔成功就繼續(因為create檔已經把handle指向檔案，所以不用在開檔) */
	}

	/* 開檔成功或create檔成功後 */
	/*先清空srAccumRec 為讀檔案作準備  */
	memset(&srAccumRec, 0x00, sizeof(srAccumRec));

	/* 把指針指到開頭*/
	inRetVal = inFILE_Seek(ulHandlePtr, 0, _SEEK_BEGIN_);

	/* inFile_seek */
	if (inRetVal != VS_SUCCESS)
	{
		/* Update Accum失敗 鎖機*/
		inFunc_EDCLock();

		/* inFILE_Seek失敗時 */
		/* Seek檔失敗，所以關檔 */
		inFILE_Close(&ulHandlePtr);

		/* Seek檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
		return (VS_ERROR);
	}

	/* 2015/11/20 修改 */
	/* 當文件內容為空時，read會回傳error;所以當read失敗， 而且檔案長度不等於0的時候，才是真的出錯 */
	if (inFILE_Read(&ulHandlePtr, (unsigned char *)&srAccumRec, SVC_ACCUM_TOTAL_REC_SIZE) == VS_ERROR && lnFILE_GetSize(&ulHandlePtr, (unsigned char *)&srAccumRec) != 0)
	{
		/* Update Accum失敗 鎖機*/
		inFunc_EDCLock();

		/* 讀檔失敗 */
		/* Read檔失敗，所以關檔 */
		inFILE_Close(&ulHandlePtr);

		/* Read檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
		return (VS_ERROR);
	}
	else
	{
		/* 計算全部交易金額、筆數 */
		if (inSVC_ACCUM_UpdateTotalAmount(pobTran, &srAccumRec) == VS_ERROR)
		{
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock();

			/* 計算全部交易金額、筆數失敗 */
			/* 計算全部交易金額、筆數失敗，所以關檔 */
			inFILE_Close(&ulHandlePtr);

			/* 計算全部交易金額、筆數失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
			return (VS_ERROR);
		}

		
		/* 先將檔案關閉 */
		if (inFILE_Close(&ulHandlePtr) == (VS_ERROR))
		{
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock();

			/* 關檔失敗 */
			/* 回傳VS_ERROR */
			return (VS_ERROR);
		}

		/* 存檔案 */
		if (inSVC_ACCUM_StoreRecord(&srAccumRec, uszFileName) == VS_ERROR)
		{
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock();

			/* 存檔失敗 */
			/* 因為inACCUM_StoreRecord失敗，所以回傳VS_ERROR */
			return (VS_ERROR);
		}

	} /* read、計算總額 和 存檔 成功 */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

