#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include "curl/curl.h"
#include <sqlite3.h>

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
#include "../FUNCTION/Sqlite.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/HDPT.h"
#include "../FUNCTION/CPT.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/Accum.h"

#include "TMSTABLE/EDCtmsFTPFLT.h"
#include "TMSTABLE/TmsFTP.h"
#include "TMSTABLE/TmsCPT.h"
#include "TMSTABLE/TmsSCT.h"

#include "../FUNCTION/FILE_FUNC/File.h"

#include "../FUNCTION/AccountFunction/PrintBillInfo.h"

#include "EDCTmsDefine.h"
#include "EDCTmsFlow.h"



extern	int	ginMachineType;
extern	char	gszTermVersionID[15 + 1];
extern	char	gszTermVersionDate[16 + 1];

/*
Function        : inEDCTMS_PRINT_PrintDownloadStatus
Date&Time   : 2019/10/1 下午 4:23
Describe        : 連線到TMS主機, 目前只能使用FTP下載
 *	因為 FUNCTION 會使用在OPT,所以回傳值只能回傳成功，否則會影響到後續的流程
 * inEDCTMS_FuncResultHandle 列印的部份移到這裏
 * 
*/
int inEDCTMS_PRINT_PrintDownloadStatus(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = 0, i = 0, inHeight = 0;
	int     k = 0, inFileCnt = 0, inSizes = 0;
	unsigned char uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	char szTemplate[60 + 1];
	char szSN[8 + 1];
	char szPrintBuf[384 + 1];
	char szFileName[60 + 1];
	char szFilePath[60 + 1];
	
	BufferHandle srBhandle;
	FONT_ATTRIB srFont_Attrib;
	
	VS_BOOL fDWLSuccess = VS_SUCCESS;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	        
	/*----列印參數表單  Start ----*/
	if (inFunc_Check_Print_Capability(ginMachineType) == VS_SUCCESS)
	{
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

		/* 印TMS下載狀態的圖 */
		inPRINT_Buffer_GetHeight((unsigned char*)"TMS_STATUS.bmp", &inHeight);

		if (inPRINT_Buffer_PutGraphic((unsigned char*)_TMS_DOWNLOAD_STATUS_, uszBuffer, &srBhandle, inHeight, _APPEND_) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie("  TMS Print Pic [%s] *Error* LINE[%d]",_TMS_DOWNLOAD_STATUS_, __LINE__);
			return (VS_ERROR);
		}

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		/* 列印時間 */
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inCREDIT_PRINT_Printing_Time(pobTran, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle);

		/* Get商店代號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetMerchantID(szTemplate);

		/* 列印商店代號 */
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_LEFT_);
		sprintf(szPrintBuf, "商店代號 %s", szTemplate);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* Get端末機代號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTerminalID(szTemplate);

		/* 列印端末機代號 */
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 13, _PADDING_LEFT_);
		sprintf(szPrintBuf, "端末機代號 %s", szTemplate);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 分隔線 */
		inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);


		/* Terminal AP Name */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		if (strlen(gszTermVersionID) > 0)
		{
			memcpy(szTemplate, gszTermVersionID, strlen(gszTermVersionID));
		}
		else
		{
			inGetTermVersionID(szTemplate);
		}
		sprintf(szPrintBuf, "VERSION ID = %s", szTemplate);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* Terminal DATE */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		if (strlen(gszTermVersionDate) > 0)
		{
			 memcpy(szTemplate, gszTermVersionDate, strlen(gszTermVersionDate));
		}
		else
		{
			  inGetTermVersionID(szTemplate);
		}
	
		sprintf(szPrintBuf, "VERSION DATE = %s", szTemplate);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* Terminal S/N */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		/* 取後8碼，但最後一碼為CheckSum，所以取8~15 */
		inFunc_GetSeriaNumber(szTemplate);
		memset(szSN, 0x00, sizeof(szSN));
		memcpy(szSN, &szTemplate[7], 8);
		sprintf(szPrintBuf, "TERMINAL S/N = %s", szSN);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, "下載檔案清單：(依下清單列示)");
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, "　　　　　　　 成功　　失敗");
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);


		/* 列印TMS下載結果 */
		for (i = 0;; i++)
		{
			/* 下載結果已經存在File Index，取得檔名後直接列印結果 */
			if (inLoadEDCFTPFLTRec(i) < 0)
				break;

			memset(szFilePath, 0x00, sizeof(szFilePath));
			inGetEDCFTPFilePath(szFilePath);
			memset(szFileName, 0x00, sizeof(szFileName));

			k = strlen(szFilePath);
			inSizes = strlen(szFilePath);
			inFileCnt = 0;
			while (1)
			{
				if (szFilePath[inSizes --] == 0x2F)
					break;

				inFileCnt ++;
			}

			inFileCnt --;
			memcpy(&szFileName[0], &szFilePath[k - inFileCnt], inFileCnt);                  

			inFunc_PAD_ASCII(szFileName, szFileName, ' ', 16, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, szFileName);

			/* 取得下載結果 Y or N */
			inGetEDCFTPFileIndex(szTemplate);

			if (!memcmp(&szTemplate[0], "Y", 1))
				strcat(szPrintBuf, "●");
			else
			{
				strcat(szPrintBuf, "　　　　●");
				fDWLSuccess = VS_ERROR;
			}                

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}

		if (fDWLSuccess == VS_SUCCESS)
		{
			inPRINT_Buffer_GetHeight((unsigned char*)"TMS_SUCCESS.bmp", &inHeight);

			if (inPRINT_Buffer_PutGraphic((unsigned char*)_TMS_DOWNLOAD_SUCCESS_, uszBuffer, &srBhandle, inHeight, _APPEND_) != VS_SUCCESS)
				return (VS_ERROR);
		}
		else
		{
			inPRINT_Buffer_GetHeight((unsigned char*)"TMS_FAILURE.bmp", &inHeight);

			if (inPRINT_Buffer_PutGraphic((unsigned char*)_TMS_DOWNLOAD_FAILURE_, uszBuffer, &srBhandle, inHeight, _APPEND_) != VS_SUCCESS)
				return (VS_ERROR);
		}

		for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);

		/*----列印參數表單  End ----*/
	}
	
	return (VS_SUCCESS);
}

