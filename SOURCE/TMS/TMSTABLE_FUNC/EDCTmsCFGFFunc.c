#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include "curl/curl.h"

#include "../../INCLUDES/Define_1.h"
#include "../../INCLUDES/Define_2.h"
#include "../../INCLUDES/TransType.h"
#include "../../INCLUDES/Transaction.h"
#include "../../DISPLAY/Display.h"
#include "../../DISPLAY/DispMsg.h"
#include "../../DISPLAY/DisTouch.h"
#include "../../PRINT/Print.h"
#include "../../PRINT/PrtMsg.h"

#include "../../FUNCTION/Function.h"
#include "../../FUNCTION/CFGT.h"

#include "../../FUNCTION/FILE_FUNC/File.h"

#include "../../COMM/Comm.h"

#include "EDCTmsCFGFFunc.h"

extern int ginDebug; /* Debug使用 extern */

/*
Function        : inEDCTMS_Inital_CFGTRec
Date&Time   : 2015/8/31 下午 2:00
Describe        : 初始 CFGT Table 
 *	目前沒在用 20190620 [SAM]
 * inEDCTMS_Inital_CFGTRec
 */
int inEDCTMS_CFGF_InitalRec(void)
{
	unsigned long uldat_Handle; /* FILE Handle */
	int inPackCount = 0; /* uszWriteBuff_Record的index */
	int inRetVal;
	//      char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];	/* debug message */
	long lnCFGTLength = 0; /* CFGT.dat檔案總長度 */
	unsigned char *uszWriteBuff_Record;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inEDCTMS_Inital_CFGTRec() START!!");
	}

	/* 開啟原檔案CFGT.dat */
	inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *) _CFGT_FILE_NAME_);

	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 取得CFGT.dat檔案大小 */
	lnCFGTLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *) _CFGT_FILE_NAME_);

	/* 取得檔案大小失敗 */
	if (lnCFGTLength == VS_ERROR)
	{
		inFILE_Close(&uldat_Handle);
		return (VS_ERROR);
	}

	inFILE_Close(&uldat_Handle);

	/* 組Write Record封包 */
	/* 給WriteBuff記憶體大小 */
	uszWriteBuff_Record = malloc(_SIZE_CFGT_REC_ + _SIZE_CFGT_COMMA_0D0A_);
	memset(uszWriteBuff_Record, 0x00, _SIZE_CFGT_REC_ + _SIZE_CFGT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

	inPackCount = 0; /* uszWriteBuff_Record的index位置 */
	/* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* 設定預設值 */
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* BarCodeReaderEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], "N", 1);
	inPackCount += 1;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* EMVPINBypassEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], "Y", 1);
	inPackCount += 1;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* CUPOnlinePINEntryTimeout */
	memcpy(&uszWriteBuff_Record[inPackCount], "020", 3);
	inPackCount += 3;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* SignPadMode */
	memcpy(&uszWriteBuff_Record[inPackCount], "0", 1);
	inPackCount += 1;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* ESCPrintMerchantCopy */
	memcpy(&uszWriteBuff_Record[inPackCount], "N", 1);
	inPackCount += 1;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* ESCPrintMerchantCopyStartDate */
	memcpy(&uszWriteBuff_Record[inPackCount], "00000000", 8);
	inPackCount += 8;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* ESCPrintMerchantCopyEndDate */
	memcpy(&uszWriteBuff_Record[inPackCount], "00000000", 8);
	inPackCount += 8;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* ESCReciptUploadUpLimit */
	memcpy(&uszWriteBuff_Record[inPackCount], "000", 3);
	inPackCount += 3;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* ContactlessReaderMode */
	memcpy(&uszWriteBuff_Record[inPackCount], "1", 1);
	inPackCount += 1;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* TMSDownloadMode */
	memcpy(&uszWriteBuff_Record[inPackCount], "1", 1);
	inPackCount += 1;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* AMEXContactlessEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], "N", 1);
	inPackCount += 1;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* CUPContactlessEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], "Y", 1);
	inPackCount += 1;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* SmartPayContactlessEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], "N", 1);
	inPackCount += 1;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* Pay_Item_Enable */
	memcpy(&uszWriteBuff_Record[inPackCount], "N", 1);
	inPackCount += 1;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* Store_Stub_CardNo_Truncate_Enable */
	memcpy(&uszWriteBuff_Record[inPackCount], "Y", 1);
	inPackCount += 1;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* Integrate_Device */
	memcpy(&uszWriteBuff_Record[inPackCount], "N", 1);
	inPackCount += 1;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* FES_ID */
	memcpy(&uszWriteBuff_Record[inPackCount], "000", 3);
	inPackCount += 3;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* Integrate_Device_AP_ID */
	memcpy(&uszWriteBuff_Record[inPackCount], "               ", 15);
	inPackCount += 15;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* Short_Receipt_Mode */
	memcpy(&uszWriteBuff_Record[inPackCount], "N", 1);
	inPackCount += 1;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* I_FES_Mode */
	memcpy(&uszWriteBuff_Record[inPackCount], "N", 1);
	inPackCount += 1;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* DHCP_Mode */
	memcpy(&uszWriteBuff_Record[inPackCount], "N", 1);
	inPackCount += 1;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* ESVC_Priority */
	memcpy(&uszWriteBuff_Record[inPackCount], "    ", 4);
	inPackCount += 4;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* DFS_Contactless_Enable */
	memcpy(&uszWriteBuff_Record[inPackCount], "N", 1);
	inPackCount += 1;
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* Cloud_MFES */
	memcpy(&uszWriteBuff_Record[inPackCount], "N", 1);
	inPackCount += 1;

	/* 把預設值插在原檔案換行符號之前 -2 0x0D 0x0A */
	if (inFILE_Data_Insert((unsigned char*) _CFGT_FILE_NAME_, (lnCFGTLength - 2), inPackCount, (unsigned char*) uszWriteBuff_Record) != VS_SUCCESS)
	{
		/* 釋放記憶體 */
		free(uszWriteBuff_Record);
		return (VS_ERROR);
	}

	/* 釋放記憶體 */
	free(uszWriteBuff_Record);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inEDCTMS_Inital_CFGTRec() END!!");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        : inEDCTMS_CFGF_UpdateConfig
