/* 
 * File:   ExDevicePackData.h
 * Author: Sam chang
 *
 * Created on 2022年11月17日, 下午 2:37
 */

#ifndef	__EXDEVICEPACKDATA_H__
#define	__EXDEVICEPACKDATA_H__

#ifdef	__cplusplus
extern "C" {
#endif
	
int inJSON_DebugPackData(cJSON* cjson_temp);
int inJSON_PackSaleData(TRANSACTION_OBJECT *pobTran, char* pszPackData);
int inJSON_PackTransStopData(char * pszPackData, char* szRespData);
int inJSON_PackSignPadData(TRANSACTION_OBJECT *pobTran, char* pszPackData);
int inJSON_PackReSignData(TRANSACTION_OBJECT *pobTran, char* pszPackData);
int inJSON_PackReadMifaeCardData(TRANSACTION_OBJECT *pobTran, char* pszPackData);
int inJSON_PackWirteMifaeCardData(TRANSACTION_OBJECT *pobTran, char* pszPackData);
int inJSON_PackScanData(TRANSACTION_OBJECT *pobTran, char* pszPackData);
int inJSON_PackDisplayMsgData(TRANSACTION_OBJECT *pobTran, char* pszPackData);
int inJSON_PackRebootData(TRANSACTION_OBJECT *pobTran, char* pszPackData);
int inJSON_PackScanData(TRANSACTION_OBJECT *pobTran, char* pszPackData);

#ifdef	__cplusplus
}
#endif

#endif	/* EXDEVICEPACKDATA_H */

