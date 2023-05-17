
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>

#include "../../../SOURCE/INCLUDES/Define_1.h"
#include "../../../SOURCE/INCLUDES/Define_2.h"
#include "../../../SOURCE/INCLUDES/TransType.h"
#include "../../../SOURCE/INCLUDES/Transaction.h"
#include "../../../SOURCE/DISPLAY/Display.h"
#include "../../../SOURCE/DISPLAY/DispMsg.h"
#include "../../../SOURCE/DISPLAY/DisTouch.h"

#include "../../EVENT/MenuMsg.h"

#include "../../FUNCTION/Function.h"
#include "../../FUNCTION/Sqlite.h"
#include "../../FUNCTION/HDT.h"
#include "../../FUNCTION/HDPT.h"
#include "../FILE_FUNC/File.h"
#include "../FILE_FUNC/FIleLogFunc.h"

#include "CreditCheckFunc.h"

/*
Function        :inCREDIT_CheckTransactionFunction
Date&Time   : 20190308
Describe        :確認交易功能是否打開
 *  0 銷售(1Byte) 1 取消(1Byte) 2 結帳(1Byte) 3 退貨(1Byte) 4 交易補登(1Byte) 5 預先授權(1Byte) 6小費(1Byte)+
 *  7 調帳(1Byte) 8 分期付款(1Byte) 9 紅利扣抵(1Byte) 10 分期調帳(1Byte) 11 紅利調帳(1Byte)
 *  12 郵購(1Byte)+用０補足 20 Byt
 */
int inCREDIT_CheckTransactionFunction(int inCode)
{
	char szTransFunc[20 + 1];
	unsigned char uszTxnEnable = VS_TRUE;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	/*  目前功能開關只適用在主要的主機 [SAM]*/
	if (inLoadHDTRec(0) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" Credit Check Trans Func Load HDT 0 *Error* Line[%d]", __LINE__);
	}

	/* 未完成，先鎖起來 */
	if (inCode == _CUP_MAIL_ORDER_REFUND_)
	{
		uszTxnEnable = VS_FALSE;
	}

	memset(szTransFunc, 0x00, sizeof (szTransFunc));
	if (inGetTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);

	if (inCode == _SALE_ ||
		inCode == _CUP_SALE_ ||
		inCode == _FISC_SALE_)
	{
		if (szTransFunc[0] != 0x31)
		{
			uszTxnEnable = VS_FALSE;
		}
	} else if (inCode == _VOID_ ||
			inCode == _CUP_VOID_ ||
			inCode == _CUP_PRE_AUTH_VOID_ ||
			inCode == _CUP_PRE_COMP_VOID_ ||
			inCode == _FISC_VOID_)
	{
		if (szTransFunc[1] != 0x31)
		{
			uszTxnEnable = VS_FALSE;
		}
	} else if (inCode == _SETTLE_)
	{
		if (szTransFunc[2] != 0x31)
		{
			uszTxnEnable = VS_FALSE;
		}
	} else if (inCode == _REFUND_ ||
			inCode == _CUP_REFUND_ ||
			inCode == _FISC_REFUND_)
	{
		if (szTransFunc[3] != 0x31)
		{
			uszTxnEnable = VS_FALSE;
		}
	}/* NCCC(含MPAS)的退貨功能一併管理分期、紅利退貨 */
		/* 分期退貨要看兩個開關 */
	else if (inCode == _INST_REFUND_ ||
			inCode == _CUP_INST_REFUND_)
	{

		if (szTransFunc[3] != 0x31 || szTransFunc[8] != 0x31)
		{
			uszTxnEnable = VS_FALSE;
		}
	}/* 紅利退貨要看兩個開關 */
	else if (inCode == _REDEEM_REFUND_ ||
			inCode == _CUP_REDEEM_REFUND_)
	{
		if (szTransFunc[3] != 0x31 || szTransFunc[9] != 0x31)
		{
			uszTxnEnable = VS_FALSE;
		}
	} else if (inCode == _SALE_OFFLINE_)
	{
		if (szTransFunc[4] != 0x31)
		{
			uszTxnEnable = VS_FALSE;
		}
	} else if (inCode == _PRE_AUTH_ ||
			inCode == _PRE_COMP_ ||
			inCode == _CUP_PRE_AUTH_ ||
			inCode == _CUP_PRE_COMP_)
	{
		if (szTransFunc[5] != 0x31)
		{
			uszTxnEnable = VS_FALSE;
		}
	} else if (inCode == _TIP_)
	{
		if (szTransFunc[6] != 0x31)
		{
			uszTxnEnable = VS_FALSE;
		}
	} else if (inCode == _ADJUST_)
	{
		if (szTransFunc[7] != 0x31)
		{
			uszTxnEnable = VS_FALSE;
		}
	} else if (inCode == _INST_SALE_ ||
			inCode == _CUP_INST_SALE_)
	{
		if (szTransFunc[8] != 0x31)
		{
			uszTxnEnable = VS_FALSE;
		}
	} else if (inCode == _REDEEM_SALE_ ||
			inCode == _CUP_REDEEM_SALE_)
	{
		if (szTransFunc[9] != 0x31)
		{
			uszTxnEnable = VS_FALSE;
		}
	} else if (inCode == _INST_ADJUST_)
	{
		if (szTransFunc[10] != 0x31)
		{
			uszTxnEnable = VS_FALSE;
		}
	} else if (inCode == _REDEEM_ADJUST_)
	{
		if (szTransFunc[11] != 0x31)
		{
			uszTxnEnable = VS_FALSE;
		}
	}

	/* 關閉銀聯分期紅利 */
	if (inCode == _CUP_INST_SALE_ ||
			inCode == _CUP_REDEEM_SALE_)
	{
		uszTxnEnable = VS_FALSE;
	}


	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);

	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	} else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        : inCREDIT_CheckTransactionFunctionFlow
