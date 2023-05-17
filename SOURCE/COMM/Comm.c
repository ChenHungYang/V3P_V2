#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"
#include "../DISPLAY/DispMsg.h"
#include "../PRINT/Print.h"
#include "../EVENT/MenuMsg.h"
#include "../FUNCTION/Function.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/CPT.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/ECR_FUNC/ECR.h"

#include "../TMS/TMSTABLE/TmsCPT.h"
#include "../TMS/TMSTABLE/TmsFTP.h"

#include "../FUNCTION/UNIT_FUNC/TimeUint.h"

#include "../FUNCTION/FILE_FUNC/File.h"
#include "../FUNCTION/FILE_FUNC/FIleLogFunc.h"
#include "../FUNCTION/COSTCO_FUNC/Costco.h"

#include "Comm.h"
#include "Ethernet.h"
#include "Modem.h"
#include "WiFi.h"
#include "GPRS.h"

COMM_OBJECT     gsrobCommunication;
extern  int     ginDebug;  /* Debug使用 extern */
extern	char	gszTermVersionID[16 + 1];

/* File Descripter */
int	ginECR_ServerFd = -1;	/* 被動接收資料用這個handle，目前只有MP200的ECR用到 */
int	ginECR_ResponseFd = -1;	/* 回覆ECR用Handle */
int	ginTrans_ClientFd = -1;	/* 主動送或主動接收使用(Ex:送電文使用) */
/*
Function        :inCOMM_InitCommDevice
Date&Time       :
Describe        :Get Communication Mode 並將function pointer設定到對應通訊模式的fuction
*/
int inCOMM_InitCommDevice()
{
        char    szCommmode[1 + 1] = {0};
	char	szDialBackupEnable[2 + 1] = {0};

        /* inCOMM_InitCommDevice() START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inCOMM_InitCommDevice() START！");
        }

	memset(szDialBackupEnable, 0x00, sizeof(szDialBackupEnable));
	inGetDialBackupEnable(szDialBackupEnable);
	if (memcmp(szDialBackupEnable, "Y", strlen("Y")) == 0)
	{
		inSetCommMode(_COMM_ETHERNET_MODE_);
		inSaveCFGTRec(0);
	}
	inDISP_LogPrintfWithFlag(" CFGF DialBack[%s]", szDialBackupEnable);
        memset(szCommmode,0x00,sizeof(szCommmode));
        /* 取得通訊模式 */
        if (inGetCommMode(szCommmode) == VS_ERROR)
        {
                return (VS_ERROR);
        }

	inDISP_LogPrintfWithFlag(" CFGF szCommmode[%s]", szCommmode);	
		
        /* Modem Mode */
        if (memcmp(szCommmode, _COMM_MODEM_MODE_, 1) == 0)
        {
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("MODEM");
		}

		inCOMM_MODEM_SetFuncIndex();
		if (inCOMM_DoInitial() != VS_SUCCESS)
                {
			/* 聯合沒定義初始化錯誤的流程，只能用Log紀錄 */
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("MODEM Initial Error");
			}

                        return (VS_ERROR);
                }
        }
        /* 乙太網路 Mode */
        else if (memcmp(szCommmode, _COMM_ETHERNET_MODE_, 1) == 0)
        {
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("ETHERNET");
		}

		inCOMM_ETHERNET_SetFuncIndex();
                if (inCOMM_DoInitial() != VS_SUCCESS)
                {
			/* 聯合沒定義初始化錯誤的流程，只能用Log紀錄 */
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("ETHERNET Initial Error");
			}

                        return (VS_ERROR);
                }
        }
	else if (memcmp(szCommmode, _COMM_GPRS_MODE_, 1) == 0)
        {
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("GPRS");
		}

		inCOMM_GPRS_SetFuncIndex();
		if (inCOMM_DoInitial() != VS_SUCCESS)
                {
			/* 聯合沒定義初始化錯誤的流程，只能用Log紀錄 */
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("GPRS Initial Error");
			}

                        return (VS_ERROR);
                }

        }
	else if (memcmp(szCommmode, _COMM_WIFI_MODE_, 1) == 0)
        {
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("WiFi");
		}

		inCOMM_WiFi_SetFuncIndex();
		if (inCOMM_DoInitial() != VS_SUCCESS)
                {
			/* 聯合沒定義初始化錯誤的流程，只能用Log紀錄 */
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("WiFi Initial Error");
			}

                        return (VS_ERROR);
                }
        }

        /* inCOMM_InitCommDevice()_END */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inCOMM_InitCommDevice()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inCOMM_ConnectStart
Date&Time       :
Describe        :執行Begin 和Check，若已連線（uszConnectionBit == VS_TRUE）則直接回傳成功
*/
int inCOMM_ConnectStart(TRANSACTION_OBJECT *pobTran)
{
        char    szCommmode[1 + 1] = {0};
        char	szDebugMsg[100 + 1] = {0};
	char	szDialBackupEnable[2 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};

#ifndef _COMMUNICATION_CAPBILITY_
	return (VS_SUCCESS);
#endif

        /* inCOMM_ConnectStart() START */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inCOMM_ConnectStart() START!");

	/* 表示已連線，不用重新連線 */
        if (pobTran->uszConnectionBit == VS_TRUE)
        {
                if (ginDebug == VS_TRUE)
                        inDISP_LogPrintf("pobTran->uszConnectionBit == VS_TRUE");
                
                return (VS_SUCCESS);
        }

	/* 連線中‧‧‧‧‧ */
	if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
	{
		/* 不顯示訊息 */
	}
	else
	{
#ifdef _CONNECT_DISP_MODIFY		
		inDISP_PutGraphic(_CONNECTING_, 0, _COORDINATE_Y_LINE_8_7_);
#else
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		inDISP_PutGraphic(_CONNECTING_, 0, _COORDINATE_Y_LINE_8_7_);
#endif
	}

	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		memset(szCommmode, 0x00, sizeof(szCommmode));
		/* 取得通訊模式 */
		if (inGetCommMode(szCommmode) == VS_ERROR)
		{
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("inGetCommMode(szCommmode) == VS_ERROR");

			return (VS_ERROR);
		}

		/* Modem Mode */
		if (memcmp(szCommmode, _COMM_MODEM_MODE_, 1) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("MODEM MODE");
			}

			if (gsrobCommunication.inBegin != NULL)
			{
				if (gsrobCommunication.inBegin(pobTran) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
						inDISP_LogPrintf("_COMM_MODEM_MODE_ .inBegin Error");

					return (VS_ERROR);
				}
			}
			else
			{
				return (VS_ERROR);
			}


			if (gsrobCommunication.inCheck != NULL)
			{
				if (gsrobCommunication.inCheck() != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
						inDISP_LogPrintf("_COMM_MODEM_MODE_ .inCheck Error");

					return (VS_ERROR);
				}
			}
			else
			{
				return (VS_ERROR);
			}
		}
		/* 乙太網路 Mode */
		else if (memcmp(szCommmode, _COMM_ETHERNET_MODE_, 1) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("ETHERNET MODE");
			}

			if (gsrobCommunication.inBegin != NULL)
			{
				if (gsrobCommunication.inBegin(pobTran) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
						inDISP_LogPrintf("_COMM_ETHERNET_MODE_ .inBegin Error");

					return (VS_ERROR);
				}
			}
			else
			{
				return (VS_ERROR);
			}

			if (gsrobCommunication.inCheck != NULL)
			{
				if (gsrobCommunication.inCheck() != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
						inDISP_LogPrintf("_COMM_ETHERNET_MODE_ inCheck Error");

					/* 如果有DialBackup，從這裡開始撥 */
					memset(szDialBackupEnable, 0x00, sizeof(szDialBackupEnable));
					inGetDialBackupEnable(szDialBackupEnable);
					if (memcmp(szDialBackupEnable, "Y", strlen("Y")) == 0)
					{
						if (inCOMM_MODEM_DialBackUpOn(pobTran) != VS_SUCCESS)
						{
							/* 如果有撥接備援，在這裡回復Ethernet */
							inCOMM_MODEM_DialBackUpOff(pobTran);

							return (VS_ERROR);
						}
					}
					else
					{
						return (VS_ERROR);
					}
				}
			}
			else
			{
				return (VS_ERROR);
			}
		}
		else if (memcmp(szCommmode, _COMM_GPRS_MODE_, 1) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("GPRS");
			}

			if (gsrobCommunication.inBegin != NULL)
			{
				if (gsrobCommunication.inBegin(pobTran) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
						inDISP_LogPrintf("_COMM_GPRS_MODE_ .inBegin Error");

					return (VS_ERROR);
				}
			}
			else
			{
				return (VS_ERROR);
			}

			if (gsrobCommunication.inCheck != NULL)
			{
				if (gsrobCommunication.inCheck() != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("_COMM_GPRS_MODE_ .inCheck Error");
					}

					return (VS_ERROR);
				}
			}
			else
			{
				return (VS_ERROR);
			}

		}
		else if (memcmp(szCommmode, _COMM_WIFI_MODE_, 1) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("WiFi");
			}

			if (gsrobCommunication.inBegin != NULL)
			{
				if (gsrobCommunication.inBegin(pobTran) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
						inDISP_LogPrintf("_COMM_WIFI_MODE_ .inBegin Error");

					return (VS_ERROR);
				}
			}
			else
			{
				return (VS_ERROR);
			}

			if (gsrobCommunication.inCheck != NULL)
			{
				if (gsrobCommunication.inCheck() != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("_COMM_WIFI_MODE_ .inCheck Error");
					}

					return (VS_ERROR);
				}
			}
			else
			{
				return (VS_ERROR);
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "COMM_MODE : %s", szCommmode);
				inDISP_LogPrintf(szDebugMsg);
			}

		}
		/* 表示已連線 */
		pobTran->uszConnectionBit = VS_TRUE;

		/* inCOMM_ConnectStart() END! */
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("inCOMM_ConnectStart() END!");

		return (VS_SUCCESS);
	}
}

