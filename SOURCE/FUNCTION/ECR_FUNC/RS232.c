#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include <stdlib.h>
#include <sqlite3.h>


#include "../../INCLUDES/Define_1.h"
#include "../../INCLUDES/Define_2.h"
#include "../../INCLUDES/TransType.h"
#include "../../INCLUDES/Transaction.h"
#include "../../EVENT/Menu.h"
#include "../../PRINT/Print.h"
#include "../../DISPLAY/Display.h"
#include "../../DISPLAY/DisTouch.h"
#include "../../EVENT/MenuMsg.h"
#include "../../EVENT/Flow.h"

#include "../FILE_FUNC/File.h"
#include "../FILE_FUNC/FIleLogFunc.h"
#include "../Function.h"
#include "../FuncTable.h"
#include "../Sqlite.h"
#include "../CDT.h"
#include "../CFGT.h"
#include "../EDC.h"
#include "../HDT.h"
#include "../HDPT.h"
#include "../SCDT.h"
#include "../VWT.h"

#include "ECR.h"
#include "RS232.h"

#include "../MULTI_FUNC/MultiFunc.h"

#include "../../FUNCTION/CREDIT_FUNC/CreditCheckFunc.h"

#include "../../FUNCTION/UNIT_FUNC/TimeUint.h"

#include "../../../FUBON/FUBONEcr.h"

#include "../COSTCO_FUNC/Costco.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

extern int		ginDebug;
extern int		ginISODebug;
extern int		ginDisplayDebug;
extern unsigned char	guszCTLSInitiOK;
extern ECR_TABLE	gsrECROb;

ECR_TRANS_TABLE stRS232_ECRTable[] =
{
	/* 第零組標準【400】【8N1】規格 */
	{
		inRS232_ECR_8N1_Standard_Initial,
		inRS232_ECR_8N1_Standard_Receive_Packet,
		inRS232_ECR_8N1_Standard_Send_Packet,
		inRS232_ECR_8N1_Standard_Send_Error,
		inRS232_ECR_8N1_Standard_Close,
	},
	
	/* 第一組標準【144】【7E1】規格 */
	{
		inRS232_ECR_7E1_Standard_Initial,
		inRS232_ECR_7E1_Standard_Receive_Packet,
		inRS232_ECR_7E1_Standard_Send_Packet,
		inRS232_ECR_7E1_Standard_Send_Error,
		inRS232_ECR_7E1_Standard_Close,
	},
	
	/* 第二組邦柏(107)客製化需求【400】【8N1】規格 */
	{
		inRS232_ECR_8N1_Customer_107_Bumper_Initial,
		inRS232_ECR_8N1_Customer_107_Bumper_Receive_Packet,
		inRS232_ECR_8N1_Customer_107_Bumper_Send_Packet,
		inRS232_ECR_8N1_Customer_107_Bumper_Send_Error,
		inRS232_ECR_8N1_Customer_107_Bumper_Close,
	},
	/* 第三組 富邦規格【400】【8N1】規格 */
	{
		inFUBON_RS232_ECR_8N1_Standard_Initial,
		inFUBON_RS232_ECR_8N1_Standard_Receive_Packet,
		inFUBON_RS232_ECR_8N1_Standard_Send_Packet,
		inFUBON_RS232_ECR_8N1_Standard_Send_Error,
		inFUBON_RS232_ECR_8N1_Standard_Close,
	},
        /* 第四組 富邦好市多(Costco)規格【變動長度】【8N1】規格 */
	{
		inCostco_GasStation_RS232_ECR_8N1_Initial,
		inCostco_GasStation_RS232_ECR_8N1_Receive_Packet,
		inCostco_GasStation_RS232_ECR_8N1_Send_Packet,
		inCostco_GasStation_RS232_ECR_8N1_Send_Error,
//		inCostco_GasStation_RS232_ECR_8N1_Close,
	},
	
};

/*
Function        :inRS232_Open
Date&Time       :2017/7/13 上午 11:54
Describe        :
*/
int inRS232_Open(unsigned char uszComport, unsigned long ulBaudRate, unsigned char uszParity, unsigned char uszDataBits, unsigned char uszStopBits)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_RS232Open(uszComport, ulBaudRate, uszParity, uszDataBits, uszStopBits);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "CTOS_RS232Open Error: 0x%04x", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", uszComport + 1, ulBaudRate, uszDataBits, uszParity, uszStopBits);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
		    inDISP_LogPrintf("CTOS_RS232Open OK");
		    memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		    sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", uszComport, ulBaudRate, uszDataBits, uszParity, uszStopBits);
		    inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_SUCCESS);
	}
	
}

