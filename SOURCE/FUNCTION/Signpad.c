#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <signature.h>
#include <epad.h>
#include <sqlite3.h>

#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/Transaction.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"
#include "../DISPLAY/DispMsg.h"
#include "../EVENT/MenuMsg.h"
#include "../EVENT/Menu.h"
//#include "../../FISC/NCCCfisc.h"
//#include "../../NCCC/NCCCesc.h"
#include "Function.h"
#include "Sqlite.h"
#include "Batch.h"
#include "Card.h"
#include "CFGT.h"
#include "HDT.h"
#include "HDPT.h"
#include "EDC.h"
#include "FILE_FUNC/File.h"
#include "FILE_FUNC/FIleLogFunc.h"
#include "Signpad.h"
#include "UNIT_FUNC/TimeUint.h"

/* [外接設備設定] */
#include "ECR_FUNC/ECR.h"
#include "MULTI_FUNC/MultiFunc.h"
#include "MULTI_FUNC/MultiHostFunc.h"
#include "MULTI_FUNC/JsonMultiHostFunc.h"

SIGNPAD_OBJECT	gsrSignpad = {_SIGNPAD_LEFT_, VS_FALSE, 0};
extern int	ginDebug;

/* [外接設備設定] */
extern MULTI_TABLE	gstMultiOb;

/*
Function        : inSIGN_TouchSignature_Start
Date&Time   : 2017/7/24 上午 11:27
Describe        : 簽名底層API開始
 * uiX : 簽名區塊起點的X座標點 
 * uiY : 簽名區塊起點的Y座標點 
 * uiWidth : 簽名區塊的寬度大小
 * uiHeight: 簽名區塊的長度大小 
 * 
 * API會利用初始的X，Y座標點，以 uiWidth 、 uiHeight 畫出來的大小進行簽名區塊的劃分
*/
int inSIGN_TouchSignature_Start(unsigned int uiX ,unsigned int uiY,unsigned int uiWidth ,unsigned int uiHeight,unsigned char *uszBMPFileName,unsigned long ulTimeout)
{

#ifndef	_LOAD_KEY_AP_
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_TouchSignatureStart(uiX, uiY, uiWidth, uiHeight , uszBMPFileName, ulTimeout);
	if (usRetVal != d_OK)
	{
		inDISP_DispLogAndWriteFlie("  BMP  Signature Start *Error* Result[0x%04x] Line[%d]", usRetVal, __LINE__);
		return (VS_ERROR);
	}
	
#endif

	return (VS_SUCCESS);
}

/*
Function        :inSIGN_TouchSignature_Internal_END
Date&Time       :2017/4/20 上午 11:56
Describe        :
*/
int inSIGN_TouchSignature_Internal_END(void)
{

#ifndef	_LOAD_KEY_AP_
	unsigned short	usRetVal;

	usRetVal = CTOS_TouchSignatureTerminate();
	if (usRetVal != d_OK)
	{
		inDISP_DispLogAndWriteFlie("  BMP Terminate Signature *Error* Result[0x%04x] Line[%d]", usRetVal, __LINE__);
		return (VS_ERROR);
	}
#endif

	return (VS_SUCCESS);
}

/*
Function        :inSIGN_GetSignatureStatus
Date&Time       :2017/7/24 上午 10:34
Describe        :
		Signature Status
		d_SCP_STATUS_NO_DATA	  0x00000001    未碰到螢幕
		d_SCP_STATUS_IDLE	  0x00000002    有碰到螢幕後手離開螢幕
		d_SCP_STATUS_SIGNING      0x00000003    手在螢幕上
*/
int inSIGN_GetSignatureStatus(unsigned long *ulStatus , unsigned long *ulDuration)
{

#ifndef	_LOAD_KEY_AP_

	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_GetSignatureStatus(ulStatus, ulDuration);

	if (usRetVal != d_OK)
	{
		inDISP_DispLogAndWriteFlie("  BMP Get Signature Status  *Error* Result[0x%04x] Line[%d]", usRetVal, __LINE__);
		return (VS_ERROR);
	}
#endif

	return (VS_SUCCESS);
}