/*
Function        :inCOMM_Send
Date&Time       :2016/10/4 下午 4:12
Describe        :
*/
int inCOMM_Send(unsigned char *uszSendBuff, int inSendSize, int inSendTimeout , unsigned char uszDispMsgBit)
{
	int	inRetVal;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	if (uszDispMsgBit == VS_TRUE)
	{
/* 把清圖的步驟拿掉  2019/12/5 上午 10:04 */
//		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		inDISP_PutGraphic(_SEND_, 0, _COORDINATE_Y_LINE_8_7_);/* 傳送中... */
	}

	if (gsrobCommunication.inSend != NULL)
	{
		inRetVal = gsrobCommunication.inSend(uszSendBuff, inSendSize, inSendTimeout);
		if (inRetVal != VS_SUCCESS){
                            inDISP_DispLogAndWriteFlie(" gsrobCommunication Send *Error* inRetVal[%d] TimeOut[%d] Line[%d] ", inRetVal, inSendTimeout,   __LINE__);
                     	return (inRetVal);
			
		}
	}
	else
	{
                   inDISP_DispLogAndWriteFlie(" gsrobCommunication Send *Error* inSend Is NULL Line[%d]",  __LINE__);
		return (VS_ERROR);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
Function        :inCOMM_Receive
Date&Time       :2016/10/4 下午 4:12
Describe        :
*/
int inCOMM_Receive(unsigned char *uszReceiveBuff, int inReceiveSize, int inReceiveTimeout, unsigned char uszDispMsgBit)
{
	int	inReceiveCnt;
	
	/* inCOMM_Receive() START */
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "COMM_462 COMM_RECEIVE INIT");
	
	if (uszDispMsgBit == VS_TRUE)
	{
/* 把清圖的步驟拿掉  2019/12/5 上午 10:04 */
//		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		inDISP_PutGraphic(_RECEIVE_, 0, _COORDINATE_Y_LINE_8_7_);/* 接收中... */
	}

	if (gsrobCommunication.inReceive != NULL)
	{
		inReceiveCnt = gsrobCommunication.inReceive(uszReceiveBuff, inReceiveSize, inReceiveTimeout);
		/* 如果收到的資料數小於等於0(Timeout會回傳小於0)*/
		if (inReceiveCnt < 0)
		{
                             inDISP_DispLogAndWriteFlie(" gsrobCommunication Rece *Error* inReceiveCnt[%d]  TimOut[%d] Line[%d] ", inReceiveCnt, inReceiveTimeout,  __LINE__);
			return (inReceiveCnt);
		}else if (inReceiveCnt == 0)
		{        
                             inDISP_DispLogAndWriteFlie(" gsrobCommunication Rece *Error* inReceiveCnt[%d]  TimOut[%d] Line[%d] ", inReceiveCnt, inReceiveTimeout,  __LINE__);
			return (VS_ERROR);
		}
	}
	else
	{
		inDISP_DispLogAndWriteFlie(" gsrobCommunication Rece *Error* Is NULL Line[%d] ",  __LINE__);
		return (VS_ERROR);
	}

        /* inCOMM_Receive() END */

	inDISP_LogPrintfArea(TRUE,(char *)"Recv Data:",10,uszReceiveBuff,inReceiveCnt);
	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "COMM_462 inCOMM_Receive END");
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
        return (VS_SUCCESS);
}

/*
Function        :inCOMM_End
Date&Time       :
Describe        :執行inEND完後，將uszConnectionBit設定成VS_ERROR
*/
int inCOMM_End(TRANSACTION_OBJECT *pobTran)
{
#ifndef _COMMUNICATION_CAPBILITY_
	return (VS_SUCCESS);
#endif
        /* inCOMM_Receive() START */

        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inCOMM_End() START!");

        if (gsrobCommunication.inEnd != NULL)
                gsrobCommunication.inEnd();
        else
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                        inDISP_LogPrintf(" gsrobCommunication.inEnd == NULL !! ");
                return (VS_ERROR);
        }

        /* 表示不是連線中的狀態 */
        pobTran->uszConnectionBit = VS_FALSE;

        /* inCOMM_End() END */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inCOMM_End() END!");

        return (VS_SUCCESS);
}

/*
Function        :inCOMM_MODEM_Mode_Change
Date&Time       :2017/3/30 上午 9:34
Describe        :
*/
int inCOMM_MODEM_Mode_Change(TRANSACTION_OBJECT *pobTran)
{
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inCOMM_MODEM_Mode_Change() START !");
	}

	if (pobTran->uszDialBackup == VS_TRUE)
	{
		inCOMM_MODEM_DialBackUpOff(pobTran);
	}


	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCOMM_MODEM_Mode_Change() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inMODEM_DialBackUpOn
Date&Time       :2017/3/29 下午 5:23
Describe        :
*/
int inCOMM_MODEM_DialBackUpOn(TRANSACTION_OBJECT *pobTran)
{
	char	szDebugMsg[100 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inCOMM_MODEM_DialBackUpOn() START !");
	}

	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inCOMM_MODEM_DialBackUpOn() END !");
			inDISP_LogPrintf("----------------------------------------");
		}

		return (VS_SUCCESS);
	}
	else
	{

		/* 先預設沒撥接備援成功 */
		pobTran->uszDialBackup = VS_FALSE;


		/* 切換通訊模式為【撥接】 */
		if (inLoadCFGTRec(0) != VS_SUCCESS)
		{
			return (VS_ERROR);		/* 共用參數檔【Config.txt】 */
		}

		inSetCommMode(_COMM_MODEM_MODE_);

		if (inSaveCFGTRec(0) != VS_SUCCESS)
		{
			return (VS_ERROR);		/* 共用參數檔【Config.txt】 */
		}

		inCOMM_MODEM_SetFuncIndex();
		if (inCOMM_DoInitial() != VS_SUCCESS)
		{
			inDISP_Msg_BMP(_ERR_INIT_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "MODEM", _LINE_8_5_);

			return (VS_ERROR);
		}

		pobTran->uszDialBackup = VS_TRUE;

		if (gsrobCommunication.inBegin(pobTran) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "DialBackup Begin Fail");
				inDISP_LogPrintf(szDebugMsg);
			}
			return (VS_COMM_ERROR);
		}

		if (gsrobCommunication.inCheck() != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "DialBackup Check Fail");
				inDISP_LogPrintf(szDebugMsg);
			}
			return (VS_COMM_ERROR);
		}

		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inCOMM_MODEM_DialBackUpOn() END !");
			inDISP_LogPrintf("----------------------------------------");
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inCOMM_MODEM_DialBackUpOff
Date&Time       :2017/3/29 下午 6:02
Describe        :把撥接切回原模式
*/
int inCOMM_MODEM_DialBackUpOff(TRANSACTION_OBJECT *pobTran)
{
	char	szDialBackupEnable[2 + 1] = {0};
	char	szCommMode[2 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inCOMM_MODEM_DialBackUpOn() START !");
	}

	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inCOMM_MODEM_DialBackUpOff() END !");
			inDISP_LogPrintf("----------------------------------------");
		}

		return (VS_SUCCESS);
	}
	else
	{
		/* 如果有DialBackup */
		memset(szDialBackupEnable, 0x00, sizeof(szDialBackupEnable));
		inGetDialBackupEnable(szDialBackupEnable);
		memset(szCommMode, 0x00, sizeof(szCommMode));
		inGetCommMode(szCommMode);

		if (memcmp(szDialBackupEnable, "Y", strlen("Y")) == 0 && (pobTran->uszDialBackup == VS_TRUE	||
									  memcmp(szCommMode, _COMM_MODEM_MODE_, strlen(_COMM_MODEM_MODE_)) == 0))
		{
			inCOMM_End(pobTran); /* 有可能沒有斷線，先斷線 */

			/* 切換通訊模式為【ETHERNET】 */
			if (inLoadCFGTRec(0) != VS_SUCCESS)
			{
				return (VS_ERROR);		/* 共用參數檔【Config.txt】 */
			}

			inSetCommMode(_COMM_ETHERNET_MODE_);

			if (inSaveCFGTRec(0) != VS_SUCCESS)
			{
				return (VS_ERROR);		/* 共用參數檔【Config.txt】 */
			}

			inCOMM_ETHERNET_SetFuncIndex();
			if (inCOMM_DoInitial() != VS_SUCCESS)
			{
				inDISP_Msg_BMP(_ERR_INIT_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "ETHERNET", _LINE_8_5_);

				return (VS_ERROR);
			}

		}

		pobTran->uszDialBackup = VS_FALSE;

		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inCOMM_MODEM_DialBackUpOff() END !");
			inDISP_LogPrintf("----------------------------------------");
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inCOMM_CreditPredialDisconnect
Date&Time       :
Describe        :
*/
int inCOMM_CreditPredialDisconnect(TRANSACTION_OBJECT *pobTran)
{
        /* inCOMM_CreditPredialDisconnect() START */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inCOMM_CreditPredialDisconnect() START!");

        /* inCOMM_CreditPredialDisconnect() END */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inCOMM_CreditPredialDisconnect() END!");

        return (VS_SUCCESS);
}

/*
Function        :inCOMM_DoInitial
Date&Time       :2018/9/20 下午 4:53
Describe        :執行設定後的Initial Function
*/
int inCOMM_DoInitial(void)
{
        int     inRetVal = VS_SUCCESS;

        /* inCOMM_DoInitial() START */
        if (ginDebug == VS_TRUE)
	{
                inDISP_LogPrintf("inCOMM_DoInitial() START!");
	}

	if (gsrobCommunication.inInitialize != NULL)
	{
		inRetVal = gsrobCommunication.inInitialize();
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("Initial Function 尚未設定");
		}
		
		return (VS_ERROR);
	}

        /* inCOMM_ETHERNET_SetFuncIndex() END */
        if (ginDebug == VS_TRUE)
	{
                inDISP_LogPrintf("inCOMM_DoInitial() END!");
	}

        return (inRetVal);
}

