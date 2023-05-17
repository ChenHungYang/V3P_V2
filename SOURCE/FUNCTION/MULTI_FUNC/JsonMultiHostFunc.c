
#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include <stdlib.h>
#include <emv_cl.h>
#include <sqlite3.h>

#include "../../INCLUDES/Define_1.h"
#include "../../INCLUDES/Define_2.h"
#include "../../INCLUDES/TransType.h"
#include "../../INCLUDES/Transaction.h"
#include "../../EVENT/Menu.h"
#include "../../PRINT/Print.h"
#include "../../DISPLAY/Display.h"
#include "../../EVENT/MenuMsg.h"
#include "../../EVENT/Flow.h"
#include "../../EVENT/Event.h"
#include "../APDU.h"
#include "../FILE_FUNC/File.h"
#include "../FILE_FUNC/FIleLogFunc.h"
#include "../Function.h"
#include "../FuncTable.h"
#include "../Sqlite.h"
#include "../CDT.h"
#include "../Card.h"
#include "../CFGT.h"
#include "../EDC.h"
#include "../HDT.h"
#include "../HDPT.h"
#include "../KMS.h"
#include "../SCDT.h"
#include "../VWT.h"
#include "../ECR_FUNC/ECR.h"
#include "../ECR_FUNC/RS232.h"
#include "../../COMM/WiFi.h"
#include "../MVT.h"
#include "../../../CTLS/CTLS.h"
#include "../Signpad.h"
#include "../FILE_FUNC/File.h"
#include "../FILE_FUNC/FIleLogFunc.h"

#ifdef __MUTI_FUCN_TEST_

#include "../../../JSON/cJSON.h"
#include "MultiFunc.h"
#include "MultiHostFunc.h"
#include "ExDevicePackData.h"
#include "ExDeviceUnPackData.h"

extern int	ginDebug;
extern int	ginDisplayDebug;
extern int	ginEngineerDebug;
extern int	ginMachineType;
extern MULTI_TABLE	gstMultiOb;

extern MULTI_TRANS_TABLE srMultiHostTransTb[];


