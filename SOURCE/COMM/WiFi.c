#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
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
#include "../EVENT/Flow.h"
#include "../FUNCTION/CDT.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/CPT.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/ECR_FUNC/ECR.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/HDPT.h"
#include "../FUNCTION/MULTI_FUNC/MultiFunc.h"

#include "../FUNCTION/CARD_FUNC/CardFunction.h"

#include "Comm.h"
#include "Ethernet.h"
#include "WiFi.h"

#include "../FUNCTION/COSTCO_FUNC/Costco.h"

extern	int		ginDebug;
extern	int		ginDisplayDebug;
extern	int		ginEngineerDebug;
extern	int		ginMachineType;
extern ECR_TABLE	gsrECROb;
extern unsigned char	guszCTLSInitiOK;

extern  VS_BOOL         fLinkCostcoPayHost;     /* CostcoPay主機專用 */

extern int	ginECR_ServerFd;		/* File Descripter */
extern int	ginECR_ResponseFd;
extern int	ginTrans_ClientFd;
extern int	ginEthernetFlow;
int		ginWiFiSwitch = VS_FALSE;	/* 看WiFi是開還是關 */
unsigned char	guszNum = 0;
CTOS_stWifiInfo	gstWiFiInfo[_WiFi_Info_MaxNum_];

ECR_TRANS_TABLE stWiFi_ECRTable[] =
{
	/* 第零組標準【400】【8N1】規格 */
	{
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
	},
	/* 第一組標準【144】【7E1】規格 */
	{
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
	},
	/* 第二組邦柏(107)客製化需求【400】【8N1】規格 */
	{
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
	},

	/* 第一組 PEPPER 雀巢機器人咖啡館專案 */
	{
		inWiFi_ECR_01_Pepper_Nestle_Initial,
		inWiFi_ECR_01_Pepper_Nestle_Receive_Packet,
		inWiFi_ECR_01_Pepper_Nestle_Send_Packet,
		inWiFi_ECR_01_Pepper_Nestle_Send_Error,
		inWiFi_ECR_01_Pepper_Nestle_Close,
	},
	
};

/* Castle沒提供WiFi連Socket方法，要自行實做 */
/*
Function        :inWiFi_Open
Date&Time       :2017/5/3 上午 9:37
Describe        :
*/
int inWiFi_Open()
{
	int	inRetVal;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inWiFi_Open() START !");
	}
	
	inRetVal = CTOS_WifiOpen();
	/* 如果錯誤是IO_Processing，代表已開WiFi */
	if (inRetVal == d_OK	||
	    inRetVal == d_WIFI_IO_PROCESSING)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "WiFi Open OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		ginWiFiSwitch = VS_TRUE;
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "WiFi Open Err :0x%04X", inRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inWiFi_Open() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_Close
Date&Time       :2017/5/3 上午 9:37
Describe        :
*/
int inWiFi_Close()
{
	int	inRetVal;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inWiFi_Close() START !");
	}
	
	inRetVal = CTOS_WifiClose();
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "WiFi Close Err :0x%04X", inRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "WiFi Close OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		ginWiFiSwitch = VS_FALSE;
	}
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inWiFi_Close() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_Connect_Flow
Date&Time       :2017/9/29 下午 1:57
Describe        :WiFi只能用原生的，所以另寫一隻
*/
int inWiFi_Connect_Flow(char *szHostIP, char *szPort)
{
	int	inRetVal = VS_ERROR;
	char	szI_FES_Mode[2 + 1];
	char	szTRTFileName[12 + 1];
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inWiFi_Connect_Flow() START !");

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
		
		inRetVal = inETHERNET_Connect_TLS_Flow(szHostIP, szPort);
		/* 標示現在連線方式，斷線後重新恢復成_ETHERNET_FLOW_CTOS_ */
		ginEthernetFlow = _ETHERNET_FLOW_IFES_;
	}
	/* 用native方式建Socket，之後要優化再考慮使用(優點自己決定Timeout等等細微設定) */
	/* WiFi沒有虹堡API */
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Connect Flow: Native");
			inDISP_LogPrintf(szDebugMsg);
		}
		
		inRetVal = inETHERNET_Connect_By_Native(szHostIP, szPort);
		/* 標示現在連線方式，斷線後重新恢復成_ETHERNET_FLOW_CTOS_ */
		ginEthernetFlow = _ETHERNET_FLOW_NATIVE_;

	}
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inWiFi_Connect_Flow() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (inRetVal);
}

/*
Function        :inWiFi_DisConnect_Flow
Date&Time       :2017/9/29 下午 2:24
Describe        :分流
*/
int inWiFi_DisConnect_Flow()
{
	int	inRetVal = VS_ERROR;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inWiFi_DisConnect_Flow() START !");
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
	else
	{
		inRetVal = inETHERNET_DisConnect_By_Native();
		if (inRetVal == VS_SUCCESS)
		{
			ginEthernetFlow = _ETHERNET_FLOW_CTOS_;
		}
	}
		
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inWiFi_DisConnect_Flow() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_Initial
Date&Time       :2017/7/18 下午 3:13
Describe        :
*/
int inWiFi_Initial()
{
	int	inRetVal = 0x00;
	int	inRetryNow = 0;
	int	inRetryMax = 3;
	char	szEDCIPAddress[16 + 1];
	char	szSubMask[16 + 1];
	char	szGateWay[16 + 1];
	char	szDHCPMode[2 + 1];
	char	szDebugMsg[100 + 1];
	char	szAutoConnect[10 + 1];
	
	/* inWiFi_Initial() START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inWiFi_Initial() START！");
        }
	
	memset(szEDCIPAddress, 0x00, sizeof(szEDCIPAddress));
        memset(szSubMask, 0x00, sizeof(szSubMask));
        memset(szGateWay, 0x00, sizeof(szGateWay));
	
	inDISP_ClearAll();
	inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SET_COMM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <通訊設定> */
	
	memset(szAutoConnect, 0x00, sizeof(szAutoConnect));
	inGetAutoConnect(szAutoConnect);
	if (szAutoConnect[1] == '1')
	{
		if (inWiFi_IsConected() == VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Already Connected");
				inDISP_LogPrintf(szDebugMsg);
			}
		}

	}
	else
	{
		inWiFi_WifiDisconnectAP();
		
		inRetryNow = 0;
		/* 若上次有設定AutoConnect，則WiFi Open時會自動抓上一次的AP */
		do
		{
			/* 停一秒，不然API會一直跳d_WIFI_IO_PROCESSING 0x6001 */
			inDISP_Wait(1000);
			
			inRetVal = inWiFi_Open();
			inRetryNow++;

		} while ((inRetryNow < inRetryMax) && (inRetVal != VS_SUCCESS));
		
		/* 若重試三次還是失敗就跳出 */
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
	}
	
        if (inGetTermIPAddress(szEDCIPAddress) == VS_ERROR)
        {
                /* inGetEDCIP ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inDISP_LogPrintf("inGetEDCIP() ERROR!!");
                return (VS_ERROR);
        }
        
        if (inGetTermGetewayAddress(szGateWay) == VS_ERROR)
        {
                /* inGetGateWay ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inDISP_LogPrintf("inGetGateWay() ERROR!!");
                return (VS_ERROR);
        }
        
        if (inGetTermMASKAddress(szSubMask) == VS_ERROR)
        {
                /* inGetSubMask ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inDISP_LogPrintf("inGetSubMask() ERROR!!");
                return (VS_ERROR);
        }

	memset(szDHCPMode, 0x00, sizeof(szDHCPMode));
	inGetDHCP_Mode(szDHCPMode);
	if (memcmp(szDHCPMode, "Y", strlen("Y")) == 0)
	{
		/* DHCP有開不用設定這三項 */
	}
	else
	{
		/* IP */
		inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_IP, (unsigned char*)szEDCIPAddress, strlen(szEDCIPAddress));
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("d_WIFI_CONFIG_IP Error");
			}
		}

		/* MASK */
		inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_MASK, (unsigned char*)szGateWay, strlen(szGateWay));
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("d_WIFI_CONFIG_MASK Error");
			}
		}

		/* GATEWAY */
		inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_GATEWAY, (unsigned char*)szSubMask, strlen(szSubMask));
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("d_WIFI_CONFIG_GATEWAY Error");
			}
		}
	}
	
	/* inWiFi_Initial()_END */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inWiFi_Initial()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inWiFi_Begin
Date&Time       :2017/7/18 下午 3:13
Describe        :
*/
int inWiFi_Begin(TRANSACTION_OBJECT *pobtran)
{
        char    szCommIndex[2 + 1];
        char    szDebugBuf[64 + 1];

        /* inWiFi_Begin() START */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inWiFi_Begin() START！");
        }

        /* 20230204 Miyano add 連線至CostcoPay主機時專用 */
        if(fLinkCostcoPayHost == VS_TRUE)
        {
                if (ginDebug == VS_TRUE)
                {
                        inDISP_LogPrintf("fSendToCostcoPayHost Flag_On!");
                        inDISP_LogPrintf("----------------------------------------");
                        inDISP_LogPrintf("inWiFi_Begin()_END");
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

        /* inWiFi_Begin()_END */
        if (ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("----------------------------------------");
                inDISP_LogPrintf("inWiFi_Begin()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inWiFi_SetConfig
Date&Time       :2017/7/18 下午 3:14
Describe        :
*/
int inWiFi_SetConfig(void)
{
	int	inRetVal = 0;
	int	inConnectMaxCnt = 1;		/* 連線最大重試次數(含第一次) */
	int	inConnectNowCnt = 0;		/* 連線目前重試次數 */
        char	szHostIPPrimary[16 + 1];
	char	szHostIPSecond[16 + 1];
	char	szHostPortNoPrimary[6 + 1];
	char	szHostPortNoSecond[6 + 1];
	char	szConfig[16 + 1];
	char	szDebugMsg[100 + 1];
	char	szDHCPMode[2 + 1];
	char	szAutoConnect[10 + 1];

        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inWiFi_SetConfig() START!");
        
        /* 20230204 Miyano add 連線至CostcoPay主機時專用 */
        if(fLinkCostcoPayHost == VS_TRUE)
        {
                return inWiFi_SetConfig_Costco();
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
	
	/* Auto Connect */
	memset(szConfig, 0x00, sizeof(szConfig));
	memset(szAutoConnect, 0x00, sizeof(szAutoConnect));
	inGetAutoConnect(szAutoConnect);
	if (szAutoConnect[1] == '1')
	{
		szConfig[0] = '1';
	}
	else
	{
		szConfig[0] = '0';
	}
	
	inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_AUTOCON_AP, (unsigned char*)szConfig, 1);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("d_WIFI_CONFIG_AUTOCON_AP Error");
		}
	}

        /*
                d_WiFi_CONFIG_DHCP
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
        
	inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_DHCP, (unsigned char*)szConfig, 1);
	if (inRetVal != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)
                        inDISP_LogPrintf("d_ETHERNET_CONFIG_DHCP Error");
	
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
		
		inRetVal = inWiFi_Connect_Flow(szHostIPPrimary, szHostPortNoPrimary);
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
		
		inRetVal = inWiFi_Connect_Flow(szHostIPSecond, szHostPortNoSecond);
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
	inWiFi_END();
	
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inWiFi_SetConfig() END!");
        
	/* 若連線成功，中間就會Return出去，若一直失敗，則最後一定回傳失敗 */
        return (VS_ERROR);
}

/*
Function        :inWiFi_END
Date&Time       :2017/7/19 上午 11:09
Describe        :斷線
*/
int inWiFi_END(void)
{
        /* inWiFi_END() START! */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inWiFi_END() START!");
	
	if (inWiFi_DisConnect_Flow() != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("inWiFi_Disconnect_Flow() == VS_ERROR");
		return (VS_ERROR);
	}

        /* inWiFi_END() END! */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inWiFi_END() END!");

        return (VS_SUCCESS);
}

/*
Function        :inWiFi_Flush
Date&Time       :2017/9/29 下午 2:35
Describe        :清空Comport的資料，WiFi無此API可使用
*/
int inWiFi_Flush(void)
{
        /* inWiFi_Flush() START! */
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inWiFi_Flush() START!");
	
	
        /* inWiFi_Flush() END! */
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inWiFi_Flush() END!");

        return (VS_SUCCESS);
}

/*
Function        :inWiFi_DeInitial
Date&Time       :2017/9/29 下午 2:36
Describe        :關閉Comport
*/
int inWiFi_DeInitial(void)
{
	int	inRetVal = 0;
	char	szDebugMsg[100 + 1];   /* debug message */
        
        /* inWiFi_DeInitial() START! */
        if (ginDebug == VS_TRUE)
                inDISP_LogPrintf("inWiFi_DeInitial() START!");

	inRetVal = inWiFi_Close();
	if (inRetVal != VS_SUCCESS)
	{
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "inWiFi_Close ERR");
                        inDISP_LogPrintf(szDebugMsg);
                }
	}
	
        /* inWiFi_DeInitial() END! */
	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inWiFi_DeInitial() END!");

        return (VS_SUCCESS);
}

