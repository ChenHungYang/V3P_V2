#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include "curl/curl.h"
#include <sqlite3.h>

#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DispMsg.h"
#include "../PRINT/Print.h"
#include "../FUNCTION/Function.h"
#include "../FUNCTION/Sqlite.h"
#include "../EVENT/MenuMsg.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/CPT.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/HDPT.h"
#include "../COMM/TLS.h"

#include "../TMS/TMSTABLE/TmsCPT.h"


#include "Comm.h"
#include "GPRS.h"

#include "../FUNCTION/COSTCO_FUNC/Costco.h"

extern	int		ginTrans_ClientFd;
extern	unsigned int	guiCTX_ID;		/* TLS Context ID */
extern	unsigned int	guiSSL_ID;		/* TLS Session ID */

extern  int     ginDebug;  /* Debug使用 extern */
extern  int     ginISODebug;

extern  VS_BOOL         fLinkCostcoPayHost;     /* CostcoPay主機專用 */

int	ginGPRSFlow = -1;			/* (-1:NORMAL 1:IFES)紀錄GPRS走哪個流程，用這個紀錄是因為下TMS有可能修改到CFGT，所以IFES Mode可能會被改到，導致無法斷線 */

/*
Function        :inGPRS_Open
Date&Time       :2018/3/1 上午 11:58
Describe        :When baIP is "\x00\x00\x00\x00", the local IP will be assigned by GPRS Server.
*/
int inGPRS_Open(unsigned char *uszIP, unsigned char *uszAPNName, unsigned char *uszID, unsigned char *uszPW)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inGPRS_Open() START !");
	}
	
	CTOS_PPP_SetTO(45000);
	
	usRetVal = CTOS_TCP_GPRSOpen(uszIP, uszAPNName, uszID, uszPW);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("GPRS_OPEN OK!");
		}
	}
	else if (usRetVal == d_TCP_IO_PROCESSING)
	{
//		if (ginDebug == VS_TRUE)
//		{
//			inDISP_LogPrintf("GPRS_OPEN IO PROCESSING!");
//		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_TCP_GPRSOpen Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inGPRS_Open() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Close
Date&Time       :2018/3/1 下午 1:41
Describe        :
*/
int inGPRS_Close(void)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inGPRS_Close() START !");
	}
	
	usRetVal = CTOS_TCP_GPRSClose();
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("CTOS_TCP_GPRSClose OK!");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_TCP_GPRSClose Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inGPRS_Close() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Close_Asynchronous
Date&Time       :2018/3/1 下午 1:46
Describe        :Please call CTOS_TCP_GPRSStatus() to check whether this action is finished.(非同步，所以要用CTOS_TCP_GPRSStatus來確認狀態是否完成)
*/
int inGPRS_Close_Asynchronous(void)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inGPRS_Close_Asynchronous() START !");
	}
	
	usRetVal = CTOS_TCP_GPRSClose_A();
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("CTOS_TCP_GPRSClose_A OK!");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_TCP_GPRSClose_A Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inGPRS_Close_Asynchronous() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Connect_Flow
Date&Time       :2018/3/1 下午 3:22
Describe        :分流使用CTOS還是使用原生C的Connect
*/
int inGPRS_Connect_Flow(char *szHostIP, char *szPort)
{
	int		inRetVal = VS_ERROR;
	char		szI_FES_Mode[2 + 1];
	char		szTRTFileName[12 + 1];
	char		szDebugMsg[100 + 1];
	unsigned short	usPort = 0;
	unsigned char	uszIP[4 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inGPRS_Connect_Flow() START !");

		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "IP:%s Port:%s", szHostIP, szPort);
		inDISP_LogPrintf(szDebugMsg);

	}
	
	/* 一律先重置Handle */
	ginTrans_ClientFd = -1;
	
	/* 用原生C Socket */
	memset(szI_FES_Mode, 0x00, sizeof(szI_FES_Mode));
	inGetI_FES_Mode(szI_FES_Mode);
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "IFES: %s, %s", szI_FES_Mode, szTRTFileName);
		inDISP_LogPrintf(szDebugMsg);
	}
	
	/* 判斷是IFES及非大來，其他全跑IFES */
	/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
	if (memcmp(szI_FES_Mode, "Y", strlen("Y")) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Connect Flow: TLS");
			inDISP_LogPrintf(szDebugMsg);
		}
		
		inRetVal = inGPRS_Connect_TLS_Flow(szHostIP, szPort);
		/* 標示現在連線方式，斷線後重新恢復成_ETHERNET_FLOW_CTOS_ */
		ginGPRSFlow = _GPRS_FLOW_IFES_;
	}
	/* 虹堡API */
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Connect Flow: NORMAL");
			inDISP_LogPrintf(szDebugMsg);
		}
		
		/*  轉換IP表示形式 */
		inFunc_IP_Transform(szHostIP, (char*)uszIP);
		usPort = atoi(szPort);	
		inGPRS_Connect_Ex((unsigned char*)&ginTrans_ClientFd, uszIP, usPort);
		inRetVal = inGPRS_IsConnected();
		/* 標示現在連線方式，斷線後重新恢復成_ETHERNET_FLOW_CTOS_ */
		ginGPRSFlow = _GPRS_FLOW_NORMAL_;
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inGPRS_Connect_Flow() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (inRetVal);
}

