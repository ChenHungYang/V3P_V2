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
#include "../DISPLAY/DisTouch.h"
#include "../DISPLAY/DispMsg.h"
#include "../PRINT/Print.h"
#include "../FUNCTION/Function.h"
#include "../FUNCTION/Sqlite.h"
#include "../EVENT/MenuMsg.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/HDPT.h"
#include "../FUNCTION/CPT.h"
#include "../FUNCTION/EDC.h"

#include "../TMS/TMSTABLE/TmsCPT.h"

#include "../FUNCTION/UNIT_FUNC/TimeUint.h"

#include "Comm.h"
#include "TLS.h"
#include "Ethernet.h"

#include "../FUNCTION/COSTCO_FUNC/Costco.h"

extern  int		ginDebug;		/* Debug使用 extern */
extern  int		ginISODebug;
extern	int		ginTrans_ClientFd;
extern	unsigned int	guiCTX_ID;		/* TLS Context ID */
extern	unsigned int	guiSSL_ID;		/* TLS Session ID */
extern  VS_BOOL         fLinkCostcoPayHost;     /* CostcoPay主機專用 */

int	ginEthernetFlow = -1;			/* (1:TLS 2:Native 3:CTOS)紀錄Ethernet走哪個流程，用這個紀錄是因為下TMS有可能修改到CFGT，所以IFES Mode可能會被改到，導致無法斷線 */

static int st_RecReceiveLog = VS_TRUE;

/*
Function        :inETHERNET_Open
Date&Time       :2017/7/18 下午 3:36
Describe        :
*/
int inETHERNET_Open()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inETHERNET_Open() START !");
	}
	
	usRetVal = CTOS_EthernetOpen();
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("ETHERNET_OPEN OK!");
		}
	}
	else if (usRetVal == d_ETHERNET_ALREADY_OPENED)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ETHERNET_ALREADY_OPENED");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_EthernetOpen Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inETHERNET_Open() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Close
Date&Time       :2017/7/19 上午 11:21
Describe        :
*/
int inETHERNET_Close()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inETHERNET_Close() START !");
	}
	
	usRetVal = CTOS_EthernetClose();
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("ETHERNET_CLOSE OK!");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_EthernetClose Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inETHERNET_Close() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Cofig_Set
Date&Time       :2017/7/18 下午 3:45
Describe        :
*/
int inETHERNET_Cofig_Set(unsigned char uszTag, unsigned char *uszValue, unsigned char uszLen)
{
	unsigned short	usRetVal = 0x00;
	
	usRetVal = CTOS_EthernetConfigSet(uszTag, uszValue, uszLen);
	if (usRetVal == d_OK)
	{

	}
	else
	{
		inDISP_DispLogAndWriteFlie(" CTOS_EthernetConfigSet *Error* [0x%04X] Line[%d]",usRetVal,  __LINE__);
		inDISP_DispLogAndWriteFlie(" Tag: %d Len:%d Value:%s Line[%d]", uszTag, uszLen, uszValue, __LINE__);
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Cofig_Get
Date&Time       :2018/5/15 下午 5:26
Describe        :
*/
int inETHERNET_Cofig_Get(unsigned char uszTag, unsigned char *uszValue, unsigned char *uszLen)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal;
	
	usRetVal = CTOS_EthernetConfigGet(uszTag, uszValue, uszLen);
	if (usRetVal == d_OK)
	{
                
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_EthernetConfigGet Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Tag: %d Len:%d Value:%s", uszTag, *uszLen, uszValue);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Connect_Flow
Date&Time       :2017/7/18 下午 4:53
Describe        :分流使用CTOS還是使用原生C的Connect
*/
int inETHERNET_Connect_Flow(char *szHostIP, char *szPort)
{
	int	inRetVal = VS_ERROR;
	char	szI_FES_Mode[2 + 1];
	char	szTRTFileName[12 + 1];
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inETHERNET_Connect_Flow() START !");

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
                    inDISP_DispLogAndWriteFlie("Connect Flow: TLS Line[%d]", __LINE__);

                    inRetVal = inETHERNET_Connect_TLS_Flow(szHostIP, szPort);
		/* 標示現在連線方式，斷線後重新恢復成_ETHERNET_FLOW_CTOS_ */
		ginEthernetFlow = _ETHERNET_FLOW_IFES_;
	}
	/* 用 native方式建Socket，之後要優化再考慮使用(優點自己決定Timeout等等細微設定) */
	else if (1)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Connect Flow: Native");
			inDISP_LogPrintf(szDebugMsg);
		}
                    
                    inDISP_DispLogAndWriteFlie("Connect Flow: Native Line[%d]",  __LINE__);	
		inRetVal = inETHERNET_Connect_By_Native(szHostIP, szPort);
		/* 標示現在連線方式，斷線後重新恢復成_ETHERNET_FLOW_CTOS_ */
		ginEthernetFlow = _ETHERNET_FLOW_NATIVE_;

	}
	/* 虹堡API */
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Connect Flow: CTOS");
			inDISP_LogPrintf(szDebugMsg);
		}
		inDISP_DispLogAndWriteFlie("Connect Flow: CTOS Line[%d]",  __LINE__);	
		inRetVal = inETHERNET_Connect();
		/* 標示現在連線方式，斷線後重新恢復成_ETHERNET_FLOW_CTOS_ */
		ginEthernetFlow = _ETHERNET_FLOW_CTOS_;
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inETHERNET_Connect_Flow() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (inRetVal);
}

/*
Function        :inETHERNET_Connect
Date&Time       :2017/7/18 下午 4:53
Describe        :ctos api
*/
int inETHERNET_Connect(void)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inETHERNET_Connect() START !");
	}
	
	usRetVal = CTOS_EthernetConnect();
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("CTOS_EthernetConnect OK!");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_EthernetConnect Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inETHERNET_Connect() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Connect_By_Native
Date&Time       :2017/7/31 下午 4:31
Describe        :原生C Socket連線
*/
int inETHERNET_Connect_By_Native(char *szHostIP, char *szPort)
{
	int			inRetVal;
	int			inOption = 0;
	int			inFlags = 0;
	struct sockaddr_in	stAddr;			/* 放Host address的結構 */
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inETHERNET_Connect_By_Native() START !");
	}
	
	/* 初始化address */
	memset(&stAddr, 0x00, sizeof (stAddr)); //clear all zero.
	stAddr.sin_family = AF_INET;
	stAddr.sin_addr.s_addr = inet_addr(szHostIP);
	stAddr.sin_port = htons(atoi(szPort));

	ginTrans_ClientFd = socket(AF_INET, SOCK_STREAM, 0);
	if (ginTrans_ClientFd == -1)
	{		
		inDISP_DispLogAndWriteFlie(" ETH_NAT Socket ClintFd -1 *Error* Line[%d]",  __LINE__);
		return (VS_ERROR);
	}
	
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

    /* 新增條件，玉山自助不需要重試，但需要改建立連線的總時長為30秒 
     * 而重新建立連線時間改為每0.5秒一次 2021/9/14 上午 10:42 [SAM] */    
