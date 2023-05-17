#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include "curl/curl.h"
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/Transaction.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DispMsg.h"
#include "../DISPLAY/DisTouch.h"
#include "../PRINT/Print.h"
#include "../PRINT/PrtMsg.h"

#include "../FUNCTION/Function.h"
#include "../FUNCTION/EDC.h"

#include "TMSTABLE/TmsCPT.h"
#include "TMSTABLE/TmsSCT.h"
#include "TMSTABLE/TmsFTP.h"

#include "EDCTmsFlow.h"
#include "EDCTmsScheduleFunc.h"


extern char	gszTermVersionDate[16 + 1] ;	/* 暫存的TerminalVersionDate，重開機要還原(測試導向功能) */;

//static char gzTmepDate[9] = {0};
//static char gzTmepTime[7] = {0};


/*
Function        : inEDCTMS_SCHDULE_CheckEffectiveDateTime
Date&Time   : 2016/2/1 上午 9:57
Describe        : 參數生效日期時間確認, 目前用不到
 * inEDCTMS_Schedule_Effective_Date_Time_Check
*/
int inEDCTMS_SCHDULE_CheckEffectiveDateTime(TRANSACTION_OBJECT * pobTran)
{
	int		inSecond = 0, inEffectiveSecond = 0;
	char		szDate[8 + 1], szTime[6 + 1];
	char		szEffectiveDate[8 + 1], szEffectiveTime[6 + 1];
	char		szTemplate[16 + 1];
	RTC_NEXSYS	srRTC;
	VS_BOOL		fDateCheck = VS_FALSE, fTimeCheck = VS_FALSE;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	if (inEDCTMS_CheckTmsFileListExistFlow(pobTran) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  TMS Schudle Load FileList不存在  Line[%d]",__LINE__);
		return (VS_ERROR);
	}
        
	if (inLoadTMSCPTRec(0) < 0)
	{
		inDISP_DispLogAndWriteFlie("  TMS Schudle Load TSMCPT *Error* Line[%d]",__LINE__);
		return (VS_ERROR);
	}
        
	memset(szEffectiveDate, 0x00, sizeof(szEffectiveDate));
	memset(szEffectiveTime, 0x00, sizeof(szEffectiveTime));

	/* 取得參數生效日期 */
	if (inGetTMSEffectiveDate(szEffectiveDate) < 0)
	{
		inDISP_DispLogAndWriteFlie("  參數生效日期取得失敗");
		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag("  Effective Date = %s", szEffectiveDate);

	/* 參數立即生效 */
	if (!memcmp(&szEffectiveDate[0], "00000000", 8))
	{
		inDISP_DispLogAndWriteFlie("  參數立即生效");
		return (VS_SUCCESS);
	}

	/* 取得參數生效時間 */
	if (inGetTMSEffectiveTime(szEffectiveTime) < 0)
	{
		inDISP_DispLogAndWriteFlie("  參數生效時間取得失敗");
		return (VS_ERROR);
	}
        
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  端末機時間取得失敗");
		return (VS_ERROR);
	}
	
	memset(szDate, 0x00, sizeof(szDate));
	sprintf(szDate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
	memset(szTime, 0x00, sizeof(szTime));
	sprintf(szTime, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);

	/* 端末機秒數 */
	inSecond = ((srRTC.uszHour) * 3600) + ((srRTC.uszMinute) * 60) + srRTC.uszSecond;
        
	/* 生效時間秒數 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szEffectiveTime[0], 2);
	inEffectiveSecond = atoi(szTemplate) * 3600;
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szEffectiveTime[2], 2);
	inEffectiveSecond = inEffectiveSecond + (atoi(szTemplate) * 60);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szEffectiveTime[4], 2);
	inEffectiveSecond = inEffectiveSecond + atoi(szTemplate);
        
	if (!memcmp(&szDate[0], &szEffectiveDate[0], 8))
	{
		fDateCheck = VS_TRUE;
	}
        
	/* 端末機時間大於生效時間 */
	if (inSecond >= inEffectiveSecond)
	{
		/* 小於生效時間3分鐘內，一直跳(因為Idle每五秒檢查一次，所以這裡每五秒跳一次) */
		if ((inSecond - inEffectiveSecond) < 60 * 3)
		{
			fTimeCheck = VS_TRUE;
		}
		/* 大於生效時間每2分鐘跳一次 */
		else
		{
			if ((srRTC.uszMinute % 2 == 0) && (srRTC.uszSecond == 0))
			{
				fTimeCheck = VS_TRUE;
			}
		}

	}
        
	inDISP_DispLogAndWriteFlie("  DateCheck[%d]  TimeCheck [%d]  ", fDateCheck, fTimeCheck);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	if (fDateCheck == VS_TRUE && fTimeCheck == VS_TRUE)
		return (VS_SUCCESS);
	else
		return (VS_ERROR);
}

/*
Function        : inEDCTMS_SCHEDULE_CheckInquireDateTime
Date&Time   : 2017/1/25 下午 2:49
Describe        : 詢問時間檢查
 * 檢查儲存的日期與 TmsCPT 的日期，如果符合規格才可進行 SYSCONFIG 下載
 * inEDCTMS_Schedule_Inquire_Date_Time_Check
*/
int inEDCTMS_SCHEDULE_CheckInquireDateTime()
{
	int	inRetVal;
	int	inScheduleDay, inTermDay;
	int	inScheduleMinute, inTermMinute;
	char	szTMSInquireStartDate[8 + 1], szTMSInquireTime[6 + 1];
	char	szTMSSctDate[8 + 1], szTMSSctTime[6 + 1];
	char	szTermDate[8 + 1], szTermTime[6 + 1], szHour[2 + 1], szMinute[2 + 1];
	VS_BOOL fDate = VS_FALSE, fTime = VS_FALSE;
	RTC_NEXSYS srRTC; /* Date & Time */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inRetVal = inLoadTMSFTPRec(0);
	if(inRetVal != VS_SUCCESS){
		inDISP_LogPrintfWithFlag("  Compare Load TmsFTP *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	inRetVal = inLoadTMSSCTRec(0);
	if(inRetVal != VS_SUCCESS){
		inDISP_DispLogAndWriteFlie("  SaveSys Load TMSSCT *Error* Line[%d]", __LINE__);
		return (VS_SUCCESS);
	}
	
	
	memset(szTMSSctDate, 0x00, sizeof(szTMSSctDate));
	memset(szTMSSctTime, 0x00, sizeof(szTMSSctTime));
	
	inGetTMSInquireStartDate(szTMSSctDate);		
	inGetTMSInquireTime(szTMSSctTime);
		
	/* 參數詢問生效日 */
	memset(szTMSInquireStartDate, 0x00, sizeof(szTMSInquireStartDate));
	inGetFTPInquireStartDate(szTMSInquireStartDate);	
	/* 參數詢問時間 */
	memset(szTMSInquireTime, 0x00, sizeof(szTMSInquireTime));
	inGetFTPInquireStartTime(szTMSInquireTime);
	inDISP_LogPrintfWithFlag("  SCT INQ Date[%s] LINE[%d]",szTMSSctDate, __LINE__);
	inDISP_LogPrintfWithFlag("  FTP INQ Date[%s] LINE[%d]",szTMSInquireStartDate, __LINE__);
	
	inDISP_LogPrintfWithFlag("  SCT INQ TIME [%s] LINE[%d]",szTMSSctTime, __LINE__);
	inDISP_LogPrintfWithFlag("  FTP INQ TIME [%s] LINE[%d]",szTMSInquireTime, __LINE__);
	
	/* 如果二個參數都一樣就不需要進行資料下載 */
	if (!memcmp(szTMSInquireStartDate, szTMSSctDate, 8) && (!memcmp(szTMSInquireTime, szTMSSctTime, 6)))
	{
		inDISP_LogPrintfWithFlag(" Date And Time Are The Same LINE[%d]", __LINE__);
		return (VS_ERROR);
	}
		
	/* 取得EDC時間日期 */
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Get System Time *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}
	memset(szTermDate, 0x00, sizeof(szTermDate));
	memset(szTermTime, 0x00, sizeof(szTermTime));
	
	sprintf(szTermDate, "20%02d%02d%02d",  srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
	sprintf(szTermTime, "%02d%02d%02d",  srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);

	inDISP_DispLogAndWriteFlie("  TRM INQ Date[%s] LINE[%d]",szTermDate, __LINE__);
	inDISP_DispLogAndWriteFlie("  SCT INQ Date[%s] LINE[%d]",szTMSSctDate, __LINE__);
	
	inDISP_DispLogAndWriteFlie("  TRM INQ TIME [%s] LINE[%d]",szTermTime, __LINE__);
	inDISP_DispLogAndWriteFlie("  SCT INQ TIME [%s] LINE[%d]",szTMSSctTime, __LINE__);
	
	/* 目前公司TMS初始日期會是 00000 所沒有立既下載的事件 2019/12/11 上午 9:59 [SAM] */
//	if (!memcmp(szTermDate, "00000000", 8))
//	{
//		inDISP_LogPrintfWithFlag("  Inquire Date is 00000000 Line[%d]", __LINE__);
//		return (VS_ERROR);
//	}
//	else
	{
		/*  使用 TmsSCT 的時間來判斷是否要下載  */
		/* 太陽日運算 */
		inTermDay = inFunc_SunDay_Sum(szTermDate);
		inScheduleDay = inFunc_SunDay_Sum(szTMSSctDate);

		if (inTermDay >= inScheduleDay)
		{
			/* 如果日期相同才需要比對時間  */
			if(inTermDay == inScheduleDay)
			{
				memset(szHour, 0x00, sizeof(szHour));
				memcpy(&szHour[0], &szTMSSctTime[0], 2);
				memset(szMinute, 0x00, sizeof(szMinute));
				memcpy(&szMinute[0], &szTMSSctTime[2], 2);
				inScheduleMinute = atoi(szHour) * 60 + atoi(szMinute);

				memset(szHour, 0x00, sizeof(szHour));
				memcpy(&szHour[0], &szTermTime[0], 2);
				memset(szMinute, 0x00, sizeof(szMinute));
				memcpy(&szMinute[0], &szTermTime[2], 2);
				inTermMinute = atoi(szHour) * 60 + atoi(szMinute);

				//if ((inTermMinute >= inScheduleMinute) && ((inTermMinute - inScheduleMinute) <= 15))
				if (inTermMinute >= inScheduleMinute)
				{
					fTime = VS_TRUE;
				}
				else
				{
					fTime = VS_FALSE;
				}
			}else{
				fTime = VS_TRUE;
			}
			
			fDate = VS_TRUE;
		}
		else
		{
			fDate = VS_FALSE;
		}

				
//		/* 目前公司TMS初始日期會是 00000 所沒有立既下載的事件 2019/12/11 上午 9:59 [SAM] */
//		if (!memcmp(szTermTime, "000000", 6))
//		{
//			inDISP_LogPrintfWithFlag("  Inquire Time is 00000000 Line[%d]", __LINE__);
//			return (VS_ERROR);
//		}
//		else
//		{
//			memset(szHour, 0x00, sizeof(szHour));
//			memcpy(&szHour[0], &szTMSInquireTime[0], 2);
//			memset(szMinute, 0x00, sizeof(szMinute));
//			memcpy(&szMinute[0], &szTMSInquireTime[2], 2);
//			inScheduleMinute = atoi(szHour) * 60 + atoi(szMinute);
//
//			memset(szHour, 0x00, sizeof(szHour));
//			memcpy(&szHour[0], &szTermTime[0], 2);
//			memset(szMinute, 0x00, sizeof(szMinute));
//			memcpy(&szMinute[0], &szTermTime[2], 2);
//			inTermMinute = atoi(szHour) * 60 + atoi(szMinute);
//
//			if ((inTermMinute >= inScheduleMinute) && ((inTermMinute - inScheduleMinute) <= 15))
//			{
//				fTime = VS_TRUE;
//			}
//			else
//			{
//				fTime = VS_FALSE;
//			}
//		}

		inDISP_LogPrintfWithFlag("  fDate[%d]  fTime [%d]  ", fDate, fTime);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		
		if ((fDate == VS_TRUE) && (fTime == VS_TRUE))
			return (VS_SUCCESS);
		else
			return (VS_ERROR);
	}
}



/*
Function        : inEDCTMS_SCHEDULE_CheckUpdteFptInquireDateTime
Date&Time   : 2019/12/13 上午 9:48
Describe        :  傳入 TmsSCT 的時間，檢查是否大於端末機時間
 *
*/
int inEDCTMS_SCHEDULE_CheckUpdteFptInquireDateTime(char *szTMSInquireStartDate, char *szTMSInquireTime)
{
	int	inScheduleDay, inTermDay;
	int	inScheduleMinute, inTermMinute;
	char	szTermDate[8 + 1], szTermTime[6 + 1], szHour[2 + 1], szMinute[2 + 1];
	VS_BOOL fUpdate = VS_FALSE;
	RTC_NEXSYS srRTC; /* Date & Time */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 取得EDC時間日期 */
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie("  Get System Time *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}
	
	memset(szTermDate, 0x00, sizeof(szTermDate));
	memset(szTermTime, 0x00, sizeof(szTermTime));
	sprintf(szTermDate, "20%02d%02d%02d",  srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
	sprintf(szTermTime, "%02d%02d%02d",  srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);

	
	/* 參數詢問生效日 */
	inDISP_LogPrintfWithFlag("  TERM Date[%s] LINE[%d]",szTermDate, __LINE__);
	inDISP_LogPrintfWithFlag("  SCT INQ Date[%s] LINE[%d]",szTMSInquireStartDate, __LINE__);
	
	/* 目前公司TMS初始日期會是 00000 所沒有立既下載的事件 2019/12/11 上午 9:59 [SAM] */
	if (!memcmp(szTMSInquireStartDate, "00000000", 8))
	{
		inDISP_LogPrintfWithFlag("  Inquire Date is 00000000 Line[%d]", __LINE__);
		return (VS_ERROR);
	}
	else
	{
		/* 太陽日運算 */
		inTermDay = inFunc_SunDay_Sum(szTermDate);
		inScheduleDay = inFunc_SunDay_Sum(szTMSInquireStartDate);
		/* TMS 主機上的時間大於端未時間時，就需要更新到 TmsFTP  */
		if (inScheduleDay >= inTermDay )
		{
			fUpdate = VS_TRUE;
			
			/* 如果日期相同才需要比對時間 */
			if (inScheduleDay == inTermDay )
			{
				/* 參數詢問時間 */
				inDISP_LogPrintfWithFlag("  TERM INQ TIME [%s] LINE[%d]",szTermTime, __LINE__);
				inDISP_LogPrintfWithFlag("  SCT INQ TIME [%s] LINE[%d]",szTMSInquireTime, __LINE__);
				/* 目前公司TMS初始日期會是 00000 所沒有立既下載的事件 2019/12/11 上午 9:59 [SAM] */
				if (!memcmp(szTMSInquireTime, "000000", 6))
				{
					inDISP_LogPrintfWithFlag("  Inquire Time is 00000000 Line[%d]", __LINE__);
					return (VS_ERROR);
				}
				else
				{
					memset(szHour, 0x00, sizeof(szHour));
					memcpy(&szHour[0], &szTMSInquireTime[0], 2);
					memset(szMinute, 0x00, sizeof(szMinute));
					memcpy(&szMinute[0], &szTMSInquireTime[2], 2);
					inScheduleMinute = atoi(szHour) * 60 + atoi(szMinute);

					memset(szHour, 0x00, sizeof(szHour));
					memcpy(&szHour[0], &szTermTime[0], 2);
					memset(szMinute, 0x00, sizeof(szMinute));
					memcpy(&szMinute[0], &szTermTime[2], 2);
					inTermMinute = atoi(szHour) * 60 + atoi(szMinute);

					if(inScheduleMinute >= inTermMinute ) 
					{
						fUpdate = VS_TRUE;
					}
					else
					{
						fUpdate = VS_FALSE;
					}
				}			
			}
		}
		else
		{
			fUpdate = VS_FALSE;
		}

		
		inDISP_LogPrintfWithFlag("  Check Ftp fUpdate[%d] ", fUpdate);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		
		if (fUpdate == VS_TRUE )
		{
			return (VS_SUCCESS);
		}else
		{
			return (VS_ERROR);
		}
	}
}




/*
Function        : inEDCTMS_SCHEDULE_CheckDownloadDateTime
Date&Time       :2017/1/25 下午 1:20
Describe        :參考Verifone機型inNCCC_TMS_CheckSchedule()，查看是否已到排程時間
 *	目前不使用 20190619 [SAM ]
 * inEDCTMS_Schedule_Download_Date_Time_Check
 * 
*/
int inEDCTMS_SCHEDULE_CheckDownloadDateTime(TRANSACTION_OBJECT * pobTran)
{
	int		inSecond = 0, inScheduleSecond = 0;
	char		szDate[8 + 1], szTime[6 + 1];
	char		szScheduleDate[8 + 1], szScheduleTime[6 + 1];
	char		szTemplate[16 + 1];
	RTC_NEXSYS	srRTC;
	VS_BOOL		fDateCheck = VS_FALSE, fTimeCheck = VS_FALSE;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

        
	if (inLoadTMSCPTRec(0) < 0)
	{
		inDISP_LogPrintfWithFlag("  inLoadTMSCPTRec(0) Error !!");
	}
        
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTMSDownloadFlag(szTemplate);
	
	/* 只有設定為排程下載時才繼續往下做*/
	if (memcmp(szTemplate, _TMS_DOWNLOAD_FLAG_SCHEDULE_, strlen(_TMS_DOWNLOAD_FLAG_SCHEDULE_)) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSScheduleRetry(szTemplate);
		if (atoi(szTemplate) >= 3)
		{
			/* 取消排程 */
			inResetTMSCPT_Schedule();
			
			/* 超過時間，刪除所有下載檔案 */
			inEDCTMS_DeleteAllDownloadFileMethodFlow(pobTran);
			
			/* 下載失敗 DCC 排程時間也要歸零 */
			inSetDCCDownloadMode("0");
			inSaveEDCRec(0);
		}
		
		memset(szScheduleDate, 0x00, sizeof(szScheduleDate));
		memset(szScheduleTime, 0x00, sizeof(szScheduleTime));
        
		/* 取得排程下載日期 */
		if (inGetTMSScheduleDate(szScheduleDate) < 0)
		{
			inDISP_LogPrintfWithFlag("  排程下載日期取得失敗");
			return (VS_ERROR);
		}

		/* 取得排程下載時間 */
		if (inGetTMSScheduleTime(szScheduleTime) < 0)
		{
			inDISP_LogPrintfWithFlag("  排程下載時間取得失敗");
                
			return (VS_ERROR);
		}

		if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
		{
			inDISP_LogPrintfWithFlag("  端未機時間取得失敗");
			return (VS_ERROR);
		}
		
		memset(szDate, 0x00, sizeof(szDate));
		sprintf(szDate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		memset(szTime, 0x00, sizeof(szTime));
		sprintf(szTime, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);

		/* 端末機秒數 */
		inSecond = ((srRTC.uszHour) * 3600) + ((srRTC.uszMinute) * 60) + srRTC.uszSecond;

		/* 生效時間秒數 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &szScheduleTime[0], 2);
		inScheduleSecond = atoi(szTemplate) * 3600;
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &szScheduleTime[2], 2);
		inScheduleSecond = inScheduleSecond + (atoi(szTemplate) * 60);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &szScheduleTime[4], 2);
		inScheduleSecond = inScheduleSecond + atoi(szTemplate);

		if (!memcmp(&szDate[0], &szScheduleDate[0], 8))
		{
			fDateCheck = VS_TRUE;
		}

		/* 端末機時間大於排程時間 */
		if (inSecond >= inScheduleSecond)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetTMSScheduleRetry(szTemplate);
			
			/* 超過排程時間，且小於15分鐘內才下載 或者 超過15分鐘但未嘗試下載，則至少下載一次 */
			if ((inSecond - inScheduleSecond) < (15 * 60) || atoi(szTemplate) == 0)
			{
				/* 小於生效時間3分鐘內，一直跳(因為Idle每五秒檢查一次，所以這裡每五秒跳一次) */
				if ((inSecond - inScheduleSecond) < 60 * 3)
				{
					fTimeCheck = VS_TRUE;
				}
				/* 大於生效時間每2分鐘跳一次 */
				else
				{
					if ((srRTC.uszMinute % 2 == 0))
					{
						fTimeCheck = VS_TRUE;
					}
				}
				
				inDISP_LogPrintfWithFlag("  fDateCheck[%d]  fTimeCheck [%d]  ", fDateCheck, fTimeCheck);
				inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
				
				if (fDateCheck == VS_TRUE && fTimeCheck == VS_TRUE)
					return (VS_SUCCESS);
				else
					return (VS_ERROR);
			}
			else
			{
				/* 取消排程 */
				inResetTMSCPT_Schedule();
				/* 超過時間，刪除所有下載檔案 */
				inEDCTMS_DeleteAllDownloadFileMethodFlow(pobTran);
				
				inDISP_LogPrintfWithFlag("  取消排程刪除檔案 ");
				return (VS_ERROR);
			}

		}
		else
		{
			/* 時間未到 */
			return (VS_ERROR);
		}

	}
	else
	{
		inDISP_LogPrintfWithFlag("  非排程下載不檢核時間");
		return (VS_ERROR);
	}
	
        
}

/*
Function        : inEDCTMS_SCHEDULE_ResetDownloadDateTime
Date&Time   : 2019/12/8 下午 2:09
Describe        : 目前用在重設 TMSFTP 裏面的排程比對時間, 但在外部需要儲存 TmsFTP 
 * 
*/
int inEDCTMS_SCHEDULE_ResetDownloadDateTime(TRANSACTION_OBJECT * pobTran)
{
	inSetFTPInquireStartDate("00000000");
	inSetFTPInquireStartTime("000000");
	
	inSetTMSInquireStartDate("00000000");
	inSetTMSInquireTime("000000");
	
	return VS_SUCCESS;
}

/*
Function        : inEDCTMS_SCHEDULE_CheckApVersionDate
Date&Time   : 2019/12/11 下午 2:38
Describe        : 詢問時間檢查
 * 檢查主機下載的 VersionDate 儲存的日期，比對 TmsCPT 的日期，如果符合規格才可進行 AP 下載
 * 
*/
int inEDCTMS_SCHEDULE_CheckApVersionDate()
{
	int	inRetVal;
	int	inScheduleDay, inTermDay;
	char	szFtpVersionDate[8 + 1];
	VS_BOOL fDate = VS_FALSE;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 讀出從主機下載的參數 */
	inRetVal = inLoadTMSFTPRec(0);
	if(inRetVal != VS_SUCCESS){
		inDISP_DispLogAndWriteFlie("  Compare Load TmsFTP *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}
	
	/* 抓取 TmsSCT 日期來比對  */
	memset(szFtpVersionDate, 0x00,sizeof(szFtpVersionDate));
	if( VS_SUCCESS !=inGetFTPApVersionDate(szFtpVersionDate))
	{
		inDISP_DispLogAndWriteFlie("  Get FTP Ap Version Date *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	inDISP_DispLogAndWriteFlie("  Check Ap  Gdate[%s]  Ftpdate[%s] Line[%d]",gszTermVersionDate, szFtpVersionDate, __LINE__);
	
	/* 目前公司TMS初始日期會是 00000000 所沒有立既下載的事件 2019/12/11 上午 9:59 [SAM] */
	if (!memcmp(&szFtpVersionDate[0], "00000000", 8))
	{
		inDISP_LogPrintfWithFlag("  Inquire Date is 00000000 Line[%d]", __LINE__);
		return (VS_ERROR);
	}
	else
	{
		/* 太陽日運算 */
		/* 抓取EDC 程式版本固定日期 */
		inTermDay = inFunc_SunDay_Sum(gszTermVersionDate);
		
		/* 抓取FTP 下載的AP 日期 */
		inScheduleDay = inFunc_SunDay_Sum(szFtpVersionDate);
		
		/* 主機 AP VERSION DATE 大於 EDC版本的時間就要進行AP下載 2019/12/11 下午 2:46 [SAM] */
		if (inScheduleDay > inTermDay )
		{
			fDate = VS_TRUE;
		}
		else
		{
			fDate = VS_FALSE;
		}

		
		inDISP_DispLogAndWriteFlie("  Term[%s] FTP[%s] fDate[%d] Line[%d]",gszTermVersionDate, szFtpVersionDate, fDate, __LINE__);
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		
		if ((fDate == VS_TRUE) )
			return (VS_SUCCESS);
		else
			return (VS_ERROR);
	}
}