/*
Function        :inWiFi_Switch
Date&Time       :2017/5/5 上午 11:48
Describe        :
*/
int inWiFi_Switch()
{
	if (ginWiFiSwitch == VS_FALSE)
	{
		inWiFi_Open();
	}
	else
	{
		inWiFi_Close();
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_Status
Date&Time       :2017/5/3 上午 9:52
Describe        :
*/
int inWiFi_Status()
{
	int		inRetVal;
	char		szDebugMsg[100 + 1];
	unsigned int	uiStatus = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inWiFi_Status() START !");
	}
	
	inRetVal = inWiFi_GetStatus(&uiStatus);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inWiFi_GetStatus Err");
			inDISP_LogPrintf(szDebugMsg);
		}
		
	}
	
	if ((uiStatus & d_WIFI_STATE_AP_CONNECTED) == d_WIFI_STATE_AP_CONNECTED)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "d_WIFI_STATE_AP_CONNECTED");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	if ((uiStatus & d_WIFI_STATE_SCANNING) == d_WIFI_STATE_SCANNING)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "d_WIFI_STATE_SCANNING");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	if ((uiStatus && d_WIFI_STATE_AP_CONNECTING) == d_WIFI_STATE_AP_CONNECTING)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "d_WIFI_STATE_AP_CONNECTING");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	if ((uiStatus & d_WIFI_STATE_CONNECTING) == d_WIFI_STATE_CONNECTING)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "d_WIFI_STATE_CONNECTING");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	if ((uiStatus & d_WIFI_STATE_SENDING) == d_WIFI_STATE_SENDING)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "d_WIFI_STATE_SENDING");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	if ((uiStatus & d_WIFI_STATE_RECEIVING) == d_WIFI_STATE_RECEIVING)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "d_WIFI_STATE_RECEIVING");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	if ((uiStatus & d_WIFI_STATE_DISCONNECTING) == d_WIFI_STATE_DISCONNECTING)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "d_WIFI_STATE_DISCONNECTING");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	if ((uiStatus & d_WIFI_STATE_AP_DISCONNECTING) == d_WIFI_STATE_AP_DISCONNECTING)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "d_WIFI_STATE_AP_DISCONNECTING");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inWiFi_Status() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_GetStatus
Date&Time       :2017/6/2 上午 10:06
Describe        :
*/
int inWiFi_GetStatus(unsigned int *uiStatus)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inWiFi_GetStatus() START !");
	}
	
	inRetVal = CTOS_WifiStatus(uiStatus);
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_WifiStatus Err :0x%04X", inRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		inRetVal = VS_ERROR;
	}
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inWiFi_GetStatus() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inWiFi_Scan
Date&Time       :2017/5/3 下午 5:42
Describe        :Scan是一段過程，所以需等到CTOS_WifiStatus吐d_ok或error才可做下一步，若吐d_WIFI_STATE_SCANNING則等到他完成
*/
int inWiFi_Scan()
{
	int	inRetVal;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inWiFi_Scan() START !");
	}
	
	inRetVal = CTOS_WifiScan();
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "WiFi Scan Err :0x%04X", inRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		
	}
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inWiFi_Scan() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_ScanProcess
Date&Time       :2017/6/2 上午 10:16
Describe        :Scan是一段過程，所以需等到CTOS_WifiStatus吐d_ok或error才可做下一步，若吐d_WIFI_STATE_SCANNING則等到他完成
*/
int inWiFi_ScanProcess()
{
	int		inRetVal = VS_SUCCESS;
	unsigned int	uiStatus = 0;
	
	inWiFi_Scan();
	
	while(1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("掃描中", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
		
		inRetVal = inWiFi_GetStatus(&uiStatus);
		if (inRetVal == VS_SUCCESS)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("掃描完成", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
			break;
		}
		else
		{
			/* 繼續掃等完成 */
			if (uiStatus & d_WIFI_STATE_SCANNING)
			{
				continue;
			}
			else
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("掃描失敗", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
				break;
			}
		}
		
	}
	
	return (inRetVal);
}

/*
Function        :inWiFi_IsConected
Date&Time       :2017/5/31 下午 3:51
Describe        :是否已連上
*/
int inWiFi_IsConected()
{
	int		inRetVal;
	char		szDebugMsg[100 + 1];
	unsigned int	uiStatus = 0;
	
	inRetVal = inWiFi_GetStatus(&uiStatus);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inWiFi_GetStatus Err");
			inDISP_LogPrintf(szDebugMsg);
		}
		
	}
	
	if ((uiStatus & d_WIFI_STATE_AP_CONNECTED) == d_WIFI_STATE_AP_CONNECTED)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

/*
Function        :inWiFi_IsScanning
Date&Time       :2017/6/2 上午 9:56
Describe        :是否掃描中
*/
int inWiFi_IsScanning()
{
	int		inRetVal;
	char		szDebugMsg[100 + 1];
	unsigned int	uiStatus = 0;
	
	inRetVal = inWiFi_GetStatus(&uiStatus);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inWiFi_GetStatus Err");
			inDISP_LogPrintf(szDebugMsg);
		}
		
	}
	
	if ((uiStatus & d_WIFI_STATE_SCANNING) == d_WIFI_STATE_SCANNING)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

/*
Function        :inWiFi_Info_Get
Date&Time       :2017/5/3 下午 5:50
Describe        :
*/
int inWiFi_Info_Get(unsigned char* uszNum, CTOS_stWifiInfo *srInfo)
{
	int		inRetVal = VS_ERROR;
	char		szDebugMsg[100 + 1];
	CTOS_stWifiInfo *srInfoTemp;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inWiFi_Info_Get() START !");
	}
	
	inRetVal = CTOS_WifiInfoGet(uszNum, &srInfoTemp);
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inWiFi_Info_Get Err :0x%04X", inRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		
	}
	else
	{
		if (*uszNum > _WiFi_Info_MaxNum_)
		{
			*uszNum = _WiFi_Info_MaxNum_;
		}
		else
		{
			
		}
			
		memcpy(srInfo, srInfoTemp, sizeof(CTOS_stWifiInfo) * (*uszNum));
	}
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inWiFi_Info_Get() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_WifiConnectAP
Date&Time       :2017/5/3 下午 5:50
Describe        :
*/
int inWiFi_WifiConnectAP(CTOS_stWifiInfo* srInfo, char*	szPassword, int inPasswordLen)
{
	int	inRetVal = -1;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inWiFi_WifiConnectAP() START !");
	}
	
	inRetVal = CTOS_WifiConnectAP(srInfo, (unsigned char*)szPassword, inPasswordLen);
	inWiFi_Status();
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inWiFi_WifiConnectAP Err :0x%04X", inRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inWiFi_WifiConnectAP() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_WifiDisconnectAP
Date&Time       :2017/5/3 下午 5:50
Describe        :
*/
int inWiFi_WifiDisconnectAP()
{
	int	inRetVal;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inWiFi_WifiDisconnectAP() START !");
	}
	
	inRetVal = CTOS_WifiDisconnectAP();
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inWiFi_WifiDisconnectAP Err :0x%04X", inRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		
	}
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inWiFi_WifiDisconnectAP() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_DispAP_Enter_Pin
Date&Time       :2017/5/5 下午 4:39
Describe        :
*/
int inWiFi_DispAP_Enter_Pin(CTOS_stWifiInfo* srInfo)
{
	int		inRetVal = VS_ERROR;
	int		inPasswordLen = 0;
	char		szPassword[20 + 1];
	unsigned int	uiStatus = 0;
	DISPLAY_OBJECT	srDispObj;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inWiFi_DispAP_Enter_Pin() START !");
	}
	
	inDISP_ClearAll();
	inDISP_ChineseFont((char*)srInfo->ESSID, _FONTSIZE_8X16_, _LINE_8_1_, _DISP_CENTER_);
	
		
	/* 沒加密，不用輸入密碼*/
	if (memcmp(srInfo->EncryptionKey, "off", strlen("off")) == 0)
	{
		memset(szPassword, 0x00, sizeof(szPassword));
		inPasswordLen = 0;
	}
	/* 有加密 */
	else
	{
		inDISP_ChineseFont("請輸入密碼", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
		memset(&srDispObj, 0x00, sizeof(srDispObj));
		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMaxLen = 20;			/* 不可超過9，long變數最多放9位 */
		srDispObj.inColor = _COLOR_RED_;
		while (1)
		{
			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;
	
			inRetVal = inDISP_Enter8x16_Character(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			{
				return (inRetVal);
			}
			else
			{
				memset(szPassword, 0x00, sizeof(szPassword));
				memcpy(szPassword, srDispObj.szOutput, srDispObj.inOutputLen);
				inPasswordLen = srDispObj.inOutputLen;
				break;
			}
		}
	}
	
	inWiFi_WifiConnectAP(srInfo, szPassword, inPasswordLen);
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_CONNECTING_, 0, _COORDINATE_Y_LINE_8_7_);
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	
	do
	{
		inRetVal = inWiFi_GetStatus(&uiStatus);
		if (inRetVal == VS_SUCCESS)
		{
			break;
		}
		else
		{
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				inRetVal = VS_TIMEOUT;
				break;
			}
			else if (uiStatus & d_WIFI_STATE_AP_CONNECTING)
			{
				continue;
			}
			else
			{
				break;
			}
		}
		
	} while (1);

	if (inRetVal != VS_SUCCESS)
	{
		inDISP_ClearAll();
		inDISP_ChineseFont("連接失敗", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
	}
	else
	{
		inDISP_ClearAll();
		inDISP_ChineseFont("連接成功", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
	}
	uszKBD_GetKey(_EDC_TIMEOUT_);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inWiFi_DispAP_Enter_Pin() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inWiFi_Disp_Info
Date&Time       :2017/5/5 下午 4:00
Describe        :
*/
int inWiFi_Disp_All_WiFi_Name(int inInfoNum, CTOS_stWifiInfo *stWiFiInfo)
{
	int	i = 0;
	int	inRetVal = VS_SUCCESS;
	int	inPage = 0;
	int	inWiFi_InOnePage = 6;
	int	inRemainCount = 0;
	int	inOffset = 0;
	char	szKey = 0;
	char	szDispMsg[100 + 1];
	
	/* 沒有WiFi device資訊 */
	if (inInfoNum == 0)
	{
		inDISP_ClearAll();
		inDISP_ChineseFont("沒有WiFi資訊", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		uszKBD_GetKey(_EDC_TIMEOUT_);
		return (VS_ERROR);
	}
		
	while (1)
	{
		inDISP_ClearAll();
		
		inOffset = inPage * inWiFi_InOnePage;
		
		inRemainCount = inInfoNum - inOffset;
		if (inRemainCount > inWiFi_InOnePage)
		{
			inRemainCount = inWiFi_InOnePage;
		}
		
		for (i = 0; i < inRemainCount; i++)
		{
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "%d. %s", i + 1, stWiFiInfo[inOffset + i].ESSID);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X44_, _LINE_16_1_ + i, _DISP_LEFT_);
		}
		
		while (1)
		{
			szKey = uszKBD_GetKey(_EDC_TIMEOUT_);
			
			if (szKey >= _KEY_1_ && szKey <= _KEY_6_ )
			{
				switch (szKey)
				{
					case _KEY_1_ :
						if (strlen((char*)stWiFiInfo[inOffset + 0].ESSID) < 0)
						{
							return (VS_ERROR);
						}
						inRetVal = inWiFi_DispAP_Enter_Pin(&stWiFiInfo[inOffset + 0]);
						break;
					case _KEY_2_ :
						if (strlen((char*)stWiFiInfo[inOffset + 1].ESSID) < 0)
						{
							return (VS_ERROR);
						}
						inRetVal = inWiFi_DispAP_Enter_Pin(&stWiFiInfo[inOffset + 1]);
						break;
					case _KEY_3_ :
						if (strlen((char*)stWiFiInfo[inOffset + 2].ESSID) < 0)
						{
							return (VS_ERROR);
						}
						inRetVal = inWiFi_DispAP_Enter_Pin(&stWiFiInfo[inOffset + 2]);
						break;
					case _KEY_4_ :
						if (strlen((char*)stWiFiInfo[inOffset + 3].ESSID) < 0)
						{
							return (VS_ERROR);
						}
						inRetVal = inWiFi_DispAP_Enter_Pin(&stWiFiInfo[inOffset + 3]);
						break;
					case _KEY_5_ :
						if (strlen((char*)stWiFiInfo[inOffset + 4].ESSID) < 0)
						{
							return (VS_ERROR);
						}
						inRetVal = inWiFi_DispAP_Enter_Pin(&stWiFiInfo[inOffset + 4]);
						break;
					case _KEY_6_ :
						if (strlen((char*)stWiFiInfo[inOffset + 5].ESSID) < 0)
						{
							return (VS_ERROR);
						}
						inRetVal = inWiFi_DispAP_Enter_Pin(&stWiFiInfo[inOffset + 5]);
						break;
					default:
						break;
				}
				
				if (inRetVal == VS_SUCCESS)
				{
					return (VS_SUCCESS);
				}
				else if (inRetVal == VS_USER_CANCEL)
				{
					return (VS_USER_CANCEL);
				}
				else if (inRetVal == VS_TIMEOUT)
				{
					return (VS_TIMEOUT);
				}
				else
				{
					return (VS_ERROR);
				}
				
			}
			else if (szKey == _KEY_UP_)
			{
				inPage--;
				if (inPage < 0)
				{
					inPage = 0;
				}
				break;
			}
			else if (szKey == _KEY_DOWN_)
			{
				if (inOffset + inWiFi_InOnePage >= inInfoNum)
				{
					/* 不變 */
				}
				else
				{
					inPage++;
				}
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				return (inRetVal);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				return (inRetVal);
			}
		}
	}
	
	return (inRetVal);
}

/*
Function        :inWiFi_ConfigGet
Date&Time       :2017/5/31 下午 1:46
Describe        :
*/
int inWiFi_ConfigGet(unsigned char uszTag, unsigned char* uszValue, unsigned char *uszLen)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1];
	
	inRetVal = CTOS_WifiConfigGet(uszTag, uszValue, uszLen);
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inWiFi_ConfigGet Err :0x%04X", inRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Tag:%d, Value:%s", uszTag, uszValue);
		inDISP_LogPrintf(szDebugMsg);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_ConfigSet
