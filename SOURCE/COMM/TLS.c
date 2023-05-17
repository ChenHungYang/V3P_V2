#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <ctos_tls2.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sqlite3.h>

#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../EVENT/MenuMsg.h"
#include "../FUNCTION/FILE_FUNC/File.h"
#include "../FUNCTION/FILE_FUNC/FIleLogFunc.h"
#include "../FUNCTION/Sqlite.h"
#include "../FUNCTION/Function.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/HDPT.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../COMM/Comm.h"
#include "../COMM/Ethernet.h"
#include "TLS.h"

extern int	ginDebug;
extern int	ginTrans_ClientFd;

unsigned int	guiCTX_ID = 0;		/* Context Object ID */
unsigned int	guiSSL_ID = 0;		/* SSL Object ID */
//static int st_CreateCTX;
//static int st_CreateSeccion;

/* TLS 流程參考(講的滿清楚的，所以我把表格複製下來了)
 * (HTTPS到底是个啥玩意儿？ https://mp.weixin.qq.com/s?__biz=MzA3MDExNzcyNA==&mid=402053009&idx=1&sn=ea531fc21a07d33f8a0408e5206c60f3)
 * 
 * 	Client				|		Server
 * 	Client  Hello			|
 * 	你好！				|
 * ----------------------------------------------------------------------------------------
 *					|		Server Hello
 *					|		嗯，你好！
 *-----------------------------------------------------------------------------------------
 *					|		Certificate
 *					|		我的证书给你，验证我吧
 *-----------------------------------------------------------------------------------------
 *					|		Server Key Exchange
 *					|		这是我给你的加密密钥相关的东东
 *-----------------------------------------------------------------------------------------
 *					|		Server Hello Done
 *					|		好，我说完了
 *-----------------------------------------------------------------------------------------
 * 	Client Key Exchange		|		
 *	这是我给你的加密密钥相关的东东	|		
 *-----------------------------------------------------------------------------------------
 *	Change Cipher Spec		|		
 *	准备转换成密文了哦		|		
 *-----------------------------------------------------------------------------------------
 *	Encrypted  Handshake Message	|		
 *	%……&*4 （密文思密达）		|		
 *-----------------------------------------------------------------------------------------
 *					|		Change Cipher Spec
 *					|		我也转换密文了
 *-----------------------------------------------------------------------------------------
 *					|		Encrypted  Handshake Message
 *					|		#%&……* （密文思密达）
 *-----------------------------------------------------------------------------------------
 *	Application  Data		|		
 *	%&￥&%*……（HTTP密文数据）	|		
 *------------------------------------------------------------------------------------------
 *					|		Application  Data
 *					|		**……&%（HTTP密文数据）
 *------------------------------------------------------------------------------------------
 *	Encrypted  Alert		|
 *	警告（实际就是说完了，拜拜~）	|
 */


/*
Function        :inTLS_Init
Date&Time       :2017/7/28 下午 5:07
Describe        :在使用TLS之前至少call過一次
*/
int inTLS_Init()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_TLS2_Init();;
	if (usRetVal == d_OK)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, " Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}