/*
Function        :inSIGN_IsSigned
Date&Time       :2017/7/24 上午 10:49
Describe        :確認是否有被簽名過
*/
int inSIGN_IsSigned(SIGNPAD_OBJECT *srSignpad)
{
	int		inRetVal = VS_ERROR;
	unsigned long	ulStatus = 0;
	unsigned long	ulDuration = 0;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	inRetVal = inSIGN_GetSignatureStatus(&ulStatus, &ulDuration);
	
	inDISP_LogPrintfWithFlag ("  inRetVal[%d] ulStatus[%d] ulDuration[%d] Sigend[%d]", inRetVal, ulStatus, ulDuration, srSignpad->inSigned);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (ulStatus == d_SCP_STATUS_NO_DATA || srSignpad->inSigned != VS_TRUE)
	{
		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inSIGN_BMPConverter_Left
Date&Time       :2017/7/24 上午 11:56
Describe        :
*/
int inSIGN_BMPConverter_Left(unsigned char *uszInputBMPFileName, unsigned char *uszOutputBMPFileName)
{

#ifndef	_LOAD_KEY_AP_
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_BMPConverter(uszInputBMPFileName, uszOutputBMPFileName, d_BMP_CONVERT_LEFT_ROTATE);
	if (usRetVal != d_OK)
	{
		inDISP_DispLogAndWriteFlie("  BMP Converter Left *Error* Result[0x%04x] Line[%d]", usRetVal, __LINE__);
		return (VS_ERROR);
	}
	
#endif
	return (VS_SUCCESS);
}

/*
Function        :inSIGN_BMPConverter_Right
Date&Time       :2017/7/24 下午 1:17
Describe        :
*/
int inSIGN_BMPConverter_Right(unsigned char *uszInputBMPFileName, unsigned char *uszOutputBMPFileName)
{

#ifndef	_LOAD_KEY_AP_
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_BMPConverter(uszInputBMPFileName, uszOutputBMPFileName, d_BMP_CONVERT_RIGHT_ROTATE);
	if (usRetVal != d_OK)
	{
		inDISP_DispLogAndWriteFlie("  BMP Converter Right *Error* Result[0x%04x] Line[%d]", usRetVal, __LINE__);
		return (VS_ERROR);
	}
#endif

	return (VS_SUCCESS);
}

/*
Function        :inSIGN_BMPConverter_OneColor
Date&Time       :2017/7/24 下午 1:17
Describe        :轉成單色
*/
int inSIGN_BMPConverter_OneColor(unsigned char *uszInputBMPFileName, unsigned char *uszOutputBMPFileName)
{
#ifndef	_LOAD_KEY_AP_
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_BMPConverter(uszInputBMPFileName, uszOutputBMPFileName, d_BMP_CONVERT_ONE_BIT_COLOR);
	if (usRetVal != d_OK)
	{
		inDISP_LogPrintfWithFlag("  BMP Converter Color *Error* Result[0x%04x] Line[%d]", usRetVal, __LINE__);
		return (VS_ERROR);
	}
	
#endif

	return (VS_SUCCESS);
}

/*
Function        :inSIGN_Text_To_BMP
Date&Time       :2017/7/24 下午 1:53
Describe        :
*/
int inSIGN_Text_To_BMP(unsigned long ulwidth, unsigned long ulHeight, unsigned short usX, unsigned short usY, unsigned char *uszString,unsigned short uszFontSize,char *szBMPFileName)
{
#ifndef	_LOAD_KEY_AP_
	vdCTOSS_TextBufferToBMP(ulwidth, ulHeight, usX, usY, uszString, uszFontSize, szBMPFileName);
#endif

	return (VS_SUCCESS);
}

/*
Function        :inSIGN_Text_To_BMPEx
Date&Time       :2017/7/24 下午 1:53
Describe        :
*/
int inSIGN_Text_To_BMPEx(unsigned short usX, unsigned short usY, unsigned char *uszString,unsigned short uszFontSize,char *szBMPFileName)
{
#ifndef	_LOAD_KEY_AP_
	vdCTOSS_TextBufferToBMPEx(usX, usY, uszString, uszFontSize, szBMPFileName);
#endif

	return (VS_SUCCESS);
}

/*
Function        :inSIGN_Rotate_TextBMP
Date&Time       :2017/7/24 下午 2:07
Describe        :用vdCTOSS_TextBufferToBMP產生的BMP只能用這個轉，有可能0是左轉，1是右轉之類的定義
*/
int inSIGN_Rotate_TextBMP(char *szFileName, int inRotate)
{
#ifndef	_LOAD_KEY_AP_
	vdCTOSS_RotateBMPFile(szFileName, inRotate);
#endif

	return (VS_SUCCESS);
}

/*
Function        :inSIGN_Display_Black_Back
Date&Time       :2017/7/24 下午 2:16
Describe        :將要簽名的背景區塊塗黑，要簽名時，會把中間留白，變成框線
*/
int inSIGN_Display_Black_Back(unsigned short usX, unsigned short usY, unsigned short usXSize, unsigned short usYSize)
{
	unsigned short	usRetVal = 0;

	/* 將要簽名的背景區塊塗黑 */
	usRetVal = CTOS_LCDGSetBox(usX, usY, usXSize, usYSize, d_LCD_FILL_1);
	if (usRetVal != d_OK)
	{
		inDISP_DispLogAndWriteFlie("  BMP  LCD Set Box *Error* Result[0x%04x] Line[%d]", usRetVal, __LINE__);
		return (VS_ERROR);
	}
	return (VS_SUCCESS);
}

/*
Function        :inSIGN_TouchSignature_Flow
Date&Time       :2015/12/29 下午 3:55
Describe        :SignPad觸控簽名功能
*/
int inSIGN_TouchSignature_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	int inCheckMessageDisp = VS_TRUE;
	char	szSignPadMode[2 + 1];
	char	szMemoSignBMPFile[32 + 1];

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	inTIME_UNIT_GetGlobalTimeToCompare("  TouchSignature Start ");
	
	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inSIGN_TouchSignature_Flow INIT" );	

	/* CFGT的Signpad開關判斷 */
	memset(szSignPadMode , 0x00, sizeof(szSignPadMode));
	inGetSignPadMode(szSignPadMode);

	/* 確認電子簽名左側畫面 20190701 [SAM] */	
	/* 需要把全域使用的變數初始化,才不會影響到  inSIGN_TouchSignature_Internal() 裏的判斷  */
	gsrSignpad.inHasBeenSelect = _SIGNPAD_STATUS_INIT_ ;

	while (1)
	{
		/* 不開signpad或免簽 */
		if (!memcmp(szSignPadMode, _SIGNPAD_MODE_0_NO_, 1))
		{
			inDISP_LogPrintfWithFlag("  不支援簽名版");

			/* 更新簽名狀態 */
			pobTran->srBRec.inSignStatus = _SIGN_BYPASS_;
			inBATCH_Update_Sign_Status_By_Sqlite(pobTran);

			inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inSIGN_TouchSignature_Flow NO SIGNPAD END");
			inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] No Sign END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_SUCCESS);
		}
		else if (pobTran->srBRec.uszF56NoSignatureBit == VS_TRUE)
		//else if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
		{
			inDISP_LogPrintfWithFlag("  達成免簽條件");
			
			/* 更新簽名狀態 */
			pobTran->srBRec.inSignStatus = _SIGN_NO_NEED_SIGN_;
			inBATCH_Update_Sign_Status_By_Sqlite(pobTran);

			 /* 因為免簽名沒圖檔，會無法上傳，在這裏補抓免簽名圖檔 20181221 [SAM] */
			memset(szMemoSignBMPFile, 0x00, sizeof(szMemoSignBMPFile));
			inFunc_ComposeFileName_InvoiceNumber(pobTran, szMemoSignBMPFile, _PICTURE_FILE_EXTENSION_, 6);
			
			/* 先刪檔避免沒簽名用到上個批次漏刪的簽名圖檔(保險起見) */
			inFILE_Delete((unsigned char*)szMemoSignBMPFile);
			
			
			inDISP_LogPrintfWithFlag("  Sign CopyFile Old[%s]  New[%s]",_CONNECTING_ , szMemoSignBMPFile);
			inRetVal = inFILE_Copy_File( (unsigned char*)"NoSign.bmp", (unsigned char*)szMemoSignBMPFile);
			
			if (inRetVal != VS_SUCCESS)
			{
				inDISP_LogPrintfWithFlag("  Sign CopyFile Err[%d]", inRetVal);
			}
			
			inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inSIGN_TouchSignature_Flow NO Signature END");

			inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] No Sign Request END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_SUCCESS);
		}
		/* 使用內建簽名板 */
		else if (!memcmp(szSignPadMode, _SIGNPAD_MODE_1_INTERNAL_, strlen(_SIGNPAD_MODE_1_INTERNAL_)))
		{
/* 沒觸控功能就不執行 */
#ifndef	_TOUCH_CAPBILITY_
	return (VS_SUCCESS);
#endif
			inDISP_LogPrintfWithFlag("  使用內建簽名板internal");
			
			/* 抓取 KioskFlag的值，如為 TRUE 就不用確認畫面 2020/3/6 下午 4:28 [SAM] */
			if(inFunc_GetKisokFlag() == VS_TRUE)
			{
				/* 更新簽名狀態 */
				pobTran->srBRec.inSignStatus = _SIGN_SIGNED_;
				inBATCH_Update_Sign_Status_By_Sqlite(pobTran);

				 /* 因為免簽名沒圖檔，會無法上傳，在這裏補抓免簽名圖檔 20181221 [SAM] */
				memset(szMemoSignBMPFile, 0x00, sizeof(szMemoSignBMPFile));
				inFunc_ComposeFileName_InvoiceNumber(pobTran, szMemoSignBMPFile, _PICTURE_FILE_EXTENSION_, 6);

				/* 先刪檔避免沒簽名用到上個批次漏刪的簽名圖檔(保險起見) */
				inFILE_Delete((unsigned char*)szMemoSignBMPFile);
			
			
				inDISP_LogPrintfWithFlag("  Sign CopyFile Old[%s]  New[%s]",_CONNECTING_ , szMemoSignBMPFile);
				inRetVal = inFILE_Copy_File( (unsigned char*)"Blanksign.bmp", (unsigned char*)szMemoSignBMPFile);

				if (inRetVal != VS_SUCCESS)
				{
					inDISP_LogPrintfWithFlag("  Sign CopyFile Err[%d]", inRetVal);
				}
				
				inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] F_[%s] Kiosk BlankSign  END -----",__FILE__, __FUNCTION__, __LINE__, szMemoSignBMPFile);
				return (VS_SUCCESS);
				
			}else
			{
				/* 新增檢查各交易金額的畫面 20190215 [SAM] */
				if(inCheckMessageDisp == VS_TRUE)
				{
					inCheckMessageDisp = VS_FALSE;
					inSIGN_Check_Trans_For_Sign(pobTran);
				}
				inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inSIGN_TouchSignature_Flow Bef Sign ");

				inSIGN_TouchSignature_Internal(pobTran);
			}
			
			inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inSIGN_TouchSignature_Flow Sign END ");
		}
		else if (!memcmp(szSignPadMode, _SIGNPAD_MODE_2_EXTERNAL_, strlen(_SIGNPAD_MODE_2_EXTERNAL_)))
		{
			inDISP_LogPrintfWithFlag("  使用外接簽名板external");
			
			/* [外接設備設定] */
			if(VS_SUCCESS == inMultiFunc_JsonCallSlave(pobTran, _MULTI_SIGNPAD_NO_, &gstMultiOb))
			{
				inRetVal = inSIGN_CheckSignatureOfVerticalView(pobTran);
				if (inRetVal == VS_SUCCESS)
				{
					if(VS_SUCCESS != inMultiFunc_JsonCallSlave(pobTran, _MULTI_TRANS_STOP_NO_, &gstMultiOb))
					{
						inDISP_LogPrintfWithFlag(" Stop Extern Device *Error* Line[%d]", __LINE__);
					}
					inDISP_LogPrintfWithFlag(" Signature Success Line[%d]", __LINE__);
				}
				/* 確認不OK，重簽 */
				else
				{
					inDISP_LogPrintfWithFlag(" ***Re-Sign*** Line[%d]", __LINE__);
					if(VS_SUCCESS != inMultiFunc_JsonCallSlave(pobTran, _MULTI_RESIGN_TRANS_NO_, &gstMultiOb))
					{
						inDISP_LogPrintfWithFlag(" Re-Sign *Error* Line[%d]", __LINE__);
					}
					inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inSIGN_TouchSignature_Flow Re INIT" );	
					continue;
				}
			}
			pobTran->srBRec.inSignStatus == _SIGN_SIGNED_;
			inBATCH_Update_Sign_Status_By_Sqlite(pobTran);
			return (VS_SUCCESS);
		}

		/* 有簽名就要，確認簽名畫面 */
		if (pobTran->srBRec.inSignStatus == _SIGN_SIGNED_)
		{
			inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inSIGN_TouchSignature_Flow Confirm");
			/* 横向確認簽名的功能  */
			inRetVal = inSIGN_CheckSignatureOfHorizontalView(pobTran);
			
			/* 確認OK跳出 */
			//inRetVal = inSIGN_CheckSignature(pobTran);
			if (inRetVal == VS_SUCCESS)
			{
				break;
			}
			/* 確認不OK，重簽 */
			else
			{
				inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inSIGN_TouchSignature_Flow Re INIT" );	
				continue;
			}
		}
		/* 應簽而沒簽名要再確認一次是否確定不要用簽名板 */
		else if (pobTran->srBRec.inSignStatus == _SIGN_BYPASS_)
		{
			/* 確認OK跳出 */
			inRetVal = inSIGN_Check_NOSignature(pobTran);
			if (inRetVal == VS_SUCCESS)
			{
				break;
			}
			/* 確認不OK，重簽 */
			else
			{
				continue;
			}
		}
		else
		{
			break;
		}

	}

	/* 恢復UI */
	inFunc_ResetTitle(pobTran);

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inSIGN_TouchSignature_Flow END");

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
Function        :inSIGN_TouchSignature_Internal
Date&Time       :2017/4/20 上午 11:44
Describe        :SignPad內建觸控簽名功能
*/
int inSIGN_TouchSignature_Internal(TRANSACTION_OBJECT *pobTran)
{
#if _MACHINE_TYPE_ != _CASTLE_TYPE_UPT1000_
	unsigned short	usResult;
#endif
	int	inChoice;
	int	inTouchSensorFunc;
	char	szMemoSignBMPFile[32 + 1];
	unsigned char   uszKey;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "Signature_Internal INIT" );	
	
	/* [校正touch IC] 依照聯合信用卡新增,修正內建signpad觸控校正 2022/11/22 [SAM] */
	/* 一定要在inSIGN_TouchSignature_Internal_START 之前 */
	inSIGN_TouchSignature_Internal_Calibration();
	
	/* 預設為左邊 */