/*
Function        :inCOMM_ETHERNET_SetFuncIndex
Date&Time       :
Describe        :將function pointer設定成對應乙太網路模式的fuction
*/
int inCOMM_ETHERNET_SetFuncIndex(void)
{
        int     inRetVal = VS_SUCCESS;

        /* inCOMM_ETHERNET_SetFuncIndex() START */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inCOMM_ETHERNET_SetFuncIndex() START!");

	gsrobCommunication.inInitialize = inETHERNET_Initial;
	gsrobCommunication.inBegin = inETHERNET_Begin;
	gsrobCommunication.inCheck = inETHERNET_SetConfig;
	gsrobCommunication.inSend = inETHERNET_Send;
	gsrobCommunication.inReceive = inETHERNET_Receive;
	gsrobCommunication.inEnd = inETHERNET_END;
	gsrobCommunication.inFlush = inETHERNET_Flush;
	gsrobCommunication.inDeinitialize = inETHERNET_DeInitial;

        /* inCOMM_ETHERNET_SetFuncIndex() END */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inCOMM_ETHERNET_SetFuncIndex() END!");

        return (inRetVal);
}

/*
Function        :inCOMM_MODEM_SetFuncIndex
Date&Time       :2016/6/3 上午 11:29
Describe        :將function pointer設定成對應撥接模式的fuction
*/
int inCOMM_MODEM_SetFuncIndex(void)
{
	int     inRetVal = VS_SUCCESS;

        /* inCOMM_MODEM_SetFuncIndex() START */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inCOMM_MODEM_SetFuncIndex() START!");

        gsrobCommunication.inInitialize = inModem_Initial;
        gsrobCommunication.inBegin = inModem_Begin;
        gsrobCommunication.inCheck = inModem_Connect;
        gsrobCommunication.inSend = inModem_Send;
        gsrobCommunication.inReceive = inModem_Receive;
        gsrobCommunication.inEnd = inModem_END;
        gsrobCommunication.inFlush = inModem_Flush;
	gsrobCommunication.inDeinitialize = inModem_DeInitial;

        /* inCOMM_MODEM_SetFuncIndex() END */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inCOMM_MODEM_SetFuncIndex() END!");

        return (inRetVal);
}

/*
Function        :inCOMM_WiFi_SetFuncIndex
Date&Time       :2017/7/21 下午 2:44
Describe        :將function pointer設定成對應WiFi模式的fuction
*/
int inCOMM_WiFi_SetFuncIndex(void)
{
	int     inRetVal = VS_SUCCESS;

        /* inCOMM_WiFi_SetFuncIndex() START */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inCOMM_WiFi_SetFuncIndex() START!");

        gsrobCommunication.inInitialize = inWiFi_Initial;
	gsrobCommunication.inBegin = inWiFi_Begin;
        gsrobCommunication.inCheck = inWiFi_SetConfig;
        gsrobCommunication.inSend = inETHERNET_Send;		/* Send和Receive可和Ethernet共用 */
        gsrobCommunication.inReceive = inETHERNET_Receive;
        gsrobCommunication.inEnd = inWiFi_END;
        gsrobCommunication.inFlush = inWiFi_Flush;
	gsrobCommunication.inDeinitialize = inWiFi_DeInitial;

        /* inCOMM_WiFi_SetFuncIndex() END */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inCOMM_WiFi_SetFuncIndex() END!");

        return (inRetVal);
}

/*
Function        :inCOMM_GPRS_SetFuncIndex
Date&Time       :2018/3/2 上午 11:37
Describe        :將function pointer設定成對應GPRS模式的fuction
*/
int inCOMM_GPRS_SetFuncIndex(void)
{
	int     inRetVal = VS_SUCCESS;

        /* inCOMM_GPRS_SetFuncIndex() START */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inCOMM_GPRS_SetFuncIndex() START!");

	gsrobCommunication.inInitialize = inGPRS_Initial;
	gsrobCommunication.inBegin = inGPRS_Begin;
	gsrobCommunication.inCheck = inGPRS_SetConfig;
	gsrobCommunication.inSend = inGPRS_Send;		/* Send和Receive可和Ethernet共用 */
	gsrobCommunication.inReceive = inGPRS_Receive;
	gsrobCommunication.inEnd = inGPRS_END;
	gsrobCommunication.inFlush = inGPRS_Flush;
	gsrobCommunication.inDeinitialize = inGPRS_DeInitial;

        /* inCOMM_GPRS_SetFuncIndex() END */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inCOMM_GPRS_SetFuncIndex() END!");

        return (inRetVal);
}

/*
Function        :inCOMM_Fun3_SetCommWay
Date&Time       :2016/6/3 上午 11:29
Describe        :功能3設定通訊模式
*/
int inCOMM_Fun3_SetCommWay(void)
{
	int	inPage = 1;
	int	inRetVal = VS_SUCCESS;

	while (inPage != 0)
	{
		if (inPage == 1)
		{
			inRetVal = inCOMM_Fun3_SetCommWay_Page1();
			if (inRetVal == VS_NEXT_PAGE)
			{
				inPage = 2;
			}
			else
			{
				inPage = 0;
			}
		}
		else if (inPage == 2)
		{
			inRetVal = inCOMM_Fun3_SetCommWay_Page2();
			if (inRetVal == VS_PREVIOUS_PAGE)
			{
				inPage = 1;
			}
			else
			{
				inPage = 0;
			}
		}
	}

	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

        return (inRetVal);
}