Date&Time       :2017/5/31 下午 1:46
Describe        :
*/
int inWiFi_ConfigSet(unsigned char uszTag, unsigned char* uszValue, unsigned char uszLen)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1];
	
	inRetVal = CTOS_WifiConfigSet(uszTag, uszValue, uszLen);
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inWiFi_ConfigSet Err :0x%04X", inRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Tag:%d, Value:%s", uszTag, uszValue);
		inDISP_LogPrintf(szDebugMsg);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_SetConfig_Flow
Date&Time       :2017/5/31 下午 1:53
Describe        :
*/
int inWiFi_SetConfig_Flow()
{
	int		inRetVal = VS_ERROR;
	int		j = 0;
	int		inDot = 0;
	char		szDispMsg[16 + 1] = {0};
	char		szAutoConnect[10 + 1] = {};
	unsigned char	uszChange = VS_FALSE;	/* 預設沒改變，若改變EDC IP相關，bit on起來並在結尾重新initial裝置 */
	unsigned char	uszCancel = VS_FALSE;	/* 跳出Ethernet設定迴圈 */
	DISPLAY_OBJECT  srDispObj;
	
	/* Load EDC Record */
        inLoadEDCRec(0);
	
	inDISP_ClearAll();
	while(1)
	{
		/* 輸入DHCP */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("DHCP(輸入Y/N)", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetDHCP_Mode(szDispMsg);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		while (1)
		{
			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 1;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;
	
			inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			{
				uszCancel = VS_TRUE;
				break;
			}

			if (strlen(srDispObj.szOutput) > 0)
			{
				if (srDispObj.szOutput[0] == 'Y'	||
				    srDispObj.szOutput[0] == 'N')
				{
					inSetDHCP_Mode(srDispObj.szOutput);
					inSaveCFGTRec(0);
					/* 有做更改 */
					uszChange = VS_TRUE;
					break;
				}
				else
				{
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					inDISP_EnglishFont("!!  DHCP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
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
		
		/* Auto Connect */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("Auto Connect", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("(輸入1/0)", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		memset(szAutoConnect, 0x00, sizeof(szAutoConnect));
		inGetAutoConnect(szAutoConnect);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		memcpy(szDispMsg, &szAutoConnect[1], 1);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		while (1)
		{
			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 1;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;
	
			inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			{
				uszCancel = VS_TRUE;
				break;
			}

			if (strlen(srDispObj.szOutput) > 0)
			{
				if (srDispObj.szOutput[0] == '1'	||
				    srDispObj.szOutput[0] == '0')
				{
					memcpy(&szAutoConnect[1], srDispObj.szOutput, 1);
					
					inSetAutoConnect(szAutoConnect);
					inSaveEDCRec(0);
					/* 有做更改 */
					uszChange = VS_TRUE;
					break;
				}
				else
				{
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					inDISP_EnglishFont("!! ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
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
		
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetDHCP_Mode(szDispMsg);
		if (memcmp(szDispMsg, "Y", strlen("Y")) == 0)
		{
			
		}
		/* 非DHCP才要設定 */
		else
		{
			/* 輸入EDC IP */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_EDIT_EDC_IP_, 0, _COORDINATE_Y_LINE_8_4_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetTermIPAddress(szDispMsg);
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

				break;
			}

			/* 跳出最大的迴圈*/
			if (uszCancel == VS_TRUE)
			{
				break;
			}

			/* 輸入EDC SUBMASK */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_EDIT_EDC_SUB_MASK_, 0, _COORDINATE_Y_LINE_8_4_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetTermMASKAddress(szDispMsg);
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
						inSetTermMASKAddress(srDispObj.szOutput);
						inSaveEDCRec(0);
						/* 有做更改 */
						uszChange = VS_TRUE;
						break;
					}
					else
					{
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						inDISP_EnglishFont("!!  Submask ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
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

			/* 輸入EDC Geteway */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_EDIT_EDC_GATEWAY_, 0, _COORDINATE_Y_LINE_8_4_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetTermGetewayAddress(szDispMsg);
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
						inSetTermGetewayAddress(srDispObj.szOutput);
						inSaveEDCRec(0);
						/* 有做更改 */
						uszChange = VS_TRUE;
						break;
					}
					else
					{
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						inDISP_EnglishFont("!!  Geteway ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
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
		}
		
		/* 輸入EDC port */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("請輸入port", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetTermECRPort(szDispMsg);
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
				if (strlen(srDispObj.szOutput) <= srDispObj.inMaxLen)
				{
					inSetTermECRPort(srDispObj.szOutput);
					inSaveEDCRec(0);
					/* 有做更改 */
					uszChange = VS_TRUE;
					break;
				}
				else
				{
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					inDISP_EnglishFont("!!  PORT ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
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
	}
	
	inWiFi_AutoSetCofig();
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_AutoSetCofig
Date&Time       :2017/10/18 下午 6:05
Describe        :
*/
int inWiFi_AutoSetCofig()
{
	int	inRetVal = VS_ERROR;
	char	szAutoConnect[10 + 1] = {};
	char	szTemplate[20 + 1];
	char	szConfig[30 + 1];
	
	/* DHCP */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetDHCP_Mode(szTemplate);
	memset(szConfig, 0x00, sizeof(szConfig));
	if (szTemplate[0] == 'Y')
	{
		szConfig[0] = '1';
	}
	else
	{
		szConfig[0] = '0';
	}
	
	inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_DHCP, (unsigned char*)szConfig, 1);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("d_WIFI_CONFIG_DHCP Error");
		}
	}
	
	/* Auto Connect */
	memset(szConfig, 0x00, sizeof(szConfig));
	memset(szAutoConnect, 0x00, sizeof(szAutoConnect));
	inGetAutoConnect(szAutoConnect);
	if (memcmp(&szAutoConnect[1], "1", strlen("1")) == 0)
	{
		szConfig[0] = '1';
	}
	else
	{
		szConfig[0] = '0';
	}
	
	inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_AUTOCON_AP, (unsigned char*)szConfig, 1);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("d_WIFI_CONFIG_AUTOCON_AP Error");
		}
	}
	
	
	/* 啟用DHCP，不用設定IP */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetDHCP_Mode(szTemplate);
	memset(szConfig, 0x00, sizeof(szConfig));
	if (szTemplate[0] == 'Y')
	{
		/* IP */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, "0.0.0.0", strlen("0.0.0.0"));
		memset(szConfig, 0x00, sizeof(szConfig));
		strcpy(szConfig, szTemplate);

		inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_IP, (unsigned char*)szConfig, strlen(szConfig));
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("d_WIFI_CONFIG_IP Error");
			}
		}

		/* MASK */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, "0.0.0.0", strlen("0.0.0.0"));
		memset(szConfig, 0x00, sizeof(szConfig));
		strcpy(szConfig, szTemplate);

		inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_MASK, (unsigned char*)szConfig, strlen(szConfig));
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("d_WIFI_CONFIG_MASK Error");
			}
		}

		/* GATEWAY */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, "0.0.0.0", strlen("0.0.0.0"));
		memset(szConfig, 0x00, sizeof(szConfig));
		strcpy(szConfig, szTemplate);

		inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_GATEWAY, (unsigned char*)szConfig, strlen(szConfig));
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("d_WIFI_CONFIG_GATEWAY Error");
			}
		}
	}
	else
	{
		/* IP */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTermIPAddress(szTemplate);
		memset(szConfig, 0x00, sizeof(szConfig));
		strcpy(szConfig, szTemplate);

		inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_IP, (unsigned char*)szConfig, strlen(szConfig));
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("d_WIFI_CONFIG_IP Error");
			}
		}

		/* MASK */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTermMASKAddress(szTemplate);
		memset(szConfig, 0x00, sizeof(szConfig));
		strcpy(szConfig, szTemplate);

		inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_MASK, (unsigned char*)szConfig, strlen(szConfig));
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("d_WIFI_CONFIG_MASK Error");
			}
		}

		/* GATEWAY */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTermGetewayAddress(szTemplate);
		memset(szConfig, 0x00, sizeof(szConfig));
		strcpy(szConfig, szTemplate);

		inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_GATEWAY, (unsigned char*)szConfig, strlen(szConfig));
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("d_WIFI_CONFIG_GATEWAY Error");
			}
		}
	}
	
	/* DNSIP */
	memset(szConfig, 0x00, sizeof(szConfig));
	strcpy(szConfig, "8.8.8.8");
	
	inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_DNSIP, (unsigned char*)szConfig, strlen(szConfig));
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("d_WIFI_CONFIG_DNSIP Error");
		}
	}
	
