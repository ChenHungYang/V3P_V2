/**
 **	A Template for developing new terminal application
 **/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
/** These two files are necessary for calling CTOS API **/
#include <ctosapi.h>
#include "SOURCE/INCLUDES/Define_1.h"
#include "SOURCE/INCLUDES/Define_2.h"
#include "SOURCE/INCLUDES/Transaction.h"
#include "SOURCE/INCLUDES/TransType.h"
#include "SOURCE/DISPLAY/Display.h"
#include "SOURCE/DISPLAY/DisTouch.h"
#include "SOURCE/FUNCTION/Function.h"
#include "SOURCE/EVENT/Event.h"
#include "SOURCE/EVENT/Menu.h"
#include "SOURCE/EVENT/MenuMsg.h"
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

#include "SOURCE/FUNCTION/FILE_FUNC/File.h"
#include "SOURCE/FUNCTION/FILE_FUNC/FIleLogFunc.h"

/* FOR_TEST_USE 這是測試功能的註解*/

/* 這裡放library include的注意事項 */
/* MP200不能include 專案不能Link cyassl會和虹堡現有library衝突 */
/* V3UL一定要include bluetooth，不然會出現 look up error */
/* 虹堡的系統用 little endian  0x78 0x56 0x43 0x21 */

/* 新增了對 UPT 使用的判斷  _CASTLE_TYPE_UPT1000_   20190605 [SAM]*/

/* 新增 __NCCC_DCC_FUNC__ ，聯合DCC 流程，如果要用可以參考 20190731 [SAM] */
/* 新增 __NCCC_ESVC_FUNC__ ，聯合電簽 流程，如果要用可以參考 20190731 [SAM] */

/*新增 __TIME_UNIT_COUNT__  計算執行時間用，出版要關*/
/*新增 __NEW_SIGN_FUNC__    使用縮放功能 */

/*新增 _USE_FIX_FTP_PARAMETER_   因為要選擇，如果不是使用嘉利的TMS就使用固定的IP 值 2019/10/8 [SAM] */


#if 0
/* Debug使用 extern */
/* 以Comport輸出Log */

int ginTimeLogDebug = VS_FALSE;

int ginDebug = VS_TRUE;

/* 以列印方式列印Log */
int ginISODebug = VS_TRUE;

/* 以顯示方式顯示Log */
int ginDisplayDebug = VS_TRUE;

/* 工程師測試功能使用 */
int ginEngineerDebug = VS_TRUE;

/* TraceLog紀錄功能用 */
int ginFindRunTime = VS_FALSE;

/* UPT SDK 驗測時使用 */
int ginExamBit = VS_FALSE;

#else

int ginTimeLogDebug = VS_FALSE;

/* Debug使用 extern */
/* 以Comport輸出Log */
int ginDebug = VS_FALSE;

/* 以列印方式列印Log */
int ginISODebug = VS_FALSE;

/* 以顯示方式顯示Log */
int ginDisplayDebug = VS_FALSE;

/* 工程師測試功能使用 */
int ginEngineerDebug = VS_FALSE;

/* TraceLog紀錄功能用 */
int ginFindRunTime = VS_FALSE;

/* UPT SDK 驗測時使用 */
int ginExamBit = VS_FALSE;

#endif


int ginRecordTime[2][20] = {{0},{0}}; /* V3UL用來紀錄哪邊花比較多時間用的 */

/* 用來紀錄是哪種機器類型 */
int ginMachineType = 0;

/* 用來分別是那一個銀行的版本 */
int ginAPVersionType = 0;

/* 富邦用來判斷是否要檢查帳務用功能 */
int ginFuUnBlanceSettleFlag = 0;

unsigned char guszMKCheckValue[24] = {0};
unsigned char guszWKkCheckValue[24] = {0};

#ifdef _DEBUG_9F1F_TAG_
	unsigned char gusz9F1F[129] = {0};
	long gln9F1FLen = 0;
#endif

/* [新增電票悠遊卡功能]  把電票要用的參數先統一寫這  [SAM] 2022/6/8 下午 6:00 */
unsigned long gulDemoTicketPoint; /* DEMO用 */


/*
 * 機器讀出的 sizeof
 * int [4]
 * char [1]
 * unsigned short [2]
 * unsigned long [4]
 * 
 */

/**
 ** The main entry of the terminal application
 **/
int main(int argc, char *argv[])
{
	int inRetVal;

	inRetVal = inFILE_LOG_InitLogFile();
	if (inRetVal == VS_ERROR)
	{
		inDISP_LogPrintfWithFlag(" Flie Log Init Err [%d]", inRetVal);
	}

	/* 紀錄機器類型 */
	inFunc_Decide_Machine_Type(&ginMachineType);

	/* 確認銀行版本 */
	inFunc_Decide_APVersion_Type(&ginAPVersionType);

	/* 開機流程(開機流程不能中途跳出，所以都強制return success) */
	inEVENT_Responder(_BOOTING_EVENT_);

	/* Idle流程 */
	while (1)
	{
		inMENU_Decide_Idle_Menu();
	}

	return 0;
}