/*
Function        :inCOMM_Fun3_SetCommWay_Page1
Date&Time       :2018/3/8 下午 5:29
Describe        :
*/
int inCOMM_Fun3_SetCommWay_Page1(void)
{
	int		inRetVal = VS_SUCCESS;
	int		inSelect = 0;		/* 勾選目前的通訊模式 */
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char		szKey = 0x00;
	char		szTMSOK[2 + 1] = {0};
	char		szCommMode[2 + 1] = {0};
	char		szIFESMode[2 + 1] = {0};
	char		szDHCP[2 + 1] = {0};
	unsigned char	uszInitialBit = VS_FALSE;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);

	/* 確認現在連接模式 */
	inLoadCFGTRec(0);
	memset(szCommMode, 0x00, sizeof(szCommMode));
	inGetCommMode(szCommMode);

	/* 是否使用SSL */
	memset(szIFESMode, 0x00, sizeof(szIFESMode));
	inGetI_FES_Mode(szIFESMode);

	/* 是否使用DHCP */
	memset(szDHCP, 0x00, sizeof(szDHCP));
	inGetDHCP_Mode(szDHCP);

	inDISP_LogPrintfWithFlag("  TMS[%s] COMM[%s] FES[%s] DHCP[%s] -----",szTMSOK, szCommMode, szIFESMode,szDHCP);
	
	/* 選擇通訊模式 1. 撥接 2. TCP/IP */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_SET_COMM_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);

	/* Modem */
	if (memcmp(szCommMode, _COMM_MODEM_MODE_, 1) == 0)
	{
		inSelect = _COMM_SELECT_MENU_MODEM_;
		inDISP_ChineseFont_Point_Color("V", _FONTSIZE_32X22_, _LINE_32_22_, _COLOR_WHITE_, _COLOR_BUTTON_, 2);
	}
	else if (memcmp(szDHCP, "N", strlen("N")) == 0			&&
		 memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0	&&
		 memcmp(szIFESMode, "N", strlen("N")) == 0)
	{
		inSelect = _COMM_SELECT_MENU_ETHERNET_UCL_;
		inDISP_ChineseFont_Point_Color("V", _FONTSIZE_32X22_, _LINE_32_22_, _COLOR_WHITE_, _COLOR_BUTTON_, 9);
	}
	else if (memcmp(szDHCP, "N", strlen("N")) == 0			&&
		 memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0	&&
		 memcmp(szIFESMode, "Y", strlen("Y")) == 0)
	{
		inSelect = _COMM_SELECT_MENU_ETHERNET_SSL_;
		inDISP_ChineseFont_Point_Color("V", _FONTSIZE_32X22_, _LINE_32_22_, _COLOR_WHITE_, _COLOR_BUTTON_, 17);
	}
	/* Ping 只當功能用*/
	else if (0)
	{
		inSelect = _COMM_SELECT_MENU_ETHERNET_PING_;
	}
	else if (memcmp(szDHCP, "Y", strlen("Y")) == 0			&&
		 memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0	&&
		 memcmp(szIFESMode, "N", strlen("N")) == 0)
	{
		inSelect = _COMM_SELECT_MENU_DHCP_UCL_;
		inDISP_ChineseFont_Point_Color("V", _FONTSIZE_32X22_, _LINE_32_30_, _COLOR_WHITE_, _COLOR_BUTTON_, 9);
	}
	else if (memcmp(szDHCP, "Y", strlen("Y")) == 0			&&
		 memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0	&&
		 memcmp(szIFESMode, "Y", strlen("Y")) == 0)
	{
		inSelect = _COMM_SELECT_MENU_DHCP_SSL_;
		inDISP_ChineseFont_Point_Color("V", _FONTSIZE_32X22_, _LINE_32_30_, _COLOR_WHITE_, _COLOR_BUTTON_, 17);
	}
	else if (memcmp(szDHCP, "N", strlen("N")) == 0			&&
		 memcmp(szCommMode, _COMM_WIFI_MODE_, 1) == 0		&&
		 memcmp(szIFESMode, "N", strlen("N")) == 0)
	{
		inSelect = _COMM_SELECT_MENU_DHCP_UCL_;
	}
	else if (memcmp(szDHCP, "N", strlen("N")) == 0			&&
		 memcmp(szCommMode, _COMM_WIFI_MODE_, 1) == 0		&&
		 memcmp(szIFESMode, "Y", strlen("Y")) == 0)
	{
		inSelect = _COMM_SELECT_MENU_DHCP_SSL_;
	}

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
	while (1)
	{
		uszInitialBit = VS_FALSE;

		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		if (szKey == _KEY_ENTER_)
		{
			switch (inSelect)
			{
				case _COMM_SELECT_MENU_MODEM_:
					szKey = _KEY_1_;
					break;
				case _COMM_SELECT_MENU_ETHERNET_UCL_ :
					szKey = _KEY_2_;
					break;
				case _COMM_SELECT_MENU_ETHERNET_SSL_ :
					szKey = _KEY_3_;
					break;
				case _COMM_SELECT_MENU_ETHERNET_PING_ :
					szKey = _KEY_4_;
					break;
				case _COMM_SELECT_MENU_DHCP_UCL_ :
					szKey = _KEY_5_;
					break;
				case _COMM_SELECT_MENU_DHCP_SSL_ :
					szKey = _KEY_6_;
					break;
				default:
					break;
			}
		}

		if (szKey == _KEY_1_ ||
		    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
		{
                            /* 如果下載完TMS，不能手動改成撥接 */
			if (memcmp(szTMSOK, "Y", 1) == 0)
			{
				continue;
			}

			/* 原本不是MODEM MODE */
			if (inSelect != _COMM_SELECT_MENU_MODEM_)
			{
				/* 設定成Modem Mode*/
				inSetCommMode(_COMM_MODEM_MODE_);
				/* 不加密模式 */
				inSetEncryptMode("0");
				inSaveCFGTRec(0);
				uszInitialBit = VS_TRUE;
			}

			/* 設定PABX 數據機設定 及 授權電話 */
			if (inCOMM_Fun3_SetPhone() != VS_SUCCESS)
			{

			}
			else
			{
				uszInitialBit = VS_TRUE;
			}

			/* 是否要Initial */
			if (uszInitialBit == VS_TRUE)
			{
				/* DeInitial*/
				inCOMM_DeInitCommDevice();
				/* 重開module */
				inCOMM_InitCommDevice();
			}

			break;
		}
		/* Ethernet TCP/IP(UCL) */
		else if (szKey == _KEY_2_			||
			 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
		{
			/* 原本不是ETHERNET MODE */
			if (inSelect != _COMM_SELECT_MENU_ETHERNET_UCL_)
			{
				/* 設定成Ethernet Mode*/
				inSetCommMode(_COMM_ETHERNET_MODE_);
				/* tSAM加密模式 */
				//inSetEncryptMode("1");
				inSetDHCP_Mode("N");
				inSetI_FES_Mode("N");
				inSaveCFGTRec(0);
				uszInitialBit = VS_TRUE;
			}

			/* 設定IP */
			if (inCOMM_Fun3_Ethernet_SetIPAddress() != VS_SUCCESS)
			{

			}
			else
			{
				uszInitialBit = VS_TRUE;
			}

			/* 是否要Initial */
			if (uszInitialBit == VS_TRUE)
			{
				/* DeInitial*/
				inCOMM_DeInitCommDevice();
				/* 重開module */
				inCOMM_InitCommDevice();
			}

			break;
		}
		/* Ethernet TCP/IP(SSL) */
		else if (szKey == _KEY_3_			||
			 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)
		{
			if (inSelect != _COMM_SELECT_MENU_ETHERNET_SSL_)
			{
				/* 設定成Ethernet Mode*/
				inSetCommMode(_COMM_ETHERNET_MODE_);
				/* tSAM加密模式 */
				//inSetEncryptMode("1");
				inSetDHCP_Mode("N");
				inSetI_FES_Mode("Y");
				inSaveCFGTRec(0);
				uszInitialBit = VS_TRUE;
			}

			/* 設定IP */
			if (inCOMM_Fun3_Ethernet_SetIPAddress() != VS_SUCCESS)
			{

			}
			else
			{
				uszInitialBit = VS_TRUE;
			}

			/* 是否要Initial */
			if (uszInitialBit == VS_TRUE)
			{
				/* DeInitial*/
				inCOMM_DeInitCommDevice();
				/* 重開module */
				inCOMM_InitCommDevice();
			}

			break;
		}
		/* Ethernet TCP/IP(PING) */
		else if (szKey == _KEY_4_			||
			 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_7_)
		{

		}
		/* DHCP */
		else if (szKey == _KEY_5_			||
			 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_8_)
		{
			/* 原本不是ETHERNET MODE */
			if (inSelect != _COMM_SELECT_MENU_DHCP_UCL_)
			{
				/* 設定成Ethernet Mode*/
				inSetCommMode(_COMM_ETHERNET_MODE_);
				/* tSAM加密模式 */
				//inSetEncryptMode("1");
				inSetDHCP_Mode("Y");
				inSetI_FES_Mode("N");
				inSaveCFGTRec(0);
				uszInitialBit = VS_TRUE;
			}

			/* 設定IP */
			if (inCOMM_Fun3_Ethernet_SetIPAddress() != VS_SUCCESS)
			{

			}
			else
			{
				uszInitialBit = VS_TRUE;
			}

			/* 是否要Initial */
			if (uszInitialBit == VS_TRUE)
			{
				/* DeInitial*/
				inCOMM_DeInitCommDevice();
				/* 重開module */
				inCOMM_InitCommDevice();
			}

			break;
		}
		/* DHCP(SSL) */
		else if (szKey == _KEY_6_			||
			 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_9_)
		{
			/* 原本不是ETHERNET MODE */
			if (inSelect != _COMM_SELECT_MENU_DHCP_SSL_)
			{
				/* 設定成Ethernet Mode*/
				inSetCommMode(_COMM_ETHERNET_MODE_);
				/* tSAM加密模式 */
				//inSetEncryptMode("1");
				inSetDHCP_Mode("Y");
				inSetI_FES_Mode("Y");
				inSaveCFGTRec(0);
				uszInitialBit = VS_TRUE;
			}

			/* 設定IP */
			if (inCOMM_Fun3_Ethernet_SetIPAddress() != VS_SUCCESS)
			{

			}
			else
			{
				uszInitialBit = VS_TRUE;
			}

			/* 是否要Initial */
			if (uszInitialBit == VS_TRUE)
			{
				/* DeInitial*/
				inCOMM_DeInitCommDevice();
				/* 重開module */
				inCOMM_InitCommDevice();
			}

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
		else if (szKey == _KEY_DOWN_			||
			 inChoice == _DisTouch_Slide_Right_To_Left_)
		{
			inRetVal = VS_NEXT_PAGE;
			break;
		}
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	return (inRetVal);
}

/*
Function        :inCOMM_Fun3_SetCommWay_Page2
Date&Time       :2018/3/8 下午 5:29
Describe        :
*/
int inCOMM_Fun3_SetCommWay_Page2(void)
{
	int		inRetVal = VS_SUCCESS;
	int		inSelect = 0;		/* 勾選目前的通訊模式 */
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char		szKey = 0x00;
	char		szTMSOK[2 + 1] = {0};
	char		szCommMode[2 + 1] = {0};
	char		szIFESMode[2 + 1] = {0};
	char		szDHCP[2 + 1] = {0};
	unsigned char	uszInitialBit = VS_FALSE;

	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);

	/* 確認現在連接模式 */
	inLoadCFGTRec(0);
	memset(szCommMode, 0x00, sizeof(szCommMode));
	inGetCommMode(szCommMode);

	/* 是否使用SSL */
	memset(szIFESMode, 0x00, sizeof(szIFESMode));
	inGetI_FES_Mode(szIFESMode);

	/* 是否使用DHCP */
	memset(szDHCP, 0x00, sizeof(szDHCP));
	inGetDHCP_Mode(szDHCP);

	/* 選擇通訊模式 1. GPRS 2. WIFI */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_SET_COMM_OPTION_2_, 0, _COORDINATE_Y_LINE_8_4_);

	/* Modem */
	if (memcmp(szCommMode, _COMM_GPRS_MODE_, 1) == 0)
	{
		inSelect = _COMM_SELECT_MENU_GPRS_SSL_;
		inDISP_ChineseFont_Point_Color("V", _FONTSIZE_32X22_, _LINE_32_22_, _COLOR_WHITE_, _COLOR_BUTTON_, 2);
	}
	else if (memcmp(szDHCP, "N", strlen("Y")) == 0			&&
		 memcmp(szCommMode, _COMM_WIFI_MODE_, 1) == 0	&&
		 memcmp(szIFESMode, "Y", strlen("Y")) == 0)
	{
		inSelect = _COMM_SELECT_MENU_WIFI_SSL_;
		inDISP_ChineseFont_Point_Color("V", _FONTSIZE_32X22_, _LINE_32_22_, _COLOR_WHITE_, _COLOR_BUTTON_, 9);
	}

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
	while (1)
	{
		uszInitialBit = VS_FALSE;

		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		if (szKey == _KEY_ENTER_)
		{
			switch (inSelect)
			{
				case _COMM_SELECT_MENU_GPRS_SSL_:
					szKey = _KEY_1_;
					break;
				case _COMM_SELECT_MENU_WIFI_SSL_ :
					szKey = _KEY_2_;
					break;
				default:
					break;
			}
		}

		if (szKey == _KEY_1_			||
		    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
		{
			/* 原本不是GPRS MODE */
			if (memcmp(szCommMode, _COMM_GPRS_MODE_, 1) != 0)
			{
				/* 設定成Ethernet Mode*/
				inSetCommMode(_COMM_GPRS_MODE_);
				/* tSAM加密模式 */
				//inSetEncryptMode("1");
				inSetI_FES_Mode("Y");
				inSaveCFGTRec(0);
				inCOMM_InitCommDevice();
			}

			/* 設定IP */
			if (inCOMM_Fun3_GPRS_SetIPAddress() != VS_SUCCESS)
			{

			}
			else
			{
				uszInitialBit = VS_TRUE;
			}

			/* 是否要Initial */
			if (uszInitialBit == VS_TRUE)
			{
				/* DeInitial*/
				inCOMM_DeInitCommDevice();
				/* 重開module */
				inCOMM_InitCommDevice();
			}

		}
		else if (szKey == _KEY_2_			||
			 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
		{
			if (inSelect != _COMM_SELECT_MENU_WIFI_SSL_)
			{
				/* 設定成Ethernet Mode*/
				inSetCommMode(_COMM_WIFI_MODE_);
				inSetI_FES_Mode("Y");
				inSaveCFGTRec(0);
				uszInitialBit = VS_TRUE;
			}

			inWiFi_Test_Menu();

			/* 設定IP */
			if (inCOMM_Fun3_Ethernet_SetIPAddress() != VS_SUCCESS)
			{

			}
			else
			{
				uszInitialBit = VS_TRUE;
			}

			/* 是否要Initial */
			if (uszInitialBit == VS_TRUE)
			{
				/* DeInitial*/
				inCOMM_DeInitCommDevice();
				/* 重開module */
				inCOMM_InitCommDevice();
			}

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
		else if (szKey == _KEY_UP_			||
			 inChoice == _DisTouch_Slide_Left_To_Right_)
		{
			inRetVal = VS_PREVIOUS_PAGE;
			break;
		}
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	return (inRetVal);
}

/*
Function        :inCOMM_Fun3_SetPhone
Date&Time       :2016/6/3 上午 11:29
Describe        :功能3設定Modem Phone
*/
int inCOMM_Fun3_SetPhone(void)
{
	int		inRetVal = 0;
	char		szDispMsg[16 + 1];
	unsigned char   uszKey;
	DISPLAY_OBJECT  srDispObj;

	/* Load EDC Record */
        inLoadEDCRec(0);

	/* 是否撥外線 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_CHECK_PABX_CODE_, 0, _COORDINATE_Y_LINE_8_4_);
	inDISP_BEEP(1, 0);

	while (1)
	{
		uszKey = uszKBD_GetKey(30);

		if (uszKey == _KEY_0_)
		{
			/* 輸入PABX CODE */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_EDIT_PABX_CODE_, 0, _COORDINATE_Y_LINE_8_4_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetPABXCode(szDispMsg);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

			while (1)
			{
				memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
				srDispObj.inMaxLen = 2;
				srDispObj.inY = _LINE_8_7_;
				srDispObj.inR_L = _DISP_RIGHT_;
				srDispObj.inColor = _COLOR_RED_;

				memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
				srDispObj.inOutputLen = 0;

				inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

				if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
					return (VS_ERROR);

				if (strlen(srDispObj.szOutput) > 0)
				{
					inSetPABXCode(srDispObj.szOutput);
					inSaveEDCRec(0);
					break;
				}

				break;
			}

			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			return (VS_ERROR);
		}
		else
		{
			continue;
		}

	}

	/* 是否修改數據機 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_CHECK_MODEM_, 0, _COORDINATE_Y_LINE_8_4_);
	inDISP_BEEP(1, 0);

	while (1)
	{
		uszKey = uszKBD_GetKey(30);

		if (uszKey == _KEY_0_)
		{
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			return (VS_ERROR);
		}
		else
		{
			continue;
		}
	}

	/* 下面放各機型可調整數據機設定 */

	return (VS_SUCCESS);
}

/*
Function        :inCOMM_Fun3_SetPhoneIPAddress
Date&Time       :2016/6/3 上午 11:29
Describe        :功能3設定Ethernet IP
*/
int inCOMM_Fun3_Ethernet_SetIPAddress(void)
{
	int		inCPTIndex = -1;
	int		inRetVal = 0, inDot = 0, i = 0, j = 0;
	char		szDispMsg[16 + 1] = {0};
	char		szTemplate[16 + 1] = {0};
	char		szTemplate2[16 + 1] = {0};
	char		szCOMMIndex[2 + 1] = {0};
	char		szHostName[10 + 1] = {0};

	unsigned char   uszKey;
	unsigned char	uszChange = VS_FALSE;	/* 預設沒改變，若改變EDC IP相關，bit on起來並在結尾重新initial裝置 */
	unsigned char	uszCancel = VS_FALSE;	/* 跳出Ethernet設定迴圈 */
	unsigned char	uszDHCPBit = VS_FALSE;
	unsigned char	uszLen = 0;
	DISPLAY_OBJECT  srDispObj;

	/* 看是否是DHCP模式 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetDHCP_Mode(szTemplate);
	if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
	{
		uszDHCPBit = VS_TRUE;
	}

	/* 修改TCP IP請按0確認 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_CHECK_EDIT_TCPIP_, 0, _COORDINATE_Y_LINE_8_4_);
	inDISP_BEEP(1, 0);

	while (1)
	{
		uszKey = uszKBD_GetKey(30);

		if (uszKey == _KEY_0_)
		{
			break;
		}
		else if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
		{
			return (VS_ERROR);
		}
		else
		{
			continue;
		}
	}

	/* Load EDC Record */
	inLoadEDCRec(0);

	/* Load TMS CPT Record */
	inLoadTMSCPTRec(0);

	/* Load TMS FTP Record */
	inLoadTMSFTPRec(0);
	
	while(1)
	{
		/* 輸入EDC IP */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_EDIT_EDC_IP_, 0, _COORDINATE_Y_LINE_8_4_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		if (uszDHCPBit == VS_TRUE)
		{
			uszLen = sizeof(szDispMsg);
			inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_IP, (unsigned char*)szDispMsg, &uszLen);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
			
			inSetTermIPAddress(szDispMsg);
			inSaveEDCRec(0);
		}
		else
		{
			inGetTermIPAddress(szDispMsg);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
		}

		if (uszDHCPBit == VS_TRUE)
		{
			while (1)
			{
				uszKey = uszKBD_GetKey(30);
				if (uszKey == _KEY_ENTER_)
				{
					break;
				}
				else if (uszKey == _KEY_CANCEL_	||
					 uszKey == _KEY_TIMEOUT_)
				{
					uszCancel = VS_TRUE;
					break;
				}
				else
				{
					continue;
				}
			}
		}
		else
		{
			while (1)
			{
				memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
				srDispObj.inMaxLen = 15;
				srDispObj.inY = _LINE_8_7_;
				srDispObj.inR_L = _DISP_RIGHT_;
				srDispObj.inColor = _COLOR_RED_;

				memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
				srDispObj.inOutputLen = 0;

				inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

				if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				{
					uszCancel = VS_TRUE;
					break;
				}

				if (strlen(srDispObj.szOutput) > 0)
				{
					inDot = 0;

					for (j = 0 ;; j ++)
					{
						if (srDispObj.szOutput[j] == '.')
							inDot ++;
						else if (srDispObj.szOutput[j] == 0x00)
							break;
					}

					if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
					{
						inSetTermIPAddress(srDispObj.szOutput);
						inSaveEDCRec(0);
						/* 有做更改 */
						uszChange = VS_TRUE;
						break;
					}
					else
					{
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
						inDISP_BEEP(2, 500);
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						continue;
					}
				}
				else
				{
					break;
				}
			}
		}

		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}

		/* 輸入EDC SUB MASK */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_EDIT_EDC_SUB_MASK_, 0, _COORDINATE_Y_LINE_8_4_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		if (uszDHCPBit == VS_TRUE)
		{
			uszLen = sizeof(szDispMsg);
			inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_MASK, (unsigned char*)szDispMsg, &uszLen);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
			
			inSetTermMASKAddress(szDispMsg);
			inSaveEDCRec(0);
		}
		else
		{
			inGetTermMASKAddress(szDispMsg);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
		}

		if (uszDHCPBit == VS_TRUE)
		{
			while (1)
			{
				uszKey = uszKBD_GetKey(30);
				if (uszKey == _KEY_ENTER_)
				{
					break;
				}
				else if (uszKey == _KEY_CANCEL_	||
					 uszKey == _KEY_TIMEOUT_)
				{
					uszCancel = VS_TRUE;
					break;
				}
				else
				{
					continue;
				}
			}
		}
		else
		{
			while (1)
			{
				memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
				srDispObj.inMaxLen = 15;
				srDispObj.inY = _LINE_8_7_;
				srDispObj.inR_L = _DISP_RIGHT_;
				srDispObj.inColor = _COLOR_RED_;

				memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
				srDispObj.inOutputLen = 0;

				inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

				if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				{
					uszCancel = VS_TRUE;
					break;
				}


				if (strlen(srDispObj.szOutput) > 0)
				{
					inDot = 0;

					for (j = 0 ;; j ++)
					{
						if (srDispObj.szOutput[j] == '.')
							inDot ++;
						else if (srDispObj.szOutput[j] == 0x00)
							break;
					}

					if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
					{
						inSetTermMASKAddress(srDispObj.szOutput);
						inSaveEDCRec(0);
						/* 有做更改 */
						uszChange = VS_TRUE;
						break;
					}
					else
					{
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
						inDISP_BEEP(2, 500);
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						continue;
					}
				}
				else
				{
					break;
				}
			}
		}

		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}

		/* 輸入EDC Geteway */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_EDIT_EDC_GATEWAY_, 0, _COORDINATE_Y_LINE_8_4_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		if (uszDHCPBit == VS_TRUE)
		{
			uszLen = sizeof(szDispMsg);
			inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_GATEWAY, (unsigned char*)szDispMsg, &uszLen);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
			
			inSetTermGetewayAddress(szDispMsg);
			inSaveEDCRec(0);
		}
		else
		{
			inGetTermGetewayAddress(szDispMsg);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
		}

		if (uszDHCPBit == VS_TRUE)
		{
			while (1)
			{
				uszKey = uszKBD_GetKey(30);
				if (uszKey == _KEY_ENTER_)
				{
					break;
				}
				else if (uszKey == _KEY_CANCEL_	||
					 uszKey == _KEY_TIMEOUT_)
				{
					uszCancel = VS_TRUE;
					break;
				}
				else
				{
					continue;
				}
			}
		}
		else
		{
			while (1)
			{
				memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
				srDispObj.inMaxLen = 15;
				srDispObj.inY = _LINE_8_7_;
				srDispObj.inR_L = _DISP_RIGHT_;
				srDispObj.inColor = _COLOR_RED_;

				memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
				srDispObj.inOutputLen = 0;

				inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

				if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				{
					uszCancel = VS_TRUE;
					break;
				}


				if (strlen(srDispObj.szOutput) > 0)
				{
					inDot = 0;

					for (j = 0 ;; j ++)
					{
						if (srDispObj.szOutput[j] == '.')
							inDot ++;
						else if (srDispObj.szOutput[j] == 0x00)
							break;
					}

					if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
					{
						inSetTermGetewayAddress(srDispObj.szOutput);
						inSaveEDCRec(0);
						/* 有做更改 */
						uszChange = VS_TRUE;
						break;
					}
					else
					{
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
						inDISP_BEEP(2, 500);
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						continue;
					}
				}
				else
				{
					break;
				}
			}
		}

		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}