//	/* MAC */
//	memset(szConfig, 0x00, sizeof(szConfig));
//	strcpy(szConfig, "");
//	
//	inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_MAC, (unsigned char*)szConfig, strlen(szConfig));
//	if (inRetVal != VS_SUCCESS)
//	{
//		if (ginDebug == VS_TRUE)
//		{
//			inDISP_LogPrintf("d_WIFI_CONFIG_MAC Error");
//		}
//	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_GetConfig_Flow
Date&Time       :2017/6/1 上午 11:19
Describe        :
*/
int inWiFi_GetConfig_Flow()
{
	int		inRetVal;
	char		szTemplate[20 + 1];
	char		szConfig[30 + 1];
	unsigned char	uszLen = 0;
	
	inDISP_ClearAll();
	
	/* DHCP */
	memset(szConfig, 0x00, sizeof(szConfig));
	uszLen = sizeof(szConfig);
	inRetVal = inWiFi_ConfigGet(d_WIFI_CONFIG_DHCP, (unsigned char*)szConfig, &uszLen);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("d_WIFI_CONFIG_DHCP Error");
		}
	}
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (szConfig[0] == '1')
		{
			sprintf(szTemplate, "DHCP: ON");
		}
		else
		{
			sprintf(szTemplate, "DHCP: OFF");
		}
			
		inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_1_, _DISP_LEFT_);
	}

	/* Auto Connect */
	memset(szConfig, 0x00, sizeof(szConfig));
	uszLen = sizeof(szConfig);
	inRetVal = inWiFi_ConfigGet(d_WIFI_CONFIG_AUTOCON_AP, (unsigned char*)szConfig, &uszLen);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("d_WIFI_CONFIG_AUTOCON_AP Error");
		}
	}
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (szConfig[0] == '1')
		{
			sprintf(szTemplate, "AUTOCON_AP: ON");
		}
		else
		{
			sprintf(szTemplate, "AUTOCON_AP: OFF");
		}
			
		inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
	}
	
	/* IP */
	memset(szConfig, 0x00, sizeof(szConfig));
	uszLen = sizeof(szConfig);
	inRetVal = inWiFi_ConfigGet(d_WIFI_CONFIG_IP, (unsigned char*)szConfig, &uszLen);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("d_WIFI_CONFIG_IP Error");
		}
	}
	else
	{
		inDISP_ChineseFont("IP:", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%s", szConfig);
		inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
	}
	
	/* MASK */
	memset(szConfig, 0x00, sizeof(szConfig));
	uszLen = sizeof(szConfig);
	inRetVal = inWiFi_ConfigGet(d_WIFI_CONFIG_MASK, (unsigned char*)szConfig, &uszLen);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("d_WIFI_CONFIG_MASK Error");
		}
	}
	else
	{
		inDISP_ChineseFont("MASK: ", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%s", szConfig);
		inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
	}
	
	/* GATEWAY */
	memset(szConfig, 0x00, sizeof(szConfig));
	uszLen = sizeof(szConfig);
	inRetVal = inWiFi_ConfigGet(d_WIFI_CONFIG_GATEWAY, (unsigned char*)szConfig, &uszLen);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("d_WIFI_CONFIG_GATEWAY Error");
		}
	}
	else
	{
		inDISP_ChineseFont("GATEWAY: ", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%s", szConfig);
		inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	}
	
	uszKBD_GetKey(_EDC_TIMEOUT_);
	inDISP_ClearAll();
	
	/* DNSIP */
	memset(szConfig, 0x00, sizeof(szConfig));
	uszLen = sizeof(szConfig);
	inRetVal = inWiFi_ConfigGet(d_WIFI_CONFIG_DNSIP, (unsigned char*)szConfig, &uszLen);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("d_WIFI_CONFIG_DNSIP Error");
		}
	}
	else
	{
		inDISP_ChineseFont("DNSIP: ", _FONTSIZE_8X22_, _LINE_8_1_, _DISP_LEFT_);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%s", szConfig);
		inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
	}
	
	/* MAC */
	memset(szConfig, 0x00, sizeof(szConfig));
	uszLen = sizeof(szConfig);
	inRetVal = inWiFi_ConfigGet(d_WIFI_CONFIG_MAC, (unsigned char*)szConfig, &uszLen);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("d_WIFI_CONFIG_MAC Error");
		}
	}
	else
	{
		inDISP_ChineseFont("MAC: ", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%s", szConfig);
		inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
	}
	
	/* port */
	memset(szConfig, 0x00, sizeof(szConfig));
	inRetVal = inGetTermECRPort(szConfig);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("TermECRPort Error");
		}
	}
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "port: %s", szConfig);
		inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
	}
	
	uszKBD_GetKey(_EDC_TIMEOUT_);
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_Server_Open
Date&Time       :2017/5/3 下午 3:31
Describe        :開一個Server端的Socket
*/
int inWiFi_Server_Open()
{
	int			inHostPort = 0;
	int			inRetVal = 0;
	char			szDebugMsg[500 + 1];
	char			szECRTCPPort[4 + 1];
	struct  sockaddr_in	srServer_addr;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("inWiFi_Server_Open() START !");
	}
	
	/*	AF = Address Family
	 * 	PF = Protocol Family 
	 * 	AF_INET = PF_INET
	 * 	所以，理論上建立socket時是指定協議，應該用PF_xxxx，設置地址時應該用AF_xxxx。當然AF_INET和 PF_INET的值是相同的，混用也不會有太大的問題。
	 *	INET 表示 Internet
	 * 
	*/
	memset(szECRTCPPort, 0x00, sizeof(szECRTCPPort));
	inGetTermECRPort(szECRTCPPort);
	inHostPort = atoi(szECRTCPPort);
	
	ginECR_ServerFd = socket(PF_INET, SOCK_STREAM, 0);
	if (ginECR_ServerFd == -1)
	{
		return (VS_ERROR);
	}
	
	/* init struct dest */
	memset(&srServer_addr, 0x00, sizeof(srServer_addr));
	srServer_addr.sin_family = AF_INET;
	srServer_addr.sin_port = htons(inHostPort);		/* htons(host to network short)用來將主機的無符號短整形數轉換成網絡字節順序，因為主機是從高字節到低字節的 原因參考:http://beej-zhtw.netdpi.net/09-man-manual/9-12-htons-htonl-ntohs-ntohl */
	srServer_addr.sin_addr.s_addr = INADDR_ANY;		/* 因為是server端，位置設為空，也可寫成inet_addr("0.0.0.0) */
	
	/* Give the socket FD the local address ADDR (which is LEN bytes long).  */
	/* 網路上的Sample Code都說要轉(struct sockaddr*) */
	bind(ginECR_ServerFd, (struct sockaddr*)&srServer_addr, sizeof(srServer_addr));
	
	/* Prepare to accept connections on socket FD.
	 * N connection requests will be queued before further requests are refused.
	 * Returns 0 on success, -1 for errors.  */
	inRetVal = listen(ginECR_ServerFd, _Socket_Connections_);
	if (inRetVal == -1)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Listen Fail");
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	inWiFi_Create_Accept_Thread();
	
//	/* Close Server 不關 */
//	close(ginECR_ServerFd);
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inWiFi_Server_Open() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_Create_Accept_Thread
Date&Time       :2017/6/1 下午 2:21
Describe        :
*/
int inWiFi_Create_Accept_Thread()
{
	pthread_t		uliId;
	
	pthread_create(&uliId, NULL, (void*)vdWiFi_Accept_Thread, NULL);
	
	return (VS_SUCCESS);
}

/*
Function        :vdWiFi_Accept_Thread
Date&Time       :2017/5/31 下午 6:07
Describe        :
*/
void vdWiFi_Accept_Thread()
{
	int			inAddrLen = 0;
	struct	sockaddr_in	srClient_addr;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		inDISP_LogPrintf("vdWiFi_Accept_Thread() START !");
	}
	
	inAddrLen = sizeof(srClient_addr);
	/* Wait and Accept connection */
	ginECR_ResponseFd = accept(ginECR_ServerFd, (struct sockaddr*)&srClient_addr, (socklen_t*)&inAddrLen);
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("vdWiFi_Accept_Thread() END !");
		inDISP_LogPrintf("----------------------------------------");
	}
}

