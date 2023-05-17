
#ifndef __TIME_UNIT_H__
#define __TIME_UNIT_H__

int inTIME_UNIT_InitCalculateRunTimeGlobal_Start(unsigned long *ulRunTime, char *szMessage);
int inTIME_UNIT_GetRunTimeGlobal(unsigned long ulRunTime, char *szMessage);
int inTIME_UNIT_CalculateRunTimeGlobalStart(char *szMessage);
int inTIME_UNIT_GetGlobalTimeToCompare(char *szMessage);

#endif