/*
Function        :inGPRS_Connect_Ex
Date&Time       :2018/3/1 下午 2:06
Describe        :GPRS連線(不含SSL)未測試
*/
int inGPRS_Connect_Ex(unsigned char *uszSocket, unsigned char *uszHostIP, unsigned short usPort)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("理論上GPRS要搭配SSL，所以此function未完整測試");
	}
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inGPRS_Connect_Ex() START !");
	}
	
	/* 連線Timout 2秒 */
	inCOMM_TCP_SetConnectTO(30000);
	inCOMM_TCP_SetRetryCounter(10);
	
	usRetVal = CTOS_TCP_GPRSConnectEx(uszSocket, uszHostIP, usPort);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("CTOS_TCP_GPRSConnectEx OK!");
		}
	}
	else if (usRetVal == d_TCP_IO_PROCESSING)
	{
//		if (ginDebug == VS_TRUE)
//		{
//			inDISP_LogPrintf("CTOS_TCP_GPRSConnectEx IO PROCESSING!");
//		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_TCP_GPRSConnectEx Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inGPRS_Connect_Ex() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Connect_By_Native
Date&Time       :2018/3/8 下午 4:21
Describe        :原生C Socket連線
*/
int inGPRS_Connect_By_Native(char *szHostIP, char *szPort)
{
	int			inRetVal;
	int			inOption = 0;
	int			inFlags = 0;
	char			szDebugMsg[100 + 1];
	struct sockaddr_in	stAddr;			/* 放Host address的結構 */
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inGPRS_Connect_By_Native() START !");
	}
	
	/* 初始化address */
	/* clear all zero. */
	memset(&stAddr, 0x00, sizeof (stAddr));
	stAddr.sin_family = AF_INET;
	stAddr.sin_addr.s_addr = inet_addr(szHostIP);
	stAddr.sin_port = htons(atoi(szPort));

	ginTrans_ClientFd = socket(AF_INET, SOCK_STREAM, 0);
	if (ginTrans_ClientFd == -1)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Create Socket Fail");
			inDISP_LogPrintf(szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	
	/* 綁定GPRS */
	CTOS_TCP_BindToDevice(ginTrans_ClientFd, d_TCP_DEVICE_GPRS);
	
	/* 系统預設的狀態發送與接收一次為8688 Bytes(約為8.5K) 應該夠用，所以不必特別調整buffer大小 */
	
	
	/* 開關 ON */
	inOption = 1;
	/* 允許重用本地位址和埠 */
	setsockopt(ginTrans_ClientFd, SOL_SOCKET, SO_REUSEADDR, &inOption, sizeof(inOption));

	/* Linux内核中对connect的超时时间限制是75s， Soliris 9是几分钟，因此通常认为是75s到几分钟不等*/
	/* 師爺給我翻譯翻譯：不用非阻塞Timeut就是75秒，所以一定要用非阻塞 */
	/* 先獲得現在Handle的開關狀態 */
	inFlags = fcntl(ginTrans_ClientFd, F_GETFL, 0);
	/* 把非阻塞開關On起來(做or運算)，並設定回去 */
	fcntl(ginTrans_ClientFd, F_SETFL, inFlags | O_NONBLOCK);
	
	/* 連線Timout 2秒 */
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 10);

	do
	{
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "connect Errno: %d", errno);
				inDISP_LogPrintf(szDebugMsg);
				
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Connect Timeout");
				inDISP_LogPrintf(szDebugMsg);
			}
			
			return (VS_TIMEOUT);
		}
		
		inRetVal = connect(ginTrans_ClientFd, (void*)&stAddr, sizeof(stAddr));
		/* 通常只有同一台機器內Clinet連server，才有可能馬上成功 */
		if (inRetVal == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Connect Succeed imediately");
				inDISP_LogPrintf(szDebugMsg);
			}
		}
		else
		{
			
		}
		
	} while (inRetVal != 0);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inGPRS_Connect_By_Native() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Connect_TLS_Flow
Date&Time       :2017/8/2 下午 4:10
Describe        :
*/
int inGPRS_Connect_TLS_Flow(char *szHostIP, char *szPort)
{
	int	inRetVal = VS_ERROR;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inGPRS_Connect_TLS_Flow() START !");
	}
	
	/* 1.建socket */
	inRetVal = inGPRS_Connect_By_Native(szHostIP, szPort);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	/* 2.處理CTX */
	inRetVal = inTLS_Process_CTX_Flow(&guiCTX_ID);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	/* 3,處理TLS */
	inRetVal = inTLS_Process_TLS_Flow(&guiCTX_ID, &guiSSL_ID);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inGPRS_Connect_TLS_Flow() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_DisConnect_Flow
Date&Time       :2018/3/1 下午 6:24
Describe        :分流
*/
int inGPRS_DisConnect_Flow(void)
{
	int	inRetVal = VS_ERROR;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inGPRS_DisConnect_Flow() START !");
	}
	
	if (ginGPRSFlow == _GPRS_FLOW_IFES_)
	{
		inRetVal = inGPRS_DisConnect_TLS_Flow();
		if (inRetVal == VS_SUCCESS)
		{
			ginGPRSFlow = _GPRS_FLOW_NORMAL_;
		}
		else
		{
			ginGPRSFlow = _GPRS_FLOW_NORMAL_;
			return (VS_ERROR);
		}
	}
	/* 虹堡API */
	else
	{
		inRetVal = inGPRS_Disconnect((unsigned char)ginTrans_ClientFd);
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inGPRS_DisConnect_Flow() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Disconnect
Date&Time       :2018/3/1 下午 2:06
Describe        :
*/
int inGPRS_Disconnect(unsigned char uszSocket)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inGPRS_Disconnect() START !");
	}
	
	usRetVal = CTOS_TCP_GPRSDisconnect(uszSocket);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("CTOS_TCP_GPRSDisconnect OK!");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_TCP_GPRSDisconnect Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inGPRS_Disconnect() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_DisConnect_By_Native