Date&Time   : 2015/8/31 下午 2:00
Describe        : 更新 CFGT Table 
 * inEDCTMS_Update_Config
 * 因為TMS下戴的 Config.txt 檔會比我們預設的 CFGT.dat內容個數少，所以只能挑出相對應的欄位做部份更新 [SAM] 2022/8/12 下午 2:36
 */
int inEDCTMS_CFGF_UpdateConfig(void)
{
	int i = 0, k = 0, j = 0;
	int inRetVal = VS_SUCCESS;
	int inParamCount = 0;
	int inRecCnt = 1;
	int inCtlsCondition = 0; /* 因公司的TMS 不會控管到VMJ的感應開關，所以新增此條件 20190213 [SAM]  */
	unsigned char *uszReadData;
//	unsigned char *uszTemp;
	unsigned long uldat_Handle; /* FILE Handle */
	long lnCFGTLength = 0; /* CFGT.dat檔案總長度 */
	char szRamRec[512];
	char szDebugLog[100];


	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	/* 開啟原檔案CFGT.dat */
	inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *) "Config.txt");

	if (inRetVal != VS_SUCCESS)
	{
		inDISP_DispLogAndWriteFlie(" Config.txt Open *Error* RetVal[%ld]", inRetVal);
		return (VS_ERROR);
	}

	/* 取得CFGT.dat檔案大小 */
	lnCFGTLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *) "Config.txt");

	/* 取得檔案大小失敗 */
	if (lnCFGTLength == VS_ERROR)
	{
		inFILE_Close(&uldat_Handle);
		inDISP_DispLogAndWriteFlie(" Config.txt lnFILE_GetSize *Error* Val[%ld]", lnCFGTLength);
		return (VS_ERROR);
	}

	/*
	 * allocate 記憶體
	 * allocate時多分配一個byte以防萬一（ex:換行符號）
	 */
	uszReadData = malloc(lnCFGTLength + 1);

	/* 初始化 uszTemp uszReadData */
	memset(uszReadData, 0x00, lnCFGTLength + 1);

	/* seek 到檔案開頭 & 從檔案開頭開始read */
	/* 指到第一個 BYTE */
	if ((inRetVal = inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_)) == VS_ERROR)
	{
		inFILE_Close(&uldat_Handle);
		free(uszReadData);
		inDISP_DispLogAndWriteFlie(" Config.txt inFILE_Seek *Error* Val[%ld]", inRetVal);
		return (VS_ERROR);
	}

	/* 讀檔案 */
	if ((inRetVal = inFILE_Read(&uldat_Handle, &uszReadData[0], lnCFGTLength)) == VS_ERROR)
	{
		inFILE_Close(&uldat_Handle);
		free(uszReadData);
		inDISP_DispLogAndWriteFlie(" Config.txt inFILE_Read *Error* Val[%ld]", inRetVal);
		return (VS_ERROR);
	}

	inFILE_Close(&uldat_Handle);

	/* 擷取檔案資料，以行數為單位，讀取到換行為止  */
	for (i = 0; i < lnCFGTLength; i++)
	{
		if (uszReadData[i] == 0x2C)
			inParamCount++;

		if (uszReadData[i] == 0x0A && uszReadData[i - 1] == 0x0D)
			break;
	}

	inDISP_DispLogAndWriteFlie(" Config.txt Cont [%ld]", inParamCount);
	
	/* 開始讀取預設的 CFGF.dat 檔 */
	if (inLoadCFGTRec(0) != VS_SUCCESS)
	{
		free(uszReadData);
		inDISP_DispLogAndWriteFlie(" Config.txt inLoadCFGTRec *Error* Line[%ld]", __LINE__);
		return (VS_ERROR);
	}

	for (i = 0; i < inRecCnt; i++)
	{

		/* 01_客製化專屬參數 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		j = 0;
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				inDISP_LogPrintfWithFlag("  Run Bef szRamRec[%x] uszReadData[%x] ",
						szRamRec[k - 1], uszReadData[j]);
				inDISP_LogPrintfWithFlag("  Customer_Indicator : [%s]", szRamRec);
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			inDISP_LogPrintfWithFlag(" CustomIndicator_ERROR");
			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inDISP_LogPrintfWithFlag("  CustomIndicator [%s]", szRamRec);
			inDISP_LogPrintfWithFlag("  j[%d] k[%d] ", j, k);
			inSetCustomIndicator(szRamRec);
		}

		/* 02_FES 模式 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				inDISP_LogPrintfWithFlag("  Run Bef szRamRec[%x] uszReadData[%x] ",
						szRamRec[k - 1], uszReadData[j]);
				inDISP_LogPrintfWithFlag("FES_Mode : [%s]", szRamRec);
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			inDISP_LogPrintfWithFlag("FES_Mode_ERROR");
			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			/* 目前設固定, 為ATS 05 20190214 [SAM] */
			inSetNCCCFESMode("05");
			//inSetNCCCFESMode(szRamRec);
		}

		//		/* 01_客製化專屬參數 */
		//		memset(szRamRec, 0x00, sizeof(szRamRec));
		//		j = 0;
		//		k = 0;
		//		
		//		if (VS_SUCCESS == inEDCTMS_ProcessTableData(&uszReadData[j], (unsigned char*)&szRamRec[k], &j, &k ))
		//		{
		//			inDISP_LogPrintfWithFlag("  CustomIndicator [%s]", szRamRec); 
		//			
		//			inDISP_LogPrintfWithFlag("  j[%d] k[%d] ", j, k); 
		//			
		//			inSetCustomIndicator(szRamRec);
		//		}else
		//		{
		//			inRetVal = VS_ERROR;
		//			inDISP_LogPrintfWithFlag("  CustomIndicator_ERROR"); 
		//			break;
		//		}
		//			
		//
		//		/* 02_FES 模式 */
		//		memset(szRamRec, 0x00, sizeof(szRamRec));
		//		k = 0;
		//		if (VS_SUCCESS == inEDCTMS_ProcessTableData(&uszReadData[j],(unsigned char*) &szRamRec[k], &j, &k ))
		//		{
		//			inDISP_LogPrintfWithFlag("  NCCCFESMode [%s]", szRamRec); 
		//			/* 目前設固定, 為ATS 05 20190214 [SAM] */
		//			inSetNCCCFESMode("05");
		//			//inSetNCCCFESMode(szRamRec);
		//		}else
		//		{
		//			inRetVal = VS_ERROR;
		//			inDISP_LogPrintfWithFlag("  FES_Mode_ERROR"); 
		//			break;
		//		}

		/* 03_通訊模式 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				inDISP_LogPrintfWithFlag("  Communication_Mode : [%s]", szRamRec);
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			inDISP_LogPrintfWithFlag("  Communication_Mode_ERROR");
			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			//inSetCommMode(szRamRec);

			/* 秉翰要求TMS下載固定設為 2 */
			inSetCommMode(_COMM_ETHERNET_MODE_);
		}

		/* 04_撥接備援 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				inDISP_LogPrintfWithFlag("  Dial_Up_Backup_Enable : [%s]", szRamRec);
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			inDISP_LogPrintfWithFlag("  Dial_Up_Backup_Enable_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetDialBackupEnable(szRamRec);
		}

		/* 05_加密模式 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				inDISP_LogPrintfWithFlag("  Encryption_Mode_Enable : [%s]", szRamRec);
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			inDISP_LogPrintfWithFlag("  Encryption_Mode_Enable_ERROR");
			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetEncryptMode(szRamRec);
		}

		/* 06_不可連續刷卡檢核 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				inDISP_LogPrintfWithFlag("  Split_Transaction_Check_Enable : [%s]", szRamRec);
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			inDISP_LogPrintfWithFlag("  Split_Transaction_Check_Enable_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetSplitTransCheckEnable(szRamRec);
		}

		/* 07_城市別 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				inDISP_LogPrintfWithFlag("  City_Name : [%s]", szRamRec);
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			inDISP_LogPrintfWithFlag("  City_Name_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetCityName(szRamRec);
		}

		/* 08_櫃號功能 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				inDISP_LogPrintfWithFlag("  Store_ID_Enable : [%s]", szRamRec);
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			inDISP_LogPrintfWithFlag("  Store_ID_Enable_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetStoreIDEnable(szRamRec);
		}

		/* 09_櫃號輸入最短位數(含) */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				inDISP_LogPrintfWithFlag("  Min_Store_ID_Length : [%s]", szRamRec);
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("Min_Store_ID_Length_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetMinStoreIDLen(szRamRec);
		}

		/* 10_櫃號輸入最長位數(含) */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "Max_Store_ID_Length : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("Max_Store_ID_Length_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetMaxStoreIDLen(szRamRec);
		}

		/* 11_輸入櫃號的提示訊息是否由TMS控制 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "Prompt Enable : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("Prompt Enable_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetPrompt(szRamRec);
		}

		/* 12_櫃號提示訊息的內容 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "Prompt_Data : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("Prompt_Data_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetPromptData(szRamRec);
		}

		/* 13_ECR連線是否啟動 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "ECR_Enable : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("ECR_Enable_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetECREnable(szRamRec);
		}

		/* 14_ECR卡號遮掩 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "ECR_CardNo_Truncate_Enable : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("ECR_CardNo_Truncate_Enable_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetECRCardNoTruncateEnable(szRamRec);
		}

		/* 15_ECR卡片有效期回傳 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "ECR_ExpireDate_Return_Enable : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("ECR_ExpireDate_Return_Enable_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetECRExpDateReturnEnable(szRamRec);
		}

		/* 16_列印產品代碼 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "Product_Code_Enable : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("Product_Code_Enable_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetProductCodeEnable(szRamRec);
		}

		/* 17_列印 Slogan */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "Print_Slogan : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("Print_Slogan_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetPrtSlogan(szRamRec);
		}

		/* 18_活動開始日 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "Slogan_Start_Date : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("Slogan_Start_Date_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetSloganStartDate(szRamRec);
		}

		/* 19_活動結束日 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "Slogan_End_Date : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("Slogan_End_Date_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetSloganEndDate(szRamRec);
		}

		/* 20_Slogan列印位置 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "Slogan_Print_Position : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("Slogan_Print_Position_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetSloganPrtPosition(szRamRec);
		}

		/* 21_帳單列印模式 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "Print_Mode : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("Print_Mode_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetPrtMode(szRamRec);
		}

		/* 22_啟動非接觸式讀卡功能 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "Contactless_Function_Enable : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("Contactless_Function_Enable_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetContactlessEnable(szRamRec);
			if (szRamRec[0] == 'Y')
				inCtlsCondition = 1;

		}

		/* 23_接受 Visa Paywave非接觸式卡片 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "VISA_Paywave_Enable : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("VISA_Paywave_Enable_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			if (inCtlsCondition == 1)
				inSetVISAPaywaveEnable("Y");
			else
				inSetVISAPaywaveEnable("N");
		}

		/* 24_接受 JCB Jspeedy非接觸式卡片 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "JCB_Jspeedy_Enable : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("JCB_Jspeedy_Enable_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			if (inCtlsCondition == 1)
				inSetJCBJspeedyEnable("Y");
			else
				inSetJCBJspeedyEnable("N");
		}

		/* 25_接受 MC Paypass非接觸式卡片 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "MC_Paypass_Enable : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("MC_Paypass_Enable_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			if (inCtlsCondition == 1)
				inSetMCPaypassEnable("Y");
			else
				inSetMCPaypassEnable("N");
		}

		/* 26_CUP退貨限額 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "CUP_Refund_Limit : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("CUP_Refund_Limit_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetCUPRefundLimit(szRamRec);
		}

		/* 27_CUP自動安全認證次數 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "CUP_Key_Exchange_Times : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("CUP_Key_Exchange_Times_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetCUPKeyExchangeTimes(szRamRec);
		}

		/* 28_交易電文是否上傳MAC驗證 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "MAC_Enable : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("MAC_Enable_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetMACEnable(szRamRec);
		}

		/* 29_密碼機模式 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "Pinpad_Mode : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("Pinpad_Mode_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetPinpadMode(szRamRec);
		}

		/* 30_輸入CVV2功能 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "FORCE_CVV2 : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("FORCE_CVV2_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetFORCECVV2(szRamRec);
		}

		/* 31_輸入AMEX 4DBC開關 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "4DBC_Enable : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("4DBC_Enable_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSet4DBC(szRamRec);
		}

		/* 32_啟動特殊卡別參數檔功能 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "Special_Card_Range_Enable : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("Special_Card_Range_Enable_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetSpecialCardRangeEnable(szRamRec);
		}

		/* 33_列印商店 Logo */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "Print_Merchant_Logo : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("Print_Merchant_Logo_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetPrtMerchantLogo(szRamRec);
		}

		/* 34_列印商店表頭 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "Print_Merchant_Name : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("Print_Merchant_Name_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetPrtMerchantName(szRamRec);
		}

		/* 35_列印商店提示語 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "Print_Notice : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("Print_Notice_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetPrtNotice(szRamRec);
		}

		/* 36_列印促銷廣告 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "Print_Promotion Advertisement : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("Print_Promotion Advertisement_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetPrtPromotionAdv(szRamRec);
		}

		/* 37_郵購及定期性行業Indicator */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "Electronic_Commerce_Flag : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("Electronic_Commerce_Flag_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetElecCommerceFlag(szRamRec);
		}

		/* 38_DCC詢價版本 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "Dcc_Flow_Version  : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("Dcc_Flow_Version_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetDccFlowVersion(szRamRec);
		}

		/* 39_DCC是否接受VISA卡 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "Support_Dcc_Visa  : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("Support_Dcc_Visa _ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetSupDccVisa(szRamRec);
		}

		/* 40_DCC是否接受MasterCard卡 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "Support_Dcc_MasterCard : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("Support_Dcc_MasterCard_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetSupDccMasterCard(szRamRec);
		}

		/* 41_DHCP Retry 次數 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "DHCP_Retry_Times : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("DHCP_Retry_Times_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetDHCPRetryTimes(szRamRec);
		}

		/* 42_銀行別 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "BANK_Indicator : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("BANK_Indicator_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetBankID(szRamRec);
		}

		/* 43_VEPS開關 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "VEPS_FLAG : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("VEPS_FLAG_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetVEPSFlag(szRamRec);
		}

		/* 44_連接埠參數 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "ComportMode : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("ComportMode_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetComportMode(szRamRec);
		}

		/* 45_FTP模式 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			if (szRamRec[k - 1] == 0x2C)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "FTPMode : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("FTPMode_ERROR");

			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetFTPMode(szRamRec);
		}

		/* 46_感應設備 */
		memset(szRamRec, 0x00, sizeof (szRamRec));
		k = 0;

		while (1)
		{
			szRamRec[k++] = uszReadData[j++];

			/* 因TMS檔案不會有不可視字元 所以最後一組所以直接判斷是否為換行符號 */
			if (szRamRec[k - 1] == 0x2C ||
					szRamRec[k - 1] == 0x0D ||
					szRamRec[k - 1] == 0x0A ||
					szRamRec[k - 1] == 0x00)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugLog, 0x00, sizeof (szDebugLog));
					sprintf(szDebugLog, "ContlessMode : [%s]", szRamRec);
					inDISP_LogPrintf(szDebugLog);
				}
				break;
			}
		}

		if (szRamRec[0] == 0x2C || szRamRec[0] == 0x00)
		{
			inRetVal = VS_ERROR;
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("ContlessMode_ERROR");
			break;
		} else
		{
			szRamRec[k - 1] = 0x00;
			inSetContlessMode(szRamRec);
		}
		
		/* 以下是公司TMS上沒定義到的參數，所以要在這邊更新 [SAM] */
		inSetBarCodeReaderEnable("N");
		inSetEMVPINBypassEnable("Y");
		inSetCUPOnlinePINEntryTimeout("020");
		inSetSignPadMode("1"); /* 設定簽名版 */
		inSetESCPrintMerchantCopy("N");
		inSetESCPrintMerchantCopyStartDate("00000000");
		inSetESCPrintMerchantCopyEndDate("00000000");
		inSetESCReciptUploadUpLimit("000");
		inSetContactlessReaderMode("1");
		inSetTMSDownloadMode("2"); /* FTP */
		inSetAMEXContactlessEnable("N");
		inSetCUPContactlessEnable("Y");
		inSetSmartPayContactlessEnable("N");
		inSetPayItemEnable("N");
		inSetStore_Stub_CardNo_Truncate_Enable("N");
		inSetIntegrate_Device("N");
		inSetFES_ID("000");
		inSetIntegrate_Device_AP_ID("               ");
		inSetShort_Receipt_Mode("N");
		inSetI_FES_Mode("N");
		inSetDHCP_Mode("N");
		inSetESVC_Priority("0000");
		inSetDFS_Contactless_Enable("N");
		inSetCloud_MFES("N");

		/* 共用參數檔【Config.txt】 */
		if (inSaveCFGTRec(0) < 0)
		{
			inDISP_DispLogAndWriteFlie(" TMS UPDATE Save CFGF *Error* Line[%d]", __LINE__);
			inRetVal = VS_ERROR;
			break;
		}
	}

	free(uszReadData);

	inDISP_LogPrintfWithFlag("  Final RetVal [%d] ", inRetVal);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}