#if (_MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_) && (defined(_ESUN_MAIN_HOST_) )
        inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
        do
	{
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" ETH_NAT connect Errno[%d]  Timeout *Error* Line[%d]", errno, __LINE__);
			return (VS_TIMEOUT);
		}
		
		inRetVal = connect(ginTrans_ClientFd, (void*)&stAddr, sizeof(stAddr));
		/* 通常只有同一台機器內Clinet連server，才有可能馬上成功 */
		if (inRetVal == 0)
		{
			inDISP_DispLogAndWriteFlie(" ETH_NAT Connect Succeed imediately err[%d] Line[%d]",  errno,__LINE__);
		}
		else
		{
                            /*  因為不要重建通訊太頻繁，會有多封包產生，設定如果Connect 失敗 0.5秒重建一次  */
			inDISP_Wait(500);
		}
		
	} while (inRetVal != 0);
        
#else	
	/* 連線Timout 2秒 */
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 2);
    
        do
	{
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" ETH_NAT connect Errno[%d]  Timeout *Error* Line[%d]", errno, __LINE__);
			return (VS_TIMEOUT);
		}
		
		inRetVal = connect(ginTrans_ClientFd, (void*)&stAddr, sizeof(stAddr));
		/* 通常只有同一台機器內Clinet連server，才有可能馬上成功 */
		if (inRetVal == 0)
		{
			inDISP_DispLogAndWriteFlie(" ETH_NAT Connect Succeed immediately Line[%d]",  __LINE__);
		}
		else
		{
			
		}
		
	} while (inRetVal != 0);