/*
Function        :inRS232_Close
Date&Time       :2017/7/13 下午 1:50
Describe        :
*/
int inRS232_Close(unsigned char uszComport)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_RS232Close(uszComport);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "CTOS_RS232Close Error: 0x%04x", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "COM%d", uszComport + 1);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
		    inDISP_LogPrintf("CTOS_RS232Close OK");
		    memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		    sprintf(szDebugMsg, "COM%d", uszComport + 1);
		    inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_SUCCESS);
	}
	
}

/*
Function        :inRS232_FlushRxBuffer
Date&Time       :2016/6/21 上午 10:54
Describe        :清空buffer中的資料
*/
int inRS232_FlushRxBuffer(unsigned char uszComPort)
{
	unsigned short	usRetVal;
	
	/* 沒設定完成，不用檢查 */
	if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
	{
		inDISP_DispLogAndWriteFlie(" RS232 FlashRxBuf Not OK [%u] Line[%d]",gsrECROb.srSetting.uszSettingOK, __LINE__);
		return (VS_ERROR);
	}
	
	/* 清空接收的buffer */
	usRetVal = CTOS_RS232FlushRxBuffer(uszComPort);
	if (usRetVal != d_OK)
	{
		inDISP_DispLogAndWriteFlie(" RS232 FlashRxBuf *Error* RetVal[0x%04x] Line[%d]", usRetVal, __LINE__);
		return (VS_ERROR);
	}
	else
	{
		inDISP_LogPrintfWithFlag(" RS232 FlashRxBuf Successs Com[%d]", uszComPort + 1);
		return (VS_SUCCESS);
	}
	       
}

/*
Function        :inRS232_FlushTxBuffer
Date&Time       :2017/6/26 下午 5:15
Describe        :清空Txbuffer中的資料
*/
int inRS232_FlushTxBuffer(unsigned char uszComPort)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	/* 沒設定完成，不用檢查 */
	if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
	{
		return (VS_ERROR);
	}
	
	/* 清空接收的buffer */
	usRetVal = CTOS_RS232FlushTxBuffer(uszComPort);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_RS232FlushTxBuffer Error: 0x%04x", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		/* 收到的資料長度歸0 */
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_RS232FlushTxBuffer Successs Com%d", uszComPort + 1);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_SUCCESS);
	}
	       
}

/*
Function        :inRS232_Data_Send_Check
Date&Time       :2016/6/20 下午 6:02
Describe        :檢查是否IO忙碌，可以傳送東西
*/
int inRS232_Data_Send_Check(unsigned char uszComPort)
{
	unsigned short	usRetVal;
	
	/* 檢查是否IO忙碌，可以傳送東西 */
        usRetVal = CTOS_RS232TxReady(uszComPort);
	if (usRetVal != d_OK)
	{
		/* Rx not ready 有可能沒開comport或其他原因 */
                return (VS_ERROR);         
        }
	else
		return (VS_SUCCESS);
}

/*
Function        :inRS232_Data_Send
Date&Time       :2016/7/11 下午 2:37
Describe        :RS232送東西API
*/
int inRS232_Data_Send(unsigned char uszComPort, unsigned char *uszSendBuff, unsigned short usSendSize)
{
	int	inRetVal;
	char	szDebugMsg[100 + 1];

	inRS232_FlushTxBuffer(uszComPort);
	
	inRetVal = CTOS_RS232TxData(uszComPort, uszSendBuff, usSendSize);
	if (inRetVal == d_OK)
	{
		return (VS_SUCCESS);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_RS232TxData Error: 0x%04x", inRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		
		return (VS_ERROR);
	}
}

/*
Function        :inRS232_Data_Receive_Check
Date&Time       :2016/6/20 下午 6:02
Describe        :RS232有收到東西，就觸發事件
*/
int inRS232_Data_Receive_Check(unsigned char uszComPort, unsigned short *usReceiveLen)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;

	/* 檢查資料是否可以從port被取得，看是否有收到資料長度，若有資料，*usReceiveLen會被放入資料長度 */
	usRetVal = CTOS_RS232RxReady(uszComPort, usReceiveLen);
	
	if (usRetVal == d_OK)
	{

	}
	/* 接底座若離開會跑這一個 */
	else if (usRetVal == d_ETHERNET_HARDWARE_ERROR)
	{

	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "CTOS_RS232RxReady : 0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		/* Rx not ready 有可能沒開comport或其他原因 */
                return (VS_ERROR);         
        }
	
	/* 有收到資料就回傳成功 */
	if (*usReceiveLen > 0)	
		return (VS_SUCCESS);
	else
		return (VS_ERROR);
	
}