/* 富邦不需要輸入TMS IP ,先拿掉 20190219 [SAM]*/
#if 0
		/* 輸入TMS IP */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_EDIT_TMS_IP, 0, _COORDINATE_Y_LINE_8_4_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetTMSIPAddress(szDispMsg);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		while (1)
		{
			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 15;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;

			inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			{
				uszCancel = VS_TRUE;
				break;
			}


			if (strlen(srDispObj.szOutput) > 0)
			{
				inDot = 0;

				for (j = 0 ;; j ++)
				{
					if (srDispObj.szOutput[j] == '.')
						inDot ++;
					else if (srDispObj.szOutput[j] == 0x00)
						break;
				}

				if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
				{
					inSetTMSIPAddress(srDispObj.szOutput);
					inSaveTMSCPTRec(0);
					break;
				}
				else
				{
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
					inDISP_BEEP(2, 500);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					continue;
				}
			}

			break;
		}

		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}

		/* 輸入TMS PORT */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_EDIT_TMS_PORT_, 0, _COORDINATE_Y_LINE_8_4_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetTMSPortNum(szDispMsg);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		while (1)
		{
			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 5;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;

			inRetVal = inDISP_Enter8x16(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			{
				uszCancel = VS_TRUE;
				break;
			}


			if (strlen(srDispObj.szOutput) > 0)
			{
				inSetTMSPortNum(srDispObj.szOutput);
				inSaveTMSCPTRec(0);
				break;
			}

			break;
		}

		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}