#endif
    
	

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inETHERNET_Connect_By_Native() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Connect_TLS_Flow
Date&Time       :2017/8/2 下午 4:10
Describe        :
*/
int inETHERNET_Connect_TLS_Flow(char *szHostIP, char *szPort)
{
	int	inRetVal = VS_ERROR;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inETHERNET_Connect_TLS_Flow() START !");
	}
	
	/* 1.建socket */
	inRetVal = inETHERNET_Connect_By_Native(szHostIP, szPort);
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
		inDISP_LogPrintf("inETHERNET_Connect_TLS_Flow() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
	inDISP_DispLogAndWriteFlie(" TLS Connect CTX[%lu] SSLID[%lu] Line[%d]",  guiCTX_ID, guiSSL_ID ,__LINE__);
	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_DisConnect_Flow
Date&Time       :2017/8/2 下午 5:35
Describe        :分流
*/
int inETHERNET_DisConnect_Flow()
{
	int	inRetVal = VS_ERROR;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inETHERNET_DisConnect_Flow() START !");
	}
	
	if (ginEthernetFlow == _ETHERNET_FLOW_IFES_)
	{
		inRetVal = inETHERNET_DisConnect_TLS_Flow();
		if (inRetVal == VS_SUCCESS)
		{
			ginEthernetFlow = _ETHERNET_FLOW_CTOS_;
		}
		else
		{
			return (VS_ERROR);
		}
	}
	/* 用native方式建Socket，之後要優化再考慮使用(優點自己決定Timeout等等細微設定) */
	else if (ginEthernetFlow == _ETHERNET_FLOW_NATIVE_)
	{
		inRetVal = inETHERNET_DisConnect_By_Native();
		if (inRetVal == VS_SUCCESS)
		{
			ginEthernetFlow = _ETHERNET_FLOW_CTOS_;
		}
	}
	/* 虹堡API */
	else
	{
		inRetVal = inETHERNET_DisConnect();
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inETHERNET_DisConnect_Flow() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_DisConnect
Date&Time       :2017/7/19 上午 11:30
Describe        :
*/
int inETHERNET_DisConnect()
{
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inETHERNET_DisConnect() START !");
	}
	
	usRetVal = CTOS_EthernetDisconnect();
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("CTOS_EthernetDisconnect OK!");
		}
	}
	else
	{
		inDISP_DispLogAndWriteFlie(" ETH_CTOS DisConnect RVal[0x%04X]  *Error* Line[%d]", usRetVal, __LINE__);
		return (VS_ERROR);
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inETHERNET_DisConnect() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_DisConnect_By_Native
Date&Time       :2017/8/2 下午 5:37
Describe        :
*/
int inETHERNET_DisConnect_By_Native()
{
	int		inRetVal = VS_ERROR;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inETHERNET_DisConnect_By_Native() START !");
	}
    
	inRetVal = close(ginTrans_ClientFd);
	if (inRetVal == 0)
	{
                    
		inRetVal= VS_SUCCESS;
	}
	else
	{
                   inDISP_DispLogAndWriteFlie(" ETH_NAT DisConnect RVal[%d]  ErNo[%d] *Error* Line[%d]", inRetVal, errno, __LINE__);
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inETHERNET_DisConnect_By_Native() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_DisConnect_TLS_Flow
Date&Time       :2017/8/2 下午 5:45
Describe        :
*/
int inETHERNET_DisConnect_TLS_Flow()
{
	int	inRetVal = VS_ERROR;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inETHERNET_DisConnect_TLS_Flow() START !");
	}
        
        inDISP_DispLogAndWriteFlie(" TLS DisConnect CTX[%lu] SSLID[%lu] Line[%d]",  guiCTX_ID, guiSSL_ID ,__LINE__);	
    
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
	
	inRetVal = inETHERNET_DisConnect_By_Native();
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
         inDISP_DispLogAndWriteFlie("inETHERNET_DisConnect_TLS_Flow() END");
    
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inETHERNET_DisConnect_TLS_Flow() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Send_Ready_Flow
Date&Time       :2017/8/1 下午 2:19
Describe        :分流
*/
int inETHERNET_Send_Ready_Flow()
{
	int	inRetVal = VS_ERROR;
	
	if (ginEthernetFlow == _ETHERNET_FLOW_IFES_)
	{
		inRetVal = inETHERNET_Send_Ready_TLS(guiSSL_ID);
	}
	/* 用native方式建Socket，之後要優化再考慮使用(優點自己決定Timeout等等細微設定) */
	else if (ginEthernetFlow == _ETHERNET_FLOW_NATIVE_)
	{
		inRetVal = inETHERNET_Send_Ready_By_Native(ginTrans_ClientFd);
	}
	/* 虹堡API */
	else
	{
		inRetVal = inETHERNET_Send_Ready();
	}
	
	return (inRetVal);
}

/*
Function        :inETHERNET_Send_Ready
Date&Time       :2017/7/18 下午 5:28
Describe        :表示現在Ethernet可以傳送資料
*/
int inETHERNET_Send_Ready()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_EthernetTxReady();
	if (usRetVal == d_OK)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inETHERNET_Send_Ready Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Send_Ready_By_Native
Date&Time       :2017/8/1 下午 1:40
Describe        :
*/
int inETHERNET_Send_Ready_By_Native(int inFileHandle)
{
	fd_set		stWfd;			/* 用來判斷是否可以寫的Fd */
	struct timeval	stTimeVal;
	
	stTimeVal.tv_sec = 0;
	stTimeVal.tv_usec = 0;
	
	/* 將 set 整個清為零。 */
	FD_ZERO(&stWfd);
	/* 將 fd 新增到 set。 */
	FD_SET(inFileHandle, &stWfd);
	
	
	/* nfds is the highest-numbered file descriptor in any of the three sets, plus 1. FD最高位再加一 */
	/* 若Timeout仍未有Fd，則回傳0 */
	select(inFileHandle + 1, NULL, &stWfd, NULL, &stTimeVal);
	if (FD_ISSET(inFileHandle, &stWfd))
	{
		
	}
	else
	{
                    inDISP_DispLogAndWriteFlie(" Native Send Ready *Error* error[%d] Line[%d]", errno, __LINE__);
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Send_Ready_TLS
Date&Time       :2017/8/2 下午 4:56
Describe        :表示現在TLS可以傳送資料
*/
int inETHERNET_Send_Ready_TLS(unsigned int uiSSL_ID)
{
	int	inRetVal = VS_ERROR;
	
	inRetVal = inTLS_TLS2_GetSession(uiSSL_ID);
	
	return (inRetVal);
}

/*
Function        :inETHERNET_Send_Data_Flow
Date&Time       :2017/8/1 下午 2:19
Describe        :分流
*/
int inETHERNET_Send_Data_Flow(unsigned char* uszData, unsigned short usLen)
{
	int	inRetVal = VS_ERROR;
	
	if (ginEthernetFlow == _ETHERNET_FLOW_IFES_)
	{
		inRetVal = inETHERNET_Send_Data_TLS(guiSSL_ID, uszData, usLen);
	}
	/* 用native方式建Socket，之後要優化再考慮使用(優點自己決定Timeout等等細微設定) */
	else if (ginEthernetFlow == _ETHERNET_FLOW_NATIVE_)
	{
		inRetVal = inETHERNET_Send_Data_By_Native(ginTrans_ClientFd, uszData, usLen);
	}
	else
	{
		inRetVal = inETHERNET_Send_Data(uszData, usLen);
	}
	
	return (inRetVal);
}

/*
Function        :inETHERNET_Send_Data
Date&Time       :2017/7/18 下午 5:28
Describe        :傳送資料
*/
int inETHERNET_Send_Data(unsigned char* uszData, unsigned short usLen)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_EthernetTxData(uszData, usLen);
	if (usRetVal == d_OK)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_EthernetTxData Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Send_Data_By_Native
Date&Time       :2017/8/1 下午 2:28
Describe        :傳送資料
*/
int inETHERNET_Send_Data_By_Native(int inFileHandle, unsigned char* uszData, unsigned short usLen)
{
	int	inTempLen = 0;
	char	szDebugMsg[100 + 1];
	
	/* Send message */  
	inTempLen = send(inFileHandle, uszData, usLen, 0);
	
	if (inTempLen >= 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "send Errno: %d", errno);
			inDISP_LogPrintf(szDebugMsg);
		}
		inDISP_DispLogAndWriteFlie(" Native Send Data *Error* err[%d] len[%d] Line[%d]", errno, inTempLen, __LINE__);
		return (VS_ERROR);
	}
	
}

/*
Function        :inETHERNET_Send_Data_TLS
Date&Time       :2017/8/2 下午 5:03
Describe        :傳送資料
*/
int inETHERNET_Send_Data_TLS(unsigned int uiSSL_ID, unsigned char* uszData, unsigned short usLen)
{
	int	inRetVal = VS_ERROR;
	int	inDataLen = 0;
	
	inDataLen = usLen;
	
	inRetVal = inTLS_Send_Data(uiSSL_ID, uszData, &inDataLen);
	
	usLen = (unsigned short)inDataLen;
	
	return (inRetVal);
}

/*
Function        :inETHERNET_Receive_Ready_Flow
Date&Time       :2017/8/1 下午 2:47
Describe        :目前只有虹堡API才有receive check的功能，用native沒辦法實做，所以決定不使用
 */
int inETHERNET_Receive_Ready_Flow(unsigned short *usLen)
{
	int		inRetVal;
	unsigned short	usBufferMaxLen = 0;

	if (ginEthernetFlow == _ETHERNET_FLOW_IFES_)
	{
		inRetVal = inETHERNET_Receive_Ready_TLS(guiSSL_ID, usLen);
	}
	/* 用native方式建Socket，之後要優化再考慮使用(優點自己決定Timeout等等細微設定) */
	else if (ginEthernetFlow == _ETHERNET_FLOW_NATIVE_)
	{
		inRetVal = inETHERNET_Receive_Ready_By_Native(ginTrans_ClientFd, usLen);
	}
	else
	{
		usBufferMaxLen = *usLen;
		inRetVal = inETHERNET_Receive_Ready(usLen);
		
		if (*usLen > usBufferMaxLen)
		{
			*usLen = usBufferMaxLen;
		}
	}
	
	return (inRetVal);
}

/*
Function        :inETHERNET_Receive_Ready
Date&Time       :2017/7/18 下午 5:57
Describe        :表示現在Ethernet可以接收資料
*/
int inETHERNET_Receive_Ready(unsigned short *usLen)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_EthernetRxReady(usLen);
	if (usRetVal == d_OK)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inETHERNET_Receive_Ready Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Receive_Ready_By_Native
Date&Time       :2017/8/1 下午 3:10
Describe        :
*/
int inETHERNET_Receive_Ready_By_Native(int inFileHandle, unsigned short *usLen)
{
	fd_set		stRfd;			/* 用來判斷是否可以讀的Fd */
	struct timeval	stTimeVal;

	stTimeVal.tv_sec = 0;
	stTimeVal.tv_usec = 0;
	
	/* 將 set 整個清為零。 */
	FD_ZERO(&stRfd);
	/* 將 fd 新增到 set。 */
	FD_SET(inFileHandle, &stRfd);
	
	/* 初始化 */
	*usLen = 0;
	/* nfds is the highest-numbered file descriptor in any of the three sets, plus 1. FD最高位再加一 */
	/* 若Timeout仍未有Fd，則回傳0 */
	select(inFileHandle + 1, &stRfd, NULL, NULL, &stTimeVal);
	if (FD_ISSET(inFileHandle, &stRfd))
	{
		/* Native沒有check功能，所以直接設大一點，等到read時再根據實際回傳讀取 */
		*usLen = _COMM_RECEIVE_MAX_LENGTH_;
		st_RecReceiveLog = VS_TRUE;
	}
	else
	{   
		if(st_RecReceiveLog == VS_TRUE)
		{
			st_RecReceiveLog = VS_FALSE;
			inDISP_DispLogAndWriteFlie(" Native Receive Ready Data *Error* err[%d] Line[%d]", errno, __LINE__);
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Receive_Ready_TLS
Date&Time       :2017/8/2 下午 4:56
Describe        :表示現在TLS可以接收資料
*/
int inETHERNET_Receive_Ready_TLS(unsigned int uiSSL_ID, unsigned short *usLen)
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
Function        :inETHERNET_Receive_Data_Flow
Date&Time       :2017/8/1 下午 3:43
Describe        :分流
*/
int inETHERNET_Receive_Data_Flow(unsigned char* uszData, unsigned short *usLen)
{
	int	inRetVal = VS_ERROR;
	
	if (ginEthernetFlow == _ETHERNET_FLOW_IFES_)
	{
		inRetVal = inETHERNET_Receive_Data_TLS(guiSSL_ID, uszData, usLen);
	}
	/* 用native方式建Socket，之後要優化再考慮使用(優點自己決定Timeout等等細微設定) */
	else if (ginEthernetFlow == _ETHERNET_FLOW_NATIVE_)
	{
		inRetVal = inETHERNET_Receive_Data_By_Native(ginTrans_ClientFd, uszData, usLen);
	}
	else
	{
		inRetVal = inETHERNET_Receive_Data(uszData, usLen);
	}
	
	
	return (inRetVal);
}

/*
Function        :inETHERNET_Receive_Data
Date&Time       :2017/7/18 下午 6:03
Describe        :接收資料
*/
int inETHERNET_Receive_Data(unsigned char* uszData, unsigned short *usLen)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_EthernetRxData(uszData, usLen);
	if (usRetVal == d_OK)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_EthernetRxData Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Receive_Data_By_Native
Date&Time       :2017/8/1 下午 3:45
Describe        :
*/
int inETHERNET_Receive_Data_By_Native(int inFileHandle, unsigned char* uszData, unsigned short *usLen)
{
	int	inTempLen = 0;

	st_RecReceiveLog = VS_TRUE;

	inTempLen = recv(inFileHandle, uszData, *usLen, 0);

	/* 如果回傳-1會變成65535，所以要做安全檢核 */
	if (inTempLen > 0)
	{
		*usLen = (unsigned short)inTempLen;
	}
	else
	{
		*usLen = 0;
		inDISP_DispLogAndWriteFlie(" Native Receive Data *Error* err[%d] TempLen[%d] Line[%d]", errno, inTempLen, __LINE__);
		return (VS_ERROR);
	}

	
	
	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Receive_Data_TLS
Date&Time       :2017/8/2 下午 5:03
Describe        :接收資料
*/
int inETHERNET_Receive_Data_TLS(unsigned int uiSSL_ID, unsigned char* uszData, unsigned short *usLen)
{
	int	inRetVal = VS_ERROR;
	st_RecReceiveLog = VS_TRUE;
	inRetVal = inTLS_Read_Data(uiSSL_ID, uszData, (int*)usLen);
	
	return (inRetVal);
}

/*
Function        :inETHERNET_Flush_Rx
Date&Time       :2017/7/19 上午 11:18
Describe        :清空Comport
*/
int inETHERNET_Flush_Rx()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_EthernetFlushRxData();
	if (usRetVal == d_OK)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_EthernetFlushRxData Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Initial
Date&Time       :2017/7/18 下午 3:37
Describe        :
*/
int inETHERNET_Initial(void)
{
	int	inRetVal = VS_ERROR;
	char	szKey = VS_ERROR;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_CUP_LOGON_;
        char	szEDCIPAddress[16 + 1];
        char	szSubMask[16 + 1];
        char	szGatWay[16 + 1];
        char	szConfig[16 + 1];
	char	szDHCPMode[2 + 1];
	
        /* inETHERNET_Initial() START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inETHERNET_Initial() START！");
        }
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SET_COMM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <通訊設定> */
	
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

        /* 打開TCP IP */
	/* Portable 機型若沒接上底座再開Ethernet會失敗 */
	/* 這邊要分是否為有底座機型，有底座要先接回才能Initial */
	if (inFunc_Is_Portable_Type() == VS_TRUE)
	{
		do
		{
			inRetVal = inETHERNET_Open();
        
			if (inRetVal == VS_SUCCESS)
			{
				break;
			}
			else
			{			
				/* 未接上底座，提示接上底座後並按確認 */
				if (inFunc_Is_Cradle_Attached() != VS_SUCCESS)
				{
					/* 請插回底座 繼續交易 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_CHECK_CRADLE_ATTATCHED_, 0, _COORDINATE_Y_LINE_8_4_);
					inDISP_BEEP(1, 0);
					
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
							
							inRetVal = VS_SUCCESS;
							break;
						}
						/* 一定要 Intial後才能進入idle，所以鎖起來 
						else if (szKey == _KEY_CANCEL_)
						{
							inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
							
							inRetVal = VS_ERROR;
						}
						*/
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
		inRetVal = inETHERNET_Open();
        
		if (inRetVal == VS_SUCCESS)
		{
			
		}
		else
		{
			inDISP_DispLogAndWriteFlie(" Inital inETHERNET_Open *Error* Line[%d]",  __LINE__);
			return (VS_ERROR);
		}
		
	}
	
	memset(szDHCPMode, 0x00, sizeof(szDHCPMode));
	inGetDHCP_Mode(szDHCPMode);
	if (memcmp(szDHCPMode, "Y", strlen("Y")) == 0)
	{
		
		 /* 因 DHCP 模式打開，所以固定設定為1 */
		szConfig[0] = 0x31;
	
		if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_DHCP, (unsigned char*)"1", 1) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" d_ETHERNET_CONFIG_DHCP *Error* Line[%d]",  __LINE__);
			return (VS_ERROR);
		}
		
		/*
			d_ETHERNET_CONFIG_UPDATE_EXIT
			Save the settings to the non-volatile memory in Ethernet module.      
		 */
		/* 儲存設定 */
		memset(szConfig, 0x00, sizeof(szConfig));
		if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_UPDATE_EXIT, (unsigned char*)szConfig, 0) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" d_ETHERNET_CONFIG_UPDATE_EXIT *Error* Line[%d]",  __LINE__);
			return (VS_ERROR);
		}
	}
	else
	{
		/* 這邊先設定IP是因為要讓Idle下可以Ping得到 */
		memset(szEDCIPAddress, 0x00, sizeof(szEDCIPAddress));
		if (inGetTermIPAddress(szEDCIPAddress) == VS_ERROR)
		{
			/* inGetEDCIP ERROR */
			inDISP_DispLogAndWriteFlie(" inGetTermIPAddress *Error* Line[%d]",  __LINE__);
			return (VS_ERROR);
		}

		/* Set EDC IP Address */
		if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_IP, (unsigned char*)szEDCIPAddress, strlen(szEDCIPAddress)) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" d_ETHERNET_CONFIG_IP *Error* Line[%d]",  __LINE__);
			return (VS_ERROR);
		}

		memset(szGatWay, 0x00, sizeof(szGatWay));
		if (inGetTermGetewayAddress(szGatWay) == VS_ERROR)
		{
			/* inGetGateWay ERROR */
			inDISP_DispLogAndWriteFlie(" inGetGateWay *Error* Line[%d]",  __LINE__);
			return (VS_ERROR);
		}

		/* Set Geteway IP */
		if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_GATEWAY, (unsigned char*)szGatWay, strlen(szGatWay)) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" d_ETHERNET_CONFIG_GATEWAY *Error* Line[%d]",  __LINE__);
			return (VS_ERROR);
		}

		memset(szSubMask, 0x00, sizeof(szSubMask));
		if (inGetTermMASKAddress(szSubMask) == VS_ERROR)
		{
			/* inGetSubMask ERROR */
			/* debug */
			inDISP_DispLogAndWriteFlie(" inGetSubMask *Error* Line[%d]",  __LINE__);

			return (VS_ERROR);
		}

		/* Set EDC SUB Mask */
		if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_MASK, (unsigned char*)szSubMask, strlen(szSubMask)) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" d_ETHERNET_CONFIG_MASK *Error* Line[%d]",  __LINE__);
			return (VS_ERROR);
		}

		
		 /* 因 DHCP 模式沒開，所以固定設定 0 */
		if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_DHCP, (unsigned char*)"0", 1) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" d_ETHERNET_CONFIG_DHCP *Error* Line[%d]",  __LINE__);
			return (VS_ERROR);
		}
		
		/*
			d_ETHERNET_CONFIG_UPDATE_EXIT
			Save the settings to the non-volatile memory in Ethernet module.      
		 */
		/* 儲存設定 */
		memset(szConfig, 0x00, sizeof(szConfig));
		if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_UPDATE_EXIT, (unsigned char*)szConfig, 0) != VS_SUCCESS)
		{
			inDISP_DispLogAndWriteFlie(" d_ETHERNET_CONFIG_UPDATE_EXIT *Error* Line[%d]",  __LINE__);
			return (VS_ERROR);
		}
		
		/* 魔法，勿動 START 
		* 這個Bug的發生時機在當機器直接斷電時，下次開機後，
		* 連線狀態會一直Timeout。
		* 正規解法是把設定IP、Submask、gateway 放到inETHERNET_SetConfig 再設定
		*/
		{
		       memset(szGatWay, 0x00, sizeof(szGatWay));
		       if (inGetTermGetewayAddress(szGatWay) == VS_ERROR)
		       {
			       /* inGetGateWay ERROR */
			       /* debug */
			       inDISP_DispLogAndWriteFlie(" inGetTermGetewayAddress *Error* Line[%d]",  __LINE__);
			       return (VS_ERROR);
		       }

		       /* Set Geteway IP */
		       if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_GATEWAY, (unsigned char*)szGatWay, strlen(szGatWay)) != VS_SUCCESS)
		       {
			       inDISP_DispLogAndWriteFlie(" d_ETHERNET_CONFIG_GATEWAY *Error* Line[%d]",  __LINE__);
			       return (VS_ERROR);
		       }

		}
		/* 魔法，勿動 End */
	}
		
        /* inETHERNET_Initial()_END */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inETHERNET_Initial()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Begin