/*
Function        :inRS232_Data_Receive
Date&Time       :2016/7/6 下午 3:28
Describe        :RS232收東西API
*/
int inRS232_Data_Receive(unsigned char uszComPort, unsigned char *uszReceBuff, unsigned short *usReceSize)
{
	int	inRetVal;

//	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	inRetVal = CTOS_RS232RxData(uszComPort, uszReceBuff, usReceSize);
	if (inRetVal == d_OK)
	{
//		inDISP_LogPrintfWithFlag("--> Success Return ");
//		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}
	else
	{
//		inDISP_LogPrintfWithFlag("--> Error Return [%d]", inRetVal);
//		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
}



/*
Function        : inRS232_ECR_Initial
Date&Time   : 2017/6/1 下午 5:46
Describe        :
*/
int inRS232_ECR_Initial(void)
{
	int	inRetVal = VS_ERROR;
	
	
	inRetVal = stRS232_ECRTable[gsrECROb.srSetting.inVersion].inEcrInitial(&gsrECROb);
	
	return (inRetVal);
}

/*
Function        :inRS232_ECR_Receive_Transaction
Date&Time       :2016/6/21 上午 11:42
Describe        :從ECR接收金額及交易別資料
*/
int inRS232_ECR_Receive_Transaction(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;

	/* 清空上次交易的資料 */
	memset(&gsrECROb.srTransData, 0x00, sizeof(ECR_TRANSACTION_DATA));

	/* 收資料 */	
	inRetVal = stRS232_ECRTable[gsrECROb.srSetting.inVersion].inEcrRece(pobTran, &gsrECROb);
	
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inRS232_ECR_Receive_Transaction ok");
		}
		return (VS_SUCCESS);
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inRS232_ECR_Receive_Transaction Timeout");
		}
		
		return (VS_TIMEOUT);
	}
	else if (inRetVal == VS_USER_CANCEL)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inRS232_ECR_Receive_Transaction Cancel");
		}
		
		return (VS_USER_CANCEL);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inRS232_ECR_Receive_Transaction Error");
		}
		
		return (VS_ERROR);
	}
	
        
}

/*
Function        :inRS232_ECR_Load_TMK_From_520
Date&Time       :2017/11/14 下午 4:58
Describe        :
*/
int inRS232_ECR_Load_TMK_From_520(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	unsigned short	usReceiveLen = 0;
	
	inDISP_TimeoutStart(10);
	
	while (1)
	{
		inRetVal = inRS232_Data_Receive_Check(gsrECROb.srSetting.uszComPort, &usReceiveLen);
		if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_4_);
			inDISP_ChineseFont("Waiting", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
			inDISP_Timer_Start(_TIMER_NEXSYS_3_, 1);
		}
		
		if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_4_);
			inDISP_ChineseFont("Timeout", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (usReceiveLen > 0)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_4_);
			inDISP_ChineseFont("Processing", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
			inRetVal = VS_SUCCESS;
			break;
		}
		
	}
	
	if (inRetVal == VS_SUCCESS)
	{
		/* 清空上次交易的資料 */
		memset(&gsrECROb.srTransData, 0x00, sizeof(ECR_TRANSACTION_DATA));
		/* 收資料 */	
		inRetVal = stRS232_ECRTable[gsrECROb.srSetting.inVersion].inEcrRece(pobTran, &gsrECROb);
	}
	
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inRS232_ECR_Load_TMK_From_520 ok");
		}
		return (VS_SUCCESS);
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inRS232_ECR_Load_TMK_From_520 Timeout");
		}
		
		return (VS_TIMEOUT);
	}
	else if (inRetVal == VS_USER_CANCEL)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inRS232_ECR_Load_TMK_From_520 Cancel");
		}
		
		return (VS_USER_CANCEL);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inRS232_ECR_Load_TMK_From_520 Error");
		}
		
		return (VS_ERROR);
	}
	
        
}