Date&Time   : 2019/03/13
Describe        : 確認交易功能是否打開，因為根據Host，所以每個Host一個Function
 */
int inCREDIT_CheckTransactionFunctionFlow(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_SUCCESS;
	unsigned char uszTxnEnable = VS_TRUE;

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	inRetVal = inCREDIT_CheckTransactionFunction(pobTran->srBRec.inCode);
	if (inRetVal != VS_SUCCESS)
	{
		uszTxnEnable = VS_FALSE;
	}

	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] FALSE END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	} else
	{
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}
}

/*
Function		: inCREDIT_VoidCheckFunc
Date&Time	: 2019/03/12 
Describe		: 確認是否能進行取消交易，如果銀行判斷不同，請再自行新增函式，不要再此功能上修改。
 */
int inCREDIT_CheckVoidFunc(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_SUCCESS;
	char szTRTFileName[12 + 1];

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memset(szTRTFileName, 0x00, sizeof (szTRTFileName));
	inGetTRTFileName(szTRTFileName);

	if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		/* 顯示此交易已取消 */
		inRetVal = inDISP_Msg_BMP(_ERR_CANCEL_ALREADY_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		inDISP_DispLogAndWriteFlie("---- VOID BIT = VS_TRUE Line[%d] END ---- ", __LINE__);
		return (inRetVal);
	} else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)/* 擋銀聯交易 沒按銀聯鍵取消 */
	{
		/* 請按銀聯鍵 */
		inRetVal = inDISP_Msg_BMP(_ERR_PLEASE_PRESS_CUP_BUTTON_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		inDISP_DispLogAndWriteFlie("---- uszCUPTransBit = VS_TRUE Line[%d] END ---- ", __LINE__);
		return (inRetVal);
	} else if (pobTran->srBRec.inCode == _TIP_)/* NCCC 小費不能取消 */
	{
		/* 顯示小費交易 交易不能取消 請按清除鍵 */
		inRetVal = inDISP_Msg_BMP(_ERR_CAN_NOT_CANCEL_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "小費交易", _LINE_8_5_);
		inDISP_DispLogAndWriteFlie("---- inCode = _TIP_ Line[%d] END ---- ", __LINE__);
		return (inRetVal);
	} else if (pobTran->srBRec.inCode == _ADJUST_)
	{
		/* 顯示調整交易 交易不能取消 請按清除鍵 */
		inRetVal = inDISP_Msg_BMP(_ERR_CAN_NOT_CANCEL_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "調整交易", _LINE_8_5_);
		inDISP_DispLogAndWriteFlie("---- inCode = _ADJUST_ Line[%d] END ---- ", __LINE__);
		return (inRetVal);
	} else if (pobTran->srBRec.inOrgCode == _PRE_AUTH_)/* 預先授權不能取消 */
	{
		/* 顯示預先授權不能取消 */
		inRetVal = inDISP_Msg_BMP(_ERR_PREAUTH_CANCEL_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		inDISP_DispLogAndWriteFlie("---- inOrgCode = _PRE_AUTH_ Line[%d] END ---- ", __LINE__);
		return (inRetVal);
	} else if (pobTran->srBRec.inOrgCode == _LOYALTY_REDEEM_)
	{
		/* 無交易紀錄 */
		inRetVal = inDISP_Msg_BMP(_ERR_RECORD_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		inDISP_DispLogAndWriteFlie("---- inOrgCode = _LOYALTY_REDEEM_ Line[%d] END ---- ", __LINE__);
		return (inRetVal);
	} else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		/* 取消交易不能取消Smart Pay交易 */
		/* 請依正確卡別操作 */
		inRetVal = inDISP_Msg_BMP(_ERR_PLEASE_FOLLOW_RIGHT_OPT_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		inDISP_DispLogAndWriteFlie("---- uszFiscTransBit = VS_TRUE Line[%d] END ---- ", __LINE__);
		return (inRetVal);
	} else
	{
		if (pobTran->srBRec.uszHappyGoMulti == TRUE)
		{
			if (!memcmp(szTRTFileName, _TRT_FILE_NAME_HG_, strlen(_TRT_FILE_NAME_HG_)))
			{
				/* 取消HG混合交易，要選擇信用卡主機 */
				/* 請選擇信用卡主機 */
				inRetVal = inDISP_Msg_BMP(_ERR_CHOOSE_CREDIT_HOST_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
				inDISP_DispLogAndWriteFlie("---- uszHappyGoMulti = TRUE Line[%d] END ---- ", __LINE__);
				return (inRetVal);
			} else
			{

			}
		}
	}

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
Function        : inCREDIT_CheckTipFunc
Date&Time   : 2019/03/13
Describe        : 確認是否能進行小費交易
 */
int inCREDIT_CheckTipFunc(TRANSACTION_OBJECT *pobTran)
{
	/* 改正向表列比較簡單 */
	if (pobTran->srBRec.inCode != _SALE_ &&
		pobTran->srBRec.inCode != _SALE_OFFLINE_)
	{
		/* 顯示不能做小費 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_Msg_BMP(_ERR_TIP_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		return (VS_ERROR);
	} else
		return (VS_SUCCESS);
}

/*
Function        : inCREDIT_CheckAdjustFunc
Date&Time   : 2019/03/13
Describe        : 確認是否能進行調帳交易
 */
int inCREDIT_CheckAdjustFunc(TRANSACTION_OBJECT *pobTran)
{

	/* 改為正向表列  2022/10/21 [SAM] */
	if(pobTran->srBRec.inCode == _SALE_ ||
		pobTran->srBRec.inCode == _SALE_OFFLINE_ ) 
	{
		return VS_SUCCESS;
	}else
	{
		inDISP_Msg_BMP(_ERR_ADJUST_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		return (VS_ERROR);
	}	

//	if (pobTran->srBRec.inCode == _TIP_ ||
//		pobTran->srBRec.inCode == _VOID_ ||
//		pobTran->srBRec.inCode == _REFUND_ ||
//		pobTran->srBRec.inCode == _INST_REFUND_ ||
//		pobTran->srBRec.inCode == _REDEEM_REFUND_ ||
//		pobTran->srBRec.inCode == _CUP_REFUND_ ||
//		pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_ ||
//		pobTran->srBRec.inCode == _ADJUST_ ||
//		pobTran->srBRec.inCode == _INST_ADJUST_ ||
//		pobTran->srBRec.inCode == _REDEEM_ADJUST_ ||
//		pobTran->srBRec.inCode == _REDEEM_SALE_ || 
//		pobTran->srBRec.inCode == _INST_SALE_ ||
//		pobTran->srBRec.inCode == _PRE_COMP_ )
//	{
//		inDISP_Msg_BMP(_ERR_ADJUST_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
//		return (VS_ERROR);
//	} else
//		return (VS_SUCCESS);
}




