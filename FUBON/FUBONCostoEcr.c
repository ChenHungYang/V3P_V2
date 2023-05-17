
#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Flow.h"

#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/CFGT.h"

#include "../SOURCE/FUNCTION/ECR_FUNC/ECR.h"
#include "../SOURCE/FUNCTION/ECR_FUNC/RS232.h"
#include "../SOURCE/FUNCTION/ECR_FUNC/EcrPackFunc.h"


#include "FUBONCostoEcr.h"

extern unsigned char guszCTLSInitiOK;

int inFUBON_CostcoEcr_8N1_SelectSaleTransType(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int inPageLoop = _ECR_PAGE_LOOP_1_;
	int inRetVal = VS_SUCCESS;
	int inChioce1 = _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_1_;
	int inCount = 1;
	int inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_2_TO_7_3X4_;
	char szKey = 0x00;
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("1一般交易", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
	inDISP_ChineseFont("2多利金抵扣", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
	inDISP_ChineseFont("3分期付款", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
	inDISP_ChineseFont("4購物卡扣款", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
	
	/* 設定Timeout */
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	
	while (inPageLoop != _ECR_PAGE_LOOP_0_)
	{
		while (inPageLoop == _ECR_PAGE_LOOP_1_)
		{
			szKey = 0;
			szKey = uszKBD_Key();
			inChioce1 = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc); /* 回傳MENU畫面的按鈕選擇 */

			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			switch (inChioce1)
			{
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_1_:
					inCount = 1;
					inPageLoop = _ECR_PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_2_:
					inCount = 2;
					inPageLoop = _ECR_PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_3_:
					inCount = 3;
					inPageLoop = _ECR_PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_4_:
					inCount = 4;
					inPageLoop = _ECR_PAGE_LOOP_2_;
					break;
				default:
					break;
			}
			
			if (szKey == _KEY_1_)
			{
				inCount = 1;
				inPageLoop = _ECR_PAGE_LOOP_2_;
				break;
			} else if (szKey == _KEY_2_)
			{
				inCount = 2;
				inPageLoop = _ECR_PAGE_LOOP_2_;
				break;
			} else if (szKey == _KEY_3_)
			{
				inCount = 3;
				inPageLoop = _ECR_PAGE_LOOP_2_;
				break;
			} else if (szKey == _KEY_4_)
			{
				inCount = 4;
				inPageLoop = _ECR_PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				inPageLoop = _ECR_PAGE_LOOP_0_;
				break;
			} else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				inPageLoop = _ECR_PAGE_LOOP_0_;
				break;
			} else if (szKey == _KEY_ENTER_)
			{
				inPageLoop = _ECR_PAGE_LOOP_2_;
				break;
			}

		}

		/* 判斷MENU點進去按鈕之後要做的事情 */
		if (inPageLoop == _ECR_PAGE_LOOP_2_)
		{
			switch (inCount)
			{
				case 1:
//					inRetVal = inMENU_TMS_PARAMETER_PRINT(srEventMenuItem); /* TMS參數列印 */
					inPageLoop = _ECR_PAGE_LOOP_0_;
					break;

				default:
					inRetVal = VS_SUCCESS;
					inPageLoop = _ECR_PAGE_LOOP_1_;
					break;
			}
		}

		/* 代表回上一頁，要回復UI */
		if (inPageLoop == _ECR_PAGE_LOOP_1_ &&
			inRetVal == VS_ERROR)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("1一般交易", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
			inDISP_ChineseFont("2多利金抵扣", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
			inDISP_ChineseFont("3分期付款", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
			inDISP_ChineseFont("4購物卡扣款", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		}/* 功能未開而返回，不刷畫面 */
		else if (inPageLoop == _ECR_PAGE_LOOP_1_ &&
			inRetVal == VS_SUCCESS)
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		}

	}/* _PAGE_LOOP_0_ */
	
	return (inRetVal);
}

/*
Function		: inFUBON_CostcoRs232Ecr_8N1_Standard_Initial
Date&Time		: 2022/9/13 上午 11:58
Describe		: 
 * Costco 使用的ECR設定 
 */
int inFUBON_CostcoRs232Ecr_8N1_Standard_Initial(ECR_TABLE *srECROb)
{
	int inChoice = 0;
	int inTouchSensorFunc = _Touch_CUP_LOGON_;
	char szKey = 0x00;
	char szECRComPort[4 + 1];
	unsigned char uszParity;
	unsigned char uszDataBits;
	unsigned char uszStopBits;
	unsigned long ulBaudRate;
	unsigned short usRetVal;

	memset(&uszParity, 0x00, sizeof (uszParity));
	memset(&uszDataBits, 0x00, sizeof (uszDataBits));
	memset(&uszStopBits, 0x00, sizeof (uszStopBits));
	memset(&ulBaudRate, 0x00, sizeof (ulBaudRate));

	/* 從EDC.Dat抓出哪一個Comport */
	/* inGetECRComPort */
	memset(&szECRComPort, 0x00, sizeof (szECRComPort));
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
	uszParity = 'E';

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
				inDISP_LogPrintfWithFlag("inRS232_Open OK");
				inDISP_LogPrintfWithFlag("COM%d BaudRate:%lu %d%c%d",
						srECROb->srSetting.uszComPort, ulBaudRate, uszDataBits, uszParity, uszStopBits);
				break;

			} else
			{
				inDISP_LogPrintfWithFlag("inRS232_Open Error: 0x%04x", usRetVal);
				inDISP_LogPrintfWithFlag("COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort + 1, ulBaudRate, uszDataBits, uszParity, uszStopBits);

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

						if (szKey == _KEY_ENTER_ ||
								szKey == _KEY_TIMEOUT_ ||
								inChoice == _CUPLogOn_Touch_KEY_2_)
						{
							inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
							break;
						} else if (szKey == _KEY_0_)
						{
							/* 壓住0後3秒內按clear */
							inDISP_Timer_Start(_TIMER_NEXSYS_4_, 3);
							do
							{
								szKey = uszKBD_Key_In();
							} while (szKey == 0 &&
									inTimerGet(_TIMER_NEXSYS_4_) != VS_SUCCESS);

							/* 按clear */
							if (szKey == _KEY_CLEAR_)
							{
								return (VS_SUCCESS);
							}
						} else
						{

						}

					}/* 重新初始化迴圈 */
					/* 清空Touch資料 */
					inDisTouch_Flush_TouchFile();

				}/* 若接上底座還是錯誤，就回傳錯誤 */
				else
				{
					return (VS_ERROR);
				}
			}
		} while (1);
	}		/* CounterTop 機型 */
	else
	{
		usRetVal = inRS232_Open(srECROb->srSetting.uszComPort, ulBaudRate, uszParity, uszDataBits, uszStopBits);

		if (usRetVal != VS_SUCCESS)
		{
			inDISP_LogPrintfWithFlag("inRS232_Open Error: 0x%04x", usRetVal);
			inDISP_LogPrintfWithFlag("COM%d BaudRate:%lu %d%c%d",
					srECROb->srSetting.uszComPort + 1, ulBaudRate, uszDataBits, uszParity, uszStopBits);
			return (VS_ERROR);
		} else
		{
			inDISP_LogPrintfWithFlag("inRS232_Open OK");
			inDISP_LogPrintfWithFlag("COM%d BaudRate:%lu %d%c%d",
					srECROb->srSetting.uszComPort, ulBaudRate, uszDataBits, uszParity, uszStopBits);
		}
	}

	/* 清空接收的buffer */
	inRS232_FlushRxBuffer(srECROb->srSetting.uszComPort);

	return (VS_SUCCESS);
}

