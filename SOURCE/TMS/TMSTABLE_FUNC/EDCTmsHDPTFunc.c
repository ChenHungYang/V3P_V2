#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include "curl/curl.h"
#include <sqlite3.h>

#include "../../INCLUDES/Define_1.h"
#include "../../INCLUDES/Define_2.h"
#include "../../INCLUDES/TransType.h"
#include "../../INCLUDES/Transaction.h"
#include "../../DISPLAY/Display.h"
#include "../../DISPLAY/DispMsg.h"
#include "../../DISPLAY/DisTouch.h"
#include "../../PRINT/Print.h"
#include "../../PRINT/PrtMsg.h"

#include "../../FUNCTION/Function.h"
#include "../../FUNCTION/Sqlite.h"
#include "../../FUNCTION/HDPT.h"

#include "EDCTmsHDPTFunc.h"


/*
Function        : inEDCTMS_HDPT_InitialFileValue
Date&Time   : 2017/1/3 下午 4:52
Describe        : 若參數生效，初始化HDPT
 * inEDCTMS_Initial_HDPT
*/
int inEDCTMS_HDPT_InitialFileValue()
{
	int	i;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	for(i = 0; ; i++)
	{
		if (inLoadHDPTRec(i) < 0)
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
		inSetMustISRUploadEnableBit("0");/* 富邦新增的參數 20190130 [SAM] */
		
		switch (i)
		{
			case 0:
				inSetTRTFileName(_TRT_FILE_NAME_CREDIT_);
				break;
			case 1:
				inSetTRTFileName(_TRT_FILE_NAME_DCC_);
				break;
			case 2:
				inSetTRTFileName(_TRT_FILE_NAME_AMEX_);
				break;
			case 3:
				inSetTRTFileName(_TRT_FILE_NAME_DINERS_);
				break;
			case 4:
				inSetTRTFileName(_TRT_FILE_NAME_REDEMPTION_);
				break;
			case 5:
				inSetTRTFileName(_TRT_FILE_NAME_INSTALLMENT_);
				break;
			case 6:
				inSetTRTFileName(_TRT_FILE_NAME_UNION_PAY_);
				break;
			case 7:
				inSetTRTFileName(_TRT_FILE_NAME_FISC_);
				break;
			case 8:
				inSetTRTFileName(_TRT_FILE_NAME_MAIL_ORDER_);
				break;
			case 9:
				inSetTRTFileName("OILCASHTRT");
				break;
			case 10:
				inSetTRTFileName("SCTRT");
				break;
			case 11:
				inSetTRTFileName("SCINSTTRT");
				break;
			case 12:
				inSetTRTFileName("EGIFTTRT");
				break;
			case 13:
				inSetTRTFileName("CMASTRT");
				break;
			case 14:
				inSetTRTFileName("IPASSTRT");
				break;
			case 15:
				inSetTRTFileName(_TRT_FILE_NAME_ESC_);
				break;
			case 16:
				inSetTRTFileName("ESUNPARKTRT");
				break;
			case 17:
				inSetTRTFileName("TSBPARKTRT");
				break;
			case 18:
				inSetTRTFileName("CBPARKTRT");
				break;
			case 19:
				inSetTRTFileName("ALIPAYTRT");
				break;
			case 20:
				inSetTRTFileName("NPBTTRT");
				break;
			default:
				break;
		}
		
		inSaveHDPTRec(i);
	}
	
	/* 目前不會使用DCC 需要先拿掉 20190108 [SAM]  */
//	if (inEDCTMS_DCC_Relist() == VS_SUCCESS)
//	{
//
//	}
//	else
//	{
//		/* 只要Relist失敗就有可能造成 HDT和HDPT的順序不同 */
//		inFunc_EDCLock();
//	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (VS_SUCCESS);
}