/*
Function        :inRS232_ECR_Send_Transaction_Result
Date&Time       :2016/7/11 下午 3:44
Describe        :印帳單前要送給ECR
*/
int inRS232_ECR_Send_Transaction_Result(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;

	/* 送資料 */	
	inRetVal = stRS232_ECRTable[gsrECROb.srSetting.inVersion].inEcrSend(pobTran, &gsrECROb);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inRS232_ECR_Send_Transaction_Result Error");
		}
		
		
		return (inRetVal);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inRS232_ECR_Send_Transaction_Result ok");
		}
		
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_SendError
Date&Time       :2016/7/18 上午 10:34
Describe        :送錯誤訊息給ECR
*/
int inRS232_ECR_SendError(TRANSACTION_OBJECT * pobTran, int inErrorType)
{
        if (pobTran->uszECRBit != VS_TRUE)
                return (VS_SUCCESS);
        
#ifdef _ESUN_MAIN_HOST_
        if( gsrECROb.srTransData.inErrorType  == _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
        {
                gsrECROb.srTransData.inErrorType = inErrorType;
        }
#else
        gsrECROb.srTransData.inErrorType = inErrorType;
#endif
        if (stRS232_ECRTable[gsrECROb.srSetting.inVersion].inEcrSendError(pobTran, &gsrECROb) != VS_SUCCESS)
            return (VS_ERROR);

        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_DeInitial
Date&Time       :2018/5/22 上午 10:14
Describe        :反初始化
*/
int inRS232_ECR_DeInitial(void)
{	
	if (stRS232_ECRTable[gsrECROb.srSetting.inVersion].inEcrEnd(&gsrECROb) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}



/*
Function        :inRS232_ECR_NCCC_7E1_To_8N1
Date&Time       :2017/11/15 下午 4:29
Describe        :回傳VS_ERROR代表不用轉換
*/
int inRS232_ECR_NCCC_7E1_To_8N1(ECR_TABLE * srECROb, unsigned char *uszReceiveBuffer)
{
	char	szCOMPort[4 + 1];
	
	memset(szCOMPort, 0x00, sizeof(szCOMPort));
	inGetECRComPort(szCOMPort);
	/* 非RS232，不用繼續判斷 */
	if (memcmp(szCOMPort, "COM1", strlen("COM1")) != 0	&&
	    memcmp(szCOMPort, "COM2", strlen("COM2")) != 0	&&
	    memcmp(szCOMPort, "COM3", strlen("COM3")) != 0	&&
	    memcmp(szCOMPort, "COM4", strlen("COM4")) != 0)
	{
		
		return (VS_ERROR);
	}
	
	
	if (srECROb->srSetting.inVersion == 1)
	{
		if (uszReceiveBuffer[1] == 'I')
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("Receive 'I'!");
			}
			
			inECR_DeInitial();
			
			inSetECRVersion("00");
			inSaveEDCRec(0);

			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("7E1轉8N1", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);

			inECR_Initial();

			return (VS_SUCCESS);
		}
		/* 不需要轉換協定 */
		return (VS_ERROR);
	}
	else
	{
		/* 不需要轉換協定 */
		return (VS_ERROR);
	}
}

/*
Function        :inRS232_ECR_NCCC_7E1_To_8N1_Cheat
Date&Time       :2018/5/22 上午 9:57
Describe        :我已經沒招了，直接作弊，parity Bit不同根本沒辦法判別，還轉個毛，直接判斷收到是0x00就轉8N1
*/
int inRS232_ECR_NCCC_7E1_To_8N1_Cheat(ECR_TABLE * srECROb, unsigned char *uszReceiveBuffer)
{
	char	szCOMPort[4 + 1];
	
	memset(szCOMPort, 0x00, sizeof(szCOMPort));
	inGetECRComPort(szCOMPort);
	/* 非RS232，不用繼續判斷 */
	if (memcmp(szCOMPort, "COM1", strlen("COM1")) != 0	&&
	    memcmp(szCOMPort, "COM2", strlen("COM2")) != 0	&&
	    memcmp(szCOMPort, "COM3", strlen("COM3")) != 0	&&
	    memcmp(szCOMPort, "COM4", strlen("COM4")) != 0)
	{
		
		return (VS_ERROR);
	}
	
	
	if (srECROb->srSetting.inVersion == 1)
	{
		if (uszReceiveBuffer[0] == 0x00)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("Receive 'I'!");
			}
			
			inECR_DeInitial();
			
			inSetECRVersion("00");
			inSaveEDCRec(0);

			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("7E1轉8N1", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);
			
			inECR_Initial();

			return (VS_SUCCESS);
		}
		/* 不需要轉換協定 */
		return (VS_ERROR);
	}
	else
	{
		/* 不需要轉換協定 */
		return (VS_ERROR);
	}
}