#endif
		/* 輸入 FTP IP */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont(" FTP HOST IP?", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetFTPIPAddress(szDispMsg);
		inFunc_PAD_ASCII(szDispMsg, szDispMsg, ' ', 16, _PAD_RIGHT_FILL_LEFT_);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		while (1)
		{
			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 15;
			srDispObj.inY = _LINE_8_6_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;

			inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			{
				uszCancel = VS_TRUE;
				break;
			}


			if (strlen(srDispObj.szOutput) > 0)
			{
				inSetFTPIPAddress(srDispObj.szOutput);
				inSaveTMSFTPRec(0);
			}

			break;
		}

		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}

		/* 輸入 FTP PORT */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont(" FTP HOST PORT?", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetFTPPortNum(szDispMsg);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_);

		while (1)
		{
			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 6;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;

			inRetVal = inDISP_Enter8x16(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			{
				uszCancel = VS_TRUE;
				break;
			}


			if (strlen(srDispObj.szOutput) > 0)
			{
				inSetFTPPortNum(srDispObj.szOutput);
				inSaveTMSFTPRec(0);
				break;
			}

			break;
		}

		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}
		
                /* 客製化參數為 Costco時增加 TSP IP/Port，Miyano 20230111 */
                if (vbCheckCostcoCustom(Costco_New))
                {
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                        inDISP_ChineseFont(" TSP HOST IP?", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
                        memset(szDispMsg, 0x00, sizeof(szDispMsg));
                        inGetTSP_IP(szDispMsg);
                        inFunc_PAD_ASCII(szDispMsg, szDispMsg, ' ', 16, _PAD_RIGHT_FILL_LEFT_);
                        inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

                        while (1)
                        {
                                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                                srDispObj.inMaxLen = 15;
                                srDispObj.inY = _LINE_8_6_;
                                srDispObj.inR_L = _DISP_RIGHT_;
                                srDispObj.inColor = _COLOR_RED_;

                                memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
                                srDispObj.inOutputLen = 0;

                                inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

                                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                                {
                                        uszCancel = VS_TRUE;
                                        break;
                                }


                                if (strlen(srDispObj.szOutput) > 0)
                                {
                                        inSetTSP_IP(srDispObj.szOutput);
                                }

                                break;
                        }

                        /* 跳出最大的迴圈*/
                        if (uszCancel == VS_TRUE)
                        {
                                break;
                        }

                        /* 輸入 TSP PORT */
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                        inDISP_ChineseFont(" TSP HOST PORT?", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
                        memset(szDispMsg, 0x00, sizeof(szDispMsg));
                        inGetTSP_Port(szDispMsg);
                        inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_);

                        while (1)
                        {
                                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                                srDispObj.inMaxLen = 6;
                                srDispObj.inY = _LINE_8_7_;
                                srDispObj.inR_L = _DISP_RIGHT_;
                                srDispObj.inColor = _COLOR_RED_;

                                memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
                                srDispObj.inOutputLen = 0;

                                inRetVal = inDISP_Enter8x16(&srDispObj);

                                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                                {
                                        uszCancel = VS_TRUE;
                                        break;
                                }


                                if (strlen(srDispObj.szOutput) > 0)
                                {
                                        inSetTSP_Port(srDispObj.szOutput);
                                        break;
                                }

                                break;
                        }

                        /* 跳出最大的迴圈*/
                        if (uszCancel == VS_TRUE)
                        {
                                break;
                        }
                }

		/* 輸入HOST IP & PORT */
		for (i = 0 ;; i ++)
		{
			if (inLoadHDTRec(i) < 0)
			{
				inDISP_LogPrintf(" COMM Func3 Eth Set Ip Load HDT[%d] *Error* Line[%d]", 0 ,__LINE__);
				break;
			}	
			
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetHostEnable(szTemplate);

			if (!memcmp(&szTemplate[0], "Y", 1))
			{
				memset(szCOMMIndex, 0x00, sizeof(szCOMMIndex));
				inGetCommunicationIndex(szCOMMIndex);
				inCPTIndex = atoi(szCOMMIndex) - 1;
				/* HDT對應到CPT */
				if (inLoadCPTRec(inCPTIndex) < 0)
					break;

				memset(szHostName, 0x00, sizeof(szHostName));
				inGetHostLabel(szHostName);
				inFunc_DiscardSpace(szHostName);

				/* 因為富邦CUP是同批次，所以要跳過CUP 20190218 [SAM] */	
				if(!memcmp(szHostName, _HOST_NAME_CUP_, strlen(_HOST_NAME_CUP_)) )
					continue;
					
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				/* 輸入HOST IP */
				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, " %s HOST IP?", szHostName);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);

				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				inGetHostIPPrimary(szDispMsg);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

				while (1)
				{
					memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
					srDispObj.inMaxLen = 15;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;

					inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
					{
						uszCancel = VS_TRUE;
						break;
					}


					if (strlen(srDispObj.szOutput) > 0)
					{
						inDot = 0;

						for (j = 0 ;; j ++)
						{
							if (srDispObj.szOutput[j] == '.')
								inDot ++;
							else if (srDispObj.szOutput[j] == 0x00)
								break;
						}

						if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
						{
							inSetHostIPPrimary(srDispObj.szOutput);
							inSaveCPTRec(inCPTIndex);
							break;
						}
						else
						{
							inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
							inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
							inDISP_BEEP(2, 500);
							inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
							continue;
						}
					}

					break;
				}

				/* 跳出最大的迴圈*/
				if (uszCancel == VS_TRUE)
				{
					break;
				}

				/* 輸入HOST Port */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, " %s PORT NUM?", szHostName);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);

				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetHostPortNoPrimary(szTemplate);

				memset(szTemplate2, 0x00, sizeof(szTemplate2));
				inGetTCPHeadFormat(szTemplate2);

				sprintf(szDispMsg, "%s (%s)", szTemplate, szTemplate2);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

				while (1)
				{
					memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
					srDispObj.inMaxLen = 15;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;

					inRetVal = inDISP_Enter8x16(&srDispObj);

					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
					{
						uszCancel = VS_TRUE;
						break;
					}

					if (strlen(srDispObj.szOutput) > 0)
					{
						inSetHostPortNoPrimary(srDispObj.szOutput);
						inSaveCPTRec(inCPTIndex);
						break;
					}

					break;
				}

				/* 跳出最大的迴圈*/
				if (uszCancel == VS_TRUE)
				{
					break;
				}

			}
			else
			{
				continue;
			}

		}
		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}

	}

	/* 如果有更改EDC IP相關，重新initial裝置 重open */
	if (uszChange == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}


}

