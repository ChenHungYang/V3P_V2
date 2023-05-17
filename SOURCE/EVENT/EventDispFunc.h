
#ifndef __EVENT_DISP_FUNC_H__
#define __EVENT_DISP_FUNC_H__

int inJSON_EMV_SetTag57Data(TRANSACTION_OBJECT *pobTran, char *szType, char *szData);
int inJSON_EMV_SetTag5AData(TRANSACTION_OBJECT *pobTran, char *szType, char *szData);


int inEVENT_DSIP_DecideDisplayIdleImage(void);
int inEVENT_DSIP_DecideDisplayIdleImageFullScreen(void);

#endif
