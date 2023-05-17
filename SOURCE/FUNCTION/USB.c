#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/Transaction.h"
#include "../EVENT/Menu.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"
#include "../EVENT/MenuMsg.h"
#include "../EVENT/Flow.h"
#include "../FUNCTION/Function.h"
#include "../FUNCTION/Sqlite.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/SCDT.h"
#include "HDPT.h"
#include "CDT.h"
#include "ECR_FUNC/ECR.h"
#include "USB.h"

/* 20230413 Miyano add for USB Host Start */
#include <fcntl.h>
#include <unistd.h>
//#include <stdint.h>
#include <termios.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "MULTI_FUNC/MultiFunc.h"
/* 20230413 Miyano add for USB Host End */

extern int		ginDebug;
extern int		ginISODebug;
extern int		ginDisplayDebug;
extern int		ginEngineerDebug;
extern ECR_TABLE	gsrECROb;

/* 20230413 Miyano add */
extern MULTI_TABLE	gstMultiOb;
int                     ginHandleUSBPort = 0;

ECR_TRANS_TABLE stUSB_ECRTable[] =
{
	/* 第零組標準【400】【8N1】規格 */
	{
		inUSB_ECR_8N1_Standard_Initial,
		inUSB_ECR_8N1_Standard_Receive_Packet,
		inUSB_ECR_8N1_Standard_Send_Packet,
		inUSB_ECR_8N1_Standard_Send_Error,
		inUSB_ECR_8N1_Standard_Close,
	},
	
	/* 第一組標準【144】【7E1】規格 */
	{
		inUSB_ECR_7E1_Standard_Initial,
		inUSB_ECR_7E1_Standard_Receive_Packet,
		inUSB_ECR_7E1_Standard_Send_Packet,
		inUSB_ECR_7E1_Standard_Send_Error,
		inUSB_ECR_7E1_Standard_Close,
	},
	
	/* 第二組邦柏(107)客製化需求【400】【8N1】規格 */
	{
		inUSB_ECR_8N1_Customer_107_Bumper_Initial,
		inUSB_ECR_8N1_Customer_107_Bumper_Receive_Packet,
		inUSB_ECR_8N1_Customer_107_Bumper_Send_Packet,
		inUSB_ECR_8N1_Customer_107_Bumper_Send_Error,
		inUSB_ECR_8N1_Customer_107_Bumper_Close,
	},
};

/*
Function        :inUSB_Open
Date&Time       :2017/6/26 下午 4:30
Describe        :
*/
int inUSB_Open()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = VS_ERROR;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_USBOpen();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBOpen Error: 0x%04x", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBOpen Successs");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_Close
Date&Time       :2017/6/26 下午 4:31
Describe        :
*/
int inUSB_Close()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = VS_ERROR;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_USBClose();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBClose Error: 0x%04x", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBClose Successs");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_FlushTxBuffer
Date&Time       :2017/6/26 下午 5:15
Describe        :清空Txbuffer中的資料
*/
int inUSB_FlushTxBuffer()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	/* 沒設定完成，不用檢查 */
	if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
	{
		return (VS_ERROR);
	}
	
	/* 清空接收的buffer */
	usRetVal = CTOS_USBTxFlush();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBTxFlush Error: 0x%04x", usRetVal);
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
			sprintf(szDebugMsg, "CTOS_USBTxFlush Successs");
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_SUCCESS);
	}
	       
}

/*
Function        :inUSB_FlushRxBuffer
Date&Time       :2017/6/26 下午 5:24
Describe        :清空buffer中的資料
*/
int inUSB_FlushRxBuffer()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
		
	/* 清空接收的buffer */
	usRetVal = CTOS_USBRxFlush();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBRxFlush Error: 0x%04x", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBRxFlush Successs");
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_SUCCESS);
	}
	       
}

/*
Function        :inUSB_Data_Send_Check
Date&Time       :2017/6/26 下午 4:33
Describe        :
*/
int inUSB_Data_Send_Check()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = VS_ERROR;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_USBTxReady();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBTxReady Error: 0x%04x", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBTxReady Successs");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_Data_Send
Date&Time       :2017/6/26 下午 4:38
Describe        :
*/
int inUSB_Data_Send(unsigned char *uszSendBuff, unsigned short usSendSize)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = VS_ERROR;
	
	/* 清空接收的buffer */
	/* Length of data. The maximum value is 2048. */
	usRetVal = CTOS_USBTxData(uszSendBuff, usSendSize);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBTxData Error: 0x%04x", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBTxData Successs");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_Data_Receive_Check
Date&Time       :2017/6/26 下午 5:01
Describe        :
*/
int inUSB_Data_Receive_Check(unsigned short *usReceiveLen)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = VS_ERROR;
	
//        inDISP_LogPrintf("inUSB_Data_Receive_Check Start");
        
	/* 清空接收的buffer */
	usRetVal = CTOS_USBRxReady(usReceiveLen);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBRxReady Error: 0x%04x", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		/* 有收到資料就回傳成功 */
		if (*usReceiveLen > 0)
		{
			return (VS_SUCCESS);
		}
		else
		{
			return (VS_ERROR);
		}
	}
}