/*
Function        :inCOMM_Fun3_GPRS_SetIPAddress
Date&Time       :2018/3/2 下午 2:59
Describe        :功能3設定Ethernet IP
*/
int inCOMM_Fun3_GPRS_SetIPAddress(void)
{
	int		inCPTIndex = -1;
	int		inRetVal = 0, inDot = 0, i = 0, j = 0;
        char		szDispMsg[16 + 1] = {0};
        char		szTemplate[16 + 1] = {0};
	char		szTemplate2[16 + 1] = {0};
	char		szCOMMIndex[2 + 1] = {0};
	char		szHostName[10 + 1] = {0};
	char		szIFESMode[2 + 1] = {0};
	unsigned char   uszKey;
	unsigned char	uszChange = VS_FALSE;	/* 預設沒改變，若改變EDC IP相關，bit on起來並在結尾重新initial裝置 */
	unsigned char	uszCancel = VS_FALSE;	/* 跳出Ethernet設定迴圈 */
	DISPLAY_OBJECT  srDispObj;

	/* 修改TCP IP請按0確認 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_CHECK_EDIT_TCPIP_, 0, _COORDINATE_Y_LINE_8_4_);
	inDISP_BEEP(1, 0);

	while (1)
	{
		uszKey = uszKBD_GetKey(30);

		if (uszKey == _KEY_0_)
		{
			break;
		}
		else if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
		{
			return (VS_ERROR);
		}
		else
		{
			continue;
		}
	}

	/* Load EDC Record */
        inLoadEDCRec(0);

        /* Load TMS CPT Record */
        inLoadTMSCPTRec(0);

	while(1)
	{
		/* 輸入TMS IP */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_EDIT_TMS_IP, 0, _COORDINATE_Y_LINE_8_4_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetTMSIPAddress(szDispMsg);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		while (1)
		{
			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 15;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;

			inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			{
				uszCancel = VS_TRUE;
				break;
			}


			if (strlen(srDispObj.szOutput) > 0)
			{
				inDot = 0;

				for (j = 0 ;; j ++)
				{
					if (srDispObj.szOutput[j] == '.')
						inDot ++;
					else if (srDispObj.szOutput[j] == 0x00)
						break;
				}

				if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
				{
					inSetTMSIPAddress(srDispObj.szOutput);
					inSaveTMSCPTRec(0);
					break;
				}
				else
				{
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
					inDISP_BEEP(2, 500);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					continue;
				}
			}

			break;
		}

		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}

		/* 輸入TMS PORT */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_EDIT_TMS_PORT_, 0, _COORDINATE_Y_LINE_8_4_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetTMSPortNum(szDispMsg);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		while (1)
		{
			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 5;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;

			inRetVal = inDISP_Enter8x16(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			{
				uszCancel = VS_TRUE;
				break;
			}


			if (strlen(srDispObj.szOutput) > 0)
			{
				inSetTMSPortNum(srDispObj.szOutput);
				inSaveTMSCPTRec(0);
			}

			break;
		}

		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}

		/* 輸入 TMS IP */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSDownloadMode(szTemplate);

		if (memcmp(szTemplate, _TMS_DLMODE_FTPS_, strlen(_TMS_DLMODE_FTPS_)) == 0)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("FTP HOST IP?", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetFTPIPAddress(szDispMsg);
			inFunc_PAD_ASCII(szDispMsg, szDispMsg, ' ', 16, _PAD_RIGHT_FILL_LEFT_);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

			while (1)
			{
				memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
				srDispObj.inMaxLen = 15;
				srDispObj.inY = _LINE_8_6_;
				srDispObj.inR_L = _DISP_RIGHT_;
				srDispObj.inColor = _COLOR_RED_;

				memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
				srDispObj.inOutputLen = 0;

				inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

				if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				{
					uszCancel = VS_TRUE;
					break;
				}


				if (strlen(srDispObj.szOutput) > 0)
				{
					inSetFTPIPAddress(srDispObj.szOutput);
					inSaveTMSFTPRec(0);
				}

				break;
			}
		}

		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}

		/* 輸入HOST IP & PORT */
		for (i = 0 ;; i ++)
		{
			if (inLoadHDTRec(i) < 0)
			{
				inDISP_LogPrintf(" COMM Func3 Gprs Set Ip Load HDT[%d] *Error* Line[%d]", 0 ,__LINE__);
				break;
			}

			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetHostEnable(szTemplate);

			if (!memcmp(&szTemplate[0], "Y", 1))
			{
				memset(szCOMMIndex, 0x00, sizeof(szCOMMIndex));
				inGetCommunicationIndex(szCOMMIndex);
				inCPTIndex = atoi(szCOMMIndex) - 1;
				/* HDT對應到CPT */
				if (inLoadCPTRec(inCPTIndex) < 0)
					break;

				memset(szHostName, 0x00, sizeof(szHostName));
				inGetHostLabel(szHostName);
				inFunc_DiscardSpace(szHostName);

				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				/* 輸入HOST IP */
				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, " %s HOST IP?", szHostName);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);

				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				inGetHostIPPrimary(szDispMsg);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

				while (1)
				{
					memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
					srDispObj.inMaxLen = 15;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;

					inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
					{
						uszCancel = VS_TRUE;
						break;
					}


					if (strlen(srDispObj.szOutput) > 0)
					{
						inDot = 0;

						for (j = 0 ;; j ++)
						{
							if (srDispObj.szOutput[j] == '.')
								inDot ++;
							else if (srDispObj.szOutput[j] == 0x00)
								break;
						}

						if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
						{
							inSetHostIPPrimary(srDispObj.szOutput);
							inSaveCPTRec(inCPTIndex);
							break;
						}
						else
						{
							inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
							inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
							inDISP_BEEP(2, 500);
							inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
							continue;
						}
					}

					break;
				}

				/* 跳出最大的迴圈*/
				if (uszCancel == VS_TRUE)
				{
					break;
				}

				/* 輸入HOST Port */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, " %s PORT NUM?", szHostName);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);

				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetHostPortNoPrimary(szTemplate);

				memset(szTemplate2, 0x00, sizeof(szTemplate2));
				inGetTCPHeadFormat(szTemplate2);

				sprintf(szDispMsg, "%s (%s)", szTemplate, szTemplate2);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

				while (1)
				{
					memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
					srDispObj.inMaxLen = 15;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;

					inRetVal = inDISP_Enter8x16(&srDispObj);

					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
					{
						uszCancel = VS_TRUE;
						break;
					}

					if (strlen(srDispObj.szOutput) > 0)
					{
						inSetHostPortNoPrimary(srDispObj.szOutput);
						inSaveCPTRec(inCPTIndex);
						break;
					}

					break;
				}

				/* 跳出最大的迴圈*/
				if (uszCancel == VS_TRUE)
				{
					break;
				}

				/* IFES才能輸入第二組 */
				memset(szIFESMode, 0x00, sizeof(szIFESMode));
				inGetI_FES_Mode(szIFESMode);
				if (memcmp(szIFESMode, "Y", strlen("Y")) == 0)
				{
					/* 這裡輸入第二組IP */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					/* 輸入HOST IP */
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, " %s HOST IP2?", szHostName);
					inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					inGetHostIPSecond(szDispMsg);
					inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

					while (1)
					{
						memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
						srDispObj.inMaxLen = 15;
						srDispObj.inY = _LINE_8_7_;
						srDispObj.inR_L = _DISP_RIGHT_;
						srDispObj.inColor = _COLOR_RED_;

						memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
						srDispObj.inOutputLen = 0;

						inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

						if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						{
							uszCancel = VS_TRUE;
							break;
						}


						if (strlen(srDispObj.szOutput) > 0)
						{
							inDot = 0;

							for (j = 0 ;; j ++)
							{
								if (srDispObj.szOutput[j] == '.')
									inDot ++;
								else if (srDispObj.szOutput[j] == 0x00)
									break;
							}

							if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
							{
								inSetHostIPSecond(srDispObj.szOutput);
								inSaveCPTRec(inCPTIndex);
								break;
							}
							else
							{
								inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
								inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
								inDISP_BEEP(2, 500);
								inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
								continue;
							}
						}

						break;
					}

					/* 跳出最大的迴圈*/
					if (uszCancel == VS_TRUE)
					{
						break;
					}

					/* 輸入HOST Port */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, " %s PORT NUM2?", szHostName);
					inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					memset(szTemplate, 0x00, sizeof(szTemplate));
					inGetHostPortNoSecond(szTemplate);

					memset(szTemplate2, 0x00, sizeof(szTemplate2));
					inGetTCPHeadFormat(szTemplate2);

					sprintf(szDispMsg, "%s (%s)", szTemplate, szTemplate2);
					inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

					while (1)
					{
						memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
						srDispObj.inMaxLen = 15;
						srDispObj.inY = _LINE_8_7_;
						srDispObj.inR_L = _DISP_RIGHT_;
						srDispObj.inColor = _COLOR_RED_;

						memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
						srDispObj.inOutputLen = 0;

						inRetVal = inDISP_Enter8x16(&srDispObj);

						if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						{
							uszCancel = VS_TRUE;
							break;
						}

						if (strlen(srDispObj.szOutput) > 0)
						{
							inSetHostPortNoSecond(srDispObj.szOutput);
							inSaveCPTRec(inCPTIndex);
							break;
						}

						break;
					}

					/* 跳出最大的迴圈*/
					if (uszCancel == VS_TRUE)
					{
						break;
					}
				}
			}
			else
			{
				continue;
			}

		}
		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}

	}

	/* 如果有更改EDC IP相關，重新initial裝置 重open */
	if (uszChange == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}


}