/*
Function        :inRS232_ECR_SelectTransType
Date&Time       :2016/6/22 上午 9:47
Describe        :Indicator為'S'時的交易選單
*/
int inRS232_ECR_SelectTransType(ECR_TABLE * srECROb)
{
	int			inTransType = 0;
	int			inRetVal = VS_SUCCESS;
	int			inChoice = 0;
	int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char			szTemplate[10 + 1];
	char			szDebugMsg[100 + 1];
	char			szKey = 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay1[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	, _SALE_		, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_ECR_OPTION_SALE_1_SALE_		},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	, _REDEEM_SALE_	, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_ECR_OPTION_SALE_2_REDEEM_SALE_	},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_	, _INST_SALE_		, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_ECR_OPTION_SALE_3_INST_SALE_		},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_7_	, _CUP_SALE_		, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_ECR_OPTION_SALE_4_CUP_SALE_		},
		{_Touch_NONE_				, _TRANS_TYPE_NULL_	, NULL						, ""							}
	};
	MENU_CHECK_TABLE	srMenuChekDisplay2[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	, _SALE_OFFLINE_	, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_ECR_OPTION_ADJUST_1_SALE_OFFLINE_	},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	, _REDEEM_ADJUST_	, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_ECR_OPTION_ADJUST_2_REDEEM_ADJUST_	},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_	, _INST_ADJUST_		, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_ECR_OPTION_ADJUST_3_INST_ADJUST_	},
		{_Touch_NONE_				, _TRANS_TYPE_NULL_	, NULL						, ""							}
	};
	MENU_CHECK_TABLE	srMenuChekDisplay3[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	, _REFUND_		, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_ECR_OPTION_REFUND_1_REFUND_		},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	, _REDEEM_REFUND_	, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_ECR_OPTION_REFUND_2_REDEEM_REFUND_	},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_	, _INST_REFUND_		, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_ECR_OPTION_REFUND_3_INST_REFUND_	},
		{_Touch_NONE_				, _TRANS_TYPE_NULL_	, NULL						, ""							}
	};
		
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memcpy(szTemplate, &srECROb->srTransData.szTransType[0], 2);
        inTransType = atoi(szTemplate);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "%d", inTransType);
		inDISP_LogPrintf(szDebugMsg);
        }
	
        switch (inTransType)
        {
                case _ECR_8N1_SALE_NO_ :
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */
			inDISP_PutGraphic(_MENU_SALE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
			/* 檢查功能開關，並顯示反白的圖 */
			inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay1);
			
			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
                        while (1)
                        {
                                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				szKey = uszKBD_Key();
                                
				/* 回傳Timeout */
				if (inTimerGet(_ECR_RS232_RECEIVE_TIMER_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}
				
				/* 一般交易 */
				/* 觸控要功能有開才能算 */
                                if (szKey == _KEY_1_			||
				   ((inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)	&& (inCREDIT_CheckTransactionFunction(_SALE_) == VS_SUCCESS)))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_SALE_, 2);
					inRetVal = VS_SUCCESS;
					break;
				}
				/* 紅利扣抵 */
                                else if (szKey == _KEY_2_			||
					((inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)	&& (inCREDIT_CheckTransactionFunction(_REDEEM_SALE_) == VS_SUCCESS)))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_REDEEM_, 2);
					inRetVal = VS_SUCCESS;
					break;
				}
				/* 分期付款 */
                                else if (szKey == _KEY_3_			||
					((inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)	&& (inCREDIT_CheckTransactionFunction(_INST_SALE_) == VS_SUCCESS)))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_INSTALLMENT_, 2);
					inRetVal = VS_SUCCESS;
					break;
				}
				else if (szKey == _KEY_4_			||
					((inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_7_)	&& (inCREDIT_CheckTransactionFunction(_CUP_SALE_) == VS_SUCCESS)))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_SALE_, 2);
					memcpy(&srECROb->srTransData.szCUPIndicator[0], "C", strlen("C"));
					inRetVal = VS_SUCCESS;
					break;
				}
                                else if (szKey == _KEY_CANCEL_)
				{
					inRetVal = VS_USER_CANCEL;
					break;
				}
				else if (szKey == _KEY_TIMEOUT_)
				{
					inRetVal = VS_TIMEOUT;
					break;
				}
                        }
			/* 清空Touch資料 */
			inDisTouch_Flush_TouchFile();

                        break;
                case _ECR_8N1_OFFLINE_NO_ :
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜交易補登＞ */
			inDISP_PutGraphic(_MENU_SALE_OFFLINE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
			/* 檢查功能開關，並顯示反白的圖 */
			inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay2);

			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
                        while (1)
                        {
                                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				szKey = uszKBD_Key();
                                
				/* 回傳Timeout */
				if (inTimerGet(_ECR_RS232_RECEIVE_TIMER_) == VS_SUCCESS)
				{
					return (VS_TIMEOUT);
				}
				
				/* 一般補登 */
                                if (szKey == _KEY_1_			||
				   (inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	&& (inCREDIT_CheckTransactionFunction(_SALE_OFFLINE_) == VS_SUCCESS)))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_OFFLINE_, 2);
					inRetVal = VS_SUCCESS;
					break;
                                }
				/* 紅利調帳 */
                                else if (szKey == _KEY_2_			||
					((inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)	&& (inCREDIT_CheckTransactionFunction(_REDEEM_ADJUST_) == VS_SUCCESS)))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_REDEEM_ADJUST_, 2);
					inRetVal = VS_SUCCESS;
					break;
                                }
				/* 分期調帳 */
                                else if (szKey == _KEY_3_			||
					((inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)	&& (inCREDIT_CheckTransactionFunction(_INST_ADJUST_) == VS_SUCCESS)))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_INSTALLMENT_ADJUST_, 2);
					inRetVal = VS_SUCCESS;
					break;
                                }
                                else if (szKey == _KEY_CANCEL_)
				{
					inRetVal = VS_USER_CANCEL;
					break;
				}
				else if (szKey == _KEY_TIMEOUT_)
				{
					inRetVal = VS_TIMEOUT;
					break;
				}
                        }
			/* 清空Touch資料 */
			inDisTouch_Flush_TouchFile();

                        break;
                case _ECR_8N1_REFUND_NO_ :
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜一般退貨＞ */
			inDISP_PutGraphic(_MENU_REFUND_OPTION_2_, 0, _COORDINATE_Y_LINE_8_4_);
			/* 檢查功能開關，並顯示反白的圖 */
			inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay3);

			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
                        while (1)
                        {
                                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				szKey = uszKBD_Key();
                                
				/* 回傳Timeout */
				if (inTimerGet(_ECR_RS232_RECEIVE_TIMER_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}
				
				/* 一般退貨 */
                                if (szKey == _KEY_1_			||
				   (inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	&& (inCREDIT_CheckTransactionFunction(_REFUND_) == VS_SUCCESS)))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_REFUND_, 2);
					inRetVal = VS_SUCCESS;
					break;
                                }
				/* 紅利退貨 */
                                else if (szKey == _KEY_2_			||
					(inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	&& (inCREDIT_CheckTransactionFunction(_REDEEM_REFUND_) == VS_SUCCESS)))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_REDEEM_REFUND_, 2);
					inRetVal = VS_SUCCESS;
					break;
                                }
				/* 分期退貨 */
                                else if (szKey == _KEY_3_			||
					((inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)	&& (inCREDIT_CheckTransactionFunction(_INST_REFUND_) == VS_SUCCESS)))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_INSTALLMENT_REFUND_, 2);
					inRetVal = VS_SUCCESS;
					break;
                                }
                                else if (szKey == _KEY_CANCEL_)
				{
					inRetVal = VS_USER_CANCEL;
					break;
				}
				else if (szKey == _KEY_TIMEOUT_)
				{
					inRetVal = VS_TIMEOUT;
					break;
				}
                        }
			/* 清空Touch資料 */
			inDisTouch_Flush_TouchFile();
			
                        break;

                default :
			inRetVal = VS_ERROR;
                        break;
        }
	
	return (inRetVal);
}