Date&Time       :2017/8/2 下午 5:37
Describe        :
*/
int inGPRS_DisConnect_By_Native(void)
{
	int		inRetVal = VS_ERROR;
	char		szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inGPRS_DisConnect_By_Native() START !");
	}
	
	inRetVal = close(ginTrans_ClientFd);
	if (inRetVal == 0)
	{
		inRetVal= VS_SUCCESS;
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Disconnect Errno: %d", errno);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inGPRS_DisConnect_By_Native() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_DisConnect_TLS_Flow
Date&Time       :2017/8/2 下午 5:45
Describe        :
*/
int inGPRS_DisConnect_TLS_Flow(void)
{
	int	inRetVal = VS_ERROR;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inGPRS_DisConnect_TLS_Flow() START !");
	}
	
	inTLS_TLS2_Disconnect(guiSSL_ID, 0);
	
	inRetVal = inTLS_TLS2_Free(&guiSSL_ID);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	inRetVal = inTLS_CTX_Free(&guiCTX_ID);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	inRetVal = inGPRS_DisConnect_By_Native();
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inGPRS_DisConnect_TLS_Flow() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Send_Ready_Flow
Date&Time       :2017/8/1 下午 2:19
Describe        :分流
*/
int inGPRS_Send_Ready_Flow()
{
	int	inRetVal = VS_ERROR;
	
	if (ginGPRSFlow == _GPRS_FLOW_IFES_)
	{
		inRetVal = inGPRS_Send_Ready_TLS(guiSSL_ID);
	}
	else
	{
		inRetVal = inGPRS_Send_Ready();
	}
	
	return (inRetVal);
}

/*
Function        :inGPRS_Send_Ready
Date&Time       :2017/7/18 下午 5:28
Describe        :表示現在GPRS可以傳送資料
*/
int inGPRS_Send_Ready()
{
	unsigned int	uiStatus = 0;
	unsigned char	uszEnable = VS_TRUE;
	
	if (inGPRS_GetSocketStatus(&uiStatus, (unsigned char)ginTrans_ClientFd) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ESTABLISHED)
	{
		
	}
	else
	{
		inDISP_LogPrintf("SendReady : Not ESTABLISHED");
		uszEnable = VS_TRUE;
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_CONNECTED)
	{
		
	}
	else
	{
		inDISP_LogPrintf("SendReady : Not CONNECTED");
		uszEnable = VS_TRUE;
	}
		
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_SENDING)
	{
		inDISP_LogPrintf("SendReady : SENDING");
		uszEnable = VS_TRUE;
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_RECEIVING)
	{
		inDISP_LogPrintf("SendReady : RECEIVING");
		uszEnable = VS_TRUE;
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_DISCONNECTING)
	{
		inDISP_LogPrintf("SendReady : DISCONNECTING");
		uszEnable = VS_TRUE;
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ONHOOKING)
	{
		
	}
	else
	{
		inDISP_LogPrintf("SendReady : ONHOOKING");
		uszEnable = VS_TRUE;
	}
	
	if (uszEnable == VS_TRUE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inGPRS_Send_Ready_TLS
Date&Time       :2017/8/2 下午 4:56
Describe        :表示現在TLS可以傳送資料
*/
int inGPRS_Send_Ready_TLS(unsigned int uiSSL_ID)
{
	int	inRetVal = VS_ERROR;
	
	inRetVal = inTLS_TLS2_GetSession(uiSSL_ID);
	
	return (inRetVal);
}

/*
Function        :inGPRS_Send_Data_Flow
Date&Time       :2017/8/1 下午 2:19
Describe        :分流
*/
int inGPRS_Send_Data_Flow(unsigned char* uszData, unsigned short usLen)
{
	int	inRetVal = VS_ERROR;
	
	if (ginGPRSFlow == _GPRS_FLOW_IFES_)
	{
		inRetVal = inGPRS_Send_Data_TLS(guiSSL_ID, uszData, usLen);
	}
	else
	{
		inRetVal = inGPRS_Send_Data((unsigned char)ginTrans_ClientFd, uszData, usLen);
	}
	
	return (inRetVal);
}

/*
Function        :inGPRS_Send_Data
Date&Time       :2017/7/18 下午 5:28
Describe        :傳送資料
*/
int inGPRS_Send_Data(unsigned char uszSocket, unsigned char* uszData, unsigned short usLen)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_TCP_GPRSTx(uszSocket, uszData, usLen);
	if (usRetVal == d_OK)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_GPRSTxData Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Send_Data_TLS
Date&Time       :2017/8/2 下午 5:03
Describe        :傳送資料
*/
int inGPRS_Send_Data_TLS(unsigned int uiSSL_ID, unsigned char* uszData, unsigned short usLen)
{
	int	inRetVal = VS_ERROR;
	int	inDataLen = 0;
	
	inDataLen = usLen;
	
	inRetVal = inTLS_Send_Data(uiSSL_ID, uszData, &inDataLen);
	
	usLen = (unsigned short)inDataLen;
	
	return (inRetVal);
}

/*
Function        :inGPRS_Receive_Ready_Flow
Date&Time       :2018/3/2 上午 11:19
Describe        :目前只有虹堡API才有receive check的功能，用native沒辦法實做，所以決定不使用
 */
int inGPRS_Receive_Ready_Flow(unsigned short *usLen)
{
	int		inRetVal;
	unsigned short	usBufferMaxLen = 0;

	if (ginGPRSFlow == _GPRS_FLOW_IFES_)
	{
		inRetVal = inGPRS_Receive_Ready_TLS(guiSSL_ID, usLen);
	}
	else
	{
		usBufferMaxLen = *usLen;
		inRetVal = inGPRS_Receive_Ready(usLen);
		if (*usLen > usBufferMaxLen)
		{
			*usLen = usBufferMaxLen;
		}
	}
	
	return (inRetVal);
}

/*
Function        :inGPRS_Receive_Ready
Date&Time       :2017/7/18 下午 5:57
Describe        :表示現在GPRS可以接收資料
*/
int inGPRS_Receive_Ready(unsigned short *usLen)
{
	unsigned int	uiStatus = 0;
	unsigned char	uszEnable = VS_TRUE;
	
	if (inGPRS_GetSocketStatus(&uiStatus, (unsigned char)ginTrans_ClientFd) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ESTABLISHED)
	{
		
	}
	else
	{
		inDISP_LogPrintf("SendReady : Not ESTABLISHED");
		uszEnable = VS_TRUE;
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_CONNECTED)
	{
		
	}
	else
	{
		inDISP_LogPrintf("SendReady : Not CONNECTED");
		uszEnable = VS_TRUE;
	}
		
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_SENDING)
	{
		inDISP_LogPrintf("SendReady : SENDING");
		uszEnable = VS_TRUE;
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_RECEIVING)
	{
		inDISP_LogPrintf("SendReady : RECEIVING");
		uszEnable = VS_TRUE;
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_DISCONNECTING)
	{
		inDISP_LogPrintf("SendReady : DISCONNECTING");
		uszEnable = VS_TRUE;
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ONHOOKING)
	{
		
	}
	else
	{
		inDISP_LogPrintf("SendReady : ONHOOKING");
		uszEnable = VS_TRUE;
	}
	
	if (uszEnable == VS_TRUE)
	{
		return (VS_ERROR);
	}
	else
	{
		/* Native沒有check功能，所以直接設大一點，等到read時再根據實際回傳讀取 */
		*usLen = _COMM_RECEIVE_MAX_LENGTH_;
		
		return (VS_SUCCESS);
	}
}

/*
Function        :inGPRS_Receive_Ready_TLS
Date&Time       :2017/8/2 下午 4:56
Describe        :表示現在TLS可以接收資料
*/
int inGPRS_Receive_Ready_TLS(unsigned int uiSSL_ID, unsigned short *usLen)
{
	int	inRetVal = VS_ERROR;
	
	inRetVal = inTLS_TLS2_GetSession(uiSSL_ID);
	if (inRetVal == VS_SUCCESS)
	{
		*usLen = _COMM_RECEIVE_MAX_LENGTH_;
	}
	else
	{
		*usLen = 0;
	}
	
	return (inRetVal);
}

/*
Function        :inGPRS_Receive_Data_Flow
Date&Time       :2017/8/1 下午 3:43
Describe        :分流
*/
int inGPRS_Receive_Data_Flow(unsigned char* uszData, unsigned short *usLen)
{
	int	inRetVal = VS_ERROR;
	
	if (ginGPRSFlow == _GPRS_FLOW_IFES_)
	{
		inRetVal = inGPRS_Receive_Data_TLS(guiSSL_ID, uszData, usLen);
	}
	else
	{
		inRetVal = inGPRS_Receive_Data((unsigned char)ginTrans_ClientFd, uszData, usLen);
	}
	
	
	return (inRetVal);
}

/*
Function        :inGPRS_Receive_Data
Date&Time       :2017/7/18 下午 6:03
Describe        :接收資料
*/
int inGPRS_Receive_Data(unsigned char uszSocket, unsigned char* uszData, unsigned short *usLen)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = 0;
	
	usRetVal = CTOS_TCP_GPRSRx(uszSocket, uszData, usLen);
	if (usRetVal == d_OK)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_GPRSRxData Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Receive_Data_TLS
Date&Time       :2017/8/2 下午 5:03
Describe        :接收資料
*/
int inGPRS_Receive_Data_TLS(unsigned int uiSSL_ID, unsigned char* uszData, unsigned short *usLen)
{
	int	inRetVal = VS_ERROR;
	
	inRetVal = inTLS_Read_Data(uiSSL_ID, uszData, (int*)usLen);
	
	return (inRetVal);
}

/*
Function        :inGPRS_Initial
Date&Time       :2018/3/1 下午 3:38
Describe        :
*/
int inGPRS_Initial(void)
{
        /* inGPRS_Initial() START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inGPRS_Initial() START！");
        }
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SET_COMM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示<通訊設定> */

	inGPRS_CheckStatus();
	inGPRS_Open((unsigned char*)"", (unsigned char*)"internet", (unsigned char*)"", (unsigned char*)"");
	inGPRS_CheckStatus();
        
        /* inGPRS_Initial()_END */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inGPRS_Initial()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inGPRS_Begin
Date&Time       :2018/3/1 下午 3:44
Describe        :
*/
int inGPRS_Begin(TRANSACTION_OBJECT *pobtran)
{
        char    szCommIndex[2 + 1];
        char    szDebugBuf[64 + 1];

        /* inGPRS_Begin() START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inGPRS_Begin() START！");
        }
        
        /* 20230204 Miyano add 連線至CostcoPay主機時專用 */
        if(fLinkCostcoPayHost == VS_TRUE)
        {
                if (ginDebug == VS_TRUE)
                {
                        inDISP_LogPrintf("fSendToCostcoPayHost Flag_On!");
                        inDISP_LogPrintf("----------------------------------------");
                        inDISP_LogPrintf("inETHERNET_Begin()_END");
                }
                return (VS_SUCCESS);
        }
        
        memset(szCommIndex, 0x00, sizeof(szCommIndex));
        /* 從HDT內get CPT的index */
        if (inGetCommunicationIndex(szCommIndex) == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inDISP_LogPrintf("inGetCommunicationIndex Error!");
                
                return (VS_ERROR);
        }
        else
        {
                /* CPT index從0開始，但是CPTRec index從1開始，所以要減一  */
                pobtran->srBRec.inCPTIndex = atoi(szCommIndex) - 1;
        }

        if (inLoadCPTRec(pobtran->srBRec.inCPTIndex) == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugBuf, "inLoadCPTRec(%d) Error!", pobtran->srBRec.inCPTIndex);
                        inDISP_LogPrintf(szDebugBuf);
                }
                /* Load CPT失敗 */
                return (VS_ERROR);
        }

        /* inGPRS_Begin()_END */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inGPRS_Begin()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inGPRS_SetConfig
Date&Time       :2018/3/1 下午 3:45
Describe        :如果IP衝突會Connect會TimeOut
*/
int inGPRS_SetConfig(void)
{
	int	inRetVal = 0;
	int	inConnectMaxCnt = 1;		/* 連線最大重試次數(含第一次) */
	int	inConnectNowCnt = 0;		/* 連線目前重試次數 */
	char	szHostIPPrimary[16 + 1] = {0};
	char	szHostIPSecond[16 + 1] = {0};
	char	szHostPortNoPrimary[6 + 1] = {0};
	char	szHostPortNoSecond[6 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};
	
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inGPRS_SetConfig() START!");
	
        /* 20230204 Miyano add 連線至CostcoPay主機時專用 */
        if(fLinkCostcoPayHost == VS_TRUE)
        {
                return inGPRS_SetConfig_Costco();
        }      
        
	/* Set Host IP */
        memset(szHostIPPrimary,0x00,sizeof(szHostIPPrimary));
        if (inGetHostIPPrimary(szHostIPPrimary) == VS_ERROR)
        {
                /* inGetHostIPPrimary ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inDISP_LogPrintf("inGetHostIPPrimary() ERROR!!");

                return (VS_ERROR);
        }
         
	/* Set Host Port */
        memset(szHostPortNoPrimary,0x00,sizeof(szHostPortNoPrimary));
        if (inGetHostPortNoPrimary(szHostPortNoPrimary) == VS_ERROR)
        {
                /* Get HostPortNumber Primary ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inDISP_LogPrintf("inGetHostPortNoPrimary() ERROR!!");
                
                return (VS_ERROR);
        }

	/* 重置目前重試次數 */
	inConnectNowCnt = 0;
	/* 若連線失敗看是否要重試 */
	do 
	{
		if (ginDebug == VS_TRUE)
		{
			inFunc_CalculateRunTimeGlobal_Start();
		}
		
		inRetVal = inGPRS_Connect_Flow(szHostIPPrimary, szHostPortNoPrimary);
		inConnectNowCnt ++;
		
		if (ginDebug == VS_TRUE)
		{
			inFunc_WatchRunTime();
		}
		
	}while (inRetVal != VS_SUCCESS && inConnectNowCnt < inConnectMaxCnt);
        
	
	/* 第一授權主機連線成功 */
        if (inRetVal == VS_SUCCESS)
        {
                return (VS_SUCCESS);
        }
        else
	{
               /* 若第一授權主機連線不成功，改連第二授權主機 */ 
	}
	
	/* 看狀態 */
	inGPRS_CheckStatus();
	
	/* 第一授權IP失敗 嘗試第二IP */
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "First IP Conenct Fail");
		inDISP_LogPrintf(szDebugMsg);
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Try Second IP...");
		inDISP_LogPrintf(szDebugMsg);
	}
	
	inDISP_Clear_Line(_LINE_8_6_, _LINE_8_6_);
	inDISP_ChineseFont("第二組IP", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
	/* 嘗試連線第二授權主機 */
	/* 更改HostIP */
	/* Set Host IP */
        memset(szHostIPSecond,0x00,sizeof(szHostIPSecond));
        if (inGetHostIPSecond(szHostIPSecond) == VS_ERROR)
        {
                /* inGetHostIPSecond ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inDISP_LogPrintf("inGetHostIPSecond() ERROR!!");

                return (VS_ERROR);
        }
	
	/* Set Host Port */
        memset(szHostPortNoSecond, 0x00, sizeof(szHostPortNoSecond));
        if (inGetHostPortNoSecond(szHostPortNoSecond) == VS_ERROR)
        {
                /* Get HostPortNumber Primary ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inDISP_LogPrintf("inGetHostPortNoSecond() ERROR!!");
                
                return (VS_ERROR);
        }
        		
	/* 重置目前重試次數 */
	inConnectNowCnt = 0;
	/* 若連線失敗看是否要重試 */
	do 
	{
		if (ginDebug == VS_TRUE)
		{
			inFunc_CalculateRunTimeGlobal_Start();
		}
		
		inRetVal = inGPRS_Connect_Flow(szHostIPSecond, szHostPortNoSecond);
		inConnectNowCnt ++;
		
		if (ginDebug == VS_TRUE)
		{
			inFunc_WatchRunTime();
		}
		
	}while (inRetVal != VS_SUCCESS && inConnectNowCnt < inConnectMaxCnt);

	/* 第二授權主機連線成功 */
        if (inRetVal == VS_SUCCESS)
        {
                return (VS_SUCCESS);
        }
        else
	{
               /* 若第二授權主機連線不成功，直接斷線 */ 
	}
	
	/* 第二授權IP失敗 */
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Second IP Conenct Fail");
		inDISP_LogPrintf(szDebugMsg);
	}
	
	/* 到最後還失敗，斷線 */
	inGPRS_END();
	
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inGPRS_SetConfig() END!");
        
	/* 若連線成功，中間就會Return出去，若一直失敗，則最後一定回傳失敗 */
        return (VS_ERROR);
}

/*
Function        :inGPRS_Send
Date&Time       :2018/3/2 上午 10:51
Describe        :
*/
int inGPRS_Send(unsigned char *uszSendBuff, int inSendSize, int inSendTimeout)
{
        char            szDataHead[1 + 1];
	char		szDebugMsg[100 + 1];
        unsigned char   szSendData[1024 + 32];

        memset(szSendData, 0x00, sizeof(szSendData));
        memset(szDataHead, 0x00, sizeof(szDataHead));

        /* inGPRS_Send() START! */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inGPRS_Send() START!");

        /* Get HeadFormat */
        if (inGetTCPHeadFormat(szDataHead) == VS_ERROR  && fLinkCostcoPayHost != VS_TRUE)   /* 20230204 Miyano add */
        {
                /* inGetTCPHeadFormat ERROR */
                if (ginDebug == VS_TRUE)
                        inDISP_LogPrintf("inGetTCPHeadFormat Error!");
                
                return (VS_ERROR);
        }
        else if(fLinkCostcoPayHost == VS_TRUE)  /* 20230204 Miyano add */
        {
                szDataHead[0] == 'H';
        }
	
	/*	範例:Size = 1024
	 * 	H:0x04 0x00
	 *	B:0x10 0x24
	 */
        if (szDataHead[0] == 'H')
        {
		szSendData[0] = ((inSendSize & 0x0000FF00) >> 8);
		szSendData[1] = (inSendSize & 0x000000FF);
        }
        else
        {
		szSendData[0] = (inSendSize / 100) / 10 * 16 + (inSendSize / 100) % 10;
		szSendData[1] = (inSendSize % 100) / 10 * 16 + (inSendSize % 100) % 10;
        }
	
	if (ginISODebug == VS_TRUE)
	{
		inPRINT_ChineseFont("----------------------------------------",_PRT_ISO_);
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "%02X %02X :length = %d", szSendData[0], szSendData[1], inSendSize);
		inPRINT_ChineseFont_Format(szDebugMsg, "  ", 34, _PRT_ISO_);
		inPRINT_ChineseFont("----------------------------------------", _PRT_ISO_);
	}
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "%02X %02X :length = %d", szSendData[0], szSendData[1], inSendSize);
		inDISP_LogPrintf(szDebugMsg);
		inDISP_LogPrintf("----------------------------------------");
	}

        memcpy(&szSendData[2], uszSendBuff, inSendSize);

        /* 因為多放長度所以要加2Byte */
        inSendSize += 2;

	/* 有Timeout，要計時 */
	if (inSendTimeout != 0)
                inDISP_Timer_Start(_TIMER_NEXSYS_1_, inSendTimeout);

	while (1)
	{
		if (inSendTimeout != 0)
                {
                        if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
                        {
                                if (ginDebug == VS_TRUE)
                                {
                                        inDISP_LogPrintf("inGPRS_Send() TIMEOUT");
				}
				
				return (VS_ERROR);
			}
		}

		/* 如果可以Send就Send */
		if (inGPRS_Send_Ready_Flow()== VS_SUCCESS)
		{
			if (inGPRS_Send_Data_Flow(szSendData, inSendSize) != VS_SUCCESS)
			{
				continue;
			}
			else
				break;
		}
	}

        /* inGPRS_Send() END! */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inGPRS_Send() END!");

        return (VS_SUCCESS);
}

