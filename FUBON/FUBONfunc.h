#ifndef __FUBONFUNC__
#define __FUBONFUNC__

#define _FUBON_NONE_				0
#define _FUBON_ENCRYPTION_TSAM_		1
#define _FUBON_ENCRYPTION_SOFTWARE_	2


#define _CUSTOMER_000_STANDARD_			"000"
#define _CUSTOMER_001_TAIMALL_			"001"
#define _CUSTOMER_002_DREAM_MALL_		"002"
#define _CUSTOMER_003_HANKYU_			"003"
#define _CUSTOMER_004_KSSOGO_			"004"
#define _CUSTOMER_005_STANDARD_			"005"

#define CUSTOMER_000_STANDARD		0
#define CUSTOMER_001_TAIMALL		1
#define CUSTOMER_002_DREAM_MALL		2
#define CUSTOMER_003_HANKYU		3
#define CUSTOMER_004_KSSOGO		4
#define CUSTOMER_005_STANDARD		5

#define DISP_STR_LEN                    26              /* String disp'd on terminal  */

#define szGetCustomerIndicator()			_CUSTOMER_000_STANDARD_
#define inGetEncryptionModeEnable()		_FUBON_NONE_

#define PACK_SERVICE_CODE		1
#define NON_PACK_SERVICE_CODE			0

int inFUBON_RunTRT(TRANSACTION_OBJECT *pobTran, int inTRTCode);
int inFUBON_Func_Must_SETTLE(TRANSACTION_OBJECT *pobTran);
int inFUBON_FuncSetOnlineOffline(TRANSACTION_OBJECT *pobTran);
int inFUBON_OnlineEMV_Complete(TRANSACTION_OBJECT *pobTran);
int inFUBON_OnlineAnalyseMagneticManual(TRANSACTION_OBJECT *pobTran);
int inFUBON_OnlineAnalyseEMV(TRANSACTION_OBJECT *pobTran);
int inFUBON_GetSTAN(TRANSACTION_OBJECT *pobTran);
int inFUBON_SetSTAN(TRANSACTION_OBJECT *pobTran);
int inFUBON_CUP_FuncGetCMK(TRANSACTION_OBJECT *pobTran, char* Output);
int inFUBON_UserCehckLogon(TRANSACTION_OBJECT *pobTran);
int inFUBON_DispHostResponseCode(TRANSACTION_OBJECT *pobTran);
int inFUBON_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran);

int inFUBON_BootingCheckBatchData(TRANSACTION_OBJECT *pobTran);
int inFUBON_EnableFuUnBlanceSettleFlag(void);
int inFUBON_DisableFuUnBlanceSettleFlag(TRANSACTION_OBJECT *pobTran);
int inFUBON_GetFuUnBlanceSettleFlag(void);
int inFUBON_DispFuUnBlanceSettleMsg(void);
int inFUBON_InputYearDateTime(TRANSACTION_OBJECT *pobTran);
int inFUBON_PackField35Data(TRANSACTION_OBJECT *pobTran, char* szPackData, int inPackszServiceCode);

#endif
