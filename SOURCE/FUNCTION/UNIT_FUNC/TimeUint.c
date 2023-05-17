

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <ctosapi.h>

#include "../../INCLUDES/Define_1.h"

#include "../../DISPLAY/Display.h"

#ifdef __TIME_UNIT_COUNT__
static unsigned long ulSecond = 0;
static unsigned long ulMilliSecond = 0;
#endif
static unsigned long ulRunTime = 0;

/*
Function	  : inTIME_UNIT_InitCalculateRunTimeGlobal_Start   inFunc_CalculateRunTimeGlobal_Start
Date&Time : 20190703
Describe	  :  抓取第一次要計算的ms時間，用來看精度秒以下的RunTime
 */
int inTIME_UNIT_InitCalculateRunTimeGlobal_Start(unsigned long *ulRunTime, char *szMessage) {

#ifdef __TIME_UNIT_COUNT__
	*ulRunTime = CTOS_TickGet();

	inDISP_LogPrintfWithFlagForTimeTest("TIME_UNIT_INIT [%s] RunTime[%lu] ", szMessage, *ulRunTime);
#else
	*ulRunTime = 0;
#endif
	return (VS_SUCCESS);
}

/*
Function	: inTIME_UNIT_GetRunTimeGlobal   inFunc_GetRunTimeGlobal
Date&Time: 
Describe	: 用來看精度秒以下的RunTime
 */
int inTIME_UNIT_GetRunTimeGlobal(unsigned long ulRunTime, char *szMessage) {

#ifdef __TIME_UNIT_COUNT__	
	unsigned long ulRunTimeStart, ulRunTimeEnd, ulInterval;

	ulRunTimeStart = 0;
	ulRunTimeEnd = 0;
	ulInterval = 0;
	ulMilliSecond = 0;
	ulSecond = 0;

	/* 設定第一次抓取的時間 */
	ulRunTimeStart = ulRunTime;

	/* 抓取現在的時間 */
	ulRunTimeEnd = CTOS_TickGet();

	/* 計算差距時間 */
	ulInterval = ulRunTimeEnd - ulRunTimeStart;

	/* 超過秒的以秒計算 */
	ulSecond = ulInterval / 100;

	/* 未超過秒的以毫秒計算 */
	ulMilliSecond = 10 * (ulInterval % 100);

	inDISP_LogPrintfWithFlagForTimeTest("TIME_UNIT_RUN_TIME RunTimeStart[%lu] RunTimeEnd[%lu]", ulRunTimeStart, ulRunTimeEnd);
	inDISP_LogPrintfWithFlagForTimeTest("TIME_UNIT_RUN_TIME [%s] Sec[%ld] Mill[%ld]", szMessage, ulSecond, ulMilliSecond);
#endif	

	return (VS_SUCCESS);
}

/*
Function	  : inTIME_UNIT_CalculateRunTimeGlobalStart
Date&Time : 2016/3/7 上午 10:43
Describe	  : 用來看精度秒以下的RunTime
 */
int inTIME_UNIT_CalculateRunTimeGlobalStart(char *szMessage) {
	ulRunTime = CTOS_TickGet();

	inDISP_LogPrintfWithFlagForTimeTest("TIME_UNIT_INIT_GL [%s] RunTime[%lu] ", szMessage, ulRunTime);

	return (VS_SUCCESS);
}

int inTIME_UNIT_GetGlobalTimeToCompare(char *szMessage) {

#ifdef __TIME_UNIT_COUNT__
	unsigned long ulRunTimeEnd, ulInterval;

	ulRunTimeEnd = 0;
	ulInterval = 0;
	ulMilliSecond = 0;
	ulSecond = 0;

	/* 抓取現在的時間 */
	ulRunTimeEnd = CTOS_TickGet();

	/* 計算差距時間 */
	ulInterval = ulRunTimeEnd - ulRunTime;

	/* 超過秒的以秒計算 */
	ulSecond = ulInterval / 100;

	/* 未超過秒的以毫秒計算 */
	ulMilliSecond = 10 * (ulInterval % 100);

	inDISP_LogPrintfWithFlagForTimeTest("TIME_UNIT_RUN_TIME_GL  RunTime [%lu] RunTimeEnd[%lu]", ulRunTime, ulRunTimeEnd);
	inDISP_LogPrintfWithFlagForTimeTest("TIME_UNIT_RUN_TIME_GL [%s] Sec[%ld] Mill[%ld]", szMessage, ulSecond, ulMilliSecond);
#endif	
	return (VS_SUCCESS);
}

/*
Function		: ulgetUnixTimeCnt
Date&Time	: 2021/11/29 上午 10:30
Describe		: 回傳unixtime
 * [新增電票悠遊卡功能]  [SAM] 2022/6/8 下午 5:57
 */
