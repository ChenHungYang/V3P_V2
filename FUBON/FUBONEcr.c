
#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/EVENT/Menu.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Batch.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/SCDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/FILE_FUNC/File.h"
#include "../SOURCE/FUNCTION/FILE_FUNC/FIleLogFunc.h"
#include "../SOURCE/FUNCTION/ECR_FUNC/ECR.h"
#include "../SOURCE/FUNCTION/ECR_FUNC/RS232.h"

#include "../SOURCE/FUNCTION/USB.h"
#include "../SOURCE/FUNCTION/BaseUSB.h"
#include "../SOURCE/FUNCTION/KMS.h"
#include "../SOURCE/FUNCTION/Accum.h"

#include "../SOURCE/COMM/Ethernet.h"
#include "../SOURCE/COMM/WiFi.h"

#include "../SOURCE/FUNCTION/ECR_FUNC/EcrPackFunc.h"
#include "../SOURCE/FUNCTION/CARD_FUNC/CardFunction.h"

#include "FUBONEcr.h"

extern	int		ginDebug;
extern	int		ginISODebug;
extern	int		ginDisplayDebug;
extern	int		ginECR_ResponseFd;
extern	int		ginMachineType;
extern	unsigned char	guszCTLSInitiOK;

//static unsigned char  guszBanCardBinArray[15000];
//static unsigned long gulBanCardLen = 0;



/*
Function        : inFUBON_RS232_ECR_8N1_Standard_Initial
Date&Time       : 2019/01/18
Describe        : 
*/
int inFUBON_RS232_ECR_8N1_Standard_Initial(ECR_TABLE *srECROb)
{
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CUP_LOGON_;
	char		szKey = 0x00;
	char		szDebugMsg[100 + 1];
	char		szECRComPort[4 + 1];
	unsigned char	uszParity;
	unsigned char	uszDataBits;
	unsigned char	uszStopBits;
	unsigned long	ulBaudRate;
	unsigned short	usRetVal;
	
	memset(&uszParity, 0x00, sizeof(uszParity));
	memset(&uszDataBits, 0x00, sizeof(uszDataBits));
	memset(&uszStopBits, 0x00, sizeof(uszStopBits));
	memset(&ulBaudRate, 0x00, sizeof(ulBaudRate));
	
	/* 從EDC.Dat抓出哪一個Comport */
	/* inGetECRComPort */
	memset(&szECRComPort, 0x00, sizeof(szECRComPort));
	inGetECRComPort(szECRComPort);
	/* Verifone用handle紀錄，Castle用Port紀錄 */
	if (!memcmp(szECRComPort, "COM1", 4))
		srECROb->srSetting.uszComPort = d_COM1;
	else if (!memcmp(szECRComPort, "COM2", 4))
		srECROb->srSetting.uszComPort = d_COM2;
	if (!memcmp(szECRComPort, "COM3", 4))
		srECROb->srSetting.uszComPort = d_COM3;
	if (!memcmp(szECRComPort, "COM4", 4))
		srECROb->srSetting.uszComPort = d_COM4;
		
	/* BaudRate = 9600 */
	ulBaudRate = 9600;
	
	/* Parity */
	uszParity = 'N';
	
	/* Data Bits */
	uszDataBits = 8;
	
	/* Stop Bits */
	uszStopBits = 1;
	
	
        /* 開port */
	/* Portable 機型若沒接上底座再開Ethernet會失敗 */
	if (inFunc_Is_Portable_Type() == VS_TRUE)
	{
		do
		{
			usRetVal = inRS232_Open(srECROb->srSetting.uszComPort, ulBaudRate, uszParity, uszDataBits, uszStopBits);
        
			if (usRetVal == VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf("inRS232_Open OK");
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort, ulBaudRate, uszDataBits, uszParity, uszStopBits);
					inDISP_LogPrintf(szDebugMsg);
				}
				break;
				
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "inRS232_Open Error: 0x%04x", usRetVal);
					inDISP_LogPrintf(szDebugMsg);
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort + 1, ulBaudRate, uszDataBits, uszParity, uszStopBits);
					inDISP_LogPrintf(szDebugMsg);
				}
				
				/* 未接上底座，提示接上底座後並按確認 */
				if (inFunc_Is_Cradle_Attached() != VS_SUCCESS)
				{
					/* 請插回底座 繼續交易 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_CHECK_CRADLE_ATTATCHED_, 0, _COORDINATE_Y_LINE_8_4_);
					inDISP_BEEP(1, 0);
					
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
					
					while (1)
					{
						inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
						szKey = uszKBD_Key();

						/* Timeout */
						if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
						{
							szKey = _KEY_TIMEOUT_;
							inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
						}

						if (szKey == _KEY_ENTER_		||
						    szKey == _KEY_TIMEOUT_		||
						    inChoice == _CUPLogOn_Touch_KEY_2_)
						{
							inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
							break;
						}
						else if (szKey == _KEY_0_)
						{
							/* 壓住0後3秒內按clear */
							inDISP_Timer_Start(_TIMER_NEXSYS_4_, 3);
							do
							{
								szKey = uszKBD_Key_In();
							}while (szKey == 0	&&
								inTimerGet(_TIMER_NEXSYS_4_) != VS_SUCCESS);
							
							/* 按clear */
							if (szKey == _KEY_CLEAR_)
							{
								return (VS_SUCCESS);
							}
						}
						else
						{

						}
						
					}/* 重新初始化迴圈 */
					/* 清空Touch資料 */
					inDisTouch_Flush_TouchFile();
					
				}
				/* 若接上底座還是錯誤，就回傳錯誤 */
				else
				{
					return (VS_ERROR);
				}
				
			}
			
		}
		while (1);
				
	}
	/* CounterTop 機型 */
	else
	{
		usRetVal = inRS232_Open(srECROb->srSetting.uszComPort, ulBaudRate, uszParity, uszDataBits, uszStopBits);
        
		if (usRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "inRS232_Open Error: 0x%04x", usRetVal);
				inDISP_LogPrintf(szDebugMsg);
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort + 1, ulBaudRate, uszDataBits, uszParity, uszStopBits);
				inDISP_LogPrintf(szDebugMsg);
			}
			return (VS_ERROR);         
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("inRS232_Open OK");
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort, ulBaudRate, uszDataBits, uszParity, uszStopBits);
				inDISP_LogPrintf(szDebugMsg);
			}
		}
		
	}
        
	/* 清空接收的buffer */
        inRS232_FlushRxBuffer(srECROb->srSetting.uszComPort);
	
        return (VS_SUCCESS);
}