/*
Function        :inRS232_ECR_SelectOtherCardType
Date&Time       :2018/2/8 下午 5:04
Describe        :Indicator為'0'時的交易選單
*/
int inRS232_ECR_SelectOtherCardType(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = VS_SUCCESS;
	int			inChoice = 0;
	int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char			szKey = 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay1[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	, _SALE_		, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_ECR_OPTION_OTHERCARD_1_CREDITCARD_	},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	, _TRANS_TYPE_NULL_	, inMENU_Check_CUP_Enable			, _ICON_HIGHTLIGHT_ECR_OPTION_OTHERCARD_2_CUP_CARD_	},
		{_Touch_NONE_					, _TRANS_TYPE_NULL_	, NULL						, ""							}
	};
		
	inFunc_ResetTitle(pobTran);
	inDISP_PutGraphic(_MENU_ECR_TRANSTYPE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
	/* 檢查功能開關，並顯示反白的圖 */
	inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay1);

	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();

		/* 回傳Timeout */
		if (inTimerGet(_ECR_RS232_RECEIVE_TIMER_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		/* 一般交易 */
		/* 觸控要功能有開才能算 */
		if (szKey == _KEY_1_			||
		   ((inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)	&& (inCREDIT_CheckTransactionFunction(_SALE_) == VS_SUCCESS)))
		{
			
			inRetVal = VS_SUCCESS;
			break;
		}
		/* 銀聯交易 */
		else if (szKey == _KEY_2_			||
			((inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)	&& (inMENU_Check_CUP_Enable(_TRANS_TYPE_NULL_) == VS_SUCCESS)))
		{
			pobTran->srBRec.uszCUPTransBit = VS_TRUE;
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	return (inRetVal);
}

/*
Function        :inRS232_ECR_8N1_Standard_Initial
Date&Time       :2016/6/20 下午 2:55
Describe        :initial 8N1 COM PORT
*/
int inRS232_ECR_8N1_Standard_Initial(ECR_TABLE *srECROb)
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
//						else if (szKey == _KEY_CANCEL_)
//						{
//							inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
//							
//							return (VS_ERROR);
//						}
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
Function        :inRS232_ECR_8N1_Standard_Receive_Packet
Date&Time       :2016/7/6 下午 4:04
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_8N1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
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
	inRetVal = inECR_8N1_Standard_Unpack(pobTran, srECROb, szDataBuffer);
	
	if (inRetVal != VS_SUCCESS)
	{
		inRS232_ECR_SendError(pobTran, inRetVal);
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Standard_Send_Packet
Date&Time       :2016/7/11 下午 3:29
Describe        :處理要送給收銀機的資料
*/
int inRS232_ECR_8N1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponce == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_8N1_Standard_Pack(pobTran, srECROb, szDataBuf);
	
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
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Standard_Send_Error
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
*/
int inRS232_ECR_8N1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponce == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_8N1_Standard_Pack(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inECR_8N1_Standard_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
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
Function        :inRS232_ECR_8N1_Standard_Close
Date&Time       :2016/7/11 下午 3:34
Describe        :關閉Comport
*/
int inRS232_ECR_8N1_Standard_Close(ECR_TABLE* srECRob)
{
        /*關閉port*/
        if (inRS232_Close(srECRob->srSetting.uszComPort) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Standard_Initial
Date&Time       :2016/6/20 下午 2:55
Describe        :initial 7E1 COM PORT
*/
int inRS232_ECR_7E1_Standard_Initial(ECR_TABLE *srECROb)
{
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CUP_LOGON_;
	char		szKey = 0x00;
	char		szDebugMsg[100 + 1] = {0};
	char		szECRComPort[4 + 1] = {0};
	unsigned char	uszParity = 0;
	unsigned char	uszDataBits = 0;
	unsigned char	uszStopBits = 0;
	unsigned long	ulBaudRate = 0;
	unsigned short	usRetVal = 0;
	
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
	uszParity = 'E';
	
	/* Data Bits */
	uszDataBits = 7;
	
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
//						else if (szKey == _KEY_CANCEL_)
//						{
//							inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
//							
//							return (VS_ERROR);
//						}
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
Function        :inRS232_ECR_7E1_Standard_Receive_Packet
Date&Time       :2016/7/6 下午 4:04
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_7E1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_7E1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_7E1_Standard_Unpack(pobTran, szDataBuffer, srECROb);
	
	if (inRetVal != VS_SUCCESS)
	{
		inRS232_ECR_SendError(pobTran, inRetVal);
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Standard_Send_Packet
Date&Time       :2016/7/11 下午 3:29
Describe        :處理要送給收銀機的資料
*/
int inRS232_ECR_7E1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponce == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_7E1_Standard_Pack(pobTran, szDataBuf, srECROb);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Standard_Send_Error
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
*/
int inRS232_ECR_7E1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponce == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_7E1_Standard_Pack(pobTran, szDataBuf, srECROb);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inECR_7E1_Standard_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponce = VS_TRUE;
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Standard_Close
Date&Time       :2016/7/11 下午 3:34
Describe        :關閉Comport
*/
int inRS232_ECR_7E1_Standard_Close(ECR_TABLE* srECROb)
{
        /*關閉port*/
        if (inRS232_Close(srECROb->srSetting.uszComPort) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_LOAD_TMK_FROM_520_Standard_Initial
Date&Time       :2016/6/20 下午 2:55
Describe        :initial 8N1 COM PORT
*/
int inRS232_ECR_LOAD_TMK_FROM_520_Standard_Initial(ECR_TABLE *srECROb)
{
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CUP_LOGON_;
	char		szKey = 0x00;
	char		szDebugMsg[100 + 1] = {0};
	char		szECRComPort[4 + 1] = {0};
	unsigned char	uszParity = 0;
	unsigned char	uszDataBits = 0;
	unsigned char	uszStopBits = 0;
	unsigned long	ulBaudRate = 0;
	unsigned short	usRetVal = 0;
	
	
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
		
	
	/* BaudRate = 19200 */
	ulBaudRate = 19200;
	
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

						if (szKey == _KEY_ENTER_		 || 
						    szKey == _KEY_TIMEOUT_		 ||
						    inChoice == _CUPLogOn_Touch_KEY_2_)
						{
							inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
							break;
						}
//						else if (szKey == _KEY_CANCEL_)
//						{
//							inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
//							
//							return (VS_ERROR);
//						}
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
Function        :inRS232_ECR_LOAD_TMK_FROM_520_Standard_Receive_Packet
Date&Time       :2016/7/6 下午 4:04
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_LOAD_TMK_FROM_520_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	inRetVal = inECR_Not_Check_Sizes_Receive(pobTran, srECROb, szDataBuffer, _ECR_LOAD_TMK_FROM_520_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_LOAD_TMK_FROM_520_Standard_Unpack(pobTran, srECROb, szDataBuffer);
	
	if (inRetVal != VS_SUCCESS)
	{
		inRS232_ECR_SendError(pobTran, inRetVal);
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_LOAD_TMK_FROM_520_Standard_Send_Packet
Date&Time       :2016/7/11 下午 3:29
Describe        :處理要送給收銀機的資料
*/
int inRS232_ECR_LOAD_TMK_FROM_520_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponce == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_LOAD_TMK_FROM_520_Standard_Pack(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_LOAD_TMK_FROM_520_Standard_Data_Size_);
	
	/* 標示已送給ECR回覆電文 */
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_LOAD_TMK_FROM_520_Standard_Send_Error
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
*/
int inRS232_ECR_LOAD_TMK_FROM_520_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponce == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_LOAD_TMK_FROM_520_Standard_Pack(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_LOAD_TMK_FROM_520_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponce = VS_TRUE;
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_LOAD_TMK_FROM_520_Standard_Close
Date&Time       :2016/7/11 下午 3:34
Describe        :關閉Comport
*/
int inRS232_ECR_LOAD_TMK_FROM_520_Standard_Close(ECR_TABLE* srECRob)
{
        /*關閉port*/
        if (inRS232_Close(srECRob->srSetting.uszComPort) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}


/*
Function        :inRS232_ECR_8N1_Customer_107_Bumper_Initial
Date&Time       :2018/7/23 上午 11:52
Describe        :initial 8N1 COM PORT
*/
int inRS232_ECR_8N1_Customer_107_Bumper_Initial(ECR_TABLE *srECROb)
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
//						else if (szKey == _KEY_CANCEL_)
//						{
//							inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
//							
//							return (VS_ERROR);
//						}
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
Function        :inRS232_ECR_8N1_Customer_107_Bumper_Receive_Packet
Date&Time       :2016/7/6 下午 4:04
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_8N1_Customer_107_Bumper_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
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
	inRetVal = inECR_8N1_Customer_107_Bumper_Unpack(pobTran, srECROb, szDataBuffer);
	
	if (inRetVal != VS_SUCCESS)
	{
		inRS232_ECR_SendError(pobTran, inRetVal);
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_107_Bumper_Send_Packet
Date&Time       :2016/7/11 下午 3:29
Describe        :處理要送給收銀機的資料
*/
int inRS232_ECR_8N1_Customer_107_Bumper_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponce == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_8N1_Customer_107_Bumper_Pack(pobTran, srECROb, szDataBuf);
	
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
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_107_Bumper_Send_Error
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
*/
int inRS232_ECR_8N1_Customer_107_Bumper_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponce == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_8N1_Customer_107_Bumper_Pack(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inECR_8N1_Customer_107_Bumper_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
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
Function        :inRS232_ECR_8N1_Customer_107_Bumper_Close
Date&Time       :2016/7/11 下午 3:34
Describe        :關閉Comport
*/
int inRS232_ECR_8N1_Customer_107_Bumper_Close(ECR_TABLE* srECRob)
{
        /*關閉port*/
        if (inRS232_Close(srECRob->srSetting.uszComPort) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}