/*
Function        : inMultiFunc_Host_JsonDataRecv
Date&Time   : 2022/11/17 下午 4:28
Describe        : 實際執行接收收銀機傳來的資料
*/
int inMultiFunc_Host_JsonDataRecv(unsigned char inHandle, int inRespTimeOut, 
				char *szGetMultiData, unsigned char uszSendAck, MULTI_TABLE* stMultiFuncOb)
{
	int i;
	int inRetVal;
	int inRetry = 0; /* 目前已重試次數 */
		
	unsigned short usReceiveBufferSize; /* uszReceiveBuffer的長度，*/
	unsigned short usOneSize = 1; /* 一次只讀一個byte */
	unsigned short usReceiveLen = 0;
	unsigned char uszLRC; /* LRC的值 */
	unsigned char uszReceiveBuffer[11024]; /* 包含STX 和 ETX的原始電文 */
	unsigned char uszSTX[2] = {0};
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	
	inDISP_LogPrintfWithFlag("  inMultiHost Recv TimeOut[%d] Line[%d]", stMultiFuncOb->srSetting.inTimeout, __LINE__);
	inFunc_CalculateRunTimeGlobal_Start();

	/* 設定Timeout */
	if(stMultiFuncOb->srSetting.inTimeout > 0){
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, stMultiFuncOb->srSetting.inTimeout);
	}else{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 20);
	}
	
	/* 設定最大可接收的長度  */
	usReceiveLen = 11020;
	/* 初始化放收到資料的陣列 */
	memset(uszReceiveBuffer, 0x00, sizeof (uszReceiveBuffer));
		
	usReceiveBufferSize = 0;
	
	/* 檢查 Comport有東西 */
	while (inMultiFunc_Host_Data_Receive_Check(stMultiFuncOb->srSetting.uszComPort, usOneSize) != VS_SUCCESS)
	{
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == 0)
		{
			inDISP_LogPrintfWithFlag("  inMultiHost Recv Check TimeOut *Error* Line[%d]", __LINE__);
			return (VS_TIMEOUT);
		}
	}
	
	/* 檢查資料開頭是否為 STX */
	while(1)
	{
		/* 這是Buffer的大小，若沒收到則該值會轉為0，所以需重置 */
		usOneSize = 1;
		if (inMultiFunc_Host_Data_Receive(stMultiFuncOb->srSetting.uszComPort, uszSTX[0], usOneSize) == VS_SUCCESS)
		{
			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == 0)
			{
				inDISP_LogPrintfWithFlag("  inMultiHost Recv DataTimeOut *Error* Line[%d]", __LINE__);
				return (VS_TIMEOUT);
			}

			if (uszSTX[0] == _STX_)
			{
				inDISP_LogPrintfWithFlag("  inMultiHost Recv Get STX Success");
				uszReceiveBuffer[usReceiveBufferSize ++] = _STX_;
				break;
			}
			else
			{
				if( usOneSize >= 1 )
					inDISP_LogPrintfWithFlag("  inMultiHost Recv Non STX Data [%02X]", uszSTX[0]);
				uszSTX[0] = 0x00;
			}
		}
		else
		{
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Receive STX ERR END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ESCAPE);
		}
	}
	
	/* 接收資料並判斷是否有收到 ETX */
	while (1)
	{
		/* 如果timeout就跳出去 */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			return (VS_TIMEOUT);
		}
	
		/* 修改可接收的最大資料長度 */
		usOneSize = _MULTI_RECV_SIZE_;
		inRetVal = inMultiFunc_Host_Data_Receive(stMultiFuncOb->srSetting.uszComPort, uszReceiveBuffer[usReceiveBufferSize], usOneSize);
		if (inRetVal == VS_SUCCESS)
		{
			inDISP_LogPrintfWithFlag("-->[%d] Aft RunDataRece usOneSize[%d]  ", __LINE__, usOneSize);

			/* buffer讀幾個byte，長度就減掉幾個Bytes */
			
			usReceiveBufferSize += usOneSize;

			inDISP_LogPrintfWithFlag("-->[%d] Count  usReceiveBufferSize[%d]  ", __LINE__, usReceiveBufferSize);

			/* 如果長度已收完，再去判斷 */
//			if (usReceiveLen <= 0)
//			{
				if (uszReceiveBuffer[(usReceiveBufferSize - 2)] == _ETX_)
				{
					inDISP_LogPrintfWithFlag("Receive ETX!");
					inFunc_WatchRunTime();
					break;
				} else
				{
					if(usReceiveLen > usReceiveBufferSize)
						continue;
					else
						inDISP_LogPrintfWithFlag("Not Receive ETX!");
				}
//			}
		} else 
		{
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Receive ETX ERR END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ESCAPE);
		}
	}
	
	/* 有收到STX、ETX而且收到長度也對，就開始算LRC */
	/* 計算收到DATA的LRC （Stx Not Include）*/
	for (i = 1; i <= (usReceiveBufferSize - 2); i++)
	{
		uszLRC ^= uszReceiveBuffer[i];
	}

	if (uszReceiveBuffer[usReceiveBufferSize - 1] == uszLRC)
	{
		/* 比對收到的LRC是否正確，若正確回傳ACK */
		inMultiFunc_Host_Send_ACKorNAK(stMultiFuncOb, _ACK_);
		inFunc_WatchRunTime();
	} else
	{
		/* 比對失敗 */
		inDISP_LogPrintfWithFlag("LRC error! Retry: %d", inRetry);
		inDISP_LogPrintfWithFlag(" CountLRC[%X]", uszLRC);
		inDISP_LogPrintfWithFlag(" CountLRC[%X]", uszReceiveBuffer[usReceiveBufferSize - 1]);

		/* 若錯誤回傳NAK */
		inMultiFunc_Host_Send_ACKorNAK(stMultiFuncOb, _NAK_);
		/* retry次數+ 1 */
		inRetry++;

		/* 初始化資料 */
		memset(uszReceiveBuffer, 0x00, sizeof (uszReceiveBuffer));
		usReceiveBufferSize = 0;
		/* 修改為沒有重試次數，如果收到資料失敗就回錯誤 */
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Receive LRC ERR END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ESCAPE);
	}

	/* 去除STX、ETX、LRC，把資料放到szrRealReceBuffer */
	memcpy(szGetMultiData, &uszReceiveBuffer[1], (usReceiveBufferSize-3));

	
	/* 列印紙本電文和顯示電文訊息 */
	inECR_Print_Receive_ISODeBug(szGetMultiData, usReceiveBufferSize - 3, usReceiveBufferSize);

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}