/*
Function        :inGPRS_Receive
Date&Time       :
Describe        :
*/
int inGPRS_Receive(unsigned char *uszReceiveBuff, int inReceiveSize, int inReceiveTimeout)
{
	int		inRetVal = 0;
        int     	inReceivelen = 0;			/* Comport當前收到的長度 */
        int     	inDataLength = 0;			/* 收到的資料長度(不含Head) */
	int		inExpectLength = 0;
        char    	szDataHead[1 + 1];
        char    	szDebugMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        unsigned char 	uszRawBuffer[inReceiveSize + 1];
	unsigned short 	usOutputLen = 0;			/* 目前comport可以讀取的資料長度 */
	
        /* inGPRS_Receive() START! */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inGPRS_Receive() START!");

	memset(uszRawBuffer, 0x00, sizeof(uszRawBuffer));

        /* 設定接收Timeout時間 */
	if (inReceiveTimeout != 0)
                inDISP_Timer_Start(_TIMER_NEXSYS_1_, inReceiveTimeout);

	/* 第一段先收到comport沒資料為止，或Timeout */
	while (1)
	{
		if (inReceiveTimeout != 0)
                {
                        if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
                        {
				if (ginDebug == VS_TRUE)
				{
					if (inReceiveTimeout != 0)
					{
						inDISP_LogPrintf("inGPRS_Receive TimeOut 1");
					}
					
				}
                        	return (VS_TIMEOUT);
                        }
                }
		
		/* 設定為剩餘的Buffer空間大小，才符合API的用法 */
		usOutputLen = inReceiveSize - inReceivelen;
		
		if (inGPRS_Receive_Ready_Flow(&usOutputLen) == VS_SUCCESS)
		{
			inRetVal = inGPRS_Receive_Data_Flow(&uszRawBuffer[inReceivelen], &usOutputLen);
			if (inRetVal == VS_SUCCESS)
			{
				inReceivelen = inReceivelen + (int)usOutputLen;
			}
		}

		/* 沒有可以收的就跳出去(usOutputLen == 0代表沒有可已從buffer中讀取的資料) */
		if (inReceivelen > 0 && usOutputLen == 0)
		{
			break;
		}
	}
        
        /* Get HeadFormat */
	/*	範例:Size = 1024
	 * 	H:0x04 0x00
	 *	B:0x10 0x24
	 */
        memset(szDataHead, 0x00, sizeof(szDataHead));
        if (inGetTCPHeadFormat(szDataHead) == VS_ERROR && fLinkCostcoPayHost != VS_TRUE)
        {
                /* inGetTCPHeadFormat ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                                inDISP_LogPrintf("inGetTCPHeadFormat() ERROR!!");

                return (VS_ERROR);
        }
        else if(fLinkCostcoPayHost == VS_TRUE)  /* 20230204 Miyano add */
        {
                szDataHead[0] == 'H';
        }
	
	/* 如果有收到，就解析Head的封包長度並確認是否全收到 */
	/* 因為封包欄位佔2bytes，所以至少應該收到2byte */
	if (inReceivelen >= 2)
	{
		/* 根據Head算出預期長度 */
		if (szDataHead[0] == 'H')
		{
			inExpectLength = uszRawBuffer[0] * 256 + uszRawBuffer[1];
		}
		else
		{
			inExpectLength = ((uszRawBuffer[0] / 16 * 10 + uszRawBuffer[0] % 16 ) * 100) + (uszRawBuffer[1] / 16 * 10 + uszRawBuffer[1] % 16);
		}
		
		if (ginISODebug == VS_TRUE)
		{
			inPRINT_ChineseFont("----------------------------------------",_PRT_ISO_);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%02X %02X :length = %d", uszRawBuffer[0], uszRawBuffer[1], inExpectLength);
			inPRINT_ChineseFont_Format(szDebugMsg, "  ", 34, _PRT_ISO_);
			inPRINT_ChineseFont("----------------------------------------", _PRT_ISO_);
		}
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("----------------------------------------");
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%02X %02X :length = %d", uszRawBuffer[0], uszRawBuffer[1], inExpectLength);
			inDISP_LogPrintf(szDebugMsg);
			inDISP_LogPrintf("----------------------------------------");
		}

		
		memcpy(&uszReceiveBuff[inDataLength], &uszRawBuffer[2], inReceivelen - 2);
		/* inDataLength為目前收到的封包數 */
		inDataLength += inReceivelen - 2;
		
		/* 比對目前接收的長度(含Length)是否與主機下的長度一致(inDataLength = inExpectLength)，若小於應接收長度則繼續接收剩下的封包 */
		while (inDataLength < inExpectLength)
		{
			/* 準備收下一次，清空暫存Buffer */
			inReceivelen = 0;
			memset(uszRawBuffer, 0x00, sizeof(uszRawBuffer));
			
                        while (1)
                        {
				/* Timeout的話 */
                                if (inReceiveTimeout != 0)
                                {
                                        if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
                                        {
						if (ginDebug == VS_TRUE)
						{
							if (inReceiveTimeout != 0)
							{
								inDISP_LogPrintf("inGPRS_Receive TimeOut 2");
							}

							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "Receve Len: %d", inDataLength);
							inDISP_LogPrintf(szDebugMsg);
						}
                                                return (VS_TIMEOUT);
                                        }
                                }

				/* 設定為剩餘的Buffer空間大小，才符合API的用法 */
				usOutputLen = inReceiveSize - inReceivelen;
				
				if (inGPRS_Receive_Ready_Flow(&usOutputLen) == VS_SUCCESS)
				{
					inRetVal = inGPRS_Receive_Data_Flow(&uszRawBuffer[inReceivelen], &usOutputLen);
					if (inRetVal == VS_SUCCESS)
					{
						inReceivelen = inReceivelen + (int)usOutputLen;
					}
				}

				/* 沒有可以收的就跳出去(usOutputLen == 0代表沒有可已從buffer中讀取的資料) */
				if (inReceivelen > 0 && usOutputLen == 0)
				{
					break;
				}
                        }

                        memcpy(&uszReceiveBuff[inDataLength], &uszRawBuffer[0], inReceivelen);
                        inDataLength += inReceivelen;
		}

	}
	/* 連Head都沒收到 */
	else
	{
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "inReceivelen ERROR!! :(inReceivelen = %d)", inReceivelen);
                        inDISP_LogPrintf(szDebugMsg);
                }

		return (VS_ERROR);
	}
	     
        /* inGPRS_Receive() END! */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inGPRS_Receive() END!");

	return (inDataLength);
}