/*
Function        :inTLS_Context_New
Date&Time       :2017/7/28 下午 4:58
Describe        :建立一個新的SSL Context for library使用
*/
int inTLS_CTX_New(int inMethod, unsigned int *inCTX_ID)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_TLS2_CTX_New(inMethod, inCTX_ID);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "TLS2 New context OK CTX: %u", *inCTX_ID);
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "TLS2 New context Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
        
                    inDISP_DispLogAndWriteFlie("  TLS New *Error* usRetVal[0x%04X] Line[%d] ", usRetVal,  __LINE__);
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inTLS_CTX_LoadCACertificationFromFile
Date&Time       :2017/7/31 上午 11:03
Describe        :指定檔案讀取CA憑證
*/
int inTLS_CTX_LoadCACertificationFromFile(unsigned int uiCTX_ID,unsigned char *uszFileName)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_TLS2_CTX_LoadCACertificationFromFile(uiCTX_ID, uszFileName);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("CTX Load CA Cer OK");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTX Load CA Cer Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
                    inDISP_DispLogAndWriteFlie("  CTX Load CA Cer *Error* usRetVal[0x%04X] Line[%d] ", usRetVal,  __LINE__);
        
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inTLS_CTX_LoadCACertificationFromFile
Date&Time       :2017/7/31 上午 11:03
Describe        :指定檔案讀取憑證(Client)
*/
int inTLS_CTX_LoadCertificationFromFile(unsigned int uiCTX_ID,unsigned char *uszFileName, int inFiletype)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_TLS2_CTX_LoadCertificateFromFile(uiCTX_ID, uszFileName, inFiletype);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("CTX load Cer OK");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTX load Cer Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inTLS_CTX_LoadCACertificationFromFile
Date&Time       :2017/7/31 上午 11:03
Describe        :指定檔案讀取kry(Clinet)
*/
int inTLS_CTX_LoadPrivateKeyFromFile(unsigned int uiCTX_ID,unsigned char *uszFileName, int inFiletype)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_TLS2_CTX_LoadPrivateKeyFromFile(uiCTX_ID, uszFileName, inFiletype);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("CTX load key OK");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTX load key Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inTLS_CTX_SetVerificationMode
Date&Time       :2017/7/31 上午 11:42
Describe        :設定SSL Context的驗證方式
*/
int inTLS_CTX_SetVerificationMode(unsigned int uiCTX_ID,unsigned int uiMode)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_TLS2_CTX_SetVerificationMode(uiCTX_ID, uiMode);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("CTX Set Verify Mode OK");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTX Set Verify Mode Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
                    inDISP_DispLogAndWriteFlie("  CTX Verify Mode *Error* usRetVal[0x%04X] Line[%d] ", usRetVal,  __LINE__);
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inTLS_TLS2_CTX_Free_
Date&Time       :2017/8/2 下午 3:39
Describe        :Free TLS Context Object
*/
int inTLS_CTX_Free(unsigned int *uiCTX_ID)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_TLS2_CTX_Free(*uiCTX_ID);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "TLS_CTX Free OK CTXID: %u", *uiCTX_ID);
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "TLS_CTX Free Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
                   inDISP_DispLogAndWriteFlie("  TLS_CTX Free *Error* usRetVal[0x%04X] Line[%d] ", usRetVal,  __LINE__);
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}


/*
Function        :inTLS_TLS_New
Date&Time       :2017/7/31 上午 11:55
Describe        :創一個TLS連線
*/
int inTLS_TLS_New(unsigned int uiCTX, unsigned int *uiSSLID)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_TLS2_New(uiCTX, uiSSLID);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "TLS New OK CTX: %u,SSLID: %u", uiCTX, *uiSSLID);
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "TLS New Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
                    inDISP_DispLogAndWriteFlie(" inTLS_TLS_New *Error* usRetVal[0x%04X] Line[%d]", usRetVal,  __LINE__);
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_SetVerificationMode
Date&Time       :2017/7/31 下午 1:15
Describe        :設定SSL連線的驗證方式
 * This function sets the verification mode of SSL session for remote peers
 * 
 * use either SSL_VERIFY_NONE or SSL_VERIFY_PEER, the last 2 options
 * are 'ored' with SSL_VERIFY_PEER if they are desired 
*/
int inTLS_SetVerificationMode(unsigned int uiSSLID, unsigned int uiMode)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_TLS2_SetVerificationMode(uiSSLID, uiMode);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("TLS Set Verify Mode OK");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "TLS Set Verify Mode Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
                    inDISP_DispLogAndWriteFlie(" TLS Set Verify Mode *Error* usRetVal[0x%04X] Line[%d]", usRetVal,  __LINE__);
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inTLS_SetCipherList
Date&Time       :2017/7/31 下午 3:49
Describe        :加密算法套裝列表：客戶端支持的加密-簽名算法的列表，讓伺服器去選擇
 *	szList :The null-terminated text string and a colon-delimited list of cipher suites to use with the specified SSL session.
 *	szList可以填"ALL"