/*
Function        : inFUBON_RS232_ECR_8N1_Standard_Receive_Packet
Date&Time       : 2019/01/18
Describe        :處理收銀機傳來的資料
*/
int inFUBON_RS232_ECR_8N1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_8N1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inFUBON_ECR_8N1_Standard_Unpack(pobTran, srECROb, szDataBuffer);
	
	if (inRetVal != VS_SUCCESS)
	{
		inRS232_ECR_SendError(pobTran, inRetVal);
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        : inFUBON_RS232_ECR_8N1_Standard_Send_Packet
Date&Time       : 2019/01/18
Describe        :處理要送給收銀機的資料
*/
int inFUBON_RS232_ECR_8N1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponce == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inFUBON_ECR_8N1_Standard_Pack(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_8N1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponce = VS_TRUE;
	
	return (VS_SUCCESS);
}

/*
Function        : inFUBON_RS232_ECR_8N1_Standard_Send_Error
Date&Time       : 2019/01/18
Describe        :傳送錯誤訊息ECR
*/
int inFUBON_RS232_ECR_8N1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponce == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inFUBON_ECR_8N1_Standard_Pack(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inFUBON_ECR_8N1_Standard_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_8N1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponce = VS_TRUE;
	
        return (VS_SUCCESS);
}

/*
Function        : inFUBON_RS232_ECR_8N1_Standard_Close
Date&Time       :  2019/01/18
Describe        :關閉Comport
*/
int inFUBON_RS232_ECR_8N1_Standard_Close(ECR_TABLE* srECRob)
{
        /*關閉port*/
        if (inRS232_Close(srECRob->srSetting.uszComPort) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}




/*
Function        : inFUBON_ECR_8N1_Standard_Unpack
Date&Time       :2017/11/16 上午 10:45
Describe        :分析收銀機傳來的資料
*/
int inFUBON_ECR_8N1_Standard_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer)
{
	int     inRetVal = VS_SUCCESS;
	int     inTransType = 0;		/* 電文中的交易別字串轉為數字儲存 */
	int     inCountUnpackLen = 0;
	char	szTemplate[100 + 1] = {0};
	char	szCTLSEnable[2 + 1] = {0};
	
        	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 收到資料就嗶一下。  */
	CTOS_Beep();
	/* ECR Indicator */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &szDataBuffer[inCountUnpackLen], 1);
	inCountUnpackLen ++;
	inDISP_LogPrintfWithFlag(" ECR Indicator : [%s]  ", szTemplate);

	/* 欄位為M 卻不存在，回傳錯誤 */
	if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
	{
		inDISP_LogPrintfWithFlag(" ECR Indicator Not Exist Error ");
		return (VS_ERROR);
	}

	/* ECR Indicator必為'I'，否則error */
	if (memcmp(&szTemplate[0], "I", 1) != 0)
	{
		inDISP_LogPrintfWithFlag(" ECR Indicator Not [I] = [%x] ", szTemplate[0]);
		return (VS_ERROR);
	}
	
	/* ECR Version Date [6 Bytes] */
	inCountUnpackLen += 6;
	
	/* Unattend Flag [1 Bytes]  */
	/* 記錄是否為自助交易 2020/2/4 下午 5:20 [SAM] */
	if(szDataBuffer[inCountUnpackLen] == 'Y')
	{
		pobTran->uszKioskFlag = 'Y';
		inFunc_SetKisokFlag("Y");
	}else{
		pobTran->uszKioskFlag = 'N';
		inFunc_SetKisokFlag("N");
	}
	pobTran->uszEcrTransFlag = 'Y';
	inCountUnpackLen += 1;
	
	
	inDISP_LogPrintfWithFlag(" Recive  Kiosk[%u] Line[%d]", pobTran->uszKioskFlag, __LINE__);
	
	/* Trans Type (交易別) [2 Bytes ]*/
	memset(srECROb->srTransData.szTransType, 0x00, sizeof(srECROb->srTransData.szTransType));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &szDataBuffer[inCountUnpackLen], 2);
	inCountUnpackLen += 2;

	inTransType = atoi(szTemplate);
	memcpy(srECROb->srTransData.szTransType, szTemplate, 2);

	inDISP_LogPrintfWithFlag(" Trans Type : %s  ", szTemplate);
	
	/* Card Type Indicator [1 Bytes] */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szDataBuffer[inCountUnpackLen], 1);
	switch( inTransType)
	{
		case _ECR_8N1_SALE_NO_:
		case _ECR_8N1_REFUND_NO_:
		case _ECR_8N1_VOID_NO_:	
			/* 如果是'C'代表是銀聯卡交易 */
			memcpy(&srECROb->srTransData.szCUPIndicator[0], &szTemplate[0], 1);
			
			if (memcmp(&srECROb->srTransData.szCUPIndicator[0], "C", 1) == 0)
			{
				pobTran->srBRec.uszCUPTransBit = VS_TRUE;
			}

			inDISP_LogPrintfWithFlag(" CUPTransBit : [%d]", pobTran->srBRec.uszCUPTransBit);
			break;
		default:
			break;
	}
	
	
	inCountUnpackLen += 1;
	
	/* Host ID  [2 Bytes] */
	inCountUnpackLen += 2;
	
	/* Receipt No  [6 Bytes] */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szDataBuffer[13], 6);
	switch (inTransType)
	{
		case _ECR_8N1_VOID_NO_:				/* 取消交易 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &szDataBuffer[13], 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				inDISP_LogPrintfWithFlag(" 交易序號 未送 ");
				return (VS_ERROR);
			}
			else
			{
				pobTran->srBRec.lnOrgInvNum = atol(szTemplate);
			}
			
			inDISP_LogPrintfWithFlag(" 交易序號 : [%ld]", pobTran->srBRec.lnOrgInvNum);
			
			break;
		default:
			break;
	}
	
	inCountUnpackLen += 6;
	
	/* Card Number [19 Bytes] */
	inCountUnpackLen += 19;
	
	/* Expiration Date [4 Bytes] */
	inCountUnpackLen += 4;
	
	/* Trans Amount [12 Bytes] */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szDataBuffer[inCountUnpackLen], 10);
	switch( inTransType)
	{
		case _ECR_8N1_SALE_NO_:
		case _ECR_8N1_REFUND_NO_:
		case _ECR_8N1_OFFLINE_NO_:
		case _ECR_8N1_INSTALLMENT_NO_:
		case _ECR_8N1_REDEEM_NO_:
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:
		case _ECR_8N1_REDEEM_REFUND_NO_:
			//交易金額
			pobTran->srBRec.lnTxnAmount = atol(szTemplate);
			pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
			pobTran->srBRec.lnTotalTxnAmount = atol(szTemplate);

			inDISP_LogPrintfWithFlag(" 交易金額 : [%ld]", pobTran->srBRec.lnTxnAmount);
			
			/* 自助機交易不能超過 3000 2020/1/302020/1/30 下午 4:56 [SAM] */
			if(pobTran->uszKioskFlag == 'Y' && inTransType == _ECR_8N1_SALE_NO_  )
			{
				if(pobTran->srBRec.lnTxnAmount > 3000)
				{
					srECROb->srTransData.inErrorType = _ECR_RESPONSE_CODE_EXCESSIVE_TRANS_AMT_;
					return (VS_ERROR);
				}
					
			}
			break;
		default:
			break;
	}

	inCountUnpackLen += 12;
	
	/* Trans Date  [6 Bytes] */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szDataBuffer[inCountUnpackLen], 6);
	switch( inTransType)
	{
		case _ECR_8N1_REFUND_NO_:
			/* 目前ECR規格不會帶此資料，所以改成都用手輸，拿掉此判斷  2022/12/14 [SAM]*/
//			if(pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				if(szTemplate[0] != 0x20)
//				{
//					memset(pobTran->srBRec.szCUP_TD, 0x00, sizeof(pobTran->srBRec.szCUP_TD));
//					memcpy(pobTran->srBRec.szCUP_TD, &szTemplate[2], 4);
//
//					if (inFunc_CheckValidOriDate(pobTran->srBRec.szCUP_TD) != VS_SUCCESS)
//					{
//						inDISP_LogPrintfWithFlag("Date Not Valid: %s", pobTran->srBRec.szCUP_TD);
//						pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
//						return (VS_ERROR);
//					}
//				}
//			}
			
			inDISP_LogPrintfWithFlag("  Receive Time : [%s]", szTemplate);
			inDISP_LogPrintfWithFlag("  BRec Time    : [%s]", pobTran->srBRec.szCUP_TD);
			break;
		default:
			break;
	}
	inCountUnpackLen += 6;		
	
	/* Trans Time [6 Bytes] */
	inCountUnpackLen += 6;
	
	/* Approval Code [9 Bytes] */
	memcpy(&szTemplate[0], &szDataBuffer[inCountUnpackLen], 9);
	switch( inTransType)
	{
		case _ECR_8N1_OFFLINE_NO_:
		case _ECR_8N1_REFUND_NO_:
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:
		case _ECR_8N1_REDEEM_REFUND_NO_:	
			
			if(pobTran->srBRec.szAuthCode[0] != 0x20 && pobTran->srBRec.szAuthCode[1] != 0x20)
			{
				memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
				memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			}
			
		
			inDISP_LogPrintfWithFlag("  Receive Approval Code : [%s]", szTemplate);
			inDISP_LogPrintfWithFlag("  BRec Approval Code    : [%s]", pobTran->srBRec.szAuthCode);
			break;
		default:
			break;
	}
	inCountUnpackLen += 9;	
	
	/* Contactless Indicator [1 Bytes] */
	inCountUnpackLen += 1;
	
	/* ECR Response Code [4 Bytes] */
	inCountUnpackLen += 4;

	/* Merchant ID [15 Bytes] */
	inCountUnpackLen += 15;
	
	/* Terminal ID [8 Bytes] */
	inCountUnpackLen += 8;
	
	/* Filler [12 Bytes] */
	inCountUnpackLen += 12;
	
	/* Store ID [18 Bytes] */
	memcpy(&szTemplate[0], &szDataBuffer[inCountUnpackLen], 18);
	switch( inTransType)
	{
		case _ECR_8N1_SALE_NO_:
		case _ECR_8N1_REFUND_NO_:
		case _ECR_8N1_OFFLINE_NO_:
		case _ECR_8N1_INSTALLMENT_NO_:
		case _ECR_8N1_REDEEM_NO_:
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:
		case _ECR_8N1_REDEEM_REFUND_NO_:

			memcpy(pobTran->srBRec.szStoreID, &szTemplate[0], 18);

			inDISP_LogPrintfWithFlag(" Store ID : [%s]", pobTran->srBRec.szStoreID);
			break;
		default:
			break;
	}
	inCountUnpackLen += 18;
	
	/* Redeem & Installment  Indicator [1 Bytes] */
	inCountUnpackLen += 1;
	
	/* Redeem Paid Amount [12 Bytes] */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szDataBuffer[inCountUnpackLen], 10);
	switch( inTransType)
	{
		case _ECR_8N1_REDEEM_REFUND_NO_:
			if(atol(szTemplate) > 0)
				pobTran->srBRec.lnRedemptionPaidCreditAmount = atol(szTemplate);
			inDISP_LogPrintfWithFlag(" 實際支付金額 : [%ld]", pobTran->srBRec.lnRedemptionPaidCreditAmount);
			break;
		default:
			break;
	}
	inCountUnpackLen += 12;
	
	/* Redeem Point [8 Bytes] */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szDataBuffer[inCountUnpackLen], 8);
	switch( inTransType)
	{
		case _ECR_8N1_REDEEM_REFUND_NO_:
			if(atol(szTemplate) > 0)
				pobTran->srBRec.lnRedemptionPoints = atol(szTemplate);
			inDISP_LogPrintfWithFlag(" 紅利點數 : [%ld]", pobTran->srBRec.lnRedemptionPoints);
			break;
		default:
			break;
	}
	inCountUnpackLen += 8;
	
	/* Points of Balance [8 Bytes] */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szDataBuffer[inCountUnpackLen], 8);
	switch( inTransType)
	{
		case _ECR_8N1_REDEEM_REFUND_NO_:
			if(atol(szTemplate) > 0)
				pobTran->srBRec.lnRedemptionPointsBalance = atol(szTemplate);
			inDISP_LogPrintfWithFlag(" 剩餘點數 : [%ld]", pobTran->srBRec.lnRedemptionPointsBalance);
			break;
		default:
			break;
	}
	inCountUnpackLen += 8;
			
	/* Redeem Amount [12 Bytes] */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szDataBuffer[inCountUnpackLen], 10);
	switch( inTransType)
	{
		case _ECR_8N1_REDEEM_REFUND_NO_:
			if(atol(szTemplate) > 0)
				pobTran->srBRec.lnRedemptionPaidAmount = atol(szTemplate);
			inDISP_LogPrintfWithFlag(" 折抵金額 : [%ld]", pobTran->srBRec.lnRedemptionPaidAmount);
			break;
		default:
			break;
	}
	inCountUnpackLen += 12;
			
	/* Installment Period [2 Bytes] */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szDataBuffer[inCountUnpackLen], 2);
	switch( inTransType)
	{
		case _ECR_8N1_INSTALLMENT_NO_:
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:
			if(atol(szTemplate) > 0)
				pobTran->srBRec.lnInstallmentPeriod = atol(szTemplate);
			
			inDISP_LogPrintfWithFlag(" 分期期數 : [%ld]", pobTran->srBRec.lnInstallmentPeriod);
			break;
		default:
			break;
	}
	
	inCountUnpackLen += 2;
			
	/* Down Payment Amount [12 Bytes] */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szDataBuffer[inCountUnpackLen], 10);
	switch( inTransType)
	{
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:
			if(atol(szTemplate) > 0)
				pobTran->srBRec.lnInstallmentDownPayment = atol(szTemplate);
			inDISP_LogPrintfWithFlag(" 首期金額 : [%ld]", pobTran->srBRec.lnInstallmentDownPayment);
			break;
		default:
			break;
	}
	inCountUnpackLen += 12;
			
	/* nstallment Payment Amount [12 Bytes] */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szDataBuffer[inCountUnpackLen], 10);
	switch( inTransType)
	{
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:
			if(atol(szTemplate) > 0)
				pobTran->srBRec.lnInstallmentPayment = atol(szTemplate);
			inDISP_LogPrintfWithFlag(" 每期金額 : [%ld]", pobTran->srBRec.lnInstallmentPayment);
			break;
		default:
			break;
	}
	inCountUnpackLen += 12;
			
	/* Formality Fee [12 Bytes] */	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szDataBuffer[inCountUnpackLen], 10);
	switch( inTransType)
	{
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:
			
			if(atol(szTemplate) > 0)
				pobTran->srBRec.lnInstallmentFormalityFee = atol(szTemplate);
			inDISP_LogPrintfWithFlag(" 手續費 : [%ld]", pobTran->srBRec.lnInstallmentFormalityFee);
			break;
		default:
			break;
	}
	inCountUnpackLen += 12;
	
	/* Card Type [2 Bytes] */
	inCountUnpackLen += 2;
	
	/* Batch No [6 Bytes] */
	inCountUnpackLen += 6;
	
	/* Encrypted Card Number [50 Bytes] */
	inCountUnpackLen += 50;
	
	/* Filler [130 Bytes] */
	inCountUnpackLen += 130;
			
	inDISP_LogPrintfWithFlag(" ECR Unpack Total  [%d]", inCountUnpackLen);
	
	/* 跑OPT */
	switch (inTransType)
	{
		case _ECR_8N1_REFUND_NO_:
			
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_CUP_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯退貨> */

				pobTran->inFunctionID = _CUP_REFUND_;
				
				memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				
				if (!memcmp(&szCTLSEnable[0], "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_CUP_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_CUP_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_;

				}


				pobTran->inTransactionCode = _CUP_REFUND_;
				pobTran->srBRec.inCode = _CUP_REFUND_;
				pobTran->srBRec.inOrgCode = _CUP_REFUND_;
				
				inDISP_LogPrintfWithFlag("--- START ECR Sale  CUP ----");
			}else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				pobTran->inFunctionID = _REFUND_;

				memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);                       
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_;
				}

				pobTran->inRunTRTID = _TRT_REFUND_;

				pobTran->inTransactionCode = _REFUND_;
				pobTran->srBRec.inCode = _REFUND_;
				pobTran->srBRec.inOrgCode = _REFUND_;
			}

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜交易補登＞ */
			
			pobTran->inFunctionID = _SALE_OFFLINE_;
			pobTran->inRunOperationID = _OPERATION_SALE_OFFLINE_;
			pobTran->inRunTRTID = _TRT_SALE_OFFLINE_;

			pobTran->inTransactionCode = _SALE_OFFLINE_;
			pobTran->srBRec.inCode = _SALE_OFFLINE_;
			pobTran->srBRec.inOrgCode = _SALE_OFFLINE_;
			
			inDISP_LogPrintfWithFlag("--- START ECR Offline ----");
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			
			inDISP_LogPrintfWithFlag("--- END ECR Offline [%d]----", inRetVal);
			break;
		
		case _ECR_8N1_REDEEM_REFUND_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期退貨＞ */

			pobTran->inFunctionID = _REDEEM_REFUND_;

			pobTran->inRunOperationID = _OPERATION_REFUND_;
			pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;

			pobTran->srBRec.uszRedeemBit = VS_TRUE;
			
			pobTran->inTransactionCode = _REDEEM_REFUND_;
			pobTran->srBRec.inCode = _REDEEM_REFUND_;
			pobTran->srBRec.inOrgCode = _REDEEM_REFUND_;
			
			inDISP_LogPrintfWithFlag("--- START ECR Redeem Refund ----");

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);

			inDISP_LogPrintfWithFlag("--- END ECR Redeem Refund [%d]----", inRetVal);
			
			break;
		
		case _ECR_8N1_REDEEM_NO_:
			
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期付款＞ */

			pobTran->inFunctionID = _REDEEM_SALE_;

			pobTran->inRunOperationID = _OPERATION_I_R_;
			pobTran->inRunTRTID = _TRT_REDEEM_SALE_;
			pobTran->srBRec.uszRedeemBit = VS_TRUE;
			
			pobTran->inTransactionCode = _REDEEM_SALE_;
			pobTran->srBRec.inCode = _REDEEM_SALE_;
			pobTran->srBRec.inOrgCode = _REDEEM_SALE_;
			
			inDISP_LogPrintfWithFlag("--- START ECR Redeem Sale ----");

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);

			inDISP_LogPrintfWithFlag("--- END ECR Redeem Sale [%d]----", inRetVal);
			
			break;
		
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期退貨＞ */

			pobTran->inFunctionID = _INST_REFUND_;

			pobTran->inRunOperationID = _OPERATION_REFUND_;
			pobTran->inRunTRTID = _TRT_INST_REFUND_;

			pobTran->srBRec.uszInstallmentBit = VS_TRUE;
			
			pobTran->inTransactionCode = _INST_REFUND_;
			pobTran->srBRec.inCode = _INST_REFUND_;
			pobTran->srBRec.inOrgCode = _INST_REFUND_;
			
			inDISP_LogPrintfWithFlag("--- START ECR Inst Refund ----");

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);

			inDISP_LogPrintfWithFlag("--- END ECR Inst Refund [%d]----", inRetVal);
			
			break;
		
		case _ECR_8N1_INSTALLMENT_NO_:
			
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期付款＞ */

			pobTran->inFunctionID = _INST_SALE_;

			pobTran->inRunOperationID = _OPERATION_I_R_;
			pobTran->inRunTRTID = _TRT_INST_SALE_;
			pobTran->srBRec.uszInstallmentBit = VS_TRUE;
			
			pobTran->inTransactionCode = _INST_SALE_;
			pobTran->srBRec.inCode = _INST_SALE_;
			pobTran->srBRec.inOrgCode = _INST_SALE_;
			
			inDISP_LogPrintfWithFlag("--- START ECR Inst Sale ----");

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);

			inDISP_LogPrintfWithFlag("--- END ECR Inst Sale [%d]----", inRetVal);
			
			break;
		case _ECR_8N1_SALE_NO_:
			
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯一般交易＞ */

				pobTran->inFunctionID = _CUP_SALE_;
				pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
				pobTran->inRunTRTID = _TRT_CUP_SALE_;

				pobTran->inTransactionCode = _CUP_SALE_;
				pobTran->srBRec.inCode = _CUP_SALE_;
				pobTran->srBRec.inOrgCode = _CUP_SALE_;
				
				inDISP_LogPrintfWithFlag("--- START ECR Sale  CUP ----");
			}else
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜一般交易＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */

				pobTran->inFunctionID = _SALE_;

				memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);                       
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_SALE_;
				}

				pobTran->inTransactionCode = _SALE_;
				pobTran->srBRec.inCode = _SALE_;
				pobTran->srBRec.inOrgCode = _SALE_;
				
				inDISP_LogPrintfWithFlag("--- START ECR Sale ----");
			}
			

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			
			inECR_PACK_GetTotalAmountRecord(pobTran);
			
			inDISP_LogPrintfWithFlag("--- END ECR Sale [%d]----", inRetVal);
			break;
	
		case _ECR_8N1_VOID_NO_:
			
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_CUP_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */

				pobTran->inFunctionID = _CUP_VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_CUP_VOID_;

				pobTran->inTransactionCode = _CUP_VOID_;
				pobTran->srBRec.inCode = _CUP_VOID_;
				pobTran->srBRec.inOrgCode = _CUP_VOID_;
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */

				pobTran->inFunctionID = _VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_VOID_;

				pobTran->inTransactionCode = _VOID_;
				pobTran->srBRec.inCode = _VOID_;
				pobTran->srBRec.inOrgCode = _VOID_;

			}
			
			inDISP_LogPrintfWithFlag("--- START ECR VOID ----");
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			
			inDISP_LogPrintfWithFlag("--- END ECR VOID [%d]----", inRetVal);
			break;
               
		case _ECR_8N1_SETTLEMENT_NO_:
			inDISP_ClearAll();
			/* 第三層顯示 ＜結帳交易＞ */
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜結帳交易＞ */

			pobTran->inFunctionID = _SETTLE_;
			pobTran->inRunOperationID = _OPERATION_SETTLE_;
			pobTran->inRunTRTID = _TRT_SETTLE_;

			pobTran->inTransactionCode = _SETTLE_;
			pobTran->srBRec.inCode = _SETTLE_;
			pobTran->srBRec.inOrgCode = _SETTLE_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <交易查詢> */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_REVIEW_DETAIL_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			/* 因為沒有要處理後續，所以接收完成就回應成功 */
			inECR_Send_Transaction_Result(pobTran);
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜總額查詢＞ */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_REVIEW_TOTAL_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			/* 因為沒有要處理後續，所以接收完成就回應成功 */
			inECR_Send_Transaction_Result(pobTran);
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_MENU_REPORT_DETAIL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_DETAIL_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 明細列印 */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_DETAIL_REPORT_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			/* 因為沒有要處理後續，所以接收完成就回應成功 */
			inECR_Send_Transaction_Result(pobTran);
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_MENU_REPORT_TOTAL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TOTAIL_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 總額列印 */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_TOTAL_REPORT_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			/* 因為沒有要處理後續，所以接收完成就回應成功 */
			inECR_Send_Transaction_Result(pobTran);
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			
			
			break;
		default:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			inRetVal = VS_ERROR;
			break;
        }

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
        return (inRetVal);
}