/*
Function        :inWiFi_IsAccept
Date&Time       :2017/6/2 下午 3:08
Describe        :
*/
int inWiFi_IsAccept(void)
{
	if (ginECR_ResponseFd >= 0)
	{

		if (ginDisplayDebug == VS_TRUE)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("Debug: 接受連線", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);
		}

		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

/*
Function        :inWiFi_Close_Client
Date&Time       :2017/6/1 下午 3:09
Describe        :
*/
int inWiFi_Close_Client(void)
{
	/* close(client) */
	close(ginECR_ResponseFd);
	ginECR_ResponseFd = -1;
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_ConnectedAP_InfoGet
Date&Time       :2017/6/2 下午 5:27
Describe        :
*/
int inWiFi_ConnectedAP_InfoGet(unsigned char uszTag, unsigned char* uszValue, unsigned char *uszLen)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = 0x00;
	
	if (ginMachineType == _CASTLE_TYPE_V3UL_ ||
            ginMachineType == _CASTLE_TYPE_V3P_  ||
            ginMachineType == _CASTLE_TYPE_V3C_)
	{
		
	}
	else
	{
		usRetVal = CTOS_WifiConnectedAPInfoGet(uszTag, uszValue, uszLen);

		if (usRetVal != d_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CTOS_WifiConnectedAPInfoGet Err :0x%04X", usRetVal);
				inDISP_LogPrintf(szDebugMsg);
			}
			return (VS_ERROR);
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Tag:%d, Value:%s", uszTag, uszValue);
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_ConnectedAP_InfoGet_Flow
Date&Time       :2017/6/2 下午 5:27
Describe        :
 *		signal和quality的差別
 *		Signal level: 
 *			raw signal level either in dBm or RSSI from the card.
 *		Link Quality: 
 *			a subjective measure of how good the link is, including signal strength, 
 *			speed the link is operating at vs. capable speed, packet loss, retries, etc.
 * 
 *		Signal level is just the raw measure of RF energy received by the radio on the card and is one component of link quality.
 * 
 *		Link Quality in correctly-written drivers should be a value in the range [0, 100] inclusive representing a percentage between “worst” link and 
 *		“best” link.
 * 
 *		Link quality in Linux wireless drivers has never been extremely reliable, though this is getting much better as time goes on. The ipw 
 *		drivers are good examples of how to do link quality correctly
*/
int inWiFi_ConnectedAP_InfoGet_Flow()
{
	int		inRetVal = VS_ERROR;
	char		szTemplate[100 + 1] = {0};
	char		szConfig[100 + 1] = {0};
	unsigned char	uszLen = 0;
	
	if (ginMachineType == _CASTLE_TYPE_V3UL_ || 
            ginMachineType == _CASTLE_TYPE_V3P_  ||
            ginMachineType == _CASTLE_TYPE_V3C_)
	{
		
	}
	else
	{
		

		inDISP_ClearAll();

		inRetVal = inWiFi_IsConected();
		if (inRetVal == VS_SUCCESS)
		{
			inDISP_ChineseFont("已連接", _FONTSIZE_8X22_, _LINE_8_1_, _DISP_CENTER_);
		}
		else
		{
			inDISP_ChineseFont("未連接", _FONTSIZE_8X22_, _LINE_8_1_, _DISP_CENTER_);

		}

		/* SSID */
		memset(szConfig, 0x00, sizeof(szConfig));
		uszLen = sizeof(szConfig);
		inRetVal = inWiFi_ConnectedAP_InfoGet(d_WIFIAP_SSID, (unsigned char*)szConfig, &uszLen);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("d_WIFIAP_SSID Error");
			}
		}
		else
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "SSID: %s", szConfig);

			inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
		}

		/* Quality */
		memset(szConfig, 0x00, sizeof(szConfig));
		uszLen = sizeof(szConfig);
		inRetVal = inWiFi_ConnectedAP_InfoGet(d_WIFIAP_Quality, (unsigned char*)szConfig, &uszLen);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("d_WIFIAP_Quality Error");
			}
		}
		else
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "Quality: %s", szConfig);

			inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		}

		/* SignalLv */
		memset(szConfig, 0x00, sizeof(szConfig));
		uszLen = sizeof(szConfig);
		inRetVal = inWiFi_ConnectedAP_InfoGet(d_WIFIAP_SignalLv, (unsigned char*)szConfig, &uszLen);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("d_WIFIAP_SignalLv Error");
			}
		}
		else
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "SignalLv: %s", szConfig);

			inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		}

		/* NoiseLv */
		memset(szConfig, 0x00, sizeof(szConfig));
		uszLen = sizeof(szConfig);
		inRetVal = inWiFi_ConnectedAP_InfoGet(d_WIFIAP_NoiseLv, (unsigned char*)szConfig, &uszLen);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("d_WIFIAP_NoiseLv Error");
			}
		}
		else
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "NoiseLv: %s", szConfig);

			inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		}

		/* MAC */
		memset(szConfig, 0x00, sizeof(szConfig));
		uszLen = sizeof(szConfig);
		inRetVal = inWiFi_ConnectedAP_InfoGet(d_WIFIAP_MAC, (unsigned char*)szConfig, &uszLen);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("d_WIFIAP_MAC Error");
			}
		}
		else
		{
			inDISP_ChineseFont("MAC:", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%s", szConfig);

			inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
		}

		uszKBD_GetKey(_EDC_TIMEOUT_);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_Get_Quality
Date&Time       :2018/3/13 下午 4:44
Describe        :
*/
int inWiFi_Get_Quality(unsigned char* uszQuality)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_WifiQualityGet(uszQuality);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_WifiQualityGet() OK");
			inDISP_LogPrintf(szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "WiFiQuality :%u", *uszQuality);
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else if (usRetVal == d_WIFI_NOT_OPEN	||
		 usRetVal == d_WIFI_NOT_SUPPORTED)
	{
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_WifiQualityGet Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_Test
Date&Time       :2017/5/5 下午 12:00
Describe        :
*/
int inWiFi_Test_Menu()
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey;
	
	while (1)
	{
		inDISP_ClearAll();
		inDISP_ChineseFont("1.開WiFi", _FONTSIZE_8X22_, _LINE_8_1_, _DISP_LEFT_);
		inDISP_ChineseFont("2.關WiFi", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
		inDISP_ChineseFont("3.Scan WiFi", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		inDISP_ChineseFont("4.Choose AP", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("5.Disconnect AP", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		inDISP_ChineseFont("6.Set Config", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
		inDISP_ChineseFont("7.Status", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
		inDISP_ChineseFont("8.Get Config", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
		uszKey = uszKBD_GetKey(_EDC_TIMEOUT_);
		
		if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (uszKey == _KEY_1_)
		{
			inRetVal = inWiFi_Open();
			if (inRetVal == VS_SUCCESS)
			{
				inDISP_ClearAll();
				inDISP_ChineseFont("WiFi Open OK", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
			}
			else
			{
				inDISP_ClearAll();
				inDISP_ChineseFont("WiFi Open Failed", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
			}
			uszKBD_GetKey(_EDC_TIMEOUT_);
		}
		else if (uszKey == _KEY_2_)
		{
			inRetVal = inWiFi_Close();
			if (inRetVal == VS_SUCCESS)
			{
				inDISP_ClearAll();
				inDISP_ChineseFont("WiFi Close OK", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
			}
			else
			{
				inDISP_ClearAll();
				inDISP_ChineseFont("WiFi Close Failed", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
			}
			uszKBD_GetKey(_EDC_TIMEOUT_);
		}
		else if (uszKey == _KEY_3_)
		{
			inDISP_ClearAll();
			inWiFi_ScanProcess();
			uszKBD_GetKey(_EDC_TIMEOUT_);
		}
		else if (uszKey == _KEY_4_)
		{
			inWiFi_AutoSetCofig();
				
			memset(&gstWiFiInfo, 0x00, sizeof(gstWiFiInfo));
			guszNum = 0;
			inWiFi_Info_Get(&guszNum, gstWiFiInfo);
			inWiFi_Disp_All_WiFi_Name((int)guszNum, gstWiFiInfo);
		}
		else if (uszKey == _KEY_5_)
		{
			inWiFi_WifiDisconnectAP();
			inRetVal = inDISP_ClearAll();
			if (inRetVal != VS_SUCCESS)
			{
				inDISP_ChineseFont("Disconnect 失敗", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
			}
			else
			{
				inDISP_ChineseFont("Disconnect 成功", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
			}
			uszKBD_GetKey(_EDC_TIMEOUT_);
		}
		else if (uszKey == _KEY_6_)
		{
			inRetVal = inWiFi_SetConfig_Flow();
			if (inRetVal != VS_SUCCESS)
			{
				inDISP_ClearAll();
				inDISP_ChineseFont("設定失敗", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
			}
			else
			{
				inDISP_ClearAll();
				inDISP_ChineseFont("設定成功", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
			}
			
			uszKBD_GetKey(_EDC_TIMEOUT_);
			
		}
		else if (uszKey == _KEY_7_)
		{
			inWiFi_ConnectedAP_InfoGet_Flow();
		}
		else if (uszKey == _KEY_8_)
		{
			inWiFi_GetConfig_Flow();
		}
		
	}
	
	inDISP_ClearAll();
	
	
	return (inRetVal);
}

/*
Function        :inWiFi_Initial
Date&Time       :2017/6/2 上午 9:43
Describe        :WiFi開機initial
*/
int inWiFi_ECR_Initial()
{
	int	inRetVal = VS_ERROR;
	
	inRetVal = stWiFi_ECRTable[gsrECROb.srSetting.inVersion].inEcrInitial(&gsrECROb);
	
	return (inRetVal);
}

/*
Function        :inWiFi_ECR_Receive_Transaction
Date&Time       :2017/5/31 下午 5:08
Describe        :
*/
int inWiFi_ECR_Receive_Transaction(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;

	/* 清空上次交易的資料 */
	memset(&gsrECROb.srTransData, 0x00, sizeof(ECR_TRANSACTION_DATA));
	memset(&gsrECROb.srTransDataTSB, 0x00, sizeof(ECR_TRANSACTION_DATA_TSB));
	
	/* 收資料 */	
	inRetVal = stWiFi_ECRTable[gsrECROb.srSetting.inVersion].inEcrRece(pobTran, &gsrECROb);
	
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inWiFi_ECR_Receive_Transaction ok");
		}
		return (VS_SUCCESS);
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inWiFi_ECR_Receive_Transaction Timeout");
		}
		
		return (VS_TIMEOUT);
	}
	else if (inRetVal == VS_USER_CANCEL)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inWiFi_ECR_Receive_Transaction Cancel");
		}
		
		return (VS_USER_CANCEL);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inWiFi_ECR_Receive_Transaction Error");
		}
		
		return (inRetVal);
	}
}
/*
Function        :inWiFi_ECR_Send_Result
Date&Time       :2017/6/1 下午 3:16
Describe        :回傳ECR
*/
int inWiFi_ECR_Send_Result(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;

	if (pobTran->uszECRBit != VS_TRUE)
		return (VS_SUCCESS);
	
	/* 送資料 */	
	inRetVal = stWiFi_ECRTable[gsrECROb.srSetting.inVersion].inEcrSend(pobTran, &gsrECROb);
	
	if (inRetVal == VS_ERROR)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inWiFi_ECR_Send_Result Error");
		}
		
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inWiFi_ECR_Send_Result ok");
		}
		
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inWiFi_ECR_Send_Error
Date&Time       :2017/6/1 下午 3:20
Describe        :
*/
int inWiFi_ECR_Send_Error(TRANSACTION_OBJECT *pobTran, int inErrorType)
{
	if (pobTran->uszECRBit != VS_TRUE)
		return (VS_SUCCESS);
	
	switch (gsrECROb.srSetting.inVersion)
	{
		case	1:
			if (gsrECROb.srTransData.inErrorType  == 0)
			{
				gsrECROb.srTransData.inErrorType = inErrorType;
			}
			if (gsrECROb.srTransDataTSB.inErrorType  == 0)
			{
				gsrECROb.srTransDataTSB.inErrorType = inErrorType;
			}
			
			break;
		default:
			gsrECROb.srTransData.inErrorType = inErrorType;
			gsrECROb.srTransDataTSB.inErrorType = inErrorType;
			break;
	}
	
	
	if (stWiFi_ECRTable[gsrECROb.srSetting.inVersion].inEcrSendError(pobTran, &gsrECROb) != VS_SUCCESS)
		return (VS_ERROR);

        return (VS_SUCCESS);
}

/*
Function        :inWiFi_ECR_Receive
Date&Time       :2017/5/31 下午 5:08
Describe        :
*/
int inWiFi_ECR_Receive(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer, int inDataSize)
{
	int		inRetVal;
	int		inTempReceiveSize = 0;
	int		inRetry = 0;				/* 目前已重試次數 */
	int		inRecMaxRetry = _ECR_WIFI_RETRYTIMES_;	/* 最大重試次數 */
	char		szReceiveBuffer[1024 + 1];
	unsigned short	usReceiveBufferSize = 0;		/* uszReceiveBuffer的長度，*/
	unsigned char	uszTempBuffer[1024 + 1];		/* 從 Buffer收到的片段 */
	
	/* 設定Timeout */
	srECROb->srSetting.inTimeout = _ECR_WIFI_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inDISP_Timer_Start(_ECR_WIFI_RECEIVE_TIMER_, srECROb->srSetting.inTimeout);
	
	/* 初始化放收到資料的陣列 */
	memset(szReceiveBuffer, 0x00, sizeof(szReceiveBuffer));
	usReceiveBufferSize = 0;

	while (1)
	{
		/* 超過重試次數 */
		if (inRetry > inRecMaxRetry)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("Retry too many times!");
			}
			
			return (VS_ERROR);
		}
		
		memset(uszTempBuffer, 0x00, sizeof(uszTempBuffer));
		inTempReceiveSize = 0;
		
		do
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_WIFI_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			inTempReceiveSize = sizeof(uszTempBuffer);
			inETHERNET_Receive_Data_By_Native(ginECR_ResponseFd, uszTempBuffer, (unsigned short*)&inTempReceiveSize);
		} while(inTempReceiveSize <= 0);
		
		memcpy(&szReceiveBuffer[usReceiveBufferSize], uszTempBuffer, inTempReceiveSize);
		usReceiveBufferSize += inTempReceiveSize;
		
		/* 長度相符 */
		if (usReceiveBufferSize == inDataSize)
		{
			break;
		}
	}
	
	/* 把資料放到szrRealReceBuffer */
	memcpy(szDataBuffer, &szReceiveBuffer[0], usReceiveBufferSize);
	
	/* 列印紙本電文和顯示電文訊息 */
	inECR_Print_Receive_ISODeBug(szDataBuffer, usReceiveBufferSize, inDataSize);
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_ECR_Receive_01_TSB
Date&Time       :2017/5/31 下午 5:08
Describe        :因為台新ECR為非固定長度，故另寫一隻
*/
int inWiFi_ECR_Receive_01_TSB(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer, int inDataSize)
{
	int		inRetVal = VS_ERROR;
	unsigned short	usReceiveBufferSize = 0;		/* uszReceiveBuffer的長度，*/
	
	/* 因台新連線規格為變動長度，故邊收邊判斷 */
	inDataSize = 0;
	
	int		inTempReceiveSize = 0;
	int		inRetry = 0;				/* 目前已重試次數 */
	int		inRecMaxRetry = _ECR_WIFI_RETRYTIMES_;	/* 最大重試次數 */
	char		szReceiveBuffer[1024 + 1];
	char		szTemplate[100 + 1];
	unsigned char	uszTempBuffer[1024 + 1];		/* 從 Buffer收到的片段 */
	
	/* 設定Timeout */
	srECROb->srSetting.inTimeout = _ECR_WIFI_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inDISP_Timer_Start(_ECR_WIFI_RECEIVE_TIMER_, srECROb->srSetting.inTimeout);
	
	/* 初始化放收到資料的陣列 */
	memset(szReceiveBuffer, 0x00, sizeof(szReceiveBuffer));
	usReceiveBufferSize = 0;

	while (1)
	{
		/* 超過重試次數 */
		if (inRetry > inRecMaxRetry)
		{
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("Retry too many times!");
			}
			
			return (VS_ERROR);
		}
		
		memset(uszTempBuffer, 0x00, sizeof(uszTempBuffer));
		inTempReceiveSize = 0;
		
		do
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_WIFI_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			inTempReceiveSize = sizeof(uszTempBuffer);
			inETHERNET_Receive_Data_By_Native(ginECR_ResponseFd, uszTempBuffer, (unsigned short*)&inTempReceiveSize);
		} while(inTempReceiveSize <= 0);
		
		memcpy(&szReceiveBuffer[usReceiveBufferSize], uszTempBuffer, inTempReceiveSize);
		usReceiveBufferSize += inTempReceiveSize;
		
		/* 可以開始判斷長度 */
		if (usReceiveBufferSize >= 8	&& 
		    inDataSize == 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &szReceiveBuffer[4], 4);
			inDataSize = 8 + atoi(szTemplate) + 5;
			
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf("inDataSize: %d", inDataSize);
			}
		}
		
		/* 長度符合後跳出 */
		if (inDataSize > 0	&& 
		    usReceiveBufferSize == inDataSize)
		{
			break;
		}
	}
	
	/* 把資料放到szrRealReceBuffer */
	/* 根據台新ICMP 連線規格 */
	memcpy(szDataBuffer, &szReceiveBuffer[0], usReceiveBufferSize);
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_ECR_Send
Date&Time       :2017/6/2 下午 2:14
Describe        :傳送要給收銀機的資料
*/
int inWiFi_ECR_Send(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer, int inDataSize)
{
	int	i = 0;
	int	inRetVal;
	int	inRetry = 0;
	int	inRecMaxRetry = _ECR_WIFI_RETRYTIMES_;					/* 最大重試次數 */
	int	inSendLen = 0;
	char	szSendBuf[_ECR_WIFI_BUFF_SIZE_];					/* 包含STX、ETX、LRC的電文 */
	
	/* 把Data Buffer(純資料)裡所有0x00值改成0x20 */
	for (i = 0; i < inDataSize; i++)
	{
		if (szDataBuffer[i] == 0x00)
		{
			szDataBuffer[i] = 0x20;
		}
	}
	
	/* 在要傳送Buffer裡放要傳送的資料 */
	memcpy(&szSendBuf[inSendLen], szDataBuffer, inDataSize);
	inSendLen += inDataSize;

	while (1)
	{

		/* 測試Flag */
		if (ginEngineerDebug == VS_TRUE)
		{
			inRetVal = VS_SUCCESS;
		}
		else
		{
			/* 經由port傳送資料 */
			inRetVal = inETHERNET_Send_Data_By_Native(ginECR_ResponseFd, (unsigned char*)szSendBuf, (unsigned short)inSendLen);
		}
		
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		else
		{
			
			/* 傳送Retry */
			if (inRetry < inRecMaxRetry)
			{
				/* 成功 */
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf("ECR ACK OK!");
				}
				return (VS_SUCCESS);
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

		}/* inRS232_Data_Send */
	
	}/* while(1) */
	
}

/*
Function        :inWiFi_ECR_Pepper_Demo_Initial
Date&Time       :2017/6/2 上午 10:46
Describe        :
*/
int inWiFi_ECR_00_Pepper_Demo_Initial(ECR_TABLE *srECROb)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1];
	
	/* 開port */
	inRetVal = inWiFi_Open();
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "WiFi Open OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "WiFi Open Not OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	inRetVal = inWiFi_Server_Open();
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inWiFi_Server_Open OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inWiFi_Server_Open Not OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (inRetVal);
}

/*
Function        :inWiFi_ECR_Pepper_Demo_Receive_Packet
Date&Time       :2017/6/2 上午 10:53
Describe        :處理收銀機傳來的資料
*/
int inWiFi_ECR_00_Pepper_Demo_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_WIFI_BUFF_SIZE_];
	
	
	memset(szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	if (ginDisplayDebug == VS_TRUE)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("Debug: 開始讀取buffer", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);
	}
	
/* 測試Flag */
	if (ginEngineerDebug == VS_TRUE)
	{
		char	szDemoData[235 + 1];
		memset(szDemoData, 0x00, sizeof(szDemoData));
	
		/* 這邊塞假ECR內容 */

		memcpy(szDataBuffer, szDemoData, strlen(szDemoData));
	
		inRetVal = VS_SUCCESS;
	}
	else
	{
		inRetVal = inWiFi_ECR_Receive(pobTran, srECROb, szDataBuffer, _Pepper_Demo_Data_Size_);
	}
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	else
	{
		/* 列印紙本電文和顯示電文訊息 */
		inECR_Print_Receive_ISODeBug(szDataBuffer, _Pepper_Nestle_Sale_Data_Size_, _Pepper_Nestle_Sale_Data_Size_);
		
		if (ginDisplayDebug == VS_TRUE)
		{
			inDISP_Clear_Line(_LINE_8_6_, _LINE_8_8_);
			inDISP_ChineseFont("Debug: 讀取Buffer成功", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_CENTER_);
		}
	}
/* -----------------------開始分析資料------------------------------------------ */
	if (ginDisplayDebug == VS_TRUE)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("Debug: 開始分析資料", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);
	}
	
	inRetVal = inWiFi_ECR_00_Pepper_Demo_Unpack(pobTran, srECROb, szDataBuffer);

	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	else
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			inDISP_Clear_Line(_LINE_8_6_, _LINE_8_8_);
			inDISP_ChineseFont("Debug: 分析資料完成", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_CENTER_);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_ECR_Pepper_Demo_Unpack