/*
Function        :inUSB_Data_Receive
Date&Time       :2017/6/26 下午 5:05
Describe        :
*/
int inUSB_Data_Receive(unsigned char *uszReceBuff, unsigned short *usReceSize)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = VS_ERROR;
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	usRetVal = CTOS_USBRxData(uszReceBuff, usReceSize);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBRxData Error: 0x%04x", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBRxData Successs (%u)", *usReceSize);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_SUCCESS);
	}
}

/*
Function        :inUSB_ECR_Initial
Date&Time       :2017/11/13 下午 2:37
Describe        :
*/
int inUSB_ECR_Initial(void)
{
	int	inRetVal = VS_ERROR;
	
	inRetVal = stUSB_ECRTable[gsrECROb.srSetting.inVersion].inEcrInitial(&gsrECROb);
	
	return (inRetVal);
}

/*
Function        :inUSB_ECR_Receive_Transaction
Date&Time       :2016/6/21 上午 11:42
Describe        :從ECR接收金額及交易別資料
*/
int inUSB_ECR_Receive_Transaction(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;

	/* 清空上次交易的資料 */
	memset(&gsrECROb.srTransData, 0x00, sizeof(ECR_TRANSACTION_DATA));
	/* 收資料 */	
	inRetVal = stUSB_ECRTable[gsrECROb.srSetting.inVersion].inEcrRece(pobTran, &gsrECROb);
	
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inUSB_ECR_Receive_Transaction ok");
		}
		return (VS_SUCCESS);
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inUSB_ECR_Receive_Transaction Timeout");
		}
		
		return (VS_TIMEOUT);
	}
	else if (inRetVal == VS_USER_CANCEL)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inUSB_ECR_Receive_Transaction Cancel");
		}
		
		return (VS_USER_CANCEL);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inUSB_ECR_Receive_Transaction Error");
		}
		
		return (VS_ERROR);
	}
	
        
}