Date&Time       :2017/7/18 下午 4:16
Describe        :
*/
int inETHERNET_Begin(TRANSACTION_OBJECT *pobtran)
{
        char    szCommIndex[2 + 1];
        char    szDebugBuf[64 + 1];

        /* inETHERNET_Begin() START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inETHERNET_Begin() START！");
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

        /* inETHERNET_Begin()_END */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inETHERNET_Begin()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inETHERNET_SetConfig
Date&Time       :2017/7/18 下午 3:14
Describe        :如果IP衝突會Connect會TimeOut
*/
int inETHERNET_SetConfig(void)
{
	int		inRetVal = 0;
	int		inConnectMaxCnt = 2;		/* 連線最大重試次數(含第一次) */
	int		inConnectNowCnt = 0;		/* 連線目前重試次數 */
	char		szHostIPPrimary[16 + 1] = {0};
	char		szHostIPSecond[16 + 1] = {0};
	char		szHostPortNoPrimary[6 + 1] = {0};
	char		szHostPortNoSecond[6 + 1] = {0};
	char		szConfig[16 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szDHCPMode[2 + 1] = {0};
	char		szAutoConnect[10 + 1] = {0};
	char		szTemplate[50 + 1] = {0};
	char		szIFESMode[2 + 1] = {0};
	unsigned char	uszLen = 0;
	
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inETHERNET_SetConfig() START!");
	
        /* 20230204 Miyano add 連線至CostcoPay主機時專用 */
        if(fLinkCostcoPayHost == VS_TRUE)
        {
                return inETHERNET_SetConfig_Costco();
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
        
	inRetVal = inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_HOSTIP, (unsigned char*)szHostIPPrimary, strlen(szHostIPPrimary));
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "d_ETHERNET_CONFIG_HOSTIP Error : 0x%04x", inRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
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
        
	inRetVal = inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_HOSTPORT, (unsigned char*)szHostPortNoPrimary, strlen(szHostPortNoPrimary));
	if (inRetVal != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)
                        inDISP_LogPrintf("d_ETHERNET_CONFIG_HOSTPORT Error");
	
                return (VS_ERROR);
	}
        
        /*
                d_ETHERNET_CONFIG_AUTOCON
                Set Connection Mode.
                = 0 : Auto-connect. When Auto-connect is set, the Ethernet module will automatically try to connect to the host every 5 seconds.
                = 1 : Not support
                = 2 : Manual. The connection must be established manually by calling CTOS_EthernetConnectxxx() function.
        */
        /* 預設成2 */
	memset(szConfig, 0x00, sizeof(szConfig));
	memset(szAutoConnect, 0x00, sizeof(szAutoConnect));
	inGetAutoConnect(szAutoConnect);
	if (memcmp(&szAutoConnect[0], "1", strlen("1")) == 0)
	{
		szConfig[0] = '0';
	}
	else
	{
		szConfig[0] = '2';
	}
	
	if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_AUTOCON, (unsigned char*)szConfig, 1) != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)        
                        inDISP_LogPrintf("d_ETHERNET_CONFIG_AUTOCON Error");
		
                return (VS_ERROR);
	}
	