Date&Time       :2017/6/2 下午 1:29
Describe        :分析收銀機傳來的資料
*/
int inWiFi_ECR_00_Pepper_Demo_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer)
{
	int	inRetVal;
	char	szAmount[12 + 1];
	char	szDispMsg[22 + 1];
	
	memset(szAmount, 0x00, sizeof(szAmount));
	memcpy(szAmount, szDataBuffer, 10);

	/* 分析資料 */
	pobTran->srBRec.lnTxnAmount = atol(szAmount);
	
	/*金額不合法*/
	if (pobTran->srBRec.lnTxnAmount <= 0)
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "%s :%ld", "Debug: 金額不合法", pobTran->srBRec.lnTxnAmount);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);
		}
		return (VS_ERROR);
	}
	
	inDISP_ClearAll();
	/* 第一層顯示第一行 ＜一般交易＞ */
	inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_1_);

	pobTran->inFunctionID = _SALE_;
	pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
	pobTran->inRunTRTID = _TRT_SALE_CTLS_;

	pobTran->inTransactionCode = _SALE_;
	pobTran->srBRec.inCode = _SALE_;
	pobTran->srBRec.inOrgCode = _SALE_;

	inRetVal = inFLOW_RunOperation(pobTran, _OPERATION_SALE_CTLS_);
	
	return (inRetVal);
}

/*
Function        :inWiFi_ECR_Pepper_Demo_Pack
Date&Time       :2017/10/5 下午 4:52
Describe        :先把要送的資料組好
*/
int inWiFi_ECR_00_Pepper_Demo_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer)
{
	/* 這裡組回傳成功或失敗的訊息 */
	memcpy(szDataBuffer, "Success     ", _Pepper_Demo_Data_Size_);
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_ECR_Pepper_Demo_Send_Packet
Date&Time       :2017/6/2 下午 2:43
Describe        :處理要送給收銀機的資料
*/
int inWiFi_ECR_00_Pepper_Demo_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_WIFI_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransDataTSB.uszIsResponce == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inWiFi_ECR_00_Pepper_Demo_Pack(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	if (ginDisplayDebug == VS_TRUE)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("Debug: 開始回傳結果", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);
	}
	
	inRetVal = inWiFi_ECR_Send(pobTran, srECROb, szDataBuf, _Pepper_Demo_Data_Size_);
	/*---------------------------------- 印Debug 開始---------------------------------------------------------------*/
	/* 列印紙本電文和顯示電文訊息 */
	inECR_Print_Send_ISODeBug(szDataBuf, _Pepper_Demo_Data_Size_, _Pepper_Demo_Data_Size_);
	/*---------------------------------- 印Debug 結束---------------------------------------------------------------*/
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	else
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			inDISP_Clear_Line(_LINE_8_6_, _LINE_8_8_);
			inDISP_ChineseFont("Debug: 回傳結果完成", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_CENTER_);
		}
	}
	
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransDataTSB.uszIsResponce = VS_TRUE;
	/* 結束後，重置並開新thread */
	inWiFi_Close_Client();
	inWiFi_Create_Accept_Thread();
			
        return (VS_SUCCESS);
}

/*
Function        :inWiFi_ECR_Pepper_Demo_Send_Error
Date&Time       :2017/6/2 下午 1:35
Describe        :傳送錯誤訊息ECR
*/
int inWiFi_ECR_00_Pepper_Demo_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_WIFI_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransDataTSB.uszIsResponce == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inWiFi_ECR_00_Pepper_Demo_Pack(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	/* 這裡組回傳成功或失敗的訊息 */
	memcpy(szDataBuf, "Fail        ", 12);
	
/* ---------------------傳送電文--------------------------------------------- */
	/* Send message */  
	inRetVal = inWiFi_ECR_Send(pobTran, srECROb, szDataBuf, _Pepper_Demo_Data_Size_);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransDataTSB.uszIsResponce = VS_TRUE;
	/* 結束後，重置並開新thread */
	inWiFi_Close_Client();
	inWiFi_Create_Accept_Thread();
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_ECR_Pepper_Demo_Close
Date&Time       :2016/7/11 下午 3:34
Describe        :關閉Handle
*/
int inWiFi_ECR_00_Pepper_Demo_Close(ECR_TABLE* srECRob)
{
	/*清空接收的buffer*/
	
	/* 關閉連線 */
        inWiFi_Close_Client();

        return (VS_SUCCESS);
}

/*
Function        :inWiFi_ECR_Pepper_Nestle_Initial
Date&Time       :2017/6/2 上午 10:46
Describe        :
*/
int inWiFi_ECR_01_Pepper_Nestle_Initial(ECR_TABLE *srECROb)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1];
	
	/* 開port */
	inRetVal = inWiFi_Open();
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "WiFi Open OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "WiFi Open Not OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	inRetVal = inWiFi_Server_Open();
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inWiFi_Server_Open OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inWiFi_Server_Open Not OK");
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (inRetVal);
}