/*
Function		: inFUBON_CostcoRs232Ecr_8N1_Standard_Receive_Packet
Date&Time	: 2022/9/13 下午 2:44
Describe		: 處理收銀機傳來的資料
 */
int inFUBON_CostcoRs232Ecr_8N1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int inRetVal;
	char szDataBuffer[_ECR_RS232_BUFF_SIZE_]; /* 電文不包含STX和LRC */

	memset(&szDataBuffer, 0x00, sizeof (szDataBuffer));
	/* -----------------------開始接收資料------------------------------------------ */
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, FU_COSTCO_ECR_107_8N1_LEN);

	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}

	/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inFUBON_CostcoRs232Ecr_8N1_Standard_Unpack(pobTran, srECROb, szDataBuffer);

	if (inRetVal != VS_SUCCESS)
	{
		inRS232_ECR_SendError(pobTran, inRetVal);
		return (inRetVal);
	}


	return (VS_SUCCESS);
}

/*
Function		: inFUBON_RS232_ECR_8N1_Standard_Send_Packet
Date&Time	: 2019/01/18
Describe        :處理要送給收銀機的資料
 */
int inFUBON_CostcoRs232Ecr_8N1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int inRetVal = VS_ERROR;
	char szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0}; /* 封包資料 */

	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponce == VS_TRUE)
		return (VS_SUCCESS);

	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof (szDataBuf));
	/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inFUBON_CostcoRs232Ecr_8N1_Standard_Pack(pobTran, srECROb, szDataBuf);

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
int inFUBON_CostcoRs232Ecr_8N1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int inRetVal;
	char szDataBuf[_ECR_RS232_BUFF_SIZE_]; /* 封包資料 */

	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponce == VS_TRUE)
		return (VS_SUCCESS);

	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof (szDataBuf));
	/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inFUBON_CostcoRs232Ecr_8N1_Standard_Pack(pobTran, srECROb, szDataBuf);

	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}

	/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inFUBON_CostcoRs232Ecr_8N1_Standard_Pack_ResponseCode(pobTran, srECROb, szDataBuf);

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
int inFUBON_CostcoRs232Ecr_8N1_Standard_Close(ECR_TABLE* srECRob)
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
int inFUBON_CostcoRs232Ecr_8N1_Standard_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer)
{
	int i;
	int inRetVal = VS_SUCCESS;
	int inTransType = 0; /* 電文中的交易別字串轉為數字儲存 */
	int inCountUnpackLen = 0;
	char szTemplate[100 + 1] = {0};
	char szCTLSEnable[2 + 1] = {0};

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	/* 收到資料就嗶一下。  */
	CTOS_Beep();
	
	/* Trans Type (交易別) [1 Bytes ]*/
	memset(srECROb->srTransData.szTransType, 0x00, sizeof (srECROb->srTransData.szTransType));	
	inTransType = szDataBuffer[inCountUnpackLen];
	/* 因為固定欄位為2Bytes, 所以用第一個byte就好*/
	srECROb->srTransData.szTransType[0] = szDataBuffer[inCountUnpackLen];
	inCountUnpackLen += 1;

	inDISP_LogPrintfWithFlag(" Trans Type[%x] ", inTransType);

	/* Trans Amount [12 Bytes] */
	memset(szTemplate, 0x00, sizeof (szTemplate));
	memcpy(&szTemplate[0], &szDataBuffer[inCountUnpackLen], 10);
	switch (inTransType)
	{
		case _FU_COSTCO_8N1_SALE_NO_:
		case _FU_COSTCO_8N1_REFUND_NO_:
			for (i = 0; i < 12; i ++)
			{
				if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9')) //金額可能位移，不能有空白
					continue;
				else
				{
					srECROb->srTransData.inErrorType = _ECR_RESPONSE_CODE_EXCESSIVE_TRANS_AMT_;
					return (VS_ERROR);
				}
			}
			
			//交易金額
			pobTran->srBRec.lnTxnAmount = atol(szTemplate);
			pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
			pobTran->srBRec.lnTotalTxnAmount = atol(szTemplate);

			inDISP_LogPrintfWithFlag(" 交易金額 : [%ld]", pobTran->srBRec.lnTxnAmount);
			break;
		default:
			break;
	}

	inCountUnpackLen += 12;

	
	/* 跑OPT */
	switch (inTransType)
	{
		case _FU_COSTCO_8N1_REFUND_NO_:

			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_CUP_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜銀聯退貨> */

				pobTran->inFunctionID = _CUP_REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);

				if (!memcmp(&szCTLSEnable[0], "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_CUP_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_CUP_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_;

				}


				pobTran->inTransactionCode = _CUP_REFUND_;
				pobTran->srBRec.inCode = _CUP_REFUND_;
				pobTran->srBRec.inOrgCode = _CUP_REFUND_;

				inDISP_LogPrintfWithFlag("--- START ECR Sale  CUP ----");
			} else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜一般退貨＞ */

				pobTran->inFunctionID = _REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_;
				} else
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

		case _ECR_8N1_INSTALLMENT_REFUND_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜分期退貨＞ */

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
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜分期付款＞ */

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
		case _FU_COSTCO_8N1_SALE_NO_:

			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜銀聯一般交易＞ */

				pobTran->inFunctionID = _CUP_SALE_;
				pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
				pobTran->inRunTRTID = _TRT_CUP_SALE_;

				pobTran->inTransactionCode = _CUP_SALE_;
				pobTran->srBRec.inCode = _CUP_SALE_;
				pobTran->srBRec.inOrgCode = _CUP_SALE_;

				inDISP_LogPrintfWithFlag("--- START ECR Sale  CUP ----");
			} else
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜一般交易＞ */
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜一般交易＞ */

				pobTran->inFunctionID = _SALE_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_SALE_CTLS_;
				} else
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

			inDISP_LogPrintfWithFlag("--- END ECR Sale [%d]----", inRetVal);
			break;

		default:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			inRetVal = VS_ERROR;
			break;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