//	memset(szConfig, 0x00, sizeof(szConfig));
//	memcpy()
//	if (inETHERNET_Cofig_Set( d_ETHERNET_CONFIG_CONNTIMEOUT, (unsigned char*)szConfig, 1) != VS_SUCCESS)
//	{
//                if (ginDebug == VS_TRUE)        
//                        inDISP_LogPrintf("d_ETHERNET_CONFIG_AUTOCON Error");
//		
//                return (VS_ERROR);
//	}
	

        /*
                d_ETHERNET_CONFIG_DHCP
                Set IP configuration.
                = 0 : Static. Use the static IP set in the Ethernet configuration.
                = 1 : DHCP. Retrieve the dynamic IP from the DHCP server.
        */
        /* DHCP預設0 */
	memset(szConfig, 0x00, sizeof(szConfig));
	memset(szDHCPMode, 0x00, sizeof(szDHCPMode));
	inGetDHCP_Mode(szDHCPMode);
	if (memcmp(szDHCPMode, "Y", strlen("Y")) == 0)
	{
		szConfig[0] = 0x31;
	}
	else
	{
		szConfig[0] = 0x30;
	}
        
	if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_DHCP, (unsigned char*)szConfig, 1) != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)
                        inDISP_LogPrintf("d_ETHERNET_CONFIG_DHCP Error");
	
                return (VS_ERROR);
	}

        /*
                d_ETHERNET_CONFIG_UPDATE_EXIT
                Save the settings to the non-volatile memory in Ethernet module.      
        */
	memset(szConfig, 0x00, sizeof(szConfig));
	if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_UPDATE_EXIT, (unsigned char*)szConfig, 0) != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)
                        inDISP_LogPrintf("d_ETHERNET_CONFIG_UPDATE_EXIT Error");
	
                return (VS_ERROR);
	}