/*
Function        :inWiFi_ECR_Pepper_Nestle_Receive_Packet
Date&Time       :2017/6/2 上午 10:53
Describe        :處理收銀機傳來的資料
*/
int inWiFi_ECR_01_Pepper_Nestle_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuffer[_ECR_WIFI_BUFF_SIZE_];
	
	memset(szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	if (ginDisplayDebug == VS_TRUE)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("Debug: 開始讀取buffer", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);
	}
	
	/* 測試Flag */
	if (ginEngineerDebug == VS_TRUE)
	{
		char	szDemoData[235 + 1];
		memset(szDemoData, 0x00, sizeof(szDemoData));
		memcpy(szDemoData, _ECR_WIFI_DEMO_DATA_, strlen(_ECR_WIFI_DEMO_DATA_));
		/* 這邊有一個很怪的BUG，抓到szDemoData內的資料最前面會多一個0x02，所以改抓第一個位置 */
		memcpy(szDataBuffer, &szDemoData[1], strlen(szDemoData) - 1);
	
		inRetVal = VS_SUCCESS;
	}
	else
	{
		inRetVal = inWiFi_ECR_Receive_01_TSB(pobTran, srECROb, szDataBuffer, _Pepper_Nestle_Sale_Data_Size_);
	}
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	else
	{
		/* 列印紙本電文和顯示電文訊息 */
		inECR_Print_Receive_ISODeBug(szDataBuffer, _Pepper_Nestle_Sale_Data_Size_, _Pepper_Nestle_Sale_Data_Size_);
		
		if (ginDisplayDebug == VS_TRUE)
		{
			inDISP_Clear_Line(_LINE_8_6_, _LINE_8_8_);
			inDISP_ChineseFont("Debug: 讀取Buffer成功", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_CENTER_);
		}
	}
/* -----------------------開始分析資料------------------------------------------ */
	if (ginDisplayDebug == VS_TRUE)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("Debug: 開始分析資料", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);
	}
	
	inRetVal = inWiFi_ECR_01_Pepper_Nestle_Unpack(pobTran, srECROb, szDataBuffer);

	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	else
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			inDISP_Clear_Line(_LINE_8_6_, _LINE_8_8_);
			inDISP_ChineseFont("Debug: 分析資料完成", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_CENTER_);
		}	
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_ECR_Pepper_Nestle_Unpack
Date&Time       :2017/6/2 下午 1:29
Describe        :分析收銀機傳來的資料
*/
int inWiFi_ECR_01_Pepper_Nestle_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer)
{
	int	inRetVal = VS_ERROR;
	int	inCnt = 0;
	int	inDataLen = 0;
	int	inTransType = 0;
	char	szCTLSEnable[2 + 1];
	char	szTemplate[100 + 1];
	char	szDebugMsg[100 + 1];
	
	/* STX B1 */
	inCnt += 1;
	
	/* SQNo B2 */
	memcpy(srECROb->srTransDataTSB.szSQNo, &szDataBuffer[1], 1);
	inCnt += 1;
	
	/* CMD B3~B4*/
	memcpy(srECROb->srTransDataTSB.szTransType, &szDataBuffer[2], 2);
	inTransType = atoi(srECROb->srTransDataTSB.szTransType);
	inCnt += 2;
	
	/* DATALEN B5~B8 */
	memcpy(srECROb->srTransDataTSB.szDataLen, &szDataBuffer[4], 4);
	inDataLen = atoi(srECROb->srTransDataTSB.szDataLen);
	inCnt += 4;
	
	/* DATA B9~BN */
	switch (inTransType)
	{
		case _ECR_WIFI_01_TRANSTYPE_SALE_NO_:
			/* HostID */
			inCnt += 2;
			
			/* InvoiceNo */
			inCnt += 6;
			
			/* CardNo */
			inCnt += 19;
			
			/* NoSignatureFlag */
			inCnt += 1;

			/* TransPayWay */
			inCnt += 1;

			/* TransAmount */
			memcpy(szTemplate, &szDataBuffer[37], 10);
			pobTran->srBRec.lnTxnAmount = atol(szTemplate);
			inCnt += 12;
			
			/* 金額檢核 */
			if (pobTran->srBRec.lnTxnAmount == 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "金額為0");
					inDISP_LogPrintf(szDebugMsg);
				}
				gsrECROb.srTransDataTSB.inErrorType = _ECR_RESPONSE_CODE_AMOUNT_ERROR_;
				
				return (VS_ERROR);
			}
			else if (pobTran->srBRec.lnTxnAmount > 3000)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "金額太大");
					inDISP_LogPrintf(szDebugMsg);
				}
				gsrECROb.srTransDataTSB.inErrorType = _ECR_RESPONSE_CODE_AMOUNT_ERROR_;
				
				return (VS_ERROR);
			}

			/* TransDate */
			inCnt += 6;

			/* TransTime */
			inCnt += 6;

			/* ApprovalNo */
			inCnt += 6;

			/* TID */
			inCnt += 8;

			/* RRN */
			inCnt += 12;

			/* BatchNo */
			inCnt += 6;

			/* StoreID */
			memcpy(pobTran->srBRec.szStoreID, &szDataBuffer[inCnt], 18);
			inCnt += 18;

			/* CardType */
			inCnt += 2;
			
			/* TC */
			inCnt += 16;

			/* TipsFlag */
			inCnt += 1;

			/* TipsPercent */
			inCnt += 2;

			/* CUP/Inst/Bonus (銀聯/分期/紅利 資訊)*/
			inCnt += 96;
			
			if (inDataLen > 220)
			{
				/* MID */
				inCnt += 15;
			}
			
			break;
		default:
			inRetVal = VS_ERROR;
			break;
			
	};
	
	/* CRC BN+1 ~ BN+4 */
	memcpy(srECROb->srTransDataTSB.szCRC, &szDataBuffer[inCnt], 4);
	inCnt += 4;
	
	/* ETX BN+5 */
	inCnt += 1;
	
	switch (inTransType)
	{
		case _ECR_WIFI_01_TRANSTYPE_SALE_NO_:
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

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		default:
			
			inRetVal = VS_ERROR;
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inWiFi_ECR_Pepper_Nestle_Pack
Date&Time       :2017/10/5 下午 4:52
Describe        :先把要送的資料組好
*/
int inWiFi_ECR_01_Pepper_Nestle_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer)
{
	int 	i = 0, inCardLen = 0;
	int	inCnt = 0;
	int	inDataLen = 0;
	char	szTemplate[100 + 1];
	char	szHostLabel[8 + 1];
	
	/* STX B1 */
	szDataBuffer[0] = 'S';
	inCnt += 1;
	
	/* SQNo B2 */
	memcpy(&szDataBuffer[1], srECROb->srTransDataTSB.szSQNo, 1);
	inCnt += 1;
	
	/* CMD B3~B4*/
	memcpy(&szDataBuffer[2], srECROb->srTransDataTSB.szTransType, 2);
	inCnt += 2;
	
	/* DATALEN B5~B8 */
	inDataLen = atoi(srECROb->srTransDataTSB.szDataLen);
	/* 加上RESPONSE CODE的長度(8 byte)*/
	inDataLen += 8;
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%04d", inDataLen);
	memcpy(&szDataBuffer[4], szTemplate, 4);
	inCnt += 4;
	
	/* RESPONSE CODE B9 ~ B16 */
	/* 指明CMD執行結果，00000000=CMD執行成功；其餘失敗 */
	memcpy(&szDataBuffer[8], "00000000", 8);
	inCnt += 8;
	
	/* DATA B17 ~ BN */
	/* Host ID */
	memset(szHostLabel, 0x00, sizeof(szHostLabel));
	inGetHostLabel(szHostLabel);
	
	if (memcmp(szHostLabel, _HOST_NAME_DINERS_, strlen(_HOST_NAME_DINERS_)) == 0)
	{
		memcpy(&szDataBuffer[inCnt], _ECR_WIFI_01_HOSTID_DINERS_, strlen(_ECR_WIFI_01_HOSTID_DINERS_));
	}
	else if (memcmp(szHostLabel, _HOST_NAME_AMEX_, strlen(_HOST_NAME_AMEX_)) == 0)
	{
		memcpy(&szDataBuffer[inCnt], _ECR_WIFI_01_HOSTID_AMEX_, strlen(_ECR_WIFI_01_HOSTID_AMEX_));
	}
	else
	{
		memcpy(&szDataBuffer[inCnt], _ECR_WIFI_01_HOSTID_TSB_, strlen(_ECR_WIFI_01_HOSTID_TSB_));
	}
	inCnt += 2;
			
	/* InvoiceNo */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
	memcpy(&szDataBuffer[inCnt], szTemplate, 6);
	inCnt += 6;
			
	/* CardNo */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcpy(szTemplate, pobTran->srBRec.szPAN);

	/* HAPPG_GO 卡不掩飾 */
	if (!memcmp(&pobTran->srBRec.szPAN[0], "9552", 4))
	{

	}
	else
	{
		if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
		{
			inFunc_FormatPAN_UCARD(szTemplate);
		}

		/* 卡號長度 */
		inCardLen = strlen(szTemplate);

		/* 前6後4為明碼 */
		if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
		{
			for (i = 2; i < (inCardLen - 4); i ++)
				szTemplate[i] = '*';
		}
		else
		{
			for (i = 6; i < (inCardLen - 4); i ++)
				szTemplate[i] = '*';
		}

	}
	
	if (strlen(szTemplate) > 0)
	{
		memcpy(&szDataBuffer[inCnt], szTemplate, strlen(szTemplate));
	}
	else
	{
		memcpy(&szDataBuffer[inCnt], "                   ", 19);
	}
	inCnt += 19;
			
	/* NoSignatureFlag */
	if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
	{
		memcpy(&szDataBuffer[inCnt], "Y", strlen("Y"));
	}
	else
	{
		memcpy(&szDataBuffer[inCnt], "N", strlen("N"));
	}
	inCnt += 1;

	/* TransPayWay */
	/* 過卡方式 */
	if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
	{
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
			memcpy(&szDataBuffer[inCnt], "C", strlen("C"));
		else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			memcpy(&szDataBuffer[inCnt], "W", strlen("W"));
		else
		{
			if (pobTran->srBRec.uszManualBit == VS_TRUE)
			{
				/* 【需求單-105244】端末設備支援以感應方式進行退貨交易 */
				/* 電文轉Manual Keyin但是簽單要印感應的W */
				if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
					memcpy(&szDataBuffer[inCnt], "W", strlen("W"));
				else
					memcpy(&szDataBuffer[inCnt], "M", strlen("M"));
			}
			else
				memcpy(&szDataBuffer[inCnt], "S", strlen("S"));
		}

	}
	else
	{
		if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			memcpy(&szDataBuffer[inCnt], "W", strlen("W"));
		else
			memcpy(&szDataBuffer[inCnt], "C", strlen("C"));
	}
	inCnt += 1;

	/* TransAmount */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);
	memcpy(&szDataBuffer[inCnt], szTemplate, 12);
	inCnt += 12;

	/* TransDate */
	memcpy(&szDataBuffer[inCnt], &pobTran->srBRec.szDate[2], 6);
	inCnt += 6;

	/* TransTime */
	memcpy(&szDataBuffer[inCnt], pobTran->srBRec.szTime, 6);
	inCnt += 6;

	/* ApprovalNo */
	if (strlen(pobTran->srBRec.szAuthCode) > 0)
	{
		memcpy(&szDataBuffer[inCnt], pobTran->srBRec.szAuthCode, 6);
	}
	else
	{
		memcpy(&szDataBuffer[inCnt], "      ", 6);
	}
	inCnt += 6;

	/* TID */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);
	memcpy(&szDataBuffer[inCnt], szTemplate, 8);
	inCnt += 8;

	/* RRN */
	if (strlen(pobTran->srBRec.szRefNo) > 0)
	{
		memcpy(&szDataBuffer[inCnt], pobTran->srBRec.szRefNo, 12);
	}
	else
	{
		memcpy(&szDataBuffer[inCnt], "            ", 12);
	}
	inCnt += 12;

	/* BatchNo */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%06ld", pobTran->srBRec.lnBatchNum);
	memcpy(&szDataBuffer[inCnt], szTemplate, 6);
	inCnt += 6;

	/* StoreID */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", pobTran->srBRec.szStoreID);
	memcpy(&szDataBuffer[inCnt], szTemplate, 18);
	inCnt += 18;

	/* CardType */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcpy(szTemplate, pobTran->srBRec.szCardLabel);

	if (!memcmp(szTemplate, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)))
		memcpy(&szDataBuffer[inCnt], _ECR_WIFI_01_CARDTYPE_VISA_, 2);
	else if (!memcmp(szTemplate, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
		memcpy(&szDataBuffer[inCnt], _ECR_WIFI_01_CARDTYPE_MASTERCARD_, 2);
	else if (!memcmp(szTemplate, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)))
		memcpy(&szDataBuffer[inCnt], _ECR_WIFI_01_CARDTYPE_JCB_, 2);
	else if (!memcmp(szTemplate, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
		memcpy(&szDataBuffer[inCnt], _ECR_WIFI_01_CARDTYPE_UCARD_, 2);
	else if (!memcmp(szTemplate, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)))
		memcpy(&szDataBuffer[inCnt], _ECR_WIFI_01_CARDTYPE_DINERS_, 2);
	else if (!memcmp(szTemplate, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
		memcpy(&szDataBuffer[inCnt], _ECR_WIFI_01_CARDTYPE_AMEX_, 2);
	else if (!memcmp(szTemplate, _CARD_TYPE_SMARTPAY_, strlen(_CARD_TYPE_SMARTPAY_)))
		memcpy(&szDataBuffer[inCnt], _ECR_WIFI_01_CARDTYPE_SMARTPAY_, 2);
	else if (!memcmp(szTemplate, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_)))
		memcpy(&szDataBuffer[inCnt], _ECR_WIFI_01_CARDTYPE_CUP_, 2);
	else
		memcpy(&szDataBuffer[inCnt], "FF", 2);
	inCnt += 2;
			
	/* TC */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%02X%02X%02X%02X%02X%02X%02X%02X",
										pobTran->srEMVRec.usz9F26_ApplCryptogram[0],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[1],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[2],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[3],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[4],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[5],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[6],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[7]);
	if (strlen(szTemplate) > 0)
	{
		memcpy(&szDataBuffer[inCnt], szTemplate, 16);
	}
	else
	{
		memcpy(&szDataBuffer[inCnt], "                ", 16);
	}
	inCnt += 16;

	/* TipsFlag */
	memcpy(&szDataBuffer[inCnt], "N", 1);
	inCnt += 1;

	/* TipsPercent */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTipPercent(szTemplate);
	memcpy(&szDataBuffer[inCnt], &szTemplate[1], 2);
	inCnt += 2;

	/* CUP/Inst/Bonus (銀聯/分期/紅利 資訊)*/
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		
	}
	else if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		
	}
	else if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		
	}
	inCnt += 96;
	
	if (atoi(srECROb->srTransDataTSB.szDataLen) > 220)
	{
		/* MID */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetMerchantID(szTemplate);
		memcpy(&szDataBuffer[inCnt], szTemplate, 15);
		inCnt += 15;
	}
	
	/* CRC BN+1 ~ BN+4 */
	if (strlen(srECROb->srTransDataTSB.szCRC) > 0)
	{
		memcpy(&szDataBuffer[inCnt], srECROb->srTransDataTSB.szCRC, 4);
	}
	else
	{
		memcpy(&szDataBuffer[inCnt], "    ", 4);
	}
	inCnt += 4;
	
	/* ETX BN+5 */
	memcpy(&szDataBuffer[inCnt], "E", 1);
	inCnt += 1;
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_ECR_Pepper_Nestle_Send_Packet
Date&Time       :2017/6/2 下午 2:43
Describe        :處理要送給收銀機的資料
*/
int inWiFi_ECR_01_Pepper_Nestle_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_WIFI_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransDataTSB.uszIsResponce == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inWiFi_ECR_01_Pepper_Nestle_Pack(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	if (ginDisplayDebug == VS_TRUE)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("Debug: 開始回傳結果", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);
	}
	
	inRetVal = inWiFi_ECR_Send(pobTran, srECROb, szDataBuf, _Pepper_Nestle_Sale_Data_Size_ + 8);
	/*---------------------------------- 印Debug 開始---------------------------------------------------------------*/
	/* 列印紙本電文和顯示電文訊息 */
	inECR_Print_Send_ISODeBug(szDataBuf, _Pepper_Nestle_Sale_Data_Size_ + 8,  _Pepper_Nestle_Sale_Data_Size_ + 8);
	/*---------------------------------- 印Debug 結束---------------------------------------------------------------*/
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	else
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			inDISP_Clear_Line(_LINE_8_6_, _LINE_8_8_);
			inDISP_ChineseFont("Debug: 回傳結果完成", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_CENTER_);
		}
	}
	
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransDataTSB.uszIsResponce = VS_TRUE;
	/* 結束後，重置並開新thread */
	inWiFi_Close_Client();
	inWiFi_Create_Accept_Thread();
			
        return (VS_SUCCESS);
}