/*
Function        :inUSB_ECR_Send_Transaction_Result
Date&Time       :2016/7/11 下午 3:44
Describe        :印帳單前要送給ECR
*/
int inUSB_ECR_Send_Transaction_Result(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;

	/* 送資料 */	
	inRetVal = stUSB_ECRTable[gsrECROb.srSetting.inVersion].inEcrSend(pobTran, &gsrECROb);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inUSB_ECR_Send_Transaction_Result Error");
		}
		
		
		return (inRetVal);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inUSB_ECR_Send_Transaction_Result ok");
		}
		
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_SendError
Date&Time       :2016/7/18 上午 10:34
Describe        :送錯誤訊息給ECR
*/
int inUSB_ECR_SendError(TRANSACTION_OBJECT * pobTran, int inErrorType)
{
	if (pobTran->uszECRBit != VS_TRUE)
		return (VS_SUCCESS);
	
	gsrECROb.srTransData.inErrorType = inErrorType;
	
	if (stUSB_ECRTable[gsrECROb.srSetting.inVersion].inEcrSendError(pobTran, &gsrECROb) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_DeInitial
Date&Time       :2018/7/26 下午 2:12
Describe        :反初始化
*/
int inUSB_ECR_DeInitial(void)
{	
	if (stUSB_ECRTable[gsrECROb.srSetting.inVersion].inEcrEnd(&gsrECROb) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_NCCC_144_To_400
Date&Time       :2018/7/26 下午 2:15
Describe        :回傳VS_ERROR代表不用轉換
*/
int inUSB_ECR_NCCC_144_To_400(ECR_TABLE * srECROb, unsigned char *uszReceiveBuffer)
{
	char	szCOMPort[4 + 1];
	
	memset(szCOMPort, 0x00, sizeof(szCOMPort));
	inGetECRComPort(szCOMPort);
	/* 非USB，不用繼續判斷 */
	if (memcmp(szCOMPort, "USB1", strlen("USB1")) != 0)
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
			inDISP_ChineseFont("144轉400", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);

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
Function        :inUSB_ECR_Receive
Date&Time       :2016/7/6 下午 4:05
Describe        :接收收銀機傳來的資料
*/
int inUSB_ECR_Receive(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer, int inDataSize)
{
	int		i;
	int		inRetVal;
	int		inRetry = 0;							/* 目前已重試次數 */
	int		inRecMaxRetry = _ECR_USB_RETRYTIMES_;				/* 最大重試次數 */
	char		szDebugMsg[100 + 1];						/* DebugMsg */
	unsigned short	usReceiveBufferSize;						/* uszReceiveBuffer的長度，*/
	unsigned short	usOneSize = 1;							/* 一次只讀一個byte */
	unsigned short	usReceiveLen = 0;
	unsigned char	uszLRC;								/* LRC的值 */
	unsigned char	uszTempBuffer[1024 + 1];					/* 從USB Buffer收到的片段 */
	unsigned char	uszReceiveBuffer[_ECR_USB_BUFF_SIZE_];			/* 包含STX 和 ETX的原始電文 */
	unsigned char	uszSTX = VS_FALSE;						/* 是否收到STX */
	unsigned char	uszETX = VS_FALSE;						/* 是否收到ETX */
	
	/* 設定Timeout */
	srECROb->srSetting.inTimeout = _ECR_USB_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inDISP_Timer_Start(_ECR_USB_RECEIVE_TIMER_, srECROb->srSetting.inTimeout);
	
	/* 初始化放收到資料的陣列 */
	memset(uszReceiveBuffer, 0x00, sizeof(uszReceiveBuffer));
	usReceiveBufferSize = 0;
	
	while(1)
	{
		memset(uszTempBuffer, 0x00, sizeof(uszTempBuffer));
		uszLRC = 0;
		
		/* 超過重試次數 */
		if (inRetry > inRecMaxRetry)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("Retry too many times!");
			}
			
			return (VS_ERROR);
		}
		
		/* 當USB中有東西(也可用有USB事件來判斷)就開始分析 */
		while (usReceiveLen == 0)
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_USB_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			inUSB_Data_Receive_Check(&usReceiveLen);
		}
		
		if (ginDebug == VS_TRUE)
		{
			char szDebugMsg[100 + 1];

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Len : %d", usReceiveLen);
			inDISP_LogPrintf(szDebugMsg);
		}


		/* 如果沒收到STX就會進這隻，直到分析出STX */
		while (uszSTX == VS_FALSE && usReceiveLen > 0)
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_USB_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			/* 這邊一次只收一個byte */
			memset(uszTempBuffer, 0x00, 1);
			usOneSize = 1;
			inRetVal = inUSB_Data_Receive(uszTempBuffer, &usOneSize);
			if (inRetVal == VS_SUCCESS)
			{
				/* buffer讀出一個byte，長度減一 */
				usReceiveLen --;

				if (uszTempBuffer[0] == _STX_)
				{
					/* 收到STX */
					uszSTX = VS_TRUE;
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("Receive STX!");
					}

					memcpy(&uszReceiveBuffer[usReceiveBufferSize], &uszTempBuffer[0], 1);
					usReceiveBufferSize++;
					break;
				}
				else
				{
					/* 沒收到STX */
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("Not Receive STX!");
					}

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						inFunc_BCD_to_ASCII(szDebugMsg, uszTempBuffer, usOneSize);
						inDISP_LogPrintf(szDebugMsg);
					}

					/* 繼續收 */
					continue;
				}

			}

		}

		/* 如果有收到STX，沒收到ETX就會進這隻，直到分析出ETX */			
		while (uszSTX == VS_TRUE && uszETX == VS_FALSE && usReceiveLen > 0)
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_USB_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			usOneSize = 1;
			inRetVal = inUSB_Data_Receive(&uszReceiveBuffer[usReceiveBufferSize], &usOneSize);

			if (inRetVal == VS_SUCCESS)
			{
				/* 若為為7E1協定，且收到STX之後的第一個為"I"，則自動轉為8N1 (NCCC ECR特殊規定) */
				if ((srECROb->srSetting.inVersion == 1)	&&
				    usReceiveBufferSize == 1		&&
				    uszReceiveBuffer[usReceiveBufferSize] == 'I')
				{
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("Receive 'I'!");
					}
					inSetECRVersion("00");
					inSaveEDCRec(0);
					
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("7E1轉8N1", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);
					
					inUSB_ECR_Initial();
					
					return (VS_ERROR);
				}
				
				/* buffer讀出一個byte，長度減一 */
				usReceiveLen --;

				/* 當已收到401byte 確認下個byte是不是ETX */
				if (usReceiveBufferSize == (inDataSize + 3) - 2)
				{
					if (uszReceiveBuffer[usReceiveBufferSize] == _ETX_)
					{	
						/* 是ETX */
						uszETX = VS_TRUE;
						usReceiveBufferSize ++;
						if (ginDebug == VS_TRUE)
						{
							inDISP_LogPrintf("Receive ETX!");
						}

						/* 再收LRC */
						usOneSize = 1;
						inRetVal = inUSB_Data_Receive(&uszReceiveBuffer[usReceiveBufferSize], &usOneSize);
						if (inRetVal == VS_SUCCESS)
						{
							usReceiveLen --;
							usReceiveBufferSize ++;
						}
						else
						{
							/* 沒收到LRC 資料長度有誤*/
						}
						break;
					}
					else
					{
						/* 不是是ETX */
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							inFunc_BCD_to_ASCII(szDebugMsg, uszTempBuffer, usOneSize);
							inDISP_LogPrintf(szDebugMsg);
							inDISP_LogPrintf("Not Receive ETX!");
						}
					}	
				}
				usReceiveBufferSize ++;
			}

		}


		/* 有收到STX、ETX而且收到長度也對，就開始算LRC */
		if (uszSTX == VS_TRUE && uszETX == VS_TRUE)
		{
			/* 資料長度錯誤，清Buffer，送NAK，重收 */
			if (usReceiveBufferSize != inDataSize + 3)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "DataLength error!:%d Retry: %d", usReceiveBufferSize, inRetry);
					inDISP_LogPrintf(szDebugMsg);
				}
				
				/* 回傳NAK */
				inUSB_ECR_Send_ACKorNAK(srECROb, _NAK_);
				/* retry次數+ 1 */
				inRetry ++;
				
				/* 初始化資料 */
				memset(uszReceiveBuffer, 0x00, sizeof(uszReceiveBuffer));
				usReceiveBufferSize = 0;
				uszSTX = VS_FALSE;
				uszETX = VS_FALSE;
				
				continue;
			}
			
			/* 計算收到DATA的LRC （Stx Not Include）*/
			for (i = 1; i <= (usReceiveBufferSize - 2); i++)
			{
				uszLRC ^= uszReceiveBuffer[i];
			}
			
			if (uszReceiveBuffer[usReceiveBufferSize-1] == uszLRC)
			{
				/* 比對收到的LRC是否正確，若正確回傳ACK */
				inUSB_ECR_Send_ACKorNAK(srECROb, _ACK_);
				
				break;
			}
			else
			{
				/* 比對失敗 */
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "LRC error! Retry: %d", inRetry);
					inDISP_LogPrintf(szDebugMsg);
					
					inDISP_LogPrintf("CountLRC");
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%X", uszLRC);
					inDISP_LogPrintf(szDebugMsg);
					
					inDISP_LogPrintf("DataSendLRC");
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(&szDebugMsg[0], "%X", uszReceiveBuffer[usReceiveBufferSize-1]);
					inDISP_LogPrintf(szDebugMsg);
				}
				
				/* 若錯誤回傳NAK */
				inUSB_ECR_Send_ACKorNAK(srECROb, _NAK_);
				/* retry次數+ 1 */
				inRetry ++;
				
				/* 初始化資料 */
				memset(uszReceiveBuffer, 0x00, sizeof(uszReceiveBuffer));
				usReceiveBufferSize = 0;
				uszSTX = VS_FALSE;
				uszETX = VS_FALSE;
				
				continue;
			}
			
		}/* 算LRC END */
		
	}/* Receive END */
	
	/* 去除STX、ETX、LRC，把資料放到szrRealReceBuffer */
	memcpy(szDataBuffer, &uszReceiveBuffer[1], inDataSize);
	
	/* 列印紙本電文和顯示電文訊息 */
	inECR_Print_Receive_ISODeBug(szDataBuffer, usReceiveBufferSize - 3, inDataSize);
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_Send
Date&Time       :2016/7/11 下午 2:25
Describe        :傳送要給收銀機的資料
*/
int inUSB_ECR_Send(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer, int inDataSize)
{
	int		i;
	int		inRetVal;
	int		inRetry = 0;
	int		inRecMaxRetry = _ECR_USB_RETRYTIMES_;					/* 最大重試次數 */
	int		inSendLen = 0;
	char		szDebugMsg[100 +1];
	unsigned char	uszSendBuf[_ECR_USB_BUFF_SIZE_];					/* 包含STX、ETX、LRC的電文 */
	unsigned char	uszLRC = 0;
	
	
	/* Send之前清Buffer，避免收到錯的回應 */
	inUSB_FlushRxBuffer();
	
	/* 在要傳送Buffer裡放STX */
	uszSendBuf[inSendLen] = _STX_;
	inSendLen ++;
	
	/* 把Data Buffer(純資料)裡所有0x00值改成0x20 */
	for (i = 0; i < inDataSize; i++)
	{
		if (szDataBuffer[i] == 0x00)
		{
			szDataBuffer[i] = 0x20;
		}
	}
	
	/* 在要傳送Buffer裡放要傳送的資料 */
	memcpy(&uszSendBuf[inSendLen], szDataBuffer, inDataSize);
	inSendLen += inDataSize;
	
	/* 在要傳送Buffer裡放ETX */
	uszSendBuf[inSendLen] = _ETX_;
	inSendLen ++;
	
	/* 運算LRC(STX Not include) */
	for (i = 1; i < (inSendLen); i++)
	{
		uszLRC ^= uszSendBuf[i];
	}

	/* 在要傳送Buffer裡放LRC */
	uszSendBuf[inSendLen] = uszLRC;
	inSendLen ++;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "LRC : 0x%02X", uszLRC);
		inDISP_LogPrintf(szDebugMsg);
	}
	
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 2);
	
	while (1)
	{
		/* 檢查port是否已經準備好要送資料 */
		while (1)
		{
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			if (inUSB_Data_Send_Check() == VS_SUCCESS)
			{
				break;
			}
		};

		/* 經由port傳送資料 */
		inRetVal = inUSB_Data_Send(uszSendBuf, (unsigned short)inSendLen);
                
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		else
		{
			
		/*---------------------------------- 印Debug 開始---------------------------------------------------------------*/
			/* 列印紙本電文和顯示電文訊息 */
			inECR_Print_Send_ISODeBug(szDataBuffer, inSendLen, inDataSize);
		/*---------------------------------- 印Debug 結束---------------------------------------------------------------*/
			/* 傳送Retry */
			if (inRetry < inRecMaxRetry)
			{
				/* 接收ACK OR NAK */
				inRetVal = inUSB_ECR_Receive_ACKandNAK(srECROb);

				/* 超過一秒沒收到回應 */
				if (inRetVal == VS_TIMEOUT)
				{
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("Not Receive Response, Retry");
					}
					inRetry++;
					continue;
				}
				/* 收到NAK */
				else if (inRetVal == _NAK_)
				{
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("Receive NAK, Retry");
					}
					inRetry++;
					continue;
				}
				/* 收到ACK */	
				else
				{	
					/* 成功 */
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("ECR ACK OK!");
					}
					return (VS_SUCCESS);
				}
			}
			/* 超過最大重試次數，仍要完成交易，收銀機提示補登畫面 */
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf("Exceed max retry times!");
				}
				return (VS_SUCCESS);
			}

		}/* inUSB_Send */
	
	}/* while(1) */
	
}