/* 新增條件，玉山自助由計次建立連線改為以秒數建立，建立通訊的時長為30秒  2021/9/14 上午 10:42 [SAM] */
#if (_MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_) && (defined(_ESUN_MAIN_HOST_) )    
        /* 因不需重試次數，所以設定一個最大值，以免進入原來設定的迴圈 */
	inConnectNowCnt = 99;
#else    
	/* 重置目前重試次數 */
	inConnectNowCnt = 0;
#endif    
	/* 若連線失敗看是否要重試 */
    	do 
	{
		if (ginDebug == VS_TRUE)
		{
			inFunc_CalculateRunTimeGlobal_Start();
		}
		
		inRetVal = inETHERNET_Connect_Flow(szHostIPPrimary, szHostPortNoPrimary);
		inConnectNowCnt ++;
		
//		if(inRetVal != VS_SUCCESS && inConnectNowCnt == 1)
//		{
//			/* DeInitial*/
//				inCOMM_DeInitCommDevice();
//				/* 重開module */
//				inCOMM_InitCommDevice();
//		}
//		
		
		if (ginDebug == VS_TRUE)
		{
			inFunc_WatchRunTime();
		}
		
	}while (inRetVal != VS_SUCCESS && inConnectNowCnt < inConnectMaxCnt);
        
	
	/* 第一授權主機連線成功 */
	if (inRetVal == VS_SUCCESS)
	{
		/* 如果是DHCP，*/
		if (memcmp(szDHCPMode, "Y", strlen("Y")) == 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			uszLen = sizeof(szTemplate);
			inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_IP, (unsigned char*)szTemplate, &uszLen);
			inSetTermIPAddress(szTemplate);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			uszLen = sizeof(szTemplate);
			inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_MASK, (unsigned char*)szTemplate, &uszLen);
			inSetTermMASKAddress(szTemplate);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			uszLen = sizeof(szTemplate);
			inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_GATEWAY, (unsigned char*)szTemplate, &uszLen);
			inSetTermGetewayAddress(szTemplate);

			inSaveEDCRec(0);
		}

		return (VS_SUCCESS);
	}
	else
	{
		/* 若第一授權主機連線不成功，改連第二授權主機 */
		/* IFES情況下才連第二組 */
		memset(szIFESMode, 0x00, sizeof(szIFESMode));
		inGetI_FES_Mode(szIFESMode);
		if (memcmp(szIFESMode, "Y", strlen("Y")) != 0)
		{
			/* 到最後還失敗，斷線 */
			inETHERNET_END();
			return (VS_ERROR);
		}
       
	}
	
	/* 看狀態 */
	inEthernet_Watch_Status();
	
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
        
	inRetVal = inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_HOSTIP, (unsigned char*)szHostIPSecond, strlen(szHostIPSecond));
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
                {
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "d_ETHERNET_CONFIG_HOSTIP Error : 0x%04x", inRetVal);
                        inDISP_LogPrintf(szDebugMsg);
                }
	
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
        
	if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_HOSTPORT, (unsigned char*)szHostPortNoSecond, strlen(szHostPortNoSecond)) != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)
                        inDISP_LogPrintf("d_ETHERNET_CONFIG_HOSTPORT Error");
	
                return (VS_ERROR);
	}
	
	/* 紀錄在記憶體內 */
	memset(szConfig, 0x00, sizeof(szConfig));
	if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_UPDATE_EXIT, (unsigned char*)szConfig, 0) != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)
                        inDISP_LogPrintf("d_ETHERNET_CONFIG_UPDATE_EXIT Error");
	
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
		
		inRetVal = inETHERNET_Connect_Flow(szHostIPSecond, szHostPortNoSecond);
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
	inETHERNET_END();
	
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inETHERNET_SetConfig() END!");
        
	/* 若連線成功，中間就會Return出去，若一直失敗，則最後一定回傳失敗 */
        return (VS_ERROR);
}

/*
Function        :inETHERNET_Send
Date&Time       :2017/7/18 下午 5:25
Describe        :送電文，並根據電文格式前面加長度
*/
int inETHERNET_Send(unsigned char *uszSendBuff, int inSendSize, int inSendTimeout)
{
	char	szDataHead[1 + 1];
	char	szDebugMsg[100 + 1];
	unsigned char   szSendData[inSendSize + 32];

        memset(szSendData, 0x00, sizeof(szSendData));
        memset(szDataHead, 0x00, sizeof(szDataHead));

        /* inETHERNET_Send() START! */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inETHERNET_Send() START!");

        /* Get HeadFormat */
        if (inGetTCPHeadFormat(szDataHead) == VS_ERROR && fLinkCostcoPayHost != VS_TRUE)
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
                                        inDISP_LogPrintf("inETHERNET_Send() TIMEOUT");
				}
				
				return (VS_ERROR);
			}
		}

		/* 如果可以Send就Send */
		if (inETHERNET_Send_Ready_Flow()== VS_SUCCESS)
		{
			if (inETHERNET_Send_Data_Flow(szSendData, inSendSize) != VS_SUCCESS)
			{
				continue;
			}
			else
				break;
		}
	}

        /* inETHERNET_Send() END! */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inETHERNET_Send() END!");

        return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Receive