//	gsrSignpad.inPosition = _SIGNPAD_RIGHT_;
	
	/* 確認電子簽名左側畫面 20190701 [SAM] */
	/* 如果已設定過簽名方向，再重簽時不要再重新設定簽名方向  */
	if(gsrSignpad.inHasBeenSelect != _SIGNPAD_HAS_BEEN_SELECT_)
	{
		/* 初始化結構 */
		memset(&gsrSignpad, 0x00, sizeof(gsrSignpad));
		
		gsrSignpad.inPosition = _SIGNPAD_RIGHT_;
		gsrSignpad.inHasBeenSelect = _SIGNPAD_HAS_BEEN_SELECT_;
		
	}else{
		/* 如果有執行過簽名，只要初始化這些參數，方向及是否進行過簽名的FLAG不需要變動 */
		gsrSignpad.inSigned = VS_FALSE;
		gsrSignpad.ulSignTimeStart = 0;
	}
	
	inTouchSensorFunc = _Touch_SIGNATURE_;
	inSIGN_TouchSignature_Internal_START(pobTran, &gsrSignpad);

	/* 初始化 用Invoice Number來命名 */
	memset(szMemoSignBMPFile, 0x00, sizeof(szMemoSignBMPFile));
	inFunc_ComposeFileName_InvoiceNumber(pobTran, szMemoSignBMPFile, _PICTURE_FILE_EXTENSION_, 6);
	/* 先刪檔避免沒簽名用到上個批次漏刪的簽名圖檔(保險起見) */
	inFILE_Delete((unsigned char*)szMemoSignBMPFile);
	
	/* 清鍵盤buffer */
	inFlushKBDBuffer();

	while (1)
	{
		uszKey = uszKBD_Key();
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);

		/* 判斷是否在簽名板下筆 */
		if (inChoice == _SignTouch_Signpad_)
		{
			gsrSignpad.inSigned = VS_TRUE;
			/* 簽名板，於落筆簽名後，應重新計算timeout時間。 */
			inSIGN_TimeoutStart(&gsrSignpad.ulSignTimeStart);
		}

		/* 確認TimeOut並顯示 */
		/* 進入簽名板流程後，應於1分鐘後開始提示聲 */
		if (inSIGN_TimeoutCheck(&gsrSignpad, 60, VS_FALSE) == VS_TIMEOUT)
		{
			inDISP_BEEP(1, 2000);
		}
		/* 1分30秒後Timeout。 */
		/*if (inSIGN_TimeoutCheck(&gsrSignpad, 90, VS_FALSE) == VS_TIMEOUT)
		{
			uszKey = _KEY_TIMEOUT_;
		}*/

		if (inChoice == _SignTouch_Clear_ || uszKey == _KEY_CANCEL_)
		{
			inSIGN_TouchSignature_Internal_END();
			inSIGN_TouchSignature_Internal_START(pobTran, &gsrSignpad);
		}
		else if (inChoice == _SignTouch_Ok_ || uszKey == _KEY_ENTER_ || uszKey == _KEY_TIMEOUT_)
		{
			/* 判斷簽名狀態 沒下過筆直接按確認則不存圖片 */
			if (inSIGN_IsSigned(&gsrSignpad) != VS_SUCCESS)
			{
				memset(szMemoSignBMPFile, 0x00, sizeof(szMemoSignBMPFile));
				inSIGN_TouchSignature_Internal_END();
				/* 刪檔 */
				inFunc_Delete_Data("", SignBMPFile, _AP_ROOT_PATH_);

				/* 沒簽名，當筆要出紙本 */
				/* 更新簽名狀態 */
				pobTran->srBRec.inSignStatus = _SIGN_BYPASS_;
				inBATCH_Update_Sign_Status_By_Sqlite(pobTran);

			}
			else
			{
				inDISP_ClearAll();
				inDISP_ChineseFont("圖檔處理中...", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				
				inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "Signature_Internal Process BMP");
				inSIGN_TouchSignature_Internal_END();
				inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "Signature_Internal Start Process");
				
				/* 因為原圖不會存在FS DATA 中，所以先拷備一份到 FS DATA裏  20190627 [SAM] */
				if(VS_SUCCESS != inFunc_Copy_Data(SignBMPFile, _AP_ROOT_PATH_, TempSignBmpFile, _FS_DATA_PATH_))
				{
					inDISP_DispLogAndWriteFlie("  SIGN Copy  Sign To FsData File *Error* Line[%d]",  __LINE__);
				}
				inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "Signature_Internal Copy SignBMPFile");
				
				/* 因為原圖片還是需要轉為正向但顯示圖還是横向，所以把FS DATA 裏的簽名檔重新命名 20190627 [SAM]  */
//				inFunc_Move_Data(SignBMPFile,_FS_DATA_PATH_, TempSignBmpFile, _FS_DATA_PATH_);
//				inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "Signature_Internal Rename SignBMPFile");
				
				/* 圖片旋轉 */
				if (gsrSignpad.inPosition == _SIGNPAD_LEFT_)
				{
					inSIGN_BMPConverter_Left((unsigned char *)SignBMPFile, (unsigned char *)SignBMPFile);
				}
				else if (gsrSignpad.inPosition == _SIGNPAD_RIGHT_)
				{
					inSIGN_BMPConverter_Right((unsigned char *)SignBMPFile, (unsigned char *)SignBMPFile);
				}				
				inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "Signature_Internal Convert  SignBMPFile");
				
				
				/* 圖片轉換成單色 */
				if(VS_SUCCESS != inSIGN_BMPConverter_OneColor((unsigned char *)SignBMPFile, (unsigned char *)szMemoSignBMPFile))
				{
					inDISP_DispLogAndWriteFlie("  SIGN Conver BMP *Error* Line[%d]",  __LINE__);
				}
							
				
				inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "Signature_Internal Convert Color szMemoSignBMPFile");
				
#ifdef __NEW_SIGN_FUNC__
				
#if _MACHINE_TYPE_ != _CASTLE_TYPE_UPT1000_
				//usResult = CTOS_LCDGResizeImage((unsigned char *)szMemoSignBMPFile, (unsigned char *)szMemoSignBMPFile, 384, 254, d_1_BITDEPTH);
				usResult = CTOS_LCDGResizeImage((unsigned char *)szMemoSignBMPFile, (unsigned char *)szMemoSignBMPFile, 384, 254);
				if( d_OK != usResult)
				{
					inDISP_LogPrintfWithFlag("  SIGN Resize *Error* Result[0x%04x] Line[%d]", usResult, __LINE__);
				}
#endif
				
#endif
				
				inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "Signature_Internal Resize szMemoSignBMPFile");
	
				/* 放到fs_data內 */
				if(VS_SUCCESS != inFunc_Move_Data(szMemoSignBMPFile, _AP_ROOT_PATH_, "", _FS_DATA_PATH_))
				{
					inDISP_DispLogAndWriteFlie("  SIGN Move  szMemoSignBMPFile To FsData File *Error* Line[%d]",  __LINE__);
				}
				
				inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "Signature_Internal Move szMemoSignBMPFile");
			

				if(VS_SUCCESS != inFunc_Delete_Data("", SignBMPFile, _AP_ROOT_PATH_))
				{
					inDISP_DispLogAndWriteFlie("  SIGN Del  SignBMPFile File *Error* Line[%d]",  __LINE__);
				}
				
				inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "Signature_Internal Del SignBMPFile");
				
				/* 更新簽名狀態 */
				pobTran->srBRec.inSignStatus = _SIGN_SIGNED_;
				inBATCH_Update_Sign_Status_By_Sqlite(pobTran);

			}
			
			inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "Signature_Internal END");
			
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_SUCCESS);
		}
		else if (inChoice == _SignTouch_Rotate_ || uszKey == d_KBD_DOT)
		{

			/* 判斷是否已經下筆 有下筆則擋住旋轉，（因為判斷整個畫面，所以按畫面上的按鈕也算下筆） */
			/* 現在將條件從d_SCP_STATUS_SIGNING && Duration為 0改為 Duration為 0，可以讓按一下或一直押著都旋轉，但仍無法判斷 */
			if (inSIGN_IsSigned(&gsrSignpad) != VS_SUCCESS)
			{
				/* Signature Status為整個螢幕檢測 使用觸控旋轉的話 結果會是d_SCP_STATUS_SIGNING  Duration為 0 */
			}
			else
			{
				continue;
			}

			inSIGN_TouchSignature_Internal_END();

			if (gsrSignpad.inPosition == _SIGNPAD_LEFT_)
			{
				gsrSignpad.inPosition = _SIGNPAD_RIGHT_;
				inSIGN_TouchSignature_Internal_START(pobTran, &gsrSignpad);
			}
			else if (gsrSignpad.inPosition == _SIGNPAD_RIGHT_)
			{
				gsrSignpad.inPosition = _SIGNPAD_LEFT_;
				inSIGN_TouchSignature_Internal_START(pobTran, &gsrSignpad);
			}

		}
		else
		{
			continue;
		}
        }
}