/*
Function        :inUSB_ECR_Send_ACKorNAK
Date&Time       :2016/6/21 下午 5:36
Describe        :送ACK OR NAK
*/
int inUSB_ECR_Send_ACKorNAK (ECR_TABLE * srECROb, int inAckorNak)
{
	unsigned char	uszSendBuffer[2 + 1];
	unsigned short	usRetVal;
    	
	memset(uszSendBuffer, 0x00, sizeof(uszSendBuffer));
	
	/* 檢查port是否已經準備好要送資料 */
        while (inUSB_Data_Send_Check() != VS_SUCCESS)
	{
		/* 等TxReady*/
	};
	
	if (inAckorNak == _ACK_)
	{	    
		/* 成功，回傳ACK */
		uszSendBuffer[0] = _ACK_;
		uszSendBuffer[1] = _ACK_;
		
		usRetVal = inUSB_Data_Send(uszSendBuffer, 2);
		
		if (usRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("Send ACK_ACK Not OK");
			}

			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("ACK_ACK");
			}
			
		}
        
	}	
	else if (inAckorNak == _NAK_)
	{
		/* 失敗，回傳NAK */
		uszSendBuffer[0] = _NAK_;
		uszSendBuffer[1] = _NAK_;
		
		usRetVal = inUSB_Data_Send(uszSendBuffer, 2);
		
		if (usRetVal != VS_SUCCESS)
		{		
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("Send NAK NAK Not OK");
			}
			
			return (VS_ERROR);
		}
		else
		{	
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("NAK_NAK");
			}
			
		}
	
	}
	else
	{
		/* 傳入錯誤的參數 */
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("Can't Send Neither ACK or NAK");
		}
		
		return (VS_ERROR);
	}
        
        return (VS_SUCCESS);
}

