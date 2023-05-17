/*
 * 主要使用在設定畫面第一層的函式，命名規則如下。
 *  inMENU01_NewUiTransactionMenu
 *  in: 回傳值型態
 *  MENU01 : 顯示UI層級
 *  NewUiTransactionMenu : 函式功能名稱  
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctosapi.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>


#include "../../INCLUDES/Define_1.h"
#include "../../INCLUDES/Define_2.h"
#include "../../INCLUDES/Transaction.h"
#include "../../INCLUDES/TransType.h"
#include "../../PRINT/Print.h"
#include "../../DISPLAY/Display.h"
#include "../../DISPLAY/DisTouch.h"

#include "../MenuMsg.h"
#include "../Menu.h"
#include "../Flow.h"

#include "../../FUNCTION/CREDIT_FUNC/CreditCheckFunc.h"
#include "../../FUNCTION/Function.h"
#include "../../FUNCTION/EDC.h"
#include "../../FUNCTION/CFGT.h"

#include "Layer1Menu.h"
#include "Layer2Menu.h"


extern unsigned char guszCTLSInitiOK;
extern int ginIdleMSRStatus, ginIdleICCStatus, ginMenuKeyIn; /* */
extern int	ginMachineType;