/*
Function        :inSIGN_TouchSignature_START
Date&Time       :2015/12/29 下午 3:55
Describe        :用來判斷左邊簽名還是右邊簽名 並啟動簽名功能
*/
int inSIGN_TouchSignature_Internal_START(TRANSACTION_OBJECT *pobTran, SIGNPAD_OBJECT *srSignpad)
{
	int	inSigned = 0;
	char    szDispBuffer[64 + 1];
	char    szAmount[20 + 1];
	char	szOutputAmount[20 + 1];
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Position[%d] START -----",__FILE__, __FUNCTION__, __LINE__, srSignpad->inPosition);
	memset(szAmount, 0x00, sizeof(szAmount));
	/* 要含小費 */
	if ((pobTran->srBRec.uszVOIDBit == VS_TRUE &&
		(pobTran->srBRec.inOrgCode != _REFUND_ && pobTran->srBRec.inOrgCode != _INST_REFUND_ && pobTran->srBRec.inOrgCode != _REDEEM_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_MAIL_ORDER_REFUND_))	||
		 pobTran->srBRec.inCode == _REFUND_			||
		 pobTran->srBRec.inCode == _INST_REFUND_		||
		 pobTran->srBRec.inCode == _REDEEM_REFUND_		||
		 pobTran->srBRec.inCode == _CUP_REFUND_		||
		 pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
	{
		inSigned = _SIGNED_MINUS_;
		if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
		{
			sprintf(szAmount, "%ld", atol(pobTran->srBRec.szDCC_FCA) + atol(pobTran->srBRec.szDCC_TIPFCA));
		}
		else
		{
			sprintf(szAmount, "%ld", 0 - pobTran->srBRec.lnTotalTxnAmount);
		}
	}
	else
	{
		inSigned = _SIGNED_NONE_;
		if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
		{
			sprintf(szAmount, "%ld", atol(pobTran->srBRec.szDCC_FCA) + atol(pobTran->srBRec.szDCC_TIPFCA));
		}
		else
		{
			sprintf(szAmount, "%ld", pobTran->srBRec.lnTotalTxnAmount);
		}
	}

	/* DCC要加入含小費的金額 */
	memset(szOutputAmount, 0x00, sizeof(szOutputAmount));
	if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
		inFunc_Amount_Comma_DCC(szAmount, "", 0x00, inSigned, 18, _PAD_LEFT_FILL_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szOutputAmount);
	else
		inFunc_Amount_Comma_DCC(szAmount, "NT$", 0x00, inSigned, 18, _PAD_LEFT_FILL_RIGHT_, "0", "", szOutputAmount);

	memset(szDispBuffer, 0x00, sizeof(szDispBuffer));
	sprintf(szDispBuffer, "金額：%s", szOutputAmount);

	if (srSignpad->inPosition == _SIGNPAD_LEFT_)
	{
		/* 簽名底圖 */
		inDISP_PutGraphic(_SIGNATURE_BACKGROUND_LEFT_, 0, _COORDINATE_Y_LINE_8_1_);

/* 修改簽名版的大小及顯示 20190625 [SAM] */
#ifdef __ORIGINAL_SIGN_FLOW__
		/* 顯示金額 */
		inSIGN_Text_To_BMPEx(1, 5, (unsigned char *)szDispBuffer, _CHINESE_FONT_12X19_, AmountBMPFile);
		inDISP_PutGraphic(AmountBMPFile, 0,  _COORDINATE_Y_LINE_8_1_);

		/* 簽名區塊 & 初始化觸控區 */
		inSIGN_Display_Black_Back(_SIGNEDPAD_OUT_LEFT_X1_, _SIGNEDPAD_OUT_LEFT_Y1_, _SIGNEDPAD_OUT_WIDTH_, _SIGNEDPAD_OUT_HIGHT_);
		inSIGN_TouchSignature_Start(_SIGNEDPAD_LEFT_X1_, _SIGNEDPAD_LEFT_Y1_, _SIGNEDPAD_WIDTH_, _SIGNEDPAD_HIGHT_, (unsigned char *)SignBMPFile, 0);
#else				
		/* 簽名區塊 & 初始化觸控區 */
		inSIGN_Display_Black_Back(_LEFT_SIGNEDPAD_WRITE_LINE_X_ - _LINE_WIDTH_, 0,  4,  _LCD_YSIZE_);
		inSIGN_TouchSignature_Start(0, 0, (_LEFT_SIGNEDPAD_WRITE_LINE_X_ - 4), _LCD_YSIZE_, (unsigned char *)SignBMPFile, 0);			
#endif
				
	}
	else
	{
		/* 簽名底圖 */
		inDISP_PutGraphic(_SIGNATURE_BACKGROUND_RIGHT_, 0, _COORDINATE_Y_LINE_8_1_);
/* 修改簽名版的大小及顯示 20190625 [SAM] */
#ifdef __ORIGINAL_SIGN_FLOW__
                /* 顯示金額 */
                inSIGN_Text_To_BMPEx(1, 1, (unsigned char *)szDispBuffer, _CHINESE_FONT_12X19_, AmountBMPFile);
                /* 旋轉兩次朝另一個方向 */
                inSIGN_Rotate_TextBMP(AmountBMPFile, 0);
                inSIGN_Rotate_TextBMP(AmountBMPFile, 0);
                inDISP_PutGraphic(AmountBMPFile, 260, 90);

                /* 簽名區塊 & 初始化觸控區 */
                inSIGN_Display_Black_Back(_SIGNEDPAD_OUT_RIGHT_X1_, _SIGNEDPAD_OUT_RIGHT_Y1_, _SIGNEDPAD_OUT_WIDTH_, _SIGNEDPAD_OUT_HIGHT_);
                inSIGN_TouchSignature_Start(_SIGNEDPAD_RIGHT_X1_, _SIGNEDPAD_RIGHT_Y1_, _SIGNEDPAD_WIDTH_, _SIGNEDPAD_HIGHT_, (unsigned char *)SignBMPFile, 0);
				
#else		
		/* 劃出一條間隔線 */
		inSIGN_Display_Black_Back(_RIGHT_SIGNEDPAD_WRITE_LINE_X_, 0, 4, _LCD_YSIZE_);
		/* 簽名區塊 & 初始化觸控區 */
		inSIGN_TouchSignature_Start((_RIGHT_SIGNEDPAD_WRITE_LINE_X_ + _LINE_WIDTH_) , 0 , (_LCD_XSIZE_ - _RIGHT_SIGNEDPAD_WRITE_LINE_X_ + _LINE_WIDTH_), _LCD_YSIZE_, (unsigned char *)SignBMPFile, 0);
				
#endif		
	}

	/* 重置是否已下筆狀態 */
	srSignpad->inSigned = VS_FALSE;
	/* 重置TimeOut */
	/* 進入簽名板流程後，應於1分鐘後開始提示聲，1分30秒後Timeout。 */
	inSIGN_TimeoutStart(&srSignpad->ulSignTimeStart);
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}

/*
Function	:inSIGN_TimeoutCheck
Date&Time	:2017/4/27 上午 10:59
Describe	:CTLS Check是否TimeOut
*/
int inSIGN_TimeoutCheck(SIGNPAD_OBJECT *srSignpad, int inTimeOut, unsigned char uszDispTimeout)
{
	int		inRemainSecond = 0;
	char		szTemplate[10 + 1];
	unsigned long	ulSecond = 0;
	unsigned long	ulMilliSecond = 0;

	inFunc_GetRunTime(srSignpad->ulSignTimeStart, &ulSecond, &ulMilliSecond);
	inRemainSecond = inTimeOut - (int)ulSecond;

	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%02d", inRemainSecond);

	if (uszDispTimeout == VS_TRUE)
	{
		if (srSignpad->inPosition == _SIGNPAD_LEFT_)
		{
			inSIGN_Text_To_BMP(160, 70, 1, 5, (unsigned char *)szTemplate, _CHINESE_FONT_12X19_, TimeBMPFile);
			inSIGN_Rotate_TextBMP(TimeBMPFile, 0);
			inDISP_PutGraphic(TimeBMPFile, 0,  _COORDINATE_Y_LINE_8_7_);
		}
		else
		{
			inSIGN_Text_To_BMP(384, 70, 1, 5, (unsigned char *)szTemplate, _CHINESE_FONT_12X19_, TimeBMPFile);
			inSIGN_Rotate_TextBMP(TimeBMPFile, 0);
			inSIGN_Rotate_TextBMP(TimeBMPFile, 0);
			inDISP_PutGraphic(TimeBMPFile, 260, 0);
		}
	}

	if (inRemainSecond <= 0)
		return (VS_TIMEOUT);
	else
        	return (VS_SUCCESS);
}

/*
Function	:inSIGN_TimeoutStart
Date&Time	:2017/7/24 上午 11:50
Describe	:取得開始時秒數
*/
int inSIGN_TimeoutStart(unsigned long *ulRunTime)
{
	*ulRunTime = ulFunc_CalculateRunTime_Start();

	return (VS_SUCCESS);
}

/*
Function        :inSIGN_CheckSignature
Date&Time       :2017/7/24 下午 2:24
Describe        :請收銀員核對簽名，正確請按[0]，簽名不符請按清除
*/
int inSIGN_CheckSignature(TRANSACTION_OBJECT *pobTran)
{
	unsigned long  ulFileHandle = 0;
	int inFileResult;
	int	inRetVal = VS_ERROR;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_SIGN_CHECK_;
	char	szKey = 0x00;
	char	szMemoSignBMPFile[32 + 1];
	char	szPath[100 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 恢復UI */
	inDISP_ClearAll();

	/* 初始化 用Invoice Number來命名 */
	memset(szMemoSignBMPFile, 0x00, sizeof(szMemoSignBMPFile));
	inFunc_ComposeFileName_InvoiceNumber(pobTran, szMemoSignBMPFile, _PICTURE_FILE_EXTENSION_, 6);
	
	inFileResult = lnFILE_GetSize(&ulFileHandle, (unsigned char *)szMemoSignBMPFile);
	inDISP_LogPrintfWithFlag("  SignBmpSize[%d] ", inFileResult);
	
	/* 前面要加上路徑 */
	memset(szPath, 0x00, sizeof(szPath));
	sprintf(szPath, "./fs_data/%s", szMemoSignBMPFile);
	
	inDISP_PutGraphic(_SIGNPAD_CHECK_SIGN_, 0, _COORDINATE_Y_LINE_8_4_);

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();

		/* Timeout 不自動下一步所以註解 */
//		/* Timeout */
//		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
//		{
//			szKey = _KEY_TIMEOUT_;
//		}

		if (szKey == _KEY_0_			||
		    inChoice == _SIGN_CHECK_Touch_KEY_1_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (szKey == _KEY_CANCEL_			||
			 inChoice == _SIGN_CHECK_Touch_KEY_2_)
		{
			/* 刪檔重簽 */
			inFILE_Delete((unsigned char*)szMemoSignBMPFile);

			/* 初始化簽名狀態 */
			pobTran->srBRec.inSignStatus = _SIGN_NONE_;

			inRetVal = VS_ERROR;
			break;
		}

	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	
	inDISP_LogPrintfWithFlag("  Result[%d] ", inRetVal);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (inRetVal);
}

/*
Function        :inSIGN_Check_NOSignature
Date&Time       :2017/7/28 下午 3:45
Describe        :確認是否真的不簽名，顯示
		"此交易非電子簽名
		 會列印紙本簽單
		 列印紙本請按【0】
		 電子簽名請按清除
		"
*/
int inSIGN_Check_NOSignature(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	int	inChoice = _DisTouch_No_Event_;
	int	inTouchSensorFunc = _Touch_NOSIGN_CHECK_;
	char	szKey = 0x00;
	char	szMemoSignBMPFile[32 + 1];

	/* 恢復UI */
	inDISP_ClearAll();

	/* 此交易非電子簽名 */
	inDISP_PutGraphic(_SIGNPAD_CHECK_NOSIGN_, 0, _COORDINATE_Y_LINE_8_4_);
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();

//		/* Timeout */
//		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
//		{
//			szKey = _KEY_TIMEOUT_;
//		}

		if (szKey == _KEY_0_			||
		    inChoice == _NOSIGN_CHECK_Touch_KEY_1_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (szKey == _KEY_CANCEL_			||
			 inChoice == _NOSIGN_CHECK_Touch_KEY_2_)
		{

			/* 刪檔重簽 */
			inFILE_Delete((unsigned char*)szMemoSignBMPFile);

			/* 初始化簽名狀態 */
			pobTran->srBRec.inSignStatus = _SIGN_NONE_;

			inRetVal = VS_ERROR;
			break;
		}
//		else if (szKey == _KEY_TIMEOUT_)
//		{
//			inRetVal = VS_SUCCESS;
//			break;
//		}
	}

	return (inRetVal);
}

/*----------------測試function------------------*/
int inSIGN_TouchSignature_Test(void)
{
	TRANSACTION_OBJECT pobTran;

	memset(&pobTran, 0x00, sizeof(pobTran));
	inSIGN_TouchSignature_Flow(&pobTran);

	return (VS_SUCCESS);
}


int inSIGN_CheckTransSign(TRANSACTION_OBJECT *pobTran)
{
	int inSign = 1 ;/* 1為正號 2為負號 */
	if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
	{
		switch (pobTran->srBRec.inCode)
		{
			case _REDEEM_SALE_ :
			case _REDEEM_ADJUST_ :
			case _INST_SALE_ :
			case _INST_ADJUST_ :
			case _SALE_ :
			case _SALE_OFFLINE_ :
			case _PRE_COMP_ :
			case _CUP_SALE_ :
			case _CUP_PRE_COMP_ :
				inSign = 1;
				break;
			case _REDEEM_REFUND_ :
			case _INST_REFUND_ :
			case _REFUND_ :
			case _CUP_REFUND_ :
				inSign = 2;
				break;
			case _TIP_ :

//				srAccumRec->lnTotalTipsCount ++;
//				srAccumRec->llTotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
//				srAccumRec->llTotalAmount += pobTran->srBRec.lnTipTxnAmount;
//
//				if (pobTran->srBRec.uszRedeemBit != VS_TRUE && pobTran->srBRec.uszInstallmentBit != VS_TRUE)
//				{
//
//					srAccumRec->lnTotalCreditTipsCount ++;
//					srAccumRec->llTotalCreditTipsAmount += pobTran->srBRec.lnTipTxnAmount;
//					srAccumRec->llTotalCreditAmount += pobTran->srBRec.lnTipTxnAmount;
//				}
				inSign = 1;
				break;
			case _ADJUST_:
//				srAccumRec->llTotalSaleAmount -= (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnAdjustTxnAmount);
//				srAccumRec->llTotalAmount -= (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnAdjustTxnAmount);
//
//				if (pobTran->srBRec.uszRedeemBit != VS_TRUE && pobTran->srBRec.uszInstallmentBit != VS_TRUE)
//				{
//					srAccumRec->llTotalCreditSaleAmount -= (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnAdjustTxnAmount);
//					srAccumRec->llTotalCreditAmount -= (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnAdjustTxnAmount);
//				}
				inSign = 1;
				break;
			default :
				return (inSign);
		}
	}
	/* 取消的時候 */
	else
	{
		switch (pobTran->srBRec.inOrgCode)
		{
			case _REDEEM_SALE_ :
			case _REDEEM_ADJUST_ :
			case _INST_SALE_ :
			case _INST_ADJUST_ :
			case _SALE_ :
			case _SALE_OFFLINE_ :
			case _PRE_COMP_ :
			case _CUP_SALE_ :
			case _CUP_PRE_COMP_ :
			case _ADJUST_ :
				inSign = 2;
				break;
			case _REDEEM_REFUND_ :
			case _INST_REFUND_ :
			case _REFUND_ :
			case _CUP_REFUND_ :
				inSign = 1;
				break;
			case _TIP_ :
				inSign = 2;
//				srAccumRec->lnTotalTipsCount --;
//				srAccumRec->lnTotalSaleCount --;
//				srAccumRec->llTotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
//				srAccumRec->llTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
//				srAccumRec->llTotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
//
//				if (pobTran->srBRec.uszRedeemBit != VS_TRUE && pobTran->srBRec.uszInstallmentBit != VS_TRUE)
//				{
//
//					srAccumRec->lnTotalCreditTipsCount --;
//					srAccumRec->lnTotalCreditSaleCount --;
//					srAccumRec->llTotalCreditTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
//					srAccumRec->llTotalCreditSaleAmount -= pobTran->srBRec.lnTxnAmount;
//					srAccumRec->llTotalCreditAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
//				}

				break;
			default :
				return (inSign);
		}
	}
	return (inSign);
}


int inSIGN_Check_Trans_For_Sign(TRANSACTION_OBJECT *pobTran)
{
	if(pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		inSIGN_Check_Inst_Amount_Confirm(pobTran);
		
	}else if(pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		inSIGN_Check_Redeem_Amount_Confirm(pobTran);
	}else if(pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		inSIGN_Check_Cup_Amount_Confirm(pobTran);
	}else{
		inDISP_LogPrintfWithFlag("  SIGN Check Err");
	}
	
	return VS_SUCCESS;
}

/*
Function        :inSIGN_Check_Esc_Upload_Fail_Confirm
Date&Time       : 2019/02/14
Describe        :
 *
*/
int inSIGN_Check_Esc_Upload_Fail_Confirm(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	int	inChoice = _DisTouch_No_Event_;
	int	inTouchSensorFunc = _Touch_TEST_;
	int	inWaitTimeOut = VS_FALSE; 
	char	szKey = 0x00;

#ifdef _FUBON_MAIN_HOST_	
	/* 富邦自助不使用確認畫面，直接列印 2020/3/6 下午 2:44 [SAM] */
	if(pobTran->uszKioskFlag == 'Y')
	{
		inDISP_DispLogAndWriteFlie(" Sign Esc Upload Fail  KioskFlag = Y Line[%d]", __LINE__);
		return VS_SUCCESS;
	}
#endif
	
	/* 確定金額正負符號  */
	inFunc_ResetTitle(pobTran);
	/* 電子簽名上傳失敗UI CHECK_EC_FAIL.bmp  */
	inDISP_PutGraphic(_SIGNPAD_CHECK_EC_FAIL_, 0, _COORDINATE_Y_LINE_8_4_);
	
	/* 設定TIME OUT 時間 */
	/* 目前富邦不設TIME OUT 時間，先拿掉  */
	if(inWaitTimeOut == VS_TRUE)
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
	
	/* 設定時間，30秒後要嗶聲提示 */
	inDISP_Timer_Start(_TIMER_NEXSYS_2_, 30);

	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();

		/* Timeout */
		if(inWaitTimeOut == VS_TRUE)
		{
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}
		}
		
		if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_2_, 2);
			inDISP_BEEP(1, 0);
		}
		
		if (szKey == _KEY_ENTER_			||
		   (szKey == _KEY_TIMEOUT_ && inWaitTimeOut == VS_TRUE ) ||
		   inChoice == _SIGN_FAIL_ENTER_BUTTON_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
	}

	return (inRetVal);
}