unsigned long ulgetUnixTimeCnt(unsigned char *OutDate, unsigned char *date) {
	const char init_mdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	int year = 0x00, mon = 0x00, day = 0x00, hour = 0x00, min = 0x00, sec = 0x00;
	char temp[14];
	unsigned long tcnt = 0x00; //1.0.5.2
	int i = 0x00;
	char mdays[12];

	memset(temp, 0, sizeof (temp));
	memset(mdays, 0, sizeof (mdays));
	memcpy(mdays, init_mdays, 12);
	memcpy(temp, date, 14);
	sec = atoi(&temp[12]);
	temp[12] = 0;
	min = atoi(&temp[10]);
	temp[10] = 0;
	hour = atoi(&temp[8]);
	temp[8] = 0;
	day = atoi(&temp[6]);
	temp[6] = 0;
	mon = atoi(&temp[4]);
	temp[4] = 0;
	year = atoi(temp);

	tcnt = 0;
	for (i = 1970; i < year; i++) {

		tcnt += (86400 * 365); //YEAR
		if (i / 4 * 4 == i) {
			tcnt += 86400; //閏年
		}
	}
	if (year / 4 * 4 == year) {
		mdays[1] = 29;
	}
	for (i = 1; i < mon; i++)
		tcnt += (mdays[i - 1] * 86400);
	tcnt += ((day - 1) * 86400);
	tcnt += (hour * 3600);
	tcnt += (min * 60);
	tcnt += sec;

	OutDate[0] = (unsigned char) tcnt;
	OutDate[1] = (unsigned char) (tcnt >> 8);
	OutDate[2] = (unsigned char) (tcnt >> 16);
	OutDate[3] = (unsigned char) (tcnt >> 24);

	return tcnt;
}



//TODO: 看要不要整理名字
/*
Function		: inICERChkSpecialYear2
Date&Time	: 2021/11/29 上午 10:30
Describe		: 20211129,unix time減七天再轉回來
 * [新增電票悠遊卡功能]  [SAM] 2022/6/8 下午 5:58
*/
int inICERChkSpecialYear2(int inYear)
{

	if((inYear % 4) != 0)//不是閏年就傳回 1
		return(1);

	//每100年不閏,但每400年要閏
	if((inYear % 100) == 0)//100的倍數
		if((inYear % 400) != 0)//且不是400的倍數,就不閏,傳回 1
			return(1);

	return(0);
}

//TODO: 看要不要整理名字
/*
Function		: ECC_UnixToDateTime2
Date&Time	: 2021/11/29 上午 10:30
Describe		: 20211129,unix time減七天再轉回來
 * [新增電票悠遊卡功能]  [SAM]  2022/6/8 下午 5:58
*/
short ECC_UnixToDateTime2(unsigned char *bUnixDate,unsigned char *bDate,int inSize,int inTargetDay)
{
char   tmpUnix[50];
int inDay, inTotalDay, inRTC_Year, inRTC_Yday, inRTC_Mon, inRTC_Mday, inRTC_Hour, inRTC_Min, inRTC_Sec;
unsigned long ulUnixTime,ulTolSecond;
int inDayOfMon[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

	ulUnixTime = (bUnixDate[3] * 256 * 256 * 256) +
		     (bUnixDate[2] * 256 * 256) +
		     (bUnixDate[1] * 256) +
		     (bUnixDate[0]);

	if(ulUnixTime == 0L)
	{
		memcpy(bDate,"19700101000000",inSize);
		return VS_ERROR;
	}
        
       /*在此要減去天數*/
        ulUnixTime = ulUnixTime - (inTargetDay*86400);
        
	if(ulUnixTime < (10956 * 86400))//10956 * 86400表示從1970年開始到1999年為止的總秒數
		return VS_ERROR;
	ulUnixTime -= (10956 * 86400);//先把1970年開始到1999年為止的總秒數扣掉,表示從2000年開始計算
	inTotalDay = ulUnixTime / 86400;//86400表示1天24小時的總秒數
	ulTolSecond = ulUnixTime % 86400;//不足一天的總秒數

	//計算inYear
	for(inRTC_Year=2000;;inRTC_Year++)
	{
		inDay = 365;
		if(inICERChkSpecialYear2(inRTC_Year) == 0)//檢查 inYear 是否是閏年
			inDay = 366;

		if(inTotalDay <= inDay)//inTotalDay 總天數少於第 inYear 年的 inDay 天數就離開
			break;

		inTotalDay -= inDay;
	}
	inRTC_Yday = inTotalDay;

	//計算inMon
	for(inRTC_Mon=1;;inRTC_Mon++)
	{
		inDayOfMon[1] = 28;//先設定2月為28天;
		if(inRTC_Mon == 2)
			if(inICERChkSpecialYear2(inRTC_Year) == 0)//inYear表示該年,檢查該年是否是閏年
				inDayOfMon[1] = 29;//若為閏年,2月為29天

		if(inTotalDay <= inDayOfMon[inRTC_Mon - 1])//inTotalDay 總天數少於該年的第inDayOfMon[inMon - 1]月天數就離開
			break;

		inTotalDay -= inDayOfMon[inRTC_Mon - 1];
	}

	//計算inDay
	inRTC_Mday = inTotalDay;

	//計算inHour
	inRTC_Hour = (int )(ulTolSecond / 3600);//一小時有3600秒
	ulTolSecond %= 3600;//不足一小時的總秒數

	//計算inMin
	inRTC_Min = (int )(ulTolSecond / 60);//一分鐘有60秒

	//計算inSec
	inRTC_Sec = (int )(ulTolSecond % 60);//不足一分鐘的總秒數

	sprintf(tmpUnix,"%04d%02d%02d%02d%02d%02d", inRTC_Year, inRTC_Mon, inRTC_Mday, inRTC_Hour, inRTC_Min, inRTC_Sec);

	memcpy(bDate,tmpUnix,inSize);

	return 0;
}