Date&Time       :2017/7/18 下午 5:55
Describe        :
*/
int inETHERNET_Receive(unsigned char *uszReceiveBuff, int inReceiveSize, int inReceiveTimeout)
{
	int		inRetVal = 0;
	int     	inReceivelen = 0;			/* Comport當前收到的長度 */
	int     	inDataLength = 0;			/* 收到的資料長度(不含Head) */
	int		inExpectLength = 0;
	char    	szDataHead[1 + 1];
	char    	szDebugMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
	unsigned char 	uszRawBuffer[inReceiveSize + 1];
	unsigned short 	usOutputLen = 0;			/* 目前comport可以讀取的資料長度 */
	
	/* inETHERNET_Receive() START! */
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "ETH_1917 inETHERNET_Receive_INIT");	
	
	inDISP_LogPrintfWithFlag("  inReceiveSize[%d] inReceivelen[%d]",inReceiveSize , inReceivelen);
	
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

				if (inReceiveTimeout != 0)
				{
					inDISP_DispLogAndWriteFlie("  inETHERNET_Receive TimeOut[%d]  *Error* Line[%d]",inReceiveTimeout, __LINE__);
				}
				return (VS_TIMEOUT);
			}
		}
		
		/* 設定為剩餘的Buffer空間大小，才符合API的用法 */
		usOutputLen = inReceiveSize - inReceivelen;

		if (inETHERNET_Receive_Ready_Flow(&usOutputLen) == VS_SUCCESS)
		{
			inRetVal = inETHERNET_Receive_Data_Flow(&uszRawBuffer[inReceivelen], &usOutputLen);
			if (inRetVal == VS_SUCCESS)
			{
				inDISP_LogPrintfWithFlag("  Aft Eth Rec inReceivelen[%d] usOutputLen[%ld]",inReceivelen , usOutputLen);
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
	if (inGetTCPHeadFormat(szDataHead) == VS_ERROR  && fLinkCostcoPayHost != VS_TRUE)
	{
		/* inGetTCPHeadFormat ERROR */
		/* debug */
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d] TCPHead ERROR END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_ERROR);
	}
        else if(fLinkCostcoPayHost == VS_TRUE)  /* 20230204 Miyano add */
        {
                szDataHead[0] == 'H';
        }

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "ETH_1917 Aft_inETHERNET_Receive_Data_Flow");
	
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
		
		inDISP_LogPrintfWithFlag("  %02X %02X :length = %d", uszRawBuffer[0], uszRawBuffer[1], inExpectLength);
				
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
						if (inReceiveTimeout != 0)
						{
							inDISP_LogPrintfWithFlag("  inETHERNET_Receive TimeOut 2");
						}

						inDISP_LogPrintfWithFlag("  Receve Len: %d", inDataLength);
						return (VS_TIMEOUT);
					}
				}

				/* 設定為剩餘的Buffer空間大小，才符合API的用法 */
				usOutputLen = inReceiveSize - inReceivelen;

				if (inETHERNET_Receive_Ready_Flow(&usOutputLen) == VS_SUCCESS)
				{
					inRetVal = inETHERNET_Receive_Data_Flow(&uszRawBuffer[inReceivelen], &usOutputLen);
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
		inDISP_LogPrintfWithFlag("  inReceivelen ERROR!! :(inReceivelen = %d)", inReceivelen);

		return (VS_ERROR);
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "ETH_1917 inETHERNET_Receive END");
	/* inETHERNET_Receive() END! */
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

	return (inDataLength);
}

/*
Function        :inETHERNET_END
Date&Time       :2017/7/19 上午 11:09
Describe        :斷線
*/
int inETHERNET_END(void)
{
        /* inETHERNET_END() START! */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inETHERNET_END() START!");
	
	if (inETHERNET_DisConnect_Flow() != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("inETHERNET_Disconnect_Flow() == VS_ERROR");
		return (VS_ERROR);
	}

        /* inETHERNET_END() END! */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inETHERNET_END() END!");

        return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Flush
Date&Time       :2017/7/19 上午 11:14
Describe        :清空Comport的資料
*/
int inETHERNET_Flush(void)
{
	int	inRetVal = 0;
	char	szDebugMsg[100 + 1];   /* debug message */
        
        /* inETHERNET_Flush() START! */
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inETHERNET_Flush() START!");
	
	inRetVal = inETHERNET_Flush_Rx();
	if (inRetVal != VS_SUCCESS)
	{
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "inETHERNET_Flush Error");
                        inDISP_LogPrintf(szDebugMsg);
                }
		return (VS_ERROR);
	}
	
        /* inETHERNET_Flush() END! */
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inETHERNET_Flush() END!");

        return (VS_SUCCESS);
}

/*
Function        :inETHERNET_DeInitial
Date&Time       :2017/7/19 上午 11:20
Describe        :關閉Comport
*/
int inETHERNET_DeInitial(void)
{
	int	inRetVal = 0;
	char	szDebugMsg[100 + 1];   /* debug message */
        
        /* inETHERNET_DeInitial() START! */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inETHERNET_DeInitial() START!");

	inRetVal = inETHERNET_Close();
	if (inRetVal != VS_SUCCESS)
	{
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "inETHERNET_Close ERR");
                        inDISP_LogPrintf(szDebugMsg);
                }
	}
	
        /* inETHERNET_DeInitial() END! */
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inETHERNET_DeInitial() END!");

        return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Get_Status
Date&Time       :2018/3/9 下午 1:23
Describe        :
*/
int inETHERNET_Get_Status(unsigned int *uiStatus)
{
	char		szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
	unsigned short  usRetval;
	
	if (uiStatus == NULL)
		return (VS_ERROR);
	
	usRetval = CTOS_EthernetStatus(uiStatus);
	
	if (usRetval != d_OK)
	{
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "CTOS_EthernetStatus Error : 0x%04x", usRetval);
                        inDISP_LogPrintf(szErrorMsg);
                }
	}
	
	return (VS_SUCCESS);
}