/*
Function        : inEDCTMS_PRINT_PrintScheduleMessage
Date&Time   : 2017/1/25 上午 10:16
Describe        : 預告排程時間和排程自動下載完成都使用這一隻，
 *		 1.提示時間在inStatus填入_TMS_PRT_SCHEDULE_NOFTFY_
 *		 2.提示下載完成在inStatus填入_TMS_PRT_SCHEDULE_SUCCESS_
 *  目前沒在用 20190620 [SAM]
 * inEDCTMS_PRINT_ScheduleMessage
*/
int inEDCTMS_PRINT_PrintScheduleMessage(TRANSACTION_OBJECT *pobTran, int inStatus)
{
	int			inRetVal;
	char			szPrintBuf[42 + 1], szTemplate[42 + 1], szDateTime[15 + 1];
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* 初始化 */
	inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

	if (inLoadHDTRec(0) < 0)			/* 主機參數檔【HostDef.txt】 */
	{
		return (VS_ERROR);
	}
	if (inLoadHDPTRec(0) < 0)
	{
		return (VS_ERROR);
	}

	/* 重要訊息通知 */
	inRetVal = inCREDIT_PRINT_Schedule_ByBuffer(pobTran, uszBuffer, &srFont_Attrib, &srBhandle);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 列印時間 */
	inRetVal = inCREDIT_PRINT_Printing_Time(pobTran, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle);
	if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);

	/* Print NCCC LOGO 384*60 */
	inRetVal = inCREDIT_PRINT_Logo_ByBuffer(pobTran, uszBuffer, &srFont_Attrib, &srBhandle);
	if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);

	inRetVal = inCREDIT_PRINT_Tidmid_ByBuffer(pobTran, uszBuffer, &srFont_Attrib, &srBhandle);
	if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);

	/* 自動排程下載時間提示 */
	if (inStatus == _EDC_TMS_PRT_SCHEDULE_NOFTFY_)
	{
		/* 排程日期 */
		memset(szDateTime, 0x00, sizeof(szDateTime));
		inGetTMSScheduleDate(szDateTime);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &szDateTime[0], 4);
		strcat(szTemplate, "/");
		memcpy(&szTemplate[5], &szDateTime[4], 2);
		strcat(szTemplate, "/");
		memcpy(&szTemplate[8], &szDateTime[6], 2);
		szTemplate[10] = ' ';
	
		/* 排程時間 */
		memset(szDateTime, 0x00, sizeof(szDateTime));
		inGetTMSScheduleTime(szDateTime);
		
		memcpy(&szTemplate[11], &szDateTime[0], 2);
		strcat(szTemplate, ":");
		memcpy(&szTemplate[14], &szDateTime[2], 2);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "本機將於 %s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("進行資料自動更新作業，屆時【請勿關機】。", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("此為系統自動化作業，無需人員操作及留守，", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("僅需注意於上述作業時間本機必須為【開機】", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("狀態即可。作業開始時，本機螢幕會顯示【", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("資料更新中，請勿關機】之訊息。", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	/* 自動排程下載完成提示*/
	else if (inStatus == _EDC_TMS_PRT_SCHEDULE_SUCCESS_)
	{
		inPRINT_Buffer_PutIn("本機【已完成自動下載作業】，請確認目前螢幕", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("顯示為聯合信用卡中心待機畫面即可開始使用。", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	inRetVal = inCREDIT_PRINT_End_ByBuffer(pobTran, uszBuffer, &srFont_Attrib, &srBhandle);
	if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);

	inRetVal = inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
	if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (VS_SUCCESS);
}

/*
Function        : inEDCTMS_PRINT_PrintReturnTask
Date&Time   : 2019/10/14 下午 3:17
Describe        :列印TMS至現回報簽單
 * 目前沒在用 20190620 [SAM]
 * inEDCTMS_PRINT_Return_Task
*/
int inEDCTMS_PRINT_PrintReturnTask(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal, i = 0, inHeight = 0;
        char			szTemplate[60 + 1];
        char			szPrintBuf[384 + 1];
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);
        /* 列印下載結果 */
	/* 至現回報簽單 */
	inPRINT_Buffer_GetHeight((unsigned char*)"TASK.bmp", &inHeight);

	/* Title 至現回報 */
	if (inPRINT_Buffer_PutGraphic((unsigned char*)_TMS_TASK_TITLE_, uszBuffer, &srBhandle, inHeight, _APPEND_) != VS_SUCCESS)
		return (VS_ERROR);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

	/* 列印時間 */
	inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	inCREDIT_PRINT_Printing_Time(pobTran, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle);

	/* Get商店代號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetMerchantID(szTemplate);

	/* 列印商店代號 */
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_LEFT_);
	sprintf(szPrintBuf, "商店代號 %s", szTemplate);/* 新增一隔空白對齊 */
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* Get端末機代號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);

	/* 列印端末機代號 */
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 13, _PADDING_LEFT_);
	sprintf(szPrintBuf, "端末機代號 %s", szTemplate);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 分隔線 */
	inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* Terminal AP Name */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	if (strlen(gszTermVersionID) > 0)
	{
		memcpy(szTemplate, gszTermVersionID, strlen(gszTermVersionID));
	}
	else
	{
		inGetTermVersionID(szTemplate);
	}
	sprintf(szPrintBuf, "VERSION ID = %s", szTemplate);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* Terminal AP Version */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	inGetTermVersionDate(szTemplate);
	sprintf(szPrintBuf, "VERSION DATE = %s", szTemplate);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	/* BATCH NO. */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcpy(szPrintBuf, "BATCH NO. = ");
	//strcat(szPrintBuf, gsrTMS_Field58.szBatchNumber);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	inPRINT_Buffer_GetHeight((unsigned char*)"END.bmp", &inHeight);

	if (inPRINT_Buffer_PutGraphic((unsigned char*)_END_LOGO_, uszBuffer, &srBhandle, inHeight, _APPEND_) != VS_SUCCESS)
		return (VS_ERROR);

	for (i = 0; i < 8; i++)
	{
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (VS_SUCCESS);
}


