
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>
#include <emv_cl.h>

#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/DISPLAY/DisTouch.h"

#include "../SOURCE/FUNCTION/Function.h"

#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/CFGT.h"

#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Flow.h"

#include "../SOURCE/FUNCTION/ECR_FUNC/ECR.h"
#include "../SOURCE/FUNCTION/ECR_FUNC/RS232.h"

#include "../SOURCE/FUNCTION/MULTI_FUNC/MultiFunc.h"
#include "../SOURCE/FUNCTION/MULTI_FUNC/MultiHostFunc.h"
#include "../SOURCE/FUNCTION/MULTI_FUNC/JsonMultiHostFunc.h"

#include "../EMVSRC/EMVsrc.h"



extern  int     ginIdleMSRStatus, ginMenuKeyIn, ginIdleICCStatus;
extern  int     ginEventCode;
extern MULTI_TABLE	gstMultiOb;	/* [修改外接感應設備] 2022/11/22 [SAM] */

/*
Function		: inSVC_SetSTAN
Date&Time		: 2022/9/14 上午 11:54
Describe		: 
 * 抓取 HDPT 檔案中的 STAN資料
 */
int inSVC_GetSTAN(TRANSACTION_OBJECT *pobTran)
{
	char szSTANNum[12 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memset(szSTANNum, 0x00, sizeof (szSTANNum));
	if (inGetSTANNum(szSTANNum) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" SVC GetSTAN *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	pobTran->srBRec.lnSTANNum = atol(szSTANNum);

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}


/*
Function		: inSVC_SetSTAN
Date&Time		: 2022/9/14 上午 11:54
Describe		: 
 * 抓取 HDPT 檔案中的 STAN資料，加1後再存回 HDPT 檔案中。
 */
int inSVC_SetSTAN(TRANSACTION_OBJECT *pobTran)
{
	long lnSTAN;
	char szSTANNum[12 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memset(szSTANNum, 0x00, sizeof (szSTANNum));
	if (inGetSTANNum(szSTANNum) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" SVC GetSTAN *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	lnSTAN = atol(szSTANNum);
	if (lnSTAN++ > 999999)
		lnSTAN = 1;

	memset(szSTANNum, 0x00, sizeof (szSTANNum));
	sprintf(szSTANNum, "%06ld", lnSTAN);
	if (inSetSTANNum(szSTANNum) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" SVC SetSTAN *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) < 0)
	{
		inDISP_DispLogAndWriteFlie(" SVC SetSTAN Save Hdt[%d] *Error* Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}


/*
Function        : inSVC_SyncHostTerminalDateTime
Date&Time   : 2022/9/15 下午 12:05
Describe        : 同步主機時間
 *  TODO: 要再加入檢查功能，想一下要怎麼做
 */
int inSVC_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran)
{

	inDISP_LogPrintfWithFlag(" SVC_SyncHostBrecDate[%s] to Terminal", pobTran->srBRec.szDate);
	inDISP_LogPrintfWithFlag(" SVC_SyncHostBrecTime[%s] to Terminal", pobTran->srBRec.szTime);

	inFunc_SetEDCDateTime(pobTran->srBRec.szDate, pobTran->srBRec.szTime);
	return (VS_SUCCESS);
}


int inSVC_CalMac(TRANSACTION_OBJECT *pobTran,char* szInputData,int inInputDataLen)
{
	int     inForMacLen, inForMacBuf;
	int     inMACLen = 0;
	char    szMACBuf[ 8 + 1 ], MACResult[ 8 + 1 ];
	char    DiversifyData[16+1];
	char    TerminalIDAscData[8+1];
	char    TerminalIDBcdData[4+1];
	char    InputDataBuffer[1024];
	char    MacCommandBuffer[1024];
	int     i = 0 ;
	int     MacCommandBufferLen = 0 ;

	inDISP_LogPrintfWithFlag("inSVC_CalMac(inInputDataLen[%d]) START!!", inInputDataLen);
	inDISP_LogPrintfWithFlag("szInputData([%s])", szInputData);

	memset( szMACBuf, 0x00, sizeof( szMACBuf ) );
	memset( MACResult, 0x00, sizeof( MACResult ) );
	memset( InputDataBuffer, 0x00, sizeof( InputDataBuffer ) );
	memset( MacCommandBuffer, 0x00, sizeof( MacCommandBuffer ) );

	memcpy(InputDataBuffer, szInputData, inInputDataLen);
	memset( (char*)&InputDataBuffer[inInputDataLen], 0xff, 8 );//補不足八的部分

	if ( 0 != ( inInputDataLen % 8 ) )
	{
		inMACLen = 8 - ( inInputDataLen % 8 ) + inInputDataLen;
	}
	else
		inMACLen = inInputDataLen;

	memcpy( &szMACBuf[ 0 ], (char*)&InputDataBuffer[ 0 ], 8 );//added by HsinChun, 因為從8之後開始 xor.

	for( inForMacLen = 8 ;  inForMacLen < inMACLen ; inForMacLen = inForMacLen + 8 )
	{
		for( inForMacBuf = 0 ; inForMacBuf < 8 ; inForMacBuf++ )
		{
			szMACBuf[ inForMacBuf ] = szMACBuf[ inForMacBuf ] ^ InputDataBuffer[ inForMacLen + inForMacBuf ];
		}
	}

	memset(DiversifyData,0x00,sizeof(DiversifyData));
	memset(TerminalIDAscData,0x00,sizeof(TerminalIDAscData));
	memset(TerminalIDBcdData,0x00,sizeof(TerminalIDBcdData));
// TODO 要補回規則
//        strcpy((char *)TerminalIDAscData, szGetTerminalID());
//        SVC_DSP_2_HEX(TerminalIDAscData, TerminalIDBcdData, 8);
	memcpy(TerminalIDAscData, "\x01\x23\x45\x67", 4);
	memcpy(DiversifyData,TerminalIDBcdData,4);

	for( i = 0 ; i < 4; i++)
	{
		DiversifyData[i+4] = ~DiversifyData[i];
	}

	memset(&DiversifyData[8],0xFF, 8);

	MacCommandBuffer [MacCommandBufferLen] = 0x01; //Data Diversify Alg
	MacCommandBufferLen = MacCommandBufferLen + 1;
	MacCommandBuffer [MacCommandBufferLen] = 16; //Data Diversify Length
	MacCommandBufferLen = MacCommandBufferLen + 1;
	memcpy(&MacCommandBuffer [MacCommandBufferLen],DiversifyData,16);
	MacCommandBufferLen = MacCommandBufferLen + 16;

	MacCommandBuffer [MacCommandBufferLen] = 3; //Data Mac Alg.
	MacCommandBufferLen = MacCommandBufferLen + 1;
	MacCommandBuffer [MacCommandBufferLen] = 8; //Data Mac Length.
	MacCommandBufferLen = MacCommandBufferLen + 1;

	memcpy(&MacCommandBuffer [MacCommandBufferLen],szMACBuf,8);
	MacCommandBufferLen = MacCommandBufferLen + 8;

	memcpy(&MacCommandBuffer [MacCommandBufferLen],"\x00\x00\x00\x00\x00\x00\x00\x00",8);
	MacCommandBufferLen = MacCommandBufferLen + 8;

	if( (VS_SUCCESS) != inSVC_GenMac(pobTran, MacCommandBuffer, MacCommandBufferLen))
	{
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}


/*
Function	: inSVC_GetSvcCardFieldsFunction_CTLS
Date&Time	: 2022/12/26 下午 3:11
Describe	: 
*/
int inSVC_GetSvcCardFieldsFunction_CTLS(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inMSR_RetVal = -1;	/* 磁條事件的反應，怕和其他用到inRetVal的搞混所以獨立出來 */
	int		inEMV_RetVal = -1;	/* 晶片卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
	int		inCTLS_RetVal = -1;	/* 感應卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
	char		szKey = -1;
	char		szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	long		lnTimeout = 0;
	char		szCTLSMode[1 + 1] = {0};/* [修改外接感應設備] 2022/11/22 [SAM] */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* [修改外接感應設備] 2022/11/22 [SAM] */
	memset(szCTLSMode, 0x00, sizeof(szCTLSMode));
	inGetContactlessReaderMode(szCTLSMode);
	
	/* 過卡方式參數初始化  */
	pobTran->srBRec.uszManualBit = VS_FALSE;
	/* 初始化 ginEventCode */
	ginEventCode = -1;
	
	/*  */
	if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
            !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)) )
	{
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Not External  END -----",__FILE__, __FUNCTION__, __LINE__);
		return VS_ERROR;
	}
			
	/* [外接設備設定] */
	if(inMultiFunc_JsonCallSlave(pobTran, _MULTI_READ_MIFARE_NO_, &gstMultiOb) != VS_SUCCESS)
	{
		return VS_ERROR;
	}
		
		
//	inRetVal = inMultiFunc_JsonCallSlave(pobTran, _MULTI_READ_MIFARE_NO_, &gstMultiOb);
//	
//	/* Send CTLS Readly for Sale Command */
//	if (inCTLS_SendReadyForSale_Flow(pobTran) != VS_SUCCESS)
//	{
//		/* 如果Send 失敗，轉成沒感應的界面 */
//		inRetVal = inFunc_GetCardFields_ICC(pobTran);
//
//		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] Send CTLS *Error* END -----",__FILE__, __FUNCTION__, __LINE__);		
//		return (inRetVal);
//	}
	
	/* 顯示對應交易別的感應畫面 */
	inCTLS_Decide_Display_Image(pobTran);

	/* [修改外接感應設備] 2022/11/22 [SAM] */
	/* 因為不使用內建時呼叫會當機 */
	if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
            !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)) )
	{
		/* 顯示虛擬LED(不閃爍)，但inCTLS_ReceiveReadyForSales(V3機型)也會顯示LED（閃爍），所以沒有實質作用 */
                inCTLS_LED_Wait_Start();
	}
	
	/* 進入畫面時先顯示金額 */
	if (pobTran->srBRec.lnTxnAmount > 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 16, VS_TRUE);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_LEFT_);
	}

	/* 設定Timeout */
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	if (pobTran->uszECRBit == VS_TRUE)
	{
		lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
	}
	else
	{
		lnTimeout = 30;
	}

	/* inSecond剩餘倒數時間 */
	inDISP_TimeoutStart(lnTimeout);

	/* 先設定為讀卡失敗，有成功再設定回沒設定錯誤碼 2022/2/18 [SAM] */
	inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_READER_CARD_ERROR_);
    
	while (1)
	{                  
		/* （idle畫面刷卡此function不會發生） or 刷卡事件發生 */
		if (ginIdleMSRStatus == VS_TRUE || ginEventCode == _SWIPE_EVENT_)
		{
			/* [修改外接感應設備] 2022/11/22 [SAM] */
			if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
                            !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)) )
			{
				if (gstMultiOb.stMulti_TransData.unzAlreadySend == VS_TRUE)
					inMultiFunc_Host_CallSendErrorFunc(pobTran, _ECR_RESPONSE_CODE_CTLS_HOST_CANCEL_);
			} else
			{
				/* 取消感應交易 */
				inCTLS_CancelTransacton_Flow();
			}

			return (VS_SUCCESS);
		}
		/* （idle畫面人工輸入此function不會發生）or Menu Keyin */
		else if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
		{
			/* 表示是手動輸入 */
			pobTran->srBRec.uszManualBit = VS_TRUE;

			/* [修改外接感應設備] 2022/11/22 [SAM] */
			if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
                            !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)) )
			{
				if (gstMultiOb.stMulti_TransData.unzAlreadySend == VS_TRUE)
					inMultiFunc_Host_CallSendErrorFunc(pobTran, _ECR_RESPONSE_CODE_CTLS_HOST_CANCEL_);
			} else
			{
				/* 取消感應交易 */
				inCTLS_CancelTransacton_Flow();
			}

			return (VS_SUCCESS);
		}
		/* （idle 插晶片卡此function不會發生）or 晶片卡事件 */
		else if  (ginIdleICCStatus == VS_TRUE || ginEventCode == _EMV_DO_EVENT_)
		{
			/* [修改外接感應設備] 2022/11/22 [SAM] */
			if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
                            !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)) )
			{
				if (gstMultiOb.stMulti_TransData.unzAlreadySend == VS_TRUE)
					inMultiFunc_Host_CallSendErrorFunc(pobTran, _ECR_RESPONSE_CODE_CTLS_HOST_CANCEL_);
			} else
			{
				/* 取消感應交易 */
				inCTLS_CancelTransacton_Flow();
			}
			return (VS_SUCCESS);
		}
		/* 感應事件 */
		else if (ginEventCode == _SENSOR_EVENT_)
		{

			/* [修改外接感應設備]  新增外接條件 2022/11/22 [SAM] */
			if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
                            !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)) )
			{
				inDISP_LogPrintfWithFlag("CTLS MODE :External");
				
				inCTLS_RetVal = inMultiFunc_CallSlaveCtlsResult(pobTran);
				
				switch (inCTLS_RetVal)
				{
					case VS_SUCCESS:
						inDISP_LogPrintfWithFlag("感應成功[ %d] Line[%d]", inCTLS_RetVal, __LINE__);
						pobTran->srBRec.uszContactlessBit = VS_TRUE;
						pobTran->fSvcActivedCardAuto = VS_FALSE;

						if(pobTran->srBRec.inCode == _SVC_ACTIVECARD_AUTORELOAD_)
						{
							pobTran->srBRec.inCode = _SVC_ACTIVECARD_;
							pobTran->fSvcActivedCardAuto = VS_TRUE;
						}
						/* 因為在一般流程 inCARD_LoadGetCDTIndex 會執行，所以SVC要補讀HDPT資料 */
						if( inFunc_Get_HDPT_General_Data(pobTran) != VS_SUCCESS)
						{
							inDISP_LogPrintfWithFlag(" Get HDPT DATA *Error* Line[%d]",  __LINE__);
							return (VS_ERROR);
						}
						
						inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_NOT_SET_ERROR_);
						return (VS_SUCCESS);
					default:
						inDISP_LogPrintfWithFlag("感應失敗[ %d] Line[%d]", inCTLS_RetVal, __LINE__);
						if (pobTran->inErrorMsg == _ERROR_CODE_V3_NONE_)
						{
							pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
						}
						return (VS_WAVE_ERROR);
				}
			} 
		}
		
		/* 進迴圈前先清MSR BUFFER */
		inCARD_Clean_MSR_Buffer();
		while (1)
		{
			ginEventCode = -1;

			/* ------------偵測刷卡------------------*/
			inMSR_RetVal = inCARD_MSREvent();

			if (inMSR_RetVal == VS_SUCCESS)
			{
				/* 刷卡事件 */
				ginEventCode = _SWIPE_EVENT_;
			}
			/* 回復錯誤訊息蓋掉的圖 */
			else if (inMSR_RetVal == VS_SWIPE_ERROR)
			{
				inFunc_ResetTitle(pobTran);
				inDISP_Msg_BMP(_ERR_READ_CARD_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 1, "", 0);

				/* 顯示對應交易別的感應畫面 */
				inCTLS_Decide_Display_Image(pobTran);

				/* 回復虛擬燈號 */
				EMVCL_ShowVirtualLED(NULL);

				/* 回復顯示金額 */
				if (pobTran->srBRec.lnTxnAmount > 0)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
					inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 16, VS_TRUE);
					inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_LEFT_);
				}

				/* 抓取 KioskFlag的值，如為 TRUE 就不用確認畫面 2020/3/6 下午 4:28 [SAM] */
				if(inFunc_GetKisokFlag() == VS_TRUE)
				{	
					inDISP_LogPrintfWithFlag(" FU Kiosk GetCardFields Ctls Line[%d]", __LINE__);
					inCTLS_CancelTransacton_Flow();
					/* Timeout */
					return (VS_TIMEOUT);
				}
			}

			/* ------------偵測晶片卡---------------- */
			inEMV_RetVal = inEMV_ICCEvent();
			if (inEMV_RetVal == VS_SUCCESS)
			{
				/* 晶片卡事件 */
				ginEventCode = _EMV_DO_EVENT_;
			}

			/* ------------偵測感應卡------------------ */
			inCTLS_RetVal = inCTLS_ReceiveReadyForSales_Flow(pobTran);
			inDISP_LogPrintfWithFlag(" FU GetCardFields inCTLS_RetVal[%d] Line[%d]", inCTLS_RetVal, __LINE__);
			
			/* [修改外接感應設備] 2022/11/22 [SAM] */
			if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
                            !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)) )
			{
				if (inCTLS_RetVal != VS_WAVE_NO_DATA)
				{	
					/* 感應卡事件 */
					ginEventCode = _SENSOR_EVENT_;					
				}
			} else
			{
				if (inCTLS_RetVal == VS_SUCCESS)
				{
					/* 感應卡事件 */
					ginEventCode = _SENSOR_EVENT_;
				}
			}
			
			
			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 感應倒數時間 && Display Countdown */
			if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
			{
				/* 感應時間到Timeout */
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
				/* [修改外接感應設備] 2022/11/22 [SAM] */
				if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
                                    !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)) )
				{
					if (gstMultiOb.stMulti_TransData.unzAlreadySend == VS_TRUE)
						inMultiFunc_Host_CallSendErrorFunc(pobTran, _ECR_RESPONSE_CODE_CTLS_HOST_CANCEL_);
				} else
				{
					/* 取消感應交易 */
					inCTLS_CancelTransacton_Flow();
				}
				/* Timeout */
				inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_TIMEOUT_);
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
				/* [修改外接感應設備] 2022/11/22 [SAM] */
				if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
                                    !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)) )
				{
					if (gstMultiOb.stMulti_TransData.unzAlreadySend == VS_TRUE)
						inMultiFunc_Host_CallSendErrorFunc(pobTran, _ECR_RESPONSE_CODE_CTLS_HOST_CANCEL_);
				} else
				{
					/* 取消感應交易 */
					inCTLS_CancelTransacton_Flow();
				}
				/* Cancel */
				return (VS_USER_CANCEL);
			}
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* key in事件 */
				ginEventCode = _MENUKEYIN_EVENT_;
			}

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}

		}/* while (1) 偵測事件迴圈...*/
	} /* while (1) 對事件做回應迴圈...*/

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (VS_SUCCESS);
}