int inUSB_ECR_Receive_ACKandNAK(ECR_TABLE * srECROb)
{
	int		inRetVal;
	char		szDebugMsg[100 + 1];
	unsigned char	uszBuf[_ECR_USB_BUFF_SIZE_];
	unsigned short	usTwoSize = 2;
	unsigned short	usReceiveLen = 0;
		
	/* 設定Timeout */
	inRetVal = inDISP_Timer_Start(_TIMER_NEXSYS_2_, _ECR_RECEIVE_ACK_TIMEOUT_);
	
	while (1)
	{
		memset(uszBuf, 0x00, sizeof(uszBuf));
		/* 當USB中有東西(也可用有USB事件來判斷)就開始分析 */
		while (usReceiveLen == 0)
		{
			inUSB_Data_Receive_Check(&usReceiveLen);
			
			/* 如果timeout就跳出去 */
			if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
		}
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Len : %d", usReceiveLen);
			inDISP_LogPrintf(szDebugMsg);
		}
                
		while (usReceiveLen > 0)
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(1) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			/* 這邊一次只收兩個byte */
			usTwoSize = 2;
			inRetVal = inUSB_Data_Receive(uszBuf, &usTwoSize);
                        
			if (inRetVal == VS_SUCCESS)
			{
				/* buffer讀出兩個byte，長度減二 */
				usReceiveLen -= 2;
				
				/* 判斷收到資料是否為ACK */	
				if (uszBuf[0] == _ACK_ && uszBuf[1] == _ACK_)
				{ 
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("Receive ACK!");
					}
					
					return (_ACK_);				
				}
				/* 判斷收到資料是否為NAK */
				else if (uszBuf[0] == _NAK_ && uszBuf[1] == _NAK_)
				{	
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("Receive NAK!");
					}
					
					return (_NAK_);
				}
				else 
				{
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("Receive Not Ack Neither NAK!");
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						inFunc_BCD_to_ASCII(szDebugMsg, uszBuf, usTwoSize);
						inDISP_LogPrintf(szDebugMsg);
					}

					continue;
				}

			}/* inUSB_Receive */
			
		}/* while (usReceiveLen > 0) (有收到資料) */
			
	}/* while(1)...... */
		
}

