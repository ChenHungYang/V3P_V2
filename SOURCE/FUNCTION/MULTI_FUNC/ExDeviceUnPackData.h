/* 
 * File:   ExDeviceUnPackData.h
 * Author: Sam chang
 *
 * Created on 2022年11月22日, 上午 11:26
 */

#ifndef EXDEVICEUNPACKDATA_H
#define	EXDEVICEUNPACKDATA_H

#ifdef	__cplusplus
extern "C" {
#endif
	
	
#define MAX_EMV_BUFFER_VALUE	256
#define MAX_BUFFER_VALUE 2048

#define _SCANDATA_FILE_NAME_ "scandata.sc"
	
typedef  struct 
{
	int inTagType;
	BYTE bTagName[30 + 1];
	BYTE bTagMessage[MAX_BUFFER_VALUE + 1];
}STRUCT_JASON_DATA;	


typedef int (*EMV_SET_FUNC_POINT)(TRANSACTION_OBJECT*, char*, char*);

typedef struct
{
	int inSetValue;
	BYTE bTagName[30];
	BYTE bTagMessage[MAX_EMV_BUFFER_VALUE + 1];
	EMV_SET_FUNC_POINT	 inSetFunctionPoint;
}STRUCT_JSON_EMV_DATA;


int inJSON_EMV_SetTag57Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag5AData(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag5F20Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag5F24Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag5F2AData(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag5F34Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag82Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag84Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag8AData(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag95Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag9AData(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag9BData(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag9CData(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag9F02Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag9F03Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag9F08Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag9F09Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag9F10Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag9F1AData(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag9F1EData(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag9F26Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag9F27Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag9F33Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag9F34Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag9F35Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag9F36Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag9F37Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag9F41Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag9F63Data(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);
int inJSON_EMV_SetTag9F1FData(TRANSACTION_OBJECT *pobTran, char *szTag, char *szData);

int inJSON_DebugUnPackDataTest(STRUCT_JASON_DATA *stUnpackData);
//int inJSON_DebugUnPackData(STRUCT_JSON_EMV_DATA *stUnpakData);
int inJSON_DebugUnPackData(void);
//int inJSON_AnylisysEmvDataIntoBrec(TRANSACTION_OBJECT *pobTran, char *szEmvTag, char* szEmvData);
//int inJSON_AnylisysEmvDataIntoBrec(TRANSACTION_OBJECT *pobTran, STRUCT_JSON_EMV_DATA *stUnpakData);
int inJSON_AnylisysEmvDataIntoBrec(TRANSACTION_OBJECT *pobTran);
int inJSON_GetCjsonString(cJSON *Param, char *szResult);
int inJSON_CheckEmvTagAndUnPackData(TRANSACTION_OBJECT *pobTran, cJSON *root);
int inJSON_UnPackEmvDataAfterParse(TRANSACTION_OBJECT *pobTran, cJSON *root);
int inJSON_UnPackReceiveData(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *srUpkMultiTable);



#ifdef	__cplusplus
}
#endif

#endif	/* EXDEVICEUNPACKDATA_H */