Function        : inFUBON_ECR_8N1_Standard_Pack
Date&Time       :2017/11/16 上午 10:45
Describe        :先把要送的資料組好
 */
int inFUBON_CostcoRs232Ecr_8N1_Standard_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer)
{
	int inPacketSizes = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	inDISP_LogPrintfWithFlag("  TransType[%s] Kiosk[%u] Line[%d]", srECROb->srTransData.szTransType, pobTran->uszKioskFlag, __LINE__);

	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2))
	{
		/* Indicator  (1 Byte) */
		inECR_PACK_EcrIndicator(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* VersionDate  (6 Bytes) */
		inECR_PACK_VersionDate(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Unattend Flag  (1 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 1);

		/* TransType  (2 Byte) */
		inECR_PACK_TransType(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Card Card Type Indicator (1 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 1);

		/* HOST ID (2 Bytes) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 2);

		/* Trace No [Inv. no] (6 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 6);

		/* Card No (19 Byte)(左靠右補空白) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 19);

		/* Card Expire Date (4 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 4);

		/* Trans Amount (12 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 12);

		/* Trans Date (6 Byte) */
		inECR_PACK_TransDate(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Trans Time (6 Byte) */
		inECR_PACK_TransTime(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Approval No (9 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 9);

		/* Card Type Indicator (1 Byte)*/
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 1);

		/* Response Text (4 Byte) */
		inECR_PACK_SuccessResponseCode(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Merchant Id (15 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 15);

		/* Terminal ID (8 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 8);

		/* Filler  (12 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 12);

		/* Store Id  (18 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 18);

		/* Indicator  (1 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 1);

		/* RedeemptionAmount  (12 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 12);

		/* RedeemptionPoint  (8 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 8);

		/* RedeemptionBalancePoint  (8 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 8);

		/* RedeemptionPaidAmount  (12 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 12);

		/* InstallmentPeriod  (2 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 2);

		/* InstallmentDownPaymentAmount  (12 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 12);

		/* InstallmentAmount  (12 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 12);

		/* InstallmentFormalityFee  (12 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 12);

		/* Card Type (2 Byte)*/
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 2);

		/* Batch No.  (6 Byte)*/
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 6);

		/* Encrypted Card Number  (50 Bytes)*/
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 50);

		if (pobTran->uszKioskFlag == 'Y')
		{
			if (VS_SUCCESS == inECR_PACK_GetTotalAmountRecord(pobTran))
			{
				/* Sale Count (3 Bytes) */
				inECR_PACK_SaleTotalCount(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

				/* Sale Total Amt  (12 Bytes) */
				inECR_PACK_SaleTotalAmt(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

				/* Refund Count  (3 Bytes) */
				inECR_PACK_RefundTotalCount(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

				/* Refund Total Amt  (12 Bytes) */
				inECR_PACK_RefundTotalAmt(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);
			} else
			{
				inDISP_DispLogAndWriteFlie(" Fubon Settle Read Amt Rec is spaec Line[%d]", __LINE__);

				/* Sale Count (2 Bytes) */
				inECR_PACK_Zero(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 3);

				/* Sale Total Amt  (12 Bytes) */
				inECR_PACK_Zero(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 12);

				/* Refund Count  (2 Bytes) */
				inECR_PACK_Zero(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 3);

				/* Refund Total Amt  (12 Bytes) */
				inECR_PACK_Zero(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 12);
			}

			/* Filler  (100 Byte) */
			inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 100);
		} else
		{
			/* Filler  (130 Byte) */
			inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 130);
		}

	} else
	{
		//	/* 主要是先抓取此筆交易的TransType  */
		//	inECR_PACK_GetTransType(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);	

		//		if(!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_ ,2))
		//		{
		//			memcpy(szTransTempBit, &szTransTable[1][0], sizeof(&szTransTable[1][0]));
		//			inDISP_LogPrintfWithFlag(" SALE Arr[%s] Line[%d] ",szTransTempBit, __LINE__);
		//		}else if(!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_ ,2))
		//		{
		//			
		//		}else if(!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_ ,2))
		//		{
		//			
		//		}else if(!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_ ,2))
		//		{
		//		
		//		}



		/* Indicator  (1 Byte) */
		//		if(szTransTempBit[1] == 'Y')
		inECR_PACK_EcrIndicator(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* VersionDate  (6 Bytes) */
		inECR_PACK_VersionDate(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Filler  (1 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 1);

		/* TransType  (2 Byte) */
		inECR_PACK_TransType(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		inECR_PACK_CardTypeIndicator(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		inECR_PACK_HostId(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Trace No [Inv. no] (6 Byte) */
		inECR_PACK_InvoiceNumber(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Card No (19 Byte)(左靠右補空白) */
		inECR_PACK_CardNumber(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Card Expire Date (4 Byte) */
		inECR_PACK_ExpireDate(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Trans Amount (12 Byte) */
		inECR_PACK_TransAmount(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Trans Date (6 Byte) & Trans Time (6 Byte) */
		inECR_PACK_TransDate(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		inECR_PACK_TransTime(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Approval No (9 Byte) */
		inECR_PACK_ApprovalNumber(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Contactless Indicator (1 Byte)*/
		inECR_PACK_CtlsIndicator(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Response Text (4 Byte) */
		inECR_PACK_SuccessResponseCode(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Merchant Id (15 Byte) */
		inECR_PACK_MerchantId(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Terminal ID (8 Byte) */
		inECR_PACK_TerminalId(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Filler  (12 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 12);

		/* Store Id  (18 Byte) */
		inECR_PACK_StoreId(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Indicator  (1 Byte) */
		if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
			inECR_PACK_RedeemIndicator(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);
		else
			inECR_PACK_InstallmentIndicator(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* RedeemptionAmount  (12 Byte) */
		inECR_PACK_RedeemptionAmount(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* RedeemptionPoint  (8 Byte) */
		inECR_PACK_RedeemptionPoint(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* RedeemptionBalancePoint  (8 Byte) */
		inECR_PACK_RedeemptionBalancePoint(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* RedeemptionPaidAmount  (12 Byte) */
		inECR_PACK_RedeemptionPaidAmount(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* InstallmentPeriod  (2 Byte) */
		inECR_PACK_InstallmentPeriod(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* InstallmentDownPaymentAmount  (12 Byte) */
		inECR_PACK_InstallmentDownPaymentAmount(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* InstallmentAmount  (12 Byte) */
		inECR_PACK_InstallmentAmount(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* InstallmentFormalityFee  (12 Byte) */
		inECR_PACK_InstallmentFormalityFee(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Card Type (2 Byte)*/
		inECR_PACK_CardType(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Batch No.  (6 Byte)*/
		inECR_PACK_BatchNumber(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Encrypted Card Number  (50 Bytes)*/
		inECR_PACK_HashCardNumber(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes);

		/* Filler  (130 Byte) */
		inECR_PACK_Filler(pobTran, &szDataBuffer[inPacketSizes], &inPacketSizes, 130);
	}

	inDISP_LogPrintfWithFlag("--- PackSize  [%d]  ", inPacketSizes);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d END -----", __FILE__, __FUNCTION__, __LINE__);

	return (inPacketSizes);
}

/*
Function        : inFUBON_ECR_8N1_Standard_Pack_ResponseCode
Date&Time       : 
Describe        :
 */
int inFUBON_CostcoRs232Ecr_8N1_Standard_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf)
{
	int inRespCodeAddr = 62;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag(" ---> ErrorType[%d]", srECROb->srTransData.inErrorType);

	if (srECROb->srTransData.inErrorType == VS_CALLBANK)
	{
		memcpy(&szDataBuf[inRespCodeAddr], "0002", 4);
	} else if (srECROb->srTransData.inErrorType == VS_TIMEOUT ||
			srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_TIMEOUT_)
	{
		memcpy(&szDataBuf[inRespCodeAddr], "0003", 4);
	} else if (srECROb->srTransData.inErrorType == VS_USER_OPER_ERR || /* 操作錯誤 & 交易流程有誤 */
			srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_)
	{
		memcpy(&szDataBuf[inRespCodeAddr], "0004", 4);
	} else if (srECROb->srTransData.inErrorType == VS_ISO_PACK_ERR ||
			srECROb->srTransData.inErrorType == VS_ISO_UNPACK_ERROR ||
			srECROb->srTransData.inErrorType == VS_COMM_ERROR ||
			srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_COMM_ERROR_)
	{
		memcpy(&szDataBuf[inRespCodeAddr], "0005", 4);
	} else if (srECROb->srTransData.inErrorType == VS_USER_CANCEL ||
			srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_USER_TERMINATE_ERROR_)
	{
		memcpy(&szDataBuf[inRespCodeAddr], "0006", 4);
	} else
	{
		memcpy(&szDataBuf[inRespCodeAddr], "0001", 4);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}