/*
Function        :inUSB_ECR_SelectTransType
Date&Time       :2016/6/22 上午 9:47
Describe        :Indicator為'S'時的交易選單
*/
int inUSB_ECR_SelectTransType(ECR_TABLE * srECROb)
{
	int	inTransType = 0;
	int	inRetVal = VS_SUCCESS;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char	szTemplate[10 + 1];
	char	szDebugMsg[100 + 1];
	char	szKey = 0x00;
		
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
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */
			inDISP_PutGraphic(_MENU_SALE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
			
			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
                        while (1)
                        {
                                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				szKey = uszKBD_Key();
                                
				/* 回傳Timeout */
				if (inTimerGet(_ECR_USB_RECEIVE_TIMER_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}
				
				/* 一般交易 */
                                if (szKey == _KEY_1_			||
				    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_SALE_, 2);
					inRetVal = VS_SUCCESS;
					break;
				}
				/* 紅利扣抵 */
                                else if (szKey == _KEY_2_			||
					 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_REDEEM_, 2);
					inRetVal = VS_SUCCESS;
					break;
				}
				/* 分期付款 */
                                else if (szKey == _KEY_3_			||
					 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_INSTALLMENT_, 2);
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
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜交易補登＞ */
			inDISP_PutGraphic(_MENU_SALE_OFFLINE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);

			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
                        while (1)
                        {
                                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				szKey = uszKBD_Key();
                                
				/* 回傳Timeout */
				if (inTimerGet(_ECR_USB_RECEIVE_TIMER_) == VS_SUCCESS)
				{
					return (VS_TIMEOUT);
				}
				
				/* 一般補登 */
                                if (szKey == _KEY_1_			||
				    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_OFFLINE_, 2);
					inRetVal = VS_SUCCESS;
					break;
                                }
				/* 紅利調帳 */
                                else if (szKey == _KEY_2_			||
					 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_REDEEM_ADJUST_, 2);
					inRetVal = VS_SUCCESS;
					break;
                                }
				/* 分期調帳 */
                                else if (szKey == _KEY_3_			||
					 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)
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
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */
			inDISP_PutGraphic(_MENU_REFUND_OPTION_2_, 0, _COORDINATE_Y_LINE_8_4_);

			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
                        while (1)
                        {
                                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				szKey = uszKBD_Key();
                                
				/* 回傳Timeout */
				if (inTimerGet(_ECR_USB_RECEIVE_TIMER_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}
				
				/* 一般退貨 */
                                if (szKey == _KEY_1_			||
				    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_REFUND_, 2);
					inRetVal = VS_SUCCESS;
					break;
                                }
				/* 紅利退貨 */
                                else if (szKey == _KEY_2_			||
					 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_REDEEM_REFUND_, 2);
					inRetVal = VS_SUCCESS;
					break;
                                }
				/* 分期退貨 */
                                else if (szKey == _KEY_3_			||
					 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)
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
Function        :inUSB_ECR_8N1_Standard_Initial
Date&Time       :2016/6/20 下午 2:55
Describe        :initial 8N1 COM PORT
*/
int inUSB_ECR_8N1_Standard_Initial(ECR_TABLE *srECROb)
{
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CUP_LOGON_;
	char		szKey = 0x00;
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = 0x00;
			
        /* 開port */
	/* Portable 機型若沒接上底座再開Ethernet會失敗 */
	if (inFunc_Is_Portable_Type() == VS_TRUE)
	{
		do
		{
			usRetVal = inUSB_Open();
        
			if (usRetVal == VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf("inUSB_Open OK");
				}
				break;
				
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "inUSB_Open Error: 0x%04x", usRetVal);
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
		usRetVal = inUSB_Open();
        
		if (usRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "inUSB_Open Error: 0x%04x", usRetVal);
				inDISP_LogPrintf(szDebugMsg);
			}
			return (VS_ERROR);         
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("inUSB_Open OK");
			}
		}
		
	}
        
	/* 清空接收的buffer */
        inUSB_FlushRxBuffer();
	
        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_8N1_Standard_Receive_Packet
Date&Time       :2016/7/6 下午 4:04
Describe        :處理收銀機傳來的資料
*/
int inUSB_ECR_8N1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_USB_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	
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
		inUSB_ECR_SendError(pobTran, inRetVal);
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_8N1_Standard_Send_Packet
Date&Time       :2016/7/11 下午 3:29
Describe        :處理要送給收銀機的資料
*/
int inUSB_ECR_8N1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_USB_BUFF_SIZE_] = {0};	/* 封包資料 */
	
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
Function        :inUSB_ECR_8N1_Standard_Send_Error
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
*/
int inUSB_ECR_8N1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_USB_BUFF_SIZE_];	/* 封包資料 */
	
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
Function        :inUSB_ECR_8N1_Standard_Close
Date&Time       :2016/7/11 下午 3:34
Describe        :關閉Comport
*/
int inUSB_ECR_8N1_Standard_Close(ECR_TABLE* srECRob)
{
        /*關閉port*/
        if (inUSB_Close() != VS_TRUE)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_7E1_Standard_Initial
Date&Time       :2016/6/20 下午 2:55
Describe        :initial 7E1 COM PORT
*/
int inUSB_ECR_7E1_Standard_Initial(ECR_TABLE *srECROb)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = 0x00;
	
        /* 開port */
        usRetVal = inUSB_Open();
        
        if (usRetVal != d_OK)
        {
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inUSB_Open Error: 0x%04x", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
                return (VS_ERROR);         
        }
        else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inUSB_Open OK");
		}
	}
		
	/* 清空接收的buffer */
        inUSB_FlushRxBuffer();
	
        return (VS_SUCCESS);
}		