/*
Function        :inGetEthernetStatus
Date&Time       :2016/10/11 下午 3:50
Describe        :
*/
int inEthernet_Watch_Status(void)
{
	unsigned char	uszTemplate[40 + 1];
	unsigned char	uszLen = 0;
	unsigned int	uiStatus = 0;

	/* Get the status of the Ethernet */
	inETHERNET_Get_Status(&uiStatus);

	/* if Ethernet is connected */
	if (uiStatus & d_STATUS_ETHERNET_CONNECTED)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("EthernetStatus : Connected");
		}
	}
	/* if Ethernet is command mode */
	if (uiStatus & d_STATUS_ETHERNET_COMMAND_MODE)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("EthernetStatus : Command Mode");
		}
	}
	
	/* if Ethernet is phyical online */
	if (uiStatus & d_STATUS_ETHERNET_PHYICAL_ONLINE)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("EthernetStatus : Phyical Online");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("網路線沒插好！！");
		}
	}
	
	/* if Ethernet is Rx readey */
	if (uiStatus & d_STATUS_ETHERNET_RX_READY)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("EthernetStatus : Rx Ready");
		}
	}
	
	/* if Ethernet is Tx busy */
	if (uiStatus & d_STATUS_ETHERNET_TX_BUSY)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("EthernetStatus : Tx Busy");
		}
	}
	
	/* IP */
	memset(uszTemplate, 0x00, sizeof(uszTemplate));
	uszLen = sizeof(uszTemplate);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_IP, uszTemplate, &uszLen);
	
	/* MASK */
	memset(uszTemplate, 0x00, sizeof(uszTemplate));
	uszLen = sizeof(uszTemplate);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_MASK, uszTemplate, &uszLen);
	
	/* GATEWAY */
	memset(uszTemplate, 0x00, sizeof(uszTemplate));
	uszLen = sizeof(uszTemplate);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_GATEWAY, uszTemplate, &uszLen);
	
	/* HOSTIP */
	memset(uszTemplate, 0x00, sizeof(uszTemplate));
	uszLen = sizeof(uszTemplate);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_HOSTIP, uszTemplate, &uszLen);
	
	/* HOST PORT */
	memset(uszTemplate, 0x00, sizeof(uszTemplate));
	uszLen = sizeof(uszTemplate);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_HOSTPORT, uszTemplate, &uszLen);
	
	/* CONFIG VERSION */
	memset(uszTemplate, 0x00, sizeof(uszTemplate));
	uszLen = sizeof(uszTemplate);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_VERSION, uszTemplate, &uszLen);
	
	/* CONFIG MAC */
	memset(uszTemplate, 0x00, sizeof(uszTemplate));
	uszLen = sizeof(uszTemplate);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_MAC, uszTemplate, &uszLen);
	
	/* CONFIG DHCP */
	memset(uszTemplate, 0x00, sizeof(uszTemplate));
	uszLen = sizeof(uszTemplate);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_DHCP, uszTemplate, &uszLen);
	
	/* CONFIG DNSIP */
	memset(uszTemplate, 0x00, sizeof(uszTemplate));
	uszLen = sizeof(uszTemplate);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_DNSIP, uszTemplate, &uszLen);
	
	/* CONFIG URL*/
	memset(uszTemplate, 0x00, sizeof(uszTemplate));
	uszLen = sizeof(uszTemplate);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_HOSTURL, uszTemplate, &uszLen);
	
	/* CONFIG AUTOCON */
	memset(uszTemplate, 0x00, sizeof(uszTemplate));
	uszLen = sizeof(uszTemplate);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_AUTOCON, uszTemplate, &uszLen);
  
	return (VS_SUCCESS);
}

/*
Function        inEthernet_IsPhysicalOnine
Date&Time       :2018/3/9 下午 1:27
Describe        :回傳成功代表有插實體網路線
*/
int inEthernet_IsPhysicalOnine()
{
	unsigned int	uiStatus = 0;
	
	/* Get the status of the Ethernet */
	inETHERNET_Get_Status(&uiStatus);
	
	/* if Ethernet is phyical online */
	if (uiStatus & d_STATUS_ETHERNET_PHYICAL_ONLINE)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
	
	
}


/*
Function        :inETHERNET_Ping_IP
Date&Time       :2018/11/2 下午 1:23
Describe        :
 * [新增電票悠遊卡功能]  [SAM]  2022/6/8 下午 5:55
*/
int inETHERNET_Ping_IP(char *szIP)
{
	int		inLen = 0;
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;
	
	inLen = strlen(szIP);

	usRetVal = CTOS_EthernetPing((unsigned char*)szIP, inLen);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_EthernetPing Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_EthernetPing() OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}


/*
Function        :inETHERNET_Check_Ethernet_Config_Correct
Date&Time       :2019/10/18 上午 11:55
Describe        :ping Gateway如果成功，至少表示區域網路設置是對的，到gateway之前是通的
 * [新增電票悠遊卡功能]  [SAM]  2022/6/8 下午 5:55
*/
int inETHERNET_Check_Ethernet_Config_Correct(void)
{
	int		inRetVal = VS_ERROR;
	char		szGatewayIP[16 + 1] = {0};
	unsigned char	uszLen = 0;
	
	memset(szGatewayIP, 0x00, sizeof(szGatewayIP));
	uszLen = sizeof(szGatewayIP);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_GATEWAY, (unsigned char*)szGatewayIP, &uszLen);
	
	inRetVal = inETHERNET_Ping_IP(szGatewayIP);
	if (inRetVal == VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

int inETHERNET_SetConfig_Costco(void)
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
        inRetVal = inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_HOSTIP, (unsigned char*)szHostIPPrimary, strlen(szHostIPPrimary));
        
        if(inGetTSP_Port(szHostPortNoPrimary) != VS_SUCCESS)
        {
                inDISP_LogPrintf("inGetTSP_IP Error!");
                return (VS_ERROR);
        }
        
        inRetVal = inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_HOSTPORT, (unsigned char*)szHostPortNoPrimary, strlen(szHostPortNoPrimary));
        
        /* 重置目前重試次數 */
	inConnectNowCnt = 0;
   
	/* 若連線失敗看是否要重試 */
    	do 
	{
		if (ginDebug == VS_TRUE)
		{
			inFunc_CalculateRunTimeGlobal_Start();
		}
		
		inRetVal = inETHERNET_Connect_Flow(szHostIPPrimary, szHostPortNoPrimary);
		inConnectNowCnt ++;
		
		if (ginDebug == VS_TRUE)
		{
			inFunc_WatchRunTime();
		}
		
	}while (inRetVal != VS_SUCCESS && inConnectNowCnt < inConnectMaxCnt);
        
        /* 連線成功 */
	if (inRetVal != VS_SUCCESS)
	{
		inETHERNET_END();
                return (VS_ERROR);
	}
	return VS_SUCCESS;
}