/*
 * Function		: inMultiFunc_Host_JsonRecePacket
 * Date&Time	: 2022/11/17 下午 4:26
 * Describe		: 
 * 處理外接設備透過 Rs232 傳送回來的資料,
 * 如果是沒有立既要接收的資料都會另外處理
 */
int inMultiFunc_Host_JsonRecePacket(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *stMultiOb)
{
	int inRetVal;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);	
	
	memset(stMultiOb->stMulti_TransData.szReceData, 0x00, sizeof (stMultiOb->stMulti_TransData.szReceData));

	/* -----------------------開始接收資料------------------------------------------ */
	/* szReceData  讀出來的資料會少掉 STX ，所以算欄位時需減少一個 BYTE*/
	inRetVal = inMultiFunc_Host_JsonDataRecv(stMultiOb->srSetting.uszComPort,
					  2,
					  stMultiOb->stMulti_TransData.szReceData,
					  VS_FALSE,
					  stMultiOb);

	if (inRetVal != VS_SUCCESS)
	{
		inDISP_LogPrintfWithFlag("第一階段分析不OK");
		return (inRetVal);
	}
	else
	{
		inDISP_LogPrintfWithFlag("第一階段分析OK");
	}

	if (!memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_SIGNPAD_, 2) ||
		!memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_RESIGN_TRANS_, 2)	)
	{
		inRetVal = inJSON_UnPackReceiveData(pobTran, stMultiOb);
	}
	else if (!memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_SIGN_CONFIRM_, 2))
	{
		/* 將【COM PORT】清空 */
		inMultiFunc_Host_FlushRxBuffer(stMultiOb->srSetting.uszComPort);
		inRetVal = VS_SUCCESS;
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
	return inRetVal;
}

/*
 * Function		: inMultiFunc_Host_JsonPackData
 * Date&Time	: 2022/11/17 下午 3:23
 * Describe		: 
 * 使用 Json方式進行資料組合，可利用 MULTI_TABLE 裏的 szTransType 進行判斷並組合相對應的資料 
 */