/*
Function        :inUSB_ECR_7E1_Standard_Receive_Packet
Date&Time       :2016/7/6 下午 4:04
Describe        :處理收銀機傳來的資料
*/
int inUSB_ECR_7E1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_USB_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
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
		inUSB_ECR_SendError(pobTran, inRetVal);
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_7E1_Standard_Send_Packet
Date&Time       :2016/7/11 下午 3:29
Describe        :處理要送給收銀機的資料
*/
int inUSB_ECR_7E1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_USB_BUFF_SIZE_] = {0};	/* 封包資料 */
	
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
	inRetVal = inUSB_ECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_7E1_Standard_Send_Error
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
*/
int inUSB_ECR_7E1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_USB_BUFF_SIZE_];	/* 封包資料 */
	
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
	inRetVal = inUSB_ECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponce = VS_TRUE;
	
        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_7E1_Standard_Close
Date&Time       :2016/7/11 下午 3:34
Describe        :關閉Comport
*/
int inUSB_ECR_7E1_Standard_Close(ECR_TABLE* srECROb)
{
        /*關閉port*/
        if (inUSB_Close() != VS_TRUE)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inUSB_SetCDCMode
Date&Time       :2017/6/26 下午 5:47
Describe        :
*/
int inUSB_SetCDCMode()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_USBSetCDCMode();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBSetCDCMode Error: 0x%04x", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBSetCDCMode Successs");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_SetSTDMode
Date&Time       :2017/6/26 下午 5:47
Describe        :
*/
int inUSB_SetSTDMode()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_USBSetSTDMode();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inUSB_SetSTDMode Error: 0x%04x", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inUSB_SetSTDMode Successs");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_SetVidPid
Date&Time       :2017/6/26 下午 5:58
Describe        :
*/
int inUSB_SetVidPid(unsigned int uiVidPid)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_USBSetVidPid(uiVidPid);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBSetVidPid Error: 0x%04x", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBSetVidPid Successs");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_GetVidPid
Date&Time       :2017/6/26 下午 5:59
Describe        :
*/
int inUSB_GetVidPid(unsigned int *uiVidPid)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_USBGetVidPid(uiVidPid);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBGetVidPid Error: 0x%04x", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBGetVidPid Successs");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_HostOpen
Date&Time       :2017/6/26 下午 6:14
Describe        :
*/
int inUSB_HostOpen(unsigned short usVendorID, unsigned short usProductID)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_USBHostOpen(usVendorID, usProductID);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBHostOpen Error: 0x%04x", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBHostOpen Successs");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_HostClose
Date&Time       :2017/6/26 下午 6:14
Describe        :
*/
int inUSB_HostClose()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_USBHostClose();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBHostClose Error: 0x%04x", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBHostClose Successs");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :CTOS_USBHostTxData
Date&Time       :2017/6/26 下午 6:14
Describe        :
*/
int inUSB_HostSend(unsigned char *uszSendBuffer, unsigned long ulSendLen, unsigned long ulTransTimeout)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_USBHostTxData(uszSendBuffer, ulSendLen, ulTransTimeout);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBHostTxData Error: 0x%04x", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBHostTxData Successs");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_HostReceive
Date&Time       :2017/6/26 下午 6:14
Describe        :
*/
int inUSB_HostReceive(unsigned char *uszReceiveBuffer, unsigned long *ulReceiveLen, unsigned long ulTransTimeout)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_USBHostRxData(uszReceiveBuffer, ulReceiveLen, ulTransTimeout);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBHostRxData Error: 0x%04x", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBHostRxData Successs");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}


/*
Function        :inUSB_SelectMode
Date&Time       :2019/2/18 下午 1:58
Describe        :設定為Host Mode或device Mode
*/
int inUSB_SelectMode(int inMode)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_USBSelectMode(inMode);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBSelectMode Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBSelectMode() OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_GetMode
Date&Time       :2019/3/12 上午 10:19
Describe        :設定為Host Mode或device Mode
*/
int inUSB_GetStatus(unsigned int *uiStatus)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_USBGetStatus(uiStatus);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBGetStatus Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBGetStatus() OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_Get_Host_Device_Mode
Date&Time       :2019/3/12 上午 11:02
Describe        :
*/
int inUSB_Get_Host_Device_Mode(int *inMode)
{
	int		inRetVal = VS_ERROR;
	unsigned int	uiStatus = 0;
	
	inRetVal = inUSB_GetStatus(&uiStatus);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	else
	{
		/* 代表為Host Mode 反之，device Mode */
		if ((uiStatus & d_MK_USB_STATUS_HOSTMODE) == d_MK_USB_STATUS_HOSTMODE)
		{
			*inMode = d_USB_HOST_MODE;
		}
		else
		{
			*inMode = d_USB_DEVICE_MODE;
		}
		
		return (VS_SUCCESS);
	}
}