int inSVC_DisplaySvcPoint(TRANSACTION_OBJECT *pobTran)
{
	char	szSvcKey;
	char	szDispMsg[42 + 1], szTemplate[42 + 1];
	long	lnBalaceAmount = 0;

	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII( szTemplate, (unsigned char*)pobTran->srBRec.szSvcPoint1, 5);
	lnBalaceAmount = atol(szTemplate)/100;
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "NT$ %lu", lnBalaceAmount);
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("購物卡餘額", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
	inDISP_ChineseFont("請按確認或清除鍵", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);


	szSvcKey = 0x00;
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
	inDISP_BEEP(3, 100);
	
	while (1) 
	{
		szSvcKey = uszKBD_Key();
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS) {
			szSvcKey = _KEY_TIMEOUT_;
		}

		if (szSvcKey == _KEY_ENTER_) {
			break;
		} else if (szSvcKey == _KEY_TIMEOUT_ || szSvcKey == _KEY_CLEAR_) {
			return (VS_ERROR);
		}
	}

	return (VS_SUCCESS);
}

/*
Function	: inSVC_FuncDisplayError
Date&Time	: 2022/12/29 上午 11:20
Describe	: 
 * 跑完 TRT 後，主要顯示的訊息
*/
int inSVC_FuncDisplayError(TRANSACTION_OBJECT *pobTran)
{
	char	szTemplate[50 + 1];

	/* 沒有設定Error code 跳過 */
	if (pobTran->inErrorMsg == 0x00)
	{
		return (VS_SUCCESS);
	}

	inDISP_LogPrintfWithFlag(" Func Disp inErrorMsg : %d", pobTran->inErrorMsg);

	/* 統一在這裡顯示錯誤訊息 */
	/* 結帳出錯一律顯示結帳失敗 */
	if (pobTran->inRunTRTID == _TRT_SETTLE_ ||
	    pobTran->inErrorMsg == _ERROR_CODE_V3_SETTLE_NOT_SUCCESS_)
	{
		/* 連動結帳時，不顯示請按清除鍵 */
		if (pobTran->uszAutoSettleBit == VS_TRUE)
		{
			inDISP_LogPrintfWithFlag(" Func Disp Error Msg in Auto Settle");
			inDISP_Msg_BMP(_ERR_SETTLE_FAILED_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 2, "", 0);
		}
		else
		{
			inDISP_LogPrintfWithFlag(" Func Disp Error Msg in Single Settle");
			inDISP_Msg_BMP(_ERR_SETTLE_FAILED_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}

	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_USER_CANCEL_)
	{
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_USER_TERMINATE_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 2, "", 0);
		}
		else
		{
			inDISP_Msg_BMP(_ERR_USER_TERMINATE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_COMM_)
	{
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_CONNECT_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 2, "", 0);			/* 通訊失敗 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_CONNECT_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_ISO_PACK_	||
		 pobTran->inErrorMsg == _ERROR_CODE_V3_ISO_UNPACK_)
	{
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_ISO_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 2, "", 0);			/* 電文錯誤 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_ISO_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);				/* 電文錯誤 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_01_READ_FAIL_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&&
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
//			inFISC_CTLS_LED_TONE(VS_ERROR);
		}

		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_READ_CARD_, _COORDINATE_Y_LINE_8_6_, _NO_KEY_MSG_, 2, "", 0);			/* 讀卡失敗 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_READ_CARD_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);			/* 讀卡失敗 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_02_6982_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&&
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
//			inFISC_CTLS_LED_TONE(VS_ERROR);
		}

		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_CARD_FAIL_, _COORDINATE_Y_LINE_8_6_, _NO_KEY_MSG_, 2, "", 0);			/* 卡片失效 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_CARD_FAIL_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);			/* 卡片失效 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_06_TMS_NOT_SUPPORT_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&&
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
//			inFISC_CTLS_LED_TONE(VS_ERROR);
		}

		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_NOT_SUP_SMARTPAY_CTLS_CARD_, _COORDINATE_Y_LINE_8_5_, _NO_KEY_MSG_, 2, "", 0);		/* 不接受此感應卡 請改插卡 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_NOT_SUP_SMARTPAY_CTLS_CARD_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);		/* 不接受此感應卡 請改插卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_07_AMT_OVERLIMIT_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&&
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
//			inFISC_CTLS_LED_TONE(VS_ERROR);
		}

		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_OVER_SMARTPAY_CTLS_LIMIT_, _COORDINATE_Y_LINE_8_5_, _NO_KEY_MSG_, 2, "", 0);	/* 超過感應限額 請改插卡 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_OVER_SMARTPAY_CTLS_LIMIT_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);	/* 超過感應限額 請改插卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_09_NOT_RIGHT_INCODE_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&&
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
//			inFISC_CTLS_LED_TONE(VS_ERROR);
		}

		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_PLEASE_FOLLOW_RIGHT_OPT_, _COORDINATE_Y_LINE_8_5_, _NO_KEY_MSG_, 2, "", 0);		/* 請依正確卡別操作 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_PLEASE_FOLLOW_RIGHT_OPT_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);	/* 請依正確卡別操作 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_03_TIME_ERROR_	||
		 pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_04_MAC_TAC_		||
		 pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_05_NO_INCODE_	||
		 pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_08_NO_CARD_BIN_	||
		 pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_10_LOGON_FAIL_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&&
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
//			inFISC_CTLS_LED_TONE(VS_ERROR);
		}

		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_CTLS_RECEIVE_, _COORDINATE_Y_LINE_8_5_, _NO_KEY_MSG_, 2, "", 0);			/* 感應失敗 請改插卡或刷卡 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_CTLS_RECEIVE_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);		/* 感應失敗 請改插卡或刷卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_11_FISC_FALLBACK_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&&
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
//			inFISC_CTLS_LED_TONE(VS_ERROR);
		}

		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP("", 0, _NO_KEY_MSG_, 2, "請改插金融卡", _LINE_8_6_);			/* 請改插金融卡 */
		}
		else
		{
			inDISP_Msg_BMP("", 0, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "請改插金融卡", _LINE_8_6_);			/* 請改插金融卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_12_SEND_APDU_FAIL_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&&
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
//			inFISC_CTLS_LED_TONE(VS_ERROR);
		}

		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_CTLS_RECEIVE_, _COORDINATE_Y_LINE_8_5_, _NO_KEY_MSG_, 2, "", 0);			/* 感應失敗 請改插卡或刷卡 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_CTLS_RECEIVE_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);		/* 感應失敗 請改插卡或刷卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_MULTI_FUNC_CTLS_)
	{
		/* 感應失敗 請改插卡或刷卡 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_CTLS_RECEIVE_, _COORDINATE_Y_LINE_8_5_, _NO_KEY_MSG_, 2, "", 0);			/* 感應失敗 請改插卡或刷卡 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_CTLS_RECEIVE_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);		/* 感應失敗 請改插卡或刷卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_CTLS_REAL_)
	{
		/* 亮紅燈 */
//		inFISC_CTLS_LED_TONE(VS_ERROR);
		/* 不接受此感應卡 請改刷卡或插卡 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_NOT_SUP_CTLS_CARD_, _COORDINATE_Y_LINE_8_5_, _NO_KEY_MSG_, 2, "", 0);		/* 不接受此感應卡 請改刷卡或插卡 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_NOT_SUP_CTLS_CARD_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);		/* 不接受此感應卡 請改刷卡或插卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_OVER_AMOUNT_)
	{
		/* 亮紅燈 */
//		inFISC_CTLS_LED_TONE(VS_ERROR);
		/* 超過感應限額 請改插卡或刷卡 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_OVER_SMARTPAY_CTLS_LIMIT_, _COORDINATE_Y_LINE_8_5_, _NO_KEY_MSG_, 2, "", 0);		/* 超過感應限額 請改插卡 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_OVER_SMARTPAY_CTLS_LIMIT_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);		/* 超過感應限額 請改插卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_TIP_TOO_BIG_)
	{
		/* 亮紅燈 */
//		inFISC_CTLS_LED_TONE(VS_ERROR);
		/* 金額小於手續費 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_1_, _BEEP_1TIMES_MSG_, 2, "金額小於手續費", _LINE_8_6_);
		}
		else
		{
			inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_1_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "金額小於手續費", _LINE_8_6_);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_WAVE_ERROR_)
	{
		/* 感應失敗 請改插卡或刷卡 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_CTLS_RECEIVE_, _COORDINATE_Y_LINE_8_5_, _BEEP_1TIMES_MSG_, 2, "", 0);
		}
		else
		{
			inDISP_Msg_BMP(_ERR_CTLS_RECEIVE_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_WAVE_ERROR_NOT_ONE_CARD_)
	{
		/* 感應失敗 超過一張卡 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_CTLS_NOT_ONE_CARD_, _COORDINATE_Y_LINE_8_5_, _BEEP_1TIMES_MSG_, 2, "", 0);
		}
		else
		{
			inDISP_Msg_BMP(_ERR_CTLS_NOT_ONE_CARD_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC)
	{
		/* 亮紅燈 */
//		inFISC_CTLS_LED_TONE(VS_ERROR);
		/* 不接受此感應卡 請改刷卡或插卡 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_NOT_SUP_CTLS_CARD_, _COORDINATE_Y_LINE_8_5_, _NO_KEY_MSG_, 2, "", 0);		/* 不接受此感應卡 請改刷卡或插卡 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_NOT_SUP_CTLS_CARD_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);		/* 不接受此感應卡 請改刷卡或插卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FUNC_CLOSE_)
	{
		/* 此功能已關閉 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 2, "", 0);
		}
		else
		{
			inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_CTLS_DATA_SHORT_)
	{
		/* 感應資料不足 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_CTLS_DATA_SHORT_, _COORDINATE_Y_LINE_8_5_, _BEEP_1TIMES_MSG_, 2, "", 0);
		}
		else
		{
			inDISP_Msg_BMP(_ERR_CTLS_DATA_SHORT_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_WAVE_ERROR_Z1_)
	{
		/* 拒絕交易 Z1 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_USE_MS_OR_ICC_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 2, "拒絕交易 Z1", _LINE_8_5_);
		}
		else
		{
			inDISP_Msg_BMP(_ERR_USE_MS_OR_ICC_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "拒絕交易 Z1", _LINE_8_5_);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_TICKET_AMOUNT_TOO_MUCH_IN_ONE_TRANSACTION_)
	{
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_4_, _BEEP_3TIMES_MSG_, 2, "金額超過單筆上限", _LINE_8_5_);
		}
		else
		{
			inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_4_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "金額超過單筆上限", _LINE_8_5_);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_TICKET_AMOUNT_NOT_ENOUGH_)
	{
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_4_, _BEEP_3TIMES_MSG_, 2, "餘額不足", _LINE_8_5_);
		}
		else
		{
			inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_4_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "餘額不足", _LINE_8_5_);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_TRT_NOT_FOUND_)
	{
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_4_, _BEEP_3TIMES_MSG_, 2, "找不到TRT", _LINE_8_5_);
		}
		else
		{
			inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_4_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "找不到TRT", _LINE_8_5_);
		}
	}
	/* 手續費金額有誤 */
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_ECR_INST_FEE_NOT_0_)
	{
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_INSTFEE_NOT_0_, _COORDINATE_Y_LINE_8_4_, _BEEP_3TIMES_MSG_, 2, "", 0);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{

			inDISP_Msg_BMP(_ERR_INSTFEE_NOT_0_, _COORDINATE_Y_LINE_8_4_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}
		else
		{
			inDISP_Msg_BMP(_ERR_INSTFEE_NOT_0_, _COORDINATE_Y_LINE_8_4_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}
	}
	/* 晶片卡被取出 */
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_EMV_CARD_OUT_)
	{
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_CARD_REMOVED_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 2, "", 0);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_CARD_REMOVED_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 1, "", 0);
		}
		else
		{
			inDISP_Msg_BMP(_ERR_CARD_REMOVED_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 1, "", 0);
		}
	}
	/* 請改刷磁條 */
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_EMV_FALLBACK_)
	{
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_USE_MS_, _COORDINATE_Y_LINE_8_6_, _BEEP_3TIMES_MSG_, 2, "", 0);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_USE_MS_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 1, "", 0);
		}
		else
		{
			inDISP_Msg_BMP(_ERR_USE_MS_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 1, "", 0);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_RECV_)
	{
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP(_ERR_RECV_FAILED_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 2, "", 0);			/* 通訊失敗 */
		}
		else
		{
			inDISP_Msg_BMP(_ERR_RECV_FAILED_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		}
	}
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "未定義錯誤 : %d", pobTran->inErrorMsg);

		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_4_, _BEEP_1TIMES_MSG_, 2, "未定義錯誤", 0);
		}
		else
		{
			inDISP_Msg_BMP("", _COORDINATE_Y_LINE_8_4_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "未定義錯誤", 0);
		}
	}

	/* 為了強調Timeout時間 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	/* 顯示完就清空(為了只顯示一次) */
	pobTran->inErrorMsg = 0x00;

	return (VS_SUCCESS);
}