int inMultiFunc_Host_JsonPackData(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *srMultiHost )
{
	char 	szTempTransType[2 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	memset(szTempTransType, 0x00, sizeof(szTempTransType));
	memcpy(szTempTransType, srMultiHost->stMulti_TransData.szTransType, 2);
	
	if (!memcmp(szTempTransType, _MULTI_SALE_, 2))
	{
		srMultiHost->stMulti_TransData.lnTotalPacketSize = inJSON_PackSaleData(pobTran, srMultiHost->stMulti_TransData.szSendData);
		
		if(srMultiHost->stMulti_TransData.lnTotalPacketSize == 0)
		{
			inDISP_LogPrintfWithFlag(" Trans[%s] Host Pack *Error* Line[%d]",szTempTransType, __LINE__);
		}
	}else if (!memcmp(szTempTransType, _MULTI_SIGNPAD_, 2))
	{
		srMultiHost->stMulti_TransData.lnTotalPacketSize = inJSON_PackSignPadData(pobTran, srMultiHost->stMulti_TransData.szSendData);
		
		if(srMultiHost->stMulti_TransData.lnTotalPacketSize == 0)
		{
			inDISP_LogPrintfWithFlag(" Trans[%s] Host Pack *Error* Line[%d]",szTempTransType, __LINE__);
		}
	}else if (!memcmp(szTempTransType, _MULTI_RESIGN_TRANS_, 2))
	{
		srMultiHost->stMulti_TransData.lnTotalPacketSize = inJSON_PackReSignData(pobTran, srMultiHost->stMulti_TransData.szSendData);
		
		if(srMultiHost->stMulti_TransData.lnTotalPacketSize == 0)
		{
			inDISP_LogPrintfWithFlag(" Trans[%s] Host Pack *Error* Line[%d]",szTempTransType, __LINE__);
		}
	}else if (!memcmp(szTempTransType, _MULTI_TRANS_STOP_, 2))
	{
		
		srMultiHost->stMulti_TransData.lnTotalPacketSize = inJSON_PackTransStopData(pobTran, srMultiHost->stMulti_TransData.szSendData);
		
		if(srMultiHost->stMulti_TransData.lnTotalPacketSize == 0)
		{
			inDISP_LogPrintfWithFlag(" Trans[%s] Host Pack *Error* Line[%d]",szTempTransType, __LINE__);
		}
	}else if (!memcmp(szTempTransType, _MULTI_READ_MIFARE_, 2))
	{
		srMultiHost->stMulti_TransData.lnTotalPacketSize = inJSON_PackReadMifaeCardData(pobTran, srMultiHost->stMulti_TransData.szSendData);
		
		if(srMultiHost->stMulti_TransData.lnTotalPacketSize == 0)
		{
			inDISP_LogPrintfWithFlag(" Trans[%s] Host Pack *Error* Line[%d]",szTempTransType, __LINE__);
		}
	}else if (!memcmp(szTempTransType, _MULTI_SCAN_TRANS_, 2))
	{
		srMultiHost->stMulti_TransData.lnTotalPacketSize = inJSON_PackScanData(pobTran, srMultiHost->stMulti_TransData.szSendData);
		
		if(srMultiHost->stMulti_TransData.lnTotalPacketSize == 0)
		{
			inDISP_LogPrintfWithFlag(" Trans[%s] Host Pack *Error* Line[%d]",szTempTransType, __LINE__);
		}
	}
		
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	
	return (srMultiHost->stMulti_TransData.lnTotalPacketSize);
}

/*
 * 此功能只用在讀卡接收步驟
 * 在這邊接收ECR傳送的資料，如 ECR buffer 有資料，會讀到有 STX 才會進行正式讀取資料
 * 而 Time Out 設定要重設 
 */
int inMultiFunc_Host_JasonWaitRecvDataForCallSlve(TRANSACTION_OBJECT *pobTran)
{
	int	i, inSize = 0;
    	
	unsigned short	usMaxRecLen = _MULTI_MAX_SIZES_;
	unsigned char	uszLRCData;
	unsigned char	uszSTX[2 + 1] = {0};
	unsigned char	uszRecBuf[_MULTI_MAX_SIZES_ + 4] = {0};
	unsigned short	usOneSize = 1;
		
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	inDISP_LogPrintfWithFlag(" Multi Jaon Receive Com[%u] Line[%d]",gstMultiOb.srSetting.uszComPort, __LINE__);
	
	/* 這邊需要先檢查有沒有資料，COM PORT 沒資料就要先回傳再等待 */
	if (inRS232_Data_Receive_Check(gstMultiOb.srSetting.uszComPort, &usOneSize) != VS_SUCCESS)
	{        
		inDISP_LogPrintfWithFlag(" Multi Host Check Rec Data Receive *warning* Line[%d]", __LINE__);
		return (VS_WAVE_NO_DATA);
	}
	
//	while (1)
//	{
		/* Comport有東西 */
//		while (inRS232_Data_Receive_Check(gstMultiOb.srSetting.uszComPort, &usOneSize) != VS_SUCCESS)
//		{
//			/* Timeout */
//			if (inTimerGet(_TIMER_NEXSYS_2_) == 0)
//			{
//				inDISP_LogPrintfWithFlag(" Json Multi Host Check Rec Data Timout *Error* Line[%d]", __LINE__);
//				return (VS_TIMEOUT);
//			}
//		}
		/* 這是Buffer的大小，若沒收到則該值會轉為0，所以需重置 */
		usOneSize = 1;
		if (inRS232_Data_Receive(gstMultiOb.srSetting.uszComPort, &uszSTX[0], &usOneSize) == VS_SUCCESS)
		{
			/* Timeout */
//			if (inTimerGet(_TIMER_NEXSYS_2_) == 0)
//			{
//				inDISP_LogPrintfWithFlag(" Json Multi Host Rec Data Timout *Error* Line[%d]", __LINE__);
//				return (VS_TIMEOUT);
//			}
			/* 檢核有無收到第一個STX */
			if (uszSTX[0] == _STX_)
			{
				/*
				* 如果要開始接收ECR時間，要檢查剩餘 Time out 時間
				*  再設定到 gstMultiOb.srSetting.inTimeout
				*/
				if(VS_TIMEOUT == inDISP_SetMultiReceiveTimeout())
				{					
					return (VS_TIMEOUT);
				}
				
				inDISP_LogPrintfWithFlag(" Json Multi Host Rec STX Start Reset TimeOut[%u] Line[%d]",gstMultiOb.srSetting.inTimeout, __LINE__);
				
				/* 如果有收到資料，才設定 TIME OUT 進行持續接收 2022/11/29 */
				inDISP_Timer_Start(_TIMER_NEXSYS_2_, gstMultiOb.srSetting.inTimeout);

				uszRecBuf[inSize ++] = _STX_;
				
				usMaxRecLen -- ;
				
				while (1)
				{
					/* 銀行別 + 回應碼 + 功能別 */
					//usOneSize = 100;
					usOneSize = usMaxRecLen;
					
					inDISP_LogPrintfWithFlag(" Json Multi Host Rec Read Len[%u] Line[%d]", usOneSize,  __LINE__);
					
					if (inRS232_Data_Receive(gstMultiOb.srSetting.uszComPort, &uszRecBuf[inSize], &usOneSize) == VS_SUCCESS)
					{
						/* 已接收的長度 */
						inSize += usOneSize;
						/* 可再進行接收長度的限制 */
						usMaxRecLen -= usOneSize;

						if (uszRecBuf[inSize - 2] == _ETX_)
						{
							/* 算LRC */
							for (i = 1; i < (inSize-1); i++)
							{
								uszLRCData ^= uszRecBuf[i];
							}
							break;
						}
						
					}
					
					if (inTimerGet(_TIMER_NEXSYS_2_) == 0)
					{
						inDISP_LogPrintfWithFlag(" Json Multi Host Rec Data Aft EXT Timout *Error* Line[%d]", __LINE__);
						return (VS_TIMEOUT);
					}

				}
				
				/* 驗證LRC */
				if (uszLRCData == uszRecBuf[inSize - 1])
				{
					inDISP_LogPrintfWithFlag(" Json Multi Host Rec LRC OK Line[%d]", __LINE__);
//					break;
				}
				else
				{
					inDISP_LogPrintfWithFlag(" Json Multi Host Rec LRC NOT OK Line[%d]", __LINE__);
					return (VS_FAILURE);
				}
				
    			}
			else
			{
				if( usOneSize >= 1 )
					inDISP_LogPrintfWithFlag(" Json Multi Host Rec Not Found STX Val[0x%x] Line[%d]", uszSTX[0], __LINE__ );
				return (VS_WAVE_NO_DATA);
			}
    		}
    		else
    		{		
			inDISP_LogPrintfWithFlag("-----Json [%s][%s][%d]  No Data *Error* END -----",__FILE__, __FUNCTION__, __LINE__);
    			return (VS_WAVE_NO_DATA);
    		}
//	}
	
	/*  資料不含 STX ETX LRC */
	memset(gstMultiOb.stMulti_TransData.szReceData, 0x00, sizeof (gstMultiOb.stMulti_TransData.szReceData));
	memcpy(gstMultiOb.stMulti_TransData.szReceData, (char*)&uszRecBuf[1], inSize - 3);

	inDISP_LogPrintfWithFlag(" Json Multi Host Rec RealDataLen[%d] Line[%d]", (inSize - 3), __LINE__);

	inMulti_HOST_PrintReceiveDeBug((char *)gstMultiOb.stMulti_TransData.szReceData, inSize -3, inSize);

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}



/*
 * Function		: inMultiFunc_Host_JsonSendPacketWaitACK
 * Date&Time	: 2022/11/25 上午 11:13
 * Describe		: 
 * 在此處進行傳送資料組合
 * 使用開啟的 Rs232 Port 進行資料的傳送
 * 並且要收到外接設備回應的 ACK 訊息後，通訊才視為成功
 */
int inMultiFunc_Host_JsonSendPacketWaitACK(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *srMultiHost)
{
	int 	inSendPacketSizes, inRetVal = VS_ERROR ;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag("  ComPort [%u]", (srMultiHost->srSetting.uszComPort +1));

	memset(srMultiHost->stMulti_TransData.szSendData, 0x20, sizeof(srMultiHost->stMulti_TransData.szSendData));
	
	/* 先預設初始值，如有需要改變再進行變動 */
	srMultiHost->stMulti_Optional_Setting.inACKTimeOut = 3;
	srMultiHost->stMulti_Optional_Setting.inMaxRetries = 1;
	srMultiHost->stMulti_Optional_Setting.uszPadStxEtx = VS_TRUE;
	srMultiHost->stMulti_Optional_Setting.uszWaitForAck = VS_TRUE;

	/* 組封包 */
	inSendPacketSizes = inMultiFunc_Host_JsonPackData(pobTran, srMultiHost);

	
	inDISP_LogPrintfWithFlag("  SnedPackAck PackLen [%d]", inSendPacketSizes);
	
	/* 送封包 */
	if (!memcmp(srMultiHost->stMulti_TransData.szTransType, _MULTI_SIGN_CONFIRM_, 2))
		srMultiHost->stMulti_Optional_Setting.inACKTimeOut = 1;
	
	inRetVal = inMultiFunc_Host_DataSend(srMultiHost, srMultiHost->stMulti_TransData.szSendData, inSendPacketSizes);

	inDISP_LogPrintfWithFlag("  MultiFUnc Send RetVal[%d]", inRetVal);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
	return (inRetVal);
}



/*
Function        : inMultiFunc_Host_JsonSendError
Date&Time   : 
Describe        : 傳送錯誤訊息ECR
 */
int inMultiFunc_Host_JsonSendError(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *stMultiOb)
{
	int	inRetVal = VS_SUCCESS;
	int	inTotalSize = 0;
	char	szPackData[_MULTI_MAX_SIZES_ + 1];
	char	szRespData[4] = {0};

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);	
	
	memset(szPackData, 0x20, sizeof(szPackData));

	inDISP_LogPrintfWithFlag("  Multi Host Send inErrorType[%d]", stMultiOb->stMulti_TransData.inErrorType);

	/* Response Code (4 Bytes) */
	if (stMultiOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_SUCCESS_)
		memcpy(szRespData, "00", 2);
	else if (stMultiOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_USER_TERMINATE_ERROR_)
		memcpy(szRespData, "02", 2);
	else if (stMultiOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_TIMEOUT_)
		memcpy(szRespData, "03", 2);
	else if (stMultiOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_CTLS_HOST_CANCEL_)
		memcpy(szRespData, "04", 2);
	else if (stMultiOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_AMOUNT_ERROR_)
		memcpy(szRespData, "06", 2);
	else if (stMultiOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_)
		memcpy(szRespData, "00", 2);
	else if (stMultiOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_CTLS_ERROR_)
		memcpy(szRespData, "00", 2);
	else
		memcpy(szRespData, "01", 2);

	inTotalSize = inJSON_PackTransStopData(szPackData, szRespData);
	
	/* 送封包 */
	/* 用完要清掉設定 */
	memset(&stMultiOb->stMulti_Optional_Setting, 0x00, sizeof(stMultiOb->stMulti_Optional_Setting));
	stMultiOb->stMulti_Optional_Setting.inACKTimeOut = 2;
	stMultiOb->stMulti_Optional_Setting.inMaxRetries = 0;
	stMultiOb->stMulti_Optional_Setting.uszPadStxEtx = VS_TRUE;
	stMultiOb->stMulti_Optional_Setting.uszWaitForAck = VS_FALSE;
	
	inRetVal = inMultiFunc_Host_Send(pobTran, stMultiOb, szPackData, inTotalSize);

	/* 用完要清掉設定 */
	memset(&stMultiOb->stMulti_Optional_Setting, 0x00, sizeof(stMultiOb->stMulti_Optional_Setting));
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d]  Send Fail END -----",__FILE__, __FUNCTION__, __LINE__);	
		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
	
	return (VS_SUCCESS);
}