/*
Function        :inCOMM_DeInitCommDevice
Date&Time       :2016/8/9 下午 4:05
Describe        :關閉通訊裝置來重新Initial
*/
int inCOMM_DeInitCommDevice()
{
        char    szCommmode[1 + 1];

        /* inCOMM_InitCommDevice() START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inCOMM_DeInitCommDevice() START！");
        }

        memset(szCommmode,0x00,sizeof(szCommmode));
        /* 取得通訊模式 */
        if (inGetCommMode(szCommmode) == VS_ERROR)
        {
                return (VS_ERROR);
        }


        if (gsrobCommunication.inDeinitialize() != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("_COMM_ .inDeinitialize Error");

		return (VS_ERROR);
	}
	else
	{
		gsrobCommunication.inInitialize = NULL;
		gsrobCommunication.inBegin = NULL;
		gsrobCommunication.inCheck = NULL;
		gsrobCommunication.inSend = NULL;
		gsrobCommunication.inReceive = NULL;
		gsrobCommunication.inEnd = NULL;
		gsrobCommunication.inFlush = NULL;
		gsrobCommunication.inDeinitialize = NULL;
	}

        /* inCOMM_DeInitCommDevice()_END */
        if (ginDebug == VS_TRUE)
        {
		inDISP_LogPrintf("inCOMM_DeInitCommDevice()_END");
                inDISP_LogPrintf("----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inCOMM_TCP_SetConnectTO
Date&Time       :2018/3/1 下午 2:40
Describe        :注意，這只能用在Modem和GPRS上，
 *		 The timeout value in millisecond
*/
int inCOMM_TCP_SetConnectTO(unsigned long ulTime)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = 0;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inCOMM_TCP_SetConnectTO() START !");
	}

	usRetVal = CTOS_TCP_SetConnectTO(ulTime);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("CTOS_TCP_SetConnectTO OK!");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_TCP_SetConnectTO Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}


	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCOMM_TCP_SetConnectTO() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCOMM_TCP_SetRetryCounter
Date&Time       :2018/3/1 下午 2:40
Describe        :注意，這只能用在Modem和GPRS上，
 *		 The number of times in retry counter.
 *		 Please set this value to be larger than 2.(最小不可低於二)
*/
int inCOMM_TCP_SetRetryCounter(unsigned short usTime)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = 0;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inCOMM_TCP_SetRetryCounter() START !");
	}

	usRetVal = CTOS_TCP_SetRetryCounter(usTime);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("CTOS_TCP_SetRetryCounter OK!");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_TCP_SetRetryCounter Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}


	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inCOMM_TCP_SetRetryCounter() END !");
		inDISP_LogPrintf("----------------------------------------");
	}

	return (VS_SUCCESS);
}

int inCOMM_Test(void)
{
        TRANSACTION_OBJECT	pobTran;
        int			inSendSize = 0;
        int			inReceiveSize = 0;
        int			i;
        char			szTemplate[1024 + 1];
	char			szReceive[1024 + 1];
        unsigned char		szSend[1024 + 1] = {"\x60\x00\x22\x80\x00\x02\x00\x30\x20\x07\x80\x20\xC1\x02\x04\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x01\x00\x52\x00\x01\x00\x22\x00\x37\x43\x11\x78\x97\x23\x94\x81\x09\xD1\x70\x42\x01\x14\x95\x01\x17\x10\x00\x0F\x31\x32\x30\x30\x34\x39\x39\x33\x30\x30\x30\x30\x30\x31\x36\x36\x33\x30\x30\x30\x30\x33\x36\x00\x14\x31\x34\x30\x30\x33\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x16\x5F\x2A\x02\x09\x01\x82\x02\x3C\x00\x95\x05\x00\x00\x00\x80\x00\x9A\x03\x15\x09\x07\x9C\x01\x00\x9F\x02\x06\x00\x00\x00\x00\x10\x00\x9F\x03\x06\x00\x00\x00\x00\x00\x00\x9F\x09\x02\x00\x8C\x9F\x10\x07\x06\x01\x0A\x03\xA0\xA0\x06\x9F\x1A\x02\x01\x58\x9F\x26\x08\xDD\xA4\xE3\x11\x68\xA6\xBE\x1B\x9F\x27\x01\x80\x9F\x33\x03\x60\x28\xD0\x9F\x34\x03\x1E\x03\x00\x9F\x35\x01\x22\x9F\x36\x02\x02\x5E\x9F\x37\x04\x16\x68\xF6\x18\x9F\x41\x04\x00\x00\x00\x00\x9F\x53\x01\x52\x00\x06\x30\x30\x30\x30\x30\x31"};
//        char    szTemplate1[4 + 1];

        inDISP_LogPrintf("inCOMM_Test()");

        inSendSize = 438;

/*
        if (inLoadCFGTRec(0) < 0)
        {
                return (VS_ERROR);
        }

        if (inLoadCPTRec(0) < 0)
        {
                return (VS_ERROR);
        }
*/

        inCOMM_InitCommDevice();
        inCOMM_ConnectStart(&pobTran);
        inCOMM_Send(szSend, inSendSize, 10, VS_TRUE);

        memset(szReceive,0x00,sizeof(szReceive));
        inCOMM_Receive((unsigned char *)szReceive, 1024, 30, VS_TRUE);
        inReceiveSize = atoi(&szReceive[0]);

        memset(szTemplate, 0x00, sizeof(szTemplate));
        int j = 0;
        for (i = 0; i < 60; i++)
        {
                sprintf(&szTemplate[j], "%02X ", szSend[i]);
                j += 3;

                if (j == 24)
                {
                    inDISP_LogPrintf(szTemplate);
                    memset(szTemplate, 0x00, sizeof(szTemplate));
                    j = 0;
                }

        }
/*
  sprintf(&szTemplate[0], "%02X ", szReceive[0]);
*/
                inDISP_LogPrintf(szTemplate);

        inCOMM_End(&pobTran);
        return (VS_SUCCESS);
}