/*
Function        :inGPRS_END
Date&Time       :
Describe        :
*/
int inGPRS_END(void)
{
        /* inGPRS_END() START! */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inGPRS_END() START!");
	
	if (inGPRS_DisConnect_Flow() != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("inGPRS_Disconnect() == VS_ERROR");
		return (VS_ERROR);
	}

        /* inGPRS_END() END! */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inGPRS_END() END!");

        return (VS_SUCCESS);
}

/*
Function        :inGPRS_Flush
Date&Time       :2018/3/2 上午 9:22
Describe        :
*/
int inGPRS_Flush(void)
{
        /* inGPRS_Flush() START! */
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inGPRS_Flush() START!");
	
        /* inGPRS_Flush() END! */
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inGPRS_Flush() END!");

        return (VS_SUCCESS);
}

/*
Function        :inGPRS_DeInitial
Date&Time       :
Describe        :
*/
int inGPRS_DeInitial(void)
{
	int	inRetval = 0;
        
        /* inGPRS_DeInitial() START! */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inGPRS_DeInitial() START!");

	inRetval = inGPRS_Close();
	if (inRetval != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inGPRS_DeInitial Failed");
		}
	}
	
        /* inGPRS_DeInitial() END! */
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inGPRS_DeInitial() END!");

        return (VS_SUCCESS);
}