/*  
 *	會利用在 inMultiFunc_HostData_Recv_CTLS 抓取到的 st_uszMultiHostCTLSData 來處理晶片資料
 */
int inMultiFunc_Host_JsonUnpackCallSlaveCtlsResult(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR;
	
	inRetVal = inJSON_UnPackReceiveData(pobTran, &gstMultiOb);
	
	return inRetVal;
}

/*
 * Function		: inMultiFunc_JsonCallSlave
 * Date&Time	: 2022/11/17 下午 2:08
 * Describe		: 
 * 啟動外接設備的函式，針對傳進來的 inTranCode ，進行參數的設定。
 * 要設定的資料:
 * szTransType: 交易類別
 * 
 */
int inMultiFunc_JsonCallSlave(TRANSACTION_OBJECT *pobTran, int inTranCode, MULTI_TABLE * srMultiTable)
{
	int inStructIndex = 0; /* 使用哪個結構，會在使用交易步驟中決定(X) */
        /* 20230328 Miyano add 目前A30只有開發RS232傳輸，若要加USB待改 */

	int inRetVal, inTimeout;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);	
	
	if(pobTran == NULL)
	{
		inDISP_LogPrintfWithFlag(" *Error* pobTran is NULL in [%s] Line[%d] ", __FUNCTION__, __LINE__);	
		return VS_ERROR;
	}

	switch(inTranCode)
	{
		case _MULTI_TRANS_STOP_NO_:
			memcpy(&srMultiTable->stMulti_TransData.szTransType[0], _MULTI_TRANS_STOP_, 2);
			srMultiTable->stMulti_TransData.lnTotalPacketSize = _MULTI_SUB_SIZE_NONE_;/* Sub Size 0 */
//			inStructIndex = 0;
			inTimeout = 100;
			break;
		case _MULTI_SALE_NO_:
			memcpy(&srMultiTable->stMulti_TransData.szTransType[0], _MULTI_SALE_, 2);
			srMultiTable->stMulti_TransData.lnTotalPacketSize = _MULTI_SUB_SIZE_NONE_;/* Sub Size 0 */
//			inStructIndex = 0;
			inTimeout = 100;
			break;
		case _MULTI_READ_MIFARE_NO_:
			memcpy(&srMultiTable->stMulti_TransData.szTransType[0], _MULTI_READ_MIFARE_, 2);
			srMultiTable->stMulti_TransData.lnTotalPacketSize = _MULTI_SUB_SIZE_NONE_;/* Sub Size 0 */
//			inStructIndex = 0;
			inTimeout = 100;
			break;
		case _MULTI_SIGNPAD_NO_ :
			memcpy(&srMultiTable->stMulti_TransData.szTransType[0], _MULTI_SIGNPAD_, 2);
			srMultiTable->stMulti_TransData.lnTotalPacketSize = _MULTI_SUB_SIZE_SMALL_;/* Sub Size 100 */
//			inStructIndex = 0;
			inTimeout = 200;
			break;
		case _MULTI_RESIGN_TRANS_NO_ :
			memcpy(&srMultiTable->stMulti_TransData.szTransType[0], _MULTI_RESIGN_TRANS_, 2);
			srMultiTable->stMulti_TransData.lnTotalPacketSize = _MULTI_SUB_SIZE_SMALL_;/* Sub Size 100 */
//			inStructIndex = 0;
			inTimeout = 200;
			break;
		case _MULTI_SLAVE_REBOOT_NO_ :
			memcpy(&srMultiTable->stMulti_TransData.szTransType[0], _MULTI_SLAVE_REBOOT_, 2);
			srMultiTable->stMulti_TransData.lnTotalPacketSize = _MULTI_SUB_SIZE_NONE_;/* Sub Size 0 */
//			inStructIndex = 0;
			inTimeout = 100;
			break;
		case _MULTI_SIGN_CONFIRM_NO_ :
			/* 優化整合型周邊設備交易時間  */
			memcpy(&srMultiTable->stMulti_TransData.szTransType[0], _MULTI_SIGN_CONFIRM_, 2);
			srMultiTable->stMulti_TransData.lnTotalPacketSize = _MULTI_SUB_SIZE_SMALL_;/* Sub Size 100 */
//			inStructIndex = 0;
			inTimeout = 0;
			break;
                case _MULTI_SCAN_TRANS_NO_ :
			/* 20230131 Miyano 掃碼交易  */
			memcpy(&srMultiTable->stMulti_TransData.szTransType[0], _MULTI_SCAN_TRANS_, 2);
			srMultiTable->stMulti_TransData.lnTotalPacketSize = _MULTI_SUB_SIZE_MAX_;/* Sub Size 100 */
//			inStructIndex = 0;
			inTimeout = 100;
			break;
		default :
			return (VS_ERROR);
	}
	inDISP_LogPrintfWithFlag(" Json Call Slave TotalSz[%ld] Type[%s] Line[%d]",
					srMultiTable->stMulti_TransData.lnTotalPacketSize, 
					srMultiTable->stMulti_TransData.szTransType, __LINE__);	
	
	
	srMultiTable->stMulti_Optional_Setting.uszWaitForAck = VS_FALSE;
	/* Host Send */
	inRetVal = srMultiHostTransTb[inStructIndex].inMultiSend(pobTran, srMultiTable);

	/* 有特定情況，傳送後一要等資料，所以有設 TimeOut 時間再停頓 */
	if (inTimeout > 0)
		inDISP_Wait(inTimeout);

	if (inRetVal == VS_SUCCESS)
	{
		/* 傳送成功就認為必需回傳資料 */
		srMultiTable->stMulti_TransData.unzAlreadySend = VS_TRUE;
	}
		
	switch(inTranCode)
	{
		case _MULTI_TRANS_STOP_NO_:
			srMultiTable->stMulti_TransData.inCTLS_Timeout = 30;
			break;
		case _MULTI_PIN_NO_ :
			srMultiTable->stMulti_TransData.inCTLS_Timeout = 35;
			break;
		case _MULTI_SIGN_CONFIRM_NO_ :
		
			srMultiTable->stMulti_TransData.inCTLS_Timeout = 35;
			break;
		case _MULTI_SIGNPAD_NO_ :
		case _MULTI_RESIGN_TRANS_NO_:
			srMultiTable->stMulti_TransData.inCTLS_Timeout = 95;
			break;
		case _MULTI_SALE_NO_:
		case _MULTI_READ_MIFARE_NO_:
                case _MULTI_SCAN_TRANS_NO_ :
			srMultiTable->stMulti_TransData.inCTLS_Timeout = 20;
			return (VS_SUCCESS);
		case _MULTI_SLAVE_REBOOT_NO_ :
			return (VS_SUCCESS);
		default :
			return (VS_ERROR);
	}

	/* Host Recv */
	/* 如果要立既等接收，就在這邊進行 2022/11/22 [SAM] */
	inRetVal = srMultiHostTransTb[inStructIndex].inMultiRece(pobTran, srMultiTable);
		
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
	return (inRetVal);
}

#endif