/*
Function        :inWiFi_ECR_Pepper_Nestle_Send_Error
Date&Time       :2017/6/2 下午 1:35
Describe        :傳送錯誤訊息ECR
*/
int inWiFi_ECR_01_Pepper_Nestle_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_WIFI_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransDataTSB.uszIsResponce == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inWiFi_ECR_01_Pepper_Nestle_Pack(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	/* 這裡組回傳成功或失敗的訊息 */
	if (srECROb->srTransData.inErrorType == VS_TIMEOUT		||
	    srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_TIMEOUT_)
	{
		memcpy(&szDataBuf[8], "1XXX0102", 8);
	}
	else if (srECROb->srTransData.inErrorType == VS_ISO_PACK_ERR		|| 
		 srECROb->srTransData.inErrorType == VS_ISO_UNPACK_ERROR	||
		 srECROb->srTransData.inErrorType == VS_COMM_ERROR)
	{
		memcpy(&szDataBuf[8], "1XXX0160", 8);
	}
	else if (srECROb->srTransData.inErrorType == VS_USER_CANCEL || 
		 srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_USER_TERMINATE_ERROR_)
	{
		memcpy(&szDataBuf[8], "1XXX0101", 8);
	}
	else if (srECROb->srTransDataTSB.inErrorType == _ECR_RESPONSE_CODE_AMOUNT_ERROR_)
	{
		memcpy(&szDataBuf[8], "1XXX0126", 8);
	}
	else if (srECROb->srTransDataTSB.inErrorType == VS_CARD_EXP_ERR)
	{
		memcpy(&szDataBuf[8], "1xxx0110", 8);
	}
	else if (srECROb->srTransDataTSB.inErrorType == VS_NO_CARD_BIN)
	{
		memcpy(&szDataBuf[8], "1xxx0111", 8);
	}
	else if (srECROb->srTransDataTSB.inErrorType == VS_WAVE_ERROR)
	{
		memcpy(&szDataBuf[8], "1xxx0119", 8);
	}
	else if (srECROb->srTransDataTSB.inErrorType == VS_CARD_PAN_ERROR)
	{
		memcpy(&szDataBuf[8], "1xxx011C", 8);
	}
	else
	{
		if (pobTran->inTransactionResult == _TRAN_RESULT_DECLINED_)
		{
			memcpy(&szDataBuf[8], "2XXX0005", 8);
		}
		else
		{
			memcpy(&szDataBuf[8], "1XXX0000", 8);
		}
	}
	
	
/* ---------------------傳送電文--------------------------------------------- */
	/* Send message */
	inRetVal = inWiFi_ECR_Send(pobTran, srECROb, szDataBuf, _Pepper_Nestle_Sale_Data_Size_ + 8);
	/*---------------------------------- 印Debug 開始---------------------------------------------------------------*/
	/* 列印紙本電文和顯示電文訊息 */
	inECR_Print_Send_ISODeBug(szDataBuf, _Pepper_Nestle_Sale_Data_Size_ + 8,  _Pepper_Nestle_Sale_Data_Size_ + 8);
	/*---------------------------------- 印Debug 結束---------------------------------------------------------------*/
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransDataTSB.uszIsResponce = VS_TRUE;
	/* 結束後，重置並開新thread */
	inWiFi_Close_Client();
	inWiFi_Create_Accept_Thread();
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_ECR_Pepper_Nestle_Close
Date&Time       :2016/7/11 下午 3:34
Describe        :關閉Handle
*/
int inWiFi_ECR_01_Pepper_Nestle_Close(ECR_TABLE* srECRob)
{
	/* 清空接收的Buffer */
	
	/* 關閉連線 */
        inWiFi_Close_Client();

        return (VS_SUCCESS);
}

#include <ctos_tls2.h>
#include <openssl/ssl.h>
int inWiFi_TLS_Test()
{
	unsigned int	uiCTX_ID;
	unsigned int	uiSSL_ID;		/* SSL Object ID */
	CTOS_TLS2_CTX_New(d_TLS2_METHOD_CLIENT_TLS_V1_2, &uiCTX_ID);
	CTOS_TLS2_CTX_LoadCACertificationFromFile(uiCTX_ID, (unsigned char*)"nccc.com.tw.pem");
	CTOS_TLS2_CTX_SetVerificationMode(uiCTX_ID, d_TLS2_VERIFY_PEER);
	CTOS_TLS2_New(uiCTX_ID, &uiSSL_ID);
	/* 設定驗證方式 */
	CTOS_TLS2_SetVerificationMode(uiSSL_ID, SSL_VERIFY_NONE);
	/* 設定加密列表 */
	CTOS_TLS2_SetCipherList(uiSSL_ID, (unsigned char*)"ALL");	
	/* 設定socket */
	CTOS_TLS2_SetSocket(uiSSL_ID, ginTrans_ClientFd);	
	/* 設定協議標準 */
	CTOS_TLS2_SetProtocolVersion(uiSSL_ID, d_TLS2_VERSION_TLSV1_2);
//	CTOS_TLS2_Connect(uiSSL_ID);
//	
//	CTOS_TLS2_Disconnect(uiSSL_ID, 0);
	CTOS_TLS2_Free(uiSSL_ID);
	CTOS_TLS2_CTX_Free(uiCTX_ID);
	
	TRANSACTION_OBJECT	pobTran;
	memset(&pobTran, 0x00, sizeof(pobTran));
	
//	inCOMM_ConnectStart(&pobTran);
//	
//	inCOMM_End(&pobTran);
	
	
	return (VS_SUCCESS);
}

int inWiFi_SetConfig_Costco(void)
{
        int	inRetVal = 0;
	int	inConnectMaxCnt = 1;		/* 連線最大重試次數(含第一次) */
	int	inConnectNowCnt = 0;		/* 連線目前重試次數 */
        char	szHostIPPrimary[16 + 1];
	char	szHostIPSecond[16 + 1];
	char	szHostPortNoPrimary[6 + 1];
	char	szHostPortNoSecond[6 + 1];
	char	szConfig[16 + 1];
	char	szDebugMsg[100 + 1];
	char	szDHCPMode[2 + 1];
	char	szAutoConnect[10 + 1];

        
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
        
        /* Auto Connect */
	memset(szConfig, 0x00, sizeof(szConfig));
	memset(szAutoConnect, 0x00, sizeof(szAutoConnect));
	inGetAutoConnect(szAutoConnect);
	if (szAutoConnect[1] == '1')
	{
		szConfig[0] = '1';
	}
	else
	{
		szConfig[0] = '0';
	}
	
	inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_AUTOCON_AP, (unsigned char*)szConfig, 1);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("d_WIFI_CONFIG_AUTOCON_AP Error");
		}
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
		
		inRetVal = inWiFi_Connect_Flow(szHostIPPrimary, szHostPortNoPrimary);
		inConnectNowCnt ++;
		
		if (ginDebug == VS_TRUE)
		{
			inFunc_WatchRunTime();
		}
		
	}while (inRetVal != VS_SUCCESS && inConnectNowCnt < inConnectMaxCnt);
        
        /* 連線成功 */
	if (inRetVal != VS_SUCCESS)
	{
		inWiFi_END();
                return (VS_ERROR);
	}
	return VS_SUCCESS;
}