/*
Function        :inGPRS_Status
Date&Time       :2018/3/2 上午 9:50
Describe        :
*/
int inGPRS_GetStatus(unsigned int *uiStatus)
{
	char		szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1] = {0};   /* debug message */
	unsigned short  usRetval = 0;
	
	if (uiStatus == NULL)
		return (VS_ERROR);
	
	do
	{
		usRetval = CTOS_TCP_GPRSStatus(uiStatus);
	
		if (usRetval == d_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("CTOS_TCP_GPRSStatus OK");
			}
			break;
		}
		else if (usRetval == d_TCP_IO_PROCESSING)
		{
//			if (ginDebug == VS_TRUE)
//			{
//				inDISP_LogPrintf("CTOS_TCP_GPRSStatus IOProcessing");
//			}
		}
		else
		{
			/* debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "CTOS_TCP_GPRSStatus Error : 0x%04x", usRetval);
				inDISP_LogPrintf(szErrorMsg);
			}
			return (VS_ERROR);
		}
		
	} while (usRetval == d_TCP_IO_PROCESSING);
	
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_CheckStatus
Date&Time       :2018/3/1 下午 3:59
Describe        :
*/
int inGPRS_CheckStatus(void)
{
	unsigned int	uiStatus = 0;

	/* Get the status of the GPRS */
	if (inGPRS_GetStatus(&uiStatus) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ESTABLISHED)
	{
		inDISP_LogPrintf("GPRSStatus : ESTABLISHED");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_CONNECTED)
	{
		inDISP_LogPrintf("GPRSStatus : CONNECTED");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ESTABLISHING)
	{
		inDISP_LogPrintf("GPRSStatus : ESTABLISHING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_CONNECTING)
	{
		inDISP_LogPrintf("GPRSStatus : CONNECTING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_SENDING)
	{
		inDISP_LogPrintf("GPRSStatus : SENDING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_RECEIVING)
	{
		inDISP_LogPrintf("GPRSStatus : RECEIVING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_DISCONNECTING)
	{
		inDISP_LogPrintf("GPRSStatus : DISCONNECTING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ONHOOKING)
	{
		inDISP_LogPrintf("GPRSStatus : ONHOOKING");
	}
  
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_GetSocketStatus
Date&Time       :2018/3/2 上午 9:50
Describe        :
*/
int inGPRS_GetSocketStatus(unsigned int *uiStatus, unsigned char uszSocket)
{
	char		szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1] = {0};   /* debug message */
	unsigned short  usRetval = 0;
	
	if (uiStatus == NULL)
		return (VS_ERROR);
	
	usRetval = CTOS_TCP_GPRSSocketStatus(uszSocket, uiStatus);
	
	if (usRetval != d_OK)
	{
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "CTOS_TCP_GPRSSocketStatus Error : 0x%04x", usRetval);
                        inDISP_LogPrintf(szErrorMsg);
                }
	}
	
	inGPRS_CheckSocketStatus(uszSocket);
	
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_CheckSocketStatus
Date&Time       :2018/3/2 上午 10:04
Describe        :
*/
int inGPRS_CheckSocketStatus(unsigned char uszSocket)
{
	unsigned int	uiStatus = 0;

	/* Get the status of the GPRS */
	inGPRS_GetSocketStatus(&uiStatus, uszSocket);

	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ESTABLISHED)
	{
		inDISP_LogPrintf("GPRSStatus : ESTABLISHED");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_CONNECTED)
	{
		inDISP_LogPrintf("GPRSStatus : CONNECTED");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ESTABLISHING)
	{
		inDISP_LogPrintf("GPRSStatus : ESTABLISHING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_CONNECTING)
	{
		inDISP_LogPrintf("GPRSStatus : CONNECTING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_SENDING)
	{
		inDISP_LogPrintf("GPRSStatus : SENDING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_RECEIVING)
	{
		inDISP_LogPrintf("GPRSStatus : RECEIVING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_DISCONNECTING)
	{
		inDISP_LogPrintf("GPRSStatus : DISCONNECTING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ONHOOKING)
	{
		inDISP_LogPrintf("GPRSStatus : ONHOOKING");
	}
  
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_NCCCTMS_Check
Date&Time       :2017/7/19 上午 11:33
Describe        :
*/
int inGPRS_NCCCTMS_Check(void)
{
        char    szTMSIPAddress[16 + 1];
        char    szTMSPortNum[6 + 1];

        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inGPRS_NCCCTMS_Check() START!");
        
	/* Set TMS IP */
        memset(szTMSIPAddress,0x00,sizeof(szTMSIPAddress));
        if (inGetTMSIPAddress(szTMSIPAddress) == VS_ERROR)
        {
                /* inGetHostIPPrimary ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inDISP_LogPrintf("inGetTMSIPAddress() ERROR!!");

                return (VS_ERROR);
        }
        
	/* Set TMS Port */
        memset(szTMSPortNum,0x00,sizeof(szTMSPortNum));
        if (inGetTMSPortNum(szTMSPortNum) == VS_ERROR)
        {
                /* Get HostPortNumber Primary ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inDISP_LogPrintf("inGetTMSPortNum() ERROR!!");
                
                return (VS_ERROR);
        }

	if (inGPRS_Connect_Flow(szTMSIPAddress, szTMSPortNum) != VS_SUCCESS)
	{
		if (inGPRS_DisConnect_Flow() != VS_SUCCESS)
                {
                        if (ginDebug == VS_TRUE)
                                inDISP_LogPrintf("inGPRS_DisConnect_Flow Error");
                        
                        return (VS_ERROR);
                }
		return (VS_ERROR);
	}

        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inGPRS_NCCCTMS_Check() END!");
        
        return (VS_SUCCESS);
}

/*
Function        :inGPRS_IsConnected
Date&Time       :2018/3/2 下午 2:39
Describe        :
*/
int inGPRS_IsConnected(void)
{
	unsigned int	uiStatus = 0;
	unsigned char	uszFlag = VS_FALSE;

	/* Get the status of the GPRS */
	if (inGPRS_GetStatus(&uiStatus) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ESTABLISHED)
	{
		inDISP_LogPrintf("GPRSStatus : ESTABLISHED");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_CONNECTED)
	{
		inDISP_LogPrintf("GPRSStatus : CONNECTED");
		uszFlag = VS_TRUE;
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ESTABLISHING)
	{
		inDISP_LogPrintf("GPRSStatus : ESTABLISHING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_CONNECTING)
	{
		inDISP_LogPrintf("GPRSStatus : CONNECTING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_SENDING)
	{
		inDISP_LogPrintf("GPRSStatus : SENDING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_RECEIVING)
	{
		inDISP_LogPrintf("GPRSStatus : RECEIVING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_DISCONNECTING)
	{
		inDISP_LogPrintf("GPRSStatus : DISCONNECTING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ONHOOKING)
	{
		inDISP_LogPrintf("GPRSStatus : ONHOOKING");
	}
  
	if (uszFlag == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

/*
Function        :
Date&Time       :
Describe        :
*/
int inGPRS_Test(void)
{
	inGPRS_Open((unsigned char*)"", (unsigned char*)"internet", (unsigned char*)"", (unsigned char*)"");
	inGPRS_CheckStatus();
	
	return (VS_SUCCESS);
}

int inGPRS_SetConfig_Costco(void)
{
        int		inRetVal = 0;
	int		inConnectMaxCnt = 2;		/* 連線最大重試次數(含第一次) */
	int		inConnectNowCnt = 0;		/* 連線目前重試次數 */
	char		szHostIPPrimary[16 + 1] = {0};
	char		szHostPortNoPrimary[6 + 1] = {0};

        
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("fSendToCostcoPayHost Flag_On!");
        }
        
        if(inGetTSP_IP(szHostIPPrimary) != VS_SUCCESS)
        {
                inDISP_LogPrintf("inGetTSP_IP Error!");
                return (VS_ERROR);
        }
        
        if(inGetTSP_Port(szHostPortNoPrimary) != VS_SUCCESS)
        {
                inDISP_LogPrintf("inGetTSP_IP Error!");
                return (VS_ERROR);
        }
        
        /* 重置目前重試次數 */
	inConnectNowCnt = 0;
   
	/* 若連線失敗看是否要重試 */
    	do 
	{
		if (ginDebug == VS_TRUE)
		{
			inFunc_CalculateRunTimeGlobal_Start();
		}
		
		inRetVal = inGPRS_Connect_Flow(szHostIPPrimary, szHostPortNoPrimary);
		inConnectNowCnt ++;
		
		if (ginDebug == VS_TRUE)
		{
			inFunc_WatchRunTime();
		}
		
	}while (inRetVal != VS_SUCCESS && inConnectNowCnt < inConnectMaxCnt);
        
        /* 連線成功 */
	if (inRetVal != VS_SUCCESS)
	{
		inGPRS_END();
                return (VS_ERROR);
	}
	return VS_SUCCESS;
}

