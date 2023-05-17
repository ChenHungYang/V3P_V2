#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include "curl/curl.h"

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
#include "../../FUNCTION/EDC.h"
#include "../../FUNCTION/MVT.h"


#include "EDCTmsMVTFunc.h"

/*
Function        : inEDCTMS_MVT_SyncMccCode
Date&Time   : 2017/9/15 下午 3:53
Describe        :
 * inEDCTMS_Sync_MCCCode
*/
int inEDCTMS_MVT_SyncMccCode()
{
	/* 將szMVTMerchantCategoryCode 同步到EDC.dat */
	char	szMVTMerchantCategoryCode[4 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	inLoadMVTRec(0);
	memset(szMVTMerchantCategoryCode, 0x00, sizeof(szMVTMerchantCategoryCode));
	inGetMVTMerchantCategoryCode(szMVTMerchantCategoryCode);

	inSetMCCCode(szMVTMerchantCategoryCode);
	inSaveEDCRec(0);
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (VS_SUCCESS);
}