/*
Function        :inSIGN_Check_Inst_Amount_Confirm
Date&Time       : 2019/02/14
Describe        :
 *
*/
int inSIGN_Check_Inst_Amount_Confirm(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	int	inChoice = _DisTouch_No_Event_;
	int	inTouchSensorFunc = _Touch_TEST_;
	int	inSignStatus = 0 ; /* 1為正號 2為負號 */
	int	inWaitTimeOut = VS_FALSE; 
	char	szKey = 0x00;
	char szDisplayTemp[32];

	/* 確定金額正負符號  */
	inSignStatus = inSIGN_CheckTransSign(pobTran);
	
	/* 重新顯示表頭 */
	inFunc_ResetTitle(pobTran);
			
	/* 分期付款 */
	inDISP_PutGraphic(_SIGNPAD_CHECK_INST_CONF_, 0, _COORDINATE_Y_LINE_8_4_);
	
	memset(szDisplayTemp, 0x00, sizeof(szDisplayTemp));
	if( inSignStatus == 1)
		sprintf(szDisplayTemp, "%ld", pobTran->srBRec.lnTotalTxnAmount);
	else
		sprintf(szDisplayTemp, "-%ld", pobTran->srBRec.lnTotalTxnAmount);
	inFunc_Amount(szDisplayTemp, "$", ' ', _SIGNED_NONE_, 11, _PAD_RIGHT_FILL_LEFT_);
	inDISP_EnglishFont_Point_Color(szDisplayTemp, _FONTSIZE_16X16_, _LINE_16_7_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	
	memset(szDisplayTemp, 0x00, sizeof(szDisplayTemp));
	sprintf(szDisplayTemp, "%11ld", pobTran->srBRec.lnInstallmentPeriod);
	inDISP_EnglishFont_Point_Color(szDisplayTemp, _FONTSIZE_16X16_, _LINE_16_8_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	
	memset(szDisplayTemp, 0x00, sizeof(szDisplayTemp));
	if( inSignStatus == 1)
		sprintf(szDisplayTemp, "%ld", pobTran->srBRec.lnInstallmentDownPayment);
	else
		sprintf(szDisplayTemp, "-%ld", pobTran->srBRec.lnInstallmentDownPayment);
	
	inFunc_Amount(szDisplayTemp, "$", ' ', _SIGNED_NONE_, 11, _PAD_RIGHT_FILL_LEFT_);
	inDISP_EnglishFont_Point_Color(szDisplayTemp, _FONTSIZE_16X16_, _LINE_16_9_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	
	
	memset(szDisplayTemp, 0x00, sizeof(szDisplayTemp));
	
	if( inSignStatus == 1)
		sprintf(szDisplayTemp, "%ld", pobTran->srBRec.lnInstallmentPayment);
	else
		sprintf(szDisplayTemp, "-%ld", pobTran->srBRec.lnInstallmentPayment);
	
	inFunc_Amount(szDisplayTemp, "$", ' ', _SIGNED_NONE_, 11, _PAD_RIGHT_FILL_LEFT_);
	inDISP_EnglishFont_Point_Color(szDisplayTemp, _FONTSIZE_16X16_, _LINE_16_10_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	
	memset(szDisplayTemp, 0x00, sizeof(szDisplayTemp));
	if( inSignStatus == 1)
		sprintf(szDisplayTemp, "%ld", pobTran->srBRec.lnInstallmentFormalityFee);
	else
		sprintf(szDisplayTemp, "-%ld", pobTran->srBRec.lnInstallmentFormalityFee);
	
	inFunc_Amount(szDisplayTemp, "$", ' ', _SIGNED_NONE_, 11, _PAD_RIGHT_FILL_LEFT_);
	inDISP_EnglishFont_Point_Color(szDisplayTemp, _FONTSIZE_16X16_, _LINE_16_11_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	
	
	/* 設定TIME OUT 時間 */
	/* 目前富邦不設TIME OUT 時間，先拿掉  */
	if(inWaitTimeOut == VS_TRUE)
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);

	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();

		/* Timeout */
		if(inWaitTimeOut == VS_TRUE)
		{
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}
		}
		
		if (szKey == _KEY_ENTER_			||
		   (szKey == _KEY_TIMEOUT_ && inWaitTimeOut == VS_TRUE ) ||
		   inChoice == _SIGN_FAIL_ENTER_BUTTON_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
	}

	return (inRetVal);
}

/*
Function        :inSIGN_Check_Redeem_Amount_Confirm
Date&Time       : 2019/02/14
Describe        :
 *
*/
int inSIGN_Check_Redeem_Amount_Confirm(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	int	inDispBlanacePoint = 1;
	int	inChoice = _DisTouch_No_Event_;
	int	inTouchSensorFunc = _Touch_TEST_;
	int	inSignStatus = 0;
	int	inWaitTimeOut = VS_FALSE; 
	char	szKey = 0x00;
	char szDisplayTemp[32];

	/* 確定金額正負符號  */
	inSignStatus = inSIGN_CheckTransSign(pobTran);
	
	/* 判斷是否要顯示餘額點數 20190220 [SAM] */
	if(pobTran->srBRec.uszVOIDBit == VS_TRUE || pobTran->srBRec.inCode ==_REDEEM_REFUND_)
		inDispBlanacePoint = 0;
	
	/* 重新顯示表頭 */
	inFunc_ResetTitle(pobTran);
	
	/* 紅利抵扣顯示金額圖片  */
	if( inDispBlanacePoint == 1)
		inDISP_PutGraphic(_SIGNPAD_CHECK_REDE_CONF_, 0, _COORDINATE_Y_LINE_8_4_);
	else
		inDISP_PutGraphic(_SIGNPAD_CHECK_REDE_REFUND_CONF_, 0, _COORDINATE_Y_LINE_8_4_);
	
	
	memset(szDisplayTemp, 0x00, sizeof(szDisplayTemp));
	if( inSignStatus == 1)
		sprintf(szDisplayTemp, "%ld", pobTran->srBRec.lnTotalTxnAmount);
	else
		sprintf(szDisplayTemp, "-%ld", pobTran->srBRec.lnTotalTxnAmount);
	inFunc_Amount(szDisplayTemp, "$", ' ', _SIGNED_NONE_, 11, _PAD_RIGHT_FILL_LEFT_);
	inDISP_EnglishFont_Point_Color(szDisplayTemp, _FONTSIZE_16X16_, _LINE_16_8_, _COLOR_BLACK_,_COLOR_WHITE_, 12);
	
		
	memset(szDisplayTemp, 0x00, sizeof(szDisplayTemp));
	if( inSignStatus == 1)
		sprintf(szDisplayTemp, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount);
	else
		sprintf(szDisplayTemp, "-%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount);
	inFunc_Amount(szDisplayTemp, "$", ' ', _SIGNED_NONE_, 11, _PAD_RIGHT_FILL_LEFT_);
	inDISP_EnglishFont_Point_Color(szDisplayTemp, _FONTSIZE_16X16_, _LINE_16_9_, _COLOR_BLACK_,_COLOR_WHITE_, 12);
	
		
	memset(szDisplayTemp, 0x00, sizeof(szDisplayTemp));
	if( inSignStatus == 1)
		sprintf(szDisplayTemp, "%ld", pobTran->srBRec.lnRedemptionPaidAmount);
	else
		sprintf(szDisplayTemp, "-%ld", pobTran->srBRec.lnRedemptionPaidAmount);
	inFunc_Amount(szDisplayTemp, "$", ' ', _SIGNED_NONE_, 11, _PAD_RIGHT_FILL_LEFT_);
	inDISP_EnglishFont_Point_Color(szDisplayTemp, _FONTSIZE_16X16_, _LINE_16_10_, _COLOR_BLACK_,_COLOR_WHITE_, 12);
	
	
	memset(szDisplayTemp, 0x00, sizeof(szDisplayTemp));
	if( inSignStatus == 1)
		sprintf(szDisplayTemp, "%ld", pobTran->srBRec.lnRedemptionPoints);
	else
		sprintf(szDisplayTemp, "-%ld", pobTran->srBRec.lnRedemptionPoints);	
	inFunc_Amount(szDisplayTemp, "", ' ', _SIGNED_NONE_, 11, _PAD_RIGHT_FILL_LEFT_);
	inDISP_EnglishFont_Point_Color(szDisplayTemp, _FONTSIZE_16X16_, _LINE_16_11_, _COLOR_BLACK_,_COLOR_WHITE_, 12);
	
	if( inDispBlanacePoint == 1)
	{
		memset(szDisplayTemp, 0x00, sizeof(szDisplayTemp));
		sprintf(szDisplayTemp, "%ld", pobTran->srBRec.lnRedemptionPointsBalance);
		inFunc_Amount(szDisplayTemp, "", ' ', _SIGNED_NONE_, 11, _PAD_RIGHT_FILL_LEFT_);
		inDISP_EnglishFont_Point_Color(szDisplayTemp, _FONTSIZE_16X16_, _LINE_16_12_, _COLOR_BLACK_,_COLOR_WHITE_, 12);
	}
	
	
	/* 設定TIME OUT 時間 */
	if(inWaitTimeOut == VS_TRUE)
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);

	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();

		/* Timeout */
		if(inWaitTimeOut == VS_TRUE)
		{
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}
		}
		
		if (szKey == _KEY_ENTER_			||
		(szKey == _KEY_TIMEOUT_ && inWaitTimeOut == VS_TRUE ) ||
		 inChoice == _SIGN_FAIL_ENTER_BUTTON_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
	}

	return (inRetVal);
}


/*
Function        :inSIGN_Check_Redeem_Amount_Confirm
Date&Time       : 2019/02/14
Describe        :
 *
*/
int inSIGN_Check_Cup_Amount_Confirm(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	int	inChoice = _DisTouch_No_Event_;
	int	inTouchSensorFunc = _Touch_TEST_;
	int	inSignStatus = 0;
	int	inWaitTimeOut = VS_FALSE; 
	char	szKey = 0x00;
	char szDisplayTemp[32];

	/* 確定金額正負符號  */
	inSignStatus = inSIGN_CheckTransSign(pobTran);
	
	/* 重新顯示表頭 */
	inFunc_ResetTitle(pobTran);
	
	/* 銀聯交易 */
	inDISP_PutGraphic(_SIGNPAD_CHECK_CUP_CONF_, 0, _COORDINATE_Y_LINE_8_4_);
	
	memset(szDisplayTemp, 0x00, sizeof(szDisplayTemp));
	if( inSignStatus == 1)
		sprintf(szDisplayTemp, "%ld", pobTran->srBRec.lnTotalTxnAmount);
	else
		sprintf(szDisplayTemp, "-%ld", pobTran->srBRec.lnTotalTxnAmount);
	inFunc_Amount(szDisplayTemp, "$", ' ', _SIGNED_NONE_, 11, _PAD_RIGHT_FILL_LEFT_);
	inDISP_EnglishFont_Point_Color(szDisplayTemp, _FONTSIZE_16X16_, _LINE_16_8_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	
		
	/* 設定TIME OUT 時間 */
	if(inWaitTimeOut == VS_TRUE)
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);

	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();

		/* Timeout */
		if(inWaitTimeOut == VS_TRUE)
		{
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}
		}
		
		if (szKey == _KEY_ENTER_  ||
		    (szKey == _KEY_TIMEOUT_ && inWaitTimeOut == VS_TRUE ) ||
			inChoice == _SIGN_FAIL_ENTER_BUTTON_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
	}

	return (inRetVal);
}

/*
Function        : inSIGN_DisplayBlackLineOnBack
Date&Time   : 2019/6/25 
Describe        : 將要簽名的背景區塊塗黑，要簽名時，會把中間留白，變成框線
*/
int inSIGN_DisplayBlackLineOnBack(unsigned short usX, unsigned short usY, unsigned short usXSize, unsigned short usYSize)
{
	unsigned short	usRetVal = 0;

	/* 將要簽名的背景區塊塗黑 */
	usRetVal = CTOS_LCDGSetBox(usX, usY, usXSize, usYSize, d_LCD_FILL_1);
	if (usRetVal != d_OK)
	{
		inDISP_DispLogAndWriteFlie("  BMP LCD Set Box *Error* Result[0x%04x] Line[%d]", usRetVal, __LINE__);
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}




/*
Function        : inSIGN_CheckSignatureOfHorizontalView
Date&Time   : 20190627 [SAM]
Describe        : 橫向顯示請收銀員核對簽名，正確請按[0]，簽名不符請按清除
*/
int inSIGN_CheckSignatureOfHorizontalView(TRANSACTION_OBJECT *pobTran)
{
	unsigned long ulFileHandle =0;
	int inFileResult;
	int	inRetVal = VS_ERROR;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_CheckConfirmSignature_;
	char	szKey = 0x00;
	char	szMemoSignBMPFile[32 + 1];
	char	szTempSignBmpPath[100 + 1];

	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 恢復UI */
	inDISP_ClearAll();

	/* 原簽名圖檔用 */
	/* 初始化 用Invoice Number來命名 */
	memset(szMemoSignBMPFile, 0x00, sizeof(szMemoSignBMPFile));
	inFunc_ComposeFileName_InvoiceNumber(pobTran, szMemoSignBMPFile, _PICTURE_FILE_EXTENSION_, 6);
	

	inFileResult = lnFILE_GetSize(&ulFileHandle,(unsigned char *) szMemoSignBMPFile);
	inDISP_LogPrintfWithFlag("  SignBmpSize[%d] ", inFileResult);
	
	/* 確認横向簽名圖檔用 */
	memset(szTempSignBmpPath, 0x00, sizeof(szTempSignBmpPath));
	sprintf(szTempSignBmpPath, "./fs_data/%s", TempSignBmpFile);

	inDISP_LogPrintfWithFlag("  Sign Position[%d] ", gsrSignpad.inPosition);
	
	if (gsrSignpad.inPosition == _SIGNPAD_LEFT_)
	{
		inDISP_PutGraphic(_SIGNATURE_CHECK_SIGN_LEFT_, 0, _COORDINATE_Y_LINE_8_1_);
		inDISP_PutGraphic(szTempSignBmpPath, 0, _COORDINATE_Y_LINE_8_1_);
	}else{
		
		inDISP_PutGraphic(_SIGNATURE_CHECK_SIGN_RIGHT_, 0, _COORDINATE_Y_LINE_8_1_);
		/* 顯示要確認的横向圖片  */
		inDISP_PutGraphic(szTempSignBmpPath, 60, _COORDINATE_Y_LINE_8_1_);
	}
	
//	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();

		/* Timeout 不自動下一步所以註解 */
//		/* Timeout */
//		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
//		{
//			szKey = _KEY_TIMEOUT_;
//		}

		if (szKey == _KEY_ENTER_			||
		    inChoice == _SIGN_CHECK_Touch_KEY_1_)
		{
			inRetVal = VS_SUCCESS;
			
			/* 刪掉横向簽名備份檔 */
			inFILE_Delete((unsigned char *)TempSignBmpFile);
			break;
		}
		else if (szKey == _KEY_CANCEL_			||
			 inChoice == _SIGN_CHECK_Touch_KEY_2_)
		{
			/* 刪檔簽名檔重簽 */
			inFILE_Delete((unsigned char*)szMemoSignBMPFile);

			/* 刪掉横向簽名備份檔 */
			inFILE_Delete((unsigned char *)TempSignBmpFile);
			
			/*  FILE DEBUG  MUST BE DELETE*/
//			inFunc_ls("-R", "./");
			
			/* 初始化簽名狀態 */
			pobTran->srBRec.inSignStatus = _SIGN_NONE_;

			inRetVal = VS_ERROR;
			break;
		}

	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	inDISP_LogPrintfWithFlag("  Result[%d] ", inRetVal);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (inRetVal);
}




/*
Function        : inSIGN_TestSignatureView
Date&Time   : 20190702
Describe        : [測試簽名用] 啟動簽名
*/
int inSIGN_TestSignatureView( TRANSACTION_OBJECT *pobTran)
{
	char	szSignPadMode[2 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* CFGT的Signpad開關判斷 */
	memset(szSignPadMode , 0x00, sizeof(szSignPadMode));
	inGetSignPadMode(szSignPadMode);

	/* 需要把全域使用的變數初始化,才不會影響到  inSIGN_TouchSignature_Internal() 裏的判斷 */
	gsrSignpad.inHasBeenSelect = _SIGNPAD_STATUS_INIT_ ;	
		
	while (1)
	{
		/* 不開signpad或免簽 */
		if (!memcmp(szSignPadMode, _SIGNPAD_MODE_0_NO_, 1))
		{
			inDISP_LogPrintfWithFlag( "不支援簽名版");
			return (VS_SUCCESS);
		}
		
		/* 使用內建簽名板 */
		else if (!memcmp(szSignPadMode, _SIGNPAD_MODE_1_INTERNAL_, strlen(_SIGNPAD_MODE_1_INTERNAL_)))
		{
#ifndef	_TOUCH_CAPBILITY_
	return (VS_SUCCESS);
#endif
			inDISP_LogPrintfWithFlag("使用內建簽名板internal");
	
			inSIGN_TestTouchSignature_Internal(pobTran);
			
			return VS_USER_CANCEL;
		}
	

	}
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (VS_SUCCESS);

}



/*
Function        : inSIGN_TestTouchSignature_Internal
Date&Time   : 20190702
Describe        : [測試簽名用] SignPad內建觸控簽名功能
*/
int inSIGN_TestTouchSignature_Internal(TRANSACTION_OBJECT *pobTran)
{
	int		inChoice;
	int		inTouchSensorFunc;
	unsigned char   uszKey;

	/* 初始化結構 */
	memset(&gsrSignpad, 0x00, sizeof(gsrSignpad));

	/* 預設為左邊 */
	gsrSignpad.inPosition = _SIGNPAD_RIGHT_;

	inTouchSensorFunc = _Touch_SIGNATURE_;
	inSIGN_TouchSignature_Internal_START(pobTran, &gsrSignpad);

	/* 清鍵盤buffer */
	inFlushKBDBuffer();

	while (1)
	{
		uszKey = uszKBD_Key();
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);

		/* 判斷是否在簽名板下筆 */
		if (inChoice == _SignTouch_Signpad_)
		{
			gsrSignpad.inSigned = VS_TRUE;
			/* 簽名板，於落筆簽名後，應重新計算timeout時間。 */
			inSIGN_TimeoutStart(&gsrSignpad.ulSignTimeStart);
		}

		/* 確認TimeOut並顯示 */
		/* 進入簽名板流程後，應於1分鐘後開始提示聲 */
		if (inSIGN_TimeoutCheck(&gsrSignpad, 60, VS_FALSE) == VS_TIMEOUT)
		{
			inDISP_BEEP(1, 2000);
		}
		
		/* 1分30秒後Timeout。 */
		/*if (inSIGN_TimeoutCheck(&gsrSignpad, 90, VS_FALSE) == VS_TIMEOUT)
		{
			uszKey = _KEY_TIMEOUT_;
		}*/

		if (inChoice == _SignTouch_Clear_ || uszKey == _KEY_CANCEL_)
		{
			inSIGN_TouchSignature_Internal_END();
			inSIGN_TouchSignature_Internal_START(pobTran, &gsrSignpad);
		}
		else if (inChoice == _SignTouch_Ok_ || uszKey == _KEY_ENTER_ || uszKey == _KEY_TIMEOUT_)
		{
			inSIGN_TouchSignature_Internal_END();
			/* 因為是測試要刪檔 */
			inFunc_Delete_Data("", SignBMPFile, _AP_ROOT_PATH_);
			return (VS_SUCCESS);
		}
		else if (inChoice == _SignTouch_Rotate_ || uszKey == d_KBD_DOT)
		{

			/* 判斷是否已經下筆 有下筆則擋住旋轉，（因為判斷整個畫面，所以按畫面上的按鈕也算下筆） */
			/* 現在將條件從d_SCP_STATUS_SIGNING && Duration為 0改為 Duration為 0，可以讓按一下或一直押著都旋轉，但仍無法判斷 */
			if (inSIGN_IsSigned(&gsrSignpad) != VS_SUCCESS)
			{
				/* Signature Status為整個螢幕檢測 使用觸控旋轉的話 結果會是d_SCP_STATUS_SIGNING  Duration為 0 */
			}
			else
			{
				continue;
			}

			inSIGN_TouchSignature_Internal_END();

			if (gsrSignpad.inPosition == _SIGNPAD_LEFT_)
			{
				gsrSignpad.inPosition = _SIGNPAD_RIGHT_;
				inSIGN_TouchSignature_Internal_START(pobTran, &gsrSignpad);
			}
			else if (gsrSignpad.inPosition == _SIGNPAD_RIGHT_)
			{
				gsrSignpad.inPosition = _SIGNPAD_LEFT_;
				inSIGN_TouchSignature_Internal_START(pobTran, &gsrSignpad);
			}

		}
		else
		{
				continue;
		}

	}

}


/*
Function        :inSIGN_TouchSignature_Internal_Calibration
Date&Time       :2020/7/15 下午 1:46
Describe        :SignPad內建觸控校正ICs
*/
int inSIGN_TouchSignature_Internal_Calibration(void)
{
	char		szDebugMsg[100 + 1] = {0};

	CTOS_TouchSignatureCalibration();
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Touch IC Calibration");
		inDISP_LogPrintf(AT, szDebugMsg);
	}

	
	return (VS_SUCCESS);
}



/*
Function        : inSIGN_CheckSignatureOfVerticalView
Date&Time   : 2022/12/5 下午 2:22
Describe        : 直向顯示請收銀員核對簽名，正確請按[0]，簽名不符請按清除
 * [外接設備設定]
*/
int inSIGN_CheckSignatureOfVerticalView(TRANSACTION_OBJECT *pobTran)
{
	unsigned long ulFileHandle =0;
	int inFileResult;
	int	inRetVal = VS_ERROR;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_CheckConfirmSignature_;
	char	szKey = 0x00;
	char	szMemoSignBMPFile[32 + 1];
	char	szTempSignBmpPath[100 + 1];
	char	szDispBuf[24] = {0};
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	/* 恢復UI */
	inDISP_ClearAll();

	/* 原簽名圖檔用 */
	/* 初始化 用Invoice Number來命名 */
	memset(szMemoSignBMPFile, 0x00, sizeof(szMemoSignBMPFile));
	inFunc_ComposeFileName_InvoiceNumber(pobTran, szMemoSignBMPFile, _PICTURE_FILE_EXTENSION_, 6);
	

	inFileResult = lnFILE_GetSize(&ulFileHandle,(unsigned char *) szMemoSignBMPFile);
	inDISP_LogPrintfWithFlag("  SignBmpSize[%d] Name[%s] ", inFileResult, szMemoSignBMPFile);
	
	/* 確認横向簽名圖檔用 */
	memset(szTempSignBmpPath, 0x00, sizeof(szTempSignBmpPath));
	sprintf(szTempSignBmpPath, "./fs_data/%s", szMemoSignBMPFile);

	inDISP_PutGraphic(szTempSignBmpPath, 64, _COORDINATE_Y_LINE_8_1_);
	/* 提示檢核簽名和授權碼 */
	inDISP_PutGraphic(_CHECK_SIGNATURE_1_, 0, _COORDINATE_Y_LINE_8_5_);
	memset(szDispBuf, 0x00, sizeof(szDispBuf));
	sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
	inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_7_, _COLOR_RED_, _COLOR_WHITE_, 11);
	
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();

		if (szKey == _KEY_ENTER_			||
		    inChoice == _SIGN_CHECK_Touch_KEY_1_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (szKey == _KEY_CANCEL_			||
			 inChoice == _SIGN_CHECK_Touch_KEY_2_)
		{
			/* 刪檔簽名檔重簽 */
			inFILE_Delete((unsigned char*)szMemoSignBMPFile);

			/* 初始化簽名狀態 */
			pobTran->srBRec.inSignStatus = _SIGN_NONE_;
			inRetVal = VS_ERROR;
			break;
		}

	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	inDISP_LogPrintfWithFlag("  Result[%d] ", inRetVal);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (inRetVal);
}