*/
int inTLS_SetCipherList(unsigned int uiSSLID,unsigned char *uszList)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_TLS2_SetCipherList(uiSSLID, uszList);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("TLS Set CipherList OK");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "TLS Set CipherList Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
                inDISP_DispLogAndWriteFlie(" TLS Set CipherList *Error* usRetVal[0x%04X] Line[%d]", usRetVal,  __LINE__);
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inTLS_SetSocket
Date&Time       :2017/8/1 下午 5:49
Describe        :
*/
int inTLS_SetSocket(unsigned int uiSSL_ID, int inSocketHandle)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_TLS2_SetSocket(uiSSL_ID, inSocketHandle);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("TLS Set Socket OK");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "TLS Set Socket Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
        
                inDISP_DispLogAndWriteFlie(" TLS Set Socket *Error* usRetVal[0x%04X] Line[%d]", usRetVal,  __LINE__);
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inTLS_SetProtocolVersion
Date&Time       :2017/8/1 下午 5:52
Describe        :
*/
int inTLS_SetProtocolVersion(unsigned int uiSSL_ID, unsigned int uiProtocolVersion)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_TLS2_SetProtocolVersion(uiSSL_ID, uiProtocolVersion);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("TLS Set Protocol Version OK");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "TLS Set Protocol Version Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
                    inDISP_DispLogAndWriteFlie(" TLS Set Protocol Version *Error* usRetVal[0x%04X] Line[%d]", usRetVal,  __LINE__);
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inTLS_TLS2_Connect
Date&Time       :2017/8/1 下午 5:52
Describe        :
*/
int inTLS_TLS2_Connect(unsigned int uiSSL_ID)
{
//	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_TLS2_Connect(uiSSL_ID);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("TLS Connect OK");
		}
	}
	else
	{
                inDISP_DispLogAndWriteFlie("  TLS Connect *Error* usRetVal[0x%04X] Line[%d] ", usRetVal,  __LINE__);
//		if (ginDebug == VS_TRUE)
//		{
//			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
//			sprintf(szDebugMsg, "TLS Connect Err :0x%04X", usRetVal);
//			inDISP_LogPrintf(szDebugMsg);
//		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inTLS_Send_Data
Date&Time       :2017/8/2 上午 9:36
Describe        :Send Data的function
*/
int inTLS_Send_Data(unsigned int uiSSL_ID, unsigned char *uszData, int *inDataLen)
{
	int		inTempLen = 0;
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;
	
	inTempLen = *inDataLen;

	usRetVal = CTOS_TLS2_Write(uiSSL_ID, uszData, inDataLen);
	/* 若沒全部傳送完也算失敗 */
	if (usRetVal == d_OK && *inDataLen == inTempLen)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("TLS Send_Data OK");
		}
	}
	else if (usRetVal == d_TLS2_WRITE_FAIL)
	{
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "TLS Send_Data Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
                    inDISP_DispLogAndWriteFlie("  TLS Send_Data *Error* usRetVal[0x%04X] Line[%d] ", usRetVal,  __LINE__);
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inTLS_Read_Data
Date&Time       :2017/8/2 上午 11:28
Describe        :
*/
int inTLS_Read_Data(unsigned int uiSSL_ID, unsigned char *uszData, int *inDataLen)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_TLS2_Read(uiSSL_ID, uszData, inDataLen);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("TLS Read_Data OK");
		}
	}
	else if (usRetVal == d_TLS2_READ_FAIL)
	{
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "TLS Read_Data Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
                    inDISP_DispLogAndWriteFlie("  TLS Read_Data *Error* usRetVal[0x%04X] Line[%d] ", usRetVal,  __LINE__);
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inTLS_TLS_Disconnect
Date&Time       :2017/8/2 下午 3:27
Describe        :TLS斷線
 * uiFlag:RFU, this field should be 0.
*/
int inTLS_TLS2_Disconnect(unsigned int uiSSL_ID, unsigned int uiFlag)
{
//	int		inRetVal = VS_ERROR;
	char		szDebugMsg[100 + 1];
//	unsigned char	uszData[1 + 1];
//	unsigned short	usLen = 0;
	unsigned short	usRetVal = 0;
	
	/* 保留欄位 等API文件更新再開放使用 */
	uiFlag = 0;
	
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 1);
	do
	{
		usRetVal = CTOS_TLS2_Disconnect(uiSSL_ID, uiFlag);
		if (usRetVal == d_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "TLS2 Disconnect OK SSL_ID: %u", uiSSL_ID);
				inDISP_LogPrintf(szDebugMsg);
			}
		}
		else
		{
//			do
//			{
//				usLen = 1;
//				inRetVal = inETHERNET_Receive_Data_TLS(uiSSL_ID, uszData, &usLen);
//			} while (inRetVal == VS_SUCCESS);
		}
		
	} while (usRetVal != d_OK && inTimerGet(_TIMER_NEXSYS_1_) != VS_SUCCESS);
	
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, " TLS2 Disconnect Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			CTOS_TLS2_GetErrorMessage (uiSSL_ID, (unsigned char*)szDebugMsg);
			inDISP_LogPrintf(szDebugMsg);
		}
                    memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                    CTOS_TLS2_GetErrorMessage (uiSSL_ID, (unsigned char*)szDebugMsg);
		inDISP_DispLogAndWriteFlie(" TLS2 Disconnect *Error* usRetVal[0x%04X] Line[%d] ", usRetVal,  __LINE__);
                    inDISP_DispLogAndWriteFlie(" TLS2 Disconnect *Error* Msg[%s] Line[%d] ", szDebugMsg,  __LINE__);
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inTLS_TLS2_Free
Date&Time       :2017/8/2 下午 3:35
Describe        :釋放TLSObject
*/
int inTLS_TLS2_Free(unsigned int *uiSSL_ID)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("----------------------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inTLS_TLS2_Free() START ! SSLID:%u", *uiSSL_ID);
		inDISP_LogPrintf(szDebugMsg);
	}

	usRetVal = CTOS_TLS2_Free(*uiSSL_ID);
	if (usRetVal == d_OK)
	{		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "TLS2 Free OK SSLID: %u", *uiSSL_ID);
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "TLS2 Free Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
        
        		inDISP_DispLogAndWriteFlie("  TLS2 Free *Error* usRetVal[0x%04X] Line[%d] ", usRetVal,  __LINE__);
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inTLS_TLS2_GetSession
Date&Time       :2017/8/2 下午 4:57
Describe        :確認現在Session狀態
*/
int inTLS_TLS2_GetSession(unsigned int uiSSL_ID)
{
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_TLS2_GetSession(uiSSL_ID);
	if (usRetVal != d_OK)
	{
		inDISP_DispLogAndWriteFlie("  TLS2 Session *Error* usRetVal[0x%04X] Line[%d] ", usRetVal,  __LINE__);
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inTLS_Process_CTX_Flow
Date&Time       :2017/8/2 下午 4:26
Describe        :
*/
int inTLS_Process_CTX_Flow(unsigned int *uiCTX_ID)
{
        int	inRetVal = VS_ERROR;
        char	szCACertification[40 + 1];
        char	szHostName[8 + 1];
	
        /* 處理context */
        /* 新建一個CTX */
        inRetVal = inTLS_CTX_New(d_TLS2_METHOD_CLIENT_TLS_V1_2, uiCTX_ID);
        if (inRetVal != VS_SUCCESS)
        {
                return (inRetVal);
        }

        /* 載入CA憑證 */
        memset(szHostName, 0x00, sizeof(szHostName));
        inGetHostLabel(szHostName);
	
        memset(szCACertification, 0x00, sizeof(szCACertification));
        
	memcpy(szCACertification, _PEM_CA_CREDIT_NCCC_FILE_NAME_, strlen(_PEM_CA_CREDIT_NCCC_FILE_NAME_));

        inDISP_LogPrintfWithFlag(" ESUN CA[%s][%d]",szCACertification, __LINE__);
        
	inRetVal = inTLS_CTX_LoadCACertificationFromFile(*uiCTX_ID, (unsigned char*)szCACertification);
	if (inRetVal != VS_SUCCESS)
	{         
                    inDISP_DispLogAndWriteFlie(" ESUN CA[%s][%d]",szCACertification, __LINE__);
                    return (inRetVal);
	}
	
	/* 載入client憑證 NCCC不驗 */
//	inRetVal = inTLS_CTX_LoadCertificationFromFile(*uiCTX_ID, (unsigned char*)"", d_TLS2_FILETYPE_PEM);
//	if (inRetVal != VS_SUCCESS)
//	{
//		return (inRetVal);
//	}
	
	/* 載入client key NCCC不驗 */
//	inRetVal = inTLS_CTX_LoadPrivateKeyFromFile(*uiCTX_ID, (unsigned char*)"", d_TLS2_FILETYPE_PEM);
//	if (inRetVal != VS_SUCCESS)
//	{
//		return (inRetVal);
//	}
	
	/* 根據Verifone code，設定為d_TLS2_VERIFY_PEER */
	inRetVal = inTLS_CTX_SetVerificationMode(*uiCTX_ID, d_TLS2_VERIFY_PEER);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	return (inRetVal);
}

/*
Function        :inTLS_Process_TLS_Flow
Date&Time       :2017/8/2 下午 4:26
Describe        :
*/
int inTLS_Process_TLS_Flow(unsigned int *uiCTX_ID, unsigned int *uiSSLID)
{
	int	inRetVal = VS_ERROR;
	char	szDebugMsg[100 + 1];
	char	szHostName[8 + 1];
	
	/* 處理TLS */
	/* 建立新TLS Object */
	inRetVal = inTLS_TLS_New(*uiCTX_ID, uiSSLID);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	/* 設定驗證方式 */
	inRetVal = inTLS_SetVerificationMode(*uiSSLID, SSL_VERIFY_NONE);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	/* 設定加密列表 */
	inRetVal = inTLS_SetCipherList(*uiSSLID, (unsigned char*)"ALL");
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	/* 設定socket */
	inRetVal = inTLS_SetSocket(*uiSSLID, ginTrans_ClientFd);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
 
#if (_MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_) && defined(_ESUN_MAIN_HOST_)
          /* 設定TSL連線時間 要注意設定時需要比我們在等待的時間長   2021/11/10 [SAM] */
	inRetVal = inTLS_TLS_SetTls2TimeOut(*uiSSLID, 15);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
#endif	    
	/* 設定協議標準 */
	memset(szHostName, 0x00, sizeof(szHostName));
	inGetHostLabel(szHostName);
	
	inRetVal = inTLS_SetProtocolVersion(*uiSSLID, d_TLS2_VERSION_TLSV1_2);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	/* 1秒內要連線 */
#if (_MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_) && defined(_ESUN_MAIN_HOST_)
        /* 修改時間為10 秒  2021/11/10 [SAM] */
        inDISP_Timer_Start(_TIMER_NEXSYS_1_, 10);	
#else
          inDISP_Timer_Start(_TIMER_NEXSYS_1_, 1);	
#endif
	do
	{
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "TLS Connet Timeout");
				inDISP_LogPrintf(szDebugMsg);
				
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				CTOS_TLS2_GetErrorMessage (*uiSSLID, (unsigned char*)szDebugMsg);
				inDISP_LogPrintf(szDebugMsg);
			}
            
                            memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                            CTOS_TLS2_GetErrorMessage (*uiSSLID, (unsigned char*)szDebugMsg);
                            inDISP_DispLogAndWriteFlie(" TLS Connet Timeout ErrMsg[%s] Line[%d]", szDebugMsg,  __LINE__);
            
			return (VS_TIMEOUT);
		}
		
		inRetVal = inTLS_TLS2_Connect(*uiSSLID);
		
	} while (inRetVal != VS_SUCCESS);
	
	return (inRetVal);
}



