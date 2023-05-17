#include <emvaplib.h>   /* V3C 用 */
#include <typedef.h>    /* UPT用 */

#define _EMV_CONFIG_FILENAME_       	"emv_config.xml"

#define _TAG_91_ISS_AUTH_DATA_		0x91
#define _TAG_71_ISUER_SCRPT_TEMPL_		0x71
#define _TAG_72_ISUER_SCRPT_TEMPL_		0x72

/* 用以標示是否要顯示請按消除鍵 */
#define _REMOVE_CARD_ERROR_			0
#define _REMOVE_CARD_NOT_ERROR_		1

#define _EMV_RESULT_APPROVE_				1
#define _EMV_RESULT_DECLINE_				2
#define _EMV_RESULT_UNKOWN_			3

#define _CVM_RESULT_NEED_SIGNATURE_		1
#define _CVM_RESULT_NO_NEED_SIGNATURE_	2

#define _EMV_DECESION_ONLINE			1
#define _EMV_DECESION_OFFLINE			2

/* SELECT AID 使用哪一個界面 */
#define _CONTACT_TYPE_00_CONTACT_		0
#define _CONTACT_TYPE_01_CONTACTLESS_		1

int inCheckFallbackFlag(TRANSACTION_OBJECT *pobTran);
int inEMV_ICCEvent(void);
int inEMV_CheckRemoveCard(TRANSACTION_OBJECT *pobTran, int inIsError);
int inEMV_GetEMVCardData(TRANSACTION_OBJECT *pobTran);
int inEMV_GetEMVTag(TRANSACTION_OBJECT *pobTran);
int inEMV_CreditSelectAID(TRANSACTION_OBJECT *pobTran, int inContactType);
int inEMV_CreditPowerON(TRANSACTION_OBJECT *pobTran);
int inEMV_CreditPowerOFF(TRANSACTION_OBJECT *pobTran);
int inEMV_SelectICCAID(TRANSACTION_OBJECT *pobTran);

/* CallBack Function*/
void OnDisplayShow(IN char *pStrMsg);
void OnErrorMsg(IN char *pStrMsg);
void OnEMVConfigActive(INOUT BYTE* pActiveIndex);
USHORT OnTxnDataGet(OUT EMV_TXNDATA *pTxnData);
USHORT OnAppList(IN BYTE AppNum, IN char AppLabel[][d_LABEL_STR_SIZE+1], OUT BYTE *pAppSelectedIndex);
USHORT OnAppSelectedConfirm(IN BOOL IsRequiredbyCard, IN BYTE *pLabel, IN BYTE bLabelLen);
BOOL OnTerminalDataGet(IN USHORT usTag, INOUT USHORT *pLen, OUT BYTE *pValue);
BOOL OnCAPKGet(IN BYTE *pRID, IN BYTE bKeyIndex, OUT BYTE *pModulus, OUT USHORT *pModulusLen, OUT BYTE *pExponent, OUT USHORT *pExponentLen);
void OnGetPINNotify(IN BYTE bPINType, IN USHORT bRemainingCounter, OUT BOOL* pIsUseDefaultGetPINFunc, OUT DEFAULT_GETPIN_FUNC_PARA *pPara);
USHORT OnOnlinePINBlockGet(OUT ONLINE_PIN_DATA *pOnlinePINData);
USHORT OnOfflinePINBlockGet(void);
void OnOfflinePINVerifyResult(IN USHORT usResult);
void OnTxnOnline(IN ONLINE_PIN_DATA *pOnlinePINData, OUT EMV_ONLINE_RESPONSE_DATA* pOnlineResponseData);
void OnTxnIssuerScriptResult(IN BYTE* pScriptResult, IN USHORT pScriptResultLen);
void OnTxnResult(IN BYTE bResult, IN BOOL IsSignatureRequired);
void OnTotalAmountGet(IN BYTE *pPAN, IN BYTE bPANLen, OUT ULONG *pAmount);
void OnExceptionFileCheck(IN BYTE *pPAN, IN BYTE bPANLen, OUT BOOL *isException);
BOOL OnCAPKRevocationCheck(IN BYTE *pbRID, IN BYTE bCAPKIdx, BYTE *pbSerialNumuber);
void OnEventTxnForcedOnline(OUT BYTE *pbForcedONL);
USHORT OnEventAppListEx(IN BYTE bAppNum, IN EMV_APP_LIST_EX_DATA *pstAppListExData, OUT BYTE *pbAppSelectedIndex);

int inEMV_Initial(void);
int inEMV_Process(TRANSACTION_OBJECT *pobTran);
int inEMV_SecondGenerateAC(TRANSACTION_OBJECT *pobTran);
int inNCCC_FuncEMVPrepareBatch(TRANSACTION_OBJECT *pobTran);
int inNCCC_EMVUnPackData55(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf, int inLen);
int inEMV_Set_TagValue_During_Txn(unsigned short usTag, unsigned short usLen, unsigned char *uszValue);
int inNCCC_DCC_EMV_Set_Value(TRANSACTION_OBJECT * pobTran);
int inEMV_Get_Tag_Value(unsigned short usTag, unsigned short *usLen, unsigned char *uszValue);
int inEMV_SetICCReadFailure(int inFallbackSwitch);
int inEMV_GetKernelVersion(void);
int inEMV_GetCardNoFlow(TRANSACTION_OBJECT *pobTran);

/* 流程 */
int inEMV_Decide_SetTxnOnlineOffline(TRANSACTION_OBJECT * pobTran);
int inEMV_Decide_OnlineEMV_Complete(void);
int inEMV_Decide_DispHostRespCodeMsg(void);


int inEMV_FubonRunCupLogOn(TRANSACTION_OBJECT *pobTran, char * szCheckHostName);

