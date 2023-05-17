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
#include "../../FUNCTION/TDT.h"

#include "EDCTmsTDTFunc.h"

extern	int	ginDebug;  /* Debug使用 extern */

/*
Function        : inEDCTMS_TDT_ReInitialTicketTable
Date&Time   : 2018/1/29 下午 6:02
Describe        : 初始 TDT 資料
 * inEDCTMS_DeInitial_Ticket
*/
int inEDCTMS_TDT_ReInitialTicketTable()
{
	int	i = 0;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	for (i = 0; i < 4; i++)
	{
		inLoadTDTRec(i);
		/* [新增電票悠遊卡功能] 修改初始化TDT的規則 [SAM] 2022/6/24 上午 10:47 */	
		if (i == _TDT_INDEX_00_IPASS_)
		{
			inSetTicket_HostName(_HOST_NAME_IPASS_);
		}
		else if (i == _TDT_INDEX_01_ECC_)
		{
			inSetTicket_HostName(_HOST_NAME_ECC_);
		}
		else if (i == _TDT_INDEX_02_ICASH_)
		{
			inSetTicket_HostName(_HOST_NAME_ICASH_);
		}
		else if (i == _TDT_INDEX_03_HAPPYCASH_)
		{
			inSetTicket_HostName(_HOST_NAME_HAPPYCASH_);
		}
		inSetTicket_HostTransFunc("                    ");
		inSetTicket_HostEnable("N");
		inSetTicket_LogOnOK("N");
		inSetTicket_SAM_Slot("01");
		inSetTicket_ReaderID("    ");
		inSetTicket_STAN("000001");
		inSetTicket_LastTransDate("00000000");
		inSetTicket_LastRRN("            ");
		inSetTicket_Device1("                    ");
		inSetTicket_Device2("                    ");
		inSetTicket_Batch("        ");
		inSetTicket_NeedNewBatch("Y");
		inSetTicket_Device3("                    ");
		inSaveTDTRec(i);
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}