/*
Function        : inTLS_TLS_SetTls2TimeOut
Date&Time   : 2021/11/10 下午 3:08
Describe        : 設定TLS的連線
*/
int inTLS_TLS_SetTls2TimeOut(unsigned int uiSSLID, int inTimeOut)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_TLS2_SetTimeout(uiSSLID, inTimeOut);
	if (usRetVal == d_OK)
	{
		inDISP_LogPrintfWithFlag("TLS New OK SSLID: %lu", uiSSLID);
	}
	else
	{
                    memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                    CTOS_TLS2_GetErrorMessage (uiSSLID, (unsigned char*)szDebugMsg);
                   
                    inDISP_DispLogAndWriteFlie(" inTLS_TLS_SetTls2TimeOut *Error* ErrMsg[%s] Line[%d]", szDebugMsg,  __LINE__);
                    inDISP_DispLogAndWriteFlie(" inTLS_TLS_SetTls2TimeOut *Error* usRetVal[0x%04X] Line[%d]", usRetVal,  __LINE__);
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}




/*
Function        :inTLS_Test
Date&Time       :2017/7/28 下午 5:04
Describe        :用來做簡易測試
*/
int inTLS_Test()
{
	int			i;
	int			inRetVal = VS_ERROR;
	int			inDataLen = 0;
	char			szReadData[1024 + 1];
	unsigned int		uiCTX_ID;
	unsigned int		uiSSLID;
	TRANSACTION_OBJECT	pobTran;
	
	memset(&pobTran, 0x00, sizeof(pobTran));
	inCOMM_InitCommDevice();
	
	pobTran.srBRec.inHDTIndex = 0;
	
	inCOMM_ConnectStart(&pobTran);
	
	/* 處理context */
	inTLS_CTX_New(d_TLS2_METHOD_CLIENT_TLS_V1_2, &uiCTX_ID);
	inTLS_CTX_LoadCACertificationFromFile(uiCTX_ID, (unsigned char*)"ca.crt");
	inTLS_CTX_LoadCertificationFromFile(uiCTX_ID, (unsigned char*)"client.csr.pem", d_TLS2_FILETYPE_PEM);
	inTLS_CTX_LoadPrivateKeyFromFile(uiCTX_ID, (unsigned char*)"client.key.pem", d_TLS2_FILETYPE_PEM);
	inTLS_CTX_SetVerificationMode(uiCTX_ID, d_TLS2_VERIFY_PEER);
	
	/* 處理TLS */
	inTLS_TLS_New(uiCTX_ID, &uiSSLID);
	inTLS_SetVerificationMode(uiSSLID, SSL_VERIFY_NONE);
	inTLS_SetCipherList(uiSSLID, (unsigned char*)"ALL");
	inTLS_SetSocket(uiSSLID, ginTrans_ClientFd);
	inTLS_SetProtocolVersion(uiSSLID, d_TLS2_VERSION_TLSV1);
	
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		
	do
	{
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			return (VS_TIMEOUT);
		}
		
		inRetVal = inTLS_TLS2_Connect(uiSSLID);
		
	} while (inRetVal != VS_SUCCESS);
	
	for (i = 0; i < 5; i++)
	{
		inDataLen = strlen("12345TEST");
		inTLS_Send_Data(uiSSLID, (unsigned char*)"12345TEST" , &inDataLen);
		uszKBD_GetKey(30);
	}
	
	for (i = 0; i < 5; i++)
	{
		memset(szReadData, 0x00, sizeof(szReadData));
		inDataLen = sizeof(szReadData);
		inTLS_Read_Data(uiSSLID, (unsigned char*)szReadData , &inDataLen);
		uszKBD_GetKey(30);
	}
	
	inTLS_TLS2_Disconnect(uiSSLID, 0);
	inTLS_TLS2_Free(&uiSSLID);
	inTLS_CTX_Free(&uiCTX_ID);
	
	inCOMM_End(&pobTran);
	
	return (VS_SUCCESS);
}