/*
Function        :inUSB_ECR_8N1_Customer_107_Bumper_Initial
Date&Time       :2018/7/23 上午 11:52
Describe        :initial 8N1 COM PORT
*/
int inUSB_ECR_8N1_Customer_107_Bumper_Initial(ECR_TABLE *srECROb)
{
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CUP_LOGON_;
	char		szKey = 0x00;
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = 0x00;
	
        /* 開port */
	/* Portable 機型若沒接上底座再開Ethernet會失敗 */
	if (inFunc_Is_Portable_Type() == VS_TRUE)
	{
		do
		{
			usRetVal = inUSB_Open();
        
			if (usRetVal == VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf("inUSB_Open OK");
				}
				break;
				
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "inUSB_Open Error: 0x%04x", usRetVal);
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
		usRetVal = inUSB_Open();
        
		if (usRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "inUSB_Open Error: 0x%04x", usRetVal);
				inDISP_LogPrintf(szDebugMsg);
			}
			return (VS_ERROR);         
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("inUSB_Open OK");
			}
		}
		
	}
        
	/* 清空接收的buffer */
        inUSB_FlushRxBuffer();
	
        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_8N1_Customer_107_Bumper_Receive_Packet
Date&Time       :2016/7/6 下午 4:04
Describe        :處理收銀機傳來的資料
*/
int inUSB_ECR_8N1_Customer_107_Bumper_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_USB_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	
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
		inUSB_ECR_SendError(pobTran, inRetVal);
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_8N1_Customer_107_Bumper_Send_Packet
Date&Time       :2016/7/11 下午 3:29
Describe        :處理要送給收銀機的資料
*/
int inUSB_ECR_8N1_Customer_107_Bumper_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_USB_BUFF_SIZE_] = {0};	/* 封包資料 */
	
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
Function        :inUSB_ECR_8N1_Customer_107_Bumper_Send_Error
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
*/
int inUSB_ECR_8N1_Customer_107_Bumper_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_USB_BUFF_SIZE_];	/* 封包資料 */
	
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
Function        :inUSB_ECR_8N1_Customer_107_Bumper_Close
Date&Time       :2016/7/11 下午 3:34
Describe        :關閉Comport
*/
int inUSB_ECR_8N1_Customer_107_Bumper_Close(ECR_TABLE* srECRob)
{
        /*關閉port*/
        if (inUSB_Close() != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/* 20230413 Miyano add 新方式的收送 */
/* 20230413 Miyano 開USB Port */
int OpenUSBPort(int USB_MODE)
{
    int         inRetVal;
    struct      termios new, old;
    errno = 0;

    if(USB_MODE == USB_Host)
        ginHandleUSBPort = open("/dev/ttyACM0", O_RDWR | O_NONBLOCK);
    else if(USB_MODE == USB_Dev)
        ginHandleUSBPort = open("/dev/ttyGS", O_RDWR | O_NONBLOCK);
    
    if(ginHandleUSBPort > 0)
    {
            inRetVal = VS_SUCCESS;
    }
    else
    {
            inRetVal = VS_ERROR;
    }
    
    /* Try */
    inRetVal = tcgetattr(ginHandleUSBPort, &old);
    if (inRetVal == -1 || errno)
    {            
        close(ginHandleUSBPort);
        gstMultiOb.srSetting.uszSettingOK = VS_FAILURE;             
        return VS_ERROR;
    } 
    cfmakeraw(&new);          
    inRetVal = tcsetattr(ginHandleUSBPort, TCSANOW, &new);
    if (inRetVal == -1 || errno)
    {            
        close(ginHandleUSBPort);
        gstMultiOb.srSetting.uszSettingOK = VS_FAILURE;             
        return VS_ERROR;
    } 

    return      inRetVal;
}

int inUSB_Host_Send(unsigned char *uszSendBuff, unsigned short *usSendSize)
{
        struct termios new, old;
        int inRetVal = VS_SUCCESS;
        errno = 0;

        /* Try 送之前先關Port 再開 */
        {
            close(ginHandleUSBPort);
            OpenUSBPort(USB_Host);
        }
        
        inRetVal = write(ginHandleUSBPort, uszSendBuff, usSendSize);
        
        if (inRetVal < 0 || errno)
        {               
            if(ginDebug == VS_TRUE)
            {
                inDISP_LogPrintfAt(AT,"Send ERROR");
            }
            return VS_ERROR;                
        }

        return VS_SUCCESS;
}

int inUSB_Host_Read(unsigned char *uszReadBuff, unsigned short *usReadSize)
{
        struct termios new, old;
        int inRetVal = VS_SUCCESS;
        errno = 0;

        /* Try */
        inRetVal = tcgetattr(ginHandleUSBPort, &old);

        if (inRetVal == -1 || errno)
        {            
            close(ginHandleUSBPort);            
            return VS_ERROR;
        } 
        
	cfmakeraw(&new);

	inRetVal = tcsetattr(ginHandleUSBPort, TCSANOW, &new);

        if (inRetVal == -1 || errno)
        {            
            close(ginHandleUSBPort);            
            return VS_ERROR;
        }

        memset((char *) uszReadBuff, 0x30, (int* )usReadSize);
        inRetVal = read(ginHandleUSBPort, uszReadBuff, usReadSize);

        if (inRetVal < 0 || errno)
        {               
            if(ginDebug == VS_TRUE)
            {
                    
            }
            return VS_ERROR;                
        }

        inRetVal = tcsetattr(ginHandleUSBPort, TCSANOW, &old);
        
        return VS_SUCCESS;
}