/*
Function        : inFUBON_ECR_8N1_Standard_Pack
Date&Time       :2017/11/16 上午 10:45
Describe        :先把要送的資料組好
*/
int inFUBON_ECR_8N1_Standard_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer)
{
	int	inPacketSizes = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	inDISP_LogPrintfWithFlag("  TransType[%s] Kiosk[%u] Line[%d]",srECROb->srTransData.szTransType, pobTran->uszKioskFlag, __LINE__);
	
	if(!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_ ,2))
	{
		/* Indicator  (1 Byte) */
		inECR_PACK_EcrIndicator(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);	

		/* VersionDate  (6 Bytes) */
		inECR_PACK_VersionDate(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Unattend Flag  (1 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 1);

		/* TransType  (2 Byte) */
		inECR_PACK_TransType(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);	
		
		/* Card Card Type Indicator (1 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 1);	

		/* HOST ID (2 Bytes) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 2);

		/* Trace No [Inv. no] (6 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 6);

		/* Card No (19 Byte)(左靠右補空白) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 19);

		/* Card Expire Date (4 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 4);

		/* Trans Amount (12 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 12);

		/* Trans Date (6 Byte) */
		inECR_PACK_TransDate(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);
		
		/* Trans Time (6 Byte) */
		inECR_PACK_TransTime(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* Approval No (9 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 9);

		/* Card Type Indicator (1 Byte)*/
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 1);

		/* Response Text (4 Byte) */
		inECR_PACK_SuccessResponseCode(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* Merchant Id (15 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 15);

		/* Terminal ID (8 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 8);

		/* Filler  (12 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 12);

		/* Store Id  (18 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 18);

		/* Indicator  (1 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 1);

		/* RedeemptionAmount  (12 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 12);

		/* RedeemptionPoint  (8 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 8);

		/* RedeemptionBalancePoint  (8 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 8);

		/* RedeemptionPaidAmount  (12 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 12);

		/* InstallmentPeriod  (2 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 2);

		/* InstallmentDownPaymentAmount  (12 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 12);

		/* InstallmentAmount  (12 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 12);

		/* InstallmentFormalityFee  (12 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 12);

		/* Card Type (2 Byte)*/
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 2);

		/* Batch No.  (6 Byte)*/
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 6);

		/* Encrypted Card Number  (50 Bytes)*/
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 50);
		
		if(pobTran->uszKioskFlag == 'Y')
		{ 
			if( VS_SUCCESS == inECR_PACK_GetTotalAmountRecord(pobTran))
			{
				/* Sale Count (3 Bytes) */
				inECR_PACK_SaleTotalCount(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

				/* Sale Total Amt  (12 Bytes) */
				inECR_PACK_SaleTotalAmt(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

				/* Refund Count  (3 Bytes) */
				inECR_PACK_RefundTotalCount(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

				/* Refund Total Amt  (12 Bytes) */
				inECR_PACK_RefundTotalAmt(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);
			}else
			{
				inDISP_DispLogAndWriteFlie(" Fubon Settle Read Amt Rec is spaec Line[%d]",__LINE__);

				/* Sale Count (2 Bytes) */
				inECR_PACK_Zero(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 3);

				/* Sale Total Amt  (12 Bytes) */
				inECR_PACK_Zero(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 12);

				/* Refund Count  (2 Bytes) */
				inECR_PACK_Zero(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 3);

				/* Refund Total Amt  (12 Bytes) */
				inECR_PACK_Zero(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 12);
			}
			
			/* Filler  (100 Byte) */
			inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 100);
		}else
		{
			/* Filler  (130 Byte) */
			inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 130);
		}
		
	}else{
		/* Indicator  (1 Byte) */
		inECR_PACK_EcrIndicator(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);	

		/* VersionDate  (6 Bytes) */
		inECR_PACK_VersionDate(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Filler  (1 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 1);

		/* TransType  (2 Byte) */
		inECR_PACK_TransType(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);	

		inECR_PACK_CardTypeIndicator(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);	

		inECR_PACK_HostId(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);	

		/* Trace No [Inv. no] (6 Byte) */
		inECR_PACK_InvoiceNumber(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* Card No (19 Byte)(左靠右補空白) */
		inECR_PACK_CardNumber(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* Card Expire Date (4 Byte) */
		inECR_PACK_ExpireDate(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* Trans Amount (12 Byte) */
		inECR_PACK_TransAmount(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* Trans Date (6 Byte) & Trans Time (6 Byte) */
		inECR_PACK_TransDate(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		inECR_PACK_TransTime(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* Approval No (9 Byte) */
		inECR_PACK_ApprovalNumber(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* Contactless Indicator (1 Byte)*/
		inECR_PACK_CtlsIndicator(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* Response Text (4 Byte) */
		inECR_PACK_SuccessResponseCode(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* Merchant Id (15 Byte) */
		inECR_PACK_MerchantId(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* Terminal ID (8 Byte) */
		inECR_PACK_TerminalId(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* Filler  (12 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 12);

		/* Store Id  (18 Byte) */
		inECR_PACK_StoreId(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* Indicator  (1 Byte) */
		if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
			inECR_PACK_RedeemIndicator(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);
		else
			inECR_PACK_InstallmentIndicator(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* RedeemptionAmount  (12 Byte) */
		inECR_PACK_RedeemptionAmount(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* RedeemptionPoint  (8 Byte) */
		inECR_PACK_RedeemptionPoint(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* RedeemptionBalancePoint  (8 Byte) */
		inECR_PACK_RedeemptionBalancePoint(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* RedeemptionPaidAmount  (12 Byte) */
		inECR_PACK_RedeemptionPaidAmount(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* InstallmentPeriod  (2 Byte) */
		inECR_PACK_InstallmentPeriod(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* InstallmentDownPaymentAmount  (12 Byte) */
		inECR_PACK_InstallmentDownPaymentAmount(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* InstallmentAmount  (12 Byte) */
		inECR_PACK_InstallmentAmount(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* InstallmentFormalityFee  (12 Byte) */
		inECR_PACK_InstallmentFormalityFee(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* Card Type (2 Byte)*/
		inECR_PACK_CardType(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* Batch No.  (6 Byte)*/
		inECR_PACK_BatchNumber(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* Encrypted Card Number  (50 Bytes)*/
		inECR_PACK_NewHashCardNumber(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes);

		/* Filler  (130 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes],&inPacketSizes, 130);
	}
	
	inDISP_LogPrintfWithFlag("--- PackSize  [%d]  ", inPacketSizes);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d END -----",__FILE__, __FUNCTION__, __LINE__);

	return (inPacketSizes);
}




/*
Function        : inFUBON_ECR_8N1_Standard_Pack_ResponseCode
Date&Time       : 
Describe        :
*/
int inFUBON_ECR_8N1_Standard_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf)
{
	int inRespCodeAddr = 76;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" ---> ErrorType[%d]",srECROb->srTransData.inErrorType);
        
	if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_EXCESSIVE_TRANS_AMT_)
	{
		memcpy(&szDataBuf[inRespCodeAddr], "0006", 4);
	} else	if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_NEAR_MAX_SETTLE_CNT_)
	{
		memcpy(&szDataBuf[inRespCodeAddr], "0007", 4);
	}else	if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_PLESE_SETTLE_)
	{
		memcpy(&szDataBuf[inRespCodeAddr], "0008", 4);
	}else if (srECROb->srTransData.inErrorType == VS_CALLBANK)
	{
		memcpy(&szDataBuf[inRespCodeAddr], "0002", 4);
	}
	else if (srECROb->srTransData.inErrorType == VS_TIMEOUT		||
		 srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_TIMEOUT_)
	{
		memcpy(&szDataBuf[inRespCodeAddr], "0003", 4);
	}
	else if (srECROb->srTransData.inErrorType == VS_USER_OPER_ERR	||	/* 操作錯誤 & 交易流程有誤 */
		 srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_) 
	{
		memcpy(&szDataBuf[inRespCodeAddr], "0004", 4);
	}
	else if (srECROb->srTransData.inErrorType == VS_ISO_PACK_ERR		|| 
		 srECROb->srTransData.inErrorType == VS_ISO_UNPACK_ERROR	||
		 srECROb->srTransData.inErrorType == VS_COMM_ERROR		||
		 srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_COMM_ERROR_)
	{
		memcpy(&szDataBuf[inRespCodeAddr], "0005", 4);
	}
	else if (srECROb->srTransData.inErrorType == VS_USER_CANCEL || 
		 srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_USER_TERMINATE_ERROR_)
	{
		memcpy(&szDataBuf[inRespCodeAddr], "0001", 4);
	}
	else
	{	
		memcpy(&szDataBuf[inRespCodeAddr], "0001", 4);
	}
	
        
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}





